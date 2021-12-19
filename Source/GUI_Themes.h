//
//  GUI_Themes.h
//  NoiseGenerator - App
//
//  Created by Tom Wilson on 27/11/2021.
//

#pragma once

#ifndef GUI_Themes_h
#define GUI_Themes_h

#include <JuceHeader.h>

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
    
    //Sine Wave 1 Label
    
    
    //SigGen Voice GUI Object (With Variable Configurations)
    class SigGenerator{
    public:
        
//        SigGenerator( coord_t _position):position{_position}{}
        SigGenerator(){}
        ~SigGenerator(){}
        
        typedef struct SliderRange_S{
            float min = 0.0, max = 0.0;
        }slider_range_t;
        
        //TODO:
        //Config also needs to contain Styles. e.g. This is a Noise/Signal generator with controls x/y/z
        typedef struct InitConfig_S{
            slider_range_t level_slider_range = {0.0, 0.25};
            juce::Slider::SliderStyle level_slider_style = juce::Slider::SliderStyle::LinearVertical;   //default Vertical
            float slider_level = 0.0;
            
        }init_config_t;
        
        void Init( init_config_t* config ){
            
            levelSlider.setSliderStyle(config->level_slider_style);
            levelSlider.setRange (config->level_slider_range.min, config->level_slider_range.max);
            levelSlider.setValue (config->slider_level, juce::dontSendNotification);
            levelSlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 100, 20);
            //TODO: Init should take a user callback for onValue Change. This can be connected to the appropriate oscillator
//            levelSlider.onValueChange = [this]
//            {
//                sliderLevel = (float) levelSlider.getValue();
//                WhiteNoise_0.SetAmplitude(sliderLevel);
//            };
            levelLabel.setText ("White Noise", juce::dontSendNotification);

//            TODO: can't add these here because of dependency on JUCE component... find workaround....
//            addAndMakeVisible (&levelSlider);       //Dependency: juce::AudioAppComponent
//            addAndMakeVisible (&levelLabel);
//            addAndMakeVisible(&noiseActiveButton);
            
            //TODO: Add Callback for mute button
//            noiseActiveButton.onClick = [this]() { noiseActiveButtonClicked(); };   //attach click callback
            noiseActiveButton.setClickingTogglesState(true);                        //Enable Button Toggling
            noiseActiveButton.setToggleState(false, juce::dontSendNotification);    //Set initial Toggle State
//            noiseActiveButtonClicked();                                             //Call the buttonClick() Function to initialise all states.
        }
        
        void SetLevelSlider( float val ){
            levelSlider.setValue( val, juce::dontSendNotification );
        }
        
        /*
         * To be called whenever GUI window is resized (or initialised) so that components can be scaled and repositioned.
         */
        void Resize( unsigned int window_height, unsigned int window_width ){
            //Update any dynamic positions, dependent on window resizing
            const unsigned int noise_active_button_pos_y = window_height - 60;

            //Redraw GUI Objects.
            levelLabel.setBounds (100, 10, 90, 20);
            levelLabel.setCentrePosition( GUI_Themes::THEME_CENTER_LINE + 100, 20);
            
            levelSlider.setBounds (100, 40, 100, noise_active_button_pos_y - GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT - GUI_Themes::THEME_STANDARD_Y_SPACING - 15);
            
            noiseActiveButton.setBounds ( GUI_Themes::THEME_STANDARD_X_SPACING + 100, noise_active_button_pos_y, GUI_Themes::NOISE_ACTIVE_BUTTON_WIDTH, GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT);
            noiseActiveButton.setCentrePosition( GUI_Themes::THEME_CENTER_LINE + 100, noise_active_button_pos_y);
        }
        
    private:
        coord_t position;
        juce::Slider levelSlider;
        juce::Label levelLabel;
        juce::TextButton noiseActiveButton;
        juce::Value val_true;
    };
    
};


#endif /* GUI_Themes_h */
