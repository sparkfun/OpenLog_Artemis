void menuIMU()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure IMU");

    Serial.print("1) Sensor Logging: ");
    if (settings.enableIMU == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.enableIMU == true)
    {
      Serial.print("2) Toggle Accel Logging: ");
      if (settings.logIMUAccel) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Toggle Gyro Logging: ");
      if (settings.logIMUGyro) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Toggle Magnometer Logging: ");
      if (settings.logIMUMag) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Toggle Temperature Logging: ");
      if (settings.logIMUTemp) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
