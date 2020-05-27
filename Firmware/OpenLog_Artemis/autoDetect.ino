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

  All of this works and has the side benefit of enabling regular devices, that support multiple address, to
  auto-detect, begin(), and behave as before, but now in multiples.

  In the case where a device has two I2C address that are used in one library (ex: MS8607) the first address is stored in
  the node list, the 2nd address is ignored.

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

//Add a device to the linked list
//Creates a class but does not begin or configure the device
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
  switch (deviceType)
  {
    case DEVICE_MULTIPLEXER:
      {
        temp->classPtr = new QWIICMUX; //This allocates the memory needed for this class
        temp->configPtr = new struct_multiplexer;
      }
      break;
    case DEVICE_LOADCELL_NAU7802:
      {
        temp->classPtr = new NAU7802;
        temp->configPtr = new struct_NAU7802;
      }
      break;
    case DEVICE_DISTANCE_VL53L1X:
      {
        temp->classPtr = new SFEVL53L1X(qwiic);
        temp->configPtr = new struct_VL53L1X;
      }
      break;
    case DEVICE_GPS_UBLOX:
      {
        temp->classPtr = new SFE_UBLOX_GPS;
        temp->configPtr = new struct_uBlox;
      }
      break;
    case DEVICE_PROXIMITY_VCNL4040:
      {
        temp->classPtr = new VCNL4040;
        temp->configPtr = new struct_VCNL4040;
      }
      break;
    case DEVICE_TEMPERATURE_TMP117:
      {
        temp->classPtr = new TMP117;
        temp->configPtr = new struct_TMP117;
      }
      break;
    case DEVICE_PRESSURE_LPS25HB:
      {
        temp->classPtr = new LPS25HB;
        temp->configPtr = new struct_LPS25HB;
      }
      break;
    case DEVICE_PRESSURE_MS5637:
      {
        temp->classPtr = new MS5637;
        temp->configPtr = new struct_MS5637;
      }
      break;
    case DEVICE_PHT_BME280:
      {
        temp->classPtr = new BME280;
        temp->configPtr = new struct_BME280;
      }
      break;
    case DEVICE_UV_VEML6075:
      {
        temp->classPtr = new VEML6075;
        temp->configPtr = new struct_VEML6075;
      }
      break;
    case DEVICE_VOC_CCS811:
      {
        temp->classPtr = new CCS811(address);
        temp->configPtr = new struct_CCS811;
      }
      break;
    case DEVICE_VOC_SGP30:
      {
        temp->classPtr = new SGP30;
        temp->configPtr = new struct_SGP30;
      }
      break;
    case DEVICE_CO2_SCD30:
      {
        temp->classPtr = new SCD30;
        temp->configPtr = new struct_SCD30;
      }
      break;
    case DEVICE_PHT_MS8607:
      {
        temp->classPtr = new MS8607;
        temp->configPtr = new struct_MS8607;
      }
      break;
    case DEVICE_TEMPERATURE_MCP9600:
      {
        temp->classPtr = new MCP9600;
        temp->configPtr = new struct_MCP9600;
      }
      break;
    case DEVICE_HUMIDITY_AHT20:
      {
        temp->classPtr = new AHT20;
        temp->configPtr = new struct_AHT20;
      }
      break;
    case DEVICE_HUMIDITY_SHTC3:
      {
        temp->classPtr = new SHTC3;
        temp->configPtr = new struct_SHTC3;
      }
      break;
    default:
      Serial.printf("addDevice Device type not found: %d\n", deviceType);
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

  return true;
}

//Begin()'s all devices in the node list
bool beginQwiicDevices()
{
  bool everythingStarted = true;

  //Step through the list
  node *temp = head;

  if (temp == NULL)
  {
    Serial.println("beginDevices: No devices detected");
    return (true);
  }

  while (temp != NULL)
  {
    openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

    //Attempt to begin the device
    switch (temp->deviceType)
    {
      case DEVICE_MULTIPLEXER:
        {
          QWIICMUX *tempDevice = (QWIICMUX *)temp->classPtr;
          temp->online = tempDevice->begin(temp->address, qwiic); //Address, Wire port
        }
        break;
      case DEVICE_LOADCELL_NAU7802:
        {
          NAU7802 *tempDevice = (NAU7802 *)temp->classPtr;
          struct_NAU7802 *nodeSetting = (struct_NAU7802 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_DISTANCE_VL53L1X:
        {
          SFEVL53L1X *tempDevice = (SFEVL53L1X *)temp->classPtr;
          struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (tempDevice->begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
            temp->online = true;
        }
        break;
      case DEVICE_GPS_UBLOX:
        {
          SFE_UBLOX_GPS *tempDevice = (SFE_UBLOX_GPS *)temp->classPtr;
          struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr; //Create a local pointer that points to same spot as node does
          temp->online = tempDevice->begin(qwiic, temp->address); //Wire port, Address
        }
        break;
      case DEVICE_PROXIMITY_VCNL4040:
        {
          VCNL4040 *tempDevice = (VCNL4040 *)temp->classPtr;
          struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_TEMPERATURE_TMP117:
        {
          TMP117 *tempDevice = (TMP117 *)temp->classPtr;
          temp->online = tempDevice->begin(temp->address, qwiic); //Address, Wire port
        }
        break;
      case DEVICE_PRESSURE_LPS25HB:
        {
          LPS25HB *tempDevice = (LPS25HB *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic, temp->address); //Wire port, Address
        }
        break;
      case DEVICE_PRESSURE_MS5637:
        {
          MS5637 *tempDevice = (MS5637 *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_PHT_BME280:
        {
          BME280 *tempDevice = (BME280 *)temp->classPtr;
          tempDevice->setI2CAddress(temp->address);
          temp->online = tempDevice->beginI2C(qwiic); //Wire port
        }
        break;
      case DEVICE_UV_VEML6075:
        {
          VEML6075 *tempDevice = (VEML6075 *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_VOC_CCS811:
        {
          CCS811 *tempDevice = (CCS811 *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_VOC_SGP30:
        {
          SGP30 *tempDevice = (SGP30 *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_CO2_SCD30:
        {
          SCD30 *tempDevice = (SCD30 *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_PHT_MS8607:
        {
          MS8607 *tempDevice = (MS8607 *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_TEMPERATURE_MCP9600:
        {
          MCP9600 *tempDevice = (MCP9600 *)temp->classPtr;
          temp->online = tempDevice->begin(temp->address, qwiic); //Address, Wire port
        }
        break;
      case DEVICE_HUMIDITY_AHT20:
        {
          AHT20 *tempDevice = (AHT20 *)temp->classPtr;
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_HUMIDITY_SHTC3:
        {
          SHTC3 *tempDevice = (SHTC3 *)temp->classPtr;
          if (tempDevice->begin(qwiic) == 0) //Wire port. Returns 0 on success.
            temp->online = true;
        }
        break;
      default:
        Serial.printf("addDevice Device type not found: %d\n", temp->deviceType);
        break;
    }

    if (temp->online == false) everythingStarted = false;

    temp = temp->next;
  }

  return everythingStarted;
}

//Pretty print all the online devices
void printOnlineDevice()
{
  //Step through the list
  node *temp = head;

  if (temp == NULL)
  {
    Serial.println("printOnlineDevice: No devices detected");
    return;
  }

  while (temp != NULL)
  {
    char sensorOnlineText[75];
    if (temp->online)
    {
      if (temp->muxAddress == 0)
        sprintf(sensorOnlineText, "%s online at address 0x%02X\n", getDeviceName(temp->deviceType), temp->address);
      else
        sprintf(sensorOnlineText, "%s online at address 0x%02X.0x%02X.%d\n", getDeviceName(temp->deviceType), temp->address, temp->muxAddress, temp->portNumber);
    }
    else
    {
      sprintf(sensorOnlineText, "%s failed to respond\n", getDeviceName(temp->deviceType));
    }
    Serial.print(sensorOnlineText);

    temp = temp->next;
  }
}

//Given the node number, apply the node's configuration settings to the device
void configureDevice(uint8_t nodeNumber)
{
  node *temp = getNodePointer(nodeNumber);
  configureDevice(temp);
}

//Given the node pointer, apply the node's configuration settings to the device
void configureDevice(node * temp)
{
  uint8_t deviceType = (uint8_t)temp->deviceType;

  openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

  switch (deviceType)
  {
    case DEVICE_MULTIPLEXER:
      //Nothing to configure
      break;
    case DEVICE_LOADCELL_NAU7802:
      {
        NAU7802 *sensor = (NAU7802 *)temp->classPtr;
        struct_NAU7802 *sensorSetting = (struct_NAU7802 *)temp->configPtr;

        sensor->setSampleRate(NAU7802_SPS_320); //Sample rate can be set to 10, 20, 40, 80, or 320Hz
        sensor->setCalibrationFactor(sensorSetting->calibrationFactor);
        sensor->setZeroOffset(sensorSetting->zeroOffset);
      }
      break;
    case DEVICE_DISTANCE_VL53L1X:
      {
        SFEVL53L1X *sensor = (SFEVL53L1X *)temp->classPtr;
        struct_VL53L1X *sensorSetting = (struct_VL53L1X *)temp->configPtr;

        if (sensorSetting->distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
          sensor->setDistanceModeShort();
        else
          sensor->setDistanceModeLong();

        sensor->setIntermeasurementPeriod(sensorSetting->intermeasurementPeriod - 1);
        sensor->setXTalk(sensorSetting->crosstalk);
        sensor->setOffset(sensorSetting->offset);

        sensor->startRanging(); //Write configuration bytes to initiate measurement
      }
      break;
    case DEVICE_GPS_UBLOX:
      {
        SFE_UBLOX_GPS *sensor = (SFE_UBLOX_GPS *)temp->classPtr;
        struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr;

        sensor->setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

        //sensor->setAutoPVT(true); //Tell the GPS to "send" each solution
        sensor->setAutoPVT(false); //We will poll the device for PVT solutions
        if (1000000UL / settings.usBetweenReadings <= 10) //If we are slower than 10Hz logging rate
          sensor->setNavigationFrequency(1000000 / settings.usBetweenReadings); //Set output rate equal to our query rate
        else
          sensor->setNavigationFrequency(10); //Set nav freq to 10Hz. Max output depends on the module used.

        sensor->saveConfiguration(); //Save the current settings to flash and BBR
      }
      break;
    case DEVICE_PROXIMITY_VCNL4040:
      {
        VCNL4040 *sensor = (VCNL4040 *)temp->classPtr;
        struct_VCNL4040 *sensorSetting = (struct_VCNL4040 *)temp->configPtr;

        sensor->powerOnAmbient(); //Turn on ambient sensing
        sensor->setLEDCurrent(sensorSetting->LEDCurrent);
        sensor->setIRDutyCycle(sensorSetting->IRDutyCycle);
        sensor->setProxIntegrationTime(sensorSetting->proximityIntegrationTime);
        sensor->setProxResolution(sensorSetting->resolution);
        sensor->setAmbientIntegrationTime(sensorSetting->ambientIntegrationTime);
      }
      break;
    case DEVICE_TEMPERATURE_TMP117:
      {
        TMP117 *sensor = (TMP117 *)temp->classPtr;
        struct_TMP117 *sensorSetting = (struct_TMP117 *)temp->configPtr;

        sensor->setConversionAverageMode(sensorSetting->conversionAverageMode);
        sensor->setConversionCycleBit(sensorSetting->conversionCycle);
        sensor->setContinuousConversionMode();
      }
      break;
    case DEVICE_PRESSURE_MS5637:
      //Nothing to configure
      break;
    case DEVICE_PRESSURE_LPS25HB:
      //Nothing to configure
      break;
    case DEVICE_PHT_BME280:
      //Nothing to configure
      break;
    case DEVICE_UV_VEML6075:
      //Nothing to configure
      break;
    case DEVICE_VOC_CCS811:
      //Nothing to configure
      break;
    case DEVICE_VOC_SGP30:
      {
        SGP30 *sensor = (SGP30 *)temp->classPtr;
        sensor->initAirQuality(); //Initializes sensor for air quality readings
      }
      break;
    case DEVICE_CO2_SCD30:
      {
        SCD30 *sensor = (SCD30 *)temp->classPtr;
        struct_SCD30 *sensorSetting = (struct_SCD30 *)temp->configPtr;

        sensor->setMeasurementInterval(sensorSetting->measurementInterval);
        sensor->setAltitudeCompensation(sensorSetting->altitudeCompensation);
        sensor->setAmbientPressure(sensorSetting->ambientPressure);
        //sensor->setTemperatureOffset(sensorSetting->temperatureOffset);
      }
      break;
    case DEVICE_PHT_MS8607:
      {
        MS8607 *sensor = (MS8607 *)temp->classPtr;
        struct_MS8607 *sensorSetting = (struct_MS8607 *)temp->configPtr;

        if (sensorSetting->enableHeater == true)
          sensor->enable_heater();
        else
          sensor->disable_heater();

        sensor->set_pressure_resolution(sensorSetting->pressureResolution);
        sensor->set_humidity_resolution(sensorSetting->humidityResolution);
      }
      break;
    case DEVICE_TEMPERATURE_MCP9600:
      {
        MCP9600 *sensor = (MCP9600 *)temp->classPtr;

        //set the resolution on the ambient (cold) junction
        Ambient_Resolution ambientRes = RES_ZERO_POINT_0625; //_25 and _0625
        sensor->setAmbientResolution(ambientRes);

        Thermocouple_Resolution thermocoupleRes = RES_14_BIT; //12, 14, 16, and 18
        sensor->setThermocoupleResolution(thermocoupleRes);
      }
      break;
    case DEVICE_HUMIDITY_AHT20:
      //Nothing to configure
      break;
    case DEVICE_HUMIDITY_SHTC3:
      //Nothing to configure
      break;
    default:
      Serial.printf("configureDevice: Unknown device type %d: %s\n", deviceType, getDeviceName((deviceType_e)deviceType));
      break;
  }
}

//Apply device settings to each node
void configureQwiicDevices()
{
  //Step through the list
  node *temp = head;

  while (temp != NULL)
  {
    configureDevice(temp);
    temp = temp->next;
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
    case DEVICE_LOADCELL_NAU7802:
      ptr = (FunctionPointer)menuConfigure_NAU7802;
      break;
    case DEVICE_DISTANCE_VL53L1X:
      ptr = (FunctionPointer)menuConfigure_VL53L1X;
      break;
    case DEVICE_GPS_UBLOX:
      ptr = (FunctionPointer)menuConfigure_uBlox;
      break;
    case DEVICE_PROXIMITY_VCNL4040:
      ptr = (FunctionPointer)menuConfigure_VCNL4040;
      break;
    case DEVICE_TEMPERATURE_TMP117:
      ptr = (FunctionPointer)menuConfigure_TMP117;
      break;
    case DEVICE_PRESSURE_MS5637:
      ptr = (FunctionPointer)menuConfigure_MS5637;
      break;
    case DEVICE_PRESSURE_LPS25HB:
      ptr = (FunctionPointer)menuConfigure_LPS25HB;
      break;
    case DEVICE_PHT_BME280:
      ptr = (FunctionPointer)menuConfigure_BME280;
      break;
    case DEVICE_UV_VEML6075:
      ptr = (FunctionPointer)menuConfigure_VEML6075;
      break;
    case DEVICE_VOC_CCS811:
      ptr = (FunctionPointer)menuConfigure_CCS811;
      break;
    case DEVICE_VOC_SGP30:
      ptr = (FunctionPointer)menuConfigure_SGP30;
      break;
    case DEVICE_CO2_SCD30:
      ptr = (FunctionPointer)menuConfigure_SCD30;
      break;
    case DEVICE_PHT_MS8607:
      ptr = (FunctionPointer)menuConfigure_MS8607;
      break;
    case DEVICE_TEMPERATURE_MCP9600:
      ptr = (FunctionPointer)menuConfigure_MCP9600;
      break;
    case DEVICE_HUMIDITY_AHT20:
      ptr = (FunctionPointer)menuConfigure_AHT20;
      break;
    case DEVICE_HUMIDITY_SHTC3:
      ptr = (FunctionPointer)menuConfigure_SHTC3;
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

//Bubble sort the given linked list by the device address
//https://www.geeksforgeeks.org/c-program-bubble-sort-linked-list/
void bubbleSortDevices(struct node * start)
{
  int swapped, i;
  struct node *ptr1;
  struct node *lptr = NULL;

  //Checking for empty list
  if (start == NULL) return;

  do
  {
    swapped = 0;
    ptr1 = start;

    while (ptr1->next != lptr)
    {
      if (ptr1->address > ptr1->next->address)
      {
        swap(ptr1, ptr1->next);
        swapped = 1;
      }
      ptr1 = ptr1->next;
    }
    lptr = ptr1;
  }
  while (swapped);
}

//Swap data of two nodes a and b
void swap(struct node * a, struct node * b)
{
  node temp;

  temp.deviceType = a->deviceType;
  temp.address = a->address;
  temp.portNumber = a->portNumber;
  temp.muxAddress = a->muxAddress;
  temp.online = a->online;
  temp.classPtr = a->classPtr;
  temp.configPtr = a->configPtr;

  a->deviceType = b->deviceType;
  a->address = b->address;
  a->portNumber = b->portNumber;
  a->muxAddress = b->muxAddress;
  a->online = b->online;
  a->classPtr = b->classPtr;
  a->configPtr = b->configPtr;

  b->deviceType = temp.deviceType;
  b->address = temp.address;
  b->portNumber = temp.portNumber;
  b->muxAddress = temp.muxAddress;
  b->online = temp.online;
  b->classPtr = temp.classPtr;
  b->configPtr = temp.configPtr;
}

//The functions below are specific to the steps of auto-detection rather than node manipulation
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Available Qwiic devices
//We no longer use defines in the search table. These are just here for reference.
#define ADR_VEML6075 0x10
#define ADR_NAU7802 0x2A
#define ADR_VL53L1X 0x29
#define ADR_AHT20 0x38
#define ADR_MS8607 0x40 //Humidity portion of the MS8607 sensor
#define ADR_UBLOX 0x42 //But can be set to any address
#define ADR_TMP117 0x48 //Alternates: 0x49, 0x4A, and 0x4B
#define ADR_SGP30 0x58
#define ADR_CCS811_2 0x5A
#define ADR_CCS811_1 0x5B
#define ADR_LPS25HB_2 0x5C
#define ADR_LPS25HB_1 0x5D
#define ADR_VCNL4040_OR_MCP9600 0x60
#define ADR_SCD30 0x61
#define ADR_MCP9600_1 0x67 //0x60 to 0x67
#define ADR_MULTIPLEXER 0x70 //0x70 to 0x77
#define ADR_SHTC3 0x70
#define ADR_BME280_2 0x76
#define ADR_MS5637 0x76
//#define ADR_MS8607 0x76 //Pressure portion of the MS8607 sensor. We'll catch the 0x40 first
#define ADR_BME280_1 0x77

//Given an address, returns the device type if it responds as we would expect
deviceType_e testDevice(uint8_t i2cAddress, uint8_t muxAddress, uint8_t portNumber)
{
  switch (i2cAddress)
  {
    case 0x10:
      {
        //Confidence: High - Checks ID
        VEML6075 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_UV_VEML6075);
      }
      break;
    case 0x2A:
      {
        //Confidence: High - Checks 8 bit revision code (0x0F)
        NAU7802 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          if (sensor.getRevisionCode() == 0x0F)
            return (DEVICE_LOADCELL_NAU7802);
      }
      break;
    case 0x29:
      {
        //Confidence: High - Checks 16 bit ID
        SFEVL53L1X sensor(qwiic); //Start with given wire port
        if (sensor.begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
          return (DEVICE_DISTANCE_VL53L1X);
      }
      break;
    case 0x38:
      {
        //Confidence: Medium - begin() does a variety of inits and checks
        AHT20 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_HUMIDITY_AHT20);
      }
      break;
    case 0x40:
      {
        //Humidity portion of the MS8607 sensor
        //Confidence: High - does CRC on internal EEPROM read
        MS8607 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_PHT_MS8607);
      }
      break;
    case 0x42:
      {
        //Confidence: High - Sends/receives CRC checked data response
      
        qwiic.setPullups(0); //Disable pullups to minimize CRC issues
        SFE_UBLOX_GPS sensor;
        if (sensor.begin(qwiic, i2cAddress) == true) //Wire port, address
          return (DEVICE_GPS_UBLOX);
        qwiic.setPullups(1); //Re-enable pullups for normal discovery
      }
      break;
    case 0x48:
      {
        //Confidence: High - Checks 16 bit ID
        TMP117 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_TMP117);
      }
      break;
    case 0x49:
      {
        //Confidence: High - Checks 16 bit ID
        TMP117 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_TMP117);
      }
      break;
    case 0x4A:
      {
        //Confidence: High - Checks 16 bit ID
        TMP117 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_TMP117);
      }
      break;
    case 0x4B:
      {
        //Confidence: High - Checks 16 bit ID
        TMP117 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_TMP117);
      }
      break;
    case 0x58:
      {
        SGP30 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_VOC_SGP30);
      }
      break;
    case 0x5A:
    case 0x5B:
      {
        CCS811 sensor(i2cAddress); //Start with given I2C address
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_VOC_CCS811);
      }
      break;
    case 0x5C:
    case 0x5D:
      {
        LPS25HB sensor;
        if (sensor.begin(qwiic, i2cAddress) == true) //Wire port, address
          return (DEVICE_PRESSURE_LPS25HB);
      }
      break;
    case 0x60:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);

        //Confidence: High - Checks ID
        VCNL4040 sensor1;
        if (sensor1.begin(qwiic) == true) //Wire port
          return (DEVICE_PROXIMITY_VCNL4040);
      }
      break;
    case 0x61:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);

        //Confidence: Medium - Begin doesn't confirm anything, but a readMeasurement() must pass CRC check
        SCD30 sensor1;
        if (sensor1.begin(qwiic) == true) //Wire port
          if (sensor1.readMeasurement() == true) //This reads the measurement register and calculates a CRC on the interchange
            return (DEVICE_CO2_SCD30);
      }
      break;
    case 0x62:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x63:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x64:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x65:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x66:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x67:
      {
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x70:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: High - 16 bit ID check with CRC
        SHTC3 sensor;
        if (sensor.begin(qwiic) == 0) //Wire port. Device returns 0 upon success.
          return (DEVICE_HUMIDITY_SHTC3);

        //Confidence: Medium - Write/Read/Clear to 0x00
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x71:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: Medium - Write/Read/Clear to 0x00
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x72:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: Medium - Write/Read/Clear to 0x00
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x73:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: Medium - Write/Read/Clear to 0x00
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x74:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: Medium - Write/Read/Clear to 0x00
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x75:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: Medium - Write/Read/Clear to 0x00
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x76:
      {
        //Confidence: High - does CRC on internal EEPROM read
        MS5637 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_PRESSURE_MS5637);

        //Confidence: High - ID check but may pass with BMP280
        BME280 sensor1;
        sensor1.setI2CAddress(i2cAddress);
        if (sensor1.beginI2C(qwiic) == true) //Wire port
          return (DEVICE_PHT_BME280);

        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: Medium - Write/Read/Clear to 0x00
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);

        //Pressure portion of the MS8607 combo sensor. We'll catch the 0x40 first
        //By the time we hit this address, MS8607 should have already been started by its first address
        //Since we don't need to harvest this address, this will cause this extra I2C address to be ignored/not added to node list, and not printed.
        return (DEVICE_UNKNOWN_DEVICE);
      }
      break;
    case 0x77:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);

        BME280 sensor;
        sensor.setI2CAddress(i2cAddress);
        if (sensor.beginI2C(qwiic) == true) //Wire port
          return (DEVICE_PHT_BME280);
      }
      break;
    default:
      {
        if (muxAddress == 0)
          Serial.printf("Unknown device at address (0x%02X)\n", i2cAddress);
        else
          Serial.printf("Unknown device at address (0x%02X)(Mux:0x%02X Port:%d)\n", i2cAddress, muxAddress, portNumber);
        return DEVICE_UNKNOWN_DEVICE;
      }
      break;
  }
  Serial.printf("Known I2C address but device failed identification at address 0x%02X\n", i2cAddress);
  return DEVICE_UNKNOWN_DEVICE;
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
      return "GPS-ublox";
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
    case DEVICE_PHT_MS8607:
      return "PHT-MS8607";
      break;
    case DEVICE_TEMPERATURE_MCP9600:
      return "Temperature-MCP9600";
      break;
    case DEVICE_HUMIDITY_AHT20:
      return "Humidity-AHT20";
      break;
    case DEVICE_HUMIDITY_SHTC3:
      return "Humidity-SHTC3";
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
