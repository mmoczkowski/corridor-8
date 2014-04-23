/***************************************************************************
 *   CORR7MEN.C - Game menu related functions for Corridor 7
 *
 *                                                     04/17/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include "keycodes.h"
#include <ctype.h>
#include <memcheck.h>

#define   MAXMENUCHOICES 24
#define   FRAMETILE      4094

char scantoasc[128]={
       0,  0,'1','2','3','4','5','6','7','8','9','0','-','=',  0,  0,
     'q','w','e','r','t','y','u','i','o','p','[',']',  0,  0,'a','s',
     'd','f','g','h','j','k','l',';', 39,'`',  0, 92,'z','x','c','v',
     'b','n','m',',','.','/',  0,'*',  0, 32,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,'7','8','9','-','4','5','6','+','1',
     '2','3','0','.',  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

char scantoascwithshift[128]={
       0,  0,'!','@','#','$','%','^','&','*','(',')','_','+',  0,  0,
     'Q','W','E','R','T','Y','U','I','O','P','{','}',  0,  0,'A','S',
     'D','F','G','H','J','K','L',':', 34,'~',  0,'|','Z','X','C','V',
     'B','N','M','<','>','?',  0,'*',  0, 32,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,'7','8','9','-','4','5','6','+','1',
     '2','3','0','.',  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

enum {
     FONT_SMALLFONT,
     FONT_BIGFONT,
     FONT_NORMALTEXT,
     FONT_SMALLTEXT,
     NUMFONTS
};

int  configSoundFlag,
     inMenuFlag,
     minline,
     saveScreenShotFlag,
     updateStep;

long fontScale[NUMFONTS]={49152L,65536L,32768L},
     fontXSize[NUMFONTS]={8L,13L,8L},
     fontYSize[NUMFONTS]={12L,16L,8L};

enum {
     MENUOPTION_NONE,
     MENUOPTION_BOOLEAN,
     MENUOPTION_SLIDER,
     MENUOPTION_BULLET,
     MENUOPTION_GRAPHIC,
     MENUOPTION_LEFTJUST,
     MENUOPTION_RIGHTJUST
};

struct menuOption {
     char active;
     short type;
     char *label;
     long misc1;
     long misc2;
     short column;
     short line;
     short font;
};

unsigned long menuActiveBits;

int  *menuCur,
     menuNum,
     menuOpt;

struct menuOption *menuArr[32];

enum {
     MMENU_NEWMISSION,
     MMENU_OPTIONS,
     MMENU_RETRIEVE,
     MMENU_STORE,
     MMENU_CREDITS,
     MMENU_DEMO,
     MMENU_ORDER,
     MMENU_QUIT,
     NUMMENUOPTIONS
};

int  curMenu;

struct menuOption menuNamesArr[NUMMENUOPTIONS]={
     {1,MENUOPTION_NONE,"NEW MISSION",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"OPTIONS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"RETRIEVE MISSION",0,0,0,0,FONT_BIGFONT},
     {0,MENUOPTION_NONE,"STORE MISSION",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"CREDITS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"BACK TO DEMO",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"HOW TO ORDER",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"QUIT",0,0,0,0,FONT_BIGFONT}
},*menuNames[NUMMENUOPTIONS];

struct menuOption menu2NamesArr[NUMMENUOPTIONS]={
     {1,MENUOPTION_NONE,"NEW MISSION",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"OPTIONS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"RETRIEVE MISSION",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"STORE MISSION",0,0,0,0,FONT_BIGFONT},
     {0,MENUOPTION_NONE,"CREDITS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"RESUME MISSION",0,0,0,0,FONT_BIGFONT},
     {0,MENUOPTION_NONE,"HOW TO ORDER",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"QUIT",0,0,0,0,FONT_BIGFONT}
},*menu2Names[NUMMENUOPTIONS];

enum {
     EMENU_MISSION1,
     EMENU_MISSION2,
     EMENU_MISSION3,
     NUMMISSIONS
};

int  curMission;

struct menuOption missionNamesArr[NUMMISSIONS]={
     {1,MENUOPTION_NONE,"MISSION 1",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"MISSION 2",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"MISSION 3",0,0,0,0,FONT_BIGFONT}
},*missionNames[NUMMISSIONS];

int  curSkill=1;

struct menuOption skillNamesArr[NUMSKILLS]={
     {1,MENUOPTION_NONE,"CORPORAL",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"LIEUTENANT",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"CAPTAIN",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"MAJOR",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"PRESIDENT",0,0,0,0,FONT_BIGFONT}
},*skillNames[NUMSKILLS];

int  curActor;

enum {
     AMENU_ALLIEDSPACEMARINE,
     AMENU_ALLIEDFEMALEMARINE,
     NUMACTORS
};

struct menuOption alliedActorNamesArr[NUMACTORS]={
     {1,MENUOPTION_GRAPHIC,"SPACE MARINE",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_GRAPHIC,"FEMALE MARINE",0,0,0,0,FONT_BIGFONT}
},*alliedActorNames[NUMACTORS];

enum {
     AMENU_AXISSPACEMARINE,
     AMENU_AXISFEMALEMARINE,
};

struct menuOption axisActorNamesArr[NUMACTORS]={
     {1,MENUOPTION_GRAPHIC,"SPACE MARINE",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_GRAPHIC,"FEMALE MARINE",0,0,0,0,FONT_BIGFONT}
},*axisActorNames[NUMACTORS];

int  curAlliance;

enum {
     AMENU_ALLIED,
     AMENU_AXIS,
     NUMALLIANCES
};

struct menuOption allianceNamesArr[NUMALLIANCES]={
     {1,MENUOPTION_NONE,"ALLIED SIDE",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"AXIS SIDE",0,0,0,0,FONT_BIGFONT}
},*allianceNames[NUMALLIANCES];

enum {
     OMENU_GRAPHICS,
     OMENU_SOUND,
     OMENU_CONTROLLER,
     OMENU_EXIT,
     NUMOPTIONS
};

int  curOption;

struct menuOption optionNamesArr[NUMOPTIONS]={
     {1,MENUOPTION_NONE,"GRAPHICS OPTIONS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"SOUND OPTIONS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"CONTROLLER OPTIONS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_NONE,"PREVIOUS MENU",0,0,0,0,FONT_BIGFONT}
},*optionNames[NUMOPTIONS];

#define   NUMSAVESPOTS        10
#define   MAXSAVENAMESIZE     17

int  curSaveSpot;

struct menuOption saveGameNamesArr[NUMSAVESPOTS],
     *saveNames[NUMSAVESPOTS];

char saveGameName[NUMSAVESPOTS][MAXSAVENAMESIZE];

enum {
     GMENU_GORELEVEL,
     GMENU_AUTOHORIZON,
     GMENU_AUTOAIM,
     GMENU_DETAILLEVEL,
     GMENU_EXIT,
     NUMGRAPHICOPTIONS
};

int  curGraphics;

struct menuOption graphicNamesArr[NUMGRAPHICOPTIONS]={
     {1,MENUOPTION_SLIDER,"GORE LEVEL",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_BOOLEAN,"AUTO HORIZON",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_BOOLEAN,"AUTO AIM",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_SLIDER,"DETAIL LEVEL",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"PREVIOUS MENU",0,0,0,0,FONT_BIGFONT}
},*graphicNames[NUMGRAPHICOPTIONS];

enum {
     SMENU_SOUNDVOL,
     SMENU_MUSICVOL,
     SMENU_STEREO,
     SMENU_EXIT,
     NUMSOUNDOPTIONS
};

int  curSound;

struct menuOption soundNamesArr[NUMSOUNDOPTIONS]={
     {1,MENUOPTION_SLIDER,"DIGITAL VOLUME",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_SLIDER,"MUSIC VOLUME",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_BOOLEAN,"REVERSE STEREO",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"PREVIOUS MENU",0,0,0,0,FONT_BIGFONT}
},*soundNames[NUMSOUNDOPTIONS];

enum {
     CMENU_KEYBOARD,
     CMENU_MOUSE,
     CMENU_JOYSTICKS,
     CMENU_VRHEADSETS,
     CMENU_EXIT,
     NUMCONTROLLEROPTIONS
};

int  curController;

struct menuOption controllerNamesArr[NUMCONTROLLEROPTIONS]={
     {1,MENUOPTION_LEFTJUST,"KEYBOARD",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"MOUSE",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"JOYSTICKS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"VR HEADSETS",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"PREVIOUS MENU",0,0,0,0,FONT_BIGFONT}
},*controllerNames[NUMCONTROLLEROPTIONS];

char actionName[MAXACTIONS][30];

int  curKeyboard;

struct menuOption keyboardNamesArr[MAXACTIONS]={
     {1,MENUOPTION_NONE,&actionName[ACT_FORWARD],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_BACKWARD],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_LEFT],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_RIGHT],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_RUN],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_STRAFE],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_USEWEAP],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_USEITEM],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_JUMP],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_CROUCH],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_LOOKUP],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_LOOKDN],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_CENTER],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_STRAFEL],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_STRAFER],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_MAP],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_ZOOMIN],0,0,0,0,FONT_NORMALTEXT},
     {1,MENUOPTION_NONE,&actionName[ACT_ZOOMOUT],0,0,0,0,FONT_NORMALTEXT}
},*keyboardNames[MAXACTIONS];

enum {
     MMENU_ENABLE,
     MMENU_CONFIGURE,
     MMENU_MOUSEXSENSE,
     MMENU_MOUSEYSENSE,
     MMENU_EXIT,
     NUMMOUSEOPTIONS
};

int  curMouse,
     curMouseOptions;

struct menuOption mouseNamesArr[NUMMOUSEOPTIONS]={
     {1,MENUOPTION_BOOLEAN,"MOUSE ENABLED",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"CONFIGURE MOUSE",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_SLIDER,"MOUSE X SENSITIVITY",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_SLIDER,"MOUSE Y SENSITIVITY",0,0,0,0,FONT_BIGFONT},
     {1,MENUOPTION_LEFTJUST,"PREVIOUS MENU",0,0,0,0,FONT_BIGFONT}
},*mouseNames[NUMMOUSEOPTIONS];

#if 0
char soundCardName[9][30];

int  autoDetectFlag,
     curDMA,
     curIRQ,
     curPort,
     curSoundCard,
     firstSoundCard,
     lastSoundCard;

struct menuOption soundCardArr[9]={
     {0,MENUOPTION_NONE,&soundCardName[0],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[1],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[2],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[3],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[4],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[5],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[6],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[7],0,0,0,0,FONT_SMALLFONT},
     {0,MENUOPTION_NONE,&soundCardName[8],0,0,0,0,FONT_SMALLFONT}
},*soundCardNames[9];
#endif

int  curWeaponEdit;

#if 0
extern
char *sDIGIDeviceName[];

extern
W32  wDIGIDeviceID,
     wMIDIDeviceID,
     wDIGIIndex,
     wMIDIIndex;

extern
W16  wDIGIPortList[_SETUP_MAX_DEVICES],
     wDIGIIRQList[_SETUP_MAX_DEVICES],
     wDIGIDMAList[_SETUP_MAX_DEVICES];
#endif

extern
int  stereoSwapFlag;

#if 0
extern
_SOS_CAPABILITIES sDIGICapabilities;
#endif

extern
_SOS_DIGI_DRIVER sDIGIDriver;

void
MEN_stripTrailingBlanks(char *stg)
{
     int  j,len;

     len=strlen(stg);
     if (len > 0) {
          j=len-1;
          while (stg[j] == ' ' && j > 0) {
               stg[j]=0;
               j--;
          }
          strupr(stg);
     }
}

void
MEN_initMenus(void)
{
     int  i;

     for (i=0 ; i < NUMMENUOPTIONS ; i++) {
          menuNames[i]=&menuNamesArr[i];
          menu2Names[i]=&menu2NamesArr[i];
     }
     for (i=0 ; i < NUMALLIANCES ; i++) {
          allianceNames[i]=&allianceNamesArr[i];
     }
     for (i=0 ; i < NUMMISSIONS ; i++) {
          missionNames[i]=&missionNamesArr[i];
     }
     for (i=0 ; i < NUMSKILLS ; i++) {
          skillNames[i]=&skillNamesArr[i];
     }
     for (i=0 ; i < NUMACTORS ; i++) {
          alliedActorNames[i]=&alliedActorNamesArr[i];
          axisActorNames[i]=&axisActorNamesArr[i];
     }
     for (i=0 ; i < NUMOPTIONS ; i++) {
          optionNames[i]=&optionNamesArr[i];
     }
     for (i=0 ; i < NUMGRAPHICOPTIONS ; i++) {
          graphicNames[i]=&graphicNamesArr[i];
     }
     for (i=0 ; i < NUMCONTROLLEROPTIONS ; i++) {
          controllerNames[i]=&controllerNamesArr[i];
     }
     for (i=0 ; i < MAXACTIONS ; i++) {
          keyboardNames[i]=&keyboardNamesArr[i];
     }
     for (i=0 ; i < NUMMOUSEOPTIONS ; i++) {
          mouseNames[i]=&mouseNamesArr[i];
     }
     for (i=0 ; i < NUMSOUNDOPTIONS ; i++) {
          soundNames[i]=&soundNamesArr[i];
     }
#if 0
     for (i=0 ; i < 9 ; i++) {
          soundCardNames[i]=&soundCardArr[i];
     }
#endif
}

void
MEN_printext(long x,long y,int shade,char *stg,int font)
{
     int  i,len,pic;

     if (font == FONT_NORMALTEXT || font == FONT_SMALLTEXT) {
          if (shade != -1) {
               shade=-1;
          }
          else {
               shade=0;
          }
          switch (font) {
          case FONT_NORMALTEXT:
               i=0;
               break;
          case FONT_SMALLTEXT:
               i=1;
               break;
          }
          printext256(x,y,REDBASE,-1,stg,i);
          printext256(x-1,y-1,REDBASE+16,shade,stg,i);
          return;
     }
     len=strlen(stg);
     for (i=0 ; i < len ; i++) {
          pic=0;
          if (isalpha(stg[i])) {
               pic=graphicLetAPic+(stg[i]-'A');
          }
          else if (isdigit(stg[i])) {
               pic=graphicNum0Pic+(stg[i]-'0');
          }
          else {
               x+=fontXSize[font];
          }
          if (pic != 0) {
               rotatesprite(x<<16,y<<16,fontScale[font],0,pic,shade,0,2+16,
                            0L,0L,xdim-1L,ydim-1L);
               x+=fontXSize[font];
          }
     }
}

void
MEN_showHeader(char *hdr)
{
     rotatesprite(320L<<15,tilesizy[menuTitleBarPic]<<16,65536L,0,
                  menuTitleBarPic,0,0,2,0L,0L,xdim-1L,ydim-1L);
     MEN_printext((320L>>1)-((strlen(hdr)*fontXSize[FONT_BIGFONT])>>1),
                  (tilesizy[menuTitleBarPic]>>1)+2L,-1,hdr,FONT_BIGFONT);
}

void
MEN_displayMenu(int cur,short items,short backpic,char *hdr,
                struct menuOption *men[])
{
     int  activeItems,i,len=0,shade;
     long x,y;
     char *ptr;

     if (backpic != 0) {
          if (!ENG_inSetupVideoMode()) {
               ENG_setupVideo();
          }
     }
     if (playingGameFlag) {
          if (backpic == 0 && !GAM_inSetupVideoMode()) {
               GAM_setupVideo(viewSize);
          }
          if (saveScreenShotFlag) {
               MEN_setViewToTile(cur);
          }
          GAM_doFrame();
          if (saveScreenShotFlag) {
               MEN_setViewBack();
          }
     }
     if (backpic != 0) {
          rotatesprite(320L<<15,200L<<15,65536L,0,backpic,0,0,2,
                       0L,0L,xdim-1L,ydim-1L);
     }
     MEN_showHeader(hdr);
     activeItems=0;
     x=0L;
     for (i=0 ; i < items ; i++) {
          if (men[i]->active) {
               activeItems++;
               switch (men[i]->type) {
               case MENUOPTION_SLIDER:
                    x+=(fontYSize[men[i]->font]*2);
                    break;
               default:
                    x+=fontYSize[men[i]->font];
                    break;
               }
          }
     }
     y=(200L>>1)-(x>>1)+10L;
     minline=y;
     for (i=0 ; i < items ; i++) {
          shade=8;
          if (cur == i) {
               shade=-1;
          }
          if (men[i]->active) {
               men[i]->line=y;
               ptr=men[i]->label;
               switch (men[i]->type) {
               default:
               case MENUOPTION_NONE:
               case MENUOPTION_BULLET:
               case MENUOPTION_GRAPHIC:
                    len=strlen(ptr);
                    x=(320L>>1)-((len*fontXSize[men[i]->font])>>1);
                    men[i]->column=x;
                    MEN_printext(x,y,shade,ptr,men[i]->font);
                    y+=fontYSize[men[i]->font];
                    break;
               case MENUOPTION_LEFTJUST:
                    len=tilesizx[menuTitleBarPic]>>1;
                    x=(320L>>1)-len+fontXSize[men[i]->font];
                    men[i]->column=x;
                    MEN_printext(x,y,shade,ptr,men[i]->font);
                    y+=fontYSize[men[i]->font];
                    break;
               case MENUOPTION_SLIDER:
                    len=tilesizx[menuTitleBarPic]>>1;
                    x=(320L>>1)-len+fontXSize[men[i]->font];
                    men[i]->column=x;
                    MEN_printext(x,y,shade,ptr,men[i]->font);
                    y+=fontYSize[men[i]->font];
                    x=160L-(tilesizx[menuSliderBarPic]>>1);
                    rotatesprite(x<<16,y<<16,65536L,0,menuSliderBarPic,
                                 0,0,2+16,0L,0L,xdim-1L,ydim-1L);
                    x+=(men[i]->misc1*((tilesizx[menuSliderBarPic]-8)/
                        men[i]->misc2));
                    rotatesprite((x+3)<<16,(y+1)<<16,65536L,0,menuSliderKnobPic,
                                 0,0,2+16,0L,0L,xdim-1L,ydim-1L);
                    y+=fontYSize[men[i]->font];
                    break;
               case MENUOPTION_RIGHTJUST:
                    len=strlen(ptr);
                    x=320L-(len*fontXSize[men[i]->font]);
                    x-=fontXSize[men[i]->font];
                    men[i]->column=x;
                    MEN_printext(x,y,shade,ptr,men[i]->font);
                    y+=fontYSize[men[i]->font];
                    break;
               case MENUOPTION_BOOLEAN:
                    len=tilesizx[menuTitleBarPic]>>1;
                    x=(320L>>1)-len+fontXSize[men[i]->font];
                    men[i]->column=x;
                    MEN_printext(x,y,shade,ptr,men[i]->font);
                    if (men[i]->misc1) {
                         ptr="ON ";
                    }
                    else {
                         ptr="OFF";
                    }
                    x=(320L>>1)+len-(fontXSize[men[i]->font]*strlen(ptr))
                                    -fontXSize[men[i]->font];
                    MEN_printext(x,y,shade,ptr,men[i]->font);
                    y+=fontYSize[men[i]->font];
                    break;
               }
          }
     }
}

int
MEN_updateMenuTimer(void)
{
     static long updateclock;

     if (totalclock > updateclock) {
          updateStep++;
          if (menuMarkerBegPic+updateStep >= menuMarkerEndPic) {
               updateStep=0L;
          }
          updateclock=totalclock+TMR_getSecondFraction(16);
          return(1);
     }
     return(0);
}

void
MEN_showSelectedLine(short n,struct menuOption *men[])
{
     short pic=0;
     long x,x2=0L,y=0L;

     y=men[n]->line+(tilesizy[menuMarkerBegPic]>>1)
                   -(tilesizy[menuMarkerBegPic]>>2);
     switch (men[n]->type) {
     case MENUOPTION_RIGHTJUST:
          pic=men[n]->misc1;
     case MENUOPTION_LEFTJUST:
     case MENUOPTION_BOOLEAN:
          x=men[n]->column-(tilesizx[menuMarkerBegPic]>>1);
          break;
     default:
          x=(320L>>1)-(tilesizx[menuTitleBarPic]>>1);
          x2=(320L>>1)+(tilesizx[menuTitleBarPic]>>1);
          break;
     }
     rotatesprite(x<<16,y<<16,fontScale[men[n]->font],0,
                  menuMarkerBegPic+updateStep,0,0,2,
                  0L,0L,xdim-1L,ydim-1L);
     if (x2 != 0L) {
          rotatesprite(x2<<16,y<<16,fontScale[men[n]->font],0,
                       menuMarkerBegPic+updateStep,0,0,2,
                       0L,0L,xdim-1L,ydim-1L);
     }
     if (pic != 0) {
          x=(320L>>2)+10L;
          rotatesprite(x<<16,(200L>>1)<<16,men[n]->misc2,512,pic,0,0,2+4+64,
                       0L,0L,xdim-1L,ydim-1L);
     }
}

int
MEN_getInput(void)
{
     int  k;
     static int mouseTics;

     k=KBD_keyPressed();
     mouseTics+=globalMouseY;
     if (mouseTics < -200) {
          k=K_UP;
          mouseTics=0;
     }
     else if (mouseTics > 200) {
          k=K_DOWN;
          mouseTics=0;
     }
     switch (k&0x7F) {
     case K_UP:
          do {
               menuOpt--;
               if (menuOpt < 0) {
                    menuOpt=menuNum-1;
               }
          } while ((menuActiveBits&(1L<<menuOpt)) == 0);
          SND_playMenuChooseSound();
          break;
     case K_DOWN:
          do {
               menuOpt++;
               if (menuOpt >= menuNum) {
                    menuOpt=0;
               }
          } while ((menuActiveBits&(1L<<menuOpt)) == 0);
          SND_playMenuChooseSound();
          break;
     case K_RIGHT:
          switch (menuArr[menuOpt]->type) {
          case MENUOPTION_SLIDER:
               if (menuArr[menuOpt]->misc1 < menuArr[menuOpt]->misc2-1) {
                    menuArr[menuOpt]->misc1++;
                    SND_playMenuSlideSound();
                    inMenuFlag=0;
               }
               break;
          }
          break;
     case K_LEFT:
          switch (menuArr[menuOpt]->type) {
          case MENUOPTION_SLIDER:
               if (menuArr[menuOpt]->misc1 > 0) {
                    menuArr[menuOpt]->misc1--;
                    SND_playMenuSlideSound();
                    inMenuFlag=0;
               }
               break;
          }
          break;
     default:
          if (k > 0) {
               if (k == K_SPACE || k == K_ENTER) {
                    *menuCur=menuOpt;
                    inMenuFlag=0;
                    SND_playMenuDoneSound();
               }
               else if (k == K_ESC) {
                    inMenuFlag=-1;
                    SND_playMenuQuitSound();
               }
          }
          break;
     }
     return(k);
}

void
MEN_doMenuFrame(int opt,int num,short pic,char *hdr,struct menuOption *men[])
{
     int  update=0;

     update=MEN_updateMenuTimer();
     if (update || playingGameFlag) {
          MEN_displayMenu(opt,num,pic,hdr,men);
          MEN_showSelectedLine(opt,men);
          nextpage();
          GFX_fadeIn(32);
          update=0;
     }
}

int
MEN_doMenu(int *cur,int num,short backpic,char *hdr,struct menuOption *men[])
{
     int  i,lastMenuOpt=-1;

     inMenuFlag=1;
     menuCur=cur;
     menuOpt=*menuCur;
     menuNum=num;
     menuActiveBits=0L;
     for (i=0 ; i < num ; i++) {
          menuArr[i]=men[i];
          if (men[i]->active) {
               menuActiveBits|=(1L<<i);
          }
     }
     KBD_resetKeys();
     while (inMenuFlag == 1) {
          MEN_doMenuFrame(menuOpt,menuNum,backpic,hdr,men);
          if (!playingGameFlag) {
               MEN_getInput();
               GFX_animatePalette();
          }
          if (men == saveNames && menuOpt != lastMenuOpt) {
               MEN_loadScreenShot(menuOpt);
          }
          lastMenuOpt=menuOpt;
     }
     *menuCur=menuOpt;
     if (inMenuFlag == -1) {
          inMenuFlag=0;
          return(0);
     }
     return(1);
}

int
MEN_graphicOptions(void)
{
     int  pic=0,
          r=1;

     while (r) {
          pic=menuBackgroundPic;
          graphicNames[GMENU_AUTOHORIZON]->misc1=horizCheckFlag;
          graphicNames[GMENU_AUTOAIM]->misc1=WEP_getAutoAimSetting();
          graphicNames[GMENU_GORELEVEL]->misc1=GAM_getGoreLevel();
          graphicNames[GMENU_GORELEVEL]->misc2=MAXGORECHOICES;
          graphicNames[GMENU_DETAILLEVEL]->misc1=GAM_getDetailLevel();
          graphicNames[GMENU_DETAILLEVEL]->misc2=MAXDETAILLEVEL;
          r=MEN_doMenu(&curGraphics,NUMGRAPHICOPTIONS,pic,"GRAPHICS OPTIONS",
                       graphicNames);
          if (r) {
               switch (curGraphics) {
               case GMENU_GORELEVEL:
                    GAM_setGoreLevel(graphicNames[curGraphics]->misc1);
                    break;
               case GMENU_AUTOHORIZON:
                    horizCheckFlag^=1;
                    break;
               case GMENU_AUTOAIM:
                    WEP_toggleAutoAimSetting();
                    break;
               case GMENU_DETAILLEVEL:
                    GAM_setDetailLevel(graphicNames[curGraphics]->misc1);
                    GAM_scanDetailSprites();
                    break;
               case GMENU_EXIT:
                    r=0;
                    break;
               }
          }
     }
     return(r);
}

int
MEN_soundOptions(void)
{
     int  pic=0,
          r=1;

     while (r) {
          pic=menuBackgroundPic;
          soundNames[SMENU_STEREO]->misc1=stereoSwapFlag;
          soundNames[SMENU_SOUNDVOL]->misc1=SND_getSoundVolume();
          soundNames[SMENU_SOUNDVOL]->misc2=16;
#if 0
          soundNames[SMENU_MUSICVOL]->misc1=SND_getMusicVolume();
#endif
          soundNames[SMENU_MUSICVOL]->misc2=16;
          r=MEN_doMenu(&curSound,NUMSOUNDOPTIONS,pic,"SOUND OPTIONS",
                       soundNames);
          if (r) {
               switch (curSound) {
               case SMENU_SOUNDVOL:
                    SND_setSoundVolume(soundNames[curSound]->misc1);
                    break;
               case SMENU_MUSICVOL:
#if 0
                    SND_setMusicVolume(soundNames[curSound]->misc1);
#endif
                    break;
               case SMENU_STEREO:
                    stereoSwapFlag^=1;
                    if (stereoSwapFlag) {
                         sDIGIDriver.wDriverFormat|=_DRV_SWAP_CHANNELS;
                    }
                    else {
                         sDIGIDriver.wDriverFormat&=~_DRV_SWAP_CHANNELS;
                    }
                    break;
               case SMENU_EXIT:
                    r=0;
                    break;
               }
          }
     }
     return(r);
}

int
MEN_keyboardConfiguration(void)
{
     int  i,
          pic=0,
          r;

     for (i=0 ; i < MAXACTIONS ; i++) {
          sprintf(actionName[i],"%s %-6s",
                  controlArray[i].desc,keystrings[keys[i]&0x7F]);
          keyboardNames[i]->misc1=keys[i];
     }
     r=1;
     while (r) {
          pic=menuBackgroundPic;
          r=MEN_doMenu(&curKeyboard,MAXACTIONS,pic,"KEYBOARD CONFIG",
                       keyboardNames);
          if (r) {
               sprintf(actionName[curKeyboard],"%s %-6s",
                       controlArray[curKeyboard].desc,"");
               while ((i=KBD_keyPressed()) == 0) {
                    MEN_doMenuFrame(curKeyboard,MAXACTIONS,pic,
                                    "PRESS A KEY",keyboardNames);
               }
               sprintf(actionName[curKeyboard],"%s %-6s",
                       controlArray[curKeyboard].desc,keystrings[i&0x7F]);
               keyboardNames[curKeyboard]->misc1=i;
          }
     }
     for (i=0 ; i < MAXACTIONS ; i++) {
          keys[i]=keyboardNames[i]->misc1;
     }
     return(r);
}

//** NOTE: mouseConfiguration uses the keyboard menu arrays

int
MEN_mouseConfiguration(void)
{
     int  i,j,
          pic=0,
          r;

     for (i=0 ; i < MAXACTIONS ; i++) {
          keyboardNames[i]->misc1=0;
          for (j=1 ; j < MAXMOUSEBUTTONS ; j++) {
               if (configMouse[j] == i) {
                    sprintf(actionName[i],"%s %s",
                            controlArray[i].desc,
                            mouseControlLabel[j]);
                    keyboardNames[i]->misc1=j;
                    break;
               }
          }
          if (keyboardNames[i]->misc1 == 0) {
               sprintf(actionName[i],"%s %13s",controlArray[i].desc,"");
          }
     }
     r=1;
     while (r) {
          pic=menuBackgroundPic;
          r=MEN_doMenu(&curMouse,MAXACTIONS,pic,"MOUSE CONFIG",
                       keyboardNames);
          if (r) {
               sprintf(actionName[curMouse],"%s %13s",
                       controlArray[curMouse].desc,"");
               while ((i=KBD_keyPressed()) == 0) {
                    MEN_doMenuFrame(curMouse,MAXACTIONS,pic,
                                    "PRESS MOUSE BUTTON",keyboardNames);
               }
               if (globalMouseB != 0) {
                    if ((globalMouseB&0x01) != 0) {
                         i=1;
                    }
                    else if ((globalMouseB&0x02) != 0) {
                         i=3;
                    }
                    else {
                         i=2;
                    }
                    for (j=0 ; j < MAXACTIONS ; j++) {
                         if (keyboardNames[j]->misc1 == i) {
                              keyboardNames[j]->misc1=0;
                              sprintf(actionName[j],"%s %13s",
                                      controlArray[j].desc,"");
                              break;
                         }
                    }
                    sprintf(actionName[curMouse],"%s %s",
                            controlArray[curMouse].desc,
                            mouseControlLabel[i]);
                    keyboardNames[curMouse]->misc1=i;
               }
          }
     }
     for (i=0 ; i < MAXACTIONS ; i++) {
          j=keyboardNames[i]->misc1;
          if (j != 0 && j < MAXMOUSEBUTTONS) {
               configMouse[j]=i;
          }
     }
     return(r);
}

int
MEN_mouseOptions(void)
{
     int  pic=0,
          r=1;

     mouseNames[MMENU_ENABLE]->misc1=configMouse[0];
     mouseNames[MMENU_MOUSEXSENSE]->misc1=PLR_getMouseXSense();
     mouseNames[MMENU_MOUSEXSENSE]->misc2=MAXMOUSESENSE;
     mouseNames[MMENU_MOUSEYSENSE]->misc1=PLR_getMouseYSense();
     mouseNames[MMENU_MOUSEYSENSE]->misc2=MAXMOUSESENSE;
     while (r) {
          pic=menuBackgroundPic;
          r=MEN_doMenu(&curMouseOptions,NUMMOUSEOPTIONS,pic,
                       "MOUSE OPTIONS",mouseNames);
          if (r) {
               switch (curMouseOptions) {
               case MMENU_ENABLE:
                    mouseNames[curMouseOptions]->misc1^=1;
                    break;
               case MMENU_CONFIGURE:
                    MEN_mouseConfiguration();
                    break;
               case MMENU_EXIT:
                    r=0;
                    break;
               }
          }
     }
     configMouse[0]=mouseNames[MMENU_ENABLE]->misc1;
     PLR_setMouseXSense(mouseNames[MMENU_MOUSEXSENSE]->misc1);
     PLR_setMouseYSense(mouseNames[MMENU_MOUSEYSENSE]->misc1);
     return(r);
}

int
MEN_controllerOptions(void)
{
     int  pic=0,
          r=1;

     while (r) {
          pic=menuBackgroundPic;
          r=MEN_doMenu(&curController,NUMCONTROLLEROPTIONS,pic,
                       "CONTROLLER OPTIONS",controllerNames);
          if (r) {
               switch (curController) {
               case CMENU_KEYBOARD:
                    MEN_keyboardConfiguration();
                    break;
               case CMENU_MOUSE:
                    MEN_mouseOptions();
                    break;
               case CMENU_JOYSTICKS:
//                    MEN_joystickControllers();
                    break;
               case CMENU_VRHEADSETS:
//                    MEN_vrHeadsetControllers();
                    break;
               case CMENU_EXIT:
                    r=0;
                    break;
               }
          }
     }
     return(r);
}

int
MEN_gameOptions(void)              // setup/configure game options
{
     int  pic=0,
          r=1;

     while (r) {
          pic=menuBackgroundPic;
          r=MEN_doMenu(&curOption,NUMOPTIONS,pic,"GAME OPTIONS",optionNames);
          if (r) {
               switch (curOption) {
               case OMENU_GRAPHICS:
                    MEN_graphicOptions();
                    break;
               case OMENU_SOUND:
                    MEN_soundOptions();
                    break;
               case OMENU_CONTROLLER:
                    MEN_controllerOptions();
                    break;
               case OMENU_EXIT:
                    r=0;
                    break;
               }
          }
     }
     return(r);
}

void
MEN_saveGameData(void)
{
     GAM_saveGame(curSaveSpot);
}

int
MEN_getSaveName(short pic,char *hdr)
{
     int  c,stat,x;
     char backup[MAXSAVENAMESIZE],
          locbuf[MAXSAVENAMESIZE],
          *ptr;

     strcpy(backup,saveNames[curSaveSpot]->label);
     strcpy(locbuf,backup);
     ptr=locbuf;
     MEN_stripTrailingBlanks(ptr);
     if (strcmp(ptr,"EMPTY") == 0) {
          memset(ptr,0,MAXSAVENAMESIZE);
     }
     x=strlen(ptr);
     sprintf(saveNames[curSaveSpot]->label,"%-16s",locbuf);
     KBD_resetKeys();
     inMenuFlag=1;
     saveScreenShotFlag=1;
     keyfifoplc=keyfifoend;
     MEN_getSaveScreenShot(curSaveSpot);
     do {
          MEN_doMenuFrame(curSaveSpot,NUMSAVESPOTS,pic,hdr,saveNames);
          while (keyfifoplc != keyfifoend) {
               c=keyfifo[keyfifoplc];
               stat=keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)];
               if (stat == 1) {
                    switch (c) {
                    case K_ESC:
                         strcpy(saveNames[curSaveSpot]->label,backup);
                         saveScreenShotFlag=0;
                         inMenuFlag=0;
                         return(0);
                    case K_BACKSPACE:
                         if (x > 0) {
                              ptr[x]=0;
                              ptr[x-1]=0;
                              x--;
                         }
                         break;
                    case K_ENTER:
                         saveScreenShotFlag=0;
                         inMenuFlag=0;
                         return(1);
                    default:
                         if (scantoasc[c] != 0) {
                              ptr[x++]=toupper(scantoasc[c]);
                              if (x >= MAXSAVENAMESIZE) {
                                   x=MAXSAVENAMESIZE-1;
                              }
                              ptr[x]=0;
                         }
                         break;
                    }
                    sprintf(saveNames[curSaveSpot]->label,"%-16s",locbuf);
               }
               keyfifoplc=(keyfifoplc+1)&(KEYFIFOSIZ-1);
          }
     } while (1);
}

void
MEN_allocFrameTile(void)
{
     walock[FRAMETILE]=255;
     if (waloff[FRAMETILE] == 0) {
          allocache(&waloff[FRAMETILE],320L*200L,&walock[FRAMETILE]);
     }
}

void
MEN_freeFrameTile(void)
{
     walock[FRAMETILE]=1;
}

int
MEN_saveGameOption(void)
{
     int  i,pic=0,r=1;
     FILE *fp;

     for (i=0 ; i < NUMSAVESPOTS ; i++) {
          saveNames[i]=&saveGameNamesArr[i];
          saveNames[i]->label=&saveGameName[i];
          sprintf(tempbuf,SAVENAMEFMT,i);
          if ((fp=fopen(tempbuf,"r+b")) != NULL) {
               GAM_fread(saveNames[i]->label,sizeof(char),MAXSAVENAMESIZE,fp);
               fclose(fp);
          }
          else {
               sprintf(tempbuf,"%-16s","EMPTY");
               strcpy(saveNames[i]->label,tempbuf);
          }
          saveNames[i]->active=1;
          saveNames[i]->type=MENUOPTION_RIGHTJUST;
          saveNames[i]->misc1=0;
          saveNames[i]->misc2=0;
          saveNames[i]->font=FONT_SMALLFONT;
     }
     MEN_allocFrameTile();
     while (r) {
          pic=menuBackgroundPic;
          r=MEN_doMenu(&curSaveSpot,NUMSAVESPOTS,pic,"SAVE GAME",saveNames);
          if (r) {
               if (MEN_getSaveName(pic,"SAVE GAME")) {
                    MEN_saveGameData();
                    MEN_saveScreenShot();
               }
               r=0;
          }
     }
     MEN_freeFrameTile();
     return(r);
}

int
MEN_getLoadName(void)
{
     sprintf(tempbuf,"%-16s","EMPTY");
     if (strcmp(saveNames[curSaveSpot]->label,tempbuf) != 0) {
          return(1);
     }
     return(0);
}

void
MEN_loadGameData(void)
{
     GAM_loadGame(curSaveSpot);
}

int
MEN_loadGameOption(void)
{
     int  i,pic=0,r=1;
     FILE *fp;

     for (i=0 ; i < NUMSAVESPOTS ; i++) {
          saveNames[i]=&saveGameNamesArr[i];
          saveNames[i]->label=&saveGameName[i];
          sprintf(tempbuf,SAVENAMEFMT,i);
          if ((fp=fopen(tempbuf,"r+b")) != NULL) {
               GAM_fread(saveNames[i]->label,sizeof(char),MAXSAVENAMESIZE,fp);
               fclose(fp);
          }
          else {
               sprintf(tempbuf,"%-16s","EMPTY");
               strcpy(saveNames[i]->label,tempbuf);
          }
          saveNames[i]->active=1;
          saveNames[i]->type=MENUOPTION_RIGHTJUST;
          saveNames[i]->misc1=0;
          saveNames[i]->misc2=0;
          saveNames[i]->font=FONT_SMALLFONT;
     }
     walock[FRAMETILE]=255;
     if (waloff[FRAMETILE] == 0) {
          allocache(&waloff[FRAMETILE],320L*200L,&walock[FRAMETILE]);
     }
     pic=menuBackgroundPic;
     r=MEN_doMenu(&curSaveSpot,NUMSAVESPOTS,pic,"LOAD GAME",saveNames);
     walock[FRAMETILE]=1;
     if (r) {
          if (MEN_getLoadName()) {
               MEN_loadGameData();
          }
     }
     return(r);
}

int
MEN_areYouSure(void)
{
     return(1);
}

int
MEN_mainMenu(void)
{
     int  pic=0,
          r;

     if (playingGameFlag) {
          curMenu=MMENU_STORE;
     }
     else {
          curMenu=MMENU_NEWMISSION;
     }
     while (inGameFlag) {
          if (gameType != GAMETYPE_SINGLE || (noWaitFlag && !playingGameFlag)
              || newMapLoadedFlag) {
               curMenu=MMENU_NEWMISSION;
               r=1;
          }
          else {
               if (!playingGameFlag) {
                    pic=menuBackgroundPic;
                    r=MEN_doMenu(&curMenu,NUMMENUOPTIONS,pic,
                                 "CORRIDOR 8 GW MENU",menuNames);
               }
               else {
                    r=MEN_doMenu(&curMenu,NUMMENUOPTIONS,0,
                                 "CORRIDOR 8 GW MENU",menu2Names);
               }
          }
          if (r) {
               switch (curMenu) {
               case MMENU_NEWMISSION:
                    if (playingGameFlag) {
                         if (MEN_areYouSure()) {
                              newGameFlag=1;
                              return(0);
                         }
                    }
                    return(1);
               case MMENU_OPTIONS:
                    MEN_gameOptions();
                    break;
               case MMENU_RETRIEVE:
                    if (playingGameFlag) {
                         if (!MEN_areYouSure()) {
                              return(1);
                         }
                    }
                    if (MEN_loadGameOption()) {
                         loadLevelFlag=1;
                         return(1);
                    }
                    break;
               case MMENU_STORE:
                    MEN_saveGameOption();
                    if (playingGameFlag) {
                         GAM_setupVideo(viewSize);
                    }
                    return(1);
               case MMENU_CREDITS:
                    GAM_demoGameCredits();
                    break;
               case MMENU_DEMO:
                    if (playingGameFlag) {
                         return(1);
                    }
                    return(0);
               case MMENU_ORDER:
                    GAM_orderScreen();
                    break;
               case MMENU_QUIT:
                    if (MEN_areYouSure()) {
                         inGameFlag=0;
                         playingGameFlag=0;
                         inMenuFlag=0;
                         return(0);
                    }
                    break;
               }
          }
          else {
               if (playingGameFlag) {
                    return(1);
               }
               else {
                    return(0);
               }
          }
     }
     return(0);
}

int
MEN_getMission(void)               // get episode to play
{
     int  pic,
          r;

     pic=menuBackgroundPic;
     r=MEN_doMenu(&curMission,NUMMISSIONS,pic,"CHOOSE A MISSION",
                  missionNames);
     return(r);
}

int
MEN_getSkill(void)                 // get skill level
{
     int  pic=0,
          r;

     pic=menuBackgroundPic;
     r=MEN_doMenu(&curSkill,NUMSKILLS,pic,"CHOOSE DIFFICULTY",skillNames);
     return(r);
}

int
MEN_getAlliedActor(void)           // get actor to play
{
     int  pic=0,
          r;

     pic=menuBackgroundPic;
     r=MEN_doMenu(&curActor,NUMACTORS,pic,"CHOOSE A CHARACTER",
                  alliedActorNames);
     return(r);
}

int
MEN_getAxisActor(void)             // get actor to play
{
     int  pic=0,
          r;

     pic=menuBackgroundPic;
     r=MEN_doMenu(&curActor,NUMACTORS,pic,"CHOOSE A CHARACTER",
                  axisActorNames);
     return(r);
}

int
MEN_getAlliance(void)              // get actor to play
{
     int  pic=0,
          r;

     pic=menuBackgroundPic;
     r=MEN_doMenu(&curAlliance,NUMALLIANCES,pic,"CHOOSE A SIDE",
                  allianceNames);
     return(r);
}

#if 0
void
MEN_fillSoundCardNames(void)
{
     int  i,j;

     j=firstSoundCard;
     if (j == 0) {
          strcpy(soundCardName[0],"AUTO DETECT");
     }
     else {
          strcpy(soundCardName[0],"PREVIOUS MENU");
     }
     soundCardNames[0]->active=1;
     for (i=1 ; i < 8 ; i++) {
          if (sDIGIDeviceName[j] != NULL) {
               soundCardNames[i]->active=1;
               strcpy(soundCardName[i],sDIGIDeviceName[j]);
               MEN_stripTrailingBlanks(soundCardName[i]);
               j++;
          }
          else {
               soundCardNames[i]->active=0;
          }
     }
     strcpy(soundCardName[8],"MORE OPTIONS");
     soundCardNames[8]->active=1;
     lastSoundCard=j;
}

int
MEN_getSoundPort(void)
{
     int  i,pic,r;

     for (i=0 ; i < 9 ; i++) {
          soundCardNames[i]->active=0;
     }
     i=0;
     while ((short)wDIGIPortList[i] != -1 && i < 9) {
          if (sDIGIDriver.sHardware.wPort == wDIGIPortList[i]) {
               curPort=i;
          }
          sprintf(soundCardName[i],"PORT %03X",wDIGIPortList[i]);
          soundCardNames[i]->active=1;
          i++;
     }
     if (i == 0) {
          return(0);
     }
     if (i == 1) {
          return(1);
     }
     pic=menuBackgroundPic;
     r=MEN_doMenu(&curPort,i,pic,"CHOOSE THE PORT",soundCardNames);
     if (r) {
          sDIGIDriver.sHardware.wPort=wDIGIPortList[curPort];
          return(1);
     }
     return(0);
}

int
MEN_getSoundIRQ(void)
{
     int  i,pic,r;

     for (i=0 ; i < 9 ; i++) {
          soundCardNames[i]->active=0;
     }
     i=0;
     while ((short)wDIGIIRQList[i] != -1 && i < 9) {
          if (sDIGIDriver.sHardware.wIRQ == wDIGIIRQList[i]) {
               curIRQ=i;
          }
          sprintf(soundCardName[i],"IRQ %d",wDIGIIRQList[i]);
          soundCardNames[i]->active=1;
          i++;
     }
     if (i == 0) {
          sDIGIDriver.sHardware.wIRQ=-1;
          return(1);
     }
     if (i == 1) {
          return(1);
     }
     pic=menuBackgroundPic;
     r=MEN_doMenu(&curIRQ,i,pic,"CHOOSE THE SOUND IRQ",soundCardNames);
     if (r) {
          sDIGIDriver.sHardware.wIRQ=wDIGIIRQList[curIRQ];
          return(1);
     }
     return(0);
}

int
MEN_getSoundDMA(void)
{
     int  i,pic,r;

     for (i=0 ; i < 9 ; i++) {
          soundCardNames[i]->active=0;
     }
     i=0;
     while ((short)wDIGIDMAList[i] != -1 && i < 9) {
          if (sDIGIDriver.sHardware.wDMA == wDIGIDMAList[i]) {
               curDMA=i;
          }
          sprintf(soundCardName[i],"DMA %d",wDIGIDMAList[i]);
          soundCardNames[i]->active=1;
          i++;
     }
     if (i == 0) {
          sDIGIDriver.sHardware.wDMA=-1;
          return(1);
     }
     if (i == 1) {
          return(1);
     }
     pic=menuBackgroundPic;
     r=MEN_doMenu(&curDMA,i,pic,"CHOOSE THE DMA CHANNEL",soundCardNames);
     if (r) {
          sDIGIDriver.sHardware.wDMA=wDIGIDMAList[curDMA];
          return(1);
     }
     return(0);
}

int
MEN_getSoundConfig(void)
{
     int  more=1,
          pic=0,
          r,
          sndState;

     if (SND_digiInitialized()) {
          return(1);
     }
     if (!configSoundFlag && sosEZGetConfig("HMISET.CFG")) {
          wDIGIIndex=SND_getDIGIIndex(wDIGIDeviceID);
          wMIDIIndex=SND_getMIDIIndex(wMIDIDeviceID);
          return(1);
     }
     pic=menuBackgroundPic;
     do {
          MEN_fillSoundCardNames();
          r=MEN_doMenu(&curSoundCard,9,pic,"CHOOSE A SOUND CARD",
                       soundCardNames);
          if (r) {
               switch (curSoundCard) {
               case 0:
                    if (firstSoundCard == 0) {
                         setupDIGIPerformDetection();
                         firstSoundCard=wDIGIIndex/7;
                         curSoundCard=((wDIGIIndex+1)&7);
                         autoDetectFlag=wDIGIIndex;
                         break;
                    }
                    firstSoundCard-=7;
                    break;
               case 8:
                    if (sDIGIDeviceName[lastSoundCard] == NULL) {
                         firstSoundCard=0;
                    }
                    else {
                         firstSoundCard+=7;
                    }
                    break;
               default:
                    if (firstSoundCard == 0 && curSoundCard == 1) {
                         r=0;
                         more=0;
                    }
                    wDIGIIndex=firstSoundCard+(curSoundCard-1);
                    if (!SND_getCapsIndex(wDIGIIndex)) {
                         break;
                    }
                    sndState=0;
                    if (wDIGIIndex == autoDetectFlag) {
                         more=0;
                    }
                    while (more && sndState >= 0 && sndState < 3) {
                         switch (sndState) {
                         case 0:
                              if (MEN_getSoundPort()) {
                                   sndState++;
                              }
                              else {
                                   sndState--;
                              }
                              break;
                         case 1:
                              if (MEN_getSoundIRQ()) {
                                   sndState++;
                              }
                              else {
                                   sndState--;
                              }
                              break;
                         case 2:
                              if (MEN_getSoundDMA()) {
                                   more=0;
                              }
                              else {
                                   sndState--;
                              }
                              break;
                         }
                    }
                    break;
               }
          }
          else {
               more=0;
          }
     } while (more);
     if (r) {
          SND_writeConfig();
     }
     return(r);
}
#endif

void
MEN_loadScreenShot(short spot)
{
     long imagepos;
     void *image;
     FILE *fp;

     saveNames[spot]->misc1=0;
     saveNames[spot]->misc2=0;
     sprintf(tempbuf,SAVENAMEFMT,spot);
     if ((fp=fopen(tempbuf,"rb")) == NULL) {
          return;
     }
     if (waloff[FRAMETILE] == 0 || walock[FRAMETILE] != 255) {
          fclose(fp);
          return;
     }
     fseek(fp,-sizeof(long),SEEK_END);
     fread(&imagepos,sizeof(long),1,fp);
     if (imagepos > 0L) {
          fseek(fp,imagepos,SEEK_SET);
          image=(void *)waloff[FRAMETILE];
          GAM_fread(image,sizeof(char),320L*200L,fp);
          saveNames[spot]->misc1=FRAMETILE;
          saveNames[spot]->misc2=24576L;
          tilesizx[FRAMETILE]=200;
          tilesizy[FRAMETILE]=320;
     }
     fclose(fp);
}

void
MEN_saveScreenShot(void)
{
     long imagepos;
     void *image;
     FILE *fp;

     sprintf(tempbuf,SAVENAMEFMT,curSaveSpot);
     if ((fp=fopen(tempbuf,"r+b")) == NULL) {
          return;
     }
     imagepos=ftell(fp);
     fseek(fp,0L,SEEK_END);
     if (waloff[FRAMETILE] != 0 && walock[FRAMETILE] == 255) {
          imagepos=ftell(fp);
          image=(void *)waloff[FRAMETILE];
          GAM_fwrite(image,sizeof(char),320L*200L,fp);
     }
     fwrite(&imagepos,sizeof(long),1,fp);
     fclose(fp);
}

void
MEN_setViewToTile(short spot)
{
     setviewtotile(FRAMETILE,200L,320L);
     saveNames[spot]->misc1=FRAMETILE;
     saveNames[spot]->misc2=24576L;
}

void
MEN_setViewBack(void)
{
     setviewback();
}

void
MEN_getSaveScreenShot(short spot)
{
     MEN_setViewToTile(spot);
     ENG_drawScreen(GAM_getViewSprite(),0L);
     MEN_setViewBack();
}

void
MEN_showWeaponInfo(short weap)
{
     if (weaponPtr[weap]->registered) {
          sprintf(tempbuf,"WEAPON #%d: %s",weap,weaponPtr[weap]->name);
     }
     else {
          sprintf(tempbuf,"WEAPON #%d: (UNREGISTERED)",weap);
     }
     MEN_printext(16L,16L,0,tempbuf,FONT_SMALLTEXT);
}

void
MEN_saveGame(FILE *fp)
{
     GAM_fwrite(&saveGameName[curSaveSpot],sizeof(char),MAXSAVENAMESIZE,fp);
     GAM_fwrite(&curMission,sizeof(int),1,fp);
     GAM_fwrite(&curSkill,sizeof(int),1,fp);
     GAM_fwrite(&curActor,sizeof(int),1,fp);
     GAM_fwrite(&curAlliance,sizeof(int),1,fp);
}

void
MEN_loadGame(FILE *fp)
{
     GAM_fread(&saveGameName[curSaveSpot],sizeof(char),MAXSAVENAMESIZE,fp);
     GAM_fread(&curMission,sizeof(int),1,fp);
     GAM_fread(&curSkill,sizeof(int),1,fp);
     GAM_fread(&curActor,sizeof(int),1,fp);
     GAM_fread(&curAlliance,sizeof(int),1,fp);
}

