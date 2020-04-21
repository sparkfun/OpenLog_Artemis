//Init / begin comm with all enabled sensors
bool beginSensors()
{
  beginSensorOutput = "";

  //If no sensors are available then return
  if (detectQwiicDevices() == false)
    return false;

  determineMaxI2CSpeed(); //Try for 400kHz but reduce to 100kHz or low if certain devices are attached

  if (qwiicAvailable.LPS25HB && settings.sensor_LPS25HB.log && !qwiicOnline.LPS25HB)
  {
    if (pressureSensor_LPS25HB.begin(qwiic) == true) //Wire port, Address.
    {
      qwiicOnline.LPS25HB = true;
      beginSensorOutput += "LPS25HB Online\n";
    }
    else
      beginSensorOutput + "LPS25HB failed to respond. Check wiring and address jumper.\n";
  }

  if (qwiicAvailable.NAU7802 && settings.sensor_NAU7802.log && !qwiicOnline.NAU7802)
  {
    if (loadcellSensor_NAU7802.begin(qwiic) == true) //Wire port
    {
      loadcellSensor_NAU7802.setSampleRate(NAU7802_SPS_320); //Sample rate can be set to 10, 20, 40, 80, or 320Hz

      //Setup scale with stored values
      loadcellSensor_NAU7802.setCalibrationFactor(settings.sensor_NAU7802.calibrationFactor);
      loadcellSensor_NAU7802.setZeroOffset(settings.sensor_NAU7802.zeroOffset);
      qwiicOnline.NAU7802 = true;
      beginSensorOutput += "NAU7802 Online\n";
    }
    else
      beginSensorOutput += "NAU7802 failed to respond. Check wiring\n.";
  }

  if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && !qwiicOnline.uBlox)
  {
    if (gpsSensor_ublox.begin(qwiic, 0x42) == true) //Wire port, Address. Default is 0x42.
    {
      gpsSensor_ublox.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

      //gpsSensor_ublox.setAutoPVT(true); //Tell the GPS to "send" each solution
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

  if (qwiicAvailable.MCP9600 && settings.sensor_MCP9600.log && !qwiicOnline.MCP9600)
  {
    if (thermoSensor_MCP9600.begin(0x66, qwiic) == true) //Address, Wire port
    {
      //set the resolution on the ambient (cold) junction
      Ambient_Resolution ambientRes = RES_ZERO_POINT_0625; //_25 and _0625
      thermoSensor_MCP9600.setAmbientResolution(ambientRes);

      Thermocouple_Resolution thermocoupleRes = RES_14_BIT; //12, 14, 16, and 18
      thermoSensor_MCP9600.setThermocoupleResolution(thermocoupleRes);

      qwiicOnline.MCP9600 = true;
      beginSensorOutput += "MCP9600 Online\n";
    }
    else
      beginSensorOutput += "MCP9600 failed to respond. Check wiring and address jumper.\n";
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
      beginSensorOutput += "VCNL4040 Online\n";
    }
    else
      beginSensorOutput += "VCNL4040 failed to respond. Check wiring.\n";
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
      beginSensorOutput += "VL53L1X Online\n";
    }
    else
      beginSensorOutput += "VX53L1X failed to respond. Check wiring.\n";
  }

  if (qwiicAvailable.TMP117 && settings.sensor_TMP117.log && !qwiicOnline.TMP117)
  {
    if (tempSensor_TMP117.begin(0x48, qwiic) == true) //Address, Wire port
    {
      tempSensor_TMP117.setConversionAverageMode(settings.sensor_TMP117.conversionAverageMode);
      tempSensor_TMP117.setConversionCycleBit(settings.sensor_TMP117.conversionCycle);
      tempSensor_TMP117.setContinuousConversionMode();

      qwiicOnline.TMP117 = true;
      beginSensorOutput += "TMP117 Online\n";
    }
    else
      beginSensorOutput += "TMP117 failed to respond. Check wiring and address jumpers.\n";
  }

  if (qwiicAvailable.CCS811 && settings.sensor_CCS811.log && !qwiicOnline.CCS811)
  {
    if (vocSensor_CCS811.begin(qwiic) == true) //Wire port
    {
      qwiicOnline.CCS811 = true;
      beginSensorOutput += "CCS811 Online\n";
    }
    else
      beginSensorOutput += "CCS811 failed to respond. Check wiring and address jumpers. Adr must be 0x5B.\n";
  }

  if (qwiicAvailable.BME280 && settings.sensor_BME280.log && !qwiicOnline.BME280)
  {
    if (phtSensor_BME280.beginI2C(qwiic) == true) //Wire port
    {
      qwiicOnline.BME280 = true;
      beginSensorOutput += "BME280 Online\n";
    }
    else
      beginSensorOutput += "BME280 failed to respond. Check wiring and address jumpers. Adr must be 0x77.\n";
  }

  if (qwiicAvailable.SGP30 && settings.sensor_SGP30.log && !qwiicOnline.SGP30)
  {
    if (vocSensor_SGP30.begin(qwiic) == true) //Wire port
    {
      //Initializes sensor for air quality readings
      vocSensor_SGP30.initAirQuality();

      qwiicOnline.SGP30 = true;
      beginSensorOutput += "SGP30 Online\n";
    }
    else
      beginSensorOutput += "SGP30 failed to respond. Check wiring.\n";
  }

  if (qwiicAvailable.VEML6075 && settings.sensor_VEML6075.log && !qwiicOnline.VEML6075)
  {
    if (uvSensor_VEML6075.begin(qwiic) == true) //Wire port
    {
      qwiicOnline.VEML6075 = true;
      beginSensorOutput += "VEML6075 Online\n";
    }
    else
      beginSensorOutput += "VEML6075 failed to respond. Check wiring.\n";
  }

  if (qwiicAvailable.MS5637 && settings.sensor_MS5637.log && !qwiicOnline.MS5637)
  {
    if (pressureSensor_MS5637.begin(qwiic) == true) //Wire port
    {
      qwiicOnline.MS5637 = true;
      beginSensorOutput += "MS5637 Online\n";
    }
    else
      beginSensorOutput += "MS5637 failed to respond. Check wiring.\n";
  }

  if (qwiicAvailable.SCD30 && settings.sensor_SCD30.log && !qwiicOnline.SCD30)
  {
    if (co2Sensor_SCD30.begin(qwiic) == true) //Wire port
    {
      co2Sensor_SCD30.setMeasurementInterval(settings.sensor_SCD30.measurementInterval);
      co2Sensor_SCD30.setAltitudeCompensation(settings.sensor_SCD30.altitudeCompensation);
      co2Sensor_SCD30.setAmbientPressure(settings.sensor_SCD30.ambientPressure);
      //co2Sensor_SCD30.setTemperatureOffset(settings.sensor_SCD30.temperatureOffset);

      qwiicOnline.SCD30 = true;
      beginSensorOutput += "SCD30 Online\n";
    }
    else
      beginSensorOutput += "SCD30 failed to respond. Check wiring.\n";
  }

  if (qwiicAvailable.MS8607 && settings.sensor_MS8607.log && !qwiicOnline.MS8607)
  {
    if (pressureSensor_MS8607.begin(qwiic) == true) //Wire port. This checks both 0x40 and 0x76 sensor addresses
    {
      if (settings.sensor_MS8607.enableHeater == true)
        pressureSensor_MS8607.enable_heater();
      else
        pressureSensor_MS8607.disable_heater();

      pressureSensor_MS8607.set_pressure_resolution(settings.sensor_MS8607.pressureResolution);
      pressureSensor_MS8607.set_humidity_resolution(settings.sensor_MS8607.humidityResolution);

      qwiicOnline.MS8607 = true;
      beginSensorOutput += "MS8607 Online\n";
    }
    else
      beginSensorOutput += "MS8607 failed to respond. Check wiring.\n";
  }

  return true;
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
      outputData += (String)pressureSensor_LPS25HB.getPressure_hPa() + ",";
      helperText += "pressure_hPa,";
    }
    if (settings.sensor_LPS25HB.logTemp)
    {
      outputData += (String)pressureSensor_LPS25HB.getTemperature_degC() + ",";
      helperText += "pressure_degC,";
    }
  }

  if (qwiicOnline.NAU7802 && settings.sensor_NAU7802.log)
  {
    if (settings.sensor_NAU7802.log)
    {
      float currentWeight = loadcellSensor_NAU7802.getWeight(false, settings.sensor_NAU7802.averageAmount); //Do not allow negative weights, take average of X readings
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
      outputData += (String)thermoSensor_MCP9600.getThermocoupleTemp() + ",";
      helperText += "thermo_degC,";
    }
    if (settings.sensor_MCP9600.logAmbientTemp)
    {
      outputData += (String)thermoSensor_MCP9600.getAmbientTemp() + ",";
      helperText += "thermo_ambientDegC,";
    }
  }

  if (qwiicOnline.uBlox && settings.sensor_uBlox.log)
  {
    //Calling getPVT returns true if there actually is a fresh navigation solution available.
    //getPVT will block/wait up to 1000ms to receive new data. This will affect the global reading rate.
    //    if (gpsSensor_ublox.getPVT())
    gpsSensor_ublox.getPVT();
    //    {
    if (settings.sensor_uBlox.logDate)
    {
      char gpsDate[11]; //10/12/2019
      if (settings.americanDateStyle == true)
        sprintf(gpsDate, "%02d/%02d/%d", gpsSensor_ublox.getMonth(), gpsSensor_ublox.getDay(), gpsSensor_ublox.getYear());
      else
        sprintf(gpsDate, "%02d/%02d/%d", gpsSensor_ublox.getDay(), gpsSensor_ublox.getMonth(), gpsSensor_ublox.getYear());
      outputData += String(gpsDate) + ",";
      helperText += "gps_Date,";
    }
    if (settings.sensor_uBlox.logTime)
    {
      char gpsTime[12]; //09:14:37.41
      int adjustedHour = gpsSensor_ublox.getHour();
      if (settings.hour24Style == false)
      {
        if (adjustedHour > 12) adjustedHour -= 12;
      }
      sprintf(gpsTime, "%02d:%02d:%02d.%03d", adjustedHour, gpsSensor_ublox.getMinute(), gpsSensor_ublox.getSecond(), gpsSensor_ublox.getMillisecond());
      outputData += String(gpsTime) + ",";
      helperText += "gps_Time,";
    }
    if (settings.sensor_uBlox.logPosition)
    {
      outputData += (String)gpsSensor_ublox.getLatitude() + ",";
      outputData += (String)gpsSensor_ublox.getLongitude() + ",";
      helperText += "gps_Lat,gps_Long,";
    }
    if (settings.sensor_uBlox.logAltitude)
    {
      outputData += (String)gpsSensor_ublox.getAltitude() + ",";
      helperText += "gps_Alt,";
    }
    if (settings.sensor_uBlox.logAltitudeMSL)
    {
      outputData += (String)gpsSensor_ublox.getAltitudeMSL() + ",";
      helperText += "gps_AltMSL,";
    }
    if (settings.sensor_uBlox.logSIV)
    {
      outputData += (String)gpsSensor_ublox.getSIV() + ",";
      helperText += "gps_SIV,";
    }
    if (settings.sensor_uBlox.logFixType)
    {
      outputData += (String)gpsSensor_ublox.getFixType() + ",";
      helperText += "gps_FixType,";
    }
    if (settings.sensor_uBlox.logCarrierSolution)
    {
      outputData += (String)gpsSensor_ublox.getCarrierSolutionType() + ","; //0=No solution, 1=Float solution, 2=Fixed solution. Useful when querying module to see if it has high-precision RTK fix.
      helperText += "gps_CarrierSolution,";
    }
    if (settings.sensor_uBlox.logGroundSpeed)
    {
      outputData += (String)gpsSensor_ublox.getGroundSpeed() + ",";
      helperText += "gps_GroundSpeed,";
    }
    if (settings.sensor_uBlox.logHeadingOfMotion)
    {
      outputData += (String)gpsSensor_ublox.getHeading() + ",";
      helperText += "gps_Heading,";
    }
    if (settings.sensor_uBlox.logpDOP)
    {
      outputData += (String)gpsSensor_ublox.getPDOP() + ",";
      helperText += "gps_pDOP,";
    }
    if (settings.sensor_uBlox.logiTOW)
    {
      outputData += (String)gpsSensor_ublox.getTimeOfWeek() + ",";
      helperText += "gps_iTOW,";
    }

    gpsSensor_ublox.flushPVT(); //Mark all PVT data as used
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

  if (qwiicOnline.MS5637 && settings.sensor_MS5637.log)
  {
    if (settings.sensor_MS5637.logPressure)
    {
      outputData += (String)pressureSensor_MS5637.getPressure() + ",";
      helperText += "hPa,";
    }
    if (settings.sensor_MS5637.logTemp)
    {
      outputData += (String)pressureSensor_MS5637.getTemperature() + ",";
      helperText += "degC,";
    }
  }

  if (qwiicOnline.SCD30 && settings.sensor_SCD30.log)
  {
    if (settings.sensor_SCD30.logCO2)
    {
      outputData += (String)co2Sensor_SCD30.getCO2() + ",";
      helperText += "co2_ppm,";
    }
    if (settings.sensor_SCD30.logHumidity)
    {
      outputData += (String)co2Sensor_SCD30.getHumidity() + ",";
      helperText += "humidity_%,";
    }
    if (settings.sensor_SCD30.logTemperature)
    {
      outputData += (String)co2Sensor_SCD30.getTemperature() + ",";
      helperText += "degC,";
    }
  }

  if (qwiicOnline.MS8607 && settings.sensor_MS8607.log)
  {
    if (settings.sensor_MS8607.logPressure)
    {
      outputData += (String)pressureSensor_MS8607.getPressure() + ",";
      helperText += "hPa,";
    }
    if (settings.sensor_MS8607.logHumidity)
    {
      outputData += (String)pressureSensor_MS8607.getHumidity() + ",";
      helperText += "humidity_%,";
    }
    if (settings.sensor_MS8607.logPressure)
    {
      outputData += (String)pressureSensor_MS8607.getTemperature() + ",";
      helperText += "degC,";
    }
  }

  if (settings.logHertz)
  {
    uint64_t currentMillis;

    //If we are sleeping between readings then we cannot rely on millis() as it is powered down
    //Used RTC instead
    if (settings.usBetweenReadings >= maxUsBeforeSleep)
    {
      currentMillis = rtcMillis();
    }
    else
    {
      //Calculate the actual update rate based on the sketch start time and the
      //number of updates we've completed.
      currentMillis = millis();
    }

    float actualRate = measurementCount * 1000.0 / (currentMillis - measurementStartTime);
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

  //If user wants to limit the I2C bus speed, do it here
  if (maxSpeed > settings.qwiicBusMaxSpeed)
    maxSpeed = settings.qwiicBusMaxSpeed;

  qwiic.setClock(maxSpeed);
}
