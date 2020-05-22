/*
  Autodetect theory of operation:

  The TCA9548A I2C mux introduces a can of worms but enables multiple (up to 64) of
  a single I2C device to be connected to a given I2C bus. You just have to turn on/off
  a given port while you communicate with said device.

  This is how the autodection algorithm works:
   Scan bus for muxes (0x70 to 0x77)
   Begin() any muxes. This causes them to turn off all their ports.
   With any possible muxes turned off, finish scanning main branch
   Any detected device is stored as a node in a linked list containing their address and device type,
   If muxes are found, begin scanning mux0/port0. Any new device is stored with their address and mux address/port.
   Begin() all devices in our linked list. Connections through muxes are performed as needed.

  All of this works has the side benefit of enabling regular devices, that support multiple address, to
  auto-detect, begin(), and behave as before, but now in multiples.

  Future work:

  Theoretically you could attach 8 muxes configured to 0x71 off the 8 ports of an 0x70 mux. We could
  do this for other muxes as well to create a mux monster:
   - 0x70 - (port 0) 0x71 - 8 ports - device * 8
                     0x72 - 8 ports - device * 8
                     0x73 - 8 ports - device * 8
                     0x74 - 8 ports - device * 8
                     0x75 - 8 ports - device * 8
                     0x76 - 8 ports - device * 8
                     0x77 - 8 ports - device * 8
  This would allow a maximum of 8 * 7 * 8 = 448 of the *same* I2C device address to be
  connected at once. We don't support this sub-muxing right now. So the max we support
  is 64 identical address devices. That should be enough.
*/
//Given node number, get a pointer to the node
node *getNodePointer(uint8_t nodeNumber)
{
  //Search the list of nodes
  node *temp = new node;
  temp = head;

  int counter = 0;
  while (temp != NULL)
  {
    if (nodeNumber == counter)
      return (temp);
    counter++;
    temp = temp->next;
  }

  return (NULL);
}

node *getNodePointer(deviceType_e deviceType, uint8_t address, uint8_t muxAddress, uint8_t portNumber)
{
  //Search the list of nodes
  node *temp = new node;
  temp = head;
  while (temp != NULL)
  {
    if (temp->address == address)
      if (temp->muxAddress == muxAddress)
        if (temp->portNumber == portNumber)
          if (temp->deviceType == deviceType) return (temp);

    temp = temp->next;
  }

  return (NULL);
}

//Given nodenumber, pull out the device type
uint8_t getDeviceType(uint8_t nodeNumber)
{
  node *temp = getNodePointer(nodeNumber);
  return (temp->deviceType);
}

//Given nodeNumber, return the config pointer
void *getConfigPointer(uint8_t nodeNumber)
{
  //Search the list of nodes
  node *temp = getNodePointer(nodeNumber);
  return (temp->configPtr);
}

void addDevice(uint8_t address, deviceType_e deviceType)
{
  //Create class instantiation for this device
  //Create logging details for this device

  node *temp = new node;

  //Setup this node
  temp->address = address;
  temp->portNumber = 0;
  temp->muxAddress = 0;
  temp->deviceType = deviceType;

  //Instantiate a class and settings struct for this device
  //Attempt to begin the device
  switch (deviceType)
  {
    case DEVICE_VOC_CCS811:
      {
        temp->classPtr = new CCS811(address);
        temp->configPtr = new struct_CCS811;

        CCS811 *tempDevice = (CCS811 *)temp->classPtr;
        temp->online = tempDevice->begin(qwiic); //Wire port
        break;
      }
    case DEVICE_DISTANCE_VL53L1X:
      {
        temp->classPtr = new SFEVL53L1X(qwiic);
        temp->configPtr = new struct_VL53L1X;

        SFEVL53L1X *tempDevice = (SFEVL53L1X *)temp->classPtr;
        struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does
        if (tempDevice->begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
        {
          temp->online = true;
          configureDevice(temp); //Configure this device with the node's config settings
        }
        break;
      }
    default:
      Serial.println("addDevice: Device not found");
      break;
  }

  //Link to next node
  temp->next = NULL;
  if (head == NULL)
  {
    head = temp;
    tail = temp;
    temp = NULL;
  }
  else
  {
    tail->next = temp;
    tail = temp;
  }
}

//Given a bunch of ID'ing info, return the config pointer to a node
void *getConfigPointer(deviceType_e deviceType, uint8_t address, uint8_t muxAddress, uint8_t portNumber)
{
  //Search the list of nodes
  node *temp = getNodePointer(deviceType, address, muxAddress, portNumber);
  return (temp->configPtr);
}

//Given the node number, apply the node's configuration settings to the device
void configureDevice(uint8_t nodeNumber)
{
  node *temp = getNodePointer(nodeNumber);
  configureDevice(temp);
}

//Given the node pointer, apply the node's configuration settings to the device
void configureDevice(node *temp)
{
  uint8_t deviceType = temp->deviceType;
  switch (deviceType)
  {
    case DEVICE_VOC_CCS811:
      break;
    case DEVICE_DISTANCE_VL53L1X:
      {
        SFEVL53L1X *tempDevice = (SFEVL53L1X *)temp->classPtr;
        struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does

        if (nodeSetting->distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
          tempDevice->setDistanceModeShort();
        else
          tempDevice->setDistanceModeLong();

        tempDevice->setIntermeasurementPeriod(nodeSetting->intermeasurementPeriod - 1);
        tempDevice->setXTalk(nodeSetting->crosstalk);
        tempDevice->setOffset(nodeSetting->offset);

        tempDevice->startRanging(); //Write configuration bytes to initiate measurement
      }
      break;
    default:
      Serial.println("configureDevice: Unknown device type");
      break;
  }
}

//Returns a pointer to the menu function that configures this particular device type
FunctionPointer getConfigFunctionPtr(uint8_t nodeNumber)
{
  FunctionPointer ptr = NULL;

  node *temp = getNodePointer(nodeNumber);
  deviceType_e deviceType = temp->deviceType;

  switch (deviceType)
  {
    case DEVICE_VOC_CCS811:
      ptr = (FunctionPointer)menuConfigure_CCS811;
      break;
    case DEVICE_DISTANCE_VL53L1X:
      ptr = (FunctionPointer)menuConfigure_VL53L1X;
      break;
    default:
      Serial.println("getConfigFunctionPtr: Unknown device type");
      break;
  }

  return (ptr);
}


//Given a device number return the string associated with that entry
const char* getDeviceName(deviceType_e deviceNumber)
{
  switch (deviceNumber)
  {
    case DEVICE_MULTIPLEXER:
      return "Multiplexer";
      break;
    case DEVICE_LOADCELL_NAU7802:
      return "LoadCell-NAU7802";
      break;
    case DEVICE_DISTANCE_VL53L1X:
      return "Distance-VL53L1X";
      break;
    case DEVICE_GPS_UBLOX:
      return "GPS-Ublox";
      break;
    case DEVICE_PROXIMITY_VCNL4040:
      return "Proximity-VCNL4040";
      break;
    case DEVICE_TEMPERATURE_TMP117:
      return "Temperature-TMP117";
      break;
    case DEVICE_PRESSURE_MS5637:
      return "Pressure-MS5637";
      break;
    case DEVICE_PRESSURE_LPS25HB:
      return "Pressure-LPS25HB";
      break;
    case DEVICE_PHT_BME280:
      return "PHT-BME280";
      break;
    case DEVICE_UV_VEML6075:
      return "UV-VEML6075";
      break;
    case DEVICE_VOC_CCS811:
      return "VOC-CCS811";
      break;
    case DEVICE_VOC_SGP30:
      return "VOC-SGP30";
      break;
    case DEVICE_CO2_SCD30:
      return "CO2-SCD30";
      break;
    case DEVICE_PRESSURE_MS8607:
      return "Pressure-MS8607";
      break;

    case DEVICE_UNKNOWN_DEVICE:
      return "Unknown device";
      break;
    default:
      return "Unknown Status";
      break;
  }
  return "None";
}

//Below is all the functions related to auto-detection rather than node manipulation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
