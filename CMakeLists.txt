cmake_minimum_required(VERSION 3.22)
project(Eclipse VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

# Only auto-copy into the system plugin folder on a local build, not on CI.
if(DEFINED ENV{CI})
    set(ECLIPSE_COPY FALSE)
else()
    set(ECLIPSE_COPY TRUE)
endif()

juce_add_plugin(Eclipse
    COMPANY_NAME "Eclipse Audio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    PLUGIN_MANUFACTURER_CODE Ecla
    PLUGIN_CODE Ecps
    FORMATS VST3 Standalone
    VST3_CATEGORIES Fx Distortion
    PRODUCT_NAME "Eclipse"
    COPY_PLUGIN_AFTER_BUILD ${ECLIPSE_COPY})

target_sources(Eclipse PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(Eclipse PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Eclipse
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
