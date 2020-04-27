/*
  Power down all parts of the board
  Wake after three seconds
  Write to a file
  Power back down
*/

//#include <Wire.h>

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

char sensorDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
char serialDataFileName[30] = ""; //We keep a record of this file name so that we can re-open it upon wakeup from sleep
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

uint32_t msToSleep = 3000; //This is the user editable number of ms to sleep between RTC checks
#define TIMER_FREQ 32768L //Counter/Timer 6 will use the 32kHz clock
uint32_t sysTicksToSleep = msToSleep * TIMER_FREQ / 1000;

const byte PIN_QWIIC_POWER = 18;
const byte PIN_STAT_LED = 19;
const byte PIN_IMU_POWER = 22;

void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Serial.println("SD Write Test");

  qwiicPowerOff();
  microSDPowerOff();
  imuPowerOff();

  SPI.begin();

  beginSD();

  beginDataLogging();

  Serial.println("Setup done");
}

void loop(void)
{
  String outputData = "Test string\n";

  char temp[512];
  outputData.toCharArray(temp, 512); //Convert string to char array so sdfat can record it
  sensorDataFile.write(temp, strlen(temp)); //Record the buffer to the card
  sensorDataFile.sync();

  Serial.println("File recorded");

  goToSleep();
}

//Power everything down and wait for RTC interrupt wakeup
void goToSleep()
{
  //digitalWrite(LOGIC_DEBUG, HIGH);

  Serial.flush();
  Serial.end(); //Power down UART

  power_adc_disable(); //Power down ADC

  for (int x = 0 ; x < 50 ; x++)
    am_hal_gpio_pinconfig(x , g_AM_HAL_GPIO_DISABLE);

  qwiicPowerOff();
  microSDPowerOff();
  imuPowerOff();

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
