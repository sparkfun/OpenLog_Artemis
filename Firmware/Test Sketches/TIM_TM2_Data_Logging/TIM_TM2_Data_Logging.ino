/*
  This is an example for OpenLog Artemis - based on an example from the Qwiic Sound Trigger repo:

  Qwiic Sound Trigger (VM1010) Example: accurate sound event logging with the u-blox ZED-F9P and UBX TIM_TM2 messages

  Qwiic Sound Trigger: https://www.sparkfun.com/products/17979

  By: Paul Clark
  SparkFun Electronics
  Date: August 17th, 2021
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  Based on the u-blox GNSS library example DataLoggingExample2_TIM_TM2:
  Configuring the ZED-F9P GNSS to automatically send TIM TM2 reports over I2C and log them to file on SD card
  
  This example shows how to configure the u-blox ZED-F9P GNSS to send TIM TM2 reports when a sound event is detected
  and automatically log the data to SD card in UBX format.

  This version uses v2.1.0 of the SparkFun Apollo3 (artemis) core.
  
  Please note: v2.1.1 of the core contains a change to the I2C interface which makes communication
  with u-blox modules over I2C less reliable. If you are building this code yourself,
  please use V2.1.0 of the core.
  
  The Board should be set to SparkFun Apollo3 \ RedBoard Artemis ATP.

  ** Please note: this example will only work with u-blox ADR or High Precision GNSS or Time Sync products **

  Hardware Connections:
  Solder a 6-way row of header pins to the Qwiic Sound Trigger so you can access the TRIG pin.
  Connect your ZED-F9P GNSS breakout to the OLA using a Qwiic cable.
  Connect an antenna to your GNSS board.
  Insert a formatted micro-SD card into the socket on the OLA.
  Connect the Qwiic Sound Trigger to the ZED-F9P using a second Qwiic cable.
  Use a jumper cable to connect the TRIG pin on the Qwiic Sound Trigger to the INT pin on the ZED-F9P breakout.
  
  Ensure you have the SparkFun Apollo3 boards installed: http://boardsmanager/All#SparkFun_Apollo3
  Press upload to upload the code onto the Artemis.
  Open the Serial Monitor at 115200 baud to see the output.

  Pull pin 32 low to stop logging. Logging will also stop on low battery.

  To minimise I2C bus errors, it is a good idea to open the I2C pull-up split pad links on
  the u-blox module breakout.

  Each time the Qwiic Sound Trigger detects a sound, it pulls its TRIG pin high. The ZED-F9P will
  detect this on its INT pin and generate a TIM TM2 message. The OLA will log the TIM TM2 message
  to SD card. You can then study the timing of the sound pulse with nanosecond resolution!
  The code will "debounce" the sound event and reset the VM1010 for the next sound event after 250ms.

  Note: TIM TM2 can only capture the timing of one rising edge and one falling edge per
  navigation solution. So with setNavigationFrequency set to 4Hz, we can only see the timing
  of one rising edge every 250ms. The code "debounces" each sound event to make sure there will
  only be one rising edge event per navigation solution.

  TIM TM2 messages are only produced when a rising or falling edge is detected on the INT pin.
  If you disconnect your TRIG to INT jumper wire, the messages will stop.

  Data is logged in u-blox UBX format. Please see the u-blox protocol specification for more details.
  You can replay and analyze the data using u-center:
  https://www.u-blox.com/en/product/u-center

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  Qwiic Sound Trigger: https://www.sparkfun.com/products/17979
*/

#include <SparkFun_PCA9536_Arduino_Library.h> // Click here to get the library: http://librarymanager/All#SparkFun_PCA9536
PCA9536 myTrigger;

#include <SPI.h>

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

#include <SdFat.h> //SdFat v2.0.7 by Bill Greiman. Click here to get the library: http://librarymanager/All#SdFat_exFAT

#define SD_FAT_TYPE 3 // SD_FAT_TYPE = 0 for SdFat/File, 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24)) // 24MHz

#if SD_FAT_TYPE == 1
SdFat32 sd;
File32 myFile; //File that all GNSS data is written to
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile myFile; //File that all GNSS data is written to
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile myFile; //File that all GNSS data is written to
#else // SD_FAT_TYPE == 0
SdFat sd;
File myFile; //File that all GNSS data is written to
#endif  // SD_FAT_TYPE

// OLA Specifics:

//Setup Qwiic Port
#include <Wire.h>
const byte PIN_QWIIC_SCL = 8;
const byte PIN_QWIIC_SDA = 9;
TwoWire qwiic(PIN_QWIIC_SDA,PIN_QWIIC_SCL); //Will use pads 8/9

//Define the pin functions
//Depends on hardware version. This can be found as a marking on the PCB.
//x04 was the SparkX 'black' version.
//v10 was the first red version.
#define HARDWARE_VERSION_MAJOR 1
#define HARDWARE_VERSION_MINOR 0

#if(HARDWARE_VERSION_MAJOR == 0 && HARDWARE_VERSION_MINOR == 4)
const byte PIN_MICROSD_CHIP_SELECT = 10;
const byte PIN_IMU_POWER = 22;
#elif(HARDWARE_VERSION_MAJOR == 1 && HARDWARE_VERSION_MINOR == 0)
const byte PIN_MICROSD_CHIP_SELECT = 23;
const byte PIN_IMU_POWER = 27;
const byte PIN_PWR_LED = 29;
const byte PIN_VREG_ENABLE = 25;
const byte PIN_VIN_MONITOR = 34; // VIN/3 (1M/2M - will require a correction factor)
#endif

const byte PIN_POWER_LOSS = 3;
const byte PIN_MICROSD_POWER = 15;
const byte PIN_QWIIC_POWER = 18;
const byte PIN_STAT_LED = 19;
const byte PIN_IMU_INT = 37;
const byte PIN_IMU_CHIP_SELECT = 44;
const byte PIN_STOP_LOGGING = 32;
const byte BREAKOUT_PIN_32 = 32;
const byte BREAKOUT_PIN_TX = 12;
const byte BREAKOUT_PIN_RX = 13;
const byte BREAKOUT_PIN_11 = 11;
const byte PIN_SPI_SCK = 5;
const byte PIN_SPI_CIPO = 6;
const byte PIN_SPI_COPI = 7;

// Globals and Consts

float vinCorrectionFactor = 1.47; //Correction factor for the VIN measurement; to compensate for the divider impedance
float lowBatteryThreshold = 3.4; // Low battery voltage threshold (Volts)
int lowBatteryReadings = 0; // Count how many times the battery voltage has read low
const int lowBatteryReadingsLimit = 10; // Don't declare the battery voltage low until we have had this many consecutive low readings (to reject sampling noise)
const int sdPowerDownDelay = 100; //Delay for this many ms before turning off the SD card power
bool powerLossSeen = false; //Interrupt flag for power loss detection
bool stopLoggingSeen = false; //Interrupt flag for stop logging detection
bool ignorePowerLossInterrupt = true; // Ignore the power loss interrupt - when attaching the interrupt
bool ignoreStopLoggingInterrupt = true; // Ignore the stop logging interrupt - when attaching the interrupt

// Data Logging Specifics:

#define packetLength 36 // TIM TM2 is 28 + 8 bytes in length (including the sync chars, class, id, length and checksum bytes)
#define VM1010_MODE 0 // The VM1010 mode pin is connected to GPIO0 on the PCA9536
#define VM1010_TRIG 1 // The VM1010 trigger pin (Dout) is connected to GPIO1 on the PCA9536

// Uncomment the next line to keep the SD file open between writes:
//  This will result in much faster writes, but there is a risk of the log file being left open when the power is disconnected
//  and the data being lost
//#define KEEP_FILE_OPEN // Uncomment this line to keep the SD file open between writes

int dotsPrinted = 0; // Print dots in rows of 50 while waiting for a TIM TM2 message

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Callback: printTIMTM2data will be called when new TIM TM2 data arrives
// See u-blox_structs.h for the full definition of UBX_TIM_TM2_data_t
//         _____  You can use any name you like for the callback. Use the same name when you call setAutoTIMTM2callback
//        /                  _____  This _must_ be UBX_TIM_TM2_data_t
//        |                 /                   _____ You can use any name you like for the struct
//        |                 |                  /
//        |                 |                  |
void printTIMTM2data(UBX_TIM_TM2_data_t ubxDataStruct)
{
  // It is the rising edge of the sound event (TRIG) which is important
  // The falling edge is less useful, as it will be "debounced" by the loop code

  if (ubxDataStruct.flags.bits.newRisingEdge) // 1 if a new rising edge was detected
  {
    Serial.println();
    Serial.print(F("Sound Event Detected!"));
  
    Serial.print(F("  Rising Edge Counter: ")); // Rising edge counter
    Serial.print(ubxDataStruct.count);
  
    Serial.print(F("  Time Of Week: "));
    Serial.print(ubxDataStruct.towMsR); // Time Of Week of rising edge (ms)
    Serial.print(F(" ms + "));
    Serial.print(ubxDataStruct.towSubMsR); // Millisecond fraction of Time Of Week of rising edge in nanoseconds
    Serial.println(F(" ns"));
  
    dotsPrinted = 0; // Reset dotsPrinted
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void setup()
{
  //If 3.3V rail drops below 3V, system will power down and maintain RTC
  pinMode(PIN_POWER_LOSS, INPUT); // BD49K30G-TL has CMOS output and does not need a pull-up

  delay(1); // Let PIN_POWER_LOSS stabilize

  if (digitalRead(PIN_POWER_LOSS) == LOW) powerLossISR(); //Check PIN_POWER_LOSS just in case we missed the falling edge
  ignorePowerLossInterrupt = true; // Ignore the power loss interrupt - when attaching the interrupt
  attachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS), powerLossISR, FALLING);
  ignorePowerLossInterrupt = false;

  powerLEDOn(); // Turn the power LED on - if the hardware supports it

  pinMode(PIN_STAT_LED, OUTPUT); // Flash the STAT LED during SD writes
  digitalWrite(PIN_STAT_LED, LOW);

  Serial.begin(115200);

  SPI.begin();

  //while (!Serial); //Wait for user to open terminal
  Serial.println("SparkFun OpenLog Artemis Example");
  Serial.println("u-blox GNSS TIM_TM2 Sound Event Logging");

  analogReadResolution(14); //Increase ADC resolution from default of 10-bit

  beginQwiic(); // Turn the qwiic power on as early as possible

  beginSD(); // Enable power for the SD card

  enableCIPOpullUp(); // Enable CIPO pull-up after beginSD

  imuPowerOff(); // We're not using the IMU so turn it off

  // Initialize the Sound Trigger PCA9536 with a begin function
  if (myTrigger.begin(qwiic) == false)
  {
    Serial.println(F("Sound Trigger (PCA9536) not detected. Please check wiring. Freezing..."));
    while (1)
      ;
  }

  // Configure VM1010_TRIG (GPIO1) as an input for the VM1010 trigger signal (Dout)
  myTrigger.pinMode(VM1010_TRIG, INPUT);

  // Configure VM1010_MODE (GPIO0) as an input for now.
  // The pull-up resistor on the sound trigger will hold the VM1010 in "Wake On Sound" mode.
  // We will configure VM1010_MODE as an output when we want to pull the MODE pin low to clear the wake-up event.
  myTrigger.pinMode(VM1010_MODE, INPUT);

  delay(2500); // Give the GNSS time to power up

  pinMode(PIN_STOP_LOGGING, INPUT_PULLUP);
  delay(1); // Let the pin stabilize
  ignoreStopLoggingInterrupt = true; // Ignore the stop logging interrupt - when attaching the interrupt
  attachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING), stopLoggingISR, FALLING); // Enable the stop logging interrupt
  pinMode(PIN_STOP_LOGGING, INPUT_PULLUP); //Re-attach the pull-up (bug in v2.1.0 of the core)
  ignoreStopLoggingInterrupt = false;

  Serial.println("Initializing SD card...");

  // See if the card is present and can be initialized:
  if (!sd.begin(SD_CONFIG))
  {
    Serial.println("Card failed, or not present. Freezing...");
    // don't do anything more:
    while (1);
  }
  Serial.println("SD card initialized.");

  // Create or open a file called "TIM_TM2.ubx" on the SD card.
  // If the file already exists, the new data is appended to the end of the file.
  myFile.open("TIM_TM2.ubx", FILE_WRITE);
  if(!myFile)
  {
    Serial.println(F("Failed to create UBX data file! Freezing..."));
    while (1);
  }
  #ifndef KEEP_FILE_OPEN
  myFile.close(); // Close the log file. It will be reopened when a sound event is detected.
  #endif

  //myGNSS.enableDebugging(); // Uncomment this line to enable lots of helpful GNSS debug messages on Serial
  //myGNSS.enableDebugging(Serial, true); // Or, uncomment this line to enable only the important GNSS debug messages on Serial

  myGNSS.disableUBX7Fcheck(); // RAWX data can legitimately contain 0x7F, so we need to disable the "7F" check in checkUbloxI2C

  // TIM TM2 messages are 36 bytes long.
  // In this example, the data will arrive no faster than four messages per second.
  // So, setting the file buffer size to 361 bytes should be more than adequate.
  // I.e. room for ten messages plus an empty tail byte.
  myGNSS.setFileBufferSize(361); // setFileBufferSize must be called _before_ .begin

  if (myGNSS.begin(qwiic) == false) //Connect to the u-blox module using Qwiic port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing..."));
    while (1);
  }

  // Uncomment the next line if you want to reset your module back to the default settings with 1Hz navigation rate
  // (This will also disable any "auto" messages that were enabled and saved by other examples and reduce the load on the I2C bus)
  //myGNSS.factoryDefault(); delay(5000);

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR
  
  myGNSS.setNavigationFrequency(4); //Produce four navigation solutions per second

  myGNSS.setAutoTIMTM2callback(&printTIMTM2data); // Enable automatic TIM TM2 messages with callback to printTIMTM2data

  myGNSS.logTIMTM2(); // Enable TIM TM2 data logging

  while (Serial.available()) // Make sure the Serial buffer is empty
  {
    Serial.read();
  }

  Serial.println(F("Press any key to stop logging."));

  resetSoundTrigger(); // Reset the sound trigger - in case it has already been triggered
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void resetSoundTrigger()
{
  // Reset the sound event by:
  myTrigger.digitalWrite(VM1010_MODE, LOW); // Get ready to pull the VM1010 MODE pin low
  myTrigger.pinMode(VM1010_MODE, OUTPUT); // Change the PCA9536 GPIO0 pin to an output. It will pull the VM1010 MODE pin low
  myTrigger.pinMode(VM1010_MODE, INPUT); // Change the PCA9536 GPIO0 pin back to an input (with pull-up), so it will not 'fight' the mode button  
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void loop()
{
  checkBattery(); // Check for low battery

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  if ((powerLossSeen) || (stopLoggingSeen) || (Serial.available() > 0)) // Check for power loss or stop logging interrupts
  {
    stopLogging();
  }

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  myGNSS.checkUblox(); // Check for the arrival of new data and process it.
  myGNSS.checkCallbacks(); // Check if any callbacks are waiting to be processed.

  // =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  // Check to see if a new packetLength-byte TIM TM2 message has been stored
  // This indicates if a sound event was detected
  if (myGNSS.fileBufferAvailable() >= packetLength)
  {
    uint8_t myBuffer[packetLength]; // Create our own buffer to hold the data while we write it to SD card

    myGNSS.extractFileBufferData((uint8_t *)&myBuffer, packetLength); // Extract exactly packetLength bytes from the UBX file buffer and put them into myBuffer

    digitalWrite(PIN_STAT_LED, HIGH); // We will use PIN_STAT_LED to indicate when data is being written to the SD card

    #ifndef KEEP_FILE_OPEN
    myFile.open("TIM_TM2.ubx", FILE_WRITE); // Reopen the file
    #endif
    
    myFile.write(myBuffer, packetLength); // Write exactly packetLength bytes from myBuffer to the ubxDataFile on the SD card
    
    #ifndef KEEP_FILE_OPEN
    myFile.close(); // Close the log file after the write. Slower but safer...
    #endif

    digitalWrite(PIN_STAT_LED, LOW); // Turn the LED off

    //printBuffer(myBuffer); // Uncomment this line to print the data

    // "debounce" the sound event by 250ms so we only capture one rising edge per navigation solution
    for (int i = 0; i < 250; i++)
    {
      if (powerLossSeen) // Check for power loss
      {
        break; // Stop the debounce if the battery is low
      }
      delay(1); // Wait 250 * 1ms
    }

    resetSoundTrigger(); // Reset the sound trigger. This will generate a falling edge TIM_TM2 message
  }

  Serial.print("."); // Print dots in rows of 50 while we wait for a sound event
  delay(50);
  if (++dotsPrinted > 50)
  {
    Serial.println();
    dotsPrinted = 0;
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Print the buffer contents as Hexadecimal
// You should see:
// SYNC CHAR 1: 0xB5
// SYNC CHAR 2: 0x62
// CLASS: 0x0D for TIM
// ID: 0x03 for TM2
// LENGTH: 2-bytes Little Endian (0x1C00 = 28 bytes for TIM TM2)
// PAYLOAD: LENGTH bytes
// CHECKSUM_A
// CHECKSUM_B
// Please see the u-blox protocol specification for more details
void printBuffer(uint8_t *ptr)
{
  for (int i = 0; i < packetLength; i++)
  {
    if (ptr[i] < 16) Serial.print("0"); // Print a leading zero if required
    Serial.print(ptr[i], HEX); // Print the byte as Hexadecimal
    Serial.print(" ");
  }
  Serial.println();
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Stop Logging ISR
void stopLoggingISR(void)
{
  if (ignoreStopLoggingInterrupt == false)
  {
    Serial.println(F("Stop Logging Seen!"));
    detachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING)); //Prevent multiple interrupts
    stopLoggingSeen = true;
  }
}

//Power Loss ISR
void powerLossISR(void)
{
  if (ignorePowerLossInterrupt == false)
  {
    Serial.println(F("Power Loss Detected!"));
    detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS)); //Prevent multiple interrupts
    powerLossSeen = true;
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

void stopLogging(void) // Stop logging; close the SD file; go into deep sleep
{
  Serial.println(F("\nstopLogging:"));
  
  #ifdef KEEP_FILE_OPEN
  myFile.close(); // Close the data file
  #endif
  
  Serial.println(F("Logging stopped. Freezing..."));
  Serial.flush(); // Make sure final message is printed

  delay(sdPowerDownDelay); // Give the SD card time to finish writing ***** THIS IS CRITICAL *****

  detachInterrupt(digitalPinToInterrupt(PIN_STOP_LOGGING)); //Prevent stop logging button from waking us from sleep
  detachInterrupt(digitalPinToInterrupt(PIN_POWER_LOSS)); //Prevent voltage supervisor from waking us from sleep

  qwiic.end(); //Power down I2C

  SPI.end(); //Power down SPI

  powerControlADC(false); //Power down ADC. It it started by default before setup().

  Serial.end(); //Power down UART
  
  powerLEDOff(); // Turn the power LED off - if the hardware supports it
  
  imuPowerOff();

  microSDPowerOff();

  qwiicPowerOff(); // Power off the Qwiic bus

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
    if ((x != PIN_MICROSD_POWER) &&
        //(x != PIN_LOGIC_DEBUG) &&
        (x != PIN_QWIIC_POWER) &&
        (x != PIN_IMU_POWER))
    {
      am_hal_gpio_pinconfig(x, g_AM_HAL_GPIO_DISABLE);
    }
  }

  //We can't leave these power control pins floating
  imuPowerOff();
  microSDPowerOff();
  qwiicPowerOff(); // Power off the Qwiic bus

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

// OLA Specifics

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

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

void qwiicPowerOn()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
#if(HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, LOW);
#else
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#endif
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

void beginQwiic()
{
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  qwiicPowerOn();
  qwiic.begin();
  setQwiicPullups(0); //Just to make it really clear what pull-ups are being used, set pullups here.
}

void beginSD()
{
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected

  // For reasons I don't understand, we seem to have to wait for at least 1ms after SPI.begin before we call microSDPowerOn.
  // If you comment the next line, the Artemis resets at microSDPowerOn when beginSD is called from wakeFromSleep...
  // But only on one of my V10 red boards. The second one I have doesn't seem to need the delay!?
  delay(1);

  microSDPowerOn();
}

void setQwiicPullups(uint32_t i2cBusPullUps)
{
  //Change SCL and SDA pull-ups manually using pin_config
  am_hal_gpio_pincfg_t sclPinCfg = g_AM_BSP_GPIO_IOM1_SCL;
  am_hal_gpio_pincfg_t sdaPinCfg = g_AM_BSP_GPIO_IOM1_SDA;

  if (i2cBusPullUps == 0)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_NONE; // No pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_NONE;
  }
  else if (i2cBusPullUps == 1)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K; // Use 1K5 pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K;
  }
  else if (i2cBusPullUps == 6)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_6K; // Use 6K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_6K;
  }
  else if (i2cBusPullUps == 12)
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_12K; // Use 12K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_12K;
  }
  else
  {
    sclPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_24K; // Use 24K pull-ups
    sdaPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_24K;
  }

  pin_config(PinName(PIN_QWIIC_SCL), sclPinCfg);
  pin_config(PinName(PIN_QWIIC_SDA), sdaPinCfg);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

bool enableCIPOpullUp()
{
  //Add 1K5 pull-up on CIPO
  am_hal_gpio_pincfg_t cipoPinCfg = g_AM_BSP_GPIO_IOM0_MISO;
  cipoPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K;
  pin_config(PinName(PIN_SPI_CIPO), cipoPinCfg);
  return (true);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Read the VIN voltage
float readVIN()
{
  // Only supported on >= V10 hardware
#if(HARDWARE_VERSION_MAJOR == 0)
  return(0.0); // Return 0.0V on old hardware
#else
  int div3 = analogRead(PIN_VIN_MONITOR); //Read VIN across a 1/3 resistor divider
  float vin = (float)div3 * 3.0 * 2.0 / 16384.0; //Convert 1/3 VIN to VIN (14-bit resolution)
  vin = vin * vinCorrectionFactor; //Correct for divider impedance (determined experimentally)
  //Serial.print(F("VIN (Volts): "));
  //Serial.println(vin, 2);
  return (vin);
#endif
}

// Read the battery voltage
// If it is low, increment lowBatteryReadings
// If lowBatteryReadings exceeds lowBatteryReadingsLimit then powerDown
void checkBattery(void)
{
#if(HARDWARE_VERSION_MAJOR >= 1)
  float voltage = readVIN(); // Read the battery voltage
  if (voltage < lowBatteryThreshold) // Is the voltage low?
  {
    lowBatteryReadings++; // Increment the low battery count
    if (lowBatteryReadings > lowBatteryReadingsLimit) // Have we exceeded the low battery count limit?
    {
      // Gracefully powerDown
      Serial.println(F("Low battery detected!"));
      stopLogging();
    }
  }
  else
  {
    lowBatteryReadings = 0; // Reset the low battery count
  }
#endif
}
