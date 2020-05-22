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

    Serial.print(F("4) Set Log Rate in Hz: "));
    if (settings.usBetweenReadings < 1000000UL) //Take more than one measurement per second
    {
      //Display Integer Hertz
      int logRate = 1000000UL / settings.usBetweenReadings;
      Serial.printf("%d\n", logRate);
    }
    else
    {
      //Display fractional Hertz
      uint32_t logRateSeconds = settings.usBetweenReadings / 1000000UL;
      Serial.printf("%.06lf\n", 1.0 / logRateSeconds);
    }

    Serial.print(F("5) Set Log Rate in seconds between readings: "));
    if (settings.usBetweenReadings > 1000000UL) //Take more than one measurement per second
    {
      Serial.printf("%llu\n", settings.usBetweenReadings / 1000000UL);
    }
    else
    {
      Serial.printf("%.06lf\n", settings.usBetweenReadings / 1000000.0);
    }

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.logData ^= 1;
    else if (incoming == '2')
      settings.enableTerminalOutput ^= 1;
    else if (incoming == '3')
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
        recordSettings();
        Serial.printf("Terminal now set at %dbps. Please reset device and open terminal at new baud rate. Freezing...\n", settings.serialTerminalBaudRate);
        while (1);
      }
    }
    else if (incoming == '4')
    {
      float rateLimit = 1.0 / (((float)settings.sensor_uBlox.minMeasInterval) / 1000.0);
      int maxOutputRate = (int)rateLimit;

      Serial.printf("How many readings per second would you like to log? (Current max is %d): ", maxOutputRate);
      int tempRPS = getNumber(menuTimeout); //Timeout after x seconds
      if (tempRPS < 1 || tempRPS > maxOutputRate)
        Serial.println(F("Error: Readings Per Second out of range"));
      else
        settings.usBetweenReadings = 1000000UL / tempRPS;
        
      qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
    }
    else if (incoming == '5')
    {
      Serial.println(F("How many seconds would you like to sleep between readings? (1 to 6,000,000,000):"));
      uint64_t tempSeconds = getNumber(menuTimeout); //Timeout after x seconds
      if (tempSeconds < 1 || tempSeconds > 6000000000ULL)
        Serial.println(F("Error: Readings Per Second out of range"));
      else
        //settings.recordPerSecond = tempRPS;
        settings.usBetweenReadings = 1000000UL * tempSeconds;

      qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
    }
    else if (incoming == 'x')
      return;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
