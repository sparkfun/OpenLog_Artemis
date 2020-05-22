void printMajorDebug(String thingToPrint)
{
  if(settings.printMajorDebugMessages == true)
  {
    Serial.print(thingToPrint);    
  }
}

void printMinorDebug(String thingToPrint)
{
  if(settings.printMinorDebugMessages == true)
  {
    Serial.print(thingToPrint);    
  }
}

//Option not known
void printUnknown(uint8_t unknownChoice)
{
  Serial.print(F("Unknown choice: "));
  Serial.print(unknownChoice);
  Serial.println();
}
void printUnknown(int unknownValue)
{
  Serial.print(F("Unknown value: "));
  Serial.print(unknownValue);
  Serial.println();
}

//Blocking wait for user input
void waitForInput()
{
  delay(5); //Wait for any incoming chars to hit buffer (let's keep this short!)
  while (Serial.available() > 0) Serial.read(); //Clear buffer
  while (Serial.available() == 0)
  {
    storeData(); //Keep reading I2C data and writing it to SD
  }
}

//Get single byte from user
//Waits for and returns the character that the user provides
//Returns STATUS_GETNUMBER_TIMEOUT if input times out
//Returns 'x' if user presses 'x'
uint8_t getByteChoice(int numberOfSeconds)
{
  bool termOut = settings.enableTerminalOutput; //Store settings.enableTerminalOutput
  settings.enableTerminalOutput = false; //Disable terminal messages while waiting for a choice
  
  Serial.flush();
  delay(5); //Wait for any incoming chars to hit buffer (let's keep this short!)
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  long startTime = millis();
  byte incoming;
  while (1)
  {
    if (Serial.available() > 0)
    {
      incoming = Serial.read();
//      Serial.print(F("byte: 0x"));
//      Serial.println(incoming, HEX);
      if (incoming >= 'a' && incoming <= 'z') break;
      if (incoming >= 'A' && incoming <= 'Z') break;
      if (incoming >= '0' && incoming <= '9') break;
    }

    if ( (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      Serial.println(F("No user input received."));
      settings.enableTerminalOutput = termOut; //Restore settings.enableTerminalOutput
      return (STATUS_GETBYTE_TIMEOUT); //Timeout. No user input.
    }

    storeData(); //Keep reading I2C data and writing it to SD

    delay(1);
  }

  settings.enableTerminalOutput = termOut; //Restore settings.enableTerminalOutput
  return (incoming);
}

//Get a string/value from user, remove all non-numeric values
//Returns STATUS_GETNUMBER_TIMEOUT if input times out
//Returns STATUS_PRESSED_X if user presses 'x'
int64_t getNumber(int numberOfSeconds)
{
  bool termOut = settings.enableTerminalOutput; //Store settings.enableTerminalOutput
  settings.enableTerminalOutput = false; //Disable terminal messages while waiting for a number
  
  delay(5); //Wait for any incoming chars to hit buffer (let's keep this short!)
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  //Get input from user
  char cleansed[20]; //Good for very large numbers: 123,456,789,012,345,678\0

  long startTime = millis();
  int spot = 0;
  while (spot < 20 - 1) //Leave room for terminating \0
  {
    while (Serial.available() == 0) //Wait for user input
    {
      if ( (millis() - startTime) / 1000 >= numberOfSeconds)
      {
        if (spot == 0)
        {
          Serial.println(F("No user input received. Do you have line endings turned on?"));
          settings.enableTerminalOutput = termOut; //Restore settings.enableTerminalOutput
          return (STATUS_GETNUMBER_TIMEOUT); //Timeout. No user input.
        }
        else if (spot > 0)
        {
          break; //Timeout, but we have data
        }
      }

      storeData(); //Keep reading I2C data and writing it to SD
    }

    //See if we timed out waiting for a line ending
    if (spot > 0 && (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      Serial.println(F("Do you have line endings turned on?"));
      break; //Timeout, but we have data
    }

    byte incoming = Serial.read();
    if (incoming == '\n' || incoming == '\r')
    {
      Serial.println();
      break;
    }

    if (isDigit(incoming) == true)
    {
      Serial.write(incoming); //Echo user's typing
      cleansed[spot++] = (char)incoming;
    }

    if (incoming == 'x')
    {
      settings.enableTerminalOutput = termOut; //Restore settings.enableTerminalOutput
      return (STATUS_PRESSED_X);
    }
  }

  cleansed[spot] = '\0';

  uint64_t largeNumber = 0;
  for(int x = 0 ; x < spot ; x++)
  {
    largeNumber *= 10;
    largeNumber += (cleansed[x] - '0');
  }

  settings.enableTerminalOutput = termOut; //Restore settings.enableTerminalOutput
  return (largeNumber);
}
