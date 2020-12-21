void menuIMU()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure IMU"));

    Serial.print(F("1) Sensor Logging: "));
    if (settings.enableIMU == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    if (settings.enableIMU == true)
    {
      Serial.print(F("2) Accelerometer Logging: "));
      if (settings.logIMUAccel) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      Serial.print(F("3) Gyro Logging: "));
      if (settings.logIMUGyro) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      Serial.print(F("4) Magnotometer Logging: "));
      if (settings.logIMUMag) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      Serial.print(F("5) Temperature Logging: "));
      if (settings.logIMUTemp) Serial.println(F("Enabled"));
      else Serial.println(F("Disabled"));

      if (online.IMU == true)
      {
        Serial.print(F("6) Accelerometer Full Scale: +/- "));
        switch (settings.imuAccFSS)
        {
          case 0:
            Serial.println(F("2g"));
            break;
          case 1:
            Serial.println(F("4g"));
            break;
          case 2:
            Serial.println(F("8g"));
            break;
          case 3:
            Serial.println(F("16g"));
            break;
          default:
            Serial.println(F("UNKNOWN"));
            break;
        }

        Serial.print(F("7) Accelerometer Digital Low Pass Filter: "));
        if (settings.imuAccDLPF)
        {
          Serial.println(F("Enabled"));
          Serial.print(F("8) Accelerometer DLPF Bandwidth (Hz): "));
          switch (settings.imuAccDLPFBW)
          {
            case 0:
              Serial.println(F("246.0 (3dB)  265.0 (Nyquist)"));
              break;
            case 1:
              Serial.println(F("246.0 (3dB)  265.0 (Nyquist)"));
              break;
            case 2:
              Serial.println(F("111.4 (3dB)  136.0 (Nyquist)"));
              break;
            case 3:
              Serial.println(F("50.4 (3dB)  68.8 (Nyquist)"));
              break;
            case 4:
              Serial.println(F("23.9 (3dB)  34.4 (Nyquist)"));
              break;
            case 5:
              Serial.println(F("11.5 (3dB)  17.0 (Nyquist)"));
              break;
            case 6:
              Serial.println(F("5.7 (3dB)  8.3 (Nyquist)"));
              break;
            case 7:
              Serial.println(F("473 (3dB)  499 (Nyquist)"));
              break;
            default:
              Serial.println(F("UNKNOWN"));
              break;
          }
        }
        else
        {
          Serial.println(F("Disabled  (Bandwidth is 1209 Hz (3dB) 1248 Hz (Nyquist))"));
        }

        Serial.print(F("9) Gyro Full Scale: +/- "));
        switch (settings.imuGyroFSS)
        {
          case 0:
            Serial.println(F("250dps"));
            break;
          case 1:
            Serial.println(F("500dps"));
            break;
          case 2:
            Serial.println(F("1000dps"));
            break;
          case 3:
            Serial.println(F("2000dps"));
            break;
          default:
            Serial.println(F("UNKNOWN"));
            break;
        }

        Serial.print(F("10) Gyro Digital Low Pass Filter: "));
        if (settings.imuGyroDLPF)
        {
          Serial.println(F("Enabled"));
          Serial.print(F("11) Gyro DLPF Bandwidth (Hz): "));
          switch (settings.imuGyroDLPFBW)
          {
            case 0:
              Serial.println(F("196.6 (3dB)  229.8 (Nyquist)"));
              break;
            case 1:
              Serial.println(F("151.8 (3dB)  187.6 (Nyquist)"));
              break;
            case 2:
              Serial.println(F("119.5 (3dB)  154.3 (Nyquist)"));
              break;
            case 3:
              Serial.println(F("51.2 (3dB)  73.3 (Nyquist)"));
              break;
            case 4:
              Serial.println(F("23.9 (3dB)  35.9 (Nyquist)"));
              break;
            case 5:
              Serial.println(F("11.6 (3dB)  17.8 (Nyquist)"));
              break;
            case 6:
              Serial.println(F("5.7 (3dB)  8.9 (Nyquist)"));
              break;
            case 7:
              Serial.println(F("361.4 (3dB)  376.5 (Nyquist)"));
              break;
            default:
              Serial.println(F("UNKNOWN"));
              break;
          }
        }
        else
        {
          Serial.println(F("Disabled  (Bandwidth is 12106 Hz (3dB) 12316 Hz (Nyquist))"));
        }
      }
    }
    
    Serial.println(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
    {
      settings.enableIMU ^= 1;
      if (settings.enableIMU == true) beginIMU();
      else online.IMU = false;
    }
    else if (settings.enableIMU == true)
    {
      if (incoming == 2)
        settings.logIMUAccel ^= 1;
      else if (incoming == 3)
        settings.logIMUGyro ^= 1;
      else if (incoming == 4)
        settings.logIMUMag ^= 1;
      else if (incoming == 5)
        settings.logIMUTemp ^= 1;
      else if ((incoming == 6) && (online.IMU == true))
      {
        Serial.println(F("Enter Accelerometer Full Scale (0 to 3): "));
        Serial.println(F("0: +/- 2g"));
        Serial.println(F("1: +/- 4g"));
        Serial.println(F("2: +/- 8g"));
        Serial.println(F("3: +/- 16g"));
        int afs = getNumber(menuTimeout); //x second timeout
        if (afs < 0 || afs > 3)
          Serial.println(F("Error: Out of range"));
        else
        {
          settings.imuAccFSS = afs;
          ICM_20948_fss_t FSS;
          FSS.a = settings.imuAccFSS;
          FSS.g = settings.imuGyroFSS;
          ICM_20948_Status_e retval = myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), FSS);
          if (retval != ICM_20948_Stat_Ok)
          {
            Serial.println(F("Error: Could not configure the IMU!"));
          }
        }
      }
      else if ((incoming == 7) && (online.IMU == true))
      {
        settings.imuAccDLPF ^= 1;
        ICM_20948_Status_e retval = myICM.enableDLPF(ICM_20948_Internal_Acc, settings.imuAccDLPF);
        if (retval != ICM_20948_Stat_Ok)
        {
          Serial.println(F("Error: Could not configure the IMU!"));
        }
      }
      else if ((incoming == 8) && (online.IMU == true) && (settings.imuAccDLPF == true))
      {
        Serial.println(F("Enter Accelerometer DLPF Bandwidth (0 to 7): "));
        Serial.println(F("0: 246.0 (3dB)  265.0 (Nyquist) (Hz)"));
        Serial.println(F("1: 246.0 (3dB)  265.0 (Nyquist) (Hz)"));
        Serial.println(F("2: 111.4 (3dB)  136.0 (Nyquist) (Hz)"));
        Serial.println(F("3: 50.4  (3dB)  68.8  (Nyquist) (Hz)"));
        Serial.println(F("4: 23.9  (3dB)  34.4  (Nyquist) (Hz)"));
        Serial.println(F("5: 11.5  (3dB)  17.0  (Nyquist) (Hz)"));
        Serial.println(F("6: 5.7   (3dB)  8.3   (Nyquist) (Hz)"));
        Serial.println(F("7: 473   (3dB)  499   (Nyquist) (Hz)"));
        int afbw = getNumber(menuTimeout); //x second timeout
        if (afbw < 0 || afbw > 7)
          Serial.println(F("Error: Out of range"));
        else
        {
          settings.imuAccDLPFBW = afbw;
          ICM_20948_dlpcfg_t dlpcfg;
          dlpcfg.a = settings.imuAccDLPFBW;
          dlpcfg.g = settings.imuGyroDLPFBW;
          ICM_20948_Status_e retval = myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), dlpcfg);
          if (retval != ICM_20948_Stat_Ok)
          {
            Serial.println(F("Error: Could not configure the IMU!"));
          }
        }
      }
      else if ((incoming == 9) && (online.IMU == true))
      {
        Serial.println(F("Enter Gyro Full Scale (0 to 3): "));
        Serial.println(F("0: +/- 250dps"));
        Serial.println(F("1: +/- 500dps"));
        Serial.println(F("2: +/- 1000dps"));
        Serial.println(F("3: +/- 2000dps"));
        int gfs = getNumber(menuTimeout); //x second timeout
        if (gfs < 0 || gfs > 3)
          Serial.println(F("Error: Out of range"));
        else
        {
          settings.imuGyroFSS = gfs;
          ICM_20948_fss_t FSS;
          FSS.a = settings.imuAccFSS;
          FSS.g = settings.imuGyroFSS;
          ICM_20948_Status_e retval = myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), FSS);
          if (retval != ICM_20948_Stat_Ok)
          {
            Serial.println(F("Error: Could not configure the IMU!"));
          }
        }
      }
      else if ((incoming == 10) && (online.IMU == true))
      {
        settings.imuGyroDLPF ^= 1;
        ICM_20948_Status_e retval = myICM.enableDLPF(ICM_20948_Internal_Gyr, settings.imuGyroDLPF);
        if (retval != ICM_20948_Stat_Ok)
        {
          Serial.println(F("Error: Could not configure the IMU!"));
        }
      }
      else if ((incoming == 11) && (online.IMU == true) && (settings.imuGyroDLPF == true))
      {
        Serial.println(F("Enter Gyro DLPF Bandwidth (0 to 7): "));
        Serial.println(F("0: 196.6 (3dB)  229.8 (Nyquist) (Hz)"));
        Serial.println(F("1: 151.8 (3dB)  187.6 (Nyquist) (Hz)"));
        Serial.println(F("2: 119.5 (3dB)  154.3 (Nyquist) (Hz)"));
        Serial.println(F("3: 51.2  (3dB)  73.3  (Nyquist) (Hz)"));
        Serial.println(F("4: 23.9  (3dB)  35.9  (Nyquist) (Hz)"));
        Serial.println(F("5: 11.6  (3dB)  17.8  (Nyquist) (Hz)"));
        Serial.println(F("6: 5.7   (3dB)  8.9   (Nyquist) (Hz)"));
        Serial.println(F("7: 361.4 (3dB)  376.5 (Nyquist) (Hz)"));
        int gfbw = getNumber(menuTimeout); //x second timeout
        if (gfbw < 0 || gfbw > 7)
          Serial.println(F("Error: Out of range"));
        else
        {
          settings.imuGyroDLPFBW = gfbw;
          ICM_20948_dlpcfg_t dlpcfg;
          dlpcfg.a = settings.imuAccDLPFBW;
          dlpcfg.g = settings.imuGyroDLPFBW;
          ICM_20948_Status_e retval = myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), dlpcfg);
          if (retval != ICM_20948_Stat_Ok)
          {
            Serial.println(F("Error: Could not configure the IMU!"));
          }
        }
      }
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
}
