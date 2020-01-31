/******************************************************************************
  Read basic CO2 and TVOCs

  Marshall Taylor @ SparkFun Electronics
  Nathan Seidle @ SparkFun Electronics

  April 4, 2017

  https://github.com/sparkfun/CCS811_Air_Quality_Breakout
  https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library

  Read the TVOC and CO2 values from the SparkFun CSS811 breakout board

  A new sensor requires at 48-burn in. Once burned in a sensor requires
  20 minutes of run in before readings are considered good.

  Hardware Connections (Breakoutboard to Arduino):
  3.3V to 3.3V pin
  GND to GND pin
  SDA to A4
  SCL to A5

******************************************************************************/
#include <Wire.h>

#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811

#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

CCS811 mySensor(CCS811_ADDR);
TwoWire qwiic(1); //Will use pads 8/9

void setup()
{
  Serial.begin(115200);
  Serial.println("CCS811 Basic Example");

  const byte PIN_QWIIC_PWR = 18;
  pinMode(PIN_QWIIC_PWR, OUTPUT);
  digitalWrite(PIN_QWIIC_PWR, LOW); //qwiicPowerOn();
  delay(10);
  qwiic.begin();


  //This begins the CCS811 sensor and prints error status of .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = mySensor.beginWithStatus(qwiic);
  if (returnCode != CCS811Core::CCS811_Stat_SUCCESS)
  {
    Serial.print("CCS811 begin exited with: ");
    Serial.println(mySensor.statusString(returnCode));
    while(1);

  }
  Serial.println("CCS811 sensor online!");


  //  if (returnCode == false)
  //  {
  //    delay(100);
  //    if (mySensor.begin(qwiic) == false)
  //    {
  //      Serial.print("CCS811 error. Please check wiring. Freezing...");
  //      while (1)
  //        ;
  //    }
  //  }
}

void loop()
{
  //Check to see if data is ready with .dataAvailable()
  if (mySensor.dataAvailable())
  {
    //If so, have the sensor read and calculate the results.
    //Get them later
    mySensor.readAlgorithmResults();

    Serial.print("CO2[");
    //Returns calculated CO2 reading
    Serial.print(mySensor.getCO2());
    Serial.print("] tVOC[");
    //Returns calculated TVOC reading
    Serial.print(mySensor.getTVOC());
    Serial.print("] millis[");
    //Display the time since program start
    Serial.print(millis());
    Serial.print("]");
    Serial.println();
  }

  delay(10); //Don't spam the I2C bus
}
