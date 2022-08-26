

#pragma once

// Flags for output destinations

#define OL_OUTPUT_SERIAL   	0x1
#define OL_OUTPUT_SDCARD	0x2

void printHelperText(uint8_t);
void getData(char *buffer, size_t lenBuffer);
uint8_t getByteChoice(int numberOfSeconds, bool updateDZSERIAL = false);
void menuMain(bool alwaysOpen = false);
void beginSD(bool silent = false);
void beginIMU(bool silent = false);
