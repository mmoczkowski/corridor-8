/***************************************************************************
 *   ICORP.H   - main include file for Corridor 7                          *
 *                                                                         *
 *                                                     02/27/96  Les Bird  *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>
#include <dos.h>
#include <io.h>
#include <conio.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <process.h>

//** BUILD includes
#include "build.h"
#include "names.h"
#include "pragmas.h"

//** SOS includes
#include "sos.h"
#include "sosm.h"
#include "profile.h"
#if 0
#include "sosez.h"
#endif

//** SCITECH PMODE.LIB includes
#include "pmpro.h"

//** Corridor 7 includes
#include "corr7.h"

