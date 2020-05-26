//Configure the attached GNSS module
void menuConfigure_uBlox()
{
  while (1)
  {
    Serial.println();
    Serial.print(F("Menu: Configure uBlox GNSS Receiver "));

    if (qwiicOnline.uBlox == false)
    {
      Serial.println(F("\n**No GNSS device detected on Qwiic bus**"));
    }
    else
    {
      //Print the module information
      Serial.print(minfo.mod);
      if (minfo.SPG == true)
      {
        Serial.print(F(" SPG")); //Standard Precision
      }
      if (minfo.HPG == true)
      {
        Serial.print(F(" HPG")); //High Precision (ZED-F9P)
      }
      if (minfo.ADR == true)
      {
        Serial.print(F(" ADR")); //Dead Reckoning (ZED-F9K)
      }
      if (minfo.UDR == true)
      {
        Serial.print(F(" UDR")); //Untethered Dead Reckoning
      }
      if (minfo.TIM == true)
      {
        Serial.print(F(" TIM")); //Time sync (ZED-F9T)
      }
      if (minfo.FTS == true)
      {
        Serial.print(F(" FTS")); //Frequency and time sync
      }
      if (minfo.LAP == true)
      {
        Serial.print(F(" LAP")); //Lane accurate (ZED-F9R)
      }      
      if (minfo.HDG == true)
      {
        Serial.print(F(" HDG")); //Heading (ZED-F9H)
      }
      Serial.println();
      
      Serial.print(F(" 1) Sensor Logging                                                             : "));
      if (settings.sensor_uBlox.log == true) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));
  
      if (settings.sensor_uBlox.log == true)
      {
        Serial.print(F(" 2) Use a power management task to put the module to sleep                     : "));
        if (settings.sensor_uBlox.powerManagement == true) Serial.println(F("Yes"));
        else Serial.println(F("No"));
  
        Serial.print(F("10) Log UBX-NAV-CLOCK     (Clock Solution)                                     : "));
        if (settings.sensor_uBlox.logUBXNAVCLOCK == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("11) Log UBX-NAV-HPPOSECEF (High Precision Position Earth-Centered Earth-Fixed) : "));
        if (settings.sensor_uBlox.logUBXNAVHPPOSECEF == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("12) Log UBX-NAV-HPPOSLLH  (High Precision Position Lat/Lon/Height)             : "));
        if (settings.sensor_uBlox.logUBXNAVHPPOSLLH == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("13) Log UBX-NAV-ODO       (Odometer)                                           : "));
        if (settings.sensor_uBlox.logUBXNAVODO == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("14) Log UBX-NAV-POSECEF   (Position Earth-Centered Earth-Fixed)                : "));
        if (settings.sensor_uBlox.logUBXNAVPOSECEF == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("15) Log UBX-NAV-POSLLH    (Position Lat/Lon/Height)                            : "));
        if (settings.sensor_uBlox.logUBXNAVPOSLLH == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("16) Log UBX-NAV-PVT       (Position, Velocity, Time)                           : "));
        if (settings.sensor_uBlox.logUBXNAVPVT == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("17) Log UBX-NAV-STATUS    (Receiver Navigation Status)                         : "));
        if (settings.sensor_uBlox.logUBXNAVSTATUS == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("18) Log UBX-NAV-TIMEUTC   (UTC Time Solution) (** Used to sync the OLA RTC **) : "));
        if (settings.sensor_uBlox.logUBXNAVTIMEUTC == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("19) Log UBX-NAV-VELECEF   (Velocity Solution Earth-Centered Earth-Fixed)       : "));
        if (settings.sensor_uBlox.logUBXNAVVELECEF == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("20) Log UBX-NAV-VELNED    (Velocity Solution North/East/Down)                  : "));
        if (settings.sensor_uBlox.logUBXNAVVELNED == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("30) Log UBX-RXM-SFRBX     (Broadcast Navigation Data Subframe)                 : "));
        if (settings.sensor_uBlox.logUBXRXMSFRBX == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("40) Log UBX-TIM-TM2       (Time Mark Data)                                     : "));
        if (settings.sensor_uBlox.logUBXTIMTM2 == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        if ((minfo.HPG == true) || (minfo.HDG == true) || (minfo.ADR == true) || (minfo.LAP == true))
        {
          Serial.print(F("50) Log UBX-NAV-RELPOSNED (Relative Position North/East/Down)                  : "));
          if (settings.sensor_uBlox.logUBXNAVRELPOSNED == true) Serial.println(F("Enabled"));
          else Serial.println(F("Disabled"));
        }
        if ((minfo.HPG == true) || (minfo.TIM == true) || (minfo.FTS == true))
        {
          Serial.print(F("60) Log UBX-RXM-RAWX      (Multi-GNSS Raw Measurement)                         : "));
          if (settings.sensor_uBlox.logUBXRXMRAWX == true) Serial.println(F("Enabled"));
          else Serial.println(F("Disabled"));
        }
  
        Serial.print(F("90) USB port     (Disabling this can reduce the load on the module)            : "));
        if (settings.sensor_uBlox.enableUSB == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("91) UART1 port   (Disabling this can reduce the load on the module)            : "));
        if (settings.sensor_uBlox.enableUART1 == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("92) UART2 port   (Disabling this can reduce the load on the module)            : "));
        if (settings.sensor_uBlox.enableUART2 == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.print(F("93) SPI port     (Disabling this can reduce the load on the module)            : "));
        if (settings.sensor_uBlox.enableSPI == true) Serial.println(F("Enabled"));
        else Serial.println(F("Disabled"));
  
        Serial.flush();
      }
    }
    Serial.println(F(" x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
    {
      settings.sensor_uBlox.log ^= 1;
    }
    else if ((settings.sensor_uBlox.log == true) && (qwiicOnline.uBlox == true))
    {
      if (incoming == 2)
        settings.sensor_uBlox.powerManagement ^= 1;
      if (incoming == 10)
        settings.sensor_uBlox.logUBXNAVCLOCK ^= 1;
      else if (incoming == 11)
        settings.sensor_uBlox.logUBXNAVHPPOSECEF ^= 1;
      else if (incoming == 12)
        settings.sensor_uBlox.logUBXNAVHPPOSLLH ^= 1;
      else if (incoming == 13)
        settings.sensor_uBlox.logUBXNAVODO ^= 1;
      else if (incoming == 14)
        settings.sensor_uBlox.logUBXNAVPOSECEF ^= 1;
      else if (incoming == 15)
        settings.sensor_uBlox.logUBXNAVPOSLLH ^= 1;
      else if (incoming == 16)
        settings.sensor_uBlox.logUBXNAVPVT ^= 1;
      else if (incoming == 17)
        settings.sensor_uBlox.logUBXNAVSTATUS ^= 1;
      else if (incoming == 18)
        settings.sensor_uBlox.logUBXNAVTIMEUTC ^= 1;
      else if (incoming == 19)
        settings.sensor_uBlox.logUBXNAVVELECEF ^= 1;
      else if (incoming == 20)
        settings.sensor_uBlox.logUBXNAVVELNED ^= 1;
      else if (incoming == 30)
        settings.sensor_uBlox.logUBXRXMSFRBX ^= 1;
      else if (incoming == 40)
        settings.sensor_uBlox.logUBXTIMTM2 ^= 1;
      else if ((incoming == 50) && ((minfo.HPG == true) || (minfo.HDG == true) || (minfo.ADR == true) || (minfo.LAP == true)))
        settings.sensor_uBlox.logUBXNAVRELPOSNED ^= 1;
      else if ((incoming == 60) && ((minfo.HPG == true) || (minfo.TIM == true) || (minfo.FTS == true)))
        settings.sensor_uBlox.logUBXRXMRAWX ^= 1;
      else if (incoming == 90)
        settings.sensor_uBlox.enableUSB ^= 1;
      else if (incoming == 91)
        settings.sensor_uBlox.enableUART1 ^= 1;
      else if (incoming == 92)
        settings.sensor_uBlox.enableUART2 ^= 1;
      else if (incoming == 93)
        settings.sensor_uBlox.enableSPI ^= 1;
      else if (incoming == STATUS_PRESSED_X)
        break;
      else if (incoming == STATUS_GETNUMBER_TIMEOUT)
        break;
      else
        printUnknown(incoming);
    }
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  qwiicOnline.uBlox = false; //Mark as offline so it will be started with new settings
}
