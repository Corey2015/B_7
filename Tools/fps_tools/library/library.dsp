# Microsoft Developer Studio Project File - Name="library" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=library - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "library.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "library.mak" CFG="library - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "library - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "library - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "library - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "." /I "..\include" /I ".\windows" /I "include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "__WINDOWS__" /D "__ARDUINO_USBCDC__" /D "__F747B__" /YX /FD /c
# ADD BASE RSC /l 0x404 /d "NDEBUG"
# ADD RSC /l 0x404 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\fps.lib"

!ELSEIF  "$(CFG)" == "library - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "." /I "..\include" /I ".\windows" /I "include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "__WINDOWS__" /D "__ARDUINO_USBCDC__" /D "__F747B__" /D "__DEBUG__" /YX /FD /GZ /c
# ADD BASE RSC /l 0x404 /d "_DEBUG"
# ADD RSC /l 0x404 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\fps.lib"

!ENDIF 

# Begin Target

# Name "library - Win32 Release"
# Name "library - Win32 Debug"
# Begin Group "library"

# PROP Default_Filter ""
# Begin Group "windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\windows\Array.h
# End Source File
# Begin Source File

SOURCE=.\windows\Board.h
# End Source File
# Begin Source File

SOURCE=.\windows\Board_Arduino_USBCDC.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\Board_Arduino_USBCDC.h
# End Source File
# Begin Source File

SOURCE=.\windows\Board_ClassWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\Board_ClassWrapper.h
# End Source File
# Begin Source File

SOURCE=.\windows\Device.h
# End Source File
# Begin Source File

SOURCE=.\windows\Error.h
# End Source File
# Begin Source File

SOURCE=.\windows\fps_control_windows.c
# End Source File
# Begin Source File

SOURCE=.\windows\inttypes.h
# End Source File
# Begin Source File

SOURCE=.\windows\Protocol.h
# End Source File
# Begin Source File

SOURCE=.\windows\SerialPort.h
# End Source File
# Begin Source File

SOURCE=.\windows\SerialPort_Windows.cpp
# End Source File
# Begin Source File

SOURCE=.\windows\SerialPort_Windows.h
# End Source File
# Begin Source File

SOURCE=.\windows\stdint.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\debug.c
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\f747a_control.c
# End Source File
# Begin Source File

SOURCE=.\f747a_control.h
# End Source File
# Begin Source File

SOURCE=.\f747a_register.h
# End Source File
# Begin Source File

SOURCE=.\f747b_control.c
# End Source File
# Begin Source File

SOURCE=.\f747b_control.h
# End Source File
# Begin Source File

SOURCE=.\f747b_register.h
# End Source File
# Begin Source File

SOURCE=.\fps_calibration.c
# End Source File
# Begin Source File

SOURCE=.\fps_calibration.h
# End Source File
# Begin Source File

SOURCE=.\fps_control.c
# End Source File
# Begin Source File

SOURCE=.\fps_control.h
# End Source File
# Begin Source File

SOURCE=.\fps_register.h
# End Source File
# End Group
# Begin Group "include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\fps.h
# End Source File
# End Group
# End Target
# End Project
