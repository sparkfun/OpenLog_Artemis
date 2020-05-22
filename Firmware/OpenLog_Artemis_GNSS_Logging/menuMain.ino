
//Display the options
//If user doesn't respond within a few seconds, return to main loop
void menuMain()
{
  //Disable debug messages when menu is open
  bool prevPrintMajorDebugMessages = settings.printMajorDebugMessages;
  bool prevPrintMinorDebugMessages = settings.printMinorDebugMessages;
  settings.printMajorDebugMessages = false;
  settings.printMinorDebugMessages = false;
  
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Main Menu"));

    Serial.println(F("1) Configure Logging"));

    Serial.println(F("2) Detect / Configure Attached Devices"));

    if (settings.logData && online.microSD && online.dataLogging)
    {
      Serial.println(F("f) Open New Log File"));
    }

    if (qwiicAvailable.uBlox && qwiicOnline.uBlox)
    {
      Serial.println(F("g) Reset GNSS"));
    }

    Serial.println(F("r) Reset all OLA settings to default"));

    Serial.println(F("d) Debug Menu"));

    Serial.println(F("x) Return to logging"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      menuLogRate();
    else if (incoming == '2')
      menuAttachedDevices();
    else if (incoming == 'f')
      openNewLogFile();
    else if (incoming == 'g')
    {
      Serial.println(F("\n\rResetting GNSS to factory defaults. Continue? Press 'y':"));
      byte gContinue = getByteChoice(menuTimeout);
      if (gContinue == 'y')
      {
        resetGNSS();
        Serial.println(F("GNSS reset. Please reset OpenLog Artemis and open a terminal at 115200bps..."));
        while (1);
      }
      else
        Serial.println(F("GNSS reset aborted"));
    }
    else if (incoming == 'd')
      menuDebug(&prevPrintMajorDebugMessages, &prevPrintMinorDebugMessages);
    else if (incoming == 'r')
    {
      Serial.println(F("\n\rResetting to factory defaults. Continue? Press 'y':"));
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        EEPROM.erase();
        if (sd.exists("OLA_GNSS_settings.cfg"))
          sd.remove("OLA_GNSS_settings.cfg");

        Serial.println(F("Settings erased. Please reset OpenLog Artemis and open a terminal at 115200bps..."));
        while (1);
      }
      else
        Serial.println(F("Reset aborted"));
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  Serial.println(F("\nReturning to logging..."));
  
  //Restore debug messages
  settings.printMajorDebugMessages = prevPrintMajorDebugMessages;
  settings.printMinorDebugMessages = prevPrintMinorDebugMessages;

  recordSettings(); //Once all menus have exited, record the new settings to EEPROM and config file

  beginSensors(); //Once all menus have exited, start any sensors that are available, logging, but not yet online/begun.

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars

  //If we are sleeping between readings then we cannot rely on millis() as it is powered down. Used RTC instead.
  measurementStartTime = rtcMillis();

}
