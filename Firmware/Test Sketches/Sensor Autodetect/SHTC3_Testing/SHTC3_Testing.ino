/*
  Take humidity and temperature readings with the SHTC3 using I2C
  By: Owen Lyke
  SparkFun Electronics
  Date: August 24 2018
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
  Example1_BasicReadings
  To connect the sensor to an Arduino:
  This library supports the sensor using the I2C protocol
  On Qwiic enabled boards simply connnect the sensor with a Qwiic cable and it is set to go
  On non-qwiic boards you will need to connect 4 wires between the sensor and the host board
  (Arduino pin) = (Display pin)
  SCL = SCL on display carrier
  SDA = SDA
  GND = GND
  3.3V = 3.3V
*/

#include "Wire.h"
TwoWire qwiic(1); //Will use pads 8/9

#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3
SHTC3 mySHTC3;              // Declare an instance of the SHTC3 class

const byte PIN_QWIIC_POWER = 18;

void setup() {
  Serial.begin(115200);

  qwiic.begin();
  qwiicPowerOn();

  Serial.println("SHTC3 Example");

  if (mySHTC3.begin(qwiic) != 0)
  {
    Serial.println("The SHTC3 did not respond. Check wiring. Freezing...");
    while (1);
  }
}

void loop() {

  mySHTC3.update();

  Serial.print(mySHTC3.toPercent());                   // "toPercent" returns the percent humidity as a floating point number
  Serial.print("% ");
  Serial.print(mySHTC3.toDegC());                        // "toDegF" and "toDegC" return the temperature as a flaoting point number in deg F and deg C respectively
  Serial.println("C");

  delay(10);
}

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  digitalWrite(PIN_QWIIC_POWER, HIGH);
}
void qwiicPowerOff()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  digitalWrite(PIN_QWIIC_POWER, LOW);
}
