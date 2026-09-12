cmake_minimum_required(VERSION 3.20)

project(AudioEditor
    VERSION 1.0.0
    LANGUAGES C CXX
)

# ==================================================
# C++ Standard
# ==================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==================================================
# JUCE
# ==================================================

add_subdirectory(JUCE)

# ==================================================
# AudioEditor application
# ==================================================

juce_add_gui_app(AudioEditor
    PRODUCT_NAME "AudioEditor"
    COMPANY_NAME "AudioEditor"
    VERSION "1.0.0"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)

juce_generate_juce_header(AudioEditor)

# ==================================================
# Source files
# ==================================================

target_sources(AudioEditor
    PRIVATE

        # Application
        src/Main.cpp
        src/MainComponent.cpp
        src/MainComponent.h

        # Project system
        src/Parser.cpp
        src/Parser.h
        src/Project.h

        # Audio
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

        # Timeline
        src/TimelineComponent.cpp
        src/TimelineComponent.h
)

# ==================================================
# Include directories
# ==================================================

target_include_directories(AudioEditor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ==================================================
# JUCE libraries
# ==================================================

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

# ==================================================
# JUCE configuration
# ==================================================

target_compile_definitions(AudioEditor
    PRIVATE

        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

# ==================================================
# Windows application settings
# ==================================================

if (WIN32)

    set_target_properties(AudioEditor
        PROPERTIES

        WIN32_EXECUTABLE TRUE
    )

endif()

# ==================================================
# Output directories
#
# Makes the generated files easier to find.
# ==================================================

set_target_properties(AudioEditor
    PROPERTIES

    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_BINARY_DIR}/bin"

    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_BINARY_DIR}/bin/Debug"

    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_BINARY_DIR}/bin/Release"
)
