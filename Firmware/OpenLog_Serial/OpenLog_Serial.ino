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
#define SD_CHIP_SELECT 10

SdFat sd;
SdFile workingFile; //File that all data is written to
SdFile serialLog; //Record all incoming serial to this file
char fileName[9];
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Add RTC interface for Artemis
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include "RTC.h" //Include RTC library included with the Aruino_Apollo3 core
APM3_RTC myRTC; //Create instance of RTC class
bool log_RTC = true;
bool online_RTC = true; //It's always online
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

const byte statLED = 19; //Should not be the SPI SCK pin 13 on most boards

//Blinking LED error codes
#define ERROR_SD_INIT     3
#define ERROR_NEW_BAUD    5
#define ERROR_CARD_INIT   6
#define ERROR_VOLUME_INIT 7
#define ERROR_ROOT_INIT   8
#define ERROR_FILE_OPEN   9

//Uart Serial1(1, 13, 12); // RX, TX. Declares a Uart object called Serial1 using instance 1 of Apollo3 UART peripherals with RX on pin 25 and TX on pin 24 (note, you specify *pins* not Apollo3 pads. This uses the variant's pin map to determine the Apollo3 pad)

void setup() {
  Serial.begin(500000);
  Serial.println();
  Serial.println("Artemis OpenLog");

  Serial1.begin(115200);

  //Power up SD
  pinMode(23, OUTPUT);
  digitalWrite(23, HIGH);

  //myRTC.setToCompilerTime(); //Easily set RTC using the system __DATE__ and __TIME__ macros from compiler
  //myRTC.setTime(7, 28, 51, 0, 21, 10, 15); //Manually set RTC back to the future: Oct 21st, 2015 at 7:28.51 AM

  pinMode(statLED, OUTPUT);
  digitalWrite(statLED, LOW);

  if (sd.begin(SD_CHIP_SELECT, SD_SCK_MHZ(24)) == false)
  {
    //systemError(ERROR_CARD_INIT);
    Serial.println("SD init failed");
    while (1);
  }
  if (sd.chdir() == false)
  {
    //systemError(ERROR_ROOT_INIT); //Change to root directory. All new file creation will be in root.
    Serial.println("SD chdr failed");
    while (1);
  }

  sprintf(fileName, "serLog.txt");

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (!serialLog.open(fileName, O_CREAT | O_APPEND | O_WRITE)) systemError(ERROR_FILE_OPEN);

  overallStartTime = millis();
}

int total = 0;

void loop() {
  
  //Check for incoming serial to record to serial log file
  if (Serial1.available())
  {
    char temp[512];

    int counter = 0;
    while(Serial1.available())
    {
      temp[counter++] = Serial1.read();
      if(counter == 512) break;
    }

    total += counter;
      
    serialLog.write(temp, counter); //Record the buffer to the card
    serialLog.sync();
  }

  //Delay until we reach the requested read rate
  if (millis() - overallStartTime > 500)
  {
    overallStartTime = millis();
    if (digitalRead(statLED) == HIGH)
      digitalWrite(statLED, LOW);
    else
      digitalWrite(statLED, HIGH);
    Serial.println("Total: " + (String)total);
  }
}
