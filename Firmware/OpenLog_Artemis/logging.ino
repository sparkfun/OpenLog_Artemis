//Print a message both to terminal and to log
void msg(const char * message)
{
  Serial.println(message);
  if (online.microSD)
    sensorDataFile.println(message);
}

//Returns next available log file name
//Checks the spots in EEPROM for the next available LOG# file name
//Updates EEPROM and then appends to the new log file.
char* findNextAvailableLog(int &newFileNumber, const char *fileLeader)
{
  File newFile; //This will contain the file for SD writing

  if (newFileNumber > 0)
    newFileNumber--; //Check if last log file was empty

  //Search for next available log spot
  static char newFileName[40];
  while (1)
  {
    sprintf(newFileName, "%s%05u.TXT", fileLeader, newFileNumber); //Splice the new file number into this file name

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
  recordSettings(); //Record new file number to EEPROM and to config file

  Serial.print(F("Created log file: "));
  Serial.println(newFileName);

  return (newFileName);
}
