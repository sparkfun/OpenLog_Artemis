/*
  OpenLog Artemis GNSS Logging
  By: Paul Clark (PaulZC)
  Date: May 19th, 2020

  Based on:
  OpenLog Artemis
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 26th, 2019
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).
  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15793

  This firmware runs the OpenLog Artemis and is dedicated to logging
  messages from the u-blox F9 and M9 GNSS receivers.
  
  Messages are streamed directly to SD without being processed.

  All GNSS configuration is done using UBX-CFG-VALSET, UBX-CFG-VALGET and
  UBX-CFG-VALDEL which is only supported on devices like the ZED-F9P and
  NEO-M9N running communication protocols greater than 23.01.

  TO DO:
  Check UBX-MON-VER to see which GNSS is connected and hence:
  - if UBX-NAV-RELPOSNED and UBX-RXM-RAWX are supported.
  - the minimum usBetweenReadings

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

#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS gpsSensor_ublox;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
uint64_t measurementStartTime; //Used to calc the actual update rate. Max is ~80,000,000ms in a 24 hour period.
unsigned long measurementCount = 0; //Used to calc the actual update rate.
String outputData;
String beginSensorOutput;
unsigned long lastReadTime = 0; //Used to delay until user wants to record a new reading
unsigned long lastDataLogSyncTime = 0; //Used to record to SD every half second
bool helperTextPrinted = false; //Print the column headers only once
unsigned int totalCharactersPrinted = 0; //Limit output rate based on baud rate and number of characters to print
bool takeReading = true; //Goes true when enough time has passed between readings or we've woken from sleep
const uint32_t maxUsBeforeSleep = 2000000; //Number of us between readings before sleep is activated.
const byte menuTimeout = 45; //Menus will exit/timeout after this number of seconds
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//unsigned long startTime = 0;

#define DUMP(varname) {Serial.printf("%s: %llu\n", #varname, varname);}

void setup() {
  //If 3.3V rail drops below 3V, system will power down and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);

  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, LOW);

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

  beginIMU(); //Disable IMU

  if (online.microSD == true) msg("SD card online");
  else msg("SD card offline");

  if (online.dataLogging == true) msg("Data logging online");
  else msg("Datalogging offline");

  if (settings.enableTerminalOutput == false && settings.logData == true) Serial.println("Logging to microSD card with no terminal output");

  if (beginSensors() == true) Serial.println(beginSensorOutput); //159 - 865ms but varies based on number of devices attached
  else msg("No sensors detected");

  //If we are sleeping between readings then we cannot rely on millis() as it is powered down. Used RTC instead.
  if (settings.usBetweenReadings >= maxUsBeforeSleep)
    measurementStartTime = rtcMillis();
  else
    measurementStartTime = millis();

  //Serial.printf("Setup time: %.02f ms\n", (micros() - startTime) / 1000.0);

  //If we are immediately going to go to sleep after the first reading then
  //first present the user with the config menu in case they need to change something
  if (settings.usBetweenReadings >= maxUsBeforeSleep)
    menuMain();
}

void loop() {
  if (Serial.available()) menuMain(); //Present user menu

  //micros() resets to 0 during sleep so only test if we are not sleeping
  if (settings.usBetweenReadings < maxUsBeforeSleep)
  {
    if ((micros() - lastReadTime) >= settings.usBetweenReadings)
      takeReading = true;
  }

  //Is it time to get new data?
  if (takeReading == true)
  {
    takeReading = false;
    lastReadTime = micros();

    getData(); //Query all enabled sensors for data

    //Print to terminal
    if (settings.enableTerminalOutput == true)
      Serial.print(outputData); //Print to terminal

    //Record to SD
    if (settings.logData == true)
    {
      if (settings.enableSD && online.microSD)
      {
        char temp[512];
        outputData.toCharArray(temp, 512); //Convert string to char array so sdfat can record it
        gnssDataFile.write(temp, strlen(temp)); //Record the buffer to the card

        //Force sync every 500ms
        if (millis() - lastDataLogSyncTime > 500)
        {
          lastDataLogSyncTime = millis();
          digitalWrite(PIN_STAT_LED, HIGH);
          gnssDataFile.sync();
          digitalWrite(PIN_STAT_LED, LOW);
        }
      }
    }

    //Go to sleep if time between readings is greater than 2 seconds
    if (settings.usBetweenReadings > maxUsBeforeSleep)
    {
      goToSleep();
    }
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
    //        Serial.println("SD init failed. Do you have the correct board selected in Arduino? Is card present? Formatted?");
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
        Serial.println("SD init failed. Is card present? Formatted?");
        digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected
        online.microSD = false;
        return;
      }
    }
    //    }

    //Change to root directory. All new file creation will be in root.
    if (sd.chdir() == false)
    {
      Serial.println("SD change directory failed");
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

void beginIMU()
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
    if (strlen(gnssDataFileName) == 0)
      strcpy(gnssDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));

    // O_CREAT - create the file if it does not exist
    // O_APPEND - seek to the end of the file prior to each write
    // O_WRITE - open for write
    if (gnssDataFile.open(gnssDataFileName, O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println("Failed to create sensor data file");
      online.dataLogging = false;
      return;
    }

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
  //    Serial.println("Data logging disabled because microSD offline");
  //    online.serialLogging = false;
  //  }
  //  else
  //  {
  //    Serial.println("Unknown microSD state");
  //  }
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
