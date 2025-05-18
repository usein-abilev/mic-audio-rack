#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "plugin_host.h"
#include "ui/plugin_window.h"
#include "ui/plugin_chain.h"

class MainComponent : public juce::Component,
                      private juce::ComboBox::Listener,
                      private juce::Button::Listener {
   public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

   private:
    juce::AudioDeviceManager deviceManager;
    juce::AudioProcessorPlayer audioPlayer;
    std::unique_ptr<PluginHost> pluginHost;
    std::unique_ptr<PluginChainUI> pluginChainUI;

    juce::ComboBox inputDeviceBox;
    juce::ComboBox outputDeviceBox;
    juce::ToggleButton monoToggle;
    juce::Slider gainSlider;

    void comboBoxChanged(juce::ComboBox* changedBox) override;
    void buttonClicked(juce::Button* button) override;
    void updateAudioDevice();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};