
/* #*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#

                        MemCheck 3.5 Header File
                           (DOS or Windows)

           Copyright (c) 1990-1996, StratosWare Corporation.  
                         All rights reserved.

        Note to Developers:
        -------------------
        This file should be #included AFTER any other #includes in
        each source file which is to be memory checked, and BEFORE
        any code that performs any operations on allocated pointers.
        If it isn't, MemCheck will not pickup source file and line
        information for intercepted functions.

        The MCCONFIG.EXE (DOS) or MCCONFIGW.EXE application distributed 
        with MemCheck will do this safely and quickly for you.

        Most specifically, this header file shouldn't precede
        any prototypes of routines that MemCheck intercepts, like
        GlobalAlloc(), GetDC(), malloc(), free(), _fstrcpy(), and so on.

        The Final Cut:
        --------------
        To ENTIRELY remove MemCheck from your code, just #define
        the constant "NOMEMCHECK", or equivalently, "NOMC".
        
        This header file will then automatically 'evaporate' all
        MemCheck calls!

        This is MUCH PREFERABLE to placing #if-#endif's around
        the header file's inclusion, as in

                    #ifdef _DEBUG        /-* DON'T DO THIS! *-/
                    #include <memcheck.h>
                    #endif

        Using the "#ifdef DEBUG" as above doesn't allow the
        MemCheck header file to evaporate the MemCheck API
        calls you may have placed in your code, like mc_startcheck()
        and mc_endcheck().

        StratosWare Corporation
        1756 Plymouth Road, Suite 1500
        Ann Arbor, MI 48105-1890

        Internet:   techsup@stratosware.com
                    sales@stratosware.com
                    info@stratosware.com
        WWW:        http://www.stratosware.com/swc
        CompuServe: 70244,1372
        Fax Lines:  313-996-2955 
                    313-747-8519
        BBS:        313-996-2993
        Phone:      800-WE-DEBUG
                    313-996-2944 International

*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*# */


/* Avoid multiple inclusions */
#ifndef _MEMCHECK_H_
#define _MEMCHECK_H_

/* Define platform:
    _CC_WINDOWS_    Windows
    _CC_DOS_        DOS
*/
#ifndef _CC_WINDOWS_
#ifndef _CC_DOS_
#if defined(_Windows) || defined(WINDOWS) || defined(_WINDOWS) || defined(_WINDOWS_) || defined(_INC_WINDOWS) || defined(__WINDOWS_H)
#   if !defined (__DPMI16__) && !defined (__DPMI32__) && !defined (DOSX286)
#       define _CC_WINDOWS_
#   else
#       define _CC_DOS_
#   endif
#else
#   define _CC_DOS_
#endif
#endif  /* not DOS */
#endif  /* not WINDOWS */

/* bifurcation! */
#if defined(_CC_WINDOWS_) 

    /* Include Windows header */
    #include "_mcwin.h"

#else

    /* Include DOS header */
    #include "_mcdos.h"

#endif

#endif  /* _MEMCHECK_H_ */
