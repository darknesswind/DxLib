//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用ログプログラム
// 
//  	Ver 3.11f
// 
//-----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ---------------------------------------------------------------
#include "DxLogWin.h"

#ifndef DX_NON_LOG

#include "../DxStatic.h"
#include "../DxLib.h"
#include "../DxLog.h"
#include "../DxBaseFunc.h"
#include "../DxMemory.h"
#include "DxWindow.h"
#include "DxWinAPI.h"

namespace DxLib
{

// 宏定义 -----------------------------------------------------------------

// 结构体声明 -----------------------------------------------------------------

// 数据定义 -----------------------------------------------------------------

// 函数声明 -------------------------------------------------------------------

// Program -----------------------------------------------------------------

// エラー書き出しファイルを初期化する処理の環境依存部分
extern int ErrorLogInitialize_Native( void )
{
	HANDLE fp ;
	TCHAR String[ 512 ] ;
	int Len ;

	// ログ出力抑制フラグが立っているか、ユーザー指定のウインドウを
	// 使用している場合は出力を行わない
	if( LogData.NotLogOutFlag == TRUE /* || WinData.UserWindowFlag == TRUE */ ) return 0 ;

	// 日付つきのログ出力が指定されている場合はファイル名を作成
	if( LogData.UseLogDateName == TRUE )
	{
		DATEDATA Date ;
		TCHAR String[128] ;

		// 日付を取得
		NS_GetDateTime( &Date ) ;

		// 文字列の作成
		lstrcpy( LogData.LogFileName, _T( "Log" ) ) ;
		_ITOT( Date.Year, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Mon, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Day, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Hour, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Min, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( "_" ) ) ;
		_ITOT( Date.Sec, String, 10 ) ;
		lstrcat( LogData.LogFileName, String ) ;
		lstrcat( LogData.LogFileName, _T( ".txt" ) ) ;
	}
	else
	{
		// それ以外の場合は通常のファイル名
		lstrcpy( LogData.LogFileName, _T( "Log.txt" ) ) ;
	}

	Len = lstrlen( LogData.LogOutDirectory ) ;
	_TSTRNCPY( String, LogData.LogOutDirectory, Len ) ;
	if( String[ Len - 1 ] != _T( '\\' ) )
	{
		String[ Len ] = _T( '\\' ) ;
		Len ++ ;
	}
	lstrcpy( String + Len, LogData.LogFileName ) ;

	// エラーログファイルを再作成する
	DeleteFile( String ) ;
	fp = CreateFile( String, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL ) ;

#ifdef UNICODE
	BYTE BOM[ 2 ] = { 0xff, 0xfe } ;
	DWORD WriteSize ;
	WriteFile( fp, &BOM, 2, &WriteSize, NULL ) ;
#endif

	if( fp != NULL ) CloseHandle( fp ) ;

	// 終了
	return 0 ;
}

// エラー文書を書き出す
extern int NS_ErrorLogAdd( const TCHAR *ErrorStr )
{
	HANDLE fp ;
	DWORD WriteSize, StringLength ;
	TCHAR *Temp, DefaultTemp[256], *d, *AllocTemp = NULL ;
	const TCHAR *p1, *p2 ;

	if( WinAPIData.Win32Func.WinMMDLL == NULL )
		return -1 ;

	StringLength = lstrlen( ErrorStr ) ;
	Temp = DefaultTemp ;

	// もし文字列が余りにも長かったらテンポラリ領域をここで確保する
	if( StringLength > 255 )
	{
		Temp = AllocTemp = ( TCHAR * )NS_DxAlloc( StringLength * 2 * sizeof( TCHAR ) ) ;
	}

	// \n を \r\n に変換しながらコピー
	{
		Temp[0] = _T( '\0' ) ;
		p1 = ErrorStr ;
		d = Temp ;

		p2 = _TSTRCHR( p1, _T( '\n' ) ) ;
		while( p2 != NULL && ( ErrorStr == p2 || p2[-1] != _T( '\r' ) ) )
		{
			_MEMCPY( d, p1, ( p2 - p1 ) * sizeof( TCHAR ) ) ;
			d += p2 - p1 ;
			d[0] = _T( '\r' ) ;
			d[1] = _T( '\n' ) ;
			d[2] = _T( '\0' ) ;
			d += 2 ;
			p1 = p2 + 1 ;

			p2 = _TSTRCHR( p1, _T( '\n' ) ) ;
		}
		lstrcpy( d, p1 ) ;
		ErrorStr = Temp ;
	}

	// ＤＸライブラリが作成したウインドウである場合のみファイルに出力する
	if( /*WinData.MainWindow != NULL && WinData.UserWindowFlag == FALSE */ true )
	{
		// ログ出力抑制フラグが立っていた場合は出力を行わない
		if( LogData.NotLogOutFlag == FALSE && LogData.LogOutDirectory[0] != _T( '\0' ) )
		{
//			TCHAR MotoPath[MAX_PATH] ;
			TCHAR String[ 512 ] ;
			int Len ;

			// エラーログファイルを開く
//			FGETDIR( MotoPath ) ;
//			FSETDIR( LogData.LogOutDirectory ) ;
			Len = lstrlen( LogData.LogOutDirectory ) ;
			_TSTRNCPY( String, LogData.LogOutDirectory, Len ) ;
//			lstrcpy( String, LogData.LogOutDirectory ) ;
			if( String[ Len - 1 ] != _T( '\\' ) )
			{
				String[ Len ] = _T( '\\' ) ;
				Len ++ ;
			}
			lstrcpy( String + Len, LogData.LogFileName ) ;
			fp = CreateFile( String, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			if( fp != NULL )
			{
				SetFilePointer( fp, 0, NULL, FILE_END ) ;

				// 規定分だけタブを排出
				if( LogData.ErTabStop == FALSE )
				{
					int i ;	

					// タイムスタンプを出力
					if( LogData.NonUseTimeStampFlag == 0 )
					{
						_TSPRINTF( _DXWTP( String ), _DXWTR( "%d:" ), WinAPIData.Win32Func.timeGetTimeFunc() - LogData.LogStartTime ) ;
						WriteFile( fp, String, lstrlen( String ) * sizeof( TCHAR ), &WriteSize, NULL ) ;
						OutputDebugString( String ) ;
					}

					if( LogData.ErTabNum != 0 )
					{
						for( i = 0 ; i < LogData.ErTabNum ; i ++ )
							String[i] = _T( '\t' ) ;
						String[i] = _T( '\0' ) ;
						WriteFile( fp, String, LogData.ErTabNum * sizeof( TCHAR ), &WriteSize, NULL ) ;
						OutputDebugString( String ) ;
					}
				}

				// エラーログファイルに書き出す
				WriteFile( fp, ErrorStr, lstrlen( ErrorStr ) * sizeof( TCHAR ), &WriteSize, NULL ) ;

				// エラーログをアウトプットに書き出す
				OutputDebugString( ErrorStr ) ;

				// 最後の文字が改行意外だった場合はタブストップフラグを立てる
				LogData.ErTabStop = ErrorStr[ lstrlen( ErrorStr ) - 1 ] != _T( '\n' ) ;

				CloseHandle( fp ) ;
			}
//			FSETDIR( MotoPath ) ;
		}
	}
	else
	{
		// エラーログをアウトプットに書き出す
		OutputDebugString( ErrorStr ) ;
	}

	// テンポラリ領域を確保していたら開放する
	if( AllocTemp != NULL )
	{
		NS_DxFree( AllocTemp ) ;
	}
	
	// 終了
	return -1 ;
}


// ログ機能の初期化を行うかどうかを取得する
extern int IsInitializeLog( void )
{
	// ＤＸライブラリが終了状態に入っていたら初期化しない
	return NS_GetUseDDrawObj() == NULL || WinData.MainWindow == NULL ? FALSE : TRUE ;
}


}

#endif // DX_NON_LOG

