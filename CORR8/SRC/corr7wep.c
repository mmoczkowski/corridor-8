/***************************************************************************
 *   CORR7WEP.C - Weapon functions for Corridor 7
 *
 *                                                     03/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   FLATPROJECTILEANGLEADJ   500
#define   NUMSHOCKWAVESPRITES      12

int  autoAimFlag=1,
     weaponChangePending,
     weaponChangeStep;

long weaponChangeClock,
     weaponx,
     weapony;

short ammo[MAXSPRITES][MAXWEAPONTYPES],
     maxAmmo[MAXSPRITES][MAXWEAPONTYPES],
     weaponAnimDelay[MAXSPRITES],
     weaponTicDelay[MAXSPRITES];

short heatSourceIndex[MAXHEATSOURCES];

short lockonFOV=16,
     missileView=-1;

short disperseCount;

long disperseSteps;

struct weaponData weaponParms[MAXWEAPONS],
     *weaponPtr[MAXWEAPONS];

short
WEP_getNextHeatSource(void)
{
     short i;

     for (i=0 ; i < MAXHEATSOURCES ; i++) {
          if (heatSourceIndex[i] == -1) {
               return(i);
          }
     }
     crash("WEP_getNextHeatSource: Too many heat sources on map (max=%d)",
           MAXHEATSOURCES);
     return(0);
}

void
WEP_addHeatSource(short s)
{
     heatSourceIndex[WEP_getNextHeatSource()]=s;
}

int
WEP_getAutoAimSetting(void)
{
     return(autoAimFlag);
}

void
WEP_toggleAutoAimSetting(void)
{
     autoAimFlag^=1;
     if (autoAimFlag) {
          lockonFOV=16;
     }
     else {
          lockonFOV=2;
     }
}

BOOL
WEP_findPicName(char *picname,W32 *picnum)
{
     short count;
     char locbuf[80],*ptr,*ptr2;
     FILE *fp;

     fp=fopen("names.h","rt");
     if (fp != NULL) {
          do {
               count=0;
               memset(locbuf,0,sizeof(locbuf));
               fgets(locbuf,sizeof(locbuf),fp);
               if (feof(fp) || strlen(locbuf) == 0) {
                    continue;
               }
               ptr=locbuf;
               strupr(ptr);
               if (*ptr != '#') {
                    continue;
               }
               while (*ptr != ' ' && ++count < strlen(locbuf)) {
                    ptr++;
               }
               while (*ptr == ' ' && ++count < strlen(locbuf)) {
                    ptr++;
               }
               ptr2=ptr;
               while (*ptr2 != ' ' && ++count < strlen(locbuf)) {
                    ptr2++;
               }
               *ptr2=0;
               if (stricmp(ptr,picname) == 0) {
                    ptr=ptr2+1;
                    *picnum=atoi(ptr);
                    fclose(fp);
                    return(_TRUE);
               }
          } while (feof(fp) == 0);
     }
     fclose(fp);
     return(_FALSE);
}

long
WEP_getDistance(spritetype *spr1,spritetype *spr2)
{
     unsigned long xx,yy,zz;
     long dx,x,y,z;

     x=(spr2->x-spr1->x);
     xx=x*x;
     y=(spr2->y-spr1->y);
     yy=y*y;
     z=(spr2->z-spr1->z)>>8;
     zz=z*z;
     dx=ksqrt(tmulscale1(x,x,y,y,z,z));
     return(dx);
}

short
WEP_getAngle(spritetype *src,spritetype *trg)
{
     return(getangle(trg->x-src->x,trg->y-src->y));
}

int                                // returns 1 if spr can see trg within fov
WEP_canSee(spritetype *spr,spritetype *trg,short fov)
{
     short ang1,ang2,hiang,loang;
     long z1,z2;

     z1=spr->z-(tilesizy[spr->picnum]<<7);
     z2=trg->z-(tilesizy[trg->picnum]<<7);
     if (cansee(spr->x,spr->y,z1,spr->sectnum,
                trg->x,trg->y,z2,trg->sectnum)) {
          if (fov == 2048) {
               return(1);
          }
          ang1=spr->ang;
          ang2=WEP_getAngle(spr,trg);
          hiang=(ang1+(fov>>1))&2047;
          loang=(hiang-fov)&2047;
          if (loang > hiang) {
               if (ang2 > hiang && ang2 < loang) {
                    return(0);
               }
          }
          else if (ang2 < loang || ang2 > hiang) {
               return(0);
          }
          return(1);
     }
     return(0);
}

int
WEP_canSeeRange(spritetype *spr,spritetype *trg,short fov,long dx)
{
     long rdx;
     
     if (WEP_canSee(spr,trg,fov) && (rdx=WEP_getDistance(spr,trg)) <= dx) {
          return(rdx);
     }
     return(0);
}

struct weaponData *
WEP_getSpriteWeaponPtr(short s)
{
     return(weaponPtr[currentWeapon[s]]);
}

short
WEP_getWeaponAmmoType(short w)
{
     if (weaponPtr[w]->registered) {
          return(weaponPtr[w]->registered-1);
     }
     return(0);
}

short
WEP_getSpriteAmmoType(short s)
{
     return(WEP_getWeaponAmmoType(currentWeapon[s]));
}

void
WEP_hitscanExplode(short wall,short sect,long x,long y,long z,short weapon)
{
     int  s;
     spritetype *spr;
     struct weaponData *weap;

     weap=weaponPtr[weapon];
     s=ENG_insertsprite(sect,STAT_HITSCANEXPLODE);
     spr=spritePtr[s];
     spr->x=x;
     spr->y=y;
     spr->z=z;
     spr->cstat=SPRC_TRANSLUCENT;
     if (wall != -1) {
          spr->cstat|=SPRC_WALLSPRITE;
     }
     spr->shade=-4;
     spr->clipdist=32;
     spr->xrepeat=weap->explosionSize;
     spr->yrepeat=weap->explosionSize;
     currentWeapon[s]=weapon;
     if (wall != -1) {
          spr->ang=ENG_getNormal(wall);
     }
     if (weaponPtr[weapon]->hitscanFlag == 0) {
          WEP_explodeSprite(s);
     }
     else {
          EFF_setSpriteFrames(s,weap->explosionRate,weap->explosionStartPic,
                              weap->explosionStopPic);
     }
}

int
WEP_damagePoints(short weap,long distx)
{
     long d,p;
     struct weaponData *wptr;

     wptr=weaponPtr[weap];
     d=krand()+wptr->minDamage;
     if (d > wptr->maxDamage) {
          d=wptr->maxDamage;
     }
     else if (d < 0L) {
          d=0L;
     }
     if (wptr->range > 0) {
          p=128-divscale7(distx,wptr->range);     // percent of damage felt
          d=mulscale7(d,p);
     }
     return((int)d);
}

void
WEP_damageSprite(short snum,short shooter,short weapon,long distx)
{
     int  d=0;
     spritetype *spr;

     spr=spritePtr[snum];
     if (ACT_getHealth(snum) > 0) {
          ACT_setKiller(snum,shooter);
          d=WEP_damagePoints(weapon,distx);
          ACT_damageActor(snum,d,1);
          EFF_spawnBlood(snum,shooter,d);
          GAM_applyForce(snum,shooter,d);
          if (ACT_isActor(snum)) {
               ACT_actorPain(snum);
          }
     }
     if (spr->statnum == STAT_PROJECTILE || spr->statnum == STAT_PROXIMITY) {
          WEP_explodeSprite(snum);
     }
}

void
WEP_explodeSprite(short snum)
{
     changespritestat(snum,STAT_EXPLODESPRITE);
}

void
WEP_spawnShockwave(short s)
{
     short ang,angsteps,i,j;
     struct weaponData *weapPtr;
     spritetype *spr,*src;

     src=spritePtr[s];
     weapPtr=WEP_getSpriteWeaponPtr(s);
     ang=0;
     angsteps=mulscale16(divscale16(2048,360),(360/NUMSHOCKWAVESPRITES));
     for (i=0 ; i < NUMSHOCKWAVESPRITES ; i++) {
          j=ENG_insertsprite(src->sectnum,STAT_SHOCKWAVE);
          currentWeapon[j]=currentWeapon[s];
          spr=spritePtr[j];
          spr->x=src->x;
          spr->y=src->y;
          spr->z=src->z;
          spr->ang=ang;
          spr->owner=src->owner;
          spr->xrepeat=weapPtr->explosionSize;
          spr->yrepeat=weapPtr->explosionSize;
          spr->clipdist=spr->xrepeat;
          spr->cstat|=(SPRC_WALLSPRITE|SPRC_TRANSLUCENT);
          spr->shade=-127;
          EFF_setSpriteFrames(j,weapPtr->explosionRate,
                              weapPtr->explosionStartPic,
                              weapPtr->explosionStopPic);
          WEP_setAngleVelocities(j);
          ang=(ang+angsteps)&2047;
     }
}

void
WEP_projectileExplode(short snum)
{
     short i,nexti,j,k,w;
     long dx;
     spritetype *spr,*spr2,*spr3;
     struct weaponData *weap;

     spr2=spritePtr[snum];
     j=ENG_insertsprite(spr2->sectnum,STAT_PROJECTILEEXPLODE);
     spr=spritePtr[j];
     spr->x=spr2->x;
     spr->y=spr2->y;
     spr->z=spr2->z;
     spr->cstat=128;
     spr->clipdist=spr->xrepeat;
     spr->ang=spr2->ang;
     spr->owner=spr2->owner;
     GAM_underWater(j,GAM_isUnderWater(snum));
     w=currentWeapon[snum];
     if (w >= 0 && w < MAXWEAPONS) {
          weap=weaponPtr[w];
          spr->xrepeat=weap->explosionSize;
          spr->yrepeat=weap->explosionSize;
          EFF_setSpriteFrames(j,weap->explosionRate,weap->explosionStartPic,
                              weap->explosionStopPic);
          if (weap->explosionType == EXPLOSION_SHOCKWAVE) {
               WEP_spawnShockwave(snum);
          }
          else {
               for (k=STAT_PROJDAMAGEBEG ; k < STAT_PROJDAMAGEEND ; k++) {
                    for (i=headspritestat[k] ; i >= 0 ; i=nexti) {
                         nexti=nextspritestat[i];
                         spr3=spritePtr[i];
                         if (spr3->statnum >= MAXSTATUS) {
                              continue;
                         }
                         if (i == snum) {
                              continue;
                         }
                         if (WEP_canSee(spr,spr3,2048)) {
                              dx=WEP_getDistance(spr,spr3);
                              if (dx < weap->range) {
                                   WEP_damageSprite(i,j,w,dx);
                                   if (ACT_isDead(i)) {
                                        ACT_setKiller(i,spr->owner);
                                   }
                              }
                         }
                    }
               }
          }
     }
     else {
          spr->xrepeat=64;
          spr->yrepeat=64;
          EFF_setSpriteFrames(j,TMR_getSecondFraction(TICS_ONESIXTEENTH),
                              EXPLOSIONPICBEG,EXPLOSIONPICEND);
     }
     SND_playExplosionSound(j);
     ENG_deletesprite(snum);
}

short
WEP_findHeatSource(short s)
{
     short i=0,j;
     spritetype *spr2;
     struct weaponData *weapPtr;

     weapPtr=WEP_getSpriteWeaponPtr(s);
     while ((j=heatSourceIndex[i++]) >= 0) {
          if (j == spritePtr[s]->owner) {
               continue;
          }
          spr2=spritePtr[j];
          if (spr2->statnum >= MAXSTATUS) {
               continue;
          }
          if (EFF_testSpriteFlag(j,EXTFLAGS_HEATSOURCE) == 0) {
               continue;
          }
          if (WEP_canSee(spritePtr[s],spr2,weapPtr->projectileFOV)) {
               if (!ACT_isDead(j)) {
                    return(j);
               }
          }
     }
     for (i=headspritestat[STAT_PLAYER] ; i != -1 ; i=j) {
          j=nextspritestat[i];
          if (i == spritePtr[s]->owner) {
               continue;
          }
          if (WEP_canSee(spritePtr[s],spritePtr[i],weapPtr->projectileFOV)) {
               return(i);
          }
     }
     return(-1);
}

long
WEP_getSpriteHorizontalAdj(spritetype *spr1,spritetype *spr2)
{
     long adj,dx,z1,z2;

     dx=ACT_getDistance(spr1,spr2);
     if (dx > 0L) {
          z1=spr1->z;
          if ((spr1->cstat&SPRC_FLOORSPRITE) == 0) {
               z1-=(spr1->yrepeat<<7);
          }
          z2=spr2->z;
          if ((spr2->cstat&SPRC_FLOORSPRITE) == 0) {
               z2-=(spr2->yrepeat<<7);
          }
          adj=(((z2-z1)/dx)<<3);
#if 0
          if (adj < 0L) {
               adj-=4L;
          }
          else if (adj > 0L) {
               adj+=4L;
          }
#endif
     }
     else {
          adj=0L;
     }
     return(adj);
}

long
WEP_getSpriteHorizontal(spritetype *spr1,spritetype *spr2,long hor)
{
     long adj;

     adj=WEP_getSpriteHorizontalAdj(spr1,spr2);
     hor=100-adj;
     if (hor > 200L) {
          hor=200L;
     }
     else if (hor < 0L) {
          hor=0L;
     }
     return(hor);
}

short
WEP_lockon(short owner)
{
     short i,nexti,k,targ=-1;
     long dx,mindx=0x7FFFFFFF;
     spritetype *spr,*spr2;

     spr=spritePtr[owner];     
     for (k=STAT_PATROL ; k <= STAT_DODGING ; k++) {
          for (i=headspritestat[k] ; i >= 0 ; i=nexti) {
               nexti=nextspritestat[i];
               spr2=spritePtr[i];
               if (WEP_canSee(spr,spr2,lockonFOV)) {
                    if ((dx=WEP_getDistance(spr,spr2)) < mindx) {
                         mindx=dx;
                         targ=i;
                    }
               }
          }
     }
     for (i=headspritestat[STAT_PLAYER] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr2=spritePtr[i];
          if (WEP_canSee(spr,spr2,lockonFOV)) {
               if ((dx=WEP_getDistance(spr,spr2)) < mindx) {
                    mindx=dx;
                    targ=i;
               }
          }
     }
     return(targ);
}

short
WEP_disperse(short d)
{
     short dsp;

     if (d <= 0) {
          return(0);
     }
     if ((krand()&1) != 0) {
          dsp=krand()%d;
     }
     else {
          dsp=-(krand()%d);
     }
     return(dsp);
}

short
WEP_fixedDisperse(short n)
{
     long ang=0L;

     if (disperseSteps > 0L && n > 0) {
          ang=mulscale16(disperseSteps,n);
     }
     return((short)ang);
}

short
WEP_spawnProjectile(short owner,long z,long hor)
{
     short s,target,weap;
     spritetype *spr;
     struct weaponData *weapPtr;

     weap=currentWeapon[owner];
     weapPtr=weaponPtr[weap];
     s=ENG_insertsprite(spritePtr[owner]->sectnum,weapPtr->detonation);
     spr=spritePtr[s];
     spr->x=spritePtr[owner]->x;
     spr->y=spritePtr[owner]->y;
     spr->z=z;
     spr->picnum=weapPtr->projectilePic;
     spr->cstat=SPRC_REALCENTERED|SPRC_BLOCKINGH;
     spr->xrepeat=weapPtr->projectileXSize;
     spr->yrepeat=weapPtr->projectileYSize;
     spr->clipdist=spr->xrepeat;
     switch (weapPtr->disperseType) {
     case DISPERSE_FIXED:
          spr->ang=(spritePtr[owner]->ang+
                    WEP_fixedDisperse(disperseCount))&2047;
          break;
     default:
          spr->ang=(spritePtr[owner]->ang+
                    WEP_disperse(weapPtr->dispersement))&2047;
          break;
     }
     spr->owner=owner;
     switch (weapPtr->projectileFace) {
     case PROJXFLAT:
          spr->cstat|=SPRC_WALLSPRITE;
          spr->ang=(spr->ang-FLATPROJECTILEANGLEADJ)&2047;
          break;
     case PROJYFLAT:
          spr->cstat|=SPRC_FLOORSPRITE;
          break;
     }
     EFF_resetSpriteFlag(s,EFF_getSpriteFlag(s));
     currentWeapon[s]=weap;
     if (weapPtr->gravityFlag) {
          EFF_setSpriteFlag(s,EXTFLAGS_GRAVITY);
     }
     spriteHoriz[s]=hor;
     if (ACT_isActor(owner)) {
          if ((target=ACT_getAttackee(spritePtr[owner])) >= 0) {
               spriteHoriz[s]=WEP_getSpriteHorizontal(spr,spritePtr[target],hor);
          }
     }
     else if (PLR_isPlayer(NULL,owner)) {
          if ((target=WEP_lockon(s)) >= 0) {
               spriteHoriz[s]=WEP_getSpriteHorizontal(spr,spritePtr[target],hor);
          }
     }
     ACT_setKiller(s,-1);
     if (weapPtr->detonation == STAT_PROXIMITY) {
          weaponTicDelay[s]=-1;
     }
     WEP_setAngleVelocities(s);
     GAM_underWater(s,GAM_isUnderWater(owner));
     return(s);
}

void
WEP_shootWeapon(short owner,long z,long hor,
                short *hitsect,short *hitwall,short *hitspr,
                long *hitx,long *hity,long *hitz)
{
     short a,sect,target;
     long x,y,xvect,yvect,zvect;
     spritetype *spr;
     struct weaponData *weapPtr;

     spr=spritePtr[owner];
     a=spr->ang;
     x=spr->x;
     y=spr->y;
     sect=spr->sectnum;
     weapPtr=WEP_getSpriteWeaponPtr(owner);
     if (weapPtr->hitscanFlag || weapPtr->projectileSpeed == 10) {
          if (ACT_isActor(owner)) {
               target=ACT_getAttackee(spr);
               hor=WEP_getSpriteHorizontal(spr,spritePtr[target],hor);
          }
          else if ((target=WEP_lockon(owner)) >= 0) {
               a=WEP_getAngle(spr,spritePtr[target]);
               hor=WEP_getSpriteHorizontal(spr,spritePtr[target],hor);
          }
          a+=WEP_disperse(weapPtr->dispersement);
          zvect=(100-hor)*2000L;
          xvect=sintable[(a+512)&2047];
          yvect=sintable[a&2047];
          hitscan(x,y,z,sect,xvect,yvect,zvect,
                  hitsect,hitwall,hitspr,hitx,hity,hitz);
     }
     else {
          WEP_spawnProjectile(owner,z,hor);
     }
}

short
WEP_turnSprite(short a,short ang,short speed)
{
     if (a > ang) {
          if (a-ang > 1024) {
               a=(a+speed)&2047;
               if (a > ang) {
                    a=ang;
               }
          }
          else {
               a=(a-speed)&2047;
               if (a < ang) {
                    a=ang;
               }
          }
     }
     else if (a < ang) {
          if (ang-a > 1024) {
               a=(a-speed)&2047;
               if (a < ang) {
                    a=ang;
               }
          }
          else {
               a=(a+speed)&2047;
               if (a > ang) {
                    a=ang;
               }
          }
     }
     return(a);
}

void
WEP_setAngleVelocities(short s)
{
     short ang,speed;
     long hor;
     struct weaponData *weapPtr;
     spritetype *spr;

     hor=spriteHoriz[s];
     weapPtr=WEP_getSpriteWeaponPtr(s);
     spr=spritePtr[s];
     ang=spr->ang;
     if (spr->statnum == STAT_SHOCKWAVE) {
          speed=7;
     }
     else {
          if ((spr->cstat&SPRC_WALLSPRITE) != 0) {
               ang=(ang+FLATPROJECTILEANGLEADJ)&2047;
          }
          speed=weapPtr->projectileSpeed;
     }
     if (speed == 0) {
          spr->xvel=0;
          spr->yvel=0;
          spr->zvel=0;
          return;
     }
     spr->xvel=(sintable[(ang+512)&2047])>>speed;
     spr->yvel=(sintable[ang])>>speed;
     spr->zvel=(100-hor)<<(11-speed);
}

int
WEP_getAmmo(short s,short ammotype)
{
     if (ammotype >= 0 && ammotype < MAXWEAPONTYPES) {
          return(ammo[s][ammotype]);
     }
     return(0);
}

void
WEP_adjAmmo(short snum,short type,short n)
{
     if (type < 0 || type >= MAXWEAPONTYPES) {
          return;
     }
     ammo[snum][type]+=n;
     if (ammo[snum][type] > maxAmmo[snum][type]) {
          ammo[snum][type]=maxAmmo[snum][type];
     }
     if (ammo[snum][type] < 0) {
          ammo[snum][type]=0;
     }
}

void
WEP_setAmmo(short snum,short type,short n)
{
     if (type < 0 || type >= MAXWEAPONTYPES) {
          return;
     }
     ammo[snum][type]=n;
}

short
WEP_getMaxAmmo(short snum,short type)
{
     if (type < 0 || type >= MAXWEAPONTYPES) {
          return(0);
     }
     return(maxAmmo[snum][type]);
}

void
WEP_setMaxAmmo(short snum,short type,short n)
{
     if (type < 0 || type >= MAXWEAPONTYPES) {
          return;
     }
     if (n > maxAmmo[snum][type]) {
          maxAmmo[snum][type]=n;
     }
}

void
WEP_defaultAmmo(short snum)
{
     int  w;

     for (w=0 ; w < MAXWEAPONS ; w++) {
          if (weaponPtr[w]->registered) {
               WEP_setAmmo(snum,WEP_getWeaponAmmoType(w),
                           weaponPtr[w]->defaultAmmo);
               WEP_setMaxAmmo(snum,WEP_getWeaponAmmoType(w),
                              weaponPtr[w]->defaultAmmo);
          }
     }
}

void
WEP_addWeapon(short s,short weap)
{
}

int
WEP_firingWeapon(short s)
{
     if (firingWeapon[s] > 0 || fireFrame[s] != 0) {
          return(1);
     }
     return(0);
}

int
WEP_fireFrame(short s)
{
     struct weaponData *weapPtr;

     weapPtr=WEP_getSpriteWeaponPtr(s);
     if (fireFrame[s] == weapPtr->weaponShootFrame) {
          return(1);
     }
     return(0);
}

int
WEP_nextFrame(short s)
{
     struct weaponData *weapPtr;

     weaponAnimDelay[s]-=TICWAITPERFRAME;
     if (weaponAnimDelay[s] <= 0) {
          weapPtr=WEP_getSpriteWeaponPtr(s);
          weaponAnimDelay[s]+=weapPtr->animDelay;
          fireFrame[s]++;
          if (WEP_fireFrame(s)) {
               return(1);
          }
          else if (fireFrame[s] > weapPtr->fireFrames) {
               weaponTicDelay[s]+=weapPtr->ticDelay;
               weaponAnimDelay[s]=0;
               fireFrame[s]=0;
          }
     }
     return(0);
}

int
WEP_readyToFire(short s)
{
     if (weaponTicDelay[s] > 0) {
          weaponTicDelay[s]-=TICWAITPERFRAME;
     }
     else if (WEP_firingWeapon(s)) {
          return(WEP_nextFrame(s));
     }
     return(0);
}

int
WEP_lowerWeapon(void)
{
     long y;

     weaponChangeStep+=((fakeClock-weaponChangeClock)<<1);
     y=weapony+weaponChangeStep;
     if (y > 200) {
          weapony=200;
          return(1);
     }
     weapony=y;
     return(0);
}

int
WEP_raiseWeapon(void)
{
     long y;

     weaponChangeStep+=((fakeClock-weaponChangeClock)<<1);
     y=200-weaponChangeStep;
     if (y <= weapony) {
          return(1);
     }
     weapony=y;
     return(0);
}

void
WEP_changeWeaponAnim(short s)
{
     short viewWeapon;

     if (WEP_firingWeapon(s) || ACT_isDead(s)) {
          return;
     }
     if (WEP_changingWeapon()) {
          if (weaponChangePending > 0) {
               if (WEP_lowerWeapon()) {
                    viewWeapon=weaponChangePending-1;
                    PLR_setViewWeapon(viewWeapon);
                    weaponChangePending=-1;
                    weaponChangeStep=0;
                    EFF_displayMessage("%s",weaponPtr[viewWeapon]->name);
               }
          }
          else if (weaponChangePending < 0) {
               if (WEP_raiseWeapon()) {
                    weaponChangePending=0;
                    weaponChangeStep=0;
               }
          }
     }
}

void
WEP_spriteFireWeapon(short snum)
{
     short count,fire,hitsect=-1,hitwall=-1,hitspr=-1,weapon;
     long h,hitx,hity,hitz,x,y,z;
     spritetype *spr;
     struct weaponData *weapPtr;

     if (ACT_isDead(snum)) {
          firingWeapon[snum]=0;
          fireFrame[snum]=0;
          return;
     }
     weapon=currentWeapon[snum];
     weapPtr=weaponPtr[weapon];
     fire=WEP_readyToFire(snum);
     spr=spritePtr[snum];
     spr->picnum=ACT_getSpritePic(snum);
     if (WEP_getAmmo(snum,WEP_getWeaponAmmoType(weapon)) == 0 &&
         cheatUnlimitedAmmo == 0) {
          return;
     }
     if (fire) {
          x=spr->x;
          y=spr->y;
          h=spriteHoriz[snum]+horizSlopeAdj[snum];
          z=spr->z-(spr->yrepeat<<7);
          SND_playWeaponSound(snum,weapPtr->sample,0);
          if (GAM_getMadeNoise() == -1) {
               GAM_setMadeNoise(snum);
          }
          count=weapPtr->projectiles;
          disperseSteps=divscale16((weapPtr->dispersement<<1),count);
          for (disperseCount=0 ; disperseCount < count ; disperseCount++) {
               if (weapPtr->hitscanFlag || weapPtr->projectileSpeed < 3) {
                    WEP_shootWeapon(snum,z,h-3+(krand()&7),&hitsect,&hitwall,
                                    &hitspr,&hitx,&hity,&hitz);
                    if (hitspr != -1) {
                         WEP_damageSprite(hitspr,snum,weapon,0L);
                         if (ACT_isDead(hitspr)) {
                              ACT_setKiller(hitspr,snum);
                         }
                    }
                    WEP_hitscanExplode(hitwall,hitsect,hitx,hity,hitz,weapon);
               }
               else {
                    WEP_shootWeapon(snum,z,h,&hitsect,&hitwall,&hitspr,
                                    &hitx,&hity,&hitz);
               }
          }
          WEP_adjAmmo(snum,weapPtr->registered-1,-1);
     }
}

void
WEP_weaponActivated(short s)
{
     firingWeapon[s]=1;
}

int
WEP_changingWeapon(void)
{
     if (weaponChangePending == 0) {
          return(0);
     }
     return(1);
}

void
WEP_changeWeapon(short weap)
{
     if (WEP_changingWeapon() == 0) {
          weaponChangePending=weap+1;
     }
}

void
WEP_drawWeapon(short s,short weapon,short frame)
{
     short pic;
     static short blastPic;
     struct weaponData *weapPtr;

     weapPtr=weaponPtr[weapon];
     if (!WEP_firingWeapon(s) || ACT_isDead(s)) {
          pic=weapPtr->holdWeaponStartPic;
          if (pic > 0) {
               if (waloff[pic+frame] == 0) {
                    loadtile(pic+frame);
               }
               if (weapPtr->autoCenterFlag) {
                    weaponx=(320>>1)-(tilesizx[pic+frame]>>1);
                    weapony=200-(tilesizy[pic+frame]>>1);
                    weapony-=(tilesizy[pic+frame]>>2);
               }
               else {
                    weaponx=(320>>1);
                    weapony=(200>>1);
               }
          }
          else {
               weaponx=(320>>1);
               weapony=200;
          }
          if (ACT_isDead(s)) {
               if (weapony < 200) {
                    WEP_lowerWeapon();
               }
               weaponChangePending=-1;
          }
          WEP_changeWeaponAnim(s);
     }
     else {
          pic=weapPtr->fireWeaponStartPic;
          if (pic > 0) {
               if (waloff[pic+frame] == 0) {
                    loadtile(pic+frame);
               }
               if (weapPtr->autoCenterFlag) {
                    weaponx=(320>>1)-(tilesizx[pic+frame]>>1);
                    weapony=200-(tilesizy[pic+frame]>>1);
                    weapony-=(tilesizy[pic+frame]>>2);
               }
               else {
                    weaponx=(320>>1);
                    weapony=(200>>1);
               }
          }
          else {
               if (weapPtr->autoCenterFlag) {
                    weaponx=(320>>1);
                    weapony=200;
               }
               else {
                    weaponx=(320>>1);
                    weapony=(200>>1);
               }
          }
     }
     if (qsetmode == 200L && GAM_isViewSprite(s) && pic > 0) {
          if (weapPtr->autoCenterFlag) {
               rotatesprite(weaponx<<16,weapony<<16,65536L,0,pic+frame,
                            0,0,2+16,windowx1,windowy1,windowx2,windowy2);
          }
          else {
               if (frame == weapPtr->weaponShootFrame) {
                    blastPic=MBLAST1APIC+((rand()&3)<<1);
                    rotatesprite(weaponx<<16,weapony<<16,65536L,0,blastPic,
                                 0,0,2,windowx1,windowy1,windowx2,windowy2);
               }
               else if (frame == weapPtr->weaponShootFrame+1) {
                    rotatesprite(weaponx<<16,weapony<<16,65536L,0,blastPic+1,
                                 0,0,2,windowx1,windowy1,windowx2,windowy2);
               }
               rotatesprite(weaponx<<16,weapony<<16,65536L,0,pic+frame,
                            0,0,2,windowx1,windowy1,windowx2,windowy2);
          }
     }
     weaponChangeClock=fakeClock;
}

void
WEP_doPhysics(short i,spritetype *spr)
{
     short clipheight,sect;
     long hiz,loz,x,y,z;

     x=spr->x;
     y=spr->y;
     z=spr->z;
     sect=spr->sectnum;
     clipheight=4L<<8;
     getzsofslope(sect,x,y,&hiz,&loz);
     if (z < loz-clipheight) {
          if (GAM_isUnderWater(i)) {
               z+=gravityConstant;
          }
          else {
               spr->zvel=kmax(spr->zvel+gravityConstant,32767L);
               z+=spr->zvel;
          }
     }
     else if (EFF_isAboveWaterSector(sect) && !GAM_isUnderWater(i)) {
          EFF_warpBelowWater(i,spr,&sect,&x,&y,&z);
     }
     else {
          z=loz;
     }
     setsprite(i,x,y,z);
}

void
WEP_doStatusProjectile(short i)
{
     short a,hitobject,o,sect;
     long dax,day,daz;
     struct weaponData *weapPtr;
     spritetype *spr,*trg;

     spr=spritePtr[i];
     weapPtr=WEP_getSpriteWeaponPtr(i);
     switch (weapPtr->guidance) {
     case GUIDANCE_HEATSEEKER:
          if (weaponTicDelay[i] > 0) {
               weaponTicDelay[i]-=TICWAITPERFRAME;
               if (weaponTicDelay[i] < 0) {
                    weaponTicDelay[i]=0;
               }
          }
          if (ACT_getKiller(i) < 0 && weaponTicDelay[i] == 0) {
               ACT_setKiller(i,WEP_findHeatSource(i));
               weaponTicDelay[i]=weapPtr->trackerFreq;
          }
          if ((o=ACT_getKiller(i)) > 0) {
               if (ACT_isDead(o)) {
                    ACT_setKiller(i,-1);
                    weaponTicDelay[i]=weapPtr->trackerFreq;
               }
               else if (weaponTicDelay[i] == 0) {
                    trg=spritePtr[o];
                    if (WEP_canSee(spr,trg,2048)) {
                         a=WEP_getAngle(spr,trg);
                    }
                    else {
                         a=spr->ang;
                    }
                    if ((spr->cstat&SPRC_WALLSPRITE) != 0) {
                         a=(a-FLATPROJECTILEANGLEADJ)&2047;
                    }
                    spr->ang=WEP_turnSprite(spr->ang,a,
                                            weapPtr->projectileTurnRate);
                    spriteHoriz[i]=WEP_getSpriteHorizontal(spr,trg,100);
                    WEP_setAngleVelocities(i);
                    weaponTicDelay[i]=weapPtr->trackerFreq;
               }
          }
          break;
     }
     if (weapPtr->gravityFlag) {
          WEP_doPhysics(i,spr);
     }
     dax=(((long)spr->xvel)*TICWAITPERFRAME)<<12;
     day=(((long)spr->yvel)*TICWAITPERFRAME)<<12;
     daz=(((long)spr->zvel)*TICWAITPERFRAME)>>2;
     hitobject=movesprite((short)i,dax,day,daz,4L<<8,4L<<8,1);
     updatesector(spr->x,spr->y,&o);
     if (o < 0) {
          ENG_deletesprite(i);
          return;
     }
     if (hitobject != 0) {
          if ((hitobject&0xC000) == 32768) {      // hit a wall
               o=hitobject&4095;
          }
          else if ((hitobject&0xC000) == 49152) { // hit a sprite
               o=hitobject&4095;
               if (spritePtr[o]->statnum == STAT_PROJECTILE) {
                    if (spritePtr[o]->owner == spr->owner) {
                         return;
                    }
               }
          }
          else if (hitobject != 0) {    // hit a ceiling/floor
               o=hitobject&4095;
               dax=spr->x;
               day=spr->y;
               daz=spr->z;
               sect=spr->sectnum;
               if (EFF_isAboveWaterSector(o) && spr->zvel > 0) {
                    EFF_warpBelowWater(i,spr,&sect,&dax,&day,&daz);
                    setsprite(i,dax,day,daz);
                    return;
               }
               if (EFF_isBelowWaterSector(o) && spr->zvel < 0) {
                    EFF_warpAboveWater(i,spr,&sect,&dax,&day,&daz);
                    setsprite(i,dax,day,daz);
                    return;
               }
          }
          spr->xrepeat=64;
          spr->yrepeat=64;
          WEP_explodeSprite(i);
     }
     setsprite(i,spr->x,spr->y,spr->z);
}

void
WEP_doStatusProximity(short i)
{
     short j,nextj,stat;
     long dx;
     struct weaponData *weapPtr;
     spritetype *spr,*spr2;
     
     spr=spritePtr[i];
     weapPtr=WEP_getSpriteWeaponPtr(i);
     if (weapPtr->projectileSpeed > 0 && weapPtr->projectileSpeed <= 10) {
          WEP_doStatusProjectile(i);
     }
     else if (weapPtr->gravityFlag) {
          WEP_doPhysics(i,spr);
     }
//** don't arm weapon until owner walks out of range
     if (spr->owner != -1) {
          if (weaponTicDelay[i] == -1) {
               j=spr->owner;
               spr2=spritePtr[j];
               if (!ACT_isDead(j)) {
                    if (WEP_canSee(spr,spr2,2048)) {
                         dx=WEP_getDistance(spr,spr2);
                         if (dx < weapPtr->detrange) {
                              return;
                         }
                    }
               }
//** arm weapon now
               weaponTicDelay[i]=0;
               SND_playProximityArmedSound(i);
               return;
          }
     }
     for (stat=STAT_PROJDAMAGEBEG ; stat < STAT_PROJDAMAGEEND ; stat++) {
          for (j=headspritestat[stat] ; j >= 0 ; j=nextj) {
               nextj=nextspritestat[j];
               spr2=spritePtr[j];
               dx=WEP_getDistance(spr,spr2);
               if (dx < weapPtr->detrange) {
                    if (WEP_canSee(spr,spr2,2048)) {
                         WEP_explodeSprite(i);
                         return;
                    }
               }
          }
     }
     for (j=headspritestat[STAT_PROJECTILE] ; j >= 0 ; j=nextj) {
          nextj=nextspritestat[j];
          spr2=spritePtr[j];
          dx=WEP_getDistance(spr,spr2);
          if (dx < weapPtr->detrange) {
               if (WEP_canSee(spr,spr2,2048)) {
                    WEP_explodeSprite(i);
                    return;
               }
          }
     }
}

void
WEP_doStatusHitscanExplode(short i)
{
     spritetype *spr;

     spr=spritePtr[i];
     frameDelay[i]-=TICWAITPERFRAME;
     if (frameDelay[i] <= 0) {
          if (frames[i] > 0) {
               frameDelay[i]+=frameDelayReset[i];
               frames[i]--;
               spr->picnum++;
          }
          else {
               ENG_deletesprite(i);
          }
     }
}

void
WEP_doStatusProjectileExplode(short i)
{
     spritetype *spr;

     frameDelay[i]-=TICWAITPERFRAME;
     if (frameDelay[i] <= 0) {
          if (frames[i] > 0) {
               spr=spritePtr[i];
               frameDelay[i]+=frameDelayReset[i];
               frames[i]--;
               spr->picnum++;
          }
          else {
               ENG_deletesprite((short)i);
          }
     }
}

void
WEP_doStatusShockwave(short i)
{
     short hitobject,j,nextj,stat,xrepeat;
     long dax,day,daz,dx;
     struct weaponData *weapPtr;
     spritetype *spr,*spr2;

     spr=spritePtr[i];
     WEP_doStatusProjectileExplode(i);
     if (spr->statnum < MAXSTATUS) {
          weapPtr=WEP_getSpriteWeaponPtr(i);
          xrepeat=spr->xrepeat+(TICWAITPERFRAME<<1);
          if (xrepeat > 255) {
               xrepeat=255;
          }
          spr->xrepeat=xrepeat;
          dax=(((long) spr->xvel)*TICWAITPERFRAME)<<12;
          day=(((long) spr->yvel)*TICWAITPERFRAME)<<12;
          daz=0L;
          hitobject=movesprite((short)i,dax,day,daz,4L<<8,4L<<8,1);
          updatesector(spr->x,spr->y,&j);
          if (j < 0) {
               ENG_deletesprite(i);
               return;
          }
          if (hitobject != 0) {
               if ((hitobject&0xC000) == 32768) {      // hit a wall
                    ENG_deletesprite(i);
                    return;
               }
          }
          setsprite(i,spr->x,spr->y,spr->z);
          for (stat=STAT_PROJDAMAGEBEG ; stat < STAT_PROJDAMAGEEND ; stat++) {
               for (j=headspritestat[stat] ; j >= 0 ; j=nextj) {
                    nextj=nextspritestat[j];
                    spr2=spritePtr[j];
                    if ((dx=WEP_canSeeRange(spr,spr2,2048,weapPtr->range))) {
                         WEP_damageSprite(j,i,currentWeapon[i],dx);
                         if (ACT_isDead(j)) {
                              ACT_setKiller(j,spr->owner);
                         }
                    }
               }
          }
     }
}

void
WEP_doStatusCode(void)
{
     short i,nexti;

     if (editorEnabledFlag && gameModeFlag == 0) {
          return;
     }
     for (i=headspritestat[STAT_HITSCANEXPLODE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          WEP_doStatusHitscanExplode(i);
     }
     for (i=headspritestat[STAT_SHOCKWAVE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          WEP_doStatusShockwave(i);
     }
     for (i=headspritestat[STAT_PROJECTILE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          WEP_doStatusProjectile(i);
     }
     for (i=headspritestat[STAT_PROXIMITY] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          WEP_doStatusProximity(i);
     }
     for (i=headspritestat[STAT_EXPLODESPRITE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          WEP_projectileExplode(i);
     }
     for (i=headspritestat[STAT_PROJECTILEEXPLODE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          WEP_doStatusProjectileExplode(i);
     }
}

void
WEP_setMissileView(short snum)
{
     missileView=snum;
}

short
WEP_getMissileView(void)
{
     if (missileView >= 0 && spritePtr[missileView]->statnum >= MAXSTATUS) {
          missileView=-1;
     }
     return(missileView);
}

short
WEP_findMissileView(short owner)
{
     short i,nexti;

     for (i=headspritestat[STAT_PROJECTILE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          if (spritePtr[i]->owner == owner) {
               return(i);
          }
     }
     for (i=headspritestat[STAT_PROXIMITY] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          if (spritePtr[i]->owner == owner) {
               return(i);
          }
     }
     return(-1);
}

void
WEP_showMissileViewStatus(void)
{
     short k,snum;
     struct weaponData *weapPtr;
     spritetype *spr,*spr2;

     if ((snum=WEP_getMissileView()) < 0) {
          return;
     }
     spr=spritePtr[snum];
     weapPtr=WEP_getSpriteWeaponPtr(snum);
     debugOut(windowx1,windowy1,"%s: ID=%d X=%d Y=%d Z=%d",weapPtr->name,
              snum,spr->x,spr->y,spr->z);
     switch (weapPtr->guidance) {
     case GUIDANCE_HEATSEEKER:
     case GUIDANCE_CRUISE:
          debugOut(windowx1,windowy1,"TRACKING MODE... %s",
                   (weapPtr->guidance == GUIDANCE_HEATSEEKER) ?
                  "HEAT SEEKER" : "CRUISE MISSILE");
          if ((k=ACT_getKiller(snum)) < 0) {
               sprintf(tempbuf,"STATUS.......... SEARCHING FOR TARGET");
          }
          else {
               spr2=spritePtr[k];
               debugOut(windowx1,windowy1,"STATUS.......... TARGET ACQUIRED");
               debugOut(windowx1,windowy1,"DISTANCE........ %ld",
                        WEP_getDistance(spr,spr2));
          }
          break;
     default:
          debugOut(windowx1,windowy1,"TRACKING MODE: DUMB");
          break;
     }
}

void
WEP_resetHeatSourceIndex(void)
{
     memset(heatSourceIndex,-1,sizeof(heatSourceIndex));
}

int
WEP_isArmedDevice(short s)
{
     switch (spritePtr[s]->statnum) {
     case STAT_PROJECTILEEXPLODE:
     case STAT_SHOCKWAVE:
     case STAT_EXPLODESPRITE:
     case STAT_PROXIMITY:
     case STAT_PROJECTILE:
          return(1);
     }
     return(0);
}

void
WEP_scanMap(void)
{
     short i,s;
     spritetype *spr;
          
     memset(&missileView,-1,sizeof(missileView));
     for (i=0 ; i < MAXSPRITES ; i++) {
          spr=spritePtr[i];
          if (spr->statnum < MAXSTATUS) {
               if (WEP_isArmedDevice(i)) {
                    if (spr->extra >= 0) {
                         currentWeapon[i]=spr->extra;
                         spr->extra=-1;
                         s=WEP_spawnProjectile(i,spr->z,100L);
                         spritePtr[s]->picnum=spr->picnum;
                         spritePtr[s]->xrepeat=spr->xrepeat;
                         spritePtr[s]->yrepeat=spr->yrepeat;
                         spritePtr[s]->clipdist=spr->xrepeat;
                         spritePtr[s]->cstat=spr->cstat;
                         spritePtr[s]->owner=-1;
                         spritePtr[s]->extra=-1;
                         weaponTicDelay[s]=0;
                         ENG_deletesprite(i);
                    }
               }
          }
     }
}

void
WEP_saveGame(FILE *fp)
{
     GAM_fwrite(weaponParms,sizeof(struct weaponData),MAXWEAPONS,fp);
}

void
WEP_loadGame(FILE *fp)
{
     GAM_fread(weaponParms,sizeof(struct weaponData),MAXWEAPONS,fp);
}

void
WEP_debug(void)
{
     short nexts,s;

     for (s=headspritestat[STAT_PROJECTILE] ; s >= 0 ; s=nexts) {
          nexts=nextspritestat[s];
          debugOut(windowx1,windowy1,"PROJ%05d: EXTRA=%d KILLER=%d",s,
                   spritePtr[s]->extra,ACT_getKiller(s));
     }
     for (s=headspritestat[STAT_PROXIMITY] ; s >= 0 ; s=nexts) {
          nexts=nextspritestat[s];
          debugOut(windowx1,windowy1,"PROX%05d: EXTRA=%d KILLER=%d "
                   "OWNER=%d",s,spritePtr[s]->extra,ACT_getKiller(s),
                   spritePtr[s]->owner);
     }
}

