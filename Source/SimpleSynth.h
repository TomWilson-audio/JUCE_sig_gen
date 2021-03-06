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

/*
 *  TODO:
 *      - The Whole Button/Slider Listener design isn't very scalable. It would be a lot cleaner if the Audio Sign Gen Objects were attached to GUI objects.
 *      - Set Up a CPU profiling mode for an excessive number of simultaneous Sines. This will allow comparison of sin(), LUT, CORDIC, Polynomial, Bhaskara etc... Sine methods...
 */


#pragma once

#include "GUI_Components.h"
#include "SigGen.h"

//==============================================================================
class MainContentComponent   :  public juce::AudioAppComponent
//                                public juce::Button::Listener,
//                                public juce::Slider::Listener
{
public:
    MainContentComponent()
    {
        addAndMakeVisible (&GUI_TopScene);     //Add Top Level, Parent Scene for the GUI

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
        
        /*
         * Attach Audio Objects to GUI Objects
         */
        GUI_TopScene.AttachAudioComponentToGuiComponent(&WhiteNoise_0, 0);
        
        for (unsigned int sine_osc_n = 0; sine_osc_n < N_SINE_WAVE_OSCS; sine_osc_n++ ){
            GUI_TopScene.AttachPeriodicAudioComponentToGuiComponent(&SineOscs[sine_osc_n], sine_osc_n + 1);
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
        setSize (1560, 512);
        
        static const float Base_Hz = 440.0;
        for (unsigned int sine_osc_n = 0; sine_osc_n < N_SINE_WAVE_OSCS; sine_osc_n++){
            SineOscs[sine_osc_n].Mute(true);
            SineOscs[sine_osc_n].SetSampleRate( sampleRate );
            SineOscs[sine_osc_n].SetFrequency(Base_Hz);
            SineOscs[sine_osc_n].SetAmplitude(0.1);
        }
        
        resetParameters();
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        auto numSamplesRemaining = bufferToFill.numSamples;
        
        for (auto sample = 0; sample < numSamplesRemaining; ++sample)
        {
            //TODO: Sum and Mix all Generated Signals
            float output = WhiteNoise_0.getSample();
            for( unsigned int osc_n = 0; osc_n < N_SINE_WAVE_OSCS; osc_n++){
                output+= SineOscs[osc_n].getSample();
            }
            
            for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel){
                bufferToFill.buffer->setSample (channel, sample, output);
            }
        }
    }

    void releaseResources() override {}

    void resized() override     //Called whenever the GUI Window is resized (including Initialization)
    {
        //Redraw GUI Window over MainComponent Window
        GUI_TopScene.setBounds(0, 0, getWidth(), getHeight());
    }

    void resetParameters()
    {
    }
    
private:
    SceneComponent GUI_TopScene;            //Absolute Top Level Scene for the Main Content Component
    
    static const unsigned int N_SINE_WAVE_OSCS = 9;
    WhiteNoiseGen WhiteNoise_0;
    SineWaveOscillator SineOscs[N_SINE_WAVE_OSCS];
    
    static const unsigned int N_SIG_GENS = 2; //TODO: There should be a Config Class that contains N_SIG Gens etc... so it can be reference by GUI and Audio System
  
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
