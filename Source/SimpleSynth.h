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
class MainContentComponent   :  public juce::AudioAppComponent,
                                public juce::Button::Listener
{
public:
    MainContentComponent()
    {
        val_true.setValue(true);    //This is stupid. "val_true" is only used as a constant-true as consitional in button state check. There must be a jucier way to do this.
        
        addAndMakeVisible (&topScene);     //Add Top Level, Parent Scene for the GUI
        
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

        addAndMakeVisible (&levelSlider);
        addAndMakeVisible (&levelLabel);
        
        //Add "Noise Active" Button:
        addAndMakeVisible(&noiseActiveButton);
        noiseActiveButton.onClick = [this]() { noiseActiveButtonClicked(); };   //attach click callback
        noiseActiveButton.setClickingTogglesState(true);                        //Enable Button Toggling
        noiseActiveButton.setToggleState(false, juce::dontSendNotification);    //Set initial Toggle State
        noiseActiveButtonClicked();                                             //Call the buttonClick() Function to initialise all states.
//        noiseActiveButton.setRepaintsOnMouseActivity(true);

        setSize (1024, 512);
        
        /*
         *  Add GUI Listeners (Allowing Audio Processing to Action on any GUI changes!
         */
        topScene.AddListenerToAll_GUIActions(this);

        // Some platforms require permissions to open input channels so request that here
        if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
            && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
        {
            juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                               [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
        }
        else
        {
            // Specify the number of input and output channels that we want to open
            setAudioChannels (0, 2);
        }
        
        printf("MainContentComponent Constructor - SimpleSynth.h\r\n");
    }

    ~MainContentComponent() override
    {
        printf("\r\nSHUTTING DOWN\r\n");
        shutdownAudio();
    }

    void prepareToPlay (int, double sampleRate) override
    {
        printf("\r\nPrepare To Play: SR = %f\r\n", sampleRate);
        
        //TODO: Set (or update) SampleRate For All Oscillators
        SineOsc_0.SetSampleRate( sampleRate );
        SineOsc_0.SetFrequency(220.0);
        SineOsc_0.SetAmplitude(0.1);
        
        resetParameters();
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto numSamplesRemaining = bufferToFill.numSamples;
        
        for (auto sample = 0; sample < numSamplesRemaining; ++sample)
        {
            //TODO: Sum and Mix all Generated Signals
            float output = WhiteNoise_0.getSample() + SineOsc_0.getSample();
            
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel){
                bufferToFill.buffer->setSample (channel, sample, output);
            }
        }
    }

    void releaseResources() override {}

    void resized() override     //Called whenever the GUI Window is resized (including Initialization)
    {
        printf("RESIZE!!!\r\n");
        
        topScene.setBounds(0, 0, getWidth(), getHeight());
        
        //Update any dynamic positions, dependent on window resizing
        const unsigned int noise_active_button_pos_y = getHeight() - 60;

        //Redraw GUI Objects.
        levelLabel.setBounds (10, 10, 90, 20);
        levelLabel.setCentrePosition( GUI_Themes::THEME_CENTER_LINE, 20);
        
        levelSlider.setBounds (10, 40, 100, noise_active_button_pos_y - GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT - GUI_Themes::THEME_STANDARD_Y_SPACING - 15);
        
        noiseActiveButton.setBounds ( GUI_Themes::THEME_STANDARD_X_SPACING, noise_active_button_pos_y, GUI_Themes::NOISE_ACTIVE_BUTTON_WIDTH, GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT);
        noiseActiveButton.setCentrePosition( GUI_Themes::THEME_CENTER_LINE, noise_active_button_pos_y);
    }
    
    //==============================================================================
    /*
     *  Also Called on Resize
     */
    void paint (juce::Graphics& g) override
    {
        printf("PAINT!!!\r\n");
        // (Our component is opaque, so we must completely fill the background with a solid colour)
//        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
        g.fillAll (juce::Colours::darkgrey);
     
        g.setColour (juce::Colours::lightgrey);
        g.setFont (juce::Font ("Times New Roman", 20.0f, juce::Font::italic));
        g.drawText ("Signal Sources", getLocalBounds(), juce::Justification::centredTop, true);
        
        
        //Basic Graphics Tutorial Gubbins (for Reference):
        g.setColour (juce::Colours::green);
        g.drawLine (10, 300, 590, 300, 5);
        
        juce::Line<float> arrowLine(10, 350, 590, 350);
        g.drawArrow( arrowLine, 5, 25, 25 );
        
        juce::Rectangle<float> house (300, 120, 200, 170);
        g.setColour (juce::Colours::blueviolet);
//        g.fillRect (house);
        g.fillCheckerBoard (house, 30, 10, juce::Colours::sandybrown, juce::Colours::saddlebrown);
        
        g.setColour (juce::Colours::yellow);
//        g.drawEllipse (getWidth() - 70, 10, 60, 60, 5);
        g.fillEllipse (getWidth() - 70, 10, 60, 60);
        
        g.setColour (juce::Colours::red);
        juce::Path roof;
        roof.addTriangle (300, 110, 500, 110, 400, 70);
        g.fillPath (roof);
    }
    
    //Handles all Button Clicks from the GUI, so they can be applied to audio Processing
    void buttonClicked (juce::Button* button) override
    {
        if(button == topScene.GetWhiteNoiseGUI_Instance(0)->getMuteButtonInstance()){
            //TODO: This is a quick test of the button listeners. White Noise GUI within Simple Synth Needs replacing with modular design.
            printf("AUDIO PROCESSING: Mute[0] Listener Called\r\n");
            const juce::Button* pressedButton = topScene.GetWhiteNoiseGUI_Instance(0)->getMuteButtonInstance();     //FYI: for code cleanliness
            
            //TEST: ECHO Button Press Functionality.
            noiseActiveButton.setToggleState(pressedButton->getToggleState(), juce::dontSendNotification);
            noiseActiveButtonClicked();
        }
        else if(button == topScene.GetWhiteNoiseGUI_Instance(0)->getMuteButtonInstance()){
            printf("AUDIO PROCESSING: Mute[1] Listener Called\r\n");
        }
    }

    void resetParameters()
    {
    }
    
    void noiseActiveButtonClicked()
    {
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
    SceneComponent topScene;            //Absolute Top Level Scene for the Main Content Component
    
    WhiteNoiseGen WhiteNoise_0;
    SineWaveOscillator SineOsc_0;
    
    juce::Slider levelSlider;
    juce::Label levelLabel;
    juce::TextButton noiseActiveButton;
    juce::Value val_true;
    
    GUI_Themes::coord_t gui_component_pos_0 = {100, 10};
    
    float sliderLevel;
    unsigned int button_presses;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
