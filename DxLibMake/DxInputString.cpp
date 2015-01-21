// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		文字列入力プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

#include "DxInputString.h"

#ifndef DX_NON_INPUTSTRING

// Include ------------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxGraphicsBase.h"
#include "DxFont.h"
#include "DxUseCLib.h"
#include "DxLog.h"
#include "Windows/DxWindow.h"
#include "Windows/DxWinAPI.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 文字列入力ハンドルの有効性チェック
#define KEYHCHK( HAND, KPOINT )																			\
	  ( ( ( (HAND) & DX_HANDLEERROR_MASK ) != 0 ) ||													\
		( ( (HAND) & DX_HANDLETYPE_MASK ) != DX_HANDLETYPE_MASK_KEYINPUT ) ||								\
		( ( (HAND) & DX_HANDLEINDEX_MASK ) >= MAX_INPUT_NUM ) ||										\
		( ( KPOINT = &CharBuf.InputData[ (HAND) & DX_HANDLEINDEX_MASK ] )->UseFlag == FALSE ) ||		\
		( ( (KPOINT)->ID << DX_HANDLECHECK_ADDRESS ) != ( (HAND) & DX_HANDLECHECK_MASK ) ) )


#define CURSORBRINKWAIT				(500)				// カーソル点滅の間隔

// 结构体定义 --------------------------------------------------------------------

// テーブル-----------------------------------------------------------------------

// 仮想キーコードをコントロール文字コードに変換するためのテーブル
char CtrlCode[ 10 ][ 2 ] =
{
	VK_DELETE	,	CTRL_CODE_DEL		,	// ＤＥＬキー

	VK_LEFT		,	CTRL_CODE_LEFT		,	// ←キー
	VK_RIGHT	,	CTRL_CODE_RIGHT		,	// →キー
	VK_UP		,	CTRL_CODE_UP		,	// ↑キー
	VK_DOWN		,	CTRL_CODE_DOWN		,	// ↓キー

	VK_HOME		,	CTRL_CODE_HOME		,	// ＨＯＭＥキー
	VK_END		,	CTRL_CODE_END		,	// ＥＮＤキー
	VK_PRIOR	,	CTRL_CODE_PAGE_UP	,	// ＰＡＧＥ　ＵＰキー
	VK_NEXT		,	CTRL_CODE_PAGE_DOWN	,	// ＰＡＧＥ　ＤＯＷＮキー
	0			,	0
} ;

// 入力モードテーブル
static INPUTMODE InputModeTable[] =
{
	{ IME_CMODE_CHARCODE											, _T( "コード入力" ) } ,
	{ IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE						, _T( "全角ひらがな" ) } ,
	{ IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA	, _T( "全角カタカナ" ) } ,
	{ IME_CMODE_NATIVE | IME_CMODE_KATAKANA							, _T( "半角カタカナ" ) } ,
	{ IME_CMODE_FULLSHAPE											, _T( "全角英数" ) } ,
	{ IME_CMODE_ALPHANUMERIC										, _T( "半角英数" ) } ,
	{ 0 , _T( "" ) } ,
} ;

// 内部大域変数宣言 --------------------------------------------------------------

CHARBUFFER CharBuf ;										// 文字コードバッファ

// 関数プロトタイプ宣言-----------------------------------------------------------

// 字符编码缓存操作相关
#ifndef DX_NON_KEYEX
static	int			ResetKeyInputCursorBrinkCount( void ) ;										// キー入力時のカーソル点滅処理のカウンタをリセット
static	int			SetIMEOpenState( int OpenFlag ) ;											// ＩＭＥを使用状態を変更する
static	int			KeyInputSelectAreaDelete( INPUTDATA *Input ) ;								// キー入力の選択範囲を削除する
#endif

// プログラム --------------------------------------------------------------------

// 字符编码缓存操作相关

// 文字コードバッファ関係の初期化
extern int InitializeInputCharBuf( void )
{
	int IMEUseFlag ;
	int IMEUseFlag_OSSet ;

	DXST_ERRORLOG_ADD( _T( "文字コードバッファの初期化を行います... " ) ) ; 

	// ゼロ初期化
	IMEUseFlag = CharBuf.IMEUseFlag ;
	IMEUseFlag_OSSet = CharBuf.IMEUseFlag_OSSet ;
	_MEMSET( &CharBuf, 0, sizeof( CharBuf ) ) ;
	CharBuf.IMEUseFlag = IMEUseFlag ;
	CharBuf.IMEUseFlag_OSSet = IMEUseFlag_OSSet ;

	// 色のセット
	CharBuf.StrColor1  = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor3  = NS_GetColor( 255 , 255 ,   0 ) ;
	CharBuf.StrColor2  = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor4  = NS_GetColor(   0 , 255 , 255 ) ;
	CharBuf.StrColor5  = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor6  = NS_GetColor( 255 ,   0 ,   0 ) ;
	CharBuf.StrColor7  = NS_GetColor( 255 , 255 ,  20 ) ;
	CharBuf.StrColor8  = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor9  = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor10 = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor11 = CharBuf.StrColor3;
	CharBuf.StrColor12 = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor13 = NS_GetColor( 255 , 255 , 255 ) ;
	CharBuf.StrColor14 = NS_GetColor(   0 ,   0 ,   0 ) ;
	CharBuf.StrColor15 = NS_GetColor( 128 , 128 , 128 ) ;
	CharBuf.StrColor16 = NS_GetColor(   0 ,   0 ,   0 ) ;

	CharBuf.StrColor17Enable = FALSE ;
	CharBuf.StrColor17 = NS_GetColor(   0 ,   0 ,   0 ) ;

	// 描画に使用するフォントはデフォルトフォント
	CharBuf.UseFontHandle = -1 ;

	// 入力アクティブハンドルを初期化
	CharBuf.ActiveInputHandle = -1 ;
	
	// カーソル点滅処理関係の初期化
	CharBuf.CBrinkFlag = FALSE ;
	CharBuf.CBrinkCount = 0 ;
	CharBuf.CBrinkWait = CURSORBRINKWAIT ;
	CharBuf.CBrinkDrawFlag = TRUE ;

	DXST_ERRORLOG_ADD( _T( "完了しました\n" ) ) ;

	// 終了
	return 0 ;
}


// バッファにコードをストックする
extern int NS_StockInputChar( TCHAR CharCode )
{
	// バッファが一杯の場合はなにもしない
	if( ( CharBuf.EdPoint + 1 == CharBuf.StPoint ) ||
		( CharBuf.StPoint == 0 && CharBuf.EdPoint == CHARBUFFER_SIZE ) )
	{
		return -1 ;
	}

	// バッファに文字コードを代入
	CharBuf.CharBuffer[ CharBuf.EdPoint ] = CharCode ;
	CharBuf.EdPoint ++ ;
	if( CharBuf.EdPoint == CHARBUFFER_SIZE + 1 ) CharBuf.EdPoint = 0 ;

	// 終了
	return 0 ;
}

// 文字コードバッファをクリアする
extern int NS_ClearInputCharBuf( void )
{
	while( NS_ProcessMessage() == 0 )
	{
		if( NS_GetInputChar( TRUE ) == 0 ) break ;
	} 

	// スタートポインタとエンドポインタを初期化
	CharBuf.EdPoint = CharBuf.StPoint = 0 ;

#ifndef DX_NON_KEYEX
	// IME入力の状態もリセットする
	if( WinData.MainWindow )
	{
		if( CharBuf.IMEUseFlag_OSSet == TRUE && CharBuf.IMESwitch == TRUE )
		{
			HIMC Imc;

			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( WinData.MainWindow ) ;
			if( Imc )
			{
				_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
				WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc , NI_COMPOSITIONSTR ,  CPS_CANCEL , 0  );

				WinAPIData.ImmFunc.ImmReleaseContextFunc( WinData.MainWindow , Imc );
			}
		}
	}
#endif

	// 終了
	return 0 ;
}

// 文字コードバッファに溜まったデータから１バイト分取得する
extern TCHAR NS_GetInputChar( int DeleteFlag )
{
	TCHAR RetChar ;

	// バッファに文字がなかった場合は０を返す
	if( CharBuf.EdPoint == CharBuf.StPoint ) return 0 ;

	// 返す文字コードをセット
	RetChar = CharBuf.CharBuffer[ CharBuf.StPoint ] ;

	// 有効データスタートポインタをインクリメント
	if( DeleteFlag )
	{
		CharBuf.StPoint ++ ;
		if( CharBuf.StPoint == CHARBUFFER_SIZE + 1 ) CharBuf.StPoint = 0 ;
	}

	if( RetChar == -1 ) 
	{
		DXST_ERRORLOG_ADD( _T( "文字バッファエラー\n" ) ) ;
		return ( TCHAR )-1 ;
	}

	// 文字コードを返す
	return RetChar ;
}

// 文字コードバッファに溜まったデータから１文字分取得する
extern int NS_GetOneChar( TCHAR *CharBuffer , int DeleteFlag )
{
	int Ret ;

	CharBuffer[0] = 0 ;
	CharBuffer[1] = 0 ;

	Ret = 0 ;

	// バッファに文字がなかった場合は０を返す
	if( CharBuf.EdPoint == CharBuf.StPoint ) return 0 ;

	// １文字目を取得
	CharBuffer[0] = CharBuf.CharBuffer[ CharBuf.StPoint ] ;

	Ret ++ ;

	// 有効データスタートポインタをインクリメント
	CharBuf.StPoint ++ ;
	if( CharBuf.StPoint == CHARBUFFER_SIZE + 1 ) CharBuf.StPoint = 0 ;

	// 最初の文字が全角文字の１バイト目かを取得し、そうであった場合は２バイト目を取得
	if( NS_GetCtrlCodeCmp( CharBuffer[0] ) == 0 &&
		_TMULT( *CharBuffer, _GET_CHARSET() ) == TRUE )
	{
		// バッファに文字がなかった場合は０を返す	
		if( CharBuf.EdPoint == CharBuf.StPoint )
		{
			// １バイト分バッファポインタを戻す
			if( CharBuf.StPoint == 0 )
				CharBuf.StPoint = CHARBUFFER_SIZE ;
			else
				CharBuf.StPoint -- ;
			
			return 0 ;
		}

		// ２文字目を取得
		CharBuffer[1] = CharBuf.CharBuffer[ CharBuf.StPoint ] ;

		Ret ++ ;

		// 有効データスタートポインタをインクリメント
		if( DeleteFlag )
		{
			CharBuf.StPoint ++ ;
			if( CharBuf.StPoint == CHARBUFFER_SIZE + 1 ) CharBuf.StPoint = 0 ;
		}
	}

	// もしバッファ削除フラグがっ立っていなければ１バイト分戻す
	if( !DeleteFlag )
	{
		if( CharBuf.StPoint == 0 )
			CharBuf.StPoint = CHARBUFFER_SIZE ;
		else
			CharBuf.StPoint -- ;
	}

	// 取得したバイト数を返す
	return Ret ;
}

// 文字コードバッファに溜まったデータから１バイト分取得する、バッファになにも文字コードがない場合はキーが押されるまで待つ
extern TCHAR NS_GetInputCharWait( int DeleteFlag )
{
	TCHAR RetChar = 0 ;

	// キーバッファに文字コードが溜まるまで待つ
	while( NS_ProcessMessage() == 0 )
	{
		if( ( RetChar = NS_GetInputChar( DeleteFlag ) ) != 0 ) break ;
	}

	// キーコードを返す
	return RetChar ;
}

// 文字コードバッファに溜まったデータから１文字分取得する、バッファに何も文字コードがない場合はキーが押されるまで待つ
extern int NS_GetOneCharWait( TCHAR *CharBuffer , int DeleteFlag ) 
{
	int Ret = 0 ;

	// キーバッファに文字コードが溜まるまで待つ
	while( NS_ProcessMessage() == 0 )
	{
		if( ( Ret = NS_GetOneChar( CharBuffer , DeleteFlag ) ) != 0 ) break ;
	}

	// 取得したバイト数を返す
	return Ret ;
}


#ifndef DX_NON_KEYEX

// ＩＭＥメッセージのコールバック関数
extern LRESULT IMEProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HIMC Imc ;
	static int Flag = FALSE ;

	// ＩＭＥ無視状態だった場合はなにもせず終了
/*	if( CharBuf.IMEUseFlag == FALSE )
	{
//		DefWindowProc( hWnd , message , wParam , lParam ) ;
		return 0;
	}
*/
	// 変化があったフラグセット
	CharBuf.ChangeFlag = TRUE ;

	// カーソル点滅処理のカウンタをリセット
	ResetKeyInputCursorBrinkCount() ;

	// スイッチ
	switch( message )
	{
	// ＩＭＥウインドウアクティブ状態変化
	case WM_IME_SETCONTEXT :
//		lParam &= ~ISC_SHOWUICANDIDATEWINDOW ;
		lParam = 0;
		return DefWindowProc( hWnd , message , wParam , lParam ) ;

	// ＩＭＥ入力開始メッセージ
	case WM_IME_STARTCOMPOSITION :

		CharBuf.InputPoint = 0 ;
		CharBuf.IMEInputFlag = TRUE ;

		// ＩＭＥの使用状態を得る
		if( CharBuf.IMESwitch == FALSE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA )
		{
			CharBuf.IMESwitch = TRUE ;
		}
		return 0;

	// ＩＭＥ入力終了メッセージ
	case WM_IME_ENDCOMPOSITION :
		_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
		CharBuf.IMEInputFlag = FALSE ;

		// ＩＭＥの使用状態を得る
		if( CharBuf.IMESwitch == TRUE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA )
		{
			CharBuf.IMESwitch = FALSE ;
		}
		break ;


	// ＩＭＥ文字編集変化メッセージ
	case WM_IME_COMPOSITION :
		{
//			HWND DefHwnd = ImmGetDefaultIMEWnd( WinData.MainWindow ) ;
			HWND DefHwnd = hWnd;

			// 入力コンテキストを取得
//			SendMessage( DefHwnd , WM_CLOSE , 0 , 0 ) ;
			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
//			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( hWnd ) ;

			// 編集文字変化時の処理
			if( lParam & GCS_COMPSTR )
			{
				// 編集中文字列の取得
				_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , CharBuf.InputString , CHARBUFFER_SIZE );
			}

			// 編集中文字決定時の処理
			if( lParam & GCS_RESULTSTR )
			{
				_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_RESULTSTR , CharBuf.InputString , CHARBUFFER_SIZE );
				// バッファにストック
				{
					int StrLen , i ;

					StrLen = lstrlen( CharBuf.InputString ) ;
					for( i = 0 ; i < StrLen ; i ++ )
					{
						NS_StockInputChar( CharBuf.InputString[ i ] ) ;
					}
				}

				// 編集中文字列初期化
				_MEMSET( CharBuf.InputString, 0, sizeof( CharBuf.InputString ) );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , CharBuf.InputString , CHARBUFFER_SIZE );

				// 変換候補の情報をリセット
				if( CharBuf.CandidateList )
				{
					DXFREE( CharBuf.CandidateList ) ;
					CharBuf.CandidateList = NULL ;
					CharBuf.CandidateListSize = 0 ;
				}
			}

			// カーソル位置変化時の処理
			if( lParam & GCS_CURSORPOS )
			{
				int OlgPoint = CharBuf.InputPoint ;
				CharBuf.InputPoint = NS_GetStringPoint2( CharBuf.InputString , WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_CURSORPOS , NULL , 0 ) ) ;

				// 変換候補の情報をリセット
				if( CharBuf.CandidateList && OlgPoint != CharBuf.InputPoint )
				{
					DXFREE( CharBuf.CandidateList ) ;
					CharBuf.CandidateList = NULL ;
					CharBuf.CandidateListSize = 0 ;
				}
			}

			// 文節情報変化時
			if( lParam & GCS_COMPCLAUSE )
			{
				// 文節情報の取得
				_MEMSET( CharBuf.ClauseData, 0, sizeof( CharBuf.ClauseData ) ) ;
				CharBuf.ClauseNum = WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPCLAUSE , ( void * )CharBuf.ClauseData , 1024 * sizeof( int ) ) / 4 ;
			}

			// 入力文字属性変化時
			if( lParam & GCS_COMPATTR )
			{
				_MEMSET( CharBuf.CharAttr, 0, sizeof( CharBuf.CharAttr ) ) ;
				CharBuf.CharAttrNum = WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPATTR , ( void * )CharBuf.CharAttr , 1024 * sizeof( BYTE ) ) ;
			}

			// 入力コンテキストの削除
			WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//			WinAPIData.ImmFunc.ImmReleaseContextFunc( hWnd , Imc ) ;
		}
		break;

	// ＩＭＥ状態変化メッセージ
	case WM_IME_NOTIFY :

		switch( wParam )
		{
		// ＩＭＥのＯＮ、ＯＦＦ変化
		case IMN_SETOPENSTATUS :

			if( Flag ) break ;

			Flag = TRUE ;

			DefWindowProc( hWnd , message , wParam , lParam ) ;

			// キーが押されている間ここで止まる
			{
				__int64 Time = NS_GetNowHiPerformanceCount() ;

				while( NS_ProcessMessage() == 0 && NS_GetNowHiPerformanceCount() - Time < 2000 ){}
			}

			Flag = FALSE ;

			// ＩＭＥの使用状態を得る
			{
				HWND DefHwnd = hWnd;
//				HWND DefHwnd = ImmGetDefaultIMEWnd( hWnd ) ;
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;

//				SendMessage( DefHwnd , WM_CLOSE , 0 , 0 ) ;
//				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( hWnd ) ;

				CharBuf.IMESwitch = WinAPIData.ImmFunc.ImmGetOpenStatusFunc( Imc ) ;
				WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//				WinAPIData.ImmFunc.ImmReleaseContextFunc( hWnd , Imc );
			}

			break ;


		// 候補変化時の処理
		case IMN_CHANGECANDIDATE:
		case IMN_OPENCANDIDATE :
		case IMN_SETCANDIDATEPOS :
			{
				DWORD BufSize ;
//				HWND DefHwnd = ImmGetDefaultIMEWnd( hWnd ) ;
				HWND DefHwnd = hWnd;
//				int Result ;

				// 入力コンテキストを取得
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
//				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( hWnd ) ;

				// バッファサイズ取得
//				BufSize = ImmGetCandidateListCount( Imc , &ListSize ) ;
				BufSize = WinAPIData.ImmFunc.ImmGetCandidateListFunc( Imc , 0 , NULL , 0 ) ;
				if( BufSize != 0 )
				{
					CharBuf.CandidateListSize = ( int )BufSize ;
					if( BufSize == 0 )
					{
						WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
						DXST_ERRORLOGFMT_ADD(( _T( "漢字変換候補のデータサイズが０でした  DefHWND:%x IMC:%x\n" ), DefHwnd, Imc )) ;
						return 0 ;
					}

					// バッファ用メモリの確保
					if( CharBuf.CandidateList != NULL )
							DXFREE( CharBuf.CandidateList ) ;
					if( ( CharBuf.CandidateList = ( CANDIDATELIST * )DXALLOC( BufSize ) ) == NULL )
					{
						WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
#ifndef DX_NON_LITERAL_STRING
						return DxLib_FmtError( _T( "漢字変換候補を保存するメモリの確保に失敗しました( 確保しようとしたサイズ %d バイト )" ), BufSize ) ;
#else
						return DxLib_FmtError( "" ) ;
#endif
					}

					// データの取得
					if( WinAPIData.ImmFunc.ImmGetCandidateListFunc( Imc , 0 , CharBuf.CandidateList , BufSize ) == 0 )
					{
						DXFREE( CharBuf.CandidateList ) ;
						CharBuf.CandidateList = NULL ;
						WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );

						DXST_ERRORLOGFMT_ADD(( _T( "漢字変換候補の取得に失敗しました  BufSize:%d\n" ), BufSize )) ;
						return 0 ;
					}
				}

				// 入力コンテキストの削除
				WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//				WinAPIData.ImmFunc.ImmReleaseContextFunc( hWnd , Imc );
			}
			break ;

		// 入力候補ウインドウを閉じようとしている時の処理
		case IMN_CLOSECANDIDATE :

			// メモリの解放
			if( CharBuf.CandidateList )
				DXFREE( CharBuf.CandidateList ) ;
			CharBuf.CandidateList = NULL ;
			CharBuf.CandidateListSize = 0 ;
			break ;

		default :
//			return 0;
			return DefWindowProc( hWnd , message , wParam , lParam ) ;
		}
		break;
	}

	return 0 ;
}



// 画面上に入力中の文字列を描画する
extern int NS_DrawIMEInputString( int x , int y , int SelectStringNum )
{
	int StrLen , FontSize , FontHandle ;
	int PointX , PointY ;
	int CPointX , CPointY ;
	int StrWidth , Width ;
	RECT DrawRect ;
	int Use3DFlag ;
	static TCHAR StringBuf[2048] ;

	if( CharBuf.IMEUseFlag_OSSet == FALSE || CharBuf.IMESwitch == FALSE ) return -1 ;

	// 使用するフォントのハンドルをセットする
	FontHandle = CharBuf.UseFontHandle == -1 ? NS_GetDefaultFontHandle() : CharBuf.UseFontHandle ;

	// フォントサイズを得る
	FontSize = NS_GetFontSizeToHandle( FontHandle ) ;

	// ３Ｄ有効フラグを得る
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// 描画可能領域を得る
	NS_GetDrawArea( &DrawRect ) ;

	// 入力文字列がない場合はここで終了
	if( CharBuf.InputString[ 0 ] == 0 )
	{
		// ３Ｄ有効フラグを元に戻す
		NS_SetUse3DFlag( Use3DFlag ) ;
		return 0 ;
	}

	// 描画文字列の長さを得る
	StrWidth = NS_GetDrawStringWidthToHandle( CharBuf.InputString , lstrlen( CharBuf.InputString ) , FontHandle ) ;

	// 描画位置を補正
	if( x < DrawRect.left ) x = DrawRect.left ;
	if( y < DrawRect.top  ) y = DrawRect.top ;

	// 入力文字列を描画
	{
		// 入力文字を覆う矩形の描画
		NS_DrawObtainsBox( x , y , x + StrWidth , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor3 , TRUE ) ;

		// 変換時の選択文節の強調表示
//		if( CharBuf.ClauseNum != 2 )
		{
			int i , Point ;

			for( i = 0 ; i < CharBuf.CharAttrNum ; i ++ )
			{
				if( CharBuf.CharAttr[ i ] == ATTR_TARGET_CONVERTED )
				{
					Point = NS_GetDrawStringWidthToHandle( CharBuf.InputString     , i , FontHandle ) ;
					Width = NS_GetDrawStringWidthToHandle( CharBuf.InputString + i , 1 , FontHandle ) ;

					NS_DrawObtainsBox( x + Point , y , x + Point + Width , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor6 , TRUE ) ;
				}

				if( _TMULT( CharBuf.InputString[ i ], _GET_CHARSET() ) )
					i ++ ;
			}
/*
			Point = NS_GetStringPoint( CharBuf.InputString , CharBuf.InputPoint ) ;	

			for( i = 0 ; i < CharBuf.ClauseNum - 1 && CharBuf.ClauseData[ i ] != Point ; i ++ ){}
			if( i != CharBuf.ClauseNum - 1 )
			{
				int Width ;

				Point = NS_GetDrawStringWidthToHandle( CharBuf.InputString , Point , FontHandle ) ;
				Width = CharBuf.ClauseData[ i + 1 ] - CharBuf.ClauseData[ i ] ;
				Width = NS_GetDrawStringWidthToHandle( &CharBuf.InputString[ CharBuf.ClauseData[ i ] ] , Width , FontHandle ) ;

				NS_DrawObtainsBox( x + Point , y , x + Point + Width , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor6 , TRUE ) ;
			}
*/
		}

		NS_DrawObtainsString( x , y , FontSize + FontSize / 10 * 3 , CharBuf.InputString , CharBuf.StrColor16 , CharBuf.StrColor17Enable ? CharBuf.StrColor17 : CharBuf.StrColor3 , FontHandle  ) ;
	}

	// カーソルのある位置を算出
	{
		StrLen = NS_GetStringPoint( CharBuf.InputString , CharBuf.InputPoint ) ;

		// 先頭文字位置までの幅を算出
		if( StrLen == 0 )
		{
			CPointX = 0 ;
		}
		else
		{
			// 文字幅を得る
			CPointX = NS_GetDrawStringWidthToHandle( CharBuf.InputString , StrLen , FontHandle ) ;
		}
		CPointX += x ;
		CPointY = y ;
	}

	// カーソルを描画
	{
		// 選択文字列が存在する場合のみ描画
		if( !CharBuf.CandidateList && CharBuf.CBrinkDrawFlag == TRUE )
		{
			// カーソルの描画
			NS_DrawObtainsBox( CPointX , CPointY , CPointX + 2 , CPointY + FontSize , FontSize + FontSize / 10 * 3  , CharBuf.StrColor4 , TRUE ) ;
		}
	}

	// 下線を描画
	{
		int i , StrNum , StrWidth , PointX , PointY , LineP ;

		StrNum = 0 ;
		PointX = x ;
		PointY = y ;
		LineP = FontSize / 10 ;
		for( i = 0 ; i < CharBuf.ClauseNum ; i ++ )
		{
			StrWidth = NS_GetDrawStringWidthToHandle( &CharBuf.InputString[ StrNum ] , CharBuf.ClauseData[ i ] - StrNum , FontHandle ) ;
			NS_DrawObtainsBox( PointX + LineP * 2				, PointY + FontSize + LineP , 
								PointX + StrWidth - LineP * 2	, PointY + FontSize + LineP * 2 , FontSize + FontSize / 10 * 3 , CharBuf.StrColor5 , TRUE ) ;
			PointX += StrWidth ;
			StrNum = CharBuf.ClauseData[ i ] ;
		}
	}


	// 候補リストが出ている場合はその描画
	if( CharBuf.CandidateList )
	{
		DWORD i ;
		RECT SelectRect ;
		int j ,h ;
		int MaxWidth ;
		int SelectionNumWidth ;

		// 最長選択候補を調べる
		MaxWidth = 0 ;
		j = 0 ;
		for( i = ( CharBuf.CandidateList->dwSelection / SelectStringNum ) * SelectStringNum ; j < SelectStringNum ; i ++ , j ++ )
		{
			if( i == CharBuf.CandidateList->dwCount ) break ;

			_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d:%s" ), j + 1, ( TCHAR * )( ( BYTE * )CharBuf.CandidateList + CharBuf.CandidateList->dwOffset[i] ) ) ;
			Width = NS_GetDrawStringWidthToHandle( StringBuf , lstrlen( StringBuf ) , FontHandle ) ;
			if( Width > MaxWidth ) MaxWidth = Width ;
		}
		_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d/%d" ), CharBuf.CandidateList->dwSelection + 1, CharBuf.CandidateList->dwCount ) ;
		SelectionNumWidth = NS_GetDrawStringWidthToHandle( StringBuf , lstrlen( StringBuf ) , FontHandle ) ;
		if( SelectionNumWidth > MaxWidth ) MaxWidth = SelectionNumWidth ;
		j ++ ;

		// 描画範囲をセット
		{
			h = FontSize / 3 ;
			SETRECT( SelectRect , CPointX , CPointY + FontSize + FontSize / 2 , CPointX + MaxWidth + h * 2, CPointY + FontSize * ( 1 + j ) + FontSize / 2 + h * 2 ) ;

			// インプット文字描画位置より下の位置に配置できるか検証
			if( SelectRect.bottom > DrawRect.bottom )
			{
				// 出来ない場合はインプット文字描画位置より上の位置に配置を試みる
				if( y - FontSize * j - FontSize / 2 - h * 2 < DrawRect.top )
				{	
					// それでも駄目な場合は無理やり下の位置に描画する
					j = SelectRect.bottom - DrawRect.bottom ;
				}
				else
				{
					// 上の位置を起点に配置
					j = SelectRect.top - ( y - FontSize * j - FontSize / 2 - h * 2 ) ;
				}

				// 高さを補正
				SelectRect.bottom -= j ;
				SelectRect.top -= j ;
			}

			// インプット文字描画幅のせいで描画可能領域右端に到達していた場合は補正
			if( SelectRect.right > DrawRect.right )
			{
				j = SelectRect.right - DrawRect.right ;	

				SelectRect.left -= j ;
				SelectRect.right -= j ;
			}
		}

		// 描画範囲を黒で塗りつぶし
		NS_DrawBox( SelectRect.left , SelectRect.top , SelectRect.right , SelectRect.bottom , CharBuf.StrColor12 , TRUE ) ;
		NS_DrawBox( SelectRect.left , SelectRect.top , SelectRect.right , SelectRect.bottom , CharBuf.StrColor11 , FALSE ) ;

		// 候補を描画
		PointX = SelectRect.left + h ;
		PointY = SelectRect.top + h ;
		j = 0 ;
		for( i = ( CharBuf.CandidateList->dwSelection / SelectStringNum ) * SelectStringNum ; j < SelectStringNum ; i ++ , j ++ )
		{
			if( i == CharBuf.CandidateList->dwCount ) break ;
			_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d:%s" ), j + 1, ( TCHAR * )( ( BYTE * )CharBuf.CandidateList + CharBuf.CandidateList->dwOffset[i] ) ) ;
			NS_DrawStringToHandle( PointX , PointY + j * FontSize , StringBuf,
				( i == CharBuf.CandidateList->dwSelection ) ? CharBuf.StrColor6 : CharBuf.StrColor1 , FontHandle ,
				( i == CharBuf.CandidateList->dwSelection ) ? CharBuf.StrColor9 : CharBuf.StrColor8 ) ;
		}
 
		// 候補総数の内何番目か、の情報を描画する
		_TSPRINTF( _DXWTP( StringBuf ), _DXWTR( "%d/%d" ), CharBuf.CandidateList->dwSelection + 1, CharBuf.CandidateList->dwCount ) ;
		NS_DrawStringToHandle( PointX + MaxWidth - SelectionNumWidth, PointY + j * FontSize , StringBuf, CharBuf.StrColor1 , FontHandle , CharBuf.StrColor8 ) ;

		// 候補リストの開始インデックスと候補リストのページサイズを変更
		{
			HWND DefHwnd = WinData.MainWindow;
			HIMC Imc ;

			// 入力コンテキストを取得
			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;

			// 候補リストのページサイズを変更
			WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc, NI_SETCANDIDATE_PAGESIZE, 0, SelectStringNum ) ;

			// 候補リストの開始インデックスの変更メッセージを送る
			WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc, NI_SETCANDIDATE_PAGESTART, 0, ( CharBuf.CandidateList->dwSelection / SelectStringNum ) * SelectStringNum ) ;

			// 入力コンテキストの削除
			WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
		}
	}

	// ３Ｄ有効フラグを元に戻す
	NS_SetUse3DFlag( Use3DFlag ) ;

	// 終了
	return 0 ;
}

// ＩＭＥを使用するかどうかの状態を更新する
extern void RefreshIMEFlag( int Always )
{
	int NewUseFlag ;

	NewUseFlag = CharBuf.IMEUseFlag || CharBuf.IMEUseFlag_System ;

	// フラグが以前と同じ場合は何もせず終了
	if( NewUseFlag == CharBuf.IMEUseFlag_OSSet && Always == FALSE ) return ;

	// 有効フラグをセット
	if( WinAPIData.WINNLSEnableIME_Func )
		WinAPIData.WINNLSEnableIME_Func( WinData.MainWindow , NewUseFlag ) ;

	// フラグ保存
	CharBuf.IMEUseFlag_OSSet = NewUseFlag ;

	// もしFALSEだった場合は各種メモリを解放する
	if( NewUseFlag == FALSE ) 
	{
		DXFREE( CharBuf.CandidateList ) ;
		CharBuf.CandidateList = NULL ;
		CharBuf.CandidateListSize = 0 ;

		_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE ) ;

		// 入力中か、フラグも倒す
		CharBuf.IMEInputFlag = FALSE ;
	}

	// 終了
	return ;
}

// ＩＭＥを使用するかどうかをセットする
extern int NS_SetUseIMEFlag( int UseFlag )
{
	// フラグ保存
	CharBuf.IMEUseFlag = UseFlag ;

	// 状態を更新
	RefreshIMEFlag() ;

	// 終了
	return 0 ;
}


// ＩＭＥで入力できる最大文字数を MakeKeyInput の設定に合わせるかどうかをセットする( TRUE:あわせる  FALSE:あわせない(デフォルト) )
extern int NS_SetInputStringMaxLengthIMESync( int Flag )
{
	CharBuf.IMEInputStringMaxLengthIMESync = Flag ;

	// 終了
	return 0 ;
}

// ＩＭＥで一度に入力できる最大文字数を設定する( 0:制限なし  1以上:指定の文字数で制限 )
extern int NS_SetIMEInputStringMaxLength( int Length )
{
	CharBuf.IMEInputMaxLength = Length ;

	// 終了
	return 0 ;
}

// ＩＭＥを使用状態を変更する
static int SetIMEOpenState( int OpenFlag ) 
{
	HIMC Imc ;
	HWND DefHwnd = WinData.MainWindow;

	// 入力コンテキストを取得
	Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;

	// 使用状態を変更する
	WinAPIData.ImmFunc.ImmSetOpenStatusFunc( Imc, OpenFlag == TRUE ? true : false ) ;

	// 入力コンテキストの削除
	WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );

	// 終了
	return 0 ;
}

// キー入力の選択範囲を削除する
static int KeyInputSelectAreaDelete( INPUTDATA * Input )
{
	int smin, smax, DelNum ;

	if( Input->SelectStart == -1 ) return 0 ;

	if( Input->SelectStart > Input->SelectEnd )
	{
		smin = Input->SelectEnd ;
		smax = Input->SelectStart ;
	}
	else
	{
		smin = Input->SelectStart ;
		smax = Input->SelectEnd ;
	}
	DelNum = smax - smin ;
	Input->Point = smin ;

	if( smax != Input->StrLength )
	{
		_MEMMOVE( &Input->Buffer[ smin ], &Input->Buffer[ smax ], ( Input->StrLength - smax ) * sizeof( TCHAR ) ) ;
	}
	Input->StrLength -= DelNum ;
	Input->Buffer[ Input->StrLength ] = _T( '\0' ) ;
	Input->SelectStart = -1 ;
	Input->SelectEnd = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// 終了
	return 0 ;
}


#endif

// 全角文字、半角文字入り乱れる中から指定の全半混在文字数での半角文字数を得る
extern int NS_GetStringPoint( const TCHAR *String , int Point )
{
	int i , p ;

	p = 0 ;
	for( i = 0 ; i < Point ; i ++ )
	{
		if( *( String + p ) == _T( '\0' ) ) break ; 
		if( _TMULT( String[p], _GET_CHARSET() ) == FALSE )	p ++ ;
		else												p += 2 ;
	}

	return p ;
}

// 全角文字、半角文字入り乱れる中から指定の半角文字数での全角文字数を得る
extern int NS_GetStringPoint2( const TCHAR *String , int Point )
{
	int i , p ;

	p = 0 ;
	i = 0 ;
	while( i < Point )
	{
		if( *( String + i ) == _T( '\0' ) ) break ; 
		if( _TMULT( String[i], _GET_CHARSET() ) == FALSE )	i ++ ;
		else												i += 2 ;

		p ++ ;
	}

	return p ;
}

// 全角文字、半角文字入り乱れる中から文字数を取得する
extern int NS_GetStringLength( const TCHAR *String )
{
	int i , p ;

	p = 0 ;
	i = 0 ;
	while( *( String + i ) != _T( '\0' ) )
	{
		if( _TMULT( String[i], _GET_CHARSET() ) == FALSE )	i ++ ;
		else												i += 2 ;

		p ++ ;
	}

	return p ;
}

#ifndef DX_NON_FONT

// 規定領域に収めたかたちで文字列を描画
extern int NS_DrawObtainsString(
	int x , int y ,
	int AddY ,
	const TCHAR *String ,
	int StrColor ,
	int StrEdgeColor ,
	int FontHandle,
	int SelectBackColor ,
	int SelectStrColor ,
	int SelectStrEdgeColor ,
	int SelectStart ,
	int SelectEnd )
{
	int PointX , PointY ;
	int Width , StrWidth ;
	int Use3DFlag ;
	RECT DrawRect , Rect ;

	// FontHandle が -1 の場合はデフォルトのフォントを使用する
	if( FontHandle == -1 ) FontHandle = NS_GetDefaultFontHandle() ;

	// ３Ｄ有効フラグを得る
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// 描画可能領域を得る
	NS_GetDrawArea( &DrawRect ) ;

	// 描画位置を補正
	if( x < DrawRect.left ) x = DrawRect.left ;
	if( y < DrawRect.top  ) y = DrawRect.top ;

	while( x > DrawRect.right )
	{
		x -= DrawRect.right - DrawRect.left ;
		y += AddY ;
	}

	// 入力文字列を描画
	if( SelectStart == -1 )
	{
		// 描画文字列の長さを得る
		StrWidth = NS_GetDrawStringWidthToHandle( String , lstrlen( String ) , FontHandle ) ;

		Width = StrWidth ;
		PointX = x ;
		PointY = y ;
		for(;;)
		{
			NS_DrawStringToHandle( PointX , PointY , String , StrColor , FontHandle , StrEdgeColor ) ;

			SETRECT( Rect , PointX , PointY , PointX + StrWidth , PointY + AddY ) ;
			RectClipping( &Rect , &DrawRect ) ;

			Width -= Rect.right - Rect.left ;

			if( Width > 0 )
			{
				PointX = DrawRect.left - ( StrWidth - Width ) ;
				PointY += AddY ;
			}
			else break ;
		}
	}
	else
	{
		int i, Num, StrLen, smin, smax ;
		TCHAR TempBuf[ 3 ] ;

		if( SelectStart < SelectEnd )
		{
			smax = SelectEnd ;
			smin = SelectStart ;
		}
		else
		{
			smax = SelectStart ;
			smin = SelectEnd ;
		}

		StrLen = lstrlen( String ) ;
		PointX = x ;
		PointY = y ;
		for( i = 0 ; i < StrLen ; )
		{
			if( _TMULT( String[ i ], _GET_CHARSET() ) )
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = String[ i + 1 ] ;
				TempBuf[ 2 ] = 0 ;
				Num = 2 ;
			}
			else
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = 0 ;
				Num = 1 ;
			}

			StrWidth = NS_GetDrawStringWidthToHandle( TempBuf, lstrlen( TempBuf ), FontHandle ) ;
			if( i >= smin && i < smax )
			{
				NS_DrawBox( PointX, PointY, PointX + StrWidth, PointY + NS_GetFontSizeToHandle( FontHandle ), SelectBackColor, TRUE ) ; 
				NS_DrawStringToHandle( PointX, PointY, TempBuf, SelectStrColor, FontHandle, SelectStrEdgeColor ) ;
			}
			else
			{
				NS_DrawStringToHandle( PointX, PointY, TempBuf, StrColor, FontHandle, StrEdgeColor ) ;
			}

			if( PointX + StrWidth > DrawRect.right )
			{
				PointX = DrawRect.left - ( DrawRect.right - PointX ) ;
				PointY += AddY ;
			}
			else
			{
				PointX += StrWidth ;
				i += Num ;
			}
		}
	}

	// ３Ｄ描画フラグを元に戻す
	NS_SetUse3DFlag( Use3DFlag ) ;

	// 終了
	return 0 ;
}

// 描画可能領域に収まるように改行しながら文字列を描画( クリップが文字単位 )
extern int NS_DrawObtainsString_CharClip(
	int x, int y,
	int AddY,
	const TCHAR *String,
	int StrColor,
	int StrEdgeColor,
	int FontHandle,
	int SelectBackColor,
	int SelectStrColor,
	int SelectStrEdgeColor,
	int SelectStart,
	int SelectEnd
)
{
	int PointX , PointY ;
	int StrWidth ;
	int Use3DFlag ;
	RECT DrawRect ;
	int i, Num, StrLen ;
	TCHAR TempBuf[ 3 ] ;

	// FontHandle が -1 の場合はデフォルトのフォントを使用する
	if( FontHandle == -1 ) FontHandle = NS_GetDefaultFontHandle() ;

	// ３Ｄ有効フラグを得る
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// 描画可能領域を得る
	NS_GetDrawArea( &DrawRect ) ;

	// 描画位置を補正
	if( x < DrawRect.left ) x = DrawRect.left ;
	if( y < DrawRect.top  ) y = DrawRect.top ;

	while( x > DrawRect.right )
	{
		x -= DrawRect.right - DrawRect.left ;
		y += AddY ;
	}

	StrLen = lstrlen( String ) ;

	// 入力文字列を描画
	if( SelectStart == -1 )
	{
		PointX = x ;
		PointY = y ;
		for( i = 0 ; i < StrLen ; )
		{
			if( _TMULT( String[ i ], _GET_CHARSET() ) )
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = String[ i + 1 ] ;
				TempBuf[ 2 ] = 0 ;
				Num = 2 ;
			}
			else
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = 0 ;
				Num = 1 ;
			}

			StrWidth = NS_GetDrawStringWidthToHandle( TempBuf, Num, FontHandle ) ;
			if( PointX + StrWidth > DrawRect.right )
			{
				PointX = x ;
				PointY += AddY ;
			}

			NS_DrawStringToHandle( PointX, PointY, TempBuf, StrColor, FontHandle, StrEdgeColor ) ;

			PointX += StrWidth ;
			i += Num ;
		}
	}
	else
	{
		int smin, smax ;

		if( SelectStart < SelectEnd )
		{
			smax = SelectEnd ;
			smin = SelectStart ;
		}
		else
		{
			smax = SelectStart ;
			smin = SelectEnd ;
		}

		PointX = x ;
		PointY = y ;
		for( i = 0 ; i < StrLen ; )
		{
			if( _TMULT( String[ i ], _GET_CHARSET() ) )
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = String[ i + 1 ] ;
				TempBuf[ 2 ] = 0 ;
				Num = 2 ;
			}
			else
			{
				TempBuf[ 0 ] = String[ i ] ;
				TempBuf[ 1 ] = 0 ;
				Num = 1 ;
			}

			StrWidth = NS_GetDrawStringWidthToHandle( TempBuf, Num, FontHandle ) ;
			if( PointX + StrWidth > DrawRect.right )
			{
				PointX = x ;
				PointY += AddY ;
			}

			if( i >= smin && i < smax )
			{
				NS_DrawBox( PointX, PointY, PointX + StrWidth, PointY + NS_GetFontSizeToHandle( FontHandle ), SelectBackColor, TRUE ) ; 
				NS_DrawStringToHandle( PointX, PointY, TempBuf, SelectStrColor, FontHandle, SelectStrEdgeColor ) ;
			}
			else
			{
				NS_DrawStringToHandle( PointX, PointY, TempBuf, StrColor, FontHandle, StrEdgeColor ) ;
			}

			PointX += StrWidth ;
			i += Num ;
		}
	}

	// ３Ｄ描画フラグを元に戻す
	NS_SetUse3DFlag( Use3DFlag ) ;

	// 終了
	return 0 ;
}

#endif // DX_NON_FONT

// 規定領域に収めたかたちで矩形を描画 
extern int NS_DrawObtainsBox( int x1 , int y1 , int x2 , int y2 , int AddY , int Color , int FillFlag )
{
	int AllWidth , PointX , PointY ;
	int Width , Height ;
	int Use3DFlag ;
	RECT DrawRect , Rect ;

	// ３Ｄ有効フラグを得る
	Use3DFlag = NS_GetUse3DFlag() ;
	NS_SetUse3DFlag( FALSE ) ;

	// 描画する長さと高さを得る
	AllWidth = x2 - x1 ;
	Height = y2 - y1 ;

	// 描画可能領域を得る
	NS_GetDrawArea( &DrawRect ) ;

	// 描画位置を補正
	if( x1 < DrawRect.left ){ x2 += DrawRect.left - x1 ; x1 = DrawRect.left ; }
	if( y1 < DrawRect.top  ){ y2 += DrawRect.top  - y1 ; y1 = DrawRect.top  ; }

	while( x1 > DrawRect.right )
	{
		x1 -= DrawRect.right - DrawRect.left ;
		y1 += AddY ;
	}

	// 入力文字列を描画
	Width = AllWidth ;
	PointX = x1 ;
	PointY = y1 ;
	for(;;)
	{
		NS_DrawBox( PointX , PointY , PointX + AllWidth , PointY + Height , Color , FillFlag ) ;

		SETRECT( Rect , PointX , PointY , PointX + AllWidth , PointY + AddY ) ;
		RectClipping( &Rect , &DrawRect ) ;

		Width -= Rect.right - Rect.left ;

		if( Width > 0 )
		{
			PointX = DrawRect.left - ( AllWidth - Width ) ;
			PointY += AddY ;
		}
		else break ;
	}

	// ３Ｄ描画フラグを元に戻す
	NS_SetUse3DFlag( Use3DFlag ) ;

	// 終了
	return 0 ;
}



// アスキーコントロールコードか調べる
extern int NS_GetCtrlCodeCmp( TCHAR Char ) 
{
	int i ;

	for( i = 0 ; CtrlCode[ i ][ 0 ] != 0  &&  ( TCHAR )CtrlCode[ i ][ 1 ] != Char ; i ++ ){}
	
	// 結果を返す
	return CtrlCode[ i ][ 0 ] != 0 ;
}

// ＩＭＥに変化があったか、フラグの取得
extern int GetIMEChangeFlag( void )
{
	int Flag = CharBuf.ChangeFlag ;

	CharBuf.ChangeFlag = FALSE ;
	return Flag ;
}



#ifndef DX_NON_KEYEX

// 文字列の入力取得
extern int NS_InputStringToCustom( int x , int y , int BufLength , TCHAR *StrBuffer , int CancelValidFlag , int SingleCharOnlyFlag , int NumCharOnlyFlag, int DoubleCharOnlyFlag )
{
	int DrawScreen ;
	int ScreenGraphFront, ScreenGraphBack, Use3DFlag , UseSysMemFlag ;
	int EndFlag = 0 ;
	RECT DrawRect ;
	int FontSize ;
	int DrawWidth , DrawHeight ;
//	TCHAR *Buffer ;
	int Red , Green , Blue ;
	int InputHandle ;
	int ScreenWidth, ScreenHeight ;
	MATRIX ViewMatrix, ProjectionMatrix, ViewportMatrix ;
	MATRIX WorldMatrix ;
	int ProjectionMatrixMode ;
	float ProjFov ;
	float ProjSize ;
	float ProjNear ;
	float ProjFar ;
	float ProjDotAspect ;
	MATRIX ProjMatrix ;

	// 各種データ保存
	Use3DFlag = GetUse3DFlag() ;
	UseSysMemFlag = GetUseSystemMemGraphCreateFlag() ;
	DrawScreen = GetActiveGraph() ;
	GetTransColor( &Red , &Green , &Blue ) ;
	ViewMatrix           = GBASE.ViewMatrix ;
	ProjectionMatrixMode = GBASE.ProjectionMatrixMode ;
	ProjFov              = GBASE.ProjFov ;
	ProjSize             = GBASE.ProjSize ;
	ProjMatrix           = GBASE.ProjMatrix ;
	ProjNear             = GBASE.ProjNear ;
	ProjFar              = GBASE.ProjFar ;
	ProjDotAspect        = GBASE.ProjDotAspect ;
	ProjectionMatrix     = GBASE.ProjectionMatrix ;
	ViewportMatrix       = GBASE.ViewportMatrix ;
	WorldMatrix          = GBASE.WorldMatrix ;
	
	// 画面の大きさを得る
	GetDrawScreenSize( &ScreenWidth, &ScreenHeight ) ;

	// 描画領域を得る
	GetDrawArea( &DrawRect ) ;

	// ＶＲＡＭの使用、３Ｄ描画機能の使用変更
	SetUseSystemMemGraphCreateFlag( CheckFontChacheToTextureFlag( GetDefaultFontHandle() ) == TRUE ? FALSE : TRUE ) ;
	SetUse3DFlag( FALSE ) ;

	// 透過色を真っ黒にする
	SetTransColor( 0 , 0 , 0 ) ;

	// ＩＭＥを有効にする
	CharBuf.IMEUseFlag_System = TRUE ;
	RefreshIMEFlag() ;

	// 画面を保存するためのメモリを確保
	DrawWidth = DrawRect.right - DrawRect.left ;
	DrawHeight = DrawRect.bottom - DrawRect.top ; 
	ScreenGraphFront = MakeGraph( ScreenWidth, ScreenHeight ) ;
	ScreenGraphBack = MakeGraph( ScreenWidth, ScreenHeight ) ;
	SetDrawScreen( DX_SCREEN_FRONT ) ;
	SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;
	GetDrawScreenGraph( 0, 0, ScreenWidth, ScreenHeight, ScreenGraphFront ) ;
	SetDrawScreen( DX_SCREEN_BACK ) ;
	SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;
	GetDrawScreenGraph( 0, 0, ScreenWidth, ScreenHeight, ScreenGraphBack ) ;

	// 文字入力用バッファの確保
//	if( ( Buffer = ( TCHAR * )DXCALLOC( BufLength + 1 ) ) == NULL ) return -1 ;

	// 入力ハンドルを得る
	if( ( InputHandle = MakeKeyInput( BufLength , CancelValidFlag , SingleCharOnlyFlag , NumCharOnlyFlag, DoubleCharOnlyFlag ) ) != -1 )
	{
		// 入力をアクティブにする
		SetActiveKeyInput( InputHandle ) ;

		// 入力文字数を０にする
		FontSize = GetFontSize() ;

		// 座標の補正
//		x -= DrawRect.left ;
//		y -= DrawRect.top ;

		// １回描画
		DrawKeyInputString( x , y , InputHandle ) ;

		// 描画先を裏画面に
		SetDrawScreen( DX_SCREEN_BACK ) ;
		while( ProcessMessage() == 0 )
		{
			// 入力が終了している場合は終了
			EndFlag = CheckKeyInput( InputHandle ) ;
			if( EndFlag ) break ;

			// 入力文字の更新
			{
				// もと画面の描画
				DrawGraph( 0 , 0 , ScreenGraphFront , FALSE ) ;

				// ＩＭＥ入力モードの描画
				DrawKeyInputModeString( DrawRect.right , DrawRect.bottom ) ; 

				// 入力状態の描画
				SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;
				DrawKeyInputString( x , y , InputHandle ) ;
				SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;

				// 表画面に転送
				ScreenFlip() ;
			}
		}
	}

	// ウインドウが閉じられた場合は－１を返す
	if( ProcessMessage() == -1 ) return -1;

	// キャンセルされていない場合は文字列を取得
	if( EndFlag == 1 )
	{
		GetKeyInputString( StrBuffer , InputHandle ) ;
	}

	// 画面を元にもどす
	SetDrawArea( 0, 0, ScreenWidth, ScreenHeight ) ;
	DrawGraph( 0, 0, ScreenGraphBack, FALSE ) ;
	SetDrawScreen( DX_SCREEN_FRONT ) ;
	DrawGraph( 0, 0, ScreenGraphFront, FALSE ) ;

	// グラフィックデータを消す
	DeleteKeyInput( InputHandle ) ;
	DeleteGraph( ScreenGraphFront ) ;
	DeleteGraph( ScreenGraphBack ) ;

	// データを元に戻す
	SetDrawScreen( DrawScreen ) ;
	SetUse3DFlag( Use3DFlag ) ;
	SetUseSystemMemGraphCreateFlag( UseSysMemFlag ) ;
	SetTransColor( Red , Green , Blue ) ;
	SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;

	SetTransformToWorld( &WorldMatrix );
	SetTransformToView( &ViewMatrix );
	switch( ProjectionMatrixMode )
	{
	case 0 :	// 遠近法
		SetupCamera_Perspective( ProjFov ) ;
		break ;

	case 1 :	// 正射影
		SetupCamera_Ortho( ProjSize ) ;
		break ;

	case 2 :	// 行列直指定
		SetupCamera_ProjectionMatrix( ProjMatrix ) ;
		break ;
	}
	SetCameraNearFar( ProjNear, ProjFar ) ;
	SetCameraDotAspect( ProjDotAspect ) ;
//	SetTransformToProjection( &ProjectionMatrix );
	SetTransformToViewport( &ViewportMatrix );

	// 終了
	return EndFlag ;
}


// 文字列の入力取得
extern int NS_KeyInputString( int x , int y , int CharMaxLength , TCHAR *StrBuffer , int CancelValidFlag )
{
	return NS_InputStringToCustom( x , y , CharMaxLength , StrBuffer , CancelValidFlag , FALSE , FALSE, FALSE ) ;
}

// 半角文字列のみの入力取得
extern int NS_KeyInputSingleCharString( int x , int y , int CharMaxLength , TCHAR *StrBuffer , int CancelValidFlag )
{
	return NS_InputStringToCustom( x , y, CharMaxLength , StrBuffer , CancelValidFlag , TRUE , FALSE, FALSE ) ;
}

// 数値の入力
extern int NS_KeyInputNumber( int x , int y , int MaxNum , int MinNum , int CancelValidFlag )
{
	int Num ;
	TCHAR Buffer[ 50 ] ;
	int StrLen , hr ;

	// 最大文字列幅を取得
	{
		int MaxLen , MinLen ;

		_TSPRINTF( _DXWTP( Buffer ), _DXWTR( "%d" ), MaxNum ) ;		MaxLen = lstrlen( Buffer ) ;
		_TSPRINTF( _DXWTP( Buffer ), _DXWTR( "%d" ), MinNum ) ;		MinLen = lstrlen( Buffer ) ;

		StrLen = MaxLen > MinLen ? MaxLen : MinLen ;
	}

	// 数字を取得
	hr = InputStringToCustom( x , y , StrLen , Buffer , CancelValidFlag , TRUE , TRUE ) ;
	if( hr == -1 ) return MinNum - 1 ;
	if( hr == 2 ) return MaxNum + 1 ;

	// 数値に変換
	Num = _TTOI( _DXWTP( Buffer ) ) ;

	// 規定範囲の中に取得した数値が当てはまらない間は繰り返す
	while( ProcessMessage() == 0 && ( Num > MaxNum || Num < MinNum ) )
	{
		hr = InputStringToCustom( x , y , StrLen , Buffer , CancelValidFlag , TRUE , TRUE ) ;
		if( hr == -1 ) return MinNum - 1 ;
		if( hr == 2 ) return MaxNum + 1 ;

		// 数値に変換
		Num = _TTOI( _DXWTP( Buffer ) ) ;
	}

	// 終了
	return Num ;
}

// IMEの入力モード文字列の取得
extern int NS_GetIMEInputModeStr( TCHAR *GetBuffer ) 
{
	HIMC	Imc ;
	DWORD	InputState , SentenceState , Buf ;
	TCHAR	*SelectStr ;
//	int		i ;
/*	DWORD	InputModePal[] =	{
IME_CMODE_ALPHANUMERIC , IME_CMODE_NATIVE , IME_CMODE_KATAKANA ,IME_CMODE_LANGUAGE , 
IME_CMODE_FULLSHAPE ,IME_CMODE_ROMAN , IME_CMODE_CHARCODE , IME_CMODE_HANJACONVERT , 
IME_CMODE_SOFTKBD , IME_CMODE_NOCONVERSION,IME_CMODE_EUDC , IME_CMODE_SYMBOL , 
IME_CMODE_FIXED  	} ;
	int	
IME_CMODE_ALPHANUMERICp ,IME_CMODE_NATIVEp , IME_CMODE_KATAKANAp,IME_CMODE_LANGUAGEp ,
IME_CMODE_FULLSHAPEp ,IME_CMODE_ROMANp ,IME_CMODE_CHARCODEp, IME_CMODE_HANJACONVERTp, 
IME_CMODE_SOFTKBDp ,IME_CMODE_NOCONVERSIONp,IME_CMODE_EUDCp ,IME_CMODE_SYMBOLp, 
IME_CMODE_FIXEDp ;
*/
	// ＩＭＥが使われていないときは-1を返す
	if( CharBuf.IMESwitch == FALSE ) return -1 ;

	// 入力コンテキストを取得
	HWND DefHwnd = WinData.MainWindow;
	Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
//	Imc = WinAPIData.ImmFunc.ImmGetContextFunc( WinData.MainWindow ) ;

	// 入力モードを得る
	WinAPIData.ImmFunc.ImmGetConversionStatusFunc( Imc , &InputState , &SentenceState ) ;

/*	IME_CMODE_ALPHANUMERICp = ( ( InputState & IME_CMODE_ALPHANUMERIC ) == IME_CMODE_ALPHANUMERIC ) ;
	IME_CMODE_NATIVEp = ( ( InputState & IME_CMODE_NATIVE ) == IME_CMODE_NATIVE ) ;
	IME_CMODE_KATAKANAp = ( ( InputState & IME_CMODE_KATAKANA ) == IME_CMODE_KATAKANA ) ;
	IME_CMODE_LANGUAGEp = ( ( InputState & IME_CMODE_LANGUAGE ) == IME_CMODE_LANGUAGE ) ;
	IME_CMODE_FULLSHAPEp = ( ( InputState & IME_CMODE_FULLSHAPE ) == IME_CMODE_FULLSHAPE ) ;
	IME_CMODE_ROMANp = ( ( InputState & IME_CMODE_ROMAN ) == IME_CMODE_ROMAN ) ;
	IME_CMODE_CHARCODEp = ( ( InputState & IME_CMODE_CHARCODE ) == IME_CMODE_CHARCODE ) ;
	IME_CMODE_HANJACONVERTp = ( ( InputState & IME_CMODE_HANJACONVERT ) == IME_CMODE_HANJACONVERT ) ;
	IME_CMODE_SOFTKBDp = ( ( InputState & IME_CMODE_SOFTKBD ) == IME_CMODE_SOFTKBD ) ;
	IME_CMODE_NOCONVERSIONp = ( ( InputState & IME_CMODE_NOCONVERSION ) == IME_CMODE_NOCONVERSION ) ;
	IME_CMODE_EUDCp = ( ( InputState & IME_CMODE_EUDC ) == IME_CMODE_EUDC ) ;
	IME_CMODE_SYMBOLp = ( ( InputState & IME_CMODE_SYMBOL ) == IME_CMODE_SYMBOL ) ;
	IME_CMODE_FIXEDp = ( ( InputState & IME_CMODE_FIXED ) == IME_CMODE_FIXED ) ;
*/
	// 判断
	Buf = InputState & ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA ) ;
	if( Buf == ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE | IME_CMODE_KATAKANA ) )
	{
		SelectStr = _T( "全角カタカナ" ) ;
	}
	else
	{
		Buf = InputState & ( IME_CMODE_NATIVE | IME_CMODE_KATAKANA ) ;
		if( Buf == ( IME_CMODE_NATIVE | IME_CMODE_KATAKANA ) )
		{
			SelectStr = _T( "半角カタカナ" ) ;
		}
		else
		{
			Buf = InputState & ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE ) ;
			if( Buf == ( IME_CMODE_NATIVE | IME_CMODE_FULLSHAPE ) )
			{
				SelectStr = _T( "全角ひらがな" ) ;
			}
			else
			{
				Buf = InputState & IME_CMODE_FULLSHAPE ;
				if( Buf == IME_CMODE_FULLSHAPE )
				{
					SelectStr = _T( "全角英数" ) ;
				}
				else
				{
					Buf = InputState & IME_CMODE_ALPHANUMERIC ;
					if( Buf == IME_CMODE_ALPHANUMERIC )
					{
						SelectStr = _T( "半角英数" ) ;
					}
					else
					{
						SelectStr = _T( "不明な入力モード" ) ;
					}
				}
			}
		}
	}

	lstrcpy( GetBuffer , SelectStr ) ;

/*
	// 判断
	for( i = 0 ; InputModeTable[ i ].InputState != 0 && InputModeTable[ i ].InputState != InputState ; i ++ ){}
	if( InputModeTable[ i ].InputState == 0 )
	{
		_STRCPY( GetBuffer , "不明な入力モード" ) ;
	}
	else
	{
		_STRCPY( GetBuffer , InputModeTable[ i ].InputName ) ;
	}
*/
	// 入力コンテキストの削除
	WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
//	WinAPIData.ImmFunc.ImmReleaseContextFunc( WinData.MainWindow , Imc );

	// 終了
	return 0 ;
}

// InputString関数使用時の文字の各色を変更する
extern int NS_SetKeyInputStringColor(
	ULONGLONG NmlStr ,				// 入力文字列の色
	ULONGLONG NmlCur ,				// ＩＭＥ非使用時のカーソルの色
	ULONGLONG IMEStrBack ,			// ＩＭＥ使用時の入力文字列の背景の色
	ULONGLONG IMECur ,				// ＩＭＥ使用時のカーソルの色
	ULONGLONG IMELine ,				// ＩＭＥ使用時の変換文字列の下線
	ULONGLONG IMESelectStr ,		// ＩＭＥ使用時の選択対象の変換候補文字列の色
	ULONGLONG IMEModeStr  ,			// ＩＭＥ使用時の入力モード文字列の色(「全角ひらがな」等)
	ULONGLONG NmlStrE ,				// 入力文字列の縁の色
	ULONGLONG IMESelectStrE ,		// ＩＭＥ使用時の選択対象の変換候補文字列の縁の色
	ULONGLONG IMEModeStrE	,		// ＩＭＥ使用時の入力モード文字列の縁の色
	ULONGLONG IMESelectWinE ,		// ＩＭＥ使用時の変換候補ウインドウの縁の色
	ULONGLONG IMESelectWinF	,		// ＩＭＥ使用時の変換候補ウインドウの下地の色
	ULONGLONG SelectStrBackColor ,	// 選択された入力文字列の背景の色
	ULONGLONG SelectStrColor ,		// 選択された入力文字列の色
	ULONGLONG SelectStrEdgeColor,	// 選択された入力文字列の縁の色
	ULONGLONG IMEStr,				// ＩＭＥ使用時の入力文字列の色
	ULONGLONG IMEStrE				// ＩＭＥ使用時の入力文字列の縁の色
	)
{
	if( NmlStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor1) = *((DWORD *)&NmlStr) ;
	}

	if( NmlCur != -1 )
	{
		*((DWORD *)&CharBuf.StrColor2) = *((DWORD *)&NmlCur) ;
	}

	if( IMEStrBack != -1 )
	{
		*((DWORD *)&CharBuf.StrColor3) = *((DWORD *)&IMEStrBack) ;
	}

	if( IMECur != -1 )
	{
		*((DWORD *)&CharBuf.StrColor4) = *((DWORD *)&IMECur) ;
	}

	if( IMELine != -1 )
	{
		*((DWORD *)&CharBuf.StrColor5) = *((DWORD *)&IMELine) ;
	}

	if( IMESelectStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor6) = *((DWORD *)&IMESelectStr) ;
	}

	if( IMEModeStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor7) = *((DWORD *)&IMEModeStr) ;
	}

	if( NmlStrE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor8) = *((DWORD *)&NmlStrE) ;
	}

	if( IMESelectStrE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor9) = *((DWORD *)&IMESelectStrE) ;
	}

	if( IMEModeStrE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor10) = *((DWORD *)&IMEModeStrE) ;
	}

	if( IMESelectWinE != -1 )
	{
		*((DWORD *)&CharBuf.StrColor11) = *((DWORD *)&IMESelectWinE) ;
	}
	else
	{
		if( IMEStrBack != -1 )
			CharBuf.StrColor11 = CharBuf.StrColor3;
	}

	if( IMESelectWinF != -1 )
	{
		*((DWORD *)&CharBuf.StrColor12) = *((DWORD *)&IMESelectWinF) ;
	}
	else
	{
		CharBuf.StrColor12 = NS_GetColor( 0, 0, 0 );
	}

	if( IMESelectWinF != -1 )
	{
		*((DWORD *)&CharBuf.StrColor12) = *((DWORD *)&IMESelectWinF) ;
	}
	else
	{
		CharBuf.StrColor12 = NS_GetColor( 0, 0, 0 );
	}

	if( SelectStrBackColor != -1 )
	{
		*((DWORD *)&CharBuf.StrColor13) = *((DWORD *)&SelectStrBackColor) ;
	}
	else
	{
		CharBuf.StrColor13 = NS_GetColor( 255, 255, 255 );
	}

	if( SelectStrColor != -1 )
	{
		*((DWORD *)&CharBuf.StrColor14) = *((DWORD *)&SelectStrColor) ;
	}
	else
	{
		CharBuf.StrColor14 = NS_GetColor( 0, 0, 0 );
	}

	if( SelectStrEdgeColor != -1 )
	{
		*((DWORD *)&CharBuf.StrColor15) = *((DWORD *)&SelectStrEdgeColor) ;
	}
	else
	{
		CharBuf.StrColor15 = NS_GetColor( 128, 128, 128 );
	}

	if( IMEStr != -1 )
	{
		*((DWORD *)&CharBuf.StrColor16) = *((DWORD *)&IMEStr) ;
	}

	if( IMEStrE != -1 )
	{
		CharBuf.StrColor17Enable = TRUE ;
		*((DWORD *)&CharBuf.StrColor17) = *((DWORD *)&IMEStrE) ;
	}

	// 終了
	return 0 ;
}

// キー入力文字列描画関連で使用するフォントのハンドルを変更する(-1でデフォルトのフォントハンドル)
extern int NS_SetKeyInputStringFont( int FontHandle )
{
	CharBuf.UseFontHandle = FontHandle ;

	// 終了
	return 0 ;
}

// 入力モード文字列を描画する
extern int NS_DrawKeyInputModeString( int x , int y )
{
	RECT DrawRect ;
	int FontSize, FontHandle ;

	FontHandle = CharBuf.UseFontHandle == -1 ? NS_GetDefaultFontHandle() : CharBuf.UseFontHandle ;
	FontSize = NS_GetFontSizeToHandle( FontHandle ) ;
	NS_GetDrawArea( &DrawRect ) ;

	// 入力モードを描画
	if( CharBuf.IMESwitch )
	{
		TCHAR InputModeStr[ 20 ] ;
		int Width ;

		// 文字列取得
		if( NS_GetIMEInputModeStr( InputModeStr ) != -1 )
		{
			// 幅取得
			Width = NS_GetDrawStringWidthToHandle( InputModeStr , lstrlen( InputModeStr ) , FontHandle ) ;

			if( Width    + x > DrawRect.right  ) x = DrawRect.right  - Width ;
			if( FontSize + y > DrawRect.bottom ) y = DrawRect.bottom - ( FontSize + 2 ) ;
			if( x < DrawRect.left ) x = DrawRect.left ;
			if( y < DrawRect.top  ) y = DrawRect.top ;

			// 描画
			NS_DrawStringToHandle( x , y , InputModeStr , CharBuf.StrColor7 , FontHandle , CharBuf.StrColor10 )  ;
		}
	}

	// 終了
	return 0 ;
}

// キー入力データ初期化
extern int NS_InitKeyInput( void )
{
	int i ;
	INPUTDATA * Input ;

	Input = CharBuf.InputData ;
	for( i = 0 ; i < MAX_INPUT_NUM ; i ++, Input ++ )
	{
		if( Input->UseFlag == TRUE )
		{
			NS_DeleteKeyInput( i | DX_HANDLETYPE_MASK_KEYINPUT | ( Input->ID << DX_HANDLECHECK_ADDRESS ) ) ;
		}
	}

	// ＩＭＥの入力情報取得用に確保しているメモリがある場合は解放
	if( CharBuf.IMEInputData )
	{
		DXFREE( CharBuf.IMEInputData ) ;
		CharBuf.IMEInputData = NULL ;
	}

	// アクティブハンドルを-1にする
	CharBuf.ActiveInputHandle = -1 ;

	// 終了
	return 0 ;
}

// 新しいキー入力データの作成
extern int NS_MakeKeyInput( int MaxStrLength , int CancelValidFlag , int SingleCharOnlyFlag , int NumCharOnlyFlag, int DoubleCharOnlyFlag )
{
	int i, Result ;
	INPUTDATA * Input ;

	// 使われていないキー入力データを探す
	for( i = 0 ; i != MAX_INPUT_NUM && CharBuf.InputData[ i ].UseFlag ; i ++ ){}
	if( i == MAX_INPUT_NUM ) return -1 ;
	Input = &CharBuf.InputData[ i ] ;

	// データの初期化
	_MEMSET( Input, 0, sizeof( *Input ) ) ;
	if( ( Input->Buffer	= ( TCHAR * )DXCALLOC( ( MaxStrLength + 1 ) * sizeof( TCHAR ) ) ) == NULL ) return -1 ;
	Input->MaxStrLength			= MaxStrLength ;
	Input->CancelValidFlag		= CancelValidFlag ;
	Input->SingleCharOnlyFlag	= SingleCharOnlyFlag ;
	Input->NumCharOnlyFlag		= NumCharOnlyFlag ;
	Input->DoubleCharOnlyFlag	= DoubleCharOnlyFlag ;
	Input->UseFlag				= TRUE ;
	Input->ID					= CharBuf.HandleID ;
	Input->SelectStart			= -1 ;
	Input->SelectEnd			= -1 ;
	CharBuf.HandleID ++ ;
	if( CharBuf.HandleID >= ( DX_HANDLECHECK_MASK >> DX_HANDLECHECK_ADDRESS ) )
		CharBuf.HandleID = 0 ;

	// ＩＭＥを有効にする
	CharBuf.IMEUseFlag_System = TRUE ;
	RefreshIMEFlag() ;

	// 文字コードバッファをクリアする
	NS_ClearInputCharBuf() ;

	// ハンドル値の作成
	Result = i | DX_HANDLETYPE_MASK_KEYINPUT | ( Input->ID << DX_HANDLECHECK_ADDRESS ) ;

	// 終了
	return Result ;
}

// キー入力データの削除
extern int NS_DeleteKeyInput( int InputHandle )
{
	INPUTDATA * Input ;
	int i ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// 確保していたメモリの解放
	if( Input->Buffer != NULL ) DXFREE( Input->Buffer ) ;

	// データの初期化
	_MEMSET( &CharBuf.InputData[ InputHandle & DX_HANDLEINDEX_MASK ], 0, sizeof( INPUTDATA ) ) ;
	
	if( CharBuf.ActiveInputHandle == InputHandle )
	{
		CharBuf.ActiveInputHandle = -1 ;
	}

	// すべてのキー入力が無効になっていればＩＭＥは無効にする
	for( i = 0 ; i != MAX_INPUT_NUM && !CharBuf.InputData[ i & DX_HANDLEINDEX_MASK ].UseFlag ; i ++ ){}
	if( i == MAX_INPUT_NUM )
	{
		CharBuf.IMEUseFlag_System = FALSE ;
		RefreshIMEFlag() ;
	}

	// 終了
	return 0 ;
}

// 指定のキー入力をアクティブにする
extern int NS_SetActiveKeyInput( int InputHandle )
{
	INPUTDATA * Input ;
	INPUTDATA * ActInput ;

	// ハンドルチェック
	Input = NULL ;
	if( InputHandle > 0 )
	{
		if( KEYHCHK( InputHandle, Input ) ) return -1 ;
	}

	// カーソル点滅処理のカウンタをリセット
	ResetKeyInputCursorBrinkCount() ;

	// 今までと同じ場合は何もしない
	if( InputHandle == CharBuf.ActiveInputHandle &&
		( ( Input != NULL && Input->EndFlag == FALSE ) || ( Input == NULL && InputHandle >= 0 ) ) ) return 0 ;

	// 今までアクティブだったハンドルで選択領域がある場合は解除する
	if( CharBuf.ActiveInputHandle != -1 )
	{
		if( KEYHCHK( CharBuf.ActiveInputHandle, ActInput ) ) return -1 ;

		ActInput->SelectStart = -1 ;
		ActInput->SelectEnd = -1 ;
	}

	if( InputHandle < 0 )
	{
		CharBuf.ActiveInputHandle = -1 ;
	}
	else
	{
		// アクティブな入力ハンドルを変更する前に入力文字列をクリアしておく
		NS_ClearInputCharBuf() ;

		CharBuf.ActiveInputHandle = InputHandle ;

		// エンドフラグの類を初期化する
		Input->EndFlag = FALSE ;
		Input->CancellFlag = FALSE ;
	}

	// 終了
	return 0 ;
}

// 現在アクティブになっているキー入力ハンドルを取得する
extern int NS_GetActiveKeyInput( void )
{
	return CharBuf.ActiveInputHandle ;
}

// 入力が終了しているか取得する
extern int NS_CheckKeyInput( int InputHandle )
{
	INPUTDATA * Input ;
	int Result ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	Result = Input->CancellFlag ? 2 : ( Input->EndFlag ? 1 : 0 ) ;

	return Result ;
}

// 入力が完了したキー入力を再度編集状態に戻す
extern int NS_ReStartKeyInput( int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	Input->EndFlag = FALSE ;

	// 終了
	return 0 ;
}

// キー入力処理関数
extern int NS_ProcessActKeyInput( void )
{
	INPUTDATA * Input ;
	TCHAR C[ 3 ] = { 0 , 0 , 0 } ;
	int CharLen, len, DelNum ;
	HIMC Imc;

	// ＩＭＥのリフレッシュ処理
	{
		HWND DefHwnd;
		static TCHAR str[256];

		switch( CharBuf.IMERefreshStep )
		{
		case 1:
//			DefHwnd = ImmGetDefaultIMEWnd( WinData.MainWindow ) ;
			DefHwnd = WinData.MainWindow;
			if( DefHwnd )
			{
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
				if( Imc )
				{
					_MEMSET( CharBuf.InputString, 0, CHARBUFFER_SIZE );
					_MEMSET( CharBuf.InputTempString, 0, CHARBUFFER_SIZE );
					WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , CharBuf.InputTempString , CHARBUFFER_SIZE );
					WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc , NI_COMPOSITIONSTR ,  CPS_CANCEL , 0  );

					WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
				}
			}
			CharBuf.IMERefreshStep ++ ;
//			break;

		case 2:
			SetIMEOpenState( FALSE );
			CharBuf.IMERefreshStep ++ ;
//			break;

		case 3:
			SetIMEOpenState( TRUE );
			CharBuf.IMERefreshStep ++ ;
//			break;

		case 4:
//			DefHwnd = ImmGetDefaultIMEWnd( WinData.MainWindow ) ;
			DefHwnd = WinData.MainWindow;
			if( DefHwnd )
			{
				Imc = WinAPIData.ImmFunc.ImmGetContextFunc( DefHwnd ) ;
				if( Imc )
				{
					WinAPIData.ImmFunc.ImmSetOpenStatusFunc( Imc, TRUE );
					WinAPIData.ImmFunc.ImmSetCompositionStringFunc( Imc, SCS_SETSTR, CharBuf.InputTempString, lstrlen( CharBuf.InputTempString ), 0, 0 );
					WinAPIData.ImmFunc.ImmReleaseContextFunc( DefHwnd , Imc );
				}
			}
			CharBuf.IMERefreshStep = 0 ;
//			break;
		}
	}

	if( CharBuf.ActiveInputHandle == -1 ) return 0 ;
	if( KEYHCHK( CharBuf.ActiveInputHandle, Input ) ) return -1 ;
	if( Input->EndFlag ) return 0 ;

	// ＩＭＥの入力文字数制限処理
	{
		TCHAR TempString[ CHARBUFFER_SIZE ] ;
		int i, len, maxlen ;
		bool flag ;

		// 文字数が指定最大バイト数を超えていたら強制的に文字数を減らす
		flag = false ;
		len = lstrlen( CharBuf.InputString ) ;
		maxlen = 65536 ;
		if( CharBuf.IMEInputStringMaxLengthIMESync && Input->StrLength + len > Input->MaxStrLength )
		{
			flag = true ;
			maxlen = Input->MaxStrLength - Input->StrLength ;
		}

		if( CharBuf.IMEInputMaxLength && len > CharBuf.IMEInputMaxLength )
		{
			flag = true ;
			if( maxlen > CharBuf.IMEInputMaxLength ) maxlen = CharBuf.IMEInputMaxLength ;
		}

		if( flag )
		{
			// 入力コンテキストの取得
			Imc = WinAPIData.ImmFunc.ImmGetContextFunc( WinData.MainWindow ) ;
			if( Imc )
			{
				// 実バッファにある文字列もそうなっているか調べる
				_MEMSET( TempString, 0, sizeof( TempString ) );
				WinAPIData.ImmFunc.ImmGetCompositionStringFunc( Imc , GCS_COMPSTR , TempString , CHARBUFFER_SIZE );
				len = lstrlen( TempString ) ;
				if( len > maxlen )
				{
					// 文字列を短くする
					for( i = 0; i < len ; )
					{
						if( _TMULT( TempString[ i ], _GET_CHARSET() ) )
						{
							if( i + 2 > maxlen ) break;
							i += 2 ;
						}
						else
						{
							if( i + 1 > maxlen ) break;
							i ++ ;
						}
					}
					TempString[ i ] = _T( '\0' );

					// 設定を初期化
					WinAPIData.ImmFunc.ImmNotifyIMEFunc( Imc , NI_COMPOSITIONSTR ,  CPS_CANCEL , 0  );
					WinAPIData.ImmFunc.ImmSetCompositionStringFunc( Imc, SCS_SETSTR, TempString, lstrlen( TempString ), 0, 0 );
				}

				// 入力コンテキストの削除
				WinAPIData.ImmFunc.ImmReleaseContextFunc( WinData.MainWindow , Imc );
			}
		}
	}

	// キーコードバッファの続く限り処理
	while( Input->EndFlag == FALSE )
	{
		if( ( CharLen = NS_GetOneChar( C , TRUE ) ) == 0 ) break ;

		// カーソル点滅処理のカウンタをリセット
		ResetKeyInputCursorBrinkCount() ;
		
		switch( C[0] )
		{
			// 上下タブキーは無視
		case CTRL_CODE_UP :
		case CTRL_CODE_DOWN :
		case CTRL_CODE_TAB :
			break ;

			// ホームボタン
		case CTRL_CODE_HOME :
			if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
			{
				if( Input->SelectStart == -1 )
				{
					Input->SelectStart = Input->Point ;
					Input->SelectEnd   = 0 ;
				}
				else
				{
					Input->SelectEnd   = 0 ;
				}
			}
			else
			{
				Input->SelectStart = -1 ;
				Input->SelectEnd = -1 ;
			}
			Input->Point = 0 ;
			break ;

			// エンドボタン
		case CTRL_CODE_END :
			if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
			{
				if( Input->SelectStart == -1 )
				{
					Input->SelectStart = Input->Point ;
					Input->SelectEnd   = Input->StrLength ;
				}
				else
				{
					Input->SelectEnd   = Input->StrLength ;
				}
			}
			else
			{
				Input->SelectStart = -1 ;
				Input->SelectEnd = -1 ;
			}
			Input->Point = Input->StrLength ;
			break ;

			// カット
		case CTRL_CODE_CUT :
CUT:
			if( Input->SelectStart != -1 )
			{
				TCHAR *Buffer ;
				int smin, smax, size ;

				if( Input->SelectStart > Input->SelectEnd )
				{
					smin = Input->SelectEnd ;
					smax = Input->SelectStart ;
				}
				else
				{
					smin = Input->SelectStart ;
					smax = Input->SelectEnd ;
				}
				size = smax - smin ;
				Buffer = ( TCHAR * )DXALLOC( ( size + 1 ) * sizeof( TCHAR ) ) ;
				if( Buffer )
				{
					_MEMCPY( Buffer, &Input->Buffer[ smin ], size * sizeof( TCHAR ) ) ;
					Buffer[ size ] = _T( '\0' ) ;
					NS_SetClipboardText( Buffer ) ;
					DXFREE( Buffer ) ;
				}
				KeyInputSelectAreaDelete( Input ) ;
			}
			break ;

			// コピー
		case CTRL_CODE_COPY :
			if( Input->SelectStart != -1 )
			{
				TCHAR *Buffer ;
				int smin, smax, size ;

				if( Input->SelectStart > Input->SelectEnd )
				{
					smin = Input->SelectEnd ;
					smax = Input->SelectStart ;
				}
				else
				{
					smin = Input->SelectStart ;
					smax = Input->SelectEnd ;
				}
				size = smax - smin ;
				Buffer = ( TCHAR * )DXALLOC( ( size + 1 ) * sizeof( TCHAR ) ) ;
				if( Buffer )
				{
					_MEMCPY( Buffer, &Input->Buffer[ smin ], size * sizeof( TCHAR ) ) ;
					Buffer[ size ] = _T( '\0' ) ;
					NS_SetClipboardText( Buffer ) ;
					DXFREE( Buffer ) ;
				}
			}
			break ;

			// ペースト
		case CTRL_CODE_PASTE :
			if( NS_GetClipboardText( NULL ) != -1 )
			{
				int Size ;
				TCHAR *Buffer, *p ;

				Size = NS_GetClipboardText( NULL ) ;
				Buffer = ( TCHAR * )DXALLOC( Size ) ;
				if( Buffer )
				{
					NS_GetClipboardText( Buffer ) ;
					for( p = Buffer ; *p ; p++ )
					{
						if( _TMULT( *p, _GET_CHARSET() ) == TRUE )
						{
							NS_StockInputChar( *p ) ;
							p++ ;
							NS_StockInputChar( *p ) ;
						}
						else
						{
							if( *p != _T( '\r' ) && *p != _T( '\n' ) )
							{
								NS_StockInputChar( *p ) ;
							}
						}
					}
					DXFREE( Buffer ) ;
				}
			}
			break ;

			// エンター文字だった場合はここで終了
		case CTRL_CODE_CR :
			{
				Input->EndFlag = TRUE ;
				CharBuf.ActiveInputHandle = -1 ;
			}
			break ;

			// エスケープキーだった場合はキャンセル
		case CTRL_CODE_ESC :
			if( Input->CancelValidFlag )
			{
				Input->CancellFlag = TRUE ;
				Input->EndFlag = TRUE ;
				CharBuf.ActiveInputHandle = -1 ;
			}
			break ;

			// デリートキーだった場合は文字の削除
		case CTRL_CODE_DEL :
			if( Input->SelectStart == -1 )
			{
				if( Input->Point != Input->StrLength )
				{
					DelNum = _TMULT( Input->Buffer[Input->Point], _GET_CHARSET() ) == TRUE ? 2 : 1 ;  
					_MEMMOVE( &Input->Buffer[ Input->Point ] , &Input->Buffer[ Input->Point + DelNum ] , ( Input->StrLength - Input->Point ) * sizeof( TCHAR ) ) ;
					Input->StrLength -= DelNum ;
				}
			}
			else
			{
				if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
				{
					goto CUT ;
				}
				KeyInputSelectAreaDelete( Input ) ;
			}
			break ;

			// バックスペースキーだった場合は一つ前の文字を削除
		case CTRL_CODE_BS :
			if( Input->SelectStart == -1 )
			{
				if( Input->Point > 0 )
				{
					int DelNum ;

					DelNum = 1 ;
					if( Input->Point > 1 )
					{
//						DelNum = _TMULT( Input->Buffer[Input->Point - 2], _GET_CHARSET() ) == TRUE ? 2 : 1 ;
						DelNum = CheckDoubleChar( Input->Buffer, Input->Point - 1, _GET_CHARSET() ) == 2 ? 2 : 1 ;
					}
		
					Input->Point -= DelNum ;
					_MEMMOVE( &Input->Buffer[ Input->Point ] , &Input->Buffer[ Input->Point + DelNum ] , ( Input->StrLength - Input->Point ) * sizeof( TCHAR ) ) ;
					Input->StrLength -= DelNum ;
				}
			}
			else
			{
				KeyInputSelectAreaDelete( Input ) ;
			}
			break ;

			// 移動キーの場合はその処理
		case CTRL_CODE_LEFT :
			if( Input->Point > 0 )
			{
				len = 1 ;
//				if( Input->Point > 1 && _TMULT( Input->Buffer[Input->Point - 2], _GET_CHARSET() ) == TRUE )
				if( Input->Point > 1 && CheckDoubleChar( Input->Buffer, Input->Point - 1, _GET_CHARSET() ) == 2 )
					len = 2 ;

				if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
				{
					if( Input->SelectStart == -1 )
					{
						Input->SelectStart = Input->Point ;
						Input->SelectEnd   = Input->Point - len ;
					}
					else
					{
						Input->SelectEnd   = Input->Point - len ;
					}
				}
				else
				{
					Input->SelectStart = -1 ;
					Input->SelectEnd = -1 ;
				}
				Input->Point -= len ;
			}
			break ;

		case CTRL_CODE_RIGHT :
			if( Input->Point < Input->StrLength )
			{
				len = 1 ;
				if( _TMULT( Input->Buffer[Input->Point], _GET_CHARSET() ) == TRUE )
					len = 2 ;

				if( NS_CheckHitKey( KEY_INPUT_LSHIFT ) || NS_CheckHitKey( KEY_INPUT_RSHIFT ) )
				{
					if( Input->SelectStart == -1 )
					{
						Input->SelectStart = Input->Point ;
						Input->SelectEnd   = Input->Point + len ;
					}
					else
					{
						Input->SelectEnd   = Input->Point + len ;
					}
				}
				else
				{
					Input->SelectStart = -1 ;
					Input->SelectEnd = -1 ;
				}
				Input->Point += len ;
			}
			break ;

		default:
			KeyInputSelectAreaDelete( Input ) ;

			// 数値文字のみ時の処理
			if( Input->NumCharOnlyFlag )
			{
				// 数値文字でなければキャンセル
				if( C[ 0 ] < _T( '0' ) || C[ 0 ] > _T( '9' ) )
				{
					// マイナス文字でカーソル位置が０だった場合は入力可能
					if( C[ 0 ] != _T( '-' ) || Input->Point != 0 )
					{
						// ピリオドも一個までＯＫ
						if( ( C[ 0 ] == _T( '.' ) && _TSTRCHR( Input->Buffer, _T( '.' ) ) != NULL ) || C[ 0 ] != _T( '.' ) ) break ;
					}
				}
			}

			// １バイト文字のみ時の処理
			if( Input->SingleCharOnlyFlag )
			{
				if( CharLen != 1 ) break ;
			}

			// ２バイト文字のみ時の処理
			if( Input->DoubleCharOnlyFlag )
			{
				if( CharLen == 1 ) break ;
			}

			// 文字列の追加
			{
				// バッファに空きがない時の処理
				while( Input->StrLength + CharLen > Input->MaxStrLength )
				{
					int /*DelNum,*/ Pos, CLen ;

					// 最後の文字を削る
					Pos = 0 ;
					while( Pos < Input->StrLength )
					{
						if( _TMULT( Input->Buffer[Pos], _GET_CHARSET() ) == TRUE )	CLen = 2 ;
						else														CLen = 1 ;
						Pos += CLen ;
					}
					Input->Buffer[Pos - CLen] = _T( '\0' ) ;
					Input->StrLength -= CLen ;

					// カーソルも移動する
					if( Input->Point == Pos ) Input->Point -= CLen ;
				}
				
				_MEMMOVE( &Input->Buffer[ Input->Point + CharLen ] , &Input->Buffer[ Input->Point ] , ( Input->StrLength - Input->Point + 1 ) * sizeof( TCHAR ) ) ;
				_MEMMOVE( &Input->Buffer[ Input->Point ] , C , CharLen * sizeof( TCHAR ) ) ;

				Input->Point += CharLen ;
				Input->StrLength += CharLen ;
				break ;
			}
		}
	}

	// 終了
	return 0 ;
}

// キー入力中データの描画
extern int NS_DrawKeyInputString( int x , int y , int InputHandle )
{
	INPUTDATA * Input ;
	int Use3DFlag , FontSize ;
	int PointX , FontHandle ;
	int AddY, StrWidth ;
	RECT DrawRect ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;
	if( Input->EndFlag ) return -1 ;

	// 各種データ保存
	Use3DFlag = NS_GetUse3DFlag() ;

	// ３Ｄ描画機能の使用変更
	NS_SetUse3DFlag( FALSE ) ;

	// 使用するフォントのハンドルをセットする
	FontHandle = CharBuf.UseFontHandle == -1 ? NS_GetDefaultFontHandle() : CharBuf.UseFontHandle ;

	// フォントサイズの取得
	FontSize = NS_GetFontSizeToHandle( FontHandle ) ;
	
	// カーソル点滅処理
	if( CharBuf.CBrinkFlag == TRUE )
	{
		if( NS_GetNowCount() - CharBuf.CBrinkCount > CharBuf.CBrinkWait )
		{
			CharBuf.CBrinkDrawFlag = CharBuf.CBrinkDrawFlag == TRUE ? FALSE : TRUE ;
			CharBuf.CBrinkCount = NS_GetNowCount() ;
		}
	}
	else
	{
		CharBuf.CBrinkDrawFlag = TRUE ;
	}

	// 描画開始文字位置の決定
	{
		int DrawWidth, DrawHNum ;
		int tx, ty, w ;

		NS_GetDrawArea( &DrawRect ) ;
		AddY = FontSize + FontSize / 10 * 3 ;

		if( DrawRect.left == DrawRect.right ||
			DrawRect.bottom == DrawRect.top )
			return -1 ;

		tx = x ;
		ty = y ;
		if( tx < DrawRect.left ) tx = DrawRect.left ;
		if( ty < DrawRect.top  ) ty = DrawRect.top ;

		while( tx > DrawRect.right )
		{
			tx -= DrawRect.right - DrawRect.left ;
			ty += AddY ;
		}

		DrawHNum = ( DrawRect.bottom - ty ) / AddY ;
		if( DrawHNum == 0 )
		{
			DrawWidth = DrawRect.right - tx ;
			DrawHNum = 1 ;
		}
		else
		{
			DrawWidth = ( DrawRect.right - tx ) + ( DrawHNum - 1 ) * ( DrawRect.right - DrawRect.left ) ;
		}

		StrWidth = NS_GetDrawStringWidthToHandle( Input->Buffer , Input->StrLength , FontHandle ) + 4 ;

		if( StrWidth < DrawWidth )
		{
			Input->DrawStartPos = 0 ;
		}
		else
		{
			if( Input->DrawStartPos > Input->Point )
			{
				Input->DrawStartPos = Input->Point ;
			}
			else
			{
				w = NS_GetDrawStringWidthToHandle( Input->Buffer + Input->DrawStartPos, Input->Point - Input->DrawStartPos, FontHandle ) ;
				if( w > DrawWidth )
				{
					int TotalWidth, Width, Num ;

					TotalWidth = 0 ;
					Input->DrawStartPos = Input->Point ;
					for(;;)
					{
						if( Input->DrawStartPos <= 0 )
						{
							break ;
						}

						if( Input->DrawStartPos > 1 )
						{
							Num = _TMULT( Input->Buffer[ Input->DrawStartPos - 2 ], _GET_CHARSET() ) == TRUE ? 2 : 1 ;
						}
						else
						{
							Num = 1 ;
						}

						Width = NS_GetDrawStringWidthToHandle( Input->Buffer + Input->DrawStartPos - Num, Num, FontHandle ) ;
						if( TotalWidth + Width > DrawWidth ) break ;
						TotalWidth += Width ;
						Input->DrawStartPos -= Num ;
					}
				}
			}
		}
	}

	// カーソルの描画
	PointX = NS_GetDrawStringWidthToHandle( Input->Buffer + Input->DrawStartPos, Input->Point - Input->DrawStartPos, FontHandle ) + x ;
	if( CharBuf.CBrinkDrawFlag == TRUE && InputHandle == CharBuf.ActiveInputHandle )
//		|| ( InputHandle == CharBuf.ActiveInputHandle && CharBuf.IMESwitch == TRUE ) )
	{
		NS_DrawObtainsBox( PointX , y , PointX + 2 , y + FontSize , FontSize + FontSize / 10 * 3 , CharBuf.StrColor2 , TRUE ) ;
	}

	// 文字列の描画
	NS_DrawObtainsString( x , y , AddY , Input->Buffer + Input->DrawStartPos, CharBuf.StrColor1 , CharBuf.StrColor8 , FontHandle, CharBuf.StrColor13, CharBuf.StrColor14, CharBuf.StrColor15, Input->SelectStart, Input->SelectEnd ) ;

	// 入力中文字列の描画
	if( InputHandle == CharBuf.ActiveInputHandle )
	{
		NS_DrawIMEInputString( PointX , y , 5 ) ; 
	}

	// データを元に戻す
	NS_SetUse3DFlag( Use3DFlag ) ;

	// 終了
	return 0 ;
}

// キー入力時のカーソルの点滅する早さをセットする
extern int NS_SetKeyInputCursorBrinkTime( int Time )
{
	CharBuf.CBrinkWait = Time ;

	// カーソル点滅処理のカウンタをリセット
	ResetKeyInputCursorBrinkCount() ;

	// 終了
	return 0 ;
}

// キー入力データの指定の領域を選択状態にする( SelectStart と SelectEnd に -1 を指定すると選択状態が解除されます )
extern int NS_SetKeyInputSelectArea( int SelectStart, int SelectEnd, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( SelectStart == -1 && SelectEnd == -1 )
	{
		Input->SelectStart = -1 ;
		Input->SelectEnd   = -1 ;
	}
	else
	{
		if( SelectStart < 0                ) SelectStart = 0 ;
		if( SelectStart > Input->StrLength ) SelectStart = Input->StrLength ;
		if( SelectEnd   < 0                ) SelectEnd   = 0 ;
		if( SelectEnd   > Input->StrLength ) SelectEnd   = Input->StrLength ;

		Input->SelectStart = SelectStart ;
		Input->SelectEnd   = SelectEnd ;
	}

	// 終了
	return 0 ;
}

// キー入力データの選択領域を取得する
extern int NS_GetKeyInputSelectArea( int *SelectStart, int *SelectEnd, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( SelectStart ) *SelectStart = Input->SelectStart ;
	if( SelectEnd   ) *SelectEnd   = Input->SelectEnd ;

	// 終了
	return 0 ;
}

// キー入力の描画開始文字位置を取得する
extern int NS_GetKeyInputDrawStartPos( int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	return Input->DrawStartPos ;
}

// キー入力の描画開始文字位置を設定する
extern int NS_SetKeyInputDrawStartPos( int DrawStartPos, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( DrawStartPos < 0                ) DrawStartPos = 0 ;
	if( DrawStartPos > Input->StrLength ) DrawStartPos = Input->StrLength ;

	Input->DrawStartPos = DrawStartPos ;

	return Input->DrawStartPos ;
}

// キー入力時のカーソルを点滅させるかどうかをセットする
extern int NS_SetKeyInputCursorBrinkFlag( int Flag )
{
	CharBuf.CBrinkFlag = Flag ;

	// カーソル点滅処理のカウンタをリセット
	ResetKeyInputCursorBrinkCount() ;
	
	// 終了
	return 0 ;
}

// キー入力時のカーソル点滅処理のカウンタをリセット
static int ResetKeyInputCursorBrinkCount( void )
{
	CharBuf.CBrinkCount = NS_GetNowCount() ;
	CharBuf.CBrinkDrawFlag = TRUE ;
	
	// 終了
	return 0 ;
}
	



// キー入力データに指定の文字列をセットする
extern int NS_SetKeyInputString( const TCHAR *String , int InputHandle )
{
	INPUTDATA * Input ;
	int StrLen ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	StrLen = lstrlen( String ) ;
	if( StrLen > Input->MaxStrLength )
		StrLen = Input->MaxStrLength ;

	_TSTRNCPY( Input->Buffer, String, StrLen ) ;
	Input->Buffer[ StrLen ] = _T( '\0' ) ;
	Input->StrLength = StrLen ;
	Input->Point = StrLen ;
	Input->SelectEnd = -1 ;
	Input->SelectStart = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// カーソル点滅処理のカウンタをリセット
	ResetKeyInputCursorBrinkCount() ;
	
	// 終了
	return 0 ;
}

// キー入力データに指定の数値を文字に置き換えてセットする
extern int NS_SetKeyInputNumber( int Number , int InputHandle )
{
	INPUTDATA * Input ;
	TCHAR StrBuf[ 512 ] ;
	int StrLen ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// 文字に置き換える
	_ITOT( Number, StrBuf, 10 ) ;

	StrLen = lstrlen( StrBuf ) ;
	if( StrLen > Input->MaxStrLength )
		StrLen = Input->MaxStrLength ;

	_TSTRNCPY( Input->Buffer , StrBuf , StrLen ) ;
	Input->Buffer[ StrLen ] = _T( '\0' ) ;
	Input->StrLength = StrLen ;
	Input->Point = StrLen ;
	Input->SelectEnd = -1 ;
	Input->SelectStart = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// カーソル点滅処理のカウンタをリセット
	ResetKeyInputCursorBrinkCount() ;

	// 終了
	return 0 ;
}

// キー入力データに指定の浮動小数点値を文字に置き換えてセットする
extern int NS_SetKeyInputNumberToFloat( float Number, int InputHandle )
{
	INPUTDATA * Input ;
	TCHAR StrBuf[ 256 ] ;
	int StrLen ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// 文字に置き換える
	_TSPRINTF( _DXWTP( StrBuf ), _DXWTR( "%f" ), Number ) ;

	StrLen = lstrlen( StrBuf ) ;
	if( StrLen > Input->MaxStrLength )
		StrLen = Input->MaxStrLength ;

	_TSTRNCPY( Input->Buffer , StrBuf, StrLen ) ;
	Input->Buffer[ StrLen ] = _T( '\0' ) ;
	Input->StrLength = StrLen ;
	Input->Point = StrLen ;
	Input->SelectEnd = -1 ;
	Input->SelectStart = -1 ;
	if( Input->StrLength < Input->DrawStartPos ) Input->DrawStartPos = Input->StrLength ;

	// カーソル点滅処理のカウンタをリセット
	ResetKeyInputCursorBrinkCount() ;

	// 終了
	return 0 ;
}

// 入力中の文字列データを取得する
extern int NS_GetKeyInputString( TCHAR *StrBuffer , int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	lstrcpy( StrBuffer , Input->Buffer ) ;

	// 終了
	return 0 ;
}

// 入力データの文字列を整数値として取得する
extern int NS_GetKeyInputNumber( int InputHandle )
{
	INPUTDATA * Input ;
	int Number ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// 数値に変換
	if( _TSTRCHR( Input->Buffer, _T( '.' ) ) )
	{
		Number = _FTOL( ( float )_TTOF( _DXWTP( Input->Buffer ) ) ) ;
	}
	else
	{
		Number = _TTOI( _DXWTP( Input->Buffer ) ) ;
	}

	// 終了
	return Number ;
}

// 入力データの文字列を浮動小数点値として取得する
extern float NS_GetKeyInputNumberToFloat( int InputHandle )
{
	INPUTDATA * Input ;
	float Number ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// 数値に変換
	if( _TSTRCHR( Input->Buffer, _T( '.' ) ) )
	{
		Number = ( float )_TTOF( _DXWTP( Input->Buffer ) ) ;
	}
	else
	{
		Number = ( float )_TTOI( _DXWTP( Input->Buffer ) ) ;
	}

	// 終了
	return Number ;
}

// キー入力の現在のカーソル位置を設定する
extern int NS_SetKeyInputCursorPosition( int Position, int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	if( Position < 0                ) Position = 0 ;
	if( Position > Input->StrLength ) Position = Input->StrLength ;

	Input->Point = Position ;

	// 終了
	return 0 ;
}

// キー入力の現在のカーソル位置を取得する
extern int NS_GetKeyInputCursorPosition( int InputHandle )
{
	INPUTDATA * Input ;

	if( KEYHCHK( InputHandle, Input ) ) return -1 ;

	// 終了
	return Input->Point ;
}

// ＩＭＥで入力中の文字列の情報を取得する
extern const IMEINPUTDATA *NS_GetIMEInputData( void )
{
	LPIMEINPUTDATA IMEInput ;
	int TotalSize, i, Len ;
	LPIMEINPUTCLAUSEDATA IMEClause ;

	// 既に確保されていたら解放
	if( CharBuf.IMEInputData )
	{
		DXFREE( CharBuf.IMEInputData ) ;
		CharBuf.IMEInputData = NULL ;
	}

	if( CharBuf.IMEUseFlag_OSSet == FALSE || CharBuf.IMESwitch == FALSE /* || CharBuf.ClauseNum == 0 */ ) return NULL ;

	if( CharBuf.InputString[ 0 ] == 0 ) return NULL ;

	// 確保すべきメモリ容量を計算
	Len = lstrlen( CharBuf.InputString ) ;
	TotalSize = sizeof( IMEINPUTDATA ) ;
	TotalSize += ( Len + 1 ) * sizeof( TCHAR ) ;
	if( CharBuf.ClauseNum > 1 )
	{
		TotalSize += ( CharBuf.ClauseNum - 1 ) * sizeof( IMEINPUTCLAUSEDATA ) ;
	}
	if( CharBuf.CandidateList != NULL )
	{
		TotalSize += CharBuf.CandidateList->dwCount * sizeof( TCHAR ** ) + CharBuf.CandidateListSize ;
	}

	// メモリの確保
	IMEInput = ( LPIMEINPUTDATA )DXALLOC( TotalSize ) ;
	if( IMEInput == NULL )
	{
		DXST_ERRORLOG_ADD( _T( "ＩＭＥ入力情報用のメモリの確保に失敗しました\n" ) ) ;
		return NULL ;
	}
	CharBuf.IMEInputData = IMEInput ;

	// 情報のセット
	{
		const void *NextP ;

		// 文字列情報のセット
		IMEInput->InputString = ( TCHAR * )( IMEInput + 1 );
		_TSTRNCPY( ( TCHAR * )IMEInput->InputString, CharBuf.InputString, Len + 1 ) ;

		// カーソル位置のセット
		IMEInput->CursorPosition = NS_GetStringPoint( CharBuf.InputString , CharBuf.InputPoint ) ;

		// 分節情報のセット
		if( CharBuf.ClauseNum > 1 )
		{
			IMEInput->ClauseNum = CharBuf.ClauseNum - 1 ;
			IMEClause = ( LPIMEINPUTCLAUSEDATA )( IMEInput->InputString + Len + 1 ) ;
			IMEInput->ClauseData = IMEClause ;
			IMEInput->SelectClause = -1 ;
			for( i = 0 ; i < IMEInput->ClauseNum ; i ++ , IMEClause ++ )
			{
				IMEClause->Position = CharBuf.ClauseData[ i ] ;
				IMEClause->Length   = CharBuf.ClauseData[ i + 1 ] - IMEClause->Position ;
				if( CharBuf.ClauseData[ i ] == IMEInput->CursorPosition )
				{
					IMEInput->SelectClause = i ;
				}
			}

			NextP = IMEInput->ClauseData + IMEInput->ClauseNum ;
		}
		else
		{
			IMEInput->ClauseNum = 0 ;
			IMEInput->ClauseData = NULL ;
			IMEInput->SelectClause = -1 ;

			NextP = IMEInput->InputString + Len + 1 ;
		}

		// 変換候補情報のセット
		if( CharBuf.CandidateList != NULL )
		{
			PCANDIDATELIST CandidateListBase ;
			TCHAR **CandidateList ;

			CandidateList = ( TCHAR ** )NextP ;
			IMEInput->CandidateList = ( const TCHAR ** )CandidateList ;
			IMEInput->CandidateNum = CharBuf.CandidateList->dwCount ;
			CandidateListBase = ( PCANDIDATELIST )( IMEInput->CandidateList + IMEInput->CandidateNum ) ;
			_MEMCPY( CandidateListBase, CharBuf.CandidateList, CharBuf.CandidateListSize ) ;
			for( i = 0 ; i < IMEInput->CandidateNum ; i ++ )
			{
				CandidateList[ i ] = ( TCHAR * )( ( BYTE * )CandidateListBase + CandidateListBase->dwOffset[ i ] ) ;
			}
			IMEInput->SelectCandidate = ( int )CharBuf.CandidateList->dwSelection ;
		}
		else
		{
			IMEInput->CandidateList   = NULL ;
			IMEInput->CandidateNum    = 0 ;
			IMEInput->SelectCandidate = 0 ;
		}

		// 文字列変換中かどうかをセット
		if( CharBuf.CharAttrNum == 0 )
		{
			IMEInput->ConvertFlag = FALSE ;
		}
		else
		{
			for( i = 0 ; i < CharBuf.CharAttrNum && CharBuf.CharAttr[ i ] != ATTR_INPUT ; i ++ ){}
			IMEInput->ConvertFlag = i == CharBuf.CharAttrNum ? TRUE : FALSE ;
		}
	}

	// 作成した情報を返す
	return CharBuf.IMEInputData ;
}

#endif

}

#endif // DX_NON_INPUTSTRING

