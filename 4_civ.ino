/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  � Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/* Icom CI-V functions

*/


/*--------------------------- getFreq() ----------------------------------------------------
read CI-V frequency
issues CI-V command to read frequency from radio and tehn decoded the
frequency returned.
Returns: frequency or 0 if fail
Global: inBuff, readFreq
Calls: civRead(), decodeBCD()
  */
float getFreq()
{
	int n;									// chars read into budder
	float f;
	int inBuff[12];						// civ frequency inBuff buffer

	n = civWrite(civReadFreq);					// request read frequency from radio
	if (n == 0)								// timed out
		return (0);

	n = civRead(inBuff);					// buffer, printflg
	if (inBuff[2] == CIVADDR && inBuff[n - 1] == 0xFD)	// check format of serial stream
	{
		f = (decodeBCDFreq(inBuff) / 1000);		// decode frequency  and convert to kHz
		return (f / 1000);                  // return MHZ
	}
	else
		return (0);							// nothing read, return 0
}

/*--------------------------- getBand() --------------------------------------------------------------------
compares to HF band table start and end band limits
arg: float frequency (MHz).
returns: hfband band number
*/
int getBand(float freq)
{
	int cBand = -1;							// local band number
	static int pBand;
	int flg = false;

	// get band   -1 = no band, 0=160mtrs, 1=80mtrs, etc.
	for (int i = 0; i < NUM_BANDS; i++)
	{
		if (freq >= hfBand[i].bandStart && freq <= hfBand[i].bandEnd)
		{
			cBand = i;
			flg = true;
			break;
		}
	}

	if (cBand == pBand)				// compare to previousBand
		return(cBand);						// return if no change

	// update label with band txt
	if (!flg)
	{
		char txt[] = "No Band ";
		displayLabelStr(band, txt);
		cBand = -1;
	}
	else
	{
		// do not use restoreFrame(band);
		displayLabel(band);
		val[band].updateFlg = true;			// force value update
	}

	// set global bandCurr & return
	pBand = cBand;					// save to previousBand
	return(cBand);
}





/**************************  civ functions ********************************/

/*------------------------------ civRead() -------------------------------------------------
read CIV stream from radio
reads CI-V stream from the radio until end of frame (oxFD) received
NOTE: delay required between character reads to allow for buffer fill
Returns: number of characters received or 0 if fail
Global: inBuff
Calls:
*/
int civRead(int* buff)						// read into buff, serial print if flg set
//int civRead(char* buff)						// read into buff, serial print if flg set
{
	int n = 0;
	char inChar = '\0';						// incoming character
	bool timeOutFlg = false;

	civTimeOut.reset();						// set timeout timer
	do
	{
		delay(CIV_READ_DELAY);				// delay for read buffer
		if (civSerial.available() > 0)      // character waiting?
		{
			inChar = civSerial.read();	    // read it - empties input buffer
		}
		if (civTimeOut.check())
		{
			timeOutFlg = true;				// timedout, set flg
			break;							// break from loop
		}
	} while (inChar != 0xFE && !timeOutFlg);	// preamble start
	buff[n++] = inChar;						// load buff with next character
	if (timeOutFlg)
		return (0);							// timeout occurred, return 0

	// get rest of frame
	civTimeOut.reset();						// set timeout timer
	do
	{
		delay(CIV_READ_DELAY);
		if (civSerial.available() > 0) 		// character waiting?
		{
			inChar = civSerial.read();		// read it - empties input buffer
		}
		if (civTimeOut.check())				// timeout if looping too long
		{
			timeOutFlg = true;				// set timeout flg
			break;
		}
		buff[n++] = inChar;					// load buff with next character
	} while (inChar != 0xFD && !timeOutFlg);	// check for end of inBuff (0xFD)

	if (timeOutFlg)
		return (0);							// timeout occurred, return 0
	else
		return (n);							// n = number of characters read, 0 = error
}

/* ------------------------------ civWrite() ---------------------------------------------------------------
send CI-V command to radio
sends inBuff data to radio, character at a time ip to end character (0xFD)
Returns: n = chars written, 0 if error
Global: inBuff
Calls:
*/
//int civWrite(const char* buff)
int civWrite(int* buff)
{
	int n = 0;
	char inChar = '\0';						// incoming, read char
	bool timeOutFlg = false;

	// flush send buffer
	civSerial.flush();

	civTimeOut.reset();						// set timeout timer

	// send civ write preamble
	civSerial.write(civPreamble[0]);		// send first char
	delay(CIV_WRITE_DELAY);					// just one delay required

	for (int i = 1; i < 4; i++)				// send rest of 4 char preamble
	{
		civSerial.write(civPreamble[i]);
		do
		{
			if (civSerial.available() > 0) 	// character waiting?
			{
				inChar = civSerial.read();
			}
			if (civTimeOut.check())
			{
				timeOutFlg = true;
				break;
			}
		} while (inChar != civPreamble[i] && !timeOutFlg);	// compare to written char
	}

	if (timeOutFlg)							// timed out, return n=0 bytes
		return (0);

	// send rest of ci-v command
	civTimeOut.reset();						// set timeout timer
	do
	{										// send code + end character
		civSerial.write(buff[n]);			// write character

		do
		{
			// delay(CIV_DELAY);
			if (civSerial.available() > 0) 	// character waiting?
			{
				inChar = civSerial.read();
			}
		} while (inChar != buff[n]);
		if (civTimeOut.check())
		{
			timeOutFlg = true;
			break;
		}
	} while (buff[n++] != 0xFD && !timeOutFlg);	// do until end of buff

	if (timeOutFlg)
		return (0);
	else
		return (n);							// return number of bytes
}

/*------------------------------------ decodeBCDFreq() -----------------------------------------
 function to decode frequency data.
  Returns: frequency in Hz or 0 if fail
  Global: none
  Calls: none
  Called By: civreadFreq()
*/
float decodeBCDFreq(int* buff)
{
	unsigned long m, k1, k2, h;

	m = (buff[8]);
	m = m - m / 16 * 6;					// calculate MHz
	k1 = buff[7];
	k1 = k1 - k1 / 16 * 6;				// calculate KHz, first part
	k2 = buff[6];
	k2 = k2 - k2 / 16 * 6;				// calculate second part, kHz
	h = buff[5];
	h = h - h / 16 * 6;

	return ((m * 1000000) + (k1 * 10000)
		+ (k2 * 100) + h);				// freq variable stores frequency in MMkkkH  format
}

/* --------------------------------- encodeBCDFreq() -----------------------------------
function to encode float freq to CI-V format.
buff[4] - buff[8]	Called by: initAutoBand(), autoBand()
*/
void encodeBCDFreq(int* buff, float f)		// buffer, value   set Frequency
{
	long  x;

	f = f * 1000000;					// covert to Hz
	x = int(f / 1000000);				// maths to convert to BCD Hex
	buff[4] = (int(x / 10) * 16 - int(x / 10) * 10 + x);	// encode MHz

	f = f - x * 1000000;
	x = int(f / 10000);
	buff[3] = (int(x / 10) * 16 - int(x / 10) * 10 + x);	// encode KHz

	f = f - x * 10000;
	x = int(f / 100);
	buff[2] = (int(x / 10) * 16 - int(x / 10) * 10 + x);	// encode KHz

	f = f - x * 100;
	x = int(f / 1);
	buff[1] = (int(x / 10) * 16 - int(x / 10) * 10 + x);	//encode Hz
}

/*---------------------------------- civPrintBuffer() --------------------------------
diagnostic - prints contents of civ buffer
*/
void civBuffPrint(int* buff)
{
	int n = 0;
	while (buff[n] != 0xFD)
	{
		Serial.print(buff[n], HEX);
		Serial.print(" ");
		n++;
	}
	Serial.print(buff[n++], HEX);

	Serial.print("      Chars: ");
	Serial.println(n,DEC);
}

