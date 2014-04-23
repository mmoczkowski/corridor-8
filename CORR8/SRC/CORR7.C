/***************************************************************************
 *   CORR7.C   - main routines for Corridor 7 using the BUILD engine
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include "graph.h"
#include "env.h"
#include <memcheck.h>

#define   GAMENAME       "CORRIDOR 8: GALACTIC WARS"
#define   VERSION        "0.91c"
#define   MAXDEBUGLINES  25

jmp_buf env;

enum {
     PARM_PLAYERS,
     PARM_LEVEL,
     PARM_EDIT,
     PARM_NETGAME,
     PARM_MDMGAME,
     PARM_MPGAME,
     PARM_NOSOUND,
     PARM_NOWAIT,
     PARM_NOENEMIES,
     PARM_DEATHMATCH,
     PARM_CONFIG,
     PARM_VIDMODE,
     PARM_WEAPONEDIT,
     PARM_QUESTIONMARK,
     PARM_HELP,
     PARM_DEBUG,
     PARM_DUMP,
     MAXPARMS
};

char *parmList[MAXPARMS]={
     "PLAYERS",
     "MAP",
     "EDIT",
     "NET",
     "MODEM",
     "MPLAYER",
     "NOSOUND",
     "NOWAIT",
     "NOENEMIES",
     "DEATHMATCH",
     "CONFIG",
     "VIDMODE",
     "WEAPONEDIT",
     "?",
     "HELP",
     "DEBUG",
     "DUMP"
};

char *parmDesc[MAXPARMS]={
     "number of players (ex: /players 4)",
     "map file to play (ex: /map test)",
     "editor mode",
     "network (IPX) multiplayer game",
     "modem or serial multiplayer game",
     "MPath internet game",
     "disable sound effects",
     "skip menus and go directly to game",
     "do not spawn enemies",
     "deathmatch mode (multiplayer games only)",
     "configure sound card",
     "configure video mode",
     "edit weapons",
     "help screen",
     "help screen",
     "debug mode",
     "dump debug info to DEBUG.xxx file"
};

char *videoModeList[MAXVIDMODES]={
     "CHAIN",
     "VESA",
     "SCREENBUFFER",
     "TSENG",
     "PARADISE",
     "S3CHIPSET"
};

short chainResX[]={
     256,320,360,400
};

#define   MAXCHAINXRES   (sizeof(chainResX)/sizeof(short))

short chainResY[]={
     200,240,256,270,300,350,360,400,480,512,540
};

#define   MAXCHAINYRES   (sizeof(chainResY)/sizeof(short))

short chainResolutions[]={
     0,0,0,1,0,2,0,3,0,4,0,5,0,6,0,7,0,8,0,9,0,10,
     1,0,1,1,1,2,1,3,1,4,1,5,1,6,1,7,1,8,1,9,1,10,
     2,0,2,1,2,2,2,3,2,4,2,5,2,6,2,7,2,8,2,9,2,10,
     3,0,3,1,3,2,3,3,3,4,3,5,3,6,3,7,3,8,3,9,3,10
};

#define   MAXCHAINRESOLUTIONS ((sizeof(chainResolutions)/sizeof(short))/2)

short vesaResX[]={
      320, 360, 640, 800,1024,1280,1600
};

#define   MAXVESAXRES    (sizeof(vesaResX)/sizeof(short))

short vesaResY[]={
      200, 240, 400, 350, 400, 480, 600, 768,1024,1200
};

#define   MAXVESAYRES    (sizeof(vesaResY)/sizeof(short))

short vesaResolutions[]={
     0,0,1,0,
     0,1,1,1,
     0,2,1,2,
     2,3,2,4,2,5,
     3,6,
     4,7,
     5,8
};

#define   MAXVESARESOLUTIONS  ((sizeof(vesaResolutions)/sizeof(short))/2)

int  videoMode=VID_SCREENBUFFER;

int  noWaitFlag;

long videoResX=320,
     videoResY=200;

char debugstr[MAXDEBUGLINES][160];

long debugx[MAXDEBUGLINES],
     debugy[MAXDEBUGLINES];

extern
int  _argc;

extern
char **_argv;

void
lprintf(char *fmt,...)
{
     char locbuf[80];
     va_list argptr;

     va_start(argptr,fmt);
     vsprintf(locbuf,fmt,argptr);
     va_end(argptr);
     _outtext(locbuf);
}

void
lprintat(short x,short y,char *fmt,...)
{
     char locbuf[80];
     va_list argptr;

     va_start(argptr,fmt);
     vsprintf(locbuf,fmt,argptr);
     va_end(argptr);
     if (x == -1) {
          x=40-(strlen(locbuf)>>1);
     }
     _settextposition(y,x);
     lprintf(locbuf);
}

void
initVid(void)
{
     _setvideomode(_TEXTC80);
     _setbkcolor(7);
     _settextcolor(15);
     _settextwindow(1,1,2,80);
     _clearscreen(_GWINDOW);
     lprintat(-1,1,"%s V%s\n",GAMENAME,VERSION);
     _setbkcolor(1);
     _settextwindow(2,1,25,80);
     _clearscreen(_GWINDOW);
}

void
startup(void)
{
     initVid();
     ACT_init();
     EFF_init();
     ENG_init();
     GAM_init();
     MUL_init();
     PLR_init();
}

void
shutdown(void)
{
     uninitgroupfile();
     uninittimer();
     uninitkeys();
     GAM_uninitGame();
     MUL_uninitMultiPlayers();
     ENG_uninitBuild();
     if (mc_is_active()) {
          fprintf(stderr,"MEMCHECK Reports %ld bytes of dynamic memory used\n",
                  mc_bytes_allocated());
     }
     GAM_unInitPointers();
     SND_unInitPointers();
     ACT_unInit();
     EFF_unInit();
     ENG_unInit();
     GAM_unInit();
     MUL_unInit();
     PLR_unInit();
}

void
crash(char *fmt,...)
{
     char errBuf[_MAX_PATH];
     va_list argptr;

     va_start(argptr,fmt);
     vsprintf(errBuf,fmt,argptr);
     va_end(argptr);
     if (ENG_editorGameMode()) {
          EFF_notify(0L,"%-40.40s",errBuf);
          KBD_resetKeys();
          while (KBD_keyPressed() == 0);
          playingGameFlag=-1;
          inGameFlag=0;
          inMenuFlag=0;
          longjmp(env,1);          // returns to gameMain()
     }
     else {
          writeControlConfigs();
          shutdown();
          fprintf(stderr,"\n%s\n",errBuf);
          exit(0);
     }
}

char *
getVideoModeName(int vidmode)
{
     return(videoModeList[vidmode]);
}

int
validVideoMode(char *name,short *xsize,short *ysize)
{
     int  i,j;

     for (i=0 ; i < MAXVIDMODES ; i++) {
          if (stricmp(name,videoModeList[i]) == 0) {
               break;
          }
     }
     switch (i) {
     case VID_CHAIN:
          for (j=0 ; j < MAXCHAINXRES ; j++) {
               if (*xsize <= chainResX[j]) {
                    *xsize=chainResX[j];
                    break;
               }
          }
          for (j=0 ; j < MAXCHAINYRES ; j++) {
               if (*ysize <= chainResY[j]) {
                    *ysize=chainResY[j];
                    break;
               }
          }
          break;
     case VID_VESA:
          for (j=0 ; j < MAXVESAXRES ; j++) {
               if (*xsize <= vesaResX[j]) {
                    *xsize=vesaResX[j];
                    break;
               }
          }
          for (j=0 ; j < MAXVESAYRES ; j++) {
               if (*ysize <= vesaResY[j]) {
                    *ysize=vesaResY[j];
                    break;
               }
          }
          break;
     case VID_SCREENBUFFER:
     case VID_TSENG:
     case VID_PARADISE:
     case VID_S3CHIPSET:
          *xsize=320;
          *ysize=200;
          break;
     default:
          return(-1);
     }
     return(i);
}

short resolutionCount,
     resolutionIndex;

void
showVideoResolutions(short n,short active)
{
     short hi,i,lo,x,xi,y,yi;
     short *r,*xr,*yr;
     short tempResolutions[]={0,0},
          tempResX[1]={320},
          tempResY[1]={200};

     switch (n) {
     case VID_CHAIN:
          r=&chainResolutions[0];
          xr=&chainResX[0];
          yr=&chainResY[0];
          resolutionCount=MAXCHAINRESOLUTIONS;
          break;
     case VID_VESA:
          r=&vesaResolutions[0];
          xr=&vesaResX[0];
          yr=&vesaResY[0];
          resolutionCount=MAXVESARESOLUTIONS;
          break;
     case VID_SCREENBUFFER:
     case VID_TSENG:
     case VID_PARADISE:
     case VID_S3CHIPSET:
          r=&tempResolutions[0];
          xr=&tempResX[0];
          yr=&tempResY[0];
          resolutionCount=1;
          break;
     }
     if (resolutionIndex >= resolutionCount) {
          resolutionIndex=resolutionCount-1;
     }
     for (i=0 ; i < resolutionCount ; i++) {
          xi=*(r+(i<<1));
          yi=*(r+(i<<1)+1);
          x=xr[xi];
          y=yr[yi];
          if (x == videoResX && y == videoResY) {
               resolutionIndex=i;
          }
     }
     lo=resolutionIndex-5;
     if (lo < 0) {
          lo=0;
     }
     hi=lo+11;
     _setbkcolor(active ? 11 : 3);
     _settextcolor(0);
     lprintat(47,5," %-15s ",videoModeList[videoMode]);
     _setbkcolor(1);
     _settextcolor(active ? 11 : 3);
     for (i=lo ; i < hi ; i++) {
          if (i >= resolutionCount) {
               lprintat(47,6+i-lo,"±               ±");
          }
          else {
               if (i == resolutionIndex) {
                    _setbkcolor(active ? 7 : 1);
                    _settextcolor(15);
               }
               xi=*(r+(i<<1));
               yi=*(r+(i<<1)+1);
               lprintat(47,6+i-lo,"±  %4d x %4d  ±",xr[xi],yr[yi]);
               if (_gettextcolor() == 15) {
                    _setbkcolor(1);
                    _settextcolor(active ? 11 : 3);
               }
          }
     }
     lprintat(47,6+i-lo,"±±±±±±±±±±±±±±±±±");
}

void
showVideoModes(short active)
{
     short i;

     _setbkcolor(1);
     _settextcolor(active ? 11 : 3);
     lprintat(16,5,"±±±±±±±±±±±±±±±±±±");
     for (i=0 ; i < MAXVIDMODES ; i++) {
          if (videoMode == i) {
               _setbkcolor(active ? 7 : 1);
               _settextcolor(15);
          }
          lprintat(16,6+(i*2),"±  %-12s  ±",videoModeList[i]);
          if (_gettextcolor() == 15) {
               showVideoResolutions(i,active ? 0 : 1);
               _setbkcolor(1);
               _settextcolor(active ? 11 : 3);
          }
          lprintat(16,6+(i*2)+1,"±                ±");
     }
     lprintat(16,5+(i*2),"±±±±±±±±±±±±±±±±±±");
}

void
showSelectedVideoMode(void)
{
     _displaycursor(_GCURSORON);
     lprintf("\n");
     lprintf("Graphics mode: %s\n",videoModeList[videoMode]);
     lprintf("   Resolution: %d x %d\n",videoResX,videoResY);
     lprintf("\n");
     lprintf("Loading...");
}

void
checkUserOptions(void)
{
     int  choosing=0,more=1;
     short oldVideoMode,oldVideoResX,oldVideoResY;

     oldVideoMode=videoMode;
     oldVideoResX=videoResX;
     oldVideoResY=videoResY;
     _displaycursor(_GCURSOROFF);
     _settextwindow(3,1,25,80);
     _setbkcolor(1);
     _clearscreen(_GWINDOW);
     lprintat(-1,10," TAB ");
     lprintat(-1,19,"Press ENTER to use the highlighted video");
     lprintat(-1,20,"mode and resolution or ESC to use the default");
     lprintat(-1,21,"of %s @ %dx%d",videoModeList[oldVideoMode],
              oldVideoResX,oldVideoResY);
     do {
          _setbkcolor(1);
          _settextcolor(11);
          switch (choosing) {
          case 0:
               _settextcolor(10);
               lprintat(-1,1," Use the UP/DOWN arrow keys ");
               lprintat(-1,2,"   to select a video mode   ");
               break;
          case 1:
               _settextcolor(13);
               lprintat(-1,1," Use the UP/DOWN arrow keys ");
               lprintat(-1,2,"to select a video resolution");
               break;
          }
          showVideoModes(choosing ? 0 : 1);
          switch (getch()) {
          case 0:
               switch (getch()) {
               case 0x4B:     // left arrow
                    if (choosing == 1) {
                         choosing=0;
                    }
                    break;
               case 0x4D:     // right arrow
                    if (choosing == 0) {
                         choosing=1;
                    }
                    break;
               case 0x50:     // down arrow
                    switch (choosing) {
                    case 0:
                         videoMode++;
                         if (videoMode >= MAXVIDMODES) {
                              videoMode=0;
                         }
                         break;
                    case 1:
                         resolutionIndex++;
                         if (resolutionIndex >= resolutionCount) {
                              resolutionIndex=resolutionCount-1;
                         }
                         switch (videoMode) {
                         case VID_CHAIN:
                              videoResX=chainResX[chainResolutions[resolutionIndex*2]];
                              videoResY=chainResY[chainResolutions[(resolutionIndex*2)+1]];
                              break;
                         case VID_VESA:
                              videoResX=vesaResX[vesaResolutions[resolutionIndex*2]];
                              videoResY=vesaResY[vesaResolutions[(resolutionIndex*2)+1]];
                              break;
                         case VID_SCREENBUFFER:
                         case VID_TSENG:
                         case VID_PARADISE:
                         case VID_S3CHIPSET:
                              videoResX=320;
                              videoResY=200;
                              break;
                         }
                         break;
                    }
                    break;
               case 0x48:     // up arrow
                    switch (choosing) {
                    case 0:
                         videoMode--;
                         if (videoMode < 0) {
                              videoMode=MAXVIDMODES-1;
                         }
                         break;
                    case 1:
                         resolutionIndex--;
                         if (resolutionIndex < 0) {
                              resolutionIndex=0;
                         }
                         switch (videoMode) {
                         case VID_CHAIN:
                              videoResX=chainResX[chainResolutions[resolutionIndex*2]];
                              videoResY=chainResY[chainResolutions[(resolutionIndex*2)+1]];
                              break;
                         case VID_VESA:
                              videoResX=vesaResX[vesaResolutions[resolutionIndex*2]];
                              videoResY=vesaResY[vesaResolutions[(resolutionIndex*2)+1]];
                              break;
                         case VID_SCREENBUFFER:
                         case VID_TSENG:
                         case VID_PARADISE:
                         case VID_S3CHIPSET:
                              videoResX=320;
                              videoResY=200;
                              break;
                         }
                         break;
                    }
                    break;
               }
               break;
          case 0x09:     // TAB key
               choosing^=1;
               break;
          case 0x0D:
               more=0;
               switch (videoMode) {
               case VID_CHAIN:
                    videoResX=chainResX[chainResolutions[resolutionIndex*2]];
                    videoResY=chainResY[chainResolutions[(resolutionIndex*2)+1]];
                    break;
               case VID_VESA:
                    videoResX=vesaResX[vesaResolutions[resolutionIndex*2]];
                    videoResY=vesaResY[vesaResolutions[(resolutionIndex*2)+1]];
                    break;
               case VID_SCREENBUFFER:
               case VID_TSENG:
               case VID_PARADISE:
               case VID_S3CHIPSET:
                    videoResX=320;
                    videoResY=200;
                    break;
               }
               break;
          case 0x1B:
               more=0;
               videoMode=oldVideoMode;
               videoResX=oldVideoResX;
               videoResY=oldVideoResY;
               break;
          }
     } while (more);
     _setbkcolor(1);
     _settextcolor(14);
     _clearscreen(_GWINDOW);
}

void
showCommandLineOptionsAndExit(void)
{
     short i;

     lprintf("\n");
     lprintf("Command line options:\n");
     lprintf("\n");
     for (i=0 ; i < MAXPARMS ; i++) {
          lprintf("     /%-16s %s\n",parmList[i],parmDesc[i]);
     }
     lprintf("\n");
     crash("");
}

void
checkParms(void)
{
     int  argc,i,j;
     char **argv,ext[_MAX_EXT];

     argc=_argc;
     argv=_argv;
     if (argc < 2) {
          return;
     }
     for (j=0 ; j < MAXPARMS ; j++) {
          for (i=0 ; i < argc ; i++) {
               if (argv[i][0] != '-' && argv[i][0] != '/') {
                    continue;
               }
               if (stricmp(argv[i]+1,parmList[j]) == 0) {
                    switch (j) {
                    case PARM_PLAYERS:
                         if (i+1 < argc) {
                              waitplayers=atoi(argv[i+1]);
                         }
                         break;
                    case PARM_LEVEL:
                         if (i+1 < argc) {
                              strcpy(&mapFileName,argv[i+1]);
                              _splitpath(mapFileName,NULL,NULL,NULL,ext);
                              if (strlen(ext) == 0) {
                                   strcat(mapFileName,".MAP");
                              }
                              bypassLevelFlag=1;
                         }
                         break;
                    case PARM_EDIT:
                         editorEnabledFlag=1;
                         noWaitFlag=1;
                         break;
                    case PARM_NETGAME:
                         gameType=GAMETYPE_NETGAME;
                         break;
                    case PARM_MDMGAME:
                         gameType=GAMETYPE_MODEM;
                         break;
                    case PARM_MPGAME:
                         gameType=GAMETYPE_MPATH;
                         break;
                    case PARM_NOSOUND:
                         noSoundFlag=1;
                         break;
                    case PARM_NOWAIT:
                         noWaitFlag=1;
                         break;
                    case PARM_CONFIG:
                         configSoundFlag=1;
                         break;
                    case PARM_NOENEMIES:
                         noEnemiesFlag=1;
                         break;
                    case PARM_VIDMODE:
                         configVideoFlag=1;
                         break;
                    case PARM_QUESTIONMARK:
                    case PARM_HELP:
                         showCommandLineOptionsAndExit();
                         break;
                    case PARM_DEBUG:
                         debugModeFlag=1;
                         break;
                    case PARM_DUMP:
                         dumpDebugInfoFlag=1;
                         break;
                    case PARM_DEATHMATCH:
                         gameMode=GAMEMODE_DEATHMATCH;
                         break;
                    case PARM_WEAPONEDIT:
                         weaponEditFlag=1;
                         break;
                    }
               }
          }
     }
}

void
gameMain(void)
{
     if (setjmp(env)) {
          return;
     }
     GAM_initGame();
     longjmp(env,1);
}

long faketimercalls,
     fakeClock;

void
faketimerhandler(void)
{
     if (totalclock >= fakeClock) {
          fakeClock=totalclock+TICWAITPERFRAME;
          if (playingGameFlag) {
               MUL_getInput();
          }
          else if (editorEnabledFlag) {
               MUL_moveThings();
          }
          faketimercalls++;
     }
}

short
movesprite(short spritenum, long dx, long dy, long dz, long ceildist,
           long flordist, char cliptype)
{
     long daz,
          zoffs;
     short retval,
          dasectnum,
          datempshort;
     spritetype *spr;

     spr=spritePtr[spritenum];
     if ((spr->cstat&128) == 0) {
          zoffs=-((tilesizy[spr->picnum]*spr->yrepeat)<<1);
     }
     else {
          zoffs=0;
     }
     dasectnum=spr->sectnum;     // Can't modify sprite sectors directly
                                   // because of linked lists
     daz=spr->z+zoffs;         // Must do this if not using the new
                                   // centered centering (of course)
     retval=clipmove(&spr->x,&spr->y,&daz,&dasectnum,dx,dy,
                     ((long)spr->clipdist)<<2,ceildist,flordist,cliptype);
     if (dasectnum < 0) {
          retval=-1;
     }
     if ((dasectnum != spr->sectnum) && (dasectnum >= 0)) {
          changespritesect(spritenum,dasectnum);
     }

// Set the blocking bit to 0 temporarly so getzrange doesn't pick up
// its own sprite

     datempshort=spr->cstat;
     spr->cstat&=~1;
     getzrange(spr->x,spr->y,spr->z-1,spr->sectnum,
               &globhiz,&globhihit,&globloz,&globlohit,
               ((long)spr->clipdist)<<2,cliptype);
     if ((sectorPtr[spr->sectnum]->floorstat&0x02) != 0 ||
         (sectorPtr[spr->sectnum]->ceilingstat&0x02) != 0) {
          getzsofslope(spr->sectnum,spr->x,spr->y,&globhiz,&globloz);
     }
     spr->cstat=datempshort;
     daz=spr->z+zoffs+dz;
     if ((daz <= globhiz) || (daz > globloz)) {
          if (retval != 0) {
               return(retval);
          }
          return(16384+dasectnum);
     }
     spr->z=daz-zoffs;
     return(retval);
}

void
debugOut(long x,long y,char *fmt,...)
{
     va_list argptr;

     if (debugline < MAXDEBUGLINES) {
          va_start(argptr,fmt);
          vsprintf(&debugstr[debugline],fmt,argptr);
          va_end(argptr);
          debugx[debugline]=x;
          debugy[debugline]=y+(debugline<<3);
          debugline++;
     }
}

void
debugOutput(void)
{
     int  i;

     if (debugline > 0) {     
          if (qsetmode == 200L) {
               for (i=0 ; i < debugline ; i++) {
                    printext256(debugx[i],debugy[i],P256COLOR,-1,&debugstr[i],1);
               }
          }
          debugline=0;
     }
}

void
debugFPOut(char *fmt,...)
{
     va_list argptr;

     if (dbgfp == NULL) {
          return;
     }
     va_start(argptr,fmt);
     vfprintf(dbgfp,fmt,argptr);
     va_end(argptr);
}

