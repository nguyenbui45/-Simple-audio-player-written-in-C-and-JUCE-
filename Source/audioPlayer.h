/*
  ==============================================================================

    main.cpp
    Created: 23 May 2023 5:19:04pm
    Author:  nguyenbui45

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <iostream>

class MainContentComponent: public juce::ChangeListener, public juce::AudioAppComponent, private juce::Timer
{ 
    private:
            // enumerize states by numbers
        enum TransportState{
                Stopped,
                Starting,
                Playing,
                Pausing,
                Paused,
                Stopping
        };
    
        // Declare button on audio player
        juce::TextButton openButton_;
        juce::TextButton playButton_;
        juce::TextButton stopButton_;
        juce::Label currentPositionLabel_;
        
        //length of audio file
        int lengthInSecond_;
        
        // Declare thumbnail
        juce::AudioThumbnail thumbnail_;
        juce::AudioThumbnailCache thumbnailCache_;
        
        std::unique_ptr<juce::FileChooser> chooser; // pointer control the file
    
        juce::AudioFormatManager formatManager; // variable to register a audio format
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource; // pointer of AudioFormatReaderSource class to check if audio playback pass to
        juce::AudioTransportSource transportSource; // variable to listen to the state's change
        TransportState state_; // enum of state
        
        void timerCallback() override{
            if(transportSource.isPlaying()) {
                juce::RelativeTime position(transportSource.getCurrentPosition());
                auto minutes = ((int) position.inMinutes()) % 60;
                auto seconds = ((int) position.inSeconds()) % 60;
                auto millis = ((int) position.inMilliseconds()) % 1000;
                auto full_minutes = ((int) lengthInSecond_ / 60);
                auto full_seconds = ((int) lengthInSecond_ - (full_minutes * 60));
                auto full_millis = ((int) 0);
                auto positionString = juce::String::formatted("%02d:%02d:%03d/%02d:%02d:%03d",minutes,seconds,millis,full_minutes,full_seconds,full_millis);
                
                currentPositionLabel_.setText(positionString,juce::dontSendNotification);
            }
            else {
                currentPositionLabel_.setText("Stopped",juce::dontSendNotification);
            }
            
            repaint();
        }
        
        
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
                        stopButton_.setButtonText("Stop");
                        playButton_.setButtonText("Start");
                        transportSource.setPosition(0.0);
                        break;
                    case Starting:
                        transportSource.start();
                        break;
                    case Playing:
                        stopButton_.setEnabled(true);
                        stopButton_.setButtonText("Stop");
                        playButton_.setButtonText("Pause");
                        break;
                    case Stopping:
                        transportSource.stop();
                        break;
                    case Pausing:
                        transportSource.stop();
                        break;
                    case Paused:
                        playButton_.setButtonText("Resume");
                        stopButton_.setButtonText("Stop");
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
                        thumbnail_.setSource (new juce::FileInputSource (file));
                        // Since AudioTransportSource now be our newlly allocated AudioFormatReaderSource object.
                        // we can save the AudioFormatReaderSource object in our readerSource member.
                        // To do this, we must transfer the ownership from newSource by std::make_unique.release()
                        readerSource.reset(newSource.release());
                        lengthInSecond_ = reader->lengthInSamples / reader->sampleRate;
                        
                    }
                }
            });
        }
        
        void playButtonClicked_(){
            if ((state_ == Stopped) || (state_ ==Paused))   
                changeState_(Starting);
            else if(state_ == Playing)
                changeState_(Pausing);
                
        }
        
        void stopButtonClicked_(){
            if(state_ == Paused)
                changeState_(Stopped);
            else
                changeState_(Stopping);
        }
                
            
                                
        
        
        
    
    public:
        MainContentComponent(): state_(Stopped),thumbnailCache_(5),thumbnail_(512,formatManager,thumbnailCache_){
            // initialization of buttons
            juce::Component::addAndMakeVisible(&openButton_);
            openButton_.setButtonText("Choose song");
            openButton_.onClick = [this]{openButtonClicked_();};
            openButton_.setColour(juce::TextButton::buttonColourId, juce::Colours::grey); // color decoration of button
        
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
            
            juce::Component::addAndMakeVisible(&currentPositionLabel_);
            currentPositionLabel_.setText("Stopped",juce::dontSendNotification);
            
            
            
            formatManager.registerBasicFormats();// register a basic format method()
            transportSource.addChangeListener(this); // add a listener so that we can respond to changes in its state
            
            setAudioChannels (0, 2);
            
            //thumbnail
            thumbnail_.addChangeListener(this);
            startTimer (20);
            
        }
        
        ~MainContentComponent() override{
            shutdownAudio();
        }
        
        void resized() override{
            openButton_.setBounds(10,getHeight()-100,70,70);
            playButton_.setBounds(90,getHeight()-100,70,70);
            stopButton_.setBounds(170,getHeight()-100,70,70);
            currentPositionLabel_.setBounds(260,getHeight()-70,100,30);
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
                if(transportSource.isPlaying()){
                    changeState_(Playing);
                }
                else if((state_ == Stopping) || (state_ == Playing))
                    changeState_(Stopped);
                else if(state_ == Pausing)
                    changeState_(Paused);
            }
            
            if(source == &thumbnail_)
                    thumbnailChanged();
        }
        
        
        void thumbnailChanged(){
            repaint();
        }
          
        
        
        /*
        
        
                                            DRAWING THE THUMBNAIL
        
        */
        
        void paint(juce::Graphics& g) override{
            juce::Rectangle<int> thumbnailBounds(10,10,getWidth()  -20,getHeight()-200);
            
            if(thumbnail_.getNumChannels() == 0)
                paintIfNoFileLoaded(g,thumbnailBounds);
            else
                paintIfFileLoaded(g,thumbnailBounds);
        }
        
        void paintIfNoFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds) {
            g.setColour (juce::Colours::grey);
            g.fillRect (thumbnailBounds);
            g.setColour (juce::Colours::white);
            g.drawFittedText ("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
        }
        
        void paintIfFileLoaded (juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds){
            g.setColour (juce::Colours::black);
            g.fillRect (thumbnailBounds);
 
            g.setColour (juce::Colours::wheat);
 
            auto audioLength = (float) thumbnail_.getTotalLength();                               // [12]
            thumbnail_.drawChannels (g, thumbnailBounds, 0.0, audioLength, 1.0f);
 
            g.setColour (juce::Colours::green);
 
            auto audioPosition = (float) transportSource.getCurrentPosition();
            auto drawPosition = (audioPosition / audioLength) * (float) thumbnailBounds.getWidth()
                            + (float) thumbnailBounds.getX();                                // [13]
            g.drawLine (drawPosition, (float) thumbnailBounds.getY(), drawPosition,
                    (float) thumbnailBounds.getBottom(), 2.0f);                              // [14]
        }
            
    
};
