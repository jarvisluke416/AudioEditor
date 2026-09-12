#include <JuceHeader.h>
#include "MainComponent.h"
#include "Project.h"
#include "Parser.h"
#include "ProjectRenderer.h"
#include "WavWriter.h"
#include <stdexcept>
#include <algorithm>

// ==================================================
// Forward declarations
// ==================================================

static juce::String cleanEditorProjectText(
    const juce::String& originalText);

static Project parseEditorProject(
    const juce::String& editorText);

// ==================================================
// Constructor
// ==================================================

MainComponent::MainComponent()
{
    setSize(1000, 750);

    // ==================================================
    // Audio
    // ==================================================

    formatManager.registerBasicFormats();

    // ==================================================
    // Title
    // ==================================================

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

    // ==================================================
    // Main buttons
    // ==================================================

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

    // ==================================================
    // Instrument menu
    // ==================================================

    instrumentLabel.setText(
        "Instrument:",
        juce::dontSendNotification);

    addAndMakeVisible(instrumentLabel);
    addAndMakeVisible(instrumentBox);
    addAndMakeVisible(addNoteButton);

    instrumentBox.addItem("Guitar", 1);
    instrumentBox.addItem("Electric Guitar", 2);
    instrumentBox.addItem("Piano", 3);
    instrumentBox.addItem("Electric Piano", 4);
    instrumentBox.addItem("Bass", 5);
    instrumentBox.addItem("Organ", 6);
    instrumentBox.addItem("Synth Lead", 7);
    instrumentBox.addItem("Synth Pad", 8);
    instrumentBox.addItem("Strings", 9);
    instrumentBox.addItem("Flute", 10);
    instrumentBox.addItem("Brass", 11);
    instrumentBox.addItem("Bell", 12);

    instrumentBox.setSelectedId(
        1,
        juce::dontSendNotification);

    addNoteButton.setButtonText(
        "Add Instrument");

    addNoteButton.onClick = [this]
    {
        addInstrumentNote();
    };

    // ==================================================
    // Drum menu
    // ==================================================

    drumLabel.setText(
        "Drum:",
        juce::dontSendNotification);

    addAndMakeVisible(drumLabel);
    addAndMakeVisible(drumBox);
    addAndMakeVisible(addDrumButton);

    drumBox.addItem("Kick", 1);
    drumBox.addItem("Snare", 2);
    drumBox.addItem("Hi-Hat", 3);
    drumBox.addItem("Open Hi-Hat", 4);
    drumBox.addItem("Clap", 5);
    drumBox.addItem("Rimshot", 6);
    drumBox.addItem("Tom", 7);
    drumBox.addItem("Low Tom", 8);
    drumBox.addItem("Mid Tom", 9);
    drumBox.addItem("High Tom", 10);
    drumBox.addItem("Crash", 11);
    drumBox.addItem("Ride", 12);
    drumBox.addItem("Cowbell", 13);
    drumBox.addItem("Tambourine", 14);
    drumBox.addItem("Shaker", 15);

    drumBox.setSelectedId(
        1,
        juce::dontSendNotification);

    addDrumButton.setButtonText(
        "Add Drum");

    addDrumButton.onClick = [this]
    {
        addDrumEvent();
    };

    // ==================================================
    // Note menu
    // ==================================================

    noteLabel.setText(
        "Note:",
        juce::dontSendNotification);

    addAndMakeVisible(noteLabel);
    addAndMakeVisible(noteBox);

    const char* noteNames[] =
    {
        "C",
        "C#",
        "D",
        "D#",
        "E",
        "F",
        "F#",
        "G",
        "G#",
        "A",
        "A#",
        "B"
    };

    int noteId = 1;

    for (int octave = 2;
         octave <= 6;
         ++octave)
    {
        for (const char* note : noteNames)
        {
            noteBox.addItem(
                juce::String(note) +
                juce::String(octave),
                noteId++);
        }
    }

    noteBox.setSelectedId(
        25,
        juce::dontSendNotification);

    // ==================================================
    // Info
    // ==================================================

    tempoLabel.setText(
        "Tempo: -- BPM",
        juce::dontSendNotification);

    tracksLabel.setText(
        "Tracks: --",
        juce::dontSendNotification);

    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(tracksLabel);

    // ==================================================
    // Project editor
    // ==================================================

    projectInfo.setMultiLine(true);
    projectInfo.setReadOnly(false);
    projectInfo.setScrollbarsShown(true);
    projectInfo.setReturnKeyStartsNewLine(true);

    projectInfo.setFont(
        juce::Font(16.0f));

    projectInfo.setColour(
        juce::TextEditor::backgroundColourId,
        juce::Colour::fromRGB(
            20,
            20,
            24));

    projectInfo.setColour(
        juce::TextEditor::textColourId,
        juce::Colours::white);

    projectInfo.setColour(
        juce::TextEditor::outlineColourId,
        juce::Colour::fromRGB(
            70,
            70,
            75));

    projectInfo.setText(
        "No project loaded.\n\n"
        "You can open a .song file or "
        "paste/type a project directly "
        "into this editor.",
        false);

    addAndMakeVisible(projectInfo);

    // ==================================================
    // Status
    // ==================================================

    statusLabel.setText(
        "AudioEditor is ready.",
        juce::dontSendNotification);

    statusLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::lightgrey);

    addAndMakeVisible(statusLabel);

    // ==================================================
    // Start audio
    // ==================================================

    setAudioChannels(
        0,
        2);
}

// ==================================================
// Destructor
// ==================================================

MainComponent::~MainComponent()
{
    stopPlayback();
    shutdownAudio();
}

// ==================================================
// Stop playback
// ==================================================

void MainComponent::stopPlayback()
{
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();
}

// ==================================================
// Project loaded?
// ==================================================

bool MainComponent::isProjectLoaded() const
{
    return currentProject.existsAsFile();
}

// ==================================================
// Audio file loaded?
// ==================================================

bool MainComponent::isAudioFileLoaded() const
{
    return currentAudioFile.existsAsFile();
}

// ==================================================
// Instrument name
// ==================================================

juce::String MainComponent::instrumentName(
    InstrumentType instrument) const
{
    switch (instrument)
    {
        case InstrumentType::Guitar:
            return "Guitar";

        case InstrumentType::ElectricGuitar:
            return "Electric Guitar";

        case InstrumentType::Piano:
            return "Piano";

        case InstrumentType::ElectricPiano:
            return "Electric Piano";

        case InstrumentType::Bass:
            return "Bass";

        case InstrumentType::Organ:
            return "Organ";

        case InstrumentType::SynthLead:
            return "Synth Lead";

        case InstrumentType::SynthPad:
            return "Synth Pad";

        case InstrumentType::Strings:
            return "Strings";

        case InstrumentType::Flute:
            return "Flute";

        case InstrumentType::Brass:
            return "Brass";

        case InstrumentType::Bell:
            return "Bell";
    }

    return "Guitar";
}

// ==================================================
// Drum name
// ==================================================

juce::String MainComponent::drumName(
    DrumType drum) const
{
    switch (drum)
    {
        case DrumType::Kick:
            return "Kick";

        case DrumType::Snare:
            return "Snare";

        case DrumType::HiHat:
            return "Hi-Hat";

        case DrumType::OpenHiHat:
            return "Open Hi-Hat";

        case DrumType::Clap:
            return "Clap";

        case DrumType::Rimshot:
            return "Rimshot";

        case DrumType::Tom:
            return "Tom";

        case DrumType::LowTom:
            return "Low Tom";

        case DrumType::MidTom:
            return "Mid Tom";

        case DrumType::HighTom:
            return "High Tom";

        case DrumType::Crash:
            return "Crash";

        case DrumType::Ride:
            return "Ride";

        case DrumType::Cowbell:
            return "Cowbell";

        case DrumType::Tambourine:
            return "Tambourine";

        case DrumType::Shaker:
            return "Shaker";
    }

    return "Kick";
}

// ==================================================
// Add instrument note
// ==================================================

void MainComponent::addInstrumentNote()
{
    try
    {
        const int selectedInstrument =
            instrumentBox.getSelectedId();

        const int selectedNote =
            noteBox.getSelectedId();

        if (selectedInstrument <= 0)
        {
            throw std::runtime_error(
                "Select an instrument.");
        }

        if (selectedNote <= 0)
        {
            throw std::runtime_error(
                "Select a note.");
        }

        juce::String instrumentCommand;

        switch (selectedInstrument)
        {
            case 1:
                instrumentCommand = "GUITAR";
                break;

            case 2:
                instrumentCommand = "ELECTRICGUITAR";
                break;

            case 3:
                instrumentCommand = "PIANO";
                break;

            case 4:
                instrumentCommand = "ELECTRICPIANO";
                break;

            case 5:
                instrumentCommand = "BASS";
                break;

            case 6:
                instrumentCommand = "ORGAN";
                break;

            case 7:
                instrumentCommand = "SYNTHLEAD";
                break;

            case 8:
                instrumentCommand = "SYNTHPAD";
                break;

            case 9:
                instrumentCommand = "STRINGS";
                break;

            case 10:
                instrumentCommand = "FLUTE";
                break;

            case 11:
                instrumentCommand = "BRASS";
                break;

            case 12:
                instrumentCommand = "BELL";
                break;

            default:
                throw std::runtime_error(
                    "Invalid instrument selection.");
        }

        const juce::String note =
            noteBox.getText();

        const juce::String event =
            instrumentCommand +
            " " +
            note +
            " 0 1";

        juce::String text =
            projectInfo.getText();

        if (text.contains(
                "No project loaded."))
        {
            text =
                "TEMPO 120\n"
                "TRACK 1 INSTRUMENT\n"
                "LENGTH 8\n\n";
        }

        if (text.trim().isEmpty())
        {
            text =
                "TEMPO 120\n"
                "TRACK 1 INSTRUMENT\n"
                "LENGTH 8\n\n";
        }

        if (text.isNotEmpty() &&
            !text.endsWithChar('\n'))
        {
            text += "\n";
        }

        text += event;
        text += "\n";

        projectInfo.setText(
            text,
            false);

        statusLabel.setText(
            "Added " +
            instrumentName(
                static_cast<InstrumentType>(
                    selectedInstrument - 1)) +
            " " +
            note +
            " at beat 0.",
            juce::dontSendNotification);
    }
    catch (const std::exception& e)
    {
        statusLabel.setText(
            "Add Note error: " +
            juce::String(e.what()),
            juce::dontSendNotification);
    }
}

// ==================================================
// Add drum event
// ==================================================

void MainComponent::addDrumEvent()
{
    try
    {
        const int selectedDrum =
            drumBox.getSelectedId();

        if (selectedDrum <= 0)
        {
            throw std::runtime_error(
                "Select a drum.");
        }

        juce::String drumCommand;

        switch (selectedDrum)
        {
            case 1:
                drumCommand = "KICK";
                break;

            case 2:
                drumCommand = "SNARE";
                break;

            case 3:
                drumCommand = "HIHAT";
                break;

            case 4:
                drumCommand = "OPENHIHAT";
                break;

            case 5:
                drumCommand = "CLAP";
                break;

            case 6:
                drumCommand = "RIMSHOT";
                break;

            case 7:
                drumCommand = "TOM";
                break;

            case 8:
                drumCommand = "LOWTOM";
                break;

            case 9:
                drumCommand = "MIDTOM";
                break;

            case 10:
                drumCommand = "HIGHTOM";
                break;

            case 11:
                drumCommand = "CRASH";
                break;

            case 12:
                drumCommand = "RIDE";
                break;

            case 13:
                drumCommand = "COWBELL";
                break;

            case 14:
                drumCommand = "TAMBOURINE";
                break;

            case 15:
                drumCommand = "SHAKER";
                break;

            default:
                throw std::runtime_error(
                    "Invalid drum selection.");
        }

        const juce::String event =
            "DRUM " +
            drumCommand +
            " 0";

        juce::String text =
            projectInfo.getText();

        if (text.contains(
                "No project loaded."))
        {
            text =
                "TEMPO 120\n"
                "TRACK 1 INSTRUMENT\n"
                "LENGTH 8\n\n";
        }

        if (text.trim().isEmpty())
        {
            text =
                "TEMPO 120\n"
                "TRACK 1 INSTRUMENT\n"
                "LENGTH 8\n\n";
        }

        if (text.isNotEmpty() &&
            !text.endsWithChar('\n'))
        {
            text += "\n";
        }

        text += event;
        text += "\n";

        projectInfo.setText(
            text,
            false);

        statusLabel.setText(
            "Added " +
            drumName(
                static_cast<DrumType>(
                    selectedDrum - 1)) +
            " at beat 0.",
            juce::dontSendNotification);
    }
    catch (const std::exception& e)
    {
        statusLabel.setText(
            "Add Drum error: " +
            juce::String(e.what()),
            juce::dontSendNotification);
    }
}

// ==================================================
// Open project/audio
// ==================================================

void MainComponent::openProject()
{
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

            if (extension == ".song")
            {
                stopPlayback();

                currentProject = result;
                currentAudioFile = juce::File{};

                try
                {
                    Project project =
                        Parser::parseFile(
                            currentProject
                                .getFullPathName()
                                .toStdString());

                    projectInfo.setText(
                        currentProject
                            .loadFileAsString(),
                        false);

                    tempoLabel.setText(
                        "Tempo: " +
                        juce::String(
                            project.tempo,
                            1) +
                        " BPM",
                        juce::dontSendNotification);

                    tracksLabel.setText(
                        "Tracks: " +
                        juce::String(
                            static_cast<int>(
                                project.tracks.size())),
                        juce::dontSendNotification);

                    statusLabel.setText(
                        "Project loaded. Press Play.",
                        juce::dontSendNotification);
                }
                catch (const std::exception& e)
                {
                    currentProject =
                        juce::File{};

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

            if (loadAudioFileForPlayback(result))
            {
                fileChooser.reset();
                return;
            }

            statusLabel.setText(
                "Could not decode this media file.",
                juce::dontSendNotification);

            fileChooser.reset();
        });
}

// ==================================================
// Load audio file
// ==================================================

bool MainComponent::loadAudioFileForPlayback(
    const juce::File& audioFile)
{
    if (!audioFile.existsAsFile())
        return false;

    const juce::String editorText =
        projectInfo.getText();

    const bool hasProjectText =
        !editorText.trim().isEmpty() &&
        !editorText.contains(
            "No project loaded.");

    if (hasProjectText)
    {
        try
        {
            Project project =
                parseEditorProject(
                    editorText);

            if (project.tracks.empty())
            {
                throw std::runtime_error(
                    "No TRACK exists in the project.");
            }

            const int trackNumber =
                project.tracks.front().number;

            juce::String text =
                editorText;

            if (text.isNotEmpty() &&
                !text.endsWithChar('\n'))
            {
                text += "\n";
            }

            text +=
                "AUDIO \"" +
                audioFile.getFullPathName() +
                "\" 0\n";

            projectInfo.setText(
                text,
                false);

            currentAudioFile =
                audioFile;

            tempoLabel.setText(
                "Tempo: " +
                juce::String(
                    project.tempo,
                    1) +
                " BPM",
                juce::dontSendNotification);

            tracksLabel.setText(
                "Tracks: " +
                juce::String(
                    static_cast<int>(
                        project.tracks.size())),
                juce::dontSendNotification);

            statusLabel.setText(
                "Added audio to Track " +
                juce::String(trackNumber) +
                ": " +
                audioFile.getFileName(),
                juce::dontSendNotification);

            return true;
        }
        catch (const std::exception& e)
        {
            statusLabel.setText(
                "Add audio error: " +
                juce::String(e.what()),
                juce::dontSendNotification);

            return false;
        }
    }

    try
    {
        juce::String text =
            "TEMPO 120\n"
            "TRACK 1 INSTRUMENT\n"
            "LENGTH 8\n\n"
            "AUDIO \"" +
            audioFile.getFullPathName() +
            "\" 0\n";

        projectInfo.setText(
            text,
            false);

        currentProject = juce::File{};
        currentAudioFile = audioFile;

        tempoLabel.setText(
            "Tempo: 120.0 BPM",
            juce::dontSendNotification);

        tracksLabel.setText(
            "Tracks: 1",
            juce::dontSendNotification);

        statusLabel.setText(
            "Audio added to Track 1: " +
            audioFile.getFileName(),
            juce::dontSendNotification);

        return true;
    }
    catch (...)
    {
        return false;
    }
}

// ==================================================
// Save editor to project
// ==================================================

bool MainComponent::saveEditorToProject()
{
    if (!currentProject.existsAsFile())
        return false;

    return currentProject.replaceWithText(
        projectInfo.getText());
}

// ==================================================
// Clean editor text
// ==================================================

static juce::String cleanEditorProjectText(
    const juce::String& originalText)
{
    juce::StringArray lines;

    lines.addLines(originalText);

    juce::String cleaned;

    for (const auto& line : lines)
    {
        const juce::String trimmed =
            line.trim();

        if (trimmed == "No project loaded.")
            continue;

        if (trimmed ==
            "You can open a .song file or paste/type a project directly into this editor.")
        {
            continue;
        }

        if (trimmed == "No file loaded.")
            continue;

        if (trimmed ==
            "Click \"Open File\" to load a .song, .wav, .mp3, or other media file.")
        {
            continue;
        }

        cleaned += line;
        cleaned += "\n";
    }

    return cleaned.trim();
}

// ==================================================
// Parse editor project
// ==================================================

static Project parseEditorProject(
    const juce::String& editorText)
{
    const juce::String cleanedText =
        cleanEditorProjectText(
            editorText);

    if (cleanedText.isEmpty())
    {
        throw std::runtime_error(
            "The project editor is empty.");
    }

    const juce::File tempProject =
        juce::File::getSpecialLocation(
            juce::File::tempDirectory)
            .getChildFile(
                "AudioEditor_typed_project.song");

    if (!tempProject.replaceWithText(
            cleanedText))
    {
        throw std::runtime_error(
            "Could not create temporary project file.");
    }

    try
    {
        Project project =
            Parser::parseFile(
                tempProject
                    .getFullPathName()
                    .toStdString());

        tempProject.deleteFile();

        return project;
    }
    catch (...)
    {
        tempProject.deleteFile();
        throw;
    }
}

// ==================================================
// Play project
// ==================================================

void MainComponent::playProject()
{
    stopPlayback();

    try
    {
        Project project;

        if (isProjectLoaded())
        {
            if (!saveEditorToProject())
            {
                statusLabel.setText(
                    "Could not save project.",
                    juce::dontSendNotification);

                return;
            }

            project =
                Parser::parseFile(
                    currentProject
                        .getFullPathName()
                        .toStdString());
        }
        else
        {
            project =
                parseEditorProject(
                    projectInfo.getText());
        }

        if (project.tracks.empty())
        {
            throw std::runtime_error(
                "No track data loaded. Add at least one TRACK declaration.");
        }

        tempoLabel.setText(
            "Tempo: " +
            juce::String(
                project.tempo,
                1) +
            " BPM",
            juce::dontSendNotification);

        tracksLabel.setText(
            "Tracks: " +
            juce::String(
                static_cast<int>(
                    project.tracks.size())),
            juce::dontSendNotification);

        constexpr int sampleRate = 48000;

        EditorAudioBuffer rendered =
            ProjectRenderer::render(
                project,
                sampleRate);

        juce::File playbackDirectory;

        if (isProjectLoaded())
        {
            playbackDirectory =
                currentProject
                    .getParentDirectory();
        }
        else
        {
            playbackDirectory =
                juce::File::getSpecialLocation(
                    juce::File::tempDirectory);
        }

        const juce::File playbackFile =
            playbackDirectory.getChildFile(
                "_audioeditor_playback.wav");

        if (!WavWriter::write(
                playbackFile
                    .getFullPathName()
                    .toStdString(),
                rendered))
        {
            throw std::runtime_error(
                "Could not create playback WAV.");
        }

        auto reader =
            std::unique_ptr<
                juce::AudioFormatReader>(
                formatManager.createReaderFor(
                    playbackFile));

        if (reader == nullptr)
        {
            throw std::runtime_error(
                "Could not open rendered playback WAV.");
        }

        const double readerSampleRate =
            reader->sampleRate;

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
    catch (...)
    {
        statusLabel.setText(
            "Play error: unknown error.",
            juce::dontSendNotification);
    }
}

        // ==================================================
// Render project
// ==================================================

void MainComponent::renderProject()
{
    try
    {
        Project project;

        // ==================================================
        // Load project
        // ==================================================

        if (isProjectLoaded())
        {
            if (!saveEditorToProject())
            {
                statusLabel.setText(
                    "Could not save project.",
                    juce::dontSendNotification);

                return;
            }

            project =
                Parser::parseFile(
                    currentProject
                        .getFullPathName()
                        .toStdString());
        }
        else
        {
            project =
                parseEditorProject(
                    projectInfo.getText());
        }

        // ==================================================
        // Validate project
        // ==================================================

        if (project.tracks.empty())
        {
            throw std::runtime_error(
                "No track data loaded. Add at least one TRACK declaration.");
        }

        // ==================================================
        // Update interface
        // ==================================================

        statusLabel.setText(
            "Rendering project...",
            juce::dontSendNotification);

        tempoLabel.setText(
            "Tempo: " +
            juce::String(
                project.tempo,
                1) +
            " BPM",
            juce::dontSendNotification);

        tracksLabel.setText(
            "Tracks: " +
            juce::String(
                static_cast<int>(
                    project.tracks.size())),
            juce::dontSendNotification);

        // ==================================================
        // Render audio
        // ==================================================

        constexpr int sampleRate = 48000;

        EditorAudioBuffer rendered =
            ProjectRenderer::render(
                project,
                sampleRate);

        // ==================================================
        // Ask user where to save the WAV
        // ==================================================

        fileChooser =
            std::make_unique<juce::FileChooser>(
                "Save Rendered WAV",
                juce::File{},
                "*.wav");

        fileChooser->launchAsync(
            juce::FileBrowserComponent::saveMode |
            juce::FileBrowserComponent::canSelectFiles |
            juce::FileBrowserComponent::warnAboutOverwriting,
            [this, rendered = std::move(rendered)]
            (const juce::FileChooser& chooser) mutable
            {
                // ==================================================
                // Get selected output file
                // ==================================================

                juce::File outputFile =
                    chooser.getResult();

                // ==================================================
                // User cancelled
                // ==================================================

                if (outputFile == juce::File{})
                {
                    statusLabel.setText(
                        "Render cancelled.",
                        juce::dontSendNotification);

                    fileChooser.reset();
                    return;
                }

                // ==================================================
                // Make sure the filename ends in .wav
                // ==================================================

                if (outputFile.getFileExtension()
                        .isEmpty())
                {
                    outputFile =
                        outputFile.withFileExtension(
                            ".wav");
                }

                // ==================================================
                // Write WAV
                // ==================================================

                const bool success =
                    WavWriter::write(
                        outputFile
                            .getFullPathName()
                            .toStdString(),
                        rendered);

                if (!success)
                {
                    statusLabel.setText(
                        "Failed to save WAV file.",
                        juce::dontSendNotification);

                    fileChooser.reset();
                    return;
                }

                // ==================================================
                // Success
                // ==================================================

                statusLabel.setText(
                    "WAV saved: " +
                    outputFile.getFullPathName(),
                    juce::dontSendNotification);

                fileChooser.reset();
            });
    }
    catch (const std::exception& e)
    {
        statusLabel.setText(
            "Render error: " +
            juce::String(e.what()),
            juce::dontSendNotification);
    }
    catch (...)
    {
        statusLabel.setText(
            "Render error: unknown error.",
            juce::dontSendNotification);
    }
}
// ==================================================
// Prepare audio
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
// Release resources
// ==================================================

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

// ==================================================
// Paint
// ==================================================

void MainComponent::paint(
    juce::Graphics& g)
{
    g.fillAll(
        juce::Colour::fromRGB(
            30,
            30,
            34));
}

// ==================================================
// Layout
// ==================================================

void MainComponent::resized()
{
    auto area =
        getLocalBounds().reduced(25);

    // ==================================================
    // Title
    // ==================================================

    titleLabel.setBounds(
        area.removeFromTop(45));

    // ==================================================
    // Main buttons
    // ==================================================

    auto mainButtonArea =
        area.removeFromTop(50);

    openButton.setBounds(
        mainButtonArea
            .removeFromLeft(150)
            .reduced(5));

    playButton.setBounds(
        mainButtonArea
            .removeFromLeft(120)
            .reduced(5));

    renderButton.setBounds(
        mainButtonArea
            .removeFromLeft(150)
            .reduced(5));

    // ==================================================
    // Instrument / drum controls
    // ==================================================

    auto instrumentArea =
        area.removeFromTop(45);

    instrumentLabel.setBounds(
        instrumentArea
            .removeFromLeft(85)
            .reduced(3));

    instrumentBox.setBounds(
        instrumentArea
            .removeFromLeft(190)
            .reduced(3));

    addNoteButton.setBounds(
        instrumentArea
            .removeFromLeft(130)
            .reduced(3));

    drumLabel.setBounds(
        instrumentArea
            .removeFromLeft(55)
            .reduced(3));

    drumBox.setBounds(
        instrumentArea
            .removeFromLeft(150)
            .reduced(3));

    addDrumButton.setBounds(
        instrumentArea
            .removeFromLeft(105)
            .reduced(3));

    // ==================================================
    // Note controls
    // ==================================================

    auto noteArea =
        area.removeFromTop(40);

    noteLabel.setBounds(
        noteArea
            .removeFromLeft(50)
            .reduced(3));

    noteBox.setBounds(
        noteArea
            .removeFromLeft(100)
            .reduced(3));

    // ==================================================
    // Tempo / tracks
    // ==================================================

    auto infoArea =
        area.removeFromTop(35);

    tempoLabel.setBounds(
        infoArea
            .removeFromLeft(180));

    tracksLabel.setBounds(
        infoArea
            .removeFromLeft(180));

    // ==================================================
    // Status
    // ==================================================

    auto statusArea =
        getLocalBounds()
            .reduced(25)
            .removeFromBottom(30);

    statusLabel.setBounds(
        statusArea);

    // ==================================================
    // Project editor
    // ==================================================

    projectInfo.setBounds(
        area.reduced(5));
}
