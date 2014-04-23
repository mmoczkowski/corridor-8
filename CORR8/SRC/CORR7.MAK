#########################################################################
#
#       Build file for Corridor 7 game
#
#       02/27/96 Les Bird
#
#########################################################################

#       Make flags & paths

DBGVERS = 1

EXENAME = CORR7
OBJDIR  = ..\TARGET
RUNDIR  = ..\RUN
LIBDIR  = ..\LIBS

BUILD   = $(LIBDIR)\BUILD
SOS40   = $(LIBDIR)\SOS40
NETNOW  = $(LIBDIR)\NETNOW
MEMCHK  = $(LIBDIR)\MC35
SMKLIB  = $(LIBDIR)\SMACKER
PMPLIB  = $(LIBDIR)\PMPRO
SOSDLIB = $(SOS40)\LIB
NETNLIB = $(NETNOW)\LIB
MEMCLIB = $(MEMCHK)\LIB
SOSDINC = $(SOS40)\INCLUDE
NETNINC = $(NETNOW)\INCLUDE
MEMCINC = $(MEMCHK)\INCLUDE

.BEFORE
        @set INCLUDE=$(%WATCOM)\H;$(MEMCINC);$(BUILD);$(SOSDINC);$(NETNINC);$(SMKLIB);$(PMPLIB)
        @set DOS4G=QUIET

#       MAKE Directives

.ERASE
.SILENT

CC      = *WCC386
LINK    = WLINK
CFLAGS  = -w4 -zp4 -4r -s -mf -fo$(OBJDIR)\$^&
ASM     = WASM
AFLAGS  = -w4 -mf -fo$(OBJDIR)\$^&

!ifeq DBGVERS 1
DCFLAGS = -d2
DLFLAGS = DEBUG ALL
!endif

.EXTENSIONS:
.EXTENSIONS:    .LIB    .EXE    .OBJ    .C      .ASM

#       DEF file list

DEFLIST = CORR7.DEF &
        CORR7ACT.DEF &
        CORR7CFG.DEF &
        CORR7EFF.DEF &
        CORR7ENG.DEF &
        CORR7GAM.DEF &
        CORR7GFX.DEF &
        CORR7KBD.DEF &
        CORR7MEN.DEF &
        CORR7MUL.DEF &
        CORR7PLR.DEF &
        CORR7SMK.DEF &
        CORR7SND.DEF &
        CORR7TMR.DEF &
        CORR7WEP.DEF &
        READCFG.DEF

#       OBJ file list

OBJLIST = $(OBJDIR)\CORR7.OBJ &
          $(OBJDIR)\CORR7ACT.OBJ &
          $(OBJDIR)\CORR7ASM.OBJ &
          $(OBJDIR)\CORR7CFG.OBJ &
          $(OBJDIR)\CORR7EFF.OBJ &
          $(OBJDIR)\CORR7ENG.OBJ &
          $(OBJDIR)\CORR7GAM.OBJ &
          $(OBJDIR)\CORR7GFX.OBJ &
          $(OBJDIR)\CORR7KBD.OBJ &
          $(OBJDIR)\CORR7MEN.OBJ &
          $(OBJDIR)\CORR7MUL.OBJ &
          $(OBJDIR)\CORR7PLR.OBJ &
          $(OBJDIR)\CORR7SMK.OBJ &
          $(OBJDIR)\CORR7SND.OBJ &
          $(OBJDIR)\CORR7TMR.OBJ &
          $(OBJDIR)\CORR7WEP.OBJ &
          $(OBJDIR)\PROFILE.OBJ &
          $(OBJDIR)\READCFG.OBJ

ENGLIST = $(BUILD)\BUILD.OBJ &
          $(BUILD)\ENGINE.OBJ &
          $(BUILD)\CACHE1D.OBJ &
          $(BUILD)\A.OBJ &
          $(BUILD)\MULTI.OBJ

LIBS    = $(SOSDLIB)\SOSW1CR.LIB &
          $(NETNLIB)\NETNOWR.LIB &
          $(SMKLIB)\SMACK.LIB &
          $(PMPLIB)\PMODE.LIB &
          $(MEMCLIB)\MC35A3R.LIB &
          $(MEMCLIB)\RXA3R.LIB

#

.c.obj : .AUTODEPEND
        $(CC) $(CFLAGS) $(DCFLAGS) $<

.asm.obj : .AUTODEPEND
        $(ASM) $(AFLAGS) $<

#

ALL :   $(RUNDIR)\$(EXENAME).EXE

#

$(OBJDIR)\CORR7.OBJ : &
                CORR7.C         CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7ACT.OBJ : &
                CORR7ACT.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7ASM.OBJ : &
                CORR7ASM.ASM &
                CORR7.MAK

$(OBJDIR)\CORR7CFG.OBJ : &
                CORR7CFG.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7EFF.OBJ : &
                CORR7EFF.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7ENG.OBJ : &
                CORR7ENG.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7GAM.OBJ : &
                CORR7GAM.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7GFX.OBJ : &
                CORR7GFX.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7KBD.OBJ : &
                CORR7KBD.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7MEN.OBJ : &
                CORR7MEN.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7MUL.OBJ : &
                CORR7MUL.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7PLR.OBJ : &
                CORR7PLR.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7SMK.OBJ : &
                CORR7SMK.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7SND.OBJ : &
                CORR7SND.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7TMR.OBJ : &
                CORR7TMR.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(OBJDIR)\CORR7WEP.OBJ : &
                CORR7WEP.C      CORR7.H         ICORP.H &
                CORR7.MAK

$(RUNDIR)\$(EXENAME).EXE : $(OBJLIST) $(ENGLIST) $(EXENAME).LNK
        $(LINK) @$(EXENAME)

$(EXENAME).LNK : $(OBJLIST) $(ENGLIST)
        %create $^@
        %append $^@ SYSTEM DOS4G
        %append $^@ NAME $(RUNDIR)\$(EXENAME).EXE
        %append $^@ OPTION MAP
!ifeq DBGVERS 1
        %append $^@ $(DLFLAGS)
!endif
        for %i in ($(OBJLIST)) do %append $^@ FILE %i
        for %i in ($(ENGLIST)) do %append $^@ FILE %i
        for %i in ($(LIBS)) do %append $^@ LIB %i

