/*

*/
#include <Wire.h>
TwoWire qwiic(1);

#include <SparkFun_Qwiic_Humidity_AHT20.h> //Click here to get the library: http://librarymanager/All#Qwiic_Humidity_AHT20 by SparkFun
AHT20 humiditySensor;

void setup()
{
  Serial.begin(115200);
  Serial.println("Qwiic Humidity AHT20 examples");

  qwiic.begin(); //Join I2C bus

  //Check if the AHT20 will acknowledge
  if (humiditySensor.begin(qwiic) == false)
  {
    Serial.println("AHT20 not detected. Please check wiring. Freezing.");
    while (1);
  }
  Serial.println("AHT20 acknowledged.");
}

void loop()
{
  //If a new measurement is available
  if (humiditySensor.available() == true)
  {
    //Get the new temperature and humidity value
    float temperature = humiditySensor.getTemperature();
    float humidity = humiditySensor.getHumidity();

    //Print the results
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.print(" C\t");
    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.print("% RH");

    Serial.println();
  }

  //The AHT20 can respond with a reading every ~50ms. However, increased read time can cause the IC to heat around 1.0C above ambient.
  //The datasheet recommends reading every 2 seconds.
  delay(2000);
}
