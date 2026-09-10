#pragma once
#include <JuceHeader.h>
#include "Parser.h"

// ==================================================
// Main AudioEditor window
// ==================================================

class MainComponent : public juce::AudioAppComponent
{
public:
MainComponent();
~MainComponent() override;

void paint(juce::Graphics&) override;
void resized() override;

// ==================================================
// AudioAppComponent
// ==================================================

void prepareToPlay(
    int samplesPerBlockExpected,
    double sampleRate) override;

void getNextAudioBlock(
    const juce::AudioSourceChannelInfo& bufferToFill) override;

void releaseResources() override;


private:

// ==================================================
// File handling
// ==================================================

void openProject();
void playProject();
void renderProject();

bool saveEditorToProject();

// Opened .song project
juce::File currentProject;

// Opened WAV / MP3 / MP4
juce::File currentAudioFile;

// ==================================================
// File chooser
// ==================================================

std::unique_ptr<juce::FileChooser> fileChooser;

// ==================================================
// UI
// ==================================================

juce::TextButton openButton { "Open File" };
juce::TextButton playButton { "Play" };
juce::TextButton renderButton { "Render WAV" };

juce::Label titleLabel;
juce::Label tempoLabel;
juce::Label tracksLabel;

juce::TextEditor projectInfo;

juce::Label statusLabel;

// ==================================================
// Audio playback
// ==================================================

juce::AudioFormatManager formatManager;

juce::AudioTransportSource transportSource;

std::unique_ptr<juce::AudioFormatReaderSource>
    readerSource;

// ==================================================
// Helpers
// ==================================================

bool isProjectLoaded() const;
bool isAudioFileLoaded() const;

bool loadAudioFileForPlayback(
    const juce::File& audioFile);

void stopPlayback();

// ==================================================
// JUCE
// ==================================================

JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)


};