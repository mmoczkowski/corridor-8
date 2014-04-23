/***************************************************************************
 *   CORR7.H   - include file contains Corridor 7 specific defines
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#ifndef _PROTOTYPING_
#include "corr7.def"
#include "corr7act.def"
#include "corr7cfg.def"
#include "corr7eff.def"
#include "corr7eng.def"
#include "corr7gam.def"
#include "corr7gfx.def"
#include "corr7kbd.def"
#include "corr7men.def"
#include "corr7mul.def"
#include "corr7plr.def"
#include "corr7smk.def"
#include "corr7snd.def"
#include "corr7tmr.def"
#include "corr7wep.def"
#include "readcfg.def"
#endif

#include "c7cntrl.h"

#define   GAMEINIFILE         "CORR7.INI"

#define   PLAYERDEBUG
//#define   ANALYZESPRITESDEBUG
//#define   ANALYZEMEMPTRSDEBUG
//#define   ANALYZESOUNDINDEXDEBUG
#define   TRACKDEBUG

#define   GRAYBASE            0
#define   REDBASE             96
#define   BLUEBASE            144
#define   GREENBASE           216

#define   P256COLOR           GRAYBASE+23
#define   PMSGCOLOR           BLUEBASE+16

#define   MOVEFIFOSIZE        256
#define   TICWAITPERFRAME     3
#define   MAXTEMPBUFSIZE      MAXXDIM

#define   MESSAGESIZE         80
#define   MAXMESSAGES         4
#define   MAXDOORS            100
#define   MAXSECTORPOINTS     32
#define   MAXHEATSOURCES      200
#define   MAXFOOTSTEPS        32
#define   MAXSTARTSPOTS       32
#define   MAXDEFINEDACTORS    30
#define   MAXLIGHTEFFECTS     100
#define   MAXSECTOREFFECTS    100

#define   DEFAULTDOORSPEED    10

#define   MAXMOUSESENSE       16

#define   SAVENAMEFMT         "SAVE%04d.MAP"

//** bit flags for player->moves

enum {
     MOVES_CROUCH=1,
     MOVES_JUMP=2,
     MOVES_SHOOT=4,
     MOVES_LOOKUP=8,
     MOVES_LOOKDN=16,
     MOVES_USE=32,
     MOVES_AUTOCENTER=64
};

enum {
     STAT_NONE,
     STAT_HITSCANEXPLODE,
     STAT_PROJECTILEEXPLODE,
     STAT_SHOCKWAVE,
     STAT_EXPLODESPRITE,
     STAT_DIEANIM,
     STAT_BLOOD,
     STAT_PROXIMITY,
     STAT_PATHMARKER,
     STAT_MOVINGSECTORMARKER,
     STAT_FOOTSTEP,
     STAT_AMBIENT,
     STAT_PROJECTILE,
     STAT_PROJDAMAGEBEG=255,
     STAT_PLAYER=256,              // these status sprites can be damaged
     STAT_ALIVE,                   // ...by projectiles
     STAT_PATROL,
     STAT_WANDER,
     STAT_AMBUSH,
     STAT_GUARD,
     STAT_PAIN,
     STAT_CHASE,
     STAT_CHASING,
     STAT_ATTACK,
     STAT_ATTACKING,
     STAT_DODGE,
     STAT_DODGING,
     STAT_PROJDAMAGEEND,
     STAT_LASTSTAT                 // place holder, do not delete
};

enum {
     INV_BLUEKEY,
     INV_YELLOWKEY,
     INV_REDKEY,
     MAXINVENTORYITEMS
};

enum {
     GUN_1,
     GUN_2,
     GUN_3,
     GUN_4,
     GUN_5,
     GUN_6,
     GUN_7,
     GUN_8,
     GUN_9,
     GUN_0,
     MAXWEAPONS
};

enum {
     PROJNORMAL,
     PROJXFLAT,
     PROJYFLAT
};

enum {
     EXPLOSION_NORMAL,
     EXPLOSION_SHOCKWAVE
};

enum {
     GAMETYPE_SINGLE,
     GAMETYPE_MODEM,
     GAMETYPE_NETGAME,
     GAMETYPE_MPATH
};

enum {
     GAMEMODE_SINGLE,
     GAMEMODE_DEATHMATCH
};

enum {
     MOUSE_ENABLE,
     MOUSE_LEFTBUTTON,
     MOUSE_MIDDLEBUTTON,
     MOUSE_RIGHTBUTTON
};

enum {
     SPRC_BLOCKING=1,
     SPRC_TRANSLUCENT=2,
     SPRC_XFLIPPED=4,
     SPRC_YFLIPPED=8,
     SPRC_WALLSPRITE=16,
     SPRC_FLOORSPRITE=32,
     SPRC_ONESIDED=64,
     SPRC_REALCENTERED=128,
     SPRC_BLOCKINGH=256,
     SPRC_REVTRANS=512,
     SPRC_INVISIBLE=32768
};

enum {
     SPRF_SKILLCORPORALFLAG=1,
     SPRF_SKILLLIEUTENANTFLAG=2,
     SPRF_SKILLCAPTAINFLAG=4,
     SPRF_SKILLMAJORFLAG=8,
     SPRF_SKILLPRESIDENTFLAG=16,
     SPRF_DEAFACTORFLAG=32,
     SPRF_MULTIPLAYERONLYFLAG=64
};

enum {
     WALC_BLOCKING=1,
     WALC_BOTTOMSWAP=2,
     WALC_BOTTOMALIGNED=4,
     WALC_XFLIPPED=8,
     WALC_MASKINGWALL=16,
     WALC_1WAYWALL=32,
     WALC_BLOCKINGH=64,
     WALC_TRANSLUSCENT=128
};

enum {
     TICS_ONESECOND=1,
     TICS_ONEHALF=2,
     TICS_ONEFOURTH=4,
     TICS_ONEEIGHTH=8,
     TICS_ONESIXTEENTH=16,
     TICS_ONETHIRTYSECOND=32
};

//** Sprite (thing) lo-tags 1 thru 3006 are Doom compatible things
enum {                             // sprite lo-tags
     SPR_NORMAL,
     SPR_PLAYER1STARTTAG,          // 1=
     SPR_PLAYER2STARTTAG,          // 2=
     SPR_PLAYER3STARTTAG,          // 3=
     SPR_PLAYER4STARTTAG,          // 4=
     SPR_BLUEKEYTAG,               // 5=
     SPR_YELLOWKEYTAG,             // 6=
     SPR_SPIDERBOSSTAG,            // 7=
     SPR_BACKPACKTAG,              // 8=
     SPR_HUMANSERGTAG,             // 9=
     SPR_DEATHMATCHSTARTTAG=11,    // 11=
     SPR_REDKEYTAG=13,             // 13=
     SPR_TELEPORTTAG,              // 14=
     SPR_CYBERDEMONBOSSTAG=16,     // 16=
     SPR_CELLCHARGEPACKTAG,        // 17=
     SPR_REDSKULLKEYTAG=38,        // 38=
     SPR_YELLOWSKULLKEYTAG,        // 39=
     SPR_BLUESKULLKEYTAG,          // 40=
     SPR_SPECTRETAG=58,            // 58=
     SPR_ARCHVILETAG=64,           // 64=
     SPR_CHAINGUNNERSTAG,          // 65=
     SPR_REVENANTTAG,              // 66=
     SPR_MANCUBUSTAG,              // 67=
     SPR_ARACHNOTRONTAG,           // 68=
     SPR_HELLKNIGHTTAG,            // 69=
     SPR_PAINELEMENTALTAG=71,      // 71=
     SPR_DOUBLESHOTGUNTAG=82,      // 82=
     SPR_MEGASPHERETAG,            // 83=
     SPR_WOLFSSTAG,                // 84=
     SPR_BOSSBRAINTAG=88,          // 88=
     SPR_BOSSSHOOTERTAG,           // 89=
     SPR_SHOTGUNTAG=2001,          // 2001=
     SPR_CHAINGUNTAG,              // 2002=
     SPR_ROCKETLAUNCHERTAG,        // 2003=
     SPR_PLASMAGUNTAG,             // 2004=
     SPR_CHAINSAWTAG,              // 2005=
     SPR_BFG9000TAG,               // 2006=
     SPR_AMMOCLIPTAG,              // 2007=
     SPR_SHOTGUNSHELLSTAG,         // 2008=
     SPR_ROCKETTAG=2010,           // 2010=
     SPR_STIMPACKTAG,              // 2011=
     SPR_MEDIKITTAG,               // 2012=
     SPR_SOULSPHERETAG,            // 2013=
     SPR_HEALTHPOTIONTAG,          // 2014=
     SPR_SPIRITARMORTAG,           // 2015=
     SPR_GREENARMORTAG=2018,       // 2018=
     SPR_BLUEARMORTAG,             // 2019=
     SPR_INVULNERABILITYTAG=2022,  // 2022=
     SPR_BERSERKTAG,               // 2023=
     SPR_INVISIBILITYTAG,          // 2024=
     SPR_RADIATIONSUITTAG,         // 2025=
     SPR_COMPUTERMAPTAG,           // 2026=
     SPR_LIGHTAMPLIFICATIONTAG=2045, // 2045=
     SPR_BOXOFROCKETSTAG,          // 2046=
     SPR_CELLCHARGETAG,            // 2047=
     SPR_BOXOFAMMOTAG,             // 2048=
     SPR_BOXOFSHELLSTAG,           // 2049=
     SPR_IMPTAG=3001,              // 3001=
     SPR_DEMONTAG,                 // 3002=
     SPR_BARONTAG,                 // 3003=
     SPR_FORMERHUMANTAG,           // 3004=
     SPR_CACODEMONTAG,             // 3005=
     SPR_LOSTSOULTAG,              // 3006=
     SPR_PLAYER5STARTTAG=4000,     // 4000=player #5 start spot
     SPR_PLAYER6STARTTAG,          // 4001=player #6 start spot
     SPR_PLAYER7STARTTAG,          // 4002=player #7 start spot
     SPR_PLAYER8STARTTAG,          // 4003=player #8 start spot
     SPR_SWITCHTAG,                // 4004=sprite switch
     SPR_PANCEILTAG,               // 4005=pan ceiling texture
     SPR_PANFLOORTAG,              // 4006=pan floor texture
     SPR_MOVINGSECTORTAG,          // 4007=moving sector marker
     SPR_DETAIL1,                  // 4008=low detail sprites (removed last)
     SPR_DETAIL2,                  // 4009=medium detail sprites
     SPR_DETAIL3,                  // 4010=high detail sprites
     SPR_DETAIL4,                  // 4011=extra high detail sprites
     SPR_LASTTAG
};

//** Sector tags 1 thru 17 are for Doom compatibility
enum {                             // sector lo-tags
     SEC_NORMAL,                   // 0=normal sector
     SEC_BLINKOFFTAG,              // 1=blink off randomly
     SEC_BLINKONTAG,               // 2=blink on .5 sec intervals
     SEC_BLINKON1STAG,             // 3=blink on 1 sec intervals
     SEC_HIDAMAGEBLINKTAG,         // 4=hi damage, blink .5 sec intervals
     SEC_MEDDAMAGETAG,             // 5=medium damage
     SEC_LODAMAGETAG=7,            // 7=lo damage sector
     SEC_OSCILLATETAG,             // 8=oscillate lighting
     SEC_SECRETTAG,                // 9=secret sector
     SEC_CLOSE30STAG,              // 10=close after 30 seconds
     SEC_HIDAMAGEENDTAG,           // 11=hi damage until health <=10%, end
     SEC_SYNCBLINKONTAG,           // 12=synchronized blink 1 sec intervals
     SEC_SYNCBLINKOFFTAG,          // 13=synchronized blink off .5 sec int.
     SEC_OPEN5MTAG,                // 14=open sector after 5 minutes
     SEC_HIDAMAGETAG=16,           // 16=hi damage sector
     SEC_FLICKERTAG,               // 17=flicker light randomly
     SEC_DOORUPTAG=200,            // 200=rising door
     SEC_DOORUPONCETAG,            // 201=one-time rising door
     SEC_DOORDOWNTAG,              // 202=drop door
     SEC_DOORDOWNONCETAG,          // 203=one-time drop door
     SEC_PLATFORMDOWNTAG,          // 204=platform lift
     SEC_DOORHSPLITTAG,            // 205=horizontal split door
     SEC_DOORHSPLITONCETAG,        // 206=one-time horizontal split door
     SEC_DOORSLIDECENTERTAG,       // 207=sliding door (opens in center)
     SEC_ROTATESECTORTAG,          // 208=rotating sector
     SEC_BONUSSECTORTAG,           // 209=bonus sector (health, armor, etc)
     SEC_PLATFORMELEVATORTAG,      // 210=platform elevator (ceil & floor)
     SEC_ABOVEWATERTAG,            // 211=
     SEC_BELOWWATERTAG,            // 212=
     SEC_ABOVEWATERLODAMAGETAG,    // 213=
     SEC_BELOWWATERLODAMAGETAG,    // 214=
     SEC_ABOVEWATERHIDAMAGETAG,    // 215=
     SEC_BELOWWATERHIDAMAGETAG,    // 216=
     SEC_MOVINGSECTORTAG,          // 217=moving sector
     SEC_AUTOMOVINGSECTORTAG,      // 218=timer activated moving sector
     SEC_MOVINGSECTORFIXEDTAG,     // 219=moving sector, no rotate
     SEC_AUTOMOVINGSECTORFIXEDTAG, // 220=timer activated no rotate
     SEC_PLATFORMUPTAG,            // 221=platform up
     SEC_PANCEILTAG=400,           // auto-set tag (do not delete)
     SEC_PANFLOORTAG,              // auto-set tag (do not delete)
     SEC_LASTTAG
};

enum {
     SECF_LASTTAG
};

enum {
     SECBIT_LAST
};

enum {
     BON_NOTHING,
     BON_HEALTH,
     BON_BULLETAMMO,
     BON_MISSILEAMMO,
     BON_ENERGYAMMO,
     BON_MINEAMMO,
     BON_BODYARMOR,
     BON_WEAPON,
     MAXBONUSITEMS
};

enum {                             // wall lo-tags
     WAL_NORMAL,
     WAL_MRMDOOROPENCLOSETAG=1,    // 1 thru 141 are for DOOM compatibility
     WAL_W1MDOOROPENSTAYTAG,       // 2=
     WAL_W1MDOORCLOSETAG,          // 3=
     WAL_W1MDOOROPENCLOSETAG,      // 4=
     WAL_W1SFLOORUPLOCEILTAG,      // 5=
     WAL_W1FCRUSHERSTARTRESUMETAG, // 6=
     WAL_S1SSTAIRS8UNITTAG,        // 7=
     WAL_W1SSTAIRS8UNITTAG,        // 8=
     WAL_S1SCHANGEDONUTTAG,        // 9=
     WAL_W1FLIFTDOWNUPTAG,         // 10=
     WAL_S1EXITSTANDARDTAG,        // 11=
     WAL_W1LIGHTSBRIGHTESTADJTAG,  // 12=
     WAL_W1LIGHTSONFULLTAG,        // 13=
     WAL_S1SFLOORUP32NULLTAG,      // 14=??
     WAL_S1SFLOORUP24NULLTAG,      // 15=??
     WAL_W1MDOORCLOSE30OPENTAG,    // 16=
     WAL_W1LIGHTSBLINKON1STAG,     // 17=
     WAL_S1SFLOORUPHIFLOORTAG,     // 18=
     WAL_W1SFLOORDOWNHIFLOORTAG,   // 19=
     WAL_S1SFLOORUPHIFLOORNULLTAG, // 20=??
     WAL_S1FLIFTDOWNUPTAG,         // 21=
     WAL_W1SFLOORUPHIFLOORNULLTAG, // 22=??
     WAL_S1SFLOORDOWNLOFLOORTAG,   // 23=
     WAL_G1SFLOORUPLOCEILTAG,      // 24=
     WAL_W1MCRUSHERSTARTRESUMETAG, // 25=
     WAL_MRMBLUEDOOROPENCLOSETAG,  // 26=
     WAL_MRMYELLOWDOOROPENCLOSETAG,// 27=
     WAL_MRMREDDOOROPENCLOSETAG,   // 28=
     WAL_S1MDOOROPENCLOSETAG,      // 29=
     WAL_W1SFLOORUPSHORTTEXTURETAG,// 30=
     WAL_M1MDOOROPENSTAYTAG,       // 31=
     WAL_M1MBLUEDOOROPENSTAYTAG,   // 32=
     WAL_M1MREDDOOROPENSTAYTAG,    // 33=
     WAL_M1MYELLOWDOOROPENSTAYTAG, // 34=
     WAL_W1LIGHTSOFFTAG,           // 35=
     WAL_W1FFLOORDOWN8HITAG,       // 36=
     WAL_W1SFLOORDOWNLOADJXFERTAG, // 37=??
     WAL_W1SFLOORDOWNLOADJTAG,     // 38=
     WAL_W1TELEPORTTAG,            // 39=
     WAL_W1SCEILUPHICEILTAG,       // 40=
     WAL_S1SCEILDOWNFLOORTAG,      // 41=
     WAL_SRMDOORCLOSETAG,          // 42=
     WAL_SRSCEILDOWNFLOORTAG,      // 43=
     WAL_W1SCEILDOWN8ABOVEFLOORTAG,// 44=
     WAL_SRSFLOORDOWNHIFLOORTAG,   // 45=
     WAL_GRMDOOROPENSTAYTAG,       // 46=
     WAL_G1SCHANGER47TAG,          // 47=??
     WAL_SCROLLTEXTURELEFTTAG,     // 48=
     WAL_S1SCEILDOWN8ABOVEFLOORTAG,// 49=
     WAL_S1MDOORCLOSETAG,          // 50=
     WAL_S1EXITSECRETLEVELTAG,     // 51
     WAL_W1EXITNEXTLEVELTAG,       // 52=
     WAL_W1SLIFTSTARTRESUMETAG,    // 53=
     WAL_S1LIFTPAUSETAG,           // 54=
     WAL_S1SFLOORUP8BELOWCEILTAG,  // 55=
     WAL_W1SFLOORUP8BELOWCEILTAG,  // 56=
     WAL_W1CRUSHERPAUSETAG,        // 57=
     WAL_W1SFLOORUP24TAG,          // 58=
     WAL_W1SCHANGER59TAG,          // 59=??
     WAL_SRSFLOORDOWNLOFLOORTAG,   // 60=
     WAL_SRMDOOROPENSTAYTAG,       // 61=
     WAL_SRFLIFTDOWNUPTAG,         // 62=
     WAL_SRMDOOROPENCLOSETAG,      // 63=
     WAL_SRSFLOORUPLOCEILTAG,      // 64=
     WAL_SRSFLOORUP8BELOWCEILTAG,  // 65=
     WAL_SRSCHANGER66TAG,          // 66=
     WAL_SRSCHANGER67TAG,          // 67=
     WAL_SRSCHANGER68TAG,          // 68=
     WAL_SRSFLOORUPHIFLOORTAG,     // 69=
     WAL_SRFFLOORDOWN8ABOVEADJTAG, // 70=
     WAL_S1FFLOORDOWN8ABOVEADJTAG, // 71=
     WAL_WRSCEILDOWN8ABOVEFLOORTAG,// 72=
     WAL_WRSCRUSHERSTARTRESUMETAG, // 73=
     WAL_WRCRUSHERPAUSETAG,        // 74=
     WAL_WRMDOORCLOSETAG,          // 75=
     WAL_WRMDOORCLOSE30OPENTAG,    // 76=
     WAL_WRFCRUSHERSTARTRESUMETAG, // 77=
     WAL_NOTHING78,                // 78=
     WAL_WRLIGHTSOFFTAG,           // 79=
     WAL_WRLIGHTSBRIGHTESTADJTAG,  // 80=
     WAL_WRLIGHTSONFULLTAG,        // 81=
     WAL_WRSFLOORDOWNLOFLOORTAG,   // 82=
     WAL_WRSFLOORDOWNHIFLOORTAG,   // 83=
     WAL_WRSCHANGER84TAG,          // 84=??
     WAL_NOTHING85,                // 85=
     WAL_WRMDOOROPENSTAYTAG,       // 86=
     WAL_WRSLIFTSTARTRESUMETAG,    // 87=
     WAL_WRFLIFTDOWNUPTAG,         // 88=
     WAL_WRLIFTPAUSETAG,           // 89=
     WAL_WRMDOOROPENCLOSETAG,      // 90=
     WAL_WRSFLOORUPLOCEILTAG,      // 91=
     WAL_WRSFLOORUP24TAG,          // 92=
     WAL_WRSCHANGER93TAG,          // 93=??
     WAL_WRSFLOORUP8BELOWCEILTAG,  // 94=
     WAL_WRSCHANGER95TAG,          // 95=
     WAL_WRSFLOORUPSHORTTEXTURETAG,// 96=
     WAL_WRTELEPORTTAG,            // 97=
     WAL_WRFFLOORDOWN8ABOVEADJTAG, // 98=
     WAL_SRTBLUEDOOROPENSTAYTAG,   // 99=
     WAL_W1TSTAIRS16TAG,           // 100=
     WAL_S1SFLOORUPLOCEILTAG,      // 101=
     WAL_S1SFLOORDOWNHIFLOORTAG,   // 102=
     WAL_S1MDOOROPENSTAYTAG,       // 103=
     WAL_W1LIGHTSDIMMESTADJTAG,    // 104=
     WAL_WRTDOOROPENCLOSETAG,      // 105=
     WAL_WRTDOOROPENSTAYTAG,       // 106=
     WAL_WRTDOORCLOSETAG,          // 107=
     WAL_W1TDOOROPENCLOSETAG,      // 108=
     WAL_W1TDOOROPENSTAYTAG,       // 109=
     WAL_W1TDOORCLOSETAG,          // 110=
     WAL_S1TDOOROPENCLOSETAG,      // 111=
     WAL_S1TDOOROPENSTAYTAG,       // 112=
     WAL_S1TDOORCLOSETAG,          // 113=
     WAL_SRTDOOROPENCLOSETAG,      // 114=
     WAL_SRTDOOROPENSTAYTAG,       // 115=
     WAL_SRTDOORCLOSETAG,          // 116=
     WAL_MRTDOOROPENCLOSETAG,      // 117=
     WAL_M1TDOOROPENSTAYTAG,       // 118=
     WAL_W1SFLOORUPHIFLOORTAG,     // 119=
     WAL_WRTLIFTDOWNUPTAG,         // 120=
     WAL_W1TLIFTDOWNUPTAG,         // 121=
     WAL_S1TLIFTDOWNUPTAG,         // 122=
     WAL_SRTLIFTDOWNUPTAG,         // 123=
     WAL_W1EXITSECRETLEVELTAG,     // 124=
     WAL_1TELEPORTMONSTERTAG,      // 125=
     WAL_RTELEPORTMONSTERTAG,      // 126=
     WAL_S1TSTAIRS16TAG,           // 127=
     WAL_WRSFLOORUPHIFLOORTAG,     // 128=
     WAL_WRTFLOORUPHIFLOORTAG,     // 129=
     WAL_W1TFLOORUPHIFLOORTAG,     // 130=
     WAL_S1TFLOORUPHIFLOORTAG,     // 131=
     WAL_SRTFLOORUPHIFLOORTAG,     // 132=
     WAL_S1TBLUEDOOROPENSTAYTAG,   // 133=
     WAL_SRTREDDOOROPENSTAYTAG,    // 134=
     WAL_S1TREDDOOROPENSTAYTAG,    // 135=
     WAL_SRTYELLOWDOOROPENSTAYTAG, // 136=
     WAL_S1TYELLOWDOOROPENSTAYTAG, // 137=
     WAL_SRLIGHTSONFULLTAG,        // 138=
     WAL_SRLIGHTSOFFTAG,           // 139=
     WAL_S1MFLOORUP512TAG,         // 140=
     WAL_W1FCRUSHERSTARTSILENTTAG, // 141=
     WAL_DOORSEARCHTAG=200,        // 200=search tag for sliding doors
     WAL_TRIGGERTAG,               // 201=sector trigger
     WAL_WARPTAG,                  // 202=warp sector
     WAL_SWITCHTAG,                // 203=wall switch
     WAL_LASTTAG
};

enum {
     SWITCH_OFF,
     SWITCH_ON,
     SWITCH_NEXTLEVEL=9999
};

enum {
     ACT_PATROL,
     ACT_WANDER,
     ACT_AMBUSH,
     ACT_GUARD,
     ACT_ATTACK,
     ACT_LAST
};

enum {
     GORELEVEL_1,
     GORELEVEL_2,
     GORELEVEL_3,
     GORELEVEL_4,
     MAXGORECHOICES
};

enum {
     DETAILLEVEL_1,
     DETAILLEVEL_2,
     DETAILLEVEL_3,
     DETAILLEVEL_4,
     MAXDETAILLEVEL
};

enum {
     PLAYSOUND_NONE,
     PLAYSOUND_OPEN,
     PLAYSOUND_CLOSE,
     PLAYSOUND_STOP,
     PLAYSOUND_MOVING,
     PLAYSOUND_ENTER,
     PLAYSOUND_LEAVE,
     PLAYSOUND_LAST
};

struct player {
     int  viewMode;
     short spriteNum;
     unsigned long moves;
     long zoom;                    // 2d overhead map zoom factor
     short gridSize;               // 2d grid size
     short autocenter;             // auto-center the view
     short viewSize;               // 3d view size
     short footSteps;
     short footStepSprite[MAXFOOTSTEPS];
     long weapons;                 // bit array of weapon inventory
     long score;
};

enum {
     K_ESC=1,
     K_1=2, K_2, K_3, K_4, K_5, K_6, K_7, K_8, K_9, K_0,
     K_MINUS=12, K_EQUAL, K_BACKSPACE, K_TAB,
     K_Q=16, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, K_P,
     K_LEFTBRACKET=26, K_RIGHTBRACKET, K_ENTER, K_CONTROL,
     K_A=30, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L,
     K_SEMICOLON=39, K_APOSTROPHE, K_TILDE, K_LEFTSHIFT, K_BACKSLASH,
     K_Z=44, K_X, K_C, K_V, K_B, K_N, K_M,
     K_COMMA=51, K_PERIOD, K_SLASH, K_RIGHTSHIFT,
     K_ASTERISK=55, K_LEFTALT=56, K_SPACE,
     K_F1=59, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10,
     K_NUMLOCK=69,
     K_HOME=71, K_UP, K_PGUP,
     K_GRAYMINUS=74,
     K_LEFT=75, K_GRAY5, K_RIGHT,
     K_GRAYPLUS=78,
     K_END=79, K_DOWN, K_PGDN,
     K_INS=82, K_DEL,
     K_F11=87, K_F12
};

extern
char scantoasc[128];

extern
char scantoascwithshift[128];

//**
//** Prototypes for BUILD functions
//**

#define   KEYFIFOSIZ     64

void      allocache(long *,long,char *);
int       cansee(long,long,long,short,long,long,long,short);
void      changespritesect(short,short);
void      changespritestat(short,short);
void      clear2dscreen(char);
void      clearmidstatbar16(void);
void      clearview(long);
int       clipmove(long *,long *,long *,short *,long,long,long,long,long,char);
void      deletesprite(short);
long      divscale(long,long,long);
long      divscale1(long,long);
long      divscale2(long,long);
long      divscale3(long,long);
long      divscale4(long,long);
long      divscale5(long,long);
long      divscale6(long,long);
long      divscale7(long,long);
long      divscale8(long,long);
long      divscale9(long,long);
long      divscale10(long,long);
long      divscale11(long,long);
long      divscale12(long,long);
long      divscale13(long,long);
long      divscale14(long,long);
long      divscale15(long,long);
long      divscale16(long,long);
long      divscale17(long,long);
long      divscale18(long,long);
long      divscale19(long,long);
long      divscale20(long,long);
long      divscale21(long,long);
long      divscale22(long,long);
long      divscale23(long,long);
long      divscale24(long,long);
long      divscale25(long,long);
long      divscale26(long,long);
long      divscale27(long,long);
long      divscale28(long,long);
long      divscale29(long,long);
long      divscale30(long,long);
long      divscale31(long,long);
long      divscale32(long,long);
long      dmulscale(long,long,long,long,long);
long      dmulscale1(long,long,long,long);
long      dmulscale2(long,long,long,long);
long      dmulscale3(long,long,long,long);
long      dmulscale4(long,long,long,long);
long      dmulscale5(long,long,long,long);
long      dmulscale6(long,long,long,long);
long      dmulscale7(long,long,long,long);
long      dmulscale8(long,long,long,long);
long      dmulscale9(long,long,long,long);
long      dmulscale10(long,long,long,long);
long      dmulscale11(long,long,long,long);
long      dmulscale12(long,long,long,long);
long      dmulscale13(long,long,long,long);
long      dmulscale14(long,long,long,long);
long      dmulscale15(long,long,long,long);
long      dmulscale16(long,long,long,long);
long      dmulscale17(long,long,long,long);
long      dmulscale18(long,long,long,long);
long      dmulscale19(long,long,long,long);
long      dmulscale20(long,long,long,long);
long      dmulscale21(long,long,long,long);
long      dmulscale22(long,long,long,long);
long      dmulscale23(long,long,long,long);
long      dmulscale24(long,long,long,long);
long      dmulscale25(long,long,long,long);
long      dmulscale26(long,long,long,long);
long      dmulscale27(long,long,long,long);
long      dmulscale28(long,long,long,long);
long      dmulscale29(long,long,long,long);
long      dmulscale30(long,long,long,long);
long      dmulscale31(long,long,long,long);
long      dmulscale32(long,long,long,long);
void      dragpoint(short,long,long);
void      draw2dscreen(long,long,short,long,short);
void      drawline16(long,long,long,long,char);
void      drawline256(long,long,long,long,char);
void      drawmapview(long,long,long,short);
void      drawmasks(void);
void      drawrooms(long, long, long, short, long, short);
void      editinput(void);
short     getangle(long,long);
long      getceilzofslope(short, long, long);
long      getflorzofslope(short, long, long);
void      getmousevalues(short *,short *,short *);
short     getnumber16(char *,short,long);
int       getoutputcirclesize(void);
short     getpacket(short *,char *);
long      gettile(long);
void      getzrange(long,long,long,short,long *,long *,long *,long *,long,char);
void      getzsofslope(short, long, long, long *, long *);
void      hitscan(long,long,long,short,long,long,long,short *,short *,short *,
                  long *,long *,long *);
void      initcrc(void);
void      initengine(char, long, long);
void      initgroupfile(char *groupfilename);
void      initkeys(void);
void      initmouse(void);
void      initmultiplayers(char,char,char);
short     insertsprite(short, short);
int       inside(long,long,short);
void      kclose(long);
void      keytimerstuff(void);
long      kfilelength(long);
int       kinp(int);
long      klabs(long);
long      klseek(long,long,long);
long      kmax(long,long);
long      kmin(long,long);
int       kopen4load(char *,char);
void      koutp(int, int);
unsigned  krand(void);
long      kread(long,void *,long);
long      ksqrt(unsigned long);
int       loadboard(char *,long *,long *,long *,short *,short *);
int       loadpics(char *);
void      loadtile(short);
void      makepalookup(long,char *,signed char,signed char,signed char,char);
long      moddiv(long,long);
long      mulscale(long,long,long);
long      mulscale1(long,long);
long      mulscale2(long,long);
long      mulscale3(long,long);
long      mulscale4(long,long);
long      mulscale5(long,long);
long      mulscale6(long,long);
long      mulscale7(long,long);
long      mulscale8(long,long);
long      mulscale9(long,long);
long      mulscale10(long,long);
long      mulscale11(long,long);
long      mulscale12(long,long);
long      mulscale13(long,long);
long      mulscale14(long,long);
long      mulscale15(long,long);
long      mulscale16(long,long);
long      mulscale17(long,long);
long      mulscale18(long,long);
long      mulscale19(long,long);
long      mulscale20(long,long);
long      mulscale21(long,long);
long      mulscale22(long,long);
long      mulscale23(long,long);
long      mulscale24(long,long);
long      mulscale25(long,long);
long      mulscale26(long,long);
long      mulscale27(long,long);
long      mulscale28(long,long);
long      mulscale29(long,long);
long      mulscale30(long,long);
long      mulscale31(long,long);
long      mulscale32(long,long);
void      neartag(long,long,long,short,short,short *,short *,short *,long *,
                  long,char);
void      nextpage(void);
short     nextsectorneighborz(short,long,short,short);
void      printext256(long,long,int,int,char *,int);
void      printext16(long,long,short,short,char *,char);
void      printmessage16(char *);
int       pushmove(long *,long *,long *,short *,long,long,long,char);
void      qsetmode640480(void);
void      rotatepoint(long,long,long,long,short,long *,long *);
void      rotatesprite(long,long,long,short,short,signed char,char,char,long,
                       long,long,long);
int       saveboard(char *,long *,long *,long *,short *,short *);
long      scale(long,long,long);
void      screencapture(char *,char);
short     sectorofwall(short);
void      sendlogon(void);
void      sendlogoff(void);
void      sendpacket(short,char *,short);
void      setbrightness(int,char *);
void      setgamemode(void);
void      setsprite(short,long,long,long);
void      setview(long, long, long, long);
void      setviewback(void);
void      setviewtotile(short, long, long);
void      setvmode(int);
void      showengineinfo(void);
long      tmulscale1(long, long, long, long, long, long);
long      tmulscale2(long, long, long, long, long, long);
long      tmulscale3(long, long, long, long, long, long);
long      tmulscale4(long, long, long, long, long, long);
long      tmulscale5(long, long, long, long, long, long);
long      tmulscale6(long, long, long, long, long, long);
long      tmulscale7(long, long, long, long, long, long);
long      tmulscale8(long, long, long, long, long, long);
long      tmulscale9(long, long, long, long, long, long);
long      tmulscale10(long, long, long, long, long, long);
long      tmulscale11(long, long, long, long, long, long);
long      tmulscale12(long, long, long, long, long, long);
long      tmulscale13(long, long, long, long, long, long);
long      tmulscale14(long, long, long, long, long, long);
long      tmulscale15(long, long, long, long, long, long);
long      tmulscale16(long, long, long, long, long, long);
long      tmulscale17(long, long, long, long, long, long);
long      tmulscale18(long, long, long, long, long, long);
long      tmulscale19(long, long, long, long, long, long);
long      tmulscale20(long, long, long, long, long, long);
long      tmulscale21(long, long, long, long, long, long);
long      tmulscale22(long, long, long, long, long, long);
long      tmulscale23(long, long, long, long, long, long);
long      tmulscale24(long, long, long, long, long, long);
long      tmulscale25(long, long, long, long, long, long);
long      tmulscale26(long, long, long, long, long, long);
long      tmulscale27(long, long, long, long, long, long);
long      tmulscale28(long, long, long, long, long, long);
long      tmulscale29(long, long, long, long, long, long);
long      tmulscale30(long, long, long, long, long, long);
long      tmulscale31(long, long, long, long, long, long);
long      tmulscale32(long, long, long, long, long, long);
void      uninitengine(void);
void      uninitgroupfile(void);
void      uninitkeys(void);
void      uninitmultiplayers(void);
void      updatesector(long, long, short *);

extern
long frameplace,
     qsetmode;

extern
short connecthead,
     connectpoint2[MAXPLAYERS],
     myconnectindex,
     numplayers;

extern volatile
char keyfifo[KEYFIFOSIZ],
     keyfifoplc,
     keyfifoend;

//**
//** Defines in CORR7.C
//**

extern
int  videoMode;

extern
long videoResX,
     videoResY;

extern
long fakeClock;

extern
char tempbuf[MAXTEMPBUFSIZE];

extern
int  debugline;

extern
int  noWaitFlag;

//**
//** Defines in CORR7ACT.C
//**

extern
int  noEnemiesFlag;

extern
short spriteHoriz[MAXSPRITES];

extern
char currentWeapon[MAXSPRITES],
     fireFrame[MAXSPRITES];

extern
char attackers[MAXSPRITES],
     firingWeapon[MAXSPRITES];

extern
short nextSprite[MAXSPRITES];

#if 0
extern
short actorPic[MAXSPRITES],
     attackDelay[MAXSPRITES],
     health[MAXSPRITES],
     killer[MAXSPRITES],
     nextSprite[MAXSPRITES],
     waitDoor[MAXSPRITES];
#endif

extern
short enemyPic[MAXDEFINEDACTORS],
     enemyPicAngles[MAXDEFINEDACTORS],
     enemyFirePic1[MAXDEFINEDACTORS],
     enemyFirePic2[MAXDEFINEDACTORS],
     enemyFireAngles[MAXDEFINEDACTORS],
     enemyPainPic[MAXDEFINEDACTORS],
     enemyPainAngles[MAXDEFINEDACTORS],
     enemyDiePic1[MAXDEFINEDACTORS],
     enemyDiePic2[MAXDEFINEDACTORS],
     enemyGorePic1[MAXDEFINEDACTORS],
     enemyGorePic2[MAXDEFINEDACTORS],
     enemyDieAngles[MAXDEFINEDACTORS];

//**
//** Defines in CORR7CFG.C
//**

//**
//** Defines in CORR7EFF.C
//**

extern
long mymapchksum;

extern
short frames[MAXSPRITES],
     frameDelay[MAXSPRITES],
     frameDelayReset[MAXSPRITES];

#if 0
extern
short doorIndex[MAXSECTORS];
#endif

enum {
     EXTFLAGS_SKILL1=1,
     EXTFLAGS_SKILL2=2,
     EXTFLAGS_SKILL3=4,
     EXTFLAGS_SKILL4=8,
     EXTFLAGS_SKILL5=16,
     EXTFLAGS_DEAFACTOR=32,
     EXTFLAGS_MULTIPLAYERONLY=64,
     EXTFLAGS_GRAVITY=128,
     EXTFLAGS_HEATSOURCE=256,
     EXTFLAGS_LASTFLAG
};

struct sectorCenters {
     long centerx;
     long centery;
     long centerz;
};

struct sectorEffect {
     short effectTag;
     char status;
     long clock;
     short ang;
     short speed;
};

extern
struct sectorEffect sectorEffectStruct[MAXSECTOREFFECTS];

extern
struct sectorCenters sectorCenter[MAXSECTORS],
     *sectorCenterPtr[MAXSECTORS];

struct message {
     char buf[MESSAGESIZE];
     short delay;
};

typedef enum {
     DOORSTATE_IDLE,
     DOORSTATE_OPEN,
     DOORSTATE_CLOSE,
     DOORSTATE_OPENED,
     DOORSTATE_CLOSED,
     DOORSTATE_WAITING
} doorstate;

typedef enum {
     DOORKEY_RED,
     DOORKEY_BLUE,
     DOORKEY_GREEN,
     DOORKEY_YELLOW
} doorkey;

struct doorData {             // for special sectors
     doorstate state;
     doorkey key;
     char movingSectorFlag;
     short sectorIndex;
     short movingDoorIndex;
     long hiz;
     long loz;
     long height;
     short speed;
     long delay;
     short wallIndex[MAXSECTORPOINTS];
     long wallx[MAXSECTORPOINTS];
     long wally[MAXSECTORPOINTS];
};

extern
struct doorData door[MAXDOORS],
     *doorPtr[MAXDOORS];

extern
struct message message[MAXMESSAGES],
     *messagePtr[MAXMESSAGES];

//**
//** Defines in CORR7ENG.C
//**

extern
short horizSlopeAdj[MAXSPRITES];

extern
long diez[MAXPLAYERS];

extern
int  viewSize;

extern
int  configVideoFlag,
     debugModeFlag,
     weaponEditFlag;

extern
long globloz,
     globlohit,
     globhiz,
     globhihit;

extern
long avgFPS;

extern
walltype  *wallPtr[MAXWALLS];

extern
sectortype *sectorPtr[MAXSECTORS];

extern
spritetype *spritePtr[MAXSPRITES];

extern
int  editorEnabledFlag,
     gameModeFlag;

//**
//** Defines in CORR7GAM.C
//**

extern
char mapFileName[_MAX_PATH];

extern
int  cheatGodMode,
     cheatUnlimitedAmmo;

extern
int  bypassLevelFlag,
     dumpDebugInfoFlag,
     gameInitializedFlag,
     inGameFlag,
     loadLevelFlag,
     newGameFlag,
     newMapLoadedFlag,
     playingGameFlag;

extern
short graphicLetAPic,
     graphicNum0Pic,
     graphicBigNum0Pic,
     menuMarkerBegPic,
     menuMarkerEndPic,
     menuBackgroundPic,
     menuTitleBarPic,
     menuSliderBarPic,
     menuSliderKnobPic,
     titleScreenPic,
     backgroundPic,
     statusBarPic,
     goreChunksBegPic,
     goreChunksEndPic;

extern
short starta,
     starts;

extern
long startx,
     starty,
     startz;

extern
short forceAng[MAXSPRITES],
     forceVel[MAXSPRITES],
     horizAdj[MAXSPRITES];

extern
long horizVel[MAXSPRITES],
     jumpVel[MAXSPRITES];

#if 0
extern
char onGround[MAXSPRITES>>3];
#endif

extern
long damageHeight,
     gravityConstant,
     jumpVelocity,
     defaultPlayerWidth,
     defaultPlayerHeight;

extern
long loadpos;

extern
FILE *dbgfp;

//**
//** Defines in CORR7KBD.C
//**

extern volatile
char keystatus[256];

extern
char tkeys[256];

extern
short globalMouseB,
     globalMouseX,
     globalMouseY;

//**
//** Defines in CORR7MEN.C
//**

enum {
     SKILL_CORPORAL,
     SKILL_LIEUTENANT,
     SKILL_CAPTAIN,
     SKILL_MAJOR,
     SKILL_PRESIDENT,
     NUMSKILLS
};

extern
int  configSoundFlag,
     inMenuFlag;

extern
int  curActor,
     curAlliance,
     curMission,
     curSkill;

//**
//** Defines in CORR7MUL.C
//**

struct moveFIFO {
     short avel,fvel,svel;
     unsigned long moves;
     char weapon;
};

struct startSpot {
     long x,y,z;
     short ang,sectnum;
};

extern
int  gameMode,
     gameType,
     outofsync,
     waitplayers;

extern
int  moveFIFObeg,
     moveFIFOend;

extern
short numStartSpots;

extern
char syncseed[MOVEFIFOSIZE];

extern
long lockseed;

extern
struct moveFIFO moveFIFOBuf[MAXPLAYERS][MOVEFIFOSIZE];

extern
struct startSpot startSpot[MAXSTARTSPOTS],
     startSpotDeathMatch[MAXSTARTSPOTS];

//**
//** Defines in CORR7PLR.C
//**

extern
char keys[MAXKEYS];

extern
int  currentView,
     localViewSize;

extern
int  doMainMenuFlag,
     horizCheckFlag,
     singleStepFlag;

extern
int  playerHealth,
     playerMaxHealth,
     playerScale;

extern
long locavel,
     locfvel,
     locsvel;

extern
struct player plr[MAXPLAYERS],
     *player[MAXPLAYERS];

//**
//** Defines in CORR7SND.C
//**

extern
int  noSoundFlag;

extern
_SOS_HARDWARE sDIGISettings;

extern
_SOS_MIDI_HARDWARE sMIDISettings;

//**
//** Defines in CORR7TMR.C
//**

extern volatile
long ticsPerFrame;

//**
//** Defines in CORR7WEP.C
//**

enum {
     WEAPON_BULLET=1,
     WEAPON_SHELL,
     WEAPON_MISSILE,
     WEAPON_MINE,
     WEAPON_ENERGY,
     MAXWEAPONTYPES
};

enum {
     GUIDANCE_DUMB,
     GUIDANCE_HEATSEEKER,
     GUIDANCE_CRUISE,
     MAXGUIDANCETYPES
};

enum {
     DISPERSE_RANDOM,
     DISPERSE_FIXED
};

extern
short ammo[MAXSPRITES][MAXWEAPONTYPES];

struct weaponData {
     char registered;              // 0 if not registered or >=1 weapon type
     char *name;                   // name of weapon
     char guidance;                // type of guiding system
     short trackerFreq;            // tracking frequency
     short detonation;             // detonation type
     short detrange;               // detonation range if detonation=proximity
     short ticDelay;               // delay between firing rounds
     short animDelay;              // delay between animating frames
     short defaultAmmo;            // default ammo for this weapon
     char hitscanFlag;             // 1=hitscan, 0=projectile
     short projectiles;            // number of projectiles to shoot
     char disperseType;            // type of dispersement (FIXED, RANDOM)
     short dispersement;           // scatter field of projectiles
     short projectilePic;          // pic of projectile
     short projectileXSize;        // size of projectile (xrepeat)
     short projectileYSize;        // size of projectile (yrepeat)
     short projectileSpeed;        // speed of projectile
     short projectileTurnRate;     // turning speed of this projectile
     short projectileViews;        // views this object has
     char projectileFace;          // face of projectile (XFLAT/YFLAT/NORMAL)
     long projectileFOV;           // field of view for locking on to targets
     char gravityFlag;             // affected by gravity
     char windFlag;                // affected by winds
     short minDamage;              // minimum damage inflicted
     short maxDamage;              // maximum damage inflicted
     char autoCenterFlag;          // 0=editart centering, 1=auto centering
     short holdWeaponStartPic;     // first frame of weapon hold sequence
     short holdWeaponStopPic;      // last frame of weapon hold sequence
     short fireWeaponStartPic;     // first frame of weapon when fired
     short fireWeaponStopPic;      // last frame of weapon when fired
     short fireFrames;
     short weaponShootFrame;       // call shootweapon() on this frame
     short range;                  // range of explosion for damage
     char explosionType;           // type of explosion (NORMAL, SHOCKWAVE)
     short explosionStartPic;      // first explosion frame when object hit
     short explosionStopPic;       // last explosion frame when object hit
     short explosionSize;          // xrepeat, yrepeat of explosion pic
     short explosionRate;          // rate to animate explosion
     char *sample;                 // sound sample to play when fired
};

extern
struct weaponData weaponParms[MAXWEAPONS],
     *weaponPtr[MAXWEAPONS];

