# Microsoft Developer Studio Generated NMAKE File, Based on game.dsp
!IF "$(CFG)" == ""
CFG=game - Win32 Debug Alpha
!MESSAGE No configuration specified. Defaulting to game - Win32 Debug Alpha.
!ENDIF 

!IF "$(CFG)" != "game - Win32 Release" && "$(CFG)" != "game - Win32 Debug" && "$(CFG)" != "game - Win32 Debug Alpha" && "$(CFG)" != "game - Win32 Release Alpha"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 Debug Alpha"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Debug Alpha" (based on "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "game - Win32 Release Alpha" (based on "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "game - Win32 Release"

OUTDIR=.\..\release
INTDIR=.\release

ALL : "..\stroggdm\gamex86.dll"


CLEAN :
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_chase.obj"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_svcmds.obj"
	-@erase "$(INTDIR)\g_target.obj"
	-@erase "$(INTDIR)\g_trigger.obj"
	-@erase "$(INTDIR)\g_turret.obj"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_config.obj"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_stalker.obj"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\pa_helpers.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "..\stroggdm\gamex86.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /MT /W2 /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb" /machine:I386 /def:".\game.def" /out:"..\stroggdm\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_chase.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_svcmds.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_turret.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_config.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_stalker.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\pa_helpers.obj" \
	"$(INTDIR)\q_shared.obj"

"..\stroggdm\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

OUTDIR=.\..\debug
INTDIR=.\debug
# Begin Custom Macros
OutDir=.\..\debug
# End Custom Macros

ALL : "$(OUTDIR)\gamex86.dll" "$(OUTDIR)\game.bsc"


CLEAN :
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_ai.sbr"
	-@erase "$(INTDIR)\g_chase.obj"
	-@erase "$(INTDIR)\g_chase.sbr"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_cmds.sbr"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_combat.sbr"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_func.sbr"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_items.sbr"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_main.sbr"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_misc.sbr"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_monster.sbr"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_phys.sbr"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_save.sbr"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_spawn.sbr"
	-@erase "$(INTDIR)\g_svcmds.obj"
	-@erase "$(INTDIR)\g_svcmds.sbr"
	-@erase "$(INTDIR)\g_target.obj"
	-@erase "$(INTDIR)\g_target.sbr"
	-@erase "$(INTDIR)\g_trigger.obj"
	-@erase "$(INTDIR)\g_trigger.sbr"
	-@erase "$(INTDIR)\g_turret.obj"
	-@erase "$(INTDIR)\g_turret.sbr"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_utils.sbr"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\g_weapon.sbr"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_actor.sbr"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_berserk.sbr"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss2.sbr"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss3.sbr"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss31.sbr"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_boss32.sbr"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_brain.sbr"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_chick.sbr"
	-@erase "$(INTDIR)\m_config.obj"
	-@erase "$(INTDIR)\m_config.sbr"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flash.sbr"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_flipper.sbr"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_float.sbr"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_flyer.sbr"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gladiator.sbr"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_gunner.sbr"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_hover.sbr"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_infantry.sbr"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_insane.sbr"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_medic.sbr"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_move.sbr"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_mutant.sbr"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_parasite.sbr"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_soldier.sbr"
	-@erase "$(INTDIR)\m_stalker.obj"
	-@erase "$(INTDIR)\m_stalker.sbr"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_supertank.sbr"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\m_tank.sbr"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_client.sbr"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_hud.sbr"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_menu.sbr"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_trail.sbr"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_view.sbr"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\p_weapon.sbr"
	-@erase "$(INTDIR)\pa_helpers.obj"
	-@erase "$(INTDIR)\pa_helpers.sbr"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\q_shared.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\game.bsc"
	-@erase "$(OUTDIR)\gamex86.dll"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "$(OUTDIR)\gamex86.pdb"
	-@erase ".\debug\gamex86.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\g_ai.sbr" \
	"$(INTDIR)\g_chase.sbr" \
	"$(INTDIR)\g_cmds.sbr" \
	"$(INTDIR)\g_combat.sbr" \
	"$(INTDIR)\g_func.sbr" \
	"$(INTDIR)\g_items.sbr" \
	"$(INTDIR)\g_main.sbr" \
	"$(INTDIR)\g_misc.sbr" \
	"$(INTDIR)\g_monster.sbr" \
	"$(INTDIR)\g_phys.sbr" \
	"$(INTDIR)\g_save.sbr" \
	"$(INTDIR)\g_spawn.sbr" \
	"$(INTDIR)\g_svcmds.sbr" \
	"$(INTDIR)\g_target.sbr" \
	"$(INTDIR)\g_trigger.sbr" \
	"$(INTDIR)\g_turret.sbr" \
	"$(INTDIR)\g_utils.sbr" \
	"$(INTDIR)\g_weapon.sbr" \
	"$(INTDIR)\m_actor.sbr" \
	"$(INTDIR)\m_berserk.sbr" \
	"$(INTDIR)\m_boss2.sbr" \
	"$(INTDIR)\m_boss3.sbr" \
	"$(INTDIR)\m_boss31.sbr" \
	"$(INTDIR)\m_boss32.sbr" \
	"$(INTDIR)\m_brain.sbr" \
	"$(INTDIR)\m_chick.sbr" \
	"$(INTDIR)\m_config.sbr" \
	"$(INTDIR)\m_flash.sbr" \
	"$(INTDIR)\m_flipper.sbr" \
	"$(INTDIR)\m_float.sbr" \
	"$(INTDIR)\m_flyer.sbr" \
	"$(INTDIR)\m_gladiator.sbr" \
	"$(INTDIR)\m_gunner.sbr" \
	"$(INTDIR)\m_hover.sbr" \
	"$(INTDIR)\m_infantry.sbr" \
	"$(INTDIR)\m_insane.sbr" \
	"$(INTDIR)\m_medic.sbr" \
	"$(INTDIR)\m_move.sbr" \
	"$(INTDIR)\m_mutant.sbr" \
	"$(INTDIR)\m_parasite.sbr" \
	"$(INTDIR)\m_soldier.sbr" \
	"$(INTDIR)\m_stalker.sbr" \
	"$(INTDIR)\m_supertank.sbr" \
	"$(INTDIR)\m_tank.sbr" \
	"$(INTDIR)\p_client.sbr" \
	"$(INTDIR)\p_hud.sbr" \
	"$(INTDIR)\p_menu.sbr" \
	"$(INTDIR)\p_trail.sbr" \
	"$(INTDIR)\p_view.sbr" \
	"$(INTDIR)\p_weapon.sbr" \
	"$(INTDIR)\pa_helpers.sbr" \
	"$(INTDIR)\q_shared.sbr"

"$(OUTDIR)\game.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb" /map:"$(INTDIR)\gamex86.map" /debug /machine:I386 /def:".\game.def" /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_chase.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_svcmds.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_turret.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_config.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_stalker.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\pa_helpers.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

OUTDIR=.\..\DebugAxp
INTDIR=.\DebugAxp
# Begin Custom Macros
OutDir=.\..\DebugAxp
# End Custom Macros

ALL : "$(OUTDIR)\gameaxp.dll"


CLEAN :
	-@erase "$(OUTDIR)\gameaxp.dll"
	-@erase "$(OUTDIR)\gameaxp.exp"
	-@erase "$(OUTDIR)\gameaxp.ilk"
	-@erase "$(OUTDIR)\gameaxp.lib"
	-@erase "$(OUTDIR)\gameaxp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /base:"0x20000000" /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\gameaxp.pdb" /debug /machine:ALPHA /def:".\game.def" /out:"$(OUTDIR)\gameaxp.dll" /implib:"$(OUTDIR)\gameaxp.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	

"$(OUTDIR)\gameaxp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

OUTDIR=.\..\ReleaseAXP
INTDIR=.\ReleaseAXP
# Begin Custom Macros
OutDir=.\..\ReleaseAXP
# End Custom Macros

ALL : "$(OUTDIR)\gameaxp.dll"


CLEAN :
	-@erase "$(OUTDIR)\gameaxp.dll"
	-@erase "$(OUTDIR)\gameaxp.exp"
	-@erase "$(OUTDIR)\gameaxp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib /nologo /base:"0x20000000" /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gameaxp.pdb" /machine:ALPHA /def:".\game.def" /out:"$(OUTDIR)\gameaxp.dll" /implib:"$(OUTDIR)\gameaxp.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	

"$(OUTDIR)\gameaxp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("game.dep")
!INCLUDE "game.dep"
!ELSE 
!MESSAGE Warning: cannot find "game.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "game - Win32 Release" || "$(CFG)" == "game - Win32 Debug" || "$(CFG)" == "game - Win32 Debug Alpha" || "$(CFG)" == "game - Win32 Release Alpha"
SOURCE=.\g_ai.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_ai.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_ai.obj"	"$(INTDIR)\g_ai.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_chase.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_chase.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_chase.obj"	"$(INTDIR)\g_chase.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_cmds.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_cmds.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_cmds.obj"	"$(INTDIR)\g_cmds.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_combat.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_combat.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_combat.obj"	"$(INTDIR)\g_combat.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_func.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_func.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_func.obj"	"$(INTDIR)\g_func.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_items.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_items.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_items.obj"	"$(INTDIR)\g_items.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_main.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_main.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_main.obj"	"$(INTDIR)\g_main.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_misc.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_misc.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_misc.obj"	"$(INTDIR)\g_misc.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_monster.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_monster.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_monster.obj"	"$(INTDIR)\g_monster.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_phys.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_phys.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_phys.obj"	"$(INTDIR)\g_phys.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_save.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_save.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_save.obj"	"$(INTDIR)\g_save.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_spawn.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_spawn.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_spawn.obj"	"$(INTDIR)\g_spawn.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_svcmds.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_svcmds.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_svcmds.obj"	"$(INTDIR)\g_svcmds.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_target.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_target.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_target.obj"	"$(INTDIR)\g_target.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_trigger.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_trigger.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_trigger.obj"	"$(INTDIR)\g_trigger.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_turret.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_turret.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_turret.obj"	"$(INTDIR)\g_turret.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_utils.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_utils.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_utils.obj"	"$(INTDIR)\g_utils.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\g_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\g_weapon.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\g_weapon.obj"	"$(INTDIR)\g_weapon.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_actor.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_actor.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_actor.obj"	"$(INTDIR)\m_actor.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_berserk.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_berserk.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_berserk.obj"	"$(INTDIR)\m_berserk.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_boss2.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_boss2.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_boss2.obj"	"$(INTDIR)\m_boss2.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_boss3.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_boss3.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_boss3.obj"	"$(INTDIR)\m_boss3.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_boss31.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_boss31.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_boss31.obj"	"$(INTDIR)\m_boss31.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_boss32.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_boss32.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_boss32.obj"	"$(INTDIR)\m_boss32.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_brain.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_brain.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_brain.obj"	"$(INTDIR)\m_brain.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_chick.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_chick.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_chick.obj"	"$(INTDIR)\m_chick.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_config.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_config.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_config.obj"	"$(INTDIR)\m_config.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_flash.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_flash.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_flash.obj"	"$(INTDIR)\m_flash.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_flipper.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_flipper.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_flipper.obj"	"$(INTDIR)\m_flipper.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_float.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_float.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_float.obj"	"$(INTDIR)\m_float.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_flyer.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_flyer.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_flyer.obj"	"$(INTDIR)\m_flyer.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_gladiator.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_gladiator.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_gladiator.obj"	"$(INTDIR)\m_gladiator.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_gunner.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_gunner.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_gunner.obj"	"$(INTDIR)\m_gunner.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_hover.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_hover.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_hover.obj"	"$(INTDIR)\m_hover.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_infantry.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_infantry.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_infantry.obj"	"$(INTDIR)\m_infantry.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_insane.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_insane.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_insane.obj"	"$(INTDIR)\m_insane.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_medic.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_medic.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_medic.obj"	"$(INTDIR)\m_medic.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_move.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_move.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_move.obj"	"$(INTDIR)\m_move.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_mutant.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_mutant.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_mutant.obj"	"$(INTDIR)\m_mutant.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_parasite.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_parasite.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_parasite.obj"	"$(INTDIR)\m_parasite.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_soldier.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_soldier.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_soldier.obj"	"$(INTDIR)\m_soldier.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_stalker.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_stalker.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_stalker.obj"	"$(INTDIR)\m_stalker.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_supertank.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_supertank.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_supertank.obj"	"$(INTDIR)\m_supertank.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\m_tank.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\m_tank.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\m_tank.obj"	"$(INTDIR)\m_tank.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\p_client.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\p_client.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\p_client.obj"	"$(INTDIR)\p_client.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\p_hud.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\p_hud.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\p_hud.obj"	"$(INTDIR)\p_hud.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\p_menu.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\p_menu.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\p_menu.obj"	"$(INTDIR)\p_menu.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\p_trail.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\p_trail.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\p_trail.obj"	"$(INTDIR)\p_trail.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\p_view.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\p_view.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\p_view.obj"	"$(INTDIR)\p_view.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\p_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\p_weapon.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\p_weapon.obj"	"$(INTDIR)\p_weapon.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\pa_helpers.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\pa_helpers.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\pa_helpers.obj"	"$(INTDIR)\pa_helpers.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 

SOURCE=.\q_shared.c

!IF  "$(CFG)" == "game - Win32 Release"


"$(INTDIR)\q_shared.obj" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"


"$(INTDIR)\q_shared.obj"	"$(INTDIR)\q_shared.sbr" : $(SOURCE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

!ENDIF 


!ENDIF 

