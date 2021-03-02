# Microsoft Developer Studio Project File - Name="SWTC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SWTC - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SWTC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SWTC.mak" CFG="SWTC - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SWTC - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SWTC - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SWTC - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SWTC_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SWTC_EXPORTS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "NDEBUG"
# ADD RSC /l 0x1409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib /nologo /dll /machine:I386 /out:"d:\quake2\swtc\gamex86.dll"

!ELSEIF  "$(CFG)" == "SWTC - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SWTC_EXPORTS" /YX /FD /ZI /GZ /c
# ADD CPP /nologo /MTd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SWTC_EXPORTS" /FR /YX /FD /ZI /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1409 /d "_DEBUG"
# ADD RSC /l 0x1409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib opengl32.lib glu32.lib /nologo /dll /debug /machine:I386 /out:"..\gamex86.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SWTC - Win32 Release"
# Name "SWTC - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\source\b_ai.c
# End Source File
# Begin Source File

SOURCE=.\source\b_spawn.c
# End Source File
# Begin Source File

SOURCE=.\source\fog.c
# End Source File
# Begin Source File

SOURCE=.\source\g_ai.c
# End Source File
# Begin Source File

SOURCE=.\source\g_chase.c
# End Source File
# Begin Source File

SOURCE=.\source\g_cmds.c
# End Source File
# Begin Source File

SOURCE=.\source\g_combat.c
# End Source File
# Begin Source File

SOURCE=.\source\g_ctf.c
# End Source File
# Begin Source File

SOURCE=.\source\g_func.c
# End Source File
# Begin Source File

SOURCE=.\source\g_items.c
# End Source File
# Begin Source File

SOURCE=.\source\g_main.c
# End Source File
# Begin Source File

SOURCE=.\source\g_misc.c
# End Source File
# Begin Source File

SOURCE=.\source\g_monster.c
# End Source File
# Begin Source File

SOURCE=.\source\g_npc_ai.c
# End Source File
# Begin Source File

SOURCE=.\source\g_phys.c
# End Source File
# Begin Source File

SOURCE=.\source\g_save.c
# End Source File
# Begin Source File

SOURCE=.\source\g_spawn.c
# End Source File
# Begin Source File

SOURCE=.\source\g_svcmds.c
# End Source File
# Begin Source File

SOURCE=.\source\g_target.c
# End Source File
# Begin Source File

SOURCE=.\source\g_trigger.c
# End Source File
# Begin Source File

SOURCE=.\source\g_utils.c
# End Source File
# Begin Source File

SOURCE=.\source\g_weapon.c
# End Source File
# Begin Source File

SOURCE=.\source\game.def
# End Source File
# Begin Source File

SOURCE=.\source\m_bird.c
# End Source File
# Begin Source File

SOURCE=.\source\m_flash.c
# End Source File
# Begin Source File

SOURCE=.\source\m_jawa.c
# End Source File
# Begin Source File

SOURCE=.\source\m_jawa2.c
# End Source File
# Begin Source File

SOURCE=.\source\m_jawa3.c
# End Source File
# Begin Source File

SOURCE=.\source\m_map.c
# End Source File
# Begin Source File

SOURCE=.\source\m_move.c
# End Source File
# Begin Source File

SOURCE=.\source\m_officer.c
# End Source File
# Begin Source File

SOURCE=.\source\m_r2.c
# End Source File
# Begin Source File

SOURCE=.\source\m_route.c
# End Source File
# Begin Source File

SOURCE=.\source\m_ske27.c
# End Source File
# Begin Source File

SOURCE=.\source\m_trooper.c
# End Source File
# Begin Source File

SOURCE=.\source\m_utils.c
# End Source File
# Begin Source File

SOURCE=.\source\m_vehicle.c
# End Source File
# Begin Source File

SOURCE=.\source\n_gonk.c
# End Source File
# Begin Source File

SOURCE=.\source\n_male.c
# End Source File
# Begin Source File

SOURCE=.\source\n_mse.c
# End Source File
# Begin Source File

SOURCE=.\source\n_officer.c
# End Source File
# Begin Source File

SOURCE=.\source\n_raptor.c
# End Source File
# Begin Source File

SOURCE=.\source\n_rat.c
# End Source File
# Begin Source File

SOURCE=.\source\n_smuggler.c
# End Source File
# Begin Source File

SOURCE=.\source\n_sparrow.c
# End Source File
# Begin Source File

SOURCE=.\source\p_cam.c
# End Source File
# Begin Source File

SOURCE=.\source\p_client.c
# End Source File
# Begin Source File

SOURCE=.\source\p_force.c
# End Source File
# Begin Source File

SOURCE=.\source\p_hud.c
# End Source File
# Begin Source File

SOURCE=.\source\p_menu.c
# End Source File
# Begin Source File

SOURCE=.\source\p_trail.c
# End Source File
# Begin Source File

SOURCE=.\source\p_view.c
# End Source File
# Begin Source File

SOURCE=.\source\p_weapon.c
# End Source File
# Begin Source File

SOURCE=.\source\q_shared.c
# End Source File
# Begin Source File

SOURCE=.\source\s_computer.c
# End Source File
# Begin Source File

SOURCE=.\source\s_conversation.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\source\s_conversation.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
