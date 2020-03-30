/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2019  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*------------------------------------splashScreen() --------------------
displays startup messages, allows for screen calibration
*/
void splashScreen()
{
	char line0[] = "SWR / Power Meter";
	char line1[] = "+ Icom CI-V Control";
	char line2[] = "by GI8GZM";
	char line3[] = VERSION;							// display current version
	char line4[] = "Touch to Calibrate Screen";

	tft.fillScreenVGradient(ALT_BG, BLUE);
	tft.setTextColor(WHITE);

	tft.setFont(FONT24);
	displayScreenText(line0, 50);

	tft.setFont(FONT18);
	displayScreenText(line1, 100);
	displayScreenText(line2, 150);

	tft.setFont(FONT14);
	displayScreenText(line3, 10);
	displayScreenText(line4, 220);

	// delay - allows for screen touch to call screenCal()
	for (int i = 0; i < SPLASH_DELAY; i += 10)
	{
		if (!ts.tirqTouched())
		{
			screenCal();						// go to screen calibration
			break;								// break from for.. loop
		}
		delay(10);								// delay 10 millisecs
	}
}

/*---------------------------------- displayScreenText() ---------------------------------------
displays lines of horizantly centered text , at vertical y
*/
void displayScreenText(char* line, int y)
{
	int x = (320 - tft.strPixelLen(line)) / 2;
	tft.setCursor(x, y);
	tft.print(line);
}

/*--------------------------------- tunerBandOpts() --------------------------------------------
Options 0 - enter here after long touch on FreqTune or ABand buttons
calls freqTimeOpts() to draw check circles/ boxes for tuning and auto band
calls setParamOpts to set tuner frequency difference and band change time
*/
void tunerBandOpts()			// set frequency difference to trigger autotune
{
	int tNum, chkNum;				// drawTouchBoxOpts Number, number selected box

	// loop until Exit touched
	do
	{
		tft.fillScreen(ALT_BG);
		tft.setTextColor(WHITE);

		// reset chkNum for loop to work
		chkNum = 0;

		// draw check boxes for startBand to endBand + Exit & More..
		tNum = drawTuneBandOpts();

		// clear touch buffer
		//while (ts.touched());

		// accept option changes until Exit or More.. is touched
		do
		{
			if (ts.tirqTouched())
			{
				chkNum = chkParamOpts(tNum);
				int bNum = chkNum / 2;				// band number

				// get number of item touched. Ignore -1 (touched but no item)
				if (chkNum < NUM_BANDS * 2 && chkNum != -1)
				{
					// even number selected - freqTune Options
					if (!(chkNum % 2))
					{
						hfBand[bNum].isTtune = !hfBand[bNum].isTtune;
						drawCircleOpts(tb[chkNum].x, tb[chkNum].y, hfBand[bNum].isTtune, chkNum);
					}
					// odd number - freqTune Options
					else
					{
						hfBand[bNum].isABand = !hfBand[bNum].isABand;
						drawCircleOpts(tb[chkNum].x, tb[chkNum].y, hfBand[bNum].isABand, chkNum);
					}
					// update EEPROM
					hfProm[bNum].isTtune = hfBand[bNum].isTtune;
					hfProm[bNum].isABand = hfBand[bNum].isABand;
					putBandEEPROM(bNum);					// save data to EEPROM
				}
			}
		} while (chkNum < NUM_BANDS * 2);

		// bNum is either Exit or More..(last touch item - tNum)
		if (chkNum == tNum)
			setParamOpts();		// More... selected
	} while (chkNum != (tNum - 1));		// Exit

	// done
	initDisplay();
}

/*--------------------------- drawTuneBandOpts() --------------------
Options screen 1
drawTuneBandOpts() - draw check circles for AutoBand options
returns: number off check boxes including Exit and More...
*/
int drawTuneBandOpts()
{
	int x = 20, y = 70;
	const int xd = 50, yd = 30;
	int i = 0;								// drawTouchBoxOpts index
	int w;
	int xStart = 20, yStart = 70;
	int colWidth = 170;
	//TS_Point p;							// touch screen result structure

	// screen header, centred
	tft.setFont(FONT14);
	char txt[] = "Freq Tuning and AutoBand Enable";
	tft.setCursor((320 - tft.strPixelLen(txt)) / 2, 10);
	tft.printf(txt);

	// column labels
	tft.setFont(FONT12);
	tft.setCursor(5, 40);
	tft.printf("Tune  ABand    Band");
	tft.setCursor(5 + colWidth, 40);
	tft.printf("Tune  ABand    Band");

	// check boxes (circles actually!)  two per band
	x = xStart;
	y = yStart;
	for (i = 0; i < NUM_BANDS; i++)
	{
		// new set?
		if (i >= 6)
		{
			x = xStart + colWidth;
			y = yStart + yd * (i - 6);
		}
		else
			y = yStart + yd * i;

		// tuneFlg are even, two circles per line
		drawCircleOpts(x, y, hfBand[i].isTtune, i * 2);

		// space to next column
		x += xd;
		drawCircleOpts(x, y, hfBand[i].isABand, i * 2 + 1);

		// display band
		tft.setCursor(x, y - 5);
		tft.printf("%8d%s", hfBand[i].mtrs, " m");

		x -= xd;			// reset x
	}

	// allow for 2 items per line, starting at 0
	i = i * 2;
	// draw exit boxes
	x = 170; y = 210;
	w = drawTouchBoxOpts(x, y, "Exit", i++);
	x += w;
	w = drawTouchBoxOpts(x, y, "More...", i);

	// return max drawTouchBoxOpts index
	return i;
}

/*--------------------------------- setParamOpts() -------------------------------------------------
options screen 2
set tune freq difference and ft8 band change time
*/
void setParamOpts()
{
	int x, y;
	int i = 0, n = 0;
	bool isFlg;

	tft.fillScreen(ALT_BG);
	tft.setTextColor(WHITE);

	// Draw Tuner Freq Difference section
	i = setParam(freqTuneOpt, "Tuner Freq Diff:", i);
	displayValue(freqTuneOpt, freqTunePar.val);
	i = setOptFlg(freqTuneOpt, "On at Start Up", freqTunePar, i);

	// Draw AutoBand section
	i = setParam(aBandTimeOpt, "AutoBand Timer:", i);
	displayValue(aBandTimeOpt, aBandPar.val);
	i = setOptFlg(aBandTimeOpt, "On at Start Up", aBandPar, i);

	// draw exit box
	x = 135; y = 210;
	drawTouchBoxOpts(x, y, "Exit", i);

	// touch screen options
	//while (ts.touched());			// clear touch buffer

	// check for touch changes
	do
	{
		// check which box touched
		n = chkParamOpts(i);
		isFlg = false;

		switch (n)
		{
			// increase / decrease frquency
		case 0:
			freqTunePar.val += 20;
			break;
		case 1:
			freqTunePar.val -= 20;
			if (freqTunePar.val <= 0)
				freqTunePar.val = 20;
			break;
		case 2:
			// toggle stat
			freqTunePar.isFlg = !freqTunePar.isFlg;			// set/reset Freq Tune Flag for startup
			isFlg = freqTunePar.isFlg;
			drawCircleOpts(tb[n].x, tb[n].y, isFlg, n);
			break;

			// increase/decrease time
		case 3:
			aBandPar.val += 15;
			break;
		case 4:
			aBandPar.val -= 15;
			if (aBandPar.val <= 0)
				aBandPar.val = 15;
			break;
		case 5:
			aBandPar.isFlg = !aBandPar.isFlg;
			isFlg = aBandPar.isFlg;
			drawCircleOpts(tb[n].x, tb[n].y, isFlg, n);
			break;

			// Exit box
		case 6:
		default:
			break;
		}

		// update display
		displayValue(freqTuneOpt, freqTunePar.val);
		displayValue(aBandTimeOpt, aBandPar.val);

		// update structures to EEPROM
		putParEEPROM(freqTunePar);
		putParEEPROM(aBandPar);
		//EEPROM.put(freqTunePar.eeAddr, freqTunePar);
		//EEPROM.put(aBandPar.eeAddr, aBandPar);
	} while (n != i);		// last item is Exit

	// clean up
	eraseFrame(freqTuneOpt);
	eraseFrame(aBandTimeOpt);
}

/* --------------------------------- chkParamOpts() -------------------------------------------
chkParamOpts()
args: num - number offSet items to check, optionBox pointer
returns number of box touched. -1 if touched but not checkBox item
*/
int chkParamOpts(int n)
{
	int x = 0, y = 0;						// local variables
	int i;									// item touched
	int tStatus = 0;
	TS_Point p;								// touch screen result structure
	bool isTouch = false;

	do
	{
		tStatus = touched();					// check for touch
		if (!tStatus) 							// tStatus: 0 = no touch, 1 = touched, 2 = long touch
			return -1;							// false/no touch

		p = ts.getPoint();
		// map(value, fromLow, fromHigh, toLow, toHigh). defined in touchOptions.h
		x = map(p.x, xMapL, xMapR, 0, 320);
		y = map(p.y, yMapT, yMapB, 0, 240);

		// get touch area that was touched
		for (i = 0; i <= n; i++)
		{
			// x,y between frame width and height with offset
			if (x > tb[i].x - T_OFFSET && x < tb[i].x + T_OFFSET)
			{
				if (y > tb[i].y - T_OFFSET && y < tb[i].y + T_OFFSET)
				{
					isTouch = true;
					break;
				}
			}
		}
	} while (!isTouch);

	// make sure touch buffer empty
	//while (ts.touched());

	return i;		// return item touched
}

/*----------------------------------------setParam()--------------------------------------
	draw text and value, +/- buttons
	returns touch index = original i+2
*/
int setParam(int posn, const char* txt, int index)
{
	int x = 30, y;
	ILI9341_t3_font_t fnt = FONT14;

	tft.setFont(fnt);
	tft.setTextColor(WHITE);
	y = fr[posn].y + fnt.cap_height;
	tft.setCursor(x, y);

	tft.printf(txt);
	restoreFrame(posn);

	// draw plus/minus underneath
	drawPlusMinusOpts(posn, index);
	index += 2;									// two items: Plus & Minus
	return index;
}

/*------------------------------setOptFlg()----------------------------------------
*/
int setOptFlg(int posn, const char* txt, param par, int i)
{
	int x = 40, y;

	tft.setFont(FONT14);
	tft.setTextColor(WHITE);
	y = fr[posn].y + 50;
	drawCircleOpts(x, y, par.isFlg, i);
	i++;
	tft.setCursor(x, y - 5);

	tft.print("     ");
	tft.printf(txt);

	return i;
}

/*---------drawPlusMinusOpts()------------------------------------
args: x, position, ti is touch index
draws +, - boxes using symbol font.  Reversed +/- automatic
adds offsets to allow for circle position
saves offset locations in tb[i].x and tb[i].y
*/
void drawPlusMinusOpts(int posn, int ti)
{
	int x, y;
	int offSet = 15;
	frame* fPtr = &fr[posn];

	//	x = fPtr->x + BW + 5;
	x = fPtr->x + fPtr->w + 5;
	y = fPtr->y - 5;

	tft.setFont(FONT_PM);
	tft.setTextColor(WHITE);

	tft.setCursor(x, y);
	tft.write(PLUS_SYMBOL);
	x += offSet;
	y += offSet;
	//tft.drawCircle(x, y, radius, RED);
	tb[ti].x = x;
	tb[ti].y = y;
	ti++;

	//x = x + 30;
	x = fPtr->x + fPtr->w + 5;
	y = y + 10;
	tft.setCursor(x, y);
	tft.write(MINUS_SYMBOL);
	x += offSet;
	y += offSet;
	//tft.drawCircle(x, y, radius, GREEN);
	tb[ti].x = x;
	tb[ti].y = y;
}

/*------------------------------------drawTouchBoxOpts()----------------------------
args: x,y, text in box, drawTouchBoxOpts[num]
returns:	box width
*/
int drawTouchBoxOpts(int x, int y, const char* txt, int i)
{
	int r = 5;
	int s, w, h;						//  string length, box width, height adjustment
	ILI9341_t3_font_t fnt = FONT16;
	char t[10];

	strcpy(t, txt);
	// set font, get width and height from font & text
	tft.setFont(fnt);
	s = tft.strPixelLen(t);							// width of string
	w = s + 20;										// box width
	h = fnt.cap_height + 10;
	tft.drawRoundRect(x, y, w, h, r, WHITE);
	tft.setCursor(x + (w - s) / 2, y + 5);
	tft.print(txt);

	tb[i].x = x + w / 2;
	tb[i].y = y + 10;

	// return width
	return w;
}

/*------------------drawCircleOpts()---------------------------------------
x, y; position
flag: 1 = filled circle
ti: touch box index
*/
void drawCircleOpts(int x, int y, bool isFlg, int ti)
{
	int r = 10;

	if (isFlg)
	{
		tft.fillCircle(x, y, r, WHITE);
		tft.drawCircle(x, y, r, WHITE);
	}
	else
	{
		tft.fillCircle(x, y, r, ALT_BG);			// alternate B/Ground
		tft.drawCircle(x, y, r, WHITE);
	}
	tb[ti].x = x;
	tb[ti].y = y;
}

/*--------------------------------------setSamples()--------------------------------------------
set samples options
measure - samples register size, default & alternate
calibrate - samples register size
*/
void setSamples()
{
	int x, y;
	int i = 0;
	int n;
	int samplesStat;									// samplesAvg status

	if (samplesAvg == samplesDefPar.val)				// save current samplesAvg setting
		samplesStat = 1;								// restore on exit
	if (samplesAvg == samplesAltPar.val)
		samplesStat = 2;
	if (samplesAvg == samplesCalPar.val)
		samplesStat = 3;

	tft.fillScreen(ALT_BG);
	tft.setTextColor(WHITE);

	// screen header, centred
	tft.setFont(FONT14);
	char txt[] = "Cyclic Bufffer Samples";
	tft.setCursor((320 - tft.strPixelLen(txt)) / 2, 10);
	tft.printf(txt);

	i = setParam(samplesDefOpt, "Default Samples:", i);			// returns new i
	displayValue(samplesDefOpt, samplesDefPar.val);

	i = setParam(samplesAltOpt, "Alternate Samples", i);		// returns new i
	displayValue(samplesAltOpt, samplesAltPar.val);

	i = setParam(samplesCalOpt, "Calibrate Samples", i);
	displayValue(samplesCalOpt, samplesCalPar.val);

	x = 135; y = 210;
	drawTouchBoxOpts(x, y, "Exit", i);

	// touch screen options
	do
	{
		n = chkParamOpts(i);						// check which box touched
		switch (n)
		{
		case 0:										// increment sample size, limit to max
			if (samplesDefPar.val == 1)
				samplesDefPar.val += 99;
			else
				samplesDefPar.val += 100;
			if (samplesDefPar.val >= MAXBUF)
				samplesDefPar.val = MAXBUF;
			break;
		case 1:										// decrement sample size, min = 1
			samplesDefPar.val -= 100;
			if (samplesDefPar.val <= 1)
				samplesDefPar.val = 1;
			break;
		case 2:										// increment sample size, limit to max
			if (samplesAltPar.val == 1)
				samplesAltPar.val += 99;
			else samplesAltPar.val += 100;
			if (samplesAltPar.val >= MAXBUF)
				samplesAltPar.val = MAXBUF;
			break;
		case 3:										// decrement sample size, min = 1
			samplesAltPar.val -= 100;
			if (samplesAltPar.val <= 1)
				samplesAltPar.val = 1;
			break;
		case 4:										// increment calibrate sample size
			if (samplesCalPar.val == 1)
				samplesCalPar.val += 99;
			else samplesCalPar.val += 100;
			if (samplesCalPar.val >= MAXBUF)
				samplesCalPar.val = MAXBUF;
			break;
		case 5:										// decrement calibrate sample size, min = 1
			samplesCalPar.val -= 100;
			if (samplesCalPar.val <= 1)
				samplesCalPar.val = 1;
			break;
		case 6:
		default:
			break;
		}

		// re-display parameters
		displayValue(samplesDefOpt, samplesDefPar.val);
		displayValue(samplesAltOpt, samplesAltPar.val);
		displayValue(samplesCalOpt, samplesCalPar.val);

		// save to EEPROM
		putParEEPROM(samplesDefPar);
		putParEEPROM(samplesAltPar);
		putParEEPROM(samplesCalPar);
	} while (n < i);								// do while touched item is less than toatl items

	// clean up
	eraseFrame(samplesDefOpt);
	eraseFrame(samplesAltOpt);
	eraseFrame(samplesCalOpt);

	switch (samplesStat)							// reset samplesAvg using new values
	{
	case 1:
		samplesAvg = samplesDefPar.val;
		break;
	case 2:
		samplesAvg = samplesAltPar.val;
		break;
	case 3:
		samplesAvg = samplesCalPar.val;
	}

	initDisplay();									// reset display
}