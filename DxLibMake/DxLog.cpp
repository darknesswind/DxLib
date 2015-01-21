// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ログプログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ------------------------------------------------------------------
#include "DxLog.h"

#ifndef DX_NON_LOG

#include "DxLib.h"
#include "DxFont.h"
#include "DxStatic.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

LOGDATA LogData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	int			ErrorLogInitialize_Native( void ) ;										// エラー書き出しファイルを初期化する処理の環境依存部分

#ifndef DX_NON_PRINTF_DX

// 日志输出功能函数
static	int			RefreshLogFont( void ) ;												// ログ処理で使用しているフォントを更新する
static	int			CrLog( void ) ;															// 改行処理
static	int			AddCharLog( const TCHAR *C ) ;											// 一文字入るか調べて、必要なら改行する
static	int			AddLog( const TCHAR *String ) ;											// ログ出力
static	int			ClrLog( void ) ;														// ログの初期化

#endif // DX_NON_PRINTF_DX

// プログラム --------------------------------------------------------------------

// エラー書き出しファイルを初期化する
extern int ErrorLogInitialize( void )
{
	if( ErrorLogInitialize_Native() < 0 )
		return -1 ;

	// タブ数を初期化
	LogData.ErTabNum = 0 ; 

	// タブ排出ストップフラグを倒す
	LogData.ErTabStop = FALSE ;

	// スタート時のタイムスタンプを取得
	LogData.LogStartTime = NS_GetNowCount() ;

	// 終了
	return 0 ;
}

// エラー文書を書き出す
extern int ErrorLogAddA( const char *ErrorStr )
{
#ifdef UNICODE
	wchar_t ErrorStrW[ 1024 ] ;
	MBCharToWChar( 932, ErrorStr, ( DXWCHAR * )ErrorStrW, 1024 ) ;
	return NS_ErrorLogAdd( ErrorStrW ) ;
#else
	return NS_ErrorLogAdd( ErrorStr ) ;
#endif
}

// エラー文書を書き出す
extern int ErrorLogAddW( const wchar_t *ErrorStr )
{
#ifdef UNICODE
	return NS_ErrorLogAdd( ErrorStr ) ;
#else
	char ErrorStrA[ 1024 ] ;
	WCharToMBChar( 932, ( DXWCHAR * )ErrorStr, ErrorStrA, 1024 ) ;
	return NS_ErrorLogAdd( ErrorStrA ) ;
#endif
}

// 書式付きエラー文書を書き出す
extern int ErrorLogFmtAddA( const char *FormatString , ... )
{
	va_list VaList ;
	char String[ 1024 ] ;
	
	// ログ出力用のリストをセットする
	va_start( VaList , FormatString ) ;

	// 編集後の文字列を取得する
	_VSPRINTF( String , FormatString , VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 改行文字を追加する
	_STRCAT( String , "\n" ) ;

	// ログ出力する
	return ErrorLogAddA( String ) ;
}

// 書式付きエラー文書を書き出す
extern int ErrorLogFmtAddW( const wchar_t *FormatString , ... )
{
	va_list VaList ;
	wchar_t String[ 1024 ] ;
	
	// ログ出力用のリストをセットする
	va_start( VaList , FormatString ) ;

	// 編集後の文字列を取得する
	_VSWPRINTF( ( WORD * )String, ( WORD * )FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 改行文字を追加する
	_WCSCAT( String , L"\n" ) ;

	// ログ出力する
	return ErrorLogAddW( String ) ;
}

// 書式付きエラー文書を書き出す
extern int NS_ErrorLogFmtAdd( const TCHAR *FormatString , ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	int Result ;

	// ログ出力用のリストをセットする
	va_start( VaList , FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String , FormatString , VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 改行文字を追加する
	_TSTRCAT( String , _T( "\n" ) ) ;

	// ログ出力する
	Result = DXST_ERRORLOG_ADD( String ) ;
	
	return Result ;
}


// タブ数を増やす
extern int NS_ErrorLogTabAdd( void )
{
	LogData.ErTabNum ++ ;

	return 0 ;
}

// タブ数を減らす
extern int NS_ErrorLogTabSub( void )
{
	if( LogData.ErTabNum != 0 ) LogData.ErTabNum -- ;

	return 0 ;
}

// タイムスタンプの有無を設定する
extern int NS_SetUseTimeStampFlag( int UseFlag )
{
	LogData.NonUseTimeStampFlag = UseFlag != 0 ? 0 : 1 ;

	return 0 ;
}

// 書式付きログ文字列を書き出す
extern int NS_AppLogAdd( const TCHAR *String, ... )
{
	va_list VaList ;
	TCHAR StringBuf[ 1024 ] ;

	va_start( VaList, String ) ;
	_TVSPRINTF( StringBuf, String, VaList ) ;
	va_end( VaList ) ;

	return NS_ErrorLogAdd( StringBuf ) ;
}

// エラー書き出しファイルの後始末
extern int ErrorLogTerminate( void )
{
	// 終了
	return 0 ; 
}

// ログファイル名に日付をつけるかどうかをセットする
extern int NS_SetUseDateNameLogFile( int Flag )
{
	LogData.UseLogDateName = Flag ;

	// 終了
	return 0 ;
}

// ログ出力を行うか否かのセット
extern int NS_SetOutApplicationLogValidFlag( int Flag )
{
	// フラグセット
	LogData.NotLogOutFlag = !Flag ;

	// 終了
	return 0 ;
}

// ログファイルを保存するディレクトリパスを設定する
extern int NS_SetApplicationLogSaveDirectory( const TCHAR *DirectoryPath )
{
	// パスの保存
	_TSTRCPY( LogData.LogOutDirectory, DirectoryPath ) ;

	// 終了
	return 0 ;
}










#ifndef DX_NON_PRINTF_DX

// 日志输出功能函数

// ログ機能の初期化
extern int InitializeLog( void )
{
	if( IsInitializeLog() == FALSE ) return -1 ;

	// 既に初期化されていたら何もしない
	if( LogData.LogInitializeFlag == TRUE ) return 0 ;

	// 初期化フラグを立てる
	LogData.LogInitializeFlag = TRUE ;

	// ログフォントのサイズをセット
	LogData.LogFontSize = LOG_FONTSIZE ;

	// フォントの作成
	LogData.LogFontHandle = -1 ;
	LogData.LogFontHandleLostFlag = TRUE ;

	// ログの有り無しフラグをセット
	LogData.LogDrawFlag = FALSE ;

	// ログのファイル出力有り無しフラグをセット
	LogData.LogFileOutFlag = FALSE ;

	// ログの幅をセット
	LogData.LogTabWidth = LOG_TABWIDTH ;

	// カーソルの位置をセット
	LogData.LogX = LogData.LogY = 0 ;

	// ストリングを初期化
	_MEMSET( LogData.LogString, 0, sizeof( LogData.LogString ) ) ;

	// 終了
	return 0 ;
}

// ログ処理の後始末を行う
extern int TerminateLog( void )
{
	// ログ処理が初期化されていなかったなにもしない
	if( LogData.LogInitializeFlag == FALSE ) return 0 ;

	// フォントハンドルを削除する
	if( LogData.LogFontHandleLostFlag == FALSE )
	{
		NS_DeleteFontToHandle( LogData.LogFontHandle ) ;
	}
	LogData.LogFontHandleLostFlag = TRUE ;
	LogData.LogFontHandle = -1 ;

	// 初期化フラグを倒す
	LogData.LogInitializeFlag = FALSE ;

	// 終了
	return 0 ;
}

// ログ処理で使用しているフォントを更新する
static int RefreshLogFont( void )
{
	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	if( LogData.LogFontHandleLostFlag == FALSE ) return 0 ;

	// フォントハンドルを作成する
	CREATEFONTTOHANDLE_GPARAM GParam ;
	InitCreateFontToHandleGParam( &GParam ) ;
	LogData.LogFontHandle = CreateFontToHandle_UseGParam( &GParam, NULL, LogData.LogFontSize, LOG_FONTTICK, DX_FONTTYPE_EDGE, -1, -1, FALSE, -1, FALSE ) ;

	// フォントハンドルが削除された際に立つフラグのポインタをセットする
	LogData.LogFontHandleLostFlag = FALSE ;
	NS_SetFontLostFlag( LogData.LogFontHandle, &LogData.LogFontHandleLostFlag  ) ;

	return 0 ;
}

// 改行処理
static int CrLog( void )
{
	int ScWidth, ScHeight, StrHeight ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// フォントが削除されている場合は再作成する
	RefreshLogFont() ;

	// 画面のサイズを取得
	NS_GetDrawScreenSize( &ScWidth , &ScHeight ) ;
	
	// 画面内に収まる行数を算出
	StrHeight = ScHeight / LogData.LogFontSize ;
	if( StrHeight > LOG_MAXHEIGHT ) StrHeight = LOG_MAXHEIGHT ;

	// 改行したら画面から外れてしまわないか調べ、外れるかどうかで処理を分岐
	if( StrHeight <= LogData.LogY + 1 )
	{
		// 外れる場合は一行分データを全て上げる

		// 文字列を１行分ずらす
		_MEMMOVE( LogData.LogString[0], LogData.LogString[1], sizeof( TCHAR ) * LOG_MAXLENGTH * LogData.LogY ) ;
	}
	else
	{
		// Ｙ座標をインクリメント
		LogData.LogY ++ ;
	}

	// 新しい行の文字列を初期化する
	_MEMSET( LogData.LogString[ LogData.LogY ], 0, sizeof( TCHAR ) * LOG_MAXLENGTH ) ;

	// 描画幅を初期化
	LogData.LogDrawWidth = 0 ;

	// Ｘ座標を初期化する
	LogData.LogX = 0 ;

	// 終了
	return 0 ;
}

// 一文字ログに追加する、必要なら改行する
static int AddCharLog( const TCHAR *C )
{
	int ScWidth, ScHeight ;
	int Width = 0, Length, i ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// フォントが削除されている場合は再作成する
	RefreshLogFont() ;

	// 画面のサイズを取得
	NS_GetDrawScreenSize( &ScWidth , &ScHeight ) ;

	// 新しい文字の描画幅を取得
//	Length = _mbclen( (unsigned char *)C ) ;
	if( _TMULT( *C, _GET_CODEPAGE() ) == TRUE )	Length = 2 ;
	else										Length = 1 ;

	// 文字列がバッファをオーバーしないか調べる、出る場合は改行する
	if( LogData.LogX + Length >= LOG_MAXLENGTH )
	{
		CrLog() ;
	}
	else
	{
		// 画面外に出ないか調べる、出る場合は改行する
		Width = NS_GetDrawStringWidthToHandle( C , Length , LogData.LogFontHandle ) ;
		if( Width + LogData.LogDrawWidth >= ScWidth ) 
		{
			CrLog() ;
		}
	}

	// 描画幅を加算
	LogData.LogDrawWidth += Width ;

	// 文字を追加
	for( i = 0 ; i < Length ; i ++ )
	{
		LogData.LogString[ LogData.LogY ][ LogData.LogX + i ] = C[ i ]  ;
	}
	LogData.LogString[ LogData.LogY ][ LogData.LogX + i ] = _T( '\0' ) ;

	// カーソル位置をインクリメント
	LogData.LogX += Length  ;

	// 終了
	return Length ;
}


// ログ出力
static int AddLog( const TCHAR *String )
{
	int StrLen ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// フォントが削除されている場合は再作成する
	RefreshLogFont() ;

	// 一文字づつ追加していく
	{
		int i ;
		const TCHAR *C ;

		// 文字が途切れるまで繰り返し
		StrLen = lstrlen( String ) ;
		for( i = 0 ; i < StrLen ; i ++ )
		{
			C = &String[ i ] ;

			// 次の文字のタイプによって処理を分岐
			switch( *C )
			{
			case _T( '\n' ) :
				// 改行コードの場合は改行処理を行う
				CrLog() ;
				break ;

			case _T( '\t' ) :
				{
					int j ;

					// タブの場合はタブの文字数分スペースを追加
					for( j = 0 ; j < LogData.LogTabWidth ; j ++ )
						AddCharLog( _T( " " ) )  ;
				}
				break ;

			case _T( '\a' ) : case _T( '\b' ) : case _T( '\?' ) : 
			case _T( '\f' ) : case _T( '\r' ) : case _T( '\v' ) :
				break ;

			default :
				// それ以外の場合は普通に追加
				if( AddCharLog( C ) == 2 ) i ++ ;
				break ;
			}
		}
	}

	// 終了
	return 0 ;
}

// ログの初期化
static int ClrLog( void )
{
	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// フォントが削除されている場合は再作成する
	RefreshLogFont() ;

	// ログを初期化
	_MEMSET( LogData.LogString, 0, sizeof( LogData.LogString ) ) ;

	// カーソル位置を初期化
	LogData.LogX = LogData.LogY = 0 ;
	LogData.LogDrawWidth = 0 ;

	// 終了
	return 0 ;
}

// ログ出力フラグをセットする
extern int NS_SetLogDrawOutFlag( int DrawFlag )
{
	// ログ描画出力フラグをセットする
	LogData.LogDrawFlag = DrawFlag ;

	// 終了
	return 0 ;
}

// ログ出力をするかフラグの取得
extern int NS_GetLogDrawFlag( void )
{
	return LogData.LogDrawFlag ;
}

// printfDx で画面に出力するログフォントのサイズを変更する
extern int NS_SetLogFontSize( int Size )
{
	if( Size < 0 ) Size = LOG_FONTSIZE;

	LogData.LogFontSize = Size;

	DeleteFontToHandle( LogData.LogFontHandle );
	LogData.LogFontHandle = -1;
	LogData.LogFontHandleLostFlag = TRUE;

	RefreshLogFont();

	// 終了
	return 0;
}

// ログを描画する
extern int DrawLog( void )
{
	int ScWidth , ScHeight ;
	int StrHeight , i ;

	if( LogData.LogInitializeFlag == FALSE )
		if( InitializeLog() < 0 ) return 0 ;

	// フォントが削除されている場合は再作成する
	RefreshLogFont() ;

	// 画面のサイズを取得する
	NS_GetDrawScreenSize( &ScWidth , &ScHeight ) ;
	
	// 画面にログを描画する
	StrHeight = ScHeight / LogData.LogFontSize  ;
	if( StrHeight >= LOG_MAXHEIGHT ) StrHeight = LOG_MAXHEIGHT ;
	for( i = 0 ; i < StrHeight ; i ++ )
	{
		NS_DrawStringToHandle( 0 , i * LogData.LogFontSize , LogData.LogString[ i ] , NS_GetColor( 255 , 255 , 255 ) , LogData.LogFontHandle , NS_GetColor( 0 , 0 , 0 ) ) ;
	}

	// 終了
	return 0 ;
}








// 简易屏幕输出函数
extern int NS_printfDx( const TCHAR *FormatString , ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	int Result ;

	// ログ出力用のリストをセットする
	va_start( VaList , FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String , FormatString , VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// ログ出力フラグを立てる
	LogData.LogDrawFlag = TRUE ;

	// ログ出力関数を呼ぶ
	Result = AddLog( String ) ;

	// 終了
	return Result ;
}

// 簡易画面出力をクリアする
extern int NS_clsDx( void )
{
	// ログを消す
	ClrLog() ;

	// ログ出力フラグを倒す
	LogData.LogDrawFlag = FALSE ;

	// 終了
	return 0 ;
}

#endif // DX_NON_PRINTF_DX


}

#endif // DX_NON_LOG

