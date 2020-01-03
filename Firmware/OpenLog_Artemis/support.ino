
//Option not known
void printUnknown(uint8_t unknownChoice)
{
  Serial.print("Unknown choice: ");
  Serial.write(unknownChoice);
  Serial.println();
}
void printUnknown(int unknownValue)
{
  Serial.print("Unknown value: ");
  Serial.write(unknownValue);
  Serial.println();
}

//Blocking wait for user input
void waitForInput()
{
  delay(10); //Wait for any incoming chars to hit buffer
  while (Serial.available() > 0) Serial.read(); //Clear buffer
  while (Serial.available() == 0);
}

//Get single byte from user
//Waits for and returns the character that the user provides
//Returns 255 if user does not respond in a certain amount time
byte getByteChoice(int numberOfSeconds)
{
  delay(10); //Wait for any incoming chars to hit buffer
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  long startTime = millis();
  while (Serial.available() == 0)
  {
    if ( (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      Serial.println("No user input recieved.");
      return (255); //Timeout. No user input.
    }

    delay(10);
  }

  return (Serial.read());
}

//Get a string/value from user, remove all non-numeric values
//Returns 255 is user presses 'x' or input times out
int getNumber(int numberOfSeconds)
{
  delay(10); //Wait for any incoming chars to hit buffer
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  //Get input from user
  char cleansed[20];

  long startTime = millis();
  int spot = 0;
  while (spot < 20 - 1) //Leave room for terminating \0
  {
    while (Serial.available() == 0) //Wait for user input
    {
      if ( (millis() - startTime) / 1000 >= numberOfSeconds)
      {
        Serial.println("No user input recieved.");
        return (255); //Timeout. No user input.
      }
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

    if (spot == 0 && incoming == 'x')
    {
      return (255);
    }
  }

  cleansed[spot] = '\0';

  String tempValue = cleansed;
  return (tempValue.toInt());
}
