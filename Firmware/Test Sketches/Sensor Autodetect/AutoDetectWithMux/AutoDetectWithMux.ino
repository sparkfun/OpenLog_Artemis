/*
  This sketch serves as the MVP of the new node model for autodection. It demonstrates
  -Discovery
  -Data harvesting
  -Setting file record/lookup
  -Configuration menu

   For this sketch, only CCS811, BME280, VL53L1X and Mux's are auto identified.

  Autodetect theory of operation:

  The TCA9548A I2C mux introduces a can of worms but enables multiple (up to 64) of
  a single I2C device to be connected to a given I2C bus. You just have to turn on/off
  a given port while you communicate with said device.

  This is how the autodection algorithm works:
   Scan bus for muxes (0x70 to 0x77)
   Begin() any muxes. This causes them to turn off all their ports.
   With any possible muxes turned off, finish scanning main branch
   Any detected device is stored as a node in a linked list containing their address and device type,
   If muxes are found, begin scanning mux0/port0. Any new device is stored with their address and mux address/port.
   Begin() all devices in our linked list. Connections through muxes are performed as needed.

  All of this works and has the side benefit of enabling regular devices, that support multiple address, to
  auto-detect, begin(), and behave as before, but now in multiples.

  Future work:

  Theoretically you could attach 8 muxes configured to 0x71 off the 8 ports of an 0x70 mux. We could
  do this for other muxes as well to create a mux monster:
   - 0x70 - (port 0) 0x71 - 8 ports - device * 8
                     0x72 - 8 ports - device * 8
                     0x73 - 8 ports - device * 8
                     0x74 - 8 ports - device * 8
                     0x75 - 8 ports - device * 8
                     0x76 - 8 ports - device * 8
                     0x77 - 8 ports - device * 8
  This would allow a maximum of 8 * 7 * 8 = 448 of the *same* I2C device address to be
  connected at once. We don't support this sub-muxing right now. So the max we support
  is 64 identical address devices. That should be enough.
*/

#include "settings.h"

#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9

#include "SparkFun_I2C_Mux_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_I2C_Mux
#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
#include "SparkFunBME280.h" //Click here to get the library: http://librarymanager/All#SparkFun_BME280

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

char outputData[1000];

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

  detectQwiicDevices();

  loadDeviceSettingsFromFile();

  bubbleSortDevices(head);

  printDetectedDevices();

  printHelperText();
}

int counter = 0;

void loop()
{
  gatherDeviceValues();
  Serial.print(outputData);
  Serial.printf("%d,", counter++);
  Serial.println();
  delay(500);

  if (Serial.available())
  {
    while (Serial.available()) Serial.read();
    menuAttachedDevices();

    recordDeviceSettingsToFile();

    delay(10);
    while (Serial.available()) Serial.read();
  }
}

//Bubble sort the given linked list
//https://www.geeksforgeeks.org/c-program-bubble-sort-linked-list/
void bubbleSortDevices(struct node *start)
{
  int swapped, i;
  struct node *ptr1;
  struct node *lptr = NULL;

  //Checking for empty list
  if (start == NULL) return;

  do
  {
    swapped = 0;
    ptr1 = start;

    while (ptr1->next != lptr)
    {
      if (ptr1->address > ptr1->next->address)
      {
        swap(ptr1, ptr1->next);
        swapped = 1;
      }
      ptr1 = ptr1->next;
    }
    lptr = ptr1;
  }
  while (swapped);
}

//function to swap data of two nodes a and b
void swap(struct node *a, struct node *b)
{
//  int temp = a->data;
//  a->data = b->data;
//  b->data = temp;

  node temp;

  temp.deviceType = a->deviceType;
  temp.address = a->address;
  temp.portNumber = a->portNumber;
  temp.muxAddress = a->muxAddress;
  temp.online = a->online;
  temp.classPtr = a->classPtr;
  temp.configPtr = a->configPtr;
  //temp.next = a->next; //Nope

  a->deviceType = b->deviceType;
  a->address = b->address;
  a->portNumber = b->portNumber;
  a->muxAddress = b->muxAddress;
  a->online = b->online;
  a->classPtr = b->classPtr;
  a->configPtr = b->configPtr;

  b->deviceType = temp.deviceType;
  b->address = temp.address;
  b->portNumber = temp.portNumber;
  b->muxAddress = temp.muxAddress;
  b->online = temp.online;
  b->classPtr = temp.classPtr;
  b->configPtr = temp.configPtr;
  
}

//Step through the node list and print helper text for the enabled readings
void printHelperText()
{
  char helperText[1000];
  helperText[0] = '\0';

  //Step through list, printing values as we go
  node *temp = head;
  while (temp != NULL)
  {

    //If this node successfully begin()'d
    if (temp->online == true)
    {
      //Switch on device type to set proper class and setting struct
      switch (temp->deviceType)
      {
        case DEVICE_MULTIPLEXER:
          {
            //No data to print for a mux
          }
          break;
        case DEVICE_DISTANCE_VL53L1X:
          {
            struct_VL53L1X *nodeSetting = (struct_VL53L1X *)temp->configPtr; //Create a local pointer that points to same spot as node does
            if (nodeSetting->logDistance)
              strcat(helperText, "distance_mm,");
            if (nodeSetting->logRangeStatus)
              strcat(helperText, "distance_rangeStatus(0=good),");
            if (nodeSetting->logSignalRate)
              strcat(helperText, "distance_signalRate,");
          }
          break;
        case DEVICE_PHT_BME280:
          {
            struct_BME280 *nodeSetting = (struct_BME280 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            if (nodeSetting->logPressure)
              strcat(helperText, "pressure_Pa,");
            if (nodeSetting->logHumidity)
              strcat(helperText, "humidity_%,");
            if (nodeSetting->logAltitude)
              strcat(helperText, "altitude_m,");
            if (nodeSetting->logTemperature)
              strcat(helperText, "temp_degC,");
          }
          break;
        case DEVICE_VOC_CCS811:
          {
            struct_CCS811 *nodeSetting = (struct_CCS811 *)temp->configPtr; //Create a local pointer that points to same spot as node does
            if (nodeSetting->logTVOC)
              strcat(helperText, "tvoc_ppb,");
            if (nodeSetting->logCO2)
              strcat(helperText, "co2_ppm,");
          }
          break;
        default:
          Serial.printf("printerHelperText device not found: %d", temp->deviceType);
          break;
      }
    }
    temp = temp->next;
  }

  Serial.println(helperText);
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
