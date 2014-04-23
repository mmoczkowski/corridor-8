/***************************************************************************
 *   WHMDM.C   - Witchaven modem & serial module                           *
 *                                                                         *
 *                                                     - Les Bird 08/09/95 *
 ***************************************************************************/

#include "stdio.h"
#include "conio.h"
#include "dos.h"
#include "graph.h"
#include "stdarg.h"
#include "string.h"
#include "serial.h"

#define   SCNSLINKED

#define   CLKIPS    18

#define   SCRSIZE   4000

#define   NUMSETOPTS          6

#ifdef SCNSLINKED
extern
char scntbl[][SCRSIZE];
#endif

enum {
     SETSER,
     SETCOM,
     SETBPS,
     SETINITS,
     SETDIALO,
     SETANSS,
     SETDIALS,
     SETDIALN,
     DIALMDM
};

char *setopts[]={
     "COM PORT:",
     "BAUD RATE:",
     "INIT STRING:",
     "ANSWER STRING:",
     "DIAL COMMAND:",
     "PHONE NUMBER:"
};

#define   ANSSLEN   48
#define   DIALSLEN  48
#define   DIALNLEN  48
#define   INITSLEN  48

char answerstring[ANSSLEN],
     dialstring[DIALSLEN],
     initstring[INITSLEN],
     mdmresp[1024],
     dialnumber[DIALNLEN],
     sbuf[SCRSIZE],
     *vmem;

short connected,
     oldvmode,
     quitprog;

long bps,
     comport,
     comtype=0,
     dialopt=0;

volatile
long totalclock;

struct serialData sd;

void (__interrupt __far *oldtimer)(void);

void __interrupt __far
newtimer(void)
{
     totalclock++;
     _chain_intr(oldtimer);
}

int
getBIOSvmode(void)
{
     union REGPACK regs;

     memset(&regs,0,sizeof(union REGPACK));
     regs.h.ah=0x0F;
     regs.h.al=0x00;
     intr(0x10,&regs);
     return(regs.h.al);
}

void
setBIOSvmode(int m)
{
     union REGPACK regs;

     memset(&regs,0,sizeof(union REGPACK));
     regs.h.ah=0x00;
     regs.h.al=m;
     intr(0x10,&regs);
}

void
writesettings(void)
{
     FILE *fp;

     fp=fopen("modem.dat","w");
     if (fp != NULL) {
          fprintf(fp,"COM PORT: %d\n",comport);
          fprintf(fp,"BAUD RATE: %ld\n",bps);
          fprintf(fp,"INIT STRING: %s\n",initstring);
          fprintf(fp,"ANSWER STRING: %s\n",answerstring);
          fprintf(fp,"DIAL COMMAND: %s\n",dialstring);
          fprintf(fp,"PHONE NUMBER: %s\n",dialnumber);
          fclose(fp);
     }
}

void
stripblanks(char *stg)
{
     short i;
     char buf[80];

     strcpy(buf,stg);
     for (i=0 ; i < strlen(buf) ; i++) {
          if (buf[i] == ' ') {
               continue;
          }
          *stg++=buf[i];
     }
     *stg=0;
}

void
readsettings(void)
{
     short i,n;
     long l;
     char buf[80],*ptr;
     FILE *fp;

     fp=fopen("modem.dat","r");
     if (fp == NULL) {
          return;
     }
     while (fgets(buf,80,fp) != NULL) {
          for (i=0 ; i < NUMSETOPTS ; i++) {
               if ((ptr=strstr(buf,setopts[i])) != NULL) {
                    ptr+=strlen(setopts[i]);
                    switch (i) {
                    case 0:   // com port
                         sscanf(ptr,"%d",&n);
                         if (n < 1 || n > 4) {
                              break;
                         }
                         comport=n;
                         break;
                    case 1:   // baud rate
                         sscanf(ptr,"%ld",&l);
                         if (l < 2400L || l > 115200L) {
                              break;
                         }
                         bps=l;
                         break;
                    case 2:   // init string
                         strcpy(initstring,ptr);
                         initstring[strlen(initstring)-1]=0;
                         stripblanks(initstring);
                         break;
                    case 3:   // answer command
                         strcpy(answerstring,ptr);
                         answerstring[strlen(answerstring)-1]=0;
                         stripblanks(answerstring);
                         break;
                    case 4:   // dial command
                         strcpy(dialstring,ptr);
                         dialstring[strlen(dialstring)-1]=0;
                         stripblanks(dialstring);
                         break;
                    case 5:   // phone number
                         strcpy(dialnumber,ptr);
                         dialnumber[strlen(dialnumber)-1]=0;
                         stripblanks(dialnumber);
                         break;
                    }
               }
          }
     }
     fclose(fp);
}

void
showscn(char *sname)
{
#ifndef SCNSLINKED
     FILE *fp;

     fp=fopen(sname,"rb");
     if (fp != NULL) {
          fread(sbuf,1,SCRSIZE,fp);
          fclose(fp);
          movedata(FP_SEG(sbuf),FP_OFF(sbuf),FP_SEG(vmem),FP_OFF(vmem),SCRSIZE);
     }
#else
     movedata(FP_SEG(scntbl),FP_OFF(scntbl),FP_SEG(vmem),FP_OFF(vmem),SCRSIZE);
#endif
}

void
gprintf(FILE *fp,char *fmt,...)
{
     char buf[80];
     va_list argptr;

     va_start(argptr,fmt);
     vsprintf(buf,fmt,argptr);
     va_end(argptr);
     _outtext(buf);
}

void
waitsecs(long secs)
{
     long delayclock;

     delayclock=totalclock+(secs*CLKIPS);
     while (totalclock < delayclock && !kbhit());
}

void
mdmsendstr(char *stg)
{
     short i;

     for (i=0 ; i < strlen(stg) ; i++) {
          writeSer(&sd,stg[i]);
     }
     writeSer(&sd,'\r');
}

void
mdmreset(void)
{
     _settextcolor(0);
     gprintf(stdout,"\nResetting com port..");
     setDTR(&sd,0);
     waitsecs(1);
     setDTR(&sd,1);
}

void
showinstructions(char *stg)
{
     int  ctrx;

     _setbkcolor(1);
     _settextcolor(14);
     _settextposition(25,1);
     gprintf(stdout,"같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�");
     if (strlen(stg) == 0) {
          return;
     }
     ctrx=strlen(stg)/2;
     _settextposition(25,40-ctrx-1);
     gprintf(stdout," %s ",stg);
}

void
mdmdial(void)
{
     long c,delayclock,mdmsendcmd,mdmstate=0,more=1,stat,x=0;
     char cmdstg[80];

     showinstructions("PRESS ANY KEY TO ABORT");
     _settextwindow(14,1,24,80);
     _settextposition(14,1);
     stat=initSerial(&sd,comport-1,USE_16550,256L,256L);
     if (stat == SER_OK) {
          setBPS(&sd,bps);
          mdmreset();
          mdmsendcmd=1;
          _settextcolor(0);
          gprintf(stdout,"\nInitializing...");
          do {
               if (mdmsendcmd) {
                    switch (mdmstate) {
                    case 0:
                         _settextcolor(11);
                         gprintf(stdout,"\nSETUP: %s",initstring);
                         mdmsendstr(initstring);
                         delayclock=totalclock+(CLKIPS*2);
                         break;
                    case 1:
                         _settextcolor(11);
                         if (dialopt) {
                              sprintf(cmdstg,"%s%s",dialstring,dialnumber);
                         }
                         else {
                              sprintf(cmdstg,"%s",answerstring);
                         }
                         gprintf(stdout,"\nSETUP: %s",cmdstg);
                         mdmsendstr(cmdstg);
                         delayclock=totalclock+(CLKIPS*45);
                         break;
                    }
                    mdmsendcmd=0;
               }
               if (rxBuffEmpty(&sd) == 0) {
                    if (x == 0) {
                         _settextcolor(14);
                         gprintf(stdout,"\nMODEM: ");
                    }
                    c=readSer(&sd);
                    if (c == 0) {
                         continue;
                    }
                    mdmresp[x++]=c;
                    if (c == '\n' || c == '\r') {
                         switch (mdmstate) {
                         case 0:
                              strupr(mdmresp);
                              if (strstr(mdmresp,"OK") != 0) {
                                   mdmstate++;
                              }
                              else if (strstr(mdmresp,"ERROR") != 0) {
                                   mdmsendcmd=1;
                              }
                              break;
                         case 1:
                              strupr(mdmresp);
                              if (dialopt == 0) {
                                   if (strstr(mdmresp,"OK") != 0) {
                                        mdmstate++;
                                        delayclock=totalclock+CLKIPS;
                                   }
                                   break;
                              }
                              if (strstr(mdmresp,"CONNECT") != 0) {
                                   connected=1;
                                   more=0;
                              }
                              else if (strstr(mdmresp,"BUSY") != 0
                                 || strstr(mdmresp,"NO") != 0) {
                                   mdmsendcmd=1;
                              }
                              break;
                         case 2:
                              strupr(mdmresp);
                              if (strstr(mdmresp,"CONNECT") != 0) {
                                   connected=1;
                                   more=0;
                              }
                              break;
                         }
                         memset(mdmresp,0,sizeof(mdmresp));
                         x=0;
                    }
                    else {
                         gprintf(stdout,"%c",c);
                    }
               }
               switch (mdmstate) {
               case 0:
                    if (totalclock >= delayclock) {
                         mdmreset();
                         mdmsendcmd=1;
                    }
                    break;
               case 1:
                    if (totalclock >= delayclock) {
                         mdmreset();
                         mdmsendcmd=1;
                    }
                    break;
               case 2:
                    if (totalclock >= delayclock) {
                         _settextcolor(0);
                         gprintf(stdout,"\nWaiting for call...");
                         delayclock=totalclock+(CLKIPS*15);
                    }
                    break;
               }
               if (kbhit()) {
                    getch();
                    gprintf(stdout,"\nAborting...");
                    more=0;
                    mdmreset();
               }
          } while (more);
          if (!connected) {
               setRTS(&sd,0);
               setDTR(&sd,0);
          }
     }
     _settextwindow(1,1,25,80);
     _settextcolor(14);
}

void
mdmterm(void)
{
     int  c,hexmode=0,more;
     struct rccoord pos;

     showinstructions("CONNECTED");
     _settextwindow(14,1,24,80);
     _settextposition(14,1);
     _settextcolor(0);
     _displaycursor(_GCURSORON);
     do {
          if (kbhit()) {
               c=getch();
               switch (c) {
               case 0x00:
                    switch(getch()) {
                    case 0x44:
                         quitprog=2;
                         more=0;
                         break;
                    case 0x23:     // ALT-H (hex mode)
                         hexmode^=1;
                         break;
                    }
                    break;
               }
               if (hexmode) {
                    gprintf(stdout,"(%02X)",c);
               }
               else if (isascii(c)) {
                    switch (c) {
                    case 0x08:
                         writeSer(&sd,c);
                         if (dialopt == 0) {
                              pos=_gettextposition();
                              if (pos.col > 1) {
                                   pos.col--;
                                   _settextposition(pos.row,pos.col);
                                   gprintf(stdout," ");
                                   _settextposition(pos.row,pos.col);
                              }
                         }
                         break;
                    case 0x0A:
                    case 0x0D:
                         writeSer(&sd,c);
                         if (dialopt == 0) {
                              gprintf(stdout,"\n");
                         }
                         break;
                    case 0x1B:
                         _settextcolor(0);
                         gprintf(stdout,"\n(DISCONNECTING)\n");
                         setRTS(&sd,0);
                         setDTR(&sd,0);
                         connected=0;
                         more=0;
                         break;
                    default:
                         writeSer(&sd,c);
                         if (dialopt == 0) {
                              gprintf(stdout,"%c",c);
                         }
                         break;
                    }
               }
          }
          else if (rxBuffEmpty(&sd) == 0) {
               c=readSer(&sd);
               if (hexmode) {
                    gprintf(stdout,"(%02X)",c);
               }
               else if (isascii(c)) {
                    switch (c) {
                    case 0x08:
                         pos=_gettextposition();
                         if (pos.col > 1) {
                              pos.col--;
                              _settextposition(pos.row,pos.col);
                              gprintf(stdout," ");
                              _settextposition(pos.row,pos.col);
                         }
                         break;
                    case 0x0A:
                    case 0x0D:
                         gprintf(stdout,"\n");
                         break;
                    default:
                         _settextcolor(10);
                         gprintf(stdout,"%c",c);
                         if (dialopt == 0) {
                              writeSer(&sd,c);
                         }
                         break;
                    }
               }
          }
     } while (more);
     _displaycursor(_GCURSOROFF);
     deInitSerial(&sd);
}

void
showvalues(int m)
{
     int  i;

     _settextwindow(1,1,25,80);
     for (i=0 ; i < DIALMDM ; i++) {
          if (i == m) {
               _settextcolor(0);
               _setbkcolor(7);
          }
          else {
               if (i > SETBPS) {
                    if (comtype) {
                         switch (i) {
                         case SETINITS:
                         case SETDIALO:
                              _settextcolor(14);
                              break;
                         case SETANSS:
                              if (dialopt) {
                                   _settextcolor(8);
                              }
                              else {
                                   _settextcolor(14);
                              }
                              break;
                         case SETDIALS:
                         case SETDIALN:
                              if (dialopt) {
                                   _settextcolor(14);
                              }
                              else {
                                   _settextcolor(8);
                              }
                              break;
                         }
                    }
                    else {
                         _settextcolor(8);
                    }
               }
               else {
                    _settextcolor(14);
               }
               _setbkcolor(1);
          }
          switch (i) {
          case SETSER:
               _settextposition(5,24);
               gprintf(stdout,"%s",comtype ? "MODEM " : "SERIAL");
               break;
          case SETCOM:
               _settextposition(6,24);
               gprintf(stdout,"%d",comport);
               break;
          case SETBPS:
               _settextposition(7,24);
               gprintf(stdout,"%5d",bps);
               break;
          case SETINITS:
               _settextposition(8,24);
               gprintf(stdout,"%-48.48s",initstring);
               break;
          case SETDIALO:
               _settextposition(9,24);
               gprintf(stdout,"%s",dialopt ? "DIAL  " : "ANSWER");
               break;
          case SETANSS:
               _settextposition(10,24);
               gprintf(stdout,"%-48.48s",answerstring);
               break;
          case SETDIALS:
               _settextposition(11,24);
               gprintf(stdout,"%-48.48s",dialstring);
               break;
          case SETDIALN:
               _settextposition(12,24);
               gprintf(stdout,"%-48.48s",dialnumber);
               break;
          }
     }
}

void main(void)
{
     long c,lastbps,menustate=0,more,oldcomport,oldbps;
     char oldinit[60];

     oldtimer=_dos_getvect(0x08);
     _dos_setvect(0x08,newtimer);
     oldvmode=getBIOSvmode();
     _setvideomode(_TEXTC80);
     vmem=0xB800<<4;
     showscn("mdmscrn.bin");
     _settextwindow(1,1,25,80);
     _setbkcolor(1);
     _settextcolor(14);
     bps=9600L;
     comport=1;
     strcpy(initstring,"AT&F&C1&D2");
     strcpy(dialstring,"ATDT");
     readsettings();
     _displaycursor(_GCURSOROFF);
     do {
          switch (menustate) {
          case SETCOM:
               oldcomport=comport;
               more=1;
               showvalues(menustate);
               showinstructions("Press the spacebar to toggle between COM port options");
               do {
                    switch (getch()) {
                    case 0:
                         switch (getch()) {
                         case 0x48:     // up arrow
                              menustate--;
                              if (menustate < 0) {
                                   menustate=0;
                              }
                              more=0;
                              break;
                         case 0x50:     // down arrow
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                         break;
                    case 0x20:
                         comport++;
                         if (comport > 4) {
                              comport=1;
                         }
                         showvalues(menustate);
                         break;
                    case 0x0D:
                         more=0;
                         break;
                    case 0x1B:
                         quitprog=1;
                         comport=oldcomport;
                         more=0;
                         break;
                    }
               } while (more);
               if (menustate == SETCOM) {
                    menustate++;
               }
               break;
          case SETSER:
               more=1;
               showvalues(menustate);
               showinstructions("Press the spacebar to toggle between SERIAL & MODEM");
               do {
                    switch (getch()) {
                    case 0:
                         switch (getch()) {
                         case 0x48:     // up arrow
                              menustate--;
                              if (menustate < 0) {
                                   menustate=0;
                              }
                              more=0;
                              break;
                         case 0x50:     // down arrow
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                         break;
                    case 0x20:
                         comtype^=1;
                         break;
                    case 0x0D:
                         more=0;
                         break;
                    case 0x1B:
                         quitprog=1;
                         more=0;
                         break;
                    }
                    showvalues(menustate);
               } while (more);
               if (menustate == SETSER) {
                    menustate++;
               }
               break;
          case SETBPS:
               oldbps=bps;
               more=1;
               showvalues(menustate);
               showinstructions("Press the spacebar to toggle between baud rate selections");
               do {
                    c=getch();
                    if (c == 0) {
                         switch (getch()) {
                         case 0x48:
                              menustate--;
                              if (menustate < 0) {
                                   menustate=0;
                              }
                              more=0;
                              break;
                         case 0x50:
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                    }
                    else if (c == 0x20) {
                         switch (bps) {
                         case 9600:
                              bps=14400;
                              break;
                         case 14400:
                              bps=19200;
                              break;
                         case 19200:
                              bps=28800;
                              break;
                         case 28800:
                              bps=9600;
                              break;
                         }
                    }
                    else if (c == 0x0D) {
                         more=0;
                    }
                    else if (c == 0x1B) {
                         bps=oldbps;
                         quitprog=1;
                         more=0;
                    }
                    showvalues(menustate);
               } while (more);
               if (menustate == SETBPS) {
                    if (comtype) {
                         menustate++;
                    }
                    else {
                         menustate=SETSER;
                    }
               }
               break;
          case SETINITS:
               strcpy(oldinit,initstring);
               more=1;
               showvalues(menustate);
               showinstructions("Type in an initialization string for your modem");
               do {
                    c=getch();
                    c=toupper(c);
                    if (c == 0) {
                         switch (getch()) {
                         case 0x48:
                              menustate--;
                              if (menustate < 0) {
                                   menustate=0;
                              }
                              more=0;
                              break;
                         case 0x50:
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                    }
                    if (c == 0x08) {
                         initstring[strlen(initstring)-1]=0;
                    }
                    else if (c == 0x0D) {
                         more=0;
                    }
                    else if (c == 0x1B) {
                         strcpy(initstring,oldinit);
                         quitprog=1;
                         more=0;
                    }
                    else if (strlen(initstring) < INITSLEN) {
                         initstring[strlen(initstring)]=c;
                    }
                    showvalues(menustate);
               } while (more);
               if (menustate == SETINITS) {
                    menustate++;
               }
               break;
          case SETANSS:
               strcpy(oldinit,answerstring);
               more=1;
               showvalues(menustate);
               showinstructions("Type in your modem's answer command string or use \"ATS0=1\"");
               do {
                    c=getch();
                    c=toupper(c);
                    if (c == 0) {
                         switch (getch()) {
                         case 0x48:
                              menustate--;
                              if (menustate < 0) {
                                   menustate=0;
                              }
                              more=0;
                              break;
                         case 0x50:
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                    }
                    if (c == 0x08) {
                         answerstring[strlen(answerstring)-1]=0;
                    }
                    else if (c == 0x0D) {
                         more=0;
                    }
                    else if (c == 0x1B) {
                         strcpy(answerstring,oldinit);
                         quitprog=1;
                         more=0;
                    }
                    else if (strlen(answerstring) < ANSSLEN) {
                         answerstring[strlen(answerstring)]=c;
                    }
                    showvalues(menustate);
               } while (more);
               if (menustate == SETANSS) {
                    menustate=DIALMDM;
               }
               break;
          case SETDIALS:
               strcpy(oldinit,dialstring);
               more=1;
               showvalues(menustate);
               showinstructions("Type in your modem's dial command string or use \"ATDT\"");
               do {
                    c=getch();
                    c=toupper(c);
                    if (c == 0) {
                         switch (getch()) {
                         case 0x48:
                              menustate=SETDIALO;
                              more=0;
                              break;
                         case 0x50:
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                    }
                    if (c == 0x08) {
                         dialstring[strlen(dialstring)-1]=0;
                    }
                    else if (c == 0x0D) {
                         more=0;
                    }
                    else if (c == 0x1B) {
                         strcpy(dialstring,oldinit);
                         quitprog=1;
                         more=0;
                    }
                    else if (strlen(dialstring) < DIALSLEN) {
                         dialstring[strlen(dialstring)]=c;
                    }
                    showvalues(menustate);
               } while (more);
               if (menustate == SETDIALS) {
                    menustate++;
               }
               break;
          case SETDIALO:
               more=1;
               showvalues(menustate);
               showinstructions("Press the spacebar to toggle between ANSWER and DIAL");
               do {
                    switch (getch()) {
                    case 0:
                         switch (getch()) {
                         case 0x48:
                              menustate--;
                              if (menustate < 0) {
                                   menustate=0;
                              }
                              more=0;
                              break;
                         case 0x50:
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                         break;
                    case 0x20:
                         dialopt^=1;
                         break;
                    case 0x0D:
                         more=0;
                         break;
                    case 0x1B:
                         quitprog=1;
                         more=0;
                         break;
                    }
                    showvalues(menustate);
               } while (more);
               if (menustate == SETDIALO) {
                    if (dialopt) {
                         menustate=SETDIALS;
                    }
                    else {
                         menustate=SETANSS;
                    }
               }
               break;
          case SETDIALN:
               strcpy(oldinit,dialnumber);
               more=1;
               showvalues(menustate);
               showinstructions("Type in the phone number to dial");
               do {
                    c=toupper(getch());
                    switch (c) {
                    case 0x00:
                         switch (getch()) {
                         case 0x48:
                              menustate--;
                              if (menustate < 0) {
                                   menustate=0;
                              }
                              more=0;
                              break;
                         case 0x50:
                              more=0;
                              break;
                         case 0x44:     // F10
                              more=0;
                              if (comtype) {
                                   menustate=DIALMDM;
                              }
                              else {
                                   quitprog=2;
                              }
                              break;
                         }
                         break;
                    case 0x08:
                         dialnumber[strlen(dialnumber)-1]=0;
                         break;
                    case 0x0D:
                         more=0;
                         break;
                    case 0x1B:
                         strcpy(dialnumber,oldinit);
                         quitprog=1;
                         more=0;
                         break;
                    default:
                         if (strlen(dialnumber) < DIALNLEN) {
                              dialnumber[strlen(dialnumber)]=c;
                         }
                         break;
                    }
                    showvalues(menustate);
               } while (more);
               if (menustate == SETDIALN) {
                    menustate++;
               }
               break;
          case DIALMDM:
               mdmdial();
               if (connected) {
                    mdmterm();
               }
               menustate=0;
               break;
          }
          showvalues(menustate);
     } while ((!connected) && (!quitprog));
     setBIOSvmode(oldvmode);
     fprintf(stdout,"\n");
     _dos_setvect(0x08,oldtimer);
     writesettings();
     if (quitprog == 2) {
          if (comtype == 0) {
               exit(2);
          }
          else if (connected) {
               exit(1);
          }
     }
     exit(0);
}

