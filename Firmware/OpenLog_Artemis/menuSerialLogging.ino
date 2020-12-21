//TODO Add time stamp option after a certain timeout

void menuSerialLogging()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Serial Logging"));

    Serial.print(F("1) Log serial data: "));
    if (settings.logSerial == true) Serial.println(F("Enabled, analog logging on RX/A13 pin disabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("2) Output serial data to TX pin: "));
    if (settings.outputSerial == true) Serial.println(F("Enabled, analog logging on TX/A12 pin disabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("3) zmodem start delay: "));
    Serial.print(settings.zmodemStartDelay);
    Serial.println(F(" seconds"));

    if ((settings.logSerial == true) || (settings.outputSerial == true))
    {
      Serial.print(F("4) Set serial baud rate: "));
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

        beginSerialLogging(); //Start up port and log file (this will set online.serialLogging to true if successful)
      }
      else
      {
        if (online.serialLogging)
        {
          //Shut it all down
          updateDataFileAccess(&serialDataFile); // Update the file access time & date
          serialDataFile.close();

          online.serialLogging = false;
        }
        settings.logSerial = false;
      }
    }
    else if (incoming == '2')
    {
      if (settings.outputSerial == false)
      {
        settings.outputSerial = true;
        settings.logA12 = false; //Disable analog readings on TX pin

        beginSerialOutput(); //Start up port (this will set online.serialOutput to true if successful)
      }
      else
      {
        online.serialOutput = false;
        settings.outputSerial = false;
      }
    }
    else if (incoming == '3')
    {
      Serial.print(F("Enter zmodem start delay (5 to 60): "));
      int newDelay = getNumber(menuTimeout); //Timeout after x seconds
      if (newDelay < 5 || newDelay > 60)
      {
        Serial.println(F("Error: start delay out of range"));
      }
      else
      {
        settings.zmodemStartDelay = (uint8_t)newDelay;
      }
    }
    else if((settings.logSerial == true) || (settings.outputSerial == true))
    {
      if (incoming == '4')
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
