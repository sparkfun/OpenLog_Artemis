//Add the new sensor settings below
struct struct_uBlox {
  bool log = true;
  bool logUBXNAVCLOCK = false;
  bool logUBXNAVHPPOSECEF = false;
  bool logUBXNAVHPPOSLLH = false;
  bool logUBXNAVODO = false;
  bool logUBXNAVPOSECEF = false;
  bool logUBXNAVPOSLLH = false;
  bool logUBXNAVPVT = true;
  bool logUBXNAVRELPOSNED = false;
  bool logUBXNAVTIMEUTC = false;
  bool logUBXNAVVELECEF = false;
  bool logUBXNAVVELNED = false;
  bool logUBXRXMRAWX = false;
  bool logUBXRXMSFRBX = false;
  int i2cSpeed = 400000; //Default to 400kHz
};

//This is all the settings that can be set on OpenLog. It's recorded to NVM and the config file.
struct struct_settings {
  int sizeOfSettings = 0;
  int nextSerialLogNumber = 1;
  int nextDataLogNumber = 1;
  uint64_t usBetweenReadings = 100000; //100,000us = 100ms = 10 readings per second.
  bool enableSD = true;
  bool enableTerminalOutput = true;
  bool logData = true;
  int serialTerminalBaudRate = 115200;
  bool showHelperText = true;
  bool printDebugMessages = false;
  bool powerDownQwiicBusBetweenReads = false;
  int qwiicBusMaxSpeed = 400000;
  struct_uBlox sensor_uBlox;
} settings;

//These are the devices on board OpenLog that may be on or offline.
struct struct_online {
  bool microSD = false;
  bool dataLogging = false;
  bool serialLogging = false;
  bool IMU = false;
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
