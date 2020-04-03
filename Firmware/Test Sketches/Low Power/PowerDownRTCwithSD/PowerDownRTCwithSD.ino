/* Author: Nathan Seidle
  Created: Septempter 27th, 2019
  License: MIT. See SparkFun Arduino Apollo3 Project for more information

  This example demonstrates how to put the core to sleep for a number of
  milliseconds before waking and printing the current time/date. This
  is helpful for checking power consumption of the core while RTC+CT6 are running.

  6mA with Power LED, no microSD, USB
  4.58, no power LED, no microSD, USB
  So power LED is ~1.5mA

  1.05mA no power LED, no microSD, no USB
  So USB is ~3.5mA

  Fully powered down, board is pulling 450uA from battery but that
  seems to be because RTC batt is charging at ~360uA.

  Measurements should be done at the MEAS jumper with the RTC battery disconnected. 

  3/5/20 -
    6.61mA, no LED, USB, SD
    4.78mA, no LED, USB, no SD
    0.67mA, no LED, no USB, SD with pin set LOW
    0.65mA, no LED, no USB, SD with pin set HIGH
    0.16mA, no LED, no USB, no SD

  3/6/20
    0.65mA, no LED, no USB, Kingston 16GB SD
    0.152mA, no LED, no USB, SparkX SD
    0.191mA, no LED, no USB, SparkX SD
    0.188mA, no LED, no USB, SparkX SD
    0.098mA, no LEd, no USB, no SD

    Various SD cards pull different current even when shut off.

  3/18/20
    0.171mA no USB, no SD
    0.110mA no USB, no SD after overnight 

*/

#include "RTC.h" //Include RTC library included with the Aruino_Apollo3 core
APM3_RTC myRTC; //Create instance of RTC class

#include <Wire.h>

//microSD Interface
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <SPI.h>
#include <SdFat.h> //We use SdFat-Beta from Bill Greiman for increased read/write speed

const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_MICROSD_POWER = 15; //x04

#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz
#define SD_CONFIG_MAX_SPEED SdSpiConfig(PIN_MICROSD_CHIP_SELECT, DEDICATED_SPI, SD_SCK_MHZ(24)) //Max of 24MHz

SdFat sd;
File sensorDataFile; //File that all sensor data is written to
File serialDataFile; //File that all incoming serial data is written to

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

const byte PIN_POWER_LOSS = 3;

const byte LOGIC_DEBUG = 11;

const byte PIN_QWIIC_POWER = 18;
const byte PIN_IMU_POWER = 22;

//uint32_t msToSleep = 10; //This is the user editable number of ms to sleep between RTC checks
////#define TIMER_FREQ 3000000L //Counter/Timer 6 will use the HFRC oscillator of 3MHz
//#define TIMER_FREQ 32768 //Counter/Timer 6 will use the external 32kHz Xtal
//uint32_t sysTicksToSleep = msToSleep * (TIMER_FREQ / 1000);

void setup()
{
  pinMode(PIN_POWER_LOSS, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);

  Serial.begin(115200);
  Serial.println("SparkFun RTC Example");

  Wire.begin();

  qwiicPowerOff();
  microSDPowerOff();
  imuPowerOff();
  SPI.begin();

  beginSD();

  pinMode(LOGIC_DEBUG, OUTPUT);

  digitalWrite(LOGIC_DEBUG, HIGH);
  delay(50);
  digitalWrite(LOGIC_DEBUG, LOW);

  myRTC.getTime();
  Serial.printf(" %02d:", myRTC.hour);
  Serial.printf("%02d:", myRTC.minute);
  Serial.printf("%02d", myRTC.seconds);
  Serial.printf(" %02d/", myRTC.month);
  Serial.printf("%02d/", myRTC.dayOfMonth);
  Serial.printf("%02d", myRTC.year);
  Serial.println();

  Serial.println("d) Display time");
  Serial.println("s) Set RTC to compiler macro");
  Serial.println("r) System reset");
  Serial.println("p) Power down");
  Serial.println("q) Toggle Qwiic Power");
  Serial.println("m) Toggle microSD Power");

}

void loop()
{
  if (Serial.available())
  {
    byte choice = Serial.read();

    if (choice == 's')
    {
      Serial.println("Set RTC to compiler");
      myRTC.setToCompilerTime(); //Easily set RTC using the system __DATE__ and __TIME__ macros from compiler
      //myRTC.setTime(7, 28, 51, 0, 21, 10, 15); //Manually set RTC back to the future: Oct 21st, 2015 at 7:28.51 AM
      //myRTC.setTime(1, 0, 0, 0, 21, 10, 15); //Manually set RTC back to the future: Oct 21st, 2015 at 7:28.51 AM
    }
    else if (choice == 'r')
    {
      Serial.println("System reset");
      am_hal_reset_control(AM_HAL_RESET_CONTROL_SWPOI, 0); //Cause a system Power On Init to release as much of the stack as possible
    }
    else if (choice == 'p')
    {
      Serial.println("Power down");
      powerDown();
    }
    else if (choice == 'd')
    {
      myRTC.getTime();

      Serial.printf(" %02d:", myRTC.hour);
      Serial.printf("%02d:", myRTC.minute);
      Serial.printf("%02d", myRTC.seconds);

      Serial.printf(" %02d/", myRTC.month);
      Serial.printf("%02d/", myRTC.dayOfMonth);
      Serial.printf("%02d", myRTC.year);

      Serial.println();
    }
    else if (choice == 'q')
    {
      Serial.println("Toggle Qwiic Power");
      if (digitalRead(PIN_QWIIC_POWER) == HIGH)
      {
        qwiicPowerOn();
        Serial.println("Qwiic Power On / Pin Low");
      }
      else
      {
        qwiicPowerOff();
        Serial.println("Qwiic Power Off / Pin High");
      }
    }
    else if (choice == 'm')
    {
      Serial.println("Toggle microSD Power");
      if (digitalRead(PIN_MICROSD_POWER) == HIGH)
      {
        microSDPowerOn();
        Serial.println("microSD On / Pin Low");
      }
      else
      {
        microSDPowerOff();
        Serial.println("microSD Off / Pin High");
      }
    }

    while (Serial.available()) Serial.read();
  }
}

//Power down the entire system but maintain running of RTC
//This function takes 100us to run including GPIO setting
//This puts the Apollo3 into 2.36uA to 2.6uA consumption mode
void powerDown()
{
  //digitalWrite(LOGIC_DEBUG, HIGH);

  //The supervisor circuit tends to wake us from sleep if it
  //remains as an interrupt
  detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS));

  Serial.flush(); //Finish any prints

  Wire.end(); //Power down I2C
  //qwiic.end(); //Power down I2C

  SPI.end(); //Power down SPI

  power_adc_disable(); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART
  Serial1.end();

  for (int x = 0 ; x < 50 ; x++)
      am_hal_gpio_pinconfig(x , g_AM_HAL_GPIO_DISABLE);


  //We can't leave these power control pins floating
  qwiicPowerOff();
  imuPowerOff();
  //microSDPowerOff(); //355

//  const byte PIN_SPI_MISO = 6;
//  const byte PIN_SPI_MOSI = 7;
//  const byte PIN_SPI_SCK = 5;
//  const byte PIN_SPI_CS = 10;
//
//  pinMode(PIN_SPI_CS, OUTPUT);
//  digitalWrite(PIN_SPI_CS, HIGH); //pullup the CS pin on the SD card (but only if you don’t already have a hardware pullup on your module)
//  pinMode(PIN_SPI_MOSI, OUTPUT);
//  digitalWrite(PIN_SPI_MOSI, HIGH); //pullup the MOSI pin on the SD card
//  pinMode(PIN_SPI_MISO, INPUT_PULLUP); //pullup the MISO pin on the SD card
//  pinMode(PIN_SPI_SCK, OUTPUT);
//  digitalWrite(PIN_SPI_SCK, LOW); //pull DOWN the 13scl pin on the SD card (IDLES LOW IN MODE0)
//  // NOTE: In Mode (0), the SPI interface holds the CLK line low when the bus is inactive, so DO NOT put a pullup on it.

  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

  //Power down Flash, SRAM, cache
  //  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE); //Turn off CACHE
  //  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_FLASH_512K); //Turn off everything but lower 512k
  //  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_SRAM_64K_DTCM); //Turn off everything but lower 64k
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); //Turn off all memory (doesn't recover)

  //digitalWrite(LOGIC_DEBUG, LOW);

  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP); //Sleep forever
}
