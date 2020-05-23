
//Let's see what's on the I2C bus
//Scan I2C bus including sub-branches of multiplexers
//Creates a linked list of devices
//Creates appropriate classes for each device
//Begin()s each device in list
//Returns true if devices detected > 0
bool detectQwiicDevices()
{
  bool somethingDetected = false;

  qwiic.setClock(100000); //During detection, go slow

  qwiic.setPullups(1); //Set pullups to 1k. If we don't have pullups, detectQwiicDevices() takes ~900ms to complete. We'll disable pullups if something is detected.

  //24k causes a bunch of unknown devices to be falsely detected.
  //qwiic.setPullups(24); //Set pullups to 24k. If we don't have pullups, detectQwiicDevices() takes ~900ms to complete. We'll disable pullups if something is detected.

  //Depending on what hardware is configured, the Qwiic bus may have only been turned on a few ms ago
  //Give sensors, specifically those with a low I2C address, time to turn on
  delay(100); //SCD30 required >50ms to turn on

  //First scan for Muxes. Valid addresses are 0x70 to 0x77.
  //If any are found, they will be begin()'d causing their ports to turn off
  Serial.println("Scanning for Muxes");
  uint8_t muxCount = 0;
  for (uint8_t address = 0x70 ; address < 0x78 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      somethingDetected = true;

      deviceType_e foundType = testDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType == DEVICE_MULTIPLEXER)
      {
        addDevice(foundType, address, 0, 0); //Add this device to our map
        muxCount++;
      }
    }
  }

  //Before going into sub branches, complete the scan of the main branch for all devices
  Serial.println("Scanning main bus");
  for (uint8_t address = 1 ; address < 127 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      somethingDetected = true;
      deviceType_e foundType = testDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType != DEVICE_UNKNOWN_DEVICE)
      {
        if (addDevice(foundType, address, 0, 0) == true) //Records this device. //Returns false if device was already recorded.
          Serial.printf("-Added %s at address 0x%02X\n", getDeviceName(foundType), address);
      }
      else
        Serial.printf("-Device %s failed testing at address 0x%02X\n", getDeviceName(foundType), address);
    }
  }

  //If we have muxes, begin scanning their sub nets
  if (muxCount > 0)
  {
    Serial.println("Scanning sub nets");

    //Step into first mux and begin stepping through ports
    for (int muxNumber = 0 ; muxNumber < muxCount ; muxNumber++)
    {
      //The node tree starts with muxes so we can align node numbers
      node *muxNode = getNodePointer(muxNumber);
      QWIICMUX *myMux = (QWIICMUX *)muxNode->classPtr;

      for (int portNumber = 0 ; portNumber < 7 ; portNumber++)
      {
        myMux->setPort(portNumber);

        //Scan this new bus for new addresses
        for (uint8_t address = 1 ; address < 127 ; address++)
        {
          qwiic.beginTransmission(address);
          if (qwiic.endTransmission() == 0)
          {
            somethingDetected = true;

            deviceType_e foundType = testDevice(address, muxNode->address, portNumber);
            if (foundType != DEVICE_UNKNOWN_DEVICE)
            {
              if (addDevice(foundType, address, muxNode->address, portNumber) == true) //Record this device, with mux port specifics.
                Serial.printf("-Added %s at address 0x%02X.0x%02X.%d\n", getDeviceName(foundType), address, muxNode->address, portNumber);
            }
            else
              Serial.printf("-%s found at address 0x%02X\n", getDeviceName(foundType), address);
          } //End I2c check
        } //End I2C scanning
      } //End mux port stepping
    } //End mux stepping
  } //End mux > 0

  if (somethingDetected)
  {
    //createClassesForDetectedDevices();
    //beginDetectedDevices(); //Step through the linked list and begin() everything in our map
    printDetectedDevices();
  }

  if (somethingDetected) qwiic.setPullups(0); //We've detected something on the bus so disable pullups

  Serial.println("Autodetect complete");
  Serial.flush();

  return (somethingDetected);
}

void menuAttachedDevices()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Attached Devices");

    int availableDevices = 0;

    //Step through node list
    node *temp = head;

    if (temp == NULL)
      Serial.println("**No devices detected on Qwiic bus**");

    while (temp != NULL)
    {
      char strAddress[50];
      if (temp->muxAddress == 0)
        sprintf(strAddress, "(0x%02X)", temp->address);
      else
        sprintf(strAddress, "(0x%02X)(Mux:0x%02X Port:%d)", temp->muxAddress, temp->portNumber, temp->address);

      char strDeviceMenu[10];
      sprintf(strDeviceMenu, "%d)", availableDevices++ + 1);

      switch (temp->deviceType)
      {
        case DEVICE_MULTIPLEXER:
          Serial.printf("%s Multiplexer %s\n", strDeviceMenu, strAddress);
          break;
        case DEVICE_DISTANCE_VL53L1X:
          Serial.printf("%s VL53L1X Distance Sensor %s\n", strDeviceMenu, strAddress);
          break;
        case DEVICE_PHT_BME280:
          Serial.printf("%s BME280 Pressure/Humidity/Temp (PHT) Sensor %s\n", strDeviceMenu, strAddress);
          break;
        case DEVICE_VOC_CCS811:
          Serial.printf("%s CCS811 tVOC and CO2 Sensor %s\n", strDeviceMenu, strAddress);
          break;
        default:
          Serial.printf("Unknown device type %d in menuAttachedDevices\n", temp->deviceType);
          break;
      }

      temp = temp->next;
    }

    Serial.printf("%d) Configure Qwiic Settings\n", availableDevices++ + 1);

    Serial.println("x) Exit");

    int nodeNumber = getNumber(menuTimeout); //Timeout after x seconds
    if (nodeNumber > 0 && nodeNumber < availableDevices)
    {
      //Lookup the function we need to call based the node number
      FunctionPointer functionPointer = getConfigFunctionPtr(nodeNumber - 1);

      //Get the configPtr for this given node
      void *deviceConfigPtr = getConfigPointer(nodeNumber - 1);
      functionPointer(deviceConfigPtr); //Call the appropriate config menu with a pointer to this node's configPtr

      configureDevice(nodeNumber); //Reconfigure this device with the new settings
    }
    else if (nodeNumber == availableDevices)
    {
      //menuConfigure_QwiicBus();
    }
    else if (nodeNumber == STATUS_PRESSED_X)
      break;
    else if (nodeNumber == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(nodeNumber);
  }
}



void menuConfigure_Multiplexer(void *configPtr)
{
  //struct_multiplexer *sensor = (struct_multiplexer*)configPtr;

  Serial.println();
  Serial.println("Menu: Configure Multiplexer");

  Serial.println("There are currently no configurable options for this device.");
  delay(500);
}

//There is short and long range mode
//The Intermeasurement period seems to set the timing budget (PLL of the device)
//Setting the Intermeasurement period too short causes the device to freeze up
//The intermeasurement period that gets written as X gets read as X+1 so we get X and write X-1.
void menuConfigure_VL53L1X(void *configPtr)
{
  struct_VL53L1X *sensor = (struct_VL53L1X*)configPtr;

  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure VL53L1X Distance Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensor->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensor->log == true)
    {
      Serial.print("2) Log Distance: ");
      if (sensor->logDistance == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Range Status: ");
      if (sensor->logRangeStatus == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Signal Rate: ");
      if (sensor->logSignalRate == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Set Distance Mode: ");
      if (sensor->distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
        Serial.print("Short");
      else
        Serial.print("Long");
      Serial.println();

      Serial.printf("6) Set Intermeasurement Period: %d ms\n", sensor->intermeasurementPeriod);
      Serial.printf("7) Set Offset: %d mm\n", sensor->offset);
      Serial.printf("8) Set Cross Talk (counts per second): %d cps\n", sensor->crosstalk);
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensor->log ^= 1;
    else if (sensor->log == true)
    {
      if (incoming == '2')
        sensor->logDistance ^= 1;
      else if (incoming == '3')
        sensor->logRangeStatus ^= 1;
      else if (incoming == '4')
        sensor->logSignalRate ^= 1;
      else if (incoming == '5')
      {
        if (sensor->distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
          sensor->distanceMode = VL53L1X_DISTANCE_MODE_LONG;
        else
          sensor->distanceMode = VL53L1X_DISTANCE_MODE_SHORT;

        //Error check
        if (sensor->distanceMode == VL53L1X_DISTANCE_MODE_LONG && sensor->intermeasurementPeriod < 140)
        {
          sensor->intermeasurementPeriod = 140;
          Serial.println("Intermeasurement Period increased to 140ms");
        }
      }
      else if (incoming == '6')
      {
        int min = 20;
        if (sensor->distanceMode == VL53L1X_DISTANCE_MODE_LONG)
          min = 140;


        Serial.printf("Set timing budget (%d to 1000ms): ", min);
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < min || amt > 1000)
          Serial.println("Error: Out of range");
        else
          sensor->intermeasurementPeriod = amt;
      }
      else if (incoming == '7')
      {
        Serial.print("Set Offset in mm (0 to 4000mm): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 4000)
          Serial.println("Error: Out of range");
        else
          sensor->offset = amt;
      }
      else if (incoming == '8')
      {
        Serial.print("Set Crosstalk in Counts Per Second: ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 4000)
          Serial.println("Error: Out of range");
        else
          sensor->crosstalk = amt;
      }
      else if (incoming == 'x')
        break;
      else if (incoming == STATUS_GETBYTE_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  //qwiicOnline.VL53L1X = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_BME280(void *configPtr)
{
  struct_BME280 *sensor = (struct_BME280*)configPtr;

  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure BME280 Pressure/Humidity/Temperature Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensor->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensor->log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (sensor->logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Humidity: ");
      if (sensor->logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Altitude: ");
      if (sensor->logAltitude == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Log Temperature: ");
      if (sensor->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensor->log ^= 1;
    else if (sensor->log == true)
    {
      if (incoming == '2')
        sensor->logPressure ^= 1;
      else if (incoming == '3')
        sensor->logHumidity ^= 1;
      else if (incoming == '4')
        sensor->logAltitude ^= 1;
      else if (incoming == '5')
        sensor->logTemperature ^= 1;
      else if (incoming == 'x')
        break;
      else if (incoming == STATUS_GETBYTE_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  //qwiicOnline.BME280 = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_CCS811(void *configPtr)
{
  struct_CCS811 *sensor = (struct_CCS811*)configPtr;

  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure CCS811 tVOC and CO2 Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensor->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensor->log == true)
    {
      Serial.print("2) Log tVOC: ");
      if (sensor->logTVOC == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log CO2: ");
      if (sensor->logCO2 == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensor->log ^= 1;
    else if (sensor->log == true)
    {
      if (incoming == '2')
        sensor->logTVOC ^= 1;
      else if (incoming == '3')
        sensor->logCO2 ^= 1;
      else if (incoming == 'x')
        break;
      else if (incoming == STATUS_GETBYTE_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  //  sensor->online = false; //Mark as offline so it will be started with new settings
}
