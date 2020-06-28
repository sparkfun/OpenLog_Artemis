

//Read values from the devices on the node list
void gatherDeviceValues()
{
  char tempData[50];
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
