/***************************************************************************
 *   CORR7MUL.C - multiplayer routines for Corridor 7 game
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include "hmistd.h"
#include "netnow.h"
#include <memcheck.h>

#define   USEHMINETNOW

enum {
     NETGAME_LOGON=254,
     NETGAME_LOGOFF
};

int  gameMode=GAMEMODE_SINGLE,
     gameType,
     multiInitializedFlag,
     waitplayers;

int  moveFIFObeg,
     moveFIFOend;

int  gotpck,
     outofsync,
     sndpck;

long gotLastPacketClock,
     sendlogonclock;

short numStartSpots;

struct startSpot startSpot[MAXSTARTSPOTS],
     startSpotDeathMatch[MAXSTARTSPOTS];

long lockseed;

#ifdef USEHMINETNOW
struct nethdr {
     _XFER_BLOCK_HEADER sBlock;
     _NETNOW_NODE_ADDR sNodeAddr;
} netSendHdr,netRecvHdr;
#endif

struct netpck {
     short fvel;
     short svel;
     short avel;
     long moves;
     char weapon;
};

#define   PACKBUFSIZE    (MAXPLAYERS*sizeof(struct netpck))

struct moveFIFO moveFIFOBuf[MAXPLAYERS][MOVEFIFOSIZE];

struct netpck netpck[MAXPLAYERS],
     *netpckPtr[MAXPLAYERS];

char packbuf[PACKBUFSIZE];

#ifdef USEHMINETNOW
_NETNOW_NODE_ADDR myNodeAddr;
#endif

#define   NUMSETOPTS     2

short comp=2;

long comBaud=9600L;

char comtable[] = {0x00, 0x01, 0x02, 0x03, 0x04};

char *setopts[] = {
     "COM PORT:",
     "BAUD RATE:"
};

void
MUL_init(void)
{
}

void
MUL_unInit(void)
{
}

void
MUL_readINIFile(void)
{
     short i,
          n;
     long l;
     char buf[80],
         *ptr;
     FILE *fp;

     fp = fopen("modem.dat", "r");
     if (fp == NULL) {
          return;
     }
     while (fgets(buf, 80, fp) != NULL) {
          for (i = 0; i < NUMSETOPTS; i++) {
               if ((ptr = strstr(buf, setopts[i])) != NULL) {
                    ptr += strlen(setopts[i]);
                    switch (i) {
                    case 0:        // com port
                         sscanf(ptr, "%d", &n);
                         if (n < 1 || n > 4) {
                              break;
                         }
                         comp = n;
                         break;
                    case 1:        // baud rate
                         sscanf(ptr, "%ld", &l);
                         if (l < 2400L || l > 115200L) {
                              break;
                         }
                         comBaud = l;
                         break;
                    }
               }
          }
     }
     fclose(fp);
}

void
MUL_buildConnectPoints(void)
{
     int  i;

     connecthead=0;
     for (i=1 ; i < numplayers ; i++) {
          connectpoint2[i-1]=i;
     }
     i=numplayers-1;
     if (i < 0) {
          i=0;
     }
     connectpoint2[i]=-1;
}

#ifdef USEHMINETNOW
void
MUL_hmiInitMultiplayers(char opt4,char opt5,char pri)
{
     int  i;

     connecthead=0;
     for (i=0 ; i < MAXPLAYERS ; i++) {
          connectpoint2[i]=-1;
     }
     myconnectindex=connecthead;
     switch (hmiNETNOWInitSystem(opt4-3)) {
     case _NETNOW_NO_ERROR:
          break;
     case _NETNOW_NO_NETWORK:
          crash("IPX/NETBIOS not detected");
          break;
     default:
          crash("NETNOW initialization error");
          break;
     }
     for (i=0 ; i < _NETNOW_MAX_LISTEN_PACKETS ; i++) {
          hmiNETNOWPostListen();
     }
     hmiNETNOWGetNetworkAddr(&myNodeAddr);
     memmove(&netSendHdr.sNodeAddr,&myNodeAddr,sizeof(_NETNOW_NODE_ADDR));
     myconnectindex=hmiNETNOWGetConsoleNode();
}

void
MUL_hmiUninitMultiplayers(void)
{
     hmiNETNOWUnInitSystem();
}

void
MUL_hmiSendLogon(void)
{
     _XFER_BLOCK_HEADER *sBlock;
     char locbuf=NETGAME_LOGON;

     sendlogonclock=totalclock+TMR_getSecondTics(1);
     sBlock=&netSendHdr.sBlock;
     sBlock->wType=NETGAME_LOGON;
     sBlock->wNode=myconnectindex;
     sBlock->wLength=sizeof(locbuf);
     hmiNETNOWSendData((PSTR)&netSendHdr,sizeof(struct nethdr),
                       (PSTR)&locbuf,sizeof(locbuf),
                       _NETNOW_BROADCAST);
}

void
MUL_hmiSendLogoff(void)
{
     _XFER_BLOCK_HEADER *sBlock;
     char locbuf=NETGAME_LOGOFF;

     sBlock=&netSendHdr.sBlock;
     sBlock->wType=NETGAME_LOGOFF;
     sBlock->wNode=myconnectindex;
     sBlock->wLength=sizeof(locbuf);
     hmiNETNOWSendData((PSTR)&netSendHdr,sizeof(struct nethdr),
                       (PSTR)&locbuf,sizeof(locbuf),
                       _NETNOW_BROADCAST);
}

void
MUL_hmiSendPacket(short node,char *pdata,short len)
{
     BOOL rc;
     _XFER_BLOCK_HEADER *sBlock;

     sBlock=&netSendHdr.sBlock;
     sBlock->wType=_XFER_BLOCK_DATA;
     sBlock->wNode=myconnectindex;
     sBlock->wLength=len;
     do {
          rc=hmiNETNOWSendData((PSTR)&netSendHdr,sizeof(struct nethdr),
                               pdata,sBlock->wLength,node);
     } while (rc == _FALSE);
}

short
MUL_hmiGetPacket(short *node,char *buf)
{
     short len=0;
     BOOL e;
     PSTR pPacket;
     _XFER_BLOCK_HEADER *sBlock;

     e=hmiNETNOWGetHeader((PSTR)&netRecvHdr,sizeof(struct nethdr),&pPacket);
     if (e) {
          sBlock=&netRecvHdr.sBlock;
          *node=sBlock->wNode;
          len=sBlock->wLength;
          hmiNETNOWGetBlock(pPacket,buf,len);
          hmiNETNOWPostListen();
          switch (sBlock->wType) {
          case NETGAME_LOGON:
               if (sendlogonclock) {
                    hmiNETNOWAddNode(&netRecvHdr.sNodeAddr);
                    hmiNETNOWSortNodes();
                    myconnectindex=hmiNETNOWGetConsoleNode();
                    numplayers=hmiNETNOWGetActiveNodes();
                    MUL_buildConnectPoints();
               }
               break;
          case NETGAME_LOGOFF:
#if 0
               hmiNETNOWDeleteNode(sBlock->wNode);
               hmiNETNOWSortNodes();
               myconnectindex=hmiNETNOWGetConsoleNode();
               MUL_buildConnectPoints();
#endif
               break;
          case _XFER_BLOCK_DATA:
               break;
          default:
               break;
          }
     }
     if (sendlogonclock) {
          if (totalclock >= sendlogonclock) {
               MUL_hmiSendLogon();
          }
     }
     return(len);
}
#endif

void
MUL_uninitMultiPlayers(void)
{
     if (multiInitializedFlag) {
          switch (gameType) {
          case GAMETYPE_NETGAME:
#ifdef USEHMINETNOW
               MUL_hmiSendLogoff();
               MUL_hmiUninitMultiplayers();
#else
               sendlogoff();
               uninitmultiplayers();
#endif
               break;
          default:
               sendlogoff();
               uninitmultiplayers();
               break;
          }
     }
}

void
MUL_initMultiPlayers(void)
{
     int  i;
     char option4,option5;

     for (i=0 ; i < MAXPLAYERS ; i++) {
          netpckPtr[i]=&netpck[i];
     }
     switch (gameType) {
     case GAMETYPE_SINGLE:
          waitplayers=1;
          option4=0;
          option5=0;
          initmultiplayers(option4,option5,0);
          break;
     case GAMETYPE_MODEM:
          waitplayers=2;
          MUL_readINIFile();
          option4 = option5 = 0;
          option4 = comtable[comp];
          switch (comBaud) {
          case 2400:
               break;
          case 4800:
               option5 |= 0x01;
               break;
          case 14400:
               option5 |= 0x03;
               break;
          case 19200:
               option5 |= 0x04;
               break;
          case 28800:
               option5 |= 0x05;
               break;
          default:
          case 9600:
               option5 |= 0x02;
               break;
          }
          if (comp == 1 || comp == 3) {
               option5 |= 0x20;
          }
          else {
               option5 |= 0x10;
          }
          initmultiplayers(option4,option5,0);
          break;
     case GAMETYPE_NETGAME:
          if (waitplayers >= MAXPLAYERS) {
               crash("MUL_initMultiPlayers: Too many players (%d)",waitplayers);
          }
          if (waitplayers < 2) {
               waitplayers=2;
          }
          option4=waitplayers+3;
          option5=0;
#ifdef USEHMINETNOW
          MUL_hmiInitMultiplayers(option4,option5,0);
#else
          initmultiplayers(option4,option5,0);
#endif
          break;
     }
     multiInitializedFlag=1;
}

void
MUL_sendPacket(short node,char *buf,short len)
{
     short i;

     switch (gameType) {
     case GAMETYPE_NETGAME:
#ifdef USEHMINETNOW
          MUL_hmiSendPacket(node,buf,len);
#else
          sendpacket(node,buf,len);
#endif
          break;
     default:
          sendpacket(node,buf,len);
          break;
     }
     if (dbgfp != NULL) {
          if (len > 0) {
               debugFPOut("%02d->%02d: ",myconnectindex,node);
               for (i=0 ; i < len ; i++) {
                    debugFPOut("%02X ",buf[i]);
               }
               debugFPOut("\n");
          }
     }
}

short
MUL_getPacket(short *node,char *buf)
{
     short i,len;

     switch (gameType) {
     case GAMETYPE_NETGAME:
#ifdef USEHMINETNOW
          len=MUL_hmiGetPacket(node,buf);
#else
          len=getpacket(node,buf);
#endif
          break;
     default:
          len=getpacket(node,buf);
          break;
     }
     if (dbgfp != NULL) {
          if (len > 0) {
               debugFPOut("%02d<-%02d: ",myconnectindex,*node);
               for (i=0 ; i < len ; i++) {
                    debugFPOut("%02X ",buf[i]);
               }
               debugFPOut("\n");
          }
     }
     return(len);
}

short
MUL_getOutputCircleSize(void)
{
     short bytes;

     switch (gameType) {
     case GAMETYPE_NETGAME:
#ifdef USEHMINETNOW
          bytes=1;
#else
          bytes=getoutputcirclesize();
#endif
          break;
     default:
          bytes=getoutputcirclesize();
          break;
     }
     return(bytes);
}

void
MUL_sendLogon(void)
{
     switch (gameType) {
     case GAMETYPE_NETGAME:
#ifdef USEHMINETNOW
          MUL_hmiSendLogon();
#else
          sendlogon();
#endif
          break;
     default:
          sendlogon();
          break;
     }
}

void
MUL_sendLogoff(void)
{
     switch (gameType) {
     case GAMETYPE_NETGAME:
#ifdef USEHMINETNOW
          MUL_hmiSendLogoff();
#else
          sendlogoff();
#endif
          break;
     default:
          sendlogoff();
          break;
     }
}

int
MUL_waitForPlayers(void)
{
     short p,
          ready;
     long mapchksum;

     if (waitplayers < 2) {
          return(1);
     }
     MUL_sendLogon();
     while (numplayers < waitplayers) {
          rotatesprite(320L<<15,200L<<15,65536,0,backgroundPic,0,0,2+64+128,
                       0L,0L,xdim-1L,ydim-1L);
          switch (gameType) {
          case GAMETYPE_MODEM:
               sprintf(tempbuf,"GAMETYPE: MODEM");
               printext256((xdim>>1)-(strlen(tempbuf)<<2),(ydim>>3),
                           P256COLOR,-1,tempbuf,0);
               sprintf(tempbuf,"COMPORT:  %d",comp);
               printext256((xdim>>1)-(strlen(tempbuf)<<2),(ydim>>3)+10,
                           P256COLOR,-1,tempbuf,0);
               sprintf(tempbuf,"BAUD RATE:  %ld",comBaud);
               printext256((xdim>>1)-(strlen(tempbuf)<<2),(ydim>>3)+20,
                           P256COLOR,-1,tempbuf,0);
               break;
          case GAMETYPE_NETGAME:
               sprintf(tempbuf,"GAMETYPE: NETWORK/IPX");
               printext256((xdim>>1)-(strlen(tempbuf)<<2),(ydim>>2),
                           P256COLOR,-1,tempbuf,0);
               sprintf(tempbuf,"PLAYERS:  %d",waitplayers);
               printext256((xdim>>1)-(strlen(tempbuf)<<2),(ydim>>2)+10,
                           P256COLOR,-1,tempbuf,0);
               break;
          }
          sprintf(tempbuf,"%d of %d PLAYERS READY",numplayers,waitplayers);
          printext256((xdim>>1)-(strlen(tempbuf)<<2),(ydim>>1),
                      P256COLOR,-1,tempbuf,0);
          MUL_getPacket(&p,tempbuf);
          if (keystatus[1]) {
               MUL_sendLogoff();
               return(0);
          }
          nextpage();
          GFX_fadeIn(8);
     }
     sendlogonclock=0L;
     ready=1;
     if (myconnectindex != connecthead) {
          mapchksum=ENG_getMapChecksum();
          tempbuf[0]=10;
          tempbuf[1]=(mapchksum>>24)&0xFF;
          tempbuf[2]=(mapchksum>>16)&0xFF;
          tempbuf[3]=(mapchksum>>8)&0xFF;
          tempbuf[4]=(mapchksum)&0xFF;
          MUL_sendPacket(connecthead,tempbuf,5);
     }
     while (1) {
          EFF_notify(0L,"SYNCING %d PLAYERS",numplayers);
          if (keystatus[1]) {
               return(0);
          }
          if (MUL_getPacket(&p,tempbuf) > 0) {
               if (myconnectindex == connecthead) {
                    if (tempbuf[0] == 10) {
                         mapchksum=((long)tempbuf[1]<<24)|
                                   ((long)tempbuf[2]<<16)|
                                   ((long)tempbuf[3]<<8) |
                                   ((long)tempbuf[4]);
                         if (mapchksum == ENG_getMapChecksum()) {
                              tempbuf[0]=10;
                         }
                         else {
                              tempbuf[0]=11;
                              ready++;
                         }
                         MUL_sendPacket(p,tempbuf,1);
                    }
                    else if (tempbuf[0] == 1) {
                         ready++;
                    }
                    if (ready >= numplayers) {
                         break;
                    }
               }
               else {
                    if (p == connecthead) {
                         if (tempbuf[0] == 10) {
                              tempbuf[0]=1;
                              MUL_sendPacket(connecthead,tempbuf,1);
                              break;
                         }
                         else if (tempbuf[0] == 11) {
                              KBD_resetKeys();
                              while (!KBD_keyPressed()) {
                                   EFF_notify(0L,"WRONG MAP");
                              }
                              return(0);
                         }
                    }
               }
          }
     }
     currentView=myconnectindex;
     clearview(0L);
     nextpage();
     return(1);
}

void
MUL_moveThings(void)
{
     gotLastPacketClock=totalclock;
     moveFIFOend=(moveFIFOend+1)&(MOVEFIFOSIZE-1);
}

void
MUL_getInput(void)
{
     int  n,tmpend;
     long moves;
     short p,packbufleng;
     struct moveFIFO *movePtr;

     for (p=connecthead ; p >= 0 ; p=connectpoint2[p]) {
          movePtr=&moveFIFOBuf[p][moveFIFOend];
          memset(movePtr,0,sizeof(struct moveFIFO));
     }
     switch (gameType) {
     case GAMETYPE_SINGLE:
          PLR_getInput();
          movePtr=&moveFIFOBuf[myconnectindex][moveFIFOend];
          movePtr->avel=locavel;
          movePtr->fvel=locfvel;
          movePtr->svel=locsvel;
          movePtr->moves=player[myconnectindex]->moves;
          movePtr->weapon=PLR_getViewWeapon();
          MUL_moveThings();
          break;
     case GAMETYPE_MODEM:
     case GAMETYPE_NETGAME:
          while ((packbufleng=MUL_getPacket(&p,packbuf)) > 0) {
               switch (packbuf[0]) {
               case 0:             // got a master sync packet
                    if (myconnectindex != connecthead) {
                         n=1;
                         tmpend=packbuf[n++];
                         for (p=connecthead ; p >= 0 ; p=connectpoint2[p]) {
                              movePtr=&moveFIFOBuf[p][moveFIFOend];
                              movePtr->fvel=packbuf[n++]<<8;
                              movePtr->fvel|=packbuf[n++];
                              movePtr->svel=packbuf[n++]<<8;
                              movePtr->svel|=packbuf[n++];
                              movePtr->avel=packbuf[n++]<<8;
                              movePtr->avel|=packbuf[n++];
                              movePtr->moves=packbuf[n++]<<24;
                              movePtr->moves|=packbuf[n++]<<16;
                              movePtr->moves|=packbuf[n++]<<8;
                              movePtr->moves|=packbuf[n++];
                              movePtr->weapon=packbuf[n++];
                         }
                         if ((moveFIFOend&0xFF) != tmpend) {
                              outofsync=1;
                         }
                         MUL_moveThings();
                         gotpck++;
                    }
                    break;
               case 1:             // got a slave input packet
                    if (myconnectindex == connecthead) {
                         n=1;
                         netpckPtr[p]->fvel=packbuf[n++]<<8;
                         netpckPtr[p]->fvel|=packbuf[n++];
                         netpckPtr[p]->svel=packbuf[n++]<<8;
                         netpckPtr[p]->svel|=packbuf[n++];
                         netpckPtr[p]->avel=packbuf[n++]<<8;
                         netpckPtr[p]->avel|=packbuf[n++];
                         netpckPtr[p]->moves=packbuf[n++]<<24;
                         netpckPtr[p]->moves|=packbuf[n++]<<16;
                         netpckPtr[p]->moves|=packbuf[n++]<<8;
                         netpckPtr[p]->moves|=packbuf[n++];
                         netpckPtr[p]->weapon=packbuf[n++];
                         gotpck++;
                    }
                    break;
               case 254:           // got a bad map sync error packet
                    MUL_sendLogoff();
                    while (!KBD_keyPressed()) {
                         EFF_notify(0L,"WRONG MAP");
                    }
                    break;
               case 255:           // got a player quit packet
                    EFF_displayMessage("%d LEFT THE GAME",p);
                    ACT_killPlayer(PLR_getPlayerSprite(p));
                    if (p == connecthead) {
                         crash("SERVER QUIT THE GAME");
                    }
                    break;
               }
          }
          PLR_getInput();
          if (MUL_getOutputCircleSize() > 16) {
               return;
          }
          if (myconnectindex == connecthead) {
               netpckPtr[myconnectindex]->fvel=locfvel;
               netpckPtr[myconnectindex]->svel=locsvel;
               netpckPtr[myconnectindex]->avel=locavel;
               netpckPtr[myconnectindex]->moves=player[myconnectindex]->moves;
               netpckPtr[myconnectindex]->weapon=PLR_getViewWeapon();
               n=0;
               packbuf[n++]=0;
               packbuf[n++]=moveFIFOend&0xFF;
               for (p=connecthead ; p >= 0 ; p=connectpoint2[p]) {
                    packbuf[n++]=(netpckPtr[p]->fvel>>8)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->fvel)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->svel>>8)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->svel)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->avel>>8)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->avel)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->moves>>24)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->moves>>16)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->moves>>8)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->moves)&0xFF;
                    packbuf[n++]=(netpckPtr[p]->weapon);
                    movePtr=&moveFIFOBuf[p][moveFIFOend];
                    movePtr->fvel=netpckPtr[p]->fvel;
                    movePtr->svel=netpckPtr[p]->svel;
                    movePtr->avel=netpckPtr[p]->avel;
                    movePtr->moves=netpckPtr[p]->moves;
                    movePtr->weapon=netpckPtr[p]->weapon;
               }
               for (p=connectpoint2[connecthead] ; p >= 0 ; p=connectpoint2[p]) {
                    MUL_sendPacket(p,packbuf,n);
               }
               MUL_moveThings();
               sndpck++;
          }
          else {                   // slave computer(s)
               moves=player[myconnectindex]->moves;
               n=0;
               packbuf[n++]=1;
               packbuf[n++]=locfvel>>8;
               packbuf[n++]=locfvel;
               packbuf[n++]=locsvel>>8;
               packbuf[n++]=locsvel;
               packbuf[n++]=locavel>>8;
               packbuf[n++]=locavel;
               packbuf[n++]=(moves>>24)&0xFF;
               packbuf[n++]=(moves>>16)&0xFF;
               packbuf[n++]=(moves>>8)&0xFF;
               packbuf[n++]=(moves)&0xFF;
               packbuf[n++]=PLR_getViewWeapon();
               MUL_sendPacket(connecthead,packbuf,n);
          }
          break;
     }
}

void
MUL_debug(void)
{
     debugOut(windowx1,windowy1,"RSEED=%ld",randomseed);
     switch (outofsync) {
     case 1:
          debugOut(windowx1,windowy1,"OUT OF SYNC: MISSED PACKET");
          break;
     case 2:
          debugOut(windowx1,windowy1,"OUT OF SYNC: RANDOMSEED");
          break;
     }
}

void
MUL_scanSprite(short i)
{
     if (spritePtr[i]->statnum >= MAXSTATUS) {
          return;
     }
     if (ACT_isActor(i)) {
          return;
     }
     if (PLR_isPlayer(NULL,i)) {
          return;
     }
     if (SND_isAmbientSound(i) || SND_isSectorSound(i)) {
          return;
     }
     switch (spritePtr[i]->lotag) {
     case SPR_PLAYER1STARTTAG:
     case SPR_PLAYER2STARTTAG:
     case SPR_PLAYER3STARTTAG:
     case SPR_PLAYER4STARTTAG:
     case SPR_PLAYER5STARTTAG:
     case SPR_PLAYER6STARTTAG:
     case SPR_PLAYER7STARTTAG:
     case SPR_PLAYER8STARTTAG:
          if (gameMode != GAMEMODE_DEATHMATCH) {
               startSpot[numStartSpots].x=spritePtr[i]->x;
               startSpot[numStartSpots].y=spritePtr[i]->y;
               startSpot[numStartSpots].z=spritePtr[i]->z;
               startSpot[numStartSpots].ang=spritePtr[i]->ang;
               startSpot[numStartSpots].sectnum=spritePtr[i]->sectnum;
               numStartSpots++;
          }
          else {
               startSpotDeathMatch[numStartSpots].x=spritePtr[i]->x;
               startSpotDeathMatch[numStartSpots].y=spritePtr[i]->y;
               startSpotDeathMatch[numStartSpots].z=spritePtr[i]->z;
               startSpotDeathMatch[numStartSpots].ang=spritePtr[i]->ang;
               startSpotDeathMatch[numStartSpots].sectnum=spritePtr[i]->sectnum;
               numStartSpots++;
          }
          ENG_deletesprite(i);
          break;
     case SPR_DEATHMATCHSTARTTAG:
          if (gameMode == GAMEMODE_DEATHMATCH) {
               startSpotDeathMatch[numStartSpots].x=spritePtr[i]->x;
               startSpotDeathMatch[numStartSpots].y=spritePtr[i]->y;
               startSpotDeathMatch[numStartSpots].z=spritePtr[i]->z;
               startSpotDeathMatch[numStartSpots].ang=spritePtr[i]->ang;
               startSpotDeathMatch[numStartSpots].sectnum=spritePtr[i]->sectnum;
               numStartSpots++;
          }
          ENG_deletesprite(i);
          break;
     }
}

void
MUL_scanMap(void)
{
     short i;

     memset(&numStartSpots,0,sizeof(numStartSpots));
     memset(startSpot,0,sizeof(startSpot));
     memset(startSpotDeathMatch,0,sizeof(startSpotDeathMatch));
     for (i=0 ; i < MAXSPRITES ; i++) {
          MUL_scanSprite(i);
     }
}

void
MUL_saveGame(FILE *fp)
{
     GAM_fwrite(&gameMode,sizeof(int),1,fp);
     GAM_fwrite(&gameType,sizeof(int),1,fp);
     GAM_fwrite(&numStartSpots,sizeof(short),1,fp);
     GAM_fwrite(startSpot,sizeof(struct startSpot),MAXSTARTSPOTS,fp);
     GAM_fwrite(startSpotDeathMatch,sizeof(struct startSpot),MAXSTARTSPOTS,fp);
}

void
MUL_loadGame(FILE *fp)
{
     GAM_fread(&gameMode,sizeof(int),1,fp);
     GAM_fread(&gameType,sizeof(int),1,fp);
     GAM_fread(&numStartSpots,sizeof(short),1,fp);
     GAM_fread(startSpot,sizeof(struct startSpot),MAXSTARTSPOTS,fp);
     GAM_fread(startSpotDeathMatch,sizeof(struct startSpot),MAXSTARTSPOTS,fp);
}

