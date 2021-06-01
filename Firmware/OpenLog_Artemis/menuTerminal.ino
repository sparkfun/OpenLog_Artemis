void menuLogRate()
{
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Terminal Output"));

    SerialPrint(F("1) Log to microSD: "));
    if (settings.logData == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("2) Log to Terminal: "));
    if (settings.enableTerminalOutput == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("3) Set Serial Terminal Baud Rate: "));
    Serial.print(settings.serialTerminalBaudRate);
    if (settings.useTxRxPinsForTerminal == true)
      SerialLog.print(settings.serialTerminalBaudRate);
    SerialPrintln(F(" bps"));

    if (settings.useGPIO11ForTrigger == false)
    {
      SerialPrint(F("4) Set Log Rate in Hz: "));
      if (settings.logMaxRate == true) SerialPrintln(F("Max rate enabled"));
      else
      {
        if (settings.usBetweenReadings < 1000000ULL) //Take more than one measurement per second
        {
          //Display Integer Hertz
          int logRate = (int)(1000000ULL / settings.usBetweenReadings);
          SerialPrintf2("%d\r\n", logRate);
        }
        else
        {
          //Display fractional Hertz
          uint32_t logRateSeconds = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
          SerialPrintf2("%.06lf\r\n", 1.0 / logRateSeconds);
        }
      }
  
      SerialPrint(F("5) Set Log Rate in seconds between readings: "));
      if (settings.logMaxRate == true) SerialPrintln(F("Max rate enabled"));
      else
      {
        if (settings.usBetweenReadings > 1000000ULL) //Take more than one measurement per second
        {
          uint32_t interval = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
          SerialPrintf2("%d\r\n", interval);
        }
        else
        {
          float rate = (float)(settings.usBetweenReadings / 1000000.0);
          SerialPrintf2("%.06f\r\n", rate);
        }
      }
  
      SerialPrint(F("6) Enable maximum logging: "));
      if (settings.logMaxRate == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }

    SerialPrint(F("7) Output Actual Hertz: "));
    if (settings.logHertz == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("8) Output Column Titles: "));
    if (settings.showHelperText == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("9) Output Measurement Count: "));
    if (settings.printMeasurementCount == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("10) Open New Log Files After (s): "));
    SerialPrintf2("%d", settings.openNewLogFilesAfter);
    if (settings.openNewLogFilesAfter == 0) SerialPrintln(F(" (Never)"));
    else SerialPrintln(F(""));

    SerialPrint(F("11) Frequent log file access timestamps: "));
    if (settings.frequentFileAccessTimestamps == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("12) Use pin 11 to trigger logging: "));
    if (settings.useGPIO11ForTrigger == true) SerialPrintln(F("Yes"));
    else SerialPrintln(F("No"));

    SerialPrint(F("13) Logging is triggered when the signal on pin 11 is: "));
    if (settings.fallingEdgeTrigger == true) SerialPrintln(F("Falling"));
    else SerialPrintln(F("Rising"));

    SerialPrint(F("14) Use TX and RX pins for Terminal: "));
    if (settings.useTxRxPinsForTerminal == true)
    {
      SerialPrintln(F("Enabled"));
      SerialPrintln(F("                                     Analog logging on TX/A12 and RX/A13 is permanently disabled"));
      SerialPrintln(F("                                     Serial logging on RX/A13 is permanently disabled"));
    }
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("15) Use Pin 11 to control fast/slow logging: "));
    if (settings.useGPIO11ForFastSlowLogging == true) SerialPrintln(F("Yes"));
    else SerialPrintln(F("No"));

    if (settings.useGPIO11ForFastSlowLogging == true)
    {
      SerialPrint(F("16) Log slowly when Pin 11 is: "));
      if (settings.slowLoggingWhenPin11Is == true) SerialPrintln(F("High"));
      else SerialPrintln(F("Low"));
    }

    SerialPrint(F("17) Use RTC to control fast/slow logging: "));
    if (settings.useRTCForFastSlowLogging == true) SerialPrintln(F("Yes"));
    else SerialPrintln(F("No"));

    if ((settings.useGPIO11ForFastSlowLogging == true) || (settings.useRTCForFastSlowLogging == true))
    {
      SerialPrint(F("18) Slow logging interval (seconds): "));
      SerialPrintf2("%d\r\n", settings.slowLoggingIntervalSeconds);
    }

    if (settings.useRTCForFastSlowLogging == true)
    {
      SerialPrint(F("19) Slow logging starts at: "));
      int slowHour = settings.slowLoggingStartMOD / 60;
      int slowMin = settings.slowLoggingStartMOD % 60;
      SerialPrintf3("%02d:%02d\r\n", slowHour, slowMin);

      SerialPrint(F("20) Slow logging ends at: "));
      slowHour = settings.slowLoggingStopMOD / 60;
      slowMin = settings.slowLoggingStopMOD % 60;
      SerialPrintf3("%02d:%02d\r\n", slowHour, slowMin);
    }

    if ((settings.useGPIO11ForTrigger == false) && (settings.usBetweenReadings >= maxUsBeforeSleep))
    {
      SerialPrint(F("21) Minimum awake time between sleeps: "));
      SerialPrintf2("%dms\r\n", settings.minimumAwakeTimeMillis);
    }

    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      settings.logData ^= 1;
    else if (incoming == 2)
      settings.enableTerminalOutput ^= 1;
    else if (incoming == 3)
    {
      SerialPrint(F("Enter baud rate (1200 to 500000): "));
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 1200 || newBaud > 500000)
      {
        SerialPrintln(F("Error: baud rate out of range"));
      }
      else
      {
        settings.serialTerminalBaudRate = newBaud;
        recordSystemSettings(); //Normally recorded upon all menu exits
        recordDeviceSettingsToFile(); //Normally recorded upon all menu exits
        SerialPrintf2("Terminal now set at %dbps. Please reset device and open terminal at new baud rate. Freezing...\r\n", settings.serialTerminalBaudRate);
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
  
        SerialPrintf2("How many readings per second would you like to log? (Current max is %d): ", maxOutputRate);
        int tempRPS = getNumber(menuTimeout); //Timeout after x seconds
        if (tempRPS < 1 || tempRPS > maxOutputRate)
          SerialPrintln(F("Error: Readings Per Second out of range"));
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
        SerialPrintln(F("How many seconds would you like to wait between readings? (1 to 129,600):"));
        int64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
        if (tempSeconds < 1 || tempSeconds > 129600)
          SerialPrintln(F("Error: logging interval out of range"));
        else
        {
          settings.usBetweenReadings = 1000000ULL * ((uint64_t)tempSeconds);

          if (settings.usBetweenReadings >= maxUsBeforeSleep) // Check if minimumAwakeTimeMillis needs to be reduced
          {
            // Limit minimumAwakeTimeMillis to usBetweenReadings minus one second
            if (settings.minimumAwakeTimeMillis > ((settings.usBetweenReadings / 1000ULL) - 1000ULL))
              settings.minimumAwakeTimeMillis = (unsigned long)((settings.usBetweenReadings / 1000ULL) - 1000ULL);
          }
        }
      }
    }
    else if (incoming == 6)
    {
      if (settings.useGPIO11ForTrigger == false)
      {
        if (settings.logMaxRate == false)
        {
          SerialPrintln(F("\r\nEnabling max log rate will disable the IMU, \r\nterminal output, and serial logging. \r\nOnly analog values will be logged. Continue?"));
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
  
            SerialPrintln(F("OpenLog Artemis configured for max data rate. Please reset. Freezing..."));
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
      SerialPrintln(F("Open new log files after this many seconds (0 or 10 to 129,600) (0 = Never):"));
      int64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
      if ((tempSeconds < 0) || ((tempSeconds > 0) && (tempSeconds < 10)) || (tempSeconds > 129600ULL))
        SerialPrintln(F("Error: Invalid interval"));
      else
        settings.openNewLogFilesAfter = tempSeconds;
    }
    else if (incoming == 11)
      settings.frequentFileAccessTimestamps ^= 1;
    else if (incoming == 12)
    {
      if (settings.identifyBioSensorHubs == false)
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
          settings.useGPIO11ForFastSlowLogging = false;
          settings.useRTCForFastSlowLogging = false;
        }
      }
      else
      {
        SerialPrintln(F(""));
        SerialPrintln(F("Triggering on pin 11 is not possible. \"Detect Bio Sensor Pulse Oximeter\" is enabled."));
        SerialPrintln(F(""));
      }      
    }
    else if (incoming == 13)
    {
      if (settings.identifyBioSensorHubs == false)
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
      else
      {
        SerialPrintln(F(""));
        SerialPrintln(F("Triggering on pin 11 is not possible. \"Detect Bio Sensor Pulse Oximeter\" is enabled."));
        SerialPrintln(F(""));
      }      
    }
    else if (incoming == 14)
    {
      if (settings.useTxRxPinsForTerminal == false)
      {
        SerialPrintln(F(""));
        SerialPrintln(F("\"Use TX and RX pins for terminal\" can only be disabled by \"Reset all settings to default\"."));
        SerialPrintln(F("Analog logging on TX/A12 and RX/A13 will be disabled."));
        SerialPrintln(F("Serial logging will be disabled."));
        SerialPrintln(F("Are you sure? Press 'y' to confirm: "));
        byte bContinue = getByteChoice(menuTimeout);
        if (bContinue == 'y')
        {
          settings.useTxRxPinsForTerminal = true;
          if (online.serialLogging == true)
          {
            serialDataFile.sync();
            updateDataFileAccess(&serialDataFile); // Update the file access time & date
            serialDataFile.close();
          }
          online.serialLogging = false;
          settings.logSerial = false;
          settings.outputSerial = false;
          online.serialOutput = false;
          settings.logA12 = false;
          settings.logA13 = false;
          SerialLog.begin(settings.serialTerminalBaudRate); // (Re)Start the serial port
        }
        else
          SerialPrintln(F("\"Use TX and RX pins for terminal\"  aborted"));
      }
      else
      {
        SerialPrintln(F(""));
        SerialPrintln(F("\"Use TX and RX pins for terminal\" can not be disabled."));
        SerialPrintln(F("You need to use \"Reset all settings to default\" from the main menu."));
        SerialPrintln(F(""));
      }
    }
    else if (incoming == 15)
    {
      if (settings.identifyBioSensorHubs == false)
      {
        if (settings.useGPIO11ForFastSlowLogging == false) // If the user is trying to enable Pin 11 fast / slow logging
        {
          settings.useGPIO11ForFastSlowLogging = true;
          settings.useRTCForFastSlowLogging = false;
          settings.logA11 = false; // Disable analog logging on pin 11
          pinMode(PIN_TRIGGER, INPUT_PULLUP);
          delay(1); // Let the pin stabilize
          // Disable triggering
          if (settings.useGPIO11ForTrigger == true)
          {
            detachInterrupt(digitalPinToInterrupt(PIN_TRIGGER)); // Disable the interrupt
            triggerEdgeSeen = false; // Make sure the flag is clear
          }
          settings.useGPIO11ForTrigger = false;
        }
        else // If the user is trying to disable Pin 11 fast / slow logging
        {
          settings.useGPIO11ForFastSlowLogging = false;        
          pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up
        }
      }
      else
      {
        SerialPrintln(F(""));
        SerialPrintln(F("Fast / slow logging via pin 11 is not possible. \"Detect Bio Sensor Pulse Oximeter\" is enabled."));
        SerialPrintln(F(""));
      }              
    }
    else if (incoming == 16)
    {
      if (settings.identifyBioSensorHubs == false)
      {
        if (settings.useGPIO11ForFastSlowLogging == true)
        {
          settings.slowLoggingWhenPin11Is ^= 1;
        }
      }
      else // If the user is trying to disable Pin 11 fast / slow logging
      {
        settings.useGPIO11ForFastSlowLogging = false;        
        pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up
      }
    }
    else if (incoming == 17)
    {
      if (settings.useRTCForFastSlowLogging == false) // If the user is trying to enable RTC fast / slow logging
      {
        settings.useRTCForFastSlowLogging = true;
        if (settings.useGPIO11ForFastSlowLogging == true)
        {
          pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up          
        }
        settings.useGPIO11ForFastSlowLogging = false;
        settings.logA11 = false; // Disable analog logging on pin 11
        // Disable triggering
        if (settings.useGPIO11ForTrigger == true)
        {
          detachInterrupt(digitalPinToInterrupt(PIN_TRIGGER)); // Disable the interrupt
          pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up
          triggerEdgeSeen = false; // Make sure the flag is clear
        }
        settings.useGPIO11ForTrigger = false;
      }
      else // If the user is trying to disable RTC fast / slow logging
      {
        settings.useRTCForFastSlowLogging = false;        
      }
    }
    else if (incoming == 18)
    {
      if ((settings.useGPIO11ForFastSlowLogging == true) || (settings.useRTCForFastSlowLogging == true))
      {
        //The Deep Sleep duration is set with am_hal_stimer_compare_delta_set, the duration of which is uint32_t
        //So the maximum we can sleep for is 2^32 / 32768 = 131072 seconds = 36.4 hours
        //Let's limit this to 36 hours = 129600 seconds
        SerialPrintln(F("How many seconds would you like to sleep between readings? (5 to 129,600):"));
        int64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
        if (tempSeconds < 5 || tempSeconds > 129600)
          SerialPrintln(F("Error: sleep interval out of range"));
        else
          settings.slowLoggingIntervalSeconds = (int)tempSeconds;
      }      
    }
    else if (incoming == 19)
    {
      if (settings.useRTCForFastSlowLogging == true)
      {
        SerialPrintln(F("Enter the Hour slow logging should start (0 to 23):"));
        int64_t tempMOD = getNumber(menuTimeout); //Timeout after x seconds
        if (tempMOD < 0 || tempMOD > 23)
          SerialPrintln(F("Error: time out of range"));
        else
        {
          settings.slowLoggingStartMOD = (int)tempMOD * 60; // Convert to minutes
          SerialPrintln(F("\r\nEnter the Minute slow logging should start (0 to 59):"));
          tempMOD = getNumber(menuTimeout); //Timeout after x seconds
          if (tempMOD < 0 || tempMOD > 59)
            SerialPrintln(F("Error: time out of range"));
          else
            settings.slowLoggingStartMOD += (int)tempMOD;
        }
      }      
    }
    else if (incoming == 20)
    {
      if (settings.useRTCForFastSlowLogging == true)
      {
        SerialPrintln(F("Enter the Hour slow logging should end (0 to 23):"));
        int64_t tempMOD = getNumber(menuTimeout); //Timeout after x seconds
        if (tempMOD < 0 || tempMOD > 23)
          SerialPrintln(F("Error: time out of range"));
        else
        {
          settings.slowLoggingStopMOD = (int)tempMOD * 60; // Convert to minutes
          SerialPrintln(F("\r\nEnter the Minute slow logging should end (0 to 59):"));
          tempMOD = getNumber(menuTimeout); //Timeout after x seconds
          if (tempMOD < 0 || tempMOD > 59)
            SerialPrintln(F("Error: time out of range"));
          else
            settings.slowLoggingStopMOD += (int)tempMOD;
        }
      }      
    }
    else if (incoming == 21)
    {
      if (settings.useGPIO11ForTrigger == false)
      {
        if (settings.usBetweenReadings >= maxUsBeforeSleep)
        {
          // Limit minimumAwakeTimeMillis to usBetweenReadings minus one second
          unsigned long maxAwakeMillis = (unsigned long)((settings.usBetweenReadings / 1000ULL) - 1000ULL);
          SerialPrintf2("Enter minimum awake time (ms: 0 to %d): : ", maxAwakeMillis);
          int newAwake = getNumber(menuTimeout); //Timeout after x seconds
          if (newAwake < 0 || newAwake > maxAwakeMillis)
          {
            SerialPrintln(F("Error: awake time out of range"));
          }
          else
          {
            settings.minimumAwakeTimeMillis = newAwake;
          }
        }
      }
    }
    else if (incoming == STATUS_PRESSED_X)
      return;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
