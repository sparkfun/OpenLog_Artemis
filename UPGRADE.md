# OpenLog Artemis : Upgrade Instructions

The [**/Binaries**](./Binaries) folder contains the binary files for the different versions of the OLA firmware.

You can upload these to the OLA using the [Artemis Firmware Upload GUI](https://github.com/sparkfun/Artemis-Firmware-Upload-GUI) instead of the Arduino IDE.
Which is handy if you want to quickly update the firmware in the field, or are not familiar with the IDE.

The firmware is customized for the different versions of the OLA hardware. You will find versions for the **X04 SparkX (Black) OLA** and **V10 SparkFun (Red) OLA** plus any subsequent revisions. The filename tells you which hardware the firmware is for and what version it is:

* **OpenLog_Artemis-V10-v211.bin** - is the _stable_ version (2.11) for the **V10 SparkFun (Red) OLA**
* **OpenLog_Artemis-X04-v210.bin** - is the _stable_ version (2.10) for the **X04 SparkX (Black) OLA**

<table class="table table-hover table-striped table-bordered">
  <tr align="center">
   <td><a href="https://www.sparkfun.com/products/16832"><img src="https://cdn.sparkfun.com//assets/parts/1/5/7/5/3/16832-SparkFun_OpenLog_Artemis-02a.jpg"></a></td>
   <td><a href="https://www.sparkfun.com/products/15846"><img src="https://cdn.sparkfun.com//assets/parts/1/4/4/8/0/15846-OpenLog_Artemis-04.jpg"></a></td>
  </tr>
  <tr align="center">
    <td><a href="https://www.sparkfun.com/products/16832">SparkFun OpenLog Artemis V10 (DEV-16832)</a></td>
    <td><a href="https://www.sparkfun.com/products/15846">SparkX OpenLog Artemis X04 (SPX-15846)</a></td>
  </tr>
</table>

* From time to time, you may also see versions with names like **OpenLog_Artemis-V10-v17_BETA.bin**
  * These are _beta_ versions containing new features and improvements which are still being tested and documented

* **OpenLog_Artemis-V10-v210-NoPowerLossProtection.bin** is a special build of v2.10 which has no power loss protection
  * With the normal firmware, the OLA goes into deep sleep if the power fails or is disconnected. This allows the Real Time Clock to keep running, but it does mean that a reset is required to wake the OLA again.
  * With the **NoPowerLossProtection** firmware, the code does not go into deep sleep when the power is lost, meaning that it will restart automatically as soon as power is reconnected.
  * This can be useful for embedded OLA systems which are hard to access. The downside is that the Real Time Clock setting is lost when the OLA restarts.

## To use:

* Download and extract the [OLA repo ZIP](https://github.com/sparkfun/OpenLog_Artemis/archive/main.zip)
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
