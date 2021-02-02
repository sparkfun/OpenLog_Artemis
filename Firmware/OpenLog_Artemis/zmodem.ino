// ecm-bitflipper's Arduino ZModem:
// https://github.com/ecm-bitflipper/Arduino_ZModem

// V2.1.3
// 2020-09-02
//  - Updated for the OLA by Paul Clark
//  - CD, MD, RD, PWD and RZ disabled
//  - added CAT/TYPE

#define ultoa utoa

// Arghhh. These three links have disappeared!
// See this page for the original code:
// http://www.raspberryginger.com/jbailey/minix/html/dir_acf1a49c3b8ff2cb9205e4a19757c0d6.html
// From: http://www.raspberryginger.com/jbailey/minix/html/zm_8c-source.html
// docs at: http://www.raspberryginger.com/jbailey/minix/html/zm_8c.html
// The minix files here might be the same thing:
// http://www.cise.ufl.edu/~cop4600/cgi-bin/lxr/http/source.cgi/commands/zmodem/

#include "zmodem_config.h"
#include "zmodem.h"
#include "zmodem_zm.h"

/*
  Originally was an example by fat16lib of reading a directory
  and listing its files by directory entry number.
See: http://forum.arduino.cc/index.php?topic=173562.0

  Heavily modified by Pete (El Supremo) to recursively list the files
  starting at a specified point in the directory structure and then
  use zmodem to transmit them to the PC via the ZSERIAL port

  Further heavy modifications by Dylan (monte_carlo_ecm, bitflipper, etc.)
  to create a user driven "file manager" of sorts.
  Many thanks to Pete (El Supremo) who got this started.  Much work remained
  to get receive (rz) working, mostly due to the need for speed because of the
  very small (64 bytes) Serial buffer in the Arduino.

  I have tested this with an Arduino Mega 2560 R3 interfacing with Windows 10
  using Hyperterminal, Syncterm and TeraTerm.  All of them seem to work, though
  their crash recovery (partial file transfer restart) behaviours vary.
  Syncterm kicks out a couple of non-fatal errors at the beginning of sending
  a file to the Arduino, but appears to always recover and complete the transfer.

  This sketch should work on any board with at least 30K of flash and 2K of RAM.
  Go to zmodem_config.h and disable some of the ARDUINO_SMALL_MEMORY_* macros
  for maximum peace of mind and stability if you don't need all the features
  (send, receive and file management).

V2.1.2
2018-05-11
  - Fixes for Arduino IDE 1.8.5
  - Attempted to patch for use on Teensy

V2.1
2015-03-06
  - Large scale code clean-up, reduction of variable sizes where they were
    unnecessarily large, sharing variables previously unshared between sz and
    rz, and creative use of the send/receive buffer allowed this sketch to
    BARELY fit and run with all features enabled on a board with 30K flash and
    2K of RAM.  Uno & Nano users - enjoy.
  - Some boards were unstable at baud rates above 9600.  I tracked this back
    to overrunning the SERIAL_TX_BUFFER_SIZE to my surprise.  Added a check
    if a flush() is required both in the help and directory listings, as well
    as the sendline() macro.

V2.0
2015-02-23
  - Taken over by Dylan (monte_carlo_ecm, bitflipper, etc.)
  - Added Serial based user interface
  - Added support for SparkFun MP3 shield based SDCard (see zmodem_config.h)
  - Moved CRC tables to PROGMEM to lighten footprint on dynamic memory (zmodem_crc16.cpp)
  - Added ZRQINIT at start of sz.  All terminal applications I tested didn't strictly need it, but it's
    super handy for getting the terminal application to auto start the download
  - Completed adaptation of rz to Arduino
  - Removed directory recursion for sz in favour of single file or entire current directory ("*") for sz
  - Optimized zdlread, readline, zsendline and sendline
      into macros for rz speed - still only up to 57600 baud
  - Enabled "crash recovery" for both sz and rz.  Various terminal applications may respond differently
      to restarting partially completed transfers; experiment with yours to see how it behaves.  This
      feature could be particularly useful if you have an ever growing log file and you just need to
      download the entries since your last download from your Arduino to your computer.

V1.03
140913
  - remove extraneous code such as the entire main() function
    in sz and rz and anything dependent on the vax, etc.
  - moved purgeline, sendline, readline and bttyout from rz to zm
    so that the the zmodem_rz.cpp file is not required when compiling
    sz 
    
V1.02
140912
  - yup, sz transfer still works.
    10 files -- 2853852 bytes
    Time = 265 secs
    
V1.01
140912
This was originally working on a T++2 and now works on T3
  - This works on a T3 using the RTC/GPS/uSD breadboard
    It sent multiple files - see info.h
  - both rz and sz sources compile together here but have not
    yet ensured that transmit still works.
    
V1.00
130630
  - it compiles. It even times out. But it doesn't send anything
    to the PC - the TTYUSB LEDs don't blink at all
  - ARGHH. It does help to open the Serial1 port!!
  - but now it sends something to TTerm but TTerm must be answering
    with a NAK because they just repeat the same thing over
    and over again.

V2.00
130702
  - IT SENT A FILE!!!!
    It should have sent two, but I'll take it!
  - tried sending 2012/09 at 115200 - it sent the first file (138kB!)
    but hangs when it starts on the second one. The file is created
    but is zero length.
    
  - THIS VERSION SENDS MULTIPLE FILES

*/

#define error(s) printDebug(s)

extern int Filesleft;
extern long Totalleft;

extern SdFile fout;

static bool oneTime = false; // Display the Tera Term Change Directory note only once (so it does not get on people's nerves!)

// Dylan (monte_carlo_ecm, bitflipper, etc.) - This function was added because I found
// that SERIAL_TX_BUFFER_SIZE was getting overrun at higher baud rates.  This modified
// Serial.print() function ensures we are not overrunning the buffer by flushing if
// it gets more than half full.

size_t DSERIALprint(const __FlashStringHelper *ifsh)
{
  PGM_P p = reinterpret_cast<PGM_P>(ifsh);
  size_t n = 0;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (c == 0) break;
    if (DSERIAL->availableForWrite() > SERIAL_TX_BUFFER_SIZE / 2) DSERIAL->flush();
    if (DSERIAL->write(c)) n++;
    else break;
  }
  return n;
}

#define DSERIALprintln(_p) ({ DSERIALprint(_p); DSERIAL->write("\r\n"); })

void sdCardHelp(void)
{
  DSERIALprint(F("\r\n"));
  DSERIALprint(Progname);
  DSERIALprint(F(" - Transfer rate: "));
  DSERIAL->flush(); DSERIAL->println(settings.serialTerminalBaudRate); DSERIAL->flush();
  DSERIALprintln(F("Available Commands:")); DSERIAL->flush();
  DSERIALprintln(F("HELP     - Print this list of commands")); DSERIAL->flush();
  DSERIALprintln(F("DIR      - List files in current working directory - alternate LS")); DSERIAL->flush();
  DSERIALprintln(F("DEL file - Delete file - alternate RM")); DSERIAL->flush();
  DSERIALprintln(F("SZ  file - Send file from OLA to terminal using ZModem (\"SZ *\" will send all files)")); DSERIAL->flush();
  DSERIALprintln(F("SS  file - Send file from OLA using serial TX pin")); DSERIAL->flush();
  DSERIALprintln(F("CAT file - Type file to this terminal - alternate TYPE")); DSERIAL->flush();
  DSERIALprintln(F("X        - Exit to OpenLog Artemis Main Menu")); DSERIAL->flush();
  DSERIALprint(F("\r\n"));
}

SdFile root; // Copied from SdFat OpenNext example
SdFile fout;
//dir_t *dir ;

int count_files(int *file_count, long *byte_count)
{
  *file_count = 0;
  *byte_count = 0;

  if (!root.open("/"))
    return 0;

  root.rewind();

  while (fout.openNext(&root, O_RDONLY))
  {
    // read next directory entry in current working directory
    if (!fout.isDir())
    {
      *file_count = *file_count + 1;
      *byte_count = *byte_count + fout.fileSize();
    }
    fout.close();
  }

  root.close();
  
  return 0;
}

void sdCardMenu(void)
{
  sdCardHelp(); // Display the help
  
  bool keepGoing = true;
  
  while (keepGoing)
  {
    char *cmd = oneKbuf;
    char *param;
  
    *cmd = 0;

    while (DSERIAL->available()) DSERIAL->read();
    
    char c = 0;
    while(1)
    {
      if (DSERIAL->available() > 0)
      {
        c = DSERIAL->read();
        if ((c == 8 or c == 127) && strlen(cmd) > 0) cmd[strlen(cmd)-1] = 0;
        if (c == '\n' || c == '\r') break;
        DSERIAL->write(c);
        if (c != 8 && c != 127) strncat(cmd, &c, 1);
      }
      else
      {
        // Dylan (monte_carlo_ecm, bitflipper, etc.) -
        // This delay is required because I found that if I hard loop with DSERIAL.available,
        // in certain circumstances the Arduino never sees a new character.  Various forum posts
        // seem to confirm that a short delay is required when using this style of reading
        // from Serial
        checkBattery();
        delay(1);
      }
    }
     
    param = strchr(cmd, 32);
    if (param > 0)
    {
      *param = 0;
      param = param + 1;
    }
    else
    {
      param = &cmd[strlen(cmd)];
    }
  
    strupr(cmd);
    DSERIAL->println();
  //  DSERIALprintln(command);
  //  DSERIALprintln(parameter);
  
    if (!strcmp_P(cmd, PSTR("HELP")))
    {     
      sdCardHelp();
    }
    
    else if (!strcmp_P(cmd, PSTR("DIR")) || !strcmp_P(cmd, PSTR("LS"))) // DIRectory
    {
      DSERIALprintln(F("\r\nRoot Directory Listing:"));

      if (DSERIAL == &Serial)
        sd.ls("/", LS_DATE | LS_SIZE); // Do a non-recursive LS of the root directory showing file modification dates and sizes
      else
        sd.ls(&SerialLog, "/", LS_DATE | LS_SIZE);
  
      DSERIALprintln(F("End of Directory\r\n"));
    }
    
    else if (!strcmp_P(cmd, PSTR("DEL")) || !strcmp_P(cmd, PSTR("RM"))) // ReMove / DELete file
    {
      if (!sd.remove(param))
      {
        DSERIALprint(F("\r\nFailed to delete file "));
        DSERIAL->flush(); DSERIAL->println(param); DSERIAL->flush();
        DSERIALprintln(F("\r\n"));
      }
      else
      {
        DSERIALprint(F("\r\nFile "));
        DSERIAL->flush(); DSERIAL->print(param); DSERIAL->flush();
        DSERIALprintln(F(" deleted\r\n"));
      }
    }

    else if (!strcmp_P(cmd, PSTR("SZ"))) // Send file(s) using zmodem
    {
      if (!strcmp_P(param, PSTR("*")))
      {
       
        count_files(&Filesleft, &Totalleft);
        DSERIALprint(F("\r\nTransferring ")); DSERIAL->print(Filesleft); DSERIALprint(F(" files (")); DSERIAL->print(Totalleft); DSERIALprintln(F(" bytes)")); 
        
        root.open("/"); // (re)open the root directory
        root.rewind(); // rewind

        if (Filesleft > 0)
        {
          DSERIALprint(F("Starting zmodem transfer in ")); DSERIAL->print(settings.zmodemStartDelay); DSERIALprintln(F(" seconds..."));
          DSERIALprintln(F("(If you are using Tera Term, you need to start your File\\Transfer\\ZMODEM\\Receive now!)"));
          if (oneTime == false)
          {
            DSERIALprintln(F("(Also, if you are using Tera Term, you need to change the directory to something sensible."));
            DSERIALprintln(F(" Use File\\Change directory... to change where the received files are stored.)"));
            oneTime = true;
          }
          for (int i = 0; i < (((int)settings.zmodemStartDelay) * 1000); i++)
          {
            checkBattery();
            delay(1);
          }  
          
          sendzrqinit();
          for (int i = 0; i < 200; i++)
          {
            checkBattery();
            delay(1);
          }  
          
          //while (sd.vwd()->readDir(dir) == sizeof(*dir)) {
          while (fout.openNext(&root, O_RDONLY))
          {
            // read next directory entry in current working directory
            if (!fout.isDir()) {
              char fname[30];
              size_t fsize = 30;
              fout.getName(fname, fsize);
              //DSERIAL->print("fname: "); DSERIAL->println(fname);
              if (wcs(fname) == ERROR)
              {
                for (int i = 0; i < 500; i++)
                {
                  checkBattery();
                  delay(1);
                }  
                fout.close();
                break;
              }
              else
              {
                for (int i = 0; i < 500; i++)
                {
                  checkBattery();
                  delay(1);
                }  
              }
            }
            fout.close();
          }
          saybibi();
          DSERIALprintln(F("\r\nzmodem transfer complete!\r\n"));
        }
        else
        {
          DSERIALprintln(F("\r\nNo files found to send\r\n"));
        }

        root.close();
      }
      else
      {
        if (!fout.open(param, O_READ))
        {
          DSERIALprintln(F("\r\nfile.open failed!\r\n"));
        }
        else
        {
          DSERIALprint(F("\r\nStarting zmodem transfer in ")); DSERIAL->print(settings.zmodemStartDelay); DSERIALprintln(F(" seconds..."));
          DSERIALprintln(F("(If you are using Tera Term, you need to start your File\\Transfer\\ZMODEM\\Receive now!)"));
          if (oneTime == false)
          {
            DSERIALprintln(F("(Also, if you are using Tera Term, you need to change the directory to something sensible."));
            DSERIALprintln(F(" Use File\\Change directory... to change where the received files are stored.)"));
            oneTime = true;
          }
          for (int i = 0; i < (((int)settings.zmodemStartDelay) * 1000); i++)
          {
            checkBattery();
            delay(1);
          }  
            
          // Start the ZMODEM transfer
          Filesleft = 1;
          Totalleft = fout.fileSize();
          //ZSERIAL->print(F("rz\r"));
          sendzrqinit();
          for (int i = 0; i < 200; i++)
          {
            checkBattery();
            delay(1);
          }  
          wcs(param);
          saybibi();
          fout.close();
          DSERIALprintln(F("\r\nzmodem transfer complete!\r\n"));
        }
      }
    }

    else if (!strcmp_P(cmd, PSTR("SS"))) // Send file to serial TX pin
    {
      if (!fout.open(param, O_READ))
      {
        DSERIALprintln(F("\r\nfile.open failed!\r\n"));
      }
      else
      {
        settings.logA12 = false; //Disable analog readings on TX pin
        if (settings.useTxRxPinsForTerminal == false)
          SerialLog.begin(settings.serialLogBaudRate); // (Re)start the serial port

        DSERIALprint(F("\r\nSending ")); DSERIAL->print(param); DSERIALprint(F(" to the TX pin at ")); DSERIAL->print(settings.serialLogBaudRate); DSERIALprintln(F(" baud"));

        while (fout.available())
        {
          char ch;
          if (fout.read(&ch, 1) == 1) // Read a single char
            SerialLog.write(ch); // Send it via SerialLog (TX pin)
        }
        
        fout.close();
        DSERIALprintln(F("\r\nFile sent!\r\n"));
      }
    }

    else if (!strcmp_P(cmd, PSTR("CAT")) || !strcmp_P(cmd, PSTR("TYPE"))) // concatenate / type file to the terminal
    {
      if (!fout.open(param, O_READ))
      {
        DSERIALprintln(F("\r\nfile.open failed!\r\n"));
      }
      else
      {
        DSERIALprint(F("\r\n"));

        while (fout.available())
        {
          char ch;
          if (fout.read(&ch, 1) == 1) // Read a single char
            DSERIAL->write(ch); // Send it via SerialLog (TX pin)
        }
        
        fout.close();
        DSERIALprintln(F("\r\n"));
      }
    }

    else if (!strcmp_P(cmd, PSTR("X"))) // eXit to main menu
    {
      keepGoing = false;
    }
  }
}
