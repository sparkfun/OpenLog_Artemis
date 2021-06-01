// Read the battery voltage
// If it is low, increment lowBatteryReadings
// If lowBatteryReadings exceeds lowBatteryReadingsLimit then powerDown
void checkBattery(void)
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  if (settings.enableLowBatteryDetection == true)
  {
    float voltage = readVIN(); // Read the battery voltage
    if (voltage < settings.lowBatteryThreshold) // Is the voltage low?
    {
      lowBatteryReadings++; // Increment the low battery count
      if (lowBatteryReadings > lowBatteryReadingsLimit) // Have we exceeded the low battery count limit?
      {
        // Gracefully powerDown

        //Save files before powerDown
        if (online.dataLogging == true)
        {
          sensorDataFile.sync();
          updateDataFileAccess(&sensorDataFile); // Update the file access time & date
          sensorDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
        }
        if (online.serialLogging == true)
        {
          serialDataFile.sync();
          updateDataFileAccess(&serialDataFile); // Update the file access time & date
          serialDataFile.close();
        }
      
        delay(sdPowerDownDelay); // Give the SD card time to finish writing ***** THIS IS CRITICAL *****

        SerialPrintln(F("***      LOW BATTERY VOLTAGE DETECTED! GOING INTO POWERDOWN      ***"));
        SerialPrintln(F("*** PLEASE CHANGE THE POWER SOURCE AND RESET THE OLA TO CONTINUE ***"));
      
        SerialFlush(); //Finish any prints

        powerDown(); // power down and wait for reset
      }
    }
    else
    {
      lowBatteryReadings = 0; // Reset the low battery count (to reject noise)
    }    
  }
#endif
}

//Power down the entire system but maintain running of RTC
//This function takes 100us to run including GPIO setting
//This puts the Apollo3 into 2.36uA to 2.6uA consumption mode
//With leakage across the 3.3V protection diode, it's approx 3.00uA.
void powerDown()
{
  //Prevent voltage supervisor from waking us from sleep
  detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS));

  //Prevent stop logging button from waking us from sleep
  if (settings.useGPIO32ForStopLogging == true)
  {
    detachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING)); // Disable the interrupt
    pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
  }

  //Prevent trigger from waking us from sleep
  if (settings.useGPIO11ForTrigger == true)
  {
    detachInterrupt(digitalPinToInterrupt(PIN_TRIGGER)); // Disable the interrupt
    pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up
  }

  //WE NEED TO POWER DOWN ASAP - we don't have time to close the SD files
  //Save files before going to sleep
  //  if (online.dataLogging == true)
  //  {
  //    sensorDataFile.sync();
  //    sensorDataFile.close();
  //  }
  //  if (online.serialLogging == true)
  //  {
  //    serialDataFile.sync();
  //    serialDataFile.close();
  //  }

  //SerialFlush(); //Don't waste time waiting for prints to finish

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

  //Disable pads
  for (int x = 0; x < 50; x++)
  {
    if ((x != ap3_gpio_pin2pad(PIN_POWER_LOSS)) &&
        //(x != ap3_gpio_pin2pad(PIN_LOGIC_DEBUG)) &&
        (x != ap3_gpio_pin2pad(PIN_MICROSD_POWER)) &&
        (x != ap3_gpio_pin2pad(PIN_QWIIC_POWER)) &&
        (x != ap3_gpio_pin2pad(PIN_IMU_POWER)))
    {
      am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);
    }
  }

  //powerLEDOff();

  //Make sure PIN_POWER_LOSS is configured as an input for the WDT
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  //We can't leave these power control pins floating
  imuPowerOff();
  microSDPowerOff();

  //Keep Qwiic bus powered on if user desires it - but only for X04 to avoid a brown-out
#if(HARDWARE_VERSION_MAJOR == 0)
  if (settings.powerDownQwiicBusBetweenReads == true)
    qwiicPowerOff();
  else
    qwiicPowerOn(); //Make sure pins stays as output
#else
  qwiicPowerOff();
#endif

  //Power down cache, flash, SRAM
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); // Power down all flash and cache
  am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_384K); // Retain all SRAM

  //Keep the 32kHz clock running for RTC
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);

  while (1) // Stay in deep sleep until we get reset
  {
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP); //Sleep
  }
}

//Reset the Artemis
void resetArtemis(void)
{
  //Save files before resetting
  if (online.dataLogging == true)
  {
    sensorDataFile.sync();
    updateDataFileAccess(&sensorDataFile); // Update the file access time & date
    sensorDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
  }
  if (online.serialLogging == true)
  {
    serialDataFile.sync();
    updateDataFileAccess(&serialDataFile); // Update the file access time & date
    serialDataFile.close();
  }

  delay(sdPowerDownDelay); // Give the SD card time to finish writing ***** THIS IS CRITICAL *****

  SerialFlush(); //Finish any prints

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

  //Disable pads
  for (int x = 0; x < 50; x++)
  {
    if ((x != ap3_gpio_pin2pad(PIN_POWER_LOSS)) &&
        //(x != ap3_gpio_pin2pad(PIN_LOGIC_DEBUG)) &&
        (x != ap3_gpio_pin2pad(PIN_MICROSD_POWER)) &&
        (x != ap3_gpio_pin2pad(PIN_QWIIC_POWER)) &&
        (x != ap3_gpio_pin2pad(PIN_IMU_POWER)))
    {
      am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);
    }
  }

  //We can't leave these power control pins floating
  imuPowerOff();
  microSDPowerOff();

  //Disable power for the Qwiic bus
  qwiicPowerOff();

  //Disable the power LED
  powerLEDOff();

  //Enable the Watchdog so it can reset the Artemis
  startWatchdog();
  while (1) // That's all folks! Artemis will reset in 1.25 seconds
    ;
}

//Power everything down and wait for interrupt wakeup
void goToSleep(uint32_t sysTicksToSleep)
{
  //printDebug("goToSleep: sysTicksToSleep = " + (String)sysTicksToSleep + "\r\n");

  //printDebug("goToSleep: online.IMU = " + (String)online.IMU + "\r\n");

  //Prevent voltage supervisor from waking us from sleep
  detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS));

  //Prevent stop logging button from waking us from sleep
  if (settings.useGPIO32ForStopLogging == true)
  {
    detachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING)); // Disable the interrupt
    pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
  }

  //Prevent trigger from waking us from sleep
  //(This should be redundant. We should not be going to sleep if triggering is enabled?)
  if (settings.useGPIO11ForTrigger == true)
  {
    detachInterrupt(digitalPinToInterrupt(PIN_TRIGGER)); // Disable the interrupt
    pinMode(PIN_TRIGGER, INPUT); // Remove the pull-up
  }

  //Save files before going to sleep
  if (online.dataLogging == true)
  {
    sensorDataFile.sync();
    updateDataFileAccess(&sensorDataFile); // Update the file access time & date
    sensorDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
  }
  if (online.serialLogging == true)
  {
    serialDataFile.sync();
    updateDataFileAccess(&serialDataFile); // Update the file access time & date
    serialDataFile.close();
  }

  delay(sdPowerDownDelay); // Give the SD card time to finish writing ***** THIS IS CRITICAL *****

  SerialFlush(); //Finish any prints

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

  //Disable pads
  for (int x = 0; x < 50; x++)
  {
    if ((x != ap3_gpio_pin2pad(PIN_POWER_LOSS)) &&
        //(x != ap3_gpio_pin2pad(PIN_LOGIC_DEBUG)) &&
        (x != ap3_gpio_pin2pad(PIN_MICROSD_POWER)) &&
        (x != ap3_gpio_pin2pad(PIN_QWIIC_POWER)) &&
        (x != ap3_gpio_pin2pad(PIN_IMU_POWER)))
    {
      am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);
    }
  }

  //Make sure PIN_POWER_LOSS is configured as an input for the WDT
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  //We can't leave these power control pins floating
  imuPowerOff();
  microSDPowerOff();

  //Keep Qwiic bus powered on if user desires it
  if (settings.powerDownQwiicBusBetweenReads == true)
    qwiicPowerOff();
  else
    qwiicPowerOn(); //Make sure pins stays as output

  //Leave the power LED on if the user desires it
  if (settings.enablePwrLedDuringSleep == true)
    powerLEDOn();
  //else
  //  powerLEDOff();

  //Adjust sysTicks down by the amount we've be at 48MHz
  //Read millis _before_ we switch to the lower power clock!
  uint32_t msBeenAwake = millis();
  uint32_t sysTicksAwake = msBeenAwake * 32768L / 1000L; //Convert to 32kHz systicks

  //Power down cache, flash, SRAM
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); // Power down all flash and cache
  am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_384K); // Retain all SRAM

  //Use the lower power 32kHz clock. Use it to run CT6 as well.
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);

  //Check that sysTicksToSleep is >> sysTicksAwake
  if (sysTicksToSleep > (sysTicksAwake + 3277)) // Abort if we are trying to sleep for < 100ms
  {
    sysTicksToSleep -= sysTicksAwake;

    //Setup interrupt to trigger when the number of ms have elapsed
    am_hal_stimer_compare_delta_set(6, sysTicksToSleep);

    //We use counter/timer 6 to cause us to wake up from sleep but 0 to 7 are available
    //CT 7 is used for Software Serial. All CTs are used for Servo.
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREG);  //Clear CT6
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREG); //Enable C/T G=6

    //Enable the timer interrupt in the NVIC.
    NVIC_EnableIRQ(STIMER_CMPR6_IRQn);

    //Deep Sleep
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

    //Turn off interrupt
    NVIC_DisableIRQ(STIMER_CMPR6_IRQn);
    am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREG); //Disable C/T G=6
  }

  //We're BACK!
  wakeFromSleep();
}

//Power everything up gracefully
void wakeFromSleep()
{
  //Go back to using the main clock
  //am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW);
  //NVIC_EnableIRQ(STIMER_IRQn);
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ);

  //Turn on ADC
  uint32_t adcError = (uint32_t)ap3_adc_setup();
  if (settings.logA11 == true) adcError += (uint32_t)ap3_set_pin_to_analog(11); // Set _pad_ 11 to analog if enabled for logging
  if (settings.logA12 == true) adcError += (uint32_t)ap3_set_pin_to_analog(12); // Set _pad_ 12 to analog if enabled for logging
  if (settings.logA13 == true) adcError += (uint32_t)ap3_set_pin_to_analog(13); // Set _pad_ 13 to analog if enabled for logging
  if (settings.logA32 == true) adcError += (uint32_t)ap3_set_pin_to_analog(32); // Set _pad_ 32 to analog if enabled for logging
#if(HARDWARE_VERSION_MAJOR >= 1)
  adcError += (uint32_t)ap3_set_pin_to_analog(PIN_VIN_MONITOR); // Set _pad_ PIN_VIN_MONITOR to analog
#endif

  //Run setup again

  //If 3.3V rail drops below 3V, system will enter low power mode and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  delay(1); // Let PIN_POWER_LOSS stabilize

  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerDown, FALLING);

  if (digitalRead(PIN_POWER_LOSS) == LOW) powerDown(); //Check PIN_POWER_LOSS just in case we missed the falling edge

  if (settings.useGPIO32ForStopLogging == true)
  {
    pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
    delay(1); // Let the pin stabilize
    attachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING), stopLoggingISR, FALLING); // Enable the interrupt
    stopLoggingSeen = false; // Make sure the flag is clear
  }

  if (settings.useGPIO11ForTrigger == true) //(This should be redundant. We should not be going to sleep if triggering is enabled?)
  {
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
    delay(1); // Let the pin stabilize
    if (settings.fallingEdgeTrigger == true)
      attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER), triggerPinISR, FALLING); // Enable the interrupt
    else
      attachInterrupt(digitalPinToInterrupt(PIN_TRIGGER), triggerPinISR, RISING); // Enable the interrupt
    triggerEdgeSeen = false; // Make sure the flag is clear
  }

  if (settings.useGPIO11ForFastSlowLogging == true)
  {
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
  }

  pinMode(PIN_STAT_LED, OUTPUT);
  digitalWrite(PIN_STAT_LED, LOW);

  powerLEDOn();

  Serial.begin(settings.serialTerminalBaudRate);

  if (settings.useTxRxPinsForTerminal == true)
  {
    SerialLog.begin(settings.serialTerminalBaudRate); // Start the serial port
  }

  printDebug(F("wakeFromSleep: I'm awake!\r\n"));
  printDebug("wakeFromSleep: adcError is " + (String)adcError + ".");
  if (adcError > 0)
    printDebug(F(" This indicates an error was returned by ap3_adc_setup or one of the calls to ap3_set_pin_to_analog."));
  printDebug(F("\r\n"));

  beginQwiic(); //Power up Qwiic bus as early as possible

  SPI.begin(); //Needed if SD is disabled

  beginSD(); //285 - 293ms

  enableCIPOpullUp(); // Enable CIPO pull-up after beginSD

  beginDataLogging(); //180ms

  if (settings.useTxRxPinsForTerminal == false)
  {
    beginSerialLogging(); //20 - 99ms
    beginSerialOutput();
  }

  beginIMU(); //61ms
  //printDebug("wakeFromSleep: online.IMU = " + (String)online.IMU + "\r\n");

  //If we powered down the Qwiic bus, then re-begin and re-configure everything
  if (settings.powerDownQwiicBusBetweenReads == true)
  {
    beginQwiicDevices(); // beginQwiicDevices will wait for the qwiic devices to power up
    //loadDeviceSettingsFromFile(); //Apply device settings after the Qwiic bus devices have been detected and begin()'d
    configureQwiicDevices(); //Apply config settings to each device in the node list
  }

  //SerialPrintf2("Wake up time: %.02f ms\r\n", (micros() - startTime) / 1000.0);

  //When we wake up micros has been reset to zero so we need to let the main loop know to take a reading
  takeReading = true;
}

void stopLogging(void)
{
  detachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING)); // Disable the interrupt

  //Save files before going to sleep
  if (online.dataLogging == true)
  {
    sensorDataFile.sync();
    updateDataFileAccess(&sensorDataFile); // Update the file access time & date
    sensorDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
  }
  if (online.serialLogging == true)
  {
    serialDataFile.sync();
    updateDataFileAccess(&serialDataFile); // Update the file access time & date
    serialDataFile.close();
  }

  SerialPrint(F("Logging is stopped. Please reset OpenLog Artemis and open a terminal at "));
  Serial.print((String)settings.serialTerminalBaudRate);
  if (settings.useTxRxPinsForTerminal == true)
      SerialLog.print((String)settings.serialTerminalBaudRate);
  SerialPrintln(F("bps..."));
  delay(sdPowerDownDelay); // Give the SD card time to shut down
  powerDown();
}

void waitForQwiicBusPowerDelay() // Wait while the qwiic devices power up
{
  //Depending on what hardware is configured, the Qwiic bus may have only been turned on a few ms ago
  //Give sensors, specifically those with a low I2C address, time to turn on
  // If we're not using the SD card, everything will have happened much quicker than usual.
  unsigned long qwiicPowerHasBeenOnFor = millis() - qwiicPowerOnTime;
  if (qwiicPowerHasBeenOnFor < qwiicPowerOnDelayMillis)
  {
    unsigned long delayFor = qwiicPowerOnDelayMillis - qwiicPowerHasBeenOnFor;
    for (unsigned long i = 0; i < delayFor; i++)
    {
      checkBattery();
      delay(1);
    }
  }
}

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
#if(HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, LOW);
#else
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#endif

  qwiicPowerOnTime = millis(); //Record this time so we wait enough time before detecting certain sensors
}
void qwiicPowerOff()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
#if(HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#else
  digitalWrite(PIN_QWIIC_POWER, LOW);
#endif
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

void powerLEDOn()
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, HIGH); // Turn the Power LED on
#endif
}
void powerLEDOff()
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, LOW); // Turn the Power LED off
#endif
}

//Returns the number of milliseconds according to the RTC
//(In increments of 10ms)
//Watch out for the year roll-over!
uint64_t rtcMillis()
{
  myRTC.getTime();
  uint64_t millisToday = 0;
  int dayOfYear = calculateDayOfYear(myRTC.dayOfMonth, myRTC.month, myRTC.year + 2000);
  millisToday += ((uint64_t)dayOfYear * 86400000ULL);
  millisToday += ((uint64_t)myRTC.hour * 3600000ULL);
  millisToday += ((uint64_t)myRTC.minute * 60000ULL);
  millisToday += ((uint64_t)myRTC.seconds * 1000ULL);
  millisToday += ((uint64_t)myRTC.hundredths * 10ULL);

  return (millisToday);
}

//Returns the day of year
//https://gist.github.com/jrleeman/3b7c10712112e49d8607
int calculateDayOfYear(int day, int month, int year)
{
  // Given a day, month, and year (4 digit), returns
  // the day of year. Errors return 999.

  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // Verify we got a 4-digit year
  if (year < 1000) {
    return 999;
  }

  // Check if it is a leap year, this is confusing business
  // See: https://support.microsoft.com/en-us/kb/214019
  if (year % 4  == 0) {
    if (year % 100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (year % 400 == 0) {
        daysInMonth[1] = 29;
      }
    }
  }

  // Make sure we are on a valid day of the month
  if (day < 1)
  {
    return 999;
  } else if (day > daysInMonth[month - 1]) {
    return 999;
  }

  int doy = 0;
  for (int i = 0; i < month - 1; i++) {
    doy += daysInMonth[i];
  }

  doy += day;
  return doy;
}

//WatchDog Timer code by Adam Garbo:
//https://forum.sparkfun.com/viewtopic.php?f=169&t=52431&p=213296#p213296

// Watchdog timer configuration structure.
am_hal_wdt_config_t g_sWatchdogConfig = {

  // Configuration values for generated watchdog timer event.
  .ui32Config = AM_HAL_WDT_LFRC_CLK_16HZ | AM_HAL_WDT_ENABLE_RESET | AM_HAL_WDT_ENABLE_INTERRUPT,

  // Number of watchdog timer ticks allowed before a watchdog interrupt event is generated.
  .ui16InterruptCount = 16, // Set WDT interrupt timeout for 1 second.

  // Number of watchdog timer ticks allowed before the watchdog will issue a system reset.
  .ui16ResetCount = 20 // Set WDT reset timeout for 1.25 seconds.
};

void startWatchdog()
{
  // LFRC must be turned on for this example as the watchdog only runs off of the LFRC.
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_LFRC_START, 0);

  // Configure the watchdog.
  am_hal_wdt_init(&g_sWatchdogConfig);

  // Enable the interrupt for the watchdog in the NVIC.
  NVIC_EnableIRQ(WDT_IRQn);
  //NVIC_SetPriority(WDT_IRQn, 0); // Set the interrupt priority to 0 = highest (255 = lowest)
  //am_hal_interrupt_master_enable(); // ap3_initialization.cpp does this - no need to do it here

  // Enable the watchdog.
  am_hal_wdt_start();
}

// Interrupt handler for the watchdog.
extern "C++" void am_watchdog_isr(void)
{
  // Clear the watchdog interrupt.
  am_hal_wdt_int_clear();

  // DON'T Restart the watchdog.
  //am_hal_wdt_restart(); // "Pet" the dog.
}
