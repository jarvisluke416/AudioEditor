#include <JuceHeader.h>
#include "MainComponent.h"

// ==================================================
// AudioEditor Application
// ==================================================

class AudioEditorApplication : public juce::JUCEApplication
{
public:
    AudioEditorApplication() = default;

    // ==================================================
    // Application information
    // ==================================================

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

    // ==================================================
    // Start application
    // ==================================================

    void initialise(const juce::String&) override
    {
        mainWindow =
            std::make_unique<MainWindow>(
                getApplicationName());
    }

    // ==================================================
    // Shutdown
    // ==================================================

    void shutdown() override
    {
        mainWindow.reset();
    }

    // ==================================================
    // Quit
    // ==================================================

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(
        const juce::String&) override
    {
    }

private:

    // ==================================================
    // Main window
    // ==================================================

    class MainWindow
        : public juce::DocumentWindow
    {
    public:

        MainWindow(const juce::String& name)
            : juce::DocumentWindow(
                  name,
                  juce::Colours::darkgrey,
                  juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar(true);

            setContentOwned(
                new MainComponent(),
                true);

            centreWithSize(
                1000,
                750);

            setResizable(
                true,
                true);

            setVisible(true);
        }

        ~MainWindow() override = default;

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()
                ->systemRequestedQuit();
        }
    };

    // ==================================================
    // Application window
    // ==================================================

    std::unique_ptr<MainWindow> mainWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        AudioEditorApplication)
};

// ==================================================
// Launch application
// ==================================================

START_JUCE_APPLICATION(AudioEditorApplication)
