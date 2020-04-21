//Returns next available log file name
//Checks the spots in EEPROM for the next available LOG# file name
//Updates EEPROM and then appends to the new log file.
char* findNextAvailableLog(int &newFileNumber, const char *fileLeader)
{
  File newFile; //This will contain the file for SD writing

  Serial.println("Next avail");
  Serial.flush();

  if (newFileNumber > 0)
    newFileNumber--; //Check if last log file was empty

  //Search for next available log spot
  static char newFileName[40];
  while (1)
  {
    Serial.println("1");
    Serial.flush();

    sprintf(newFileName, "%s%05u.TXT", fileLeader, newFileNumber); //Splice the new file number into this file name

    Serial.println("2");
    Serial.flush();

    Serial.println(newFileName);
    Serial.flush();

    if (sd.exists(newFileName) == false) break; //File name not found so we will use it.

    Serial.println("3");
    Serial.flush();

    //File exists so open and see if it is empty. If so, use it.
    newFile = sd.open(newFileName, O_READ);
    //newFile.open(newFileName, O_READ); //exFat
    if (newFile.size() == 0) break; // File is empty so we will use it.

    Serial.println("4");
    Serial.flush();

    newFile.close(); // Close this existing file we just opened.

    newFileNumber++; //Try the next number
  }
    Serial.println("5");
    Serial.flush();
  newFile.close(); //Close this new file we just opened

    Serial.println("6");
    Serial.flush();
  newFileNumber++; //Increment so the next power up uses the next file #
  //recordSettings(); //Record new file number to EEPROM and to config file

  Serial.print(F("Created log file: "));
  Serial.println(newFileName);
  Serial.flush();

  return (newFileName);
}

void beginDataLogging()
{
  int fileNumber = 10;

  //If we don't have a file yet, create one. Otherwise, re-open the last used file
  if (strlen(sensorDataFileName) == 0)
    strcpy(sensorDataFileName, findNextAvailableLog(fileNumber, "dataLog"));

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (sensorDataFile.open(sensorDataFileName, O_CREAT | O_APPEND | O_WRITE) == false)
  {
    Serial.println("Failed to create sensor data file");
    return;
  }

  Serial.println("Sensor data file open");
  Serial.flush();
}

void beginSD()
{
  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected

  microSDPowerOn();

  //Max power up time is 250ms: https://www.kingston.com/datasheets/SDCIT-specsheet-64gb_en.pdf
  //Max current is 200mA average across 1s, peak 300mA
  delay(10);

  if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
  {
    delay(250); //Give SD more time to power up, then try again
    if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
    {
      Serial.println("SD init failed. Do you have the correct board selected in Arduino? Is card present? Formatted?");
      digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected
      return;
    }
  }
  //    }

  //Change to root directory. All new file creation will be in root.
  if (sd.chdir() == false)
  {
    Serial.println("SD change directory failed");
    return;
  }

  Serial.println("SD begin complete");
  Serial.flush();
}

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  digitalWrite(PIN_QWIIC_POWER, LOW);
}
void qwiicPowerOff()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  digitalWrite(PIN_QWIIC_POWER, HIGH);
}

void microSDPowerOn()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, LOW);
}
void microSDPowerOff()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, HIGH);
}

void imuPowerOn()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, HIGH);
}
void imuPowerOff()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, LOW);
}
