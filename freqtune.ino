/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*------------------------------ freqDiffTune() --------------------------------------------------------------------
checks for change in freq beyound set amount and activates the radio tuner if enabled.
Called by: loop()
Calls: activateTuner(), civReadFreq()
Global variables: lab[], FreqTunePrevFreq, freqTune FreqDiff
*/
void freqDiffTune(float fCurr)
{
	float fDiff = 0.0;
	int currBand;

	currBand = getBand(fCurr);
	//return if off or out of band
	if (currBand == -1 || !lab[freqTune].stat || !hfBand[currBand].isTtune)
		return;

	//change in radio freq greater than set difference?  Activate tuner
	fDiff = abs(fCurr - val[tuner].prevValue) * 1000;  // kHz
	displayValue(freqTune, constrain(freqTunePar.val - fDiff, 0, 9999));
	if (fDiff >= freqTunePar.val)
	{
		lab[tuner].stat = true;
		displayValue(freq, fCurr);
		displayValue(band, hfBand[currBand].mtrs);
		displayValue(ref, hfBand[currBand].ref);
		val[band].isUpdate = true;

		// check if in band
		if (currBand > -1)
			tunerActivate();
	}
}

/*------------------------------- freqTuneButton() -----------------------------------------------------------------------
  freqTuneButton()   -  called by touch()
  lab[freqTune].stat - 0: autotune OFF,  1: autotune ON
  tStat = 0 (don't toggle), 1 (short touch), 2 (long Touch)
  toggles status when touched if tStat <> 0
*/
void freqTuneButton(int tStat)
{
	// toggle freqTune on/off when button touched
	if (tStat)
		lab[freqTune].stat = !lab[freqTune].stat;

	if (lab[freqTune].stat)
	{
		strcpy(lab[freqTune].txt, "F/Tune: ");
		lab[freqTune].colour = MENU_BG;
		fr[freqTune].bg = MENU_COLOUR;
		val[freqTune].isUpdate = true;					// force display
		val[tuner].prevValue = getFreq();
	}
	if (!lab[freqTune].stat)
	{
		strcpy(lab[freqTune].txt, "FreqTune OFF");
		lab[freqTune].colour = MENU_COLOUR;
		fr[freqTune].bg = MENU_BG;
	}
	displayLabel(freqTune);
}

/* ------------------------------- tunerActivate() ------------------------------------------------------
triggers the auto tuner function of the radio
Called by: freqTune(), touch()
Calls: displayLabel(), civWrite(), civRead(), civReadFreq(), civReadTuner(), measure()
Global: fr[], lab[], val[], setTuner2, FreqTunePrevFreq
*/
void tunerActivate()
{
	// return if disabled
	if (!fr[tuner].isEnable)
		return;

	// change label text, colour while active
	lab[tuner].font = FONT28;
	lab[tuner].colour = FG_COLOUR;
	fr[tuner].bg = RED;
	strcpy(lab[tuner].txt, "Tune");
	displayLabel(tuner);

	// initiate tuner and loop while tuning
	civWrite(civWriteTuner);
	while (getTunerStat() == 2)						// 2 = radio tuning
		measure();

	val[tuner].prevValue = getFreq();				// save tuner frequency
	lab[tuner].stat = -1;							// ensure tunerStatus() runs
	tunerStatus();

	if (lab[aBand].stat) 							// if autoband enabled, restart timer after tuning
		aBandTimer.reset();							// reset time
}

/*----------------------------  tunerStatus() -------------------------------------------------------------------------
returns tuner status. 0=off, 1=tuner on, not activated, 2-tuning
records tunerStat. if new status = previous, just return
changes text / colours depending on status
radio and software goes to tuner from off
*/
int tunerStatus()								// display Tuner button status.
{
	int s;

	if (!fr[tuner].isEnable)
		return -1;		// check enabled?

	// get tuner status
	s = getTunerStat();
	if (s == lab[tuner].stat)				// lab[tune].stat = 1    always forces status check
		return s;							// no change in status

	switch (s)
	{
		//tuner off at radio (radio startup or switched off at radio)
	case 0:
		lab[tuner].font = FONT18;						// set font and colour
		lab[tuner].colour = FG_COLOUR;
		fr[tuner].bg = BG_COLOUR;
		strcpy(lab[tuner].txt, "Tune Off");
		lab[freqTune].stat = false;
		// update freq tuner button
		freqTuneButton(0);
		break;

		// tuner on, can activated by radio or software
	case 1:
		lab[tuner].font = FONT28;
		lab[tuner].colour = FG_COLOUR;
		fr[tuner].bg = BG_COLOUR;
		strcpy(lab[tuner].txt, "Tune");
		break;

		// tuning in operation
	case 2:
		lab[tuner].font = FONT24;
		lab[tuner].colour = FG_COLOUR;
		fr[tuner].bg = RED;
		strcpy(lab[tuner].txt, "Tuning");
		// loop and measure until done
		while (getTunerStat() == 2) measure();

		val[tuner].prevValue = getFreq();		// done, save tuner frequency
		break;

		// should never get here
	default:
		break;
	}

	// update display and save status
	displayLabel(tuner);
	lab[tuner].stat = s;

	return s;
}

/*----------------------------- getTunerStat() -------------------------------------------------
issues CIV command to read Radio Tuner status
Returns: 0 = off, 1 = on, 2 = tuning
*/
int getTunerStat()
{
	int n;
	int inBuff[12];					// civ frequency inBuff buffer

	civWrite(civReadTuner);				// request read frequency from radio
	n = civRead(inBuff);
	if (inBuff[2] == CIVADDR && inBuff[n - 1] == 0xFD)	// check format of serial stream
		return inBuff[6];				// return tuner status
	else
		return -1;
}

