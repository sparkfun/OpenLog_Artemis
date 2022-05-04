/*
  OpenLog Artemis
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 26th, 2019
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).
  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15793

  This firmware runs the OpenLog Artemis. A large variety of system settings can be
  adjusted by connecting at 115200bps.

  The Board should be set to SparkFun Apollo3 \ RedBoard Artemis ATP.

  v1.0 Power Consumption:
   Sleep between reads, RTC fully charged, no Qwiic, SD, no USB, no Power LED: 260uA
   10Hz logging IMU, no Qwiic, SD, no USB, no Power LED: 9-27mA

  TODO:
  (done) Create settings file for sensor. Load after qwiic bus is scanned.
  (done on larger Strings) Remove String dependencies.
  (done) Bubble sort list of devices.
  (done) Remove listing for muxes.
  (done) Verify the printing of all sensors is %f, %d correct
  (done) Add begin function seperate from everything, call after wakeup instead of detect
  (done) Add counter to output to look for memory leaks on long runs
  (done) Add AHT20 support
  (done) Add SHTC3 support
  (done) Change settings extension to txt
  (done) Fix max I2C speed to use linked list
  Currently device settings are not recorded to EEPROM, only deviceSettings.txt
  Is there a better way to dynamically create size of outputData array so we don't ever get larger than X sensors outputting?
  Find way to store device configs into EEPROM
  Log four pressure sensors and graph them on plotter
  (checked) Test GPS - not sure about %d with int32s. Does lat, long, and alt look correct?
  (done) Test NAU7802s
  (done) Test SCD30s (Add an extended delay for the SCD30. (Issue #5))
  (won't do?) Add a 'does not like to be powered cycled' setting for each device type. I think this has been superceded by "Add individual power-on delays for each sensor type?.
  (done) Add support for logging VIN
  (done) Investigate error in time between logs (https://github.com/sparkfun/OpenLog_Artemis/issues/13)
  (done) Invesigate RTC reset issue (https://github.com/sparkfun/OpenLog_Artemis/issues/13 + https://forum.sparkfun.com/viewtopic.php?f=123&t=53157)
    The solution is to make sure that the OLA goes into deep sleep as soon as the voltage monitor detects that the power has failed.
    The user will need to press the reset button once power has been restored. Using the WDT to check the monitor and do a POR wasn't reliable.
  (done) Investigate requires-reset issue on battery power (") (X04 + CCS811/BME280 enviro combo)
  (done) Add a fix so that the MS8607 does not also appear as an MS5637
  (done) Add "set RTC from GPS" functionality
  (done) Add UTCoffset functionality (including support for negative numbers)
  (done) Figure out how to give the u-blox time to establish a fix if it has been powered down between log intervals. The user can specify up to 60s for the Qwiic power-on delay.
  Add support for VREG_ENABLE
  (done) Add support for PWR_LED
  (done) Use the WDT to reset the Artemis when power is reconnected (previously the Artemis would have stayed in deep sleep)
  Add a callback function to the u-blox library so we can abort waiting for UBX data if the power goes low
  (done) Add support for the ADS122C04 ADC (Qwiic PT100)
  (done) Investigate why usBetweenReadings appears to be longer than expected. We needed to read millis _before_ enabling the lower power clock!
  (done) Correct u-blox pull-ups
  (done) Add an olaIdentifier to prevent problems when using two code variants that have the same sizeOfSettings
  (done) Add a fix for the IMU wake-up issue identified in https://github.com/sparkfun/OpenLog_Artemis/issues/18
  (done) Add a "stop logging" feature on GPIO 32: allow the pin to be used to read a stop logging button instead of being an analog input
  (done) Allow the user to set the default qwiic bus pull-up resistance (u-blox will still use 'none')
  (done) Add support for low battery monitoring using VIN
  (done) Output sensor data via the serial TX pin (Issue #32)
  (done) Add support for SD card file transfer (ZMODEM) and delete. (Issue #33) With thanks to: ecm-bitflipper (https://github.com/ecm-bitflipper/Arduino_ZModem)
  (done) Add file creation and access timestamps
  (done) Add the ability to trigger data collection via Pin 11 (Issue #36)
  (done) Correct the measurement count misbehaviour (Issue #31)
  (done) Use the corrected IMU temperature calculation (Issue #28)
  (done) Add individual power-on delays for each sensor type. Add an extended delay for the SCD30. (Issue #5)
  (done) v1.7: Fix readVin after sleep bug: https://github.com/sparkfun/OpenLog_Artemis/issues/39
  (done) Change detectQwiicDevices so that the MCP9600 (Qwiic Thermocouple) is detected correctly
  (done) Add support for the MPRLS0025PA micro pressure sensor
  (done) Add support for the SN-GCJA5 particle sensor
  (done) Add IMU accelerometer and gyro full scale and digital low pass filter settings to menuIMU
  (done) Add a fix to make sure the MS8607 is detected correctly: https://github.com/sparkfun/OpenLog_Artemis/issues/54
  (done) Add logMicroseconds: https://github.com/sparkfun/OpenLog_Artemis/issues/49
  (done) Add an option to use autoPVT when logging GNSS data: https://github.com/sparkfun/OpenLog_Artemis/issues/50
  (done) Corrected an issue when using multiple MS8607's: https://github.com/sparkfun/OpenLog_Artemis/issues/62
  (done) Add a feature to use the TX and RX pins as a duplicate Terminal
  (done) Add serial log timestamps with a token (as suggested by @DennisMelamed in PR https://github.com/sparkfun/OpenLog_Artemis/pull/70 and Issue https://github.com/sparkfun/OpenLog_Artemis/issues/63)
  (done) Add "sleep on pin" functionality based @ryanneve's PR https://github.com/sparkfun/OpenLog_Artemis/pull/64 and Issue https://github.com/sparkfun/OpenLog_Artemis/issues/46
  (done) Add "wake at specified times" functionality based on Issue https://github.com/sparkfun/OpenLog_Artemis/issues/46
  (done) Add corrections for the SCD30 based on Forum post by paulvha: https://forum.sparkfun.com/viewtopic.php?p=222455#p222455
  (done) Add support for the SGP40 VOC Index sensor
  (done) Add support for the SDP3X Differential Pressure sensor
  (done) Add support for the MS5837 - as used in the BlueRobotics BAR02 and BAR30 water pressure sensors
  (done) Correct an issue which was causing the OLA to crash when waking from sleep and outputting serial data https://github.com/sparkfun/OpenLog_Artemis/issues/79
  (done) Correct low-power code as per https://github.com/sparkfun/OpenLog_Artemis/issues/78
  (done) Correct a bug in menuAttachedDevices when useTxRxPinsForTerminal is enabled https://github.com/sparkfun/OpenLog_Artemis/issues/82
  (done) Add ICM-20948 DMP support. Requires v1.2.6 of the ICM-20948 library. DMP logging is limited to: Quat6 or Quat9, plus raw accel, gyro and compass. https://github.com/sparkfun/OpenLog_Artemis/issues/47
  (done) Add support for exFAT. Requires v2.0.6 of Bill Greiman's SdFat library. https://github.com/sparkfun/OpenLog_Artemis/issues/34
  (done) Add minimum awake time: https://github.com/sparkfun/OpenLog_Artemis/issues/83
  (done) Add support for the Pulse Oximeter: https://github.com/sparkfun/OpenLog_Artemis/issues/81
  (done - but does not work) Add support for the Qwiic Button. The QB uses clock-stretching and the Artemis really doesn't enjoy that...
  (done) Increase DMP data resolution to five decimal places https://github.com/sparkfun/OpenLog_Artemis/issues/90

  (in progress) Update to Apollo3 v2.1.0 - FIRMWARE_VERSION_MAJOR = 2.
  (done) Implement printf float (OLA uses printf float in _so_ many places...): https://github.com/sparkfun/Arduino_Apollo3/issues/278
  (worked around) attachInterrupt(PIN_POWER_LOSS, powerDownOLA, FALLING); triggers an immediate interrupt - https://github.com/sparkfun/Arduino_Apollo3/issues/416
  (done) Add a setQwiicPullups function
  (done) Check if we need ap3_set_pin_to_analog when coming out of sleep
  (done) Investigate why code does not wake from deep sleep correctly
  (worked around) Correct SerialLog RX: https://github.com/sparkfun/Arduino_Apollo3/issues/401
    The work-around is to use Serial1 in place of serialLog and then to manually force UART1 to use pins 12 and 13
    We need a work-around anyway because if pins 12 or 13 have been used as analog inputs, Serial1.begin does not re-configure them for UART TX and RX
  (in progress) Reduce sleep current as much as possible. v1.2.1 achieved ~110uA. With v2.1.0 the draw is more like 260uA...

  (in progress) Update to Apollo3 v2.2.0 - FIRMWARE_VERSION_MAJOR = 2; FIRMWARE_VERSION_MINOR = 1.
  (done) Add a fix for issue #109 - check if a BME280 is connected before calling multiplexerBegin: https://github.com/sparkfun/OpenLog_Artemis/issues/109
  (done) Correct issue #104. enableSD was redundant. The microSD power always needs to be on if there is a card inserted, otherwise the card pulls
         the SPI lines low, preventing communication with the IMU:  https://github.com/sparkfun/OpenLog_Artemis/issues/104

  v2.2:
    Use Apollo3 v2.2.1 with changes by paulvha to fix Issue 117 (Thank you Paul!)
      https://github.com/sparkfun/OpenLog_Artemis/issues/117#issuecomment-1085881142
    Also includes Paul's SPI.end fix
      https://github.com/sparkfun/Arduino_Apollo3/issues/442
      In libraries/SPI/src/SPI.cpp change end() to:
        void arduino::MbedSPI::end() {
            if (dev) {
                delete dev;
                dev = NULL;
            }
        }      
    Use SdFat v2.1.2
    Compensate for missing / not-populated IMU
    Add support for yyyy/mm/dd and ISO 8601 date style (Issue 118)
    Add support for fractional time zone offsets

  v2.3 BETA:
    Change GNSS maximum rate to 25Hz as per:
      https://github.com/sparkfun/OpenLog_Artemis/issues/121
      https://forum.sparkfun.com/viewtopic.php?f=172&t=57512
*/

const int FIRMWARE_VERSION_MAJOR = 2;
const int FIRMWARE_VERSION_MINOR = 3;

//Define the OLA board identifier:
//  This is an int which is unique to this variant of the OLA and which allows us
//  to make sure that the settings in EEPROM are correct for this version of the OLA
//  (sizeOfSettings is not necessarily unique and we want to avoid problems when swapping from one variant to another)
//  It is the sum of:
//    the variant * 0x100 (OLA = 1; GNSS_LOGGER = 2; GEOPHONE_LOGGER = 3)
//    the major firmware version * 0x10
//    the minor firmware version
#define OLA_IDENTIFIER 0x122 // Stored as 290 decimal in OLA_settings.txt

#include "settings.h"

//Define the pin functions
//Depends on hardware version. This can be found as a marking on the PCB.
//x04 was the SparkX 'black' version.
//v10 was the first red version.
#define HARDWARE_VERSION_MAJOR 1
#define HARDWARE_VERSION_MINOR 0

#if(HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 4)
const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_IMU_POWER = 22;
#elif(HARDWARE_VERSION_MAJOR == 1 && HARDWARE_VERSION_MINOR == 0)
const byte PIN_MICROSD_CHIP_SELECT = 23;
const byte PIN_IMU_POWER = 27;
const byte PIN_PWR_LED = 29;
const byte PIN_VREG_ENABLE = 25;
const byte PIN_VIN_MONITOR = 34; // VIN/3 (1M/2M - will require a correction factor)
#endif

const byte PIN_POWER_LOSS = 3;
//const byte PIN_LOGIC_DEBUG = 11; // Useful for debugging issues like the slippery mux bug
const byte PIN_MICROSD_POWER = 15;
const byte PIN_QWIIC_POWER = 18;
const byte PIN_STAT_LED = 19;
const byte PIN_IMU_INT = 37;
const byte PIN_IMU_CHIP_SELECT = 44;
const byte PIN_STOP_LOGGING = 32;
const byte BREAKOUT_PIN_32 = 32;
const byte BREAKOUT_PIN_TX = 12;
const byte BREAKOUT_PIN_RX = 13;
const byte BREAKOUT_PIN_11 = 11;
const byte PIN_TRIGGER = 11;
const byte PIN_QWIIC_SCL = 8;
const byte PIN_QWIIC_SDA = 9;

const byte PIN_SPI_SCK = 5;
const byte PIN_SPI_CIPO = 6;
const byte PIN_SPI_COPI = 7;

// Include this many extra bytes when starting a mux - to try and avoid the slippery mux bug
// This should be 0 but 3 or 7 seem to work better depending on which way the wind is blowing.
const byte EXTRA_MUX_STARTUP_BYTES = 3;

enum returnStatus {
  STATUS_GETBYTE_TIMEOUT = 255,
  STATUS_GETNUMBER_TIMEOUT = -123455555,
  STATUS_PRESSED_X,
};

//Setup Qwiic Port
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Wire.h>
TwoWire qwiic(PIN_QWIIC_SDA,PIN_QWIIC_SCL); //Will use pads 8/9
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//EEPROM for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <EEPROM.h>
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>

#include <SdFat.h> //SdFat by Bill Greiman: http://librarymanager/All#SdFat_exFAT

#define SD_FAT_TYPE 3 // SD_FAT_TYPE = 0 for SdFat/File, 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) // 24MHz

#if SD_FAT_TYPE == 1
SdFat32 sd;
File32 sensorDataFile; //File that all sensor data is written to
File32 serialDataFile; //File that all incoming serial data is written to
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile sensorDataFile; //File that all sensor data is written to
ExFile serialDataFile; //File that all incoming serial data is written to
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile sensorDataFile; //File that all sensor data is written to
FsFile serialDataFile; //File that all incoming serial data is written to
#else // SD_FAT_TYPE == 0
SdFat sd;
File sensorDataFile; //File that all sensor data is written to
File serialDataFile; //File that all incoming serial data is written to
#endif  // SD_FAT_TYPE

//#define PRINT_LAST_WRITE_TIME // Uncomment this line to enable the 'measure the time between writes' diagnostic

char sensorDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
char serialDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
const int sdPowerDownDelay = 100; //Delay for this many ms before turning off the SD card power
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Add RTC interface for Artemis
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "RTC.h" //Include RTC library included with the Aruino_Apollo3 core
Apollo3RTC myRTC; //Create instance of RTC class
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Create UART instance for OpenLog style serial logging
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//UART SerialLog(BREAKOUT_PIN_TX, BREAKOUT_PIN_RX);  // Declares a Uart object called SerialLog with TX on pin 12 and RX on pin 13

uint64_t lastSeriaLogSyncTime = 0;
uint64_t lastAwakeTimeMillis;
const int MAX_IDLE_TIME_MSEC = 500;
bool newSerialData = false;
char incomingBuffer[256 * 2]; //This size of this buffer is sensitive. Do not change without analysis using OpenLog_Serial.
int incomingBufferSpot = 0;
int charsReceived = 0; //Used for verifying/debugging serial reception
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Add ICM IMU interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "ICM_20948.h"  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
ICM_20948_SPI myICM;
icm_20948_DMP_data_t dmpData; // Global storage for the DMP data - extracted from the FIFO
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Header files for all compatible Qwiic sensors
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "SparkFun_I2C_Mux_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_I2C_Mux
#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
#include "SparkFunBME280.h" //Click here to get the library: http://librarymanager/All#SparkFun_BME280
#include "SparkFun_LPS25HB_Arduino_Library.h"  //Click here to get the library: http://librarymanager/All#SparkFun_LPS25HB
#include "SparkFun_VEML6075_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_VEML6075
#include "SparkFun_PHT_MS8607_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_PHT_MS8607
#include "SparkFun_MCP9600.h" //Click here to get the library: http://librarymanager/All#SparkFun_MCP9600
#include "SparkFun_SGP30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include "SparkFun_VCNL4040_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_VCNL4040
#include "SparkFun_MS5637_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_MS5637
#include "SparkFun_TMP117.h" //Click here to get the library: http://librarymanager/All#SparkFun_TMP117
#include "SparkFun_u-blox_GNSS_Arduino_Library.h" //http://librarymanager/All#SparkFun_u-blox_GNSS
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_NAU7802
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include "SparkFun_Qwiic_Humidity_AHT20.h" //Click here to get the library: http://librarymanager/All#Qwiic_Humidity_AHT20 by SparkFun
#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
#include "SparkFun_ADS122C04_ADC_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_ADS122C04
#include "SparkFun_MicroPressure.h" // Click here to get the library: http://librarymanager/All#SparkFun_MicroPressure
#include "SparkFun_Particle_Sensor_SN-GCJA5_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_Particle_Sensor_SN-GCJA5
#include "SparkFun_SGP40_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP40
#include "SparkFun_SDP3x_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SDP3x
#include "MS5837.h" // Click here to download the library: https://github.com/sparkfunX/BlueRobotics_MS5837_Library
#include "SparkFun_Qwiic_Button.h" // Click here to get the library: http://librarymanager/All#SparkFun_Qwiic_Button_Switch
#include "SparkFun_Bio_Sensor_Hub_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_Bio_Sensor

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
uint64_t measurementStartTime; //Used to calc the actual update rate. Max is ~80,000,000ms in a 24 hour period.
uint64_t lastSDFileNameChangeTime; //Used to calculate the interval since the last SD filename change
unsigned long measurementCount = 0; //Used to calc the actual update rate.
unsigned long measurementTotal = 0; //The total number of recorded measurements. (Doesn't get reset when the menu is opened)
char outputData[512 * 2]; //Factor of 512 for easier recording to SD in 512 chunks
unsigned long lastReadTime = 0; //Used to delay until user wants to record a new reading
unsigned long lastDataLogSyncTime = 0; //Used to record to SD every half second
unsigned int totalCharactersPrinted = 0; //Limit output rate based on baud rate and number of characters to print
bool takeReading = true; //Goes true when enough time has passed between readings or we've woken from sleep
bool sleepAfterRead = false; //Used to keep the code awake for at least minimumAwakeTimeMillis
const uint64_t maxUsBeforeSleep = 2000000ULL; //Number of us between readings before sleep is activated.
const byte menuTimeout = 15; //Menus will exit/timeout after this number of seconds
volatile static bool stopLoggingSeen = false; //Flag to indicate if we should stop logging
uint64_t qwiicPowerOnTime = 0; //Used to delay after Qwiic power on to allow sensors to power on, then answer autodetect
unsigned long qwiicPowerOnDelayMillis; //Wait for this many milliseconds after turning on the Qwiic power before attempting to communicate with Qwiic devices
int lowBatteryReadings = 0; // Count how many times the battery voltage has read low
const int lowBatteryReadingsLimit = 10; // Don't declare the battery voltage low until we have had this many consecutive low readings (to reject sampling noise)
volatile static bool triggerEdgeSeen = false; //Flag to indicate if a trigger interrupt has been seen
char serialTimestamp[40]; //Buffer to store serial timestamp, if needed
volatile static bool powerLossSeen = false; //Flag to indicate if a power loss event has been seen
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

uint8_t getByteChoice(int numberOfSeconds, bool updateDZSERIAL = false); // Header

// gfvalvo's flash string helper code: https://forum.arduino.cc/index.php?topic=533118.msg3634809#msg3634809
void SerialPrint(const char *);
void SerialPrint(const __FlashStringHelper *);
void SerialPrintln(const char *);
void SerialPrintln(const __FlashStringHelper *);
void DoSerialPrint(char (*)(const char *), const char *, bool newLine = false);

#define DUMP( varname ) {Serial.printf("%s: %d\r\n", #varname, varname); if (settings.useTxRxPinsForTerminal == true) Serial1.printf("%s: %d\r\n", #varname, varname);}
#define SerialPrintf1( var ) {Serial.printf( var ); if (settings.useTxRxPinsForTerminal == true) Serial1.printf( var );}
#define SerialPrintf2( var1, var2 ) {Serial.printf( var1, var2 ); if (settings.useTxRxPinsForTerminal == true) Serial1.printf( var1, var2 );}
#define SerialPrintf3( var1, var2, var3 ) {Serial.printf( var1, var2, var3 ); if (settings.useTxRxPinsForTerminal == true) Serial1.printf( var1, var2, var3 );}
#define SerialPrintf4( var1, var2, var3, var4 ) {Serial.printf( var1, var2, var3, var4 ); if (settings.useTxRxPinsForTerminal == true) Serial1.printf( var1, var2, var3, var4 );}
#define SerialPrintf5( var1, var2, var3, var4, var5 ) {Serial.printf( var1, var2, var3, var4, var5 ); if (settings.useTxRxPinsForTerminal == true) Serial1.printf( var1, var2, var3, var4, var5 );}

// The Serial port for the Zmodem connection
// must not be the same as DSERIAL unless all
// debugging output to DSERIAL is removed
Stream *ZSERIAL;

// Serial output for debugging info for Zmodem
Stream *DSERIAL;

void setup() {
  //If 3.3V rail drops below 3V, system will power down and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  delay(1); // Let PIN_POWER_LOSS stabilize

  if (digitalRead(PIN_POWER_LOSS) == LOW) powerDownOLA(); //Check PIN_POWER_LOSS just in case we missed the falling edge
  //attachInterrupt(PIN_POWER_LOSS, powerDownOLA, FALLING); // We can't do this with v2.1.0 as attachInterrupt causes a spontaneous interrupt
  attachInterrupt(PIN_POWER_LOSS, powerLossISR, FALLING);
  powerLossSeen = false; // Make sure the flag is clear

  powerLEDOn(); // Turn the power LED on - if the hardware supports it

  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, HIGH); // Turn the STAT LED on while we configure everything

  SPI.begin(); //Needed if SD is disabled

  //Do not start Serial1 before productionTest() otherwise the pin configuration gets overwritten
  //and subsequent Serial1.begin's don't restore the pins to UART mode...

  productionTest(); //Check if we need to go into production test mode

  //We need to manually restore the Serial1 TX and RX pins after they were changed by productionTest()
  configureSerial1TxRx();

  Serial.begin(115200); //Default for initial debug messages if necessary
  Serial1.begin(115200); //Default for initial debug messages if necessary

  //pinMode(PIN_LOGIC_DEBUG, OUTPUT); // Debug pin to assist tracking down slippery mux bugs
  //digitalWrite(PIN_LOGIC_DEBUG, HIGH);

  // Use the worst case power on delay for the Qwiic bus for now as we don't yet know what sensors are connected
  // (worstCaseQwiicPowerOnDelay is defined in settings.h)
  qwiicPowerOnDelayMillis = worstCaseQwiicPowerOnDelay;

  EEPROM.init();

  beginQwiic(); // Turn the qwiic power on as early as possible

  beginSD(); //285 - 293ms

  enableCIPOpullUp(); // Enable CIPO pull-up _after_ beginSD

  loadSettings(); //50 - 250ms

  if (settings.useTxRxPinsForTerminal == true)
  {
    Serial1.flush(); //Complete any previous prints at the previous baud rate
    Serial1.begin(settings.serialTerminalBaudRate); // Restart the serial port
  }
  else
  {
    Serial1.flush(); //Complete any previous prints
    Serial1.end(); // Stop the SerialLog port
  }

  Serial.flush(); //Complete any previous prints
  Serial.begin(settings.serialTerminalBaudRate);

  SerialPrintf3("Artemis OpenLog v%d.%d\r\n", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR);

  if (settings.useGPIO32ForStopLogging == true)
  {
    SerialPrintln(F("Stop Logging is enabled. Pull GPIO pin 32 to GND to stop logging."));
    pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
    delay(1); // Let the pin stabilize
    attachInterrupt(PIN_STOP_LOGGING, stopLoggingISR, FALLING); // Enable the interrupt
    am_hal_gpio_pincfg_t intPinConfig = g_AM_HAL_GPIO_INPUT_PULLUP;
    intPinConfig.eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO;
    pin_config(PinName(PIN_STOP_LOGGING), intPinConfig); // Make sure the pull-up does actually stay enabled
    stopLoggingSeen = false; // Make sure the flag is clear
  }

  if (settings.useGPIO11ForTrigger == true)
  {
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
    delay(1); // Let the pin stabilize
    am_hal_gpio_pincfg_t intPinConfig = g_AM_HAL_GPIO_INPUT_PULLUP;
    if (settings.fallingEdgeTrigger == true)
    {
      SerialPrintln(F("Falling-edge triggering is enabled. Sensor data will be logged on a falling edge on GPIO pin 11."));
      attachInterrupt(PIN_TRIGGER, triggerPinISR, FALLING); // Enable the interrupt
      intPinConfig.eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO;
    }
    else
    {
      SerialPrintln(F("Rising-edge triggering is enabled. Sensor data will be logged on a rising edge on GPIO pin 11."));
      attachInterrupt(PIN_TRIGGER, triggerPinISR, RISING); // Enable the interrupt
      intPinConfig.eIntDir = AM_HAL_GPIO_PIN_INTDIR_LO2HI;
    }
    pin_config(PinName(PIN_TRIGGER), intPinConfig); // Make sure the pull-up does actually stay enabled
    triggerEdgeSeen = false; // Make sure the flag is clear
  }

  analogReadResolution(14); //Increase from default of 10

  beginDataLogging(); //180ms
  lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change

  serialTimestamp[0] = '\0'; // Empty the serial timestamp buffer

  if (settings.useTxRxPinsForTerminal == false)
  {
    beginSerialLogging(); //20 - 99ms
    beginSerialOutput(); // Begin serial data output on the TX pin
  }

  beginIMU(); //61ms

  if (online.microSD == true) SerialPrintln(F("SD card online"));
  else SerialPrintln(F("SD card offline"));

  if (online.dataLogging == true) SerialPrintln(F("Data logging online"));
  else SerialPrintln(F("Datalogging offline"));

  if (online.serialLogging == true) SerialPrintln(F("Serial logging online"));
  else SerialPrintln(F("Serial logging offline"));

  if (online.IMU == true) SerialPrintln(F("IMU online"));
  else SerialPrintln(F("IMU offline - or not present"));

  if (settings.logMaxRate == true) SerialPrintln(F("Logging analog pins at max data rate"));

  if (settings.enableTerminalOutput == false && settings.logData == true) SerialPrintln(F("Logging to microSD card with no terminal output"));

  if (detectQwiicDevices() == true) //159 - 865ms but varies based on number of devices attached
  {
    beginQwiicDevices(); //Begin() each device in the node list
    loadDeviceSettingsFromFile(); //Load config settings into node list
    configureQwiicDevices(); //Apply config settings to each device in the node list
    int deviceCount = printOnlineDevice(); // Pretty-print the online devices

    if ((deviceCount == 0) && (settings.resetOnZeroDeviceCount == true)) // Check for resetOnZeroDeviceCount
    {
      if ((Serial.available()) || ((settings.useTxRxPinsForTerminal == true) && (Serial1.available())))
        menuMain(); //Present user menu - in case the user wants to disable resetOnZeroDeviceCount
      else
      {
        SerialPrintln(F("*** Zero Qwiic Devices Found! Resetting... ***"));
        SerialFlush();
        resetArtemis(); //Thank you and goodnight...
      }
    }
  }
  else
    SerialPrintln(F("No Qwiic devices detected"));

  if (settings.showHelperText == true) printHelperText(false); //printHelperText to terminal and sensor file

  //If we are sleeping between readings then we cannot rely on millis() as it is powered down
  //Use RTC instead
  measurementStartTime = rtcMillis();

  digitalWrite(PIN_STAT_LED, LOW); // Turn the STAT LED off now that everything is configured

  lastAwakeTimeMillis = rtcMillis();

  //If we are immediately going to go to sleep after the first reading then
  //first present the user with the config menu in case they need to change something
  if (checkIfItIsTimeToSleep())
    menuMain();
}

void loop() {

  checkBattery(); // Check for low battery

<<<<<<< HEAD
  //if ((Serial.available()) || ((settings.useTxRxPinsForTerminal == true) && (Serial1.available())))
  if ((Serial.peek() >= 9) || ((settings.useTxRxPinsForTerminal == true) && (Serial1.peek() >= 9))) // is it a TAB (9) or higher
=======
  if ((Serial.available()) || ((settings.useTxRxPinsForTerminal == true) && (Serial1.available())))
>>>>>>> parent of 883838a (Update OpenLog_Artemis.ino)
    menuMain(); //Present user menu

  if (settings.logSerial == true && online.serialLogging == true && settings.useTxRxPinsForTerminal == false)
  {
    size_t timestampCharsLeftToWrite = strlen(serialTimestamp);
    //SerialPrintf2("timestampCharsLeftToWrite is %d\r\n", timestampCharsLeftToWrite);
    //SerialFlush();

    if (Serial1.available() || (timestampCharsLeftToWrite > 0))
    {
      while (Serial1.available() || (timestampCharsLeftToWrite > 0))
      {
        if (timestampCharsLeftToWrite > 0) // Based on code written by @DennisMelamed in PR #70
        {
          incomingBuffer[incomingBufferSpot++] = serialTimestamp[0]; // Add a timestamp character to incomingBuffer

          for (size_t i = 0; i < timestampCharsLeftToWrite; i++)
          {
            serialTimestamp[i] = serialTimestamp[i+1]; // Shuffle the remaining chars along by one
          }

          timestampCharsLeftToWrite -= 1;
        }
        else
        {
          incomingBuffer[incomingBufferSpot++] = Serial1.read();

          //Get the RTC timestamp if we just received the timestamp token
          if (settings.timestampSerial && (incomingBuffer[incomingBufferSpot-1] == settings.timeStampToken))
          {
            getTimeString(&serialTimestamp[2]);
            serialTimestamp[0] = 0x0A; // Add Line Feed at the start of the timestamp
            serialTimestamp[1] = '^'; // Add an up-arrow to indicate the timestamp relates to the preceeding data
            serialTimestamp[strlen(serialTimestamp) - 1] = 0x0A; // Change the final comma of the timestamp to a Line Feed
          }
        }

        if (incomingBufferSpot == sizeof(incomingBuffer))
        {
          digitalWrite(PIN_STAT_LED, HIGH); //Toggle stat LED to indicating log recording
          serialDataFile.write(incomingBuffer, sizeof(incomingBuffer)); //Record the buffer to the card
          digitalWrite(PIN_STAT_LED, LOW);
          incomingBufferSpot = 0;
        }
        charsReceived++;
        checkBattery();
      }

      //If we are sleeping between readings then we cannot rely on millis() as it is powered down
      //Use RTC instead
      lastSeriaLogSyncTime = rtcMillis(); //Reset the last sync time to now
      newSerialData = true;
    }
    else if (newSerialData == true)
    {
      if ((rtcMillis() - lastSeriaLogSyncTime) > MAX_IDLE_TIME_MSEC) //If we haven't received any characters recently then sync log file
      {
        if (incomingBufferSpot > 0)
        {
          //Write the remainder of the buffer
          digitalWrite(PIN_STAT_LED, HIGH); //Toggle stat LED to indicating log recording
          serialDataFile.write(incomingBuffer, incomingBufferSpot); //Record the buffer to the card
          serialDataFile.sync();
          if (settings.frequentFileAccessTimestamps == true)
            updateDataFileAccess(&serialDataFile); // Update the file access time & date
          digitalWrite(PIN_STAT_LED, LOW);

          incomingBufferSpot = 0;
        }

        newSerialData = false;
        lastSeriaLogSyncTime = rtcMillis(); //Reset the last sync time to now
        printDebug("Total chars received: " + (String)charsReceived + "\r\n");
      }
    }
  }

  //In v2.1 of the core micros() becomes corrupted during deep sleep so only test if we are not sleeping
  if (settings.usBetweenReadings < maxUsBeforeSleep)
  {
    if ((micros() - lastReadTime) >= settings.usBetweenReadings)
      takeReading = true;
  }

  //Check for a trigger event
  if (settings.useGPIO11ForTrigger == true)
  {
    if (triggerEdgeSeen == true)
    {
      takeReading = true; // If triggering is enabled and a trigger event has been seen, then take a reading.
    }
    else
    {
      takeReading = false; // If triggering is enabled and a trigger even has not been seen, then make sure we don't take a reading based on settings.usBetweenReadings.
    }
  }

  //Is it time to get new data?
  if ((settings.logMaxRate == true) || (takeReading == true))
  {
    takeReading = false;
    lastReadTime = micros();

#ifdef PRINT_LAST_WRITE_TIME
    if (settings.printDebugMessages)
    {
      // Print how long it has been since the last write
      char tempTimeRev[20]; // Char array to hold to usBR (reversed order)
      char tempTime[20]; // Char array to hold to usBR (correct order)
      static uint64_t lastWriteTime; //Used to calculate the time since the last SD write (sleep-proof)
      unsigned long usBR = rtcMillis() - lastWriteTime;
      unsigned int i = 0;
      if (usBR == 0ULL) // if usBetweenReadings is zero, set tempTime to "0"
      {
        tempTime[0] = '0';
        tempTime[1] = 0;
      }
      else
      {
        while (usBR > 0)
        {
          tempTimeRev[i++] = (usBR % 10) + '0'; // divide by 10, convert the remainder to char
          usBR /= 10; // divide by 10
        }
        unsigned int j = 0;
        while (i > 0)
        {
          tempTime[j++] = tempTimeRev[--i]; // reverse the order
          tempTime[j] = 0; // mark the end with a NULL
        }
      }

      //printDebug("ms since last write: " + (String)tempTime + "\r\n");
      printDebug((String)tempTime + "\r\n");

      lastWriteTime = rtcMillis();
    }
#endif

    getData(); //Query all enabled sensors for data

    //Print to terminal
    if (settings.enableTerminalOutput == true)
      SerialPrint(outputData); //Print to terminal

    //Output to TX pin
    if ((settings.outputSerial == true) && (online.serialOutput == true))
      Serial1.print(outputData); //Print to TX pin

    //Record to SD
    if ((settings.logData == true) && (online.microSD))
    {
      digitalWrite(PIN_STAT_LED, HIGH);
      uint32_t recordLength = sensorDataFile.write(outputData, strlen(outputData));
      if (recordLength != strlen(outputData)) //Record the buffer to the card
      {
        if (settings.printDebugMessages == true)
        {
          SerialPrintf3("*** sensorDataFile.write data length mismatch! *** recordLength: %d, outputDataLength: %d\r\n", recordLength, strlen(outputData));
        }
      }

      //Force sync every 500ms
      if (rtcMillis() - lastDataLogSyncTime > 500)
      {
        lastDataLogSyncTime = rtcMillis();
        sensorDataFile.sync();
        if (settings.frequentFileAccessTimestamps == true)
          updateDataFileAccess(&sensorDataFile); // Update the file access time & date
      }

      //Check if it is time to open a new log file
      uint64_t secsSinceLastFileNameChange = rtcMillis() - lastSDFileNameChangeTime; // Calculate how long we have been logging for
      secsSinceLastFileNameChange /= 1000ULL; // Convert to secs
      if ((settings.openNewLogFilesAfter > 0) && (((unsigned long)secsSinceLastFileNameChange) >= settings.openNewLogFilesAfter))
      {
        //Close existings files
        if (online.dataLogging == true)
        {
          sensorDataFile.sync();
          updateDataFileAccess(&sensorDataFile); // Update the file access time & date
          sensorDataFile.close();
          strcpy(sensorDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));
          beginDataLogging(); //180ms
          if (settings.showHelperText == true) printHelperText(false); //printHelperText to terminal and sensor file
        }
        if (online.serialLogging == true)
        {
          serialDataFile.sync();
          updateDataFileAccess(&serialDataFile); // Update the file access time & date
          serialDataFile.close();
          strcpy(serialDataFileName, findNextAvailableLog(settings.nextSerialLogNumber, "serialLog"));
          beginSerialLogging();
        }

        lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
      }

      digitalWrite(PIN_STAT_LED, LOW);
    }

    if ((settings.useGPIO32ForStopLogging == true) && (stopLoggingSeen == true)) // Has the user pressed the stop logging button?
    {
      stopLogging();
    }

    triggerEdgeSeen = false; // Clear the trigger seen flag here - just in case another trigger was received while we were logging data to SD card

    // Code changes here are based on suggestions by @ryanneve in Issue #46, PR #64 and Issue #83
    if (checkIfItIsTimeToSleep())
    {
      sleepAfterRead = true;
    }
  }

  if (sleepAfterRead == true)
  {
    // Check if we should stay awake because settings.minimumAwakeTimeMillis is non-zero
    if ((settings.usBetweenReadings >= maxUsBeforeSleep) && (settings.minimumAwakeTimeMillis > 0))
    {
      // Check if we have been awake long enough (millis is reset to zero when waking from sleep)
      // goToSleep will automatically compensate for how long we have been awake
      if ((rtcMillis() - lastAwakeTimeMillis) < settings.minimumAwakeTimeMillis)
        return; // Too early to sleep - leave sleepAfterRead set true
    }

    sleepAfterRead = false;
    goToSleep(howLongToSleepFor());
  }
}

uint32_t howLongToSleepFor(void)
{
  //Counter/Timer 6 will use the 32kHz clock
  //Calculate how many 32768Hz system ticks we need to sleep for:
  //sysTicksToSleep = msToSleep * 32768L / 1000
  //We need to be careful with the multiply as we will overflow uint32_t if msToSleep is > 131072

  //goToSleep will automatically compensate for how long we have been awake

  uint32_t msToSleep;

  if (checkSleepOnFastSlowPin())
    msToSleep = (uint32_t)(settings.slowLoggingIntervalSeconds * 1000UL);
  else if (checkSleepOnRTCTime())
  {
    // checkSleepOnRTCTime has returned true, so we know that we are between slowLoggingStartMOD and slowLoggingStopMOD
    // We need to check how long it is until slowLoggingStopMOD (accounting for midnight!) and adjust the sleep duration
    // if slowLoggingStopMOD occurs before slowLoggingIntervalSeconds

    msToSleep = (uint32_t)(settings.slowLoggingIntervalSeconds * 1000UL); // Default to this

    myRTC.getTime(); // Get the RTC time
    long secondsOfDay = (myRTC.hour * 60 * 60) + (myRTC.minute * 60) + myRTC.seconds;

    long slowLoggingStopSOD = settings.slowLoggingStopMOD * 60; // Convert slowLoggingStop to seconds-of-day

    long secondsUntilStop = slowLoggingStopSOD - secondsOfDay; // Calculate how long it is until slowLoggingStop

    // If secondsUntilStop is negative then we know that now is before midnight and slowLoggingStop is after midnight
    if (secondsUntilStop < 0) secondsUntilStop += 24 * 60 * 60; // Add a day's worth of seconds if required to make secondsUntilStop positive

    if (secondsUntilStop < settings.slowLoggingIntervalSeconds) // If we need to sleep for less than slowLoggingIntervalSeconds
      msToSleep = (secondsUntilStop + 1) * 1000UL; // Adjust msToSleep, adding one extra second to make sure the next wake is > slowLoggingStop
  }
  else // checkSleepOnUsBetweenReadings
  {
    msToSleep = (uint32_t)(settings.usBetweenReadings / 1000ULL); // Sleep for usBetweenReadings
  }

  uint32_t sysTicksToSleep;
  if (msToSleep < 131000)
  {
    sysTicksToSleep = msToSleep * 32768L; // Do the multiply first for short intervals
    sysTicksToSleep = sysTicksToSleep / 1000L; // Now do the divide
  }
  else
  {
    sysTicksToSleep = msToSleep / 1000L; // Do the division first for long intervals (to avoid an overflow)
    sysTicksToSleep = sysTicksToSleep * 32768L; // Now do the multiply
  }

  return (sysTicksToSleep);
}

bool checkIfItIsTimeToSleep(void)
{

  if (checkSleepOnUsBetweenReadings()
  || checkSleepOnRTCTime()
  || checkSleepOnFastSlowPin())
    return(true);
  else
    return(false);
}

//Go to sleep if the time between readings is greater than maxUsBeforeSleep (2 seconds) and triggering is not enabled
bool checkSleepOnUsBetweenReadings(void)
{
  if ((settings.useGPIO11ForTrigger == false) && (settings.usBetweenReadings >= maxUsBeforeSleep))
    return (true);
  else
    return (false);
}

//Go to sleep if Fast/Slow logging on Pin 11 is enabled and Pin 11 is in the correct state
bool checkSleepOnFastSlowPin(void)
{
  if ((settings.useGPIO11ForFastSlowLogging == true) && (digitalRead(PIN_TRIGGER) == settings.slowLoggingWhenPin11Is))
    return (true);
  else
    return (false);
}

// Go to sleep if useRTCForFastSlowLogging is enabled and RTC time is between the start and stop times
bool checkSleepOnRTCTime(void)
{
  // Check if we should be sleeping based on useGPIO11ForFastSlowLogging and slowLoggingStartMOD + slowLoggingStopMOD
  bool sleepOnRTCTime = false;
  if (settings.useRTCForFastSlowLogging == true)
  {
    if (settings.slowLoggingStartMOD != settings.slowLoggingStopMOD) // Only perform the check if the start and stop times are not equal
    {
      myRTC.getTime(); // Get the RTC time
      int minutesOfDay = (myRTC.hour * 60) + myRTC.minute;

      if (settings.slowLoggingStartMOD > settings.slowLoggingStopMOD) // If slow logging starts later than the stop time (i.e. slow over midnight)
      {
        if ((minutesOfDay >= settings.slowLoggingStartMOD) || (minutesOfDay < settings.slowLoggingStopMOD))
          sleepOnRTCTime = true;
      }
      else // Slow logging starts earlier than the stop time
      {
        if ((minutesOfDay >= settings.slowLoggingStartMOD) && (minutesOfDay < settings.slowLoggingStopMOD))
          sleepOnRTCTime = true;
      }
    }
  }
  return(sleepOnRTCTime);
}

void beginQwiic()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pin_config(PinName(PIN_QWIIC_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  qwiicPowerOn();
  qwiic.begin();
  setQwiicPullups(settings.qwiicBusPullUps); //Just to make it really clear what pull-ups are being used, set pullups here.
}

void setQwiicPullups(uint32_t qwiicBusPullUps)
{
  //Change SCL and SDA pull-ups manually using pin_config
  am_hal_gpio_pincfg_t sclPinCfg = g_AM_BSP_GPIO_IOM1_SCL;
  am_hal_gpio_pincfg_t sdaPinCfg = g_AM_BSP_GPIO_IOM1_SDA;

  if (qwiicBusPullUps == 0)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_NONE; // No pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_NONE;
  }
  else if (qwiicBusPullUps == 1)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K; // Use 1K5 pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K;
  }
  else if (qwiicBusPullUps == 6)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_6K; // Use 6K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_6K;
  }
  else if (qwiicBusPullUps == 12)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_12K; // Use 12K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_12K;
  }
  else
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_24K; // Use 24K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_24K;
  }

  pin_config(PinName(PIN_QWIIC_SCL), sclPinCfg);
  pin_config(PinName(PIN_QWIIC_SDA), sdaPinCfg);
}

void beginSD()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pin_config(PinName(PIN_MICROSD_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  pin_config(PinName(PIN_MICROSD_CHIP_SELECT), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected

  // If the microSD card is present, it needs to be powered on otherwise the IMU will fail to start
  // (The microSD card will pull the SPI pins low, preventing communication with the IMU)

  // For reasons I don't understand, we seem to have to wait for at least 1ms after SPI.begin before we call microSDPowerOn.
  // If you comment the next line, the Artemis resets at microSDPowerOn when beginSD is called from wakeFromSleep...
  // But only on one of my V10 red boards. The second one I have doesn't seem to need the delay!?
  delay(5);

  microSDPowerOn();

  //Max power up time is 250ms: https://www.kingston.com/datasheets/SDCIT-specsheet-64gb_en.pdf
  //Max current is 200mA average across 1s, peak 300mA
  for (int i = 0; i < 10; i++) //Wait
  {
    checkBattery();
    delay(1);
  }

  if (sd.begin(SD_CONFIG) == false) // Try to begin the SD card using the correct chip select
  {
    printDebug(F("SD init failed (first attempt). Trying again...\r\n"));
    for (int i = 0; i < 250; i++) //Give SD more time to power up, then try again
    {
      checkBattery();
      delay(1);
    }
    if (sd.begin(SD_CONFIG) == false) // Try to begin the SD card using the correct chip select
    {
      SerialPrintln(F("SD init failed (second attempt). Is card present? Formatted?"));
      SerialPrintln(F("Please ensure the SD card is formatted correctly using https://www.sdcard.org/downloads/formatter/"));
      digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected
      online.microSD = false;
      return;
    }
  }

  //Change to root directory. All new file creation will be in root.
  if (sd.chdir() == false)
  {
    SerialPrintln(F("SD change directory failed"));
    online.microSD = false;
    return;
  }

  online.microSD = true;
}

void enableCIPOpullUp() // updated for v2.1.0 of the Apollo3 core
{
  //Add 1K5 pull-up on CIPO
  am_hal_gpio_pincfg_t cipoPinCfg = g_AM_BSP_GPIO_IOM0_MISO;
  cipoPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K;
  pin_config(PinName(PIN_SPI_CIPO), cipoPinCfg);
}

void disableCIPOpullUp() // updated for v2.1.0 of the Apollo3 core
{
  am_hal_gpio_pincfg_t cipoPinCfg = g_AM_BSP_GPIO_IOM0_MISO;
  pin_config(PinName(PIN_SPI_CIPO), cipoPinCfg);
}

void configureSerial1TxRx(void) // Configure pins 12 and 13 for UART1 TX and RX
{
  am_hal_gpio_pincfg_t pinConfigTx = g_AM_BSP_GPIO_COM_UART_TX;
  pinConfigTx.uFuncSel = AM_HAL_PIN_12_UART1TX;
  pin_config(PinName(BREAKOUT_PIN_TX), pinConfigTx);
  am_hal_gpio_pincfg_t pinConfigRx = g_AM_BSP_GPIO_COM_UART_RX;
  pinConfigRx.uFuncSel = AM_HAL_PIN_13_UART1RX;
  pinConfigRx.ePullup = AM_HAL_GPIO_PIN_PULLUP_WEAK; // Put a weak pull-up on the Rx pin
  pin_config(PinName(BREAKOUT_PIN_RX), pinConfigRx);
}

void beginIMU()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  pin_config(PinName(PIN_IMU_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  pinMode(PIN_IMU_CHIP_SELECT, OUTPUT);
  pin_config(PinName(PIN_IMU_CHIP_SELECT), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_IMU_CHIP_SELECT, HIGH); //Be sure IMU is deselected

  if (settings.enableIMU == true && settings.logMaxRate == false)
  {
    //Reset ICM by power cycling it
    imuPowerOff();
    for (int i = 0; i < 10; i++) //10 is fine
    {
      checkBattery();
      delay(1);
    }
    imuPowerOn();
    for (int i = 0; i < 25; i++) //Allow ICM to come online. Typical is 11ms. Max is 100ms. https://cdn.sparkfun.com/assets/7/f/e/c/d/DS-000189-ICM-20948-v1.3.pdf
    {
      checkBattery();
      delay(1);
    }

    if (settings.printDebugMessages) myICM.enableDebugging();
    myICM.begin(PIN_IMU_CHIP_SELECT, SPI, 4000000); //Set IMU SPI rate to 4MHz
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      printDebug("beginIMU: first attempt at myICM.begin failed. myICM.status = " + (String)myICM.status + "\r\n");
      //Try one more time with longer wait

      //Reset ICM by power cycling it
      imuPowerOff();
      for (int i = 0; i < 10; i++) //10 is fine
      {
        checkBattery();
        delay(1);
      }
      imuPowerOn();
      for (int i = 0; i < 100; i++) //Allow ICM to come online. Typical is 11ms. Max is 100ms.
      {
        checkBattery();
        delay(1);
      }

      myICM.begin(PIN_IMU_CHIP_SELECT, SPI, 4000000); //Set IMU SPI rate to 4MHz
      if (myICM.status != ICM_20948_Stat_Ok)
      {
        printDebug("beginIMU: second attempt at myICM.begin failed. myICM.status = " + (String)myICM.status + "\r\n");
        digitalWrite(PIN_IMU_CHIP_SELECT, HIGH); //Be sure IMU is deselected
        SerialPrintln(F("ICM-20948 failed to init."));
        imuPowerOff();
        online.IMU = false;
        return;
      }
    }

    //Give the IMU extra time to get its act together. This seems to fix the IMU-not-starting-up-cleanly-after-sleep problem...
    //Seems to need a full 25ms. 10ms is not enough.
    for (int i = 0; i < 25; i++) //Allow ICM to come online.
    {
      checkBattery();
      delay(1);
    }

    bool success = true;

    //Check if we are using the DMP
    if (settings.imuUseDMP == false)
    {
      //Perform a full startup (not minimal) for non-DMP mode
      ICM_20948_Status_e retval = myICM.startupDefault(false);
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not startup the IMU in non-DMP mode!"));
        success = false;
      }
      //Update the full scale and DLPF settings
      retval = myICM.enableDLPF(ICM_20948_Internal_Acc, settings.imuAccDLPF);
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not configure the IMU Accelerometer DLPF!"));
        success = false;
      }
      retval = myICM.enableDLPF(ICM_20948_Internal_Gyr, settings.imuGyroDLPF);
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not configure the IMU Gyro DLPF!"));
        success = false;
      }
      ICM_20948_dlpcfg_t dlpcfg;
      dlpcfg.a = settings.imuAccDLPFBW;
      dlpcfg.g = settings.imuGyroDLPFBW;
      retval = myICM.setDLPFcfg((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), dlpcfg);
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not configure the IMU DLPF BW!"));
        success = false;
      }
      ICM_20948_fss_t FSS;
      FSS.a = settings.imuAccFSS;
      FSS.g = settings.imuGyroFSS;
      retval = myICM.setFullScale((ICM_20948_Internal_Acc | ICM_20948_Internal_Gyr), FSS);
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not configure the IMU Full Scale!"));
        success = false;
      }
    }
    else
    {
      // Initialize the DMP
      ICM_20948_Status_e retval = myICM.initializeDMP();
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not startup the IMU in DMP mode!"));
        success = false;
      }
      if (settings.imuLogDMPQuat6)
      {
        retval = myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR);
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not enable the Game Rotation Vector (Quat6)!"));
          success = false;
        }
        retval = myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, 0); // Set ODR to 55Hz
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not set the Quat6 ODR!"));
          success = false;
        }
      }
      if (settings.imuLogDMPQuat9)
      {
        retval = myICM.enableDMPSensor(INV_ICM20948_SENSOR_ROTATION_VECTOR);
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not enable the Rotation Vector (Quat9)!"));
          success = false;
        }
        retval = myICM.setDMPODRrate(DMP_ODR_Reg_Quat9, 0); // Set ODR to 55Hz
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not set the Quat9 ODR!"));
          success = false;
        }
      }
      if (settings.imuLogDMPAccel)
      {
        retval = myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_ACCELEROMETER);
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not enable the DMP Accelerometer!"));
          success = false;
        }
        retval = myICM.setDMPODRrate(DMP_ODR_Reg_Accel, 0); // Set ODR to 55Hz
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not set the Accel ODR!"));
          success = false;
        }
      }
      if (settings.imuLogDMPGyro)
      {
        retval = myICM.enableDMPSensor(INV_ICM20948_SENSOR_RAW_GYROSCOPE);
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not enable the DMP Gyroscope!"));
          success = false;
        }
        retval = myICM.setDMPODRrate(DMP_ODR_Reg_Gyro, 0); // Set ODR to 55Hz
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not set the Gyro ODR!"));
          success = false;
        }
        retval = myICM.setDMPODRrate(DMP_ODR_Reg_Gyro_Calibr, 0); // Set ODR to 55Hz
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not set the Gyro Calibr ODR!"));
          success = false;
        }
      }
      if (settings.imuLogDMPCpass)
      {
        retval = myICM.enableDMPSensor(INV_ICM20948_SENSOR_MAGNETIC_FIELD_UNCALIBRATED);
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not enable the DMP Compass!"));
          success = false;
        }
        retval = myICM.setDMPODRrate(DMP_ODR_Reg_Cpass, 0); // Set ODR to 55Hz
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not set the Compass ODR!"));
          success = false;
        }
        retval = myICM.setDMPODRrate(DMP_ODR_Reg_Cpass_Calibr, 0); // Set ODR to 55Hz
        if (retval != ICM_20948_Stat_Ok)
        {
          SerialPrintln(F("Error: Could not set the Compass Calibr ODR!"));
          success = false;
        }
      }
      retval = myICM.enableFIFO(); // Enable the FIFO
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not enable the FIFO!"));
        success = false;
      }
      retval = myICM.enableDMP(); // Enable the DMP
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not enable the DMP!"));
        success = false;
      }
      retval = myICM.resetDMP(); // Reset the DMP
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not reset the DMP!"));
        success = false;
      }
      retval = myICM.resetFIFO(); // Reset the FIFO
      if (retval != ICM_20948_Stat_Ok)
      {
        SerialPrintln(F("Error: Could not reset the FIFO!"));
        success = false;
      }
    }

    if (success)
    {
      online.IMU = true;
      delay(50); // Give the IMU time to get its first measurement ready
    }
    else
    {
      //Power down IMU
      imuPowerOff();
      online.IMU = false;
    }
  }
  else
  {
    //Power down IMU
    imuPowerOff();
    online.IMU = false;
  }
}

void beginDataLogging()
{
  if (online.microSD == true && settings.logData == true)
  {
    //If we don't have a file yet, create one. Otherwise, re-open the last used file
    if (strlen(sensorDataFileName) == 0)
      strcpy(sensorDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));

    // O_CREAT - create the file if it does not exist
    // O_APPEND - seek to the end of the file prior to each write
    // O_WRITE - open for write
    if (sensorDataFile.open(sensorDataFileName, O_CREAT | O_APPEND | O_WRITE) == false)
    {
      SerialPrintln(F("Failed to create sensor data file"));
      online.dataLogging = false;
      return;
    }

    updateDataFileCreate(&sensorDataFile); // Update the file create time & date
    sensorDataFile.sync();

    online.dataLogging = true;
  }
  else
    online.dataLogging = false;
}

void beginSerialLogging()
{
  if (online.microSD == true && settings.logSerial == true)
  {
    //If we don't have a file yet, create one. Otherwise, re-open the last used file
    if (strlen(serialDataFileName) == 0)
      strcpy(serialDataFileName, findNextAvailableLog(settings.nextSerialLogNumber, "serialLog"));

    if (serialDataFile.open(serialDataFileName, O_CREAT | O_APPEND | O_WRITE) == false)
    {
      SerialPrintln(F("Failed to create serial log file"));
      //systemError(ERROR_FILE_OPEN);
      online.serialLogging = false;
      return;
    }

    updateDataFileCreate(&serialDataFile); // Update the file create time & date
    serialDataFile.sync();

    //We need to manually restore the Serial1 TX and RX pins
    configureSerial1TxRx();

    Serial1.begin(settings.serialLogBaudRate);

    online.serialLogging = true;
  }
  else
    online.serialLogging = false;
}

void beginSerialOutput()
{
  if (settings.outputSerial == true)
  {
    //We need to manually restore the Serial1 TX and RX pins
    configureSerial1TxRx();

    Serial1.begin(settings.serialLogBaudRate); // (Re)start the serial port
    online.serialOutput = true;
  }
  else
    online.serialOutput = false;
}

#if SD_FAT_TYPE == 1
void updateDataFileCreate(File32 *dataFile)
#elif SD_FAT_TYPE == 2
void updateDataFileCreate(ExFile *dataFile)
#elif SD_FAT_TYPE == 3
void updateDataFileCreate(FsFile *dataFile)
#else // SD_FAT_TYPE == 0
void updateDataFileCreate(File *dataFile)
#endif  // SD_FAT_TYPE
{
  myRTC.getTime(); //Get the RTC time so we can use it to update the create time
  //Update the file create time
  dataFile->timestamp(T_CREATE, (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds);
}

#if SD_FAT_TYPE == 1
void updateDataFileAccess(File32 *dataFile)
#elif SD_FAT_TYPE == 2
void updateDataFileAccess(ExFile *dataFile)
#elif SD_FAT_TYPE == 3
void updateDataFileAccess(FsFile *dataFile)
#else // SD_FAT_TYPE == 0
void updateDataFileAccess(File *dataFile)
#endif  // SD_FAT_TYPE
{
  myRTC.getTime(); //Get the RTC time so we can use it to update the last modified time
  //Update the file access time
  dataFile->timestamp(T_ACCESS, (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds);
  //Update the file write time
  dataFile->timestamp(T_WRITE, (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds);
}

//Called once number of milliseconds has passed
extern "C" void am_stimer_cmpr6_isr(void)
{
  uint32_t ui32Status = am_hal_stimer_int_status_get(false);
  if (ui32Status & AM_HAL_STIMER_INT_COMPAREG)
  {
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG);
  }
}

//Power Loss ISR
void powerLossISR(void)
{
  powerLossSeen = true;
}

//Stop Logging ISR
void stopLoggingISR(void)
{
  stopLoggingSeen = true;
}

//Trigger Pin ISR
void triggerPinISR(void)
{
  triggerEdgeSeen = true;
}

void SerialFlush(void)
{
  Serial.flush();
  if (settings.useTxRxPinsForTerminal == true)
  {
    Serial1.flush();
  }
}

// gfvalvo's flash string helper code: https://forum.arduino.cc/index.php?topic=533118.msg3634809#msg3634809

void SerialPrint(const char *line)
{
  DoSerialPrint([](const char *ptr) {return *ptr;}, line);
}

void SerialPrint(const __FlashStringHelper *line)
{
  DoSerialPrint([](const char *ptr) {return (char) pgm_read_byte_near(ptr);}, (const char*) line);
}

void SerialPrintln(const char *line)
{
  DoSerialPrint([](const char *ptr) {return *ptr;}, line, true);
}

void SerialPrintln(const __FlashStringHelper *line)
{
  DoSerialPrint([](const char *ptr) {return (char) pgm_read_byte_near(ptr);}, (const char*) line, true);
}

void DoSerialPrint(char (*funct)(const char *), const char *string, bool newLine)
{
  char ch;

  while ((ch = funct(string++)))
  {
    Serial.print(ch);
    if (settings.useTxRxPinsForTerminal == true)
      Serial1.print(ch);
  }

  if (newLine)
  {
    Serial.println();
    if (settings.useTxRxPinsForTerminal == true)
      Serial1.println();
  }
}
