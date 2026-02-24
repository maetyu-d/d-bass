cmake_minimum_required(VERSION 3.22)

project(DBassPlugin VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (JUCE_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${JUCE_DIR}")
endif()

if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/JUCE" JUCE)
elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" JUCE)
else()
    find_package(JUCE CONFIG QUIET)
    if (NOT JUCE_FOUND)
        message(FATAL_ERROR
            "JUCE not found. Set JUCE_DIR, install JUCE with CMake config, "
            "or place a JUCE checkout at ./JUCE or ../JUCE")
    endif()
endif()

juce_add_plugin(DBassPlugin
    COMPANY_NAME "Codex"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Cdx1
    PLUGIN_CODE Dbs1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "D-Bass"
)

target_sources(DBassPlugin
    PRIVATE
        Source/BassPluginProcessor.cpp
        Source/BassPluginProcessor.h
        Source/BassPluginEditor.cpp
        Source/BassPluginEditor.h
)

target_compile_definitions(DBassPlugin
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(DBassPlugin
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
