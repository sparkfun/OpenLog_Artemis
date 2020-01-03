//Add the new sensor settings below
struct struct_LPS25HB {
  bool log = true;
  bool logPressure = true;
  bool logTemp = true;
};

struct struct_NAU7802 {
  bool log = true;
  float calibrationFactor = 0; //Value used to convert the load cell reading to lbs or kg
  long zeroOffset = 1000; //Zero value that is found when scale is tared. Default to 1000 so we don't get inf.
  int decimalPlaces = 2;
};

struct struct_MCP9600 {
  bool log = true;
  bool logTemp = true;
};

struct struct_uBlox {
  bool log = true;
  bool logDate = true;
  bool logTime = true;
  bool logPosition = true;
  bool logAltitude = true;
  bool logAltitudeMSL = false;
  bool logSIV = true;
  bool logFixType = true;
  bool logCarrierSolution = false;
  bool logGroundSpeed = true;
  bool logHeadingOfMotion = true;
  bool logpDOP = true;
  bool logiTOW = false;
};

struct struct_VL53L1X {
  bool log = true;
  bool logDistance = true;
};

struct struct_BH1749NUC {
  bool log = true;
  bool logRed = true;
};

struct struct_settings {
  int nextSerialLogNumber = 1;
  int nextDataLogNumber = 1;
  unsigned int recordPerSecond = 10;
  bool logMaxRate = false;
  bool enableRTC = true;
  bool enableIMU = true;
  bool enableSD = true;
  bool enableTerminalOutput = true;
  bool logDate = true;
  bool logSerial = true;
  bool logIMUAccel = true;
  bool logIMUGyro = true;
  bool logIMUMag = true;
  bool logIMUTemp = true;
  bool logRTC = true;
  bool logHertz = true;
  bool getRTCfromGPS = false;
  bool correctForDST = false;
  bool americanDateStyle = true;
  bool hour24Style = true;
  int serialBaudRate = 9600;
  bool showHelperText = true;
  bool logA11 = false;
  bool logA12 = false;
  bool logA13 = false;
  bool logA32 = false;
  bool logAnalogVoltages = true;
  int localUTCOffset = -7; //Default to Denver because we can
  struct_LPS25HB sensor_LPS25HB;
  struct_uBlox sensor_uBlox;
  struct_VL53L1X sensor_VL53L1X;
  struct_NAU7802 sensor_NAU7802;
  struct_MCP9600 sensor_MCP9600;
  struct_BH1749NUC sensor_BH1749NUC;
} settings;

struct struct_online {
  bool microSD = false;
  bool serialLogging = false;
  bool IMU = false;
} online;

//Add a new sensor here and the two structs below
struct struct_QwiicSensors {
  bool LPS25HB;
  bool MCP9600;
  bool BH1749NUC;
  bool NAU7802;
  bool uBlox;
  bool VL53L1X;
};

struct_QwiicSensors qwiicAvailable = {
  .LPS25HB = false,
  .MCP9600 = false,
  .BH1749NUC = false,
  .NAU7802 = false,
  .uBlox = false,
  .VL53L1X = false,
};

struct_QwiicSensors qwiicOnline = {
  .LPS25HB = false,
  .MCP9600 = false,
  .BH1749NUC = false,
  .NAU7802 = false,
  .uBlox = false,
  .VL53L1X = false,
};
