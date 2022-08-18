

#pragma once

// Flags for output destinations

#define OL_OUTPUT_SERIAL   	0x1
#define OL_OUTPUT_SDCARD	0x2

void printHelperText(uint8_t);
void getData(char *buffer, size_t lenBuffer);

