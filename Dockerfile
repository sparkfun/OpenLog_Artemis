FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y curl git unzip && apt-get clean

RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin sh

RUN arduino-cli config init

RUN arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/sparkfun/Arduino_Apollo3/main/package_sparkfun_apollo3_index.json

RUN arduino-cli core update-index
RUN arduino-cli core install "Sparkfun:apollo3@2.2.1"

RUN arduino-cli lib update-index

# RUN arduino-cli lib install "SparkFun Qwiic Power Switch Arduino Library"
RUN arduino-cli lib install "SdFat@2.2.2"
RUN arduino-cli lib install "SparkFun 9DoF IMU Breakout - ICM 20948 - Arduino Library"
RUN arduino-cli lib install "SparkFun I2C Mux Arduino Library"
RUN arduino-cli lib install "SparkFun CCS811 Arduino Library"
RUN arduino-cli lib install "SparkFun VL53L1X 4m Laser Distance Sensor"
RUN arduino-cli lib install "SparkFun BME280"
RUN arduino-cli lib install "SparkFun LPS25HB Pressure Sensor Library"
RUN arduino-cli lib install "SparkFun VEML6075 Arduino Library"
RUN arduino-cli lib install "SparkFun PHT MS8607 Arduino Library"
RUN arduino-cli lib install "SparkFun MCP9600 Thermocouple Library"
RUN arduino-cli lib install "SparkFun SGP30 Arduino Library"
RUN arduino-cli lib install "SparkFun VCNL4040 Proximity Sensor Library"
RUN arduino-cli lib install "SparkFun MS5637 Barometric Pressure Library"
RUN arduino-cli lib install "SparkFun High Precision Temperature Sensor TMP117 Qwiic"
RUN arduino-cli lib install "SparkFun u-blox GNSS Arduino Library"
RUN arduino-cli lib install "SparkFun 6DoF ISM330DHCX"
RUN arduino-cli lib install "SparkFun Qwiic Scale NAU7802 Arduino Library"
RUN arduino-cli lib install "SparkFun SCD30 Arduino Library"
RUN arduino-cli lib install "SparkFun Qwiic Humidity AHT20"
RUN arduino-cli lib install "SparkFun SHTC3 Humidity and Temperature Sensor Library"
RUN arduino-cli lib install "SparkFun ADS122C04 ADC Arduino Library"
RUN arduino-cli lib install "SparkFun MicroPressure Library"
RUN arduino-cli lib install "SparkFun Particle Sensor Panasonic SN-GCJA5"
RUN arduino-cli lib install "SparkFun SGP40 Arduino Library"
RUN arduino-cli lib install "SparkFun Qwiic Button and Qwiic Switch Library"
RUN arduino-cli lib install "SparkFun Bio Sensor Hub Library"
RUN arduino-cli lib install "SparkFun MMC5983MA Magnetometer Arduino Library"
RUN arduino-cli lib install "SparkFun ADS1015 Arduino Library"
RUN arduino-cli lib install "SparkFun KX13X Arduino Library"
RUN arduino-cli lib install "SparkFun SDP3x Arduino Library"
# RUN arduino-cli lib install "BlueRobotics MS5837 Library"

WORKDIR /root/Arduino/libraries
RUN curl -L https://github.com/bluerobotics/BlueRobotics_MS5837_Library/archive/refs/heads/master.zip -o ms5837.zip
RUN unzip ms5837.zip

WORKDIR /work

ADD . .

# Patch Apollo Core
WORKDIR /work/Extras
RUN unzip UartPower3.zip
RUN cp HardwareSerial.h /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/arduino/mbed-bridge/core-extend/HardwareSerial.h
RUN cp HardwareSerial.cpp /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/arduino/mbed-bridge/core-implement/HardwareSerial.cpp
RUN cp UnbufferedSerial.h /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/mbed-os/drivers/UnbufferedSerial.h
RUN cp serial_api.c /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/mbed-os/targets/TARGET_Ambiq_Micro/TARGET_Apollo3/device/serial_api.c
RUN cp libmbed-os.a /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/variants/SFE_ARTEMIS_ATP/mbed/libmbed-os.a

WORKDIR /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/libraries/SPI
RUN patch -p1 /work/Extras/spi.diff

# Enable DMP on ICM 20948
RUN sed -i 's|//#define ICM|#define ICM|g' /root/Arduino/libraries/SparkFun_9DoF_IMU_Breakout_-_ICM_20948_-_Arduino_Library/src/util/ICM_20948_C.h

WORKDIR /work/Firmware/OpenLog_Artemis

CMD arduino-cli compile -v -e -b SparkFun:apollo3:sfe_artemis_atp
