Change Log
======================
v2.9:
* Adds support for the TMP102 temperature sensor

v2.8:
---------

* Corrects the serial token timestamp printing - resolves #192
* The charsReceived debug print ("Total chars received: ") now excludes the length of the timestamps
* Consistent use of File32/ExFile/FsFile/File. Don't use SdFile for temporary files

v2.7:
---------

* Corrects the crash on startup when logging serial data - resolves #182

v2.6:
---------

* Add workflow actions to automatically compile the firmware binaries
* Add support for the LPS28DFW - thank you @gauteh #179 , resolves #177
* Adds a Docker file for compiling the firmware - thank you @gauteh #179
* Only disable I2C SDA and SCL during sleep when I2C bus is being powered down - thank you @whipple63 #167 , resolves #155
* Add calibrationConcentration support for the SCD30 - thank you @hotstick #181
* Add limited support for the VEML7700 light sensor - resolves #184
* Adds much improved support for the NAU7802 scale
  * Negative weights can now be logged - resolves #176
  * The calibration mode can now be set to: None, Internal (default), or External
  * The sample rate, gain and LDO voltage can be selected via the menu
  * The scale can be zero'd and calibrated separately - there are separate menu entries for zero, calibrate and calibration weight entry
* Corrects a major bug which prevented mutiple NAU7802, SCD30 or u-blox GNSS from being configured correctly when connected through a mux
* Corrects a minor bug - corrected array size checking for olaftoa

v2.5:
---------

* Add Tony Whipple's PR #146 - thank you @whipple63
* Add support for the ISM330DHCX, MMC5983MA, KX134 and ADS1015
* Resolve issue #87

v2.4:
---------

* Add noPowerLossProtection to the main branch
* Add changes by KDB: If we are streaming to Serial, start the stream with a Mime Type marker, followed by CR
* Add debug option to only open the menu using a printable character: based on https://github.com/sparkfun/OpenLog_Artemis/pull/125

v2.3:
---------

* Resolve https://forum.sparkfun.com/viewtopic.php?f=171&t=58109

v2.2:
---------

* Use Apollo3 v2.2.1 with changes by paulvha to fix Issue 117 (Thank you Paul!)
  * https://github.com/sparkfun/OpenLog_Artemis/issues/117#issuecomment-1085881142
* Also includes Paul's SPI.end fix
  * https://github.com/sparkfun/Arduino_Apollo3/issues/442

In libraries/SPI/src/SPI.cpp change ```end()``` to:

```
    void arduino::MbedSPI::end() {
        if (dev) {
            delete dev;
            dev = NULL;
        }
    }   
```   

* Use SdFat v2.1.2
* Compensate for missing / not-populated IMU
* Add support for yyyy/mm/dd and ISO 8601 date style (Issue 118)
  * Add support for fractional time zone offsets

v2.1
---------

v2.1 is a 'mini' release:

* The compiled binaries for v2.1 are based on v2.2.0 of the Apollo3 core
* Corrects issue [109](https://github.com/sparkfun/OpenLog_Artemis/issues/109)
* Corrects issue [104](https://github.com/sparkfun/OpenLog_Artemis/issues/104)

v2.0
---------

* Upgrades the OLA firmware to use v2.1.0 of the Apollo3 core
  * Please note: v2.1.1 of Apollo3 contains a feature which makes communication with the u-blox GNSS modules problematic. Please use v2.1.0 until a fix is available
  * If you are compiling the code yourself, please set the board to RedBoard Artemis ATP

v1.11
---------

* Adds support for the ICM-20948 Digital Motion Processor [47](https://github.com/sparkfun/OpenLog_Artemis/issues/47)
  * The OLA's orientation can be logged as a 6-axis or 9-axis Quaternion. Note: only Quat9 provides absolute orientation - using the magnetometer. Quat6 uses the accel and gyro only
  * Open Menu 3 and select option 12 to enable the DMP
* Adds support for exFAT microSD cards [34](https://github.com/sparkfun/OpenLog_Artemis/issues/34)
  * Based on v2.0.6 of Bill Greiman's SdFat library
* Adds a minimum awake time, making it easier to open the Serial menu when the OLA is sleeping between measurements [83](https://github.com/sparkfun/OpenLog_Artemis/issues/83)
* Adds support for the Bio Sensor Hub Pulse Oximeter and Heart Rate Sensor [81](https://github.com/sparkfun/OpenLog_Artemis/issues/81)
  * Requires exclusive use of pins 32 and 11
  * Open Menu 6 (Detect / Configure Attached Devices) to enable Oximeter detection
* Adds stand-alone examples for:
  * ICM-20948 DMP (orientation in Quat6 and Quat9)
  * GNSS RAWX logging
  * GNSS TIM-TM2 logging
* Does not add support for the Qwiic Button [81](https://github.com/sparkfun/OpenLog_Artemis/issues/81)
  * We tried to add support for the QB, but it uses I2C clock stretching and causes all kinds of badness with the Artemis

v1.10
---------

* Improved support for the MS5837
* Adds IMU DMP examples
* Corrects support for the SDP31
* Updates the low-power code as per [78](https://github.com/sparkfun/OpenLog_Artemis/issues/78)
* Corrects wakeFromSleep when using the Tx pin for serial output [79](https://github.com/sparkfun/OpenLog_Artemis/issues/79)

v1.9
---------

* Allows the TX and RX pins to be used for the serial terminal / console
  * This is very cool as it means you can now connect the OLA to your choice of (e.g.) Bluetooth or radio modem and access your data over that instead of USB
  * File transfer using ZMODEM is possible on both USB and TX/RX
* Added support for the SGP40 air quality sensor - which provides a robust VOC Index for indoor air quality
* Added support for the SDP3X differential pressure sensor - opening up possibilities for air flow and air speed measurement
* Added support for the MS5837 depth / pressure sensor - as used in the BlueRobotics Bar02
* Corrects an issue which was corrupting data when using multiple MS8607s [62](https://github.com/sparkfun/OpenLog_Artemis/issues/62)
* Adds serial logging timestamps with a configurable timestamp token [63](https://github.com/sparkfun/OpenLog_Artemis/issues/63) - thank you @DennisMelamed
* Adds a _slow logging_ feature to extend battery life
  * Pin11 can be used to enter slow logging mode [60](https://github.com/sparkfun/OpenLog_Artemis/issues/60) - thank you @ryanneve
  * Slow logging can be entered once per day, with configurable start and end time [46](https://github.com/sparkfun/OpenLog_Artemis/issues/46)
* Adds improved support for the SCD30 [67](https://github.com/sparkfun/OpenLog_Artemis/issues/67) - thank you @paulvha
* Corrects a bug in logMicroseconds [57](https://github.com/sparkfun/OpenLog_Artemis/issues/57)

v1.8
---------

* Added a fix to make sure the MS8607 is detected correctly [54](https://github.com/sparkfun/OpenLog_Artemis/issues/54)
* Added logMicroseconds [49](https://github.com/sparkfun/OpenLog_Artemis/issues/49)
* Added an option to use autoPVT when logging GNSS data [50](https://github.com/sparkfun/OpenLog_Artemis/issues/50)

v1.7
---------

* Corrected the readVin after sleep bug [39](https://github.com/sparkfun/OpenLog_Artemis/issues/39)
* Corrected detection of the MCP9600 (Qwiic Thermocouple) [41](https://github.com/sparkfun/OpenLog_Artemis/issues/41)
* Added support for the MPR MicroPressure Sensor [35](https://github.com/sparkfun/OpenLog_Artemis/issues/35)
* Added support for the SN-GCJA5 Particle Sensor
* IMU full scale and Digital Low Pass Filter settings can now be configured via Menu 3 [42](https://github.com/sparkfun/OpenLog_Artemis/issues/42)

v1.6
---------

* The sensor readings can now be streamed to the serial TX pin (in addition to being displayed in the terminal and logged to SD card)
  * You can enable this feature using Menu 4 (Configure Serial Logging) Option 2
  * The baud rate is set using Menu 4 Option 4 (the serial pin and the terminal have separate baud rates)
  * When serial output is enabled, analog logging on pin 12 is disabled
  * This resolves issue [#32](https://github.com/sparkfun/OpenLog_Artemis/issues/32)
* A new SD Card File Transfer menu has been added
  * You can access this by selecting Menu s from the Main Menu
  * You can now:
    * List the files on the SD card using ls or dir
    * Delete a single file using del filename or rm filename
    * Type the contents of a single file to the terminal using type filename or cat filename
    * Copy or transfer a single file to the serial TX pin using ss filename
    * Transfer a single file or all files to the terminal using the ZMODEM protocol
      * Tera Term supports ZMODEM. Select File\Transfer\ZMODEM\Receive from the pull-down menus
      * To transfer a single file: sz filename
      * To transfer all files: sz *
  * The ZMODEM start delay can be changed using Menu 4 Option 3
  * The code is based on ecm-bitflipper's Arduino_ZModem
  * This resolves issue [#33](https://github.com/sparkfun/OpenLog_Artemis/issues/33)
* The serial and data log files are now timestamped with create and access timestamps
  * By default, the access timestamp is only set when the file is closed
  * You can enable frequent file access timestamps using Menu 1 Option 11
    * Frequent timestamping requires additional SD card writes and may cause problems when logging data at high rates
* An additional delay allows the IMU to start cleanly after being powered down during sleep
  * This resolves issue [#18](https://github.com/sparkfun/OpenLog_Artemis/issues/18)
* The measurement count no longer resets when the menus are opened
  * This resolves issue [#31](https://github.com/sparkfun/OpenLog_Artemis/issues/31)
* Low battery detection is now supported on the v10 (red) version of the OLA
  * Automatic powerDown on low battery can be enabled using Menu 7 Option 4
  * The low battery threshold voltage can be set using Menu 7 Option 5
* The code now defines a power-on delay for each sensor type
  * When waking from sleep, the code will now wait until attempting to begin 'slow' sensors like the SCD30 or a u-blox module
  * You can extend/override the power-on delay by selecting Menu 6, then Configure Qwiic Settings , then Option 3
  * You can extend the delay to a maximum of 60 seconds, which is more than enough time to allow a u-blox module to establish a fix before the data is read
  * This resolves issue [#5](https://github.com/sparkfun/OpenLog_Artemis/issues/5)
* Instead of logging data at regular time intervals, data logging can now also be triggered via Pin 11
  * This new feature is enabled via Menu 1 Option 12
  * Data logging can be triggered on falling or rising edges (Menu 1 Option 13)
  * When triggering is enabled, the normal log rate settings are ignored
  * Pin 11 generates an interrupt rather than being polled, allowing fast events to trigger logging
  * This resolves issue [#36](https://github.com/sparkfun/OpenLog_Artemis/issues/36)
* The IMU temperature reading has been corrected
  * This resolves issue [#28](https://github.com/sparkfun/OpenLog_Artemis/issues/28)

v1.5
---------

* Added the CIPO pull-up
* Improved mux scanning
* Added productionTest
* Reduced the maximum serial logging baud rate to 500000
* Reduced RAM footprint - most Serial.prints now use flash helper text
* Qwiic power-up delay improvements
* Released for V10 production

v1.4
---------

* Added support for the ADS122C04
* Investigated the RTC-reset issue [#13](https://github.com/sparkfun/OpenLog_Artemis/issues/13)
* Added the _stop logging_ feature on Pin 32
* Allow the user to select the Qwiic pull-up resistance
* Qwiic pull-ups are disabled when communicating with a u-blox module
* Regressed to SdFat (FAT32) - we found that SdFat-Beta caused very occasional logging problems
* Added the VIN correction factor

v1.3
---------

* Add 100ms startup time to Qwiic auto-detection. Lack of SD card was causing some sensors to be pinged before they had enough time to power on and ack.
* Add 2000ms startup time to SCD30. This sensor requires significant time to boot. See issue [#4](https://github.com/sparkfun/OpenLog_Artemis/issues/4).

v1.2
---------

* Fix bug in findNextAvailableLog() that was causing logging to fail during after a sleep/wakeup and sometimes corrupt the microSD card.
* Fix calculation of actualHz when power sleeping >2s.
* Log sensor configurations to config file. This allows users to configure an OpenLog Artemis and then deploy that configuration to multiple units as needed.

v1.1
---------

* Add support for exFat microSD cards. Tested up to 512GB. Smaller FAT16 and FAT32 formatted cards are still supported.
* Add support for MS8607 PHT sensor
* Add ability to turn on/off power on Qwiic bus when time between sensor readings is > 2s. By default the bus powers down to save power but there may be instances where user wants to keep sensors powered up and running between reads. This can be accessed via the Attached Devices menu.
* Add ability to limit I2C bus speed. Most devices operate at 400kHz I2C. Some (MCP9600) are automatically limited by OLA to 100kHz. There may be instances, because of bus length or other, where the user may want to artifically limit the bus speed. This can be access via the Attached Devices menu.

v1.0
---------
Initial release.
