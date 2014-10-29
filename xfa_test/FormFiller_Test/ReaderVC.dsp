# Microsoft Developer Studio Project File - Name="ReaderVC" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=ReaderVC - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ReaderVC.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ReaderVC.mak" CFG="ReaderVC - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ReaderVC - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ReaderVC - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ReaderVC - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "FPDFSDK_EXPORTS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /out:"../../lib/rel_w32_vc6/ReaderVC.exe"

!ELSEIF  "$(CFG)" == "ReaderVC - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "FPDFSDK_EXPORTS" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Msimg32.lib /nologo /subsystem:windows /map /debug /machine:I386 /out:"../../lib/dbg_w32_vc6/ReaderVC.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "ReaderVC - Win32 Release"
# Name "ReaderVC - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BookMarkView.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ConvertDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportPage.cpp
# End Source File
# Begin Source File

SOURCE=.\FindDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\FX_SplitterWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\GotoPageDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\JS_ResponseDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\ReaderVC.cpp
# End Source File
# Begin Source File

SOURCE=.\ReaderVC.rc
# End Source File
# Begin Source File

SOURCE=.\ReaderVCDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\ReaderVCView.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TestJsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ZoomDlg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BookMarkView.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\ConvertDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExportPage.h
# End Source File
# Begin Source File

SOURCE=.\FindDlg.h
# End Source File
# Begin Source File

SOURCE=.\FX_SplitterWnd.h
# End Source File
# Begin Source File

SOURCE=.\GotoPageDlg.h
# End Source File
# Begin Source File

SOURCE=.\JS_ResponseDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\ReaderVC.h
# End Source File
# Begin Source File

SOURCE=.\ReaderVCDoc.h
# End Source File
# Begin Source File

SOURCE=.\ReaderVCView.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TestJsDlg.h
# End Source File
# Begin Source File

SOURCE=.\ZoomDlg.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\BigHandCursor.cur
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap12.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap19.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap20.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap21.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap22.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap23.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap26.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bookmark.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bound.cur
# End Source File
# Begin Source File

SOURCE=.\res\formOptions.bmp
# End Source File
# Begin Source File

SOURCE=.\res\point.cur
# End Source File
# Begin Source File

SOURCE=.\res\Rd_SelText_Icon.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Rd_SelText_Icon_Dis.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ReaderVC.ico
# End Source File
# Begin Source File

SOURCE=.\res\ReaderVC.rc2
# End Source File
# Begin Source File

SOURCE=.\res\ReaderVCDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\SmallHandCursor.cur
# End Source File
# Begin Source File

SOURCE=.\res\tool_actual_size.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_bookmark1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_clockwise.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_counterclockwise.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_first.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_fit_page.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_fit_width.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hand1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_about2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_actual_size.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_bookmark1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_clockwise.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_counterclockwise.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_first.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_fit_page.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_fit_width.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_hand1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_last.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_next.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_open1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_prev.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_print.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_search.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_snap_shot.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_zoomin.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_hot_zoomout.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_last.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_next.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_prev.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_print.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_search.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_snap_shot.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_zoomin.bmp
# End Source File
# Begin Source File

SOURCE=.\res\tool_zoomout.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\unknowncolor_gray.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
