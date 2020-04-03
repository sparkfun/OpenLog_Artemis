/*
  Power down all parts of the board
  Wake after three seconds
  Take and log reading
  Power back down

  Qwiic power pin takes ~2uA
  Qwiic is turning on/off 6.18mA to 4.52mA
  Core is running at 606uA and correctly shuts down to 2uA but board is pulling 3.91mA.

  Hmm, disabling the port in device manager gets me to 1.07mA. With micro in LP, I'm at 0.48mA.
  With USB disabled, in LP, board is 0.288mA from battery. 0.195 with microSD evacuated.
  CH340E is 0.09mA in suspend mode.
  MCP73831 is ~1uA on battery (Vdd <= Vbat - 50mV)
  AP2112K is ~50uA
  So I think that's where the ~169uA

  .88mA in LP, with SD, no USB suspend

  On batt, no LP, 0.739mA! Charge LED is on. Core is at .583mA (0.150mA)
  0.173mA LP, no SD, on batt. No suspend over USB.
  0.169ma LP, no SD, on batt, with suspend over USB.

  Future rev - maybe attach to VUSB to detect when USB is connected or not.

  Working:
  2.22uA LP, 250uA board. Wake every 5 s.


  The hard power down of IOMs seems to be an issue.
  We should implement IOM power down with .end functions
  Wire, SPI, UART
  Working... 140uA in sleep
  205 with all IOMs not disabled so something is taking power
  131 with UARTs powered down so they seem to be a culprit
  127 without hard IOM power down. Good.
  
*/

#include <Wire.h>

#include "SparkFun_MS5637_Arduino_Library.h"

MS5637 barometricSensor;
TwoWire qwiic(1); //Will use pads 8/9

uint32_t msToSleep = 3000; //This is the user editable number of ms to sleep between RTC checks
#define TIMER_FREQ 32768L //Counter/Timer 6 will use the 32kHz clock
uint32_t sysTicksToSleep = msToSleep * TIMER_FREQ / 1000;

const byte PIN_QWIIC_POWER = 18;
const byte PIN_STAT_LED = 19;
const byte PIN_IMU_POWER = 22;
const byte PIN_MICROSD_POWER = 15;

void setup(void) {
  Serial.begin(115200);
  Serial.println("Qwiic Pressure Sensor MS5637 Example");

  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pinMode(PIN_IMU_POWER, OUTPUT);
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pinMode(PIN_STAT_LED, OUTPUT);

  digitalWrite(PIN_QWIIC_POWER, LOW);
  digitalWrite(PIN_IMU_POWER, LOW);
  digitalWrite(PIN_MICROSD_POWER, LOW);

  qwiicPowerOn();
  delay(100);
  qwiic.begin();

  if (barometricSensor.begin(qwiic) == false)
  {
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
    //while (1);
  }
  else
    Serial.println("MS5637 sensor detected.");

  Serial.flush();
  qwiic.end();

  delay(100);
  qwiic.begin();
  if (barometricSensor.begin(qwiic) == false)
  {
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
    //while (1);
  }
  else
    Serial.println("MS5637 sensor detected.");

  Serial.println("Setup done");
}

void loop(void) {

  float temperature = 1.2;
  float pressure = 2.5;
  //temperature = barometricSensor.getTemperature();
  //pressure = barometricSensor.getPressure();

  Serial.print("Temperature=");
  Serial.print(temperature, 1);
  Serial.print("(C)");

  Serial.print(" Pressure=");
  Serial.print(pressure, 3);
  Serial.print("(hPa or mbar)");

  Serial.println();

  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, HIGH);
  delay(100);
  digitalWrite(PIN_STAT_LED, LOW);

  goToSleep();
}

//Power everything down and wait for RTC interrupt wakeup
void goToSleep()
{
  //digitalWrite(LOGIC_DEBUG, HIGH);

  qwiic.end(); //Power down I2C

  Serial.end(); //Power down UART

  power_adc_disable(); //Power down ADC
  
  digitalWrite(PIN_QWIIC_POWER, HIGH); //HIGH = Off
  digitalWrite(PIN_IMU_POWER, LOW);
  digitalWrite(PIN_MICROSD_POWER, LOW);

  for (int x = 0 ; x < 50 ; x++)
    am_hal_gpio_pinconfig(x , g_AM_HAL_GPIO_DISABLE);

  //We can't leave these power control pins floating
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pinMode(PIN_IMU_POWER, OUTPUT);
  pinMode(PIN_MICROSD_POWER, OUTPUT);

  qwiicPowerOff();
  digitalWrite(PIN_IMU_POWER, LOW);
  digitalWrite(PIN_MICROSD_POWER, LOW);

  //We use counter/timer 6 to cause us to wake up from sleep but 0 to 7 are available
  //CT 7 is used for Software Serial. All CTs are used for Servo.
  am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG); //Clear CT6
  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREG); //Enable C/T G=6

  //Use the lower power 32kHz clock
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);

  //Right here we need to reduce the sleep sys ticks by the amount we've been awake so that we hit exactly 0.12Hz

  //Setup interrupt to trigger when the number of ms have elapsed
  am_hal_stimer_compare_delta_set(6, sysTicksToSleep);

  // Turn OFF Flash1
  am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_512K);

  // Power down SRAM
  // Nathan seems to have gone a little off script here and isn't using
  // am_hal_pwrctrl_memory_deepsleep_powerdown or
  // am_hal_pwrctrl_memory_deepsleep_retain. I wonder why?
  //PWRCTRL->MEMPWDINSLEEP_b.SRAMPWDSLP = PWRCTRL_MEMPWDINSLEEP_SRAMPWDSLP_ALLBUTLOWER32K;
  PWRCTRL->MEMPWDINSLEEP_b.SRAMPWDSLP = PWRCTRL_MEMPWDINSLEEP_SRAMPWDSLP_ALLBUTLOWER64K;

  //Enable the timer interrupt in the NVIC.
  NVIC_EnableIRQ(STIMER_CMPR6_IRQn);

  // Go to Deep Sleep.
  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

  //Turn off interrupt
  NVIC_DisableIRQ(STIMER_CMPR6_IRQn);

  //We're BACK!
  wakeFromSleep();
}

//Power everything up gracefully
void wakeFromSleep()
{
  // Set the clock frequency. (redundant?)
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

  // Set the default cache configuration. (redundant?)
  am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
  am_hal_cachectrl_enable();

  // Note: because we called setupRTC earlier,
  // we do NOT want to call am_bsp_low_power_init() here.
  // It would configure the board for low power operation
  // and calls am_hal_pwrctrl_low_power_init()
  // but it also stops the RTC oscillator!
  // (BSP = Board Support Package)

  // Initialize for low power in the power control block.  (redundant?)
  am_hal_pwrctrl_low_power_init();

  // Power up SRAM
  PWRCTRL->MEMPWDINSLEEP_b.SRAMPWDSLP = PWRCTRL_MEMPWDINSLEEP_SRAMPWDSLP_NONE;

  // Turn on Flash
  // There is a chance this could fail but I guess we should move on regardless and not do a while(1);
  am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_ALL);

  // Go back to using the main clock
  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW); // (posssibly redundant?)
  NVIC_EnableIRQ(STIMER_IRQn); // (posssibly redundant?)
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ);

  // Restore the TX/RX connections between the Artemis module and the CH340S
  am_hal_gpio_pinconfig(48 /* TXO-0 */, g_AM_BSP_GPIO_COM_UART_TX);
  am_hal_gpio_pinconfig(49 /* RXI-0 */, g_AM_BSP_GPIO_COM_UART_RX);

  // Reenable the debugger GPIOs
  am_hal_gpio_pinconfig(20 /* SWDCLK */, g_AM_BSP_GPIO_SWDCK);
  am_hal_gpio_pinconfig(21 /* SWDIO */, g_AM_BSP_GPIO_SWDIO);

  // Turn on ADC
  ap3_adc_setup();

  // Start the console serial port again (zzz will have ended it)
  Serial.begin(115200);
  Serial.println("Back on");

  //Turn on Qwiic
  qwiicPowerOn();
  delay(100);
  qwiic.begin();

  //Restart Sensors
  if (barometricSensor.begin(qwiic) == false)
  {
    Serial.println("MS5637 sensor did not respond. Please check wiring.");
    //while (1);
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
  digitalWrite(PIN_QWIIC_POWER, LOW);
}

void qwiicPowerOff()
{
  digitalWrite(PIN_QWIIC_POWER, HIGH);
}
