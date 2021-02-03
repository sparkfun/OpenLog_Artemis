/*
	Keep everything for ANSI prototypes.
From: http://stackoverflow.com/questions/2607853/why-prototype-is-used-header-files
*/

#ifndef ZMODEM_FIXES_H
#define ZMODEM_FIXES_H

#define SERIAL_TX_BUFFER_SIZE AP3_UART_RINGBUFF_SIZE

////////////////////////////////////////////////////////


#define _PROTOTYPE(function, params) function params

#include <SdFat.h>

extern SdFat sd;

#include <string.h>

// Dylan (monte_carlo_ecm, bitflipper, etc.) - changed serial read/write to macros to try to squeeze
// out higher speed

#define READCHECK
#define TYPICAL_SERIAL_TIMEOUT 1200

extern Stream *ZSERIAL;
extern int Rxtimeout;
#define TIMEOUT (-2)

//#define readline(timeout) ({ char _c; ZSERIAL->readBytes(&_c, 1) > 0 ? _c : TIMEOUT; })
//#define zdlread(void) ({ int _z; ((_z = readline(Rxtimeout)) & 0140) ? _z : zdlread2(_z); })
//#define sendline(_c) ZSERIAL->write(char(_c))
//#define sendline(_c) ({ if (ZSERIAL->availableForWrite() > SERIAL_TX_BUFFER_SIZE / 2) ZSERIAL->flush(); ZSERIAL->write(char(_c)); })
//#define zsendline(_z) ({ (_z & 0140 ) ? sendline(_z) : zsendline2(_z); })

int readline(int timeout);  // Header. Code is in zmodem_zm.cpp

void sendline(int _c); // Header. Code is in zmodem_zm.cpp
#define xsendline(c) sendline(c)

//int zdlread2(int); // Header. Code is in zmodem_zm.cpp

void zsendline(int _z); // Header. Code is in zmodem_zm.cpp

//int zdlread(void)
//{
//  int _z;
//  if ((_z = readline(Rxtimeout)) & 0140)
//    return (_z);
//  else
//    return (zdlread2(_z));
//}

void sendzrqinit(void);
int wctxpn(const char *name);
#define ARDUINO_RECV
//int wcrx(void);
int wcreceive(int argc, char **argp);

extern int Filcnt;

#define register int

// If this is not defined the default is 1024.
// It must be a power of 2

#ifdef ARDUINO_SMALL_MEMORY
#define TXBSIZE 1024
#else
#define TXBSIZE 1024
#endif

#define sleep(x) delay((x)*1000L)
#define signal(x,y)

// Handle the calls to exit - one has SS_NORMAL
#define SS_NORMAL 0
#define exit(n)

// For now, evaluate it to zero so that it does not
// enter the "if" statement's clause
#define setjmp(...)

#define printf(s, ... ) DSERIAL->println(s);
#define fprintf(...)

// fseek(in, Rxpos, 0)
//#define fseek(fd, pos, rel) sdfile->seekSet(pos)
//#define fclose(c)

// ignore calls to mode() function in rbsb.cpp
#define mode(a)

#define sendbrk()

//extern int Fromcu;
void purgeline(void);

#ifndef UNSL
#define UNSL unsigned
#endif

void flushmo(void);

#endif
