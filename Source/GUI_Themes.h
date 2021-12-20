//
//  GUI_Themes.h
//  NoiseGenerator - App
//
//  Created by Tom Wilson on 27/11/2021.
//
//  Refs:
//  - https://docs.juce.com/master/tutorial_component_parents_children.html
//

#pragma once

#include <JuceHeader.h>
#include "SigGen.h"

class GUI_Themes{
public:
    
    typedef struct coord_S{
        unsigned int x = 0, y = 0;
    }coord_t;
    
    //White Noise Channel
    static const unsigned int THEME_CENTER_LINE = 60;
    static const unsigned int THEME_STANDARD_X_SPACING = 10;        //e.g. space between buttons in a row.
    static const unsigned int THEME_STANDARD_Y_SPACING = 10;        //e.g. space between buttons in a column.
    static const unsigned int NOISE_ACTIVE_BUTTON_WIDTH = 60;
    static const unsigned int NOISE_ACTIVE_BUTTON_HEIGHT = 28;
    static const unsigned int NOISE_ACTIVE_BUTTON_POS_X = 10;
    static const unsigned int CLICK_COUNT_LABEL_POS_X = NOISE_ACTIVE_BUTTON_POS_X;
};

/*
 *  GUI Components.....
 */

//==============================================================================
class SigGenVoiceGUI : public juce::Component
{
public:
    
    typedef struct SliderRange_S{
        float min = 0.0, max = 0.0;
    }level_slider_range_t;
    
    typedef void (*activeButtonClicked_CB_t)( SigGenVoiceGUI* instance );
    
    typedef struct Config_S{
        level_slider_range_t level_slider_range = {0.0, 0.25};
        juce::Slider::SliderStyle level_slider_style = juce::Slider::SliderStyle::LinearVertical;   //default Vertical
        float slider_level = 0.0;
        std::string Title = "Sig Gen";
        activeButtonClicked_CB_t activeButtonClicked_CB = NULL;
    }config_t;
    
    SigGenVoiceGUI(){
//        instance_n = instance_count;
//        instance_count++;
    }
    
    void Init( config_t* _config )
    {
        config = *_config;       //Copy Config Variables
        
        levelSlider.setSliderStyle(config.level_slider_style);
        levelSlider.setRange (config.level_slider_range.min, config.level_slider_range.max);
        levelSlider.setValue (config.slider_level, juce::dontSendNotification);
        levelSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 100, 20);
//        levelSlider.addListener(Listener *listener)
        //TODO: Init config should take a user callback for onValue Change. This can be connected to the appropriate oscillator
//            levelSlider.onValueChange = [this]
//            {
//                sliderLevel = (float) levelSlider.getValue();
//                WhiteNoise_0.SetAmplitude(sliderLevel);
//            };
            
        levelLabel.setText (config.Title, juce::dontSendNotification);
        
        //Add "Noise Active" Button:
        noiseActiveButton.onClick = [this]() { noiseActiveButtonClicked(); };   //attach click callback
        noiseActiveButton.setClickingTogglesState(true);                        //Enable Button Toggling
        noiseActiveButton.setToggleState(false, juce::dontSendNotification);    //Set initial Toggle State
        noiseActiveButtonClicked();                                             //Call the buttonClick() Function to initialise all states.
//        noiseActiveButton.setRepaintsOnMouseActivity(true);
        
        addAndMakeVisible(levelSlider);
        addAndMakeVisible(levelLabel);
        addAndMakeVisible(noiseActiveButton);
    }
    
    ~SigGenVoiceGUI(){}     //TODO: Delete Graphical Components in Constructor.

    void paint (juce::Graphics& g) override {

    }
    
    void resized() override {
        const unsigned int noise_active_button_pos_y = getHeight() - 60;
        
        levelLabel.setBounds (10, 10, 90, 20);
        levelLabel.setCentrePosition( GUI_Themes::THEME_CENTER_LINE, 20);
        
        levelSlider.setBounds (10, 40, 100, noise_active_button_pos_y - GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT - GUI_Themes::THEME_STANDARD_Y_SPACING - 15);
        
        noiseActiveButton.setBounds ( GUI_Themes::THEME_STANDARD_X_SPACING, noise_active_button_pos_y, GUI_Themes::NOISE_ACTIVE_BUTTON_WIDTH, GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT);
        noiseActiveButton.setCentrePosition( GUI_Themes::THEME_CENTER_LINE, noise_active_button_pos_y);
    }
    
    juce::TextButton* getMuteButtonInstance(void){
        return &noiseActiveButton;
    }

private:
    config_t config;
//    static unsigned int instance_count;
    unsigned int instance_n;
    
    juce::Slider levelSlider;
    juce::Label levelLabel;
    juce::TextButton noiseActiveButton;
    
    static const unsigned int LEVEL_SLIDER_CENTER_LINE = 60;
    static const unsigned int X_SPACING = 10;        //e.g. space between buttons in a row.
    static const unsigned int Y_SPACING = 10;        //e.g. space between buttons in a column.
    static const unsigned int NOISE_ACTIVE_BUTTON_WIDTH = 60;
    static const unsigned int NOISE_ACTIVE_BUTTON_HEIGHT = 28;
    static const unsigned int NOISE_ACTIVE_BUTTON_POS_X = 10;
    static const unsigned int CLICK_COUNT_LABEL_POS_X = NOISE_ACTIVE_BUTTON_POS_X;
    
    
    //============================================================
    //Mouse Click Behaviour Handling.... (including User Callbacks)
    
    void noiseActiveButtonClicked( void )
    {
        bool state = false;     //passed to callBack;
        if( noiseActiveButton.getToggleStateValue() == true ){
            state = true;
            noiseActiveButton.setButtonText("Active");
            noiseActiveButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
                
//            WhiteNoise_0.Mute(false);
//            WhiteNoise_0.SetAmplitude(sliderLevel);  //Set Amplitude to Slider Value
//            DBG("White Noise ON");
        }else{
//            WhiteNoise_0.Mute(true);
            noiseActiveButton.setButtonText("Muted");
            noiseActiveButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
//            DBG("White Noise OFF");
        }
        
        //Execute User Callback
        if( config.activeButtonClicked_CB )
            config.activeButtonClicked_CB( this );

    }
    
    
    //==============================================================================
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SigGenVoiceGUI)
};


//==============================================================================
class SceneComponent    : public juce::Component
{
public:
    SceneComponent()
    {
        /*
         * Configure and Initialise SigGen GUI components
         */
        SigGenVoiceGUI::config_t whiteNoiseGUI_config;
        whiteNoiseGUI_config.slider_level = 0.0;
         
        /*
         *  Subscribe the Top-Level Scene as a listener to Buttons contained in Sub-Modules
         */
        for(int noise_gen = 0; noise_gen < N_NOISE_GENS; noise_gen++ ){
            
            switch( noise_gen ){
                case 0: whiteNoiseGUI_config.Title = "Noise 0"; break;
                case 1: whiteNoiseGUI_config.Title = "Noise 1"; break;
                default: whiteNoiseGUI_config.Title = "Noise N"; break;
            }
            
            whiteNoiseGUI[noise_gen].Init( &whiteNoiseGUI_config );
            addAndMakeVisible (whiteNoiseGUI[noise_gen] );
        }
    }
    
    /*
     *  Allows Audio Processing Modules to Listen to GUI Actions.
     */
    void AddListenerToAll_GUIActions( juce::Button::Listener* Listener ){
        //Noise Gens
        for(int noise_gen = 0; noise_gen < N_NOISE_GENS; noise_gen++ ){
            //Buttons
            whiteNoiseGUI[noise_gen].getMuteButtonInstance()->addListener(Listener);
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey);
    }

    void resized() override
    {
        printf("Top Level Scene Resized. %d x %d \r\n", getWidth(), getHeight());
        
        //TODO: Loop for N_whiteNoise. Configure their const Positioning in Themes Class.
        whiteNoiseGUI[0].setBounds(500, 10, 100, 250);
        whiteNoiseGUI[1].setBounds(600, 10, 100, 250);
    }
    
    /*
     *  Access to all GUI components.
     */
    SigGenVoiceGUI* GetWhiteNoiseGUI_Instance(unsigned int instance_n){
        if( instance_n >= N_NOISE_GENS )
            return NULL;
        
        return &whiteNoiseGUI[instance_n];
    }

private:
    
    enum { N_NOISE_GENS = 2 };
    SigGenVoiceGUI whiteNoiseGUI[N_NOISE_GENS];     //TODO: Change to std::vector so the Num Voices can be changed Dynamically
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComponent)
};
