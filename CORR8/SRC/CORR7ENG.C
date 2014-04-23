/***************************************************************************
 *   CORR7ENG.C - BUILD related functions for Corridor 7
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   AVERAGEFRAMES       64
#define   TOPSTATBARY         30
#define   UNDERWATERTILE      4093

short floorTexture,
     ceilTexture,
     wallTexture,
     spriteTexture;

int  configVideoFlag,
     debugModeFlag,
     engineInitializedFlag,
     showFrameRateFlag=1,
     weaponEditFlag;

int  viewSize;

long avgFPS,
     framecnt,
     frameval[AVERAGEFRAMES];

long globloz,
     globlohit,
     globhiz,
     globhihit;

long mapchksum;

short renderAng,
     renderSect;

short lotagSearch,
     hitagSearch,
     picnumSearch,
     searchNum;

long renderX,
     renderY,
     renderZ;

short horizSlopeAdj[MAXSPRITES];

long diez[MAXPLAYERS];

long engWindowx1,
     engWindowx2,
     engWindowy1,
     engWindowy2;

int  debugline,
     midstatbarx,
     midstatbary;

char nameBuf[17];

int  editorEnabledFlag,
     gameModeFlag=0;

#define   NUMKEYS        19

static
char defBuildKeys[NUMKEYS]={
     0xc8, 0xd0, 0xcb, 0xcd, 0x2a, 0x9d, 0x1d, 0x39,
     0x1e, 0x2c, 0xd1, 0xc9, 0x33, 0x34,
     0x9c, 0x1c, 0xd, 0xc, 0xf
};

extern
short searchsector,
     searchwall,
     searchstat;

extern
short pointhighlight;

extern
short ang,
     cursectnum;

extern
short startang,
     startsectnum;

extern
long cachecount;

extern
long posx,
     posy,
     posz;

extern
long startposx,
     startposy,
     startposz;

extern
long zmode,
     kensplayerheight;

extern
char buildkeys[NUMKEYS],
     names[MAXTILES][17];

walltype   *wallPtr[MAXWALLS];
spritetype *spritePtr[MAXSPRITES];
sectortype *sectorPtr[MAXSECTORS];

#define   MAXMEMPTRS     64

struct membuf {
     long cache_ptr;
     long cache_length;
     char cache_lock;
} memptr[MAXMEMPTRS];

void
ENG_init(void)
{
     int  i;

     for (i=0 ; i < MAXSECTORS ; i++) {
          sectorPtr[i]=&sector[i];
     }
     for (i=0 ; i < MAXSPRITES ; i++) {
          spritePtr[i]=&sprite[i];
     }
     for (i=0 ; i < MAXWALLS ; i++) {
          wallPtr[i]=&wall[i];
     }
}

void
ENG_unInit(void)
{
}

int
ENG_editorGameMode(void)
{
     if (editorEnabledFlag && gameModeFlag) {
          return(1);
     }
     return(0);
}

int
ENG_inSetupVideoMode(void)
{
     if (windowx1 != engWindowx1 || windowy1 != engWindowy1 ||
         windowx2 != engWindowx2 || windowy2 != engWindowy2) {
          return(0);
     }
     return(1);
}

void
ENG_setupVideo(void)
{
     if (qsetmode != 200) {
          setgamemode();
     }
     setview(0L,0L,xdim-1L,ydim-1L);    // need this here for editor mode
     engWindowx1=windowx1;
     engWindowy1=windowy1;
     engWindowx2=windowx2;
     engWindowy2=windowy2;
     clearview(0L);
     outp(0x3c8,255);         // make transparent pink into black to
     outp(0x3c9,0);           // ..cover up bleed thru when rendering
     outp(0x3c9,0);
     outp(0x3c9,0);
}

void
ENG_init2DMap(void)
{
     int  i;

     for (i=0 ; i < (MAXSECTORS>>3) ; i++) {
          show2dsector[i]=0;
     }
     for (i=0 ; i < (MAXWALLS>>3) ; i++) {
          show2dwall[i]=0;
     }
     for (i=0 ; i < (MAXSPRITES>>3) ; i++) {
          show2dsprite[i]=0;
     }
     automapping=1;
}

void
ENG_resetMap(void)
{
     int  i;

     i=getboolean16("Reset ceiling textures? ",0);
     if (i) {
          setgamemode();
          ceilTexture=gettile(0L);
          qsetmode640480();
          clear2dscreen(0);
          draw2dscreen(posx,posy,ang,768L,0);
          clearmidstatbar16();
          printmessage16("");
          EFF_notify(0L,"ASSIGNING CEILING TEXTURES");
          for (i=0 ; i < numsectors ; i++) {
               sectorPtr[i]->ceilingpicnum=ceilTexture;
          }
     }
     i=getboolean16("Reset floor textures? ",0);
     if (i) {
          setgamemode();
          floorTexture=gettile(0L);
          qsetmode640480();
          clear2dscreen(0);
          draw2dscreen(posx,posy,ang,768L,0);
          clearmidstatbar16();
          printmessage16("");
          EFF_notify(0L,"ASSIGNING FLOOR TEXTURES");
          for (i=0 ; i < numsectors ; i++) {
               sectorPtr[i]->floorpicnum=floorTexture;
          }
     }
     i=getboolean16("Reset sector tags? ",0);
     if (i) {
          EFF_notify(0L,"RESETTING SECTOR TAGS");
          for (i=0 ; i < numsectors ; i++) {
               sectorPtr[i]->lotag=0;
               sectorPtr[i]->hitag=0;
               sectorPtr[i]->extra=-1;
          }
     }
     i=getboolean16("Reset wall textures? ",0);
     if (i) {
          setgamemode();
          wallTexture=gettile(0L);
          qsetmode640480();
          clear2dscreen(0);
          draw2dscreen(posx,posy,ang,768L,0);
          clearmidstatbar16();
          printmessage16("");
          EFF_notify(0L,"ASSIGNING WALL TEXTURES");
          for (i=0 ; i < numwalls ; i++) {
               wallPtr[i]->picnum=wallTexture;
               wallPtr[i]->overpicnum=wallTexture;
          }
     }
     i=getboolean16("Reset wall tags? ",0);
     if (i) {
          EFF_notify(0L,"RESETTING WALL TAGS");
          for (i=0 ; i < numwalls ; i++) {
               wallPtr[i]->lotag=0;
               wallPtr[i]->hitag=0;
               wallPtr[i]->extra=-1;
          }
     }
     i=getboolean16("Reset sprite textures? ",0);
     if (i) {
          setgamemode();
          spriteTexture=gettile(0L);
          qsetmode640480();
          clear2dscreen(0);
          draw2dscreen(posx,posy,ang,768L,0);
          clearmidstatbar16();
          printmessage16("");
          EFF_notify(0L,"ASSIGNING SPRITE TEXTURES");
          for (i=0 ; i < MAXSPRITES ; i++) {
               if (spritePtr[i]->statnum < MAXSTATUS) {
                    spritePtr[i]->picnum=spriteTexture;
               }
          }
     }
     i=getboolean16("Reset sprite tags? ",0);
     if (i) {
          EFF_notify(0L,"RESETTING SPRITE TAGS");
          for (i=0 ; i < MAXSPRITES ; i++) {
               if (spritePtr[i]->statnum < MAXSTATUS) {
                    spritePtr[i]->lotag=0;
                    spritePtr[i]->hitag=0;
                    spritePtr[i]->xrepeat=64;
                    spritePtr[i]->yrepeat=64;
                    spritePtr[i]->extra=-1;
                    changespritestat(i,STAT_NONE);
               }
          }
     }
     EFF_notify(0L,"DONE.");
}

short
ENG_insertsprite(short sectnum,short statnum)
{
     short j;
     spritetype *spr;

     j=insertsprite(sectnum,statnum);
     spr=spritePtr[j];
     spr->cstat=0;
     spr->shade=sectorPtr[sectnum]->floorshade;
     spr->pal=0;
     spr->lotag=0;
     spr->hitag=0;
     spr->extra=0;
     show2dsprite[j>>3]|=(1<<(j&7));
     return(j);
}

void
ENG_deletesprite(short spritenum)
{
     if (spritePtr[spritenum]->statnum < MAXSTATUS) {
          show2dsprite[spritenum>>3]&=~(1<<(spritenum&7));
          deletesprite(spritenum);
     }
}

void
ENG_checksumReset(void)
{
     mapchksum=randomseed;
}

long
ENG_getMapChecksum(void)
{
     return(mapchksum);
}

void
ENG_checksumSprite(short s)
{
     mapchksum+=spritePtr[s]->x;
     mapchksum+=spritePtr[s]->y;
     mapchksum+=spritePtr[s]->z;
     mapchksum+=spritePtr[s]->cstat;
     mapchksum+=spritePtr[s]->picnum;
     mapchksum+=spritePtr[s]->shade;
     mapchksum+=spritePtr[s]->pal;
     mapchksum+=spritePtr[s]->clipdist;
     mapchksum+=spritePtr[s]->filler;
     mapchksum+=spritePtr[s]->xrepeat;
     mapchksum+=spritePtr[s]->yrepeat;
     mapchksum+=spritePtr[s]->xoffset;
     mapchksum+=spritePtr[s]->yoffset;
     mapchksum+=spritePtr[s]->sectnum;
     mapchksum+=spritePtr[s]->statnum;
     mapchksum+=spritePtr[s]->ang;
     mapchksum+=spritePtr[s]->owner;
     mapchksum+=spritePtr[s]->xvel;
     mapchksum+=spritePtr[s]->yvel;
     mapchksum+=spritePtr[s]->zvel;
     mapchksum+=spritePtr[s]->lotag;
     mapchksum+=spritePtr[s]->hitag;
     mapchksum+=spritePtr[s]->extra;
}

void
ENG_checksumSector(short s)
{
     short endwall,startwall,w;

     mapchksum+=sectorPtr[s]->wallptr;
     mapchksum+=sectorPtr[s]->wallnum;
     mapchksum+=sectorPtr[s]->ceilingz;
     mapchksum+=sectorPtr[s]->floorz;
     mapchksum+=sectorPtr[s]->ceilingstat;
     mapchksum+=sectorPtr[s]->floorstat;
     mapchksum+=sectorPtr[s]->ceilingpicnum;
     mapchksum+=sectorPtr[s]->ceilingheinum;
     mapchksum+=sectorPtr[s]->ceilingshade;
     mapchksum+=sectorPtr[s]->ceilingpal;
     mapchksum+=sectorPtr[s]->ceilingxpanning;
     mapchksum+=sectorPtr[s]->ceilingypanning;
     mapchksum+=sectorPtr[s]->floorpicnum;
     mapchksum+=sectorPtr[s]->floorheinum;
     mapchksum+=sectorPtr[s]->floorshade;
     mapchksum+=sectorPtr[s]->floorpal;
     mapchksum+=sectorPtr[s]->floorxpanning;
     mapchksum+=sectorPtr[s]->floorypanning;
     mapchksum+=sectorPtr[s]->visibility;
     mapchksum+=sectorPtr[s]->filler;
     mapchksum+=sectorPtr[s]->lotag;
     mapchksum+=sectorPtr[s]->hitag;
     mapchksum+=sectorPtr[s]->extra;
     startwall=sectorPtr[s]->wallptr;
     endwall=startwall+sectorPtr[s]->wallnum;
     for (w=startwall ; w < endwall ; w++) {
          mapchksum+=wallPtr[w]->x;
          mapchksum+=wallPtr[w]->y;
          mapchksum+=wallPtr[w]->point2;
          mapchksum+=wallPtr[w]->nextwall;
          mapchksum+=wallPtr[w]->nextsector;
          mapchksum+=wallPtr[w]->cstat;
          mapchksum+=wallPtr[w]->picnum;
          mapchksum+=wallPtr[w]->overpicnum;
          mapchksum+=wallPtr[w]->shade;
          mapchksum+=wallPtr[w]->pal;
          mapchksum+=wallPtr[w]->xrepeat;
          mapchksum+=wallPtr[w]->yrepeat;
          mapchksum+=wallPtr[w]->xpanning;
          mapchksum+=wallPtr[w]->ypanning;
          mapchksum+=wallPtr[w]->lotag;
          mapchksum+=wallPtr[w]->hitag;
          mapchksum+=wallPtr[w]->extra;
     }
}

void
ENG_initBuild(void)
{
     int  i,j,k,l;
     FILE *fp;

     initengine(videoMode,videoResX,videoResY);
     pskyoff[0]=0;
     pskyoff[1]=0;
     pskyoff[2]=0;
     pskyoff[3]=0;
     pskybits=2;
     for (i=0 ; i < 256 ; i++) {
          tempbuf[i]=i;
     }
     for (i=0 ; i < 17 ; i++) {
          makepalookup(i,tempbuf,0,0,0,1);
     }
     makepalookup(17,tempbuf,24,24,24,1);
     makepalookup(18,tempbuf,8,8,48,1);
     if ((fp=fopen("lookup.dat","rb")) != NULL) {
          l=getc(fp);
          for (j=0 ; j < l ; j++) {
               k=getc(fp);
               for (i=0 ; i < 256 ; i++) {
                    tempbuf[i]=getc(fp);
               }
               makepalookup(k,tempbuf,0,0,0,1);
          }
          fclose(fp);
     }
     if (editorEnabledFlag == 0) {
          loadpics("tiles000.art");
     }
     localViewSize=xdim;
     if (editorEnabledFlag == 0) {
          ENG_setupVideo();
     }
     GFX_initFadePalette();
     engineInitializedFlag=1;
}

void
ENG_uninitBuild(void)
{
     if (engineInitializedFlag) {
          uninitengine();
          setvmode(0x03);
          showengineinfo();
          engineInitializedFlag=0;
     }
}

char *
ENG_picName(short picnum)
{
     strncpy(nameBuf,names[picnum],17);
     if (strlen(nameBuf) == 0) {
          sprintf(nameBuf,"%d",picnum);
     }
     return(nameBuf);
}

void
ENG_analyzeSprites(long dax, long day)
{
     int  angles,frames,i,j,k;
     spritetype *tspr;
     walltype *wall;
     sectortype *sect;

     if (editorEnabledFlag && gameModeFlag == 0) {
          tempbuf[0]=0;
          switch (searchstat) {
          case 0:             // mouse pointer is on wall
          case 4:             // mouse pointer is on masked wall
               wall=wallPtr[searchwall];
               debugOut(windowx1,windowy1,"WALL: %d PIC=%s SHADE=%d PAL=%d "
                        "LOTAG=%d HITAG=%d",searchwall,
                        ENG_picName(wall->picnum),wall->shade,wall->pal,
                        wall->lotag,wall->hitag);
               break;
          case 1:             // mouse pointer is on ceiling/sector
               sect=sectorPtr[searchsector];
               debugOut(windowx1,windowy1,"CEILING: %d PIC=%s SHADE=%d PAL=%d "
                        "HEIGHT=%d",searchsector,
                        ENG_picName(sect->ceilingpicnum),sect->ceilingshade,
                        sect->ceilingpal,sect->ceilingz);
               break;
          case 2:             // mouse pointer is on floor/sector
               sect=sectorPtr[searchsector];
               debugOut(windowx1,windowy1,"FLOOR: %d PIC=%s SHADE=%d PAL=%d "
                        "HEIGHT=%d",searchsector,ENG_picName(sect->floorpicnum),
                        sect->floorshade,sect->floorpal,sect->floorz);
               break;
          case 3:
               tspr=spritePtr[searchwall];
               debugOut(windowx1,windowy1,"SPRITE: %d PIC=%s X=%d Y=%d Z=%d "
                        "SHADE=%d PAL=%d STATUS=%d",searchwall,
                        ENG_picName(tspr->picnum),tspr->x,tspr->y,tspr->z,
                        tspr->shade);
               debugOut(windowx1,windowy1,"SPRITE: %d PAL=%d STATUS=%d "
                        "LOTAG=%d HITAG=%d EXTRA=%d",searchwall,tspr->pal,
                        tspr->statnum,tspr->lotag,tspr->hitag,tspr->extra);
               break;
          }
     }
     for (i=0,tspr=&tsprite[0] ; i < spritesortcnt ; i++,tspr++) {
          angles=0;
          frames=0;
          for (j=0 ; j < MAXDEFINEDACTORS ; j++) {
               if (tspr->picnum == enemyPic[j]) {
                    frames=4;
                    angles=enemyPicAngles[j];
                    break;
               }
               else if (tspr->picnum >= enemyFirePic1[j] &&
                        tspr->picnum <= enemyFirePic2[j]) {
                    frames=(enemyFirePic2[j]-enemyFirePic1[j])+1;
                    angles=enemyFireAngles[j];
                    break;
               }
               else if (tspr->picnum == enemyPainPic[j]) {
                    frames=1;
                    angles=enemyPainAngles[j];
                    break;
               }
               else if (tspr->picnum >= enemyDiePic1[j] &&
                        tspr->picnum <= enemyDiePic2[j]) {
                    frames=(enemyDiePic2[j]-enemyDiePic1[j])+1;
                    angles=enemyDieAngles[j];
                    break;
               }
               else if (tspr->picnum >= enemyGorePic1[j] &&
                        tspr->picnum <= enemyGorePic2[j]) {
                    frames=(enemyGorePic2[j]-enemyGorePic1[j])+1;
                    angles=enemyDieAngles[j];
                    break;
               }
          }
          switch (frames) {
          case 4:
               switch (angles) {
               case 5:
                    k=getangle(tspr->x-dax,tspr->y-day);
                    k=(((tspr->ang+3072+128-k)&2047)>>8)&7;
                    if (k <= 4) {
                         tspr->picnum+=(k<<2);
                         tspr->cstat&=~4;
                    }
                    else {
                         tspr->picnum+=((8-k)<<2);
                         tspr->cstat|=4;
                    }
                    break;
               default:       // 1 angle
                    break;
               }
               break;
          case 2:
               switch (angles) {
               case 5:
                    k=getangle(tspr->x-dax,tspr->y-day);
                    k=(((tspr->ang+3072+128-k)&2047)>>8)&7;
                    if (k <= 4) {
                         tspr->picnum+=(k<<1);
                         tspr->cstat&=~4;
                    }
                    else {
                         tspr->picnum+=((8-k)<<1);
                         tspr->cstat|=4;
                    }
                    break;
               default:       // 1 angle
                    break;
               }
               break;
          default:            // n frames
               switch (angles) {
               case 5:
                    k=getangle(tspr->x-dax,tspr->y-day);
                    k=(((tspr->ang+3072+128-k)&2047)>>8)&7;
                    if (k <= 4) {
                         tspr->picnum+=(k*frames);
                         tspr->cstat&=~4;
                    }
                    else {
                         tspr->picnum+=((8-k)*frames);
                         tspr->cstat|=4;
                    }
                    break;
               default:       // 1 angle
                    break;
               }
               break;
          }
     }
}

void
ENG_tileSineWave(short tilenum)
{
     long j,x,y1,y2,xsize,ysize;
     char *ptr,*ptr2;

     xsize=tilesizy[tilenum];
     ysize=tilesizx[tilenum];
     ptr=(char *)waloff[tilenum];
     ptr2=(char *)waloff[tilenum]+(xsize*(ysize-1));
     for (y1=0,y2=ysize ; y1 < y2 ; y1++,y2--) {
          j=sintable[((y2+numframes)<<5)&2047];
          j+=sintable[((y2-numframes)<<6)&2047];
          j>>=14;
          memmove(ptr,&ptr[j],xsize-j);
          memmove(ptr2,&ptr2[j],xsize-j);
#if 0
          for (x=xsize-j ; x < xsize ; x++) {
               ptr[x]=ptr[x-xsize-j];
               ptr2[x]=ptr2[x-xsize-j];
          }
#endif
          ptr+=xsize;
          ptr2-=xsize;
     }
}

void
ENG_drawScreen(short snum,long smoothRatio)
{
     short playerHeight;
     long hor;
     struct player *plr;
     spritetype *spr;

     spr=spritePtr[snum];
     renderX=spr->x;
     renderY=spr->y;
     renderZ=spr->z;
     renderAng=spr->ang;
     renderSect=spr->sectnum;
     hor=spriteHoriz[snum]+horizAdj[snum];
     if (spr->statnum != STAT_PROJECTILE) {
          playerHeight=ACT_getActorViewHeight(spr);
          renderZ-=playerHeight;
          if (ACT_isDead(snum)) {
               renderZ+=diez[currentView];
          }
          hor+=horizSlopeAdj[snum];
     }
     plr=player[currentView];
     if (plr->viewMode >= 2) { // in a 3D view mode
          if (qsetmode != 200 || viewSize != plr->viewSize) {
               ENG_setupVideo();
               GAM_setupVideo(plr->viewSize);
          }
          switch (sectorPtr[renderSect]->lotag) {
          case SEC_BELOWWATERTAG:
          case SEC_BELOWWATERLODAMAGETAG:
          case SEC_BELOWWATERHIDAMAGETAG:
               if (walock[UNDERWATERTILE] != 255) {
                    walock[UNDERWATERTILE]=255;
                    if (waloff[UNDERWATERTILE] == 0) {
                         allocache(&waloff[UNDERWATERTILE],320L*200L,
                                   &walock[UNDERWATERTILE]);
                    }
                    tilesizx[UNDERWATERTILE]=200;
                    tilesizy[UNDERWATERTILE]=320;
               }
               setviewtotile(UNDERWATERTILE,200L,320L);
               break;
          default:
               if (walock[UNDERWATERTILE] == 255) {
                    walock[UNDERWATERTILE]=1;
               }
               break;
          }
          drawrooms(renderX,renderY,renderZ,renderAng,hor,renderSect);
          ENG_analyzeSprites(renderX,renderY);
          drawmasks();
          WEP_drawWeapon(snum,currentWeapon[snum],fireFrame[snum]);
          switch (sectorPtr[renderSect]->lotag) {
          case SEC_BELOWWATERTAG:
          case SEC_BELOWWATERLODAMAGETAG:
          case SEC_BELOWWATERHIDAMAGETAG:
               setviewback();
               if (walock[UNDERWATERTILE] == 255) {
                    ENG_tileSineWave(UNDERWATERTILE);
                    rotatesprite(320L<<15,200L<<15,65536L,512,
                                 UNDERWATERTILE,0,0,2+4+64,
                                 windowx1,windowy1,windowx2,windowy2);
               }
               break;
          }
          if (cachecount != 0L) {
               rotatesprite(320L-16L<<16,200L-16L<<16,32768,0,DISKPIC,
                            0,0,2+64,windowx1,windowy1,windowx2,windowy2);
               cachecount=0L;
          }
     }
     switch (plr->viewMode) {
     case 3:
          PLR_checkStatus();
          ENG_debugChain();
          EFF_showMessages();
          break;
     case 2:
          ENG_drawOverheadMap(renderX,renderY,plr->zoom,renderAng);
          PLR_checkStatus();
          ENG_debugChain();
          EFF_showMessages();
          break;
     case 1:
          WEP_changeWeaponAnim(snum);
          if (qsetmode != 200L) {
               ENG_setupVideo();
               GAM_setupVideo(plr->viewSize);
          }
          clearview(GREENBASE+8L);
          ENG_drawOverheadMap(renderX,renderY,plr->zoom,renderAng);
          PLR_checkStatus();
          ENG_debugChain();
          EFF_showMessages();
          break;
     case 0:
          WEP_changeWeaponAnim(snum);
          clear2dscreen(2);
          if (qsetmode == 200L) {
               qsetmode640480();
          }
          PLR_checkStatus();
          ENG_debugChain();
          EFF_showMessages();
          draw2dscreen(renderX,renderY,renderAng,plr->zoom,plr->gridSize);
          break;
     }
     WEP_showMissileViewStatus();
}

short
ENG_getParallel(short wall)
{
     long wx1,wy1,wx2,wy2;
     walltype *wal;

     wal=wallPtr[wall];
     wx1=wal->x;
     wy1=wal->y;
     wal=wallPtr[wal->point2];
     wx2=wal->x;
     wy2=wal->y;
     return((short)getangle(wx2-wx1,wy2-wy1));
}

short
ENG_getNormal(short wall)
{
     return((ENG_getParallel(wall)+512)&2047);
}

long
ENG_alcmem(long numbytes)
{
     int  i;

     for (i=0 ; i < MAXMEMPTRS ; i++) {
          if (memptr[i].cache_ptr == 0L) {
               break;
          }
          else if (memptr[i].cache_lock < 200) {
               if (memptr[i].cache_length >= numbytes) {
                    memptr[i].cache_lock=202;
                    return(memptr[i].cache_ptr);
               }
          }
     }
     if (i == MAXMEMPTRS) {
          return(0L);
     }
     memptr[i].cache_lock=214;
     memptr[i].cache_length=numbytes;
     allocache(&(memptr[i].cache_ptr), memptr[i].cache_length,
               &(memptr[i].cache_lock));
     return(memptr[i].cache_ptr);
}

void
ENG_alcfree(long ptr)
{
     int  i;

     for (i=0 ; i < MAXMEMPTRS ; i++) {
          if (memptr[i].cache_ptr == ptr) {
               memptr[i].cache_lock=1;
               break;
          }
     }
}

extern
int  gotpck,
     sndpck;

void
ENG_frameRate(void)
{
     long i;

     i=totalclock;
     if (i != frameval[framecnt]) {
          avgFPS=(TMR_getSecondTics(1)*AVERAGEFRAMES)/(i-frameval[framecnt]);
          debugOut(windowx1,windowy1,"P=%d V=%d: %3ld %s (%d,%d-%d,%d)",myconnectindex,
                   currentView,avgFPS,
                   (myconnectindex == connecthead) ? "MASTER" : "SLAVE",
                   windowx1,windowy1,windowx2,windowy2);
          if (editorEnabledFlag) {
               debugOut(windowx1,windowy1,"%s NUMSECTORS=%d NUMWALLS=%d",
                        gameModeFlag ? "GAME MODE" : "EDIT MODE",
                        numsectors,numwalls);
          }
          frameval[framecnt]=i;
     }
     framecnt=((framecnt+1)&(AVERAGEFRAMES-1));
}

void
ENG_debug(void)
{
#ifdef ANALYZEMEMPTRSDEBUG
     int  i;

     memset(tempbuf,0,sizeof(tempbuf));
     for (i=0 ; i < MAXMEMPTRS ; i++) {
          if (i > 0 && (i%80) == 0) {
               debugOut(windowx1,windowy1,tempbuf);
               memset(tempbuf,0,sizeof(tempbuf));
          }
          if (memptr[i].cache_lock > 1) {
               strcat(tempbuf,"1");
          }
          else {
               strcat(tempbuf,"0");
          }
     }
     if ((i%80) != 0) {
          debugOut(windowx1,windowy1,tempbuf);
     }
#endif
}

void
ENG_debugChain(void)
{
     if (showFrameRateFlag) {
          ENG_frameRate();
     }
     if (debugModeFlag) {
          if (gameModeFlag) {
               PLR_debug();
               ENG_debug();
               SND_debug();
               MUL_debug();
               EFF_debug();
               WEP_debug();
               ACT_debug();
          }
     }
     debugOutput();
}

char *
ENG_getStatusName(short stat)
{
     switch (stat) {
     case STAT_NONE:
          return("NONE");
     case STAT_HITSCANEXPLODE:
          return("HITSCAN EXPLODE");
     case STAT_PROJECTILEEXPLODE:
          return("PROJECTILE EXPLODE");
     case STAT_SHOCKWAVE:
          return("SHOCKWAVE EXPLODE");
     case STAT_EXPLODESPRITE:
          return("EXPLODE SPRITE");
     case STAT_DIEANIM:
          return("DIE ANIMATION");
     case STAT_BLOOD:
          return("GORE");
     case STAT_PROXIMITY:
          return("PROXIMITY DEVICE");
     case STAT_PATHMARKER:
          return("PATH MARKER");
     case STAT_MOVINGSECTORMARKER:
          return("VEHICLE PATH");
     case STAT_FOOTSTEP:
          return("FOOTSTEP");
     case STAT_AMBIENT:
          return("AMBIENT SOUND");
     case STAT_PROJECTILE:
          return("PROJECTILE");
     case STAT_PLAYER:
          return("PLAYER");
     case STAT_ALIVE:
          return("ALIVE");
     case STAT_PATROL:
          return("PATROL");
     case STAT_WANDER:
          return("WANDER");
     case STAT_AMBUSH:
          return("AMBUSH");
     case STAT_GUARD:
          return("GUARD");
     case STAT_PAIN:
          return("PAIN");
     case STAT_CHASE:
          return("CHASE");
     case STAT_CHASING:
          return("CHASING");
     case STAT_ATTACK:
          return("ATTACK");
     case STAT_ATTACKING:
          return("ATTACKING");
     case STAT_DODGE:
          return("DODGE");
     case STAT_DODGING:
          return("DODGING");
     }
     return(NULL);
}

char *
ENG_getSectorTagName(short tag)
{
     switch (tag) {
     case SEC_BLINKOFFTAG:
          return("BLINK OFF");
     case SEC_BLINKONTAG:
          return("BLINK ON");
     case SEC_BLINKON1STAG:
          return("BLINK ON 1SEC");
     case SEC_HIDAMAGEBLINKTAG:
          return("HI DAMAGE BLINK");
     case SEC_MEDDAMAGETAG:
          return("MEDIUM DAMAGE");
     case SEC_LODAMAGETAG:
          return("LO DAMAGE");
     case SEC_OSCILLATETAG:
          return("OSCILLATE");
     case SEC_SECRETTAG:
          return("SECRET ROOM");
     case SEC_CLOSE30STAG:
          return("CLOSE AFTER 30S");
     case SEC_HIDAMAGEENDTAG:
          return("HI DAMAGE ENDLEV");
     case SEC_SYNCBLINKONTAG:
          return("SYNC BLINK ON");
     case SEC_SYNCBLINKOFFTAG:
          return("SYNC BLINK OFF");
     case SEC_OPEN5MTAG:
          return("OPEN AFTER 5 MIN");
     case SEC_HIDAMAGETAG:
          return("HI DAMAGE");
     case SEC_FLICKERTAG:
          return("FLICKER");
     case SEC_DOORUPTAG:
          return("DOOR UP");
     case SEC_DOORUPONCETAG:
          return("DOOR UP ONCE");
     case SEC_DOORDOWNTAG:
          return("DOOR DOWN");
     case SEC_DOORDOWNONCETAG:
          return("DOOR DOWN ONCE");
     case SEC_PLATFORMDOWNTAG:
          return("PLATFORM LIFT");
     case SEC_DOORHSPLITTAG:
          return("HORIZ DOOR");
     case SEC_DOORHSPLITONCETAG:
          return("HORIZ DOOR ONCE");
     case SEC_DOORSLIDECENTERTAG:
          return("SLIDE DOOR");
     case SEC_ROTATESECTORTAG:
          return("ROTATE SECTOR");
     case SEC_BONUSSECTORTAG:
          return("BONUS SECTOR");
     case SEC_PLATFORMELEVATORTAG:
          return("PLATFORM ELEV");
     case SEC_ABOVEWATERTAG:
          return("ABOVE WATER");
     case SEC_BELOWWATERTAG:
          return("BELOW WATER");
     case SEC_ABOVEWATERLODAMAGETAG:
          return("ABOVE WATER HI DAMAGE");
     case SEC_BELOWWATERLODAMAGETAG:
          return("BELOW WATER HI DAMAGE");
     case SEC_ABOVEWATERHIDAMAGETAG:
          return("ABOVE WATER LO DAMAGE");
     case SEC_BELOWWATERHIDAMAGETAG:
          return("BELOW WATER LO DAMAGE");
     case SEC_MOVINGSECTORTAG:
          return("MOVING SEC");
     case SEC_AUTOMOVINGSECTORTAG:
          return("MOVING SEC TMR");
     case SEC_MOVINGSECTORFIXEDTAG:
          return("FIXED MOVING");
     case SEC_AUTOMOVINGSECTORFIXEDTAG:
          return("FIXED MOVING TMR");
     case SEC_PLATFORMUPTAG:
          return("PLATFORM UP");
     }
     return(NULL);
}

char *
ENG_getWallTagName(short tag)
{
     switch (tag) {
     case WAL_W1LIGHTSOFFTAG:
          return("LIGHTS OFF W1");
     case WAL_WRLIGHTSOFFTAG:
          return("LIGHTS OFF WR");
     case WAL_SRLIGHTSOFFTAG:
          return("LIGHTS OFF SR");
     case WAL_W1LIGHTSONFULLTAG:
          return("LIGHTS ON W1");
     case WAL_WRLIGHTSONFULLTAG:
          return("LIGHTS ON WR");
     case WAL_SRLIGHTSONFULLTAG:
          return("LIGHTS ON SR");
     case WAL_DOORSEARCHTAG:
          return("DOORMARK");
     case WAL_SWITCHTAG:
          return("SWITCH");
     case WAL_TRIGGERTAG:
          return("TRIGGER");
     case WAL_WARPTAG:
          return("TELEPORT");
     }
     return(NULL);
}

char *
ENG_getSpriteTagName(short tag)
{
     switch (tag) {
     case SPR_PLAYER1STARTTAG:
          return("PLAYER 1 START");
     case SPR_PLAYER2STARTTAG:
          return("PLAYER 2 START");
     case SPR_PLAYER3STARTTAG:
          return("PLAYER 3 START");
     case SPR_PLAYER4STARTTAG:
          return("PLAYER 4 START");
     case SPR_BLUEKEYTAG:
          return("ACCESS: BLUE1");
     case SPR_YELLOWKEYTAG:
          return("ACCESS: YELLOW1");
     case SPR_BACKPACKTAG:
          return("AMMO: FULL PACK");
     case SPR_DEATHMATCHSTARTTAG:
          return("DEATHMATCH START");
     case SPR_REDKEYTAG:
          return("ACCESS: RED1");
     case SPR_TELEPORTTAG:
          return("TELEPORT");
     case SPR_CELLCHARGEPACKTAG:
          return("AMMO: FULL ENRGY");
     case SPR_REDSKULLKEYTAG:
          return("ACCESS: RED2");
     case SPR_YELLOWSKULLKEYTAG:
          return("ACCESS: YELLOW2");
     case SPR_BLUESKULLKEYTAG:
          return("ACCESS: BLUE2");
     case SPR_DOUBLESHOTGUNTAG:
          return(weaponPtr[GUN_4]->name);
     case SPR_MEGASPHERETAG:
          return("HEALTH: MEGA");
     case SPR_SHOTGUNTAG:
          return(weaponPtr[GUN_3]->name);
     case SPR_CHAINGUNTAG:
          return(weaponPtr[GUN_1]->name);
     case SPR_ROCKETLAUNCHERTAG:
          return(weaponPtr[GUN_5]->name);
     case SPR_PLASMAGUNTAG:
          return(weaponPtr[GUN_6]->name);
     case SPR_CHAINSAWTAG:
          return(weaponPtr[GUN_2]->name);
     case SPR_BFG9000TAG:
          return(weaponPtr[GUN_7]->name);
     case SPR_AMMOCLIPTAG:
          return("AMMO: BULLET");
     case SPR_SHOTGUNSHELLSTAG:
          return("AMMO: SHELLS");
     case SPR_ROCKETTAG:
          return("AMMO: ENERGY");
     case SPR_STIMPACKTAG:
          return("HEALTH: SMALL");
     case SPR_MEDIKITTAG:
          return("HEALTH: LARGE");
     case SPR_SOULSPHERETAG:
          return("HEALTH: SUPER");
     case SPR_HEALTHPOTIONTAG:
          return("HEALTH: 1UP");
     case SPR_SPIRITARMORTAG:
          return("ARMOR: SUPER");
     case SPR_GREENARMORTAG:
          return("ARMOR: 100%");
     case SPR_BLUEARMORTAG:
          return("ARMOR: 200%");
     case SPR_INVULNERABILITYTAG:
          return("BONUS: NO HURT");
     case SPR_BERSERKTAG:
          return("BONUS: 4XDAMAGE");
     case SPR_INVISIBILITYTAG:
          return("ITEM: CLOAKING");
     case SPR_RADIATIONSUITTAG:
          return("ITEM: RAD SUIT");
     case SPR_COMPUTERMAPTAG:
          return("ITEM: MAPPER");
     case SPR_LIGHTAMPLIFICATIONTAG:
          return("ITEM: NIGHT VIS");
     case SPR_BOXOFROCKETSTAG:
          return("AMMO: 100% ENRGY");
     case SPR_CELLCHARGETAG:
          return("AMMO:  50% ENRGY");
     case SPR_BOXOFAMMOTAG:
          return("AMMO: BULLET BOX");
     case SPR_BOXOFSHELLSTAG:
          return("AMMO: SHELL BOX");
     case SPR_PLAYER5STARTTAG:
          return("PLAYER 5 START");
     case SPR_PLAYER6STARTTAG:
          return("PLAYER 6 START");
     case SPR_PLAYER7STARTTAG:
          return("PLAYER 7 START");
     case SPR_PLAYER8STARTTAG:
          return("PLAYER 8 START");
     case SPR_SWITCHTAG:
          return("SWITCH");
     case SPR_PANCEILTAG:
          return("PAN CEILING");
     case SPR_PANFLOORTAG:
          return("PAN FLOOR");
     case SPR_MOVINGSECTORTAG:
          return("MOVING SECTOR MARKER");
     case SPR_DETAIL1:
          return("LOW DETAIL");
     case SPR_DETAIL2:
          return("MEDIUM DETAIL");
     case SPR_DETAIL3:
          return("HIGH DETAIL");
     case SPR_DETAIL4:
          return("EXTRA HIGH DETAIL");
     }
     return(NULL);
}

char *
ENG_getActorModeName(short n)
{
     switch (n) {
     case ACT_PATROL:
          return("PATROL");
     case ACT_WANDER:
          return("WANDER");
     case ACT_AMBUSH:
          return("AMBUSH");
     case ACT_GUARD:
          return("GUARD");
     case ACT_ATTACK:
          return("ATTACK");
     }
     return(NULL);
}

int
ENG_getInput(char *ch,char *stat)
{
     if (keyfifoplc != keyfifoend) {
          *ch=keyfifo[keyfifoplc];
          *stat=keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)];
          keyfifoplc=(keyfifoplc+2)&(KEYFIFOSIZ-1);
          if (*stat == 1) {
               return(1);
          }
     }
     return(0);
}

#if 0
int
ENG_getHighlightedInput(short cur,short num,char *hdr,char *(*nameFnc)(short))
{
     short c,more=1,n,startx,starty;
     char ch,stat,*ptr;

     if (qsetmode != 200L) {
          c=cur;
          clearmidstatbar16();
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,hdr);
          startx=midstatbarx;
          starty=midstatbary;
          do {
               ENG_setMidStatbarPos(startx,starty);
               for (n=0 ; n < num ; n++) {
                    ptr=nameFnc(n);
                    if (ptr != NULL) {
                         if (c == n) {
                              ENG_printHelpText16(15,1,"%3d = %s",n,ptr);
                         }
                         else {
                              ENG_printHelpText16(14,1,"%3d = %s",n,ptr);
                         }
                    }
               }
               keyfifoplc=keyfifoend;
               while (ENG_getInput(&ch,&stat) == 0);
               if (ch == K_UP) {
                    if (c > 0) {
                         c--;
                    }
               }
               else if (ch == K_DOWN) {
                    if (c < num-1) {
                         c++;
                    }
               }
               else if (ch == K_ENTER) {
                    more=2;
               }
               else if (ch == K_ESC) {
                    more=0;
               }
          } while (more == 1);
     }
     if (more != 0) {
          return(c);
     }
     return(cur);
}
#endif

void
ENG_setMidStatbarPos(short x,short y)
{
     midstatbarx=x;
     midstatbary=y;
}

void
ENG_printHelpText16(short color,short font,char *fmt,...)
{
     char locbuf[80];
     va_list argptr;

     va_start(argptr,fmt);
     vsprintf(locbuf,fmt,argptr);
     va_end(argptr);
     if (strlen(locbuf) == 0 && midstatbary == TOPSTATBARY+10) {
          return;
     }
     printext16(midstatbarx,midstatbary,color,-1,locbuf,font);
     if (midstatbary < TOPSTATBARY+10) {
          midstatbary=TOPSTATBARY+10;
     }
     else {
          midstatbary+=7;
          if (midstatbary >= TOPSTATBARY+10+72) {
               midstatbary=TOPSTATBARY+10;
               midstatbarx+=100;
          }
     }
}

void
ENG_showSectorTags(void)
{
     short i;
     char *ptr;

     if (qsetmode != 200L) {
          clearmidstatbar16();
//** sector tags
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"-SECTOR TAGS-");
          for (i=0 ; i < SEC_LASTTAG ; i++) {
               ptr=ENG_getSectorTagName(i);
               if (ptr != NULL) {
                    ENG_printHelpText16(14,1,"%4d = %-16.16s",i,ptr);
               }
          }
     }
}

void
ENG_showWallTags(void)
{
     short i;
     char *ptr;

     if (qsetmode != 200L) {
          clearmidstatbar16();
//** wall tags
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"-WALL TAGS-");
          for (i=0 ; i < WAL_LASTTAG ; i++) {
               ptr=ENG_getWallTagName(i);
               if (ptr != NULL) {
                    ENG_printHelpText16(14,1,"%4d = %-16.16s",i,ptr);
               }
          }
     }
}

void
ENG_showSpriteTags(void)
{
     short i;
     char *ptr;

     if (qsetmode != 200L) {
          clearmidstatbar16();
//** sprite tags
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"-SPRITE TAGS-");
          for (i=0 ; i < SPR_LASTTAG ; i++) {
               ptr=ENG_getSpriteTagName(i);
               if (ptr != NULL) {
                    ENG_printHelpText16(14,1,"%4d = %-16.16s",i,ptr);
               }
          }
     }
}

void
ENG_showSpriteStatnums(void)
{
     short i;
     char *ptr;

     if (qsetmode != 200L) {
          clearmidstatbar16();
//** sprite statnums
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"-SPRITE STATNUMS-");
          for (i=0 ; i < STAT_LASTSTAT ; i++) {
               ptr=ENG_getStatusName(i);
               if (ptr != NULL) {
                    ENG_printHelpText16(14,1,"%4d = %s",i,ptr);
               }
          }
     }
}

void
ENG_showEditorCommands(void)
{
     if (qsetmode != 200L) {
          clearmidstatbar16();
//** function key commands
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"EDITOR HELP");
          ENG_printHelpText16(14,1," F1  = HELP SCREEN");
          ENG_printHelpText16(14,1," F2  = SECTOR TAGS");
          ENG_printHelpText16(14,1,"^F2  = WALL TAGS");
          ENG_printHelpText16(14,1," F3  = SPRITE TAGS");
          ENG_printHelpText16(14,1,"^F3  = SPRITE STATNUMS");
          ENG_printHelpText16(14,1," F4  = MAP TOTALS");
          ENG_printHelpText16(14,1," F5  = SECTOR INFO");
          ENG_printHelpText16(14,1," F6  = WALL/SPRITE INFO");
          ENG_printHelpText16(14,1," F7  = EDIT SECTOR");
          ENG_printHelpText16(14,1," F8  = EDIT WALL/SPRITE");
//** key combination commands
          ENG_setMidStatbarPos(108,TOPSTATBARY);
          ENG_printHelpText16(11,0,"");
          ENG_printHelpText16(14,1,"^R  = RESET MAP");
          ENG_printHelpText16(14,1,"^[  = SEARCH BACK");
          ENG_printHelpText16(14,1," [  = SEARCH BACK AGAIN");
          ENG_printHelpText16(14,1,"^]  = SEARCH AHEAD");
          ENG_printHelpText16(14,1," ]  = SEARCH AHEAD AGAIN");
          ENG_printHelpText16(14,1,"^F  = FIND SPRITE");
          ENG_printHelpText16(14,1," E  = SPRITE STATNUM");
     }
}

extern
char actorPicIndex[MAXTILES];

short actorPicTotal[MAXDEFINEDACTORS],
     actorStatusTotal[STAT_LASTSTAT],
     sectorTagTotal[SEC_LASTTAG],
     spriteTagTotal[SPR_LASTTAG],
     wallTagTotal[WAL_LASTTAG];

void
ENG_showCounts(void)
{
     short i,j;
     char *ptr;

     if (qsetmode == 200L) {
          return;
     }
     memset(actorPicTotal,0,sizeof(actorPicTotal));
     memset(actorStatusTotal,0,sizeof(actorStatusTotal));
     memset(sectorTagTotal,0,sizeof(sectorTagTotal));
     memset(spriteTagTotal,0,sizeof(spriteTagTotal));
     memset(wallTagTotal,0,sizeof(wallTagTotal));
     clearmidstatbar16();
     EFF_notify(0L,"Counting sectors...");
     for (i=0 ; i < numsectors ; i++) {
          if (sectorPtr[i]->lotag >= SEC_LASTTAG) {
               continue;
          }
          if (sectorPtr[i]->lotag < 0) {
               continue;
          }
          sectorTagTotal[sectorPtr[i]->lotag]++;
     }
     printmessage16("");
     ENG_setMidStatbarPos(8,TOPSTATBARY);
     ENG_printHelpText16(11,0,"-SECTORS-");
     for (i=SEC_NORMAL ; i < SEC_LASTTAG ; i++) {
          if ((ptr=ENG_getSectorTagName(i)) != NULL && sectorTagTotal[i] > 0) {
               ENG_printHelpText16(14,1,"%-15.15s=%4d",ptr,sectorTagTotal[i]);
          }
     }
     EFF_notify(0L,"Counting walls...");
     for (i=0 ; i < numwalls ; i++) {
          if (wallPtr[i]->lotag >= WAL_LASTTAG) {
               continue;
          }
          if (wallPtr[i]->lotag < 0) {
               continue;
          }
          wallTagTotal[wallPtr[i]->lotag]++;
     }
     printmessage16("");
     ENG_setMidStatbarPos(midstatbarx+100,TOPSTATBARY);
     ENG_printHelpText16(11,0,"-WALLS-");
     for (i=WAL_NORMAL ; i < WAL_LASTTAG ; i++) {
          if ((ptr=ENG_getWallTagName(i)) != NULL && wallTagTotal[i] > 0) {
               ENG_printHelpText16(14,1,"%-15.15s=%4d",ptr,wallTagTotal[i]);
          }
     }
     EFF_notify(0L,"Counting sprites...");
     for (i=0 ; i < MAXSPRITES ; i++) {
          if (spritePtr[i]->statnum < MAXSTATUS) {
               if (SND_isSectorSound(i)) {
                    continue;
               }
               if (SND_isAmbientSound(i)) {
                    continue;
               }
               if (ACT_isActor(i)) {
                    j=actorPicIndex[i];
                    if (j >= 0 && j < MAXDEFINEDACTORS) {
                         actorPicTotal[j]++;
                    }
                    actorStatusTotal[spritePtr[i]->statnum]++;
                    continue;
               }
               if (spritePtr[i]->lotag >= SPR_LASTTAG) {
                    continue;
               }
               if (spritePtr[i]->lotag < 0) {
                    continue;
               }
               spriteTagTotal[spritePtr[i]->lotag]++;
          }
     }
     printmessage16("");
     ENG_setMidStatbarPos(midstatbarx+100,TOPSTATBARY);
     ENG_printHelpText16(11,0,"-SPRITES-");
     for (i=SPR_NORMAL ; i < SPR_LASTTAG ; i++) {
          if ((ptr=ENG_getSpriteTagName(i)) != NULL && spriteTagTotal[i] > 0) {
               ENG_printHelpText16(14,1,"%-15.15s=%4d",ptr,spriteTagTotal[i]);
          }
     }
     ENG_setMidStatbarPos(midstatbarx+100,TOPSTATBARY);
     ENG_printHelpText16(11,0,"-ACTORS-");
     for (i=0 ; i < MAXDEFINEDACTORS ; i++) {
          if ((j=actorPicTotal[i]) > 0) {
               ENG_printHelpText16(14,1,"ACTOR%02d        =%4d",i+1,j);
          }
     }
     for (i=STAT_NONE ; i < STAT_LASTSTAT ; i++) {
          if ((ptr=ENG_getStatusName(i)) != NULL) {
               if (actorStatusTotal[i] > 0) {
                    ENG_printHelpText16(14,1,"%-15.15s=%4d",ptr,
                                        actorStatusTotal[i]);
               }
          }
     }
}

void
ENG_showWeaponHelp(void)
{
     short i;

     if (qsetmode != 200L) {
          clearmidstatbar16();
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"WEAPONS");
          for (i=0 ; i < MAXWEAPONS ; i++) {
               if (weaponPtr[i]->registered) {
                    sprintf(tempbuf,"%2d  = %s",i,weaponPtr[i]->name);
                    strupr(tempbuf);
                    ENG_printHelpText16(14,1,tempbuf);
               }
          }
          ENG_printHelpText16(14,1,"%2d  = RANDOM",MAXWEAPONS);
     }
}

void
ENG_showActorTypes(void)
{
     short n;
     char *ptr;

     if (qsetmode != 200L) {
          clearmidstatbar16();
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"ACTOR MODES");
          for (n=0 ; n < ACT_LAST ; n++) {
               ptr=ENG_getActorModeName(n);
               if (ptr != NULL) {
                    ENG_printHelpText16(14,1,"%3d = %s",n,ptr);
               }
          }
     }
}

char *
ENG_getBonusItemName(short i)
{
     switch (i) {
     case BON_NOTHING:
          return("NOTHING");
     case BON_HEALTH:
          return("HEALTH");
     case BON_BULLETAMMO:
          return("BULLETS");
     case BON_MISSILEAMMO:
          return("ROCKETS");
     case BON_MINEAMMO:
          return("MINES");
     case BON_ENERGYAMMO:
          return("ENERGY");
     case BON_BODYARMOR:
          return("ARMOR");
     case BON_WEAPON:
          return("WEAPON");
     }
     return(NULL);
}

void
ENG_showBonusItems(void)
{
     short i;
     char *ptr;

     if (qsetmode != 200L) {
          clearmidstatbar16();
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"BONUS ITEMS");
          for (i=0 ; i < MAXBONUSITEMS ; i++) {
               if ((ptr=ENG_getBonusItemName(i)) != NULL) {
                    sprintf(tempbuf,"%2d  = %s",i,ptr);
                    strupr(tempbuf);
                    ENG_printHelpText16(14,1,tempbuf);
               }
          }
     }
}

void                               // initialize timer & sound routines
inittimer(void)
{
     TMR_initTimer();
}

void                               // uninitialize timer & sound routines
uninittimer(void)
{
     TMR_uninitTimer();
}

void 
ExtUnInit(void)
{
     shutdown();
}

void
ExtInit(void)
{
     startup();
     initgroupfile("duke3d.grp");
     memcpy((void *)buildkeys, (void *)defBuildKeys, sizeof(defBuildKeys));
     checkParms();
     CFG_readConfigFile();
     if (configVideoFlag) {
          checkUserOptions();
     }
     showSelectedVideoMode();
     GAM_initUniverse();
     ENG_initBuild();
     MUL_initMultiPlayers();
     PLR_initMouse();
     PLR_initPlayers();
     if (editorEnabledFlag == 0) {
          initcrc();
          initkeys();         // these must stay here because in editor mode
          inittimer();        // ..they are called when this function exits
          SND_initSOS();
          ENG_setupVideo();
          GFX_fadeIn(255);
          gameModeFlag=1;
          gameMain();
          crash("normal shutdown");
     }
     kensplayerheight=defaultPlayerHeight-(defaultPlayerHeight>>2);
}

void 
ExtPreCheckKeys(void)
{
}

void 
ExtAnalyzeSprites(void)
{
     ENG_analyzeSprites(posx,posy);
}

void
ENG_getDistanceAngleInfo(void)
{
     short ang1,ang2,fov=256,hiang,loang,s1=-1,s2=-1;
     long dx;
     spritetype *spr1,*spr2;

     s1=getnumber16("Enter source sprite number: ",s1,MAXSPRITES);
     if (s1 == -1) {
          return;
     }
     s2=getnumber16("Enter target sprite number: ",s2,MAXSPRITES);
     if (s2 == -1) {
          return;
     }
     fov=getnumber16("Enter field of view of source sprite (0-2048): ",fov,2048);
     printmessage16("");
     spr1=spritePtr[s1];
     if (spr1->statnum >= MAXSTATUS) {
          printmessage16("SOURCE SPRITE IS INVALID");
          return;
     }
     spr2=spritePtr[s2];
     if (spr2->statnum >= MAXSTATUS) {
          printmessage16("TARGET SPRITE IS INVALID");
          return;
     }
     ang1=spr1->ang;
     ang2=getangle(spr2->x-spr1->x,spr2->y-spr1->y);
     hiang=(spr1->ang+(fov>>1))&2047;
     loang=(hiang-fov)&2047;
     dx=WEP_getDistance(spr1,spr2);
     clearmidstatbar16();
     ENG_setMidStatbarPos(8,TOPSTATBARY);
     ENG_printHelpText16(11,0,"SPRITE %d FOV=%d",s1,fov);
     ENG_printHelpText16(14,1,"FACING ANGLE   =%d",ang1);
     ENG_printHelpText16(14,1,"ANGLE TO TARGET=%d",ang2);
     ENG_printHelpText16(14,1,"DIST TO TARGET =%ld",dx);
     ENG_printHelpText16(14,1,"FOV LOANG      =%d",loang);
     ENG_printHelpText16(14,1,"FOV HIANG      =%d",hiang);
     if (WEP_canSee(spr1,spr2,fov)) {
          ENG_printHelpText16(10,1,"SOURCE CAN SEE TARGET");
     }
     else {
          ENG_printHelpText16(12,1,"SOURCE CANNOT SEE TARGET");
     }
}

void
ENG_getSearchSprite(void)
{
     short nickdata;

     nickdata=getnumber16("Enter sprite number to find: ",0,MAXSPRITES);
     if (spritePtr[nickdata]->statnum < MAXSTATUS) {
          posx=spritePtr[nickdata]->x;
          posy=spritePtr[nickdata]->y;
          posz=spritePtr[nickdata]->z;
          ExtShowSpriteData(nickdata);
          printmessage16("FOUND SPRITE");
     }
     else {
          printmessage16("SPRITE IS INVALID");
     }
}

void
ENG_getSearchKeys(void)
{
     short nickdata,
          startpic;

     nickdata=getnumber16("lotag to search for (0=none): ",0,65536L);
     lotagSearch=nickdata;
     nickdata=getnumber16("hitag to search for (0=none): ",0,65536L);
     hitagSearch=nickdata;
     picnumSearch=-1;
     if (getboolean16("Search for picnum? ",0)) {
          switch (searchstat) {
          case 0:
          case 4:
               startpic=wallPtr[searchwall]->picnum;
               break;
          case 1:
               startpic=sectorPtr[searchsector]->ceilingpicnum;
               break;
          case 2:
               startpic=sectorPtr[searchsector]->floorpicnum;
               break;
          case 3:
               startpic=spritePtr[searchwall]->picnum;
               break;
          default:
               startpic=0;
               break;
          }
          setgamemode();
          picnumSearch=gettile(0L);
          qsetmode640480();
          clearmidstatbar16();
          printmessage16("");
     }
}

void
ENG_getPrevSprite(void)
{
     int  match,
          startSearch;

     startSearch=searchNum;
     while (1) {
          if (searchNum > 0) {
               searchNum--;
          }
          else {
               searchNum=MAXSPRITES-1;
               printmessage16("SEARCH WRAPPED");
          }
          if (searchNum == startSearch) {
               break;
          }
          if (spritePtr[searchNum]->statnum >= MAXSTATUS) {
               continue;
          }
          match=0;
          if (lotagSearch > 0) {
               if (spritePtr[searchNum]->lotag == lotagSearch) {
                    match=1;
               }
          }
          if (hitagSearch > 0) {
               if (spritePtr[searchNum]->hitag == hitagSearch) {
                    match=2;
               }
          }
          if (picnumSearch > 0) {
               if (spritePtr[searchNum]->picnum == picnumSearch) {
                    match=4;
               }
          }
          if ((lotagSearch > 0 && hitagSearch > 0 && (match&3) == 3) ||
              (lotagSearch > 0 && hitagSearch == 0 && (match&1) == 1) ||
              (lotagSearch == 0 && hitagSearch > 0 && (match&2) == 2) ||
              (picnumSearch >= 0 && (match&4) == 4)) {
               posx=spritePtr[searchNum]->x;
               posy=spritePtr[searchNum]->y;
               posz=spritePtr[searchNum]->z;
               ExtShowSpriteData(searchNum);
               sprintf(tempbuf,"Found sprite #%d",searchNum);
               printmessage16(tempbuf);
               return;
          }
     }
     printmessage16("END OF SEARCH");
     searchNum=0;
}

void
ENG_getNextSprite(void)
{
     int  match,
          startSearch;

     startSearch=searchNum;
     while (1) {
          if (searchNum < MAXSPRITES-1) {
               searchNum++;
          }
          else {
               searchNum=0;
               printmessage16("SEARCH WRAPPED");
          }
          if (searchNum == startSearch) {
               break;
          }
          if (spritePtr[searchNum]->statnum >= MAXSTATUS) {
               continue;
          }
          match=0;
          if (lotagSearch > 0) {
               if (spritePtr[searchNum]->lotag == lotagSearch) {
                    match|=1;
               }
          }
          if (hitagSearch > 0) {
               if (spritePtr[searchNum]->hitag == hitagSearch) {
                    match|=2;
               }
          }
          if (picnumSearch >= 0) {
               if (spritePtr[searchNum]->picnum == picnumSearch) {
                    match|=4;
               }
          }
          if ((lotagSearch > 0 && hitagSearch > 0 && (match&3) == 3) ||
              (lotagSearch > 0 && hitagSearch == 0 && (match&1) == 1) ||
              (lotagSearch == 0 && hitagSearch > 0 && (match&2) == 2) ||
              (picnumSearch >= 0 && (match&4) == 4)) {
               posx=spritePtr[searchNum]->x;
               posy=spritePtr[searchNum]->y;
               posz=spritePtr[searchNum]->z;
               ExtShowSpriteData(searchNum);
               sprintf(tempbuf,"Found sprite #%d",searchNum);
               printmessage16(tempbuf);
               return;
          }
     }
     printmessage16("END OF SEARCH");
     searchNum=0;
}

void
ExtCheckKeys(void)
{
     char ch,stat;
     short s;
     static long oqsetmode;
     static int altflag,firstpass,soundinitflag;

     if (!soundinitflag) {
          SND_initSOS();
          soundinitflag=1;
     }
     if (gameModeFlag) {
          strcpy(mapFileName,"edtboard.map");
          saveboard(mapFileName,&posx,&posy,&posz,&ang,&cursectnum);
          gameMain();
          SND_stopAllSounds();
          gameModeFlag=0;
          loadboard(mapFileName,&posx,&posy,&posz,&ang,&cursectnum);
          posx=renderX;
          posy=renderY;
          posz=renderZ;
          ang=renderAng;
          cursectnum=renderSect;
          ENG_setupVideo();
          keystatus[K_F10]=0;
     }
     if (qsetmode == 200L) {       // In 3D mode
          if (oqsetmode != 200L) {
               oqsetmode=qsetmode;
          }
          ENG_debugChain();
          if (keystatus[K_F10] != 0) {
               keystatus[K_F10]=0;
               gameModeFlag=1;
          }
          editinput();
          EFF_showMessages();
     }
     else {
          if (firstpass == 0) {
               firstpass=1;
               printmessage16("EDITOR ENHANCEMENTS BY LES BIRD (F1=HELP)");
               ENG_showEditorCommands();
          }
          if (oqsetmode == 200L) {
               keyfifoplc=keyfifoend;
          }
          oqsetmode=qsetmode;
          while (keyfifoplc != keyfifoend) {
               ch=keyfifo[keyfifoplc];
               stat=keyfifo[(keyfifoplc+1)&(KEYFIFOSIZ-1)];
               if (stat == 1) {
                    if (ch == K_CONTROL) {
                         altflag=1;
                    }
                    if (altflag) {
                         if (ch == K_R) {
                              ENG_resetMap();
                              altflag=0;
                         }
                         else if (ch == K_LEFTBRACKET) {
                              ENG_getSearchKeys();
                              ENG_getPrevSprite();
                              altflag=0;
                         }
                         else if (ch == K_RIGHTBRACKET) {
                              ENG_getSearchKeys();
                              ENG_getNextSprite();
                              altflag=0;
                         }
                         else if (ch == K_F) {
                              ENG_getSearchSprite();
                         }
                         else if (ch == K_D) {
                              ENG_getDistanceAngleInfo();
                         }
                         else if (ch >= K_1 && ch <= K_4) {
                              if (pointhighlight != -1) {
                                   if (pointhighlight >= 0x4000) {
                                        s=pointhighlight&0x3FFF;
                                        switch (ch) {
                                        case K_1:
                                             spritePtr[s]->lotag=SPR_DETAIL1;
                                             break;
                                        case K_2:
                                             spritePtr[s]->lotag=SPR_DETAIL2;
                                             break;
                                        case K_3:
                                             spritePtr[s]->lotag=SPR_DETAIL3;
                                             break;
                                        case K_4:
                                             spritePtr[s]->lotag=SPR_DETAIL4;
                                             break;
                                        }
                                   }
                              }
                         }
                         else if (ch == K_F2) {
                              ENG_showWallTags();
                         }
                         else if (ch == K_F3) {
                              ENG_showSpriteStatnums();
                         }
                    }
                    else {
                         if (ch == K_F1) {
                              ENG_showEditorCommands();
                         }
                         else if (ch == K_F2) {
                              ENG_showSectorTags();
                         }
                         else if (ch == K_F3) {
                              ENG_showSpriteTags();
                         }
                         else if (ch == K_F4) {
                              ENG_showCounts();
                         }
                         else if (ch == K_LEFTBRACKET) {
                              ENG_getPrevSprite();
                         }
                         else if (ch == K_RIGHTBRACKET) {
                              ENG_getNextSprite();
                         }
                    }
               }
               else {
                    if (ch == K_CONTROL) {
                         altflag=0;
                    }
               }
               keyfifoplc=(keyfifoplc+2)&(KEYFIFOSIZ-1);
          }
     }
}

void 
ExtCleanUp(void)
{
}

void 
ExtLoadMap(const char *mapname)
{
     strcpy(mapFileName,mapname);
     ENG_showCounts();
     fakeClock=totalclock;
     startx=startposx;
     starty=startposy;
     startz=startposz;
     starta=startang;
     starts=startsectnum;
}

void 
ExtSaveMap(const char *mapname)
{
}

const char *
ExtGetSectorCaption(short sectnum)
{
     short j;
     char appbuf[40],*ptr;
     sectortype *sec;

     sec=sectorPtr[sectnum];
     if ((sec->lotag | sec->hitag) == 0) {
          tempbuf[0] = 0;
     }
     else {
          if ((ptr=ENG_getSectorTagName(sec->lotag)) != NULL) {
               sprintf(tempbuf," %hu,%s ",(unsigned short)sec->hitag,ptr);
               if (EFF_operatableSector(sectnum)) {
                    if (EFF_getDoorSpeed(sectnum) == 255) {
                         EFF_setDoorSpeed(sectnum,DEFAULTDOORSPEED);
                    }
               }
               else if (EFF_isMovingSector(sectnum)) {
                    if (EFF_getDoorSpeed(sectnum) == 255) {
                         EFF_setDoorSpeed(sectnum,1);
                    }
               }
          }
          else {
               sprintf(tempbuf, " %hu,%hu ",
                       (unsigned short) sec->hitag,
                       (unsigned short) sec->lotag);
          }
          if (sec->extra < 0) {
               if (EFF_operatableSector(sectnum) || EFF_isMovingSector(sectnum)) {
                    EFF_setDoorLock(sectnum,0);
               }
          }
          if (EFF_isBonusSector(sectnum)) {
               j=EFF_getBonusSectorItem(sectnum);
               if ((ptr=ENG_getBonusItemName(j)) != NULL) {
                    sprintf(appbuf,"(%s)",ptr);
                    strcat(tempbuf,appbuf);
               }
          }
          else if (sec->hitag > 0) {
               if (EFF_operatableSector(sectnum) || EFF_isMovingSector(sectnum)) {
                    if ((j=EFF_getDoorLock(sectnum)) > 0) {
                         sprintf(appbuf, "(LOCK=%d) ",j);
                         strcat(tempbuf,appbuf);
                    }
                    else {
                         strcat(tempbuf, "(NO LOCK) ");
                    }
               }
          }
     }
     return (tempbuf);
}

const char *
ExtGetWallCaption(short wallnum)
{
     short j;
     char *ptr;
     walltype *wal;

     wal=wallPtr[wallnum];
     if ((wal->lotag | wal->hitag) == 0) {
          tempbuf[0] = 0;
     }
     else {
          switch (wal->lotag) {
          case WAL_SWITCHTAG:
               if ((j=EFF_getWallKey(wallnum)) > 0) {
                    sprintf(tempbuf," %hu,SWITCH (KEY=%hu) ",
                            (unsigned short)wal->hitag,
                            (unsigned short)j);
               }
               else {
                    sprintf(tempbuf," %hu,SWITCH (NOKEY) ",
                            (unsigned short)wal->hitag);
               }
               break;
          default:
               if ((ptr=ENG_getWallTagName(wal->lotag)) != NULL) {
                    sprintf(tempbuf," %hu,%s",(unsigned short)wal->hitag,ptr);
               }
               else {
                    sprintf(tempbuf," %hu,%hu ",(unsigned short)wal->hitag,
                            (unsigned short)wal->lotag);
               }
               break;
          }
     }
     return (tempbuf);
}

const char *
ExtGetSpriteCaption(short spritenum)
{
     short j;
     spritetype *spr;
     char locbuf[40],*nam,*ptr;

     spr=spritePtr[spritenum];
     sprintf(tempbuf," %d",spritenum);
     nam=ENG_picName(spr->picnum);
     if (ACT_isActor(spritenum)) {
          if (strlen(nam) > 0) {
               sprintf(locbuf,": ACTOR (%s) ",nam);
          }
          else {
               sprintf(locbuf,": ACTOR ");
          }
          strcat(tempbuf,locbuf);
     }
     else if (SND_isAmbientSound(spritenum)) {
          sprintf(locbuf,": %d,AMBIENT(%s)",SND_getSoundLoopCount(spritenum)+1,
                  SND_getSoundName(SND_getSoundNum(spritePtr[spritenum])));
          strcat(tempbuf,locbuf);
     }
     else if (SND_isSectorSound(spritenum)) {
          sprintf(locbuf,": %d,SOUNDFX(%s)",SND_getSoundLoopCount(spritenum)+1,
                  SND_getSoundName(SND_getSoundNum(spritePtr[spritenum])));
          strcat(tempbuf,locbuf);
     }
     else if (spr->lotag > 0 || spr->hitag > 0) {
          switch (spr->lotag) {
          case SPR_TELEPORTTAG:
               sprintf(locbuf,": %d,%s ",EFF_getWarpSpriteTag(spritenum),
                       ENG_getSpriteTagName(spr->lotag));
               strcat(tempbuf,locbuf);
               break;
          case SPR_SWITCHTAG:
               if ((j=EFF_getSpriteKey(spritenum)) <= 0) {
                    sprintf(locbuf,": %hu,SWITCH (NO KEY) ",
                            (unsigned short) spr->hitag);
                    strcat(tempbuf,locbuf);
               }
               else {
                    sprintf(locbuf,": %hu,SWITCH (KEY=%hu) ",
                            (unsigned short) spr->hitag,
                            (unsigned short) j);
                    strcat(tempbuf,locbuf);
               }
               break;
          default:
               if (spr->picnum == enemyPic[0]) {
                    sprintf(locbuf,": PSTART ");
               }
               else if ((ptr=ENG_getSpriteTagName(spr->lotag)) != NULL) {
                    sprintf(locbuf,": %d,%s ",spr->hitag,ptr);
               }
               else if (strlen(nam) > 0) {
                    sprintf(locbuf,": %hu,%hu (%s) ",
                            (unsigned short) spr->hitag,
                            (unsigned short) spr->lotag,
                            nam);
               }
               else {
                    sprintf(locbuf,": %hu,%hu ",
                            (unsigned short) spr->hitag,
                            (unsigned short) spr->lotag);
               }
               strcat(tempbuf,locbuf);
               break;
          }
     }
     else {
          if (strlen(nam) > 0) {
               sprintf(locbuf,": %s ",nam);
               strcat(tempbuf,locbuf);
          }
          else if (EFF_testSpriteFlag(spritenum,EXTFLAGS_HEATSOURCE)) {
               strcat(tempbuf,": (HEAT) ");
          }
          else {
               strcat(tempbuf," ");
          }
     }
     return(tempbuf);
}

//printext16 parameters:
//printext16(long xpos, long ypos, short col, short backcol,
//           char name[82], char fontsize)
//  xpos 0-639   (top left)
//  ypos 0-479   (top left)
//  col 0-15
//  backcol 0-15, -1 is transparent background
//  name
//  fontsize 0=8*8, 1=3*5

//drawline16 parameters:
// drawline16(long x1, long y1, long x2, long y2, char col)
//  x1, x2  0-639
//  y1, y2  0-143  (status bar is 144 high, origin is top-left of STATUS BAR)
//  col     0-15

void 
ExtShowSectorData(short sectnum)
{                                  // F5
     short i,endwall,startwall;
     sectortype *sec;
     char *ptr;

     sec=sectorPtr[sectnum];
     if (qsetmode == 200) {        // In 3D mode
     }
     else {
          clearmidstatbar16();
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"SECTOR %d",sectnum);
          if (EFF_operatableSector(sectnum) || EFF_isMovingSector(sectnum)) {
               ENG_printHelpText16(14,1,"SPEED.............. %d",
                                   EFF_getDoorSpeed(sectnum));
               if (EFF_isBonusSector(sectnum)) {
                    ENG_printHelpText16(14,1,"BONUS VALUE........ %d",
                                        EFF_getBonusSectorValue(sectnum));
               }
               else {
                    ENG_printHelpText16(14,1,"LOCK............... %d",
                                        EFF_getDoorLock(sectnum));
               }
          }
          else {
               ENG_printHelpText16(14,1,"EXTRA.............. %d",sec->extra);
          }
          ENG_setMidStatbarPos(208,TOPSTATBARY);
          ENG_printHelpText16(11,0,"EFFECTS");
          ptr=ENG_getSectorTagName(sec->lotag);
          if (ptr != NULL) {
               ENG_printHelpText16(14,1,"%s",ptr);
          }
          startwall=sec->wallptr;
          endwall=startwall+sec->wallnum;
          for (i=startwall ; i < endwall ; i++) {
               if (wallPtr[i]->lotag > 0) {
                    ptr=ENG_getWallTagName(wallPtr[i]->lotag);
                    if (ptr != NULL) {
                         ENG_printHelpText16(14,1,"%s",ptr);
                    }
               }
          }
     }
}

void 
ExtShowWallData(short wallnum)
{                                  // F6
     walltype *wal;

     wal=wallPtr[wallnum];
     if (qsetmode == 200) {        // In 3D mode
     }
     else {
          clearmidstatbar16();
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          ENG_printHelpText16(11,0,"WALL %d",wallnum);
          ENG_printHelpText16(14,1,"SWITCHSTATUS....... %d",
                              EFF_getWallSwitchStatus(wallnum));
          if (wal->lotag == WAL_SWITCHTAG) {
               ENG_printHelpText16(14,1,"KEY................ %d",
                                   EFF_getWallKey(wallnum));
          }
          else {
               ENG_printHelpText16(14,1,"EXTRA.............. %d",wal->extra);
          }
     }
}

void 
ExtShowSpriteData(short spritenum)
{                                  // F6
     int  j;
     spritetype *spr;
     char *ptr;

     spr=spritePtr[spritenum];
     if (qsetmode == 200) {        // In 3D mode
     }
     else {
          clearmidstatbar16();
          ENG_setMidStatbarPos(8,TOPSTATBARY);
          if (ACT_isActor(spritenum)) {
               ptr=ENG_getStatusName(spr->statnum);
               ENG_printHelpText16(11,0,"SPRITE %d MODE=%s",spritenum,
                                   (ptr != NULL) ? ptr : "INVALID");
          }
          else {
               j=EFF_testSpriteFlag(spritenum,EXTFLAGS_HEATSOURCE);
               ENG_printHelpText16(11,0,"SPRITE %d %s %s",spritenum,
                                   ENG_picName(spr->picnum),
                                   j != 0 ? "(HEAT SOURCE)" : "");
          }
          if (ACT_isActor(spritenum)) {
               ENG_printHelpText16(14,1,"WALK SPEED......... %d",
                                   ACT_getWalkSpeed(spritenum));
               ENG_printHelpText16(14,1,"CHASE SPEED........ %d",
                                   ACT_getChaseSpeed(spritenum));
               ENG_printHelpText16(14,1,"AGGRESSIVENESS..... %d",
                                   ACT_getAggression(spritenum));
               ENG_printHelpText16(14,1,"WEAPON............. %d",
                                   ACT_getWeapon(spritenum));
               ENG_printHelpText16(14,1,"HEALTH............. %d",
                                   ACT_getHealth(spritenum));
          }
          else if (SND_isAmbientSound(spritenum)) {
               j=SND_getSoundNum(spritePtr[spritenum]);
               ENG_printHelpText16(14,1,"SOUND TO PLAY...... %s",
                                   SND_getSoundName(j));
               ENG_printHelpText16(14,1,"SOUND TRIGGER...... DISTANCE");
               ENG_printHelpText16(14,1,"TIMES TO LOOP...... INFINITE");
               return;
          }
          else if (SND_isSectorSound(spritenum)) {
               j=SND_getSoundNum(spritePtr[spritenum]);
               ENG_printHelpText16(14,1,"SOUND TO PLAY...... %s",
                                   SND_getSoundName(j));
               j=SND_getSoundPlaytime(spritenum);
               ENG_printHelpText16(14,1,"SOUND TRIGGER...... %s",
                                   SND_getSoundPlaytimeName(j));
               ENG_printHelpText16(14,1,"TIMES TO LOOP...... %d",
                                   SND_getSoundLoopCount(spritenum)+1);
               return;
          }
          switch (spr->lotag) {
          case SPR_TELEPORTTAG:
               ENG_printHelpText16(14,1,"WARP TAG........... %d",
                                   EFF_getWarpSpriteTag(spritenum));
               break;
          case SPR_SWITCHTAG:
               ENG_printHelpText16(14,1,"SECTOR LINK........ %d",spr->hitag);
               ENG_printHelpText16(14,1,"LOCK............... %d",
                                   EFF_getSpriteKey(spritenum));
               break;
          }
          ENG_printHelpText16(14,1,"FLAGS.............. %X",
                              EFF_getSpriteFlag(spritenum));
     }
}

void 
ExtEditSectorData(short sectnum)
{                                  // F7
     short nickdata;
     sectortype *sec;

     sec=sectorPtr[sectnum];
     if (qsetmode == 200) {        // In 3D mode
     }
     else {                        // In 2D mode
          ExtShowSectorData(sectnum);
          if (EFF_operatableSector(sectnum) || EFF_isMovingSector(sectnum)) {
               sprintf(tempbuf,"Speed (1=slow - 127=fast): ");
               nickdata=EFF_getDoorSpeed(sectnum);
               nickdata=getnumber16(tempbuf,nickdata,128L);
               EFF_setDoorSpeed(sectnum,nickdata);
               if (EFF_isBonusSector(sectnum)) {
                    EFF_setDoorLock(sectnum,0);
                    ENG_showBonusItems();
                    sprintf(tempbuf,"Bonus item for this sector: ");
                    nickdata=EFF_getBonusSectorItem(sectnum);
                    nickdata=getnumber16(tempbuf,nickdata,127L);
                    EFF_setBonusSectorItem(sectnum,nickdata);
                    ExtShowSectorData(sectnum);
                    sprintf(tempbuf,"Bonus value: ");
                    nickdata=EFF_getBonusSectorValue(sectnum);
                    nickdata=getnumber16(tempbuf,nickdata,65536L);
                    EFF_setBonusSectorValue(sectnum,nickdata);
               }
               else {
                    sprintf(tempbuf,"Key required to operate (0-127): ");
                    nickdata=EFF_getDoorLock(sectnum);
                    nickdata=getnumber16(tempbuf, nickdata,128L);
                    EFF_setDoorLock(sectnum,nickdata);
               }
          }
          else {
               sprintf(tempbuf, "Sector (%d) extra variable: ", sectnum);
               nickdata=sec->extra;
               nickdata=getnumber16(tempbuf, nickdata, 65536L);
               sec->extra=nickdata;
          }
          printmessage16("");
          ExtShowSectorData(sectnum);
     }
}

void 
ExtEditWallData(short wallnum)
{                                  // F8
     short nickdata;
     walltype *wal;

     wal=wallPtr[wallnum];
     if (qsetmode == 200) {        // In 3D mode
     }
     else {
          ExtShowWallData(wallnum);
          switch (wal->lotag) {
          case WAL_SWITCHTAG:
               sprintf(tempbuf, "Wall (%d) key: ", wallnum);
               nickdata=EFF_getWallKey(wallnum);
               nickdata=getnumber16(tempbuf, nickdata, 65536L);
               EFF_setWallKey(wallnum,nickdata);
               break;
          default:
               sprintf(tempbuf, "Wall (%d) extra value: ",wallnum);
               wal->extra=getnumber16(tempbuf,wal->extra,65536L);
               break;
          }
          printmessage16("");
          ExtShowWallData(wallnum);
     }
}

void 
ExtEditSpriteData(short spritenum)
{                                  // F8
     short nickdata;
     spritetype *spr;

     spr=spritePtr[spritenum];
     if (qsetmode == 200) {        // In 3D mode
     }
     else {
          ExtShowSpriteData(spritenum);
          if (ACT_isActorPic(spritenum)) {
               spr->cstat|=0x101;
               ENG_showActorTypes();
               switch (spr->statnum) {
               case STAT_PATROL:
                    nickdata=ACT_PATROL;
                    break;
               case STAT_WANDER:
                    nickdata=ACT_WANDER;
                    break;
               case STAT_AMBUSH:
                    nickdata=ACT_AMBUSH;
                    break;
               case STAT_GUARD:
                    nickdata=ACT_GUARD;
                    break;
               case STAT_ATTACK:
                    nickdata=ACT_ATTACK;
                    break;
               default:
                    nickdata=ACT_AMBUSH;
                    break;
               }
               sprintf(tempbuf,"Sprite (%d) actor mode: ",spritenum);
               nickdata = getnumber16(tempbuf, nickdata, 65536L);
               switch (nickdata) {
               case ACT_PATROL:
                    changespritestat(spritenum,STAT_PATROL);
                    break;
               case ACT_WANDER:
                    changespritestat(spritenum,STAT_WANDER);
                    break;
               case ACT_AMBUSH:
                    changespritestat(spritenum,STAT_AMBUSH);
                    break;
               case ACT_GUARD:
                    changespritestat(spritenum,STAT_GUARD);
                    break;
               case ACT_ATTACK:
                    changespritestat(spritenum,STAT_ATTACK);
                    break;
               default:
                    changespritestat(spritenum,STAT_GUARD);
                    break;
               }
               ExtShowSpriteData(spritenum);
               sprintf(tempbuf,"Walk speed (0=FAST,7=STOP): ");
               nickdata=ACT_getWalkSpeed(spritenum);
               if (nickdata == 0) {
                    nickdata=4;
               }
               nickdata=getnumber16(tempbuf,nickdata,8L);
               ACT_setWalkSpeed(spritenum,nickdata);
               sprintf(tempbuf,"Chase speed (0=FAST,7=STOP): ");
               nickdata=ACT_getChaseSpeed(spritenum);
               if (nickdata == 0) {
                    nickdata=2;
               }
               nickdata=getnumber16(tempbuf,nickdata,8L);
               ACT_setChaseSpeed(spritenum,nickdata);
               sprintf(tempbuf,"Aggressiveness (0=NONE,7=VERY): ");
               nickdata=ACT_getAggression(spritenum);
               if (nickdata == 0) {
                    nickdata=2;
               }
               nickdata=getnumber16(tempbuf,nickdata,8L);
               ACT_setAggression(spritenum,nickdata);
               ENG_showWeaponHelp();
               sprintf(tempbuf,"Weapon (0-%d): ",MAXWEAPONS);
               nickdata=ACT_getWeapon(spritenum);
               nickdata=getnumber16(tempbuf,nickdata,MAXWEAPONS+1);
               ACT_setWeapon(spritenum,nickdata);
               ExtShowSpriteData(spritenum);
               sprintf(tempbuf,"Health (1-32767): ");
               if ((nickdata=ACT_getHealth(spritenum)) <= 0) {
                    ACT_setHealth(spritenum,1000);
                    nickdata=ACT_getHealth(spritenum);
               }
               nickdata=getnumber16(tempbuf,nickdata,32768L);
               ACT_setHealth(spritenum,nickdata);
               nickdata=1;
          }
          else if (SND_isAmbientSound(spritenum)) {
               SND_showSoundList();
               sprintf(tempbuf,"Ambient sound to play: ");
               nickdata=SND_getSoundNum(spritePtr[spritenum]);
               nickdata=getnumber16(tempbuf,nickdata,65536L);
               SND_setSoundNum(spritePtr[spritenum],nickdata);
               printmessage16("");
               ExtShowSpriteData(spritenum);
               return;
          }
          else if (SND_isSectorSound(spritenum)) {
               SND_showSoundList();
               sprintf(tempbuf,"Sound to play: ");
               nickdata=SND_getSoundNum(spritePtr[spritenum]);
               nickdata=getnumber16(tempbuf,nickdata,65536L);
               SND_setSoundNum(spritePtr[spritenum],nickdata);
               SND_showSoundPlaytimes();
               sprintf(tempbuf,"Sound trigger: ");
               nickdata=SND_getSoundPlaytime(spritenum);
               nickdata=getnumber16(tempbuf,nickdata,65536L);
               SND_setSoundPlaytime(spritenum,nickdata);
               ExtShowSpriteData(spritenum);
               sprintf(tempbuf,"Loop how many times: ");
               nickdata=SND_getSoundLoopCount(spritenum)+1;
               nickdata=getnumber16(tempbuf,nickdata,65536L);
               SND_setSoundLoopCount(spritenum,nickdata-1);
               printmessage16("");
               ExtShowSpriteData(spritenum);
               return;
          }
          switch (spr->lotag) {
          case SPR_TELEPORTTAG:
               sprintf(tempbuf,"Warp tag for this sprite (1-65535): ");
               nickdata=EFF_getWarpSpriteTag(spritenum);
               nickdata=getnumber16(tempbuf,nickdata,65536L);
               EFF_setWarpSpriteTag(spritenum,nickdata);
               nickdata=0;
               break;
          case SPR_SWITCHTAG:
               sprintf(tempbuf,"Sprite (%d) key: ",spritenum);
               nickdata=EFF_getSpriteKey(spritenum);
               nickdata=getnumber16(tempbuf,nickdata,65536L);
               EFF_setSpriteKey(spritenum,nickdata);
               nickdata=0;
               break;
          default:
               if (!ACT_isActorPic(spritenum)) {
                    if (WEP_isArmedDevice(spritenum)) {
                         ENG_showWeaponHelp();
                         sprintf(tempbuf,"Associate sprite (%d) with weapon: ",
                                 spritenum);
                         spr->extra=getnumber16(tempbuf,spr->extra,MAXWEAPONS);
                         nickdata=0;
                    }
                    else {
                         sprintf(tempbuf,"Sprite (%d) extra variable: ",
                                 spritenum);
                         spr->extra=getnumber16(tempbuf,spr->extra,65536L);
                         nickdata=0;
                    }
               }
               break;
          }
          if (nickdata || ACT_isActorPic(spritenum)) {
               spr->cstat|=0x100;
               EFF_setSpriteFlag(spritenum,EXTFLAGS_HEATSOURCE);
          }
          else {
               sprintf(tempbuf,"Is sprite (%d) a heat source: ",spritenum);
               nickdata=getboolean16(tempbuf,nickdata);
               if (nickdata) {
                    EFF_setSpriteFlag(spritenum,EXTFLAGS_HEATSOURCE);
               }
               else {
                    EFF_resetSpriteFlag(spritenum,EXTFLAGS_HEATSOURCE);
               }
          }
          printmessage16("");
          ExtShowSpriteData(spritenum);
     }
}

void
ENG_getTextureAssignments(short *floors,short *ceils,short *walls,short *sprites)
{
     sprintf(tempbuf,"Texture for floors: ");
     *floors=getnumber16(tempbuf,*floors,65536L);
     sprintf(tempbuf,"Texture for ceilings: ");
     *ceils=getnumber16(tempbuf,*ceils,65536L);
     sprintf(tempbuf,"Texture for walls: ");
     *walls=getnumber16(tempbuf,*walls,65536L);
     sprintf(tempbuf,"Texture for sprites: ");
     *sprites=getnumber16(tempbuf,*sprites,65536L);
}

int
getboolean16(char *stg,short bool)
{
     char buffer[80];
     short truefalse;

     truefalse=bool;
     while (keystatus[K_ENTER] == 0 && keystatus[K_ESC] == 0) {
          if (!truefalse) {
               sprintf(buffer,"%sNO_ ",stg);
          }
          else {
               sprintf(buffer,"%sYES_ ",stg);
          }
          printmessage16(buffer);
          if (keystatus[K_Y]) {
               keystatus[K_Y]=0;
               truefalse=1;
               break;
          }
          else if (keystatus[K_N]) {
               keystatus[K_N]=0;
               truefalse=0;
               break;
          }
     }
     if (keystatus[K_ESC]) {
          truefalse=0;
     }
     else {
          keystatus[K_ENTER]=0;
          keystatus[K_Y]=0;
          keystatus[K_N]=0;
     }
     return(truefalse);
}

// Just thought you might want my getnumber16 code
/*
getnumber16(char namestart[80], short num, long maxnumber)
{
     char buffer[80];
     long j, k, n, danum, oldnum;

     danum = (long)num;
     oldnum = danum;
     while ((keystatus[0x1c] != 2) && (keystatus[0x1] == 0))  //Enter, ESC
     {
          sprintf(&buffer,"%s%ld_ ",namestart,danum);
          printmessage16(buffer);

          for(j=2;j<=11;j++)                //Scan numbers 0-9
               if (keystatus[j] > 0)
               {
                    keystatus[j] = 0;
                    k = j-1;
                    if (k == 10) k = 0;
                    n = (danum*10)+k;
                    if (n < maxnumber) danum = n;
               }
          if (keystatus[0xe] > 0)    // backspace
          {
               danum /= 10;
               keystatus[0xe] = 0;
          }
          if (keystatus[0x1c] == 1)   //L. enter
          {
               oldnum = danum;
               keystatus[0x1c] = 2;
               asksave = 1;
          }
     }
     keystatus[0x1c] = 0;
     keystatus[0x1] = 0;
     return((short)oldnum);
}
*/

void
ENG_saveGame(FILE *fp)
{
}

void
ENG_loadGame(FILE *fp)
{
}

void
ENG_drawOverheadMap(long cposx,long cposy,long czoom,short cang)
{
     short col,daang,endwall,i,j,k,l,startwall,tilenum;
     long cosang,sinang,
          dax,day,
          ox,oy,
          sprx,spry,
          x1,y1,x2,y2,x3,y3,x4,y4,
          xvect,yvect,xvect2,yvect2,
          xspan,yspan,
          xrepeat,yrepeat,
          xoff,yoff,
          z1,z2;
     walltype *wal,*wal2;
     spritetype *spr;
     sectortype *secti,*sectw;

     if (player[currentView]->viewMode == 1) {
          drawmapview(cposx,cposy,czoom,cang);
     }
     xvect=sintable[(-cang)&2047]*czoom;
     yvect=sintable[(1536-cang)&2047]*czoom;
     xvect2=mulscale16(xvect,yxaspect);
     yvect2=mulscale16(yvect,yxaspect);
// Draw red lines
     for (i=0 ; i < numsectors ; i++) {
          secti=sectorPtr[i];
          startwall=secti->wallptr;
          endwall=secti->wallptr+secti->wallnum;
          z1=secti->ceilingz;
          z2=secti->floorz;
          for (j=startwall,wal=wallPtr[startwall] ; j < endwall ; j++,wal++) {
               k=wal->nextwall;
               if (k < 0) {
                    continue;
               }
               if ((show2dwall[j>>3]&(1<<(j&7))) == 0) {
                    continue;
               }
               if ((k > j) && ((show2dwall[k>>3]&(1<<(k&7))) > 0)) {
                    continue;
               }
               sectw=sectorPtr[wal->nextsector];
               if (sectw->ceilingz == z1) {
                    if (sectw->floorz == z2) {
                         if ((wal->cstat&0x30) == 0 ||
                             (wallPtr[wal->nextwall]->cstat&0x30) == 0) {
                              continue;
                         }
                    }
               }
               if (player[currentView]->viewMode > 1) {
                    if (secti->floorz != secti->ceilingz) {
                         if (sectw->floorz != sectw->ceilingz) {
                              if ((wal->cstat&0x30) == 0 ||
                                  (wallPtr[wal->nextwall]->cstat&0x30) == 0) {
                                   if (secti->floorz == sectw->floorz) {
                                        continue;
                                   }
                              }
                         }
                    }
                    if (secti->floorpicnum != sectw->floorpicnum) {
                         continue;
                    }
                    if (secti->floorshade != sectw->floorshade) {
                         continue;
                    }
               }
               ox=wal->x-cposx;
               oy=wal->y-cposy;
               x1=dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
               y1=dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
               wal2=wallPtr[wal->point2];
               ox=wal2->x-cposx;
               oy=wal2->y-cposy;
               x2=dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
               y2=dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
               drawline256(x1,y1,x2,y2,GREENBASE+20);
          }
     }
// Draw sprites
     k=GAM_getViewSprite();
     for (i=0 ; i < numsectors ; i++) {
          for (j=headspritesect[i] ; j >= 0 ; j = nextspritesect[j]) {
               if ((show2dsprite[j>>3]&(1<<(j&7))) > 0) {
                    spr=spritePtr[j];
                    if (spr->cstat&0x8000) {
                         continue;
                    }
                    col=REDBASE+16;
                    if (spr->cstat&1) {
                         col=REDBASE+20;
                    }
                    if (j == k) {
                         col=GRAYBASE+23;
                    }
                    sprx=spr->x;
                    spry=spr->y;
                    switch (spr->cstat&(16+32)) {
                    case 0:
                         ox=sprx-cposx;
                         oy=spry-cposy;
                         x1=dmulscale16(ox,xvect,-oy,yvect);
                         y1=dmulscale16(oy,xvect2,ox,yvect2);
                         if (player[currentView]->viewMode > 1) {
                              ox=(sintable[(spr->ang+512)&2047]>>7);
                              oy=(sintable[(spr->ang)&2047]>>7);
                              x2=dmulscale16(ox,xvect,-oy,yvect);
                              y2=dmulscale16(oy,xvect,ox,yvect);
                              if (j == k) {
                                   x2=0L;
                                   y2=-(czoom<<5);
                              }
                              x3=mulscale16(x2,yxaspect);
                              y3=mulscale16(y2,yxaspect);
                              drawline256(x1-x2+(xdim<<11),y1-y3+(ydim<<11),
                                          x1+x2+(xdim<<11),y1+y3+(ydim<<11),
                                          col);
                              drawline256(x1-y2+(xdim<<11),y1+x3+(ydim<<11),
                                          x1+x2+(xdim<<11),y1+y3+(ydim<<11),
                                          col);
                              drawline256(x1+y2+(xdim<<11),y1-x3+(ydim<<11),
                                          x1+x2+(xdim<<11),y1+y3+(ydim<<11),
                                          col);
                         }
                         else {
                              if ((gotsector[i>>3]&(1<<(i&7))) > 0 &&
                                   czoom > 96) {
                                   daang=(spr->ang-cang)&2047;
                                   if (j == k) {
                                        x1=0;
                                        y1=0;
                                        daang=0;
                                   }
                                   rotatesprite((x1<<4)+(xdim<<15),
                                                (y1<<4)+(ydim<<15),
                                                mulscale16(czoom*spr->yrepeat,
                                                           yxaspect),
                                                daang,spr->picnum,spr->shade,
                                                spr->pal,
                                                (spr->cstat&2)>>1,
                                                windowx1,windowy1,
                                                windowx2,windowy2);
                              }
                         }
                         break;
                    case 16:
                         x1=sprx;
                         y1=spry;
                         tilenum=spr->picnum;
                         xoff=(long)((signed char)((picanm[tilenum]>>8)&255))+
                                     ((long)spr->xoffset);
                         if ((spr->cstat&4) > 0) {
                              xoff=-xoff;
                         }
                         k=spr->ang;
                         l=spr->xrepeat;
                         dax=sintable[k&2047]*l;
                         day=sintable[(k+1536)&2047]*l;
                         l=tilesizx[tilenum];
                         k=(l>>1)+xoff;
                         x1-=mulscale16(dax,k);
                         x2=x1+mulscale16(dax,l);
                         y1-=mulscale16(day,k);
                         y2=y1+mulscale16(day,l);
                         ox=x1-cposx;
                         oy=y1-cposy;
                         x1=dmulscale16(ox,xvect,-oy,yvect);
                         y1=dmulscale16(oy,xvect2,ox,yvect2);
                         ox=x2-cposx;
                         oy=y2-cposy;
                         x2=dmulscale16(ox,xvect,-oy,yvect);
                         y2=dmulscale16(oy,xvect2,ox,yvect2);
                         drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                     x2+(xdim<<11),y2+(ydim<<11),col);
                         break;
                    case 32:
                         if (player[currentView]->viewMode > 1) {
                              tilenum=spr->picnum;
                              xoff=(long)((signed char)((picanm[tilenum]>>8)
                                          &255))+((long)spr->xoffset);
                              yoff=(long)((signed char)((picanm[tilenum]>>16)
                                          &255))+((long)spr->yoffset);
                              if ((spr->cstat&4) > 0) {
                                   xoff=-xoff;
                              }
                              if ((spr->cstat&8) > 0) {
                                   yoff=-yoff;
                              }
                              k=spr->ang;
                              cosang=sintable[(k+512)&2047];
                              sinang=sintable[k];
                              xspan=tilesizx[tilenum];
                              xrepeat=spr->xrepeat;
                              yspan=tilesizy[tilenum];
                              yrepeat=spr->yrepeat;
                              dax=((xspan>>1)+xoff)*xrepeat;
                              day=((yspan>>1)+yoff)*yrepeat;
                              x1=sprx+dmulscale16(sinang,dax,cosang,day);
                              y1=spry+dmulscale16(sinang,day,-cosang,dax);
                              l=xspan*xrepeat;
                              x2=x1-mulscale16(sinang,l);
                              y2=y1+mulscale16(cosang,l);
                              l=yspan*yrepeat;
                              k=-mulscale16(cosang,l);
                              x3=x2+k;
                              x4=x1+k;
                              k=-mulscale16(sinang,l);
                              y3=y2+k;
                              y4=y1+k;
                              ox=x1-cposx;
                              oy=y1-cposy;
                              x1=dmulscale16(ox,xvect,-oy,yvect);
                              y1=dmulscale16(oy,xvect2,ox,yvect2);
                              ox=x2-cposx;
                              oy=y2-cposy;
                              x2=dmulscale16(ox,xvect,-oy,yvect);
                              y2=dmulscale16(oy,xvect2,ox,yvect2);
                              ox=x3-cposx;
                              oy=y3-cposy;
                              x3=dmulscale16(ox,xvect,-oy,yvect);
                              y3=dmulscale16(oy,xvect2,ox,yvect2);
                              ox=x4-cposx;
                              oy=y4-cposy;
                              x4=dmulscale16(ox,xvect,-oy,yvect);
                              y4=dmulscale16(oy,xvect2,ox,yvect2);
                              drawline256(x1+(xdim<<11),y1+(ydim<<11),
                                          x2+(xdim<<11),y2+(ydim<<11),col);
                              drawline256(x2+(xdim<<11),y2+(ydim<<11),
                                          x3+(xdim<<11),y3+(ydim<<11),col);
                              drawline256(x3+(xdim<<11),y3+(ydim<<11),
                                          x4+(xdim<<11),y4+(ydim<<11),col);
                              drawline256(x4+(xdim<<11),y4+(ydim<<11),
                                          x1+(xdim<<11),y1+(ydim<<11),col);
                         }
                         break;
                    }
               }
          }
     }
// Draw white lines
     for (i = 0 ; i < numsectors ; i++) {
          startwall=sectorPtr[i]->wallptr;
          endwall=sectorPtr[i]->wallptr+sectorPtr[i]->wallnum;
          k=-1;
          for (j=startwall,wal=wallPtr[startwall] ; j < endwall ; j++,wal++) {
               if (wal->nextwall >= 0) {
                    continue;
               }
               if ((show2dwall[j>>3]&(1<<(j&7))) == 0) {
                    continue;
               }
               if (tilesizx[wal->picnum] == 0) {
                    continue;
               }
               if (tilesizy[wal->picnum] == 0) {
                    continue;
               }
               if (j == k) {
                    x1=x2;
                    y1=y2;
               }
               else {
                    ox=wal->x-cposx;
                    oy=wal->y-cposy;
                    x1=dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
                    y1=dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
               }
               k=wal->point2;
               wal2=wallPtr[k];
               ox=wal2->x-cposx;
               oy=wal2->y-cposy;
               x2=dmulscale16(ox,xvect,-oy,yvect)+(xdim<<11);
               y2=dmulscale16(oy,xvect2,ox,yvect2)+(ydim<<11);
               if (qsetmode == 200L) {
                    drawline256(x1,y1,x2,y2,GRAYBASE+23);
               }
               else {
                    drawline16(x1,y1,x2,y2,15);
               }
          }
     }
}

