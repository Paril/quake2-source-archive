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

OUTDIR=.\..\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : "..\gamex86.dll"

!ELSE 

ALL : "..\gamex86.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\ctf.obj"
	-@erase "$(INTDIR)\cycle.obj"
	-@erase "$(INTDIR)\ents.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\flag.obj"
	-@erase "$(INTDIR)\g_ai.obj"
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
	-@erase "$(INTDIR)\hook.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\ipban.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\motd.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\runes.obj"
	-@erase "$(INTDIR)\teamplay.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vwep.obj"
	-@erase "$(INTDIR)\weaponf.obj"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "..\gamex86.dll"
	-@erase ".\Release\gamex86.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /ML /W3 /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winmm.lib /nologo\
 /base:"0x20000000" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)\gamex86.pdb" /map:"$(INTDIR)\gamex86.map" /machine:I386\
 /def:".\game.def" /out:"c:\quake2\vanctf\gamex86.dll"\
 /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\ctf.obj" \
	"$(INTDIR)\cycle.obj" \
	"$(INTDIR)\ents.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\flag.obj" \
	"$(INTDIR)\g_ai.obj" \
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
	"$(INTDIR)\hook.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\ipban.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\motd.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj" \
	"$(INTDIR)\runes.obj" \
	"$(INTDIR)\teamplay.obj" \
	"$(INTDIR)\vwep.obj" \
	"$(INTDIR)\weaponf.obj"

"..\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "game - Win32 Debug"

OUTDIR=.\..\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : "..\gamex86.dll"

!ELSE 

ALL : "..\gamex86.dll"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\ctf.obj"
	-@erase "$(INTDIR)\cycle.obj"
	-@erase "$(INTDIR)\ents.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\flag.obj"
	-@erase "$(INTDIR)\g_ai.obj"
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
	-@erase "$(INTDIR)\hook.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\ipban.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\motd.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\runes.obj"
	-@erase "$(INTDIR)\teamplay.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\vwep.obj"
	-@erase "$(INTDIR)\weaponf.obj"
	-@erase "$(OUTDIR)\gamex86.exp"
	-@erase "$(OUTDIR)\gamex86.lib"
	-@erase "$(OUTDIR)\gamex86.pdb"
	-@erase "..\gamex86.dll"
	-@erase ".\Debug\gamex86.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /G5 /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D\
 "_WINDOWS" /D "BUILDING_REF_GL" /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\"\
 /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Debug/
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
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\game.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winmm.lib /nologo\
 /base:"0x20000000" /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)\gamex86.pdb" /map:"$(INTDIR)\gamex86.map" /debug\
 /debugtype:both /machine:I386 /def:".\game.def"\
 /out:"c:\quake2\vanctf\gamex86.dll" /implib:"$(OUTDIR)\gamex86.lib" 
DEF_FILE= \
	".\game.def"
LINK32_OBJS= \
	"$(INTDIR)\ctf.obj" \
	"$(INTDIR)\cycle.obj" \
	"$(INTDIR)\ents.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\flag.obj" \
	"$(INTDIR)\g_ai.obj" \
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
	"$(INTDIR)\hook.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\ipban.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\motd.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj" \
	"$(INTDIR)\runes.obj" \
	"$(INTDIR)\teamplay.obj" \
	"$(INTDIR)\vwep.obj" \
	"$(INTDIR)\weaponf.obj"

"..\gamex86.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
	-@erase "$(INTDIR)\ctf.obj"
	-@erase "$(INTDIR)\cycle.obj"
	-@erase "$(INTDIR)\ents.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\flag.obj"
	-@erase "$(INTDIR)\g_ai.obj"
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
	-@erase "$(INTDIR)\hook.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\ipban.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\motd.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\runes.obj"
	-@erase "$(INTDIR)\teamplay.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\vwep.obj"
	-@erase "$(INTDIR)\weaponf.obj"
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
CPP_PROJ=/nologo /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /QA21164 /MTd\
 /c 
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
	"$(INTDIR)\ctf.obj" \
	"$(INTDIR)\cycle.obj" \
	"$(INTDIR)\ents.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\flag.obj" \
	"$(INTDIR)\g_ai.obj" \
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
	"$(INTDIR)\hook.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\ipban.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\motd.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj" \
	"$(INTDIR)\runes.obj" \
	"$(INTDIR)\teamplay.obj" \
	"$(INTDIR)\vwep.obj" \
	"$(INTDIR)\weaponf.obj"

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
	-@erase "$(INTDIR)\ctf.obj"
	-@erase "$(INTDIR)\cycle.obj"
	-@erase "$(INTDIR)\ents.obj"
	-@erase "$(INTDIR)\fileio.obj"
	-@erase "$(INTDIR)\flag.obj"
	-@erase "$(INTDIR)\g_ai.obj"
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
	-@erase "$(INTDIR)\hook.obj"
	-@erase "$(INTDIR)\id.obj"
	-@erase "$(INTDIR)\ipban.obj"
	-@erase "$(INTDIR)\m_move.obj"
	-@erase "$(INTDIR)\motd.obj"
	-@erase "$(INTDIR)\p_client.obj"
	-@erase "$(INTDIR)\p_hud.obj"
	-@erase "$(INTDIR)\p_trail.obj"
	-@erase "$(INTDIR)\p_view.obj"
	-@erase "$(INTDIR)\p_weapon.obj"
	-@erase "$(INTDIR)\q_shared.obj"
	-@erase "$(INTDIR)\runes.obj"
	-@erase "$(INTDIR)\teamplay.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vwep.obj"
	-@erase "$(INTDIR)\weaponf.obj"
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
CPP_PROJ=/nologo /Gt0 /W3 /GX /Zd /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)\game.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /QA21164 /c 
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
	"$(INTDIR)\ctf.obj" \
	"$(INTDIR)\cycle.obj" \
	"$(INTDIR)\ents.obj" \
	"$(INTDIR)\fileio.obj" \
	"$(INTDIR)\flag.obj" \
	"$(INTDIR)\g_ai.obj" \
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
	"$(INTDIR)\hook.obj" \
	"$(INTDIR)\id.obj" \
	"$(INTDIR)\ipban.obj" \
	"$(INTDIR)\m_move.obj" \
	"$(INTDIR)\motd.obj" \
	"$(INTDIR)\p_client.obj" \
	"$(INTDIR)\p_hud.obj" \
	"$(INTDIR)\p_trail.obj" \
	"$(INTDIR)\p_view.obj" \
	"$(INTDIR)\p_weapon.obj" \
	"$(INTDIR)\q_shared.obj" \
	"$(INTDIR)\runes.obj" \
	"$(INTDIR)\teamplay.obj" \
	"$(INTDIR)\vwep.obj" \
	"$(INTDIR)\weaponf.obj"

"$(OUTDIR)\gameaxp.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "game - Win32 Release" || "$(CFG)" == "game - Win32 Debug" ||\
 "$(CFG)" == "game - Win32 Debug Alpha" || "$(CFG)" ==\
 "game - Win32 Release Alpha"
SOURCE=.\ctf.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_CTF_C=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\ctf.obj" : $(SOURCE) $(DEP_CPP_CTF_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_CTF_C=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\ctf.obj" : $(SOURCE) $(DEP_CPP_CTF_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_CTF_C=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\ctf.obj" : $(SOURCE) $(DEP_CPP_CTF_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_CTF_C=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\ctf.obj" : $(SOURCE) $(DEP_CPP_CTF_C) "$(INTDIR)"


!ENDIF 

SOURCE=.\cycle.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_CYCLE=\
	".\cycle.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\cycle.obj" : $(SOURCE) $(DEP_CPP_CYCLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_CYCLE=\
	".\cycle.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\cycle.obj" : $(SOURCE) $(DEP_CPP_CYCLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_CYCLE=\
	".\cycle.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\cycle.obj" : $(SOURCE) $(DEP_CPP_CYCLE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_CYCLE=\
	".\cycle.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\cycle.obj" : $(SOURCE) $(DEP_CPP_CYCLE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ents.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_ENTS_=\
	".\ents.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ents.obj" : $(SOURCE) $(DEP_CPP_ENTS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_ENTS_=\
	".\ents.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ents.obj" : $(SOURCE) $(DEP_CPP_ENTS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_ENTS_=\
	".\ents.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ents.obj" : $(SOURCE) $(DEP_CPP_ENTS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_ENTS_=\
	".\ents.h"\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ents.obj" : $(SOURCE) $(DEP_CPP_ENTS_) "$(INTDIR)"


!ENDIF 

SOURCE=.\fileio.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_FILEI=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\fileio.obj" : $(SOURCE) $(DEP_CPP_FILEI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_FILEI=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\fileio.obj" : $(SOURCE) $(DEP_CPP_FILEI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_FILEI=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\fileio.obj" : $(SOURCE) $(DEP_CPP_FILEI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_FILEI=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\fileio.obj" : $(SOURCE) $(DEP_CPP_FILEI) "$(INTDIR)"


!ENDIF 

SOURCE=.\flag.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_FLAG_=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\flag.obj" : $(SOURCE) $(DEP_CPP_FLAG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_FLAG_=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\flag.obj" : $(SOURCE) $(DEP_CPP_FLAG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_FLAG_=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\flag.obj" : $(SOURCE) $(DEP_CPP_FLAG_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_FLAG_=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\flag.obj" : $(SOURCE) $(DEP_CPP_FLAG_) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_ai.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_AI_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_ai.obj" : $(SOURCE) $(DEP_CPP_G_AI_) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_cmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_CMD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\id.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_CMD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\id.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_CMD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\id.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_CMD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\id.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_cmds.obj" : $(SOURCE) $(DEP_CPP_G_CMD) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_combat.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_COM=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_combat.obj" : $(SOURCE) $(DEP_CPP_G_COM) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_func.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_FUN=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_func.obj" : $(SOURCE) $(DEP_CPP_G_FUN) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_items.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_ITE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_ITE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_ITE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_ITE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\g_items.obj" : $(SOURCE) $(DEP_CPP_G_ITE) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_main.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MAI=\
	".\ctf.h"\
	".\cycle.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MAI=\
	".\ctf.h"\
	".\cycle.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MAI=\
	".\ctf.h"\
	".\cycle.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MAI=\
	".\ctf.h"\
	".\cycle.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_main.obj" : $(SOURCE) $(DEP_CPP_G_MAI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_misc.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MIS=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MIS=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MIS=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MIS=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_misc.obj" : $(SOURCE) $(DEP_CPP_G_MIS) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_monster.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_MON=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_monster.obj" : $(SOURCE) $(DEP_CPP_G_MON) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_phys.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_PHY=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_phys.obj" : $(SOURCE) $(DEP_CPP_G_PHY) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_save.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SAV=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SAV=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SAV=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SAV=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_save.obj" : $(SOURCE) $(DEP_CPP_G_SAV) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_spawn.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SPA=\
	".\ctf.h"\
	".\ents.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\weaponf.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SPA=\
	".\ctf.h"\
	".\ents.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\weaponf.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SPA=\
	".\ctf.h"\
	".\ents.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\weaponf.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SPA=\
	".\ctf.h"\
	".\ents.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\weaponf.h"\
	

"$(INTDIR)\g_spawn.obj" : $(SOURCE) $(DEP_CPP_G_SPA) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_svcmds.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_SVC=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_svcmds.obj" : $(SOURCE) $(DEP_CPP_G_SVC) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_target.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TAR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_target.obj" : $(SOURCE) $(DEP_CPP_G_TAR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_trigger.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TRI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_trigger.obj" : $(SOURCE) $(DEP_CPP_G_TRI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_turret.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_TUR=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_turret.obj" : $(SOURCE) $(DEP_CPP_G_TUR) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_utils.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_UTI=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\g_utils.obj" : $(SOURCE) $(DEP_CPP_G_UTI) "$(INTDIR)"


!ENDIF 

SOURCE=.\g_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_G_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\g_weapon.obj" : $(SOURCE) $(DEP_CPP_G_WEA) "$(INTDIR)"


!ENDIF 

SOURCE=.\hook.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_HOOK_=\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\hook.obj" : $(SOURCE) $(DEP_CPP_HOOK_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_HOOK_=\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\hook.obj" : $(SOURCE) $(DEP_CPP_HOOK_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_HOOK_=\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\hook.obj" : $(SOURCE) $(DEP_CPP_HOOK_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_HOOK_=\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\hook.obj" : $(SOURCE) $(DEP_CPP_HOOK_) "$(INTDIR)"


!ENDIF 

SOURCE=.\id.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_ID_C2e=\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_C2e) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_ID_C2e=\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_C2e) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_ID_C2e=\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_C2e) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_ID_C2e=\
	".\g_local.h"\
	".\game.h"\
	".\id.h"\
	".\q_shared.h"\
	

"$(INTDIR)\id.obj" : $(SOURCE) $(DEP_CPP_ID_C2e) "$(INTDIR)"


!ENDIF 

SOURCE=.\ipban.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_IPBAN=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\ipban.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ipban.obj" : $(SOURCE) $(DEP_CPP_IPBAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_IPBAN=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\ipban.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ipban.obj" : $(SOURCE) $(DEP_CPP_IPBAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_IPBAN=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\ipban.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ipban.obj" : $(SOURCE) $(DEP_CPP_IPBAN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_IPBAN=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\ipban.h"\
	".\q_shared.h"\
	

"$(INTDIR)\ipban.obj" : $(SOURCE) $(DEP_CPP_IPBAN) "$(INTDIR)"


!ENDIF 

SOURCE=.\m_move.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_M_MOV=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\m_move.obj" : $(SOURCE) $(DEP_CPP_M_MOV) "$(INTDIR)"


!ENDIF 

SOURCE=.\motd.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_MOTD_=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\motd.obj" : $(SOURCE) $(DEP_CPP_MOTD_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_MOTD_=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\motd.obj" : $(SOURCE) $(DEP_CPP_MOTD_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_MOTD_=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\motd.obj" : $(SOURCE) $(DEP_CPP_MOTD_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_MOTD_=\
	".\fileio.h"\
	".\g_local.h"\
	".\game.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\motd.obj" : $(SOURCE) $(DEP_CPP_MOTD_) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_client.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_CLI=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\ipban.h"\
	".\m_player.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\vwep.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_CLI=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\ipban.h"\
	".\m_player.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\vwep.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_CLI=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\ipban.h"\
	".\m_player.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\vwep.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_CLI=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\hook.h"\
	".\ipban.h"\
	".\m_player.h"\
	".\motd.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\teamplay.h"\
	".\vwep.h"\
	

"$(INTDIR)\p_client.obj" : $(SOURCE) $(DEP_CPP_P_CLI) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_hud.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_HUD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_HUD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_HUD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_HUD=\
	".\ctf.h"\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\p_hud.obj" : $(SOURCE) $(DEP_CPP_P_HUD) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_trail.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_TRA=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	

"$(INTDIR)\p_trail.obj" : $(SOURCE) $(DEP_CPP_P_TRA) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_view.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_VIE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_VIE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_VIE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_VIE=\
	".\flag.h"\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\p_view.obj" : $(SOURCE) $(DEP_CPP_P_VIE) "$(INTDIR)"


!ENDIF 

SOURCE=.\p_weapon.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\vwep.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\vwep.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\vwep.h"\
	

"$(INTDIR)\p_weapon.obj" : $(SOURCE) $(DEP_CPP_P_WEA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_P_WEA=\
	".\g_local.h"\
	".\game.h"\
	".\m_player.h"\
	".\q_shared.h"\
	".\runes.h"\
	".\vwep.h"\
	

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
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_Q_SHA=\
	".\q_shared.h"\
	

"$(INTDIR)\q_shared.obj" : $(SOURCE) $(DEP_CPP_Q_SHA) "$(INTDIR)"


!ENDIF 

SOURCE=.\runes.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_RUNES=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\runes.obj" : $(SOURCE) $(DEP_CPP_RUNES) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_RUNES=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\runes.obj" : $(SOURCE) $(DEP_CPP_RUNES) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_RUNES=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\runes.obj" : $(SOURCE) $(DEP_CPP_RUNES) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_RUNES=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\runes.h"\
	

"$(INTDIR)\runes.obj" : $(SOURCE) $(DEP_CPP_RUNES) "$(INTDIR)"


!ENDIF 

SOURCE=.\teamplay.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_TEAMP=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\teamplay.obj" : $(SOURCE) $(DEP_CPP_TEAMP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_TEAMP=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\teamplay.obj" : $(SOURCE) $(DEP_CPP_TEAMP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_TEAMP=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\teamplay.obj" : $(SOURCE) $(DEP_CPP_TEAMP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_TEAMP=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\teamplay.h"\
	

"$(INTDIR)\teamplay.obj" : $(SOURCE) $(DEP_CPP_TEAMP) "$(INTDIR)"


!ENDIF 

SOURCE=.\vwep.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_VWEP_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\vwep.h"\
	

"$(INTDIR)\vwep.obj" : $(SOURCE) $(DEP_CPP_VWEP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_VWEP_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\vwep.h"\
	

"$(INTDIR)\vwep.obj" : $(SOURCE) $(DEP_CPP_VWEP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_VWEP_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\vwep.h"\
	

"$(INTDIR)\vwep.obj" : $(SOURCE) $(DEP_CPP_VWEP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_VWEP_=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\vwep.h"\
	

"$(INTDIR)\vwep.obj" : $(SOURCE) $(DEP_CPP_VWEP_) "$(INTDIR)"


!ENDIF 

SOURCE=.\weaponf.c

!IF  "$(CFG)" == "game - Win32 Release"

DEP_CPP_WEAPO=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\weaponf.h"\
	

"$(INTDIR)\weaponf.obj" : $(SOURCE) $(DEP_CPP_WEAPO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug"

DEP_CPP_WEAPO=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\weaponf.h"\
	

"$(INTDIR)\weaponf.obj" : $(SOURCE) $(DEP_CPP_WEAPO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Debug Alpha"

DEP_CPP_WEAPO=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\weaponf.h"\
	

"$(INTDIR)\weaponf.obj" : $(SOURCE) $(DEP_CPP_WEAPO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "game - Win32 Release Alpha"

DEP_CPP_WEAPO=\
	".\g_local.h"\
	".\game.h"\
	".\q_shared.h"\
	".\weaponf.h"\
	

"$(INTDIR)\weaponf.obj" : $(SOURCE) $(DEP_CPP_WEAPO) "$(INTDIR)"


!ENDIF 


!ENDIF 

