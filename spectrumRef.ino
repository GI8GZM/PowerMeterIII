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
		eraseFrame(ref);								// revert to txPwr
		restoreFrame(txPwr);
	}
	else
	{
		// long touch saves current radio ref to hfBand
		float freq = getFreq();							// need frequency to get band
		int band = getBand(freq);						// get band number

		float r = getRef();								// get current spectrum ref
		hfBand[band].ref = r;
		hfProm[band].ref = r;							// save to EEPROM
		putBandEEPROM(band);							// update EEProm

		// blink frame to show write
		eraseFrame(ref);
		delay(100);										// 0.1 sec blink to show memory write
		restoreFrame(ref);
		displayValue(ref, r);							// update reffarme with value
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

	ref = getRef();										// read Ref from radio
	if (band != prevBand)								// band change?
	{
		ref = hfBand[band].ref;							// get ref
		putRef(ref);									// send spec ref to radio
		prevBand = band;								// save current band
	}
	return ref;
}

/*------------------------------ getRef() -------------------------------
reads radio sprectrum Ref setting
Returns float (ref)
*/
float getRef()
{
	int n;												// chars read into buffer
	int u = 0, d = 0;									// units & decimals
	float sref = 0.0;									// spectrum ref
	int inBuff[12];										// civ frequency inBuff buffer

	n = civWrite(civReadRef);							// request read frequency from radio
	n = civRead(inBuff);								// buffer, printflg
	if (inBuff[2] == CIVADDR && inBuff[n - 1] == 0xFD)	// check format of serial stream
	{
		u = getBCD(inBuff[n - 4]);						// convert from BCD
		d = getBCD(inBuff[n - 3]);
	}
	sref = u + (float)d / 100.0;						// format
	if (inBuff[n - 2])
		sref = sref * -1;								// if negative

	return sref;
}

/*------------------------------ putRef() ---------------------------------
set radio spectrum reference
ref - spectrum reference to set
*/
void putRef(float ref)
{
	// civWriteRef[] = 7 bytes, excluding preamble
	int u, d;											// units & decimals

	// convert to BVD format for CI-V
	if (ref < 0)										// check if float negative

		civWriteRef[5] = 0x01;							// negative
	else
		civWriteRef[5] = 0x00;							// positive

	// convert float to BCD
	ref = abs(ref) * 10;								// allow for 1 decimal
	u = (int)ref / 10;									// units
	d = (int)ref % 100;									// decimal
	civWriteRef[3] = putBCD(u);
	civWriteRef[4] = putBCD(d);

	civWrite(civWriteRef);
}


