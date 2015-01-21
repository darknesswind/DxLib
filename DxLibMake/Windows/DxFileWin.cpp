//-----------------------------------------------------------------------------
// 
// 		ＤＸLibrary		Windows文件相关程序
// 
//  	Ver 3.11f
// 
//-----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ---------------------------------------------------------------
#include "DxFileWin.h"
#include "../DxFile.h"
#include "../DxBaseFunc.h"
#include "DxWindow.h"
#include "../DxLog.h"
#include <windows.h>

namespace DxLib
{

// 宏定义 -----------------------------------------------------------------

// 结构体声明 -----------------------------------------------------------------

// 数据定义 -----------------------------------------------------------------

// 函数声明 -------------------------------------------------------------------

DWORD	WINAPI		FileAccessThreadFunction( void *FileAccessThreadData ) ;					// FileAccess专用线程用函数
int					SetFilePointer64( HANDLE FileHandle, ULONGLONG Position ) ;					// 设定文件的访问位置( 0:成功  -1:失败 )

// Program -----------------------------------------------------------------

// 设定文件的访问位置
int SetFilePointer64( HANDLE FileHandle, ULONGLONG Position )
{
	DWORD High, Low ;
	DWORD Result ;

	Low = ( DWORD )Position ;
	High = ( DWORD )( Position >> 32 ) ;
	Result = SetFilePointer( FileHandle, ( LONG )Low, ( LONG * )&High, FILE_BEGIN ) ;
	if( Result == 0xffffffff && GetLastError() != NO_ERROR )
	{
		return -1 ;
	}

	return 0 ;
}

// 生成临时文件
extern HANDLE CreateTemporaryFile( TCHAR *TempFileNameBuffer )
{
	TCHAR String1[MAX_PATH], String2[MAX_PATH] ;
	HANDLE FileHandle ;
	int Length ;

	// 取得临时文件的目录路径
	if( GetTempPath( 256, String1 ) == 0 ) return NULL ;

	// 字符串的最后加上目录分隔符
	Length = lstrlen( String1 ) ;
	if( String1[Length-1] != _T( '\\' ) )
	{
		String1[Length] = _T( '\\' ) ;
		String1[Length+1] = _T( '\0' ) ;
	}

	// 生成临时文件的文件名
	if( GetTempFileName( String1, _T( "tmp" ), 0, String2 ) == 0 ) return NULL ;

	// 转换为完整路径
	ConvertFullPathT_( String2, String1 ) ;

	// 打开临时文件
	DeleteFile( String1 ) ;
	FileHandle = CreateFile( String1, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( FileHandle == NULL ) return NULL ;

	// 保存临时文件名
	if( TempFileNameBuffer != NULL ) lstrcpy( TempFileNameBuffer, String1 ) ;

	// 返回句柄
	return FileHandle ;
}



// FileAccess专用线程用函数
DWORD WINAPI FileAccessThreadFunction( void *FileAccessThreadData )
{
	FILEACCESSTHREAD *dat = (FILEACCESSTHREAD *)FileAccessThreadData ;
	DWORD res, ReadSize ;

	for(;;)
	{
		for(;;)
		{
			// 是否使用Cache的分支处理
			if( dat->CacheBuffer )
			{
				// 指令来到之前稍等一下
				res = WaitForSingleObject( dat->FuncEvent, 100 ) ;

				// 超时且文件被打开的情况下进行缓存处理
				if( res == WAIT_TIMEOUT && dat->Handle != NULL )
				{
					// 如果缓存满了则什么都不做
					if( dat->CacheSize != FILEACCESSTHREAD_DEFAULT_CACHESIZE )
					{
						// 设置读取的初始位置
						SetFilePointer64( dat->Handle, dat->CachePosition + dat->CacheSize ) ;

						// 读取
						ReadFile( dat->Handle, &dat->CacheBuffer[dat->CacheSize], ( DWORD )( FILEACCESSTHREAD_DEFAULT_CACHESIZE - dat->CacheSize ), &ReadSize, NULL ) ;
						
						// 增加有效Size
						dat->CacheSize += ( LONGLONG )ReadSize ;
					}
				}
				else
				{
					break ;
				}
			}
			else
			{
				// 等待指令
				res = WaitForSingleObject( dat->FuncEvent, INFINITE ) ;
				if( res == WAIT_TIMEOUT && dat->Handle != NULL ) continue;
				break;
			}
		}

//		WaitForSingleObject( dat->FuncEvent, INFINITE ) ;

		// 解除事件的信号状态
		ResetEvent( dat->FuncEvent ) ;
		ResetEvent( dat->CompEvent ) ;

		// 判断收到的指令
		switch( dat->Function )
		{
		case FILEACCESSTHREAD_FUNCTION_OPEN :
			dat->Handle = CreateFile( dat->FilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			if( dat->Handle == INVALID_HANDLE_VALUE )
			{
				dat->ErrorFlag = TRUE ;
				goto END ;
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_CLOSE :
			CloseHandle( dat->Handle ) ;
			dat->Handle = NULL ;
			break ;

		case FILEACCESSTHREAD_FUNCTION_READ :
			// 缓存加载位置一致时从缓存传送数据
			if( dat->CacheBuffer && dat->ReadPosition == dat->CachePosition && dat->CacheSize != 0 )
			{
				DWORD MoveSize ;

				// 调整Size
				MoveSize = dat->ReadSize ;
				if( MoveSize > dat->CacheSize ) MoveSize = ( DWORD )dat->CacheSize ;

				// 传送
				_MEMCPY( dat->ReadBuffer, dat->CacheBuffer, MoveSize ) ;

				// 传递加载Size和位置
				dat->ReadBuffer = (void *)( (BYTE *)dat->ReadBuffer + MoveSize ) ;
				dat->ReadPosition += MoveSize ;
				dat->ReadSize -= MoveSize ;
				
				// 更新缓存信息
				dat->CachePosition += MoveSize ;
				dat->CacheSize     -= MoveSize ;
				if( dat->CacheSize != 0 ) _MEMMOVE( &dat->CacheBuffer[0], &dat->CacheBuffer[MoveSize], ( size_t )dat->CacheSize ) ;
			}

			// 希望のデータが全て読めていない場合は更にファイルから読み込む
			if( dat->ReadSize != 0 )
			{
				SetFilePointer64( dat->Handle, dat->ReadPosition ) ;
				ReadFile( dat->Handle, dat->ReadBuffer, dat->ReadSize, &dat->ReadSize, NULL ) ;

				// 初始化缓存
				if( dat->CacheBuffer )
				{
					dat->CachePosition = dat->ReadPosition + dat->ReadSize ;
					dat->CacheSize = 0 ;
				}
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_SEEK :
			SetFilePointer64( dat->Handle, dat->SeekPoint ) ;

			// 初始化缓存
			if( dat->CacheBuffer )
			{
				dat->CachePosition = (DWORD)dat->SeekPoint ;
				dat->CacheSize = 0 ;
			}
			break ;

		case FILEACCESSTHREAD_FUNCTION_EXIT :
			if( dat->Handle != NULL ) CloseHandle( dat->Handle ) ;
			dat->Handle = NULL ;
			goto END ;
		}

		// 指令が完了したら完了イベントをシグナル状態にする
		SetEvent( dat->CompEvent ) ;
	}

END:
	// エラー時の為に完了イベントをシグナル状態にする
	SetEvent( dat->CompEvent ) ;
	dat->EndFlag = TRUE ;
	ExitThread( 1 ) ;

	return 0 ;
}



// ファイルアクセス関数
extern DWORD_PTR ReadOnlyFileAccessOpen( const TCHAR *Path, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	WINFILEACCESS *FileAccess ;
	DWORD Code ;
	DWORD High ;
	
//	UseCacheFlag = UseCacheFlag ;
	BlockReadFlag = BlockReadFlag ;

	FileAccess = (WINFILEACCESS *)DXALLOC( sizeof( WINFILEACCESS ) ) ;
	if( FileAccess == NULL ) return 0 ;

	_MEMSET( FileAccess, 0, sizeof( WINFILEACCESS ) ) ;

//	// キャッシュを使用するかどうかをスレッドを使用するかどうかにしてしまう
//	FileAccess->UseThread = UseCacheFlag ;

	// キャッシュを使用するかどうかのフラグをセット
	FileAccess->UseCacheFlag = UseCacheFlag ;
	FileAccess->ThreadData.CacheBuffer = NULL;

	// 非同期読み書きフラグをセット
	FileAccess->UseASyncReadFlag = UseASyncReadFlag ;

	// キャッシュ、若しくは非同期読み書きを行う場合はスレッドを使用する
	FileAccess->UseThread = FileAccess->UseCacheFlag || FileAccess->UseASyncReadFlag ;

	// スレッドを使用する場合としない場合で処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// スレッドを使用する場合はファイルアクセス専用スレッドを立てる

		// 最初にファイルを開けるかどうか確かめておく
		FileAccess->Handle = CreateFile( Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( FileAccess->Handle == INVALID_HANDLE_VALUE )
		{
			DXFREE( FileAccess ) ;
			return 0 ;
		}
		FileAccess->Size = GetFileSize( FileAccess->Handle, &High ) ;
		FileAccess->Size |= ( ( ULONGLONG )High ) << 32 ;
		CloseHandle( FileAccess->Handle ) ;
		FileAccess->Handle = NULL ;

		// キャッシュ用メモリの確保
		if( FileAccess->UseCacheFlag )
		{
			FileAccess->ThreadData.CacheBuffer = (BYTE *)DXALLOC( FILEACCESSTHREAD_DEFAULT_CACHESIZE );
			if( FileAccess->ThreadData.CacheBuffer == NULL )
			{
				DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
				DXFREE( FileAccess ) ;
				DXST_ERRORLOG_ADD( _T( "ファイル読み込みキャッシュ用メモリの確保に失敗しました\n" ) ) ;
				return 0 ;
			}
		}

		// 専用スレッドデータを初期化する
		FileAccess->ThreadData.Handle = NULL ;
		FileAccess->ThreadData.ThreadHandle = NULL ;
		FileAccess->ThreadData.FuncEvent = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
		FileAccess->ThreadData.CompEvent = CreateEvent( NULL, TRUE, TRUE, NULL ) ;

		FileAccess->ThreadData.ThreadHandle = CreateThread(
												NULL,
												0,
												(LPTHREAD_START_ROUTINE)FileAccessThreadFunction, 
												&FileAccess->ThreadData,
												0,
												&FileAccess->ThreadData.ThreadID ) ;
		if( FileAccess->ThreadData.ThreadHandle == NULL )
		{
			if( FileAccess->ThreadData.CacheBuffer ) DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
			CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
			CloseHandle( FileAccess->ThreadData.CompEvent ) ;
			DXFREE( FileAccess ) ;
			DXST_ERRORLOG_ADD( _T( "ファイルアクセス専用スレッドの作成に失敗しました\n" ) ) ;
			return 0 ;
		}
		SetThreadPriority( FileAccess->ThreadData.ThreadHandle, THREAD_PRIORITY_LOWEST ) ;

		// ファイルオープン指令はここで完了してしまう
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_OPEN ;
		lstrcpy( FileAccess->ThreadData.FilePath, Path ) ;

		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// 指令が終了するまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;
		if( FileAccess->ThreadData.ErrorFlag == TRUE )
		{
			if( FileAccess->ThreadData.CacheBuffer ) DXFREE( FileAccess->ThreadData.CacheBuffer ) ;
			CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
			CloseHandle( FileAccess->ThreadData.CompEvent ) ;
			do
			{
				Sleep(0);
				GetExitCodeThread( FileAccess->ThreadData.ThreadHandle, &Code );
			}while( Code == STILL_ACTIVE );
			CloseHandle( FileAccess->ThreadData.ThreadHandle ) ;
			DXFREE( FileAccess ) ;
			DXST_ERRORLOG_ADD( _T( "ファイルのオープンに失敗しました\n" ) ) ;
			return 0 ;
		}
	}
	else
	{
		// スレッドを使用しない場合はこの場でファイルを開く
		FileAccess->Handle = CreateFile( Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( FileAccess->Handle == INVALID_HANDLE_VALUE )
		{
			DXFREE( FileAccess ) ;
			return 0 ;
		}
		FileAccess->Size = GetFileSize( FileAccess->Handle, &High ) ;
		FileAccess->Size |= ( ( ULONGLONG )High ) << 32 ;
	}
	FileAccess->EofFlag = FALSE ;
	FileAccess->Position = 0 ;

	return (DWORD_PTR)FileAccess ;
}

extern int ReadOnlyFileAccessClose( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	BOOL Result;
	DWORD Code ;

	// スレッドを使用する場合としない場合で処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// これ以前の指令が出ていた場合の為に指令完了イベントがシグナル状態になるまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドに終了指令を出す
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_EXIT ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// 指令が終了するまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドが終了するのを待つ
		do
		{
			Sleep(1);
			GetExitCodeThread( FileAccess->ThreadData.ThreadHandle, &Code );
		}while( Code == STILL_ACTIVE );

		// キャッシュを使用していた場合はキャッシュ用メモリを開放する
		if( FileAccess->ThreadData.CacheBuffer )
			DXFREE( FileAccess->ThreadData.CacheBuffer ) ;

		// イベントやスレッドを閉じる
		CloseHandle( FileAccess->ThreadData.ThreadHandle ) ;
		CloseHandle( FileAccess->ThreadData.CompEvent ) ;
		CloseHandle( FileAccess->ThreadData.FuncEvent ) ;
		Result = 0 ;
	}
	else
	{
		// 使用していない場合はこの場でハンドルを閉じて終了
		Result = CloseHandle( FileAccess->Handle ) ;
	}
	DXFREE( FileAccess ) ;

	return Result != 0 ? 0 : -1/*EOF*/ ;
}

extern LONGLONG ReadOnlyFileAccessTell( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;

	return ( LONGLONG )FileAccess->Position ;
}

extern int ReadOnlyFileAccessSeek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	ULONGLONG Pos = 0 ;
	int Result ;

	switch( SeekType )
	{
	case SEEK_CUR :
		if( FileAccess->Position + SeekPoint < 0 )
		{
			Pos = 0 ;
		}
		else
		{
			Pos = FileAccess->Position + SeekPoint ;
		}
		break ;

	case SEEK_END :
		if( FileAccess->Size + SeekPoint < 0 )
		{
			Pos = 0 ;
		}
		else
		{
			Pos = FileAccess->Size + SeekPoint ;
		}
		break ;

	case SEEK_SET :
		if( SeekPoint < 0 )
		{
			Pos = 0 ;
		}
		else
		{
			Pos = SeekPoint ;
		}
		break ;
	}

	// スレッドを使用しているかどうかで処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// これ以前の指令が出ていた場合の為に指令完了イベントがシグナル状態になるまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドにファイル位置変更指令を出す
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_SEEK ;
		FileAccess->ThreadData.SeekPoint = Pos ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;
	}
	else
	{
		// ファイルアクセス位置を変更する
		Result = SetFilePointer64( FileAccess->Handle, Pos ) ;
		if( Result == -1 ) return -1 ;
	}

	// 位置を保存しておく
	FileAccess->Position = Pos ;

	// 終端チェックフラグを倒す
	FileAccess->EofFlag = FALSE ;

	// 終了
	return 0 ;
}

extern size_t ReadOnlyFileAccessRead( void *Buffer, size_t BlockSize, size_t DataNum, DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = ( WINFILEACCESS * )Handle ;
	DWORD Result, BytesRead ;

	if( BlockSize == 0 ) return 0 ;

	// 終端チェック
	if( FileAccess->Position == FileAccess->Size )
	{
		FileAccess->EofFlag = TRUE ;
		return 0 ;
	}

	// 項目数調整
	if( BlockSize * DataNum + FileAccess->Position > FileAccess->Size )
	{
		DataNum = ( size_t )( ( FileAccess->Size - FileAccess->Position ) / BlockSize ) ;
	}
	
	if( DataNum == 0 )
	{
		FileAccess->EofFlag = TRUE ;
		return 0 ;
	}

	// スレッドを使用しているかどうかで処理を分岐
	if( FileAccess->UseThread == TRUE )
	{
		// これ以前の指令が出ていた場合の為に指令完了イベントがシグナル状態になるまで待つ
		WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;

		// スレッドにファイル読み込み指令を出す
		FileAccess->ThreadData.Function = FILEACCESSTHREAD_FUNCTION_READ ;
		FileAccess->ThreadData.ReadBuffer = Buffer ;
		FileAccess->ThreadData.ReadPosition = FileAccess->Position ;
		FileAccess->ThreadData.ReadSize = ( DWORD )( BlockSize * DataNum ) ;
		ResetEvent( FileAccess->ThreadData.CompEvent ) ;
		SetEvent( FileAccess->ThreadData.FuncEvent ) ;

		// 非同期かどうかで処理を分岐
		if( FileAccess->UseASyncReadFlag == FALSE )
		{
			// 同期読み込みの場合は指令が完了するまで待つ
			WaitForSingleObject( FileAccess->ThreadData.CompEvent, INFINITE ) ;
		}

		BytesRead = ( DWORD )( BlockSize * DataNum ) ;
		Result = 1 ;
	}
	else
	{
		Result = ReadFile( FileAccess->Handle, Buffer, DWORD( BlockSize * DataNum ), &BytesRead, NULL ) ;
	}

	FileAccess->Position += ( DWORD )( DataNum * BlockSize ) ;
	return Result != 0 ? BytesRead / BlockSize : 0 ;
}

extern int ReadOnlyFileAccessEof( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = (WINFILEACCESS *)Handle ;

	return FileAccess->EofFlag ? EOF : 0 ;
}

extern int ReadOnlyFileAccessIdleCheck( DWORD_PTR Handle )
{
	WINFILEACCESS *FileAccess = (WINFILEACCESS *)Handle ;

	if( FileAccess->UseThread == TRUE )
	{
		return WaitForSingleObject( FileAccess->ThreadData.CompEvent, 0 ) == WAIT_TIMEOUT ? FALSE : TRUE ;
	}
	else
	{
		return TRUE ;
	}
}

extern int ReadOnlyFileAccessChDir( const TCHAR *Path )
{
	return SetCurrentDirectory( Path ) ;
}

extern int ReadOnlyFileAccessGetDir( TCHAR *Buffer )
{
	return GetCurrentDirectory( MAX_PATH, Buffer ) ;
}

static void _WIN32_FIND_DATA_To_FILEINFO( WIN32_FIND_DATA *FindData, FILEINFO *FileInfo )
{
	// ファイル名のコピー
	lstrcpy( FileInfo->Name, FindData->cFileName );

	// ディレクトリかどうかのフラグをセット
	FileInfo->DirFlag = (FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;

	// ファイルのサイズをセット
	FileInfo->Size = ( LONGLONG )( ( ( ( ULONGLONG )FindData->nFileSizeHigh ) << 32 ) + FindData->nFileSizeLow ) ;

	// ファイルタイムを保存
	_FileTimeToLocalDateData( &FindData->ftCreationTime, &FileInfo->CreationTime );
	_FileTimeToLocalDateData( &FindData->ftLastWriteTime, &FileInfo->LastWriteTime );
}

// 戻り値: -1=エラー  -1以外=FindHandle
extern DWORD_PTR ReadOnlyFileAccessFindFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	WIN32_FIND_DATA FindData;
	HANDLE FindHandle;

	FindHandle = FindFirstFile( FilePath, &FindData );
	if( FindHandle == INVALID_HANDLE_VALUE ) return ( DWORD_PTR )-1;

	if( Buffer ) _WIN32_FIND_DATA_To_FILEINFO( &FindData, Buffer );

	return (DWORD_PTR)FindHandle;
}

// 戻り値: -1=エラー  0=成功
extern int ReadOnlyFileAccessFindNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	WIN32_FIND_DATA FindData;

	if( FindNextFile( (HANDLE)FindHandle, &FindData ) == 0 ) return -1;

	if( Buffer ) _WIN32_FIND_DATA_To_FILEINFO( &FindData, Buffer );

	return 0;
}

// 戻り値: -1=エラー  0=成功
extern int ReadOnlyFileAccessFindClose( DWORD_PTR FindHandle )
{
	// ０以外が返ってきたら成功
	return FindClose( (HANDLE)FindHandle ) != 0 ? 0 : -1;
}







}

