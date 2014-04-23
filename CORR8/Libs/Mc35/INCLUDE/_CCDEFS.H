/*
 * $History: _ccdefs.h $
 * 
 * *****************  Version 2  *****************
 * User: Keithb       Date: 6/11/96    Time: 2:03a
 * Updated in $/PE Mapper
 * Added CC_TOUCH
 * 
 * *****************  Version 1  *****************
 * User: Myk          Date: 5/31/96    Time: 9:22p
 * Created in $/MCW40
 */

/* #*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#

    StratosWare compiler definitions & constants.

    Copyright (c) 1990-1996, StratosWare Corporation.  
    All rights reserved.

    Modification History

    WHO     WHEN        WHAT
    KWB     04/12/95    Yanked from MEMCHECK.H
    MJW     05/25/95    Corrected DPMI16 SW_STACKLEN macro
    MJW     06/14/95    Retro-fit SW_STACKSLOP to STACKSLOP
    MJW     07/25/95    Changed Watcom STACKxxx decls to default
    MJW     01/07/96    Changed Borland STACKSLOP to 512
    KWB     02/21/96    Added _CC_MODEL_NAME_ 
    KWB     03/05/96    Added _CC_MFC_
    KWB     03/26/96    Added _CC_WIN32_
	MJW		04/30/96	Added _CC_BCTNT_ recognition
*/
#ifndef CCDEFS_H_
#define CCDEFS_H_

/*  Compiler Constant Definitions.
    _CC_WINDOWS_    Windows
    _CC_DOS_        Dos
    _CC_CPP_        C++ language accepted
    _CC_FILE_       Name of file being compiled (from __FILE__)
    _CC_LINE_       Alias for __LINE__ (for consistency)

    Compiler            Defines
    --------            -------
    Microsoft           _CC_MSC_, _CC_MSC_COMPATIBLE_
     VC++4.0            _CC_MSC10_
     VC++2.0            _CC_MSC9_
        V8.x            _CC_MSC8_
        V7.x            _CC_MSC7_
        V6.x            _CC_MSC6_
        V5.x            _CC_MSC5_

    Borland*            _CC_BORLAND_, _CC_BCC_
        V3.x            _CC_BORLAND3_
        V4.x            _CC_BORLAND4_
        PowerPack/16    _CC_POWERPACK_, _CC_POWERPACK16_
        PowerPack/32    _CC_POWERPACK_, _CC_POWERPACK32_, _CC32_

    WATCOM              _CC_WATCOM_, _CC_MSC_COMPATIBLE_
        WAT/386         _CC_WATCOM32_, _CC32_

    * Borland has no good way of determining compiler
      version.  __BORLANDC__ returns some truly funky
      hex constant that "will increase in future versions."

    Define              Meaning
    ------              --------
    _CC_NAME_           Text name of compiler, e.g. "Watcom C/C++386"
    _CC_MODEL_NAME_     Text name of ambient model, e.g. "Large", "Flat"
    _CC_VER_            Compiler's native version signature
    _CC32_ *            32-bit compile
    _CC16_              16-bit compile
    _PROTECTED_         16- or 32-bit protected mode compile
    _CC_LCODE_              Defined if large code model
    _CC_LDATA_              Defined if large data model
    SW_STACKTOP         Highest stack address 
    SW_STACKEND         Lowest stack address 
    SW_STACKLEN         Stack length (top - end)
    _CC_ANSI20_         Compiler supports C++ 2.0, e.g. new[]
    _CC_MFC_         	Using Microsoft Foundation Classes

    * If _CC32_ is defined, _PROTECTED_ is also defined
*/

#ifdef __cplusplus
#define _CC_CPP_
#endif
        
#ifdef _CC_CPP_
extern "C" {
#endif

/* Define ver, rev of this file.
   Use with #error if certain version required.
*/
#define _CCDEFS_VER_    2
#define _CCDEFS_REV_    0

#ifndef _CC_FILE_
#define _CC_FILE_ __FILE__
#endif
#ifndef _CC_LINE_
#define _CC_LINE_ __LINE__
#endif

/* Define platform:
    _CC_WINDOWS_    Windows
    _CC_DOS_        Dos
*/
#ifndef _CC_WINDOWS_
#ifndef _CC_DOS_

/* Temp var */
#	ifdef ISWIN_TMP
#	undef ISWIN_TMP
#	endif

#if defined(_Windows) || defined(WINDOWS) || defined(_WINDOWS)
#	define ISWIN_TMP
#endif
#if defined(_INC_WINDOWS) || defined(__WINDOWS_H) || defined(_WINDOWS_)
#	define ISWIN_TMP
#endif

#if defined(ISWIN_TMP)
#   if !defined (__DPMI16__) && !defined (__DPMI32__) && !defined (DOSX286)
#       define _CC_WINDOWS_
#   else
#       define _CC_DOS_
#   endif
#else
#   define _CC_DOS_
#endif

/* Free temp */
#undef ISWIN_TMP

#endif  /* not WINDOWS */
#endif  /* not DOS */


/*  NOTE: Microsoft C 5.x users have to "#define _MSC_VER 500"
    at the top of this file.
*/
#if defined(_MSC_VER)
/*
     _MSC_VER            Microsoft C version; currently defined as 700.
     M_I86   _M_I86      Member of the I86 processor family.
     M_I86mM _M_I86mM    Memory model type:
                         <m>= T            Tiny
                              S            Small (default)
                              C            Compact model
                              M            Medium model
                              L            Large model
                              H            Huge model
                         Identifiers defined by /AT, /AS, /AC, /AM,
                         /AL, and /AH, respectively.
     _MSDOS              MS-DOS operating system.
     _QC                 Microsoft QuickC Compiler.
     _WINDLL             Windows protected-mode dynamic-link library
                         is selected with /GD.
     _WINDOWS            Windows protected-mode is selected with /GA,
                         /Gn, /GW, /Mq, or /GD.
	_WIN32				Win32 environment.  Always defined >= VC++2.0
    _MFC_VER, _AFXRES,
    _AFXDLL, _USRDLL    Using Microsoft Foundation Class libraries
	_DLL				Win32 DLL.  Always defined >= VC++1.0
*/
#   define _CC_MSC_
#   define _CC_MSC_COMPATIBLE_

/*  #   define name2(x,v)  x ## #v  */
/*  #   define name(x,v)  name2(x,v)    */
/*  #   define _CC_NAME_  name("Microsoft C/C++ ", _MSC_VER)    */
#   define _CC_NAME_    "Microsoft C/C++"
#   define _CC_VER_     _MSC_VER

	/* See note above */
#   if (_MSC_VER >= 900) && defined(_WIN32)
#		define _CC32_
#		define _CC_WIN32_
#	endif

#   if (_MSC_VER >= 1000)
#       define _CC_MSC10_
#   elif (_MSC_VER >= 900)
#       define _CC_MSC9_
#   elif (_MSC_VER >= 800)
#       define _CC_MSC8_
#   elif (_MSC_VER >= 700)
#       define _CC_MSC7_
#   elif (_MSC_VER >= 600)
#       define _CC_MSC6_
#   elif (_MSC_VER >= 500)  /* see note above */
#       define _CC_MSC5_
#   else
#       error Unrecognized version of Microsoft compiler!
#   endif

#elif defined(__BORLANDC__)
#   define _CC_BORLAND_ /* Borland product */
#   define _CC_BCC_     /* Borland C compiler */
#   define _CC_VER_     __BORLANDC__

    /* Major compiler rev */
#   if (__BORLANDC__ >= 0x0450)
#       define _CC_NAME_    "Borland C/C++"
#       define _CC_BORLAND4_
#   elif (__BORLANDC__ >= 0x400)
#       define _CC_NAME_    "Borland C/C++"
#       define _CC_BORLAND3_
#   else
        /* not needed */
#   endif

    /* Borland 4.0 PowerPack BCC.EXE defines (-WX):
            __DPMI16__ _Windows

       With -WXD, -WXDE:
            __DLL__ __DPMI16__ _Windows

       Borland 4.0 PowerPack BCC32.EXE defines (-WX):
            __CONSOLE__ __DPMI32__ __FLAT__ __WIN32__ _Windows

       With -WXD:
            __DLL__ __CONSOLE__ __DPMI32__ __FLAT__ __WIN32__ _Windows
    */
#   if defined(__DPMI16__)
#       undef _CC_NAME_
#       define _CC_NAME_    "Borland PowerPack DPMI16"
#       define _CC_POWERPACK_
#       define _CC_POWERPACK16_
#       define _PROTECTED_
#   endif
#   if defined(__DPMI32__) || defined (_CC_BCTNT_)
#       undef _CC_NAME_
#       define _CC_NAME_    "Borland 32"
#       define _CC_POWERPACK_
#       define _CC_POWERPACK32_
#       define _CC32_       /* flat model */
#   endif

/* Turbo C++ */
#elif defined(MCTCP)    /* homebrew */
#   define _CC_BORLAND_ /* Borland product */
#   define _CC_TCP_     /* Turbo C++ */

#elif defined(__TURBOC__)
/*
        __TURBOC__      Gives the current Turbo C version
                        number, a hexadecimal number.  Version
                        1.0 id 0x1000; version 1.2 is 0x0102, etc.
        __TINY__, __SMALL__, __MEDIUM__, 
        __COMPACT__, __LARGE__, __HUGE__
                        Model defintions 
        __MSDOS__       Signals that we're not yet in the
                        twenty-first century
*/
#   define _CC_BORLAND_ /* Borland C product */
#   define _CC_TCC_     /* Turbo C/C++ compiler */
#   define _CC_NAME_    "Borland Turbo C"
#   define _CC_VER_     __TURBOC__


#elif defined(_INTELC32_)
/*
        _INTELC32_      has the value 1.
        _ARCHITECTURE_  is 386 if the nomod486 pragma is ineffect,
                        486 otherwise.
*/
#   define _CC_INTEL_       /* Intel Code Builder */
#   define _CC_MSC_COMPATIBLE_
#   define _CC32_           /* flat model */
#   define _CC_NAME_    "Intel CodeBuilder"
#   define _CC_VER_     1   /* all there ever was */

#elif defined(__WATCOMC__) 
/*  
        __WATCOMC__     Used to determine if the WATCOM C
                        or C/386 compiler is compiling.
                        Ver x 100 + rev, e.g. "850" for 8.5
        __386__         identifies the target machine as an
                        Intel 80386 under the WATCOM C/386 compiler
        MSDOS           Signals that we're not yet in the
                        twenty-first century
        __FLAT__, __SMALL__, __MEDIUM__, 
        __COMPACT__, __LARGE__  
                        Model defintions (flat is default)
                        Also id's MSC-compatible model defines

        8.5 and later:
        __WINDOWS__     Identifies 16-bit Windows ("zw" or "zW" opts)
        __WINDOWS_386__ 32-bit Microsoft Windows "zW" opt 
        __NETWARE_386__ Novell Netware 386, defined by the 
                        Novell/Watcom C 
        __OS2__         IBM OS/2-hosted version of Watcom 
*/
#   define _CC_WATCOM_      /* Watcom C product */
#   define _CC_VER_     __WATCOMC__
#   define _CC_MSC_COMPATIBLE_
#   ifdef __386__
#       define _CC32_       /* flat model */
#       define _CC_WATCOM32_
#       if (__WATCOMC__ >= 1000) 
#           define _CC_NAME_  "Watcom C/C++386"
#       else
#           define _CC_NAME_  "Watcom C 386"
#       endif
#   else
#       if (__WATCOMC__ >= 1000) 
#           define _CC_NAME_  "Watcom C/C++"
#       else
#           define _CC_NAME_  "Watcom C"
#       endif
#   endif

#elif defined(__STDC__)     /* Standard ANSI C */
#   define _CC_ANSI_
#   define _CC32_           /* no segmentation via far/near */
#   define _CC_NAME_  "ANSI compiler"
#   define far
#   define near
#   define huge
#   define cdecl

/* Avoids parameter mismatches from _far, etc. */
#   define _far
#   define _near
#   define _huge
#   define _cdecl

#else       /* UNSUPPORTED / UNRECOGNIZED COMPILER */

#error Unrecognized compiler 
/*
    If you're using Microsoft C5.1, you must
    define the constant _MSC_VER, e.g.

    #define _MSC_VER    500

    Place this statement at the top of the
    header file that includes this file
    or in your project header file.

    Note that Microsoft C 5.0 is NOT supported (it's non-ANSI).
*/
    
#endif  /* compiler constant definitions */

/* END of _CC..._ name setups; ripple & alias */

/* Sheer utility & avoidance of !_CC32_ */
#ifndef _CC32_
#   define _CC16_   
#endif

/* 32-bit compilers are always protected mode */
#ifdef _CC32_
#ifndef _PROTECTED_
#   define _PROTECTED_
#endif
#endif  /* CC32 */

/* Phar Lap support */
#ifdef DOSX286
#ifndef _PROTECTED_
#   define _PROTECTED_
#endif
#endif  /* DOSX286 */

/* C++ 2.0 compliance: _CC_ANSI20_ */
#if defined(_CC_BORLAND_)
    /* Only BC++ 4.x and later, but Borland has
       seriously mangled version info (__BORLANDC__) */
#   if defined (__BCPLUSPLUS__)
#       if (__BCPLUSPLUS__ > 0x0310)    /* after 3.1 */
#           define _CC_ANSI20_
#       endif
#   elif defined(_CC_POWERPACK_)
#           define _CC_ANSI20_
#   endif
#elif defined (_CC_MSC_)
    /* Current release through 8.x doesn't support new[] */
#elif defined (_CC_WATCOM_)
#   if (__WATCOMC__ >= 1000)    /* 10.x */
#           define _CC_ANSI20_
#   endif
#endif

/* MFC recognizance */
#if !defined(_CC_MFC_)	/* could be user-defined */
#	if defined(_CC_CPP_)	/* must be C++ to be MFC */
#  	if defined(_MFC_VER) || defined(_AFXRES) || defined(_AFXDLL) || defined(_USRDLL)
#      	define  _CC_MFC_	/* using Microsoft Foundation Classes */
#  	endif
#  	endif	/* C++ */
#endif

/*******************************************************************/
/*******************************************************************/

/*  *** Code and Data Size Constants *** 
    _CC_LCODE_ not used by this header file.
    _CC_LDATA_ *is* used, however.

    May be commented out if these constants are already defined.
*/

/*  #ifndef _CC_LCODE_  */
#if defined(M_I86MM) || defined(M_I86LM) || defined(M_I86HM)
#   define _CC_LCODE_   1
#elif defined(__MEDIUM__) || defined(__LARGE__) || defined(__HUGE__)
#   define _CC_LCODE_   1
#endif
/*  #endif  */

#if defined(__COMPACT__) || defined(__LARGE__) || defined(__HUGE__)
#   define _CC_LDATA_   1
#elif defined(M_I86CM) || defined(M_I86LM) || defined(M_I86HM)
#   define _CC_LDATA_   1
#endif

/* Macro "aliases" */

#ifndef _LCODE
#   ifdef _CC_LCODE_
#       define _LCODE   _CC_LCODE_
#   endif
#endif

#ifndef _LDATA
#   ifdef _CC_LDATA_
#       define _LDATA   _CC_LDATA_
#   endif
#endif


/* *** Model names *** */

/* Real mode or 16-bit p-mode */
#if defined(M_I86SM) || defined(__SMALL__)
#   define _CC_MODEL_NAME_  "Small"
#elif defined(M_I86MM) || defined(__MEDIUM__)
#   define _CC_MODEL_NAME_  "Medium"
#elif defined(M_I86CM) || defined(__COMPACT__)
#   define _CC_MODEL_NAME_  "Compact"
#elif defined(M_I86LM) || defined(__LARGE__)
#   define _CC_MODEL_NAME_  "Large"
#elif defined(M_I86HM) || defined(__HUGE__)
#   define _CC_MODEL_NAME_  "Huge"

/* 32-bit */
#elif defined(_CC32_)
#   define _CC_MODEL_NAME_  "Flat"

#else  /* unknown! */
#   define _CC_MODEL_NAME_  "Unknown!"
#endif /* model name definitions */

/* Warning message helpers 
   Usage:  #pragma _CCWARN("(MemCheck)", \
   			"MC_NEW_OVERLOADED must be defined in MFC projects"))
   Result:
      D:\DEV\WIN32\MFCTEST\MAINFRM.CPP(22) : warning (MemCheck) : xxxx

   Note: params to _CCWARN/_CCERROR *must* be quoted strings.
*/
#ifdef _CC_MSC_COMPATIBLE_
	#define _CCSTR(__s) #__s
	#define _CCIMSG(__f,__l,__msgclass,__msgtype,__msg) __f "(" _CCSTR(__l) ") : " __msgclass " " __msgtype " : " __msg

	#define _CCERROR(__msgtype,__msg) \
			message(_CCIMSG(__FILE__,__LINE__,"error",__msgtype,__msg))
	#define _CCWARN(__msgtype,__msg)  \
			message(_CCIMSG(__FILE__,__LINE__,"warning",__msgtype,__msg))
	#define _CCMSG(__msg)  message(__msg)
#else /* NOT MSC-compatible */
	#define _CCERROR(__msgtype,__msg) 
	#define _CCWARN(__msgtype,__msg)  
	#define _CCMSG(__msg)  
#endif

/*******************************************************************/
/*******************************************************************/

/* *** Physical Stack Address *** */

#if defined(_CC_BORLAND_)   /* -------------------------- */

    /*
    BORLAND RTL Notes:
    ;----------------------------------------------------------------------
    ; In large data models, the stack is in its own segment.  The stack
    ; starts at SS:__stklen and grows down to SS:0.
    ;
    ; In small data models, the stack is in the DGROUP, and grows down
    ; to meet the heap.  The end of the heap is marked by ___brklvl.
    (KWB: Note that the brklvl is adjusted upwards until it meets
    _stklen...)
    ;----------------------------------------------------------------------
    */

/*  #   define STACKSLOP    256 */
/* According to borland docs, a full 512 bytes are reserved for emulator
    data */
#   define STACKSLOP    512
#   define SW_STACKSLOP STACKSLOP
    extern unsigned cdecl _stklen;
#   if defined(_CC_POWERPACK_)
#       define  SW_STACKLEN (mc_stacklen())
#       define  SW_STACKTOP (mc_stacktop())
#       define  SW_STACKEND (mc_stackend())
#   else /* not P-Pack */
#   define  SW_STACKLEN (_stklen)
#   ifdef _CC_LDATA_
            /*  Compact, Large, Huge Models ...

                The stack starts at SS:_stklen and
                grows downward to SS:0
            */
#           define  SW_STACKTOP ((unsigned) _stklen)
#           define  SW_STACKEND ((unsigned) 0 + STACKSLOP)

#   else
        /*  Small, Medium Models ...

            The stack starts at SS:0xFFFE and grows
            downwards _stklen bytes.
        */
#       define  SW_STACKTOP ((unsigned) 0xFFFE)
#       define  SW_STACKEND (SW_STACKTOP - _stklen + STACKSLOP)
#   endif
#   endif   /* not PowerPack */

#elif defined (_CC_MSC_)  /* ------------------------------- */

    extern char cdecl end;              /* end of stack */
    extern unsigned cdecl _atopsp;      /* top of stack */

#   define  SW_STACKTOP _atopsp
#   define  STACKSLOP   256
#   define  SW_STACKSLOP    STACKSLOP
#   define  SW_STACKLEN (SW_STACKTOP - ((unsigned)(char near*)&end))

#   ifdef _CC_LDATA_
        /* If in far data, stack could be in its own
            seg. tho not usually. see /FARSTACK */
#       define  SW_STACKEND ( ((unsigned)((char near*)&end)) + STACKSLOP)
#   else
        /*  If near data, must be in DS; use near ptr */
#       define  SW_STACKEND ((unsigned)&end + STACKSLOP)
#   endif

#elif defined (_CC_WATCOM_)  /* ------------------------------- */
/* Note that only WC10 and up accept cdecl as a modifier. */
#if defined (_CC32_) && (!defined (_CC_ANSI20_))
    extern unsigned STACKLOW;
    extern unsigned STACKTOP;
#   define SW_STACKTOP  (STACKTOP)
#ifndef STACKSLOP
#   define STACKSLOP    0
#endif
#   define SW_STACKSLOP STACKSLOP
#   define SW_STACKEND  ((unsigned) (STACKLOW + STACKSLOP))
#   define SW_STACKLEN  (STACKTOP - STACKLOW)
#else
    extern unsigned _STACKLOW;      /* end of stack */
    extern unsigned _STACKTOP;      /* top of stack */
#   define  SW_STACKTOP (_STACKTOP)
#ifndef STACKSLOP
#   define  STACKSLOP   0                   /* no slop in WC */
#endif
#   define  SW_STACKSLOP    STACKSLOP 
#   ifdef _CC_LDATA_
#       define  SW_STACKEND ((unsigned) (_STACKLOW + STACKSLOP))
#   else
#       define  SW_STACKEND ((unsigned) (_STACKLOW + STACKSLOP))
#   endif
#   define  SW_STACKLEN (SW_STACKTOP-_STACKLOW)
#endif /* _CC32_ */

#else                   /* Unknown compiler ---------------- */

#error Unknown compiler 

#endif      /* defining stack top, end */


/* -- Useful pragmas --- */

/* Use with #pragma PRAGMA_xxx */
#if defined(_CC_MSC_)
/* ...: warning C4100: 'nLine' : unreferenced formal parameter */
#   define PRAGMA_NOARGS warning(disable:4100)
#   define PRAGMA_MSG(s) message(s)
#elif defined(_CC_BORLAND_)
#   define PRAGMA_NOARGS argsused
#   define PRAGMA_MSG(s)
#endif

/* -- Close off file --- */

#ifndef _CC_NAME_
#   define _CC_NAME_    "<unknown>"
#endif
#ifndef _CC_VER_
#   define _CC_VER_     0
#endif

#if !defined(CC_TOUCH)

	/*
	 *  'TOUCH' macro so high warning levels don't generate
	 *  'unreferenced variable' warnings.
	 */
	#if defined (_CC_BORLAND_) || defined (_CC_WATCOM_)
	#   define CC_TOUCH(var)   (var)=(var)
	#else
	#   define CC_TOUCH(var)   if (var)
	#endif

#endif


#ifdef _CC_CPP_
}
#endif

#endif  /* already #included */

/* End */
