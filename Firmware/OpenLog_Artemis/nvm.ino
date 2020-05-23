void loadSettings()
{
  //First load any settings from NVM
  //After, we'll load settings from config file if available
  //We'll then re-record settings so that the settings from the file over-rides internal NVM settings

  //Check to see if EEPROM is blank
  uint32_t testRead = 0;
  if (EEPROM.get(0, testRead) == 0xFFFFFFFF)
  {
    recordSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
    Serial.println("Default settings applied");
  }

  //Check that the current settings struct size matches what is stored in EEPROM
  //Misalignment happens when we add a new feature or setting
  int tempSize = 0;
  EEPROM.get(0, tempSize); //Load the sizeOfSettings
  if (tempSize != sizeof(settings))
  {
    Serial.println("Settings wrong size. Default settings applied");
    recordSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
  }

  //Read current settings
  EEPROM.get(0, settings);

  //Load any settings from config file
  if (loadSettingsFromFile() == true)
    recordSettings(); //Record these new settings to EEPROM and config file
}

//Record the current settings struct to EEPROM and then to config file
void recordSettings()
{
  settings.sizeOfSettings = sizeof(settings);
  EEPROM.put(0, settings);
  recordSettingsToFile();
}

//Export the current settings to a config file
void recordSettingsToFile()
{
  if (online.microSD == true)
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

    settingsFile.println("sizeOfSettings=" + (String)settings.sizeOfSettings);
    settingsFile.println("nextSerialLogNumber=" + (String)settings.nextSerialLogNumber);
    settingsFile.println("nextDataLogNumber=" + (String)settings.nextDataLogNumber);

    char tempTime[20];
    sprintf(tempTime, "%lu", settings.usBetweenReadings);
    settingsFile.println("usBetweenReadings=" + (String)tempTime);

    settingsFile.println("logMaxRate=" + (String)settings.logMaxRate);
    settingsFile.println("enableRTC=" + (String)settings.enableRTC);
    settingsFile.println("enableIMU=" + (String)settings.enableIMU);
    settingsFile.println("enableSD=" + (String)settings.enableSD);
    settingsFile.println("enableTerminalOutput=" + (String)settings.enableTerminalOutput);
    settingsFile.println("logDate=" + (String)settings.logDate);
    settingsFile.println("logTime=" + (String)settings.logTime);
    settingsFile.println("logData=" + (String)settings.logData);
    settingsFile.println("logSerial=" + (String)settings.logSerial);
    settingsFile.println("logIMUAccel=" + (String)settings.logIMUAccel);
    settingsFile.println("logIMUGyro=" + (String)settings.logIMUGyro);
    settingsFile.println("logIMUMag=" + (String)settings.logIMUMag);
    settingsFile.println("logIMUTemp=" + (String)settings.logIMUTemp);
    settingsFile.println("logRTC=" + (String)settings.logRTC);
    settingsFile.println("logHertz=" + (String)settings.logHertz);
    settingsFile.println("getRTCfromGPS=" + (String)settings.getRTCfromGPS);
    settingsFile.println("correctForDST=" + (String)settings.correctForDST);
    settingsFile.println("americanDateStyle=" + (String)settings.americanDateStyle);
    settingsFile.println("hour24Style=" + (String)settings.hour24Style);
    settingsFile.println("serialTerminalBaudRate=" + (String)settings.serialTerminalBaudRate);
    settingsFile.println("serialLogBaudRate=" + (String)settings.serialLogBaudRate);
    settingsFile.println("showHelperText=" + (String)settings.showHelperText);
    settingsFile.println("logA11=" + (String)settings.logA11);
    settingsFile.println("logA12=" + (String)settings.logA12);
    settingsFile.println("logA13=" + (String)settings.logA13);
    settingsFile.println("logA32=" + (String)settings.logA32);
    settingsFile.println("logAnalogVoltages=" + (String)settings.logAnalogVoltages);
    settingsFile.println("localUTCOffset=" + (String)settings.localUTCOffset);
    settingsFile.println("printDebugMessages=" + (String)settings.printDebugMessages);
    settingsFile.println("powerDownQwiicBusBetweenReads=" + (String)settings.powerDownQwiicBusBetweenReads);
    settingsFile.println("qwiicBusMaxSpeed=" + (String)settings.qwiicBusMaxSpeed);
    //    settingsFile.println("=" + (String)settings.sensor_LPS25HB.);

    //Step through the device node list, recording each node's settings
    char base[75];
    node *temp = head;
    while (temp != NULL)
    {
      sprintf(base, "%s.%d.%d.%d.%d.", getDeviceName(temp->deviceType), temp->deviceType, temp->address, temp->muxAddress, temp->portNumber);

      switch (temp->deviceType)
      {
        case DEVICE_MULTIPLEXER:
          {
            //Currently, no settings for multiplexer to record
            //struct_multiplexer *nodeSetting = (struct_multiplexer *)temp->configPtr; //Create a local pointer that points to same spot as node does
            //settingsFile.println((String)base + "log=" + nodeSetting->log);
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
        case DEVICE_PHT_BME280:
          {
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logHumidity=" + nodeSetting->logHumidity);
            settingsFile.println((String)base + "logPressure=" + nodeSetting->logPressure);
            settingsFile.println((String)base + "logAltitude=" + nodeSetting->logAltitude);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
            break;
          }
        case DEVICE_VOC_CCS811:
          {
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logTVOC=" + nodeSetting->logTVOC);
            settingsFile.println((String)base + "logCO2=" + nodeSetting->logCO2);
            break;
          }
        default:
          Serial.printf("recordSettingsToFile Unknown device: %s\n", base);
          //settingsFile.println((String)base + "=UnknownDeviceSettings");
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
  if (online.microSD == true)
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

      char line[50];
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
  char settingName[30];
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
    //We may want to cause a factory reset from the settings file rather than the menu
    //If user sets sizeOfSettings to -1 in config file, OLA will factory reset
    if (d == -1)
    {
      EEPROM.erase();
      sd.remove("OLA_settings.cfg");
      Serial.println("OpenLog Artemis has been factory reset. Freezing. Please restart and open terminal at 115200bps.");
      while (1);
    }

    //Check to see if this setting file is compatible with this version of OLA
    if (d != sizeof(settings))
      Serial.printf("Warning: Settings size is %d but current firmware expects %d. Attempting to use settings from file.\n", d, sizeof(settings));

  }
  else if (strcmp(settingName, "nextSerialLogNumber") == 0)
    settings.nextSerialLogNumber = d;
  else if (strcmp(settingName, "nextDataLogNumber") == 0)
    settings.nextDataLogNumber = d;
  else if (strcmp(settingName, "usBetweenReadings") == 0)
    settings.usBetweenReadings = d;
  else if (strcmp(settingName, "logMaxRate") == 0)
    settings.logMaxRate = d;
  else if (strcmp(settingName, "enableRTC") == 0)
    settings.enableRTC = d;
  else if (strcmp(settingName, "enableIMU") == 0)
    settings.enableIMU = d;
  else if (strcmp(settingName, "enableSD") == 0)
    settings.enableSD = d;
  else if (strcmp(settingName, "enableTerminalOutput") == 0)
    settings.enableTerminalOutput = d;
  else if (strcmp(settingName, "logDate") == 0)
    settings.logDate = d;
  else if (strcmp(settingName, "logTime") == 0)
    settings.logTime = d;
  else if (strcmp(settingName, "logData") == 0)
    settings.logData = d;
  else if (strcmp(settingName, "logSerial") == 0)
    settings.logSerial = d;
  else if (strcmp(settingName, "logIMUAccel") == 0)
    settings.logIMUAccel = d;
  else if (strcmp(settingName, "logIMUGyro") == 0)
    settings.logIMUGyro = d;
  else if (strcmp(settingName, "logIMUMag") == 0)
    settings.logIMUMag = d;
  else if (strcmp(settingName, "logIMUTemp") == 0)
    settings.logIMUTemp = d;
  else if (strcmp(settingName, "logRTC") == 0)
    settings.logRTC = d;
  else if (strcmp(settingName, "logHertz") == 0)
    settings.logHertz = d;
  else if (strcmp(settingName, "getRTCfromGPS") == 0)
    settings.getRTCfromGPS = d;
  else if (strcmp(settingName, "correctForDST") == 0)
    settings.correctForDST = d;
  else if (strcmp(settingName, "americanDateStyle") == 0)
    settings.americanDateStyle = d;
  else if (strcmp(settingName, "hour24Style") == 0)
    settings.hour24Style = d;
  else if (strcmp(settingName, "serialTerminalBaudRate") == 0)
    settings.serialTerminalBaudRate = d;
  else if (strcmp(settingName, "serialLogBaudRate") == 0)
    settings.serialLogBaudRate = d;
  else if (strcmp(settingName, "showHelperText") == 0)
    settings.showHelperText = d;
  else if (strcmp(settingName, "logA11") == 0)
    settings.logA11 = d;
  else if (strcmp(settingName, "logA12") == 0)
    settings.logA12 = d;
  else if (strcmp(settingName, "logA13") == 0)
    settings.logA13 = d;
  else if (strcmp(settingName, "logA32") == 0)
    settings.logA32 = d;
  else if (strcmp(settingName, "logAnalogVoltages") == 0)
    settings.logAnalogVoltages = d;
  else if (strcmp(settingName, "localUTCOffset") == 0)
    settings.localUTCOffset = d;
  else if (strcmp(settingName, "printDebugMessages") == 0)
    settings.printDebugMessages = d;
  else if (strcmp(settingName, "powerDownQwiicBusBetweenReads") == 0)
    settings.powerDownQwiicBusBetweenReads = d;
  else if (strcmp(settingName, "qwiicBusMaxSpeed") == 0)
    settings.qwiicBusMaxSpeed = d;
  else
  {
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

    if (count < 5)
    {
      Serial.printf("Incomplete setting: %s\n", settingName);
      return false;
    }

    //Serial.printf("%d: %d.%d.%d - %s\n", deviceType, address, muxAddress, portNumber, deviceSettingName);
    //Serial.flush();

    //Find the device in the list that has this device type and address
    void *deviceConfigPtr = getConfigPointer(deviceType, address, muxAddress, portNumber);
    if (deviceConfigPtr == NULL)
    {
      Serial.printf("Setting in file found but no matching device on bus is available: %s\n", settingName);
      Serial.flush();
    }
    else
    {
      switch (deviceType)
      {
        case DEVICE_MULTIPLEXER:
          {
            Serial.println("There are no known settings for a multiplexer to load.");
          }
          break;
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
          }
          break;
        case DEVICE_PHT_BME280:
          {
            struct_BME280 *nodeSetting = (struct_BME280 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does

            //Apply the appropriate settings
            if (strcmp(deviceSettingName, "log") == 0)
              nodeSetting->log = d;
            else if (strcmp(deviceSettingName, "logHumidity") == 0)
              nodeSetting->logHumidity = d;
            else if (strcmp(deviceSettingName, "logPressure") == 0)
              nodeSetting->logPressure = d;
            else if (strcmp(deviceSettingName, "logAltitude") == 0)
              nodeSetting->logAltitude = d;
            else if (strcmp(deviceSettingName, "logTemperature") == 0)
              nodeSetting->logTemperature = d;
            else
              Serial.printf("Unknown device setting: %s\n", deviceSettingName);
          }
          break;
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
          }
          break;

        default:
          Serial.printf("Unknown device type: %d\n", deviceType);
          Serial.flush();
          break;
      }
    }
  }

  return (true);
}
