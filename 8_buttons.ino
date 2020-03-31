/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*-----  button functions -----------------------------------------------

buttons.ino   -- functions for button presses
FreqTune
Autoband
Samples
NettPwr
txPwr

*/

/*------------------------------ touchNettPwr() ---------------------------------
short touch (1)- toggles  number of samples used for averaging
long touch (2) - switches to dBm display. any touch goes back to nettPwr
*/
void nettPwrButton(int tStat)							// passed touch status
{
	if (tStat == 1)
	{
		// normal measure mode
		if (lab[civ].stat == true)						// toggle samplesAvg
		{
			if (samplesAvg == samplesDefPar.val)
				samplesAvg = samplesAltPar.val;
			else
				samplesAvg = samplesDefPar.val;
		}

		// calMode
		if (lab[civ].stat != true)
		{
			if (samplesAvg == samplesCalPar.val)
				samplesAvg = samplesAltPar.val;			// toggle CalPar / AltPar
			else
				samplesAvg = samplesCalPar.val;
		}

		// display modified label showing samples number
		invertLabel(options);							// invert display (toggle)
		sprintf(lab[options].txt, "Samples: %d", samplesAvg);
		restoreFrame(options);							// draw lable
	}

	// check long touch - change to dBm display
	if (tStat == 2)
	{
		eraseFrame(nettPwr);
		restoreFrame(dBm);
		displayValue(dBm, dBm);							// needed with longtouch to show when done
	}
}

/*------------------------ dBmButton()------------------------------------
dBm displayed by nettPower long touch
short touch restores to nettPwr (Watts) display
*/
void dbmButton(int tStat)
{
	eraseFrame(dBm);									// erases and disables  nettPwr frame
	fr[nettPwr].bg = BG_COLOUR;
	restoreFrame(nettPwr);
}

/*-----------------------------peakPwrButton() --------------------------------------------------
short touch(1) - swap from Peak average power to PEP
long touch(2) - reboot Teensy
program call(0)
*/
void peakPwrButton(int tStat)							// 0 = program call. sets display.  1 - short touch, 2 - long touch
{
	if (tStat == 1)
		lab[peakPwr].stat = !lab[peakPwr].stat;

	if (lab[peakPwr].stat)								// do for all tStat
		strcpy(lab[peakPwr].txt, "Peak");
	else
		strcpy(lab[peakPwr].txt, "PEP");

	restoreFrame(peakPwr);

	//_reboot_Teensyduino_();
	if (tStat == 2)
		CPU_RESTART;
}

/*---------------------- swrButton ------------------------------
short touch - swap from one decimal to two decmal places
long touch - initialise display
*/
void swrButton(int tStat)
{
	if (tStat == 2)
	{
		isCivEnable = !isCivEnable;						// swap between civ mode and basic mode
		initDisplay();									// initialise screen, civMode frames or basicMode frames
	}
	else
	{
		if (val[swr].decs == 1)							// change decimals
		{
			val[swr].font = FONT24;
			val[swr].decs = 2;
		}
		else
		{
			val[swr].font = FONT28;						// fewer decimals, larger font
			val[swr].decs = 1;
		}
	}
	restoreFrame(swr);
	displayValue(swr, 1.0);
}

/*---------------------------- autoBandButton() -----------------------------------
short touch - swap frequency (Mhz) band(metres) & txPwr/ref
long touch - null
*/
void autoBandButton(int tStat)
{
	if (tStat == 2)										// long touch
		return;
	else												// short touch	
	{
		if (fr[freq].isTouch)
		{												// if enabled, band active
			eraseFrame(freq);
			restoreFrame(band);
			restoreFrame(txPwr);
		}
		else
		{												// dispaly frequency
			eraseFrame(band);
			eraseFrame(txPwr);
			eraseFrame(sRef);
			restoreFrame(freq);
		}
	}
}

/*------------------------------ samplesButton --------------------------------------------------------------------
toggles between calibrate mode and ci-v mode
tStat = 0 (don't toggle), 1 (short touch), 2 (long Touch)
*/
void samplesButton(int tStat)
{
	if (tStat == 1)
		setSamples();									// set samples numbers

	if (tStat == 2)
	{
		lab[civ].stat = !lab[civ].stat;					// toggle calibrate/normal mode  = !lab[civ].stat}

		if (!lab[civ].stat)								// true is normal civmode
		{
			samplesAvg = samplesCalPar.val;
			calMode();									// go to calibrate mode
		}
		else
		{
			samplesAvg = samplesDefPar.val;
			initDisplay();								// set normal mode
		}
	}
}