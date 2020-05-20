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
      gpsSensor_ublox.newCfgValset8(0x10720001, 1, VAL_LAYER_BBR | VAL_LAYER_BBR); // CFG-I2COUTPROT-UBX : Enable UBX output on the I2C port (in RAM and BBR)
      gpsSensor_ublox.addCfgValset8(0x10720002, 0); // CFG-I2COUTPROT-NMEA : Disable NMEA output on the I2C port
      gpsSensor_ublox.addCfgValset8(0x10720004, 0); // CFG-I2COUTPROT-RTCM3X : Disable RTCM3 output on the I2C port
      gpsSensor_ublox.sendCfgValset8(0x20920001, 0, 1100); // CFG-INFMSG-UBX_I2C : Disable INFo messages on the I2C port (maxWait 1100ms)

      //Enable the selected messages
      //TO DO: Check if selecting an invalid message for this module causes this to NACK
      gpsSensor_ublox.newCfgValset8(0x20910065, settings.sensor_uBlox.logUBXNAVCLOCK, VAL_LAYER_BBR | VAL_LAYER_BBR); // CFG-MSGOUT-UBX_NAV_CLOCK_I2C (in RAM and BBR)
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
      gpsSensor_ublox.sendCfgValset8(0x20910178, settings.sensor_uBlox.logUBXTIMTM2, 1100); // CFG-MSGOUT-UBX_TIM_TM2_I2C (maxWait 1100ms)

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
      gpsSensor_ublox.newCfgValset16(0x30210001, measRate, VAL_LAYER_BBR | VAL_LAYER_BBR); // CFG-RATE-MEAS : Configure measurement period (in RAM and BBR)
      gpsSensor_ublox.sendCfgValset16(0x30210002, 1, 1100); // CFG-RATE-NAV : 1 measurement per navigation solution (maxWait 1100ms)

      qwiicOnline.uBlox = true;
      beginSensorOutput += "uBlox GPS Online\n";
    }
    else
      beginSensorOutput += "uBlox GPS failed to respond. Check wiring and I2C address of module with uCenter.\n";
  }

  return true;
}

//Query each enabled sensor for it's most recent data
void getData()
{
  measurementCount++;

  outputData = "";
  String helperText = "";

  if (qwiicOnline.uBlox && settings.sensor_uBlox.log)
  {
    gpsSensor_ublox.getPVT();
    if (settings.sensor_uBlox.logUBXNAVPVT)
    {
      outputData += (String)gpsSensor_ublox.getLatitude() + ",";
      outputData += (String)gpsSensor_ublox.getLongitude();
      helperText += "gps_Lat,gps_Long";
    }
    gpsSensor_ublox.flushPVT(); //Mark all PVT data as used
  }

  outputData += '\n';
  helperText += '\n';

  totalCharactersPrinted += outputData.length();

  if (settings.showHelperText == true)
  {
    if (helperTextPrinted == false)
    {
      helperTextPrinted = true;
      outputData = helperText + outputData;
    }
  }
}

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
