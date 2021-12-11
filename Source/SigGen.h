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
    
    virtual ~SigGen(){};
    
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
    ~WhiteNoiseGen(){};
    
    float CalcSample() override
    {
        return random.nextFloat() * amplitude;;
    }
    
private:
    juce::Random random;
};
