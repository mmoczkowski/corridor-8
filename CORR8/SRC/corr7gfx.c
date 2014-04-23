/***************************************************************************
 *   CORR7GFX.C - MegaGraph Library interface for Corridor 7
 *
 *                                                     04/04/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   GFXMODULE_ENABLED

#define   NUMREDSHIFTS        4
#define   REDSTEPS            8

#define   NUMGREENSHIFTS      4
#define   GREENSTEPS          16

#define   NUMBLUESHIFTS       4
#define   BLUESTEPS           8

//static
int  fadeInFlag,
     fadeOutFlag;

static
int  fadeIntensity=255,
     fadeSpeed,
     redShift,
     greenShift,
     blueShift;

long animPalClock;

static
char fullpal[768];

static
char bluePal[NUMBLUESHIFTS][768],
     greenPal[NUMGREENSHIFTS][768],
     redPal[NUMREDSHIFTS][768];

extern void near asmwaitvrt(int p);
#pragma aux asmwaitvrt "*_" \
     parm caller [ecx] \
     modify [eax edx];

extern void near asmsetpalette(char *p);
#pragma aux asmsetpalette "*_" \
     parm caller [esi] \
     modify [eax ecx edx];

void
GFX_setPalette(char *pal)
{
#ifdef GFXMODULE_ENABLED
     asmwaitvrt(1);
     asmsetpalette(pal);
#endif
}

void
GFX_initPalShifts(void)
{
#ifdef GFXMODULE_ENABLED
     int  i,j,r,g,b,delta;

     for (i=0 ; i < NUMREDSHIFTS ; i++) {
          for (j=0 ; j < 256 ; j++) {
               r=j*3;
               g=r+1;
               b=r+2;
               delta=64-fullpal[r];
               redPal[i][r]=fullpal[r]+delta*(i+1)/REDSTEPS;
               delta=-fullpal[g];
               redPal[i][g]=fullpal[g]+delta*(i+1)/REDSTEPS;
               delta=-fullpal[b];
               redPal[i][b]=fullpal[b]+delta*(i+1)/REDSTEPS;
          }
     }
     for (i=0 ; i < NUMGREENSHIFTS ; i++) {
          for (j=0 ; j < 256 ; j++) {
               r=j*3;
               g=r+1;
               b=r+2;
               delta=64-fullpal[g];
               greenPal[i][g]=fullpal[g]+delta*(i+1)/GREENSTEPS;
               delta=-fullpal[r];
               greenPal[i][r]=fullpal[r]+delta*(i+1)/GREENSTEPS;
               delta=-fullpal[b];
               greenPal[i][b]=fullpal[b]+delta*(i+1)/GREENSTEPS;
          }
     }
     for (i=0 ; i < NUMBLUESHIFTS ; i++) {
          for (j=0 ; j < 256 ; j++) {
               r=j*3;
               g=r+1;
               b=r+2;
               delta=64-fullpal[b];
               bluePal[i][b]=fullpal[b]+delta*(i+1)/BLUESTEPS;
               delta=-fullpal[g];
               bluePal[i][g]=fullpal[g]+delta*(i+1)/BLUESTEPS;
               delta=-fullpal[r];
               bluePal[i][r]=fullpal[r]+delta*(i+1)/BLUESTEPS;
          }
     }
#endif
}

void
GFX_initFadePalette(void)
{
#ifdef GFXMODULE_ENABLED
     memmove(fullpal,palette,768);
     GFX_initPalShifts();
     animPalClock=totalclock;
#endif
}

void
GFX_fadePalette(int intensity)
{
#ifdef GFXMODULE_ENABLED
     int  i;

     for (i=0 ; i < 768 ; i++) {
          palette[i]=(fullpal[i]*intensity)>>8;
     }
#endif
}

int
GFX_screenFaded(void)
{
#ifdef GFXMODULE_ENABLED
     if (fadeIntensity != 255) {
          return(1);
     }
#endif
     return(0);
}

void
GFX_waitFade(void)
{
#ifdef GFXMODULE_ENABLED
     KBD_resetKeys();
     do {
          if (fadeOutFlag) {
               GFX_fadeOut(fadeSpeed);
          }
          else if (fadeInFlag) {
               GFX_fadeIn(fadeSpeed);
          }
          else {
               return;
          }
          if (KBD_keyPressed()) {
               if (fadeOutFlag) {
                    GFX_fadeOut(255);
               }
               else if (fadeInFlag) {
                    GFX_fadeIn(255);
               }
               return;
          }
     } while (1);
#endif
}

void
GFX_fadeOut(int speed)
{
#ifdef GFXMODULE_ENABLED
     if (fadeIntensity > 0) {
          fadeOutFlag=1;
          fadeInFlag=0;
          fadeSpeed=speed;
     }
     if (playingGameFlag) {
          GFX_animatePalette();
     }
#endif
}

void
GFX_waitFadeOut(int speed)
{
#ifdef GFXMODULE_ENABLED
     GFX_fadeOut(speed);
     GFX_waitFade();
#endif
}

void
GFX_fadeIn(int speed)
{
#ifdef GFXMODULE_ENABLED
     if (fadeIntensity < 255) {
          fadeInFlag=1;
          fadeOutFlag=0;
          fadeSpeed=speed;
     }
     if (playingGameFlag) {
          GFX_animatePalette();
     }
#endif
}

void
GFX_waitFadeIn(int speed)
{
#ifdef GFXMODULE_ENABLED
     GFX_fadeIn(speed);
     GFX_waitFade();
#endif
}

void
GFX_stopFade(void)
{
#ifdef GFXMODULE_ENABLED
     fadeOutFlag=0;
     fadeInFlag=0;
     fadeSpeed=0;
#endif
}

void
GFX_redShift(int n)
{
#ifdef GFXMODULE_ENABLED
     redShift+=n;
     if (redShift > 255) {
          redShift=255;
     }
#endif
}

void
GFX_greenShift(int n)
{
#ifdef GFXMODULE_ENABLED
     greenShift+=(n>>2);
     if (greenShift > 255) {
          greenShift=255;
     }
#endif
}

void
GFX_blueShift(int n)
{
#ifdef GFXMODULE_ENABLED
     blueShift+=(n>>1);
     if (blueShift > 255) {
          blueShift=255;
     }
#endif
}

void
GFX_animatePalette(void)
{
#ifdef GFXMODULE_ENABLED
     int  delta,
          red=0,green=0,blue=0,
          updatePalFlag=0;
     static int lastred,lastgreen,lastblue;

     delta=totalclock-animPalClock;
     animPalClock=totalclock;
     if (fadeOutFlag) {
          GFX_fadePalette(fadeIntensity);
          if (fadeIntensity == 0) {
               fadeOutFlag=0;
          }
          else {
               fadeIntensity-=(fadeSpeed*delta);
               if (fadeIntensity < 0) {
                    fadeIntensity=0;
               }
          }
          updatePalFlag=1;
     }
     else if (fadeInFlag) {
          GFX_fadePalette(fadeIntensity);
          if (fadeIntensity == 255) {
               fadeInFlag=0;
          }
          else {
               fadeIntensity+=(fadeSpeed*delta);
               if (fadeIntensity > 255) {
                    fadeIntensity=255;
               }
          }
          updatePalFlag=1;
     }
     if (updatePalFlag || redShift > 0 || greenShift > 0 || blueShift > 0) {
          if (redShift > 0) {
               redShift-=delta;
               if (redShift < 0) {
                    redShift=0;
               }
               red=redShift/10;
               if (red >= NUMREDSHIFTS) {
                    red=NUMREDSHIFTS-1;
               }
          }
          if (greenShift > 0) {
               greenShift-=delta;
               if (greenShift < 0) {
                    greenShift=0;
               }
               green=greenShift/10;
               if (green >= NUMGREENSHIFTS) {
                    green=NUMGREENSHIFTS-1;
               }
          }
          if (blueShift > 0) {
               blueShift-=delta;
               if (blueShift < 0) {
                    blueShift=0;
               }
               blue=blueShift/10;
               if (blue >= NUMBLUESHIFTS) {
                    blue=NUMBLUESHIFTS-1;
               }
          }
          if (red > 0 && qsetmode == 200L) {
               if (red != lastred) {
                    GFX_setPalette(&redPal[red]);
               }
               lastred=red;
          }
          else if (green > 0 && qsetmode == 200L) {
               if (green != lastgreen) {
                    GFX_setPalette(&greenPal[green]);
               }
               lastgreen=green;
          }
          else if (blue > 0 && qsetmode == 200L) {
               if (blue != lastblue) {
                    GFX_setPalette(&bluePal[blue]);
               }
               lastblue=blue;
          }
          else if (qsetmode == 200L) {
               GFX_setPalette(&palette[0]);
          }
     }
#endif
}

void
GFX_initPlayerShifts(void)
{
#ifdef GFXMODULE_ENABLED
     redShift=1;
     blueShift=1;
     greenShift=1;
#endif
}

