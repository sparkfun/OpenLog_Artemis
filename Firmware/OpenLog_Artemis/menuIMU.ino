// Return true if IMU requires a restart
bool menuIMU()
{
  bool restartIMU = false;
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Configure IMU"));

    SerialPrint(F("1) Sensor Logging: "));
    if (settings.enableIMU == true) SerialPrintln(F("Enabled"));
    else SerialPrintln(F("Disabled"));

    if (settings.enableIMU == true)
    {
      if (settings.imuUseDMP == false)
      {
        SerialPrint(F("2) Accelerometer Logging: "));
        if (settings.logIMUAccel) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
  
        SerialPrint(F("3) Gyro Logging: "));
        if (settings.logIMUGyro) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
  
        SerialPrint(F("4) Magnetometer Logging: "));
        if (settings.logIMUMag) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
  
        SerialPrint(F("5) Temperature Logging: "));
        if (settings.logIMUTemp) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
  
        if (online.IMU == true)
        {
          SerialPrint(F("6) Accelerometer Full Scale: +/- "));
          switch (settings.imuAccFSS)
          {
            case 0:
              SerialPrintln(F("2g"));
              break;
            case 1:
              SerialPrintln(F("4g"));
              break;
            case 2:
              SerialPrintln(F("8g"));
              break;
            case 3:
              SerialPrintln(F("16g"));
              break;
            default:
              SerialPrintln(F("UNKNOWN"));
              break;
          }
  
          SerialPrint(F("7) Accelerometer Digital Low Pass Filter: "));
          if (settings.imuAccDLPF)
          {
            SerialPrintln(F("Enabled"));
            SerialPrint(F("8) Accelerometer DLPF Bandwidth (Hz): "));
            switch (settings.imuAccDLPFBW)
            {
              case 0:
                SerialPrintln(F("246.0 (3dB)  265.0 (Nyquist)"));
                break;
              case 1:
                SerialPrintln(F("246.0 (3dB)  265.0 (Nyquist)"));
                break;
              case 2:
                SerialPrintln(F("111.4 (3dB)  136.0 (Nyquist)"));
                break;
              case 3:
                SerialPrintln(F("50.4 (3dB)  68.8 (Nyquist)"));
                break;
              case 4:
                SerialPrintln(F("23.9 (3dB)  34.4 (Nyquist)"));
                break;
              case 5:
                SerialPrintln(F("11.5 (3dB)  17.0 (Nyquist)"));
                break;
              case 6:
                SerialPrintln(F("5.7 (3dB)  8.3 (Nyquist)"));
                break;
              case 7:
                SerialPrintln(F("473 (3dB)  499 (Nyquist)"));
                break;
              default:
                SerialPrintln(F("UNKNOWN"));
                break;
            }
          }
          else
          {
            SerialPrintln(F("Disabled  (Bandwidth is 1209 Hz (3dB) 1248 Hz (Nyquist))"));
          }
  
          SerialPrint(F("9) Gyro Full Scale: +/- "));
          switch (settings.imuGyroFSS)
          {
            case 0:
              SerialPrintln(F("250dps"));
              break;
            case 1:
              SerialPrintln(F("500dps"));
              break;
            case 2:
              SerialPrintln(F("1000dps"));
              break;
            case 3:
              SerialPrintln(F("2000dps"));
              break;
            default:
              SerialPrintln(F("UNKNOWN"));
              break;
          }
  
          SerialPrint(F("10) Gyro Digital Low Pass Filter: "));
          if (settings.imuGyroDLPF)
          {
            SerialPrintln(F("Enabled"));
            SerialPrint(F("11) Gyro DLPF Bandwidth (Hz): "));
            switch (settings.imuGyroDLPFBW)
            {
              case 0:
                SerialPrintln(F("196.6 (3dB)  229.8 (Nyquist)"));
                break;
              case 1:
                SerialPrintln(F("151.8 (3dB)  187.6 (Nyquist)"));
                break;
              case 2:
                SerialPrintln(F("119.5 (3dB)  154.3 (Nyquist)"));
                break;
              case 3:
                SerialPrintln(F("51.2 (3dB)  73.3 (Nyquist)"));
                break;
              case 4:
                SerialPrintln(F("23.9 (3dB)  35.9 (Nyquist)"));
                break;
              case 5:
                SerialPrintln(F("11.6 (3dB)  17.8 (Nyquist)"));
                break;
              case 6:
                SerialPrintln(F("5.7 (3dB)  8.9 (Nyquist)"));
                break;
              case 7:
                SerialPrintln(F("361.4 (3dB)  376.5 (Nyquist)"));
                break;
              default:
                SerialPrintln(F("UNKNOWN"));
                break;
            }
          }
          else
          {
            SerialPrintln(F("Disabled  (Bandwidth is 12106 Hz (3dB) 12316 Hz (Nyquist))"));
          }
        }
      }

      SerialPrint(F("12) Digital Motion Processor (DMP): "));
      if (settings.imuUseDMP) SerialPrintln(F("Enabled"));
      else SerialPrintln(F("Disabled"));

      if (settings.imuUseDMP == true)
      {
        SerialPrint(F("13) Game Rotation Vector (Quat6) Logging: "));
        if (settings.imuLogDMPQuat6) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
        SerialPrint(F("14) Rotation Vector (Quat9) Logging: "));
        if (settings.imuLogDMPQuat9) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
        SerialPrint(F("15) Accelerometer Logging: "));
        if (settings.imuLogDMPAccel) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
        SerialPrint(F("16) Gyro Logging: "));
        if (settings.imuLogDMPGyro) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
        SerialPrint(F("17) Compass Logging: "));
        if (settings.imuLogDMPCpass) SerialPrintln(F("Enabled"));
        else SerialPrintln(F("Disabled"));
      }
    }
    
    SerialPrintln(F("x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after x seconds

    if (incoming == 1)
    {
      settings.enableIMU ^= 1;
      if (settings.enableIMU == true) beginIMU();
      else online.IMU = false;
    }
    else if (settings.enableIMU == true)
    {
      if (settings.imuUseDMP == false)
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
          SerialPrintln(F("Enter Accelerometer Full Scale (0 to 3): "));
          SerialPrintln(F("0: +/- 2g"));
          SerialPrintln(F("1: +/- 4g"));
          SerialPrintln(F("2: +/- 8g"));
          SerialPrintln(F("3: +/- 16g"));
          int afs = getNumber(menuTimeout); //x second timeout
          if (afs < 0 || afs > 3)
            SerialPrintln(F("Error: Out of range"));
          else
          {
            settings.imuAccFSS = afs;
            ICM_20948_fss_t FSS;
            FSS.a = settings.imuAccFSS;
            FSS.g = settings.imuGyroFSS;
            ICM_20948_Status_e retval = myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), FSS);
            if (retval != ICM_20948_Stat_Ok)
            {
              SerialPrintln(F("Error: Could not configure the IMU!"));
            }
          }
        }
        else if ((incoming == 7) && (online.IMU == true))
        {
          settings.imuAccDLPF ^= 1;
          ICM_20948_Status_e retval = myICM.enableDLPF(ICM_20948_Internal_Acc, settings.imuAccDLPF);
          if (retval != ICM_20948_Stat_Ok)
          {
            SerialPrintln(F("Error: Could not configure the IMU!"));
          }
        }
        else if ((incoming == 8) && (online.IMU == true) && (settings.imuAccDLPF == true))
        {
          SerialPrintln(F("Enter Accelerometer DLPF Bandwidth (0 to 7): "));
          SerialPrintln(F("0: 246.0 (3dB)  265.0 (Nyquist) (Hz)"));
          SerialPrintln(F("1: 246.0 (3dB)  265.0 (Nyquist) (Hz)"));
          SerialPrintln(F("2: 111.4 (3dB)  136.0 (Nyquist) (Hz)"));
          SerialPrintln(F("3: 50.4  (3dB)  68.8  (Nyquist) (Hz)"));
          SerialPrintln(F("4: 23.9  (3dB)  34.4  (Nyquist) (Hz)"));
          SerialPrintln(F("5: 11.5  (3dB)  17.0  (Nyquist) (Hz)"));
          SerialPrintln(F("6: 5.7   (3dB)  8.3   (Nyquist) (Hz)"));
          SerialPrintln(F("7: 473   (3dB)  499   (Nyquist) (Hz)"));
          int afbw = getNumber(menuTimeout); //x second timeout
          if (afbw < 0 || afbw > 7)
            SerialPrintln(F("Error: Out of range"));
          else
          {
            settings.imuAccDLPFBW = afbw;
            ICM_20948_dlpcfg_t dlpcfg;
            dlpcfg.a = settings.imuAccDLPFBW;
            dlpcfg.g = settings.imuGyroDLPFBW;
            ICM_20948_Status_e retval = myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), dlpcfg);
            if (retval != ICM_20948_Stat_Ok)
            {
              SerialPrintln(F("Error: Could not configure the IMU!"));
            }
          }
        }
        else if ((incoming == 9) && (online.IMU == true))
        {
          SerialPrintln(F("Enter Gyro Full Scale (0 to 3): "));
          SerialPrintln(F("0: +/- 250dps"));
          SerialPrintln(F("1: +/- 500dps"));
          SerialPrintln(F("2: +/- 1000dps"));
          SerialPrintln(F("3: +/- 2000dps"));
          int gfs = getNumber(menuTimeout); //x second timeout
          if (gfs < 0 || gfs > 3)
            SerialPrintln(F("Error: Out of range"));
          else
          {
            settings.imuGyroFSS = gfs;
            ICM_20948_fss_t FSS;
            FSS.a = settings.imuAccFSS;
            FSS.g = settings.imuGyroFSS;
            ICM_20948_Status_e retval = myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), FSS);
            if (retval != ICM_20948_Stat_Ok)
            {
              SerialPrintln(F("Error: Could not configure the IMU!"));
            }
          }
        }
        else if ((incoming == 10) && (online.IMU == true))
        {
          settings.imuGyroDLPF ^= 1;
          ICM_20948_Status_e retval = myICM.enableDLPF(ICM_20948_Internal_Gyr, settings.imuGyroDLPF);
          if (retval != ICM_20948_Stat_Ok)
          {
            SerialPrintln(F("Error: Could not configure the IMU!"));
          }
        }
        else if ((incoming == 11) && (online.IMU == true) && (settings.imuGyroDLPF == true))
        {
          SerialPrintln(F("Enter Gyro DLPF Bandwidth (0 to 7): "));
          SerialPrintln(F("0: 196.6 (3dB)  229.8 (Nyquist) (Hz)"));
          SerialPrintln(F("1: 151.8 (3dB)  187.6 (Nyquist) (Hz)"));
          SerialPrintln(F("2: 119.5 (3dB)  154.3 (Nyquist) (Hz)"));
          SerialPrintln(F("3: 51.2  (3dB)  73.3  (Nyquist) (Hz)"));
          SerialPrintln(F("4: 23.9  (3dB)  35.9  (Nyquist) (Hz)"));
          SerialPrintln(F("5: 11.6  (3dB)  17.8  (Nyquist) (Hz)"));
          SerialPrintln(F("6: 5.7   (3dB)  8.9   (Nyquist) (Hz)"));
          SerialPrintln(F("7: 361.4 (3dB)  376.5 (Nyquist) (Hz)"));
          int gfbw = getNumber(menuTimeout); //x second timeout
          if (gfbw < 0 || gfbw > 7)
            SerialPrintln(F("Error: Out of range"));
          else
          {
            settings.imuGyroDLPFBW = gfbw;
            ICM_20948_dlpcfg_t dlpcfg;
            dlpcfg.a = settings.imuAccDLPFBW;
            dlpcfg.g = settings.imuGyroDLPFBW;
            ICM_20948_Status_e retval = myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), dlpcfg);
            if (retval != ICM_20948_Stat_Ok)
            {
              SerialPrintln(F("Error: Could not configure the IMU!"));
            }
          }
        }
        else if (incoming == 12)
        {
          settings.imuUseDMP ^= 1;
          restartIMU = true;
        }
        else if (incoming == STATUS_PRESSED_X)
          break;
        else if (incoming == STATUS_GETNUMBER_TIMEOUT)
          break;
        else
          printUnknown(incoming);
      }
      else if (settings.imuUseDMP == true)
      {
        if (incoming == 12)
        {
          settings.imuUseDMP ^= 1;
          restartIMU = true;
        }
        else if (incoming == 13)
        {
          if (settings.imuLogDMPQuat6 == true)
          {
            settings.imuLogDMPQuat6 = false;
          }
          else
          {
            settings.imuLogDMPQuat6 = true;
            settings.imuLogDMPQuat9 = false;
          }
          restartIMU = true;
        }
        else if (incoming == 14)
        {
          if (settings.imuLogDMPQuat9 == true)
          {
            settings.imuLogDMPQuat9 = false;
          }
          else
          {
            settings.imuLogDMPQuat9 = true;
            settings.imuLogDMPQuat6 = false;
          }
          restartIMU = true;
        }
        else if (incoming == 15)
        {
          settings.imuLogDMPAccel ^= 1;
          restartIMU = true;
        }
        else if (incoming == 16)
        {
          settings.imuLogDMPGyro ^= 1;
          restartIMU = true;
        }
        else if (incoming == 17)
        {
          settings.imuLogDMPCpass ^= 1;
          restartIMU = true;
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
    else if (incoming == STATUS_PRESSED_X)
      break;
    else if (incoming == STATUS_GETNUMBER_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
  return (restartIMU);
}
