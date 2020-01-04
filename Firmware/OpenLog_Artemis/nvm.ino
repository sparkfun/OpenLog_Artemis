void loadSettings()
{
  //EEPROM.erase();

  //Check to see if EEPROM is blank
  uint32_t testRead = 0;
  if (EEPROM.get(0, testRead) == 0xFFFFFFFF)
  {
    recordSettings(); //Record default settings. At power on, settings are in default state

    //Serial.println("Default settings applied");
  }

  //Check that the current settings struct size matches what is stored in EEPROM
  //Misalignment happens when we add a new feature or setting
  int tempSize = 0;
  EEPROM.get(0, tempSize); //Load the sizeOfSettings
  if(tempSize != sizeof(settings))
  {
    //Serial.println("Settings wrong size. Default settings applied");
    recordSettings(); //Record default settings. At power on, settings are in default state
  }

  //Read current settings
  EEPROM.get(0, settings);
}

void recordSettings()
{
  settings.sizeOfSettings = sizeof(settings);
  EEPROM.put(0, settings);
}

//Export the current settings to a CSV output
void recordSettingsToFile()
{
  if (online.microSD == true)
  {
    if (sd.exists("OLA_settings.cfg"))
      sd.remove("OLA_settings.cfg");

    if (settingsFile.open("OLA_settings.cfg", O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println("Failed to create settings file");
      return;
    }
    
    settingsFile.println("OpenLogAretmisSettings:");
    settingsFile.println("sizeOfSettings=" + (String)settings.sizeOfSettings + ",");
    settingsFile.println("nextSerialLogNumber=" + (String)settings.nextSerialLogNumber + ",");
    settingsFile.println("nextDataLogNumber=" + (String)settings.nextDataLogNumber + ",");
    settingsFile.println("recordPerSecond=" + (String)settings.recordPerSecond + ",");
    settingsFile.println("logMaxRate=" + (String)settings.logMaxRate + ",");
    settingsFile.println("enableRTC=" + (String)settings.enableRTC + ",");
    settingsFile.println("enableIMU=" + (String)settings.enableIMU + ",");
    
    //settingsFile.sync();
    settingsFile.close();
  }
}
