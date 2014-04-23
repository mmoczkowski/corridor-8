/***************************************************************************
 *   READCFG.C - Routines to read GAMEINIFILE file for Corridor 7
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

char controlConfigFile[_MAX_PATH];

static
char tempBuf[256];

struct controlStruct controlArray[MAXACTIONS+1]={
     {KEYFWD,       "MOVE FORWARD "},
     {KEYBACK,      "MOVE BACKWARD"},
     {KEYLEFT,      "TURN LEFT    "},
     {KEYRIGHT,     "TURN RIGHT   "},
     {KEYRUN,       "RUN MODE     "},
     {KEYSTRAFE,    "STRAFE MODE  "},
     {KEYFIRE,      "USE WEAPON   "},
     {KEYUSE,       "OPEN/CLOSE   "},
     {KEYJUMP,      "JUMP         "},
     {KEYCROUCH,    "DUCK         "},
     {KEYLKUP,      "LOOK UP      "},
     {KEYLKDN,      "LOOK DOWN    "},
     {KEYCNTR,      "LOOK CENTER  "},
     {KEYSTFL,      "STRAFE LEFT  "},
     {KEYSTFR,      "STRAFE RIGHT "},
     {KEYMAP,       "MAP MODE     "},
     {KEYZOOMI,     "MAP ZOOM IN  "},
     {KEYZOOMO,     "MAP ZOOM OUT "},
     {0,            NULL           }
};

char *controlAction[MAXACTIONS+1];

#ifdef CON_MOUSE
char *mouseControlLabel[MAXMOUSEBUTTONS]={
     "MOUSE ENABLE ",
     "LEFT BUTTON  ",
     "MIDDLE BUTTON",
     "RIGHT BUTTON "
};
#endif

#ifdef CON_JOYSTICK
char *joystickControlLabel[MAXJOYSTICKBUTTONS]={
     "JOYSTICK ENABLE",
     "BUTTON 1       ",
     "BUTTON 2       ",
     "BUTTON 3       ",
     "BUTTON 4       "
};
#endif

#ifdef CON_AVENGER
char *avengerControlLabel[MAXAVENGERBUTTONS]={
     "AVENGER ENABLE",
     "BUTTON A      ",
     "BUTTON B      ",
     "BUTTON C      ",
     "BUTTON D      ",
     "BUTTON E      ",
     "BUTTON F      "
};
#endif

#ifdef CON_GAMEPAD
char *gamepadControlLabel[MAXGAMEPADBUTTONS]={
     "GAMEPAD ENABLE",
     "BUTTON 1      ",
     "BUTTON 2      ",
     "BUTTON 3      ",
     "BUTTON 4      "
};
#endif

#ifdef CON_WINGMAN
char *wingmanControlLabel[MAXWINGMANBUTTONS]={
     "WINGMAN ENABLE",
     "TOP BUTTON    ",
     "THUMB BUTTON  ",
     "TRIGGER BUTTON",
     "MIDDLE BUTTON "
};
#endif

#ifdef CON_VFX1
char *VFX1ControlLabel[MAXVFX1BUTTONS]={
     "CYBERPUCK ENABLE",
     "TOP BUTTON      ",
     "MIDDLE BUTTON   ",
     "BOTTOM BUTTON   "
};
#endif

//** Supported video modes
extern
char *videoModeList[];

char configKeyboard[MAXACTIONS];

#ifdef CON_MOUSE
char configMouse[MAXMOUSEBUTTONS];
#endif
#ifdef CON_JOYSTICK
char configJoystick[MAXJOYSTICKBUTTONS];
#endif
#ifdef CON_AVENGER
char configAvenger[MAXAVENGERBUTTONS];
#endif
#ifdef CON_GAMEPAD
char configGamepad[MAXGAMEPADBUTTONS];
#endif
#ifdef CON_WINGMAN
char configWingman[MAXWINGMANBUTTONS];
#endif
#ifdef CON_VFX1
char configVFX1[MAXVFX1BUTTONS];
#endif

W32  wDIGIDevice=-1,
     wMIDIDevice=-1;

_SOS_HARDWARE sDIGISettings;
_SOS_MIDI_HARDWARE sMIDISettings;

extern
char configFile[];

BOOL
readHMICFGFile(PSTR szName,PSTR szDIGIName,PSTR szMIDIName)
{
     _INI_INSTANCE sInstance;
     W32  wError;

     if (hmiINIOpen(&sInstance,szName) == _FALSE) {
          return(_FALSE);
     }
     if (!hmiINILocateSection(&sInstance,"DIGITAL")) {
          hmiINIClose(&sInstance);
          return(_FALSE);
     }
     wError=hmiINIGetItemDecimal(&sInstance,"DeviceID",&wDIGIDevice);
     wError+=hmiINIGetItemDecimal(&sInstance,"DevicePort",&sDIGISettings.wPort);
     wError+=hmiINIGetItemDecimal(&sInstance,"DeviceDMA",&sDIGISettings.wDMA);
     wError+=hmiINIGetItemDecimal(&sInstance,"DeviceIRQ",&sDIGISettings.wIRQ);
     if (szDIGIName != NULL) {
          wError+=hmiINIGetItemString(&sInstance,"DeviceName",&szDIGIName[0],
                                      128);
     }
     else {
          wError+=1;
     }
     if (wError != 5) {
          wDIGIDevice=-1;
          sDIGISettings.wPort=-1;
          sDIGISettings.wDMA=-1;
          sDIGISettings.wIRQ=-1;
     }
     if (!hmiINILocateSection(&sInstance,"MIDI")) {
          hmiINIClose(&sInstance);
          return(_FALSE);
     }
     wError=hmiINIGetItemDecimal(&sInstance,"DeviceID",&wMIDIDevice);
     wError+=hmiINIGetItemDecimal(&sInstance,"DevicePort",&sMIDISettings.wPort);
     if (szMIDIName != NULL) {
          wError+=hmiINIGetItemString(&sInstance,"DeviceName",&szMIDIName[0],
                                      128);
     }
     else {
          wError+=1;
     }
     if (wError != 3) {
          wMIDIDevice=-1;
          sMIDISettings.wPort=-1;
          sMIDISettings.wIRQ=-1;
     }
     hmiINIClose(&sInstance);
     return(_TRUE);
}

void
readKeyboardConfig(_INI_INSTANCE *sInstance)
{
     int  i,j;

     for (i=0 ; i < MAXACTIONS ; i++) {
          configKeyboard[i]=255;
     }
     if (!hmiINILocateSection(sInstance,"KEYBOARD")) {
          return;
     }
     for (i=0 ; i < MAXACTIONS ; i++) {
          if (hmiINIGetItemString(sInstance,controlAction[i],tempBuf,16)) {
               if (strlen(tempBuf) > 0) {
                    sscanf(tempBuf,"%x",&j);
                    configKeyboard[i]=(char)j;
               }
          }
     }
}

void
writeKeyboardConfig(_INI_INSTANCE *sInstance)
{
     int  i;

     if (!hmiINILocateSection(sInstance,"KEYBOARD")) {
          hmiINIAddSection(sInstance,"KEYBOARD");
     }
     for (i=0 ; i < MAXACTIONS ; i++) {
          if (!hmiINILocateItem(sInstance,controlAction[i])) {
               if (configKeyboard[i] != 255) {
                    hmiINIAddItemDecimal(sInstance,controlAction[i],
                                         (WORD)configKeyboard[i],
                                         strlen(controlAction[i])+4,16);
               }
          }
          else {
               hmiINIWriteDecimal(sInstance,(WORD)configKeyboard[i]);
          }
     }
}

#ifdef CON_MOUSE
void
readMouseConfig(_INI_INSTANCE *sInstance)
{
     int  i,j;

     for (i=0 ; i < MAXMOUSEBUTTONS ; i++) {
          configMouse[i]=255;
     }
     if (!hmiINILocateSection(sInstance,"MOUSE")) {
          return;
     }
     for (i=0 ; i < MAXMOUSEBUTTONS ; i++) {
          if (hmiINIGetItemString(sInstance,mouseControlLabel[i],tempBuf,16)) {
               if (strlen(tempBuf) > 0) {
                    sscanf(tempBuf,"%x",&j);
                    configMouse[i]=j;
               }
          }
     }
}

void
writeMouseConfig(_INI_INSTANCE *sInstance)
{
     int  i;

     if (!hmiINILocateSection(sInstance,"MOUSE")) {
          hmiINIAddSection(sInstance,"MOUSE");
     }
     for (i=0 ; i < MAXMOUSEBUTTONS ; i++) {
          if (!hmiINILocateItem(sInstance,mouseControlLabel[i])) {
               if (configMouse[i] != 255) {
                    hmiINIAddItemDecimal(sInstance,mouseControlLabel[i],
                                         (WORD)configMouse[i],
                                         strlen(mouseControlLabel[i])+4,16);
               }
          }
          else {
               hmiINIWriteDecimal(sInstance,(WORD)configMouse[i]);
          }
     }
}
#endif

#ifdef CON_JOYSTICK
void
readJoystickConfig(_INI_INSTANCE *sInstance)
{
     int  i,j;

     for (i=0 ; i < MAXJOYSTICKBUTTONS ; i++) {
          configJoystick[i]=255;
     }
     if (!hmiINILocateSection(sInstance,"JOYSTICK")) {
          return;
     }
     for (i=0 ; i < MAXJOYSTICKBUTTONS ; i++) {
          if (hmiINIGetItemString(sInstance,joystickControlLabel[i],
                                  tempBuf,16)) {
               if (strlen(tempBuf) > 0) {
                    sscanf(tempBuf,"%x",&j);
                    configJoystick[i]=j;
               }
          }
     }
}

void
writeJoystickConfig(_INI_INSTANCE *sInstance)
{
     int  i;

     if (!hmiINILocateSection(sInstance,"JOYSTICK")) {
          hmiINIAddSection(sInstance,"JOYSTICK");
     }
     for (i=0 ; i < MAXJOYSTICKBUTTONS ; i++) {
          if (!hmiINILocateItem(sInstance,joystickControlLabel[i])) {
               if (strlen(tempBuf) > 0) {
                    hmiINIAddItemDecimal(sInstance,joystickControlLabel[i],
                                         (WORD)configJoystick[i],
                                         strlen(joystickControlLabel[i])+4,16);
               }
          }
          else {
               hmiINIWriteDecimal(sInstance,(WORD)configJoystick[i]);
          }
     }
}
#endif

#ifdef CON_AVENGER
void
readAvengerConfig(_INI_INSTANCE *sInstance)
{
     int  i,j;

     for (i=0 ; i < MAXAVENGERBUTTONS ; i++) {
          configAvenger[i]=255;
     }
     if (!hmiINILocateSection(sInstance,"AVENGER")) {
          return;
     }
     for (i=0 ; i < MAXAVENGERBUTTONS ; i++) {
          if (hmiINIGetItemString(sInstance,avengerControlLabel[i],
                                  tempBuf,16)) {
               if (strlen(tempBuf) > 0) {
                    sscanf(tempBuf,"%x",&j);
                    configAvenger[i]=j;
               }
          }
     }
}

void
writeAvengerConfig(_INI_INSTANCE *sInstance)
{
     int  i;

     if (!hmiINILocateSection(sInstance,"AVENGER")) {
          hmiINIAddSection(sInstance,"AVENGER");
     }
     for (i=0 ; i < MAXAVENGERBUTTONS ; i++) {
          if (!hmiINILocateItem(sInstance,avengerControlLabel[i])) {
               if (configAvenger[i] != 255) {
                    hmiINIAddItemDecimal(sInstance,avengerControlLabel[i],
                                         (WORD)configAvenger[i],
                                         strlen(avengerControlLabel[i])+4,16);
               }
          }
          else {
               hmiINIWriteDecimal(sInstance,(WORD)configAvenger[i]);
          }
     }
}
#endif

#ifdef CON_GAMEPAD
void
readGamepadConfig(_INI_INSTANCE *sInstance)
{
     int  i,j;

     for (i=0 ; i < MAXGAMEPADBUTTONS ; i++) {
          configGamepad[i]=255;
     }
     if (!hmiINILocateSection(sInstance,"GAMEPAD")) {
          return;
     }
     for (i=0 ; i < MAXGAMEPADBUTTONS ; i++) {
          if (hmiINIGetItemString(sInstance,gamepadControlLabel[i],
                                  tempBuf,16)) {
               if (strlen(tempBuf) > 0) {
                    sscanf(tempBuf,"%x",&j);
                    configGamepad[i]=j;
               }
          }
     }
}

void
writeGamepadConfig(_INI_INSTANCE *sInstance)
{
     int  i;

     if (!hmiINILocateSection(sInstance,"GAMEPAD")) {
          hmiINIAddSection(sInstance,"GAMEPAD");
     }
     for (i=0 ; i < MAXGAMEPADBUTTONS ; i++) {
          if (!hmiINILocateItem(sInstance,gamepadControlLabel[i])) {
               if (configGamepad[i] != 255) {
                    hmiINIAddItemDecimal(sInstance,gamepadControlLabel[i],
                                         (WORD)configGamepad[i],
                                         strlen(gamepadControlLabel[i])+4,16);
               }
          }
          else {
               hmiINIWriteDecimal(sInstance,(WORD)configGamepad[i]);
          }
     }
}
#endif

#ifdef CON_WINGMAN
void
readWingmanConfig(_INI_INSTANCE *sInstance)
{
     int  i,j;

     for (i=0 ; i < MAXWINGMANBUTTONS ; i++) {
          configWingman[i]=255;
     }
     if (!hmiINILocateSection(sInstance,"WINGMAN")) {
          return;
     }
     for (i=0 ; i < MAXWINGMANBUTTONS ; i++) {
          if (hmiINIGetItemString(sInstance,wingmanControlLabel[i],
                                  tempBuf,16)) {
               if (strlen(tempBuf) > 0) {
                    sscanf(tempBuf,"%x",&j);
                    configWingman[i]=j;
               }
          }
     }
}

void
writeWingmanConfig(_INI_INSTANCE *sInstance)
{
     int  i;

     if (!hmiINILocateSection(sInstance,"WINGMAN")) {
          hmiINIAddSection(sInstance,"WINGMAN");
     }
     for (i=0 ; i < MAXWINGMANBUTTONS ; i++) {
          if (!hmiINILocateItem(sInstance,wingmanControlLabel[i])) {
               if (configWingman[i] != 255) {
                    hmiINIAddItemDecimal(sInstance,wingmanControlLabel[i],
                                         (WORD)configWingman[i],
                                         strlen(wingmanControlLabel[i])+4,16);
               }
          }
          else {
               hmiINIWriteDecimal(sInstance,(WORD)configWingman[i]);
          }
     }
}
#endif

#ifdef CON_VFX1
void
readVFX1Config(_INI_INSTANCE *sInstance)
{
     int  i,j;

     for (i=0 ; i < MAXVFX1BUTTONS ; i++) {
          configVFX1[i]=255;
     }
     if (!hmiINILocateSection(sInstance,"VFX1")) {
          return;
     }
     for (i=0 ; i < MAXVFX1BUTTONS ; i++) {
          if (hmiINIGetItemString(sInstance,VFX1ControlLabel[i],
                                  tempBuf,16)) {
               if (strlen(tempBuf) > 0) {
                    sscanf(tempBuf,"%x",&j);
                    configVFX1[i]=j;
               }
          }
     }
}

void
writeVFX1Config(_INI_INSTANCE *sInstance)
{
     int  i;

     if (!hmiINILocateSection(sInstance,"VFX1")) {
          hmiINIAddSection(sInstance,"VFX1");
     }
     for (i=0 ; i < MAXVFX1BUTTONS ; i++) {
          if (!hmiINILocateItem(sInstance,VFX1ControlLabel[i])) {
               if (configVFX1[i] != 255) {
                    hmiINIAddItemDecimal(sInstance,VFX1ControlLabel[i],
                                         (WORD)configVFX1[i],
                                         strlen(VFX1ControlLabel[i])+4,16);
               }
          }
          else {
               hmiINIWriteDecimal(sInstance,(WORD)configVFX1[i]);
          }
     }
}
#endif

void
readVideoMode(_INI_INSTANCE *sIns)
{
     W32  wTemp,wTemp1,wTemp2;
     BYTE szTemp[16];

     if (hmiINILocateSection(sIns,"VIDEO") == _TRUE) {
          wTemp1=0;
          wTemp2=0;
          if (hmiINILocateItem(sIns,"VMODE") == _TRUE) {
               if (hmiINIGetString(sIns,szTemp,16) == _TRUE) {
                    if (hmiINILocateItem(sIns,"XSIZE") == _TRUE) {
                         if (hmiINIGetDecimal(sIns,&wTemp) == _TRUE) {
                              wTemp1=wTemp;
                         }
                    }
                    if (hmiINILocateItem(sIns,"YSIZE") == _TRUE) {
                         if (hmiINIGetDecimal(sIns,&wTemp) == _TRUE) {
                              wTemp2=wTemp;
                         }
                    }
               }
          }
          wTemp=validVideoMode(szTemp,(short *)&wTemp1,(short *)&wTemp2);
          if ((int)wTemp != -1) {
               videoMode=wTemp;
               videoResX=wTemp1;
               videoResY=wTemp2;
          }
     }
}

void
writeVideoMode(_INI_INSTANCE *sIns)
{
     if (!hmiINILocateSection(sIns,"VIDEO")) {
          hmiINIAddSection(sIns,"VIDEO");
     }
     if (!hmiINILocateItem(sIns,"VMODE")) {
          hmiINIAddItemString(sIns,"VMODE",(PSTR)getVideoModeName(videoMode),16);
     }
     else {
          hmiINIWriteString(sIns,(PSTR)getVideoModeName(videoMode));
     }
     if (!hmiINILocateItem(sIns,"XSIZE")) {
          hmiINIAddItemDecimal(sIns,"XSIZE",videoResX,16,10);
     }
     else {
          hmiINIWriteDecimal(sIns,videoResX);
     }
     if (!hmiINILocateItem(sIns,"YSIZE")) {
          hmiINIAddItemDecimal(sIns,"YSIZE",videoResY,16,10);
     }
     else {
          hmiINIWriteDecimal(sIns,videoResY);
     }
}

void
readControlConfigs(int readhmi)
{
     int  i;
     _INI_INSTANCE sInstance;

     if (readhmi) {
          readHMICFGFile(configFile,NULL,NULL);
     }
     strcpy(controlConfigFile,GAMEINIFILE);
     if (!hmiINIOpen(&sInstance,controlConfigFile)) {
          return;
     }
     for (i=0 ; i < MAXACTIONS ; i++) {
          controlAction[i]=controlArray[i].desc;
     }
     controlAction[i]=NULL;
     readKeyboardConfig(&sInstance);
#ifdef CON_MOUSE
     readMouseConfig(&sInstance);
#endif
#ifdef CON_JOYSTICK
     readJoystickConfig(&sInstance);
#endif
#ifdef CON_AVENGER
     readAvengerConfig(&sInstance);
#endif
#ifdef CON_GAMEPAD
     readGamepadConfig(&sInstance);
#endif
#ifdef CON_WINGMAN
     readWingmanConfig(&sInstance);
#endif
#ifdef CON_VFX1
     readVFX1Config(&sInstance);
#endif
     readVideoMode(&sInstance);
     hmiINIClose(&sInstance);
}

void
writeControlConfigs(void)
{
     int  hFile;
     _INI_INSTANCE sInstance;

     if (!hmiINIOpen(&sInstance,controlConfigFile)) {
          if ((hFile=creat(controlConfigFile,S_IREAD|S_IWRITE)) == -1) {
               return;
          }
          close(hFile);
          if (!hmiINIOpen(&sInstance,controlConfigFile)) {
               return;
          }
     }
     writeKeyboardConfig(&sInstance);
#ifdef CON_MOUSE
     writeMouseConfig(&sInstance);
#endif
#ifdef CON_JOYSTICK
     writeJoystickConfig(&sInstance);
#endif
#ifdef CON_AVENGER
     writeAvengerConfig(&sInstance);
#endif
#ifdef CON_GAMEPAD
     writeGamepadConfig(&sInstance);
#endif
#ifdef CON_WINGMAN
     writeWingmanConfig(&sInstance);
#endif
#ifdef CON_VFX1
     writeVFX1Config(&sInstance);
#endif
     writeVideoMode(&sInstance);
     hmiINIClose(&sInstance);
}
