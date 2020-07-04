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
  Test GPS - not sure about %d with int32s. Does lat, long, and alt look correct?
  Test NAU7802s
  Test SCD30s
  Add a 'does not like to be powered cycled' setting for each device type.
  (done) Add support for logging VIN
  (done) Investigate error in time between logs (https://github.com/sparkfun/OpenLog_Artemis/issues/13)
  Invesigate RTC reset issue (https://github.com/sparkfun/OpenLog_Artemis/issues/13 + https://forum.sparkfun.com/viewtopic.php?f=123&t=53157)
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
  Add support for the Qwiic PT100
  Investigate why usBetweenReadings appears to be ~0.8s longer than expected
*/


const int FIRMWARE_VERSION_MAJOR = 1;
const int FIRMWARE_VERSION_MINOR = 4;

#include "settings.h"

//Define the pin functions
//Depends on hardware version. This can be found as a marking on the PCB.
//x04 was the SparkX 'black' version.
//v10 was the first red version.
//(Hardware version 0-5 does not exist. It is just an easy way to test high speed logging on the x04 hardware.)
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

enum returnStatus {
  STATUS_GETBYTE_TIMEOUT = 255,
  STATUS_GETNUMBER_TIMEOUT = -123455555,
  STATUS_PRESSED_X,
};

//Setup Qwiic Port
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//EEPROM for storing settings
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <EEPROM.h>
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>

#include <SdFat.h> //We use SdFat-Beta from Bill Greiman for increased read/write speed: https://github.com/greiman/SdFat-beta

#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz
#define SD_CONFIG_MAX_SPEED SdSpiConfig(PIN_MICROSD_CHIP_SELECT, DEDICATED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz

#define USE_EXFAT 1
//#define PRINT_LAST_WRITE_TIME // Uncomment this line to enable the 'measure the time between writes' diagnostic

#ifdef USE_EXFAT
//ExFat
SdFs sd;
FsFile sensorDataFile; //File that all sensor data is written to
FsFile serialDataFile; //File that all incoming serial data is written to
#else
//Fat16/32
SdFat sd;
File sensorDataFile; //File that all sensor data is written to
File serialDataFile; //File that all incoming serial data is written to
#endif

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
volatile static bool wakeOnPowerReconnect = true; // volatile copy of settings.wakeOnPowerReconnect for the ISR
volatile static bool lowPowerSeen = false; // volatile flag to indicate if a low power interrupt has been seen
volatile static bool waitingForReset = false; // volatile flag to indicate if we are waiting for a reset (used by the WDT ISR)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//unsigned long startTime = 0;

#define DUMP(varname) {Serial.printf("%s: %llu\n", #varname, varname);}

void setup() {
  //If 3.3V rail drops below 3V, system will power down and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up
  
  delay(1); // Let PIN_POWER_LOSS stabilize

  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), lowPowerISR, FALLING);

  startWatchdog(); // Set up and start the WDT
  
  powerLEDOn(); // Turn the power LED on - if the hardware supports it
  
  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, HIGH); // Turn the STAT LED on while we configure everything

  if (digitalRead(PIN_POWER_LOSS) == LOW) powerDown(); //Check PIN_POWER_LOSS just in case we missed the falling edge

  Serial.begin(115200); //Default for initial debug messages if necessary
  Serial.println();

  SPI.begin(); //Needed if SD is disabled

  beginSD(); //285 - 293ms

  if (lowPowerSeen == true) powerDown(); //Power down if required

  loadSettings(); //50 - 250ms

  if (lowPowerSeen == true) powerDown(); //Power down if required

  Serial.flush(); //Complete any previous prints
  Serial.begin(settings.serialTerminalBaudRate);
  Serial.printf("Artemis OpenLog v%d.%d\n", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR);

  beginQwiic();

  analogReadResolution(14); //Increase from default of 10

  beginDataLogging(); //180ms
  lastSDFileNameChangeTime = rtcMillis(); // Record the time of the file name change

  if (lowPowerSeen == true) powerDown(); //Power down if required

  beginSerialLogging(); //20 - 99ms

  if (lowPowerSeen == true) powerDown(); //Power down if required

  beginIMU(); //61ms

  if (lowPowerSeen == true) powerDown(); //Power down if required

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
    if (lowPowerSeen == true) powerDown(); //Power down if required
    loadDeviceSettingsFromFile(); //Load config settings into node list
    if (lowPowerSeen == true) powerDown(); //Power down if required
    configureQwiicDevices(); //Apply config settings to each device in the node list
    if (lowPowerSeen == true) powerDown(); //Power down if required
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

  if (lowPowerSeen == true) powerDown(); //Power down if required

  //If we are immediately going to go to sleep after the first reading then
  //first present the user with the config menu in case they need to change something
  if (settings.usBetweenReadings >= maxUsBeforeSleep)
    menuMain();
}

void loop() {
  if (lowPowerSeen == true) powerDown(); //Power down if required
  
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
        if (lowPowerSeen == true) powerDown(); //Power down if required
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
      if (lowPowerSeen == true) powerDown(); //Power down if required
      delay(1);
    }

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
      printDebug("SD init failed (first attempt). Trying again...\n");
      for (int i = 0; i < 250; i++) //Give SD more time to power up, then try again
      {
        if (lowPowerSeen == true) powerDown(); //Power down if required
        delay(1);
      }
      if (sd.begin(SD_CONFIG) == false) //Slightly Faster SdFat Beta (we don't have dedicated SPI)
      {
        Serial.println("SD init failed (second attempt). Is card present? Formatted?");
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

  if (settings.enableIMU == true && settings.logMaxRate == false)
  {
    //Reset ICM by power cycling it
    imuPowerOff();
    for (int i = 0; i < 10; i++) //10 is fine
    {
      if (lowPowerSeen == true) powerDown(); //Power down if required
      delay(1);
    }
    imuPowerOn();
    for (int i = 0; i < 25; i++) //Allow ICM to come online. Typical is 11ms. Max is 100ms. https://cdn.sparkfun.com/assets/7/f/e/c/d/DS-000189-ICM-20948-v1.3.pdf
    {
      if (lowPowerSeen == true) powerDown(); //Power down if required
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
        if (lowPowerSeen == true) powerDown(); //Power down if required
        delay(1);
      }
      imuPowerOn();
      for (int i = 0; i < 100; i++) //Allow ICM to come online. Typical is 11ms. Max is 100ms.
      {
        if (lowPowerSeen == true) powerDown(); //Power down if required
        delay(1);
      }

      myICM.begin(PIN_IMU_CHIP_SELECT, SPI, 4000000); //Set IMU SPI rate to 4MHz
      if (myICM.status != ICM_20948_Stat_Ok)
      {
        digitalWrite(PIN_IMU_CHIP_SELECT, HIGH); //Be sure IMU is deselected
        msg("ICM-20948 failed to init.");
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


  //  else if (settings.logSerial == false && online.microSD == true)
  //  {
  //    msg("Serial logging disabled");
  //    online.serialLogging = false;
  //  }
  //  else if (online.microSD == false)
  //  {
  //    Serial.println("Serial logging disabled because microSD offline");
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

//WatchDog Timer code by Adam Garbo:
//https://forum.sparkfun.com/viewtopic.php?f=169&t=52431&p=213296#p213296

// Watchdog timer configuration structure.
am_hal_wdt_config_t g_sWatchdogConfig = {

  // Configuration values for generated watchdog timer event.
  .ui32Config = AM_HAL_WDT_LFRC_CLK_16HZ | AM_HAL_WDT_ENABLE_RESET | AM_HAL_WDT_ENABLE_INTERRUPT,

  // Number of watchdog timer ticks allowed before a watchdog interrupt event is generated.
  .ui16InterruptCount = 16, // Set WDT interrupt timeout for 1 second.

  // Number of watchdog timer ticks allowed before the watchdog will issue a system reset.
  .ui16ResetCount = 20 // Set WDT reset timeout for 1.25 seconds.
};

void startWatchdog()
{
  // Set the clock frequency.
  //am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0); // ap3_initialization.cpp does this - no need to do it here

  // Set the default cache configuration
  //am_hal_cachectrl_config(&am_hal_cachectrl_defaults); // ap3_initialization.cpp does this - no need to do it here
  //am_hal_cachectrl_enable(); // ap3_initialization.cpp does this - no need to do it here

  // Configure the board for low power operation.
  //am_bsp_low_power_init(); // ap3_initialization.cpp does this - no need to do it here

  // Clear reset status register for next time we reset.
  //am_hal_reset_control(AM_HAL_RESET_CONTROL_STATUSCLEAR, 0); // Nice - but we don't really care what caused the reset

  // LFRC must be turned on for this example as the watchdog only runs off of the LFRC.
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_LFRC_START, 0);

  // Configure the watchdog.
  am_hal_wdt_init(&g_sWatchdogConfig);

  // Enable the interrupt for the watchdog in the NVIC.
  NVIC_EnableIRQ(WDT_IRQn);
  //NVIC_SetPriority(WDT_IRQn, 0); // Set the interrupt priority to 0 = highest (255 = lowest)
  //am_hal_interrupt_master_enable(); // ap3_initialization.cpp does this - no need to do it here

  // Enable the watchdog.
  am_hal_wdt_start();
}

// Interrupt handler for the watchdog.
extern "C" void am_watchdog_isr(void) {
  // Clear the watchdog interrupt.
  am_hal_wdt_int_clear();

  // Always restart the watchdog unless wakeOnPowerReconnect is true and 
  // and waitingForReset is true and PIN_POWER_LOSS has gone high
  // (indicating power has been reapplied and we should let the WDT reset everything)
  if ((wakeOnPowerReconnect == false) || (waitingForReset == false) || (digitalRead(PIN_POWER_LOSS) == LOW))
  {
    // Restart the watchdog.
    am_hal_wdt_restart(); // "Pet" the dog.
  }
}
