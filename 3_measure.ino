/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

/*--------------------------- measure() ------------------------------------------
   reads ADC values recorded by ADC using interrupt timer getADC().
   calculates forward, reflected power, nett power, peak envelope power
   peak power calculated and held for 3-4 secs
   swr calculated from fwd and reflected power
   Calls: pwrCalc()
*/
void measure()
{
	unsigned long cr0, cr1, cr0Pk, cr1Pk;
	float vf, vr, vfp, vrp;
	float fPwr, rPwr, nPwr,								// calculated forward, reflected, nett power
		fPkPwr, rPkPwr, pep,							// pep powers
		dbm = 0,										// dBm
		swrV = 1.0;										// vSWR calculated from voltage
	float pkPwr = 0;									// PEAK POWER

	lab[nettPwr].stat = true;							// set true for nettPwr colour change

	do
	{
		// r0, r1 etc are measured independantly by timer interrupt function getADC()
		// come here to check results
		noInterrupts();									// stop interrupts while copying data
		cr1 = buf1Tot;									// buffer totals
		cr0 = buf0Tot;
		cr1Pk = buf1Pk;
		cr0Pk = buf0Pk;
		interrupts();									// restart interrupts

		// voltage calculations
		{
			vf = cr1 * 3.3 / adc->adc0->getMaxValue() / samplesAvg - VF_ZEROADJ;		// fwd volts zero Adjusted
			vr = cr0 * 3.3 / adc->adc1->getMaxValue() / samplesAvg - VR_ZEROADJ;		// ref volts zero adjusted
			vfp = cr1Pk * 3.3 / adc->adc0->getMaxValue() - VF_ZEROADJ;	// fwd volts peak
			vrp = cr0Pk * 3.3 / adc->adc1->getMaxValue() - VR_ZEROADJ;	// ref volts peak
		}
		//calculate power via rms voltage
		//fPwr = pwrRmsCalc(vf);
		//rPwr = pwrRMSCalc(vr);
		//fPkPwr = pwrRmsCalc(vfp);
		//rPkPwr = pwrRmsCalc(vrp);

		//calculate power directly from volts
		fPwr = pwrCalc(vf, 'F');						//  fwd volts
		rPwr = pwrCalc(vr, 'R');						//  ref volts
		fPkPwr = pwrCalc(vfp, 'F');						// peak power, fwd
		rPkPwr = pwrCalc(vrp, 'R');						// pk pwr, ref

		// nett power
		nPwr = fPwr - rPwr;								// nett power to antenna
		if (nPwr < 0)
			nPwr = 0.0;									// no -ve power

		// pep
		pep = fPkPwr - rPkPwr;
		if (pep < nPwr)									//  pep can be less than nPwr if peak ref is high
			pep = nPwr;
		if (pep < 0)
			pep = 0.0;

		// select pep or average peak
		if (lab[peakPwr].stat)							// 0 - pep, 1 - peak average
		{
			if (nPwr >= pkPwr)
				pkPwr = nPwr;
			else if (pkPwrTimer.check())				// check peak power hold timer
			{											// come here if pkPwr < nPwr and timer expired
				pkPwr = nPwr;							// drop back down
				pkPwrTimer.reset();						// reset timer
			}
		}
		else
		{
			if (pep >= pkPwr)							// record peak pep
				pkPwr = pep;
			else if (pepTimer.check())
			{
				pkPwr = pep;
				pepTimer.reset();
			}
		}

		// dBm =  10x log10 (1000 x watts)
		dbm = 10 * log10(nPwr * 1000);					// use nett power
		if (dbm < 0)									// can't be -ve
			dbm = 0.0;

		// swr calculation
		if (nPwr > PWR_THRESHOLD)						// power on and vf > vr
		{
			//float rc = sqrt(rPwr / fPwr);				// reflection coefficient
			//swrV = (1 + vrAvg / vfAvg) / (1 - vrAvg / vfAvg);
			//swrV = (vfAvg + vrAvg) / (vfAvg - vrAvg);
			//swrV = (vfPk + vrPk) / (vfPk - vrPk);

			float rc = sqrt(rPkPwr / fPkPwr);			// reflection coefficient (peak power)
			if (rc == NAN || isnan(rc))					// check for valid calculation
				swrV = 1.0;
			else										// seems ok?
			{
				swrV = (1 + rc) / (1 - rc);				// swr calc
				if (swrV <= 1.0)
					swrV = 1.0;							// trap errors
				if (swrV > 999.9)
					swrV = 999.9;						// maximum swr display 999.9
			}

			// set colours for SWR display
			int sc = GREEN;
			if (swrV > 1.5)
				sc = YELLOW;
			if (swrV > 2)
				sc = ORANGE;
			if (swrV > 3)
				sc = RED;
			val[swr].colour = sc;
			displayValue(swr, swrV);					// draw swr while power on
		}

		// if power is on
		if (nPwr > PWR_THRESHOLD)
		{
			// display nett power.  if power on, use RED background. runs once if power is on.
			if (fr[nettPwr].isEnable && lab[nettPwr].stat)
			{	// stat true. used to control change of backgroud colour
				fr[nettPwr].bg = RED;
				restoreFrame(nettPwr);
				lab[nettPwr].stat = false;				// ensure doesn't change to RED next time
			}
		}

		// display all enabled
		displayValue(nettPwr, nPwr);					// display Watts value
		displayValue(dBm, dbm);
		displayValue(peakPwr, pkPwr);
		displayValue(fwdPwr, fPwr);
		displayValue(refPwr, rPwr);
		displayValue(fwdVolts, vf);						// display with zero offset
		displayValue(refVolts, vr);
		displayMeter(nettPwrMeter, nPwr, pkPwr);		// display nPwr Meter
		displayMeter(swrMeter, swrV, 1);				// display if enabled

		// get and display frequency, needed here for swr / frequency manual sweep
		// this takes 30 mSecs, so only update if SWR meter and freq display is enabled
		// useful for SWR checking vs Frequency
		if (fr[swrMeter].isEnable && fr[freq].isEnable)
			displayValue(freq, getFreq());				// read frequency from radio

		// check for screen touch
		if (ts.tirqTouched())
		{
			if (isDim)
				resetDimmer();							// reset dimmer
			else
				touchChk(NUM_FRAMES);
		}
		if (nPwr >= PWR_THRESHOLD)
			resetDimmer();
	} while (nPwr >= PWR_THRESHOLD);					// do at least once and while power applied

	// power off - display exit power and reverse label
	if (fr[nettPwr].isEnable && !lab[nettPwr].stat)
	{
		fr[nettPwr].bg = BG_COLOUR;						// change back to background colour
		restoreFrame(nettPwr);
		displayValue(nettPwr, 0);						// ensure value displayed, zero power
	}
	lab[nettPwr].stat = true;							// reset update flag.  nettPwr b/g is background
}

/*----------------------------------- pwrCalc() ----------------------------------------------------------------
calculates pwr in Watts directly from ADC volts
applies constants from calibration procedure
*/
float pwrCalc(float v, char direction)						// 
{
	float pwr = 0.0;

	// calculate power. if Flg true, fwd power. if false ref power
	if (direction == 'f'  || direction == 'F')			
	{
		if (v < FWD_V_SPLIT_PWR)								// low power below split (non-linear)
			pwr = log(v) * FWD_LO_MULT_PWR + FWD_LO_ADD_PWR;
		else
			pwr = v * v * FWD_HI_MULT2_PWR + v * FWD_HI_MULT1_PWR + FWD_HI_ADD_PWR;		// high power
	}
	else
	{
		if (v < REF_V_SPLIT_PWR)								// low power below split (non-linear)
			pwr = log(v) * REF_LO_MULT_PWR + REF_LO_ADD_PWR;
		else
			pwr = v * v * REF_HI_MULT2_PWR + v * REF_HI_MULT1_PWR + REF_HI_ADD_PWR;		// high power
	}

	if (pwr < 0 || !isnormal(pwr) || isnan(pwr))		// check for division by zero and < 0
		pwr = 0.0;

	return pwr;										// return power (watts)
}

/*----------------------------------- pwrRmsCalc() ----------------------------------------------------
alternative power calculation
calculates Vrms from ADC volts and then power in watts
*/
float pwrRmsCalc(float vRms)
{
	float pwr = 0.0;

	// calculate Vrms
	if (vRms < V_SPLIT)
		vRms = log(vRms) * LO_MULT + LO_ADD;			// low power below split
	else
		vRms = vRms * HI_MULT + HI_ADD;					// high power

	if (vRms < 0 || !isnormal(vRms))					// check for division by zero and < 0
		vRms = 0.0;

	// calc power in watts
	pwr = sq(vRms) / 50;

	return pwr;
}