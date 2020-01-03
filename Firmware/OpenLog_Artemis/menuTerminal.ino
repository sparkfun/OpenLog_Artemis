void menuLogRate()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Terminal Output");

    Serial.print("1) Terminal Output: ");
    if (settings.enableTerminalOutput == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("2) Set log rate: ");
    if (settings.logMaxRate == true) Serial.println("Max rate enabled");
    else Serial.println(settings.recordPerSecond);

    Serial.print("3) Max log rate: ");
    if (settings.logMaxRate == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("4) Log actual Hertz: ");
    if (settings.logHertz == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("5) Print column titles: ");
    if (settings.showHelperText == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.println("x) Exit");

    byte incoming = getByteChoice(10); //Timeout after 10 seconds

    if (incoming == '1')
      settings.enableTerminalOutput ^= 1;
    else if (incoming == '2')
    {
      Serial.print("How many readings per second would you like to log? : ");
      int tempRPS = getNumber(10); //Timeout after 10 seconds
      if (tempRPS < 1 || tempRPS > 1000)
        Serial.println("Error: RPS out of range");
      else
        settings.recordPerSecond = tempRPS;
    }
    else if (incoming == '3')
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
    else if (incoming == '4')
      settings.logHertz ^= 1;
    else if (incoming == '5')
      settings.showHelperText ^= 1;
    else if (incoming == 'x')
      return;
    else if (incoming == 255)
      return;
    else
      printUnknown(incoming);
  }
}
