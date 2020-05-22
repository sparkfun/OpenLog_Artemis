/*
  This sketch is to bring in the logging options for each sensor instantiation
*/

#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9

#include <SparkFun_I2C_Mux_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_I2C_Mux
QWIICMUX **multiplexer;

#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
SFEVL53L1X **distanceSensor_vl53l1x; //Create pointer to a set of pointers to the sensor class

#include "SparkFunBME280.h" //Click here to get the library: http://librarymanager/All#SparkFun_BME280
BME280 **phtSensor_BME280;

#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
CCS811 **vocSensor_CCS811;

typedef enum
{
  DEVICE_DISTANCE_VL53L1X = 0,
  DEVICE_LOADCELL_NAU7802,
  DEVICE_MULTIPLEXER,
  DEVICE_VOC_CCS811,
  DEVICE_PHT_BME280,
  DEVICE_TOTAL_DEVICES, //Marks the end, used to iterate loops
  DEVICE_UNKNOWN_DEVICE,
} deviceType_e;

uint8_t deviceCounts[DEVICE_TOTAL_DEVICES];

struct node
{
  deviceType_e deviceType;
  uint8_t deviceNumber = 0; //The position within the class pointer array. Related to deviceCounts

  uint8_t address = 0;
  uint8_t portNumber = 0;
  uint8_t muxAddress = 0;
  bool online = false; //Goes true once successfully begin()'d

  void *loggingConfigStruct; //The struct containing this devices logging options
  node *next;
};

node *head = NULL;
node *tail = NULL;

const byte PIN_QWIIC_POWER = 18;

struct struct_CCS811 {
  bool log = true;
  bool logTVOC = true;
  bool logCO2 = true;
  int tempValue = 0;
};

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("I2C Multi Detection");

  qwiicPowerOn();
  qwiic.begin();

  qwiicAutoDetect();
}

void loop()
{
  //Begin reading devices
//  node *temp = new node;
//  temp = head;
//
//  while (temp != NULL)
//  {
//    if (temp->address == address)
//      if (temp->muxAddress == muxAddress)
//        if (temp->portNumber == portNumber) return (true);
//
//
//    temp = temp->next;
//  }

}

//Scan I2C bus including sub-branches of multiplexers
//Creates a linked list of devices
//Creates appropriate classes for each device 
//Begin()s each device in list
//Returns true if devices detected > 0
bool qwiicAutoDetect()
{
  bool somethingDetected = false;

  //First scan for Muxes
  Serial.println("Scanning for Muxes");
  for (uint8_t address = 0x70 ; address < 0x78 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      somethingDetected = true;

      deviceType_e foundType = testDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType == DEVICE_MULTIPLEXER)
      {
        recordDevice(address, foundType); //Add this device to our map
        deviceCounts[foundType]++; //Increase the count of this type of device
      }
    }
  }

  //If a mux is detected, begin() to turn off all its ports
  if (deviceCounts[DEVICE_MULTIPLEXER] > 0)
  {
    Serial.printf("%d multiplexor(s) found\n", deviceCounts[DEVICE_MULTIPLEXER]);
    createClassesForDetectedDevices();
    beginDetectedDevices(); //Begin all muxes to disable their output ports
  }

  //Before going into sub branches, complete the scan of the main branch for all devices
  Serial.println("Scanning main bus");
  for (uint8_t address = 1 ; address < 127 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      somethingDetected = true;
      deviceType_e foundType = testDevice(address, 0, 0); //No mux or port numbers for this test
      if (foundType != DEVICE_UNKNOWN_DEVICE)
      {
        if (recordDevice(address, foundType) == true) //Records this device. Returns true if new.
          deviceCounts[foundType]++; //If this is a newly discovered device, increase the count of this type of device
        Serial.printf("-%s found at address 0x%02X\n", getDeviceName(foundType), address);
      }
      else
        Serial.printf("-%s found at address 0x%02X\n", getDeviceName(foundType), address);
    }
  }

  //If we have muxes, begin scanning their sub nets
  if (deviceCounts[DEVICE_MULTIPLEXER] > 0)
  {
    Serial.println("Scanning sub nets");

    //Step into first mux and begin stepping through ports
    for (int muxNumber = 0 ; muxNumber < deviceCounts[DEVICE_MULTIPLEXER] ; muxNumber++)
    {
      uint8_t muxAddress = getDeviceAddress(DEVICE_MULTIPLEXER, muxNumber);

      for (int portNumber = 0 ; portNumber < 7 ; portNumber++)
      {
        multiplexer[muxNumber]->setPort(portNumber);

        //Scan this new bus for new addresses
        for (uint8_t address = 1 ; address < 127 ; address++)
        {
          qwiic.beginTransmission(address);
          if (qwiic.endTransmission() == 0)
          {
            somethingDetected = true;

            //Ignore devices we've already logged. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
            if (deviceExists(address, muxAddress, portNumber) == false)
            {
              deviceType_e foundType = testDevice(address, muxAddress, portNumber);
              if (foundType != DEVICE_UNKNOWN_DEVICE)
              {
                if (recordDevice(address, foundType, muxAddress, portNumber) == true) //Record this device, with mux port specifics. Returns true if new.
                  deviceCounts[foundType]++; //If this is a newly discovered device, increase the count of this type of device
                Serial.printf("-New %s found at address 0x%02X.0x%02X.0x%02X\n", getDeviceName(foundType), address, muxAddress, portNumber);
              }
              else
                Serial.printf("-%s found at address 0x%02X\n", getDeviceName(foundType), address);
            } //End device exists check
          } //End I2c check
        } //End I2C scanning
      } //End mux port stepping
    } //End mux stepping
  } //End mux > 0

  if (somethingDetected)
  {
    createClassesForDetectedDevices();
    beginDetectedDevices(); //Step through the linked list and begin() everything in our map
    printDetectedDevices();
  }

  return (somethingDetected);
}

// Available Qwiic devices
#define ADR_VL53L1X 0x29
#define ADR_CCS811_2 0x5A
#define ADR_CCS811_1 0x5B
#define ADR_MUX_1 0x70 //to 0x77
#define ADR_MUX_2 0x71
#define ADR_BME280_2 0x76
#define ADR_BME280_1 0x77

//Given an address, returns the device type if it responds as we would expect
deviceType_e testDevice(uint8_t i2cAddress, uint8_t muxAddress, uint8_t portNumber)
{
  switch (i2cAddress)
  {
    case 0x29:
      {
        SFEVL53L1X distanceSensor_VL53L1X(qwiic); //Start with given wire port
        if (distanceSensor_VL53L1X.begin() == 0) //Returns 0 if init was successful. Wire port passed in constructor.
          return (DEVICE_DISTANCE_VL53L1X);
        break;
      }
    case 0x5A:
    case 0x5B:
      {
        CCS811 vocSensor_CCS811(i2cAddress); //Start with given I2C address
        if (vocSensor_CCS811.begin(qwiic) == true) //Wire port
          return (DEVICE_VOC_CCS811);
        break;
      }
    case 0x70:
    case 0x71:
    case 0x72:
    case 0x73:
    case 0x74:
    case 0x75:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(i2cAddress, muxAddress, portNumber) == true) return (getDeviceType(i2cAddress));
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
          return (DEVICE_MULTIPLEXER);
        break;
      }
    case 0x76:
    case 0x77:
      {
        //Ignore devices we've already recorded. This was causing the mux to get tested, a begin() would happen, and the mux would be reset.
        if (deviceExists(i2cAddress, muxAddress, portNumber) == true) return (getDeviceType(i2cAddress));

        //Try a mux first. This will write/read to 0x00 register.
        QWIICMUX multiplexer;
        if (multiplexer.begin(i2cAddress, qwiic) == true) //Address, Wire port
        {
          Serial.println("Mux found?");
          return (DEVICE_MULTIPLEXER);
        }

        BME280 phtSensor_BME280;
        phtSensor_BME280.setI2CAddress(i2cAddress);
        if (phtSensor_BME280.beginI2C(qwiic) == true) //Wire port
          return (DEVICE_PHT_BME280);
        break;
      }
    default:
      {
        Serial.printf("-Unknown device at address 0x%02X\n", i2cAddress);
        return DEVICE_UNKNOWN_DEVICE;
        break;
      }
  }
  Serial.printf("-Known I2C address but device responded unexpectedly at address 0x%02X\n", i2cAddress);
  return DEVICE_UNKNOWN_DEVICE;
}


void createClassesForDetectedDevices()
{
  //Create classes for the devices we found
  for (int dType = 0 ; dType < DEVICE_TOTAL_DEVICES ; dType++)
  {
    int devicesInThisClass = deviceCounts[dType];
    //Serial.printf("Devices in this class: %d\n", devicesInThisClass);

    if (devicesInThisClass > 0)
    {
      switch (dType)
      {
        case DEVICE_MULTIPLEXER:
          multiplexer = new QWIICMUX *[devicesInThisClass]; //Create set of pointers to this class
          for (int x = 0 ; x < devicesInThisClass ; x++) //Assign pointers to instances of this class
            multiplexer[x] = new QWIICMUX;
          break;
        case DEVICE_DISTANCE_VL53L1X:
          distanceSensor_vl53l1x = new SFEVL53L1X *[devicesInThisClass];
          for (int x = 0 ; x < devicesInThisClass ; x++)
            distanceSensor_vl53l1x[x] = new SFEVL53L1X(qwiic); //Construct with arguments
          break;
        case DEVICE_PHT_BME280:
          phtSensor_BME280 = new BME280 *[devicesInThisClass];
          for (int x = 0 ; x < devicesInThisClass ; x++)
            phtSensor_BME280[x] = new BME280;
          break;
        case DEVICE_VOC_CCS811:
          vocSensor_CCS811 = new CCS811 *[devicesInThisClass];
          for (int x = 0 ; x < devicesInThisClass ; x++)
          {
            //Lookup the address of this device
            uint8_t tempAddress = getDeviceAddress(DEVICE_VOC_CCS811, x);
            vocSensor_CCS811[x] = new CCS811(tempAddress); //Construct with the I2C address of this device
          }
          break;
        default:
          Serial.println("CreateClass Error: Unknown Type");
          break;
      }
    }//devicesInThisClass > 0
  }//Loop through dType array
}

//Once the linked list has been created, step through it and
//attempt to begin each device
//If muxes were previously detected, activate the proper mux port before beginning device
void beginDetectedDevices()
{
  Serial.println("Beginning devices:");
  uint8_t startedDevicesInClass[DEVICE_TOTAL_DEVICES] = {0};
  uint8_t deviceNumber = 0;

  node *temp = new node;
  temp = head;
  while (temp != NULL)
  {
    //As each instance is started, increment the device number. If begin succeeds, the
    //device number is stored in the node.
    deviceNumber = startedDevicesInClass[temp->deviceType];
    bool beginSuccess = false;

    openConnection(temp->address); //If muxes are detected, open a connection to this address

    switch (temp->deviceType)
    {
      case DEVICE_MULTIPLEXER:
        beginSuccess = multiplexer[deviceNumber]->begin(temp->address, qwiic);
        break;

      case DEVICE_DISTANCE_VL53L1X:
        Serial.printf("Distance-VL53L1X\n");
        break;

      case DEVICE_VOC_CCS811:
        beginSuccess = vocSensor_CCS811[deviceNumber]->begin(qwiic); //Wire port
        break;

      case DEVICE_PHT_BME280:
        phtSensor_BME280[deviceNumber]->setI2CAddress(temp->address);
        beginSuccess = phtSensor_BME280[deviceNumber]->beginI2C(qwiic); //Wire port
        break;

      default:
        Serial.printf("Unknown device type: %d\n", temp->deviceType);
        break;
    }

    if (beginSuccess == true)
    {
      startedDevicesInClass[temp->deviceType]++;
      temp->online = true;
      temp->deviceNumber = deviceNumber;

      //https://stackoverflow.com/questions/14768230/malloc-for-struct-and-pointer-in-c
      
      struct_CCS811 sensor_CCS811; //Create the struct
      sensor_CCS811.log = true;
      sensor_CCS811.tempValue = 56;
      temp->loggingConfigStruct = &sensor_CCS811; //Make the node point at this struct

      struct_CCS811 *local_config = (struct_CCS811 *)temp->loggingConfigStruct; //Create a local pointer, to give definition. Point pointer at the previous struct.
      local_config->log = true; //Change struct
      Serial.printf("temp value: %d\n", local_config->tempValue);
      while(1);
      
      Serial.printf("-%s %d online!\n", getDeviceName(temp->deviceType), deviceNumber);
    }
    else
    {
      temp->online = false;
      temp->deviceNumber = 255;
      Serial.printf("-%s %d failed to init\n", getDeviceName(temp->deviceType), deviceNumber);
    }

    temp = temp->next;
  }
}

//Given a device number return the string associated with that entry
const char* getDeviceName(deviceType_e deviceNumber)
{
  switch (deviceNumber)
  {
    case DEVICE_DISTANCE_VL53L1X:
      return "Distance-VL53L1X";
      break;
    case DEVICE_LOADCELL_NAU7802:
      return "LoadCell-NAU7802";
      break;
    case DEVICE_MULTIPLEXER:
      return "Multiplexer";
      break;
    case DEVICE_VOC_CCS811:
      return "VOC-CCS811";
      break;
    case DEVICE_PHT_BME280:
      return "PHT-BME280";
      break;
    case DEVICE_UNKNOWN_DEVICE:
      return "Unknown device";
      break;
    default:
      return "Unknown Status";
      break;
  }
  return "None";
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
