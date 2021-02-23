# Microsoft Developer Studio Project File - Name="gunslinger" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=gunslinger - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gunslinger.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gunslinger.mak" CFG="gunslinger - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gunslinger - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gunslinger - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gunslinger - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "gunsling"
# PROP BASE Intermediate_Dir "gunsling"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"gamex86.dll"

!ELSEIF  "$(CFG)" == "gunslinger - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "gunslinger - Win32 Release"
# Name "gunslinger - Win32 Debug"
# Begin Source File

SOURCE=.\src\botstuff.c
# End Source File
# Begin Source File

SOURCE=.\src\compile.bat
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

SOURCE=.\src\g_local.h
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

SOURCE=.\src\game.dsp
# End Source File
# Begin Source File

SOURCE=.\src\game.h
# End Source File
# Begin Source File

SOURCE=.\src\game.plg
# End Source File
# Begin Source File

SOURCE=.\gunslinger.ico
# End Source File
# Begin Source File

SOURCE=.\gunslinger2.ico
# End Source File
# Begin Source File

SOURCE=.\src\m_actor.c
# End Source File
# Begin Source File

SOURCE=.\src\m_actor.h
# End Source File
# Begin Source File

SOURCE=.\src\m_berserk.c
# End Source File
# Begin Source File

SOURCE=.\src\m_berserk.h
# End Source File
# Begin Source File

SOURCE=.\src\m_boss2.c
# End Source File
# Begin Source File

SOURCE=.\src\m_boss2.h
# End Source File
# Begin Source File

SOURCE=.\src\m_boss3.c
# End Source File
# Begin Source File

SOURCE=.\src\m_boss31.c
# End Source File
# Begin Source File

SOURCE=.\src\m_boss31.h
# End Source File
# Begin Source File

SOURCE=.\src\m_boss32.c
# End Source File
# Begin Source File

SOURCE=.\src\m_boss32.h
# End Source File
# Begin Source File

SOURCE=.\src\m_brain.c
# End Source File
# Begin Source File

SOURCE=.\src\m_brain.h
# End Source File
# Begin Source File

SOURCE=.\src\m_chick.c
# End Source File
# Begin Source File

SOURCE=.\src\m_chick.h
# End Source File
# Begin Source File

SOURCE=.\src\m_flash.c
# End Source File
# Begin Source File

SOURCE=.\src\m_flipper.c
# End Source File
# Begin Source File

SOURCE=.\src\m_flipper.h
# End Source File
# Begin Source File

SOURCE=.\src\m_float.c
# End Source File
# Begin Source File

SOURCE=.\src\m_float.h
# End Source File
# Begin Source File

SOURCE=.\src\m_flyer.c
# End Source File
# Begin Source File

SOURCE=.\src\m_flyer.h
# End Source File
# Begin Source File

SOURCE=.\src\m_gladiator.c
# End Source File
# Begin Source File

SOURCE=.\src\m_gladiator.h
# End Source File
# Begin Source File

SOURCE=.\src\m_gunner.c
# End Source File
# Begin Source File

SOURCE=.\src\m_gunner.h
# End Source File
# Begin Source File

SOURCE=.\src\m_hover.c
# End Source File
# Begin Source File

SOURCE=.\src\m_hover.h
# End Source File
# Begin Source File

SOURCE=.\src\m_infantry.c
# End Source File
# Begin Source File

SOURCE=.\src\m_infantry.h
# End Source File
# Begin Source File

SOURCE=.\src\m_insane.c
# End Source File
# Begin Source File

SOURCE=.\src\m_insane.h
# End Source File
# Begin Source File

SOURCE=.\src\m_medic.c
# End Source File
# Begin Source File

SOURCE=.\src\m_medic.h
# End Source File
# Begin Source File

SOURCE=.\src\m_move.c
# End Source File
# Begin Source File

SOURCE=.\src\m_mutant.c
# End Source File
# Begin Source File

SOURCE=.\src\m_mutant.h
# End Source File
# Begin Source File

SOURCE=.\src\m_parasite.c
# End Source File
# Begin Source File

SOURCE=.\src\m_parasite.h
# End Source File
# Begin Source File

SOURCE=.\src\m_player.h
# End Source File
# Begin Source File

SOURCE=.\src\m_rider.h
# End Source File
# Begin Source File

SOURCE=.\src\m_soldier.c
# End Source File
# Begin Source File

SOURCE=.\src\m_soldier.h
# End Source File
# Begin Source File

SOURCE=.\src\m_supertank.c
# End Source File
# Begin Source File

SOURCE=.\src\m_supertank.h
# End Source File
# Begin Source File

SOURCE=.\src\m_tank.c
# End Source File
# Begin Source File

SOURCE=.\src\m_tank.h
# End Source File
# Begin Source File

SOURCE=.\src\makefile
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

SOURCE=.\src\q_devels.c
# End Source File
# Begin Source File

SOURCE=.\src\q_devels.h
# End Source File
# Begin Source File

SOURCE=.\src\q_shared.c
# End Source File
# Begin Source File

SOURCE=.\src\q_shared.h
# End Source File
# Begin Source File

SOURCE=.\src\z_gq.c
# End Source File
# Begin Source File

SOURCE=.\src\z_gq.h
# End Source File
# Begin Source File

SOURCE=.\src\z_menu.c
# End Source File
# Begin Source File

SOURCE=.\src\z_menu.h
# End Source File
# End Target
# End Project
