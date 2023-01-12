# OpenLog Artemis : Sensor Units

This document summarizes the units used for each sensor measurement.

---
## Index

### Built-in Inertial Measurement Unit:

- [ICM-20948 IMU](#ICM-20948-IMU)

### Global Navigation Satellite System (GNSS) navigation data:

- [u-blox GNSS boards](#u-blox-GNSS-boards)

### Pressure, Altitude, Humidity and Temperature Data:

- [BME280 atmospheric sensor](#BME280-atmospheric-sensor)
- [LPS25HB absolute pressure sensor](#LPS25HB-absolute-pressure-sensor)
- [MS8607 PHT sensor](#MS8607-PHT-sensor)
- [MPR0025PA MicroPressure sensor](#MPR0025PA-MicroPressure-sensor)
- [MS5637 barometric pressure sensor](#MS5637-barometric-pressure-sensor)
- [MS5837 depth and pressure sensor](#MS5837-depth-pressure-sensor)
- [AHT20 humidity and temperature sensor](#AHT20-humidity-and-temperature-sensor)
- [SHTC3 humidity and temperature sensor](#SHTC3-humidity-and-temperature-sensor)

### Differential Pressure:

- [SDP3X differential pressure sensor](#SDP3X-differential-pressure-sensor)

### Air Quality and Environmental Sensors:

- [CCS811 air quality sensor](#CCS811-air-quality-sensor)
- [VEML6075 UV light sensor](#VEML6075-UV-light-sensor)
- [SGP30 air quality and Volatile Organic Compound (VOC) sensor](#SGP30-air-quality-and-VOC-sensor)
- [SGP40 air quality (VOC index) sensor](#SGP40-air-quality-sensor)
- [SCD30 CO2 humidity and temperature sensor](#SCD30-CO2-humidity-and-temperature-sensor)
- [SN-GCJA5 Particle Sensor](#SN-GCJA5-Particle-Sensor)

### Distance:

- [VL53L1X laser Time of Flight (ToF) sensor](#VL53L1X-laser-ToF-sensor)
- [VCNL4040 proximity sensor](#VCNL4040-proximity-sensor)

### Precision Temperature Sensors:

- [MCP9600 thermocouple amplifier](#MCP9600-thermocouple-amplifier)
- [Qwiic PT100 ADS122C04 platinum resistance sensor](#Qwiic-PT100-ADS122C04-platinum-resistance-sensor)
- [TMP117 precision temperature sensor](#TMP117-precision-temperature-sensor)

### Weight:

- [NAU7802 load cell sensor](#NAU7802-load-cell-sensor)

### ADC:

- [Qwiic PT100 ADS122C04 platinum resistance sensor](#Qwiic-PT100-ADS122C04-platinum-resistance-sensor)

### Biometric Sensors:

- [Pulse Oximeter and Heart Rate Sensor](#Pulse-Oximeter)

### Inertial Measurement Unit:

- [ISM330DHCX IMU](#ISM330DHCX-IMU)

### Magnetometer:

- [MMC5983MA magnetometer](#MMC5983MA-Magnetometer)

---
## Sensor Units

---
## ICM-20948 IMU

| []() | | |
|---|---|---|
| Accelerometer | aX,aY,aZ | milli g |
| Gyro | gX,gY,gZ | Degrees per Second |
| Magnetometer | mX,mY,mZ | micro Tesla |
| Temperature | imu_degC | Degrees Centigrade |

---
## u-blox GNSS boards

| []() | | |
|---|---|---|
| Date | gps_Date | MM/DD/YYYY or DD/MM/YYYY |
| Time | gps_Time | HH:MM:SS.SSS |
| Lat & Lon | gps_Lat,gps_Long | Degrees<sup>-7</sup> |
| Altitude | gps_Alt | mm |
| Altitude MSL | gps_AltMSL | mm |
| SIV | gps_SIV | Count |
| Fix Type | gps_FixType | 0-5 |
| Carrier Soln. | gps_CarrierSolution | 0-2 |
| Ground Speed | gps_GroundSpeed | mm/s |
| Heading | gps_Heading | Degrees<sup>-5</sup> |
| PDOP | gps_pDOP | 10<sup>-2</sup> (dimensionless) |
| Time Of Week | gps_iTOW | ms |

Lat = Latitude  
Lon = Longitude  
MSL = Metres above Sea Level  
SIV = Satellites In View  
PDOP = Positional Dilution Of Precision  

Fix Type:  
0: No  
1: Dead Reckoning Only  
2: 2D  
3: 3D  
4: GNSS + Dead Reckoning  
5: Time Only

Carrier Solution:  
0: No  
1: Float Solution  
2: Fixed Solution  

---
## BME280 atmospheric sensor

| []() | | |
|---|---|---|
| Pressure | pressure_Pa | Pascals |
| Humidity | humidity_% | Percent |
| Altitude | altitude_m | m |
| Temperature | temp_degC | Degrees Centigrade |

---
## LPS25HB absolute pressure sensor

| []() | | |
|---|---|---|
| Pressure | pressure_hPa | hectoPascals |
| Temperature | pressure_degC | Degrees Centigrade |

---
## MS8607 PHT sensor

| []() | | |
|---|---|---|
| Humidity | humidity_% | Percent |
| Pressure | hPa | hectoPascals |
| Temperature | degC | Degrees Centigrade |

---
## MPR0025PA MicroPressure sensor

| []() | | |
|---|---|---|
| Pressure (PSI) | PSI | Pounds per Square Inch |
| Pressure (Pa) | Pa | Pascals |
| Pressure (kPa) | kPa | kiloPascals |
| Pressure (torr) | torr | torr |
| Pressure (inHg) | inHg | inches of Mercury |
| Pressure (atm) | atm | atmospheres |
| Pressure (bar) | bar | barometric pressure  |

---
## MS5637 barometric pressure sensor

| []() | | |
|---|---|---|
| Pressure | pressure_hPa | hectoPascals |
| Temperature | temperature_degC | Degrees Centigrade |

---
## MS5837 depth pressure sensor

| []() | | |
|---|---|---|
| Pressure | mbar | millibar |
| Temperature | degC | Degrees Centigrade |
| Depth | depth_m | Metres |
| Altitude | alt_m | Metres |

---
## SDP3X differential pressure sensor

| []() | | |
|---|---|---|
| Pressure | Pa | Pascals |
| Temperature | degC | Degrees Centigrade |

---
## AHT20 humidity and temperature sensor

| []() | | |
|---|---|---|
| Humidity | humidity_% | Percent |
| Temperature | degC | Degrees Centigrade |

---
## SHTC3 humidity and temperature sensor

| []() | | |
|---|---|---|
| Humidity | humidity_% | Percent |
| Temperature | degC | Degrees Centigrade |

---
## CCS811 air quality sensor

| []() | | |
|---|---|---|
| VOC | tvoc_ppb | Parts Per Billion |
| CO<sub>2</sub> | co2_ppm | Parts Per Million |

VOC = Volatile Organic Compounds

---
## VEML6075 UV light sensor

| []() | | |
|---|---|---|
| UVA | uva |  |
| UVB | uvb |  |
| UV Index | uvIndex |  |

---
## SGP30 air quality and VOC sensor

| []() | | |
|---|---|---|
| Total VOC | tvoc_ppb | Parts Per Billion |
| CO<sub>2</sub> | co2_ppm | Parts Per Million |
| H<sub>2</sub> | H2 | none |
| Ethanol | ethanol | none |

---
## SGP40 air quality sensor

| []() | | |
|---|---|---|
| VOC Index | VOCindex | none |

---
## SCD30 CO2 humidity and temperature sensor

| []() | | |
|---|---|---|
| CO<sub>2</sub> | co2_ppm | Parts Per Million |
| Humidity | humidity_% | Percent |
| Temperature | degC | Degrees Centigrade |

---
## SN-GCJA5 Particle Sensor

| []() | | |
|---|---|---|
| Particle Density (1.0µm) | PM1_0 | µg/m<sup>3</sup> |
| Particle Density (2.5µm) | PM2_5 | µg/m<sup>3</sup> |
| Particle Density (10µm) | PM10 | µg/m<sup>3</sup> |
| Particle Count (0.5µm) | PC0_5 | Count |
| Particle Count (1.0µm) | PC1_0 | Count |
| Particle Count (2.5µm) | PC2_5 | Count |
| Particle Count (5.0µm) | PC5_0 | Count |
| Particle Count (7.5µm) | PC7_5 | Count |
| Particle Count (10µm) | PC10 | Count |
| Sensor Status | Sensors |  |
| Photodiode Status | PD | 0-3 |
| Laser Diode Status | LD | 0-3 |
| Fan Status | Fan | 0-3 |

Sensor status:
| []() | |
|---|---|
|  | PD LD Fan |
| 0 | 0  0  0 |
| 1 | Any 1, nor 2 & 3 |
| 2 | Any 2 |
| 3 | Any 3 nor 2 |

PD status:  
0: Normal status  
1: Normal status (within -80% against initial value), with S/W correction  
2: Abnormal (below -90% against initial value), loss of function  
3: Abnormal (below -80% against initial value), with S/W correction  

LD operational status:  
0: Normal status  
1: Normal status (within -70% against initial LOP), with S/W correction  
2: Abnormal (below -90% against initial LOP) or no LOP, loss of function  
3: Abnormal (below -70% against initial LOP), with S/W correction  

Fan operational status:  
0: Normal status  
1: Normal status (1,000rpm or more), with S/W correction  
2: In initial calibration  
3: Abnormal (below 1,000rpm), out of control
  

---
## VL53L1X laser ToF sensor

| []() | | |
|---|---|---|
| Distance | distance_mm | mm |
| Range Status | distance_rangeStatus(0=good) |  |
| Signal Rate | distance_signalRate |  |

---
## VCNL4040 proximity sensor

| []() | | |
|---|---|---|
| Proximity | prox(no unit) | none |
| Ambient Light | ambient_lux |  |

---
## MCP9600 thermocouple amplifier

| []() | | |
|---|---|---|
| Temperature | thermo_degC | Degrees Centigrade |
| Ambient Temperature | thermo_ambientDegC | Degrees Centigrade |

---
## Qwiic PT100 ADS122C04 platinum resistance sensor

| []() | | |
|---|---|---|
| Temperature (C) | degC | Degrees Centigrade |
| Temperature (F) | degF | Degrees Fahrenheit |
| Temperature Internal | degC | Degrees Centigrade |
| Raw Voltage | V*2.048/2^23 | Volts * 2.048 / 2<sup>23</sup> |

---
## TMP117 precision temperature sensor

| []() | | |
|---|---|---|
| Temperature | degC | Degrees Centigrade |

---
## NAU7802 load cell sensor

| []() | | |
|---|---|---|
| Weight | weight(no unit) | none |

---
## Pulse Oximeter

| []() | | |
|---|---|---|
| bpm | heart rate | beats per minute |
| conf% | confidence | percent |
| O2% | oxygen level | percent |
| stat | sensor status | 0 to 3 |
| eStat | extended status | -6 to 1 |
| O2R | oxygen SpO2 R value | |

Sensor status:  
0: No object detected  
1: Object detected  
2: Object other than finger detected  
3: Finger detected  

Extended status:  
0: Success  
+1: Not ready  
-1: Object detected  
-2: Excessive sensor device motion  
-3: No object detected  
-4: Pressing too hard  
-5: Object other than finger detected  
-6: Excessive finger motion  

---
## ISM330DHCX IMU

| []() | | |
|---|---|---|
| Accelerometer | aX,aY,aZ | milli g |
| Gyro | gX,gY,gZ | milli Degrees per Second |

---
## MMC5983MA Magnetometer

| []() | | |
|---|---|---|
| Magnetometer | mX,mY,mZ | Gauss |
| Temperature | degC | Degrees Centigrade |

