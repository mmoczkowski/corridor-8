/***************************************************************************
 *   CORR7CFG.C - configuration file routines for Corridor 7 game
 *
 *                                                     02/27/96  Les Bird
 ***************************************************************************/

#include "icorp.h"
#include <memcheck.h>

void
CFG_readConfigFile(void)
{
     int  i;

     readControlConfigs(0);
     for (i=0 ; i < MAXACTIONS ; i++) {
          keys[i]=configKeyboard[i];
     }
}

void
CFG_writeConfigFile(char *configName)
{
}

