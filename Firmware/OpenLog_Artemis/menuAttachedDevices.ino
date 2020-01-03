#define MAX_NUMBER_OF_QWIIC_DEVICES 30

void menuAttachedDevices()
{
  while (1)
  {
    //TODO - Power on Qwiic BUS

    Serial.println();
    Serial.println("Menu: Configure Attached Devices");

    //See what's on the I2C bus. Will set the qwiicAvailable bools.
    if (detectQwiicDevices() == false)
    {
      Serial.println("No devices detected on Qwiic bus");
      delay(3000);
      return;
    }

    //Create array of pointers to the configure functions
    typedef void(*FunctionPointer)();
    FunctionPointer functionPointers[MAX_NUMBER_OF_QWIIC_DEVICES]; //Array is 4 * dim = 120 bytes

    //If device is detected, connect the configure function
    int availableDevices = 1;
    if (qwiicAvailable.LPS25HB)
    {
      functionPointers[availableDevices - 1] = menuConfigure_LPS25HB;
      Serial.printf("%d) Configure LPS25HB Pressure Sensor\n", availableDevices++);
    }
    if (qwiicAvailable.NAU7802)
    {
      functionPointers[availableDevices - 1] = menuConfigure_NAU7802;
      Serial.printf("%d) NAU7802 Load Cell Amplifier\n", availableDevices++);
    }
    if (qwiicAvailable.uBlox)
    {
      functionPointers[availableDevices - 1] = menuConfigure_uBlox;
      Serial.printf("%d) uBlox GPS Receiver\n", availableDevices++);
    }

    if (qwiicAvailable.MCP9600)
    {
      functionPointers[availableDevices - 1] = menuConfigure_MCP9600;
      Serial.printf("%d) MCP9600 Thermocouple Amplifier\n", availableDevices++);
    }

    Serial.println("x) Exit");

    byte incoming = getByteChoice(10); //Timeout after 10 seconds

    //TODO if user enables the MCP9600, go to 100kHz max I2C speed

    if (incoming >= '1' && incoming < ('1' + availableDevices - 1))
    {
      functionPointers[incoming - '0' - 1]();
    }
    else if (incoming == 'x')
      break;
    else if (incoming == 255)
      break;
    else
      printUnknown(incoming);
  }
}

//Let's see what's on the I2C bus
bool detectQwiicDevices()
{
  bool somethingDetected = false;

  //Serial.println("Scanning Qwiic bus");

  qwiic.setClock(100000); //During detection, go slow

  for (uint8_t address = 1 ; address < 127 ; address++)
  {
    qwiic.beginTransmission(address);
    if (qwiic.endTransmission() == 0)
    {
      //Serial.printf("Device found at address 0x%02X\n", address);
      if (testDevice(address) == false)
        Serial.printf("Unknown I2C device found at address 0x%02X\n", address);
      else
        somethingDetected = true;
    }
  }

  return (somethingDetected);
}

// Available Qwiic devices
#define ADR_NAU7802 0x2A
#define ADR_BH1749NUC_2 0x38
#define ADR_BH1749NUC_1 0x39
#define ADR_UBLOX 0x42
#define ADR_LPS25HB_2 0x5C
#define ADR_LPS25HB_1 0x5D
#define ADR_MCP9600_2 0x60
#define ADR_MCP9600_1 0x66

//Given an address, see if it repsonds as we would expect
//Returns false if I2C address is not known
bool testDevice(uint8_t i2cAddress)
{
  switch (i2cAddress)
  {
    case ADR_LPS25HB_1:
      if (pressureSensor.begin(qwiic, ADR_LPS25HB_1) == true) //Wire port, Address.
        if (pressureSensor.isConnected() == true)
          qwiicAvailable.LPS25HB = true;
      break;
    case ADR_LPS25HB_2:
      if (pressureSensor.begin(qwiic, ADR_LPS25HB_2) == true) //Wire port, Address.
        if (pressureSensor.isConnected() == true)
          qwiicAvailable.LPS25HB = true;
      break;
    case ADR_MCP9600_1:
      if (thermoSensor.begin(ADR_MCP9600_1, qwiic) == true) //Address, Wire port
        if (thermoSensor.isConnected() == true)
          qwiicAvailable.MCP9600 = true;
      break;
    case ADR_BH1749NUC_1:
      if (rgb.begin(BH1749NUC_ADDRESS_DEFAULT, qwiic) == BH1749NUC_SUCCESS) //Address, Wire port
        qwiicAvailable.BH1749NUC = true;
      break;
    case ADR_NAU7802:
      if (nauScale.begin(qwiic) == true) //Wire port
        qwiicAvailable.NAU7802 = true;
      break;
    case ADR_UBLOX:
      if (myGPS.begin(qwiic, ADR_UBLOX) == true) //Wire port, address
        qwiicAvailable.uBlox = true;
      break;
    default:
      Serial.printf("Unknown device at address 0x%02X\n", i2cAddress);
      return false;
      break;
  }
  return true;
}

void menuConfigure_LPS25HB()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure LPS25HB Pressure Sensor");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_LPS25HB.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_LPS25HB.log == true)
    {
      Serial.print("2) Log Pressure: ");
      if (settings.sensor_LPS25HB.logPressure == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log Temperature: ");
      if (settings.sensor_LPS25HB.logTemp == true) Serial.println("Enabled");
      else Serial.println("Disabled");
    }
    Serial.println("x) Exit");

    byte incoming = getByteChoice(10); //Timeout after 10 seconds

    if (incoming == '1')
      settings.sensor_LPS25HB.log ^= 1;
    else if (settings.sensor_LPS25HB.log == true)
    {
      if (incoming == '2')
        settings.sensor_LPS25HB.logPressure ^= 1;
      else if (incoming == '3')
        settings.sensor_LPS25HB.logTemp ^= 1;
      else if (incoming == 'x')
        return;
      else if (incoming == 255)
        return;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      return;
    else if (incoming == 255)
      return;
    else
      printUnknown(incoming);
  }
}
void menuConfigure_NAU7802()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure NAU7802 Load Cell Amplifier");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_NAU7802.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_NAU7802.log == true)
    {
      Serial.println("2) Calibrate Scale");
      Serial.printf("\tScale calibration factor: %f\n", settings.sensor_NAU7802.calibrationFactor);
      Serial.printf("\tScale zero offset: %d\n", settings.sensor_NAU7802.zeroOffset);
      Serial.printf("\tWeight currently on scale: %f\n", nauScale.getWeight());

      Serial.printf("3) Number of decimal places: %d\n", settings.sensor_NAU7802.decimalPlaces);
    }

    Serial.println("x) Exit");

    byte incoming = getByteChoice(10); //Timeout after 10 seconds

    if (incoming == '1')
    {
      settings.sensor_NAU7802.log ^= 1;
    }
    else if (settings.sensor_NAU7802.log == true)
    {
      if (incoming == '2')
      {
        //Gives user the ability to set a known weight on the scale and calculate a calibration factor
        Serial.println();
        Serial.println(F("Scale calibration"));

        Serial.println(F("Setup scale with no weight on it. Press a key when ready."));
        waitForInput();

        nauScale.calculateZeroOffset(64); //Zero or Tare the scale. Average over 64 readings.
        Serial.print(F("New zero offset: "));
        Serial.println(nauScale.getZeroOffset());

        Serial.println(F("Place known weight on scale. Press a key when weight is in place and stable."));
        waitForInput();

        Serial.print(F("Please enter the weight, without units, currently sitting on the scale (for example '4.25'): "));
        waitForInput();

        //Read user input
        float weightOnScale = Serial.parseFloat();
        nauScale.calculateCalibrationFactor(weightOnScale, 64); //Tell the library how much weight is currently on it. Average over 64 readings.

        settings.sensor_NAU7802.calibrationFactor = nauScale.getCalibrationFactor();
        settings.sensor_NAU7802.zeroOffset = nauScale.getZeroOffset();

        Serial.println();
      }
      else if (incoming == '3')
      {
        Serial.print("Enter number of decimal places to print (1 to 10): ");
        int places = getNumber(10);
        if (places < 1 || places > 10)
        {
          Serial.println("Error: Decimal places out of range");
        }
        else
        {
          settings.sensor_NAU7802.decimalPlaces = places;
        }
      }
      else if (incoming == 'x')
        return;
      else if (incoming == 255)
        return;
      else
        printUnknown(incoming);
    }
    else if (incoming == 'x')
      return;
    else if (incoming == 255)
      return;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_uBlox()
{
  while (1)
  {
    Serial.println();
    Serial.println("Menu: Configure uBlox GPS Receiver");

    Serial.print("1) Sensor Logging: ");
    if (settings.sensor_uBlox.log == true) Serial.println("Enabled");
    else Serial.println("Disabled");

    if (settings.sensor_uBlox.log == true)
    {
      Serial.print("2) Log GPS Date: ");
      if (settings.sensor_uBlox.logDate == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("3) Log GPS Time: ");
      if (settings.sensor_uBlox.logTime == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("4) Log Longitude/Latitude: ");
      if (settings.sensor_uBlox.logPosition == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("5) Log Altitude: ");
      if (settings.sensor_uBlox.logAltitude == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("6) Log Altitude Mean Sea Level: ");
      if (settings.sensor_uBlox.logAltitudeMSL == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("7) Log Satellites In View: ");
      if (settings.sensor_uBlox.logSIV == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("8) Log Fix Type: ");
      if (settings.sensor_uBlox.logFixType == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("9) Log Carrier Solution: ");
      if (settings.sensor_uBlox.logCarrierSolution == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("10) Log Ground Speed: ");
      if (settings.sensor_uBlox.logGroundSpeed == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("11) Log Heading of Motion: ");
      if (settings.sensor_uBlox.logHeadingOfMotion == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.print("12) Log Position Dilution of Precision (pDOP): ");
      if (settings.sensor_uBlox.logpDOP == true) Serial.println("Enabled");
      else Serial.println("Disabled");

      Serial.flush();

      Serial.print("13) Log Interval Time Of Week (iTOW): ");
      if (settings.sensor_uBlox.logiTOW == true) Serial.println("Enabled");
      else Serial.println("Disabled");

    }
    Serial.println("x) Exit");

    int incoming = getNumber(10); //Timeout after 10 seconds

    if (incoming == 1)
    {
      settings.sensor_uBlox.log ^= 1;
    }
    else if (settings.sensor_NAU7802.log == true)
    {
      if (incoming == 2)
        settings.sensor_uBlox.logDate ^= 1;
      else if (incoming == 3)
        settings.sensor_uBlox.logTime ^= 1;
      else if (incoming == 4)
        settings.sensor_uBlox.logPosition ^= 1;
      else if (incoming == 5)
        settings.sensor_uBlox.logAltitude ^= 1;
      else if (incoming == 6)
        settings.sensor_uBlox.logAltitudeMSL ^= 1;
      else if (incoming == 7)
        settings.sensor_uBlox.logSIV ^= 1;
      else if (incoming == 8)
        settings.sensor_uBlox.logFixType ^= 1;
      else if (incoming == 9)
        settings.sensor_uBlox.logCarrierSolution ^= 1;
      else if (incoming == 10)
        settings.sensor_uBlox.logGroundSpeed ^= 1;
      else if (incoming == 11)
        settings.sensor_uBlox.logHeadingOfMotion ^= 1;
      else if (incoming == 12)
        settings.sensor_uBlox.logpDOP ^= 1;
      else if (incoming == 13)
        settings.sensor_uBlox.logiTOW ^= 1;
      else if (incoming == 255) //getNumber() will return 255 if 'x' is pressed
        return;
      else
        printUnknown(incoming);
    }
    else if (incoming == 255) //getNumber() will return 255 if 'x' is pressed
      return;
    else
      printUnknown(incoming);
  }
}

void menuConfigure_MCP9600()
{
  Serial.println("Configure thermo");
  delay(1000);
}
