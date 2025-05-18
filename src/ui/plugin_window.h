#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

class PluginEditorWindow : public juce::DocumentWindow {
   public:
    PluginEditorWindow(juce::AudioProcessorEditor* pluginEditor)
        : juce::DocumentWindow(
              "Plugin Editor", juce::Colours::darkgrey, juce::DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        setResizable(true, false);
        setSize(800, 600);
        setContentOwned(pluginEditor, true);

        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    ~PluginEditorWindow() override {
        std::cout << "PluginEditorWindow destructor called" << std::endl;
    }

    void setEditor(juce::AudioProcessorEditor* newEditor) {
        setContentOwned(newEditor, true);  // takes ownership and deletes the old one
        centreWithSize(newEditor->getWidth(), newEditor->getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override { setVisible(false); }
};
