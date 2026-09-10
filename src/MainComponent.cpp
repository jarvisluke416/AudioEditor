#include <JuceHeader.h>
#include "MainComponent.h"
#include "Project.h"
#include "Parser.h"
#include "ProjectRenderer.h"
#include "WavWriter.h"

// ==================================================
// Constructor
// ==================================================

MainComponent::MainComponent()
{
    setSize(900, 650);

    // ----------------------------------------------
    // Audio format support
    // ----------------------------------------------

    formatManager.registerBasicFormats();

    // ----------------------------------------------
    // Title
    // ----------------------------------------------

    titleLabel.setText(
        "AudioEditor",
        juce::dontSendNotification);

    titleLabel.setFont(
        juce::Font(
            28.0f,
            juce::Font::bold));

    titleLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    addAndMakeVisible(titleLabel);

    // ----------------------------------------------
    // Buttons
    // ----------------------------------------------

    openButton.setButtonText("Open File");
    playButton.setButtonText("Play");
    renderButton.setButtonText("Render WAV");

    addAndMakeVisible(openButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(renderButton);

    openButton.onClick = [this]
    {
        openProject();
    };

    playButton.onClick = [this]
    {
        playProject();
    };

    renderButton.onClick = [this]
    {
        renderProject();
    };

    // ----------------------------------------------
    // Tempo / tracks
    // ----------------------------------------------

    tempoLabel.setText(
        "Tempo: -- BPM",
        juce::dontSendNotification);

    tracksLabel.setText(
        "Tracks: --",
        juce::dontSendNotification);

    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(tracksLabel);

    // ----------------------------------------------
    // Editable song text
    // ----------------------------------------------

    projectInfo.setMultiLine(true);
    projectInfo.setReadOnly(false);
    projectInfo.setScrollbarsShown(true);
    projectInfo.setReturnKeyStartsNewLine(true);
    projectInfo.setFont(juce::Font(16.0f));

    projectInfo.setColour(
        juce::TextEditor::backgroundColourId,
        juce::Colour::fromRGB(20, 20, 24));

    projectInfo.setColour(
        juce::TextEditor::textColourId,
        juce::Colours::white);

    projectInfo.setColour(
        juce::TextEditor::outlineColourId,
        juce::Colour::fromRGB(70, 70, 75));

    projectInfo.setText(
        "No file loaded.\n\n"
        "Click \"Open File\" to load a .song, .wav, .mp3, or other media file.",
        false);

    addAndMakeVisible(projectInfo);

    // ----------------------------------------------
    // Status
    // ----------------------------------------------

    statusLabel.setText(
        "AudioEditor is ready.",
        juce::dontSendNotification);

    statusLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::lightgrey);

    addAndMakeVisible(statusLabel);

    // ----------------------------------------------
    // Start audio device
    // ----------------------------------------------

    setAudioChannels(0, 2);
}

// ==================================================
// Destructor
// ==================================================

MainComponent::~MainComponent()
{
    transportSource.stop();
    transportSource.setSource(nullptr);

    readerSource.reset();

    shutdownAudio();
}

// ==================================================
// Open File
// ==================================================

void MainComponent::openProject()
{
    /*
        We now allow:

        .song
        .wav
        .mp3
        .mp4
        .m4a
        .aiff
        .flac
        .ogg
        .wma

        JUCE's format manager will determine whether
        it can actually decode the selected media file.

        .song files are handled by our Parser.
        Media files are loaded directly into the
        AudioTransportSource.
    */

    fileChooser =
        std::make_unique<juce::FileChooser>(
            "Open AudioEditor File",
            juce::File{},
            "*.song;*.wav;*.mp3;*.mp4;*.m4a;*.aiff;*.aif;*.flac;*.ogg;*.wma");

    statusLabel.setText(
        "Select a project or audio file...",
        juce::dontSendNotification);

    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser)
        {
            const juce::File result =
                chooser.getResult();

            if (!result.existsAsFile())
            {
                statusLabel.setText(
                    "Open cancelled.",
                    juce::dontSendNotification);

                fileChooser.reset();
                return;
            }

            const juce::String extension =
                result.getFileExtension()
                    .toLowerCase();

            // ------------------------------------------
            // .song project
            // ------------------------------------------

            if (extension == ".song")
            {
                currentProject = result;

                try
                {
                    Project project =
                        Parser::parseFile(
                            currentProject
                                .getFullPathName()
                                .toStdString());

                    const juce::String sourceText =
                        currentProject.loadFileAsString();

                    projectInfo.setText(
                        sourceText,
                        false);

                    tempoLabel.setText(
                        "Tempo: " +
                        juce::String(project.tempo, 1) +
                        " BPM",
                        juce::dontSendNotification);

                    tracksLabel.setText(
                        "Tracks: " +
                        juce::String(
                            static_cast<int>(
                                project.tracks.size())),
                        juce::dontSendNotification);

                    statusLabel.setText(
                        "Project loaded. Press Play to render and play it.",
                        juce::dontSendNotification);
                }
                catch (const std::exception& e)
                {
                    projectInfo.setText(
                        "Could not load project:\n\n" +
                        juce::String(e.what()),
                        false);

                    statusLabel.setText(
                        "Error loading project.",
                        juce::dontSendNotification);
                }

                fileChooser.reset();
                return;
            }

            // ------------------------------------------
            // Direct media file
            // ------------------------------------------

            auto reader =
                std::unique_ptr<juce::AudioFormatReader>(
                    formatManager.createReaderFor(result));

            if (reader == nullptr)
            {
                statusLabel.setText(
                    "AudioEditor could not open this media file.",
                    juce::dontSendNotification);

                fileChooser.reset();
                return;
            }

            // Stop anything currently playing.
            transportSource.stop();
            transportSource.setSource(nullptr);
            readerSource.reset();

            const double readerSampleRate =
                reader->sampleRate;

            const juce::int64 lengthInSamples =
                reader->lengthInSamples;

            readerSource =
                std::make_unique<
                    juce::AudioFormatReaderSource>(
                        reader.release(),
                        true);

            transportSource.setSource(
                readerSource.get(),
                0,
                nullptr,
                readerSampleRate);

            transportSource.setPosition(0.0);

            // This file is not a .song project.
            currentProject = juce::File{};

            // Display useful information.
            projectInfo.setText(
                "Media file loaded:\n\n" +
                result.getFileName() +
                "\n\n"
                "Path:\n" +
                result.getFullPathName(),
                false);

            tempoLabel.setText(
                "Sample Rate: " +
                juce::String(
                    readerSampleRate,
                    0) +
                " Hz",
                juce::dontSendNotification);

            tracksLabel.setText(
                "Channels: " +
                juce::String(
                    readerSource->getAudioFormatReader()
                        ->numChannels),
                juce::dontSendNotification);

            statusLabel.setText(
                "Media loaded. Press Play.",
                juce::dontSendNotification);

            fileChooser.reset();
        });
}

// ==================================================
// Save editor contents back to .song
// ==================================================

bool MainComponent::saveEditorToProject()
{
    if (!currentProject.existsAsFile())
        return false;

    const juce::String text =
        projectInfo.getText();

    return currentProject.replaceWithText(text);
}

// ==================================================
// Play
// ==================================================

void MainComponent::playProject()
{
    // =================================================
    // CASE 1:
    // A normal WAV/MP3/MP4/etc. is already loaded.
    // =================================================

    if (!currentProject.existsAsFile())
    {
        if (readerSource != nullptr)
        {
            transportSource.setPosition(0.0);
            transportSource.start();

            statusLabel.setText(
                "Playing media...",
                juce::dontSendNotification);

            return;
        }

        statusLabel.setText(
            "Open a .song, .wav, .mp3, or other supported media file first.",
            juce::dontSendNotification);

        return;
    }

    // =================================================
    // CASE 2:
    // A .song project is loaded.
    // =================================================

    try
    {
        // ------------------------------------------
        // Save edited text.
        // ------------------------------------------

        if (!saveEditorToProject())
        {
            statusLabel.setText(
                "Could not save project.",
                juce::dontSendNotification);

            return;
        }

        // ------------------------------------------
        // Parse edited project.
        // ------------------------------------------

        Project project =
            Parser::parseFile(
                currentProject
                    .getFullPathName()
                    .toStdString());

        // ------------------------------------------
        // Stop existing playback.
        // ------------------------------------------

        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();

        // ------------------------------------------
        // Render project.
        // ------------------------------------------

        constexpr int sampleRate = 48000;

        EditorAudioBuffer rendered =
            ProjectRenderer::render(
                project,
                sampleRate);

        // ------------------------------------------
        // Create temporary WAV.
        // ------------------------------------------

        const juce::File playbackFile =
            currentProject
                .getParentDirectory()
                .getChildFile(
                    "_audioeditor_playback.wav");

        if (!WavWriter::write(
                playbackFile
                    .getFullPathName()
                    .toStdString(),
                rendered))
        {
            statusLabel.setText(
                "Could not create playback audio.",
                juce::dontSendNotification);

            return;
        }

        // ------------------------------------------
        // Open rendered WAV.
        // ------------------------------------------

        auto reader =
            std::unique_ptr<juce::AudioFormatReader>(
                formatManager.createReaderFor(
                    playbackFile));

        if (reader == nullptr)
        {
            statusLabel.setText(
                "Could not open rendered playback audio.",
                juce::dontSendNotification);

            return;
        }

        const double readerSampleRate =
            reader->sampleRate;

        readerSource =
            std::make_unique<
                juce::AudioFormatReaderSource>(
                    reader.release(),
                    true);

        // ------------------------------------------
        // Connect reader to transport.
        // ------------------------------------------

        transportSource.setSource(
            readerSource.get(),
            0,
            nullptr,
            readerSampleRate);

        // ------------------------------------------
        // Start playback.
        // ------------------------------------------

        transportSource.setPosition(0.0);
        transportSource.start();

        statusLabel.setText(
            "Playing project...",
            juce::dontSendNotification);
    }
    catch (const std::exception& e)
    {
        statusLabel.setText(
            "Play error: " +
            juce::String(e.what()),
            juce::dontSendNotification);
    }
}

// ==================================================
// Render Project
// ==================================================

void MainComponent::renderProject()
{
    if (!currentProject.existsAsFile())
    {
        statusLabel.setText(
            "Render WAV is available for .song projects.",
            juce::dontSendNotification);

        return;
    }

    try
    {
        // ------------------------------------------
        // Save current text editor contents.
        // ------------------------------------------

        if (!saveEditorToProject())
        {
            statusLabel.setText(
                "Could not save project.",
                juce::dontSendNotification);

            return;
        }

        statusLabel.setText(
            "Rendering project...",
            juce::dontSendNotification);

        // ------------------------------------------
        // Parse project.
        // ------------------------------------------

        Project project =
            Parser::parseFile(
                currentProject
                    .getFullPathName()
                    .toStdString());

        // ------------------------------------------
        // Render.
        // ------------------------------------------

        constexpr int sampleRate = 48000;

        EditorAudioBuffer rendered =
            ProjectRenderer::render(
                project,
                sampleRate);

        // ------------------------------------------
        // Write WAV.
        // ------------------------------------------

        const juce::File outputFile =
            currentProject
                .getParentDirectory()
                .getChildFile(
                    "render.wav");

        const bool success =
            WavWriter::write(
                outputFile
                    .getFullPathName()
                    .toStdString(),
                rendered);

        if (!success)
        {
            statusLabel.setText(
                "Failed to write render.wav.",
                juce::dontSendNotification);

            return;
        }

        statusLabel.setText(
            "Render complete: " +
            outputFile.getFileName(),
            juce::dontSendNotification);
    }
    catch (const std::exception& e)
    {
        statusLabel.setText(
            "Render error: " +
            juce::String(e.what()),
            juce::dontSendNotification);
    }
}

// ==================================================
// Audio preparation
// ==================================================

void MainComponent::prepareToPlay(
    int samplesPerBlockExpected,
    double sampleRate)
{
    transportSource.prepareToPlay(
        samplesPerBlockExpected,
        sampleRate);
}

// ==================================================
// Audio callback
// ==================================================

void MainComponent::getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (transportSource.isPlaying())
    {
        transportSource.getNextAudioBlock(
            bufferToFill);
    }
    else
    {
        bufferToFill.clearActiveBufferRegion();
    }
}

// ==================================================
// Release audio resources
// ==================================================

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

// ==================================================
// Paint
// ==================================================

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(
        juce::Colour::fromRGB(
            30, 30, 34));
}

// ==================================================
// Layout
// ==================================================

void MainComponent::resized()
{
    auto area =
        getLocalBounds().reduced(25);

    // ----------------------------------------------
    // Title
    // ----------------------------------------------

    titleLabel.setBounds(
        area.removeFromTop(45));

    // ----------------------------------------------
    // Buttons
    // ----------------------------------------------

    auto buttonArea =
        area.removeFromTop(50);

    openButton.setBounds(
        buttonArea
            .removeFromLeft(150)
            .reduced(5));

    playButton.setBounds(
        buttonArea
            .removeFromLeft(120)
            .reduced(5));

    renderButton.setBounds(
        buttonArea
            .removeFromLeft(150)
            .reduced(5));

    // ----------------------------------------------
    // Tempo / tracks
    // ----------------------------------------------

    auto infoArea =
        area.removeFromTop(35);

    tempoLabel.setBounds(
        infoArea
            .removeFromLeft(180));

    tracksLabel.setBounds(
        infoArea
            .removeFromLeft(180));

    // ----------------------------------------------
    // Text editor
    // ----------------------------------------------

    projectInfo.setBounds(
        area.reduced(5));

    // ----------------------------------------------
    // Status
    // ----------------------------------------------

    statusLabel.setBounds(
        getLocalBounds()
            .reduced(25)
            .removeFromBottom(30));
}
