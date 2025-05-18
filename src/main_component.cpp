#include "main_component.h"

#include "processors/gain_processor.h"
#include "ui/plugin_window.h"
// C:\Program Files\VstPlugins\SoundToys\EchoBoy.dll

MainComponent::MainComponent() {
    setSize(800, 600);

    auto initialiseError = deviceManager.initialise(2, 2, nullptr, true);

    if (initialiseError.isNotEmpty()) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Audio Device Error",
            initialiseError);
    }

    deviceManager.addAudioCallback(&audioPlayer);

    // initialize the plugin host (plugin chain manager)
    this->pluginHost = std::make_unique<PluginHost>();
    audioPlayer.setProcessor(this->pluginHost->getGraph());

    // Create UI and plugin entries

    auto& loadedPluginList = pluginHost->getLoadedPluginList();
    if (loadedPluginList.getNumTypes() == 0) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Plugin Scan Error",
            "No plugins found.");
        std::cout << "No plugins found." << std::endl;
        return;
    }

    auto pluginsDescriptions = loadedPluginList.getTypes();
    for (const auto& pluginDesc : pluginsDescriptions) {
        std::cout << "Plugin found: " << pluginDesc.descriptiveName
                  << "; Plugin number:" << pluginDesc.uniqueId << std::endl;
    }

    // TEST VST3 ADD
    pluginHost->addPlugin(*std::find_if(pluginsDescriptions.begin(),
        pluginsDescriptions.end(),
        [](const juce::PluginDescription& desc) {
            return desc.descriptiveName == "Pro-Q 3";
        }));

    pluginHost->addPlugin(*std::find_if(pluginsDescriptions.begin(),
        pluginsDescriptions.end(),
        [](const juce::PluginDescription& desc) {
            return desc.descriptiveName == "ValhallaDelay";
        }));

    pluginHost->updateGraph();

    // Set up the UI components
    pluginChainUI = std::make_unique<PluginChainUI>(*pluginHost);
    addAndMakeVisible(pluginChainUI.get());

    inputDeviceBox.setTextWhenNothingSelected("Select input device");
    outputDeviceBox.setTextWhenNothingSelected("Select output device");
    monoToggle.setButtonText("Force mono");

    gainSlider.setRange(0.0, 2.0, 0.01);
    gainSlider.setValue(1.0);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
    gainSlider.onValueChange = [this]() {
        auto gain = gainSlider.getValue();
        pluginHost->setMasterGainDecibels(juce::Decibels::gainToDecibels(gain));
    };

    addAndMakeVisible(inputDeviceBox);
    addAndMakeVisible(outputDeviceBox);
    addAndMakeVisible(monoToggle);
    addAndMakeVisible(gainSlider);

    inputDeviceBox.addListener(this);
    outputDeviceBox.addListener(this);
    monoToggle.addListener(this);

    // select first audio backend
    auto* type = deviceManager.getAvailableDeviceTypes()[0];
    type->scanForDevices();

    auto inputDevices = type->getDeviceNames(true);
    auto outputDevices = type->getDeviceNames(false);

    for (int i = 0; i < inputDevices.size(); ++i) inputDeviceBox.addItem(inputDevices[i], i + 1);

    for (int i = 0; i < outputDevices.size(); ++i) outputDeviceBox.addItem(outputDevices[i], i + 1);

    updateAudioDevice();
}

MainComponent::~MainComponent() {
    std::cout << "Destructor called" << std::endl;
    audioPlayer.setProcessor(nullptr);
    deviceManager.removeAudioCallback(&audioPlayer);
    deviceManager.closeAudioDevice();
}

void MainComponent::paint(juce::Graphics& g) { g.fillAll(juce::Colours::darkgrey); }

void MainComponent::resized() {
    inputDeviceBox.setBounds(20, 20, getWidth() - 40, 30);
    outputDeviceBox.setBounds(20, 60, getWidth() - 40, 30);
    monoToggle.setBounds(20, 100, getWidth() - 40, 30);
    gainSlider.setBounds(20, 140, getWidth() - 40, 30);

    if (pluginChainUI) {
        auto area = getLocalBounds();
        area.setY(200);
        area.setHeight(getHeight() - 200);
        area.reduce(20, 20);
        pluginChainUI->setBounds(area);
    }
}

void MainComponent::comboBoxChanged(juce::ComboBox* box) {
    if (box == &inputDeviceBox || box == &outputDeviceBox) updateAudioDevice();
}

void MainComponent::buttonClicked(juce::Button* button) {
    if (button == &monoToggle) {
        pluginHost->setMonoInput(monoToggle.getToggleState());
    }
}

void MainComponent::updateAudioDevice() {
    auto config = deviceManager.getAudioDeviceSetup();
    auto* type = deviceManager.getAvailableDeviceTypes()[0];
    type->scanForDevices();

    auto inputDevices = type->getDeviceNames(true);
    auto outputDevices = type->getDeviceNames(false);

    const int inputIndex = inputDeviceBox.getSelectedItemIndex();
    const int outputIndex = outputDeviceBox.getSelectedItemIndex();

    config.inputDeviceName = (inputIndex >= 0) ? inputDevices[inputIndex] : juce::String();
    config.outputDeviceName = (outputIndex >= 0) ? outputDevices[outputIndex] : juce::String();

    config.useDefaultInputChannels = true;
    config.useDefaultOutputChannels = true;

    auto result = deviceManager.setAudioDeviceSetup(config, true);
    if (result.isNotEmpty()) {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
            "Audio Device Error",
            result);
    }
}
