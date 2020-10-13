# OpenLog Artemis : How To Add A New Sensor

These are  _abbreviated_ instructions on how to add a new sensor to the OLA firmware. It's more of an aide-memoire really... Sorry about that.

Caveat: this is _currently_ how a new sensor is added, for version v1.n of the OLA firmware using v1.2.n of the Apollo3 boards (the OLA code is compiled using the SparkFun RedBoard Artemis ATP (All The Pins) board). This will change, dramatically, when we upgrade to v2 of Apollo3 and integrate BLE support.

## Use The Release Candidate Branch

First up, please target any commits at the _**release_candidate**_ branch, so they can be tested before being merged into the _master_ branch.

- https://github.com/sparkfun/OpenLog_Artemis/tree/release_candidate

## OpenLog_Artemis.ino

- [Add the new sensor to the comments at the start of the file](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-7d096a133c17fd6db382abb9a3c6ea7b42ec505961876cecf404a55be5945347R71)

- [#include the library .h file remembering to include the library manager helper link](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-7d096a133c17fd6db382abb9a3c6ea7b42ec505961876cecf404a55be5945347R203)

## Sensors.ino

### gatherDeviceValues

- This is where the sensor readings are actually read. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-fba25af49a58a7a24fb75cb34321e25dd4a94a9d3515ac051fcaa4502e444f7fR725-R798)

### printHelperText

- [Add helper text for the sensor readings](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-fba25af49a58a7a24fb75cb34321e25dd4a94a9d3515ac051fcaa4502e444f7fR1132-R1165)

## autoDetect.ino

### addDevice

- This code is called to add the discovered device to the linked list of active devices. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-68cc245ab0d3c1bed2bfc22b403edc3ed73d347a35a21179b3a6ec27a458803bR234-R239). The class name comes from the library. The config struct is defined in settings.h.

### beginQwiicDevices

- This is the code that is called to start (begin) the device. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-68cc245ab0d3c1bed2bfc22b403edc3ed73d347a35a21179b3a6ec27a458803bR453-R461). Update _qwiicPowerOnDelayMillis_ if this device needs extra time to get its act together on power-up.

### configureDevice

- If the sensor needs to be configured before use, that gets done here. [Add a case for the new sensor, even if it doesn't need to be configured.](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-68cc245ab0d3c1bed2bfc22b403edc3ed73d347a35a21179b3a6ec27a458803bR705-R707)

### getConfigFunctionPtr

- [Add a pointer to the sensor menu function (defined in menuAttachedDevices.ino)](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-68cc245ab0d3c1bed2bfc22b403edc3ed73d347a35a21179b3a6ec27a458803bR795-R797)

### swap

- [Add the device's I2C address(es) here - just for reference (we don't use these #defines any longer)](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-68cc245ab0d3c1bed2bfc22b403edc3ed73d347a35a21179b3a6ec27a458803bR929)

### testDevice

- This is the code used to detect the sensor. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-68cc245ab0d3c1bed2bfc22b403edc3ed73d347a35a21179b3a6ec27a458803bR987-R994). Provide some indication of how robust the detection is. Confidence would be high if the .begin writes and reads unique data to/from the sensor. Confidence is low if .begin only uses the standard _isConnected_ I2C address test.

### getDeviceName

- This is where the sensor name is defined as text. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-68cc245ab0d3c1bed2bfc22b403edc3ed73d347a35a21179b3a6ec27a458803bR1423-R1425). Keep it brief and follow the same format as the other sensors: usually _TYPE_ (PRESSURE, TEMPERATURE etc.) followed by the abbreviated manufacturer's part number

## menuAttachedDevices.ino

### menuAttachedDevices

- This is the code which lists all the attached devices as a menu. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-6174875faf8039f2627c16aaf48e4db57f5a2c8c883061ac97202d74e9a46ef8R309-R311)

### menuConfigure_

- [Add a new menuConfigure_ for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-6174875faf8039f2627c16aaf48e4db57f5a2c8c883061ac97202d74e9a46ef8R2026-R2141)
- Boolean settings are simple to toggle. [Make them exclusive if you need to](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-6174875faf8039f2627c16aaf48e4db57f5a2c8c883061ac97202d74e9a46ef8R1942-R1948)
- _int64_t_ values are requested using _getNumber_ (defined in support.ino)
- _double_ values are requested using _getDouble_ (defined in support.ino)

## nvm.ino

### recordDeviceSettingsToFile

- This function gets called to write the device settings to file. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-629ae89c3c660583493d544d3a7902728f4a8eefb65800c3acb64aea37d5d88dR611-R629). Include all of the sensor settings.

### parseDeviceLine

- This function gets called when the device settings are read back from file. [Add a case for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-629ae89c3c660583493d544d3a7902728f4a8eefb65800c3acb64aea37d5d88dR1116-R1150)

## settings.h

### enum

- [Add the device to the enumerated sensors](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-c853eddd04f78093fed5ec20b822c3c224bfa5f268738ce4c479b45667f86fe9R25). Insert the new one above _DEVICE_TOTAL_DEVICES_

### settings struct

- [Add a settings struct for the new sensor](https://github.com/sparkfun/OpenLog_Artemis/commit/2a26acd279fa93cfe84f1bc518c0e7a041b3bc44#diff-c853eddd04f78093fed5ec20b822c3c224bfa5f268738ce4c479b45667f86fe9R265-R281). Include both _log_ and _powerOnDelayMillis_

## README.md

- Add the new sensor to the list in [README.md](./README.md). Include a link to the product page
- Update the OLA product page on Sparkle - add the new sensor to the list

## CHANGELOG.md

- Update [CHANGELOG.md](./CHANGELOG.md). Start a new version if you need to. Add the new sensor to the change notes
