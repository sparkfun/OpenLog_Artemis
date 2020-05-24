//Returns next available log file name
//Checks the spots in EEPROM for the next available LOG# file name
//Updates EEPROM and then appends to the new log file.
char* findNextAvailableLog(int &newFileNumber, const char *fileLeader)
{
  File newFile; //This will contain the file for SD writing

  if (newFileNumber < 2) //If the settings have been reset, let's warn the user that this could take a while!
  {
    Serial.println(F("Finding the next available log file."));
    Serial.println(F("This could take a long time if the SD card contains many existing log files."));
  }

  if (newFileNumber > 0)
    newFileNumber--; //Check if last log file was empty

  //Search for next available log spot
  static char newFileName[40];
  while (1)
  {
    sprintf(newFileName, "%s%05u.ubx", fileLeader, newFileNumber); //Splice the new file number into this file name

//    if (settings.printMinorDebugMessages == true)
//    {
//      Serial.print(F("findNextAvailableLog: trying "));
//      Serial.println(newFileName);
//    }

    if (sd.exists(newFileName) == false) break; //File name not found so we will use it.

    //File exists so open and see if it is empty. If so, use it.
    //newFile = sd.open(newFileName, O_READ);
    newFile.open(newFileName, O_READ); //exFat
    if (newFile.size() == 0) break; // File is empty so we will use it.

    newFile.close(); // Close this existing file we just opened.

    newFileNumber++; //Try the next number
  }
  newFile.close(); //Close this new file we just opened

  newFileNumber++; //Increment so the next power up uses the next file #
  recordSettings(); //Record new file number to EEPROM and to config file (newFileNumber points to settings.nextDataLogNumber)

  Serial.print(F("Created log file: "));
  Serial.println(newFileName);

  return (newFileName);
}
