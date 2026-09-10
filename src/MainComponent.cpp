#include <JuceHeader.h>
#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(900, 600);

    titleLabel.setText(
        "AudioEditor",
        juce::dontSendNotification);

    titleLabel.setFont(
        juce::Font(28.0f, juce::Font::bold));

    titleLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    addAndMakeVisible(titleLabel);

    openButton.setButtonText("Open Project");
    renderButton.setButtonText("Render WAV");

    addAndMakeVisible(openButton);
    addAndMakeVisible(renderButton);

    tempoLabel.setText(
        "Tempo: -- BPM",
        juce::dontSendNotification);

    tracksLabel.setText(
        "Tracks: --",
        juce::dontSendNotification);

    addAndMakeVisible(tempoLabel);
    addAndMakeVisible(tracksLabel);

    projectInfo.setMultiLine(true);
    projectInfo.setReadOnly(true);
    projectInfo.setScrollbarsShown(true);
    projectInfo.setFont(juce::Font(16.0f));

    projectInfo.setText(
        "No project loaded.\n\n"
        "Click \"Open Project\" to load a .song file.",
        false);

    addAndMakeVisible(projectInfo);

    statusLabel.setText(
        "AudioEditor is ready.",
        juce::dontSendNotification);

    statusLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::lightgrey);

    addAndMakeVisible(statusLabel);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(
        juce::Colour::fromRGB(30, 30, 34));
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(25);

    titleLabel.setBounds(
        area.removeFromTop(45));

    auto buttonArea =
        area.removeFromTop(50);

    openButton.setBounds(
        buttonArea.removeFromLeft(150)
            .reduced(5));

    renderButton.setBounds(
        buttonArea.removeFromLeft(150)
            .reduced(5));

    auto infoArea =
        area.removeFromTop(45);

    tempoLabel.setBounds(
        infoArea.removeFromLeft(180));

    tracksLabel.setBounds(
        infoArea.removeFromLeft(180));

    projectInfo.setBounds(
        area.reduced(5));

    statusLabel.setBounds(
        getLocalBounds()
            .reduced(25)
            .removeFromBottom(30));
}
