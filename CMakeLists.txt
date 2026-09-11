cmake_minimum_required(VERSION 3.20)

project(AudioEditor
    VERSION 1.0
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# JUCE
add_subdirectory(JUCE)

juce_add_gui_app(AudioEditor
    PRODUCT_NAME "AudioEditor"
    COMPANY_NAME "AudioEditor"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)

juce_generate_juce_header(AudioEditor)

target_sources(AudioEditor
    PRIVATE
        src/Main.cpp
        src/MainComponent.cpp
        src/MainComponent.h

        src/Parser.cpp
        src/Parser.h
        src/Project.h

        src/WavWriter.cpp
        src/WavWriter.h

        src/SimpleSynth.cpp
        src/SimpleSynth.h

        src/AudioFile.cpp
        src/AudioFile.h

        src/Mixer.cpp
        src/Mixer.h

        src/AudioBuffer.h

        src/ProjectRenderer.cpp
        src/ProjectRenderer.h

        src/TimelineComponent.cpp
        src/TimelineComponent.h
)

target_include_directories(AudioEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(AudioEditor
    PRIVATE
        juce::juce_gui_extra
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_core
)

target_compile_definitions(AudioEditor
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
