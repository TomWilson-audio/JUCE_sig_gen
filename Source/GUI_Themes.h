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
    
    typedef struct Config_S{
        level_slider_range_t level_slider_range = {0.0, 0.25};
        juce::Slider::SliderStyle level_slider_style = juce::Slider::SliderStyle::LinearVertical;   //default Vertical
        float slider_level = 0.0;
        std::string Title = "Sig Gen";
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
        levelSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 90, 20);
        levelSlider.onValueChange = [this]()
        {
            if(AudioComponent)
                AudioComponent->SetAmplitude((float)levelSlider.getValue());
            else
                printf("WARNING: Audio Component Not Attached to GUI\r\n");
        };
            
        levelLabel.setText (config.Title, juce::dontSendNotification);
        
        //Add "Noise Active" Button:
        noiseActiveButton.onClick = [this]() {      //attach click callback
            noiseActiveButtonClicked();
        };
        
        noiseActiveButton.setClickingTogglesState(true);                        //Enable Button Toggling
        noiseActiveButton.setToggleState(false, juce::dontSendNotification);    //Set initial Toggle State
        noiseActiveButtonClicked();                                             //Call the buttonClick() Function to initialise all states.
//        noiseActiveButton.setRepaintsOnMouseActivity(true);
        
        addAndMakeVisible(levelSlider);
        addAndMakeVisible(levelLabel);
        addAndMakeVisible(noiseActiveButton);
    }
    
    void AttachAudioComponent( SigGen* component ){
        AudioComponent = component;
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
    
    juce::Slider* getLevelSliderInstance(void){
        return &levelSlider;
    }

private:
    config_t config;
    
    juce::Slider levelSlider;
    juce::Label levelLabel;
    juce::TextButton noiseActiveButton;
    
    SigGen* AudioComponent = NULL;
    
    static const unsigned int LEVEL_SLIDER_CENTER_LINE = 60;
    static const unsigned int X_SPACING = 10;        //e.g. space between buttons in a row.
    static const unsigned int Y_SPACING = 10;        //e.g. space between buttons in a column.
    static const unsigned int NOISE_ACTIVE_BUTTON_WIDTH = 60;
    static const unsigned int NOISE_ACTIVE_BUTTON_HEIGHT = 28;
    static const unsigned int NOISE_ACTIVE_BUTTON_POS_X = 10;
    static const unsigned int CLICK_COUNT_LABEL_POS_X = NOISE_ACTIVE_BUTTON_POS_X;
    
    
    //============================================================
    //Mouse Click Behaviour Handling...
    void noiseActiveButtonClicked( void )
    {
        if( noiseActiveButton.getToggleStateValue() == true ){
            noiseActiveButton.setButtonText("Active");
            noiseActiveButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
            if(AudioComponent)
                AudioComponent->Mute(false);
        }else{
            noiseActiveButton.setButtonText("Muted");
            noiseActiveButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
            if(AudioComponent)
                AudioComponent->Mute(true);
        }
        if(AudioComponent == NULL)
            printf("WARNING: Audio Component Not Attached to GUI\r\n");
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
        whiteNoiseGUI_config.slider_level = 0.1;
         
        for(int noise_gen = 0; noise_gen < N_SIG_GEN_VOICE_GUIS; noise_gen++ ){
            
            switch( noise_gen ){
                case 0: whiteNoiseGUI_config.Title = "White Noise"; break;
                case 1: whiteNoiseGUI_config.Title = "Sig Gen[0]"; break;
                default: whiteNoiseGUI_config.Title = "Noise N"; break;
            }
            
            sigGenVoiceGUI[noise_gen].Init( &whiteNoiseGUI_config );
            addAndMakeVisible (sigGenVoiceGUI[noise_gen] );
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey);
        
        g.setColour (juce::Colours::lightgrey);
        g.setFont (juce::Font ("Times New Roman", 20.0f, juce::Font::italic));
        g.drawText ("Signal Sources", getLocalBounds(), juce::Justification::centredTop, true);
        
        //Basic Graphics Tutorial Gubbins (for Reference):
//        g.setColour (juce::Colours::green);
//        g.drawLine (10, 300, 590, 300, 5);
//
//        juce::Line<float> arrowLine(10, 350, 590, 350);
//        g.drawArrow( arrowLine, 5, 25, 25 );
//
//        juce::Rectangle<float> house (300, 120, 200, 170);
//        g.setColour (juce::Colours::blueviolet);
////        g.fillRect (house);
//        g.fillCheckerBoard (house, 30, 10, juce::Colours::sandybrown, juce::Colours::saddlebrown);
//
//        g.setColour (juce::Colours::yellow);
////        g.drawEllipse (getWidth() - 70, 10, 60, 60, 5);
//        g.fillEllipse (getWidth() - 70, 10, 60, 60);
//
//        g.setColour (juce::Colours::red);
//        juce::Path roof;
//        roof.addTriangle (300, 110, 500, 110, 400, 70);
//        g.fillPath (roof);
    }

    void resized() override
    {
        printf("Top Level Scene Resized. %d x %d \r\n", getWidth(), getHeight());
        
        //TODO: Loop for N_WhiteNoise. Configure their const Positioning in Themes Class.
        sigGenVoiceGUI[0].setBounds(10, 10, 100, 250);
        sigGenVoiceGUI[1].setBounds(100, 10, 100, 250);
    }
    
    /*
     *  Attach Audio Component to GUI Component
     */
    void AttachAudioComponentToGuiComponenet( SigGen* AudioComponent, const unsigned int Gui_index ){
//        if( Gui_index >= N_SIG_GEN_VOICE_GUIS )        //TODO: Assert, rather than return NULL
        sigGenVoiceGUI[Gui_index].AttachAudioComponent( AudioComponent );
    }

private:
    
    enum { N_SIG_GEN_VOICE_GUIS = 2 };
    SigGenVoiceGUI sigGenVoiceGUI[N_SIG_GEN_VOICE_GUIS];     //TODO: Change to std::vector so the Num Voices can be changed Dynamically
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComponent)
};
