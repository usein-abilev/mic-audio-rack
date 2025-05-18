#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <math.h>

#include "plugin_host.h"

class PluginListItem : public juce::Component {
   public:
    PluginListItem(PluginEntry& e,
        std::function<void()> onSelect,
        std::function<void(bool state)> onBypass,
        bool isBypass,
        bool isSelected)
        : plugin(e),
          onSelectCallback(onSelect),
          onBypassCallback(onBypass),
          bypass(isBypass),
          selected(isSelected) {
        nameLabel.setText(plugin.name, juce::dontSendNotification);
        nameLabel.setInterceptsMouseClicks(false, true);
        addAndMakeVisible(nameLabel);

        toggleButton.setButtonText(bypass ? "enable" : "bypass");
        toggleButton.onClick = [this]() {
            bypass = !bypass;
            toggleButton.setButtonText(bypass ? "enable" : "bypass");
            onBypassCallback(bypass);
        };
        addAndMakeVisible(toggleButton);
    }

    void resized() override {
        auto area = getLocalBounds().reduced(4);
        toggleButton.setBounds(area.removeFromRight(80));
        nameLabel.setBounds(area);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(selected ? juce::Colours::darkblue.withAlpha(0.5f) : juce::Colours::darkgrey);
        g.setColour(juce::Colours::black);
        g.drawRect(getLocalBounds(), 1);
    }

    void mouseUp(const juce::MouseEvent& event) override {
        if (onSelectCallback) {
            onSelectCallback();
        }
    }

   private:
    PluginEntry& plugin;
    juce::Label nameLabel;
    juce::TextButton toggleButton, selectButton;
    std::function<void()> onSelectCallback;
    std::function<void(bool state)> onBypassCallback;
    bool selected;
    bool bypass;
};

/**
 * Component that displays a list of plugins in the chain.
 * Each plugin is represented by a PluginListItem.
 */
class PluginChainUI : public juce::Component {
   public:
    PluginChainUI(PluginHost& parentPluginHost)
        : pluginHost(parentPluginHost), pluginChain(parentPluginHost.getPluginEntries()) {
        addAndMakeVisible(viewport);
        pluginListContent.reset(new juce::Component());
        viewport.setViewedComponent(pluginListContent.get(), false);
        viewport.setScrollBarsShown(true, false);

        addButton.setButtonText("Add Plugin");
        addButton.onClick = [this]() {
            showPluginMenu();
        };
        addAndMakeVisible(addButton);

        showPluginButton.setButtonText("Show Plugin");
        showPluginButton.onClick = [this]() {
            auto& selectedEntry = pluginChain[selectedIndex];
            this->showPluginContent(*selectedEntry);
        };
        addAndMakeVisible(showPluginButton);

        removeButton.setButtonText("Delete Plugin");
        removeButton.onClick = [this]() {
            pluginHost.removePlugin(selectedIndex);
            if (selectedIndex >= pluginChain.size()) {
                selectedIndex = pluginChain.size() - 1;
            } else if (selectedIndex - 1 >= 0) {
                selectedIndex--;
            }
            refreshList();
        };
        addAndMakeVisible(removeButton);

        moveUpButton.setButtonText("Move Up");
        addAndMakeVisible(moveUpButton);
        moveUpButton.onClick = [this]() {
            int newIndex = std::max(0.0, static_cast<double>(selectedIndex - 1));
            pluginHost.movePlugin(selectedIndex, newIndex);
            selectedIndex = newIndex;
            refreshList();
        };

        moveDownButton.setButtonText("Move Down");
        addAndMakeVisible(moveDownButton);
        moveDownButton.onClick = [this]() {
            int newIndex = std::min(static_cast<double>(pluginHost.getPluginEntries().size() - 1),
                static_cast<double>(selectedIndex + 1));
            pluginHost.movePlugin(selectedIndex, newIndex);
            selectedIndex = newIndex;
            refreshList();
        };

        refreshList();
    }

    void refreshList() {
        pluginListContent->removeAllChildren();
        for (size_t i = 0; i < pluginChain.size(); ++i) {
            int idx = static_cast<int>(i);

            auto& entry = pluginChain[i];
            auto* item = new PluginListItem(
                *entry,
                [this, i]() {
                    if (selectedIndex == static_cast<int>(i)) return;
                    selectedIndex = static_cast<int>(i);
                    this->refreshList();
                },
                [this, i](bool state) {
                    pluginHost.bypassPlugin(i, state);
                },
                entry->bypass,
                selectedIndex == idx);

            pluginListContent->addAndMakeVisible(item);
            item->setBounds(0, idx * this->itemHeight, this->itemWidth, this->itemHeight);
        }

        pluginListContent->resized();
        viewport.repaint();
        resized();
    }

    void paint(juce::Graphics& g) override { g.fillAll(static_cast<juce::Colour>(0xFF2a2a2a)); }

    void resized() override {
        auto area = getLocalBounds();
        auto controlArea = area.removeFromBottom(40).reduced(4);
        int btnWidth = controlArea.getWidth() / 5;

        addButton.setBounds(controlArea.removeFromLeft(btnWidth));
        showPluginButton.setBounds(controlArea.removeFromLeft(btnWidth));
        removeButton.setBounds(controlArea.removeFromLeft(btnWidth));
        moveUpButton.setBounds(controlArea.removeFromLeft(btnWidth));
        moveDownButton.setBounds(controlArea);

        int listHeight = static_cast<int>(pluginChain.size()) * itemHeight;

        pluginListContent->setSize(this->itemWidth, listHeight);

        auto viewportArea = area.removeFromLeft(this->itemWidth + 4);
        viewport.setBounds(viewportArea);
    }

   private:
    static constexpr int itemHeight = 34;
    static constexpr int itemWidth = 300;
    static constexpr int itemMargin = 4;
    static constexpr int viewportWidth = 300;
    static constexpr int viewportHeight = 400;

    int selectedIndex = -1;

    juce::Viewport viewport;
    PluginHost& pluginHost;
    std::vector<std::unique_ptr<PluginEntry>>& pluginChain;
    std::map<int, juce::PluginDescription> vstPluginMap;
    std::unique_ptr<juce::Component> pluginListContent;
    std::unique_ptr<PluginEditorWindow> pluginEditorWindow;
    juce::TextButton addButton, showPluginButton, removeButton, moveUpButton, moveDownButton;

    void showPluginMenu() {
        vstPluginMap.clear();

        juce::PopupMenu menu;
        juce::PopupMenu vstPluginsMenu;

        auto& pluginsList = pluginHost.getLoadedPluginList().getTypes();

        for (int i = 0; i < pluginsList.size(); ++i) {
            vstPluginsMenu.addItem(i + 1, pluginsList[i].descriptiveName);
            vstPluginMap[i + 1] = pluginsList[i];
        }

        menu.addSubMenu("VST Plugins", vstPluginsMenu);

        menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(addButton),
            [this](int result) {
                if (result == 0) {
                    return;
                }

                auto it = vstPluginMap.find(result);
                if (it != vstPluginMap.end()) {
                    const juce::PluginDescription& desc = it->second;
                    pluginHost.addPlugin(desc, -1);
                    refreshList();
                }
            });
    }

    void showPluginContent(PluginEntry& entry) {
        if (!entry.node) {
            std::cerr << "Cannot open plugin entry: its node doesn't exist" << std::endl;
            return;
        }

        if (pluginEditorWindow) {
            // If the current plugin is reopened, its window must be destroyed and freed
            // Probably not the best way to do this, but it works so...
            pluginEditorWindow = nullptr;
        }

        auto editor = entry.node->getProcessor()->createEditor();
        if (editor) {
            pluginEditorWindow = std::make_unique<PluginEditorWindow>(editor);
            pluginEditorWindow->setSize(600, 300);
            pluginEditorWindow->setOpaque(true);
            pluginEditorWindow->setVisible(true);
        } else {
            std::cout << "Plugin doesn't have an editor" << std::endl;
        }
    }
};