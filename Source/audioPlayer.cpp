/*
  ==============================================================================

    main.cpp
    Created: 24 May 2023 1:02:24am
    Author:  nguyenbui45

  ==============================================================================
*/

#include "audioPlayer.h"
#include <iostream>

class Application: public juce:: JUCEApplication{
    private:
    
        class MainWindow : public juce::DocumentWindow{
            private:
                JUCEApplication& app;
                
                JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
            
            public:
                MainWindow(const juce::String& name, juce:: Component* c,JUCEApplication& a): DocumentWindow(name,
                                                                                                            juce::Colours::transparentBlack,
                                                                                                            juce::DocumentWindow::allButtons), app(a)
                                                                                               {
                                                                                        
            
                    setUsingNativeTitleBar(true);
                    setContentOwned(c, true);   
            
                    #if JUCE_ANDROID || JUCE_IOS
                    setFullScreen(true);
                    #else
                    setResizable(true,false);
                    setResizeLimits(500,450,10000,10000);
                    centreWithSize(getWidth(), getHeight());
                    #endif
            
                    setVisible(true);
            }
        
        
            void closeButtonPressed() override{
                app.systemRequestedQuit();
            }
        };
        
        std::unique_ptr<MainWindow> mainWindow;
                            
    
    
    
    public:   
        Application() = default;
        
        const juce::String getApplicationName() override {
            return "Audio playback";
        }
        const juce::String getApplicationVersion() override{
            return "1.0.0";
        }
        
        void initialise(const juce::String&) override{
            mainWindow.reset( new MainWindow(getApplicationName(), new MainContentComponent,*this));
        }
        
        void shutdown() override {
            mainWindow = nullptr;
        }
};


START_JUCE_APPLICATION (Application) // A macro to start JUCE app
        

