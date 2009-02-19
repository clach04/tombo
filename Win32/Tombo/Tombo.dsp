# Microsoft Developer Studio Project File - Name="Tombo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Tombo - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "Tombo.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "Tombo.mak" CFG="Tombo - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "Tombo - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "Tombo - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Tombo/Win32/Tombo", OKAAAAAA"
# PROP Scc_LocalPath "Desktop"
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Tombo - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../../lib/" /I "../../Src" /I "../../oniguruma/" /I "../../Src/regex" /I "./res" /I "../../expat/include/" /I "../../Src/TomboLib" /I "../../Src/YAEdit" /I "." /D "NDEBUG" /D "TOMBO" /D "PLATFORM_WIN32" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib imm32.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"LIBCMT.LIB"

!ELSEIF  "$(CFG)" == "Tombo - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../../lib/" /I "../../Src" /I "../../oniguruma/" /I "../../Src/regex" /I "./res" /I "../../expat/include/" /I "../../Src/TomboLib" /I "../../Src/YAEdit" /I "." /D "_DEBUG" /D "TOMBO" /D "PLATFORM_WIN32" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib imm32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"LIBCMT.LIB" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "Tombo - Win32 Release"
# Name "Tombo - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\Src\AboutDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\gnupg\blowfish.c
# End Source File
# Begin Source File

SOURCE=..\..\Src\BookMark.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\BookMarkDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\Clipboard.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Crypt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\CryptManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\DetailsViewWndProc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\DialogTemplate.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\DirectoryScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\DirList.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\FileSelector.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\FilterAddDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\FilterCtlDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\FilterDefDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\GrepDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MainFrame.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\gnupg\md5.c
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoDetailsView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoFolder.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoNote.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoSelectView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Message.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\NewFolderDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\PlatformLayer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Property.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\PropertyPage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Repository.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\RepositoryImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchEngine.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchTree.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\SelectViewWndProc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\SipControl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\StatusBar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\StringSplitter.cpp
# End Source File
# Begin Source File

SOURCE=.\res\Tombo.rc
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboPropertyTab.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboURI.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TreeViewItem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TSParser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\URIScanner.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\regex\util.c
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\VarBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\VFManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\VFStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\Win32Platform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEditor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\Src\AboutDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\BookMark.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\BookMarkDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Chipher.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\Clipboard.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Crypt.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\CryptManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\DialogTemplate.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\DirectoryScanner.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\DirList.h
# End Source File
# Begin Source File

SOURCE=..\..\expat\include\expat.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\FileSelector.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\FilterAddDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\FilterCtlDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\FilterDefDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\GrepDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MainFrame.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoDetailsView.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoFolder.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoNote.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\MemoSelectView.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Message.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\NewFolderDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\oniguruma\oniguruma.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordDialog.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\PasswordManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\PlatformLayer.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Property.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\PropertyPage.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\RegexUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Repository.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\RepositoryImpl.h
# End Source File
# Begin Source File

SOURCE=.\res\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchDlg.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchEngine.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\SearchTree.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\SipControl.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\StatusBar.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\StringSplitter.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Tombo.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboPropertyTab.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboURI.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TreeViewItem.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TSParser.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\URIScanner.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\VFManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\VFStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\Win32Platform.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEditor.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\Src\image\draftimg.bmp
# End Source File
# Begin Source File

SOURCE=.\draftimg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\draftimg.bmp
# End Source File
# Begin Source File

SOURCE=..\..\Src\image\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=..\..\Src\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=.\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=.\res\memoimg.bmp
# End Source File
# Begin Source File

SOURCE=..\..\Src\image\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=.\res\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=.\resE\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=.\Tombo.ICO
# End Source File
# Begin Source File

SOURCE=..\..\Src\image\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\toolbar.bmp
# End Source File
# End Group
# Begin Group "YAEdit"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Src\YAEdit\FontWidthCache.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\FontWidthCache.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\LineManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\LineManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\LineWrapper.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\LineWrapper.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\LineWrapperImpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\LineWrapperImpl.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\MemManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\MemManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\PhysicalLineManager.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\PhysicalLineManager.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\Region.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\Region.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\YAEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\YAEdit.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\YAEditDoc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\YAEditDoc.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\YAEditView.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\YAEdit\YAEditView.h
# End Source File
# End Group
# Begin Group "utils"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\Src\TomboLib\AutoPtr.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\AutoPtr.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\File.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\File.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\FileInputStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\FileInputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\List.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\List.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\Logger.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\Logger.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\TString.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\TString.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\Uniconv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\Uniconv.h
# End Source File
# Begin Source File

SOURCE=..\..\Src\TomboLib\VarBuffer.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\expat\lib\Win32\libexpatw.lib
# End Source File
# Begin Source File

SOURCE=..\..\oniguruma\win32\oniguruma.lib
# End Source File
# End Target
# End Project
