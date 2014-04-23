
#if 0
void
PLR_movePlayer(short p)
{
     static int  resetPlayerFlag;
     static short ang,sect,snum;
     static short horizCheckSect;
     static long avel,fvel,svel,moves,goalz,x,xvect,y,yvect,z,zAdj;
     static long horizCheckX,horizCheckY,horizCheckZ;
     static struct moveFIFO *movePtr;
     static struct player *plr;
     static spritetype *spr;

     movePtr=&moveFIFOBuf[p][moveFIFObeg];
     plr=player[p];
     snum=plr->spriteNum;
     spr=spritePtr[snum];
     ang=spr->ang;
     sect=spr->sectnum;
     x=spr->x;
     y=spr->y;
     z=spr->z-playerHeight;
     avel=movePtr->avel;
     fvel=movePtr->fvel;
     svel=movePtr->svel;
     moves=movePtr->moves;
     currentWeapon[snum]=movePtr->weapon;
     if (fvel != 0 || svel != 0) {
          xvect=0L;
          yvect=0L;
          if (fvel != 0) {
               xvect+=((fvel*(long)sintable[(ang+512)&2047])>>3);
               yvect+=((fvel*(long)sintable[ang])>>3);
          }
          if (svel != 0) {
               xvect+=((svel*(long)sintable[ang])>>3);
               yvect+=((svel*(long)sintable[(ang+1536)&2047])>>3);
          }
          clipmove(&x,&y,&z,&sect,xvect,yvect,playerClipSize,4<<8,4<<8,0);
     }
     if (avel != 0) {
          ang+=((avel*TICWAITPERFRAME)>>4);
          ang&=2047;
     }
     if (!ACT_isDead(snum)) {
          spr->cstat &= ~(SPRC_BLOCKING);
     }
     if (pushmove(&x,&y,&z,&sect,playerClipSize,4<<8,4<<8,0) < 0) {
          ACT_killPlayer(snum);
     }
     getzrange(x,y,z,sect,&globhiz,&globhihit,&globloz,&globlohit,
               playerClipSize,0);
     if (!ACT_isDead(snum)) {
          spr->cstat |= (SPRC_BLOCKING);
     }
     horizSlopeGoal[snum]=0;
     if (horizCheckFlag && GAM_isOnGround(snum)) {
          horizCheckX=x+((long)sintable[(ang+512)&2047]>>4);
          horizCheckY=y+((long)sintable[ang]>>4);
          updatesector(horizCheckX,horizCheckY,&horizCheckSect);
          if (horizCheckSect != -1) {
               if ((sectorPtr[horizCheckSect]->floorstat&2) != 0) {
                    horizCheckZ=getflorzofslope(horizCheckSect,horizCheckX,
                                                horizCheckY);
                    horizSlopeGoal[snum]=(spr->z-horizCheckZ)>>7;
               }
          }
     }
     if (horizSlopeAdj[snum] < horizSlopeGoal[snum]) {
          horizSlopeAdj[snum]+=(TICWAITPERFRAME<<1);
          if (horizSlopeAdj[snum] > horizSlopeGoal[snum]) {
               horizSlopeAdj[snum]=horizSlopeGoal[snum];
          }
     }
     else if (horizSlopeAdj[snum] > horizSlopeGoal[snum]) {
          horizSlopeAdj[snum]-=(TICWAITPERFRAME<<1);
          if (horizSlopeAdj[snum] < horizSlopeGoal[snum]) {
               horizSlopeAdj[snum]=horizSlopeGoal[snum];
          }
     }
     resetPlayerFlag=0;
     if ((moves&MOVES_USE) != 0 && (omoves[p]&MOVES_USE) == 0) {
          if (ACT_isDead(snum)) {
               resetPlayerFlag=1;
          }
          else {
               PLR_useCheck(snum,x,y,z,sect,ang);
          }
     }
     ACT_inSector(snum,sect);
     if (sect != spr->sectnum) {
          ACT_newSector(snum,&sect,&ang,&x,&y,&z);
     }
     if ((globlohit&0x4000) == 0x4000) {     // on a sector
          if ((globlohit&0x3FF) != sect) {
               if (EFF_isBonusSector(globlohit&0x3FF)) {
                    ACT_getBonusSectorItem(snum,globlohit&0x3FF);
               }
          }
     }
     if ((globlohit&0xC000) == 0xC000) {     // on a sprite
          EFF_onSprite(snum,globlohit&(MAXSPRITES-1),&ang,&sect,&x,&y,&z);
     }
     if ((moves&MOVES_LOOKUP) != 0) {
          if (spriteHoriz[snum] > 100-(200>>1)) {
               spriteHoriz[snum]-=(TICWAITPERFRAME<<1);
          }
     }
     if ((moves&MOVES_LOOKDN) != 0) {
          if (spriteHoriz[snum] < 100+(200>>1)) {
               spriteHoriz[snum]+=(TICWAITPERFRAME<<1);
          }
     }
     if ((moves&MOVES_AUTOCENTER) != 0) {
          plr->autocenter=1;
     }
     if (plr->autocenter != 0) {
          if (spriteHoriz[snum] < 100) {
               spriteHoriz[snum]+=(TICWAITPERFRAME<<1);
          }
          else if (spriteHoriz[snum] > 100) {
               spriteHoriz[snum]-=(TICWAITPERFRAME<<1);
          }
          else {
               plr->autocenter=0;
          }
     }
     if (ACT_isDead(snum)) {
          zAdj=0;
          PLR_dieAnim(snum);
     }
     else {
          if ((moves&MOVES_SHOOT) != 0) {
               WEP_weaponActivated(snum);
          }
          else {
               firingWeapon[snum]=0;
          }
          if ((moves&MOVES_JUMP) != 0) {
               horizVel[snum]-=jumpVelocity;
          }
          if ((moves&MOVES_CROUCH) != 0) {
               zAdj=playerHeight>>2;
          }
          else {
               zAdj=playerHeight;
          }
     }
     goalz=globloz-zAdj;
     if (!ACT_isDead(snum) && goalz <= globhiz+(4<<8)) {
          if (globloz-globhiz > (playerHeight>>2)) {
               goalz=globhiz+(4<<8);
          }
     }
     GAM_doPhysics(snum,goalz,&x,&y,&z);
     if (!ACT_isDead(snum)) {
          z+=playerHeight;
     }
     else {
          z=goalz;
     }
     spr->ang=ang;
     setsprite(snum,x,y,z);
     PLR_dropMarker(p);
     if (resetPlayerFlag) {
          GAM_setupVideo(plr->viewSize);
          PLR_initPlayer(p);
          PLR_initPlayerSprite(p);
     }
     omoves[p]=moves;
}
#endif
