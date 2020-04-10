Change Log
======================

v1.1
---------

* Add support for exFat microSD cards. Tested up to 512GB. Smaller FAT16 and FAT32 formatted cards are still supported.
* Add support for MS8607 PHT sensor
* Add ability to turn on/off power on Qwiic bus when time between sensor readings is > 2s. By default the bus powers down to save power but there may be instances where user wants to keep sensors powered up and running between reads. This can be accessed via the Attached Devices menu.
* Add ability to limit I2C bus speed. Most devices operate at 400kHz I2C. Some (MCP9600) are automatically limited by OLA to 100kHz. There may be instances, because of bus length or other, where the user may want to artifically limit the bus speed. This can be access via the Attached Devices menu.


v1.0
---------
Initial release.