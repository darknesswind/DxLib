# Microsoft Developer Studio Project File - Name="DxLibMake" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=DxLibMake - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "DxLibMake.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "DxLibMake.mak" CFG="DxLibMake - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "DxLibMake - Win32 Release" ("Win32 (x86) Static Library" 用)
!MESSAGE "DxLibMake - Win32 Debug" ("Win32 (x86) Static Library" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DxLibMake - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Release\DxLib.lib"

!ELSEIF  "$(CFG)" == "DxLibMake - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /GX /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"Debug\DxLib_d.lib"

!ENDIF 

# Begin Target

# Name "DxLibMake - Win32 Release"
# Name "DxLibMake - Win32 Debug"
# Begin Group "Windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Windows\DxFileWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxFileWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxGraphicsAPIWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxGraphicsAPIWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxGraphicsWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxGraphicsWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxGuid.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxGuid.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxLogWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxLogWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxMemoryWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxMemoryWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxNetworkWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxNetworkWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxShaderCodeBin_Base.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxShaderCodeBin_Filter.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxShaderCodeBin_Model.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxShaderCodeBin_RgbaMix.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxSystemWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxSystemWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxThreadWin.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxThreadWin.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxWinAPI.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxWinAPI.h
# End Source File
# Begin Source File

SOURCE=..\Windows\DxWindow.cpp
# End Source File
# Begin Source File

SOURCE=..\Windows\DxWindow.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\DxArchive_.cpp
# End Source File
# Begin Source File

SOURCE=..\DxArchive_.h
# End Source File
# Begin Source File

SOURCE=..\DxASyncLoad.cpp
# End Source File
# Begin Source File

SOURCE=..\DxASyncLoad.h
# End Source File
# Begin Source File

SOURCE=..\DxBaseFunc.cpp
# End Source File
# Begin Source File

SOURCE=..\DxBaseFunc.h
# End Source File
# Begin Source File

SOURCE=..\DxBaseImage.cpp
# End Source File
# Begin Source File

SOURCE=..\DxBaseImage.h
# End Source File
# Begin Source File

SOURCE=..\DxCompileConfig.h
# End Source File
# Begin Source File

SOURCE=..\DxDataType.h
# End Source File
# Begin Source File

SOURCE=..\DxDataTypeWin.h
# End Source File
# Begin Source File

SOURCE=..\DxDirectX.h
# End Source File
# Begin Source File

SOURCE=..\DxFile.cpp
# End Source File
# Begin Source File

SOURCE=..\DxFile.h
# End Source File
# Begin Source File

SOURCE=..\DxFont.cpp
# End Source File
# Begin Source File

SOURCE=..\DxFont.h
# End Source File
# Begin Source File

SOURCE=..\DxGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\DxGraphics.cpp
# End Source File
# Begin Source File

SOURCE=..\DxGraphics.h
# End Source File
# Begin Source File

SOURCE=..\DxGraphicsAPI.cpp
# End Source File
# Begin Source File

SOURCE=..\DxGraphicsAPI.h
# End Source File
# Begin Source File

SOURCE=..\DxGraphicsBase.cpp
# End Source File
# Begin Source File

SOURCE=..\DxGraphicsBase.h
# End Source File
# Begin Source File

SOURCE=..\DxHandle.cpp
# End Source File
# Begin Source File

SOURCE=..\DxHandle.h
# End Source File
# Begin Source File

SOURCE=..\DxInput.cpp
# End Source File
# Begin Source File

SOURCE=..\DxInput.h
# End Source File
# Begin Source File

SOURCE=..\DxInputString.cpp
# End Source File
# Begin Source File

SOURCE=..\DxInputString.h
# End Source File
# Begin Source File

SOURCE=..\DxKeyHookBinary.cpp
# End Source File
# Begin Source File

SOURCE=..\DxLib.h
# End Source File
# Begin Source File

SOURCE=..\DxLog.cpp
# End Source File
# Begin Source File

SOURCE=..\DxLog.h
# End Source File
# Begin Source File

SOURCE=..\DxMask.cpp
# End Source File
# Begin Source File

SOURCE=..\DxMask.h
# End Source File
# Begin Source File

SOURCE=..\DxMath.cpp
# End Source File
# Begin Source File

SOURCE=..\DxMath.h
# End Source File
# Begin Source File

SOURCE=..\DxMemImg.cpp
# End Source File
# Begin Source File

SOURCE=..\DxMemImg.h
# End Source File
# Begin Source File

SOURCE=..\DxMemory.cpp
# End Source File
# Begin Source File

SOURCE=..\DxMemory.h
# End Source File
# Begin Source File

SOURCE=..\DxModel.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModel.h
# End Source File
# Begin Source File

SOURCE=..\DxModelFile.h
# End Source File
# Begin Source File

SOURCE=..\DxModelLoader0.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModelLoader1.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModelLoader2.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModelLoader3.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModelLoader3.h
# End Source File
# Begin Source File

SOURCE=..\DxModelLoader4.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModelLoader4.h
# End Source File
# Begin Source File

SOURCE=..\DxModelLoaderVMD.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModelLoaderVMD.h
# End Source File
# Begin Source File

SOURCE=..\DxModelRead.cpp
# End Source File
# Begin Source File

SOURCE=..\DxModelRead.h
# End Source File
# Begin Source File

SOURCE=..\DxMovie.cpp
# End Source File
# Begin Source File

SOURCE=..\DxMovie.h
# End Source File
# Begin Source File

SOURCE=..\DxNetwork.cpp
# End Source File
# Begin Source File

SOURCE=..\DxNetwork.h
# End Source File
# Begin Source File

SOURCE=..\DxPixelShader.h
# End Source File
# Begin Source File

SOURCE=..\DxRingBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\DxRingBuffer.h
# End Source File
# Begin Source File

SOURCE=..\DxSoftImage.cpp
# End Source File
# Begin Source File

SOURCE=..\DxSoftImage.h
# End Source File
# Begin Source File

SOURCE=..\DxSound.cpp
# End Source File
# Begin Source File

SOURCE=..\DxSound.h
# End Source File
# Begin Source File

SOURCE=..\DxSoundConvert.cpp
# End Source File
# Begin Source File

SOURCE=..\DxSoundConvert.h
# End Source File
# Begin Source File

SOURCE=..\DxStatic.h
# End Source File
# Begin Source File

SOURCE=..\DxSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\DxSystem.h
# End Source File
# Begin Source File

SOURCE=..\DxThread.cpp
# End Source File
# Begin Source File

SOURCE=..\DxThread.h
# End Source File
# Begin Source File

SOURCE=..\DxVertexShader.h
# End Source File
# End Target
# End Project
