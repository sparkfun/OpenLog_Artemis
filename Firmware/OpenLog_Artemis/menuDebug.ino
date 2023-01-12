void menuDebug(bool *dataLogUpdated, bool *serialLogUpdated)
{
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Debug Settings"));

    SerialPrint(F("1) Debug Messages: "));
    if (settings.printDebugMessages == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("2) Reset on Zero Device Count: "));
    if (settings.resetOnZeroDeviceCount == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("3) GNSS Debug Messages: "));
    if (settings.printGNSSDebugMessages == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("4) Only Open Main Menu With Printable Char: "));
    if (settings.openMenuWithPrintable == true) SerialPrintln(F("Yes"));
    else SerialPrintln(F("No"));

    SerialPrintln(F("5) Reboot the logger"));

    SerialPrintln(F("6) Set nextDataLogNumber"));

    SerialPrintln(F("7) Set nextSerialLogNumber"));

    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.printDebugMessages ^= 1;
    }
    else if (incoming == '2')
    {
      if (settings.resetOnZeroDeviceCount == false)
      {
        SerialPrintln(F(""));
        SerialPrintln(F("Enabling resetOnZeroDeviceCount will cause the OLA to completely reset if no devices are found on the Qwiic bus."));
        SerialPrintln(F("Do not enable this option if you are only logging IMU or Serial data."));
        SerialPrintln(F("Are you sure? Press 'y' to confirm: "));
        byte bContinue = getByteChoice(menuTimeout);
        if (bContinue == 'y')
        {
          settings.resetOnZeroDeviceCount ^= 1;
        }
        else
          SerialPrintln(F("\"resetOnZeroDeviceCount\"  aborted"));
      }
      else
      {
        settings.resetOnZeroDeviceCount ^= 1;
      }
    }
    else if (incoming == '3')
    {
      settings.printGNSSDebugMessages ^= 1;
    }
    else if (incoming == '4')
    {
      settings.openMenuWithPrintable ^= 1;
    }
    else if (incoming == '5')
    {
        SerialPrint(F("Are you sure? Press 'y' to confirm: "));
        byte bContinue = getByteChoice(menuTimeout);
        if (bContinue == 'y')
        {
          SerialPrintln(F("y"));
          resetArtemis();
        }
    }
    else if (incoming == '6')
    {
        SerialPrint(F("Enter the new nextDataLogNumber ( >= 1 ) ( or x to abort ): "));
        int64_t newNum = getNumber(menuTimeout);
        if ((newNum > 0) && (newNum < 99999)) // Needs to be >= 1 as it is the _next_ file
        {
          settings.nextDataLogNumber = newNum;
          *dataLogUpdated = true;
        }
    }
    else if (incoming == '7')
    {
        SerialPrint(F("Enter the new nextSerialLogNumber ( >= 1 ) ( or x to abort ): "));
        int64_t newNum = getNumber(menuTimeout);
        if ((newNum > 0) && (newNum < 99999)) // Needs to be >= 1 as it is the _next_ file
        {
          settings.nextSerialLogNumber = newNum;
          *serialLogUpdated = true;
        }
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
