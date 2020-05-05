#include <Wire.h>
TwoWire qwiic(1); //Will use pads 8/9

void setup()
{
  Serial.begin(115200);
  Serial.println("Scanning...");

  const byte PIN_QWIIC_PWR = 18;
  pinMode(PIN_QWIIC_PWR, OUTPUT);
  digitalWrite(PIN_QWIIC_PWR, LOW); //qwiicPowerOn();

  qwiic.begin();
  qwiic.setClock(100000);

  qwiic.setPullups(1); //Set pullups to 1k. If we don't have pullups, detectQwiicDevices() takes ~900ms to complete. We'll disable pullups if something is detected.

  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    qwiic.beginTransmission(address);
    error = qwiic.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println();

      nDevices++;
    }
//    else if (error == 4)
//    {
//      Serial.print("Unknown error at address 0x");
//      if (address < 16)
//        Serial.print("0");
//      Serial.println(address, HEX);
//    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void loop()
{

}
