# Microsoft Developer Studio Project File - Name="ref_gl" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
# TARGTYPE "Win32 (ALPHA) Dynamic-Link Library" 0x0602

CFG=ref_gl - Win32 Debug Alpha
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ref_gl.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ref_gl.mak" CFG="ref_gl - Win32 Debug Alpha"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ref_gl - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ref_gl - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ref_gl - Win32 Debug Alpha" (based on "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE "ref_gl - Win32 Release Alpha" (based on "Win32 (ALPHA) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "ref_gl - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\ref_gl__"
# PROP BASE Intermediate_Dir ".\ref_gl__"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\release"
# PROP Intermediate_Dir ".\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
CPP=cl.exe
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G6 /MT /W3 /GX /O2 /Ob2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib libjpeg.lib glu32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"c:\quake2\ref_pbgl.dll"
# SUBTRACT LINK32 /incremental:yes /debug

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\ref_gl__"
# PROP BASE Intermediate_Dir ".\ref_gl__"
# PROP BASE Target_Dir "."
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\debug\ref_gl"
# PROP Intermediate_Dir "..\..\debug\ref_gl"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir "."
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /G5 /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /FR /YX /FD /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /subsystem:windows /dll /profile /map /debug /machine:I386 /nodefaultlib:"LIBCMT" /out:"../../ref_pbgl.dll"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug Alpha"
# PROP BASE Intermediate_Dir "Debug Alpha"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\DebugAxp"
# PROP Intermediate_Dir ".\DebugAxp"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MTd /Gt0 /W3 /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /QA21164 /MTd /Gt0 /W3 /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "C_ONLY" /YX /FD /QAieee1 /c
MTL=midl.exe
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 winmm.lib opengl32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA
# ADD LINK32 opengl32.lib kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /subsystem:windows /dll /debug /machine:ALPHA

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ref_gl__"
# PROP BASE Intermediate_Dir "ref_gl__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\ReleaseAXP"
# PROP Intermediate_Dir ".\ReleaseAXP"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
CPP=cl.exe
# ADD BASE CPP /nologo /MT /Gt0 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /QA21164 /MT /Gt0 /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /YX /FD /QAieee1 /c
# SUBTRACT CPP /Z<none> /Fr
MTL=midl.exe
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
RSC=rc.exe
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib /nologo /subsystem:windows /dll /machine:ALPHA
# SUBTRACT BASE LINK32 /debug
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winmm.lib /nologo /subsystem:windows /dll /machine:ALPHA
# SUBTRACT LINK32 /debug

!ENDIF 

# Begin Target

# Name "ref_gl - Win32 Release"
# Name "ref_gl - Win32 Debug"
# Name "ref_gl - Win32 Debug Alpha"
# Name "ref_gl - Win32 Release Alpha"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "jpeg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\jpeg\jcapimin.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcapistd.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jccoefct.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jccolor.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcdctmgr.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jchuff.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jchuff.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jcinit.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcmainct.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcmarker.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcmaster.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcomapi.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jcparam.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcphuff.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcprepct.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jcsample.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdapimin.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdapistd.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdatadst.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdatasrc.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdcoefct.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdcolor.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdct.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jddctmgr.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdhuff.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdhuff.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jdinput.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdmainct.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdmarker.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdmaster.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdmerge.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdphuff.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdpostct.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jdsample.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jerror.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jerror.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jfdctflt.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jfdctfst.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jfdctint.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jidctflt.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jidctfst.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jidctint.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jidctred.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jinclude.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jmemmgr.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jmemnobs.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jmemsys.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jpegint.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jpeglib.h
# End Source File
# Begin Source File

SOURCE=.\jpeg\jquant1.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jquant2.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jutils.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\jpeg\jversion.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\gl_cin.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_draw.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_DR=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_DR=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_DR=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_DR=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_image.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_IM=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_IM=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_IM=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_IM=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_light.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_LI=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_LI=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_LI=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_LI=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_mesh.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_ME=\
	".\anorms.h"\
	".\anormtab.h"\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_ME=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_ME=\
	".\anorms.h"\
	".\anormtab.h"\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_ME=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_model.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_MO=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_MO=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_MO=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_MO=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_refl.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rmain.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_RM=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_RM=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_RM=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_RM=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rmisc.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_RMI=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_RMI=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_RMI=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_RMI=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_rsurf.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_RS=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_RS=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_RS=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GL_RS=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_script.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gl_warp.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GL_WA=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	".\warpsin.h"\
	
NODEP_CPP_GL_WA=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GL_WA=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	".\warpsin.h"\
	
NODEP_CPP_GL_WA=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\glw_imp.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_GLW_I=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GLW_I=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\glw_win.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\winquake.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_GLW_I=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_GLW_I=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\glw_win.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\winquake.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\game\q_shared.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

NODEP_CPP_Q_SHA=\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

NODEP_CPP_Q_SHA=\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\q_shwin.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

NODEP_CPP_Q_SHW=\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\winquake.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

NODEP_CPP_Q_SHW=\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\winquake.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\win32\qgl_win.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

DEP_CPP_QGL_W=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_QGL_W=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\glw_win.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

DEP_CPP_QGL_W=\
	".\gl_local.h"\
	".\gl_model.h"\
	".\qgl.h"\
	
NODEP_CPP_QGL_W=\
	"..\..\..\..\projects\paintball2\paintball2\client\ref.h"\
	"..\..\..\..\projects\paintball2\paintball2\game\q_shared.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qcommon.h"\
	"..\..\..\..\projects\paintball2\paintball2\qcommon\qfiles.h"\
	"..\..\..\..\projects\paintball2\paintball2\win32\glw_win.h"\
	".\L\gl.h"\
	".\L\glu.h"\
	

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\r_skm.c

!IF  "$(CFG)" == "ref_gl - Win32 Release"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Debug Alpha"

!ELSEIF  "$(CFG)" == "ref_gl - Win32 Release Alpha"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "3dfx"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3dfx.h
# End Source File
# Begin Source File

SOURCE=.\glide.h
# End Source File
# Begin Source File

SOURCE=.\glidesys.h
# End Source File
# Begin Source File

SOURCE=.\glideutl.h
# End Source File
# Begin Source File

SOURCE=.\sst1vid.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\anormtab.h
# End Source File
# Begin Source File

SOURCE="C:\Program Files\Microsoft Visual Studio\VC98\Include\BASETSD.H"
# End Source File
# Begin Source File

SOURCE="F:\Program Files\Microsoft Visual Studio\VC98\Include\BASETSD.H"
# End Source File
# Begin Source File

SOURCE=.\gl_cin.h
# End Source File
# Begin Source File

SOURCE=.\gl_local.h
# End Source File
# Begin Source File

SOURCE=.\gl_model.h
# End Source File
# Begin Source File

SOURCE=.\gl_refl.h
# End Source File
# Begin Source File

SOURCE=.\gl_script.h
# End Source File
# Begin Source File

SOURCE=.\glext.h
# End Source File
# Begin Source File

SOURCE=..\win32\glw_win.h
# End Source File
# Begin Source File

SOURCE=.\jconfig.h
# End Source File
# Begin Source File

SOURCE=.\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=.\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\md5.h
# End Source File
# Begin Source File

SOURCE=..\game\q_shared.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qcommon.h
# End Source File
# Begin Source File

SOURCE=..\qcommon\qfiles.h
# End Source File
# Begin Source File

SOURCE=.\qgl.h
# End Source File
# Begin Source File

SOURCE=.\qmenu.h
# End Source File
# Begin Source File

SOURCE=..\client\ref.h
# End Source File
# Begin Source File

SOURCE=.\ref_gl.h
# End Source File
# Begin Source File

SOURCE=.\warpsin.h
# End Source File
# Begin Source File

SOURCE=..\win32\winquake.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\ref_gl.def
# End Source File
# End Group
# End Target
# End Project
