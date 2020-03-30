/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*
display functions
drawframe(), displayLabel(), displayValue(), drawMeterScale(), displayMeter()
invertLabel(), eraseFrame();
*/

/*--------------------------------------  displayLabel() --------------------------------------------
display frame and lable at posn
 Calls:	displayLabelStr()
*/
void displayLabel(int posn)
{
	displayLabelStr(posn, lab[posn].txt);
}

/*---------------------------------------  displayLabelStr() ----------------------------------------
displays frame and text at posn
Calls:	drawFrame()
*/
void displayLabelStr(int posn, char* txt)
{
	int x, y;
	frame* fPtr = &fr[posn];
	label* lPtr = &lab[posn];

	if (!fPtr->isEnable) return;							// check enabled

	// draw associated frame
	drawFrame(posn);

	// set label txt colour and font size
	tft.setTextColor(lPtr->colour);
	tft.setFont(lPtr->font);

	// horizontal justify label position in frame
	switch (lPtr->xJustify)
	{
	case 'R':												// right justified
		x = fPtr->x + fPtr->w - tft.strPixelLen(txt) - GAP;
		break;
	case 'C':												// centered
		x = fPtr->x + (fPtr->w - tft.strPixelLen(txt)) / 2;
		break;
	case 'L':												// default -left justified
	default:
		x = fPtr->x + GAP;
	}

	// vertical justify
	switch (lPtr->yJustify)
	{
	case 'T':												// top of frame
		y = fPtr->y + GAP;
		break;
	case 'M':												// middle of frame
		y = fPtr->y + (fPtr->h - lPtr->font.cap_height) / 2;
		break;
	case 'B':												// bottom, default
	default:
		y = fPtr->y + fPtr->h - GAP - lPtr->font.cap_height;
		break;
	}

	// print label text in frame
	tft.setCursor(x, y);									// x, y = top left of text
	tft.print(txt);
}

/*------------------------------  displayValue() --------------------------------------------------
updates value if changed from previous.
use value.updateflg = true to force update
Calls:	dtostrf()  - decimal to string formatted
*/
void displayValue(int posn, float curr)						// frame position, float current value to display
{
	frame* fPtr = &fr[posn];
	value* vPtr = &val[posn];
	label* lPtr = &lab[posn];
	int x, xp, y;
	const int buffSize = 10;								// char buffer size
	char strCurr[buffSize + 1],								// char buffers for converted string
		strPrev[buffSize + 1],
		strKeep[buffSize + 1];
	int pixLenCurr, pixLenPrev, pixLenLabel, pixLenKeep;	// pixel lentghs of string values

	// return if disabled
	if (!fPtr->isEnable) return;

	tft.setFont(vPtr->font);								// set font attribs
	tft.setTextColor(vPtr->colour);

	// convert float values to strings
	int digits = 0;
	dtostrf(vPtr->prevValue, digits, vPtr->decs, strPrev); 	// convert prevValue to char txt
	dtostrf(curr, digits, vPtr->decs, strCurr);   			// convert float to char txt
	if (!vPtr->isUpdate)									// update not forced
		if (!strcmp(strPrev, strCurr))
			return;											// return if no change in value - reduces flicker

	/*
		compare strings left to right. strKeep[] will contain matching character/digits
		use pixel length of strKeep to calculate blanking rectangle  ie - remaning digits to right
		In case 123.00 is confused with 12.3, check the num of digits to left of decimal
	*/

	// initialise keep buffer - this the left->right Value string to keep.
	for (int i = 0; i < buffSize; i++)
		strKeep[i] = '\0';

	for (int i = 0; i < buffSize; i++)
	{		// compare prev to current
		if (strPrev[i] == strCurr[i])
			strKeep[i] = strPrev[i];						// strKeep = Prev if curr different to previous
	}

	// pixel length of value strings
	pixLenCurr = tft.strPixelLen(strCurr);
	pixLenPrev = tft.strPixelLen(strPrev);
	pixLenKeep = tft.strPixelLen(strKeep);
	// different values with same start digit cause problems with erase, eg 10.23 ans 100.34
	// compare first characters and stricng lengths
	if (strPrev[0] == strCurr[0] && pixLenPrev != pixLenCurr) pixLenKeep = 0;

	// check for label position.  adjust value to be middle of free space
	// vertical text justify
	y = fPtr->y + (fPtr->h - vPtr->font.cap_height) / 2;
	if (lPtr->yJustify == 'T')
		y = y + lPtr->font.cap_height / 2;
	if (lPtr->yJustify == 'B')
		y = y - lPtr->font.cap_height / 2;

	// set x, xp depending on xJustify and label position.
	// x is current value position, xp is previous value
	tft.setFont(lPtr->font);								// set label font
	pixLenLabel = tft.strPixelLen(lPtr->txt);				// get label length
	tft.setFont(vPtr->font);								// reset to value font

	// label text centred
	x = fPtr->x + (fPtr->w - pixLenCurr) / 2;
	xp = fPtr->x + (fPtr->w - pixLenPrev) / 2;

	// label text is Left justified
	if (lPtr->xJustify == 'L')
	{
		x = fPtr->x + (fPtr->w + pixLenLabel - pixLenCurr) / 2;
		xp = fPtr->x + (fPtr->w + pixLenLabel - pixLenPrev) / 2;
	}

	// label text is Right justified
	if (lPtr->xJustify == 'R')
	{
		x = fPtr->x + (fPtr->w - pixLenLabel - pixLenCurr) / 2;
		xp = fPtr->x + (fPtr->w - pixLenLabel - pixLenPrev) / 2;
	}

	//Erase previous value from first digit change
	tft.fillRect(xp - 2 + pixLenKeep, y - 2,				// fill rectangle from pixLenKeep
		pixLenPrev - pixLenKeep + 2,
		vPtr->font.cap_height + 5, fPtr->bg);

	// update current value.  overprint entire string.  Not worth trying to print only erased portion
	tft.setCursor(x, y);
	tft.print(strCurr);

	// save to previous value
	vPtr->prevValue = curr;
	// reset update flag
	vPtr->isUpdate = false;
}

/*---------------------------------  displayMeter() --------------------------------------------
Draws the meter bar in the frame
	indicator is used to show peak power.
	set indicator to one for swr
	meter bar thickness is adjusted according to frame size + font
*/
void displayMeter(int posn, float curr, float peak)
{
	frame* fPtr = &fr[posn];								// display value in this frame
	value* vPtr = &val[posn];
	meter* mPtr = &mtr[posn - nettPwrMeter];				// adjust for array position
	int yb;													// meter base line y co-ord
	int ym;													// y co-ord for meter bar
	int x, xPeak;											// x c-ords
	int wCurr, wPrev, wPeak;								// widths
	int thick;
	int span;

	// if frame disabled return
	if (!fPtr->isEnable) return;

	x = fPtr->x + mPtr->xGap + 5;											// match scale start position
	span = fPtr->w - 2 * mPtr->xGap - 5;									// adjust for GAP to frame
	yb = fPtr->y + fPtr->h - mPtr->font.cap_height - mPtr->yGap - 7;		// baseline Y for meter scale (same as drawMeterScale)

	wCurr = map(curr, mPtr->sStart, mPtr->sEnd, 0, span);					// scale for main value
	wPrev = map(vPtr->prevValue, mPtr->sStart, mPtr->sEnd, 0, span);		// scale for previous value
	wPeak = map(peak, mPtr->sStart, mPtr->sEnd, 0, span);					// scale pixels for peak indicator

	//meter bar thickness
	thick = yb - fPtr->y - mPtr->yGap;										// thickness
	ym = fPtr->y + mPtr->yGap;												// y o-ord for meter bar

	// erase previous peak indicator value before drawing power
	xPeak = x + wPeak - mPtr->pkWidth;										// for indicator
	if (mPtr->pkPrevPosn != xPeak)
		tft.fillRect(mPtr->pkPrevPosn, ym, mPtr->pkWidth, thick, mPtr->bColour);	// erase indicator if not same as previous

	// draw meter, erase only changed meter value
	wCurr -= mPtr->pkWidth;													// adjust for peak indicator width
	wPrev -= mPtr->pkWidth;
	if (wCurr < wPrev)														// chech if curr < prev ...
		tft.fillRect(x + wCurr, ym, wPrev - wCurr, thick, fPtr->bg);		// ... erase difference
	else
		//draw new meter value if curr > prev.  draws start line if wCurr = wPrev
		tft.fillRectVGradient(x + wPrev, ym, wCurr - wPrev, thick, mPtr->tColour, mPtr->bColour);

	// peak indicator - don't draw if zero
	if (wPeak > mPtr->pkWidth)
		tft.fillRectVGradient(xPeak, ym, mPtr->pkWidth, thick, mPtr->pkColour, mPtr->bColour);	// draw indicator

	// save current values to previous
	vPtr->prevValue = curr;
	mPtr->pkPrevPosn = xPeak;
}

/*---------------------------------  drawMeterScale() ----------------------------------------------
draws scale for meter.
	Inserts major and minor ticks and tick labels
	writes label for meter function
*/
void drawMeterScale(int posn)
{
	frame* fPtr = &fr[posn];
	meter* mPtr = &mtr[posn - nettPwrMeter];
	int x, xs; 												// x co-ords, xs scale co-ord
	int y, yTxt, yLine;										// y co-ords for text and line
	int span;												// pixel span of meter
	float scale = mPtr->sEnd - mPtr->sStart;				// get scale factor

	if (!fPtr->isEnable)
		return;

	tft.setTextColor(mPtr->colour);
	tft.setFont(mPtr->font);								// set font attribs

	span = fPtr->w - 2 * mPtr->xGap;						// adjust for GAP to frame
	x = fPtr->x + mPtr->xGap;
	y = fPtr->y + fPtr->h - mPtr->yGap;						// base Y for meter (bottom)

	yTxt = y - mPtr->font.cap_height - 2;					// y coord for scale text
	yLine = yTxt - 5;										// y coord for line (allow two rows text)

	// draw base line
	tft.drawFastHLine(x, yLine, span, mPtr->colour);		// draw scale line

	// Minor scale ticks
	for (int i = 0; i < mPtr->minor; i++)
	{		// get positions for tick
		xs = i * span / mPtr->minor + x;					// x co-ord
		tft.drawFastVLine(xs, yLine, 2, mPtr->colour);  	// minor scale ticks
	}

	// Major scale ticks
	for (int i = 0; i < mPtr->major; i++)
	{		// get positions for major ticks
		xs = i * span / mPtr->major + x;					// x co-ord
		tft.drawFastVLine(xs, yLine, 5, mPtr->colour);  	// major scale
		tft.setCursor(xs - 5, yTxt);						// set cursor for text
		if (scale > 10) 									// no decimals, >10
			tft.print(scale * i / (mPtr->major) + mPtr->sStart, 0);	// print major tick values
		else
			tft.print(scale * i / (mPtr->major) + mPtr->sStart, 1);	// print major tick values, one decimal place
	}
	xs = span + x;
	tft.drawFastVLine(xs, yLine, 4, mPtr->colour);  		// major scale last tick

	tft.setCursor(xs - 10, yTxt);							// adjust no end overlap
	if (scale > 10)
		tft.print(float(mPtr->sEnd), 0);					// print end of scale value
	else
		tft.print(float(mPtr->sEnd), 1);					// print end of scale value
}

/*------------------------------- invertLabel() ----------------------------------------
inverts current text colour and background colour
*/
void invertLabel(int posn)
{
	int col;

	col = fr[posn].bg;										// save bg colour
	fr[posn].bg = lab[posn].colour;							// swap colour
	lab[posn].colour = col;
	displayLabel(posn);										// draw label
}

/*------------------------------- drawFrame() ----------------------------------------------
frame position
draws frame if enabled, fills background colour and draws outline
*/
void drawFrame(int posn)
{
	frame* fPtr = &fr[posn];

	// draw frame if enabled
	if (fPtr->isEnable)
	{
		// filled rectangle
		tft.fillRoundRect(fPtr->x, fPtr->y, fPtr->w, fPtr->h, RADIUS, fPtr->bg);
		// draw outline
		if (fPtr->isOutLine)
			tft.drawRoundRect(fPtr->x, fPtr->y, fPtr->w, fPtr->h, RADIUS, LINE_COLOUR);
	}
}

/*------------------------------- eraseFrame() ----------------------------------------------------
   erases frame, outline and text to background
*/
void eraseFrame(int posn)
{
	fr[posn].isEnable = false;							// disable flags
	fr[posn].isTouch = false;
	// fill inside frame with background
	tft.fillRoundRect(fr[posn].x, fr[posn].y,			// erase frame - fill with background colour
		fr[posn].w, fr[posn].h, RADIUS, BG_COLOUR);
}

/*--------------------------------  restoreFrame() -------------------------------------------------
arg: frame position
restores frame after possible mess up such as font change, or overwite by other data
*/
void restoreFrame(int posn)
{
	eraseFrame(posn);									// erases and disables frame
	fr[posn].isEnable = true;							// enable flags
	fr[posn].isTouch = true;
	drawFrame(posn);									// redraw frame
	displayLabel(posn);									// redisplay label
	val[posn].isUpdate = true;							// force value redraw
}