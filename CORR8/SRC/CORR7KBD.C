/***************************************************************************
 *   CORR7KBD.C - Keyboard functions for Corridor 7 game
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

char tkeys[256];

short globalMouseB,
     globalMouseX,
     globalMouseY;

void
KBD_resetKeys(void)
{
     int  k;

     for (k=0 ; k < 256 ; k++) {
          keystatus[k]=0;
     }
}

int
KBD_keyPressed(void)
{
     int  k;
     static int lastMouseB;

     for (k=0 ; k < 256 ; k++) {
          if (keystatus[k] && k != 0xAA) {
               KBD_resetKeys();
               return(k);
          }
     }
     PLR_getMouse(&globalMouseX,&globalMouseY,&globalMouseB);
     if (globalMouseB != 0 && lastMouseB == 0) {
          lastMouseB=globalMouseB;
          if ((globalMouseB&1) != 0) {
               return(K_ENTER);
          }
          else if ((globalMouseB&2) != 0) {
               return(K_ESC);
          }
          else if ((globalMouseB&4) != 0) {
               return(K_ESC);
          }
     }
     lastMouseB=globalMouseB;
     return(0);
}

void
KBD_copyKeys(void)
{
     memmove(tkeys,(char *)keystatus,sizeof(tkeys));
}

