
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
        case DEVICE_VOC_CCS811:
          {
            CCS811 *nodeDevice = (CCS811 *)temp->classPtr;
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            if (nodeSetting->log == true)
            {
              nodeDevice->readAlgorithmResults();
              if (nodeSetting->logTVOC)
                Serial.printf(" TVOC: %d", nodeDevice->getTVOC());
              if (nodeSetting->logCO2)
                Serial.printf(" CO2: %d", nodeDevice->getCO2());
            }
            break;
          }
        case DEVICE_DISTANCE_VL53L1X:
          {
            SFEVL53L1X *nodeDevice = (SFEVL53L1X *)temp->classPtr;
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does
            if (nodeSetting->log == true)
            {
              if (nodeSetting->logDistance)
                Serial.printf(" Distance: %d", nodeDevice->getDistance());
              if (nodeSetting->logRangeStatus)
                Serial.printf(" RangeStatus: %d", nodeDevice->getRangeStatus());
              if (nodeSetting->logSignalRate)
                Serial.printf(" SignalRate: %d", nodeDevice->getSignalRate());
            }
            break;
        }        default:
          break;
      }

    }
    temp = temp->next;
  }
}
