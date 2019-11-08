/*
 * This program is a simple binary write/read benchmark.
 */
#include <SPI.h>
#include "SdFat.h"

// SD chip select pin
const uint8_t chipSelect = 4;

// Size of read/write.
const size_t BUF_SIZE = 512;

// File size in MB where MB = 1,000,000 bytes.
const uint32_t FILE_SIZE_MB = 1;

// Write pass count.
const uint8_t WRITE_COUNT = 1;

// File size in bytes.
const uint32_t FILE_SIZE = 1000000UL*FILE_SIZE_MB;

uint8_t buf[BUF_SIZE];

SdFat sd;
SdFile file;

//------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  
  delay(100);
}
//------------------------------------------------------------------------------
void loop() {
  float s;
  uint32_t t;
  uint32_t maxLatency;
  uint32_t minLatency;
  uint32_t totalLatency;

  // Discard any input.
  while(Serial.available() > 0) Serial.read();

  // F( stores strings in flash to save RAM
  Serial.println(F("Type any character to start\n"));
  while (!Serial.available()) {
    SysCall::yield();
  }
  
  if (!sd.begin(chipSelect, SD_SCK_MHZ(10))) {
    sd.initErrorHalt();
  }

  Serial.printf("Type is FAT%d\n", int(sd.vol()->fatType()));
  Serial.printf("Card size: %lu GB (GB = 1E9 bytes)\n", sd.card()->cardSize()*512E-9);

  // open or create file - truncate existing file.
  if (!file.open("bench.dat", O_RDWR | O_CREAT | O_TRUNC)) {
    Serial.println("open failed");
  }

  // fill buf with known data
  for (size_t i = 0; i < (BUF_SIZE-2); i++) {
    buf[i] = 'A' + (i % 26);
  }
  buf[BUF_SIZE-2] = '\r';
  buf[BUF_SIZE-1] = '\n';

  Serial.printf("File size %d MB\n", FILE_SIZE_MB);
  Serial.printf("Buffer size %d bytes\n", BUF_SIZE);
  Serial.println("Starting write test, please wait.\n");

  uint32_t n = FILE_SIZE/sizeof(buf);

  Serial.println("write speed and latency");
  Serial.println(F("speed,max,min,avg"));
  Serial.println(F("KB/Sec,usec,usec,usec"));

  for (uint8_t nTest = 0; nTest < WRITE_COUNT; nTest++) {
    file.truncate(0);
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      uint32_t m = micros();
      if (file.write(buf, sizeof(buf)) != sizeof(buf)) {
        sd.errorPrint("write failed");
        file.close();
        return;
      }
      m = micros() - m;
      if (maxLatency < m) {
        maxLatency = m;
      }
      if (minLatency > m) {
        minLatency = m;
      }
      totalLatency += m;
    }
    file.sync();
    t = millis() - t;
    s = file.fileSize();
    Serial.printf("%.2f, %d, %d, %d\n", s/t, maxLatency, minLatency, totalLatency/n);
  }

  Serial.println("Done");
  file.close();
}
