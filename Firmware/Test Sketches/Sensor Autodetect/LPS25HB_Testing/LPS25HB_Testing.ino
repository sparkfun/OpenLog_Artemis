/*
  
*/

#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9

#include <SparkFun_LPS25HB_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_LPS25HB
LPS25HB pressureSensor; // Create an object of the LPS25HB class

const byte PIN_QWIIC_POWER = 18;

void setup()
{
  Serial.begin(115200);
  Serial.println("LPS25HB Pressure Sensor Example 1 - Basic Readings");
  Serial.println();

  qwiic.begin();
}

int testAmount = 20;
void loop()
{
  if(Serial.available())
  {
    byte incoming = Serial.read();
    if(incoming == 'a') testAmount++;
    else if(incoming == 'z') testAmount--;
    else if(incoming == '+') testAmount += 10;
    else if(incoming == '-') testAmount -= 10;
  }
  
  qwiicPowerOff();
  qwiic.setPullups(0); //0.14V and dropping, but causes OLA to power reset
  //qwiic.setPullups(1); //No reset, 1.96V (power LED dim)
  //qwiic.setPullups(10); //No reset, 1.27V off
  //qwiic.setPullups(27); //Max pull up resistors. 1.08V
  delay(4000);
  
  qwiicPowerOn();
  delay(1000);
  delay(testAmount);
  
  pressureSensor.begin(qwiic); // Begin links an I2C port and I2C address to the sensor, sets an I2C speed, begins I2C on the main board, and then sets default settings

  if (pressureSensor.isConnected() == false) // The library supports some different error codes such as "DISCONNECTED"
  {
    Serial.println("LPS25HB disconnected. Reset the board to try again.");     // Alert the user that the device cannot be reached
    Serial.println("Are you using the right Wire port and I2C address?");      // Suggest possible fixes
    Serial.println("See Example2_I2C_Configuration for how to change these."); // Suggest possible fixes
    Serial.println("");
    //while (1)
      //;
  }
  
  Serial.print("Pressure in hPa: ");
  Serial.print(pressureSensor.getPressure_hPa()); // Get the pressure reading in hPa
  Serial.print(", Temperature (degC): ");
  Serial.println(pressureSensor.getTemperature_degC()); // Get the temperature in degrees C

  delay(40); // Wait - 40 ms corresponds to the maximum update rate of the sensor (25 Hz)
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
