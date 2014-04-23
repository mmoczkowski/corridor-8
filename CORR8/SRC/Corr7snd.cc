/***************************************************************************
 *   CORR7SND.C - SOS related functions for Corridor 7
 *
 *                                                     02/28/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   MAXSAMPLES               16
#define   SNDBUFSIZE               32768L
#define   MAXSOUNDS                100  // default+user definable sounds

#define   LOWPRIORITY         256
#define   MEDPRIORITY         128
#define   HIPRIORITY          0

int  noSoundFlag,
     stereoSwapFlag;

int  musicVolume=15,
     soundVolume=15;

W32  hDriver,
     hSong;

W32  hDIGISample[MAXSAMPLES];

W32  wDIGIDeviceIDs[_SETUP_MAX_DEVICES],
     wDIGIPorts[_SETUP_MAX_DEVICES],
     wDIGIIRQ[_SETUP_MAX_DEVICES],
     wDIGIDMA[_SETUP_MAX_DEVICES],
     wDIGIOrder[_SETUP_MAX_DEVICES],
     wDIGIIndex,
     wDIGILastDetect=-1;

W16  wDIGIPortList[_SETUP_MAX_DEVICES],
     wDIGIIRQList[_SETUP_MAX_DEVICES],
     wDIGIDMAList[_SETUP_MAX_DEVICES],
     wDIGIStereo[_SETUP_MAX_DEVICES],
     wDIGI16Bit[_SETUP_MAX_DEVICES];

W32  wMIDIDeviceIDs[_SETUP_MAX_DEVICES],
     wMIDIPortList[_SETUP_MAX_DEVICES][_SETUP_MIDI_PLIST],
     wMIDIPorts[_SETUP_MAX_DEVICES],
     wMIDIIndexs[_SETUP_MAX_DEVICES],
     wMIDIIndex,
     wMIDIMelodicSize,
     wMIDIDrumSize;

W32  wDeviceID,
     wMDeviceID;

PSTR pMIDIMelodic,
     pMIDIDrum;

int  nextSample;

struct sndmemptr {
     long cache_ptr;
     long cache_length;
     char cache_lock;
} sndptr[MAXSAMPLES];

char configFile[_MAX_PATH],
     digiDeviceName[_SETUP_MAX_DEVICES][_SETUP_STRLEN],
     *sDIGIDeviceName[_SETUP_MAX_DEVICES],
     midiDeviceName[_SETUP_MAX_DEVICES][_SETUP_STRLEN],
     *sMIDIDeviceName[_SETUP_MAX_DEVICES];

enum {
     S_EXPLOSION,
     S_PLAYERDIE,
     S_TELEPORT,
     S_DOOROPEN,
     S_DOORCLOSE,
     S_PLATFORMSTART,
     S_PLATFORMSTOP,
     S_PLATFORMMOVE,
     S_PLAYERPAIN,
     S_SWITCH1,
     S_SWITCH2,
     S_PLAYERGORE,
     S_ENEMY1DIE,
     S_ENEMY1SIGHT
};

char *soundDesc[MAXSOUNDS],
     *soundIndex[MAXSOUNDS];


_SOS_CAPABILITIES sDIGICapabilities;
_SOS_SAMPLE *pDIGISampleData[MAXSAMPLES],
     *sData;

extern
W32  wDIGIDeviceID,
     wMIDIDeviceID;

extern
HANDLE hDIGIDriver,
     hDIGITimer,
     hMIDIDriver;

extern
_SOS_DIGI_DRIVER sDIGIDriver;

extern
_SOS_MIDI_DRIVER sMIDIDriver;

#if 0
PSTR
setupLoadFile(PSTR szName,W32 *wLoadSize)
{
     W32  hFile,wSize;
     PSTR pData;

     sprintf(tempbuf,"%s",szName);
     if ((hFile=open(tempbuf,O_RDONLY|O_BINARY)) == -1) {
          return(_NULL);
     }
     wSize=lseek(hFile,0,SEEK_END);
     if ((pData=(PSTR)ENG_alcmem(wSize)) == _NULL) {
          close(hFile);
          return(_NULL);
     }
     lseek(hFile,0,SEEK_SET);
     if (read(hFile,pData,wSize) != wSize) {
          close(hFile);
          ENG_alcfree((long)pData);
          return(_NULL);
     }
     close(hFile);
     *wLoadSize=wSize;
     return(pData);
}
#endif

int
setupInit(char *setupFile)
{
     int  i,p;
     char buf[40];
     W32  wTemp;
     _INI_INSTANCE sInstance;

     sprintf(tempbuf,"%s",setupFile);
     if (!hmiINIOpen(&sInstance,tempbuf)) {
          return(0);
     }
     if (!hmiINILocateSection(&sInstance,"DIGITAL")) {
          hmiINIClose(&sInstance);
          return(0);
     }
     for (i=0 ; i < _SETUP_MAX_DEVICES ; i++) {
          sDIGIDeviceName[i]=&digiDeviceName[i];
     }
     i=0;
     do {
          sprintf(buf,"Device%03d",i);
          if (hmiINIGetItemString(&sInstance,buf,(PSTR)sDIGIDeviceName[i],
                                  _SETUP_STRLEN)) {
               hmiINIGetDecimal(&sInstance,&wDIGIDeviceIDs[i]);
               hmiINIGetDecimal(&sInstance,&wDIGIPorts[i]);
               hmiINIGetDecimal(&sInstance,&wDIGIIRQ[i]);
               hmiINIGetDecimal(&sInstance,&wDIGIDMA[i]);
               hmiINIGetDecimal(&sInstance,&wMIDIIndexs[i]);
               hmiINIGetString(&sInstance,buf,16);
               if (stricmp(buf,"YES") == 0) {
                    wDIGIStereo[i]=_TRUE;
               }
               else {
                    wDIGIStereo[i]=_FALSE;
               }
               hmiINIGetString(&sInstance,buf,16);
               if (stricmp(buf,"YES") == 0) {
                    wDIGI16Bit[i]=_TRUE;
               }
               else {
                    wDIGI16Bit[i]=_FALSE;
               }
          }
          else {
               break;
          }
          i++;
     } while (1);
     sDIGIDeviceName[i]=NULL;
     if (!hmiINILocateSection(&sInstance,"DETECTION")) {
          hmiINIClose(&sInstance);
          return(0);
     }
     if (!hmiINILocateItem(&sInstance,"DeviceOrder")) {
          hmiINIClose(&sInstance);
          return(0);
     }
     i=0;
     while (hmiINIGetDecimal(&sInstance,&wDIGIOrder[i])) {
          i++;
     }
     wDIGIOrder[i]=-1;
     if (!hmiINILocateSection(&sInstance,"MIDI")) {
          hmiINIClose(&sInstance);
          return(0);
     }
     for (i=0 ; i < _SETUP_MAX_DEVICES ; i++) {
          sMIDIDeviceName[i]=&midiDeviceName[i];
     }
     i=0;
     do {
          sprintf(buf,"Device%03d",i);
          if (hmiINIGetItemString(&sInstance,buf,(PSTR)sMIDIDeviceName[i],
                                  _SETUP_STRLEN)) {
               hmiINIGetDecimal(&sInstance,&wMIDIDeviceIDs[i]);
               if (i < _SETUP_MAX_DEVICES) {
                    i++;
               }
          }
          else {
               break;
          }
     } while (1);
     sMIDIDeviceName[i]=NULL;
     if (!hmiINILocateSection(&sInstance,"MIDISETTINGS")) {
          hmiINIClose(&sInstance);
          return(0);
     }
     i=0;
     do {
          sprintf(buf,"Device%03d",i);
          if (hmiINILocateItem(&sInstance,buf)) {
               p=0;
               hmiINIGetDecimal(&sInstance,&wTemp);
               while (hmiINIGetDecimal(&sInstance,&wMIDIPortList[i][p])) {
                    if (p == wTemp-1) {
                         wMIDIPorts[i]=wMIDIPortList[i][p];
                    }
                    if (p < _SETUP_MIDI_PLIST) {
                         p++;
                    }
               }
               wMIDIPortList[i][p]=-1;
               if (i < _SETUP_MAX_DEVICES) {
                    i++;
               }
          }
          else {
               break;
          }
     } while (1);
     hmiINIClose(&sInstance);
     return(1);
}

BOOL
setupDIGIAutoDetectDevice(W32 wDeviceID)
{
     W32  wError;

     if (wDeviceID == -1) {
          return(_FALSE);
     }
     sosDIGIDetectInit((PSTR)_NULL);
     if ((wError=sosDIGIDetectFindHardware(wDeviceID,&sDIGIDriver))) {
          sosDIGIDetectUnInit();
          return(_FALSE);
     }
     if ((wError=sosDIGIDetectGetSettings(&sDIGIDriver))) {
          sosDIGIDetectUnInit();
          return(_FALSE);
     }
     memcpy(&sDIGICapabilities,&sDIGIDriver.sCaps,sizeof(_SOS_CAPABILITIES));
     if (sDIGICapabilities.wFlags&_SOS_DCAPS_AUTO_REINIT) {
          sDIGIDriver.sHardware.wIRQ=-1;
     }
     if ((W32)*sDIGICapabilities.lpDMAList == -1) {
          sDIGIDriver.sHardware.wDMA=-1;
     }
     wDIGIDeviceID=wDeviceID;
     sosDIGIDetectUnInit();
     return(_TRUE);
}

W32
setupDIGIPerformDetection(void)
{
     sDIGIDriver.sHardware.wPort=-1;
     sDIGIDriver.sHardware.wDMA=-1;
     sDIGIDriver.sHardware.wIRQ=-1;
     wDIGIDeviceID=-1;
     if (wDIGILastDetect == -1) {
          wDIGILastDetect=0;
     }
     else {
          wDIGILastDetect++;
     }
     while (!setupDIGIAutoDetectDevice(
            wDIGIDeviceIDs[wDIGIOrder[wDIGILastDetect]])) {
          wDIGILastDetect++;
     }
     if (wDIGIOrder[wDIGILastDetect] != -1) {
          wDIGIIndex=wDIGIOrder[wDIGILastDetect];
          wMIDIIndex=wMIDIIndexs[wDIGIIndex];
     }
     else {
          wDIGILastDetect=-1;
     }
     return(0);
}

char *
SND_getSoundCardName(W32 index)
{
     return(sDIGIDeviceName[index]);
}

BOOL
SND_getCaps(W32 wDeviceID)
{
     int  i;
     short far *lpData;

     if (sosDIGIDetectInit((PSTR)_NULL)) {
          return(_FALSE);
     }
     if (sosDIGIDetectGetCaps(wDeviceID,&sDIGIDriver)) {
          return(_FALSE);
     }
     memcpy(&sDIGICapabilities,&sDIGIDriver.sCaps,sizeof(_SOS_CAPABILITIES));
     lpData=sDIGICapabilities.lpPortList;
     i=0;
     while (*lpData != -1) {
          wDIGIPortList[i++]=*lpData++;
     }
     wDIGIPortList[i]=-1;
     lpData=sDIGICapabilities.lpIRQList;
     i=0;
     while (*lpData != -1) {
          wDIGIIRQList[i++]=*lpData++;
     }
     wDIGIIRQList[i]=-1;
     lpData=sDIGICapabilities.lpDMAList;
     i=0;
     while (*lpData != -1) {
          wDIGIDMAList[i++]=*lpData++;
     }
     wDIGIDMAList[i]=-1;
     sosDIGIDetectUnInit();
     return(_TRUE);
}

BOOL
SND_getCapsIndex(W32 index)
{
     return(SND_getCaps(wDIGIDeviceIDs[index]));
}

VOID cdecl
sosDIGIDoneCallback(_SOS_SAMPLE *sData)
{
     int  index;

     for (index=0 ; index < MAXSAMPLES ; index++) {
          if (hDIGISample[index] == sData->hSample) {
               break;
          }
     }
     hDIGISample[index]=-1;
     SND_alcfree((long)sData);
}

long
SND_alcmem(long numbytes)
{
     int  i;

     for (i=0 ; i < MAXSAMPLES ; i++) {
          if (sndptr[i].cache_lock < 200) {
               if (sndptr[i].cache_length >= numbytes) {
                    sndptr[i].cache_lock=203;
                    return(sndptr[i].cache_ptr);
               }
          }
     }
     return(0L);
}

void
SND_alcfree(long ptr)
{
     int  i;

     for (i=0 ; i < MAXSAMPLES ; i++) {
          if (sndptr[i].cache_ptr == ptr) {
               sndptr[i].cache_lock=1;
               return;
          }
     }
}

BOOL
SND_SOSInitialized(void)
{
     if (_sSOSSystem.wFlags != _SOS_SYSTEM_INITIALIZED) {
          return(_FALSE);
     }
     return(_TRUE);
}

BOOL
SND_digiInitialized(void)
{
     if ((sDIGIDriver.wFlags&_SOS_DRV_INITIALIZED) == 0) {
          return(_FALSE);
     }
     return(_TRUE);
}

void
SND_initSOS(void)
{
     int  i;

     if (!SND_SOSInitialized()) {
          setupInit(GAMEINIFILE);
          if (!noSoundFlag) {
               if (MEN_getSoundConfig()) {
                    SND_loadSoundDriverIndex(wDIGIIndex);
                    SMK_init();
               }
          }
     }
     for (i=0 ; i < MAXSAMPLES ; i++) {
          hDIGISample[i]=-1;
     }
     for (i=0 ; i < MAXSAMPLES ; i++) {
          sndptr[i].cache_lock=1;
          sndptr[i].cache_length=SNDBUFSIZE;
          allocache(&(sndptr[i].cache_ptr), sndptr[i].cache_length,
                    &(sndptr[i].cache_lock));
     }
}

void
SND_initSounds(char *iniFile)
{
     short i,n;
     char locbuf[80];
     _INI_INSTANCE soundIns;

     if (!hmiINIOpen(&soundIns,GAMEINIFILE)) {
          crash("SND_initSoundFilenames: %s FILE NOT FOUND!",GAMEINIFILE);
     }
     else {
          if (!hmiINILocateSection(&soundIns,"SOUNDS")) {
               crash("SND_initSoundFilenames: SOUNDS section missing in %s!",
                     iniFile);
          }
          for (i=0,n=0 ; i < MAXSOUNDS ; i++) {
               sprintf(locbuf,"SOUNDDESC%d",i+1);
               if (hmiINIGetItemString(&soundIns,locbuf,tempbuf,128)) {
                    soundDesc[i]=strdup(tempbuf);
               }
               sprintf(locbuf,"SOUNDFILE%d",i+1);
               if (hmiINIGetItemString(&soundIns,locbuf,tempbuf,128)) {
                    soundIndex[i]=strdup(tempbuf);
               }
          }
          hmiINIClose(&soundIns);
     }
}

void
SND_unInitPointers(void)
{
     int  i;

     for (i=0 ; i < MAXSOUNDS ; i++) {
          if (soundDesc[i] != NULL) {
               free(soundDesc[i]);
          }
          if (soundIndex[i] != NULL) {
               free(soundIndex[i]);
          }
     }
}

W32
SND_getDIGIIndex(W32 wDeviceID)
{
     W32  i;

     for (i=0 ; i < _SETUP_MAX_DEVICES ; i++) {
          if (wDIGIDeviceIDs[i] == wDeviceID) {
               return(i);
          }
     }
     return(0);
}

W32
SND_getMIDIIndex(W32 wDeviceID)
{
     W32  i;

     for (i=0 ; i < _SETUP_MAX_DEVICES ; i++) {
          if (wMIDIDeviceIDs[i] == wDeviceID) {
               return(i);
          }
     }
     return(0);
}

void
SND_loadSoundDriver(W32 wDeviceID,W32 wMDeviceID)
{
     W32  wIndex;

     wIndex=SND_getDIGIIndex(wDeviceID);
     if (wIndex != -1) {
          if (wDIGIStereo[wIndex]) {
               wDeviceID+=1;
          }
#if 0
          if (wDIGI16Bit[wIndex]) {
               wDeviceID+=2;
          }
#endif
     }
     sDIGIDriver.wDriverRate=_SOSEZ_SAMPLE_RATE;
     sDIGIDriver.wDMABufferSize=_SOSEZ_DMA_BUFFER;
     if (!SND_SOSInitialized()) {
          sosDIGIInitSystem(_NULL,_SOS_DEBUG_NORMAL);
          sosMIDIInitSystem(_NULL,_SOS_DEBUG_NORMAL);
     }
     if (wMDeviceID != -1) {
          sMIDIDriver.wID=wMDeviceID;
          if (sosMIDIInitDriver(&sMIDIDriver,&hMIDIDriver)) {
               crash(sosGetErrorString(_SOSEZ_MIDI_INIT));
          }
     }
     if (wDeviceID != -1) {
          sDIGIDriver.wID=wDeviceID;
          if (sosDIGIInitDriver(&sDIGIDriver,&hDIGIDriver)) {
               crash(sosGetErrorString(_SOSEZ_DIGI_INIT));
          }
     }
     if (wDeviceID != -1) {
          sosTIMERRegisterEvent(_SOSEZ_TIMER_RATE,sDIGIDriver.pfnMixFunction,
                                &hDIGITimer);
     }
     if (wMDeviceID == _MIDI_FM || wMDeviceID == _MIDI_OPL3) {
          if ((pMIDIMelodic=sosEZLoadPatch("melodic.bnk")) == _NULL) {
               crash(sosGetErrorString(_SOSEZ_PATCH_MELODIC));
          }
          if ((pMIDIDrum=sosEZLoadPatch("drum.bnk")) == _NULL) {
               crash(sosGetErrorString(_SOSEZ_PATCH_DRUM));
          }
          if ((sosMIDISetInsData(hMIDIDriver,pMIDIMelodic,1))) {
               crash(sosGetErrorString(_SOSEZ_PATCH_INIT));
          }
          if ((sosMIDISetInsData(hMIDIDriver,pMIDIDrum,1))) {
               crash(sosGetErrorString(_SOSEZ_PATCH_INIT));
          }
     }
}

void
SND_loadSoundDriverIndex(W32 wIndex)
{
     W32  wDeviceID,
          wMDeviceID=-1;

     wDeviceID=wDIGIDeviceIDs[wIndex];
     SND_loadSoundDriver(wDeviceID,wMDeviceID);
}

void
SND_uninitSOS(void)
{
     if (SND_digiInitialized()) {
          sosTIMERRemoveEvent(hDIGITimer);
          sosDIGIUnInitDriver(hDriver,_TRUE,_TRUE);
          sosMIDIUnInitDriver(hMIDIDriver,_TRUE);
     }
     if (SND_SOSInitialized()) {
          sosMIDIUnInitSystem();
          sosDIGIUnInitSystem();
     }
}

short
SND_getSoundVolume(void)
{
     return(soundVolume);
}

void
SND_setSoundVolume(short vol)
{
     soundVolume=vol;
}

short
SND_getMusicVolume(void)
{
     return(musicVolume);
}

void
SND_setMusicVolume(short vol)
{
     musicVolume=vol;
}

W32
SND_getNextSample(void)
{
     W32  startSample;

     startSample=nextSample;
     while (hDIGISample[nextSample] != -1) {
          nextSample=(nextSample+1)&(MAXSAMPLES-1);
          if (nextSample == startSample) {
               return(-1);
          }
     }
     return(nextSample);
}

W32
SND_playSample(W32 owner,W32 pri,W32 wVol,char *wavFile,W32 loopCount)
{
     char soundPath[_MAX_PATH];

     if (!SND_digiInitialized()) {
          return(-1);
     }
     if (SND_getNextSample() == -1) {
          return(-1);
     }
     SND_alcfree((long)pDIGISampleData[nextSample]);
     sprintf(soundPath,"wavs\\%s",wavFile);
     sData=sosEZLoadSample(soundPath);
     if (sData == NULL) {
          return(-1);
     }
     sData->pfnSampleDone=(VOID *)&sosDIGIDoneCallback;
     if (wVol > (soundVolume<<11)) {
          wVol=soundVolume<<11;
     }
     sData->wVolume=MK_VOLUME(wVol,wVol);
     sData->wPanPosition=_PAN_CENTER;
     sData->wLoopCount=loopCount;
     sData->wID=owner;
     sData->wPriority=pri;
     pDIGISampleData[nextSample]=sData;
     hDIGISample[nextSample]=sosDIGIStartSample(hDriver,sData);
     return(hDIGISample[nextSample]);
}

void
SND_setSampleLocation(long x,long y,short snum,W32 hSample)
{
     short ang;
     long xx,yy;
     W32  wPan,wVol;
     DWORD sqdx;
     spritetype *spr;

     spr=spritePtr[snum];
     if (spr->x == x && spr->y == y) {
          wVol=0x7FFF;
          wPan=_PAN_CENTER;
     }
     else {
          xx=ksqrt(spr->x-x)<<1;
          yy=ksqrt(spr->y-y)<<1;
          sqdx=(xx*xx)+(yy*yy);
          if (sqdx > 0x7FFF) {
               sqdx=0x7FFF;
          }
          wVol=0x7FFF-sqdx;
          ang=(getangle(spr->x-x,spr->y-y)-spr->ang)&2047;
          if (ang > 1024) {   // right side
               if (ang <= 1536) {
                    wPan=0x8000+((ang-1024)<<6);
               }
               else {
                    wPan=0x8000+((2048-ang)<<6);
               }
               if (wPan > 0xFFFF) {
                    wPan=0xFFFF;
               }
          }
          else {              // left side
               if (ang >= 512) {
                    wPan=0x8000-((1024-ang)<<6);
               }
               else {
                    wPan=0x8000-(ang<<6);
               }
               if (wPan > 0x8000) {
                    wPan=0;
               }
          }
     }
     sosDIGISetPanLocation(hDriver,hSample,wPan);
     if (wVol > (soundVolume<<11)) {
          wVol=soundVolume<<11;
     }
     if (wVol < 0x400) {
          wVol=0x400;
     }
     sosDIGISetSampleVolume(hDriver,hSample,wVol);
}

W32
SND_playSampleXY(long x,long y,W32 owner,W32 pri,char *wavFile,W32 loopCount)
{
     W32  hSample;

     hSample=SND_playSample(owner,pri,0,wavFile,loopCount);
     SND_setSampleLocation(x,y,GAM_getViewSprite(),hSample);
     return(hSample);
}

W32
SND_playSampleLocation(W32 owner,W32 pri,char *wavFile,W32 loopCount)
{
     W32  hSample;
     long x,y;

     x=spritePtr[owner]->x;
     y=spritePtr[owner]->y;
     hSample=SND_playSample(owner,pri,0,wavFile,loopCount);
     SND_setSampleLocation(x,y,GAM_getViewSprite(),hSample);
     return(hSample);
}

W32
SND_playSampleIndex(W32 owner,W32 pri,W32 sound,W32 loopCount)
{
     W32  hSample;

     hSample=SND_playSampleLocation(owner,pri,soundIndex[sound],loopCount);
     return(hSample);
}

W32
SND_playSampleIndexXY(long x,long y,W32 owner,W32 pri,W32 sound,W32 loopCount)
{
     W32  hSample;

     hSample=SND_playSampleXY(x,y,owner,pri,soundIndex[sound],loopCount);
     return(hSample);
}

void
SND_stopSampleIndex(W32 owner)
{
     W32  hSample;

     if ((hSample=sosDIGIGetSampleHandle(hDriver,owner)) > 0) {
          sosDIGIStopSample(hDriver,hSample);
     }
}

W32
SND_soundPlaying(W32 owner)
{
     W32  hSample;

     if ((hSample=sosDIGIGetSampleHandle(hDriver,owner)) > 0) {
          return(hSample);
     }
     return(-1);
}

void
SND_stopAllSounds(void)
{
     W32  hSample,i;

     for (i=0 ; i < MAXSAMPLES ; i++) {
          if ((hSample=hDIGISample[i]) > 0) {
               if (sosDIGISampleDone(hDriver,hSample) == _FALSE) {
                    sosDIGIStopSample(hDriver,hSample);
               }
          }
     }
}

void
SND_debugSoundHandles(void)
{
#ifdef ANALYZESOUNDINDEXDEBUG
     int  i;

     strcpy(tempbuf,"SOUNDI: ");
     for (i=0 ; i < MAXSAMPLES ; i++) {
          if (hDIGISample[i] != -1) {
               strcat(tempbuf,"1");
          }
          else {
               strcat(tempbuf,"0");
          }
     }
     debugOut(windowx1,windowy1,tempbuf);
#endif
}

void
SND_debugMemoryPtrs(void)
{
#ifdef ANALYZESOUNDINDEXDEBUG
     int  i;

     strcpy(tempbuf,"SOUNDP: ");
     for (i=0 ; i < MAXSAMPLES ; i++) {
          if (sndptr[i].cache_lock >= 200) {
               strcat(tempbuf,"1");
          }
          else {
               strcat(tempbuf,"0");
          }
     }
     debugOut(windowx1,windowy1,tempbuf);
#endif
}

void
SND_writeConfig(void)
{
     _INI_INSTANCE sInstance;

     wDIGIDeviceID=wDIGIDeviceIDs[wDIGIIndex];
     wMIDIDeviceID=wMIDIDeviceIDs[wMIDIIndex];
     if (!hmiINIOpen(&sInstance,GAMEINIFILE)) {
          return;
     }
     if (!hmiINILocateSection(&sInstance,"DIGITAL")) {
          hmiINIAddSection(&sInstance,"DIGITAL");
     }
     if (!hmiINILocateItem(&sInstance,"DeviceID")) {
          hmiINIAddItemDecimal(&sInstance,"DeviceID",wDIGIDeviceID,
                               _SETUP_JUSTIFY,16);
     }
     else {
          if (!hmiINIWriteDecimal(&sInstance,wDIGIDeviceID)) {
               goto done;
          }
     }
     if (!hmiINILocateItem(&sInstance,"DevicePort")) {
          hmiINIAddItemDecimal(&sInstance,"DevicePort",
                               sDIGIDriver.sHardware.wPort,_SETUP_JUSTIFY,16);
     }
     else {
          if (!hmiINIWriteDecimal(&sInstance,sDIGIDriver.sHardware.wPort)) {
               goto done;
          }
     }
     if (!hmiINILocateItem(&sInstance,"DeviceDMA")) {
          hmiINIAddItemDecimal(&sInstance,"DeviceDMA",
                               sDIGIDriver.sHardware.wDMA,_SETUP_JUSTIFY,10);
     }
     else {
          if (!hmiINIWriteDecimal(&sInstance,sDIGIDriver.sHardware.wDMA)) {
               goto done;
          }
     }
     if (!hmiINILocateItem(&sInstance,"DeviceIRQ")) {
          hmiINIAddItemDecimal(&sInstance,"DeviceIRQ",
                               sDIGIDriver.sHardware.wIRQ,_SETUP_JUSTIFY,10);
     }
     else {
          if (!hmiINIWriteDecimal(&sInstance,sDIGIDriver.sHardware.wIRQ)) {
               goto done;
          }
     }
     if (!hmiINILocateItem(&sInstance,"DeviceName")) {
          if (wDIGIIndex == -1) {
               hmiINIAddItemString(&sInstance,"DeviceName","No Digital Device",
                                   _SETUP_JUSTIFY);
          }
          else {
               hmiINIAddItemString(&sInstance,"DeviceName",
                                   sDIGIDeviceName[wDIGIIndex],_SETUP_JUSTIFY);
          }
     }
     else {
          if (wDIGIIndex == -1) {
               hmiINIWriteString(&sInstance,"No Digital Device");
          }
          else {
               hmiINIWriteString(&sInstance,sDIGIDeviceName[wDIGIIndex]);
          }
     }
     if (!hmiINILocateSection(&sInstance,"MIDI")) {
          hmiINIAddSection(&sInstance,"MIDI");
     }
     if (!hmiINILocateItem(&sInstance,"DeviceID")) {
          hmiINIAddItemDecimal(&sInstance,"DeviceID",wMIDIDeviceID,
                               _SETUP_JUSTIFY,16);
     }
     else {
          if (!hmiINIWriteDecimal(&sInstance,wMIDIDeviceID)) {
               goto done;
          }
     }
     if (!hmiINILocateItem(&sInstance,"DevicePort")) {
          hmiINIAddItemDecimal(&sInstance,"DevicePort",
                               sMIDIDriver.sHardware.wPort,_SETUP_JUSTIFY,16);
     }
     else {
          if (!hmiINIWriteDecimal(&sInstance,sMIDIDriver.sHardware.wPort)) {
               goto done;
          }
     }
     if (!hmiINILocateItem(&sInstance,"DeviceName")) {
          if (wMIDIIndex == -1) {
               hmiINIAddItemString(&sInstance,"DeviceName","No MIDI Device",
                                   _SETUP_JUSTIFY);
          }
          else {
               hmiINIAddItemString(&sInstance,"DeviceName",
                                   sMIDIDeviceName[wMIDIIndex],_SETUP_JUSTIFY);
          }
     }
     else {
          if (wMIDIIndex == -1) {
               hmiINIWriteString(&sInstance,"No MIDI Device");
          }
          else {
               hmiINIWriteString(&sInstance,sMIDIDeviceName[wMIDIIndex]);
          }
     }
done:
     hmiINIClose(&sInstance);
     return;
}

//**
//** game sound routines
//**

int
SND_isAmbientSound(short s)
{
     if (spritePtr[s]->picnum == AMBIENTSOUNDPIC) {
          return(1);
     }
     return(0);
}

int
SND_isSectorSound(short s)
{
     if (spritePtr[s]->picnum == SECTORSOUNDPIC) {
          return(1);
     }
     return(0);
}

short
SND_getSoundNum(short s)
{
     return(spritePtr[s]->lotag);
}

void
SND_setSoundNum(short s,short n)
{
     spritePtr[s]->lotag=n;
}

short
SND_getSoundPlaytime(short s)
{
     return(spritePtr[s]->extra);
}

void
SND_setSoundPlaytime(short s,short n)
{
     spritePtr[s]->extra=n;
}

short
SND_getSoundLoopCount(short s)
{
     return(spritePtr[s]->hitag);
}

void
SND_setSoundLoopCount(short s,short n)
{
     spritePtr[s]->hitag=n;
}

void
SND_playAmbientSounds(short s)
{
     short i,nexti,sound;
     long x,y;
     W32  hSample;
     spritetype *spr,*spr2;

     spr=spritePtr[s];
     for (i=headspritestat[STAT_AMBIENT] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr2=spritePtr[i];
          sound=SND_getSoundNum(i);
          hSample=SND_soundPlaying(i);
          if (WEP_getDistance(spr2,spr) < 10000L) {
               if (hSample == -1) {
                    SND_playSampleIndex(i,LOWPRIORITY,sound,-1);
               }
               else {
                    x=spritePtr[i]->x;
                    y=spritePtr[i]->y;
                    SND_setSampleLocation(x,y,s,hSample);
               }
               continue;
          }
          if (hSample != -1) {
               SND_stopSampleIndex(i);
          }
     }
}

void
SND_playSectorMovingSound(short s,int playflag)
{
     short i,p,sound;
     long x,y;
     W32  hSample;

     p=GAM_getViewSprite();
     for (i=headspritesect[s] ; i >= 0 ; i=nextspritesect[i]) {
          if (SND_isSectorSound(i)) {
               if (SND_getSoundPlaytime(i) == PLAYSOUND_MOVING) {
                    sound=SND_getSoundNum(i);
                    hSample=SND_soundPlaying(i);
                    if (playflag) {
                         if (hSample == -1) {
                              SND_playSampleIndex(i,LOWPRIORITY,sound,-1);
                         }
                         else {
                              x=spritePtr[i]->x;
                              y=spritePtr[i]->y;
                              SND_setSampleLocation(x,y,p,hSample);
                         }
                         continue;
                    }
                    if (hSample != -1) {
                         SND_stopSampleIndex(i);
                    }
               }
          }
     }
}

void
SND_playEnterSectorSounds(short s)
{
     short i,loopCount,sound;

     for (i=headspritesect[s] ; i >= 0 ; i=nextspritesect[i]) {
          if (SND_isSectorSound(i)) {
               if (SND_getSoundPlaytime(i) == PLAYSOUND_ENTER) {
                    sound=SND_getSoundNum(i);
                    loopCount=SND_getSoundLoopCount(i);
                    SND_playSampleIndex(i,LOWPRIORITY,sound,loopCount);
               }
          }
     }
}

void
SND_playLeaveSectorSounds(short s)
{
     short i,loopCount,sound;

     for (i=headspritesect[s] ; i >= 0 ; i=nextspritesect[i]) {
          if (SND_isSectorSound(i)) {
               if (SND_getSoundPlaytime(i) == PLAYSOUND_LEAVE) {
                    sound=SND_getSoundNum(i);
                    loopCount=SND_getSoundLoopCount(i);
                    SND_playSampleIndex(i,LOWPRIORITY,sound,loopCount);
               }
          }
     }
}

void
SND_playSectorOpenSound(short s)
{
     short i,loopCount,sound;

     for (i=headspritesect[s] ; i >= 0 ; i=nextspritesect[i]) {
          if (SND_isSectorSound(i)) {
               if (SND_getSoundPlaytime(i) == PLAYSOUND_OPEN) {
                    sound=SND_getSoundNum(i);
                    loopCount=SND_getSoundLoopCount(i);
                    SND_playSampleIndex(i,MEDPRIORITY,sound,loopCount);
               }
          }
     }
}

void
SND_playSectorCloseSound(short s)
{
     short i,loopCount,sound;

     for (i=headspritesect[s] ; i >= 0 ; i=nextspritesect[i]) {
          if (SND_isSectorSound(i)) {
               if (SND_getSoundPlaytime(i) == PLAYSOUND_CLOSE) {
                    sound=SND_getSoundNum(i);
                    loopCount=SND_getSoundLoopCount(i);
                    SND_playSampleIndex(i,MEDPRIORITY,sound,loopCount);
               }
          }
     }
}

void
SND_playSectorStopSound(short s)
{
     short i,loopCount,sound;

     for (i=headspritesect[s] ; i >= 0 ; i=nextspritesect[i]) {
          if (SND_isSectorSound(i)) {
               if (SND_getSoundPlaytime(i) == PLAYSOUND_STOP) {
                    sound=SND_getSoundNum(i);
                    loopCount=SND_getSoundLoopCount(i);
                    SND_playSampleIndex(i,MEDPRIORITY,sound,loopCount);
               }
          }
     }
}

void
SND_playTeleportSound(short s)
{
     SND_playSampleIndex(s,MEDPRIORITY,S_TELEPORT,0);
}

void
SND_playerPainSound(short s)
{
     if ((krand()&4) == 4) {
          SND_playSampleIndex(s,MEDPRIORITY,S_PLAYERPAIN,0);
     }
}

void
SND_playerDieSound(short s)
{
     SND_playSampleIndex(s,HIPRIORITY,S_PLAYERDIE,0);
}

void
SND_playerGoreSound(short s)
{
     SND_playSampleIndex(s,HIPRIORITY,S_PLAYERGORE,0);
}

void
SND_playEnemy1DieSound(short s)
{
     SND_playSampleIndex(s,HIPRIORITY,S_ENEMY1DIE,0);
}

void
SND_playEnemy1SightSound(short s)
{
     SND_playSampleIndex(s,HIPRIORITY,S_ENEMY1SIGHT,0);
}

void
SND_playExplosionSound(short s)
{
     SND_playSampleIndex(s,MEDPRIORITY,S_EXPLOSION,0);
}

void
SND_playSwitchOnSound(short s)
{
     SND_playSampleIndex(s,MEDPRIORITY,S_SWITCH1,0);
}

void
SND_playSwitchOffSound(short s)
{
     SND_playSampleIndex(s,MEDPRIORITY,S_SWITCH2,0);
}

void
SND_playWallSwitchOnSound(short w)
{
     long x,y;

     x=(wallPtr[w]->x+wallPtr[wallPtr[w]->point2]->x)>>1;
     y=(wallPtr[w]->y+wallPtr[wallPtr[w]->point2]->y)>>1;
     SND_playSampleIndexXY(x,y,MAXSPRITES+w,MEDPRIORITY,S_SWITCH1,0);
}

void
SND_playWallSwitchOffSound(short w)
{
     long x,y;

     x=(wallPtr[w]->x+wallPtr[wallPtr[w]->point2]->x)>>1;
     y=(wallPtr[w]->y+wallPtr[wallPtr[w]->point2]->y)>>1;
     SND_playSampleIndexXY(x,y,MAXSPRITES+w,MEDPRIORITY,S_SWITCH2,0);
}

void
SND_playProximityArmedSound(short s)
{
     SND_playSampleIndex(s,HIPRIORITY,S_SWITCH2,0);
}

void
SND_playMenuChooseSound(void)
{
     SND_playSample(-1,HIPRIORITY,0x7FFF,soundIndex[S_PLATFORMSTOP],0);
}

void
SND_playMenuSlideSound(void)
{
     SND_playSample(-1,HIPRIORITY,0x7FFF,soundIndex[S_PLATFORMMOVE],0);
}

void
SND_playMenuDoneSound(void)
{
     SND_playSample(-1,HIPRIORITY,0x7FFF,soundIndex[S_SWITCH2],0);
}

void
SND_playMenuQuitSound(void)
{
     SND_playSample(-1,HIPRIORITY,0x7FFF,soundIndex[S_SWITCH1],0);
}

void
SND_playWeaponSound(W32 owner,char *wavfile,W32 loopCount)
{
     SND_playSampleLocation(owner,HIPRIORITY,wavfile,loopCount);
}

char *
SND_getSoundName(short i)
{
     if (soundDesc[i] != NULL) {
          return(soundDesc[i]);
     }
     return("");
}

void
SND_showSoundList(void)
{
     short i;

     clearmidstatbar16();
     ENG_setMidStatbarPos(8,32);
     ENG_printHelpText16(11,0,"SOUND LIST");
     for (i=0 ; i < MAXSOUNDS ; i++) {
          if (soundIndex[i] != NULL) {
               ENG_printHelpText16(14,1,"%3d = %16.16s",i,SND_getSoundName(i));
          }
     }
}

char *
SND_getSoundPlaytimeName(short i)
{
     switch (i) {
     case PLAYSOUND_NONE:
          return("NEVER");
     case PLAYSOUND_OPEN:
          return("SECTOR OPEN");
     case PLAYSOUND_CLOSE:
          return("SECTOR CLOSE");
     case PLAYSOUND_STOP:
          return("SECTOR STOP");
     case PLAYSOUND_MOVING:
          return("SECTOR MOVING");
     case PLAYSOUND_ENTER:
          return("ENTER SECTOR");
     case PLAYSOUND_LEAVE:
          return("LEAVE SECTOR");
     }
     return(NULL);
}

void
SND_showSoundPlaytimes(void)
{
     short i;
     char *ptr;

     clearmidstatbar16();
     ENG_setMidStatbarPos(8,32);
     ENG_printHelpText16(11,0,"SOUND PLAY TIMES");
     for (i=PLAYSOUND_NONE ; i < PLAYSOUND_LAST ; i++) {
          if ((ptr=SND_getSoundPlaytimeName(i)) != NULL) {
               ENG_printHelpText16(14,1,"%3d = %s",i,ptr);
          }
     }
}

void
SND_scanSprite(short s)
{
     short sectnum;
     struct sectorCenters *scptr;

     if (spritePtr[s]->statnum >= MAXSTATUS) {
          return;
     }
     if (SND_isAmbientSound(s)) {
          changespritestat(s,STAT_AMBIENT);
          spritePtr[s]->cstat&=~(SPRC_BLOCKING|SPRC_BLOCKINGH);
          spritePtr[s]->cstat|=SPRC_INVISIBLE;
     }
     else if (SND_isSectorSound(s)) {
          sectnum=spritePtr[s]->sectnum;
          scptr=sectorCenterPtr[sectnum];
          setsprite(s,scptr->centerx,scptr->centery,scptr->centerz);
          spritePtr[s]->cstat&=~(SPRC_BLOCKING|SPRC_BLOCKINGH);
          spritePtr[s]->cstat|=SPRC_INVISIBLE;
     }
}

void
SND_scanMap(void)
{
     short i;

     for (i=0 ; i < MAXSPRITES ; i++) {
          SND_scanSprite(i);
     }
}

