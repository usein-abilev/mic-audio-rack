#include <juce_audio_devices/juce_audio_devices.h>

#include "main_component.h"

class MicAudioRackApplication : public juce::JUCEApplication {
   public:
    const juce::String getApplicationName() override { return "MicAudioRack"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String&) override {
        mainWindow.reset(new MainWindow("MicAudioRack", new MainComponent(), *this));
    }

    void shutdown() override { mainWindow = nullptr; }

    class MainWindow : public juce::DocumentWindow {
       public:
        MainWindow(juce::String name, juce::Component* c, JUCEApplication& app)
            : DocumentWindow(name, juce::Colours::black, DocumentWindow::allButtons), owner(app) {
            setUsingNativeTitleBar(true);
            setContentOwned(c, true);
            setResizable(true, true);
            centreWithSize(800, 600);
            setVisible(true);
        }

        void closeButtonPressed() override { owner.systemRequestedQuit(); }

       private:
        JUCEApplication& owner;
    };

   private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(MicAudioRackApplication)
