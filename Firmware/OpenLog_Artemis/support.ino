void printDebug(String thingToPrint)
{
  if(settings.printDebugMessages == true)
  {
    Serial.print(thingToPrint);
    if (settings.useTxRxPinsForTerminal == true)
      SerialLog.print(thingToPrint);
  }
}


//Option not known
void printUnknown(uint8_t unknownChoice)
{
  SerialPrint(F("Unknown choice: "));
  Serial.write(unknownChoice);
  if (settings.useTxRxPinsForTerminal == true)
      SerialLog.write(unknownChoice);
  SerialPrintln(F(""));
}
void printUnknown(int unknownValue)
{
  SerialPrint(F("Unknown value: "));
  Serial.write(unknownValue);
  if (settings.useTxRxPinsForTerminal == true)
      SerialLog.write(unknownValue);
  SerialPrintln(F(""));
}

//Blocking wait for user input
void waitForInput()
{
  for (int i = 0; i < 10; i++) //Wait for any incoming chars to hit buffer
  {
    checkBattery();
    delay(1);
  }
  
  while (Serial.available() > 0) Serial.read(); //Clear buffer
  
  if (settings.useTxRxPinsForTerminal == true)
    while (SerialLog.available() > 0) SerialLog.read(); //Clear buffer

  bool keepChecking = true;    
  while (keepChecking)
  {
    if (Serial.available())
      keepChecking = false;
      
    if (settings.useTxRxPinsForTerminal == true)
    {
      if (SerialLog.available())
        keepChecking = false;
    }
    
    checkBattery();
  }
}

//Get single byte from user
//Waits for and returns the character that the user provides
//Returns STATUS_GETNUMBER_TIMEOUT if input times out
//Returns 'x' if user presses 'x'
uint8_t getByteChoice(int numberOfSeconds, bool updateDZSERIAL)
{
  SerialFlush();

  for (int i = 0; i < 50; i++) //Wait for any incoming chars to hit buffer
  {
    checkBattery();
    delay(1);
  }
  
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  if (settings.useTxRxPinsForTerminal == true)
    while (SerialLog.available() > 0) SerialLog.read(); //Clear buffer

  long startTime = millis();
  byte incoming;
  while (1)
  {
    if (Serial.available() > 0)
    {
      incoming = Serial.read();
      if (updateDZSERIAL)
      {
        DSERIAL = &Serial;
        ZSERIAL = &Serial;
      }
//      SerialPrint(F("byte: 0x"));
//      Serial.println(incoming, HEX);
//      if (settings.useTxRxPinsForTerminal == true)
//        SerialLog.println(incoming, HEX);
      if (incoming >= 'a' && incoming <= 'z') break;
      if (incoming >= 'A' && incoming <= 'Z') break;
      if (incoming >= '0' && incoming <= '9') break;
    }

    if ((settings.useTxRxPinsForTerminal == true) && (SerialLog.available() > 0))
    {
      incoming = SerialLog.read();
      if (updateDZSERIAL)
      {
        DSERIAL = &SerialLog;
        ZSERIAL = &SerialLog;
      }
//      SerialPrint(F("byte: 0x"));
//      Serial.println(incoming, HEX);
//      if (settings.useTxRxPinsForTerminal == true)
//        SerialLog.println(incoming, HEX);
      if (incoming >= 'a' && incoming <= 'z') break;
      if (incoming >= 'A' && incoming <= 'Z') break;
      if (incoming >= '0' && incoming <= '9') break;
    }

    if ( (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      SerialPrintln(F("No user input received."));
      return (STATUS_GETBYTE_TIMEOUT); //Timeout. No user input.
    }

    checkBattery();
    delay(1);
  }

  return (incoming);
}

//Get a string/value from user, remove all non-numeric values
//Returns STATUS_GETNUMBER_TIMEOUT if input times out
//Returns STATUS_PRESSED_X if user presses 'x'
int64_t getNumber(int numberOfSeconds)
{
  for (int i = 0; i < 10; i++) //Wait for any incoming chars to hit buffer
  {
    checkBattery();
    delay(1);
  }
  
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  if (settings.useTxRxPinsForTerminal == true)
    while (SerialLog.available() > 0) SerialLog.read(); //Clear buffer

  //Get input from user
  char cleansed[20]; //Good for very large numbers: 123,456,789,012,345,678\0

  long startTime = millis();
  int spot = 0;
  while (spot < 20 - 1) //Leave room for terminating \0
  {
    bool serialAvailable = false;
    while (serialAvailable == false) //Wait for user input
    {
      if (Serial.available())
        serialAvailable = true;

      if ((settings.useTxRxPinsForTerminal == true) && (SerialLog.available()))
        serialAvailable = true;
        
      checkBattery();
      
      if ( (millis() - startTime) / 1000 >= numberOfSeconds)
      {
        if (spot == 0)
        {
          SerialPrintln(F("No user input received. Do you have line endings turned on?"));
          return (STATUS_GETNUMBER_TIMEOUT); //Timeout. No user input.
        }
        else if (spot > 0)
        {
          break; //Timeout, but we have data
        }
      }
    }

    //See if we timed out waiting for a line ending
    if (spot > 0 && (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      SerialPrintln(F("Do you have line endings turned on?"));
      break; //Timeout, but we have data
    }

    byte incoming;

    if (Serial.available())
      incoming = Serial.read();
    
    else
      incoming = SerialLog.read();

    if (incoming == '\n' || incoming == '\r')
    {
      SerialPrintln(F(""));
      break;
    }

    if ((isDigit(incoming) == true) || ((incoming == '-') && (spot == 0))) // Check for digits and a minus sign
    {
      Serial.write(incoming); //Echo user's typing

      if (settings.useTxRxPinsForTerminal == true)
        SerialLog.write(incoming); //Echo user's typing
      
      cleansed[spot++] = (char)incoming;
    }

    if (incoming == 'x')
    {
      return (STATUS_PRESSED_X);
    }
  }

  cleansed[spot] = '\0';

  int64_t largeNumber = 0;
  int x = 0;
  if (cleansed[0] == '-') // If our number is negative
  {
    x = 1; // Skip the minus
  }
  for( ; x < spot ; x++)
  {
    largeNumber *= 10;
    largeNumber += (cleansed[x] - '0');
  }
  if (cleansed[0] == '-') // If our number is negative
  {
    largeNumber = 0 - largeNumber; // Make it negative
  }
  return (largeNumber);
}

//Get a string/value from user, remove all non-numeric values
//Returns STATUS_GETNUMBER_TIMEOUT if input times out
//Returns STATUS_PRESSED_X if user presses 'x'
double getDouble(int numberOfSeconds)
{
  for (int i = 0; i < 10; i++) //Wait for any incoming chars to hit buffer
  {
    checkBattery();
    delay(1);
  }
  
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  if (settings.useTxRxPinsForTerminal == true)
    while (SerialLog.available() > 0) SerialLog.read(); //Clear buffer

  //Get input from user
  char cleansed[20]; //Good for very large numbers: 123,456,789,012,345,678\0

  long startTime = millis();
  int spot = 0;
  bool dpSeen = false;
  while (spot < 20 - 1) //Leave room for terminating \0
  {
    bool serialAvailable = false;
    while (serialAvailable == false) //Wait for user input
    {
      if (Serial.available())
        serialAvailable = true;

      if ((settings.useTxRxPinsForTerminal == true) && (SerialLog.available()))
        serialAvailable = true;

      checkBattery();
      
      if ( (millis() - startTime) / 1000 >= numberOfSeconds)
      {
        if (spot == 0)
        {
          SerialPrintln(F("No user input received. Do you have line endings turned on?"));
          return (STATUS_GETNUMBER_TIMEOUT); //Timeout. No user input.
        }
        else if (spot > 0)
        {
          break; //Timeout, but we have data
        }
      }
    }

    //See if we timed out waiting for a line ending
    if (spot > 0 && (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      SerialPrintln(F("Do you have line endings turned on?"));
      break; //Timeout, but we have data
    }

    byte incoming;

    if (Serial.available())
      incoming = Serial.read();
    
    else
      incoming = SerialLog.read();

    if (incoming == '\n' || incoming == '\r')
    {
      SerialPrintln(F(""));
      break;
    }

    if ((isDigit(incoming) == true) || ((incoming == '-') && (spot == 0)) || ((incoming == '.') && (dpSeen == false))) // Check for digits/minus/dp
    {
      Serial.write(incoming); //Echo user's typing
      
      if (settings.useTxRxPinsForTerminal == true)
        SerialLog.write(incoming); //Echo user's typing
      
      cleansed[spot++] = (char)incoming;
    }

    if (incoming == '.')
      dpSeen = true;

    if (incoming == 'x')
    {
      return (STATUS_PRESSED_X);
    }
  }

  cleansed[spot] = '\0';

  double largeNumber = 0;
  int x = 0;
  if (cleansed[0] == '-') // If our number is negative
  {
    x = 1; // Skip the minus
  }
  for( ; x < spot ; x++)
  {
    if (cleansed[x] == '.')
      break;
    largeNumber *= 10;
    largeNumber += (cleansed[x] - '0');
  }
  if (x < spot) // Check if we found a '.'
  {
    x++;
    double divider = 0.1;
    for( ; x < spot ; x++)
    {
      largeNumber += (cleansed[x] - '0') * divider;
      divider /= 10;
    }
  }
  if (cleansed[0] == '-') // If our number is negative
  {
    largeNumber = 0 - largeNumber; // Make it negative
  }
  return (largeNumber);
}
