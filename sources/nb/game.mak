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

OUTDIR=.\..
INTDIR=.\release
# Begin Custom Macros
OutDir=.\..\ 
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\release\gamex86.dll"

!ELSE 

ALL : "$(OUTDIR)\release\gamex86.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\camclient.obj"
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
	-@erase "$(INTDIR)\ini.obj"
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
	-@erase "$(INTDIR)\nb_backprop2.obj"
	-@erase "$(INTDIR)\nb_explore.obj"
	-@erase "$(INTDIR)\nb_game.obj"
	-@erase "$(INTDIR)\nb_GAonechild.obj"
	-@erase "$(INTDIR)\nb_geneticalgorithm.obj"
	-@erase "$(INTDIR)\nb_imitation.obj"
	-@erase "$(INTDIR)\nb_in.obj"
	-@erase "$(INTDIR)\nb_model2.obj"
	-@erase "$(INTDIR)\nb_move.obj"
	-@erase "$(INTDIR)\nb_nnet.obj"
	-@erase "$(INTDIR)\nb_out.obj"
	-@erase "$(INTDIR)\nb_save.obj"
	-@erase "$(INTDIR)\nb_spawn.obj"
	-@erase "$(INTDIR)\nb_structGA.obj"
	-@erase "$(INTDIR)\nb_test.obj"
	-@erase "$(INTDIR)\nb_think.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "$(OUTDIR)\release\gamex86.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /MT /W2 /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\release/
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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib winmm.lib /nologo /base:"0x20000000"\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb"\
 /machine:I386 /def:".\game.def" /out:"$(OUTDIR)\release\gamex86.dll"\
 /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\camclient.obj" \
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
	"$(INTDIR)\ini.obj" \
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
	"$(INTDIR)\nb_backprop2.obj" \
	"$(INTDIR)\nb_explore.obj" \
	"$(INTDIR)\nb_game.obj" \
	"$(INTDIR)\nb_GAonechild.obj" \
	"$(INTDIR)\nb_geneticalgorithm.obj" \
	"$(INTDIR)\nb_imitation.obj" \
	"$(INTDIR)\nb_in.obj" \
	"$(INTDIR)\nb_model2.obj" \
	"$(INTDIR)\nb_move.obj" \
	"$(INTDIR)\nb_nnet.obj" \
	"$(INTDIR)\nb_out.obj" \
	"$(INTDIR)\nb_save.obj" \
	"$(INTDIR)\nb_spawn.obj" \
	"$(INTDIR)\nb_structGA.obj" \
	"$(INTDIR)\nb_test.obj" \
	"$(INTDIR)\nb_think.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj"

"$(OUTDIR)\release\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
	-@erase "$(INTDIR)\camclient.obj"
	-@erase "$(INTDIR)\camclient.sbr"
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
	-@erase "$(INTDIR)\ini.obj"
	-@erase "$(INTDIR)\ini.sbr"
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
	-@erase "$(INTDIR)\nb_backprop.obj"
	-@erase "$(INTDIR)\nb_backprop.sbr"
	-@erase "$(INTDIR)\nb_backprop2.obj"
	-@erase "$(INTDIR)\nb_backprop2.sbr"
	-@erase "$(INTDIR)\nb_explore.obj"
	-@erase "$(INTDIR)\nb_explore.sbr"
	-@erase "$(INTDIR)\nb_game.obj"
	-@erase "$(INTDIR)\nb_game.sbr"
	-@erase "$(INTDIR)\nb_GAonechild.obj"
	-@erase "$(INTDIR)\nb_GAonechild.sbr"
	-@erase "$(INTDIR)\nb_geneticalgorithm.obj"
	-@erase "$(INTDIR)\nb_geneticalgorithm.sbr"
	-@erase "$(INTDIR)\nb_imitation.obj"
	-@erase "$(INTDIR)\nb_imitation.sbr"
	-@erase "$(INTDIR)\nb_in.obj"
	-@erase "$(INTDIR)\nb_in.sbr"
	-@erase "$(INTDIR)\nb_model2.obj"
	-@erase "$(INTDIR)\nb_model2.sbr"
	-@erase "$(INTDIR)\nb_move.obj"
	-@erase "$(INTDIR)\nb_move.sbr"
	-@erase "$(INTDIR)\nb_nnet.obj"
	-@erase "$(INTDIR)\nb_nnet.sbr"
	-@erase "$(INTDIR)\nb_old.obj"
	-@erase "$(INTDIR)\nb_old.sbr"
	-@erase "$(INTDIR)\nb_out.obj"
	-@erase "$(INTDIR)\nb_out.sbr"
	-@erase "$(INTDIR)\nb_save.obj"
	-@erase "$(INTDIR)\nb_save.sbr"
	-@erase "$(INTDIR)\nb_spawn.obj"
	-@erase "$(INTDIR)\nb_spawn.sbr"
	-@erase "$(INTDIR)\nb_structGA.obj"
	-@erase "$(INTDIR)\nb_structGA.sbr"
	-@erase "$(INTDIR)\nb_test.obj"
	-@erase "$(INTDIR)\nb_test.sbr"
	-@erase "$(INTDIR)\nb_think.obj"
	-@erase "$(INTDIR)\nb_think.sbr"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_client.sbr"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_hud.sbr"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_trail.sbr"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_view.sbr"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\p_weapon.sbr"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\q_shared.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
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
CPP_PROJ=/nologo /G5 /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "BUILDING_REF_GL" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\game.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
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
	"$(INTDIR)\camclient.sbr" \
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
	"$(INTDIR)\ini.sbr" \
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
	"$(INTDIR)\nb_backprop.sbr" \
	"$(INTDIR)\nb_backprop2.sbr" \
	"$(INTDIR)\nb_explore.sbr" \
	"$(INTDIR)\nb_game.sbr" \
	"$(INTDIR)\nb_GAonechild.sbr" \
	"$(INTDIR)\nb_geneticalgorithm.sbr" \
	"$(INTDIR)\nb_imitation.sbr" \
	"$(INTDIR)\nb_in.sbr" \
	"$(INTDIR)\nb_model2.sbr" \
	"$(INTDIR)\nb_move.sbr" \
	"$(INTDIR)\nb_nnet.sbr" \
	"$(INTDIR)\nb_old.sbr" \
	"$(INTDIR)\nb_out.sbr" \
	"$(INTDIR)\nb_save.sbr" \
	"$(INTDIR)\nb_spawn.sbr" \
	"$(INTDIR)\nb_structGA.sbr" \
	"$(INTDIR)\nb_test.sbr" \
	"$(INTDIR)\nb_think.sbr" \
	"$(INTDIR)\p_client.sbr" \
	"$(INTDIR)\p_hud.sbr" \
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
	"$(INTDIR)\camclient.obj" \
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
	"$(INTDIR)\ini.obj" \
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
	"$(INTDIR)\nb_backprop.obj" \
	"$(INTDIR)\nb_backprop2.obj" \
	"$(INTDIR)\nb_explore.obj" \
	"$(INTDIR)\nb_game.obj" \
	"$(INTDIR)\nb_GAonechild.obj" \
	"$(INTDIR)\nb_geneticalgorithm.obj" \
	"$(INTDIR)\nb_imitation.obj" \
	"$(INTDIR)\nb_in.obj" \
	"$(INTDIR)\nb_model2.obj" \
	"$(INTDIR)\nb_move.obj" \
	"$(INTDIR)\nb_nnet.obj" \
	"$(INTDIR)\nb_old.obj" \
	"$(INTDIR)\nb_out.obj" \
	"$(INTDIR)\nb_save.obj" \
	"$(INTDIR)\nb_spawn.obj" \
	"$(INTDIR)\nb_structGA.obj" \
	"$(INTDIR)\nb_test.obj" \
	"$(INTDIR)\nb_think.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
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
	-@erase "$(INTDIR)\camclient.obj"
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
	-@erase "$(INTDIR)\ini.obj"
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
	-@erase "$(INTDIR)\nb_backprop.obj"
	-@erase "$(INTDIR)\nb_backprop2.obj"
	-@erase "$(INTDIR)\nb_explore.obj"
	-@erase "$(INTDIR)\nb_game.obj"
	-@erase "$(INTDIR)\nb_GAonechild.obj"
	-@erase "$(INTDIR)\nb_geneticalgorithm.obj"
	-@erase "$(INTDIR)\nb_imitation.obj"
	-@erase "$(INTDIR)\nb_in.obj"
	-@erase "$(INTDIR)\nb_model2.obj"
	-@erase "$(INTDIR)\nb_move.obj"
	-@erase "$(INTDIR)\nb_nnet.obj"
	-@erase "$(INTDIR)\nb_old.obj"
	-@erase "$(INTDIR)\nb_out.obj"
	-@erase "$(INTDIR)\nb_save.obj"
	-@erase "$(INTDIR)\nb_spawn.obj"
	-@erase "$(INTDIR)\nb_structGA.obj"
	-@erase "$(INTDIR)\nb_test.obj"
	-@erase "$(INTDIR)\nb_think.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
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
	"$(INTDIR)\camclient.obj" \
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
	"$(INTDIR)\ini.obj" \
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
	"$(INTDIR)\nb_backprop.obj" \
	"$(INTDIR)\nb_backprop2.obj" \
	"$(INTDIR)\nb_explore.obj" \
	"$(INTDIR)\nb_game.obj" \
	"$(INTDIR)\nb_GAonechild.obj" \
	"$(INTDIR)\nb_geneticalgorithm.obj" \
	"$(INTDIR)\nb_imitation.obj" \
	"$(INTDIR)\nb_in.obj" \
	"$(INTDIR)\nb_model2.obj" \
	"$(INTDIR)\nb_move.obj" \
	"$(INTDIR)\nb_nnet.obj" \
	"$(INTDIR)\nb_old.obj" \
	"$(INTDIR)\nb_out.obj" \
	"$(INTDIR)\nb_save.obj" \
	"$(INTDIR)\nb_spawn.obj" \
	"$(INTDIR)\nb_structGA.obj" \
	"$(INTDIR)\nb_test.obj" \
	"$(INTDIR)\nb_think.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
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
	-@erase "$(INTDIR)\camclient.obj"
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
	-@erase "$(INTDIR)\ini.obj"
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
	-@erase "$(INTDIR)\nb_backprop.obj"
	-@erase "$(INTDIR)\nb_backprop2.obj"
	-@erase "$(INTDIR)\nb_explore.obj"
	-@erase "$(INTDIR)\nb_game.obj"
	-@erase "$(INTDIR)\nb_GAonechild.obj"
	-@erase "$(INTDIR)\nb_geneticalgorithm.obj"
	-@erase "$(INTDIR)\nb_imitation.obj"
	-@erase "$(INTDIR)\nb_in.obj"
	-@erase "$(INTDIR)\nb_model2.obj"
	-@erase "$(INTDIR)\nb_move.obj"
	-@erase "$(INTDIR)\nb_nnet.obj"
	-@erase "$(INTDIR)\nb_old.obj"
	-@erase "$(INTDIR)\nb_out.obj"
	-@erase "$(INTDIR)\nb_save.obj"
	-@erase "$(INTDIR)\nb_spawn.obj"
	-@erase "$(INTDIR)\nb_structGA.obj"
	-@erase "$(INTDIR)\nb_test.obj"
	-@erase "$(INTDIR)\nb_think.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
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
	"$(INTDIR)\camclient.obj" \
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
	"$(INTDIR)\ini.obj" \
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
	"$(INTDIR)\nb_backprop.obj" \
	"$(INTDIR)\nb_backprop2.obj" \
	"$(INTDIR)\nb_explore.obj" \
	"$(INTDIR)\nb_game.obj" \
	"$(INTDIR)\nb_GAonechild.obj" \
	"$(INTDIR)\nb_geneticalgorithm.obj" \
	"$(INTDIR)\nb_imitation.obj" \
	"$(INTDIR)\nb_in.obj" \
	"$(INTDIR)\nb_model2.obj" \
	"$(INTDIR)\nb_move.obj" \
	"$(INTDIR)\nb_nnet.obj" \
	"$(INTDIR)\nb_old.obj" \
	"$(INTDIR)\nb_out.obj" \
	"$(INTDIR)\nb_save.obj" \
	"$(INTDIR)\nb_spawn.obj" \
	"$(INTDIR)\nb_structGA.obj" \
	"$(INTDIR)\nb_test.obj" \
	"$(INTDIR)\nb_think.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
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
SOURCE=.\camclient.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_CAMCL=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\camclient.obj" : $(SOURCE) $(DEP_CPP_CAMCL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_CAMCL=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\camclient.obj"	"$(INTDIR)\camclient.sbr" : $(SOURCE)\
 $(DEP_CPP_CAMCL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_CAMCL=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\camclient.obj" : $(SOURCE) $(DEP_CPP_CAMCL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_CAMCL=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\camclient.obj" : $(SOURCE) $(DEP_CPP_CAMCL) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_ai.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj"	"$(INTDIR)\g_ai.sbr" : $(SOURCE) $(DEP_CPP_G_AI_)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_chase.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj" : $(SOURCE) $(DEP_CPP_G_CHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj"	"$(INTDIR)\g_chase.sbr" : $(SOURCE) $(DEP_CPP_G_CHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj" : $(SOURCE) $(DEP_CPP_G_CHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_CHA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj" : $(SOURCE) $(DEP_CPP_G_CHA) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_cmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_CMD=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_CMD=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj"	"$(INTDIR)\g_cmds.sbr" : $(SOURCE) $(DEP_CPP_G_CMD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_CMD=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_CMD=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_combat.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_COM=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_COM=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj"	"$(INTDIR)\g_combat.sbr" : $(SOURCE) $(DEP_CPP_G_COM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_COM=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_COM=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_func.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj"	"$(INTDIR)\g_func.sbr" : $(SOURCE) $(DEP_CPP_G_FUN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_items.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj"	"$(INTDIR)\g_items.sbr" : $(SOURCE) $(DEP_CPP_G_ITE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_ITE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_main.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MAI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MAI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj"	"$(INTDIR)\g_main.sbr" : $(SOURCE) $(DEP_CPP_G_MAI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MAI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MAI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_misc.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj"	"$(INTDIR)\g_misc.sbr" : $(SOURCE) $(DEP_CPP_G_MIS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MIS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_monster.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj"	"$(INTDIR)\g_monster.sbr" : $(SOURCE)\
 $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_phys.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj"	"$(INTDIR)\g_phys.sbr" : $(SOURCE) $(DEP_CPP_G_PHY)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_save.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj"	"$(INTDIR)\g_save.sbr" : $(SOURCE) $(DEP_CPP_G_SAV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SAV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_spawn.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj"	"$(INTDIR)\g_spawn.sbr" : $(SOURCE) $(DEP_CPP_G_SPA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SPA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_svcmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj"	"$(INTDIR)\g_svcmds.sbr" : $(SOURCE) $(DEP_CPP_G_SVC)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_target.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj"	"$(INTDIR)\g_target.sbr" : $(SOURCE) $(DEP_CPP_G_TAR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_trigger.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj"	"$(INTDIR)\g_trigger.sbr" : $(SOURCE)\
 $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_turret.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj"	"$(INTDIR)\g_turret.sbr" : $(SOURCE) $(DEP_CPP_G_TUR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_utils.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj"	"$(INTDIR)\g_utils.sbr" : $(SOURCE) $(DEP_CPP_G_UTI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj"	"$(INTDIR)\g_weapon.sbr" : $(SOURCE) $(DEP_CPP_G_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ENDIF 

SOURCE=.\ini.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_INI_C=\
	".\ini.h"\
	

"$(INTDIR)\ini.obj" : $(SOURCE) $(DEP_CPP_INI_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_INI_C=\
	".\ini.h"\
	

"$(INTDIR)\ini.obj"	"$(INTDIR)\ini.sbr" : $(SOURCE) $(DEP_CPP_INI_C)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_INI_C=\
	".\ini.h"\
	

"$(INTDIR)\ini.obj" : $(SOURCE) $(DEP_CPP_INI_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_INI_C=\
	".\ini.h"\
	

"$(INTDIR)\ini.obj" : $(SOURCE) $(DEP_CPP_INI_C) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_actor.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_actor.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_actor.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj"	"$(INTDIR)\m_actor.sbr" : $(SOURCE) $(DEP_CPP_M_ACT)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_actor.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_ACT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_actor.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_actor.obj" : $(SOURCE) $(DEP_CPP_M_ACT) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_berserk.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_berserk.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_berserk.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj"	"$(INTDIR)\m_berserk.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_berserk.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BER=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_berserk.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_berserk.obj" : $(SOURCE) $(DEP_CPP_M_BER) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss2.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss2.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss2.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj"	"$(INTDIR)\m_boss2.sbr" : $(SOURCE) $(DEP_CPP_M_BOS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss2.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss2.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss2.obj" : $(SOURCE) $(DEP_CPP_M_BOS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss3.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj"	"$(INTDIR)\m_boss3.sbr" : $(SOURCE) $(DEP_CPP_M_BOSS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss3.obj" : $(SOURCE) $(DEP_CPP_M_BOSS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss31.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss31.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss31.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj"	"$(INTDIR)\m_boss31.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss31.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS3=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss31.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss31.obj" : $(SOURCE) $(DEP_CPP_M_BOSS3) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_boss32.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj"	"$(INTDIR)\m_boss32.sbr" : $(SOURCE)\
 $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BOSS32=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_boss32.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_boss32.obj" : $(SOURCE) $(DEP_CPP_M_BOSS32) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_brain.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_brain.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_brain.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj"	"$(INTDIR)\m_brain.sbr" : $(SOURCE) $(DEP_CPP_M_BRA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_brain.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_BRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_brain.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_brain.obj" : $(SOURCE) $(DEP_CPP_M_BRA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_chick.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_chick.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_chick.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj"	"$(INTDIR)\m_chick.sbr" : $(SOURCE) $(DEP_CPP_M_CHI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_chick.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_CHI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_chick.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_chick.obj" : $(SOURCE) $(DEP_CPP_M_CHI) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flash.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLA=\
	".\q_shared.h"\
	

"$(INTDIR)\m_flash.obj" : $(SOURCE) $(DEP_CPP_M_FLA) "$(INTDIR)"


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
	".\ini.h"\
	".\m_flipper.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_flipper.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj"	"$(INTDIR)\m_flipper.sbr" : $(SOURCE)\
 $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_flipper.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLI=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_flipper.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flipper.obj" : $(SOURCE) $(DEP_CPP_M_FLI) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_float.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_float.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_float.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj"	"$(INTDIR)\m_float.sbr" : $(SOURCE) $(DEP_CPP_M_FLO)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_float.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_float.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_float.obj" : $(SOURCE) $(DEP_CPP_M_FLO) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_flyer.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_flyer.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_flyer.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj"	"$(INTDIR)\m_flyer.sbr" : $(SOURCE) $(DEP_CPP_M_FLY)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_flyer.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_FLY=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_flyer.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_flyer.obj" : $(SOURCE) $(DEP_CPP_M_FLY) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_gladiator.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gladiator.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gladiator.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj"	"$(INTDIR)\m_gladiator.sbr" : $(SOURCE)\
 $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gladiator.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_GLA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gladiator.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gladiator.obj" : $(SOURCE) $(DEP_CPP_M_GLA) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_gunner.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gunner.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gunner.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj"	"$(INTDIR)\m_gunner.sbr" : $(SOURCE) $(DEP_CPP_M_GUN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gunner.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_GUN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_gunner.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_gunner.obj" : $(SOURCE) $(DEP_CPP_M_GUN) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_hover.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_hover.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_hover.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj"	"$(INTDIR)\m_hover.sbr" : $(SOURCE) $(DEP_CPP_M_HOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_hover.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_HOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_hover.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_hover.obj" : $(SOURCE) $(DEP_CPP_M_HOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_infantry.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_infantry.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_infantry.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj"	"$(INTDIR)\m_infantry.sbr" : $(SOURCE)\
 $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_infantry.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_INF=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_infantry.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_infantry.obj" : $(SOURCE) $(DEP_CPP_M_INF) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_insane.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_insane.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_insane.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj"	"$(INTDIR)\m_insane.sbr" : $(SOURCE) $(DEP_CPP_M_INS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_insane.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_INS=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_insane.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_insane.obj" : $(SOURCE) $(DEP_CPP_M_INS) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_medic.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_medic.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_medic.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj"	"$(INTDIR)\m_medic.sbr" : $(SOURCE) $(DEP_CPP_M_MED)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_medic.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MED=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_medic.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_medic.obj" : $(SOURCE) $(DEP_CPP_M_MED) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_move.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj"	"$(INTDIR)\m_move.sbr" : $(SOURCE) $(DEP_CPP_M_MOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_mutant.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_mutant.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_mutant.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj"	"$(INTDIR)\m_mutant.sbr" : $(SOURCE) $(DEP_CPP_M_MUT)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_mutant.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MUT=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_mutant.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_mutant.obj" : $(SOURCE) $(DEP_CPP_M_MUT) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_parasite.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_parasite.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_parasite.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj"	"$(INTDIR)\m_parasite.sbr" : $(SOURCE)\
 $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_parasite.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_PAR=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_parasite.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_parasite.obj" : $(SOURCE) $(DEP_CPP_M_PAR) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_soldier.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_soldier.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_soldier.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj"	"$(INTDIR)\m_soldier.sbr" : $(SOURCE)\
 $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_soldier.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_SOL=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_soldier.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_soldier.obj" : $(SOURCE) $(DEP_CPP_M_SOL) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_supertank.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_supertank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_supertank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj"	"$(INTDIR)\m_supertank.sbr" : $(SOURCE)\
 $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_supertank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_SUP=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_supertank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_supertank.obj" : $(SOURCE) $(DEP_CPP_M_SUP) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_tank.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_tank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_tank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj"	"$(INTDIR)\m_tank.sbr" : $(SOURCE) $(DEP_CPP_M_TAN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_tank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_TAN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_tank.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_tank.obj" : $(SOURCE) $(DEP_CPP_M_TAN) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_backprop.c

!IF  "$(CFG)" == "game - Win32 Release"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_BA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_backprop.obj"	"$(INTDIR)\nb_backprop.sbr" : $(SOURCE)\
 $(DEP_CPP_NB_BA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_BA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_backprop.obj" : $(SOURCE) $(DEP_CPP_NB_BA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_BA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_backprop.obj" : $(SOURCE) $(DEP_CPP_NB_BA) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_backprop2.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_BAC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_backprop2.obj" : $(SOURCE) $(DEP_CPP_NB_BAC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_BAC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_backprop2.obj"	"$(INTDIR)\nb_backprop2.sbr" : $(SOURCE)\
 $(DEP_CPP_NB_BAC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_BAC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_backprop2.obj" : $(SOURCE) $(DEP_CPP_NB_BAC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_BAC=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_backprop2.obj" : $(SOURCE) $(DEP_CPP_NB_BAC) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_explore.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_EX=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_explore.obj" : $(SOURCE) $(DEP_CPP_NB_EX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_EX=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_explore.obj"	"$(INTDIR)\nb_explore.sbr" : $(SOURCE)\
 $(DEP_CPP_NB_EX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_EX=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_explore.obj" : $(SOURCE) $(DEP_CPP_NB_EX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_EX=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_explore.obj" : $(SOURCE) $(DEP_CPP_NB_EX) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_game.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_GA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_game.obj" : $(SOURCE) $(DEP_CPP_NB_GA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_GA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_game.obj"	"$(INTDIR)\nb_game.sbr" : $(SOURCE) $(DEP_CPP_NB_GA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_GA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_game.obj" : $(SOURCE) $(DEP_CPP_NB_GA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_GA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_game.obj" : $(SOURCE) $(DEP_CPP_NB_GA) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_GAonechild.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_GAO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_GAonechild.obj" : $(SOURCE) $(DEP_CPP_NB_GAO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_GAO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_GAonechild.obj"	"$(INTDIR)\nb_GAonechild.sbr" : $(SOURCE)\
 $(DEP_CPP_NB_GAO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_GAO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_GAonechild.obj" : $(SOURCE) $(DEP_CPP_NB_GAO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_GAO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_GAonechild.obj" : $(SOURCE) $(DEP_CPP_NB_GAO) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_geneticalgorithm.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_GE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_geneticalgorithm.obj" : $(SOURCE) $(DEP_CPP_NB_GE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_GE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_geneticalgorithm.obj"	"$(INTDIR)\nb_geneticalgorithm.sbr" : \
$(SOURCE) $(DEP_CPP_NB_GE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_GE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_geneticalgorithm.obj" : $(SOURCE) $(DEP_CPP_NB_GE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_GE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_geneticalgorithm.obj" : $(SOURCE) $(DEP_CPP_NB_GE) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_imitation.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_IM=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_imitation.obj" : $(SOURCE) $(DEP_CPP_NB_IM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_IM=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_imitation.obj"	"$(INTDIR)\nb_imitation.sbr" : $(SOURCE)\
 $(DEP_CPP_NB_IM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_IM=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_imitation.obj" : $(SOURCE) $(DEP_CPP_NB_IM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_IM=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_imitation.obj" : $(SOURCE) $(DEP_CPP_NB_IM) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_in.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_IN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_in.obj" : $(SOURCE) $(DEP_CPP_NB_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_IN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_in.obj"	"$(INTDIR)\nb_in.sbr" : $(SOURCE) $(DEP_CPP_NB_IN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_IN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_in.obj" : $(SOURCE) $(DEP_CPP_NB_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_IN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_in.obj" : $(SOURCE) $(DEP_CPP_NB_IN) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_model2.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_MO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_model2.obj" : $(SOURCE) $(DEP_CPP_NB_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_MO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_model2.obj"	"$(INTDIR)\nb_model2.sbr" : $(SOURCE)\
 $(DEP_CPP_NB_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_MO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_model2.obj" : $(SOURCE) $(DEP_CPP_NB_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_MO=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_model2.obj" : $(SOURCE) $(DEP_CPP_NB_MO) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_move.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_move.obj" : $(SOURCE) $(DEP_CPP_NB_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_move.obj"	"$(INTDIR)\nb_move.sbr" : $(SOURCE) $(DEP_CPP_NB_MOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_move.obj" : $(SOURCE) $(DEP_CPP_NB_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_move.obj" : $(SOURCE) $(DEP_CPP_NB_MOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_nnet.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_NN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_nnet.obj" : $(SOURCE) $(DEP_CPP_NB_NN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_NN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_nnet.obj"	"$(INTDIR)\nb_nnet.sbr" : $(SOURCE) $(DEP_CPP_NB_NN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_NN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_nnet.obj" : $(SOURCE) $(DEP_CPP_NB_NN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_NN=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_nnet.obj" : $(SOURCE) $(DEP_CPP_NB_NN) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_old.c

!IF  "$(CFG)" == "game - Win32 Release"

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_OL=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_old.obj"	"$(INTDIR)\nb_old.sbr" : $(SOURCE) $(DEP_CPP_NB_OL)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_OL=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_old.obj" : $(SOURCE) $(DEP_CPP_NB_OL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_OL=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_old.obj" : $(SOURCE) $(DEP_CPP_NB_OL) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_out.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_OU=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_out.obj" : $(SOURCE) $(DEP_CPP_NB_OU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_OU=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_out.obj"	"$(INTDIR)\nb_out.sbr" : $(SOURCE) $(DEP_CPP_NB_OU)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_OU=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_out.obj" : $(SOURCE) $(DEP_CPP_NB_OU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_OU=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_out.obj" : $(SOURCE) $(DEP_CPP_NB_OU) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_save.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_SA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_save.obj" : $(SOURCE) $(DEP_CPP_NB_SA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_SA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_save.obj"	"$(INTDIR)\nb_save.sbr" : $(SOURCE) $(DEP_CPP_NB_SA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_SA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_save.obj" : $(SOURCE) $(DEP_CPP_NB_SA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_SA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_save.obj" : $(SOURCE) $(DEP_CPP_NB_SA) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_spawn.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_SP=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_spawn.obj" : $(SOURCE) $(DEP_CPP_NB_SP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_SP=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_spawn.obj"	"$(INTDIR)\nb_spawn.sbr" : $(SOURCE) $(DEP_CPP_NB_SP)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_SP=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_spawn.obj" : $(SOURCE) $(DEP_CPP_NB_SP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_SP=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_spawn.obj" : $(SOURCE) $(DEP_CPP_NB_SP) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_structGA.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_ST=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_structGA.obj" : $(SOURCE) $(DEP_CPP_NB_ST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_ST=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_structGA.obj"	"$(INTDIR)\nb_structGA.sbr" : $(SOURCE)\
 $(DEP_CPP_NB_ST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_ST=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_structGA.obj" : $(SOURCE) $(DEP_CPP_NB_ST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_ST=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_structGA.obj" : $(SOURCE) $(DEP_CPP_NB_ST) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_test.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_TE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_test.obj" : $(SOURCE) $(DEP_CPP_NB_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_TE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_test.obj"	"$(INTDIR)\nb_test.sbr" : $(SOURCE) $(DEP_CPP_NB_TE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_TE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_test.obj" : $(SOURCE) $(DEP_CPP_NB_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_TE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_test.obj" : $(SOURCE) $(DEP_CPP_NB_TE) "$(INTDIR)"


!ENDIF 

SOURCE=.\nb_think.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_NB_TH=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_think.obj" : $(SOURCE) $(DEP_CPP_NB_TH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_NB_TH=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_think.obj"	"$(INTDIR)\nb_think.sbr" : $(SOURCE) $(DEP_CPP_NB_TH)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_NB_TH=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_think.obj" : $(SOURCE) $(DEP_CPP_NB_TH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_NB_TH=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\nb_think.obj" : $(SOURCE) $(DEP_CPP_NB_TH) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_client.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_CLI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_CLI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj"	"$(INTDIR)\p_client.sbr" : $(SOURCE) $(DEP_CPP_P_CLI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_CLI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_CLI=\
	".\camclient.h"\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_hud.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj"	"$(INTDIR)\p_hud.sbr" : $(SOURCE) $(DEP_CPP_P_HUD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_HUD=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_trail.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj"	"$(INTDIR)\p_trail.sbr" : $(SOURCE) $(DEP_CPP_P_TRA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_view.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj"	"$(INTDIR)\p_view.sbr" : $(SOURCE) $(DEP_CPP_P_VIE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_VIE=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj"	"$(INTDIR)\p_weapon.sbr" : $(SOURCE) $(DEP_CPP_P_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\ini.h"\
	".\m_player.h"\
	".\nb_nnet.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ENDIF 

SOURCE=.\q_shared.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


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

