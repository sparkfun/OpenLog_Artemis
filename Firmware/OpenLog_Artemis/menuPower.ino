void menuPower()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Power Options");

    Serial.print("1) Turn off Qwiic bus power between readings (>2s): ");
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println("Yes");
    else Serial.println("No");

#if(HARDWARE_VERSION_MAJOR >= 1)
    Serial.print("2) Power LED During Sleep: ");
    if (settings.enablePwrLedDuringSleep == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("3) VIN measurement correction factor: ");
    Serial.printf("%.3f\n", settings.vinCorrectionFactor);
#endif

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.powerDownQwiicBusBetweenReads ^= 1;
    }
#if(HARDWARE_VERSION_MAJOR >= 1)
    else if (incoming == '2')
    {
      settings.enablePwrLedDuringSleep ^= 1;
    }
    else if (incoming == '3')
    {
      Serial.println("Enter the new correction factor:");
      double tempCF = getDouble(menuTimeout); //Timeout after x seconds
      if ((tempCF < 1.0) || (tempCF > 2.0))
        Serial.println("Error: Correction factor out of range");
      else
        settings.vinCorrectionFactor = (float)tempCF;
    }
#endif
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
