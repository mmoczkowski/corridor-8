/***************************************************************************
 *   CORR7CD.C - CD-ROM & CD player routines for Corridor 7 game
 *
 *                                                     06/04/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

char far *cdDrivesPtr;

void
CD_getDriveLetters(void)
{
     PMREGS regs;
     PMSREGS sregs;

     if (cdDrivesPtr == NULL) {
          PM_allocRealSeg(26,&pmsel,&pmoff,&rmseg,&rmoff);
          cdDrivesPtr=MK_PHYS(rmseg,rmoff);
     }
     regs.ax=0x150D;
     regs.bx=rmoff;
     sregs.es=rmseg;
     PM_int386x(0x2F,&regs,&regs,&sregs);
}

int
CD_mscdexLoaded(void)
{
     PMREGS regs;

     regs.ax=0x1500;
     regs.bx=0;
     PM_int386(0x2F,&regs,&regs);
     if (regs.bx == 0) {
          return(0);
     }
     CD_getDriveLetters();
     return(1);
}


