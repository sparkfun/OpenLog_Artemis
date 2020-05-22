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
          Serial.print(F("UBX Class:0x")); //Let's keep this message short!
          Serial.print(ubx_class, HEX);
          Serial.print(F(" ID:0x"));
          Serial.println(ubx_ID, HEX);
        }
        ubx_state = frame_valid;
      }
    }
    break;
  }

  return(true);
}
