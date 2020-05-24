/*
  OpenLog Artemis GNSS Logging
  By: Paul Clark (PaulZC)
  Date: May 24th, 2020

  This firmware runs on the OpenLog Artemis and is dedicated to logging UBX
  messages from the u-blox F9 and M9 GNSS receivers.
  
  Messages are streamed directly to SD in UBX format without being processed.
  The SD log files can be analysed afterwards with (e.g.) u-center or RTKLIB.

  You can disable SD card logging if you want to (menu 1 option 1).
  By default, abbreviated UBX messages are displayed in the serial monitor with timestamps.
  You can disable this with menu 1 option 2.
  The message interval can be adjusted (menu 1 option 4 | 5).
  The minimum message interval is adjusted according to which module you have attached.
  At high message rates, Galileo, BeiDou and GLONASS are automatically disabled if required.
  The logging duration and sleep duration can be adjusted (menu 1 option 6 & 7).
  If you want the logger to log continuously, set the sleep duration to zero.
  If you want the logger to open a new log file after sleeping, use menu 1 option 8.

  You can configure the GNSS module and which messages it produces using menu 2 sub-menu 1.
  You can disable GNSS logging using option 1.
  There are two ways to power down the GNSS module while the OLA is asleep:
  a power management task, or switch off the Qwiic power.
  Option 2 enables / disables the power management task. The task duration is set
  to one second less than the sleep duration so the module will be ready when the OLA wakes up.
  Individual messages can be enabled / disabled with options 10-60.
  Leave the UBX-NAV-TIMEUTC message enabled if you want the OLA to set its RTC from GNSS.
  Disabling the USB/UART/SPI ports can reduce the load on the module and give improved
  performance when logging RAWX data at high rates. You can enable / disable the ports
  with options 90-93.
  You will only see messages which are available on your module. E.g. the RAWX message
  will be hidden if you do not have a HPG (ZED-F9P), TIM (ZED-F9T) or FTS module attached.

  If the OLA RTC has been synchronised to GNSS (UTC) time, the SD files will have correct
  created and modified time stamps.

  Diagnostic messages are split into major and minor. You can enable either or both
  via menu d.

  During logging, you can instruct the OLA to close the current log file
  and open a new log file using option f.

  Only the I2C port settings are stored in the GNSS' battery-backed memory.
  All other settings are set in RAM only. So removing the power will restore
  the majority of the module's settings.
  If you need to completely reset the GNSS module, use option g followed by y.

  Option r will reset all of the OLA settings. Afterwards, it can take the code a long
  time to open the next available log file as it needs to check all existing files first.

  The settings are stored in a file called OLA_GNSS_settings.cfg.
  (The settings for the regular OpenLog_Artemis are stored separately in OLA_settings.cfg)

  All GNSS configuration is done using UBX-CFG-VALSET and UBX-CFG-VALGET
  which is only supported on devices like the ZED-F9P and
  NEO-M9N running communication protocols greater than 27.01.

  Only UBX data is logged to SD. ACKs and NACKs are automatically stripped out.

  Based on:
  OpenLog Artemis
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 26th, 2019
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).
  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15793

*/

const int FIRMWARE_VERSION_MAJOR = 1;
const int FIRMWARE_VERSION_MINOR = 1;

#include "settings.h"

const byte PIN_STAT_LED = 19;
const byte PIN_POWER_LOSS = 3;
const byte PIN_LOGIC_DEBUG = 11; //TODO remove from production

enum returnStatus {
  STATUS_GETBYTE_TIMEOUT = 255,
  STATUS_GETNUMBER_TIMEOUT = -123455555,
  STATUS_PRESSED_X,
};

//Setup Qwiic Port
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9
const byte PIN_QWIIC_POWER = 18;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//EEPROM for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <EEPROM.h>
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //We use SdFat-Beta from Bill Greiman for increased read/write speed: https://github.com/greiman/SdFat-beta

const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_MICROSD_POWER = 15; //x04

#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz
#define SD_CONFIG_MAX_SPEED SdSpiConfig(PIN_MICROSD_CHIP_SELECT, DEDICATED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz

//ExFat
SdFs sd;
FsFile gnssDataFile; //File that all GNSS data is written to

//Fat16/32
//SdFat sd;
//File gnssDataFile; //File that all GNSS data is written to

char gnssDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Add RTC interface for Artemis
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "RTC.h" //Include RTC library included with the Aruino_Apollo3 core
APM3_RTC myRTC; //Create instance of RTC class
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Create UART instance for OpenLog style serial logging
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Uart SerialLog(1, 13, 12);  // Declares a Uart object called Serial1 using instance 1 of Apollo3 UART peripherals with RX on pin 13 and TX on pin 12 (note, you specify *pins* not Apollo3 pads. This uses the variant's pin map to determine the Apollo3 pad)
unsigned long lastSeriaLogSyncTime = 0;
const int MAX_IDLE_TIME_MSEC = 500;
bool newSerialData = false;
char incomingBuffer[256 * 2]; //This size of this buffer is sensitive. Do not change without analysis using OpenLog_Serial.
int incomingBufferSpot = 0;
int charsReceived = 0; //Used for verifying/debugging serial reception
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Add ICM IMU interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
const byte PIN_IMU_CHIP_SELECT = 44;
const byte PIN_IMU_POWER = 22;
const byte PIN_IMU_INT = 37;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Header files for all possible Qwiic sensors
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#define MAX_PAYLOAD_SIZE 384 // Override MAX_PAYLOAD_SIZE for getModuleInfo which can return up to 348 bytes

#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS gpsSensor_ublox;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
uint64_t measurementStartTime; //Used to calc the elapsed time
String outputData;
String beginSensorOutput;
unsigned long lastReadTime = 0; //Used to delay between uBlox reads
unsigned long lastDataLogSyncTime = 0; //Used to sync SD every half second
bool helperTextPrinted = false; //Print the column headers only once
bool takeReading = true; //Goes true when enough time has passed between readings or we've woken from sleep
const byte menuTimeout = 45; //Menus will exit/timeout after this number of seconds
bool rtcHasBeenSyncd = false; //Flag to indicate if the RTC been sync'd to GNSS
bool rtcNeedsSync = true; //Flag to indicate if the RTC needs to be sync'd (after sleep)

struct minfoStructure // Structure to hold the GNSS module info
{
  char swVersion[30];
  char hwVersion[10];
  char extension[10][30];
  int protVerMajor;
  int protVerMinor;
  char mod[8]; //The module type from the "MOD=" extension (7 chars + NULL)
  bool SPG; //Standard Precision
  bool HPG; //High Precision (ZED-F9P)
  bool ADR; //Automotive Dead Reckoning (ZED-F9K)
  bool UDR; //Untethered Dead Reckoning (NEO-M8U which does not support protocol 27)
  bool TIM; //Time sync (ZED-F9T) (Guess!)
  bool FTS; //Frequency and Time Sync
  bool LAP; //Lane Accurate (ZED-F9R)
  bool HDG; //Heading (ZED-F9H)
} minfo; //Module info

// Custom UBX Packet for getModuleInfo and powerManagementTask
uint8_t customPayload[MAX_PAYLOAD_SIZE]; // This array holds the payload data bytes
// The next line creates and initialises the packet information which wraps around the payload
ubxPacket customCfg = {0, 0, 0, 0, 0, customPayload, 0, 0, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED};

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//unsigned long startTime = 0;

#define DUMP(varname) {Serial.printf("%s: %llu\n", #varname, varname);}

void setup() {
  //If 3.3V rail drops below 3V, system will power down and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);

  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, LOW);

  if (PIN_LOGIC_DEBUG >= 0)
  {
    pinMode(PIN_LOGIC_DEBUG, OUTPUT); //Debug pin
    digitalWrite(PIN_LOGIC_DEBUG, HIGH); //Make this high, trigger debug on falling edge
  }

  Serial.begin(115200); //Default for initial debug messages if necessary
  Serial.println();

  SPI.begin(); //Needed if SD is disabled

  beginSD(); //285 - 293ms

  loadSettings(); //50 - 250ms

  Serial.flush(); //Complete any previous prints
  Serial.begin(settings.serialTerminalBaudRate);
  Serial.printf("Artemis OpenLog GNSS v%d.%d\n", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR);

  beginQwiic();

  analogReadResolution(14); //Increase from default of 10

  beginDataLogging(); //180ms

  disableIMU(); //Disable IMU

  if (online.microSD == true) Serial.println("SD card online");
  else Serial.println("SD card offline");

  if (online.dataLogging == true) Serial.println("Data logging online");
  else Serial.println("Datalogging offline");

  if (settings.enableTerminalOutput == false && settings.logData == true) Serial.println(F("Logging to microSD card with no terminal output"));

  if (beginSensors() == true) Serial.println(beginSensorOutput); //159 - 865ms but varies based on number of devices attached
  else Serial.println("No sensors detected");

  //If we are sleeping between readings then we cannot rely on millis() as it is powered down. Used RTC instead.
  measurementStartTime = rtcMillis();

//  //If we are immediately going to go to sleep after the first reading then
//  //first present the user with the config menu in case they need to change something
//  if (settings.usBetweenReadings == settings.usLoggingDuration)
//    menuMain();
}

void loop() {
  if (Serial.available()) menuMain(); //Present user menu

  storeData(); //storeData is the workhorse. It reads I2C data and writes it to SD.

  uint64_t timeNow = rtcMillis();

  // Is sleep enabled and is it time to go to sleep?
  if ((settings.usSleepDuration > 0) && (timeNow > (measurementStartTime + (settings.usLoggingDuration / 1000))))
  {
    if (settings.printMajorDebugMessages == true)
    {
      Serial.println(F("Going to sleep..."));
    }

    goToSleep();

    //Update measurementStartTime so we know when to go back to sleep
    measurementStartTime = measurementStartTime + (settings.usLoggingDuration / 1000) + (settings.usSleepDuration / 1000);
    
    rtcNeedsSync = true; //Let's re-sync the RTC after sleep
  }
}

void beginQwiic()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  qwiicPowerOn();
  qwiic.begin();
}

void beginSD()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected

  if (settings.enableSD == true)
  {
    microSDPowerOn();

    //Max power up time is 250ms: https://www.kingston.com/datasheets/SDCIT-specsheet-64gb_en.pdf
    //Max current is 200mA average across 1s, peak 300mA
    delay(10);

    //We can get faster SPI transfer rates if we have only one device enabled on the SPI bus
    //But we have a chicken and egg problem: We need to load settings before we enable SD, but we
    //need the SD to load the settings file. For now, we will disable the logMaxRate option.
    //    if (settings.logMaxRate == true)
    //    {
    //      if (sd.begin(SD_CONFIG_MAX_SPEED) == false) //Very Fast SdFat Beta (dedicated SPI, no IMU)
    //      {
    //        Serial.println(F("SD init failed. Do you have the correct board selected in Arduino? Is card present? Formatted?"));
    //        online.microSD = false;
    //        return;
    //      }
    //    }
    //    else
    //    {
    if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
    {
      delay(250); //Give SD more time to power up, then try again
      if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
      {
        Serial.println(F("SD init failed. Is card present? Formatted?"));
        digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected
        online.microSD = false;
        return;
      }
    }
    //    }

    //Change to root directory. All new file creation will be in root.
    if (sd.chdir() == false)
    {
      Serial.println(F("SD change directory failed"));
      online.microSD = false;
      return;
    }

    online.microSD = true;
  }
  else
  {
    microSDPowerOff();
    online.microSD = false;
  }
}

void disableIMU()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  pinMode(PIN_IMU_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_IMU_CHIP_SELECT, HIGH); //Be sure IMU is deselected

  imuPowerOff();
}

void beginDataLogging()
{
  if (online.microSD == true && settings.logData == true)
  {
    //If we don't have a file yet, create one. Otherwise, re-open the last used file
    if ((strlen(gnssDataFileName) == 0) || (settings.openNewLogFile == true))
      strcpy(gnssDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));

    // O_CREAT - create the file if it does not exist
    // O_APPEND - seek to the end of the file prior to each write
    // O_WRITE - open for write
    if (gnssDataFile.open(gnssDataFileName, O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println(F("Failed to create sensor data file"));
      online.dataLogging = false;
      return;
    }

    updateDataFileCreate(); //Update the data file creation time stamp

    online.dataLogging = true;
  }
  else
    online.dataLogging = false;


  //  else if (settings.logData == false && online.microSD == true)
  //  {
  //      online.dataLogging = false;
  //  }
  //  else if (online.microSD == false)
  //  {
  //    Serial.println(F("Data logging disabled because microSD offline"));
  //    online.serialLogging = false;
  //  }
  //  else
  //  {
  //    Serial.println(F("Unknown microSD state"));
  //  }
}

void updateDataFileCreate()
{
  if (rtcHasBeenSyncd == true) //Update the create time stamp if the RTC is valid
  {
    myRTC.getTime(); //Get the RTC time so we can use it to update the create time
    //Update the file create time
    bool result = gnssDataFile.timestamp(T_CREATE, (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds);
    if (settings.printMinorDebugMessages == true)
    {
      Serial.print(F("updateDataFileCreate: gnssDataFile.timestamp T_CREATE returned "));
      Serial.println(result);
    }
  }
}

void updateDataFileAccess()
{
  if (rtcHasBeenSyncd == true) //Update the write and access time stamps if RTC is valid
  {
    myRTC.getTime(); //Get the RTC time so we can use it to update the last modified time
    //Update the file access time
    bool result = gnssDataFile.timestamp(T_ACCESS, (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds);
    if (settings.printMinorDebugMessages == true)
    {
      Serial.print(F("updateDataFileAccess: gnssDataFile.timestamp T_ACCESS returned "));
      Serial.println(result);
    }
    //Update the file write time
    result = gnssDataFile.timestamp(T_WRITE, (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds);
    if (settings.printMinorDebugMessages == true)
    {
      Serial.print(F("updateDataFileAccess: gnssDataFile.timestamp T_WRITE returned "));
      Serial.println(result);
    }
  }
}

void updateDataFileWrite()
{
  if (rtcHasBeenSyncd == true) //Update the write time stamp if RTC is valid
  {
    myRTC.getTime(); //Get the RTC time so we can use it to update the last modified time
    //Update the file write time
    bool result = gnssDataFile.timestamp(T_WRITE, (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds);
    if (settings.printMinorDebugMessages == true)
    {
      Serial.print(F("updateDataFileAccess: gnssDataFile.timestamp T_WRITE returned "));
      Serial.println(result);
    }
  }
}

void printUint64(uint64_t val)
{
  Serial.print("0x");
  uint8_t Byte = (val >> 56) & 0xFF;
  Serial.print(Byte, HEX);
  Byte = (val >> 48) & 0xFF;
  Serial.print(Byte, HEX);
  Byte = (val >> 40) & 0xFF;
  Serial.print(Byte, HEX);
  Byte = (val >> 32) & 0xFF;
  Serial.print(Byte, HEX);
  Byte = (val >> 24) & 0xFF;
  Serial.print(Byte, HEX);
  Byte = (val >> 16) & 0xFF;
  Serial.print(Byte, HEX);
  Byte = (val >> 8) & 0xFF;
  Serial.print(Byte, HEX);
  Byte = (val >> 0) & 0xFF;
  Serial.println(Byte, HEX);
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
