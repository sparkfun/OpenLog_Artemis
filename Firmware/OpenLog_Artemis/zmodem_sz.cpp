/*% cc -compat -M2 -Ox -K -i -DTXBSIZE=16384  -DNFGVMIN -DREADCHECK sz.c -lx -o sz; size sz
 
 Following is used for testing, might not be reasonable for production
 <-xtx-*> cc -Osal -DTXBSIZE=32768  -DSV sz.c -lx -o $B/sz; size $B/sz
 
 ****************************************************************************
 *
 * sz.c By Chuck Forsberg,  Omen Technology INC
 *
 ****************************************************************************
 *
 * Typical Unix/Xenix/Clone compiles:
 *
 *      cc -O sz.c -o sz                USG (SYS III/V) Unix
 *      cc -O -DSV sz.c -o sz           Sys V Release 2 with non-blocking input
 *                                      Define to allow reverse channel checking
 *      cc -O -DV7  sz.c -o sz          Unix Version 7, 2.8 - 4.3 BSD
 *
 *      cc -O -K -i -DNFGVMIN -DREADCHECK sz.c -lx -o sz        Classic Xenix
 *
 *      ln sz sb                        **** All versions ****
 *      ln sz sx                        **** All versions ****
 *
 ****************************************************************************
 *
 * Typical VMS compile and install sequence:
 *
 *              define LNK$LIBRARY   SYS$LIBRARY:VAXCRTL.OLB
 *              cc sz.c
 *              cc vvmodem.c
 *              link sz,vvmodem
 *      sz :== $disk$user2:[username.subdir]sz.exe
 *
 *  If you feel adventureous, remove the #define BADSYNC line
 *  immediately following the #ifdef vax11c line!  Some VMS
 *  systems know how to fseek, some don't.
 *
 ****************************************************************************
 *
 *
 * A program for Unix to send files and commands to computers running
 *  Professional-YAM, PowerCom, YAM, IMP, or programs supporting Y/XMODEM.
 *
 *  Sz uses buffered I/O to greatly reduce CPU time compared to UMODEM.
 *
 *  USG UNIX (3.0) ioctl conventions courtesy Jeff Martin
 *
 *  2.1x hacks to avoid VMS fseek() bogosity, allow input from pipe
 *     -DBADSEEK -DTXBSIZE=32768  
 *  2.x has mods for VMS flavor
 *
 * 1.34 implements tx backchannel garbage count and ZCRCW after ZRPOS
 * in accordance with the 7-31-87 ZMODEM Protocol Description
 */

#include "zmodem_config.h"
#include "zmodem_fixes.h"

#ifdef ARDUINO_SMALL_MEMORY_INCLUDE_SZ

#define xsendline(c) sendline(c)

#define ultoa utoa

#include "zmodem.h"
#include "zmodem_zm.h"
#include "zmodem_crc16.cpp"

#include <stdio.h>

#define PATHLEN 64

#define HOWMANY 2

#define Txwindow 0      /* Control the size of the transmitted window */
#define Txwspac 0       /* Spacing between zcrcq requests */
unsigned Txwcnt;        /* Counter used to space ack requests */
#define Lrxpos rxbytes
extern long Lrxpos;            /* Receiver's last reported offset */

int Filesleft = 0;
long Totalleft = 0L;

/*
 * Attention string to be executed by receiver to interrupt streaming data
 *  when an error is detected.  A pause (0336) may be needed before the
 *  ^C (03) or after it.
 */
#ifdef READCHECK
char Myattn[] = { 
  0 };
#else
#ifdef USG
char Myattn[] = { 
  03, 0336, 0 };
#else
char Myattn[] = { 
  0 };
#endif
#endif

//FILE *in;

#ifdef BADSEEK
#define Canseek 0        /* 1: Can seek 0: only rewind -1: neither (pipe) */
#ifndef TXBSIZE
#define TXBSIZE 16384           /* Must be power of two, < MAXINT */
#endif
#else
#define Canseek 1        /* 1: Can seek 0: only rewind -1: neither (pipe) */
#endif

#ifdef TXBSIZE
#define TXBMASK (TXBSIZE-1)
#define Txb oneKbuf              /* Circular buffer for file reads */
#define txbuf Txb              /* Pointer to current file segment */
// Force the user to specify a buffer size
//#else
//char txbuf[1024];
#endif

//long vpos = 0;                  /* Number of bytes read from file */

#define Modem2 0           /* XMODEM Protocol - don't send pathnames */

#define Ascii 0            /* Add CR's for brain damaged programs */
#define Fullname 0         /* transmit full pathname */
#define Unlinkafter 0      /* Unlink file after it is sent */
#define Dottoslash 0       /* Change foo.bar.baz to foo/bar/baz */

int errcnt=0;           /* number of files unreadable */
#define blklen Blklen
extern int blklen;         /* length of transmitted records */
#define Optiong 0            /* Let it rip no wait for sector ACK's */

int Totsecs;            /* total number of sectors this file */
//int Filcnt=0;           /* count of number of files opened */
uint8_t Lfseen=0;
#define Rxbuflen 16384      /* Receiver's max buffer length */
int Tframlen = 0;       /* Override for tx frame length */
#define blkopt 0           /* Override value for zmodem blklen */
//int Rxflags = 0;
#define bytcnt Bytesleft
extern long bytcnt;
//int Wantfcs32 = TRUE;   /* want to send 32 bit FCS */
#define Lzconv 0    /* Local ZMODEM file conversion request */

#define  Lskipnocor 0
#define Lztrans 0

//int Command;            /* Send a command, then exit. */
//char *Cmdstr;           /* Pointer to the command string */
//int Cmdtries = 11;
//int Cmdack1;            /* Rx ACKs command, then do it */
//int Exitcode = 0;
#define Test 0               /* 1= Force receiver to send Attn, etc with qbf. */
/* 2= Character transparency test */

//char qbf[] = "The quick brown fox jumped over the lazy dog's back 1234567890\r\n";

long Lastsync;          /* Last offset to which we got a ZRPOS */
uint8_t Beenhereb4;         /* How many times we've been ZRPOS'd same place */

// Pete (El Supremo)
_PROTOTYPE(int wcs , (const char *oname ));
_PROTOTYPE(int wctxpn , (const char *name));

_PROTOTYPE(int wctx , (long flen ));
_PROTOTYPE(int wcputsec , (char *buf , int sectnum , int cseclen ));
_PROTOTYPE(int filbuf , (char *buf , int count ));
_PROTOTYPE(int zfilbuf , (void));
_PROTOTYPE(void flushmo , (void));
_PROTOTYPE(void purgeline , (void));
_PROTOTYPE(void canit , (void));

//void zperr();
#define zperr(a, ... )

_PROTOTYPE(int sendzsinit , (void));
_PROTOTYPE(int zsendfile , (char *buf , int blen ));
_PROTOTYPE(int zsendfdata , (void));
_PROTOTYPE(int getinsync , (int flag ));
_PROTOTYPE(void saybibi , (void));
//_PROTOTYPE(void bttyout , (int c ));
_PROTOTYPE(int zsendcmd , (char *buf , int blen ));


#ifndef ARDUINO
FILE *fout;
#else
extern SdFile fout;
#endif

int wcs(const char *oname)
{
//  char name[PATHLEN];

//  strcpy(name, oname);
  
  Eofseen = 0;  
//  vpos = 0;

  switch (wctxpn(oname)) {
   case ERROR:
     return ERROR;
   case ZSKIP:
     return OK;
  }

//  ++Filcnt;
  if(!Zmodem && wctx(fout.fileSize())==ERROR)
    return ERROR;
  return 0;
}


/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */
int wctxpn(const char *name)
{

  char *p, *q;

//DSERIAL->println("\nwctxpn");

  strcpy(txbuf,name);
  p = q = txbuf + strlen(txbuf)+1;
  //Pete (El Supremo) fix bug - was 1024, should be TXBSIZE??
  while (q < (txbuf + TXBSIZE))
    *q++ = 0;
//  if (!Ascii && (in!=stdin) && *name && fstat(fileno(in), &f)!= -1)
  if (!Ascii)
    // I will have to figure out how to convert the uSD date/time format to a UNIX epoch
//    sprintf(p, "%lu %lo %o 0 %d %ld", fout.fileSize(), 0L,0600, Filesleft, Totalleft);
// Avoid sprintf to save memory for small boards.  This sketch doesn't know what time it is anyway
    ultoa(fout.fileSize(), p, 10);
    strcat_P(p, PSTR(" 0 0 0 "));
    q = p + strlen(p);
    ultoa(Filesleft, q, 10);
    strcat_P(q, PSTR(" "));
    q = q + strlen(q);
    ultoa(Totalleft, q, 10);

  Totalleft -= fout.fileSize();
//DSERIAL->print(F("wctxpn sf = "));
//DSERIAL->print(sf);
//DSERIAL->print(F("  length = "));
//DSERIAL->println(Totalleft);
  if (--Filesleft <= 0)
    Totalleft = 0;
  if (Totalleft < 0)
    Totalleft = 0;

  /* force 1k blocks if name won't fit in 128 byte block */
  //Pete (El Supremo) This can't be right??!
  if (txbuf[125])
//    blklen=1024;
    blklen = TXBSIZE;
  else {          /* A little goodie for IMP/KMD */
    blklen = 128;
    txbuf[127] = (fout.fileSize() + 127) >>7;
    txbuf[126] = (fout.fileSize() + 127) >>15;
  }
  return zsendfile(txbuf, 1+strlen(p)+(p-txbuf));
}


int wctx(long flen)
{
  int thisblklen;
  int sectnum, attempts, firstch;
  long charssent;
  
//DSERIAL->println("\nwctx");

  charssent = 0;  
  firstsec=TRUE;  
  thisblklen = blklen;
  vfile(F("wctx:file length=%ld"), flen);

  while ((firstch=readline(Rxtimeout))!=NAK && firstch != WANTCRC
    && firstch != WANTG && firstch!=TIMEOUT && firstch!=CAN)
    ;
  if (firstch==CAN) {
    zperr("Receiver CANcelled");
    return ERROR;
  }
  if (firstch==WANTCRC)
    Crcflg=TRUE;
  if (firstch==WANTG)
    Crcflg=TRUE;
  sectnum=0;
  for (;;) {
    if (flen <= (charssent + 896L))
      thisblklen = 128;
    if ( !filbuf(txbuf, thisblklen))
      break;
    if (wcputsec(txbuf, ++sectnum, thisblklen)==ERROR)
      return ERROR;
    charssent += thisblklen;
  }
  //fclose(in);
  fout.close();
  attempts=0;
  do {
    purgeline();
    sendline(EOT);
    //fflush(stdout);
    ++attempts;
  }
  while ((firstch=(readline(Rxtimeout)) != ACK) && attempts < Tx_RETRYMAX);
  if (attempts == Tx_RETRYMAX) {
    zperr("No ACK on EOT");
    return ERROR;
  }
  else
    return OK;
}



int wcputsec(char *buf,int sectnum,int cseclen)
{
  int checksum, wcj;
  char *cp;
  unsigned oldcrc;
  int firstch;
  uint8_t attempts;

  firstch=0;      /* part of logic to detect CAN CAN */

  if (Verbose>2)
    fprintf(stderr, "Sector %3d %2dk\n", Totsecs, Totsecs/8 );
  else if (Verbose>1)
    fprintf(stderr, "\rSector %3d %2dk ", Totsecs, Totsecs/8 );
  for (attempts=0; attempts <= Tx_RETRYMAX; attempts++) {
    Lastrx= firstch;
    sendline(cseclen==1024?STX:SOH);
    sendline(sectnum);
    sendline(-sectnum -1);
    oldcrc=checksum=0;
    for (wcj=cseclen,cp=buf; --wcj>=0; ) {
      sendline(*cp);
      oldcrc=updcrc((0377& *cp), oldcrc);
      checksum += *cp++;
    }
    if (Crcflg) {
      oldcrc=updcrc(0,updcrc(0,oldcrc));
      sendline((int)oldcrc>>8);
      sendline((int)oldcrc);
    }
    else
      sendline(checksum);

    if (Optiong) {
      firstsec = FALSE; 
      return OK;
    }
    firstch = readline(Rxtimeout);
gotnak:
    switch (firstch) {
    case CAN:
      if(Lastrx == CAN) {
cancan:
        zperr("Cancelled");  
        return ERROR;
      }
      break;
    case TIMEOUT:
      zperr("Timeout on sector ACK"); 
      continue;
    case WANTCRC:
      if (firstsec)
        Crcflg = TRUE;
    case NAK:
      zperr("NAK on sector"); 
      continue;
    case ACK: 
      firstsec=FALSE;
      Totsecs += (cseclen>>7);
      return OK;
    case ERROR:
      zperr("Got burst for sector ACK"); 
      break;
    default:
      zperr("Got %02x for sector ACK", firstch); 
      break;
    }
    for (;;) {
      Lastrx = firstch;
      if ((firstch = readline(Rxtimeout)) == TIMEOUT)
        break;
      if (firstch == NAK || firstch == WANTCRC)
        goto gotnak;
      if (firstch == CAN && Lastrx == CAN)
        goto cancan;
    }
  }
  zperr("Retry Count Exceeded");
  return ERROR;
}



/* fill buf with count chars padding with ^Z for CPM */
int filbuf(char *buf,int count)
{
  int c, m;

//DSERIAL->println("\nfilbuf");

  if ( !Ascii) {
//    m = read(fileno(in), buf, count);
    m = fout.read(buf, count);
//DSERIAL->println(F("filbuf: '"));
//for(int i=0;i<m;i++) {
//  DSERIAL->print(buf[i]);
//}
//DSERIAL->println(F("'"));
    if (m <= 0)
      return 0;
    while (m < count)
      buf[m++] = 032;
    return count;
  }
  m=count;
  if (Lfseen) {
    *buf++ = 012; 
    --m; 
    Lfseen = 0;
  }
//  while ((c=getc(in))!=EOF) {
  while((c = fout.read()) != -1) {
    if (c == 012) {
      *buf++ = 015;
      if (--m == 0) {
        Lfseen = TRUE; 
        break;
      }
    }
    *buf++ =c;
    if (--m == 0)
      break;
  }
  if (m==count)
    return 0;
  else
    while (--m>=0)
      *buf++ = CPMEOF;
  return count;
}



/* Fill buffer with blklen chars */
int zfilbuf(void)
{
  int n;

// This code works:
//  n = fread(txbuf, 1, blklen, in);
  n = fout.read(txbuf,blklen);
  
  if (n < blklen)
    Eofseen = 1;
  return n;
}

/* Send file name and related info */
int zsendfile(char *buf, int blen)
{
  int c;
  UNSL long crc;

//DSERIAL->println(F("\nzsendfile"));

  for (;;) {
    Txhdr[ZF0] = Lzconv;    /* file conversion request */
    Txhdr[ZF1] = Lzmanag;   /* file management request */
    if (Lskipnocor)
      Txhdr[ZF1] |= ZMSKNOLOC;
    Txhdr[ZF2] = Lztrans;   /* file transport request */
    Txhdr[ZF3] = 0;
    zsbhdr(ZFILE, Txhdr);
    zsdata(buf, blen, ZCRCW);
again:
    c = zgethdr(Rxhdr, 1);
    switch (c) {
    case ZRINIT:
      while ((c = readline(50)) > 0)
        if (c == ZPAD) {
          goto again;
        }
      /* **** FALL THRU TO **** */
    default:
      continue;
    case ZCAN:
    case TIMEOUT:
    case ZABORT:
    case ZFIN:
//DSERIAL->println(F("\nzsendfile - ZFIN"));

      return ERROR;
    case ZCRC:
      crc = 0xFFFFFFFFL;
      if (Canseek >= 0) {
        fout.seekSet(0);
        while (((c = fout.read()) != -1)) // && --Rxpos)
          crc = UPDC32(c, crc);
        crc = ~crc;
//        clearerr(in);   /* Clear EOF */
//>>> Need to implement the seek
//        fseek(in, 0L, 0);
          fout.seekSet(0);
      }
      stohdr(crc);
      zsbhdr(ZCRC, Txhdr);
      goto again;
    case ZSKIP:
      fout.close();
      //fclose(in); 
//DSERIAL->println(F("\nzsendfile - ZSKIP"));
      return c;
    case ZRPOS:
      /*
       * Suppress zcrcw request otherwise triggered by
       * lastyunc==bytcnt
       */
//>>> Need to implement the seek
//      if (Rxpos && fseek(in, Rxpos, 0))
      if(Rxpos && !fout.seekSet(Rxpos))
        return ERROR;
      Lastsync = (bytcnt = Txpos = Rxpos) -1;
      int ret = zsendfdata();
//DSERIAL->print(F("\nzsendfile - exit - "));
//DSERIAL->println(ret);
      return(ret);
    }
  }
}



/* Send the data in the file */
int zsendfdata(void)
{
  int c, n;
  uint8_t e;
  int newcnt;
  uint8_t junkcount;          /* Counts garbage chars received by TX */

//DSERIAL->print(F("\nzsendfdata: "));
//DSERIAL->print(F("number = "));
//DSERIAL->print(Filesleft+1);
//DSERIAL->print(F("   length = "));
//DSERIAL->println(Totalleft); 
  Lrxpos = 0;
  junkcount = 0;
  Beenhereb4 = FALSE;
somemore:
  //if (setjmp(intrjmp)) {
  if (0) {
waitack:
    junkcount = 0;
    c = getinsync(0);
gotack:
    switch (c) {
    default:
    case ZCAN:
      fout.close();
      //fclose(in);
//DSERIAL->println(F("zsendfdata - error - 1"));
      return ERROR;
    case ZSKIP:
      fout.close();
      //fclose(in);
      return c;
    case ZACK:
    case ZRPOS:
      break;
    case ZRINIT:
      return OK;
    }
#ifdef READCHECK
    /*
                 * If the reverse channel can be tested for data,
     *  this logic may be used to detect error packets
     *  sent by the receiver, in place of setjmp/longjmp
     *  rdchk(fdes) returns non 0 if a character is available
     */
    while (ZSERIAL->available()) {
#ifdef SV
      switch (checked)
#else
        switch (readline(1))
#endif
        {
        case CAN:
        case ZPAD:
          c = getinsync(1);
          goto gotack;
        case XOFF:              /* Wait a while for an XON */
        case XOFF|0200:
          readline(100);
        }
    }
#endif
  }

//DSERIAL->println("zsendfdata - 1");

//  if ( !Fromcu)
//    signal(SIGINT, onintr);
  newcnt = Rxbuflen;
  Txwcnt = 0;
  stohdr(Txpos);
  zsbhdr(ZDATA, Txhdr);

//DSERIAL->println("zsendfdata - 2");

  do {
    n = zfilbuf();
// AHA - it reads the 18 chars here
//DSERIAL->println(n);
    if (Eofseen)
      e = ZCRCE;
    else if (junkcount > 3)
      e = ZCRCW;
    else if (bytcnt == Lastsync)
      e = ZCRCW;
    else if (Rxbuflen && (newcnt -= n) <= 0)
      e = ZCRCW;
    else if (Txwindow && (Txwcnt += n) >= Txwspac) {
      Txwcnt = 0;  
      e = ZCRCQ;
    }
    else
      e = ZCRCG;
    if (Verbose>1)
      fprintf(stderr, "\r%7ld ZMODEM%s    ",Txpos, Crc32t?" CRC-32":"");
    zsdata(txbuf, n, e);
    bytcnt = Txpos += n;
    if (e == ZCRCW)
      goto waitack;
#ifdef READCHECK
    /*
                 * If the reverse channel can be tested for data,
     *  this logic may be used to detect error packets
     *  sent by the receiver, in place of setjmp/longjmp
     *  rdchk(fdes) returns non 0 if a character is available
     */
//    fflush(stdout);
    while (ZSERIAL->available()) {
#ifdef SV
      switch (checked)
#else
        switch (readline(1))
#endif
        {
        case CAN:
        case ZPAD:
          c = getinsync(1);
          if (c == ZACK)
            break;
#ifdef TCFLSH
          ioctl(iofd, TCFLSH, 1);
#endif
          /* zcrce - dinna wanna starta ping-pong game */
          zsdata(txbuf, 0, ZCRCE);
          goto gotack;
        case XOFF:              /* Wait a while for an XON */
        case XOFF|0200:
          readline(100);
        default:
          ++junkcount;
        }
    }
#endif  /* READCHECK */

  } while (!Eofseen);
  
//DSERIAL->println("zsendfdata - 4"); 

//  if ( !Fromcu)
//    signal(SIGINT, SIG_IGN);

  for (;;) {
    stohdr(Txpos);
    zsbhdr(ZEOF, Txhdr);
    switch (getinsync(0)) {
    case ZACK:
//DSERIAL->println(F("zsendfdata - ZAK"));
      continue;
    case ZRPOS:
//DSERIAL->println(F("zsendfdata - ZRPOS"));
      goto somemore;
    case ZRINIT:
//DSERIAL->println(F("zsendfdata - OK"));
      return OK;
    case ZSKIP:
      fout.close();
      //fclose(in);
//DSERIAL->println(F("zsendfdata - ZSKIP"));
      return c;
    default:
      fout.close();
      //fclose(in);
//DSERIAL->println(F("zsendfdata - error - 2"));
      return ERROR;
    }
  }
}




/*
 * Respond to receiver's complaint, get back in sync with receiver
 */
int getinsync(int flag)
{
  int c;

  for (;;) {
    if (Test) {
//DSERIAL->println(F("***** Signal Caught *****"));
      Rxpos = 0; 
      c = ZRPOS;
    } 
    else
      c = zgethdr(Rxhdr, 0);
    switch (c) {
    case ZCAN:
    case ZABORT:
    case ZFIN:
    case TIMEOUT:
//DSERIAL->println(F("getinsync - timeout"));
      return ERROR;
    case ZRPOS:
      /* ************************************* */
      /*  If sending to a buffered modem, you  */
      /*   might send a break at this point to */
      /*   dump the modem's buffer.            */
//      clearerr(in);   /* In case file EOF seen */
//      if (fseek(in, Rxpos, 0)) {
      // seekSet returns true on success
      if(!fout.seekSet(Rxpos)) {
//DSERIAL->println(F("getinsync - fseek"));
        return ERROR;
      }
      Eofseen = 0;
      bytcnt = Lrxpos = Txpos = Rxpos;
      if (Lastsync == Rxpos) {
        if (++Beenhereb4 > 4)
          if (blklen > 32)
            blklen /= 2;
      }
      Lastsync = Rxpos;
      return c;
    case ZACK:
      Lrxpos = Rxpos;
      if (flag || Txpos == Rxpos)
        return ZACK;
      continue;
    case ZRINIT:
    case ZSKIP:
      fout.close();      
      //fclose(in);
      return c;
    case ERROR:
    default:
      zsbhdr(ZNAK, Txhdr);
      continue;
    }
  }
}

// Dylan (monte_carlo_ecm, bitflipper, etc.) - I added this simple ZRQINIT string to trigger
// terminal program's receive auto start feature.  This was missing in the code as I found it.
// All the terminal applications I tried would receive files anyway if I manually started
// the download, but sending the ZRQINIT is the right way to initiate a ZMODEM transfer
// according to the protocol documentation.

#define ZRQINIT_STR F("\x2a\x2a\x18\x42" \
"\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30\x30" \
"\x0d\x0a\x11")
                 
void sendzrqinit(void)
{
  ZSERIAL->print(ZRQINIT_STR);
}

/* Say "bibi" to the receiver, try to do it cleanly */
void saybibi(void)
{
  for (;;) {
    stohdr(0L);             /* CAF Was zsbhdr - minor change */
    zshhdr(ZFIN, Txhdr);    /*  to make debugging easier */
    switch (zgethdr(Rxhdr, 0)) {
    case ZFIN:
      sendline('O'); 
      sendline('O'); 
      flushmo();
    case ZCAN:
    case TIMEOUT:
      return;
    }
  }
}

#endif

/* End of sz.c */
