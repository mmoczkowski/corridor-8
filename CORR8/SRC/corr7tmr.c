/***************************************************************************
 *   CORR7TMR.C - Timer related routines for Corridor 7 game
 *
 *                                                     02/29/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

#define   CLKIPS         120

int  timerInitializedFlag;

W32  hTimer;

long oTPSClock,
     oTPSFrames;

volatile
long ticsPerFrame=TICWAITPERFRAME;

void
TMR_resetTicsPerFrame(void)
{
     oTPSClock = totalclock;
     oTPSFrames = numframes;
}

void
TMR_timerHandler(void)
{
     totalclock++;
     if (!playingGameFlag) {
          GFX_animatePalette();
     }
     keytimerstuff();
}

void
TMR_initTimer(void)
{
     if (timerInitializedFlag == 0) {
          sosTIMERInitSystem(_TIMER_DOS_RATE,_SOS_DEBUG_NORMAL);
          sosTIMERRegisterEvent(CLKIPS,TMR_timerHandler,&hTimer);
          timerInitializedFlag=1;
     }
}

void
TMR_uninitTimer(void)
{
     if (timerInitializedFlag) {
          sosTIMERRemoveEvent(hTimer);
          SND_uninitSOS();
          sosTIMERUnInitSystem(0);
          timerInitializedFlag=0;
     }
}

void
TMR_registerEvent(W32 rate,void (*pEvent)(void),HANDLE *hEvent)
{
     sosTIMERRegisterEvent(rate,pEvent,hEvent);
}

void
TMR_removeEvent(HANDLE hEvent)
{
     sosTIMERRemoveEvent(hEvent);
}

long
TMR_getSecondTics(long seconds)
{
     return(seconds*CLKIPS);
}

long
TMR_getSecondFraction(int frac)
{
     switch (frac) {
     case TICS_ONESECOND:
          return(CLKIPS);
     case TICS_ONEHALF:
          return(CLKIPS>>1);
     case TICS_ONEFOURTH:
          return(CLKIPS>>2);
     case TICS_ONEEIGHTH:
          return(CLKIPS>>3);
     case TICS_ONESIXTEENTH:
          return(CLKIPS>>4);
     case TICS_ONETHIRTYSECOND:
          return(CLKIPS>>5);
     }
     return(0);
}

