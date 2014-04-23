/* #*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#

    MemCheck(R) 3.5 Professional
    Copyright (c) 1990-1996, StratosWare Corporation.  
    All rights reserved. 
    1-800-WE-DEBUG

    _MCDOS.H - internal MemCheck for DOS header

    DON'T INCLUDE THIS FILE DIRECTLY!
    Include via '#include <memcheck.h>'.

    Modification History

    WHO     WHEN        WHAT
    KWB     04/12/95    Yanked from 3.0, began 3.5 thread
    MJW     05/22/95    Changed MC_ExceptList[] to type MCEXCEPTOLD
    MJW     06/02/95    Added stack overflow handlers & related noize
    MJW     06/07/95    Conditioned #pragma -instrinsics out of DPMI32 build
    MJW     06/09/95    Added mc_stack_free(), mc_stack_xxx_used()
    MJW     06/15/95    Added MCE_STACK_OVERFLOW to MCError
    MJW     06/26/95    Converted MC_ExceptList internals to _MCFAR
    MJW     06/30/95    Corrected definition of 32 bit MCEXCEPTINFO 
    MJW     07/11/95    Implemented new interception method
    MJW     07/31/95    Insured keyword pascal is intact
    MJW     08/28/95    Renamed mc_errno() to _mc_errno(). Transparent to user.
    MJW     11/01/95    Integrated mcproto.h, protomc.h, and mcmacro.h
    MJW     11/27/95    Added defaults D_DZScanFreq & D_DZBytes
	MJW		01/23/96	Added Advanced Pointer Validation
	MJW		01/29/96	Fixed mcwatcom_strcpy() problem with sizeof()
	MJW		01/30/96	Changed DZScanFreq to unsigned short to allow > 256
	MJW		02/19/96	Followed MCW35 technique for operator new declaration
	MJW		03/01/96	Added support for Globalxxx() Windows API emulation
	MJW		03/04/96	Added MC_NO_NEW_OVERLOADED macro for ease of use
	MJW		03/12/96	Added TRACKF_INIT flag for tracking functions
						Added MC_ALWAYS_HANDLE_EXCEPTION()
	MJW		03/19/96	Finalized Phar Lap TNT support for Borland
						Added run-time MCF_HANDLE_EXCEPT
	MJW		03/21/96	Version 3.5 Release
	MJW		04/01/96	MSC6 -> _CC_MSC6_
	MJW		04/08/96	Fixed Borland setvect() decl for __cplusplus
						Added _MCCDECL to _HOOK() prototypes
	MJW		04/09/96	Added fcloseall() interception to prevent premature
						closing of log file.
	MJW		04/16/96	Changed _HOOK() macro to use _RTL
	MJW		05/02/96	MCCRITSECT structure / MCCS_X() macro consistency
	MJW		05/13/96	MC_NEW_OVERLOADED conditional tightened
						Added MC_SET_DLL_CFGFILE() for PP16 DLL support
*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*# */

/* Avoid multiple inclusions */
#ifndef MCDOS_H_
#define MCDOS_H_

#include "_ccdefs.h"
/* NOTE: this assumes that the target platform check
    in the MemCheck header file (MEMCHECK.H) has
    already been done.  The target platform should, of course,
    be DOS, indicated by the definition of _CC_DOS_.
*/
#ifndef _CC_DOS_
#error Include MEMCHECK.H instead!
#endif

#if !defined (NULL)     /* pull in stdio.h if not already */
#   include <stdio.h>
#endif

/* Backup... sometimes NULL defined in other headers */
#if !defined (_IOFBF)   /* pull in stdio.h if not already */
#   include <stdio.h>
#endif

/* Standard includes */
#include <string.h>
#include <stdlib.h>
#ifndef _CC_WATCOM_
#	include <memory.h>
#endif
#if defined(_CC_MSC_COMPATIBLE_)
	#include <malloc.h>
#elif defined(_CC_BORLAND_)
	#include <alloc.h>
#endif

/* Windows API emulation. 
		If you are using Windows memory-management API calls (emulated
	by a DOS extender such as Boland's DPMI32 or Phar Lap's TNT DOS-
	Extender), you should have MC_USE_WIN_ALLOCS #defined.
		If MemCheck determines that you have included the windows.h header
	file, it will #define MC_USE_WIN_ALLOCS for you.
		If you encounter problems related to GlobalAlloc(), etc, at compile-
	time, change the next line to '#if 0'.
*/
#if 1	

/*	#if defined (_CC_POWERPACK_)	*/
#if defined (WINVER)
#	define MC_USE_WIN_ALLOCS
#endif

#endif

/* Include windows.h if this DOS app uses Windows emulation APIs */
#if defined(MC_USE_WIN_ALLOCS)
#	include <windows.h>
#endif

#if defined (__HUGE__)
#define MC_NO_APV
#endif

/* --- Compatibility layers --- */

/* Include only if older API emulation needed */
/* Layer: 2.1 and earlier */
/* #include <mcc21.h> */    

/* --- 3.0-to-3.5 compatibility layer --- */

#ifdef NEW_OVERLOADED
#   define MC_NEW_OVERLOADED
#endif

/* --- Equivalence & shorthand --- */

#if defined(NOMC)
#   define NOMEMCHECK
#endif

/* --- Insure proper calling convention --- */
/*
    Some header files #define out pascal under certain conditions.
    This must be rectified for proper linkage.
*/

#if defined (_CC_POWERPACK32_)  
#if defined pascal
#   undef pascal
#   define pascal __pascal  /* This should do the trick */
#endif /* defined pascal */
#endif /* defined _CC_POWERPACK32_ */

/* --- C++ new and delete --- */

/*  Please read the MemCheck User's Manual, 
    section "Integration With C++."  

    Basically, defining MC_NEW_OVERLOADED allows seamless
    transmission of the exact file and line location
    of calls to operator new in your source code.

*/
#ifdef __cplusplus
#   if ( !defined (MC_NEW_OVERLOADED) )
        /* it's default now */
#       define MC_NEW_OVERLOADED
#   endif
	/* allow for possiblity that NEW_OVERLOADED was #defined externally */
#	if ( defined (MC_NEW_OVERLOADED) && defined(MC_NO_NEW_OVERLOADED) )
#		undef MC_NEW_OVERLOADED
#	endif
#endif

/* --- Errors --- */

/*  Error Number Definitions
    Returned by mc_endcheck() and mc_error_flags().
    In MemCheck 3.0, a global error number was added much
    like the "errno" variable in standard C.   

    MC Developer's Note: make sure to add an entry in XETEXT
        for each new error.
*/
enum MCError {
    MCE_NO_ERROR=0,     /* it's debugging time & all well */
    MCE_NULL_SOURCE,    /* null source ptr on copy */
    MCE_NULL_DEST,      /* null dest ptr on copy */
    MCE_UNDERWRITE,     /* allocated buf underwritten (front) */
    MCE_OVERWRITE,      /* allocated buf overwritten (end) */
    MCE_LEAK,           /* forgot to free alloc'd memory */
    MCE_LEAKAGE=MCE_LEAK,
    MCE_UNFREED_MEMORY=MCE_LEAK,
    MCE_NULL_PTR_ASSIGN,    /* assigned data through null ptr */
    MCE_BAD_STACK_PTR,      /* bad stack pointer */
    MCE_STACK_OVERWRITE,    /* copy trashes stack frame */
    MCE_INTERNAL,           /* internal error msg */
    MCE_OVERLAPPING_COPY,   /* source overlaps dest on copy */
    MCE_INVALID_PTR ,       /* bad ptr on free, realloc */
    MCE_DEST_OVERWRITE,     /* copy too big for dest buffer */
    MCE_OUT_OF_MEMORY,      /* out of memory */
    MCE_OOM=MCE_OUT_OF_MEMORY,
    MCE_GPF_PTR,            /* ptr caused GPF */
    MCE_STACK_OVERFLOW,     /* sp detected below danger level */ 
    MCE_FREE_OF_OBJECT,     /* free of pointer allocated with new */
    MCE_INVALID_FILE_PTR,   /* Use of invalid file ptr on fclose, freopen */
	MCE_COPY_TO_CODE,		/* transfer destination is code ptr */
	MCE_BAD_HEAP_PTR,		/* bad heap pointer	*/
    MCE_LAST_ERROR          /* ALWAYS last */
};

/*  MemCheck Error Flags 

    The MemCheck error flag is just an unsigned long 
    (specifically, a MCEFLAGS typedef) with "sticky" bits 
    corresponding to the above MCE_... error numbers.  
    You can use the error flags macro MC_EFLAG(e) to 
    test the global MC_Errno double flag word.
*/
/* Returns a long word with the e-th bit set */
#define MC_EFLAG(e) ( (e) ? ((MCEFLAGS)1 << (e-1)) : (MCEFLAGS)0)

#define EFLAG_NULL_PTR          MC_EFLAG(MCE_NULL_DEST)
#define EFLAG_BAD_PTR           MC_EFLAG(MCE_UNALLOCED_PTR)
#define EFLAG_FRONT_MAGIC       MC_EFLAG(MCE_UNDERWRITE)
#define EFLAG_BACK_MAGIC        MC_EFLAG(MCE_OVERWRITE)
#define EFLAG_PTRS_NOT_FREED    MC_EFLAG(MCE_LEAK)
#define EFLAG_TRACK_FILE        0   /*obsolete in 3.0+*/
#define EFLAG_NULL_ASSIGN       MCE_FLAG(MCE_NULL_PTR_ASSIGN)

/* --- MemCheck version --- */

/*  Standard from MemCheck 3.0 onwards.
    Access major version and revision
    via mc_version() and mc_revision() macros.
*/
#define _MC_VERSION     0x0350      /* welcome 3.5 */
#define mc_version()    ((int)((_MC_VERSION & 0xFF00) >> 8))
#define mc_revision()   ((int)(_MC_VERSION & 0x00FF))

/* --- Output formatting --- */

#if defined (_CC32_)    /* 32-bit Intel target */
    /* Far pointer */
    #define MC_FMT_FP "0x%p"
    /* Far string */
    #define MC_FMT_FS "%s"
#else
    /* Far pointer */
    #define MC_FMT_FP "%Fp"
    /* Far string */
    #define MC_FMT_FS "%Fs"
#endif

/* --- Based-text strings --- */

/* ALERT At what version numbers does based support come in for MSC & BC? */
#if !defined(_CC_LDATA_)
        #define MC_BASED_CODE
#elif defined(_CC_MSC_)
    #if (_MSC_VER >= 700)
        #define MC_BASED_CODE  __based(__segname("_MCDATA"))
    #else
        #define MC_BASED_CODE  far
    #endif /* msc_ver >= 700 */
#elif defined(_CC_BORLAND_)
    #if defined (_CC32_)
        #define MC_BASED_CODE
    #else
        #define MC_BASED_CODE  far
    #endif
#else
    /* no basing */
    #define MC_BASED_CODE
#endif
typedef char MC_BASED_CODE *MCSTR;

/* --- MemCheck function id's --- */

/* Use: _MCID(mc_startcheck) or _MCID(malloc) etc. */
#define _MCID(f)    (MCID)(__paste(MCID_,f))
enum MCIDS {
    MCID_mc_startcheck,
    MCID_mc_endcheck,       
    MCID_mc_check_buffers,      
    MCID_mc_check,          
    MCID_mc_register,
    MCID_mc_unregister,
    MCID_mc_set_alignsize,  
    MCID_mc_set_checkbytes, 
    MCID_mc_nullcheck,          
    MCID_mc_breakpoint, 

    MCID_mc_debug,      
    MCID_mc_set_location,
    MCID_mc_stack_trace,
    MCID_mc_report,

    MCID_calloc,
    MCID_free,
    MCID_malloc,
    MCID_memcpy,
    MCID_memmove,
    MCID_memset,
    MCID_realloc,
    MCID_sprintf,
    MCID_strcat,
    MCID_strcpy,
    MCID_strncat,
    MCID_strncpy,
    MCID_vsprintf,

    MCID_cgets,
    MCID_delete,
    MCID_farcalloc,
    MCID_farfree,
    MCID_farmalloc,
    MCID_farrealloc,
    MCID_getcwd,
    MCID_memccpy,
    MCID_movmem,
    MCID_new,
    MCID_setmem,
    MCID_setvect,
    MCID_stpcpy,
    MCID_strdup,
    MCID_strnset,
    MCID_strset,
    MCID_swab,
    MCID_tempnam,
    MCID__cgets,
    MCID__dos_setvect,
    MCID__expand,
    MCID__fcalloc,
    MCID__fexpand,
    MCID__ffree,
    MCID__fmalloc,
    MCID__fmemccpy,
    MCID__fmemcpy,
    MCID__fmemmove,
    MCID__fmemset,
    MCID__fmovmem,
    MCID__fmsize,
    MCID__frealloc,
    MCID__fsetmem,
    MCID__fstrcat,
    MCID__fstrcpy,
    MCID__fstrdup,
    MCID__fstrncat,
    MCID__fstrncpy,
    MCID__fstrnset,
    MCID__fstrset,
    MCID__fullpath,
    MCID__getcwd,
    MCID__getdcwd,
    MCID__halloc,
    MCID__hfree,
    MCID__memccpy,
    MCID__msize,
    MCID__ncalloc,
    MCID__nexpand,
    MCID__nfree,
    MCID__nmalloc,
    MCID__nmsize,
    MCID__nrealloc,
    MCID__nstrdup,
    MCID__strdup,
    MCID__strnset,
    MCID__strset,
    MCID__swab,
    MCID__tempnam,
    MCID_fopen,
    MCID__fsopen,
    MCID_fclose,
    MCID_freopen,
	MCID_GlobalAlloc,
	MCID_GlobalReAlloc,
	MCID_GlobalFree,
	MCID_GlobalDosAlloc, 
	MCID_GlobalDosFree,
	MCID_fcloseall,
	MCID__fcloseall,
    MCID_LAST
};

/* --- Stack Information enum --- */
/* Passed as parameters to mc_stack_info() by mc_stack_xxx macros.
    No need for MemCheck user to use these; use the equivilent
    API function instead.
*/
enum MCSIENUM {
    _MCSTACK_LENGTH,
    _MCSTACK_USABLE,
    _MCSTACK_USED,
    _MCSTACK_MAX_USED,
    _MCSTACK_FREE,
    _MCSTACK_UNUSED,
	_MCSTACK_RESERVED
};

/* --- TOUCH macro --- */
/* 
    ... so high warning levels don't generate
    'unreferenced variable' warnings, especially when
    making Production libraries... 
*/
#if defined (_CC_WATCOM_)
#   define TOUCH(var)   var = var
#elif defined (_CC_BORLAND4_)
#   define TOUCH(var)   var = var
#else
#   define TOUCH(var)   if (var)
#endif


/* Default log name */
#define MEMCHECK_LOG    "MEMCHECK.LOG"

#define MAX_MEMORY  1000    /* 1000K is more than ever possible */

/* User-Modifiable Defaults */

#define D_CheckByteCt   sizeof(int)     /* word size is default */
#define D_AlignSize     sizeof(int)     /* align returned memory ptrs */

/*  Number of bytes to copy from null segment (to determine null
    pointer assignments) 
*/
#define D_NULLCHECK_BYTES_FAR       16      /* at 0000:0000 (far NULL) */
#define D_NULLCHECK_BYTES_NEAR      16      /* at   DS:0000 (near NULL) */
#define MAX_NULLCHECK_BYTES_FAR     1024    /* extent of irupt vect tbl */
#define MAX_NULLCHECK_BYTES_NEAR    66      /* reserved in DS */

/* Length of stack Danger Zone (if stack checking is on)    */
#define D_DZBytes                   512

/* Frequency of intensive stack check (if stack checking on)    */
#define D_DZScanFreq                10

/* Unroll the double-negative */
/*
    Debugging code specific to MemCheck can be 
    conditionally compiled by placing it within
    #if-#endif sections:  (NOTE that this is NOT
    required when just using API functions)

        #ifdef MEMCHECK

        void _MCCALLBACK trackf_special (int op, MEMRECP memrecp)
        {
           (... your custom callback code ...)
        }

        #endif

    instead of the more arcane

        #ifndef NOMEMCHECK
        :
        #endif

    (Both approaches work equally well, however...)
*/
#ifndef NOMEMCHECK      /* MemCheck active */
#define MEMCHECK    
#endif


/* *** Calling Conventions *** */

#if !defined (_CC_ANSI_)
#define     _MCAPI          pascal      /* MemCheck API functions   */
#define     _FASTAPI        pascal      /* speed-critical functions */
#define     _MCCDECL        cdecl       /* MemCheck varargs API     */
#define     _MCCALLBACK     cdecl       /* callback functions       */
#define     _MCVAR          cdecl       /* MemCheck global variable */
#else
#define     _MCAPI          /* MemCheck API functions   */
#define     _FASTAPI        /* speed-critical functions */
#define     _MCCDECL        /* MemCheck varargs API     */
#define     _MCCALLBACK     /* callback functions       */
#define     _MCVAR          /* MemCheck global variable */
#endif
#if !defined(_CC_WATCOM_)
#   define  _RTL            _MCCDECL    /* RTL calling conv */
#else
#   define  _RTL                        /* RTL calling conv */

/*  WATCOM C++ does not currently (2/17/94)
    accept "cdecl" as a modifier on variables...  
*/
#   undef _MCVAR
#   define _MCVAR
#endif  /* _CC_WATCOM_ */

/*   32-bit compiler-independent stuff */
#if !defined(_CC32_) 
#define _MCFAR      far
#define _MCFARCALL  far
#define _MCNEAR     near
#define _MCNEARCALL near
#define _MCHUGE     huge
#define _MCHFAR     far     
#else
#define _MCHFAR
#define _MCFAR
#define _MCFARCALL
#define _MCNEAR
#define _MCNEARCALL
#define _MCHUGE 
#if defined (_CC_WATCOM_)
#   undef _MCHFAR
#   define _MCHFAR  far /* only Watcom allows far pointers in 32 bit */
#endif
#endif  /* _CC32_ */

/*
    MSC declares the following routines as "far"... 
    So does Borland.  WATCOM does not; define glue.

        _fstrset        _fstrnset       _fstrcpy 
        _fstrncpy       _fstrcat        _fstrncat
        _fmemset        _fmemmove       _fmemccpy
        _fstrdup (WC10 only)            _fmemcpy
*/
#if !defined (_CC32_)
#if defined(_CC_WATCOM_)
#   define _MCFARGLUE   
#else
#   define _MCFARGLUE   far 
#endif
#else
#   define _MCFARGLUE
#endif

/*  Microsoft C7 and later will not have
    have a malloc_mc, only _fmalloc_mc; likewise
    with free_mc.
    The RTLMALLOC and RTLFREE macros are used
    to refer to a generically present back-end malloc
    and free.
*/
#if defined (_CC_MSC_)
#   if defined (_CC_LDATA_)
#       define RTLMALLOC    RTL(_fmalloc)
#       define RTLFREE      RTL(_ffree)
#   else
#       define RTLMALLOC    RTL(_nmalloc)
#       define RTLFREE      RTL(_nfree)
#   endif
#else   /* non-MSC */
#   define RTLMALLOC    RTL(malloc)
#   define RTLFREE      RTL(free)
#endif


/*  WATCOM defines its atexit funcs as a "register",
    which causes a param type mismatch.  
    _ATEXITFUNC calling convention smooths this out.
*/
#if defined (_CC_WATCOM_)
#   define _ATEXITFUNC  
#else
#   define _ATEXITFUNC      _MCCDECL
#endif

/*  MemCheck Tracking Mode 
    
    Returned by mc_get_mode().
    Indicates whether information on each allocation
    is being stored in memory or on disk.
*/
#define MC_USING_MEMORY     1
#define MC_USING_DISK       2

/* Min, max orders for each node in the B-tree */

#define BT_ORDER_MIN        5
#define BT_ORDER_MAX        255     /* maximum tree order */
#define BT_ORDER_DEFAULT    19      /* default tree order */

/*
    Returned by mc_get_speed().
    Pass as params to mc_set_speed().
*/
#define MC_RUN_NORMAL       1
#define MC_RUN_FAST         2

/*  For mc_report():
    "Flags" field of the MEMREC structure
    is set to REPORT_START or REPORT_END
    to indicate begin and end of report.

    NOTE: If REPORT_START or REPORT_END conflicts
        with defines in your project, just comment
        them out and use the MC_... variants instead.
*/
#define REPORT_START        (MRFLAGS)0xFE
#define REPORT_END          (MRFLAGS)0xFD

#define MC_REPORT_START     (MRFLAGS)0xFE   /* alternates in case of conflict */
#define MC_REPORT_END       (MRFLAGS)0xFD


/*
    Maximum number of breakpoints that
    can be set via mc_breakpoint().
*/
#define MC_MAX_BREAKS       50


/*  "Optype" parameter on Tracking function callback. */
#define TRACKF_INIT		0		/* first call to tracking function */
#define TRACKF_ADD      1       /* record being added to tree */
#define TRACKF_DEL      2       /* record being deleted from tree */

/*  Used for the mcflags field of MEMREC to indicate
    whether file & line are exact or approximate
*/
#define MRFLAG_EXACT_LOC        ( (MRFLAGS) 0x01)

/*
    Set if the values for a MEMREC are already converted
    to "user" values.
*/
#define MRFLAG_USER_SPECS       ( (MRFLAGS) 0x02)
#define MRFLAG_NO_CHECKBYTES    ( (MRFLAGS) 0x04)

/* Alternate name */
#define mc_message      mc_debug 

/*
    Parameter to mc_check_transfer() that
    specifies that the given data transfer function cannot
    have overlapping source & destination.
    (MCID's are unsigned bytes.)
*/
#define MCF_NO_OVERLAP      ((unsigned)0x8000)
#define NO_OVERLAP(mcid)    ((mcid) | MCF_NO_OVERLAP)

/*  Parameter to mc_check_transfer indicating that
    the found memory record is not needed */
#define NO_MEMREC           ((MEMRECP)NULL) 
#define NOMEMREC            NO_MEMREC

/*  Parameter to mc_check_transfer indicating that
    there is no source pointer operand associated
    with the data transfer being checked: e.g. memset.  */
#define NO_SOURCE           ((void _MCFAR *)0xFFFFFFFA)


/* *** TYPEDEFS *** */

typedef char *          MCSF;       /* MemCheck source file */
typedef unsigned int    MCSL;       /* MemCheck source line */
typedef unsigned char   MCID;       /* MemCheck function ID */

typedef unsigned long   MCEFLAGS;   /* MemCheck error flags */
typedef void _MCFAR *   MCPTR;      /* type of ptr stored in tree */
typedef FILE _MCFAR *	MCFILEP;	/* type of FILE structure (internal) */
typedef unsigned char   MRFLAGS;    /* flags in MEMRECORD */
typedef unsigned long   MCFLAGS;    /* MemCheck settings flags */

/* MemCheck Rocket allocator prototypes */
typedef void _MCFAR * (_MCFAR *ROCKETALLOCF)    (size_t);
typedef void          (_MCFAR *ROCKETFREEF)     (void _MCFAR *);

#pragma pack(1)
/* 
    Memory Tracking Structure (MEMREC)

    This is the data structure for buffers being
    tracked by MemCheck.
*/
typedef struct MemRecord 
    {
        MCPTR           ptr;            /* heap/registered ptr */
        MCID            mcid;           /* MemCheck function ID */
        MRFLAGS         flags;          /* internal MC flags */
        unsigned long   allocno;        /* cardinality of allocation */
        unsigned long   size;           /* size of block */
        MCSF            file;           /* source file */
        MCSL            line;           /* source line */

    } MEMREC, _MCFAR *MEMRECP;


/* *** SETTINGS *** */
/* These are values that describe the life of a MemCheck run. */

typedef struct MCSETTINGS { 
    /*
        Bit Flag                What
        --- ---------------     -----------------------------------
        0   MCF_ACTIVE          MemCheck active or off
        1   MCF_FAST_MODE       Fast mode or normal
        2   MCF_PROTECTED_MODE  Protected mode or real
        3   MCF_FAR_NULL_CHECK  Check for far NULL ptr assigns *
        4   MCF_NEAR_NULL_CHECK Check for far NULL ptr assigns *
        5   MCF_STANDARD_STACK  Standard stack frame *
        6   MCF_AUTOINIT        Start up automatically *
        7   MCF_CLEAR_ON_FREE   Clear buffers when freed 
        8   MCF_DISK_ROCKET     Use DiskRocket options
        9   MCF_IDX_IN_MEMORY   Use memory only for Rocket indexes *
                                (only if DiskRocket linked)
        10  MCF_SOURCE_ONLY     Intercept in source code only 
        11  MCF_STACK_TRACK     Intercept stack checking calls
        12  MCF_LOG_ON          Log MemCheck messages to log file
        13  MCF_LOG_FLUSH       Flush the log file after each write
        14  MCF_CHECK_FILES     Track fopen()/fclose() usage
		15  MCF_APV				Perform Advanced Pointer Validation 
       	16  MCF_HANDLE_EXCEPT	Handle processor exceptions

		ALERT we must interface with the Control Panel here.

        17 - 31 Reserved
    */
    MCFLAGS         Flags;              /* Main settings flags */

    unsigned short  MaxMem;             /* Max mem for tree usage, in K */
    unsigned short  NearNullBytes;      /* bytes to check in near null */
    unsigned short  FarNullBytes;       /*  "    "    "    " far   "   */
    unsigned char   CheckByteCt;        /* check byte count */
    unsigned char   AlignSize;          /* alignment boundary size */
    char            TrackingDir[36];    /* Rocket stores temp files here */
    unsigned short  DZBytes;            /* width of danger zone if stk track */
    unsigned short  DZScanFreq;         /* frequency of intensive stack check */

    } MCSETTINGS, *MCSETTINGSP;


/* Random 32-bit .CFG file sentinel */
#define MC_CFG_FILE_SENTINEL        ( (unsigned long) 0x10F23BC4 )

typedef struct MCCfgInfo {

    unsigned long   sentinel;           /* always MC_CFG_FILE_SENTINEL */
    MCSETTINGS      MemCheckSettings;   /* saved by user */

    } MCCFGINFO, *MCCFGINFOP;


/* *** Exception Handling *** */

/*
    MCEXCEPTLIST is used by MemCheck to hold exception handler's addresses.
*/
typedef struct {
    unsigned char   xNum;           /* exception number     */
    unsigned short  xPrevCs;        /* previous handler's cs */
    unsigned        xIp;            /* previous handler's ip */
    } MCEXCEPTLIST;

#ifndef _CC32_

typedef struct regdump_tag
{
    unsigned int ax;
    unsigned int bx;
    unsigned int cx;
    unsigned int dx;
    unsigned int si;
    unsigned int di;
	unsigned int bp;
	unsigned int sp;
	unsigned short	cs;
	unsigned short	ds;
	unsigned short	es;
	unsigned short  ss;
    unsigned short flags;
} MCREGS;
/* 16-bit exception stack */
typedef struct {

    unsigned    xRetIP;
    unsigned    xRetCS;
    unsigned    xErr;
    unsigned    xIP;
    unsigned    xCS;
    unsigned    xFlags;
    unsigned    xSP;
    unsigned    xSS;

    } MCEXCEPTINFO;

#else
typedef struct regdump_tag
{
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int esi;
    unsigned int edi;
	unsigned int ebp;
	unsigned int esp;
	unsigned short	cs;
	unsigned short	ds;
	unsigned short	es;
	unsigned short	fs;
	unsigned short  gs;
	unsigned short  ss;
    unsigned int flags;
} MCREGS;

/* 32-bit exception stack */
typedef struct {

    unsigned long   xRetEIP;
    unsigned short  xRetCS;
    unsigned short  xRsvd1;
    unsigned long   xErr;
    unsigned long   xEIP;
    unsigned short  xCS;
    unsigned short  xRsvd2;
    unsigned long   xFlags;
    unsigned long   xESP;
    unsigned short  xSS;
    unsigned short  xRsvd3;

    } MCEXCEPTINFO;
#endif  /* _CC32_ */

/* Values for MCCRITSECT.action */
#define MCCS_ENTER_SECTION  0
#define MCCS_LEAVE_SECTION  1

#define MCCS_ACTION(pMCCS)  (pMCCS->nAction)
#define MCCS_ENTER(pMCCS)   ((*(pMCCS->pLocked))++)   /* inc flag */
#define MCCS_LEAVE(pMCCS)   ((*(pMCCS->pLocked))--)   /* dec flag */

/* Critical section object - ptr to this passed to crit sect callback */
typedef struct {
    int             nAction;        /* MCCS_ENTER/LEAVE_SECTION */
    int *           pLocked;        /* # times entered */
    unsigned long   ulRsvd;     	/* internal use */
    } MCCRITSECT;

#pragma pack()

#define MC_FEXIT    ( (MCID) 0xFF )


/* Error Reporting Function typedef */
#ifndef _ERF_DEFINED
#define _ERF_DEFINED
typedef void (_MCCALLBACK *ERF) (char *);
#endif


/* *** Callback Functions *** */

/* Interception callback (on every interception) */
typedef void (_MCCALLBACK * GLOBALF) (void);

/* Called whenever nodes added to or deleted from MC database */
typedef void (_MCCALLBACK *TRACKF) (int, MEMRECP);

/* User-definable check function to add to transfer checking */
typedef void (_MCCALLBACK * CHECKF) (
                int ,           /* 0 or MCE_... error val for this xfer op */
                void _MCFAR *,  /* user ptr dest */
                long            /* bytes to copy to dest */
            );

/* Funcs called at startup or shutdown */
typedef void (_MCCALLBACK *STARTF) (void);
typedef void (_MCCALLBACK *ENDF)   (void);

/* Report function type passed to mc_report() */
typedef void (_MCCALLBACK *REPORTF) (MEMRECP);

/* Additional heap pointer verification (troubleshoot only) */
typedef int (_MCCALLBACK *VERIFYF) (void _MCFAR *);

typedef void (*MCVOIDFP) (void);

/* Exception handler */
typedef void (_MCCALLBACK _MCFAR *MCEXCEPTF) (void);

/* Stack Overflow handler */
typedef void (_MCCALLBACK _MCFAR *MCSTACKF) (void);

/* Multitasking enter/exit critical section callback.
   Specify as param to mc_set_critf() at beginning of program;
   callback function will be called with MCCS_ENTER_SECTION
   on entry to MemCheck, or MCCS_LEAVE_SECTION on exit.

   NOT TO BE CONFUSED WITH A MEMCHECK "GLOBAL" FUNCTION.
   Global functions (GLOBALF's) are used to perform any
   actions on the interception of a runtime function;
   CRITF's must be used ONLY to serialize access to MemCheck.
*/
typedef void (_MCCALLBACK * MCCRITF) (MCCRITSECT *);


/*  Stack Frame Handler 

    You can define your own function to 
    record, analyze, or inspect each stack frame
    when mc_stack_trace() is called.

    You *cannot* modify ANY of the values passed
    in, as the "const" typing indicates.  If you need to
    modify a value, make a copy.  See the MemCheck 3.5
    documentation for more details on stack frame handlers.
*/

typedef void (_MCFAR _MCCDECL *_SSFRAMEHANDLER ) (
            short    const ,        /* AX: near/far/error flag   */
            unsigned short const ,  /* CX: near (default) rtn CS */
            unsigned short const ,  /* ES: far rtn CS            */
            unsigned const ,        /* DI: rtn offset from stack */
            short const             /* DX: frame count           */
    );

/*  Values for "flag" constant parameter to a
    stack frame handler. 
*/
#define TRACE_BAD_FRAME     0x00        /* couldn't recognize frame     */
#define TRACE_FAR_CALL      0x01        /* frame represents a far call  */
#define TRACE_NEAR_CALL     0x02        /*   "        "     " near  "   */
#define TRACE_BAD_CHAIN     0x03        /* frame BP chewed up           */
#define TRACE_BEGIN         0x80        /* signals begin walk           */
#define TRACE_END           0x81        /* signals end of walk          */


/* MC Settings Structure, "flags" member: */
#define MCF_ACTIVE          (MCFLAGS)(0x01)
#define MCF_FAST_MODE       (MCFLAGS)(0x02)
#define MCF_PROTECTED_MODE  (MCFLAGS)(0x04)
#define MCF_FAR_NULL_CHECK  (MCFLAGS)(0x08)
#define MCF_NEAR_NULL_CHECK (MCFLAGS)(0x10)
#define MCF_STANDARD_STACK  (MCFLAGS)(0x20)
#define MCF_AUTOINIT        (MCFLAGS)(0x40)
#define MCF_CLEAR_ON_FREE   (MCFLAGS)(0x80)
#define MCF_DISK_ROCKET     (MCFLAGS)(0x100)
#define MCF_IDX_IN_MEMORY   (MCFLAGS)(0x200)
#define MCF_SOURCE_ONLY     (MCFLAGS)(0x400)
#define MCF_STACK_TRACK     (MCFLAGS)(0x800)
#define MCF_LOG_ON          (MCFLAGS)(0x1000)
#define MCF_LOG_FLUSH       (MCFLAGS)(0x2000)
#define MCF_CHECK_FILES     (MCFLAGS)(0x4000)
#define MCF_APV				(MCFLAGS)(0x8000)
#define MCF_HANDLE_EXCEPT	(MCFLAGS)(0x10000)

/* *** Conditional Compilation *** */

/* --------------------------------------------------------------
    If MEMCHECK is not being `compiled out' (via definition
    of the constant NOMEMCHECK), include this section...
-------------------------------------------------------------- */

#if !defined(MEMCHECK)

/* Include Section for `MemCheck Not Active' */

/*  *****************************
     MemCheck Not Active Section
    *****************************

    This section completely removes or
    "evaporates" all MemCheck function references
    from your projects when you compile with
    NOMEMCHECK #defined.

    There's no need to remove any MemCheck
    headers or statements from your code
    to produce a full production version
    of your application.

                    o
                   ooo
                 ooooooo
                ooooooooo
               ooooooooooo
                   ooo
                   ooo
                   ooo
                   ooo
                                    */

#ifndef MEMCHECK_MODULE

/* if you don't want notification when MemCheck is being compiled
	out (if NOMC is defined), just comment out the next lines. */
#if defined(_CC_MSC_COMPATIBLE_) 
#pragma message ("[MemCheck 3.5 compiled out]")
#endif

/*  Evaporate all MemCheck 3.5 API
    statements to do nothing, safely... */

#   define  mc_alloc_count()                0L
#   define  mc_blocks_allocated()           0L
#   define  mc_blocks_freed()               0L
#   define  mc_breakpoint(fi)               0
#   define  mc_bytes_allocated()            0L
#   define  mc_bytes_freed()                0L
#   define  mc_check(p)                     0
#   define  mc_check_buffers()              0
#   define  mc_cur_file()                   "No file"
#   define  mc_cur_line()                   0
#   define  mc_debug(s)
#   define  mc_debugf(_args)
#   define  mc_debug_on()
#   define  mc_debug_off()
#   define  mc_endcheck()                   (MCEFLAGS)0
#   define  mc_errno()                      MCE_NO_ERROR
#   define  mc_error_flags()                (MCEFLAGS)0
#   define  mc_error_text(e)                "MemCheck not active"
#   define  mc_except_text(e)               "MemCheck not active"
#   define  mc_file()                       "No file"
#   define  mc_find_buffer(p,mr)            0
#   define  mc_func()                       (MCID)0
#   define  mc_func_name(mcid)              ("")
#   define  mc_get_erf()                    (ERF)NULL
#   define  mc_get_mode()                   0
#   define  mc_get_speed()                  0
#   define  mc_in_source()                  0
#   define  mc_is_active()                  0
#   define  mc_line()                       0
#   define  mc_load_debugger()
#   define  mc_location_text()              "MemCheck not active"
#   define  mc_log_flush()                  
#   define  mc_log_writeln(_s)              
#   define  mc_memory_leaked()              0L
#   define  mc_memrec()                     (MEMRECP)NULL
#   define  mc_memrec_text()                "MemCheck not active"
#   define  mc_msg_continued()              0
#   define  mc_nullcheck()                  0
#   define  mc_null_snapshot() 
#   define  mc_register(p,s)    
#   define  mc_report(_f)
#   define  mc_set_alignsize(_s)
#	define  mc_set_apv(_toggle)				0
#   define  mc_set_breakfile(_f)
#   define  mc_set_checkbytes(_cb)
#   define  mc_set_checkf(_f)               (CHECKF)NULL
#   define  mc_set_critf(_f)                
#   define  mc_set_endf(erf)                (ENDF)NULL
#   define  mc_set_erf(erf)                 (ERF)NULL
#   define  mc_set_globalf(_f)              (GLOBALF)NULL
#   define  mc_set_globalexitf(_f)          (GLOBALF)NULL
#   define  mc_set_speed(_s)
#   define  mc_set_location()
#   define  mc_set_log_flush(val_0_or_1)    
#   define  mc_set_trackf(_f)               (TRACKF)NULL
#   define  mc_set_tracefile(_f)
#   define  mc_set_tree_order(_q)
#   define  mc_set_track_dir(_dir)          
#   define  mc_source_ptr()                 (MCPTR)NULL
#   define  mc_stack_trace(_memo)           0

#   define  mc_stack_length()               0
#   define  mc_stack_used()                 0
#   define  mc_stack_max_used()             0
#   define  mc_stack_free()                 0
#   define  mc_stack_unused()               0
#	define 	mc_stack_reserved()				0

#   define  mc_startcheck(_erf)
#   define  mc_unregister(p)                
#   define  mc_set_exceptf(_f)              ((MCEXCEPTF)NULL)
#   define  mc_get_exceptf()                ((MCEXCEPTF)NULL)
#   define  mc_set_stackf(_f)               ((MCSTACKF)NULL)
#   define  mc_get_stackf()                 ((MCSTACKF)NULL)

/* *** Stock error reporting functions *** */
#   define  erf_default(_msg)
#   define  erf_find_leaks(_msg)
#   define  erf_logfile(_msg)
#   define  erf_log_only(_msg)
#   define  erf_standard(_msg)
#   define  erf_stderr(_msg)
#   define  erf_stdout(_msg)
#   define  erf_trace(_msg)
#   define  erf_trace_all(_msg)
#   define  erf_trace_obj(_msg)

/* Internal Helpers */
#   define  _direct_output(_s)              
#   define  _mcsl(_f,_l)
#   define  _mcsl_delete(_f,_l)
#   define  _mcsl_new(_f,_l)
#   define  _mcslx(_f,_l,_s)
#   define  _mc_set_delflag()
#   define  _mc_set_location(_f,_l)         
#   define  _mc_set_newflag()

/* Link-time compileouts */
#   define  MC_BEGIN_EXCEPTLIST
#   define  MC_END_EXCEPTLIST
#   define  MC_HANDLE_EXCEPTION(e)
#   define  MC_SET_AUTOINIT(_toggle)
#	define  MC_SET_APV(_toggle)
#	define  MC_SET_CHECK_FILES(_toggle)
#   define  MC_SET_CHECK_FREQ(_freq)    
#   define  MC_SET_CHECK_STACK(_toggle)
#   define  MC_SET_CHECKF(_f)   
#   define  MC_SET_CRITF(_f)    
#	define  MC_SET_DLL_CFGFILE(_cfgfilename)
#   define  MC_SET_ENDF(_f) 
#   define  MC_SET_ENV_VAR(_envvarname)
#   define  MC_SET_ERF(_f)  
#   define  MC_SET_EXCEPTF(_f)
#   define  MC_SET_GLOBALF(_f)  
#   define  MC_SET_GLOBALEXITF(_f)  
#   define  MC_SET_LOGFILE(_logfilename)    
#   define  MC_SET_LOG(val_0_or_1)
#   define  MC_SET_LOG_FLUSH(val_0_or_1)
#   define  MC_SET_PANIC_BUFFERS(_q)
#	define  MC_SET_PERMIT_CODE_WRITES(_toggle)
#	define	MC_SET_SEARCH_HEAP(_toggle)
#   define  MC_SET_SSFH(_f) 
#   define  MC_SET_STARTF(_f)   
#   define  MC_SET_STACK_DANGER_ZONE(_val)
#   define  MC_SET_STACK_CHECK_FREQ(_val)
#   define  MC_SET_STACKF(_f)
#   define  MC_SET_TRACKF(_f)   
#   define  MC_SET_VERIFYF(_f)  
#	define  MC_SET_EXCEPTF(_f)
#	define  MC_ALWAYS_HANDLE_EXCEPTION(_toggle)

/* Back-end direct */
#define RTL(_f)     _f

/* *** C++ *** */
#ifdef __cplusplus

#define NEW(_object)        new _object
#define DELETE(_object)     delete _object
#define DELETE_ARR(_arr)    delete[] _arr

#define cpp_malloc(_s)      malloc(_s)
#define cpp_calloc(_n,_s)   calloc(_n,_s)
#define cpp_free(_p)        free(_p)

/* Borland C++ */
#define cpp_farmalloc(_s)   farmalloc(_s)
#define cpp_farfree(_fp)    farfree(_fp)

/* Microsoft C++-compatibles */
#define cpp__fmalloc(_s)    _fmalloc(_s)
#define cpp__ffree(_fp)     _ffree(_fp)

#endif  /* C++ */

#endif  /* not MEMCHECK_MODULE */


/* #*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*# */

#else   /* MEMCHECK is defined */

/* #*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*# */

#if defined(_CC_MSC_COMPATIBLE_) 
#pragma message ("MemCheck V3.5")
#endif

/*
    *************************
     MemCheck Active Section
    *************************

    The rest of this header file deals with
    MemCheck's being compiled into an application.

                                                   */

/* Specify that vars and funcs are straight C.. */
#ifdef __cplusplus
extern "C" {
#endif


/* *** ANSI Location Defines *** */

/*  Allow for the possibility of _MCSF_ being
    defined to reference a single, static module
    filename.  This prevents a multiplicity of 
    static filenames getting added to the DGROUP, e.g.

        static char *_thisfile = __FILE__;
        #define _MCSF_  ((MCSF)thisfile)
        #include <memcheck.h>

    This is only needed under MSC pre-VC++.
    Borland has "-d" Merge duplicate strings.
    VC++ has "/Gf" Elim duplicate strings.
*/
#if !defined (_MCSF_)
#   ifdef _CC_FILE_
#      define _MCSF_        _CC_FILE_
#   else
#      define _MCSF_        (MCSF)NULL
#   endif
#endif

#ifdef _CC_LINE_
#   define _MCSL_       (MCSL)_CC_LINE_
#else
#   define _MCSL_       (MCSL)0
#endif


/* *** Standard ANSI C Includes *** 

    For va_list function call args 
    Inclusion of this header won't change
    the behavior of host code.
*/
#if !defined (va_start)     /* avoid multiple inclusion... */
#   include <stdarg.h>
#endif


/* *** Compiler-specific includes *** */


/*lint -save -e537 Repeated include files (if necessary) */
#if defined(_CC_MSC_COMPATIBLE_)

#   if !defined (_INC_MALLOC)   /* C7.x and later optimization */
#       include <malloc.h>
#   endif

#   if !defined (_INC_STRING)   /* C7.x and later optimization */
#       include <string.h>
#   endif   

#elif defined(_CC_BORLAND_)

#   if !defined (__ALLOC_H)
#       include <alloc.h>           
#   endif

/* String functions must be proto'd before pragmas */
#   if !defined (__STRING_H)
#       include <string.h>
#   endif

#endif  /* Compiler-specific includes */
/*lint -restore */


/***************************************/
/* *** MemCheck 3.5 API Prototypes *** */
/***************************************/

/* Internal helper macro - proto shorthand */
#define _LOCP   MCSF,MCSL

extern  unsigned long _MCAPI mc_alloc_count (void);
extern  unsigned long _MCAPI mc_blocks_allocated (void);
extern  unsigned long _MCAPI mc_blocks_freed     (void);
extern  unsigned long _MCAPI mc_bytes_allocated  (void);
extern  unsigned long _MCAPI mc_bytes_freed      (void);
extern  int         _MCAPI mc_check (void _MCFAR *);
extern  int         _MCAPI mc_check_buffers (void);
extern  MCSF        _MCAPI mc_cur_file (void);
extern  MCSL        _MCAPI mc_cur_line (void);
extern  void        _MCCDECL mc_debugv (const char *, ...);
extern  void        _MCAPI mc_debug (const char *);
extern  MCEFLAGS    _MCAPI mc_endcheck (void);
extern  MCEFLAGS    _MCAPI mc_error_flags (void);
extern  char *      _MCAPI mc_error_text (int);
#define mc_errno()  _mc_errno()
/*  extern  int         _MCAPI mc_errno (void); */
extern  char *      _MCAPI mc_except_text (unsigned);
extern  MCSF        _MCAPI mc_file (void);
extern  int         _MCAPI mc_find_buffer(void _MCFAR *realptr,MEMRECP memrecp);
extern  MCID        _MCAPI mc_func (void);
extern  char *      _MCAPI mc_func_name(MCID);
extern  ERF         _MCAPI mc_get_erf (void);
extern  MCEXCEPTF   _MCAPI mc_get_exceptf (void);
extern  int         _MCAPI mc_get_mode (void);
extern  MCSTACKF    _MCAPI mc_get_stackf (void);
extern  int         _MCAPI mc_get_speed (void);
extern  char *      _MCAPI mc_get_tracefile (void);
extern  int         _MCAPI mc_in_source (void);
extern  int         _MCAPI mc_is_active (void);
extern  MCSL        _MCAPI mc_line (void);
extern  char *      _MCAPI mc_location_text (void);
#define  mc_load_debugger()     _asm int 3
extern  unsigned long _MCAPI mc_memory_leaked (void);
extern  char *      _MCAPI mc_memrec_text (MEMRECP);
extern  MEMRECP     _MCAPI mc_memrec (void);
extern  int         _MCAPI mc_msg_continued (void);
extern  int         _MCAPI mc_nullcheck (void);
extern  void        _MCAPI mc_null_snapshot (void);
extern  void        _MCAPI mc_register (void _MCFAR *, unsigned long);
extern  void        _MCAPI mc_report (REPORTF);
extern  void        _MCAPI mc_set_alignsize (unsigned int);
extern  char		_MCAPI mc_set_apv(char);
extern  void        _MCAPI mc_set_breakfile (char *);
extern  void        _MCAPI mc_set_checkbytes (unsigned int);
extern  CHECKF      _MCAPI mc_set_checkf (CHECKF);
extern  void        _MCAPI mc_set_critf (MCCRITF);
extern  ENDF        _MCAPI mc_set_endf (ENDF);
extern  ERF         _MCAPI mc_set_erf (ERF);
extern  MCEXCEPTF   _MCAPI mc_set_exceptf (MCEXCEPTF);
extern  GLOBALF     _MCAPI mc_set_globalf (GLOBALF);
extern  GLOBALF     _MCAPI mc_set_globalexitf (GLOBALF);
#define mc_set_location() _mc_set_location(_MCSF_,_MCSL_)
extern unsigned char _MCAPI mc_set_log_flush(unsigned char);
extern  MCPTR       _MCAPI mc_source_ptr (void);
extern  void        _MCAPI mc_set_speed (int);
extern  MCSTACKF    _MCAPI mc_set_stackf (MCSTACKF);
extern  void        _MCAPI mc_set_tracefile (char *);
extern  void        _MCAPI mc_set_track_dir (char *);
extern  TRACKF      _MCAPI mc_set_trackf (TRACKF);
extern  void        _MCAPI mc_set_tree_order (int);
extern  int         _MCAPI mc_stack_trace (char *);
extern  unsigned    _MCAPI mc_stack_info (int);

#define mc_stack_length()   mc_stack_info(_MCSTACK_LENGTH)
#define mc_stack_used()     mc_stack_info(_MCSTACK_USED)
#define mc_stack_max_used() mc_stack_info(_MCSTACK_MAX_USED)
#define mc_stack_unused()   mc_stack_info(_MCSTACK_UNUSED)
#define mc_stack_free()     mc_stack_info(_MCSTACK_FREE)
#define mc_stack_reserved()	mc_stack_info(_MCSTACK_RESERVED)
extern  void        _MCAPI mc_startcheck (_LOCP, ERF);
extern  void        _ATEXITFUNC mc_endcheck_at_exit (void);
extern  void        _MCAPI mc_unregister (void _MCFAR *);

/* Debugging versions of the MemCheck library only */
extern  void _MCAPI mc_debug_on  (void);
extern  void _MCAPI mc_debug_off (void);

/* --- Log control --- */

extern void _MCAPI mc_log_flush(void);
extern void _MCAPI mc_log_writeln(char *pszMsg);
extern int _MCCDECL mc_log_write(char *pszFmt,...);

/* *** INTERNAL API HELPERS *** */
extern void _MCAPI   _mc_set_location (_LOCP);
extern  int _MCAPI   _mc_errno (void);
extern void _FASTAPI _mcsl  (MCSF,MCSL);            /* location run-ahead */
extern void _FASTAPI _mcslx (MCSF,MCSL,size_t);     /* location run-ahead */
extern void _FASTAPI _mcsl_new (MCSF,MCSL);         /* location run-ahead */
extern void _FASTAPI _mcsl_delete (MCSF,MCSL);      /* location run-ahead */
extern void _FASTAPI _mc_set_newflag (void);        /* new's a'comin'     */
extern void _FASTAPI _mc_set_delflag (void);        /* delete's a'comin'  */

/* Misc - uses INT 9 to output directly to screen */
#if !defined (_CC32_)
extern void _MCCDECL _MCFAR _direct_output (char _MCFAR *);
#else
#define _direct_output(s)   printf ("%s\n", s)
#endif

/*
    mc_breakpoint() is now a MemCheck Tracking function (TRACKF).
    Tracking functions get called every time
    MemCheck adds or deletes from its database.
*/
#define mc_breakpoint(_f)   \
            mc_set_trackf( (mc_set_breakfile (_f), trackf_breakpoint) )
#define mc_breakpoint_trace(_f) \
            mc_set_trackf( (mc_set_tracefile (_f), trackf_breakpoint_trace) )


/* *** Exception handling evaporation for real mode *** */
#if !defined (_PROTECTED_)
#define     mc_set_exceptf(_f)  ((MCEXCEPTF)NULL)
#define     mc_get_exceptf()    ((MCEXCEPTF)NULL)
#endif

/* *** Advanced-user API extenders *** */

/*  extern  int     _MCAPI mc_find_buffer(void _MCFAR *, MEMRECP);  */
extern  int     _MCAPI mc_check_transfer(
                            void _MCFAR *,
                            void _MCFAR *,
                            unsigned long,
                            unsigned,
                            unsigned,
                            MEMRECP);

/*  mc_get_settings

    Write your own "get settings" routine
    to override the one shipped with MemCheck.
    You can hard-wire any settings you like, e.g.
    always ON for versions of your app shipped to 
    testers/QA stations, etc.
*/
extern void _MCCALLBACK mc_get_settings (MCSETTINGS *);


/* ***  Callbacks / Functionality Extenders *** 

    Function Type           Called...
    --------------          ------------------------------
    Error reporting         To handle each MemCheck error message 
    Global Interception     On each MemCheck interception
    Checking                On every data transfer check
    Tracking                On every allocation/deallocation
    Start                   On mc_startcheck or AutoInit
    End                     At mc_endcheck or MemCheck shutdown
    Exception Handler       On a handled processor exception
    Stack Overflow Handler  On stack entering Danger Zone
    Refer to your MemCheck 3.5 manual for further details.

    *** STOCK FUNCTIONS *** 
    These functions are available in the MemCheck
    libraries as "ready-made" for your programming 
    pleasure in the categories above.
*/

/* *** Stock error reporting functions *** */

extern  void    _MCCALLBACK erf_default     (char *);
extern  void    _MCCALLBACK erf_find_leaks  (char *);
extern  void    _MCCALLBACK erf_logfile     (char *);
extern  void    _MCCALLBACK erf_log_only    (char *);
extern  void    _MCCALLBACK erf_standard    (char *);
extern  void    _MCCALLBACK erf_stderr      (char *);
extern  void    _MCCALLBACK erf_stdout      (char *);
extern  void    _MCCALLBACK erf_trace       (char *);
extern  void    _MCCALLBACK erf_trace_all   (char *);
extern  void    _MCCALLBACK erf_trace_obj   (char *);

#define erf_printf  erf_stdout      /* alias*/

/* *** Stock Tracking Functions *** */

extern  void    _MCCALLBACK trackf_default          (int, MEMRECP);
extern  void    _MCCALLBACK trackf_all              (int, MEMRECP);
extern  void    _MCCALLBACK trackf_all_2            (int, MEMRECP);
extern  void    _MCCALLBACK trackf_breakpoint       (int, MEMRECP);
extern  void    _MCCALLBACK trackf_breakpoint_trace (int, MEMRECP);
extern  void    _MCCALLBACK trackf_big_alloc        (int, MEMRECP);

/* *** Stock End Functions *** */

extern  void    _MCCALLBACK endf_default    (void); /* does nothing */
extern  void    _MCCALLBACK endf_info       (void); /* write run info to log */
extern  void    _MCCALLBACK endf_alert      (void); /* warn if run errs */
extern  void    _MCCALLBACK endf_summary    (void); /* warn if run errs */

/* *** Stock Start functions *** */

extern  void    _MCCALLBACK startf_default  (void); /* does nothing */
extern  void    _MCCALLBACK startf_info     (void); /* write options to log */

/* *** Stock Check Functions *** */

extern  void    _MCCALLBACK checkf_default (int,void _MCFAR *,long);
extern  void    _MCCALLBACK checkf_dataseg (
    int,            /* 0 or MCE_... error val for this xfer op */
    void _MCFAR *,  /* user ptr dest */
    long            /* bytes to copy to dest */
    );
extern  void    _MCCALLBACK checkf_verify_heap (int,void _MCFAR *,long);
                            
/* *** Stock Global Interception Functions *** */

extern  void        _MCCALLBACK globalf_default       (void); /* does nothing */
extern  void        _MCCALLBACK globalf_supercheck    (void);
extern  void        _MCCALLBACK globalf_check_buffers (void);
extern  void        _MCCALLBACK globalf_heapcheck     (void);

/* *** Stock Report Functions *** */
extern  void        _MCCALLBACK reportf_default (MEMRECP);

/* *** Stock Exception Handlers *** */
extern void         _MCCALLBACK _MCFAR exceptf_default (void);

/* *** Stock Stack Overflow Handlers *** */
extern void         _MCCALLBACK _MCFAR stackf_default (void);

/* *** Stock Stack Frame Handlers *** */
extern void _MCFAR _MCCDECL ssfh_info (
    short const, unsigned short const, unsigned short const, unsigned const, short const);

extern void _MCFAR _MCCDECL ssfh_fast (
    short const, unsigned short const, unsigned short const, unsigned const, short const);
                /*  int         const   _flag,      */

extern void _MCFAR _MCCDECL ssfh_standard (
    short const, unsigned short const, unsigned short const, unsigned const, short const);

extern void _MCFAR _MCCDECL ssfh_debug (
    short const, unsigned short const, unsigned short const, unsigned const, short const);

/* *** End Stock Global Interception Functions *** */

/* *** Utility Stack Functions *** */
extern unsigned int _MCAPI mc_stacktop (void);  /* high address */
extern unsigned int _MCAPI mc_stackend (void);  /* low  address */
extern unsigned int _MCAPI mc_stacklen (void);  /* total length */

/* *** Function External Variables. *** */
/* 
    These are used effectively with MemCheck 3.5's AutoInit
    setting.  Under AutoInit, MemCheck fires itself up automatically
    before main() is reached.  Under these circumstances,
    there's no chance to have changed any defaults (like the
    ERF or error reporting function).  These variables provide
    a link-level method of setting these functions:

        #include <memcheck.h>
        :
        // Sets custom erf at link-time
        MC_SET_ERF (erf_custom_startup);
        :
*/
/* *** GLOBALS *** */

extern  ERF         _MCVAR MC_ERF;          /* error reporting func ptr     */
extern  CHECKF      _MCVAR MC_CheckF;       /* transfer check func          */
extern  MCCRITF     _MCVAR MC_CritF;        /* crit section enter/exit */
extern  GLOBALF     _MCVAR MC_GlobalF;      /* global interception callback */
extern  GLOBALF     _MCVAR MC_GlobalExitF;  /* called on exit interception */
extern  TRACKF      _MCVAR MC_TrackF;       /* alloc/dealloc callback       */
extern  MCSTACKF    _MCVAR MC_StackF;       /* called on stack overflow     */
extern  STARTF      _MCVAR MC_StartF;       /* startup callback             */
extern  ENDF        _MCVAR MC_EndF;         /* shutdown callback            */

extern  VERIFYF     _MCVAR MC_VerifyF;      /* troubleshooting              */

extern  char		_MCVAR MC_AlwaysHandleException;
extern  char *      _MCVAR MC_LogFile;      /* log file name used           */
extern  char        _MCVAR MC_UserAutoInit;
extern  char        _MCVAR MC_UserAPV;
extern  char		_MCVAR MC_SearchHeap;
extern  char		_MCVAR MC_PermitCodeWrite;
extern  char        _MCVAR MC_UserLogOn;
extern  char        _MCVAR MC_UserLogFlush;
extern  unsigned char _MCVAR MC_UserStackTrack; /* track stack usage */
extern  unsigned short _MCVAR MC_UserDZScanFreq;
extern  unsigned short _MCVAR MC_UserDZBytes; /* Stack DangerZone bytes */
extern  unsigned char _MCVAR MC_UserCheckFiles; /* track fopen/fclose */
extern  int         _MCVAR MC_CheckFreq;    /* for globalf_supercheck() et al */
extern  char *      _MCVAR MC_EnvVar;       /* Env var to detect 'active'   */
extern  unsigned short _MCVAR MC_DataSeg;   /* DS value                     */

extern  int         _MCVAR MC_nMaxTraceDepth;
extern  char *      _MCVAR MCST_Desc;       /* trace descrip to mc_..trc()  */

extern  MCSETTINGS  _MCVAR MC_DefaultSettings;  /* default settings         */
extern  MCSETTINGS  _MCVAR MC_Settings;         /* real settings--
                                                   USE WITH CARE!!!         */

extern  MCVOIDFP    _MCVAR MC_PMMap1;       /* p-mode func in map seg 1     */

/* Protected Mode Exception Handling */
extern  MCEXCEPTLIST    _MCVAR _MCFAR MC_ExceptList[];/* exceptions to handle-
                                    See Exception Handling Control below.   */
extern  MCEXCEPTINFO    _MCVAR MC_ExceptInfo;   /* in exception */
extern  MCREGS    		_MCVAR MC_ExceptRegs;   /* in exception */
extern  MCEXCEPTF       _MCVAR MC_ExceptF;      /* installed hdler */

/* Rocket Guidance Systems */
extern ROCKETALLOCF _MCVAR  MC_RocketAllocF;
extern ROCKETFREEF  _MCVAR  MC_RocketFreeF;
extern unsigned char _MCVAR MC_PanicBufCount;   /* anti-tree failure        */

/*  This char is used to fill freed buffers
    if the "ClearOnFree" option in effect.
    Default buffer clear char is 0.
*/
extern unsigned char _MCVAR MC_FreedBufferFillChar;

/*  Link-time defaults.

    These macros are "covers" to insulate you, the developer,
    from the underlying implementation, as well as to provide
    such bennies as compiling clean out of your code when
    NOMEMCHECK or NOMC is defined.  
    
    Use instead of accessing vars directly!

    To use, place the following in ONE MODULE e.g. your main module
    (any *one* module will work fine) after the MemCheck 
    header file has been included:

        #include <memcheck.h>
        MC_SET_...(params);

    For example, to change the default log file that MemCheck
    uses at runtime to C:\MYDEV\MYPROG.LOG:

        #include <memcheck.h>
        MC_SET_LOGFILE ("C:\\MYDEV\\MPROG.LOG");

    Most of these macros have runtime function equivalents,
    such as mc_set_erf() for MC_SET_ERF(), etc.  Notable
    exceptions to this are the following values that
    must generally have link-time initializations:

        MC_SET_LOGFILE()
        MC_SET_AUTOINIT()
        MC_SET_STARTF()
        MC_SET_STACK_DANGER_ZONE()
        MC_SET_CHECK_STACK()
*/
#define MC_SET_AUTOINIT(_toggle)        \
            char _MCVAR MC_UserAutoInit = (char)(_toggle);
#define MC_SET_APV(_toggle)	\
			char _MCVAR MC_UserAPV = (char)(_toggle);
#define MC_SET_DLL_CFGFILE(_cfgfilename)	\
			char * _MCVAR MC_DllCfgFile = (_cfgfilename)
#define MC_SET_CHECK_FILES(_toggle) \
			unsigned char _MCVAR MC_UserCheckFiles = \
				(unsigned char)(_toggle)
#define MC_SET_CHECKF(_f)   \
            CHECKF  _MCVAR MC_CheckF    = (_f)
#define MC_SET_CHECK_FREQ(_freq)        \
            int _MCVAR MC_CheckFreq     = (_freq)
#define MC_SET_CHECK_STACK(_toggle)     \
            unsigned char _MCVAR MC_UserStackTrack =    \
                (unsigned char)(_toggle)
#define MC_SET_CRITF(_f)    \
            MCCRITF _MCVAR MC_CritF     = (_f)
#define MC_SET_ENDF(_f)     \
            ENDF    _MCVAR MC_EndF      = (_f)
#define MC_SET_ENV_VAR(_envvarname)     \
            char *  _MCVAR MC_EnvVar    = (_envvarname)
#define MC_SET_ERF(_f)      \
            ERF     _MCVAR MC_ERF       = (_f)
#define MC_SET_GLOBALF(_f)  \
            GLOBALF _MCVAR MC_GlobalF   = (_f)
#define MC_SET_GLOBALEXITF(_f)  \
            GLOBALF _MCVAR MC_GlobalExitF = (_f)
#define MC_SET_LOGFILE(_f)      \
            char *  _MCVAR MC_LogFile   = (_f)
#define MC_SET_LOG(val_0_or_1) \
            char _MCVAR MC_UserLogOn = (unsigned char)(val_0_or_1)
#define MC_SET_LOG_FLUSH(val_0_or_1) \
            char _MCVAR MC_UserLogFlush = (unsigned char)(val_0_or_1)
#define MC_SET_PANIC_BUFFERS(_q)        \
            unsigned char _MCVAR MC_PanicBufCount = (_q)
#define MC_SET_PERMIT_CODE_WRITES(_toggle)	\
			char _MCVAR MC_PermitCodeWrite = (_toggle);
#define MC_SET_SEARCH_HEAP(_toggle)	\
			char _MCVAR MC_SearchHeap = (_toggle);
#define MC_SET_STACK_DANGER_ZONE(_q)            \
            unsigned short _MCVAR MC_UserDZBytes = (_q)
#define MC_SET_STACK_CHECK_FREQ(_q)         \
            unsigned short _MCVAR MC_UserDZScanFreq = (_q)
#define MC_SET_SSFH(_f)     \
            _SSFRAMEHANDLER _MCVAR near MC_SFrameHandler = (_f)
#define MC_SET_STARTF(_f)       \
            STARTF  _MCVAR MC_StartF    = (_f)
#define MC_SET_STACKF(_f)   \
            MCSTACKF    _MCVAR MC_StackF    = (_f)
#define MC_SET_TRACKF(_f)   \
            TRACKF  _MCVAR MC_TrackF    = (_f)
#define MC_SET_VERIFYF(_f)  \
            VERIFYF _MCVAR MC_VerifyF   = (_f)

#if defined (_PROTECTED_)
#define MC_SET_EXCEPTF(_f)      \
            MCEXCEPTF   _MCVAR MC_ExceptF = (_f)
#define MC_ALWAYS_HANDLE_EXCEPTION(_toggle)	\
			char _MCVAR MC_AlwaysHandleException= (_toggle)
#else
#define MC_SET_EXCEPTF(_f)
#define MC_ALWAYS_HANDLE_EXCEPTION(_toggle)
#endif

/* *** Exception Handling Control *** */

/*  Use the MC_BEGIN_EXCEPTLIST, MC_HANDLE_EXCEPTION,
    and MC_END_EXCEPTLIST macros to change the exceptions 
    MemCheck handles in protected mode by default.

    Usage (exactly as typed):
            #include <memcheck.h>
            :
            MC_BEGIN_EXCEPTLIST
                MC_HANDLE_EXCEPTION (0x0)
                MC_HANDLE_EXCEPTION (0xD)
            MC_END_EXCEPTLIST

    NOTE:
    To turn off MemCheck's exception handling completely, use 

            MC_SET_EXCEPTF(NULL);

    instead of trying to define an empty EXCEPTLIST...
*/

#if defined (_PROTECTED_)   /* only protected-mode development... */


#define MC_BEGIN_EXCEPTLIST     \
            MCEXCEPTLIST _MCVAR _MCFAR MC_ExceptList[] = {
#define MC_HANDLE_EXCEPTION(e)  \
                (unsigned char)(e), 0, 0, 
#define MC_END_EXCEPTLIST       \
                (unsigned char)0xFF,0,0};

#else                       /* Not a protected-mode program     */
#define MC_BEGIN_EXCEPTLIST
#define MC_HANDLE_EXCEPTION(e)
#define MC_END_EXCEPTLIST
#endif

/* ------------- End MemCheck 3.5 Library Calls --------------- */

/*  Formulaic rogue varargs interceptions;
    most host-code-compatible method...
    "Are you experienced?"

    "It is better to be mugged than
     to live in fear." - Anon.
*/
#define _VA_DEF(f,r,p)          \
        typedef r (_RTL *p_##f) p;  \
        extern p_##f _MCAPI _loc_##f (_LOCP);

/* Declare sprintf helper function */
_VA_DEF(sprintf,int,(char *, const char *, ...))

                               /* * * * * * * * * * * * * * * * * * * * * * *
    *************************
          Back-End RTL
    *************************
*/

/* *** Back-end functions *** */


/*  For the conversion that MSC underwent
    from C 6 to 7, where non-ANSI calls
    have underbars 
*/
#if defined (_CC_MSC_) && !defined (_CC_MSC6_) 
#if (_MSC_VER >= 700)
#   define _C7A
#endif
#endif

#ifdef _C7A
#define C7ANSI(func)    _##func
#else
#define C7ANSI(func)    func
#endif

#undef _C7A


/* ---------------------------------------------- */
/* These are the renamed ("zapped") RTL functions */
/* ---------------------------------------------- */

/*  Macro to access true back-end RTL.
    Used internally by the MemCheck API functions. 
*/
#define __paste(x,y)    x ## y
#define RTL(func)       __paste(func,_mc)

extern void * _RTL RTL(calloc)(size_t, size_t);
extern void _RTL RTL(free)(void *);
extern void * _RTL RTL(malloc)(size_t);
extern void * _RTL RTL(memcpy)(void *, const void *, size_t);
extern void * _RTL RTL(memmove)(void *, const void *, size_t);
extern void * _RTL RTL(memset)(void *, int, size_t);
extern void * _RTL RTL(realloc)(void *, size_t);
extern int _RTL RTL(vsprintf)(char *, const char *, va_list); 
extern char * _RTL RTL(strcat)(char *, const char *);
extern char * _RTL RTL(strcpy)(char *, const char *);
extern char * _RTL RTL(strncat)(char *, const char *, size_t);
extern char * _RTL RTL(strncpy)(char *, const char *, size_t);

/*	extern FILE * _RTL RTL(fopen)(const char *, const char *);	*/
/*	extern FILE * _RTL RTL(_fsopen)(const char *, const char *, int);	*/
/*	extern int  _RTL RTL(fclose)(FILE *);	*/
/*	extern FILE * _RTL RTL(freopen)(const char *, const char *, FILE *);	*/
extern int _RTL RTL(fcloseall)(void);
extern int _RTL RTL(_fcloseall)(void);

#if defined (_CC_MSC_) || defined (_CC_WATCOM_)
extern void _MCHFAR * _RTL RTL(_fcalloc)(size_t, size_t);
extern void _RTL RTL(_ffree)(void _MCHFAR *);
extern void _MCHFAR * _RTL RTL(_fmalloc)(size_t);
extern void _MCHFAR * _RTL RTL(_frealloc)(void _MCHFAR *, size_t);
extern size_t _RTL RTL(_fmsize)(void _MCHFAR *);
extern void _MCHFAR * _RTL RTL(_fexpand)(void _MCHFAR *, size_t);
#endif
#if defined (_CC_MSC_COMPATIBLE_) || defined (_CC_POWERPACK32_)
extern void * _RTL RTL(_expand)(void *, size_t);
#endif
#if defined (_CC_BORLAND4_)
extern size_t _RTL RTL(_msize)(void *);
#endif
#if !defined (_CC_POWERPACK32_)
extern void _MCHFAR * _MCFARGLUE _RTL RTL(_fmemmove)(void _MCHFAR *, const void _MCHFAR *, size_t);
extern void _MCHFAR * _MCFARGLUE _RTL RTL(_fmemcpy)(void _MCHFAR *, const void _MCHFAR *, size_t);
extern void _MCHFAR * _MCFARGLUE _RTL RTL(_fmemset)(void _MCHFAR *, int, size_t);
extern void _MCHFAR * _MCFARGLUE _RTL RTL(_fmemccpy)(void _MCHFAR *, const void _MCHFAR *, int, unsigned int);
extern char _MCHFAR * _MCFARGLUE _RTL RTL(_fstrcat)(char _MCHFAR *, const char _MCHFAR *);
extern char _MCHFAR * _MCFARGLUE _RTL RTL(_fstrncat)(char _MCHFAR *, const char _MCHFAR *, size_t);
extern char _MCHFAR * _MCFARGLUE _RTL RTL(_fstrcpy)(char _MCHFAR *, const char _MCHFAR *);
extern char _MCHFAR * _MCFARGLUE _RTL RTL(_fstrncpy)(char _MCHFAR *, const char _MCHFAR *, size_t); 
extern char _MCHFAR * _MCFARGLUE _RTL RTL(_fstrdup)(const char _MCHFAR *);
extern char _MCHFAR * _MCFARGLUE _RTL RTL(_fstrset)(char _MCHFAR *, int);
extern char _MCHFAR * _MCFARGLUE _RTL RTL(_fstrnset)(char _MCHFAR *, int, size_t);
#endif
#if !defined(_CC_BORLAND_)
extern void _RTL RTL(_nfree)(void near *);
extern void near * _RTL RTL(_nmalloc)(size_t);
extern void near * _RTL RTL(_ncalloc)(size_t, size_t);
extern void near * _RTL RTL(_nrealloc)(void near *, size_t);

extern void near * _RTL RTL(_nexpand)(void near *, size_t);
extern size_t _RTL RTL(_nmsize)(void _MCNEAR *);
extern char near * _MCHFAR _RTL RTL(_nstrdup)(const char _MCHFAR *);
#endif

#if !defined (_CC_POWERPACK32_)
#	if defined __cplusplus
#		if defined (_CC_BORLAND_)
extern void _RTL RTL(_dos_setvect)(unsigned, void __interrupt (_MCHFAR *)(...));
#		else	/* _CC_BORLAND_ */
extern void _RTL RTL(_dos_setvect)(unsigned, void (interrupt _MCHFAR *)(...));
#		endif
#	else	/* __cplusplus */
#		if defined (_CC_BORLAND_)
extern void _RTL RTL(_dos_setvect)(unsigned, void __interrupt (_MCHFAR *)());
#		else	/* _CC_BORLAND_ */
extern void _RTL RTL(_dos_setvect)(unsigned, void (interrupt _MCHFAR *)());
#		endif
#	endif /* __cplusplus */
#endif /* !PP32 */

#if !defined (_CC_WATCOM_)
extern char * _RTL RTL(_getdcwd)(int, char *, int);
#endif
extern char * _RTL RTL(getcwd)(char *, int);
#if defined (_MSC_VER)
#   if (_MSC_VER > 600)
extern void * _RTL RTL(_memccpy)(void *, const void *, int, size_t);
extern char * _RTL RTL(_strdup)(const char *);
extern char * _RTL RTL(_strset)(char *, int);
extern char * _RTL RTL(_strnset)(char *, int, size_t);
extern void _RTL RTL(_swab)(char *, char *, int);
extern char * _RTL RTL(_tempnam)(char *, char *);
extern char * _RTL RTL(_cgets)(char *);
extern char * _RTL RTL(_getcwd)(char *, int);
#endif
#endif
extern void * _RTL RTL(memccpy)(void *, const void *, int, unsigned int);
extern char * _RTL RTL(strdup)(const char *);
extern char * _RTL RTL(strset)(char *, int);
extern char * _RTL RTL(strnset)(char *, int, size_t);
extern void _RTL RTL(swab)(char *, char *, int);
extern char * _RTL RTL(cgets)(char *);
#if !defined (_CC_WATCOM_)
extern char * _RTL RTL(tempnam)(char *, char *);
#endif
#if defined (_CC_BORLAND_) && defined (_CC16_)
extern void _MCFAR * _RTL RTL(farmalloc)(unsigned long);
extern void _MCFAR * _RTL RTL(farrealloc)(void _MCFAR *, unsigned long);
extern void _RTL RTL(farfree)(void _MCFAR *);
extern void _MCFAR * _RTL RTL(farcalloc)(unsigned long, unsigned long);
#if defined __cplusplus
extern void _RTL RTL(setvect)(int, void interrupt(*)(...));
#else
extern void _RTL RTL(setvect)(int, void interrupt(*)());
#endif
extern void _MCFAR _RTL RTL(_fmovmem)(const void _MCFAR *, void _MCFAR *, unsigned);
extern void _MCHFAR _RTL RTL(_fsetmem)(void _MCHFAR *, unsigned, char);  
#endif
#if defined (_CC_BORLAND_)
extern void _RTL RTL(movmem)(const void *, void *, unsigned);
extern void _RTL RTL(setmem)(void *, unsigned, char);
extern char * _RTL RTL(stpcpy)(char *, const char *);
extern char * _RTL RTL(_stpcpy)(char *, const char *);
#endif
extern char * _RTL RTL(_fullpath)(char *, const char *, size_t); 
extern int _RTL RTL(sprintf)(char *, const char *, ...); 

/***************************************************************** 
 * --------   Function Call Interception Definitions   --------- * 
 *****************************************************************/


/*	#define _HOOK(_f)   _MCCDECL __paste(mc_,_f)	*/
#define _HOOK(_f)   _RTL __paste(mc_,_f)

#if (!defined (MEMCHECK_MODULE) || defined (INCODE_HOOK))

/*  Intrinsic Function Disabling

    It's important to disable function inlining for
    all intercepted functions.
*/

#if defined(_CC_MSC_)

/*  Intrinsics (== in-line functions) not permissible
    since they're implemented as macros...
*/
#pragma function(strcat)
#pragma function(strcpy)
#pragma function(memcpy)
#pragma function(memset)

#pragma function(strset)

#if defined(_MSC_VER)
#if (_MSC_VER >= 700)
#pragma function(_fmemcpy)
#pragma function(_fmemset)
#pragma function(_fstrcat)
#pragma function(_fstrcpy)
#pragma function(_fstrset)
#pragma function(_strset)
#endif
#endif /* defined _MSC_VER */

#elif defined(_CC_BORLAND_)

/* Turbo C/DPMI32 not like pragmae */
#if !defined (_CC_TCC_) && !defined (_CC_POWERPACK32_)

/*  Eliminate duplicate strings.
    This can save a bit of space in large
    programs particularly, since each call to 
    MemCheck references an otherwise separate
    copy of the current filename. 
*/
#pragma option -d

/*  Intrinsics (== in-line functions) not permissible
    since they're implemented as macros, for one...
*/
#pragma intrinsic -strcat
#pragma intrinsic -strncat
#pragma intrinsic -strcpy
#pragma intrinsic -strncpy
#pragma intrinsic -stpcpy
#pragma intrinsic -strset
#pragma intrinsic -strnset
#pragma intrinsic -memcpy
#pragma intrinsic -memset

#endif  /* not Turbo C and not DPMI32 */

/* end Borland compiler intrinsics */

#elif defined (_CC_WATCOM_)

/*  NOTE:  unfortunately, WATCOM C/C++ compilers
    force inlining of the strcpy() function regardless
    of whether you want it inlined or not, all the time.
    So this pragma, while it should ensure that
    strcpy() is a function call, does not...  :{

    So we take other measures below: see _mcwatcom_strcpy()
*/
#pragma function(strcpy)


#pragma function(strcat)
#pragma function(memcpy)
#pragma function(memset)

#pragma function(_fmemcpy)
#pragma function(_fmemset)
#pragma function(_fstrcat)
#pragma function(_fstrcpy)

#endif

/* End disable function inlining */

#endif /* !defined (MEMCHECK_MODULE) || defined (INCODE_HOOK) */

#ifndef MEMCHECK_MODULE
/*
    This section targets user's code only
*/

/* Func interceptors... */

/*
    MC_NO_TRANSFER_SIZE is used to eliminate errors or warnings
    like "sizeof returns 0" or "Not allowed type in sizeof <expr>".
    These occur for unsized variables declared like

        extern unsigned char gHelpString[];     

    The optimal solution is to "size" the extern, e.g.

        extern unsigned char gHelpString[80];   

    but where this may not be practical, MC_NO_TRANSFER_SIZE may
    be defined on a module-by-module OR project-wide basis.
*/
#ifdef MC_NO_XFER_SIZE      /* beta compat */
#   define  MC_NO_TRANSFER_SIZE
#endif
#ifdef NO_TRANSFER_SIZE     /* alternate */
#   define  MC_NO_TRANSFER_SIZE
#endif

#if defined (MC_NO_TRANSFER_SIZE)
#   define _XSIZEOF(d)      0
#else   /* standard; transmit sizeof dest */
#   define _XSIZEOF(d)      sizeof(d)
#endif /* if defined (MC_NO_TRANSFER_SIZE) */

#define _INTERCEPT0(_f)				_HOOK(_f)(_MCSF_, _MCSL_)
#define _INTERCEPT1(_f,p1)          _HOOK(_f)(p1, _MCSF_,_MCSL_)
#define _INTERCEPT2(_f,p1,p2)       _HOOK(_f)(p1,p2,_MCSF_,_MCSL_)
#define _INTERCEPT3(_f,p1,p2,p3)    _HOOK(_f)(p1,p2,p3,_MCSF_,_MCSL_)   
#define _INTERCEPT4(_f,p1,p2,p3,p4) _HOOK(_f)(p1,p2,p3,p4,_MCSF_,_MCSL_)
#define _INTERCEPTX1(_f,p1)         _HOOK(_f)(p1,_MCSF_,_MCSL_,_XSIZEOF(p1))
#define _INTERCEPTX2(_f,p1,p2)      _HOOK(_f)(p1,p2,_MCSF_,_MCSL_,_XSIZEOF(p1))
#define _INTERCEPTX3(_f,p1,p2,p3)   \
                    _HOOK(_f)(p1,p2,p3,_MCSF_,_MCSL_,_XSIZEOF(p1))
#define _INTERCEPTX4(_f,p1,p2,p3,p4)    \
                    _HOOK(_f)(p1,p2,p3,p4,_MCSF_,_MCSL_,_XSIZEOF(p1))


#define     _VA_INTERCEPT(_f)   (*_loc_##_f(_MCSF_,_MCSL_))
#define     _SETLOC(_f)         (mc_set_location(),_f)

/* This macro should be used only with "cover function" calls
	as described in the technote "xcover.txt".  DO NOT USE
	any of the _INTERCEPT3, etc. macros for this purpose. */
#define _INTERCEPT(_f)  (_mcsl(_MCSF_, _MCSL_),_f)

/* NOTE near _mcsl with #if (_MCC_NEAR_INTERCEPT == 0) */
extern void * _HOOK(calloc)(size_t, size_t, MCSF,MCSL);
extern void _HOOK(free)(void *, MCSF, MCSL);
extern void * _HOOK(malloc)(size_t, MCSF, MCSL);
extern void * _HOOK(memcpy)(void *,const void *,size_t,MCSF,MCSL,int);
extern void * _HOOK(memmove)(void *,const void *,size_t,MCSF,MCSL,int);
extern void * _HOOK(memset)(void *,int,size_t,MCSF,MCSL,int);
extern void * _HOOK(realloc)(void *, size_t, MCSF,MCSL);
extern int _HOOK(vsprintf)(char *,const char *,va_list,MCSF,MCSL,int); 
extern char * _HOOK(strcat)(char *,const char *,MCSF,MCSL,int);
extern char * _HOOK(strcpy)(char *,const char *,MCSF,MCSL,int);
extern char * _HOOK(strncat)(char *,const char *,size_t,MCSF,MCSL,int);
extern char * _HOOK(strncpy)(char *,const char *,size_t,MCSF,MCSL,int);

/*	extern FILE * _HOOK(fopen)(const char *,const char *,MCSF,MCSL);	*/
/*	extern FILE * _HOOK(_fsopen)(const char *,const char *, int, MCSF,MCSL);	*/
/*	extern int    _HOOK(fclose)(FILE *, MCSF,MCSL); 	*/
/*	extern FILE * _HOOK(freopen)(const char *,const char *,FILE *,MCSF,MCSL);	*/
extern int _HOOK(fcloseall)(void);
extern int _HOOK(_fcloseall)(void);

#if defined (_CC_MSC_) || defined (_CC_WATCOM_)
extern void _MCHFAR * _HOOK(_fcalloc)(size_t, size_t, MCSF,MCSL);
extern void _HOOK(_ffree)(void _MCHFAR *, MCSF, MCSL);
extern void _MCHFAR * _HOOK(_fmalloc)(size_t, MCSF, MCSL);
extern void _MCHFAR * _HOOK(_frealloc)(void _MCHFAR *, size_t, MCSF,MCSL);
extern size_t _HOOK(_fmsize)(void _MCHFAR *, MCSF, MCSL);
extern void _MCHFAR * _HOOK(_fexpand)(void _MCHFAR *, size_t, MCSF,MCSL);
#endif
#if defined (_CC_MSC_COMPATIBLE_) || defined (_CC_POWERPACK32_)
extern void * _HOOK(_expand)(void *, size_t, MCSF,MCSL);
#endif
#if defined (_CC_BORLAND4_)
extern size_t _HOOK(_msize)(void *, MCSF, MCSL);
#endif
#if !defined (_CC_POWERPACK32_)
extern void _MCHFAR * _MCFARGLUE _HOOK(_fmemmove)(void _MCHFAR *,const void _MCHFAR *,size_t,MCSF,MCSL,int);
extern void _MCHFAR * _MCFARGLUE _HOOK(_fmemcpy)(void _MCHFAR *,const void _MCHFAR *,size_t,MCSF,MCSL,int);
extern void _MCHFAR * _MCFARGLUE _HOOK(_fmemset)(void _MCHFAR *,int,size_t,MCSF,MCSL,int);
extern void _MCHFAR * _MCFARGLUE _HOOK(_fmemccpy)(void _MCHFAR *,const void _MCHFAR *,int,int,MCSF,MCSL,int);
extern char _MCHFAR * _MCFARGLUE _HOOK(_fstrcat)(char _MCHFAR *,const char _MCHFAR *,MCSF,MCSL,int);
extern char _MCHFAR * _MCFARGLUE _HOOK(_fstrncat)(char _MCHFAR *,const char _MCHFAR *,size_t,MCSF,MCSL,int);
extern char _MCHFAR * _MCFARGLUE _HOOK(_fstrcpy)(char _MCHFAR *,const char _MCHFAR *,MCSF,MCSL,int);
extern char _MCHFAR * _MCFARGLUE _HOOK(_fstrncpy)(char _MCHFAR *,const char _MCHFAR *,size_t,MCSF,MCSL,int); 
extern char _MCHFAR * _MCFARGLUE _HOOK(_fstrdup)(const char _MCHFAR *, MCSF, MCSL);
extern char _MCHFAR * _MCFARGLUE _HOOK(_fstrset)(char _MCHFAR *,int,MCSF,MCSL,int);
extern char _MCHFAR * _MCFARGLUE _HOOK(_fstrnset)(char _MCHFAR *,int,size_t,MCSF,MCSL,int);
#endif
#if !defined(_CC_BORLAND_)
extern void _HOOK(_nfree)(void near *, MCSF, MCSL);
extern void near * _HOOK(_nmalloc)(size_t, MCSF, MCSL);
extern void near * _HOOK(_ncalloc)(size_t, size_t, MCSF,MCSL);
extern void near * _HOOK(_nrealloc)(void near *, size_t, MCSF,MCSL);
extern void near * _HOOK(_nexpand)(void near *, size_t, MCSF,MCSL);
extern size_t _HOOK(_nmsize)(void _MCNEAR *, MCSF, MCSL);
extern char near * _MCHFAR _HOOK(_nstrdup)(const char _MCHFAR *, MCSF, MCSL);
#endif

#if !defined (_CC_POWERPACK32_)
#	if defined __cplusplus
#		if defined (_CC_BORLAND_)
extern void _HOOK(_dos_setvect)(unsigned, void __interrupt (_MCHFAR *)(...), MCSF,MCSL);
#		else	/* _CC_BORLAND_ */
extern void _HOOK(_dos_setvect)(unsigned, void (interrupt _MCHFAR *)(...), MCSF,MCSL);
#		endif
#	else /* cpp */
#		if defined (_CC_BORLAND_)
extern void _HOOK(_dos_setvect)(unsigned, void __interrupt (_MCHFAR *)(), MCSF,MCSL);
#		else	/* _CC_BORLAND_ */
extern void _HOOK(_dos_setvect)(unsigned, void (interrupt _MCHFAR *)(), MCSF,MCSL);
#		endif
#	endif /* cpp */
#endif /* pp32 */

#if !defined (_CC_WATCOM_)
extern char * _HOOK(_getdcwd)(int,char *,int,MCSF,MCSL,int);
#endif
extern char * _HOOK(getcwd)(char *,int,MCSF,MCSL,int);
#if defined (_MSC_VER_)
#if (_MSC_VER > 600)
extern void * _HOOK(_memccpy)(void *,const void *,int,int,MCSF,MCSL,int);
extern char * _HOOK(_strdup)(const char *, MCSF, MCSL);
extern char * _HOOK(_strset)(char *,int,MCSF,MCSL,int);
extern char * _HOOK(_strnset)(char *,int,size_t,MCSF,MCSL,int);
extern void _HOOK(_swab)(char *,char *,int,MCSF,MCSL,int);
extern char * _HOOK(_tempnam)(char *, char *, MCSF,MCSL);
extern char * _HOOK(_cgets)(char *, MCSF,MCSL,int);
extern char * _HOOK(_getcwd)(char *,int,MCSF,MCSL,int);
#endif
#endif
extern void * _HOOK(memccpy)(void *,const void *,int,int,MCSF,MCSL,int);
extern char * _HOOK(strdup)(const char *, MCSF, MCSL);
extern char * _HOOK(strset)(char *,int,MCSF,MCSL,int);
extern char * _HOOK(strnset)(char *,int,size_t,MCSF,MCSL,int);
extern void _HOOK(swab)(char *,char *,int,MCSF,MCSL,int);
extern char * _HOOK(cgets)(char *, MCSF,MCSL,int);
#if !defined (_CC_WATCOM_)
extern char * _HOOK(tempnam)(char *, char *, MCSF,MCSL);
#endif
#if defined (_CC_BORLAND_) && defined (_CC16_)
extern void _MCFAR * _HOOK(farmalloc)(unsigned long, MCSF, MCSL);
extern void _MCFAR * _HOOK(farrealloc)(void _MCFAR *, unsigned long, MCSF,MCSL);
extern void _HOOK(farfree)(void _MCFAR *, MCSF, MCSL);
extern void _MCFAR * _HOOK(farcalloc)(unsigned long, unsigned long, MCSF,MCSL);
#if defined __cplusplus
extern void _HOOK(setvect)(int, void interrupt(*)(...), MCSF,MCSL);
#else
extern void _HOOK(setvect)(int, void interrupt(*)(), MCSF,MCSL);
#endif
extern void _MCFAR _HOOK(_fmovmem)(const void _MCFAR *,void _MCFAR *,unsigned,MCSF,MCSL,int);
extern void _MCHFAR _HOOK(_fsetmem)(void _MCHFAR *,unsigned,char,MCSF,MCSL,int);  

#endif
#if defined (_CC_BORLAND_)
extern void _HOOK(movmem)(const void *,void *,unsigned,MCSF,MCSL,int);
extern void _HOOK(setmem)(void *,unsigned,char,MCSF,MCSL,int);
extern char * _HOOK(stpcpy)(char *,const char *,MCSF,MCSL,int);
extern char * _HOOK(_stpcpy)(char *,const char *,MCSF,MCSL,int);
#endif
extern char * _HOOK(_fullpath)(char *,const char *,size_t,MCSF,MCSL,int); 

#undef _HOOK
#define _HOOK(_f)   __paste(mc_,_f)

/*lint -save -e652 Define of symbol declared previously */
#if defined (NO_INTERCEPT)
#define MC_NO_INTERCEPT
#endif
/* *** ANSI Standard C *** */
#if !defined (MC_NO_INTERCEPT)
#define calloc(a,b) _INTERCEPT2(calloc,a,b)
#define free(a) _INTERCEPT1(free,a)
#define malloc(a) _INTERCEPT1(malloc,a)
#define memcpy(a, b, c) _INTERCEPTX3(memcpy, a, b, c)
#define memmove(a, b, c) _INTERCEPTX3(memmove, a, b, c)
#define memset(a, b, c) _INTERCEPTX3(memset, a, b, c)
#define realloc(a,b) _INTERCEPT2(realloc,a,b)
#define vsprintf(a, b, c) _INTERCEPTX3(vsprintf, a, b, c) 
#define strcat(a, b) _INTERCEPTX2(strcat, a, b)
#define strcpy(a, b) _INTERCEPTX2(strcpy, a, b)
#define strncat(a, b, c) _INTERCEPTX3(strncat, a, b, c)
#define strncpy(a, b, c) _INTERCEPTX3(strncpy, a, b, c)

/*	#define fopen(a, b) _INTERCEPT2(fopen, a, b)	*/
/*	#define _fsopen(a, b, c) _INTERCEPT3(_fsopen, a, b, c)	*/
/*	#define fclose(a)   _INTERCEPT1(fclose, a)	*/
/*	#define freopen(a, b, c) _INTERCEPT3(freopen, a, b, c)	*/

#if defined (_CC_MSC_) || defined (_CC_WATCOM_)
#define _fcalloc(a,b) _INTERCEPT2(_fcalloc,a,b)
#define _ffree(a) _INTERCEPT1(_ffree,a)
#define _fmalloc(a) _INTERCEPT1(_fmalloc,a)
#define _frealloc(a,b) _INTERCEPT2(_frealloc,a,b)
#define _fmsize(a) _INTERCEPT1(_fmsize,a)
#define _fexpand(a,b) _INTERCEPT2(_fexpand,a,b)
#endif
#if defined (_CC_MSC_COMPATIBLE_) || defined (_CC_POWERPACK32_)
#define _expand(a,b) _INTERCEPT2(_expand,a,b)
#endif
#if defined (_CC_BORLAND4_)
#define _msize(a) _INTERCEPT1(_msize,a)
#endif
#if !defined (_CC_POWERPACK32_)
#define _fmemmove(a, b, c) _INTERCEPTX3(_fmemmove, a, b, c)
#define _fmemcpy(a, b, c) _INTERCEPTX3(_fmemcpy, a, b, c)
#define _fmemset(a, b, c) _INTERCEPTX3(_fmemset, a, b, c)
#define _fmemccpy(a,b,c,d) _INTERCEPTX4(_fmemccpy,a,b,c,d)
#define _fstrcat(a, b) _INTERCEPTX2(_fstrcat, a, b)
#define _fstrncat(a, b, c) _INTERCEPTX3(_fstrncat, a, b, c)
#define _fstrcpy(a, b) _INTERCEPTX2(_fstrcpy, a, b)
#define _fstrncpy(a, b, c) _INTERCEPTX3(_fstrncpy, a, b, c) 
#define _fstrdup(a) _INTERCEPT1(_fstrdup,a)
#define _fstrset(a, b) _INTERCEPTX2(_fstrset, a, b)
#define _fstrnset(a, b, c) _INTERCEPTX3(_fstrnset, a, b, c)
#endif
#if !defined(_CC_BORLAND_)
#define _nfree(a) _INTERCEPT1(_nfree,a)
#define _nmalloc(a) _INTERCEPT1(_nmalloc,a)
#define _ncalloc(a,b) _INTERCEPT2(_ncalloc,a,b)
#define _nrealloc(a,b) _INTERCEPT2(_nrealloc,a,b)

#define _nexpand(a,b) _INTERCEPT2(_nexpand,a,b)
#define _nmsize(a) _INTERCEPT1(_nmsize,a)
#define _nstrdup(a) _INTERCEPT1(_nstrdup,a)
#endif
#if !defined (_CC_POWERPACK32_)
#define _dos_setvect(a,b) _INTERCEPT2(_dos_setvect,a,b)
#endif
#if !defined (_CC_WATCOM_)
#define _getdcwd(a, b, c) _INTERCEPTX3(_getdcwd, a, b, c)
#endif
#define getcwd(a, b) _INTERCEPTX2(getcwd, a, b)
#if defined (_MSC_VER_)
#if (_MSC_VER > 600)
#define _memccpy(a,b,c,d) _INTERCEPTX4(_memccpy,a,b,c,d)
#define _strdup(a) _INTERCEPT1(_strdup,a)
#define _strset(a, b) _INTERCEPTX2(_strset, a, b)
#define _strnset(a, b, c) _INTERCEPTX3(_strnset, a, b, c)
#define _swab(a, b, c) _INTERCEPTX3(_swab, a, b, c)
#define _tempnam(a,b) _INTERCEPT2(_tempnam,a,b)
#define _cgets(a) _INTERCEPTX1(_cgets, a)
#define _getcwd(a, b) _INTERCEPTX2(_getcwd, a, b)
#endif
#endif
#define memccpy(a,b,c,d) _INTERCEPTX4(memccpy,a,b,c,d)
#define strdup(a) _INTERCEPT1(strdup,a)
#define strset(a, b) _INTERCEPTX2(strset, a, b)
#define strnset(a, b, c) _INTERCEPTX3(strnset, a, b, c)
#define swab(a, b, c) _INTERCEPTX3(swab, a, b, c)
#define cgets(a) _INTERCEPTX1(cgets, a)
#if !defined (_CC_WATCOM_)
#define tempnam(a,b) _INTERCEPT2(tempnam,a,b)
#endif
#if defined (_CC_BORLAND_) && defined (_CC16_)
#define farmalloc(a) _INTERCEPT1(farmalloc,a)
#define farrealloc(a,b) _INTERCEPT2(farrealloc,a,b)
#define farfree(a) _INTERCEPT1(farfree,a)
#define farcalloc(a,b) _INTERCEPT2(farcalloc,a,b)
#define setvect(a,b) _INTERCEPT2(setvect,a,b)
#define _fmovmem(a, b, c) _INTERCEPTX3(_fmovmem, a, b, c)
#define _fsetmem(a, b, c) _INTERCEPTX3(_fsetmem, a, b, c);  
#define movmem(a, b, c) _INTERCEPTX3(movmem, a, b, c)
#define setmem(a, b, c) _INTERCEPTX3(setmem, a, b, c)
#endif
#if defined (_CC_BORLAND_)
#define stpcpy(a, b) _INTERCEPTX2(stpcpy, a, b)
#define _stpcpy(a, b) _INTERCEPTX2(_stpcpy, a, b)
#endif
#define _fullpath(a, b, c) _INTERCEPTX3(_fullpath, a, b, c) 



#if defined (_CC_WATCOM_)
#   undef strcpy    
#   define strcpy(d, s)     _mcwatcom_strcpy(d,s, _XSIZEOF(d), _MCSF_, _MCSL_)
    extern char * _RTL _mcwatcom_strcpy (char *, const char *, size_t, MCSF, MCSL);
#endif

/*
    Borland's Turbo Debugger gets confoosed and executes
    a `Run' instead of a `Step' when _INTERCEPT macro is used...
    Comment out the #define sprintf... to resolve this problem.
*/

/* There's a really good reason for this... */
#ifndef _lint
#define sprintf                 _INTERCEPT(sprintf)
#endif  

/* transfer functions that don't have destination as first argument... */
#undef swab
#undef _swab
#undef _getdcwd
#define swab(s, d, n)       _HOOK(swab)(s, d, n,_MCSF_,_MCSL_,_XSIZEOF(d))
#define _swab(s, d, n)      _HOOK(_swab)(s, d, n,_MCSF_,_MCSL_,_XSIZEOF(d))
#define _getdcwd(r, d, b)   _HOOK(_getdcwd)(r, d, b, _MCSF_, _MCSL_, _XSIZEOF(d))

/*  Location Transmitters

    You can add any non-intercepted functions to
    this bunch... Just updates MemCheck's file and line
    information via mc_set_location(), which is thousands
    of times faster than anything that does I/O.
    The only time this section could be a problem is
    if the header file is included before any other header
    files which prototype these routines.

    Borland's TD (Turbo Debugger) also has problems here (see note).
*/
#ifndef _lint   /* LINT not like */

/*  Borland's Turbo Debugger gets confoosed and executes
    a `Run' instead of a `Step' when _SETLOC macro is used...
*/
#if !defined (_CC_BORLAND_)
#if 1   /* Change this to '0' to omit this section */

#define printf              _SETLOC(printf)
#define fprintf             _SETLOC(fprintf)
#define fopen				_SETLOC(fopen)
#define fclose				_SETLOC(fclose)
#define fread               _SETLOC(fread)
#define fwrite              _SETLOC(fwrite)

#define system              _SETLOC(system)
#define exec                _SETLOC(exec)
#define spawnl              _SETLOC(spawnl)
#define spawnlp             _SETLOC(spawnlp)
#define spawnle             _SETLOC(spawnle)
#define spawnlpe            _SETLOC(spawnlpe)
#define spawnv              _SETLOC(spawnv)
#define spawnvp             _SETLOC(spawnvp)
#define spawnve             _SETLOC(spawnve)
#define spawnvpe            _SETLOC(spawnvpe)

#endif  /* end location transmission section */
#endif  /* not Borland C++ */
#endif  /* not def _lint */


/* ****  THIRD-PARTY MAPPINGS  **** */

/*  Vermont Views V3.xx

    The following code will transmit the exact file
    and line of any mem_get() and mem_free() calls to
    MemCheck, so that it can report on the location where
    these functions are called, instead of the location of
    the calloc() or free().

    If you've used MCCONFIG to configure the Vermont Views source 
    code, you *must* either NOT include the MemCheck header file 
    in the MEM_GET.C and MEM_FREE.C modules, or, if you do, then
    #define MC_NO_INTERCEPT beforehand, e.g.

         Module MEM_GET.C ... 
        :
        #define MC_NO_INTERCEPT
        #include <memcheck.h>
        :

    MCCONFIG may be used to configure even the shrouded
    Vermont Views source code.

    See also: TechNote "Using MemCheck 3.0 Professional
        With Vermont Views", available on the StratosWare
        BBS (313) 996-2993 as VIEWS.TXT, or by fax.
*/
#if defined (VV_SYS)    /* should do the trick */
#   define mem_get(s)   _INTERCEPT(mem_get(s))
#   define mem_free(p)  _INTERCEPT(mem_free(p))
#endif


/* ****  POWERPACK-SPECIFIC MAPPINGS  **** */
/* Borland's PowerPack 16 and 32 provide emulation for Windows memory
	allocation calls.  
*/
#if defined (MC_USE_WIN_ALLOCS)

HGLOBAL WINAPI mc_GlobalAlloc(UINT, DWORD, MCSF, MCSL);
HGLOBAL WINAPI mc_GlobalReAlloc(HGLOBAL, DWORD, UINT, MCSF, MCSL);
HGLOBAL WINAPI mc_GlobalFree(HGLOBAL, MCSF, MCSL);

DWORD   WINAPI mc_GlobalDosAlloc(DWORD, MCSF, MCSL);
UINT    WINAPI mc_GlobalDosFree(UINT, MCSF, MCSL);

#define GlobalAlloc(ui, dw)			_INTERCEPT2(GlobalAlloc, ui, dw)
#define GlobalReAlloc(hg, dw, ui)	_INTERCEPT3(GlobalReAlloc, hg, dw, ui)
#define GlobalFree(hg)				_INTERCEPT1(GlobalFree, hg)
#if defined (_CC_POWERPACK16_)
#define GlobalDosAlloc(dw)			_INTERCEPT1(GlobalDosAlloc, dw)
#define GlobalDosFree(ui)			_INTERCEPT1(GlobalDosFree, ui)
#endif
#endif /* MC_USE_WIN_ALLOCS */

/* ****  APPLICATION-SPECIFIC MAPPINGS  **** */

/*
    If your application uses allocation "cover" routines,
    MemCheck will by default report errors and leaks by
    the file and line of the malloc or free within the
    cover module.  To get MemCheck to report by file and
    line where the cover function is actually called, follow
    the instructtions in MemCheck TechNote "Transmitting File
    and Line to MemCheck 3.5 Professional Through Cover Functions."

    This is where you can place the cover function macros.
*/


/* end APPLICATION-SPECIFIC MAPPINGS */

#endif  /* not MC_NO_INTERCEPT */

/* Calls that xmit source file, line number if called in source */
/* *** MemCheck API file & line transmittal *** */
#define mc_startcheck(erf)      mc_startcheck(_MCSF_,_MCSL_,erf)
/*	#define mc_stack_info(_fl)      _INTERCEPT(mc_stack_info(_fl))	*/
#define mc_stack_trace(_memo)   _INTERCEPT(mc_stack_trace(_memo))
#define mc_debug(s)             _INTERCEPT(mc_debug(s))
#define mc_debugf(arglist)      mc_debugv arglist
#define mc_debugv               _mcsl(_MCSF_,_MCSL_),mc_debugv
#define mc_endcheck()           _INTERCEPT(mc_endcheck())
#define mc_check_buffers()      _INTERCEPT(mc_check_buffers())
#define mc_check(p)             _INTERCEPT(mc_check(p))
#define mc_register(p,s)        _INTERCEPT(mc_register(p,s))
#define mc_unregister(p)        _INTERCEPT(mc_unregister(p))
#define mc_nullcheck()          _INTERCEPT(mc_nullcheck())
#define mc_report(f)            _INTERCEPT(mc_report(f))
#define mc_set_checkbytes(u)	_INTERCEPT(mc_set_checkbytes(u))
#define mc_set_alignsize(u)		_INTERCEPT(mc_set_alignsize(u))

/*lint -restore  652 Define of symbol declared prev */

#endif  /* not MEMCHECK_MODULE, function interceptions */


/* End "C" call wrapper */
#ifdef __cplusplus
}


/*  C++ MemCheck Class

    This class can be used as an alternative to
    AutoInit, or to placing the mc_startcheck() and
    mc_endcheck() calls in your main() program.
    Just declaring an object of class 'MemCheck'
    will start MemCheck up;  usually you will place
    this 'above' any other global or statically declared
    C++ objects in your main module.

    Here are some examples of starting MemCheck up
    via object mechanics:

        MemCheck On;
        MemCheck Active;
        MemCheck Rules;

    Use your imagination!  Note that if AutoInit is ON,
    any calls to mc_startcheck() and mc_endcheck() are
    ignored.
*/
#if !defined (MC_NO_INTERCEPT)  /* must not have this def'd */

/* This class def causes a warning under MSC if not used */

/* remove warning about unused function */
#ifdef _CC_MSC_
    #if (_MSC_VER > 700)
        #pragma warning(disable:4505)
    #endif
#endif
class MemCheck {
public:
    MemCheck ()  { mc_startcheck (NULL); }
    ~MemCheck () { mc_endcheck ();       }
};
#endif /* MC_NO_INTERCEPT */


/* *** For use in new and delete modules only *** */
/*
    Replace 'mallocs' with 'cpp_mallocs', etc.
    In new and delete modules, MC_NO_INTERCEPT should be #defined, e.g.

        #define MC_NO_INTERCEPT
        #include <memcheck.h>
        :
        void * operator new ( size_t size )
        {
            if (!size)  size = 1;
            return (cpp_malloc (size));
        }
        etc.
*/
#define cpp_malloc(_s)      (_mc_set_newflag(),     malloc(_s))
#define cpp_calloc(_n,_s)   (_mc_set_newflag(),     calloc(_n,_s))
#define cpp_free(_p)        (_mc_set_delflag(),     free(_p))

/* Borland C++ */
#define cpp_farmalloc(_s)   (_mc_set_newflag(),     farmalloc(_s))
#define cpp_farfree(_fp)    (_mc_set_delflag(),     farfree(_fp))

/* Microsoft C++-compatibles */
#define cpp__fmalloc(_s)    (_mc_set_newflag(),     _fmalloc(_s))
#define cpp__ffree(_fp)     (_mc_set_delflag(),     _ffree(_fp))


/*
	It may be necessary to force the filename parameter to be a FAR pointer.
	The MFC overloaded new operator always passes file name as FAR, regardless
	of memory model.
	You can force a far pointer by using the compiler macro MC_MCSF.
*/
#if defined(_MC_MFC) || defined (MC_MCSF)
	typedef	MCSF	MCSF_NEW;
#else
	typedef	char*	MCSF_NEW;
#endif

/* C++ */
#if !defined (MC_NO_INTERCEPT)
#if !defined (MC_NO_CPP)

/*
	This method is now ON by default if the compile is C++.
      
	  void * new (size_t size, char *file, int lineno);

    Such a new operator is included in your SOURCE\CPP
    directory, and it is now (v3.5) linked into the MemCheck
	library as well.

    The substitutions for the new operator
    may not work in all situations.  To disable
    MemCheck's interception of new on a module-by-
    module basis, define MC_NO_NEW_OVERLOADED.
*/
#if defined (MC_NEW_OVERLOADED)

/*  Method 1: Placement Operators

    Use placement operators to trap file and line location transparently
    on calls to new.  
    
    Thanks for this tip to Dan Saks,
    C and C++ writer, author, teacher, and columnist--buy his books.
    He came through when no one else had a clue!

    Please consult your manual, MemCheck technotes, 
    or StratosWare Technical Support (1-800-WE-DEBUG)
    for details on how to configure your project for
    use with an overloaded new placement operator.
*/

/* Declare overloaded new with placement operators */
void *operator new (size_t _sz, MCSF_NEW file, int lineno);
#if defined (_CC_ANSI20_)
/*  Array version; only under supporting compilers 
    COMMENT LINE OUT if it causes a compile error.
*/
void *operator new[] (size_t _sz, MCSF_NEW file, int lineno);
#endif

#if defined(_CC_MSC_)
#   define  new new(_MCSF_,__LINE__)
#else   /* Borland */
#   define  new new((MCSF_NEW)_MCSF_,(int)__LINE__)
#endif

/* Backwards compatibility with the V2.1 C++ macros */
#define NEW(_object)         new _object
#define DELETE(_object)      delete _object
#define DELETE_ARR(_arr)     delete[] _arr

#else   /* !MC_NEW_OVERLOADED - end of Placement Operator intercept */

/*  New and Delete Interception, Method 2: NEW() and DELETE()

    The NEW() and DELETE() macros may be used to transmit file
    and line of new and delete.   These macros, which require
    modification of source code, i.e. "NEW(object)" for "new object",
    should probably be used only if the above overloaded new does
    not work for your code base.
    
    Please consult your manual, MemCheck technotes, 
    or StratosWare Technical Support (1-800-WE-DEBUG)
    for details on how to configure your project for
    use with NEW() and DELETE().

    If calling, please have your MemCheck serial number handy.
*/
#define NEW(_object)        (_mcsl_new(_MCSF_,_MCSL_),    new _object)
#ifndef DELETE  /* WINNT.H under BC DPMI32 defines DELETE */
#define DELETE(_object)     (_mcsl_delete(_MCSF_,_MCSL_), delete _object)
#endif
#define DELETE_ARR(_arr)    (_mcsl_delete(_MCSF_,_MCSL_), delete[] _arr)

#endif  /* !MC_NEW_OVERLOADED */

#define delete  _mcsl_delete(_MCSF_,_MCSL_), delete


/* *** FAILURES *** */

/*  These macros failed in the purpose of
    intercepting new transparently in some
    situation or other.
*/

/* Failed on " * new expr " (TV)  */
/*  #define new (mc_set_location(),0) ? NULL : new  */

/* Failed on " x = new Object " (TV) */
/*  #define new ((mc_set_location(),0) ? NULL : new)    */
/*  #define new new (mc_set_location(),0) ? NULL :  */

#endif  /* !MC_NO_CPP */
#endif  /* MC_NO_INTERCEPT */
#endif  /* cplusplus */
/******** End C++ ************/
#endif  /* End of Section for MEMCHECK Defined */
#endif  /* not already included */

/*******************************
   * End of MemCheck Header *
 *******************************/
