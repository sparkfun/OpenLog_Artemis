//TODO Add time stamp option after a certain timeout

void menuSerialLogging()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Serial Logging");

    Serial.print("1) Log serial data: ");
    if (settings.logSerial == true) Serial.println("Enabled, analog on pin 13 disabled");
    else Serial.println("Disabled");

    if (settings.logSerial == true)
    {
      Serial.print("2) Set serial baud rate: ");
      Serial.print(settings.serialBaudRate);
      Serial.println(" bps");
    }

    Serial.println("x) Exit");

    byte incoming = getByteChoice(10); //Timeout after 10 seconds

    if (incoming == '1')
    {
      if (settings.logSerial == false)
      {
        settings.logSerial = true;
        settings.logA13 = false; //Disable analog readings on RX pin

        beginSerialLogging(); //Start up port and log file
      }
      else
      {
        if (online.serialLogging)
        {
          //Shut it all down
          serialDataFile.close();

          online.serialLogging = false;
        }
        settings.logSerial = false;
      }
    }
    else if(settings.logSerial == true)
    {
      if (incoming == '2')
      {
        Serial.print("Enter baud rate (1200 to 115200): ");
        int newBaud = getNumber(10); //Timeout after 10 seconds
        if (newBaud < 1200 || newBaud > 115200)
        {
          Serial.println("Error: baud rate out of range");
        }
        else
        {
          settings.serialBaudRate = newBaud;
          SerialLog.begin(settings.serialBaudRate);
        }
      }
      else if (incoming == 'x')
        return;
      else if (incoming == 255)
        return;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      return;
    else if (incoming == 255)
      return;
    else
      printUnknown(incoming);
  }
}
