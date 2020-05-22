/*
  Use the Qwiic Mux to access multiple I2C devices on seperate busses.
  By: Nathan Seidle @ SparkFun Electronics
  Date: May 17th, 2020
  License: This code is public domain but you buy me a beer if you use this
  and we meet someday (Beerware license).

  Some I2C devices respond to only one I2C address. This can be a problem
  when you want to hook multiple of a device to the I2C bus. An I2C Mux
  solves this issue by allowing you to change the 'channel' or port that
  the master is talking to.

  This example shows how to hook up two VL53L1X laser distance sensors with the same address.
  You can read the VL53L1X hookup guide and get the library from https://learn.sparkfun.com/tutorials/qwiic-distance-sensor-vl53l1x-hookup-guide

  The TCA9548A is a mux. This means when you enableMuxPort(2) then the SDA and SCL lines of the master (Arduino)
  are connected to port 2. Whatever I2C traffic you do, such as distanceSensor.startRanging() will be communicated to whatever
  sensor you have on port 2. This example creates an array of objects. This increases RAM but allows for
  independent configuration of each sensor (one sensor may be configured for long range, the others for short, etc).

  Outputs two sets of distances in mm and ft.

  Hardware Connections:
  Attach the Qwiic Mux Shield to your RedBoard or Uno.
  Plug two Qwiic VL53L1X breakout boards into ports 0 and 1.
  Serial.print it out at 115200 baud to serial monitor.

  SparkFun labored with love to create this code. Feel like supporting open
  source? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14685
*/

#include <Wire.h>

//TwoWire Wire1(1); //Will use pads 8/9

#include <SparkFun_I2C_Mux_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_I2C_Mux
QWIICMUX myMux;

#define NUMBER_OF_SENSORS 2

#include "SparkFun_VL53L1X.h" //Click here to get the library: http://librarymanager/All#SparkFun_VL53L1X

SFEVL53L1X **distanceSensor; //Create pointer to a set of pointers to the sensor class

void setup()
{
  Serial.begin(115200);
  Serial.println("Qwiic Mux Shield Read Example");

  //Wire1.begin();
  Wire.begin();

  //Create set of pointers to the class
  distanceSensor = new SFEVL53L1X *[NUMBER_OF_SENSORS];

  //Assign pointers to instances of the class
  for (int x = 0 ; x < NUMBER_OF_SENSORS ; x++)
    distanceSensor[x] = new SFEVL53L1X(Wire);

  //Setup mux to use Wire1. If you have multiple muxes, pass address as first argument
  //if (myMux.begin(0x70, Wire1) == false)
  if (myMux.begin() == false)
  {
    Serial.println("Mux not detected. Freezing...");
    while (1)
      ;
  }
  Serial.println("Mux detected");

  byte currentPortNumber = myMux.getPort();
  Serial.print("CurrentPort: ");
  Serial.println(currentPortNumber);

  //Initialize all the sensors
  bool initSuccess = true;

  for (byte x = 0; x < NUMBER_OF_SENSORS; x++)
  {
    myMux.setPort(x);
    if (distanceSensor[x]->begin() != 0) //Begin returns 0 on a good init
    {
      Serial.print("Sensor ");
      Serial.print(x);
      Serial.println(" did not begin! Check wiring");
      initSuccess = false;
    }
    else
    {
      //Configure each sensor
      distanceSensor[x]->setIntermeasurementPeriod(180);
      distanceSensor[x]->setDistanceModeLong();
      distanceSensor[x]->startRanging(); //Write configuration bytes to initiate measurement
      Serial.print("Sensor ");
      Serial.print(x);
      Serial.println(" configured");
    }
  }

  if (initSuccess == false)
  {
    Serial.print("Freezing...");
    while (1);
  }

  Serial.println("Mux Shield online");
}

void loop()
{
  int distance[NUMBER_OF_SENSORS];
  float distanceFeet;

  for (byte x = 0; x < NUMBER_OF_SENSORS; x++)
  {
    myMux.setPort(x); //Tell mux to connect to this port, and this port only
    distance[x] = distanceSensor[x]->getDistance(); //Get the result of the measurement from the sensor

    Serial.print("\tDistance");
    Serial.print(x);
    Serial.print("(mm): ");
    Serial.print(distance[x]);

    distanceFeet = (distance[x] * 0.0393701) / 12.0;

    Serial.print("\tDistance");
    Serial.print(x);
    Serial.print("(ft): ");
    Serial.print(distanceFeet, 2);
  }

  Serial.println();

  delay(180); //Wait for next reading
}
