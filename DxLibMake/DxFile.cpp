// -------------------------------------------------------------------------------
// 
// 		ＤＸLibrary		FileAccessProgram
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ------------------------------------------------------------------
#include "DxFile.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxUseCLib.h"
#include "DxASyncLoad.h"
#include "DxSystem.h"
#include "DxLog.h"

#ifndef DX_NON_DXA
#include "DxArchive_.h"
#endif

#ifndef DX_NON_PRINTF_DX
#include "DxFont.h"
#endif

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// FileAccessHandle有效性检查
#define FILEHCHK( HAND, FPOINT )		HANDLECHK(       DX_HANDLETYPE_FILE, HAND, *( ( HANDLEINFO ** )&FPOINT ) )
#define FILEHCHK_ASYNC( HAND, FPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_FILE, HAND, *( ( HANDLEINFO ** )&FPOINT ) )

// 结构体定义 --------------------------------------------------------------------

// メモリに置かれたデータをファイルとして扱うためのデータ構造体
typedef struct tagMEMSTREAMDATA
{
	unsigned char			*DataBuffer ;
	size_t					DataSize ;
	size_t					DataPoint ;
	int						EOFFlag ;
} MEMSTREAMDATA, *LPMEMSTREAMDATA ;

// FileAccess处理用全局数据结构
struct FILEMANAGEDATA
{
	int						InitializeFlag ;					// 初期化フラグ
} ;

// 文件信息List结构体
struct FILEINFOLIST
{
	int Num;		// 文件数
					// ここに FILEINFO が Num の数だけ存在する
} ;


// 内部大域変数宣言 --------------------------------------------------------------

// FileAccess处理用全局数据结构
FILEMANAGEDATA GFileData ;

// 存档文件读取功能是否使用的Flag
int NotUseDXArchive ;


// 函数原型声明 ----------------------------------------------------------

// 文件句柄函数
static	int			InitializeFileHandle( HANDLEINFO *HandleInfo ) ;								// 初始化文件访问句柄
static	int			TerminateFileHandle( HANDLEINFO *HandleInfo ) ;									// 释放文件访问句柄


// メモリに置かれたデータをファイルのデータに例えてつかうための関数
static	LONGLONG	MemStreamTell( void *StreamDataPoint ) ;
static	int			MemStreamSeek( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType ) ;
static	size_t		MemStreamRead( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) ;
static	size_t		MemStreamWrite( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) ;
static	int			MemStreamEof( void *StreamDataPoint ) ;
static	int			MemStreamIdleCheck( void *StreamDataPoint ) ;


// 数据声明 --------------------------------------------------------------------

// StreamData访问用函数
STREAMDATASHREDTYPE2 StreamFunction =
{
#ifndef DX_NON_DXA
	DXA_DIR_Open,
	DXA_DIR_Close,
	DXA_DIR_Tell,
	DXA_DIR_Seek,
	DXA_DIR_Read,
	DXA_DIR_Eof,
	DXA_DIR_IdleCheck,
	DXA_DIR_ChDir,
	DXA_DIR_GetDir,
	DXA_DIR_FindFirst,
	DXA_DIR_FindNext,
	DXA_DIR_FindClose,
#else
	ReadOnlyFileAccessOpen,
	ReadOnlyFileAccessClose,
	ReadOnlyFileAccessTell,
	ReadOnlyFileAccessSeek,
	ReadOnlyFileAccessRead,
	ReadOnlyFileAccessEof,
	ReadOnlyFileAccessIdleCheck,
	ReadOnlyFileAccessChDir,
	ReadOnlyFileAccessGetDir,
	ReadOnlyFileAccessFindFirst,
	ReadOnlyFileAccessFindNext,
	ReadOnlyFileAccessFindClose,
#endif
/*
	_FileOpen,
	_FileClose,
	_FileTell,
	_FileSeek,
	_FileRead,
	_FileEof,
	_FileIdleCheck,
	_FileChDir,
	_FileGetDir,
*/
} ;

// ストリームデータ制御用関数ポインタのデフォルト配列

// ファイルのストリームデータ制御用ポインタ構造体
STREAMDATASHRED StreamFileFunction =
{
	( LONGLONG ( * )( void *StreamDataPoint ) )StreamTell,
	( int ( * )( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType ) )StreamSeek,
	( size_t ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )StreamRead,
	( int ( * )( void *StreamDataPoint ) )StreamEof,
	( int ( * )( void *StreamDataPoint ) )StreamIdleCheck,
	( int ( * )( void *StreamDataPoint ) )StreamClose,
} ;

// メモリのストリームデータ制御用ポインタ構造体
STREAMDATASHRED StreamMemFunction =
{
	MemStreamTell,
	MemStreamSeek,
	MemStreamRead,
	MemStreamEof,
	MemStreamIdleCheck,
	MemStreamClose,
} ;

// プログラム --------------------------------------------------------------------

// 文件访问处理的初始化、结束函数

// ファイルアクセス処理の初期化
extern int InitializeFile( void )
{
	// 既に初期化されていたら何もしない
	if( GFileData.InitializeFlag == TRUE )
		return -1 ;

	// ファイル読み込みハンドル管理情報を初期化する
	InitializeHandleManage( DX_HANDLETYPE_FILE, sizeof( FILEACCESSINFO ), MAX_FILE_NUM, InitializeFileHandle, TerminateFileHandle, DXSTRING( _T( "ファイル" ) ) ) ;

	// ストリームデータ読みこみ制御用ポインタ配列のデフォルト値をセット
	{
		StreamFileFunction.Read      = ( size_t   ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )StreamRead ;
		StreamFileFunction.Eof       = ( int      ( * )( void *StreamDataPoint ) )StreamEof ;
		StreamFileFunction.IdleCheck = ( int      ( * )( void *StreamDataPoint ) )StreamIdleCheck ;
		StreamFileFunction.Seek      = ( int      ( * )( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType ) )StreamSeek ;
		StreamFileFunction.Tell      = ( LONGLONG ( * )( void *StreamDataPoint ) )StreamTell ;
		StreamFileFunction.Close     = ( int      ( * )( void *StreamDataPoint ) )StreamClose ;
/*
		StreamFileFunction.Read  = ( size_t ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )fread ;
//		StreamFileFunction.Write = ( size_t ( * )( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint ) )fwrite ;
		StreamFileFunction.Eof   = ( int ( * )( void *StreamDataPoint ) )feof ;
		StreamFileFunction.Seek  = ( int ( * )( void *StreamDataPoint, long SeekPoint, int SeekType ) )fseek ;
		StreamFileFunction.Tell  = ( long ( * )( void *StreamDataPoint ) )ftell ;
		StreamFileFunction.Close = ( int ( * )( void *StreamDataPoint ) )fclose ;
*/

		StreamMemFunction.Read      = MemStreamRead ;
//		StreamMemFunction.Write     = MemStreamWrite;
		StreamMemFunction.Eof       = MemStreamEof ;
		StreamMemFunction.IdleCheck = MemStreamIdleCheck ;
		StreamMemFunction.Seek      = MemStreamSeek ;
		StreamMemFunction.Tell      = MemStreamTell ;
		StreamMemFunction.Close     = MemStreamClose ;
	}

	// 初期化フラグを立てる
	GFileData.InitializeFlag = TRUE ;

	// 正常終了
	return 0 ;
}

// ファイルアクセス処理の後始末
extern int TerminateFile( void )
{
	// 既に後始末されていたら何もしない
	if( GFileData.InitializeFlag == FALSE )
		return -1 ;

	// ファイルハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_FILE ) ;

	// 初期化フラグを倒す
	GFileData.InitializeFlag = FALSE ;

	// 正常終了
	return 0 ;
}

// アーカイブファイルの読み込み機能を使うかどうかを設定する( FALSE:使用しない  TRUE:使用する )
extern int NS_SetUseDXArchiveFlag( int Flag )
{
	int DefaultFlag ;

	// フラグの状態が同じ場合は何もしない
	if( NotUseDXArchive == !Flag ) return 0 ;

	// 標準の読み込み機能が使用されているかどうかを取得しておく
	DefaultFlag = NS_GetStreamFunctionDefault() ;

	// 更新Flag
	NotUseDXArchive = !Flag ;

	// 現在標準の読み込み機能が使われている場合は関数ポインタを更新する
	if( DefaultFlag == TRUE )
		NS_ChangeStreamFunction( NULL ) ;

	// 終了
	return 0 ;
}

// ＤＸライブラリでストリームデータアクセスに使用する関数を変更する
extern int NS_ChangeStreamFunction( const STREAMDATASHREDTYPE2 *StreamThread )
{
	// 関数ポインタを更新する
	if( StreamThread == NULL )
	{
		// アーカイブファイルを読み込む機能を使用するかどうかでセットする関数ポインタを変更
#ifndef DX_NON_DXA
		if( NotUseDXArchive == TRUE )
#endif
		{
			// 不使用的情况
			StreamFunction.Open			= ReadOnlyFileAccessOpen ;
			StreamFunction.Close		= ReadOnlyFileAccessClose ;
			StreamFunction.Tell			= ReadOnlyFileAccessTell ;
			StreamFunction.Seek			= ReadOnlyFileAccessSeek ;
			StreamFunction.Read			= ReadOnlyFileAccessRead ;
			StreamFunction.Eof 			= ReadOnlyFileAccessEof ;
			StreamFunction.IdleCheck	= ReadOnlyFileAccessIdleCheck ;
			StreamFunction.ChDir		= ReadOnlyFileAccessChDir ;
			StreamFunction.GetDir		= ReadOnlyFileAccessGetDir ;
			StreamFunction.FindFirst	= ReadOnlyFileAccessFindFirst ;
			StreamFunction.FindNext		= ReadOnlyFileAccessFindNext ;
			StreamFunction.FindClose	= ReadOnlyFileAccessFindClose ;
		}
#ifndef DX_NON_DXA
		else
		{
			// 使用的情况
			StreamFunction.Open			= DXA_DIR_Open ;
			StreamFunction.Close		= DXA_DIR_Close ;
			StreamFunction.Tell			= DXA_DIR_Tell ;
			StreamFunction.Seek			= DXA_DIR_Seek ;
			StreamFunction.Read			= DXA_DIR_Read ;
			StreamFunction.Eof 			= DXA_DIR_Eof ;
			StreamFunction.IdleCheck	= DXA_DIR_IdleCheck ;
			StreamFunction.ChDir		= DXA_DIR_ChDir ;
			StreamFunction.GetDir		= DXA_DIR_GetDir ;
			StreamFunction.FindFirst	= DXA_DIR_FindFirst ;
			StreamFunction.FindNext		= DXA_DIR_FindNext ;
			StreamFunction.FindClose	= DXA_DIR_FindClose ;
		}
#endif
	}
	else
	{
		StreamFunction = *StreamThread ;
	}
	
	// 終了
	return 0 ;
}

// ＤＸライブラリでストリームデータアクセスに使用する関数がデフォルトのものか調べる( TRUE:デフォルト  FALSE:デフォルトではない )
extern int NS_GetStreamFunctionDefault( void )
{
	// アーカイブファイルを読み込む機能を使用するかどうかで比較する関数を変更
#ifndef DX_NON_DXA
	if( NotUseDXArchive == TRUE )
#endif
	{
		// 不使用的情况
		if( StreamFunction.Open			!= ReadOnlyFileAccessOpen       ) return FALSE ;
		if( StreamFunction.Close		!= ReadOnlyFileAccessClose      ) return FALSE ;
		if( StreamFunction.Tell			!= ReadOnlyFileAccessTell       ) return FALSE ;
		if( StreamFunction.Seek			!= ReadOnlyFileAccessSeek       ) return FALSE ;
		if( StreamFunction.Read			!= ReadOnlyFileAccessRead       ) return FALSE ;
		if( StreamFunction.Eof 			!= ReadOnlyFileAccessEof        ) return FALSE ;
		if( StreamFunction.ChDir		!= ReadOnlyFileAccessChDir      ) return FALSE ;
		if( StreamFunction.GetDir		!= ReadOnlyFileAccessGetDir     ) return FALSE ;
		if( StreamFunction.FindFirst 	!= ReadOnlyFileAccessFindFirst  ) return FALSE ;
		if( StreamFunction.FindNext 	!= ReadOnlyFileAccessFindNext   ) return FALSE ;
		if( StreamFunction.FindClose 	!= ReadOnlyFileAccessFindClose  ) return FALSE ;
	}
#ifndef DX_NON_DXA
	else
	{
		// 使用的情况
		if( StreamFunction.Open			!= DXA_DIR_Open ) return FALSE ;
		if( StreamFunction.Close		!= DXA_DIR_Close ) return FALSE ;
		if( StreamFunction.Tell			!= DXA_DIR_Tell ) return FALSE ;
		if( StreamFunction.Seek			!= DXA_DIR_Seek ) return FALSE ;
		if( StreamFunction.Read			!= DXA_DIR_Read ) return FALSE ;
		if( StreamFunction.Eof 			!= DXA_DIR_Eof ) return FALSE ;
		if( StreamFunction.ChDir		!= DXA_DIR_ChDir ) return FALSE ;
		if( StreamFunction.GetDir 		!= DXA_DIR_GetDir ) return FALSE ;
		if( StreamFunction.FindFirst 	!= DXA_DIR_FindFirst ) return FALSE ;
		if( StreamFunction.FindNext 	!= DXA_DIR_FindNext  ) return FALSE ;
		if( StreamFunction.FindClose 	!= DXA_DIR_FindClose ) return FALSE ;
	}
#endif
	
	return TRUE ;
}











// 補助関係関数

// フルパスではないパス文字列をフルパスに変換する
// ( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )
// ( CurrentDir が NULL の場合は現在のカレントディレクトリを使用する )
extern int ConvertFullPath( const TCHAR *Src, TCHAR *Dest, const TCHAR *CurrentDir )
{
	return ConvertFullPathT_( Src, Dest, CurrentDir ) ;
}



















// ストリームデータアクセス関数
extern DWORD_PTR StreamOpen( const TCHAR *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	return StreamFunction.Open( Path, UseCacheFlag, BlockReadFlag, UseASyncReadFlag ) ;
}

extern int StreamClose( DWORD_PTR Handle )
{
	return StreamFunction.Close( Handle ) ;
}

extern LONGLONG StreamTell( DWORD_PTR Handle )
{
	return StreamFunction.Tell( Handle ) ;
}

extern int StreamSeek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType )
{
	return StreamFunction.Seek( Handle, SeekPoint, SeekType ) ;
}

extern size_t StreamRead( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle )
{
	return StreamFunction.Read( Buffer, BlockSize, DataNum, Handle ) ;
}

extern int StreamEof( DWORD_PTR Handle )
{
	return StreamFunction.Eof( Handle ) ;
}

extern int StreamIdleCheck( DWORD_PTR Handle )
{
	return StreamFunction.IdleCheck( Handle ) ;
}

extern int StreamChDir( const TCHAR *Path )
{
	return StreamFunction.ChDir( Path ) ;
}

extern int StreamGetDir( TCHAR *Buffer )
{
	return StreamFunction.GetDir( Buffer ) ;
}

extern DWORD_PTR StreamFindFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	return StreamFunction.FindFirst( FilePath, Buffer ) ;
}

extern int StreamFindNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	return StreamFunction.FindNext( FindHandle, Buffer ) ;
}

extern int StreamFindClose( DWORD_PTR FindHandle )
{
	return StreamFunction.FindClose( FindHandle ) ;
}

extern	const STREAMDATASHREDTYPE2 *StreamGetStruct( void )
{
	return &StreamFunction ;
}












// ストリームデータアクセス用関数構造体関係

// ストリームデータ読みこみ制御用関数ポインタ構造体のファイル用構造体を得る
extern STREAMDATASHRED *GetFileStreamDataShredStruct( void )
{
	return &StreamFileFunction ;
}

// ストリームデータ読みこみ制御用関数ポインタ構造体のﾒﾓﾘ用構造体を得る
extern STREAMDATASHRED *GetMemStreamDataShredStruct( void )
{
	return &StreamMemFunction ;
}













// ファイルハンドル関係

// 初始化文件访问句柄
static int InitializeFileHandle( HANDLEINFO * )
{
	// 不需要特别处理
	return 0 ;
}

// 释放文件访问句柄
static int TerminateFileHandle( HANDLEINFO *HandleInfo )
{
	FILEACCESSINFO *FileInfo = ( FILEACCESSINFO * )HandleInfo ;

	// ハンドルのタイプによって処理を分岐
	switch( FileInfo->HandleType )
	{
	case FILEHANDLETYPE_NORMAL :
		// ファイルを閉じる
		if( FileInfo->FilePointer )
		{
			FCLOSE( FileInfo->FilePointer ) ;
			FileInfo->FilePointer = 0 ;
		}
		break ;

	case FILEHANDLETYPE_FULLYLOAD :
		// ファイルの内容を格納していたメモリを解放
		if( FileInfo->FileImage != NULL )
		{
			DXFREE( FileInfo->FileImage ) ;
			FileInfo->FileImage = NULL ;
		}
		break ;
	}

	// 終了
	return 0 ;
}

// FileRead_open 的实际处理函数
static int FileRead_open_Static(
	int FileHandle,
	const TCHAR *FilePath,
	int ASync,
	int ASyncThread
)
{
	FILEACCESSINFO *FileInfo ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}
	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	// 打开文件
	if( ASync )
	{
		FileInfo->FilePointer = FOPEN_ASYNC( FilePath ) ;
	}
	else
	{
		FileInfo->FilePointer = FOPEN( FilePath ) ;
	}

	// 開けなかったらエラー
	if( FileInfo->FilePointer == 0 )
		return -1 ;

	// 正常終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_open の非同期読み込みスレッドから呼ばれる関数
static void FileRead_open_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int FileHandle ;
	const TCHAR *FilePath ;
	int ASync ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FilePath = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	ASync = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = FileRead_open_Static( FileHandle, FilePath, ASync, TRUE ) ;

	DecASyncLoadCount( FileHandle ) ;
	if( Result < 0 )
	{
		SubHandle( FileHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// 打开文件
extern int FileRead_open_UseGParam(
	const TCHAR *FilePath,
	int ASync,
	int ASyncLoadFlag
)
{
	int FileHandle ;
	FILEACCESSINFO *FileInfo ;

	// 初期化されていなかったら初期化する
	if( GFileData.InitializeFlag == FALSE )
	{
		InitializeFile() ;
	}

	// ハンドルの作成
	FileHandle = AddHandle( DX_HANDLETYPE_FILE ) ;
	if( FileHandle == -1 ) return -1 ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;
	FileInfo->HandleType = FILEHANDLETYPE_NORMAL ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FilePath, FullPath ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;
		AddASyncLoadParamInt( NULL, &Addr, ASync ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = FileRead_open_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ASync ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( FileHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( FileRead_open_Static( FileHandle, FilePath, ASync, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return FileHandle ;

ERR :
	SubHandle( FileHandle ) ;

	return 0 ;
}

// ファイルアクセス関数
extern int NS_FileRead_open( const TCHAR *FilePath , int ASync )
{
	return FileRead_open_UseGParam( FilePath, ASync, GetASyncLoadFlag() ) ;
}

extern LONGLONG NS_FileRead_size( const TCHAR *FilePath )
{
	LONGLONG Result ;
	DWORD_PTR fp ;
	fp = FOPEN( FilePath ) ;
	if( fp == 0 ) return -1 ;
	FSEEK( fp, 0L, SEEK_END ) ;
	Result = FTELL( fp ) ;
	FCLOSE( fp ) ;
	return Result ;
}

extern int NS_FileRead_close( int FileHandle )
{
	return SubHandle( FileHandle ) ;
}

extern LONGLONG NS_FileRead_tell( int FileHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	return FTELL( FileInfo->FilePointer ) ;
}

// FileRead_seek の実処理関数
static int FileRead_seek_Static( int FileHandle, LONGLONG Offset, int Origin, int ASyncThread )
{
	FILEACCESSINFO *FileInfo ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	return FSEEK( FileInfo->FilePointer, Offset, Origin ) ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_seek の非同期読み込みスレッドから呼ばれる関数
static void FileRead_seek_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	FILEACCESSINFO *FileInfo ;
	int FileHandle ;
	LONGLONG Offset ;
	int Origin ;
	int Addr ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Offset = GetASyncLoadParamLONGLONG( AParam->Data, &Addr ) ;
	Origin = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	if( !FILEHCHK_ASYNC( FileHandle, FileInfo ) )
	{
		FileInfo->HandleInfo.ASyncLoadResult = FileRead_seek_Static( FileHandle, Offset, Origin, TRUE ) ;
	}

	DecASyncLoadCount( FileHandle ) ;
}

#endif // DX_NON_ASYNCLOAD

// ファイルポインタの位置を変更する
extern int FileRead_seek_UseGParam( int FileHandle, LONGLONG Offset, int Origin, int ASyncLoadFlag )
{
	FILEACCESSINFO *FileInfo ;
	int Result = -1 ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamLONGLONG( NULL, &Addr, Offset ) ;
		AddASyncLoadParamInt( NULL, &Addr, Origin ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto END ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = FileRead_seek_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamLONGLONG( AParam->Data, &Addr, Offset ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Origin ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto END ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( FileHandle, AParam->Index ) ;

		Result = 0 ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		Result = FileRead_seek_Static( FileHandle, Offset, Origin, FALSE ) ;
	}

#ifndef DX_NON_ASYNCLOAD
END :
#endif

	// 終了
	return Result ;
}

extern int NS_FileRead_seek( int FileHandle, LONGLONG Offset, int Origin )
{
	return FileRead_seek_UseGParam( FileHandle, Offset, Origin, GetASyncLoadFlag() ) ;
}

// FileRead_read の実処理関数
static int FileRead_read_Static(
	int FileHandle,
	void *Buffer,
	int ReadSize,
	int ASyncThread
)
{
	FILEACCESSINFO *FileInfo ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	return ( int )FREAD( Buffer, 1, ReadSize, FileInfo->FilePointer ) ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_read の非同期読み込みスレッドから呼ばれる関数
static void FileRead_read_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int FileHandle ;
	void *Buffer ;
	int ReadSize ;
	int Addr ;
	FILEACCESSINFO *FileInfo ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Buffer = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	ReadSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	if( !FILEHCHK_ASYNC( FileHandle, FileInfo ) )
	{
		FileInfo->HandleInfo.ASyncLoadResult = FileRead_read_Static( FileHandle, Buffer, ReadSize, TRUE ) ;
	}

	DecASyncLoadCount( FileHandle ) ;
}

#endif // DX_NON_ASYNCLOAD


// ファイルからデータを読み込む
extern int FileRead_read_UseGParam(
	int FileHandle,
	void *Buffer,
	int ReadSize,
	int ASyncLoadFlag
)
{
	int Result = -1 ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, Buffer ) ;
		AddASyncLoadParamInt( NULL, &Addr, ReadSize ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto END ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = FileRead_read_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, Buffer ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ReadSize ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto END ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( FileHandle, AParam->Index ) ;

		Result = 0 ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		Result = FileRead_read_Static( FileHandle, Buffer, ReadSize, FALSE ) ;
	}

#ifndef DX_NON_ASYNCLOAD
END :
#endif

	// 終了
	return Result ;
}

extern int NS_FileRead_read( void *Buffer, int ReadSize, int FileHandle )
{
	return FileRead_read_UseGParam( FileHandle, Buffer, ReadSize, GetASyncLoadFlag() ) ;
}

extern int NS_FileRead_idle_chk( int FileHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( FileInfo->HandleInfo.ASyncLoadCount != 0 )
		return FALSE ;
#endif // DX_NON_ASYNCLOAD

	return FIDLECHK( FileInfo->FilePointer ) ;
}

// ファイルの終端かどうかを得る
extern int NS_FileRead_eof( int FileHandle )
{
	LONGLONG FileSize, NowPoint ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	NowPoint = FTELL( FileInfo->FilePointer                     ) ;
	           FSEEK( FileInfo->FilePointer,        0, SEEK_END ) ;
	FileSize = FTELL( FileInfo->FilePointer                     ) ;
	           FSEEK( FileInfo->FilePointer, NowPoint, SEEK_SET ) ;

	return NowPoint == FileSize ;
}

extern int NS_FileRead_gets( TCHAR *Buffer, int BufferSize, int FileHandle )
{
	LONGLONG i, ReadSize ;
	LONGLONG NowPos ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	if( FEOF( FileInfo->FilePointer ) != 0 )
	{
		return -1 ;
	}

	NowPos   = FTELL( FileInfo->FilePointer ) ;
	ReadSize = FREAD( Buffer, sizeof( TCHAR ), BufferSize - 1, FileInfo->FilePointer ) ;FSYNC( FileInfo->FilePointer )
	if( ReadSize == 0 )
	{
		return -1 ;
	}

#ifdef UNICODE
	if( *( ( WORD * )&Buffer[ 0 ] ) == 0xfeff )
	{
		FSEEK( FileInfo->FilePointer, 2, SEEK_SET ) ;
		NowPos   = FTELL( FileInfo->FilePointer ) ;
		ReadSize = FREAD( Buffer, sizeof( TCHAR ), BufferSize - 1, FileInfo->FilePointer ) ;FSYNC( FileInfo->FilePointer )
	}
#endif

	for( i = 0 ; i < ReadSize ; )
	{
		if( Buffer[i] == _T( '\0' ) ) break ;
		if( _TMULT( Buffer[i], _GET_CHARSET() ) == TRUE )
		{
			if( i + 2 > ReadSize )
			{
				FSEEK( FileInfo->FilePointer, NowPos + i * sizeof( TCHAR ), SEEK_SET ) ;
				break ;
			}
			i += 2 ;
		}
		else
		{
			if( Buffer[i] == _T( '\r' ) && Buffer[i+1] == _T( '\n' ) )
			{
				FSEEK( FileInfo->FilePointer, NowPos + ( i + 2 ) * sizeof( TCHAR ), SEEK_SET ) ;
				break ;
			}
			i ++ ;
		}
	}
	Buffer[i] = _T( '\0' ) ;

	return ( int )i ;
}

// ファイルから一文字読み出す
extern TCHAR NS_FileRead_getc( int FileHandle )
{
	size_t ReadSize ;
	TCHAR c ;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return _T( '\0' ) ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	if( FEOF( FileInfo->FilePointer ) != 0 )
	{
		return ( TCHAR )EOF ;
	}

	c = 0 ;
	ReadSize = FREAD( &c, sizeof( TCHAR ), 1, FileInfo->FilePointer ) ;
	FSYNC( FileInfo->FilePointer )
	if( ReadSize == 0 )
	{
		return ( TCHAR )EOF ;
	}

	return c ;
}

// ファイルから書式化されたデータを読み出す
extern int NS_FileRead_scanf( int FileHandle, const TCHAR *Format, ... )
{
	va_list param;
	int Result;
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_NORMAL )
		return -1 ;

	va_start( param, Format );
#ifdef UNICODE
	Result = FileRead_scanf_baseW( FileInfo->FilePointer, _DXWTP( Format ), param ) ;
#else
	Result = FileRead_scanf_base( FileInfo->FilePointer, Format, param ) ;
#endif
	va_end( param );

	return Result;
}

// ファイルの情報を取得する(戻り値  -1:エラー  -1以外:ファイル情報ハンドル  )
extern DWORD_PTR NS_FileRead_createInfo( const TCHAR *ObjectPath )
{
	int Num, i ;
	DWORD_PTR FindHandle;
	FILEINFO *FileInfo;
	FILEINFOLIST *FileInfoList;

	FindHandle = FFINDFIRST( ObjectPath, NULL );
	if( FindHandle == ( DWORD_PTR )-1 ) return ( DWORD_PTR )-1;

	Num = 0;
	do
	{
		Num ++ ;
	}while( FFINDNEXT( FindHandle, NULL ) == 0 );

	FFINDCLOSE( FindHandle );

	FileInfoList = (FILEINFOLIST *)DXALLOC( sizeof( FILEINFOLIST ) + sizeof( FILEINFO ) * Num );
	if( FileInfoList == NULL ) return ( DWORD_PTR )-1;

	FileInfoList->Num = Num;
	FileInfo = (FILEINFO *)( FileInfoList + 1 );
	FindHandle = FFINDFIRST( ObjectPath, FileInfo );
	FileInfo ++ ;
	for( i = 1; i < Num; i ++, FileInfo ++ )
		FFINDNEXT( FindHandle, FileInfo );

	FFINDCLOSE( FindHandle );

	return (DWORD_PTR)FileInfoList;
}

// ファイル情報ハンドル中のファイルの数を取得する
extern int NS_FileRead_getInfoNum( DWORD_PTR FileInfoHandle )
{
	FILEINFOLIST *FileInfoList;

	FileInfoList = (FILEINFOLIST *)FileInfoHandle;

	return FileInfoList->Num;
}

// ファイル情報ハンドル中のファイルの情報を取得する
extern int NS_FileRead_getInfo( int Index, FILEINFO *Buffer, DWORD_PTR FileInfoHandle )
{
	FILEINFOLIST *FileInfoList;

	FileInfoList = (FILEINFOLIST *)FileInfoHandle;
	if( Index < 0 || FileInfoList->Num <= Index ) return -1;

	*Buffer = ((FILEINFO *)( FileInfoList + 1 ))[Index];

	return 0;
}

// ファイル情報ハンドルを削除する
extern int NS_FileRead_deleteInfo( DWORD_PTR FileInfoHandle )
{
	DXFREE( (FILEINFOLIST *)FileInfoHandle );

	return 0;
}

// 指定のファイル又はフォルダの情報を取得し、ファイル検索ハンドルも作成する( 戻り値: -1=エラー  -1以外=ファイル検索ハンドル )
extern DWORD_PTR NS_FileRead_findFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	return FFINDFIRST( FilePath, Buffer );
}

// 条件の合致する次のファイルの情報を取得する( 戻り値: -1=エラー  0=成功 )
extern int NS_FileRead_findNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	return FFINDNEXT( FindHandle, Buffer );
}

// ファイル検索ハンドルを閉じる( 戻り値: -1=エラー  0=成功 )
extern int NS_FileRead_findClose( DWORD_PTR FindHandle )
{
	return FFINDCLOSE( FindHandle );
}








// FileRead_fullyLoad の実処理関数
static int FileRead_fullyLoad_Static(
	int FileHandle,
	const TCHAR *FilePath,
	int ASyncThread
)
{
	FILEACCESSINFO *FileInfo ;
	DWORD_PTR FilePointer ;

	if( ASyncThread )
	{
		if( FILEHCHK_ASYNC( FileHandle, FileInfo ) )
			return -1 ;
	}
	else
	{
		if( FILEHCHK( FileHandle, FileInfo ) )
			return -1 ;
	}
	if( FileInfo->HandleType != FILEHANDLETYPE_FULLYLOAD )
		return -1 ;

	// 打开文件
	FilePointer = FOPEN( FilePath ) ;

	// 開けなかったらエラー
	if( FilePointer == 0 )
		return -1 ;

	// ファイルのサイズを丸ごと読み込む
	FSEEK( FilePointer, 0, SEEK_END ) ;
	FileInfo->FileSize = FTELL( FilePointer ) ;
	FSEEK( FilePointer, 0, SEEK_SET ) ;
	FileInfo->FileImage = DXALLOC( ( size_t )FileInfo->FileSize ) ;
	if( FileInfo->FileImage == NULL )
	{
		FCLOSE( FilePointer ) ;
		FilePointer = 0 ;
		return -1 ;
	}
	FREAD( FileInfo->FileImage, FileInfo->FileSize, 1, FilePointer ) ;

	// ファイルを閉じる
	FCLOSE( FilePointer ) ;
	FilePointer = 0 ;

	// 正常終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// FileRead_fullyLoad の非同期読み込みスレッドから呼ばれる関数
static void FileRead_fullyLoad_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int FileHandle ;
	const TCHAR *FilePath ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	FileHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FilePath = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = FileRead_fullyLoad_Static( FileHandle, FilePath, TRUE ) ;

	DecASyncLoadCount( FileHandle ) ;
	if( Result < 0 )
	{
		SubHandle( FileHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// 指定のファイルの内容を全てメモリに読み込み、その情報のアクセスに必要なハンドルを返す( 戻り値  -1:エラー  -1以外:ハンドル )、使い終わったらハンドルは FileRead_fullyLoad_delete で削除する必要があります
extern	int FileRead_fullyLoad_UseGParam( const TCHAR *FilePath, int ASyncLoadFlag )
{
	int FileHandle ;
	FILEACCESSINFO *FileInfo ;

	// ハンドルの作成
	FileHandle = AddHandle( DX_HANDLETYPE_FILE ) ;
	if( FileHandle == -1 ) return -1 ;

	if( FILEHCHK( FileHandle, FileInfo ) )
		return -1 ;
	FileInfo->HandleType = FILEHANDLETYPE_FULLYLOAD ;

	// その他データを初期化
	FileInfo->FileImage = NULL ;
	FileInfo->FileSize = -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FilePath, FullPath ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, FileHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = FileRead_fullyLoad_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( FileHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( FileRead_fullyLoad_Static( FileHandle, FilePath, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return FileHandle ;

ERR :
	SubHandle( FileHandle ) ;

	return -1 ;
}

extern	int NS_FileRead_fullyLoad( const TCHAR *FilePath )
{
	return FileRead_fullyLoad_UseGParam( FilePath, GetASyncLoadFlag() ) ;
}

// FileRead_fullyLoad で読み込んだファイルをメモリから削除する
extern	int NS_FileRead_fullyLoad_delete( int FLoadHandle )
{
	return SubHandle( FLoadHandle ) ;
}

// FileRead_fullyLoad で読み込んだファイルの内容を格納したメモリアドレスを取得する
extern	const void* NS_FileRead_fullyLoad_getImage( int FLoadHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FLoadHandle, FileInfo ) )
		return NULL ;

	if( FileInfo->HandleType != FILEHANDLETYPE_FULLYLOAD )
		return NULL ;

	// ファイルを格納しているメモリアドレスを返す
	return FileInfo->FileImage ;
}

// FileRead_fullyLoad で読み込んだファイルのサイズを取得する
extern	LONGLONG NS_FileRead_fullyLoad_getSize(	int FLoadHandle )
{
	FILEACCESSINFO *FileInfo ;

	if( FILEHCHK( FLoadHandle, FileInfo ) )
		return -1 ;

	if( FileInfo->HandleType != FILEHANDLETYPE_FULLYLOAD )
		return -1 ;

	// ファイルのサイズを返す
	return FileInfo->FileSize ;
}







































// メモリに置かれたデータをファイルのデータに例えてつかうための関数
extern void *MemStreamOpen( const void *DataBuffer, unsigned int DataSize )
{
	MEMSTREAMDATA *NewMemData ;

	// メモリの確保
	if( ( NewMemData = ( MEMSTREAMDATA * )DXALLOC( sizeof( MEMSTREAMDATA ) ) ) == NULL ) return NULL ;
	_MEMSET( NewMemData, 0, sizeof( MEMSTREAMDATA) ) ;

	// データのセット
	NewMemData->DataBuffer = ( unsigned char * )DataBuffer ;
	NewMemData->DataPoint = 0 ;
	NewMemData->DataSize = DataSize ;
	NewMemData->EOFFlag = FALSE ;

	// 終了
	return NewMemData ;
}


extern int MemStreamClose( void *StreamDataPoint )
{
	// メモリの解放
	DXFREE( StreamDataPoint ) ;

	// 終了
	return 0 ;
}

static LONGLONG MemStreamTell( void *StreamDataPoint )
{
	// データポインタを返す
	return ( LONGLONG )( ( MEMSTREAMDATA * )StreamDataPoint )->DataPoint ;
}

static int MemStreamSeek( void *StreamDataPoint, LONGLONG SeekPoint, int SeekType )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;
	LONGLONG NewPoint = 0 ;

	// シークタイプによって処理を分岐
	switch( SeekType )
	{
	case STREAM_SEEKTYPE_SET :
		NewPoint = SeekPoint ;
		break ;

	case STREAM_SEEKTYPE_END :
		NewPoint = ( LONGLONG )( SeekPoint + MemData->DataSize ) ;
		break ;

	case STREAM_SEEKTYPE_CUR :
		NewPoint = ( LONGLONG )( SeekPoint + MemData->DataPoint ) ;
		break ;
	}

	// 補正
	if( NewPoint > ( LONGLONG )MemData->DataSize )
	{
		MemData->DataPoint = MemData->DataSize ;
	}
	else
	{
		if( NewPoint < 0 )
		{
			MemData->DataPoint = 0 ;
		}
		else
		{
			MemData->DataPoint = ( size_t )NewPoint ;
		}
	}

	MemData->EOFFlag = FALSE ;

	// 終了
	return 0 ;
} ;

static size_t MemStreamRead( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;

	// EOF の検出
	if( MemData->DataPoint == ( int )MemData->DataSize )
	{
		MemData->EOFFlag = TRUE ;
		return 0 ;
	}
	else
	{
		// 読みこむデータの数の補正
		if( BlockSize * DataNum > ( int )( MemData->DataSize - MemData->DataPoint ) )
		{
			DataNum = ( ( MemData->DataSize - MemData->DataPoint ) / BlockSize ) * BlockSize ;
		}

		if( DataNum != 0 )
		{
			_MEMCPY( Buffer, MemData->DataBuffer + MemData->DataPoint, DataNum * BlockSize ) ;
			MemData->DataPoint += BlockSize * DataNum ;
		}
		else
		{
			size_t MoveSize ;

			MoveSize = MemData->DataSize - MemData->DataPoint ;
			if( MoveSize != 0 )
			{
				_MEMCPY( Buffer, MemData->DataBuffer + MemData->DataPoint, MoveSize ) ;
				MemData->DataPoint += MoveSize ;
				DataNum = 1 ;
			}
		}

		MemData->EOFFlag = FALSE ;
	}

	return DataNum ;
}
	
static size_t MemStreamWrite( void *Buffer, size_t BlockSize, size_t DataNum, void *StreamDataPoint )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;

	// EOF の検出
	if( MemData->DataPoint == ( int )MemData->DataSize )
	{
		MemData->EOFFlag = TRUE ;
		return 0 ;
	}
	else
	{
		// 書きこむデータの数の補正
		if( BlockSize * DataNum > ( int )( MemData->DataSize - MemData->DataPoint ) )
		{
			DataNum = ( ( MemData->DataSize - MemData->DataPoint ) / BlockSize ) * BlockSize ;
		}

		if( DataNum != 0 )
		{
			_MEMCPY( MemData->DataBuffer + MemData->DataPoint, Buffer, DataNum * BlockSize ) ;
			MemData->DataPoint += BlockSize * DataNum ;
		}

		MemData->EOFFlag = FALSE ;
	}

	return DataNum ;
}

static int MemStreamEof( void *StreamDataPoint )
{
	MEMSTREAMDATA *MemData = ( MEMSTREAMDATA * )StreamDataPoint ;

	// EOF を返す
	return MemData->EOFFlag ;
}

static int MemStreamIdleCheck( void * /*StreamDataPoint*/ )
{
	// 常にアイドリング状態
	return TRUE ;
}























// フルパスではないパス文字列をフルパスに変換する
// ( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )
// ( CurrentDir が NULL の場合は現在のカレントディレクトリを使用する )
extern int ConvertFullPathT_( const TCHAR *Src, TCHAR *Dest, const TCHAR *CurrentDir )
{
#ifdef UNICODE
	return ConvertFullPathW_( Src, Dest, CurrentDir ) ;
#else
	return ConvertFullPath_( Src, Dest, CurrentDir ) ;
#endif
}

// フルパスではないパス文字列をフルパスに変換する
// ( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )
// ( CurrentDir が NULL の場合は現在のカレントディレクトリを使用する )
extern	int ConvertFullPath_( const char *Src, char *Dest, const char *CurrentDir )
{
	int i, j, k ;
	char iden[256], cur[MAX_PATH] ;

	if( CurrentDir == NULL )
	{
#ifdef UNICODE
		wchar_t curW[ MAX_PATH ];

		FGETDIR( curW ) ;
		WCharToMBChar( _GET_CODEPAGE(), ( DXWCHAR * )curW, cur, MAX_PATH ) ;
#else
		FGETDIR( cur ) ;
#endif
		CurrentDir = cur ;
	}

	if( Src == NULL )
	{
		_STRCPY( Dest, CurrentDir ) ;
		goto END ;
	}

	i = 0 ;
	j = 0 ;
	k = 0 ;
	
	// 最初に『\』又は『/』が２回連続で続いている場合はネットワークを介していると判断
	if( ( Src[0] == '\\' && Src[1] == '\\' ) ||
		( Src[0] == '/'  && Src[1] == '/'  ) )
	{
		Dest[0] = '\\';
		Dest[1] = '\0';

		i += 2;
		j ++ ;
	}
	else
	// 最初が『\』又は『/』の場合はカレントドライブのルートディレクトリまで落ちる
	if( Src[0] == '\\' || Src[0] == '/' )
	{
		Dest[0] = CurrentDir[0] ;
		Dest[1] = CurrentDir[1] ;
		Dest[2] = '\0' ;

		i ++ ;
		j = 2 ;
	}
	else
	// ドライブ名が書かれていたらそのドライブへ
	if( Src[1] == ':' )
	{
		Dest[0] = Src[0] ;
		Dest[1] = Src[1] ;
		Dest[2] = '\0' ;

		i = 2 ;
		j = 2 ;

		if( Src[i] == '\\' || Src[i] == '/' ) i ++ ;
	}
	else
	// それ以外の場合はカレントディレクトリ
	{
		_STRCPY( Dest, CurrentDir ) ;
		j = _STRLEN( Dest ) ;
		if( Dest[j-1] == '\\' || Dest[j-1] == '/' )
		{
			Dest[j-1] = '\0' ;
			j -- ;
		}
	}

	for(;;)
	{
		switch( Src[i] )
		{
		case '\0' :
			if( k != 0 )
			{
				Dest[j] = '\\' ; j ++ ;
				_STRCPY( &Dest[j], iden ) ;
				j += k ;
				k = 0 ;
			}
			goto END ;

		case '\\' :
		case '/' :
			// 文字列が無かったらスキップ
			if( k == 0 )
			{
				i ++ ;
				break ;
			}
			if( _STRCMP( iden, "." ) == 0 )
			{
				// なにもしない
			}
			else
			if( _STRCMP( iden, ".." ) == 0 )
			{
				// 一つ下のディレクトリへ
				j -- ;
				for(;;)
				{
					if( Dest[j] == '\\' || Dest[j] == '/' || Dest[j] == ':' ) break ;
					j -= CheckMultiByteString( Dest, j - 1, _GET_CHARSET() ) == 2 ? 2 : 1 ;
				}
//				while( Dest[j] != '\\' && Dest[j] != '/' && Dest[j] != ':' ) j -- ;
				if( Dest[j] != ':' ) Dest[j] = '\0' ;
				else j ++ ;
			}
			else
			{
				Dest[j] = '\\' ; j ++ ;
				_STRCPY( &Dest[j], iden ) ;
				j += k ;
			}

			k = 0 ;
			i ++ ;
			break ;
		
		default :
			if( CheckMultiByteChar( Src[i], _GET_CHARSET() ) == FALSE )
			{
				iden[k] = Src[i] ;
				iden[k+1] = 0 ; 
				k ++ ;
				i ++ ;
			}
			else
			{
				*(( unsigned short * )&iden[k] ) = *(( unsigned short * )&Src[i] ) ;
				iden[k+2] = '\0' ;
				k += 2 ;
				i += 2 ;
			}
			break ;
		}
	}
	
END :
	// 正常終了
	return 0 ;
}

// フルパスではないパス文字列をフルパスに変換する
// ( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )
// ( CurrentDir が NULL の場合は現在のカレントディレクトリを使用する )
extern	int ConvertFullPathW_( const wchar_t *Src, wchar_t *Dest, const wchar_t *CurrentDir )
{
	int i, j, k ;
	wchar_t iden[256], cur[MAX_PATH] ;

	if( CurrentDir == NULL )
	{
#ifdef UNICODE
		FGETDIR( cur ) ;
#else
		char curA[ MAX_PATH ] ;
		FGETDIR( curA ) ;
		MBCharToWChar( _GET_CODEPAGE(), curA, ( DXWCHAR * )cur, MAX_PATH ) ;
#endif
		CurrentDir = cur ;
	}

	if( Src == NULL )
	{
		_WCSCPY( Dest, CurrentDir ) ;
		goto END ;
	}

	i = 0 ;
	j = 0 ;
	k = 0 ;
	
	// 最初に『\』又は『/』が２回連続で続いている場合はネットワークを介していると判断
	if( ( Src[0] == L'\\' && Src[1] == L'\\' ) ||
		( Src[0] == L'/'  && Src[1] == L'/'  ) )
	{
		Dest[0] = L'\\';
		Dest[1] = L'\0';

		i += 2;
		j ++ ;
	}
	else
	// 最初が『\』又は『/』の場合はカレントドライブのルートディレクトリまで落ちる
	if( Src[0] == L'\\' || Src[0] == L'/' )
	{
		Dest[0] = CurrentDir[0] ;
		Dest[1] = CurrentDir[1] ;
		Dest[2] = L'\0' ;

		i ++ ;
		j = 2 ;
	}
	else
	// ドライブ名が書かれていたらそのドライブへ
	if( Src[1] == L':' )
	{
		Dest[0] = Src[0] ;
		Dest[1] = Src[1] ;
		Dest[2] = L'\0' ;

		i = 2 ;
		j = 2 ;

		if( Src[i] == L'\\' || Src[i] == L'/' ) i ++ ;
	}
	else
	// それ以外の場合はカレントディレクトリ
	{
		_WCSCPY( Dest, CurrentDir ) ;
		j = _WCSLEN( Dest ) ;
		if( Dest[j-1] == L'\\' || Dest[j-1] == L'/' )
		{
			Dest[j-1] = L'\0' ;
			j -- ;
		}
	}

	for(;;)
	{
		switch( Src[i] )
		{
		case L'\0' :
			if( k != 0 )
			{
				Dest[j] = L'\\' ; j ++ ;
				_WCSCPY( &Dest[j], iden ) ;
				j += k ;
				k = 0 ;
			}
			goto END ;

		case L'\\' :
		case L'/' :
			// 文字列が無かったらスキップ
			if( k == 0 )
			{
				i ++ ;
				break ;
			}
			if( _WCSCMP( iden, L"." ) == 0 )
			{
				// なにもしない
			}
			else
			if( _WCSCMP( iden, L".." ) == 0 )
			{
				// 一つ下のディレクトリへ
				j -- ;
//				while( Dest[j] != L'\\' && Dest[j] != L'/' && Dest[j] != L':' ) j -- ;
				for(;;)
				{
					if( Dest[j] == L'\\' || Dest[j] == L'/' || Dest[j] == L':' ) break ;
					j -= CheckUTF16HChar( Dest, j - 1 ) == 2 ? 2 : 1 ;
				}
				if( Dest[j] != L':' ) Dest[j] = L'\0' ;
				else j ++ ;
			}
			else
			{
				Dest[j] = L'\\' ; j ++ ;
				_WCSCPY( &Dest[j], iden ) ;
				j += k ;
			}

			k = 0 ;
			i ++ ;
			break ;
		
		default :
			if( CheckUTF16H( Src[i] ) == FALSE )
			{
				iden[k] = Src[i] ;
				iden[k+1] = 0 ; 
				k ++ ;
				i ++ ;
			}
			else
			{
				iden[k]   = Src[i] ;
				iden[k+1] = Src[i+1] ;
				iden[k+2] = '\0' ;
				k += 2 ;
				i += 2 ;
			}
			break ;
		}
	}
	
END :
	// 正常終了
	return 0 ;
}

// 指定のファイルパスを指定のフォルダパスから相対アクセスするための相対パスを作成する
// ( FilePath や StartFolderPath がフルパスではなかった場合は関数内でフルパス化されます )
// StartFolderPath の末端に / or \ があっても問題ありません
extern int CreateRelativePath_( const char *FilePath, const char *StartFolderPath, char *Dest )
{
	char filefull[ 512 ], filename[ 256 ], endfull[ 512 ], startfull[ 512 ] ;
	char endstr[ 256 ], startstr[ 256 ] ;
	int endlen, startlen, destlen ;
	char *ep, *sp ;

	// フルパス化
	ConvertFullPath_( FilePath, filefull ) ;
	AnalysisFileNameAndDirPath_( filefull, filename, endfull ) ;
	ConvertFullPath_( StartFolderPath, startfull ) ;
	startlen = _STRLEN( startfull ) ;
	if( startfull[ startlen - 1 ] == '\\' || startfull[ startlen - 1 ] == '/' )
		startfull[ startlen - 1 ] = '\0' ;

	// ドライブ文字列を取得する
	endlen = AnalysisDriveName_( endfull, endstr ) ;
	startlen = AnalysisDriveName_( startfull, startstr ) ;

	// ドライブが違ったらファイルパスのフルパスを代入する
	if( _STRCMP( endstr, startstr ) != 0 )
	{
		_STRCPY( Dest, filefull ) ;
		return 0 ;
	}
	ep = &endfull[ endlen ] ;
	sp = &startfull[ startlen ] ;

	// フォルダ名が違うところまで読み進める
	for(;;)
	{
		// \ or / を飛ばす
		while( *ep == '\\' || *ep == '/' ) ep ++ ;
		while( *sp == '\\' || *sp == '/' ) sp ++ ;

		// 終端文字だったらループを抜ける
		if( *ep == '\0' || *sp == '\0' ) break ;

		// フォルダパスを取得する
		endlen = AnalysisDirectoryName_( ep, endstr ) ;
		startlen = AnalysisDirectoryName_( sp, startstr ) ;

		// フォルダパスが違ったらループを抜ける
		if( _STRCMP( endstr, startstr ) != 0 ) break ;
		ep += endlen ;
		sp += startlen ;
	}

	// お互い同じフォルダだった場合はファイル名を出力
	if( *ep == '\0' && *sp == '\0' )
	{
		_STRCPY( Dest, filename ) ;
		return 0 ;
	}

	// 開始フォルダと違うフォルダにある場合はまず開始フォルダが現在のフォルダまで降りるパスの作成
	destlen = 0 ;
	if( *sp != '\0' )
	{
		Dest[ 0 ] = '\0' ;
		destlen = 0 ;
		for(;;)
		{
			startlen = AnalysisDirectoryName_( sp, startstr ) ;
			sp += startlen ;
			while( *sp == '\\' || *sp == '/' ) sp ++ ;
			_STRCPY( Dest + destlen, "..\\" ) ;
			destlen += 3 ;
			if( *sp == '\0' ) break ;
		}
	}

	// 目的のファイルがあるフォルダまでとファイル名をつなげて完成
	_SPRINTF( Dest + destlen, "%s\\%s", ep, filename ) ;
	return 0 ;
}


// 指定のファイルパスを指定のフォルダパスから相対アクセスするための相対パスを作成する
// ( FilePath や StartFolderPath がフルパスではなかった場合は関数内でフルパス化されます )
// StartFolderPath の末端に / or \ があっても問題ありません
extern int CreateRelativePathW_( const wchar_t *FilePath, const wchar_t *StartFolderPath, wchar_t *Dest )
{
	wchar_t filefull[ 512 ], filename[ 256 ], endfull[ 512 ], startfull[ 512 ] ;
	wchar_t endstr[ 256 ], startstr[ 256 ] ;
	int endlen, startlen, destlen ;
	wchar_t *ep, *sp ;

	// フルパス化
	ConvertFullPathW_( FilePath, filefull ) ;
	AnalysisFileNameAndDirPathW_( filefull, filename, endfull ) ;
	ConvertFullPathW_( StartFolderPath, startfull ) ;
	startlen = _WCSLEN( startfull ) ;
	if( startfull[ startlen - 1 ] == L'\\' || startfull[ startlen - 1 ] == L'/' )
		startfull[ startlen - 1 ] = L'\0' ;

	// ドライブ文字列を取得する
	endlen = AnalysisDriveNameW_( endfull, endstr ) ;
	startlen = AnalysisDriveNameW_( startfull, startstr ) ;

	// ドライブが違ったらファイルパスのフルパスを代入する
	if( _WCSCMP( endstr, startstr ) != 0 )
	{
		_WCSCPY( Dest, filefull ) ;
		return 0 ;
	}
	ep = &endfull[ endlen ] ;
	sp = &startfull[ startlen ] ;

	// フォルダ名が違うところまで読み進める
	for(;;)
	{
		// \ or / を飛ばす
		while( *ep == L'\\' || *ep == L'/' ) ep ++ ;
		while( *sp == L'\\' || *sp == L'/' ) sp ++ ;

		// 終端文字だったらループを抜ける
		if( *ep == L'\0' || *sp == L'\0' ) break ;

		// フォルダパスを取得する
		endlen = AnalysisDirectoryNameW_( ep, endstr ) ;
		startlen = AnalysisDirectoryNameW_( sp, startstr ) ;

		// フォルダパスが違ったらループを抜ける
		if( _WCSCMP( endstr, startstr ) != 0 ) break ;
		ep += endlen ;
		sp += startlen ;
	}

	// お互い同じフォルダだった場合はファイル名を出力
	if( *ep == L'\0' && *sp == L'\0' )
	{
		_WCSCPY( Dest, filename ) ;
		return 0 ;
	}

	// 開始フォルダと違うフォルダにある場合はまず開始フォルダが現在のフォルダまで降りるパスの作成
	destlen = 0 ;
	if( *sp != L'\0' )
	{
		Dest[ 0 ] = L'\0' ;
		destlen = 0 ;
		for(;;)
		{
			startlen = AnalysisDirectoryNameW_( sp, startstr ) ;
			sp += startlen ;
			while( *sp == L'\\' || *sp == L'/' ) sp ++ ;
			_WCSCPY( Dest + destlen, L"..\\" ) ;
			destlen += 3 ;
			if( *sp == L'\0' ) break ;
		}
	}

	// 目的のファイルがあるフォルダまでとファイル名をつなげて完成
	_SWPRINTF( ( WORD * )( Dest + destlen ), ( WORD * )( L"%s\\%s" ), ep, filename ) ;
	return 0 ;
}

// 特定のパス文字列から色々な情報を取得する
// ( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )
// ( CurrentDir が 0 の場合は実際のカレントディレクトリを使用する )
extern int AnalyseFilePath_(
	const char *Src,
	char *FullPath, char *DirPath, char *FileName, char *Name, char *ExeName, const char *CurrentDir )
{
	char full[256], dir[256], fname[256], name[256], exename[256];

	ConvertFullPath_( Src, full, CurrentDir );
	AnalysisFileNameAndDirPath_( full, fname, dir );
	AnalysisFileNameAndExeName_( fname, name, exename );
	if( FullPath ) _STRCPY( FullPath, full    );
	if( DirPath  ) _STRCPY( DirPath,  dir     );
	if( FileName ) _STRCPY( FileName, fname   );
	if( Name     ) _STRCPY( Name,     name    );
	if( ExeName  ) _STRCPY( ExeName,  exename );

	return 0;
}


// ファイル名も一緒になっていると分かっているパス中からファイル名とディレクトリパスを分割する
// フルパスである必要は無い、ファイル名だけでも良い
extern int AnalysisFileNameAndDirPath_( const char *Src, char *FileName, char *DirPath )
{
	int i, Last ;
	
	// ファイル名を抜き出す
	i = 0 ;
	Last = -1 ;
	while( Src[i] != '\0' )
	{
		if( CheckMultiByteChar( Src[i], _GET_CODEPAGE() ) == FALSE )
		{
			if( Src[i] == '\\' || Src[i] == '/' || Src[i] == '\0' || Src[i] == ':' ) Last = i ;
			i ++ ;
		}
		else
		{
			i += 2 ;
		}
	}
	if( FileName != NULL )
	{
		if( Last != -1 ) _STRCPY( FileName, &Src[Last+1] ) ;
		else _STRCPY( FileName, Src ) ;
	}
	
	// ディレクトリパスを抜き出す
	if( DirPath != NULL )
	{
		if( Last != -1 )
		{
			_STRNCPY( DirPath, Src, Last ) ;
			DirPath[Last] = '\0' ;
		}
		else
		{
			DirPath[0] = '\0' ;
		}
	}
	
	// 終了
	return 0 ;
}

// ファイル名も一緒になっていると分かっているパス中からファイル名とディレクトリパスを分割する
// フルパスである必要は無い、ファイル名だけでも良い
extern int AnalysisFileNameAndDirPathW_( const wchar_t *Src, wchar_t *FileName, wchar_t *DirPath )
{
	int i, Last ;
	
	// ファイル名を抜き出す
	i = 0 ;
	Last = -1 ;
	while( Src[i] != L'\0' )
	{
		if( CheckUTF16H( Src[i] ) == FALSE )
		{
			if( Src[i] == L'\\' || Src[i] == L'/' || Src[i] == L'\0' || Src[i] == L':' ) Last = i ;
			i ++ ;
		}
		else
		{
			i += 2 ;
		}
	}
	if( FileName != NULL )
	{
		if( Last != -1 ) _WCSCPY( FileName, &Src[Last+1] ) ;
		else             _WCSCPY( FileName, Src ) ;
	}
	
	// ディレクトリパスを抜き出す
	if( DirPath != NULL )
	{
		if( Last != -1 )
		{
			_WCSNCPY( DirPath, Src, Last ) ;
			DirPath[ Last ] = L'\0' ;
		}
		else
		{
			DirPath[0] = L'\0' ;
		}
	}
	
	// 終了
	return 0 ;
}

// ファイルパスからファイル名と拡張子を取得する
extern int AnalysisFileNameAndExeName_( const char *Src, char *Name, char *ExeName )
{
	char FileName[256], *p, *p2, ename[128], name[128] ;

	// ファイル名のみを取得
	AnalysisFileNameAndDirPath_( Src, FileName, 0 ) ;

	// 『.』があるかどうかで処理を分岐
	if( ( p = _STRCHR( FileName, '.' ) ) == NULL )
	{
		_STRCPY( name, FileName ) ;
		ename[0] = '\0' ;
	}
	else
	{
		for(;;)
		{
			p2 = _STRCHR( p + 1, '.' ) ;
			if( p2 == NULL ) break ;
			p = p2 ;
		}

		_STRNCPY( name, FileName, ( int )( p - FileName ) ) ;
		name[p - FileName] = '\0' ;
		_STRCPY( ename, p + 1 ) ;
	}

	if( Name != NULL ) _STRCPY( Name, name ) ;
	if( ExeName != NULL ) _STRCPY( ExeName, ename ) ;

	// 終了
	return 0 ;
}

// ファイルパスからファイル名と拡張子を取得する
extern int AnalysisFileNameAndExeNameW_( const wchar_t *Src, wchar_t *Name, wchar_t *ExeName )
{
	wchar_t FileName[256], *p, *p2, ename[128], name[128] ;

	// ファイル名のみを取得
	AnalysisFileNameAndDirPathW_( Src, FileName, 0 ) ;

	// 『.』があるかどうかで処理を分岐
	if( ( p = _WCSCHR( FileName, L'.' ) ) == NULL )
	{
		_WCSCPY( name, FileName ) ;
		ename[0] = L'\0' ;
	}
	else
	{
		for(;;)
		{
			p2 = _WCSCHR( p + 1, L'.' ) ;
			if( p2 == NULL ) break ;
			p = p2 ;
		}

		_WCSNCPY( name, FileName, ( int )( p - FileName ) ) ;
		name[p - FileName] = L'\0' ;
		_WCSCPY( ename, p + 1 ) ;
	}

	if( Name    != NULL ) _WCSCPY( Name, name ) ;
	if( ExeName != NULL ) _WCSCPY( ExeName, ename ) ;

	// 終了
	return 0 ;
}

// ファイルパスの拡張子を変えた文字列を得る
extern int GetChangeExeNamePath_( const char *Src, char *Dest, const char *ExeName )
{
	char DirPath[256], FileName[128] ;

	AnalysisFileNameAndDirPath_( Src, NULL, DirPath ) ;
	AnalysisFileNameAndExeName_( Src, FileName, 0 ) ;
	SetEnMark_( DirPath ) ;
	_STRCPY( Dest, DirPath ) ;
	_STRCAT( Dest, FileName ) ;
	_STRCAT( Dest, "." ) ;
	_STRCAT( Dest, ExeName ) ;
	
	// 終了
	return 0 ;
}


// 語尾に『\』がついていない場合は付ける
extern void SetEnMark_( char *PathBuf )
{
	int Len = _STRLEN( PathBuf ) ;
	
	if( PathBuf[Len-1] != '\\' )
	{
		PathBuf[Len] = '\\' ;
		PathBuf[Len+1] = '\0' ;
	}
}

// 渡された文字列をフルパス文字列として扱い、ドライブ名( \ or / の前まで )
// 又はネットワークフォルダ名( \ or / の前まで )を取得する、
// ドライブ名だった場合は最後の : も含まれ、ネットワークフォルダだった場合は最初の \\ も含まれます
// 戻り値は取得した文字列の長さ( ドライブ名の場合は最後の :、ネットワークフォルダの場合は \\ も文字列量に含まれます )
// Src はフルパスである必要があります
extern int AnalysisDriveName_( const char *Src, char *Dest )
{
	int len ;

	// ネットワークフォルダの場合は最初の \\ を代入する
	len = 0 ;
	if( Src[ 0 ] == '\\' || Src[ 0 ] == '/' )
	{
		// ネットワークフォルダ
		Dest[ 0 ] = Src[ 0 ] ;
		Dest[ 1 ] = Src[ 1 ] ;
		Dest += 2 ;
		Src += 2 ;
		len += 2 ;
	}

	// 後は文字列の末端か \ or / が出るまでコピー
	while( *Src != '\0' && *Src != '\\' && *Src != '/' )
	{
		if( CheckMultiByteChar( *Src, _GET_CHARSET() ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// 終端文字をセット
	*Dest = '\0' ;

	// 文字列長を返す
	return len ;
}

// 渡された文字列をフルパス文字列として扱い、ドライブ名( \ or / の前まで )
// 又はネットワークフォルダ名( \ or / の前まで )を取得する、
// ドライブ名だった場合は最後の : も含まれ、ネットワークフォルダだった場合は最初の \\ も含まれます
// 戻り値は取得した文字列の長さ( ドライブ名の場合は最後の :、ネットワークフォルダの場合は \\ も文字列量に含まれます )
// Src はフルパスである必要があります
extern int AnalysisDriveNameW_( const wchar_t *Src, wchar_t *Dest )
{
	int len ;

	// ネットワークフォルダの場合は最初の \\ を代入する
	len = 0 ;
	if( Src[ 0 ] == L'\\' || Src[ 0 ] == L'/' )
	{
		// ネットワークフォルダ
		Dest[ 0 ] = Src[ 0 ] ;
		Dest[ 1 ] = Src[ 1 ] ;
		Dest += 2 ;
		Src += 2 ;
		len += 2 ;
	}

	// 後は文字列の末端か \ or / が出るまでコピー
	while( *Src != L'\0' && *Src != L'\\' && *Src != L'/' )
	{
		if( CheckUTF16H( *Src ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// 終端文字をセット
	*Dest = L'\0' ;

	// 文字列長を返す
	return len ;
}


// 渡された文字列をフォルダパス文字列として扱い、フォルダ名( \ or / の前まで )を取得します
// 渡す文字列がフルパスで、最初にドライブ名が書かれていたら正常な結果が得られません
// ../ 等の下位フォルダに降りる文字列があった場合は .. 等が出力されます
// 戻り値は取得した文字列の長さです
extern int AnalysisDirectoryName_( const char *Src, char *Dest )
{
	int len ;

	len = 0 ;
	while( *Src != '\0' && *Src != '\\' && *Src != '/' )
	{
		if( CheckMultiByteChar( *Src, _GET_CHARSET() ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// 終端文字をセット
	*Dest = '\0' ;

	// 文字列長を返す
	return len ;
}

// 渡された文字列をフォルダパス文字列として扱い、フォルダ名( \ or / の前まで )を取得します
// 渡す文字列がフルパスで、最初にドライブ名が書かれていたら正常な結果が得られません
// ../ 等の下位フォルダに降りる文字列があった場合は .. 等が出力されます
// 戻り値は取得した文字列の長さです
extern int AnalysisDirectoryNameW_( const wchar_t *Src, wchar_t *Dest )
{
	int len ;

	len = 0 ;
	while( *Src != L'\0' && *Src != L'\\' && *Src != L'/' )
	{
		if( CheckUTF16H( *Src ) )
		{
			Dest[ 0 ] = Src[ 0 ] ;
			Dest[ 1 ] = Src[ 1 ] ;
			Dest += 2 ;
			Src += 2 ;
			len += 2 ;
		}
		else
		{
			*Dest = *Src ;
			Dest ++ ;
			Src ++ ;
			len ++ ;
		}
	}

	// 終端文字をセット
	*Dest = L'\0' ;

	// 文字列長を返す
	return len ;
}



}
