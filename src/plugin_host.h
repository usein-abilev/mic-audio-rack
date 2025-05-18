#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

struct PluginEntry {
    juce::String name;
    juce::AudioProcessorGraph::Node::Ptr node;
    std::unique_ptr<juce::AudioProcessorEditor> editor;
    bool bypass = false;
    bool external = false;
};

class PluginHost {
   public:
    PluginHost();

    juce::AudioProcessorGraph* getGraph();
    bool scanPlugins(const juce::File& pluginFile);
    void updateGraph();
    void setMonoInput(bool enabled);
    void setMasterGainDecibels(float decibels);
    bool isMonoInput() const;

    /**
     * Adds and initializes specified plugin to the audio processor graph
     */
    bool addPlugin(const juce::PluginDescription& desc, int position = -1);
    bool addPlugin(std::unique_ptr<juce::AudioProcessor> processor, int position = -1);
    bool removePlugin(int index);
    bool bypassPlugin(int index, bool bypass);

    juce::KnownPluginList& getLoadedPluginList() { return loadedPluginList; }
    std::vector<std::unique_ptr<PluginEntry>>& getPluginEntries() { return pluginEntries; }

   private:
    juce::KnownPluginList loadedPluginList;
    juce::AudioPluginFormatManager formatManager;
    std::unique_ptr<juce::AudioProcessorGraph> graph;
    std::vector<std::unique_ptr<PluginEntry>> pluginEntries;

    juce::AudioProcessorGraph::Node::Ptr inputNode;
    juce::AudioProcessorGraph::Node::Ptr outputNode;
    juce::AudioProcessorGraph::Node::Ptr masterGainNode;

    bool monoInput = false;
    void setupGraph();
    void connectPluginEntryToGraph(std::unique_ptr<PluginEntry> entry, int position);
};