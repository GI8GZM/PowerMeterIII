/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/* Ref (spectrum reference) functions */

/*------------------------------ refButton() -----------------------------------
short touch - swaps back to txpwr
long touch - saves current spectrum ref against current band
*/
void refButton(int tStat)
{
	if (tStat != 2)
	{
		eraseFrame(ref);					// revert to txPwr
		restoreFrame(txPwr);
	}
	else
	{
		// long touch saves current radio ref to hfBand

		float freq = getFreq();				// need frequency to get band
		int band = getBand(freq);			// get band number

		float r = getRef();					// get current spectrum ref
		hfBand[band].ref = r;
		hfProm[band].ref = r;				// save to EEPROM
		putBandEEPROM(band);				// update EEProm

		// blink frame to show write
		eraseFrame(ref);
		delay(100);							// 0.1 sec blink to show memory write
		restoreFrame(ref);
		displayValue(ref, r);				// update reffarme with value
	}
}

/*------------------------- setRef() ---------------------------
get Ref and if band changed, set ref
passed: current band (int)
returns: reference (float)
*/
float setRef(int band)
{
	static int prevBand;
	float ref;

	ref = getRef();							// read Ref from radio
	if (band != prevBand)					// band change?
	{
		ref = hfBand[band].ref;				// get ref
		putRef(ref);						// send spec ref to radio
		prevBand = band;					// save current band
	}
	return(ref);
}

/*------------------------------ getRef() -------------------------------
reads radio sprectrum Ref setting
Returns float (ref)
*/
float getRef()
{
	int n;												// chars read into buffer
	unsigned int n0 = 0, n1 = 0;
	int inBuff[12];										// civ frequency inBuff buffer
	float sref = 0.0;									// spectrum ref

	n = civWrite(civReadRef);							// request read frequency from radio
	n = civRead(inBuff);								// buffer, printflg
	if (inBuff[2] == CIVADDR && inBuff[n - 1] == 0xFD)			// check format of serial stream
	{
		n0 = (inBuff[n - 4] / 16) * 10 + inBuff[n - 4] % 16;	// convert from BCD
		n1 = (inBuff[n - 3] / 16) * 10 + inBuff[n - 3] % 16;
	}
	sref = n0 + (float)n1 / 100.0;						// format
	if (inBuff[n - 2]) sref = sref * -1;

	return(sref);
}

/*------------------------------ putRef() ---------------------------------
set radio sprectrum reference
ref - spectrum reference to set
*/
void putRef(float ref)
{
	int bytes = 7;							// num bytes in civ command, excludeing preamble
	int n0, n1;

	// convert to BVD format for CI-V
	// check if float negative
	if (ref < 0)
		civWriteRef[bytes - 2] = 0x01;
	else
		civWriteRef[bytes - 2] = 0x00;

	// convert float to BCD
	ref = abs(ref);
	n0 = (int)ref;
	n1 = (ref * 100) - n0 * 100;

	civWriteRef[bytes - 4] = (n0 / 10) * 16 + (n0 % 10);
	civWriteRef[bytes - 3] = (n1 / 10) * 16 + (n1 % 10);

	civWrite(civWriteRef);
}


