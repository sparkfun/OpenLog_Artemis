//Power down the entire system but maintain running of RTC
//This function takes 100us to run including GPIO setting
//This puts the Apollo3 into 2.36uA to 2.6uA consumption mode
//With leakage across the 3.3V protection diode, it's approx 3.00uA.
void powerDown()
{
  //digitalWrite(LOGIC_DEBUG, HIGH);

  detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS)); //Prevent voltage supervisor from waking us from sleep

  //Save files before going to sleep
  //  if (online.dataLogging == true)
  //  {
  //    sensorDataFile.sync();
  //  }
  //  if (online.serialLogging == true)
  //  {
  //    serialDataFile.sync();
  //  }

  //Serial.flush(); //Don't waste time waiting for prints to finish

  //  Wire.end(); //Power down I2C
  qwiic.end(); //Power down I2C

  SPI.end(); //Power down SPI

  power_adc_disable(); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART
  SerialLog.end();

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

  //Disable all pads
  for (int x = 0 ; x < 50 ; x++)
    am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);

  //We can't leave these power control pins floating
  qwiicPowerOff();
  imuPowerOff();
  microSDPowerOff();

  //Power down Flash, SRAM, cache
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE); //Turn off CACHE
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_FLASH_512K); //Turn off everything but lower 512k
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_SRAM_64K_DTCM); //Turn off everything but lower 64k
  //am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); //Turn off all memory (doesn't recover)

  //Keep the 32kHz clock running for RTC
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

  //digitalWrite(LOGIC_DEBUG, LOW);

  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP); //Sleep forever
}

//Power everything down and wait for interrupt wakeup
void goToSleep()
{
  uint32_t msToSleep = settings.usBetweenReadings / 1000;
  uint32_t sysTicksToSleep = msToSleep * 32768L / 1000; //Counter/Timer 6 will use the 32kHz clock

  detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS)); //Prevent voltage supervisor from waking us from sleep

  //Save files before going to sleep
  if (online.dataLogging == true)
  {
    sensorDataFile.sync();
    sensorDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
  }
  if (online.serialLogging == true)
  {
    serialDataFile.sync();
    serialDataFile.close();
  }

  Serial.flush(); //Finish any prints

  //  Wire.end(); //Power down I2C
  qwiic.end(); //Power down I2C

  SPI.end(); //Power down SPI

  power_adc_disable(); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART
  SerialLog.end();

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

  //Disable all pads
  for (int x = 0 ; x < 50 ; x++)
    am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);

  //We can't leave these power control pins floating
  imuPowerOff();
  //microSDPowerOff();

  //Testing file record issues
  microSDPowerOn();

  //Keep Qwiic bus powered on if user desires it
  if (settings.powerDownQwiicBusBetweenReads == true)
    qwiicPowerOff();
  else
    qwiicPowerOn(); //Make sure pins stays as output

  //Power down Flash, SRAM, cache
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE); //Turn off CACHE
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_FLASH_512K); //Turn off everything but lower 512k
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_SRAM_64K_DTCM); //Turn off everything but lower 64k
  //am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); //Turn off all memory (doesn't recover)

  //Use the lower power 32kHz clock. Use it to run CT6 as well.
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);

  //Adjust sysTicks down by the amount we've be at 48MHz
  uint32_t msBeenAwake = millis();
  uint32_t sysTicksAwake = msBeenAwake * 32768L / 1000; //Convert to 32kHz systicks
  sysTicksToSleep -= sysTicksAwake;

  //Setup interrupt to trigger when the number of ms have elapsed
  am_hal_stimer_compare_delta_set(6, sysTicksToSleep);

  //We use counter/timer 6 to cause us to wake up from sleep but 0 to 7 are available
  //CT 7 is used for Software Serial. All CTs are used for Servo.
  am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG); //Clear CT6
  am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREG); //Enable C/T G=6

  //Enable the timer interrupt in the NVIC.
  NVIC_EnableIRQ(STIMER_CMPR6_IRQn);

  //Go to Deep Sleep.
  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

  //Turn off interrupt
  NVIC_DisableIRQ(STIMER_CMPR6_IRQn);

  am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREG); //Enable C/T G=6

  //We're BACK!
  wakeFromSleep();
}

//Power everything up gracefully
void wakeFromSleep()
{
  //Power up SRAM, turn on entire Flash
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_MAX);

  //Go back to using the main clock
  //am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW);
  //NVIC_EnableIRQ(STIMER_IRQn);
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ);

  //Turn on ADC
  ap3_adc_setup();

  //Run setup again

  //If 3.3V rail drops below 3V, system will enter low power mode and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);

  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, LOW);

  Serial.begin(settings.serialTerminalBaudRate);

  SPI.begin(); //Needed if SD is disabled

  beginSD(); //285 - 293ms

  beginQwiic();

  beginDataLogging(); //180ms

  beginSerialLogging(); //20 - 99ms

  beginIMU(); //61ms

  beginSensors(); //159 - 865ms but varies based on number of devices attached

  //Serial.printf("Wake up time: %.02f ms\n", (micros() - startTime) / 1000.0);

  //When we wake up micros has been reset to zero so we need to let the main loop know to take a reading
  takeReading = true;
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

//Returns the number of milliseconds according to the RTC
//Watch out for 24 hour roll over at 86,400,000ms
uint32_t rtcMillis()
{
    myRTC.getTime();
    uint32_t millisToday = 0;
    millisToday += (myRTC.hour * 3600000UL);
    millisToday += (myRTC.minute * 60000UL);
    millisToday += (myRTC.seconds * 1000UL);
    millisToday += (myRTC.hundredths * 10UL);

    return(millisToday);  
}
