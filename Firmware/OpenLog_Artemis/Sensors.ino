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

  //  if (settings.logVL53L1X)
  //  {
  //    if (distanceSensor.begin() == 0) //Begin returns 0 on a good init.
  //      qwiicOnline.VL53L1X = true;
  //    else
  //      msg("VL53L1X failed to respond. Check wiring.");
  //  }
  //
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
        outputData += String(rtcDate);
        outputData += ",";
        helperText += "rtcDate,";
      }

      char rtcTime[12]; //09:14:37.41
      int adjustedHour = myRTC.hour;
      if (settings.hour24Style == false)
      {
        if (adjustedHour > 12) adjustedHour -= 12;
      }
      sprintf(rtcTime, "%02d:%02d:%02d.%02d", adjustedHour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
      outputData += String(rtcTime);
      outputData += ",";
      helperText += "rtcTime,";
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
      outputData += pressureSensor.getPressure_hPa();
      outputData += ",";
      helperText += "pressure_hPa,";
    }
    if (settings.sensor_LPS25HB.logTemp)
    {
      outputData += pressureSensor.getTemperature_degC();
      outputData += ",";
      helperText += "pressure_degC,";
    }
  }

  if (qwiicOnline.NAU7802 && settings.sensor_NAU7802.log)
  {
    if (settings.sensor_NAU7802.log)
    {
      float currentWeight = nauScale.getWeight(false, settings.sensor_NAU7802.averageAmount); //Do not allow negative weights, take average of X readings
      static char weight[30];
      sprintf(weight, "%.*f", settings.sensor_NAU7802.decimalPlaces, currentWeight);

      outputData += weight;
      outputData += ",";
      helperText += "weight(no unit),";
    }
  }

  if (qwiicOnline.MCP9600 && settings.sensor_MCP9600.log)
  {
    if (settings.sensor_MCP9600.logTemp)
    {
      outputData += thermoSensor.getThermocoupleTemp();
      outputData += ",";
      helperText += "thermo_degC,";
    }
    if (settings.sensor_MCP9600.logAmbientTemp)
    {
      outputData += thermoSensor.getAmbientTemp();
      outputData += ",";
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
      outputData += String(gpsDate);
      outputData += ",";
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
      outputData += String(gpsTime);
      outputData += ",";
      helperText += "gps_Time,";
    }
    if (settings.sensor_uBlox.logPosition)
    {
      outputData += myGPS.getLatitude();
      outputData += ",";
      outputData += myGPS.getLongitude();
      outputData += ",";
      helperText += "gps_Lat,gps_Long,";
    }
    if (settings.sensor_uBlox.logAltitude)
    {
      outputData += myGPS.getAltitude();
      outputData += ",";
      helperText += "gps_Alt,";
    }
    if (settings.sensor_uBlox.logAltitudeMSL)
    {
      outputData += myGPS.getAltitudeMSL();
      outputData += ",";
    }
    if (settings.sensor_uBlox.logSIV)
    {
      outputData += myGPS.getSIV();
      outputData += ",";
      helperText += "gps_SIV,";
    }
    if (settings.sensor_uBlox.logFixType)
    {
      outputData += myGPS.getFixType();
      outputData += ",";
      helperText += "gps_FixType,";
    }
    if (settings.sensor_uBlox.logCarrierSolution)
    {
      outputData += myGPS.getCarrierSolutionType(); //0=No solution, 1=Float solution, 2=Fixed solution. Useful when querying module to see if it has high-precision RTK fix.
      outputData += ",";
      helperText += "gps_CarrierSolution,";
    }
    if (settings.sensor_uBlox.logGroundSpeed)
    {
      outputData += myGPS.getGroundSpeed();
      outputData += ",";
      helperText += "gps_GroundSpeed,";
    }
    if (settings.sensor_uBlox.logHeadingOfMotion)
    {
      outputData += myGPS.getHeading();
      outputData += ",";
      helperText += "gps_Heading,";
    }
    if (settings.sensor_uBlox.logpDOP)
    {
      outputData += myGPS.getPDOP();
      outputData += ",";
      helperText += "gps_pDOP,";
    }
    if (settings.sensor_uBlox.logiTOW)
    {
      outputData += myGPS.getTimeOfWeek();
      outputData += ",";
      helperText += "gps_iTOW,";
    }

    myGPS.flushPVT(); //Mark all PVT dat
    //    }
  }

  if (qwiicOnline.VCNL4040 && settings.sensor_VCNL4040.log)
  {
    if (settings.sensor_VCNL4040.logProximity)
    {
      outputData += proximitySensor_VCNL4040.getProximity();
      outputData += ",";
      helperText += "prox(no unit),";
    }
    if (settings.sensor_VCNL4040.logAmbientLight)
    {
      outputData += proximitySensor_VCNL4040.getAmbient();
      outputData += ",";
      helperText += "ambient_lux,";
    }
  }

  //  if (qwiicOnline.VL53L1X)
  //  {
  //    distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  //    int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  //    distanceSensor.stopRanging();
  //
  //    outputData += distance; //mm
  //    outputData += ",";
  //  }

  if (settings.logHertz)
  {
    //Calculate the actual update rate based on the sketch start time and the
    //number of updates we've completed.
    float actualRate = measurementCount * 1000.0 / (millis() - measurementStartTime);

    outputData += actualRate; //Hz
    outputData += ",";
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
  {
    maxSpeed = 100000;
  }

  qwiic.setClock(maxSpeed);
}

void qwiicPowerOn()
{
  digitalWrite(QWIIC_PWR, HIGH);
}
void qwiicPowerOff()
{
  digitalWrite(QWIIC_PWR, LOW);
}
