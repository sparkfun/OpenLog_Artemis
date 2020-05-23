/*

*/

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
  node *temp = head;

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
  node *temp = head;
  while (temp != NULL)
  {
    if (temp->address == address)
      if (temp->muxAddress == muxAddress)
        if (temp->portNumber == portNumber)
          if (temp->deviceType == deviceType)
            return (temp);

    temp = temp->next;
  }
  return (NULL);
}

//Given nodenumber, pull out the device type
deviceType_e getDeviceType(uint8_t nodeNumber)
{
  node *temp = getNodePointer(nodeNumber);
  if (temp == NULL) return (DEVICE_UNKNOWN_DEVICE);
  return (temp->deviceType);
}

//Given nodeNumber, return the config pointer
void *getConfigPointer(uint8_t nodeNumber)
{
  //Search the list of nodes
  node *temp = getNodePointer(nodeNumber);
  if (temp == NULL) return (NULL);
  return (temp->configPtr);
}

//Given a bunch of ID'ing info, return the config pointer to a node
void *getConfigPointer(deviceType_e deviceType, uint8_t address, uint8_t muxAddress, uint8_t portNumber)
{
  //Search the list of nodes
  node *temp = getNodePointer(deviceType, address, muxAddress, portNumber);
  if (temp == NULL) return (NULL);
  return (temp->configPtr);
}

bool addDevice(deviceType_e deviceType, uint8_t address, uint8_t muxAddress, uint8_t portNumber)
{
  //Ignore devices we've already logged. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
  if (deviceExists(deviceType, address, muxAddress, portNumber) == true) return false;

  //Create class instantiation for this device
  //Create logging details for this device

  node *temp = new node; //Create the node in memory

  //Setup this node
  temp->deviceType = deviceType;
  temp->address = address;
  temp->muxAddress = muxAddress;
  temp->portNumber = portNumber;

  //Instantiate a class and settings struct for this device
  //Attempt to begin the device
  switch (deviceType)
  {
    case DEVICE_MULTIPLEXER:
      {
        temp->classPtr = new QWIICMUX; //This allocates the memory needed for this class
        temp->configPtr = new struct_multiplexer;

        QWIICMUX *tempDevice = (QWIICMUX *)temp->classPtr;
        temp->online = tempDevice->begin(address, qwiic); //Address, Wire port
      }
      break;
    case DEVICE_DISTANCE_VL53L1X:
      {
        temp->classPtr = new SFEVL53L1X(qwiic);
        temp->configPtr = new struct_VL53L1X;

        SFEVL53L1X *tempDevice = (SFEVL53L1X *)temp->classPtr;
        struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does
        if (tempDevice->begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
          temp->online = true;
      }
      break;
    case DEVICE_PHT_BME280:
      {
        temp->classPtr = new BME280;
        temp->configPtr = new struct_BME280;

        BME280 *tempDevice = (BME280 *)temp->classPtr;
        tempDevice->setI2CAddress(temp->address);
        temp->online = tempDevice->beginI2C(qwiic); //Wire port
      }
      break;
    case DEVICE_VOC_CCS811:
      {
        temp->classPtr = new CCS811(address);
        temp->configPtr = new struct_CCS811;

        CCS811 *tempDevice = (CCS811 *)temp->classPtr;
        temp->online = tempDevice->begin(qwiic); //Wire port
      }
      break;

    default:
      Serial.printf("addDevice Device type not found: %d\n", deviceType);
      break;
  }

  if (temp->online)
    configureDevice(temp); //Configure this device with the node's config settings

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

  return true;
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
    case DEVICE_PHT_BME280:
      //Nothing to configure
      break;
    case DEVICE_VOC_CCS811:
      //Nothing to configure
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
  if (temp == NULL) return (NULL);
  deviceType_e deviceType = temp->deviceType;

  switch (deviceType)
  {
    case DEVICE_MULTIPLEXER:
      ptr = (FunctionPointer)menuConfigure_Multiplexer;
      break;
    case DEVICE_DISTANCE_VL53L1X:
      ptr = (FunctionPointer)menuConfigure_VL53L1X;
      break;
    case DEVICE_PHT_BME280:
      ptr = (FunctionPointer)menuConfigure_BME280;
      break;
    case DEVICE_VOC_CCS811:
      ptr = (FunctionPointer)menuConfigure_CCS811;
      break;
    default:
      Serial.println("getConfigFunctionPtr: Unknown device type");
      Serial.flush();
      break;
  }

  return (ptr);
}

//Search the linked list for a given address
//Returns true if this device address already exists in our system
bool deviceExists(deviceType_e deviceType, uint8_t address, uint8_t muxAddress, uint8_t portNumber)
{
  node *temp = head;
  while (temp != NULL)
  {
    if (temp->address == address)
      if (temp->muxAddress == muxAddress)
        if (temp->portNumber == portNumber)
          if (temp->deviceType == deviceType) return (true);

    //Devices that were discovered on the main branch will be discovered over and over again
    //If a device has a 0/0 mux/port address, it's on the main branch and exists on all
    //sub branches.
    if (temp->address == address)
      if (temp->muxAddress == 0)
        if (temp->portNumber == 0)
          if (temp->deviceType == deviceType) return (true);

    temp = temp->next;
  }
  return (false);
}

//Given the address of a device, enable muxes appropriately to open connection access device
//Return true if connection was opened
bool openConnection(uint8_t muxAddress, uint8_t portNumber)
{
  if (head == NULL)
  {
    Serial.println("OpenConnection Error: No devices in list");
    return false;
  }

  if (muxAddress == 0) //This device is on main branch, nothing needed
    return true;

  //Get the pointer to the node that contains this mux address
  node *muxNode = getNodePointer(DEVICE_MULTIPLEXER, muxAddress, 0, 0);
  QWIICMUX *myMux = (QWIICMUX *)muxNode->classPtr;

  //Connect to this mux and port
  myMux->setPort(portNumber);

  return (true);
}

//The functions below are specific to the steps of auto-detection rather than node manipulation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Available Qwiic devices
#define ADR_VL53L1X 0x29
#define ADR_CCS811_2 0x5A
#define ADR_CCS811_1 0x5B
#define ADR_MUX_1 0x70 //to 0x77
#define ADR_MUX_2 0x71
#define ADR_BME280_2 0x76
#define ADR_BME280_1 0x77

//Given an address, returns the device type if it responds as we would expect
deviceType_e testDevice(uint8_t i2cAddress, uint8_t muxAddress, uint8_t portNumber)
{
  switch (i2cAddress)
  {
    case 0x29:
      {
        SFEVL53L1X distanceSensor_VL53L1X(qwiic); //Start with given wire port
        if (distanceSensor_VL53L1X.begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
          return (DEVICE_DISTANCE_VL53L1X);
        break;
      }
    case 0x5A:
    case 0x5B:
      {
        CCS811 vocSensor_CCS811(i2cAddress); //Start with given I2C address
        if (vocSensor_CCS811.begin(qwiic) == true) //Wire port
          return (DEVICE_VOC_CCS811);
        break;
      }
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
        break;
      }
    case 0x76:
    case 0x77:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Try a mux first. This will write/read to 0x00 register.
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
        {
          Serial.println("Mux found?");
          return (DEVICE_MULTIPLEXER);
        }

        BME280 phtSensor_BME280;
        phtSensor_BME280.setI2CAddress(i2cAddress);
        if (phtSensor_BME280.beginI2C(qwiic) == true) //Wire port
          return (DEVICE_PHT_BME280);
        break;
      }
    default:
      {
        Serial.printf("-Unknown device at address 0x%02X\n", i2cAddress);
        return DEVICE_UNKNOWN_DEVICE;
        break;
      }
  }
  Serial.printf("-Known I2C address but device responded unexpectedly at address 0x%02X\n", i2cAddress);
  return DEVICE_UNKNOWN_DEVICE;
}

//Print a list of all nodes and their info
void printDetectedDevices()
{
  Serial.println("Detected Device list:");
  Serial.println("Address.MuxAddress.Port#: Device Name[type]");
  Serial.flush();

  node *temp = head;

  if (temp == NULL) Serial.println("-Nothing detected");

  while (temp != NULL)
  {
    Serial.print("-");
    if (temp->online == false) Serial.print("(offline)");
    Serial.printf("0x%02X.0x%02X.%d: %s[%d]\n", temp->address, temp->muxAddress, temp->portNumber, getDeviceName(temp->deviceType), (uint8_t)temp->deviceType);
    Serial.flush();

    temp = temp->next;
  }
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
