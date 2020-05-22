
void menuAttachedDevices()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure Attached Devices");

    int availableDevices = 0;

    //Step through node list
    node *temp = new node;
    temp = head;

    if (temp == NULL)
      Serial.println("**No devices detected on Qwiic bus**");

    while (temp != NULL)
    {
      char strAddress[50];
      if (temp->muxAddress == 0)
        sprintf(strAddress, "(0x%02X)", temp->address);
      else
        sprintf(strAddress, "(Mux:0x%02X Port:%d)(0x%02X)", temp->muxAddress, temp->portNumber, temp->address);

      char strDeviceMenu[10];
      sprintf(strDeviceMenu, "%d)", availableDevices++ + 1);

      switch (temp->deviceType)
      {
        case DEVICE_VOC_CCS811:
          Serial.printf("%s CCS811 tVOC and CO2 Sensor %s\n", strDeviceMenu, strAddress);
          break;
        case DEVICE_DISTANCE_VL53L1X:
          Serial.printf("%s VL53L1X Distance Sensor %s\n", strDeviceMenu, strAddress);
          break;
        default:
          Serial.printf("Unknown device %d in menuAttachedDevices\n", temp->deviceType);
          break;
      }

      temp = temp->next;
    }

    //functionPointers[availableDevices] = menuConfigure_QwiicBus;
    Serial.printf("%d) Configure Qwiic Settings\n", availableDevices++ + 1);

    Serial.println("x) Exit");

    int nodeNumber = getNumber(menuTimeout); //Timeout after x seconds
    if (nodeNumber > 0 && nodeNumber <= availableDevices)
    {
      //Lookup the function we need to call based the node number
      FunctionPointer functionPointer = getConfigFunctionPtr(nodeNumber - 1);

      //Get the configPtr for this given node
      void *deviceConfigPtr = getConfigPointer(nodeNumber - 1);
      functionPointer(deviceConfigPtr); //Call the appropriate config menu with a pointer to this node's configPtr

      configureDevice(nodeNumber); //Reconfigure this device with the new settings
    }
    else if (nodeNumber == STATUS_PRESSED_X)
      break;
    else if (nodeNumber == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(nodeNumber);
  }
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
