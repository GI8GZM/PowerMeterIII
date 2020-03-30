/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

// pwrMeter.h

/*---------- measure() constants and defines------------------
vary Tx power and record pk-pk volts.
Record ADC voltages in calMode
graph Watts against volts
two sets of factors as detection diodes are no linear at low powers.

*/
#define PWR_THRESHOLD   1.0	    				    // power on threshold watts
#define	VF_ZEROADJ      0.000					    // ADC zero offset voltage
#define	VR_ZEROADJ      0.0016  				    // ADC zero offset voltage
#define	V_ZEROADJ       0.000  					    // ADC zero offset voltage

//Cal: 02 Mar 2020
// RMS voltage calculation
#define	LO_MULT         1.0907
#define LO_ADD          8.506			            // LO vrms = ln(v)*LO_MULT + LO_ADD
#define	V_SPLIT         0.05			            // split voltage
#define	HI_MULT         20.702
#define HI_ADD          4.2862			            // HI vrms = v*HI_MULT + HI_ADD

// Direct power conversion constants
// forward power constants
#define	FWD_V_SPLIT_PWR     0.02					 // split voltage, direct pwr conversion
#define	FWD_LO_MULT_PWR     0.1308					 // LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
#define FWD_LO_ADD_PWR      0.9748
#define	FWD_HI_MULT2_PWR    10.178
#define FWD_HI_MULT1_PWR    5.7523
#define FWD_HI_ADD_PWR      0.3202					 // HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR
// reflected power constants
#define	REF_V_SPLIT_PWR     0.02					 // split voltage, direct pwr conversion
#define	REF_LO_MULT_PWR     0.1308					 // LO pwrs = ln(v)*LO_MULT_PWR + LO_ADD_PWR
#define REF_LO_ADD_PWR      0.9748
#define	REF_HI_MULT2_PWR    10.178
#define REF_HI_MULT1_PWR    5.7523
#define REF_HI_ADD_PWR      0.3202					 // HI pwr = v*v*HI_MULT2_PWR +v*HI_MULT1_PWR + HI_ADD_PWR

#define PEP_DECAY       0.70						// decay factror for pep

 /*---------------------------Serial ports -------------------*/
#define		civSerial   Serial1					    // uses serial1 rx/tx pins 0,1
bool	    isCivEnable = true;				    // 0 = Power meter only, 1 = added CI-V
#define		btSerial    Serial3					    // bluetooth serial3 - pins 7,8

/*----------Icom CI-V Constants------------------------------*/
#define CIVADDR         0xE2			        	// this controller address
#define CIVRADIO        0x94						// Icom IC-7300 CI-V default address
#define CIVTIMEOUT      100						    // 100 milli-seconds
#define CIV_WRITE_DELAY 3					        // CIV write delay = milliseconds
#define CIV_READ_DELAY  1						    // CIV read delay to ensure rx buffer fill

/*----------Icom CI-V commands------------------------------*/
int civPreamble[] =     { 0xFE, 0xFE,  CIVRADIO, CIVADDR };			    // write command preamble
int	civReadFreq[] =     { 0x03, 0xFD };								    // read frequency
int civWriteFreq[] =    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFD };	// set frequency
int civReadTuner[] =    { 0x1C, 0x01, 0xFD };							// read tuner status
int civWriteTuner[] =   { 0x1C, 0x01, 0x02, 0xFD };					    // radio tuner activate
int civReadRef[] =      { 0x27, 0x19, 0x00, 0xFD };						// read spectrum Reference
int civWriteRef[] =     { 0x27, 0x19, 0x00,0x00,0x00,0x00,0xFD };		// spectrum Reference
int	civReadTxPwr[] =    { 0x14, 0x0A, 0xFD };						    // read RF Power setting
int	civWriteTxPwr[] =   { 0x14, 0x0A, 0x00, 0x00, 0xFD };			    // set RF Power

/*----------ILI9341 TFT display (320x240)-------------------------*/
#define     ROTATION 1									// rotation for tft and touchscreen
#define		TFT_DC 9
#define		TFT_CS 10								// TFT CS pin
ILI9341_t3	tft = ILI9341_t3(TFT_CS, TFT_DC);		// define tft device
bool		isDim = false;							// dim flag, false = no dim
#define		DIM_PIN 4								// analog out pin for display LED & dimming
#define		TFT_BRIGHT 255							// tft display full brightness
#define		TFT_DIM	10								// tft display dim value
#define		TFT_OFF	0								// tft display off
#define		SPLASH_DELAY 3000						// splash screen dealy, mSecs.. At power on, allow time for radio to boot

/*----------XPT2046 touchscreen	-----------------------------*/
#define		TS_IRQ 2								// touch interrupt pin
#define		TS_CS  6								// Touch CS. *** do NOT use pin 8...  required for Serial3
XPT2046_Touchscreen ts(TS_CS, TS_IRQ);				// allows touch screen interrupts
#define     LONGTIME 500					        // long touch time (mSecs)
#define		TOUCH_REVERSED true						// true = reversed
#if			TOUCH_REVERSED
int xMapL = 3850, xMapR = 320;
int yMapT = 3800, yMapB = 300;
#else
int xMapL = 320, xMapR = 3850;
int yMapT = 300, yMapB = 3800;
#endif
#define		MAPX map(p.x, xMapL, xMapR, 0, 320)		// reveresed touch screen mapping
#define		MAPY map(p.y, yMapT, yMapB, 0, 240)
#define	    FONT_PM AwesomeF000_16					// font for plus/minus symbols
#define		PLUS_SYMBOL 85							// + symbol
#define		MINUS_SYMBOL 86							// - symbol
#define	    T_OFFSET 15							    // touch offset distance (pixels)
//const int TICK_SYMBOL = 12;						// Awesome_F000 character
//const int CROSS_SYMBOL = 13;

/*----------ACD settings --------------------------------------*/
// ACD parameters are define in acd.ino
ADC* adc = new ADC();							    // adc object
ADC::Sync_result result;						    // ADC result structure
const int    MAXBUF = 1000;						    // maximum averaging buffer/samples size
int	         samplesAvg;						    // default cyclic buffer samples
volatile int sample;							    // ADC cyclic buffer sample
volatile long buf0Tot, buf1Tot;					    // cyclic buffer totals
volatile unsigned int buf0Pk, buf1Pk;			    // cyclic buffer peak values
volatile unsigned int buf0[MAXBUF + 1] = {}, buf1[MAXBUF + 1] = {};  // cylic buffer used by interrupt routine
#define     SAMPLE_INTERVAL 500						// ADC sample interval (microsecs)
IntervalTimer sampleTimer;						    // getADC interupt timer

/*----------Metro timers-----------------------------------------*/
Metro aBandTimer =      Metro(1000);				// autoband time milliseconds, auto reset
Metro heartBeatTimer =  Metro(250);			        // heartbeat timer
Metro longTouchTimer =  Metro(750);			        // long touch timer
Metro pkPwrTimer =      Metro(3000);			    // peak power hold timer
Metro pepTimer =        Metro(500);				   	// pep hold timer
Metro civTimeOut =      Metro(100);				    // civ read/write watchdog timer
Metro dimTimer =        Metro(15 * 60 * 1000);		// dimmer timer (mins)


/*----------pin assigns--------------------------------------*/
#define		TOGGLE_PIN 5							// high/low pulse output for timing




// constant expression to convert BCD to decimal
constexpr int getBCD(int n)
{
	return n / 16 * 10 + n % 16;
}
// constant expression to convert decimal to BCD 
constexpr int putBCD(int n)
{
	return n / 10 * 16 + n % 10;
}




/*---------- Teensy restart code (long press on Peak Power frame)--------*/
#define		CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define		CPU_RESTART_VAL 0x5FA0004
#define		CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

/*----------Teensy - enable printf functions------------------------------------*/
/* this doesn't seem to be required
 this is the Teensy magic trick for  printf to support float
asm(".global _printf_float");
 this is the magic trick for scanf to support float
asm(".global _scanf_float");
 */