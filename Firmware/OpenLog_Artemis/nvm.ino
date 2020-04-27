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

    //File settingsFile; //FAT16/32
    FsFile settingsFile; //exFat
    if (settingsFile.open("OLA_settings.cfg", O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println("Failed to create settings file");
      return;
    }

    settingsFile.println("sizeOfSettings=" + (String)settings.sizeOfSettings);
    settingsFile.println("nextSerialLogNumber=" + (String)settings.nextSerialLogNumber);
    settingsFile.println("nextDataLogNumber=" + (String)settings.nextDataLogNumber);

    char temp[20];
    sprintf(temp, "%lu", settings.usBetweenReadings);
    settingsFile.println("usBetweenReadings=" + (String)temp);

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

    settingsFile.println("sensor_MS8607.log=" + (String)settings.sensor_MS8607.log);
    settingsFile.println("sensor_MS8607.logHumidity=" + (String)settings.sensor_MS8607.logHumidity);
    settingsFile.println("sensor_MS8607.logPressure=" + (String)settings.sensor_MS8607.logPressure);
    settingsFile.println("sensor_MS8607.logTemperature=" + (String)settings.sensor_MS8607.logTemperature);
    settingsFile.println("sensor_MS8607.enableHeater=" + (String)settings.sensor_MS8607.enableHeater);
    settingsFile.println("sensor_MS8607.pressureResolution=" + (String)settings.sensor_MS8607.pressureResolution);
    settingsFile.println("sensor_MS8607.humidityResolution=" + (String)settings.sensor_MS8607.humidityResolution);

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
      //File settingsFile; //FAT16/32
      FsFile settingsFile; //exFat
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
      while(1);
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

  /*
   LPS25HB
   NAU7802
   MCP9600
   VCNL4040
   */
  
  else if (strcmp(settingName, "sensor_MS8607.log") == 0)
    settings.sensor_MS8607.log = d;
  else if (strcmp(settingName, "sensor_MS8607.logHumidity") == 0)
    settings.sensor_MS8607.logHumidity = d;
  else if (strcmp(settingName, "sensor_MS8607.logPressure") == 0)
    settings.sensor_MS8607.logPressure = d;
  else if (strcmp(settingName, "sensor_MS8607.logTemperature") == 0)
    settings.sensor_MS8607.logTemperature = d;
  else if (strcmp(settingName, "sensor_MS8607.enableHeater") == 0)
    settings.sensor_MS8607.enableHeater = d;
  else if (strcmp(settingName, "sensor_MS8607.pressureResolution") == 0)
    settings.sensor_MS8607.pressureResolution = (MS8607_pressure_resolution)d;
  else if (strcmp(settingName, "sensor_MS8607.humidityResolution") == 0)
    settings.sensor_MS8607.humidityResolution = (MS8607_humidity_resolution)d;

  //  else if (strcmp(settingName, "") == 0)
  //    settings. = d;
  else
  {
    Serial.print("Unknown setting: ");
    Serial.println(settingName);
  }

  return (true);
}
