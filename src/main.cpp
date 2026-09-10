#include <JuceHeader.h>
#include "MainComponent.h"

class AudioEditorApplication : public juce::JUCEApplication
{
public:
    AudioEditorApplication() = default;

    const juce::String getApplicationName() override
    {
        return "AudioEditor";
    }

    const juce::String getApplicationVersion() override
    {
        return "1.0.0";
    }

    bool moreThanOneInstanceAllowed() override
    {
        return true;
    }

    void initialise(const juce::String&) override
    {
        mainWindow =
            std::make_unique<MainWindow>(
                getApplicationName());
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(
        const juce::String&) override
    {
    }

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(juce::String name)
            : DocumentWindow(
                  name,
                  juce::Colours::darkgrey,
                  juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);

            setContentOwned(
                new MainComponent(),
                true);

            centreWithSize(900, 600);

            setResizable(true, true);

            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()
                ->systemRequestedQuit();
        }
    };

    std::unique_ptr<MainWindow> mainWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AudioEditorApplication)
};

START_JUCE_APPLICATION(AudioEditorApplication)
