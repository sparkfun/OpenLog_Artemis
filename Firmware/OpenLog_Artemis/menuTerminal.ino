void menuLogRate()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Terminal Output");

    Serial.print("1) Log to microSD: ");
    if (settings.logData == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("2) Log to Terminal: ");
    if (settings.enableTerminalOutput == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("3) Set Serial Baud Rate: ");
    Serial.print(settings.serialTerminalBaudRate);
    Serial.println(" bps");

    Serial.print("4) Set Log Rate in Hz: ");
    if (settings.logMaxRate == true) Serial.println("Max rate enabled");
    else
    {
      if (settings.usBetweenReadings < 1000000ULL) //Take more than one measurement per second
      {
        //Display Integer Hertz
        int logRate = (int)(1000000ULL / settings.usBetweenReadings);
        Serial.printf("%d\n", logRate);
      }
      else
      {
        //Display fractional Hertz
        uint32_t logRateSeconds = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
        Serial.printf("%.06lf\n", 1.0 / logRateSeconds);
      }
    }

    Serial.print("5) Set Log Rate in seconds between readings: ");
    if (settings.logMaxRate == true) Serial.println("Max rate enabled");
    else
    {
      if (settings.usBetweenReadings > 1000000ULL) //Take more than one measurement per second
      {
        uint32_t interval = (uint32_t)(settings.usBetweenReadings / 1000000ULL);
        Serial.printf("%d\n", interval);
      }
      else
      {
        float rate = (float)(settings.usBetweenReadings / 1000000.0);
        Serial.printf("%.06f\n", rate);
      }
    }

    Serial.print("6) Enable maximum logging: ");
    if (settings.logMaxRate == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("7) Output Actual Hertz: ");
    if (settings.logHertz == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("8) Output Column Titles: ");
    if (settings.showHelperText == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("9) Output Measurement Count: ");
    if (settings.printMeasurementCount == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.logData ^= 1;
    else if (incoming == '2')
      settings.enableTerminalOutput ^= 1;
    else if (incoming == '3')
    {
      Serial.print("Enter baud rate (1200 to 500000): ");
      int newBaud = getNumber(menuTimeout); //Timeout after x seconds
      if (newBaud < 1200 || newBaud > 500000)
      {
        Serial.println("Error: baud rate out of range");
      }
      else
      {
        settings.serialTerminalBaudRate = newBaud;
        recordSystemSettings(); //Normally recorded upon all menu exits
        recordDeviceSettingsToFile(); //Normally recorded upon all menu exits
        Serial.printf("Terminal now set at %dbps. Please reset device and open terminal at new baud rate. Freezing...\n", settings.serialTerminalBaudRate);
        while (1);
      }
    }
    else if (incoming == '4')
    {
      int maxOutputRate = settings.serialTerminalBaudRate / 10 / (totalCharactersPrinted / measurementCount);
      maxOutputRate = (maxOutputRate * 90) / 100; //Fudge reduction of 10%

      if (maxOutputRate < 10) maxOutputRate = 10; //TODO this is forced. Needed when multi seconds between readings.

      Serial.printf("How many readings per second would you like to log? (Current max is %d): ", maxOutputRate);
      int tempRPS = getNumber(menuTimeout); //Timeout after x seconds
      if (tempRPS < 1 || tempRPS > maxOutputRate)
        Serial.println("Error: Readings Per Second out of range");
      else
        settings.usBetweenReadings = 1000000ULL / tempRPS;
    }
    else if (incoming == '5')
    {
      //The Deep Sleep duration is set with am_hal_stimer_compare_delta_set, the duration of which is uint32_t
      //So the maximum we can sleep for is 2^32 / 32768 = 131072 seconds = 36.4 hours
      //Let's limit this to 36 hours = 129600 seconds
      Serial.println("How many seconds would you like to sleep between readings? (1 to 129,600):");
      uint64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
      if (tempSeconds < 1 || tempSeconds > 129600ULL)
        Serial.println("Error: Readings Per Second out of range");
      else
        //settings.recordPerSecond = tempRPS;
        settings.usBetweenReadings = 1000000ULL * tempSeconds;
    }
    else if (incoming == '6')
    {
      if (settings.logMaxRate == false)
      {
        Serial.println("\n\rEnabling max log rate will disable the IMU, \nterminal output, and serial logging. \nOnly analog values will be logged. Continue?");
        byte bContinue = getByteChoice(menuTimeout);
        if (bContinue == 'y')
        {
          settings.logMaxRate = true;
          settings.logSerial = false;
          settings.enableTerminalOutput = false;
          settings.enableIMU = false;

          //Close files on SD to be sure they are recorded fully
          serialDataFile.close();
          sensorDataFile.close();

          recordSystemSettings(); //Normally recorded upon all menu exits
          recordDeviceSettingsToFile(); //Normally recorded upon all menu exits

          Serial.println("OpenLog Artemis configured for max data rate. Please reset. Freezing...");
          while (1);
        }
      }
      else
        settings.logMaxRate = false;
    }
    else if (incoming == '7')
      settings.logHertz ^= 1;
    else if (incoming == '8')
      settings.showHelperText ^= 1;
    else if (incoming == '9')
      settings.printMeasurementCount ^= 1;
    else if (incoming == 'x')
      return;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
