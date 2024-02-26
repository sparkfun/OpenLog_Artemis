/*
  To add a new sensor to the system:

  Add the library in OpenLog_Artemis
  Add DEVICE_ name to settings.h
  Add struct_MCP9600 to settings.h - This will define what settings for the sensor we will control

  Add gathering of data to gatherDeviceValues() in Sensors
  Add helper text to printHelperText() in Sensors

  Add class creation to addDevice() in autoDetect
  Add begin fucntions to beginQwiicDevices() in autoDetect
  Add configuration functions to configureDevice() in autoDetect
  Add pointer to configuration menu name to getConfigFunctionPtr() in autodetect
  Add test case to testDevice() in autoDetect
  Add pretty print device name to getDeviceName() in autoDetect

  Add menu title to menuAttachedDevices() list in menuAttachedDevices
  Create a menuConfigure_LPS25HB() function in menuAttachedDevices

  Add settings to the save/load device file settings in nvm
*/


//Let's see what's on the I2C bus
//Scan I2C bus including sub-branches of multiplexers
//Creates a linked list of devices
//Creates appropriate classes for each device
//Begin()s each device in list
//Returns true if devices detected > 0
bool detectQwiicDevices()
{
  printDebug(F("detectQwiicDevices started\r\n"));
  bool somethingDetected = false;

  qwiic.setClock(AM_HAL_IOM_100KHZ); //During detection, go slow

  setQwiicPullups(settings.qwiicBusPullUps); //Set pullups. (Redundant. beginQwiic has done this too.) If we don't have pullups, detectQwiicDevices() takes ~900ms to complete. We'll disable pullups if something is detected.

  //24k causes a bunch of unknown devices to be falsely detected.
  //setQwiicPullups(24); //Set pullups to 24k. If we don't have pullups, detectQwiicDevices() takes ~900ms to complete. We'll disable pullups if something is detected.

  waitForQwiicBusPowerDelay(); // Wait while the qwiic devices power up

  // Note: The MCP9600 (Qwiic Thermocouple) is a fussy device. If we use beginTransmission + endTransmission more than once
  // the second and subsequent times will fail. The MCP9600 only ACKs the first time. The MCP9600 also appears to be able to
  // lock up the I2C bus if you don't discover it and then begin it in one go...
  // The following code has been restructured to try and keep the MCP9600 happy.

  SerialPrintln(F("Identifying Qwiic Muxes..."));

  //First scan for Muxes. Valid addresses are 0x70 to 0x77 (112 to 119).
  //If any are found, they will be begin()'d causing their ports to turn off
  //testMuxDevice will check if an MS8607 is attached (address 0x76) as it can cause the I2C bus to lock up if we try to detect it as a mux
  uint8_t muxCount = 0;
  for (uint8_t address = 0x70 ; address < 0x78 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      //if (address == 0x74) // Debugging the slippery mux bug - trigger the scope when we test mux 0x74
      //{
      //  digitalWrite(PIN_LOGIC_DEBUG, LOW);
      //  digitalWrite(PIN_LOGIC_DEBUG, HIGH);
      //}
      somethingDetected = true;
      if (settings.printDebugMessages == true)
      {
        SerialPrintf2("detectQwiicDevices: something detected at address 0x%02X\r\n", address);
      }
      deviceType_e foundType = testMuxDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType == DEVICE_MULTIPLEXER)
      {
        addDevice(foundType, address, 0, 0); //Add this device to our map
        if (settings.printDebugMessages == true)
        {
          SerialPrintf2("detectQwiicDevices: multiplexer found at address 0x%02X\r\n", address);
        }
        muxCount++;
      }
      else if (foundType == DEVICE_PRESSURE_MS5637)
      {
        if (settings.printDebugMessages == true)
        {
          SerialPrintf2("detectQwiicDevices: MS8607/MS5637/MS5837/BME280 found at address 0x%02X. Ignoring it for now...\r\n", address);
        }
      }
      else if (foundType == DEVICE_PHT_BME280)
      {
        if (settings.printDebugMessages == true)
        {
          SerialPrintf2("detectQwiicDevices: BME280 found at address 0x%02X. Ignoring it for now...\r\n", address);
        }
      }
      else if (foundType == DEVICE_HUMIDITY_SHTC3)
      {
        if (settings.printDebugMessages == true)
        {
          SerialPrintf2("detectQwiicDevices: SHTC3 found at address 0x%02X. Ignoring it for now...\r\n", address);
        }
      }
    }
  }

  if (muxCount > 0)
  {
    if (settings.printDebugMessages == true)
    {
      SerialPrintf2("detectQwiicDevices: found %d", muxCount);
      if (muxCount == 1)
        SerialPrintln(F(" multiplexer"));
      else
        SerialPrintln(F(" multiplexers"));
    }
    beginQwiicDevices(); //begin() the muxes to disable their ports
  }

  //Before going into mux sub branches, scan the main branch for all remaining devices
  SerialPrintln(F("Identifying Qwiic Devices..."));
  bool foundMS8607 = false; // The MS8607 appears as two devices (MS8607 and MS5637). We need to skip the MS5637/MS5837 if we have found a MS8607.
  for (uint8_t address = 1 ; address < 127 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      somethingDetected = true;
      if (settings.printDebugMessages == true)
      {
        SerialPrintf2("detectQwiicDevices: something detected at address 0x%02X\r\n", address);
      }
      deviceType_e foundType = testDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType != DEVICE_UNKNOWN_DEVICE)
      {
        if ((foundMS8607 == true) && ((foundType == DEVICE_PRESSURE_MS5637) || (foundType == DEVICE_PRESSURE_MS5837)))
        {
          ; // Skip MS5637/MS5837 as we have already found an MS8607
        }
        else
        {
          if (addDevice(foundType, address, 0, 0) == true) //Records this device. //Returns false if mux/device was already recorded.
          {
            if (settings.printDebugMessages == true)
            {
              SerialPrintf3("detectQwiicDevices: added %s at address 0x%02X\r\n", getDeviceName(foundType), address);
            }
          }
        }
        if (foundType == DEVICE_PHT_MS8607)
        {
          foundMS8607 = true; // Flag that we have found an MS8607
        }
      }
    }
  }

  if (somethingDetected == false) return (false);

  //If we have muxes, begin scanning their sub nets
  if (muxCount > 0)
  {
    SerialPrintln(F("Multiplexers found. Scanning sub nets..."));

    //Step into first mux and begin stepping through ports
    for (int muxNumber = 0 ; muxNumber < muxCount ; muxNumber++)
    {
      //The node tree starts with muxes so we can align node numbers
      node *muxNode = getNodePointer(muxNumber);
      QWIICMUX *myMux = (QWIICMUX *)muxNode->classPtr;

      printDebug("detectQwiicDevices: scanning the ports of multiplexer " + (String)muxNumber);
      printDebug(F("\r\n"));

      for (int portNumber = 0 ; portNumber < 8 ; portNumber++) //Assumes we are using a mux with 8 ports max
      {
        myMux->setPort(portNumber);
        foundMS8607 = false; // The MS8607 appears as two devices (MS8607 and MS5637). We need to skip the MS5637 if we have found a MS8607.

        printDebug("detectQwiicDevices: scanning port number " + (String)portNumber);
        printDebug(" on multiplexer " + (String)muxNumber);
        printDebug(F("\r\n"));

        //Scan this new bus for new addresses
        for (uint8_t address = 1 ; address < 127 ; address++)
        {
          // If we found a device on the main branch, we cannot/should not attempt to scan for it on mux branches or bad things may happen
          if (deviceExists(DEVICE_TOTAL_DEVICES, address, 0, 0)) // Check if we found any type of device with this address on the main branch
          {
            if (settings.printDebugMessages == true)
            {
              SerialPrintf2("detectQwiicDevices: skipping device address 0x%02X because we found one on the main branch\r\n", address);
            }
          }
          else
          {
            qwiic.beginTransmission(address);
            if (qwiic.endTransmission() == 0)
            {
              // We don't need to do anything special for the MCP9600 here, because we can guarantee that beginTransmission + endTransmission
              // have only been used once for each MCP9600 address

              somethingDetected = true;

              deviceType_e foundType = testDevice(address, muxNode->address, portNumber);
              if (foundType != DEVICE_UNKNOWN_DEVICE)
              {
                if ((foundType == DEVICE_PRESSURE_MS5637) && (foundMS8607 == true))
                {
                  ; // Skip MS5637 as we have already found an MS8607
                }
                else
                {
                  if (foundType == DEVICE_MULTIPLEXER) // Let's ignore multiplexers hanging off multiplexer ports. (Multiple muxes on the main branch is OK.)
                  {
                    if (settings.printDebugMessages == true)
                      SerialPrintf5("detectQwiicDevices: ignoring %s at address 0x%02X.0x%02X.%d\r\n", getDeviceName(foundType), address, muxNode->address, portNumber);
                  }
                  else
                  {
                    if (addDevice(foundType, address, muxNode->address, portNumber) == true) //Record this device, with mux port specifics.
                    {
                      if (settings.printDebugMessages == true)
                        SerialPrintf5("detectQwiicDevices: added %s at address 0x%02X.0x%02X.%d\r\n", getDeviceName(foundType), address, muxNode->address, portNumber);
                    }
                  }
                }
                if (foundType == DEVICE_PHT_MS8607)
                {
                  foundMS8607 = true; // Flag that we have found an MS8607
                }
              }
            } //End I2c check
          } //End not on main branch check
        } //End I2C scanning
      } //End mux port stepping

      myMux->setPortState(0); // Disable all ports on this mux now that we have finished scanning them.

    } //End mux stepping
  } //End mux > 0

  bubbleSortDevices(head); //This may destroy mux alignment to node 0.

  //*** Let's leave pull-ups set to 1k and only disable them when taking to a u-blox device ***
  //setQwiicPullups(0); //We've detected something on the bus so disable pullups.

  //We need to call setMaxI2CSpeed in configureQwiicDevices
  //We cannot do it here as the device settings have not been loaded

  SerialPrintln(F("Autodetect complete"));

  return (true);
} // /detectQwiicDevices

void menuAttachedDevices()
{
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Attached Devices"));

    int availableDevices = 0;

    //Step through node list
    node *temp = head;

    if (temp == NULL)
      SerialPrintln(F("**No devices detected on Qwiic bus**"));

    while (temp != NULL)
    {
      //Exclude multiplexers from the list
      if (temp->deviceType != DEVICE_MULTIPLEXER)
      {
        char strAddress[50];
        if (temp->muxAddress == 0)
          sprintf(strAddress, "(0x%02X)", temp->address);
        else
          sprintf(strAddress, "(0x%02X)(Mux:0x%02X Port:%d)", temp->address, temp->muxAddress, temp->portNumber);

        char strDeviceMenu[10];
        sprintf(strDeviceMenu, "%d)", availableDevices++ + 1);

        switch (temp->deviceType)
        {
          case DEVICE_MULTIPLEXER:
            //SerialPrintf3("%s Multiplexer %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_LOADCELL_NAU7802:
            SerialPrintf3("%s NAU7802 Weight Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_DISTANCE_VL53L1X:
            SerialPrintf3("%s VL53L1X Distance Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_GPS_UBLOX:
            SerialPrintf3("%s u-blox GPS Receiver %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PROXIMITY_VCNL4040:
            SerialPrintf3("%s VCNL4040 Proximity Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_TEMPERATURE_TMP117:
            SerialPrintf3("%s TMP117 High Precision Temperature Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_MS5637:
            SerialPrintf3("%s MS5637 Pressure Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_LPS25HB:
            SerialPrintf3("%s LPS25HB Pressure Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_LPS28DFW:
            SerialPrintf3("%s LPS28DFW Pressure Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PHT_BME280:
            SerialPrintf3("%s BME280 Pressure/Humidity/Temp (PHT) Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_UV_VEML6075:
            SerialPrintf3("%s VEML6075 UV Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_LIGHT_VEML7700:
            SerialPrintf3("%s VEML7700 Ambient Light Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_VOC_CCS811:
            SerialPrintf3("%s CCS811 tVOC and CO2 Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_VOC_SGP30:
            SerialPrintf3("%s SGP30 tVOC and CO2 Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_CO2_SCD30:
            SerialPrintf3("%s SCD30 CO2 Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PHT_MS8607:
            SerialPrintf3("%s MS8607 Pressure/Humidity/Temp (PHT) Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_TEMPERATURE_MCP9600:
            SerialPrintf3("%s MCP9600 Thermocouple Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_HUMIDITY_AHT20:
            SerialPrintf3("%s AHT20 Humidity Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_HUMIDITY_SHTC3:
            SerialPrintf3("%s SHTC3 Humidity Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_ADC_ADS122C04:
            SerialPrintf3("%s ADS122C04 ADC (Qwiic PT100) %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_MPR0025PA1:
            SerialPrintf3("%s MPR MicroPressure Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PARTICLE_SNGCJA5:
            SerialPrintf3("%s SN-GCJA5 Particle Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_VOC_SGP40:
            SerialPrintf3("%s SGP40 VOC Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_SDP3X:
            SerialPrintf3("%s SDP3X Differential Pressure Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_MS5837:
            SerialPrintf3("%s MS5837 (BAR30 / BAR02) Pressure Sensor %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_QWIIC_BUTTON:
            SerialPrintf3("%s Qwiic Button %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_BIO_SENSOR_HUB:
            SerialPrintf3("%s Bio Sensor Pulse Oximeter %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_ISM330DHCX:
            SerialPrintf3("%s ISM330DHCX IMU %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_MMC5983MA:
            SerialPrintf3("%s MMC5983MA Magnetometer %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_KX134:
            SerialPrintf3("%s KX134 Accelerometer %s\r\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_ADS1015:
            SerialPrintf3("%s ADS1015 ADC %s\r\n", strDeviceMenu, strAddress);
            break;
          default:
            SerialPrintf2("Unknown device type %d in menuAttachedDevices\r\n", temp->deviceType);
            break;
        }
      }

      temp = temp->next;
    }

    availableDevices++;
    SerialPrintf2("%d) Configure Qwiic Settings\r\n", availableDevices);
    availableDevices++;
    if (settings.identifyBioSensorHubs == true)
    {
      SerialPrintf2("%d) Detect Bio Sensor Pulse Oximeter: Enabled\r\n", availableDevices);
    }
    else
    {
      SerialPrintf2("%d) Detect Bio Sensor Pulse Oximeter: Disabled\r\n", availableDevices);
    }

    SerialPrintln(F("x) Exit"));

    int nodeNumber = getNumber(menuTimeout); //Timeout after x seconds
    if (nodeNumber > 0 && nodeNumber < availableDevices - 1)
    {
      //Lookup the function we need to call based the node number
      FunctionPointer functionPointer = getConfigFunctionPtr(nodeNumber - 1);

      //Get the configPtr for this given node
      void *deviceConfigPtr = getConfigPointer(nodeNumber - 1);
      functionPointer(deviceConfigPtr); //Call the appropriate config menu with a pointer to this node's configPtr

      configureDevice(nodeNumber - 1); //Reconfigure this device with the new settings
    }
    else if (nodeNumber == availableDevices - 1)
    {
      menuConfigure_QwiicBus();
    }
    else if (nodeNumber == availableDevices)
    {
      if (settings.identifyBioSensorHubs)
      {
        SerialPrintln(F(""));
        SerialPrintln(F("\"Detect Bio Sensor Pulse Oximeter\" can only be disabled by \"Reset all settings to default\""));
        SerialPrintln(F(""));
      }
      else
      {
        SerialPrintln(F(""));
        SerialPrintln(F("\"Detect Bio Sensor Pulse Oximeter\" requires exclusive use of pins 32 and 11"));
        SerialPrintln(F("Once enabled, \"Detect Bio Sensor Pulse Oximeter\" can only be disabled by \"Reset all settings to default\""));
        SerialPrintln(F("Pin 32 must be connected to the sensor RST pin"));
        SerialPrintln(F("Pin 11 must be connected to the sensor MFIO pin"));
        SerialPrintln(F("This means you cannot use pins 32 and 11 for: analog logging; triggering; fast/slow logging; stop logging; etc."));
        SerialPrintln(F("Are you sure? Press 'y' to confirm: "));
        byte bContinue = getByteChoice(menuTimeout);
        if (bContinue == 'y')
        {
          settings.identifyBioSensorHubs = true;
          settings.logA11 = false;
          settings.logA32 = false;
          if (settings.useGPIO11ForTrigger == true) // If interrupts are enabled, we need to disable and then re-enable
          {
            detachInterrupt(PIN_TRIGGER); // Disable the interrupt
            settings.useGPIO11ForTrigger = false;
          }
          settings.useGPIO11ForFastSlowLogging = false;
          if (settings.useGPIO32ForStopLogging == true)
          {
            // Disable stop logging
            settings.useGPIO32ForStopLogging = false;
            detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt
          }

          recordSystemSettings(); //Record the new settings to EEPROM and config file now in case the user resets before exiting the menus

          if (detectQwiicDevices() == true) //Detect the oximeter
          {
            beginQwiicDevices(); //Begin() each device in the node list
            configureQwiicDevices(); //Apply config settings to each device in the node list
            recordDeviceSettingsToFile(); //Record the current devices settings to device config file now in case the user resets before exiting the menus
          }

          recordSystemSettings(); //Record the new settings to EEPROM and config file now in case the user resets before exiting the menus

          if (detectQwiicDevices() == true) //Detect the oximeter
          {
            beginQwiicDevices(); //Begin() each device in the node list
            configureQwiicDevices(); //Apply config settings to each device in the node list
            recordDeviceSettingsToFile(); //Record the current devices settings to device config file now in case the user resets before exiting the menus
          }
        }
        else
          SerialPrintln(F("\"Detect Bio Sensor Pulse Oximeter\"  aborted"));
      }
    }
    else if (nodeNumber == STATUS_PRESSED_X)
      break;
    else if (nodeNumber == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(nodeNumber);
  }
}

void menuConfigure_QwiicBus()
{
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Qwiic Bus"));

    SerialPrint(F("1) Turn off bus power between readings (>2s): "));
    if (settings.powerDownQwiicBusBetweenReads == true) SerialPrintln(F("Yes"));
    else SerialPrintln(F("No"));

    SerialPrintf2("2) Set Max Qwiic Bus Speed: %d Hz\r\n", settings.qwiicBusMaxSpeed);

    SerialPrintf2("3) Set minimum Qwiic bus power up delay: %d ms\r\n", settings.qwiicBusPowerUpDelayMs);

    SerialPrint(F("4) Qwiic bus pull-ups (internal to the Artemis): "));
    if (settings.qwiicBusPullUps == 1)
      SerialPrintln(F("1.5k"));
    else if (settings.qwiicBusPullUps == 6)
      SerialPrintln(F("6k"));
    else if (settings.qwiicBusPullUps == 12)
      SerialPrintln(F("12k"));
    else if (settings.qwiicBusPullUps == 24)
      SerialPrintln(F("24k"));
    else
      SerialPrintln(F("None"));

    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.powerDownQwiicBusBetweenReads ^= 1;
    else if (incoming == '2')
    {
      if (settings.qwiicBusMaxSpeed == 100000)
        settings.qwiicBusMaxSpeed = 400000;
      else
        settings.qwiicBusMaxSpeed = 100000;
    }
    else if (incoming == '3')
    {
      // 60 seconds is more than long enough for a ZED-F9P to do a warm start after being powered cycled, so that seems a sensible maximum
      // minimumQwiicPowerOnDelay is defined in settings.h
      SerialPrintf2("Enter the minimum number of milliseconds to wait for Qwiic VCC to stabilize before communication: (%d to 60000): ", minimumQwiicPowerOnDelay);
      unsigned long amt = getNumber(menuTimeout);
      if ((amt >= minimumQwiicPowerOnDelay) && (amt <= 60000))
        settings.qwiicBusPowerUpDelayMs = amt;
      else
        SerialPrintln(F("Error: Out of range"));
    }
    else if (incoming == '4')
    {
      SerialPrint(F("Enter the Artemis pull-up resistance (0 = None; 1 = 1.5k; 6 = 6k; 12 = 12k; 24 = 24k): "));
      uint32_t pur = (uint32_t)getNumber(menuTimeout);
      if ((pur == 0) || (pur == 1) || (pur == 6) || (pur == 12) || (pur == 24))
        settings.qwiicBusPullUps = pur;
      else
        SerialPrintln(F("Error: Invalid resistance. Possible values are 0,1,6,12,24."));
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_Multiplexer(void *configPtr)
{
  //struct_multiplexer *sensor = (struct_multiplexer*)configPtr;

  SerialPrintln(F(""));
  SerialPrintln(F("Menu: Configure Multiplexer"));

  SerialPrintln(F("There are currently no configurable options for this device."));
  for (int i = 0; i < 500; i++)
  {
    checkBattery();
    delay(1);
  }
}

//There is short and long range mode
//The Intermeasurement period seems to set the timing budget (PLL of the device)
//Setting the Intermeasurement period too short causes the device to freeze up
//The intermeasurement period that gets written as X gets read as X+1 so we get X and write X-1.
void menuConfigure_VL53L1X(void *configPtr)
{
  struct_VL53L1X *sensorSetting = (struct_VL53L1X*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure VL53L1X Distance Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Distance: "));
      if (sensorSetting->logDistance == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Range Status: "));
      if (sensorSetting->logRangeStatus == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Signal Rate: "));
      if (sensorSetting->logSignalRate == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Set Distance Mode: "));
      if (sensorSetting->distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
        SerialPrint(F("Short"));
      else
        SerialPrint(F("Long"));
      SerialPrintln(F(""));

      SerialPrintf2("6) Set Intermeasurement Period: %d ms\r\n", sensorSetting->intermeasurementPeriod);
      SerialPrintf2("7) Set Offset: %d mm\r\n", sensorSetting->offset);
      SerialPrintf2("8) Set Cross Talk (counts per second): %d cps\r\n", sensorSetting->crosstalk);
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logDistance ^= 1;
      else if (incoming == '3')
        sensorSetting->logRangeStatus ^= 1;
      else if (incoming == '4')
        sensorSetting->logSignalRate ^= 1;
      else if (incoming == '5')
      {
        if (sensorSetting->distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
          sensorSetting->distanceMode = VL53L1X_DISTANCE_MODE_LONG;
        else
          sensorSetting->distanceMode = VL53L1X_DISTANCE_MODE_SHORT;

        //Error check
        if (sensorSetting->distanceMode == VL53L1X_DISTANCE_MODE_LONG && sensorSetting->intermeasurementPeriod < 140)
        {
          sensorSetting->intermeasurementPeriod = 140;
          SerialPrintln(F("Intermeasurement Period increased to 140ms"));
        }
      }
      else if (incoming == '6')
      {
        int min = 20;
        if (sensorSetting->distanceMode == VL53L1X_DISTANCE_MODE_LONG)
          min = 140;


        SerialPrintf2("Set timing budget (%d to 1000ms): ", min);
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < min || amt > 1000)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->intermeasurementPeriod = amt;
      }
      else if (incoming == '7')
      {
        SerialPrint(F("Set Offset in mm (0 to 4000mm): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 4000)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->offset = amt;
      }
      else if (incoming == '8')
      {
        SerialPrint(F("Set Crosstalk in Counts Per Second: "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 4000)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->crosstalk = amt;
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

}

void menuConfigure_BME280(void *configPtr)
{
  struct_BME280 *sensorSetting = (struct_BME280*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure BME280 Pressure/Humidity/Temperature Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Pressure: "));
      if (sensorSetting->logPressure == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Humidity: "));
      if (sensorSetting->logHumidity == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Altitude: "));
      if (sensorSetting->logAltitude == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logPressure ^= 1;
      else if (incoming == '3')
        sensorSetting->logHumidity ^= 1;
      else if (incoming == '4')
        sensorSetting->logAltitude ^= 1;
      else if (incoming == '5')
        sensorSetting->logTemperature ^= 1;
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
}

void menuConfigure_CCS811(void *configPtr)
{
  struct_CCS811 *sensorSetting = (struct_CCS811*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure CCS811 tVOC and CO2 Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log tVOC: "));
      if (sensorSetting->logTVOC == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log CO2: "));
      if (sensorSetting->logCO2 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logTVOC ^= 1;
      else if (incoming == '3')
        sensorSetting->logCO2 ^= 1;
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
}

void menuConfigure_LPS25HB(void *configPtr)
{
  struct_LPS25HB *sensorSetting = (struct_LPS25HB*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure LPS25HB Pressure Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Pressure: "));
      if (sensorSetting->logPressure == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logPressure ^= 1;
      else if (incoming == '3')
        sensorSetting->logTemperature ^= 1;
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

}

void menuConfigure_LPS28DFW(void *configPtr)
{
  struct_LPS28DFW *sensorSetting = (struct_LPS28DFW*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure LPS28DFW Pressure Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Pressure: "));
      if (sensorSetting->logPressure == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logPressure ^= 1;
      else if (incoming == '3')
        sensorSetting->logTemperature ^= 1;
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

}

void menuConfigure_NAU7802(void *configPtr)
{
  //Search the list of nodes looking for the one with matching config pointer
  node *temp = head;
  while (temp != NULL)
  {
    if (temp->configPtr == configPtr)
      break;

    temp = temp->next;
  }
  if (temp == NULL)
  {
    SerialPrintln(F("NAU7802 node not found. Returning."));
    for (int i = 0; i < 1000; i++)
    {
      checkBattery();
      delay(1);
    }
    return;
  }

  NAU7802 *sensor = (NAU7802 *)temp->classPtr;
  struct_NAU7802 *sensorConfig = (struct_NAU7802*)configPtr;

  openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure NAU7802 Load Cell Amplifier"));

    if (sensorConfig->log == true)
    {
      char tempStr[16];
      olaftoa(sensorConfig->calibrationFactor, tempStr, 6, sizeof(tempStr) / sizeof(char));
      SerialPrintf2("\r\nScale calibration factor: %s\r\n", tempStr);

      SerialPrintf2("Scale zero offset: %d\r\n", sensorConfig->zeroOffset);
      SerialPrintf2("Scale offset register: %d\r\n", sensor->get24BitRegister(NAU7802_OCAL1_B2));

      sensor->getWeight(true, 10); //Flush
      olaftoa(sensor->getWeight(true, sensorConfig->averageAmount), tempStr, sensorConfig->decimalPlaces, sizeof(tempStr) / sizeof(char));
      SerialPrintf2("Weight currently on scale: %s\r\n\r\n", tempStr);
    }

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorConfig->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorConfig->log == true)
    {
      SerialPrintln(F("2) Zero scale"));
      char tempStr[16];
      olaftoa(sensorConfig->calibrationWeight, tempStr, 6, sizeof(tempStr) / sizeof(char));
      SerialPrintln(F("3) Calibrate scale"));
      SerialPrintf2("4) Calibration weight: %s\r\n", tempStr);
      SerialPrintf2("5) Number of decimal places: %d\r\n", sensorConfig->decimalPlaces);
      SerialPrintf2("6) Average number of readings to take per weight read: %d\r\n", sensorConfig->averageAmount);
      int gain;
      switch (sensorConfig->gain)
      {
        case 0:
          gain = 1;
          break;
        case 1:
          gain = 2;
          break;
        case 2:
          gain = 4;
          break;
        case 3:
          gain = 8;
          break;
        case 4:
          gain = 16;
          break;
        case 5:
          gain = 32;
          break;
        case 6:
          gain = 64;
          break;
        case 7:
          gain = 128;
          break;
      }
      SerialPrintf2("7) Gain: %d\r\n", gain);
      int rate;
      switch (sensorConfig->sampleRate)
      {
        case 0:
          rate = 10;
          break;
        case 1:
          rate = 20;
          break;
        case 2:
          rate = 40;
          break;
        case 3:
          rate = 80;
          break;
        case 7:
          rate = 320;
          break;
      }
      SerialPrintf2("8) Sample rate: %d\r\n", rate);
      float LDO;
      switch (sensorConfig->LDO)
      {
        case 4:
          LDO = 3.3;
          break;
        case 5:
          LDO = 3.0;
          break;
        case 6:
          LDO = 2.7;
          break;
        case 7:
          LDO = 2.4;
          break;
      }
      olaftoa(LDO, tempStr, 1, sizeof(tempStr) / sizeof(char));
      SerialPrintf2("9) LDO voltage: %s\r\n", tempStr);
      SerialPrint(F("10) Calibration mode: "));
      if (sensorConfig->calibrationMode == 0) SerialPrintln(F("None"));
      else if (sensorConfig->calibrationMode == 1) SerialPrintln(F("Internal"));
      else SerialPrintln(F("External"));
    }

    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
    {
      sensorConfig->log ^= 1;
    }
    else if (sensorConfig->log == true)
    {
      if (incoming == 2)
      {
        //Gives user the ability to set a known weight on the scale and calculate a calibration factor
        SerialPrintln(F(""));
        SerialPrintln(F("Zero scale"));

        SerialPrintln(F("Setup scale with no weight on it. Press a key when ready."));
        waitForInput();

        sensor->getWeight(true, 10); //Flush

        if (sensorConfig->calibrationMode == 2) //External calibration
        {
          sensor->calibrateAFE(NAU7802_CALMOD_OFFSET); //External offset calibration

          sensorConfig->offsetReg = sensor->get24BitRegister(NAU7802_OCAL1_B2); // Save new offset
          sensorConfig->gainReg = sensor->get32BitRegister(NAU7802_GCAL1_B3); // This should not have changed, but read it anyway

          sensor->getWeight(true, 10); //Flush
        }

        sensor->calculateZeroOffset(sensorConfig->averageAmount); //Zero or Tare the scale. With external calibration, this should be ~zero

        sensorConfig->zeroOffset = sensor->getZeroOffset();
      }
      else if (incoming == 3)
      {
        //Gives user the ability to set a known weight on the scale and calculate a calibration factor
        SerialPrintln(F(""));
        SerialPrintln(F("Scale calibration"));

        SerialPrintln(F("Place calibration weight on scale. Press a key when weight is in place and stable."));
        waitForInput();

        sensor->getWeight(true, 10); //Flush

        sensor->calculateCalibrationFactor(sensorConfig->calibrationWeight, sensorConfig->averageAmount); //Tell the library how much weight is currently on it

        sensorConfig->calibrationFactor = sensor->getCalibrationFactor();
      }
      else if (incoming == 4)
      {
        SerialPrint(F("Please enter the weight, without units, for scale calibration (3) - (for example '100.0'): "));

        //Read user input
        double newWeight = getDouble(menuTimeout); //Timeout after x seconds
        if ((newWeight != STATUS_GETNUMBER_TIMEOUT) && (newWeight != STATUS_PRESSED_X))
          sensorConfig->calibrationWeight = (float)newWeight;

        SerialPrintln(F(""));
      }
      else if (incoming == 5)
      {
        SerialPrint(F("Enter number of decimal places to print (1 to 10): "));
        int places = getNumber(menuTimeout);
        if (places < 1 || places > 10)
        {
          SerialPrintln(F("Error: Decimal places out of range"));
        }
        else
        {
          sensorConfig->decimalPlaces = places;
        }
      }
      else if (incoming == 6)
      {
        //Limit number of readings to the sample rate so that the getWeight doesn't time out
        SerialPrint(F("Enter number of readings to take per weight read (>= 1, < Sample Rate): "));
        int rate;
        switch (sensorConfig->sampleRate)
        {
          case 0:
            rate = 10;
            break;
          case 1:
            rate = 20;
            break;
          case 2:
            rate = 40;
            break;
          case 3:
            rate = 80;
            break;
          case 7:
            rate = 320;
            break;
        }
        int amt = getNumber(menuTimeout);
        if (amt < 1 || amt >= rate)
        {
          SerialPrintln(F("Error: Average number of readings out of range"));
        }
        else
        {
          sensorConfig->averageAmount = amt;
        }
      }
      else if (incoming == 7)
      {
        sensorConfig->gain += 1;
        if (sensorConfig->gain == 8)
          sensorConfig->gain = 0;

        sensor->setGain(sensorConfig->gain);

        if (sensorConfig->calibrationMode == 1) //Internal calibration
        {
          sensor->getWeight(true, 10); //Flush

          sensor->calibrateAFE(NAU7802_CALMOD_INTERNAL); //Recalibrate after changing gain / sample rate          
        }

        SerialPrintln(F("\r\n\r\nGain updated. Please zero and calibrate the scale\r\n\r\n"));
      }
      else if (incoming == 8)
      {
        sensorConfig->sampleRate += 1;
        if (sensorConfig->sampleRate == 4)
          sensorConfig->sampleRate = 7;
        if (sensorConfig->sampleRate == 8)
          sensorConfig->sampleRate = 0;

        sensor->setSampleRate(sensorConfig->sampleRate);

        if (sensorConfig->calibrationMode == 1) //Internal calibration
        {
          sensor->getWeight(true, 10); //Flush

          sensor->calibrateAFE(NAU7802_CALMOD_INTERNAL); //Recalibrate after changing gain / sample rate          
        }

        // Limit averageAmount (to prevent getWeight timing out after 1s)
        if ((sensorConfig->sampleRate) == 0 && (sensorConfig->averageAmount > 9))
          sensorConfig->averageAmount = 9;
        else if ((sensorConfig->sampleRate) == 1 && (sensorConfig->averageAmount > 19))
          sensorConfig->averageAmount = 19;
        else if ((sensorConfig->sampleRate) == 2 && (sensorConfig->averageAmount > 39))
          sensorConfig->averageAmount = 39;
        else if ((sensorConfig->sampleRate) == 3 && (sensorConfig->averageAmount > 79))
          sensorConfig->averageAmount = 79;
        else if (sensorConfig->averageAmount > 319)
          sensorConfig->averageAmount = 319;

        SerialPrintln(F("\r\n\r\nSample rate updated. Please zero and calibrate the scale\r\n\r\n"));
      }
      else if (incoming == 9)
      {
        sensorConfig->LDO += 1;
        if (sensorConfig->LDO == 8)
          sensorConfig->LDO = 4;

        sensor->setLDO(sensorConfig->LDO);

        if (sensorConfig->calibrationMode == 1) //Internal calibration
        {
          delay(sensor->getLDORampDelay()); // Wait for LDO to ramp before attempting calibrateAFE

          sensor->getWeight(true, 10); //Flush

          sensor->calibrateAFE(NAU7802_CALMOD_INTERNAL); //Recalibrate after changing gain / sample rate          
        }

        SerialPrintln(F("\r\n\r\nLDO updated. Please zero and calibrate the scale\r\n\r\n"));
      }
      else if (incoming == 10)
      {
        sensorConfig->calibrationMode += 1;
        if (sensorConfig->calibrationMode == 3)
          sensorConfig->calibrationMode = 0;

        sensor->reset();
        sensor->powerUp();
        sensor->setLDO(sensorConfig->LDO);
        sensor->setGain(sensorConfig->gain);
        sensor->setSampleRate(sensorConfig->sampleRate);
        //Turn off CLK_CHP. From 9.1 power on sequencing.
        uint8_t adc = sensor->getRegister(NAU7802_ADC);
        adc |= 0x30;
        sensor->setRegister(NAU7802_ADC, adc);
        sensor->setBit(NAU7802_PGA_PWR_PGA_CAP_EN, NAU7802_PGA_PWR); //Enable 330pF decoupling cap on chan 2. From 9.14 application circuit note.
        sensor->clearBit(NAU7802_PGA_LDOMODE, NAU7802_PGA); //Ensure LDOMODE bit is clear - improved accuracy and higher DC gain, with ESR < 1 ohm
        sensor->setCalibrationFactor(sensorConfig->calibrationFactor);
        sensor->setZeroOffset(sensorConfig->zeroOffset);

        delay(sensor->getLDORampDelay()); // Wait for LDO to ramp before attempting calibrateAFE

        if (sensorConfig->calibrationMode == 1) //Internal calibration
        {
          sensor->getWeight(true, 10); //Flush

          sensor->calibrateAFE(NAU7802_CALMOD_INTERNAL); //Recalibrate after changing gain / sample rate          
        }

        sensor->getWeight(true, 10); //Flush

        SerialPrintln(F("\r\n\r\nCalibration updated. Please zero and calibrate the scale\r\n\r\n"));
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_ublox(void *configPtr)
{
  struct_ublox *sensorSetting = (struct_ublox*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure u-blox GPS Receiver"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log GPS Date: "));
      if (sensorSetting->logDate == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log GPS Time: "));
      if (sensorSetting->logTime == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Longitude/Latitude: "));
      if (sensorSetting->logPosition == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log Altitude: "));
      if (sensorSetting->logAltitude == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("6) Log Altitude Mean Sea Level: "));
      if (sensorSetting->logAltitudeMSL == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("7) Log Satellites In View: "));
      if (sensorSetting->logSIV == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("8) Log Fix Type: "));
      if (sensorSetting->logFixType == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("9) Log Carrier Solution: "));
      if (sensorSetting->logCarrierSolution == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("10) Log Ground Speed: "));
      if (sensorSetting->logGroundSpeed == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("11) Log Heading of Motion: "));
      if (sensorSetting->logHeadingOfMotion == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("12) Log Position Dilution of Precision (pDOP): "));
      if (sensorSetting->logpDOP == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("13) Log Interval Time Of Week (iTOW): "));
      if (sensorSetting->logiTOW == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrintf2("14) Set I2C Interface Speed (u-blox modules have pullups built in. Remove *all* I2C pullups to achieve 400kHz): %d\r\n", sensorSetting->i2cSpeed);

      SerialPrint(F("15) Use autoPVT: "));
      if (sensorSetting->useAutoPVT == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

      SerialPrintln(F("16) Reset GNSS to factory defaults"));

      SerialFlush();
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
    {
      sensorSetting->log ^= 1;
    }
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logDate ^= 1;
      else if (incoming == 3)
        sensorSetting->logTime ^= 1;
      else if (incoming == 4)
        sensorSetting->logPosition ^= 1;
      else if (incoming == 5)
        sensorSetting->logAltitude ^= 1;
      else if (incoming == 6)
        sensorSetting->logAltitudeMSL ^= 1;
      else if (incoming == 7)
        sensorSetting->logSIV ^= 1;
      else if (incoming == 8)
        sensorSetting->logFixType ^= 1;
      else if (incoming == 9)
        sensorSetting->logCarrierSolution ^= 1;
      else if (incoming == 10)
        sensorSetting->logGroundSpeed ^= 1;
      else if (incoming == 11)
        sensorSetting->logHeadingOfMotion ^= 1;
      else if (incoming == 12)
        sensorSetting->logpDOP ^= 1;
      else if (incoming == 13)
        sensorSetting->logiTOW ^= 1;
      else if (incoming == 14)
      {
        if (sensorSetting->i2cSpeed == 100000)
          sensorSetting->i2cSpeed = 400000;
        else
          sensorSetting->i2cSpeed = 100000;
      }
      else if (incoming == 15)
        sensorSetting->useAutoPVT ^= 1;
      else if (incoming == 16)
      {
        SerialPrintln(F("Reset GNSS module to factory defaults. This will take 5 seconds to complete."));
        SerialPrintln(F("Are you sure? Press 'y' to confirm: "));
        byte bContinue = getByteChoice(menuTimeout);
        if (bContinue == 'y')
        {
          gnssFactoryDefault();
        }
        else
          SerialPrintln(F("Reset GNSS aborted"));
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

bool isUbloxAttached()
{
  //Step through node list
  node *temp = head;

  while (temp != NULL)
  {
    switch (temp->deviceType)
    {
      case DEVICE_GPS_UBLOX:
        return (true);
    }
    temp = temp->next;
  }

  return (false);
}

void getUbloxDateTime(int &year, int &month, int &day, int &hour, int &minute, int &second, int &millisecond, bool &dateValid, bool &timeValid)
{
  //Step through node list
  node *temp = head;

  while (temp != NULL)
  {
    switch (temp->deviceType)
    {
      case DEVICE_GPS_UBLOX:
        {
          openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

          setQwiicPullups(0); //Disable pullups to minimize CRC issues

          SFE_UBLOX_GNSS *nodeDevice = (SFE_UBLOX_GNSS *)temp->classPtr;
          struct_ublox *nodeSetting = (struct_ublox *)temp->configPtr;

          //If autoPVT is enabled, flush the data to make sure we get fresh date and time
          if (nodeSetting->useAutoPVT) nodeDevice->flushPVT();

          //Get latested date/time from GPS
          //These will be extracted from a single PVT packet
          year = nodeDevice->getYear();
          month = nodeDevice->getMonth();
          day = nodeDevice->getDay();
          hour = nodeDevice->getHour();
          minute = nodeDevice->getMinute();
          second = nodeDevice->getSecond();
          dateValid = nodeDevice->getDateValid();
          timeValid = nodeDevice->getTimeValid();
          millisecond = nodeDevice->getMillisecond();

          setQwiicPullups(settings.qwiicBusPullUps); //Re-enable pullups
        }
    }
    temp = temp->next;
  }
}

void gnssFactoryDefault(void)
{
  //Step through node list
  node *temp = head;

  while (temp != NULL)
  {
    switch (temp->deviceType)
    {
      case DEVICE_GPS_UBLOX:
        {
          openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed
          
          setQwiicPullups(0); //Disable pullups to minimize CRC issues

          SFE_UBLOX_GNSS *nodeDevice = (SFE_UBLOX_GNSS *)temp->classPtr;
          struct_ublox *nodeSetting = (struct_ublox *)temp->configPtr;

          //Reset the module to the factory defaults
          nodeDevice->factoryDefault();

          delay(5000); //Blocking delay to allow module to reset

          nodeDevice->setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
          nodeDevice->saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the current ioPortsettings to flash and BBR

          setQwiicPullups(settings.qwiicBusPullUps); //Re-enable pullups
        }
    }
    temp = temp->next;
  }
}

void menuConfigure_MCP9600(void *configPtr)
{
  struct_MCP9600 *sensorSetting = (struct_MCP9600*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure MCP9600 Thermocouple Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Thermocouple Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Ambient Temperature: "));
      if (sensorSetting->logAmbientTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logTemperature ^= 1;
      else if (incoming == '3')
        sensorSetting->logAmbientTemperature ^= 1;
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

}

void menuConfigure_VCNL4040(void *configPtr)
{
  struct_VCNL4040 *sensorSetting = (struct_VCNL4040*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure VCNL4040 Proximity Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Proximity: "));
      if (sensorSetting->logProximity == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Ambient Light: "));
      if (sensorSetting->logAmbientLight == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrintf2("4) Set LED Current: %d\r\n", sensorSetting->LEDCurrent);
      SerialPrintf2("5) Set IR Duty Cycle: %d\r\n", sensorSetting->IRDutyCycle);
      SerialPrintf2("6) Set Proximity Integration Time: %d\r\n", sensorSetting->proximityIntegrationTime);
      SerialPrintf2("7) Set Ambient Integration Time: %d\r\n", sensorSetting->ambientIntegrationTime);
      SerialPrintf2("8) Set Resolution (bits): %d\r\n", sensorSetting->resolution);
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logProximity ^= 1;
      else if (incoming == '3')
        sensorSetting->logAmbientLight ^= 1;
      else if (incoming == '4')
      {
        SerialPrint(F("Enter current (mA) for IR LED drive (50 to 200mA): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 50 || amt > 200)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->LEDCurrent = amt;
      }
      else if (incoming == '5')
      {
        SerialPrint(F("Enter IR Duty Cycle (40 to 320): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 40 || amt > 320)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->IRDutyCycle = amt;
      }
      else if (incoming == '6')
      {
        SerialPrint(F("Enter Proximity Integration Time (1 to 8): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 1 || amt > 8)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->proximityIntegrationTime = amt;
      }
      else if (incoming == '7')
      {
        SerialPrint(F("Enter Ambient Light Integration Time (80 to 640ms): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 80 || amt > 640)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->ambientIntegrationTime = amt;
      }
      else if (incoming == '8')
      {
        SerialPrint(F("Enter Proximity Resolution (12 or 16 bit): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt == 12 || amt == 16)
          sensorSetting->resolution = amt;
        else
          SerialPrintln(F("Error: Out of range"));
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

}

void menuConfigure_TMP117(void *configPtr)
{
  struct_TMP117 *sensorSetting = (struct_TMP117*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure TMP117 Precision Temperature Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

}


void menuConfigure_SGP30(void *configPtr)
{
  struct_SGP30 *sensorSetting = (struct_SGP30*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure SGP30 tVOC and CO2 Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log tVOC: "));
      if (sensorSetting->logTVOC == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log CO2: "));
      if (sensorSetting->logCO2 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log H2: "));
      if (sensorSetting->logH2 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log Ethanol: "));
      if (sensorSetting->logEthanol == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logTVOC ^= 1;
      else if (incoming == '3')
        sensorSetting->logCO2 ^= 1;
      else if (incoming == '4')
        sensorSetting->logH2 ^= 1;
      else if (incoming == '5')
        sensorSetting->logEthanol ^= 1;
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
}

void menuConfigure_VEML6075(void *configPtr)
{
  struct_VEML6075 *sensorSetting = (struct_VEML6075*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure VEML6075 UV Index Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log UVA: "));
      if (sensorSetting->logUVA == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log UVB: "));
      if (sensorSetting->logUVB == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log UV Index: "));
      if (sensorSetting->logUVIndex == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logUVA ^= 1;
      else if (incoming == '3')
        sensorSetting->logUVB ^= 1;
      else if (incoming == '4')
        sensorSetting->logUVIndex ^= 1;
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

}

void menuConfigure_VEML7700(void *configPtr)
{
  struct_VEML7700 *sensorSetting = (struct_VEML7700*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure VEML7700 Ambient Light Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

}

void menuConfigure_MS5637(void *configPtr)
{
  struct_MS5637 *sensorSetting = (struct_MS5637*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure MS5637 Pressure Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Pressure: "));
      if (sensorSetting->logPressure == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logPressure ^= 1;
      else if (incoming == '3')
        sensorSetting->logTemperature ^= 1;
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

}


void menuConfigure_SCD30(void *configPtr)
{
  //Search the list of nodes looking for the one with matching config pointer
  node *temp = head;
  while (temp != NULL)
  {
    if (temp->configPtr == configPtr)
      break;

    temp = temp->next;
  }
  if (temp == NULL)
  {
    SerialPrintln(F("SCD30 node not found. Returning."));
    for (int i = 0; i < 1000; i++)
    {
      checkBattery();
      delay(1);
    }
    return;
  }

  SCD30 *sensor = (SCD30 *)temp->classPtr;
  struct_SCD30 *sensorSetting = (struct_SCD30*)configPtr;

  openConnection(temp->muxAddress, temp->portNumber); //Connect to this device through muxes as needed

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure SCD30 CO2 and Humidity Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log CO2: "));
      if (sensorSetting->logCO2 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Humidity: "));
      if (sensorSetting->logHumidity == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrintf2("5) Set Measurement Interval: %d\r\n", sensorSetting->measurementInterval);
      SerialPrintf2("6) Set Altitude Compensation: %d\r\n", sensorSetting->altitudeCompensation);
      SerialPrintf2("7) Set Ambient Pressure: %d\r\n", sensorSetting->ambientPressure);
      SerialPrintf2("8) Set Temperature Offset: %d\r\n", sensorSetting->temperatureOffset);
      SerialPrintln(F("9) Set FRC Calibration CO2"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logCO2 ^= 1;
      else if (incoming == '3')
        sensorSetting->logHumidity ^= 1;
      else if (incoming == '4')
        sensorSetting->logTemperature ^= 1;
      else if (incoming == '5')
      {
        SerialPrint(F("Enter the seconds between measurements (2 to 1800): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 2 || amt > 1800)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->measurementInterval = amt;
      }
      else if (incoming == '6')
      {
        SerialPrint(F("Enter the Altitude Compensation in meters (0 to 10000): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 10000)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->altitudeCompensation = amt;
      }
      else if (incoming == '7')
      {
        SerialPrint(F("Enter Ambient Pressure in mBar (700 to 1200): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 700 || amt > 1200)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->ambientPressure = amt;
      }
      else if (incoming == '8')
      {
        SerialPrint(F("The current temperature offset read from the sensor is: "));
        Serial.print(sensor->getTemperatureOffset(), 2);
        if (settings.useTxRxPinsForTerminal == true)
          Serial1.print(sensor->getTemperatureOffset(), 2);
        SerialPrintln(F("C"));
        SerialPrint(F("Enter new temperature offset in C (-50 to 50): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < -50 || amt > 50)
          sensorSetting->temperatureOffset = amt;
        else
          SerialPrintln(F("Error: Out of range"));
      }
      else if (incoming == '9')
      {
        SerialPrint(F("Enter Calibration CO2 in ppm (400 to 2000): "));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 400 || amt > 2000)
          SerialPrintln(F("Error: Out of range"));
        else
        {
          sensorSetting->calibrationConcentration = amt;
          sensorSetting->applyCalibrationConcentration = true;
        }
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

}


void menuConfigure_MS8607(void *configPtr)
{
  struct_MS8607 *sensorSetting = (struct_MS8607*)configPtr;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure MS8607 Pressure Humidity Temperature (PHT) Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Pressure: "));
      if (sensorSetting->logPressure == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Humidity: "));
      if (sensorSetting->logHumidity == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Heater: "));
      if (sensorSetting->enableHeater == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("6) Set Pressure Resolution: "));
      if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_256)
        SerialPrint(F("0.11"));
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_512)
        SerialPrint(F("0.062"));
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_1024)
        SerialPrint(F("0.039"));
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_2048)
        SerialPrint(F("0.028"));
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_4096)
        SerialPrint(F("0.021"));
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_8192)
        SerialPrint(F("0.016"));
      SerialPrintln(F(" mbar"));

      SerialPrint(F("7) Set Humidity Resolution: "));
      if (sensorSetting->humidityResolution == MS8607_humidity_resolution_8b)
        SerialPrint(F("8"));
      else if (sensorSetting->humidityResolution == MS8607_humidity_resolution_10b)
        SerialPrint(F("10"));
      else if (sensorSetting->humidityResolution == MS8607_humidity_resolution_11b)
        SerialPrint(F("11"));
      else if (sensorSetting->humidityResolution == MS8607_humidity_resolution_12b)
        SerialPrint(F("12"));
      SerialPrintln(F(" bits"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logPressure ^= 1;
      else if (incoming == '3')
        sensorSetting->logHumidity ^= 1;
      else if (incoming == '4')
        sensorSetting->logTemperature ^= 1;
      else if (incoming == '5')
        sensorSetting->enableHeater ^= 1;
      else if (incoming == '6')
      {
        SerialPrintln(F("Set Pressure Resolution:"));
        SerialPrintln(F("1) 0.11 mbar"));
        SerialPrintln(F("2) 0.062 mbar"));
        SerialPrintln(F("3) 0.039 mbar"));
        SerialPrintln(F("4) 0.028 mbar"));
        SerialPrintln(F("5) 0.021 mbar"));
        SerialPrintln(F("6) 0.016 mbar"));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt >= 1 && amt <= 6)
          sensorSetting->pressureResolution = (MS8607_pressure_resolution)(amt - 1);
        else
          SerialPrintln(F("Error: Out of range"));
      }
      else if (incoming == '7')
      {
        SerialPrintln(F("Set Humidity Resolution:"));
        SerialPrintln(F("1) 8 bit"));
        SerialPrintln(F("2) 10 bit"));
        SerialPrintln(F("3) 11 bit"));
        SerialPrintln(F("4) 12 bit"));
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt >= 1 && amt <= 4)
        {
          //Unfortunately these enums aren't sequential so we have to lookup
          if (amt == 1) sensorSetting->humidityResolution = MS8607_humidity_resolution_8b;
          if (amt == 2) sensorSetting->humidityResolution = MS8607_humidity_resolution_10b;
          if (amt == 3) sensorSetting->humidityResolution = MS8607_humidity_resolution_11b;
          if (amt == 4) sensorSetting->humidityResolution = MS8607_humidity_resolution_12b;
        }
        else
          SerialPrintln(F("Error: Out of range"));
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
}

void menuConfigure_AHT20(void *configPtr)
{
  struct_AHT20 *sensorSetting = (struct_AHT20*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure AHT20 Humidity Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Humidity: "));
      if (sensorSetting->logHumidity == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logHumidity ^= 1;
      else if (incoming == '3')
        sensorSetting->logTemperature ^= 1;
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
}

void menuConfigure_SHTC3(void *configPtr)
{
  struct_SHTC3 *sensorSetting = (struct_SHTC3*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure SHTC3 Humidity Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Humidity: "));
      if (sensorSetting->logHumidity == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == '2')
        sensorSetting->logHumidity ^= 1;
      else if (incoming == '3')
        sensorSetting->logTemperature ^= 1;
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
}

void menuConfigure_ADS122C04(void *configPtr)
{
  struct_ADS122C04 *sensorSetting = (struct_ADS122C04*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure ADS122C04 ADC (Qwiic PT100)"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Centigrade: "));
      if (sensorSetting->logCentigrade == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Fahrenheit: "));
      if (sensorSetting->logFahrenheit == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Internal Temperature: "));
      if (sensorSetting->logInternalTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log Raw Voltage: "));
      if (sensorSetting->logRawVoltage == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("6) Use 4-Wire Mode: "));
      if (sensorSetting->useFourWireMode == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("7) Use 3-Wire Mode: "));
      if (sensorSetting->useThreeWireMode == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("8) Use 2-Wire Mode: "));
      if (sensorSetting->useTwoWireMode == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("9) Use 4-Wire High Temperature Mode: "));
      if (sensorSetting->useFourWireHighTemperatureMode == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("10) Use 3-Wire High Temperature Mode: "));
      if (sensorSetting->useThreeWireHighTemperatureMode == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("11) Use 2-Wire High Temperature Mode: "));
      if (sensorSetting->useTwoWireHighTemperatureMode == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logCentigrade ^= 1;
      else if (incoming == 3)
        sensorSetting->logFahrenheit ^= 1;
      else if (incoming == 4)
        sensorSetting->logInternalTemperature ^= 1;
      else if (incoming == 5)
        sensorSetting->logRawVoltage ^= 1;
      else if (incoming == 6)
      {
        sensorSetting->useFourWireMode = true;
        sensorSetting->useThreeWireMode = false;
        sensorSetting->useTwoWireMode = false;
        sensorSetting->useFourWireHighTemperatureMode = false;
        sensorSetting->useThreeWireHighTemperatureMode = false;
        sensorSetting->useTwoWireHighTemperatureMode = false;
      }
      else if (incoming == 7)
      {
        sensorSetting->useFourWireMode = false;
        sensorSetting->useThreeWireMode = true;
        sensorSetting->useTwoWireMode = false;
        sensorSetting->useFourWireHighTemperatureMode = false;
        sensorSetting->useThreeWireHighTemperatureMode = false;
        sensorSetting->useTwoWireHighTemperatureMode = false;
      }
      else if (incoming == 8)
      {
        sensorSetting->useFourWireMode = false;
        sensorSetting->useThreeWireMode = false;
        sensorSetting->useTwoWireMode = true;
        sensorSetting->useFourWireHighTemperatureMode = false;
        sensorSetting->useThreeWireHighTemperatureMode = false;
        sensorSetting->useTwoWireHighTemperatureMode = false;
      }
      else if (incoming == 9)
      {
        sensorSetting->useFourWireMode = false;
        sensorSetting->useThreeWireMode = false;
        sensorSetting->useTwoWireMode = false;
        sensorSetting->useFourWireHighTemperatureMode = true;
        sensorSetting->useThreeWireHighTemperatureMode = false;
        sensorSetting->useTwoWireHighTemperatureMode = false;
      }
      else if (incoming == 10)
      {
        sensorSetting->useFourWireMode = false;
        sensorSetting->useThreeWireMode = false;
        sensorSetting->useTwoWireMode = false;
        sensorSetting->useFourWireHighTemperatureMode = false;
        sensorSetting->useThreeWireHighTemperatureMode = true;
        sensorSetting->useTwoWireHighTemperatureMode = false;
      }
      else if (incoming == 11)
      {
        sensorSetting->useFourWireMode = false;
        sensorSetting->useThreeWireMode = false;
        sensorSetting->useTwoWireMode = false;
        sensorSetting->useFourWireHighTemperatureMode = false;
        sensorSetting->useThreeWireHighTemperatureMode = false;
        sensorSetting->useTwoWireHighTemperatureMode = true;
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_MPR0025PA1(void *configPtr)
{
  struct_MPR0025PA1 *sensorSetting = (struct_MPR0025PA1*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure MPR MicroPressure Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrintf2("2) Minimum PSI: %d\r\n", sensorSetting->minimumPSI);

      SerialPrintf2("3) Maximum PSI: %d\r\n", sensorSetting->maximumPSI);

      SerialPrint(F("4) Use PSI: "));
      if (sensorSetting->usePSI == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

      SerialPrint(F("5) Use Pa: "));
      if (sensorSetting->usePA == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

      SerialPrint(F("6) Use kPa: "));
      if (sensorSetting->useKPA == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

      SerialPrint(F("7) Use torr: "));
      if (sensorSetting->useTORR == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

      SerialPrint(F("8) Use inHg: "));
      if (sensorSetting->useINHG == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

      SerialPrint(F("9) Use atm: "));
      if (sensorSetting->useATM == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

      SerialPrint(F("10) Use bar: "));
      if (sensorSetting->useBAR == true) SerialPrintln(F("Yes"));
      else SerialPrintln(F("No"));

    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
      {
        SerialPrint(F("Enter the sensor minimum pressure in PSI (this should be 0 for the MPR0025PA): "));
        int minPSI = getNumber(menuTimeout); //x second timeout
        if (minPSI < 0 || minPSI > 30)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->minimumPSI = minPSI;
      }
      else if (incoming == 3)
      {
        SerialPrint(F("Enter the sensor maximum pressure in PSI (this should be 25 for the MPR0025PA): "));
        int maxPSI = getNumber(menuTimeout); //x second timeout
        if (maxPSI < 0 || maxPSI > 30)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->maximumPSI = maxPSI;
      }
      else if (incoming == 4)
      {
        sensorSetting->usePSI = true;
        sensorSetting->usePA = false;
        sensorSetting->useKPA = false;
        sensorSetting->useTORR = false;
        sensorSetting->useINHG = false;
        sensorSetting->useATM = false;
        sensorSetting->useBAR = false;
      }
      else if (incoming == 5)
      {
        sensorSetting->usePSI = false;
        sensorSetting->usePA = true;
        sensorSetting->useKPA = false;
        sensorSetting->useTORR = false;
        sensorSetting->useINHG = false;
        sensorSetting->useATM = false;
        sensorSetting->useBAR = false;
      }
      else if (incoming == 6)
      {
        sensorSetting->usePSI = false;
        sensorSetting->usePA = false;
        sensorSetting->useKPA = true;
        sensorSetting->useTORR = false;
        sensorSetting->useINHG = false;
        sensorSetting->useATM = false;
        sensorSetting->useBAR = false;
      }
      else if (incoming == 7)
      {
        sensorSetting->usePSI = false;
        sensorSetting->usePA = false;
        sensorSetting->useKPA = false;
        sensorSetting->useTORR = true;
        sensorSetting->useINHG = false;
        sensorSetting->useATM = false;
        sensorSetting->useBAR = false;
      }
      else if (incoming == 8)
      {
        sensorSetting->usePSI = false;
        sensorSetting->usePA = false;
        sensorSetting->useKPA = false;
        sensorSetting->useTORR = false;
        sensorSetting->useINHG = true;
        sensorSetting->useATM = false;
        sensorSetting->useBAR = false;
      }
      else if (incoming == 9)
      {
        sensorSetting->usePSI = false;
        sensorSetting->usePA = false;
        sensorSetting->useKPA = false;
        sensorSetting->useTORR = false;
        sensorSetting->useINHG = false;
        sensorSetting->useATM = true;
        sensorSetting->useBAR = false;
      }
      else if (incoming == 10)
      {
        sensorSetting->usePSI = false;
        sensorSetting->usePA = false;
        sensorSetting->useKPA = false;
        sensorSetting->useTORR = false;
        sensorSetting->useINHG = false;
        sensorSetting->useATM = false;
        sensorSetting->useBAR = true;
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_SNGCJA5(void *configPtr)
{
  struct_SNGCJA5 *sensorSetting = (struct_SNGCJA5*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure SNGCJA5 Particle Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Particle Mass Density 1.0um (ug/m^3): "));
      if (sensorSetting->logPM1 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Particle Mass Density 2.5um (ug/m^3): "));
      if (sensorSetting->logPM25 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Particle Mass Density 10.0um (ug/m^3): "));
      if (sensorSetting->logPM10 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log Particle Count 0.5um: "));
      if (sensorSetting->logPC05 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("6) Log Particle Count 1.0um: "));
      if (sensorSetting->logPC1 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("7) Log Particle Count 2.5um: "));
      if (sensorSetting->logPC25 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("8) Log Particle Count 5.0um: "));
      if (sensorSetting->logPC50 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("9) Log Particle Count 7.5um: "));
      if (sensorSetting->logPC75 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("10) Log Particle Count 10.0um: "));
      if (sensorSetting->logPC10 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("11) Log Combined Sensor Status: "));
      if (sensorSetting->logSensorStatus == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("12) Log PhotoDiode Status: "));
      if (sensorSetting->logPDStatus == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("13) Log LaserDiode Status: "));
      if (sensorSetting->logLDStatus == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("14) Log Fan Status: "));
      if (sensorSetting->logFanStatus == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logPM1 ^= 1;
      else if (incoming == 3)
        sensorSetting->logPM25 ^= 1;
      else if (incoming == 4)
        sensorSetting->logPM10 ^= 1;
      else if (incoming == 5)
        sensorSetting->logPC05 ^= 1;
      else if (incoming == 6)
        sensorSetting->logPC1 ^= 1;
      else if (incoming == 7)
        sensorSetting->logPC25 ^= 1;
      else if (incoming == 8)
        sensorSetting->logPC50 ^= 1;
      else if (incoming == 9)
        sensorSetting->logPC75 ^= 1;
      else if (incoming == 10)
        sensorSetting->logPC10 ^= 1;
      else if (incoming == 11)
        sensorSetting->logSensorStatus ^= 1;
      else if (incoming == 12)
        sensorSetting->logPDStatus ^= 1;
      else if (incoming == 13)
        sensorSetting->logLDStatus ^= 1;
      else if (incoming == 14)
        sensorSetting->logFanStatus ^= 1;
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_SGP40(void *configPtr)
{
  struct_SGP40 *sensorSetting = (struct_SGP40*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure SGP40 VOC Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log VOC: "));
      if (sensorSetting->logVOC == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrintf2("3) Sensor Compensation: Relative Humidity (%): %d\r\n", sensorSetting->RH);

      SerialPrintf2("4) Sensor Compensation: Temperature (C): %d\r\n", sensorSetting->T);
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logVOC ^= 1;
      else if (incoming == 3)
      {
        SerialPrint(F("Enter the %RH for sensor compensation (0 to 100): "));
        int RH = getNumber(menuTimeout); //x second timeout
        if (RH < 0 || RH > 100)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->RH = RH;
      }
      else if (incoming == 4)
      {
        SerialPrint(F("Enter the temperature (C) for sensor compensation (-45 to 130): "));
        int T = getNumber(menuTimeout); //x second timeout
        if (T < -45 || T > 130)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->T = T;
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_SDP3X(void *configPtr)
{
  struct_SDP3X *sensorSetting = (struct_SDP3X*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure SDP3X Differential Pressure Sensor"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Pressure: "));
      if (sensorSetting->logPressure == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Temperature Compensation: "));
      if (sensorSetting->massFlow == true) SerialPrintln(F("Mass Flow"));
      else SerialPrintln(F("Differential Pressure"));

      SerialPrint(F("5) Measurement Averaging: "));
      if (sensorSetting->averaging == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logPressure ^= 1;
      else if (incoming == 3)
        sensorSetting->logTemperature ^= 1;
      else if (incoming == 4)
        sensorSetting->massFlow ^= 1;
      else if (incoming == 5)
        sensorSetting->averaging ^= 1;
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_MS5837(void *configPtr)
{
  struct_MS5837 *sensorSetting = (struct_MS5837*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure MS5837 Pressure Sensor"));

    SerialPrint(F("Sensor Model: "));
    if (sensorSetting->model == 1) SerialPrintln(F("MS5837-02BA / BlueRobotics Bar02: 2 Bar Absolute / 10m Depth"));
    else SerialPrintln(F("MS5837-30BA / BlueRobotics Bar30: 30 Bar Absolute / 300m Depth"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      char tempStr[16];

      SerialPrint(F("2) Log Pressure: "));
      if (sensorSetting->logPressure == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Depth: "));
      if (sensorSetting->logDepth == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log Altitude: "));
      if (sensorSetting->logAltitude == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      olaftoa(sensorSetting->fluidDensity, tempStr, 1, sizeof(tempStr) / sizeof(char));
      SerialPrintf2("6) Fluid Density (kg/m^3): %s\r\n", tempStr);

      olaftoa(sensorSetting->conversion, tempStr, 3, sizeof(tempStr) / sizeof(char));
      SerialPrintf2("7) Pressure Conversion Factor: %s\r\n", tempStr);
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logPressure ^= 1;
      else if (incoming == 3)
        sensorSetting->logTemperature ^= 1;
      else if (incoming == 4)
        sensorSetting->logDepth ^= 1;
      else if (incoming == 5)
        sensorSetting->logAltitude ^= 1;
      else if (incoming == 6)
      {
        SerialPrint(F("Enter the Fluid Density (kg/m^3): "));
        double FD = getDouble(menuTimeout); //x second timeout
        sensorSetting->fluidDensity = (float)FD;
      }
      else if (incoming == 7)
      {
        SerialPrint(F("Enter the Pressure Conversion Factor: "));
        double PCF = getDouble(menuTimeout); //x second timeout
        sensorSetting->conversion = (float)PCF;
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_QWIIC_BUTTON(void *configPtr)
{
  struct_QWIIC_BUTTON *sensorSetting = (struct_QWIIC_BUTTON*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Qwiic Button"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Button Presses: "));
      if (sensorSetting->logPressed == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Button Clicks: "));
      if (sensorSetting->logClicked == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Toggle LED on each click (and log the LED state): "));
      if (sensorSetting->toggleLEDOnClick == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrintf2("5) LED Brightness: %d\r\n", sensorSetting->ledBrightness);
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logPressed ^= 1;
      else if (incoming == 3)
        sensorSetting->logClicked ^= 1;
      else if (incoming == 4)
        sensorSetting->toggleLEDOnClick ^= 1;
      else if (incoming == 5)
      {
        SerialPrint(F("Enter the LED brightness (0 to 255): "));
        int bright = getNumber(menuTimeout); //x second timeout
        if (bright < 0 || bright > 255)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->ledBrightness = bright;
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_BIO_SENSOR_HUB(void *configPtr)
{
  struct_BIO_SENSOR_HUB *sensorSetting = (struct_BIO_SENSOR_HUB*)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure Bio Sensor Hub (Pulse Oximeter)"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Heart Rate: "));
      if (sensorSetting->logHeartrate == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Confidence %: "));
      if (sensorSetting->logConfidence == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Oxygen %: "));
      if (sensorSetting->logOxygen == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log Status: "));
      if (sensorSetting->logStatus == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("6) Log Extended Status: "));
      if (sensorSetting->logExtendedStatus == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("7) Log Oxygen R Value: "));
      if (sensorSetting->logRValue == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logHeartrate ^= 1;
      else if (incoming == 3)
        sensorSetting->logConfidence ^= 1;
      else if (incoming == 4)
        sensorSetting->logOxygen ^= 1;
      else if (incoming == 5)
        sensorSetting->logStatus ^= 1;
      else if (incoming == 6)
        sensorSetting->logExtendedStatus ^= 1;
      else if (incoming == 7)
        sensorSetting->logRValue ^= 1;
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_ISM330DHCX(void *configPtr)
{
  struct_ISM330DHCX *sensorSetting = (struct_ISM330DHCX *)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure ISM330DHCX IMU"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Accelerometer: "));
      if (sensorSetting->logAccel == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Gyro: "));
      if (sensorSetting->logGyro == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log Data Ready: "));
      if (sensorSetting->logDataReady == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrintf2("5) Accel Scale: %d\r\n", sensorSetting->accelScale);
      SerialPrintf2("6) Accel Rate: %d\r\n", sensorSetting->accelRate);
      SerialPrint(F("7) Accel Filter LP2: "));
      if (sensorSetting->accelFilterLP2 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
      SerialPrintf2("8) Accel Slope Filter: %d\r\n", sensorSetting->accelSlopeFilter);
      SerialPrintf2("9) Gyro Scale: %d\r\n", sensorSetting->gyroScale);
      SerialPrintf2("10) Gyro Rate: %d\r\n", sensorSetting->gyroRate);
      SerialPrint(F("11) Gyro Filter LP1: "));
      if (sensorSetting->gyroFilterLP1 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
      SerialPrintf2("12) Gyro LP1 Bandwidth: %d\r\n", sensorSetting->gyroLP1BW);
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logAccel ^= 1;
      else if (incoming == 3)
        sensorSetting->logGyro ^= 1;
      else if (incoming == 4)
        sensorSetting->logDataReady ^= 1;
      else if (incoming == 5)
      {
        SerialPrintln(F("2g : 0"));
        SerialPrintln(F("16g: 1"));
        SerialPrintln(F("4g : 2"));
        SerialPrintln(F("8g : 3"));
        SerialPrint(F("Enter the Accel Full Scale (0 to 3): "));
        int newNum = getNumber(menuTimeout); //x second timeout
        if (newNum < 0 || newNum > 3)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->accelScale = newNum;
      }
      else if (incoming == 6)
      {
        SerialPrintln(F("OFF   : 0"));
        SerialPrintln(F("12.5Hz: 1"));
        SerialPrintln(F("26Hz  : 2"));
        SerialPrintln(F("52Hz  : 3"));
        SerialPrintln(F("104Hz : 4"));
        SerialPrintln(F("208Hz : 5"));
        SerialPrintln(F("416Hz : 6"));
        SerialPrintln(F("833Hz : 7"));
        SerialPrintln(F("1666Hz: 8"));
        SerialPrintln(F("3332Hz: 9"));
        SerialPrintln(F("6667Hz: 10"));
        SerialPrintln(F("1Hz6  : 11"));
        SerialPrint(F("Enter the Accel Rate (0 to 11): "));
        int newNum = getNumber(menuTimeout); //x second timeout
        if (newNum < 0 || newNum > 11)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->accelRate = newNum;
      }
      else if (incoming == 7)
        sensorSetting->accelFilterLP2 ^= 1;
      else if (incoming == 8)
      {
        SerialPrintln(F("HP_PATH_DISABLE_ON_OUT: 0"));
        SerialPrintln(F("LP_ODR_DIV_10         : 1"));
        SerialPrintln(F("LP_ODR_DIV_20         : 2"));
        SerialPrintln(F("LP_ODR_DIV_45         : 3"));
        SerialPrintln(F("LP_ODR_DIV_100        : 4"));
        SerialPrintln(F("LP_ODR_DIV_200        : 5"));
        SerialPrintln(F("LP_ODR_DIV_400        : 6"));
        SerialPrintln(F("LP_ODR_DIV_800        : 7"));
        SerialPrintln(F("SLOPE_ODR_DIV_4       : 16"));
        SerialPrintln(F("HP_ODR_DIV_10         : 17"));
        SerialPrintln(F("HP_ODR_DIV_20         : 18"));
        SerialPrintln(F("HP_ODR_DIV_45         : 19"));
        SerialPrintln(F("HP_ODR_DIV_100        : 20"));
        SerialPrintln(F("HP_ODR_DIV_200        : 21"));
        SerialPrintln(F("HP_ODR_DIV_400        : 22"));
        SerialPrintln(F("HP_ODR_DIV_800        : 23"));
        SerialPrintln(F("HP_REF_MD_ODR_DIV_10  : 49"));
        SerialPrintln(F("HP_REF_MD_ODR_DIV_20  : 50"));
        SerialPrintln(F("HP_REF_MD_ODR_DIV_45  : 51"));
        SerialPrintln(F("HP_REF_MD_ODR_DIV_100 : 52"));
        SerialPrintln(F("HP_REF_MD_ODR_DIV_200 : 53"));
        SerialPrintln(F("HP_REF_MD_ODR_DIV_400 : 54"));
        SerialPrintln(F("HP_REF_MD_ODR_DIV_800 : 55"));
        SerialPrint(F("Enter the Accel Slope Filter setting (0 to 55): "));
        int newNum = getNumber(menuTimeout); //x second timeout
        if (newNum < 0 || newNum > 55)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->accelSlopeFilter = newNum;
      }
      else if (incoming == 9)
      {
        SerialPrintln(F("125dps : 2"));
        SerialPrintln(F("250dps : 0"));
        SerialPrintln(F("500dps : 4"));
        SerialPrintln(F("1000dps: 8"));
        SerialPrintln(F("2000dps: 12"));
        SerialPrintln(F("4000dps: 1"));
        SerialPrint(F("Enter the Gyro Full Scale (0 to 12): "));
        int newNum = getNumber(menuTimeout); //x second timeout
        if (newNum < 0 || newNum > 12)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->gyroScale = newNum;
      }
      else if (incoming == 10)
      {
        SerialPrintln(F("OFF   : 0"));
        SerialPrintln(F("12Hz  : 1"));
        SerialPrintln(F("26Hz  : 2"));
        SerialPrintln(F("52Hz  : 3"));
        SerialPrintln(F("104Hz : 4"));
        SerialPrintln(F("208Hz : 5"));
        SerialPrintln(F("416Hz : 6"));
        SerialPrintln(F("833Hz : 7"));
        SerialPrintln(F("1666Hz: 8"));
        SerialPrintln(F("3332Hz: 9"));
        SerialPrintln(F("6667Hz: 10"));
        SerialPrint(F("Enter the Gyro Rate (0 to 10): "));
        int newNum = getNumber(menuTimeout); //x second timeout
        if (newNum < 0 || newNum > 10)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->gyroRate = newNum;
      }
      else if (incoming == 11)
        sensorSetting->gyroFilterLP1 ^= 1;
      else if (incoming == 12)
      {
        SerialPrintln(F("ULTRA_LIGHT: 0"));
        SerialPrintln(F("VERY_LIGHT : 1"));
        SerialPrintln(F("LIGHT      : 2"));
        SerialPrintln(F("MEDIUM     : 3"));
        SerialPrintln(F("STRONG     : 4"));
        SerialPrintln(F("VERY_STRONG: 5"));
        SerialPrintln(F("AGGRESSIVE : 6"));
        SerialPrintln(F("XTREME     : 7"));
        SerialPrintln(F("Enter the Gyro LP1 Bandwidth (0 to 7): "));
        int newNum = getNumber(menuTimeout); //x second timeout
        if (newNum < 0 || newNum > 7)
          SerialPrintln(F("Error: Out of range"));
        else
          sensorSetting->gyroLP1BW = newNum;
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_MMC5983MA(void *configPtr)
{
  struct_MMC5983MA *sensorSetting = (struct_MMC5983MA *)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure MMC5983MA Magnetometer"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log Magnetometer: "));
      if (sensorSetting->logMag == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log Temperature: "));
      if (sensorSetting->logTemperature == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));
    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logMag ^= 1;
      else if (incoming == 3)
        sensorSetting->logTemperature ^= 1;
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_KX134(void *configPtr)
{
  struct_KX134 *sensorSetting = (struct_KX134 *)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure KX134 Accelerometer"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Range 8G: "));
      if (sensorSetting->range8G == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Range 16G: "));
      if (sensorSetting->range16G == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Range 32G: "));
      if (sensorSetting->range32G == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Range 64G: "));
      if (sensorSetting->range64G == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("6) High Speed (400Hz): "));
      if (sensorSetting->highSpeed == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
      {
        sensorSetting->range8G = true;
        sensorSetting->range16G = false;
        sensorSetting->range32G = false;
        sensorSetting->range64G = false;
      }
      else if (incoming == 3)
      {
        sensorSetting->range8G = false;
        sensorSetting->range16G = true;
        sensorSetting->range32G = false;
        sensorSetting->range64G = false;
      }
      else if (incoming == 3)
      {
        sensorSetting->range8G = false;
        sensorSetting->range16G = false;
        sensorSetting->range32G = true;
        sensorSetting->range64G = false;
      }
      else if (incoming == 5)
      {
        sensorSetting->range8G = false;
        sensorSetting->range16G = false;
        sensorSetting->range32G = false;
        sensorSetting->range64G = true;
      }
      else if (incoming == 6)
        sensorSetting->highSpeed ^= 1;
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_ADS1015(void *configPtr)
{
  struct_ADS1015 *sensorSetting = (struct_ADS1015 *)configPtr;

  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure ADS1015 ADC"));

    SerialPrint(F("1) Sensor Logging: "));
    if (sensorSetting->log == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (sensorSetting->log == true)
    {
      SerialPrint(F("2) Log A0: "));
      if (sensorSetting->logA0 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("3) Log A1: "));
      if (sensorSetting->logA1 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("4) Log A2: "));
      if (sensorSetting->logA2 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("5) Log A3: "));
      if (sensorSetting->logA3 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("6) Log A0-A1: "));
      if (sensorSetting->logA0A1 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("7) Log A0-A3: "));
      if (sensorSetting->logA0A3 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("8) Log A1-A3: "));
      if (sensorSetting->logA1A3 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("9) Log A2-A3: "));
      if (sensorSetting->logA2A3 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("10) Gain x2/3: "));
      if (sensorSetting->gain23 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("11) Gain x1: "));
      if (sensorSetting->gain1 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("12) Gain x2: "));
      if (sensorSetting->gain2 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("13) Gain x4: "));
      if (sensorSetting->gain4 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("14) Gain x8: "));
      if (sensorSetting->gain8 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      SerialPrint(F("15) Gain x16: "));
      if (sensorSetting->gain16 == true) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

    }
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
      sensorSetting->log ^= 1;
    else if (sensorSetting->log == true)
    {
      if (incoming == 2)
        sensorSetting->logA0 ^= 1;
      else if (incoming == 3)
        sensorSetting->logA1 ^= 1;
      else if (incoming == 4)
        sensorSetting->logA2 ^= 1;
      else if (incoming == 5)
        sensorSetting->logA3 ^= 1;
      else if (incoming == 6)
        sensorSetting->logA0A1 ^= 1;
      else if (incoming == 7)
        sensorSetting->logA0A3 ^= 1;
      else if (incoming == 8)
        sensorSetting->logA1A3 ^= 1;
      else if (incoming == 9)
        sensorSetting->logA2A3 ^= 1;
      else if (incoming == 10)
      {
        sensorSetting->gain23 = true;
        sensorSetting->gain1 = false;
        sensorSetting->gain2 = false;
        sensorSetting->gain4 = false;
        sensorSetting->gain8 = false;
        sensorSetting->gain16 = false;
      }
      else if (incoming == 11)
      {
        sensorSetting->gain23 = false;
        sensorSetting->gain1 = true;
        sensorSetting->gain2 = false;
        sensorSetting->gain4 = false;
        sensorSetting->gain8 = false;
        sensorSetting->gain16 = false;
      }
      else if (incoming == 12)
      {
        sensorSetting->gain23 = false;
        sensorSetting->gain1 = false;
        sensorSetting->gain2 = true;
        sensorSetting->gain4 = false;
        sensorSetting->gain8 = false;
        sensorSetting->gain16 = false;
      }
      else if (incoming == 13)
      {
        sensorSetting->gain23 = false;
        sensorSetting->gain1 = false;
        sensorSetting->gain2 = false;
        sensorSetting->gain4 = true;
        sensorSetting->gain8 = false;
        sensorSetting->gain16 = false;
      }
      else if (incoming == 14)
      {
        sensorSetting->gain23 = false;
        sensorSetting->gain1 = false;
        sensorSetting->gain2 = false;
        sensorSetting->gain4 = false;
        sensorSetting->gain8 = true;
        sensorSetting->gain16 = false;
      }
      else if (incoming == 15)
      {
        sensorSetting->gain23 = false;
        sensorSetting->gain1 = false;
        sensorSetting->gain2 = false;
        sensorSetting->gain4 = false;
        sensorSetting->gain8 = false;
        sensorSetting->gain16 = true;
      }
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}
