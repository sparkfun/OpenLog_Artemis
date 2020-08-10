void menuAnalogLogging()
{
  while (1)
  {
#if(HARDWARE_VERSION_MAJOR == 0)
    Serial.println();
    Serial.println(F("Note: VIN logging is only supported on V10+ hardware. X04 will show 0.0V."));
#endif
    Serial.println();
    Serial.println(F("Menu: Configure Analog Logging"));

    Serial.print(F("1) Log analog pin 11 (2V Max): "));
    if (settings.logA11 == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("2) Log analog pin 12 (TX) (2V Max): "));
    if (settings.logA12 == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("3) Log analog pin 13 (RX) (2V Max): "));
    if (settings.logA13 == true) Serial.println(F("Enabled, Serial logging disabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("4) Log analog pin 32 (2V Max): "));
    if (settings.logA32 == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("5) Log output type: "));
    if (settings.logAnalogVoltages == true) Serial.println(F("Calculated Voltage"));
    else Serial.println(F("Raw ADC reading"));

    Serial.print(F("6) Log VIN (battery) voltage (6V Max): "));
    if (settings.logVIN == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.logA11 ^= 1;
    else if (incoming == '2')
      settings.logA12 ^= 1;
    else if (incoming == '3')
    {
      if(settings.logA13 == false)
      {
        settings.logSerial = false; //Disable serial logging
        settings.logA13 = true;
      }
      else
        settings.logA13 = false;
    }
    else if (incoming == '4')
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
