/***************************************************************************
 *   CORR7ACT.C - Actor (alien) related functions for Corridor 7: GW
 *
 *                                                     05/02/96  Les Bird
 ***************************************************************************/

//**
//** ACTOR sprite variables used:  xvel      = packed walk/chase speed,
//**                                           aggression and weapon.
//**                               yvel      = special flags
//**                               extra     = health
//**                               owner     = attackee sprite
//**                               hitag     = link to sector/switch
//**                               lotag     = type of sprite (light, etc)
//**
//** sprite variables available:   (short) zvel
//**
//** To get actor parameters use the following functions:
//**
//** ACT_getWalkSpeed(spritenum)   - returns walking speed (0-7)
//**                               - spritePtr[spritenum]->xvel&0x0007;
//** ACT_getChaseSpeed(spritenum)  - returns chasing speed (0-7)
//**                               - (spritePtr[spritenum]->xvel>>3)&0x0007;
//** ACT_getAggression(spritenum)  - returns aggressiveness (0-7)
//**                               - (spritePtr[spritenum]->xvel>>6)&0x0007;
//** ACT_getWeapon(spritenum)      - returns weapon used (0-15)
//**                               - (spritePtr[spritenum]->xvel>>9)&0x000F;
//** ACT_getHealth(spritenum)      - returns health for sprite (0-65535)
//**                               - spritePtr[spritenum]->extra;
//** ACT_getAttackee(spritenum)    - returns sprite being attacked by spritenum
//**                               - spritePtr[spritenum]->owner;
//**

#include "icorp.h"
#include <memcheck.h>

#define   MINSPEED                 6
#define   MAXATTACKERS             5

int  noEnemiesFlag;

char actorPicFlags[MAXTILES>>3],
     actorPicIndex[MAXTILES];

char attackers[MAXSPRITES],
     currentWeapon[MAXSPRITES],
     fireFrame[MAXSPRITES],
     firingWeapon[MAXSPRITES],
     painDelay[MAXSPRITES],
     walk180[MAXSPRITES>>3];

short actorPic[MAXSPRITES],
     attackDelay[MAXSPRITES],
     hitwallDelay[MAXSPRITES],
     killer[MAXSPRITES],
     nextSprite[MAXSPRITES],
     spriteHoriz[MAXSPRITES],
     waitDoor[MAXSPRITES];

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

static
short madeNoise;

void
ACT_init(void)
{
}

void
ACT_unInit(void)
{
}

void
ACT_buildPicBitArray(void)
{
     short i,j;

     for (i=0 ; i < MAXTILES ; i++) {
          for (j=0 ; j < MAXDEFINEDACTORS ; j++) {
               if (i == enemyPic[j] ||
                   i >= enemyFirePic1[j] && i <= enemyFirePic2[j] ||
                   i >= enemyDiePic1[j] && i <= enemyDiePic2[j] ||
                   i == enemyPainPic[j]) {
                    actorPicFlags[i>>3]|=(1<<(i&7));
                    actorPicIndex[i]=j;
                    break;
               }
          }
     }
}

short
ACT_getActorPicIndex(short s)
{
     if (ACT_isActor(s) || PLR_isPlayer(NULL,s)) {
          return((short)actorPicIndex[spritePtr[s]->picnum]);
     }
     return(-1);
}

void
ACT_setActorPic(short s,short pic)
{
     actorPic[s]=pic;
}

short
ACT_getWalkSpeed(short s)
{
     return(spritePtr[s]->xvel&0x0007);
}

void
ACT_setWalkSpeed(short s,short n)
{
     spritePtr[s]->xvel&=~0x0007;
     spritePtr[s]->xvel|=(n&0x0007);
}

short
ACT_getChaseSpeed(short s)
{
     return((spritePtr[s]->xvel>>3)&0x0007);
}

void
ACT_setChaseSpeed(short s,short n)
{
     spritePtr[s]->xvel&=~(0x0007<<3);
     spritePtr[s]->xvel|=(n&0x0007)<<3;
}

short
ACT_getAggression(short s)
{
     return((spritePtr[s]->xvel>>6)&0x0007);
}

void
ACT_setAggression(short s,short n)
{
     spritePtr[s]->xvel&=~(0x0007<<6);
     spritePtr[s]->xvel|=(n&0x0007)<<6;
}

short
ACT_getWeapon(short s)
{
     return((spritePtr[s]->xvel>>9)&0x000F);
}

void
ACT_setWeapon(short s,short n)
{
     spritePtr[s]->xvel&=~(0x000F<<9);
     spritePtr[s]->xvel|=(n&0x000F)<<9;
}

short
ACT_getHealth(short s)
{
     if (ACT_isActor(s) || PLR_isPlayer(NULL,s)) {
          return(spritePtr[s]->extra);
     }
     return(0);
}

void
ACT_setHealth(short s,short n)
{
     if (ACT_isActor(s) || PLR_isPlayer(NULL,s)) {
          spritePtr[s]->extra=n;
     }
}

void
ACT_adjHealth(short s,short n)
{
     if (ACT_isActor(s) || PLR_isPlayer(NULL,s)) {
          spritePtr[s]->extra+=n;
     }
}

short
ACT_getAttackee(spritetype *spr)
{
     return(spr->owner);
}

void
ACT_setAttackee(spritetype *spr,short n)
{
     spr->owner=n;
}

short
ACT_getAttackers(short s)
{
     return(attackers[s]);
}

void
ACT_setAttackers(short s,short n)
{
     attackers[s]=n;
}

void
ACT_adjAttackers(short s,short n)
{
     attackers[s]+=n;
}

short
ACT_getKiller(short s)
{
     if (killer[s] >= 0 && spritePtr[killer[s]]->statnum < MAXSTATUS) {
          return(killer[s]);
     }
     return(-1);
}

void
ACT_setKiller(short s,short n)
{
     if (ACT_isActor(s)) {
          ACT_setAttackee(spritePtr[s],n);
     }
     killer[s]=n;
}

short
ACT_getFirePic1(short s)
{
     short pic;

     pic=ACT_getActorPicIndex(s);
     if (pic >= 0) {
          pic=enemyFirePic1[pic];
          return(pic);
     }
     return(actorPic[s]);
}

short
ACT_getFirePic2(short s)
{
     short pic;

     pic=ACT_getActorPicIndex(s);
     if (pic >= 0) {
          pic=enemyFirePic2[pic];
          return(pic);
     }
     return(actorPic[s]);
}

short
ACT_getPainDelay(short s)
{
     return((short)painDelay[s]);
}

void
ACT_setPainDelay(short s,short n)
{
     painDelay[s]=n;
}

short
ACT_getMadeNoise(void)
{
     return(-1);
#if 0
     return(madeNoise);
#endif
}

void
ACT_setMadeNoise(short s)
{
     madeNoise=s;
}

void
ACT_adjPainDelay(short s,short n)
{
     short d;

     d=painDelay[s];
     d+=n;
     if (d < 0) {
          d=0;
     }
     painDelay[s]=d;
}

short
ACT_getActorViewHeight(spritetype *spr)
{
     return((spr->yrepeat-(spr->yrepeat>>2))<<8);
}

short
ACT_getActorClipHeight(spritetype *spr)
{
     return(spr->yrepeat<<4);
}

short
ACT_getActorClipDist(spritetype *spr)
{
     return(spr->clipdist<<2);
}

int
ACT_isDead(short s)
{
     short k;

     if (ACT_getHealth(s) <= 0) {
          k=ACT_getKiller(s);
          if (k >= 0 && k != s) {
               spritePtr[s]->ang=WEP_getAngle(spritePtr[s],spritePtr[k]);
          }
          ACT_setAttackers(s,0);
          return(1);
     }
     return(0);
}

void
ACT_killPlayer(short s)
{
     short h,i,pic=0;
     spritetype *spr;

     spr=spritePtr[s];
     PLR_adjScore(s);
     i=ACT_getActorPicIndex(s);
     h=ACT_getHealth(s);
     if (h < -150 && i >= 0) {
          if (enemyGorePic1[i] > 0 && enemyGorePic2[i] > 0) {
               pic=1;
               EFF_setSpriteFrames(s,TMR_getSecondFraction(8),
                                   enemyGorePic1[i],enemyGorePic2[i]);
               SND_playerGoreSound(s);
          }
     }
     if ((h >= -150 || pic == 0) && i >= 0) {
          if (enemyDiePic1[i] > 0 && enemyDiePic2[i] > 0) {
               pic=1;
               EFF_setSpriteFrames(s,TMR_getSecondFraction(8),
                                   enemyDiePic1[i],enemyDiePic2[i]);
               if (PLR_isPlayer(NULL,s)) {
                    SND_playerDieSound(s);
               }
               else {
                    SND_playEnemy1DieSound(s);
               }
          }
     }
     if (pic > 0) {
          ACT_setAttackee(spr,-1);
          ACT_changeState(spr,s,spr->statnum,STAT_DIEANIM);
          ACT_setHealth(s,0);
          EFF_resetSpriteFlag(s,EXTFLAGS_HEATSOURCE);
     }
     else {
          spr->cstat|=SPRC_INVISIBLE;
     }
     spr->cstat&=~(SPRC_BLOCKING|SPRC_BLOCKINGH);
}

void
ACT_damageActor(short s,int d,int useskill)
{
     short redshift;

     redshift=d;
     if (useskill) {
          if (PLR_isPlayer(NULL,s)) {
               switch (curSkill) {
               case SKILL_CORPORAL:
                    d-=mulscale3(d,6);  // 75% less damage
                    break;
               case SKILL_LIEUTENANT:
                    d-=mulscale3(d,4);  // 50% less damage
                    break;
               case SKILL_CAPTAIN:
                    d-=mulscale3(d,2);  // 25% less damage
                    break;
               case SKILL_MAJOR:
               case SKILL_PRESIDENT:
                    break;
               }
          }
          else {
               switch (curSkill) {
               case SKILL_CORPORAL:
                    d+=mulscale3(d,6);
                    break;
               case SKILL_LIEUTENANT:
                    d+=mulscale3(d,4);
                    break;
               case SKILL_CAPTAIN:
                    d+=mulscale3(d,2);
                    break;
               case SKILL_MAJOR:
               case SKILL_PRESIDENT:
                    break;
               }
          }
     }
     if (!cheatGodMode || s != PLR_getPlayerSprite(myconnectindex)) {
          ACT_adjHealth(s,-d);
     }
     if (ACT_getHealth(s) <= 0) {
          ACT_killPlayer(s);
     }
     else {
          if (PLR_isPlayer(NULL,s)) {
               ACT_setPainDelay(s,TICWAITPERFRAME<<3);
          }
          else if (ACT_isActor(s)) {
               ACT_adjPainDelay(s,TICWAITPERFRAME<<3);
          }
     }
     if (GAM_isViewSprite(s)) {
          GFX_redShift(redshift>>2);
     }
}

void
ACT_healActor(short s,short d)
{
     ACT_adjHealth(s,d);
     if (GAM_isViewSprite(s)) {
          GFX_greenShift(d>>2);
     }
}

void
ACT_inSector(short snum,short sectnum)
{
     switch (sectorPtr[sectnum]->lotag) {
     case SEC_HIDAMAGEBLINKTAG:
     case SEC_HIDAMAGETAG:
          if (GAM_isOnGround(snum)) {
               if (EFF_getDamageClock() <= 0) {
                    switch (curSkill) {
                    case SKILL_CORPORAL:
                         ACT_damageActor(snum,100,0);
                         break;
                    default:
                         ACT_damageActor(snum,200,0);
                         break;
                    }
               }
          }
          break;
     case SEC_MEDDAMAGETAG:
          if (GAM_isOnGround(snum)) {
               if (EFF_getDamageClock() <= 0) {
                    switch (curSkill) {
                    case SKILL_CORPORAL:
                         ACT_damageActor(snum,50,0);
                         break;
                    default:
                         ACT_damageActor(snum,100,0);
                         break;
                    }
               }
          }
          break;
     case SEC_LODAMAGETAG:
          if (GAM_isOnGround(snum)) {
               if (EFF_getDamageClock() <= 0) {
                    switch (curSkill) {
                    case SKILL_CORPORAL:
                         ACT_damageActor(snum,20,0);
                         break;
                    default:
                         ACT_damageActor(snum,50,0);
                         break;
                    }
               }
          }
          break;
     }
}

short
ACT_getSpritePic(short s)
{
     spritetype *spr;

     spr=spritePtr[s];
     if (!ACT_isDead(s)) {
          if (WEP_firingWeapon(s)) {
               if (WEP_fireFrame(s)) {
                    spr->picnum=ACT_getFirePic1(s);
               }
               else {
                    spr->picnum=ACT_getFirePic2(s);
               }
               spr->shade=-31;
          }
          else if (spr->picnum != actorPic[s]) {
               spr->picnum=actorPic[s];
               spr->shade=0;
          }
     }
     return(spr->picnum);
}

int
ACT_stillActor(short s)
{
     if (ACT_getWalkSpeed(s) == 7 && ACT_getChaseSpeed(s) == 7) {
          return(1);
     }
     return(0);
}

short
ACT_getRoundsToFire(short s)
{
     short k;
     struct weaponData *weapPtr;

     k=krand();
     weapPtr=weaponPtr[currentWeapon[s]];
     if (weapPtr->ticDelay < TMR_getSecondFraction(TICS_ONEFOURTH)) {
          switch (curSkill) {
          case SKILL_CORPORAL:
               return(k&3);
          case SKILL_LIEUTENANT:
          case SKILL_CAPTAIN:
               return(k&7);
          case SKILL_MAJOR:
          case SKILL_PRESIDENT:
               return(k&15);
               break;
          }
     }
     return(1);
}

void
ACT_changeState(spritetype *spr,short i,short curstat,short newstat)
{
     short attackee,stat;

     stat=spr->statnum;
     if (stat < MAXSTATUS) {
          if (spr->statnum == curstat) {
               if (curstat == STAT_ATTACKING) {
                    attackee=ACT_getAttackee(spr);
                    ACT_adjAttackers(attackee,-1);
               }
               if (newstat == STAT_ATTACKING) {
                    attackee=ACT_getAttackee(spr);
                    if (ACT_getAttackers(attackee) < MAXATTACKERS) {
                         ACT_adjAttackers(attackee,1);
                    }
                    else {
                         newstat=STAT_CHASE;
                    }
               }
               changespritestat(i,newstat);
          }
     }
}

int
ACT_isActorPic(short s)
{
     short pic;

     pic=spritePtr[s]->picnum;
     if ((actorPicFlags[pic>>3]&(1<<(pic&7))) != 0) {
          return(1);
     }
     return(0);
}

int
ACT_isActor(short s)
{
     short stat;

     stat=spritePtr[s]->statnum;
     if (stat >= STAT_PATROL && stat <= STAT_DODGING) {
          return(1);
     }
     return(0);
}

void
ACT_actorPain(short snum)
{
#if 0
     spritetype *spr;

     spr=spritePtr[snum];
     if ((krand()&0x20) == 0x20) {
          if (spr->statnum != STAT_DODGING) {
               ACT_changeState(spr,snum,spr->statnum,STAT_DODGE);
          }
     }
     else {
          if (spr->statnum != STAT_ATTACKING) {
               ACT_changeState(spr,snum,spr->statnum,STAT_ATTACK);
          }
     }
#endif
}

void
ACT_setupActor(short s)
{
     short w;

     spritePtr[s]->clipdist=spritePtr[s]->xrepeat;
     ACT_setActorPic(s,spritePtr[s]->picnum);
     w=ACT_getWeapon(s);
     if (weaponPtr[w]->registered == 0) {
          w=MAXWEAPONS;
     }
     if (w == MAXWEAPONS || curSkill == SKILL_PRESIDENT) {
          do {
               w=krand()%MAXWEAPONS;
          } while (weaponPtr[w]->registered == 0);
          currentWeapon[s]=w;
     }
     else {
          currentWeapon[s]=w;
     }
     WEP_addHeatSource(s);
     WEP_defaultAmmo(s);
     attackers[s]=-1;
     fireFrame[s]=0;
     firingWeapon[s]=0;
     painDelay[s]=0;
     walk180[s>>3]&=~(1<<(s&7));
     attackDelay[s]=0;
     hitwallDelay[s]=0;
     killer[s]=-1;
     nextSprite[s]=-1;
     spriteHoriz[s]=100;
     waitDoor[s]=-1;
     GAM_adjAliensLeft(1);
}

void
ACT_scanSprite(short i)
{
     spritetype *spr;

     spr=spritePtr[i];
     if (spr->statnum < MAXSTATUS) {
          switch (spr->picnum) {
          case PATHMARKERPIC:
               spr->cstat|=SPRC_INVISIBLE;
               spr->cstat&=~(SPRC_BLOCKING|SPRC_BLOCKINGH);
               changespritestat(i,STAT_PATHMARKER);
               ENG_checksumSprite(i);
               break;
          }
          switch (spr->lotag) {
          case SPR_HUMANSERGTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_3);
                    ACT_setHealth(i,1000);
               }
               break;
          case SPR_CYBERDEMONBOSSTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,7);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,15000);
               }
               break;
          case SPR_SPECTRETAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,2);
                    ACT_setChaseSpeed(i,2);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_1);
                    ACT_setHealth(i,2500);
               }
               break;
          case SPR_ARCHVILETAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,2);
                    ACT_setChaseSpeed(i,2);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,10000);
               }
               break;
          case SPR_CHAINGUNNERSTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_4);
                    ACT_setHealth(i,2500);
               }
               break;
#if 0
          case SPR_REVENANTTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,3);
                    ACT_setChaseSpeed(i,3);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,2500);
               }
               break;
#endif
          case SPR_MANCUBUSTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,5);
                    ACT_setChaseSpeed(i,5);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,5000);
               }
               break;
          case SPR_ARACHNOTRONTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,5);
                    ACT_setChaseSpeed(i,5);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_6);
                    ACT_setHealth(i,5000);
               }
               break;
          case SPR_HELLKNIGHTTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,5000);
               }
               break;
          case SPR_PAINELEMENTALTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,2000);
               }
               break;
          case SPR_WOLFSSTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_4);
                    ACT_setHealth(i,2000);
               }
               break;
          case SPR_BOSSBRAINTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,7);
                    ACT_setWeapon(i,GUN_4);
                    ACT_setHealth(i,15000);
               }
               break;
          case SPR_BOSSSHOOTERTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,7);
                    ACT_setWeapon(i,GUN_3);
                    ACT_setHealth(i,15000);
               }
               break;
          case SPR_IMPTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,1000);
               }
               break;
          case SPR_DEMONTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,2);
                    ACT_setChaseSpeed(i,2);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_1);
                    ACT_setHealth(i,1000);
               }
               break;
          case SPR_BARONTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_6);
                    ACT_setHealth(i,5000);
               }
               break;
          case SPR_FORMERHUMANTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,0);
                    ACT_setWeapon(i,GUN_2);
                    ACT_setHealth(i,500);
               }
               break;
          case SPR_CACODEMONTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,4);
                    ACT_setChaseSpeed(i,4);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_5);
                    ACT_setHealth(i,5000);
               }
               break;
          case SPR_LOSTSOULTAG:
               if (spr->statnum == STAT_NONE) {
                    ACT_changeState(spr,i,STAT_NONE,STAT_GUARD);
                    ACT_setWalkSpeed(i,6);
                    ACT_setChaseSpeed(i,2);
                    ACT_setAggression(i,5);
                    ACT_setWeapon(i,GUN_3);
                    ACT_setHealth(i,2500);
               }
               break;
          }
          if (ACT_isActor(i)) {
               spr->cstat|=(SPRC_BLOCKING|SPRC_BLOCKINGH);
               ENG_checksumSprite(i);
               if (noEnemiesFlag) {
                    ENG_deletesprite(i);
               }
               else {
                    ACT_setupActor(i);
               }
          }
     }
}

void
ACT_scanMap(void)
{
     short i;

     for (i=0 ; i < MAXSPRITES ; i++) {
          ACT_scanSprite(i);
     }
}

void
ACT_getBonusSectorItem(short snum,short sectnum)
{
     short item,n;

     item=EFF_getBonusSectorItem(sectnum);
     switch (item) {
     case BON_HEALTH:
          n=EFF_getBonusSectorValue(sectnum);
          ACT_healActor(snum,n);
          EFF_deleteBonusSector(sectnum);
          break;
     case BON_BULLETAMMO:
          n=EFF_getBonusSectorValue(sectnum);
          WEP_adjAmmo(snum,WEAPON_BULLET,n);
          EFF_deleteBonusSector(sectnum);
          break;
     case BON_MISSILEAMMO:
          n=EFF_getBonusSectorValue(sectnum);
          WEP_adjAmmo(snum,WEAPON_MISSILE,n);
          EFF_deleteBonusSector(sectnum);
          break;
     case BON_ENERGYAMMO:
          n=EFF_getBonusSectorValue(sectnum);
          WEP_adjAmmo(snum,WEAPON_ENERGY,n);
          EFF_deleteBonusSector(sectnum);
          break;
     case BON_MINEAMMO:
          n=EFF_getBonusSectorValue(sectnum);
          WEP_adjAmmo(snum,WEAPON_MINE,n);
          EFF_deleteBonusSector(sectnum);
          break;
     }
}

void
ACT_newSector(short s,short *osec,short *nsec,short *a,long *x,long *y,long *z)
{
     SND_playLeaveSectorSounds(*osec);
     SND_playEnterSectorSounds(*nsec);
//** perform new/old sector effects
     EFF_doEnterSectorTags(s,nsec,a,x,y,z);
     EFF_doLeaveSectorTags(s,osec,a,x,y,z);
}

short
ACT_getAttackWaitDelay(short s)
{
     short a,max,min,t;

     a=ACT_getAggression(s);
     t=krand();
     switch (curSkill) {
     case SKILL_CORPORAL:
          min=TICWAITPERFRAME<<4;
          break;
     case SKILL_LIEUTENANT:
          min=TICWAITPERFRAME<<3;
          break;
     case SKILL_CAPTAIN:
          min=TICWAITPERFRAME<<2;
          break;
     case SKILL_MAJOR:
     case SKILL_PRESIDENT:
          t=TICWAITPERFRAME;
          break;
     }
     max=TICWAITPERFRAME<<(7-a);
     if (t < min) {
          t+=min;
     }
     if (t > min+max) {
          t=min+max;
     }
     return(t);
}

void
ACT_attackSprite(spritetype *spr,short s,short target)
{
     if (ACT_isDead(target)) {
          ACT_changeState(spr,s,spr->statnum,STAT_WANDER);
          return;
     }
     ACT_setAttackee(spr,target);
     if (spr->statnum != STAT_CHASE) {
          ACT_changeState(spr,s,spr->statnum,STAT_CHASE);
          SND_playEnemy1SightSound(s);
     }
}

long
ACT_getAttackRange(short s)
{
     struct weaponData *weapPtr;

     weapPtr=WEP_getSpriteWeaponPtr(s);
     if (weapPtr->hitscanFlag) {
          return(0);
     }
     return((long)weapPtr->range);
}

long
ACT_getDistance(spritetype *src,spritetype *trg)
{
     long dx,x,y;

     x=(trg->x-src->x);
     y=(trg->y-src->y);
     dx=ksqrt(dmulscale(x,x,y,y,0));
     return(dx);
}

void
ACT_setAttackAngle(spritetype *src,spritetype *trg)
{
     short ang;
     long adj,dx;
     static long lastAttackClock;

     if (fakeClock < lastAttackClock) {
          return;
     }
     lastAttackClock=fakeClock+TMR_getSecondFraction(TICS_ONESIXTEENTH);
     ang=src->ang;
     ang=WEP_getAngle(src,trg);
     dx=WEP_getDistance(src,trg);
     adj=dx>>10;
     if (adj > 56) {     // deviate a max of +/- 10 degrees based on distance
          adj=56;
     }
     adj+=krand()&0x1F;
     if (krand()&1) {
          ang+=adj;
     }
     else {
          ang-=adj;
     }
     src->ang=(ang&2047);
}

short
ACT_getNextFootStep(spritetype *spr,short s,spritetype *trg)
{
     short count,i,p,n;
     struct player *plr;
     spritetype *fspr;

     if (PLR_isPlayer(trg,-1)) {
          if ((p=PLR_getPlayerNumber(trg,-1)) >= 0) {
               plr=player[p];
               n=plr->footSteps;
               for (count=0 ; count < MAXFOOTSTEPS ; count++) {
                    if ((i=plr->footStepSprite[n]) >= 0) {
                         fspr=spritePtr[i];
                         if (WEP_canSee(spr,fspr,2048)) {
                              return(i);
                         }
                    }
                    n=(n-1)&(MAXFOOTSTEPS-1);
               }
          }
     }
     return(-1);
}

short
ACT_getAttackFollowAngle(spritetype *spr,short s,spritetype *trg)
{
     short i;
     spritetype *fspr;

     if ((i=nextSprite[s]) >= 0) { // following footprints
          fspr=spritePtr[i];
          if (WEP_canSee(spr,fspr,2048)) {
               if (WEP_getDistance(spr,fspr) < 32L) {
                    nextSprite[s]=ACT_getNextFootStep(spr,s,trg);
                    if (nextSprite[s] == -1) {
                         return(-1);
                    }
                    fspr=spritePtr[nextSprite[s]];
               }
               return(WEP_getAngle(spr,fspr));
          }
          else {
               if ((nextSprite[s]=ACT_getNextFootStep(spr,s,trg)) == -1) {
                    return(-1);
               }
          }
     }
     return(spr->ang);
}

int
ACT_gonnaFire(short s)
{
     short attackee,
          attackers,
          morale;
     spritetype *spr;

     spr=spritePtr[s];
     attackee=ACT_getAttackee(spr);
     attackers=ACT_getAttackers(attackee);
     morale=ACT_getHealth(s);
     if (morale >= ACT_getHealth(attackee)) {     // healthier than attackee
          return(1);
     }
     else if (morale >= (playerMaxHealth>>1)) {   // 50% or greater health
          if (attackers > 1) {                    // not the only attacker
               return(1);
          }
          else if ((krand()&0x20) == 0x20) {
               return(1);
          }
          else {
               ACT_changeState(spr,s,spr->statnum,STAT_DODGE);
          }
     }
     else if ((krand()&0x20) == 0x20) {
          if (krand() >= 16384 && (krand()&16384) == 16384) {
               return(1);
          }
     }
     else {
          ACT_changeState(spr,s,spr->statnum,STAT_DODGE);
     }
     return(0);
}

void
ACT_setDodgeAngle(spritetype *spr,short s)
{
     short a,k;

     k=krand();
     a=krand()&767;
     if ((k&0x20) == 0x20) {
          spr->ang=(spr->ang-a)&2047;
     }
     else {
          spr->ang=(spr->ang+a)&2047;
     }
#if 0
     short k;

     if (((k=krand())&0x20) == 0x20) {
          return((spritePtr[s]->ang+512)&2047);
     }
     else if ((k&0x02) == 0x02) {
          return((spritePtr[s]->ang-512)&2047);
     }
     else {
          return((spritePtr[s]->ang+1024)&2047);
     }
#endif
}
     
unsigned short
ACT_moveSprite(short s,long *x,long *y,long *z,short *sect,
               long xvect,long yvect,short cdist,short cheight,short ctype)
{
     unsigned short o;
     spritetype *spr;
     sectortype *sectPtr;

     spr=spritePtr[s];
     o=clipmove(x,y,z,sect,xvect,yvect,cdist,cheight,cheight,ctype);
     if (*sect != spr->sectnum && *sect >= 0) {
          changespritesect(s,*sect);
     }
     spr->cstat&=~(SPRC_BLOCKING|SPRC_BLOCKINGH);
     getzrange(*x,*y,*z,*sect,&globhiz,&globhihit,&globloz,&globlohit,cdist,0);
     sectPtr=sectorPtr[*sect];
     if ((sectPtr->floorstat&0x02) != 0 || (sectPtr->ceilingstat&0x02) != 0) {
          getzsofslope(*sect,*x,*y,&globhiz,&globloz);
     }
     spr->cstat|=(SPRC_BLOCKING|SPRC_BLOCKINGH);
     return(o);
}

void
ACT_moveActor(spritetype *spr,short s)
{
     short actorheight,ang,clipdist,clipheight,sect,speed;
     unsigned short o;
     long xvect,yvect,x,y,z;
     short nearsector,nearwall,nearsprite;
     long neardist;

     ang=spr->ang;
     clipheight=ACT_getActorClipHeight(spr);
     clipdist=ACT_getActorClipDist(spr);
     actorheight=ACT_getActorViewHeight(spr);
     switch (spr->statnum) {
     case STAT_CHASING:
     case STAT_DODGING:
     case STAT_PAIN:
          speed=ACT_getChaseSpeed(s);
          break;
     case STAT_WANDER:
     case STAT_PATROL:
          speed=ACT_getWalkSpeed(s);
          break;
     default:
          speed=7;
          break;
     }
     x=spr->x;
     y=spr->y;
     z=spr->z-actorheight;
     sect=spr->sectnum;
     xvect=0L;
     yvect=0L;
     if (speed < 7) {
          if (forceVel[s] != 0) {
               xvect=mulscale2((long)forceVel[s],
                               (long)sintable[(forceAng[s]+512)&2047]);
               yvect=mulscale2((long)forceVel[s],
                               (long)sintable[forceAng[s]]);
               forceVel[s]=kmax(forceVel[s]-(TICWAITPERFRAME<<2),0);
          }
          if (waitDoor[s] == -1) {
               xvect+=(mulscale(sintable[(ang+512)&2047],TICWAITPERFRAME,
                                speed+MINSPEED)<<12);
               yvect+=(mulscale(sintable[ang],TICWAITPERFRAME,
                                speed+MINSPEED)<<12);
          }
     }
     o=ACT_moveSprite(s,&x,&y,&z,&sect,xvect,yvect,clipdist,clipheight,0);
     if (waitDoor[s] != -1) {
          if (EFF_isDoorOpened(waitDoor[s])) {
               waitDoor[s]=-1;
          }
     }
     else if (globloz < z || globhiz > z) {
          x=spr->x;
          y=spr->y;
          z=spr->z-actorheight;
          sect=spr->sectnum;
          neartag(x,y,z,sect,ang,&nearsector,&nearwall,&nearsprite,&neardist,
                  1024L,1);
          if (nearsector == -1) {
               nearsector=sect;
          }
          if (EFF_operatableSector(nearsector)) {
               if (EFF_isDoorClosed(nearsector)) {
                    EFF_openDoor(nearsector);
                    waitDoor[s]=nearsector;
               }
          }
          else {
               ACT_changeState(spr,s,spr->statnum,STAT_DODGE);
          }
     }
     else if (o != 0) {
          if ((o&32768) == 32768) {     // hit a wall
               neartag(x,y,z,sect,ang,&nearsector,&nearwall,&nearsprite,
                       &neardist,1024L,1);
               if (nearsector == -1) {
                    nearsector=sect;
               }
               if (EFF_operatableSector(nearsector)) {
                    if (EFF_isDoorClosed(nearsector)) {
                         EFF_openDoor(nearsector);
                         waitDoor[s]=nearsector;
                    }
               }
               else {
                    if ((krand()&0x20) == 0x20) {
                         spr->ang=ENG_getParallel(o&32767);
                    }
                    else {
                         spr->ang=ENG_getNormal(o&32767);
                    }
                    ACT_changeState(spr,s,spr->statnum,STAT_DODGING);
               }
          }
          if ((o&49152) == 49152) {     // hit a sprite
               ACT_changeState(spr,s,spr->statnum,STAT_DODGE);
          }
     }
     GAM_doPhysics(s,spr,&x,&y,&z,&sect,actorheight,actorheight,clipheight);
     setsprite(s,x,y,z+actorheight);
#if 0
     short ang,osect,sect,sn,speed,w;
     unsigned short o;
     long dax,day,daz=0L,floorz,ox,oy;
     spritetype *spr;

     spr=spritePtr[s];
     ang=spr->ang;
     if (spr->statnum == STAT_CHASING || spr->statnum == STAT_DODGING) {
          speed=ACT_getChaseSpeed(s);
     }
     else {
          speed=ACT_getWalkSpeed(s);
     }
     floorz=sectorPtr[spr->sectnum]->floorz;
     globloz=floorz;
     if (waitDoor[s] == -1) {
          if (speed < 7) {
               dax=(sintable[(ang+512)&2047])>>(speed+MINSPEED);
               day=(sintable[ang])>>(speed+MINSPEED);
               daz=0L;
          }
          else {
               dax=0L;
               day=0L;
          }
          if (hitwallDelay[s] > 0) {
               hitwallDelay[s]-=TICWAITPERFRAME;
          }
          dax=(dax*TICWAITPERFRAME)<<12;
          day=(day*TICWAITPERFRAME)<<12;
          ox=spr->x;
          oy=spr->y;
          osect=spr->sectnum;
          o=movesprite(s,dax,day,daz,4L<<8,4L<<8,0);
          if (globloz > spr->z+STAIRHEIGHT) {
               spr->x=ox;
               spr->y=oy;
               spr->ang=(spr->ang+(krand()&1023))&2047;
               globloz=floorz;
          }
          else if (globloz < spr->z-STAIRHEIGHT) {
               spr->x=ox;
               spr->y=oy;
               spr->ang=(spr->ang+(krand()&1023))&2047;
               globloz=floorz;
          }
          if (o != 0) {
               if ((o&0xC000) == 0xC000) {
                    sn=o&(MAXSPRITES-1);
                    if (ACT_isActor(sn) && actorPic[s] != actorPic[sn]) {
                         ACT_attackSprite(spr,s,sn);
                    }
                    else {
                         spr->ang=(spr->ang+(krand()&1023))&2047;
                    }
               }
               else if ((o&0x8000) == 0x8000) {
                    neartag(spr->x,spr->y,spr->z,spr->sectnum,spr->ang,
                            &sect,&w,&sn,&dax,1048L,1);
                    if (sect >= 0) {
                         if (EFF_isDoor(sect) || EFF_isElevator(sect)) {
                              if (EFF_isDoorClosed(sect)) {
                                   EFF_openDoor(sect);
                              }
                              waitDoor[s]=sect;
                         }
                    }
                    else {
                         w=o&(MAXWALLS-1);
                         spr->ang=ENG_getParallel(w);
                         if (walk180[s>>3]&(1<<(s&7))) {
                              spr->ang=(spr->ang+1024)&2047;
                         }
                         if (hitwallDelay[s] <= 0) {
                              hitwallDelay[s]=TICWAITPERFRAME<<8;
                              if ((krand()&0x20) == 0x20) {
                                   walk180[s>>3]|=(1<<(s&7));
                              }
                              else {
                                   walk180[s>>3]&=~(1<<(s&7));
                              }
                         }
                    }
               }
          }
          else if ((globlohit&0xC000) == 0xC000) {
               sn=globlohit&(MAXSPRITES-1);
               EFF_onSprite(s,sn,&spr->ang,&spr->sectnum,&spr->x,&spr->y,
                            &spr->z);
          }
          if (osect != spr->sectnum) {
               ACT_newSector(s,&osect,&spr->sectnum,&spr->ang,
                             &spr->x,&spr->y,&spr->z);
          }
     }
     else {
          if (EFF_isDoorOpened(waitDoor[s])) {
               waitDoor[s]=-1;
          }
          else if (EFF_isDoorClosed(waitDoor[s])) {
               EFF_openDoor(waitDoor[s]);
          }
     }
     dax=spr->x;
     day=spr->y;
     daz=spr->z;
     GAM_doPhysics(s,globloz,&dax,&day,&daz);
     setsprite(s,dax,day,daz);
#endif
}

short
ACT_canSeePlayer(spritetype *spr,short fov)
{
     short j,nextj,vis;
     long dx;
     spritetype *trg;

     for (j=headspritestat[STAT_PLAYER] ; j >= 0 ; j=nextj) {
          nextj=nextspritestat[j];
          trg=spritePtr[j];
          if ((dx=WEP_getDistance(spr,trg)) > 10000L) {
               continue;
          }
          if (dx < 2048L) {
               if (WEP_canSee(spr,trg,2048)) {
                    return(j);
               }
               return(-1);
          }
          vis=WEP_canSee(spr,trg,fov);
          if (vis) {
               return(j);
          }
     }
     return(-1);
}

short
ACT_findPath(spritetype *spr,short fov)
{
     short i,nexti;
     spritetype *trg;

     for (i=headspritestat[STAT_PATHMARKER] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          trg=spritePtr[i];
          if (WEP_canSee(spr,trg,fov)) {
               return(i);
          }
     }
     return(-1);
}

void
ACT_setSpritePath(spritetype *spr,short s,short fov)
{
     short o;

     o=ACT_findPath(spr,fov);
     if (o >= 0 && o != abs(nextSprite[s])-1) {
          nextSprite[s]=o;
     }
}

void
ACT_doStatusPatrol(spritetype *spr,short i)
{
     short j;

     firingWeapon[i]=0;
     if ((j=ACT_canSeePlayer(spr,256)) >= 0 || (j=ACT_getAttackee(spr)) >= 0) {
          ACT_attackSprite(spr,i,j);
          nextSprite[i]=-1;
          return;
     }
     if (nextSprite[i] < 0 || spritePtr[nextSprite[i]]->statnum >= MAXSTATUS) {
          ACT_setSpritePath(spr,i,256);
     }
     else {
          if (WEP_getDistance(spr,spritePtr[nextSprite[i]]) <= 256L) {
               spr->ang=spritePtr[nextSprite[i]]->ang;
               nextSprite[i]=-(nextSprite[i]+1);
          }
          else {
               spr->ang=WEP_getAngle(spr,spritePtr[nextSprite[i]]);
          }
     }
}

void
ACT_doStatusWander(spritetype *spr,short i)
{
     short j;

     firingWeapon[i]=0;
     attackDelay[i]=0;
     nextSprite[i]=-1;
     if ((j=ACT_canSeePlayer(spr,2048)) >= 0 || (j=ACT_getAttackee(spr)) >= 0) {
          ACT_attackSprite(spr,i,j);
          return;
     }
     ACT_setSpritePath(spr,i,2048);
     if (nextSprite[i] >= 0) {
          ACT_changeState(spr,i,STAT_WANDER,STAT_PATROL);
          return;
     }
}

void
ACT_doStatusAmbush(spritetype *spr,short i)
{
     short j;

     firingWeapon[i]=0;
     if ((j=ACT_canSeePlayer(spr,1024)) >= 0 || (j=ACT_getAttackee(spr)) >= 0) {
          ACT_attackSprite(spr,i,j);
     }
}

void
ACT_doStatusGuard(spritetype *spr,short i)
{
     short j,s;
     spritetype *spr2;

     firingWeapon[i]=0;
     if ((s=ACT_getMadeNoise()) >= 0 && PLR_isPlayer(NULL,s)) {
          spr2=spritePtr[s];
          if (WEP_getDistance(spr2,spr) < 10000L) {
               ACT_attackSprite(spr,i,s);
          }
     }
     if ((j=ACT_canSeePlayer(spr,256)) >= 0 || (j=ACT_getAttackee(spr)) >= 0) {
          ACT_attackSprite(spr,i,j);
     }
}

void
ACT_doStatusPain(spritetype *spr,short i)
{
#if 0
     if (ACT_getPainDelay(i) == 0) {
          ACT_changeState(spritePtr[i],i,STAT_PAIN,STAT_DODGE);
     }
#endif
}

void
ACT_doStatusChase(spritetype *spr,short i)
{
     short a,attackee;
     long dx;
     spritetype *trg;

     firingWeapon[i]=0;
     attackee=ACT_getAttackee(spr);
     if (attackee < 0 || ACT_isDead(attackee)) {
          ACT_setAttackee(spr,-1);
          ACT_changeState(spr,i,STAT_CHASING,STAT_WANDER);
          return;
     }
     trg=spritePtr[attackee];
     dx=WEP_getDistance(spr,trg);
     if (WEP_canSee(spr,trg,2048)) {
          ACT_setAttackAngle(spr,trg);
          nextSprite[i]=-1;
          if (attackDelay[i] > 0) {
               attackDelay[i]-=TICWAITPERFRAME;
          }
          else {
               if (ACT_gonnaFire(i)) {
                    ACT_changeState(spr,i,STAT_CHASING,STAT_ATTACK);
                    return;
               }
          }
     }
     else if (nextSprite[i] == -1) {
          if ((nextSprite[i]=ACT_getNextFootStep(spr,i,trg)) == -1) {
               ACT_changeState(spr,i,STAT_CHASING,STAT_WANDER);
               ACT_setAttackee(spr,-1);
          }
     }
     else {
          if ((a=ACT_getAttackFollowAngle(spr,i,trg)) == -1) {
               ACT_changeState(spr,i,STAT_CHASING,STAT_AMBUSH);
               ACT_setAttackee(spr,-1);
          }
          else {
               spr->ang=a;
          }
     }
}

void
ACT_doStatusAttack(spritetype *spr,short i)
{
     short attackee;

     attackee=ACT_getAttackee(spr);
     if (attackee < 0 || ACT_isDead(attackee)) {
          if (ACT_stillActor(i)) {
               ACT_changeState(spr,i,STAT_ATTACKING,STAT_AMBUSH);
          }
          else {
               ACT_changeState(spr,i,STAT_ATTACKING,STAT_WANDER);
          }
          firingWeapon[i]=0;
          ACT_setAttackee(spr,-1);
          return;
     }
     ACT_setAttackAngle(spr,spritePtr[attackee]);
     if (WEP_getAmmo(i,WEP_getSpriteAmmoType(i)) == 0) {
          firingWeapon[i]=0;
          WEP_defaultAmmo(i);
     }
     else if (!WEP_firingWeapon(i)) {
          firingWeapon[i]=0;
          attackDelay[i]=ACT_getAttackWaitDelay(i);
          ACT_changeState(spr,i,STAT_ATTACKING,STAT_DODGE);
     }
}

void
ACT_doStatusDodge(spritetype *spr,short i)
{
     short attackee;

     firingWeapon[i]=0;
     attackee=ACT_getAttackee(spr);
     if (ACT_stillActor(i)) {
          ACT_changeState(spr,i,STAT_DODGING,STAT_CHASE);
          return;
     }
     if (attackee < 0 || ACT_isDead(attackee)) {
          ACT_changeState(spr,i,STAT_DODGING,STAT_WANDER);
          ACT_setAttackee(spr,-1);
          return;
     }
     if (attackDelay[i] > 0) {
          attackDelay[i]-=TICWAITPERFRAME;
          if (attackDelay[i] <= 0) {
               attackDelay[i]=ACT_getAttackWaitDelay(i);
               ACT_changeState(spr,i,STAT_DODGING,STAT_CHASE);
          }
     }
}

void
ACT_doStatusCode(void)
{
     short i,nexti,o;
     spritetype *spr;
     
     if (editorEnabledFlag && gameModeFlag == 0) {
          return;
     }
     if (noEnemiesFlag) {
          return;
     }
     for (i=headspritestat[STAT_PATROL] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          WEP_spriteFireWeapon(i);
          ACT_doStatusPatrol(spr,i);
          ACT_moveActor(spr,i);
     }
     for (i=headspritestat[STAT_WANDER] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          WEP_spriteFireWeapon(i);
          ACT_doStatusWander(spr,i);
          ACT_moveActor(spr,i);
     }
     for (i=headspritestat[STAT_AMBUSH] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          WEP_spriteFireWeapon(i);
          ACT_doStatusAmbush(spr,i);
          ACT_moveActor(spr,i);
     }
     for (i=headspritestat[STAT_GUARD] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          WEP_spriteFireWeapon(i);
          ACT_doStatusGuard(spr,i);
          ACT_moveActor(spr,i);
     }
     for (i=headspritestat[STAT_PAIN] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          WEP_spriteFireWeapon(i);
          ACT_doStatusPain(spr,i);
          ACT_moveActor(spr,i);
     }
     for (i=headspritestat[STAT_CHASE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          attackDelay[i]=ACT_getAttackWaitDelay(i);
          ACT_changeState(spritePtr[i],i,STAT_CHASE,STAT_CHASING);
     }
     for (i=headspritestat[STAT_CHASING] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          WEP_spriteFireWeapon(i);
          ACT_doStatusChase(spr,i);
          ACT_moveActor(spr,i);
     }
     for (i=headspritestat[STAT_ATTACK] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          firingWeapon[i]=ACT_getRoundsToFire(i);
          ACT_changeState(spritePtr[i],i,STAT_ATTACK,STAT_ATTACKING);
     }
     for (i=headspritestat[STAT_ATTACKING] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          o=WEP_getAmmo(i,WEP_getSpriteAmmoType(i));
          WEP_spriteFireWeapon(i);
          ACT_doStatusAttack(spr,i);
          if (WEP_getAmmo(i,WEP_getSpriteAmmoType(i)) < o) {
               firingWeapon[i]=kmax(firingWeapon[i]-1,0);
          }
     }
     for (i=headspritestat[STAT_DODGE] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          ACT_setDodgeAngle(spr,i);
          ACT_changeState(spr,i,STAT_DODGE,STAT_DODGING);
     }
     for (i=headspritestat[STAT_DODGING] ; i >= 0 ; i=nexti) {
          nexti=nextspritestat[i];
          spr=spritePtr[i];
          if (attackDelay[i] <= 0) {
               attackDelay[i]=ACT_getAttackWaitDelay(i);
          }
          WEP_spriteFireWeapon(i);
          ACT_doStatusDodge(spr,i);
          ACT_moveActor(spr,i);
     }
}

void
ACT_saveGame(FILE *fp)
{
     short i,k;

     GAM_fwrite(spriteHoriz,sizeof(short),MAXSPRITES,fp);
     for (k=STAT_PATROL ; k <= STAT_DODGING ; k++) {
          for (i=headspritestat[k] ; i >= 0 ; i=nextspritestat[i]) {
               GAM_fwrite(&currentWeapon[i],sizeof(char),1,fp);
               GAM_fwrite(&ammo[i],sizeof(short),MAXWEAPONTYPES,fp);
               GAM_fwrite(&attackers[i],sizeof(char),1,fp);
               GAM_fwrite(&fireFrame[i],sizeof(char),1,fp);
               GAM_fwrite(&firingWeapon[i],sizeof(char),1,fp);
               GAM_fwrite(&painDelay[i],sizeof(char),1,fp);
               GAM_fwrite(&actorPic[i],sizeof(short),1,fp);
               GAM_fwrite(&attackDelay[i],sizeof(short),1,fp);
               GAM_fwrite(&hitwallDelay[i],sizeof(short),1,fp);
               GAM_fwrite(&spriteHoriz[i],sizeof(short),1,fp);
               GAM_fwrite(&waitDoor[i],sizeof(short),1,fp);
               GAM_fwrite(&forceAng[i],sizeof(short),1,fp);
               GAM_fwrite(&forceVel[i],sizeof(short),1,fp);
          }
     }
     GAM_fwrite(walk180,sizeof(char),(MAXSPRITES>>3),fp);
}

void
ACT_loadGame(FILE *fp)
{
     short i,k;

     GAM_fread(spriteHoriz,sizeof(short),MAXSPRITES,fp);
     for (k=STAT_PATROL ; k <= STAT_DODGING ; k++) {
          for (i=headspritestat[k] ; i >= 0 ; i=nextspritestat[i]) {
               GAM_fread(&currentWeapon[i],sizeof(char),1,fp);
               GAM_fread(&ammo[i],sizeof(short),MAXWEAPONTYPES,fp);
               GAM_fread(&attackers[i],sizeof(char),1,fp);
               GAM_fread(&fireFrame[i],sizeof(char),1,fp);
               GAM_fread(&firingWeapon[i],sizeof(char),1,fp);
               GAM_fread(&painDelay[i],sizeof(char),1,fp);
               GAM_fread(&actorPic[i],sizeof(short),1,fp);
               GAM_fread(&attackDelay[i],sizeof(short),1,fp);
               GAM_fread(&hitwallDelay[i],sizeof(short),1,fp);
               GAM_fread(&spriteHoriz[i],sizeof(short),1,fp);
               GAM_fread(&waitDoor[i],sizeof(short),1,fp);
               GAM_fread(&forceAng[i],sizeof(short),1,fp);
               GAM_fread(&forceVel[i],sizeof(short),1,fp);
          }
     }
     GAM_fread(walk180,sizeof(char),(MAXSPRITES>>3),fp);
}

void
ACT_debug(void)
{
}

