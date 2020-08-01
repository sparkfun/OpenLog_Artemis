/*
  OpenLog Artemis: Production Test Code

  To begin, the OLA checks to see if all four breakout pins are being held low (at reset).
  If they are, the OLA waits for up to five seconds for them to be released.
  If they are not released, then the OLA assumes they are being used for analog input and continues with the normal OLA code.
  If the pins are released, the OLA becomes an I2C peripheral and waits for command bytes over the Qwiic interface.
  The command bytes instruct the OLA to test a thing and report success/fail.
  In most cases, the OLA responds by echoing the command byte, followed by 0xFF for success or 0x00 for fail.
  Special cases are:
    0x17: Read RTC Time: The OLA will respond with 0x17 followed by the RTC time in HH:MM:SS.SS format (ASCII text)
    0x55: Deep Sleep: The OLA goes immediately into deep sleep and does not respond
  Implememted command bytes are:
    0x01: VIN/3 Divider: The OLA reads VIN/3 and checks that it is 5V +/- 0.2V
    0x02: IMU Temperature: The OLA powers up the IMU and reads its temperature. Success: 20C +/- 10C
    0x03: IMU Accelerometer: The OLA powers up the IMU and reads the accel axes. Success: 1g +/- 0.1g on Z; 0g /- 0.1g on X and Y
    0x04: IMU Magnetometer: The OLA powers up the IMU and reads the mag axes. Success: 25nT +/- 5nT on Z; vector product of X and Y is 25nT +/- 5nT
    0x05: RTC Crystal: The OLA goes into deep sleep for 5 seconds and checks that the RTC has incremented. Success: RTC increments by 4-6 seconds
    0x06: Qwiic Power On: The OLA enables Qwiic power. Success: always
    0x07: Qwiic Power Off: The OLA disables Qwiic power. Success: always
    0x08: PWR LED On: The OLA turns on the PWR LED. Success: always
    0x09: PWR LED Off: The OLA turns off the PWR LED. Success: always
    0x0A: STAT LED On: The OLA turns on the STAT LED. Success: always
    0x0B: STAT LED Off: The OLA turns off the STAT LED. Success: always
    0x0C: Pin 32 High: The OLA pulls breakout pin 32 high. Success: always
    0x0D: Pin 32 High: The OLA pulls breakout pin 32 high. Success: always
    0x0E: Pin TX Low: The OLA pulls breakout pin TX low. Success: always
    0x0F: Pin TX High: The OLA pulls breakout pin TX high. Success: always
    0x10: Pin RX Low: The OLA pulls breakout pin RX low. Success: always
    0x11: Pin RX High: The OLA pulls breakout pin RX high. Success: always
    0x12: Pin 11 Low: The OLA pulls breakout pin 11 low. Success: always
    0x13: Pin 11 Low: The OLA pulls breakout pin 11 low. Success: always
    0x14: USB TX Test Start: The OLA sends "HelloHelloHello" (repeating continuously) on USB as 115200 serial. Success: always
    0x15: USB TX Test Stop: The OLA stops sending. Success: always
    0x16: USB Echo Test: The OLA will echo back on USB TX whatever it receives on USB RX. Success: always
    0x17: Read RTC Time: Special case - see above
    0x18: microSD Card Test: The OLA writes to a file on microSD and then reads the contents back. Success: file written successfully
    0x55: Deep Sleep: Special case - see above
*/

#define PROD_TEST_I2C_ADDR 0x42 // The OLA will emulate an I2C peripheral with this address during production test

static uint8_t lastI2CCommandByte = 0x00; // Store the last I2C command byte
RingBufferN<16> responseBuffer; // Buffer to hold the I2C responses
uint8_t prodTestI2cBuffer[16]; // Array for qwiic.write

void productionTest()
{
  pinMode(BREAKOUT_PIN_32, INPUT_PULLUP); // Make pin 32 an input with pull-up
  pinMode(BREAKOUT_PIN_TX, INPUT_PULLUP); // Make pin TX an input with pull-up
  pinMode(BREAKOUT_PIN_RX, INPUT_PULLUP); // Make pin RX an input with pull-up
  pinMode(BREAKOUT_PIN_11, INPUT_PULLUP); // Make pin 11 an input with pull-up

  delay(10); // Wait for the pins to settle
  
  int all_low = LOW; // Flag to indicate the combined (OR) status of the four breakout pins
  all_low |= digitalRead(BREAKOUT_PIN_32); // Read all four pins. If any one is high all_low will be high
  all_low |= digitalRead(BREAKOUT_PIN_TX);
  all_low |= digitalRead(BREAKOUT_PIN_RX);
  all_low |= digitalRead(BREAKOUT_PIN_11);

  if (all_low == HIGH)
  {
    // One or more pins are high so exit now
    pinMode(BREAKOUT_PIN_32, INPUT); // Remove the pull-ups
    pinMode(BREAKOUT_PIN_TX, INPUT);
    pinMode(BREAKOUT_PIN_RX, INPUT);
    pinMode(BREAKOUT_PIN_11, INPUT);
    return;
  }

  unsigned long start_time = millis(); // Record what time we started waiting
  int all_high = LOW; // Flag to indicate the combined (AND) status of the four breakout pins
  while (((millis() - start_time) < 5000) && (all_high == LOW)) // Wait for up to five seconds for all the pins to be released
  {
    all_high = HIGH;
    all_low &= digitalRead(BREAKOUT_PIN_32); // Read all four pins. If any one is low all_high will be low
    all_low &= digitalRead(BREAKOUT_PIN_TX);
    all_low &= digitalRead(BREAKOUT_PIN_RX);
    all_low &= digitalRead(BREAKOUT_PIN_11);
  }

  // Either the five second timeout expired or all four pins are high
  if (all_high == LOW) // If any one pin is still low - timeout must have expired so exit now
  {
    pinMode(BREAKOUT_PIN_32, INPUT); // Remove the pull-ups
    pinMode(BREAKOUT_PIN_TX, INPUT);
    pinMode(BREAKOUT_PIN_RX, INPUT);
    pinMode(BREAKOUT_PIN_11, INPUT);
    return;
  }

  // OK. The breakout pins were held low and then released within five seconds so let's go into production test mode!

  //detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS)); // Disable power loss interrupt
  digitalWrite(PIN_STAT_LED, LOW); // Turn the STAT LED off
  powerLEDOff(); // Turn the power LED on - if the hardware supports it

  Serial.println(F("OLA Production Test initiated!"));
  Serial.println(F("Waiting for command byte(s) on the Qwiic bus..."));

  qwiic.begin(PROD_TEST_I2C_ADDR); // Begin the I2C interface using the defined I2C address
  //qwiic.setPullups(settings.qwiicBusPullUps); //Just to make it really clear what pull-ups are being used, set pullups here.

  qwiic.onReceive(olaProdTestReceiveEvent); // Set up the I2C interrupt handlers
  qwiic.onRequest(olaProdTestRequestEvent);

  while (1) // Do this loop forever!
  {
    while (lastI2CCommandByte == 0x00) // Wait until we receive a command byte
    {
      ;
    }

    // Command byte received! Let's process it.
    Serial.printf("Processing command byte: 0x%02X\n", lastI2CCommandByte);
    
    switch (lastI2CCommandByte)
    {
      case 0x01: // VIN/3 Divider
      {
        float vin = readVIN(); // Read VIN
        if ((vin >= 4.8) && (vin <= 5.2)) // Success
        {
          responseBuffer.store_char(0x01);
          responseBuffer.store_char(0xFF);
        }
        else
        {
          responseBuffer.store_char(0x01);
          responseBuffer.store_char(0x00);          
        }
      } // / 0x01: VIN/3
        break;
      case 0x02:
        break;
      case 0x03:
        break;
      case 0x04:
        break;
      case 0x05:
        break;
      case 0x06:
        break;
      case 0x07:
        break;
      case 0x08:
        break;
      case 0x09:
        break;
      case 0x0A:
        digitalWrite(PIN_STAT_LED, HIGH);
        responseBuffer.store_char(0x0A);
        responseBuffer.store_char(0xFF);
        break;
      case 0x0B:
        digitalWrite(PIN_STAT_LED, LOW);
        responseBuffer.store_char(0x0A);
        responseBuffer.store_char(0xFF);
        break;
      case 0x0C:
        break;
      case 0x0D:
        break;
      case 0x0E:
        break;
      case 0x0F:
        break;
      case 0x10:
        break;
      case 0x11:
        break;
      case 0x12:
        break;
      case 0x13:
        break;
      case 0x14:
        break;
      case 0x15:
        break;
      case 0x16:
        break;
      case 0x17:
        break;
      case 0x55: // Deep sleep
      {
        detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS)); // Disable power loss interrupt
        Serial.end(); //Power down UART
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
        qwiicPowerOff();
        //Power down Flash, SRAM, cache
        am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_CACHE);         //Turn off CACHE
        am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_FLASH_512K);    //Turn off everything but lower 512k
        am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_SRAM_64K_DTCM); //Turn off everything but lower 64k
        //am_hal_pwrctrl_memory_deepsleep_powerdown(AM_HAL_PWRCTRL_MEM_ALL); //Turn off all memory (doesn't recover)
        //Keep the 32kHz clock running for RTC
        am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
        am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ);
        while (1) // Stay in deep sleep until we get reset
        {
          am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP); //Sleep
        }
      } // / 0x18: Deep sleep
        break;
      default:
        Serial.printf("Unrecognised command byte: 0x%02X\n", lastI2CCommandByte);
        break;
    }

    lastI2CCommandByte = 0x00; // Reset the I2C command byte
  }
}

// I2C interrupt handlers

void olaProdTestReceiveEvent(int numberOfBytesReceived)
{
  if (numberOfBytesReceived > 0) // Check that we received some data (!) (hopefully redundant!)
  {
    uint8_t incoming = qwiic.read(); // Read the command byte
    lastI2CCommandByte = incoming; // Store it
    if (numberOfBytesReceived > 1) // Did we receive any unexpected extra data?
    {
      for (int i = 1; i < numberOfBytesReceived; i++) // If we did, mop up the extra bytes - hopefully redundant?!
      {
        qwiic.read();
      }
    }
  }
}

void olaProdTestRequestEvent()
{
  int bytesAvailable = responseBuffer.available(); // Check how many bytes are waiting in the response buffer
  if (bytesAvailable > 0) // If there are any bytes waiting in the buffer then send them
  {
    for (int i = 0; i < bytesAvailable; i++) // Read the bytes and put them into prodTestI2cBuffer
    {
      prodTestI2cBuffer[i] = responseBuffer.read_char();
    }
    qwiic.write(prodTestI2cBuffer, bytesAvailable); // Send them
  }
}
