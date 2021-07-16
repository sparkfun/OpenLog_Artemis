void updateBLECharacteristic(int *theBLECharacteristic, char *theString)
{
  int bleCharacteristic = *theBLECharacteristic;
  if (usingBLE)
  {
    if (bleCharacteristic < numBLECharacteristics)
    {
      if (bleCharacteristic == 0) bleCharacteristic00.setValue(theString);
      if (bleCharacteristic == 1) bleCharacteristic01.setValue(theString);
      if (bleCharacteristic == 2) bleCharacteristic02.setValue(theString);
      if (bleCharacteristic == 3) bleCharacteristic03.setValue(theString);
      if (bleCharacteristic == 4) bleCharacteristic04.setValue(theString);
      if (bleCharacteristic == 5) bleCharacteristic05.setValue(theString);
      if (bleCharacteristic == 6) bleCharacteristic06.setValue(theString);
      if (bleCharacteristic == 7) bleCharacteristic07.setValue(theString);
      if (bleCharacteristic == 8) bleCharacteristic08.setValue(theString);
      if (bleCharacteristic == 9) bleCharacteristic09.setValue(theString);
      if (bleCharacteristic == 10) bleCharacteristic10.setValue(theString);
      if (bleCharacteristic == 11) bleCharacteristic11.setValue(theString);
      if (bleCharacteristic == 12) bleCharacteristic12.setValue(theString);
      if (bleCharacteristic == 13) bleCharacteristic13.setValue(theString);
      if (bleCharacteristic == 14) bleCharacteristic14.setValue(theString);
      if (bleCharacteristic == 15) bleCharacteristic15.setValue(theString);
      if (bleCharacteristic == 16) bleCharacteristic16.setValue(theString);
      if (bleCharacteristic == 17) bleCharacteristic17.setValue(theString);
      if (bleCharacteristic == 18) bleCharacteristic18.setValue(theString);
      if (bleCharacteristic == 19) bleCharacteristic19.setValue(theString);
      if (bleCharacteristic == 20) bleCharacteristic20.setValue(theString);
      if (bleCharacteristic == 21) bleCharacteristic21.setValue(theString);
      if (bleCharacteristic == 22) bleCharacteristic22.setValue(theString);
      if (bleCharacteristic == 23) bleCharacteristic23.setValue(theString);
      if (bleCharacteristic == 24) bleCharacteristic24.setValue(theString);
      if (bleCharacteristic == 25) bleCharacteristic25.setValue(theString);
      if (bleCharacteristic == 26) bleCharacteristic26.setValue(theString);
      if (bleCharacteristic == 27) bleCharacteristic27.setValue(theString);
      if (bleCharacteristic == 28) bleCharacteristic28.setValue(theString);
      if (bleCharacteristic == 29) bleCharacteristic29.setValue(theString);
      if (bleCharacteristic == 30) bleCharacteristic30.setValue(theString);
      if (bleCharacteristic == 31) bleCharacteristic31.setValue(theString);
      if (bleCharacteristic == 32) bleCharacteristic32.setValue(theString);
      if (bleCharacteristic == 33) bleCharacteristic33.setValue(theString);
      if (bleCharacteristic == 34) bleCharacteristic34.setValue(theString);
      if (bleCharacteristic == 35) bleCharacteristic35.setValue(theString);
      if (bleCharacteristic == 36) bleCharacteristic36.setValue(theString);
      if (bleCharacteristic == 37) bleCharacteristic37.setValue(theString);
      if (bleCharacteristic == 38) bleCharacteristic38.setValue(theString);
      if (bleCharacteristic == 39) bleCharacteristic39.setValue(theString);
      if (bleCharacteristic == 40) bleCharacteristic40.setValue(theString);
      if (bleCharacteristic == 41) bleCharacteristic41.setValue(theString);
      if (bleCharacteristic == 42) bleCharacteristic42.setValue(theString);
      if (bleCharacteristic == 43) bleCharacteristic43.setValue(theString);
      if (bleCharacteristic == 44) bleCharacteristic44.setValue(theString);
      if (bleCharacteristic == 45) bleCharacteristic45.setValue(theString);
      if (bleCharacteristic == 46) bleCharacteristic46.setValue(theString);
      if (bleCharacteristic == 47) bleCharacteristic47.setValue(theString);
      if (bleCharacteristic == 48) bleCharacteristic48.setValue(theString);
      if (bleCharacteristic == 49) bleCharacteristic49.setValue(theString);
      *theBLECharacteristic = bleCharacteristic + 1;
    }
  }
}

//Query each enabled sensor for its most recent data
void getData()
{
  measurementCount++;
  measurementTotal++;

  char tempData[50];
  char tempData1[16];
  char tempData2[16];
  char tempData3[16];
  outputData[0] = '\0'; //Clear string contents
  int bleCharacteristic = 0;

  if (settings.logRTC)
  {
    //Code written by @DennisMelamed in PR #70
    char timeString[37];
    getTimeString(timeString); // getTimeString is in timeStamp.ino
    strcat(outputData, timeString);
    updateBLECharacteristic(&bleCharacteristic, timeString);
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

    strcat(outputData, tempData);
    updateBLECharacteristic(&bleCharacteristic, tempData);
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

    strcat(outputData, tempData);
    updateBLECharacteristic(&bleCharacteristic, tempData);
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

    strcat(outputData, tempData);
    updateBLECharacteristic(&bleCharacteristic, tempData);
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

    strcat(outputData, tempData);
    updateBLECharacteristic(&bleCharacteristic, tempData);
  }

  if (settings.logVIN)
  {
    float voltage = readVIN();
    olaftoa(voltage, tempData1, 2, sizeof(tempData1) / sizeof(char));
    sprintf(tempData, "%s,", tempData1);
    strcat(outputData, tempData);
    updateBLECharacteristic(&bleCharacteristic, tempData);
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
          strcat(outputData, tempData);
          sprintf(tempData, "%s,", tempData1); updateBLECharacteristic(&bleCharacteristic, tempData);
          sprintf(tempData, "%s,", tempData2); updateBLECharacteristic(&bleCharacteristic, tempData);
          sprintf(tempData, "%s,", tempData3); updateBLECharacteristic(&bleCharacteristic, tempData);
        }
        if (settings.logIMUGyro)
        {
          olaftoa(myICM.gyrX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
          olaftoa(myICM.gyrY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
          olaftoa(myICM.gyrZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
          sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
          strcat(outputData, tempData);
          sprintf(tempData, "%s,", tempData1); updateBLECharacteristic(&bleCharacteristic, tempData);
          sprintf(tempData, "%s,", tempData2); updateBLECharacteristic(&bleCharacteristic, tempData);
          sprintf(tempData, "%s,", tempData3); updateBLECharacteristic(&bleCharacteristic, tempData);
        }
        if (settings.logIMUMag)
        {
          olaftoa(myICM.magX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
          olaftoa(myICM.magY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
          olaftoa(myICM.magZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
          sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
          strcat(outputData, tempData);
          sprintf(tempData, "%s,", tempData1); updateBLECharacteristic(&bleCharacteristic, tempData);
          sprintf(tempData, "%s,", tempData2); updateBLECharacteristic(&bleCharacteristic, tempData);
          sprintf(tempData, "%s,", tempData3); updateBLECharacteristic(&bleCharacteristic, tempData);
        }
        if (settings.logIMUTemp)
        {
          olaftoa(myICM.temp(), tempData1, 2, sizeof(tempData1) / sizeof(char));
          sprintf(tempData, "%s,", tempData1);
          strcat(outputData, tempData);
          updateBLECharacteristic(&bleCharacteristic, tempData);
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
        strcat(outputData, tempData);
        sprintf(tempData, "%s,", tempData1); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%s,", tempData2); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%s,", tempData3); updateBLECharacteristic(&bleCharacteristic, tempData);
      }
      if (settings.imuLogDMPQuat9)
      {
        olaftoa(((double)dmpData.Quat9.Data.Q1) / 1073741824.0, tempData1, 5, sizeof(tempData1) / sizeof(char));
        olaftoa(((double)dmpData.Quat9.Data.Q2) / 1073741824.0, tempData2, 5, sizeof(tempData2) / sizeof(char));
        olaftoa(((double)dmpData.Quat9.Data.Q3) / 1073741824.0, tempData3, 5, sizeof(tempData3) / sizeof(char));
        sprintf(tempData, "%s,%s,%s,%d,", tempData1, tempData2, tempData3, dmpData.Quat9.Data.Accuracy);
        strcat(outputData, tempData);
        sprintf(tempData, "%s,", tempData1); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%s,", tempData2); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%s,", tempData3); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%d,", dmpData.Quat9.Data.Accuracy); updateBLECharacteristic(&bleCharacteristic, tempData);
      }
      if (settings.imuLogDMPAccel)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Raw_Accel.Data.X, dmpData.Raw_Accel.Data.Y, dmpData.Raw_Accel.Data.Z);
        strcat(outputData, tempData);
        sprintf(tempData, "%d,", dmpData.Raw_Accel.Data.X); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%d,", dmpData.Raw_Accel.Data.Y); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%d,", dmpData.Raw_Accel.Data.Z); updateBLECharacteristic(&bleCharacteristic, tempData);
      }
      if (settings.imuLogDMPGyro)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Raw_Gyro.Data.X, dmpData.Raw_Gyro.Data.Y, dmpData.Raw_Gyro.Data.Z);
        strcat(outputData, tempData);
        sprintf(tempData, "%d,", dmpData.Raw_Gyro.Data.X); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%d,", dmpData.Raw_Gyro.Data.Y); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%d,", dmpData.Raw_Gyro.Data.Z); updateBLECharacteristic(&bleCharacteristic, tempData);
      }
      if (settings.imuLogDMPCpass)
      {
        sprintf(tempData, "%d,%d,%d,", dmpData.Compass.Data.X, dmpData.Compass.Data.Y, dmpData.Compass.Data.Z);
        strcat(outputData, tempData);
        sprintf(tempData, "%d,", dmpData.Compass.Data.X); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%d,", dmpData.Compass.Data.Y); updateBLECharacteristic(&bleCharacteristic, tempData);
        sprintf(tempData, "%d,", dmpData.Compass.Data.Z); updateBLECharacteristic(&bleCharacteristic, tempData);
      }
    }
  }

  //Append all external sensor data on linked list to outputData
  gatherDeviceValues(&bleCharacteristic);

  if (settings.logHertz)
  {
    uint64_t currentMillis;

    //If we are sleeping between readings then we cannot rely on millis() as it is powered down
    //Use RTC instead
    currentMillis = bestMillis();
    float actualRate;
    if ((currentMillis - measurementStartTime) < 1) // Avoid divide by zero
      actualRate = 0.0;
    else
      actualRate = measurementCount * 1000.0 / (currentMillis - measurementStartTime);
    olaftoa(actualRate, tempData1, 3, sizeof(tempData) / sizeof(char));
    sprintf(tempData, "%s,", tempData1);
    strcat(outputData, tempData);
    updateBLECharacteristic(&bleCharacteristic, tempData);
  }

  if (settings.printMeasurementCount)
  {
    sprintf(tempData, "%d,", measurementTotal);
    strcat(outputData, tempData);
    updateBLECharacteristic(&bleCharacteristic, tempData);
  }

  strcat(outputData, "\r\n");

  totalCharactersPrinted += strlen(outputData);
}

//Read values from the devices on the node list
//Append values to outputData
void gatherDeviceValues(int *bleCharacteristic)
{
  char tempData[100];
  char tempData1[20];

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
              olaftoa(currentWeight, tempData1, nodeSetting->decimalPlaces, sizeof(tempData) / sizeof(char));
              sprintf(tempData, "%s,", tempData1);
              strcat(outputData, tempData);
              updateBLECharacteristic(bleCharacteristic, tempData);
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
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logRangeStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getRangeStatus());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logSignalRate)
              {
                sprintf(tempData, "%d,", nodeDevice->getSignalRate());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
            }
          }
          break;
        case DEVICE_GPS_UBLOX:
          {
            setQwiicPullups(0); //Disable pullups to minimize CRC issues

            SFE_UBLOX_GNSS *nodeDevice = (SFE_UBLOX_GNSS *)temp->classPtr;
            struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr;

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
                if (settings.americanDateStyle == true)
                {
                  sprintf(tempData, "%s/%s/%s,", gnssMonthStr, gnssDayStr, gnssYearStr);
                }
                else
                  sprintf(tempData, "%s/%s/%s,", gnssDayStr, gnssMonthStr, gnssYearStr);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPosition)
              {
                sprintf(tempData, "%d,%d,", nodeDevice->getLatitude(), nodeDevice->getLongitude());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logAltitude)
              {
                sprintf(tempData, "%d,", nodeDevice->getAltitude());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logAltitudeMSL)
              {
                sprintf(tempData, "%d,", nodeDevice->getAltitudeMSL());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logSIV)
              {
                sprintf(tempData, "%d,", nodeDevice->getSIV());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logFixType)
              {
                sprintf(tempData, "%d,", nodeDevice->getFixType());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logCarrierSolution)
              {
                sprintf(tempData, "%d,", nodeDevice->getCarrierSolutionType());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logGroundSpeed)
              {
                sprintf(tempData, "%d,", nodeDevice->getGroundSpeed());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logHeadingOfMotion)
              {
                sprintf(tempData, "%d,", nodeDevice->getHeading());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logpDOP)
              {
                sprintf(tempData, "%d,", nodeDevice->getPDOP());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logiTOW)
              {
                sprintf(tempData, "%d,", nodeDevice->getTimeOfWeek());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logAmbientLight)
              {
                sprintf(tempData, "%d,", nodeDevice->getAmbient());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->readTempC(), tempData1, 4, sizeof(tempData) / sizeof(char)); //Resolution to 0.0078°C, accuracy of 0.1°C
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->getPressure(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->getPressure_hPa(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature_degC(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->readFloatPressure(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logHumidity)
              {
                olaftoa(nodeDevice->readFloatHumidity(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logAltitude)
              {
                olaftoa(nodeDevice->readFloatAltitudeMeters(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->readTempC(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->uva(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logUVB)
              {
                olaftoa(nodeDevice->uvb(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logUVIndex)
              {
                olaftoa(nodeDevice->index(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logCO2)
              {
                sprintf(tempData, "%d,", nodeDevice->getCO2());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logCO2)
              {
                sprintf(tempData, "%d,", nodeDevice->CO2);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logH2)
              {
                sprintf(tempData, "%d,", nodeDevice->H2);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logEthanol)
              {
                sprintf(tempData, "%d,", nodeDevice->ethanol);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logHumidity)
              {
                olaftoa(nodeDevice->getHumidity(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->getHumidity(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPressure)
              {
                olaftoa(nodeDevice->getPressure(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->getThermocoupleTemp(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logAmbientTemperature)
              {
                olaftoa(nodeDevice->getAmbientTemp(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->getHumidity(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->getTemperature(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->toPercent(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->toDegC(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->readPT100Centigrade(), tempData1, 3, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logFahrenheit)
              {
                olaftoa(nodeDevice->readPT100Fahrenheit(), tempData1, 3, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logInternalTemperature)
              {
                olaftoa(nodeDevice->readInternalTemperature(), tempData1, 3, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logRawVoltage)
              {
                sprintf(tempData, "%d,", nodeDevice->readRawVoltage());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->readPressure(), tempData1, 4, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->usePA)
              {
                olaftoa(nodeDevice->readPressure(PA), tempData1, 1, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->useKPA)
              {
                olaftoa(nodeDevice->readPressure(KPA), tempData1, 4, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->useTORR)
              {
                olaftoa(nodeDevice->readPressure(TORR), tempData1, 3, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->useINHG)
              {
                olaftoa(nodeDevice->readPressure(INHG), tempData1, 4, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->useATM)
              {
                olaftoa(nodeDevice->readPressure(ATM), tempData1, 6, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->useBAR)
              {
                olaftoa(nodeDevice->readPressure(BAR), tempData1, 6, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->getPM1_0(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPM25)
              {
                olaftoa(nodeDevice->getPM2_5(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPM10)
              {
                olaftoa(nodeDevice->getPM10(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPC05)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC0_5());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPC1)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC1_0());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPC25)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC2_5());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPC50)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC5_0());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPC75)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC7_5());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPC10)
              {
                sprintf(tempData, "%d,", nodeDevice->getPC10());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logSensorStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusSensors());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logPDStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusPD());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logLDStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusLD());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logFanStatus)
              {
                sprintf(tempData, "%d,", nodeDevice->getStatusFan());
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(pressure, tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(temperature, tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(nodeDevice->pressure(nodeSetting->conversion), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->temperature(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logDepth)
              {
                olaftoa(nodeDevice->depth(), tempData1, 3, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logAltitude)
              {
                olaftoa(nodeDevice->altitude(), tempData1, 2, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                olaftoa(((float)pressedPopped) / 1000.0, tempData1, 3, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }

              long clickedPopped = 0;
              while (nodeDevice->isClickedQueueEmpty() == false)
              {
                clickedPopped = nodeDevice->popClickedQueue();
                nodeSetting->ledState ^= 1; // Toggle nodeSetting->ledState on _every_ click (not just the most recent)
              }
              if (nodeSetting->logClicked)
              {
                olaftoa(((float)clickedPopped) / 1000.0, tempData1, 3, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }

              if (nodeSetting->toggleLEDOnClick)
              {
                if (nodeSetting->ledState)
                  nodeDevice->LEDon(nodeSetting->ledBrightness);
                else
                  nodeDevice->LEDoff();
                sprintf(tempData, "%d,", nodeSetting->ledState);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logConfidence)
              {
                sprintf(tempData, "%d,", body.confidence);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logOxygen)
              {
                sprintf(tempData, "%d,", body.oxygen);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logStatus)
              {
                sprintf(tempData, "%d,", body.status);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logExtendedStatus)
              {
                sprintf(tempData, "%d,", body.extStatus);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
              }
              if (nodeSetting->logRValue)
              {
                olaftoa(body.rValue, tempData1, 1, sizeof(tempData) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
                updateBLECharacteristic(bleCharacteristic, tempData);
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
void printHelperText(bool terminalPrint, bool filePrint)
{
  char helperText[1000];
  helperText[0] = '\0';

  if (settings.logRTC)
  {
    if (settings.logDate)
    {
      strcat(helperText, "rtcDate,");
      if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
    }
    if (settings.logTime)
    {
      strcat(helperText, "rtcTime,");
      if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
    }
    if (settings.logMicroseconds)
    {
      strcat(helperText, "micros,");
      if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
    }
  }

  if (settings.logA11)
  {
    strcat(helperText, "analog_11,");
    if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
  }

  if (settings.logA12)
  {
    strcat(helperText, "analog_12,");
    if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
  }

  if (settings.logA13)
  {
    strcat(helperText, "analog_13,");
    if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
  }

  if (settings.logA32)
  {
    strcat(helperText, "analog_32,");
    if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
  }

  if (settings.logVIN)
  {
    strcat(helperText, "VIN,");
    if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
  }

  if (online.IMU)
  {
    if (settings.imuUseDMP == false)
    {
      if (settings.logIMUAccel)
      {
        strcat(helperText, "aX,aY,aZ,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=3;
      }
      if (settings.logIMUGyro)
      {
        strcat(helperText, "gX,gY,gZ,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=3;
      }
      if (settings.logIMUMag)
      {
        strcat(helperText, "mX,mY,mZ,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=3;
      }
      if (settings.logIMUTemp)
      {
        strcat(helperText, "imu_degC,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
      }
    }
    else
    {
      if (settings.imuLogDMPQuat6)
      {
        strcat(helperText, "Q6_1,Q6_2,Q6_3,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=3;
      }
      if (settings.imuLogDMPQuat9)
      {
        strcat(helperText, "Q9_1,Q9_2,Q9_3,HeadAcc,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=4;
      }
      if (settings.imuLogDMPAccel)
      {
        strcat(helperText, "RawAX,RawAY,RawAZ,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=3;
      }
      if (settings.imuLogDMPGyro)
      {
        strcat(helperText, "RawGX,RawGY,RawGZ,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=3;
      }
      if (settings.imuLogDMPCpass)
      {
        strcat(helperText, "RawMX,RawMY,RawMZ,");
        if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics+=3;
      }
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
            {
              strcat(helperText, "weight(no unit),");
              if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
            }
          }
          break;
        case DEVICE_DISTANCE_VL53L1X:
          {
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logDistance)
              {
                strcat(helperText, "distance_mm,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logRangeStatus)
              {
                strcat(helperText, "distance_rangeStatus(0=good),");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logSignalRate)
              {
                strcat(helperText, "distance_signalRate,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_GPS_UBLOX:
          {
            struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logDate)
              {
                strcat(helperText, "gps_Date,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTime)
              {
                strcat(helperText, "gps_Time,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPosition)
              {
                strcat(helperText, "gps_Lat,gps_Long,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logAltitude)
              {
                strcat(helperText, "gps_Alt,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logAltitudeMSL)
              {
                strcat(helperText, "gps_AltMSL,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logSIV)
              {
                strcat(helperText, "gps_SIV,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logFixType)
              {
                strcat(helperText, "gps_FixType,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logCarrierSolution)
              {
                strcat(helperText, "gps_CarrierSolution,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logGroundSpeed)
              {
                strcat(helperText, "gps_GroundSpeed,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logHeadingOfMotion)
              {
                strcat(helperText, "gps_Heading,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logpDOP)
              {
                strcat(helperText, "gps_pDOP,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logiTOW)
              {
                strcat(helperText, "gps_iTOW,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PROXIMITY_VCNL4040:
          {
            struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logProximity)
              {
                strcat(helperText, "prox(no unit),");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logAmbientLight)
              {
                strcat(helperText, "ambient_lux,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_TEMPERATURE_TMP117:
          {
            struct_TMP117 *nodeSetting = (struct_TMP117 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PRESSURE_MS5637:
          {
            struct_MS5637 *nodeSetting = (struct_MS5637 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
              {
                strcat(helperText, "pressure_hPa,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "temperature_degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PRESSURE_LPS25HB:
          {
            struct_LPS25HB *nodeSetting = (struct_LPS25HB *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
              {
                strcat(helperText, "pressure_hPa,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "pressure_degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PHT_BME280:
          {
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
              {
                strcat(helperText, "pressure_Pa,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logHumidity)
              {
                strcat(helperText, "humidity_%,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logAltitude)
              {
                strcat(helperText, "altitude_m,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "temp_degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_UV_VEML6075:
          {
            struct_VEML6075 *nodeSetting = (struct_VEML6075 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logUVA)
              {
                strcat(helperText, "uva,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logUVB)
              {
                strcat(helperText, "uvb,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logUVIndex)
              {
                strcat(helperText, "uvIndex,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_VOC_CCS811:
          {
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTVOC)
              {
                strcat(helperText, "tvoc_ppb,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logCO2)
              {
                strcat(helperText, "co2_ppm,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_VOC_SGP30:
          {
            struct_SGP30 *nodeSetting = (struct_SGP30 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTVOC)
              {
                strcat(helperText, "tvoc_ppb,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logCO2)
              {
                strcat(helperText, "co2_ppm,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logH2)
              {
                strcat(helperText, "H2,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logEthanol)
              {
                strcat(helperText, "ethanol,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_CO2_SCD30:
          {
            struct_SCD30 *nodeSetting = (struct_SCD30 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logCO2)
              {
                strcat(helperText, "co2_ppm,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logHumidity)
              {
                strcat(helperText, "humidity_%,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PHT_MS8607:
          {
            struct_MS8607 *nodeSetting = (struct_MS8607 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
              {
                strcat(helperText, "humidity_%,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPressure)
              {
                strcat(helperText, "hPa,");
                
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_TEMPERATURE_MCP9600:
          {
            struct_MCP9600 *nodeSetting = (struct_MCP9600 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "thermo_degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logAmbientTemperature)
              {
                strcat(helperText, "thermo_ambientDegC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_HUMIDITY_AHT20:
          {
            struct_AHT20 *nodeSetting = (struct_AHT20 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
              {
                strcat(helperText, "humidity_%,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_HUMIDITY_SHTC3:
          {
            struct_SHTC3 *nodeSetting = (struct_SHTC3 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHumidity)
              {
                strcat(helperText, "humidity_%,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_ADC_ADS122C04:
          {
            struct_ADS122C04 *nodeSetting = (struct_ADS122C04 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logCentigrade)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logFahrenheit)
              {
                strcat(helperText, "degF,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logInternalTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logRawVoltage)
              {
                strcat(helperText, "V*2.048/2^23,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PRESSURE_MPR0025PA1:
          {
            struct_MPR0025PA1 *nodeSetting = (struct_MPR0025PA1 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->usePSI)
              {
                strcat(helperText, "PSI,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->usePA)
              {
                strcat(helperText, "Pa,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->useKPA)
              {
                strcat(helperText, "kPa,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->useTORR)
              {
                strcat(helperText, "torr,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->useINHG)
              {
                strcat(helperText, "inHg,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->useATM)
              {
                strcat(helperText, "atm,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->useBAR)
              {
                strcat(helperText, "bar,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PARTICLE_SNGCJA5:
          {
            struct_SNGCJA5 *nodeSetting = (struct_SNGCJA5 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPM1)
              {
                strcat(helperText, "PM1_0,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPM25)
              {
                strcat(helperText, "PM2_5,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPM10)
              {
                strcat(helperText, "PM10,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPC05)
              {
                strcat(helperText, "PC0_5,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPC1)
              {
                strcat(helperText, "PC1_0,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPC25)
              {
                strcat(helperText, "PC2_5,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPC50)
              {
                strcat(helperText, "PC5_0,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPC75)
              {
                strcat(helperText, "PC7_5,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPC10)
              {
                strcat(helperText, "PC10,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logSensorStatus)
              {
                strcat(helperText, "Sensors,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logPDStatus)
              {
                strcat(helperText, "PD,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logLDStatus)
              {
                strcat(helperText, "LD,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logFanStatus)
              {
                strcat(helperText, "Fan,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_VOC_SGP40:
          {
            struct_SGP40 *nodeSetting = (struct_SGP40 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logVOC)
              {
                strcat(helperText, "VOCindex,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PRESSURE_SDP3X:
          {
            struct_SDP3X *nodeSetting = (struct_SDP3X *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
              {
                strcat(helperText, "Pa,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_PRESSURE_MS5837:
          {
            struct_MS5837 *nodeSetting = (struct_MS5837 *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressure)
              {
                strcat(helperText, "mbar,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logTemperature)
              {
                strcat(helperText, "degC,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logDepth)
              {
                strcat(helperText, "depth_m,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logAltitude)
              {
                strcat(helperText, "alt_m,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_QWIIC_BUTTON:
          {
            struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logPressed)
              {
                strcat(helperText, "pressS,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logClicked)
              {
                strcat(helperText, "clickS,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->toggleLEDOnClick)
              {
                strcat(helperText, "LED,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
            }
          }
          break;
        case DEVICE_BIO_SENSOR_HUB:
          {
            struct_BIO_SENSOR_HUB *nodeSetting = (struct_BIO_SENSOR_HUB *)temp->configPtr;
            if (nodeSetting->log)
            {
              if (nodeSetting->logHeartrate)
              {
                strcat(helperText, "bpm,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logConfidence)
              {
                strcat(helperText, "conf%,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logOxygen)
              {
                strcat(helperText, "O2%,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logStatus)
              {
                strcat(helperText, "stat,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logExtendedStatus)
              {
                strcat(helperText, "eStat,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
              if (nodeSetting->logRValue)
              {
                strcat(helperText, "O2R,");
                if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
              }
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
  {
    strcat(helperText, "output_Hz,");
    if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
  }

  if (settings.printMeasurementCount)
  {
    strcat(helperText, "count,");
    if (numBLECharacteristics < kMaxCharacteristics) numBLECharacteristics++;
  }

  strcat(helperText, "\r\n");

  if (terminalPrint)
    SerialPrint(helperText);
  if ((filePrint) && (settings.logData == true) && (online.microSD) && (settings.enableSD && online.microSD))
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
