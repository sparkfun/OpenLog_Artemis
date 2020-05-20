//Data is read from I2C into GNSSbuffer
//It is read from GNSSbuffer and put into UBXbuffer until we know what type of frame it is
//If it is not an ACK/NACK, it is then moved from UBXbuffer into SDpacket and written to SD card

//Storage for the incoming I2C data
RingBufferN<32768> GNSSbuffer;

//Storage for a single UBX frame (so we can discard ACK/NACKs)
const size_t UBXbufferSize = 4096; //Needs to be large enough to hold the largest RAWX frame (8 + 16 + (32 * numMeas))
char UBXbuffer[UBXbufferSize];
size_t UBXpointer = 0;

//Storage for the SD packet
//Define packet size, buffer and buffer pointer for SD card writes
const size_t SDpacket = 512;
uint8_t SDbuffer[SDpacket];
size_t SDpointer = 0;

int maxGNSSbufferAvailable = 0; //Use this to record how full the GNSSbuffer gets

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
  qwiic.write(0xFD); //0xFD (MSB) and 0xFE (LSB) are the registers that contain number of bytes available
  if (qwiic.endTransmission(false) != 0) //Send a restart command. Do not release bus.
    return (false); //Sensor did not ACK

  qwiic.requestFrom(settings.sensor_uBlox.ubloxI2Caddress, (uint8_t)2);
  if (qwiic.available())
  {
    uint8_t msb = qwiic.read();
    uint8_t lsb = qwiic.read();
    if (lsb == 0xFF)
    {
      //I believe this is a Ublox bug. Device should never present an 0xFF.
      if (settings.printMajorDebugMessages == true)
      {
        Serial.println(F("storeData: Ublox bug, length lsb is 0xFF"));
      }
      if (PIN_LOGIC_DEBUG >= 0)
      {
        digitalWrite((uint8_t)PIN_LOGIC_DEBUG, LOW);
        delay(1);
        digitalWrite((uint8_t)PIN_LOGIC_DEBUG, HIGH);
      }
      lastReadTime = micros(); //Put off checking to avoid I2C bus traffic
      return (false);
    }
    bytesAvailable = (uint16_t)msb << 8 | lsb;
  }

  if (bytesAvailable == 0)
  {
//    if (settings.printMinorDebugMessages == true)
//    {
//      Serial.println(F("storeData: OK, zero bytes available"));
//    }
    lastReadTime = micros(); //Put off checking to avoid I2C bus traffic
    return (false);
  }

  //Check for undocumented bit error. We found this doing logic scans.
  //This error is rare but if we incorrectly interpret the first bit of the two 'data available' bytes as 1
  //then we have far too many bytes to check.
  //May be related to I2C setup time violations: https://github.com/sparkfun/SparkFun_Ublox_Arduino_Library/issues/40
  //Or pull-ups? (PaulZC)
  if (bytesAvailable & ((uint16_t)1 << 15))
  {
    //Clear the MSbit
    bytesAvailable &= ~((uint16_t)1 << 15);

    if (settings.printMajorDebugMessages == true)
    {
      Serial.print(F("storeData: Bytes available error:"));
      Serial.println(bytesAvailable);
      if (PIN_LOGIC_DEBUG >= 0)
      {
        digitalWrite((uint8_t)PIN_LOGIC_DEBUG, LOW);
        delay(1);
        digitalWrite((uint8_t)PIN_LOGIC_DEBUG, HIGH);
      }
    }
  }

//  if (bytesAvailable > 100)
//  {
//    if (settings.printDebugMessages == true)
//    {
//      Serial.print(F("storeData: Large packet of "));
//      Serial.print(bytesAvailable);
//      Serial.println(F(" bytes received"));
//    }
//  }
//  else
//  {
//    if (settings.printDebugMessages == true)
//    {
//      Serial.print(F("storeData: Reading "));
//      Serial.print(bytesAvailable);
//      Serial.println(F(" bytes"));
//    }
//  }

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

//This code has been disabled as RAWX messages can frequently contain 0x7F as valid data!
//        //Check to see if the first read is 0x7F. If it is, the module is not ready
//        //to respond. Stop, wait, and try again
//        if (x == 0)
//        {
//          if (incoming == 0x7F)
//          {
//            if (settings.printDebugMessages == true)
//            {
//              Serial.println(F("storeData: Ublox error, module not ready with data"));
//            }
//            delay(5); //In logic analyzation, the module starting responding after 1.48ms
//            if (PIN_LOGIC_DEBUG >= 0)
//            {
//              digitalWrite((uint8_t)PIN_LOGIC_DEBUG, LOW);
//              delay(1);
//              digitalWrite((uint8_t)PIN_LOGIC_DEBUG, HIGH);
//            }
//            goto TRY_AGAIN;
//          }
//        }

        processUBX(incoming); //Process the incoming byte. This will update ubx_state
        
        //TO DO: close the current file and open a new one when ubx_state == sync_lost

        UBXbuffer[UBXpointer] = incoming; //Store incoming in the UBX buffer
        UBXpointer++; //Increment the pointer
        if (UBXpointer == UBXbufferSize) //This should never happen!
        {
          Serial.print(F("storeData: UBXbuffer overflow! Freezing..."));
          while(1)
            ;
        }
        if (ubx_state == sync_lost) //If the UBX frame was invalid or we lost sync
        {
          UBXpointer = 0; //Discard the frame by resetting the UBXpointer
          ubx_state = looking_for_B5; //Start looking for the start of a new frame
        }
        else if (ubx_state == frame_valid) //If the UBX frame is valid
        {
          if (UBXbuffer[2] != 0x05) //If the frame is not an ACK/NACK
          {
            for (size_t i = 0; i < UBXpointer; i++) //For each char in the frame
            {
              //TO DO: speed this up by doing some form of memory copy
              GNSSbuffer.store_char(UBXbuffer[i]); //Copy it into the GNSS buffer
            }
          }
          else
          {
            if (settings.printMajorDebugMessages == true)
            {
              if (UBXbuffer[3] == 0x00) //If this is a NACK
              {
                Serial.print(F("UBX NACK Class:0x"));
              }
              else if (UBXbuffer[3] == 0x01) //If this is a ACK
              {
                Serial.print(F("UBX ACK Class:0x"));
              }
              Serial.print(UBXbuffer[6], HEX);
              Serial.print(F(" ID:0x"));
              Serial.println(UBXbuffer[7], HEX);
            }
          }
          UBXpointer = 0; //Reset the UBXpointer
          ubx_state = looking_for_B5; //Start looking for the start of a new frame
        }
      }
    }
    else
      return (false); //Sensor did not respond

    bytesAvailable -= bytesToRead;
  }

  // **** End of code taken from checkUbloxI2C ****

  //Record to SD
  //Only do this if logging is enabled and the SD card is ready
  if (settings.logData && settings.enableSD && online.microSD && online.dataLogging)
  {
    int bufAvail = GNSSbuffer.available(); //Check how many bytes are in the GNSS buffer
    
    while (bufAvail > 0) //While there are more than zero bytes in the GNSS buffer
    {
      if (bufAvail > maxGNSSbufferAvailable) //Check if we have reached a new Max bufAvail
      {
        maxGNSSbufferAvailable = bufAvail; //We have - so record it
        if (settings.printMajorDebugMessages == true) //If debug messages are enabled
        {
          Serial.print(F("storeData: Max bufAvail: ")); //Print the new Max bufAvail so we can watch how full the buffer gets
          Serial.println(maxGNSSbufferAvailable);
        }
      }
      uint8_t c = GNSSbuffer.read_char(); //Read a char from the buffer
      SDbuffer[SDpointer] = c; //Store it in the SDbuffer
      SDpointer++; //Increment the SDpointer
      if (SDpointer == SDpacket) //Have we reached SDpacket (512) bytes?
      {
        SDpointer = 0; //Reset the SDpointer
        digitalWrite(PIN_STAT_LED, HIGH); //Flash the LED while writing
        gnssDataFile.write(SDbuffer, SDpacket); //Record the buffer to the card
        digitalWrite(PIN_STAT_LED, LOW);
      }

      bufAvail--; //Decrement bufAvail
    }

    //Force sync every 500ms
    if (millis() - lastDataLogSyncTime > 500)
    {
      lastDataLogSyncTime = millis();
      digitalWrite(PIN_STAT_LED, HIGH); //Flash the LED while writing
      if (SDpointer > 0) //Check if we have any 'extra' bytes in SDbuffer
      {
        gnssDataFile.write(SDbuffer, SDpointer); //Write the 'extra' bytes
        SDpointer = 0; //Reset the SDpointer
      }
      gnssDataFile.sync(); //sync the file system
      digitalWrite(PIN_STAT_LED, LOW);
    }
  }
  else
  {
    //Discard the data so we don't overflow the buffer
    while(GNSSbuffer.available())
      GNSSbuffer.read_char();
  }


  return(true);
}
