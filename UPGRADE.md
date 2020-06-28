# OpenLog Artemis : Upgrade Instructions

The [**/Binaries**](./Binaries) folder contains the binary files for the different versions of the OLA firmware.

You can upload these to the OLA using the [Artemis Firmware Upload GUI](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI) instead of the Arduino IDE.
Which is handy if you want to quickly update the firmware in the field, or are not familiar with the IDE.

The firmware is customized for the different versions of the OLA hardware. You will find versions for the **X04 SparkX (Black) OLA** and **V10 SparkFun (Red) OLA** plus any subsequent revisions. The filename tells you which hardware the firmware is for and what version it is:

* OpenLog_Artemis-X04-v13.bin - is the _stable_ version for the **X04 SparkX (Black) OLA**
* OpenLog_Artemis-X04-v14_BETA.bin - is a _beta_ version for the **X04 SparkX (Black) OLA** containing new features and improvements

## To use:

* Download and extract the [OLA repo ZIP](https://github.com/sparkfun/OpenLog_Artemis/archive/master.zip)
* Download and extract the [AFU repo ZIP](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI/archive/master.zip)
* Run the AFU artemis_firmware_uploader_gui executable for your platform
  * **/Windows** contains the Windows .exe
  * **/OSX** contains an executable for macOS X
  * **/Linux** contains an executable built on Ubuntu
  * **/Raspberry_Pi__Debian** contains an executable for Raspberry Pi 4 (Debian Buster)
* Select the OLA firmware file you'd like to upload from the OLA **/Binaries** folder (should end in *.bin*)
* Attach the OLA target board using USB
* Select the COM port (hit Refresh to refresh the list of USB devices)
* Press **Upload Firmware**

The GUI does take a few seconds to load and run. _**Don't Panic**_ if the GUI does not start right away.

- Your friends at SparkFun.
