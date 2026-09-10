#pragma once

#include <JuceHeader.h>
#include "Parser.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void openProject();
    void renderProject();

    juce::TextButton openButton { "Open Project" };
    juce::TextButton renderButton { "Render WAV" };

    juce::Label titleLabel;
    juce::Label tempoLabel;
    juce::Label tracksLabel;

    juce::TextEditor projectInfo;
    juce::Label statusLabel;

    juce::File currentProject;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
