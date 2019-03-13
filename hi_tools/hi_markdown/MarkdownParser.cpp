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


namespace hise {
using namespace juce;


void MarkdownParser::parse()
{
	try
	{
		if (it.getRestString().startsWith("---"))
		{
			parseMarkdownHeader();
		}
		
		while (it.peek() != 0)
			parseBlock();

		currentParseResult = Result::ok();
	}
	catch (String& s)
	{
		currentParseResult = Result::fail(s);
	}
}


void MarkdownParser::parseLine()
{
	resetForNewLine();
	currentColour = styleData.textColour.withAlpha(0.8f);

	parseText();

	while (!Helpers::isNewElement(it.peek()))
	{
		parseText();
	}

	elements.add(new TextBlock(this, currentlyParsedBlock));
}

void MarkdownParser::resetForNewLine()
{
	currentFont = styleData.getFont();
	currentColour = styleData.textColour;
	resetCurrentBlock();
}

void MarkdownParser::parseHeadline()
{
	resetCurrentBlock();

	currentColour = Colour(SIGNAL_COLOUR);

	

	juce::juce_wchar c = it.peek();
	int headlineLevel = 3;

	while (it.matchIf('#'))
	{
		headlineLevel--;
	}

	headlineLevel = jmax(0, headlineLevel);

	currentFont = styleData.f.withHeight(styleData.fontSize * 3 / 2 + 7 * headlineLevel);

	currentFont = FontHelpers::getFontBoldened(currentFont);

	String imageURL;

	it.skipWhitespace();

	if (it.peek() == '!')
	{
		it.match('!');
		it.match('[');
		
		while (it.next(c))
		{
			if (c == ']')
				break;
		}

		it.match('(');

		while (it.next(c))
		{
			if (c == ')')
				break;

			imageURL << c;
		}
	}

	parseText();

	isBold = false;

	elements.add(new Headline(this, headlineLevel, imageURL, currentlyParsedBlock, elements.size() == 0));

}

void MarkdownParser::parseBulletList()
{
	Array<AttributedString> bulletpoints;
	Array<Array<HyperLink>> links;

	while (it.peek() == '-')
	{
		skipTagAndTrailingSpace();

		resetCurrentBlock();
		resetForNewLine();

		parseText();

		links.add(currentLinks);
		bulletpoints.add(currentlyParsedBlock);
	}

	elements.add(new BulletPointList(this, bulletpoints, links));


	currentFont = styleData.getFont();

}

void MarkdownParser::parseEnumeration()
{
	Array<AttributedString> listItems;

	Array<Array<HyperLink>> links;

	while (CharacterFunctions::isDigit(it.peek()))
	{
		while(CharacterFunctions::isDigit(it.peek()))
			skipTagAndTrailingSpace();
		
		if (it.peek() == '.')
		{
			skipTagAndTrailingSpace(); // the dot

			resetCurrentBlock();
			resetForNewLine();

			while (!Helpers::isNewElement(it.peek()))
			{
				parseText();
			}

			links.add(currentLinks);
			listItems.add(currentlyParsedBlock);
		}
		else
		{
			parseLine();
			return;
		}
	}

	elements.add(new EnumerationList(this, listItems, links));

	currentFont = styleData.getFont();
}




void MarkdownParser::parseText(bool stopAtEndOfLine)
{
	juce_wchar c;

	it.next(c);



	//while (!Helpers::isEndOfLine(c))

	while (Helpers::belongsToTextBlock(c, isCode, stopAtEndOfLine))
	{
		switch (c)
		{
		case '*':
		{
			if (!isCode)
			{
				if (it.peek() == '*')
				{
					it.next(c);

					isBold = !isBold;

					float size = currentFont.getHeight();

					if (isBold)
						currentFont = FontHelpers::getFontBoldened(styleData.getFont().withHeight(size));
					else
						currentFont = styleData.getFont().withHeight(size);
				}
				else
				{
					isItalic = !isItalic;

					if (isItalic)
						currentFont = FontHelpers::getFontItalicised(currentFont);
					else
						currentFont = FontHelpers::getFontNormalised(currentFont);
				}
			}
			else
			{
				addCharacterToCurrentBlock(c);
			}

			break;
		}
		case '`':
		{
			isCode = !isCode;

			auto size = currentFont.getHeight();
			auto b = currentFont.isBold();
			auto i = currentFont.isItalic();
			auto u = currentFont.isUnderlined();

			currentFont = isCode ? GLOBAL_MONOSPACE_FONT() : styleData.getFont();

			currentColour = isCode ? styleData.textColour : styleData.textColour.withAlpha(0.8f);

			currentFont.setHeight(size);
			currentFont.setBold(b);
			currentFont.setItalic(i);
			currentFont.setUnderline(u);

			break;
		}
		case ' ':
		{
			if (it.peek() == ' ')
			{
				it.next(c);
				addCharacterToCurrentBlock('\n');
			}
			else
			{
				addCharacterToCurrentBlock(c);
			}

			break;
		}
		case '[':
		{
			if (isCode)
			{
				addCharacterToCurrentBlock(c);
			}
			else
			{
				String urlId;
				String url;

				bool ok = false;

				bool useCodeFontForLink = false;

				while (it.next(c))
				{
					if (c == '`')
					{
						useCodeFontForLink = true;
						continue;
					}
						

					if (c == ']')
					{
						ok = true;
						break;
					}
					else
						urlId << c;
				}

				if (ok)
				{

					if (it.matchIf('('))
					{
						ok = false;

						while (it.next(c))
						{
							if (c == ')')
							{
								ok = true;
								break;
							}
							else
								url << c;
						}
					}
					else
					{
						ok = false;
					}

				}

				if (urlId.toLowerCase().startsWith("button: "))
				{
					urlId = urlId.fromFirstOccurrenceOf("Button: ", false, true);
				}

				if (ok)
				{
					auto text = currentlyParsedBlock.getText();

					auto start = text.length();

					currentFont = useCodeFontForLink ? GLOBAL_MONOSPACE_FONT().withHeight(styleData.fontSize) : styleData.getFont();
					
					currentFont.setUnderline(true);
					currentlyParsedBlock.append(urlId, currentFont, Colour(SIGNAL_COLOUR));
					currentFont.setUnderline(false);

					currentFont = isCode ? GLOBAL_MONOSPACE_FONT().withHeight(styleData.fontSize) : styleData.getFont();

					auto stop = start + urlId.length();

					HyperLink hyperLink;

					hyperLink.url = MarkdownLink::Helpers::getSanitizedFilename(url);
					hyperLink.urlRange = { start, stop };
					hyperLink.displayString = urlId;
					hyperLink.valid = true;

					currentLinks.add(std::move(hyperLink));
				}
				else
				{
					currentlyParsedBlock.append("[" + urlId + "]");
				}
					
			}

			

			break;
		}
		case '|':
			if (!isCode)
			{
				return;
			}
		case '\n':
		{
			if (!stopAtEndOfLine)
				it.advanceIfNotEOF();
		}
		default:
			addCharacterToCurrentBlock(c);

		}

		if (!it.next(c))
			return;
	}
}



void MarkdownParser::parseBlock()
{
	juce_wchar c = it.peek();

	switch (c)
	{
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9': parseEnumeration(); break;
	case '#': parseHeadline();
		break;
	case '-': parseBulletList();
		break;
	case '>': parseComment();
		break;
	case '`': if (isJavascriptBlock())
		parseJavascriptBlock();
			  else
				  parseLine();
		break;
	case '|': parseTable();
		break;
	case '$': parseButton();
		break;
	case '!': if (isImageLink())
		elements.add(parseImage());
			  else
				  parseLine();
		break;
	case '\n': it.match('\n');
		break;
	default:  parseLine();
		break;

	};
}


void MarkdownParser::parseButton()
{
	it.match('$');
	it.match('[');

	String urlId;
	String link;

	juce_wchar c;

	bool ok = false;

	while (it.next(c))
	{
		if (c == ']')
		{
			ok = true;
			break;
		}
		else
			urlId << c;
	}

	if (ok)
	{
		it.match('(');

		while (it.next(c))
		{
			if (c == ')')
			{
				ok = true;
				break;
			}
			else
				link << c;
		}

		if (!ok)
		{
			it.match(')');
			jassertfalse;
		}
	}
	else
	{
		// should throw
		it.match('[');

		jassertfalse;
	}

	elements.add(new ActionButton(this, urlId, link));
}



void MarkdownParser::parseTable()
{
	RowContent headerItems;


	it.peek();

	while (!Helpers::isEndOfLine(it.peek()))
	{
		skipTagAndTrailingSpace();

		resetForNewLine();
		resetCurrentBlock();

		CellContent newCell;

		if (isImageLink())
		{
			ScopedPointer<ImageElement> e = parseImage();
			newCell.imageURL = e->getImageURL();
		}
		else
		{
			parseText();
			currentlyParsedBlock.setFont(FontHelpers::getFontBoldened(styleData.getFont()));
			newCell.s = currentlyParsedBlock;
		}


		if (!newCell.isEmpty())
			headerItems.add(newCell);
	}

	if (!it.advanceIfNotEOF())
		throw String("Table lines needed");

	Array<int> lengths;

	while (!Helpers::isEndOfLine(it.peek()))
	{
		resetCurrentBlock();

		parseText();

		int length = 0;

		auto columnLine = currentlyParsedBlock.getText().trim();

		if (columnLine.contains(":"))
		{
			length = MarkdownTable::FixOffset + currentlyParsedBlock.getText().fromFirstOccurrenceOf(":", false, false).getIntValue();
		}
		else if (!columnLine.containsOnly("-=_ "))
		{
			throw String("Table lines illegal text: " + currentlyParsedBlock.getText());
		}
		else
		{
			length = columnLine.length();
		}

		if(length != 0)
			lengths.add(length);
	}

	Array<RowContent> rows;

	if (!it.advanceIfNotEOF())
		throw String("No table rows defined");

	while (it.peek() == '|')
	{
		rows.add(parseTableRow());
	}

	elements.add(new MarkdownTable(this, headerItems, lengths, rows));

}

MarkdownParser::ImageElement* MarkdownParser::parseImage()
{
	it.match('!');
	it.match('[');

	auto imageName = it.getRestString().upToFirstOccurrenceOf("]", false, false);

	it.advance(imageName);

	it.match(']');
	it.match('(');

	auto imageLink = it.getRestString().upToFirstOccurrenceOf(")", false, false);

	it.advance(imageLink);

	it.match(')');

	return new ImageElement(this, imageName, imageLink);


}

Array<MarkdownParser::CellContent> MarkdownParser::parseTableRow()
{
	Array<CellContent> entries;

	while (!Helpers::isEndOfLine(it.peek()))
	{
		skipTagAndTrailingSpace();
		resetCurrentBlock();
		resetForNewLine();

		CellContent c;

		if (isImageLink())
		{
			ScopedPointer<ImageElement> e = parseImage();
			c.imageURL = e->getImageURL();
		}
		else
		{
			parseText();
			c.s = currentlyParsedBlock;
			c.cellLinks = currentLinks;
		}

		if (!c.isEmpty())
			entries.add(c);
	}


	if (!it.advanceIfNotEOF())
		return entries;

	return entries;
}

bool MarkdownParser::isJavascriptBlock() const
{
	auto restString = it.getRestString();
	return restString.startsWith("```");
}

bool MarkdownParser::isImageLink() const
{
	auto restString = it.getRestString();
	return restString.startsWith("![");
}

void MarkdownParser::addCharacterToCurrentBlock(juce_wchar c)
{
	currentlyParsedBlock.append(String::charToString(c), currentFont, currentColour);
}

void MarkdownParser::resetCurrentBlock()
{
	currentlyParsedBlock = AttributedString();
	currentlyParsedBlock.setLineSpacing(8.0f);
	currentLinks.clearQuick();
}

void MarkdownParser::skipTagAndTrailingSpace()
{
	if (it.peek() == 0)
		return;

	it.advance();

	if (it.peek() == ' ')
		it.advance();
}

void MarkdownParser::parseComment()
{
	resetForNewLine();

	skipTagAndTrailingSpace();

	parseText();

	elements.add(new Comment(this, currentlyParsedBlock));


}


void MarkdownParser::parseMarkdownHeader()
{
	it.advance("---");
	it.match('\n');

	StringArray lines;


	while (!it.getRestString().startsWith("---"))
	{
		auto line = it.advanceLine().trim();
		
		if (line.isEmpty())
			break;

		lines.add(line);
	}

	MarkdownHeader = {};

	
	
	for (auto line : lines)
	{
		if (line.contains(":") && !line.trim().startsWith("-"))
		{
			MarkdownHeader::Item newItem;

			newItem.key = line.upToFirstOccurrenceOf(":", false, false).trim();
			
			auto samelineValue = line.fromFirstOccurrenceOf(":", false, false).trim();

			if (samelineValue.isNotEmpty())
				newItem.values.add(samelineValue);

			MarkdownHeader.items.add(std::move(newItem));
		}
		else
		{
			auto nextValue = line.fromFirstOccurrenceOf("-", false, false).trim();

			if (nextValue.isEmpty())
				throw String("Error at YAML Header parsing: no value");

			if (MarkdownHeader.items.isEmpty())
				throw String("Error at YAML Header parsing: no item for list");

			MarkdownHeader.items.getReference(MarkdownHeader.items.size() - 1).values.add(nextValue);
		}
	}

	it.match('-');
	it.match('-');
	it.match('-');
	it.match('\n');

	auto headline = MarkdownHeader.getKeywords()[0];

	if (headline.isNotEmpty())
	{
		AttributedString s;

		auto f = styleData.f.withHeight(styleData.fontSize * 3 / 2 + 7 * 3);

		f = FontHelpers::getFontBoldened(f);

		s.append(headline, f, styleData.headlineColour);

		elements.add(new Headline(this, 3, MarkdownHeader.getKeyValue("icon"), s, true));
	}

}




}
