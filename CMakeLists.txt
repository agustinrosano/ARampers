cmake_minimum_required(VERSION 3.22)

project(GoldPedal VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
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
    PRODUCT_NAME "ADR-AMPER"
    PLUGIN_MANUFACTURER_CODE GldM
    PLUGIN_CODE Gpdl
)

juce_generate_juce_header(GoldPedal)

target_sources(GoldPedal PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/DSP/GainModule.cpp
    Source/DSP/NAMModule.cpp
    Source/DSP/NoiseGateModule.cpp
    Source/DSP/IRModule.cpp
    Source/DSP/EQModule.cpp
    Source/DSP/PedalboardProcessor.cpp
    Source/UI/GoldLookAndFeel.cpp
    Source/UI/MeterComponent.cpp
    Source/UI/ModelPanelComponent.cpp
    Source/UI/PedalModuleComponent.cpp
    Source/UI/PresetBarComponent.cpp
    Source/UI/PresetShelfComponent.cpp
    Source/UI/IRLoaderComponent.cpp
    external/NeuralAmpModelerCore/NAM/activations.cpp
    external/NeuralAmpModelerCore/NAM/container.cpp
    external/NeuralAmpModelerCore/NAM/conv1d.cpp
    external/NeuralAmpModelerCore/NAM/convnet.cpp
    external/NeuralAmpModelerCore/NAM/dsp.cpp
    external/NeuralAmpModelerCore/NAM/get_dsp.cpp
    external/NeuralAmpModelerCore/NAM/linear.cpp
    external/NeuralAmpModelerCore/NAM/lstm.cpp
    external/NeuralAmpModelerCore/NAM/ring_buffer.cpp
    external/NeuralAmpModelerCore/NAM/util.cpp
    external/NeuralAmpModelerCore/NAM/wavenet/model.cpp
    external/NeuralAmpModelerCore/NAM/wavenet/slimmable.cpp
    external/NeuralAmpModelerCore/NAM/wavenet/a2_fast.cpp
    external/NeuralAmpModelerCore/Dependencies/AudioDSPTools/dsp/dsp.cpp
    external/NeuralAmpModelerCore/Dependencies/AudioDSPTools/dsp/ImpulseResponse.cpp
    external/NeuralAmpModelerCore/Dependencies/AudioDSPTools/dsp/NoiseGate.cpp
    external/NeuralAmpModelerCore/Dependencies/AudioDSPTools/dsp/RecursiveLinearFilter.cpp
    external/NeuralAmpModelerCore/Dependencies/AudioDSPTools/dsp/wav.cpp
)

target_include_directories(GoldPedal PRIVATE
    Source
    Source/DSP
    Source/UI
    external/NeuralAmpModelerCore
    external/NeuralAmpModelerCore/Dependencies/eigen
    external/NeuralAmpModelerCore/Dependencies/nlohmann
    external/NeuralAmpModelerCore/Dependencies/AudioDSPTools
    external/NeuralAmpModelerCore/Dependencies/AudioDSPTools/Dependencies/eigen
)

target_compile_definitions(GoldPedal PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_ASIO=1
    NAM_ENABLE_A2_FAST=1
    NAM_SAMPLE_FLOAT=1
)

target_compile_options(GoldPedal PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>
)

target_link_libraries(GoldPedal PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_extra
)
