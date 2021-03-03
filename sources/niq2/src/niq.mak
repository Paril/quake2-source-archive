# Microsoft Developer Studio Generated NMAKE File, Based on niq.dsp
!IF "$(CFG)" == ""
CFG=niq - Win32 Debug
!MESSAGE No configuration specified. Defaulting to niq - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "niq - Win32 Release" && "$(CFG)" != "niq - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "niq.mak" CFG="niq - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "niq - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "niq - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "niq - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gamex86.dll" "$(OUTDIR)\niq.bsc" "c:\quake2\niq\gamex86.dll"

!ELSE 

ALL : "$(OUTDIR)\gamex86.dll" "$(OUTDIR)\niq.bsc" "c:\quake2\niq\gamex86.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\bot_ai.obj"
	-@erase "$(INTDIR)\bot_ai.sbr"
	-@erase "$(INTDIR)\bot_die.obj"
	-@erase "$(INTDIR)\bot_die.sbr"
	-@erase "$(INTDIR)\bot_items.obj"
	-@erase "$(INTDIR)\bot_items.sbr"
	-@erase "$(INTDIR)\bot_misc.obj"
	-@erase "$(INTDIR)\bot_misc.sbr"
	-@erase "$(INTDIR)\bot_nav.obj"
	-@erase "$(INTDIR)\bot_nav.sbr"
	-@erase "$(INTDIR)\bot_spawn.obj"
	-@erase "$(INTDIR)\bot_spawn.sbr"
	-@erase "$(INTDIR)\bot_wpns.obj"
	-@erase "$(INTDIR)\bot_wpns.sbr"
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
	-@erase "$(INTDIR)\g_ctf.obj"
	-@erase "$(INTDIR)\g_ctf.sbr"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_func.sbr"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_items.sbr"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_main.sbr"
	-@erase "$(INTDIR)\g_map_mod.obj"
	-@erase "$(INTDIR)\g_map_mod.sbr"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_misc.sbr"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_monster.sbr"
	-@erase "$(INTDIR)\g_niq.obj"
	-@erase "$(INTDIR)\g_niq.sbr"
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
	-@erase "$(INTDIR)\g_unzip.obj"
	-@erase "$(INTDIR)\g_unzip.sbr"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_utils.sbr"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\g_weapon.sbr"
	-@erase "$(INTDIR)\g_zip.obj"
	-@erase "$(INTDIR)\g_zip.sbr"
	-@erase "$(INTDIR)\gslog.obj"
	-@erase "$(INTDIR)\gslog.sbr"
	-@erase "$(INTDIR)\hook.obj"
	-@erase "$(INTDIR)\hook.sbr"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_move.sbr"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_client.sbr"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_hud.sbr"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_menu.sbr"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_view.sbr"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\p_weapon.sbr"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\q_shared.sbr"
	-@erase "$(INTDIR)\stdlog.obj"
	-@erase "$(INTDIR)\stdlog.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(OUTDIR)\gamex86.dll"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "$(OUTDIR)\niq.bsc"
	-@erase "c:\quake2\niq\gamex86.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NIQ"\
 /D "NIQBOTS" /D "EBOTS" /D "OHOOK" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\niq.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\Release/

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
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\niq.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\bot_ai.sbr" \
	"$(INTDIR)\bot_die.sbr" \
	"$(INTDIR)\bot_items.sbr" \
	"$(INTDIR)\bot_misc.sbr" \
	"$(INTDIR)\bot_nav.sbr" \
	"$(INTDIR)\bot_spawn.sbr" \
	"$(INTDIR)\bot_wpns.sbr" \
	"$(INTDIR)\camclient.sbr" \
	"$(INTDIR)\g_ai.sbr" \
	"$(INTDIR)\g_chase.sbr" \
	"$(INTDIR)\g_cmds.sbr" \
	"$(INTDIR)\g_combat.sbr" \
	"$(INTDIR)\g_ctf.sbr" \
	"$(INTDIR)\g_func.sbr" \
	"$(INTDIR)\g_items.sbr" \
	"$(INTDIR)\g_main.sbr" \
	"$(INTDIR)\g_map_mod.sbr" \
	"$(INTDIR)\g_misc.sbr" \
	"$(INTDIR)\g_monster.sbr" \
	"$(INTDIR)\g_niq.sbr" \
	"$(INTDIR)\g_phys.sbr" \
	"$(INTDIR)\g_save.sbr" \
	"$(INTDIR)\g_spawn.sbr" \
	"$(INTDIR)\g_svcmds.sbr" \
	"$(INTDIR)\g_target.sbr" \
	"$(INTDIR)\g_trigger.sbr" \
	"$(INTDIR)\g_unzip.sbr" \
	"$(INTDIR)\g_utils.sbr" \
	"$(INTDIR)\g_weapon.sbr" \
	"$(INTDIR)\g_zip.sbr" \
	"$(INTDIR)\gslog.sbr" \
	"$(INTDIR)\hook.sbr" \
	"$(INTDIR)\m_move.sbr" \
	"$(INTDIR)\p_client.sbr" \
	"$(INTDIR)\p_hud.sbr" \
	"$(INTDIR)\p_menu.sbr" \
	"$(INTDIR)\p_view.sbr" \
	"$(INTDIR)\p_weapon.sbr" \
	"$(INTDIR)\q_shared.sbr" \
	"$(INTDIR)\stdlog.sbr"

"$(OUTDIR)\niq.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=../navlib/release/navlib.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb" /machine:I386 /nodefaultlib:"libc"\
 /def:".\game.def" /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\bot_ai.obj" \
	"$(INTDIR)\bot_die.obj" \
	"$(INTDIR)\bot_items.obj" \
	"$(INTDIR)\bot_misc.obj" \
	"$(INTDIR)\bot_nav.obj" \
	"$(INTDIR)\bot_spawn.obj" \
	"$(INTDIR)\bot_wpns.obj" \
	"$(INTDIR)\camclient.obj" \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_chase.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_ctf.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_map_mod.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_niq.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_svcmds.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_unzip.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\g_zip.obj" \
	"$(INTDIR)\gslog.obj" \
	"$(INTDIR)\hook.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj" \
	"$(INTDIR)\stdlog.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

InputPath=.\Release\gamex86.dll
SOURCE=$(InputPath)

"c:\quake2\niq\gamex86.dll"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xcopy release\gamex86.dll c:\quake2\niq

!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\gamex86.dll" "$(OUTDIR)\niq.bsc" "c:\quake2\niq\gamex86.dll"

!ELSE 

ALL : "$(OUTDIR)\gamex86.dll" "$(OUTDIR)\niq.bsc" "c:\quake2\niq\gamex86.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\bot_ai.obj"
	-@erase "$(INTDIR)\bot_ai.sbr"
	-@erase "$(INTDIR)\bot_die.obj"
	-@erase "$(INTDIR)\bot_die.sbr"
	-@erase "$(INTDIR)\bot_items.obj"
	-@erase "$(INTDIR)\bot_items.sbr"
	-@erase "$(INTDIR)\bot_misc.obj"
	-@erase "$(INTDIR)\bot_misc.sbr"
	-@erase "$(INTDIR)\bot_nav.obj"
	-@erase "$(INTDIR)\bot_nav.sbr"
	-@erase "$(INTDIR)\bot_spawn.obj"
	-@erase "$(INTDIR)\bot_spawn.sbr"
	-@erase "$(INTDIR)\bot_wpns.obj"
	-@erase "$(INTDIR)\bot_wpns.sbr"
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
	-@erase "$(INTDIR)\g_ctf.obj"
	-@erase "$(INTDIR)\g_ctf.sbr"
	-@erase "$(INTDIR)\g_func.obj"
	-@erase "$(INTDIR)\g_func.sbr"
	-@erase "$(INTDIR)\g_items.obj"
	-@erase "$(INTDIR)\g_items.sbr"
	-@erase "$(INTDIR)\g_main.obj"
	-@erase "$(INTDIR)\g_main.sbr"
	-@erase "$(INTDIR)\g_map_mod.obj"
	-@erase "$(INTDIR)\g_map_mod.sbr"
	-@erase "$(INTDIR)\g_misc.obj"
	-@erase "$(INTDIR)\g_misc.sbr"
	-@erase "$(INTDIR)\g_monster.obj"
	-@erase "$(INTDIR)\g_monster.sbr"
	-@erase "$(INTDIR)\g_niq.obj"
	-@erase "$(INTDIR)\g_niq.sbr"
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
	-@erase "$(INTDIR)\g_unzip.obj"
	-@erase "$(INTDIR)\g_unzip.sbr"
	-@erase "$(INTDIR)\g_utils.obj"
	-@erase "$(INTDIR)\g_utils.sbr"
	-@erase "$(INTDIR)\g_weapon.obj"
	-@erase "$(INTDIR)\g_weapon.sbr"
	-@erase "$(INTDIR)\g_zip.obj"
	-@erase "$(INTDIR)\g_zip.sbr"
	-@erase "$(INTDIR)\gslog.obj"
	-@erase "$(INTDIR)\gslog.sbr"
	-@erase "$(INTDIR)\hook.obj"
	-@erase "$(INTDIR)\hook.sbr"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\m_move.sbr"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_client.sbr"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_hud.sbr"
	-@erase "$(INTDIR)\p_menu.obj"
	-@erase "$(INTDIR)\p_menu.sbr"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_view.sbr"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\p_weapon.sbr"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\q_shared.sbr"
	-@erase "$(INTDIR)\stdlog.obj"
	-@erase "$(INTDIR)\stdlog.sbr"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(OUTDIR)\gamex86.dll"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "$(OUTDIR)\gamex86.pdb"
	-@erase "$(OUTDIR)\niq.bsc"
	-@erase "c:\quake2\niq\gamex86.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "NIQ" /D "NIQBOTS" /D "EBOTS" /D "OHOOK" /Fr"$(INTDIR)\\"\
 /Fp"$(INTDIR)\niq.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/

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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o NUL /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\niq.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\bot_ai.sbr" \
	"$(INTDIR)\bot_die.sbr" \
	"$(INTDIR)\bot_items.sbr" \
	"$(INTDIR)\bot_misc.sbr" \
	"$(INTDIR)\bot_nav.sbr" \
	"$(INTDIR)\bot_spawn.sbr" \
	"$(INTDIR)\bot_wpns.sbr" \
	"$(INTDIR)\camclient.sbr" \
	"$(INTDIR)\g_ai.sbr" \
	"$(INTDIR)\g_chase.sbr" \
	"$(INTDIR)\g_cmds.sbr" \
	"$(INTDIR)\g_combat.sbr" \
	"$(INTDIR)\g_ctf.sbr" \
	"$(INTDIR)\g_func.sbr" \
	"$(INTDIR)\g_items.sbr" \
	"$(INTDIR)\g_main.sbr" \
	"$(INTDIR)\g_map_mod.sbr" \
	"$(INTDIR)\g_misc.sbr" \
	"$(INTDIR)\g_monster.sbr" \
	"$(INTDIR)\g_niq.sbr" \
	"$(INTDIR)\g_phys.sbr" \
	"$(INTDIR)\g_save.sbr" \
	"$(INTDIR)\g_spawn.sbr" \
	"$(INTDIR)\g_svcmds.sbr" \
	"$(INTDIR)\g_target.sbr" \
	"$(INTDIR)\g_trigger.sbr" \
	"$(INTDIR)\g_unzip.sbr" \
	"$(INTDIR)\g_utils.sbr" \
	"$(INTDIR)\g_weapon.sbr" \
	"$(INTDIR)\g_zip.sbr" \
	"$(INTDIR)\gslog.sbr" \
	"$(INTDIR)\hook.sbr" \
	"$(INTDIR)\m_move.sbr" \
	"$(INTDIR)\p_client.sbr" \
	"$(INTDIR)\p_hud.sbr" \
	"$(INTDIR)\p_menu.sbr" \
	"$(INTDIR)\p_view.sbr" \
	"$(INTDIR)\p_weapon.sbr" \
	"$(INTDIR)\q_shared.sbr" \
	"$(INTDIR)\stdlog.sbr"

"$(OUTDIR)\niq.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=../navlib/debug/navlib.lib /nologo /subsystem:windows /dll\
 /incremental:no /pdb:"$(OUTDIR)\gamex86.pdb" /debug /debugtype:coff\
 /machine:I386 /nodefaultlib:"libcd" /def:".\game.def"\
 /out:"$(OUTDIR)\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" /pdbtype:sept 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\bot_ai.obj" \
	"$(INTDIR)\bot_die.obj" \
	"$(INTDIR)\bot_items.obj" \
	"$(INTDIR)\bot_misc.obj" \
	"$(INTDIR)\bot_nav.obj" \
	"$(INTDIR)\bot_spawn.obj" \
	"$(INTDIR)\bot_wpns.obj" \
	"$(INTDIR)\camclient.obj" \
	"$(INTDIR)\g_ai.obj" \
	"$(INTDIR)\g_chase.obj" \
	"$(INTDIR)\g_cmds.obj" \
	"$(INTDIR)\g_combat.obj" \
	"$(INTDIR)\g_ctf.obj" \
	"$(INTDIR)\g_func.obj" \
	"$(INTDIR)\g_items.obj" \
	"$(INTDIR)\g_main.obj" \
	"$(INTDIR)\g_map_mod.obj" \
	"$(INTDIR)\g_misc.obj" \
	"$(INTDIR)\g_monster.obj" \
	"$(INTDIR)\g_niq.obj" \
	"$(INTDIR)\g_phys.obj" \
	"$(INTDIR)\g_save.obj" \
	"$(INTDIR)\g_spawn.obj" \
	"$(INTDIR)\g_svcmds.obj" \
	"$(INTDIR)\g_target.obj" \
	"$(INTDIR)\g_trigger.obj" \
	"$(INTDIR)\g_unzip.obj" \
	"$(INTDIR)\g_utils.obj" \
	"$(INTDIR)\g_weapon.obj" \
	"$(INTDIR)\g_zip.obj" \
	"$(INTDIR)\gslog.obj" \
	"$(INTDIR)\hook.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_menu.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj" \
	"$(INTDIR)\stdlog.obj"

"$(OUTDIR)\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

InputPath=.\Debug\gamex86.dll
SOURCE=$(InputPath)

"c:\quake2\niq\gamex86.dll"	 : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xcopy debug\gamex86.dll c:\quake2\niq

!ENDIF 


!IF "$(CFG)" == "niq - Win32 Release" || "$(CFG)" == "niq - Win32 Debug"
SOURCE=.\bot_ai.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_BOT_A=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_ai.obj"	"$(INTDIR)\bot_ai.sbr" : $(SOURCE) $(DEP_CPP_BOT_A)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_BOT_A=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_ai.obj"	"$(INTDIR)\bot_ai.sbr" : $(SOURCE) $(DEP_CPP_BOT_A)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\bot_die.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_BOT_D=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_die.obj"	"$(INTDIR)\bot_die.sbr" : $(SOURCE) $(DEP_CPP_BOT_D)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_BOT_D=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_die.obj"	"$(INTDIR)\bot_die.sbr" : $(SOURCE) $(DEP_CPP_BOT_D)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\bot_items.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_BOT_I=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_items.obj"	"$(INTDIR)\bot_items.sbr" : $(SOURCE)\
 $(DEP_CPP_BOT_I) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_BOT_I=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_items.obj"	"$(INTDIR)\bot_items.sbr" : $(SOURCE)\
 $(DEP_CPP_BOT_I) "$(INTDIR)"


!ENDIF 

SOURCE=.\bot_misc.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_BOT_M=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_misc.obj"	"$(INTDIR)\bot_misc.sbr" : $(SOURCE) $(DEP_CPP_BOT_M)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_BOT_M=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_misc.obj"	"$(INTDIR)\bot_misc.sbr" : $(SOURCE) $(DEP_CPP_BOT_M)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\bot_nav.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_BOT_N=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_nav.obj"	"$(INTDIR)\bot_nav.sbr" : $(SOURCE) $(DEP_CPP_BOT_N)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_BOT_N=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_nav.obj"	"$(INTDIR)\bot_nav.sbr" : $(SOURCE) $(DEP_CPP_BOT_N)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\bot_spawn.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_BOT_S=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_spawn.obj"	"$(INTDIR)\bot_spawn.sbr" : $(SOURCE)\
 $(DEP_CPP_BOT_S) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_BOT_S=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_spawn.obj"	"$(INTDIR)\bot_spawn.sbr" : $(SOURCE)\
 $(DEP_CPP_BOT_S) "$(INTDIR)"


!ENDIF 

SOURCE=.\bot_wpns.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_BOT_W=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_wpns.obj"	"$(INTDIR)\bot_wpns.sbr" : $(SOURCE) $(DEP_CPP_BOT_W)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_BOT_W=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\bot_wpns.obj"	"$(INTDIR)\bot_wpns.sbr" : $(SOURCE) $(DEP_CPP_BOT_W)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\camclient.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_CAMCL=\
	".\camclient.h"\
	".\extra.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\camclient.obj"	"$(INTDIR)\camclient.sbr" : $(SOURCE)\
 $(DEP_CPP_CAMCL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_CAMCL=\
	".\camclient.h"\
	".\extra.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\camclient.obj"	"$(INTDIR)\camclient.sbr" : $(SOURCE)\
 $(DEP_CPP_CAMCL) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_ai.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_AI_=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj"	"$(INTDIR)\g_ai.sbr" : $(SOURCE) $(DEP_CPP_G_AI_)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_AI_=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj"	"$(INTDIR)\g_ai.sbr" : $(SOURCE) $(DEP_CPP_G_AI_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_chase.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_CHA=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj"	"$(INTDIR)\g_chase.sbr" : $(SOURCE) $(DEP_CPP_G_CHA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_CHA=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_chase.obj"	"$(INTDIR)\g_chase.sbr" : $(SOURCE) $(DEP_CPP_G_CHA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_cmds.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_CMD=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj"	"$(INTDIR)\g_cmds.sbr" : $(SOURCE) $(DEP_CPP_G_CMD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_CMD=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_cmds.obj"	"$(INTDIR)\g_cmds.sbr" : $(SOURCE) $(DEP_CPP_G_CMD)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_combat.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_COM=\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj"	"$(INTDIR)\g_combat.sbr" : $(SOURCE) $(DEP_CPP_G_COM)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_COM=\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_combat.obj"	"$(INTDIR)\g_combat.sbr" : $(SOURCE) $(DEP_CPP_G_COM)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_ctf.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_CTF=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_ctf.obj"	"$(INTDIR)\g_ctf.sbr" : $(SOURCE) $(DEP_CPP_G_CTF)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_CTF=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_ctf.obj"	"$(INTDIR)\g_ctf.sbr" : $(SOURCE) $(DEP_CPP_G_CTF)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_func.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_FUN=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj"	"$(INTDIR)\g_func.sbr" : $(SOURCE) $(DEP_CPP_G_FUN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_FUN=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj"	"$(INTDIR)\g_func.sbr" : $(SOURCE) $(DEP_CPP_G_FUN)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_items.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_ITE=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\hook.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj"	"$(INTDIR)\g_items.sbr" : $(SOURCE) $(DEP_CPP_G_ITE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_ITE=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_items.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\hook.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_items.obj"	"$(INTDIR)\g_items.sbr" : $(SOURCE) $(DEP_CPP_G_ITE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_main.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_MAI=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_main.obj"	"$(INTDIR)\g_main.sbr" : $(SOURCE) $(DEP_CPP_G_MAI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_MAI=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_main.obj"	"$(INTDIR)\g_main.sbr" : $(SOURCE) $(DEP_CPP_G_MAI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_map_mod.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_MAP=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_map_mod.obj"	"$(INTDIR)\g_map_mod.sbr" : $(SOURCE)\
 $(DEP_CPP_G_MAP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_MAP=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_map_mod.obj"	"$(INTDIR)\g_map_mod.sbr" : $(SOURCE)\
 $(DEP_CPP_G_MAP) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_misc.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_MIS=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj"	"$(INTDIR)\g_misc.sbr" : $(SOURCE) $(DEP_CPP_G_MIS)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_MIS=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_misc.obj"	"$(INTDIR)\g_misc.sbr" : $(SOURCE) $(DEP_CPP_G_MIS)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_monster.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_MON=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj"	"$(INTDIR)\g_monster.sbr" : $(SOURCE)\
 $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_MON=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj"	"$(INTDIR)\g_monster.sbr" : $(SOURCE)\
 $(DEP_CPP_G_MON) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_niq.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_NIQ=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_niq.obj"	"$(INTDIR)\g_niq.sbr" : $(SOURCE) $(DEP_CPP_G_NIQ)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_NIQ=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_niq.obj"	"$(INTDIR)\g_niq.sbr" : $(SOURCE) $(DEP_CPP_G_NIQ)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_phys.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_PHY=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj"	"$(INTDIR)\g_phys.sbr" : $(SOURCE) $(DEP_CPP_G_PHY)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_PHY=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj"	"$(INTDIR)\g_phys.sbr" : $(SOURCE) $(DEP_CPP_G_PHY)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_save.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_SAV=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_save.obj"	"$(INTDIR)\g_save.sbr" : $(SOURCE) $(DEP_CPP_G_SAV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_SAV=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_map_mod.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_save.obj"	"$(INTDIR)\g_save.sbr" : $(SOURCE) $(DEP_CPP_G_SAV)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_spawn.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_SPA=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_spawn.obj"	"$(INTDIR)\g_spawn.sbr" : $(SOURCE) $(DEP_CPP_G_SPA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_SPA=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\g_spawn.obj"	"$(INTDIR)\g_spawn.sbr" : $(SOURCE) $(DEP_CPP_G_SPA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_svcmds.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_SVC=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj"	"$(INTDIR)\g_svcmds.sbr" : $(SOURCE) $(DEP_CPP_G_SVC)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_SVC=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_svcmds.obj"	"$(INTDIR)\g_svcmds.sbr" : $(SOURCE) $(DEP_CPP_G_SVC)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_target.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_TAR=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj"	"$(INTDIR)\g_target.sbr" : $(SOURCE) $(DEP_CPP_G_TAR)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_TAR=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj"	"$(INTDIR)\g_target.sbr" : $(SOURCE) $(DEP_CPP_G_TAR)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_trigger.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_TRI=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj"	"$(INTDIR)\g_trigger.sbr" : $(SOURCE)\
 $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_TRI=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj"	"$(INTDIR)\g_trigger.sbr" : $(SOURCE)\
 $(DEP_CPP_G_TRI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_unzip.c
DEP_CPP_G_UNZ=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_unzip.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_unzip.obj"	"$(INTDIR)\g_unzip.sbr" : $(SOURCE) $(DEP_CPP_G_UNZ)\
 "$(INTDIR)"


SOURCE=.\g_utils.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_UTI=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj"	"$(INTDIR)\g_utils.sbr" : $(SOURCE) $(DEP_CPP_G_UTI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_UTI=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj"	"$(INTDIR)\g_utils.sbr" : $(SOURCE) $(DEP_CPP_G_UTI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_weapon.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_WEA=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj"	"$(INTDIR)\g_weapon.sbr" : $(SOURCE) $(DEP_CPP_G_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_WEA=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_weapon.obj"	"$(INTDIR)\g_weapon.sbr" : $(SOURCE) $(DEP_CPP_G_WEA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\g_zip.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_G_ZIP=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_zip.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\z_api.h"\
	".\z_unzip.h"\
	".\z_zip.h"\
	

"$(INTDIR)\g_zip.obj"	"$(INTDIR)\g_zip.sbr" : $(SOURCE) $(DEP_CPP_G_ZIP)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_G_ZIP=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_zip.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\z_api.h"\
	".\z_unzip.h"\
	".\z_zip.h"\
	

"$(INTDIR)\g_zip.obj"	"$(INTDIR)\g_zip.sbr" : $(SOURCE) $(DEP_CPP_G_ZIP)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\gslog.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_GSLOG=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\gslog.obj"	"$(INTDIR)\gslog.sbr" : $(SOURCE) $(DEP_CPP_GSLOG)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_GSLOG=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\gslog.obj"	"$(INTDIR)\gslog.sbr" : $(SOURCE) $(DEP_CPP_GSLOG)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\hook.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_HOOK_=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\hook.obj"	"$(INTDIR)\hook.sbr" : $(SOURCE) $(DEP_CPP_HOOK_)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_HOOK_=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\hook.obj"	"$(INTDIR)\hook.sbr" : $(SOURCE) $(DEP_CPP_HOOK_)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\m_move.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_M_MOV=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj"	"$(INTDIR)\m_move.sbr" : $(SOURCE) $(DEP_CPP_M_MOV)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_M_MOV=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj"	"$(INTDIR)\m_move.sbr" : $(SOURCE) $(DEP_CPP_M_MOV)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\p_client.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_P_CLI=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\hook.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\p_client.obj"	"$(INTDIR)\p_client.sbr" : $(SOURCE) $(DEP_CPP_P_CLI)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_P_CLI=\
	".\bot_procs.h"\
	".\camclient.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\gslog.h"\
	".\hook.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\p_trail.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\p_client.obj"	"$(INTDIR)\p_client.sbr" : $(SOURCE) $(DEP_CPP_P_CLI)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\p_hud.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_P_HUD=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj"	"$(INTDIR)\p_hud.sbr" : $(SOURCE) $(DEP_CPP_P_HUD)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_P_HUD=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_hud.obj"	"$(INTDIR)\p_hud.sbr" : $(SOURCE) $(DEP_CPP_P_HUD)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\p_menu.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_P_MEN=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_menu.obj"	"$(INTDIR)\p_menu.sbr" : $(SOURCE) $(DEP_CPP_P_MEN)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_P_MEN=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_menu.obj"	"$(INTDIR)\p_menu.sbr" : $(SOURCE) $(DEP_CPP_P_MEN)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\p_view.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_P_VIE=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj"	"$(INTDIR)\p_view.sbr" : $(SOURCE) $(DEP_CPP_P_VIE)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_P_VIE=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_view.obj"	"$(INTDIR)\p_view.sbr" : $(SOURCE) $(DEP_CPP_P_VIE)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\p_weapon.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_P_WEA=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj"	"$(INTDIR)\p_weapon.sbr" : $(SOURCE) $(DEP_CPP_P_WEA)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_P_WEA=\
	".\bot_procs.h"\
	".\g_ctf.h"\
	".\g_local.h"\
	".\g_niq.h"\
	".\game.h"\
	".\m_player.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_weapon.obj"	"$(INTDIR)\p_weapon.sbr" : $(SOURCE) $(DEP_CPP_P_WEA)\
 "$(INTDIR)"


!ENDIF 

SOURCE=.\q_shared.c
DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj"	"$(INTDIR)\q_shared.sbr" : $(SOURCE) $(DEP_CPP_Q_SHA)\
 "$(INTDIR)"


SOURCE=.\stdlog.c

!IF  "$(CFG)" == "niq - Win32 Release"

DEP_CPP_STDLO=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\stdlog.obj"	"$(INTDIR)\stdlog.sbr" : $(SOURCE) $(DEP_CPP_STDLO)\
 "$(INTDIR)"


!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

DEP_CPP_STDLO=\
	".\g_ctf.h"\
	".\g_local.h"\
	".\game.h"\
	".\p_menu.h"\
	".\q_shared.h"\
	".\stdlog.h"\
	

"$(INTDIR)\stdlog.obj"	"$(INTDIR)\stdlog.sbr" : $(SOURCE) $(DEP_CPP_STDLO)\
 "$(INTDIR)"


!ENDIF 


!ENDIF 

