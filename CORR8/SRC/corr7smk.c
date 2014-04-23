/***************************************************************************
 *   CORR7SMK.C - Corridor 7 SMACKER routines
 *
 *                                                     06/07/96 Les Bird
 ***************************************************************************/

#include "icorp.h"
#include "smack.h"
#include "svga.h"
#include <memcheck.h>

#define   MAXRADBUFFS         64
#define   SMACKTILE           4093

#pragma pack(1);

char smackPalette[768];

Smack *smk;

struct radbuftype {
     long cache_ptr;
     long cache_length;
     char cache_lock;
};

struct radbuftype radbuf[MAXRADBUFFS];

extern
HANDLE hDIGIDriver;

RCFUNC
void PTR4 *RADLINK
radmalloc(u32 numbytes)
{
     int  i;

     for (i = 0; i < MAXRADBUFFS; i++) {
          if (radbuf[i].cache_ptr == 0L) {
               break;
          }
     }
     if (i == MAXRADBUFFS) {
          crash("no more radbuff pointers");
     }
     radbuf[i].cache_lock = 200;
     radbuf[i].cache_length = numbytes;
     allocache(&(radbuf[i].cache_ptr), radbuf[i].cache_length,
               &(radbuf[i].cache_lock));
     if (radbuf[i].cache_ptr == 0L) {
          crash("radmalloc failed");
     }
     return((void PTR4 *) radbuf[i].cache_ptr);
}

RCFUNC
void RADLINK
radfree(void PTR4 * ptr)
{
     int  i;

     for (i = 0; i < MAXRADBUFFS; i++) {
          if (radbuf[i].cache_ptr == (long) ptr) {
               radbuf[i].cache_lock = 1;
               break;
          }
     }
}

void
SMK_init(void)
{
#if 0
     if (hDIGIDriver != -1) {
          SmackSoundUseSOS4(hDIGIDriver,TMR_getSecondTics(1));
     }
     else {
          SmackSoundUseSOS4(0,0);
     }
#endif
}

void
SMK_loadPalette(char *pal)
{
     memmove(smackPalette,pal,sizeof(smackPalette));
}

void
SMK_setPalette(void)
{
     GFX_setPalette(smackPalette);
}

void
SMK_playFlic(char *path)
{
#if 0
     int  frames=0,i;

     if (access(path,F_OK) != 0) {
          return;
     }
     if (!noSoundFlag) {
          smk=SmackOpen(path,SMACKTRACKS,SMACKAUTOEXTRA);
     }
     else {
          smk=SmackOpen(path,0,SMACKAUTOEXTRA);
     }
     if (smk) {
          GFX_fadeOut(255);        // make sure game palette is faded
          walock[SMACKTILE]=200;
          allocache(&waloff[SMACKTILE],320L*200L,&walock[SMACKTILE]);
          tilesizx[SMACKTILE]=200;
          tilesizy[SMACKTILE]=320;
          SmackToBuffer(smk,0L,0L,320L,200L,(void *)waloff[SMACKTILE],0);
          keystatus[1]=0;
          keystatus[28]=0;
          keystatus[57]=0;
          while (1) {
               if (smk->NewPalette) {
                    SMK_loadPalette(smk->Palette);
               }
               SmackDoFrame(smk);
               SmackNextFrame(smk);
               rotatesprite(320L<<15,200L<<15,65536L,512,SMACKTILE,0,0,2+4+64,
                            0L,0L,xdim-1L,ydim-1L);
               nextpage();
               SMK_setPalette();
               while (SmackWait(smk)) {
                    if (keystatus[1] || keystatus[57] || keystatus[28]) {
                         goto done;
                    }
               }
               if (keystatus[1] || keystatus[57] || keystatus[28]) {
                    goto done;
               }
               frames++;
               if (frames == smk->Frames) {
                    goto done;
               }
          }
done:
          walock[SMACKTILE]=1;
          SmackClose(smk);
          for (i = 0 ; i < MAXRADBUFFS ; i++) {
               radbuf[i].cache_lock=1;
          }
     }
#endif
}

