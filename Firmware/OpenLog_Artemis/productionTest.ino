/*
  OpenLog Artemis: Production Test Code

  To begin, the OLA checks to see if all four breakout pins are being held low (at reset).
  If they are, the OLA waits for up to five seconds for all four to be released.
  If they are not released, then the OLA assumes they are being used for analog input and continues with the normal OLA code.
  If the pins are released, the OLA waits for serial command bytes via the RX breakout pin (115200 baud).
  The command bytes instruct the OLA to test a thing and report success/fail.
  The OLA responds by: echoing the command byte if the test was successful; echoing the command byte with the most significant bit set if the test failed.
  E.g.: if the Flying Jalapeno sends 0x01: the OLA will return 0x01 if VIN is in range; or 0x81 if VIN is out of range.
  Implememted command bytes are:
    0x01: VIN/3 Divider: The OLA reads VIN/3 and checks that it is 5V +/- 0.25V
    0x02: IMU Temperature: The OLA powers up the IMU and reads its temperature. Success: 25C +/- 15C
    0x03: IMU Accelerometer: The OLA powers up the IMU and reads the accel axes. Success: -1g +/- 0.1g on Z; 0g +/- 0.1g on X and Y
    0x04: IMU Magnetometer: The OLA powers up the IMU and reads the mag axes. Success: 25nT +/- 5nT on Z; vector product of X and Y is 25nT +/- 5nT
    0x05: RTC Crystal: The OLA goes into deep sleep for 5 seconds and checks that the RTC has incremented. Success: RTC increments by 4.9-5.1 seconds
    0x06: Qwiic Power On: The OLA enables Qwiic power. Success: always
    0x07: Qwiic Power Off: The OLA disables Qwiic power. Success: always
    0x08: PWR LED On: The OLA turns on the PWR LED. Success: always
    0x09: PWR LED Off: The OLA turns off the PWR LED. Success: always
    0x0A: STAT LED On: The OLA turns on the STAT LED. Success: always
    0x0B: STAT LED Off: The OLA turns off the STAT LED. Success: always
    0x0C: Pin 32 High: The OLA pulls breakout pin 32 high. Success: always
    0x0D: Pin 32 Low: The OLA pulls breakout pin 32 low. Success: always
    0x0E: Qwiic SCL High: The OLA pulls the Qwiic bus SCL pin high. Success: always
    0x0F: Qwiic SCL Low: The OLA pulls the Qwiic bus SCL pin low. Success: always
    0x10: Qwiic SDA High: The OLA pulls the Qwiic bus SDA pin high. Success: always
    0x11: Qwiic SDA Low: The OLA pulls the Qwiic bus SDA pin low. Success: always
    0x12: Pin 11 High: The OLA pulls breakout pin 11 high. Success: always
    0x13: Pin 11 Low: The OLA pulls breakout pin 11 low. Success: always
    0x14: USB TX Test Start: The OLA sends "HelloHelloHello" (repeating continuously) on USB as 115200 serial. Success: always
    0x15: USB TX Test Stop: The OLA stops sending. Success: always
    0x16: USB Echo Test Start: The OLA will echo back on USB TX whatever it receives on USB RX. Success: always
    0x17: USB Echo Test Stop: The OLA stops echoing on USB. Success: always
    0x18: Read RTC Time: Special case - see below
    0x19: microSD Card Test: The OLA writes to a file on microSD and then reads the contents back. Success: file written successfully
    0x55: Deep Sleep: Special case - see below
  Special cases are:
    0x18: Read RTC Time: The OLA will respond with 0x18 followed by the RTC time in HH:MM:SS.SS format (ASCII text)
    0x55: Deep Sleep: The OLA goes immediately into deep sleep and does not respond
*/

#define verboseProdTest // Comment this line to disable the helpful Serial messages

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
    all_high &= digitalRead(BREAKOUT_PIN_32); // Read all four pins. If any one is low all_high will be low
    all_high &= digitalRead(BREAKOUT_PIN_TX);
    all_high &= digitalRead(BREAKOUT_PIN_RX);
    all_high &= digitalRead(BREAKOUT_PIN_11);
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

  // OK. The breakout pins were held low and then released (pulled-up) within five seconds so let's go into production test mode!

  //detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS)); // Disable power loss interrupt
  digitalWrite(PIN_STAT_LED, LOW); // Turn the STAT LED off
  powerLEDOff(); // Turn the power LED on - if the hardware supports it

  analogReadResolution(14); //Increase from default of 10

  readVIN(); // Read VIN now to initialise the analog pin

  SerialLog.begin(115200); // Begin the serial port using the TX and RX breakout pins

#ifdef verboseProdTest
  Serial.println(F("OLA Production Test initiated!"));
  Serial.println(F("Waiting for command bytes on the RX breakout pin..."));
#endif

  bool sendHellos = false; // Flag to indicate if we should be sending repeated Hellos via USB (command 0x14/0x15)
  unsigned long lastHelloSent = 0; // Use this to record the last time a Hello was sent
  bool echoUSB = false; // Flag to indicate if we should be echoing on USB (command 0x16/0x17)

  while (1) // Do this loop forever!
  {
    while (!SerialLog.available()) // Wait until we receive a command byte
    {
      if ((sendHellos == true) && (millis() > lastHelloSent)) // Is it time to send a Hello? (5 x 10 / 115200 = 0.434ms)
      {
        Serial.print(F("Hello"));
        lastHelloSent = millis();
      }
      if (echoUSB == true) // Should we echo everything received via USB?
      {
        while (Serial.available())
        {
          Serial.write(Serial.read()); // Echo
        }
      }
    }

    // Command byte received! Let's process it.
    uint8_t commandByte = SerialLog.read();
#ifdef verboseProdTest
    Serial.printf("Processing command byte: 0x%02X\r\n", commandByte);
#endif
    
    switch (commandByte)
    {
      case 0x01: // VIN/3 Divider
      {
        float vin = readVIN(); // Read VIN
#ifdef verboseProdTest
        Serial.printf("VIN is %fV\r\n", vin);
#endif
        if ((vin >= 4.75) && (vin <= 5.25)) // Success
        {
          SerialLog.write(0x01);
        }
        else
        {
          SerialLog.write(0x81);         
        }
      } // / 0x01: VIN/3
        break;
      case 0x02: // IMU Temperature
      {
        enableCIPOpullUp(); // Enable CIPO pull-up
        beginIMU();
        if (online.IMU)
        {
          delay(100); // Give the IMU time to get going
          if (myICM.dataReady())
          {
            myICM.getAGMT(); //Update values
#ifdef verboseProdTest
            Serial.printf("IMU Temp is: %.2fC\r\n", myICM.temp());
#endif
            if ((myICM.temp() >= 10.0) && (myICM.temp() <= 40.0))
            {
              SerialLog.write(0x02); // Test passed
            }
            else
            {
              SerialLog.write(0x82); // Test failed - readings are out of range
            }
          }
          else
          {
            SerialLog.write(0x82); // Test failed - IMU data is not ready
#ifdef verboseProdTest
            Serial.println(F("IMU data not ready!"));
#endif
          }
        }
        else
        {
          SerialLog.write(0x82); // Test failed - IMU is not online
#ifdef verboseProdTest
          Serial.println(F("IMU is not online!"));
#endif
        }
        imuPowerOff();
      } // / 0x02: IMU Temperature
        break;
      case 0x03: // IMU Accelerometer
      {
        enableCIPOpullUp(); // Enable CIPO pull-up
        beginIMU();
        if (online.IMU)
        {
          delay(100); // Give the IMU time to get going
          if (myICM.dataReady())
          {
            myICM.getAGMT(); //Update values
#ifdef verboseProdTest
            Serial.printf("IMU Accel readings are: %.2f %.2f %.2f mg\r\n", myICM.accX(), myICM.accY(), myICM.accZ());
#endif
            if (((myICM.accX() > -100) && (myICM.accX() < 100)) && // Check the readings are in range
                ((myICM.accY() > -100) && (myICM.accY() < 100)) &&
                ((myICM.accZ() > -1100) && (myICM.accZ() < -900)))
            {
              SerialLog.write(0x03); // Test passed
            }
            else
            {
              SerialLog.write(0x83); // Test failed - readings are out of range
            }
          }
          else
          {
            SerialLog.write(0x83); // Test failed - IMU data is not ready
#ifdef verboseProdTest
            Serial.println(F("IMU data not ready!"));
#endif
          }
        }
        else
        {
          SerialLog.write(0x83); // Test failed - IMU is not online
#ifdef verboseProdTest
          Serial.println(F("IMU is not online!"));
#endif
        }
        imuPowerOff();
      } // / 0x03: IMU Accelerometer
        break;
      case 0x04: // IMU Magnetometer
      {
        enableCIPOpullUp(); // Enable CIPO pull-up
        beginIMU();
        if (online.IMU)
        {
          delay(100); // Give the IMU time to get going
          if (myICM.dataReady())
          {
            myICM.getAGMT(); //Update values
#ifdef verboseProdTest
            Serial.printf("IMU Mag readings are: %.2f %.2f %.2f nT\r\n", myICM.magX(), myICM.magY(), myICM.magZ());
#endif
            float magVectorProduct = sqrt((myICM.magX() * myICM.magX()) + (myICM.magY() * myICM.magY())); // Calculate the vector product of magX and magY
#ifdef verboseProdTest
            Serial.printf("IMU Mag XY vector product is: %.2f nT\r\n", magVectorProduct);
#endif
            if (((myICM.magZ() >= 20.0) && (myICM.magZ() <= 30.0)) && // Check the readings are in range
                ((magVectorProduct >= 20.0) && (magVectorProduct <= 30.0)))
            {
              SerialLog.write(0x04); // Test passed
            }
            else
            {
              SerialLog.write(0x84); // Test failed - readings are out of range
            }
          }
          else
          {
            SerialLog.write(0x84); // Test failed - IMU data is not ready
#ifdef verboseProdTest
            Serial.println(F("IMU data not ready!"));
#endif
          }
        }
        else
        {
          SerialLog.write(0x84); // Test failed - IMU is not online
#ifdef verboseProdTest
          Serial.println(F("IMU is not online!"));
#endif
        }
        imuPowerOff();
      } // / 0x04: IMU Magnetometer
        break;
      case 0x05: // RTC Crystal
      {
        myRTC.getTime(); // Read the RTC
        unsigned long hundredthsBeforeSleep = (myRTC.hour * 360000) +  (myRTC.minute * 6000) + (myRTC.seconds * 100) + myRTC.hundredths;
#ifdef verboseProdTest
        Serial.printf("RTC time in hundredths is %d\r\n", hundredthsBeforeSleep);
        Serial.println(F("Going into deep sleep for 5 seconds..."));
#endif
        Serial.flush(); //Finish any prints
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
        //Use the lower power 32kHz clock. Use it to run CT6 as well.
        am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
        am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_G_ENABLE);
        //Setup interrupt to trigger when the number of ms have elapsed
        am_hal_stimer_compare_delta_set(6, 163840); // Sleep for 5 seconds @ 32768 kHz
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
        //Go back to using the main clock
        //am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW);
        //NVIC_EnableIRQ(STIMER_IRQn);
        am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
        am_hal_stimer_config(AM_HAL_STIMER_HFRC_3MHZ);
        //Turn on ADC
        ap3_adc_setup();
        Serial.begin(115200);
        SerialLog.begin(115200);
        SPI.begin();
        myRTC.getTime(); // Read the RTC
        unsigned long hundredthsAfterSleep = (myRTC.hour * 360000) +  (myRTC.minute * 6000) + (myRTC.seconds * 100) + myRTC.hundredths;
        unsigned long elapsedHundredths = hundredthsAfterSleep - hundredthsBeforeSleep;
#ifdef verboseProdTest
        Serial.println(F("Awake!"));
        Serial.printf("RTC time in hundredths is now %d\r\n", hundredthsAfterSleep);
        Serial.printf("Elapsed RTC time in hundredths is %d\r\n", elapsedHundredths);
#endif
        if ((elapsedHundredths > 490) && (elapsedHundredths < 510))
        {
          SerialLog.write(0x05); // Test passed
        }
        else
        {
          SerialLog.write(0x85); // Test failed
        }
      } // /0x05: RTC Crystal
        break;
      case 0x06:
        qwiicPowerOn();
        SerialLog.write(0x06);
        break;
      case 0x07:
        qwiicPowerOff();
        SerialLog.write(0x07);
        break;
      case 0x08:
        powerLEDOn();
        SerialLog.write(0x08);
        break;
      case 0x09:
        powerLEDOff();
        SerialLog.write(0x09);
        break;
      case 0x0A:
        digitalWrite(PIN_STAT_LED, HIGH);
        SerialLog.write(0x0A);
        break;
      case 0x0B:
        digitalWrite(PIN_STAT_LED, LOW);
        SerialLog.write(0x0B);
        break;
      case 0x0C:
        pinMode(BREAKOUT_PIN_32, OUTPUT);
        digitalWrite(BREAKOUT_PIN_32, HIGH);
        SerialLog.write(0x0C);
        break;
      case 0x0D:
        pinMode(BREAKOUT_PIN_32, OUTPUT);
        digitalWrite(BREAKOUT_PIN_32, LOW);
        SerialLog.write(0x0D);
        break;
      case 0x0E:
        pinMode(PIN_QWIIC_SCL, OUTPUT);
        digitalWrite(PIN_QWIIC_SCL, HIGH);
        SerialLog.write(0x0E);
        break;
      case 0x0F:
        pinMode(PIN_QWIIC_SCL, OUTPUT);
        digitalWrite(PIN_QWIIC_SCL, LOW);
        SerialLog.write(0x0F);
        break;
      case 0x10:
        pinMode(PIN_QWIIC_SDA, OUTPUT);
        digitalWrite(PIN_QWIIC_SDA, HIGH);
        SerialLog.write(0x10);
        break;
      case 0x11:
        pinMode(PIN_QWIIC_SDA, OUTPUT);
        digitalWrite(PIN_QWIIC_SDA, LOW);
        SerialLog.write(0x11);
        break;
      case 0x12:
        pinMode(BREAKOUT_PIN_11, OUTPUT);
        digitalWrite(BREAKOUT_PIN_11, HIGH);
        SerialLog.write(0x12);
        break;
      case 0x13:
        pinMode(BREAKOUT_PIN_11, OUTPUT);
        digitalWrite(BREAKOUT_PIN_11, LOW);
        SerialLog.write(0x13);
        break;
      case 0x14:
        sendHellos = true;
        SerialLog.write(0x14);
        break;
      case 0x15:
        sendHellos = false;
        SerialLog.write(0x15);
        break;
      case 0x16:
        echoUSB = true;
        SerialLog.write(0x16);
        break;
      case 0x17:
        echoUSB = false;
        SerialLog.write(0x17);
        break;
      case 0x18:
        myRTC.getTime(); // Read the RTC
        SerialLog.write(0x18);
        SerialLog.printf("%02d:%02d:%02d.%02d", myRTC.hour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
#ifdef verboseProdTest
        Serial.printf("RTC time is %02d:%02d:%02d.%02d\r\n", myRTC.hour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
#endif
        break;
      case 0x19: // SD Card Test
      {
        beginSD(); //285 - 293ms
        enableCIPOpullUp(); // Enable CIPO pull-up after beginSD
        if (online.microSD == true)
        {
          if (sd.exists("OLA_prod_test.txt"))
            sd.remove("OLA_prod_test.txt");
      
          SdFile testFile; //FAT32
          if (testFile.open("OLA_prod_test.txt", O_CREAT | O_APPEND | O_WRITE) == true)
          {
#ifdef verboseProdTest
            Serial.println(F("Test file created"));
#endif                              
            testFile.println(F("112358132134")); // Write the Fibonacci sequence - just for fun
            testFile.close(); // Close the file
            if (testFile.open("OLA_prod_test.txt", O_READ) == true)
            {
#ifdef verboseProdTest
              Serial.println(F("Test file reopened"));
#endif                              
              char line[60];
              int n = testFile.fgets(line, sizeof(line));
              if (n == 13)
              {
                if (strcmp(line, "112358132134\n") == 0) // Look for the correct sequence
                {
                  testFile.close(); // Close the file
                  SerialLog.write(0x19); // Test passed
#ifdef verboseProdTest
                  Serial.println(F("SD card test passed - file contents are correct"));
#endif                              
                }
                else
                {
                  testFile.close(); // Close the file
                  SerialLog.write(0x99); // Test failed - data did not compare
#ifdef verboseProdTest
                  Serial.println(F("Test file contents incorrect!"));
                  for (int l = 0; l < 13; l++)
                    Serial.printf("0x%02X ", line[l]);
                  Serial.println();
#endif                              
                }
              }
              else
              {
                testFile.close(); // Close the file
                SerialLog.write(0x99); // Test failed - test file contents are not the correct length
#ifdef verboseProdTest
                Serial.printf("Test file contents incorrect length (%d)!\r\n", n);
#endif                              
              }
            }
            else
            {
              SerialLog.write(0x99); // Test failed - could not reopen the test file
#ifdef verboseProdTest
              Serial.println(F("Failed to reopen test file"));
#endif              
            }
          }
          else
          {
            SerialLog.write(0x99); // Test failed - could not create the test file
#ifdef verboseProdTest
            Serial.println(F("Failed to create test file"));
#endif
          }
        }
        else
        {
          SerialLog.write(0x99); // Test failed - SD is not online
#ifdef verboseProdTest
          Serial.println(F("SD card is not online!"));
#endif          
        }
        microSDPowerOff();
      } // / 0x19: SD Card Test
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
      } // / 0x55: Deep sleep
        break;
      default:
#ifdef verboseProdTest
        Serial.printf("Unrecognised command byte: 0x%02X\r\n", commandByte);
#endif
        break;
    }
  }
}
