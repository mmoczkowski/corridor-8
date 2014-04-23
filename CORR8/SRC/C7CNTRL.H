
#define   _CONTROLS_H

#include "keydefs.h"

#define   _SETUP_STRLEN       27
#define   _SETUP_MAX_DEVICES  32
#define   _SETUP_MIDI_PLIST   8
#define   _SETUP_JUSTIFY      12

// SOS equates
#define   _SOS_DMA_BUFFER     0x1000
#define   _SOS_TIMER_RATE     120
#define   _SOS_SAMPLE_RATE    11025

void writeControlConfigs(void);
void readControlConfigs(int);

//** Configurable actions

enum {
     ACT_FORWARD,
     ACT_BACKWARD,
     ACT_LEFT,
     ACT_RIGHT,
     ACT_RUN,
     ACT_STRAFE,
     ACT_USEWEAP,
     ACT_USEITEM,
     ACT_JUMP,
     ACT_CROUCH,
     ACT_LOOKUP,
     ACT_LOOKDN,
     ACT_CENTER,
     ACT_STRAFEL,
     ACT_STRAFER,
     ACT_MAP,
     ACT_ZOOMIN,
     ACT_ZOOMOUT,
     MAXACTIONS
};

struct controlStruct {
     int  v;
     char *desc;
};

extern
struct controlStruct controlArray[];

extern
char *controlAction[];

enum {
     VID_CHAIN,
     VID_VESA,
     VID_SCREENBUFFER,
     VID_TSENG,
     VID_PARADISE,
     VID_S3CHIPSET,
     MAXVIDMODES
};

//** Define controllers supported in this game here
#define   CON_KEYBOARD   0
#define   CON_MOUSE      1
#define   CON_JOYSTICK   2
#define   CON_AVENGER    3
#define   CON_GAMEPAD    4
#define   CON_WINGMAN    5
#define   CON_VFX1       6
#define   MAXCONTROLLERS 7

extern
char controlConfigFile[];

extern
char *controlAction[];

#ifdef CON_MOUSE
#define   MAXMOUSEBUTTONS     4

extern
char *mouseControlLabel[];

extern
char configMouse[MAXMOUSEBUTTONS];
#endif

#ifdef CON_JOYSTICK
#define   MAXJOYSTICKBUTTONS  5

extern
char *joystickControlLabel[];

extern
char configJoystick[MAXJOYSTICKBUTTONS];
#endif

#ifdef CON_AVENGER
#define   MAXAVENGERBUTTONS   7

extern
char *avengerControlLabel[];

extern
char configAvenger[MAXAVENGERBUTTONS];
#endif

#ifdef CON_GAMEPAD
#define   MAXGAMEPADBUTTONS   5

extern
char *gamepadControlLabel[];

extern
char configGamepad[MAXGAMEPADBUTTONS];
#endif

#ifdef CON_WINGMAN
#define   MAXWINGMANBUTTONS   5

extern
char *wingmanControlLabel[];

extern
char configWingman[MAXWINGMANBUTTONS];
#endif

#ifdef CON_VFX1
#define   MAXVFX1BUTTONS      4

extern
char *VFX1ControlLabel[];

extern
char configVFX1[MAXVFX1BUTTONS];
#endif

extern
char *videoModeList[];

extern
char configKeyboard[MAXACTIONS];

extern
signed char videoModeOption;

