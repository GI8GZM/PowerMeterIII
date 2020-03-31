/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/* calibration functions  */

/*---------------------------------calMode()----------------------------------------
coupler calibration mode. shows adc volts directly.
uses Cal samples
*/
void calMode()
{
	lab[civ].stat = false;								// ensure mode != calmode

	for (int i = tuner; i <= freq; i++)					// erase frames as required
		eraseFrame(i);
	eraseFrame(nettPwrMeter);
	eraseFrame(swrMeter);

	for (int i = fwdPwr; i < refVolts + 1; i++)			// restore calibration frames
		restoreFrame(i);

	sprintf(lab[options].txt, "Samples: %d", samplesAvg);
	restoreFrame(options);
}


/*--------------------------- screenCal() -------------------
use this to determine x.y mapping
touch screen. Called during setup 
Note: 320 x 240 pixels
*/
void screenCal()
{
	TS_Point p;												// touch screen result structure
	int tStat;												// touch status

	int xp[5] = { 20, 300, 20,300 };						// test point locations
	int yp[5] = { 20, 20, 220,220 };
	int x = 0, y = 0;
	int xDiff = 0, yDiff = 0;
	char rTxt[50];

	do														// loop until long touch exit
	{
		tft.fillScreen(BG_COLOUR);							// refresh display
		tft.setTextColor(WHITE);
		char txt1[] = "Touch Points, Long Touch to Exit";
		int yPosn = 70;
		tft.setFont(FONT12);
		displayScreenText(txt1, 30);
		tft.setFont(FONT14);

		for (int i = 0; i < 4; i++)
		{
			tft.fillCircle(xp[i], yp[i], 2, FG_COLOUR);		// display test point
			do												// loop until tStat = touched (!=0)
			{
				delay(200);									// might help avoid double touch
				tStat = touch();							// get touched status
				if (tStat == 1) 							// tStatatus: 0 = no touch, 1 = touched, 2 = long touch
				{
					p = ts.getPoint();						// get position result
					x = map(p.x, xMapL, xMapR, 0, 320);
					y = map(p.y, yMapT, yMapB, 0, 240);
				}

				if (tStat == 2)								// long touch - exit
					break;
			} while (tStat == 0);							// loop until screen touched

			tft.fillCircle(xp[i], yp[i], 2, BG_COLOUR);		// cancel test point

			if (tStat == 2)									// check for long touch
				break;										// break from for... loop

			xDiff = xp[i] - x;
			yDiff = yp[i] - y;

			yPosn += 20;
			sprintf(rTxt, "xDiff: %d    yDiff: %d", xDiff, yDiff);
			displayScreenText(rTxt, yPosn);
		}

		if (tStat == 2)										// long touch, break from this loop
			break;

		char txt[] = ("Touch to Continue");					// hold for another touch
		displayScreenText(txt, yPosn + 50);
		while (!touch());									// wait for touch
	} while (tStat != 2);									// loop until long touch

	// exit
	tft.fillScreen(BG_COLOUR);								// refresh display
	char dTxt[] = "Exiting...";
	displayScreenText(dTxt, 200);
	delay(500);
}