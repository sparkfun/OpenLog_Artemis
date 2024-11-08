# OpenLog Artemis : How To Compile The OLA Firmware Binary

These are  _abbreviated_ instructions on how to compile the OLA firmware. It's more of an aide-memoire really... Sorry about that.

## Install Arduino IDE

Tested version: 1.8.19

(IDE version 2 has not been tested)

## Add Apollo3 To The Additional Boards Manager URLs

Open `File \ Preferences`

Click the File icon to the right of the `Additional Boards Manager URLs` text box

Add:

```
https://raw.githubusercontent.com/sparkfun/Arduino_Apollo3/main/package_sparkfun_apollo3_index.json
```

Click OK

Click OK

Close and re-open the IDE

## Install the Apollo3 Board Package

Open `Tools \ Board \ Boards Manager`

Enter `Apollo3` in the search box

Install the SparkFun Apollo3 Boards. Tested version: 2.2.1

## Install All The Required Libraries

Copy and paste the following into an empty sketch. Click on each link in turn to install the libraries via the Library Manager:

```
// SdFat by Bill Greiman (Tested version: 2.2.0): http://librarymanager/All#SdFat_exFAT
// http://librarymanager/All#SparkFun_ICM_20948_IMU
// http://librarymanager/All#SparkFun_I2C_Mux
// http://librarymanager/All#SparkFun_CCS811
// http://librarymanager/All#SparkFun_VL53L1X
// http://librarymanager/All#SparkFun_BME280
// http://librarymanager/All#SparkFun_LPS25HB
// http://librarymanager/All#SparkFun_VEML6075
// http://librarymanager/All#SparkFun_PHT_MS8607
// http://librarymanager/All#SparkFun_MCP9600
// http://librarymanager/All#SparkFun_SGP30
// http://librarymanager/All#SparkFun_VCNL4040
// http://librarymanager/All#SparkFun_MS5637
// http://librarymanager/All#SparkFun_TMP102
// http://librarymanager/All#SparkFun_TMP117
// http://librarymanager/All#SparkFun_u-blox_GNSS
// http://librarymanager/All#SparkFun_NAU7802
// http://librarymanager/All#SparkFun_SCD30
// http://librarymanager/All#Qwiic_Humidity_AHT20
// http://librarymanager/All#SparkFun_SHTC3
// http://librarymanager/All#SparkFun_ADS122C04
// http://librarymanager/All#SparkFun_MicroPressure
// http://librarymanager/All#SparkFun_Particle_Sensor_SN-GCJA5
// http://librarymanager/All#SparkFun_SGP40
// http://librarymanager/All#SparkFun_SDP3x
// http://librarymanager/All#SparkFun_Qwiic_Button_Switch
// http://librarymanager/All#SparkFun_Bio_Sensor
// http://librarymanager/All#SparkFun_6DoF_ISM330DHCX
// http://librarymanager/All#SparkFun_MMC5983MA
// http://librarymanager/All#SparkFun_ADS1015
// http://librarymanager/All#SparkFun_KX13X
// http://librarymanager/All#SparkFun_LPS28DFW_Arduino_Library
// http://librarymanager/All#SparkFun_VEML7700
```

### Blue Robotics MS5837

Please manually download and install the latest version of the Blue Robotics MS5837 library from:

https://github.com/bluerobotics/BlueRobotics_MS5837_Library/archive/refs/heads/master.zip

(Version 1.1.1 - available through the Arduino Library Manager - is not the latest version...)

## Download the OLA Firmware Zip

Open this link in a web browser to download a complete Zip of the OLA firmware repo:

https://github.com/sparkfun/OpenLog_Artemis/archive/refs/heads/main.zip

Unzip it (Extract All files)

## Copy the OLA Source Code

Navigate to the `Firmware` sub-folder

Copy the entire `OpenLog_Artemis` folder from the Zip file into your `Arduino` folder. This contains the source code for the firmware. The result should be:

```
C:\Users\<Your_User>\Documents\Arduino\OpenLog_Artemis
```

## Patch the Apollo3 Core

The Apollo3 core (2.2.1) requires patching - using code kindly provided by Paulvha. For more information, open [this link](https://github.com/sparkfun/OpenLog_Artemis/issues/117#issuecomment-1085881142) in a web browser.

Navigate to the `Extras` folder in the Zip file. Copy the `UartPower3.zip` file. Paste it into the Apollo3 board package folder. On Windows machines, this is (usually):

```
C:\Users\<Your_User>\AppData\Local\Arduino15\packages\SparkFun\hardware\apollo3
```

On Linux machines, this is (usually):
```
/home/<Your_User>/.arduino15/packages/SparkFun/hardware/apollo3/
```

Unzip it (Extract All files)

**Close the Arduino IDE**

Follow the instructions contained in `uart_power_3.odt`

In summary: replace the following five files with the ones from `UartPower3.zip` :

```
C:\Users\<Your_User>\AppData\Local\Arduino15\packages\SparkFun\hardware\apollo3\2.2.1\cores\arduino\mbed-bridge\core-extend\HardwareSerial.h
C:\Users\<Your_User>\AppData\Local\Arduino15\packages\SparkFun\hardware\apollo3\2.2.1\cores\arduino\mbed-bridge\core-implement\HardwareSerial.cpp
C:\Users\<Your_User>\AppData\Local\Arduino15\packages\SparkFun\hardware\apollo3\2.2.1\cores\mbed-os\drivers\UnbufferedSerial.h
C:\Users\<Your_User>\AppData\Local\Arduino15\packages\SparkFun\hardware\apollo3\2.2.1\cores\mbed-os\targets\TARGET_Ambiq_Micro\TARGET_Apollo3\device\serial_api.c
C:\Users\<Your_User>\AppData\Local\Arduino15\packages\SparkFun\hardware\apollo3\2.2.1\variants\SFE_ARTEMIS_ATP\mbed\libmbed-os.a
```

## Update Apollo3 SPI.cpp

Open the following file:

```
C:\Users\<Your_User>\AppData\Local\Arduino15\packages\SparkFun\hardware\apollo3\2.2.1\libraries\SPI\src\SPI.cpp
```

Search for `::end`

Replace `::end` with:

```
void arduino::MbedSPI::end() {
    if (dev) {
        delete dev;
        dev = NULL;
    }
}
```

Save the updated file

The extra code prevents badness when the Artemis goes into deep sleep

## Enable ICM29048 DMP Support

Open the following file:

```
C:\Users\<Your_User>\Documents\Arduino\libraries\SparkFun_ICM-20948_ArduinoLibrary\src\util\ICM_20948_C.h
```

Uncomment the following line (29):

```
#define ICM_20948_USE_DMP // Uncomment this line to enable DMP support. You can of course use ICM_20948_USE_DMP as a compiler flag too
```

Save the updated file

## Compile / Upload the Code

Re-open the Arduino IDE

Open the main OLA Firmware .ino:

```
C:\Users\<Your_User>\Documents\Arduino\OpenLog_Artemis\OpenLog_Artemis.ino
```

Open the `Tools \ Board` menu. Select `SparkFun Apollo3 \ RedBoard Artemis ATP`

If you have the OLA connected via USB, you can click the `Upload` (Right-Arrow) icon to compile the code and upload it onto the OLA

(The compilation takes a long time. Go make a cup of tea...)

If you want to be able to swap between firmware versions more quickly, use the `Sketch \ Export compiled Binary` to create a binary which
you can upload with the `Artemis Firmware Upload GUI`. See [UPGRADE.md](./UPGRADE.md) for more details.

## Board Versions

If you are compiling for the Red (SparkFun) OLA: leave the hardware version defines as:

```
#define HARDWARE_VERSION_MAJOR 1
#define HARDWARE_VERSION_MINOR 0
```

If you have an original Black (SparkX) OLA - way to go! Change those lines to:

```
#define HARDWARE_VERSION_MAJOR 0
#define HARDWARE_VERSION_MINOR 4
```

## No Power Loss Protection

To disable the sleep-on-power-loss functionality, uncomment this line:

```
#define noPowerLossProtection // Uncomment this line to disable the sleep-on-power-loss functionality
```

