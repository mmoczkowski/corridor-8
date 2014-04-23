/***************************************************************************
 *   CORR7EFF.C - Special effects functions for Corridor 7
 *
 *                                                     02/29/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   MAXBONUSES               100  // maximum polygon bonuses per level
#define   MAXBLOODCOUNT            16
#define   MAXGORECOUNT             64
#define   MAXMOVINGSECTORS         10
#define   MAXMOVINGSECTORPOINTS    200
#define   MAXMOVINGSECTORSECTORS   25
#define   MAXMOVINGDOORS           MAXDOORS

int  bloodCount,
     goreCount;

short lastDamageClock;

short goreSprite[MAXGORECOUNT];

short frames[MAXSPRITES],
     frameDelay[MAXSPRITES],
     frameDelayReset[MAXSPRITES];

short hitagWallIndex[MAXWALLS],
     hitagSpriteIndex[MAXSPRITES];

char spriteSwitchStatus[MAXSPRITES>>3],
     wallSwitchStatus[MAXWALLS>>3];

short belowWaterIndex[MAXSECTORS],
     doorIndex[MAXSECTORS],
     movingDoor[MAXMOVINGDOORS],
     movingSectorIndex[MAXSECTORS],
     numDoors,
     numMovingDoors,
     numMovingSectors;

short lightWallIndex[MAXLIGHTEFFECTS];

short sectorEffectIndex[MAXSECTOREFFECTS];

struct sectorEffect sectorEffectStruct[MAXSECTOREFFECTS];

struct sectorCenters sectorCenter[MAXSECTORS],
     *sectorCenterPtr[MAXSECTORS];

struct message message[MAXMESSAGES],
     *messagePtr[MAXMESSAGES];

struct doorData door[MAXDOORS],
     *doorPtr[MAXDOORS];

struct movingSectorData {
     char active;
     char speed;
     char goalSpeed;
     char maxSpeed;
     short sectnum;
     short delay;
     short pivotSprite;
     short targetSprite;
     short numSectors;
     short sector[MAXMOVINGSECTORSECTORS];
     short pointDeltaX[MAXMOVINGSECTORPOINTS];
     short pointDeltaY[MAXMOVINGSECTORPOINTS];
} movingSector[MAXMOVINGSECTORS],
     *movingSectorPtr[MAXMOVINGSECTORS];

void
EFF_init(void)
{
     int  i;

     for (i=0 ; i < MAXSECTORS ; i++) {
          sectorCenterPtr[i]=&sectorCenter[i];
     }
     for (i=0 ; i < MAXMESSAGES ; i++) {
          messagePtr[i]=&message[i];
     }
     for (i=0 ; i < MAXDOORS ; i++) {
          doorPtr[i]=&door[i];
     }
     for (i=0 ; i < MAXMOVINGSECTORS ; i++) {
          movingSectorPtr[i]=&movingSector[i];
     }
}

void
EFF_unInit(void)
{
}

short
EFF_getDoorSpeed(short s)
{
     return(sectorPtr[s]->extra&0x00FF);
}

void
EFF_setDoorSpeed(short s,short n)
{
     sectorPtr[s]->extra&=~0x00FF;
     sectorPtr[s]->extra|=(n&0x00FF);
}

short
EFF_getDoorLock(short s)
{
     return((sectorPtr[s]->extra>>8)&0x00FF);
}

void
EFF_setDoorLock(short s,short n)
{
     sectorPtr[s]->extra&=~0xFF00;
     sectorPtr[s]->extra|=((n&0x00FF)<<8);
}

int
EFF_isBonusSector(short s)
{
     if (sectorPtr[s]->lotag == SEC_BONUSSECTORTAG) {
          return(1);
     }
     return(0);
}

short
EFF_getBonusSectorItem(short s)
{
     return((sectorPtr[s]->extra>>8)&0x00FF);
}

void
EFF_setBonusSectorItem(short s,short n)
{
     sectorPtr[s]->extra&=~0xFF00;
     sectorPtr[s]->extra|=((n&0x00FF)<<8);
}

short
EFF_getBonusSectorValue(short s)
{
     return(sectorPtr[s]->hitag);
}

void
EFF_setBonusSectorValue(short s,short n)
{
     sectorPtr[s]->hitag=n;
}

void
EFF_deleteBonusSector(short s)
{
     short i,j,newpic,sectnum;
     long newx,newy,newz,startz;
     signed char newshade;
     struct doorData *dptr;

     dptr=doorPtr[doorIndex[s]];
     startz=sectorPtr[s]->floorz;
     sectnum=nextsectorneighborz(s,startz,1,1);
     newz=sectorPtr[sectnum]->floorz;
     newpic=sectorPtr[sectnum]->floorpicnum;
     newshade=sectorPtr[sectnum]->floorshade;
     sectorPtr[s]->floorz=newz;
     sectorPtr[s]->floorpicnum=newpic;
     sectorPtr[s]->floorshade=newshade;
     sectorPtr[s]->lotag=0;
     i=0;
     while ((j=dptr->wallIndex[i]) >= 0) {
          newx=dptr->wallx[i];
          newy=dptr->wally[i];
          dragpoint(j,newx,newy);
          i++;
     }
}

short
EFF_getSpriteKey(short s)
{
     return(spritePtr[s]->extra);
}

void
EFF_setSpriteKey(short s,short n)
{
     spritePtr[s]->extra=n;
}

short
EFF_getWallKey(short w)
{
     return(wallPtr[w]->extra);
}

void
EFF_setWallKey(short w,short n)
{
     wallPtr[w]->extra=n;
}

short
EFF_getWarpDestination(short w)
{
     return(wallPtr[w]->hitag);
}

void
EFF_setWarpDestination(short w,short n)
{
     wallPtr[w]->hitag=n;
}

short
EFF_getWarpSpriteTag(short s)
{
     return(spritePtr[s]->hitag);
}

void
EFF_setWarpSpriteTag(short s,short n)
{
     spritePtr[s]->hitag=n;
}

int
EFF_isWallSwitch(short w)
{
     switch (wallPtr[w]->lotag) {
     case WAL_SWITCHTAG:
          return(1);
     }
     return(0);
}

int
EFF_isWallTrigger(short w)
{
     switch (wallPtr[w]->lotag) {
     case WAL_TRIGGERTAG:
          return(1);
     }
     return(0);
}

int
EFF_isSpriteSwitch(short s)
{
     switch (spritePtr[s]->lotag) {
     case SPR_SWITCHTAG:
          return(1);
     }
     return(0);
}

int
EFF_getSpriteSwitchStatus(short s)
{
     if ((spriteSwitchStatus[s>>3]&(1<<(s&7))) != 0) {
          return(1);
     }
     return(0);
}

void
EFF_setSpriteSwitchStatus(short s,short onoff)
{
     if (onoff) {
          spriteSwitchStatus[s>>3]|=(1<<(s&7));
     }
     else {
          spriteSwitchStatus[s>>3]&=~(1<<(s&7));
     }
}

int
EFF_getWallSwitchStatus(short w)
{
     if ((wallSwitchStatus[w>>3]&(1<<(w&7))) != 0) {
          return(1);
     }
     return(0);
}

void
EFF_setWallSwitchStatus(short w,short onoff)
{
     if (onoff) {
          wallSwitchStatus[w>>3]|=(1<<(w&7));
     }
     else {
          wallSwitchStatus[w>>3]&=~(1<<(w&7));
     }
}

int
EFF_checkSwitches(short hitag,short *key)
{
     short i,n=0,retval=0,s,w;

     if (hitag > 0) {
          i=0;
          while ((s=hitagSpriteIndex[i++]) >= 0) {
               if (spritePtr[s]->hitag != hitag) {
                    continue;
               }
               if (EFF_isSpriteSwitch(s)) {
                    if (EFF_getSpriteSwitchStatus(s)) {
                         n+=EFF_getSpriteKey(s);
                         retval=1;
                    }
               }
               else if (EFF_isMovingSectorMarker(s)) {
                    retval=1;
               }
          }
          i=0;
          while ((w=hitagWallIndex[i++]) >= 0) {
               if (wallPtr[w]->hitag != hitag) {
                    continue;
               }
               if (EFF_isWallSwitch(w)) {
                    if (EFF_getWallSwitchStatus(w)) {
                         n+=EFF_getWallKey(w);
                         retval=1;
                    }
               }
               else if (EFF_isWallTrigger(w)) {
                    retval=1;
               }
          }
     }
     else {
          retval=1;
     }
     *key=n;
     return(retval);
}

int
EFF_testSpriteFlag(short s,short n)
{
     return(spritePtr[s]->yvel&n);
}

short
EFF_getSpriteFlag(short s)
{
     return(spritePtr[s]->yvel);
}

void
EFF_setSpriteFlag(short s,short n)
{
     spritePtr[s]->yvel|=n;
}

void
EFF_resetSpriteFlag(short s,short n)
{
     spritePtr[s]->yvel&=~n;
}

void
EFF_setSpriteFrames(short s, long tics, short picbeg, short picend)
{
     spritePtr[s]->picnum = picbeg;
     frameDelayReset[s] = tics;
     frameDelay[s] = tics;
     frames[s] = picend - picbeg;
}

short
EFF_getSectorLight(short s)
{
     return(sectorPtr[s]->visibility);
}

void
EFF_setSectorLight(short s,short n)
{
     sectorPtr[s]->visibility=n;
}

short
EFF_adjSectorLight(short s,short n)
{
     short v;

     v=EFF_getSectorLight(s);
     v+=n;
     if (v > 239) {
          EFF_setSectorLight(s,239);
     }
     else if (v < 0) {
          EFF_setSectorLight(s,0);
     }
     else {
          EFF_setSectorLight(s,v);
     }
     return(v);
}

void
EFF_sectorLightsOff(short s)
{
     EFF_setSectorLight(s,239);
}

void
EFF_sectorLightsOn(short s)
{
     EFF_setSectorLight(s,0);
}

short
EFF_getDamageClock(void)
{
     return(lastDamageClock);
}

void
EFF_resetDamageClock(void)
{
     lastDamageClock=0;
}

//**
//** Special effects code
//**

short
EFF_getNextSectorEffectIndex(void)
{
     short i;

     for (i=0 ; i < MAXSECTOREFFECTS ; i++) {
          if (sectorEffectIndex[i] == -1) {
               return(i);
          }
     }
     return(-1);
}

short
EFF_addSectorEffect(short s,short tag)
{
     short j;

     if ((j=EFF_getNextSectorEffectIndex()) >= 0) {
          sectorEffectIndex[j]=s;
          sectorEffectStruct[j].effectTag=tag;
     }
     return(j);
}

void
EFF_warpEffect(short s)
{
     short j;

     j=ENG_insertsprite(spritePtr[s]->sectnum,STAT_HITSCANEXPLODE);
     spritePtr[j]->x=spritePtr[s]->x;
     spritePtr[j]->y=spritePtr[s]->y;
     spritePtr[j]->z=spritePtr[s]->z-(32<<8);
     spritePtr[j]->cstat=SPRC_REALCENTERED;
     spritePtr[j]->xrepeat=64;
     spritePtr[j]->yrepeat=64;
     spritePtr[j]->shade=0;
     EFF_setSpriteFrames(j,TMR_getSecondFraction(16),WARPBEGPIC,WARPENDPIC);
     SND_playTeleportSound(j);
}

void
EFF_warpSprite(short w,short snum,short *ang,short *sect,
               long *x,long *y,long *z)
{
     short warpto;

     warpto=EFF_getWarpDestination(w);
     EFF_warpEffect(snum);
     EFF_warpEffect(warpto);
     *x=spritePtr[warpto]->x;
     *y=spritePtr[warpto]->y;
     *z=spritePtr[warpto]->z;
     *sect=spritePtr[warpto]->sectnum;
     *ang=spritePtr[warpto]->ang;
}

void
EFF_spawnSplash(spritetype *spr)
{
}

void
EFF_warpBelowWater(short s,spritetype *spr,short *sect,long *x,long *y,long *z)
{
     short clipdist,clipheight,cursect,newsect;
     long offsetx,offsety;

     cursect=*sect;     
     if ((newsect=belowWaterIndex[cursect]) == -1) {
          return;
     }
     EFF_spawnSplash(spr);
     clipdist=ACT_getActorClipDist(spr);
     clipheight=ACT_getActorClipHeight(spr);
     offsetx=sectorCenterPtr[cursect]->centerx-*x;
     offsety=sectorCenterPtr[cursect]->centery-*y;
     *sect=newsect;
     *x=sectorCenterPtr[newsect]->centerx-offsetx;
     *y=sectorCenterPtr[newsect]->centery-offsety;
     *z=sectorPtr[newsect]->ceilingz+(clipheight<<1);
     getzrange(*x,*y,*z,newsect,&globhiz,&globhihit,&globloz,&globlohit,
               clipdist,clipheight);
     GAM_underWater(s,1);
     GAM_onGround(s,0);
}

void
EFF_warpAboveWater(short s,spritetype *spr,short *sect,long *x,long *y,long *z)
{
     short clipdist,clipheight,cursect,newsect;
     long offsetx,offsety;

     cursect=*sect;
     if ((newsect=belowWaterIndex[cursect]) == -1) {
          return;
     }
     EFF_spawnSplash(spr);
     clipdist=ACT_getActorClipDist(spr);
     clipheight=ACT_getActorClipHeight(spr);
     offsetx=sectorCenterPtr[cursect]->centerx-*x;
     offsety=sectorCenterPtr[cursect]->centery-*y;
     *sect=newsect;
     *x=sectorCenterPtr[newsect]->centerx-offsetx;
     *y=sectorCenterPtr[newsect]->centery-offsety;
     *z=sectorPtr[newsect]->floorz-(clipheight<<1);
     getzrange(*x,*y,*z,newsect,&globhiz,&globhihit,&globloz,&globlohit,
               clipdist,clipheight);
     GAM_underWater(s,0);
     GAM_onGround(s,1);
}

int
EFF_isAboveWaterSector(short s)
{
     switch (sectorPtr[s]->lotag) {
     case SEC_ABOVEWATERTAG:
          return(1);
          break;
     }
     return(0);
}

int
EFF_isBelowWaterSector(short s)
{
     switch (sectorPtr[s]->lotag) {
     case SEC_BELOWWATERTAG:
          return(1);
          break;
     }
     return(0);
}

void
EFF_onSprite(short snum,short onsprite,short *ang,short *sect,
             long *x,long *y,long *z)
{
     short i,j;
     spritetype *spr;

     spr=spritePtr[snum];
     switch (spritePtr[onsprite]->lotag) {
     case SPR_BLUEKEYTAG:
          if (PLR_isPlayer(spr,-1)) {
               PLR_addInventoryItem(snum,INV_BLUEKEY);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_YELLOWKEYTAG:
          if (PLR_isPlayer(spr,-1)) {
               PLR_addInventoryItem(snum,INV_YELLOWKEY);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_BACKPACKTAG:
          for (i=0 ; i < MAXWEAPONS ; i++) {
               if (weaponPtr[i]->registered) {
                    WEP_setMaxAmmo(snum,weaponPtr[i]->registered-1,
                                   weaponPtr[i]->defaultAmmo<<1);
                    j=WEP_getMaxAmmo(snum,weaponPtr[i]->registered-1);
                    WEP_setAmmo(snum,weaponPtr[i]->registered-1,j);
               }
          }
          ENG_deletesprite(onsprite);
          break;
     case SPR_REDKEYTAG:
          if (PLR_isPlayer(spr,-1)) {
               PLR_addInventoryItem(snum,INV_REDKEY);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_CELLCHARGEPACKTAG:
          j=WEP_getMaxAmmo(snum,WEAPON_ENERGY);
          if (WEP_getAmmo(snum,WEAPON_ENERGY) < j) {
               WEP_adjAmmo(snum,WEAPON_ENERGY,j);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_REDSKULLKEYTAG:
          if (PLR_isPlayer(spr,-1)) {
               PLR_addInventoryItem(snum,INV_REDKEY);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_YELLOWSKULLKEYTAG:
          if (PLR_isPlayer(spr,-1)) {
               PLR_addInventoryItem(snum,INV_YELLOWKEY);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_BLUESKULLKEYTAG:
          if (PLR_isPlayer(spr,-1)) {
               PLR_addInventoryItem(snum,INV_BLUEKEY);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_DOUBLESHOTGUNTAG:
          i=WEP_getWeaponAmmoType(GUN_4);
          j=WEP_getMaxAmmo(snum,i);
          WEP_addWeapon(snum,GUN_4);
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,weaponPtr[GUN_4]->defaultAmmo);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_MEGASPHERETAG:
          if (ACT_getHealth(snum) < playerMaxHealth) {
               j=ACT_getHealth(onsprite);
               if (j <= 0) {
                    j=playerMaxHealth<<1;
               }
               ACT_healActor(snum,j);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_SHOTGUNTAG:
          i=WEP_getWeaponAmmoType(GUN_3);
          j=WEP_getMaxAmmo(snum,i);
          WEP_addWeapon(snum,GUN_3);
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,weaponPtr[GUN_3]->defaultAmmo);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_CHAINGUNTAG:
          i=WEP_getWeaponAmmoType(GUN_1);
          j=WEP_getMaxAmmo(snum,i);
          WEP_addWeapon(snum,GUN_1);
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,weaponPtr[GUN_1]->defaultAmmo);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_ROCKETLAUNCHERTAG:
          i=WEP_getWeaponAmmoType(GUN_5);
          j=WEP_getMaxAmmo(snum,i);
          WEP_addWeapon(snum,GUN_5);
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,weaponPtr[GUN_5]->defaultAmmo);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_PLASMAGUNTAG:
          i=WEP_getWeaponAmmoType(GUN_6);
          j=WEP_getMaxAmmo(snum,i);
          WEP_addWeapon(snum,GUN_6);
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,weaponPtr[GUN_6]->defaultAmmo);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_CHAINSAWTAG:
          i=WEP_getWeaponAmmoType(GUN_2);
          j=WEP_getMaxAmmo(snum,i);
          WEP_addWeapon(snum,GUN_2);
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,weaponPtr[GUN_2]->defaultAmmo);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_BFG9000TAG:
          i=WEP_getWeaponAmmoType(GUN_7);
          j=WEP_getMaxAmmo(snum,i);
          WEP_addWeapon(snum,GUN_7);
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,weaponPtr[GUN_7]->defaultAmmo);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_AMMOCLIPTAG:
          i=WEAPON_BULLET;
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,25);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_SHOTGUNSHELLSTAG:
          i=WEAPON_SHELL;
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,5);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_ROCKETTAG:
          i=WEAPON_MISSILE;
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,2);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_STIMPACKTAG:
          if (ACT_getHealth(snum) < playerMaxHealth) {
               j=ACT_getHealth(onsprite);
               if (j <= 0) {
                    j=playerMaxHealth>>3;
               }
               ACT_healActor(snum,j);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_MEDIKITTAG:
          if (ACT_getHealth(snum) < playerMaxHealth) {
               j=ACT_getHealth(onsprite);
               if (j <= 0) {
                    j=playerMaxHealth>>2;
               }
               ACT_healActor(snum,j);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_SOULSPHERETAG:
          if (ACT_getHealth(snum) < (playerMaxHealth<<1)) {
               j=ACT_getHealth(onsprite);
               if (j <= 0) {
                    j=playerMaxHealth<<1;
               }
               ACT_healActor(snum,j);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_HEALTHPOTIONTAG:
          j=ACT_getHealth(onsprite);
          if (j <= 0) {
               j=playerMaxHealth/100;
          }
          ACT_healActor(snum,j);
          ENG_deletesprite(onsprite);
          break;
     case SPR_SPIRITARMORTAG:
     case SPR_GREENARMORTAG:
     case SPR_BLUEARMORTAG:
     case SPR_INVULNERABILITYTAG:
     case SPR_BERSERKTAG:
     case SPR_INVISIBILITYTAG:
     case SPR_RADIATIONSUITTAG:
     case SPR_COMPUTERMAPTAG:
     case SPR_LIGHTAMPLIFICATIONTAG:
          break;
     case SPR_BOXOFROCKETSTAG:
          i=WEAPON_MISSILE;
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,25);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_CELLCHARGETAG:
          i=WEAPON_ENERGY;
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,50);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_BOXOFAMMOTAG:
          i=WEAPON_BULLET;
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,100);
               ENG_deletesprite(onsprite);
          }
          break;
     case SPR_BOXOFSHELLSTAG:
          i=WEAPON_SHELL;
          if (WEP_getAmmo(snum,i) < WEP_getMaxAmmo(snum,i)) {
               WEP_adjAmmo(snum,i,25);
               ENG_deletesprite(onsprite);
          }
          break;
     }
}

void
EFF_notify(long bcolor,char *fmt,...)
{
     va_list argptr;

     va_start(argptr,fmt);
     vsprintf(tempbuf,fmt,argptr);
     va_end(argptr);
     if (qsetmode == 0L) {
          lprintf("%s\n",tempbuf);
     }
     else if (qsetmode == 200L) {
          clearview(0L);
          if (gotpic[backgroundPic>>3]&(1<<(backgroundPic&7)) == 0) {
               loadtile(backgroundPic);
          }
          rotatesprite(320L<<15,200L<<15,65536L,0,
                       backgroundPic,0,0,2+8+64+128,
                       0L,0L,xdim-1L,ydim-1L);
          printext256((xdim >> 1) - (strlen(tempbuf) << 2), ydim >> 1,
                      P256COLOR, -1, tempbuf, 0);
          nextpage();
          GFX_fadeIn(255);
     }
     else if (editorEnabledFlag) {
          printmessage16(tempbuf);
     }
}

void
EFF_computeSectorCenter(short i)
{
     int  w;
     long cx,
          cy,
          endwall,
          startwall,
          walls;
     struct sectorCenters *sectx;
     sectortype *sect;
     
     sect=sectorPtr[i];
     sectx=sectorCenterPtr[i];
     cx=0L;
     cy=0L;
     startwall=sect->wallptr;
     endwall=sect->wallptr+sect->wallnum;
     walls=endwall-startwall;
     for (w=startwall ; w < endwall ; w++) {
          cx+=wallPtr[w]->x;
          cy+=wallPtr[w]->y;
     }
     sectx->centerx=cx/walls;
     sectx->centery=cy/walls;
     sectx->centerz=(sect->ceilingz+sect->floorz)>>1;
}

void
EFF_computeSectorCenters(void)
{
     int  i;

     if (qsetmode == 200L) {
          EFF_notify(0L,"COMPUTING %d SECTORS",numsectors);
     }
     for (i = 0; i < numsectors; i++) {
          if (qsetmode != 200L) {
               EFF_notify(0L,"COMPUTING SECTOR %d",i);
          }
          EFF_computeSectorCenter(i);
     }
}

int
EFF_isDoor(short s)
{
     switch (sectorPtr[s]->lotag) {
     case SEC_DOORUPTAG:
     case SEC_DOORUPONCETAG:
     case SEC_DOORDOWNTAG:
     case SEC_DOORDOWNONCETAG:
     case SEC_DOORHSPLITTAG:
     case SEC_DOORHSPLITONCETAG:
     case SEC_DOORSLIDECENTERTAG:
     case SEC_ROTATESECTORTAG:     // these are here because we use the
     case SEC_BONUSSECTORTAG:      // ..doorData structure array
          return(1);
     }
     return(0);
}

int
EFF_isElevator(short s)
{
     switch (sectorPtr[s]->lotag) {
     case SEC_PLATFORMDOWNTAG:
     case SEC_PLATFORMELEVATORTAG:
     case SEC_PLATFORMUPTAG:
          return(1);
     }
     return(0);
}

int
EFF_operatableSector(short s)
{
     if (EFF_isDoor(s) || EFF_isElevator(s)) {
          return(1);
     }
     return(0);
}

void
EFF_blockDoorWalls(short s)
{
     short w,wall1,wall2;
     long floorz2;

     if (EFF_isDoor(s)) {
          wall1=sectorPtr[s]->wallptr;
          wall2=wall1+sectorPtr[s]->wallnum;
          for (w=wall1 ; w < wall2 ; w++) {
               if (wallPtr[w]->nextwall != -1) {
                    wallPtr[wallPtr[w]->nextwall]->cstat|=1;
               }
          }
     }
#if 0
     else if (EFF_isElevator(s)) {
          wall1=sectorPtr[s]->wallptr;
          wall2=wall1+sectorPtr[s]->wallnum;
          for (w=wall1 ; w < wall2 ; w++) {
               if (wallPtr[w]->nextsector != -1) {
                    if (wallPtr[w]->nextwall == -1) {
                         continue;
                    }
                    floorz2=sectorPtr[wallPtr[w]->nextsector]->floorz;
                    if (labs(sectorPtr[s]->floorz-floorz2) >=
                        (defaultPlayerHeight<<8)) {
                         wallPtr[wallPtr[w]->nextwall]->cstat|=WALC_BLOCKING;
                    }
               }
          }
     }
#endif
}

void
EFF_unblockDoorWalls(short s)
{
     short w,wall1,wall2;

     wall1=sectorPtr[s]->wallptr;
     wall2=wall1+sectorPtr[s]->wallnum;
     for (w=wall1 ; w < wall2 ; w++) {
          if (wallPtr[w]->nextwall != -1) {
               wallPtr[wallPtr[w]->nextwall]->cstat&=~WALC_BLOCKING;
          }
     }
}

void
EFF_buildHitagIndex(void)
{
     int  i,j;

     for (i=0,j=0 ; i < MAXSPRITES ; i++) {
          if (spritePtr[i]->statnum >= MAXSTATUS) {
               continue;
          }
          if (EFF_isSpriteSwitch(i) || EFF_isMovingSectorMarker(i)) {
               if (EFF_getSpriteKey(i) < 0) {
                    EFF_setSpriteKey(i,0);
               }
               hitagSpriteIndex[j++]=i;
          }
     }
     hitagSpriteIndex[j]=-1;
     for (i=0,j=0 ; i < numwalls ; i++) {
          if (EFF_isWallSwitch(i) || EFF_isWallTrigger(i)) {
               if (EFF_getWallKey(i) < 0) {
                    EFF_setWallKey(i,0);
               }
               hitagWallIndex[j++]=i;
          }
     }
     hitagWallIndex[j]=-1;
}

void
EFF_scanSprite(short i)
{
     short j;
     spritetype *spr;
     struct sectorEffect *si;

     spr=spritePtr[i];
     if (spr->statnum < MAXSTATUS) {
          if (ACT_isActor(i)) {
               return;
          }
          switch (spr->lotag) {
          case SPR_PANCEILTAG:
               if ((j=EFF_addSectorEffect(spr->sectnum,SEC_PANCEILTAG)) >= 0) {
                    si=&sectorEffectStruct[j];
                    si->speed=spr->extra;
                    si->ang=spr->ang;
               }
               ENG_deletesprite(i);
               break;
          case SPR_PANFLOORTAG:
               if ((j=EFF_addSectorEffect(spr->sectnum,SEC_PANFLOORTAG)) >= 0) {
                    si=&sectorEffectStruct[j];
                    if (spr->extra < 0) {
                         spr->extra=5;
                    }
                    si->speed=spr->extra;
                    si->ang=spr->ang;
               }
               ENG_deletesprite(i);
               break;
          case SPR_BLUEKEYTAG:
          case SPR_YELLOWKEYTAG:
          case SPR_BACKPACKTAG:
          case SPR_REDKEYTAG:
          case SPR_CELLCHARGEPACKTAG:
          case SPR_REDSKULLKEYTAG:
          case SPR_YELLOWSKULLKEYTAG:
          case SPR_BLUESKULLKEYTAG:
          case SPR_DOUBLESHOTGUNTAG:
          case SPR_MEGASPHERETAG:
          case SPR_SHOTGUNTAG:
          case SPR_CHAINGUNTAG:
          case SPR_ROCKETLAUNCHERTAG:
          case SPR_PLASMAGUNTAG:
          case SPR_CHAINSAWTAG:
          case SPR_BFG9000TAG:
          case SPR_AMMOCLIPTAG:
          case SPR_SHOTGUNSHELLSTAG:
          case SPR_ROCKETTAG:
          case SPR_STIMPACKTAG:
          case SPR_MEDIKITTAG:
          case SPR_SOULSPHERETAG:
          case SPR_HEALTHPOTIONTAG:
          case SPR_SPIRITARMORTAG:
          case SPR_GREENARMORTAG:
          case SPR_BLUEARMORTAG:
          case SPR_INVULNERABILITYTAG:
          case SPR_BERSERKTAG:
          case SPR_INVISIBILITYTAG:
          case SPR_RADIATIONSUITTAG:
          case SPR_COMPUTERMAPTAG:
          case SPR_LIGHTAMPLIFICATIONTAG:
          case SPR_BOXOFROCKETSTAG:
          case SPR_CELLCHARGETAG:
          case SPR_BOXOFAMMOTAG:
          case SPR_BOXOFSHELLSTAG:
               spr->cstat|=SPRC_BLOCKING;
               break;
          case SPR_MOVINGSECTORTAG:
               if (spr->statnum == STAT_NONE) {
                    changespritestat(i,STAT_MOVINGSECTORMARKER);
                    spr->cstat&=~(SPRC_BLOCKING|SPRC_BLOCKINGH);
                    spr->cstat|=SPRC_INVISIBLE;
               }
               break;
          }
     }
}

void
EFF_scanSectorEffects(short s)
{
     short endwall,i,j,startwall,w,wto;
     long areax1,areax2,areay1,areay2;
     walltype *wal;

     startwall=sectorPtr[s]->wallptr;
     endwall=startwall+sectorPtr[s]->wallnum;
     for (w=startwall ; w < endwall ; w++) {
          wal=wallPtr[w];
          switch (wal->lotag) {
          case WAL_WARPTAG:
               wto=EFF_getWarpDestination(w);
               for (j=0 ; j < MAXSPRITES ; j++) {
                    if (spritePtr[j]->statnum >= MAXSTATUS) {
                         continue;
                    }
                    if (spritePtr[j]->lotag != SPR_TELEPORTTAG) {
                         continue;
                    }
                    if (EFF_getWarpSpriteTag(j) == wto) {
                         EFF_setWarpDestination(w,j);
                         break;
                    }
               }
               break;
          case WAL_W1LIGHTSOFFTAG:
          case WAL_WRLIGHTSOFFTAG:
          case WAL_SRLIGHTSOFFTAG:
          case WAL_W1LIGHTSONFULLTAG:
          case WAL_WRLIGHTSONFULLTAG:
          case WAL_SRLIGHTSONFULLTAG:
               for (j=0 ; j < MAXLIGHTEFFECTS ; j++) {
                    if (lightWallIndex[j] == -1) {
                         lightWallIndex[j]=w;
                         break;
                    }
               }
               break;
          case WAL_MRMDOOROPENCLOSETAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPTAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPTAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,0);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_MRMBLUEDOOROPENCLOSETAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPTAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPTAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,125+INV_BLUEKEY);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_MRMYELLOWDOOROPENCLOSETAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPTAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPTAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,125+INV_YELLOWKEY);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_MRMREDDOOROPENCLOSETAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPTAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPTAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,125+INV_REDKEY);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_M1MDOOROPENSTAYTAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPONCETAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPONCETAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,0);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_M1MBLUEDOOROPENSTAYTAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPONCETAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPONCETAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,125+INV_BLUEKEY);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_M1MREDDOOROPENSTAYTAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPONCETAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPONCETAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,125+INV_REDKEY);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_M1MYELLOWDOOROPENSTAYTAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPONCETAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPONCETAG;
                    EFF_setDoorSpeed(s,DEFAULTDOORSPEED);
                    EFF_setDoorLock(s,125+INV_YELLOWKEY);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_MRTDOOROPENCLOSETAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPTAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPTAG;
                    EFF_setDoorSpeed(s,50<<4);
                    EFF_setDoorLock(s,0);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          case WAL_M1TDOOROPENSTAYTAG:
               if (sectorPtr[s]->lotag != SEC_DOORUPONCETAG) {
                    sectorPtr[s]->lotag=SEC_DOORUPONCETAG;
                    EFF_setDoorSpeed(s,50<<4);
                    EFF_setDoorLock(s,0);
                    j=nextsectorneighborz(s,sectorPtr[s]->ceilingz,-1,-1);
                    sectorPtr[s]->ceilingz=sectorPtr[j]->ceilingz;
               }
               wal->lotag=WAL_NORMAL;
               break;
          }
     }
     switch (sectorPtr[s]->lotag) {
     case SEC_BLINKOFFTAG:
     case SEC_BLINKONTAG:
     case SEC_BLINKON1STAG:
     case SEC_HIDAMAGEBLINKTAG:
     case SEC_OSCILLATETAG:
     case SEC_SYNCBLINKONTAG:
     case SEC_SYNCBLINKOFFTAG:
     case SEC_FLICKERTAG:
          EFF_addSectorEffect(s,sectorPtr[s]->lotag);
          break;
     case SEC_ABOVEWATERTAG:
          startwall=sectorPtr[s]->wallptr;
          endwall=startwall+sectorPtr[s]->wallnum;
          areax1=0L;
          areay1=0L;
          for (j=startwall ; j < endwall ; j++) {
               areax1+=wallPtr[wallPtr[j]->point2]->x-wallPtr[j]->x;
               areay1+=wallPtr[wallPtr[j]->point2]->y-wallPtr[j]->y;
          }
          belowWaterIndex[s]=-1;
          for (j=0 ; j < numsectors && belowWaterIndex[s] == -1 ; j++) {
               switch (sectorPtr[j]->lotag) {
               case SEC_BELOWWATERTAG:
                    endwall=sectorPtr[j]->wallnum;
                    if (endwall != sectorPtr[s]->wallnum) {
                         break;
                    }
                    startwall=sectorPtr[j]->wallptr;
                    endwall+=startwall;
                    areax2=0L;
                    areay2=0L;
                    for (i=startwall ; i < endwall ; i++) {
                         areax2+=wallPtr[wallPtr[i]->point2]->x-wallPtr[i]->x;
                         areay2+=wallPtr[wallPtr[i]->point2]->y-wallPtr[i]->y;
                    }
                    if (areax1 == areax2 && areay1 == areay2) {
                         belowWaterIndex[s]=j;
                         belowWaterIndex[j]=s;
                    }
                    break;
               }
          }
          break;
     }
}

short
EFF_getNextDoorIndex(void)
{
     short i;

     if (numDoors >= MAXDOORS) {
          crash("EFF_getNextDoorIndex: Too many doors (max=%d)",MAXDOORS);
     }
     i=numDoors++;
     return(i);
}

//**
//** Moving sector code
//**

int
EFF_isMovingSectorMarker(short s)
{
     if (spritePtr[s]->statnum == STAT_MOVINGSECTORMARKER) {
          return(1);
     }
     return(0);
}

int
EFF_isAutoMovingSector(short s)
{
     switch (sectorPtr[s]->lotag) {
     case SEC_AUTOMOVINGSECTORTAG:
     case SEC_AUTOMOVINGSECTORFIXEDTAG:
          return(1);
     }
     return(0);
}

int
EFF_isMovingSector(short s)
{
     switch (sectorPtr[s]->lotag) {
     case SEC_MOVINGSECTORTAG:
     case SEC_AUTOMOVINGSECTORTAG:
     case SEC_MOVINGSECTORFIXEDTAG:
     case SEC_AUTOMOVINGSECTORFIXEDTAG:
          return(1);
     }
     return(0);
}

short
EFF_getNextMovingSectorIndex(void)
{
     short i;

     if (numMovingSectors >= MAXMOVINGSECTORS) {
          crash("EFF_getNextMovingSectorIndex: Too many moving sectors "
                "(max=%d)",MAXMOVINGSECTORS);
     }
     i=numMovingSectors++;
     return(i);
}

int
EFF_insideSector(long x,long y,short sectnum)
{
     short endwall,startwall,w;
     long lox,loy,hix,hiy;

     lox=0x7FFFFFFFL;
     loy=0x7FFFFFFFL;
     hix=0x80000000L;
     hiy=0x80000000L;
     startwall=sectorPtr[sectnum]->wallptr;
     endwall=startwall+sectorPtr[sectnum]->wallnum;
     for (w=startwall ; w < endwall ; w++) {
          if (wallPtr[w]->x < lox) {
               lox=wallPtr[w]->x;
          }
          if (wallPtr[w]->x > hix) {
               hix=wallPtr[w]->x;
          }
          if (wallPtr[w]->y < loy) {
               loy=wallPtr[w]->y;
          }
          if (wallPtr[w]->y > hiy) {
               hiy=wallPtr[w]->y;
          }
     }
     if (x >= lox && x <= hix && y >= loy && y <= hiy) {
          return(1);
     }
     return(0);
}

void
EFF_addMovingSector(short sectnum)
{
     short endwall,i,s,skip,startwall,w;
     long deltax,deltay;
     struct movingSectorData *mptr;
     sectortype *sect;
     walltype *wal;

     sect=sectorPtr[sectnum];
     i=EFF_getNextMovingSectorIndex();
     movingSectorIndex[sectnum]=i;
     mptr=movingSectorPtr[i];
     mptr->active=0;
     mptr->numSectors=0;
     mptr->pivotSprite=-1;
     mptr->targetSprite=-1;
     mptr->sectnum=sectnum;
     mptr->delay=0;
     mptr->maxSpeed=EFF_getDoorSpeed(sectnum);
     if (mptr->maxSpeed <= 0) {
          crash("EFF_addMovingSector: Speed is 0 (hitag=%d)",sect->hitag);
     }
     for (s=0 ; s < numsectors ; s++) {
          startwall=sectorPtr[s]->wallptr;
          endwall=startwall+sectorPtr[s]->wallnum;
          skip=0;
          for (w=startwall ; w < endwall ; w++) {
               wal=wallPtr[w];
               if (EFF_insideSector(wal->x,wal->y,sectnum) == 0) {
                    skip=1;
                    break;
               }
          }
          if (!skip) {
               if (mptr->numSectors >= MAXMOVINGSECTORSECTORS) {
                    crash("EFF_addMovingSector: More than %d sectors (hitag=%d)",
                          MAXMOVINGSECTORSECTORS,sect->hitag);
               }
               mptr->sector[mptr->numSectors++]=s;
               if (EFF_operatableSector(s)) {
                    doorPtr[doorIndex[s]]->movingSectorFlag=1;
               }
               if (mptr->pivotSprite >= 0) {
                    continue;
               }
               for (i=headspritesect[s] ; i >= 0 ; i=nextspritesect[i]) {
                    if (spritePtr[i]->lotag == SPR_MOVINGSECTORTAG) {
                         spritePtr[i]->cstat&=(SPRC_BLOCKING|SPRC_BLOCKINGH);
                         spritePtr[i]->cstat|=SPRC_INVISIBLE;
                         changespritestat(i,STAT_LASTSTAT);
                         mptr->pivotSprite=i;
                         break;
                    }
               }
          }
     }
     if (mptr->pivotSprite == -1) {
          crash("EFF_addMovingSector: Pivot sprite missing (hitag=%d)",
                sect->hitag);
     }
     for (s=0,i=0 ; s < mptr->numSectors ; s++) {
          startwall=sectorPtr[mptr->sector[s]]->wallptr;
          endwall=startwall+sectorPtr[mptr->sector[s]]->wallnum;
          for (w=startwall ; w < endwall ; w++) {
               if (i >= MAXMOVINGSECTORPOINTS) {
                    crash("EFF_addMovingSector: More than %d points for "
                          "moving sector (hitag=%d)",
                          MAXMOVINGSECTORPOINTS,sect->hitag);
               }
               deltax=spritePtr[mptr->pivotSprite]->x-wallPtr[w]->x;
               mptr->pointDeltaX[i]=deltax;
               deltay=spritePtr[mptr->pivotSprite]->y-wallPtr[w]->y;
               mptr->pointDeltaY[i]=deltay;
               i++;
          }
     }
     if (EFF_isAutoMovingSector(sectnum)) {
          mptr->active=1;
          mptr->goalSpeed=mptr->maxSpeed;
     }
}

void
EFF_addMovingDoor(short d,struct doorData *dptr)
{
     short i;

     if (dptr->movingDoorIndex != -1) {
          return;
     }     
     if (numMovingDoors < MAXMOVINGDOORS) {
          dptr->movingDoorIndex=numMovingDoors;
          movingDoor[numMovingDoors]=d;
          numMovingDoors++;
     }
}

void
EFF_removeMovingDoor(short d,struct doorData *dptr)
{
     short i,n;
     
     if (numMovingDoors > 0) {
          numMovingDoors--;
          i=dptr->movingDoorIndex;
          if (i < numMovingDoors) {
               n=movingDoor[numMovingDoors];
               movingDoor[i]=n;
               doorPtr[n]->movingDoorIndex=i;
          }
     }
     dptr->movingDoorIndex=-1;
}

void
EFF_scanMap(void)
{
     short i,j,n,s,w;
     short endwall,startwall;
     long x,y;

     memset(&numDoors,0,sizeof(numDoors));
     memset(&numMovingSectors,0,sizeof(numMovingSectors));
     memset(&numMovingDoors,0,sizeof(numMovingDoors));
     memset(movingSectorIndex,-1,sizeof(movingSectorIndex));
     memset(lightWallIndex,-1,sizeof(lightWallIndex));
     memset(sectorEffectIndex,-1,sizeof(sectorEffectIndex));
     memset(goreSprite,-1,sizeof(goreSprite));
     memset(spriteSwitchStatus,0,sizeof(spriteSwitchStatus));
     memset(wallSwitchStatus,0,sizeof(wallSwitchStatus));
     for (i=0 ; i < MAXDOORS ; i++) {
          doorPtr[i]->movingSectorFlag=0;
          doorPtr[i]->sectorIndex=-1;
          doorPtr[i]->movingDoorIndex=-1;
          for (j=0 ; j < MAXSECTORPOINTS ; j++) {
               doorPtr[i]->wallIndex[j]=-1;
               doorPtr[i]->wallx[j]=-1;
               doorPtr[i]->wally[j]=-1;
          }
     }
     for (i=0 ; i < numsectors ; i++) {
          ENG_checksumSector(i);
          EFF_scanSectorEffects(i);
          switch (sectorPtr[i]->lotag) {
          case SEC_DOORUPTAG:
          case SEC_DOORUPONCETAG:
          case SEC_DOORDOWNTAG:
          case SEC_DOORDOWNONCETAG:
               n=EFF_getNextDoorIndex();
               doorIndex[i]=n;
               doorPtr[n]->sectorIndex=i;
               doorPtr[n]->hiz=sectorPtr[i]->ceilingz;
               doorPtr[n]->loz=sectorPtr[i]->floorz;
               doorPtr[n]->speed=EFF_getDoorSpeed(i)<<4;
               if (doorPtr[n]->speed <= 0) {
                    doorPtr[n]->speed=DEFAULTDOORSPEED;
               }
               switch (sectorPtr[i]->lotag) {
               case SEC_DOORUPTAG:
               case SEC_DOORUPONCETAG:
                    sectorPtr[i]->ceilingz=sectorPtr[i]->floorz;
                    break;
               case SEC_DOORDOWNTAG:
               case SEC_DOORDOWNONCETAG:
                    sectorPtr[i]->floorz=sectorPtr[i]->ceilingz;
                    break;
               }
               EFF_blockDoorWalls(i);
               break;
          case SEC_PLATFORMDOWNTAG:
          case SEC_PLATFORMELEVATORTAG:
               n=EFF_getNextDoorIndex();
               doorIndex[i]=n;
               doorPtr[n]->sectorIndex=i;
               doorPtr[n]->hiz=sectorPtr[i]->floorz;
               s=nextsectorneighborz(i,sectorPtr[i]->floorz,1,1);
               if (s == -1) {
                    s=nextsectorneighborz(i,sectorPtr[i]->floorz,1,-1);
                    if (s == -1) {
                         crash("EFF_scanMap: No adjacent floor for "
                               "elevator (sector=%d)",i);
                    }
               }
               doorPtr[n]->loz=sectorPtr[s]->floorz;
               doorPtr[n]->speed=EFF_getDoorSpeed(i)<<4;
               doorPtr[n]->height=sectorPtr[i]->floorz-sectorPtr[i]->ceilingz;
               if (doorPtr[n]->speed <= 0) {
                    doorPtr[n]->speed=DEFAULTDOORSPEED;
               }
               EFF_blockDoorWalls(i);
               break;
          case SEC_PLATFORMUPTAG:
               n=EFF_getNextDoorIndex();
               doorIndex[i]=n;
               doorPtr[n]->sectorIndex=i;
               doorPtr[n]->loz=sectorPtr[i]->floorz;
               s=nextsectorneighborz(i,sectorPtr[i]->floorz,1,-1);
               if (s == -1) {
                    s=nextsectorneighborz(i,sectorPtr[i]->floorz,1,1);
                    if (s == -1) {
                         crash("EFF_scanMap: No adjacent floor for platform "
                               "(sector=%d)",i);
                    }
               }
               doorPtr[n]->hiz=sectorPtr[s]->floorz;
               doorPtr[n]->speed=EFF_getDoorSpeed(i)<<4;
               doorPtr[n]->height=sectorPtr[i]->floorz-sectorPtr[i]->ceilingz;
               if (doorPtr[n]->speed <= 0) {
                    doorPtr[n]->speed=DEFAULTDOORSPEED;
               }
               EFF_blockDoorWalls(i);
               break;
          case SEC_DOORHSPLITTAG:
          case SEC_DOORHSPLITONCETAG:
               n=EFF_getNextDoorIndex();
               doorIndex[i]=n;
               doorPtr[n]->sectorIndex=i;
               doorPtr[n]->hiz=sectorPtr[i]->ceilingz;
               doorPtr[n]->loz=sectorPtr[i]->floorz;
               doorPtr[n]->speed=EFF_getDoorSpeed(i)<<4;
               if (doorPtr[n]->speed <= 0) {
                    doorPtr[n]->speed=DEFAULTDOORSPEED;
               }
               sectorPtr[i]->ceilingz=sectorCenterPtr[i]->centerz;
               sectorPtr[i]->floorz=sectorCenterPtr[i]->centerz;
               EFF_blockDoorWalls(i);
               break;
          case SEC_DOORSLIDECENTERTAG:
               n=EFF_getNextDoorIndex();
               doorIndex[i]=n;
               doorPtr[n]->sectorIndex=i;
               doorPtr[n]->speed=EFF_getDoorSpeed(i);
               if (doorPtr[n]->speed <= 0) {
                    doorPtr[n]->speed=DEFAULTDOORSPEED;
               }
               startwall=sectorPtr[i]->wallptr;
               endwall=startwall+sectorPtr[i]->wallnum;
               for (w=startwall ; w < endwall ; w++) {
                    if (wallPtr[w]->lotag == WAL_DOORSEARCHTAG) {
                         if (doorPtr[n]->wallIndex[0] == -1) {
                              for (j=0 ; j < 6 ; j++) {
                                   x=wallPtr[(w-2)+j]->x;
                                   y=wallPtr[(w-2)+j]->y;
                                   doorPtr[n]->wallx[j]=x;
                                   doorPtr[n]->wally[j]=y;
                                   doorPtr[n]->wallIndex[j]=(w-2)+j;
                              }
                         }
                         else {
                              for (j=0 ; j < 6 ; j++) {
                                   x=wallPtr[(w-2)+j]->x;
                                   y=wallPtr[(w-2)+j]->y;
                                   doorPtr[n]->wallx[j+6]=x;
                                   doorPtr[n]->wally[j+6]=y;
                                   doorPtr[n]->wallIndex[j+6]=(w-2)+j;
                              }
                         }
                    }
               }
               break;
          case SEC_ROTATESECTORTAG:
          case SEC_BONUSSECTORTAG:
               n=EFF_getNextDoorIndex();
               doorIndex[i]=n;
               doorPtr[n]->sectorIndex=i;
               doorPtr[n]->speed=EFF_getDoorSpeed(i);
               startwall=sectorPtr[i]->wallptr;
               endwall=startwall+sectorPtr[i]->wallnum;
               for (w=startwall,j=0 ; w < endwall ; j++,w++) {
                    if (j >= MAXSECTORPOINTS) {
                         crash("EFF_scanMap: Too many points for rotating"
                               " sector (max=%d)",MAXSECTORPOINTS);
                    }
                    doorPtr[n]->wallIndex[j]=w;
                    doorPtr[n]->wallx[j]=wallPtr[w]->x;
                    doorPtr[n]->wally[j]=wallPtr[w]->y;
               }
               doorPtr[n]->state=DOORSTATE_OPEN;
               EFF_addMovingDoor(n,doorPtr[n]);
               break;
          }
          if (EFF_operatableSector(i)) {
               if (EFF_getDoorLock(i) < 0) {
                    EFF_setDoorLock(i,0);
               }
          }
     }
//** this must be after the door setup code
     for (i=0 ; i < numsectors ; i++) {
          switch (sectorPtr[i]->lotag) {
          case SEC_MOVINGSECTORTAG:
          case SEC_AUTOMOVINGSECTORTAG:
          case SEC_MOVINGSECTORFIXEDTAG:
          case SEC_AUTOMOVINGSECTORFIXEDTAG:
               EFF_addMovingSector(i);
               break;
          }
     }
     for (i=0 ; i < MAXSPRITES ; i++) {
          EFF_scanSprite(i);
     }
}

//**
//** Door & elevator code
//**

int
EFF_isDoorClosed(short s)
{
     short d;

     if ((d=doorIndex[s]) == -1) {
          return(0);
     }
     switch (doorPtr[d]->state) {
     case DOORSTATE_IDLE:
     case DOORSTATE_CLOSE:
     case DOORSTATE_CLOSED:
          return(1);
     }
     return(0);
}

int
EFF_isDoorOpened(short s)
{
     short d;

     if ((d=doorIndex[s]) == -1) {
          return(0);
     }
     switch (doorPtr[d]->state) {
     case DOORSTATE_OPENED:
     case DOORSTATE_WAITING:
          return(1);
     }
     return(0);
}

int
EFF_moveSectorCeiling(short s,sectortype *sect,long delta,long goalz)
{
     int  retval=0;
     long z;
     
     if (delta != 0L) {
          z=sect->ceilingz;
          if (z < goalz) {
               z+=delta;
               if (z >= goalz) {
                    z=goalz;
                    retval=1;
               }
          }
          else {
               z-=delta;
               if (z <= goalz) {
                    z=goalz;
                    retval=1;
               }
          }
          sect->ceilingz=z;
          return(retval);
     }
     return(0);
}

int
EFF_moveSectorFloor(short s,sectortype *sect,long delta,long goalz)
{
     int  retval=0;
     long z;
     
     if (delta != 0L) {
          z=sect->floorz;
          if (z < goalz) {
               z+=delta;
               if (z >= goalz) {
                    z=goalz;
                    retval=1;
               }
          }
          else {
               z-=delta;
               if (z <= goalz) {
                    z=goalz;
                    retval=1;
               }
          }
          sect->floorz=z;
          return(retval);
     }
     return(0);
}

int
EFF_slideDoorOpen(short d)
{
     short i,retval=0,w[12];
     long delta,dx,sx[12],sy[12],x[12],y[12];
     struct doorData *dptr;

     dptr=doorPtr[d];
     delta=dptr->speed;
     for (i=0 ; i < 12 ; i++) {
          w[i]=dptr->wallIndex[i];
          x[i]=wallPtr[w[i]]->x;
          y[i]=wallPtr[w[i]]->y;
          sx[i]=dptr->wallx[i];
          sy[i]=dptr->wally[i];
     }
     if (sy[1] == sy[0]) {
          dx=x[0];
          if (x[1] > dx) {
               x[1]=kmax(x[1]-delta,dx);
          }
          else if (x[1] < dx) {
               x[1]=kmin(x[1]+delta,dx);
          }
          x[2]=x[1]+(sx[2]-sx[1]);
          if (x[1] == dx) {
               retval|=1;
          }
          dx=x[5];
          if (x[4] > dx) {
               x[4]=kmax(x[4]-delta,dx);
          }
          else if (x[4] < dx) {
               x[4]=kmin(x[4]+delta,dx);
          }
          x[3]=x[4]+(sx[3]-sx[4]);
          if (x[4] == dx) {
               retval|=2;
          }
          dx=x[6];
          if (x[7] > dx) {
               x[7]=kmax(x[7]-delta,dx);
          }
          else if (x[7] < dx) {
               x[7]=kmin(x[7]+delta,dx);
          }
          x[8]=x[7]+(sx[8]-sx[7]);
          if (x[7] == dx) {
               retval|=4;
          }
          dx=x[11];
          if (x[10] > dx) {
               x[10]=kmax(x[10]-delta,dx);
          }
          else if (x[10] < dx) {
               x[10]=kmin(x[10]+delta,dx);
          }
          x[9]=x[10]+(sx[9]-sx[10]);
          if (x[10] == dx) {
               retval|=8;
          }
     }
     else {
          dx=y[0];
          if (y[1] > dx) {
               y[1]=kmax(y[1]-delta,dx);
          }
          else if (y[1] < dx) {
               y[1]=kmin(y[1]+delta,dx);
          }
          y[2]=y[1]+(sy[2]-sy[1]);
          if (y[1] == dx) {
               retval|=1;
          }
          dx=y[5];
          if (y[4] > dx) {
               y[4]=kmax(y[4]-delta,dx);
          }
          else if (y[4] < dx) {
               y[4]=kmin(y[4]+delta,dx);
          }
          y[3]=y[4]+(sy[3]-sy[4]);
          if (y[4] == dx) {
               retval|=2;
          }
          dx=y[6];
          if (y[7] > dx) {
               y[7]=kmax(y[7]-delta,dx);
          }
          else if (y[7] < dx) {
               y[7]=kmin(y[7]+delta,dx);
          }
          y[8]=y[7]+(sy[8]-sy[7]);
          if (y[7] == dx) {
               retval|=4;
          }
          dx=y[11];
          if (y[10] > dx) {
               y[10]=kmax(y[10]-delta,dx);
          }
          else if (y[10] < dx) {
               y[10]=kmin(y[10]+delta,dx);
          }
          y[9]=y[10]+(sy[9]-sy[10]);
          if (y[10] == dx) {
               retval|=8;
          }
     }
     dragpoint(w[1],x[1],y[1]);
     dragpoint(w[2],x[2],y[2]);
     dragpoint(w[3],x[3],y[3]);
     dragpoint(w[4],x[4],y[4]);
     dragpoint(w[7],x[7],y[7]);
     dragpoint(w[8],x[8],y[8]);
     dragpoint(w[9],x[9],y[9]);
     dragpoint(w[10],x[10],y[10]);
     if (retval == (1|2|4|8)) {
          return(1);
     }
     return(0);
}

int
EFF_slideDoorClose(short d)
{
     short i,retval=0,w[12];
     long delta,dx,sx[12],sy[12],x[12],y[12];
     struct doorData *dptr;

     dptr=doorPtr[d];
     delta=dptr->speed;
     for (i=0 ; i < 12 ; i++) {
          w[i]=dptr->wallIndex[i];
          x[i]=wallPtr[w[i]]->x;
          y[i]=wallPtr[w[i]]->y;
          sx[i]=dptr->wallx[i];
          sy[i]=dptr->wally[i];
     }
     if (sy[1] == sy[0]) {
          dx=sx[1];
          if (x[1] > dx) {
               x[1]=kmax(x[1]-delta,dx);
          }
          else if (x[1] < dx) {
               x[1]=kmin(x[1]+delta,dx);
          }
          x[2]=x[1]+(sx[2]-sx[1]);
          if (x[1] == dx) {
               retval|=1;
          }
          dx=sx[4];
          if (x[4] > dx) {
               x[4]=kmax(x[4]-delta,dx);
          }
          else if (x[4] < dx) {
               x[4]=kmin(x[4]+delta,dx);
          }
          x[3]=x[4]+(sx[3]-sx[4]);
          if (x[4] == dx) {
               retval|=2;
          }
          dx=sx[7];
          if (x[7] > dx) {
               x[7]=kmax(x[7]-delta,dx);
          }
          else if (x[7] < dx) {
               x[7]=kmin(x[7]+delta,dx);
          }
          x[8]=x[7]+(sx[8]-sx[7]);
          if (x[7] == dx) {
               retval|=4;
          }
          dx=sx[10];
          if (x[10] > dx) {
               x[10]=kmax(x[10]-delta,dx);
          }
          else if (x[10] < dx) {
               x[10]=kmin(x[10]-delta,dx);
          }
          x[9]=x[10]+(sx[9]-sx[10]);
          if (x[10] == dx) {
               retval|=8;
          }
     }
     else {
          dx=sy[1];
          if (y[1] > dx) {
               y[1]=kmax(y[1]-delta,dx);
          }
          else if (y[1] < dx) {
               y[1]=kmin(y[1]+delta,dx);
          }
          y[2]=y[1]+(sy[2]-sy[1]);
          if (y[1] == dx) {
               retval|=1;
          }
          dx=sy[4];
          if (y[4] > dx) {
               y[4]=kmax(y[4]-delta,dx);
          }
          else if (y[4] < dx) {
               y[4]=kmin(y[4]+delta,dx);
          }
          y[3]=y[4]+(sy[3]-sy[4]);
          if (y[4] == dx) {
               retval|=2;
          }
          dx=sy[7];
          if (y[7] > dx) {
               y[7]=kmax(y[7]-delta,dx);
          }
          else if (y[7] < dx) {
               y[7]=kmin(y[7]+delta,dx);
          }
          y[8]=y[7]+(sy[8]-sy[7]);
          if (y[7] == dx) {
               retval|=4;
          }
          dx=sy[10];
          if (y[10] > dx) {
               y[10]=kmax(y[10]-delta,dx);
          }
          else if (y[10] < dx) {
               y[10]=kmin(y[10]+delta,dx);
          }
          y[9]=y[10]+(sy[9]-sy[10]);
          if (y[10] == dx) {
               retval|=8;
          }
     }
     dragpoint(w[1],x[1],y[1]);
     dragpoint(w[2],x[2],y[2]);
     dragpoint(w[3],x[3],y[3]);
     dragpoint(w[4],x[4],y[4]);
     dragpoint(w[7],x[7],y[7]);
     dragpoint(w[8],x[8],y[8]);
     dragpoint(w[9],x[9],y[9]);
     dragpoint(w[10],x[10],y[10]);
     if (retval == (1|2|4|8)) {
          return(1);
     }
     return(0);
}

int
EFF_liveActorInSector(short s)
{
     short i,j;
     
     i=headspritesect[s];
     while (i != -1) {   // check for living objects in door sector
          j=nextspritesect[i];
          if (ACT_getHealth(i) > 0) {
               return(1);
          }
          i=j;
     }
     return(0);
}

void
EFF_moveDoor(short d,struct doorData *dptr)
{
     int  i,j,rotang,s;
     long beforez,goalz,deltaz,newx,newy,speed;
     sectortype *sect;
     spritetype *spr;

     s=dptr->sectorIndex;
     sect=sectorPtr[s];
     speed=dptr->speed*TICWAITPERFRAME;
     switch (sect->lotag) {
     case SEC_DOORUPTAG:
     case SEC_DOORUPONCETAG:
          switch (dptr->state) {
          case DOORSTATE_OPEN:
               if (EFF_moveSectorCeiling(s,sect,speed,dptr->hiz)) {
                    dptr->state=DOORSTATE_OPENED;
               }
               break;
          case DOORSTATE_CLOSE:
               if (EFF_moveSectorCeiling(s,sect,speed,dptr->loz)) {
                    dptr->state=DOORSTATE_CLOSED;
               }
               break;
          }
          break;
     case SEC_DOORDOWNTAG:
     case SEC_DOORDOWNONCETAG:
     case SEC_PLATFORMDOWNTAG:
     case SEC_PLATFORMELEVATORTAG:
          switch (dptr->state) {
          case DOORSTATE_OPEN:
               beforez=sect->floorz;
               if (EFF_moveSectorFloor(s,sect,speed,dptr->loz)) {
                    dptr->state=DOORSTATE_OPENED;
               }
               if (EFF_isElevator(s)) {
                    deltaz=sect->floorz-beforez;
                    for (i=headspritesect[s] ; i >= 0 ; i=j) {
                         j=nextspritesect[i];
                         spr=spritePtr[i];
                         if (GAM_isOnGround(i)) {
                              spr->z+=deltaz;
                              setsprite(i,spr->x,spr->y,spr->z);
                         }
                    }
                    if (sect->lotag == SEC_PLATFORMELEVATORTAG) {
                         sect->ceilingz=sect->floorz-dptr->height;
                    }
               }
               break;
          case DOORSTATE_CLOSE:
               beforez=sect->floorz;
               if (EFF_moveSectorFloor(s,sect,speed,dptr->hiz)) {
                    dptr->state=DOORSTATE_CLOSED;
               }
               if (EFF_isElevator(s)) {
                    deltaz=sect->floorz-beforez;
                    for (i=headspritesect[s] ; i >= 0 ; i=j) {
                         j=nextspritesect[i];
                         spr=spritePtr[i];
                         if (GAM_isOnGround(i)) {
                              spr->z+=deltaz;
                              setsprite(i,spr->x,spr->y,spr->z);
                         }
                    }
                    if (sect->lotag == SEC_PLATFORMELEVATORTAG) {
                         sect->ceilingz=sect->floorz-dptr->height;
                    }
               }
               break;
          }
          break;
     case SEC_PLATFORMUPTAG:
          switch (dptr->state) {
          case DOORSTATE_OPEN:
               beforez=sect->floorz;
               if (EFF_moveSectorFloor(s,sect,speed,dptr->hiz)) {
                    dptr->state=DOORSTATE_OPENED;
               }
               deltaz=sect->floorz-beforez;
               for (i=headspritesect[s] ; i >= 0 ; i=j) {
                    j=nextspritesect[i];
                    spr=spritePtr[i];
                    if (GAM_isOnGround(i)) {
                         spr->z+=deltaz;
                         setsprite(i,spr->x,spr->y,spr->z);
                    }
               }
               break;
          case DOORSTATE_CLOSE:
               beforez=sect->floorz;
               if (EFF_moveSectorFloor(s,sect,speed,dptr->loz)) {
                    dptr->state=DOORSTATE_CLOSED;
               }
               deltaz=sect->floorz-beforez;
               for (i=headspritesect[s] ; i >= 0 ; i=j) {
                    j=nextspritesect[i];
                    spr=spritePtr[i];
                    if (GAM_isOnGround(i)) {
                         spr->z+=deltaz;
                         setsprite(i,spr->x,spr->y,spr->z);
                    }
               }
               break;
          }
          break;
     case SEC_DOORHSPLITTAG:
     case SEC_DOORHSPLITONCETAG:
          switch (dptr->state) {
          case DOORSTATE_OPEN:
               i=0;
               if (EFF_moveSectorCeiling(s,sect,speed,dptr->hiz)) {
                    i=1;
               }
               if (EFF_moveSectorFloor(s,sect,speed,dptr->loz)) {
                    i|=2;
               }
               if (i == 3) {
                    dptr->state=DOORSTATE_OPENED;
               }
               break;
          case DOORSTATE_CLOSE:
               goalz=sectorCenterPtr[s]->centerz;
               i=0;
               if (EFF_moveSectorCeiling(s,sect,speed,goalz)) {
                    i=1;
               }
               if (EFF_moveSectorFloor(s,sect,speed,goalz)) {
                    i|=2;
               }
               if (i == 3) {
                    dptr->state=DOORSTATE_CLOSED;
               }
               break;
          }
          break;
     case SEC_DOORSLIDECENTERTAG:
          switch (dptr->state) {
          case DOORSTATE_OPEN:
               if (EFF_slideDoorOpen(d)) {
                    dptr->state=DOORSTATE_OPENED;
               }
               break;
          case DOORSTATE_CLOSE:
               if (EFF_slideDoorClose(d)) {
                    dptr->state=DOORSTATE_CLOSED;
               }
               break;
          }
          break;
     case SEC_ROTATESECTORTAG:
     case SEC_BONUSSECTORTAG:
          i=0;
          rotang=dptr->hiz-dptr->loz;
          dptr->loz=dptr->hiz;
          while ((j=dptr->wallIndex[i]) >= 0) {
               rotatepoint(sectorCenterPtr[s]->centerx,
                           sectorCenterPtr[s]->centery,
                           dptr->wallx[i],dptr->wally[i],
                           dptr->loz,&newx,&newy);
               dragpoint(j,newx,newy);
               i++;
          }
          for (i=headspritesect[s] ; i >= 0 ; i=j) {
               j=nextspritesect[i];
               if (!GAM_isOnGround(i)) {
                    continue;
               }
               if (rotang != 0) {
                    spritePtr[i]->ang=(spritePtr[i]->ang+rotang)&2047;
                    rotatepoint(sectorCenterPtr[s]->centerx,
                                sectorCenterPtr[s]->centery,
                                spritePtr[i]->x,spritePtr[i]->y,
                                rotang&2047,&newx,&newy);
                    setsprite(i,newx,newy,spritePtr[i]->z);
               }
          }
          dptr->hiz=(dptr->hiz+dptr->speed)&2047;
          break;
     }
//** play sounds here
     switch (dptr->state) {
     case DOORSTATE_OPEN:
     case DOORSTATE_CLOSE:
          SND_playSectorMovingSound(s,1);
          if (EFF_isDoor(s) && EFF_liveActorInSector(s)) {
               dptr->state=DOORSTATE_OPEN;
          }
          break;
     case DOORSTATE_OPENED:
          switch (sect->lotag) {
          case SEC_DOORUPONCETAG:
          case SEC_DOORDOWNONCETAG:
          case SEC_DOORHSPLITONCETAG:
               dptr->state=DOORSTATE_IDLE;
               EFF_removeMovingDoor(d,dptr);
               break;
          default:
               dptr->state=DOORSTATE_WAITING;
               dptr->delay=TMR_getSecondTics(4);
               break;
          }
          SND_playSectorMovingSound(s,0);
          SND_playSectorStopSound(s);
          break;
     case DOORSTATE_CLOSED:
          dptr->state=DOORSTATE_IDLE;
          EFF_removeMovingDoor(d,dptr);
          EFF_blockDoorWalls(s);
          SND_playSectorMovingSound(s,0);
          SND_playSectorStopSound(s);
          break;
     case DOORSTATE_WAITING:
          if (dptr->movingSectorFlag == 0) {
               dptr->delay-=TICWAITPERFRAME;
               if (dptr->delay <= 0) {
                    dptr->delay=0;
                    if (EFF_isDoor(s) && EFF_liveActorInSector(s)) {
                         break;
                    }
                    dptr->state=DOORSTATE_CLOSE;
                    EFF_turnSwitchesOff(sect->hitag);
                    SND_playSectorMovingSound(s,0);
                    SND_playSectorCloseSound(s);
               }
          }
          break;
     }
}

void
EFF_doMoveDoors(void)
{
     short d,i,n;

     n=numMovingDoors;
     for (i=0 ; i < n ; i++) {
          d=movingDoor[i];
          if (d >= 0) {
               EFF_moveDoor(d,doorPtr[d]);
          }
     }
}

void
EFF_openDoor(short s)
{
     short d;
     struct doorData *dptr;
     
     if ((d=doorIndex[s]) == -1) {
          return;
     }
     dptr=doorPtr[d];
     EFF_addMovingDoor(d,dptr);
     dptr->state=DOORSTATE_OPEN;
     EFF_unblockDoorWalls(s);
     SND_playSectorOpenSound(s);
}

void
EFF_closeDoor(short s)
{
     short d;
     struct doorData *dptr;
     
     if ((d=doorIndex[s]) == -1) {
          return;
     }
     dptr=doorPtr[d];
     EFF_addMovingDoor(d,dptr);
     dptr->state=DOORSTATE_CLOSE;
     EFF_blockDoorWalls(s);
     SND_playSectorCloseSound(s);
}

void
EFF_operateDoor(short s)
{
     short d,key;

     if ((d=doorIndex[s]) == -1) {
          return;
     }
     switch (sectorPtr[s]->lotag) {
     case SEC_ROTATESECTORTAG:
     case SEC_BONUSSECTORTAG:
          return;
     }
     switch (doorPtr[d]->state) {
     case DOORSTATE_IDLE:
     case DOORSTATE_CLOSE:
     case DOORSTATE_CLOSED:
          if (EFF_checkSwitches(sectorPtr[s]->hitag,&key)) {
               if (key == EFF_getDoorLock(s)) {
                    EFF_openDoor(s);
               }
          }
          break;
     case DOORSTATE_OPEN:
     case DOORSTATE_OPENED:
     case DOORSTATE_WAITING:
          EFF_closeDoor(s);
          break;
     }
}

void
EFF_openMovingSectorDoors(short s)
{
     short i,sectnum;
     struct movingSectorData *mptr;

     mptr=movingSectorPtr[movingSectorIndex[s]];
     for (i=0 ; i < mptr->numSectors ; i++) {
          sectnum=mptr->sector[i];
          if (EFF_operatableSector(sectnum)) {
               EFF_openDoor(sectnum);
          }
     }
}

void
EFF_closeMovingSectorDoors(short s)
{
     short i,sectnum;
     struct movingSectorData *mptr;

     mptr=movingSectorPtr[movingSectorIndex[s]];
     for (i=0 ; i < mptr->numSectors ; i++) {
          sectnum=mptr->sector[i];
          if (EFF_operatableSector(sectnum)) {
               EFF_closeDoor(sectnum);
          }
     }
}

void
EFF_operateMovingSector(short s)
{
     short i;
     struct movingSectorData *mptr;

     if ((i=movingSectorIndex[s]) == -1) {
          return;
     }
     mptr=movingSectorPtr[i];
     if (mptr->active) {
          mptr->goalSpeed=0;
          EFF_openMovingSectorDoors(s);
     }
     else {
          mptr->active=1;
          mptr->goalSpeed=mptr->maxSpeed;
          EFF_turnSwitchesOn(sectorPtr[s]->hitag);
          EFF_closeMovingSectorDoors(s);
     }
}

void
EFF_operateSector(short s)
{
     short lo;

     lo=sectorPtr[s]->lotag;
     if (lo > 0) {
          if (EFF_operatableSector(s)) {
               EFF_operateDoor(s);
          }
          else if (EFF_isMovingSector(s)) {
               EFF_operateMovingSector(s);
          }
     }
}

//**
//** Miscellaneous routines code
//**

void
EFF_showMessages(void)
{
     int  i,j;
     static int cycle;
     struct message *msg;
     
     j=0;
     for (i=0 ; i < MAXMESSAGES ; i++) {
          if (messagePtr[i]->delay <= 0) {
               continue;
          }
          msg=messagePtr[j];
          if (i != j) {
               memmove(msg,messagePtr[i],sizeof(struct message));
               messagePtr[i]->delay=0;
          }
          msg->delay-=TICWAITPERFRAME;
          if (msg->delay > 0) {
               if (qsetmode == 200L) {
                    printext256(windowx1,windowy2-((j+1)<<3),
                                PMSGCOLOR-(cycle&15)-i,
                                -1,msg->buf,0);
               }
          }
          j++;
     }
     cycle++;
}

void
EFF_displayMessage(char *fmt,...)
{
     int  nextMessage;
     char locbuf[MESSAGESIZE];
     va_list argptr;

     for (nextMessage=0 ; nextMessage <= MAXMESSAGES-1 ; nextMessage++) {
          if (messagePtr[nextMessage]->delay <= 0) {
               break;
          }
     }
     if (nextMessage == MAXMESSAGES) {
          for (nextMessage=1 ; nextMessage < MAXMESSAGES ; nextMessage++) {
               memmove(messagePtr[nextMessage-1],messagePtr[nextMessage],
                       sizeof(struct message));
          }
          nextMessage=MAXMESSAGES-1;
     }
     va_start(argptr,fmt);
     vsprintf(locbuf,fmt,argptr);
     va_end(argptr);
     strncpy(messagePtr[nextMessage]->buf,locbuf,MESSAGESIZE-1);
     messagePtr[nextMessage]->delay=TMR_getSecondTics(4);
}

//**
//** Sprite & wall switches code
//**

void
EFF_spriteSwitchOff(short s)
{
     if (EFF_getSpriteSwitchStatus(s)) {
          EFF_setSpriteSwitchStatus(s,SWITCH_OFF);
          spritePtr[s]->picnum--;
     }
}

void
EFF_spriteSwitchOn(short s)
{
     if (!EFF_getSpriteSwitchStatus(s)) {
          EFF_setSpriteSwitchStatus(s,SWITCH_ON);
          spritePtr[s]->picnum++;
     }
}

void
EFF_wallSwitchOff(short w)
{
     if (EFF_getWallSwitchStatus(w)) {
          EFF_setWallSwitchStatus(w,SWITCH_OFF);
          wallPtr[w]->picnum--;
     }
}

void
EFF_wallSwitchOn(short w)
{
     if (!EFF_getWallSwitchStatus(w)) {
          EFF_setWallSwitchStatus(w,SWITCH_ON);
          wallPtr[w]->picnum++;
     }
}

void
EFF_turnSwitchesOn(short hitag)
{
     short i,s,w;

     i=0;
     while ((w=hitagWallIndex[i++]) >= 0) {
          if (EFF_isWallSwitch(w)) {
               if (wallPtr[w]->hitag == hitag) {
                    EFF_wallSwitchOn(w);
               }
          }
     }
     i=0;
     while ((s=hitagSpriteIndex[i++]) >= 0) {
          if (EFF_isSpriteSwitch(s)) {
               if (spritePtr[s]->hitag == hitag) {
                    EFF_spriteSwitchOn(s);
               }
          }
     }
}

void
EFF_turnSwitchesOff(short hitag)
{
     short i,s,w;

     i=0;
     while ((w=hitagWallIndex[i++]) >= 0) {
          if (EFF_isWallSwitch(w)) {
               if (wallPtr[w]->hitag == hitag) {
                    EFF_wallSwitchOff(w);
               }
          }
     }
     i=0;
     while ((s=hitagSpriteIndex[i++]) >= 0) {
          if (EFF_isSpriteSwitch(s)) {
               if (spritePtr[s]->hitag == hitag) {
                    EFF_spriteSwitchOff(s);
               }
          }
     }
}

void
EFF_operateWalkthruWallTags(short w)
{
     short s;

     s=sectorofwall(w);
     switch (wallPtr[w]->lotag) {
     case WAL_W1LIGHTSOFFTAG:
          EFF_sectorLightsOff(s);
          EFF_turnSwitchesOff(wallPtr[w]->hitag);
          wallPtr[w]->lotag=0;
          break;
     case WAL_WRLIGHTSOFFTAG:
          EFF_sectorLightsOff(s);
          EFF_turnSwitchesOff(wallPtr[w]->hitag);
          break;
     case WAL_W1LIGHTSONFULLTAG:
          EFF_sectorLightsOn(s);
          EFF_turnSwitchesOn(wallPtr[w]->hitag);
          wallPtr[w]->lotag=0;
          break;
     case WAL_WRLIGHTSONFULLTAG:
          EFF_sectorLightsOn(s);
          EFF_turnSwitchesOn(wallPtr[w]->hitag);
          break;
     }
}

void
EFF_operateSwitchedWallTags(short w)
{
     short s;

     s=sectorofwall(w);
     switch (wallPtr[w]->lotag) {
     case WAL_SRLIGHTSOFFTAG:
          EFF_sectorLightsOff(s);
          EFF_turnSwitchesOff(wallPtr[w]->hitag);
          break;
     case WAL_SRLIGHTSONFULLTAG:
          EFF_sectorLightsOn(s);
          EFF_turnSwitchesOn(wallPtr[w]->hitag);
          break;
     }
}

void
EFF_operateThings(short hitag)
{
     short i,w;

     for (i=0 ; i < numsectors ; i++) {
          if (sectorPtr[i]->hitag == hitag) {
               EFF_operateSector(i);
          }
     }
     for (i=0 ; i < MAXLIGHTEFFECTS ; i++) {
          if ((w=lightWallIndex[i]) == -1) {
               break;
          }
          if (wallPtr[w]->hitag != hitag) {
               continue;
          }
          EFF_operateSwitchedWallTags(w);
     }
}

void
EFF_operateSprite(short s)
{
     short hi;
     spritetype *spr;

     spr=spritePtr[s];
     switch (spr->lotag) {
     case SPR_SWITCHTAG:
          hi=spr->hitag;
          if (hi > 0) {
               if (EFF_getSpriteSwitchStatus(s)) {
                    EFF_spriteSwitchOff(s);
                    SND_playSwitchOffSound(s);
               }
               else {
                    EFF_spriteSwitchOn(s);
                    SND_playSwitchOnSound(s);
               }
               if (hi == SWITCH_NEXTLEVEL) {
                    if (editorEnabledFlag) {
                         EFF_displayMessage("NO LEVEL CHANGE WHILE EDITING");
                    }
                    else {
                         GAM_nextLevel();
                    }
               }
               else {
                    EFF_operateThings(hi);
               }
          }
          break;
     }
}

void
EFF_operateWall(short w)
{
     short hi,i;
     walltype *wal;

     wal=wallPtr[w];
     switch (wal->lotag) {
     case WAL_SWITCHTAG:
          hi=wal->hitag;
          if (hi > 0) {
               if (EFF_getWallSwitchStatus(w)) {
                    EFF_wallSwitchOff(w);
                    SND_playWallSwitchOffSound(w);
               }
               else {
                    EFF_wallSwitchOn(w);
                    SND_playWallSwitchOnSound(w);
               }
               if (hi == SWITCH_NEXTLEVEL) {
                    if (editorEnabledFlag) {
                         EFF_displayMessage("NO LEVEL CHANGE WHILE EDITING");
                    }
                    else {
                         GAM_nextLevel();
                    }
               }
               else {
                    for (i=0 ; i < numsectors ; i++) {
                         if (sectorPtr[i]->hitag == hi) {
                              EFF_operateSector(i);
                         }
                    }
               }
          }
          break;
     }
}

void
EFF_doEnterSectorTags(short snum,short *s,short *ang,long *x,long *y,long *z)
{
     short clipdist,endwall,playerHeight,startwall,w;
     sectortype *sect;
     spritetype *spr;

     spr=spritePtr[snum];
     sect=sectorPtr[*s];
     startwall=sect->wallptr;
     endwall=startwall+sect->wallnum;
     playerHeight=ACT_getActorViewHeight(spr);
     clipdist=ACT_getActorClipDist(spr);
     for (w=startwall ; w < endwall ; w++) {
          switch (wallPtr[w]->lotag) {
          case WAL_TRIGGERTAG:
               EFF_operateThings(wallPtr[w]->hitag);
               break;
          case WAL_WARPTAG:
               EFF_warpSprite(w,snum,ang,s,x,y,z);
               *z-=playerHeight;
               spr->cstat&=~(SPRC_BLOCKING);
               getzrange(*x,*y,*z,*s,&globhiz,&globhihit,&globloz,
                         &globlohit,clipdist,0);
               spr->cstat|=(SPRC_BLOCKING);
               break;
          default:
               if (wallPtr[w]->lotag > 0) {
                    EFF_operateWalkthruWallTags(w);
               }
               break;
          }
     }
     if (PLR_isPlayer(spr,-1)) {
          switch (sect->lotag) {
          case SEC_SECRETTAG:
               sect->lotag=SEC_NORMAL;
               if (GAM_isViewSprite(snum)) {
                    EFF_displayMessage("SECRET ROOM");
               }
               break;
          }
     }
}

void
EFF_doLeaveSectorTags(short snum,short *s,short *ang,long *x,long *y,long *z)
{
}

void
EFF_spawnBlood(short s,short shooter,short damage)
{
     short i,j,n;
     spritetype *spr,*src,*trg;

     if (bloodCount >= MAXBLOODCOUNT) {
          return;
     }
     trg=spritePtr[s];
     src=spritePtr[shooter];
     n=((damage>>8)+1)&7;
     for (i=0 ; i < n ; i++) {
          if ((j=goreSprite[goreCount]) >= 0) {
               if (spritePtr[j]->statnum == STAT_NONE) {
                    changespritestat(j,STAT_BLOOD);
               }
               else {
                    continue;
               }
          }
          else {
               j=ENG_insertsprite(trg->sectnum,STAT_BLOOD);
               goreSprite[goreCount]=j;
          }
          goreCount=(goreCount+1)&(MAXGORECOUNT-1);
          spr=spritePtr[j];
          GAM_onGround(j,0);
          spr->picnum=goreChunksBegPic+(krand()%
                      ((goreChunksEndPic-goreChunksBegPic)+1));
          spr->x=trg->x;
          spr->y=trg->y;
          spr->z=trg->z-(trg->yrepeat<<7);
          spr->xrepeat=48;
          spr->yrepeat=48;
          spr->clipdist=2;
          spr->ang=getangle(trg->x-src->x,trg->y-src->y);
          if (krand()&1) {
               spr->ang-=(krand()&255);
          }
          else {
               spr->ang+=(krand()&255);
          }
          spr->xvel=(krand()&127);
          spr->yvel=(krand()&127);
          spr->zvel=-(krand()&2047);
          spr->owner=s;
          switch (GAM_getGoreLevel()) {
          case 0:
               spr->cstat|=SPRC_INVISIBLE;
               break;
          }
          bloodCount++;
     }
}

//**
//** Status code
//**

void
EFF_doStatusBlinkOff(short i,short s)
{
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     si->clock-=TICWAITPERFRAME;
     if (si->clock > 0) {
          return;
     }
     switch (si->status) {
     case 1:
          si->clock+=TMR_getSecondFraction(TICS_ONEHALF);
          EFF_sectorLightsOn(s);
          break;
     default:
          si->clock+=TMR_getSecondFraction(TICS_ONEEIGHTH);
          EFF_sectorLightsOff(s);
          break;
     }
     si->status^=1;
}

void
EFF_doStatusBlinkOn(short i,short s)
{
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     si->clock-=TICWAITPERFRAME;
     if (si->clock > 0) {
          return;
     }
     switch (si->status) {
     case 1:
          si->clock+=TMR_getSecondFraction(TICS_ONEHALF);
          EFF_sectorLightsOff(s);
          break;
     default:
          si->clock+=TMR_getSecondFraction(TICS_ONEEIGHTH);
          EFF_sectorLightsOn(s);
          break;
     }
     si->status^=1;
}

void
EFF_doStatusBlinkOn1s(short i,short s)
{
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     si->clock-=TICWAITPERFRAME;
     if (si->clock > 0) {
          return;
     }
     switch (si->status) {
     case 1:
          si->clock+=TMR_getSecondTics(1);
          EFF_sectorLightsOff(s);
          break;
     default:
          si->clock+=TMR_getSecondFraction(TICS_ONEEIGHTH);
          EFF_sectorLightsOn(s);
          break;
     }
     si->status^=1;
}

void
EFF_doStatusOscillate(short i,short s)
{
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     switch (si->status) {
     default:
          if (EFF_adjSectorLight(s,TICWAITPERFRAME) >= 239) {
               EFF_setSectorLight(s,239);
               si->status^=1;
          }
          break;
     case 1:
          if (EFF_adjSectorLight(s,-TICWAITPERFRAME) <= 0) {
               EFF_setSectorLight(s,0);
               si->status^=1;
          }
          break;
     }
}

void
EFF_doStatusSyncBlinkOn(short i,short s)
{
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     if (totalclock < si->clock) {
          return;
     }
     switch (si->status) {
     case 1:
          si->clock=totalclock+TMR_getSecondFraction(TICS_ONEHALF);
          EFF_sectorLightsOff(s);
          break;
     default:
          si->clock=totalclock+TMR_getSecondFraction(TICS_ONEEIGHTH);
          EFF_sectorLightsOn(s);
          break;
     }
     si->status^=1;
}

void
EFF_doStatusSyncBlinkOff(short i,short s)
{
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     if (totalclock < si->clock) {
          return;
     }
     switch (si->status) {
     case 1:
          si->clock=totalclock+TMR_getSecondFraction(TICS_ONEHALF);
          EFF_sectorLightsOn(s);
          break;
     default:
          si->clock=totalclock+TMR_getSecondFraction(TICS_ONEEIGHTH);
          EFF_sectorLightsOff(s);
          break;
     }
     si->status^=1;
}

void
EFF_doStatusFlicker(short i,short s)
{
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     si->clock-=TICWAITPERFRAME;
     if (si->clock > 0) {
          return;
     }
     switch (si->status) {
     case 1:
          si->clock+=krand()%TMR_getSecondFraction(TICS_ONEEIGHTH);
          EFF_sectorLightsOff(s);
          break;
     default:
          si->clock+=krand()%TMR_getSecondFraction(TICS_ONEEIGHTH);
          EFF_sectorLightsOn(s);
          break;
     }
     si->status=krand()&1;
}

void
EFF_doStatusPanCeil(short i,short s)
{
     long xvect,yvect;
     struct sectorEffect *si;

     si=&sectorEffectStruct[i];
     xvect=mulscale12(si->speed,sintable[(si->ang+512)&2047]);
     yvect=mulscale12(si->speed,sintable[si->ang]);
     sectorPtr[s]->ceilingxpanning-=xvect;
     sectorPtr[s]->ceilingypanning+=yvect;
}

void
EFF_doStatusPanFloor(short i,short s)
{
     short oldsect,snum;
     long deltaz,oldloz,xvect,yvect;
     struct sectorEffect *si;
     spritetype *spr;
     sectortype *sectPtr;

     si=&sectorEffectStruct[i];
     xvect=(si->speed*sintable[(si->ang+512)&2047]);
     yvect=(si->speed*sintable[si->ang]);
     sectPtr=sectorPtr[s];
     sectPtr->floorxpanning-=(xvect>>12);
     sectPtr->floorypanning+=(yvect>>12);
     for (snum=headspritesect[s] ; snum >= 0 ; snum=nextspritesect[snum]) {
          if (GAM_isOnGround(snum)) {
               spr=spritePtr[snum];
               oldsect=spr->sectnum;
               oldloz=getflorzofslope(oldsect,spr->x,spr->y);
               movesprite(snum,xvect<<4,yvect<<4,0L,4L<<8,4L<<8,0);
               if (spr->sectnum != oldsect) {
                    ACT_newSector(snum,&oldsect,&spr->sectnum,&spr->ang,
                                  &spr->x,&spr->y,&spr->z);
               }
               deltaz=getflorzofslope(spr->sectnum,spr->x,spr->y)-oldloz;
               if (deltaz != 0L) {
                    spr->z+=deltaz;
               }
               setsprite(snum,spr->x,spr->y,spr->z);
          }
     }
}

void
EFF_doStatusEffects(void)
{
     short i,s;
     struct sectorEffect *si;

     i=0;
     while ((s=sectorEffectIndex[i]) >= 0) {
          si=&sectorEffectStruct[i];
          switch (si->effectTag) {
          case SEC_BLINKOFFTAG:
               EFF_doStatusBlinkOff(i,s);
               break;
          case SEC_BLINKONTAG:
          case SEC_HIDAMAGEBLINKTAG:
               EFF_doStatusBlinkOn(i,s);
               break;
          case SEC_BLINKON1STAG:
               EFF_doStatusBlinkOn1s(i,s);
               break;
          case SEC_OSCILLATETAG:
               EFF_doStatusOscillate(i,s);
               break;
          case SEC_SYNCBLINKONTAG:
               EFF_doStatusSyncBlinkOn(i,s);
               break;
          case SEC_SYNCBLINKOFFTAG:
               EFF_doStatusSyncBlinkOff(i,s);
               break;
          case SEC_FLICKERTAG:
               EFF_doStatusFlicker(i,s);
               break;
          case SEC_PANCEILTAG:
               EFF_doStatusPanCeil(i,s);
               break;
          case SEC_PANFLOORTAG:
               EFF_doStatusPanFloor(i,s);
               break;
          }
          i++;
     }
     if (lastDamageClock > 0) {
          lastDamageClock-=TICWAITPERFRAME;
     }
     else {
          lastDamageClock=TMR_getSecondTics(1);
     }
}

void
EFF_doStatusDieAnim(short i)
{
     long locloz;
     spritetype *spr;

     spr=spritePtr[i];
     if ((sectorPtr[spr->sectnum]->floorstat&0x02) != 0) {
          locloz=getflorzofslope(spr->sectnum,spr->x,spr->y);
     }
     else {
          locloz=sectorPtr[spr->sectnum]->floorz;
     }
     if (spr->z < locloz) {
          spr->zvel+=gravityConstant;
          spr->z+=spr->zvel;
     }
     if (spr->z >= locloz) {
          spr->z=locloz;
          spr->zvel=0;
     }
     if (spr->zvel == 0 && frames[i] == 0) {
          changespritestat(i,STAT_NONE);
     }
     setsprite(i,spr->x,spr->y,spr->z);
     frameDelay[i]-=TICWAITPERFRAME;
     if (frameDelay[i] <= 0) {
          frameDelay[i]=frameDelayReset[i];
          if (frames[i] > 0) {
               frames[i]--;
               spr->picnum++;
          }
     }
}

void
EFF_doStatusBlood(short i)
{
     int  o;
     long dax,day,daz;
     spritetype *spr;

     spr=spritePtr[i];
     dax=spr->xvel*sintable[(spr->ang+512)&2047];
     day=spr->yvel*sintable[spr->ang];
     daz=spr->zvel;
     if ((spr->cstat&SPRC_WALLSPRITE) != 0) {
          daz>>=2;
     }
     o=movesprite(i,dax,day,daz,spr->yrepeat<<4,spr->yrepeat<<4,0);
     if ((o&32768) == 32768) {
          spr->cstat|=SPRC_WALLSPRITE;
          spr->xvel=0;
          spr->yvel=0;
     }
     if ((o&16384) != 16384) {
          daz=spr->zvel+gravityConstant;
          if (daz > 32767L) {
               daz=32767L;
          }
          spr->zvel=daz;
     }
     else {
          GAM_onGround(i,1);
          spr->z=globloz;
          spr->xvel=0;
          spr->yvel=0;
          spr->zvel=0;
          if ((spr->cstat&SPRC_WALLSPRITE) == 0) {
               spr->cstat|=SPRC_FLOORSPRITE;
          }
          changespritestat(i,STAT_NONE);
          bloodCount--;
     }
     setsprite(i,spr->x,spr->y,spr->z);
}

void
EFF_doStatusAnims(void)
{
     short i,nexti;

     for (i=headspritestat[STAT_DIEANIM] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          EFF_doStatusDieAnim(i);
     }
     for (i=headspritestat[STAT_BLOOD] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          EFF_doStatusBlood(i);
     }
}

void
EFF_doStatusMovingSectors(void)
{
     short endwall,i,j,n,nexts,rotang=0,s,sectnum,snum,startwall,w;
     long dx,rotx,roty,rx,ry,x,xvect,y,yvect;
     struct movingSectorData *mptr;
     spritetype *spr,*spr2;
     sectortype *sect;

     for (i=0 ; i < numMovingSectors ; i++) {
          mptr=movingSectorPtr[i];
          snum=mptr->pivotSprite;
          spr=spritePtr[snum];
          if (mptr->active) {
               if (mptr->speed < mptr->goalSpeed) {
                    mptr->speed++;
               }
               else if (mptr->speed > mptr->goalSpeed) {
                    mptr->speed--;
               }
               if (mptr->targetSprite >= 0) {
                    spr2=spritePtr[mptr->targetSprite];
                    dx=WEP_getDistance(spr,spr2);
                    if (spr2->picnum == STOPSIGNPIC) {
                         s=kmax(dx>>8,4);
                         if (s < mptr->speed) {
                              mptr->speed=s;
                              mptr->goalSpeed=s;
                         }
                         if (dx <= 32) {
                              mptr->speed=0;
                              mptr->goalSpeed=0;
                              mptr->targetSprite=-1;
                              rotang=(spr2->ang-spr->ang);
                              if (rotang != 0) {
                                   rotx=spr->x;
                                   roty=spr->y;
                                   spr->ang=(spr->ang+rotang)&2047;
                              }
                              if (spr2->hitag > 0) {
                                   EFF_operateThings(spr2->hitag);
                              }
                         }
                         else {
                              rotang=getangle(spr2->x-spr->x,spr2->y-spr->y)-spr->ang;
                              if (rotang != 0) {
                                   rotx=spr->x;
                                   roty=spr->y;
                                   spr->ang=(spr->ang+rotang)&2047;
                              }
                         }
                    }
                    else if ((dx>>8) <= 0) {
                         mptr->targetSprite=-1;
                         rotang=spr2->ang-spr->ang;
                         if (rotang != 0) {
                              rotx=spr->x;
                              roty=spr->y;
                              spr->ang=(spr->ang+rotang)&2047;
                         }
                    }
                    else {
                         rotang=getangle(spr2->x-spr->x,spr2->y-spr->y)-spr->ang;
                         if (rotang != 0) {
                              rotx=spr->x;
                              roty=spr->y;
                              spr->ang=(spr->ang+rotang)&2047;
                         }
                    }
               }
          }
          if (mptr->speed > 0) {
               xvect=mulscale10(sintable[(spr->ang+512)&2047],mptr->speed);
               yvect=mulscale10(sintable[spr->ang],mptr->speed);
               x=spr->x+xvect;
               y=spr->y+yvect;
               setsprite(snum,x,y,spr->z);
               for (j=0,n=0 ; j < mptr->numSectors ; j++) {
                    sectnum=mptr->sector[j];
                    sect=sectorPtr[sectnum];
                    startwall=sect->wallptr;
                    endwall=startwall+sect->wallnum;
                    for (w=startwall ; w < endwall ; w++) {
                         x=spr->x-mptr->pointDeltaX[n];
                         y=spr->y-mptr->pointDeltaY[n];
                         switch (sectorPtr[mptr->sectnum]->lotag) {
                         case SEC_MOVINGSECTORTAG:
                         case SEC_AUTOMOVINGSECTORTAG:
                              rotatepoint(spr->x,spr->y,x,y,spr->ang,&rx,&ry);
                              break;
                         case SEC_MOVINGSECTORFIXEDTAG:
                         case SEC_AUTOMOVINGSECTORFIXEDTAG:
                              rx=x;
                              ry=y;
                              break;
                         }
                         dragpoint(w,rx,ry);
                         n++;
                    }
                    for (s=headspritesect[sectnum] ; s >= 0 ; s=nexts) {
                         nexts=nextspritesect[s];
                         if (s != snum) {
                              spr2=spritePtr[s];
                              if (rotang != 0) {
                                   spr2->ang=(spr2->ang+rotang)&2047;
                                   rotatepoint(rotx,roty,spr2->x,spr2->y,
                                               rotang&2047,&x,&y);
                                   setsprite(s,x,y,spr2->z);
                              }
                              x=spr2->x+xvect;
                              y=spr2->y+yvect;
                              setsprite(s,x,y,spr2->z);
                         }
                    }
               }
               if (mptr->targetSprite < 0) {
                    dx=0x7FFFFFFF;
                    s=headspritestat[STAT_MOVINGSECTORMARKER];
                    while (s >= 0) {
                         nexts=nextspritestat[s];
                         spr2=spritePtr[s];
                         if (WEP_canSeeRange(spr,spr2,512,dx)) {
                              mptr->targetSprite=s;
                              dx=WEP_getDistance(spr,spr2);
                         }
                         s=nexts;
                    }
               }
          }
          else if (mptr->active) {
               if (mptr->delay <= 0) {
                    mptr->active=0;
                    switch (sectorPtr[mptr->sectnum]->lotag) {
                    case SEC_AUTOMOVINGSECTORTAG:
                    case SEC_AUTOMOVINGSECTORFIXEDTAG:
                         mptr->delay=TMR_getSecondTics(8);
                         break;
                    default:
                         EFF_turnSwitchesOff(sectorPtr[mptr->sectnum]->hitag);
                         break;
                    }
               }
          }
          else if (mptr->delay > 0) {
               mptr->delay-=TICWAITPERFRAME;
               if (mptr->delay <= 0) {
                    mptr->delay=0;
                    mptr->active=1;
                    mptr->goalSpeed=mptr->maxSpeed;
                    EFF_closeMovingSectorDoors(mptr->sectnum);
               }
          }
     }
}

void
EFF_doStatusCode(void)
{
     EFF_doMoveDoors();
     EFF_doStatusEffects();
     EFF_doStatusAnims();
     EFF_doStatusMovingSectors();
}

//**
//** Load/Save game code
//**

void
EFF_saveGame(FILE *fp)
{
     short i,k;

     GAM_fwrite(&numDoors,sizeof(short),1,fp);
     for (i=0 ; i < numDoors ; i++) {
          GAM_fwrite(doorPtr[i],sizeof(struct doorData),1,fp);
     }
     GAM_fwrite(&numMovingSectors,sizeof(short),1,fp);
     for (i=0 ; i < numMovingSectors ; i++) {
          GAM_fwrite(movingSectorPtr[i],sizeof(struct movingSectorData),1,fp);
     }
     GAM_fwrite(movingSectorIndex,sizeof(short),MAXSECTORS,fp);
     GAM_fwrite(lightWallIndex,sizeof(short),MAXLIGHTEFFECTS,fp);
     GAM_fwrite(sectorEffectIndex,sizeof(short),MAXSECTOREFFECTS,fp);
     for (i=0 ; i < MAXSECTOREFFECTS ; i++) {
          if (sectorEffectIndex[i] >= 0) {
               GAM_fwrite(&sectorEffectStruct[i],sizeof(struct sectorEffect),
                          1,fp);
          }
     }
     GAM_fwrite(&lastDamageClock,sizeof(short),1,fp);
     for (k=STAT_HITSCANEXPLODE ; k <= STAT_DIEANIM ; k++) {
          for (i=headspritestat[k] ; i >= 0 ; i=nextspritestat[i]) {
               GAM_fwrite(&frames[i],sizeof(short),1,fp);
               GAM_fwrite(&frameDelay[i],sizeof(short),1,fp);
               GAM_fwrite(&frameDelayReset[i],sizeof(short),1,fp);
          }
     }
     GAM_fwrite(&bloodCount,sizeof(short),1,fp);
     GAM_fwrite(&goreCount,sizeof(short),1,fp);
     GAM_fwrite(goreSprite,sizeof(short),MAXGORECOUNT,fp);
     GAM_fwrite(wallSwitchStatus,sizeof(char),MAXWALLS>>3,fp);
     GAM_fwrite(spriteSwitchStatus,sizeof(char),MAXSPRITES>>3,fp);
}

void
EFF_loadGame(FILE *fp)
{
     short i,k;

     GAM_fread(&numDoors,sizeof(short),1,fp);
     for (i=0 ; i < numDoors ; i++) {
          GAM_fread(doorPtr[i],sizeof(struct doorData),1,fp);
          doorIndex[doorPtr[i]->sectorIndex]=i;
     }
     GAM_fread(&numMovingSectors,sizeof(short),1,fp);
     for (i=0 ; i < numMovingSectors ; i++) {
          GAM_fread(movingSectorPtr[i],sizeof(struct movingSectorData),1,fp);
     }
     GAM_fread(movingSectorIndex,sizeof(short),MAXSECTORS,fp);
     GAM_fread(lightWallIndex,sizeof(short),MAXLIGHTEFFECTS,fp);
     GAM_fread(sectorEffectIndex,sizeof(short),MAXSECTOREFFECTS,fp);
     for (i=0 ; i < MAXSECTOREFFECTS ; i++) {
          if (sectorEffectIndex[i] >= 0) {
               GAM_fread(&sectorEffectStruct[i],sizeof(struct sectorEffect),
                         1,fp);
          }
     }
     GAM_fread(&lastDamageClock,sizeof(short),1,fp);
     for (k=STAT_HITSCANEXPLODE ; k <= STAT_DIEANIM ; k++) {
          for (i=headspritestat[k] ; i >= 0 ; i=nextspritestat[i]) {
               GAM_fread(&frames[i],sizeof(short),1,fp);
               GAM_fread(&frameDelay[i],sizeof(short),1,fp);
               GAM_fread(&frameDelayReset[i],sizeof(short),1,fp);
          }
     }
     GAM_fread(&bloodCount,sizeof(short),1,fp);
     GAM_fread(&goreCount,sizeof(short),1,fp);
     GAM_fread(goreSprite,sizeof(short),MAXGORECOUNT,fp);
     GAM_fread(wallSwitchStatus,sizeof(char),MAXWALLS>>3,fp);
     GAM_fread(spriteSwitchStatus,sizeof(char),MAXSPRITES>>3,fp);
}

void
EFF_debug(void)
{
     int  i;
     char num[8];

     sprintf(tempbuf,"MOVINGDOORS(%d): ",numMovingDoors);
     for (i=0 ; i < numMovingDoors ; i++) {
          sprintf(num,"%d ",movingDoor[i]);
          strcat(tempbuf,num);
     }
     debugOut(windowx1,windowy1,tempbuf);
}

