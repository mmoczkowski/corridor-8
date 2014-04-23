#ifndef RADMALI
#define RADMALI

#ifdef __RAD32__
  #include "malloc.h"

  #define radmalrad malloc
  #define radfrrad free
#else
  #ifdef __RADWIN__
    #ifdef RADStatus
      #define radmalrad(num) GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE,num)
      #define radfrrad GlobalFreePtr
    #else
      #define radmalloc(num) GlobalAllocPtr(GMEM_MOVEABLE|GMEM_SHARE,num)
      #define radfree GlobalFreePtr
    #endif
  #else
    #include "malloc.h"
    #define radmalrad(num) _fmalloc((int)num)
    #define radfrrad _ffree
  #endif
#endif

#ifndef radmalloc

#include "stdlib.h"

RCFUNC void PTR4* RADLINK radmalloc(u32 numbytes)
{
  void PTR4* temp;
  if (numbytes==0)
    return(0);
  temp=(numbytes!=0xffffffff)?radmalrad(numbytes):0;

#ifdef RADStatus
  if (!temp)
    StatusMessage("Not enough memory.",1);
#else
  #ifndef __RADWIN__  
    if (!temp) {
      DOSOut("  Not enough memory.\r\n");
      exit(16);
    }
  #endif
#endif
  return(temp);
}

RCFUNC void RADLINK radfree(void PTR4* ptr)
{
  radfrrad(ptr);
}

#endif

#endif
