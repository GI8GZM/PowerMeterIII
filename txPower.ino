/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*  Radio %Pwr control functions 
civ txpwr is 0-255. Represents 0-100%  
Only map before display to prevent accumulated rounding errors
*/


/*--------------------------- txPwrButton() ----------------------------------------------------------------
short touch - toggles set TXPower between original setting and 100%
long touch  - changes to spectrum reference
*/
void txPwrButton(int tStat)
{
	int pwr = 0;
	static int prevPwr = 0;								// prev setting
	static bool pwrSetFlg;								// pwr set flag

	if (tStat == 2)										// long touch
	{
		eraseFrame(txPwr);								// erase power
		restoreFrame(ref);								// display ref
	}
	else												// short touch, change power setting
	{
		if (!pwrSetFlg)
		{
			// set %Tx Power  = 100%
			val[txPwr].colour = RED;					// change colour
			prevPwr = getTxPwr();						// save current power setting
			putTxPwr(255);								// set power to 100%
			pwrSetFlg = true;							// set flag
		}
		else
		{
			// reset %Tx Power to previous level
			val[txPwr].colour = CIV_COLOUR;				// set normal colour
			putTxPwr(prevPwr);							// restore to previous
			pwr = prevPwr;
			pwrSetFlg = false;							// reset flag
		}

		pwr = map(pwr, 0, 255, 0, 100);					// map before display, avoid rounding errors
		displayValue(txPwr, pwr);						// dispaly power on display
	}
}


/*-------------------------------- getTxPwr() --------------------------------------------------------
reads RF Power setting from radio
Returns pwr = 0-255 (0-100%)
*/
int getTxPwr()
{
	int n;
	unsigned int n0 = 0, n1 = 0;						// chars read into budder
	int inBuff[12];										// civ frequency inBuff buffer
	int pwr = 0;

	civWrite(civReadPwrSet);							// request read power setting from radio
	n = civRead(inBuff);								// get number of characters in buffer (9)
	if (inBuff[2] == CIVADDR && inBuff[n - 1] == 0xFD)	// check format of serial stream
	{
		n0 = (inBuff[n-3] / 16) * 10 + inBuff[n-3] % 16;	// convert from BCD
		n1 = (inBuff[n-2] / 16) * 10 + inBuff[n-2] % 16;
	}
	pwr = n0 * 100 + n1;
	return(pwr);
}

/*------------------------------ putTxPwr() -------------------------------
set Tx %power, 0-100.
range 0-255 (= 0-100%), converts to BCD, write C-IV command
returns pwr
*/
void putTxPwr(int pwr)	
{
	int n0, n1;											// intermediate conversion

	n0 = (int)pwr % 100;								// covert to BCD
	n1 = ((pwr * 100) - n0 * 100) / 10000;
	civWritePwrSet[2] = (n1 / 10) * 16 + (n1 % 10);
	civWritePwrSet[3] = (n0 / 10) * 16 + (n0 % 10);

	civWrite(civWritePwrSet);							// write it, 0-255
}


