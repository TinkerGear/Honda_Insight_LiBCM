//Copyright 2021(c) John Sullivan
//github.com/doppelhub/Honda_Insight_LiBCM

//functions related to ignition (key) status

#include "libcm.h"

//updates when key_stateChangeHandler() called (prevents mid-loop state changes from affecting loop logic)
bool keyState_sampled = ON;

////////////////////////////////////////////////////////////////////////////////////

bool key_getSampledState(void) { return keyState_sampled; }

////////////////////////////////////////////////////////////////////////////////////

bool key_didStateChange(void)
{
	static bool keyState_previous = KEYON; //initially KEYON to force keyOff cleanup to occur when LiBCM first powered up

	bool didKeyStateChange = NO;

	if(key_getSampledState() != keyState_previous)
	{
		didKeyStateChange = YES;
		keyState_previous = key_getSampledState();
	}

	return didKeyStateChange;
}

////////////////////////////////////////////////////////////////////////////////////

void key_handleKeyEvent_off(void)
{
	Serial.print(F("OFF"));
    LED(1,LOW);
    BATTSCI_disable(); //Must disable BATTSCI when key is off to prevent backdriving MCM
    METSCI_disable();
    gpio_setFanSpeed_OEM('0');
    gpio_turnCurrentSensor_off();
    LTC6804configure_handleKeyOff();
    lcd_displayOFF();
    vPackSpoof_handleKeyOFF();
}

////////////////////////////////////////////////////////////////////////////////////

void key_handleKeyEvent_on(void)
{
	Serial.print(F("ON"));
	BATTSCI_enable();
	METSCI_enable();
	gpio_setFanSpeed_OEM('L');
	gpio_turnCurrentSensor_on();
	lcd_displayON();
	LTC68042result_maxEverCellVoltage_set(0    ); //reset maxEver cell voltage
	LTC68042result_minEverCellVoltage_set(65535); //reset minEver cell voltage
	LTC68042configure_cellBalancing_disable();
	LED(1,HIGH);
}

////////////////////////////////////////////////////////////////////////////////////

void key_stateChangeHandler(void)
{
	keyState_sampled = gpio_keyStateNow(); //this is the only time LiBCM samples actual key state

	if( key_didStateChange() == YES )
	{
		Serial.print(F("\nKey:"));
		if( key_getSampledState() == KEYON ) { key_handleKeyEvent_on() ; }
		if( key_getSampledState() == KEYOFF) { key_handleKeyEvent_off(); }
	}
}