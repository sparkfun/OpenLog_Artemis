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
      //gpsSensor_ublox.addCfgValset8(0x10720004, 0); // CFG-I2COUTPROT-RTCM3X : Disable RTCM3 output on the I2C port (Precision modules only)
      uint8_t success = gpsSensor_ublox.sendCfgValset8(0x20920001, 0, 2100); // CFG-INFMSG-UBX_I2C : Disable UBX INFo messages on the I2C port (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset failed when setting the I2C port to output UBX only")); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset was successful when setting the I2C port to output UBX only")); 
          }       
      }

      //Disable all logable messages
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
      success = gpsSensor_ublox.sendCfgValset8(0x20910178, 0, 2100); // CFG-MSGOUT-UBX_TIM_TM2_I2C (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset failed when disabling messages")); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset was successful when disabling messages")); 
          }       
      }

      //Calculate measurement rate
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
      
      //If query rate is higher than 10Hz then disable all constellations except GPS
      //If query rate is higher than 5Hz and RAWX is enabled then also disable all constellations except GPS
      if ((measRate < 100) || ((measRate < 200) && (settings.sensor_uBlox.logUBXRXMRAWX == true)))
      {
        gpsSensor_ublox.newCfgValset8(0x10310021, 0, VAL_LAYER_RAM); // CFG-SIGNAL-GAL_ENA : Disable Galileo (in RAM only)
        gpsSensor_ublox.addCfgValset8(0x10310022, 0); // CFG-SIGNAL-BDS_ENA : Disable BeiDou
        gpsSensor_ublox.addCfgValset8(0x10310024, 0); // CFG-SIGNAL-QZSS_ENA : Disable QZSS
        success = gpsSensor_ublox.sendCfgValset8(0x10310025, 0, 2100); // CFG-SIGNAL-GLO_ENA : Disable GLONASS (maxWait 2100ms)
        if (success == 0)
        {
          if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset failed when disabling constellations")); 
            }       
        }
        else
        {
          if (settings.printMinorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset was successful when disabling constellations")); 
            }       
        }        
      }
      else
      {
        gpsSensor_ublox.newCfgValset8(0x10310021, 1, VAL_LAYER_RAM); // CFG-SIGNAL-GAL_ENA : Enable Galileo (in RAM only)
        gpsSensor_ublox.addCfgValset8(0x10310022, 1); // CFG-SIGNAL-BDS_ENA : Enable BeiDou
        gpsSensor_ublox.addCfgValset8(0x10310024, 1); // CFG-SIGNAL-QZSS_ENA : Enable QZSS
        success = gpsSensor_ublox.sendCfgValset8(0x10310025, 1, 2100); // CFG-SIGNAL-GLO_ENA : Enable GLONASS (maxWait 2100ms)
        if (success == 0)
        {
          if (settings.printMajorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset failed when enabling constellations")); 
            }       
        }
        else
        {
          if (settings.printMinorDebugMessages == true)
            {
              Serial.println(F("beginSensors: sendCfgValset was successful when enabling constellations")); 
            }       
        }        
      }

      //Set output rate
      gpsSensor_ublox.newCfgValset16(0x30210001, measRate, VAL_LAYER_RAM); // CFG-RATE-MEAS : Configure measurement period (in RAM only)
      success = gpsSensor_ublox.sendCfgValset16(0x30210002, 1, 2100); // CFG-RATE-NAV : 1 measurement per navigation solution (maxWait 2100ms)
      if (success == 0)
      {
        if (settings.printMajorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset failed when setting message interval")); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset was successful when setting message interval")); 
          }       
      }

      //Enable the selected messages
      //TO DO: Selecting an invalid message (e.g. RAWX) for this module causes this to NACK. Needs to be prevented.
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
            Serial.println(F("beginSensors: sendCfgValset failed when enabling messages")); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println(F("beginSensors: sendCfgValset was successful when enabling messages")); 
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
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait > 500ms so we can be sure SD data is sync'd
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
          Serial.println(F("openNewLogFile: failed to create new sensor data file"));
        }       
        
        online.dataLogging = false;
        return;
      }

      //(Re)Enable the selected messages
      //TO DO: Selecting an invalid message (e.g. RAWX) for this module causes this to NACK. Needs to be prevented.
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
            Serial.println(F("openNewLogFile: sendCfgValset failed when enabling messages")); 
          }       
      }
      else
      {
        if (settings.printMinorDebugMessages == true)
          {
            Serial.println(F("openNewLogFile: sendCfgValset was successful when enabling messages")); 
          }       
      }
    }
  }
}

//Close the current log file and then reset the GNSS
void resetGNSS()
{
  if (settings.logData && settings.enableSD && online.microSD && online.dataLogging) //If we are logging
  {
    if (qwiicAvailable.uBlox && settings.sensor_uBlox.log && qwiicOnline.uBlox) //If the uBlox is available and logging
    {
      //Disable all messages in RAM
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
      //and success will always be false (sendCommand returns SFE_UBLOX_STATUS_SUCCESS not SFE_UBLOX_STATUS_DATA_SENT)

      unsigned long pauseUntil = millis() + 2100UL; //Wait > 500ms so we can be sure SD data is sync'd
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
      gpsSensor_ublox.factoryDefault(2100);
      gpsSensor_ublox.factoryReset();

      //Wait 5 secs
      Serial.print(F("GNSS has been reset. Waiting 5 seconds."));
      delay(1000);
      Serial.print(F("."));
      delay(1000);
      Serial.print(F("."));
      delay(1000);
      Serial.print(F("."));
      delay(1000);
      Serial.print(F("."));
      delay(1000);
      Serial.println(F("."));
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
