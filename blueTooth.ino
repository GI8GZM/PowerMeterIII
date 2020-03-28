

void blueTooth()
{
	int state = 0;

	//digitalWrite(LED_BUILTIN, LOW);

	while (btSerial.available() > 0) 			// Checks whether data is comming from the serial port
	{
		state = btSerial.read();				// Reads the data from the serial port

	//Serial.print(state);
	//Serial.print("...");
	//Serial.println(i++);

		if (state == '0')
		{
			tft.fillCircle(160, 100, 15, BG_COLOUR);
			//digitalWrite(LED_BUILTIN, LOW);		// Turn LED OFF
			btSerial.println("LED: OFF");		// Send back, to the phone, the String "LED: ON"
			Serial.println("LED: OFF");			// Send back, to the phone, the String "LED: ON"
			state = 0;
		}
		else if (state == '1')
		{
			tft.fillCircle(160, 100, 15, FG_COLOUR);
			//digitalWrite(LED_BUILTIN, HIGH);
			btSerial.println("LED: ON");
			Serial.println("LED: ON");

			state = 0;
		}
	}
	//float f = getFreq();
	//btSerial.println(f, 5);
}


