/*
  Log CO2 and tVOC
  Log H2 and ethanol as well

  I2C fails when pullups are disabled? Might be a bad board.

  This board really can't be power cycled. It need to stay on for ~15s before readings are available

  But the sensor can be queried constantly for new data (if fully powered).

  The SGP30 seems to not like to be on the bus by itself using the main OLA firmware.
*/

#include "SparkFun_SGP30_Arduino_Library.h" // Click here to get the library: http://librarymanager/All#SparkFun_SGP30
#include <Wire.h>

SGP30 mySensor; //create an object of the SGP30 class
TwoWire qwiic(1); //Will use pads 8/9

const byte PIN_QWIIC_POWER = 18;

void setup() {
  Serial.begin(115200);

  qwiicPowerOn();
  delay(200);

  qwiic.begin();
  //qwiic.setPullups(0); //Disable pull up resistors to avoid back feeding power to peripherals from SDA/SCL lines
  qwiic.setClock(400000);

  //Initialize sensor
  if (mySensor.begin(qwiic) == false) {
    Serial.println("No SGP30 Detected. Check connections.");
    while (1);
  }

  //Initializes sensor for air quality readings
  //measureAirQuality should be called in one second increments after a call to initAirQuality
  mySensor.initAirQuality();
}

int count = 0 ;

int delayBeforeBegin = 50; //Min of
int delayBeforeMeasurement = 100; //Min of

void loop()
{
  if (Serial.available())
  {
    byte incoming = Serial.read();
    if (incoming == 'a') delayBeforeMeasurement++;
    else if (incoming == 'z') delayBeforeMeasurement--;
    else if (incoming == '+') delayBeforeMeasurement += 10;
    else if (incoming == '-') delayBeforeMeasurement -= 10;

    else if (incoming == 's') delayBeforeBegin++;
    else if (incoming == 'x') delayBeforeBegin--;
    else if (incoming == '1') delayBeforeBegin += 10;
    else if (incoming == '2') delayBeforeBegin -= 10;

    Serial.printf("delayBeforeBegin: %d ", delayBeforeBegin);
    Serial.printf("delayBeforeMeasurement: %d\n", delayBeforeMeasurement);
  }

  //qwiicPowerOff();
  //qwiic.setPullups(0); //Disable pull up resistors to avoid back feeding power to peripherals from SDA/SCL lines
  //delay(200); //Make sure peripherals have powered down

  //qwiicPowerOn();
  //qwiic.setPullups(1);
  //delay(delayBeforeBegin);

  //Initialize sensor
  //  if (mySensor.begin(qwiic) == false) {
  //    Serial.println("No SGP30 Detected. Check connections.");
  //    while (1);
  //  }
  //
  //  //Initializes sensor for air quality readings
  //  //measureAirQuality should be called in one second increments after a call to initAirQuality
  //  mySensor.initAirQuality();

  //First fifteen readings will be
  //CO2: 400 ppm  TVOC: 0 ppb

  //measure CO2 and TVOC levels
  mySensor.measureAirQuality();
  mySensor.measureRawSignals();

  Serial.print(count++);
  Serial.print(" CO2: ");
  Serial.print(mySensor.CO2);
  Serial.print(" ppm\tTVOC: ");
  Serial.print(mySensor.TVOC);
  Serial.print(" ppb");


  Serial.print("\tRaw H2: ");
  Serial.print(mySensor.H2);
  Serial.print(" \tRaw Ethanol: ");
  Serial.println(mySensor.ethanol);
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
