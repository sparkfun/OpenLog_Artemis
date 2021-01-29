//TODO Add time stamp option after a certain timeout

void menuSerialLogging()
{
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Serial Logging"));

    SerialPrint(F("1) Log serial data: "));
    if (settings.logSerial == true) SerialPrintln(F("Enabled, analog logging on RX/A13 pin disabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("2) Output serial data to TX pin: "));
    if (settings.outputSerial == true) SerialPrintln(F("Enabled, analog logging on TX/A12 pin disabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("3) zmodem start delay: "));
    Serial.print(settings.zmodemStartDelay);
    if (settings.useTxRxPinsForTerminal == true)
      SerialLog.print(settings.zmodemStartDelay);
    SerialPrintln(F(" seconds"));

    if ((settings.logSerial == true) || (settings.outputSerial == true))
    {
      SerialPrint(F("4) Set serial baud rate: "));
      Serial.print(settings.serialLogBaudRate);
      if (settings.useTxRxPinsForTerminal == true)
        SerialLog.print(settings.zmodemStartDelay);
      SerialPrintln(F(" bps"));
    }

    if (settings.logSerial == true) // Suggested by @DennisMelamed in Issue #63
    {
      SerialPrint(F("5) Add RTC timestamp when token is received: "));
      if (settings.timestampSerial == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
  
      SerialPrint(F("6) Timestamp token: "));
      Serial.print(settings.timeStampToken);
      if (settings.useTxRxPinsForTerminal == true)
        SerialLog.print(settings.timeStampToken);
      SerialPrint(F(" (Decimal)"));
      switch (settings.timeStampToken)
      {
        case 0x00:
          SerialPrintln(F(" = NULL"));
          break;
        case 0x03:
          SerialPrintln(F(" = End of Text"));
          break;
        case 0x0A:
          SerialPrintln(F(" = Line Feed"));
          break;
        case 0x0D:
          SerialPrintln(F(" = Carriage Return"));
          break;
        case 0x1B:
          SerialPrintln(F(" = Escape"));
          break;
        default:
          SerialPrintln(F(""));
          break;
      }
    }

    SerialPrintln(F("x) Exit"));

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
      SerialPrint(F("Enter zmodem start delay (5 to 60): "));
      int newDelay = getNumber(menuTimeout); //Timeout after x seconds
      if (newDelay < 5 || newDelay > 60)
      {
        SerialPrintln(F("Error: start delay out of range"));
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
        SerialPrint(F("Enter baud rate (1200 to 500000): "));
        int newBaud = getNumber(menuTimeout); //Timeout after x seconds
        if (newBaud < 1200 || newBaud > 500000)
        {
          SerialPrintln(F("Error: baud rate out of range"));
        }
        else
        {
          settings.serialLogBaudRate = newBaud;
          SerialLog.begin(settings.serialLogBaudRate);
        }
      }
      else if (incoming == '5')
        settings.timestampSerial ^= 1;
      else if (incoming == '6')
      {
        SerialPrint(F("Enter the timestamp token in decimal (0 to 255): "));
        int newToken = getNumber(menuTimeout); //Timeout after x seconds
        if (newToken < 0 || newToken > 255)
        {
          SerialPrintln(F("Error: token out of range"));
        }
        else
        {
          settings.timeStampToken = (uint8_t)newToken;
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
