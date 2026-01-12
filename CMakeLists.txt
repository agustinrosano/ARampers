cmake_minimum_required(VERSION 3.22)

project(GoldPedal VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(external/JUCE)

juce_add_plugin(GoldPedal
    COMPANY_NAME "GOLDMUND"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    MICROPHONE_PERMISSION_ENABLED TRUE
    FORMATS VST3 Standalone
    PRODUCT_NAME "ACR-AMPER"
    PLUGIN_MANUFACTURER_CODE GldM
    PLUGIN_CODE Gpdl
)

juce_generate_juce_header(GoldPedal)

target_sources(GoldPedal PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/GainModule.cpp
    Source/DSP/NoiseGateModule.cpp
    Source/DSP/DriveModule.cpp
    Source/DSP/AmpModule.cpp
    Source/DSP/IRModule.cpp
    Source/DSP/EQModule.cpp
    Source/DSP/PedalboardProcessor.cpp
    Source/UI/PedalModuleComponent.cpp
    Source/UI/PresetBarComponent.cpp
    Source/UI/IRLoaderComponent.cpp
)

target_include_directories(GoldPedal PRIVATE
    Source
    Source/DSP
    Source/UI
)

target_compile_definitions(GoldPedal PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(GoldPedal PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
)
