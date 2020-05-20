//storeData is the workhorse. It reads I2C data and writes it to SD in 512 byte blocks.
//The I2C code comes directly from checkUbloxI2C in the u-blox library
//TO DO: timeout after a suitable interval and write any remaining bytes left in the buffer to SD.
bool storeData(void)
{
  //Check for new I2C data three times faster than usBetweenReadings to avoid pounding the I2C bus
  if ((micros() - lastReadTime) < (settings.usBetweenReadings / 3))
    return (false); //It's not yet time to read the I2C bus

  // **** Start of code taken from checkUbloxI2C ****

  //Get the number of bytes available from the module
  uint16_t bytesAvailable = 0;
  qwiic.beginTransmission(settings.sensor_uBlox.ubloxI2Caddress);
  qwiic.write(0xFD);                     //0xFD (MSB) and 0xFE (LSB) are the registers that contain number of bytes available
  if (qwiic.endTransmission(false) != 0) //Send a restart command. Do not release bus.
    return (false);                          //Sensor did not ACK

  qwiic.requestFrom(settings.sensor_uBlox.ubloxI2Caddress, (uint8_t)2);
  if (qwiic.available())
  {
    uint8_t msb = qwiic.read();
    uint8_t lsb = qwiic.read();
    if (lsb == 0xFF)
    {
      //I believe this is a Ublox bug. Device should never present an 0xFF.
      if (settings.printDebugMessages == true)
      {
        Serial.println(F("checkUbloxI2C: Ublox bug, length lsb is 0xFF"));
      }
//      if (checksumFailurePin >= 0)
//      {
//        digitalWrite((uint8_t)checksumFailurePin, LOW);
//        delay(10);
//        digitalWrite((uint8_t)checksumFailurePin, HIGH);
//      }
      lastReadTime = micros(); //Put off checking to avoid I2C bus traffic
      return (false);
    }
    bytesAvailable = (uint16_t)msb << 8 | lsb;
  }

  if (bytesAvailable == 0)
  {
    if (settings.printDebugMessages == true)
    {
      Serial.println(F("checkUbloxI2C: OK, zero bytes available"));
    }
    lastReadTime = micros(); //Put off checking to avoid I2C bus traffic
    return (false);
  }

  //Check for undocumented bit error. We found this doing logic scans.
  //This error is rare but if we incorrectly interpret the first bit of the two 'data available' bytes as 1
  //then we have far too many bytes to check. May be related to I2C setup time violations: https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library/issues/40
  if (bytesAvailable & ((uint16_t)1 << 15))
  {
    //Clear the MSbit
    bytesAvailable &= ~((uint16_t)1 << 15);

    if (settings.printDebugMessages == true)
    {
      Serial.print(F("checkUbloxI2C: Bytes available error:"));
      Serial.println(bytesAvailable);
//      if (checksumFailurePin >= 0)
//      {
//        digitalWrite((uint8_t)checksumFailurePin, LOW);
//        delay(10);
//        digitalWrite((uint8_t)checksumFailurePin, HIGH);
//      }
    }
  }

  if (bytesAvailable > 100)
  {
    if (settings.printDebugMessages == true)
    {
      Serial.print(F("checkUbloxI2C: Large packet of "));
      Serial.print(bytesAvailable);
      Serial.println(F(" bytes received"));
    }
  }
  else
  {
    if (settings.printDebugMessages == true)
    {
      Serial.print(F("checkUbloxI2C: Reading "));
      Serial.print(bytesAvailable);
      Serial.println(F(" bytes"));
    }
  }

  while (bytesAvailable)
  {
    qwiic.beginTransmission(settings.sensor_uBlox.ubloxI2Caddress);
    qwiic.write(0xFF);                     //0xFF is the register to read data from
    if (qwiic.endTransmission(false) != 0) //Send a restart command. Do not release bus.
      return (false);                          //Sensor did not ACK

    //Limit to 32 bytes or whatever the buffer limit is for given platform
    uint16_t bytesToRead = bytesAvailable;
    if (bytesToRead > I2C_BUFFER_LENGTH)
      bytesToRead = I2C_BUFFER_LENGTH;

  TRY_AGAIN:

    qwiic.requestFrom(settings.sensor_uBlox.ubloxI2Caddress, (uint8_t)bytesToRead);
    if (qwiic.available())
    {
      for (uint16_t x = 0; x < bytesToRead; x++)
      {
        uint8_t incoming = qwiic.read(); //Grab the actual character

        //Check to see if the first read is 0x7F. If it is, the module is not ready
        //to respond. Stop, wait, and try again
        if (x == 0)
        {
          if (incoming == 0x7F)
          {
            if (settings.printDebugMessages == true)
            {
              Serial.println(F("checkUbloxU2C: Ublox error, module not ready with data"));
            }
            delay(5); //In logic analyzation, the module starting responding after 1.48ms
//            if (checksumFailurePin >= 0)
//            {
//              digitalWrite((uint8_t)checksumFailurePin, LOW);
//              delay(10);
//              digitalWrite((uint8_t)checksumFailurePin, HIGH);
//            }
            goto TRY_AGAIN;
          }
        }

        //TO DO: do something with incoming
      }
    }
    else
      return (false); //Sensor did not respond

    bytesAvailable -= bytesToRead;
  }

  // **** End of code taken from checkUbloxI2C ****

  return(true);
}
