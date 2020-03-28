/*---------------------------------------------------------
  POWERMETER III + ICOM 7300 CONTROLLER
  © Copyright 2018-2020  Roger Mawhinney, GI8GZM.
  No publication with acknowledgement to author
*/

#define	AVERAGING 16
#define	RESOLUTION 12
#define	CONV_SPEED MED_SPEED
#define	SAMPLE_SPEED MED_SPEED

// these setting reduce zero offset
//#define		AVERAGING 4						 
//#define		RESOLUTION 16
//#define		CONV_SPEED HIGH_SPEED
//#define		SAMPLE_SPEED VERY_HIGH_SPEED

/*---------------------------------------- initADC() ----------------------------------
initialises Analog-Digital convertor
sets resolution, conversion speeds
interrupt timer interval*/
void initADC(void)
{
	// set up ADC convertors - ADC 0
	adc->adc0->setAveraging(AVERAGING); 						// set number of averages
	adc->adc0->setResolution(RESOLUTION); 						// set bits of resolution
	adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::CONV_SPEED);	// change the conversion speed
	adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::SAMPLE_SPEED);		// change the sampling speed

	// ADC 1
	adc->adc1->setAveraging(AVERAGING); 						// set number of averages
	adc->adc1->setResolution(RESOLUTION); 						// set bits of resolution
	adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::CONV_SPEED);	// change the conversion speed
	adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::SAMPLE_SPEED);		// change the sampling speed

	sampleTimer.begin(getADC, SAMPLE_INTERVAL);					// getADC to run every 500 micro seconds


}


/* -------------------------------- get ADC() ----------------------------------------------
get raw results from ADC and enter into cyclic buffer
calculates average and peak values for ACD results

SAMPLE_INTERVAL = 500usecs (= 2Khz).  Max samples MXBUF = 1000
------------------------------------------------------------------------------------------*/
void getADC()											// i = current buffer position, 0 to samplesAvg
{
	//digitalWriteFast(TOGGLE_PIN, HIGH);
	unsigned int ar0, ar1;								// ADC results
	static int prevSamplesAvg;

	if (prevSamplesAvg != samplesAvg)					// has samples number changed?
	{
		for (int i = 0; i < samplesAvg; i++)			// initialise buffers
		{
			buf0[i] = 0;
			buf1[i] = 0;
		}
		buf0Tot = 0;									// rest buffer totals
		buf1Tot = 0;
		sample = 0;										// start new buffer
	}

	result = adc->analogSyncRead(A1, A2);				// synced read ADC_0, ADC_1
	ar0 = (uint16_t)result.result_adc0;					// add to start of ff register [0]
	ar1 = (uint16_t)result.result_adc1;

	// cyclic buffer averaging
	buf0Tot = buf0Tot - buf0[sample];					// remove oldest from total
	buf0[sample] = ar0;									// replace with new sample
	buf0Tot = buf0Tot + buf0[sample];					// add newest to total

	buf1Tot = buf1Tot - buf1[sample];					// remove oldest from total
	buf1[sample] = ar1;									// record new sample
	buf1Tot = buf1Tot + buf1[sample];					// add newest to total

	// cyclic buffer peak value
	buf0Pk = 0;											// inialise fwd buffer only
	for (int j = 0; j < samplesAvg; j++)
		if (buf0Pk <= buf0[j])							// get peak forward sample
		{
			buf0Pk = buf0[j];
			buf1Pk = buf1[j];							// save ref corresponding peak
		}

	sample++;
	if (sample >= samplesAvg)							// if max buff, back to start
		sample = 0;

	prevSamplesAvg = samplesAvg;						// save number of samples

	// digitalWriteFast(TOGGLE_PIN, LOW);
}
