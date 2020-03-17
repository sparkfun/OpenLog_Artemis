void menuAnalogLogging()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Analog (2V Max) Logging");

    Serial.print("1) Log analog pin 11: ");
    if (settings.logA11 == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("2) Log analog pin 12 (TX): ");
    if (settings.logA12 == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("3) Log analog pin 13 (RX): ");
    if (settings.logA13 == true) Serial.println("Enabled, Serial logging disabled");
    else Serial.println("Disabled");

    Serial.print("4) Log analog pin 32: ");
    if (settings.logA32 == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("5) Log output type: ");
    if (settings.logAnalogVoltages == true) Serial.println("Calculated Voltage");
    else Serial.println("Raw ADC reading");

    Serial.println("x) Exit");

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
      settings.logA32 ^= 1;
    else if (incoming == '5')
      settings.logAnalogVoltages ^= 1;
    else if (incoming == 'x')
      return;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      return;
    else
      printUnknown(incoming);
  }
}
