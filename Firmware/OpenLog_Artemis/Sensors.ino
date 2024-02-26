


#include "Sensors.h"

#define HELPER_BUFFER_SIZE 1024

//Query each enabled sensor for its most recent data
void getData(char* sdOutputData, size_t lenData)
{
  measurementCount++;
  measurementTotal++;

  char tempData[50];
  char tempData1[16];
  char tempData2[16];
  char tempData3[16];
  sdOutputData[0] = '\0'; //Clear string contents

  if (settings.logRTC)
  {
    //Code written by @DennisMelamed in PR #70
    char timeString[37];
    getTimeString(timeString); // getTimeString is in timeStamp.ino
    strlcat(sdOutputData, timeString, lenData);
  }

  if (settings.logA11)
  {
    unsigned int analog11 = analogRead(11);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog11 * 2 / 16384.0;
      olaftoa(voltage, tempData1, 2, sizeof(tempData1) / sizeof(char));
      sprintf(tempData, "%s,", tempData1);
    }
    else
      sprintf(tempData, "%d,", analog11);

    strlcat(sdOutputData, tempData, lenData);
  }

  if (settings.logA12)
  {
    unsigned int analog12 = analogRead(12);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog12 * 2 / 16384.0;
      olaftoa(voltage, tempData1, 2, sizeof(tempData1) / sizeof(char));
      sprintf(tempData, "%s,", tempData1);
    }
    else
      sprintf(tempData, "%d,", analog12);

    strlcat(sdOutputData, tempData, lenData);
  }

  if (settings.logA13)
  {
    unsigned int analog13 = analogRead(13);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog13 * 2 / 16384.0;
      olaftoa(voltage, tempData1, 2, sizeof(tempData1) / sizeof(char));
      sprintf(tempData, "%s,", tempData1);
    }
    else
      sprintf(tempData, "%d,", analog13);

    strlcat(sdOutputData, tempData, lenData);
  }

  if (settings.logA32)
  {
    unsigned int analog32 = analogRead(32);

    if (settings.logAnalogVoltages == true)
    {
      float voltage = analog32 * 2 / 16384.0;
      olaftoa(voltage, tempData1, 2, sizeof(tempData1) / sizeof(char));
      sprintf(tempData, "%s,", tempData1);
    }
    else
      sprintf(tempData, "%d,", analog32);

    strlcat(sdOutputData, tempData, lenData);
  }

  if (settings.logVIN)
  {
    float voltage = readVIN();
    olaftoa(voltage, tempData1, 2, sizeof(tempData1) / sizeof(char));
    sprintf(tempData, "%s,", tempData1);
    strlcat(sdOutputData, tempData, lenData);
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
          olaftoa(myICM.accX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
          olaftoa(myICM.accY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
          olaftoa(myICM.accZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
          sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
          strlcat(sdOutputData, tempData, lenData);
        }
        if (settings.logIMUGyro)
        {
          olaftoa(myICM.gyrX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
          olaftoa(myICM.gyrY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
          olaftoa(myICM.gyrZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
          sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
          strlcat(sdOutputData, tempData, lenData);
        }
        if (settings.logIMUMag)
        {
          olaftoa(myICM.magX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
          olaftoa(myICM.magY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
          olaftoa(myICM.magZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
          sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
          strlcat(sdOutputData, tempData, lenData);
        }
        if (settings.logIMUTemp)
        {
          olaftoa(myICM.temp(), tempData1, 2, sizeof(tempData1) / sizeof(char));
          sprintf(tempData, "%s,", tempData1);
          strlcat(sdOutputData, tempData, lenData);
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
        olaftoa(((double)dmpData.Quat6.Data.Q1) / 1073741824.0, tempData1, 5, sizeof(tempData1) / sizeof(char));
        olaftoa(((double)dmpData.Quat6.Data.Q2) / 1073741824.0, tempData2, 5, sizeof(tempData2) / sizeof(char));
        olaftoa(((double)dmpData.Quat6.Data.Q3) / 1073741824.0, tempData3, 5, sizeof(tempData3) / sizeof(char));
        sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
        strlcat(sdOutputData, tempData, lenData);
      }
      if (settings.imuLogDMPQuat9)
      {
        olaftoa(((double)dmpData.Quat9.Data.Q1) / 1073741824.0, tempData1, 5, sizeof(tempData1) / sizeof(char));
        olaftoa(((double)dmpData.Quat9.Data.Q2) / 1073741824.0, tempData2, 5, sizeof(tempData2) / sizeof(char));
        olaftoa(((double)dmpData.Quat9.Data.Q3) / 1073741824.0, tempData3, 5, sizeof(tempData3) / sizeof(char));
        sprintf(tempData, "%s,%s,%s,%d,", tempData1, tempData2, tempData3, dmpData.Quat9.Data.Accuracy);
        strlcat(sdOutputData, tempData, lenData);
      }
      if (settings.imuLogDMPAccel)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Raw_Accel.Data.X, dmpData.Raw_Accel.Data.Y, dmpData.Raw_Accel.Data.Z);
        strlcat(sdOutputData, tempData, lenData);
      }
      if (settings.imuLogDMPGyro)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Raw_Gyro.Data.X, dmpData.Raw_Gyro.Data.Y, dmpData.Raw_Gyro.Data.Z);
        strlcat(sdOutputData, tempData, lenData);
      }
      if (settings.imuLogDMPCpass)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Compass.Data.X, dmpData.Compass.Data.Y, dmpData.Compass.Data.Z);
        strlcat(sdOutputData, tempData, lenData);
      }
    }
  }

  //Append all external sensor data on linked list to sdOutputData
  gatherDeviceValues(sdOutputData, lenData);

  if (settings.logHertz)
  {
    uint64_t currentMillis;

    //If we are sleeping between readings then we cannot rely on millis() as it is powered down
    //Use RTC instead
    currentMillis = rtcMillis();
    float actualRate;
    if ((currentMillis - measurementStartTime) < 1) // Avoid divide by zero
      actualRate = 0.0;
    else
      actualRate = measurementCount * 1000.0 / (currentMillis - measurementStartTime);
    olaftoa(actualRate, tempData1, 3, sizeof(tempData) / sizeof(char));
    sprintf(tempData, "%s,", tempData1);
    strlcat(sdOutputData, tempData, lenData);
  }

  if (settings.printMeasurementCount)
  {
    sprintf(tempData, "%d,", measurementTotal);
    strlcat(sdOutputData, tempData, lenData);
  }

  strlcat(sdOutputData, "\r\n", lenData);

  totalCharactersPrinted += strlen(sdOutputData);
}

//Read values from the devices on the node list
//Append values to sdOutputData
void gatherDeviceValues(char * sdOutputData, size_t lenData)
{
  char tempData[100];
  char tempData1[20];
  char tempData2[20];
  char tempData3[20];

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
              float currentWeight = nodeDevice->getWeight(true, nodeSetting->averageAmount); //Allow negative weights, take average of X readings
              olaftoa(currentWeight, tempData1, nodeSetting->decimalPlaces, sizeof(tempData1) / sizeof(char));
              sprintf(tempData, "%s,", tempData1);
              strlcat(sdOutputData, tempData, lenData);
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
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logRangeStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getRangeStatus());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logSignalRate)
              {
                sprintf(tempData, "%d,", nodeDevice->getSignalRate());
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_GPS_UBLOX:
          {
            setQwiicPullups(0); //Disable pullups to minimize CRC issues

            SFE_UBLOX_GNSS *nodeDevice = (SFE_UBLOX_GNSS *)temp->classPtr;
            struct_ublox *nodeSetting = (struct_ublox *)temp->configPtr;

            if (nodeSetting->log == true)
            {
              if (nodeSetting->logDate)
              {
                char gnssDayStr[3];
                char gnssMonthStr[3];
                char gnssYearStr[5];
                int gnssDay = nodeDevice->getDay();
                int gnssMonth = nodeDevice->getMonth();
                int gnssYear = nodeDevice->getYear();
                if (gnssDay < 10)
                  sprintf(gnssDayStr, "0%d", gnssDay);
                else
                  sprintf(gnssDayStr, "%d", gnssDay);
                if (gnssMonth < 10)
                  sprintf(gnssMonthStr, "0%d", gnssMonth);
                else
                  sprintf(gnssMonthStr, "%d", gnssMonth);
                sprintf(gnssYearStr, "%d", gnssYear);
                if (settings.dateStyle == 0)
                {
                  sprintf(tempData, "%s/%s/%s,", gnssMonthStr, gnssDayStr, gnssYearStr);
                }
                else if (settings.dateStyle == 1)
                {
                  sprintf(tempData, "%s/%s/%s,", gnssDayStr, gnssMonthStr, gnssYearStr);
                }
                else // if (settings.dateStyle == 2)
                {
                  sprintf(tempData, "%s/%s/%s,", gnssYearStr, gnssMonthStr, gnssDayStr);
                }
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTime)
              {
                int adjustedHour = nodeDevice->getHour();
                if (settings.hour24Style == false)
                  if (adjustedHour > 12) adjustedHour -= 12;

                char gnssHourStr[3];
                char gnssMinStr[3];
                char gnssSecStr[3];
                char gnssMillisStr[4];
                int gnssMin = nodeDevice->getMinute();
                int gnssSec = nodeDevice->getSecond();
                int gnssMillis = nodeDevice->getMillisecond();

                if (adjustedHour < 10)
                  sprintf(gnssHourStr, "0%d", adjustedHour);
                else
                  sprintf(gnssHourStr, "%d", adjustedHour);
                if (gnssMin < 10)
                  sprintf(gnssMinStr, "0%d", gnssMin);
                else
                  sprintf(gnssMinStr, "%d", gnssMin);
                if (gnssSec < 10)
                  sprintf(gnssSecStr, "0%d", gnssSec);
                else
                  sprintf(gnssSecStr, "%d", gnssSec);
                if (gnssMillis < 10)
                  sprintf(gnssMillisStr, "00%d", gnssMillis);
                else if (gnssMillis < 100)
                  sprintf(gnssMillisStr, "0%d", gnssMillis);
                else
                  sprintf(gnssMillisStr, "%d", gnssMillis);

                sprintf(tempData, "%s:%s:%s.%s,", gnssHourStr, gnssMinStr, gnssSecStr, gnssMillisStr);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPosition)
              {
                sprintf(tempData, "%d,%d,", nodeDevice->getLatitude(), nodeDevice->getLongitude());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logAltitude)
              {
                sprintf(tempData, "%d,", nodeDevice->getAltitude());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logAltitudeMSL)
              {
                sprintf(tempData, "%d,", nodeDevice->getAltitudeMSL());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logSIV)
              {
                sprintf(tempData, "%d,", nodeDevice->getSIV());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logFixType)
              {
                sprintf(tempData, "%d,", nodeDevice->getFixType());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logCarrierSolution)
              {
                sprintf(tempData, "%d,", nodeDevice->getCarrierSolutionType());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logGroundSpeed)
              {
                sprintf(tempData, "%d,", nodeDevice->getGroundSpeed());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logHeadingOfMotion)
              {
                sprintf(tempData, "%d,", nodeDevice->getHeading());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logpDOP)
              {
                sprintf(tempData, "%d,", nodeDevice->getPDOP());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logiTOW)
              {
                sprintf(tempData, "%d,", nodeDevice->getTimeOfWeek());
                strlcat(sdOutputData, tempData, lenData);
              }
            }

            setQwiicPullups(settings.qwiicBusPullUps); //Re-enable pullups
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
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logAmbientLight)
              {
                sprintf(tempData, "%d,", nodeDevice->getAmbient());
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->readTempC(), tempData1, 4, sizeof(tempData1) / sizeof(char)); //Resolution to 0.0078°C, accuracy of 0.1°C
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->getPressure(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->getPressure_hPa(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature_degC(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_PRESSURE_LPS28DFW:
          {
            LPS28DFW *nodeDevice = (LPS28DFW *)temp->classPtr;
            struct_LPS28DFW *nodeSetting = (struct_LPS28DFW *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              nodeDevice->getSensorData();

              if (nodeSetting->logPressure)
              {
                olaftoa(nodeDevice->data.pressure.hpa, tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->data.heat.deg_c, tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->readFloatPressure(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logHumidity)
              {
                olaftoa(nodeDevice->readFloatHumidity(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logAltitude)
              {
                olaftoa(nodeDevice->readFloatAltitudeMeters(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->readTempC(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->uva(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logUVB)
              {
                olaftoa(nodeDevice->uvb(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logUVIndex)
              {
                olaftoa(nodeDevice->index(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_LIGHT_VEML7700:
          {
            VEML7700 *nodeDevice = (VEML7700 *)temp->classPtr;
            struct_VEML7700 *nodeSetting = (struct_VEML7700 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              olaftoa(nodeDevice->getLux(), tempData1, 2, sizeof(tempData1) / sizeof(char));
              sprintf(tempData, "%s,", tempData1);
              strlcat(sdOutputData, tempData, lenData);
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
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logCO2)
              {
                sprintf(tempData, "%d,", nodeDevice->getCO2());
                strlcat(sdOutputData, tempData, lenData);
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
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logCO2)
              {
                sprintf(tempData, "%d,", nodeDevice->CO2);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logH2)
              {
                sprintf(tempData, "%d,", nodeDevice->H2);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logEthanol)
              {
                sprintf(tempData, "%d,", nodeDevice->ethanol);
                strlcat(sdOutputData, tempData, lenData);
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
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logHumidity)
              {
                olaftoa(nodeDevice->getHumidity(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->getHumidity(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPressure)
              {
                olaftoa(nodeDevice->getPressure(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->getThermocoupleTemp(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logAmbientTemperature)
              {
                olaftoa(nodeDevice->getAmbientTemp(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->getHumidity(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->toPercent(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->toDegC(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->readPT100Centigrade(), tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logFahrenheit)
              {
                olaftoa(nodeDevice->readPT100Fahrenheit(), tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logInternalTemperature)
              {
                olaftoa(nodeDevice->readInternalTemperature(), tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logRawVoltage)
              {
                sprintf(tempData, "%d,", nodeDevice->readRawVoltage());
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->readPressure(), tempData1, 4, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->usePA)
              {
                olaftoa(nodeDevice->readPressure(PA), tempData1, 1, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->useKPA)
              {
                olaftoa(nodeDevice->readPressure(KPA), tempData1, 4, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->useTORR)
              {
                olaftoa(nodeDevice->readPressure(TORR), tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->useINHG)
              {
                olaftoa(nodeDevice->readPressure(INHG), tempData1, 4, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->useATM)
              {
                olaftoa(nodeDevice->readPressure(ATM), tempData1, 6, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->useBAR)
              {
                olaftoa(nodeDevice->readPressure(BAR), tempData1, 6, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->getPM1_0(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPM25)
              {
                olaftoa(nodeDevice->getPM2_5(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPM10)
              {
                olaftoa(nodeDevice->getPM10(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPC05)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC0_5());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPC1)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC1_0());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPC25)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC2_5());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPC50)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC5_0());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPC75)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC7_5());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPC10)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC10());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logSensorStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusSensors());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logPDStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusPD());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logLDStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusLD());
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logFanStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusFan());
                strlcat(sdOutputData, tempData, lenData);
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
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(pressure, tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(temperature, tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
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
                olaftoa(nodeDevice->pressure(nodeSetting->conversion), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->temperature(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logDepth)
              {
                olaftoa(nodeDevice->depth(), tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logAltitude)
              {
                olaftoa(nodeDevice->altitude(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_QWIIC_BUTTON:
          {
            QwiicButton *nodeDevice = (QwiicButton *)temp->classPtr;
            struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              long pressedPopped = 0;
              while (nodeDevice->isPressedQueueEmpty() == false)
              {
                pressedPopped = nodeDevice->popPressedQueue();
              }
              if (nodeSetting->logPressed)
              {
                olaftoa(((float)pressedPopped) / 1000.0, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }

              long clickedPopped = 0;
              while (nodeDevice->isClickedQueueEmpty() == false)
              {
                clickedPopped = nodeDevice->popClickedQueue();
                nodeSetting->ledState ^= 1; // Toggle nodeSetting->ledState on _every_ click (not just the most recent)
              }
              if (nodeSetting->logClicked)
              {
                olaftoa(((float)clickedPopped) / 1000.0, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }

              if (nodeSetting->toggleLEDOnClick)
              {
                if (nodeSetting->ledState)
                  nodeDevice->LEDon(nodeSetting->ledBrightness);
                else
                  nodeDevice->LEDoff();
                sprintf(tempData, "%d,", nodeSetting->ledState);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
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
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logConfidence)
              {
                sprintf(tempData, "%d,", body.confidence);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logOxygen)
              {
                sprintf(tempData, "%d,", body.oxygen);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logStatus)
              {
                sprintf(tempData, "%d,", body.status);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logExtendedStatus)
              {
                sprintf(tempData, "%d,", body.extStatus);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logRValue)
              {
                olaftoa(body.rValue, tempData1, 1, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_ISM330DHCX:
          {
            SparkFun_ISM330DHCX *nodeDevice = (SparkFun_ISM330DHCX *)temp->classPtr;
            struct_ISM330DHCX *nodeSetting = (struct_ISM330DHCX *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              // Structs for X,Y,Z data
              static sfe_ism_data_t accelData;
              static sfe_ism_data_t gyroData;
              static bool dataReady;
              if ((nodeSetting->logAccel) || (nodeSetting->logGyro))
              {
                // Check if both gyroscope and accelerometer data is available.
                dataReady = nodeDevice->checkStatus();
                if( dataReady )
                {
                  nodeDevice->getAccel(&accelData);
                  nodeDevice->getGyro(&gyroData);
                }
              }
              if (nodeSetting->logAccel)
              {
                olaftoa(accelData.xData, tempData1, 2, sizeof(tempData1) / sizeof(char));
                olaftoa(accelData.yData, tempData2, 2, sizeof(tempData2) / sizeof(char));
                olaftoa(accelData.zData, tempData3, 2, sizeof(tempData3) / sizeof(char));
                sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logGyro)
              {
                olaftoa(gyroData.xData, tempData1, 2, sizeof(tempData1) / sizeof(char));
                olaftoa(gyroData.yData, tempData2, 2, sizeof(tempData2) / sizeof(char));
                olaftoa(gyroData.zData, tempData3, 2, sizeof(tempData3) / sizeof(char));
                sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logDataReady)
              {
                sprintf(tempData, "%d,", dataReady);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_MMC5983MA:
          {
            SFE_MMC5983MA *nodeDevice = (SFE_MMC5983MA *)temp->classPtr;
            struct_MMC5983MA *nodeSetting = (struct_MMC5983MA *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              // X,Y,Z data
              uint32_t rawValueX;
              uint32_t rawValueY;
              uint32_t rawValueZ;
              double normalizedX;
              double normalizedY;
              double normalizedZ;
              if (nodeSetting->logMag)
              {

                nodeDevice->getMeasurementXYZ(&rawValueX, &rawValueY, &rawValueZ);

                // The magnetic field values are 18-bit unsigned. The zero (mid) point is 2^17 (131072).
                // Normalize each field to +/- 1.0
                normalizedX = (double)rawValueX - 131072.0;
                normalizedX /= 131072.0;
                normalizedY = (double)rawValueY - 131072.0;
                normalizedY /= 131072.0;
                normalizedZ = (double)rawValueZ - 131072.0;
                normalizedZ /= 131072.0;
                // Convert to Gauss
                normalizedX *= 8.0;
                normalizedY *= 8.0;
                normalizedZ *= 8.0;

                olaftoa(normalizedX, tempData1, 6, sizeof(tempData1) / sizeof(char));
                olaftoa(normalizedY, tempData2, 6, sizeof(tempData2) / sizeof(char));
                olaftoa(normalizedZ, tempData3, 6, sizeof(tempData3) / sizeof(char));
                sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logTemperature)
              {
                int temperature = nodeDevice->getTemperature();
                sprintf(tempData, "%d,", temperature);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_ADS1015:
          {
            ADS1015 *nodeDevice = (ADS1015 *)temp->classPtr;
            struct_ADS1015 *nodeSetting = (struct_ADS1015 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logA0)
              {
                float channel_mV = nodeDevice->getSingleEndedMillivolts(0);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logA1)
              {
                float channel_mV = nodeDevice->getSingleEndedMillivolts(1);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logA2)
              {
                float channel_mV = nodeDevice->getSingleEndedMillivolts(2);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logA3)
              {
                float channel_mV = nodeDevice->getSingleEndedMillivolts(3);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logA0A1)
              {
                float channel_mV = nodeDevice->getDifferentialMillivolts(ADS1015_CONFIG_MUX_DIFF_P0_N1);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logA0A3)
              {
                float channel_mV = nodeDevice->getDifferentialMillivolts(ADS1015_CONFIG_MUX_DIFF_P0_N3);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logA1A3)
              {
                float channel_mV = nodeDevice->getDifferentialMillivolts(ADS1015_CONFIG_MUX_DIFF_P1_N3);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logA2A3)
              {
                float channel_mV = nodeDevice->getDifferentialMillivolts(ADS1015_CONFIG_MUX_DIFF_P2_N3);
                olaftoa(channel_mV, tempData1, 3, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strlcat(sdOutputData, tempData, lenData);
              }
            }
          }
          break;
        case DEVICE_KX134:
          {
            SparkFun_KX134 *nodeDevice = (SparkFun_KX134 *)temp->classPtr;
            struct_KX134 *nodeSetting = (struct_KX134 *)temp->configPtr;
            if (nodeSetting->log == true)
            {
              // X,Y,Z data
              static outputData xyzData;
              static bool dataReady = false;

              if ((nodeSetting->logAccel) || (nodeSetting->logDataReady))
              {
                // Check if accel data is available.
                dataReady = nodeDevice->dataReady();
                if (dataReady)
                  nodeDevice->getAccelData(&xyzData);
              }

              if (nodeSetting->logAccel)
              {
                olaftoa(xyzData.xData, tempData1, 4, sizeof(tempData1) / sizeof(char));
                olaftoa(xyzData.yData, tempData2, 4, sizeof(tempData2) / sizeof(char));
                olaftoa(xyzData.zData, tempData3, 4, sizeof(tempData3) / sizeof(char));
                sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
                strlcat(sdOutputData, tempData, lenData);
              }
              if (nodeSetting->logDataReady)
              {
                sprintf(tempData, "%d,", dataReady);
                strlcat(sdOutputData, tempData, lenData);
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


static void getHelperText(char* helperText, size_t lenText)
{

  helperText[0]='\0';

  if (settings.logRTC)
  {
    if (settings.logDate)
      strlcat(helperText, "rtcDate,", lenText);
    if (settings.logTime)
      strlcat(helperText, "rtcTime,", lenText);
    if (settings.logMicroseconds)
      strlcat(helperText, "micros,", lenText);
  }

  if (settings.logA11)
    strlcat(helperText, "analog_11,", lenText);

  if (settings.logA12)
    strlcat(helperText, "analog_12,", lenText);

  if (settings.logA13)
    strlcat(helperText, "analog_13,", lenText);

  if (settings.logA32)
    strlcat(helperText, "analog_32,", lenText);

  if (settings.logVIN)
    strlcat(helperText, "VIN,", lenText);

  if (online.IMU)
  {
    if (settings.imuUseDMP == false)
    {
      if (settings.logIMUAccel)
        strlcat(helperText, "aX,aY,aZ,", lenText);
      if (settings.logIMUGyro)
        strlcat(helperText, "gX,gY,gZ,", lenText);
      if (settings.logIMUMag)
        strlcat(helperText, "mX,mY,mZ,", lenText);
      if (settings.logIMUTemp)
        strlcat(helperText, "imu_degC,", lenText);
    }
    else
    {
      if (settings.imuLogDMPQuat6)
        strlcat(helperText, "Q6_1,Q6_2,Q6_3,", lenText);
      if (settings.imuLogDMPQuat9)
        strlcat(helperText, "Q9_1,Q9_2,Q9_3,HeadAcc,", lenText);
      if (settings.imuLogDMPAccel)
        strlcat(helperText, "RawAX,RawAY,RawAZ,", lenText);
      if (settings.imuLogDMPGyro)
        strlcat(helperText, "RawGX,RawGY,RawGZ,", lenText);
      if (settings.imuLogDMPCpass)
        strlcat(helperText, "RawMX,RawMY,RawMZ,", lenText);
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
              strlcat(helperText, "weight(no unit),", lenText);
          }
          break;
        case DEVICE_DISTANCE_VL53L1X:
          {
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logDistance)
                strlcat(helperText, "distance_mm,", lenText);
              if (nodeSetting->logRangeStatus)
                strlcat(helperText, "distance_rangeStatus(0=good),", lenText);
              if (nodeSetting->logSignalRate)
                strlcat(helperText, "distance_signalRate,", lenText);
            }
          }
          break;
        case DEVICE_GPS_UBLOX:
          {
            struct_ublox *nodeSetting = (struct_ublox *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logDate)
                strlcat(helperText, "gps_Date,", lenText);
              if (nodeSetting->logTime)
                strlcat(helperText, "gps_Time,", lenText);
              if (nodeSetting->logPosition)
                strlcat(helperText, "gps_Lat,gps_Long,", lenText);
              if (nodeSetting->logAltitude)
                strlcat(helperText, "gps_Alt,", lenText);
              if (nodeSetting->logAltitudeMSL)
                strlcat(helperText, "gps_AltMSL,", lenText);
              if (nodeSetting->logSIV)
                strlcat(helperText, "gps_SIV,", lenText);
              if (nodeSetting->logFixType)
                strlcat(helperText, "gps_FixType,", lenText);
              if (nodeSetting->logCarrierSolution)
                strlcat(helperText, "gps_CarrierSolution,", lenText);
              if (nodeSetting->logGroundSpeed)
                strlcat(helperText, "gps_GroundSpeed,", lenText);
              if (nodeSetting->logHeadingOfMotion)
                strlcat(helperText, "gps_Heading,", lenText);
              if (nodeSetting->logpDOP)
                strlcat(helperText, "gps_pDOP,", lenText);
              if (nodeSetting->logiTOW)
                strlcat(helperText, "gps_iTOW,", lenText);
            }
          }
          break;
        case DEVICE_PROXIMITY_VCNL4040:
          {
            struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logProximity)
                strlcat(helperText, "prox(no unit),", lenText);
              if (nodeSetting->logAmbientLight)
                strlcat(helperText, "ambient_lux,", lenText);
            }
          }
          break;
        case DEVICE_TEMPERATURE_TMP117:
          {
            struct_TMP117 *nodeSetting = (struct_TMP117 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
            }
          }
          break;
        case DEVICE_PRESSURE_MS5637:
          {
            struct_MS5637 *nodeSetting = (struct_MS5637 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strlcat(helperText, "pressure_hPa,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "temperature_degC,", lenText);
            }
          }
          break;
        case DEVICE_PRESSURE_LPS25HB:
          {
            struct_LPS25HB *nodeSetting = (struct_LPS25HB *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strlcat(helperText, "pressure_hPa,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "temperature_degC,", lenText);
            }
          }
          break;
        case DEVICE_PRESSURE_LPS28DFW:
          {
            struct_LPS28DFW *nodeSetting = (struct_LPS28DFW *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strlcat(helperText, "pressure_hPa,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "temperature_degC,", lenText);
            }
          }
          break;
        case DEVICE_PHT_BME280:
          {
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strlcat(helperText, "pressure_Pa,", lenText);
              if (nodeSetting->logHumidity)
                strlcat(helperText, "humidity_%,", lenText);
              if (nodeSetting->logAltitude)
                strlcat(helperText, "altitude_m,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "temp_degC,", lenText);
            }
          }
          break;
        case DEVICE_UV_VEML6075:
          {
            struct_VEML6075 *nodeSetting = (struct_VEML6075 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logUVA)
                strlcat(helperText, "uva,", lenText);
              if (nodeSetting->logUVB)
                strlcat(helperText, "uvb,", lenText);
              if (nodeSetting->logUVIndex)
                strlcat(helperText, "uvIndex,", lenText);
            }
          }
          break;
        case DEVICE_LIGHT_VEML7700:
          {
            struct_VEML7700 *nodeSetting = (struct_VEML7700 *)temp->configPtr;
            if (nodeSetting->log)
            {
              strlcat(helperText, "lux,", lenText);
            }
          }
          break;
        case DEVICE_VOC_CCS811:
          {
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTVOC)
                strlcat(helperText, "tvoc_ppb,", lenText);
              if (nodeSetting->logCO2)
                strlcat(helperText, "co2_ppm,", lenText);
            }
          }
          break;
        case DEVICE_VOC_SGP30:
          {
            struct_SGP30 *nodeSetting = (struct_SGP30 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTVOC)
                strlcat(helperText, "tvoc_ppb,", lenText);
              if (nodeSetting->logCO2)
                strlcat(helperText, "co2_ppm,", lenText);
              if (nodeSetting->logH2)
                strlcat(helperText, "H2,", lenText);
              if (nodeSetting->logEthanol)
                strlcat(helperText, "ethanol,", lenText);
            }
          }
          break;
        case DEVICE_CO2_SCD30:
          {
            struct_SCD30 *nodeSetting = (struct_SCD30 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logCO2)
                strlcat(helperText, "co2_ppm,", lenText);
              if (nodeSetting->logHumidity)
                strlcat(helperText, "humidity_%,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
            }
          }
          break;
        case DEVICE_PHT_MS8607:
          {
            struct_MS8607 *nodeSetting = (struct_MS8607 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
                strlcat(helperText, "humidity_%,", lenText);
              if (nodeSetting->logPressure)
                strlcat(helperText, "hPa,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
            }
          }
          break;
        case DEVICE_TEMPERATURE_MCP9600:
          {
            struct_MCP9600 *nodeSetting = (struct_MCP9600 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTemperature)
                strlcat(helperText, "thermo_degC,", lenText);
              if (nodeSetting->logAmbientTemperature)
                strlcat(helperText, "thermo_ambientDegC,", lenText);
            }
          }
          break;
        case DEVICE_HUMIDITY_AHT20:
          {
            struct_AHT20 *nodeSetting = (struct_AHT20 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
                strlcat(helperText, "humidity_%,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
            }
          }
          break;
        case DEVICE_HUMIDITY_SHTC3:
          {
            struct_SHTC3 *nodeSetting = (struct_SHTC3 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
                strlcat(helperText, "humidity_%,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
            }
          }
          break;
        case DEVICE_ADC_ADS122C04:
          {
            struct_ADS122C04 *nodeSetting = (struct_ADS122C04 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logCentigrade)
                strlcat(helperText, "degC,", lenText);
              if (nodeSetting->logFahrenheit)
                strlcat(helperText, "degF,", lenText);
              if (nodeSetting->logInternalTemperature)
                strlcat(helperText, "degC,", lenText);
              if (nodeSetting->logRawVoltage)
                strlcat(helperText, "V*2.048/2^23,", lenText);
            }
          }
          break;
        case DEVICE_PRESSURE_MPR0025PA1:
          {
            struct_MPR0025PA1 *nodeSetting = (struct_MPR0025PA1 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->usePSI)
                strlcat(helperText, "PSI,", lenText);
              if (nodeSetting->usePA)
                strlcat(helperText, "Pa,", lenText);
              if (nodeSetting->useKPA)
                strlcat(helperText, "kPa,", lenText);
              if (nodeSetting->useTORR)
                strlcat(helperText, "torr,", lenText);
              if (nodeSetting->useINHG)
                strlcat(helperText, "inHg,", lenText);
              if (nodeSetting->useATM)
                strlcat(helperText, "atm,", lenText);
              if (nodeSetting->useBAR)
                strlcat(helperText, "bar,", lenText);
            }
          }
          break;
        case DEVICE_PARTICLE_SNGCJA5:
          {
            struct_SNGCJA5 *nodeSetting = (struct_SNGCJA5 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPM1)
                strlcat(helperText, "PM1_0,", lenText);
              if (nodeSetting->logPM25)
                strlcat(helperText, "PM2_5,", lenText);
              if (nodeSetting->logPM10)
                strlcat(helperText, "PM10,", lenText);
              if (nodeSetting->logPC05)
                strlcat(helperText, "PC0_5,", lenText);
              if (nodeSetting->logPC1)
                strlcat(helperText, "PC1_0,", lenText);
              if (nodeSetting->logPC25)
                strlcat(helperText, "PC2_5,", lenText);
              if (nodeSetting->logPC50)
                strlcat(helperText, "PC5_0,", lenText);
              if (nodeSetting->logPC75)
                strlcat(helperText, "PC7_5,", lenText);
              if (nodeSetting->logPC10)
                strlcat(helperText, "PC10,", lenText);
              if (nodeSetting->logSensorStatus)
                strlcat(helperText, "Sensors,", lenText);
              if (nodeSetting->logPDStatus)
                strlcat(helperText, "PD,", lenText);
              if (nodeSetting->logLDStatus)
                strlcat(helperText, "LD,", lenText);
              if (nodeSetting->logFanStatus)
                strlcat(helperText, "Fan,", lenText);
            }
          }
          break;
        case DEVICE_VOC_SGP40:
          {
            struct_SGP40 *nodeSetting = (struct_SGP40 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logVOC)
                strlcat(helperText, "VOCindex,", lenText);
            }
          }
          break;
        case DEVICE_PRESSURE_SDP3X:
          {
            struct_SDP3X *nodeSetting = (struct_SDP3X *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strlcat(helperText, "Pa,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
            }
          }
          break;
        case DEVICE_PRESSURE_MS5837:
          {
            struct_MS5837 *nodeSetting = (struct_MS5837 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
                strlcat(helperText, "mbar,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
              if (nodeSetting->logDepth)
                strlcat(helperText, "depth_m,", lenText);
              if (nodeSetting->logAltitude)
                strlcat(helperText, "alt_m,", lenText);
            }
          }
          break;
        case DEVICE_QWIIC_BUTTON:
          {
            struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressed)
                strlcat(helperText, "pressS,", lenText);
              if (nodeSetting->logClicked)
                strlcat(helperText, "clickS,", lenText);
              if (nodeSetting->toggleLEDOnClick)
                strlcat(helperText, "LED,", lenText);
            }
          }
          break;
        case DEVICE_BIO_SENSOR_HUB:
          {
            struct_BIO_SENSOR_HUB *nodeSetting = (struct_BIO_SENSOR_HUB *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHeartrate)
                strlcat(helperText, "bpm,", lenText);
              if (nodeSetting->logConfidence)
                strlcat(helperText, "conf%,", lenText);
              if (nodeSetting->logOxygen)
                strlcat(helperText, "O2%,", lenText);
              if (nodeSetting->logStatus)
                strlcat(helperText, "stat,", lenText);
              if (nodeSetting->logExtendedStatus)
                strlcat(helperText, "eStat,", lenText);
              if (nodeSetting->logRValue)
                strlcat(helperText, "O2R,", lenText);
            }
          }
          break;
        case DEVICE_ISM330DHCX:
          {
            struct_ISM330DHCX *nodeSetting = (struct_ISM330DHCX *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logAccel)
                strlcat(helperText, "aX,aY,aZ,", lenText);
              if (nodeSetting->logGyro)
                strlcat(helperText, "gX,gY,gZ,", lenText);
              if (nodeSetting->logDataReady)
                strlcat(helperText, "dataRdy,", lenText);
            }
          }
          break;
        case DEVICE_MMC5983MA:
          {
            struct_MMC5983MA *nodeSetting = (struct_MMC5983MA *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logMag)
                strlcat(helperText, "mX,mY,mZ,", lenText);
              if (nodeSetting->logTemperature)
                strlcat(helperText, "degC,", lenText);
            }
          }
          break;
        case DEVICE_KX134:
          {
            struct_KX134 *nodeSetting = (struct_KX134 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logAccel)
                strlcat(helperText, "aX,aY,aZ,", lenText);
              if (nodeSetting->logDataReady)
                strlcat(helperText, "dataRdy,", lenText);
            }
          }
          break;
        case DEVICE_ADS1015:
          {
            struct_ADS1015 *nodeSetting = (struct_ADS1015 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logA0)
                strlcat(helperText, "A0mV,", lenText);
              if (nodeSetting->logA1)
                strlcat(helperText, "A1mV,", lenText);
              if (nodeSetting->logA2)
                strlcat(helperText, "A2mV,", lenText);
              if (nodeSetting->logA3)
                strlcat(helperText, "A3mV,", lenText);
              if (nodeSetting->logA0A1)
                strlcat(helperText, "A0A1mV,", lenText);
              if (nodeSetting->logA0A3)
                strlcat(helperText, "A0A3mV,", lenText);
              if (nodeSetting->logA1A3)
                strlcat(helperText, "A1A3mV,", lenText);
              if (nodeSetting->logA2A3)
                strlcat(helperText, "A2A3mV,", lenText);
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
    strlcat(helperText, "output_Hz,", lenText);

  if (settings.printMeasurementCount)
    strlcat(helperText, "count,", lenText);

  strlcat(helperText, "\r\n", lenText);
}


void printHelperText(uint8_t outputDest)
{
  char helperText[HELPER_BUFFER_SIZE];
  helperText[0] = '\0';

  getHelperText(helperText, sizeof(helperText));

  if(outputDest & OL_OUTPUT_SERIAL)
    SerialPrint(helperText);

  if ((outputDest & OL_OUTPUT_SDCARD) && (settings.logData == true) && (online.microSD))
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
      struct_ublox *sensor = (struct_ublox*)temp->configPtr;
      if (sensor->i2cSpeed == 100000)
      {
        //printDebug("setMaxI2CSpeed: sensor->i2cSpeed is 100000. Reducing maxSpeed to 100kHz\r\n");
        maxSpeed = 100000;
      }
    }

    temp = temp->next;
  }

  //If user wants to limit the I2C bus speed, do it here
  if (maxSpeed > settings.qwiicBusMaxSpeed)
  {
    //printDebug("setMaxI2CSpeed: maxSpeed is > settings.qwiicBusMaxSpeed. Reducing maxSpeed to " + (String)settings.qwiicBusMaxSpeed + "Hz\r\n");
    maxSpeed = settings.qwiicBusMaxSpeed;
  }

  if (maxSpeed > 200000)
  {
    printDebug("setMaxI2CSpeed: setting qwiic clock speed to " + (String)AM_HAL_IOM_400KHZ + "Hz\r\n");
    qwiic.setClock(AM_HAL_IOM_400KHZ);
  }
  else
  {
    printDebug("setMaxI2CSpeed: setting qwiic clock speed to " + (String)AM_HAL_IOM_100KHZ + "Hz\r\n");
    qwiic.setClock(AM_HAL_IOM_100KHZ);
  }
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
