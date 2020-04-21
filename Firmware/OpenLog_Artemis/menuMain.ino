
//Display the options
//If user doesn't respond within a few seconds, return to main loop
void menuMain()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Main Menu");

    Serial.println("1) Configure Terminal Output");
    //10 to 1 Hz? Global logging. Overrides the IMU, analog, etc.?
    //User may want to log Analog really fast and IMU or humidity less fast.

    Serial.println("2) Configure Time Stamp");

    Serial.println("3) Configure IMU Logging");
    //Set record freq

    Serial.println("4) Configure Serial Logging");

    Serial.println("5) Configure Analog Pin Logging");

    Serial.println("6) Detect / Configure Attached Devices");

    Serial.println("r) Reset all settings to default");

    //Serial.println("d) Debug Menu");

    Serial.println("x) Return to logging");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      menuLogRate();
    else if (incoming == '2')
      menuTimeStamp();
    else if (incoming == '3')
      menuIMU();
    else if (incoming == '4')
      menuSerialLogging();
    else if (incoming == '5')
      menuAnalogLogging();
    else if (incoming == '6')
      menuAttachedDevices();
    else if (incoming == 'd')
      menuDebug();
    else if (incoming == 'r')
    {
      Serial.println("\n\rResetting to factory defaults. Continue? Press 'y':");
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        EEPROM.erase();
        if (sd.exists("OLA_settings.cfg"))
          sd.remove("OLA_settings.cfg");

        Serial.println("Settings erased. Please reset OpenLog Artemis and open a terminal at 115200bps...");
        while (1);
      }
      else
        Serial.println("Reset aborted");
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  recordSettings(); //Once all menus have exited, record the new settings to EEPROM and config file

  beginSensors(); //Once all menus have exited, start any sensors that are available, logging, but not yet online/begun.

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars

  //Reset measurements
  measurementCount = 0;
  totalCharactersPrinted = 0;
  //If we are sleeping between readings then we cannot rely on millis() as it is powered down. Used RTC instead.
  if (settings.usBetweenReadings >= maxUsBeforeSleep)
    measurementStartTime = rtcMillis();
  else
    measurementStartTime = millis();

  //Edge case: after 10Hz reading, user sets the log rate above 2s mark. We never go to sleep because 
  //takeReading is not true. And since we don't wake up, takeReading never gets set to true.
  //Se we force it here.
  takeReading = true; 
}
