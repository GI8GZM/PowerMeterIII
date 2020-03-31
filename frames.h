/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

// frames.h
// Frame positions --------------------------------------------------------------------------

// Default Frame position number.
const int
nettPwr = 0,			// nett Pwr
peakPwr = 1,			// peak Pwr
swr = 2,				// VSWR frame
dBm = 3,				// dBm
fwdPwr = 4,				// forward Pwr
refPwr = 5,				// reflected Pwr
fwdVolts = 6,			// forward volts
refVolts = 7,			// reflected volts

nettPwrMeter = 8,		// power meter
swrMeter = 9,			// swr meter

options = 10,			// options button frame

freqTune = 11,			// freqTune button frame
aBand = 12,				// aqutoBand button frame

civ = 13,				// civ frame
tuner = 14,				// tuner
band = 15,				// band Mtrs
sRef = 16,				// spectrum Ref
txPwr = 17,				// % Tx power
freq = 18,				// freq Mhz

freqTuneOpt = 19,		// options - tune freq difference
aBandTimeOpt = 20,		// options - autoband time

samplesDefOpt = 21,		// meaurement average - samples register size
samplesAltOpt = 22,		// meaurement average - samples register size
samplesCalOpt = 23;		// calibrate average - samples register size

// frame ------------------------------------------------------------------------
#define RADIUS 5				// frame corner radius
#define LINE_COLOUR LIGHTGREY	// frame line colour
#define NUM_FRAMES 24			// total number of frames used

struct frame {
	int x;						// top left corner - x coord
	int y;						// top left corner - y coord
	int w;						// horizontal width
	int h;						// vertical height
	int bg;						// frame background colour
	bool isOutLine;			// outline flg / don't display
	bool isTouch;				// frame enabled for touch
	bool isEnable;				// enable frame & CONTENTS
};

frame fr[NUM_FRAMES];
//------------------------------------  civ (default) frame layout ------------------------------
frame civFrame[] = {
  { 5, 10,		100, 75,	BG_COLOUR,	true,	true,	true},		// 0-nettPwr (default - nettPower)
  { 110, 10,	100, 65,	BG_COLOUR,	true,	true,	true},		// 1-peakPwr
  { 215, 10,	100, 65,	BG_COLOUR,	true,	true,	true},		// 2-swr
  { 5, 10,		100, 75,	BG_COLOUR,	true,	false,	false},		// 3-dBm

  { 5, 115,		155, 30,	BG_COLOUR,	true,	false,	false},		// 4-fwdPwr (forward power)
  { 165, 115,	155, 30,	BG_COLOUR,	true,	false,	false},		// 5-refPwr (reflected power)
  { 5, 155,		155, 50,	BG_COLOUR,	true,	false,	false},		// 6-fwdVolts
  { 165, 155,	155, 50,	BG_COLOUR,	true,	false,	false},		// 7-refVolts

	// meter position
  { 5, 95,		315, 55,	BG_COLOUR,	false,	true,	true},		// 8-Pwr Meter
  { 5, 95,		315, 55,	BG_COLOUR,	false,	true,	false},		// 9-SWR Meter

	// buttons
  { 215, 215,	105, 25,	BG_COLOUR,	true,	true,	true},		// 10-options (options button)
  { 5, 215,		105, 25,	BG_COLOUR,	true,	true,	true},		// 11-freqTune (freq tune button)
  { 110, 215,	105, 25,	BG_COLOUR,	true,	true,	true},		// 12-aBand (auto band button)

	// civ	 - do not use
  { 5, 150,		315, 65,	BG_COLOUR,	true,  false,	false},		// 13-civ (CI-V panel, contains freq, band, txPwr, Ref)

	// tuner status
  {5, 160,		105, 45,	BG_COLOUR,	true,	true,	true},		// 14-tuner	(e45o radio tuner status)

  // civ data displays
  { 115, 160,	100, 45,	BG_COLOUR,	true,	true,	true},		// 15-band (hf Band)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},		// 16-ref (radio spectrum reference)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	true,	true},		// 17-txPwr	(radio - %TX Power)
  { 113, 160,	200, 45,	BG_COLOUR,	true,	false,	false},		// 18-freq	(radio frequen160, 55z)

	// variables / parameters
  { 200, 10,	90, 40,		ALT_BG,		true,	false,	false},		// 19-freqTuneOpt (options for freqTune by hf band)
  { 200, 125,	90, 40,		ALT_BG,		true,	false,	false},		// 20-aBandTimeOpt (options for autoband 45ange by hf band)
  { 200, 40,	90, 40,		ALT_BG,		true,	false,	false},		// 21-samplesDefault (averaging samples - default)
  { 200, 100,	90, 40,		ALT_BG,		true,	false,	false},		// 22-samplesAltOpt	(averaging samples - alternate)
  { 200, 160,	90, 40,		ALT_BG,		true,	false,	false},		// 23-samplesCalOpt	(averaging samples - calibrate mode)
};

// ------------------------------  basic (non civ) frame layout -------------------------------
frame basicFrame[] = {
  { 5, 10,		100, 100,	BG_COLOUR,	true,	true,	true},		// 0-nettPwr (default - nettPower)
  { 110, 10,	100, 65,	BG_COLOUR,	true,	true,	true},		// 1-peakPwr
  { 215, 10,	100, 65,	BG_COLOUR,	true,	true,	true},		// 2-swr
  { 5, 10,		100, 75,	BG_COLOUR,	true,	false,	false},		// 3-dBm

  { 5, 115,		155, 30,	BG_COLOUR,	true,	false,	false},		// 4-fwdPwr (forward power)
  { 165, 115,	155, 30,	BG_COLOUR,	true,	false,	false},		// 5-refPwr (reflected power)
  { 5, 155,		155, 50,	BG_COLOUR,	true,	false,	false},		// 6-fwdVolts
  { 165, 155,	155, 50,	BG_COLOUR,	true,	false,	false},		// 7-refVolts

	// meter position
  { 5, 110,		315, 50,	BG_COLOUR,	false,	true,	true},		// 8-Pwr Meter
  { 5, 160,		315, 50,	BG_COLOUR,	false,	true,	true},		// 9-SWR Meter

	// buttons
  { 215, 215,	105, 25,	BG_COLOUR,	true,	true,	true},		// 10-options (options button)
  { 5, 215,		105, 25,	BG_COLOUR,	true,	false,	false},		// 11-freqTune (freq tune button)
  { 110, 215,	105, 25,	BG_COLOUR,	true,	false,	false},		// 12-aBand (auto band button)

	// civ	 - do not use
  { 5, 150,		315, 65,	BG_COLOUR,	true,  false,	false},		// 13-civ (CI-V panel, contains freq, band, txPwr, Ref)

	// tuner status
  {5, 160,		105, 45,	BG_COLOUR,	true,	false,	false},		// 14-tuner	(e45o radio tuner status)

  // civ data displays
  { 115, 160,	100, 45,	BG_COLOUR,	true,	false,	false},		// 15-band (hf Band)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},		// 16-ref (radio spectrum reference)
  { 220, 160,	 95, 45,	BG_COLOUR,	true,	false,	false},		// 17-txPwr	(radio - %TX Power)
  { 113, 160,	200, 45,	BG_COLOUR,	true,	false,	false},		// 18-freq	(radio frequen160, MHz)

	// variables / parameters
  { 200, 10,	90, 40,		ALT_BG,		true,	false,	false},		// 19-freqTuneOpt (options for freqTune by hf band)
  { 200, 125,	90, 40,		ALT_BG,		true,	false,	false},		// 20-aBandTimeOpt (options for autoband 45ange by hf band)
  { 200, 40,	90, 40,		ALT_BG,		true,	false,	false},		// 21-samplesDefault (averaging samples - default)
  { 200, 100,	90, 40,		ALT_BG,		true,	false,	false},		// 22-samplesAltOpt	(averaging samples - alternate)
  { 200, 160,	90, 40,		ALT_BG,		true,	false,	false},		// 23-samplesCalOpt	(averaging samples - calibrate mode)
};

// label --------------------------------------------------------------------------------------------------------
#define GAP 5						// gap from frame outline

struct label {
	char txt[30];					// frame label text
	int colour;						// text colour
	ILI9341_t3_font_t  font;		// text font size
	char xJustify;					// 'L'eft, 'C'entre, 'R'ight
	char yJustify;					// 'T'op, 'M'iddle, 'B'ottom
	int stat;						// status, used for update label display, -1 for errors
};

label lab[] = {
  { "Watts",		FG_COLOUR,		FONT14,     'C', 'B', 0,	},		// nett Pwr frame
  { "Pep",			FG_COLOUR,		FONT14,		'C', 'B', 0,	},		// peak Pwr	 lab.stat =1 for peakpwr, .stat=0 for PEP
  { "vSWR",			FG_COLOUR,		FONT14,		'C', 'B', 0,	},		// VSWR
  { "dBm",			FG_COLOUR,		FONT14,		'C', 'B', 0,	},		// dBm

  { "Fwd Pwr",		YELLOW,			FONT10,		'L', 'M', 0,	},		// forward Pwr frame
  { "Ref Pwr",		YELLOW,			FONT10,		'R', 'M', 0,	},		// reflected Pwr
  { "Fwd Volts",	GREENYELLOW,	FONT10,		'R', 'M', 0,	},		// Forward voltage
  { "Ref Volts",	GREENYELLOW,	FONT10,		'L', 'M', 0,	},		// Reflected Voltage

  // meters
  { "       Watts",	MTXT_COLOUR,		FONT8,		'L', 'B', 0,	},	// Pwr Meter
  { "          vSWR ",	MTXT_COLOUR,	FONT8,		'L', 'B', 0,	},	// SWR Meter

  // buttons
  { "Options",		MENU_COLOUR,	 FONT12,	'C', 'M', 0,	},		// options button
  { "FreqTune On",	MENU_COLOUR,	 FONT12,	'L', 'M', 0,	},		// freqtuner button
  { "ABand On",		MENU_COLOUR,	 FONT12,	'L', 'M', 0,	},		// autoBand button

  //civ
 // { "Icom IC7300 - CIV Control", GREEN, FONT10, 'C', 'T', 1, },		// civ
  { "",				GREEN,			FONT10,		'C', 'T', true, },		// civ
  { "Tune Off",		FG_COLOUR,		FONT18,		'C', 'M', 0,	},		// tuner
  { "mtrs ",		FG_COLOUR,		FONT14,		'R', 'M', 0,	},		// band
  { "Ref ",			FG_COLOUR,		FONT14,		'R', 'M', 0,	},		// ref
  { "%Tx ",			FG_COLOUR,		FONT14,		'R', 'M', 0,	},		// % RF Power setting
  { "MHz ",			CIV_COLOUR,		FONT18,		'R', 'M', 0,	},		// freq

  // variable pa	rameters
  { " kHz",			CIV_COLOUR,		FONT14,		'R', 'M', 0,	},		// tuner freq diff
  { " Secs",		CIV_COLOUR,		FONT14,		'R', 'M', 0,	},		// aBand time, secs
  { "",				CIV_COLOUR,		FONT14,		'L', 'M', 0,	},		// default samples size
  { "",				CIV_COLOUR,		FONT14,		'R', 'M', 0,	},		// alternate samples size
  { "",				CIV_COLOUR,		FONT14,		'R', 'M', 0,	},		// calibrate samples size
};

// value ---------------------------------------------------------------------------
struct value {
	float prevValue;					// previous value
	int decs;							// decimals
	int colour;							// text colour
	ILI9341_t3_font_t  font;			// text font size
	bool isUpdate;						// true forces update
};

value val[] = {
  { 0.0, 0,	 FG_COLOUR,		FONT40,		true},				// 0 - nettPwr
  { 0.0, 0,	 FG_COLOUR,		FONT32,		true},				// 1 - peakPwr
  { 0.0, 1,	 ORANGE,		FONT28,		true},				// 2 - swr
  { 0.0, 0,	 GREEN,			FONT48,		true},				// 3 - dbm

  { 0.0, 2,	 ORANGE,		FONT18,		true},				// 4 - fwdPwr
  { 0.0, 2,	 ORANGE,		FONT18,		true},				// 5 - refPwr
  { 0.0, 4,	 ORANGE,		FONT20,		true},				// 6 - fwdVolts
  { 0.0, 4,	 ORANGE,		FONT20,		true},				// 7 - refVolts

  { 0.0, 0,	 ORANGE,		FONT18,		true},				// 8 - nettPwrMeter
  { 0.0, 0,	 ORANGE,		FONT18,		true},				// 9 - swrMeter

  { 0.0, 0,	 BG_COLOUR,		FONT16,		true},				// options button
  { 0.0, 0,	 BG_COLOUR,		FONT16,		true},				// freqtuner button
  { 0.0, 0,	 BG_COLOUR,		FONT16,		true},				// autoBand button

  { 0.0, 4,	 ORANGE,		FONT18,		true},				// 13 - civ
  { 0.0, 5,	 CIV_COLOUR,	FONT24,		true},				// tuner
  { 0.0, 0,	 CIV_COLOUR,	FONT24,		true},				// band
  { 0.0, 1,	 CIV_COLOUR,	FONT20,		true},				// ref
  { 0.0, 0,	 CIV_COLOUR,	FONT24,		true},				// % Tx Power Setting
  { 0.0, 5,	 CIV_COLOUR,	FONT24,		true},				// freq

  { 0.0, 0,	 CIV_COLOUR,	FONT18,		true},				// tune freq diff
  { 0.0, 0,	 CIV_COLOUR,	FONT18,		true},				// aBand time
  { 0.0, 0,	 CIV_COLOUR,	FONT18,		true},				// measure samples size
  { 0.0, 0,	 CIV_COLOUR,	FONT18,		true},				// measure samples size
  { 0.0, 0,	 CIV_COLOUR,	FONT18,		true},				// calibrate samples size
};

// meter -----------------------------------------------------------------------
struct meter {
	int xGap;							// incremental x co-ord (top left conrner)
	int yGap;							// incremental y co-ord (top left conrner)
	int colour;							// scale colour
	ILI9341_t3_font_t font;				// scale font (size)
	float sStart;						// start value for scale
	float sEnd;							// end value for scale
	int major;							// number major scale divisions
	int minor;							// number minor divs
	int tColour;						// meter bar gradient top colour
	int bColour;						// meter bar bottom colour
	int pkWidth;						// peak indicator width
	int pkColour;						// peak indicator colour
	int pkPrevPosn;						// peak indicator prev width
};

meter mtr[] = {
{ 10, 5,   FG_COLOUR, Arial_8,   0, 100,	4, 20, FG_COLOUR, BG_COLOUR,   5, ORANGE, 0, },
{ 10, 5,   FG_COLOUR, Arial_8,  1.0, 4.0,	4, 20, FG_COLOUR, BG_COLOUR,   5, ORANGE, 0, },
};

/*-------------------------------Frequency / Band data-----------------------------*/
#define NUM_BANDS 11
struct freqband {
	int posn;							// hf band posn
	char txt[10];						// band description
	int mtrs;							// band - metres
	float prevFreq;						// previous frequency for update
	float ft8Freq;						// FT8 freq for autoband change
	float bandStart;					// band start freq
	float bandEnd;						// band end freq
	float sRef;							// spectrum reference
	bool isTtune;						// enable tuner flag for this band
	bool isABand;						// enable autoband flag
};

freqband hfBand[] = {
	//  posn	txt			mtrs  prevFreq   FT8Freq  BandStart   BandEnd		Ref	  tuneFlg  aBandFlg
		{ 0, "160 Mtrs",	160,	0.0,	 1.840,		1.81,		2.0,		0,		false,	false,	},
		{ 1, "80 Mtrs",		80 ,	0.0,	 3.573,		3.5,		3.8,		0,		true,	true,	},
		{ 2, "60 Mtrs",		60 ,	0.0,	 5.357,		5.2585,		5.4065,		0,		false,	false,	},
		{ 3, "40 Mtrs",		40 ,	0.0,	 7.074,		7.0,		7.2,		0,		true,	true,	},
		{ 4, "30 Mtrs",		30 ,	0.0,	 10.136,	10.1,		10.15,		0,		true,	true,	},
		{ 5, "20 Mtrs",		20 ,	0.0,	 14.074,	14.0,		14.35,		0,		true,	true,	},
		{ 6, "17 Mtrs",		17 ,	0.0,	 18.100,	18.068,		18.168,		0,		true,	true,	},
		{ 7, "15 Mtrs",		15 ,	0.0,	 21.074,	21.0,		21.45,		0,		true,	true,	},
		{ 8, "12 Mtrs",		12 ,	0.0,	 24.915,	24.89,		24.99,		0,		true,	true,	},
		{ 9, "10 Mtrs",		10 ,	0.0,	 28.074,	28.0,		29.7,		0,		true,	true,	},
		{ 10, "6 Mtrs",		6,		0.0,	 50.313,	50.0,		52.0,		0,		false,	false,	},
		//  { 11, "4 Mtrs",		4,		0.0,	 70.100,	70.0,		70.5,		0,		false,	false,	),
};

/* structure for options boxes */
struct optBox										// touch check bxes/circles co-ords
{
	int x;
	int y;
};
optBox		tb[30];									// tb[] is touch area co-ord

/*----------EEPROM Options for HF Bands----------------------------------------------------*/
// EEPROM Adresses + Increments
#define		EEINCR 16								// address increment for band options and parameters
#define		EEADDR_BAND 100							// start address band info
#define		EEADDR_PARAM 10							// start address variable parameter

/* strucure for Options - hfBands  (12 locations 3x4) */
struct eeProm0
{
	float	sRef;									// band radio spectrum ref
	bool	isTtune;								// tuner enable flag
	bool	isABand;								// autoband enable falg
};
eeProm0		hfProm[NUM_BANDS];						 // hf bands info

/*----------eeProm strucure for Variable Parameters------------------------------*/
struct param										// param structure definition
{
	int		val;									// variables value
	bool	isFlg;									// variable flag
	int		eeAddr;									// eeProm address
};
param		freqTunePar = { 200,	0,	EEADDR_PARAM };					// freqTune parameters
param		aBandPar = { 120,	0,	EEADDR_PARAM + 0x10 };				// autoband paramters
param		samplesDefPar = { 5,	1,	EEADDR_PARAM + 0x20 };			// number samples for averaging - default
param		samplesAltPar = { 1,	1,	EEADDR_PARAM + 0x30 };			// alternate samples number
param		samplesCalPar = { 20,	1,	EEADDR_PARAM + 0x40 };			// samples for averaging
