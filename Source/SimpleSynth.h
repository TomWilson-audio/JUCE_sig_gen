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
 *      - FM and AM modulation controls for Audio Objects.
 */


#pragma once

#include "GUI_Components.h"
#include "SigGen.h"

#define RUN_SIG_GEN_CPU_TEST              //TODO: Create a class called TestOscillators, this can encapsulate the whole CPU test (and other tests!)


//==============================================================================
class MainContentComponent   :  public juce::AudioAppComponent,
                                public juce::Timer
{
public:
    MainContentComponent()
    {
        printf("\r\n******** MainComponent Constructor Called *********\r\n");
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
        
#ifdef RUN_SIG_GEN_CPU_TEST
        GUI_TopScene.SetMaxNumTestOscs( MAX_N_CPU_TEST_OSCS );
        GUI_TopScene.DisplayCpuTestGUI(true);
        startTimer (100);        //Run CPU timer.
#endif
        
        printf("\r\n******** MainComponent Constructor COMPLETED *********\r\n");     //NOTE: The constructor seems to be interrupted by Audio Function Calls
    }

    ~MainContentComponent() override
    {
        printf("\r\nSHUTTING DOWN\r\n");
        shutdownAudio();
    }

    //WARNING: prepareToPlay() gets called before MainContentComponent() constructor has completed execution. Presumably from a System interrupt.
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        printf("\r\nPrepare To Play: SR = %f. Expected Samples Per Block = %d\r\n", sampleRate, samplesPerBlockExpected );
        
        //TODO: Set (or update) SampleRate For All Oscillators
        WhiteNoise_0.Mute(true);                    //Init Muted.
        WhiteNoise_0.SetAmplitude(0.1);             //Init Level.
        setSize (1560, 512);
        
        static const float Base_Hz = 220.0;
        for (unsigned int sine_osc_n = 0; sine_osc_n < N_SINE_WAVE_OSCS; sine_osc_n++)
        {
            SineOscs[sine_osc_n].Mute(true);
            SineOscs[sine_osc_n].SetSampleRate( sampleRate );
            SineOscs[sine_osc_n].SetFrequency(Base_Hz);
            SineOscs[sine_osc_n].SetAmplitude(0.1);
        }
        
        createWavetable();
        
        //Wavetable Osc Test:
        auto* WT_oscillator = new WavetableOscillator (sineTable);
        WT_oscillator->SetSampleRate(sampleRate);
        WT_oscillator->SetFrequency ((float) 220.0);
        WT_oscillator->SetAmplitude(0.2f);
        WT_oscillators.add (WT_oscillator);
        
        
#ifdef RUN_SIG_GEN_CPU_TEST
        SineWaveOscillator CPU_TestOscs[MAX_N_CPU_TEST_OSCS];

        static const float testHz_base = 40.0;
        static const float testHz_range = 2000.0;
        static const float testHz_increment = testHz_range / (float)MAX_N_CPU_TEST_OSCS;
        for (unsigned int testOsc_n = 0; testOsc_n < MAX_N_CPU_TEST_OSCS; testOsc_n++ )
        {
            CpuTestOscillators.push_back(CPU_TestOscs[testOsc_n]);
            
            CpuTestOscillators.at(testOsc_n).Mute(false);
            CpuTestOscillators.at(testOsc_n).SetSampleRate( sampleRate );
            CpuTestOscillators.at(testOsc_n).SetFrequency( testHz_base + (testOsc_n * testHz_increment) );
            CpuTestOscillators.at(testOsc_n).SetAmplitude(0.0005);
        }
#endif
        
        resetParameters();
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();
        
        auto numSamplesRemaining = bufferToFill.numSamples;
        
        for (auto sample = 0; sample < numSamplesRemaining; ++sample)
        {
            float output = WhiteNoise_0.GetSample();
            for( unsigned int osc_n = 0; osc_n < N_SINE_WAVE_OSCS; osc_n++){
                output+= SineOscs[osc_n].GetSample();
            }
            
//          ************* WaveTable Osc Test: ****************
            output += WT_oscillators.getUnchecked(0)->GetSample();
            /////////////////////////////////////////////////////////////
            
#ifdef RUN_SIG_GEN_CPU_TEST
            if( GUI_TopScene.GetOscCpuTestState() ){
                unsigned int n_testOscs = GUI_TopScene.GetNumTestOscs();
                if (n_testOscs > MAX_N_CPU_TEST_OSCS ) n_testOscs = MAX_N_CPU_TEST_OSCS;
                for( unsigned int osc_n = 0; osc_n < n_testOscs; osc_n++)
                {
                    output+= CpuTestOscillators.at(osc_n).GetSample();
                }
            }
#endif
            
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
    
    //CPU Timer Callback
    void timerCallback() override
    {
        auto cpu = deviceManager.getCpuUsage() * 100;
        GUI_TopScene.SetCpuUsagePercentage( std::to_string(cpu) + "%");
    }
    
private:
    SceneComponent GUI_TopScene;            //Absolute Top Level Scene for the Main Content Component
    
    static const unsigned int N_SINE_WAVE_OSCS = 9;
    WhiteNoiseGen WhiteNoise_0;
    SineWaveOscillator SineOscs[N_SINE_WAVE_OSCS];
    
    //Wavetable Testing
    const unsigned int tableSize = 1 << 7;
    juce::AudioSampleBuffer sineTable;
    juce::OwnedArray<WavetableOscillator> WT_oscillators;
    
    std::vector<SineWaveOscillator> CpuTestOscillators;
    
    //Vague Benchmark: This Macbook Pro cannot handle 400 * SigGens with calls to sin().. Let's compare to interpolated.
    enum{
        MAX_N_CPU_TEST_OSCS = 250,
    };
    
    void createWavetable()
    {
        sineTable.setSize (1, (int) tableSize);
        auto* samples = sineTable.getWritePointer (0);
 
        auto angleDelta = juce::MathConstants<double>::twoPi / (double) (tableSize - 1);
        auto currentAngle = 0.0;
    
        for (unsigned int i = 0; i < tableSize; ++i)
        {
            auto sample = std::sin (currentAngle);
            samples[i] = (float) sample;
            currentAngle += angleDelta;
        }
    }
  
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
