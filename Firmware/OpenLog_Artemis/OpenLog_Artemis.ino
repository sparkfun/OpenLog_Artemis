/*
  October 11th, 2019

  Filename to record to
  What about changing units? mm of distance vs ft or inches? Leave it up to post processing?
  GPS: Record bare NMEA over I2C?
  GPS: Turn on off SIV, date/time, etc.
  
  TODO:
  Add ability to set RTC over terminal
  Change between US date sort and world sort
  Enable/disable DST

  Add MOSFET control of I2C 3.3V line to turn off the bus if needed
*/

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //We do not use the built-in SD.h file because it calls Serial.print
#define SD_CHIP_SELECT 4

SdFat sd;
SdFile workingFile; //File that all data is written to
char fileName[9];
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Add RTC interface for Artemis
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "RTC.h" //Include RTC library included with the Aruino_Apollo3 core
APM3_RTC myRTC; //Create instance of RTC class
bool log_RTC = true;
bool online_RTC = true; //It's always online
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Header files for all possible Qwiic sensors
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SparkFun_LPS25HB_Arduino_Library.h>  // Click here to get the library: http://librarymanager/All#SparkFun_LPS25HB
LPS25HB pressureSensor; // Create an object of the LPS25HB class
bool log_LPS25HB = true;
bool online_LPS25HB = false;

#include "SparkFun_VL53L1X.h"
SFEVL53L1X distanceSensor;
bool log_VL53L1X = true;
bool online_VL53L1X = false;

#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS myGPS;
bool log_uBlox = true;
bool online_uBlox = false;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Log record rate
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool log_Hertz = true; //Optional printing of Hz rate at end of data string
bool online_Hertz = true; //Always online
long overallStartTime; //Used to calc the actual update rate.
long updateCount = 0; //Used to calc the actual update rate.
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

String outputData;
unsigned int recordPerSecond = 2; //In Hz
unsigned int loopDelay = 1000UL / recordPerSecond;

const byte statLED = 14; //Should not be the SPI SCK pin 13 on most boards

//Blinking LED error codes
#define ERROR_SD_INIT     3
#define ERROR_NEW_BAUD    5
#define ERROR_CARD_INIT   6
#define ERROR_VOLUME_INIT 7
#define ERROR_ROOT_INIT   8
#define ERROR_FILE_OPEN   9


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Artemis OpenLog");

  myRTC.setToCompilerTime(); //Easily set RTC using the system __DATE__ and __TIME__ macros from compiler
  //myRTC.setTime(7, 28, 51, 0, 21, 10, 15); //Manually set RTC back to the future: Oct 21st, 2015 at 7:28.51 AM

  pinMode(statLED, OUTPUT);
  digitalWrite(statLED, LOW);

  Wire.begin();
  Wire.setClock(400000);

  if (sd.begin(SD_CHIP_SELECT, SD_SCK_MHZ(10)) == false)
  {
    //systemError(ERROR_CARD_INIT);
    Serial.println("SD init failed");
    while (1);
  }
  if (sd.chdir() == false)
  {
    //systemError(ERROR_ROOT_INIT); //Change to root directory. All new file creation will be in root.
    Serial.println("SD chdr failed");
    while(1);
  }

  sprintf(fileName, "data.txt");

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!workingFile.open(fileName, O_CREAT | O_APPEND | O_WRITE)) systemError(ERROR_FILE_OPEN);

  if (workingFile.fileSize() == 0) {
    //This is a trick to make sure first cluster is allocated - found in Bill's example/beta code
    workingFile.rewind();
    workingFile.sync();
  }

  beginSensors();

  overallStartTime = millis();
}

void loop() {

  unsigned long startTime = millis();

  getData(); //Query all enabled sensors for data

  Serial.print(outputData); //Print to terminal

  char temp[500];
  outputData.toCharArray(temp, 500); //Convert string to char array so sdfat can record it
  workingFile.write(temp, strlen(temp)); //Record the buffer to the card
  workingFile.sync();

  if (digitalRead(statLED) == HIGH)
    digitalWrite(statLED, LOW);
  else
    digitalWrite(statLED, HIGH);

  //Delay until we reach the requested read rate
  while (millis() - startTime < loopDelay)
    delay(1);
}
