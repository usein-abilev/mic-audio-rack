#include "plugin_host.h"

#include "processors/gain_processor.h"

PluginHost::PluginHost() {
    formatManager.addDefaultFormats();
    graph = std::make_unique<juce::AudioProcessorGraph>();
    setupGraph();
    updateGraph();

    std::cout << "PluginHost: Constructor called" << std::endl;
}

void PluginHost::setupGraph() {
    graph->clear();
    graph->enableAllBuses();

    inputNode = graph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode));

    outputNode = graph->addNode(std::make_unique<juce::AudioProcessorGraph::AudioGraphIOProcessor>(
        juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode));

    masterGainNode = graph->addNode(std::make_unique<GainProcessor>());
}

bool PluginHost::scanPlugins(const juce::File& directory) {
    juce::File dummyPedalFile =
        juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("deadman.xml");

    for (int i = 0; i < formatManager.getNumFormats(); ++i) {
        juce::AudioPluginFormat* format = formatManager.getFormat(i);
        juce::PluginDirectoryScanner scanner(loadedPluginList,
            *format,
            juce::FileSearchPath(directory.getFullPathName()),
            true,
            dummyPedalFile,
            false);
        juce::String errorMessage;
        juce::String nameOfPluginBeingScanned;
        juce::StringArray failedFiles;

        while (scanner.scanNextFile(false, nameOfPluginBeingScanned)) {
            juce::String pluginName = nameOfPluginBeingScanned;
            std::cout << "Found plugin: " << pluginName << std::endl;
        }
    }

    return true;
}

bool PluginHost::addPlugin(const juce::PluginDescription& desc, int position) {
    juce::String error;
    auto plugin = formatManager.createPluginInstance(desc, 44100.0, 512, error);

    if (!plugin || error.isNotEmpty()) {
        DBG("Failed to instantiate plugin: " + error);
        std::cerr << "Failed to instantiate plugin: " << error << std::endl;
        return false;
    }

    plugin->enableAllBuses();
    plugin->setPlayConfigDetails(2, 2, 44100.0, 512);

    auto pluginNode = graph->addNode(std::move(plugin));
    auto entry = std::make_unique<PluginEntry>();
    entry->name = desc.descriptiveName;
    entry->node = pluginNode;
    entry->editor = nullptr;
    entry->bypass = false;
    entry->external = true;
    connectPluginEntryToGraph(std::move(entry), position);

    return true;
}

bool PluginHost::addPlugin(std::unique_ptr<juce::AudioProcessor> processor, int position) {
    auto name = processor->getName();
    auto pluginNode = graph->addNode(std::move(processor));

    auto entry = std::make_unique<PluginEntry>();
    entry->name = name;
    entry->node = pluginNode;
    entry->bypass = false;
    connectPluginEntryToGraph(std::move(entry), position);

    return true;
}

bool PluginHost::removePlugin(int index) {
    if (index < 0 || index >= pluginEntries.size()) {
        return false;
    }

    pluginEntries.erase(pluginEntries.begin() + index);
    updateGraph();

    return true;
}

bool PluginHost::movePlugin(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= pluginEntries.size() || toIndex < 0 ||
        toIndex >= pluginEntries.size())
        return false;

    auto entry = std::move(pluginEntries[fromIndex]);
    pluginEntries.erase(pluginEntries.begin() + fromIndex);
    pluginEntries.insert(pluginEntries.begin() + toIndex, std::move(entry));
    updateGraph();

    return true;
}

bool PluginHost::bypassPlugin(int index, bool bypass) {
    if (index < 0 || index >= pluginEntries.size()) {
        return false;
    }

    pluginEntries.at(index)->bypass = bypass;
    updateGraph();

    return true;
}

void PluginHost::connectPluginEntryToGraph(std::unique_ptr<PluginEntry> entry, int position) {
    auto entryName = entry->name;

    if (position < 0 || position >= pluginEntries.size()) {
        pluginEntries.push_back(std::move(entry));
    } else {
        pluginEntries.insert(pluginEntries.begin() + position, std::move(entry));
    }

    updateGraph();
    std::cout << "Plugin connected to audio processor graph: " << entryName << std::endl;
}

void PluginHost::setMonoInput(bool enabled) {
    if (monoInput != enabled) {
        monoInput = enabled;
        updateGraph();
    }
}

bool PluginHost::isMonoInput() const { return monoInput; }

void PluginHost::setMasterGainDecibels(float decibels) {
    if (auto gainProcessor = dynamic_cast<GainProcessor*>(masterGainNode->getProcessor())) {
        gainProcessor->setGainDecibels(decibels);
    }
}

void PluginHost::updateGraph() {
    std::cout << "Updating plugin graph. Mono: " << (monoInput ? "true" : "false") << std::endl;
    std::cout << "Plugins count in bus: " << pluginEntries.size() << std::endl;

    auto prevConnections = graph->getConnections();
    for (auto& conn : prevConnections) {
        graph->removeConnection(conn);
    }

    auto lastNodeId = inputNode->nodeID;
    for (auto& entry : pluginEntries) {
        if (!entry->node || entry->bypass) continue;
        if (lastNodeId == inputNode->nodeID && monoInput) {
            graph->addConnection({{inputNode->nodeID, 0}, {entry->node->nodeID, 0}});
            graph->addConnection({{inputNode->nodeID, 0}, {entry->node->nodeID, 1}});
        } else {
            graph->addConnection({{lastNodeId, 0}, {entry->node->nodeID, 0}});
            graph->addConnection({{lastNodeId, 1}, {entry->node->nodeID, 1}});
        }
        lastNodeId = entry->node->nodeID;
    }

    // in case when all plugins are in bypass (or plugins list is empty)
    if (lastNodeId == inputNode->nodeID) {
        graph->addConnection({{inputNode->nodeID, 0}, {masterGainNode->nodeID, 0}});
        graph->addConnection({{inputNode->nodeID, monoInput ? 0 : 1}, {masterGainNode->nodeID, 1}});
    } else {
        graph->addConnection({{lastNodeId, 0}, {masterGainNode->nodeID, 0}});
        graph->addConnection({{lastNodeId, 1}, {masterGainNode->nodeID, 1}});
    }

    graph->addConnection({{masterGainNode->nodeID, 0}, {outputNode->nodeID, 0}});
    graph->addConnection({{masterGainNode->nodeID, 1}, {outputNode->nodeID, 1}});

    for (auto& connection : graph->getConnections()) {
        std::cout << "Connection: " << connection.source.nodeID.uid << " -> "
                  << connection.destination.nodeID.uid << std::endl;
    }
}

juce::AudioProcessorGraph* PluginHost::getGraph() { return graph.get(); }