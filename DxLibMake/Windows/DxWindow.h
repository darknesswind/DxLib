﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ウインドウプログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXWINDOW_H__
#define __DXWINDOW_H__

// Include ------------------------------------------------------------------
#include "../DxCompileConfig.h"
#include "../DxLib.h"
#include "../DxMemory.h"
#include "../DxThread.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#define WIN_MOUSEINPUT_LOG_NUM		(32)				// 保持するマウスのクリック情報

#define MAX_MENUITEMSELECT_NUM		(64)				// 保持しておける選択されたメニュー項目の最大数
#define MAX_MENUITEM_NUM			(128)				// メニュー項目の最大数
#define MAX_ACCELERATOR_NUM			(64)				// アクセラレータの最大数
#define MAX_TOOLBARITEM_NUM			(64)				// ツールバーに置けるアイテムの最大数

#define MAX_MESTAKEOVERWIN_NUM		(100)				// メッセージプロセスを肩代わりするウインドウの最大数
#define MAX_DRAGFILE_NUM			(1000)				// ドラッグ&ドロップされたファイル名を保存する最高数
#define MAX_THREADWAIT_NUM			(4096)				// 同時に待つことが出来るスレッドの最大数

typedef HHOOK 						( *MSGFUNC )( HWND MainWindow, HHOOK *pKeyboardHookHandle ) ;		// メッセージフック設定関数

#define WSA_WINSOCKMESSAGE 			(WM_USER + 261)

// 结构体定义 --------------------------------------------------------------------

// メニュー項目の情報
struct WINMENUITEMINFO
{
	HMENU					Menu ;								// メニュー
	short					Index ;								// ナンバー
	unsigned short			ID ;								// ＩＤ
	TCHAR					Name[128] ;							// 名前
} ;

// ツールバー項目の情報
struct WINTOOLBARITEMINFO
{
	int						ID ;								// ID
	int						ImageIndex ;						// 画像のインデックス
	int						Type ;								// タイプ
	int						State ;								// 状態
	int						Click ;								// クリックされたか
} ;

// ＰＣの情報
struct PCINFO
{
	TCHAR					OSString[256] ;						// ＯＳの記述
	TCHAR					DirectXString[256] ;				// ＤｉｒｅｃｔＸの記述
	TCHAR					CPUString[256] ;					// ＣＰＵの記述
	int						CPUSpeed ;							// ＣＰＵの速度(単位MHz)
	LONGLONG				FreeMemorySize ;					// 空きメモリサイズ(単位byte)
	LONGLONG				TotalMemorySize ;					// 総メモリサイズ(単位byte)
	TCHAR					VideoDriverFileName[256] ;			// ビデオカードドライバファイル名
	TCHAR					VideoDriverString[256] ;			// ビデオカードドライバの記述
	unsigned int			VideoFreeMemorySize ;				// 空きＶＲＡＭサイズ(単位byte)
	unsigned int			VideoTotalMemorySize ;				// ＶＲＡＭの総サイズ(単位byte)
} ;

#ifndef DX_NON_INPUT
// マウスの入力情報
struct WINMOUSEINPUT
{
	int						Button ;							// クリックしたボタン
	int						ClickX, ClickY ;					// クリックしたクライアント座標
} ;
#endif // DX_NON_INPUT

// ウインドウ系データ構造体
struct WINDATA
{
	PCINFO					PcInfo ;							// ＰＣの情報
	HINSTANCE				Instance ;							// ソフトのインスタンスハンドル
	HWND					MainWindow ;						// メインウインドウハンドル
	HWND					UserChildWindow ;					// 表示用子ウインドウハンドル

	int						ProcessorNum ;						// 論理ＣＰＵコアの数
	int						AeroDisableFlag ;					// Aeroを無効にしているかどうかのフラグ
	int						SystembarHeight ;					// システムバーの高さ

#ifdef DX_THREAD_SAFE
	HANDLE					ProcessMessageThreadHandle ;		// ProcessMessage をひたすら呼びつづけるプロセスのハンドル
	DWORD					ProcessMessageThreadID ;			// ProcessMessage をひたすら呼びつづけるプロセスのＩＤ
	DWORD					ProcessMessageThreadExitFlag ;		// ProcessMessage をひたすら呼びつづけるプロセスが終了時に立てるフラグ
#endif

	HRGN					WindowRgn ;							// メインウインドウのリージョン
	int						WindowStyle ;						// ウインドウのスタイルタイプインデックス値
	int						UserWindowFlag ;					// MainWindow はＤＸライブラリが作成したウインドウではないか、フラグ(ＴＲＵＥ：ユーザーから渡されたウインドウ  ＦＡＬＳＥ：ＤＸライブラリが作成したウインドウ)
	int						NotUserWindowMessageProcessDXLibFlag ;	// UserWindowFlag が立っている場合、ウインドウのメッセージ処理をＤＸライブラリが行わないかどうか、フラグ(ＴＲＵＥ：ＤＸライブラリは何もせずユーザーがメッセージ処理を行う　ＦＡＬＳＥ：ＤＸライブラリが行う)
	WNDPROC					DefaultUserWindowProc ;				// ＤＸライブラリのプロシージャを設定する前にユーザーのウインドウに設定されていたプロシージャ
	DWORD					MainThreadID ;						// InitializeWindow を呼び出したスレッドのＩＤ


	RECT					WindowEdgeRect ;					// ウインドウモードで動作している時のウインドウの矩形領域
	RECT					WindowRect ;						// ウインドウモードで動作している時のウインドウのクライアント領域
	int						WindowModeFlag ;					// ウインドウモードで動作しているか、のフラグ
	int						ChangeWindodwFlag ;					// ウインドウモードに変更中か、フラグ
	int						UseChangeWindowModeFlag ;			// ALT+ENTER によるフルスクリーン←→ウインドウの変更機能が有効であるかフラグ
	void					(*ChangeWindowModeCallBackFunction)(void *) ; // フルスクリーン←→ウインドウの変更が起きたときに呼ぶ関数
	void					*ChangeWindowModeCallBackFunctionData ;	// コールバック関数に渡すデータ

	int						QuitMessageFlag ;					// WM_QUITメッセージが送られてきたかどうかのフラグ変数
	int						CloseMessagePostFlag ;				// WM_CLOSEメッセージを送った時にＴＲＵＥになるフラグ変数
	int						DestroyMessageCatchFlag ;			// WM_DESTROY メッセージが来た時にＴＲＵＥになるフラグ変数
	int						DisplayChangeMessageFlag ;			// WM_DISPLAYCHANGE メッセージが来かたフラグ
	
	int						ScreenCopyRequestFlag ;				// ScreenCopy をして欲しいかどうかのフラグ
	int						ScreenCopyRequestStartTime ;		// ScreenCopy をして欲しいかどうかのフラグが立ったときの時間
	int						ActiveFlag ;						// 実行中のソフトがアクティブ状態か（ＴＲＵＥ：アクティブ ＦＡＬＳＥ：非アクティブ）
	int						WindowMinSizeFlag ;					// 最小化されているかどうかのフラグ
	int						WaitTimeValidFlag ;					// WaitTime が有効かどうかのフラグ
	int						WaitTime ;							// 他のソフトがアクティブになり、ソフトが停止し始めた時のカウント
	int						StopFlag ;							// 一時的にオブジェクトをリリースしているかフラグ
	int						ChangeWindowModeFlag ;				// ウインドウモードの変更を行う指定が起こったらＴＲＵＥになるフラグ
	int						NonUserCloseEnableFlag ;			// ユーザーが×ボタンを押した時にライブラリ側でウインドウを閉じるかどうかのフラグ
	int						NonDxLibEndPostQuitMessageFlag ;	//ＤＸライブラリの終了時に PostQuitMessage を呼ばないかどうかのフラグ
	int						AltF4_EndFlag ;						// 強制終了か、フラグ
	int						WM_PAINTFlag ;						// WM_PAINT 処理中か、フラグ
	int						RecvWM_PAINTFlag;					// WM_PAINT が来たかどうか、フラグ
	int						UseFPUPreserve ;					// FPU精度を落とさない設定を使用するかどうか( TRUE:使用する  FALSE:使用しない )

	int						MouseDispFlag ;						// マウスの表示フラグ
	int						MouseDispState ;					// マウスの表示状態
	int						MouseMoveZ ;						// マウスのホイールの移動量
	int						MouseMoveHZ ;						// マウスの水平ホイールの移動量
#ifndef DX_NON_INPUT
	int						MouseInputNum ;						// マウスの入力情報の数
	WINMOUSEINPUT			MouseInput[ WIN_MOUSEINPUT_LOG_NUM ] ;	// マウスの入力情報
#endif // DX_NON_INPUT

	int						WM_ACTIVATE_StockNum ;				// WM_ACTIVATE メッセージストックの数
	int						WM_ACTIVATE_StartIndex ;			// WM_ACTIVATE メッセージリングバッファの開始インデックス
	int						WM_ACTIVATE_EndIndex ;				// WM_ACTIVATE メッセージリングバッファの終了インデックス
	WPARAM					WM_ACTIVATE_wParam[ 512 ] ;			// WM_ACTIVATE にメッセージが来た祭の wParam
	int						WM_ACTIVATE_APPMes[ 512 ] ;			// WM_ACTIVATEAPP の肩代わりか

	int						PerformanceTimerFlag ;				// パフォーマンスカウンターが使えるかフラグ
	LONGLONG				PerformanceClock ;					// パフォーマンスカウンターの周波数

	SIZE					DefaultScreenSize ;					// 起動時のデスクトップの画面サイズ
	int						DefaultColorBitCount ;				// 起動時のデスクトップのカラービット数

	int						ComInitializeFlag ;					// ＣＯＭを初期化したか、のフラグ
	int						WindowCreateFlag ;					// ウインドウ作成中か、フラグ
	int						ProcessMessageFlag ;				// ProcessMessage を実行中か、フラグ
	int						DoubleStartValidFlag ;				// ２重起動を許すかどうかのフラグ(TRUE:許す FALSE:許さない)

	BASEIMAGE				PauseGraph ;						// アクティブが他に移っているときに描画する画像
	int						PauseGraphHandle ;					// アクティブが他に移っているときに描画する画像のグラフィックハンドル
	int						(*ActiveStateChangeCallBackFunction)( int ActiveState, void *UserData ) ;	// ウインドウのアクティブ状態に変化があったときに呼ばれる関数
	void					*ActiveStateChangeCallBackFunctionData ;	// ActiveStateChangeCallBackFunction に渡すデータアドレス

	TCHAR					CurrentDirectory[ MAX_PATH ] ;		// 起動時のカレントディレクトリ
	int						EnableWindowText ;					// WindowText が有効かどうか
	TCHAR					WindowText[ 256 ] ;					// メインウインドウテキスト
	TCHAR					ClassName[ 256 ] ;					// メインウインドウのクラス名

	int						NonActiveRunFlag ;					// ウインドウがアクティブではなくても処理を実行するかどうかのフラグ

	int						IconID ;							// 使用するアイコンのＩＤ
	HICON					IconHandle ;						// 使用するアイコンのハンドル

	int						UseAccelFlag ;						// アクセラレーターを使用するかどうかフラグ
	HACCEL					Accel ;								// アクセラレーター

	HMENU					Menu ;								// 使用しているメニューのハンドル
	int						MenuSetupFlag ;						// メニューをセットしてあるかどうかフラグ( TRUE:セットしてある  FALSE:してない )
	int						MenuUseFlag ;						// メニューを使用しているかフラグ
	int						MenuResourceID ;					// メニューのリソースＩＤ
	int						(*MenuProc)( WORD ID ) ;			// メニューメッセージのコールバック関数
	void					(*MenuCallBackFunction)( const TCHAR *ItemName, int ItemID ) ;	// メニューメッセージのコールバック関数
	int						NotMenuDisplayFlag ;				// メニュー表示フラグ
	int						MenuDisplayState ;					// メニューが表示されているかどうかフラグ
	int						NotMenuAutoDisplayFlag ;			// メニューを自動で表示したりしなかったりする
	int						MousePosInMenuBarFlag ;				// メニューバーの領域にマウスポインタが入っているか、フラグ
	int						SelectMenuItem[MAX_MENUITEMSELECT_NUM] ; // 選択されたメニュー項目のリスト
	int						SelectMenuItemNum ;					// 選択されたメニュー項目の数
	WINMENUITEMINFO			MenuItemInfo[MAX_MENUITEM_NUM] ;	// メニューの選択項目の情報
	int						MenuItemInfoNum ;					// メニューの選択項目の数

	int						ToolBarUseFlag ;					// ツールバーを使用するかどうかの情報
	HWND					ToolBarHandle ;						// ツールバーのウインドウハンドル
	HBITMAP					ToolBarButtonImage ;				// ツールバーのボタンのビットマップ
	int						ToolBarItemNum ;					// ツールバーのアイテムの数
	WINTOOLBARITEMINFO		ToolBarItem[MAX_TOOLBARITEM_NUM] ;	// ツールバーのボタンの情報

	int						WindowSizeChangeEnable ;			// ウインドウのサイズを変更できるかフラグ
	double					WindowSizeExRateX, WindowSizeExRateY ;	// 描画画面のサイズに対するウインドウサイズの比率
	int						ScreenNotFitWindowSize ;			// ウインドウのクライアント領域に画面をフィットさせないかどうかフラグ( TRUE:フィットさせない  FALSE:フィットする )
	int						WindowWidth, WindowHeight ;			// ウインドウのサイズ( ユーザー指定 )
	int						WindowSizeValid ;					// ウインドウのサイズが有効かどうか( TRUE:有効  FALSE:無効 )
	int						WindowSizeValidResetRequest ;		// ウインドウのサイズが有効かどうかのフラグをリセットするリクエストフラグ
	int						WindowX, WindowY ;					// ウインドウの位置( ユーザー指定 )
	int						WindowPosValid ;					// ウインドウの位置が有効かどうか( TRUE:有効  FALSE:無効 )

	int						VisibleFlag ;
	int						NotWindowVisibleFlag ;				// ウインドウを表示しないフラグ
	int						WindowMinimizeFlag ;				// ウインドウを最小化状態にするかどうかのフラグ
	int						WindowMaximizeFlag ;				// ウインドウを最大化状態にするかどうかのフラグ
	RECT					WindowMaximizedClientRect ;			// ウインドウ最大化状態でのクライアント領域のサイズ
	int						NotMoveMousePointerOutClientAreaFlag ;	// ウインドウのクライアントエリア外にマウスポインタが移動できないようにするかどうかのフラグ
	int						NotActive_WindowMoveOrSystemMenu ;	// WM_ENTERSIZEMOVE や WM_ENTERMENULOOP によって非アクティブになっている
	int						SetClipCursorFlag ;					// ClipCursor の設定が有効になっているかどうかフラグ
	int						SysCommandOffFlag ;					// タスクスイッチを抑制する処理を行うかフラグ
	HHOOK					TaskHookHandle ;					// フックハンドル
	HHOOK					GetMessageHookHandle ;				// WH_GETMESSAGE フックハンドル
	HHOOK					KeyboardHookHandle ;				// WH_KEYBOARD_LL フックハンドル
	int						LockInitializeFlag ;				// ロックをかけろというフラグ
	TCHAR					HookDLLFilePath[MAX_PATH] ;			// フック処理をするＤＬＬファイルへのパス
	int						NotUseUserHookDllFlag ;				// ユーザー指定のＤＬＬを使用していないかどうかフラグ
	HMODULE					LoadResourModule ;					// リソースから読み込む系で使用するモジュール( NULL の場合は GetModuleHandle( NULL ) を使用 )

	int						BackBufferTransColorFlag ;			// バックバッファの透過色の部分を透過させるかどうかのフラグ( TRUE:透過させる  FALSE:透過させない )
	int						UseUpdateLayerdWindowFlag ;			// UpdateLayerdWindow を使用するかどうかのフラグ( TRUE;使用する  FALSE:使用しない )
	BOOL					( WINAPI *UpdateLayeredWindow )( HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD ) ;		// UpdateLayeredWindow のＡＰＩポインタ
	HBITMAP					BackBufferTransBitmap ;				// バックバッファの透過色の部分を透過させるためのビットマップ
	void					*BackBufferTransBitmapImage ;		// BackBufferTransBitmap のイメージの先頭アドレス
	SIZE					BackBufferTransBitmapSize ;			// BackBufferTransBitmap のサイズ
	COLORDATA				BackBufferTransBitmapColorData ;	// BackBufferTransBitmap のカラーフォーマット

	HINSTANCE				MessageHookDLL ;					// メッセージフックＤＬＬのハンドル
	MSGFUNC					MessageHookCallBadk ;				// メッセージフックのコールバック関数
	DWORD					MessageHookThredID ;				// メッセージフックＤＬＬのスレッドＩＤ

	int						DirectXVersion ;					// DirectXのバージョン
	int						WindowsVersion ;					// Windowsのバージョン
	int						UseMMXFlag ;						// MMXが使えるかフラグ
	int						UseSSEFlag ;						// SSEが使えるかどうかフラグ
	int						UseSSE2Flag ;						// SSE2が使えるかどうかフラグ
	int						UseRDTSCFlag ;						// RDTSC 命令が使えるか、フラグ
	LONGLONG				OneSecCount ;						// 一秒間あたりのクロック数

	WNDPROC					UserWindowProc ;					// ユーザー定義のウインドウプロージャ
	int						UseUserWindowProcReturnValue ;		// ユーザー定義のウインドウプロージャの戻り値を使用するかどうか( TRUE:使用する  FALSE:使用しない )

	HWND					MesTakeOverWindow[MAX_MESTAKEOVERWIN_NUM] ;	// メッセージ処理を肩代わりするウインドウのリスト
	int						MesTakeOverWindowNum ;				// メッセージ処理を肩代わりするウインドウの数
	
	POINT					InitializeWindowPos ;				// 初期ウインドウの位置
	int						InitializeWindowPosValidFlag ;		// 初期ウインドウの位置の座標の有効フラグ
	
	HWND					DialogBoxHandle ;					// ダイアログボックスのハンドル
	
	int						DragFileValidFlag ;					// ファイルのドラッグ＆ドロップを許すかフラグ
	TCHAR					*DragFileName[MAX_DRAGFILE_NUM] ;	// ドラッグ＆ドロップされたファイル名
	int						DragFileNum ;						// ドラッグ＆ドロップされたファイルの数

//	WINFILEACCESS			*ReadOnlyFileAccessFirst ;			// ファイルアクセス構造体リストの先頭へのポインタ
//	int						ReadOnlyFileAccessLockFlag ;		// ファイルアクセス構造体リストへのアクセスを禁止しているか、フラグ

#if defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )
//	int						DxConflictCheckFlag ;				// ＤＸライブラリの関数が複数のタスクから同時に実行されないようにするためのカウンタその１
	int						DxConflictCheckCounter ;			// ＤＸライブラリの関数が複数のタスクから同時に実行されないようにするためのカウンタその２
	int						DxUseThreadFlag ;					// ＤＸライブラリを使用しているスレッドがあるかどうか、フラグ( FALSE:ない  TRUE:ある )
	DWORD					DxUseThreadID ;						// ＤＸライブラリを使用しているスレッドのＩＤ
	DWORD_PTR				DxConflictWaitThreadID[MAX_THREADWAIT_NUM][2] ;	// 0:待ちスレッドのID  1:イベントハンドル
	int						DxConflictWaitThreadIDInitializeFlag ;	// DxConflictWaitThreadID を初期化してあるかどうかのフラグ
	int						DxConflictWaitThreadNum ;			// 待ちスレッドの数
	DX_CRITICAL_SECTION		DxConflictCheckCriticalSection ;	// 衝突防止用クリティカルセクション
#endif
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// ウインドウのデータ
extern WINDATA WinData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// ＣＯＭ初期化、終了関係関数
extern	int			InitializeCom( void ) ;													// ＣＯＭインターフェースを初期化する
extern	int			TerminateCom( void ) ;													// ＣＯＭインターフェースを終了する

// 初期化終了系関数
extern int			InitializeWindow( void ) ;												// ウインドウ関係の初期化関数
extern int			TerminateWindow( void ) ;												// ウインドウ関係の処理終了関数

// 情報出力系
extern	int			OutSystemInfo( void ) ;													// ＯＳやＤｉｒｅｃｔＸのバージョンを取得する

// 衝突防止用関数
//extern	void	PreparationDxFunction( void ) ;											// ＤＸライブラリ内部で公開関数を使用する前に呼ぶ関数
extern	void		CheckConflictAndWaitDxFunction( void ) ;								// 同時にＤＸライブラリの関数が呼ばれていないかチェックして、同時に呼ばれていたら待つ関数
extern	void		PostConflictProcessDxFunction( void ) ;									// CheckConfictAndWaitDxFunction を使用したＤＸライブラリの関数が return する前に呼ぶべき関数

// ソフトのウインドウにフォーカスを移す
extern	void		SetAbsoluteForegroundWindow( HWND hWnd, int Flag = TRUE ) ;

// Copyright SYN
extern unsigned long CheckMMX(void) ;
extern void srandMT(unsigned long seed) ;
extern void generateMT(void) ;
extern unsigned long randMT(void) ;

/*
extern __inline void PreparationDxFunction( void )
{
#if defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )
	// メインカウンタをインクリメントする
	WinData.DxConflictCheckCounter ++ ;
#endif
}
*/


// 設定系関数
extern	int			SetWindowModeFlag( int Flag ) ;											// ウインドウモードで起動するかをセット
extern	int			SetWindowStyle( void ) ;												// ウインドウのスタイルをセットする
extern	int			RefreshDragFileValidFlag( void ) ;										// ファイルのドラッグ＆ドロップ機能を有効にするかどうかの設定を再設定する


// 情報取得警官数
extern	PCINFO		*GetPcInfoStructP( void ) ;												// ＰＣ情報構造体のアドレスを得る
extern	HWND		GetDisplayWindowHandle( void ) ;										// 表示に使用するウインドウのハンドルを取得する


// 便利関数
//extern	int			CheckConflict( int *Check_Inc_Counter ) ;									// マルチスレッド時の衝突を防ぐための関数
//extern	int			CheckConflict2( int *Check_NonInc_Counter, int *NonCheck_Inc_Counter ) ;	// マルチスレッド時の衝突を防ぐための関数2
//extern	int			CheckConflict3( int *Check_NonInc_Counter, int *Check_Inc_Counter ) ;		// マルチスレッド時の衝突を防ぐための関数3

// メッセージ処理関数
extern	void		DxActiveWait( void ) ;													// アクティブになるまで何もしない
extern	int			CheckActiveWait( void ) ;												// 非アクティブかどうかをチェックする
extern	int			GetQuitMessageFlag( void ) ;											// WM_QUIT が発行されているかどうかを取得する
extern	int			WM_ACTIVATEProcessUseStock( WPARAM wParam, int APPMes = FALSE ) ;
extern	int			WM_ACTIVATEProcess( WPARAM wParam, int APPMes = FALSE ) ;

// Aero の有効、無効設定
extern	int			SetEnableAero( int Flag ) ;

// ツールバー関係
extern	int			GetToolBarHeight( void ) ;													// ツールバーの高さを得る

// メニュー関係
extern	int			MenuAutoDisplayProcess( void ) ;																// メニューを自動的に表示したり非表示にしたりする処理を行う

// 補助関数
extern	void		_FileTimeToLocalDateData( FILETIME *FileTime, DATEDATA *DateData ) ;
extern	int			UpdateBackBufferTransColorWindow( const BASEIMAGE *SrcImage, const RECT *SrcImageRect = NULL, HDC Direct3DDC = NULL, int NotColorKey = FALSE, int PreMultipliedAlphaImage = FALSE ) ;	// 指定の基本イメージを使用して UpdateLayeredWindow を行う

// リソース関係
extern	int			GetBmpImageToResource( int ResourceID, BITMAPINFO **BmpInfoP, void **GraphDataP ) ;					// ＢＭＰリソースから BITMAPINFO と画像イメージを構築する

}

#endif // __DXWINDOW_H__
