//Init / begin comm with all enabled sensors
bool beginSensors()
{
  beginSensorOutput = "";

  //If no sensors are available then return
  if (detectQwiicDevices() == false)
    return false;

  determineMaxI2CSpeed(); //Try for 400kHz but reduce to 100kHz or low if certain devices are attached

  if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && !qwiicOnline.uBlox)
  {
    if (gpsSensor_ublox.begin(qwiic, 0x42) == true) //Wire port, Address. Default is 0x42.
    {
      gpsSensor_ublox.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

      gpsSensor_ublox.setAutoPVT(false);

      //if (settings.recordPerSecond <= 10)
      if (1000000UL / settings.usBetweenReadings <= 10)
        gpsSensor_ublox.setNavigationFrequency(1000000 / settings.usBetweenReadings); //Set output rate equal to our query rate
      else
        gpsSensor_ublox.setNavigationFrequency(10); //Max output depends on the module used.

      gpsSensor_ublox.saveConfiguration(); //Save the current settings to flash and BBR

      qwiicOnline.uBlox = true;
      beginSensorOutput += "uBlox GPS Online\n";
    }
    else
      beginSensorOutput += "uBlox GPS failed to respond. Check wiring and I2C address of module with uCenter.\n";
  }

  return true;
}

//Query each enabled sensor for it's most recent data
void getData()
{
  measurementCount++;

  outputData = "";
  String helperText = "";

  if (qwiicOnline.uBlox && settings.sensor_uBlox.log)
  {
    gpsSensor_ublox.getPVT();
    if (settings.sensor_uBlox.logUBXNAVPVT)
    {
      outputData += (String)gpsSensor_ublox.getLatitude() + ",";
      outputData += (String)gpsSensor_ublox.getLongitude();
      helperText += "gps_Lat,gps_Long";
    }
    gpsSensor_ublox.flushPVT(); //Mark all PVT data as used
  }

  outputData += '\n';
  helperText += '\n';

  totalCharactersPrinted += outputData.length();

  if (settings.showHelperText == true)
  {
    if (helperTextPrinted == false)
    {
      helperTextPrinted = true;
      outputData = helperText + outputData;
    }
  }
}

//If certain devices are attached, we need to reduce the I2C max speed
void determineMaxI2CSpeed()
{
  uint32_t maxSpeed = 400000; //Assume 400kHz

  if (settings.sensor_uBlox.i2cSpeed == 100000)
    maxSpeed = 100000;

  //If user wants to limit the I2C bus speed, do it here
  if (maxSpeed > settings.qwiicBusMaxSpeed)
    maxSpeed = settings.qwiicBusMaxSpeed;

  qwiic.setClock(maxSpeed);
}
