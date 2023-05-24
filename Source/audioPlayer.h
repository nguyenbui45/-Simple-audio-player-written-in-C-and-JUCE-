/*
  ==============================================================================

    main.cpp
    Created: 23 May 2023 5:19:04pm
    Author:  nguyenbui45

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class MainContentComponent: public juce::ChangeListener, public juce::AudioAppComponent
{ 
    private:
            // enumerize states by numbers
        enum TransportState{
                Stopped,
                Starting,
                Playing,
                Stopping
        };
    
        // Declare button on audio player
        juce::TextButton openButton_;
        juce::TextButton playButton_;
        juce::TextButton stopButton_;
        
        std::unique_ptr<juce::FileChooser> chooser; // pointer control the file
    
        juce::AudioFormatManager formatManager; // variable to register a audio format
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource; // pointer of AudioFormatReaderSource class to check if audio playback pass to
        juce::AudioTransportSource transportSource; // variable to listen to the state's change
        TransportState state_; // enum of state
        
        
        
        void changeState_(TransportState newState) {
            /*
            * At Stopped state, disable stopButton, enable playButton, reset transportSource position back to the beginning of the file
            * At Stating state, playButton is disabled as the user has already push playing, the AudioTransportSource object also start playing
            * At Playing state, stopButton is able activate, this state happen when the AudioTransportSource object report a change via changeListenerCallback() 
            * At Stopping state, AudioTransportSource object stop playing audio.
            *
            */
            if(state_ != newState){
                state_ = newState;
                
                switch(state_){
                    case Stopped:
                        stopButton_.setEnabled(false);
                        playButton_.setEnabled(true);
                        transportSource.setPosition(0.0);
                        break;
                    case Starting:
                        playButton_.setEnabled(false);
                        transportSource.start();
                        break;
                    case Playing:
                        stopButton_.setEnabled(true);
                        break;
                    case Stopping:
                        transportSource.stop();
                        break;
                }
            }
        }
           
        /*
        
                                OPENING A FILE
        
        
        */
        
        void openButtonClicked_(){
            chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play ...", juce::File{},"*.wav");
            //openMode: user can choose an existing file with attention
            //canSelecFiles: user can select file
            auto chooserFlags = juce::FileBrowserComponent::openMode |
                                juce::FileBrowserComponent::canSelectFiles; 
            // pop up chooser object
            chooser->launchAsync(chooserFlags, [this] (const juce::FileChooser& filechooser){
                auto file = filechooser.getResult();
                // if the audio is selected sucessfully at external folder
                if(file != juce::File{}){ 
                    // create audio format for the selected file
                    // formatManager.createRenderFor(file) will return nullptr if the file is not the format
                    // that AudioFormatManager can manage.
                    auto* reader = formatManager.createReaderFor(file);                  
                    if(reader != nullptr){
                        // create a new object of AudioFormatReaderSource to handle the AudioFormatReader object
                        // and delete AudioFormatReader if no logner needed.
                        auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader,true);
                        //AudioFormatReaderSource object connect with AudioTransportSource that begin used in getNextAudioBlock()
                        //
                        transportSource.setSource(newSource.get(),0,nullptr,reader->sampleRate);
                        playButton_.setEnabled(true);
                        // Since AudioTransportSource now be our newlly allocated AudioFormatReaderSource object.
                        // we can save the AudioFormatReaderSource object in our readerSource member.
                        // To do this, we must transfer the ownership from newSource by std::make_unique.release()
                        readerSource.reset(newSource.release());
                    }
                }
            });
        }
        
        void playButtonClicked_(){
            changeState_(Starting);
        }
        
        void stopButtonClicked_(){
            changeState_(Stopping);
        }
                
            
                                
        
        
        
    
    public:
        MainContentComponent(): state_(Stopped)
        {
            // initialization of buttons
            juce::Component::addAndMakeVisible(&openButton_);
            openButton_.setButtonText("Open...");
            openButton_.onClick = [this]{openButtonClicked_();};
        
            juce::Component::addAndMakeVisible(&playButton_);
            playButton_.setButtonText("Play"); // text display on button
            playButton_.onClick = [this]{playButtonClicked_();}; // declare a function button object point to if button is clicked
            playButton_.setColour(juce::TextButton::buttonColourId, juce::Colours::blue); // color decoration of button
            playButton_.setEnabled(false);
        
            juce::Component::addAndMakeVisible(&stopButton_);
            stopButton_.setButtonText("Stop");
            stopButton_.onClick = [this]{stopButtonClicked_();};
            stopButton_.setColour(juce::TextButton::buttonColourId,juce::Colours::red);
            stopButton_.setEnabled(false);
            
            formatManager.registerBasicFormats();// register a basic format method()
            transportSource.addChangeListener(this); // add a listener so that we can respond to changes in its state
            
            setAudioChannels (0, 2);
            juce::Component::setSize(500,450);
        }
        
        ~MainContentComponent() override{
            shutdownAudio();
        }
        
        void resized() override{
            openButton_.setBounds(75,100,70,70);
            playButton_.setBounds(75,200,70,70);
            stopButton_.setBounds(75,300,70,70);
        }
        
        
        
        /*
        
                                        PROCESSING THE AUDIO BLOCK
        
        
        */
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override {
            /* pass the prepareToPlay() callback funtion to any AudioSource object
            */
            transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
        }
        
        void releaseResources() override{
            /* pass th releaseResource() callback function to AudioSource object
            */
            transportSource.releaseResources();
        }
        
        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override{
        /*
        * check if input has audio playback by checking readerSource pointer
        * if not then clear the buffer 
        * else recursively call gertNextAudioBlock().
        *
        */
        
            if(readerSource.get() == nullptr){
                bufferToFill.clearActiveBufferRegion();
                return;
            }
            transportSource.getNextAudioBlock(bufferToFill);
        }
        
        
        
        
        /*
        
                                 RESPONDING TO AudioTransportSource CHANGES
        
        
        */
        
        
        
        void changeListenerCallback(juce::ChangeBroadcaster* source) override
        {
        /*
        * the ChangeBroadcaster hold a list of listeners to which it broadcasts a message when ChangeBroadcaster::sendChangeMessage() method is called
        * where ChangeListener is a class to receive callbacks() after sendChangeMessage() is called
        *
        * When chnages in the transportSource are reported, the changeListenerCallback() will be called.
        */
            if(source == &transportSource){
                if(transportSource.isPlaying())
                    changeState_(Playing);
                else
                    changeState_(Stopped);
            }
        }             
    
};
