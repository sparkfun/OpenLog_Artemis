// Read the battery voltage
// If it is low, increment lowBatteryReadings
// If lowBatteryReadings exceeds lowBatteryReadingsLimit then powerDownOLA
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
        // Gracefully powerDownOLA

        //Save files before powerDownOLA
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

#ifdef noPowerLossProtection
        SerialPrintln(F("*** LOW BATTERY VOLTAGE DETECTED! RESETTING... ***"));
        SerialPrintln(F("*** PLEASE CHANGE THE POWER SOURCE TO CONTINUE ***"));
      
        SerialFlush(); //Finish any prints

        resetArtemis(); // Reset the Artemis so we don't get stuck in a low voltage infinite loop
#else
        SerialPrintln(F("***      LOW BATTERY VOLTAGE DETECTED! GOING INTO POWERDOWN      ***"));
        SerialPrintln(F("*** PLEASE CHANGE THE POWER SOURCE AND RESET THE OLA TO CONTINUE ***"));
      
        SerialFlush(); //Finish any prints

        powerDownOLA(); // Power down and wait for reset
#endif
      }
    }
    else
    {
      lowBatteryReadings = 0; // Reset the low battery count (to reject noise)
    }    
  }
#endif

#ifndef noPowerLossProtection // Redundant - since the interrupt is not attached if noPowerLossProtection is defined... But you never know...
  if (powerLossSeen)
    powerDownOLA(); // power down and wait for reset
#endif
}

//Power down the entire system but maintain running of RTC
//This function takes 100us to run including GPIO setting
//This puts the Apollo3 into 2.36uA to 2.6uA consumption mode
//With leakage across the 3.3V protection diode, it's approx 3.00uA.
void powerDownOLA(void)
{
#ifndef noPowerLossProtection // Probably redundant - included just in case detachInterrupt causes badness when it has not been attached
  //Prevent voltage supervisor from waking us from sleep
  detachInterrupt(PIN_POWER_LOSS);
#endif

  //Prevent stop logging button from waking us from sleep
  if (settings.useGPIO32ForStopLogging == true)
  {
    detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt
    pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
  }

  //Prevent trigger from waking us from sleep
  if (settings.useGPIO11ForTrigger == true)
  {
    detachInterrupt(PIN_TRIGGER); // Disable the interrupt
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

  powerControlADC(false); // power_adc_disable(); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART
  if ((settings.useTxRxPinsForTerminal == true) || (online.serialLogging == true))
    Serial1.end();

  //Force the peripherals off
  //This will cause badness with v2.1 of the core but we don't care as we are waiting for a reset
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM1);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM2);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM3);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM4);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM5);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_ADC);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART0);
  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART1);

  //Disable pads (this disables the LEDs too)
  for (int x = 0; x < 50; x++)
  {
    if ((x != PIN_POWER_LOSS) &&
        //(x != PIN_LOGIC_DEBUG) &&
        (x != PIN_MICROSD_POWER) &&
        (x != PIN_QWIIC_POWER) &&
        (x != PIN_IMU_POWER))
    {
      am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);
    }
  }

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

#ifdef noPowerLossProtection // If noPowerLossProtection is defined, then the WDT will already be running
  stopWatchdog();
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

  powerControlADC(false); // power_adc_disable(); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART
  if ((settings.useTxRxPinsForTerminal == true) || (online.serialLogging == true))
    Serial1.end();

  //Force the peripherals off
  //This will cause badness with v2.1 of the core but we don't care as we are going to force a WDT reset
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
    if ((x != PIN_POWER_LOSS) &&
        //(x != PIN_LOGIC_DEBUG) &&
        (x != PIN_MICROSD_POWER) &&
        (x != PIN_QWIIC_POWER) &&
        (x != PIN_IMU_POWER))
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
  petTheDog = false; // Make sure the WDT will not restart
#ifndef noPowerLossProtection // If noPowerLossProtection is defined, then the WDT will already be running
  startWatchdog(); // Start the WDT to generate a reset
#endif
  while (1) // That's all folks! Artemis will reset in 1.25 seconds
    ;
}

//Power everything down and wait for interrupt wakeup
void goToSleep(uint32_t sysTicksToSleep)
{
  printDebug("goToSleep: sysTicksToSleep = " + (String)sysTicksToSleep + "\r\n");
  
#ifndef noPowerLossProtection // Probably redundant - included just in case detachInterrupt causes badness when it has not been attached
  //Prevent voltage supervisor from waking us from sleep
  detachInterrupt(PIN_POWER_LOSS);
#endif

  //Prevent stop logging button from waking us from sleep
  if (settings.useGPIO32ForStopLogging == true)
  {
    detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt
    pinMode(PIN_STOP_LOGGING, INPUT); // Remove the pull-up
  }

  //Prevent trigger from waking us from sleep
  //(This should be redundant. We should not be going to sleep if triggering is enabled?)
  if (settings.useGPIO11ForTrigger == true)
  {
    detachInterrupt(PIN_TRIGGER); // Disable the interrupt
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

  //qwiic.end(); //DO NOT Power down I2C - causes badness with v2.1 of the core: https://github.com/sparkfun/Arduino_Apollo3/issues/412

  SPI.end(); //Power down SPI

  powerControlADC(false); //Power down ADC

  //Adjust sysTicks down by the amount we've be at 48MHz
  //Read millis _before_ we switch to the lower power clock!
  uint64_t msBeenAwake = rtcMillis() - lastAwakeTimeMillis;
  uint64_t sysTicksAwake = msBeenAwake * 32768L / 1000L; //Convert to 32kHz systicks
  if (sysTicksAwake < sysTicksToSleep)
    sysTicksToSleep -= sysTicksAwake;
  else
    sysTicksToSleep = 0;
  printDebug("goToSleep: sysTicksToSleep (adjusted) = " + (String)sysTicksToSleep + "\r\n\r\n");
  
  SerialFlush(); //Finish any prints
  Serial.end();
  if ((settings.useTxRxPinsForTerminal == true) || (online.serialLogging == true))
    Serial1.end();

  //Force the peripherals off

  //With v2.1 of the core, very bad things happen if the IOMs are disabled.
  //We must leave them enabled: https://github.com/sparkfun/Arduino_Apollo3/issues/412
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM0); // SPI
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM1); // qwiic I2C
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM2);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM3);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM4);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_IOM5);
  //am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART0);
  //if (settings.useTxRxPinsForTerminal == true)
  //  am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_UART1);

  //Disable as many pins as we can
  const int pinsToDisable[] = {0,1,2,10,14,17,12,24,25,28,36,38,39,40,41,42,43,45,21,22,16,31,35,-1};
  for (int x = 0; pinsToDisable[x] >= 0; x++)
  {
    am_hal_gpio_pinconfig(pinsToDisable[x], g_AM_HAL_GPIO_DISABLE);
  }

  //Do disable CIPO, COPI, SCLK and chip selects to minimise the current draw during deep sleep
  am_hal_gpio_pinconfig(PIN_SPI_CIPO , g_AM_HAL_GPIO_DISABLE); //ICM / microSD CIPO
  am_hal_gpio_pinconfig(PIN_SPI_COPI , g_AM_HAL_GPIO_DISABLE); //ICM / microSD COPI
  am_hal_gpio_pinconfig(PIN_SPI_SCK , g_AM_HAL_GPIO_DISABLE); //ICM / microSD SCK
  am_hal_gpio_pinconfig(PIN_IMU_CHIP_SELECT , g_AM_HAL_GPIO_DISABLE); //ICM CS
  am_hal_gpio_pinconfig(PIN_IMU_INT , g_AM_HAL_GPIO_DISABLE); //ICM INT
  am_hal_gpio_pinconfig(PIN_MICROSD_CHIP_SELECT , g_AM_HAL_GPIO_DISABLE); //microSD CS

  //If requested, disable pins 48 and 49 (UART0) to stop them back-feeding the CH340
  if (settings.serialTxRxDuringSleep == false)
  {
    am_hal_gpio_pinconfig(48 , g_AM_HAL_GPIO_DISABLE); //TX0
    am_hal_gpio_pinconfig(49 , g_AM_HAL_GPIO_DISABLE); //RX0
    if (settings.useTxRxPinsForTerminal == true)
    {
      am_hal_gpio_pinconfig(12 , g_AM_HAL_GPIO_DISABLE); //TX1
      am_hal_gpio_pinconfig(13 , g_AM_HAL_GPIO_DISABLE); //RX1
    }
  }

  //Make sure PIN_POWER_LOSS is configured as an input for the WDT
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  //We can't leave these power control pins floating
  imuPowerOff();
  microSDPowerOff();

  //Keep Qwiic bus powered on if user desires it
  if (settings.powerDownQwiicBusBetweenReads == true)
  {
    //Do disable qwiic SDA and SCL to minimise the current draw during deep sleep
    am_hal_gpio_pinconfig(PIN_QWIIC_SDA , g_AM_HAL_GPIO_DISABLE);
    am_hal_gpio_pinconfig(PIN_QWIIC_SCL , g_AM_HAL_GPIO_DISABLE);
    qwiicPowerOff();
  }
  else
    qwiicPowerOn(); //Make sure pins stays as output

  //Leave the power LED on if the user desires it
  if (settings.enablePwrLedDuringSleep == true)
    powerLEDOn();
  else
    powerLEDOff();


#ifdef noPowerLossProtection
  // If noPowerLossProtection is defined, then the WDT will be running
  // We need to stop it otherwise it will wake the Artemis
  stopWatchdog();
#endif

  //Power down cache, flash, SRAM
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); // Power down all flash and cache
  am_hal_pwrctrl_memory_deepsleep_retain(AM_HAL_PWRCTRL_MEM_SRAM_384K); // Retain all SRAM

  //Use the lower power 32kHz clock. Use it to run CT6 as well.
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);

  //Check that sysTicksToSleep is >> sysTicksAwake
  if (sysTicksToSleep > 3277) // Abort if we are trying to sleep for < 100ms
  {
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

  // Power up SRAM, turn on entire Flash
  am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_MAX);

  // Update lastAwakeTimeMillis
  lastAwakeTimeMillis = rtcMillis();

  //Turn on ADC
  powerControlADC(true);

  //Re-enable analog inputs
  //if (settings.logA11 == true) adcError += (uint32_t)ap3_set_pin_to_analog(11); // Set _pad_ 11 to analog if enabled for logging
  //if (settings.logA12 == true) adcError += (uint32_t)ap3_set_pin_to_analog(12); // Set _pad_ 12 to analog if enabled for logging
  //if (settings.logA13 == true) adcError += (uint32_t)ap3_set_pin_to_analog(13); // Set _pad_ 13 to analog if enabled for logging
  //if (settings.logA32 == true) adcError += (uint32_t)ap3_set_pin_to_analog(32); // Set _pad_ 32 to analog if enabled for logging
#if(HARDWARE_VERSION_MAJOR >= 1)
  //adcError += (uint32_t)ap3_set_pin_to_analog(PIN_VIN_MONITOR); // Set _pad_ PIN_VIN_MONITOR to analog
#endif

  //Run setup again

  //If 3.3V rail drops below 3V, system will enter low power mode and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up
  pin_config(PinName(PIN_POWER_LOSS), g_AM_HAL_GPIO_INPUT); // Make sure the pin does actually get re-configured

  delay(1); // Let PIN_POWER_LOSS stabilize

#ifndef noPowerLossProtection
  if (digitalRead(PIN_POWER_LOSS) == LOW) powerDownOLA(); //Check PIN_POWER_LOSS just in case we missed the falling edge
  //attachInterrupt(PIN_POWER_LOSS, powerDownOLA, FALLING); // We can't do this with v2.1.0 as attachInterrupt causes a spontaneous interrupt
  attachInterrupt(PIN_POWER_LOSS, powerLossISR, FALLING);
#else
  // No Power Loss Protection
  // Set up the WDT to generate a reset just in case the code crashes during a brown-out
  startWatchdog();
#endif
  powerLossSeen = false; // Make sure the flag is clear

  if (settings.useGPIO32ForStopLogging == true)
  {
    pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
    pin_config(PinName(PIN_STOP_LOGGING), g_AM_HAL_GPIO_INPUT_PULLUP); // Make sure the pin does actually get re-configured
    delay(1); // Let the pin stabilize
    attachInterrupt(PIN_STOP_LOGGING, stopLoggingISR, FALLING); // Enable the interrupt
    am_hal_gpio_pincfg_t intPinConfig = g_AM_HAL_GPIO_INPUT_PULLUP;
    intPinConfig.eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO;
    pin_config(PinName(PIN_STOP_LOGGING), intPinConfig); // Make sure the pull-up does actually stay enabled
    stopLoggingSeen = false; // Make sure the flag is clear
  }

  if (settings.useGPIO11ForTrigger == true) //(This should be redundant. We should not be going to sleep if triggering is enabled?)
  {
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
    pin_config(PinName(PIN_TRIGGER), g_AM_HAL_GPIO_INPUT_PULLUP); // Make sure the pin does actually get re-configured
    delay(1); // Let the pin stabilize
    am_hal_gpio_pincfg_t intPinConfig = g_AM_HAL_GPIO_INPUT_PULLUP;
    if (settings.fallingEdgeTrigger == true)
    {
      attachInterrupt(PIN_TRIGGER, triggerPinISR, FALLING); // Enable the interrupt
      intPinConfig.eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO;
    }
    else
    {
      attachInterrupt(PIN_TRIGGER, triggerPinISR, RISING); // Enable the interrupt
      intPinConfig.eIntDir = AM_HAL_GPIO_PIN_INTDIR_LO2HI;
    }
    pin_config(PinName(PIN_TRIGGER), intPinConfig); // Make sure the pull-up does actually stay enabled
    triggerEdgeSeen = false; // Make sure the flag is clear
  }

  if (settings.useGPIO11ForFastSlowLogging == true)
  {
    pinMode(PIN_TRIGGER, INPUT_PULLUP);
    pin_config(PinName(PIN_TRIGGER), g_AM_HAL_GPIO_INPUT_PULLUP); // Make sure the pin does actually get re-configured
  }

  pinMode(PIN_STAT_LED, OUTPUT);
  pin_config(PinName(PIN_STAT_LED), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_STAT_LED, LOW);

  powerLEDOn();

  //Re-enable pins 48 and 49 (UART0) if requested
  if (settings.serialTxRxDuringSleep == false)
  {
    pin_config(PinName(48), g_AM_BSP_GPIO_COM_UART_TX);    
    pin_config(PinName(49), g_AM_BSP_GPIO_COM_UART_RX);
    if (settings.useTxRxPinsForTerminal == true)
    {
      configureSerial1TxRx();
    }
  }

  //Re-enable CIPO, COPI, SCK and the chip selects but may as well leave ICM_INT disabled
  pin_config(PinName(PIN_SPI_CIPO), g_AM_BSP_GPIO_IOM0_MISO);
  pin_config(PinName(PIN_SPI_COPI), g_AM_BSP_GPIO_IOM0_MOSI);
  pin_config(PinName(PIN_SPI_SCK), g_AM_BSP_GPIO_IOM0_SCK);
  pin_config(PinName(PIN_IMU_CHIP_SELECT), g_AM_HAL_GPIO_OUTPUT);
  pin_config(PinName(PIN_MICROSD_CHIP_SELECT) , g_AM_HAL_GPIO_OUTPUT);

  //Re-enable the SDA and SCL pins
  pin_config(PinName(PIN_QWIIC_SCL), g_AM_BSP_GPIO_IOM1_SCL);
  pin_config(PinName(PIN_QWIIC_SDA), g_AM_BSP_GPIO_IOM1_SDA);

  Serial.begin(settings.serialTerminalBaudRate);

  if (settings.useTxRxPinsForTerminal == true)
  {
    Serial1.begin(settings.serialTerminalBaudRate); // Start the serial port
  }

  printDebug(F("wakeFromSleep: I'm awake!\r\n")); SerialFlush();
  
  beginQwiic(); //Power up Qwiic bus as early as possible

  SPI.begin(); //Needed if SD is disabled

  beginSD(true); //285 - 293ms

  enableCIPOpullUp(); // Enable CIPO pull-up _after_ beginSD

  beginDataLogging(); //180ms

  if (settings.useTxRxPinsForTerminal == false)
  {
    beginSerialLogging(); //20 - 99ms
    beginSerialOutput();
  }

  beginIMU(true); //61ms
  printDebug("wakeFromSleep: online.IMU = " + (String)online.IMU + "\r\n");

  //If we powered down the Qwiic bus, then re-begin and re-configure everything
  if (settings.powerDownQwiicBusBetweenReads == true)
  {
    beginQwiicDevices(); // beginQwiicDevices will wait for the qwiic devices to power up
    //loadDeviceSettingsFromFile(); //Apply device settings after the Qwiic bus devices have been detected and begin()'d
    configureQwiicDevices(); //Apply config settings to each device in the node list
  }
  
  // Late in the process to allow time for external device to generate unwanted signals
  while(Serial.available())  // Flush the input buffer
    Serial.read();
  if (settings.useTxRxPinsForTerminal == true)
  {
    while(Serial1.available())  // Flush the input buffer
      Serial1.read();
  }
  
  //When we wake up micros has been reset to zero so we need to let the main loop know to take a reading
  takeReading = true;
}

void stopLogging(void)
{
  detachInterrupt(PIN_STOP_LOGGING); // Disable the interrupt

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
      Serial1.print((String)settings.serialTerminalBaudRate);
  SerialPrintln(F("bps..."));
  delay(sdPowerDownDelay); // Give the SD card time to shut down
  powerDownOLA();
}

void waitForQwiicBusPowerDelay() // Wait while the qwiic devices power up
{
  //Depending on what hardware is configured, the Qwiic bus may have only been turned on a few ms ago
  //Give sensors, specifically those with a low I2C address, time to turn on
  // If we're not using the SD card, everything will have happened much quicker than usual.
  uint64_t qwiicPowerHasBeenOnFor = rtcMillis() - qwiicPowerOnTime;
  printDebug("waitForQwiicBusPowerDelay: qwiicPowerHasBeenOnFor " + (String)((unsigned long)qwiicPowerHasBeenOnFor) + "ms\r\n");
  if (qwiicPowerHasBeenOnFor < (uint64_t)qwiicPowerOnDelayMillis)
  {
    uint64_t delayFor = (uint64_t)qwiicPowerOnDelayMillis - qwiicPowerHasBeenOnFor;
    printDebug("waitForQwiicBusPowerDelay: delaying for " + (String)((unsigned long)delayFor) + "\r\n");
    for (uint64_t i = 0; i < delayFor; i++)
    {
      checkBattery();
      delay(1);
    }
  }
}

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pin_config(PinName(PIN_QWIIC_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
#if(HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, LOW);
#else
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#endif

  qwiicPowerOnTime = rtcMillis(); //Record this time so we wait enough time before detecting certain sensors
}
void qwiicPowerOff()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  pin_config(PinName(PIN_QWIIC_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
#if(HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#else
  digitalWrite(PIN_QWIIC_POWER, LOW);
#endif
}

void microSDPowerOn()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pin_config(PinName(PIN_MICROSD_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_MICROSD_POWER, LOW);
}
void microSDPowerOff()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pin_config(PinName(PIN_MICROSD_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_MICROSD_POWER, HIGH);
}

void imuPowerOn()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  pin_config(PinName(PIN_IMU_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_IMU_POWER, HIGH);
}
void imuPowerOff()
{
  pinMode(PIN_IMU_POWER, OUTPUT);
  pin_config(PinName(PIN_IMU_POWER), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_IMU_POWER, LOW);
}

void powerLEDOn()
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  pinMode(PIN_PWR_LED, OUTPUT);
  pin_config(PinName(PIN_PWR_LED), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
  digitalWrite(PIN_PWR_LED, HIGH); // Turn the Power LED on
#endif
}
void powerLEDOff()
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  pinMode(PIN_PWR_LED, OUTPUT);
  pin_config(PinName(PIN_PWR_LED), g_AM_HAL_GPIO_OUTPUT); // Make sure the pin does actually get re-configured
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
