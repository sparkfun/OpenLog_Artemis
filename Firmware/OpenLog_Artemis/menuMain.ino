

#include "Sensors.h"

//Display the options
//If user doesn't respond within a few seconds, return to main loop
void menuMain(bool alwaysOpen)
{
  bool restartIMU = false;

  if (settings.openMenuWithPrintable) // If settings.openMenuWithPrintable is true, eat the first character. Return if < 9 (Tab)
  {
    if ((settings.useTxRxPinsForTerminal == true) && (Serial1.available()))
    {
      uint8_t firstChar = Serial1.read();
      if (firstChar < 9)
        return;
    }
    else if (Serial.available())
    {
      uint8_t firstChar = Serial.read();
      if (firstChar < 9)
        return;
    }
    else if (!alwaysOpen)
      return;
  }
  
  while (1)
  {
    SerialPrintln(F(""));
    SerialPrintln(F("Menu: Main Menu"));

    SerialPrintln(F("1) Configure Terminal Output"));

    SerialPrintln(F("2) Configure Time Stamp"));

    if (online.IMU)
      SerialPrintln(F("3) Configure IMU Logging"));

    if (settings.useTxRxPinsForTerminal == false)
      SerialPrintln(F("4) Configure Serial Logging"));

    SerialPrintln(F("5) Configure Analog Logging"));

    SerialPrintln(F("6) Detect / Configure Attached Devices"));

    SerialPrintln(F("7) Configure Power Options"));

    SerialPrintln(F("h) Print Sensor Helper Text (and return to logging)"));

    if (online.microSD)
      SerialPrintln(F("s) SD Card File Transfer"));

    SerialPrintln(F("r) Reset all settings to default"));

    SerialPrintln(F("q) Quit: Close log files and power down"));

    //SerialPrintln(F("d) Debug Menu"));

    SerialPrintln(F("x) Return to logging"));

    byte incoming = getByteChoice(menuTimeout, true); //Get byte choice and set DSERIAL & ZSERIAL

    if (incoming == '1')
      menuLogRate();
    else if (incoming == '2')
      menuTimeStamp();
    else if ((incoming == '3') && (online.IMU))
      restartIMU = menuIMU();
    else if ((incoming == '4') && (settings.useTxRxPinsForTerminal == false))
      menuSerialLogging();
    else if (incoming == '5')
      menuAnalogLogging();
    else if (incoming == '6')
      menuAttachedDevices();
    else if (incoming == '7')
      menuPower();
    else if (incoming == 'h')
    {
      printHelperText(OL_OUTPUT_SERIAL); //printHelperText to terminal only
      break; //return to logging
    }
    else if (incoming == 'd')
    {
      menuDebug();
    }
    else if (incoming == 's')
    {
      if (online.microSD)
      {
        //Close log files before showing sdCardMenu
        if (online.dataLogging == true)
        {
          sensorDataFile.sync();
          updateDataFileAccess(&sensorDataFile); // Update the file access time & date
          sensorDataFile.close();
        }
        if (online.serialLogging == true)
        {
          if (incomingBufferSpot > 0)
          {
            //Write the remainder of the buffer
            digitalWrite(PIN_STAT_LED, HIGH); //Toggle stat LED to indicating log recording
            serialDataFile.write(incomingBuffer, incomingBufferSpot); //Record the buffer to the card
            digitalWrite(PIN_STAT_LED, LOW);

            incomingBufferSpot = 0;

            lastSeriaLogSyncTime = rtcMillis(); //Reset the last sync time to now
          }

          serialDataFile.sync();
          updateDataFileAccess(&serialDataFile); // Update the file access time & date
          serialDataFile.close();
        }
  
        SerialPrintln(F(""));
        SerialPrintln(F(""));
        sdCardMenu(sdCardMenuTimeout); // Located in zmodem.ino
        SerialPrintln(F(""));
        SerialPrintln(F(""));
        
        if (online.dataLogging == true)
        {
          // Check if the current datafile was deleted
          if (sd.exists(sensorDataFileName) == false)
            strcpy(sensorDataFileName, findNextAvailableLog(settings.nextDataLogNumber, "dataLog"));
          beginDataLogging(); //180ms
          if (settings.showHelperText == true) 
            printHelperText(OL_OUTPUT_SERIAL | OL_OUTPUT_SDCARD); //printHelperText to terminal and sensor file
        }
        if (online.serialLogging == true)
        {
          // Check if the current serial file was deleted
          if (sd.exists(serialDataFileName) == false)
            strcpy(serialDataFileName, findNextAvailableLog(settings.nextSerialLogNumber, "serialLog"));
          beginSerialLogging();
        }
      }
    }
    else if (incoming == 'r')
    {
      SerialPrintln(F("\r\nResetting to factory defaults. Press 'y' to confirm: "));
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        EEPROM.erase();
        if (sd.exists("OLA_settings.txt"))
          sd.remove("OLA_settings.txt");
        if (sd.exists("OLA_deviceSettings.txt"))
          sd.remove("OLA_deviceSettings.txt");

        SerialPrint(F("Settings erased. Please reset OpenLog Artemis and open a terminal at "));
        Serial.print((String)settings.serialTerminalBaudRate);
        if (settings.useTxRxPinsForTerminal == true)
          Serial1.print((String)settings.serialTerminalBaudRate);
        SerialPrintln(F("bps..."));
        while (1);
      }
      else
        SerialPrintln(F("Reset aborted"));
    }
    else if (incoming == 'q')
    {
      SerialPrintln(F("\r\nQuit? Press 'y' to confirm:"));
      byte bContinue = getByteChoice(menuTimeout);
      if (bContinue == 'y')
      {
        //Save files before going to sleep
        if (online.dataLogging == true)
        {
          sensorDataFile.sync();
          updateDataFileAccess(&sensorDataFile); // Update the file access time & date
          sensorDataFile.close(); //No need to close files. https://forum.arduino.cc/index.php?topic=149504.msg1125098#msg1125098
        }
        if (online.serialLogging == true)
        {
          if (incomingBufferSpot > 0)
          {
            //Write the remainder of the buffer
            digitalWrite(PIN_STAT_LED, HIGH); //Toggle stat LED to indicating log recording
            serialDataFile.write(incomingBuffer, incomingBufferSpot); //Record the buffer to the card
            digitalWrite(PIN_STAT_LED, LOW);

            incomingBufferSpot = 0;

            lastSeriaLogSyncTime = rtcMillis(); //Reset the last sync time to now
          }

          serialDataFile.sync();
          updateDataFileAccess(&serialDataFile); // Update the file access time & date
          serialDataFile.close();
        }
        SerialPrint(F("Log files are closed. Please reset OpenLog Artemis and open a terminal at "));
        Serial.print((String)settings.serialTerminalBaudRate);
        if (settings.useTxRxPinsForTerminal == true)
          Serial1.print((String)settings.serialTerminalBaudRate);
        SerialPrintln(F("bps..."));
        delay(sdPowerDownDelay); // Give the SD card time to shut down
        powerDownOLA();
      }
      else
        SerialPrintln(F("Quit aborted"));
    }
    else if (incoming == 'x')
      break;
    else if (incoming == STATUS_GETBYTE_TIMEOUT)
      break;
    else
      printUnknown(incoming);
  }

  recordSystemSettings(); //Once all menus have exited, record the new settings to EEPROM and config file

  recordDeviceSettingsToFile(); //Record the current devices settings to device config file

  configureQwiicDevices(); //Reconfigure the qwiic devices in case any settings have changed

  if (restartIMU == true)
    beginIMU(); // Restart the IMU if required

  while (Serial.available()) Serial.read(); //Empty buffer of any newline chars

  if (settings.useTxRxPinsForTerminal == true)
    while (Serial1.available()) Serial1.read(); //Empty buffer of any newline chars

  //Reset measurements
  measurementCount = 0;
  totalCharactersPrinted = 0;
  //If we are sleeping between readings then we cannot rely on millis() as it is powered down
  //Use RTC instead
  measurementStartTime = rtcMillis();

  //Edge case: after 10Hz reading, user sets the log rate above 2s mark. We never go to sleep because 
  //takeReading is not true. And since we don't wake up, takeReading never gets set to true.
  //So we force it here.
  takeReading = true; 
}
