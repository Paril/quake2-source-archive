# Microsoft Developer Studio Project File - Name="niq" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=niq - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "niq.mak".
!MESSAGE 
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

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "niq - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "NIQ" /D "NIQBOTS" /D "EBOTS" /D "OHOOK" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 ../../navlib/release/navlib.lib /nologo /subsystem:windows /dll /machine:I386 /nodefaultlib:"libc" /out:"Release/gamex86.dll"
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
InputPath=.\Release\gamex86.dll
SOURCE=$(InputPath)

"c:\quake2\niq\gamex86.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xcopy release\gamex86.dll c:\quake2\niq

# End Custom Build

!ELSEIF  "$(CFG)" == "niq - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "NIQ" /D "NIQBOTS" /D "EBOTS" /D "OHOOK" /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ../navlib/debug/navlib.lib /nologo /subsystem:windows /dll /incremental:no /debug /debugtype:coff /machine:I386 /nodefaultlib:"libcd" /out:"Debug/gamex86.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none
# Begin Custom Build
InputPath=.\Debug\gamex86.dll
SOURCE=$(InputPath)

"c:\quake2\niq\gamex86.dll" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	xcopy debug\gamex86.dll c:\quake2\niq

# End Custom Build

!ENDIF 

# Begin Target

# Name "niq - Win32 Release"
# Name "niq - Win32 Debug"
# Begin Source File

SOURCE=.\bot_ai.c
# End Source File
# Begin Source File

SOURCE=.\bot_die.c
# End Source File
# Begin Source File

SOURCE=.\bot_items.c
# End Source File
# Begin Source File

SOURCE=.\bot_misc.c
# End Source File
# Begin Source File

SOURCE=.\bot_nav.c
# End Source File
# Begin Source File

SOURCE=.\bot_spawn.c
# End Source File
# Begin Source File

SOURCE=.\bot_wpns.c
# End Source File
# Begin Source File

SOURCE=.\camclient.c
# End Source File
# Begin Source File

SOURCE=.\camclient.h
# End Source File
# Begin Source File

SOURCE=.\extra.h
# End Source File
# Begin Source File

SOURCE=.\g_ai.c
# End Source File
# Begin Source File

SOURCE=.\g_chase.c
# End Source File
# Begin Source File

SOURCE=.\g_cmds.c
# End Source File
# Begin Source File

SOURCE=.\g_combat.c
# End Source File
# Begin Source File

SOURCE=.\g_ctf.c
# End Source File
# Begin Source File

SOURCE=.\g_ctf.h
# End Source File
# Begin Source File

SOURCE=.\g_func.c
# End Source File
# Begin Source File

SOURCE=.\g_items.c
# End Source File
# Begin Source File

SOURCE=.\g_items.h
# End Source File
# Begin Source File

SOURCE=.\g_local.h
# End Source File
# Begin Source File

SOURCE=.\g_main.c
# End Source File
# Begin Source File

SOURCE=.\g_map_mod.c
# End Source File
# Begin Source File

SOURCE=.\g_map_mod.h
# End Source File
# Begin Source File

SOURCE=.\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\g_monster.c
# End Source File
# Begin Source File

SOURCE=.\g_niq.c
# End Source File
# Begin Source File

SOURCE=.\g_niq.h
# End Source File
# Begin Source File

SOURCE=.\g_phys.c
# End Source File
# Begin Source File

SOURCE=.\g_save.c
# End Source File
# Begin Source File

SOURCE=.\g_spawn.c
# End Source File
# Begin Source File

SOURCE=.\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=.\g_target.c
# End Source File
# Begin Source File

SOURCE=.\g_trigger.c
# End Source File
# Begin Source File

SOURCE=.\g_unzip.c
# End Source File
# Begin Source File

SOURCE=.\g_unzip.h
# End Source File
# Begin Source File

SOURCE=.\g_utils.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\g_zip.c
# End Source File
# Begin Source File

SOURCE=.\g_zip.h
# End Source File
# Begin Source File

SOURCE=.\game.def
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\gslog.c
# End Source File
# Begin Source File

SOURCE=.\hook.c
# End Source File
# Begin Source File

SOURCE=.\m_move.c
# End Source File
# Begin Source File

SOURCE=.\m_player.h
# End Source File
# Begin Source File

SOURCE=.\p_client.c
# End Source File
# Begin Source File

SOURCE=.\p_hud.c
# End Source File
# Begin Source File

SOURCE=.\p_menu.c
# End Source File
# Begin Source File

SOURCE=.\p_menu.h
# End Source File
# Begin Source File

SOURCE=.\p_trail.h
# End Source File
# Begin Source File

SOURCE=.\p_view.c
# End Source File
# Begin Source File

SOURCE=.\p_weapon.c
# End Source File
# Begin Source File

SOURCE=.\q_shared.c
# End Source File
# Begin Source File

SOURCE=.\q_shared.h
# End Source File
# Begin Source File

SOURCE=.\stdlog.c
# End Source File
# Begin Source File

SOURCE=.\z_api.h
# End Source File
# Begin Source File

SOURCE=.\z_unzip.h
# End Source File
# Begin Source File

SOURCE=.\z_zip.h
# End Source File
# End Target
# End Project
