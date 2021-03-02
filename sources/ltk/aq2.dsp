# Microsoft Developer Studio Project File - Name="AQ2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=AQ2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "AQ2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AQ2.mak" CFG="AQ2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AQ2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "AQ2 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/LicenseToKill", BAAAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "AQ2 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AQ2_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AQ2_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib /nologo /dll /machine:I386 /out:"c:\quake2\action\gamex86.dll"

!ELSEIF  "$(CFG)" == "AQ2 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AQ2_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "AQ2_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib /nologo /dll /debug /machine:I386 /out:"c:\quake2\action\gamex86.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "AQ2 - Win32 Release"
# Name "AQ2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\a_cmds.c
# End Source File
# Begin Source File

SOURCE=.\src\a_doorkick.c
# End Source File
# Begin Source File

SOURCE=.\src\a_game.c
# End Source File
# Begin Source File

SOURCE=.\src\a_items.c
# End Source File
# Begin Source File

SOURCE=.\src\a_menu.c
# End Source File
# Begin Source File

SOURCE=.\src\a_radio.c
# End Source File
# Begin Source File

SOURCE=.\src\a_team.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot_ai.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot_cmds.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot_compress.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot_items.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot_movement.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot_nodes.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot_spawn.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\botchat.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\botnav.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\botscan.c
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\cgf_sfx_fog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cgf_sfx_glass.c
# End Source File
# Begin Source File

SOURCE=.\src\g_ai.c
# End Source File
# Begin Source File

SOURCE=.\src\g_chase.c
# End Source File
# Begin Source File

SOURCE=.\src\g_cmds.c
# End Source File
# Begin Source File

SOURCE=.\src\g_combat.c
# End Source File
# Begin Source File

SOURCE=.\src\g_func.c
# End Source File
# Begin Source File

SOURCE=.\src\g_items.c
# End Source File
# Begin Source File

SOURCE=.\src\g_main.c
# End Source File
# Begin Source File

SOURCE=.\src\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\src\g_monster.c
# End Source File
# Begin Source File

SOURCE=.\src\g_phys.c
# End Source File
# Begin Source File

SOURCE=.\src\g_save.c
# End Source File
# Begin Source File

SOURCE=.\src\g_spawn.c
# End Source File
# Begin Source File

SOURCE=.\src\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=.\src\g_target.c
# End Source File
# Begin Source File

SOURCE=.\src\g_trigger.c
# End Source File
# Begin Source File

SOURCE=.\src\g_turret.c
# End Source File
# Begin Source File

SOURCE=.\src\g_utils.c
# End Source File
# Begin Source File

SOURCE=.\src\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\src\game.def
# End Source File
# Begin Source File

SOURCE=.\src\m_move.c
# End Source File
# Begin Source File

SOURCE=.\src\p_client.c
# End Source File
# Begin Source File

SOURCE=.\src\p_hud.c
# End Source File
# Begin Source File

SOURCE=.\src\p_trail.c
# End Source File
# Begin Source File

SOURCE=.\src\p_view.c
# End Source File
# Begin Source File

SOURCE=.\src\p_weapon.c
# End Source File
# Begin Source File

SOURCE=.\src\q_shared.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\a_game.h
# End Source File
# Begin Source File

SOURCE=.\src\a_menu.h
# End Source File
# Begin Source File

SOURCE=.\src\a_radio.h
# End Source File
# Begin Source File

SOURCE=.\src\a_team.h
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\acebot.h
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\botchat.h
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\botnav.h
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\botscan.h
# End Source File
# Begin Source File

SOURCE=.\src\acesrc\cgf_sfx_fog.h
# End Source File
# Begin Source File

SOURCE=.\src\cgf_sfx_glass.h
# End Source File
# Begin Source File

SOURCE=.\src\g_local.h
# End Source File
# Begin Source File

SOURCE=.\src\game.h
# End Source File
# Begin Source File

SOURCE=.\src\m_player.h
# End Source File
# Begin Source File

SOURCE=.\src\q_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Docs"

# PROP Default_Filter "*.txt"
# Begin Source File

SOURCE=.\history.txt
# End Source File
# Begin Source File

SOURCE=.\LTKBots.txt
# End Source File
# End Group
# End Target
# End Project
