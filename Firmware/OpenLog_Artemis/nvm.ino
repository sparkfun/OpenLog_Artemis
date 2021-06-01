void loadSettings()
{
  //First load any settings from NVM
  //After, we'll load settings from config file if available
  //We'll then re-record settings so that the settings from the file over-rides internal NVM settings

  //Check to see if EEPROM is blank
  uint32_t testRead = 0;
  if (EEPROM.get(0, testRead) == 0xFFFFFFFF)
  {
    SerialPrintln(F("EEPROM is blank. Default settings applied"));
    recordSystemSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
  }

  //Check that the current settings struct size matches what is stored in EEPROM
  //Misalignment happens when we add a new feature or setting
  int tempSize = 0;
  EEPROM.get(0, tempSize); //Load the sizeOfSettings
  if (tempSize != sizeof(settings))
  {
    SerialPrintln(F("Settings wrong size. Default settings applied"));
    recordSystemSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
  }

  //Check that the olaIdentifier is correct
  //(It is possible for two different versions of the code to have the same sizeOfSettings - which causes problems!)
  int tempIdentifier = 0;
  EEPROM.get(sizeof(int), tempIdentifier); //Load the identifier from the EEPROM location after sizeOfSettings (int)
  if (tempIdentifier != OLA_IDENTIFIER)
  {
    SerialPrintln(F("Settings are not valid for this variant of the OLA. Default settings applied"));
    recordSystemSettings(); //Record default settings to EEPROM and config file. At power on, settings are in default state
  }

  //Read current settings
  EEPROM.get(0, settings);

  loadSystemSettingsFromFile(); //Load any settings from config file. This will over-write any pre-existing EEPROM settings.
  //Record these new settings to EEPROM and config file to be sure they are the same
  //(do this even if loadSystemSettingsFromFile returned false)
  recordSystemSettings();
}

//Record the current settings struct to EEPROM and then to config file
void recordSystemSettings()
{
  settings.sizeOfSettings = sizeof(settings);
  EEPROM.put(0, settings);
  recordSystemSettingsToFile();
}

//Export the current settings to a config file
void recordSystemSettingsToFile()
{
  if (online.microSD == true)
  {
    if (sd.exists("OLA_settings.txt"))
      sd.remove("OLA_settings.txt");

    #if SD_FAT_TYPE == 1
    File32 settingsFile;
    #elif SD_FAT_TYPE == 2
    ExFile settingsFile;
    #elif SD_FAT_TYPE == 3
    FsFile settingsFile;
    #else // SD_FAT_TYPE == 0
    File settingsFile;
    #endif  // SD_FAT_TYPE
    if (settingsFile.open("OLA_settings.txt", O_CREAT | O_APPEND | O_WRITE) == false)
    {
      SerialPrintln(F("Failed to create settings file"));
      return;
    }

    settingsFile.println("sizeOfSettings=" + (String)settings.sizeOfSettings);
    settingsFile.println("olaIdentifier=" + (String)settings.olaIdentifier);
    settingsFile.println("nextSerialLogNumber=" + (String)settings.nextSerialLogNumber);
    settingsFile.println("nextDataLogNumber=" + (String)settings.nextDataLogNumber);

    // Convert uint64_t to string
    // Based on printLLNumber by robtillaart
    // https://forum.arduino.cc/index.php?topic=143584.msg1519824#msg1519824
    char tempTimeRev[20]; // Char array to hold to usBetweenReadings (reversed order)
    char tempTime[20]; // Char array to hold to usBetweenReadings (correct order)
    uint64_t usBR = settings.usBetweenReadings;
    unsigned int i = 0;
    if (usBR == 0ULL) // if usBetweenReadings is zero, set tempTime to "0"
    {
      tempTime[0] = '0';
      tempTime[1] = 0;
    }
    else
    {
      while (usBR > 0)
      {
        tempTimeRev[i++] = (usBR % 10) + '0'; // divide by 10, convert the remainder to char
        usBR /= 10; // divide by 10
      }
      unsigned int j = 0;
      while (i > 0)
      {
        tempTime[j++] = tempTimeRev[--i]; // reverse the order
        tempTime[j] = 0; // mark the end with a NULL
      }
    }
    
    settingsFile.println("usBetweenReadings=" + (String)tempTime);

    //printDebug(F("Saving usBetweenReadings to SD card: "));
    //printDebug((String)tempTime);
    //printDebug(F("\r\n"));

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
    settingsFile.println("qwiicBusPowerUpDelayMs=" + (String)settings.qwiicBusPowerUpDelayMs);
    settingsFile.println("printMeasurementCount=" + (String)settings.printMeasurementCount);
    settingsFile.println("enablePwrLedDuringSleep=" + (String)settings.enablePwrLedDuringSleep);
    settingsFile.println("logVIN=" + (String)settings.logVIN);
    settingsFile.println("openNewLogFilesAfter=" + (String)settings.openNewLogFilesAfter);
    settingsFile.println("vinCorrectionFactor=" + (String)settings.vinCorrectionFactor);
    settingsFile.println("useGPIO32ForStopLogging=" + (String)settings.useGPIO32ForStopLogging);
    settingsFile.println("qwiicBusPullUps=" + (String)settings.qwiicBusPullUps);
    settingsFile.println("outputSerial=" + (String)settings.outputSerial);
    settingsFile.println("zmodemStartDelay=" + (String)settings.zmodemStartDelay);
    settingsFile.println("enableLowBatteryDetection=" + (String)settings.enableLowBatteryDetection);
    settingsFile.println("lowBatteryThreshold=" + (String)settings.lowBatteryThreshold);
    settingsFile.println("frequentFileAccessTimestamps=" + (String)settings.frequentFileAccessTimestamps);
    settingsFile.println("useGPIO11ForTrigger=" + (String)settings.useGPIO11ForTrigger);
    settingsFile.println("fallingEdgeTrigger=" + (String)settings.fallingEdgeTrigger);
    settingsFile.println("imuAccDLPF=" + (String)settings.imuAccDLPF);
    settingsFile.println("imuGyroDLPF=" + (String)settings.imuGyroDLPF);
    settingsFile.println("imuAccFSS=" + (String)settings.imuAccFSS);
    settingsFile.println("imuAccDLPFBW=" + (String)settings.imuAccDLPFBW);
    settingsFile.println("imuGyroFSS=" + (String)settings.imuGyroFSS);
    settingsFile.println("imuGyroDLPFBW=" + (String)settings.imuGyroDLPFBW);
    settingsFile.println("logMicroseconds=" + (String)settings.logMicroseconds);
    settingsFile.println("useTxRxPinsForTerminal=" + (String)settings.useTxRxPinsForTerminal);
    settingsFile.println("timestampSerial=" + (String)settings.timestampSerial);
    settingsFile.println("timeStampToken=" + (String)settings.timeStampToken);
    settingsFile.println("useGPIO11ForFastSlowLogging=" + (String)settings.useGPIO11ForFastSlowLogging);
    settingsFile.println("slowLoggingWhenPin11Is=" + (String)settings.slowLoggingWhenPin11Is);
    settingsFile.println("useRTCForFastSlowLogging=" + (String)settings.useRTCForFastSlowLogging);
    settingsFile.println("slowLoggingIntervalSeconds=" + (String)settings.slowLoggingIntervalSeconds);
    settingsFile.println("slowLoggingStartMOD=" + (String)settings.slowLoggingStartMOD);
    settingsFile.println("slowLoggingStopMOD=" + (String)settings.slowLoggingStopMOD);
    settingsFile.println("resetOnZeroDeviceCount=" + (String)settings.resetOnZeroDeviceCount);
    settingsFile.println("imuUseDMP=" + (String)settings.imuUseDMP);
    settingsFile.println("imuLogDMPQuat6=" + (String)settings.imuLogDMPQuat6);
    settingsFile.println("imuLogDMPQuat9=" + (String)settings.imuLogDMPQuat9);
    settingsFile.println("imuLogDMPAccel=" + (String)settings.imuLogDMPAccel);
    settingsFile.println("imuLogDMPGyro=" + (String)settings.imuLogDMPGyro);
    settingsFile.println("imuLogDMPCpass=" + (String)settings.imuLogDMPCpass);
    settingsFile.println("minimumAwakeTimeMillis=" + (String)settings.minimumAwakeTimeMillis);
    settingsFile.println("identifyBioSensorHubs=" + (String)settings.identifyBioSensorHubs);
    updateDataFileAccess(&settingsFile); // Update the file access time & date
    settingsFile.close();
  }
}

//If a config file exists on the SD card, load them and overwrite the local settings
//Heavily based on ReadCsvFile from SdFat library
//Returns true if some settings were loaded from a file
//Returns false if a file was not opened/loaded
bool loadSystemSettingsFromFile()
{
  if (online.microSD == true)
  {
    if (sd.exists("OLA_settings.txt"))
    {
      SdFile settingsFile; //FAT32
      if (settingsFile.open("OLA_settings.txt", O_READ) == false)
      {
        SerialPrintln(F("Failed to open settings file"));
        return (false);
      }

      char line[60];
      int lineNumber = 0;

      while (settingsFile.available()) {
        int n = settingsFile.fgets(line, sizeof(line));
        if (n <= 0) {
          SerialPrintf2("Failed to read line %d from settings file\r\n", lineNumber);
        }
        else if (line[n - 1] != '\n' && n == (sizeof(line) - 1)) {
          SerialPrintf2("Settings line %d too long\r\n", lineNumber);
          if (lineNumber == 0)
          {
            //If we can't read the first line of the settings file, give up
            SerialPrintln(F("Giving up on settings file"));
            return (false);
          }
        }
        else if (parseLine(line) == false) {
          SerialPrintf3("Failed to parse line %d: %s\r\n", lineNumber, line);
          if (lineNumber == 0)
          {
            //If we can't read the first line of the settings file, give up
            SerialPrintln(F("Giving up on settings file"));
            return (false);
          }
        }

        lineNumber++;
      }

      //SerialPrintln(F("Config file read complete"));
      settingsFile.close();
      return (true);
    }
    else
    {
      SerialPrintln(F("No config file found. Using settings from EEPROM."));
      //The defaults of the struct will be recorded to a file later on.
      return (false);
    }
  }

  SerialPrintln(F("Config file read failed: SD offline"));
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
  //SerialPrintf2("Line contents: %s", str);
  //SerialFlush();

  // Set strtok start of line.
  str = strtok(str, "=");
  if (!str) return false;

  //Store this setting name
  char settingName[40];
  sprintf(settingName, "%s", str);

  //Move pointer to end of line
  str = strtok(nullptr, "\n");
  if (!str) return false;

  //SerialPrintf2("s = %s\r\n", str);
  //SerialFlush();

  // Convert string to double.
  double d = strtod(str, &ptr);
  if (str == ptr || *skipSpace(ptr)) return false;

  //SerialPrintf2("d = %lf\r\n", d);
  //SerialFlush();

  // Get setting name
  if (strcmp(settingName, "sizeOfSettings") == 0)
  {
    //We may want to cause a factory reset from the settings file rather than the menu
    //If user sets sizeOfSettings to -1 in config file, OLA will factory reset
    if (d == -1)
    {
      EEPROM.erase();
      sd.remove("OLA_settings.txt");
      SerialPrintln(F("OpenLog Artemis has been factory reset. Freezing. Please restart and open terminal at 115200bps."));
      while (1);
    }

    //Check to see if this setting file is compatible with this version of OLA
    if (d != sizeof(settings))
      SerialPrintf3("Warning: Settings size is %d but current firmware expects %d. Attempting to use settings from file.\r\n", d, sizeof(settings));

  }
  else if (strcmp(settingName, "olaIdentifier") == 0)
    settings.olaIdentifier = d;
  else if (strcmp(settingName, "nextSerialLogNumber") == 0)
    settings.nextSerialLogNumber = d;
  else if (strcmp(settingName, "nextDataLogNumber") == 0)
    settings.nextDataLogNumber = d;
  else if (strcmp(settingName, "usBetweenReadings") == 0)
  {
    settings.usBetweenReadings = d;
    //printDebug(F("Read usBetweenReadings from SD card: "));
    //printDebug(String(d));
    //printDebug(F("\r\n"));
  }
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
  else if (strcmp(settingName, "qwiicBusPowerUpDelayMs") == 0)
    settings.qwiicBusPowerUpDelayMs = d;
  else if (strcmp(settingName, "printMeasurementCount") == 0)
    settings.printMeasurementCount = d;
  else if (strcmp(settingName, "enablePwrLedDuringSleep") == 0)
    settings.enablePwrLedDuringSleep = d;
  else if (strcmp(settingName, "logVIN") == 0)
    settings.logVIN = d;
  else if (strcmp(settingName, "openNewLogFilesAfter") == 0)
    settings.openNewLogFilesAfter = d;
  else if (strcmp(settingName, "vinCorrectionFactor") == 0)
    settings.vinCorrectionFactor = d;
  else if (strcmp(settingName, "useGPIO32ForStopLogging") == 0)
    settings.useGPIO32ForStopLogging = d;
  else if (strcmp(settingName, "qwiicBusPullUps") == 0)
    settings.qwiicBusPullUps = d;
  else if (strcmp(settingName, "outputSerial") == 0)
    settings.outputSerial = d;
  else if (strcmp(settingName, "zmodemStartDelay") == 0)
    settings.zmodemStartDelay = d;
  else if (strcmp(settingName, "enableLowBatteryDetection") == 0)
    settings.enableLowBatteryDetection = d;
  else if (strcmp(settingName, "lowBatteryThreshold") == 0)
    settings.lowBatteryThreshold = d;
  else if (strcmp(settingName, "frequentFileAccessTimestamps") == 0)
    settings.frequentFileAccessTimestamps = d;
  else if (strcmp(settingName, "useGPIO11ForTrigger") == 0)
    settings.useGPIO11ForTrigger = d;
  else if (strcmp(settingName, "fallingEdgeTrigger") == 0)
    settings.fallingEdgeTrigger = d;
  else if (strcmp(settingName, "imuAccDLPF") == 0)
    settings.imuAccDLPF = d;
  else if (strcmp(settingName, "imuGyroDLPF") == 0)
    settings.imuGyroDLPF = d;
  else if (strcmp(settingName, "imuAccFSS") == 0)
    settings.imuAccFSS = d;
  else if (strcmp(settingName, "imuAccDLPFBW") == 0)
    settings.imuAccDLPFBW = d;
  else if (strcmp(settingName, "imuGyroFSS") == 0)
    settings.imuGyroFSS = d;
  else if (strcmp(settingName, "imuGyroDLPFBW") == 0)
    settings.imuGyroDLPFBW = d;
  else if (strcmp(settingName, "logMicroseconds") == 0)
    settings.logMicroseconds = d;
  else if (strcmp(settingName, "useTxRxPinsForTerminal") == 0)
    settings.useTxRxPinsForTerminal = d;
  else if (strcmp(settingName, "timestampSerial") == 0)
    settings.timestampSerial = d;
  else if (strcmp(settingName, "timeStampToken") == 0)
    settings.timeStampToken = d;
  else if (strcmp(settingName, "useGPIO11ForFastSlowLogging") == 0)
    settings.useGPIO11ForFastSlowLogging = d;
  else if (strcmp(settingName, "slowLoggingWhenPin11Is") == 0)
    settings.slowLoggingWhenPin11Is = d;
  else if (strcmp(settingName, "useRTCForFastSlowLogging") == 0)
    settings.useRTCForFastSlowLogging = d;
  else if (strcmp(settingName, "slowLoggingIntervalSeconds") == 0)
    settings.slowLoggingIntervalSeconds = d;
  else if (strcmp(settingName, "slowLoggingStartMOD") == 0)
    settings.slowLoggingStartMOD = d;
  else if (strcmp(settingName, "slowLoggingStopMOD") == 0)
    settings.slowLoggingStopMOD = d;
  else if (strcmp(settingName, "resetOnZeroDeviceCount") == 0)
    settings.resetOnZeroDeviceCount = d;
  else if (strcmp(settingName, "imuUseDMP") == 0)
    settings.imuUseDMP = d;
  else if (strcmp(settingName, "imuLogDMPQuat6") == 0)
    settings.imuLogDMPQuat6 = d;
  else if (strcmp(settingName, "imuLogDMPQuat9") == 0)
    settings.imuLogDMPQuat9 = d;
  else if (strcmp(settingName, "imuLogDMPAccel") == 0)
    settings.imuLogDMPAccel = d;
  else if (strcmp(settingName, "imuLogDMPGyro") == 0)
    settings.imuLogDMPGyro = d;
  else if (strcmp(settingName, "imuLogDMPCpass") == 0)
    settings.imuLogDMPCpass = d;
  else if (strcmp(settingName, "minimumAwakeTimeMillis") == 0)
    settings.minimumAwakeTimeMillis = d;
  else if (strcmp(settingName, "identifyBioSensorHubs") == 0)
    settings.identifyBioSensorHubs = d;
  else
    {
      SerialPrintf2("Unknown setting %s. Ignoring...\r\n", settingName);
      return(false);
    }

  return (true);
}

//Export the current device settings to a config file
void recordDeviceSettingsToFile()
{
  if (online.microSD == true)
  {
    if (sd.exists("OLA_deviceSettings.txt"))
      sd.remove("OLA_deviceSettings.txt");

    #if SD_FAT_TYPE == 1
    File32 settingsFile;
    #elif SD_FAT_TYPE == 2
    ExFile settingsFile;
    #elif SD_FAT_TYPE == 3
    FsFile settingsFile;
    #else // SD_FAT_TYPE == 0
    File settingsFile;
    #endif  // SD_FAT_TYPE
    if (settingsFile.open("OLA_deviceSettings.txt", O_CREAT | O_APPEND | O_WRITE) == false)
    {
      SerialPrintln(F("Failed to create device settings file"));
      return;
    }

    //Step through the node list, recording each node's settings
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
          }
          break;
        case DEVICE_LOADCELL_NAU7802:
          {
            struct_NAU7802 *nodeSetting = (struct_NAU7802 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "calibrationFactor=" + nodeSetting->calibrationFactor);
            settingsFile.println((String)base + "zeroOffset=" + nodeSetting->zeroOffset);
            settingsFile.println((String)base + "decimalPlaces=" + nodeSetting->decimalPlaces);
            settingsFile.println((String)base + "averageAmount=" + nodeSetting->averageAmount);
          }
          break;
        case DEVICE_DISTANCE_VL53L1X:
          {
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logDistance=" + nodeSetting->logDistance);
            settingsFile.println((String)base + "logRangeStatus=" + nodeSetting->logRangeStatus);
            settingsFile.println((String)base + "logSignalRate=" + nodeSetting->logSignalRate);
            settingsFile.println((String)base + "distanceMode=" + nodeSetting->distanceMode);
            settingsFile.println((String)base + "intermeasurementPeriod=" + nodeSetting->intermeasurementPeriod);
            settingsFile.println((String)base + "offset=" + nodeSetting->offset);
            settingsFile.println((String)base + "crosstalk=" + nodeSetting->crosstalk);
          }
          break;
        case DEVICE_GPS_UBLOX:
          {
            struct_uBlox *nodeSetting = (struct_uBlox *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logDate=" + nodeSetting->logDate);
            settingsFile.println((String)base + "logTime=" + nodeSetting->logTime);
            settingsFile.println((String)base + "logPosition=" + nodeSetting->logPosition);
            settingsFile.println((String)base + "logAltitude=" + nodeSetting->logAltitude);
            settingsFile.println((String)base + "logAltitudeMSL=" + nodeSetting->logAltitudeMSL);
            settingsFile.println((String)base + "logSIV=" + nodeSetting->logSIV);
            settingsFile.println((String)base + "logFixType=" + nodeSetting->logFixType);
            settingsFile.println((String)base + "logCarrierSolution=" + nodeSetting->logCarrierSolution);
            settingsFile.println((String)base + "logGroundSpeed=" + nodeSetting->logGroundSpeed);
            settingsFile.println((String)base + "logHeadingOfMotion=" + nodeSetting->logHeadingOfMotion);
            settingsFile.println((String)base + "logpDOP=" + nodeSetting->logpDOP);
            settingsFile.println((String)base + "logiTOW=" + nodeSetting->logiTOW);
            settingsFile.println((String)base + "i2cSpeed=" + nodeSetting->i2cSpeed);
            settingsFile.println((String)base + "useAutoPVT=" + nodeSetting->useAutoPVT);
          }
          break;
        case DEVICE_PROXIMITY_VCNL4040:
          {
            struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logProximity=" + nodeSetting->logProximity);
            settingsFile.println((String)base + "logAmbientLight=" + nodeSetting->logAmbientLight);
            settingsFile.println((String)base + "LEDCurrent=" + nodeSetting->LEDCurrent);
            settingsFile.println((String)base + "IRDutyCycle=" + nodeSetting->IRDutyCycle);
            settingsFile.println((String)base + "proximityIntegrationTime=" + nodeSetting->proximityIntegrationTime);
            settingsFile.println((String)base + "ambientIntegrationTime=" + nodeSetting->ambientIntegrationTime);
            settingsFile.println((String)base + "resolution=" + nodeSetting->resolution);
          }
          break;
        case DEVICE_TEMPERATURE_TMP117:
          {
            struct_TMP117 *nodeSetting = (struct_TMP117 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
          }
          break;
        case DEVICE_PRESSURE_MS5637:
          {
            struct_MS5637 *nodeSetting = (struct_MS5637 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logPressure=" + nodeSetting->logPressure);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
          }
          break;
        case DEVICE_PRESSURE_LPS25HB:
          {
            struct_LPS25HB *nodeSetting = (struct_LPS25HB *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logPressure=" + nodeSetting->logPressure);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
          }
          break;
        case DEVICE_PHT_BME280:
          {
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logHumidity=" + nodeSetting->logHumidity);
            settingsFile.println((String)base + "logPressure=" + nodeSetting->logPressure);
            settingsFile.println((String)base + "logAltitude=" + nodeSetting->logAltitude);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
          }
          break;
        case DEVICE_UV_VEML6075:
          {
            struct_VEML6075 *nodeSetting = (struct_VEML6075 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logUVA=" + nodeSetting->logUVA);
            settingsFile.println((String)base + "logUVB=" + nodeSetting->logUVB);
            settingsFile.println((String)base + "logUVIndex=" + nodeSetting->logUVIndex);
          }
          break;
        case DEVICE_VOC_CCS811:
          {
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logTVOC=" + nodeSetting->logTVOC);
            settingsFile.println((String)base + "logCO2=" + nodeSetting->logCO2);
          }
          break;
        case DEVICE_VOC_SGP30:
          {
            struct_SGP30 *nodeSetting = (struct_SGP30 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logTVOC=" + nodeSetting->logTVOC);
            settingsFile.println((String)base + "logCO2=" + nodeSetting->logCO2);
            settingsFile.println((String)base + "logH2=" + nodeSetting->logH2);
            settingsFile.println((String)base + "logEthanol=" + nodeSetting->logEthanol);
          }
          break;
        case DEVICE_CO2_SCD30:
          {
            struct_SCD30 *nodeSetting = (struct_SCD30 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logCO2=" + nodeSetting->logCO2);
            settingsFile.println((String)base + "logHumidity=" + nodeSetting->logHumidity);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
            settingsFile.println((String)base + "measurementInterval=" + nodeSetting->measurementInterval);
            settingsFile.println((String)base + "altitudeCompensation=" + nodeSetting->altitudeCompensation);
            settingsFile.println((String)base + "ambientPressure=" + nodeSetting->ambientPressure);
            settingsFile.println((String)base + "temperatureOffset=" + nodeSetting->temperatureOffset);
          }
          break;
        case DEVICE_PHT_MS8607:
          {
            struct_MS8607 *nodeSetting = (struct_MS8607 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logHumidity=" + nodeSetting->logHumidity);
            settingsFile.println((String)base + "logPressure=" + nodeSetting->logPressure);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
            settingsFile.println((String)base + "enableHeater=" + nodeSetting->enableHeater);
            settingsFile.println((String)base + "pressureResolution=" + nodeSetting->pressureResolution);
            settingsFile.println((String)base + "humidityResolution=" + nodeSetting->humidityResolution);
          }
          break;
        case DEVICE_TEMPERATURE_MCP9600:
          {
            struct_MCP9600 *nodeSetting = (struct_MCP9600 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
            settingsFile.println((String)base + "logAmbientTemperature=" + nodeSetting->logAmbientTemperature);
          }
          break;
        case DEVICE_HUMIDITY_AHT20:
          {
            struct_AHT20 *nodeSetting = (struct_AHT20 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logHumidity=" + nodeSetting->logHumidity);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
          }
          break;
        case DEVICE_HUMIDITY_SHTC3:
          {
            struct_SHTC3 *nodeSetting = (struct_SHTC3 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logHumidity=" + nodeSetting->logHumidity);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
          }
          break;
        case DEVICE_ADC_ADS122C04:
          {
            struct_ADS122C04 *nodeSetting = (struct_ADS122C04 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logCentigrade=" + nodeSetting->logCentigrade);
            settingsFile.println((String)base + "logFahrenheit=" + nodeSetting->logFahrenheit);
            settingsFile.println((String)base + "logInternalTemperature=" + nodeSetting->logInternalTemperature);
            settingsFile.println((String)base + "logRawVoltage=" + nodeSetting->logRawVoltage);
            settingsFile.println((String)base + "useFourWireMode=" + nodeSetting->useFourWireMode);
            settingsFile.println((String)base + "useThreeWireMode=" + nodeSetting->useThreeWireMode);
            settingsFile.println((String)base + "useTwoWireMode=" + nodeSetting->useTwoWireMode);
            settingsFile.println((String)base + "useFourWireHighTemperatureMode=" + nodeSetting->useFourWireHighTemperatureMode);
            settingsFile.println((String)base + "useThreeWireHighTemperatureMode=" + nodeSetting->useThreeWireHighTemperatureMode);
            settingsFile.println((String)base + "useTwoWireHighTemperatureMode=" + nodeSetting->useTwoWireHighTemperatureMode);
          }
          break;
        case DEVICE_PRESSURE_MPR0025PA1:
          {
            struct_MPR0025PA1 *nodeSetting = (struct_MPR0025PA1 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "minimumPSI=" + nodeSetting->minimumPSI);
            settingsFile.println((String)base + "maximumPSI=" + nodeSetting->maximumPSI);
            settingsFile.println((String)base + "usePSI=" + nodeSetting->usePSI);
            settingsFile.println((String)base + "usePA=" + nodeSetting->usePA);
            settingsFile.println((String)base + "useKPA=" + nodeSetting->useKPA);
            settingsFile.println((String)base + "useTORR=" + nodeSetting->useTORR);
            settingsFile.println((String)base + "useINHG=" + nodeSetting->useINHG);
            settingsFile.println((String)base + "useATM=" + nodeSetting->useATM);
            settingsFile.println((String)base + "useBAR=" + nodeSetting->useBAR);
          }
          break;
        case DEVICE_PARTICLE_SNGCJA5:
          {
            struct_SNGCJA5 *nodeSetting = (struct_SNGCJA5 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logPM1=" + nodeSetting->logPM1);
            settingsFile.println((String)base + "logPM25=" + nodeSetting->logPM25);
            settingsFile.println((String)base + "logPM10=" + nodeSetting->logPM10);
            settingsFile.println((String)base + "logPC05=" + nodeSetting->logPC05);
            settingsFile.println((String)base + "logPC1=" + nodeSetting->logPC1);
            settingsFile.println((String)base + "logPC25=" + nodeSetting->logPC25);
            settingsFile.println((String)base + "logPC50=" + nodeSetting->logPC50);
            settingsFile.println((String)base + "logPC75=" + nodeSetting->logPC75);
            settingsFile.println((String)base + "logPC10=" + nodeSetting->logPC10);
            settingsFile.println((String)base + "logSensorStatus=" + nodeSetting->logSensorStatus);
            settingsFile.println((String)base + "logPDStatus=" + nodeSetting->logPDStatus);
            settingsFile.println((String)base + "logLDStatus=" + nodeSetting->logLDStatus);
            settingsFile.println((String)base + "logFanStatus=" + nodeSetting->logFanStatus);
          }
          break;
        case DEVICE_VOC_SGP40:
          {
            struct_SGP40 *nodeSetting = (struct_SGP40 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logVOC=" + nodeSetting->logVOC);
            settingsFile.println((String)base + "RH=" + nodeSetting->RH);
            settingsFile.println((String)base + "T=" + nodeSetting->T);
          }
          break;
        case DEVICE_PRESSURE_SDP3X:
          {
            struct_SDP3X *nodeSetting = (struct_SDP3X *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logPressure=" + nodeSetting->logPressure);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
            settingsFile.println((String)base + "massFlow=" + nodeSetting->massFlow);
            settingsFile.println((String)base + "averaging=" + nodeSetting->averaging);
          }
          break;
        case DEVICE_PRESSURE_MS5837:
          {
            struct_MS5837 *nodeSetting = (struct_MS5837 *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logPressure=" + nodeSetting->logPressure);
            settingsFile.println((String)base + "logTemperature=" + nodeSetting->logTemperature);
            settingsFile.println((String)base + "logDepth=" + nodeSetting->logDepth);
            settingsFile.println((String)base + "logAltitude=" + nodeSetting->logAltitude);
            settingsFile.println((String)base + "model=" + nodeSetting->model);
            settingsFile.println((String)base + "fluidDensity=" + nodeSetting->fluidDensity);
            settingsFile.println((String)base + "conversion=" + nodeSetting->conversion);
          }
          break;
//        case DEVICE_QWIIC_BUTTON:
//          {
//            struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)temp->configPtr;
//            settingsFile.println((String)base + "log=" + nodeSetting->log);
//            settingsFile.println((String)base + "logPressed=" + nodeSetting->logPressed);
//            settingsFile.println((String)base + "logClicked=" + nodeSetting->logClicked);
//            settingsFile.println((String)base + "toggleLEDOnClick=" + nodeSetting->toggleLEDOnClick);
//            settingsFile.println((String)base + "ledBrightness=" + nodeSetting->ledBrightness);
//          }
//          break;
        case DEVICE_BIO_SENSOR_HUB:
          {
            struct_BIO_SENSOR_HUB *nodeSetting = (struct_BIO_SENSOR_HUB *)temp->configPtr;
            settingsFile.println((String)base + "log=" + nodeSetting->log);
            settingsFile.println((String)base + "logHeartrate=" + nodeSetting->logHeartrate);
            settingsFile.println((String)base + "logConfidence=" + nodeSetting->logConfidence);
            settingsFile.println((String)base + "logOxygen=" + nodeSetting->logOxygen);
            settingsFile.println((String)base + "logStatus=" + nodeSetting->logStatus);
            settingsFile.println((String)base + "logExtendedStatus=" + nodeSetting->logExtendedStatus);
            settingsFile.println((String)base + "logRValue=" + nodeSetting->logRValue);
          }
          break;
        default:
          SerialPrintf2("recordSettingsToFile Unknown device: %s\r\n", base);
          //settingsFile.println((String)base + "=UnknownDeviceSettings");
          break;
      }
      temp = temp->next;
    }
    updateDataFileAccess(&settingsFile); // Update the file access time & date
    settingsFile.close();
  }
}

//If a device config file exists on the SD card, load them and overwrite the local settings
//Heavily based on ReadCsvFile from SdFat library
//Returns true if some settings were loaded from a file
//Returns false if a file was not opened/loaded
bool loadDeviceSettingsFromFile()
{
  if (online.microSD == true)
  {
    if (sd.exists("OLA_deviceSettings.txt"))
    {
      #if SD_FAT_TYPE == 1
      File32 settingsFile;
      #elif SD_FAT_TYPE == 2
      ExFile settingsFile;
      #elif SD_FAT_TYPE == 3
      FsFile settingsFile;
      #else // SD_FAT_TYPE == 0
      File settingsFile;
      #endif  // SD_FAT_TYPE

      if (settingsFile.open("OLA_deviceSettings.txt", O_READ) == false)
      {
        SerialPrintln(F("Failed to open device settings file"));
        return (false);
      }

      char line[150];
      int lineNumber = 0;

      while (settingsFile.available()) {
        int n = settingsFile.fgets(line, sizeof(line));
        if (n <= 0) {
          SerialPrintf2("Failed to read line %d from settings file\r\n", lineNumber);
        }
        else if (line[n - 1] != '\n' && n == (sizeof(line) - 1)) {
          SerialPrintf2("Settings line %d too long\n", lineNumber);
        }
        else if (parseDeviceLine(line) == false) {
          SerialPrintf3("Failed to parse line %d: %s\r\n", lineNumber + 1, line);
        }

        lineNumber++;
      }

      //SerialPrintln(F("Device config file read complete"));
      updateDataFileAccess(&settingsFile); // Update the file access time & date
      settingsFile.close();
      return (true);
    }
    else
    {
      SerialPrintln(F("No device config file found. Creating one with device defaults."));
      recordDeviceSettingsToFile(); //Record the current settings to create the initial file
      return (false);
    }
  }

  SerialPrintln(F("Device config file read failed: SD offline"));
  return (false); //SD offline
}

//Convert a given line from device setting file into a settingName and value
//Immediately applies the setting to the appropriate node
bool parseDeviceLine(char* str) {
  char* ptr;

  //Debug
  //SerialPrintf2("Line contents: %s", str);
  //SerialFlush();

  // Set strtok start of line.
  str = strtok(str, "=");
  if (!str) return false;

  //Store this setting name
  char settingName[150];
  sprintf(settingName, "%s", str);

  //Move pointer to end of line
  str = strtok(nullptr, "\n");
  if (!str) return false;

  //SerialPrintf2("s = %s\r\n", str);
  //SerialFlush();

  // Convert string to double.
  double d = strtod(str, &ptr);
  if (str == ptr || *skipSpace(ptr)) return false;

  //SerialPrintf2("d = %lf\r\n", d);
  //SerialFlush();

  //Break device setting into its constituent parts
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
    SerialPrintf2("Incomplete setting: %s\r\n", settingName);
    return false;
  }

  //SerialPrintf6("%d: %d.%d.%d - %s\r\n", deviceType, address, muxAddress, portNumber, deviceSettingName);
  //SerialFlush();

  //Find the device in the list that has this device type and address
  void *deviceConfigPtr = getConfigPointer(deviceType, address, muxAddress, portNumber);
  if (deviceConfigPtr == NULL)
  {
    //SerialPrintf2("Setting in file found but no matching device on bus is available: %s\r\n", settingName);
    //SerialFlush();
  }
  else
  {
    switch (deviceType)
    {
      case DEVICE_MULTIPLEXER:
        {
          SerialPrintln(F("There are no known settings for a multiplexer to load."));
        }
        break;
      case DEVICE_LOADCELL_NAU7802:
        {
          struct_NAU7802 *nodeSetting = (struct_NAU7802 *)deviceConfigPtr;

          //Apply the appropriate settings
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "calibrationFactor") == 0)
            nodeSetting->calibrationFactor = d;
          else if (strcmp(deviceSettingName, "zeroOffset") == 0)
            nodeSetting->zeroOffset = d;
          else if (strcmp(deviceSettingName, "decimalPlaces") == 0)
            nodeSetting->decimalPlaces = d;
          else if (strcmp(deviceSettingName, "averageAmount") == 0)
            nodeSetting->averageAmount = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_DISTANCE_VL53L1X:
        {
          struct_VL53L1X *nodeSetting = (struct_VL53L1X *)deviceConfigPtr;
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
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_GPS_UBLOX:
        {
          struct_uBlox *nodeSetting = (struct_uBlox *)deviceConfigPtr;

          //Apply the appropriate settings
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logDate") == 0)
            nodeSetting->logDate = d;
          else if (strcmp(deviceSettingName, "logTime") == 0)
            nodeSetting->logTime = d;
          else if (strcmp(deviceSettingName, "logPosition") == 0)
            nodeSetting->logPosition = d;
          else if (strcmp(deviceSettingName, "logAltitude") == 0)
            nodeSetting->logAltitude = d;
          else if (strcmp(deviceSettingName, "logAltitudeMSL") == 0)
            nodeSetting->logAltitudeMSL = d;
          else if (strcmp(deviceSettingName, "logSIV") == 0)
            nodeSetting->logSIV = d;
          else if (strcmp(deviceSettingName, "logFixType") == 0)
            nodeSetting->logFixType = d;
          else if (strcmp(deviceSettingName, "logCarrierSolution") == 0)
            nodeSetting->logCarrierSolution = d;
          else if (strcmp(deviceSettingName, "logGroundSpeed") == 0)
            nodeSetting->logGroundSpeed = d;
          else if (strcmp(deviceSettingName, "logHeadingOfMotion") == 0)
            nodeSetting->logHeadingOfMotion = d;
          else if (strcmp(deviceSettingName, "logpDOP") == 0)
            nodeSetting->logpDOP = d;
          else if (strcmp(deviceSettingName, "logiTOW") == 0)
            nodeSetting->logiTOW = d;
          else if (strcmp(deviceSettingName, "i2cSpeed") == 0)
            nodeSetting->i2cSpeed = d;
          else if (strcmp(deviceSettingName, "useAutoPVT") == 0)
            nodeSetting->useAutoPVT = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PROXIMITY_VCNL4040:
        {
          struct_VCNL4040 *nodeSetting = (struct_VCNL4040 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logProximity") == 0)
            nodeSetting->logProximity = d;
          else if (strcmp(deviceSettingName, "logAmbientLight") == 0)
            nodeSetting->logAmbientLight = d;
          else if (strcmp(deviceSettingName, "LEDCurrent") == 0)
            nodeSetting->LEDCurrent = d;
          else if (strcmp(deviceSettingName, "IRDutyCycle") == 0)
            nodeSetting->IRDutyCycle = d;
          else if (strcmp(deviceSettingName, "proximityIntegrationTime") == 0)
            nodeSetting->proximityIntegrationTime = d;
          else if (strcmp(deviceSettingName, "ambientIntegrationTime") == 0)
            nodeSetting->ambientIntegrationTime = d;
          else if (strcmp(deviceSettingName, "resolution") == 0)
            nodeSetting->resolution = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_TEMPERATURE_TMP117:
        {
          struct_TMP117 *nodeSetting = (struct_TMP117 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PRESSURE_MS5637:
        {
          struct_MS5637 *nodeSetting = (struct_MS5637 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logPressure") == 0)
            nodeSetting->logPressure = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PRESSURE_LPS25HB:
        {
          struct_LPS25HB *nodeSetting = (struct_LPS25HB *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logPressure") == 0)
            nodeSetting->logPressure = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PHT_BME280:
        {
          struct_BME280 *nodeSetting = (struct_BME280 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
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
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_UV_VEML6075:
        {
          struct_VEML6075 *nodeSetting = (struct_VEML6075 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logUVA") == 0)
            nodeSetting->logUVA = d;
          else if (strcmp(deviceSettingName, "logUVB") == 0)
            nodeSetting->logUVB = d;
          else if (strcmp(deviceSettingName, "logUVIndex") == 0)
            nodeSetting->logUVIndex = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_VOC_CCS811:
        {
          struct_CCS811 *nodeSetting = (struct_CCS811 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logTVOC") == 0)
            nodeSetting->logTVOC = d;
          else if (strcmp(deviceSettingName, "logCO2") == 0)
            nodeSetting->logCO2 = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_VOC_SGP30:
        {
          struct_SGP30 *nodeSetting = (struct_SGP30 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logTVOC") == 0)
            nodeSetting->logTVOC = d;
          else if (strcmp(deviceSettingName, "logCO2") == 0)
            nodeSetting->logCO2 = d;
          else if (strcmp(deviceSettingName, "logH2") == 0)
            nodeSetting->logH2 = d;
          else if (strcmp(deviceSettingName, "logEthanol") == 0)
            nodeSetting->logEthanol = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_CO2_SCD30:
        {
          struct_SCD30 *nodeSetting = (struct_SCD30 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logCO2") == 0)
            nodeSetting->logCO2 = d;
          else if (strcmp(deviceSettingName, "logHumidity") == 0)
            nodeSetting->logHumidity = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else if (strcmp(deviceSettingName, "measurementInterval") == 0)
            nodeSetting->measurementInterval = d;
          else if (strcmp(deviceSettingName, "altitudeCompensation") == 0)
            nodeSetting->altitudeCompensation = d;
          else if (strcmp(deviceSettingName, "ambientPressure") == 0)
            nodeSetting->ambientPressure = d;
          else if (strcmp(deviceSettingName, "temperatureOffset") == 0)
            nodeSetting->temperatureOffset = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PHT_MS8607:
        {
          struct_MS8607 *nodeSetting = (struct_MS8607 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logHumidity") == 0)
            nodeSetting->logHumidity = d;
          else if (strcmp(deviceSettingName, "logPressure") == 0)
            nodeSetting->logPressure = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else if (strcmp(deviceSettingName, "enableHeater") == 0)
            nodeSetting->enableHeater = d;
          else if (strcmp(deviceSettingName, "pressureResolution") == 0)
            nodeSetting->pressureResolution = (MS8607_pressure_resolution)d;
          else if (strcmp(deviceSettingName, "humidityResolution") == 0)
            nodeSetting->humidityResolution = (MS8607_humidity_resolution)d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_TEMPERATURE_MCP9600:
        {
          struct_MCP9600 *nodeSetting = (struct_MCP9600 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else if (strcmp(deviceSettingName, "logAmbientTemperature") == 0)
            nodeSetting->logAmbientTemperature = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_HUMIDITY_AHT20:
        {
          struct_AHT20 *nodeSetting = (struct_AHT20 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logHumidity") == 0)
            nodeSetting->logHumidity = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_HUMIDITY_SHTC3:
        {
          struct_SHTC3 *nodeSetting = (struct_SHTC3 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logHumidity") == 0)
            nodeSetting->logHumidity = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_ADC_ADS122C04:
        {
          struct_ADS122C04 *nodeSetting = (struct_ADS122C04 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logCentigrade") == 0)
            nodeSetting->logCentigrade = d;
          else if (strcmp(deviceSettingName, "logFahrenheit") == 0)
            nodeSetting->logFahrenheit = d;
          else if (strcmp(deviceSettingName, "logInternalTemperature") == 0)
            nodeSetting->logInternalTemperature = d;
          else if (strcmp(deviceSettingName, "logRawVoltage") == 0)
            nodeSetting->logRawVoltage = d;
          else if (strcmp(deviceSettingName, "useFourWireMode") == 0)
            nodeSetting->useFourWireMode = d;
          else if (strcmp(deviceSettingName, "useThreeWireMode") == 0)
            nodeSetting->useThreeWireMode = d;
          else if (strcmp(deviceSettingName, "useTwoWireMode") == 0)
            nodeSetting->useTwoWireMode = d;
          else if (strcmp(deviceSettingName, "useFourWireHighTemperatureMode") == 0)
            nodeSetting->useFourWireHighTemperatureMode = d;
          else if (strcmp(deviceSettingName, "useThreeWireHighTemperatureMode") == 0)
            nodeSetting->useThreeWireHighTemperatureMode = d;
          else if (strcmp(deviceSettingName, "useTwoWireHighTemperatureMode") == 0)
            nodeSetting->useTwoWireHighTemperatureMode = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PRESSURE_MPR0025PA1:
        {
          struct_MPR0025PA1 *nodeSetting = (struct_MPR0025PA1 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "minimumPSI") == 0)
            nodeSetting->minimumPSI = d;
          else if (strcmp(deviceSettingName, "maximumPSI") == 0)
            nodeSetting->maximumPSI = d;
          else if (strcmp(deviceSettingName, "usePSI") == 0)
            nodeSetting->usePSI = d;
          else if (strcmp(deviceSettingName, "usePA") == 0)
            nodeSetting->usePA = d;
          else if (strcmp(deviceSettingName, "useKPA") == 0)
            nodeSetting->useKPA = d;
          else if (strcmp(deviceSettingName, "useTORR") == 0)
            nodeSetting->useTORR = d;
          else if (strcmp(deviceSettingName, "useINHG") == 0)
            nodeSetting->useINHG = d;
          else if (strcmp(deviceSettingName, "useATM") == 0)
            nodeSetting->useATM = d;
          else if (strcmp(deviceSettingName, "useBAR") == 0)
            nodeSetting->useBAR = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PARTICLE_SNGCJA5:
        {
          struct_SNGCJA5 *nodeSetting = (struct_SNGCJA5 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logPM1") == 0)
            nodeSetting->logPM1 = d;
          else if (strcmp(deviceSettingName, "logPM25") == 0)
            nodeSetting->logPM25 = d;
          else if (strcmp(deviceSettingName, "logPM10") == 0)
            nodeSetting->logPM10 = d;
          else if (strcmp(deviceSettingName, "logPC05") == 0)
            nodeSetting->logPC05 = d;
          else if (strcmp(deviceSettingName, "logPC1") == 0)
            nodeSetting->logPC1 = d;
          else if (strcmp(deviceSettingName, "logPC25") == 0)
            nodeSetting->logPC25 = d;
          else if (strcmp(deviceSettingName, "logPC50") == 0)
            nodeSetting->logPC50 = d;
          else if (strcmp(deviceSettingName, "logPC75") == 0)
            nodeSetting->logPC75 = d;
          else if (strcmp(deviceSettingName, "logPC10") == 0)
            nodeSetting->logPC10 = d;
          else if (strcmp(deviceSettingName, "logSensorStatus") == 0)
            nodeSetting->logSensorStatus = d;
          else if (strcmp(deviceSettingName, "logPDStatus") == 0)
            nodeSetting->logPDStatus = d;
          else if (strcmp(deviceSettingName, "logLDStatus") == 0)
            nodeSetting->logLDStatus = d;
          else if (strcmp(deviceSettingName, "logFanStatus") == 0)
            nodeSetting->logFanStatus = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_VOC_SGP40:
        {
          struct_SGP40 *nodeSetting = (struct_SGP40 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logVOC") == 0)
            nodeSetting->logVOC = d;
          else if (strcmp(deviceSettingName, "RH") == 0)
            nodeSetting->RH = d;
          else if (strcmp(deviceSettingName, "T") == 0)
            nodeSetting->T = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PRESSURE_SDP3X:
        {
          struct_SDP3X *nodeSetting = (struct_SDP3X *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logPressure") == 0)
            nodeSetting->logPressure = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else if (strcmp(deviceSettingName, "massFlow") == 0)
            nodeSetting->massFlow = d;
          else if (strcmp(deviceSettingName, "averaging") == 0)
            nodeSetting->averaging = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      case DEVICE_PRESSURE_MS5837:
        {
          struct_MS5837 *nodeSetting = (struct_MS5837 *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logPressure") == 0)
            nodeSetting->logPressure = d;
          else if (strcmp(deviceSettingName, "logTemperature") == 0)
            nodeSetting->logTemperature = d;
          else if (strcmp(deviceSettingName, "logDepth") == 0)
            nodeSetting->logDepth = d;
          else if (strcmp(deviceSettingName, "logAltitude") == 0)
            nodeSetting->logAltitude = d;
          else if (strcmp(deviceSettingName, "model") == 0)
            nodeSetting->model = d;
          else if (strcmp(deviceSettingName, "fluidDensity") == 0)
            nodeSetting->fluidDensity = d;
          else if (strcmp(deviceSettingName, "conversion") == 0)
            nodeSetting->conversion = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
//      case DEVICE_QWIIC_BUTTON:
//        {
//          struct_QWIIC_BUTTON *nodeSetting = (struct_QWIIC_BUTTON *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
//          if (strcmp(deviceSettingName, "log") == 0)
//            nodeSetting->log = d;
//          else if (strcmp(deviceSettingName, "logPressed") == 0)
//            nodeSetting->logPressed = d;
//          else if (strcmp(deviceSettingName, "logClicked") == 0)
//            nodeSetting->logClicked = d;
//          else if (strcmp(deviceSettingName, "toggleLEDOnClick") == 0)
//            nodeSetting->toggleLEDOnClick = d;
//          else if (strcmp(deviceSettingName, "ledBrightness") == 0)
//            nodeSetting->ledBrightness = d;
//          else
//            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
//        }
//        break;
      case DEVICE_BIO_SENSOR_HUB:
        {
          struct_BIO_SENSOR_HUB *nodeSetting = (struct_BIO_SENSOR_HUB *)deviceConfigPtr; //Create a local pointer that points to same spot as node does
          if (strcmp(deviceSettingName, "log") == 0)
            nodeSetting->log = d;
          else if (strcmp(deviceSettingName, "logHeartrate") == 0)
            nodeSetting->logHeartrate = d;
          else if (strcmp(deviceSettingName, "logConfidence") == 0)
            nodeSetting->logConfidence = d;
          else if (strcmp(deviceSettingName, "logOxygen") == 0)
            nodeSetting->logOxygen = d;
          else if (strcmp(deviceSettingName, "logStatus") == 0)
            nodeSetting->logStatus = d;
          else if (strcmp(deviceSettingName, "logExtendedStatus") == 0)
            nodeSetting->logExtendedStatus = d;
          else if (strcmp(deviceSettingName, "logRValue") == 0)
            nodeSetting->logRValue = d;
          else
            SerialPrintf2("Unknown device setting: %s\r\n", deviceSettingName);
        }
        break;
      default:
        SerialPrintf2("Unknown device type: %d\r\n", deviceType);
        SerialFlush();
        break;
    }
  }
  return (true);
}
