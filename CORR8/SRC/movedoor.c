void
EFF_moveDoor(short d)
{
     int  i,
          j,
          rotang,
          s;
     long beforez,goalz,
          deltaz,
          newx,newy;
     struct doorData *dptr;
     sectortype *sect;

     dptr=doorPtr[d];
     if (dptr->state == DOORSTATE_IDLE) {
          return;
     }
     s=dptr->sectorIndex;
     sect=sectorPtr[s];
     switch (sect->lotag) {
     case SEC_DOORUPTAG:
     case SEC_DOORUPONCETAG:
          switch (dptr->state) {
          case DOORSTATE_OPEN:
               if (EFF_moveSectorCeiling(s,sect,-(dptr->speed*TICWAITPERFRAME),
                                         dptr->hiz)) {
                    dptr->state=DOORSTATE_OPENED;
               }
               break;
          case DOORSTATE_CLOSE:
               i=headspritesect[s];
               while (i != -1) {   // check for living objects in door sector
                    j=nextspritesect[i];
                    if (ACT_getHealth(i) > 0) {
                         dptr->state=DOORSTATE_OPEN;
                         break;
                    }
                    i=j;
               }
               if (EFF_moveSectorCeiling(s,sect,(dptr->speed*TICWAITPERFRAME),
                                         dptr->loz)) {
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
               if (EFF_moveSectorFloor(s,sect,dptr->speed*TICWAITPERFRAME,
                                        dptr->loz)) {
                    dptr->state=DOORSTATE_OPENED;
               }
               if (EFF_isElevator(s)) {
                    deltaz=sect->floorz-beforez;
                    for (i=headspritesect[s] ; i >= 0 ; i=j) {
                         j=nextspritesect[i];
                         if (GAM_isOnGround(i)) {
                              spritePtr[i]->z+=deltaz;
                         }
                    }
                    if (sect->lotag == SEC_PLATFORMELEVATORTAG) {
                         sect->ceilingz=sect->floorz-dptr->height;
                    }
               }
               break;
          case DOORSTATE_CLOSE:
               if (EFF_isDoor(s)) {
                    i=headspritesect[s];
                    while (i != -1) {   // check for living objects in sector
                         j=nextspritesect[i];
                         if (ACT_getHealth(i) > 0) {
                              dptr->state=DOORSTATE_OPEN;
                              break;
                         }
                         i=j;
                    }
               }
               beforez=sect->floorz;
               if (EFF_moveSectorFloor(s,sect,-(dptr->speed*TICWAITPERFRAME),
                                        dptr->hiz)) {
                    dptr->state=DOORSTATE_CLOSED;
               }
               if (EFF_isElevator(s)) {
                    deltaz=sect->floorz-beforez;
                    for (i=headspritesect[s] ; i >= 0 ; i=j) {
                         j=nextspritesect[i];
                         if (GAM_isOnGround(i)) {
                              spritePtr[i]->z+=deltaz;
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
               if (EFF_moveSectorFloor(s,sect,-(dptr->speed*TICWAITPERFRAME),
                                        dptr->hiz)) {
                    dptr->state=DOORSTATE_OPENED;
               }
               deltaz=sect->floorz-beforez;
               for (i=headspritesect[s] ; i >= 0 ; i=j) {
                    j=nextspritesect[i];
                    if (GAM_isOnGround(i)) {
                         spritePtr[i]->z+=deltaz;
                    }
               }
               break;
          case DOORSTATE_CLOSE:
               beforez=sect->floorz;
               if (EFF_moveSectorFloor(s,sect,dptr->speed*TICWAITPERFRAME,
                                        dptr->loz)) {
                    dptr->state=DOORSTATE_CLOSED;
               }
               deltaz=sect->floorz-beforez;
               for (i=headspritesect[s] ; i >= 0 ; i=j) {
                    j=nextspritesect[i];
                    if (GAM_isOnGround(i)) {
                         spritePtr[i]->z+=deltaz;
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
               if (EFF_moveSectorCeiling(s,sect,-(dptr->speed*TICWAITPERFRAME),
                                          dptr->hiz)) {
                    i=1;
               }
               if (EFF_moveSectorFloor(s,sect,dptr->speed*TICWAITPERFRAME,
                                        dptr->loz)) {
                    i|=2;
               }
               if (i == 3) {
                    dptr->state=DOORSTATE_OPENED;
               }
               break;
          case DOORSTATE_CLOSE:
               i=headspritesect[s];
               while (i != -1) {   // check for living objects in door sector
                    j=nextspritesect[i];
                    if (ACT_getHealth(i) > 0) {
                         dptr->state=DOORSTATE_OPEN;
                         break;
                    }
                    i=j;
               }
               goalz=sectorCenterPtr[s]->centerz;
               i=0;
               if (EFF_moveSectorCeiling(s,sect,dptr->speed*TICWAITPERFRAME,
                                          goalz)) {
                    i=1;
               }
               if (EFF_moveSectorFloor(s,sect,-(dptr->speed*TICWAITPERFRAME),
                                        goalz)) {
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
          break;
     case DOORSTATE_OPENED:
          switch (sect->lotag) {
          case SEC_DOORUPONCETAG:
          case SEC_DOORDOWNONCETAG:
          case SEC_DOORHSPLITONCETAG:
               dptr->state=DOORSTATE_IDLE;
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
          EFF_blockDoorWalls(s);
          SND_playSectorMovingSound(s,0);
          SND_playSectorStopSound(s);
          break;
     case DOORSTATE_WAITING:
          if (dptr->movingSectorFlag == 0) {
               dptr->delay-=TICWAITPERFRAME;
               if (dptr->delay <= 0) {
                    dptr->state=DOORSTATE_CLOSE;
                    EFF_turnSwitchesOff(sect->hitag);
                    SND_playSectorMovingSound(s,0);
                    SND_playSectorCloseSound(s);
               }
          }
          break;
     }
}

