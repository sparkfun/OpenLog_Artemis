Change Log
======================

v1.3
---------

* Add 100ms startup time to Qwiic auto-detection. Lack of SD card was causing some sensors to be pinged before they had enough time to power on and ack.
* Add 2000ms startup time to SCD30. This sensor requires significant time to boot. See issue #4.

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