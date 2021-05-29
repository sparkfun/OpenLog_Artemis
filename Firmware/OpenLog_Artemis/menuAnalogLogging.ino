void menuAnalogLogging()
{
  while (1)
  {
#if(HARDWARE_VERSION_MAJOR == 0)
    SerialPrintln(F(""));
    SerialPrintln(F("Note: VIN logging is only supported on V10+ hardware. X04 will show 0.0V."));
#endif
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Analog Logging"));

    if (settings.identifyBioSensorHubs == false)
    {
      SerialPrint(F("1) Log analog pin 11 (2V Max): "));
      if (settings.logA11 == true) SerialPrintln(F("Enabled. (Triggering is disabled)"));
      else SerialPrintln(F("Disabled"));
    }

    SerialPrint(F("2) Log analog pin 12 (TX) (2V Max): "));
    if (settings.useTxRxPinsForTerminal == true) SerialPrintln(F("Disabled. (TX and RX pins are being used for the Terminal)"));
    else if (settings.logA12 == true) SerialPrintln(F("Enabled. (Serial output is disabled)"));
    else SerialPrintln(F("Disabled"));

    SerialPrint(F("3) Log analog pin 13 (RX) (2V Max): "));
    if (settings.useTxRxPinsForTerminal == true) SerialPrintln(F("Disabled. (TX and RX pins are being used for the Terminal)"));
    else if (settings.logA13 == true) SerialPrintln(F("Enabled. (Serial logging is disabled)"));
    else SerialPrintln(F("Disabled"));

    if (settings.identifyBioSensorHubs == false)
    {
      SerialPrint(F("4) Log analog pin 32 (2V Max): "));
      if (settings.logA32 == true) SerialPrintln(F("Enabled. (Stop logging is disabled)"));
      else SerialPrintln(F("Disabled"));
    }

    SerialPrint(F("5) Log output type: "));
    if (settings.logAnalogVoltages == true) SerialPrintln(F("Calculated Voltage"));
    else SerialPrintln(F("Raw ADC reading"));

    SerialPrint(F("6) Log VIN (battery) voltage (6V Max): "));
    if (settings.logVIN == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      if (settings.identifyBioSensorHubs == false)
      {
        if(settings.logA11 == false)
        {
          settings.logA11 = true;
          // Disable triggering
          settings.useGPIO11ForTrigger = false;
          detachInterrupt(digitalPinToInterrupt(PIN_TRIGGER)); // Disable the interrupt
          pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up
          triggerEdgeSeen = false; // Make sure the flag is clear
        }
        else
          settings.logA11 = false;
      }
      else
      {
        SerialPrintln(F(""));
        SerialPrintln(F("Analog logging on pin 11 is not possible. Bio Sensor Hubs (Pulse Oximeters) are enabled."));
        SerialPrintln(F(""));
      }      
    }
    else if (incoming == '2')
    {
      if(settings.logA12 == false)
      {
        if (settings.useTxRxPinsForTerminal == true)
        {
          SerialPrintln(F(""));
          SerialPrintln(F("Analog logging on pin 12 is not possible. TX and RX pins are being used for the Terminal."));
          SerialPrintln(F(""));
        }
        else
        {
          online.serialOutput = false; // Disable serial output
          settings.outputSerial = false;
          settings.logA12 = true;
        }
      }
      else
        settings.logA12 = false;
    }
    else if (incoming == '3')
    {
      if(settings.logA13 == false)
      {
        if (settings.useTxRxPinsForTerminal == true)
        {
          SerialPrintln(F(""));
          SerialPrintln(F("Analog logging on pin 13 is not possible. TX and RX pins are being used for the Terminal."));
          SerialPrintln(F(""));
        }
        else
        {
          online.serialLogging = false; //Disable serial logging
          settings.logSerial = false;
          settings.logA13 = true;
        }
      }
      else
        settings.logA13 = false;
    }
    else if (incoming == '4')
    {
      if (settings.identifyBioSensorHubs == false)
      {
        if(settings.logA32 == false)
        {
          settings.logA32 = true;
          // Disable stop logging
          settings.useGPIO32ForStopLogging = false;
          detachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING)); // Disable the interrupt
          pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
        }
        else
          settings.logA32 = false;
      }
      else
      {
        SerialPrintln(F(""));
        SerialPrintln(F("Analog logging on pin 32 is not possible. Bio Sensor Hubs (Pulse Oximeters) are enabled."));
        SerialPrintln(F(""));
      }      
    }
    else if (incoming == '5')
      settings.logAnalogVoltages ^= 1;
    else if (incoming == '6')
    {
      settings.logVIN ^= 1;
    }
    else if (incoming == 'x')
      return;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
