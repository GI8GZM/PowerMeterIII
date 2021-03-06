/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  � Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*---------------------------------  eePromInit() ---------------------------------------------------------
Initialises EEPROM to default values from frame.h
   Normal start up reads EEPROM into band data and global variables
   EEPROM(0) is initialised flag, 0=not, 1= done
------------------------------------------------------------------------------------------*/
void initEEPROM(void)
{
	bool isEEInit = 0;
	int eeAddr;

	// comment this line to reset EEPROM to default values
	isEEInit = EEPROM.read(0);

	if (isEEInit) {
		// EEProm has been initialised.  Get values and set variables
		for (int i = 0; i < NUM_BANDS; i++) {
			eeAddr = EEADDR_BAND + EEINCR * i;
			EEPROM.get(eeAddr, hfProm[i]);

			hfBand[i].sRef = hfProm[i].sRef;
			hfBand[i].isTtune = hfProm[i].isTtune;
			hfBand[i].isABand = hfProm[i].isABand;
		}
		// get variables / parameters
		EEPROM.get(freqTunePar.eeAddr, freqTunePar);
		EEPROM.get(aBandPar.eeAddr, aBandPar);
		EEPROM.get(samplesDefPar.eeAddr, samplesDefPar);
		EEPROM.get(samplesAltPar.eeAddr, samplesAltPar);
		EEPROM.get(samplesCalPar.eeAddr, samplesCalPar);
	}
	else {
		// initialise EEPROM from compiled values
		for (int i = 0; i < NUM_BANDS; i++) {
			hfProm[i].sRef = hfBand[i].sRef;
			hfProm[i].isTtune = hfBand[i].isTtune;
			hfProm[i].isABand = hfBand[i].isABand;
			putBandEEPROM(i);
			//eeAddr = EEADDR_BAND + EEINCR * i;
			//EEPROM.put(eeAddr, hfProm[i]);
		}
		// init variables
		EEPROM.put(freqTunePar.eeAddr, freqTunePar);
		EEPROM.put(aBandPar.eeAddr, aBandPar);
		EEPROM.put(samplesDefPar.eeAddr, samplesDefPar);
		EEPROM.put(samplesAltPar.eeAddr, samplesAltPar);
		EEPROM.put(samplesCalPar.eeAddr, samplesCalPar);

		// set EEPROMinitialised flag
		EEPROM.write(0, true);
	}
}

// EEPROM put functions.  Simple but keeps all EEPROM functions in this module
/*-------------------------- putBandEEPROM() -------------------
puts band date to EEPROM
---------------------------------------------------------------*/
void putBandEEPROM(int bNum)
{
	int eeAddr = EEADDR_BAND + EEINCR * bNum;
	EEPROM.put(eeAddr, hfProm[bNum]);
}

/*---------------------------putParEEPROM() -----------------
put parameter data to EEROM
-----------------------------------------------------------*/
void putParEEPROM(param par)
{
	EEPROM.put(par.eeAddr, par);;
}
