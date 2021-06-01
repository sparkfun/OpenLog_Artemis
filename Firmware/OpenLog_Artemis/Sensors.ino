//Query each enabled sensor for its most recent data
void getData()
{
  measurementCount++;
  measurementTotal++;

  char tempData[50];
  outputData[0] = '\0'; //Clear string contents

  if (settings.logRTC)
  {
    //Code written by @DennisMelamed in PR #70
    char timeString[37];
    getTimeString(timeString); // getTimeString is in timeStamp.ino
    strcat(outputData, timeString);
  }

  if (settings.logA11)
  {
    unsigned int analog11 = analogRead(11);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog11 * 2 / 16384.0;
      sprintf(tempData, "%.2f,", voltage);
    }
    else
      sprintf(tempData, "%d,", analog11);

    strcat(outputData, tempData);
  }

  if (settings.logA12)
  {
    unsigned int analog12 = analogRead(12);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog12 * 2 / 16384.0;
      sprintf(tempData, "%.2f,", voltage);
    }
    else
      sprintf(tempData, "%d,", analog12);

    strcat(outputData, tempData);
  }

  if (settings.logA13)
  {
    unsigned int analog13 = analogRead(13);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog13 * 2 / 16384.0;
      sprintf(tempData, "%.2f,", voltage);
    }
    else
      sprintf(tempData, "%d,", analog13);

    strcat(outputData, tempData);
  }

  if (settings.logA32)
  {
    unsigned int analog32 = analogRead(32);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog32 * 2 / 16384.0;
      sprintf(tempData, "%.2f,", voltage);
    }
    else
      sprintf(tempData, "%d,", analog32);

    strcat(outputData, tempData);
  }

  if (settings.logVIN)
  {
    float voltage = readVIN();
    sprintf(tempData, "%.2f,", voltage);
    strcat(outputData, tempData);
  }

  if (online.IMU)
  {
    //printDebug("getData: online.IMU = " + (String)online.IMU + "\r\n");

    if (settings.imuUseDMP == false)
    {
      if (myICM.dataReady())
      {
        //printDebug("getData: myICM.dataReady = " + (String)myICM.dataReady() + "\r\n");
        
        myICM.getAGMT(); //Update values
  
        if (settings.logIMUAccel)
        {
          sprintf(tempData, "%.2f,%.2f,%.2f,", myICM.accX(), myICM.accY(), myICM.accZ());
          strcat(outputData, tempData);
        }
        if (settings.logIMUGyro)
        {
          sprintf(tempData, "%.2f,%.2f,%.2f,", myICM.gyrX(), myICM.gyrY(), myICM.gyrZ());
          strcat(outputData, tempData);
        }
        if (settings.logIMUMag)
        {
          sprintf(tempData, "%.2f,%.2f,%.2f,", myICM.magX(), myICM.magY(), myICM.magZ());
          strcat(outputData, tempData);
        }
        if (settings.logIMUTemp)
        {
          sprintf(tempData, "%.2f,", myICM.temp());
          strcat(outputData, tempData);
        }
      }
      //else
      //{
      //  printDebug("getData: myICM.dataReady = " + (String)myICM.dataReady() + "\r\n");
      //}
    }
    else
    {
      myICM.readDMPdataFromFIFO(&dmpData);
      while (myICM.status == ICM_20948_Stat_FIFOMoreDataAvail)
      {
        myICM.readDMPdataFromFIFO(&dmpData); // Empty the FIFO - make sure data contains the most recent data
      }
      if (settings.imuLogDMPQuat6)
      {
        sprintf(tempData, "%.3f,%.3f,%.3f,", ((double)dmpData.Quat6.Data.Q1) / 1073741824.0,
          ((double)dmpData.Quat6.Data.Q2) / 1073741824.0, ((double)dmpData.Quat6.Data.Q3) / 1073741824.0);
        strcat(outputData, tempData);
      }
      if (settings.imuLogDMPQuat9)
      {
        sprintf(tempData, "%.3f,%.3f,%.3f,%d,", ((double)dmpData.Quat9.Data.Q1) / 1073741824.0,
          ((double)dmpData.Quat9.Data.Q2) / 1073741824.0, ((double)dmpData.Quat9.Data.Q3) / 1073741824.0, dmpData.Quat9.Data.Accuracy);
        strcat(outputData, tempData);
      }
      if (settings.imuLogDMPAccel)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Raw_Accel.Data.X, dmpData.Raw_Accel.Data.Y, dmpData.Raw_Accel.Data.Z);
        strcat(outputData, tempData);
      }
      if (settings.imuLogDMPGyro)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Raw_Gyro.Data.X, dmpData.Raw_Gyro.Data.Y, dmpData.Raw_Gyro.Data.Z);
        strcat(outputData, tempData);
      }
      if (settings.imuLogDMPCpass)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Compass.Data.X, dmpData.Compass.Data.Y, dmpData.Compass.Data.Z);
        strcat(outputData, tempData);
      }
    }
  }

  //Append all external sensor data on linked list to outputData
  gatherDeviceValues();

  if (settings.logHertz)
  {
    uint64_t currentMillis;

    //If we are sleeping between readings then we cannot rely on millis() as it is powered down
    //Use RTC instead
    if (((settings.useGPIO11ForTrigger == false) && (settings.usBetweenReadings >= maxUsBeforeSleep))
    || (settings.useGPIO11ForFastSlowLogging == true)
    || (settings.useRTCForFastSlowLogging == true))
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
    sprintf(tempData, "%.02f,", actualRate); //Hz
    strcat(outputData, tempData);
  }

  if (settings.printMeasurementCount)
  {
    sprintf(tempData, "%d,", measurementTotal);
    strcat(outputData, tempData);
  }

  strcat(outputData, "\r\n");

  totalCharactersPrinted += strlen(outputData);
}

//Read values from the devices on the node list
//Append values to outputData
void gatherDeviceValues()
{
  char tempData[100];

  //Step through list, printing values as we go
  node *temp = head;
  while (temp != NULL)
  {
    //If this node successfully begin()'d
    if (temp->online == true)
    {
      openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

      //Switch on device type to set proper class and setting struct
      switch (temp->deviceType)
      {
        case DEVICE_MULTIPLEXER:
          {
            //No data to print for a mux
          }
          break;
        case DEVICE_LOADCELL_NAU7802:
          {
            NAU7802 *nodeDevice = (NAU7802 *)temp->classPtr;
            struct_NAU7802 *nodeSetting = (struct_NAU7802 *)temp->configPtr; //Create a local pointer that points to same spot as node does

            if (nodeSetting->log == true)
            {
              float currentWeight = nodeDevice->getWeight(false, nodeSetting->averageAmount); //Do not allow negative weights, take average of X readings
              sprintf(tempData, "%.*f,", nodeSetting->decimalPlaces, currentWeight);
              strcat(outputData, tempData);
            }
          }
          break;
        case DEVICE_DISTANCE_VL53L1X:
          {
            SFEVL53L1X *nodeDevice = (SFEVL53L1X *)temp->classPtr;
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr;

            if (nodeSetting->log == true)
            {
              if (nodeSetting->logDistance)
              {
                sprintf(tempData, "%d,", nodeDevice->getDistance());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logRangeStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getRangeStatus());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logSignalRate)
              {
                sprintf(tempData, "%d,", nodeDevice->getSignalRate());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_GPS_UBLOX:
          {
            qwiic.setPullups(0); //Disable pullups to minimize CRC issues

            SFE_UBLOX_GNSS *nodeDevice = (SFE_UBLOX_GNSS *)temp->classPtr;
            struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr;

            if (nodeSetting->log == true)
            {
              if (nodeSetting->logDate)
              {
                if (settings.americanDateStyle == true)
                  sprintf(tempData, "%02d/%02d/%d,", nodeDevice->getMonth(), nodeDevice->getDay(), nodeDevice->getYear());
                else
                  sprintf(tempData, "%02d/%02d/%d,", nodeDevice->getDay(), nodeDevice->getMonth(), nodeDevice->getYear());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTime)
              {
                int adjustedHour = nodeDevice->getHour();
                if (settings.hour24Style == false)
                  if (adjustedHour > 12) adjustedHour -= 12;
                sprintf(tempData, "%02d:%02d:%02d.%03d,", adjustedHour, nodeDevice->getMinute(), nodeDevice->getSecond(), nodeDevice->getMillisecond());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPosition)
              {
                sprintf(tempData, "%d,%d,", nodeDevice->getLatitude(), nodeDevice->getLongitude());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logAltitude)
              {
                sprintf(tempData, "%d,", nodeDevice->getAltitude());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logAltitudeMSL)
              {
                sprintf(tempData, "%d,", nodeDevice->getAltitudeMSL());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logSIV)
              {
                sprintf(tempData, "%d,", nodeDevice->getSIV());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logFixType)
              {
                sprintf(tempData, "%d,", nodeDevice->getFixType());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logCarrierSolution)
              {
                sprintf(tempData, "%d,", nodeDevice->getCarrierSolutionType());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logGroundSpeed)
              {
                sprintf(tempData, "%d,", nodeDevice->getGroundSpeed());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logHeadingOfMotion)
              {
                sprintf(tempData, "%d,", nodeDevice->getHeading());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logpDOP)
              {
                sprintf(tempData, "%d,", nodeDevice->getPDOP());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logiTOW)
              {
                sprintf(tempData, "%d,", nodeDevice->getTimeOfWeek());
                strcat(outputData, tempData);
              }
            }

            qwiic.setPullups(settings.qwiicBusPullUps); //Re-enable pullups
          }
          break;
        case DEVICE_PROXIMITY_VCNL4040:
          {
            VCNL4040 *nodeDevice = (VCNL4040 *)temp->classPtr;
            struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)temp->configPtr;

            //Get ambient takes 80ms minimum and may not play properly with power cycling
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logProximity)
              {
                sprintf(tempData, "%d,", nodeDevice->getProximity());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logAmbientLight)
              {
                sprintf(tempData, "%d,", nodeDevice->getAmbient());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_TEMPERATURE_TMP117:
          {
            TMP117 *nodeDevice = (TMP117 *)temp->classPtr;
            struct_TMP117 *nodeSetting = (struct_TMP117 *)temp->configPtr;

            if (nodeSetting->log == true)
            {
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.04f,", nodeDevice->readTempC()); //Resolution to 0.0078°C, accuracy of 0.1°C
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PRESSURE_MS5637:
          {
            MS5637 *nodeDevice = (MS5637 *)temp->classPtr;
            struct_MS5637 *nodeSetting = (struct_MS5637 *)temp->configPtr;

            if (nodeSetting->log == true)
            {
              if (nodeSetting->logPressure)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getPressure());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getTemperature());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PRESSURE_LPS25HB:
          {
            LPS25HB *nodeDevice = (LPS25HB *)temp->classPtr;
            struct_LPS25HB *nodeSetting = (struct_LPS25HB *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logPressure)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getPressure_hPa());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getTemperature_degC());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PHT_BME280:
          {
            BME280 *nodeDevice = (BME280 *)temp->classPtr;
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logPressure)
              {
                sprintf(tempData, "%.02f,", nodeDevice->readFloatPressure());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logHumidity)
              {
                sprintf(tempData, "%.02f,", nodeDevice->readFloatHumidity());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logAltitude)
              {
                sprintf(tempData, "%.02f,", nodeDevice->readFloatAltitudeMeters());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->readTempC());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_UV_VEML6075:
          {
            VEML6075 *nodeDevice = (VEML6075 *)temp->classPtr;
            struct_VEML6075 *nodeSetting = (struct_VEML6075 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logUVA)
              {
                sprintf(tempData, "%.02f,", nodeDevice->uva());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logUVB)
              {
                sprintf(tempData, "%.02f,", nodeDevice->uvb());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logUVIndex)
              {
                sprintf(tempData, "%.02f,", nodeDevice->index());
                strcat(outputData, tempData);
              }
            }
          }
          break;

        case DEVICE_VOC_CCS811:
          {
            CCS811 *nodeDevice = (CCS811 *)temp->classPtr;
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              nodeDevice->readAlgorithmResults();
              if (nodeSetting->logTVOC)
              {
                sprintf(tempData, "%d,", nodeDevice->getTVOC());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logCO2)
              {
                sprintf(tempData, "%d,", nodeDevice->getCO2());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_VOC_SGP30:
          {
            SGP30 *nodeDevice = (SGP30 *)temp->classPtr;
            struct_SGP30 *nodeSetting = (struct_SGP30 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              nodeDevice->measureAirQuality();
              nodeDevice->measureRawSignals(); //To get H2 and Ethanol

              if (nodeSetting->logTVOC)
              {
                sprintf(tempData, "%d,", nodeDevice->TVOC);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logCO2)
              {
                sprintf(tempData, "%d,", nodeDevice->CO2);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logH2)
              {
                sprintf(tempData, "%d,", nodeDevice->H2);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logEthanol)
              {
                sprintf(tempData, "%d,", nodeDevice->ethanol);
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_CO2_SCD30:
          {
            SCD30 *nodeDevice = (SCD30 *)temp->classPtr;
            struct_SCD30 *nodeSetting = (struct_SCD30 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logCO2)
              {
                sprintf(tempData, "%d,", nodeDevice->getCO2());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logHumidity)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getHumidity());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getTemperature());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PHT_MS8607:
          {
            MS8607 *nodeDevice = (MS8607 *)temp->classPtr;
            struct_MS8607 *nodeSetting = (struct_MS8607 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logHumidity)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getHumidity());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPressure)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getPressure());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getTemperature());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_TEMPERATURE_MCP9600:
          {
            MCP9600 *nodeDevice = (MCP9600 *)temp->classPtr;
            struct_MCP9600 *nodeSetting = (struct_MCP9600 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getThermocoupleTemp());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logAmbientTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getAmbientTemp());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_HUMIDITY_AHT20:
          {
            AHT20 *nodeDevice = (AHT20 *)temp->classPtr;
            struct_AHT20 *nodeSetting = (struct_AHT20 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logHumidity)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getHumidity());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getTemperature());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_HUMIDITY_SHTC3:
          {
            SHTC3 *nodeDevice = (SHTC3 *)temp->classPtr;
            struct_SHTC3 *nodeSetting = (struct_SHTC3 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              nodeDevice->update();
              if (nodeSetting->logHumidity)
              {
                sprintf(tempData, "%.02f,", nodeDevice->toPercent());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->toDegC());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_ADC_ADS122C04:
          {
            SFE_ADS122C04 *nodeDevice = (SFE_ADS122C04 *)temp->classPtr;
            struct_ADS122C04 *nodeSetting = (struct_ADS122C04 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              // The ADS122C04 supports sampling up to 2kHz but the library functions default to 20Hz.
              // To be able to log faster than 20Hz we need to use setDataRate to change the data rate (sample speed).
              // Note: readInternalTemperature and readRawVoltage are hard wired to 20Hz in the library and
              //       - at the moment - there's nothing we can do about that! If you want to log faster than
              //       20Hz, you'll need to disable readInternalTemperature and readRawVoltage.
              // At the time of writing, the maximum achieveable sample rate is ~156Hz.

              //It looks like configureDevice will take care of this. No need to do it here.
              //if (nodeSetting->useFourWireMode)
              //  nodeDevice->configureADCmode(ADS122C04_4WIRE_MODE);
              //else if (nodeSetting->useThreeWireMode)
              //  nodeDevice->configureADCmode(ADS122C04_3WIRE_MODE);
              //else if (nodeSetting->useTwoWireMode)
              //  nodeDevice->configureADCmode(ADS122C04_2WIRE_MODE);
              //else if (nodeSetting->useFourWireHighTemperatureMode)
              //  nodeDevice->configureADCmode(ADS122C04_4WIRE_HI_TEMP);
              //else if (nodeSetting->useThreeWireHighTemperatureMode)
              //  nodeDevice->configureADCmode(ADS122C04_3WIRE_HI_TEMP);
              //else if (nodeSetting->useTwoWireHighTemperatureMode)
              //  nodeDevice->configureADCmode(ADS122C04_2WIRE_HI_TEMP);

              if (settings.usBetweenReadings < 50000ULL) // Check if we are trying to sample quicker than 20Hz
              {
                if (settings.usBetweenReadings <= 1000ULL) // Check if we are trying to sample at 1kHz
                  nodeDevice->setDataRate(ADS122C04_DATA_RATE_1000SPS);
                else if (settings.usBetweenReadings <= 1667ULL) // Check if we are trying to sample at 600Hz
                  nodeDevice->setDataRate(ADS122C04_DATA_RATE_600SPS);
                else if (settings.usBetweenReadings <= 3031ULL) // Check if we are trying to sample at 330Hz
                  nodeDevice->setDataRate(ADS122C04_DATA_RATE_330SPS);
                else if (settings.usBetweenReadings <= 5715ULL) // Check if we are trying to sample at 175Hz
                  nodeDevice->setDataRate(ADS122C04_DATA_RATE_175SPS);
                else if (settings.usBetweenReadings <= 11112ULL) // Check if we are trying to sample at 90Hz
                  nodeDevice->setDataRate(ADS122C04_DATA_RATE_90SPS);
                else if (settings.usBetweenReadings <= 22223ULL) // Check if we are trying to sample at 45Hz
                  nodeDevice->setDataRate(ADS122C04_DATA_RATE_45SPS);
              }
              else
                nodeDevice->setDataRate(ADS122C04_DATA_RATE_20SPS); // Default to 20Hz
              
              if (nodeSetting->logCentigrade)
              {
                sprintf(tempData, "%.03f,", nodeDevice->readPT100Centigrade());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logFahrenheit)
              {
                sprintf(tempData, "%.03f,", nodeDevice->readPT100Fahrenheit());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logInternalTemperature)
              {
                sprintf(tempData, "%.03f,", nodeDevice->readInternalTemperature());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logRawVoltage)
              {
                sprintf(tempData, "%d,", nodeDevice->readRawVoltage());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PRESSURE_MPR0025PA1:
          {
            SparkFun_MicroPressure *nodeDevice = (SparkFun_MicroPressure *)temp->classPtr;
            struct_MPR0025PA1 *nodeSetting = (struct_MPR0025PA1 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->usePSI)
              {
                sprintf(tempData, "%.04f,", nodeDevice->readPressure());
                strcat(outputData, tempData);
              }
              if (nodeSetting->usePA)
              {
                sprintf(tempData, "%.01f,", nodeDevice->readPressure(PA));
                strcat(outputData, tempData);
              }
              if (nodeSetting->useKPA)
              {
                sprintf(tempData, "%.04f,", nodeDevice->readPressure(KPA));
                strcat(outputData, tempData);
              }
              if (nodeSetting->useTORR)
              {
                sprintf(tempData, "%.03f,", nodeDevice->readPressure(TORR));
                strcat(outputData, tempData);
              }
              if (nodeSetting->useINHG)
              {
                sprintf(tempData, "%.04f,", nodeDevice->readPressure(INHG));
                strcat(outputData, tempData);
              }
              if (nodeSetting->useATM)
              {
                sprintf(tempData, "%.06f,", nodeDevice->readPressure(ATM));
                strcat(outputData, tempData);
              }
              if (nodeSetting->useBAR)
              {
                sprintf(tempData, "%.06f,", nodeDevice->readPressure(BAR));
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PARTICLE_SNGCJA5:
          {
            SFE_PARTICLE_SENSOR *nodeDevice = (SFE_PARTICLE_SENSOR *)temp->classPtr;
            struct_SNGCJA5 *nodeSetting = (struct_SNGCJA5 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logPM1)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getPM1_0());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPM25)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getPM2_5());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPM10)
              {
                sprintf(tempData, "%.02f,", nodeDevice->getPM10());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPC05)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC0_5());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPC1)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC1_0());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPC25)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC2_5());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPC50)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC5_0());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPC75)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC7_5());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPC10)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC10());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logSensorStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusSensors());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logPDStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusPD());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logLDStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusLD());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logFanStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusFan());
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_VOC_SGP40:
          {
            SGP40 *nodeDevice = (SGP40 *)temp->classPtr;
            struct_SGP40 *nodeSetting = (struct_SGP40 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logVOC)
              {
                sprintf(tempData, "%d,", nodeDevice->getVOCindex(nodeSetting->RH, nodeSetting->T));
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PRESSURE_SDP3X:
          {
            SDP3X *nodeDevice = (SDP3X *)temp->classPtr;
            struct_SDP3X *nodeSetting = (struct_SDP3X *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              float pressure;
              float temperature;
              if ((nodeSetting->logPressure) || (nodeSetting->logTemperature))
              {
                // Each triggered measurement takes 45ms to complete so we need to use continuous measurements
                nodeDevice->readMeasurement(&pressure, &temperature); // Read the latest measurement
              }
              if (nodeSetting->logPressure)
              {
                sprintf(tempData, "%.02f,", pressure);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", temperature);
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_PRESSURE_MS5837:
          {
            MS5837 *nodeDevice = (MS5837 *)temp->classPtr;
            struct_MS5837 *nodeSetting = (struct_MS5837 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if ((nodeSetting->logPressure) || (nodeSetting->logTemperature) || (nodeSetting->logDepth) || (nodeSetting->logAltitude))
              {
                nodeDevice->read();
              }
              if (nodeSetting->logPressure)
              {
                sprintf(tempData, "%.02f,", nodeDevice->pressure(nodeSetting->conversion));
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                sprintf(tempData, "%.02f,", nodeDevice->temperature());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logDepth)
              {
                sprintf(tempData, "%.03f,", nodeDevice->depth());
                strcat(outputData, tempData);
              }
              if (nodeSetting->logAltitude)
              {
                sprintf(tempData, "%.02f,", nodeDevice->altitude());
                strcat(outputData, tempData);
              }
            }
          }
          break;
//        case DEVICE_QWIIC_BUTTON:
//          {
//            QwiicButton *nodeDevice = (QwiicButton *)temp->classPtr;
//            struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)temp->configPtr;
//            if (nodeSetting->log == true)
//            {
//              long pressedPopped = 0;
//              while (nodeDevice->isPressedQueueEmpty() == false)
//              {
//                pressedPopped = nodeDevice->popPressedQueue();
//              }
//              if (nodeSetting->logPressed)
//              {
//                sprintf(tempData, "%.03f,", ((float)pressedPopped) / 1000.0); // Record only the most recent press - that's the best we can do
//                strcat(outputData, tempData);
//              }
//              
//              long clickedPopped = 0;
//              while (nodeDevice->isClickedQueueEmpty() == false)
//              {
//                clickedPopped = nodeDevice->popClickedQueue();
//                nodeSetting->ledState ^= 1; // Toggle nodeSetting->ledState on _every_ click (not just the most recent)
//              }
//              if (nodeSetting->logClicked)
//              {
//                sprintf(tempData, "%.03f,", ((float)clickedPopped) / 1000.0); // Record only the most recent click - that's the best we can do
//                strcat(outputData, tempData);
//              }
//              
//              if (nodeSetting->toggleLEDOnClick)
//              {
//                if (nodeSetting->ledState)
//                  nodeDevice->LEDon(nodeSetting->ledBrightness);
//                else
//                  nodeDevice->LEDoff();
//                sprintf(tempData, "%d,", nodeSetting->ledState);
//                strcat(outputData, tempData);
//              }
//            }
//          }
//          break;
        case DEVICE_BIO_SENSOR_HUB:
          {
            SparkFun_Bio_Sensor_Hub *nodeDevice = (SparkFun_Bio_Sensor_Hub *)temp->classPtr;
            struct_BIO_SENSOR_HUB *nodeSetting = (struct_BIO_SENSOR_HUB *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              bioData body;
              if ((nodeSetting->logHeartrate) || (nodeSetting->logConfidence) || (nodeSetting->logOxygen) || (nodeSetting->logStatus) || (nodeSetting->logExtendedStatus) || (nodeSetting->logRValue))
              {
                body = nodeDevice->readBpm();
              }
              if (nodeSetting->logHeartrate)
              {
                sprintf(tempData, "%d,", body.heartRate);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logConfidence)
              {
                sprintf(tempData, "%d,", body.confidence);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logOxygen)
              {
                sprintf(tempData, "%d,", body.oxygen);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logStatus)
              {
                sprintf(tempData, "%d,", body.status);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logExtendedStatus)
              {
                sprintf(tempData, "%d,", body.extStatus);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logRValue)
              {
                sprintf(tempData, "%.01f,", body.rValue);
                strcat(outputData, tempData);
              }
            }
          }
          break;
        default:
          SerialPrintf2("printDeviceValue unknown device type: %s\r\n", getDeviceName(temp->deviceType));
          break;
      }

    }
    temp = temp->next;
  }
}

//Step through the node list and print helper text for the enabled readings
void printHelperText(bool terminalOnly)
{
  char helperText[1000];
  helperText[0] = '\0';

  if (settings.logRTC)
  {
    if (settings.logDate)
      strcat(helperText, "rtcDate,");
    if (settings.logTime)
      strcat(helperText, "rtcTime,");
    if (settings.logMicroseconds)
      strcat(helperText, "micros,");
  }

  if (settings.logA11)
    strcat(helperText, "analog_11,");

  if (settings.logA12)
    strcat(helperText, "analog_12,");

  if (settings.logA13)
    strcat(helperText, "analog_13,");

  if (settings.logA32)
    strcat(helperText, "analog_32,");

  if (settings.logVIN)
    strcat(helperText, "VIN,");

  if (online.IMU)
  {
    if (settings.imuUseDMP == false)
    {
      if (settings.logIMUAccel)
        strcat(helperText, "aX,aY,aZ,");
      if (settings.logIMUGyro)
        strcat(helperText, "gX,gY,gZ,");
      if (settings.logIMUMag)
        strcat(helperText, "mX,mY,mZ,");
      if (settings.logIMUTemp)
        strcat(helperText, "imu_degC,");
    }
    else
    {
      if (settings.imuLogDMPQuat6)
        strcat(helperText, "Q6_1,Q6_2,Q6_3,");
      if (settings.imuLogDMPQuat9)
        strcat(helperText, "Q9_1,Q9_2,Q9_3,HeadAcc,");
      if (settings.imuLogDMPAccel)
        strcat(helperText, "RawAX,RawAY,RawAZ,");
      if (settings.imuLogDMPGyro)
        strcat(helperText, "RawGX,RawGY,RawGZ,");
      if (settings.imuLogDMPCpass)
        strcat(helperText, "RawMX,RawMY,RawMZ,");
    }
  }

  //Step through list, printing values as we go
  node *temp = head;
  while (temp != NULL)
  {

    //If this node successfully begin()'d
    if (temp->online == true)
    {
      //Switch on device type to set proper class and setting struct
      switch (temp->deviceType)
      {
        case DEVICE_MULTIPLEXER:
          {
            //No data to print for a mux
          }
          break;
        case DEVICE_LOADCELL_NAU7802:
          {
            struct_NAU7802 *nodeSetting = (struct_NAU7802 *)temp->configPtr;
            if (nodeSetting->log)
              strcat(helperText, "weight(no unit),");
          }
          break;
        case DEVICE_DISTANCE_VL53L1X:
          {
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logDistance)
                strcat(helperText, "distance_mm,");
              if (nodeSetting->logRangeStatus)
                strcat(helperText, "distance_rangeStatus(0=good),");
              if (nodeSetting->logSignalRate)
                strcat(helperText, "distance_signalRate,");
            }
          }
          break;
        case DEVICE_GPS_UBLOX:
          {
            struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logDate)
                strcat(helperText, "gps_Date,");
              if (nodeSetting->logTime)
                strcat(helperText, "gps_Time,");
              if (nodeSetting->logPosition)
                strcat(helperText, "gps_Lat,gps_Long,");
              if (nodeSetting->logAltitude)
                strcat(helperText, "gps_Alt,");
              if (nodeSetting->logAltitudeMSL)
                strcat(helperText, "gps_AltMSL,");
              if (nodeSetting->logSIV)
                strcat(helperText, "gps_SIV,");
              if (nodeSetting->logFixType)
                strcat(helperText, "gps_FixType,");
              if (nodeSetting->logCarrierSolution)
                strcat(helperText, "gps_CarrierSolution,");
              if (nodeSetting->logGroundSpeed)
                strcat(helperText, "gps_GroundSpeed,");
              if (nodeSetting->logHeadingOfMotion)
                strcat(helperText, "gps_Heading,");
              if (nodeSetting->logpDOP)
                strcat(helperText, "gps_pDOP,");
              if (nodeSetting->logiTOW)
                strcat(helperText, "gps_iTOW,");
            }
          }
          break;
        case DEVICE_PROXIMITY_VCNL4040:
          {
            struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logProximity)
                strcat(helperText, "prox(no unit),");
              if (nodeSetting->logAmbientLight)
                strcat(helperText, "ambient_lux,");
            }
          }
          break;
        case DEVICE_TEMPERATURE_TMP117:
          {
            struct_TMP117 *nodeSetting = (struct_TMP117 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTemperature)
                strcat(helperText, "degC,");
            }
          }
          break;
        case DEVICE_PRESSURE_MS5637:
          {
            struct_MS5637 *nodeSetting = (struct_MS5637 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strcat(helperText, "pressure_hPa,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "temperature_degC,");
            }
          }
          break;
        case DEVICE_PRESSURE_LPS25HB:
          {
            struct_LPS25HB *nodeSetting = (struct_LPS25HB *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strcat(helperText, "pressure_hPa,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "pressure_degC,");
            }
          }
          break;
        case DEVICE_PHT_BME280:
          {
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strcat(helperText, "pressure_Pa,");
              if (nodeSetting->logHumidity)
                strcat(helperText, "humidity_%,");
              if (nodeSetting->logAltitude)
                strcat(helperText, "altitude_m,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "temp_degC,");
            }
          }
          break;
        case DEVICE_UV_VEML6075:
          {
            struct_VEML6075 *nodeSetting = (struct_VEML6075 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logUVA)
                strcat(helperText, "uva,");
              if (nodeSetting->logUVB)
                strcat(helperText, "uvb,");
              if (nodeSetting->logUVIndex)
                strcat(helperText, "uvIndex,");
            }
          }
          break;
        case DEVICE_VOC_CCS811:
          {
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTVOC)
                strcat(helperText, "tvoc_ppb,");
              if (nodeSetting->logCO2)
                strcat(helperText, "co2_ppm,");
            }
          }
          break;
        case DEVICE_VOC_SGP30:
          {
            struct_SGP30 *nodeSetting = (struct_SGP30 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTVOC)
                strcat(helperText, "tvoc_ppb,");
              if (nodeSetting->logCO2)
                strcat(helperText, "co2_ppm,");
              if (nodeSetting->logH2)
                strcat(helperText, "H2,");
              if (nodeSetting->logEthanol)
                strcat(helperText, "ethanol,");
            }
          }
          break;
        case DEVICE_CO2_SCD30:
          {
            struct_SCD30 *nodeSetting = (struct_SCD30 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logCO2)
                strcat(helperText, "co2_ppm,");
              if (nodeSetting->logHumidity)
                strcat(helperText, "humidity_%,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "degC,");
            }
          }
          break;
        case DEVICE_PHT_MS8607:
          {
            struct_MS8607 *nodeSetting = (struct_MS8607 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
                strcat(helperText, "humidity_%,");
              if (nodeSetting->logPressure)
                strcat(helperText, "hPa,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "degC,");
            }
          }
          break;
        case DEVICE_TEMPERATURE_MCP9600:
          {
            struct_MCP9600 *nodeSetting = (struct_MCP9600 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTemperature)
                strcat(helperText, "thermo_degC,");
              if (nodeSetting->logAmbientTemperature)
                strcat(helperText, "thermo_ambientDegC,");
            }
          }
          break;
        case DEVICE_HUMIDITY_AHT20:
          {
            struct_AHT20 *nodeSetting = (struct_AHT20 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
                strcat(helperText, "humidity_%,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "degC,");
            }
          }
          break;
        case DEVICE_HUMIDITY_SHTC3:
          {
            struct_SHTC3 *nodeSetting = (struct_SHTC3 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
                strcat(helperText, "humidity_%,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "degC,");
            }
          }
          break;
        case DEVICE_ADC_ADS122C04:
          {
            struct_ADS122C04 *nodeSetting = (struct_ADS122C04 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logCentigrade)
                strcat(helperText, "degC,");
              if (nodeSetting->logFahrenheit)
                strcat(helperText, "degF,");
              if (nodeSetting->logInternalTemperature)
                strcat(helperText, "degC,");
              if (nodeSetting->logRawVoltage)
                strcat(helperText, "V*2.048/2^23,");
            }
          }
          break;
        case DEVICE_PRESSURE_MPR0025PA1:
          {
            struct_MPR0025PA1 *nodeSetting = (struct_MPR0025PA1 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->usePSI)
                strcat(helperText, "PSI,");
              if (nodeSetting->usePA)
                strcat(helperText, "Pa,");
              if (nodeSetting->useKPA)
                strcat(helperText, "kPa,");
              if (nodeSetting->useTORR)
                strcat(helperText, "torr,");
              if (nodeSetting->useINHG)
                strcat(helperText, "inHg,");
              if (nodeSetting->useATM)
                strcat(helperText, "atm,");
              if (nodeSetting->useBAR)
                strcat(helperText, "bar,");
            }
          }
          break;
        case DEVICE_PARTICLE_SNGCJA5:
          {
            struct_SNGCJA5 *nodeSetting = (struct_SNGCJA5 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPM1)
                strcat(helperText, "PM1_0,");
              if (nodeSetting->logPM25)
                strcat(helperText, "PM2_5,");
              if (nodeSetting->logPM10)
                strcat(helperText, "PM10,");
              if (nodeSetting->logPC05)
                strcat(helperText, "PC0_5,");
              if (nodeSetting->logPC1)
                strcat(helperText, "PC1_0,");
              if (nodeSetting->logPC25)
                strcat(helperText, "PC2_5,");
              if (nodeSetting->logPC50)
                strcat(helperText, "PC5_0,");
              if (nodeSetting->logPC75)
                strcat(helperText, "PC7_5,");
              if (nodeSetting->logPC10)
                strcat(helperText, "PC10,");
              if (nodeSetting->logSensorStatus)
                strcat(helperText, "Sensors,");
              if (nodeSetting->logPDStatus)
                strcat(helperText, "PD,");
              if (nodeSetting->logLDStatus)
                strcat(helperText, "LD,");
              if (nodeSetting->logFanStatus)
                strcat(helperText, "Fan,");
            }
          }
          break;
        case DEVICE_VOC_SGP40:
          {
            struct_SGP40 *nodeSetting = (struct_SGP40 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logVOC)
                strcat(helperText, "VOCindex,");
            }
          }
          break;
        case DEVICE_PRESSURE_SDP3X:
          {
            struct_SDP3X *nodeSetting = (struct_SDP3X *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strcat(helperText, "Pa,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "degC,");
            }
          }
          break;
        case DEVICE_PRESSURE_MS5837:
          {
            struct_MS5837 *nodeSetting = (struct_MS5837 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strcat(helperText, "mbar,");
              if (nodeSetting->logTemperature)
                strcat(helperText, "degC,");
              if (nodeSetting->logDepth)
                strcat(helperText, "depth_m,");
              if (nodeSetting->logAltitude)
                strcat(helperText, "alt_m,");
            }
          }
          break;
//        case DEVICE_QWIIC_BUTTON:
//          {
//            struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)temp->configPtr;
//            if (nodeSetting->log)
//            {
//              if (nodeSetting->logPressed)
//                strcat(helperText, "pressS,");
//              if (nodeSetting->logClicked)
//                strcat(helperText, "clickS,");
//              if (nodeSetting->toggleLEDOnClick)
//                strcat(helperText, "LED,");
//            }
//          }
//          break;
        case DEVICE_BIO_SENSOR_HUB:
          {
            struct_BIO_SENSOR_HUB *nodeSetting = (struct_BIO_SENSOR_HUB *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHeartrate)
                strcat(helperText, "bpm,");
              if (nodeSetting->logConfidence)
                strcat(helperText, "conf%,");
              if (nodeSetting->logOxygen)
                strcat(helperText, "O2%,");
              if (nodeSetting->logStatus)
                strcat(helperText, "stat,");
              if (nodeSetting->logExtendedStatus)
                strcat(helperText, "eStat,");
              if (nodeSetting->logRValue)
                strcat(helperText, "O2R,");
            }
          }
          break;
        default:
          SerialPrintf2("\nprinterHelperText device not found: %d\r\n", temp->deviceType);
          break;
      }
    }
    temp = temp->next;
  }

  if (settings.logHertz)
    strcat(helperText, "output_Hz,");

  if (settings.printMeasurementCount)
    strcat(helperText, "count,");

  strcat(helperText, "\r\n");

  SerialPrint(helperText);
  if ((terminalOnly == false) && (settings.logData == true) && (online.microSD) && (settings.enableSD && online.microSD))
    sensorDataFile.print(helperText);
}

//If certain devices are attached, we need to reduce the I2C max speed
void setMaxI2CSpeed()
{
  uint32_t maxSpeed = 400000; //Assume 400kHz - but beware! 400kHz with no pull-ups can cause u-blox issues.

  //Search nodes for MCP9600s and Ublox modules
  node *temp = head;
  while (temp != NULL)
  {
    if (temp->deviceType == DEVICE_TEMPERATURE_MCP9600)
    {
      //TODO Are we sure the MCP9600, begin'd() on the bus, but not logged will behave when bus is 400kHz?
      //Check if logging is enabled
      struct_MCP9600 *sensor = (struct_MCP9600*)temp->configPtr;
      if (sensor->log == true)
        maxSpeed = 100000;
    }

    if (temp->deviceType == DEVICE_GPS_UBLOX)
    {
      //Check if i2cSpeed is lowered
      struct_uBlox *sensor = (struct_uBlox*)temp->configPtr;
      if (sensor->i2cSpeed == 100000)
        maxSpeed = 100000;
    }

    temp = temp->next;
  }

  //If user wants to limit the I2C bus speed, do it here
  if (maxSpeed > settings.qwiicBusMaxSpeed)
    maxSpeed = settings.qwiicBusMaxSpeed;

  qwiic.setClock(maxSpeed);
  for (int i = 0; i < 100; i++) //Allow time for the speed to change
  {
    checkBattery();
    delay(1);
  }  
}

//Read the VIN voltage
float readVIN()
{
  // Only supported on >= V10 hardware
#if(HARDWARE_VERSION_MAJOR == 0)
  return(0.0); // Return 0.0V on old hardware
#else
  int div3 = analogRead(PIN_VIN_MONITOR); //Read VIN across a 1/3 resistor divider
  float vin = (float)div3 * 3.0 * 2.0 / 16384.0; //Convert 1/3 VIN to VIN (14-bit resolution)
  vin = vin * settings.vinCorrectionFactor; //Correct for divider impedance (determined experimentally)
  return (vin);
#endif
}
