void loadSettings()
{
  //EEPROM.erase();

  //Check to see if EEPROM is blank
  uint32_t testRead = 0;
  if (EEPROM.get(0, testRead) == 0xFFFFFFFF)
  {
    //At power on, settings are in default state
    //Record default settings
    recordSettings();

    //Serial.println("Default settings applied");
  }

  //Read current settings
  EEPROM.get(0, settings);

  //Check settings for sanity
  if (settings.recordPerSecond > 10000)
    settings.recordPerSecond = 2;

  recordSettings();

  //Serial.print("Size of settings: ");
  //Serial.println(sizeof(settings));
}

void recordSettings()
{
  EEPROM.put(0, settings);
}
