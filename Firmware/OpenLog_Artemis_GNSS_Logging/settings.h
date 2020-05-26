//Sensor settings

// settings.sensor_uBlox.log     : indicates if the user has enabled uBlox logging. Set by menu 2, sub-menu 1, option 1. Possible redundancy with setings.logData as we only have one sensor.
// settings.enableSD             : defines if the SD card should be enabled by beginSD(). Only set in settings.h. Doesn't appear to be able to be changed by the user? Possibly redundant?
// settings.enableTerminalOutput : indicates if the user has enabled Terminal logging - i.e. should the UBX Class and ID be displayed for every frame. Set by menu 1, option 2.
// setings.logData               : indicates if the user has enabled SD card logging. Set by menu 1, option 1.
// online.microSD                : indicates if the SD card is ready for data logging. Set by beginSD().
// online.dataLogging            : indicates if the SD card log file is open and ready to receive data. Set by beginDataLogging().
// qwiicAvailable.uBlox          : indicates if there is a uBlox module connected. Set by detectQwiicDevices().
// qwiicOnline.uBlox             : indicates if the module has been configured, or needs to be configured. Set true by beginSensors().

//Default uBlox I2C address
#define ADR_UBLOX 0x42

//u-blox settings
struct struct_uBlox {
  bool log = true;
  bool powerManagement = true;
  bool logUBXNAVCLOCK = false;
  bool logUBXNAVHPPOSECEF = false;
  bool logUBXNAVHPPOSLLH = false;
  bool logUBXNAVODO = false;
  bool logUBXNAVPOSECEF = false;
  bool logUBXNAVPOSLLH = false;
  bool logUBXNAVPVT = true;
  bool logUBXNAVRELPOSNED = false;
  bool logUBXNAVSTATUS = false;
  bool logUBXNAVTIMEUTC = true;
  bool logUBXNAVVELECEF = false;
  bool logUBXNAVVELNED = false;
  bool logUBXRXMRAWX = false;
  bool logUBXRXMSFRBX = false;
  bool logUBXTIMTM2 = false;
  bool enableUSB = true;
  bool enableUART1 = true;
  bool enableUART2 = true;
  bool enableSPI = false;
  uint16_t minMeasIntervalGPS = 50; //Minimum measurement interval in ms when tracking GPS only (20Hz for ZED-F9P)
  uint16_t minMeasIntervalAll = 125; //Minimum measurement interval in ms when tracking all constallations (8Hz for ZED-F9P)
  //High-rate RAWX logging is a tricky thing. The ZED-F9P seems happy to log RAWX for all constellations slightly above 5Hz but only if the USB, UARTs and SPI are disabled.
  //I suspect it is more to do with not overloading the I2C bus, rather than not overloading the module core. RAWX frames can be over 2KB in size.
  //At 5Hz we are getting very close to overloading the I2C bus at 100kHz. TO DO: set this according to module type?
  uint16_t minMeasIntervalRAWXAll = 200; //Minimum measurement interval in ms when tracking all constallations and logging RAWX
  uint8_t ubloxI2Caddress = ADR_UBLOX; //Let's store this just in case we want to change it at some point with CFG-I2C-ADDRESS (0x20510001)
};

//This is all the settings that can be set on OpenLog. It's recorded to NVM and the config file.
struct struct_settings {
  int sizeOfSettings = 0; //sizeOfSettings **must** be the first entry
  int nextSerialLogNumber = 1;
  int nextDataLogNumber = 1;
  uint64_t usBetweenReadings = 1000000ULL; //1000,000us = 1000ms = 1 readings per second.
  uint64_t usLoggingDuration = 10000000ULL; //10,000,000us = 10s logging duration
  uint64_t usSleepDuration = 0ULL; //0us = do not sleep (continuous logging)
  bool openNewLogFile = true;
  bool enableSD = true;
  bool enableTerminalOutput = true;
  bool logData = true;
  int serialTerminalBaudRate = 115200;
  bool showHelperText = true;
  bool printMajorDebugMessages = false;
  bool printMinorDebugMessages = false;
  bool powerDownQwiicBusBetweenReads = false;
  int qwiicBusMaxSpeed = 400000;
  struct_uBlox sensor_uBlox;
} settings;

//These are the devices on board OpenLog that may be on or offline.
struct struct_online {
  bool microSD = false;
  bool dataLogging = false;
} online;

//These structs define supported sensors and if they are available and online(started).
struct struct_QwiicSensors {
  bool uBlox;
};

struct_QwiicSensors qwiicAvailable = {
  .uBlox = false,
};

struct_QwiicSensors qwiicOnline = {
  .uBlox = false,
};
