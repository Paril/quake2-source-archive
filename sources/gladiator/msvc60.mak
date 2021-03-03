# Microsoft Developer Studio Generated NMAKE File, Based on game.dsp
!IF "$(CFG)" == ""
CFG=game - Win32 Debug
!MESSAGE No configuration specified. Defaulting to game - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "game - Win32 Release" && "$(CFG)" != "game - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="game - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "game - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "game - Win32 Release"

OUTDIR=e:\quake2\gladiator
INTDIR=.\Release
# Begin Custom Macros
OutDir=e:\quake2\gladiator
# End Custom Macros

ALL : "$(OUTDIR)\gamex86.dll"


CLEAN :
	-@erase "$(INTDIR)\bl_botcfg.obj"
	-@erase "$(INTDIR)\bl_cmd.obj"
	-@erase "$(INTDIR)\bl_debug.obj"
	-@erase "$(INTDIR)\bl_main.obj"
	-@erase "$(INTDIR)\bl_redirgi.obj"
	-@erase "$(INTDIR)\bl_spawn.obj"
	-@erase "$(INTDIR)\dm_ball_rogue.obj"
	-@erase "$(INTDIR)\dm_tag_rogue.obj"
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_arena.obj"
	-@erase "$(INTDIR)\g_ch.obj"
	-@erase "$(INTDIR)\g_chase.obj"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_ctf.obj"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_log.obj"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_newai_rogue.obj"
	-@erase "$(INTDIR)\g_newdm_rogue.obj"
	-@erase "$(INTDIR)\g_newfnc_rogue.obj"
	-@erase "$(INTDIR)\g_newtarg_rogue.obj"
	-@erase "$(INTDIR)\g_newtrig_rogue.obj"
	-@erase "$(INTDIR)\g_newweap_rogue.obj"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_sphere_rogue.obj"
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
	-@erase "$(INTDIR)\m_boss5_xatrix.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_carrier_rogue.obj"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_fixbot_xatrix.obj"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_gekk_xatrix.obj"
	-@erase "$(INTDIR)\m_gladb_xatrix.obj"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_move2_rogue.obj"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_stalker_rogue.obj"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\m_turret_rogue.obj"
	-@erase "$(INTDIR)\m_widow2_rogue.obj"
	-@erase "$(INTDIR)\m_widow_rogue.obj"
	-@erase "$(INTDIR)\p_botmenu.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_lag.obj"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_menulib.obj"
	-@erase "$(INTDIR)\p_observer.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\gamex86.dll"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb" /machine:I386 /def:"E:\QuakeDLL\Q2bot\game.def" /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	"E:\QuakeDLL\Q2bot\game.def"
LINK32_OBJS= \
	"$(INTDIR)\bl_botcfg.obj" \
	"$(INTDIR)\bl_cmd.obj" \
	"$(INTDIR)\bl_debug.obj" \
	"$(INTDIR)\bl_main.obj" \
	"$(INTDIR)\bl_redirgi.obj" \
	"$(INTDIR)\bl_spawn.obj" \
	"$(INTDIR)\dm_ball_rogue.obj" \
	"$(INTDIR)\dm_tag_rogue.obj" \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_arena.obj" \
	"$(INTDIR)\g_ch.obj" \
	"$(INTDIR)\g_chase.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_ctf.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_log.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_newai_rogue.obj" \
	"$(INTDIR)\g_newdm_rogue.obj" \
	"$(INTDIR)\g_newfnc_rogue.obj" \
	"$(INTDIR)\g_newtarg_rogue.obj" \
	"$(INTDIR)\g_newtrig_rogue.obj" \
	"$(INTDIR)\g_newweap_rogue.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_sphere_rogue.obj" \
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
	"$(INTDIR)\m_boss5_xatrix.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_carrier_rogue.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_fixbot_xatrix.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gekk_xatrix.obj" \
	"$(INTDIR)\m_gladb_xatrix.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_move2_rogue.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_stalker_rogue.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\m_turret_rogue.obj" \
	"$(INTDIR)\m_widow2_rogue.obj" \
	"$(INTDIR)\m_widow_rogue.obj" \
	"$(INTDIR)\p_botmenu.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_lag.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_menulib.obj" \
	"$(INTDIR)\p_observer.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

OUTDIR=e:\quake2\gladiator
INTDIR=.\Debug
# Begin Custom Macros
OutDir=e:\quake2\gladiator
# End Custom Macros

ALL : "$(OUTDIR)\gamex86.dll"


CLEAN :
	-@erase "$(INTDIR)\bl_botcfg.obj"
	-@erase "$(INTDIR)\bl_cmd.obj"
	-@erase "$(INTDIR)\bl_debug.obj"
	-@erase "$(INTDIR)\bl_main.obj"
	-@erase "$(INTDIR)\bl_redirgi.obj"
	-@erase "$(INTDIR)\bl_spawn.obj"
	-@erase "$(INTDIR)\dm_ball_rogue.obj"
	-@erase "$(INTDIR)\dm_tag_rogue.obj"
	-@erase "$(INTDIR)\g_ai.obj"
	-@erase "$(INTDIR)\g_arena.obj"
	-@erase "$(INTDIR)\g_ch.obj"
	-@erase "$(INTDIR)\g_chase.obj"
	-@erase "$(INTDIR)\g_cmds.obj"
	-@erase "$(INTDIR)\g_combat.obj"
	-@erase "$(INTDIR)\g_ctf.obj"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_log.obj"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_newai_rogue.obj"
	-@erase "$(INTDIR)\g_newdm_rogue.obj"
	-@erase "$(INTDIR)\g_newfnc_rogue.obj"
	-@erase "$(INTDIR)\g_newtarg_rogue.obj"
	-@erase "$(INTDIR)\g_newtrig_rogue.obj"
	-@erase "$(INTDIR)\g_newweap_rogue.obj"
	-@erase "$(INTDIR)\g_phys.obj"
	-@erase "$(INTDIR)\g_save.obj"
	-@erase "$(INTDIR)\g_spawn.obj"
	-@erase "$(INTDIR)\g_sphere_rogue.obj"
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
	-@erase "$(INTDIR)\m_boss5_xatrix.obj"
	-@erase "$(INTDIR)\m_brain.obj"
	-@erase "$(INTDIR)\m_carrier_rogue.obj"
	-@erase "$(INTDIR)\m_chick.obj"
	-@erase "$(INTDIR)\m_fixbot_xatrix.obj"
	-@erase "$(INTDIR)\m_flash.obj"
	-@erase "$(INTDIR)\m_flipper.obj"
	-@erase "$(INTDIR)\m_float.obj"
	-@erase "$(INTDIR)\m_flyer.obj"
	-@erase "$(INTDIR)\m_gekk_xatrix.obj"
	-@erase "$(INTDIR)\m_gladb_xatrix.obj"
	-@erase "$(INTDIR)\m_gladiator.obj"
	-@erase "$(INTDIR)\m_gunner.obj"
	-@erase "$(INTDIR)\m_hover.obj"
	-@erase "$(INTDIR)\m_infantry.obj"
	-@erase "$(INTDIR)\m_insane.obj"
	-@erase "$(INTDIR)\m_medic.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_move2_rogue.obj"
	-@erase "$(INTDIR)\m_mutant.obj"
	-@erase "$(INTDIR)\m_parasite.obj"
	-@erase "$(INTDIR)\m_soldier.obj"
	-@erase "$(INTDIR)\m_stalker_rogue.obj"
	-@erase "$(INTDIR)\m_supertank.obj"
	-@erase "$(INTDIR)\m_tank.obj"
	-@erase "$(INTDIR)\m_turret_rogue.obj"
	-@erase "$(INTDIR)\m_widow2_rogue.obj"
	-@erase "$(INTDIR)\m_widow_rogue.obj"
	-@erase "$(INTDIR)\p_botmenu.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_lag.obj"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_menulib.obj"
	-@erase "$(INTDIR)\p_observer.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\gamex86.dll"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "$(OUTDIR)\gamex86.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb" /debug /machine:I386 /def:"E:\QuakeDLL\Q2bot\game.def" /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" /pdbtype:sept 
DEF_FILE= \
	"E:\QuakeDLL\Q2bot\game.def"
LINK32_OBJS= \
	"$(INTDIR)\bl_botcfg.obj" \
	"$(INTDIR)\bl_cmd.obj" \
	"$(INTDIR)\bl_debug.obj" \
	"$(INTDIR)\bl_main.obj" \
	"$(INTDIR)\bl_redirgi.obj" \
	"$(INTDIR)\bl_spawn.obj" \
	"$(INTDIR)\dm_ball_rogue.obj" \
	"$(INTDIR)\dm_tag_rogue.obj" \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_arena.obj" \
	"$(INTDIR)\g_ch.obj" \
	"$(INTDIR)\g_chase.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_ctf.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_log.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_newai_rogue.obj" \
	"$(INTDIR)\g_newdm_rogue.obj" \
	"$(INTDIR)\g_newfnc_rogue.obj" \
	"$(INTDIR)\g_newtarg_rogue.obj" \
	"$(INTDIR)\g_newtrig_rogue.obj" \
	"$(INTDIR)\g_newweap_rogue.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_sphere_rogue.obj" \
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
	"$(INTDIR)\m_boss5_xatrix.obj" \
	"$(INTDIR)\m_brain.obj" \
	"$(INTDIR)\m_carrier_rogue.obj" \
	"$(INTDIR)\m_chick.obj" \
	"$(INTDIR)\m_fixbot_xatrix.obj" \
	"$(INTDIR)\m_flash.obj" \
	"$(INTDIR)\m_flipper.obj" \
	"$(INTDIR)\m_float.obj" \
	"$(INTDIR)\m_flyer.obj" \
	"$(INTDIR)\m_gekk_xatrix.obj" \
	"$(INTDIR)\m_gladb_xatrix.obj" \
	"$(INTDIR)\m_gladiator.obj" \
	"$(INTDIR)\m_gunner.obj" \
	"$(INTDIR)\m_hover.obj" \
	"$(INTDIR)\m_infantry.obj" \
	"$(INTDIR)\m_insane.obj" \
	"$(INTDIR)\m_medic.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\m_move2_rogue.obj" \
	"$(INTDIR)\m_mutant.obj" \
	"$(INTDIR)\m_parasite.obj" \
	"$(INTDIR)\m_soldier.obj" \
	"$(INTDIR)\m_stalker_rogue.obj" \
	"$(INTDIR)\m_supertank.obj" \
	"$(INTDIR)\m_tank.obj" \
	"$(INTDIR)\m_turret_rogue.obj" \
	"$(INTDIR)\m_widow2_rogue.obj" \
	"$(INTDIR)\m_widow_rogue.obj" \
	"$(INTDIR)\p_botmenu.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_lag.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_menulib.obj" \
	"$(INTDIR)\p_observer.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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


!IF "$(CFG)" == "game - Win32 Release" || "$(CFG)" == "game - Win32 Debug"
SOURCE=E:\QuakeDLL\Q2bot\bl_botcfg.c

"$(INTDIR)\bl_botcfg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\bl_cmd.c

"$(INTDIR)\bl_cmd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\bl_debug.c

"$(INTDIR)\bl_debug.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\bl_main.c

"$(INTDIR)\bl_main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\bl_redirgi.c

"$(INTDIR)\bl_redirgi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\bl_spawn.c

"$(INTDIR)\bl_spawn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\dm_ball_rogue.c

"$(INTDIR)\dm_ball_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\dm_tag_rogue.c

"$(INTDIR)\dm_tag_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_ai.c

"$(INTDIR)\g_ai.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_arena.c

"$(INTDIR)\g_arena.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_ch.c

"$(INTDIR)\g_ch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_chase.c

"$(INTDIR)\g_chase.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_cmds.c

"$(INTDIR)\g_cmds.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_combat.c

"$(INTDIR)\g_combat.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_ctf.c

"$(INTDIR)\g_ctf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_func.c

"$(INTDIR)\g_func.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_items.c

"$(INTDIR)\g_items.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_log.c

"$(INTDIR)\g_log.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_main.c

"$(INTDIR)\g_main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_misc.c

"$(INTDIR)\g_misc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_monster.c

"$(INTDIR)\g_monster.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_newai_rogue.c

"$(INTDIR)\g_newai_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_newdm_rogue.c

"$(INTDIR)\g_newdm_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_newfnc_rogue.c

"$(INTDIR)\g_newfnc_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_newtarg_rogue.c

"$(INTDIR)\g_newtarg_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_newtrig_rogue.c

"$(INTDIR)\g_newtrig_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_newweap_rogue.c

"$(INTDIR)\g_newweap_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_phys.c

"$(INTDIR)\g_phys.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_save.c

"$(INTDIR)\g_save.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_spawn.c

"$(INTDIR)\g_spawn.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_sphere_rogue.c

"$(INTDIR)\g_sphere_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_svcmds.c

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_target.c

"$(INTDIR)\g_target.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_trigger.c

"$(INTDIR)\g_trigger.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_turret.c

"$(INTDIR)\g_turret.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_utils.c

"$(INTDIR)\g_utils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\g_weapon.c

"$(INTDIR)\g_weapon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_actor.c

"$(INTDIR)\m_actor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_berserk.c

"$(INTDIR)\m_berserk.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_boss2.c

"$(INTDIR)\m_boss2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_boss3.c

"$(INTDIR)\m_boss3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_boss31.c

"$(INTDIR)\m_boss31.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_boss32.c

"$(INTDIR)\m_boss32.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_boss5_xatrix.c

"$(INTDIR)\m_boss5_xatrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_brain.c

"$(INTDIR)\m_brain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_carrier_rogue.c

"$(INTDIR)\m_carrier_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_chick.c

"$(INTDIR)\m_chick.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_fixbot_xatrix.c

"$(INTDIR)\m_fixbot_xatrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_flash.c

"$(INTDIR)\m_flash.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_flipper.c

"$(INTDIR)\m_flipper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_float.c

"$(INTDIR)\m_float.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_flyer.c

"$(INTDIR)\m_flyer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_gekk_xatrix.c

"$(INTDIR)\m_gekk_xatrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_gladb_xatrix.c

"$(INTDIR)\m_gladb_xatrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_gladiator.c

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_gunner.c

"$(INTDIR)\m_gunner.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_hover.c

"$(INTDIR)\m_hover.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_infantry.c

"$(INTDIR)\m_infantry.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_insane.c

"$(INTDIR)\m_insane.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_medic.c

"$(INTDIR)\m_medic.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_move.c

"$(INTDIR)\m_move.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_move2_rogue.c

"$(INTDIR)\m_move2_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_mutant.c

"$(INTDIR)\m_mutant.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_parasite.c

"$(INTDIR)\m_parasite.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_soldier.c

"$(INTDIR)\m_soldier.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_stalker_rogue.c

"$(INTDIR)\m_stalker_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_supertank.c

"$(INTDIR)\m_supertank.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_tank.c

"$(INTDIR)\m_tank.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_turret_rogue.c

"$(INTDIR)\m_turret_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_widow2_rogue.c

"$(INTDIR)\m_widow2_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\m_widow_rogue.c

"$(INTDIR)\m_widow_rogue.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_botmenu.c

"$(INTDIR)\p_botmenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_client.c

"$(INTDIR)\p_client.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_hud.c

"$(INTDIR)\p_hud.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_lag.c

"$(INTDIR)\p_lag.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_menu.c

"$(INTDIR)\p_menu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_menulib.c

"$(INTDIR)\p_menulib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_observer.c

"$(INTDIR)\p_observer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_trail.c

"$(INTDIR)\p_trail.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_view.c

"$(INTDIR)\p_view.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\p_weapon.c

"$(INTDIR)\p_weapon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=E:\QuakeDLL\Q2bot\q_shared.c

"$(INTDIR)\q_shared.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

