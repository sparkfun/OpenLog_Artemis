//Init / begin comm with all enabled sensors
void beginSensors()
{
  if (log_LPS25HB)
  {
    if (pressureSensor.begin() == true) //Wire port, Address.
      online_LPS25HB = true;
    else
      msg("LPS25HB failed to respond. Check wiring and address jumper.");
  }

  if (log_VL53L1X)
  {
    if (distanceSensor.begin() == 0) //Begin returns 0 on a good init.
      online_VL53L1X = true;
    else
      msg("VL53L1X failed to respond. Check wiring.");
  }

  if (log_uBlox)
  {
    if (myGPS.begin() == true) //Wire port, Address. Default is 0x42.
    {
      myGPS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

      if (recordPerSecond <= 10)
        myGPS.setNavigationFrequency(recordPerSecond); //Set output rate equal to our query rate
      else
        myGPS.setNavigationFrequency(10); //Max output depends on the module used.

      myGPS.saveConfiguration(); //Save the current settings to flash and BBR

      online_uBlox = true;
    }
    else
      msg("uBlox GPS failed to respond. Check wiring and I2C address of module with uCenter.");
  }
}

//Query each enabled sensor for it's most recent data
void getData()
{
  if (log_LPS25HB && online_LPS25HB)
  {
    outputData += pressureSensor.getPressure_hPa();
    outputData += ",";
    outputData += pressureSensor.getTemperature_degC();
    outputData += ",";
  }

  if (log_VL53L1X && online_VL53L1X)
  {
    distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
    int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
    distanceSensor.stopRanging();

    outputData += distance; //mm
    outputData += ",";
  }

  if (log_uBlox && online_uBlox)
  {
    long latitude = myGPS.getLatitude();
    long longitude = myGPS.getLongitude();
    long altitude = myGPS.getAltitude();
    byte SIV = myGPS.getSIV();

    outputData += latitude;
    outputData += ",";
    outputData += longitude;
    outputData += ",";
    outputData += altitude;
    outputData += ",";
    outputData += SIV;
    outputData += ",";
  }

  if (log_RTC && online_RTC)
  {
    myRTC.getTime();
    char rtcTime[12]; //09:14:37.41
    sprintf(rtcTime, "%02d:%02d:%02d.%02d", myRTC.hour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
    outputData += String(rtcTime);
    outputData += ",";

    char rtcDate[11]; //10/12/2019
    sprintf(rtcDate, "%02d/%02d/20%02d", myRTC.month, myRTC.dayOfMonth, myRTC.year);
    outputData += String(rtcDate);
    outputData += ",";
  }

  if (log_Hertz && online_Hertz)
  {
    //Calculate the actual update rate based on the sketch start time and the
    //number of updates we've completed.
    float actualRate = updateCount++ / ((millis() - overallStartTime) / 1000.0);

    outputData += actualRate; //Hz
    outputData += "Hz,";
  }

  outputData += '\n';
}
