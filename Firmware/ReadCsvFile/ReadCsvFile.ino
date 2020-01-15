#include "SdFat.h"

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = 10;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
#endif  // HAS_SDIO_CLASS

#if SD_FAT_TYPE == 0
SdFat sd;
File file;
#elif SD_FAT_TYPE == 1
SdFat32 sd;
File32 file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

char line[50];

uint8_t test1 = 1;
float test2 = 0.1;

// Check for extra characters in field or find minus sign.
char* skipSpace(char* str) {
  while (isspace(*str)) str++;
  return str;
}

//Convert a given line from file into a settingName and value
//Sets the setting if the name is known
bool parseLine(char* str) {
  char* ptr;

  // Set strtok start of line.
  str = strtok(str, "=");
  if (!str) return false;

  //Store this setting name
  char settingName[30];
  sprintf(settingName, "%s", str);

  //Move pointer to end of line
  str = strtok(nullptr, "\n");
  if (!str) return false;

  // Convert string to double.
  double d = strtod(str, &ptr);
  if (str == ptr || *skipSpace(ptr)) return false;

  // Get setting name
  if (strcmp(settingName, "sizeOfSettings") == 0)
    test1 = d;
  else if (strcmp(settingName, "test") == 0)
    test2 = d;
  else
  {
    Serial.print("Unknown setting: ");
    Serial.println(str);
  }
  
  return (true);
}
//------------------------------------------------------------------------------

#define SD_POWER 23

void setup() {
  Serial.begin(115200);
  delay(10);

  //Power up SD
  pinMode(SD_POWER, OUTPUT);
  digitalWrite(SD_POWER, HIGH);

  // Initialize the SD.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
    return;
  }

  if (!file.open("OLA_settings.cfg", FILE_READ)) {
    Serial.println("open failed");
  }


  while (file.available()) {
    int n = file.fgets(line, sizeof(line));
    if (n <= 0) {
      Serial.println("fgets failed");
    }
    if (line[n - 1] != '\n' && n == (sizeof(line) - 1)) {
      Serial.println("line too long");
    }
    if (parseLine(line) == false) {
      Serial.print("fail: ");
      Serial.println(line);
    }
  }
  file.close();

  Serial.print("test1: ");
  Serial.println(test1);
  Serial.print("test2: ");
  Serial.println(test2);


  Serial.println(F("Done"));
}

void loop() {
}
