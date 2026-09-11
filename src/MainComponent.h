#pragma once
#include <JuceHeader.h>
#include "Project.h"

// ==================================================
// MainComponent
// ==================================================

class MainComponent
    : public juce::AudioAppComponent
{
public:

    MainComponent();

    ~MainComponent() override;

    // ----------------------------------------------
    // Audio
    // ----------------------------------------------

    void prepareToPlay(
        int samplesPerBlockExpected,
        double sampleRate) override;

    void getNextAudioBlock(
        const juce::AudioSourceChannelInfo& bufferToFill) override;

    void releaseResources() override;

    // ----------------------------------------------
    // GUI
    // ----------------------------------------------

    void paint(
        juce::Graphics& g) override;

    void resized() override;

private:

    // ==================================================
    // File / project functions
    // ==================================================

    void openProject();

    bool loadAudioFileForPlayback(
        const juce::File& audioFile);

    bool saveEditorToProject();

    bool isProjectLoaded() const;

    bool isAudioFileLoaded() const;

    // ==================================================
    // Playback / rendering
    // ==================================================

    void playProject();

    void renderProject();

    void stopPlayback();

    // ==================================================
    // Instrument / drum editor
    // ==================================================

    void addInstrumentNote();

    void addDrumEvent();

    // ==================================================
    // Names
    //
    // These MUST use the enum types from Project.h.
    // ==================================================

    juce::String instrumentName(
        InstrumentType instrument) const;

    juce::String drumName(
        DrumType drum) const;

    // ==================================================
    // Audio
    // ==================================================

    juce::AudioFormatManager formatManager;

    juce::AudioTransportSource transportSource;

    std::unique_ptr<juce::AudioFormatReaderSource>
        readerSource;

    // ==================================================
    // Files
    // ==================================================

    juce::File currentProject;

    juce::File currentAudioFile;

    std::unique_ptr<juce::FileChooser>
        fileChooser;

    // ==================================================
    // Title
    // ==================================================

    juce::Label titleLabel;

    // ==================================================
    // Main buttons
    // ==================================================

    juce::TextButton openButton;

    juce::TextButton playButton;

    juce::TextButton renderButton;

    // ==================================================
    // Instrument controls
    // ==================================================

    juce::Label instrumentLabel;

    juce::ComboBox instrumentBox;

    juce::TextButton addNoteButton;

    // ==================================================
    // Drum controls
    // ==================================================

    juce::Label drumLabel;

    juce::ComboBox drumBox;

    juce::TextButton addDrumButton;

    // ==================================================
    // Note controls
    // ==================================================

    juce::Label noteLabel;

    juce::ComboBox noteBox;

    juce::Label startBeatLabel;

    juce::TextEditor startBeatEditor;

    juce::Label durationLabel;

    juce::TextEditor durationEditor;

    // ==================================================
    // Project information
    // ==================================================

    juce::Label tempoLabel;

    juce::Label tracksLabel;

    juce::TextEditor projectInfo;

    // ==================================================
    // Status
    // ==================================================

    juce::Label statusLabel;

    // ==================================================
    // Prevent copying
    // ==================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        MainComponent
    )
};
