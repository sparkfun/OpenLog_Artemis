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
  bool somethingDetected = false;

  qwiic.setClock(100000); //During detection, go slow

  qwiic.setPullups(1); //Set pullups to 1k. If we don't have pullups, detectQwiicDevices() takes ~900ms to complete. We'll disable pullups if something is detected.

  //24k causes a bunch of unknown devices to be falsely detected.
  //qwiic.setPullups(24); //Set pullups to 24k. If we don't have pullups, detectQwiicDevices() takes ~900ms to complete. We'll disable pullups if something is detected.

  //Do a prelim scan to see if anything is out there
  for (uint8_t address = 1 ; address < 127 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      somethingDetected = true;
      break;
    }
  }
  if (somethingDetected == false) return (false);

  Serial.println("Identifying Qwiic Devices...");

  //Depending on what hardware is configured, the Qwiic bus may have only been turned on a few ms ago
  //Give sensors, specifically those with a low I2C address, time to turn on
  delay(100); //SCD30 required >50ms to turn on

  //First scan for Muxes. Valid addresses are 0x70 to 0x77.
  //If any are found, they will be begin()'d causing their ports to turn off
  Serial.println("Scanning for multiplexers");
  uint8_t muxCount = 0;
  for (uint8_t address = 0x70 ; address < 0x78 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      deviceType_e foundType = testDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType == DEVICE_MULTIPLEXER)
      {
        addDevice(foundType, address, 0, 0); //Add this device to our map
        muxCount++;
      }
    }
  }

  if(muxCount > 0) beginQwiicDevices(); //Because we are about to use a multiplexer, begin() the muxes.

  //Before going into sub branches, complete the scan of the main branch for all devices
  Serial.println("Scanning main bus");
  for (uint8_t address = 1 ; address < 127 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      deviceType_e foundType = testDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType != DEVICE_UNKNOWN_DEVICE)
      {
        if (addDevice(foundType, address, 0, 0) == true) //Records this device. //Returns false if device was already recorded.
        {
          //Serial.printf("-Added %s at address 0x%02X\n", getDeviceName(foundType), address);
        }
      }
    }
  }

  //If we have muxes, begin scanning their sub nets
  if (muxCount > 0)
  {
    Serial.println("Multiplexers found. Scanning sub nets...");

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
              {
                //Serial.printf("-Added %s at address 0x%02X.0x%02X.%d\n", getDeviceName(foundType), address, muxNode->address, portNumber);
              }
            }
          } //End I2c check
        } //End I2C scanning
      } //End mux port stepping
    } //End mux stepping
  } //End mux > 0

  bubbleSortDevices(head); //This may destroy mux alignment to node 0.
  
  qwiic.setPullups(0); //We've detected something on the bus so disable pullups

  setMaxI2CSpeed(); //Try for 400kHz but reduce to 100kHz or low if certain devices are attached

  Serial.println("Autodetect complete");

  return (true);
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
            //Serial.printf("%s Multiplexer %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_LOADCELL_NAU7802:
            Serial.printf("%s NAU7802 Weight Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_DISTANCE_VL53L1X:
            Serial.printf("%s VL53L1X Distance Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_GPS_UBLOX:
            Serial.printf("%s u-blox GPS Receiver %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PROXIMITY_VCNL4040:
            Serial.printf("%s VCNL4040 Proximity Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_TEMPERATURE_TMP117:
            Serial.printf("%s TMP117 High Precision Temperature Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_MS5637:
            Serial.printf("%s MS5637 Pressure Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PRESSURE_LPS25HB:
            Serial.printf("%s LPS25HB Pressure Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PHT_BME280:
            Serial.printf("%s BME280 Pressure/Humidity/Temp (PHT) Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_UV_VEML6075:
            Serial.printf("%s VEML6075 UV Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_VOC_CCS811:
            Serial.printf("%s CCS811 tVOC and CO2 Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_VOC_SGP30:
            Serial.printf("%s SGP30 tVOC and CO2 Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_CO2_SCD30:
            Serial.printf("%s SCD30 CO2 Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_PHT_MS8607:
            Serial.printf("%s MS8607 Pressure/Humidity/Temp (PHT) Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_TEMPERATURE_MCP9600:
            Serial.printf("%s MCP9600 Thermocouple Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_HUMIDITY_AHT20:
            Serial.printf("%s AHT20 Humidity Sensor %s\n", strDeviceMenu, strAddress);
            break;
          case DEVICE_HUMIDITY_SHTC3:
            Serial.printf("%s SHTC3 Humidity Sensor %s\n", strDeviceMenu, strAddress);
            break;
          default:
            Serial.printf("Unknown device type %d in menuAttachedDevices\n", temp->deviceType);
            break;
        }
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

      configureDevice(nodeNumber - 1); //Reconfigure this device with the new settings
    }
    else if (nodeNumber == availableDevices)
    {
      menuConfigure_QwiicBus();
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
    Serial.println();
    Serial.println("Menu: Configure Qwiic Bus");

    Serial.print("1) If sensor read time is greater than 2s, turn off bus power: ");
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.printf("2) Set Max Qwiic Bus Speed: %d Hz\n", settings.qwiicBusMaxSpeed);

    Serial.printf("3) Set Qwiic bus power up delay: %d ms\n", settings.qwiicBusPowerUpDelayMs);

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.powerDownQwiicBusBetweenReads ^= 1;
    else if (incoming == '2')
    {
      Serial.print("Enter max frequency to run Qwiic bus: (100000 to 400000): ");
      int amt = getNumber(menuTimeout);
      if (amt >= 100000 && amt <= 400000)
        settings.qwiicBusMaxSpeed = amt;
      else
        Serial.println("Error: Out of range");
    }
    else if (incoming == '3')
    {
      Serial.print("Enter number of milliseconds to wait for Qwiic VCC to stabilize before communication: (1 to 1000): ");
      int amt = getNumber(menuTimeout);
      if (amt >= 1 && amt <= 1000)
        settings.qwiicBusPowerUpDelayMs = amt;
      else
        Serial.println("Error: Out of range");
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
  struct_VL53L1X *sensorSetting = (struct_VL53L1X*)configPtr;

  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure VL53L1X Distance Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Distance: ");
      if (sensorSetting->logDistance == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Range Status: ");
      if (sensorSetting->logRangeStatus == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Signal Rate: ");
      if (sensorSetting->logSignalRate == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Set Distance Mode: ");
      if (sensorSetting->distanceMode == VL53L1X_DISTANCE_MODE_SHORT)
        Serial.print("Short");
      else
        Serial.print("Long");
      Serial.println();

      Serial.printf("6) Set Intermeasurement Period: %d ms\n", sensorSetting->intermeasurementPeriod);
      Serial.printf("7) Set Offset: %d mm\n", sensorSetting->offset);
      Serial.printf("8) Set Cross Talk (counts per second): %d cps\n", sensorSetting->crosstalk);
    }
    Serial.println("x) Exit");

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
          Serial.println("Intermeasurement Period increased to 140ms");
        }
      }
      else if (incoming == '6')
      {
        int min = 20;
        if (sensorSetting->distanceMode == VL53L1X_DISTANCE_MODE_LONG)
          min = 140;


        Serial.printf("Set timing budget (%d to 1000ms): ", min);
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < min || amt > 1000)
          Serial.println("Error: Out of range");
        else
          sensorSetting->intermeasurementPeriod = amt;
      }
      else if (incoming == '7')
      {
        Serial.print("Set Offset in mm (0 to 4000mm): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 4000)
          Serial.println("Error: Out of range");
        else
          sensorSetting->offset = amt;
      }
      else if (incoming == '8')
      {
        Serial.print("Set Crosstalk in Counts Per Second: ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 4000)
          Serial.println("Error: Out of range");
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
    Serial.println();
    Serial.println("Menu: Configure BME280 Pressure/Humidity/Temperature Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (sensorSetting->logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Humidity: ");
      if (sensorSetting->logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Altitude: ");
      if (sensorSetting->logAltitude == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Log Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
    Serial.println();
    Serial.println("Menu: Configure CCS811 tVOC and CO2 Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log tVOC: ");
      if (sensorSetting->logTVOC == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log CO2: ");
      if (sensorSetting->logCO2 == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
    Serial.println();
    Serial.println("Menu: Configure LPS25HB Pressure Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (sensorSetting->logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
    Serial.println("NAU7802 node not found. Returning.");
    delay(1000);
    return;
  }

  NAU7802 *sensor = (NAU7802 *)temp->classPtr;
  struct_NAU7802 *sensorConfig = (struct_NAU7802*)configPtr;

  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure NAU7802 Load Cell Amplifier");

    Serial.print("1) Sensor Logging: ");
    if (sensorConfig->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorConfig->log == true)
    {
      Serial.println("2) Calibrate Scale");
      Serial.printf("\tScale calibration factor: %f\n", sensorConfig->calibrationFactor);
      Serial.printf("\tScale zero offset: %d\n", sensorConfig->zeroOffset);
      Serial.printf("\tWeight currently on scale: %f\n", sensor->getWeight());

      Serial.printf("3) Number of decimal places: %d\n", sensorConfig->decimalPlaces);
      Serial.printf("4) Average number of readings to take per weight read: %d\n", sensorConfig->averageAmount);
    }

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      sensorConfig->log ^= 1;
    }
    else if (sensorConfig->log == true)
    {
      if (incoming == '2')
      {
        //Gives user the ability to set a known weight on the scale and calculate a calibration factor
        Serial.println();
        Serial.println(F("Scale calibration"));

        Serial.println(F("Setup scale with no weight on it. Press a key when ready."));
        waitForInput();

        sensor->calculateZeroOffset(64); //Zero or Tare the scale. Average over 64 readings.
        Serial.print(F("New zero offset: "));
        Serial.println(sensor->getZeroOffset());

        Serial.println(F("Place known weight on scale. Press a key when weight is in place and stable."));
        waitForInput();

        Serial.print(F("Please enter the weight, without units, currently sitting on the scale (for example '4.25'): "));
        waitForInput();

        //Read user input
        float weightOnScale = Serial.parseFloat();
        sensor->calculateCalibrationFactor(weightOnScale, 64); //Tell the library how much weight is currently on it. Average over 64 readings.

        sensorConfig->calibrationFactor = sensor->getCalibrationFactor();
        sensorConfig->zeroOffset = sensor->getZeroOffset();

        Serial.println();
      }
      else if (incoming == '3')
      {
        Serial.print("Enter number of decimal places to print (1 to 10): ");
        int places = getNumber(menuTimeout);
        if (places < 1 || places > 10)
        {
          Serial.println("Error: Decimal places out of range");
        }
        else
        {
          sensorConfig->decimalPlaces = places;
        }
      }
      else if (incoming == '4')
      {
        Serial.print("Enter number of readings to take per weight read (1 to 10): ");
        int amt = getNumber(menuTimeout);
        if (amt < 1 || amt > 10)
        {
          Serial.println("Error: Average number of readings out of range");
        }
        else
        {
          sensorConfig->averageAmount = amt;
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

void menuConfigure_uBlox(void *configPtr)
{
  struct_uBlox *sensorSetting = (struct_uBlox*)configPtr;

  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure uBlox GPS Receiver");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log GPS Date: ");
      if (sensorSetting->logDate == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log GPS Time: ");
      if (sensorSetting->logTime == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Longitude/Latitude: ");
      if (sensorSetting->logPosition == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Log Altitude: ");
      if (sensorSetting->logAltitude == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("6) Log Altitude Mean Sea Level: ");
      if (sensorSetting->logAltitudeMSL == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("7) Log Satellites In View: ");
      if (sensorSetting->logSIV == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("8) Log Fix Type: ");
      if (sensorSetting->logFixType == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("9) Log Carrier Solution: ");
      if (sensorSetting->logCarrierSolution == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("10) Log Ground Speed: ");
      if (sensorSetting->logGroundSpeed == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("11) Log Heading of Motion: ");
      if (sensorSetting->logHeadingOfMotion == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("12) Log Position Dilution of Precision (pDOP): ");
      if (sensorSetting->logpDOP == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.flush();

      Serial.print("13) Log Interval Time Of Week (iTOW): ");
      if (sensorSetting->logiTOW == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.printf("14) Set I2C Interface Speed (u-blox modules have pullups built in. Remove *all* I2C pullups to achieve 400kHz): %d\n", sensorSetting->i2cSpeed);
    }
    Serial.println("x) Exit");

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
      else if (incoming == STATUS_PRESSED_X)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else
      printUnknown(incoming);
  }

}

void menuConfigure_MCP9600(void *configPtr)
{
  struct_MCP9600 *sensorSetting = (struct_MCP9600*)configPtr;
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure MCP9600 Thermocouple Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Thermocouple Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Ambient Temperature: ");
      if (sensorSetting->logAmbientTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
    Serial.println();
    Serial.println("Menu: Configure VCNL4040 Proximity Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Proximity: ");
      if (sensorSetting->logProximity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Ambient Light: ");
      if (sensorSetting->logAmbientLight == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.printf("4) Set LED Current: %d\n", sensorSetting->LEDCurrent);
      Serial.printf("5) Set IR Duty Cycle: %d\n", sensorSetting->IRDutyCycle);
      Serial.printf("6) Set Proximity Integration Time: %d\n", sensorSetting->proximityIntegrationTime);
      Serial.printf("7) Set Ambient Integration Time: %d\n", sensorSetting->ambientIntegrationTime);
      Serial.printf("8) Set Resolution (bits): %d\n", sensorSetting->resolution);
    }
    Serial.println("x) Exit");

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
        Serial.print("Enter current (mA) for IR LED drive (50 to 200mA): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 50 || amt > 200)
          Serial.println("Error: Out of range");
        else
          sensorSetting->LEDCurrent = amt;
      }
      else if (incoming == '5')
      {
        Serial.print("Enter IR Duty Cycle (40 to 320): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 40 || amt > 320)
          Serial.println("Error: Out of range");
        else
          sensorSetting->IRDutyCycle = amt;
      }
      else if (incoming == '6')
      {
        Serial.print("Enter Proximity Integration Time (1 to 8): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 1 || amt > 8)
          Serial.println("Error: Out of range");
        else
          sensorSetting->proximityIntegrationTime = amt;
      }
      else if (incoming == '7')
      {
        Serial.print("Enter Ambient Light Integration Time (80 to 640ms): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 80 || amt > 640)
          Serial.println("Error: Out of range");
        else
          sensorSetting->ambientIntegrationTime = amt;
      }
      else if (incoming == '8')
      {
        Serial.print("Enter Proximity Resolution (12 or 16 bit): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt == 12 || amt == 16)
          sensorSetting->resolution = amt;
        else
          Serial.println("Error: Out of range");
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
    Serial.println();
    Serial.println("Menu: Configure TMP117 Precision Temperature Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.println("x) Exit");

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
    Serial.println();
    Serial.println("Menu: Configure SGP30 tVOC and CO2 Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log tVOC: ");
      if (sensorSetting->logTVOC == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log CO2: ");
      if (sensorSetting->logCO2 == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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

void menuConfigure_VEML6075(void *configPtr)
{
  struct_VEML6075 *sensorSetting = (struct_VEML6075*)configPtr;
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure VEML6075 UV Index Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log UVA: ");
      if (sensorSetting->logUVA == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log UVB: ");
      if (sensorSetting->logUVB == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log UV Index: ");
      if (sensorSetting->logUVIndex == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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


void menuConfigure_MS5637(void *configPtr)
{
  struct_MS5637 *sensorSetting = (struct_MS5637*)configPtr;
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure MS5637 Pressure Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (sensorSetting->logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
    Serial.println("SCD30 node not found. Returning.");
    delay(1000);
    return;
  }

  SCD30 *sensor = (SCD30 *)temp->classPtr;
  struct_SCD30 *sensorSetting = (struct_SCD30*)configPtr;

  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure SCD30 CO2 and Humidity Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log CO2: ");
      if (sensorSetting->logCO2 == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Humidity: ");
      if (sensorSetting->logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.printf("5) Set Measurement Interval: %d\n", sensorSetting->measurementInterval);
      Serial.printf("6) Set Altitude Compensation: %d\n", sensorSetting->altitudeCompensation);
      Serial.printf("7) Set Ambient Pressure: %d\n", sensorSetting->ambientPressure);
      Serial.printf("8) Set Temperature Offset: %d\n", sensorSetting->temperatureOffset);
    }
    Serial.println("x) Exit");

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
        Serial.print("Enter the seconds between measurements (2 to 1800): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 2 || amt > 1800)
          Serial.println("Error: Out of range");
        else
          sensorSetting->measurementInterval = amt;
      }
      else if (incoming == '6')
      {
        Serial.print("Enter the Altitude Compensation in meters (0 to 10000): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 10000)
          Serial.println("Error: Out of range");
        else
          sensorSetting->altitudeCompensation = amt;
      }
      else if (incoming == '7')
      {
        Serial.print("Enter Ambient Pressure in mBar (700 to 1200): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 700 || amt > 1200)
          Serial.println("Error: Out of range");
        else
          sensorSetting->ambientPressure = amt;
      }
      else if (incoming == '8')
      {
        Serial.print("The current temperature offset read from the sensor is: ");
        Serial.print(sensor->getTemperatureOffset(), 2);
        Serial.println("C");
        Serial.print("Enter new temperature offset in C (-50 to 50): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < -50 || amt > 50)
          sensorSetting->temperatureOffset = amt;
        else
          Serial.println("Error: Out of range");
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
    Serial.println();
    Serial.println("Menu: Configure MS8607 Pressure Humidity Temperature (PHT) Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (sensorSetting->logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Humidity: ");
      if (sensorSetting->logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Heater: ");
      if (sensorSetting->enableHeater == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("6) Set Pressure Resolution: ");
      if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_256)
        Serial.print("0.11");
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_512)
        Serial.print("0.062");
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_1024)
        Serial.print("0.039");
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_2048)
        Serial.print("0.028");
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_4096)
        Serial.print("0.021");
      else if (sensorSetting->pressureResolution == MS8607_pressure_resolution_osr_8192)
        Serial.print("0.016");
      Serial.println(" mbar");

      Serial.print("7) Set Humidity Resolution: ");
      if (sensorSetting->humidityResolution == MS8607_humidity_resolution_8b)
        Serial.print("8");
      else if (sensorSetting->humidityResolution == MS8607_humidity_resolution_10b)
        Serial.print("10");
      else if (sensorSetting->humidityResolution == MS8607_humidity_resolution_11b)
        Serial.print("11");
      else if (sensorSetting->humidityResolution == MS8607_humidity_resolution_12b)
        Serial.print("12");
      Serial.println(" bits");
    }
    Serial.println("x) Exit");

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
        Serial.println("Set Pressure Resolution:");
        Serial.println("1) 0.11 mbar");
        Serial.println("2) 0.062 mbar");
        Serial.println("3) 0.039 mbar");
        Serial.println("4) 0.028 mbar");
        Serial.println("5) 0.021 mbar");
        Serial.println("6) 0.016 mbar");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt >= 1 && amt <= 6)
          sensorSetting->pressureResolution = (MS8607_pressure_resolution)(amt - 1);
        else
          Serial.println("Error: Out of range");
      }
      else if (incoming == '7')
      {
        Serial.println("Set Humidity Resolution:");
        Serial.println("1) 8 bit");
        Serial.println("2) 10 bit");
        Serial.println("3) 11 bit");
        Serial.println("4) 12 bit");
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
          Serial.println("Error: Out of range");
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
    Serial.println();
    Serial.println("Menu: Configure AHT20 Humidity Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Humidity: ");
      if (sensorSetting->logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
    Serial.println();
    Serial.println("Menu: Configure SHTC3 Humidity Sensor");

    Serial.print("1) Sensor Logging: ");
    if (sensorSetting->log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (sensorSetting->log == true)
    {
      Serial.print("2) Log Humidity: ");
      if (sensorSetting->logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Temperature: ");
      if (sensorSetting->logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

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
