/*
 *  @author:    Tom Wilson
 *  @date:      11/12/21
 *
 *  Signal Generator Classes for use with JUCE framework. 
 *
 *
 *  TODO:
 *  1) Templated Class to enable variable sample type (helpful for embedded 16-bit sigGens)
 *
 *  READING:
 *  1) Look into MinBLEPs https://www.experimentalscene.com/articles/minbleps.php . (Which would be a new class that inherits Noise Gen)
 */

#pragma once

class SigGen{
public:
    
    SigGen(){
        SigGenList[instance_count++] = this;
    }
    virtual ~SigGen(){}
    
    inline float GetSample( void ){
        UpdateAmplitude();              //TODO: This is too much overhead for GetSample()... especially containing an if... try with a mask. 
        return CalcSample();
    }
    
    void SetAmplitude(float value)
    {
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
    
    static SigGen* GetInstance( unsigned int n ){
        //TODO: Assert for n >= instance_count;
        //TODO: Assert for n >= MAX_N_SIGNALS
        return SigGenList[n];
    }
    
protected:
    inline virtual float CalcSample() = 0;         //Calc Sample is overriden and specialised for all signal types.
    
    static constexpr float PI = 3.141592653589793238L;
    static constexpr float TWO_PI = PI * 2;
    
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
    
    static const unsigned int MAX_N_SIGNALS = 256;
    static SigGen* SigGenList[MAX_N_SIGNALS];
    static unsigned int instance_count;
};
//Define Static Members
SigGen* SigGen::SigGenList[SigGen::MAX_N_SIGNALS] = {NULL};
unsigned int SigGen::instance_count = 0;

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
    
    void SetSampleRate( float rate )
    {
        if (fS == rate )
            return;
        fS = rate;
        SetFrequency( frequency );      //Change in Sample Rate will alter cycles per sample calc.
    }
    
    void SetFrequency(float f)
    {
        frequency = f;
        cyclesPerSample = frequency / (float)fS;
        angleDelta = cyclesPerSample * TWO_PI;
//        printf("SetFreq: CyclesPerSample = %f, angleDelta = %f\r\n", cyclesPerSample, angleDelta);
    }
    
protected:
    void updateAngle()
    {
        currentAngle += angleDelta;
        if (currentAngle >= TWO_PI)
            currentAngle = 0;
    }
    
    float fS = 48000;       //default to 48K.
    float frequency = 220.0;       //default frequency to 220Hz;
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
        if( currentAngle >= PI )    //TODO: you could add duty cycle control here.
            sample *= -1.0;
        updateAngle();
        return sample;
    }
    
private:
};

/*
 *  Wavetable Oscillator (Must provide a WaveTable to constructor)
 *  //TODO: Should inherit from "PeriodicOscillator"... except SetFreq would have to be overriden to account for Table Size
 */
class WavetableOscillator : public SigGen
{
public:
    
    WavetableOscillator (const juce::AudioSampleBuffer& wavetableToUse) //TODO: remove juce dependency by using std array/vector
        : wavetable (wavetableToUse),
          tableSize (wavetable.getNumSamples() - 1)
    {
        jassert (wavetable.getNumChannels() == 1);
    }
    ~WavetableOscillator(){}

    void SetFrequency (float frequency )
    {
        auto tableSizeOverSampleRate = (float) tableSize / fS;      //This can be calc'd with every change to table or fS. Not required here! Could be a lot of calls to setFreq with Freq Modulation!!!;
        tableDelta = frequency * tableSizeOverSampleRate;
    }
    
    void SetSampleRate( float rate )
    {
        if (fS == rate )
            return;
        fS = rate;
        SetFrequency( frequency );      //Change in Sample Rate will alter cycles per sample calc.
    }

    forcedinline float CalcSample() noexcept override
    {
        auto index0 = (unsigned int) currentIndex;
        auto index1 = index0 + 1;

        auto frac = currentIndex - (float) index0;

        auto* table = wavetable.getReadPointer (0);
        auto value0 = table[index0];
        auto value1 = table[index1];

        auto currentSample = value0 + frac * (value1 - value0);

        if ((currentIndex += tableDelta) > (float) tableSize)
            currentIndex -= (float) tableSize;

        return currentSample;
    }

private:
    const juce::AudioSampleBuffer& wavetable;           //TODO: remove juce dependency by using std array/vector
    const int tableSize;
    float currentIndex = 0.0f, tableDelta = 0.0f;
    float fS = 48000.0f;       //default to 48K;
    float frequency = 220.0f;
};
