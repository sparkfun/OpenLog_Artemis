/*
  To add a new sensor to the system:

  Add the library and the class constructor in OpenLog_Artemis
  Add a struct_MCP9600 to settings.h - This will define what settings for the sensor we will control
  Add a 'struct_CCS811 sensor_CCS811;' line to struct_settings{} in settings.h - This will put the sensor settings into NVM
  Add device to the struct_QwiicSensors, qwiicAvailable, and qwiicOnline structs in settings.h - This will let OpenLog know it's online and such
  Add device to menuAttachedDevices list
  Add the device's I2C address to the detectQwiicDevices function - Make sure the device is properly recognized with a whoami function (ideally)
  Create a menuConfigure_LPS25HB() function int menuAttachedDevices - This is the config menu. Add all the features you want the user to be able to control
  Add a startup function for this sensor in Sensors - This will notify the user at startup if sensor is detect and made online.
  Add a harvesting function in Sensors - Get the data from the device
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

void menuConfigure_QwiicBus()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Qwiic Bus");

    Serial.print("1) If sensor read time is greater than 2s, turn off bus power: ");
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.print("2) Set Max Qwiic Bus Speed: ");
    Serial.println(settings.qwiicBusMaxSpeed);

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
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  //qwiicOnline.LPS25HB = false; //Mark as offline so it will be started with new settings
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
/*

void menuConfigure_LPS25HB()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure LPS25HB Pressure Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_LPS25HB.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_LPS25HB.log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (settings.sensor_LPS25HB.logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Temperature: ");
      if (settings.sensor_LPS25HB.logTemp == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_LPS25HB.log ^= 1;
    else if (settings.sensor_LPS25HB.log == true)
    {
      if (incoming == '2')
        settings.sensor_LPS25HB.logPressure ^= 1;
      else if (incoming == '3')
        settings.sensor_LPS25HB.logTemp ^= 1;
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

  qwiicOnline.LPS25HB = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_NAU7802()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure NAU7802 Load Cell Amplifier");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_NAU7802.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_NAU7802.log == true)
    {
      Serial.println("2) Calibrate Scale");
      Serial.printf("\tScale calibration factor: %f\n", settings.sensor_NAU7802.calibrationFactor);
      Serial.printf("\tScale zero offset: %d\n", settings.sensor_NAU7802.zeroOffset);
      Serial.printf("\tWeight currently on scale: %f\n", loadcellSensor_NAU7802.getWeight());

      Serial.printf("3) Number of decimal places: %d\n", settings.sensor_NAU7802.decimalPlaces);
      Serial.printf("4) Average number of readings to take per weight read: %d\n", settings.sensor_NAU7802.averageAmount);
    }

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
    {
      settings.sensor_NAU7802.log ^= 1;
    }
    else if (settings.sensor_NAU7802.log == true)
    {
      if (incoming == '2')
      {
        //Gives user the ability to set a known weight on the scale and calculate a calibration factor
        Serial.println();
        Serial.println(F("Scale calibration"));

        Serial.println(F("Setup scale with no weight on it. Press a key when ready."));
        waitForInput();

        loadcellSensor_NAU7802.calculateZeroOffset(64); //Zero or Tare the scale. Average over 64 readings.
        Serial.print(F("New zero offset: "));
        Serial.println(loadcellSensor_NAU7802.getZeroOffset());

        Serial.println(F("Place known weight on scale. Press a key when weight is in place and stable."));
        waitForInput();

        Serial.print(F("Please enter the weight, without units, currently sitting on the scale (for example '4.25'): "));
        waitForInput();

        //Read user input
        float weightOnScale = Serial.parseFloat();
        loadcellSensor_NAU7802.calculateCalibrationFactor(weightOnScale, 64); //Tell the library how much weight is currently on it. Average over 64 readings.

        settings.sensor_NAU7802.calibrationFactor = loadcellSensor_NAU7802.getCalibrationFactor();
        settings.sensor_NAU7802.zeroOffset = loadcellSensor_NAU7802.getZeroOffset();

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
          settings.sensor_NAU7802.decimalPlaces = places;
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
          settings.sensor_NAU7802.averageAmount = amt;
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

  qwiicOnline.NAU7802 = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_uBlox()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure uBlox GPS Receiver");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_uBlox.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_uBlox.log == true)
    {
      Serial.print("2) Log GPS Date: ");
      if (settings.sensor_uBlox.logDate == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log GPS Time: ");
      if (settings.sensor_uBlox.logTime == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Longitude/Latitude: ");
      if (settings.sensor_uBlox.logPosition == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Log Altitude: ");
      if (settings.sensor_uBlox.logAltitude == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("6) Log Altitude Mean Sea Level: ");
      if (settings.sensor_uBlox.logAltitudeMSL == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("7) Log Satellites In View: ");
      if (settings.sensor_uBlox.logSIV == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("8) Log Fix Type: ");
      if (settings.sensor_uBlox.logFixType == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("9) Log Carrier Solution: ");
      if (settings.sensor_uBlox.logCarrierSolution == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("10) Log Ground Speed: ");
      if (settings.sensor_uBlox.logGroundSpeed == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("11) Log Heading of Motion: ");
      if (settings.sensor_uBlox.logHeadingOfMotion == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("12) Log Position Dilution of Precision (pDOP): ");
      if (settings.sensor_uBlox.logpDOP == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.flush();

      Serial.print("13) Log Interval Time Of Week (iTOW): ");
      if (settings.sensor_uBlox.logiTOW == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.printf("14) Set I2C Interface Speed: %d\n", settings.sensor_uBlox.i2cSpeed);
    }
    Serial.println("x) Exit");

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
    {
      settings.sensor_uBlox.log ^= 1;
    }
    else if (settings.sensor_NAU7802.log == true)
    {
      if (incoming == 2)
        settings.sensor_uBlox.logDate ^= 1;
      else if (incoming == 3)
        settings.sensor_uBlox.logTime ^= 1;
      else if (incoming == 4)
        settings.sensor_uBlox.logPosition ^= 1;
      else if (incoming == 5)
        settings.sensor_uBlox.logAltitude ^= 1;
      else if (incoming == 6)
        settings.sensor_uBlox.logAltitudeMSL ^= 1;
      else if (incoming == 7)
        settings.sensor_uBlox.logSIV ^= 1;
      else if (incoming == 8)
        settings.sensor_uBlox.logFixType ^= 1;
      else if (incoming == 9)
        settings.sensor_uBlox.logCarrierSolution ^= 1;
      else if (incoming == 10)
        settings.sensor_uBlox.logGroundSpeed ^= 1;
      else if (incoming == 11)
        settings.sensor_uBlox.logHeadingOfMotion ^= 1;
      else if (incoming == 12)
        settings.sensor_uBlox.logpDOP ^= 1;
      else if (incoming == 13)
        settings.sensor_uBlox.logiTOW ^= 1;
      else if (incoming == 14)
      {
        if (settings.sensor_uBlox.i2cSpeed == 100000)
          settings.sensor_uBlox.i2cSpeed = 400000;
        else
          settings.sensor_uBlox.i2cSpeed = 100000;
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

  qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_MCP9600()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure MCP9600 Thermocouple Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_MCP9600.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_MCP9600.log == true)
    {
      Serial.print("2) Log Thermocouple Temperature: ");
      if (settings.sensor_MCP9600.logTemp == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Ambient Temperature: ");
      if (settings.sensor_MCP9600.logAmbientTemp == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_MCP9600.log ^= 1;
    else if (settings.sensor_MCP9600.log == true)
    {
      if (incoming == '2')
        settings.sensor_MCP9600.logTemp ^= 1;
      else if (incoming == '3')
        settings.sensor_MCP9600.logAmbientTemp ^= 1;
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

  qwiicOnline.MCP9600 = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_VCNL4040()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure VCNL4040 Proximity Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_VCNL4040.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_VCNL4040.log == true)
    {
      Serial.print("2) Log Proximity: ");
      if (settings.sensor_VCNL4040.logProximity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Ambient Light: ");
      if (settings.sensor_VCNL4040.logAmbientLight == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.printf("4) Set LED Current: %d\n", settings.sensor_VCNL4040.LEDCurrent);
      Serial.printf("5) Set IR Duty Cycle: %d\n", settings.sensor_VCNL4040.IRDutyCycle);
      Serial.printf("6) Set Proximity Integration Time: %d\n", settings.sensor_VCNL4040.proximityIntegrationTime);
      Serial.printf("7) Set Ambient Integration Time: %d\n", settings.sensor_VCNL4040.ambientIntegrationTime);
      Serial.printf("8) Set Resolution (bits): %d\n", settings.sensor_VCNL4040.resolution);
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_VCNL4040.log ^= 1;
    else if (settings.sensor_VCNL4040.log == true)
    {
      if (incoming == '2')
        settings.sensor_VCNL4040.logProximity ^= 1;
      else if (incoming == '3')
        settings.sensor_VCNL4040.logAmbientLight ^= 1;
      else if (incoming == '4')
      {
        Serial.print("Enter current (mA) for IR LED drive (50 to 200mA): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 50 || amt > 200)
          Serial.println("Error: Out of range");
        else
          settings.sensor_VCNL4040.LEDCurrent = amt;
      }
      else if (incoming == '5')
      {
        Serial.print("Enter IR Duty Cycle (40 to 320): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 40 || amt > 320)
          Serial.println("Error: Out of range");
        else
          settings.sensor_VCNL4040.IRDutyCycle = amt;
      }
      else if (incoming == '6')
      {
        Serial.print("Enter Proximity Integration Time (1 to 8): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 1 || amt > 8)
          Serial.println("Error: Out of range");
        else
          settings.sensor_VCNL4040.proximityIntegrationTime = amt;
      }
      else if (incoming == '7')
      {
        Serial.print("Enter Ambient Light Integration Time (80 to 640ms): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 80 || amt > 640)
          Serial.println("Error: Out of range");
        else
          settings.sensor_VCNL4040.ambientIntegrationTime = amt;
      }
      else if (incoming == '8')
      {
        Serial.print("Enter Proximity Resolution (12 or 16 bit): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt == 12 || amt == 16)
          settings.sensor_VCNL4040.resolution = amt;
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

  qwiicOnline.VCNL4040 = false; //Mark as offline so it will be started with new settings
}



void menuConfigure_TMP117()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure TMP117 Precision Temperature Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_TMP117.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_TMP117.log ^= 1;
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  qwiicOnline.TMP117 = false; //Mark as offline so it will be started with new settings
}


void menuConfigure_SGP30()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure SGP30 tVOC and CO2 Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_SGP30.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_SGP30.log == true)
    {
      Serial.print("2) Log tVOC: ");
      if (settings.sensor_SGP30.logTVOC == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log CO2: ");
      if (settings.sensor_SGP30.logCO2 == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_SGP30.log ^= 1;
    else if (settings.sensor_SGP30.log == true)
    {
      if (incoming == '2')
        settings.sensor_SGP30.logTVOC ^= 1;
      else if (incoming == '3')
        settings.sensor_SGP30.logCO2 ^= 1;
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

  qwiicOnline.SGP30 = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_VEML6075()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure VEML6075 UV Index Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_VEML6075.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_VEML6075.log == true)
    {
      Serial.print("2) Log UVA: ");
      if (settings.sensor_VEML6075.logUVA == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log UVB: ");
      if (settings.sensor_VEML6075.logUVB == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log UV Index: ");
      if (settings.sensor_VEML6075.logUVIndex == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_VEML6075.log ^= 1;
    else if (settings.sensor_VEML6075.log == true)
    {
      if (incoming == '2')
        settings.sensor_VEML6075.logUVA ^= 1;
      else if (incoming == '3')
        settings.sensor_VEML6075.logUVB ^= 1;
      else if (incoming == '3')
        settings.sensor_VEML6075.logUVIndex ^= 1;
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

  qwiicOnline.VEML6075 = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_MS5637()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure MS5637 Pressure Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_MS5637.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_MS5637.log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (settings.sensor_MS5637.logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Temperature: ");
      if (settings.sensor_MS5637.logTemp == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_MS5637.log ^= 1;
    else if (settings.sensor_MS5637.log == true)
    {
      if (incoming == '2')
        settings.sensor_MS5637.logPressure ^= 1;
      else if (incoming == '3')
        settings.sensor_MS5637.logTemp ^= 1;
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

  qwiicOnline.MS5637 = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_SCD30()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure SCD30 CO2 and Humidity Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_SCD30.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_SCD30.log == true)
    {
      Serial.print("2) Log CO2: ");
      if (settings.sensor_SCD30.logCO2 == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Humidity: ");
      if (settings.sensor_SCD30.logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Temperature: ");
      if (settings.sensor_SCD30.logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.printf("5) Set Measurement Interval: %d\n", settings.sensor_SCD30.measurementInterval);
      Serial.printf("6) Set Altitude Compensation: %d\n", settings.sensor_SCD30.altitudeCompensation);
      Serial.printf("7) Set Ambient Pressure: %d\n", settings.sensor_SCD30.ambientPressure);
      Serial.printf("8) Set Temperature Offset: %d\n", settings.sensor_SCD30.temperatureOffset);
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_SCD30.log ^= 1;
    else if (settings.sensor_SCD30.log == true)
    {
      if (incoming == '2')
        settings.sensor_SCD30.logCO2 ^= 1;
      else if (incoming == '3')
        settings.sensor_SCD30.logHumidity ^= 1;
      else if (incoming == '4')
        settings.sensor_SCD30.logTemperature ^= 1;
      else if (incoming == '5')
      {
        Serial.print("Enter the seconds between measurements (2 to 1800): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 2 || amt > 1800)
          Serial.println("Error: Out of range");
        else
          settings.sensor_SCD30.measurementInterval = amt;
      }
      else if (incoming == '6')
      {
        Serial.print("Enter the Altitude Compensation in meters (0 to 10000): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 0 || amt > 10000)
          Serial.println("Error: Out of range");
        else
          settings.sensor_SCD30.altitudeCompensation = amt;
      }
      else if (incoming == '7')
      {
        Serial.print("Enter Ambient Pressure in mBar (700 to 1200): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < 700 || amt > 1200)
          Serial.println("Error: Out of range");
        else
          settings.sensor_SCD30.ambientPressure = amt;
      }
      else if (incoming == '8')
      {
        Serial.print("The current temperature offset read from the sensor is: ");
        Serial.print(co2Sensor_SCD30.getTemperatureOffset(), 2);
        Serial.println("C");
        Serial.print("Enter new temperature offset in C (-50 to 50): ");
        int amt = getNumber(menuTimeout); //x second timeout
        if (amt < -50 || amt > 50)
          settings.sensor_SCD30.temperatureOffset = amt;
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

  qwiicOnline.SCD30 = false; //Mark as offline so it will be started with new settings
}

void menuConfigure_MS8607()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure MS8607 Pressure Humidity Temperature (PHT) Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_MS8607.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_MS8607.log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (settings.sensor_MS8607.logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Humidity: ");
      if (settings.sensor_MS8607.logHumidity == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Temperature: ");
      if (settings.sensor_MS8607.logTemperature == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Heater: ");
      if (settings.sensor_MS8607.enableHeater == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("6) Set Pressure Resolution: ");
      if (settings.sensor_MS8607.pressureResolution == MS8607_pressure_resolution_osr_256)
        Serial.print("0.11");
      else if (settings.sensor_MS8607.pressureResolution == MS8607_pressure_resolution_osr_512)
        Serial.print("0.062");
      else if (settings.sensor_MS8607.pressureResolution == MS8607_pressure_resolution_osr_1024)
        Serial.print("0.039");
      else if (settings.sensor_MS8607.pressureResolution == MS8607_pressure_resolution_osr_2048)
        Serial.print("0.028");
      else if (settings.sensor_MS8607.pressureResolution == MS8607_pressure_resolution_osr_4096)
        Serial.print("0.021");
      else if (settings.sensor_MS8607.pressureResolution == MS8607_pressure_resolution_osr_8192)
        Serial.print("0.016");
      Serial.println(" mbar");

      Serial.print("7) Set Humidity Resolution: ");
      if (settings.sensor_MS8607.humidityResolution == MS8607_humidity_resolution_8b)
        Serial.print("8");
      else if (settings.sensor_MS8607.humidityResolution == MS8607_humidity_resolution_10b)
        Serial.print("10");
      else if (settings.sensor_MS8607.humidityResolution == MS8607_humidity_resolution_11b)
        Serial.print("11");
      else if (settings.sensor_MS8607.humidityResolution == MS8607_humidity_resolution_12b)
        Serial.print("12");
      Serial.println(" bits");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.sensor_MS8607.log ^= 1;
    else if (settings.sensor_MS8607.log == true)
    {
      if (incoming == '2')
        settings.sensor_MS8607.logPressure ^= 1;
      else if (incoming == '3')
        settings.sensor_MS8607.logHumidity ^= 1;
      else if (incoming == '4')
        settings.sensor_MS8607.logTemperature ^= 1;
      else if (incoming == '5')
        settings.sensor_MS8607.enableHeater ^= 1;
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
          settings.sensor_MS8607.pressureResolution = (MS8607_pressure_resolution)(amt - 1);
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
          if (amt == 1) settings.sensor_MS8607.humidityResolution = MS8607_humidity_resolution_8b;
          if (amt == 2) settings.sensor_MS8607.humidityResolution = MS8607_humidity_resolution_10b;
          if (amt == 3) settings.sensor_MS8607.humidityResolution = MS8607_humidity_resolution_11b;
          if (amt == 4) settings.sensor_MS8607.humidityResolution = MS8607_humidity_resolution_12b;
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

  qwiicOnline.MS8607 = false; //Mark as offline so it will be started with new settings
}
*/
