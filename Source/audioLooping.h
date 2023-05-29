/*
  ==============================================================================

    main.cpp
    Created: 23 May 2023 5:19:04pm
    Author:  nguyenbui45

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent
{
public:
    MainContentComponent()
    {
        addAndMakeVisible (openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (clearButton);
        clearButton.setButtonText ("Clear");
        clearButton.onClick = [this] { clearButtonClicked(); };

		addAndMakeVisible(levelSlider);
		levelSlider.setRange(0.0,1.0);
		levelSlider.onValueChange = [this]{currentLevel = (float) levelSlider.getValue();};

        formatManager.registerBasicFormats();
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int, double) override {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
		auto level = currentLevel;
		auto startLevel = juce::approximatelyEqual(level,previousLevel) ? level:previousLevel;

        auto numInputChannels = fileBuffer.getNumChannels();
        auto numOutputChannels = bufferToFill.buffer->getNumChannels();

        auto outputSamplesRemaining = bufferToFill.numSamples;                              
        auto outputSamplesOffset = bufferToFill.startSample;                                

        while (outputSamplesRemaining > 0)
        {
            auto bufferSamplesRemaining = fileBuffer.getNumSamples() - position;            
            auto samplesThisTime = juce::jmin (outputSamplesRemaining, bufferSamplesRemaining);

            for (auto channel = 0; channel < numOutputChannels; ++channel)
            {
                bufferToFill.buffer->copyFrom (channel,                                     
                                               outputSamplesOffset,                         
                                               fileBuffer,                                  
                                               channel % numInputChannels,                  
                                               position,                                    
                                               samplesThisTime);
				bufferToFill.buffer->applyGainRamp(channel,outputSamplesOffset,samplesThisTime,startLevel,level);
            }

            outputSamplesRemaining -= samplesThisTime;                                      
            outputSamplesOffset += samplesThisTime;                                     
            position += samplesThisTime;                                                

            if (position == fileBuffer.getNumSamples())
                position = 0;                                                               
        }
		previousLevel = level;
    }

    void releaseResources() override
    {
        fileBuffer.setSize (0, 0);
    }

    void resized() override
    {
        openButton .setBounds (10, 10, getWidth() - 20, 20);
        clearButton.setBounds (10, 40, getWidth() - 20, 20);
		levelSlider.setBounds (10,70,getWidth()-20,20);
    }

private:
    void openButtonClicked()
    {
        shutdownAudio();                                                                            // [1]

        chooser = std::make_unique<juce::FileChooser> ("Select a Wave file shorter than 2 seconds to play...",
                                                       juce::File{},
                                                       "*.wav");
        auto chooserFlags = juce::FileBrowserComponent::openMode
                          | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();

            if (file == juce::File{})
                return;

            std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file)); // [2]

            if (reader.get() != nullptr)
            {
                auto duration = (float) reader->lengthInSamples / reader->sampleRate;               // [3]

                if (duration < 2)
                {
                    fileBuffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);  // [4]
                    reader->read (&fileBuffer,                                                      // [5]
                                  0,                                                                //  [5.1]
                                  (int) reader->lengthInSamples,                                    //  [5.2]
                                  0,                                                                //  [5.3]
                                  true,                                                             //  [5.4]
                                  true);                                                            //  [5.5]
                    position = 0;                                                                   // [6]
                    setAudioChannels (0, (int) reader->numChannels);                                // [7]
                }
                else
                {
                    // handle the error that the file is 2 seconds or longer..
                }
            }
        });
    }

    void clearButtonClicked()
    {
        shutdownAudio();
    }

    //==========================================================================
    juce::TextButton openButton;
    juce::TextButton clearButton;
	juce::Slider levelSlider; 

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    juce::AudioSampleBuffer fileBuffer;

    int position;
	float currentLevel = 0.0f, previousLevel = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)

};
