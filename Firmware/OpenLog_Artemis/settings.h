#include "SparkFun_PHT_MS8607_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_PHT_MS8607
#include "SparkFun_ISM330DHCX.h" // Click here to get the library: http://librarymanager/All#SparkFun_6DoF_ISM330DHCX

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
  DEVICE_ADC_ADS122C04,
  DEVICE_PRESSURE_MPR0025PA1, // 0-25 PSI, I2C Address 0x18
  DEVICE_PARTICLE_SNGCJA5,
  DEVICE_VOC_SGP40,
  DEVICE_PRESSURE_SDP3X,
  DEVICE_PRESSURE_MS5837,
  DEVICE_QWIIC_BUTTON,
  DEVICE_BIO_SENSOR_HUB,
  DEVICE_ISM330DHCX,
  DEVICE_MMC5983MA,
  DEVICE_KX134,
  DEVICE_ADS1015,
  DEVICE_PRESSURE_LPS28DFW,
  DEVICE_LIGHT_VEML7700,

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

const unsigned long worstCaseQwiicPowerOnDelay = 1000; // Remember to update this if required when adding a new sensor (currently defined by the u-blox). (This is OK for the SCD30. beginQwiicDevices will extend the delay.)
const unsigned long minimumQwiicPowerOnDelay = 10; //The minimum number of milliseconds between turning on the Qwiic power and attempting to communicate with Qwiic devices

// Power On Delay Testing:
// The minimum power-on delays for following sensors have been checked:
// Each sensor has been checked twice:
//   Fast case: as the only sensor linked to the OLA; IMU logging disabled
//   Slow case: when linked to the OLA along with a ZED-F9P (representing a heavy load on the Qwiic power regulator); IMU logging enabled.
// (Note: the delay is the value set via the "minimum Qwiic bus power up delay" menu option. It may not represent the actual sensor power-up delay, but it is the number the OLA needs to know.)
// (Note: the slow case is more about how fast the Qwiic power ramps up with a heavy load attached.)
//              Fast    Slow
// Multiplexer: 10ms    10ms
// LPS25HB:     10ms    10ms
// NAU7802:     10ms    10ms
// SHTC3:       10ms    10ms
// MS8607:      10ms    10ms
// SGP30:       10ms    10ms
// ADS122C04:   10ms    10ms
// SCD30:       5000ms  (Shorter delays seem to result in invalid CO2 readings?)
// u-blox:      1000ms

struct struct_multiplexer {
  //Ignore certain ports at detection step?
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

//Add the new sensor settings below
struct struct_LPS25HB {
  bool log = true;
  bool logPressure = true;
  bool logTemperature = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_LPS28DFW {
  bool log = true;
  bool logPressure = true;
  bool logTemperature = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_NAU7802 {
  bool log = true;
  float calibrationFactor = 1.0; //Value used to convert the load cell reading to lbs or kg. Default to 1 to avoid divide-by-zero
  int32_t zeroOffset = 0; //Zero value that is found when scale is tared. Default to 0 so we get raw readings
  int decimalPlaces = 2;
  int averageAmount = 4; //Number of readings to take per getWeight call
  float calibrationWeight = 1.0; //Default scale calibration weight. User can adjust via the menu
  int sampleRate = 3; //Library says possible values are: 10(0), 20(1), 40(2), 80(3), 320(7)
  int gain = 7; //Library says possible values are: 1(0), 2(1), 4(2), 8(3), 16(4), 32(5), 64(6), 128(7)
  int LDO = 5; //LDO voltage. Only 3.3(4), 3.0(5), 2.7(6), 2.4(7) make sense here
  int calibrationMode = 1; //0: None; 1: Use CALMOD / CALS (calibrateAFE) Internal; 2: Use CALMOD / CALS (calibrateAFE) External
  int32_t offsetReg = 0; //Value for the NAU7802 offset register
  uint32_t gainReg = 0x00800000; //Value for the NAU7802 gain register
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_MCP9600 {
  bool log = true;
  bool logTemperature= true;
  bool logAmbientTemperature = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
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
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_ublox {
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
  uint32_t i2cSpeed = 100000; //Default to 100kHz for least number of CRC issues
  unsigned long powerOnDelayMillis = 1000; // Wait for at least this many millis before communicating with this device
  bool useAutoPVT = false; // Use autoPVT - to allow data collection at rates faster than GPS
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
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
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
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_CCS811 {
  bool log = true;
  bool logTVOC = true;
  bool logCO2 = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_BME280 {
  bool log = true;
  bool logHumidity = true;
  bool logPressure = true;
  bool logAltitude = true;
  bool logTemperature = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_SGP30 {
  bool log = true;
  bool logTVOC = true;
  bool logCO2 = true;
  bool logH2 = true;
  bool logEthanol = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_VEML6075 {
  bool log = true;
  bool logUVA = true;
  bool logUVB = true;
  bool logUVIndex = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_VEML7700 {
  bool log = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_MS5637 {
  bool log = true;
  bool logPressure = true;
  bool logTemperature= true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_SCD30 {
  bool log = true;
  bool logCO2 = true;
  bool logHumidity = true;
  bool logTemperature = true;
  bool applyCalibrationConcentration = false;
  int measurementInterval = 2; //2 seconds
  int altitudeCompensation = 0; //0 m above sea level
  int ambientPressure = 1000; //mBar STP (Toto, I have a feeling we're not in Boulder anymore)
  int temperatureOffset = 0; //C - Be careful not to overwrite the value on the sensor
  int calibrationConcentration = 400; //ppm CO2 - Must be applied 2+ minutes after sensor running in chamber of calibration gas 
  unsigned long powerOnDelayMillis = 5000; // Wait for at least this many millis before communicating with this device
};

struct struct_MS8607 {
  bool log = true;
  bool logHumidity = true;
  bool logPressure = true;
  bool logTemperature = true;
  bool enableHeater = false; // The TE examples say that get_compensated_humidity and get_dew_point will only work if the heater is OFF
  MS8607_pressure_resolution pressureResolution = MS8607_pressure_resolution_osr_8192; //17ms per reading, 0.016mbar resolution
  MS8607_humidity_resolution humidityResolution = MS8607_humidity_resolution_12b; //12-bit
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_AHT20 {
  bool log = true;
  bool logHumidity = true;
  bool logTemperature = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_SHTC3 {
  bool log = true;
  bool logHumidity = true;
  bool logTemperature = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_ADS122C04 {
  bool log = true;
  bool logCentigrade = true;
  bool logFahrenheit = false;
  bool logInternalTemperature = true;
  bool logRawVoltage = false;
  bool useFourWireMode = true;
  bool useThreeWireMode = false;
  bool useTwoWireMode = false;
  bool useFourWireHighTemperatureMode = false;
  bool useThreeWireHighTemperatureMode = false;
  bool useTwoWireHighTemperatureMode = false;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_MPR0025PA1 {
  bool log = true;
  int minimumPSI = 0;
  int maximumPSI = 25;
  bool usePSI = true;
  bool usePA = false;
  bool useKPA = false;
  bool useTORR = false;
  bool useINHG = false;
  bool useATM = false;
  bool useBAR = false;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_SNGCJA5 {
  bool log = true;
  bool logPM1 = true;
  bool logPM25 = true;
  bool logPM10 = true;
  bool logPC05 = true;
  bool logPC1 = true;
  bool logPC25 = true;
  bool logPC50 = true;
  bool logPC75 = true;
  bool logPC10 = true;
  bool logSensorStatus = false;
  bool logPDStatus = true;
  bool logLDStatus = true;
  bool logFanStatus = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_SGP40 {
  bool log = true;
  bool logVOC = true;
  float RH = 50;
  float T = 25;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_SDP3X {
  bool log = true;
  bool logPressure = true;
  bool logTemperature = true;
  bool massFlow = true;
  bool averaging = false;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_MS5837 {
  bool log = true;
  bool logPressure = true;
  bool logTemperature = true;
  bool logDepth = true;
  bool logAltitude = true;
  uint8_t model = 1; // Valid options are: 0 (MS5837::MS5837_30BA); 1 (MS5837::MS5837_02BA) and 255 (MS5837::MS5837_UNRECOGNISED)
  float fluidDensity = 997;
  float conversion = 1.0;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_QWIIC_BUTTON {
  bool log = true;
  bool logPressed = true;
  bool logClicked = true;
  bool toggleLEDOnClick = true;
  bool ledState = false; // Do not store in NVM
  uint8_t ledBrightness = 255;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_BIO_SENSOR_HUB {
  bool log = true;
  bool logHeartrate = true;
  bool logConfidence = true;
  bool logOxygen = true;
  bool logStatus = true;
  bool logExtendedStatus = true;
  bool logRValue = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_ISM330DHCX {
  bool log = true;
  bool logAccel = true;
  bool logGyro = true;
  bool logDataReady = true;
  //Accelerometer Full Scale
  //#define ISM_2g    0
  //#define ISM_16g   1
  //#define ISM_4g    2
  //#define ISM_8g    3
  int accelScale = ISM_4g;
  //Acceleromter Output Data Rate
  //#define ISM_XL_ODR_OFF     0
  //#define ISM_XL_ODR_12Hz5   1
  //#define ISM_XL_ODR_26Hz    2
  //#define ISM_XL_ODR_52Hz    3
  //#define ISM_XL_ODR_104Hz   4
  //#define ISM_XL_ODR_208Hz   5
  //#define ISM_XL_ODR_416Hz   6
  //#define ISM_XL_ODR_833Hz   7
  //#define ISM_XL_ODR_1666Hz  8
  //#define ISM_XL_ODR_3332Hz  9
  //#define ISM_XL_ODR_6667Hz  10
  //#define ISM_XL_ODR_1Hz6    11
  int accelRate = ISM_XL_ODR_208Hz;
  bool accelFilterLP2 = true;
  //Accel Regular Performance Filter Settings
  //#define ISM_HP_PATH_DISABLE_ON_OUT  0x00
  //#define ISM_SLOPE_ODR_DIV_4         0x10
  //#define ISM_HP_ODR_DIV_10           0x11
  //#define ISM_HP_ODR_DIV_20           0x12
  //#define ISM_HP_ODR_DIV_45           0x13
  //#define ISM_HP_ODR_DIV_100          0x14
  //#define ISM_HP_ODR_DIV_200          0x15
  //#define ISM_HP_ODR_DIV_400          0x16
  //#define ISM_HP_ODR_DIV_800          0x17
  //#define ISM_HP_REF_MD_ODR_DIV_10    0x31
  //#define ISM_HP_REF_MD_ODR_DIV_20    0x32
  //#define ISM_HP_REF_MD_ODR_DIV_45    0x33
  //#define ISM_HP_REF_MD_ODR_DIV_100   0x34
  //#define ISM_HP_REF_MD_ODR_DIV_200   0x35
  //#define ISM_HP_REF_MD_ODR_DIV_400   0x36
  //#define ISM_HP_REF_MD_ODR_DIV_800   0x37
  //#define ISM_LP_ODR_DIV_10           0x01
  //#define ISM_LP_ODR_DIV_20           0x02
  //#define ISM_LP_ODR_DIV_45           0x03
  //#define ISM_LP_ODR_DIV_100          0x04
  //#define ISM_LP_ODR_DIV_200          0x05
  //#define ISM_LP_ODR_DIV_400          0x06
  //#define ISM_LP_ODR_DIV_800          0x07
  int accelSlopeFilter = ISM_LP_ODR_DIV_100;
  //Gyroscope Full Scale
  //#define ISM_125dps    2
  //#define ISM_250dps    0
  //#define ISM_500dps    4
  //#define ISM_1000dps   8
  //#define ISM_2000dps   12
  //#define ISM_4000dps   1
  int gyroScale = ISM_250dps;
  //Gyroscope Output Data Rate
  //#define ISM_GY_ODR_OFF     0
  //#define ISM_GY_ODR_12Hz    1
  //#define ISM_GY_ODR_26Hz    2
  //#define ISM_GY_ODR_52Hz    3
  //#define ISM_GY_ODR_104Hz   4
  //#define ISM_GY_ODR_208Hz   5
  //#define ISM_GY_ODR_416Hz   6
  //#define ISM_GY_ODR_833Hz   7
  //#define ISM_GY_ODR_1666Hz  8
  //#define ISM_GY_ODR_3332Hz  9
  //#define ISM_GY_ODR_6667Hz  10
  int gyroRate = ISM_GY_ODR_208Hz;
  bool gyroFilterLP1 = true;
  //Gyro Bandwidth set
  //#define ISM_ULTRA_LIGHT   0
  //#define ISM_VERY_LIGHT    1
  //#define ISM_LIGHT         2
  //#define ISM_MEDIUM        3
  //#define ISM_STRONG        4
  //#define ISM_VERY_STRONG   5
  //#define ISM_AGGRESSIVE    6
  //#define ISM_XTREME        7
  int gyroLP1BW = ISM_MEDIUM;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_MMC5983MA {
  bool log = true;
  bool logMag = true;
  bool logTemperature = true;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_KX134 {
  bool log = true;
  bool logAccel = true;
  bool logDataReady = true;
  bool range8G = false;
  bool range16G = false;
  bool range32G = false;
  bool range64G = true;
  bool highSpeed = false;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};

struct struct_ADS1015 {
  bool log = true;
  bool logA0 = true;
  bool logA1 = true;
  bool logA2 = true;
  bool logA3 = true;
  bool logA0A1 = false;
  bool logA0A3 = false;
  bool logA1A3 = false;
  bool logA2A3 = false;
  bool gain23 = false;
  bool gain1 = true;
  bool gain2 = false;
  bool gain4 = false;
  bool gain8 = false;
  bool gain16 = false;
  unsigned long powerOnDelayMillis = minimumQwiicPowerOnDelay; // Wait for at least this many millis before communicating with this device. Increase if required!
};


//This is all the settings that can be set on OpenLog. It's recorded to NVM and the config file.
struct struct_settings {
  int sizeOfSettings = 0; //sizeOfSettings **must** be the first entry and must be int
  int olaIdentifier = OLA_IDENTIFIER; // olaIdentifier **must** be the second entry
  int nextSerialLogNumber = 1;
  int nextDataLogNumber = 1;
  //uint32_t: Largest is 4,294,967,295 or 4,294s or 71 minutes between readings.
  //uint64_t: Largest is 9,223,372,036,854,775,807 or 9,223,372,036,854s or 292,471 years between readings.
  uint64_t usBetweenReadings = 100000ULL; //Default to 100,000us = 100ms = 10 readings per second.
  //100,000 / 1000 = 100ms. 1 / 100ms = 10Hz
  //recordPerSecond (Hz) = 1 / ((usBetweenReadings / 1000UL) / 1000UL)
  //recordPerSecond (Hz) = 1,000,000 / usBetweenReadings
  bool logMaxRate = false;
  bool enableRTC = true;
  bool enableIMU = true;
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
  bool correctForDST = false;
  int dateStyle = 0; // 0 : mm/dd/yyyy, 1 : dd/mm/yyyy, 2 : yyyy/mm/dd, 3 : ISO8601 (date and time)
  bool hour24Style = true;
  int  serialTerminalBaudRate = 115200;
  int  serialLogBaudRate = 9600;
  bool showHelperText = true;
  bool logA11 = false;
  bool logA12 = false;
  bool logA13 = false;
  bool logA32 = false;
  bool logAnalogVoltages = true;
  float localUTCOffset = 0; // Default to UTC because we should. Support offsets in 15 minute increments.
  bool printDebugMessages = false;
#if(HARDWARE_VERSION_MAJOR == 0)
  bool powerDownQwiicBusBetweenReads = false; // For the SparkX (black) board: default to leaving the Qwiic power enabled during sleep and powerDown to prevent a brown-out.
#else
  bool powerDownQwiicBusBetweenReads = true; // For the SparkFun (red) board: default to disabling Qwiic power during sleep. (Qwiic power is always disabled during powerDown on v10 hardware.)
#endif
  uint32_t qwiicBusMaxSpeed = 100000; // 400kHz with no pull-ups can cause issues. Default to 100kHz. User can change to 400 if required.
  int  qwiicBusPowerUpDelayMs = 250; // This is the minimum delay between the qwiic bus power being turned on and communication with the qwiic devices being attempted
  bool printMeasurementCount = false;
  bool enablePwrLedDuringSleep = true;
  bool logVIN = false;
  unsigned long openNewLogFilesAfter = 0; //Default to 0 (Never) seconds
  float vinCorrectionFactor = 1.47; //Correction factor for the VIN measurement; to compensate for the divider impedance
  bool useGPIO32ForStopLogging = false; //If true, use GPIO as a stop logging button
  uint32_t qwiicBusPullUps = 1; //Default to 1.5k I2C pull-ups - internal to the Artemis
  bool outputSerial = false; // Output the sensor data on the TX pin
  uint8_t zmodemStartDelay = 20; // Wait for this many seconds before starting the zmodem file transfer
  bool enableLowBatteryDetection = false; // Low battery detection
  float lowBatteryThreshold = 3.4; // Low battery voltage threshold (Volts)
  bool frequentFileAccessTimestamps = false; // If true, the log file access timestamps are updated every 500ms
  bool useGPIO11ForTrigger = false; // If true, use GPIO to trigger sensor logging
  bool fallingEdgeTrigger = true; // Default to falling-edge triggering (If false, triggering will be rising-edge)
  bool imuAccDLPF = false; // IMU accelerometer Digital Low Pass Filter - default to disabled
  bool imuGyroDLPF = false; // IMU gyro Digital Low Pass Filter - default to disabled
  int imuAccFSS = 0; // IMU accelerometer full scale - default to gpm2 (ICM_20948_ACCEL_CONFIG_FS_SEL_e)
  int imuAccDLPFBW = 7; // IMU accelerometer DLPF bandwidth - default to acc_d473bw_n499bw (ICM_20948_ACCEL_CONFIG_DLPCFG_e)
  int imuGyroFSS = 0; // IMU gyro full scale - default to 250 degrees per second (ICM_20948_GYRO_CONFIG_1_FS_SEL_e)
  int imuGyroDLPFBW = 7; // IMU gyro DLPF bandwidth - default to gyr_d361bw4_n376bw5 (ICM_20948_GYRO_CONFIG_1_DLPCFG_e)
  bool logMicroseconds = false; // Log micros()
  bool useTxRxPinsForTerminal = false; // If true, the terminal is echo'd to the Tx and Rx pins. Note: setting this to true will _permanently_ disable serial logging and analog input on those pins!
  bool timestampSerial = false; // If true, the RTC time will be added to the serial log file when timeStampToken is received
  uint8_t timeStampToken = 0x0A; // Add RTC time to the serial log when this token is received. Default to Line Feed (0x0A). Suggested by @DennisMelamed in Issue #63
  bool useGPIO11ForFastSlowLogging = false; // If true, Pin 11 will control if readings are taken quickly or slowly. Suggested by @ryanneve in Issue #46 and PR #64
  bool slowLoggingWhenPin11Is = false; // Controls the polarity of Pin 11 for fast / slow logging
  bool useRTCForFastSlowLogging = false; // If true, logging will be slow during the specified times
  int slowLoggingIntervalSeconds = 300; // Slow logging interval in seconds. Default to 5 mins
  int slowLoggingStartMOD = 1260; // Start slow logging at this many Minutes Of Day. Default to 21:00
  int slowLoggingStopMOD = 420; // Stop slow logging at this many Minutes Of Day. Default to 07:00
  bool resetOnZeroDeviceCount = false; // A work-around for I2C bus crashes. Enable this via the debug menu.
  bool imuUseDMP = false; // If true, enable the DMP
  bool imuLogDMPQuat6 = true; // If true, log INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR (Quat6)
  bool imuLogDMPQuat9 = false; // If true, log INV_ICM20948_SENSOR_ROTATION_VECTOR (Quat9 + Heading Accuracy)
  bool imuLogDMPAccel = false; // If true, log INV_ICM20948_SENSOR_RAW_ACCELEROMETER
  bool imuLogDMPGyro = false; // If true, log INV_ICM20948_SENSOR_RAW_GYROSCOPE
  bool imuLogDMPCpass = false; // If true, log INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED
  unsigned long minimumAwakeTimeMillis = 0; // Set to greater than zero to keep the Artemis awake for this long between sleeps
  bool identifyBioSensorHubs = false; // If true, Bio Sensor Hubs (Pulse Oximeters) will be included in autoDetect (requires exclusive use of pins 32 and 11)
  bool serialTxRxDuringSleep = false; // If true, the Serial Tx and Rx pins are left enabled during sleep - to prevent the COM port reinitializing
  bool printGNSSDebugMessages = false; // If true, the GNSS library debug messages are enabled
  bool openMenuWithPrintable = false; // If true, only a printable char can open the main menu. Chars < 9 (Tab) are ignored
} settings;

//These are the devices on board OpenLog that may be on or offline.
struct struct_online {
  bool microSD = false;
  bool dataLogging = false;
  bool serialLogging = false;
  bool IMU = false;
  bool serialOutput = false;
} online;
