cmake_minimum_required(VERSION 3.15)

project(MicAudioRack
    VERSION 0.1.0
    DESCRIPTION "Simple audio router for VST and custom plugins"
    LANGUAGES CXX
)

# JUCE setup
set(JUCE_DIR "D:/Tools/JUCE")
add_subdirectory(${JUCE_DIR} JUCE)

juce_add_gui_app(MicAudioRack
    VERSION 0.1.0
    COMPANY_NAME ""
    PRODUCT_NAME "MicAudioRack"
)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp src/*.h)
target_sources(MicAudioRack PRIVATE ${SOURCE_FILES})


target_compile_definitions(MicAudioRack PRIVATE
    JUCE_PLUGINHOST_VST3=1
     $<$<CONFIG:Debug>:DEBUG=1>
)
target_compile_features(MicAudioRack PRIVATE cxx_std_17)

target_link_libraries(MicAudioRack PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_gui_basics
)
