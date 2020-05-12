/*
  This example shows how to write to a file

  Be sure to select "SparkFun RedBoard Artemis ATP" to get the pin assignments correct
*/

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //We use SdFat-Beta from Bill Greiman for increased read/write speed

const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_MICROSD_POWER = 15; //x04

#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz

SdFat sd;
File sensorDataFile; //File that all sensor data is written to

char sensorDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

const byte PIN_QWIIC_POWER = 18;
const byte PIN_STAT_LED = 19;
const byte PIN_IMU_POWER = 22;

void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Serial.println("SD Write Test");

  SPI.begin();

  beginSD();

  beginDataLogging();

  Serial.println("Setup done");
}

void loop(void)
{
  String outputData = "Test string\n";

  char temp[512];
  outputData.toCharArray(temp, 512); //Convert string to char array so sdfat can record it
  sensorDataFile.write(temp, strlen(temp)); //Record the buffer to the card
  sensorDataFile.sync();

  Serial.println("File recorded");

  while(1);
}
