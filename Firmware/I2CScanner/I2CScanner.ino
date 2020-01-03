#include <Wire.h>
TwoWire myWire(1); //Will use pads 8/9

void setup()
{
  Serial.begin(115200);
  Serial.println("Scanning...");

  //Wire.begin();
  myWire.begin();
  myWire.setClock(100000);

  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    //Wire.beginTransmission(address);
    //error = Wire.endTransmission();
    myWire.beginTransmission(address);
    error = myWire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

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
