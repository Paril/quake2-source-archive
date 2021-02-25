# Microsoft Developer Studio Project File - Name="game" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=GAME - WIN32 TCTC RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "game.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "game.mak" CFG="GAME - WIN32 TCTC RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "game - Win32 TCTC Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "game___2"
# PROP BASE Intermediate_Dir "game___2"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Build Files\Release"
# PROP Intermediate_Dir "Build Files\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /MT /W3 /GX /O2 /I "./" /I "./chicken" /I "./stdlog" /I "./q2cam" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STD_LOGGING" /FR /YX /FD /c
# ADD CPP /nologo /G5 /MT /W3 /GX /O2 /I "./" /I "./chicken" /I "./stdlog" /I "./q2cam" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "STD_LOGGING" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# SUBTRACT BASE RSC /x
# ADD RSC /l 0xc09 /d "NDEBUG"
# SUBTRACT RSC /x
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /machine:I386 /out:"..\gamex86.dll"
# SUBTRACT BASE LINK32 /incremental:yes /map /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /base:"0x20000000" /subsystem:windows /dll /machine:I386 /out:"..\gamex86.dll"
# SUBTRACT LINK32 /incremental:yes /map /debug
# Begin Target

# Name "game - Win32 TCTC Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\q2cam\camclient.c
# End Source File
# Begin Source File

SOURCE=.\chicken\ctc.c
# End Source File
# Begin Source File

SOURCE=.\g_ai.c
# End Source File
# Begin Source File

SOURCE=.\g_cmds.c
# End Source File
# Begin Source File

SOURCE=.\g_combat.c
# End Source File
# Begin Source File

SOURCE=.\g_func.c
# End Source File
# Begin Source File

SOURCE=.\g_items.c
# End Source File
# Begin Source File

SOURCE=.\g_main.c
# End Source File
# Begin Source File

SOURCE=.\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\g_monster.c
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

SOURCE=.\g_turret.c
# End Source File
# Begin Source File

SOURCE=.\g_utils.c
# End Source File
# Begin Source File

SOURCE=.\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\game.def
# End Source File
# Begin Source File

SOURCE=.\StdLog\gslog.c
# End Source File
# Begin Source File

SOURCE=.\m_actor.c
# End Source File
# Begin Source File

SOURCE=.\m_berserk.c
# End Source File
# Begin Source File

SOURCE=.\m_boss2.c
# End Source File
# Begin Source File

SOURCE=.\m_boss3.c
# End Source File
# Begin Source File

SOURCE=.\m_boss31.c
# End Source File
# Begin Source File

SOURCE=.\m_boss32.c
# End Source File
# Begin Source File

SOURCE=.\m_brain.c
# End Source File
# Begin Source File

SOURCE=.\m_chick.c
# End Source File
# Begin Source File

SOURCE=.\m_flash.c
# End Source File
# Begin Source File

SOURCE=.\m_flipper.c
# End Source File
# Begin Source File

SOURCE=.\m_float.c
# End Source File
# Begin Source File

SOURCE=.\m_flyer.c
# End Source File
# Begin Source File

SOURCE=.\m_gladiator.c
# End Source File
# Begin Source File

SOURCE=.\m_gunner.c
# End Source File
# Begin Source File

SOURCE=.\m_hover.c
# End Source File
# Begin Source File

SOURCE=.\m_infantry.c
# End Source File
# Begin Source File

SOURCE=.\m_insane.c
# End Source File
# Begin Source File

SOURCE=.\m_medic.c
# End Source File
# Begin Source File

SOURCE=.\m_move.c
# End Source File
# Begin Source File

SOURCE=.\m_mutant.c
# End Source File
# Begin Source File

SOURCE=.\m_parasite.c
# End Source File
# Begin Source File

SOURCE=.\m_soldier.c
# End Source File
# Begin Source File

SOURCE=.\m_supertank.c
# End Source File
# Begin Source File

SOURCE=.\m_tank.c
# End Source File
# Begin Source File

SOURCE=.\chicken\menu.c
# End Source File
# Begin Source File

SOURCE=.\p_client.c
# End Source File
# Begin Source File

SOURCE=.\p_hud.c
# End Source File
# Begin Source File

SOURCE=.\p_trail.c
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

SOURCE=.\StdLog\stdlog.c
# End Source File
# Begin Source File

SOURCE=.\chicken\tctc.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\q2cam\camclient.h
# End Source File
# Begin Source File

SOURCE=.\chicken\ctc.h
# End Source File
# Begin Source File

SOURCE=.\chicken\ctclocal.h
# End Source File
# Begin Source File

SOURCE=.\chicken\frames.h
# End Source File
# Begin Source File

SOURCE=.\g_local.h
# End Source File
# Begin Source File

SOURCE=.\game.h
# End Source File
# Begin Source File

SOURCE=.\StdLog\gslog.h
# End Source File
# Begin Source File

SOURCE=.\m_berserk.h
# End Source File
# Begin Source File

SOURCE=.\m_boss2.h
# End Source File
# Begin Source File

SOURCE=.\m_brain.h
# End Source File
# Begin Source File

SOURCE=.\m_chick.h
# End Source File
# Begin Source File

SOURCE=.\m_flipper.h
# End Source File
# Begin Source File

SOURCE=.\m_float.h
# End Source File
# Begin Source File

SOURCE=.\m_flyer.h
# End Source File
# Begin Source File

SOURCE=.\m_gladiator.h
# End Source File
# Begin Source File

SOURCE=.\m_gunner.h
# End Source File
# Begin Source File

SOURCE=.\m_hover.h
# End Source File
# Begin Source File

SOURCE=.\m_infantry.h
# End Source File
# Begin Source File

SOURCE=.\m_insane.h
# End Source File
# Begin Source File

SOURCE=.\m_medic.h
# End Source File
# Begin Source File

SOURCE=.\m_mutant.h
# End Source File
# Begin Source File

SOURCE=.\m_parasite.h
# End Source File
# Begin Source File

SOURCE=.\m_soldier.h
# End Source File
# Begin Source File

SOURCE=.\m_supertank.h
# End Source File
# Begin Source File

SOURCE=.\m_tank.h
# End Source File
# Begin Source File

SOURCE=.\chicken\menu.h
# End Source File
# Begin Source File

SOURCE=.\q_shared.h
# End Source File
# Begin Source File

SOURCE=.\StdLog\stdlog.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
