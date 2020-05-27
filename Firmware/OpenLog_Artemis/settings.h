//Needed for the MS8607 struct below
#include "MS8607_Library.h" //Click here to get the library: http://librarymanager/All#Qwiic_MS8607

typedef enum
{
  DEVICE_MULTIPLEXER = 0,
  DEVICE_LOADCELL_NAU7802,
  DEVICE_DISTANCE_VL53L1X,
  DEVICE_GPS_UBLOX,
  DEVICE_PROXIMITY_VCNL4040,
  DEVICE_TEMPERATURE_TMP117,
  DEVICE_PRESSURE_MS5637,
  DEVICE_PRESSURE_LPS25HB,
  DEVICE_PHT_BME280,
  DEVICE_UV_VEML6075,
  DEVICE_VOC_CCS811,
  DEVICE_VOC_SGP30,
  DEVICE_CO2_SCD30,
  DEVICE_PHT_MS8607,
  DEVICE_TEMPERATURE_MCP9600,
  DEVICE_HUMIDITY_AHT20,
  DEVICE_HUMIDITY_SHTC3,

  DEVICE_TOTAL_DEVICES, //Marks the end, used to iterate loops
  DEVICE_UNKNOWN_DEVICE,
} deviceType_e;

struct node
{
  deviceType_e deviceType;

  uint8_t address = 0;
  uint8_t portNumber = 0;
  uint8_t muxAddress = 0;
  bool online = false; //Goes true once successfully begin()'d

  void *classPtr; //Pointer to this devices' class instantiation
  void *configPtr; //The struct containing this devices logging options
  node *next;
};

node *head = NULL;
node *tail = NULL;

typedef void (*FunctionPointer)(void*); //Used for pointing to device config menus

//Begin specific sensor config structs
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

struct struct_multiplexer {
  //There is nothing about a multiplexer that we want to configure
  //Ignore certain ports at detection step?
};

//Add the new sensor settings below
struct struct_LPS25HB {
  bool log = true;
  bool logPressure = true;
  bool logTemperature = true;
};

struct struct_NAU7802 {
  bool log = true;
  float calibrationFactor = 0; //Value used to convert the load cell reading to lbs or kg
  long zeroOffset = 1000; //Zero value that is found when scale is tared. Default to 1000 so we don't get inf.
  int decimalPlaces = 2;
  int averageAmount = 4; //Number of readings to take per getWeight call
};

struct struct_MCP9600 {
  bool log = true;
  bool logTemperature= true;
  bool logAmbientTemperature = true;
};

struct struct_VCNL4040 {
  bool log = true;
  bool logProximity = true;
  bool logAmbientLight = true;
  int LEDCurrent = 200; //Highest LED current
  int IRDutyCycle = 40; //Highest duty cycle
  int proximityIntegrationTime = 8;
  int ambientIntegrationTime = 80;
  int resolution = 16; //Set to 16 bit output
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
  int i2cSpeed = 100000; //Default to 100kHz for least number of CRC issues
};

#define VL53L1X_DISTANCE_MODE_SHORT 0
#define VL53L1X_DISTANCE_MODE_LONG 1
struct struct_VL53L1X {
  bool log = true;
  bool logDistance = true;
  bool logRangeStatus = true;
  bool logSignalRate = false;
  byte distanceMode = VL53L1X_DISTANCE_MODE_LONG;
  int intermeasurementPeriod = 140; //ms
  int offset = 0; //In mm
  int crosstalk = 0;
};

#define TMP117_MODE_CONTINUOUS 0
#define TMP117_MODE_SHUTDOWN 1
#define TMP117_MODE_ONESHOT 2
struct struct_TMP117 {
  bool log = true;
  bool logTemperature= true;
  int conversionMode = TMP117_MODE_CONTINUOUS;
  int conversionAverageMode = 0; //Setup for 15.5ms reads
  int conversionCycle = 0;
};

struct struct_CCS811 {
  bool log = true;
  bool logTVOC = true;
  bool logCO2 = true;
};

struct struct_BME280 {
  bool log = true;
  bool logHumidity = true;
  bool logPressure = true;
  bool logAltitude = true;
  bool logTemperature = true;
};

struct struct_SGP30 {
  bool log = true;
  bool logTVOC = true;
  bool logCO2 = true;
  bool logH2 = true;
  bool logEthanol = true;
};

struct struct_VEML6075 {
  bool log = true;
  bool logUVA = true;
  bool logUVB = true;
  bool logUVIndex = true;
};

struct struct_MS5637 {
  bool log = true;
  bool logPressure = true;
  bool logTemperature= true;
};

struct struct_SCD30 {
  bool log = true;
  bool logCO2 = true;
  bool logHumidity = true;
  bool logTemperature = true;
  int measurementInterval = 2; //2 seconds
  int altitudeCompensation = 0; //0 m above sea level
  int ambientPressure = 835; //mBar STP
  int temperatureOffset = 0; //C - Be careful not to overwrite the value on the sensor
};

struct struct_MS8607 {
  bool log = true;
  bool logHumidity = true;
  bool logPressure = true;
  bool logTemperature = true;
  bool enableHeater = false; // The TE examples say that get_compensated_humidity and get_dew_point will only work if the heater is OFF
  MS8607_pressure_resolution pressureResolution = MS8607_pressure_resolution_osr_8192; //17ms per reading, 0.016mbar resolution
  MS8607_humidity_resolution humidityResolution = MS8607_humidity_resolution_12b; //12-bit
};

struct struct_AHT20 {
  bool log = true;
  bool logHumidity = true;
  bool logTemperature = true;
};

struct struct_SHTC3 {
  bool log = true;
  bool logHumidity = true;
  bool logTemperature = true;
};

//This is all the settings that can be set on OpenLog. It's recorded to NVM and the config file.
struct struct_settings {
  int sizeOfSettings = 0;
  int nextSerialLogNumber = 1;
  int nextDataLogNumber = 1;
  //unsigned int recordPerSecond = 10;
  //uint32_t: Largest is 4,294,967,295 or 4,294s or 71 minutes between readings.
  //uint64_t: Largest is 9,223,372,036,854,775,807 or 9,223,372,036,854s or 292,471 years between readings.
  uint64_t usBetweenReadings = 100000; //100,000us = 100ms = 10 readings per second.
  //100,000 / 1000 = 100ms. 1 / 100ms = 10Hz
  //recordPerSecond (Hz) = 1 / ((usBetweenReadings / 1000UL) / 1000UL)
  //recordPerSecond (Hz) = 1,000,000 / usBetweenReadings
  bool logMaxRate = false;
  bool enableRTC = true;
  bool enableIMU = true;
  bool enableSD = true;
  bool enableTerminalOutput = true;
  bool logDate = true;
  bool logTime = true;
  bool logData = true;
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
  int serialTerminalBaudRate = 115200;
  int serialLogBaudRate = 9600;
  bool showHelperText = true;
  bool logA11 = false;
  bool logA12 = false;
  bool logA13 = false;
  bool logA32 = false;
  bool logAnalogVoltages = true;
  int localUTCOffset = -7; //Default to Denver because we can
  bool printDebugMessages = false;
  bool powerDownQwiicBusBetweenReads = true;
  int qwiicBusMaxSpeed = 400000;
  int qwiicBusPowerUpDelayMs = 250;
  bool printMeasurementCount = false;
} settings;

//These are the devices on board OpenLog that may be on or offline.
struct struct_online {
  bool microSD = false;
  bool dataLogging = false;
  bool serialLogging = false;
  bool IMU = false;
} online;
