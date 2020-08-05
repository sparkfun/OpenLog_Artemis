void menuIMU()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure IMU"));

    Serial.print(F("1) Sensor Logging: "));
    if (settings.enableIMU == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    if (settings.enableIMU == true)
    {
      Serial.print(F("2) Toggle Accelerometer Logging: "));
      if (settings.logIMUAccel) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      Serial.print(F("3) Toggle Gyro Logging: "));
      if (settings.logIMUGyro) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      Serial.print(F("4) Toggle Magnotometer Logging: "));
      if (settings.logIMUMag) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      Serial.print(F("5) Toggle Temperature Logging: "));
      if (settings.logIMUTemp) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));
    }
    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.enableIMU ^= 1;
      if (settings.enableIMU == true) beginIMU();
      else online.IMU = false;
    }
    else if (settings.enableIMU == true)
    {
      if (incoming == '2')
        settings.logIMUAccel ^= 1;
      else if (incoming == '3')
        settings.logIMUGyro ^= 1;
      else if (incoming == '4')
        settings.logIMUMag ^= 1;
      else if (incoming == '5')
        settings.logIMUTemp ^= 1;
      else if (incoming == 'x')
        break;
      else if (incoming == STATUS_GETBYTE_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
