/***************************************************************************
 *   CORR7GAM.C - GAME related functions for Corridor 7
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   DEFAULTAMMO    25

int  gravityScale=4,
     jumpScale=8;

int  cheatUnlimitedAmmo=1,
     cheatGodMode;

int  bypassLevelFlag,
     dumpDebugInfoFlag,
     gameInitializedFlag,
     inGameFlag,
     loadLevelFlag,
     newGameFlag,
     newMapLoadedFlag,
     playingGameFlag;

int  currentMap=1,
     goreLevel=MAXGORECHOICES-1;

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

//** reset when advancing levels

short aliensLeft,
     madeNoise;

short forceAng[MAXSPRITES],
     forceVel[MAXSPRITES],
     horizAdj[MAXSPRITES];

long horizVel[MAXSPRITES],
     jumpVel[MAXSPRITES];

char onGround[MAXSPRITES>>3];

//**

short starta,
     starts;

long startx,
     starty,
     startz;

long damageHeight,
     jumpVelocity=16,
     gravityConstant=32,
     defaultPlayerWidth=64,
     defaultPlayerHeight=64;

long loadpos;

long gameWindowx1,
     gameWindowx2,
     gameWindowy1,
     gameWindowy2;

char mapFileName[_MAX_PATH];

FILE *dbgfp;

enum {
     DEMO_TITLESCREEN,
     DEMO_GAMECREDITS,
     DEMO_GAMEPLAYBACK
};

enum {
     GAME_ALLIANCE,
     GAME_MISSION,
     GAME_SKILL,
     GAME_ACTOR,
     GAME_PLAY
};

void
GAM_init(void)
{
}

void
GAM_unInit(void)
{
}

W32
GAM_findPicName(_INI_INSTANCE *ins,char *name)
{
     W32  wTemp;

     hmiINIGetItemString(ins,name,tempbuf,128);
     if (strlen(tempbuf) == 0) {
          crash("GAM_findPicName: %s LABEL NOT FOUND IN CORR7.INI",name);
     }
     if (!WEP_findPicName(tempbuf,&wTemp)) {
          crash("GAM_findPicName: %s NOT FOUND IN NAMES.H",tempbuf);
     }
     return(wTemp);
}

void
GAM_initUniverse(void)
{
     int  i;
     char locbuf[40];
     W32  wTemp;
     _INI_INSTANCE weapIns;

     MEN_initMenus();
     for (i=0 ; i < MAXWEAPONS ; i++) {
          weaponPtr[i]=&weaponParms[i];
     }
     if (!hmiINIOpen(&weapIns,GAMEINIFILE)) {
          crash("GAM_initUniverse: %s FILE NOT FOUND!",GAMEINIFILE);
     }
     else {
          for (i=0 ; i < MAXWEAPONS ; i++) {
               memset(weaponPtr[i],0,sizeof(struct weaponData));
               sprintf(tempbuf,"WEAPON%d",i+1);
               if (!hmiINILocateSection(&weapIns,tempbuf)) {
                    continue;
               }
               hmiINIGetItemString(&weapIns,"WEAPNAME",tempbuf,128);
               strupr(tempbuf);
               weaponPtr[i]->name=strdup(tempbuf);
               hmiINIGetItemString(&weapIns,"TYPE",tempbuf,128);
               if (stricmp(tempbuf,"MISSILE") == 0) {
                    wTemp=WEAPON_MISSILE;
               }
               else if (stricmp(tempbuf,"MINE") == 0) {
                    wTemp=WEAPON_MINE;
               }
               else if (stricmp(tempbuf,"ENERGY") == 0) {
                    wTemp=WEAPON_ENERGY;
               }
               else if (stricmp(tempbuf,"SHELL") == 0) {
                    wTemp=WEAPON_SHELL;
               }
               else {
                    wTemp=WEAPON_BULLET;
               }
               weaponPtr[i]->registered=wTemp;
               hmiINIGetItemString(&weapIns,"GUIDANCE",tempbuf,128);
               if (stricmp(tempbuf,"HEATSEEKER") == 0) {
                    wTemp=GUIDANCE_HEATSEEKER;
               }
               else if (stricmp(tempbuf,"CRUISE") == 0) {
                    wTemp=GUIDANCE_CRUISE;
               }
               else {
                    wTemp=GUIDANCE_DUMB;
               }
               weaponPtr[i]->guidance=wTemp;
               hmiINIGetItemDecimal(&weapIns,"TRACKERFREQ",&wTemp);
               if (wTemp > 0 && wTemp <= 10) {
                    weaponPtr[i]->trackerFreq=TMR_getSecondTics(1)/wTemp;
               }
               else {
                    weaponPtr[i]->trackerFreq=TMR_getSecondFraction(2);
               }
               hmiINIGetItemString(&weapIns,"DETONATION",tempbuf,128);
               if (stricmp(tempbuf,"PROXIMITY") == 0) {
                    wTemp=STAT_PROXIMITY;
               }
               else {
                    wTemp=STAT_PROJECTILE;
               }
               weaponPtr[i]->detonation=wTemp;
               if (wTemp == STAT_PROXIMITY) {
                    hmiINIGetItemDecimal(&weapIns,"DETRANGE",&wTemp);
                    weaponPtr[i]->detrange=wTemp;
               }
               wTemp=0;
               hmiINIGetItemDecimal(&weapIns,"FIRERATE",&wTemp);
               if (wTemp > 0 && wTemp <= 10) {
                    wTemp=TMR_getSecondTics(1)/(wTemp<<1);
               }
               weaponPtr[i]->ticDelay=wTemp;
               wTemp=0;
               hmiINIGetItemDecimal(&weapIns,"ANIMRATE",&wTemp);
               if (wTemp > 0 && wTemp <= 10) {
                    wTemp=TMR_getSecondTics(1)/(wTemp<<1);
               }
               weaponPtr[i]->animDelay=wTemp;
               wTemp=0;
               hmiINIGetItemDecimal(&weapIns,"PROJECTILES",&wTemp);
               if (wTemp <= 0) {
                    wTemp=1;
               }
               weaponPtr[i]->projectiles=wTemp;
               hmiINIGetItemString(&weapIns,"DISPERSETYPE",tempbuf,128);
               if (stricmp(tempbuf,"FIXED") == 0) {
                    weaponPtr[i]->disperseType=DISPERSE_FIXED;
               }
               else {
                    weaponPtr[i]->disperseType=DISPERSE_RANDOM;
               }
               wTemp=0;
               if (hmiINIGetItemDecimal(&weapIns,"DISPERSEMENT",&wTemp)) {
                    if (wTemp >= 360) {
                         wTemp=2048;
                    }
                    else {
                         wTemp=((long)wTemp*568L)/100L;
                    }
                    wTemp>>=1;
               }
               weaponPtr[i]->dispersement=wTemp;
               hmiINIGetItemString(&weapIns,"PROJPIC",tempbuf,128);
               if (strlen(tempbuf) > 0 && WEP_findPicName(tempbuf,&wTemp)) {
                    if (weaponPtr[i]->registered == WEAPON_BULLET) {
                         weaponPtr[i]->registered=WEAPON_MISSILE;
                    }
                    weaponPtr[i]->projectilePic=wTemp;
                    if (hmiINIGetItemDecimal(&weapIns,"PROJXSIZE",&wTemp)) {
                         weaponPtr[i]->projectileXSize=wTemp;
                    }
                    else {
                         weaponPtr[i]->projectileXSize=64;
                    }
                    if (hmiINIGetItemDecimal(&weapIns,"PROJYSIZE",&wTemp)) {
                         weaponPtr[i]->projectileYSize=wTemp;
                    }
                    else {
                         weaponPtr[i]->projectileYSize=64;
                    }
                    hmiINIGetItemString(&weapIns,"PROJFACE",tempbuf,128);
                    wTemp=PROJNORMAL;
                    if (strlen(tempbuf) > 0) {
                         if (stricmp(tempbuf,"XFLAT") == 0) {
                              wTemp=PROJXFLAT;
                         }
                         else if (stricmp(tempbuf,"YFLAT") == 0) {
                              wTemp=PROJYFLAT;
                         }
                    }
                    weaponPtr[i]->projectileFace=wTemp;
                    wTemp=0;
                    hmiINIGetItemDecimal(&weapIns,"PROJSPEED",&wTemp);
                    if (wTemp > 0 && wTemp <= 10) {
                         weaponPtr[i]->projectileSpeed=10-wTemp;
                    }
                    else {
                         weaponPtr[i]->projectileSpeed=0;
                    }
                    if (hmiINIGetItemDecimal(&weapIns,"TRACKERTURN",&wTemp)) {
                         weaponPtr[i]->projectileTurnRate=(1<<wTemp);
                    }
                    if (hmiINIGetItemDecimal(&weapIns,"PROJVIEWS",&wTemp)) {
                         weaponPtr[i]->projectileViews=wTemp;
                    }
                    else {
                         weaponPtr[i]->projectileViews=1;
                    }
                    if (hmiINIGetItemDecimal(&weapIns,"TRACKERFOV",&wTemp)) {
                         weaponPtr[i]->projectileFOV=((long)wTemp*568L)/100L;
                         if (wTemp == 360) {
                              weaponPtr[i]->projectileFOV=2048;
                         }
                    }
                    else {
                         weaponPtr[i]->projectileFOV=2048;
                    }
                    hmiINIGetItemString(&weapIns,"SAMPLE",tempbuf,128);
                    weaponPtr[i]->sample=strdup(tempbuf);
               }
               else {
                    weaponPtr[i]->registered=WEAPON_BULLET;
                    weaponPtr[i]->hitscanFlag=1;
                    hmiINIGetItemString(&weapIns,"SAMPLE",tempbuf,128);
                    weaponPtr[i]->sample=strdup(tempbuf);
               }
               if (hmiINIGetQuery(&weapIns,"GRAVADJ")) {
                    weaponPtr[i]->gravityFlag=1;
               }
               if (hmiINIGetQuery(&weapIns,"WINDADJ")) {
                    weaponPtr[i]->windFlag=1;
               }
               if (hmiINIGetItemDecimal(&weapIns,"MINDAMAGE",&wTemp)) {
                    weaponPtr[i]->minDamage=wTemp;
               }
               if (hmiINIGetItemDecimal(&weapIns,"MAXDAMAGE",&wTemp)) {
                    weaponPtr[i]->maxDamage=wTemp;
               }
               if (hmiINIGetItemDecimal(&weapIns,"DEFAMMO",&wTemp)) {
                    weaponPtr[i]->defaultAmmo=wTemp;
               }
               else {
                    weaponPtr[i]->defaultAmmo=DEFAULTAMMO;
               }
               if (hmiINIGetQuery(&weapIns,"AUTOCENTER")) {
                    weaponPtr[i]->autoCenterFlag=1;
               }
               weaponPtr[i]->holdWeaponStartPic=GAM_findPicName(&weapIns,
                                                                "HOLDPIC1");
#if 0
               weaponPtr[i]->holdWeaponStopPic=GAM_findPicName(&weapIns,
                                                               "HOLDPIC2");
#endif
               weaponPtr[i]->fireWeaponStartPic=GAM_findPicName(&weapIns,
                                                                "FIREPIC1");
               weaponPtr[i]->fireWeaponStopPic=GAM_findPicName(&weapIns,
                                                               "FIREPIC2");
               weaponPtr[i]->fireFrames=weaponPtr[i]->fireWeaponStopPic-
                                        weaponPtr[i]->fireWeaponStartPic;
               if (weaponPtr[i]->fireFrames <= 0) {
                    weaponPtr[i]->fireFrames=1;
               }
               if (hmiINIGetItemDecimal(&weapIns,"FIREFRAME",&wTemp)) {
                    weaponPtr[i]->weaponShootFrame=wTemp;
               }
               if (weaponPtr[i]->weaponShootFrame == 0) {
                    weaponPtr[i]->weaponShootFrame=1;
               }
               if (hmiINIGetItemDecimal(&weapIns,"EXPLORANGE",&wTemp)) {
                    weaponPtr[i]->range=wTemp;
               }
               hmiINIGetItemString(&weapIns,"EXPLOTYPE",tempbuf,128);
               wTemp=EXPLOSION_NORMAL;
               if (strlen(tempbuf) > 0) {
                    if (stricmp(tempbuf,"SHOCKWAVE") == 0) {
                         wTemp=EXPLOSION_SHOCKWAVE;
                    }
               }
               weaponPtr[i]->explosionType=wTemp;
               weaponPtr[i]->explosionStartPic=GAM_findPicName(&weapIns,
                                                               "EXPLOPIC1");
               weaponPtr[i]->explosionStopPic=GAM_findPicName(&weapIns,
                                                              "EXPLOPIC2");
               if (hmiINIGetItemDecimal(&weapIns,"EXPLOSIZE",&wTemp)) {
                    weaponPtr[i]->explosionSize=wTemp;
               }
               else {
                    weaponPtr[i]->explosionSize=64;
               }
               wTemp=0;
               hmiINIGetItemDecimal(&weapIns,"EXPLORATE",&wTemp);
               if (wTemp > 0 && wTemp <= 10) {
                    wTemp=TMR_getSecondTics(1)/(wTemp<<1);
               }
               else {
                    wTemp=TMR_getSecondFraction(16);
               }
               weaponPtr[i]->explosionRate=wTemp;
          }
          if (hmiINILocateSection(&weapIns,"ENVIRONMENT")) {
               if (hmiINIGetItemDecimal(&weapIns,"GRAVITY",&wTemp)) {
                    gravityConstant=wTemp;
               }
               if (hmiINIGetItemDecimal(&weapIns,"GRAVITYSCALE",&wTemp)) {
                    gravityScale=wTemp;
               }
               if (hmiINIGetItemDecimal(&weapIns,"JUMPVELOCITY",&wTemp)) {
                    jumpVelocity=wTemp;
               }
               if (hmiINIGetItemDecimal(&weapIns,"JUMPSCALE",&wTemp)) {
                    jumpScale=wTemp;
               }
          }
          else {
               crash("GAM_initUniverse: NO ENVIRONMENT SECTION IN CORR7.INI");
          }
          if (hmiINILocateSection(&weapIns,"PLAYER")) {
               if (hmiINIGetItemDecimal(&weapIns,"PLAYERWIDTH",&wTemp)) {
                    defaultPlayerWidth=wTemp;
               }
               if (hmiINIGetItemDecimal(&weapIns,"PLAYERHEIGHT",&wTemp)) {
                    defaultPlayerHeight=wTemp;
               }
               if (hmiINIGetItemDecimal(&weapIns,"PLAYERMAXHEALTH",&wTemp)) {
                    playerMaxHealth=wTemp;
               }
          }
          else {
               crash("GAM_initUniverse: NO PLAYER SECTION IN CORR7.INI");
          }
          if (hmiINILocateSection(&weapIns,"ACTORS")) {
               memset(enemyPic,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyPicAngles,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyFirePic1,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyFirePic2,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyFireAngles,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyPainPic,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyPainAngles,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyDiePic1,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyDiePic2,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyGorePic1,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyGorePic2,0,sizeof(short)*MAXDEFINEDACTORS);
               memset(enemyDieAngles,0,sizeof(short)*MAXDEFINEDACTORS);
               for (i=0 ; i < MAXDEFINEDACTORS ; i++) {
                    sprintf(locbuf,"ACTOR%dPICANGLES",i+1);
                    if (hmiINIGetItemDecimal(&weapIns,locbuf,&wTemp)) {
                         enemyPicAngles[i]=wTemp;
                    }
                    sprintf(locbuf,"ACTOR%dPIC",i+1);
                    enemyPic[i]=GAM_findPicName(&weapIns,locbuf);
                    sprintf(locbuf,"ACTOR%dFIREANGLES",i+1);
                    if (hmiINIGetItemDecimal(&weapIns,locbuf,&wTemp)) {
                         enemyFireAngles[i]=wTemp;
                    }
                    sprintf(locbuf,"ACTOR%dFIREPIC1",i+1);
                    enemyFirePic1[i]=GAM_findPicName(&weapIns,locbuf);
                    sprintf(locbuf,"ACTOR%dFIREPIC2",i+1);
                    enemyFirePic2[i]=GAM_findPicName(&weapIns,locbuf);
                    sprintf(locbuf,"ACTOR%dPAINANGLES",i+1);
                    if (hmiINIGetItemDecimal(&weapIns,locbuf,&wTemp)) {
                         enemyPainAngles[i]=wTemp;
                    }
                    sprintf(locbuf,"ACTOR%dPAINPIC",i+1);
                    enemyPainPic[i]=GAM_findPicName(&weapIns,locbuf);
                    sprintf(locbuf,"ACTOR%dDIEANGLES",i+1);
                    if (hmiINIGetItemDecimal(&weapIns,locbuf,&wTemp)) {
                         enemyDieAngles[i]=wTemp;
                    }
                    sprintf(locbuf,"ACTOR%dDIEPIC1",i+1);
                    enemyDiePic1[i]=GAM_findPicName(&weapIns,locbuf);
                    sprintf(locbuf,"ACTOR%dDIEPIC2",i+1);
                    enemyDiePic2[i]=GAM_findPicName(&weapIns,locbuf);
                    sprintf(locbuf,"ACTOR%dGOREPIC1",i+1);
                    enemyGorePic1[i]=GAM_findPicName(&weapIns,locbuf);
                    sprintf(locbuf,"ACTOR%dGOREPIC2",i+1);
                    enemyGorePic2[i]=GAM_findPicName(&weapIns,locbuf);
               }
          }
          if (hmiINILocateSection(&weapIns,"FONTS")) {
               graphicLetAPic=GAM_findPicName(&weapIns,"GRAPHICLETTERA");
               graphicNum0Pic=GAM_findPicName(&weapIns,"GRAPHICNUMBER0");
               graphicBigNum0Pic=GAM_findPicName(&weapIns,"GRAPHICBIGNUM0");
          }
          else {
               crash("GAM_initUniverse: NO FONTS SECTION IN CORR7.INI");
          }
          if (hmiINILocateSection(&weapIns,"MENUS")) {
               menuMarkerBegPic=GAM_findPicName(&weapIns,"MENUMARKERBEG");
               menuMarkerEndPic=GAM_findPicName(&weapIns,"MENUMARKEREND");
               menuBackgroundPic=GAM_findPicName(&weapIns,"MENUBACKGROUND");
               menuTitleBarPic=GAM_findPicName(&weapIns,"MENUTITLEBAR");
               menuSliderBarPic=GAM_findPicName(&weapIns,"SLIDERBAR");
               menuSliderKnobPic=GAM_findPicName(&weapIns,"SLIDERKNOB");
          }
          else {
               crash("GAM_initUniverse: NO MENUS SECTION IN CORR7.INI");
          }
          if (hmiINILocateSection(&weapIns,"MISCGRAPHICS")) {
               titleScreenPic=GAM_findPicName(&weapIns,"TITLESCREEN");
               backgroundPic=GAM_findPicName(&weapIns,"BACKGROUND");
               statusBarPic=GAM_findPicName(&weapIns,"STATUSBAR");
               goreChunksBegPic=GAM_findPicName(&weapIns,"GORECHUNKSBEG");
               goreChunksEndPic=GAM_findPicName(&weapIns,"GORECHUNKSEND");
          }
          else {
               crash("GAM_initUniverse: NO MISCGRAPHICS SECTION IN CORR7.INI");
          }
          gravityConstant<<=gravityScale;
          jumpVelocity<<=jumpScale;
          damageHeight=defaultPlayerHeight<<1;
          hmiINIClose(&weapIns);
          ACT_buildPicBitArray();
          SND_initSounds(GAMEINIFILE);
     }
}

void
GAM_unInitPointers(void)
{
     int  i;

     for (i=0 ; i < MAXWEAPONS ; i++) {
          if (weaponPtr[i]->name != NULL) {
               free(weaponPtr[i]->name);
          }
          if (weaponPtr[i]->sample != NULL) {
               free(weaponPtr[i]->sample);
          }
     }
}

void
GAM_setGoreLevel(int n)
{
     goreLevel=n;
}

int
GAM_getGoreLevel(void)
{
     return(goreLevel);
}

void
GAM_adjAliensLeft(short n)
{
     aliensLeft+=n;
}

void
GAM_setAliensLeft(short n)
{
     aliensLeft=n;
}

short
GAM_getAliensLeft(void)
{
     return(aliensLeft);
}

void
GAM_setMadeNoise(short s)
{
     madeNoise=s;
}

short
GAM_getMadeNoise(void)
{
     return(-1);
#if 0
     return(madeNoise);
#endif
}

void
GAM_updateStatusBar(void)
{
     if (qsetmode != 200L) {
          return;
     }
     if (viewSize <= xdim) {
          rotatesprite(xdim<<15,(ydim-(tilesizy[statusBarPic]>>1))<<16,
                       65536L,0,statusBarPic,0,0,8+64+128,
                       0L,0L,xdim-1L,ydim-1L);
          PLR_updateStatusBar();
     }
}

int
GAM_inSetupVideoMode(void)
{
     if (windowx1 != gameWindowx1 || windowy1 != gameWindowy1 ||
         windowx2 != gameWindowx2 || windowy2 != gameWindowy2) {
          return(0);
     }
     return(1);
}

void
GAM_setupVideo(int vsize)
{
     int  x1,y1,x2,y2;

     viewSize=vsize;
     ENG_setupVideo();
     if (viewSize == 0) {
          viewSize=xdim;
     }
     else if (viewSize < 64) {
          viewSize=64;
     }
     if (viewSize > xdim) {
          x1=0;
          y1=0;
          x2=xdim-1;
          y2=ydim-1;
     }
     else {
          if (viewSize < xdim) {
               if (gotpic[backgroundPic>>3]&(1<<(backgroundPic&7)) == 0) {
                    loadtile(backgroundPic);
               }
               rotatesprite(320L<<15,200L<<15,65536L,0,
                            backgroundPic,0,0,2+8+64+128,
                            0L,0L,xdim-1L,ydim-1L);
          }
          if (gotpic[statusBarPic>>3]&(1<<(statusBarPic&7)) == 0) {
               loadtile(statusBarPic);
          }
          x1=((xdim-viewSize)>>1);
          x2=x1+viewSize-1;
          y1=(((ydim-tilesizy[statusBarPic])-scale(viewSize,
                                                 ydim-tilesizy[statusBarPic],
                                                 xdim))>>1);
          y2=y1+scale(viewSize,ydim-tilesizy[statusBarPic],xdim)-1;
          GAM_updateStatusBar();
     }
     setview(x1,y1,x2,y2);
     gameWindowx1=windowx1;
     gameWindowy1=windowy1;
     gameWindowx2=windowx2;
     gameWindowy2=windowy2;
     clearview(0L);
}

void
GAM_scanSprite(short i)
{
     spritetype *spr;

     spr=spritePtr[i];
     if (EFF_testSpriteFlag(i,EXTFLAGS_HEATSOURCE)) {
          WEP_addHeatSource(i);
     }
     if (spr->z >= sectorPtr[spr->sectnum]->floorz) {
          GAM_onGround(i,1);
     }
     else {
          GAM_onGround(i,0);
     }
}

void
GAM_scanMap(void)
{
     short i;

     EFF_computeSectorCenters();
     ENG_checksumReset();
     GAM_setAliensLeft(0);
     EFF_notify(0L,"INITIALIZING SPRITES");
     WEP_resetHeatSourceIndex();
     for (i=0 ; i < MAXSPRITES ; i++) {
          if (spritePtr[i]->statnum >= MAXSTATUS) {
               continue;
          }
          ENG_checksumSprite(i);
          if (!newMapLoadedFlag) {
               if (EFF_testSpriteFlag(i,31) == 0) {
                    EFF_setSpriteFlag(i,31);
               }
               else {
                    switch (curSkill) {
                    case SKILL_CORPORAL:
                    case SKILL_LIEUTENANT:
                         if (EFF_testSpriteFlag(i,3) == 0) {
                              ENG_deletesprite(i);
                              continue;
                         }
                         break;
                    case SKILL_CAPTAIN:
                         if (EFF_testSpriteFlag(i,4) == 0) {
                              ENG_deletesprite(i);
                              continue;
                         }
                         break;
                    case SKILL_MAJOR:
                    case SKILL_PRESIDENT:
                         if (EFF_testSpriteFlag(i,24) == 0) {
                              ENG_deletesprite(i);
                              continue;
                         }
                         break;
                    }
               }
          }
          GAM_scanSprite(i);
     }
     if (!newMapLoadedFlag) {
          ACT_scanMap();
          EFF_scanMap();
          MUL_scanMap();
          SND_scanMap();
          WEP_scanMap();
     }
     EFF_buildHitagIndex();
}

void
GAM_onGround(short s,short flag)
{
     if (flag) {
          onGround[s>>3]|=(1<<(s&7));
     }
     else {
          onGround[s>>3]&=~(1<<(s&7));
     }
}

int
GAM_isOnGround(short s)
{
     if ((onGround[s>>3]&(1<<(s&7))) != 0) {
          return(1);
     }
     return(0);
}

short
GAM_getViewSprite(void)
{
     return(PLR_getPlayerSprite(currentView));
}

int
GAM_isViewSprite(short s)
{
     if (s == PLR_getPlayerSprite(currentView)) {
          return(1);
     }
     return(0);
}

void
GAM_loadTextures(void)
{
     short endwall,i,j,startwall;

     EFF_notify(0L,"LOADING TEXTURES");
     for (i=0 ; i < numsectors ; i++) {
          if (waloff[sectorPtr[i]->ceilingpicnum] == 0) {
               loadtile(sectorPtr[i]->ceilingpicnum);
          }
          if (waloff[sectorPtr[i]->floorpicnum] == 0) {
               loadtile(sectorPtr[i]->floorpicnum);
          }
          startwall=sectorPtr[i]->wallptr;
          endwall=startwall+sectorPtr[i]->wallnum;
          for (j=startwall ; j < endwall ; j++) {
               if (waloff[wallPtr[j]->picnum] == 0) {
                    loadtile(wallPtr[j]->picnum);
               }
          }
     }
}

void
GAM_loadLevel(int mapNumber)
{
     if (bypassLevelFlag == 0 && strlen(mapFileName) == 0) {
          sprintf(mapFileName,"ABLEV%d.MAP",mapNumber);
     }
     if (loadboard(mapFileName,&startx,&starty,&startz,
                   &starta,&starts) == -1) {
          crash("Map %s not found",mapFileName);
     }
     EFF_resetDamageClock();
     GAM_loadTextures();
     ENG_init2DMap();
     GAM_scanMap();
     PLR_initPlayerSprites(waitplayers);
     fakeClock=totalclock;
     loadLevelFlag=0;
}

int
GAM_demoTitleScreen(void)
{
     long k=0,lastClock,waitClock,zoom;

     SMK_playFlic("logo.smk");
     zoom=256;
     KBD_resetKeys();
     lastClock=totalclock;
     waitClock=lastClock+TMR_getSecondTics(4);
     while (!(k=KBD_keyPressed()) && totalclock < waitClock) {
          if (zoom < 65536) {
               zoom+=((totalclock-lastClock)<<8);
               lastClock=totalclock;
               if (zoom > 65536) {
                    zoom=65536;
               }
          }
          clearview(0L);
          rotatesprite(320L<<15, 200L<<15, zoom, 0, titleScreenPic, 0, 0, 2,
                       0L, 0L, xdim-1L, ydim-1L);
          nextpage();
          GFX_fadeIn(8);
     }
     if (k) {
          return(k);
     }
     return(0);
}

int
GAM_demoGameCredits(void)
{
     long k=0,waitClock;

     ENG_setupVideo();             // sets up full screen view window
     GFX_waitFadeOut(8);
     KBD_resetKeys();
     waitClock=totalclock+TMR_getSecondTics(4);
     while (!(k=KBD_keyPressed()) && totalclock < waitClock) {
          rotatesprite(320L<<15,200L<<15,65536L,0,titleScreenPic,0,0,2,
                       0L,0L,xdim-1L,ydim-1L);
          nextpage();
          GFX_fadeIn(8);
     }
     GFX_waitFadeOut(8);
     if (k) {
          return(k);
     }
     return(0);
}

int
GAM_demoGame(void)
{
     GFX_waitFadeOut(8);
     return(0);
}

int                                // return 1 to play game, 0 exit program
GAM_demoLoop(void)
{
     int  demoState=0,r;

     GFX_fadeOut(255);
     if (gameType != GAMETYPE_SINGLE || noWaitFlag) {
          return(1);
     }
     while (1) {
          switch (demoState) {
          case DEMO_TITLESCREEN:        // showing title screen
               r=GAM_demoTitleScreen();
               break;
          case DEMO_GAMECREDITS:        // showing game credits
               r=GAM_demoGameCredits();
               break;
          case DEMO_GAMEPLAYBACK:       // play recorded game
               r=GAM_demoGame();
               break;
          default:
               demoState=DEMO_TITLESCREEN;
               continue;
               break;
          }
          if (r) {
               return(1);
          }
          demoState++;
     }
}

int
GAM_orderScreen(void)
{
     long k=0,waitClock;

     ENG_setupVideo();             // sets up full screen view window
     GFX_waitFadeOut(8);
     KBD_resetKeys();
     waitClock=totalclock+TMR_getSecondTics(15);
     while (!(k=KBD_keyPressed()) && totalclock < waitClock) {
          rotatesprite(320L<<15,200L<<15,65536L,0,titleScreenPic,0,0,2,
                       0L,0L,xdim-1L,ydim-1L);
          nextpage();
          GFX_fadeIn(8);
     }
     GFX_waitFadeOut(8);
     if (k) {
          return(k);
     }
     return(0);
}

int
GAM_helpScreen(void)
{
#if 0
     long k=0,waitClock;

     ENG_setupVideo();             // sets up full screen view window
     GFX_waitFadeOut(8);
     KBD_resetKeys();
     waitClock=totalclock+TMR_getSecondTics(15);
     rotatesprite(320L<<15,200L<<15,65536L,0,helpScreenPic,0,0,2,
                  0L,0L,xdim-1L,ydim-1L);
     nextpage();
     while (!(k=KBD_keyPressed()) && totalclock < waitClock) {
          GFX_fadeIn(8);
     }
     GFX_waitFadeOut(8);
     if (k) {
          return(k);
     }
#endif
     return(0);
}

void
GAM_checkCheats(void)
{
     while (keyfifoplc != keyfifoend) {
          if (keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)] == 1) {
               switch (keyfifo[keyfifoplc]) {
               case K_G:
                    cheatGodMode^=1;
                    EFF_displayMessage("GOD MODE %s",
                                       cheatGodMode ? "ON" : "OFF");
                    break;
               case K_O:
                    cheatUnlimitedAmmo^=1;
                    EFF_displayMessage("UNLIMITED AMMO %s",
                                       cheatUnlimitedAmmo ? "ON" : "OFF");
                    break;
               }
          }
          keyfifoplc=(keyfifoplc+2)&(KEYFIFOSIZ-1);
     }
}

void
GAM_moveThings(void)
{
     GAM_doStatusCode();
     PLR_doActors();
}

void
GAM_doMoveThings(void)
{
#if 0
     int  i,s;

     if (dumpDebugInfoFlag) {
          for (i=0 ; i < numplayers ; i++) {
               s=PLR_getPlayerSprite(i);
               debugFPOut("BEG=%03d RSEED=%9ld PLR %d: H=%4d X=%8ld Y=%8ld Z=%8ld\n",
                          moveFIFObeg,randomseed,i,ACT_getHealth(s),
                          spritePtr[s]->x,spritePtr[s]->y,spritePtr[s]->z);
          }
     }
#endif
     GAM_moveThings();
}

void
GAM_doFrame(void)
{
     short viewSprite;
     static short lastViewSprite;

     GAM_setMadeNoise(-1);
     GAM_checkCheats();
     while (moveFIFObeg != moveFIFOend) {
          GAM_doMoveThings();
          moveFIFObeg=(moveFIFObeg+1)&(MOVEFIFOSIZE-1);
     }
     GFX_animatePalette();
     SND_playAmbientSounds(GAM_getViewSprite());
     viewSprite=WEP_getMissileView();
     if (viewSprite < 0) {
          viewSprite=GAM_getViewSprite();
     }
     if (viewSprite != lastViewSprite) {
          GAM_setupVideo(viewSize);
     }
     PLR_checkStatusBar();
     ENG_drawScreen(viewSprite,0L);
     lastViewSprite=viewSprite;
     ACT_setMadeNoise(GAM_getMadeNoise());
}

void
GAM_gamePlay(void)                 // actual game loop is here
{
     clearview(0L);
     nextpage();
     if (!newMapLoadedFlag || ENG_editorGameMode()) {
          GFX_fadeIn(8);
          GAM_loadLevel(currentMap);
          if (MUL_waitForPlayers() == 0) {
               inGameFlag=0;
               clearview(0L);
               nextpage();
               return;
          }
     }
     GAM_setupVideo(player[currentView]->viewSize);
     if (dumpDebugInfoFlag) {
          sprintf(tempbuf,"DEBUG.%03d",myconnectindex);
          dbgfp=fopen(tempbuf,"w");
     }
     moveFIFObeg=0;
     moveFIFOend=0;
     loadLevelFlag=0;
     newMapLoadedFlag=0;
     if (playingGameFlag == -1) {
          playingGameFlag=0;
     }
     else {
          playingGameFlag=1;
     }
     while (playingGameFlag && !loadLevelFlag) {
          GAM_doFrame();
          nextpage();
          if (doMainMenuFlag) {
               noEnemiesFlag=1;
               playingGameFlag=MEN_mainMenu();
               noEnemiesFlag=0;
               doMainMenuFlag=0;
          }
     }
     if (dumpDebugInfoFlag) {
          fclose(dbgfp);
     }
     ENG_setupVideo();
     nextpage();
}

void
GAM_gameLoop(void)                 // prepare to play game
{
     int  gameState=0;

     if (gameType != GAMETYPE_SINGLE || (!playingGameFlag && noWaitFlag)
         || newMapLoadedFlag) {
          gameState=GAME_PLAY;
     }
     while (1) {
          switch (gameState) {
          case GAME_ALLIANCE:
               if (MEN_getAlliance()) {
                    gameState++;
               }
               else if (gameState > 0) {
                    gameState--;
               }
               else {
                    return;
               }
               break;
          case GAME_MISSION:       // ask what mission
               if (MEN_getMission()) {
                    gameState++;
               }
               else if (gameState > 0) {
                    gameState--;
               }
               else {
                    return;
               }
               break;
          case GAME_SKILL:
               if (MEN_getSkill()) {
                    gameState++;
               }
               else if (gameState > 0) {
                    gameState--;
               }
               else {
                    return;
               }
               break;
          case GAME_ACTOR:
               if (curAlliance == 0 && MEN_getAlliedActor()) {
                    gameState++;
               }
               else if (curAlliance == 1 && MEN_getAxisActor()) {
                    gameState++;
               }
               else if (gameState > 0) {
                    gameState--;
               }
               else {
                    return;
               }
               break;
          case GAME_PLAY:
               do {
                    GAM_gamePlay();
               } while (loadLevelFlag);
               if (newGameFlag) {
                    gameState=0;
                    newGameFlag=0;
               }
               else {
                    gameState++;
               }
               break;
          default:
               return;
          }
     }
}

enum {
     INGAME_MAINMENU,
     INGAME_GAMELOOP,
     INGAME_DEMOLOOP
};

void
GAM_initGame(void)
{
     int  inGameState=INGAME_DEMOLOOP;

     gameInitializedFlag=1;
     inGameFlag=1;
     ENG_setupVideo();
     while (inGameFlag) {
          switch (inGameState) {
          case INGAME_MAINMENU:
               if (MEN_mainMenu()) {
                    inGameState=INGAME_GAMELOOP;
               }
               else {
                    inGameState=INGAME_DEMOLOOP;
               }
               break;
          case INGAME_GAMELOOP:
               GAM_gameLoop();
               inGameState=INGAME_MAINMENU;
               break;
          default:
               GAM_demoLoop();
               inGameState=INGAME_MAINMENU;
               break;
          }
     }
}

void
GAM_uninitGame(void)
{
     if (gameInitializedFlag) {
          gameInitializedFlag=0;
     }
}

void
GAM_nextLevel(void)
{
     memset(mapFileName,0,sizeof(mapFileName));
     bypassLevelFlag=0;
     loadLevelFlag=1;
     currentMap++;
}

#if 0
extern
long rotatex[MAXSPRITES],
     rotatey[MAXSPRITES];

void
GAM_doRotateSectorPhysics(short snum,long *x,long *y)
{
     short sectnum;
     struct doorData *dptr;

     sectnum=spritePtr[snum]->sectnum;
     if (sectorPtr[sectnum]->lotag == SEC_ROTATESECTORTAG) {
          dptr=doorPtr[doorIndex[sectnum]];
     }
}
#endif

//**
//** GAM_doPhysics: expects globloz & globhiz to be set using getzrange()
//**

void
GAM_doPhysics(short s,long *curz,short *zvel,short zAdj,short zHeight,
              short clipheight)
{
     long goalz,newzvel;
     sectortype *sect;

//** find the Z point of view we want to be at
     newzvel=*zvel;
     if (*curz > (globloz-clipheight) || *curz < (globhiz+clipheight)) {
          sect=sectorPtr[spritePtr[s]->sectnum];
          globloz=sect->floorz;
          globhiz=sect->ceilingz;
     }
     goalz=globloz-zAdj;
     if (goalz < globhiz+clipheight) {
          goalz=(globloz+globhiz)>>1;
     }
//** apply physics to current Z until goalz reached
     *curz+=newzvel;
     if (*curz > globloz-clipheight) {
          *curz=globloz-clipheight;
     }
     if (*curz < globhiz+clipheight) {
          *curz=globhiz+clipheight;
          newzvel=0L;
     }
     if (*curz < globloz-zHeight) {
          GAM_onGround(s,0);
          newzvel+=gravityConstant;
     }
     else {
          GAM_onGround(s,1);
          if (*curz < goalz) {
               newzvel+=gravityConstant;
          }
          else {
               newzvel=(((goalz-*curz)*TICWAITPERFRAME)>>4);
          }
     }
}

void
GAM_applyForce(short target,short source,int force)
{
     short a;
     spritetype *spr1,*spr2;

     spr1=spritePtr[source];
     spr2=spritePtr[target];
     a=getangle(spr2->x-spr1->x,spr2->y-spr1->y);
     forceAng[target]=a;
     forceVel[target]=kmin(force,256);
}

void
GAM_doStatusCode(void)
{
     WEP_doStatusCode();
     EFF_doStatusCode();
     PLR_doStatusCode();
     ACT_doStatusCode();
}

void
GAM_fwrite(void *buf,size_t elsize,size_t nelem,FILE *fp)
{
     long total,totalwritten=0;
     char c,*ptr,run;

     ptr=(char *)buf;
     total=elsize*nelem;
     if (total < 4) {
          fwrite(buf,elsize,nelem,fp);
          return;
     }
     do {
          run=0;
          c=ptr[totalwritten];
          do {
               totalwritten++;
               run++;
               if (run == 255) {
                    break;
               }
          } while (ptr[totalwritten] == c && totalwritten < total);
          fwrite(&run,sizeof(char),1,fp);
          fwrite(&c,sizeof(char),1,fp);
     } while (totalwritten < total);
}

size_t
GAM_fread(void *buf,size_t elsize,size_t nelem,FILE *fp)
{
     long total,totalread=0;
     char c,*ptr,run;

     ptr=(char *)buf;
     total=elsize*nelem;
     if (total < 4) {
          fread(buf,elsize,nelem,fp);
          return(total);
     }
     do {
          fread(&run,sizeof(char),1,fp);
          fread(&c,sizeof(char),1,fp);
          do {
               ptr[totalread++]=c;
               run--;
          } while (run > 0 && totalread < total);
     } while (totalread < total);
     return(totalread);
}

void
GAM_saveGame(short savespot)
{
     FILE *fp;

     sprintf(tempbuf,SAVENAMEFMT,savespot);
     fp=fopen(tempbuf,"wb");
     if (fp == NULL) {
          return;
     }
     MEN_saveGame(fp);
     GAM_fwrite(&numsectors,sizeof(short),1,fp);
     GAM_fwrite(sector,sizeof(sectortype),numsectors,fp);
     GAM_fwrite(&numwalls,sizeof(short),1,fp);
     GAM_fwrite(wall,sizeof(walltype),numwalls,fp);
     GAM_fwrite(sprite,sizeof(spritetype),MAXSPRITES,fp);
     GAM_fwrite(headspritesect,sizeof(short),MAXSECTORS+1,fp);
     GAM_fwrite(prevspritesect,sizeof(short),MAXSPRITES,fp);
     GAM_fwrite(nextspritesect,sizeof(short),MAXSPRITES,fp);
     GAM_fwrite(headspritestat,sizeof(short),MAXSTATUS+1,fp);
     GAM_fwrite(prevspritestat,sizeof(short),MAXSPRITES,fp);
     GAM_fwrite(nextspritestat,sizeof(short),MAXSPRITES,fp);
     GAM_fwrite(&numplayers,sizeof(short),1,fp);
     GAM_fwrite(&myconnectindex,sizeof(short),1,fp);
     GAM_fwrite(&connecthead,sizeof(short),1,fp);
     GAM_fwrite(connectpoint2,sizeof(short),MAXPLAYERS,fp);
     GAM_fwrite(&startx,sizeof(long),1,fp);
     GAM_fwrite(&starty,sizeof(long),1,fp);
     GAM_fwrite(&startz,sizeof(long),1,fp);
     GAM_fwrite(&starts,sizeof(short),1,fp);
     GAM_fwrite(&starta,sizeof(short),1,fp);
     GAM_fwrite(&aliensLeft,sizeof(short),1,fp);
     ACT_saveGame(fp);
     EFF_saveGame(fp);
     ENG_saveGame(fp);
     MUL_saveGame(fp);
     PLR_saveGame(fp);
     WEP_saveGame(fp);
     fclose(fp);
}

void
GAM_loadGame(short savespot)
{
     FILE *fp;

     sprintf(tempbuf,SAVENAMEFMT,savespot);
     fp=fopen(tempbuf,"rb");
     if (fp == NULL) {
          return;
     }
     newMapLoadedFlag=1;
     MEN_loadGame(fp);
     GAM_fread(&numsectors,sizeof(short),1,fp);
     GAM_fread(sector,sizeof(sectortype),numsectors,fp);
     GAM_fread(&numwalls,sizeof(short),1,fp);
     GAM_fread(wall,sizeof(walltype),numwalls,fp);
     GAM_fread(sprite,sizeof(spritetype),MAXSPRITES,fp);
     GAM_fread(headspritesect,sizeof(short),MAXSECTORS+1,fp);
     GAM_fread(prevspritesect,sizeof(short),MAXSPRITES,fp);
     GAM_fread(nextspritesect,sizeof(short),MAXSPRITES,fp);
     GAM_fread(headspritestat,sizeof(short),MAXSTATUS+1,fp);
     GAM_fread(prevspritestat,sizeof(short),MAXSPRITES,fp);
     GAM_fread(nextspritestat,sizeof(short),MAXSPRITES,fp);
     GAM_fread(&numplayers,sizeof(short),1,fp);
     GAM_fread(&myconnectindex,sizeof(short),1,fp);
     GAM_fread(&connecthead,sizeof(short),1,fp);
     GAM_fread(connectpoint2,sizeof(short),MAXPLAYERS,fp);
     GAM_fread(&startx,sizeof(long),1,fp);
     GAM_fread(&starty,sizeof(long),1,fp);
     GAM_fread(&startz,sizeof(long),1,fp);
     GAM_fread(&starts,sizeof(short),1,fp);
     GAM_fread(&starta,sizeof(short),1,fp);
     GAM_fread(&aliensLeft,sizeof(short),1,fp);
     ACT_loadGame(fp);
     EFF_loadGame(fp);
     ENG_loadGame(fp);
     MUL_loadGame(fp);
     PLR_loadGame(fp);
     WEP_loadGame(fp);
     GAM_scanMap();
     GAM_loadTextures();
     fclose(fp);
     fakeClock=totalclock;
}

