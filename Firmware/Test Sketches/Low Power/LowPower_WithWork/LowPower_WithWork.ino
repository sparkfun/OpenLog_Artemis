/*
  Artemis Low Power: How low can we go?
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 26th, 2020
  License: This code is public domain.

  This example demonstrates how to do some work (talk to a device over I2C / IOM),
  enter deep sleep, then wake and continue work.

  SparkFun labored with love to create this code. Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15376

  How close can we get to 2.7uA in deep sleep?
  This example shows how decrease the Artemis current consumption to ~2.4uA in deep sleep
  with a wake up every 5 seconds to read a sensor.

  Note that Artemis modules with revision A1 silicon will use ~30uA. Please see the
  Ambiq errata for more information: https://www.ambiqmicro.com/static/mcu/files/Apollo3_Blue_Errata_List_v1_0_external_release.pdf

  To monitor the current cut the MEAS jumper, solder in headers, and attach
  a DMM via IC hooks (https://www.sparkfun.com/products/506).

  The USB to serial bridge draws some current:
    Serial Basic C - ~1.2uA (https://www.sparkfun.com/products/15096)
    FTDI Basic - ~5.5uA (https://www.sparkfun.com/products/9873)

  518uA on RedBoard running while(1);
  28uA on RedBoard running this sketch with MS5637 attached
  2.93uA on RedBoard with microphone VCC trace cut (mic was using ~25uA)
  2.78uA on RedBoard with MS5637 detached
*/

#include <Wire.h>

#include "SparkFun_MS5637_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_MS5637
MS5637 barometricSensor;

#include <SPI.h>
#define SPI_SPEED 1000000
#define SPI_ORDER MSBFIRST
#define SPI_MODE SPI_MODE0
SPISettings mySettings(SPI_SPEED, SPI_ORDER, SPI_MODE);

uint32_t msToSleep = 5000; //This is the user editable number of ms to sleep between RTC checks
#define TIMER_FREQ 32768L //Counter/Timer 6 will use the 32kHz clock
uint32_t sysTicksToSleep = msToSleep * TIMER_FREQ / 1000;

const byte STAT_LED = 13;

int counter = 0;

const byte PIN_IMU_POWER = 22;
const byte PIN_MICROSD_POWER = 15; //x04
const byte PIN_QWIIC_POWER = 18;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Artemis Low Power Example");

  //We don't really use Serial1 in this example but we power it up and use it just to show its use.
  Serial1.begin(115200);
  Serial1.println("Serial1 now online!");

  Wire.begin();

  //We don't really use SPI in this example but we power it up and use it just to show its use.
  SPI.begin();
  SPI.beginTransaction(mySettings);
  SPI.transfer(0xAA);
  SPI.endTransaction();

  if (barometricSensor.begin() == false)
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
  else
    Serial.println("MS5637 sensor detected.");
}

void loop(void) {

  float temperature = barometricSensor.getTemperature();
  float pressure = barometricSensor.getPressure();

  Serial.print("Temperature=");
  Serial.print(temperature, 1);
  Serial.print("(C)");

  Serial.print(" Pressure=");
  Serial.print(pressure, 3);
  Serial.print("(hPa or mbar)");

  Serial.print(" counter=");
  Serial.print(counter++);

  Serial.println();

  digitalWrite(STAT_LED, HIGH);
  delay(100);
  digitalWrite(STAT_LED, LOW);

  goToSleep();
}

//Power everything down and wait for interrupt wakeup
void goToSleep()
{
  Wire.end(); //Power down I2C

  SPI.end(); //Power down SPI
  //SPI1.end(); //This example doesn't use SPI1 but you will need to end any instance you may have created

  power_adc_disable(); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART
  Serial1.end();

  //Disable all pads
  for (int x = 0 ; x < 50 ; x++)
    am_hal_gpio_pinconfig(x , g_AM_HAL_GPIO_DISABLE);

  //We can't leave these power control pins floating
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pinMode(PIN_IMU_POWER, OUTPUT);
  pinMode(PIN_MICROSD_POWER, OUTPUT);

  qwiicPowerOff();
  imuPowerOff();
  microSDPowerOff();
  
  //We use counter/timer 6 to cause us to wake up from sleep but 0 to 7 are available
  //CT 7 is used for Software Serial. All CTs are used for Servo.
  am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG); //Clear CT6
  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREG); //Enable C/T G=6

  //Use the lower power 32kHz clock. Use it to run CT6 as well.
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);

  //Setup interrupt to trigger when the number of ms have elapsed
  am_hal_stimer_compare_delta_set(6, sysTicksToSleep);

  //Power down Flash, SRAM, cache
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE); //Turn off CACHE
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_FLASH_512K); //Turn off everything but lower 512k
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_SRAM_64K_DTCM); //Turn off everything but lower 64k
  //am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); //Turn off all memory (doesn't recover)

  //Enable the timer interrupt in the NVIC.
  NVIC_EnableIRQ(STIMER_CMPR6_IRQn);

  //Go to Deep Sleep.
  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

  //Turn off interrupt
  NVIC_DisableIRQ(STIMER_CMPR6_IRQn);

  //We're BACK!
  wakeFromSleep();
}

//Power everything up gracefully
void wakeFromSleep()
{
  //Power up SRAM, turn on entire Flash
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_MAX);
  
  //Go back to using the main clock
  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW);
  NVIC_EnableIRQ(STIMER_IRQn);
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ);

  //Turn on ADC
  ap3_adc_setup();

  //Set any pinModes
  pinMode(STAT_LED, OUTPUT);

  //Turn on Serial
  Serial.begin(115200);
  delay(10);
  Serial.println("Back on");

  //Turn on I2C
  Wire.begin();

  //Restart Sensors
  if (barometricSensor.begin() == false)
  {
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
  }
}

//Called once number of milliseconds has passed
extern "C" void am_stimer_cmpr6_isr(void)
{
  uint32_t ui32Status = am_hal_stimer_int_status_get(false);
  if (ui32Status & AM_HAL_STIMER_INT_COMPAREG)
  {
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG);
  }
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

void imuPowerOn()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, HIGH);
}
void imuPowerOff()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, LOW);
}
