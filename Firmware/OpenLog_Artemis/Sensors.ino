//Init / begin comm with all enabled sensors
void beginSensors()
{
  //See what sensors are available.
  if (detectQwiicDevices() == true) //Returns false if no sensors are detected
    msg("Beginning Sensors:");
  else
    msg("No sensors detected on Qwiic bus");

  determineMaxI2CSpeed(); //Try for 400kHz but reduce to 100kHz or low if certain devices are attached

  if (qwiicAvailable.LPS25HB && settings.sensor_LPS25HB.log && !qwiicOnline.LPS25HB)
  {
    if (pressureSensor.begin(qwiic) == true) //Wire port, Address.
    {
      qwiicOnline.LPS25HB = true;
      msg("LPS25HB Online");
    }
    else
      msg("LPS25HB failed to respond. Check wiring and address jumper.");
  }

  if (qwiicAvailable.NAU7802 && settings.sensor_NAU7802.log && !qwiicOnline.NAU7802)
  {
    if (nauScale.begin(qwiic) == true) //Wire port
    {
      nauScale.setSampleRate(NAU7802_SPS_320); //Sample rate can be set to 10, 20, 40, 80, or 320Hz

      //Setup scale with stored values
      nauScale.setCalibrationFactor(settings.sensor_NAU7802.calibrationFactor);
      nauScale.setZeroOffset(settings.sensor_NAU7802.zeroOffset);
      qwiicOnline.NAU7802 = true;
      msg("NAU7802 Online");
    }
    else
      msg("NAU7802 failed to respond. Check wiring.");
  }

  if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && !qwiicOnline.uBlox)
  {
    if (myGPS.begin(qwiic, 0x42) == true) //Wire port, Address. Default is 0x42.
    {
      myGPS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

      //myGPS.setAutoPVT(true); //Tell the GPS to "send" each solution
      myGPS.setAutoPVT(false);

      if (settings.recordPerSecond <= 10)
        myGPS.setNavigationFrequency(settings.recordPerSecond); //Set output rate equal to our query rate
      else
        myGPS.setNavigationFrequency(10); //Max output depends on the module used.

      myGPS.saveConfiguration(); //Save the current settings to flash and BBR

      qwiicOnline.uBlox = true;
      msg("uBlox GPS Online");
    }
    else
      msg("uBlox GPS failed to respond. Check wiring and I2C address of module with uCenter.");
  }

  if (qwiicAvailable.MCP9600 && settings.sensor_MCP9600.log && !qwiicOnline.MCP9600)
  {
    if (thermoSensor.begin(0x66, qwiic) == true) //Address, Wire port
    {
      //set the resolution on the ambient (cold) junction
      Ambient_Resolution ambientRes = RES_ZERO_POINT_0625; //_25 and _0625
      thermoSensor.setAmbientResolution(ambientRes);

      Thermocouple_Resolution thermocoupleRes = RES_14_BIT; //12, 14, 16, and 18
      thermoSensor.setThermocoupleResolution(thermocoupleRes);

      qwiicOnline.MCP9600 = true;
      msg("MCP9600 Online");
    }
    else
      msg("MCP9600 failed to respond. Check wiring and address jumper.");
  }

  if (qwiicAvailable.VCNL4040 && settings.sensor_VCNL4040.log && !qwiicOnline.VCNL4040)
  {
    if (proximitySensor_VCNL4040.begin(qwiic) == true) //Wire port
    {
      proximitySensor_VCNL4040.powerOnAmbient(); //Turn on ambient sensing

      proximitySensor_VCNL4040.setLEDCurrent(settings.sensor_VCNL4040.LEDCurrent);
      proximitySensor_VCNL4040.setIRDutyCycle(settings.sensor_VCNL4040.IRDutyCycle);
      proximitySensor_VCNL4040.setProxIntegrationTime(settings.sensor_VCNL4040.proximityIntegrationTime);
      proximitySensor_VCNL4040.setProxResolution(settings.sensor_VCNL4040.resolution);
      proximitySensor_VCNL4040.setAmbientIntegrationTime(settings.sensor_VCNL4040.ambientIntegrationTime);

      qwiicOnline.VCNL4040 = true;
      msg("VCNL4040 Online");
    }
    else
      msg("VCNL4040 failed to respond. Check wiring.");
  }

  if (qwiicAvailable.VL53L1X && settings.sensor_VL53L1X.log && !qwiicOnline.VL53L1X)
  {
    if (distanceSensor_VL53L1X.begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
    {
      if (settings.sensor_VL53L1X.distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
        distanceSensor_VL53L1X.setDistanceModeShort();
      else
        distanceSensor_VL53L1X.setDistanceModeLong();

      distanceSensor_VL53L1X.setIntermeasurementPeriod(settings.sensor_VL53L1X.intermeasurementPeriod - 1);
      distanceSensor_VL53L1X.setXTalk(settings.sensor_VL53L1X.crosstalk);
      distanceSensor_VL53L1X.setOffset(settings.sensor_VL53L1X.offset);

      distanceSensor_VL53L1X.startRanging(); //Write configuration bytes to initiate measurement

      qwiicOnline.VL53L1X = true;
      msg("VL53L1X Online");
    }
    else
      msg("VX53L1X failed to respond. Check wiring.");
  }

  if (qwiicAvailable.TMP117 && settings.sensor_TMP117.log && !qwiicOnline.TMP117)
  {
    if (tempSensor_TMP117.begin(0x48, qwiic) == true) //Address, Wire port
    {
      tempSensor_TMP117.setConversionAverageMode(settings.sensor_TMP117.conversionAverageMode);
      tempSensor_TMP117.setConversionCycleBit(settings.sensor_TMP117.conversionCycle);
      tempSensor_TMP117.setContinuousConversionMode();

      qwiicOnline.TMP117 = true;
      msg("TMP117 Online");
    }
    else
      msg("TMP117 failed to respond. Check wiring and address jumpers.");
  }

  if (qwiicAvailable.CCS811 && settings.sensor_CCS811.log && !qwiicOnline.CCS811)
  {
    if (vocSensor_CCS811.begin(qwiic) == true) //Wire port
    {
      qwiicOnline.CCS811 = true;
      msg("CCS811 Online");
    }
    else
      msg("CCS811 failed to respond. Check wiring and address jumpers. Adr must be 0x5B.");
  }

  if (qwiicAvailable.BME280 && settings.sensor_BME280.log && !qwiicOnline.BME280)
  {
    if (phtSensor_BME280.beginI2C(qwiic) == true) //Wire port
    {
      qwiicOnline.BME280 = true;
      msg("BME280 Online");
    }
    else
      msg("BME280 failed to respond. Check wiring and address jumpers. Adr must be 0x77.");
  }

  if (qwiicAvailable.SGP30 && settings.sensor_SGP30.log && !qwiicOnline.SGP30)
  {
    if (vocSensor_SGP30.begin(qwiic) == true) //Wire port
    {
      //Initializes sensor for air quality readings
      vocSensor_SGP30.initAirQuality();

      qwiicOnline.SGP30 = true;
      msg("SGP30 Online");
    }
    else
      msg("SGP30 failed to respond. Check wiring.");
  }

  if (qwiicAvailable.VEML6075 && settings.sensor_VEML6075.log && !qwiicOnline.VEML6075)
  {
    if (uvSensor_VEML6075.begin(qwiic) == true) //Wire port
    {
      qwiicOnline.VEML6075 = true;
      msg("VEML6075 Online");
    }
    else
      msg("VEML6075 failed to respond. Check wiring.");
  }
}

//Query each enabled sensor for it's most recent data
void getData()
{
  measurementCount++;

  outputData = "";
  String helperText = "";

  if (settings.logRTC)
  {
    //Decide if we are using the internal RTC or GPS for timestamps
    if (settings.getRTCfromGPS == false)
    {
      myRTC.getTime();

      if (settings.logDate)
      {
        char rtcDate[11]; //10/12/2019
        if (settings.americanDateStyle == true)
          sprintf(rtcDate, "%02d/%02d/20%02d", myRTC.month, myRTC.dayOfMonth, myRTC.year);
        else
          sprintf(rtcDate, "%02d/%02d/20%02d", myRTC.dayOfMonth, myRTC.month, myRTC.year);
        outputData += String(rtcDate) + ",";
        helperText += "rtcDate,";
      }

      if (settings.logTime)
      {
        char rtcTime[12]; //09:14:37.41
        int adjustedHour = myRTC.hour;
        if (settings.hour24Style == false)
        {
          if (adjustedHour > 12) adjustedHour -= 12;
        }
        sprintf(rtcTime, "%02d:%02d:%02d.%02d", adjustedHour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
        outputData += String(rtcTime) + ",";
        helperText += "rtcTime,";
      }
    } //end if use RTC for timestamp
    else //Use GPS for timestamp
    {
      Serial.println("Print GPS Timestamp / not yet implemented");
    }
  }

  if (settings.logA11)
  {
    unsigned int analog11 = analogRead(11);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog11 * 2 / 16384.0;
      char tempFloat[5];
      sprintf(tempFloat, "%.2f", voltage);
      outputData += String(tempFloat);
    }
    else
      outputData += String(analog11);

    outputData += ",";
    helperText += "analog_11,";
  }

  if (settings.logA12)
  {
    unsigned int analog12 = analogRead(12);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog12 * 2 / 16384.0;
      char tempFloat[5];
      sprintf(tempFloat, "%.2f", voltage);
      outputData += String(tempFloat);
    }
    else
      outputData += String(analog12);

    outputData += ",";
    helperText += "analog_12,";
  }

  if (settings.logA13)
  {
    unsigned int analog13 = analogRead(13);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog13 * 2 / 16384.0;
      char tempFloat[5];
      sprintf(tempFloat, "%.2f", voltage);
      outputData += String(tempFloat);
    }
    else
      outputData += String(analog13);

    outputData += ",";
    helperText += "analog_13,";
  }

  if (settings.logA32)
  {
    unsigned int analog32 = analogRead(32);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog32 * 2 / 16384.0;
      char tempFloat[5];
      sprintf(tempFloat, "%.2f", voltage);
      outputData += String(tempFloat);
    }
    else
      outputData += String(analog32);

    outputData += ",";
    helperText += "analog_32,";
  }

  if (online.IMU)
  {
    if (myICM.dataReady())
    {
      myICM.getAGMT(); //Update values

      char tempFloat[50];
      if (settings.logIMUAccel)
      {
        sprintf(tempFloat, "%.2f,%.2f,%.2f,", myICM.accX(), myICM.accY(), myICM.accZ());
        outputData += (String)tempFloat;
        helperText += "aX,aY,aZ,";
      }
      if (settings.logIMUGyro)
      {
        sprintf(tempFloat, "%.2f,%.2f,%.2f,", myICM.gyrX(), myICM.gyrY(), myICM.gyrZ());
        outputData += (String)tempFloat;
        helperText += "gX,gY,gZ,";
      }
      if (settings.logIMUMag)
      {
        sprintf(tempFloat, "%.2f,%.2f,%.2f,", myICM.magX(), myICM.magY(), myICM.magZ());
        outputData += (String)tempFloat;
        helperText += "mX,mY,mZ,";
      }
      if (settings.logIMUTemp)
      {
        sprintf(tempFloat, "%.2f,", myICM.temp());
        outputData += (String)tempFloat;
        helperText += "imu_degC,";
      }
    }
  }

  if (qwiicOnline.LPS25HB && settings.sensor_LPS25HB.log)
  {
    if (settings.sensor_LPS25HB.logPressure)
    {
      outputData += (String)pressureSensor.getPressure_hPa() + ",";
      helperText += "pressure_hPa,";
    }
    if (settings.sensor_LPS25HB.logTemp)
    {
      outputData += (String)pressureSensor.getTemperature_degC() + ",";
      helperText += "pressure_degC,";
    }
  }

  if (qwiicOnline.NAU7802 && settings.sensor_NAU7802.log)
  {
    if (settings.sensor_NAU7802.log)
    {
      float currentWeight = nauScale.getWeight(false, settings.sensor_NAU7802.averageAmount); //Do not allow negative weights, take average of X readings
      static char weight[30];
      sprintf(weight, "%.*f,", settings.sensor_NAU7802.decimalPlaces, currentWeight);

      outputData += weight;
      helperText += "weight(no unit),";
    }
  }

  if (qwiicOnline.MCP9600 && settings.sensor_MCP9600.log)
  {
    if (settings.sensor_MCP9600.logTemp)
    {
      outputData += (String)thermoSensor.getThermocoupleTemp() + ",";
      helperText += "thermo_degC,";
    }
    if (settings.sensor_MCP9600.logAmbientTemp)
    {
      outputData += (String)thermoSensor.getAmbientTemp() + ",";
      helperText += "thermo_ambientDegC,";
    }
  }

  if (qwiicOnline.uBlox && settings.sensor_uBlox.log)
  {
    //Calling getPVT returns true if there actually is a fresh navigation solution available.
    //getPVT will block/wait up to 1000ms to receive new data. This will affect the global reading rate.
    //    if (myGPS.getPVT())
    myGPS.getPVT();
    //    {
    if (settings.sensor_uBlox.logDate)
    {
      char gpsDate[11]; //10/12/2019
      if (settings.americanDateStyle == true)
        sprintf(gpsDate, "%02d/%02d/%d", myGPS.getMonth(), myGPS.getDay(), myGPS.getYear());
      else
        sprintf(gpsDate, "%02d/%02d/%d", myGPS.getDay(), myGPS.getMonth(), myGPS.getYear());
      outputData += String(gpsDate) + ",";
      helperText += "gps_Date,";
    }
    if (settings.sensor_uBlox.logTime)
    {
      char gpsTime[12]; //09:14:37.41
      int adjustedHour = myGPS.getHour();
      if (settings.hour24Style == false)
      {
        if (adjustedHour > 12) adjustedHour -= 12;
      }
      sprintf(gpsTime, "%02d:%02d:%02d.%03d", adjustedHour, myGPS.getMinute(), myGPS.getSecond(), myGPS.getMillisecond());
      outputData += String(gpsTime) + ",";
      helperText += "gps_Time,";
    }
    if (settings.sensor_uBlox.logPosition)
    {
      outputData += (String)myGPS.getLatitude() + ",";
      outputData += (String)myGPS.getLongitude() + ",";
      helperText += "gps_Lat,gps_Long,";
    }
    if (settings.sensor_uBlox.logAltitude)
    {
      outputData += (String)myGPS.getAltitude() + ",";
      helperText += "gps_Alt,";
    }
    if (settings.sensor_uBlox.logAltitudeMSL)
    {
      outputData += (String)myGPS.getAltitudeMSL() + ",";
      helperText += "gps_AltMSL,";
    }
    if (settings.sensor_uBlox.logSIV)
    {
      outputData += (String)myGPS.getSIV() + ",";
      helperText += "gps_SIV,";
    }
    if (settings.sensor_uBlox.logFixType)
    {
      outputData += (String)myGPS.getFixType() + ",";
      helperText += "gps_FixType,";
    }
    if (settings.sensor_uBlox.logCarrierSolution)
    {
      outputData += (String)myGPS.getCarrierSolutionType() + ","; //0=No solution, 1=Float solution, 2=Fixed solution. Useful when querying module to see if it has high-precision RTK fix.
      helperText += "gps_CarrierSolution,";
    }
    if (settings.sensor_uBlox.logGroundSpeed)
    {
      outputData += (String)myGPS.getGroundSpeed() + ",";
      helperText += "gps_GroundSpeed,";
    }
    if (settings.sensor_uBlox.logHeadingOfMotion)
    {
      outputData += (String)myGPS.getHeading() + ",";
      helperText += "gps_Heading,";
    }
    if (settings.sensor_uBlox.logpDOP)
    {
      outputData += (String)myGPS.getPDOP() + ",";
      helperText += "gps_pDOP,";
    }
    if (settings.sensor_uBlox.logiTOW)
    {
      outputData += (String)myGPS.getTimeOfWeek() + ",";
      helperText += "gps_iTOW,";
    }

    myGPS.flushPVT(); //Mark all PVT data as used
    //    }
  }

  if (qwiicOnline.VCNL4040 && settings.sensor_VCNL4040.log)
  {
    if (settings.sensor_VCNL4040.logProximity)
    {
      outputData += (String)proximitySensor_VCNL4040.getProximity() + ",";
      helperText += "prox(no unit),";
    }
    if (settings.sensor_VCNL4040.logAmbientLight)
    {
      outputData += (String)proximitySensor_VCNL4040.getAmbient() + ",";
      helperText += "ambient_lux,";
    }
  }

  if (qwiicOnline.VL53L1X && settings.sensor_VL53L1X.log)
  {
    if (settings.sensor_VL53L1X.logDistance)
    {
      outputData += (String)distanceSensor_VL53L1X.getDistance() + ",";
      helperText += "distance_mm,";
    }
    if (settings.sensor_VL53L1X.logRangeStatus)
    {
      outputData += (String)distanceSensor_VL53L1X.getRangeStatus() + ",";
      helperText += "distance_rangeStatus(0=good),";
    }
    if (settings.sensor_VL53L1X.logSignalRate)
    {
      outputData += (String)distanceSensor_VL53L1X.getSignalRate() + ",";
      helperText += "distance_signalRate,";
    }
  }

  if (qwiicOnline.TMP117 && settings.sensor_TMP117.log)
  {
    if (settings.sensor_TMP117.logTemp)
    {
      outputData += (String)tempSensor_TMP117.readTempC() + ",";
      helperText += "temp_degC,";
    }
  }

  if (qwiicOnline.CCS811 && settings.sensor_CCS811.log)
  {
    //if (vocSensor_CCS811.dataAvailable())
    //{
    //Get data regardless of if it is new or not
    //Sensor will have new data every 1 second
    vocSensor_CCS811.readAlgorithmResults();

    if (settings.sensor_CCS811.logTVOC)
    {
      outputData += (String)vocSensor_CCS811.getTVOC() + ",";
      helperText += "tvoc_ppb,";
    }
    if (settings.sensor_CCS811.logCO2)
    {
      outputData += (String)vocSensor_CCS811.getCO2() + ",";
      helperText += "co2_ppm,";
    }
    //}
  }

  if (qwiicOnline.BME280 && settings.sensor_BME280.log)
  {
    if (settings.sensor_BME280.logPressure)
    {
      outputData += (String)phtSensor_BME280.readFloatPressure() + ",";
      helperText += "pressure_Pa,";
    }
    if (settings.sensor_BME280.logHumidity)
    {
      outputData += (String)phtSensor_BME280.readFloatHumidity() + ",";
      helperText += "humidity_%,";
    }
    if (settings.sensor_BME280.logAltitude)
    {
      outputData += (String)phtSensor_BME280.readFloatAltitudeMeters() + ",";
      helperText += "altitude_m,";
    }
    if (settings.sensor_BME280.logTemp)
    {
      outputData += (String)phtSensor_BME280.readTempC() + ",";
      helperText += "temp_degC,";
    }
  }

  if (qwiicOnline.SGP30 && settings.sensor_SGP30.log)
  {
    vocSensor_SGP30.measureAirQuality();

    if (settings.sensor_SGP30.logTVOC)
    {
      outputData += (String)vocSensor_SGP30.TVOC + ",";
      helperText += "tvoc_ppb,";
    }
    if (settings.sensor_SGP30.logCO2)
    {
      outputData += (String)vocSensor_SGP30.CO2 + ",";
      helperText += "co2_ppm,";
    }
  }

  if (qwiicOnline.VEML6075 && settings.sensor_VEML6075.log)
  {
    if (settings.sensor_VEML6075.logUVA)
    {
      outputData += (String)uvSensor_VEML6075.uva() + ",";
      helperText += "uva,";
    }
    if (settings.sensor_VEML6075.logUVB)
    {
      outputData += (String)uvSensor_VEML6075.uvb() + ",";
      helperText += "uvb,";
    }
    if (settings.sensor_VEML6075.logUVIndex)
    {
      outputData += (String)uvSensor_VEML6075.index() + ",";
      helperText += "uvIndex,";
    }
  }

  if (settings.logHertz)
  {
    //Calculate the actual update rate based on the sketch start time and the
    //number of updates we've completed.
    float actualRate = measurementCount * 1000.0 / (millis() - measurementStartTime);

    outputData += (String)actualRate + ","; //Hz
    helperText += "output_Hz,";
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

  if (qwiicAvailable.MCP9600 == true && settings.sensor_MCP9600.log == true)
    maxSpeed = 100000;
  else if (settings.sensor_uBlox.i2cSpeed == 100000)
    maxSpeed = 100000;

  qwiic.setClock(maxSpeed);
}

void qwiicPowerOn()
{
  digitalWrite(PIN_QWIIC_PWR, LOW);
}
void qwiicPowerOff()
{
  digitalWrite(PIN_QWIIC_PWR, HIGH);
}
