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
                                public juce::Button::Listener,
                                public juce::Slider::Listener
{
public:
    MainContentComponent()
    {
        addAndMakeVisible (&topScene);     //Add Top Level, Parent Scene for the GUI
        
        /*
         *  Add GUI Listeners (Allowing Audio Processing to Action on any GUI changes)
         */
        topScene.AddListenerToAll_GUIActions(this, this);

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
        WhiteNoise_0.Mute(true);                    //Init Muted.
        WhiteNoise_0.SetAmplitude(0.1);             //Init Level.
        setSize (1024, 512);
        
        SineOsc_0.Mute(true);
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
        //Redraw GUI Window over MainComponent Window
        topScene.setBounds(0, 0, getWidth(), getHeight());
    }
    
    //Handles all Button Clicks from the GUI, so they can be applied to audio Processing
    void buttonClicked (juce::Button* button) override
    {
        for( unsigned int signal = 0; signal < N_SIG_GENS; signal++ )       //Iterate Through All Signals
        {
            //Test For MUTE Buttons (TODO: we might be able to skip this looped conditional by looking at the argument button type or name )
            if(button == topScene.GetSigGenGUI_Instance(signal)->getMuteButtonInstance())
            {
                if(topScene.GetSigGenGUI_Instance(signal)->getMuteButtonInstance()->getToggleStateValue() == true)
                    SigGen::GetInstance(signal)->Mute(false);
                else
                    SigGen::GetInstance(signal)->Mute(true);
            }
            //Add Tests for Other Button Types. E.g. Waveform Select
        }
    }
    
    //Subscribed to All Slider Value Changes from the GUI, so they can be applied to audio Processing
    void sliderValueChanged (juce::Slider* slider) override
    {
        for( unsigned int signal = 0; signal < N_SIG_GENS; signal++ )       //Iterate Through All Signals
        {
            if(slider == topScene.GetSigGenGUI_Instance(signal)->getLevelSliderInstance()){
//                printf("Slider [%d] Val = %f\r\n", signal, slider->getValue());
                SigGen::GetInstance(signal)->SetAmplitude(slider->getValue());
            }
            //TODO: Check Frequency Slider Value... You will also need to test whether Signal Type has a Frequency Control (e.g. NOISE Gen)
        }
    }

    void resetParameters()
    {
    }
    
private:
    SceneComponent topScene;            //Absolute Top Level Scene for the Main Content Component
    
    WhiteNoiseGen WhiteNoise_0;
    SineWaveOscillator SineOsc_0;
    
    static const unsigned int N_SIG_GENS = 2; //TODO: There should be a Config Class that contains N_SIG Gens etc... so it can be reference by GUI and Audio System
  
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
