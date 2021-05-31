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
        temp->classPtr = new SFE_UBLOX_GNSS;
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
    case DEVICE_ADC_ADS122C04:
      {
        temp->classPtr = new SFE_ADS122C04;
        temp->configPtr = new struct_ADS122C04;
      }
      break;
    case DEVICE_PRESSURE_MPR0025PA1:
      {
        temp->classPtr = new SparkFun_MicroPressure;
        temp->configPtr = new struct_MPR0025PA1;
      }
      break;
    case DEVICE_PARTICLE_SNGCJA5:
      {
        temp->classPtr = new SFE_PARTICLE_SENSOR;
        temp->configPtr = new struct_SNGCJA5;
      }
      break;
    case DEVICE_VOC_SGP40:
      {
        temp->classPtr = new SGP40;
        temp->configPtr = new struct_SGP40;
      }
      break;
    case DEVICE_PRESSURE_SDP3X:
      {
        temp->classPtr = new SDP3X;
        temp->configPtr = new struct_SDP3X;
      }
      break;
    case DEVICE_PRESSURE_MS5837:
      {
        temp->classPtr = new MS5837;
        temp->configPtr = new struct_MS5837;
      }
      break;
//    case DEVICE_QWIIC_BUTTON:
//      {
//        temp->classPtr = new QwiicButton;
//        temp->configPtr = new struct_QWIIC_BUTTON;
//      }
//      break;
    case DEVICE_BIO_SENSOR_HUB:
      {
        temp->classPtr = new SparkFun_Bio_Sensor_Hub(32, 11, address); // Reset pin is 32, MFIO pin is 11
        temp->configPtr = new struct_BIO_SENSOR_HUB;
      }
      break;
    default:
      SerialPrintf2("addDevice Device type not found: %d\r\n", deviceType);
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

  waitForQwiicBusPowerDelay(); // Wait while the qwiic devices power up - if required
  
  qwiicPowerOnDelayMillis = settings.qwiicBusPowerUpDelayMs; // Set qwiicPowerOnDelayMillis to the _minimum_ defined by settings.qwiicBusPowerUpDelayMs. It will be increased if required.

  int numberOfSCD30s = 0; // Keep track of how many SCD30s we begin so we can delay before starting the second and subsequent ones

  //Step through the list
  node *temp = head;

  if (temp == NULL)
  {
    printDebug(F("beginQwiicDevices: No devices detected\r\n"));
    return (true);
  }

  while (temp != NULL)
  {
    openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed
    
    if (settings.printDebugMessages == true)
    {
      SerialPrintf2("beginQwiicDevices: attempting to begin deviceType %s", getDeviceName(temp->deviceType));
      SerialPrintf4(" at address 0x%02X using mux address 0x%02X and port number %d\r\n", temp->address, temp->muxAddress, temp->portNumber);
    }
    
    //Attempt to begin the device
    switch (temp->deviceType)
    {
      case DEVICE_MULTIPLEXER:
        {
          QWIICMUX *tempDevice = (QWIICMUX *)temp->classPtr;
          struct_multiplexer *nodeSetting = (struct_multiplexer *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(temp->address, qwiic); //Address, Wire port
        }
        break;
      case DEVICE_LOADCELL_NAU7802:
        {
          NAU7802 *tempDevice = (NAU7802 *)temp->classPtr;
          struct_NAU7802 *nodeSetting = (struct_NAU7802 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_DISTANCE_VL53L1X:
        {
          SFEVL53L1X *tempDevice = (SFEVL53L1X *)temp->classPtr;
          struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
            temp->online = true;
        }
        break;
      case DEVICE_GPS_UBLOX:
        {
          qwiic.setPullups(0); //Disable pullups for u-blox comms.
          SFE_UBLOX_GNSS *tempDevice = (SFE_UBLOX_GNSS *)temp->classPtr;
          struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic, temp->address); //Wire port, Address
          qwiic.setPullups(settings.qwiicBusPullUps); //Re-enable pullups.
        }
        break;
      case DEVICE_PROXIMITY_VCNL4040:
        {
          VCNL4040 *tempDevice = (VCNL4040 *)temp->classPtr;
          struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_TEMPERATURE_TMP117:
        {
          TMP117 *tempDevice = (TMP117 *)temp->classPtr;
          struct_TMP117 *nodeSetting = (struct_TMP117 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(temp->address, qwiic); //Address, Wire port
        }
        break;
      case DEVICE_PRESSURE_LPS25HB:
        {
          LPS25HB *tempDevice = (LPS25HB *)temp->classPtr;
          struct_LPS25HB *nodeSetting = (struct_LPS25HB *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic, temp->address); //Wire port, Address
        }
        break;
      case DEVICE_PRESSURE_MS5637:
        {
          MS5637 *tempDevice = (MS5637 *)temp->classPtr;
          struct_MS5637 *nodeSetting = (struct_MS5637 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_PHT_BME280:
        {
          BME280 *tempDevice = (BME280 *)temp->classPtr;
          struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          tempDevice->setI2CAddress(temp->address);
          temp->online = tempDevice->beginI2C(qwiic); //Wire port
        }
        break;
      case DEVICE_UV_VEML6075:
        {
          VEML6075 *tempDevice = (VEML6075 *)temp->classPtr;
          struct_VEML6075 *nodeSetting = (struct_VEML6075 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_VOC_CCS811:
        {
          CCS811 *tempDevice = (CCS811 *)temp->classPtr;
          struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_VOC_SGP30:
        {
          SGP30 *tempDevice = (SGP30 *)temp->classPtr;
          struct_SGP30 *nodeSetting = (struct_SGP30 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_CO2_SCD30:
        {
          SCD30 *tempDevice = (SCD30 *)temp->classPtr;
          struct_SCD30 *nodeSetting = (struct_SCD30 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          numberOfSCD30s++; // Keep track of how many SCD30s we begin
          // Delay before starting the second and subsequent SCD30s to try and stagger the measurements and the peak current draw
          if (numberOfSCD30s >= 2)
          {
            printDebug(F("beginQwiicDevices: found more than one SCD30. Delaying for 375ms to stagger the peak current draw...\r\n"));
            delay(375);
          }
          if(settings.printDebugMessages == true) tempDevice->enableDebugging(); // Enable debug messages if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_PHT_MS8607:
        {
          MS8607 *tempDevice = (MS8607 *)temp->classPtr;
          struct_MS8607 *nodeSetting = (struct_MS8607 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_TEMPERATURE_MCP9600:
        {
          MCP9600 *tempDevice = (MCP9600 *)temp->classPtr;
          struct_MCP9600 *nodeSetting = (struct_MCP9600 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(temp->address, qwiic); //Address, Wire port
        }
        break;
      case DEVICE_HUMIDITY_AHT20:
        {
          AHT20 *tempDevice = (AHT20 *)temp->classPtr;
          struct_AHT20 *nodeSetting = (struct_AHT20 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          temp->online = tempDevice->begin(qwiic); //Wire port
        }
        break;
      case DEVICE_HUMIDITY_SHTC3:
        {
          SHTC3 *tempDevice = (SHTC3 *)temp->classPtr;
          struct_SHTC3 *nodeSetting = (struct_SHTC3 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin(qwiic) == 0) //Wire port. Returns 0 on success.
            temp->online = true;
        }
        break;
      case DEVICE_ADC_ADS122C04:
        {
          SFE_ADS122C04 *tempDevice = (SFE_ADS122C04 *)temp->classPtr;
          struct_ADS122C04 *nodeSetting = (struct_ADS122C04 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin(temp->address, qwiic) == true) //Address, Wire port. Returns true on success.
            temp->online = true;
        }
        break;
      case DEVICE_PRESSURE_MPR0025PA1:
        {
          // TO DO: Figure out how to pass minimumPSI and maximumPSI when instantiating the sensor. Maybe add an update-_minPsi-and-_maxPsi function to the library?
          SparkFun_MicroPressure *tempDevice = (SparkFun_MicroPressure *)temp->classPtr;
          struct_MPR0025PA1 *nodeSetting = (struct_MPR0025PA1 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin(temp->address, qwiic) == true) //Address, Wire port. Returns true on success.
            temp->online = true;
        }
        break;
      case DEVICE_PARTICLE_SNGCJA5:
        {
          SFE_PARTICLE_SENSOR *tempDevice = (SFE_PARTICLE_SENSOR *)temp->classPtr;
          struct_SNGCJA5 *nodeSetting = (struct_SNGCJA5 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin(qwiic) == true) //Wire port. Returns true on success.
            temp->online = true;
        }
        break;
      case DEVICE_VOC_SGP40:
        {
          SGP40 *tempDevice = (SGP40 *)temp->classPtr;
          struct_SGP40 *nodeSetting = (struct_SGP40 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin(qwiic) == true) //Wire port. Returns true on success.
            temp->online = true;
        }
        break;
      case DEVICE_PRESSURE_SDP3X:
        {
          SDP3X *tempDevice = (SDP3X *)temp->classPtr;
          struct_SDP3X *nodeSetting = (struct_SDP3X *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          tempDevice->stopContinuousMeasurement(temp->address, qwiic); //Make sure continuous measurements are stopped or .begin will fail
          if (tempDevice->begin(temp->address, qwiic) == true) //Address, Wire port. Returns true on success.
            temp->online = true;
        }
        break;
      case DEVICE_PRESSURE_MS5837:
        {
          MS5837 *tempDevice = (MS5837 *)temp->classPtr;
          struct_MS5837 *nodeSetting = (struct_MS5837 *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin(qwiic) == true) //Wire port. Returns true on success.
            temp->online = true;
        }
        break;
//      case DEVICE_QWIIC_BUTTON:
//        {
//          QwiicButton *tempDevice = (QwiicButton *)temp->classPtr;
//          struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)temp->configPtr; //Create a local pointer that points to same spot as node does
//          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
//          if (tempDevice->begin(temp->address, qwiic) == true) //Address, Wire port. Returns true on success.
//            temp->online = true;
//        }
//        break;
      case DEVICE_BIO_SENSOR_HUB:
        {
          SparkFun_Bio_Sensor_Hub *tempDevice = (SparkFun_Bio_Sensor_Hub *)temp->classPtr;
          struct_BIO_SENSOR_HUB *nodeSetting = (struct_BIO_SENSOR_HUB *)temp->configPtr; //Create a local pointer that points to same spot as node does
          if (nodeSetting->powerOnDelayMillis > qwiicPowerOnDelayMillis) qwiicPowerOnDelayMillis = nodeSetting->powerOnDelayMillis; // Increase qwiicPowerOnDelayMillis if required
          if (tempDevice->begin(qwiic) == 0x00) //Wire port. Returns 0x00 on success.
            temp->online = true;
        }
        break;
      default:
        SerialPrintf2("beginQwiicDevices: device type not found: %d\r\n", temp->deviceType);
        break;
    }

    if (temp->online == true)
    {
      printDebug(F("beginQwiicDevices: device is online\r\n"));
    }
    else
    {
      printDebug(F("beginQwiicDevices: device is **NOT** online\r\n"));
      everythingStarted = false;
    }

    temp = temp->next;
  }

  return everythingStarted;
}

//Pretty print all the online devices
int printOnlineDevice()
{
  int deviceCount = 0;

  //Step through the list
  node *temp = head;

  if (temp == NULL)
  {
    printDebug(F("printOnlineDevice: No devices detected\r\n"));
    return (0);
  }

  while (temp != NULL)
  {
    char sensorOnlineText[75];
    if (temp->online)
    {
      if (temp->muxAddress == 0)
        sprintf(sensorOnlineText, "%s online at address 0x%02X\r\n", getDeviceName(temp->deviceType), temp->address);
      else
        sprintf(sensorOnlineText, "%s online at address 0x%02X.0x%02X.%d\r\n", getDeviceName(temp->deviceType), temp->address, temp->muxAddress, temp->portNumber);

      deviceCount++;
    }
    else
    {
      sprintf(sensorOnlineText, "%s failed to respond\r\n", getDeviceName(temp->deviceType));
    }
    SerialPrint(sensorOnlineText);

    temp = temp->next;
  }

  if (settings.printDebugMessages == true)
  {
    SerialPrintf2("Device count: %d\r\n", deviceCount);
  }

  return (deviceCount);
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
        qwiic.setPullups(0); //Disable pullups for u-blox comms.

        SFE_UBLOX_GNSS *sensor = (SFE_UBLOX_GNSS *)temp->classPtr;
        struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr;

        sensor->setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)

        sensor->saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the current ioPortsettings to flash and BBR

        sensor->setAutoPVT(nodeSetting->useAutoPVT); // Use autoPVT as required
        
        if (1000000ULL / settings.usBetweenReadings <= 1) //If we are slower than 1Hz logging rate
          // setNavigationFrequency expects a uint8_t to define the number of updates per second
          // So the slowest rate we can set with setNavigationFrequency is 1Hz
          // (Whereas UBX_CFG_RATE can actually support intervals as slow as 65535ms)
          sensor->setNavigationFrequency(1); //Set output rate to 1Hz
        else if (1000000ULL / settings.usBetweenReadings <= 10) //If we are slower than 10Hz logging rate
          sensor->setNavigationFrequency((uint8_t)(1000000ULL / settings.usBetweenReadings)); //Set output rate equal to our query rate
        else
          sensor->setNavigationFrequency(10); //Set nav freq to 10Hz. Max output depends on the module used.

        qwiic.setPullups(settings.qwiicBusPullUps); //Re-enable pullups.
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
    case DEVICE_ADC_ADS122C04:
      {
        SFE_ADS122C04 *sensor = (SFE_ADS122C04 *)temp->classPtr;
        struct_ADS122C04 *sensorSetting = (struct_ADS122C04 *)temp->configPtr;

        //Configure the wite mode for readPT100Centigrade and readPT100Fahrenheit
        //(readInternalTemperature and readRawVoltage change and restore the mode automatically)
        if (sensorSetting->useFourWireMode)
          sensor->configureADCmode(ADS122C04_4WIRE_MODE);
        else if (sensorSetting->useThreeWireMode)
          sensor->configureADCmode(ADS122C04_3WIRE_MODE);
        else if (sensorSetting->useTwoWireMode)
          sensor->configureADCmode(ADS122C04_2WIRE_MODE);
        else if (sensorSetting->useFourWireHighTemperatureMode)
          sensor->configureADCmode(ADS122C04_4WIRE_HI_TEMP);
        else if (sensorSetting->useThreeWireHighTemperatureMode)
          sensor->configureADCmode(ADS122C04_3WIRE_HI_TEMP);
        else if (sensorSetting->useTwoWireHighTemperatureMode)
          sensor->configureADCmode(ADS122C04_2WIRE_HI_TEMP);
      }
      break;
    case DEVICE_PRESSURE_MPR0025PA1:
      //Nothing to configure
      break;
    case DEVICE_PARTICLE_SNGCJA5:
      //Nothing to configure
      break;
    case DEVICE_VOC_SGP40:
      //Nothing to configure
      break;
    case DEVICE_PRESSURE_SDP3X:
      {
        SDP3X *sensor = (SDP3X *)temp->classPtr;
        struct_SDP3X *sensorSetting = (struct_SDP3X *)temp->configPtr;

        // Each triggered measurement takes 45ms to complete so we need to use continuous measurements
        sensor->stopContinuousMeasurement(); //Make sure continuous measurements are stopped or startContinuousMeasurement will fail
        sensor->startContinuousMeasurement(sensorSetting->massFlow, sensorSetting->averaging); //Request continuous measurements
      }
      break;
    case DEVICE_PRESSURE_MS5837:
      {
        MS5837 *sensor = (MS5837 *)temp->classPtr;
        struct_MS5837 *sensorSetting = (struct_MS5837 *)temp->configPtr;

        //sensor->setModel(sensorSetting->model); // We could override the sensor model, but let's not...
        sensorSetting->model = sensor->getModel();
        sensor->setFluidDensity(sensorSetting->fluidDensity);
      }
      break;
//    case DEVICE_QWIIC_BUTTON:
//      {
//        QwiicButton *sensor = (QwiicButton *)temp->classPtr;
//        struct_QWIIC_BUTTON *sensorSetting = (struct_QWIIC_BUTTON *)temp->configPtr;
//
//        if (sensorSetting->ledState)
//          sensor->LEDon(sensorSetting->ledBrightness);
//        else
//          sensor->LEDoff();
//      }
//      break
    case DEVICE_BIO_SENSOR_HUB:
      {
        SparkFun_Bio_Sensor_Hub *sensor = (SparkFun_Bio_Sensor_Hub *)temp->classPtr;
        struct_BIO_SENSOR_HUB *sensorSetting = (struct_BIO_SENSOR_HUB *)temp->configPtr;

        sensor->configBpm(MODE_TWO); // MODE_TWO provides the oxygen R value
      }
      break;
    default:
      SerialPrintf3("configureDevice: Unknown device type %d: %s\r\n", deviceType, getDeviceName((deviceType_e)deviceType));
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
    case DEVICE_ADC_ADS122C04:
      ptr = (FunctionPointer)menuConfigure_ADS122C04;
      break;
    case DEVICE_PRESSURE_MPR0025PA1:
      ptr = (FunctionPointer)menuConfigure_MPR0025PA1;
      break;
    case DEVICE_PARTICLE_SNGCJA5:
      ptr = (FunctionPointer)menuConfigure_SNGCJA5;
      break;
    case DEVICE_VOC_SGP40:
      ptr = (FunctionPointer)menuConfigure_SGP40;
      break;
    case DEVICE_PRESSURE_SDP3X:
      ptr = (FunctionPointer)menuConfigure_SDP3X;
      break;
    case DEVICE_PRESSURE_MS5837:
      ptr = (FunctionPointer)menuConfigure_MS5837;
      break;
//    case DEVICE_QWIIC_BUTTON:
//      ptr = (FunctionPointer)menuConfigure_QWIIC_BUTTON;
//      break;
    case DEVICE_BIO_SENSOR_HUB:
      ptr = (FunctionPointer)menuConfigure_BIO_SENSOR_HUB;
      break;
    default:
      SerialPrintln(F("getConfigFunctionPtr: Unknown device type"));
      SerialFlush();
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
        {
          if (temp->deviceType == deviceType) return (true);
          // Use DEVICE_TOTAL_DEVICES as a special case.
          // Return true if the device address exists on the main branch so we can avoid looking for it on mux branches.
          if (deviceType == DEVICE_TOTAL_DEVICES) return (true);
        }

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
    SerialPrintln(F("OpenConnection Error: No devices in list"));
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
#define ADR_MPR0025PA1 0x18
#define ADR_SDP3X 0x21 //Alternates: 0x22, 0x23
#define ADR_NAU7802 0x2A
#define ADR_VL53L1X 0x29
#define ADR_SNGCJA5 0x33
#define ADR_AHT20 0x38
#define ADR_MS8607 0x40 //Humidity portion of the MS8607 sensor
#define ADR_UBLOX 0x42 //But can be set to any address
#define ADR_ADS122C04 0x45 //Alternates: 0x44, 0x41 and 0x40
#define ADR_TMP117 0x48 //Alternates: 0x49, 0x4A, and 0x4B
#define ADR_BIO_SENSOR_HUB 0x55
#define ADR_SGP30 0x58
#define ADR_SGP40 0x59
#define ADR_CCS811 0x5B //Alternates: 0x5A
#define ADR_LPS25HB 0x5D //Alternates: 0x5C
#define ADR_VCNL4040 0x60
#define ADR_SCD30 0x61
#define ADR_MCP9600 0x60 //0x60 to 0x67
//#define ADR_QWIIC_BUTTON 0x6F //But can be any address... Limit the range to 0x68-0x6F
#define ADR_MULTIPLEXER 0x70 //0x70 to 0x77
#define ADR_SHTC3 0x70
#define ADR_MS5637 0x76
#define ADR_MS5837 0x76
//#define ADR_MS8607 0x76 //Pressure portion of the MS8607 sensor. We'll catch the 0x40 first
#define ADR_BME280 0x77 //Alternates: 0x76

//Given an address, returns the device type if it responds as we would expect
//Does not test for multiplexers. See testMuxDevice for dedicated mux testing.
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
    case 0x18:
      {
        //Confidence: Medium - Checks the status byte power indication bit and three "always 0" bits
        SparkFun_MicroPressure sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          if ((sensor.readStatus() & 0x5A) == 0x40) // Mask the power indication bit and three "always 0" bits
            return (DEVICE_PRESSURE_MPR0025PA1);
      }
      break;
    case 0x21:
      {
        //Confidence: High - .begin reads the product ID
        SDP3X sensor;
        sensor.stopContinuousMeasurement(i2cAddress, qwiic); //Make sure continuous measurements are stopped or .begin will fail
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_PRESSURE_SDP3X);
      }
      break;
    case 0x22:
      {
        //Confidence: High - .begin reads the product ID
        SDP3X sensor;
        sensor.stopContinuousMeasurement(i2cAddress, qwiic); //Make sure continuous measurements are stopped or .begin will fail
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_PRESSURE_SDP3X);
      }
      break;
    case 0x23:
      {
        //Confidence: High - .begin reads the product ID
        SDP3X sensor;
        sensor.stopContinuousMeasurement(i2cAddress, qwiic); //Make sure continuous measurements are stopped or .begin will fail
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_PRESSURE_SDP3X);
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
    case 0x33:
      {
        //Confidence: low - basic isConnected test only...
        SFE_PARTICLE_SENSOR sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_PARTICLE_SNGCJA5);
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

        //Confidence: High - Configures ADC mode
        SFE_ADS122C04 sensor1;
        if (sensor1.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_ADC_ADS122C04);
      }
      break;
    case 0x41:
      {
        //Confidence: High - Configures ADC mode
        SFE_ADS122C04 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_ADC_ADS122C04);
      }
      break;
    case 0x42:
      {
        //Confidence: High - Sends/receives CRC checked data response
        qwiic.setPullups(0); //Disable pullups to minimize CRC issues
        SFE_UBLOX_GNSS sensor;
        if(settings.printDebugMessages == true) sensor.enableDebugging(); // Enable debug messages if required
        if (sensor.begin(qwiic, i2cAddress) == true) //Wire port, address
        {
          qwiic.setPullups(settings.qwiicBusPullUps); //Re-enable pullups to prevent ghosts at 0x43 onwards
          return (DEVICE_GPS_UBLOX);
        }
        qwiic.setPullups(settings.qwiicBusPullUps); //Re-enable pullups for normal discovery
      }
      break;
    case 0x44:
    case 0x45:
      {
        //Confidence: High - Configures ADC mode
        SFE_ADS122C04 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_ADC_ADS122C04);
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
    case 0x55:
      {
        if (settings.identifyBioSensorHubs == true)
        {
          SparkFun_Bio_Sensor_Hub sensor(32, 11, i2cAddress); // Reset pin is 32, MFIO pin is 11
          if (sensor.begin(qwiic) == 0x00) //Wire port
            return (DEVICE_BIO_SENSOR_HUB);
        }
      }
      break;
    case 0x58:
      {
        SGP30 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_VOC_SGP30);
      }
      break;
    case 0x59:
      {
        SGP40 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_VOC_SGP40);
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
        //Always do the MCP9600 first. It's fussy...
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
        //Always do the MCP9600 first. It's fussy...
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);

        //Confidence: High - begin now checks FW Ver CRC
        SCD30 sensor1;
        if(settings.printDebugMessages == true) sensor1.enableDebugging(); // Enable debug messages if required
        // Set measBegin to false. beginQwiicDevices will call begin with measBegin set true.
        if (sensor1.begin(qwiic, true, false) == true) //Wire port, autoCalibrate, measBegin
          return (DEVICE_CO2_SCD30);
      }
      break;
    case 0x62:
      {
        //Always do the MCP9600 first. It's fussy...
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x63:
      {
        //Always do the MCP9600 first. It's fussy...
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x64:
      {
        //Always do the MCP9600 first. It's fussy...
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x65:
      {
        //Always do the MCP9600 first. It's fussy...
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x66:
      {
        //Always do the MCP9600 first. It's fussy...
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
    case 0x67:
      {
        //Always do the MCP9600 first. It's fussy...
        //Confidence: High - Checks 8bit ID
        MCP9600 sensor;
        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_TEMPERATURE_MCP9600);
      }
      break;
//    case 0x68:
//    case 0x69:
//    case 0x6A:
//    case 0x6B:
//    case 0x6C:
//    case 0x6D:
//    case 0x6E:
//    case 0x6F:
//      {
//        QwiicButton sensor;
//        if (sensor.begin(i2cAddress, qwiic) == true) //Address, Wire port
//          return (DEVICE_QWIIC_BUTTON);
//      }
//      break;
    case 0x70:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: High - 16 bit ID check with CRC
        SHTC3 sensor;
        if (sensor.begin(qwiic) == 0) //Wire port. Device returns 0 upon success.
          return (DEVICE_HUMIDITY_SHTC3);
      }
      break;
    case 0x71:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x72:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x73:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x74:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x75:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x76:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        //Confidence: High - does CRC on internal EEPROM read and checks sensor version
        MS5837 sensor2;
        if (sensor2.begin(qwiic) == true) //Wire port
        {
          if (sensor2.getModel() <= 1) // Check that getModel returns 0 or 1. It will (hopefully) return 255 if an MS5637 is attached.
            return (DEVICE_PRESSURE_MS5837);          
        }

        //Confidence: High - does CRC on internal EEPROM read - but do this second as a MS5837 will appear as a MS5637
        MS5637 sensor;
        if (sensor.begin(qwiic) == true) //Wire port
          return (DEVICE_PRESSURE_MS5637);

        //Confidence: High - ID check but may pass with BMP280
        BME280 sensor1;
        sensor1.setI2CAddress(i2cAddress);
        if (sensor1.beginI2C(qwiic) == true) //Wire port
          return (DEVICE_PHT_BME280);

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
        
        BME280 sensor;
        sensor.setI2CAddress(i2cAddress);
        if (sensor.beginI2C(qwiic) == true) //Wire port
          return (DEVICE_PHT_BME280);
      }
      break;
    default:
      {
        if (muxAddress == 0)
        {
          SerialPrintf2("Unknown device at address (0x%02X)\r\n", i2cAddress);
        }
        else
        {
          SerialPrintf4("Unknown device at address (0x%02X)(Mux:0x%02X Port:%d)\r\n", i2cAddress, muxAddress, portNumber);
        }
        return DEVICE_UNKNOWN_DEVICE;
      }
      break;
  }
  SerialPrintf2("Known I2C address but device failed identification at address 0x%02X\r\n", i2cAddress);
  return DEVICE_UNKNOWN_DEVICE;
}

//Given an address, returns the device type if it responds as we would expect
//This version is dedicated to testing muxes and uses a custom .begin to avoid the slippery mux problem
//However, we also need to check if an MS8607 is attached (address 0x76) as it can cause the I2C bus to lock up if not detected correctly
deviceType_e testMuxDevice(uint8_t i2cAddress, uint8_t muxAddress, uint8_t portNumber)
{
  switch (i2cAddress)
  {
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
        
        //Confidence: Medium - Write/Read/Clear to 0x00
        if (multiplexerBegin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x76:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);

        // If an MS8607 is connected, multiplexerBegin causes the MS8607 to 'crash' and lock up the I2C bus... So we need to check if an MS8607 is connected first. 
        // We will use the MS5637 as this will test for itself, the MS5837 and the pressure sensor of the MS8607
        // Just to make life even more complicated, a mux with address 0x76 will also appear as an MS5637 due to the way the MS5637 eeprom crc check is calculated.
        // So, we can't use .begin as the test for a MS5637 / MS5837 / MS8607. We need to be more creative!
        // If we write 0xA0 to i2cAddress and then read two bytes:
        //  A mux will return 0xA0A0
        //  An MS5637 / MS5837 / MS8607 will return the value stored in its eeprom which _hopefully_ is not 0xA0A0!

        // Let's hope this doesn't cause problems for the BME280...! We should be OK as the default address for the BME280 is 0x77.
        
        qwiic.beginTransmission((uint8_t)i2cAddress);
        qwiic.write((uint8_t)0xA0);
        uint8_t i2c_status = qwiic.endTransmission();

        if (i2c_status == 0) // If the I2C write was successful
        {
          qwiic.requestFrom((uint8_t)i2cAddress, 2U); // Read two bytes
          uint8_t buffer[2];
          for (uint8_t i = 0; i < 2; i++)
          {
            buffer[i] = qwiic.read();
          }
          if ((buffer[0] != 0xA0) || (buffer[1] != 0xA0)) // If we read back something other than 0xA0A0 then we are probably talking to an MS5637 / MS5837 / MS8607, not a mux
          {
            return (DEVICE_PRESSURE_MS5637);
          }
        }

        //Confidence: Medium - Write/Read/Clear to 0x00
        if (multiplexerBegin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    case 0x77:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(DEVICE_MULTIPLEXER, i2cAddress, muxAddress, portNumber) == true) return (DEVICE_MULTIPLEXER);
        
        //Confidence: Medium - Write/Read/Clear to 0x00
        if (multiplexerBegin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
      }
      break;
    default:
      {
        if (muxAddress == 0)
        {
          SerialPrintf2("Unknown device at address (0x%02X)\r\n", i2cAddress);
        }
        else
        {
          SerialPrintf4("Unknown device at address (0x%02X)(Mux:0x%02X Port:%d)\r\n", i2cAddress, muxAddress, portNumber);
        }
        return DEVICE_UNKNOWN_DEVICE;
      }
      break;
  }
  return DEVICE_UNKNOWN_DEVICE;
}

//Returns true if mux is present
//Tests for device ack to I2C address
//Then tests if device behaves as we expect
//Leaves with all ports disabled
bool multiplexerBegin(uint8_t deviceAddress, TwoWire &wirePort)
{
  wirePort.beginTransmission(deviceAddress);
  if (wirePort.endTransmission() != 0)
    return (false); //Device did not ACK

  //Write to device, expect a return
  setMuxPortState(0xA4, deviceAddress, wirePort, EXTRA_MUX_STARTUP_BYTES); //Set port register to a known value - using extra bytes to avoid the mux problem
  uint8_t response = getMuxPortState(deviceAddress, wirePort);
  setMuxPortState(0x00, deviceAddress, wirePort, 0); //Disable all ports - seems to work just fine without extra bytes (not sure why...)
  if (response == 0xA4) //Make sure we got back what we expected
  {
    response = getMuxPortState(deviceAddress, wirePort); //Make doubly sure we got what we expected
    if (response == 0x00)
    {
      return (true); //All good
    }
  }
  return (false);
}

//Writes a 8-bit value to mux
//Overwrites any other bits
//This allows us to enable/disable multiple ports at same time
bool setMuxPortState(uint8_t portBits, uint8_t deviceAddress, TwoWire &wirePort, int extraBytes)
{
  wirePort.beginTransmission(deviceAddress);
  for (int i = 0; i < extraBytes; i++)
  {
    wirePort.write(0x00); // Writing these extra bytes seems key to avoiding the slippery mux problem
  }
  wirePort.write(portBits);
  if (wirePort.endTransmission() != 0)
    return (false); //Device did not ACK
  return (true);
}

//Gets the current port state
//Returns byte that may have multiple bits set
uint8_t getMuxPortState(uint8_t deviceAddress, TwoWire &wirePort)
{
  //Read the current mux settings
  wirePort.beginTransmission(deviceAddress);
  wirePort.requestFrom(deviceAddress, 1);
  return (wirePort.read());
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
    case DEVICE_ADC_ADS122C04:
      return "ADC-ADS122C04";
      break;
    case DEVICE_PRESSURE_MPR0025PA1:
      return "Pressure-MPR";
      break;
    case DEVICE_PARTICLE_SNGCJA5:
      return "Particle-SNGCJA5";
      break;
    case DEVICE_VOC_SGP40:
      return "VOC-SGP40";
      break;
    case DEVICE_PRESSURE_SDP3X:
      return "Pressure-SDP3X";
      break;
    case DEVICE_PRESSURE_MS5837:
      return "Pressure-MS5837";
      break;
//    case DEVICE_QWIIC_BUTTON:
//      return "Qwiic_Button";
//      break;
    case DEVICE_BIO_SENSOR_HUB:
      return "Bio-Sensor-Oximeter";
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
