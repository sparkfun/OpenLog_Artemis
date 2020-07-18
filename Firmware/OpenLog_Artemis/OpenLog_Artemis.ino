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
  Test NAU7802s
  Test SCD30s
  Add a 'does not like to be powered cycled' setting for each device type.
  (done) Add support for logging VIN
  (done) Investigate error in time between logs (https://github.com/sparkfun/OpenLog_Artemis/issues/13)
  (done) Invesigate RTC reset issue (https://github.com/sparkfun/OpenLog_Artemis/issues/13 + https://forum.sparkfun.com/viewtopic.php?f=123&t=53157)
    The solution is to make sure that the OLA goes into deep sleep as soon as the voltage monitor detects that the power has failed.
    The user will need to press the reset button once power has been restored. Using the WDT to check the monitor and do a POR wasn't reliable.
  (done) Investigate requires-reset issue on battery power (") (X04 + CCS811/BME280 enviro combo)
  (done) Add a fix so that the MS8607 does not also appear as an MS5637
  (done) Add "set RTC from GPS" functionality
  (done) Add UTCoffset functionality (including support for negative numbers)
  Figure out how to give the u-blox time to establish a fix if it has been powered down between log intervals.
    Maybe add a waitForValidFix feature? Or maybe we can work around using a big value for "Set Qwiic bus power up delay"?
  Add support for VREG_ENABLE
  (done) Add support for PWR_LED
  (done) Use the WDT to reset the Artemis when power is reconnected (previously the Artemis would have stayed in deep sleep)
  Add a callback function to the u-blox library so we can abort waiting for UBX data if the power goes low
  (done) Add support for the ADS122C04 ADC (Qwiic PT100)
  Investigate why usBetweenReadings appears to be ~0.8s longer than expected
  (done) Correct u-blox pull-ups
  (done) Add an olaIdentifier to prevent problems when using two code variants that have the same sizeOfSettings
  Add a fix for the IMU wake-up issue identified in https://github.com/sparkfun/OpenLog_Artemis/issues/18
  (done)Add a "stop logging" feature on GPIO 32: allow the pin to be used to read a stop logging button instead of being an analog input
*/


const int FIRMWARE_VERSION_MAJOR = 1;
const int FIRMWARE_VERSION_MINOR = 4;

//Define the OLA board identifier:
//  This is an int which is unique to this variant of the OLA and which allows us
//  to make sure that the settings in EEPROM are correct for this version of the OLA
//  (sizeOfSettings is not necessarily unique and we want to avoid problems when swapping from one variant to another)
//  It is the sum of:
//    the variant * 0x100 (OLA = 1; GNSS_LOGGER = 2; GEOPHONE_LOGGER = 3)
//    the major firmware version * 0x10
//    the minor firmware version
#define OLA_IDENTIFIER 0x114

#include "settings.h"

//Define the pin functions
//Depends on hardware version. This can be found as a marking on the PCB.
//x04 was the SparkX 'black' version.
//v10 was the first red version.
//(Hardware versions 0-5 and 0-6 do not actually exist. It is just an easy way to test out new features on X04.)
#define HARDWARE_VERSION_MAJOR 0
#define HARDWARE_VERSION_MINOR 4

#if(HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 4)
const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_IMU_POWER = 22;
#elif(HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 5)
const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_IMU_POWER = 22;
#elif(HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 6)
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
//const byte PIN_LOGIC_DEBUG = 11;
const byte PIN_MICROSD_POWER = 15;
const byte PIN_QWIIC_POWER = 18;
const byte PIN_STAT_LED = 19;
const byte PIN_IMU_INT = 37;
const byte PIN_IMU_CHIP_SELECT = 44;
const byte PIN_STOP_LOGGING = 32;

enum returnStatus {
  STATUS_GETBYTE_TIMEOUT = 255,
  STATUS_GETNUMBER_TIMEOUT = -123455555,
  STATUS_PRESSED_X,
};

//Setup Qwiic Port
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9
#define QWIIC_PULLUPS 1 // Default to 1k pull-ups on the Qwiic bus
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//EEPROM for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <EEPROM.h>
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //SdFat (FAT32) by Bill Greiman: http://librarymanager/All#SdFat
SdFat sd;
SdFile sensorDataFile; //File that all sensor data is written to
SdFile serialDataFile; //File that all incoming serial data is written to
//#define PRINT_LAST_WRITE_TIME // Uncomment this line to enable the 'measure the time between writes' diagnostic

char sensorDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
char serialDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
const int sdPowerDownDelay = 100; //Delay for this many ms before turning off the SD card power
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
#include "ICM_20948.h"  // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_IMU
ICM_20948_SPI myICM;
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
#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_NAU7802
#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
#include "SparkFun_Qwiic_Humidity_AHT20.h" //Click here to get the library: http://librarymanager/All#Qwiic_Humidity_AHT20 by SparkFun
#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
#include "SparkFun_ADS122C04_ADC_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_ADS122C04

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Global variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
uint64_t measurementStartTime; //Used to calc the actual update rate. Max is ~80,000,000ms in a 24 hour period.
uint64_t lastSDFileNameChangeTime; //Used to calculate the interval since the last SD filename change
unsigned long measurementCount = 0; //Used to calc the actual update rate.
char outputData[512 * 2]; //Factor of 512 for easier recording to SD in 512 chunks
unsigned long lastReadTime = 0; //Used to delay until user wants to record a new reading
unsigned long lastDataLogSyncTime = 0; //Used to record to SD every half second
unsigned int totalCharactersPrinted = 0; //Limit output rate based on baud rate and number of characters to print
bool takeReading = true; //Goes true when enough time has passed between readings or we've woken from sleep
const uint64_t maxUsBeforeSleep = 2000000ULL; //Number of us between readings before sleep is activated.
const byte menuTimeout = 15; //Menus will exit/timeout after this number of seconds
volatile static bool stopLoggingSeen = false; //Flag to indicate if we should stop logging
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//unsigned long startTime = 0;

#define DUMP(varname) {Serial.printf("%s: %llu\n", #varname, varname);}

void setup() {
  //If 3.3V rail drops below 3V, system will power down and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up
  
  delay(1); // Let PIN_POWER_LOSS stabilize

  if (digitalRead(PIN_POWER_LOSS) == LOW) powerDown(); //Check PIN_POWER_LOSS just in case we missed the falling edge
  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);

  powerLEDOn(); // Turn the power LED on - if the hardware supports it
  
  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, HIGH); // Turn the STAT LED on while we configure everything

  Serial.begin(115200); //Default for initial debug messages if necessary
  Serial.println();

  SPI.begin(); //Needed if SD is disabled

  beginSD(); //285 - 293ms

  loadSettings(); //50 - 250ms

  Serial.flush(); //Complete any previous prints
  Serial.begin(settings.serialTerminalBaudRate);
  Serial.printf("Artemis OpenLog v%d.%d\n", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR);

  if (settings.useGPIO32ForStopLogging == true)
  {
    Serial.println("Stop Logging is enabled. Pull GPIO pin 32 to GND to stop logging.");
    pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
    delay(1); // Let the pin stabilize
    attachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING), stopLoggingISR, FALLING); // Enable the interrupt
    stopLoggingSeen = false; // Make sure the flag is clear
  }

  beginQwiic();
  delay(settings.qwiicBusPowerUpDelayMs); // Give the qwiic bus time to power up

  analogReadResolution(14); //Increase from default of 10

  beginDataLogging(); //180ms
  lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change

  beginSerialLogging(); //20 - 99ms

  beginIMU(); //61ms

  if (online.microSD == true) Serial.println("SD card online");
  else Serial.println("SD card offline");

  if (online.dataLogging == true) Serial.println("Data logging online");
  else Serial.println("Datalogging offline");

  if (online.serialLogging == true) Serial.println("Serial logging online");
  else Serial.println("Serial logging offline");

  if (online.IMU == true) Serial.println("IMU online");
  else Serial.println("IMU offline");

  if (settings.logMaxRate == true) Serial.println("Logging analog pins at max data rate");

  if (settings.enableTerminalOutput == false && settings.logData == true) Serial.println("Logging to microSD card with no terminal output");

  if (detectQwiicDevices() == true) //159 - 865ms but varies based on number of devices attached
  {
    beginQwiicDevices(); //Begin() each device in the node list
    loadDeviceSettingsFromFile(); //Load config settings into node list
    configureQwiicDevices(); //Apply config settings to each device in the node list
    printOnlineDevice();
  }
  else
    Serial.println("No Qwiic devices detected");

  if (settings.showHelperText == true) printHelperText(false); //printHelperText to terminal and sensor file

  //If we are sleeping between readings then we cannot rely on millis() as it is powered down. Used RTC instead.
  if (settings.usBetweenReadings >= maxUsBeforeSleep)
    measurementStartTime = rtcMillis();
  else
    measurementStartTime = millis();

  //Serial.printf("Setup time: %.02f ms\n", (micros() - startTime) / 1000.0);

  digitalWrite(PIN_STAT_LED, LOW); // Turn the STAT LED off now that everything is configured

  //If we are immediately going to go to sleep after the first reading then
  //first present the user with the config menu in case they need to change something
  if (settings.usBetweenReadings >= maxUsBeforeSleep)
    menuMain();
}

void loop() {
  if (Serial.available()) menuMain(); //Present user menu

  if (settings.logSerial == true && online.serialLogging == true)
  {
    if (SerialLog.available())
    {
      while (SerialLog.available())
      {
        incomingBuffer[incomingBufferSpot++] = SerialLog.read();
        if (incomingBufferSpot == sizeof(incomingBuffer))
        {
          digitalWrite(PIN_STAT_LED, HIGH);
          serialDataFile.write(incomingBuffer, sizeof(incomingBuffer)); //Record the buffer to the card
          digitalWrite(PIN_STAT_LED, LOW);
          incomingBufferSpot = 0;
        }
        charsReceived++;
      }

      lastSeriaLogSyncTime = millis(); //Reset the last sync time to now
      newSerialData = true;

      //Toggle stat LED indicating log recording
    }
    else if (newSerialData == true)
    {
      if ((millis() - lastSeriaLogSyncTime) > MAX_IDLE_TIME_MSEC) //If we haven't received any characters recently then sync log file
      {
        if (incomingBufferSpot > 0)
        {
          //Write the remainder of the buffer
          digitalWrite(PIN_STAT_LED, HIGH);
          serialDataFile.write(incomingBuffer, incomingBufferSpot); //Record the buffer to the card
          serialDataFile.sync();
          digitalWrite(PIN_STAT_LED, LOW);

          incomingBufferSpot = 0;
        }

        newSerialData = false;
        lastSeriaLogSyncTime = millis(); //Reset the last sync time to now
        printDebug("Total chars received: " + (String)charsReceived);
      }
    }
  }

  //micros() resets to 0 during sleep so only test if we are not sleeping
  if (settings.usBetweenReadings < maxUsBeforeSleep)
  {
    if ((micros() - lastReadTime) >= settings.usBetweenReadings)
      takeReading = true;
  }

  //Is it time to get new data?
  if (settings.logMaxRate == true || takeReading == true)
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
      
      //printDebug("ms since last write: " + (String)tempTime + "\n");
      printDebug("\n" + (String)tempTime + "\n");

      lastWriteTime = rtcMillis();
    }
#endif
    
    getData(); //Query all enabled sensors for data

    //Print to terminal
    if (settings.enableTerminalOutput == true)
      Serial.print(outputData); //Print to terminal

    //Record to SD
    if (settings.logData == true)
    {
      if (settings.enableSD && online.microSD)
      {
        digitalWrite(PIN_STAT_LED, HIGH);
        if (sensorDataFile.write(outputData, strlen(outputData)) != strlen(outputData)) //Record the buffer to the card
        {
          printDebug("*** sensorDataFile.write data length mismatch! ***\n");
        }

        //Force sync every 500ms
        if (millis() - lastDataLogSyncTime > 500)
        {
          lastDataLogSyncTime = millis();
          sensorDataFile.sync();
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
            sensorDataFile.close();
            strcpy(sensorDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));
            beginDataLogging(); //180ms
            if (settings.showHelperText == true) printHelperText(false); //printHelperText to terminal and sensor file
          }
          if (online.serialLogging == true)
          {
            serialDataFile.sync();
            serialDataFile.close();
            strcpy(serialDataFileName, findNextAvailableLog(settings.nextSerialLogNumber, "serialLog"));
            beginSerialLogging();
          }

          lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change
        }

        digitalWrite(PIN_STAT_LED, LOW);
      }
    }

    if ((settings.useGPIO32ForStopLogging == true) && (stopLoggingSeen == true)) // Has the user pressed the stop logging button?
    {
      stopLogging();
    }

#if((HARDWARE_VERSION_MAJOR != 0) || (HARDWARE_VERSION_MINOR != 5)) // Version 0-5 always sleeps!
    //Go to sleep only if time between readings is greater than maxUsBeforeSleep (2 seconds)
    if (settings.usBetweenReadings >= maxUsBeforeSleep)
#endif
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
  qwiic.setPullups(QWIIC_PULLUPS); //Just to make it really clear what pull-ups are being used, set pullups here.
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
    for (int i = 0; i < 10; i++) //Wait
    {
      delay(1);
    }

    if (sd.begin(PIN_MICROSD_CHIP_SELECT, SD_SCK_MHZ(24)) == false) //Standard SdFat
    {
      printDebug("SD init failed (first attempt). Trying again...\n");
      for (int i = 0; i < 250; i++) //Give SD more time to power up, then try again
      {
        delay(1);
      }
      if (sd.begin(PIN_MICROSD_CHIP_SELECT, SD_SCK_MHZ(24)) == false) //Standard SdFat
      {
        Serial.println("SD init failed (second attempt). Is card present? Formatted?");
        digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected
        online.microSD = false;
        return;
      }
    }

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

  if (settings.enableIMU == true && settings.logMaxRate == false)
  {
    //Reset ICM by power cycling it
    imuPowerOff();
    for (int i = 0; i < 10; i++) //10 is fine
    {
      delay(1);
    }
    imuPowerOn();
    for (int i = 0; i < 25; i++) //Allow ICM to come online. Typical is 11ms. Max is 100ms. https://cdn.sparkfun.com/assets/7/f/e/c/d/DS-000189-ICM-20948-v1.3.pdf
    {
      delay(1);
    }

    myICM.begin(PIN_IMU_CHIP_SELECT, SPI, 4000000); //Set IMU SPI rate to 4MHz
    if (myICM.status != ICM_20948_Stat_Ok)
    {
      //Try one more time with longer wait

      //Reset ICM by power cycling it
      imuPowerOff();
      for (int i = 0; i < 10; i++) //10 is fine
      {
        delay(1);
      }
      imuPowerOn();
      for (int i = 0; i < 100; i++) //Allow ICM to come online. Typical is 11ms. Max is 100ms.
      {
        delay(1);
      }

      myICM.begin(PIN_IMU_CHIP_SELECT, SPI, 4000000); //Set IMU SPI rate to 4MHz
      if (myICM.status != ICM_20948_Stat_Ok)
      {
        digitalWrite(PIN_IMU_CHIP_SELECT, HIGH); //Be sure IMU is deselected
        Serial.println("ICM-20948 failed to init.");
        imuPowerOff();
        online.IMU = false;
        return;
      }
    }

    online.IMU = true;
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
      Serial.println("Failed to create sensor data file");
      online.dataLogging = false;
      return;
    }

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
      Serial.println("Failed to create serial log file");
      //systemError(ERROR_FILE_OPEN);
      online.serialLogging = false;
      return;
    }

    //pinMode(13, INPUT);

    SerialLog.begin(settings.serialLogBaudRate);

    online.serialLogging = true;
  }
  else
    online.serialLogging = false;
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

//Stop Logging ISR
void stopLoggingISR(void)
{
  stopLoggingSeen = true;
}
