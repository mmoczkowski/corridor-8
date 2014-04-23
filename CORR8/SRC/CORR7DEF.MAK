#########################################################################
#
#       Build file for Corridor 7 DEF files
#
#       02/29/96 Les Bird
#
#########################################################################

#       Make flags & paths

SOS40   = ..\LIBS\SOS40
NETNOW  = ..\LIBS\NETNOW
MEMCHK  = ..\LIBS\MC35
SMKLIB  = ..\LIBS\SMACKER
PMPRO   = ..\LIBS\PMPRO
BUILD   = ..\LIBS\BUILD
SOSDLIB = $(SOS40)\LIB
NETNLIB = $(NETNOW)\LIB
MEMCLIB = $(MEMCHK)\LIB
SOSDINC = $(SOS40)\INCLUDE
NETNINC = $(NETNOW)\INCLUDE
MEMCINC = $(MEMCHK)\INCLUDE

.BEFORE
        @set INCLUDE=$(%WATCOM)\H;$(BUILD);$(MEMCINC);$(PMPRO);$(SOSDINC);$(NETNINC);$(SMKLIB)
        @set DOS4G=QUIET

#       MAKE Directives

.ERASE
.SILENT

CC      = *WCC386
LINK    = WLINK
CFLAGS  = -v -zs -zg -mf -or -4r -d__DOS__ -d_PROTOTYPING_

.EXTENSIONS:
.EXTENSIONS:    .LIB    .EXE    .DEF    .C

#       Definition file list

DEFLIST : CORR7.DEF &
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

#

.c.def :
        $(CC) $(CFLAGS) $<

#

all :   $(DEFLIST)

CORR7.DEF :     CORR7.C         ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7ACT.DEF :  CORR7ACT.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7CFG.DEF :  CORR7CFG.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7EFF.DEF :  CORR7EFF.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7ENG.DEF :  CORR7ENG.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7GAM.DEF :  CORR7GAM.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7GFX.DEF :  CORR7GFX.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7KBD.DEF :  CORR7KBD.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7MUL.DEF :  CORR7MUL.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7PLR.DEF :  CORR7PLR.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7SMK.DEF : &
                CORR7SMK.C      CORR7.H         ICORP.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7SND.DEF :  CORR7SND.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7TMR.DEF :  CORR7TMR.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

CORR7WEP.DEF :  CORR7WEP.C      ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

READCFG.DEF  :  READCFG.C       ICORP.H         CORR7.H &
                CORR7.MAK       CORR7DEF.MAK

