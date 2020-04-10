/*
  Reading humidity from the MS8607
  By: PaulZC
  Date: January 28th, 2020

  Based extensively on:
  Reading barometric pressure from the MS5637
  By: Nathan Seidle
  SparkFun Electronics
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  The original library and example code was written by TEConnectivity,
  the company that made the sensor. Way to go TE! May other companies
  learn from you.

  Feel like supporting open source hardware?
  Buy a board from SparkFun!

  This example reads and displays the humidity and temperature from the MS8607.
  It also displays the temperature-compensated humidity and the dew point.
*/

#include <Wire.h>

#include <MS8607_Library.h>
MS8607 barometricSensor;

TwoWire qwiic(1); //Will use pads 8/9

void setup(void) {
  Serial.begin(115200);
  Serial.println("Qwiic PHT Sensor MS8607 Example - Humidity");

  const byte PIN_QWIIC_PWR = 18;
  pinMode(PIN_QWIIC_PWR, OUTPUT);
  digitalWrite(PIN_QWIIC_PWR, LOW); //qwiicPowerOn();
  delay(10);
  qwiic.begin();

  if (barometricSensor.begin(qwiic) == false)
  {
    Serial.println("MS8607 sensor did not respond. Trying again...");
    if (barometricSensor.begin() == false)
    {
      Serial.println("MS8607 sensor did not respond. Please check wiring.");
      while(1)
        ;
    }
  }

  MS8607_pressure_resolution pressureResolution = MS8607_pressure_resolution_osr_8192; //17ms per reading, 0.016mbar resolution

  //The sensor has 6 resolution levels. The higher the resolution the longer each
  //reading takes to complete.
//  barometricSensor.set_pressure_resolution(MS8607_pressure_resolution_osr_256); //1ms per reading, 0.11mbar resolution
//  barometricSensor.set_pressure_resolution(MS8607_pressure_resolution_osr_512); //2ms per reading, 0.062mbar resolution
//  barometricSensor.set_pressure_resolution(MS8607_pressure_resolution_osr_1024); //3ms per reading, 0.039mbar resolution
//  barometricSensor.set_pressure_resolution(MS8607_pressure_resolution_osr_2048); //5ms per reading, 0.028mbar resolution
//  barometricSensor.set_pressure_resolution(MS8607_pressure_resolution_osr_4096); //9ms per reading, 0.021mbar resolution
  barometricSensor.set_pressure_resolution(pressureResolution); //17ms per reading, 0.016mbar resolution

  // Example: set the humidity resolution
  //int err = barometricSensor.set_humidity_resolution(MS8607_humidity_resolution_8b); // 8 bits
  //int err = barometricSensor.set_humidity_resolution(MS8607_humidity_resolution_10b); // 10 bits
  //int err = barometricSensor.set_humidity_resolution(MS8607_humidity_resolution_11b); // 11 bits
  int err = barometricSensor.set_humidity_resolution(MS8607_humidity_resolution_12b); //12 bits
  if (err != MS8607_status_ok)
  {
    Serial.print("Problem setting the MS8607 sensor humidity resolution. Error code = ");
    Serial.println(err);
    Serial.println("Freezing.");
    while(1);
  }
  
  // Turn the humidity sensor heater OFF
  // The TE examples say that get_compensated_humidity and get_dew_point will only work if the heater is OFF
  err = barometricSensor.disable_heater();
  if (err != MS8607_status_ok)
  {
    Serial.print("Problem disabling the MS8607 humidity sensor heater. Error code = ");
    Serial.println(err);
    Serial.println("Freezing.");
    while(1);
  }
}

void loop(void) {

  float humidity = barometricSensor.getHumidity();
  float temperature = barometricSensor.getTemperature();
  float pressure = barometricSensor.getPressure();

  Serial.print("Humidity=");
  Serial.print(humidity, 1);
  Serial.print("(%RH)");

  Serial.print(" Pressure=");
  Serial.print(pressure, 3);
  Serial.print("(hPa or mbar)");

  Serial.print(" Temperature=");
  Serial.print(temperature, 1);
  Serial.print("(C)");

//  float compensated_RH;
//  int err = barometricSensor.get_compensated_humidity(temperature, humidity, &compensated_RH);
//  if (err != MS8607_status_ok)
//  {
//    Serial.println();
//    Serial.print("Problem getting the MS8607 compensated humidity. Error code = ");
//    Serial.println(err);
//    return;
//  }
  
//  Serial.print(" Compensated humidity=");
//  Serial.print(compensated_RH, 1);
//  Serial.print("(%RH)");
//  
//  float dew_point;
//  err = barometricSensor.get_dew_point(temperature, humidity, &dew_point);
//  if (err != MS8607_status_ok)
//  {
//    Serial.println();
//    Serial.print("Problem getting the MS8607 dew point. Error code = ");
//    Serial.println(err);
//    return;
//  }
//  
//  Serial.print(" Dew point=");
//  Serial.print(dew_point, 1);
//  Serial.print("(C)");

  Serial.println();

  delay(10);
}
