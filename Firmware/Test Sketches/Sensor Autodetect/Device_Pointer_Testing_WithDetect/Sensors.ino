
//Read values from the devices on the node list
void printDeviceValues()
{
  node *temp = new node;
  temp = head;

  //Step through list, printing values as we go
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
                Serial.printf(" Dist: %d", nodeDevice->getDistance());
              if (nodeSetting->logRangeStatus)
                Serial.printf(" Range: %d", nodeDevice->getRangeStatus());
              if (nodeSetting->logSignalRate)
                Serial.printf(" Signal: %d", nodeDevice->getSignalRate());
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
                Serial.printf(" pres: %.02f", nodeDevice->readFloatPressure());
              if (nodeSetting->logHumidity)
                Serial.printf(" hum: %.02f", nodeDevice->readFloatHumidity());
              if (nodeSetting->logAltitude)
                Serial.printf(" alt: %.02f", nodeDevice->readFloatAltitudeMeters());
              if (nodeSetting->logTemperature)
                Serial.printf(" temp: %.02f", nodeDevice->readTempC());
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
                Serial.printf(" TVOC: %d", nodeDevice->getTVOC());
              if (nodeSetting->logCO2)
                Serial.printf(" CO2: %d", nodeDevice->getCO2());
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
