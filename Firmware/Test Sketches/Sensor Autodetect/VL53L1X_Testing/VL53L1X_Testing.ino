/*
  Reading distance from the laser based VL53L1X
  By: Nathan Seidle
  SparkFun Electronics
  Date: April 4th, 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  SparkFun labored with love to create this code. Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/14667

  This example prints the distance to an object.

  Are you getting weird readings? Be sure the vacuum tape has been removed from the sensor.
*/

#include <Wire.h>
#include "SparkFun_VL53L1X.h"

//Optional interrupt and shutdown pins.
#define SHUTDOWN_PIN 2
#define INTERRUPT_PIN 3

TwoWire qwiic(1); //Will use pads 8/9
SFEVL53L1X distanceSensor(qwiic);
//Uncomment the following line to use the optional shutdown and interrupt pins.
//SFEVL53L1X distanceSensor(Wire, SHUTDOWN_PIN, INTERRUPT_PIN);

void setup(void)
{
  qwiic.begin();

  Serial.begin(115200);
  Serial.println("VL53L1X Qwiic Test");

  if (distanceSensor.begin() == 0) //Begin returns 0 on a good init
    Serial.println("Sensor online!");

  distanceSensor.setIntermeasurementPeriod(180);
  Serial.println(distanceSensor.getIntermeasurementPeriod());
  distanceSensor.setDistanceModeLong();
    distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
}

void loop(void)
{
  long startTime = millis();
  long stopTime;
  int oldDistance = distanceSensor.getDistance();
  int distance;
  while (1)
  {
//    distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
    distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
//    distanceSensor.stopRanging();
    //if (distance != oldDistance) break;
    break;

  }
  stopTime = millis();

  Serial.print("delta: ");
  Serial.print(stopTime - startTime);

  Serial.print("\tDistance(mm): ");
  Serial.print(distance);

  float distanceInches = distance * 0.0393701;
  float distanceFeet = distanceInches / 12.0;

  Serial.print("\tDistance(ft): ");
  Serial.print(distanceFeet, 2);

  Serial.print("\tperiod: ");
  Serial.print(distanceSensor.getIntermeasurementPeriod());

  Serial.println();


}
