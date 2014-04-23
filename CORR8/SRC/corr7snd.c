/***************************************************************************
 *   CORR7SND.C   - sound routines for Corridor 7 using HMI SOS 4.0
 *
 *                                                     08/19/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   DIGI_TIMER_RATE          60
#define   DIGI_SAMPLE_RATE         11025
#define   DIGI_DMA_BUFFER_SIZE     0x1000

#define   MAXSAMPLES               16
#define   SNDBUFSIZE               32768L
#define   MAXSOUNDS                100  // default+user definable sounds

#define   LOWPRIORITY              256
#define   MEDPRIORITY              128
#define   HIPRIORITY               0

int  noSoundFlag,
     stereoSwapFlag;

int  soundVolume=15;

int  nextSample;

struct sndmemptr {
     long cache_ptr;
     long cache_length;
     char cache_lock;
} sndptr[MAXSAMPLES];

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

char configFile[_MAX_PATH],
     soundFilePath[_MAX_PATH];

char *soundDesc[MAXSOUNDS],
     *soundIndex[MAXSOUNDS];

W32  hDIGISample[MAXSAMPLES];

HANDLE hDIGITimer=-1,
     hDIGIDriver;

_SOS_DIGI_DRIVER sDIGIDriver;

_SOS_SAMPLE *pDIGISampleData[MAXSAMPLES],
     *sData;

// pack the structure on a byte alignment
#pragma pack(1)

// .WAV file header structure
typedef struct _tag_wavheader {
     BYTE szRIFF[4];
     DWORD dwFormatLength;
     BYTE szWAVE[4];
     BYTE szFMT[4];
     DWORD dwWaveFormatLength;
     short wFormatTag;
     short wChannels;
     DWORD dwSamplesPerSec;
     DWORD dwAvgBytesPerSec;
     short wBlockAlign;
     short wBitsPerSample;
     BYTE szDATA[4];
     DWORD dwDataLength;
} _WAVHEADER;

extern
long cachecount;

#pragma pack()

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
     W32  wErr;

     if (SND_SOSInitialized() == _FALSE) {
          sosDIGIInitSystem(_NULL,_SOS_DEBUG_NORMAL);
     }
     if (SND_digiInitialized() == _FALSE && !noSoundFlag) {
          EFF_notify(0L,"DETECTING SOUND DEVICE");
          sDIGIDriver.wDriverRate=DIGI_SAMPLE_RATE;
          sDIGIDriver.wDMABufferSize=DIGI_DMA_BUFFER_SIZE;
          if ((wErr=sosDIGIDetectInit(_NULL)) != _ERR_NO_ERROR) {
               crash(sosGetErrorString(wErr));
          }
          if ((wErr=sosDIGIDetectFindFirst(&sDIGIDriver)) != _ERR_NO_ERROR) {
               crash(sosGetErrorString(wErr));
          }
          if ((wErr=sosDIGIDetectGetSettings(&sDIGIDriver)) != _ERR_NO_ERROR) {
               crash(sosGetErrorString(wErr));
          }
          sosDIGIDetectUnInit();
          if ((wErr=sosDIGIInitDriver(&sDIGIDriver,&hDIGIDriver)) != _ERR_NO_ERROR) {
               crash(sosGetErrorString(wErr));
          }
          TMR_registerEvent(DIGI_TIMER_RATE,sDIGIDriver.pfnMixFunction,&hDIGITimer);
          for (i=0 ; i < MAXSAMPLES ; i++) {
               sndptr[i].cache_lock=200;
               sndptr[i].cache_length=SNDBUFSIZE;
               allocache(&(sndptr[i].cache_ptr), sndptr[i].cache_length,
                         &(sndptr[i].cache_lock));
          }
     }
     for (i=0 ; i < MAXSAMPLES ; i++) {
          hDIGISample[i]=-1;
     }
}

void
SND_unInitSOS(void)
{
     if (SND_digiInitialized()) {
          if (hDIGITimer != -1) {
               TMR_removeEvent(hDIGITimer);
          }
          sosDIGIUnInitDriver(hDIGIDriver,_TRUE,_TRUE);
     }
     if (SND_SOSInitialized()) {
          sosDIGIUnInitSystem();
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
          if (access("wavs\\.",F_OK) == 0) {
               strcpy(soundFilePath,"WAVS\\");
          }
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

long
SND_alcmem(long numbytes)
{
     int  i;

     for (i=0 ; i < MAXSAMPLES ; i++) {
          if (sndptr[i].cache_lock == 200) {
               sndptr[i].cache_lock=201;
               return(sndptr[i].cache_ptr);
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
               sndptr[i].cache_lock=200;
               return;
          }
     }
}

_SOS_SAMPLE * cdecl
SND_loadSampleFile(PSTR szName)
{
     W32  hFile=-1;
     W32  wSize;
     PSTR pData;
     _SOS_SAMPLE *pSample;
     _WAVHEADER *pWaveHeader;
     char szPath[_MAX_PATH];

     if (soundFilePath[0] != 0) {
          sprintf(szPath,"%s%s",soundFilePath,szName);
          if ((hFile=open(szPath,O_RDONLY|O_BINARY)) != -1) {
               wSize=lseek(hFile,0,SEEK_END);
               lseek(hFile,0,SEEK_SET);
               if ((pData=(PSTR)SND_alcmem(wSize+sizeof(_SOS_SAMPLE))) == NULL) {
                    close(hFile);
                    return(NULL);
               }
               if (read(hFile,pData+sizeof(_SOS_SAMPLE),wSize) != wSize) {
                    close(hFile);
                    SND_alcfree((long)pData);
                    return(NULL);
               }
               close(hFile);
               cachecount++;
          }
     }
     if (hFile == -1) {
          if ((hFile=kopen4load(szName,0)) == -1) {
               return(_NULL);
          }
          wSize=klseek(hFile,0,SEEK_END);
          klseek(hFile,0,SEEK_SET);
          if ((pData=(PSTR)SND_alcmem(wSize+sizeof(_SOS_SAMPLE))) == NULL) {
               kclose(hFile);
               return(NULL);
          }
          if (kread(hFile,pData+sizeof(_SOS_SAMPLE),wSize) != wSize) {
               kclose(hFile);
               SND_alcfree((long)pData);
               return(NULL);
          }
          kclose(hFile);
          cachecount++;
     }
     memset(pData,0,sizeof(_SOS_SAMPLE));
     pSample=(_SOS_SAMPLE *)pData;
     if (!strncmp(pData+sizeof(_SOS_SAMPLE),"RIFF",0x04)) {
          pWaveHeader=(_WAVHEADER *)(pData+sizeof(_SOS_SAMPLE));
          pSample->pSample=(PSTR)pData+sizeof(_SOS_SAMPLE)+sizeof(_WAVHEADER);
          pSample->wLength=pWaveHeader->dwDataLength-sizeof(_WAVHEADER);
          pSample->wBitsPerSample=pWaveHeader->wBitsPerSample;
          pSample->wChannels=pWaveHeader->wChannels;
          if (pWaveHeader->wBitsPerSample == 0x08) {
               pSample->wFormat=_PCM_UNSIGNED;
          }
          else {
               pSample->wFormat=0x00;
          }
          pSample->wRate=pWaveHeader->dwSamplesPerSec;
     }
     else {
          pSample->pSample=(PSTR)pData+sizeof(_SOS_SAMPLE);
          pSample->wLength=(DWORD)wSize;
          pSample->wBitsPerSample=0x08;
          pSample->wChannels=0x01;
          pSample->wFormat=_PCM_UNSIGNED;
          pSample->wRate=11025;
     }
     return(pSample);
}

void cdecl
SND_digiDoneCallback(_SOS_SAMPLE *sData)
{
     int  index;

     for (index=0 ; index < MAXSAMPLES ; index++) {
          if (hDIGISample[index] == sData->hSample) {
               break;
          }
     }
     if (index < MAXSAMPLES) {
          hDIGISample[index]=-1;
     }
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
#if 0
     char soundPath[_MAX_PATH];
#endif

     if (!SND_digiInitialized()) {
          return(-1);
     }
     if (SND_getNextSample() == -1) {
          return(-1);
     }
     SND_alcfree((long)pDIGISampleData[nextSample]);
#if 0
     sprintf(soundPath,"wavs\\%s",wavFile);
     sData=SND_loadSampleFile(soundPath);
#endif
     sData=SND_loadSampleFile(wavFile);
     if (sData == NULL) {
          return(-1);
     }
     sData->pfnSampleDone=(VOID *)&SND_digiDoneCallback;
     if (wVol > (soundVolume<<11)) {
          wVol=soundVolume<<11;
     }
     sData->wVolume=MK_VOLUME(wVol,wVol);
     sData->wPanPosition=_PAN_CENTER;
     sData->wLoopCount=loopCount;
     sData->wID=owner;
     sData->wPriority=pri;
     pDIGISampleData[nextSample]=sData;
     hDIGISample[nextSample]=sosDIGIStartSample(hDIGIDriver,sData);
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

     if (!SND_digiInitialized()) {
          return;
     }
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
     sosDIGISetPanLocation(hDIGIDriver,hSample,wPan);
     if (wVol > (soundVolume<<11)) {
          wVol=soundVolume<<11;
     }
     if (wVol < 0x400) {
          wVol=0x400;
     }
     sosDIGISetSampleVolume(hDIGIDriver,hSample,wVol);
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

     if (!SND_digiInitialized()) {
          return;
     }
     hSample=sosDIGIGetSampleHandle(hDIGIDriver,owner);
     sosDIGIStopSample(hDIGIDriver,hSample);
}

W32
SND_soundPlaying(W32 owner)
{
     W32  hSample;

     if (!SND_digiInitialized()) {
          return(-1);
     }
     if (spritePtr[owner]->owner == 1) {
          return(sosDIGIGetSampleHandle(hDIGIDriver,owner));
     }
     return(-1);
}

void
SND_stopAllSounds(void)
{
     W32  hSample,i;

     if (!SND_digiInitialized()) {
          return;
     }
     for (i=0 ; i < MAXSAMPLES ; i++) {
          if ((hSample=hDIGISample[i]) >= 0) {
               if (sosDIGISampleDone(hDIGIDriver,hSample) == _FALSE) {
                    sosDIGIStopSample(hDIGIDriver,hSample);
               }
          }
     }
}

void
SND_debugSoundHandles(void)
{
#ifdef ANALYZESOUNDINDEXDEBUG
     int  i;

     strcpy(tempbuf,"SOUNDX: ");
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
SND_debugSoundIDs(void)
{
     int  i,id;
     char idchar[8];

     strcpy(tempbuf,"SOUNDID: ");
     for (i=0 ; i < MAXSAMPLES ; i++) {
          if (hDIGISample[i] != -1) {
               id=sosDIGIGetSampleID(hDIGIDriver,hDIGISample[i]);
               sprintf(idchar,"%d ",id);
               strcat(tempbuf,idchar);
          }
          else {
               strcat(tempbuf,"N ");
          }
     }
     debugOut(windowx1,windowy1,tempbuf);
}

void
SND_debug(void)
{
     SND_debugSoundHandles();
     SND_debugMemoryPtrs();
     SND_debugSoundIDs();
}

//**
//** game sound routines
//**

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
SND_getSoundNum(spritetype *spr)
{
     return(spr->lotag);
}

void
SND_setSoundNum(spritetype *spr,short n)
{
     spr->lotag=n;
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
          sound=SND_getSoundNum(spr2);
          hSample=SND_soundPlaying(i);
          if (WEP_getDistance(spr2,spr) < 10000L) {
               if (hSample == -1) {
                    SND_playSampleIndex(i,LOWPRIORITY,sound,-1);
                    spr2->owner=1;
               }
               else {
                    x=spr2->x;
                    y=spr2->y;
                    SND_setSampleLocation(x,y,s,hSample);
               }
               continue;
          }
          if (hSample != -1) {
               SND_stopSampleIndex(i);
               spr2->owner=0;
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
                    sound=SND_getSoundNum(spritePtr[i]);
                    hSample=SND_soundPlaying(i);
                    if (playflag) {
                         if (hSample == -1) {
                              SND_playSampleIndex(i,LOWPRIORITY,sound,-1);
                              spritePtr[i]->owner=1;
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
                         spritePtr[i]->owner=0;
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
                    sound=SND_getSoundNum(spritePtr[i]);
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
                    sound=SND_getSoundNum(spritePtr[i]);
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
                    sound=SND_getSoundNum(spritePtr[i]);
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
                    sound=SND_getSoundNum(spritePtr[i]);
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
                    sound=SND_getSoundNum(spritePtr[i]);
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
          spritePtr[s]->owner=0;
     }
     else if (SND_isSectorSound(s)) {
          sectnum=spritePtr[s]->sectnum;
          scptr=sectorCenterPtr[sectnum];
          setsprite(s,scptr->centerx,scptr->centery,scptr->centerz);
          spritePtr[s]->cstat&=~(SPRC_BLOCKING|SPRC_BLOCKINGH);
          spritePtr[s]->cstat|=SPRC_INVISIBLE;
          spritePtr[s]->owner=0;
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

