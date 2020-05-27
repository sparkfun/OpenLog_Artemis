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

  qwiicPowerOn();
  delay(200);

  qwiic.begin();
  qwiic.setPullups(0); //Disable pull up resistors to avoid back feeding power to peripherals from SDA/SCL lines
  qwiic.setClock(400000);
}

int delayBeforeBegin = 5; //Min of 3
int delayBeforeMeasurement = 40; //Min of 38
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

  qwiicPowerOff();
  qwiic.setPullups(0); //Disable pull up resistors to avoid back feeding power to peripherals from SDA/SCL lines
  delay(200); //Make sure peripherals have powered down

  qwiicPowerOn();
  delay(delayBeforeBegin);

  pressureSensor.begin(qwiic);

  if (pressureSensor.isConnected() == false)
    Serial.println("LPS25HB disconnected!");

  delay(delayBeforeMeasurement);
  float pressure = pressureSensor.getPressure_hPa();
  float temperature = pressureSensor.getTemperature_degC();

  Serial.print("Pressure in hPa: ");
  Serial.print(pressure);
  Serial.print(", Temperature (degC): ");
  Serial.println(temperature);

  //delay(40); // Wait - 40 ms corresponds to the maximum update rate of the sensor (25 Hz)
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
