void menuDebug()
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
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
