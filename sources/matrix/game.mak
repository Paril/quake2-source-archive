# Microsoft Developer Studio Generated NMAKE File, Based on game.dsp
!IF "$(CFG)" == ""
CFG=game - Win32 Debug Alpha
!MESSAGE No configuration specified. Defaulting to game - Win32 Debug Alpha.
!ENDIF 

!IF "$(CFG)" != "game - Win32 Release" && "$(CFG)" != "game - Win32 Debug" &&\
 "$(CFG)" != "game - Win32 Debug Alpha" && "$(CFG)" !=\
 "game - Win32 Release Alpha"
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
!MESSAGE "game - Win32 Debug Alpha" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "game - Win32 Release Alpha" (based on\
 "Win32 (ALPHA) Dynamic-Link Library")
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
# Begin Custom Macros
OutDir=.\..\release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "..\..\games\quake2\matrix\gamex86.dll" "$(OUTDIR)\game.bsc"

!ELSE 

ALL : "..\..\games\quake2\matrix\gamex86.dll" "$(OUTDIR)\game.bsc"

!ENDIF 

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
	-@erase "$(INTDIR)\leper_think.obj"
	-@erase "$(INTDIR)\leper_think.sbr"
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
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_supertank.sbr"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\m_tank.sbr"
	-@erase "$(INTDIR)\Matrix_Autobuy.obj"
	-@erase "$(INTDIR)\Matrix_Autobuy.sbr"
	-@erase "$(INTDIR)\matrix_cam.obj"
	-@erase "$(INTDIR)\matrix_cam.sbr"
	-@erase "$(INTDIR)\matrix_cmds.obj"
	-@erase "$(INTDIR)\matrix_cmds.sbr"
	-@erase "$(INTDIR)\matrix_findtarget.obj"
	-@erase "$(INTDIR)\matrix_findtarget.sbr"
	-@erase "$(INTDIR)\matrix_gweap.obj"
	-@erase "$(INTDIR)\matrix_gweap.sbr"
	-@erase "$(INTDIR)\matrix_items.obj"
	-@erase "$(INTDIR)\matrix_items.sbr"
	-@erase "$(INTDIR)\matrix_kungfu.obj"
	-@erase "$(INTDIR)\matrix_kungfu.sbr"
	-@erase "$(INTDIR)\matrix_locadam.obj"
	-@erase "$(INTDIR)\matrix_locadam.sbr"
	-@erase "$(INTDIR)\matrix_main.obj"
	-@erase "$(INTDIR)\matrix_main.sbr"
	-@erase "$(INTDIR)\matrix_match.obj"
	-@erase "$(INTDIR)\matrix_match.sbr"
	-@erase "$(INTDIR)\matrix_menu.obj"
	-@erase "$(INTDIR)\matrix_menu.sbr"
	-@erase "$(INTDIR)\matrix_misc.obj"
	-@erase "$(INTDIR)\matrix_misc.sbr"
	-@erase "$(INTDIR)\matrix_movement.obj"
	-@erase "$(INTDIR)\matrix_movement.sbr"
	-@erase "$(INTDIR)\matrix_pweap.obj"
	-@erase "$(INTDIR)\matrix_pweap.sbr"
	-@erase "$(INTDIR)\matrix_spells.obj"
	-@erase "$(INTDIR)\matrix_spells.sbr"
	-@erase "$(INTDIR)\matrix_stamina.obj"
	-@erase "$(INTDIR)\matrix_stamina.sbr"
	-@erase "$(INTDIR)\matrix_tank.obj"
	-@erase "$(INTDIR)\matrix_tank.sbr"
	-@erase "$(INTDIR)\matrix_team.obj"
	-@erase "$(INTDIR)\matrix_team.sbr"
	-@erase "$(INTDIR)\p_chase.obj"
	-@erase "$(INTDIR)\p_chase.sbr"
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
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\q_shared.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\game.bsc"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "..\..\games\quake2\matrix\gamex86.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /MT /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR"$(INTDIR)\\" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"\
 /FD /c 
CPP_OBJS=.\release/
CPP_SBRS=.\release/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
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
	"$(INTDIR)\leper_think.sbr" \
	"$(INTDIR)\m_actor.sbr" \
	"$(INTDIR)\m_berserk.sbr" \
	"$(INTDIR)\m_boss2.sbr" \
	"$(INTDIR)\m_boss3.sbr" \
	"$(INTDIR)\m_boss31.sbr" \
	"$(INTDIR)\m_boss32.sbr" \
	"$(INTDIR)\m_brain.sbr" \
	"$(INTDIR)\m_chick.sbr" \
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
	"$(INTDIR)\m_supertank.sbr" \
	"$(INTDIR)\m_tank.sbr" \
	"$(INTDIR)\Matrix_Autobuy.sbr" \
	"$(INTDIR)\matrix_cam.sbr" \
	"$(INTDIR)\matrix_cmds.sbr" \
	"$(INTDIR)\matrix_findtarget.sbr" \
	"$(INTDIR)\matrix_gweap.sbr" \
	"$(INTDIR)\matrix_items.sbr" \
	"$(INTDIR)\matrix_kungfu.sbr" \
	"$(INTDIR)\matrix_locadam.sbr" \
	"$(INTDIR)\matrix_main.sbr" \
	"$(INTDIR)\matrix_match.sbr" \
	"$(INTDIR)\matrix_menu.sbr" \
	"$(INTDIR)\matrix_misc.sbr" \
	"$(INTDIR)\matrix_movement.sbr" \
	"$(INTDIR)\matrix_pweap.sbr" \
	"$(INTDIR)\matrix_spells.sbr" \
	"$(INTDIR)\matrix_stamina.sbr" \
	"$(INTDIR)\matrix_tank.sbr" \
	"$(INTDIR)\matrix_team.sbr" \
	"$(INTDIR)\p_chase.sbr" \
	"$(INTDIR)\p_client.sbr" \
	"$(INTDIR)\p_hud.sbr" \
	"$(INTDIR)\p_menu.sbr" \
	"$(INTDIR)\p_trail.sbr" \
	"$(INTDIR)\p_view.sbr" \
	"$(INTDIR)\p_weapon.sbr" \
	"$(INTDIR)\q_shared.sbr"

"$(OUTDIR)\game.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib winmm.lib /nologo /base:"0x20000000"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb"\
 /machine:I386 /def:".\game.def" /out:"c:\games\quake2\matrix\gamex86.dll"\
 /implib:"$(OUTDIR)\gamex86.lib" 
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
	"$(INTDIR)\leper_think.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
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
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\Matrix_Autobuy.obj" \
	"$(INTDIR)\matrix_cam.obj" \
	"$(INTDIR)\matrix_cmds.obj" \
	"$(INTDIR)\matrix_findtarget.obj" \
	"$(INTDIR)\matrix_gweap.obj" \
	"$(INTDIR)\matrix_items.obj" \
	"$(INTDIR)\matrix_kungfu.obj" \
	"$(INTDIR)\matrix_locadam.obj" \
	"$(INTDIR)\matrix_main.obj" \
	"$(INTDIR)\matrix_match.obj" \
	"$(INTDIR)\matrix_menu.obj" \
	"$(INTDIR)\matrix_misc.obj" \
	"$(INTDIR)\matrix_movement.obj" \
	"$(INTDIR)\matrix_pweap.obj" \
	"$(INTDIR)\matrix_spells.obj" \
	"$(INTDIR)\matrix_stamina.obj" \
	"$(INTDIR)\matrix_tank.obj" \
	"$(INTDIR)\matrix_team.obj" \
	"$(INTDIR)\p_chase.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"..\..\games\quake2\matrix\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE)\
 $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

OUTDIR=.\..\debug
INTDIR=.\debug
# Begin Custom Macros
OutDir=.\..\debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gamex86.dll" "$(OUTDIR)\game.bsc"

!ELSE 

ALL : "$(OUTDIR)\gamex86.dll" "$(OUTDIR)\game.bsc"

!ENDIF 

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
	-@erase "$(INTDIR)\leper_think.obj"
	-@erase "$(INTDIR)\leper_think.sbr"
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
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_supertank.sbr"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\m_tank.sbr"
	-@erase "$(INTDIR)\Matrix_Autobuy.obj"
	-@erase "$(INTDIR)\Matrix_Autobuy.sbr"
	-@erase "$(INTDIR)\matrix_cam.obj"
	-@erase "$(INTDIR)\matrix_cam.sbr"
	-@erase "$(INTDIR)\matrix_cmds.obj"
	-@erase "$(INTDIR)\matrix_cmds.sbr"
	-@erase "$(INTDIR)\matrix_findtarget.obj"
	-@erase "$(INTDIR)\matrix_findtarget.sbr"
	-@erase "$(INTDIR)\matrix_gweap.obj"
	-@erase "$(INTDIR)\matrix_gweap.sbr"
	-@erase "$(INTDIR)\matrix_items.obj"
	-@erase "$(INTDIR)\matrix_items.sbr"
	-@erase "$(INTDIR)\matrix_kungfu.obj"
	-@erase "$(INTDIR)\matrix_kungfu.sbr"
	-@erase "$(INTDIR)\matrix_locadam.obj"
	-@erase "$(INTDIR)\matrix_locadam.sbr"
	-@erase "$(INTDIR)\matrix_main.obj"
	-@erase "$(INTDIR)\matrix_main.sbr"
	-@erase "$(INTDIR)\matrix_match.obj"
	-@erase "$(INTDIR)\matrix_match.sbr"
	-@erase "$(INTDIR)\matrix_menu.obj"
	-@erase "$(INTDIR)\matrix_menu.sbr"
	-@erase "$(INTDIR)\matrix_misc.obj"
	-@erase "$(INTDIR)\matrix_misc.sbr"
	-@erase "$(INTDIR)\matrix_movement.obj"
	-@erase "$(INTDIR)\matrix_movement.sbr"
	-@erase "$(INTDIR)\matrix_pweap.obj"
	-@erase "$(INTDIR)\matrix_pweap.sbr"
	-@erase "$(INTDIR)\matrix_spells.obj"
	-@erase "$(INTDIR)\matrix_spells.sbr"
	-@erase "$(INTDIR)\matrix_stamina.obj"
	-@erase "$(INTDIR)\matrix_stamina.sbr"
	-@erase "$(INTDIR)\matrix_tank.obj"
	-@erase "$(INTDIR)\matrix_tank.sbr"
	-@erase "$(INTDIR)\matrix_team.obj"
	-@erase "$(INTDIR)\matrix_team.sbr"
	-@erase "$(INTDIR)\p_chase.obj"
	-@erase "$(INTDIR)\p_chase.sbr"
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
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\q_shared.sbr"
	-@erase "$(INTDIR)\vc50.idb"
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
CPP_PROJ=/nologo /G5 /MTd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "BUILDING_REF_GL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /ZI /c 
CPP_OBJS=.\debug/
CPP_SBRS=.\debug/

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
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
	"$(INTDIR)\leper_think.sbr" \
	"$(INTDIR)\m_actor.sbr" \
	"$(INTDIR)\m_berserk.sbr" \
	"$(INTDIR)\m_boss2.sbr" \
	"$(INTDIR)\m_boss3.sbr" \
	"$(INTDIR)\m_boss31.sbr" \
	"$(INTDIR)\m_boss32.sbr" \
	"$(INTDIR)\m_brain.sbr" \
	"$(INTDIR)\m_chick.sbr" \
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
	"$(INTDIR)\m_supertank.sbr" \
	"$(INTDIR)\m_tank.sbr" \
	"$(INTDIR)\Matrix_Autobuy.sbr" \
	"$(INTDIR)\matrix_cam.sbr" \
	"$(INTDIR)\matrix_cmds.sbr" \
	"$(INTDIR)\matrix_findtarget.sbr" \
	"$(INTDIR)\matrix_gweap.sbr" \
	"$(INTDIR)\matrix_items.sbr" \
	"$(INTDIR)\matrix_kungfu.sbr" \
	"$(INTDIR)\matrix_locadam.sbr" \
	"$(INTDIR)\matrix_main.sbr" \
	"$(INTDIR)\matrix_match.sbr" \
	"$(INTDIR)\matrix_menu.sbr" \
	"$(INTDIR)\matrix_misc.sbr" \
	"$(INTDIR)\matrix_movement.sbr" \
	"$(INTDIR)\matrix_pweap.sbr" \
	"$(INTDIR)\matrix_spells.sbr" \
	"$(INTDIR)\matrix_stamina.sbr" \
	"$(INTDIR)\matrix_tank.sbr" \
	"$(INTDIR)\matrix_team.sbr" \
	"$(INTDIR)\p_chase.sbr" \
	"$(INTDIR)\p_client.sbr" \
	"$(INTDIR)\p_hud.sbr" \
	"$(INTDIR)\p_menu.sbr" \
	"$(INTDIR)\p_trail.sbr" \
	"$(INTDIR)\p_view.sbr" \
	"$(INTDIR)\p_weapon.sbr" \
	"$(INTDIR)\q_shared.sbr"

"$(OUTDIR)\game.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib winmm.lib /nologo /base:"0x20000000"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb"\
 /map:"$(INTDIR)\gamex86.map" /debug /machine:I386 /def:".\game.def"\
 /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
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
	"$(INTDIR)\leper_think.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
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
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\Matrix_Autobuy.obj" \
	"$(INTDIR)\matrix_cam.obj" \
	"$(INTDIR)\matrix_cmds.obj" \
	"$(INTDIR)\matrix_findtarget.obj" \
	"$(INTDIR)\matrix_gweap.obj" \
	"$(INTDIR)\matrix_items.obj" \
	"$(INTDIR)\matrix_kungfu.obj" \
	"$(INTDIR)\matrix_locadam.obj" \
	"$(INTDIR)\matrix_main.obj" \
	"$(INTDIR)\matrix_match.obj" \
	"$(INTDIR)\matrix_menu.obj" \
	"$(INTDIR)\matrix_misc.obj" \
	"$(INTDIR)\matrix_movement.obj" \
	"$(INTDIR)\matrix_pweap.obj" \
	"$(INTDIR)\matrix_spells.obj" \
	"$(INTDIR)\matrix_stamina.obj" \
	"$(INTDIR)\matrix_tank.obj" \
	"$(INTDIR)\matrix_team.obj" \
	"$(INTDIR)\p_chase.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
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

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gameaxp.dll"

!ELSE 

ALL : "$(OUTDIR)\gameaxp.dll"

!ENDIF 

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
	-@erase "$(INTDIR)\leper_think.obj"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_chick.obj"
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
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\Matrix_Autobuy.obj"
	-@erase "$(INTDIR)\matrix_cam.obj"
	-@erase "$(INTDIR)\matrix_cmds.obj"
	-@erase "$(INTDIR)\matrix_findtarget.obj"
	-@erase "$(INTDIR)\matrix_gweap.obj"
	-@erase "$(INTDIR)\matrix_items.obj"
	-@erase "$(INTDIR)\matrix_kungfu.obj"
	-@erase "$(INTDIR)\matrix_locadam.obj"
	-@erase "$(INTDIR)\matrix_main.obj"
	-@erase "$(INTDIR)\matrix_match.obj"
	-@erase "$(INTDIR)\matrix_menu.obj"
	-@erase "$(INTDIR)\matrix_misc.obj"
	-@erase "$(INTDIR)\matrix_movement.obj"
	-@erase "$(INTDIR)\matrix_pweap.obj"
	-@erase "$(INTDIR)\matrix_spells.obj"
	-@erase "$(INTDIR)\matrix_stamina.obj"
	-@erase "$(INTDIR)\matrix_tank.obj"
	-@erase "$(INTDIR)\matrix_team.obj"
	-@erase "$(INTDIR)\p_chase.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\gameaxp.dll"
	-@erase "$(OUTDIR)\gameaxp.exp"
	-@erase "$(OUTDIR)\gameaxp.lib"
	-@erase "$(OUTDIR)\gameaxp.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o /win32 "NUL" 
CPP=cl.exe
CPP_PROJ=/nologo /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D\
 "C_ONLY" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /QA21164 /MTd /c 
CPP_OBJS=.\DebugAxp/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo\
 /base:"0x20000000" /subsystem:windows /dll /pdb:"$(OUTDIR)\gameaxp.pdb" /debug\
 /machine:ALPHA /def:".\game.def" /out:"$(OUTDIR)\gameaxp.dll"\
 /implib:"$(OUTDIR)\gameaxp.lib" 
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
	"$(INTDIR)\leper_think.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
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
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\Matrix_Autobuy.obj" \
	"$(INTDIR)\matrix_cam.obj" \
	"$(INTDIR)\matrix_cmds.obj" \
	"$(INTDIR)\matrix_findtarget.obj" \
	"$(INTDIR)\matrix_gweap.obj" \
	"$(INTDIR)\matrix_items.obj" \
	"$(INTDIR)\matrix_kungfu.obj" \
	"$(INTDIR)\matrix_locadam.obj" \
	"$(INTDIR)\matrix_main.obj" \
	"$(INTDIR)\matrix_match.obj" \
	"$(INTDIR)\matrix_menu.obj" \
	"$(INTDIR)\matrix_misc.obj" \
	"$(INTDIR)\matrix_movement.obj" \
	"$(INTDIR)\matrix_pweap.obj" \
	"$(INTDIR)\matrix_spells.obj" \
	"$(INTDIR)\matrix_stamina.obj" \
	"$(INTDIR)\matrix_tank.obj" \
	"$(INTDIR)\matrix_team.obj" \
	"$(INTDIR)\p_chase.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

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

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gameaxp.dll"

!ELSE 

ALL : "$(OUTDIR)\gameaxp.dll"

!ENDIF 

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
	-@erase "$(INTDIR)\leper_think.obj"
	-@erase "$(INTDIR)\m_actor.obj"
	-@erase "$(INTDIR)\m_berserk.obj"
	-@erase "$(INTDIR)\m_boss2.obj"
	-@erase "$(INTDIR)\m_boss3.obj"
	-@erase "$(INTDIR)\m_boss31.obj"
	-@erase "$(INTDIR)\m_boss32.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_chick.obj"
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
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\Matrix_Autobuy.obj"
	-@erase "$(INTDIR)\matrix_cam.obj"
	-@erase "$(INTDIR)\matrix_cmds.obj"
	-@erase "$(INTDIR)\matrix_findtarget.obj"
	-@erase "$(INTDIR)\matrix_gweap.obj"
	-@erase "$(INTDIR)\matrix_items.obj"
	-@erase "$(INTDIR)\matrix_kungfu.obj"
	-@erase "$(INTDIR)\matrix_locadam.obj"
	-@erase "$(INTDIR)\matrix_main.obj"
	-@erase "$(INTDIR)\matrix_match.obj"
	-@erase "$(INTDIR)\matrix_menu.obj"
	-@erase "$(INTDIR)\matrix_misc.obj"
	-@erase "$(INTDIR)\matrix_movement.obj"
	-@erase "$(INTDIR)\matrix_pweap.obj"
	-@erase "$(INTDIR)\matrix_spells.obj"
	-@erase "$(INTDIR)\matrix_stamina.obj"
	-@erase "$(INTDIR)\matrix_tank.obj"
	-@erase "$(INTDIR)\matrix_team.obj"
	-@erase "$(INTDIR)\p_chase.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\gameaxp.dll"
	-@erase "$(OUTDIR)\gameaxp.exp"
	-@erase "$(OUTDIR)\gameaxp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o /win32 "NUL" 
CPP=cl.exe
CPP_PROJ=/nologo /MT /Gt0 /W3 /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /D "C_ONLY" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /QA21164 /c 
CPP_OBJS=.\ReleaseAXP/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib /nologo /base:"0x20000000"\
 /subsystem:windows /dll /pdb:"$(OUTDIR)\gameaxp.pdb" /machine:ALPHA\
 /def:".\game.def" /out:"$(OUTDIR)\gameaxp.dll" /implib:"$(OUTDIR)\gameaxp.lib" 
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
	"$(INTDIR)\leper_think.obj" \
	"$(INTDIR)\m_actor.obj" \
	"$(INTDIR)\m_berserk.obj" \
	"$(INTDIR)\m_boss2.obj" \
	"$(INTDIR)\m_boss3.obj" \
	"$(INTDIR)\m_boss31.obj" \
	"$(INTDIR)\m_boss32.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_chick.obj" \
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
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\Matrix_Autobuy.obj" \
	"$(INTDIR)\matrix_cam.obj" \
	"$(INTDIR)\matrix_cmds.obj" \
	"$(INTDIR)\matrix_findtarget.obj" \
	"$(INTDIR)\matrix_gweap.obj" \
	"$(INTDIR)\matrix_items.obj" \
	"$(INTDIR)\matrix_kungfu.obj" \
	"$(INTDIR)\matrix_locadam.obj" \
	"$(INTDIR)\matrix_main.obj" \
	"$(INTDIR)\matrix_match.obj" \
	"$(INTDIR)\matrix_menu.obj" \
	"$(INTDIR)\matrix_misc.obj" \
	"$(INTDIR)\matrix_movement.obj" \
	"$(INTDIR)\matrix_pweap.obj" \
	"$(INTDIR)\matrix_spells.obj" \
	"$(INTDIR)\matrix_stamina.obj" \
	"$(INTDIR)\matrix_tank.obj" \
	"$(INTDIR)\matrix_team.obj" \
	"$(INTDIR)\p_chase.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gameaxp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "game - Win32 Release" || "$(CFG)" == "game - Win32 Debug" ||\
 "$(CFG)" == "game - Win32 Debug Alpha" || "$(CFG)" ==\
 "game - Win32 Release Alpha"
SOURCE=.\g_ai.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj"	"$(INTDIR)\g_ai.sbr" : $(SOURCE) $(DEP_CPP_G_AI_)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj"	"$(INTDIR)\g_ai.sbr" : $(SOURCE) $(DEP_CPP_G_AI_)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_chase.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj"	"$(INTDIR)\g_chase.sbr" : $(SOURCE) $(DEP_CPP_G_CHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj"	"$(INTDIR)\g_chase.sbr" : $(SOURCE) $(DEP_CPP_G_CHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj" : $(SOURCE) $(DEP_CPP_G_CHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj" : $(SOURCE) $(DEP_CPP_G_CHA) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_cmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj"	"$(INTDIR)\g_cmds.sbr" : $(SOURCE) $(DEP_CPP_G_CMD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj"	"$(INTDIR)\g_cmds.sbr" : $(SOURCE) $(DEP_CPP_G_CMD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_CMD=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_combat.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj"	"$(INTDIR)\g_combat.sbr" : $(SOURCE) $(DEP_CPP_G_COM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj"	"$(INTDIR)\g_combat.sbr" : $(SOURCE) $(DEP_CPP_G_COM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_func.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj"	"$(INTDIR)\g_func.sbr" : $(SOURCE) $(DEP_CPP_G_FUN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj"	"$(INTDIR)\g_func.sbr" : $(SOURCE) $(DEP_CPP_G_FUN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_items.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj"	"$(INTDIR)\g_items.sbr" : $(SOURCE) $(DEP_CPP_G_ITE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj"	"$(INTDIR)\g_items.sbr" : $(SOURCE) $(DEP_CPP_G_ITE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_main.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj"	"$(INTDIR)\g_main.sbr" : $(SOURCE) $(DEP_CPP_G_MAI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj"	"$(INTDIR)\g_main.sbr" : $(SOURCE) $(DEP_CPP_G_MAI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MAI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_misc.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj"	"$(INTDIR)\g_misc.sbr" : $(SOURCE) $(DEP_CPP_G_MIS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj"	"$(INTDIR)\g_misc.sbr" : $(SOURCE) $(DEP_CPP_G_MIS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_monster.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj"	"$(INTDIR)\g_monster.sbr" : $(SOURCE)\
 $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj"	"$(INTDIR)\g_monster.sbr" : $(SOURCE)\
 $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_phys.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj"	"$(INTDIR)\g_phys.sbr" : $(SOURCE) $(DEP_CPP_G_PHY)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj"	"$(INTDIR)\g_phys.sbr" : $(SOURCE) $(DEP_CPP_G_PHY)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_save.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj"	"$(INTDIR)\g_save.sbr" : $(SOURCE) $(DEP_CPP_G_SAV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj"	"$(INTDIR)\g_save.sbr" : $(SOURCE) $(DEP_CPP_G_SAV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_spawn.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj"	"$(INTDIR)\g_spawn.sbr" : $(SOURCE) $(DEP_CPP_G_SPA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj"	"$(INTDIR)\g_spawn.sbr" : $(SOURCE) $(DEP_CPP_G_SPA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_svcmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj"	"$(INTDIR)\g_svcmds.sbr" : $(SOURCE) $(DEP_CPP_G_SVC)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj"	"$(INTDIR)\g_svcmds.sbr" : $(SOURCE) $(DEP_CPP_G_SVC)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_target.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj"	"$(INTDIR)\g_target.sbr" : $(SOURCE) $(DEP_CPP_G_TAR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj"	"$(INTDIR)\g_target.sbr" : $(SOURCE) $(DEP_CPP_G_TAR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_trigger.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj"	"$(INTDIR)\g_trigger.sbr" : $(SOURCE)\
 $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj"	"$(INTDIR)\g_trigger.sbr" : $(SOURCE)\
 $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_turret.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj"	"$(INTDIR)\g_turret.sbr" : $(SOURCE) $(DEP_CPP_G_TUR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj"	"$(INTDIR)\g_turret.sbr" : $(SOURCE) $(DEP_CPP_G_TUR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_utils.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj"	"$(INTDIR)\g_utils.sbr" : $(SOURCE) $(DEP_CPP_G_UTI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj"	"$(INTDIR)\g_utils.sbr" : $(SOURCE) $(DEP_CPP_G_UTI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj"	"$(INTDIR)\g_weapon.sbr" : $(SOURCE) $(DEP_CPP_G_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj"	"$(INTDIR)\g_weapon.sbr" : $(SOURCE) $(DEP_CPP_G_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ENDIF 

SOURCE=.\leper_think.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_LEPER=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\leper_think.obj"	"$(INTDIR)\leper_think.sbr" : $(SOURCE)\
 $(DEP_CPP_LEPER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_LEPER=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\leper_think.obj"	"$(INTDIR)\leper_think.sbr" : $(SOURCE)\
 $(DEP_CPP_LEPER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_LEPER=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\leper_think.obj" : $(SOURCE) $(DEP_CPP_LEPER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_LEPER=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\leper_think.obj" : $(SOURCE) $(DEP_CPP_LEPER) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_actor.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj"	"$(INTDIR)\m_actor.sbr" : $(SOURCE) $(DEP_CPP_M_ACT)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj"	"$(INTDIR)\m_actor.sbr" : $(SOURCE) $(DEP_CPP_M_ACT)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\m_actor.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_berserk.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj"	"$(INTDIR)\m_berserk.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj"	"$(INTDIR)\m_berserk.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\m_berserk.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss2.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj"	"$(INTDIR)\m_boss2.sbr" : $(SOURCE) $(DEP_CPP_M_BOS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj"	"$(INTDIR)\m_boss2.sbr" : $(SOURCE) $(DEP_CPP_M_BOS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss2.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss3.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj"	"$(INTDIR)\m_boss3.sbr" : $(SOURCE) $(DEP_CPP_M_BOSS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj"	"$(INTDIR)\m_boss3.sbr" : $(SOURCE) $(DEP_CPP_M_BOSS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss31.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj"	"$(INTDIR)\m_boss31.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj"	"$(INTDIR)\m_boss31.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss31.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss32.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj"	"$(INTDIR)\m_boss32.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj"	"$(INTDIR)\m_boss32.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\m_boss32.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_brain.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj"	"$(INTDIR)\m_brain.sbr" : $(SOURCE) $(DEP_CPP_M_BRA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj"	"$(INTDIR)\m_brain.sbr" : $(SOURCE) $(DEP_CPP_M_BRA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\m_brain.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_chick.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj"	"$(INTDIR)\m_chick.sbr" : $(SOURCE) $(DEP_CPP_M_CHI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj"	"$(INTDIR)\m_chick.sbr" : $(SOURCE) $(DEP_CPP_M_CHI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\m_chick.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flash.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj"	"$(INTDIR)\m_flash.sbr" : $(SOURCE) $(DEP_CPP_M_FLA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj"	"$(INTDIR)\m_flash.sbr" : $(SOURCE) $(DEP_CPP_M_FLA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flipper.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj"	"$(INTDIR)\m_flipper.sbr" : $(SOURCE)\
 $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj"	"$(INTDIR)\m_flipper.sbr" : $(SOURCE)\
 $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_flipper.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_float.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj"	"$(INTDIR)\m_float.sbr" : $(SOURCE) $(DEP_CPP_M_FLO)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj"	"$(INTDIR)\m_float.sbr" : $(SOURCE) $(DEP_CPP_M_FLO)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\m_float.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flyer.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj"	"$(INTDIR)\m_flyer.sbr" : $(SOURCE) $(DEP_CPP_M_FLY)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj"	"$(INTDIR)\m_flyer.sbr" : $(SOURCE) $(DEP_CPP_M_FLY)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\m_flyer.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_gladiator.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj"	"$(INTDIR)\m_gladiator.sbr" : $(SOURCE)\
 $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj"	"$(INTDIR)\m_gladiator.sbr" : $(SOURCE)\
 $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\m_gladiator.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_gunner.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj"	"$(INTDIR)\m_gunner.sbr" : $(SOURCE) $(DEP_CPP_M_GUN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj"	"$(INTDIR)\m_gunner.sbr" : $(SOURCE) $(DEP_CPP_M_GUN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\m_gunner.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_hover.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj"	"$(INTDIR)\m_hover.sbr" : $(SOURCE) $(DEP_CPP_M_HOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj"	"$(INTDIR)\m_hover.sbr" : $(SOURCE) $(DEP_CPP_M_HOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\m_hover.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_infantry.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj"	"$(INTDIR)\m_infantry.sbr" : $(SOURCE)\
 $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj"	"$(INTDIR)\m_infantry.sbr" : $(SOURCE)\
 $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\m_infantry.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_insane.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj"	"$(INTDIR)\m_insane.sbr" : $(SOURCE) $(DEP_CPP_M_INS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj"	"$(INTDIR)\m_insane.sbr" : $(SOURCE) $(DEP_CPP_M_INS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\m_insane.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_medic.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj"	"$(INTDIR)\m_medic.sbr" : $(SOURCE) $(DEP_CPP_M_MED)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj"	"$(INTDIR)\m_medic.sbr" : $(SOURCE) $(DEP_CPP_M_MED)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\m_medic.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_move.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj"	"$(INTDIR)\m_move.sbr" : $(SOURCE) $(DEP_CPP_M_MOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj"	"$(INTDIR)\m_move.sbr" : $(SOURCE) $(DEP_CPP_M_MOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_mutant.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj"	"$(INTDIR)\m_mutant.sbr" : $(SOURCE) $(DEP_CPP_M_MUT)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj"	"$(INTDIR)\m_mutant.sbr" : $(SOURCE) $(DEP_CPP_M_MUT)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\m_mutant.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_parasite.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj"	"$(INTDIR)\m_parasite.sbr" : $(SOURCE)\
 $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj"	"$(INTDIR)\m_parasite.sbr" : $(SOURCE)\
 $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\m_parasite.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_soldier.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj"	"$(INTDIR)\m_soldier.sbr" : $(SOURCE)\
 $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj"	"$(INTDIR)\m_soldier.sbr" : $(SOURCE)\
 $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\m_soldier.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_supertank.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj"	"$(INTDIR)\m_supertank.sbr" : $(SOURCE)\
 $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj"	"$(INTDIR)\m_supertank.sbr" : $(SOURCE)\
 $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\m_supertank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_tank.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj"	"$(INTDIR)\m_tank.sbr" : $(SOURCE) $(DEP_CPP_M_TAN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj"	"$(INTDIR)\m_tank.sbr" : $(SOURCE) $(DEP_CPP_M_TAN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\m_tank.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ENDIF 

SOURCE=.\Matrix_Autobuy.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\Matrix_Autobuy.obj"	"$(INTDIR)\Matrix_Autobuy.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\Matrix_Autobuy.obj"	"$(INTDIR)\Matrix_Autobuy.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\Matrix_Autobuy.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\Matrix_Autobuy.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_cam.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cam.obj"	"$(INTDIR)\matrix_cam.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cam.obj"	"$(INTDIR)\matrix_cam.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cam.obj" : $(SOURCE) $(DEP_CPP_MATRIX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cam.obj" : $(SOURCE) $(DEP_CPP_MATRIX) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_cmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cmds.obj"	"$(INTDIR)\matrix_cmds.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cmds.obj"	"$(INTDIR)\matrix_cmds.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cmds.obj" : $(SOURCE) $(DEP_CPP_MATRIX_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_cmds.obj" : $(SOURCE) $(DEP_CPP_MATRIX_) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_findtarget.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_F=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_findtarget.obj"	"$(INTDIR)\matrix_findtarget.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_F) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_F=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_findtarget.obj"	"$(INTDIR)\matrix_findtarget.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_F) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_F=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_findtarget.obj" : $(SOURCE) $(DEP_CPP_MATRIX_F) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_F=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_findtarget.obj" : $(SOURCE) $(DEP_CPP_MATRIX_F) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_gweap.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_G=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_gweap.obj"	"$(INTDIR)\matrix_gweap.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_G) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_G=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_gweap.obj"	"$(INTDIR)\matrix_gweap.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_G) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_G=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_gweap.obj" : $(SOURCE) $(DEP_CPP_MATRIX_G) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_G=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_gweap.obj" : $(SOURCE) $(DEP_CPP_MATRIX_G) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_items.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_I=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_items.obj"	"$(INTDIR)\matrix_items.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_I) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_I=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_items.obj"	"$(INTDIR)\matrix_items.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_I) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_I=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_items.obj" : $(SOURCE) $(DEP_CPP_MATRIX_I) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_I=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_items.obj" : $(SOURCE) $(DEP_CPP_MATRIX_I) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_kungfu.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_K=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_kungfu.obj"	"$(INTDIR)\matrix_kungfu.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_K) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_K=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_kungfu.obj"	"$(INTDIR)\matrix_kungfu.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_K) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_K=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_kungfu.obj" : $(SOURCE) $(DEP_CPP_MATRIX_K) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_K=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_kungfu.obj" : $(SOURCE) $(DEP_CPP_MATRIX_K) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_locadam.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_L=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_locadam.obj"	"$(INTDIR)\matrix_locadam.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_L) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_L=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_locadam.obj"	"$(INTDIR)\matrix_locadam.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_L) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_L=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_locadam.obj" : $(SOURCE) $(DEP_CPP_MATRIX_L) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_L=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_locadam.obj" : $(SOURCE) $(DEP_CPP_MATRIX_L) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_main.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_M=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_main.obj"	"$(INTDIR)\matrix_main.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_M=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_main.obj"	"$(INTDIR)\matrix_main.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_M=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_main.obj" : $(SOURCE) $(DEP_CPP_MATRIX_M) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_M=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_main.obj" : $(SOURCE) $(DEP_CPP_MATRIX_M) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_match.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_MA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_match.obj"	"$(INTDIR)\matrix_match.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_MA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_match.obj"	"$(INTDIR)\matrix_match.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_MA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_match.obj" : $(SOURCE) $(DEP_CPP_MATRIX_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_MA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_match.obj" : $(SOURCE) $(DEP_CPP_MATRIX_MA) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_menu.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_ME=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_menu.obj"	"$(INTDIR)\matrix_menu.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_ME) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_ME=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_menu.obj"	"$(INTDIR)\matrix_menu.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_ME) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_ME=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_menu.obj" : $(SOURCE) $(DEP_CPP_MATRIX_ME) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_ME=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_menu.obj" : $(SOURCE) $(DEP_CPP_MATRIX_ME) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_misc.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_MI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_misc.obj"	"$(INTDIR)\matrix_misc.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_MI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_MI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_misc.obj"	"$(INTDIR)\matrix_misc.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_MI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_MI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_misc.obj" : $(SOURCE) $(DEP_CPP_MATRIX_MI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_MI=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_misc.obj" : $(SOURCE) $(DEP_CPP_MATRIX_MI) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_movement.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_MO=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_movement.obj"	"$(INTDIR)\matrix_movement.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_MO=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_movement.obj"	"$(INTDIR)\matrix_movement.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_MO=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_movement.obj" : $(SOURCE) $(DEP_CPP_MATRIX_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_MO=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_movement.obj" : $(SOURCE) $(DEP_CPP_MATRIX_MO) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_pweap.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_P=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_pweap.obj"	"$(INTDIR)\matrix_pweap.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_P) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_P=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_pweap.obj"	"$(INTDIR)\matrix_pweap.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_P) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_P=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_pweap.obj" : $(SOURCE) $(DEP_CPP_MATRIX_P) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_P=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_pweap.obj" : $(SOURCE) $(DEP_CPP_MATRIX_P) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_spells.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_S=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_spells.obj"	"$(INTDIR)\matrix_spells.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_S=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_spells.obj"	"$(INTDIR)\matrix_spells.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_S=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_spells.obj" : $(SOURCE) $(DEP_CPP_MATRIX_S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_S=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_spells.obj" : $(SOURCE) $(DEP_CPP_MATRIX_S) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_stamina.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_ST=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_stamina.obj"	"$(INTDIR)\matrix_stamina.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_ST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_ST=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_stamina.obj"	"$(INTDIR)\matrix_stamina.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_ST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_ST=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_stamina.obj" : $(SOURCE) $(DEP_CPP_MATRIX_ST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_ST=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_stamina.obj" : $(SOURCE) $(DEP_CPP_MATRIX_ST) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_tank.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_T=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_tank.obj"	"$(INTDIR)\matrix_tank.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_T) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_T=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_tank.obj"	"$(INTDIR)\matrix_tank.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_T) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_T=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_tank.obj" : $(SOURCE) $(DEP_CPP_MATRIX_T) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_T=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_tank.obj" : $(SOURCE) $(DEP_CPP_MATRIX_T) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix_team.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MATRIX_TE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_team.obj"	"$(INTDIR)\matrix_team.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MATRIX_TE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_team.obj"	"$(INTDIR)\matrix_team.sbr" : $(SOURCE)\
 $(DEP_CPP_MATRIX_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MATRIX_TE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_team.obj" : $(SOURCE) $(DEP_CPP_MATRIX_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MATRIX_TE=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\matrix_team.obj" : $(SOURCE) $(DEP_CPP_MATRIX_TE) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_chase.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_chase.obj"	"$(INTDIR)\p_chase.sbr" : $(SOURCE) $(DEP_CPP_P_CHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_chase.obj"	"$(INTDIR)\p_chase.sbr" : $(SOURCE) $(DEP_CPP_P_CHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_chase.obj" : $(SOURCE) $(DEP_CPP_P_CHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_chase.obj" : $(SOURCE) $(DEP_CPP_P_CHA) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_client.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj"	"$(INTDIR)\p_client.sbr" : $(SOURCE) $(DEP_CPP_P_CLI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj"	"$(INTDIR)\p_client.sbr" : $(SOURCE) $(DEP_CPP_P_CLI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_CLI=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_hud.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj"	"$(INTDIR)\p_hud.sbr" : $(SOURCE) $(DEP_CPP_P_HUD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj"	"$(INTDIR)\p_hud.sbr" : $(SOURCE) $(DEP_CPP_P_HUD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_menu.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_MEN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_menu.obj"	"$(INTDIR)\p_menu.sbr" : $(SOURCE) $(DEP_CPP_P_MEN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_MEN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_menu.obj"	"$(INTDIR)\p_menu.sbr" : $(SOURCE) $(DEP_CPP_P_MEN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_MEN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_menu.obj" : $(SOURCE) $(DEP_CPP_P_MEN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_MEN=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_menu.obj" : $(SOURCE) $(DEP_CPP_P_MEN) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_trail.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj"	"$(INTDIR)\p_trail.sbr" : $(SOURCE) $(DEP_CPP_P_TRA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj"	"$(INTDIR)\p_trail.sbr" : $(SOURCE) $(DEP_CPP_P_TRA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_view.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj"	"$(INTDIR)\p_view.sbr" : $(SOURCE) $(DEP_CPP_P_VIE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj"	"$(INTDIR)\p_view.sbr" : $(SOURCE) $(DEP_CPP_P_VIE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj"	"$(INTDIR)\p_weapon.sbr" : $(SOURCE) $(DEP_CPP_P_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj"	"$(INTDIR)\p_weapon.sbr" : $(SOURCE) $(DEP_CPP_P_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\matrix.h"\
	".\matrix_def.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ENDIF 

SOURCE=.\q_shared.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj"	"$(INTDIR)\q_shared.sbr" : $(SOURCE) $(DEP_CPP_Q_SHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj"	"$(INTDIR)\q_shared.sbr" : $(SOURCE) $(DEP_CPP_Q_SHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ENDIF 


!ENDIF 

