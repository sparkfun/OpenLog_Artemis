/*
  This sketch is to bring in the logging options for each sensor instantiation
*/

#include "settings.h"

#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9

#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

const byte PIN_QWIIC_POWER = 18;

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //We use SdFat-Beta from Bill Greiman for increased read/write speed: https://github.com/greiman/SdFat-beta

const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_MICROSD_POWER = 15; //x04

#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz
#define SD_CONFIG_MAX_SPEED SdSpiConfig(PIN_MICROSD_CHIP_SELECT, DEDICATED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz

//#define USE_EXFAT 1

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
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

const byte menuTimeout = 45; //Menus will exit/timeout after this number of seconds

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("I2C Multi Detection");
  Serial.flush();

  qwiicPowerOn();
  qwiic.begin();

  SPI.begin(); //Needed if SD is disabled

  beginSD();

  addDevice(0x5B, DEVICE_VOC_CCS811);
  addDevice(0x5A, DEVICE_VOC_CCS811);
  addDevice(0x29, DEVICE_DISTANCE_VL53L1X);

  loadSettingsFromFile();
}

void loop()
{
  printDeviceValues();
  Serial.println();
  delay(250);

  if (Serial.available())
  {
    while (Serial.available()) Serial.read();
    menuAttachedDevices();

    recordSettingsToFile();

    delay(10);
    while (Serial.available()) Serial.read();
  }
}

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  digitalWrite(PIN_QWIIC_POWER, LOW);
}
void qwiicPowerOff()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  digitalWrite(PIN_QWIIC_POWER, HIGH);
}

void microSDPowerOn()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, LOW);
}
void microSDPowerOff()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, HIGH);
}
