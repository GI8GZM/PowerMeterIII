/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*-------------------------------------- autoBand() -----------------------------------------------------------------------
global variable aBandCountDown is number of seconds before band change is initiated
uses Metro timer for 1 sec count
skips disabled bands. at end band goes back to start.  if all bands disabled, will stop at current frequeny
uses lab.stat for on/off signals
*/
void autoBand(float fCurr)
{
	int currBand = 0, nextBand;
	static int aBandCountDown;
	static bool isRrestart;

	// check autoband is enabled and check status and exit conditions
	if (!fr[aBand].isEnable || !lab[aBand].stat)		// enable flag and on/off status
	{
		isRrestart = false;
		return;
	}

	// frequency changed? Turn off and update button
	if (fCurr != getFreq())
	{
		lab[aBand].stat = false;						// reset flags
		isRrestart = false;					
		aBandButton(false);								// update button display
		return;
	}

	if (!isRrestart)
	{
		isRrestart = true;
		aBandCountDown = aBandPar.val;
		aBandTimer.reset();
	}

	// is 1sec timer triggered (Metro timer)			/// only go past here if 1 sec expires
	if (!aBandTimer.check())
		return;
	else
	{
		// display countdown and return if not complete
		aBandCountDown--;
		displayValue(aBand, aBandCountDown);
		if (aBandCountDown > 0)
			return;
	}

	// only get here if countdown complete		
	aBandTimer.reset();

	//next band
	   //displayValue(aBand, aBandCountDown);			// draw aBandCountDown - display 0

	   // get next frequency
	fCurr = getFreq();
	for (int i = 0; i < NUM_BANDS; i++)
	{
		if (fCurr == hfBand[i].ft8Freq)
		{
			currBand = i;
			break;
		}
	}

	// change to next enabled frequency
	nextBand = currBand + 1;
	if (nextBand == NUM_BANDS)
		nextBand = 0;							// go round loop

	while (!hfBand[nextBand].isABand)
	{
		nextBand++;
		if (nextBand == NUM_BANDS)
			nextBand = 0;
		if (nextBand == currBand)
			return;								// round the loop, all disabled so return
	}

	encodeFreq(civWriteFreq, hfBand[nextBand].ft8Freq);	// encode new freq
	civWrite(civWriteFreq);							// change frequency - issue CAT command
	//displayValue(freq, aBandFreqPrev);
	//aBandFreqPrev = getFreq();				// save current freq

	aBandCountDown = aBandPar.val;				// reset autoband timer

	// update button
	aBandButton(false);
}

/*----------------------------------- aBandButton() ----------------------------------------------------------------
turned on/off by touch button OR  off by change in frequency
toggles status when touched if tstat <> 0
*/
void aBandButton(int tStat)
{
	if (tStat)
		lab[aBand].stat = !lab[aBand].stat;				// toggle start/stop

	// disable aBand
	if (!lab[aBand].stat)
	{
		strcpy(lab[aBand].txt, "  ABand OFF");
		lab[aBand].colour = MENU_COLOUR;
		fr[aBand].bg = MENU_BG;
		displayLabel(aBand);							// display Off label, blanks time
		//displayValue(aBand, aBandPar.val);				// display time

	}

	else												// autoband active
	{
		strcpy(lab[aBand].txt, "ABand: ");				// display label
		lab[aBand].colour = MENU_BG;
		fr[aBand].bg = MENU_COLOUR;
		displayLabel(aBand);							// display label, time is blank
		displayValue(aBand, aBandPar.val);				// display time
	}
}


