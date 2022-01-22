//
//  GUI_Themes.h
//  NoiseGenerator - App
//
//  Created by Tom Wilson on 27/11/2021.
//
//  Refs:
//  - https://docs.juce.com/master/tutorial_component_parents_children.html
//
//  TODO:
//  - Create an X-Y 2D control for SigGen Pitch(X) and Amplitude(Y). This will be a more intuitive control and should make the GUI more like a creative instrument than a SigGen Utility.
//  Some relevant notes on this forum post: https://forum.juce.com/t/help-a-newbie-2d-slider-x-y-controller/20832
//
//  - It would be much cleaner with a Base class for "SigGenGUI", where Periodic, Noise, are derived classes.
//
//

#pragma once

#include <JuceHeader.h>
#include "SigGen.h"
#include "ObjectList.h"
#include "stdio.h"

//#define DEBUG_REPORT_MOUSE_POSITION

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
    static const unsigned int NOISE_ACTIVE_BUTTON_HEIGHT = 25;
    static const unsigned int NOISE_ACTIVE_BUTTON_POS_X = 10;
    static const unsigned int CLICK_COUNT_LABEL_POS_X = NOISE_ACTIVE_BUTTON_POS_X;
};

/*
 *  GUI Components.....
 */

//==============================================================================
class SigGenVoiceGUI :  public juce::Component,
                        public ObjectList<SigGenVoiceGUI>
{
public:
    
    unsigned int ComponentWidth = 140;      //This value will be modified depending on the GUI configuration.
    unsigned int ComponentHeight = 300;
    
    typedef struct SliderRange_S{
        float min = 0.0, max = 0.0;
    }level_slider_range_t;
    
    typedef enum{
        SIG_GEN_GUI_TYPE_NOISE,
        SIG_GEN_GUI_TYPE_PERIODIC,
        N_SIG_GEN_GUI_TYPES,
    }sig_gen_gui_type_t;
    
    typedef struct Config_S{
        sig_gen_gui_type_t gui_type = SIG_GEN_GUI_TYPE_NOISE;           //Noise type by default (no freq control)
        level_slider_range_t level_slider_range = {0.0, 0.25};
        juce::Slider::SliderStyle level_slider_style = juce::Slider::SliderStyle::LinearVertical;   //default Vertical
        float slider_level = 0.0;
        std::string Title = "Sig Gen";
    }config_t;
    
    SigGenVoiceGUI(){
        AddObjectToList(this);
        if( GetObjectInstanceCount() == 1 ){    //First Sig gen GUI Object
            for( unsigned int type = 0; type < N_SIG_GEN_GUI_TYPES; type++ ){
                objectInstanceCounts.count_per_type[type] = 0;      //Zero All Counters for Each Type (They are incremented during config)
            }
        }
    }
    ~SigGenVoiceGUI(){}
    
    ///TODO: The config_t arg could be simplified just to expose sig_gen_gui_type_t and title. All other stylistic traits should be configured internally.
    void Init( config_t* _config )
    {
        //With ref to above TODO, we could have a function like SetConfigFromGuiType... Which will also come in useful when changing GUI type at runtime. 
        config = *_config;       //Copy Config Variables
        
        objectInstanceCounts.count_per_type[config.gui_type]++;     //Increment the GUI Type Counts.
        
        //Add Common Components
        AddLevelControl();       //TODO: It would be a lot cleaner if these took Args, rather than using the global config data;
        AddLabels();
        AddMuteButton();
        
        switch( config.gui_type ){
            case SIG_GEN_GUI_TYPE_NOISE:
                ComponentWidth = 100;                   //Narrower Component for Noise GUI
                break;
            case SIG_GEN_GUI_TYPE_PERIODIC:
                //Default the first Periodic Sig Gen GUI as the Sync Talker
                if( objectInstanceCounts.count_per_type[SIG_GEN_GUI_TYPE_PERIODIC] == 1)
                    SetAsSyncTalker();
                
                freqGUI_instance_n = objectInstanceCounts.count_per_type[SIG_GEN_GUI_TYPE_PERIODIC] - 1;
                AddFrequencyControl();
                break;
            default: break;
        }
    }
    
    void AttachAudioComponent( SigGen* component ){
        AudioComponent = component;
    }
    
    void AttachAudioComponent_Periodic( PeriodicOscillator* component){
        AudioComponent = component;             //Add Base Class Pointer for Amplitude Control
        AudioComponent_periodic = component;    //Add Derived Class Pointer for Periodic Controls.
    }
    
    //Set The Sync State of the GUI. i.e. Is this SigGen Synced to the f0 of it's voice group.
    
    //configures this instance as the Sync Talker for it's sync group
    void SetAsSyncTalker( void ){
       
        SetSyncState(true, false);
        
        //Test for All SigGenGuiInstances, ensuring all other members of this sync group are not set to talker.
        printf("%s - SET AS SYNC TALKER for Sync Group %d\r\n", config.Title.c_str(), syncSettings.syncGroup );
        for( unsigned int i = 0; i < GetObjectInstanceCount(); i++ ){
            if(( GetObjectFromList(i)->syncSettings.syncGroup == syncSettings.syncGroup ) &&   //matching syncGroup
               ( GetObjectFromList(i)->syncSettings.isSyncTalker == true) &&                   //isCurrently Talker
               ( GetObjectFromList(i) != this )                                                //is NOT this instance
            ){
                //Redraw gui as a sync Listener
                GetObjectFromList(i)->SetSyncState(false, true);      //TODO: refactor with "sync listener" terminology;
            }
        }
    }
    
    /*
     * For a given sync group, set the talker frequency and update all listeners.
     */
    void SetSyncGroupTalkerFrequency( float freq )
    {
        if(!AudioComponent_periodic)
            return;
            
        AudioComponent_periodic->SetFrequency(freq);    //Set Talker Freq
        
        //Update any sync'd members of the group:
        for( unsigned int i = 0; i < GetObjectInstanceCount(); i++ ){
            if(( GetObjectFromList(i)->syncSettings.syncGroup == syncSettings.syncGroup ) &&    //matching syncGroup
               ( GetObjectFromList(i)->syncSettings.isSynced == true) &&                        //isSynced
               ( GetObjectFromList(i)->syncSettings.isSyncTalker == false ) &&
               ( GetObjectFromList(i) != this )                                                 //is NOT this instance
            ){
                GetObjectFromList(i)->SetSyncListenerFrequencyRelativeToGroupTalker();
            }
        }
        
        //Update Freq Label
        frequencyLabel.setText("F: " + std::to_string(freq), juce::dontSendNotification);
    }
    
    void SetSyncListenerFrequencyRelativeToGroupTalker( void )
    {
        if( !AudioComponent_periodic )
            return;
        
        if( syncSettings.isSyncTalker || !syncSettings.isSynced ){
            printf("WARNING: Attemting to change Listener freq of non-listener GUI component\r\n");
            return;
        }

        const float freq = GetSyncGroupTalkerFrequency() * frequencySlider.getValue();//When configured as listener, freqSlider is relative to talker freq.
        AudioComponent_periodic->SetFrequency(freq);
        
        frequencyLabel.setText("F: " + std::to_string(freq), juce::dontSendNotification);
            
    }
    
    float GetSyncGroupTalkerFrequency( void )
    {
        float freq = 440.0;
        for( unsigned int i = 0; i < GetObjectInstanceCount(); i++ ){
            if(( GetObjectFromList(i)->syncSettings.syncGroup == syncSettings.syncGroup ) &&    //matching syncGroup
               ( GetObjectFromList(i)->syncSettings.isSyncTalker == true) ){                      //isCurrently Talker
                freq = GetObjectFromList(i)->frequencySlider.getValue();
                break;
            }
        }
        return freq;
    }
        
    void SetSyncState( bool isTalker, bool isSynced )
    {
        if (isTalker) isSynced = false;     //Prevent incorrect setting of Talker Sync'd to itself;
        
        syncSettings.isSyncTalker = isTalker;
        syncSettings.isSynced = isSynced;
        syncButton.setClickingTogglesState (isSynced);
        syncButtonClicked();
    }

    void paint (juce::Graphics& g) override {
        //Draw Bounding Box
        if( syncSettings.isSyncTalker )
            g.setColour (juce::Colours::red);
        else
            g.setColour (juce::Colours::darkturquoise);
        
        g.drawRoundedRectangle(0, 0, getWidth(), getHeight(), 10, 2);
    }
    
    void resized() override {
//        const unsigned int noise_active_button_pos_y = getHeight() - 60;
        const unsigned int X_CentreLine = getWidth() >> 1;
        const unsigned int X_ThirdLine = getWidth() * 0.333333;
        
        titleLabel.setBounds (10, 10, 90, 20);
        titleLabel.setCentrePosition( X_CentreLine, 14);
        titleLabel.setJustificationType( juce::Justification::centredTop );
        
        if( config.gui_type == SIG_GEN_GUI_TYPE_PERIODIC ){
            levelSlider.setBounds (10, 40, 100, 150);
            levelSlider.setCentrePosition( X_ThirdLine, 120);
            frequencySlider.setBounds( 50, 20, 100, 150);
            frequencySlider.setCentrePosition( X_ThirdLine << 1, 100);
            frequencyLabel.setBounds( 10, 200, 100, 30 );
            
            syncButton.setBounds( 0, 240, 30, 30);
            syncButton.changeWidthToFitText();
            
        }else if( config.gui_type == SIG_GEN_GUI_TYPE_NOISE ){
            levelSlider.setBounds (10, 40, 100, 150);
            levelSlider.setCentrePosition( X_CentreLine, 120);
        }
        
        noiseActiveButton.setBounds ( GUI_Themes::THEME_STANDARD_X_SPACING, 0, GUI_Themes::NOISE_ACTIVE_BUTTON_WIDTH, GUI_Themes::NOISE_ACTIVE_BUTTON_HEIGHT);
        noiseActiveButton.setCentrePosition( X_CentreLine, 280);
        
    }
    
    /*
     *  Mouse Move Used to return Co-ords to ease GUI layout.
     */
#ifdef DEBUG_REPORT_MOUSE_POSITION
    void mouseMove(const juce::MouseEvent& event) override
    {
        printf("SigGen Mouse Coord (%f, %f)\r\n", event.position.getX(), event.position.getY());
    }
#endif

private:
    
    config_t config;
    
    typedef struct ObjectInstanceCounts_S{
        unsigned int count_per_type[N_SIG_GEN_GUI_TYPES];
    }object_instance_counts_t;
    static object_instance_counts_t objectInstanceCounts;
    int freqGUI_instance_n = 0;
    
    juce::Slider levelSlider;
    juce::Slider frequencySlider;
    juce::Label frequencyLabel;
    juce::Label titleLabel;
    juce::TextButton noiseActiveButton;
    juce::ToggleButton syncButton;
    
    SigGen* AudioComponent = NULL;
    PeriodicOscillator* AudioComponent_periodic = NULL;
    
    static const unsigned int LEVEL_SLIDER_CENTER_LINE = 60;
    static const unsigned int X_SPACING = 10;        //e.g. space between buttons in a row.
    static const unsigned int Y_SPACING = 10;        //e.g. space between buttons in a column.
    static const unsigned int NOISE_ACTIVE_BUTTON_WIDTH = 60;
    static const unsigned int NOISE_ACTIVE_BUTTON_HEIGHT = 20;
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
    
    void AddLabels( void )
    {
        //Title and Other Text Labels
        titleLabel.setText (config.Title, juce::dontSendNotification);
        addAndMakeVisible(titleLabel);
    }
    
    void AddLevelControl( void )
    {
        //Level Slider
        levelSlider.setSliderStyle(config.level_slider_style);
        levelSlider.setRange (config.level_slider_range.min, config.level_slider_range.max);
        levelSlider.setValue (config.slider_level, juce::dontSendNotification);
        levelSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 20);
        levelSlider.onValueChange = [this]()
        {
            if(AudioComponent)
                AudioComponent->SetAmplitude((float)levelSlider.getValue());
            else
                printf("WARNING: Audio Component Not Attached to GUI\r\n");
        };
        addAndMakeVisible(levelSlider);
    }
    
    void AddMuteButton( void )
    {
        //Add "Noise Active" Button:
        noiseActiveButton.onClick = [this]() {      //attach click callback
            noiseActiveButtonClicked();
        };
        
        noiseActiveButton.setClickingTogglesState(true);                        //Enable Button Toggling
        noiseActiveButton.setToggleState(false, juce::dontSendNotification);    //Set initial Toggle State
        noiseActiveButtonClicked();                                             //Call the buttonClick() Function to initialise all states.
//        noiseActiveButton.setRepaintsOnMouseActivity(true);
        addAndMakeVisible(noiseActiveButton);
    }
    
    typedef enum{
        FREQ_CTRL_GUI_MODE_STANDARD,
        FREQ_CTRL_GUI_MODE_RELATIVE,
    }freq_ctrl_gui_mode_t;
    
    void SetGUIFreqControlMode( freq_ctrl_gui_mode_t mode )
    {
        if( syncSettings.isSyncTalker  ){
            frequencySlider.onValueChange = [this]()
            {
                SetSyncGroupTalkerFrequency((float)frequencySlider.getValue());
            };
            syncButton.setVisible(false);   //Talker can't sync to itself. Remove Sync Button.
        }else{
            frequencySlider.onValueChange = [this]()
            {
                if( syncSettings.isSynced )
                    SetSyncListenerFrequencyRelativeToGroupTalker();
                else{
                    if( AudioComponent_periodic)
                        AudioComponent_periodic->SetFrequency(frequencySlider.getValue());
                }
            };
        }
        
        switch( mode ){
            case FREQ_CTRL_GUI_MODE_STANDARD:
                static const unsigned int DEFAULT_FREQ = 220;
                frequencySlider.setRange (22, 15000);
                frequencySlider.setSkewFactor(0.2);
                frequencySlider.setValue (DEFAULT_FREQ, juce::dontSendNotification);
                frequencyLabel.setVisible(false);
                break;
                
            case FREQ_CTRL_GUI_MODE_RELATIVE:
                frequencySlider.setRange (0.001, 20.0);
                float defaultMultiplier = freqGUI_instance_n * 1.333333;
                frequencySlider.setValue (defaultMultiplier, juce::dontSendNotification);
                addAndMakeVisible(frequencyLabel);
                break;
        }
    }
    
    void AddFrequencyControl( void )
    {
        //Frequency Slider
        frequencySlider.setSliderStyle(config.level_slider_style);
        frequencySlider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, 90, 20);
        
        if( syncSettings.isSyncTalker == true )     //Config for the SYNC talker (only one per group)
        {
            SetGUIFreqControlMode( FREQ_CTRL_GUI_MODE_STANDARD );
        }
        else{      //SYNC Listener (Frequency control is relative to SYNC Talker)
            SetGUIFreqControlMode( FREQ_CTRL_GUI_MODE_RELATIVE );

            syncButton.setButtonText("Sync");
            syncButton.setClickingTogglesState (true);
            syncButton.triggerClick();
            syncButton.onClick = [this] { syncButtonClicked(); };
            addAndMakeVisible(syncButton);
    //            relativeFreqButton.setState(juce::Button::buttonDown);  //Why DOESN'T THIS WORK? Had to use triggerClick() instead.
        }
        
        addAndMakeVisible(frequencySlider);
//            relativeFreqButton.changeWidthToFitText();
 
    }
    
    //Sync Active Button Handling (Frequency and Amplitude Sync)
    typedef struct SyncSettings_S{
        bool isSyncTalker = false;      //else sync listener
        bool isSynced = true;
        unsigned int syncGroup = 0;     //Not yet implemented, but this would allow multiple sync groups with their own sync master.
    }sync_settings_t;
    
    sync_settings_t syncSettings;
   
    void syncButtonClicked( void )
    {
        if( syncButton.getToggleStateValue() == true ){
            syncSettings.isSynced = true;
            SetGUIFreqControlMode(FREQ_CTRL_GUI_MODE_RELATIVE);
            SetSyncListenerFrequencyRelativeToGroupTalker();
        }else{
            syncSettings.isSynced = false;
            SetGUIFreqControlMode(FREQ_CTRL_GUI_MODE_STANDARD);
            if( AudioComponent_periodic )
                AudioComponent_periodic->SetFrequency(frequencySlider.getValue());
        }
    }
    //==============================================================================
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SigGenVoiceGUI)
};
//Static Member Definitions
SigGenVoiceGUI::object_instance_counts_t SigGenVoiceGUI::objectInstanceCounts;


//==============================================================================
class SceneComponent    :   public juce::Component
{
public:
    SceneComponent()
    {
        /*
         * Configure and Initialise SigGen GUI components
         */
        SigGenVoiceGUI::config_t SigGenGUI_config;
        SigGenGUI_config.slider_level = 0.1;
         
        for(int noise_gen = 0; noise_gen < N_SIG_GEN_VOICE_GUIS; noise_gen++ ){
            
            if( noise_gen == 0 ){
                SigGenGUI_config.Title = "White Noise";
                SigGenGUI_config.gui_type = SigGenVoiceGUI::SIG_GEN_GUI_TYPE_NOISE;
            }else{
                SigGenGUI_config.Title = "Sig Gen " + std::to_string(noise_gen);
                SigGenGUI_config.gui_type = SigGenVoiceGUI::SIG_GEN_GUI_TYPE_PERIODIC;
            }
            
            sigGenVoiceGUI[noise_gen].Init( &SigGenGUI_config );
            addAndMakeVisible (sigGenVoiceGUI[noise_gen] );
        }
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkgrey);
        
        //Draw Bounding Box
        g.setColour (juce::Colours::darkturquoise);
        g.drawRoundedRectangle(0, 0, getWidth(), getHeight(), 10, 5);
        
        //Annotations
        g.setColour (juce::Colours::lightgrey);
        g.setFont (juce::Font ("Times New Roman", 20.0f, juce::Font::bold));
        g.drawText ("Signal Sources", getLocalBounds(), juce::Justification::centredTop, true);
        
        if(displayDiagnostics){
            cpuUsageLabel.setText("CPU Usage:", juce::dontSendNotification);
//            cpuUsageText.setText("0%", juce::dontSendNotification);
            cpuUsageLabel.setBounds (10, getHeight() - 25, 100, 20);
            cpuUsageText .setBounds (85, getHeight() - 25, 100, 20);
            testOscsEnable.setBounds( 200, getHeight() - 25, 100, 20);
        }
        
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
        
        static const unsigned int X_OFFSET = 10;
        unsigned int x_pos = X_OFFSET;
        
        //TODO: Loop for N_WhiteNoise. Configure their const Positioning in Themes Class.
        for( unsigned int sig_gen_gui = 0; sig_gen_gui < N_SIG_GEN_VOICE_GUIS; sig_gen_gui++)
        {
            sigGenVoiceGUI[sig_gen_gui].setBounds(x_pos, 25,
                                                  sigGenVoiceGUI[sig_gen_gui].ComponentWidth,
                                                  sigGenVoiceGUI[sig_gen_gui].ComponentHeight);
            x_pos += sigGenVoiceGUI[sig_gen_gui].ComponentWidth + 4 + X_OFFSET;
        }
    }
    
    /*
     *  Attach Audio Component to GUI Component
     */
    void AttachAudioComponentToGuiComponent( SigGen* AudioComponent, const unsigned int Gui_index ){
//        if( Gui_index >= N_SIG_GEN_VOICE_GUIS )        //TODO: Assert, rather than return NULL
        sigGenVoiceGUI[Gui_index].AttachAudioComponent( AudioComponent );
    }
    
    void AttachPeriodicAudioComponentToGuiComponent( PeriodicOscillator* AudioComponent, const unsigned int Gui_index ){
//        if( Gui_index >= N_SIG_GEN_VOICE_GUIS )        //TODO: Assert, rather than return NULL
        sigGenVoiceGUI[Gui_index].AttachAudioComponent_Periodic( AudioComponent );
    }
    
    /*
     *  Mouse Move Used to return Co-ords to ease GUI layout.
     */
#ifdef DEBUG_REPORT_MOUSE_POSITION
    void mouseMove(const juce::MouseEvent& event) override
    {
        printf("Scene Mouse Coord (%f, %f)\r\n", event.position.getX(), event.position.getY());
    }
#endif
    
    /*
     *  Show CPU Usage Labels
     */
    void DisplayCpuTestGUI( bool enabled )
    {
        displayDiagnostics = enabled;
        if(enabled){
            addAndMakeVisible(cpuUsageLabel);
            addAndMakeVisible(cpuUsageText);
            addAndMakeVisible(testOscsEnable);
            testOscsEnable.setButtonText("CPU Test Enable");
            testOscsEnable.changeWidthToFitText();
            testOscsEnable.onClick = [this] {
                //TODO: Disable/enable TestOSCs from this tick box.
                //We could also make the number of test Oscs selectable from the GUI... which will allow us to quickly measure changes in CPU usage.
            };
        }else{
            cpuUsageLabel.setVisible(false);
            cpuUsageText.setVisible(false);
            testOscsEnable.setVisible(false);
        }
    }
    
    void SetCpuUsagePercentage( juce::String percentage )
    {
        cpuUsageText.setText(percentage, juce::dontSendNotification);
    }

private:
    
    enum { N_SIG_GEN_VOICE_GUIS = 10 };
    SigGenVoiceGUI sigGenVoiceGUI[N_SIG_GEN_VOICE_GUIS];     //TODO: Change to std::vector so the Num Voices can be changed Dynamically
    
    //Diagnostics;
    juce::Label cpuUsageLabel;
    juce::Label cpuUsageText;
    juce::ToggleButton testOscsEnable;
    bool displayDiagnostics = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SceneComponent)
};
