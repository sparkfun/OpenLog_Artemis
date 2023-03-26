REM arduino-cli config init
arduino-cli config add board_manager.additional_urls https://raw.githubusercontent.com/sparkfun/Arduino_Apollo3/main/package_sparkfun_apollo3_index.json
arduino-cli core install SparkFun:Apollo3

REM arduino-cli lib install SparkFun_ICM_20948_IMU
arduino-cli lib install "SparkFun PHT MS8607 Arduino Library"
arduino-cli lib install SdFat
arduino-cli lib install "SparkFun 9DoF IMU Breakout - ICM 20948 - Arduino Library"
arduino-cli lib install "SparkFun I2C Mux Arduino Library"
arduino-cli lib install "SparkFun CCS811 Arduino Library"
arduino-cli lib install "SparkFun VL53L1X 4m Laser Distance Sensor"
arduino-cli lib install "SparkFun BME280"
arduino-cli lib install "SparkFun LPS25HB Pressure Sensor Library"
arduino-cli lib install "SparkFun VEML6075 Arduino Library"
arduino-cli lib install "SparkFun PHT MS8607 Arduino Library"
arduino-cli lib install "SparkFun MCP9600 Thermocouple Library"
arduino-cli lib install "SparkFun SGP30 Arduino Library"
arduino-cli lib install "SparkFun VCNL4040 Proximity Sensor Library"
arduino-cli lib install "SparkFun MS5637 Barometric Pressure Library"
arduino-cli lib install "SparkFun High Precision Temperature Sensor TMP117 Qwiic"
arduino-cli lib install "SparkFun u-blox GNSS Arduino Library"
arduino-cli lib install "SparkFun Qwiic Scale NAU7802 Arduino Library"
arduino-cli lib install "SparkFun SCD30 Arduino Library"
arduino-cli lib install "SparkFun Qwiic Humidity AHT20"
arduino-cli lib install "SparkFun SHTC3 Humidity and Temperature Sensor Library"
arduino-cli lib install "SparkFun ADS122C04 ADC Arduino Library"
arduino-cli lib install "SparkFun MicroPressure Library"
arduino-cli lib install "SparkFun Particle Sensor Panasonic SN-GCJA5"
arduino-cli lib install "SparkFun SGP40 Arduino Library"
arduino-cli lib install "SparkFun SDP3x Arduino Library"
arduino-cli lib install "SparkFun Qwiic Button and Qwiic Switch Library"
arduino-cli lib install "SparkFun Bio Sensor Hub Library"
arduino-cli lib install "SparkFun 6DoF ISM330DHCX"
arduino-cli lib install "SparkFun MMC5983MA Magnetometer Arduino Library"
arduino-cli lib install "SparkFun ADS1015 Arduino Library"
arduino-cli lib install "SparkFun KX13X Arduino Library"

arduino-cli config set library.enable_unsafe_install true
arduino-cli lib install --git-url https://github.com/sparkfunX/BlueRobotics_MS5837_Library
arduino-cli lib install --git-url https://github.com/nstran129/INA3221
arduino-cli lib install --git-url https://github.com/nstran129/arduino-mcp23017#87300b56596ca7c8765c3c2af90a10251eac5b15
arduino-cli lib install --git-url https://github.com/nstran129/MAX11615#c327dd457e190bf494dcad0475ac12d5ee4366e9
arduino-cli config set library.enable_unsafe_install false
