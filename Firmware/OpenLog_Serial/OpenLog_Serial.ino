/*
  October 11th, 2019

  Increase local serial, turn off except when syncing
  Try better grade card

  460800bps = 2.17 * 10^-6 or 2.17us per bit
  10 bits to the byte so 21.7us per byte
  Write takes 54.13ms or 2494 bytes

  16384 buffer size will allow for up to 355.532ms before bytes are dropped
*/

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //We do not use the built-in SD.h file because it calls Serial.print

const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_MICROSD_POWER = 15; //x04

SdFat sd;
SdFile serialDataFile; //Record all incoming serial to this file
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

long overallStartTime; //Used to calc the actual update rate.

const byte statLED = 19; //Should not be the SPI SCK pin 13 on most boards

//Create UART instance for OpenLog style serial logging
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
Uart SerialLog(1, 13, 12);  // Declares a Uart object called Serial1 using instance 1 of Apollo3 UART peripherals with RX on pin 13 and TX on pin 12 (note, you specify *pins* not Apollo3 pads. This uses the variant's pin map to determine the Apollo3 pad)
unsigned long lastSeriaLogSyncTime = 0;
const int MAX_IDLE_TIME_MSEC = 500;
bool newSerialData = false;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int charsReceived = 0;
//char incomingBuffer[256 * 8]; //94ms
//char incomingBuffer[256 * 4]; //47ms
char incomingBuffer[256 * 2]; //26ms
//char incomingBuffer[256 * 1]; //25ms
int incomingBufferSpot = 0;

void setup() {
  Serial.begin(500000);
  Serial.println("Artemis OpenLog");

  pinMode(11, OUTPUT);

  int rate = 115200 * 4;
  SerialLog.begin(rate);
  Serial.print("Listening at baud rate: ");
  Serial.println(rate);

  pinMode(statLED, OUTPUT);
  digitalWrite(statLED, LOW);

  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected

  microSDPowerOn();
  delay(50); //Give SD time to power up

  if (sd.begin(PIN_MICROSD_CHIP_SELECT, SD_SCK_MHZ(24)) == false)
  {
    Serial.println("SD init failed");
    while (1);
  }
  if (sd.chdir() == false)
  {
    Serial.println("SD chdr failed");
    while (1);
  }

  char fileName[9];
  sprintf(fileName, "serLog.txt");

  if (sd.exists(fileName))
  {
    Serial.println("serLog deleted");
    sd.remove(fileName);
  }

  if (!serialDataFile.open(fileName, O_CREAT | O_APPEND | O_WRITE))
  {
    Serial.println("Failed to open file");
    while (1);
  }
  Serial.println("serLog created");

  overallStartTime = millis();
}

void loop()
{
  //Check for incoming serial to record to serial log file
  if (SerialLog.available())
  {
    while (SerialLog.available())
    {
      incomingBuffer[incomingBufferSpot++] = SerialLog.read();
      if (incomingBufferSpot == sizeof(incomingBuffer))
      {
//        long startTime = micros();
        serialDataFile.write(incomingBuffer, sizeof(incomingBuffer)); //Record the buffer to the card
//        long endTime = micros();

        incomingBufferSpot = 0;

//        Serial.print("Write time (ms): ");
//        Serial.println((endTime - startTime) / 1000.0, 2);
      }

      charsReceived++;
    }
    lastSeriaLogSyncTime = millis();
    newSerialData = true;
  }
  else if (newSerialData == true)
  {
    if ((millis() - lastSeriaLogSyncTime) > MAX_IDLE_TIME_MSEC) //If we haven't received any characters recently then sync log file
    {
      if (incomingBufferSpot > 0)
      {
        //Write the remainder of the buffer
        serialDataFile.write(incomingBuffer, incomingBufferSpot); //Record the buffer to the card
        serialDataFile.sync();

        incomingBufferSpot = 0;
      }

      newSerialData = false;
      lastSeriaLogSyncTime = millis(); //Reset the last sync time to now

      Serial.println("Total chars recevied: " + (String)charsReceived);
    }
  }

  if (Serial.available())
  {
    byte incoming = Serial.read();

    if (incoming == '1')
    {
      Serial.println("Serial set to: 115200");
      SerialLog.begin(115200);
    }
    else if (incoming == '2')
    {
      Serial.println("Serial set to: 230400");
      SerialLog.begin(230400);
    }
    else if (incoming == '3')
    {
      Serial.println("Serial set to: 460800");
      SerialLog.begin(460800);
    }
    else if (incoming == '4')
    {
      Serial.println("Serial set to: 921600");
      SerialLog.begin(921600);
    }
    else if (incoming == '5')
    {
      Serial.println("Serial set to: 500000");
      SerialLog.begin(500000);
    }
    else if (incoming == 'r')
    {
      Serial.println("Total count reset");
      charsReceived = 0;
    }
    else if (incoming == '\r' || incoming == '\n')
    {
      //Do nothing
    }
    else
    {
      Serial.println("Not recognized");
    }
  }

  //Delay until we reach the requested read rate
  //  if (millis() - overallStartTime > 500)
  //  {
  //    overallStartTime = millis();
  //    if (digitalRead(statLED) == HIGH)
  //      digitalWrite(statLED, LOW);
  //    else
  //      digitalWrite(statLED, HIGH);
  //  }
}

void microSDPowerOn()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, LOW);
}
void microSDPowerOff()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, HIGH);
}
