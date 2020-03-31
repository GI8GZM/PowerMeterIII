/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*------------------------------------------------------------------------------------------
   XPT2046 touch functions
   Uses XPT2046 interrupts, check for a touch (ts.tirqTouched())
   Interrupt driven - only get here if screen touched
*/

/*------------------------------- touched() ------------------------------------------------------------
check for screen touch
returns 0 = no touch, 1 = short touch, 2 = long touch
*/
int touched()
{
	int status = 0;

	if (ts.tirqTouched())									// interrup. screen was touched
	{
		if (ts.touched())									// +ve touch
		{
			status = 1;										// status = short touch

			// check for long touch
			longTouchTimer.reset();							// Metro timer
			while (ts.touched() && status < 2)
			{
				if (longTouchTimer.check())					// long touch timer
					status = 2;
			}
		}
	}
	return status;
}

/*-------------------------------- touchChk() -----------------------------------------------------------
checks for touch on enabled frames
arg: num fo frames
*/
void touchChk(int i)
{
	int x = 0, y = 0;							// local variables
	TS_Point p;									// touch screen result structure
	int tStatus;								// 0= not touched, 1 = shorttouch, 2 = longtouch,
	bool isTouch = false;

	tStatus = 1;								// default short touch. 0= not touched, 1 = shorttouch, 2 = longtouch

	do
	{
		tStatus = touched();					// check for touch
		if (!tStatus) 							// tStatus: 0 = no touch, 1 = touched, 2 = long touch
			return;
		p = ts.getPoint();						// get position result

		// map(value, fromLow, fromHigh, toLow, toHigh). defined in touchOptions.h
		x = map(p.x, xMapL, xMapR, 0, 320);
		y = map(p.y, yMapT, yMapB, 0, 240);

		// get frame that was touched
		for (i = 0; i <= i; i++)				// for all frames
		{
			if (x > fr[i].x && x < (fr[i].x + fr[i].w)		// x,y between frame width and height
				&& y > fr[i].y && (y < fr[i].y + fr[i].h))
			{
				// touch enabled frame? break on first occurance for similar posn frames
				if (fr[i].isTouch)
				{
					touchActions(i, tStatus);
					isTouch = true;
					break;
				}
			}
		}
	} while (!isTouch);

	// empty touch buffer for excess long touch
	while (ts.touched());
}

/*--------------------------------- touchActions() --------------------------------------------------------------
actions to take when frame is touched
 arg: i = frame position, tStat = 0 (program call), 1 = normal/short touch, 2 = long touch
*/
void touchActions(int n, int tStat)				// touch actions for frame i, touch status
{
	// code for each detected frame
	switch (n)
	{
	case nettPwr:								// frame 0
		nettPwrButton(tStat);
		break;

	case peakPwr:
		peakPwrButton(tStat);
		break;

	case dBm:									// switch from power to dBm
		dBmButton(tStat);
		break;

	case swr:
		swrButton(tStat);
		break;

	case nettPwrMeter:							// swap with swrmeter
		eraseFrame(nettPwrMeter);
		restoreFrame(swrMeter);
		drawMeterScale(swrMeter);
		break;

	case swrMeter:								// swap with nettPwrMeter
		eraseFrame(swrMeter);
		restoreFrame(nettPwrMeter);
		drawMeterScale(nettPwrMeter);
		break;

	case freqTune:								// freqTune button
		if (tStat == 2) tunerBandOpts();		// long press
		else freqTuneButton(tStat);
		break;

	case aBand:									// auto band button
		if (tStat == 2)
			tunerBandOpts();					// long press
		else
			aBandButton(tStat);					// short press
		break;

	case options:								// calibrate mode button
		if (tStat)
			optionsButton(tStat);
		break;

	case tuner:									// activate the radio tuner
		restoreFrame(tuner);
		lab[tuner].stat = true;					// enable tuning
		tunerActivate();
		break;

	case freq:									// display frequency (MHz) or band (mtrs)
	case band:
		bandButton(tStat);
		break;

	case txPwr:									// txPwr active, so disable it and enable ref
		txPwrButton(tStat);
		break;

	case sRef:									// radio spectrum reference level
		sRefButton(tStat);
		break;

	default:									// default - do nothin
		break;
	}
}