cmake_minimum_required(VERSION 3.22)

project(DarkPhaser VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures")
endif()

include(FetchContent)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.15
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(DarkPhaser
    PRODUCT_NAME                "Dark Phaser"
    COMPANY_NAME                "Rozan Audio"
    BUNDLE_ID                   "com.rozanaudio.darkphaser"
    PLUGIN_MANUFACTURER_CODE    Rzan
    PLUGIN_CODE                 Dkph
    FORMATS                     VST3 Standalone
    VST3_CATEGORIES             Fx Modulation
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE)

target_sources(DarkPhaser PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp)

target_compile_definitions(DarkPhaser PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(DarkPhaser PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
