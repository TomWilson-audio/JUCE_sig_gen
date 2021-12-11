/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             SimpleSynthNoiseTutorial
 version:          3.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple noise synthesiser.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

#include "GUI_Themes.h"
#include "SigGen.h"

//==============================================================================
class MainContentComponent   :  public juce::AudioAppComponent
{
public:
    MainContentComponent()
    {
        val_true.setValue(true);    //This is stupid. "val_true" is only used as a constant-true as consitional in button state check. There must be a jucier way to do this. 

        WhiteNoise_0.Mute(true);                  //Init Muted.
        sliderLevel = 0.1f;     //Default slider Level
        button_presses = 0;

        levelSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        levelSlider.setRange (0.0, 0.25);
        levelSlider.setValue (sliderLevel, juce::dontSendNotification);
        levelSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 100, 20);
        levelSlider.onValueChange = [this]
        {
            sliderLevel = (float) levelSlider.getValue();
            WhiteNoise_0.SetAmplitude(sliderLevel);
        };

        levelLabel.setText ("White Noise", juce::dontSendNotification);
        updateClickCountLabel(0);

        addAndMakeVisible (&levelSlider);
        addAndMakeVisible (&levelLabel);
        addAndMakeVisible (&clickCountLabel);
        
        //Add "Noise Active" Button:
        addAndMakeVisible(&noiseActiveButton);
        noiseActiveButton.onClick = [this]() { noiseActiveButtonClicked(); };   //attach click callback
        noiseActiveButton.setClickingTogglesState(true);                        //Enable Button Toggling
        noiseActiveButton.setToggleState(false, juce::dontSendNotification);    //Set initial Toggle State
        noiseActiveButtonClicked();                                             //Call the buttonClick() Function to initialise all states.
//        noiseActiveButton.setRepaintsOnMouseActivity(true);

        setSize (800, 200);

        setAudioChannels (0, 2);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int, double) override
    {
        resetParameters();
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto numSamplesRemaining = bufferToFill.numSamples;
        
        for (auto sample = 0; sample < numSamplesRemaining; ++sample)
        {
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel){
                //TODO: Sum and Mix all Generated Signals
                float output = WhiteNoise_0.getSample();
                bufferToFill.buffer->setSample (channel, sample, output);
            }
        }
    }

    void releaseResources() override {}

    void resized() override     //Called whenever the GUI Window is resized (including Initialization)
    {
        //Update any dynamic positions, dependent on window resizing
        const unsigned int noise_active_button_pos_y = getHeight() - 60;
        const unsigned int click_count_label_pos_y = noise_active_button_pos_y + GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT;
    
        //Redraw GUI Objects.
        levelLabel.setBounds (10, 10, 90, 20);
        levelLabel.setCentrePosition( GUI_Themes::THEME_CENTER_LINE, 20);
        
        levelSlider.setBounds (10, 40, 100, noise_active_button_pos_y - GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT - GUI_Themes::THEME_STANDARD_Y_SPACING - 15);
        
        clickCountLabel.setBounds( GUI_Themes::CLICK_COUNT_LABEL_POS_X, click_count_label_pos_y, 80, 80);
        clickCountLabel.setCentrePosition( GUI_Themes::THEME_CENTER_LINE, click_count_label_pos_y);
        
        noiseActiveButton.setBounds ( GUI_Themes::THEME_STANDARD_X_SPACING, noise_active_button_pos_y, GUI_Themes::NOISE_ACTIVE_BUTTON_WIDTH, GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT);
        noiseActiveButton.setCentrePosition( GUI_Themes::THEME_CENTER_LINE, noise_active_button_pos_y);
    }

    void resetParameters()
    {
    }
    
    void noiseActiveButtonClicked(){
        updateClickCountLabel(++button_presses);
    
        if( noiseActiveButton.getToggleStateValue() == val_true ){
            noiseActiveButton.setButtonText("Active");
            noiseActiveButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
            WhiteNoise_0.Mute(false);
            WhiteNoise_0.SetAmplitude(sliderLevel);  //Set Amplitude to Slider Value
//            DBG("White Noise ON");
        }else{
            WhiteNoise_0.Mute(true);
            noiseActiveButton.setButtonText("Muted");
            noiseActiveButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
//            DBG("White Noise OFF");
        }
    }

private:
    
    WhiteNoiseGen WhiteNoise_0;
    
    juce::Random random;
    juce::Slider levelSlider;
    juce::Label levelLabel;
    juce::TextButton noiseActiveButton;
    juce::Label clickCountLabel;
    juce::Value val_true;
    
    void updateClickCountLabel( unsigned int count ){
        clickCountLabel.setText( (juce::String("Presses: ")+juce::String(count)), juce::dontSendNotification);
    }
    
    float sliderLevel;
    unsigned int button_presses;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
