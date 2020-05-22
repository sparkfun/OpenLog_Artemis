/*
  Same as I2C_Detector_Mux just expanded to include all known sensors on OLA.

  To add a new device:
  Add a device definition to the deviceType enum in main sketch
  Add printable string to getDeviceName()
*/

#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9

#include <SparkFun_I2C_Mux_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_I2C_Mux
QWIICMUX **multiplexer;

#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_NAU7802
NAU7802 **loadcellSensor_NAU7802;

#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X
SFEVL53L1X **distanceSensor_VL53L1X;

#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS **gpsSensor_ublox;

#include "SparkFun_VCNL4040_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_VCNL4040
VCNL4040 **proximitySensor_VCNL4040;

#include "SparkFun_MCP9600.h" //Click here to get the library: http://librarymanager/All#SparkFun_MCP9600
MCP9600 **thermoSensor_MCP9600;

#include "SparkFun_TMP117.h" //Click here to get the library: http://librarymanager/All#SparkFun_TMP117
TMP117 **tempSensor_TMP117;

#include "SparkFun_MS5637_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_MS5637
MS5637 **pressureSensor_MS5637;

#include "SparkFun_LPS25HB_Arduino_Library.h"  //Click here to get the library: http://librarymanager/All#SparkFun_LPS25HB
LPS25HB **pressureSensor_LPS25HB;

#include "SparkFunBME280.h" //Click here to get the library: http://librarymanager/All#SparkFun_BME280
BME280 **phtSensor_BME280;

#include "SparkFun_VEML6075_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_VEML6075
VEML6075 **uvSensor_VEML6075;

#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811
CCS811 **vocSensor_CCS811;

#include "SparkFun_SGP30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SGP30
SGP30 **vocSensor_SGP30;

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 **co2Sensor_SCD30;

#include "MS8607_Library.h" //Click here to get the library: http://librarymanager/All#Qwiic_MS8607
MS8607 **pressureSensor_MS8607;

typedef enum
{
  DEVICE_MULTIPLEXER = 0,
  DEVICE_LOADCELL_NAU7802,
  DEVICE_DISTANCE_VL53L1X,
  DEVICE_GPS_UBLOX,
  DEVICE_PROXIMITY_VCNL4040,
  DEVICE_TEMPERATURE_TMP117,
  DEVICE_PRESSURE_MS5637,
  DEVICE_PRESSURE_LPS25HB,
  DEVICE_PHT_BME280,
  DEVICE_UV_VEML6075,
  DEVICE_VOC_CCS811,
  DEVICE_VOC_SGP30,
  DEVICE_CO2_SCD30,
  DEVICE_PRESSURE_MS8607,

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
  node *next;
};

node *head = NULL;
node *tail = NULL;

const byte PIN_QWIIC_POWER = 18;

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println("I2C Multi Detection");

  qwiicPowerOn();
  qwiic.begin();

  Serial.println("Scanning");
  Serial.flush();
}

void loop()
{
  bool somethingDetected = false;

  //First scan for Muxes
  Serial.println("Scanning for Muxes");
  Serial.flush();
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
    Serial.flush();
    createClassesForDetectedDevices();
    beginDetectedDevices(); //Begin all muxes to disable their output ports
  }

  //Before going into sub branches, complete the scan of the main branch for all devices
  Serial.println("Scanning main bus");
  Serial.flush();
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
                Serial.flush();
              }
              else
              {
                Serial.printf("-%s found at address 0x%02X\n", getDeviceName(foundType), address);
                Serial.flush();
              }
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


  //  if (qwiicAvailable.QWIICMUX == true)
  //  {
  //    Serial.println("Now do sub-scans on mux");
  //    while (1);
  //  }

  while (1);
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
//          distanceSensor_vl53l1x = new SFEVL53L1X *[devicesInThisClass];
//          for (int x = 0 ; x < devicesInThisClass ; x++)
//            distanceSensor_vl53l1x[x] = new SFEVL53L1X(qwiic); //Construct with arguments
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
    case DEVICE_MULTIPLEXER:
      return "Multiplexer";
      break;
    case DEVICE_LOADCELL_NAU7802:
      return "LoadCell-NAU7802";
      break;
    case DEVICE_DISTANCE_VL53L1X:
      return "Distance-VL53L1X";
      break;
    case DEVICE_GPS_UBLOX:
      return "GPS-Ublox";
      break;
    case DEVICE_PROXIMITY_VCNL4040:
      return "Proximity-VCNL4040";
      break;
    case DEVICE_TEMPERATURE_TMP117:
      return "Temperature-TMP117";
      break;
    case DEVICE_PRESSURE_MS5637:
      return "Pressure-MS5637";
      break;
    case DEVICE_PRESSURE_LPS25HB:
      return "Pressure-LPS25HB";
      break;
    case DEVICE_PHT_BME280:
      return "PHT-BME280";
      break;
    case DEVICE_UV_VEML6075:
      return "UV-VEML6075";
      break;
    case DEVICE_VOC_CCS811:
      return "VOC-CCS811";
      break;
    case DEVICE_VOC_SGP30:
      return "VOC-SGP30";
      break;
    case DEVICE_CO2_SCD30:
      return "CO2-SCD30";
      break;
    case DEVICE_PRESSURE_MS8607:
      return "Pressure-MS8607";
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
