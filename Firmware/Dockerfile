FROM ubuntu:latest AS upstream

ARG DEBIAN_FRONTEND=noninteractive

# Get curl, git and unzip
RUN apt-get update \
    && apt-get install -y curl git unzip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Setup Arduino CLI
#RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=/usr/local/bin sh
RUN curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Start config file
RUN arduino-cli config init --additional-urls https://raw.githubusercontent.com/sparkfun/Arduino_Apollo3/main/package_sparkfun_apollo3_index.json

# Update core index
RUN arduino-cli core update-index

# Update library index
RUN arduino-cli lib update-index

# Install core
RUN arduino-cli core install "Sparkfun:apollo3@2.2.1"

# Get known libraries
# RUN arduino-cli lib install "SparkFun Qwiic Power Switch Arduino Library"
RUN arduino-cli lib install "SdFat"@2.2.2
RUN arduino-cli lib install "SparkFun 9DoF IMU Breakout - ICM 20948 - Arduino Library"@1.3.0
RUN arduino-cli lib install "SparkFun I2C Mux Arduino Library"@1.0.3
RUN arduino-cli lib install "SparkFun CCS811 Arduino Library"@2.0.3
RUN arduino-cli lib install "SparkFun VL53L1X 4m Laser Distance Sensor"@1.2.12
RUN arduino-cli lib install "SparkFun BME280"@2.0.11
RUN arduino-cli lib install "SparkFun LPS25HB Pressure Sensor Library"@1.1.1
RUN arduino-cli lib install "SparkFun VEML6075 Arduino Library"@1.1.5
RUN arduino-cli lib install "SparkFun PHT MS8607 Arduino Library"@1.0.5
RUN arduino-cli lib install "SparkFun MCP9600 Thermocouple Library"@1.0.5
RUN arduino-cli lib install "SparkFun SGP30 Arduino Library"@1.0.5
RUN arduino-cli lib install "SparkFun VCNL4040 Proximity Sensor Library"@1.0.4
RUN arduino-cli lib install "SparkFun MS5637 Barometric Pressure Library"@1.0.2
RUN arduino-cli lib install "SparkFun High Precision Temperature Sensor TMP117 Qwiic"@1.2.5
RUN arduino-cli lib install "SparkFun u-blox GNSS Arduino Library"@2.2.26
RUN arduino-cli lib install "SparkFun 6DoF ISM330DHCX"@1.0.6
RUN arduino-cli lib install "SparkFun Qwiic Scale NAU7802 Arduino Library"@1.0.6
RUN arduino-cli lib install "SparkFun SCD30 Arduino Library"@1.0.20
RUN arduino-cli lib install "SparkFun Qwiic Humidity AHT20"@1.0.3
RUN arduino-cli lib install "SparkFun SHTC3 Humidity and Temperature Sensor Library"@1.1.4
RUN arduino-cli lib install "SparkFun ADS122C04 ADC Arduino Library"@1.0.4
RUN arduino-cli lib install "SparkFun MicroPressure Library"@1.0.1
RUN arduino-cli lib install "SparkFun Particle Sensor Panasonic SN-GCJA5"@1.0.1
RUN arduino-cli lib install "SparkFun SGP40 Arduino Library"@1.0.4
RUN arduino-cli lib install "SparkFun Qwiic Button and Qwiic Switch Library"@2.0.6
RUN arduino-cli lib install "SparkFun Bio Sensor Hub Library"@1.0.6
RUN arduino-cli lib install "SparkFun MMC5983MA Magnetometer Arduino Library"@1.1.4
RUN arduino-cli lib install "SparkFun ADS1015 Arduino Library"@2.3.2
RUN arduino-cli lib install "SparkFun KX13X Arduino Library"@2.0.4
RUN arduino-cli lib install "SparkFun SDP3x Arduino Library"@1.0.4
RUN arduino-cli lib install "SparkFun LPS28DFW Arduino Library"@1.0.0
RUN arduino-cli lib install "SparkFun VEML7700 Arduino Library"@1.0.0
RUN arduino-cli lib install "SparkFun TMP102 Breakout"@1.1.2

# Enable external libs
RUN arduino-cli config set library.enable_unsafe_install true

# Add BlueRobotics_MS5837_Library from git
RUN arduino-cli lib install --git-url https://github.com/bluerobotics/BlueRobotics_MS5837_Library.git

# Deploy
FROM upstream AS deployment

# Add the source files
ADD . .

# Patch Apollo Core
RUN cd ./Extras \
    && unzip UartPower3.zip \
    && cp HardwareSerial.h /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/arduino/mbed-bridge/core-extend/HardwareSerial.h \
    && cp HardwareSerial.cpp /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/arduino/mbed-bridge/core-implement/HardwareSerial.cpp \
    && cp UnbufferedSerial.h /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/mbed-os/drivers/UnbufferedSerial.h \
    && cp serial_api.c /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/cores/mbed-os/targets/TARGET_Ambiq_Micro/TARGET_Apollo3/device/serial_api.c \
    && cp libmbed-os.a /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/variants/SFE_ARTEMIS_ATP/mbed/libmbed-os.a \
    && cp SPI.cpp /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/libraries/SPI/src/SPI.cpp

# Enable DMP on ICM 20948
RUN sed -i 's|//#define ICM_20948_USE_DMP|#define ICM_20948_USE_DMP|g' /root/Arduino/libraries/SparkFun_9DoF_IMU_Breakout_-_ICM_20948_-_Arduino_Library/src/util/ICM_20948_C.h

# Enable debug symbols
# echo "compiler.c.extra_flags=-MMD -g3" >> /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/platform.local.txt
# echo "compiler.cxx.extra_flags=-MMD -g3" >> /root/.arduino15/packages/SparkFun/hardware/apollo3/2.2.1/platform.local.txt

# Disable Power Loss Protection
# Uncomment the next two lines to disable power loss protection
#RUN cd ./OpenLog_Artemis \
#    && sed -i 's|//#define noPowerLossProtection|#define noPowerLossProtection|g' OpenLog_Artemis.ino

# X04 Hardware
# Uncomment the next three lines to compile for the original X04 SparkX OLA
#RUN cd ./OpenLog_Artemis \
#    && sed -i 's|#define HARDWARE_VERSION_MAJOR 1|#define HARDWARE_VERSION_MAJOR 0|g' OpenLog_Artemis.ino \
#    && sed -i 's|#define HARDWARE_VERSION_MINOR 0|#define HARDWARE_VERSION_MINOR 4|g' OpenLog_Artemis.ino

# Compile
RUN cd ./OpenLog_Artemis \
    && arduino-cli compile -v -e -b SparkFun:apollo3:sfe_artemis_atp OpenLog_Artemis.ino

# Copy the compile output. List the files
FROM deployment AS output
COPY --from=deployment ./OpenLog_Artemis/build/SparkFun.apollo3.sfe_artemis_atp /
CMD echo $(ls /*.*)
