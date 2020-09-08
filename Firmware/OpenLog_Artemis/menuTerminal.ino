void menuLogRate()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Terminal Output"));

    Serial.print(F("1) Log to microSD: "));
    if (settings.logData == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("2) Log to Terminal: "));
    if (settings.enableTerminalOutput == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("3) Set Serial Baud Rate: "));
    Serial.print(settings.serialTerminalBaudRate);
    Serial.println(F(" bps"));

    if (settings.useGPIO11ForTrigger == false)
    {
      Serial.print(F("4) Set Log Rate in Hz: "));
      if (settings.logMaxRate == true) Serial.println(F("Max rate enabled"));
      else
      {
        if (settings.usBetweenReadings < 1000000ULL) //Take more than one measurement per second
        {
          //Display Integer Hertz
          int logRate = (int)(1000000ULL / settings.usBetweenReadings);
          Serial.printf("%d\r\n", logRate);
        }
        else
        {
          //Display fractional Hertz
          uint32_t logRateSeconds = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
          Serial.printf("%.06lf\r\n", 1.0 / logRateSeconds);
        }
      }
  
      Serial.print(F("5) Set Log Rate in seconds between readings: "));
      if (settings.logMaxRate == true) Serial.println(F("Max rate enabled"));
      else
      {
        if (settings.usBetweenReadings > 1000000ULL) //Take more than one measurement per second
        {
          uint32_t interval = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
          Serial.printf("%d\r\n", interval);
        }
        else
        {
          float rate = (float)(settings.usBetweenReadings / 1000000.0);
          Serial.printf("%.06f\r\n", rate);
        }
      }
  
      Serial.print(F("6) Enable maximum logging: "));
      if (settings.logMaxRate == true) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));
    }

    Serial.print(F("7) Output Actual Hertz: "));
    if (settings.logHertz == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("8) Output Column Titles: "));
    if (settings.showHelperText == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("9) Output Measurement Count: "));
    if (settings.printMeasurementCount == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("10) Open New Log Files After (s): "));
    Serial.printf("%d", settings.openNewLogFilesAfter);
    if (settings.openNewLogFilesAfter == 0) Serial.println(F(" (Never)"));
    else Serial.println();

    Serial.print(F("11) Frequent log file access timestamps: "));
    if (settings.frequentFileAccessTimestamps == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("12) Use pin 11 to trigger logging: "));
    if (settings.useGPIO11ForTrigger == true) Serial.println(F("Yes"));
    else Serial.println(F("No"));

    Serial.print(F("13) Logging is triggered when the signal on pin 11 is: "));
    if (settings.fallingEdgeTrigger == true) Serial.println(F("Falling"));
    else Serial.println(F("Rising"));

    Serial.println(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      settings.logData ^= 1;
    else if (incoming == 2)
      settings.enableTerminalOutput ^= 1;
    else if (incoming == 3)
    {
      Serial.print(F("Enter baud rate (1200 to 500000): "));
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 1200 || newBaud > 500000)
      {
        Serial.println(F("Error: baud rate out of range"));
      }
      else
      {
        settings.serialTerminalBaudRate = newBaud;
        recordSystemSettings(); //Normally recorded upon all menu exits
        recordDeviceSettingsToFile(); //Normally recorded upon all menu exits
        Serial.printf("Terminal now set at %dbps. Please reset device and open terminal at new baud rate. Freezing...\r\n", settings.serialTerminalBaudRate);
        while (1);
      }
    }
    else if (incoming == 4)
    {
      if (settings.useGPIO11ForTrigger == false)
      {
        int maxOutputRate = settings.serialTerminalBaudRate / 10 / (totalCharactersPrinted / measurementCount);
        maxOutputRate = (maxOutputRate * 90) / 100; //Fudge reduction of 10%
  
        if (maxOutputRate < 10) maxOutputRate = 10; //TODO this is forced. Needed when multi seconds between readings.
  
        Serial.printf("How many readings per second would you like to log? (Current max is %d): ", maxOutputRate);
        int tempRPS = getNumber(menuTimeout); //Timeout after x seconds
        if (tempRPS < 1 || tempRPS > maxOutputRate)
          Serial.println(F("Error: Readings Per Second out of range"));
        else
          settings.usBetweenReadings = 1000000ULL / ((uint64_t)tempRPS);
      }
    }
    else if (incoming == 5)
    {
      if (settings.useGPIO11ForTrigger == false)
      {
        //The Deep Sleep duration is set with am_hal_stimer_compare_delta_set, the duration of which is uint32_t
        //So the maximum we can sleep for is 2^32 / 32768 = 131072 seconds = 36.4 hours
        //Let's limit this to 36 hours = 129600 seconds
        Serial.println(F("How many seconds would you like to sleep between readings? (1 to 129,600):"));
        int64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
        if (tempSeconds < 1 || tempSeconds > 129600)
          Serial.println(F("Error: Readings Per Second out of range"));
        else
          settings.usBetweenReadings = 1000000ULL * ((uint64_t)tempSeconds);
      }
    }
    else if (incoming == 6)
    {
      if (settings.useGPIO11ForTrigger == false)
      {
        if (settings.logMaxRate == false)
        {
          Serial.println(F("\r\nEnabling max log rate will disable the IMU, \r\nterminal output, and serial logging. \r\nOnly analog values will be logged. Continue?"));
          byte bContinue = getByteChoice(menuTimeout);
          if (bContinue == 'y')
          {
            settings.logMaxRate = true;
            settings.logSerial = false;
            settings.enableTerminalOutput = false;
            settings.enableIMU = false;
  
            //Close files on SD to be sure they are recorded fully
            updateDataFileAccess(&serialDataFile); // Update the file access time & date
            serialDataFile.close();
            updateDataFileAccess(&sensorDataFile); // Update the file access time & date          
            sensorDataFile.close();
  
            recordSystemSettings(); //Normally recorded upon all menu exits
            recordDeviceSettingsToFile(); //Normally recorded upon all menu exits
  
            Serial.println(F("OpenLog Artemis configured for max data rate. Please reset. Freezing..."));
            while (1);
          }
        }
        else
        {
          settings.logMaxRate = false;
          //settings.usBetweenReadings = 100000ULL; //Default to 100,000us = 100ms = 10 readings per second.
        }
      }
    }
    else if (incoming == 7)
      settings.logHertz ^= 1;
    else if (incoming == 8)
      settings.showHelperText ^= 1;
    else if (incoming == 9)
      settings.printMeasurementCount ^= 1;
    else if (incoming == 10)
    {
      Serial.println(F("Open new log files after this many seconds (0 or 10 to 129,600) (0 = Never):"));
      int64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
      if ((tempSeconds < 0) || ((tempSeconds > 0) && (tempSeconds < 10)) || (tempSeconds > 129600ULL))
        Serial.println(F("Error: Invalid interval"));
      else
        settings.openNewLogFilesAfter = tempSeconds;
    }
    else if (incoming == 11)
      settings.frequentFileAccessTimestamps ^= 1;
    else if (incoming == 12)
    {
      if (settings.useGPIO11ForTrigger == true)
      {
        // Disable triggering
        settings.useGPIO11ForTrigger = false;
        detachInterrupt(digitalPinToInterrupt(PIN_TRIGGER)); // Disable the interrupt
        pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up
        triggerEdgeSeen = false; // Make sure the flag is clear
      }
      else
      {
        // Enable triggering
        settings.useGPIO11ForTrigger = true;
        pinMode(PIN_TRIGGER, INPUT_PULLUP);
        delay(1); // Let the pin stabilize
        if (settings.fallingEdgeTrigger == true)
          attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER), triggerPinISR, FALLING); // Enable the interrupt
        else
          attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER), triggerPinISR, RISING); // Enable the interrupt
        triggerEdgeSeen = false; // Make sure the flag is clear
        settings.logA11 = false; // Disable analog logging on pin 11
        settings.logMaxRate = false; // Disable max rate logging
      }
    }
    else if (incoming == 13)
    {
      if (settings.useGPIO11ForTrigger == true) // If interrupts are enabled, we need to disable and then re-enable
      {
        detachInterrupt(digitalPinToInterrupt(PIN_TRIGGER)); // Disable the interrupt
        settings.fallingEdgeTrigger ^= 1; // Invert the flag
        if (settings.fallingEdgeTrigger == true)
          attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER), triggerPinISR, FALLING); // Enable the interrupt
        else
          attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER), triggerPinISR, RISING); // Enable the interrupt
        triggerEdgeSeen = false; // Make sure the flag is clear
      }
      else
        settings.fallingEdgeTrigger ^= 1; // Interrupt is not currently enabled so simply invert the flag
    }
    else if (incoming == STATUS_PRESSED_X)
      return;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
