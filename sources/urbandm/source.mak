# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=source - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to source - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "source - Win32 Release" && "$(CFG)" != "source - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "source.mak" CFG="source - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "source - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "source - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "source - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "source - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "source_0"
# PROP BASE Intermediate_Dir "source_0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "source_0"
# PROP Intermediate_Dir "source_0"
# PROP Target_Dir ""
OUTDIR=.\source_0
INTDIR=.\source_0

ALL : "$(OUTDIR)\gamex86.dll"

CLEAN : 
	-@erase "..\gamex86.dll"
	-@erase ".\source_0\g_monster.obj"
	-@erase ".\source_0\m_berserk.obj"
	-@erase ".\source_0\m_float.obj"
	-@erase ".\source_0\p_trail.obj"
	-@erase ".\source_0\m_medic.obj"
	-@erase ".\source_0\m_tank.obj"
	-@erase ".\source_0\a_mod.obj"
	-@erase ".\source_0\laser.obj"
	-@erase ".\source_0\m_chick.obj"
	-@erase ".\source_0\m_flipper.obj"
	-@erase ".\source_0\g_target.obj"
	-@erase ".\source_0\m_soldier.obj"
	-@erase ".\source_0\g_main.obj"
	-@erase ".\source_0\g_func.obj"
	-@erase ".\source_0\m_insane.obj"
	-@erase ".\source_0\m_brain.obj"
	-@erase ".\source_0\p_weapon.obj"
	-@erase ".\source_0\m_boss31.obj"
	-@erase ".\source_0\m_hover.obj"
	-@erase ".\source_0\g_trigger.obj"
	-@erase ".\source_0\m_move.obj"
	-@erase ".\source_0\m_flash.obj"
	-@erase ".\source_0\g_save.obj"
	-@erase ".\source_0\m_flyer.obj"
	-@erase ".\source_0\g_utils.obj"
	-@erase ".\source_0\m_parasite.obj"
	-@erase ".\source_0\m_supertank.obj"
	-@erase ".\source_0\g_ai.obj"
	-@erase ".\source_0\g_phys.obj"
	-@erase ".\source_0\g_items.obj"
	-@erase ".\source_0\p_hud.obj"
	-@erase ".\source_0\m_boss3.obj"
	-@erase ".\source_0\g_weapon.obj"
	-@erase ".\source_0\g_combat.obj"
	-@erase ".\source_0\m_infantry.obj"
	-@erase ".\source_0\urban_chasecam.obj"
	-@erase ".\source_0\p_view.obj"
	-@erase ".\source_0\g_svcmds.obj"
	-@erase ".\source_0\g_turret.obj"
	-@erase ".\source_0\g_misc.obj"
	-@erase ".\source_0\m_actor.obj"
	-@erase ".\source_0\m_boss2.obj"
	-@erase ".\source_0\m_gladiator.obj"
	-@erase ".\source_0\g_spawn.obj"
	-@erase ".\source_0\m_gunner.obj"
	-@erase ".\source_0\urban_locational.obj"
	-@erase ".\source_0\m_mutant.obj"
	-@erase ".\source_0\g_cmds.obj"
	-@erase ".\source_0\p_client.obj"
	-@erase ".\source_0\q_shared.obj"
	-@erase ".\source_0\m_boss32.obj"
	-@erase ".\source_0\gamex86.lib"
	-@erase ".\source_0\gamex86.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/source.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\source_0/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/source.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../gamex86.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/gamex86.pdb" /machine:I386 /def:".\game.def"\
 /out:"../gamex86.dll" /implib:"$(OUTDIR)/gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)/g_monster.obj" \
	"$(INTDIR)/m_berserk.obj" \
	"$(INTDIR)/m_float.obj" \
	"$(INTDIR)/p_trail.obj" \
	"$(INTDIR)/m_medic.obj" \
	"$(INTDIR)/m_tank.obj" \
	"$(INTDIR)/a_mod.obj" \
	"$(INTDIR)/laser.obj" \
	"$(INTDIR)/m_chick.obj" \
	"$(INTDIR)/m_flipper.obj" \
	"$(INTDIR)/g_target.obj" \
	"$(INTDIR)/m_soldier.obj" \
	"$(INTDIR)/g_main.obj" \
	"$(INTDIR)/g_func.obj" \
	"$(INTDIR)/m_insane.obj" \
	"$(INTDIR)/m_brain.obj" \
	"$(INTDIR)/p_weapon.obj" \
	"$(INTDIR)/m_boss31.obj" \
	"$(INTDIR)/m_hover.obj" \
	"$(INTDIR)/g_trigger.obj" \
	"$(INTDIR)/m_move.obj" \
	"$(INTDIR)/m_flash.obj" \
	"$(INTDIR)/g_save.obj" \
	"$(INTDIR)/m_flyer.obj" \
	"$(INTDIR)/g_utils.obj" \
	"$(INTDIR)/m_parasite.obj" \
	"$(INTDIR)/m_supertank.obj" \
	"$(INTDIR)/g_ai.obj" \
	"$(INTDIR)/g_phys.obj" \
	"$(INTDIR)/g_items.obj" \
	"$(INTDIR)/p_hud.obj" \
	"$(INTDIR)/m_boss3.obj" \
	"$(INTDIR)/g_weapon.obj" \
	"$(INTDIR)/g_combat.obj" \
	"$(INTDIR)/m_infantry.obj" \
	"$(INTDIR)/urban_chasecam.obj" \
	"$(INTDIR)/p_view.obj" \
	"$(INTDIR)/g_svcmds.obj" \
	"$(INTDIR)/g_turret.obj" \
	"$(INTDIR)/g_misc.obj" \
	"$(INTDIR)/m_actor.obj" \
	"$(INTDIR)/m_boss2.obj" \
	"$(INTDIR)/m_gladiator.obj" \
	"$(INTDIR)/g_spawn.obj" \
	"$(INTDIR)/m_gunner.obj" \
	"$(INTDIR)/urban_locational.obj" \
	"$(INTDIR)/m_mutant.obj" \
	"$(INTDIR)/g_cmds.obj" \
	"$(INTDIR)/p_client.obj" \
	"$(INTDIR)/q_shared.obj" \
	"$(INTDIR)/m_boss32.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\gamex86.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase "..\gamex86.dll"
	-@erase ".\Debug\m_boss2.obj"
	-@erase ".\Debug\m_flyer.obj"
	-@erase ".\Debug\g_phys.obj"
	-@erase ".\Debug\g_target.obj"
	-@erase ".\Debug\p_weapon.obj"
	-@erase ".\Debug\m_flipper.obj"
	-@erase ".\Debug\m_soldier.obj"
	-@erase ".\Debug\g_svcmds.obj"
	-@erase ".\Debug\g_turret.obj"
	-@erase ".\Debug\g_items.obj"
	-@erase ".\Debug\m_parasite.obj"
	-@erase ".\Debug\m_tank.obj"
	-@erase ".\Debug\m_gladiator.obj"
	-@erase ".\Debug\m_gunner.obj"
	-@erase ".\Debug\urban_chasecam.obj"
	-@erase ".\Debug\urban_locational.obj"
	-@erase ".\Debug\g_func.obj"
	-@erase ".\Debug\m_mutant.obj"
	-@erase ".\Debug\p_trail.obj"
	-@erase ".\Debug\p_client.obj"
	-@erase ".\Debug\m_infantry.obj"
	-@erase ".\Debug\g_save.obj"
	-@erase ".\Debug\m_actor.obj"
	-@erase ".\Debug\g_cmds.obj"
	-@erase ".\Debug\m_boss32.obj"
	-@erase ".\Debug\g_weapon.obj"
	-@erase ".\Debug\g_spawn.obj"
	-@erase ".\Debug\g_combat.obj"
	-@erase ".\Debug\m_berserk.obj"
	-@erase ".\Debug\g_utils.obj"
	-@erase ".\Debug\m_insane.obj"
	-@erase ".\Debug\m_boss31.obj"
	-@erase ".\Debug\p_hud.obj"
	-@erase ".\Debug\g_trigger.obj"
	-@erase ".\Debug\g_main.obj"
	-@erase ".\Debug\m_float.obj"
	-@erase ".\Debug\m_medic.obj"
	-@erase ".\Debug\m_boss3.obj"
	-@erase ".\Debug\a_mod.obj"
	-@erase ".\Debug\m_move.obj"
	-@erase ".\Debug\p_view.obj"
	-@erase ".\Debug\q_shared.obj"
	-@erase ".\Debug\laser.obj"
	-@erase ".\Debug\m_chick.obj"
	-@erase ".\Debug\g_misc.obj"
	-@erase ".\Debug\m_supertank.obj"
	-@erase ".\Debug\m_brain.obj"
	-@erase ".\Debug\m_hover.obj"
	-@erase ".\Debug\g_ai.obj"
	-@erase ".\Debug\m_flash.obj"
	-@erase ".\Debug\g_monster.obj"
	-@erase "..\gamex86.ilk"
	-@erase ".\Debug\gamex86.lib"
	-@erase ".\Debug\gamex86.exp"
	-@erase ".\Debug\gamex86.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/source.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/source.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../gamex86.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/gamex86.pdb" /debug /machine:I386 /def:".\game.def"\
 /out:"../gamex86.dll" /implib:"$(OUTDIR)/gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)/m_boss2.obj" \
	"$(INTDIR)/m_flyer.obj" \
	"$(INTDIR)/g_phys.obj" \
	"$(INTDIR)/g_target.obj" \
	"$(INTDIR)/p_weapon.obj" \
	"$(INTDIR)/m_flipper.obj" \
	"$(INTDIR)/m_soldier.obj" \
	"$(INTDIR)/g_svcmds.obj" \
	"$(INTDIR)/g_turret.obj" \
	"$(INTDIR)/g_items.obj" \
	"$(INTDIR)/m_parasite.obj" \
	"$(INTDIR)/m_tank.obj" \
	"$(INTDIR)/m_gladiator.obj" \
	"$(INTDIR)/m_gunner.obj" \
	"$(INTDIR)/urban_chasecam.obj" \
	"$(INTDIR)/urban_locational.obj" \
	"$(INTDIR)/g_func.obj" \
	"$(INTDIR)/m_mutant.obj" \
	"$(INTDIR)/p_trail.obj" \
	"$(INTDIR)/p_client.obj" \
	"$(INTDIR)/m_infantry.obj" \
	"$(INTDIR)/g_save.obj" \
	"$(INTDIR)/m_actor.obj" \
	"$(INTDIR)/g_cmds.obj" \
	"$(INTDIR)/m_boss32.obj" \
	"$(INTDIR)/g_weapon.obj" \
	"$(INTDIR)/g_spawn.obj" \
	"$(INTDIR)/g_combat.obj" \
	"$(INTDIR)/m_berserk.obj" \
	"$(INTDIR)/g_utils.obj" \
	"$(INTDIR)/m_insane.obj" \
	"$(INTDIR)/m_boss31.obj" \
	"$(INTDIR)/p_hud.obj" \
	"$(INTDIR)/g_trigger.obj" \
	"$(INTDIR)/g_main.obj" \
	"$(INTDIR)/m_float.obj" \
	"$(INTDIR)/m_medic.obj" \
	"$(INTDIR)/m_boss3.obj" \
	"$(INTDIR)/a_mod.obj" \
	"$(INTDIR)/m_move.obj" \
	"$(INTDIR)/p_view.obj" \
	"$(INTDIR)/q_shared.obj" \
	"$(INTDIR)/laser.obj" \
	"$(INTDIR)/m_chick.obj" \
	"$(INTDIR)/g_misc.obj" \
	"$(INTDIR)/m_supertank.obj" \
	"$(INTDIR)/m_brain.obj" \
	"$(INTDIR)/m_hover.obj" \
	"$(INTDIR)/g_ai.obj" \
	"$(INTDIR)/m_flash.obj" \
	"$(INTDIR)/g_monster.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "source - Win32 Release"
# Name "source - Win32 Debug"

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\a_mod.c

!IF  "$(CFG)" == "source - Win32 Release"

DEP_CPP_A_MOD=\
	".\g_local.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\game.h"\
	
NODEP_CPP_A_MOD=\
	".\FEIGN_GUN_TIME"\
	

"$(INTDIR)\a_mod.obj" : $(SOURCE) $(DEP_CPP_A_MOD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "source - Win32 Debug"

DEP_CPP_A_MOD=\
	".\g_local.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\a_mod.obj" : $(SOURCE) $(DEP_CPP_A_MOD) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_ai.c
DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_cmds.c
DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\m_player.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_combat.c
DEP_CPP_G_COM=\
	".\g_local.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_func.c
DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_items.c
DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_local.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_main.c
DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_misc.c

!IF  "$(CFG)" == "source - Win32 Release"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	
NODEP_CPP_G_MIS=\
	".\ClientHead_Touch"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "source - Win32 Debug"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_monster.c
DEP_CPP_G_MON=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_phys.c
DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_save.c
DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_spawn.c
DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_svcmds.c
DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_target.c
DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\laser.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_trigger.c
DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_turret.c
DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_utils.c
DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\g_weapon.c

!IF  "$(CFG)" == "source - Win32 Release"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	
NODEP_CPP_G_WEA=\
	".\T_RadiusDamage"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "source - Win32 Debug"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\game.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_actor.c
DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\m_actor.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_actor.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_berserk.c
DEP_CPP_M_BER=\
	".\g_local.h"\
	".\m_berserk.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_berserk.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_boss2.c
DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\m_boss2.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_boss2.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_boss3.c
DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_boss31.c
DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\m_boss31.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_boss31.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_boss32.c
DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\m_boss32.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_boss32.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_brain.c
DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\m_brain.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_brain.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_chick.c
DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\m_chick.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_chick.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_flash.c
DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_flipper.c
DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\m_flipper.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_flipper.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_float.c
DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\m_float.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_float.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_flyer.c
DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\m_flyer.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_flyer.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_gladiator.c
DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\m_gladiator.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_gladiator.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_gunner.c
DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\m_gunner.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_gunner.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_hover.c
DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\m_hover.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_hover.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_infantry.c
DEP_CPP_M_INF=\
	".\g_local.h"\
	".\m_infantry.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_infantry.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_insane.c
DEP_CPP_M_INS=\
	".\g_local.h"\
	".\m_insane.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_insane.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_medic.c
DEP_CPP_M_MED=\
	".\g_local.h"\
	".\m_medic.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_medic.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_move.c
DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_mutant.c
DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\m_mutant.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_mutant.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_parasite.c
DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\m_parasite.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_parasite.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_player.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_rider.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_soldier.c
DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\m_soldier.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_soldier.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_supertank.c
DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\m_supertank.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_supertank.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_tank.c
DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\m_tank.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\m_tank.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\p_client.c

!IF  "$(CFG)" == "source - Win32 Release"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\m_player.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	
NODEP_CPP_P_CLI=\
	".\player_die"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "source - Win32 Debug"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\m_player.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\p_hud.c
DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\p_trail.c
DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\p_view.c
DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\p_weapon.c
DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\q_shared.c
DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\q_shared.h

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\game.def

!IF  "$(CFG)" == "source - Win32 Release"

!ELSEIF  "$(CFG)" == "source - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\laser.c

!IF  "$(CFG)" == "source - Win32 Release"

DEP_CPP_LASER=\
	".\g_local.h"\
	".\laser.h"\
	".\q_shared.h"\
	".\game.h"\
	
NODEP_CPP_LASER=\
	".\Cells"\
	

"$(INTDIR)\laser.obj" : $(SOURCE) $(DEP_CPP_LASER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "source - Win32 Debug"

DEP_CPP_LASER=\
	".\g_local.h"\
	".\laser.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\laser.obj" : $(SOURCE) $(DEP_CPP_LASER) "$(INTDIR)"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\urban_chasecam.c
DEP_CPP_URBAN=\
	".\g_local.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\urban_chasecam.obj" : $(SOURCE) $(DEP_CPP_URBAN) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\urban_locational.c
DEP_CPP_URBAN_=\
	".\g_local.h"\
	".\mod.h"\
	".\q_shared.h"\
	".\game.h"\
	

"$(INTDIR)\urban_locational.obj" : $(SOURCE) $(DEP_CPP_URBAN_) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
