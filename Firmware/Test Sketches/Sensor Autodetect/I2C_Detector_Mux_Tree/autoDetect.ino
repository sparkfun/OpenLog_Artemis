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

/*
  None of these functions have part specific handling. Ie, you won't see VL53L1X or switch
  cases in this sketch.
*/

//Search the linked list, and if new, add this device and return true
bool recordDevice(uint8_t address, deviceType_e deviceType)
{
  return (recordDevice(address, deviceType, 0, 0));
}

//Search the linked list, and if new, add this device and return true
//Records the muxAddress and port number that this device lives on
bool recordDevice(uint8_t address, deviceType_e deviceType, uint8_t muxAddress, uint8_t portNumber)
{
  if (deviceExists(address, muxAddress, portNumber) == true) return (false);

  node *temp = new node;

  temp->address = address;
  temp->portNumber = portNumber;
  temp->muxAddress = muxAddress;
  temp->deviceType = deviceType;

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

  return (true);
}

//Search the linked list for a given address
//Returns true if this device address already exists in our system
bool deviceExists(uint8_t address, uint8_t muxAddress, uint8_t portNumber)
{
  node *temp = new node;
  temp = head;

  while (temp != NULL)
  {
    if (temp->address == address)
      if (temp->muxAddress == muxAddress)
        if (temp->portNumber == portNumber) return (true);

    //Devices that were discovered on the main branch will be discovered over and over again
    //If a device has a 0/0 mux/port address, it's on the main branch and exists on all
    //sub branches.
    if (temp->address == address)
      if (temp->muxAddress == 0)
        if (temp->portNumber == 0) return (true);

    temp = temp->next;
  }
  return (false);
}

//Given a deviceType and a count, return the i2c address of that device within the map
uint8_t getDeviceAddress(deviceType_e deviceType, uint8_t deviceCount)
{
  node *temp = new node;
  temp = head;

  if (temp == NULL)
  {
    Serial.println("GetDeviceAddress Error: No devices in list");
    return (0);
  }

  int currentCount = 0;
  while (temp != NULL)
  {
    if (temp->deviceType == deviceType)
    {
      if (currentCount == deviceCount)
        return (temp->address);
      currentCount++;
    }

    temp = temp->next;
  }
  Serial.println("GetDeviceAddress Error: Device not found");
  return (0);
}

//Given an address, return the device type
deviceType_e getDeviceType(uint8_t address)
{
  node *temp = new node;
  temp = head;

  if (temp == NULL)
  {
    Serial.println("GetDeviceType Error: No devices in list");
    return (DEVICE_UNKNOWN_DEVICE);
  }

  while (temp != NULL)
  {
    if (temp->address == address)
      return (temp->deviceType);

    temp = temp->next;
  }
  Serial.println("GetDeviceAddress Error: Device not found");
  return (DEVICE_UNKNOWN_DEVICE);
}

//Given the address of a device, enable muxes appropriately to open connection access device
//Return true if connection was opened
bool openConnection(uint8_t address)
{
  if (head == NULL)
  {
    Serial.println("OpenConnection Error: No devices in list");
    return false;
  }

  //Find the mux and port associated with this address
  node *temp = new node;
  temp = head;

  uint8_t muxAddress = 0;
  uint8_t portNumber = 0;

  while (temp != NULL)
  {
    if (temp->address == address)
    {
      portNumber = temp->portNumber;
      muxAddress = temp->muxAddress;
      break;
    }
    temp = temp->next;
  }

  if (muxAddress == 0)
  {
    //This device is on the main branch, no mux configure needed
    return (true);
  }

  //Get the class number for the mux associated with this device's mux address
  uint8_t deviceNumber = 0;
  for ( ; deviceNumber < deviceCounts[DEVICE_MULTIPLEXER] ; deviceNumber++)
  {
    if (getDeviceAddress(DEVICE_MULTIPLEXER, deviceNumber) == muxAddress)
      break; //Device number located
  }
  if (deviceNumber == deviceCounts[DEVICE_MULTIPLEXER])
  {
    Serial.println("OpenConnection Error: Device number not found");
    return false;
  }

  //Connect to this mux and port
  multiplexer[deviceNumber]->setPort(portNumber);

  return (true);
}

void printDetectedDevices()
{
  Serial.println("Device counts:");
  for (int x = 0 ; x < DEVICE_TOTAL_DEVICES ; x++)
    Serial.printf("-%s: %d\n", getDeviceName((deviceType_e)x), deviceCounts[x]);

  Serial.println("Device list:");
  Serial.println("Address.MuxAddress.Port#: Class[deviceNumber]");

  node *temp = new node;
  temp = head;

  if (temp == NULL) Serial.println("-Nothing detected");

  while (temp != NULL)
  {
    Serial.printf("-0x%02X.0x%02X.0x%02X: %s[%d]\n", temp->address, temp->muxAddress, temp->portNumber, getDeviceName(temp->deviceType), temp->deviceNumber);

    temp = temp->next;
  }
}
