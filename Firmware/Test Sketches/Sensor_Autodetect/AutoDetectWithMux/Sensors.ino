

//Read values from the devices on the node list
void gatherDeviceValues()
{
  char tempData[50];
  char tempData1[16];
  char tempData2[16];
  char tempData3[16];
  outputData[0] = '\0'; //Clear string contents

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
        case DEVICE_IMU_ICM20948:
          {
            ICM_20948_I2C *nodeDevice = (ICM_20948_I2C *)temp->classPtr;
            struct_ICM20948 *nodeSetting = (struct_ICM20948 *)temp->configPtr; //Create a local pointer that points to same spot as node does

            if (nodeSetting->log == true)
            {
              openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

              if (nodeDevice->dataReady())
              {
                nodeDevice->getAGMT(); // The values are only updated when you call 'getAGMT'
                if (nodeSetting->logAccel)
                {
                  olaftoa(nodeDevice->accX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                  olaftoa(nodeDevice->accY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
                  olaftoa(nodeDevice->accZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
                  sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
                  strcat(outputData, tempData);
                }
                if (nodeSetting->logGyro)
                {
                  olaftoa(nodeDevice->gyrX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                  olaftoa(nodeDevice->gyrY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
                  olaftoa(nodeDevice->gyrZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
                  sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
                  strcat(outputData, tempData);
                }
                if (nodeSetting->logMag)
                {
                  olaftoa(nodeDevice->magX(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                  olaftoa(nodeDevice->magY(), tempData2, 2, sizeof(tempData2) / sizeof(char));
                  olaftoa(nodeDevice->magZ(), tempData3, 2, sizeof(tempData3) / sizeof(char));
                  sprintf(tempData, "%s,%s,%s,", tempData1, tempData2, tempData3);
                  strcat(outputData, tempData);
                }
                if (nodeSetting->logTemp)
                {
                  olaftoa(nodeDevice->temp(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                  sprintf(tempData, "%s,", tempData1);
                  strcat(outputData, tempData);
                }
              }
            }
          }
          break;
        case DEVICE_DISTANCE_VL53L1X:
          {
            SFEVL53L1X *nodeDevice = (SFEVL53L1X *)temp->classPtr;
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does

            if (nodeSetting->log == true)
            {
              openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

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
        case DEVICE_PHT_BME280:
          {
            BME280 *nodeDevice = (BME280 *)temp->classPtr;
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            if (nodeSetting->log == true)
            {
              openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

              if (nodeSetting->logPressure)
              {
                olaftoa(nodeDevice->readFloatPressure(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logHumidity)
              {
                olaftoa(nodeDevice->readFloatHumidity(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logAltitude)
              {
                olaftoa(nodeDevice->readFloatAltitudeMeters(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
              }
              if (nodeSetting->logTemperature)
              {
                olaftoa(nodeDevice->readTempC(), tempData1, 2, sizeof(tempData1) / sizeof(char));
                sprintf(tempData, "%s,", tempData1);
                strcat(outputData, tempData);
              }
            }
          }
          break;
        case DEVICE_VOC_CCS811:
          {
            CCS811 *nodeDevice = (CCS811 *)temp->classPtr;
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            if (nodeSetting->log == true)
            {
              openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

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
        default:
          Serial.printf("printDeviceValue unknown device type: %s\n", getDeviceName(temp->deviceType));
          break;
      }

    }
    temp = temp->next;
  }
}
