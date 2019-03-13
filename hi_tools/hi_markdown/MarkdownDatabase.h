/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licenses for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licensing:
*
*   http://www.hise.audio/
*
*   HISE is based on the JUCE library,
*   which must be separately licensed for closed source applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

#pragma once

namespace hise {
using namespace juce;


class MarkdownDataBase
{
public:

	struct Item
	{
		using IteratorFunction = std::function<bool(Item&)>;

		struct Sorter
		{
			static int compareElements(Item& first, Item& second);
		};

		struct PrioritySorter
		{
			PrioritySorter(const String& searchString_) :
				searchString(searchString_)
			{};

			Array<Item> sortItems(Array<Item>& arrayToBeSorted);

			String searchString;
		};

		enum Type
		{
			Invalid = 0,
			Root,
			Folder,
			Keyword,
			Filename,
			Headline,
			numTypes
		};

#if 0
		bool containsURL(const String& urlToLookFor) const
		{
			for (const auto& c : children)
			{
				if (c.containsURL(urlToLookFor))
					return true;
			}

			return urlToLookFor.url == urlToLookFor;
		}
#endif


		bool callForEach(const IteratorFunction& f)
		{
			if (f(*this))
				return true;

			for (auto& c : children)
			{
				if (c.callForEach(f))
					return true;
			}

			return false;
		}

#if 0
		Item* findChildWithURL(const String& urlToLookFor) const
		{
			if (url == urlToLookFor)
				return const_cast<Item*>(this);

			for (const auto& c : children)
			{
				if (auto r = c.findChildWithURL(urlToLookFor))
					return r;
			}

			return nullptr;
		}
#endif

		bool swapChildWithName(Item& itemToSwap, const String& name)
		{
			for (auto& i : children)
			{
				if (i.url.toString(MarkdownLink::UrlSubPath) == name)
				{
					std::swap(i, itemToSwap);
					return true;
				}
			}

			return false;
		}

		Item getChildWithName(const String& name) const
		{
			if (url.toString(MarkdownLink::UrlSubPath) == name)
				return *this;

			for (const auto& c : children)
			{
				auto i = c.getChildWithName(name);

				if (i.type != Invalid)
					return i;
			}

			return {};
		}

		Item createChildItem(const String& subPath) const;

		Item(Type t, File root, File f, const StringArray& keywords_, String description_);
		Item() {};

		Item(const MarkdownLink& link);

		int fits(String search) const;
		String generateHtml(const String& rootString, const String& activeUrl) const;
		void addToList(Array<Item>& list) const;

		void addTocChildren(File root);

		ValueTree createValueTree() const;
		void loadFromValueTree(ValueTree& v);

		Array<Item> children;

		Type type = Type::Invalid;
		String tocString;
		
		MarkdownLink url;
		StringArray keywords;
		String description;
		bool isAlwaysOpen = false;
		Colour c;
		String icon;
	};

	struct ItemGeneratorBase
	{
		ItemGeneratorBase(File rootDirectory_):
			rootDirectory(rootDirectory_)
		{}

		File getFolderReadmeFile(const String& folderURL);

		virtual ~ItemGeneratorBase() {};
		virtual Item createRootItem(MarkdownDataBase& parent) = 0;

		File rootDirectory;
		MarkdownDataBase::Item rootItem;
	};

	struct DirectoryItemGenerator : public ItemGeneratorBase
	{
		DirectoryItemGenerator(const File& rootDirectory, Colour colour);
		Item createRootItem(MarkdownDataBase& parent) override;
		void addFileRecursive(Item& folder, File f);

		File startDirectory;
		Colour c;
	};

	

	MarkdownDataBase();
	~MarkdownDataBase();

	Item rootItem;
	const Array<Item>& getFlatList();

	void setRoot(const File& newRootDirectory);
	File getRoot() const { return rootDirectory; }

	String generateHtmlToc(const String& activeUrl) const;
	
	void addItemGenerator(ItemGeneratorBase* newItemGenerator);

	var getHtmlSearchDatabaseDump();

	File getDatabaseFile()
	{
		return getRoot().getChildFile("Content.dat");
	}

	void clear()
	{
		itemGenerators.clear();
		cachedFlatList.clear();
        rootDirectory = File();
		rootItem = {};
	}

private:

	friend class MarkdownDatabaseHolder;

	void buildDataBase();

	Array<Item> cachedFlatList;

	File rootDirectory;
	OwnedArray<ItemGeneratorBase> itemGenerators;

	

	void loadFromValueTree(ValueTree& v)
	{
		rootItem = {};
		rootItem.loadFromValueTree(v);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MarkdownDataBase);
};

class MarkdownContentProcessor;

struct MarkdownDatabaseHolder
{
	struct DatabaseListener
	{
		virtual ~DatabaseListener() {};

		virtual void databaseWasRebuild() = 0;

	private:

		JUCE_DECLARE_WEAK_REFERENCEABLE(DatabaseListener);
	};

	const MarkdownDataBase& getDatabase() const
	{
		return db;
	}

	MarkdownDataBase& getDatabase()
	{
		return db;
	}

	virtual void registerItemGenerators() = 0;
	
	virtual File getCachedDocFolder() const = 0;
	virtual File getDatabaseRootDirectory() const = 0;

	void setForceCachedDataUse(bool shouldUseCachedData, bool rebuild=true)
	{
		if (forceUseCachedData != shouldUseCachedData)
		{
			forceUseCachedData = shouldUseCachedData;

			if (rebuild)
			{
				rebuildDatabase();
			}
		}
	}

	virtual bool shouldUseCachedData() const 
	{ 
		if (forceUseCachedData)
			return true;

		return !getDatabaseRootDirectory().isDirectory(); 
	}

	File rootFile;
	MarkdownDataBase db;

	void addDatabaseListener(DatabaseListener* l) { listeners.addIfNotAlreadyThere(l); }
	void removeDatabaseListener(DatabaseListener* l) { listeners.removeAllInstancesOf(l); }

	void rebuildDatabase();


	void addContentProcessor(MarkdownContentProcessor* contentProcessor);

	void removeContentProcessor(MarkdownContentProcessor* contentToRemove)
	{
		contentProcessors.removeAllInstancesOf(contentToRemove);
	}

	void addItemGenerator(MarkdownDataBase::ItemGeneratorBase* newItemGenerator)
	{
		db.addItemGenerator(newItemGenerator);
	}

	void setProgressCounter(double* p)
	{
		progressCounter = p;
	}

private:

	double* progressCounter = nullptr;

	virtual void registerContentProcessor(MarkdownContentProcessor* processor) = 0;

	Array<WeakReference<MarkdownContentProcessor>> contentProcessors;

	bool forceUseCachedData = true;

	Array<WeakReference<DatabaseListener>> listeners;
};



}