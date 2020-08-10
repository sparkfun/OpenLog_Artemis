//TODO Add time stamp option after a certain timeout

void menuSerialLogging()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Serial Logging"));

    Serial.print(F("1) Log serial data: "));
    if (settings.logSerial == true) Serial.println(F("Enabled, analog on pin 13 disabled"));
    else Serial.println(F("Disabled"));

    if (settings.logSerial == true)
    {
      Serial.print(F("2) Set serial baud rate: "));
      Serial.print(settings.serialLogBaudRate);
      Serial.println(F(" bps"));
    }

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

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
        Serial.print(F("Enter baud rate (1200 to 500000): "));
        int newBaud = getNumber(menuTimeout); //Timeout after x seconds
        if (newBaud < 1200 || newBaud > 500000)
        {
          Serial.println(F("Error: baud rate out of range"));
        }
        else
        {
          settings.serialLogBaudRate = newBaud;
          SerialLog.begin(settings.serialLogBaudRate);
        }
      }
      else if (incoming == 'x')
        return;
      else if (incoming == STATUS_GETBYTE_TIMEOUT)
        return;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      return;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
