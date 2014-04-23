/***************************************************************************
 *   CORR7PLR.C - Player related functions for Corridor 7
 *
 *                                                     02/29/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   VELSPEED       768
#define   TURNSPEED      128
#define   MAXMOUSEXVEL   1536
#define   MAXMOUSEYVEL   1536

#define   DIEMESSAGE     "PRESS SPACEBAR TO CONTINUE"

char keys[MAXKEYS],
     okeys[256];

int  currentView,
     gammaCorrection,
     localViewSize,
     mouseEnabled=1,
     mouseXSens=8,
     mouseYSens=8,
     viewWeapon;

int  doMainMenuFlag,
     horizCheckFlag=1,
     singleStepFlag;

int  playerHealth=1000,
     playerMaxHealth;

int  stfspeed=6;

unsigned
long playerInventory[MAXPLAYERS];

long trnvel,locavel,
     fwdvel,locfvel,
     stfvel,locsvel;

long omoves[MAXPLAYERS];

short horizCheckSprite=-1,
     horizSlopeGoal[MAXSPRITES];

short savemap;

struct player plr[MAXPLAYERS],
     *player[MAXPLAYERS];

extern
int  dumpDebugInfoFlag,
     showFrameRateFlag;

void
PLR_init(void)
{
     int  i;

     for (i=0 ; i < MAXPLAYERS ; i++) {
          player[i]=&plr[i];
     }
}

void
PLR_unInit(void)
{
}

void
PLR_initMouse(void)
{
     mouseEnabled=configMouse[MOUSE_ENABLE];
     if (mouseEnabled || editorEnabledFlag) {
          initmouse();
     }
}

int
PLR_getMouseXSense(void)
{
     return(mouseXSens);
}

void
PLR_setMouseXSense(int n)
{
     mouseXSens=n;
}

int
PLR_getMouseYSense(void)
{
     return(mouseYSens);
}

void
PLR_setMouseYSense(int n)
{
     mouseYSens=n;
}

int
PLR_isPlayer(spritetype *spr,short s)
{
     if (spr != NULL && spr->statnum == STAT_PLAYER) {
          return(1);
     }
     if (s >= 0 && spritePtr[s]->statnum == STAT_PLAYER) {
          return(1);
     }
     return(0);
}

short
PLR_getPlayerNumber(spritetype *spr,short s)
{
     if (spr != NULL) {
          return(spr->owner&4095);
     }
     else if (s >= 0) {
          return(spritePtr[s]->owner&4095);
     }
     return(-1);
}

short
PLR_getPlayerSprite(short p)
{
     return(player[p]->spriteNum);
}

void
PLR_setMyWeapon(short gun)
{
     currentWeapon[PLR_getPlayerSprite(myconnectindex)]=gun;
}

short
PLR_getMyWeapon(void)
{
     return(currentWeapon[PLR_getPlayerSprite(myconnectindex)]);
}

void
PLR_setViewWeapon(short gun)
{
     viewWeapon=gun;
}

short
PLR_getViewWeapon(void)
{
     return(viewWeapon);
}

void
PLR_addInventoryItem(short s,short i)
{
     short p;

     if (PLR_isPlayer(NULL,s)) {
          p=PLR_getPlayerNumber(NULL,s);
          if (p >= 0 && p < MAXPLAYERS) {
               playerInventory[p]|=(1<<i);
          }
     }
}

void
PLR_removeInventoryItem(short s,short i)
{
     short p;

     if (PLR_isPlayer(NULL,s)) {
          p=PLR_getPlayerNumber(NULL,s);
          if (p >= 0 && p < MAXPLAYERS) {
               playerInventory[p]&=~(1<<i);
          }
     }
}

int
PLR_hasInventoryItem(short s,short i)
{
     short p;

     if (PLR_isPlayer(NULL,s)) {
          p=PLR_getPlayerNumber(NULL,s);
          if (p >= 0 && p < MAXPLAYERS) {
               if ((playerInventory[p]&(1<<i)) != 0) {
                    return(1);
               }
          }
     }
     return(0);
}

void
PLR_updateStatusHealth(void)
{
     short i,len=0,pic,s;
     long x,y;
     char locbuf[5];

     if (qsetmode != 200L) {
          return;
     }
     s=GAM_getViewSprite();
     sprintf(locbuf,"%d",ACT_getHealth(s)/10);
     for (i=0 ; i < strlen(locbuf) ; i++) {
          len+=tilesizx[graphicBigNum0Pic+locbuf[i]-'0'];
     }
     x=(320>>1)-80L-(len>>1);
     y=200-20L;
     for (i=0 ; i < strlen(locbuf) ; i++) {
          pic=graphicBigNum0Pic+locbuf[i]-'0';
          rotatesprite(x<<16,y<<16,65536L,0,pic,0,0,2+8+128,
                       0L,0L,xdim-1L,ydim-1L);
          x+=tilesizx[pic];
     }
}

void
PLR_updateStatusAmmo(void)
{
     short i,len=0,pic,s;
     long x,y;
     char locbuf[5];

     if (qsetmode != 200L) {
          return;
     }
     s=GAM_getViewSprite();
     sprintf(locbuf,"%d",WEP_getAmmo(s,WEP_getSpriteAmmoType(s)));
     for (i=0 ; i < strlen(locbuf) ; i++) {
          len+=tilesizx[graphicBigNum0Pic+locbuf[i]-'0'];
     }
     x=(320>>1)-130L-(len>>1);
     y=200-20L;
     for (i=0 ; i < strlen(locbuf) ; i++) {
          pic=graphicBigNum0Pic+locbuf[i]-'0';
          rotatesprite(x<<16,y<<16,65536L,0,pic,0,0,2+8+128,
                       0L,0L,xdim-1L,ydim-1L);
          x+=tilesizx[pic];
     }
}

void
PLR_updateStatusAliens(void)
{
     short i,len=0,pic;
     long x,y;
     char locbuf[5];

     if (qsetmode != 200L) {
          return;
     }
     sprintf(locbuf,"%d",GAM_getAliensLeft());
     for (i=0 ; i < strlen(locbuf) ; i++) {
          len+=tilesizx[graphicBigNum0Pic+locbuf[i]-'0'];
     }
     x=(320>>1)-30L-(len>>1);
     y=200-20L;
     for (i=0 ; i < strlen(locbuf) ; i++) {
          pic=graphicBigNum0Pic+locbuf[i]-'0';
          rotatesprite(x<<16,y<<16,65536L,0,pic,0,0,2+8+128,
                       0L,0L,xdim-1L,ydim-1L);
          x+=tilesizx[pic];
     }
}

void
PLR_updateStatusBar(void)
{
     if (currentView >= 0 && WEP_getMissileView() == -1) {
          PLR_updateStatusHealth();
          PLR_updateStatusAmmo();
          PLR_updateStatusAliens();
     }
}

void
PLR_checkStatusBar(void)
{
     short aliens,ammo,health,s,update=0;
     static int oldHealth,oldAmmo,oldAliens;

     s=GAM_getViewSprite();
     if ((health=ACT_getHealth(s)) != oldHealth) {
          update=1;
     }
     if ((ammo=WEP_getAmmo(s,WEP_getSpriteAmmoType(s))) != oldAmmo) {
          update=1;
     }
     if ((aliens=GAM_getAliensLeft()) != oldAliens) {
          update=1;
     }
     if (update) {
          oldHealth=health;
          oldAmmo=ammo;
          oldAliens=aliens;
          GAM_updateStatusBar();
     }
}

int
PLR_getKillScore(short s)
{
     return(1);
}

void
PLR_adjScore(short s)
{
     short k,p;

     k=ACT_getKiller(s);
     p=PLR_getPlayerNumber(NULL,k);
     switch (gameMode) {
     case GAMEMODE_DEATHMATCH:
          if (PLR_isPlayer(NULL,s)) {
               if (s != PLR_getPlayerSprite(myconnectindex)) {
                    if (k >= 0) {
                         if (p >= 0 && p < MAXPLAYERS) {
                              player[p]->score+=PLR_getKillScore(s);
                         }
                         if (k == PLR_getPlayerSprite(myconnectindex)) {
                              GAM_adjAliensLeft(1);
                         }
                    }
               }
          }
          break;
     default:
          if (ACT_isActor(s)) {
               if (k >= 0) {
                    if (p >= 0 && p < MAXPLAYERS) {
                         player[p]->score+=PLR_getKillScore(s);
                    }
               }
               GAM_adjAliensLeft(-1);
          }
          break;
     }
}

void
PLR_initPlayers(void)
{
     int  i,j;

     for (i=0 ; i < MAXPLAYERS ; i++) {
          player[i]->spriteNum=-1;
          for (j=0 ; j < MAXFOOTSTEPS ; j++) {
               player[i]->footStepSprite[j]=-1;
          }
     }
     currentView=0;
}

void
PLR_initPlayer(int p)
{
     player[p]->viewMode=3;
     player[p]->zoom=768L;
     player[p]->viewSize=localViewSize;
     if (p == currentView) {
          GFX_initPlayerShifts();
     }
}

void
PLR_initPlayerSprite(int p)
{
     int  i,j;
     long goalz;

     if (gameMode == GAMEMODE_DEATHMATCH) {
          if (numStartSpots == 0) {
               j=ENG_insertsprite(starts,STAT_PLAYER);
               spritePtr[j]->x=startx;
               spritePtr[j]->y=starty;
               spritePtr[j]->z=startz;
               spritePtr[j]->sectnum=starts;
               spritePtr[j]->ang=starta;
          }
          else {
               i=krand()%numStartSpots;
               j=ENG_insertsprite(startSpotDeathMatch[i].sectnum,STAT_PLAYER);
               spritePtr[j]->x=startSpotDeathMatch[i].x;
               spritePtr[j]->y=startSpotDeathMatch[i].y;
               spritePtr[j]->z=startSpotDeathMatch[i].z;
               spritePtr[j]->sectnum=startSpotDeathMatch[i].sectnum;
               spritePtr[j]->ang=startSpotDeathMatch[i].ang;
          }
     }
     else {
          if (p == connecthead || numStartSpots == 0) {
               j=ENG_insertsprite(starts,STAT_PLAYER);
               spritePtr[j]->x=startx;
               spritePtr[j]->y=starty;
               spritePtr[j]->z=startz;
               spritePtr[j]->sectnum=starts;
               spritePtr[j]->ang=starta;
          }
          else {
               i=krand()%numStartSpots;
               j=ENG_insertsprite(startSpot[i].sectnum,STAT_PLAYER);
               spritePtr[j]->x=startSpot[i].x;
               spritePtr[j]->y=startSpot[i].y;
               spritePtr[j]->z=startSpot[i].z;
               spritePtr[j]->sectnum=startSpot[i].sectnum;
               spritePtr[j]->ang=startSpot[i].ang;
          }
     }
     player[p]->spriteNum=j;
     spritePtr[j]->cstat=0;
     spritePtr[j]->picnum=enemyPic[curActor];
     spritePtr[j]->pal=p;
     spritePtr[j]->xrepeat=defaultPlayerWidth;
     spritePtr[j]->yrepeat=defaultPlayerHeight;
     spritePtr[j]->clipdist=spritePtr[j]->xrepeat;
     spritePtr[j]->owner=MAXSPRITES+p;
     getzrange(spritePtr[j]->x,spritePtr[j]->y,spritePtr[j]->z,
               spritePtr[j]->sectnum,&globhiz,&globhihit,
               &globloz,&globlohit,spritePtr[j]->clipdist<<1,0);
     spritePtr[j]->cstat|=(SPRC_BLOCKING|SPRC_BLOCKINGH);
     goalz=globloz;
     spritePtr[j]->z=goalz;
     GAM_onGround(j,1);
     GAM_underWater(j,0);
     ACT_setActorPic(j,spritePtr[j]->picnum);
     ACT_setHealth(j,playerHealth);
     spriteHoriz[j]=100;
     ACT_setKiller(j,-1);
     diez[p]=0L;
     if (p == myconnectindex) {
          show2dsprite[j>>3]|=(1<<(j&7));
     }
     currentWeapon[j]=GUN_1;
     fireFrame[j]=0;
     WEP_defaultAmmo(j);
     for (i=0 ; i < MAXFOOTSTEPS ; i++) {
          if ((j=player[p]->footStepSprite[i]) != -1) {
               if (spritePtr[j]->statnum < MAXSTATUS) {
                    ENG_deletesprite(j);
               }
          }
          player[p]->footStepSprite[i]=-1;
     }
     player[p]->footSteps=0;
}

void
PLR_initPlayerSprites(int nPlayers)
{
     int  i;

     if (ENG_editorGameMode()) {
          for (i=0 ; i < MAXPLAYERS ; i++) {
               connectpoint2[i]=-1;
          }
          currentView=connecthead;
          myconnectindex=connecthead;
          numplayers=1;
     }
     for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
          PLR_initPlayer(i);
          PLR_initPlayerSprite(i);
     }
}

void
PLR_useCheck(short snum,long x,long y,long z,short sector,short ang)
{
     short nearsector,nearwall,nearsprite;
     long neardist;

     neartag(x,y,z,sector,ang,&nearsector,&nearwall,&nearsprite,&neardist,
             2048L,1);
     if (nearsprite != -1) {       // using a sprite
          EFF_operateSprite(nearsprite);
     }
     if (nearwall != -1) {         // using a wall
          EFF_operateWall(nearwall);
     }
     if (!EFF_operatableSector(nearsector)) {
          if (EFF_operatableSector(sector)) {
               nearsector=sector;
          }
     }
     if (nearsector != -1 && sectorPtr[nearsector]->hitag == 0) {
          EFF_operateSector(nearsector);
     }
}

void
PLR_dieAnim(short p)
{
     short clipheight,playerHeight,snum;
     long z;

     snum=PLR_getPlayerSprite(p);
     playerHeight=ACT_getActorViewHeight(spritePtr[snum]);
     clipheight=ACT_getActorClipHeight(spritePtr[snum]);
     z=globloz-playerHeight+diez[p];
     if (z < globloz-clipheight) {
          diez[p]+=(TICWAITPERFRAME<<8);
     }
     else {
          z=playerHeight-clipheight;
     }
}

void
PLR_footStep(short p,spritetype *spr)
{
     short j,n;
     struct player *plr;
     spritetype *spr2;
     
     plr=player[p];
     n=plr->footSteps;
     j=plr->footStepSprite[n];
     if (j == -1 || WEP_getDistance(spr,spritePtr[j]) >= 512L) {
          if (j != -1) {
               plr->footSteps=(plr->footSteps+1)&(MAXFOOTSTEPS-1);
               n=plr->footSteps;
               j=plr->footStepSprite[n];
          }
          if (j == -1) {
               j=ENG_insertsprite(spr->sectnum,STAT_FOOTSTEP);
               plr->footStepSprite[n]=j;
          }
          spr2=spritePtr[j];
          spr2->x=spr->x;
          spr2->y=spr->y;
          spr2->z=sectorPtr[spr->sectnum]->floorz;
          spr2->cstat=SPRC_FLOORSPRITE|SPRC_INVISIBLE;
          spr2->picnum=PATHMARKERPIC;
          spr2->xrepeat=64;
          spr2->yrepeat=64;
          spr2->shade=spr->shade;
          spr2->pal=spr->pal;
          spr2->clipdist=spr->clipdist;
          spr2->filler=0;
          spr2->ang=spr->ang;
          spr2->owner=spr->owner;
          setsprite(j,spr2->x,spr2->y,spr2->z);
          GAM_onGround(j,1);
     }
}

void
PLR_movePlayer(short p)
{
     short ang,
          clipdist,clipheight,
          horizCheckSect,
          playerHeight,
          resetPlayerFlag,
          s,sect,
          zAdj;
     long horizCheckX,horizCheckY,horizCheckZ,x,xvect,y,yvect,z;
     struct moveFIFO *movePtr;
     spritetype *spr;
     sectortype *sectPtr;

     s=PLR_getPlayerSprite(p);
     spr=spritePtr[s];
     clipdist=ACT_getActorClipDist(spr);
     clipheight=ACT_getActorClipHeight(spr);
     playerHeight=ACT_getActorViewHeight(spr);
     zAdj=playerHeight;
     ang=spr->ang;
     x=spr->x;
     y=spr->y;
     z=spr->z-playerHeight;
     sect=spr->sectnum;
     movePtr=&moveFIFOBuf[p][moveFIFObeg];
     currentWeapon[s]=movePtr->weapon;
     if (forceVel[s] != 0 || movePtr->fvel != 0L || movePtr->svel != 0L) {
          xvect=0L;
          yvect=0L;
          if (forceVel[s] != 0) {
               xvect=mulscale2((long)forceVel[s],
                               (long)sintable[(forceAng[s]+512)&2047]);
               yvect=mulscale2((long)forceVel[s],
                               (long)sintable[forceAng[s]]);
               forceVel[s]=kmax(forceVel[s]-(TICWAITPERFRAME<<2),0);
          }
          if (movePtr->fvel != 0L) {
               xvect+=mulscale3(movePtr->fvel,(long)sintable[(ang+512)&2047]);
               yvect+=mulscale3(movePtr->fvel,(long)sintable[ang]);
          }
          if (movePtr->svel != 0L) {
               xvect+=mulscale3(movePtr->svel,(long)sintable[ang]);
               yvect+=mulscale3(movePtr->svel,(long)sintable[(ang+1536)&2047]);
          }
          clipmove(&x,&y,&z,&sect,xvect,yvect,clipdist,clipheight,clipheight,0);
     }
     if (movePtr->avel != 0) {
          ang+=((movePtr->avel*TICWAITPERFRAME)>>4);
          ang&=2047;
     }
     spr->cstat&=~SPRC_BLOCKING;
     if (pushmove(&x,&y,&z,&sect,clipdist,clipheight,clipheight,0) < 0) {
          if (!ACT_isDead(s)) {
               ACT_killPlayer(s);
          }
     }
     getzrange(x,y,z,sect,&globhiz,&globhihit,&globloz,&globlohit,clipdist,0);
     sectPtr=sectorPtr[sect];
     if ((sectPtr->floorstat&0x02) != 0 || (sectPtr->ceilingstat&0x02) != 0) {
          getzsofslope(sect,x,y,&globhiz,&globloz);
     }
     horizSlopeGoal[s]=0;
     if (horizCheckFlag && GAM_isOnGround(s)) {
          if ((sectPtr->floorstat&0x02) != 0) {   // on a slope
               horizCheckX=x+((long)sintable[(ang+512)&2047]>>4);
               horizCheckY=y+((long)sintable[ang]>>4);
               updatesector(horizCheckX,horizCheckY,&horizCheckSect);
               if (horizCheckSect != -1) {
                    horizCheckZ=getflorzofslope(horizCheckSect,
                                                horizCheckX,
                                                horizCheckY);
                    if (horizCheckZ < globloz) {
                         horizSlopeGoal[s]=kmin((globloz-horizCheckZ)>>7,200);
                    }
                    else {
                         horizSlopeGoal[s]=kmax((globloz-horizCheckZ)>>7,-200);
                    }
                    if (horizCheckSprite != -1) {
                         setsprite(horizCheckSprite,horizCheckX,
                                   horizCheckY,horizCheckZ);
                         spritePtr[horizCheckSprite]->ang=ang;
                    }
               }
          }
     }
     if (horizSlopeAdj[s] < horizSlopeGoal[s]) {
          horizSlopeAdj[s]=kmin(horizSlopeAdj[s]+(TICWAITPERFRAME<<1),
                                horizSlopeGoal[s]);
     }
     else if (horizSlopeAdj[s] > horizSlopeGoal[s]) {
          horizSlopeAdj[s]=kmax(horizSlopeAdj[s]-(TICWAITPERFRAME<<1),
                                horizSlopeGoal[s]);
     }
     if (ACT_isDead(s)) {
          zAdj=0;
          spr->zvel=0;
          PLR_dieAnim(p);
          goto playerdeadlabel;
     }
     spr->cstat|=SPRC_BLOCKING;
     PLR_footStep(p,spr);
     ACT_inSector(s,sect);
     if (sect != spr->sectnum) {
          ACT_newSector(s,&spr->sectnum,&sect,&ang,&x,&y,&z);
     }
     if ((globlohit&0x4000) == 0x4000) {     // on a sector
          if ((globlohit&(MAXSECTORS-1)) != sect) {
               if (EFF_isBonusSector(globlohit&(MAXSECTORS-1))) {
                    ACT_getBonusSectorItem(s,globlohit&(MAXSECTORS-1));
               }
          }
     }
     if ((globlohit&0xC000) == 0xC000) {     // on a sprite
          EFF_onSprite(s,globlohit&(MAXSPRITES-1),&ang,&sect,&x,&y,&z);
     }
     if ((movePtr->moves&MOVES_LOOKUP) != 0) {
          spriteHoriz[s]=kmax(spriteHoriz[s]-(TICWAITPERFRAME<<1),0);
     }
     if ((movePtr->moves&MOVES_LOOKDN) != 0) {
          spriteHoriz[s]=kmin(spriteHoriz[s]+(TICWAITPERFRAME<<1),200);
     }
     if ((movePtr->moves&MOVES_AUTOCENTER) != 0) {
          plr->autocenter=1;
     }
     if (plr->autocenter != 0) {
          if (spriteHoriz[s] < 100) {
               spriteHoriz[s]=kmin(spriteHoriz[s]+(TICWAITPERFRAME<<1),100);
          }
          else if (spriteHoriz[s] > 100) {
               spriteHoriz[s]=kmax(spriteHoriz[s]-(TICWAITPERFRAME<<1),100);
          }
          else {
               plr->autocenter=0;
          }
     }
     if ((movePtr->moves&MOVES_SHOOT) != 0) {
          WEP_weaponActivated(s);
     }
     else {
          firingWeapon[s]=0;
     }
     if (GAM_isOnGround(s)) {
          if ((movePtr->moves&MOVES_JUMP) != 0) {
               spr->zvel-=jumpVelocity;
          }
          else if ((movePtr->moves&MOVES_CROUCH) != 0) {
#if 0
               zAdj>>=1;
#endif
               if (GAM_isOnWater(s)) {
                    spr->zvel=(jumpVelocity>>1);
               }
          }
     }
     else if (GAM_isUnderWater(s)) {
          if ((movePtr->moves&MOVES_JUMP) != 0) {
               spr->zvel=-(jumpVelocity>>1);
          }
          else if ((movePtr->moves&MOVES_CROUCH) != 0) {
               spr->zvel=(jumpVelocity>>1);
          }
     }
playerdeadlabel:
     resetPlayerFlag=0;
     if ((movePtr->moves&MOVES_USE) != 0 && (omoves[p]&MOVES_USE) == 0) {
          if (ACT_isDead(s)) {
               switch (gameType) {
               case GAMETYPE_SINGLE:
                    playingGameFlag=0;
                    loadLevelFlag=1;
                    break;
               default:
                    resetPlayerFlag=1;
                    break;
               }
          }
          else {
               PLR_useCheck(s,x,y,z,sect,ang);
          }
     }
//** do game physics
     GAM_doPhysics(s,spr,&x,&y,&z,&sect,zAdj,playerHeight,clipheight);
//**
     setsprite(s,x,y,z+playerHeight);
     spr->ang=ang;
     if (ACT_isDead(s)) {
          if (resetPlayerFlag) {
               GAM_setupVideo(viewSize);
               PLR_initPlayer(p);
               PLR_initPlayerSprite(p);
          }
     }
     else {
          if (sect < 0 || sect >= numsectors) {
               ACT_killPlayer(s);
          }
     }
     omoves[p]=movePtr->moves;
}

void
PLR_doActors(void)
{
     int  p;

     for (p=connecthead ; p >= 0 ; p=connectpoint2[p]) {
          PLR_movePlayer(p);
          WEP_spriteFireWeapon(PLR_getPlayerSprite(p));
     }
}

int
PLR_oneTimeKey(int keynum)
{
     if (tkeys[keynum] != 0 && okeys[keynum] == 0) {
          return(1);
     }
     return(0);
}

void
PLR_getMouse(short *x,short *y,short *b)
{
     if (mouseEnabled) {
          getmousevalues(x,y,b);
     }
     else {
          *x=0;
          *y=0;
          *b=0;
     }
}

void
PLR_getInput(void)
{
     int  i,j,moving,running,strafing,turning;
     short mb=0,mx=0,my=0,snum;
     long mavel,mfvel,msvel,maxvel;
     spritetype *spr;
     struct player *plr;

     plr=player[myconnectindex];
//** copy current key states
     KBD_copyKeys();
//** do menu input if on main menu while playing game
     if (inMenuFlag) {
          MEN_getInput();
          goto menuskip;
     }
     snum=plr->spriteNum;
     spr=spritePtr[snum];
//** hard coded keys (local console)
     if (tkeys[K_ESC] != 0) {
          if (playingGameFlag) {
               if (qsetmode == 200L) {
                    doMainMenuFlag=1;
               }
          }
     }
     if (PLR_oneTimeKey(K_F1)) {        // HELP screen
          GAM_helpScreen();
     }
     else if (PLR_oneTimeKey(K_F2)) {   // SAVE game
          noEnemiesFlag=1;
          MEN_saveGameOption();
          noEnemiesFlag=0;
          GAM_setupVideo(viewSize);
     }
     else if (PLR_oneTimeKey(K_F3)) {   // LOAD game
          noEnemiesFlag=1;
          if (MEN_loadGameOption()) {
               loadLevelFlag=1;
          }
          noEnemiesFlag=0;
          GAM_setupVideo(viewSize);
     }
     else if (PLR_oneTimeKey(K_F4)) {   // sound options (volume)
          noEnemiesFlag=1;
          MEN_soundOptions();
          noEnemiesFlag=0;
          GAM_setupVideo(viewSize);
     }
     else if (PLR_oneTimeKey(K_F5)) {   // graphics options (detail)
          noEnemiesFlag=1;
          MEN_graphicOptions();
          noEnemiesFlag=0;
          GAM_setupVideo(viewSize);
     }
     else if (PLR_oneTimeKey(K_F6)) {   // QUICKSAVE game
          noEnemiesFlag=1;
          MEN_allocFrameTile();
          if (MEN_getSaveName(menuBackgroundPic,"QUICK SAVE")) {
               MEN_saveGameData();
               MEN_saveScreenShot();
          }
          MEN_freeFrameTile();
          noEnemiesFlag=0;
          GAM_setupVideo(viewSize);
     }
     else if (PLR_oneTimeKey(K_F7)) {   // END game
          noEnemiesFlag=1;
          if (MEN_areYouSure()) {
               playingGameFlag=0;
               inMenuFlag=0;
          }
          noEnemiesFlag=0;
          GAM_setupVideo(viewSize);
     }
     else if (PLR_oneTimeKey(K_F8)) {   // repeat messages
     }
     else if (PLR_oneTimeKey(K_F9)) {   // QUICKLOAD game
          noEnemiesFlag=1;
          if (MEN_getLoadName()) {
               MEN_loadGameData();
          }
          noEnemiesFlag=0;
          GAM_setupVideo(viewSize);
     }
     if (tkeys[K_LEFTALT] != 0) {       // special ALT-key combinations
          if (PLR_oneTimeKey(K_D)) {
               debugModeFlag^=1;
          }
          else if (PLR_oneTimeKey(K_F)) {
               showFrameRateFlag^=1;
          }
          else if (PLR_oneTimeKey(K_G)) {
               plr->gridSize=(plr->gridSize+1)%6;
          }
          else if (PLR_oneTimeKey(K_P)) {
               GFX_redShift(256);
               GFX_greenShift(512);
               GFX_blueShift(768);
               EFF_displayMessage("PALETTE SHIFT TEST");
          }
          else if (PLR_oneTimeKey(K_S)) {
               sprintf(tempbuf,"SAVE%04d.MAP",savemap++);
               saveboard(tempbuf,&spr->x,&spr->y,&spr->z,&spr->ang,
                         &spr->sectnum);
          }
          else if (PLR_oneTimeKey(K_0)) {
               if (!dumpDebugInfoFlag) {
                    if (dbgfp != NULL) {
                         fclose(dbgfp);
                         dbgfp=NULL;
                         EFF_displayMessage("DEBUG FILE CLOSED");
                    }
                    else {
                         sprintf(tempbuf,"DEBUG.%03d",myconnectindex);
                         dbgfp=fopen(tempbuf,"w");
                         EFF_displayMessage("FILE %s OPENED",tempbuf);
                    }
               }
          }
          else if (PLR_oneTimeKey(K_1)) {        // add player
               if (numplayers < MAXPLAYERS) {
                    j=connecthead;
                    i=connectpoint2[j];
                    while (i != -1) {
                         j=i;
                         i=connectpoint2[j];
                    }
                    i=connectpoint2[j]=numplayers++;
                    PLR_initPlayer(i);
                    PLR_initPlayerSprite(i);
               }
          }
          else if (PLR_oneTimeKey(K_2)) {        // switch view
               currentView=connectpoint2[currentView];
               if (currentView == -1) {
                    currentView=connecthead;
               }
               i=currentWeapon[PLR_getPlayerSprite(currentView)];
               PLR_setViewWeapon(i);
          }
          else if (PLR_oneTimeKey(K_3)) {        // switch control
               myconnectindex=connectpoint2[myconnectindex];
               if (myconnectindex == -1) {
                    myconnectindex=connecthead;
               }
          }
          else if (PLR_oneTimeKey(K_4)) {        // missile camera
               if (WEP_getMissileView() >= 0) {
                    WEP_setMissileView(-1);
               }
               else {
                    WEP_setMissileView(WEP_findMissileView(snum));
                    if (WEP_getMissileView() >= 0) {
                         EFF_displayMessage("MISSILE VIEW");
                    }
                    else {
                         EFF_displayMessage("MISSILE VIEW FAILED");
                    }
               }
          }
          else if (PLR_oneTimeKey(K_5)) {
               noEnemiesFlag^=1;
               EFF_displayMessage("ENEMIES %s",noEnemiesFlag ? "DISABLED" :
                                                               "ENABLED");
          }
          else if (PLR_oneTimeKey(K_6)) {
               if (horizCheckSprite == -1) {
                    horizCheckSprite=ENG_insertsprite(spr->sectnum,STAT_NONE);
                    spritePtr[horizCheckSprite]->x=spr->x;
                    spritePtr[horizCheckSprite]->y=spr->y;
                    spritePtr[horizCheckSprite]->z=spr->z;
                    spritePtr[horizCheckSprite]->cstat=spr->cstat;
                    spritePtr[horizCheckSprite]->picnum=spr->picnum;
                    spritePtr[horizCheckSprite]->shade=spr->shade;
                    spritePtr[horizCheckSprite]->pal=spr->pal;
                    spritePtr[horizCheckSprite]->clipdist=spr->clipdist;
                    spritePtr[horizCheckSprite]->xrepeat=16;
                    spritePtr[horizCheckSprite]->yrepeat=16;
                    spritePtr[horizCheckSprite]->ang=spr->ang;
                    EFF_displayMessage("HORIZ DEBUG ON");
               }
               else {
                    ENG_deletesprite(horizCheckSprite);
                    horizCheckSprite=-1;
                    EFF_displayMessage("HORIZ DEBUG OFF");
               }
          }
          else if (PLR_oneTimeKey(K_9)) {         // single step mode
               singleStepFlag^=1;
               EFF_displayMessage("SINGLE STEP MODE %s",singleStepFlag ?
                                  "ENABLED" : "DISABLED");
          }
     }
     if (tkeys[K_F10] != 0) {      // F10 = quick exit
          inGameFlag=0;
          playingGameFlag=0;
          inMenuFlag=0;
     }
menuskip:
     if (PLR_oneTimeKey(K_F11)) {
          if (tkeys[K_LEFTALT] != 0) {
               visibility-=64;
               if (visibility < 0) {
                    visibility=512;
               }
               EFF_displayMessage("VISIBILITY: %d",visibility);
          }
          else {
               gammaCorrection++;
               if (gammaCorrection > 8) {
                    gammaCorrection=0;
               }
               setbrightness(gammaCorrection,(char *)&palette[0]);
               EFF_displayMessage("GAMMA CORRECTION: %d",gammaCorrection);
          }
     }
     if (PLR_oneTimeKey(K_F12)) {
          screencapture("captxxxx.pcx",0);
     }
//** programmable keys
     if (playingGameFlag) {
          mavel=0;                 // mouse velocities
          mfvel=0;
          msvel=0;
          moving=0;
          running=0;
          strafing=0;
          turning=0;
          maxvel=VELSPEED;
          plr->moves=0L;
          if (inMenuFlag) {
               goto menuskip2;
          }
//** read controller buttons
          if (!ACT_isDead(snum)) {
               if (mouseEnabled) {
                    PLR_getMouse(&mx,&my,&mb);
                    if ((mb&1) != 0) {  // left mouse button
                         tkeys[keys[configMouse[MOUSE_LEFTBUTTON]]]=1;
                    }
                    if ((mb&2) != 0) {  // right mouse button
                         tkeys[keys[configMouse[MOUSE_RIGHTBUTTON]]]=1;
                    }
                    if ((mb&4) != 0) {  // middle mouse button
                         tkeys[keys[configMouse[MOUSE_MIDDLEBUTTON]]]=1;
                    }
               }
               if (tkeys[keys[KEYRUN]] != 0) {
                    maxvel<<=1;
                    running=1;
               }
               if (tkeys[keys[KEYSTRAFE]] != 0) {
                    strafing=1;
               }
//** apply force to motion & rotation
               if (tkeys[keys[KEYFWD]] != 0) {
                    if (fwdvel < 0L) {
                         fwdvel=0L;
                    }
                    fwdvel=min(fwdvel+(TICWAITPERFRAME<<6),maxvel-1);
                    moving=1;
               }
               else if (tkeys[keys[KEYBACK]] != 0) {
                    if (fwdvel > 0L) {
                         fwdvel=0L;
                    }
                    fwdvel=max(fwdvel-(TICWAITPERFRAME<<6),-maxvel);
                    moving=1;
               }
               if (tkeys[keys[KEYLEFT]] != 0) {
                    if (strafing) {
                         if (stfvel < 0L) {
                              stfvel=0L;
                         }
                         stfvel=min(stfvel+(TICWAITPERFRAME<<stfspeed),
                                    maxvel-1);
                         strafing=2;
                    }
                    else {
                         if (trnvel > 0L) {
                              trnvel=0L;
                         }
                         trnvel=max(trnvel-(TICWAITPERFRAME<<4),-TURNSPEED);
                         turning=1;
                    }
               }
               else if (tkeys[keys[KEYRIGHT]] != 0) {
                    if (strafing) {
                         if (stfvel > 0L) {
                              stfvel=0L;
                         }
                         stfvel=max(stfvel-(TICWAITPERFRAME<<stfspeed),
                                    -maxvel);
                         strafing=2;
                    }
                    else {
                         if (trnvel < 0L) {
                              trnvel=0L;
                         }
                         trnvel=min(trnvel+(TICWAITPERFRAME<<4),TURNSPEED-1);
                         turning=1;
                    }
               }
               if (tkeys[keys[KEYSTFL]] != 0) {
                    if (stfvel < 0L) {
                         stfvel=0L;
                    }
                    stfvel=min(stfvel+(TICWAITPERFRAME<<stfspeed),
                               maxvel-1);
                    strafing=2;
               }
               else if (tkeys[keys[KEYSTFR]] != 0) {
                    if (stfvel > 0L) {
                         stfvel=0L;
                    }
                    stfvel=max(stfvel-(TICWAITPERFRAME<<stfspeed),
                               -maxvel);
                    strafing=2;
               }
          }
//** look up and look down code
          if (tkeys[keys[KEYLKUP]] != 0) {
               plr->autocenter=0;
               plr->moves|=MOVES_LOOKUP;
          }
          if (tkeys[keys[KEYLKDN]] != 0) {
               plr->autocenter=0;
               plr->moves|=MOVES_LOOKDN;
          }
          if (tkeys[keys[KEYCNTR]] != 0) {
               plr->moves|=MOVES_AUTOCENTER;
          }
//** fire weapon code
          if (!ACT_isDead(snum) && !inMenuFlag) {
               for (i=0 ; i < MAXWEAPONS ; i++) {
                    if (tkeys[K_1+i] != 0) {
                         if (i != currentWeapon[snum]) {
                              WEP_changeWeapon(i);
                         }
                         break;
                    }
               }
               if (tkeys[keys[KEYFIRE]] != 0 && WEP_changingWeapon() == 0) {
                    plr->moves|=MOVES_SHOOT;
               }
//** jump & crouch code
               if (GAM_isUnderWater(snum)) {
                    if (PLR_oneTimeKey(keys[KEYJUMP])) {
                         plr->moves|=MOVES_JUMP;
                    }
                    else if (PLR_oneTimeKey(keys[KEYCROUCH])) {
                         plr->moves|=MOVES_CROUCH;
                    }
               }
               else {
                    if (PLR_oneTimeKey(keys[KEYJUMP])) {
                         if (GAM_isOnGround(snum)) {
                              plr->moves|=MOVES_JUMP;
                         }
                    }
                    if (tkeys[keys[KEYCROUCH]] != 0) {
                         plr->moves|=MOVES_CROUCH;
                    }
               }
          }
//** switch to map mode & zoom in/zoom out code
          if (PLR_oneTimeKey(keys[KEYMAP])) {
               plr->viewMode++;
               if (plr->viewMode > 3) {
                    plr->viewMode=0;
               }
          }
          if (plr->viewMode != 3) {
               if (tkeys[keys[KEYZOOMO]] != 0) {
                    if (plr->zoom > 48L) {
                         plr->zoom-=(plr->zoom>>4);
                    }
               }
               else if (tkeys[keys[KEYZOOMI]] != 0) {
                    if (plr->zoom < 4096L) {
                         plr->zoom+=(plr->zoom>>4);
                    }
               }
          }
          else if (plr->viewMode == 3) {
               if (PLR_oneTimeKey(keys[KEYZOOMO])) {
                    if (plr->viewSize > xdim) {
                         plr->viewSize=xdim;
                    }
                    else {
                         plr->viewSize-=8;
                    }
                    if (plr->viewSize < 64) {
                         plr->viewSize=64;
                    }
                    localViewSize=plr->viewSize;
               }
               else if (PLR_oneTimeKey(keys[KEYZOOMI])) {
                    plr->viewSize+=8;
                    if (plr->viewSize > xdim) {
                         plr->viewSize=xdim+1;
                    }
                    localViewSize=plr->viewSize;
               }
          }
//** operate sectors/sprites code
          if (PLR_oneTimeKey(keys[KEYUSE])) {
               plr->moves|=MOVES_USE;
          }
//** apply mouse velocities
          if (mouseEnabled) {
               if (my != 0) {
                    mfvel=min(max(-(my*mouseYSens),-MAXMOUSEYVEL),
                             MAXMOUSEYVEL-1);
               }
               if (mx != 0) {
                    if (strafing == 1) {
                         msvel=min(max(-(mx*mouseXSens),-MAXMOUSEXVEL),
                                  MAXMOUSEXVEL-1);
                    }
                    else {
                         mavel=min(max((mx*mouseXSens)>>2,-MAXMOUSEXVEL),
                                  MAXMOUSEXVEL-1);
                    }
               }
          }
//** apply physics to motion & rotation
menuskip2:
          if (!moving) {
               if (fwdvel < -VELSPEED) {
                    fwdvel=-VELSPEED;
               }
               else if (fwdvel > VELSPEED) {
                    fwdvel=VELSPEED;
               }
               if (fwdvel < 0) {
                    fwdvel=min(fwdvel+(TICWAITPERFRAME<<5),0);
               }
               else if (fwdvel > 0) {
                    fwdvel=max(fwdvel-(TICWAITPERFRAME<<5),0);
               }
          }
          if (strafing != 2) {
               if (stfvel < 0) {
                    stfvel=min(stfvel+(TICWAITPERFRAME<<6),0);
               }
               else if (stfvel > 0) {
                    stfvel=max(stfvel-(TICWAITPERFRAME<<6),0);
               }
          }
          if (!turning) {
               if (trnvel < 0) {
                    trnvel=min(trnvel+(TICWAITPERFRAME<<4),0);
               }
               else if (trnvel > 0) {
                    trnvel=max(trnvel-(TICWAITPERFRAME<<4),0);
               }
          }
          locavel=trnvel+mavel;
          locfvel=fwdvel+mfvel;
          locsvel=stfvel+msvel;
     }
     memmove(okeys,tkeys,sizeof(okeys));
}

void
PLR_doStatusCode(void)
{
}

void
PLR_checkStatus(void)
{
     long x;

     if (ACT_isDead(GAM_getViewSprite())) {
          if (qsetmode == 200L) {
               x=(xdim>>1)-(strlen(DIEMESSAGE)<<2);
               printext256(x,8L,P256COLOR,-1,DIEMESSAGE,0);
          }
     }
}

void
PLR_saveGame(FILE *fp)
{
     short i,s;

     GAM_fwrite(&currentView,sizeof(int),1,fp);
     GAM_fwrite(plr,sizeof(struct player),MAXPLAYERS,fp);
     for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
          s=PLR_getPlayerSprite(i);
          GAM_fwrite(&ammo[s],sizeof(short),MAXWEAPONTYPES,fp);
          GAM_fwrite(&attackers[s],sizeof(char),1,fp);
          GAM_fwrite(&fireFrame[s],sizeof(char),1,fp);
          GAM_fwrite(&firingWeapon[s],sizeof(char),1,fp);
          GAM_fwrite(&spriteHoriz[s],sizeof(short),1,fp);
          GAM_fwrite(&forceAng[s],sizeof(short),1,fp);
          GAM_fwrite(&forceVel[s],sizeof(short),1,fp);
     }
     GAM_fwrite(&playerHealth,sizeof(int),1,fp);
     GAM_fwrite(&playerMaxHealth,sizeof(int),1,fp);
     GAM_fwrite(&trnvel,sizeof(long),1,fp);
     GAM_fwrite(&locavel,sizeof(long),1,fp);
     GAM_fwrite(&fwdvel,sizeof(long),1,fp);
     GAM_fwrite(&locfvel,sizeof(long),1,fp);
     GAM_fwrite(&stfvel,sizeof(long),1,fp);
     GAM_fwrite(&locsvel,sizeof(long),1,fp);
}

void
PLR_loadGame(FILE *fp)
{
     short i,s;

     GAM_fread(&currentView,sizeof(int),1,fp);
     GAM_fread(plr,sizeof(struct player),MAXPLAYERS,fp);
     for (i=connecthead ; i >= 0 ; i=connectpoint2[i]) {
          s=PLR_getPlayerSprite(i);
          GAM_fread(&ammo[s],sizeof(short),MAXWEAPONTYPES,fp);
          GAM_fread(&attackers[s],sizeof(char),1,fp);
          GAM_fread(&fireFrame[s],sizeof(char),1,fp);
          GAM_fread(&firingWeapon[s],sizeof(char),1,fp);
          GAM_fread(&spriteHoriz[s],sizeof(short),1,fp);
          GAM_fread(&forceAng[s],sizeof(short),1,fp);
          GAM_fread(&forceVel[s],sizeof(short),1,fp);
     }
     GAM_fread(&playerHealth,sizeof(int),1,fp);
     GAM_fread(&playerMaxHealth,sizeof(int),1,fp);
     GAM_fread(&trnvel,sizeof(long),1,fp);
     GAM_fread(&locavel,sizeof(long),1,fp);
     GAM_fread(&fwdvel,sizeof(long),1,fp);
     GAM_fread(&locfvel,sizeof(long),1,fp);
     GAM_fread(&stfvel,sizeof(long),1,fp);
     GAM_fread(&locsvel,sizeof(long),1,fp);
}

//** debug code follows

void
PLR_debug(void)
{
     int  p,s;
     spritetype *spr;

     for (p=connecthead ; p >= 0 ; p=connectpoint2[p]) {
          s=PLR_getPlayerSprite(p);
          spr=spritePtr[s];
          debugOut(windowx1,windowy1,"P=%d(%d) X=%06d Y=%06d Z=%06d S=%06d "
                   "AN=%04d H=%d AM=%d G=%d W=%d HOR=%d",p,s,
                   spr->x,spr->y,spr->z,spr->sectnum,spr->ang,
                   ACT_getHealth(s),WEP_getAmmo(s,WEP_getSpriteAmmoType(s)),
                   GAM_isOnGround(s),GAM_isUnderWater(s),
                   spriteHoriz[s]);
     }
     debugOut(windowx1,windowy1,"LOZ=%06ld HIZ=%06ld LOHIT=%06ld HIHIT=%06ld",
             globloz,globhiz,globlohit,globhihit);
}

