/* Author: Nathan Seidle
  Created: Septempter 27th, 2019
  License: MIT. See SparkFun Arduino Apollo3 Project for more information

  This example demonstrates how to put the core to sleep for a number of
  milliseconds before waking and printing the current time/date. This
  is helpful for checking power consumption of the core while RTC+CT6 are running.
*/

#include "RTC.h" //Include RTC library included with the Aruino_Apollo3 core
APM3_RTC myRTC; //Create instance of RTC class

#define VERSION_X02

#ifdef VERSION_X02
const byte PIN_POWER_LOSS = 3;

const byte PIN_QWIIC_PWR = 18;
const byte PIN_ICM_PWR = 22;
const byte PIN_MICROSD_PWR = 23;
#endif

uint32_t msToSleep = 10; //This is the user editable number of ms to sleep between RTC checks
//#define TIMER_FREQ 3000000L //Counter/Timer 6 will use the HFRC oscillator of 3MHz
#define TIMER_FREQ 32768 //Counter/Timer 6 will use the external 32kHz Xtal
uint32_t sysTicksToSleep = msToSleep * (TIMER_FREQ / 1000);

void setup()
{
  //powerDown();
  Serial.begin(115200);
  Serial.println("SparkFun RTC Example");

  //pinMode(PIN_POWER_LOSS, INPUT);
  pinMode(PIN_POWER_LOSS, INPUT_PULLUP);
  pinMode(PIN_QWIIC_PWR, OUTPUT);
  pinMode(PIN_ICM_PWR, OUTPUT);
  pinMode(PIN_MICROSD_PWR, OUTPUT);

  digitalWrite(PIN_QWIIC_PWR, LOW);
  digitalWrite(PIN_ICM_PWR, LOW);
  digitalWrite(PIN_MICROSD_PWR, LOW);

  pinMode(11, OUTPUT);

  digitalWrite(11, HIGH);
  delay(50);
  digitalWrite(11, LOW);

  setupADCTimer();
  //attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);

  Serial.println("d) Display time");
  Serial.println("s) Set RTC to compiler macro");
  Serial.println("r) System reset");
  Serial.println("p) Power down");
  Serial.println("q) Toggle Qwiic Power");

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
      if(digitalRead(PIN_QWIIC_PWR) == HIGH)
        digitalWrite(PIN_QWIIC_PWR, LOW);
      else
        digitalWrite(PIN_QWIIC_PWR, HIGH);
    
    }
    while (Serial.available()) Serial.read();
  }

  //  //  Serial.printf(" Day of week: %d =", myRTC.weekday);
  //  //  Serial.printf(" %s", myRTC.textWeekday);

}

void powerDown()
{
  //This function takes 0.4296ms to run including GPIO setting
  //This puts the apoolo3 into 2.6uA, 2.36ua

  //int startTicks = micros(); //sysTicks();
  //digitalWrite(11, HIGH);

  digitalWrite(PIN_QWIIC_PWR, LOW);
  digitalWrite(PIN_ICM_PWR, LOW);
  digitalWrite(PIN_MICROSD_PWR, LOW);

  //Turn off ADC
  power_adc_disable();

  // Initialize for low power in the power control block
  am_hal_pwrctrl_low_power_init();

  // Stop the XTAL.
  //This stops the RTC from running
  //am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_STOP, 0);

  // Disable the RTC.
  //am_hal_rtc_osc_disable();

  // Disabling the debugger GPIOs saves about 1.2 uA total:
  am_hal_gpio_pinconfig(20 /* SWDCLK */, g_AM_HAL_GPIO_DISABLE);
  am_hal_gpio_pinconfig(21 /* SWDIO */, g_AM_HAL_GPIO_DISABLE);

  // These two GPIOs are critical: the TX/RX connections between the Artemis module and the CH340 on the Blackboard
  // are prone to backfeeding each other. To stop this from happening, we must reconfigure those pins as GPIOs
  // and then disable them completely:
  am_hal_gpio_pinconfig(48 /* TXO-0 */, g_AM_HAL_GPIO_DISABLE);
  am_hal_gpio_pinconfig(49 /* RXI-0 */, g_AM_HAL_GPIO_DISABLE);

  for (int x = 0 ; x < 50 ; x++)
    am_hal_gpio_pinconfig(x , g_AM_HAL_GPIO_DISABLE);

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

  Serial.end(); //Disable Serial
  //digitalWrite(11, LOW);

  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
}

//We use counter/timer 6 for this example but 0 to 7 are available
//CT 7 is used for Software Serial. All CTs are used for Servo.
void setupADCTimer()
{
  analogReadResolution(14); //Set resolution to 14 bit

  //Clear compare interrupt
  am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG); //Use CT6

  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREG); // Enable C/T G=6

  //Don't change from 3MHz system timer, but enable G timer
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  //  am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);

  //Setup ISR to trigger when the number of ms have elapsed
  am_hal_stimer_compare_delta_set(6, sysTicksToSleep);

  //Enable the timer interrupt in the NVIC.
  NVIC_EnableIRQ(STIMER_CMPR6_IRQn);
}

//Called once number of milliseconds has passed
extern "C" void am_stimer_cmpr6_isr(void)
{
  uint32_t ui32Status = am_hal_stimer_int_status_get(false);
  if (ui32Status & AM_HAL_STIMER_INT_COMPAREG)
  {
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG);

    //Reset compare value. ISR will trigger when the number of ms have elapsed
    am_hal_stimer_compare_delta_set(6, sysTicksToSleep);

    int div3 = analogRead(ADC_INTERNAL_VCC_DIV3); //Read VCC across a 1/3 resistor divider
    float vcc = (float)div3 * 6 / 16384.0; //Convert 1/3 VCC to VCC
    //    Serial.print(" VCC: ");
    //    Serial.print(vcc, 2);
    //    Serial.print("V");
    //    Serial.println();

    if (vcc < 3.05)
    {
      NVIC_DisableIRQ(STIMER_CMPR6_IRQn);
      powerDown();
    }
  }
}
