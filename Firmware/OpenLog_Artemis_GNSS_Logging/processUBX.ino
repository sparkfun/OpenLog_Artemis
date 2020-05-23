// processUBX states
#define looking_for_B5          0
#define looking_for_62          1
#define looking_for_class       2
#define looking_for_ID          3
#define looking_for_length_LSB  4
#define looking_for_length_MSB  5
#define processing_payload      6
#define looking_for_checksum_A  7
#define looking_for_checksum_B  8
#define sync_lost               9
#define frame_valid             10
int ubx_state = looking_for_B5;
int ubx_length = 0;
int ubx_class = 0;
int ubx_ID = 0;
int ubx_checksum_A = 0;
int ubx_checksum_B = 0;
int ubx_expected_checksum_A = 0;
int ubx_expected_checksum_B = 0;

// Process incoming data bytes according to ubx_state
// For UBX messages:
// Sync Char 1: 0xB5
// Sync Char 2: 0x62
// Class byte
// ID byte
// Length: two bytes, little endian
// Payload: length bytes
// Checksum: two bytes
// Only allow a new file to be opened when a complete packet has been processed and ubx_state has returned to "looking_for_B5"
// Or when a data error is detected (sync_lost)
bool processUBX(char c)
{
  switch (ubx_state) {
    case (looking_for_B5):
    {
      if (c == 0xB5) // Have we found Sync Char 1 (0xB5) if we were expecting one?
      {
        ubx_state = looking_for_62; // Now look for Sync Char 2 (0x62)
      }
      else
      {
        if (settings.printMajorDebugMessages == true)
        {
          Serial.println(F("processUBX: expecting Sync Char 0xB5 but did not receive one"));
        }
        ubx_state = sync_lost;
      }
    }
    break;
    case (looking_for_62):
    {
      if (c == 0x62) // Have we found Sync Char 2 (0x62) when we were expecting one?
      {
        ubx_expected_checksum_A = 0; // Reset the expected checksum
        ubx_expected_checksum_B = 0;
        ubx_state = looking_for_class; // Now look for Class byte
      }
      else
      {
        if (settings.printMajorDebugMessages == true)
        {
          Serial.println(F("processUBX: expecting Sync Char 0x62 but did not receive one"));
        }
        ubx_state = sync_lost;
      }
    }
    break;
    case (looking_for_class):
    {
      ubx_class = c;
      ubx_expected_checksum_A = ubx_expected_checksum_A + c; // Update the expected checksum
      ubx_expected_checksum_B = ubx_expected_checksum_B + ubx_expected_checksum_A;
      ubx_state = looking_for_ID; // Now look for ID byte
    }
    break;
    case (looking_for_ID):
    {
      ubx_ID = c;
      ubx_expected_checksum_A = ubx_expected_checksum_A + c; // Update the expected checksum
      ubx_expected_checksum_B = ubx_expected_checksum_B + ubx_expected_checksum_A;
      ubx_state = looking_for_length_LSB; // Now look for length LSB
    }
    break;
    case (looking_for_length_LSB):
    {
      ubx_length = c; // Store the length LSB
      ubx_expected_checksum_A = ubx_expected_checksum_A + c; // Update the expected checksum
      ubx_expected_checksum_B = ubx_expected_checksum_B + ubx_expected_checksum_A;
      ubx_state = looking_for_length_MSB; // Now look for length MSB
    }
    break;
    case (looking_for_length_MSB):
    {
      ubx_length = ubx_length + (c * 256); // Add the length MSB
      ubx_expected_checksum_A = ubx_expected_checksum_A + c; // Update the expected checksum
      ubx_expected_checksum_B = ubx_expected_checksum_B + ubx_expected_checksum_A;
      ubx_state = processing_payload; // Now look for payload bytes (length: ubx_length)
    }
    break;
    case (processing_payload):
    {
      //TO DO: extract useful things from the incoming message based on ubx_class, ubx_ID and ubx_length (byte count)
      ubx_length = ubx_length - 1; // Decrement length by one
      ubx_expected_checksum_A = ubx_expected_checksum_A + c; // Update the expected checksum
      ubx_expected_checksum_B = ubx_expected_checksum_B + ubx_expected_checksum_A;
      if (ubx_length == 0)
      {
        ubx_expected_checksum_A = ubx_expected_checksum_A & 0xff; // Limit checksums to 8-bits
        ubx_expected_checksum_B = ubx_expected_checksum_B & 0xff;
        ubx_state = looking_for_checksum_A; // If we have received length payload bytes, look for checksum bytes
      }
    }
    break;
    case (looking_for_checksum_A):
    {
      ubx_checksum_A = c;
      ubx_state = looking_for_checksum_B;
    }
    break;
    case (looking_for_checksum_B):
    {
      ubx_checksum_B = c;
      ubx_state = looking_for_B5; // All bytes received so go back to looking for a new Sync Char 1 unless there is a checksum error
      if ((ubx_expected_checksum_A != ubx_checksum_A) or (ubx_expected_checksum_B != ubx_checksum_B))
      {
        if (settings.printMajorDebugMessages == true)
        {
          Serial.println(F("processUBX: UBX checksum error"));
        }
        ubx_state = sync_lost;
      }
      else
      {
        if (settings.enableTerminalOutput == true)
        {
          //Print some useful information. Let's keep this message short!
          myRTC.getTime(); //Get the RTC time so we can print it
          Serial.printf("%04d/%02d/%02d %02d:%02d:%02d.%02d ", (myRTC.year + 2000), myRTC.month, myRTC.dayOfMonth, myRTC.hour, myRTC.minute, myRTC.seconds, myRTC.hundredths);
          //Print the frame information
          Serial.print(F("UBX Class:0x"));
          Serial.print(ubx_class, HEX);
          Serial.print(F(" ID:0x"));
          if (ubx_ID < 0x10) Serial.print(F("0"));
          Serial.print(ubx_ID, HEX);
          switch (ubx_class)
          {
            case UBX_CLASS_NAV:
            {
              switch (ubx_ID)
              {
                case UBX_NAV_CLOCK:
                  Serial.print(F(" NAV-CLOCK"));
                break;                
                case UBX_NAV_HPPOSECEF:
                  Serial.print(F(" NAV-HPPOSECEF"));
                break;                
                case UBX_NAV_HPPOSLLH:
                  Serial.print(F(" NAV-HPPOSLLH"));
                break;                
                case UBX_NAV_ODO:
                  Serial.print(F(" NAV-ODO"));
                break;                
                case UBX_NAV_POSECEF:
                  Serial.print(F(" NAV-POSECEF"));
                break;                
                case UBX_NAV_POSLLH:
                  Serial.print(F(" NAV-POSLLH"));
                break;                
                case UBX_NAV_PVT:
                  Serial.print(F(" NAV-PVT"));
                break;                
                case UBX_NAV_RELPOSNED:
                  Serial.print(F(" NAV-RELPOSNED"));
                break;                
                case UBX_NAV_STATUS:
                  Serial.print(F(" NAV-STATUS"));
                break;                
                case UBX_NAV_TIMEUTC:
                  Serial.print(F(" NAV-TIMEUTC"));
                break;                
                case UBX_NAV_VELECEF:
                  Serial.print(F(" NAV-VELECEF"));
                break;                
                case UBX_NAV_VELNED:
                  Serial.print(F(" NAV-VELNED"));
                break;                
              }
            }
            break;
            case UBX_CLASS_RXM:
            {
              switch (ubx_ID)
              {
                case UBX_RXM_RAWX:
                  Serial.print(F(" RXM-RAWX"));
                break;
                case UBX_RXM_SFRBX:
                  Serial.print(F(" RXM-SFRBX"));
                break;
              }
            }
            break;
            case UBX_CLASS_TIM:
            {
              switch (ubx_ID)
              {
                case UBX_TIM_TM2:
                  Serial.print(F(" TIM-TM2"));
                break;
              }
            }
            break;
          }
          Serial.println();
        }
        ubx_state = frame_valid;
      }
    }
    break;
  }

  return(true);
}
