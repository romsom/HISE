#ifndef __FAUST_MENU_BAR_H
#define __FAUST_MENU_BAR_H

namespace scriptnode {
namespace faust {

// Additional types for faust_jit_node
struct FaustMenuBar : public Component,
                      public ButtonListener,
                      public ComboBox::Listener

{

    FaustMenuBar(faust_jit_node *n) :
        addButton("add", this, factory),
        editButton("edit", this, factory),
        reloadButton("reset", this, factory),
        node(n)
    {
        // we must provide a valid faust_jit_node pointer
        jassert(n);
        setLookAndFeel(&claf);
        setSize(200, 24);

        addAndMakeVisible(classSelector);
        classSelector.setColour(ComboBox::ColourIds::textColourId, Colour(0xFFAAAAAA));
        classSelector.setLookAndFeel(&claf);
        classSelector.addListener(this);

        editButton.setTooltip("Edit the current Faust source file in external editor");
        addAndMakeVisible(addButton);
        addAndMakeVisible(editButton);
        addAndMakeVisible(reloadButton);
        // gather existing source files
        rebuildComboBoxItems();
    }

    struct Factory : public PathFactory
    {
        String getId() const override { return {}; }
        juce::Path createPath(const String& url) const override
        {
            // TODO: Faust Logo
            if (url == "snex")
            {
                snex::ui::SnexPathFactory f;
                return f.createPath(url);
            }

            Path p;

            LOAD_PATH_IF_URL("new", ColumnIcons::threeDots);
            LOAD_PATH_IF_URL("edit", ColumnIcons::openWorkspaceIcon);
            LOAD_PATH_IF_URL("compile", EditorIcons::compileIcon);
            LOAD_PATH_IF_URL("reset", EditorIcons::swapIcon);
            LOAD_PATH_IF_URL("add", ColumnIcons::threeDots);

            return p;
        }
    } factory;

    juce::ComboBox classSelector;
    HiseShapeButton addButton;
    HiseShapeButton editButton;
    HiseShapeButton reloadButton;

    WeakReference<faust_jit_node> node;
    hise::ScriptnodeComboBoxLookAndFeel claf;

    // Define menu options for addButton
    enum MenuOption {
        MENU_OPTION_FIRST = 1,
        NEW_FILE = MENU_OPTION_FIRST,
        // add more options here
        MENU_OPTION_LAST,
        MENU_OPTION_INVALID,
    };

    std::map<int, String> menuOptions = {
        {NEW_FILE, "Create new file"},
            // add description for more options here
        {MENU_OPTION_INVALID, "Invalid Option (BUG)"}
    };

    String getTextForMenuOptionId(int id)
    {
        if (menuOptions.count(id) > 0) return menuOptions[id];
        return menuOptions[MENU_OPTION_INVALID];
    }

    void createNewFile() {
        auto name = PresetHandler::getCustomName(node->getClassId(), "Enter the name for the Faust file");

        if (name.isNotEmpty())
        {
            node->createSourceAndSetClass(name);
            rebuildComboBoxItems();
            //refreshButtonState();
        }
    }

    void executeMenuAction(int option)
    {
        switch(option) {
        case NEW_FILE:
            createNewFile();
            break;

            // add code for more functions here
        default:
            std::cerr << "FaustMenuBar: Unknown MenuOption: " + option << std::endl;
        }
    }


    void rebuildComboBoxItems()
    {
	    // DBG("FaustMenuBar rebuilt");
        classSelector.clear(dontSendNotification);
        classSelector.addItemList(node->getAvailableClassIds(), 1);

        // if (auto w = source->getWorkbench())
        //     classSelector.setText(w->getInstanceId().toString(), dontSendNotification);
        classSelector.setText(node->getClassId());
    }

    virtual void resized() override
    {
	    // DBG("FaustMenuBar resized");
        auto b = getLocalBounds().reduced(0, 1);
        auto h = getHeight();

        addButton.setBounds(b.removeFromLeft(h-4));
        classSelector.setBounds(b.removeFromLeft(100));
        b.removeFromLeft(3);
        editButton.setBounds(b.removeFromRight(h).reduced(2));
        reloadButton.setBounds(b.removeFromRight(h + editButton.getWidth() / 2).reduced(2));

        b.removeFromLeft(10);
    }

    virtual void buttonClicked(Button* b) override
    {
        if (b == &addButton) {

            juce::PopupMenu m;
            m.setLookAndFeel(&claf);
            for (int o=MENU_OPTION_FIRST; o<MENU_OPTION_LAST; o++) {
                m.addItem(o, getTextForMenuOptionId(o), true);
            }

            int menu_selection = (MenuOption)m.show();
            if (menu_selection > 0)
                executeMenuAction(menu_selection);
        } else if (b == &editButton) {
	        DBG("Edit button pressed");
	        auto sourceFile = node->getFaustFile(node->getClassId());
	        auto sourceFilePath = sourceFile.getFullPathName();
	        if (sourceFile.existsAsFile()) {
		        juce::Process::openDocument(sourceFilePath, "");
		        DBG("Opened file: " + sourceFilePath);
	        } else {
		        DBG("File not found: " + sourceFilePath);
	        }
        } else if (b == &reloadButton) {
	        DBG("Reload button pressed");
	        node->reinitFaustWrapper();
        }
    }

	virtual void comboBoxChanged (ComboBox *comboBoxThatHasChanged) override
    {
        auto name = comboBoxThatHasChanged->getText();
        DBG("Combobox changed, new text: " + name);
        node->setClass(name);
    }

};

} // namespace faust
} // namespace scriptnode

#endif // __FAUST_MENU_BAR_H
