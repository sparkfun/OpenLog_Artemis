/*
  To add a new sensor to the system:

  Add the library and the class constructor in OpenLog_Artemis
  Add a struct_MCP9600 to settings.h - This will define what settings for the sensor we will control
  Add a 'struct_CCS811 sensor_CCS811;' line to struct_settings{} in settings.h - This will put the sensor settings into NVM
  Add device to the struct_QwiicSensors, qwiicAvailable, and qwiicOnline structs in settings.h - This will let OpenLog know it's online and such
  Add device to menuAttachedDevices list
  Add the device's I2C address to the detectQwiicDevices function - Make sure the device is properly recognized with a whoami function (ideally)
  Create a menuConfigure_LPS25HB() function int menuAttachedDevices - This is the config menu. Add all the features you want the user to be able to control
  Add a startup function for this sensor in Sensors - This will notify the user at startup if sensor is detect and made online.
  Add a harvesting function in Sensors - Get the data from the device
*/


#define MAX_NUMBER_OF_QWIIC_DEVICES 30

void menuAttachedDevices()
{
  while (1)
  {
    //TODO - Power on Qwiic Bus

    Serial.println();
    Serial.println(F("Menu: Configure GNSS Devices"));

    //See what's on the I2C bus. Will set the qwiicAvailable bools.
    if (detectQwiicDevices() == false)
      Serial.println(F("**No devices detected on Qwiic bus**"));

    //Create array of pointers to the configure functions
    typedef void(*FunctionPointer)();
    FunctionPointer functionPointers[MAX_NUMBER_OF_QWIIC_DEVICES]; //Array is 4 * dim = 120 bytes

    //If device is detected, connect the configure function
    int availableDevices = 1;
    if (qwiicAvailable.uBlox)
    {
      functionPointers[availableDevices - 1] = menuConfigure_uBlox;
      Serial.printf("%d) uBlox GNSS Receiver\n", availableDevices++);
    }

    functionPointers[availableDevices - 1] = menuConfigure_QwiicBus;
    Serial.printf("%d) Configure Qwiic Settings\n", availableDevices++);

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming >= '1' && incoming < ('1' + availableDevices - 1))
    {
      functionPointers[incoming - '0' - 1]();
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }
}

//Let's see what's on the I2C bus
bool detectQwiicDevices()
{
  bool somethingDetected = false;

  qwiic.setClock(100000); //During detection, go slow

  qwiic.setPullups(0); //Disable pull-ups as the u-blox modules have their own pull-ups

  //Depending on what hardware is configured, the Qwiic bus may have only been turned on a few ms ago
  //Give sensors, specifically those with a low I2C address, time to turn on
  delay(100); //SCD30 required >50ms to turn on

  uint8_t address = settings.sensor_uBlox.ubloxI2Caddress;
  
  qwiic.beginTransmission(address);
  if (qwiic.endTransmission() == 0)
  {
    if (settings.printMinorDebugMessages == true)
    {
      Serial.printf("Device found at address 0x%02X\n", address);
    }
    if (gpsSensor_ublox.begin(qwiic, address) == true) //Wire port, address
    {
      qwiicAvailable.uBlox = true;
      somethingDetected = true;
    }
  }

  return (somethingDetected);
}

void menuConfigure_QwiicBus()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure Qwiic Bus"));

    Serial.print(F("1) Turn off bus power when sleeping : "));
    if (settings.powerDownQwiicBusBetweenReads == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    Serial.print(F("2) Set Max Qwiic Bus Speed          : "));
    Serial.println(settings.qwiicBusMaxSpeed);

    Serial.println(F("x) Exit"));

    byte incoming = getByteChoice(menuTimeout); //Timeout after x seconds

    if (incoming == '1')
      settings.powerDownQwiicBusBetweenReads ^= 1;
    else if (incoming == '2')
    {
      Serial.print(F("Enter max frequency to run Qwiic bus: (100000 to 400000): "));
      int amt = getNumber(menuTimeout);
      if (amt >= 100000 && amt <= 400000)
        settings.qwiicBusMaxSpeed = amt;
      else
        Serial.println(F("Error: Out of range"));
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

}

void menuConfigure_uBlox()
{
  while (1)
  {
    Serial.println();
    Serial.println(F("Menu: Configure uBlox GPS Receiver"));

    Serial.print(F(" 1) Sensor Logging                                                             : "));
    if (settings.sensor_uBlox.log == true) Serial.println(F("Enabled"));
    else Serial.println(F("Disabled"));

    if (settings.sensor_uBlox.log == true)
    {
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

      Serial.print(F("18) Log UBX-NAV-TIMEUTC   (UTC Time Solution)                                  : "));
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

      if (minfo.HPG == true)
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
    Serial.println(F(" x) Exit"));

    int incoming = getNumber(menuTimeout); //Timeout after 10 seconds

    if (incoming == 1)
    {
      settings.sensor_uBlox.log ^= 1;
    }
    else if (settings.sensor_uBlox.log == true)
    {
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
      else if ((incoming == 50) && (minfo.HPG == true))
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
