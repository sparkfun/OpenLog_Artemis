/*
  Based on: u-blox Example21
  
  Module Info - extracts and prints the full module information from UBX_MON_VER
  using a custom command.
  By: @mayopan
  Date: May 9th, 2020
*/

bool getModuleInfo(uint16_t maxWait)
{
  // Let's create our custom packet
  uint8_t customPayload[MAX_PAYLOAD_SIZE]; // This array holds the payload data bytes

  // The next line creates and initialises the packet information which wraps around the payload
  ubxPacket customCfg = {0, 0, 0, 0, 0, customPayload, 0, 0, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED, SFE_UBLOX_PACKET_VALIDITY_NOT_DEFINED};

  // Referring to the u-blox M8 Receiver Description and Protocol Specification we see that
  // the module information can be read using the UBX-MON-VER message. So let's load our
  // custom packet with the correct information so we can read (poll / get) the module information.

  customCfg.cls = UBX_CLASS_MON; // This is the message Class
  customCfg.id = UBX_MON_VER;    // This is the message ID
  customCfg.len = 0;             // Setting the len (length) to zero let's us poll the current settings
  customCfg.startingSpot = 0;    // Always set the startingSpot to zero (unless you really know what you are doing)

  // Now let's send the command. The module info is returned in customPayload

  if (gpsSensor_ublox.sendCommand(&customCfg, maxWait) != SFE_UBLOX_STATUS_DATA_RECEIVED)
      return(false); //If command send fails then bail

  // Now let's extract the module info from customPayload    

  //Initialise minfo
  minfo.hwVersion[0] = 0;
  minfo.swVersion[0] = 0;
  for (int i = 0; i < 10; i++)
      minfo.extension[i][0] = 0;
  minfo.extensionNo = 0;
  minfo.protVerMajor = -1;
  minfo.protVerMinor = -1;
  minfo.SPG = false;
  minfo.HPG = false;
  minfo.ADR = false;
  minfo.UDR = false;
  minfo.TIM = false;
  minfo.FTS = false;
  minfo.LAP = false;

  uint16_t position = 0;
  for (int i = 0; i < 30; i++)
  {
      minfo.swVersion[i] = customPayload[position];
      position++;
  }
  for (int i = 0; i < 10; i++)
  {
      minfo.hwVersion[i] = customPayload[position];
      position++;
  }

  while (customCfg.len > (position + 40))
  {
      for (int i = 0; i < 30; i++)
      {
          minfo.extension[minfo.extensionNo][i] = customPayload[position];
          position++;
      }

      String ver_str = String((char *)minfo.extension[minfo.extensionNo]); // Convert extension into a String

      //Check for FWVER
      int starts_at = -1;
      int ends_at = -1;
      starts_at = ver_str.indexOf("FWVER="); // See is message contains "FWVER="
      if (starts_at >= 0) { // If it does:
        ends_at = ver_str.indexOf(" ", starts_at); // Find the following " "
        if (ends_at > starts_at) { // If the message contains both "FWVER=" and " "
          String fwver_str = ver_str.substring((starts_at + 6),ends_at); // Extract the text after the "="
          if (fwver_str == "SPG") minfo.SPG = true;
          else if (fwver_str == "HPG") minfo.HPG = true;
          else if (fwver_str == "ADR") minfo.ADR = true;
          else if (fwver_str == "UDR") minfo.UDR = true;
          else if (fwver_str == "TIM") minfo.TIM = true;
          else if (fwver_str == "FTS") minfo.FTS = true;
          else if (fwver_str == "LAP") minfo.FTS = true;
          if (settings.printMajorDebugMessages == true)
          {
            Serial.print(F("getModuleInfo: "));
            Serial.println(ver_str);
          }
        }
      }

      //Check for PROTVER
      starts_at = -1;
      ends_at = -1;
      starts_at = ver_str.indexOf("PROTVER="); // See is message contains "PROTVER="
      if (starts_at >= 0) { // If it does:
        ends_at = ver_str.indexOf(".", starts_at); // Find the following "."
        if (ends_at > starts_at) { // If the message contains both "PROTVER=" and "."
          String protver_str = ver_str.substring((starts_at + 8),ends_at); // Extract the value after the "="
          minfo.protVerMajor = (int)protver_str.toInt(); // Convert it to int
          protver_str = ver_str.substring(ends_at + 1); // Extract the value after the "."
          minfo.protVerMinor = (int)protver_str.toInt(); // Convert it to int
          if (settings.printMajorDebugMessages == true)
          {
            Serial.print(F("getModuleInfo: "));
            Serial.println(ver_str);
          }
        }
      }
      
      minfo.extensionNo++;
      if (minfo.extensionNo > 9)
          break;
  }

  return(true); //Success!
}
