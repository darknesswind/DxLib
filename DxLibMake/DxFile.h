﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ファイルアクセスプログラムヘッダファイル
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

#ifndef __DXFILE_H__
#define __DXFILE_H__

// インクルード ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxHandle.h"


// マクロ定義 --------------------------------------------------------------------

// ライブラリ内部で使用するファイルアクセス用関数
#define FSYNC( handle )                             {while( FIDLECHK( handle ) == FALSE ) Thread_Sleep(0);}
#define FOPEN( path )								StreamOpen( (path), FALSE, TRUE, FALSE )
#define FOPENT( path )								StreamOpenT( (path), FALSE, TRUE, FALSE )
#define FOPEN_CACHE( path )							StreamOpen( (path),  TRUE, TRUE, FALSE )
#define FOPEN_ASYNC( path )							StreamOpen( (path), FALSE, TRUE,  TRUE )
#define FCLOSE( handle )							StreamClose( (DWORD_PTR)(handle) )
#define FREAD( buf, length, num, handle )			StreamRead( (void *)(buf), (size_t)(length), (size_t)(num), (DWORD_PTR)(handle) )
#define FSEEK( handle, pos, mode )					StreamSeek( (DWORD_PTR)(handle), (LONGLONG)(pos), (int)(mode) )
#define FTELL( handle )								StreamTell( (DWORD_PTR)(handle) )
#define FEOF( handle )								StreamEof( (DWORD_PTR)(handle) )
#define FIDLECHK( handle )							StreamIdleCheck( (DWORD_PTR)(handle) )
#define FSETDIR( path )								StreamChDir( (path) )
#define FGETDIR( buffer )							StreamGetDir( (buffer) )
#define FFINDFIRST( path, buffer )					StreamFindFirst( (path), (buffer) )
#define FFINDNEXT( handle, buffer )					StreamFindNext( (DWORD_PTR)(handle), (buffer) )
#define FFINDCLOSE( handle )						StreamFindClose( (DWORD_PTR)(handle) )

// ファイルアクセス専用スレッドへの指令
#define FILEACCESSTHREAD_FUNCTION_OPEN		(0)				// ファイルを開け
#define FILEACCESSTHREAD_FUNCTION_CLOSE		(1)				// ファイルを閉じろ
#define FILEACCESSTHREAD_FUNCTION_READ		(2)				// ファイルを読み込め
#define FILEACCESSTHREAD_FUNCTION_SEEK		(3)				// ファイルポインタを移動しろ
#define FILEACCESSTHREAD_FUNCTION_EXIT		(4)				// 終了しろ

#define FILEACCESSTHREAD_DEFAULT_CACHESIZE	(128 * 1024)	// デフォルトのキャッシュサイズ

// ハンドルタイプ
#define FILEHANDLETYPE_NORMAL				(0)				// 通常のファイルハンドル
#define FILEHANDLETYPE_FULLYLOAD			(1)				// ファイルの内容をメモリに読み込んだ処理用のハンドル

// ファイルパスの最大長
#define FILEPATH_MAX						(512 * 3)


// 環境依存定義ヘッダファイルのインクルード --------------------------------------

#ifdef __WINDOWS__
#include "Windows/DxFileWin.h"
#endif // __WINDOWS__

#ifdef __PSVITA
#include "PSVita/DxFilePSVita.h"
#endif // __PSVITA

#ifdef __PS4
#include "PS4/DxFilePS4.h"
#endif // __PS4


#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// 構造体定義 --------------------------------------------------------------------

// ファイルアクセスハンドル情報
struct FILEACCESSINFO
{
	HANDLEINFO				HandleInfo ;						// ハンドル共通データ
	int						HandleType ;						// ハンドルタイプ( FILEHANDLETYPE_NORMAL など )

	// FILEHANDLETYPE_NORMAL でのみ使用
	DWORD_PTR				FilePointer ;						// ファイルポインタ

	// FILEHANDLETYPE_FULLYLOAD でのみ使用
	void					*FileImage ;						// 読み込んだファイルを格納したメモリ領域へのポインタ
	LONGLONG				FileSize ;							// ファイルサイズ
} ;

// ファイルアクセス処理用構造体
struct FILEACCESS
{
	int						EofFlag ;							// 終端チェックフラグ
	ULONGLONG				Position ;							// アクセス位置
	ULONGLONG				Size ;								// サイズ
	int						UseASyncReadFlag ;					// 非同期読み込みを行うかどうか

	FILEACCESS_PF			PF ;								// 環境依存情報
} ;

// ファイル検索処理用構造体
struct FINDINFO
{
	FINDINFO_PF				PF ;								// 環境依存情報
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// デフォルトストリームファンクション
extern STREAMDATASHREDTYPE2W StreamFunctionW ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// ファイルアクセス処理の初期化・終了関数
extern	int			InitializeFile( void ) ;														// ファイルアクセス処理の初期化
extern	int			TerminateFile( void ) ;															// ファイルアクセス処理の後始末

// 読み込み専用ファイルアクセス関数
extern	DWORD_PTR	ReadOnlyFileAccessOpen( const wchar_t *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag  ) ;
extern	int			ReadOnlyFileAccessClose( DWORD_PTR Handle ) ;
extern	LONGLONG	ReadOnlyFileAccessTell( DWORD_PTR Handle ) ;
extern	int			ReadOnlyFileAccessSeek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType ) ;
extern	size_t		ReadOnlyFileAccessRead( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle ) ;
extern	int			ReadOnlyFileAccessEof( DWORD_PTR Handle ) ;
extern	int			ReadOnlyFileAccessIdleCheck( DWORD_PTR Handle ) ;
extern	int			ReadOnlyFileAccessChDir( const wchar_t *Path ) ;
extern	int			ReadOnlyFileAccessGetDir( wchar_t *Buffer ) ;
extern	DWORD_PTR	ReadOnlyFileAccessFindFirst( const wchar_t *FilePath, FILEINFOW *Buffer ) ;				// 戻り値: -1=エラー  -1以外=FindHandle
extern	int			ReadOnlyFileAccessFindNext( DWORD_PTR FindHandle, FILEINFOW *Buffer ) ;					// 戻り値: -1=エラー  0=成功
extern	int			ReadOnlyFileAccessFindClose( DWORD_PTR FindHandle ) ;									// 戻り値: -1=エラー  0=成功

// 書き込み専用ファイルアクセス関数
extern	int			WriteOnlyFileAccessDelete( const wchar_t *Path ) ;
extern	DWORD_PTR	WriteOnlyFileAccessOpen( const wchar_t *Path ) ;
extern	DWORD_PTR	WriteOnlyFileAccessOpenWCHAR( const char *Path ) ; // Visual C++ 6.0 用に引数を char 型にしただけのもの
extern	int			WriteOnlyFileAccessClose( DWORD_PTR Handle ) ;
extern	int			WriteOnlyFileAccessSeek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType ) ;
extern	int			WriteOnlyFileAccessWrite( DWORD_PTR Handle, void *Buffer, size_t WriteSize, size_t *GetWriteSize = NULL ) ;
extern	int			WriteOnlyFileAccessPrintf( DWORD_PTR Handle, const char *FormatString, ... ) ;

// ストリームデータアクセス関数
extern	DWORD_PTR	StreamOpen(  const wchar_t *Path, int UseCacheFlag, int BlockFlag, int UseASyncReadFlag ) ;
extern	DWORD_PTR	StreamOpenT( const TCHAR   *Path, int UseCacheFlag, int BlockFlag, int UseASyncReadFlag ) ;
extern	int			StreamClose( DWORD_PTR Handle ) ;
extern	LONGLONG	StreamTell( DWORD_PTR Handle ) ;
extern	int			StreamSeek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType ) ;
extern	size_t		StreamRead( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle ) ;
extern	int			StreamEof( DWORD_PTR Handle ) ;
extern	int			StreamIdleCheck( DWORD_PTR Handle ) ;
extern	int			StreamChDir(  const wchar_t *Path ) ;
extern	int			StreamChDirT( const TCHAR   *Path ) ;
extern	int			StreamGetDir(  wchar_t *Buffer ) ;
extern	int			StreamGetDirT( TCHAR   *Buffer ) ;
extern	DWORD_PTR	StreamFindFirst(  const wchar_t *FilePath, FILEINFOW *Buffer ) ;		// 戻り値: -1=エラー  -1以外=FindHandle
extern	DWORD_PTR	StreamFindFirstT( const TCHAR   *FilePath, FILEINFO  *Buffer ) ;		// 戻り値: -1=エラー  -1以外=FindHandle
extern	int			StreamFindNext(  DWORD_PTR FindHandle, FILEINFOW *Buffer ) ;			// 戻り値: -1=エラー  0=成功
extern	int			StreamFindNextT( DWORD_PTR FindHandle, FILEINFO  *Buffer ) ;			// 戻り値: -1=エラー  0=成功
extern	int			StreamFindClose( DWORD_PTR FindHandle ) ;								// 戻り値: -1=エラー  0=成功
extern	const STREAMDATASHREDTYPE2W *StreamGetStruct( void ) ;

// ファイル情報の wchar_t 版と TCHAR 版のデータ変換関数
extern	int			ConvFileIntoToFileInfoW( FILEINFO  *Src, FILEINFOW *Dest ) ;
extern	int			ConvFileIntoWToFileInfo( FILEINFOW *Src, FILEINFO  *Dest ) ;

// ストリームデータアクセス用関数構造体関係
extern	STREAMDATASHRED *GetFileStreamDataShredStruct( void ) ;								// ストリームデータ読みこみ制御用関数ポインタ構造体のファイル用構造体を得る
extern	STREAMDATASHRED *GetMemStreamDataShredStruct( void ) ;								// ストリームデータ読みこみ制御用関数ポインタ構造体のﾒﾓﾘ用構造体を得る




// ファイルハンドル関数
extern	int			FileRead_open_UseGParam( const wchar_t *FilePath, int ASync, int ASyncLoadFlag = FALSE ) ;				// ファイルを開く
extern	int			FileRead_seek_UseGParam( int FileHandle, LONGLONG Offset, int Origin, int ASyncLoadFlag = FALSE ) ;		// ファイルポインタの位置を変更する
extern	int			FileRead_read_UseGParam( int FileHandle, void *Buffer, int ReadSize, int ASyncLoadFlag = FALSE ) ;		// ファイルからデータを読み込む
extern	int			FileRead_fullyLoad_UseGParam( const wchar_t *FilePath, int ASyncLoadFlag = FALSE ) ;					// 指定のファイルの内容を全てメモリに読み込み、その情報のアクセスに必要なハンドルを返す( 戻り値  -1:エラー  -1以外:ハンドル )、使い終わったらハンドルは FileRead_fullyLoad_delete で削除する必要があります
extern	int			FileRead_scanf_base(        int       FileHandle, const void  *Format,        va_list Param ) ;	// ファイルから書式化されたデータを読み出す
extern	int			FileRead_scanf_baseCHAR(    DWORD_PTR FileHandle, const char  *Format,        va_list Param ) ;	// ファイルから書式化されたデータを読み出す
extern	int			FileRead_scanf_baseUTF16LE( DWORD_PTR FileHandle, const WORD  *FormatUTF16LE, va_list Param ) ;	// ファイルから書式化されたデータを読み出す



// メモリに置かれたデータをファイルのデータに例えてつかうための関数
extern	void*		MemStreamOpen( const void *DataBuffer, unsigned int DataSize ) ;
extern	int			MemStreamClose( void *StreamDataPoint ) ;



// ファイルパス関係

// フルパスではないパス文字列をフルパスに変換する
// ( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )
// ( CurrentDir が NULL の場合は現在のカレントディレクトリを使用する )
//extern int			ConvertFullPath_( const char *Src, char *Dest, const char *CurrentDir = NULL ) ; 
extern int			ConvertFullPathW_( const wchar_t *Src, wchar_t *Dest, const wchar_t *CurrentDir = NULL ) ; 
extern int			ConvertFullPathT_( const TCHAR *Src, TCHAR *Dest, const TCHAR *CurrentDir = NULL ) ; 

// 指定のファイルパスを指定のフォルダパスから相対アクセスするための相対パスを作成する
// ( FilePath や StartFolderPath がフルパスではなかった場合は関数内でフルパス化されます )
// StartFolderPath の末端に / or \ があっても問題ありません
//extern int			CreateRelativePath_( const char *FilePath, const char *StartFolderPath, char *Dest ) ;
extern int			CreateRelativePathW_( const wchar_t *FilePath, const wchar_t *StartFolderPath, wchar_t *Dest ) ;
extern int			CreateRelativePathT_( const TCHAR   *FilePath, const TCHAR   *StartFolderPath, TCHAR   *Dest ) ;

// 特定のパス文字列から色々な情報を取得する
// ( CurrentDir はフルパスである必要がある(語尾に『\』があっても無くても良い) )
// ( CurrentDir が 0 の場合は実際のカレントディレクトリを使用する )
extern int			AnalyseFilePathW_( const wchar_t *Src, // DirPath の終端には \ は付かない
						wchar_t *FullPath, wchar_t *DirPath, wchar_t *FileName, wchar_t *Name, wchar_t *ExeName, const wchar_t *CurrentDir = 0 );
extern int			AnalyseFilePathT_( const TCHAR   *Src, // DirPath の終端には \ は付かない
						TCHAR   *FullPath, TCHAR   *DirPath, TCHAR   *FileName, TCHAR   *Name, TCHAR   *ExeName, const TCHAR   *CurrentDir = 0 );

// ファイル名も一緒になっていると分かっているパス中からファイル名とディレクトリパスを分割する
// フルパスである必要は無い、ファイル名だけでも良い
// DirPath の終端に ￥ マークは付かない
//extern int		AnalysisFileNameAndDirPath_( const char *Src, char *FileName = 0, char *DirPath = 0 ) ;
extern int			AnalysisFileNameAndDirPathW_( const wchar_t *Src, wchar_t *FileName = 0, wchar_t *DirPath = 0 ) ;
extern int			AnalysisFileNameAndDirPathT_( const TCHAR   *Src, TCHAR   *FileName = 0, TCHAR   *DirPath = 0 ) ;

// ファイルパスからファイル名と拡張子を取得する
//extern int		AnalysisFileNameAndExeName_( const char *Src, char *Name = 0, char *ExeName = 0 ) ;
extern int			AnalysisFileNameAndExeNameW_( const wchar_t *Src, wchar_t *Name = 0, wchar_t *ExeName = 0 ) ;
extern int			AnalysisFileNameAndExeNameT_( const TCHAR   *Src, TCHAR   *Name = 0, TCHAR   *ExeName = 0 ) ;

// ファイルパスの拡張子を変えた文字列を得る
//extern int		GetChangeExeNamePath_( const char *Src, char *Dest, const char *ExeName ) ;

extern void			SetEnMarkT_( TCHAR   *PathBuf ) ;				// 語尾に『\』がついていない場合は付ける
extern void			SetEnMarkW_( wchar_t *PathBuf ) ;				// 語尾に『\』がついていない場合は付ける

extern void			ChangeEnMarkToSlashT_( TCHAR   *PathBuf ) ;		// パス文字列中の『\』を『/』に変換する
extern void			ChangeEnMarkToSlashW_( wchar_t *PathBuf ) ;		// パス文字列中の『\』を『/』に変換する

// 渡された文字列をフルパス文字列として扱い、ドライブ名( :\ or :/ の前まで )
// 又はネットワークフォルダ名( \ or / の前まで )を取得する
// ネットワークフォルダだった場合は最初の \\ も含める
// 戻り値は取得した文字列の長さ( ネットワークフォルダの場合は \\ も文字列量に含まれます )
// Src はフルパスである必要があります、相対パスでは正常に動作しません
//extern int		AnalysisDriveName_( const char *Src, char *Dest ) ;
extern int			AnalysisDriveNameW_( const wchar_t *Src, wchar_t *Dest ) ;

// 渡された文字列をフォルダパス文字列として扱い、フォルダ名( \ or / の前まで )を取得します
// 渡す文字列がフルパスで、最初にドライブ名が書かれていたら正常な結果が得られません
// ../ 等の下位フォルダに降りる文字列があった場合は .. 等が出力されます
// 戻り値は取得した文字列の長さです
//extern int		AnalysisDirectoryName_( const char *Src, char *Dest ) ;
extern int			AnalysisDirectoryNameW_( const wchar_t *Src, wchar_t *Dest ) ;





// 環境依存処理用関数

// ファイルアクセス処理の初期化・終了関数
extern	int			InitializeFile_PF( void ) ;														// ファイルアクセス処理の初期化関数の環境依存の処理を行う関数
extern	int			TerminateFile_PF( void ) ;														// ファイルアクセス処理の後始末関数の環境依存の処理を行う関数

// ファイルアクセス関数
extern	int			ReadOnlyFileAccessOpen_PF(		FILEACCESS *FileAccess, const wchar_t *Path, int UseCacheFlag, int BlockReadFlag ) ;
extern	int			ReadOnlyFileAccessClose_PF(		FILEACCESS *FileAccess ) ;
extern	LONGLONG	ReadOnlyFileAccessTell_PF(		FILEACCESS *FileAccess ) ;
extern	int			ReadOnlyFileAccessSeek_PF(		FILEACCESS *FileAccess, LONGLONG SeekPoint ) ;
extern	size_t		ReadOnlyFileAccessRead_PF(		void *Buffer, size_t BlockSize, size_t DataNum, FILEACCESS *FileAccess ) ;
extern	int			ReadOnlyFileAccessIdleCheck_PF(	FILEACCESS *FileAccess ) ;
extern	int			ReadOnlyFileAccessChDir_PF(		const wchar_t *Path ) ;
extern	int			ReadOnlyFileAccessGetDir_PF(	wchar_t *Buffer ) ;
extern	int			ReadOnlyFileAccessFindFirst_PF(	FINDINFO *FindInfo, const wchar_t *FilePath, FILEINFOW *Buffer ) ;
extern	int			ReadOnlyFileAccessFindNext_PF(	FINDINFO *FindInfo, FILEINFOW *Buffer ) ;
extern	int			ReadOnlyFileAccessFindClose_PF(	FINDINFO *FindInfo ) ;

// 書き込み専用ファイルアクセス関数
extern	int			WriteOnlyFileAccessDelete_PF( const wchar_t *Path ) ;
extern	DWORD_PTR	WriteOnlyFileAccessOpen_PF(  const wchar_t *Path ) ;
extern	int			WriteOnlyFileAccessClose_PF( DWORD_PTR Handle ) ;
extern	int			WriteOnlyFileAccessSeek_PF(  DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType ) ;
extern	int			WriteOnlyFileAccessWrite_PF( DWORD_PTR Handle, void *Buffer, size_t WriteSize, size_t *GetWriteSize = NULL ) ;


#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXFILE_H__

