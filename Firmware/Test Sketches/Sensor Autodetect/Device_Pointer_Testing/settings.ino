//Export the current settings to a config file
void recordSettingsToFile()
{
  if (true == true)
  {
    if (sd.exists("OLA_settings.cfg"))
      sd.remove("OLA_settings.cfg");

#ifdef USE_EXFAT
    FsFile settingsFile; //exFat
#else
    File settingsFile; //FAT16/32
#endif
    if (settingsFile.open("OLA_settings.cfg", O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println("Failed to create settings file");
      return;
    }

    //    settingsFile.println("sizeOfSettings=" + (String)settings.sizeOfSettings);
    //
    //    settingsFile.println("sensor_MS8607.log=" + (String)settings.sensor_MS8607.log);
    //    settingsFile.println("sensor_MS8607.logHumidity=" + (String)settings.sensor_MS8607.logHumidity);
    //    settingsFile.println("sensor_MS8607.logPressure=" + (String)settings.sensor_MS8607.logPressure);
    //    settingsFile.println("sensor_MS8607.logTemperature=" + (String)settings.sensor_MS8607.logTemperature);
    //    settingsFile.println("sensor_MS8607.enableHeater=" + (String)settings.sensor_MS8607.enableHeater);
    //    settingsFile.println("sensor_MS8607.pressureResolution=" + (String)settings.sensor_MS8607.pressureResolution);
    //    settingsFile.println("sensor_MS8607.humidityResolution=" + (String)settings.sensor_MS8607.humidityResolution);

    //Step through the node list, recording each node's settings
    node *temp = new node;
    temp = head;
    char base[75];
    while (temp != NULL)
    {
      sprintf(base, "%s.%d.%d.%d.%d.", getDeviceName(temp->deviceType), temp->deviceType, temp->address, temp->muxAddress, temp->portNumber);

      switch (temp->deviceType)
      {
        case DEVICE_VOC_CCS811:
          {
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logTVOC=" + nodeSetting->logTVOC);
            settingsFile.println((String)base + "logCO2=" + nodeSetting->logCO2);
            break;
          }
        case DEVICE_DISTANCE_VL53L1X:
          {
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logDistance=" + nodeSetting->logDistance);
            settingsFile.println((String)base + "logRangeStatus=" + nodeSetting->logRangeStatus);
            settingsFile.println((String)base + "logSignalRate=" + nodeSetting->logSignalRate);
            settingsFile.println((String)base + "distanceMode=" + nodeSetting->distanceMode);
            settingsFile.println((String)base + "intermeasurementPeriod=" + nodeSetting->intermeasurementPeriod);
            settingsFile.println((String)base + "offset=" + nodeSetting->offset);
            settingsFile.println((String)base + "crosstalk=" + nodeSetting->crosstalk);
            break;
          }
        default:
          settingsFile.println((String)base + "=UnknownDeviceSettings");
          break;
      }

      temp = temp->next;
    }

    settingsFile.close();
  }
}


//If a config file exists on the SD card, load them and overwrite the local settings
//Heavily based on ReadCsvFile from SdFat library
//Returns true if some settings were loaded from a file
//Returns false if a file was not opened/loaded
bool loadSettingsFromFile()
{
  if (true == true)
  {
    if (sd.exists("OLA_settings.cfg"))
    {
#ifdef USE_EXFAT
      FsFile settingsFile; //exFat
#else
      File settingsFile; //FAT16/32
#endif
      if (settingsFile.open("OLA_settings.cfg", O_READ) == false)
      {
        Serial.println("Failed to open settings file");
        return (false);
      }

      char line[150];
      int lineNumber = 0;

      while (settingsFile.available()) {
        int n = settingsFile.fgets(line, sizeof(line));
        if (n <= 0) {
          Serial.printf("Failed to read line %d from settings file\n", lineNumber);
        }
        else if (line[n - 1] != '\n' && n == (sizeof(line) - 1)) {
          Serial.printf("Settings line %d too long\n", lineNumber);
          if (lineNumber == 0)
          {
            //If we can't read the first line of the settings file, give up
            Serial.println("Giving up on settings file");
            return (false);
          }
        }
        else if (parseLine(line) == false) {
          Serial.printf("Failed to parse line %d: %s\n", lineNumber, line);
          if (lineNumber == 0)
          {
            //If we can't read the first line of the settings file, give up
            Serial.println("Giving up on settings file");
            return (false);
          }
        }

        lineNumber++;
      }

      Serial.println("Config file read complete");
      settingsFile.close();
      return (true);
    }
    else
    {
      Serial.println("No config file found. Using settings from EEPROM.");
      //The defaults of the struct will be recorded to a file later on.
      return (false);
    }
  }

  Serial.println("Config file read failed: SD offline");
  return (false); //SD offline
}

// Check for extra characters in field or find minus sign.
char* skipSpace(char* str) {
  while (isspace(*str)) str++;
  return str;
}

//Convert a given line from file into a settingName and value
//Sets the setting if the name is known
bool parseLine(char* str) {
  char* ptr;

  //Debug
  //Serial.printf("Line contents: %s", str);
  //Serial.flush();

  // Set strtok start of line.
  str = strtok(str, "=");
  if (!str) return false;

  //Store this setting name
  char settingName[150];
  sprintf(settingName, "%s", str);

  //Move pointer to end of line
  str = strtok(nullptr, "\n");
  if (!str) return false;

  //Serial.printf("s = %s\n", str);
  //Serial.flush();

  // Convert string to double.
  double d = strtod(str, &ptr);
  if (str == ptr || *skipSpace(ptr)) return false;

  //Serial.printf("d = %lf\n", d);
  //Serial.flush();

  // Get setting name
  if (strcmp(settingName, "sizeOfSettings") == 0)
  {
    //Nothing
  }

  //Is this a device setting?
  //Break it into its constituent parts
  char deviceSettingName[50];
  deviceType_e deviceType;
  uint8_t address;
  uint8_t muxAddress;
  uint8_t portNumber;
  uint8_t count = 0;
  char *split = strtok(settingName, ".");
  while (split != NULL)
  {
    if (count == 0)
      ; //Do nothing. This is merely the human friendly device name
    else if (count == 1)
      deviceType = (deviceType_e)atoi(split);
    else if (count == 2)
      address = atoi(split);
    else if (count == 3)
      muxAddress = atoi(split);
    else if (count == 4)
      portNumber = atoi(split);
    else if (count == 5)
      sprintf(deviceSettingName, "%s", split);
    split = strtok(NULL, ".");
    count++;
  }

  Serial.printf("%d: %d.%d.%d - %s\n", deviceType, address, muxAddress, portNumber, deviceSettingName);

  //Find the device in the list that has this device type and address
  void *deviceConfigPtr = getConfigPointer(deviceType, address, muxAddress, portNumber);
  if (deviceConfigPtr != NULL)
  {
    switch (deviceType)
    {
      case DEVICE_VOC_CCS811:
      {
        struct_CCS811 *nodeSetting = (struct_CCS811 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does

        //Apply the appropriate settings
        if (strcmp(deviceSettingName, "log") == 0)
          nodeSetting->log = d;
        else if (strcmp(deviceSettingName, "logTVOC") == 0)
          nodeSetting->logTVOC = d;
        else if (strcmp(deviceSettingName, "logCO2") == 0)
          nodeSetting->logCO2 = d;
        else
          Serial.printf("Unknown device setting: %s\n", deviceSettingName);
        break;
      }
      case DEVICE_DISTANCE_VL53L1X:
      {
        struct_VL53L1X *nodeSetting = (struct_VL53L1X *)deviceConfigPtr; //Create a local pointer that points to same spot as node does

        //Apply the appropriate settings
        if (strcmp(deviceSettingName, "log") == 0)
          nodeSetting->log = d;
        else if (strcmp(deviceSettingName, "logDistance") == 0)
          nodeSetting->logDistance = d;
        else if (strcmp(deviceSettingName, "logRangeStatus") == 0)
          nodeSetting->logRangeStatus = d;
        else if (strcmp(deviceSettingName, "logSignalRate") == 0)
          nodeSetting->logSignalRate = d;
        else if (strcmp(deviceSettingName, "distanceMode") == 0)
          nodeSetting->distanceMode = d;
        else if (strcmp(deviceSettingName, "intermeasurementPeriod") == 0)
          nodeSetting->intermeasurementPeriod = d;
        else if (strcmp(deviceSettingName, "offset") == 0)
          nodeSetting->offset = d;
        else if (strcmp(deviceSettingName, "crosstalk") == 0)
          nodeSetting->crosstalk = d;
        else
          Serial.printf("Unknown device setting: %s\n", deviceSettingName);
        break;
      }
      default:
        break;
    }
  }

  //  else if (strcmp(settingName, "sensor_MS8607.log") == 0)
  //    settings.sensor_MS8607.log = d;
  //  else if (strcmp(settingName, "sensor_MS8607.pressureResolution") == 0)
  //    settings.sensor_MS8607.pressureResolution = (MS8607_pressure_resolution)d;
  //  else if (strcmp(settingName, "") == 0)
  //    settings. = d;
  //  else
  //  {
  //    Serial.print("Unknown setting: ");
  //    Serial.println(settingName);
  //  }

  return (true);
}

void beginSD()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected

  if (true == true)
  {
    microSDPowerOn();

    //Max power up time is 250ms: https://www.kingston.com/datasheets/SDCIT-specsheet-64gb_en.pdf
    //Max current is 200mA average across 1s, peak 300mA
    delay(10);

    if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
    {
      delay(250); //Give SD more time to power up, then try again
      if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
      {
        Serial.println("SD init failed. Is card present? Formatted?");
        digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected
        //online.microSD = false;
        return;
      }
    }
    //    }

    //Change to root directory. All new file creation will be in root.
    if (sd.chdir() == false)
    {
      Serial.println("SD change directory failed");
      //online.microSD = false;
      return;
    }

    //online.microSD = true;
  }
  else
  {
    microSDPowerOff();
    //online.microSD = false;
  }
}
