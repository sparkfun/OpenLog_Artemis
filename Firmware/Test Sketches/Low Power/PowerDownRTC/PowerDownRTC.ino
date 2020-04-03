/* Author: Nathan Seidle
  Created: Septempter 27th, 2019
  License: MIT. See SparkFun Arduino Apollo3 Project for more information

  This example demonstrates how to put the core to sleep for a number of
  milliseconds before waking and printing the current time/date. This
  is helpful for checking power consumption of the core while RTC+CT6 are running.

  3.2 microSD, no USB
  2.4 no microSD, no USB

  6mA with Power LED, no microSD, USB
  4.58, no power LED, no microSD, USB
  So power LED is ~1.5mA
  1.05mA no power, no microSD, no USB
  So USB is ~3.5mA
  Fully powered down, board is pulling 450uA from battery but that
  seems to be because RTC batt is charging at ~360uA.

  6.88 with microSD
  5.07 with microSD on?

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



*/

#include "RTC.h" //Include RTC library included with the Aruino_Apollo3 core
APM3_RTC myRTC; //Create instance of RTC class

const byte PIN_POWER_LOSS = 3;

const byte LOGIC_DEBUG = 11;

const byte PIN_QWIIC_POWER = 18;
const byte PIN_IMU_POWER = 22;
const byte PIN_MICROSD_POWER = 15;

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

  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pinMode(PIN_IMU_POWER, OUTPUT);
  pinMode(PIN_MICROSD_POWER, OUTPUT);

  digitalWrite(PIN_QWIIC_POWER, LOW);
  digitalWrite(PIN_IMU_POWER, LOW);
  digitalWrite(PIN_MICROSD_POWER, LOW);

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
        digitalWrite(PIN_QWIIC_POWER, LOW);
        Serial.println("Qwiic Power On / Pin Low");
      }
      else
      {
        digitalWrite(PIN_QWIIC_POWER, HIGH);
        pinMode(5, OUTPUT);
        digitalWrite(5, LOW);
        pinMode(6, OUTPUT);
        digitalWrite(6, LOW);
        pinMode(7, OUTPUT);
        digitalWrite(7, LOW);
        pinMode(10, OUTPUT);
        digitalWrite(10, LOW);
        Serial.println("Qwiic Power Off / Pin High");
      }
    }
    else if (choice == 'm')
    {
      Serial.println("Toggle microSD Power");
      if (digitalRead(PIN_MICROSD_POWER) == HIGH)
      {
        digitalWrite(PIN_MICROSD_POWER, LOW);
        pinMode(5, INPUT);
        pinMode(6, INPUT);
        pinMode(7, INPUT);
        pinMode(10, INPUT);
        Serial.println("microSD On / Pin Low");
      }
      else
      {
        digitalWrite(PIN_MICROSD_POWER, HIGH);
        Serial.println("microSD Off / Pin High");
      }
    }    while (Serial.available()) Serial.read();
  }
}

//Power down the entire system but maintain running of RTC
//This function takes 100us to run including GPIO setting
//This puts the Apollo3 into 2.36uA to 2.6uA consumption mode
void powerDown()
{
  //digitalWrite(LOGIC_DEBUG, HIGH);

  //Force the peripherals off
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM1);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM2);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM3);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM4);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM5);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART1);

  //The supervisor circuit tends to wake us from sleep if it
  //remains as an interrupt
  detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS));

  //Serial.flush(); //Finish any prints

  //Wire.end(); //Power down I2C
  //qwiic.end(); //Power down I2C

  //SPI.end(); //Power down SPI

  power_adc_disable(); //Power down ADC. It it started by default before setup().

  //  Serial.end(); //Power down UART
  //  Serial1.end();

  for (int x = 0 ; x < 50 ; x++)
    am_hal_gpio_pinconfig(x , g_AM_HAL_GPIO_DISABLE);

  //We can't leave these power control pins floating
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pinMode(PIN_IMU_POWER, OUTPUT);
  pinMode(PIN_MICROSD_POWER, OUTPUT);

  digitalWrite(PIN_QWIIC_POWER, HIGH); //High = off
  digitalWrite(PIN_IMU_POWER, LOW);
  digitalWrite(PIN_MICROSD_POWER, HIGH); //High = off

const byte PIN_SPI_MISO = 6;
const byte PIN_SPI_MOSI = 7;
const byte PIN_SPI_SCK = 5;
const byte PIN_SPI_CS = 10;

  pinMode(PIN_SPI_CS, OUTPUT);
  digitalWrite(PIN_SPI_CS, HIGH); //pullup the CS pin on the SD card (but only if you don’t already have a hardware pullup on your module)
  pinMode(PIN_SPI_MOSI, OUTPUT);
  digitalWrite(PIN_SPI_MOSI, HIGH); //pullup the MOSI pin on the SD card
  pinMode(PIN_SPI_MISO, INPUT_PULLUP); //pullup the MISO pin on the SD card
  pinMode(PIN_SPI_SCK, OUTPUT);
  digitalWrite(PIN_SPI_SCK, LOW); //pull DOWN the 13scl pin on the SD card (IDLES LOW IN MODE0)
  // NOTE: In Mode (0), the SPI interface holds the CLK line low when the bus is inactive, so DO NOT put a pullup on it.


  // The default Arduino environment runs the System Timer (STIMER) off the 48 MHZ HFRC clock source.
  // The HFRC appears to take over 60 uA when it is running, so this is a big source of extra
  // current consumption in deep sleep.
  // For systems that might want to use the STIMER to generate a periodic wakeup, it needs to be left running.
  // However, it does not have to run at 48 MHz. If we reconfigure STIMER (system timer) to use the 32768 Hz
  // XTAL clock source instead the measured deepsleep power drops by about 64 uA.
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);

  // This option selects 32768 Hz via crystal osc. This appears to cost about 0.1 uA versus selecting "no clock"
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

  // This option would be available to systems that don't care about passing time, but might be set
  // to wake up on a GPIO transition interrupt.
  // am_hal_stimer_config(AM_HAL_STIMER_NO_CLK);

  // Turn OFF Flash1
  am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_512K);

  // Power down SRAM
  PWRCTRL->MEMPWDINSLEEP_b.SRAMPWDSLP = PWRCTRL_MEMPWDINSLEEP_SRAMPWDSLP_ALLBUTLOWER32K;

  //Serial.end(); //Disable Serial
  //digitalWrite(LOGIC_DEBUG, LOW);

  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
}

//We use counter/timer 6 for this example but 0 to 7 are available
//CT 7 is used for Software Serial. All CTs are used for Servo.
//void setupADCTimer()
//{
//  analogReadResolution(14); //Set resolution to 14 bit
//
//  //Clear compare interrupt
//  am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG); //Use CT6
//
//  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREG); // Enable C/T G=6
//
//  //Don't change from 3MHz system timer, but enable G timer
//  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
//  //  am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);
//  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);
//
//  //Setup ISR to trigger when the number of ms have elapsed
//  am_hal_stimer_compare_delta_set(6, sysTicksToSleep);
//
//  //Enable the timer interrupt in the NVIC.
//  NVIC_EnableIRQ(STIMER_CMPR6_IRQn);
//}

//Called once number of milliseconds has passed
//extern "C" void am_stimer_cmpr6_isr(void)
//{
//  uint32_t ui32Status = am_hal_stimer_int_status_get(false);
//  if (ui32Status & AM_HAL_STIMER_INT_COMPAREG)
//  {
//    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG);
//
//    //Reset compare value. ISR will trigger when the number of ms have elapsed
//    am_hal_stimer_compare_delta_set(6, sysTicksToSleep);
//
//    int div3 = analogRead(ADC_INTERNAL_VCC_DIV3); //Read VCC across a 1/3 resistor divider
//    float vcc = (float)div3 * 6 / 16384.0; //Convert 1/3 VCC to VCC
//    //    Serial.print(" VCC: ");
//    //    Serial.print(vcc, 2);
//    //    Serial.print("V");
//    //    Serial.println();
//
//    if (vcc < 3.05)
//    {
//      NVIC_DisableIRQ(STIMER_CMPR6_IRQn);
//      powerDown();
//    }
//  }
//}
