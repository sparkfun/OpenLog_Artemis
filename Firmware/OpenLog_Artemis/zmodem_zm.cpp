// See this page for all code: http://www.raspberryginger.com/jbailey/minix/html/dir_acf1a49c3b8ff2cb9205e4a19757c0d6.html
// From: http://www.raspberryginger.com/jbailey/minix/html/zm_8c-source.html
// docs at: http://www.raspberryginger.com/jbailey/minix/html/zm_8c.html

// Look at all the files:
// http://www.raspberryginger.com/jbailey/minix/html/files.html

#ifndef ZMODEM_ZM_CPP
#define ZMODEM_ZM_CPP

/*
 *   Z M . C
 *    ZMODEM protocol primitives
 *    05-09-88  Chuck Forsberg Omen Technology Inc
 *
 * Entry point Functions:
 *      zsbhdr(type, hdr) send binary header
 *      zshhdr(type, hdr) send hex header
 *      zgethdr(hdr, eflag) receive header - binary or hex
 *      zsdata(buf, len, frameend) send data
 *      zrdata(buf, len) receive data
 *      stohdr(pos) store position data in Txhdr
 *      long rclhdr(hdr) recover position offset from header
 */

#ifdef ARDUINO
#include "zmodem_fixes.h"
#include "zmodem.h"
#include "zmodem_crc16.cpp"
#include "zmodem_zm.h"
#else
#ifndef CANFDX
#include "zmodem.h"
#endif
#endif

int zdlread(void); // Header. Code is below.
int zdlread2(int c); // Header. Code is below.

// Shared globals
long Bytesleft; // from rz - Shared with sz bytcnt
long rxbytes;   // from rz - Shared with sz Lrxpos
int Blklen;     // from rz - Shared with sz blklen

//#define Rxtimeout 100            /* Tenths of seconds to wait for something */
int Rxtimeout = 100;            /* Tenths of seconds to wait for something */
#define Verbose 0

// This buffer blends Txb (from sz) and secbuf (from rz) into a single buffer, saving 1K
// of memory.
char oneKbuf[1025];

/* Globals used by ZMODEM functions */
uint8_t Rxframeind;         /* ZBIN ZBIN32, or ZHEX type of frame received */
uint8_t Rxtype;             /* Type of header received */
int Rxcount;            /* Count of data bytes received */
char Rxhdr[4];          /* Received header */
char Txhdr[4];          /* Transmitted header */
long Rxpos;             /* Received file position */
long Txpos;             /* Transmitted file position */
int8_t Txfcs32;            /* TRUE means send binary frames with 32 bit FCS */
int8_t Crc32t;             /* Display flag indicating 32 bit CRC being sent */
int8_t Crc32;              /* Display flag indicating 32 bit CRC being received */
//int Znulls;             /* Number of nulls to send at beginning of ZDATA hdr */
char Attn[ZATTNLEN+1];  /* Attention string rx sends to tx on err */

char zconv;             /* ZMODEM file conversion request */
char zmanag;            /* ZMODEM file management request */
char ztrans;            /* ZMODEM file transport request */
uint8_t Zctlesc;            /* Encode control characters */
#define Zrwindow 1400    /* RX window size (controls garbage count) */
//int Nozmodem = 0;       /* If invoked as "rb" */
int lastsent;           /* Last char we sent */
uint8_t Not8bit;            /* Seven bits seen on header */
//char Lzmanag;           /* Local ZMODEM file management request */
//int Restricted = 0;       /* restricted; no /.. or ../ in filenames */
//int Quiet=0;            /* overrides logic that would otherwise set verbose */
uint8_t Eofseen;            /* EOF seen on input set by zfilbuf */


uint8_t firstsec;
char Lastrx;
char Crcflg;
uint8_t errors;

// Dylan (monte_carlo_ecm, bitflipper, etc.) - Removing this for release to save
// memory, only used for debugging
/*
char *frametypes[] = {
  (char *)"Carrier Lost",         // -3
  (char *)"TIMEOUT",              // -2
  (char *)"ERROR",                // -1
#define FTOFFSET 3
  (char *)"ZRQINIT",
  (char *)"ZRINIT",
  (char *)"ZSINIT",
  (char *)"ZACK",
  (char *)"ZFILE",
  (char *)"ZSKIP",
  (char *)"ZNAK",
  (char *)"ZABORT",
  (char *)"ZFIN",
  (char *)"ZRPOS",
  (char *)"ZDATA",
  (char *)"ZEOF",
  (char *)"ZFERR",
  (char *)"ZCRC",
  (char *)"ZCHALLENGE",
  (char *)"ZCOMPL",
  (char *)"ZCAN",
  (char *)"ZFREECNT",
  (char *)"ZCOMMAND",
  (char *)"ZSTDERR",
  (char *)"xxxxx"
#define FRTYPES 22      // Total number of frame types in this array
  //  not including psuedo negative entries
};
*/
#define badcrc F("Bad CRC");





/* Send ZMODEM binary header hdr of type type */
void zsbhdr(int type, char *hdr)
{


  vfile(F("zsbhdr: %s %lx"), frametypes[type+FTOFFSET], rclhdr(hdr));
/*  if (type == ZDATA)
    for (n = Znulls; --n >=0; )
      xsendline(0);
*/
  xsendline(ZPAD);
  xsendline(ZDLE);
//Pete (El Supremo) This looks wrong but it is correct - the code fails if == is used
  if ((Crc32t = Txfcs32)) {
    int n;
    UNSL long crc;

    xsendline(ZBIN32);
    zsendline(type);
    crc = 0xFFFFFFFFL;
    crc = UPDC32(type, crc);

    for (n=4; --n >= 0; ++hdr) {
      crc = UPDC32((0377 & *hdr), crc);
      zsendline(*hdr);
    }
    crc = ~crc;
    for (n=4; --n >= 0;) {
      zsendline((int)crc);
      crc >>= 8;
    }
  } else {
    int n;
    unsigned short crc;

    xsendline(ZBIN);
    zsendline(type);
    crc = updcrc(type, 0);

    for (n=4; --n >= 0; ++hdr) {
      zsendline(*hdr);
      crc = updcrc((0377& *hdr), crc);
    }
    crc = updcrc(0,updcrc(0,crc));
    zsendline(crc>>8);
    zsendline(crc);
  }
  if (type != ZDATA)
    flushmo();
}


/* Send ZMODEM HEX header hdr of type type */
void zshhdr(int type,char *hdr)
{
  int n;
  unsigned short crc;

  vfile(F("zshhdr: %s %lx"), frametypes[type+FTOFFSET], rclhdr(hdr));
  sendline(ZPAD);
  sendline(ZPAD);
  sendline(ZDLE);
  sendline(ZHEX);
  zputhex(type);
  Crc32t = 0;

  crc = updcrc(type, 0);
  for (n=4; --n >= 0; ++hdr) {
    zputhex(*hdr);
    crc = updcrc((0377 & *hdr), crc);
  }
  crc = updcrc(0,updcrc(0,crc));
  zputhex(crc>>8);
  zputhex(crc);

  /* Make it printable on remote machine */
  sendline(015);
  sendline(0212);
  /*
         * Uncork the remote in case a fake XOFF has stopped data flow
   */
  if (type != ZFIN && type != ZACK)
    sendline(021);
  flushmo();
}


/*
 * Send binary array buf of length length, with ending ZDLE sequence frameend
 */
/*
static char *Zendnames[] = {
  (char *)"ZCRCE",
  (char *)"ZCRCG",
  (char *)"ZCRCQ",
  (char *)"ZCRCW"
};
*/

void zsdata(char *buf,int length,int frameend)
{

  vfile(F("zsdata: %d %s"), length, Zendnames[(frameend-ZCRCE)&3]);
  if (Crc32t) {
    int c;
    UNSL long crc;

    crc = 0xFFFFFFFFL;
    for (;--length >= 0; ++buf) {
      c = *buf & 0377;
      if (c & 0140)
        xsendline(lastsent = c);
      else
        zsendline(c);
      crc = UPDC32(c, crc);
    }
    xsendline(ZDLE);
    xsendline(frameend);
    crc = UPDC32(frameend, crc);

    crc = ~crc;
    for (length=4; --length >= 0;) {
      zsendline((int)crc);
      crc >>= 8;
    }
  } else {
    unsigned short crc;

    crc = 0;
    for (;--length >= 0; ++buf) {
      zsendline(*buf);
      crc = updcrc((0377 & *buf), crc);
    }
    xsendline(ZDLE);
    xsendline(frameend);
    crc = updcrc(frameend, crc);

    crc = updcrc(0,updcrc(0,crc));
    zsendline(crc>>8);
    zsendline(crc);
  }
  if (frameend == ZCRCW) {
    xsendline(XON);
    flushmo();
  }
}

/*
 * Receive array buf of max length with ending ZDLE sequence
 *  and CRC.  Returns the ending character or error code.
 *  NB: On errors may store length+1 bytes!
 */
int zrdata(char *buf,int length)
{
  int c;
  char *end;
  int d;

  if (Rxframeind == ZBIN32) {
    UNSL long crc;

    crc = 0xFFFFFFFFL;
    Rxcount = 0;
    end = buf + length;
    while (buf <= end) {
      if ((c = zdlread()) & ~0377) {
  crcfoo32:
        switch (c) {
        case GOTCRCE:
        case GOTCRCG:
        case GOTCRCQ:
        case GOTCRCW:
          d = c;
          c &= 0377;
          crc = UPDC32(c, crc);
          if ((c = zdlread()) & ~0377)
            goto crcfoo32;
          crc = UPDC32(c, crc);
          if ((c = zdlread()) & ~0377)
            goto crcfoo32;
          crc = UPDC32(c, crc);
          if ((c = zdlread()) & ~0377)
            goto crcfoo32;
          crc = UPDC32(c, crc);
          if ((c = zdlread()) & ~0377)
            goto crcfoo32;
          crc = UPDC32(c, crc);
          if (crc != 0xDEBB20E3) {
            zperr(badcrc);
            return ERROR;
          }
          Rxcount = length - (end - buf);
          vfile(F("zrdat32: %d %s"), Rxcount,
          Zendnames[(d-GOTCRCE)&3]);
          return d;
        case GOTCAN:
          zperr("Sender Canceled");
          return ZCAN;
        case TIMEOUT:
          zperr("TIMEOUT");
          return c;
        default:
          zperr("Bad data subpacket");
          return c;
        }
      }
      *buf++ = c;
      crc = UPDC32(c, crc);
    }
    zperr("Data subpacket too long");
    return ERROR;
  } else {
    unsigned short crc;

    crc = Rxcount = 0;
    end = buf + length;
    while (buf <= end) {
      if ((c = zdlread()) & ~0377) {
  crcfoo16:
        switch (c) {
        case GOTCRCE:
        case GOTCRCG:
        case GOTCRCQ:
        case GOTCRCW:
          crc = updcrc((d=c)&0377, crc);
          if ((c = zdlread()) & ~0377)
            goto crcfoo16;
          crc = updcrc(c, crc);
          if ((c = zdlread()) & ~0377)
            goto crcfoo16;
          crc = updcrc(c, crc);
          if (crc & 0xFFFF) {
            zperr(badcrc);
            return ERROR;
          }
          Rxcount = length - (end - buf);
          vfile(F("zrdata: %d  %s"), Rxcount,
          Zendnames[(d-GOTCRCE)&3]);
          return d;
        case GOTCAN:
          zperr("Sender Canceled");
          return ZCAN;
        case TIMEOUT:
          zperr("TIMEOUT");
          return c;
        default:
          zperr("Bad data subpacket");
          return c;
        }
      }
      *buf++ = c;
      crc = updcrc(c, crc);
    }
    zperr("Data subpacket too long");
    return ERROR;
  }
}

/*
 * Read a ZMODEM header to hdr, either binary or hex.
 *  eflag controls local display of non zmodem characters:
 *      0:  no display
 *      1:  display printing characters only
 *      2:  display all non ZMODEM characters
 *  On success, set Zmodem to 1, set Rxpos and return type of header.
 *   Otherwise return negative on error.
 *   Return ERROR instantly if ZCRCW sequence, for fast error recovery.
 */
int zgethdr(char *hdr,int eflag)
{
  int c, n, cancount;

  n = Zrwindow; //+ Baudrate;        /* Max bytes before start of frame */
  Rxframeind = Rxtype = 0;

startover:
  cancount = 5;
again:
  /* Return immediate ERROR if ZCRCW sequence seen */
  ZSERIAL->setTimeout(Rxtimeout * 100);
  c = readline(Rxtimeout);
  ZSERIAL->setTimeout(TYPICAL_SERIAL_TIMEOUT);

  switch (c) {
  case RCDO:
  case TIMEOUT:
    goto fifi;
  case CAN:
gotcan:
    if (--cancount <= 0) {
      c = ZCAN;
      goto fifi;
    }
    switch (c = readline(1)) {
    case TIMEOUT:
      goto again;
    case ZCRCW:
      c = ERROR;
      /* **** FALL THRU TO **** */
    case RCDO:
      goto fifi;
    default:
      break;
    case CAN:
      if (--cancount <= 0) {
        c = ZCAN;
        goto fifi;
      }
      goto again;
    }
    /* **** FALL THRU TO **** */
  default:
agn2:
    if ( --n == 0) {
      zperr("Garbage count exceeded");
      return(ERROR);
    }
    if (eflag && ((c &= 0177) & 0140))
      bttyout(c);
    else if (eflag > 1)
      bttyout(c);
#ifdef UNIX
    fflush(stderr);
#endif
    goto startover;
  case ZPAD|0200:         /* This is what we want. */
    Not8bit = c;
  case ZPAD:              /* This is what we want. */
    break;
  }
  cancount = 5;
splat:
  switch (c = noxrd7()) {
  case ZPAD:
    goto splat;
  case RCDO:
  case TIMEOUT:
    goto fifi;
  default:
    goto agn2;
  case ZDLE:              /* This is what we want. */
    break;
  }

  switch (c = noxrd7()) {
  case RCDO:
  case TIMEOUT:
    goto fifi;
  case ZBIN:
    Rxframeind = ZBIN;
    Crc32 = FALSE;
    c =  zrbhdr(hdr);
    break;
  case ZBIN32:
    Crc32 = Rxframeind = ZBIN32;
    c =  zrbhdr32(hdr);
    break;
  case ZHEX:
    Rxframeind = ZHEX;
    Crc32 = FALSE;
    c =  zrhhdr(hdr);
    break;
  case CAN:
    goto gotcan;
  default:
    goto agn2;
  }
  Rxpos = hdr[ZP3] & 0377;
  Rxpos = (Rxpos<<8) + (hdr[ZP2] & 0377);
  Rxpos = (Rxpos<<8) + (hdr[ZP1] & 0377);
  Rxpos = (Rxpos<<8) + (hdr[ZP0] & 0377);
fifi:

  switch (c) {
  case GOTCAN:
    c = ZCAN;
    /* **** FALL THRU TO **** */
  case ZNAK:
  case ZCAN:
  case ERROR:
  case TIMEOUT:
  case RCDO:
//    zperr("Got %s", frametypes[c+FTOFFSET]);
    /* **** FALL THRU TO **** */
//  default:
//    if (c >= -3 && c <= FRTYPES)
//      vfile(F("zgethdr: %s %lx"), frametypes[c+FTOFFSET], Rxpos);
//    else
//      vfile(F("zgethdr: %d %lx"), c, Rxpos);
break;
  }
  return c;
}

//#endif

/* Receive a binary style header (type and position) */
int zrbhdr(char *hdr)
{
  int c, n;
  unsigned short crc;

  if ((c = zdlread()) & ~0377)
    return c;
  Rxtype = c;
  crc = updcrc(c, 0);

  for (n=4; --n >= 0; ++hdr) {
    if ((c = zdlread()) & ~0377)
      return c;
    crc = updcrc(c, crc);
    *hdr = c;
  }
  if ((c = zdlread()) & ~0377)
    return c;
  crc = updcrc(c, crc);
  if ((c = zdlread()) & ~0377)
    return c;
  crc = updcrc(c, crc);
  if (crc & 0xFFFF) {
    zperr(badcrc);
    return ERROR;
  }
#ifdef ZMODEM
  Protocol = ZMODEM;
#endif
//  Zmodem = 1;
  return Rxtype;
}



/* Receive a binary style header (type and position) with 32 bit FCS */
int zrbhdr32(char *hdr)
{
  int c, n;
  UNSL long crc;

  if ((c = zdlread()) & ~0377)
    return c;
  Rxtype = c;
  crc = 0xFFFFFFFFL;
  crc = UPDC32(c, crc);
#ifdef DEBUGZ
  vfile(F("zrbhdr32 c=%X  crc=%lX"), c, crc);
#endif

  for (n=4; --n >= 0; ++hdr) {
    if ((c = zdlread()) & ~0377)
      return c;
    crc = UPDC32(c, crc);
    *hdr = c;
#ifdef DEBUGZ
    vfile(F("zrbhdr32 c=%X  crc=%lX"), c, crc);
#endif
  }
  for (n=4; --n >= 0;) {
    if ((c = zdlread()) & ~0377)
      return c;
    crc = UPDC32(c, crc);
#ifdef DEBUGZ
    vfile(F("zrbhdr32 c=%X  crc=%lX"), c, crc);
#endif
  }
  if (crc != 0xDEBB20E3) {
    zperr(badcrc);
    return ERROR;
  }
#ifdef ZMODEM
  Protocol = ZMODEM;
#endif
//  Zmodem = 1;
  return Rxtype;
}




/* Receive a hex style header (type and position) */
int zrhhdr(char *hdr)
{
  int c;
  unsigned short crc;
  int n;

  if ((c = zgethex()) < 0)
    return c;
  Rxtype = c;
  crc = updcrc(c, 0);

  for (n=4; --n >= 0; ++hdr) {
    if ((c = zgethex()) < 0)
      return c;
    crc = updcrc(c, crc);
    *hdr = c;
  }
  if ((c = zgethex()) < 0)
    return c;
  crc = updcrc(c, crc);
  if ((c = zgethex()) < 0)
    return c;
  crc = updcrc(c, crc);
  if (crc & 0xFFFF) {
    zperr(badcrc);
    return ERROR;
  }
  switch ( c = readline(1)) {
  case 0215:
    Not8bit = c;
    /* **** FALL THRU TO **** */
  case 015:
    /* Throw away possible cr/lf */
    switch (c = readline(1)) {
    case 012:
      Not8bit |= c;
    }
  }
#ifdef ZMODEM
  Protocol = ZMODEM;
#endif
//  Zmodem = 1;
  return Rxtype;
}

/* Send a byte as two hex digits */
/*void zputhex(int c)
{
  static char     digits[]  = "0123456789abcdef";

  if (Verbose>8)
    vfile(F("zputhex: %02X"), c);
  sendline(digits[(c&0xF0)>>4]);
  sendline(digits[(c)&0xF]);
} */

PROGMEM static const char digits[17] = "0123456789abcdef";

void zputhex(int c)
{
//  static char     digits[]  = "0123456789abcdef";

  if (Verbose>8)
    vfile(F("zputhex: %02X"), c);
  sendline(pgm_read_byte(digits+((c&0xF0)>>4)));
  sendline(pgm_read_byte(digits+((c)&0xF)));
}

/*
 * Send character c with ZMODEM escape sequence encoding.
 *  Escape XON, XOFF. Escape CR following @ (Telenet net escape)
 */
int zsendline2(int c)
{

  /* Quick check for non control characters */
  if (c & 0140)
    xsendline(lastsent = c);
  else {
    switch (c &= 0377) {
    case ZDLE:
      xsendline(ZDLE);
      xsendline (lastsent = (c ^= 0100));
      break;
    case 015:
    case 0215:
      if (!Zctlesc && (lastsent & 0177) != '@')
        goto sendit;
      /* **** FALL THRU TO **** */
    case 020:
    case 021:
    case 023:
    case 0220:
    case 0221:
    case 0223:
      xsendline(ZDLE);
      c ^= 0100;
sendit:
      xsendline(lastsent = c);
      break;
    default:
      if (Zctlesc && ! (c & 0140)) {
        xsendline(ZDLE);
        c ^= 0100;
      }
      xsendline(lastsent = c);
    }
  }
  return 0;
}


/* Decode two lower case hex digits into an 8 bit byte value */

int zgethex(void)
{
  int c, n;

  if ((c = noxrd7()) < 0)
    return c;
  n = c - '0';
  if (n > 9)
    n -= ('a' - ':');
  if (n & ~0xF)
    return ERROR;
  if ((c = noxrd7()) < 0)
    return c;
  c -= '0';
  if (c > 9)
    c -= ('a' - ':');
  if (c & ~0xF)
    return ERROR;
  c += (n<<4);
  return c;
}



/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */

int zdlread(void)
{
  int _z;
  if ((_z = readline(Rxtimeout)) & 0140)
    return (_z);
  else
    return (zdlread2(_z));
}

/*
int zdlread(void)
{
  int c;

again:
  // Quick check for non control characters
  if ((c = readline(Rxtimeout)) & 0140)
    return c;
  switch (c) {
  case ZDLE:
    break;
  case 023:
  case 0223:
  case 021:
  case 0221:
    goto again;
  default:
    if (Zctlesc && !(c & 0140)) {
      goto again;
    }
    return c;
  }
again2:
  if ((c = readline(Rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;
  switch (c) {
  case CAN:
    return GOTCAN;
  case ZCRCE:
  case ZCRCG:
  case ZCRCQ:
  case ZCRCW:
    return (c | GOTOR);
  case ZRUB0:
    return 0177;
  case ZRUB1:
    return 0377;
  case 023:
  case 0223:
  case 021:
  case 0221:
    goto again2;
  default:
    if (Zctlesc && ! (c & 0140)) {
      goto again2;
    }
    if ((c & 0140) ==  0100)
      return (c ^ 0100);
    break;
  }
  if (Verbose>1)
    zperr("Bad escape sequence %x", c);
  return ERROR;
}
*/

int zdlread2(int c)
{
again:
  // Quick check for non control characters

  switch (c) {
  case ZDLE:
    break;
  case 023:
  case 0223:
  case 021:
  case 0221:
    if ((c = readline(Rxtimeout)) & 0140)
      return c;
    goto again;
  default:
    if (Zctlesc && !(c & 0140)) {
      if ((c = readline(Rxtimeout)) & 0140)
        return c;
      goto again;
    }
    return c;
  }
again2:
  if ((c = readline(Rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;
  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;
  switch (c) {
  case CAN:
    return GOTCAN;
  case ZCRCE:
  case ZCRCG:
  case ZCRCQ:
  case ZCRCW:
    return (c | GOTOR);
  case ZRUB0:
    return 0177;
  case ZRUB1:
    return 0377;
  case 023:
  case 0223:
  case 021:
  case 0221:
    goto again2;
  default:
    if (Zctlesc && ! (c & 0140)) {
      goto again2;
    }
    if ((c & 0140) ==  0100)
      return (c ^ 0100);
    break;
  }
  if (Verbose>1)
    zperr("Bad escape sequence %x", c);
  return ERROR;
}

/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */
int noxrd7(void)
{
  int c;

  for (;;) {
    if ((c = readline(Rxtimeout)) < 0)
      return c;
    switch (c &= 0177) {
    case XON:
    case XOFF:
      continue;
    default:
      if (Zctlesc && !(c & 0140))
        continue;
    case '\r':
    case '\n':
    case ZDLE:
      return c;
    }
  }
}



/* Store long integer pos in Txhdr */
void stohdr(long pos)
{
  Txhdr[ZP0] = pos;
  Txhdr[ZP1] = pos>>8;
  Txhdr[ZP2] = pos>>16;
  Txhdr[ZP3] = pos>>24;
}


#ifndef NOTDEF
/* Recover a long integer from a header */
long rclhdr(char *hdr)
{
  long l;

  l = (hdr[ZP3] & 0377);
  l = (l << 8) | (hdr[ZP2] & 0377);
  l = (l << 8) | (hdr[ZP1] & 0377);
  l = (l << 8) | (hdr[ZP0] & 0377);
  return l;
}
#endif

/*
 *  Send a character to modem.  Small is beautiful.
 */
// Why was this called sendline ??
//void sendline(int c)
//{
//  ZSERIAL->write(c & 0xFF);
//  ZSERIAL->write(char(c));
//  ZSERIAL->flush();

//DSERIAL->print("SEND: ");
//DSERIAL->print(c, HEX);
//DSERIAL->println();

//}
/*
//>>> Needs to be fixed up - see the original in rz
// like sendline, this does not read a line!
int readline(int timeout)
{
  long then;
  unsigned char c;

  then = millis();
  while(ZSERIAL->available() < 1) {
    if(millis() - then > (unsigned int)timeout*100UL) {
DSERIAL->println("readline - TIMEOUT");
      return(TIMEOUT);
    }
  }
  c = ZSERIAL->read();
//DSERIAL->print("READ: ");
//DSERIAL->print(c, HEX);
//DSERIAL->println();
  return(c);
}
*/

/*
 * Purge the modem input queue of all characters
 */
void purgeline(void)
{
  while(ZSERIAL->available())ZSERIAL->read();
}

/*
 * Local console output simulation
 */
void bttyout(int c)
{
#ifndef ARDUINO
  if (Verbose || Fromcu)
    putc(c, stderr);
#endif
}

void flushmo(void)
{
  ZSERIAL->flush();
}

/* send cancel string to get the other end to shut up */
void canit(void)
{
  for (int i=0; i < 10; ++i) {
    ZSERIAL->write(24);
  }
  for (int i=0; i < 10; ++i) {
    ZSERIAL->write(8);
  }
  ZSERIAL->flush();
}

int readline(int timeout)
{
  char _c;
  if (ZSERIAL->readBytes(&_c, 1) > 0)
    return (_c);
  else
    return (TIMEOUT);
}

void sendline(int _c)
{
  if (ZSERIAL->availableForWrite() > SERIAL_TX_BUFFER_SIZE / 2)
    ZSERIAL->flush();
  ZSERIAL->write(char(_c));
}

void zsendline(int _z)
{
  if (_z & 0140)
    sendline(_z);
  else
    zsendline2(_z);
}

/* End of zm.c */
#endif
