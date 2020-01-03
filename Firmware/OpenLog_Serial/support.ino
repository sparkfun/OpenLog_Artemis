

//Handle errors by printing the error type and blinking LEDs in certain way
//The function will never exit - it loops forever inside blinkError
void systemError(byte errorType)
{
  Serial.print(F("Error "));
  switch (errorType)
  {
    case ERROR_CARD_INIT:
      Serial.print(F("card.init"));
      blinkError(ERROR_SD_INIT);
      break;
    case ERROR_VOLUME_INIT:
      Serial.print(F("volume.init"));
      blinkError(ERROR_SD_INIT);
      break;
    case ERROR_ROOT_INIT:
      Serial.print(F("root.init"));
      blinkError(ERROR_SD_INIT);
      break;
    case ERROR_FILE_OPEN:
      Serial.print(F("file.open"));
      blinkError(ERROR_SD_INIT);
      break;
  }
}

//Blinks the status LEDs to indicate a type of error
void blinkError(byte ERROR_TYPE) {
  while (1) {
    for (int x = 0 ; x < ERROR_TYPE ; x++) {
      digitalWrite(statLED, HIGH);
      delay(200);
      digitalWrite(statLED, LOW);
      delay(200);
    }

    delay(2000);
  }
}

//Print a message both to terminal and to log
void msg(const char * message)
{
  Serial.println(message);
  workingFile.println(message);
}
