void menuPower()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Power Options");

    Serial.print("1) Turn off Qwiic bus power between readings (>2s): ");
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println("Yes");
    else Serial.println("No");

    Serial.print("2) Use pin 32 to Stop Logging: ");
    if (settings.useGPIO32ForStopLogging == true) Serial.println("Yes");
    else Serial.println("No");

#if(HARDWARE_VERSION_MAJOR >= 1)
    Serial.print("3) Power LED During Sleep: ");
    if (settings.enablePwrLedDuringSleep == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("4) VIN measurement correction factor: ");
    Serial.printf("%.3f\n", settings.vinCorrectionFactor);
#endif

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.powerDownQwiicBusBetweenReads ^= 1;
    }
    else if (incoming == '2')
    {
      if (settings.useGPIO32ForStopLogging == true)
      {
        // Disable stop logging
        settings.useGPIO32ForStopLogging = false;
        detachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING)); // Disable the interrupt
        pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
      }
      else
      {
        // Enable stop logging
        settings.useGPIO32ForStopLogging = true;
        pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
        delay(1); // Let the pin stabilize
        attachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING), stopLoggingISR, FALLING); // Enable the interrupt
        stopLoggingSeen = false; // Make sure the flag is clear
        settings.logA32 = false; // Disable analog logging on pin 32
      }
    }
#if(HARDWARE_VERSION_MAJOR >= 1)
    else if (incoming == '3')
    {
      settings.enablePwrLedDuringSleep ^= 1;
    }
    else if (incoming == '4')
    {
      Serial.println("Please measure the voltage on the MEAS pin and enter it here:");
      float tempCF = (float)getDouble(menuTimeout); //Timeout after x seconds
      int div3 = analogRead(PIN_VIN_MONITOR); //Read VIN across a 1/3 resistor divider
      float vin = (float)div3 * 3.0 * 2.0 / 16384.0; //Convert 1/3 VIN to VIN (14-bit resolution)
      tempCF = tempCF / vin; //Calculate the new correction factor
      if ((tempCF < 1.0) || (tempCF > 2.0))
        Serial.println("Error: Correction factor out of range");
      else
        settings.vinCorrectionFactor = tempCF;
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
