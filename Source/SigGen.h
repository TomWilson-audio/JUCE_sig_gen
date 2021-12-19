/*
 *  @author:    Tom Wilson
 *  @date:      11/12/21
 *
 *  Signal Generator Classes for use with JUCE framework. 
 *
 *
 *  TODO:
 *  1) Templated Class to enable variable sample type (helpful for embedded 16-bit sigGens)
 */

#pragma once

#include <JuceHeader.h>

class SigGen{
public:
    
    SigGen() {}
    virtual ~SigGen(){}
    
    virtual float CalcSample() = 0;         //Calc Sample is specialised for all signal types.
    
    float getSample( void ){
        UpdateAmplitude();
        return CalcSample();
    }
    
    void SetAmplitude(float value)
    {
        if( value == targetAmplitude )
            return;
        
        if( muted ){
            unmutedAmplitude = value;
            return;
        }
        
        SetTargetAmplitude(value);
    }
    
    /*  //Used to attach LFOs and other control signals that need to bypass amplitude ramping
     *  constexpr inline void ModulateAmplitude(float modify){
     *      amplitude += modify;
     *  }
     */
    
    void Mute( bool state )
    {
        muted = state;
        if(state){
            unmutedAmplitude = targetAmplitude;
            SetTargetAmplitude(0.00);
        }else{
            SetTargetAmplitude(unmutedAmplitude);
        }
    }
    
protected:
    float targetAmplitude = 0.0f;
    float amplitude = 0.0f;
    float unmutedAmplitude = 0.0f;
    float amplitudeFadeStep = 0.0f;
    static constexpr unsigned int AMPLITUDE_RAMP_LENGTH_SAMPLES = 512;
    unsigned int rampRemainingSamples = 0;
    bool muted = false;
    
    constexpr inline void UpdateAmplitude(void){
        if(rampRemainingSamples){
            rampRemainingSamples--;
            amplitude += amplitudeFadeStep;
        }
    }
    
private:
    constexpr inline void SetTargetAmplitude( const float value ){
        targetAmplitude = value;
        amplitudeFadeStep = (targetAmplitude - amplitude) / (float) AMPLITUDE_RAMP_LENGTH_SAMPLES;
        rampRemainingSamples = AMPLITUDE_RAMP_LENGTH_SAMPLES;
    }
};

class WhiteNoiseGen : public SigGen
{
public:
    WhiteNoiseGen(){}
    ~WhiteNoiseGen(){}
    
    float CalcSample() override
    {
        return amplitude * random.nextFloat();
    }
    
private:
    juce::Random random;
};

class PeriodicOscillator : public SigGen
{
public:
    PeriodicOscillator(){}
    ~PeriodicOscillator(){}
    
    void SetSampleRate( float rate ){
        fS = rate;
    }
    
    void SetFrequency(float f)
    {
        cyclesPerSample = f / (float)fS;
        angleDelta = cyclesPerSample * juce::MathConstants<float>::twoPi;
    }
    
    void updateAngle()
    {
        currentAngle += angleDelta;
        if (currentAngle >= juce::MathConstants<float>::twoPi)
            currentAngle = 0;
    }
    
protected:
    float fS = 48000;       //default to 48K.
    float cyclesPerSample;
    float currentAngle = 0.0, angleDelta = 0.0;
    
private:

};

class SineWaveOscillator : public PeriodicOscillator
{
public:
    SineWaveOscillator(){}
    ~SineWaveOscillator(){}
    
    float CalcSample() override
    {
        auto currentSample = std::sin(currentAngle);
        updateAngle();
        return amplitude * currentSample;
    }
    
private:

};

class SquareWaveOscillator : public PeriodicOscillator      
{
public:
    SquareWaveOscillator(){}
    ~SquareWaveOscillator(){}
    
    float CalcSample() override
    {
        float sample = amplitude * 0.5;
        if( currentAngle >= juce::MathConstants<float>::pi )    //TODO: you could add duty cycle control here.
            sample *= -1.0;
        updateAngle();
        return sample;
    }
    
private:
};
