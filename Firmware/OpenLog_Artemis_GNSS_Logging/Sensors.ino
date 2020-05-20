//Init / begin comm with all enabled sensors
bool beginSensors()
{
  beginSensorOutput = "";

  //If no sensors are available then return
  if (detectQwiicDevices() == false)
    return false;

  determineMaxI2CSpeed(); //Try for 400kHz but reduce to 100kHz or low if certain devices are attached

  if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && !qwiicOnline.uBlox)
  {
    if (gpsSensor_ublox.begin(qwiic, settings.sensor_uBlox.ubloxI2Caddress) == true) //Wire port, Address. Default is 0x42.
    {
      //Set the I2C port to output UBX only (turn off NMEA noise)
      gpsSensor_ublox.newCfgValset8(0x10720001, 1, VAL_LAYER_RAM); // CFG-I2COUTPROT-UBX : Enable UBX output on the I2C port (in RAM only)
      gpsSensor_ublox.addCfgValset8(0x10720002, 0); // CFG-I2COUTPROT-NMEA : Disable NMEA output on the I2C port
      gpsSensor_ublox.addCfgValset8(0x10720004, 0); // CFG-I2COUTPROT-RTCM3X : Disable RTCM3 output on the I2C port
      uint8_t success = gpsSensor_ublox.sendCfgValset8(0x20920001, 0, 2100); // CFG-INFMSG-UBX_I2C : Disable INFo messages on the I2C port (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println("beginSensors: sendCfgValset failed when setting the I2C port to output UBX only"); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println("beginSensors: sendCfgValset was successful when setting the I2C port to output UBX only"); 
          }       
      }

      //Enable the selected messages
      //TO DO: Check if selecting an invalid message for this module causes this to NACK
      gpsSensor_ublox.newCfgValset8(0x20910065, settings.sensor_uBlox.logUBXNAVCLOCK, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_CLOCK_I2C (in RAM only)
      gpsSensor_ublox.addCfgValset8(0x2091002e, settings.sensor_uBlox.logUBXNAVHPPOSECEF); // CFG-MSGOUT-UBX_NAV_HPPOSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910033, settings.sensor_uBlox.logUBXNAVHPPOSLLH); // CFG-MSGOUT-UBX_NAV_HPPOSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x2091007e, settings.sensor_uBlox.logUBXNAVODO); // CFG-MSGOUT-UBX_NAV_ODO_I2C
      gpsSensor_ublox.addCfgValset8(0x20910024, settings.sensor_uBlox.logUBXNAVPOSECEF); // CFG-MSGOUT-UBX_NAV_POSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910029, settings.sensor_uBlox.logUBXNAVPOSLLH); // CFG-MSGOUT-UBX_NAV_POSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x20910006, settings.sensor_uBlox.logUBXNAVPVT); // CFG-MSGOUT-UBX_NAV_PVT_I2C
      gpsSensor_ublox.addCfgValset8(0x2091008d, settings.sensor_uBlox.logUBXNAVRELPOSNED); // CFG-MSGOUT-UBX_NAV_RELPOSNED_I2C
      gpsSensor_ublox.addCfgValset8(0x2091001a, settings.sensor_uBlox.logUBXNAVSTATUS); // CFG-MSGOUT-UBX_NAV_STATUS_I2C
      gpsSensor_ublox.addCfgValset8(0x2091005b, settings.sensor_uBlox.logUBXNAVTIMEUTC); // CFG-MSGOUT-UBX_NAV_TIMEUTC_I2C
      gpsSensor_ublox.addCfgValset8(0x2091003d, settings.sensor_uBlox.logUBXNAVVELECEF); // CFG-MSGOUT-UBX_NAV_VELECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910042, settings.sensor_uBlox.logUBXNAVVELNED); // CFG-MSGOUT-UBX_NAV_VELNED_I2C
      gpsSensor_ublox.addCfgValset8(0x209102a4, settings.sensor_uBlox.logUBXRXMRAWX); // CFG-MSGOUT-UBX_RXM_RAWX_I2C
      gpsSensor_ublox.addCfgValset8(0x20910231, settings.sensor_uBlox.logUBXRXMSFRBX); // CFG-MSGOUT-UBX_RXM_SFRBX_I2C
      success = gpsSensor_ublox.sendCfgValset8(0x20910178, settings.sensor_uBlox.logUBXTIMTM2, 2100); // CFG-MSGOUT-UBX_TIM_TM2_I2C (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println("beginSensors: sendCfgValset failed when enabling messages"); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println("beginSensors: sendCfgValset was successful when enabling messages"); 
          }       
      }

      //Set output rate equal to our query rate
      uint16_t measRate;
      if (settings.usBetweenReadings < (((uint32_t)settings.sensor_uBlox.minMeasInterval) * 1000)) // Check if usBetweenReadings is too low
      {
        measRate = settings.sensor_uBlox.minMeasInterval;
      }
      else if (settings.usBetweenReadings > (0xFFFF * 1000)) // Check if usBetweenReadings is too high
      {
        measRate = 0xFFFF;
      }
      else
      {
        measRate = (uint16_t)(settings.usBetweenReadings / 1000); // Convert usBetweenReadings to ms
      }
      gpsSensor_ublox.newCfgValset16(0x30210001, measRate, VAL_LAYER_RAM); // CFG-RATE-MEAS : Configure measurement period (in RAM only)
      success = gpsSensor_ublox.sendCfgValset16(0x30210002, 1, 2100); // CFG-RATE-NAV : 1 measurement per navigation solution (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println("beginSensors: sendCfgValset failed when setting message interval"); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println("beginSensors: sendCfgValset was successful when setting message interval"); 
          }       
      }

      qwiicOnline.uBlox = true;
      beginSensorOutput += "uBlox GPS Online\n";
    }
    else
      beginSensorOutput += "uBlox GPS failed to respond. Check wiring and I2C address of module with uCenter.\n";
  }

  return true;
}

//Close the current log file and open a new one
//This should probably be defined in OpenLog_Artemis_GNSS_Logging as it involves files
//but it is defined here as it is uBlox-specific
void openNewLogFile()
{
  if (settings.logData && settings.enableSD && online.microSD && online.dataLogging) //If we are logging
  {
    if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && qwiicOnline.uBlox) //If the uBlox is available and logging
    {
      //Disable all messages in RAM
      //TO DO: Check if selecting an invalid message for this module causes this to NACK
      gpsSensor_ublox.newCfgValset8(0x20910065, 0, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_CLOCK_I2C (in RAM only)
      gpsSensor_ublox.addCfgValset8(0x2091002e, 0); // CFG-MSGOUT-UBX_NAV_HPPOSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910033, 0); // CFG-MSGOUT-UBX_NAV_HPPOSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x2091007e, 0); // CFG-MSGOUT-UBX_NAV_ODO_I2C
      gpsSensor_ublox.addCfgValset8(0x20910024, 0); // CFG-MSGOUT-UBX_NAV_POSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910029, 0); // CFG-MSGOUT-UBX_NAV_POSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x20910006, 0); // CFG-MSGOUT-UBX_NAV_PVT_I2C
      gpsSensor_ublox.addCfgValset8(0x2091008d, 0); // CFG-MSGOUT-UBX_NAV_RELPOSNED_I2C
      gpsSensor_ublox.addCfgValset8(0x2091001a, 0); // CFG-MSGOUT-UBX_NAV_STATUS_I2C
      gpsSensor_ublox.addCfgValset8(0x2091005b, 0); // CFG-MSGOUT-UBX_NAV_TIMEUTC_I2C
      gpsSensor_ublox.addCfgValset8(0x2091003d, 0); // CFG-MSGOUT-UBX_NAV_VELECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910042, 0); // CFG-MSGOUT-UBX_NAV_VELNED_I2C
      gpsSensor_ublox.addCfgValset8(0x209102a4, 0); // CFG-MSGOUT-UBX_RXM_RAWX_I2C
      gpsSensor_ublox.addCfgValset8(0x20910231, 0); // CFG-MSGOUT-UBX_RXM_SFRBX_I2C
      uint8_t success = gpsSensor_ublox.sendCfgValset8(0x20910178, settings.sensor_uBlox.logUBXTIMTM2, 0); // CFG-MSGOUT-UBX_TIM_TM2_I2C (maxWait 0!)
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //The ACK will end up being logged to SD card. I don't think there is much we can do about that?
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println("openNewLogFile: sendCfgValset failed when disabling messages"); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println("openNewLogFile: sendCfgValset was successful when disabling messages"); 
          }       
      }

      unsigned long pauseUntil = millis() + 550UL; //Wait > 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData(); //storeData is the workhorse. It reads I2C data and writes it to SD.
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
      gnssDataFile.sync();
      gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098

      strcpy(gnssDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));

      // O_CREAT - create the file if it does not exist
      // O_APPEND - seek to the end of the file prior to each write
      // O_WRITE - open for write
      if (gnssDataFile.open(gnssDataFileName, O_CREAT | O_APPEND | O_WRITE) == false)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println("openNewLogFile: failed to create new sensor data file");
          }       
        
        online.dataLogging = false;
        return;
      }

      //(Re)Enable the selected messages
      //TO DO: Check if selecting an invalid message for this module causes this to NACK
      gpsSensor_ublox.newCfgValset8(0x20910065, settings.sensor_uBlox.logUBXNAVCLOCK, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_CLOCK_I2C (in RAM only)
      gpsSensor_ublox.addCfgValset8(0x2091002e, settings.sensor_uBlox.logUBXNAVHPPOSECEF); // CFG-MSGOUT-UBX_NAV_HPPOSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910033, settings.sensor_uBlox.logUBXNAVHPPOSLLH); // CFG-MSGOUT-UBX_NAV_HPPOSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x2091007e, settings.sensor_uBlox.logUBXNAVODO); // CFG-MSGOUT-UBX_NAV_ODO_I2C
      gpsSensor_ublox.addCfgValset8(0x20910024, settings.sensor_uBlox.logUBXNAVPOSECEF); // CFG-MSGOUT-UBX_NAV_POSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910029, settings.sensor_uBlox.logUBXNAVPOSLLH); // CFG-MSGOUT-UBX_NAV_POSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x20910006, settings.sensor_uBlox.logUBXNAVPVT); // CFG-MSGOUT-UBX_NAV_PVT_I2C
      gpsSensor_ublox.addCfgValset8(0x2091008d, settings.sensor_uBlox.logUBXNAVRELPOSNED); // CFG-MSGOUT-UBX_NAV_RELPOSNED_I2C
      gpsSensor_ublox.addCfgValset8(0x2091001a, settings.sensor_uBlox.logUBXNAVSTATUS); // CFG-MSGOUT-UBX_NAV_STATUS_I2C
      gpsSensor_ublox.addCfgValset8(0x2091005b, settings.sensor_uBlox.logUBXNAVTIMEUTC); // CFG-MSGOUT-UBX_NAV_TIMEUTC_I2C
      gpsSensor_ublox.addCfgValset8(0x2091003d, settings.sensor_uBlox.logUBXNAVVELECEF); // CFG-MSGOUT-UBX_NAV_VELECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910042, settings.sensor_uBlox.logUBXNAVVELNED); // CFG-MSGOUT-UBX_NAV_VELNED_I2C
      gpsSensor_ublox.addCfgValset8(0x209102a4, settings.sensor_uBlox.logUBXRXMRAWX); // CFG-MSGOUT-UBX_RXM_RAWX_I2C
      gpsSensor_ublox.addCfgValset8(0x20910231, settings.sensor_uBlox.logUBXRXMSFRBX); // CFG-MSGOUT-UBX_RXM_SFRBX_I2C
      success = gpsSensor_ublox.sendCfgValset8(0x20910178, settings.sensor_uBlox.logUBXTIMTM2, 2100); // CFG-MSGOUT-UBX_TIM_TM2_I2C (maxWait 1100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println("openNewLogFile: sendCfgValset failed when enabling messages"); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println("openNewLogFile: sendCfgValset was successful when enabling messages"); 
          }       
      }
    }
  }
}

//Close the current log file and then reset the GNSS
bool resetGNSS()
{
  if (settings.logData && settings.enableSD && online.microSD && online.dataLogging) //If we are logging
  {
    if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && qwiicOnline.uBlox) //If the uBlox is available and logging
    {
      //Disable all messages in RAM
      //TO DO: Check if selecting an invalid message for this module causes this to NACK
      gpsSensor_ublox.newCfgValset8(0x20910065, 0, VAL_LAYER_RAM); // CFG-MSGOUT-UBX_NAV_CLOCK_I2C (in RAM only)
      gpsSensor_ublox.addCfgValset8(0x2091002e, 0); // CFG-MSGOUT-UBX_NAV_HPPOSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910033, 0); // CFG-MSGOUT-UBX_NAV_HPPOSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x2091007e, 0); // CFG-MSGOUT-UBX_NAV_ODO_I2C
      gpsSensor_ublox.addCfgValset8(0x20910024, 0); // CFG-MSGOUT-UBX_NAV_POSECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910029, 0); // CFG-MSGOUT-UBX_NAV_POSLLH_I2C
      gpsSensor_ublox.addCfgValset8(0x20910006, 0); // CFG-MSGOUT-UBX_NAV_PVT_I2C
      gpsSensor_ublox.addCfgValset8(0x2091008d, 0); // CFG-MSGOUT-UBX_NAV_RELPOSNED_I2C
      gpsSensor_ublox.addCfgValset8(0x2091001a, 0); // CFG-MSGOUT-UBX_NAV_STATUS_I2C
      gpsSensor_ublox.addCfgValset8(0x2091005b, 0); // CFG-MSGOUT-UBX_NAV_TIMEUTC_I2C
      gpsSensor_ublox.addCfgValset8(0x2091003d, 0); // CFG-MSGOUT-UBX_NAV_VELECEF_I2C
      gpsSensor_ublox.addCfgValset8(0x20910042, 0); // CFG-MSGOUT-UBX_NAV_VELNED_I2C
      gpsSensor_ublox.addCfgValset8(0x209102a4, 0); // CFG-MSGOUT-UBX_RXM_RAWX_I2C
      gpsSensor_ublox.addCfgValset8(0x20910231, 0); // CFG-MSGOUT-UBX_RXM_SFRBX_I2C
      uint8_t success = gpsSensor_ublox.sendCfgValset8(0x20910178, settings.sensor_uBlox.logUBXTIMTM2, 0); // CFG-MSGOUT-UBX_TIM_TM2_I2C (maxWait 0!)
      //Using a maxWait of zero means we don't wait for the ACK/NACK
      //The ACK will end up being logged to SD card. I don't think there is much we can do about that?
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println("openNewLogFile: sendCfgValset failed when disabling messages"); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println("openNewLogFile: sendCfgValset was successful when disabling messages"); 
          }       
      }

      unsigned long pauseUntil = millis() + 550UL; //Wait > 500ms so we can be sure SD data is sync'd
      while (millis() < pauseUntil) //While we are pausing, keep writing data to SD
      {
        storeData(); //storeData is the workhorse. It reads I2C data and writes it to SD.
      }

      //We've waited long enough for the last of the data to come in
      //so now we can close the current file and open a new one
      Serial.print(F("Closing: "));
      Serial.println(gnssDataFileName);
      gnssDataFile.sync();
      gnssDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098

      //Reset the GNSS
      gpsSensor_ublox.factoryDefault(1100);
      gpsSensor_ublox.factoryReset();

      //Wait 3 secs
      Serial.print(F("GNSS has been reset. Waiting 3 seconds."));
      delay(1000);
      Serial.print(F("."));
      delay(1000);
      Serial.print(F("."));
      delay(1000);
      Serial.println(F("."));

      //Call beginSensors to reset logging
      return(beginSensors());
    }
  }
}

////Query each enabled sensor for it's most recent data
//void getData()
//{
//  measurementCount++;
//
//  outputData = "";
//  String helperText = "";
//
//  if (qwiicOnline.uBlox && settings.sensor_uBlox.log)
//  {
//    gpsSensor_ublox.getPVT();
//    if (settings.sensor_uBlox.logUBXNAVPVT)
//    {
//      outputData += (String)gpsSensor_ublox.getLatitude() + ",";
//      outputData += (String)gpsSensor_ublox.getLongitude();
//      helperText += "gps_Lat,gps_Long";
//    }
//    gpsSensor_ublox.flushPVT(); //Mark all PVT data as used
//  }
//
//  outputData += '\n';
//  helperText += '\n';
//
//  totalCharactersPrinted += outputData.length();
//
//  if (settings.showHelperText == true)
//  {
//    if (helperTextPrinted == false)
//    {
//      helperTextPrinted = true;
//      outputData = helperText + outputData;
//    }
//  }
//}

//If certain devices are attached, we need to reduce the I2C max speed
void determineMaxI2CSpeed()
{
  uint32_t maxSpeed = 400000; //Assume 400kHz

  if (settings.sensor_uBlox.i2cSpeed == 100000)
    maxSpeed = 100000;

  //If user wants to limit the I2C bus speed, do it here
  if (maxSpeed > settings.qwiicBusMaxSpeed)
    maxSpeed = settings.qwiicBusMaxSpeed;

  qwiic.setClock(maxSpeed);
}
