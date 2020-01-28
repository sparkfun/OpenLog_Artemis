
//Power down the entire system but maintain running of RTC
//This function takes 100us to run including GPIO setting
//This puts the Apollo3 into 2.36uA to 2.6uA consumption mode
void powerDown()
{
  //int startTicks = micros(); //sysTicks();
  digitalWrite(PIN_LOGIC_DEBUG, HIGH);

  //  digitalWrite(PIN_QWIIC_PWR, LOW);
  //  digitalWrite(PIN_ICM_PWR, LOW);
  //  digitalWrite(PIN_MICROSD_PWR, LOW);

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
  digitalWrite(PIN_LOGIC_DEBUG, LOW);

  am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
}

void sdPowerOn()
{
  digitalWrite(PIN_MICROSD_POWER, HIGH);
}
void sdPowerOff()
{
  digitalWrite(PIN_MICROSD_POWER, LOW);
}

//Option not known
void printUnknown(uint8_t unknownChoice)
{
  Serial.print("Unknown choice: ");
  Serial.write(unknownChoice);
  Serial.println();
}
void printUnknown(int unknownValue)
{
  Serial.print("Unknown value: ");
  Serial.write(unknownValue);
  Serial.println();
}

//Blocking wait for user input
void waitForInput()
{
  delay(10); //Wait for any incoming chars to hit buffer
  while (Serial.available() > 0) Serial.read(); //Clear buffer
  while (Serial.available() == 0);
}

//Get single byte from user
//Waits for and returns the character that the user provides
//Returns 255 if user does not respond in a certain amount time
byte getByteChoice(int numberOfSeconds)
{
  Serial.flush();
  delay(50); //Wait for any incoming chars to hit buffer
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  long startTime = millis();
  byte incoming;
  while (1)
  {
    if (Serial.available() > 0)
    {
      incoming = Serial.read();
      Serial.print("byte: 0x");
      Serial.println(incoming, HEX);
      if (incoming >= 'a' && incoming <= 'z') break;
      if (incoming >= 'A' && incoming <= 'Z') break;
      if (incoming >= '0' && incoming <= '9') break;
    }

    if ( (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      Serial.println("No user input recieved.");
      return (255); //Timeout. No user input.
    }

    delay(10);
  }

  return (incoming);
}

//Get a string/value from user, remove all non-numeric values
//Returns 255 is user presses 'x' or input times out
int getNumber(int numberOfSeconds)
{
  delay(10); //Wait for any incoming chars to hit buffer
  while (Serial.available() > 0) Serial.read(); //Clear buffer

  //Get input from user
  char cleansed[20];

  long startTime = millis();
  int spot = 0;
  while (spot < 20 - 1) //Leave room for terminating \0
  {
    while (Serial.available() == 0) //Wait for user input
    {
      if ( (millis() - startTime) / 1000 >= numberOfSeconds)
      {
        if (spot == 0)
        {
          Serial.println("No user input recieved. Do you have line endings turned on?");
          return (255); //Timeout. No user input.
        }
        else if (spot > 0)
        {
          break; //Timeout, but we have data
        }
      }
    }

    //See if we timed out waiting for a line ending
    if (spot > 0 && (millis() - startTime) / 1000 >= numberOfSeconds)
    {
      Serial.println("Do you have line endings turned on?");
      break; //Timeout, but we have data
    }

    byte incoming = Serial.read();
    if (incoming == '\n' || incoming == '\r')
    {
      Serial.println();
      break;
    }

    if (isDigit(incoming) == true)
    {
      Serial.write(incoming); //Echo user's typing
      cleansed[spot++] = (char)incoming;
    }

    if (incoming == 'x')
    {
      return (255);
    }
  }

  cleansed[spot] = '\0';
  Serial.print("cleansed: ");
  Serial.println(cleansed);

  String tempValue = cleansed;
  Serial.print("tempValue: ");
  Serial.println(tempValue);
  return (tempValue.toInt());
}
