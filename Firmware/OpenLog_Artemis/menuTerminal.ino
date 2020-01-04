void menuLogRate()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Terminal Output");

    Serial.print("1) Terminal Output: ");
    if (settings.enableTerminalOutput == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("2) Set serial baud rate: ");
    Serial.print(settings.serialTerminalBaudRate);
    Serial.println(" bps");

    Serial.print("3) Set log rate: ");
    if (settings.logMaxRate == true) Serial.println("Max rate enabled");
    else Serial.println(settings.recordPerSecond);

    Serial.print("4) Max log rate: ");
    if (settings.logMaxRate == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("5) Log actual Hertz: ");
    if (settings.logHertz == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("6) Print column titles: ");
    if (settings.showHelperText == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.println("x) Exit");

    byte incoming = getByteChoice(10); //Timeout after 10 seconds

    if (incoming == '1')
      settings.enableTerminalOutput ^= 1;
    else if (incoming == '2')
    {
      Serial.print("Enter baud rate (1200 to 500000): ");
      int newBaud = getNumber(10); //Timeout after 10 seconds
      if (newBaud < 1200 || newBaud > 500000)
      {
        Serial.println("Error: baud rate out of range");
      }
      else
      {
        settings.serialTerminalBaudRate = newBaud;
        recordSettings();
        Serial.printf("Terminal now set at %dbps. Please reset device and open terminal at new baud rate. Freezing...\n", settings.serialTerminalBaudRate);
        while(1);
      }
    }
    else if (incoming == '3')
    {
      int maxOutputRate = settings.serialTerminalBaudRate / 10 / (totalCharactersPrinted / measurementCount);
      maxOutputRate = (maxOutputRate * 90) / 100; //Fudge reduction of 10%
      
      Serial.printf("How many readings per second would you like to log? (Current max is %d): ", maxOutputRate);
      int tempRPS = getNumber(10); //Timeout after 10 seconds
      if (tempRPS < 1 || tempRPS > maxOutputRate)
        Serial.println("Error: Readings Per Second out of range");
      else
        settings.recordPerSecond = tempRPS;
    }
    else if (incoming == '4')
    {
      if (settings.logMaxRate == false)
      {
        Serial.println("\n\rEnabling max log rate will disable the IMU, \nterminal output, and serial logging. \nOnly analog values will be logged. Continue?");
        byte bContinue = getByteChoice(10);
        if (bContinue == 'y')
        {
          settings.logMaxRate = true;
          settings.logSerial = false;
          settings.enableTerminalOutput = false;
          settings.enableIMU = false;

          //Close files on SD to be sure they are recorded fully
          serialDataFile.close();
          sensorDataFile.close();

          recordSettings(); //Normally recorded upon all menu exits

          Serial.println("OpenLog Artemis configured for max data rate. Please reset. Freezing...");
          while (1);
        }
      }
      else
        settings.logMaxRate = false;
    }
    else if (incoming == '5')
      settings.logHertz ^= 1;
    else if (incoming == '6')
      settings.showHelperText ^= 1;
    else if (incoming == 'x')
      return;
    else if (incoming == 255)
      return;
    else
      printUnknown(incoming);
  }
}
