/*
  getNodePointer(nodeNumber)
  getNodePointer(deviceType, address, mux, port)
  addDevice(address, deviceType)

  getConfigPointer(nodeNumber)
  getConfigPointer(deviceType, address, mux, port)

  getConfigFunctionPtr(nodeNumber)
  getDeviceName(deviceType)

  getNodeNumber(deviceType, address, mux, port)

  configureDevice(nodePointer)
  configureDevice(nodeNumber)
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
    case DEVICE_DISTANCE_VL53L1X:
      return "Distance-VL53L1X";
      break;
    case DEVICE_LOADCELL_NAU7802:
      return "LoadCell-NAU7802";
      break;
    case DEVICE_MULTIPLEXER:
      return "Multiplexer";
      break;
    case DEVICE_VOC_CCS811:
      return "VOC-CCS811";
      break;
    case DEVICE_PHT_BME280:
      return "PHT-BME280";
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
