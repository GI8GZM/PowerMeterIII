/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author

*/

/*
	Teensy 3.2
	ILI9341 TFT liquid crystal display, 240x320 dots, RGB colour.
	XPT2046 Touch screen,   Needs mapped to match TFT display
	*/

	/*
	  Version III:
		001	27/2/2020 Start of Timer Interrupt version
			ADC measure routine now interrupt timer controlled.
			PEP function improved to read correctly
			Cyclic buffer udated every 500 microsecs
			Cyclic buffer samples up to 2000 moving average
			Added power calculation direct from ADC volts
			SWR calculated from PEP
			Changed options to show samples.  No longer in nettPwr
		002 Re-organised touch buttons now in own module
			Bug in Ref - not setting radio on band change
			Improvements to pep display.
			Recoded civEnableFlg function
			Improved samplesAvg code
		003 Abandoned - visual micro compile problems
		004	Working version
			adc initialisation seperate function
			github added
		005 bluetooth tests
			code restructing
			added seperate calculation for fwd & ref

	  Versions  II:
		003 change frame, label structure
		004 finialise structures, displays and touch
		005 changed displayValue() to reduce flicker
			ADC converter coded - measure()
			tidy up code. add erase and reverse frame
			add menu touch button for calibrate mode
			add average SWR dispaly when power off
		006 - 	cleanup code
			added samples rolling average to measure()
			added zero power offset reduction
			improved pointers for value.  all display routine use frame posn.
			serial/ civ interface
		007 -	improved displayValue()
			improved civWrite() - incorporated delays and timeouts
			implemented Tuner status, touch button and control
		008 - 	Amend and tidy up structures
			clean up code
			invert nett power label when power is on
			Implemented auto tuner depending on frequency
		009	added autoBand()
			improved displayValue()
		100	started using Visual Studio 2017
			improved freqTune features
			changed touch() to detect long touch > 2secs. to be used in options
		101	changed voltage calulation to rms volt, then power
			changed swr display
			fixed several bugs
			cleaned up code, touch buttons logic
			Added band Ref dB, %Tx Power, band(mtrs)
			Added dispLabelStr() - to allow dynamic label display
		102	Using Visual Studio Community + Visual Micro
			Removed lab[].dispLabFlg and val[].dispValFlg, val[].digits
			added options for freqtune and aband
			Added %tx power. Added spectrum ref
			Added setup for variables / parameters
			cleaned code
			Added EEPROM functions

		103 - skipped
		104	Final code  - January 2019
			Changed ACD averaging to accumulated readings
			Tidied up .h header files & comments
		105	streamline Value structure and updateValue()
			added val.updateFlg
			improved measure circular buffer avaraging
			added display dimming
		106	Improved touch handling
			All timers using Metro library
		107	Option for power meter only, no CI-V
			Improved speed meterdisplay()
			Removed meter.thick, now adjust according to frame settings
			Fixed bug in selecting Tuning and Aband enable
			Added #define for reversed touch co-ords
			Added check for no CI-V connection. Change to power/swr meter only

		108	20/8/19
			fixed Aband timing issue
			fix aband "catching up " time on start
			fix Aband display start to show correct start time, ie 120 instead of 119
			start ABand timing after Tuner operation complete
			When changing band via WSJT, ABand continues if set.
		109	26/2/2020
			Peak display slow decay

	*/

	// libraries
#include <XPT2046_Touchscreen.h>								// touchscreen library
#include <ILI9341_t3.h>											// display (320x240)
#include <EEPROM.h>												// EEPROM
#include <ADC.h>												// analog - digital converter
#include <Metro.h>												// metro timers
#include "fontsColours.h"										// Teensy fonts
#include "frames.h"												// varaiables and parameters
#include "pwrMeter.h"											// PowerMeterII defines, constants & global variables

#define VERSION "PowerMeterIII_v004"							// software version

/*-------------------------------------------------------------------------------------------------------
  setup() - initialise functions
  runs once
*/
void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(TOGGLE_PIN, OUTPUT);								// set toggle pin for timing

	pinMode(A1, INPUT);											// physical pin 15, fwd volts
	pinMode(A2, INPUT);											// physical pin 16, ref volts

	initADC();													// initialise ADC, set interrupt timer

	tft.begin();												// TFT begin
	tft.setRotation(ROTATION);									// horizontal, pins to right. Cursor 0,0 = TOP LEFT of screen

	ts.begin();													// touch screen begin
	ts.setRotation(ROTATION);									// set touch rotation

	Serial.begin(19200);										// serial for debug
	delay(100);
	Serial.println("PowerMeter III");
	Serial.println("------------------------------------");
	Serial.println("Bluetooth test");
	Serial.println("------------------------------------");

	// civSerial
	civSerial.begin(19200);										// start teensy Serial1. RX1 - pin 0, TX1 - pin 1
	//bluetooth module HC - 05.  Default speed - 9600
	btSerial.begin(9600);										// start Serial3. RX3 - pin 7, TX3 - pin 8,

	/*-----initialise system------------------------------------ ------------------------------*/
	// initialise variables etc from EEPROM
	initEEPROM();

	//initialise  others
	samplesAvg = samplesDefPar.val;								// set cyclic buffer default sample size
	lab[freqTune].stat = freqTunePar.flg;						// freq tune startup status, flag set from options
	lab[aBand].stat = aBandPar.flg;								// autoband status

	if (!(bool)getFreq())										// check if civ not working -
		civEnableFlg = false;									// disable civMode, display basic mode

	// display splash screen
	analogWrite(DIM_PIN, TFT_BRIGHT);							// screen on, full bright
	splashScreen();												// display splash screen

	// initialise timers
	aBandTimer.reset();											// autoband timer
	heartBeatTimer.reset();										// heartbeat reset

	// draw screen etc
	initDisplay();
}

/*-----------------------------------------------------------------------------------------------
  loop() - main loop
  executes continuously
*/
void loop()
{
	int currBand;
	float sRef = 0, currFreq;

	//digitalWrite(TOGGLE_PIN, !digitalRead(TOGGLE_PIN));

	heartBeat();

	blueTooth();

	// dimmer timer - dim display if not active, touch to undim
	if (dimTimer.check())										// check Metro timer
		setDimmer();

	// measure & display power, swr etc - main function
	measure();

	// do following if civMode enabled
	if (civEnableFlg)
	{
		// get and display frequency
		currFreq = getFreq();
		displayValue(freq, currFreq);

		// display band Mtrs
		currBand = getBand(currFreq);							// -1(no band) or 0(160m) to 11 (4m)
		if (currBand >= 0)
			displayValue(band, hfBand[currBand].mtrs);			// display band metres

		// display spectrum ref
		sRef = setRef(currBand);
		displayValue(ref, sRef);								// display Ref

		// get tuner status
		tunerStatus();

		// check for freq difference tune
		freqDiffTune(currFreq);

		// run FT8 auto band change
		autoBand(currFreq);

		// display %TX RF Power
		//map before display, avoid rounding errors
		int pwr = getTxPwr();
		pwr = map(pwr, 0, 255, 0, 100);
		displayValue(txPwr, pwr);
	}

	// reset dimmer - check if screen has been touched
	if (ts.tirqTouched())
	{
		if (dimFlg)
			resetDimmer();										// reset dimmer
		else
			touchChk(NUM_FRAMES);
	}
}

/*------------------------------------------------------------------------------------------
 initDisplay()
	Initialises system to standard screen layout
	called by: setup() and Options()
*/
void initDisplay(void)
{
	if (!civEnableFlg)										// if civMode disabled
	{
		copyFrame(basicFrame);								// copy basic frame layout
		val[nettPwr].font = FONT48;							// increase font size
	}
	else
	{
		copyFrame(civFrame);								// copy normal frame settings
		val[nettPwr].font = FONT40;							// reset nettPwr font
	}

	// clear screen
	analogWrite(DIM_PIN, TFT_OFF);							// tft display OFF
	tft.fillScreen(BG_COLOUR);								// set tft background colour

	// draw enabled display frames
	for (int i = 0; i < NUM_FRAMES; i++)
	{
		displayLabel(i);
		val[i].updateFlg = true;							// force redisplay
	}

	// set measurement samples Reg size (averaging)
	if (samplesAvg == samplesAltPar.val)
	{
		lab[options].colour = MENU_BG;						// samples button
		fr[options].bg = MENU_COLOUR;
	}
	if (samplesAvg == samplesDefPar.val)					// display Default sample size
	{
		lab[options].colour = MENU_COLOUR;					// samples button
		fr[options].bg = MENU_BG;
	}
	sprintf(lab[options].txt, "Samples: %d", samplesAvg);
	restoreFrame(options);

	// draw meter scales  - only displays enabled
	drawMeterScale(nettPwrMeter);							// draw Pwr Meter scale
	drawMeterScale(swrMeter);								// swr meter

	// set current frequency for freq difference tuner
	val[tuner].prevValue = getFreq();
	lab[tuner].stat = -1;									// force tuner status check

	// frequency tune
	freqTuneButton(0);										// use (0) as it's a program call

	// autoband options
	aBandButton(0);											// use (0) as it's a program call

	// empty tirqtouch buffer for first operation
	if (ts.tirqTouched())
		ts.touched();

	delay(100);												// not too fast to avoid saturating CI-V commands

	// turn screen on full bright
	analogWrite(DIM_PIN, TFT_BRIGHT);
}

/*---------------------------------- resetDimmer() ----------------------
reset tft dimmer
*/
void resetDimmer()
{
	// reset dimmer flag
	ts.touched();											// clear pending touch
	dimFlg = false;											// reset flag
	analogWrite(DIM_PIN, TFT_BRIGHT);						// tft full bright
	dimTimer.reset();										// reset dimmer timer
}

/*---------------------------- setDimmer() ------------------------------
tft display dimmer
*/
void setDimmer()
{
	dimFlg = true;
	analogWrite(DIM_PIN, TFT_DIM);							// tft dim
}

/*--------------------------- heartbeat() ------------------------------
heartbeat()  - timer,displays pulsing dot top left corner
*/
void heartBeat()
{
	static bool heartBeatFlg;
	if (heartBeatTimer.check())								// check Metro timer
	{
		if (heartBeatFlg)									// draw circle
			tft.fillCircle(12, 18, 5, FG_COLOUR);
		else
			tft.fillCircle(12, 18, 5, BG_COLOUR);
		heartBeatFlg = !heartBeatFlg;						// set/reset flag
	}
}

/*--------------------------- copyFrame() ----------------------------------------
diagnostic -  copies default frame setting (frame.h) to  frame pointer
--------------------------------------------------------------------------------*/
void copyFrame(frame* fPtr)
{
	for (int i = 0; i < NUM_FRAMES; i++)
	{
		fr[i].x = fPtr[i].x;
		fr[i].y = fPtr[i].y;
		fr[i].w = fPtr[i].w;
		fr[i].h = fPtr[i].h;
		fr[i].bg = fPtr[i].bg;
		fr[i].outLineFlg = fPtr[i].outLineFlg;
		fr[i].touchFlg = fPtr[i].touchFlg;
		fr[i].enableFlg = fPtr[i].enableFlg;
	}
}