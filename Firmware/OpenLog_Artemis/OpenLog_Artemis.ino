/*
  Skimmer Alert System
  By: Nathan Seidle
  SparkFun Electronics
  Date: November 26th, 2019
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).
  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/15793

  This firmware runs the OpenLog Artemis. A large variety of system settings can be
  adjusted by connecting at 115200bps.

  TODO:
  get time stamps from GPS
  manual set RTC to GPS

  disable local sensor logging, log only serial
  (done) disable terminal output for max analog logging
  (nah) Change filename(s) to record to (data vs serial logging)
  (done) Auto number the datalog and serial log file names
  Enable/disable time stamping of incoming serial
  Toggle LED on serial data recording vs sensor recording
  If VCC is detected as dropping below 3V (diode drop from batt backup) then go into shutdown
  (done) reset all settings to default
  Check out the file creation. Use FILE_WRITE instead of the O_s. Might go faster without append...
  Support multiples of a given sensor. How to support two MCP9600s attached at the same time?
  Allow user to export the current settings to a settings.txt file that the can use to setup other OpenLogs
  Setup a sleep timer, wake up ever 5 seconds, power up Qwiic, take reading, power down I2C bus, sleep.
  Could you store the date from the RTC because it won't change that much?

  Eval how long it takes to boot (SD, log creation, IMU begin, etc)

  What about changing units? mm of distance vs ft or inches? Leave it up to post processing?
  GPS: Record bare NMEA over I2C?
  GPS: Turn on off SIV, date/time, etc.
  enable logging of SIV from GPS

  Python/windows program to load new hex files

  Add MOSFET control of I2C 3.3V line to turn off the bus if needed

  What happens when user enables analog on pin 12/tx and keeps serial on pin 13/rx and then changes baud rate? Does analog still work?
  What happens when user enables serial on 13/rx then enables analog read on pin 12/tx? Does analog still work?

  Max rates:
  ~1140Hz for 3 channel analog, max data rate
*/


/*
  Sensors could be detected at power on
  We then begin sensors
  But user could then run 6) detect sensors.
  So when do we run Begin on sensors?

  We need to call startSensors at the exit of main menu
  If a device is available, logging, and not online, then start/online it.

  We need a function that determines the max I2C speed. MCP9600 is 100kHz.
  Call at end of startSensors.
  Max speed is 400kHz. If MCP9600 is online, set max to 100kHz.



*/


#include "settings.h"

const byte statLED = 19;

//Setup Qwiic Port
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "Wire.h"
TwoWire qwiic(1); //Will use pads 8/9
//#define qwiic Wire
#define QWIIC_PWR 18 //X02
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//EEPROM for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <EEPROM.h>
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //We use SdFat-Beta from Bill Greiman for increased read/write speed
#define SD_CHIP_SELECT 10
#define SD_POWER 23
#define SD_CONFIG SdSpiConfig(SD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz
#define SD_CONFIG_MAX_SPEED SdSpiConfig(SD_CHIP_SELECT, DEDICATED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz

SdFat sd;
File sensorDataFile; //File that all sensor data is written to
File serialDataFile; //File that all incoming serial data is written to
bool newSerialData = false;
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
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Add ICM IMU interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "ICM_20948.h"  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
//#define IMU_CHIP_SELECT 18 //X01
#define IMU_CHIP_SELECT 44 //X02
#define IMU_POWER 22
//#define IMU_INT 17 //X01
#define IMU_INT 37 //X02
ICM_20948_SPI myICM;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Header files for all possible Qwiic sensors
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SparkFun_LPS25HB_Arduino_Library.h>  //Click here to get the library: http://librarymanager/All#SparkFun_LPS25HB
LPS25HB pressureSensor;

#include <SparkFun_MCP9600.h>  //Click here to get the library: http://librarymanager/All#SparkFun_MCP9600
MCP9600 thermoSensor;

#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_NAU7802
NAU7802 nauScale;

#include <SparkFun_BH1749NUC_Arduino_Library.h>
BH1749NUC rgb;

#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
SFEVL53L1X distanceSensor;

#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS myGPS;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
unsigned long measurementStartTime; //Used to calc the actual update rate.
unsigned long measurementCount = 0; //Used to calc the actual update rate.
String outputData;
unsigned long lastReadTime = 0; //Used to delay until user wants to record a new reading
unsigned long lastDataLogSyncTime = 0; //Used to record to SD every half second
bool helperTextPrinted = false; //Print the column headers only once
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("Artemis OpenLog");

  pinMode(statLED, OUTPUT);
  digitalWrite(statLED, LOW);

  loadSettings();

  pinMode(QWIIC_PWR, OUTPUT);
  qwiicPowerOn();
  qwiic.begin();
  //qwiic.setPullups(24); //Set pullups to 24k
  qwiic.setPullups(0); //Disable pullups

  SPI.begin(); //Needed if SD is disabled

  analogReadResolution(14); //Increase from default of 10

  //settings.serialBaudRate = 115200;
  //myRTC.setToCompilerTime(); //Set RTC using the system __DATE__ and __TIME__ macros from compiler

  beginSD();

  beginSerialLogging();

  beginIMU();

  beginSensors();

  measurementStartTime = millis();

  if (settings.logMaxRate == true) Serial.println("Logging analog pins at max data rate");
}

void loop() {

  if (Serial.available()) menuMain(); //Present user menu

  if (settings.logSerial == true && online.serialLogging == true)
  {
    if (SerialLog.available())
    {
      char temp[256];
      uint16_t counter = 0;
      while (SerialLog.available())
      {
        temp[counter++] = SerialLog.read();
        if (counter == 512) break;
      }

      serialDataFile.write(temp, counter); //Record the buffer to the card
      lastSeriaLogSyncTime = millis(); //Reset the last sync time to now
      newSerialData = true;

      //Toggle stat LED indicating log recording
    }
    //No characters received?
    else if (newSerialData == true)
    {
      if ((millis() - lastSeriaLogSyncTime) > MAX_IDLE_TIME_MSEC) //If we haven't received any characters recently then sync log file
      {
        newSerialData = false;
        serialDataFile.sync();
        lastSeriaLogSyncTime = millis(); //Reset the last sync time to now
      }
    }
  }

  //Is it time to get new data?
  if (settings.logMaxRate == true || (millis() - lastReadTime) >= (1000UL / settings.recordPerSecond))
  {
    lastReadTime = millis();

    getData(); //Query all enabled sensors for data

    //Print to terminal
    if (settings.enableTerminalOutput == true)
      Serial.print(outputData); //Print to terminal

    //Record to SD
    if (settings.enableSD && online.microSD)
    {
      char temp[512];
      outputData.toCharArray(temp, 512); //Convert string to char array so sdfat can record it
      sensorDataFile.write(temp, strlen(temp)); //Record the buffer to the card

      //Force sync every 500ms
      if (millis() - lastDataLogSyncTime > 500)
      {
        lastDataLogSyncTime = millis();
        digitalWrite(statLED, HIGH);
        sensorDataFile.sync();
        digitalWrite(statLED, LOW);
      }
    }

    //    if (digitalRead(statLED) == HIGH)
    //      digitalWrite(statLED, LOW);
    //    else
    //      digitalWrite(statLED, HIGH);
  }
}

void beginSD()
{
  if (settings.enableSD == true)
  {
    //Power up SD
    pinMode(SD_POWER, OUTPUT);
    digitalWrite(SD_POWER, HIGH);

    if (settings.logMaxRate == true)
    {
      if (sd.begin(SD_CONFIG_MAX_SPEED) == false) //Very Fast SdFat Beta (dedicated SPI, no IMU)
      {
        Serial.println("SD init failed. Is card present? Formatted?");
        online.microSD = false;
        return;
      }
    }
    else
    {
      if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
      {
        Serial.println("SD init failed. Is card present? Formatted?");
        online.microSD = false;
        return;
      }
    }

    if (sd.chdir() == false)
    {
      Serial.println("SD change directory failed");
      //systemError(ERROR_ROOT_INIT); //Change to root directory. All new file creation will be in root.
      online.microSD = false;
      return;
    }

    // O_CREAT - create the file if it does not exist
    // O_APPEND - seek to the end of the file prior to each write
    // O_WRITE - open for write

    if (sensorDataFile.open(findNextAvailableLog(settings.nextDataLogNumber, "dataLog"), O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println("Failed to create sensor data file");
      //systemError(ERROR_FILE_OPEN);
      online.microSD = false;
      return;
    }

    online.microSD = true;

    msg("SD card online");
  }
  else
  {
    //Power down SD
    pinMode(SD_POWER, OUTPUT);
    //digitalWrite(SD_POWER, LOW); //Unfortunately it seems powering down SD with card in place causes IMU to fail to init
    digitalWrite(SD_POWER, HIGH);

    //Be sure SD is deselected
    pinMode(SD_CHIP_SELECT, OUTPUT);
    digitalWrite(SD_CHIP_SELECT, HIGH);
    Serial.println("SD offline/disabled");
    online.microSD = false;
  }
}

void beginIMU()
{
  if (settings.enableIMU == true && settings.logMaxRate == false)
  {
    //Power up ICM
    pinMode(IMU_POWER, OUTPUT);
    digitalWrite(IMU_POWER, HIGH);

    myICM.begin(IMU_CHIP_SELECT, SPI, 4000000); //Set IMU SPI rate to 4MHz
    if (myICM.status != ICM_20948_Stat_Ok) {
      msg("ICM-20948 failed to init.");
      online.IMU = false;
      return;
    }

    online.IMU = true;
    msg("IMU online");
  }
  else
  {
    //Power down IMU
    pinMode(IMU_POWER, OUTPUT);
    digitalWrite(IMU_POWER, LOW);

    //Be sure IMU is deselected
    pinMode(IMU_CHIP_SELECT, OUTPUT);
    digitalWrite(IMU_CHIP_SELECT, HIGH);

    msg("IMU disabled");
    online.IMU = false;
  }
}

void beginSerialLogging()
{
  if (online.microSD == true && settings.logSerial == true)
  {
    if (serialDataFile.open(findNextAvailableLog(settings.nextSerialLogNumber, "serialLog"), O_CREAT | O_APPEND | O_WRITE) == false)
    {
      Serial.println("Failed to create serial log file");
      //systemError(ERROR_FILE_OPEN);
      online.serialLogging = false;
      return;
    }

    //pinMode(13, INPUT);

    SerialLog.begin(settings.serialBaudRate);

    msg("Serial logging online");
    online.serialLogging = true;
  }
  else if (settings.logSerial == false && online.microSD == true)
  {
    msg("Serial logging disabled");
    online.serialLogging = false;
  }
  else if (online.microSD == false)
  {
    Serial.println("Serial logging disabled because microSD offline");
    online.serialLogging = false;
  }
  else
  {
    Serial.println("Unknown microSD state");
  }
}
