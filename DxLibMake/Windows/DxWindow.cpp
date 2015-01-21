// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ウインドウ関係制御プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ------------------------------------------------------------------
#include "DxWindow.h"
//#include <winsock2.h>
//#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <Winuser.h>
#include <commctrl.h>
#include <float.h>
#ifndef DX_NON_KEYEX
//#include <imm.h>
#endif
#include "../DxLog.h"
#include "../DxGraphics.h"
#include "../DxModel.h"
#include "../DxFile.h"
#include "../DxSound.h"
#include "../DxInput.h"
#include "../DxMovie.h"
#include "../DxMask.h"
#include "../DxFont.h"
#include "../DxSystem.h"
#include "../DxInputString.h"
#include "../DxNetwork.h"
#include "../DxBaseImage.h"
#include "../DxASyncLoad.h"
#include "../DxGraphicsBase.h"
#include "DxWindow.h"
#include "DxWinAPI.h"
#include "DxGuid.h"

#ifdef DX_NON_INLINE_ASM
  #include <math.h>
#endif


/*
#ifdef DX_USE_VISUAL_C_MEMDUMP
	#include <crtdbg.h>
#endif
*/
namespace DxLib
{

// マクロ定義 -------------------------------------
#ifdef __BCC
	#ifdef sprintf
	#undef sprintf
	#endif
	
	#ifdef vsprintf
	#undef vsprintf
	#endif
#endif

//メインウインドウのクラス名
#define DXCLASSNAME		_T( "D123987X" )

//メインウインドウのデフォルトタイトル
#ifndef DX_NON_LITERAL_STRING
	#define WIN_DEFAULT_TITLE	"DxLib"
#else
	#define WIN_DEFAULT_TITLE	"Soft"
#endif

#ifdef UNICODE
	#define CLIPBOARD_TEXT CF_UNICODETEXT
#else
	#define CLIPBOARD_TEXT CF_TEXT
#endif





#ifndef DX_NON_INLINE_ASM

// RDTSC ニーモニックと cpuid ニーニックの定義
#ifdef __BCC
	#define RDTSC __emit__(0fh) ;	__emit__(031h) ;
	#define cpuid __emit__(0xf) ; __emit__(0xa2) ;
#else
	#define cpuid __asm __emit 0fh __asm __emit 0a2h
	#define RDTSC __asm __emit 0fh __asm __emit 031h
#endif

#endif // DX_NON_INLINE_ASM

#ifndef WH_KEYBOARD_LL
#define WH_KEYBOARD_LL    			(13)
#endif
#ifndef LLKHF_ALTDOWN
#define LLKHF_ALTDOWN    			(0x00000020)
#endif
#ifndef LLKHF_UP
#define LLKHF_UP           			(0x00000080)
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 				(0x20A)				// マウスのホイール動作メッセージの定義
#endif
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL				(0x20E)
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA					(120)
#endif


//#define CHECKMULTIBYTECHAR(CP)		(( (unsigned char)*(CP) >= 0x81 && (unsigned char)*(CP) <= 0x9F ) || ( (unsigned char)*(CP) >= 0xE0 && (unsigned char)*(CP) <= 0xFC ))	// TRUE:２バイト文字  FALSE:１バイト文字

// ツールバー用定義
#define TOOLBAR_COMMANDID_BASE		(0x500)


// ウインドウスタイル定義
//#define WSTYLE_WINDOWMODE			(WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_VISIBLE)	// 通常
//#define WSTYLE_WINDOWMODE			(WS_POPUP | WS_SYSMENU | WS_VISIBLE)								// タスクバーなし
//#define WEXSTYLE_WINDOWMODE		(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE)
//#define WSTYLE_FULLSCREENMODE		(WS_POPUP)
//#define WEXSTYLE_FULLSCREENMODE	(WS_EX_TOPMOST)

// ウインドウスタイルの定義
#define WSTYLE_NUM					(12)		// ウインドウスタイルの数

#ifndef DX_GCC_COMPILE
// レイヤードウインドウ用の定義
#define WS_EX_LAYERED				0x00080000
#endif

#ifndef LWA_COLORKEY
#define LWA_COLORKEY				0x01
#endif
#ifndef LWA_ALPHA
#define LWA_ALPHA					0x02
#endif

#ifndef ULW_COLORKEY
#define ULW_COLORKEY				0x01
#endif
#ifndef ULW_ALPHA
#define ULW_ALPHA					0x02
#endif
#ifndef ULW_OPAQUE
#define ULW_OPAQUE					0x04
#endif

#define AC_SRC_OVER                 0x00
#define AC_SRC_ALPHA                0x01



struct __MEMORYSTATUSEX
{
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORDLONG ullTotalPhys;
    DWORDLONG ullAvailPhys;
    DWORDLONG ullTotalPageFile;
    DWORDLONG ullAvailPageFile;
    DWORDLONG ullTotalVirtual;
    DWORDLONG ullAvailVirtual;
    DWORDLONG ullAvailExtendedVirtual;
} ;

typedef WINBASEAPI BOOL ( WINAPI * GLOBALMEMORYSTATUSEX_FUNC )( __MEMORYSTATUSEX * ) ;


//  0 :デフォルト
//  1 :タスクバーなし
//  2 :タスクバーなし枠なし
//  3 :枠なし
//  4 :何もなし
//  5 :最小化無し
//  6 :ツールバーに適したウインドウ
//  7 :最大化ボタンつきウインドウ、初期状態が通常サイズ
//  8 :最大化ボタンつきウインドウ、初期状態が最大化状態
//  9 :デフォルトに縁の立体化無し
// 10 :最大化ボタンつきウインドウ、縁の立体化無し
// 11 :閉じる無し、最小化無し
const DWORD WStyle_WindowModeTable[WSTYLE_NUM] = 
{
	WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION,
	WS_POPUP | WS_SYSMENU,
	WS_POPUP | WS_SYSMENU,
	WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION,
	WS_POPUP | WS_SYSMENU,
	WS_POPUP | WS_SYSMENU | WS_CAPTION,
	WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION,
	WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION,
	WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZE,
	WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION,
	WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_MAXIMIZE,
	WS_POPUP | WS_CAPTION,
} ;

const DWORD WExStyle_WindowModeTable[WSTYLE_NUM] = 
{
	WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
	WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
	0,
	WS_EX_TOOLWINDOW,
	WS_EX_TOOLWINDOW,
	WS_EX_WINDOWEDGE,
	WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
	WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
	WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
	WS_EX_WINDOWEDGE,
	WS_EX_WINDOWEDGE,
	WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE,
} ;

const DWORD WStyle_FullScreenModeTable[WSTYLE_NUM] = 
{
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
	WS_POPUP,
} ;

const DWORD WExStyle_FullScreenModeTable[WSTYLE_NUM] = 
{
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
	WS_EX_TOPMOST,
} ;

//typedef LRESULT ( CALLBACK *MSGFUNC)(int, WPARAM, LPARAM) ;		// メッセージフックのコールバック関数
#define F10MES				( WM_USER + 111 )
#define F12MES				( WM_USER + 112 )

// 结构体定义 --------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 圧縮キーボードフックＤＬＬバイナリデータ
extern BYTE DxKeyHookBinary[];

WINDATA WinData ;												// ウインドウのデータ

// 関数プロトタイプ宣言-----------------------------------------------------------

#ifndef DX_NON_INPUT

// マウスの入力情報を保存する
static	void		StockMouseInputInfo( int Button ) ;

#endif // DX_NON_INPUT

// メッセージ処理関数
static	LRESULT		CALLBACK DxLib_WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) ;	// メインウインドウのメッセージコールバック関数
LRESULT CALLBACK	LowLevelKeyboardProc (INT nCode, WPARAM wParam, LPARAM lParam)	;			// フックされた時のコールバック関数
#ifdef DX_THREAD_SAFE
DWORD	WINAPI		ProcessMessageThreadFunction( LPVOID ) ;									// ProcessMessage をひたすら呼びつづけるスレッド
#endif

// ツールバー関係
static	int			SearchToolBarButton( int ID ) ;												// 指定のＩＤのボタンのインデックスを得る

// メニュー関係
static	int			SearchMenuItem( const TCHAR *ItemName, int ItemID, HMENU SearchMenu, HMENU *Menu, int *Index ) ;					// メニュー構造の中から、選択項目の位置情報を得る( -1:エラー  0:見つからなかった  1:見つかった )

static	int			GetDisplayMenuState( void ) ;												// メニューが表示するべきかどうかを取得する
static	int			_GetMenuItemInfo( HMENU Menu, int Index, MENUITEMINFO *Buffer ) ;			// メニューアイテムの情報を取得する
static	HMENU		MenuItemSubMenuSetup( const TCHAR *ItemName, int ItemID ) ;					// 指定の選択項目にサブメニューを付けられるように準備をする

static	int			ListupMenuItemInfo( HMENU Menu ) ;											// メニューの選択項目の情報の一覧を作成する時に使用する関数
static	int			AddMenuItemInfo( HMENU Menu, int Index, int ID, const TCHAR *Name ) ;		// メニューの選択項目の情報を追加する
static	int			DeleteMenuItemInfo( const TCHAR *Name, int ID ) ;							// メニューの選択項目の情報を削除する
static	WINMENUITEMINFO *SearchMenuItemInfo( const TCHAR *Name, int ID ) ;					// メニューの選択項目の情報を取得する
static	int			GetNewMenuItemID( void ) ;																		// 新しい選択項目のＩＤを取得する

// メッセージ処理関数
		int			WM_SIZEProcess( void ) ;
		int			WM_SIZINGProcess( WPARAM wParam, LPARAM lParam ) ;
		int			WM_MOVEProcess( LPARAM lParam ) ;




// プログラム --------------------------------------------------------------------

// 初期化終了系関数

#pragma optimize("", off)

// ウインドウ関係の初期化関数
extern int InitializeWindow( void )
{
	WNDCLASSEX wc ;
//	WNDCLASSEXA wc ;
//	WNDCLASSEXW wc ;
	int WindowSizeX , WindowSizeY ;
	HDC hdc ;
	TIMECAPS tc ;
	TCHAR CurrentDirectory[ MAX_PATH ] ;
	TCHAR WindowText[ MAX_PATH ] ;
	TCHAR ClassName[ 256 ] ;

	LONGLONG		OneSecCount										= WinData.OneSecCount ;
	int				UseRDTSCFlag									= WinData.UseRDTSCFlag ;
	int 			WindowModeFlag 									= WinData.WindowModeFlag ;
	int				NonActiveRunFlag								= WinData.NonActiveRunFlag  ;
	int				UseChangeWindowModeFlag							= WinData.UseChangeWindowModeFlag ;
	void 			(*ChangeWindowModeCallBackFunction)(void *) 	= WinData.ChangeWindowModeCallBackFunction ;
	void 			*ChangeWindowModeCallBackFunctionData 			= WinData.ChangeWindowModeCallBackFunctionData ;
	HACCEL			Accel											= WinData.Accel ;
	HMENU 			Menu 											= WinData.Menu ;
	int				MenuUseFlag 									= WinData.MenuUseFlag ;
	int 			(*MenuProc)( WORD ID )							= WinData.MenuProc ;
	void			(*MenuCallBackFunction)( const TCHAR *ItemName, int ItemID ) = WinData.MenuCallBackFunction ;
	int				NotMenuDisplayFlag								= WinData.NotMenuDisplayFlag ;
	int				NotMenuAutoDisplayFlag							= WinData.NotMenuAutoDisplayFlag ;
//	int 			MenuShredRunFlag								= WinData.MenuShredRunFlag ;
//	int 			MenuStartKey 									= WinData.MenuStartKey ;
	int 			WindowStyl 										= WinData.WindowStyle ;
	HRGN 			WindowRgn 										= WinData.WindowRgn ;
	int 			IconID 											= WinData.IconID ;
	HICON			IconHandle										= WinData.IconHandle ;
	int				ProcessorNum									= WinData.ProcessorNum ;
	int 			DirectXVersion 									= WinData.DirectXVersion ;
	int 			WindowsVersion 									= WinData.WindowsVersion ;
	int 			DoubleStartValidFlag							= WinData.DoubleStartValidFlag ;
	WNDPROC 		UserWindowProc 									= WinData.UserWindowProc ;
	int				DragFileValidFlag								= WinData.DragFileValidFlag ;
	int				EnableWindowText								= WinData.EnableWindowText  ;
	int				WindowX											= WinData.WindowX;
	int				WindowY											= WinData.WindowY;
	int				WindowPosValid									= WinData.WindowPosValid;
	int				WindowWidth										= WinData.WindowWidth;
	int				WindowHeight									= WinData.WindowHeight;
	int				WindowSizeValid									= WinData.WindowSizeValid;
	int				WindowSizeChangeEnable							= WinData.WindowSizeChangeEnable ;
	int				ScreenNotFitWindowSize							= WinData.ScreenNotFitWindowSize ;
	double			WindowSizeExRateX								= WinData.WindowSizeExRateX ;
	double			WindowSizeExRateY								= WinData.WindowSizeExRateY ;
	int				NotWindowVisibleFlag							= WinData.NotWindowVisibleFlag ;
	int				WindowMinimizeFlag								= WinData.WindowMinimizeFlag ;
	int				UseFPUPreserve									= WinData.UseFPUPreserve ;
	int				NonUserCloseEnableFlag							= WinData.NonUserCloseEnableFlag ;
	int				NonDxLibEndPostQuitMessageFlag					= WinData.NonDxLibEndPostQuitMessageFlag ;
	HWND			MainWindow										= WinData.MainWindow ;
	HWND			UserChildWindow									= WinData.UserChildWindow ;
	int				UserWindowFlag									= WinData.UserWindowFlag ;
	int				NotUserWindowMessageProcessDXLibFlag			= WinData.NotUserWindowMessageProcessDXLibFlag ;
	int				NotMoveMousePointerOutClientAreaFlag			= WinData.NotMoveMousePointerOutClientAreaFlag ;
	int				BackBufferTransColorFlag						= WinData.BackBufferTransColorFlag ;
	int				UseUpdateLayerdWindowFlag						= WinData.UseUpdateLayerdWindowFlag ;
//	int				DxConflictCheckFlag								= WinData.DxConflictCheckFlag ;
#if defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )
	int				DxConflictCheckCounter							= WinData.DxConflictCheckCounter ;
	int				DxUseThreadFlag									= WinData.DxUseThreadFlag ;
	DWORD			DxUseThreadID									= WinData.DxUseThreadID ;
	DWORD_PTR		DxConflictWaitThreadID[MAX_THREADWAIT_NUM][2] ;
	int				DxConflictWaitThreadIDInitializeFlag			= WinData.DxConflictWaitThreadIDInitializeFlag ;
	int				DxConflictWaitThreadNum							= WinData.DxConflictWaitThreadNum ;
	DX_CRITICAL_SECTION DxConflictCheckCriticalSection					= WinData.DxConflictCheckCriticalSection ;
	_MEMCPY( DxConflictWaitThreadID, WinData.DxConflictWaitThreadID, sizeof( DxConflictWaitThreadID ) ) ;
#endif // defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )
	PCINFO			PcInfo											= WinData.PcInfo ;
	int				UseMMXFlag										= WinData.UseMMXFlag ;
	int				UseSSEFlag										= WinData.UseSSEFlag ;
	int				UseSSE2Flag										= WinData.UseSSE2Flag ;
	HMODULE			LoadResourModule								= WinData.LoadResourModule ;
	double			ExtendRateX, ExtendRateY ;
	HMODULE			Dll ;

	lstrcpy( CurrentDirectory, WinData.CurrentDirectory ) ;
	lstrcpy( WindowText, WinData.WindowText ) ;
	lstrcpy( ClassName, WinData.ClassName ) ;

	// 初期化
	_MEMSET( &WinData, 0, sizeof( WINDATA ) ) ;

	lstrcpy( WinData.CurrentDirectory, CurrentDirectory ) ;
	lstrcpy( WinData.WindowText, WindowText ) ;
	lstrcpy( WinData.ClassName, ClassName ) ;

	WinData.OneSecCount								= OneSecCount ;
	WinData.UseRDTSCFlag							= UseRDTSCFlag ;
	WinData.UseMMXFlag								= UseMMXFlag ;
	WinData.UseSSEFlag								= UseSSEFlag ;
	WinData.UseSSE2Flag								= UseSSE2Flag ;
	WinData.WindowModeFlag							= WindowModeFlag ;
	WinData.UseChangeWindowModeFlag					= UseChangeWindowModeFlag ;
	WinData.ChangeWindowModeCallBackFunction 		= ChangeWindowModeCallBackFunction ;
	WinData.ChangeWindowModeCallBackFunctionData 	= ChangeWindowModeCallBackFunctionData ;
	WinData.Accel									= Accel ;
	WinData.Menu 									= Menu ;
	WinData.MenuUseFlag 							= MenuUseFlag ; 
	WinData.MenuProc 								= MenuProc ;
//	WinData.MenuShredRunFlag						= MenuShredRunFlag ;
//	WinData.MenuStartKey 							= MenuStartKey ;
	WinData.WindowStyle 							= WindowStyl ;
	WinData.WindowRgn 								= WindowRgn ;
	WinData.IconID 									= IconID ;
	WinData.IconHandle								= IconHandle ;
	WinData.ProcessorNum							= ProcessorNum ;
	WinData.DirectXVersion 							= DirectXVersion ;
	WinData.WindowsVersion 							= WindowsVersion ;
	WinData.NonActiveRunFlag 						= NonActiveRunFlag ;
	WinData.DoubleStartValidFlag 					= DoubleStartValidFlag ;
	WinData.UserWindowProc 							= UserWindowProc ;
	WinData.DragFileValidFlag						= DragFileValidFlag ;
	WinData.EnableWindowText						= EnableWindowText ;
	WinData.WindowX									= WindowX;
	WinData.WindowY									= WindowY;
	WinData.WindowPosValid							= WindowPosValid;
	WinData.WindowWidth								= WindowWidth;
	WinData.WindowHeight							= WindowHeight;
	WinData.WindowSizeValid							= WindowSizeValid;
	WinData.MenuCallBackFunction					= MenuCallBackFunction ;
	WinData.NotMenuDisplayFlag						= NotMenuDisplayFlag ;
	WinData.NotMenuAutoDisplayFlag					= NotMenuAutoDisplayFlag ;
	WinData.WindowSizeChangeEnable					= WindowSizeChangeEnable ;
	WinData.ScreenNotFitWindowSize					= ScreenNotFitWindowSize ;
	WinData.WindowSizeExRateX						= WindowSizeExRateX ;
	WinData.WindowSizeExRateY						= WindowSizeExRateY ;
	WinData.NotWindowVisibleFlag					= NotWindowVisibleFlag ;
	WinData.WindowMinimizeFlag						= WindowMinimizeFlag ;
	WinData.UseFPUPreserve							= UseFPUPreserve ;
	WinData.NonUserCloseEnableFlag					= NonUserCloseEnableFlag ;
	WinData.NonDxLibEndPostQuitMessageFlag			= NonDxLibEndPostQuitMessageFlag ;
	WinData.MainWindow								= MainWindow ;
	WinData.UserChildWindow							= UserChildWindow ;
	WinData.UserWindowFlag							= UserWindowFlag ;
	WinData.NotUserWindowMessageProcessDXLibFlag	= NotUserWindowMessageProcessDXLibFlag ;
	WinData.NotMoveMousePointerOutClientAreaFlag	= NotMoveMousePointerOutClientAreaFlag ;
	WinData.BackBufferTransColorFlag				= BackBufferTransColorFlag ;
	WinData.UseUpdateLayerdWindowFlag				= UseUpdateLayerdWindowFlag ;
//	WinData.DxConflictCheckFlag						= DxConflictCheckFlag ;
#if defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )
	WinData.DxConflictCheckCounter					= DxConflictCheckCounter ;
	WinData.DxUseThreadFlag							= DxUseThreadFlag ;
	WinData.DxUseThreadID							= DxUseThreadID ;
	WinData.DxConflictWaitThreadIDInitializeFlag	= DxConflictWaitThreadIDInitializeFlag ;
	WinData.DxConflictWaitThreadNum					= DxConflictWaitThreadNum ;
	WinData.DxConflictCheckCriticalSection			= DxConflictCheckCriticalSection ;
	WinData.WaitTime								= -1 ;
	_MEMCPY( WinData.DxConflictWaitThreadID, DxConflictWaitThreadID, sizeof( DxConflictWaitThreadID ) ) ;
#endif // defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )
	WinData.PcInfo									= PcInfo ;
	WinData.LoadResourModule						= LoadResourModule ;

	// インスタンスハンドルの取得
	WinData.Instance = GetModuleHandle( NULL ) ;

	// スレッドＩＤの取得
	WinData.MainThreadID = GetCurrentThreadId() ;


#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込み処理の初期化
	InitializeASyncLoad( WinData.MainThreadID ) ;
#endif // DX_NON_ASYNCLOAD

	// ファイルアクセス処理の初期化
	InitializeFile() ;


	// UpdateLayerdWindow のアドレスを取得する
	Dll = LoadLibrary( _T( "user32" ) ) ;
	if( Dll != NULL )
	{
		WinData.UpdateLayeredWindow = ( BOOL ( WINAPI * )( HWND, HDC, POINT*, SIZE*, HDC, POINT*, COLORREF, BLENDFUNCTION*, DWORD ) )GetProcAddress( Dll, "UpdateLayeredWindow" ) ;
		FreeLibrary( Dll ) ;
	}

	// クラス名が何も設定されていない場合はデフォルトの名前を設定する
	if( WinData.ClassName[0] == _T( '\0' ) )
	{
		// ウインドウタイトルが設定されている場合はそれを設定する
		if( WinData.WindowText[0] != _T( '\0' ) )
		{
//			lstrcpyA( WinData.ClassName, WinData.WindowText );
			lstrcpy( WinData.ClassName, WinData.WindowText );
		}
		else
		{
//			lstrcpyA( WinData.ClassName, ( char * )DXCLASSNAME );
			lstrcpy( WinData.ClassName, DXCLASSNAME );
		}
	}

	// 空きメモリ容量の書き出し
	{
		TCHAR str[256];

		if( WinData.WindowsVersion >= DX_WINDOWSVERSION_2000 )
		{
			GLOBALMEMORYSTATUSEX_FUNC GlobalMemoryStatusExFunc ;
			__MEMORYSTATUSEX MemEx ;
			HRESULT mhr ;
			HMODULE Kernel32DLL ;

			Kernel32DLL = LoadLibrary( _T( "Kernel32.dll" ) ) ;
			if( Kernel32DLL )
			{
				GlobalMemoryStatusExFunc = ( GLOBALMEMORYSTATUSEX_FUNC )GetProcAddress( Kernel32DLL, "GlobalMemoryStatusEx" ) ;
				if( GlobalMemoryStatusExFunc )
				{
					_MEMSET( &MemEx, 0, sizeof( MemEx ) ) ;
					MemEx.dwLength = sizeof( MemEx ) ;
					mhr = GlobalMemoryStatusExFunc( &MemEx ) ;
					DXST_ERRORLOGFMT_ADD(( _T( "メモリ総量:%.2fMB  空きメモリ領域:%.2fMB " ),
								( double )( LONGLONG )MemEx.ullTotalPhys / 0x100000 , ( double )( LONGLONG )MemEx.ullAvailPhys / 0x100000 )) ;  
					_TSPRINTF( _DXWTP( str ), _DXWTR(  "%.2fMB\n" ), ( double )( LONGLONG )MemEx.ullAvailPhys / 0x100000 );
					OutputDebugString( str ) ;

					WinData.PcInfo.FreeMemorySize = MemEx.ullAvailPhys ;
					WinData.PcInfo.TotalMemorySize = MemEx.ullTotalPhys ;
				}

				FreeLibrary( Kernel32DLL ) ;
			}
		}

		if( WinData.PcInfo.TotalMemorySize == 0 )
		{
			MEMORYSTATUS Mem ;

			GlobalMemoryStatus( &Mem ) ;
			DXST_ERRORLOGFMT_ADD(( _T( "メモリ総量:%.2fMB  空きメモリ領域:%.2fMB " ),
						( double )Mem.dwTotalPhys / 0x100000 , ( double )Mem.dwAvailPhys / 0x100000 )) ;  
			_TSPRINTF( _DXWTP( str ), _DXWTR(  "%.2fMB\n" ), ( double )Mem.dwAvailPhys / 0x100000 );
			OutputDebugString( str ) ;

			WinData.PcInfo.FreeMemorySize = Mem.dwAvailPhys ;
			WinData.PcInfo.TotalMemorySize = Mem.dwTotalPhys ;
		}
	}

	// デスクチップのサイズとカラービット深度を保存
	WinData.DefaultScreenSize.cx = GetSystemMetrics( SM_CXSCREEN ) ;
	WinData.DefaultScreenSize.cy = GetSystemMetrics( SM_CYSCREEN ) ;
	hdc = GetDC( NULL ) ;
	WinData.DefaultColorBitCount = GetDeviceCaps( hdc , PLANES ) * GetDeviceCaps( hdc , BITSPIXEL ) ;
	ReleaseDC( NULL , hdc ) ;

	// 描画領域のサイズを取得
	NS_GetDrawScreenSize( &WindowSizeX , &WindowSizeY ) ;
	if( GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 )
	{
		WindowSizeX = 640 ;
		WindowSizeY = 480 ;
	}

	// パフォーマンスカウンター情報の初期化
	{
		// パフォーマンスカウンターの周波数を取得する
		QueryPerformanceFrequency( ( LARGE_INTEGER * )&WinData.PerformanceClock ) ;

		// 使えるかどうかを保存しておく
		WinData.PerformanceTimerFlag = WinData.PerformanceClock != 0 ;
	}

	// タイマの精度を設定する
	{
		WinAPIData.Win32Func.timeGetDevCapsFunc( &tc , sizeof(TIMECAPS) );

		// マルチメディアタイマーのサービス精度を最大に 
		WinAPIData.Win32Func.timeBeginPeriodFunc( tc.wPeriodMin ) ;
	}

	// タイマーの精度を検査する
	if( WinData.PerformanceTimerFlag )
	{
		int Cnt1 , Cnt2 , Time1 , NowTime1, StartTime ;
		LONGLONG Time2 , NowTime2 ;

		DXST_ERRORLOG_ADD( _T( "タイマーの精度を検査します\n" ) ) ;

		Cnt1 = 0 ;
		Cnt2 = 0 ;
		Time1 = WinAPIData.Win32Func.timeGetTimeFunc() ;
		QueryPerformanceCounter( ( LARGE_INTEGER * )&Time2 ) ; 
		StartTime = WinAPIData.Win32Func.timeGetTimeFunc() ;
		while( /*!NS_ProcessMessage() &&*/ Cnt1 != 60 && Cnt2 != 60 && WinAPIData.Win32Func.timeGetTimeFunc() - StartTime < 1000 )
		{
			QueryPerformanceCounter( ( LARGE_INTEGER * )&NowTime2 ) ;
			if( NowTime2 != Time2 )
			{
				Cnt2 ++ ;
				Time2 = NowTime2 ;
			}

			if( ( NowTime1 = WinAPIData.Win32Func.timeGetTimeFunc() ) != Time1 )
			{
				Cnt1 ++ ;
				Time1 = NowTime1 ;
			}
		}

		{
			DXST_ERRORLOGFMT_ADD(( _T( "精度結果 更新回数 マルチメディアタイマー：%d  パフォーマンスカウンター：%d" ), Cnt1 , Cnt2 )) ;

#ifndef DX_NON_LOG
			NS_ErrorLogAdd( _T( " " ) ); // 謎のバグ回避用
#endif
			if( Cnt1 > Cnt2 )
			{
				DXST_ERRORLOGFMT_ADD(( _T( "Multi Media Timer を使用します Timer 精度 : %d.00 ms " ), tc.wPeriodMin )) ;
				WinData.PerformanceTimerFlag = FALSE ;
			}
			else
			{
				DXST_ERRORLOGFMT_ADD(( _T( "パフォーマンスカウンターを使用します タイマー精度 : %lf KHz " ), WinData.PerformanceClock / 1000.0 )) ;
			}
#ifndef DX_NON_LOG
			NS_ErrorLogAdd( _T( " " ) ); // 謎のバグ回避用
#endif
		}
	}

	// ランダム係数を初期化
#ifndef DX_NON_MERSENNE_TWISTER
	srandMT( ( unsigned int )NS_GetNowCount() ) ;
#else
	srand( NS_GetNowCount() ) ;
#endif

	// スクリーンセーバー無効
	SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, FALSE, NULL, SPIF_SENDWININICHANGE ) ;

	// 使用する文字セットをセット
	if( _GET_CHARSET() == 0 )
	{
		switch( GetOEMCP() )
		{
		case 949 : _SET_CHARSET( DX_CHARSET_HANGEUL ) ; _SET_CODEPAGE( 949 ) ; break ;
		case 950 : _SET_CHARSET( DX_CHARSET_BIG5 ) ;    _SET_CODEPAGE( 950 ) ; break ;
		case 936 : _SET_CHARSET( DX_CHARSET_GB2312 ) ;  _SET_CODEPAGE( 936 ) ; break ;
		case 932 : _SET_CHARSET( DX_CHARSET_SHFTJIS ) ; _SET_CODEPAGE( 932 ) ; break ;
		default :  _SET_CHARSET( DX_CHARSET_SHFTJIS ) ; _SET_CODEPAGE( 0   ) ; break ;
		}
	}

	// ウインドウに関連する処理は NotWinFlag が立っている場合は実行しない
	if( DxSysData.NotWinFlag == FALSE )
	{
		// 二重起動防止処理
		DXST_ERRORLOG_ADD( _T( "ソフトの二重起動検査... " ) ) ;
		{
			if( FindWindow( WinData.ClassName , NULL ) != NULL )
			{
				if( WinData.DoubleStartValidFlag == FALSE )
				{
					DXST_ERRORLOG_ADD( _T( "二重起動されています、ソフトを終了します\n" ) ) ;
					WinData.QuitMessageFlag = TRUE;
					DxLib_EndRequest() ;
					return -1 ;
				}
				else
				{
					DXST_ERRORLOG_ADD( _T( "二重起動されていますが続行します\n" ) ) ;
				}
			}
			else
			{
				DXST_ERRORLOG_ADD( _T( "二重起動はされていませんでした\n" ) ) ;
			}
		}

		if( WinData.MenuUseFlag && DxSysData.NotWinFlag == FALSE )
		{
			WinData.MenuItemInfoNum = 0 ;
			ListupMenuItemInfo( WinData.Menu ) ;
		}

		// メニューを読み込む
	/*	if( WinData.MenuUseFlag )
		{
			// メニューをロード
			WinData.Menu = LoadMenu( WinData.Instance, MAKEINTRESOURCE( WinData.MenuResourceID ) ) ;
	//		WinData.MenuResourceID = FALSE ;

			// 失敗していたら終了
			if( WinData.Menu == NULL ) WinData.MenuUseFlag = FALSE ;
		}
		else
		{
			WinData.Menu = NULL ;
		}
	*/

		// ソフトプロセスの実行優先順位を最高レベルにセット
	//	DXST_ERRORLOG_ADD( _T( "ソフトの実行優先レベルを上げました\n" ) ) ;
	//	SetPriorityClass( GetCurrentProcess() , HIGH_PRIORITY_CLASS );

		// ウインドウがユーザーから提供されていない場合はＤＸライブラリが作成する
		// ユーザーから提供されている場合はプロシージャが呼ばれるようにする
		if( WinData.UserWindowFlag == TRUE )
		{
#ifdef _WIN64
			WinData.DefaultUserWindowProc = (WNDPROC)GetWindowLongPtr( WinData.MainWindow, GWLP_WNDPROC ) ;
#else
			WinData.DefaultUserWindowProc = (WNDPROC)GetWindowLong( WinData.MainWindow, GWL_WNDPROC ) ;
#endif
		}
		else
		{
			// ウインドウ作成中フラグを立てる
			WinData.WindowCreateFlag = TRUE ;

			// メインウインドウのウインドウクラスを登録
			_MEMSET( &wc , 0, sizeof( wc ) ) ;
			{
				wc.style			= CS_HREDRAW | CS_VREDRAW ;
				wc.lpfnWndProc		= DxLib_WinProc ;
				wc.cbClsExtra		= 0 ;
				wc.cbWndExtra		= 0 ;
				wc.hInstance		= WinData.Instance ;
				wc.hIcon			= WinData.IconHandle != NULL ? WinData.IconHandle : ( LoadIcon( WinData.Instance , ( WinData.IconID == 0 ) ? IDI_APPLICATION : MAKEINTRESOURCE( WinData.IconID ) ) ) ;
				wc.hCursor			= LoadCursor( NULL , IDC_ARROW ) ;
		//		wc.hbrBackground	= WinData.WindowModeFlag ? (HBRUSH)( COLOR_WINDOW + 1 ) : (HBRUSH)GetStockObject(BLACK_BRUSH); ;
		//		wc.hbrBackground	= ( WinData.WindowRgn == NULL && WinData.WindowModeFlag == TRUE ) ? (HBRUSH)GetStockObject(BLACK_BRUSH) : (HBRUSH)GetStockObject(NULL_BRUSH);
				wc.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH);
				wc.lpszMenuName		= NULL ;
				wc.lpszClassName	= WinData.ClassName ;
				wc.cbSize			= sizeof( WNDCLASSEX );
				wc.hIconSm			= NULL ;

				DXST_ERRORLOG_ADD( _T( "ウインドウクラスを登録します... " ) ) ;
//				if( !RegisterClassExA( &wc ) )
//				if( !RegisterClassExW( &wc ) )
				if( !RegisterClassEx( &wc ) )
				{
					WinData.QuitMessageFlag = TRUE;
					DxLib_EndRequest() ;
					//return 	DXST_ERRORLOG_ADD( _T( "ウインドウクラスの登録に失敗しました\n" ) ) ;
				}
				else
				{
					DXST_ERRORLOG_ADD( _T( "登録に成功しました\n" ) ) ;
				}
			}

			// ウインドウネームをセット
			if( WinData.EnableWindowText == FALSE ) lstrcpy( WinData.WindowText , _T( WIN_DEFAULT_TITLE ) ) ;
//			if( WinData.WindowText[0] == _T( '\0' ) ) lstrcpy( WinData.WindowText , _T( WIN_DEFAULT_TITLE ) ) ;
//			if( WinData.WindowText[0] == L'\0' ) _WCSCPY( WinData.WindowText , WIN_DEFAULT_TITLEW ) ;
//			if( WinData.WindowText[0] == '\0' ) lstrcpyA( WinData.WindowText , WIN_DEFAULT_TITLE ) ;

			// ウインドウを生成
			if( WinData.WindowModeFlag )
			{
				RECT Rect ;
				LONG AddStyle, AddExStyle ;

				// ウインドウモード時
				DXST_ERRORLOG_ADD( _T( "ウインドウモード起動用のウインドウを作成します\n" ) ) ;

				AddExStyle = 0 ;
//				if( WinData.NotWindowVisibleFlag == TRUE ) AddExStyle |= WS_EX_TRANSPARENT ;

				// バックバッファの透過色の部分を透過させるフラグが立っているか、
				// UpdateLayerdWindow を使用するフラグが経っている場合は WS_EX_LAYERED を追加する
				if( WinData.BackBufferTransColorFlag == TRUE ||
					WinData.UseUpdateLayerdWindowFlag == TRUE ) AddExStyle |= WS_EX_LAYERED ;

				AddStyle = 0 ;
				// 非同期ウインドウモード変更機能が有効になっているときは WS_MAXIMIZEBOX を追加する
				if( WinData.UseChangeWindowModeFlag == TRUE ) AddStyle |= WS_MAXIMIZEBOX ;

				// 表示フラグが立っている場合は WS_VISIBLE を追加する
				if( WinData.VisibleFlag             == TRUE ) AddStyle |= WS_VISIBLE ;

				// ウインドウサイズが変更できるようになっている場合は WS_THICKFRAME を追加する
				if( WinData.WindowSizeChangeEnable  == TRUE ) AddStyle |= WS_THICKFRAME ;

				// マウス表示フラグを立てる
				WinData.MouseDispFlag = TRUE ;

				// クライアント領域の算出
				if( WinData.WindowSizeValid == TRUE )
				{
					if( WinData.ScreenNotFitWindowSize )
					{
						// フィット指定が無い場合はそのままのウインドウサイズをセット
						WindowSizeX = WinData.WindowWidth ;
						WindowSizeY = WinData.WindowHeight ;
					}
					else
					{
						// フィット指定がある場合は拡大率を指定する
						ExtendRateX = ( double )WinData.WindowWidth / WindowSizeX ;
						ExtendRateY = ( double )WinData.WindowHeight / WindowSizeY ;
						NS_SetWindowSizeExtendRate( ExtendRateX, ExtendRateY ) ;

						NS_GetWindowSizeExtendRate( &ExtendRateX, &ExtendRateY ) ;
						WindowSizeX = _DTOL( WindowSizeX * ExtendRateX ) ;
						WindowSizeY = _DTOL( WindowSizeY * ExtendRateY ) ;
					}
				}
				else
				{
					NS_GetWindowSizeExtendRate( &ExtendRateX, &ExtendRateY ) ;
					WindowSizeX = _DTOL( WindowSizeX * ExtendRateX ) ;
					WindowSizeY = _DTOL( WindowSizeY * ExtendRateY ) ;
				}

				// システムバーを含めたウインドウのサイズを取得
				SETRECT( Rect, 0, 0, WindowSizeX, WindowSizeY ) ;
				AdjustWindowRectEx( &Rect ,
									WStyle_WindowModeTable[WinData.WindowStyle]   + AddStyle, FALSE,
									WExStyle_WindowModeTable[WinData.WindowStyle] + AddExStyle );
				WinData.SystembarHeight = -Rect.top ;

				// 設定領域をちょいと細工
		//		Rect.left   += - 3;
		//		Rect.top    += - 3;
		//		Rect.right  += + 3;
		//		Rect.bottom += + 3;

				// 中心に持ってくる
				WindowSizeX = Rect.right  - Rect.left ;
				WindowSizeY = Rect.bottom - Rect.top  ;
				Rect.left   += ( GetSystemMetrics( SM_CXSCREEN ) - WindowSizeX ) / 2;
				Rect.top    += ( GetSystemMetrics( SM_CYSCREEN ) - WindowSizeY ) / 2;
				Rect.right  += ( GetSystemMetrics( SM_CXSCREEN ) - WindowSizeX ) / 2;
				Rect.bottom += ( GetSystemMetrics( SM_CYSCREEN ) - WindowSizeY ) / 2;

				// ウインドウの作成
				WinData.MainWindow = 
//					CreateWindowExA(
					CreateWindowEx(
						WExStyle_WindowModeTable[WinData.WindowStyle] + AddExStyle,
						WinData.ClassName ,
						WinData.WindowText ,
						WStyle_WindowModeTable[WinData.WindowStyle] + AddStyle,
						WinData.WindowPosValid == TRUE ? WinData.WindowX : Rect.left,
						WinData.WindowPosValid == TRUE ? WinData.WindowY : Rect.top,
						WindowSizeX,
						WindowSizeY,
						NULL,WinData.Menu,
						WinData.Instance,
						NULL );
			}
			else
			{

				DXST_ERRORLOG_ADD( _T( "フルスクリーンモード用のウインドウを作成します\n" ) ) ;
				// フルスクリーンモード時

				// マウス表示フラグを倒す
				WinData.MouseDispFlag = FALSE ;

				WinData.MainWindow = 
//					CreateWindowExA( 
					CreateWindowEx( 
						WExStyle_FullScreenModeTable[WinData.WindowStyle] ,
						WinData.ClassName ,
						WinData.WindowText ,
						WStyle_FullScreenModeTable[WinData.WindowStyle] ,
						0 ,	0 ,
						WindowSizeX , WindowSizeY ,
						NULL ,
						NULL , 
						WinData.Instance ,
						NULL ) ;
			}
			if( WinData.MainWindow == NULL )
			{
				WinData.QuitMessageFlag = TRUE;
				DxLib_EndRequest() ;
				return DXST_ERRORLOG_ADD( _T( "ウインドウの作成に失敗しました\n" ) ) ;
			}
			DXST_ERRORLOG_ADD( _T( "ウインドウの作成に成功しました\n" ) ) ;

			// ウインドウの表示、更新
			if( WinData.NotWindowVisibleFlag )
			{
				ShowWindow( WinData.MainWindow , SW_HIDE ) ;
			}
			else
			if( WinData.WindowMinimizeFlag )
			{
				ShowWindow( WinData.MainWindow , SW_MINIMIZE ) ;
				UpdateWindow( WinData.MainWindow ) ;
			}
			else
			{
				DXST_ERRORLOG_ADD( _T( "ウインドウを表示します\n" ) ) ;
				ShowWindow( WinData.MainWindow , SW_SHOW ) ;
				UpdateWindow( WinData.MainWindow ) ;
			}
		}


		// 各種データをセットする
		{
			WinData.CloseMessagePostFlag = FALSE ;							// WM_CLOSEメッセージが送られるとＴＲＵＥになるフラグ変数をＦＡＬＳＥにして置く
			WinData.DestroyMessageCatchFlag = FALSE ;						// WM_DESTROYメッセージを受け取った時にＴＲＵＥになるフラグ変数をＦＡＬＳＥにしておく
			WinData.ActiveFlag = TRUE ;										// アクティブフラグをたてる
			if( WinData.WindowModeFlag == FALSE )
			{
				DXST_ERRORLOG_ADD( _T( "カーソルを不可視にしました\n" ) ) ;
				SetCursor( NULL ) ;										// フルスクリーンモードだった場合カーソルを消去
			}
			DXST_ERRORLOG_ADD( _T( "ＩＭＥを無効にしました\n" ) ) ;

			if( WinData.WindowMinimizeFlag == FALSE 
#ifndef DX_NON_INPUTSTRING
				&& CharBuf.IMEUseFlag == FALSE
#endif // DX_NON_INPUTSTRING
			)
			{
				if( WinAPIData.WINNLSEnableIME_Func )
					WinAPIData.WINNLSEnableIME_Func( WinData.MainWindow , FALSE ) ;						// MS_IME無効
			}
			else
			{
#ifndef DX_NON_INPUTSTRING
				if( CharBuf.IMEUseFlag )
				{
					if( WinAPIData.WINNLSEnableIME_Func )
						WinAPIData.WINNLSEnableIME_Func( WinData.MainWindow , TRUE ) ;						// MS_IME無効
				}
#endif // DX_NON_INPUTSTRING
			}

			// ウインドウのクライアント領域を保存する
			if( WinData.WindowModeFlag )
			{
				GetClientRect( GetDisplayWindowHandle() , &WinData.WindowRect )  ;
				ClientToScreen( GetDisplayWindowHandle() , ( LPPOINT )&WinData.WindowRect ) ;
				ClientToScreen( GetDisplayWindowHandle() , ( LPPOINT )&WinData.WindowRect + 1 ) ;
			}
		}

		// マウスの表示設定を反映させる
		NS_SetMouseDispFlag( WinData.MouseDispFlag ) ;

		// 強制終了か，フラグを倒す
		WinData.AltF4_EndFlag = FALSE ;

		if( WinData.UserWindowFlag == FALSE )
		{
			// メインウインドウをアクティブにする
			if( WinData.NotWindowVisibleFlag == FALSE &&
				WinData.WindowMinimizeFlag == FALSE )
				BringWindowToTop( WinData.MainWindow ) ;

			// 少しだけ待つ
/*
			{
				int Time = NS_GetNowCount() ;
				while( NS_GetNowCount() - Time < 200 )
					NS_ProcessMessage() ;
			}
*/
			// サイズ補正処理
			if( WinData.NotWindowVisibleFlag == FALSE &&
				WinData.WindowMinimizeFlag == FALSE )
			{
				WM_SIZEProcess() ;
			}

			// ウインドウ作成中フラグを倒す
			WinData.WindowCreateFlag = FALSE ;
		}

		// 初期サイズ指定は完了しているのでフラグを倒す
		WinData.WindowPosValid = FALSE ;
	}

#ifdef DX_THREAD_SAFE
	// ProcessMessage をひたすら呼びつづけるスレッドを立てる
	WinData.ProcessMessageThreadHandle = CreateThread( NULL, 0, ProcessMessageThreadFunction, NULL, 0, &WinData.ProcessMessageThreadID ) ;
//	WinData.ProcessMessageThreadHandle = NULL ;
	if( WinData.ProcessMessageThreadHandle == NULL )
	{
		DXST_ERRORLOG_ADD( _T( "ProcessMessage をひたすら呼びつづけるスレッドの立ち上げに失敗しました\n" ) ) ;
	}
#endif

#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込み処理を行うスレッドを立てる
	if( SetupASyncLoadThread( WinData.ProcessorNum ) < 0 )
	{
		DXST_ERRORLOG_ADD( _T( "非同期読み込み処理を行うスレッドの立ち上げに失敗しました\n" ) ) ;
		WinData.QuitMessageFlag = TRUE;
		DxLib_EndRequest() ;
		return -1 ;
	}
#endif // DX_NON_ASYNCLOAD

	// 終了
	return 0 ;
}

#pragma optimize("", on)

// ウインドウ関係の処理終了関数
extern int TerminateWindow( void )
{
	// もし既にウインドウが無い場合は何もせず終了
	if( WinData.MainWindow == NULL ) return 0 ;

	// スクリーンセーバー有効
	SystemParametersInfo( SPI_SETSCREENSAVEACTIVE, TRUE, NULL, SPIF_SENDWININICHANGE ) ;

	// ウインドウに関連する処理は DxSysData.NotWinFlag が立っていない場合のみ実行する
	if( DxSysData.NotWinFlag == FALSE )
	{
		// MS_IME有効
		if( WinAPIData.WINNLSEnableIME_Func )
			WinAPIData.WINNLSEnableIME_Func( WinData.MainWindow, TRUE ) ;

		// マウスカーソルを表示する
		NS_SetMouseDispFlag( TRUE ) ;

		// タスクスイッチの有効、無効をセットする
		{
	/*		if( WinData.WindowsVersion < DX_WINDOWSVERSION_NT31 )
			{
				// Win95 カーネルの場合の処理
				UINT nPreviousState;
				if( WinData.SysCommandOffFlag == TRUE )
					SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, FALSE, &nPreviousState, 0 ) ;
			}
			else
	*/		{
				// WinNT カーネルの場合の処理

				// キーボードフックを無効に
				if( WinData.TaskHookHandle != NULL )
				{
					UnhookWindowsHookEx( WinData.TaskHookHandle ) ;
					WinData.TaskHookHandle = NULL ;
				}

				// メッセージフックを無効に
				if( WinData.GetMessageHookHandle != NULL )
				{
					UnhookWindowsHookEx( WinData.GetMessageHookHandle ) ;
					UnhookWindowsHookEx( WinData.KeyboardHookHandle ) ;
					FreeLibrary( WinData.MessageHookDLL ) ;
					WinData.GetMessageHookHandle = NULL ;
					WinData.KeyboardHookHandle = NULL ;
				}

				// キーボードフックＤＬＬとしてテンポラリファイルを使用していた場合はファイルを削除する
				if( WinData.NotUseUserHookDllFlag )
				{
					DeleteFile( WinData.HookDLLFilePath ) ;
					WinData.NotUseUserHookDllFlag = FALSE ;
				}
			}
		}

		// リージョンを削除する
		NS_SetWindowRgnGraph( NULL ) ;

		// メニューを無効にする
		NS_SetUseMenuFlag( FALSE ) ;

		// ツールバーを無効にする
		NS_SetupToolBar( NULL, 0 ) ;

		// ユーザーのウインドウを使用しているかどうかで処理を分岐
		if( WinData.UserWindowFlag == TRUE )
		{
			// ウインドウプロージャを元に戻す
#ifdef _WIN64
			SetWindowLongPtr( WinData.MainWindow, GWLP_WNDPROC, ( LONG_PTR )WinData.DefaultUserWindowProc ) ;
#else
			SetWindowLong( WinData.MainWindow, GWL_WNDPROC, (LONG)WinData.DefaultUserWindowProc ) ;
#endif

			// 終了状態にする
			WinData.QuitMessageFlag = TRUE ;
			DxLib_EndRequest() ;
		}
		else
		{
			// クローズフラグが倒れていたらWM_CLOSEメッセージを送る
			if( WinData.CloseMessagePostFlag == FALSE )
			{
				WinData.CloseMessagePostFlag = TRUE ;
				PostMessage( WinData.MainWindow , WM_CLOSE, 0, 0 );
			}

			// エンド処理が終るまでループ
			while( NS_ProcessMessage() == 0 && WinData.DestroyMessageCatchFlag == FALSE ){}

			// レジストリの削除
			UnregisterClass( WinData.ClassName, WinData.Instance ) ;
		}

	#ifndef DX_NON_KEYEX
		// キー入力ハンドルの初期化
		NS_InitKeyInput() ;
	#endif

	/*
		// 背景用ＢＭＰオブジェクトの削除
		if( WinData.PauseGraph )
		{
			DeleteObject( ( HGDIOBJ )WinData.PauseGraph ) ;
			WinData.PauseGraph = NULL ;
		}o
	*/
		// 背景用ＢＭＰオブジェクトの削除
		if( WinData.PauseGraph.GraphData != NULL )
		{
			NS_ReleaseGraphImage( &WinData.PauseGraph ) ;
			NS_DeleteGraph( WinData.PauseGraphHandle ) ;
			WinData.PauseGraphHandle = 0 ;
		}

		// ドラッグ＆ドロップされたファイル名のバッファを解放
		NS_DragFileInfoClear() ;
	}
	else
	{
		WinData.QuitMessageFlag = TRUE;
		DxLib_EndRequest() ;
	}

#ifdef DX_THREAD_SAFE

	// ProcessMessage をひたすら呼びつづけるスレッドを閉じる
	if( WinData.ProcessMessageThreadHandle != NULL )
	{
		// スレッドが終了するまで待つ
		while( WinData.ProcessMessageThreadExitFlag == 0 )
		{
			Sleep( 1 ) ;
		}

		// スレッドのハンドルを閉じる
		CloseHandle( WinData.ProcessMessageThreadHandle ) ;
	}

#endif

#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込み処理用のスレッドを閉じる
	CloseASyncLoadThread() ;
#endif // DX_NON_ASYNCLOAD

	// ファイルアクセス処理の後始末
	TerminateFile() ;

#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込み処理の後始末
	TerminateASyncLoad() ;
#endif // DX_NON_ASYNCLOAD
	
	// タイマの精度を元に戻す
	{
		TIMECAPS tc ;

		WinAPIData.Win32Func.timeGetDevCapsFunc( &tc , sizeof(TIMECAPS) );

		// マルチメディアタイマーのサービス精度を最大に
		WinAPIData.Win32Func.timeEndPeriodFunc( tc.wPeriodMin ) ;
	}

	// バックバッファの透過色の部分を透過させるためのビットマップがある場合は削除する
	if( WinData.BackBufferTransBitmap )
	{
		DeleteObject( WinData.BackBufferTransBitmap ) ;
		WinData.BackBufferTransBitmap = NULL ;
	}

	// メインウインドウハンドルにヌルをセットする
	//WinData.MainWindow = NULL ;

	// 浮動小数点の計算精度を元に戻す
//	_control87( 0x00000000, 0x00030000 );

	// 終了
	return 0 ;
}







// ＣＯＭ初期化、終了関係関数

// ＣＯＭインターフェースを初期化する
extern	int	InitializeCom( void )
{
	if( WinData.ComInitializeFlag ) return -1 ;

	DXST_ERRORLOG_ADD( _T( "ＣＯＭの初期化... " ) ) ;

	// ＣＯＭの初期化
	if( FAILED( WinAPIData.Win32Func.CoInitializeExFunc( NULL, COINIT_APARTMENTTHREADED ) ) )
		return DXST_ERRORLOG_ADD( _T( "ＣＯＭの初期化に失敗しました\n" ) ) ;

	DXST_ERRORLOG_ADD( _T( "成功しました\n" ) ) ;

	WinData.ComInitializeFlag = TRUE ;

	// 終了
	return 0 ;
}

// ＣＯＭインターフェースを終了する
extern	int	TerminateCom( void )
{
	if( WinData.ComInitializeFlag == FALSE ) return -1 ;
	
	// ＣＯＭの終了
	WinAPIData.Win32Func.CoUninitializeFunc () ;

	DXST_ERRORLOG_ADD( _T( "ＣＯＭを終了... 完了\n" ) ) ;

	WinData.ComInitializeFlag = FALSE ;

	// 終了
	return 0 ;
}































































// 便利関数

// 指定のリソースを取得する( -1:失敗  0:成功 )
extern int NS_GetResourceInfo( const TCHAR *ResourceName, const TCHAR *ResourceType, void **DataPointerP, int *DataSizeP )
{
	HRSRC ResourceHandle = NULL ;
	HGLOBAL ResourceMem = NULL ;
	LPVOID ResourceData = NULL ;
	DWORD ResourceSize = 0 ;
	HMODULE ResourceModule ;

	// リソースハンドルを取得する
	ResourceModule = WinData.LoadResourModule == NULL ? GetModuleHandle( NULL ) : WinData.LoadResourModule ;
	ResourceHandle = FindResource( ResourceModule, ResourceName, ResourceType ) ;
	if( ResourceHandle == NULL ) return -1 ;

	// リソースのサイズを取得する
	ResourceSize = SizeofResource( ResourceModule, ResourceHandle ) ;
	if( ResourceSize == 0 ) return -1 ;

	// リソースをメモリに読み込む
	ResourceMem = LoadResource( ResourceModule, ResourceHandle ) ;
	if( ResourceMem == NULL ) return -1 ;

	// リソースデータが存在するメモリを取得する
	ResourceData = LockResource( ResourceMem ) ;
	if( ResourceData == NULL ) return -1 ;

	// 取得した情報を書き込む
	if( DataPointerP != NULL ) *DataPointerP = ResourceData ;
	if( DataSizeP != NULL ) *DataSizeP = ResourceSize ;

	// 終了
	return 0 ;
}

// リソースＩＤからリソースＩＤ文字列を得る 
extern const TCHAR *NS_GetResourceIDString( int ResourceID )
{
	return MAKEINTRESOURCE( ResourceID ) ;
}

#if defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )

// 同時にＤＸライブラリの関数が呼ばれていないかチェックして、同時に呼ばれていたら待つ関数
extern void CheckConflictAndWaitDxFunction( void )
{
//	int Time ;
//	int num ;
	DWORD ThreadID ;
	HANDLE EventHandle ;

	// DxConflictWaitThreadID が初期化されていなかったら初期化する
	if( WinData.DxConflictWaitThreadIDInitializeFlag == FALSE )
	{
		int i ;

		// 初期化フラグを立てる
		WinData.DxConflictWaitThreadIDInitializeFlag = TRUE ;

		// 衝突時に使用するイベントの作成
		for( i = 0 ; i < MAX_THREADWAIT_NUM ; i ++ )
		{
			WinData.DxConflictWaitThreadID[ i ][ 1 ] = ( DWORD_PTR )CreateEvent( NULL, TRUE, FALSE, NULL ) ;
		}

		// クリティカルセクションも初期化する
		CriticalSection_Initialize( &WinData.DxConflictCheckCriticalSection ) ;
	}

	// この関数を呼び出したスレッドのＩＤを得る
	ThreadID = GetCurrentThreadId() ;

START :

	// この関数が使用中の場合は待つ
	CRITICALSECTION_LOCK( &WinData.DxConflictCheckCriticalSection ) ;
//	CheckConflict( &WinData.DxConflictCheckFlag ) ;

	// どのスレッドもＤＸライブラリの関数を使用していないか、
	// もしくは使用していてもそれが自分のスレッドだったら通過
	if( WinData.DxUseThreadFlag == FALSE || WinData.DxUseThreadID == ThreadID )
	{
		// 待っているスレッドが居て、それが自分のスレッドではない場合は待ち
		if( WinData.DxUseThreadFlag == FALSE && WinData.DxConflictWaitThreadNum != 0 && WinData.DxUseThreadID != ThreadID )
		{
			if( WinData.DxConflictWaitThreadID[ 0 ][ 0 ] != ThreadID )
			{
				goto WAIT ;
			}
			else
			{
				// もし待っていたのが自分だったらリストをスライドさせる
				if( WinData.DxConflictWaitThreadNum != 1 )
				{
					EventHandle = ( HANDLE )WinData.DxConflictWaitThreadID[ 0 ][ 1 ] ;
					_MEMCPY( &WinData.DxConflictWaitThreadID[ 0 ][ 0 ], &WinData.DxConflictWaitThreadID[ 1 ][ 0 ], sizeof( DWORD_PTR ) * 2 * ( WinData.DxConflictWaitThreadNum - 1 ) ) ;
					WinData.DxConflictWaitThreadID[ WinData.DxConflictWaitThreadNum - 1 ][ 1 ] = ( DWORD_PTR )EventHandle ;
					WinData.DxConflictWaitThreadID[ WinData.DxConflictWaitThreadNum - 1 ][ 0 ] = ( DWORD_PTR )0 ;
				}

				// 待っている数を減らす
				WinData.DxConflictWaitThreadNum -- ;
			}
		}

		// 情報をセット
		WinData.DxUseThreadFlag = TRUE ;
		WinData.DxUseThreadID = ThreadID ;

		// 使用中カウンタをインクリメントする
//		if( WinData.DxConflictCheckCounter != 0 )
//		{
//			DXST_ERRORLOG_ADD( _T( "エラー:衝突発生 No.0 \n" ) ) ;
//		}
		WinData.DxConflictCheckCounter ++ ;

		goto END ;
	}

	// そうじゃない場合は待ち
WAIT:

	// もし待っているスレッドが限界を越えていたら単純な待ち処理を行う
	if( WinData.DxConflictWaitThreadNum == MAX_THREADWAIT_NUM )
	{
		// とりあえずこの関数を使用中、フラグを倒す
		CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//		WinData.DxConflictCheckFlag -- ;

//		DXST_ERRORLOG_ADD( _T( "エラー:衝突発生 No.4 \n" ) ) ;

		// 少し寝る
		Sleep( 1 ) ;

		// 最初に戻る
		goto START ;
	}

	// 待ってるスレッドがいますよという情報を追加する
	WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum][0] = ThreadID ;
	EventHandle = (HANDLE)WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum][1] ;
	WinData.DxConflictWaitThreadNum ++ ;

	// とりあえずこの関数を使用中、フラグを倒す
	CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//	WinData.DxConflictCheckFlag -- ;

	// 自分の番が来るまで待つ
	WaitForSingleObject( EventHandle, INFINITE ) ;
	ResetEvent( EventHandle ) ;
/*
	Time = WinAPIData.Win32Func.timeGetTimeFunc() ;
//	while( WinData.DxUseThreadFlag == TRUE )
	while( WinData.DxUseThreadFlag == TRUE || WinData.DxConflictWaitThreadID[0] != ThreadID )
	{
		Sleep( 0 ) ;
		if( WinAPIData.Win32Func.timeGetTimeFunc() - Time > 3000 )
		{
			HANDLE fp ;
			DWORD WriteSize ;
//			const char *ErrorStr = "長時間停止しっぱなしです\n" ;
			char ErrorStr[128] ;
			char String[256] ;
			Time = WinAPIData.Win32Func.timeGetTimeFunc() ;
			
			_SPRINTF( ErrorStr, "長時間停止しっぱなしです con:%d consub:%d\n", WinData.DxConflictCheckCounter, WinData.DxConflictCheckCounterSub ) ;
			
			// ログ出力抑制フラグが立っていた場合は出力を行わない
			if( DxSysData.NotLogOutFlag == FALSE && WinData.LogOutDirectory[0] != '\0' )
			{
				char MotoPath[MAX_PATH] ;

				// エラーログファイルを開く
				FGETDIR( MotoPath ) ;
				FSETDIR( WinData.LogOutDirectory ) ;
				fp = CreateFile( WinData.LogFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
				if( fp != NULL )
				{
					SetFilePointer( fp, 0, NULL, FILE_END ) ;

					// 規定分だけタブを排出
					if( DxSysData.ErTabStop == FALSE )
					{
						int i ;	

						// タイムスタンプを出力
						if( WinData.NonUseTimeStampFlag == 0 )
						{
							_SPRINTF( String, "%d:", WinAPIData.Win32Func.timeGetTimeFunc() - DxSysData.LogStartTime ) ;
							WriteFile( fp, String, lstrlenA( String ), &WriteSize, NULL ) ;
						}

						for( i = 0 ; i < DxSysData.ErTabNum ; i ++ )
							String[i] = '\t' ;
						String[i] = '\0' ;
						WriteFile( fp, String, DxSysData.ErTabNum, &WriteSize, NULL ) ;
						OutputDebugString( String ) ;
					}

					// エラーログファイルに書き出す
					WriteFile( fp, ErrorStr, lstrlenA( String ), &WriteSize, NULL ) ;
//					fputs( ErrorStr , fp ) ;

					// エラーログをアウトプットに書き出す
					OutputDebugString( ErrorStr ) ;

					// 最後の文字が改行意外だった場合はタブストップフラグを立てる
					DxSysData.ErTabStop = ErrorStr[ lstrlenA( ErrorStr ) - 1 ] != '\n' ;

//					fclose( fp ) ;
					CloseHandle( fp ) ;
				}
				FSETDIR( MotoPath ) ;
			}
		}
	}
*/
	
	// 待ってるスレッドがあるよカウンタをデクリメントする
//	WinData.DxConflictWaitThreadCounter -- ;
//	if( WinData.DxConflictWaitThreadCounter < 0 )
//	{
//		WinData.DxConflictWaitThreadCounter = 0 ;
//	}

	// 最初に戻る
	goto START ;

END :

	// 使用中フラグを倒す
	CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//	WinData.DxConflictCheckFlag -- ;
}
#endif // defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )


#if defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )

// CheckConfictAndWaitDxFunction を使用したＤＸライブラリの関数が return する前に呼ぶべき関数
extern void PostConflictProcessDxFunction( void )
{
	int WaitFlag = 0 ;
	
	// この関数が使用中の場合は待つ
	CRITICALSECTION_LOCK( &WinData.DxConflictCheckCriticalSection ) ;
//	CheckConflict( &WinData.DxConflictCheckFlag ) ;

	// カウンタが１の場合は、これでＤＸライブラリ使用中状態が
	// 解除されることになるので、処理を分岐
	if( WinData.DxConflictCheckCounter == 1 )
	{
		// 情報をリセットする
		WinData.DxUseThreadFlag = FALSE ;
		WinData.DxUseThreadID = 0xffffffff ;

		// もし待っているスレッドがある場合は、スリープするフラグを立てる
		if( WinData.DxConflictWaitThreadNum > 0 )
			WaitFlag = 1 ;
	}
//	else
//	{
//		DXST_ERRORLOG_ADD( _T( "エラー:衝突発生 No.1\n" ) ) ;
//	}

	// カウンタをデクリメントして終了
	WinData.DxConflictCheckCounter -- ;

	// この関数を使用中、フラグを倒す
	CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//	WinData.DxConflictCheckFlag -- ;

	// スリープするフラグが立っていたらスリープする
	if( WaitFlag == 1 )
	{
//		DWORD ThreadID = GetCurrentThreadId() ;

		// 次に実行すべきスレッドのイベントをシグナル状態にする
		SetEvent( (HANDLE)WinData.DxConflictWaitThreadID[0][1] ) ;

		// 寝る
		Sleep( 0 ) ;

/*		// 待ってるスレッドがあるよカウンタが０になるのを待つ
		while( WinData.DxConflictWaitThreadNum > 0 )
		{
			Sleep( 0 ) ;
		}
		Sleep( 0 ) ;
*/
	}
}
#endif // defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )




















// メッセージ処理関数

// WM_PAINT用関数
int DrawBackGraph( HDC /*DestDC*/ )
{
	int DrawScreenWidth, DrawScreenHeight, DrawScreen, DrawMode, WaitVSync ;
	int Width, Height, NonActiveRunFlag, BackUpScreen ;

	if( WinData.AltF4_EndFlag == 1 ) return 0 ;

	if( WinData.PauseGraph.GraphData == NULL ) return -1 ;

	NonActiveRunFlag = WinData.NonActiveRunFlag ;
	WinData.NonActiveRunFlag = TRUE ;

	// グラフィックハンドルが無効になっていたら作り直す
//	if( GetGraphData( WinData.PauseGraphHandle ) < 0 )
	if( IsValidGraphHandle( WinData.PauseGraphHandle ) == FALSE )
	{
		WinData.PauseGraphHandle = CreateGraphFromGraphImageBase( &WinData.PauseGraph, NULL, TRUE ) ;
		if( WinData.PauseGraphHandle < 0 )
		{
			WinData.NonActiveRunFlag = NonActiveRunFlag ;
			return -1 ;
		}
	}

	// 設定を一時的に変更する
	DrawScreen = NS_GetActiveGraph() ;
	DrawMode = NS_GetDrawMode() ;
	WaitVSync = NS_GetWaitVSyncFlag() ;
	NS_SetDrawScreen( DX_SCREEN_BACK ) ;
	NS_SetDrawMode( DX_DRAWMODE_BILINEAR ) ;
	NS_SetWaitVSyncFlag( FALSE ) ;

	// 現在のウインドウのクライアント領域のサイズを得る
	NS_GetDrawScreenSize( &DrawScreenWidth, &DrawScreenHeight ) ;

	// 画面一次保存用画像の作成
	{
		SETUP_GRAPHHANDLE_GPARAM GParam ;

		InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam, 32, FALSE, FALSE ) ;
		BackUpScreen = MakeGraph_UseGParam( &GParam, DrawScreenWidth, DrawScreenHeight, FALSE, FALSE ) ;
	}

	// 画面画像の取得
	NS_GetDrawScreenGraph( 0, 0, DrawScreenWidth, DrawScreenHeight, BackUpScreen ) ;

	// ポーズ画像のサイズを取得する
	NS_GetGraphSize( WinData.PauseGraphHandle, &Width, &Height ) ;

	// 画面一杯に描画する
	if( Width == DrawScreenWidth && Height == DrawScreenHeight )
	{
		NS_DrawGraph( 0, 0, WinData.PauseGraphHandle, FALSE ) ;
	}
	else
	{
		NS_DrawExtendGraph( 0, 0, DrawScreenWidth, DrawScreenHeight, WinData.PauseGraphHandle, FALSE ) ;
	}

	// 表画面にコピー
	NS_ScreenCopy() ;

	// 裏画面にもとの画像を描画する
	NS_DrawGraph( 0, 0, BackUpScreen, FALSE ) ;

	// 一次保存用画像の削除
	NS_DeleteGraph( BackUpScreen ) ;

	// 描画設定を元に戻す
	NS_SetDrawScreen( DrawScreen ) ;
	NS_SetDrawMode( DrawMode ) ;
	NS_SetWaitVSyncFlag( WaitVSync ) ;
	WinData.NonActiveRunFlag = NonActiveRunFlag ;

#if 0
	HBITMAP OldBmp ;
	BITMAP BmpData ;
//	SIZE DrawPoint ;
	HDC hdc ;
//	DWORD hr ;
	int DrawScreenWidth, DrawScreenHeight ;

	// メニュー作動中とそれ以外で処理を分岐
/*	if( WinData.MenuShredRunFlag == TRUE ) 
	{
		// 作動中の場合の処理

		// 裏画面にメニュー突入直前のグラフィックを描画する
		NS_SetDrawScreen( DX_SCREEN_BACK ) ;
		NS_DrawGraph( 0, 0, WinData.MenuPauseGraph, FALSE ) ;

		// 裏画面の内容を表画面に反映させる
		ScreenCopy() ;
	}
	else
*/	{
		// 作動中でない場合の処理
		if( ( hdc = CreateCompatibleDC( NULL ) ) == NULL ) return 0 ;

		OldBmp = ( HBITMAP )SelectObject( hdc , ( HGDIOBJ )WinData.PauseGraph ) ;

		// 描画
/*
		NS_GetDrawScreenSize( ( int * )&DrawPoint.cx , ( int * )&DrawPoint.cy ) ;
		GetObject( ( HGDIOBJ )WinData.PauseGraph , sizeof( BITMAP ) , ( void * )&BmpData ) ;
		DrawPoint.cx = ( DrawPoint.cx - BmpData.bmWidth ) / 2 ;
		DrawPoint.cy = ( DrawPoint.cy -  BmpData.bmHeight ) / 2 ;
		hr = BitBlt( DestDC , DrawPoint.cx , DrawPoint.cy , BmpData.bmWidth , BmpData.bmHeight ,
				hdc , 0 , 0 , SRCCOPY ) ;
*/
		// 現在のウインドウのクライアント領域のサイズを得る
		NS_GetDrawScreenSize( &DrawScreenWidth, &DrawScreenHeight ) ;

		// ポーズ画像の情報を取得する
		GetObject( ( HGDIOBJ )WinData.PauseGraph , sizeof( BITMAP ) , ( void * )&BmpData ) ;

		// ウインドウ一杯に描画する
		StretchBlt( DestDC,
					0, 0,
					_DTOL( DrawScreenWidth * WinData.WindowSizeExRateX ),
					_DTOL( DrawScreenHeight * WinData.WindowSizeExRateY ),
					
					hdc,
					0, 0,
					BmpData.bmWidth, BmpData.bmHeight,
					
					SRCCOPY ) ;

		// 終了処理	
		SelectObject( hdc , ( HGDIOBJ )OldBmp ) ;
		DeleteDC( hdc ) ;
	}
#endif

	// 終了
	return 0 ;
}


// メインウインドウのメッセージコールバック関数
extern LRESULT CALLBACK DxLib_WinProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LRESULT Ret = -1000, UserProcRet = 0 ;

	// メッセージ出力
#if 0
	{
		const char *MessageName = NULL ;
		const char *SubName = NULL ;

		switch( message )
		{
		case WM_SETCURSOR :				MessageName = "WM_SETCURSOR" ;				break ;
		case WM_DROPFILES :				MessageName = "WM_DROPFILES" ;				break ;
		case WM_CREATE :				MessageName = "WM_CREATE" ;					break ;
		case WM_COMMAND :				MessageName = "WM_COMMAND" ;				break ;
		case WM_IME_SETCONTEXT :		MessageName = "WM_IME_SETCONTEXT" ;			break ;
		case WM_IME_STARTCOMPOSITION :	MessageName = "WM_IME_STARTCOMPOSITION" ;	break ;
		case WM_IME_ENDCOMPOSITION :	MessageName = "WM_IME_ENDCOMPOSITION" ;		break ;
		case WM_IME_COMPOSITION :		MessageName = "WM_IME_COMPOSITION" ;		break ;
		case WM_IME_NOTIFY :			MessageName = "WM_IME_NOTIFY" ;				break ;
		case WM_SIZING :				MessageName = "WM_SIZING" ;					break ;
		case WM_SIZE :					MessageName = "WM_SIZE" ;					break ;
		case WM_DISPLAYCHANGE :			MessageName = "WM_DISPLAYCHANGE" ;			break ;
		case WM_NCHITTEST :				goto MESNAMEEND ; MessageName = "WM_NCHITTEST" ;				break ;
		case WM_LBUTTONDOWN :			MessageName = "WM_LBUTTONDOWN" ;			break ;
		case WM_RBUTTONDOWN :			MessageName = "WM_RBUTTONDOWN" ;			break ;
		case WM_MBUTTONDOWN :			MessageName = "WM_MBUTTONDOWN" ;			break ;
		case WM_MOUSEMOVE :				goto MESNAMEEND ; MessageName = "WM_MOUSEMOVE" ;				break ;
		case WM_LBUTTONUP :				MessageName = "WM_LBUTTONUP" ;				break ;
		case WM_RBUTTONUP :				MessageName = "WM_RBUTTONUP" ;				break ;
		case WM_MBUTTONUP :				MessageName = "WM_MBUTTONUP" ;				break ;
		case WM_PAINT :					MessageName = "WM_PAINT" ;					break ;
		case WM_SYSCHAR :				MessageName = "WM_SYSCHAR" ;				break ;
		case WM_CHAR :					MessageName = "WM_CHAR" ;					break ;
		case F10MES :					MessageName = "F10MES" ;					break ;
		case F12MES :					MessageName = "F12MES" ;					break ;
		case WM_KEYDOWN :				MessageName = "WM_KEYDOWN" ;				break ;
		case WM_MOUSEWHEEL :			MessageName = "WM_MOUSEWHEEL" ;				break ;
		case WM_MOUSEHWHEEL :			MessageName = "WM_MOUSEHWHEEL" ;			break ;
		case WM_MOVE :					goto MESNAMEEND ; MessageName = "WM_MOVE" ;					break ;
		case WM_MOVING :				goto MESNAMEEND ; MessageName = "WM_MOVING" ;					break ;
		case WM_ACTIVATEAPP :			MessageName = "WM_ACTIVATEAPP" ;			break ;
		case WM_ACTIVATE :				MessageName = "WM_ACTIVATE" ;				break ;
		case WM_CLOSE :					MessageName = "WM_CLOSE" ;					break ;
		case WM_DESTROY :				MessageName = "WM_DESTROY" ;				break ;
		case WM_QUIT :					MessageName = "WM_QUIT" ;					break ;
		case MM_MCINOTIFY :				MessageName = "MM_MCINOTIFY" ;				break ;
		case WSA_WINSOCKMESSAGE :		MessageName = "WSA_WINSOCKMESSAGE" ;		break ;
		case WM_ENTERSIZEMOVE :			MessageName = "WM_ENTERSIZEMOVE" ;			break ;
		case WM_EXITSIZEMOVE :			MessageName = "WM_EXITSIZEMOVE" ;			break ;
		case WM_ENTERMENULOOP :			MessageName = "WM_ENTERMENULOOP" ;			break ;
		case WM_EXITMENULOOP :			MessageName = "WM_EXITMENULOOP" ;			break ;
		case WM_NCRBUTTONDOWN :			MessageName = "WM_NCRBUTTONDOWN" ;			break ;

		case WM_SYSCOMMAND :
			MessageName = "WM_SYSCOMMAND" ;

			switch( wParam & 0xfff0 )
			{
			case SC_SIZE			: SubName = "SC_SIZE" ;				break ;
			case SC_MOVE			: SubName = "SC_MOVE" ;				break ;
			case SC_MINIMIZE		: SubName = "SC_MINIMIZE" ;			break ;
			case SC_MAXIMIZE		: SubName = "SC_MAXIMIZE" ;			break ;
			case SC_NEXTWINDOW		: SubName = "SC_NEXTWINDOW" ;		break ;
			case SC_PREVWINDOW		: SubName = "SC_PREVWINDOW" ;		break ;
			case SC_CLOSE			: SubName = "SC_CLOSE" ;			break ;
			case SC_VSCROLL			: SubName = "SC_VSCROLL" ;			break ;
			case SC_HSCROLL			: SubName = "SC_HSCROLL" ;			break ;
			case SC_MOUSEMENU		: SubName = "SC_MOUSEMENU" ;		break ;
			case SC_KEYMENU			: SubName = "SC_KEYMENU" ;			break ;
			case SC_ARRANGE			: SubName = "SC_ARRANGE" ;			break ;
			case SC_RESTORE			: SubName = "SC_RESTORE" ;			break ;
			case SC_TASKLIST		: SubName = "SC_TASKLIST" ;			break ;
			case SC_SCREENSAVE		: SubName = "SC_SCREENSAVE" ;		break ;
			case SC_HOTKEY			: SubName = "SC_HOTKEY" ;			break ;
			case SC_DEFAULT			: SubName = "SC_DEFAULT" ;			break ;
			case SC_MONITORPOWER	: SubName = "SC_MONITORPOWER" ;		break ;
			case SC_CONTEXTHELP		: SubName = "SC_CONTEXTHELP" ;		break ;
			case SC_SEPARATOR		: SubName = "SC_SEPARATOR" ;		break ;
			}
			DXST_ERRORLOGFMT_ADDA(( "HWND:%08x	Message:%s	WParam:%08d:%08xh	LParam:%08d:%08xh", hWnd, MessageName, wParam, wParam, lParam, lParam ));
			DXST_ERRORLOGFMT_ADDA(( "%s uCmdType:%s		xPos:%-5d yPos:%-5d", MessageName, SubName, LOWORD( lParam ), HIWORD( lParam ) ));
			goto MESNAMEEND ;

		case WM_NCMOUSEMOVE :			goto MESNAMEEND ; MessageName = "@ WM_NCMOUSEMOVE" ;			break ;
		case WM_GETMINMAXINFO :			MessageName = "@ WM_GETMINMAXINFO" ;		break ;
		case WM_NCLBUTTONDOWN :			MessageName = "@ WM_NCLBUTTONDOWN" ;		break ;
		case WM_NCACTIVATE :			MessageName = "@ WM_NCACTIVATE" ;			break ;
		case WM_KILLFOCUS :				MessageName = "@ WM_KILLFOCUS" ;			break ;
		case WM_WINDOWPOSCHANGING :		goto MESNAMEEND ; MessageName = "@ WM_WINDOWPOSCHANGING" ;	break ;
		case WM_WINDOWPOSCHANGED :		goto MESNAMEEND ; MessageName = "@ WM_WINDOWPOSCHANGED" ;		break ;
		case WM_SETFOCUS :				MessageName = "@ WM_SETFOCUS" ;				break ;
		case WM_NCMOUSELEAVE :			MessageName = "@ WM_NCMOUSELEAVE" ;			break ;
		case WM_SYSKEYDOWN :			MessageName = "@ WM_SYSKEYDOWN" ;			break ;
		case WM_SYSKEYUP :				MessageName = "@ WM_SYSKEYUP" ;				break ;
		case WM_INITMENU :				MessageName = "@ WM_INITMENU" ;				break ;
		case WM_MENUSELECT :			MessageName = "@ WM_MENUSELECT" ;			break ;
		case WM_ENTERIDLE :				goto MESNAMEEND ; MessageName = "@ WM_ENTERIDLE" ;			break ;
		case WM_CAPTURECHANGED :		MessageName = "@ WM_CAPTURECHANGED" ;		break ;
		case WM_STYLECHANGING :			MessageName = "@ WM_STYLECHANGING" ;		break ;
		case WM_STYLECHANGED :			MessageName = "@ WM_STYLECHANGED" ;			break ;
		case WM_NCCALCSIZE :			MessageName = "@ WM_NCCALCSIZE" ;			break ;
		case WM_NCPAINT :				MessageName = "@ WM_NCPAINT" ;				break ;
		case WM_ERASEBKGND :			MessageName = "@ WM_ERASEBKGND" ;			break ;
		case WM_NCDESTROY :				MessageName = "@ WM_NCDESTROY" ;			break ;
		case WM_NCCREATE :				MessageName = "@ WM_NCCREATE" ;				break ;

		default :
			MessageName = NULL ;
			break ;
		}

		if( MessageName )
		{
			DXST_ERRORLOGFMT_ADDA(( "HWND:%08x	Message:%s	WParam:%08d:%08xh	LParam:%08d:%08xh", hWnd, MessageName, wParam, wParam, lParam, lParam ));
		}
		else
		{
			DXST_ERRORLOGFMT_ADDA(( "HWND:%08x	Message:%08d:%08xh	WParam:%08d:%08xh	LParam:%08d:%08xh", hWnd, message, message, wParam, wParam, lParam, lParam ));
		}
	}
MESNAMEEND:
#endif

	// ユーザーから提供されたウインドウを使用している場合はそのウインドウのプロシージャを呼ぶ
	if( WinData.UserWindowFlag == TRUE )
	{
		if( WinData.DefaultUserWindowProc != NULL )
		{
			UserProcRet = WinData.DefaultUserWindowProc( hWnd, message, wParam, lParam ) ;
		}
	}

	// ユーザー定義のプロージャ関数があったらそれを呼ぶ
	if( WinData.UserWindowProc != NULL ) 
	{
		LRESULT RetValue ;

		WinData.UseUserWindowProcReturnValue = FALSE ;

		RetValue = WinData.UserWindowProc( hWnd, message, wParam, lParam ) ;

		// フラグが立っていたらここで終了
		if( WinData.UseUserWindowProcReturnValue )
		{
			return RetValue ;
		}
	}
	
	switch( message )
	{
	case WM_DROPFILES :
		// ファイルがドラッグ＆ドロップされた場合の処理
		{
			int FileNum, i, size ;
			HDROP hDrop = ( HDROP )wParam ;
			
			// ファイルの数を取得
			FileNum = ( int )DragQueryFile( hDrop, 0xffffffff, NULL, 0 ) ;

			// ファイルの数だけ繰り返し
			for( i = 0 ; i < FileNum && WinData.DragFileNum < MAX_DRAGFILE_NUM ; i ++, WinData.DragFileNum ++ )
			{
				// 必要なバッファのサイズを取得する
				size = DragQueryFile( hDrop, i, NULL, 0 ) ;
				
				// バッファの確保
				WinData.DragFileName[WinData.DragFileNum] = ( TCHAR * )DXALLOC( ( size + 1 ) * sizeof( TCHAR ) ) ;
				if( WinData.DragFileName[WinData.DragFileNum] == NULL ) break ;
				
				// ファイル名の取得
				DragQueryFile( hDrop, i, WinData.DragFileName[WinData.DragFileNum], size + 1 ) ;
			}
			
			// 取得の終了
			DragFinish( hDrop ) ;
		}
		break ;
	
	case WM_CREATE :
		// ウインドウが作成された直後の時の処理
		if( WinData.WindowRgn != NULL )
		{
			SetWindowRgn( hWnd, WinData.WindowRgn, FALSE ) ;
		}
		break ;

	case WM_COMMAND :
		// ツールバーのコマンドか調べる
		if( WinData.ToolBarUseFlag == TRUE && LOWORD( wParam ) >= TOOLBAR_COMMANDID_BASE )
		{
			int Index ;
			WINTOOLBARITEMINFO *but ;

			Index = SearchToolBarButton( LOWORD( wParam ) - TOOLBAR_COMMANDID_BASE ) ;
			if( Index != -1 )
			{
				but = &WinData.ToolBarItem[ Index ] ;

				// タイプが普通のボタンの場合のみ Clik を TRUE にする
				if( but->Type == TOOLBUTTON_TYPE_NORMAL )
					but->Click = TRUE ;

				// 終了
				break ;
			}
		}

		// メニューのコマンドか調べる
		if( WinData.MenuUseFlag == TRUE /*&& HIWORD( wParam ) == 0*/ )
		{
			WORD ItemID ;

			ItemID = LOWORD( wParam ) ;
			
			// コールバック関数が登録されている場合はコールバック関数を呼ぶ
			if( WinData.MenuProc != NULL )
			{
				WinData.MenuProc( ItemID ) ;
			}

			if( WinData.MenuCallBackFunction != NULL )
			{
				TCHAR NameBuffer[128] ;
				
				// 選択項目の名前を取得する
				NS_GetMenuItemName( (int)ItemID, NameBuffer ) ;
				
				// コールバック関数を呼ぶ
				WinData.MenuCallBackFunction( NameBuffer, (int)ItemID ) ;
			}

			// 選択された項目の情報に追加
			if( WinData.SelectMenuItemNum < MAX_MENUITEMSELECT_NUM )
			{
				int i ;
			
				// 既にあったら追加しない
				for( i = 0 ; i < WinData.SelectMenuItemNum ; i ++ )
					if( WinData.SelectMenuItem[i] == ItemID ) break ;

				// 無かった場合のみ追加
				if( i == WinData.SelectMenuItemNum )
				{
					WinData.SelectMenuItem[WinData.SelectMenuItemNum] = ItemID ;
					WinData.SelectMenuItemNum ++ ;
				}
			}
		}
		break ;

#ifndef DX_NON_KEYEX
	// ＩＭＥメッセージが来た場合は独自関数に渡す
	case WM_IME_SETCONTEXT :
	case WM_IME_STARTCOMPOSITION :
	case WM_IME_ENDCOMPOSITION :
	case WM_IME_COMPOSITION :
	case WM_IME_NOTIFY:
		return IMEProc( hWnd , message , wParam , lParam ) ;
#endif

	// ウインドウのサイズが変更されている時の処理
	case WM_SIZING :
		GetWindowRect( hWnd, &WinData.WindowEdgeRect ) ;

		// ウインドウモードの場合のみ処理
		if( WinData.WindowModeFlag == TRUE )
		{
			// ユーザーのウインドウを使用している場合は幅の調整はしない
			if( WinData.UserWindowFlag == FALSE )
			{
				WPARAM Side = wParam ;
				RECT *NextRect = (RECT *)lParam ;
				int Width, Height, NextWidth, NextHeight ;
				int AddWidth, AddHeight ;
				RECT WinRect, CliRect ;
				int WidthBigFlag ;

				// 描画画面のサイズを取得する
				NS_GetDrawScreenSize( &Width, &Height ) ;

				// 現在のウインドウの領域とクライアント領域を取得する
				GetWindowRect( hWnd, &WinRect ) ;
				GetClientRect( hWnd, &CliRect ) ;

				// クライアント領域以外の部分の幅と高さを算出
				AddWidth = ( WinRect.right - WinRect.left ) - ( CliRect.right - CliRect.left ) ;
				AddHeight = ( WinRect.bottom - WinRect.top ) - ( CliRect.bottom - CliRect.top ) ;

				// クライアント領域のサイズを得る
				NextWidth = ( NextRect->right - NextRect->left ) - AddWidth ;
				NextHeight = ( NextRect->bottom - NextRect->top ) - AddHeight ;
				if( NextWidth  + AddWidth  > WinData.DefaultScreenSize.cx ) NextWidth  = WinData.DefaultScreenSize.cx - AddWidth ;
				if( NextHeight + AddHeight > WinData.DefaultScreenSize.cy ) NextHeight = WinData.DefaultScreenSize.cy - AddHeight ;

				// クライアント領域に画面をフィットさせるかどうかで処理を分岐
				if( WinData.ScreenNotFitWindowSize == FALSE && WinData.WindowSizeValid == FALSE )
				{
					// フィットさせる場合

					// 幅と高さ、比率的にどちらがより大きいかを算出
					WidthBigFlag = ( (double)NextWidth / Width > (double)NextHeight / Height ) ? TRUE : FALSE ;

					// 新しい表示比率を設定する
					switch( Side )
					{
					case WMSZ_LEFT :
					case WMSZ_RIGHT :
			WIDTH_SIZE_BASE :
						if( NextWidth + AddWidth > WinData.DefaultScreenSize.cx ) NextWidth = WinData.DefaultScreenSize.cx - AddWidth ;
						WinData.WindowSizeExRateY =
						WinData.WindowSizeExRateX = (double)NextWidth / Width ;
						NextHeight = NextWidth * Height / Width ;
						break ;

					case WMSZ_TOP :
					case WMSZ_BOTTOM :
			HEIGHT_SIZE_BASE :
						if( NextHeight + AddHeight > WinData.DefaultScreenSize.cy ) NextHeight = WinData.DefaultScreenSize.cy - AddHeight ;
						WinData.WindowSizeExRateY =
						WinData.WindowSizeExRateX = (double)NextHeight / Height ;
						NextWidth = NextHeight * Width / Height ;
						break ;

					case WMSZ_TOPLEFT :
					case WMSZ_TOPRIGHT :
					case WMSZ_BOTTOMLEFT :
					case WMSZ_BOTTOMRIGHT :
						if( WidthBigFlag )
						{
							goto WIDTH_SIZE_BASE ;
						}
						else
						{
							goto HEIGHT_SIZE_BASE ;
						}
						break ;
					}

					// サイズが 128 以下にはならないようにする
					if( NextWidth < 128 || NextHeight < 128 )
					{
						if( Width > Height )
						{
							NextHeight = 128 ;
							NextWidth = NextHeight * Width / Height ;
						}
						else
						{
							NextWidth = 128 ;
							NextHeight = NextWidth * Height / Width ;
						}

						WinData.WindowSizeExRateY =
						WinData.WindowSizeExRateX = (double)NextWidth / Width ;
					}

					// ウインドウの形を補正する
					switch( Side )
					{
					case WMSZ_TOPLEFT :
						NextRect->left   = NextRect->right - ( NextWidth + AddWidth ) ;
						NextRect->top   = NextRect->bottom - ( NextHeight + AddHeight ) ;
						break ;

					case WMSZ_TOPRIGHT :
						NextRect->right = NextRect->left   + ( NextWidth + AddWidth ) ;
						NextRect->top   = NextRect->bottom - ( NextHeight + AddHeight ) ;
						break ;

					case WMSZ_LEFT :
						NextRect->left   = NextRect->right - ( NextWidth + AddWidth ) ;
						NextRect->bottom = NextRect->top   + ( NextHeight + AddHeight ) ;
						break ;

					case WMSZ_RIGHT :
						NextRect->right = NextRect->left   + ( NextWidth + AddWidth ) ;
						NextRect->bottom = NextRect->top   + ( NextHeight + AddHeight ) ;
						break ;

					case WMSZ_TOP :
						NextRect->right = NextRect->left   + ( NextWidth + AddWidth ) ;
						NextRect->top   = NextRect->bottom - ( NextHeight + AddHeight ) ;
						break ;

					case WMSZ_BOTTOM :
						NextRect->right = NextRect->left   + ( NextWidth + AddWidth ) ;
						NextRect->bottom = NextRect->top   + ( NextHeight + AddHeight ) ;
						break ;

					case WMSZ_BOTTOMLEFT :
						NextRect->left   = NextRect->right - ( NextWidth + AddWidth ) ;
						NextRect->bottom = NextRect->top   + ( NextHeight + AddHeight ) ;
						break ;

					case WMSZ_BOTTOMRIGHT :
						NextRect->right = NextRect->left   + ( NextWidth + AddWidth ) ;
						NextRect->bottom = NextRect->top   + ( NextHeight + AddHeight ) ;
						break ;
					}
				}
				else
				{
					int MainScreenW, MainScreenH ;

					// フィットさせない場合

					// 幅、高さが画面のサイズ以上になっていたら制限する
					NS_GetDrawScreenSize( &MainScreenW, &MainScreenH ) ;
					MainScreenW = _DTOL( MainScreenW * WinData.WindowSizeExRateX ) ;
					MainScreenH = _DTOL( MainScreenH * WinData.WindowSizeExRateY ) ;

					// ウインドウの形を補正する
					if( NextWidth > MainScreenW )
					{
						if( Side == WMSZ_RIGHT ||
							Side == WMSZ_TOPRIGHT ||
							Side == WMSZ_BOTTOMRIGHT )
						{
							NextRect->right = NextRect->left + ( MainScreenW + AddWidth ) ;
						}
						else
						{
							NextRect->left  = NextRect->right - ( MainScreenW + AddWidth ) ;
						}
					}

					if( NextHeight > MainScreenH )
					{
						if( Side == WMSZ_BOTTOM ||
							Side == WMSZ_BOTTOMLEFT ||
							Side == WMSZ_BOTTOMRIGHT )
						{
							NextRect->bottom = NextRect->top + ( MainScreenH + AddHeight ) ;
						}
						else
						{
							NextRect->top = NextRect->bottom - ( MainScreenH + AddHeight ) ;
						}
					}
/*
					DXST_ERRORLOGFMT_ADD(( _T( "left:%d top:%d right:%d bottom:%d" ),
						NextRect->left,
						NextRect->top,
						NextRect->right,
						NextRect->bottom )) ;
*/
				}
			}
		}
		break ;


	// ウインドウのサイズを一定に保つ処理
	case WM_SIZE :
		GetWindowRect( hWnd, &WinData.WindowEdgeRect ) ;

		// 最大化ではなかったら最大化状態フラグを倒す
		if( wParam != SIZE_MAXIMIZED )
		{
			WinData.WindowMaximizeFlag = FALSE ;
		}

		WinData.WindowMinSizeFlag = wParam == SIZE_MINIMIZED ? TRUE : FALSE ;

		// もし最大化された場合で且つ非同期ウインドウモード変更機能が有効になっている場合はウインドウモード変更処理を行う
		if( wParam == SIZE_MAXIMIZED && WinData.UseChangeWindowModeFlag )
		{
			// もしウインドウモード変更中やウインドウ作成直後、それかウインドウモードではなかった場合は何もせずに終了
			if( WinData.ChangeWindodwFlag == FALSE &&
				WinData.WindowCreateFlag == FALSE &&
				WinData.WindowModeFlag == TRUE )
			{
				// ウインドウモードからフルスクリーンモードに移行する
				WinData.ChangeWindowModeFlag = TRUE ;
			}
		}
		else
		{
			// 最大化だった場合でまだ最大化処理を行っておらず、且つクライアント領域に画面をフィットさせる場合はウインドウのスケールを変更する
			if( wParam == SIZE_MAXIMIZED && 
				WinData.WindowMaximizeFlag == FALSE /* &&
				WinData.ScreenNotFitWindowSize == FALSE &&
				WinData.WindowSizeValid == FALSE */ )
			{
//				RECT CliRect ;
//				int Width, Height, NextWidth, NextHeight ;

				// 描画画面のサイズを取得する
//				NS_GetDrawScreenSize( &Width, &Height ) ;

				// ウインドウのクライアント領域を取得する
				GetClientRect( hWnd, &WinData.WindowMaximizedClientRect ) ;

				// 新しいサイズを取得する
//				NextWidth  = CliRect.right  - CliRect.left ;
//				NextHeight = CliRect.bottom - CliRect.top ;

				// 倍率を算出する
//				WinData.WindowSizeExRateX = ( double )NextWidth / Width ;
//				WinData.WindowSizeExRateY = ( double )NextHeight / Height ;

				// 倍率の小さいほうに合わせる
//				if( WinData.WindowSizeExRateX < WinData.WindowSizeExRateY )
//				{
//					WinData.WindowSizeExRateY = WinData.WindowSizeExRateX ;
//				}
//				else
//				{
//					WinData.WindowSizeExRateX = WinData.WindowSizeExRateY ;
//				}

				// 最大化状態フラグを立てる
				WinData.WindowMaximizeFlag = TRUE ;
			}
			WM_SIZEProcess() ;
		}

		// ツールバーが有効な場合はツールバーにも送る
		if( WinData.ToolBarUseFlag == TRUE )
		{
			SendMessage( WinData.ToolBarHandle, WM_SIZE, wParam, lParam ) ;
		}
		break ;




	// 画面解像度変更時の処理
	case WM_DISPLAYCHANGE :
/*		// フルスクリーンモードで、既にメインウインドウ以外がアクティブになっていた
		// 場合は、更に別のウインドウがアクティブになりデスクトップ画面に戻ったと判断
		if( WinData.WindowModeFlag == FALSE && WinData.StopFlag == TRUE )
		{
			int Cx = LOWORD( lParam ), Cy = HIWORD( lParam ), Ccb = wParam ;
			int Sx, Sy, Scb ;

			// 元々のデスクトップ画面のサイズを得る
			NS_GetDefaultState( &Sx , &Sy , &Scb ) ;

			// もしデスクトップ画面と同じ画面モードであれば確実
			if( Scb == Ccb && Sx == Cx && Sy == Cy )
			{
				// DirectX 関連オブジェクトの一時的解放処理を行う
//				DestroyGraphSystem() ;
			}
		}
		else
*/
		// ウインドウモード時のエラー終了
		if( WinData.WindowModeFlag == TRUE && WinData.ChangeWindodwFlag == FALSE )
		{
			WPARAM ColorBit = wParam ;
			int Cx = LOWORD( lParam ) , Cy = HIWORD( lParam ) ;
			int Sx , Sy , Cb;

			// 望みの画面モードを取得
			NS_GetScreenState( &Sx, &Sy, &Cb ) ;

			// もし復元スレッドが登録されていない場合はソフト終了、２４ビットカラーモードの場合も終了
			if( ColorBit != ( UINT_PTR )Cb || Sx != Cx || Sy != Cy )
			{
				if( !NS_GetValidRestoreShredPoint() || ColorBit == 24 || 
					( ColorBit != ( UINT_PTR )Cb && ColorBit == 8 ) || Sx > Cx || Sy > Cy )
				{
					DXST_ERRORLOG_ADD( _T( "復元関数が登録されていないためか画面が２４ビットカラーに変更されたため終了します\n" ) ) ;
					DXST_ERRORLOG_ADD( _T( "またはスクリーンが小さすぎるためか色ビット数の違いが大きいためか終了します\n" ) ) ;
				
					// クローズフラグが倒れていたらWM_CLOSEメッセージを送る
					if( !WinData.CloseMessagePostFlag )
					{
						WinData.CloseMessagePostFlag = TRUE ;
						PostMessage( WinData.MainWindow , WM_CLOSE, 0, 0 );
					}

					break ;
				}

#ifndef DX_NON_MOVIE
				// 再生中のムービーオブジェクトをすべて止める
				DisableMovieAll() ;

				// ムービーグラフィックの再生
				PlayMovieAll() ;
#endif

//				MessageBox( hWnd , "動作保証外の動作が発生したため終了します" , "エラー" , MB_OK ) ;
//				return DxLib_Error( DXSTRING( "ウインドウモード時にデスクトップの解像度が変更されました、終了します" ) ) ;
			}
		}

		WinData.DisplayChangeMessageFlag = TRUE ;
		break ;

	// メニューに重なっているかテスト
	case WM_NCHITTEST :
		if( GRA2.ValidHardWare && WinData.WindowModeFlag == FALSE )
		{
			if( HIWORD( lParam ) < 8 )
				return HTMENU ;
		}
		break ;

	// ウインドウのサイズや位置の変更が開始されるか、システムメニューの選択が開始されたら非アクティブと同じ扱いにする
	case WM_ENTERSIZEMOVE :
	case WM_ENTERMENULOOP :
		if( WinData.WindowModeFlag == FALSE )
			break ;

		if( WinData.NotActive_WindowMoveOrSystemMenu == FALSE )
		{
			WinData.NotActive_WindowMoveOrSystemMenu = TRUE ;
			WM_ACTIVATEProcess( 0, 0 ) ;
		}
		break ;

	// タイトルバー上で右クリックがされた場合は非アクティブ扱いにする
	case WM_NCRBUTTONDOWN :
		if( WinData.WindowModeFlag == FALSE )
			break ;

		if( WinData.NotActive_WindowMoveOrSystemMenu == FALSE )
		{
			WinData.NotActive_WindowMoveOrSystemMenu = TRUE ;
			WM_ACTIVATEProcess( 0, 0 ) ;
		}
		break ;

	// ウインドウのサイズや位置の変更が開始されるか、システムメニューの選択が終了
	case WM_EXITSIZEMOVE :
	case WM_EXITMENULOOP :
		if( WinData.NotActive_WindowMoveOrSystemMenu )
		{
			WinData.NotActive_WindowMoveOrSystemMenu = FALSE ;
			WM_ACTIVATEProcessUseStock( 1, 0 ) ;
		}
		break ;

	// カーソル更新時
	case WM_SETCURSOR :
		if( WinData.NotActive_WindowMoveOrSystemMenu )
		{
			WinData.NotActive_WindowMoveOrSystemMenu = FALSE ;
			WM_ACTIVATEProcessUseStock( 1, 0 ) ;
		}

		if( GRA2.ValidHardWare && WinData.WindowModeFlag == FALSE && WinData.MenuUseFlag == TRUE )
		{
			if( HTMENU == LOWORD( lParam ) || HTCAPTION == LOWORD( lParam ) )
			{
				if( WinData.MousePosInMenuBarFlag < 5 )
				{
					SetD3DDialogBoxMode( TRUE ) ;
					DrawMenuBar( hWnd ) ;
					WinData.MousePosInMenuBarFlag ++ ;
					NS_SetMouseDispFlag( -1 ) ;
				}
			}
			else
			{
				if( WinData.MousePosInMenuBarFlag > 0 && HTNOWHERE != LOWORD( lParam ) )
				{
					SetD3DDialogBoxMode( FALSE ) ;
					WinData.MousePosInMenuBarFlag = FALSE ;
				}
			}
		}

	case WM_LBUTTONDOWN :
	case WM_RBUTTONDOWN :
	case WM_MBUTTONDOWN :
	case WM_MOUSEMOVE :
	case WM_LBUTTONUP :
	case WM_RBUTTONUP :
	case WM_MBUTTONUP :

#ifndef DX_NON_INPUT
		// ボタン入力コードをセットする
		switch( message )
		{
		case WM_LBUTTONDOWN : StockMouseInputInfo( MOUSE_INPUT_LEFT   ) ; break ;
		case WM_RBUTTONDOWN : StockMouseInputInfo( MOUSE_INPUT_RIGHT  ) ; break ;
		case WM_MBUTTONDOWN : StockMouseInputInfo( MOUSE_INPUT_MIDDLE ) ; break ;
		}
#endif // DX_NON_INPUT

		// マウスを使用しない設定になっていた場合マウスカーソルを消す
		if( /*!WinData.WindowModeFlag &&*/ /*!WinData.MouseDispFlag*/ WinData.MouseDispState == FALSE )
		{
			SetCursor( NULL ) ;
			while( ShowCursor( FALSE ) > -1 ){} ;
		}
		break ;

	// 再描画メッセージ
	case WM_PAINT :
		{
			PAINTSTRUCT PaintStr ;

			// WM_PAINT メッセージが来たことを記録しておく
			WinData.RecvWM_PAINTFlag = TRUE;

			if( BeginPaint( hWnd , &PaintStr ) == NULL ) break ;

			// ウインドウモードの場合のみ処理を行う
			if( WinData.WindowModeFlag == TRUE )
			{
				WinData.WM_PAINTFlag = TRUE ;

				// 非アクティブで、且つ非アクティブ時に表示する専用の画像がある場合はそれを描画する
				if( ( WinData.ActiveFlag == FALSE && WinData.PauseGraph.GraphData != NULL && WinData.NonActiveRunFlag == FALSE ) /*|| 
					( WinData.MenuUseFlag == TRUE && WinData.MenuShredRunFlag == TRUE )*/ )
				{
					// バックグラウンド描画
					if( WinData.WindowMinSizeFlag == FALSE ) 
					{
						DrawBackGraph( PaintStr.hdc ) ;
					}
				}
				else
				{
					// それ以外の場合

					// ＤＸライブラリを初期化中で、且つリージョンが設定されて
					// いない場合は画面を黒く初期化する
					if( DxSysData.DxLib_InitializeFlag == FALSE && WinData.WindowRgn == NULL )
					{
						// バックグラウンドカラーが有効な場合はその色を使用する
						if( GRA2.EnableBackgroundColor == TRUE )
						{
							HBRUSH Brush;

							Brush = CreateSolidBrush( ( GRA2.BackgroundBlue << 16 ) | ( GRA2.BackgroundGreen << 8 ) | GRA2.BackgroundRed );
							if( Brush != NULL )
							{
								FillRect( PaintStr.hdc, &PaintStr.rcPaint, Brush ) ;
								DeleteObject( Brush );
							}
							else
							{
								FillRect( PaintStr.hdc, &PaintStr.rcPaint, (HBRUSH)GetStockObject( BLACK_BRUSH ) ) ;
							}
						}
						else
						{
							FillRect( PaintStr.hdc, &PaintStr.rcPaint, (HBRUSH)GetStockObject( BLACK_BRUSH ) ) ;
						}
					}
					else
					{
						// とりあえず裏画面をコピーしておく
						if( NS_GetActiveGraph() == DX_SCREEN_BACK )
						{
							RECT ClientRect ;
							int waitVsync = NS_GetWaitVSyncFlag();
							NS_SetWaitVSyncFlag( FALSE );
							GetClientRect( GetDisplayWindowHandle(), &ClientRect ) ;

							if( PaintStr.rcPaint.left == ClientRect.left && 
								PaintStr.rcPaint.right == ClientRect.right &&
								PaintStr.rcPaint.top == ClientRect.top &&
								PaintStr.rcPaint.bottom == ClientRect.bottom )
							{
								NS_ScreenCopy() ;
							}
							else
							{
								/*
								DXST_ERRORLOGFMT_ADD(( _T( "left:%d top:%d right:%d bottom:%d" ),
									PaintStr.rcPaint.left,
									PaintStr.rcPaint.top,
									PaintStr.rcPaint.right,
									PaintStr.rcPaint.bottom )) ;
								ScreenFlipBase( &PaintStr.rcPaint ) ;
								*/
								if( WinData.ScreenCopyRequestFlag == FALSE )
								{
									WinData.ScreenCopyRequestFlag = TRUE ;
									WinData.ScreenCopyRequestStartTime = NS_GetNowCount() ;
								}
								else
								{
									if( NS_GetNowCount() - WinData.ScreenCopyRequestStartTime > 1000 / 60 * 4 )
									{
										NS_ScreenCopy() ;
										WinData.ScreenCopyRequestStartTime = NS_GetNowCount() ;
									}
								}
							}
							NS_SetWaitVSyncFlag( waitVsync );
						}
					}
				}

				WinData.WM_PAINTFlag = FALSE ;
			}
			EndPaint( hWnd , &PaintStr ) ;
		}
		break ;

	// システム文字コードメッセージ
	case WM_SYSCHAR :
		{
			// もしALT+ENTERがおされ、非同期ウインドウモード変更機能が有効になっていたら
			if( (TCHAR)wParam == CTRL_CODE_CR )
			{
				// ウインドウモード変更フラグを立てる
				if( WinData.UseChangeWindowModeFlag )
				{
					WinData.ChangeWindowModeFlag = TRUE ;
					return 0 ;
				}
			}
		}
		break ;

#ifndef DX_NON_INPUTSTRING
	// 文字コードメッセージ
	case WM_CHAR :

		// 文字をバッファにコピー
		if( wParam == 10 )
		{
			NS_StockInputChar( ( TCHAR )CTRL_CODE_CR ) ;
		}
		else
		{
			if( wParam >= 0x20            || 
				wParam == CTRL_CODE_BS    || 
				wParam == CTRL_CODE_TAB   || 
				wParam == CTRL_CODE_CR    || 
				wParam == CTRL_CODE_DEL   || 
				wParam == CTRL_CODE_ESC   ||
				wParam == CTRL_CODE_COPY  ||
				wParam == CTRL_CODE_PASTE ||
				wParam == CTRL_CODE_CUT   ||
				wParam == CTRL_CODE_ALL   )
				NS_StockInputChar( ( TCHAR )wParam ) ;
		}

		break ;
#endif // DX_NON_INPUTSTRING

	case WM_SYSCOMMAND :
		if( ( wParam & 0xfff0 ) == SC_MOVE )
		{
			if( WinData.NotActive_WindowMoveOrSystemMenu == FALSE )
			{
				WinData.NotActive_WindowMoveOrSystemMenu = TRUE ;
				WM_ACTIVATEProcess( 0, 0 ) ;
			}
		}

		if( ( wParam & 0xfff0 ) == SC_KEYMENU && WinData.SysCommandOffFlag == TRUE )
			return 0 ;

		if( ( wParam & 0xfff0 ) == SC_MONITORPOWER )
			return 1 ;		// 省電力モードを防止する

		if( ( wParam & 0xfff0 ) == SC_SCREENSAVE )
			return 1 ;		// スクリーンセーバーの起動を防止する
//		if( wParam == SC_CLOSE && WinData.SysCommandOffFlag == TRUE )
//			return 0 ;
		break ;

/*	case WM_SYSKEYDOWN :
		{
			int VKey = ( int )wParam ;

			if( VKey == VK_F10 )
			{
				SetF10Input() ;
			}
		}
		break ;

	case WM_SYSKEYUP :
		{
			int VKey = ( int )wParam ;

			if( VKey == VK_F10 )
			{
				ResetF10Input() ;
			}
		}
		break ;
*/

#ifndef DX_NON_INPUT
	// Ｆ１０メッセージ
	case F10MES :
		{
			if( wParam == 1 )
			{
				SetF10Input() ;
			}
			else
			{
				ResetF10Input() ;
			}
		}
		break ;

	// Ｆ１２メッセージ
	case F12MES :
		{
			if( wParam == 1 )
			{
				SetF12Input() ;
			}
			else
			{
				ResetF12Input() ;
			}
		}
		break ;
#endif // DX_NON_INPUT

#ifndef DX_NON_INPUTSTRING
	// キー押下メッセージ
	case WM_KEYDOWN:
		{
			int VKey = ( int )wParam ;
			char *CCode = ( char * )CtrlCode ;

			// バッファが一杯の場合はなにもしない
			if( ( CharBuf.EdPoint + 1 == CharBuf.StPoint ) ||
				( CharBuf.StPoint == 0 && CharBuf.EdPoint == CHARBUFFER_SIZE ) ) break ;

			// コントロール文字コードに対応するキーが
			// 押されていたらバッファに格納する
			while( *CCode )
			{
				if( *CCode == VKey )
				{
					// バッファに文字コードを代入
					NS_StockInputChar( ( TCHAR )*( CCode + 1 ) ) ;
					break ;
				}
				CCode += 2 ;
			}
		}
		break ;
#endif // DX_NON_INPUTSTRING

	// ホイールの移動量取得
	case WM_MOUSEWHEEL :
		{
			WinData.MouseMoveZ += (SHORT)HIWORD(wParam) ;
		}
		break ;

	// ホイールの横移動量取得
	case WM_MOUSEHWHEEL :
		{
			WinData.MouseMoveHZ += (SHORT)HIWORD(wParam) ;
		}
		break ;

	// ウインドウ移動時処理
	case WM_MOVE :
		GetWindowRect( hWnd, &WinData.WindowEdgeRect ) ;
		if( WinData.WindowModeFlag && !WinData.ChangeWindodwFlag )
		{
			WM_MOVEProcess( lParam ) ;
		}
		break ;

	// ウインドウ移動の抑制
	case WM_MOVING :
		GetWindowRect( hWnd, &WinData.WindowEdgeRect ) ;
		if( WinData.WindowModeFlag == FALSE )
		{
			RECT rect ;
			GetWindowRect( WinData.MainWindow , &rect ) ;
			*( RECT *)lParam = rect ;
		}
		break ;

	// アクティブ状態変化時
	case WM_ACTIVATEAPP :
		// 非アクティブになった場合のみ WM_ACTIVATE として処理する
		if( wParam == 0 ) goto ACTIVATELABEL ;
		break ;

	case WM_ACTIVATE :
ACTIVATELABEL:
#ifndef DX_NON_ASYNCLOAD
		/*
		// 非同期読み込みの途中の場合は、非同期読み込みが終了するまで待つ
		if( WinData.QuitMessageFlag == FALSE && WinData.CloseMessagePostFlag == FALSE )
		{
			while( NS_GetASyncLoadNum() > 0 )
			{
				ProcessASyncLoadRequestMainThread() ;
				if( WinData.ProcessorNum <= 1 )
				{
					Sleep( 2 ) ;
				}
				else
				{
					Sleep( 0 ) ;
				}
			}
		}
		*/
#endif // DX_NON_ASYNCLOAD

		WM_ACTIVATEProcessUseStock( wParam, message == WM_ACTIVATEAPP ? TRUE : FALSE ) ;
		break;

	// ウインドウクローズ時
	case WM_CLOSE :
		// ユーザー提供のウインドウだったら何もしない
		if( WinData.UserWindowFlag == FALSE )
		{
			DXST_ERRORLOG_ADD( _T( "ウインドウを閉じようとしています\n" ) ) ;

			// WM_DESTROYメッセージを生成
			if( WinData.NonUserCloseEnableFlag == FALSE || WinData.AltF4_EndFlag == 1 )
				DestroyWindow( WinData.MainWindow ) ;

			// 強制終了か判定
			if( WinData.AltF4_EndFlag == 0 ) WinData.AltF4_EndFlag = 4 ;

			return 0 ;
		}
		break ;

	// ウインドウ破棄時
	case WM_DESTROY :
		// ユーザー提供のウインドウだったら何もしない
		if( WinData.UserWindowFlag == FALSE )
		{
			DXST_ERRORLOG_ADD( _T( "ウインドウが破棄されようとしています\n" ) ) ;

			// WM_DESTROYメッセージを受けた証拠を残す
			WinData.DestroyMessageCatchFlag = TRUE ;

			// ソフトの終了メッセージを送る
			if( WinData.NonDxLibEndPostQuitMessageFlag == FALSE )
				PostQuitMessage( 0 ) ;
			WinData.QuitMessageFlag = TRUE ;
			DxLib_EndRequest() ;

			DXST_ERRORLOG_ADD( _T( "ソフトを終了する準備が整いました\n" ) ) ;
		}
		break ;

	case WM_QUIT :
		break ;

#ifndef DX_NON_SOUND
	// ＭＩＤＩ演奏終了時
	case MM_MCINOTIFY:	
		MidiCallBackProcess() ;					// ＭＩＤＩ演奏終了時の処理
		break;
#endif // DX_NON_SOUND

#ifndef DX_NON_NETWORK
	// ＷｉｎＳｏｃｋメッセージ
	case WSA_WINSOCKMESSAGE :
		return WinSockProc( hWnd , message , wParam , lParam ) ;
#endif

	}

	if( WinData.UserWindowFlag == TRUE )
	{
		if( Ret == -1000 )	return UserProcRet ;
		else				return DefWindowProc( hWnd , message , wParam , lParam ) ;
	}
	else
	{
		return DefWindowProc( hWnd , message , wParam , lParam ) ;
	}
}

// 非アクティブかどうかをチェックする
extern int CheckActiveWait( void )
{
	if(
		GBASE.ScreenFlipFlag   == FALSE &&
		WinData.WM_PAINTFlag   == FALSE &&
		( WinData.ActiveFlag   == FALSE || WinData.WindowMinSizeFlag == TRUE ) &&
		WinData.UserWindowFlag == FALSE &&
		DxSysData.NotWinFlag     == FALSE &&
		(
		  (
			WinData.WindowModeFlag   == TRUE  &&
		    WinData.NonActiveRunFlag == FALSE
		  ) ||	
		  (
			WinData.WindowModeFlag   == FALSE &&
		    WinData.NonActiveRunFlag == FALSE
		  )
		)
	  )
	{
		return TRUE ;
	}
	else
	{
		return FALSE ;
	}
}

// アクティブになるまで何もしない
extern void DxActiveWait( void )
{
	if( CheckActiveWait() == TRUE && WinData.QuitMessageFlag == FALSE && WinData.MainWindow != NULL )
	{
		WinData.WaitTime = NS_GetNowCount();
		WinData.WaitTimeValidFlag = TRUE ;
		do
		{
			if( NS_ProcessMessage() != 0 )
			{
				break ;
			}
		}while( WinData.ActiveFlag == FALSE || WinData.WindowMinSizeFlag == TRUE );
	}
}

// WM_QUIT が発行されているかどうかを取得する
extern int GetQuitMessageFlag( void )
{
	return WinData.QuitMessageFlag;
}




// ウインドウ関係情報取得関数

// ウインドウのクライアント領域を取得する
extern int NS_GetWindowCRect( RECT *RectBuf )
{
	int H;

	H = GetToolBarHeight();
	*RectBuf = WinData.WindowRect ;
	RectBuf->top    += H ;
	RectBuf->bottom += H ;
	
	return 0 ;
}

// ウインドウのアクティブフラグを取得
extern int NS_GetWindowActiveFlag( void )
{
	return WinData.ActiveFlag ;
}

// メインウインドウのハンドルを取得する
extern HWND NS_GetMainWindowHandle( void )
{
	return WinData.MainWindow ;
}

// ウインドウモードで起動しているか、のフラグを取得する
extern int NS_GetWindowModeFlag( void )
{
	return WinData.WindowModeFlag ;
}

// 起動時のデスクトップの画面モードを取得する
extern int NS_GetDefaultState( int *SizeX, int *SizeY, int *ColorBitDepth )
{
	if( DxSysData.DxLib_InitializeFlag == FALSE )
	{
		if( SizeX ) *SizeX = GetSystemMetrics( SM_CXSCREEN ) ;
		if( SizeY ) *SizeY = GetSystemMetrics( SM_CYSCREEN ) ;
		if( ColorBitDepth )
		{
			HDC hdc ;

			hdc = GetDC( NULL ) ;
			*ColorBitDepth = GetDeviceCaps( hdc , PLANES ) * GetDeviceCaps( hdc , BITSPIXEL ) ;
			ReleaseDC( NULL , hdc ) ;
		}
	}
	else
	{
		if( SizeX ) *SizeX = WinData.DefaultScreenSize.cx ;
		if( SizeY ) *SizeY = WinData.DefaultScreenSize.cy ;
		if( ColorBitDepth ) *ColorBitDepth = WinData.DefaultColorBitCount ;
	}

	// 終了
	return 0 ;
}

// ソフトがアクティブかどうかを取得する
extern int NS_GetActiveFlag( void )
{
	return WinData.ActiveFlag ;
}

// 非アクティブになり、処理が一時停止していたかどうかを取得する(引数 ResetFlag=TRUE:状態をリセット FALSE:状態をリセットしない    戻り値: 0=一時停止はしていない  1=一時停止していた )
extern int NS_GetNoActiveState( int ResetFlag )
{
	int Return;

	Return = WinData.WaitTimeValidFlag ;
	if( ResetFlag == TRUE )
	{
		WinData.WaitTimeValidFlag = FALSE ;
	}

	return Return ;
}

// マウスを表示するかどうかのフラグを取得する
extern int NS_GetMouseDispFlag( void )
{
	return WinData.MouseDispFlag ;
}

// ウインドウがアクティブではない状態でも処理を続行するか、フラグを取得する
extern int NS_GetAlwaysRunFlag( void )
{
	return WinData.NonActiveRunFlag ;
}

// ＤＸライブラリと DirectX のバージョンと Windows のバージョンを得る
extern int NS__GetSystemInfo( int *DxLibVer, int *DirectXVer, int *WindowsVer )
{
	if( DxLibVer ) *DxLibVer = DXLIB_VERSION ;
	if( DirectXVer ) *DirectXVer = WinData.DirectXVersion ;
	if( WindowsVer ) *WindowsVer = WinData.WindowsVersion ;

	// 終了
	return 0 ;
}

// ＰＣの情報を得る
extern int NS_GetPcInfo( TCHAR *OSString, TCHAR *DirectXString,
					TCHAR *CPUString, int *CPUSpeed/*単位MHz*/,
					double *FreeMemorySize/*単位MByte*/, double *TotalMemorySize,
					TCHAR *VideoDriverFileName, TCHAR *VideoDriverString,
					double *FreeVideoMemorySize/*単位MByte*/, double *TotalVideoMemorySize )
{
	if( OSString != NULL )				lstrcpy( OSString, WinData.PcInfo.OSString ) ;
	if( DirectXString != NULL )			lstrcpy( DirectXString, WinData.PcInfo.DirectXString ) ;
	if( CPUString != NULL )				lstrcpy( CPUString, WinData.PcInfo.CPUString ) ;
	if( CPUSpeed != NULL )				*CPUSpeed = WinData.PcInfo.CPUSpeed ;
	if( FreeMemorySize != NULL )		*FreeMemorySize = ( double )WinData.PcInfo.FreeMemorySize / 0x100000 ;
	if( TotalMemorySize != NULL )		*TotalMemorySize = ( double )WinData.PcInfo.TotalMemorySize / 0x100000 ;
	if( VideoDriverFileName != NULL )	lstrcpy( VideoDriverFileName, WinData.PcInfo.VideoDriverFileName ) ;
	if( VideoDriverString != NULL )		lstrcpy( VideoDriverString, WinData.PcInfo.VideoDriverString ) ;
	if( FreeVideoMemorySize != NULL )	*FreeVideoMemorySize = ( double )WinData.PcInfo.VideoFreeMemorySize / 0x100000 ;
	if( TotalVideoMemorySize != NULL )	*TotalVideoMemorySize = ( double )WinData.PcInfo.VideoTotalMemorySize / 0x100000 ;

	return 0 ;
}

// ＭＭＸが使えるかどうかの情報を得る
extern int NS_GetUseMMXFlag( void ) 
{
	return WinData.UseMMXFlag ;
}

// ＳＳＥが使えるかどうかの情報を得る
extern int NS_GetUseSSEFlag( void )
{
	return WinData.UseSSEFlag ;
}

// ＳＳＥ２が使えるかどうかの情報を得る
extern int NS_GetUseSSE2Flag( void )
{
	return WinData.UseSSE2Flag ;
}

// ウインドウを閉じようとしているかの情報を得る
extern int NS_GetWindowCloseFlag( void )
{
	return WinData.CloseMessagePostFlag ;
}

// ソフトのインスタンスを取得する
extern HINSTANCE NS_GetTaskInstance( void )
{
	return WinData.Instance ;
}

// リージョンを使っているかどうかを取得する
extern int NS_GetUseWindowRgnFlag( void )
{
	return WinData.WindowRgn != NULL ;
}

// ウインドウのサイズを変更できるかどうかのフラグを取得する
extern int NS_GetWindowSizeChangeEnableFlag( int *FitScreen )
{
	if( FitScreen ) *FitScreen = WinData.ScreenNotFitWindowSize == TRUE ? FALSE : TRUE ;
	return WinData.WindowSizeChangeEnable ;
}

// 描画画面のサイズに対するウインドウサイズの比率を取得する
extern double NS_GetWindowSizeExtendRate( double *ExRateX, double *ExRateY )
{
	if( WinData.WindowSizeExRateX <= 0.0 )
		WinData.WindowSizeExRateX = 1.0 ;

	if( WinData.WindowSizeExRateY <= 0.0 )
		WinData.WindowSizeExRateY = 1.0 ;

	// ウインドウモードかどうかで処理を分岐
	if( WinData.WindowModeFlag )
	{
		// 最大化状態の場合かどうかで処理を分岐
		if( WinData.WindowMaximizeFlag &&
			WinData.ScreenNotFitWindowSize == FALSE &&
			WinData.WindowSizeValid == FALSE )
		{
			int Width ;
			int Height ;
			int MaxCWidth ;
			int MaxCHeight ;
			double MaxCExRateX ;
			double MaxCExRateY ;

			// 画面のサイズを取得
			NS_GetDrawScreenSize( &Width, &Height ) ;

			// 最大化状態のクライアント領域のサイズを取得する
			MaxCWidth  = WinData.WindowMaximizedClientRect.right  - WinData.WindowMaximizedClientRect.left ;
			MaxCHeight = WinData.WindowMaximizedClientRect.bottom - WinData.WindowMaximizedClientRect.top ;

			// 計算誤差対応のため１ドットだけ小さいサイズにする
			MaxCWidth  -= 1 ;
			MaxCHeight -= 1 ;

			// 倍率を算出する
			MaxCExRateX = ( double )MaxCWidth  / Width ;
			MaxCExRateY = ( double )MaxCHeight / Height ;

			// 倍率の小さいほうに合わせる
			if( MaxCExRateX < MaxCExRateY )
			{
				MaxCExRateY = MaxCExRateX ;
			}
			else
			{
				MaxCExRateX = MaxCExRateY ;
			}

			if( ExRateX ) *ExRateX = MaxCExRateX ;
			if( ExRateY ) *ExRateY = MaxCExRateY ;
			return MaxCExRateX ;
		}
		else
		{
			if( ExRateX ) *ExRateX = WinData.WindowSizeExRateX ;
			if( ExRateY ) *ExRateY = WinData.WindowSizeExRateY ;
			return WinData.WindowSizeExRateX ;
		}
	}
	else
	{
		if( ExRateX ) *ExRateX = 1.0 ;
		if( ExRateY ) *ExRateY = 1.0 ;
		return 1.0 ;
	}
}

// ウインドウモードのウインドウのクライアント領域のサイズを取得する
extern int NS_GetWindowSize( int *Width, int *Height )
{
	RECT Rect ;

	GetClientRect( GetDisplayWindowHandle(), &Rect ) ;
	if( Width  ) *Width  = Rect.right  - Rect.left ;
	if( Height ) *Height = Rect.bottom - Rect.top  ;

	// 終了
	return 0 ;
}

// ウインドウモードのウインドウの位置を取得する( 枠も含めた左上座標 )
extern int NS_GetWindowPosition( int *x, int *y )
{
	RECT Rect ;

	GetWindowRect( WinData.MainWindow, &Rect ) ;
	if( x ) *x = Rect.left ;
	if( y ) *y = Rect.top ;

	// 終了
	return 0 ;
}


// ウインドウの閉じるボタンが押されたかどうかを取得する
extern int NS_GetWindowUserCloseFlag( int StateResetFlag )
{
	int Result ;

	Result = WinData.AltF4_EndFlag == 4 ? TRUE : FALSE ;

	// 状態をリセットしろフラグが立っていたらフラグをリセットする
	if( StateResetFlag == TRUE && WinData.AltF4_EndFlag == 4 && WinData.NonUserCloseEnableFlag == TRUE )
	{
		WinData.AltF4_EndFlag = FALSE ;
	}

	return Result ;
}

// WM_PAINT メッセージが来たかどうかを取得する
// (戻り値   TRUE:WM_PAINTメッセージが来た(一度取得すると以後、再び WM_PAINTメッセージが来るまで FALSE が返ってくるようになる)
//          FALSE:WM_PAINT メッセージは来ていない)
extern int NS_GetPaintMessageFlag( void )
{
	int Result;

	Result = WinData.RecvWM_PAINTFlag;
	WinData.RecvWM_PAINTFlag = FALSE;

	return Result;
}

// パフォーマンスカウンタが有効かどうかを取得する(戻り値  TRUE:有効  FALSE:無効)
extern int NS_GetValidHiPerformanceCounter( void )
{
	return WinData.PerformanceTimerFlag;
}













// Aero の有効、無効設定
extern int SetEnableAero( int Flag )
{
	// フラグが同じだったら何もしない
	if( WinData.AeroDisableFlag == !Flag )
		return 0 ;

	if( WinAPIData.DF_DwmEnableComposition )
	{
		WinAPIData.DF_DwmEnableComposition( Flag ) ;
	}

	// フラグを保存
	WinData.AeroDisableFlag = !Flag ;

	// 終了
	return 0 ;
}







// 設定関係関数

// ウインドウモードで起動するかをセット
extern int	SetWindowModeFlag( int Flag )
{
	// フラグを保存
	WinData.WindowModeFlag = Flag ;

	if( Flag ) 
	{
		DXST_ERRORLOG_ADD( _T( "ウインドウモードフラグが立てられました\n" ) ) ;

		// メニューが存在する時はメニューをセットする
		if( WinData.MenuUseFlag == TRUE ) 
		{
			SetMenu( WinData.MainWindow, WinData.Menu ) ;
		}
	}
	else
	{
		DXST_ERRORLOG_ADD( _T( "ウインドウモードフラグが倒されました\n" ) ) ;

		// メニューが存在する時はメニューを外す
		if( WinData.MenuUseFlag == TRUE ) 
		{
			SetMenu( WinData.MainWindow, NULL ) ;
		}
	}

	// 終了
	return 0 ;
}

// ウインドウのスタイルをセットする
extern int SetWindowStyle( void )
{
	RECT Rect ;
	int WindowSizeX, WindowSizeY ;
	int ClientRectWidth, ClientRectHeight ;

	if( WinData.MainWindow == NULL || WinData.UserWindowFlag == TRUE ) return 0 ;

	// クライアント領域のサイズを保存
	GetClientRect( WinData.MainWindow, &Rect ) ;
	ClientRectWidth = Rect.right - Rect.left ;
	ClientRectHeight = Rect.bottom - Rect.top ;

	// ユーザー提供のウインドウだったらパラメータ取得以外は何もしない
	if( WinData.UserWindowFlag == TRUE )
	{
		// ウインドウのクライアント領域を保存する
		GetClientRect( WinData.MainWindow, &WinData.WindowRect )  ;
		ClientToScreen( WinData.MainWindow, ( LPPOINT )&WinData.WindowRect ) ;
		ClientToScreen( WinData.MainWindow, ( LPPOINT )&WinData.WindowRect + 1 ) ;

		// ウインドウスタイルを変更
		if( WinData.WindowModeFlag == TRUE )	NS_SetMouseDispFlag( TRUE ) ;
		else									NS_SetMouseDispFlag( FALSE ) ;
	}
	else
	{
		// 描画領域のサイズを取得
		NS_GetDrawScreenSize( &WindowSizeX , &WindowSizeY ) ;
		if( GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 )
		{
			WindowSizeX = 640 ;
			WindowSizeY = 480 ;
		}

		// ウインドウスタイルを変更
		if( WinData.WindowModeFlag == TRUE )
		{
			LONG AddStyle, AddExStyle ;
			double ExtendRateX, ExtendRateY ;

			// ウインドウモードの場合

			DXST_ERRORLOG_ADD( _T( "ウインドウスタイルをウインドウモード用に変更します... " ) ) ;

			AddExStyle = 0 ;
			//if( WinData.NotWindowVisibleFlag == TRUE ) AddExStyle |= WS_EX_TRANSPARENT ;

			// バックバッファの透過色の部分を透過させるフラグが立っているか、
			// UpdateLayerdWindow を使用するフラグが経っている場合は WS_EX_LAYERED を追加する
			if( WinData.BackBufferTransColorFlag == TRUE ||
				WinData.UseUpdateLayerdWindowFlag == TRUE ) AddExStyle |= WS_EX_LAYERED ;

			AddStyle = 0 ;
			// 非同期ウインドウモード変更機能が有効になっているときは WS_MAXIMIZEBOX を追加する
			if( WinData.UseChangeWindowModeFlag == TRUE ) AddStyle |= WS_MAXIMIZEBOX ;

			// 表示フラグが立っている場合は WS_VISIBLE を追加する
			if( WinData.VisibleFlag == TRUE ) AddStyle |= WS_VISIBLE ;

			// ウインドウサイズが変更できるようになっている場合は WS_THICKFRAME を追加する
			if( WinData.WindowSizeChangeEnable == TRUE ) AddStyle |= WS_THICKFRAME ;

			// スタイルの変更
			SetWindowLong( WinData.MainWindow , GWL_EXSTYLE , WExStyle_WindowModeTable[ WinData.WindowStyle ] + AddExStyle ) ;
			SetWindowLong( WinData.MainWindow , GWL_STYLE   , WStyle_WindowModeTable[ WinData.WindowStyle ]   + AddStyle ) ;

			// クライアント領域の算出
			if( WinData.WindowSizeValid == TRUE )
			{
				WindowSizeX = WinData.WindowWidth ;
				WindowSizeY = WinData.WindowHeight ;
			}
			else
			if( WinData.ScreenNotFitWindowSize == TRUE )
			{
				WindowSizeX = ClientRectWidth ;
				WindowSizeY = ClientRectHeight ;
			}
			else
			{
				NS_GetWindowSizeExtendRate( &ExtendRateX, &ExtendRateY ) ;
				WindowSizeX = _DTOL( WindowSizeX * ExtendRateX ) ;
				WindowSizeY = _DTOL( WindowSizeY * ExtendRateY ) + GetToolBarHeight();
			}
			GetToolBarHeight();

			// 位置を変更
			SETRECT( Rect, 0, 0, WindowSizeX, WindowSizeY ) ;
			AdjustWindowRectEx( &Rect,
								WStyle_WindowModeTable[WinData.WindowStyle] + AddStyle, FALSE,
								WExStyle_WindowModeTable[WinData.WindowStyle] + AddExStyle );

			// 設定領域をちょいと細工
	//		Rect.left   += - 3;
	//		Rect.top    += - 3;
	//		Rect.right  += + 3;
	//		Rect.bottom += + 3;

			// 中心に持ってくる
			WindowSizeX = Rect.right  - Rect.left ;
			WindowSizeY = Rect.bottom - Rect.top  ;
			Rect.left   += ( WinData.DefaultScreenSize.cx - WindowSizeX ) / 2;
			Rect.top    += ( WinData.DefaultScreenSize.cy - WindowSizeY ) / 2;
			Rect.right  += ( WinData.DefaultScreenSize.cx - WindowSizeX ) / 2;
			Rect.bottom += ( WinData.DefaultScreenSize.cy - WindowSizeY ) / 2;

			// ウインドウの位置とサイズを変更
			SetWindowPos( WinData.MainWindow,
							HWND_NOTOPMOST,
							WinData.WindowPosValid == TRUE ? WinData.WindowX : Rect.left,
							WinData.WindowPosValid == TRUE ? WinData.WindowY : Rect.top,
							WindowSizeX,
							WindowSizeY,
							0/*SWP_NOZORDER*/ );
			SetWindowPos( WinData.MainWindow,
							HWND_TOP,
							WinData.WindowPosValid == TRUE ? WinData.WindowX : Rect.left,
							WinData.WindowPosValid == TRUE ? WinData.WindowY : Rect.top,
							WindowSizeX,
							WindowSizeY,
							0/*SWP_NOZORDER*/ );

			// ウインドウのクライアント領域を保存する
			GetClientRect( WinData.MainWindow , &WinData.WindowRect )  ;
			ClientToScreen( WinData.MainWindow , ( LPPOINT )&WinData.WindowRect ) ;
			ClientToScreen( WinData.MainWindow , ( LPPOINT )&WinData.WindowRect + 1 ) ;

			// 補正
			if( WinData.WindowRect.left < 0 || WinData.WindowRect.top < 0 )
			{
				int left, top ;

				if( WinData.WindowRect.left < 0 )
				{
					left = -WinData.WindowRect.left ;
					Rect.left   += left ;
					Rect.right  += left ;
					WinData.WindowRect.left   += left ;
					WinData.WindowRect.right  += left ;
				}

				if( WinData.WindowRect.top < 0 )
				{
					top  = -WinData.WindowRect.top  ;
					Rect.top    += top  ;
					Rect.bottom += top  ;
					WinData.WindowRect.top    += top  ;
					WinData.WindowRect.bottom += top  ;
				}

				SetWindowPos( WinData.MainWindow,
								HWND_TOP,
								WinData.WindowPosValid == TRUE ? WinData.WindowX : Rect.left,
								WinData.WindowPosValid == TRUE ? WinData.WindowY : Rect.top,
								WindowSizeX,
								WindowSizeY,
								0/*SWP_NOZORDER*/ );
			}
	
			NS_SetMouseDispFlag( TRUE ) ;
			DXST_ERRORLOG_ADD( _T( "完了\n" ) ) ;
		}
		else
		{
			DXST_ERRORLOG_ADD( _T( "ウインドウスタイルをフルスクリーンモード用に変更します... " ) ) ;

			if( GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 )
			{
				WindowSizeX = 640 ;
				WindowSizeY = 480 ;
			}

			// フルスクリーンモードの場合
			SetWindowLong( WinData.MainWindow, GWL_EXSTYLE, WExStyle_FullScreenModeTable[WinData.WindowStyle] ) ;
			SetWindowLong( WinData.MainWindow, GWL_STYLE  , WStyle_FullScreenModeTable[WinData.WindowStyle]  ) ;

			// ウインドウ矩形を変更
			SETRECT( WinData.WindowRect, 0, 0, WindowSizeX, WindowSizeY ) ;

			// ウインドウを表示
			ShowWindow( WinData.MainWindow , SW_SHOW ) ;
			UpdateWindow( WinData.MainWindow ) ;

			// Ｚオーダーの維持
			SetWindowPos( WinData.MainWindow, HWND_TOPMOST, 0, 0, WindowSizeX, WindowSizeY, /*SWP_NOSIZE | SWP_NOMOVE |*/ SWP_NOREDRAW ) ; 

			NS_SetMouseDispFlag( FALSE ) ;
			DXST_ERRORLOG_ADD( _T( "完了\n" ) ) ;
		}

		SetActiveWindow( WinData.MainWindow ) ;
	}

	// マウスのセット信号を出す
	PostMessage( WinData.MainWindow , WM_SETCURSOR , ( WPARAM )WinData.MainWindow , 0 ) ;

	// 終了
	return 0 ;
}

// 情報取得警官数

// ＰＣ情報構造体のアドレスを得る
extern PCINFO *GetPcInfoStructP( void ) 
{
	return &WinData.PcInfo ;
}


// ウインドウモードを変更する
extern int NS_ChangeWindowMode( int Flag )
{
	int Ret ;

	DXST_ERRORLOG_ADD( _T( "ChangeWindowMode実行 \n" ) ) ;

	// 今までと同じモードだった場合はなにもせず終了
	if( Flag == WinData.WindowModeFlag ) return 0 ;

	// フルスクリーンモード指定の場合は現在の画面解像度が使用可能かどうかを調べる
	if( Flag == FALSE && WinData.MainWindow != NULL && WinData.UserWindowFlag == FALSE )
	{
		int Num, i, Width, Height ;
		DISPLAYMODEDATA Mode ;

		NS_GetDrawScreenSize( &Width, &Height ) ; 

		Num = NS_GetDisplayModeNum() ;
		for( i = 0 ; i < Num ; i ++ )
		{
			Mode = NS_GetDisplayMode( i ) ;
			if( Mode.Width == Width && Mode.Height == Height )
				break ;
		}
		if( i == Num )
		{
			if( !( Width == 320 && Height == 240 && GRA2.NotUseHardWare == FALSE ) )
			{
				// 対応していない場合はエラー
				return -1 ;
			}
		}

		// 320x240 の解像度の場合は640x480の解像度で320x240の画面をエミュレーションする
		if( GRA2.MainScreenSizeX == 320 && GRA2.MainScreenSizeY == 240 )
		{
			GRH.FullScreenEmulation320x240 = TRUE ;
			SetMainScreenSize( 640, 480 ) ;
		}
	}

	WinData.VisibleFlag = TRUE ;

	// WM_DISPLAYCHANGE メッセージが来たかフラグを倒す
	WinData.DisplayChangeMessageFlag = FALSE ;

	// ウインドウモードフラグを変更する
	if( SetWindowModeFlag( Flag ) == -1 ) return -1 ;

	// まだウインドウさえ作成されていないか、ユーザーが作成したウインドウを使用している場合はここで終了
	if( WinData.MainWindow == NULL || WinData.UserWindowFlag == TRUE ) return 0 ;

	// ウインドウモード変更中フラグを立てる
	WinData.ChangeWindodwFlag = TRUE ;

	// ウインドウスタイルを変更する
	SetWindowStyle() ;

	// 画面モードを変更する
	Ret = ChangeGraphMode( -1, -1, -1, TRUE, -1 ) ;

	// ウインドウモード変更中フラグを倒す
	WinData.ChangeWindodwFlag = FALSE ;

	// フルスクリーンになった直後はメニューを表示していない状態にする
	if( Flag == FALSE )
	{
		WinData.MousePosInMenuBarFlag = FALSE ;
	}

	return Ret ;
}

// ＤＸライブラリの文字列処理で前提とする文字列セットを設定する
extern int NS_SetUseCharSet( int CharSet /* = DX_CHARSET_SHFTJIS 等 */ )
{
	switch( CharSet )
	{
	default :
	case DX_CHARSET_DEFAULT :
		_SET_CHARSET( DX_CHARSET_DEFAULT ) ;
		_SET_CODEPAGE( 0 ) ;
		break ;

	case DX_CHARSET_SHFTJIS :
		_SET_CHARSET( DX_CHARSET_SHFTJIS ) ;
		_SET_CODEPAGE( 932 ) ;
		break ;

	case DX_CHARSET_HANGEUL :
		_SET_CHARSET( DX_CHARSET_HANGEUL ) ;
		_SET_CODEPAGE( 949 ) ;
		break ;

	case DX_CHARSET_BIG5 :
		_SET_CHARSET( DX_CHARSET_BIG5 ) ;
		_SET_CODEPAGE( 950 ) ;
		break ;

	case DX_CHARSET_GB2312 :
		_SET_CHARSET( DX_CHARSET_GB2312 ) ;
		_SET_CODEPAGE( 936 ) ;
		break ;
	}

	// 終了
	return 0 ;
}

// アクティブウインドウが他のソフトに移っている際に表示する画像のロード(NULL で解除)
static int LoadPauseGraphToBase( const TCHAR *FileName, const void *MemImage, int MemImageSize )
{
	BASEIMAGE RgbImage ;

	// 画像データの情報がなかったら何もせず終了
	if( FileName == NULL && MemImage == NULL ) return 0 ;

	// 画像のロード
	if( FileName != NULL )
	{
		if( NS_CreateGraphImage_plus_Alpha( FileName, NULL, 0, LOADIMAGE_TYPE_FILE,
													  NULL, 0, LOADIMAGE_TYPE_FILE,
											&RgbImage, NULL, FALSE ) < 0 )
		{
			return -1 ;
		}
	}
	else
	{
		if( NS_CreateGraphImage_plus_Alpha( NULL, MemImage, MemImageSize, LOADIMAGE_TYPE_MEM, 
												  NULL, 	0,            LOADIMAGE_TYPE_MEM,
											&RgbImage, NULL, FALSE ) < 0 )
		{
			return -1 ;
		}
	}

	// すでにグラフィックがある場合は破棄
	if( WinData.PauseGraph.GraphData != NULL )
	{
		NS_ReleaseGraphImage( &WinData.PauseGraph ) ;
		NS_DeleteGraph( WinData.PauseGraphHandle ) ;
	}

	// 新しい画像データのセット
	WinData.PauseGraph = RgbImage ;

	// グラフィックハンドルを作成する
	WinData.PauseGraphHandle = CreateGraphFromGraphImageBase( &RgbImage, NULL, TRUE ) ;
/*
	// すでにグラフィックがある場合は破棄
	if( WinData.PauseGraph )
	{
		DeleteObject( ( HGDIOBJ )WinData.PauseGraph ) ;
		WinData.PauseGraph = 0 ;
	}


	if( FileName != NULL )
	{
		WinData.PauseGraph = NS_CreateDIBGraphVer2( FileName, NULL, 0, LOADIMAGE_TYPE_FILE, FALSE, NULL ) ;
	}
	else
	{
		WinData.PauseGraph = NS_CreateDIBGraphVer2( NULL, MemImage, MemImageSize, LOADIMAGE_TYPE_MEM, FALSE, NULL ) ;
	}
*/

	// 終了
	return 0 ;
}

// アクティブウインドウが他のソフトに移っている際に表示する画像のロード(NULL で解除)
extern int NS_LoadPauseGraph( const TCHAR *FileName )
{
	return LoadPauseGraphToBase( FileName, NULL, 0 ) ;
}

// アクティブウインドウが他のソフトに移っている際に表示する画像のロード(NULL で解除)
extern int NS_LoadPauseGraphFromMem( const void *MemImage, int MemImageSize )
{
	return LoadPauseGraphToBase( NULL, MemImage, MemImageSize ) ;
}

// ウインドウのアクティブ状態に変化があったときに呼ばれるコールバック関数をセットする( NULL をセットすると呼ばれなくなる )
extern int NS_SetActiveStateChangeCallBackFunction( int (*CallBackFunction)( int ActiveState, void *UserData ), void *UserData )
{
	// ポインタを保存
	WinData.ActiveStateChangeCallBackFunction = CallBackFunction ;
	WinData.ActiveStateChangeCallBackFunctionData = UserData ;

	// 終了
	return 0 ;
}

// メインウインドウのウインドウテキストを変更する
extern int NS_SetWindowText( const TCHAR *WindowText )
{
	return NS_SetMainWindowText( WindowText ) ;
}

// メインウインドウのウインドウテキストを変更する
extern int NS_SetMainWindowText( const TCHAR *WindowText )
{
	// テキストの保存
	lstrcpy( WinData.WindowText, WindowText ) ;

	// WindowText は有効、のフラグを立てる
	WinData.EnableWindowText = TRUE ;

	// メインウインドウが作られていない場合は保存のみ行う
	if( WinData.MainWindow )
	{
		// メインウインドウテキストの変更
		::SetWindowText( WinData.MainWindow , WinData.WindowText ) ;
	}

	// 終了
	return 0 ;
}

// メインウインドウのクラス名を設定する
extern int NS_SetMainWindowClassName( const TCHAR *ClassName )
{
	// メインウインドウが既に作成されている場合は設定不可
	if( WinData.MainWindow != NULL ) return -1;

	// テキストの保存
	lstrcpy( WinData.ClassName, ClassName ) ;

	// 終了
	return 0 ;
}

// ウインドウがアクティブではない状態でも処理を続行するか、フラグをセットする
extern int NS_SetAlwaysRunFlag( int Flag )
{
	// フラグをセット
	WinData.NonActiveRunFlag = Flag ;
	
	// 終了
	return 0 ;
}

// 使用するアイコンのＩＤをセットする
extern int NS_SetWindowIconID( int ID )
{
	WinData.IconID = ID ;

	// もし既にウインドウが作成されていたら、アイコンを変更する
	if( WinData.MainWindow != NULL )
	{
#ifdef _WIN64
		SetClassLongPtr( WinData.MainWindow, GCLP_HICON, ( LONG_PTR )LoadIcon( WinData.Instance , ( WinData.IconID == 0 ) ? IDI_APPLICATION : MAKEINTRESOURCE( WinData.IconID ) ) ) ;
#else
		SetClassLong( WinData.MainWindow, GCL_HICON, ( LONG_PTR )LoadIcon( WinData.Instance , ( WinData.IconID == 0 ) ? IDI_APPLICATION : MAKEINTRESOURCE( WinData.IconID ) ) ) ;
#endif
    }
	
	// 終了
	return 0 ;
}

// 使用するアイコンのハンドルをセットする
extern int NS_SetWindowIconHandle( HICON Icon )
{
	WinData.IconHandle = Icon ;

	// もし既にウインドウが作成されていたら、アイコンを変更する
	if( WinData.MainWindow != NULL )
	{
#ifdef _WIN64
		SetClassLongPtr( WinData.MainWindow, GCLP_HICON, ( LONG_PTR )Icon ) ;
#else
		SetClassLong( WinData.MainWindow, GCL_HICON, ( LONG_PTR )Icon ) ;
#endif
    }
	
	// 終了
	return 0 ;
}

// 最大化ボタンやALT+ENTERキーによる非同期なウインドウモードの変更の機能の設定を行う
extern int NS_SetUseASyncChangeWindowModeFunction( int Flag, void (*CallBackFunction)(void*), void *Data )
{
	// フラグのセット
	WinData.UseChangeWindowModeFlag = Flag ;

	// フラグに従ったウインドウのスタイルを設定する
	if( WinData.WindowModeFlag )
		SetWindowStyle() ;

	// コールバック関数の保存
	WinData.ChangeWindowModeCallBackFunction = CallBackFunction ;

	// コールバック関数に渡すデータの保存
	WinData.ChangeWindowModeCallBackFunctionData = Data ;

	// 終了
	return 0 ;
}

// ウインドウのスタイルを変更する
extern int NS_SetWindowStyleMode( int Mode )
{
	if( Mode < 0 || Mode >= WSTYLE_NUM ) return -1 ; 

	// スタイル値の変更
	WinData.WindowStyle = Mode ;

	// ウインドウの属性を変更
	if( WinData.WindowModeFlag ) SetWindowStyle() ;

	// ウインドウの再描画
	UpdateWindow( WinData.MainWindow ) ;

	// 終了
	return 0 ;
}

// ウインドウのサイズを変更できるかどうかのフラグをセットする
// NotFitScreen:ウインドウのクライアント領域に画面をフィットさせる(拡大させる)かどうか  TRUE:フィットさせる  FALSE:フィットさせない
extern int NS_SetWindowSizeChangeEnableFlag( int Flag, int FitScreen )
{
	int NotFitWindowSize ;

	NotFitWindowSize = FitScreen == TRUE ? FALSE : TRUE ;

	// フラグが同じ場合は何もしない
	if( WinData.WindowSizeChangeEnable == Flag &&
		WinData.ScreenNotFitWindowSize == NotFitWindowSize ) return 0 ;

	// フラグを保存
	WinData.WindowSizeChangeEnable = Flag ;
	WinData.ScreenNotFitWindowSize = NotFitWindowSize ;

	// ウインドウにフィットさせる場合は SetWindowSize ではウインドウの拡大率が変化するようにした
	/*
	// ウインドウにフィットさせる場合は SetWindowSize の設定は無効にする
	if( FitScreen == TRUE )
	{
		WinData.WindowSizeValid = FALSE ;
	}
	*/

	// ウインドウモードの場合のみウインドウスタイルを更新
	if( WinData.WindowModeFlag == TRUE )
		SetWindowStyle() ;

	// 終了
	return 0 ;
}

// 描画画面のサイズに対するウインドウサイズの比率を設定する
extern int NS_SetWindowSizeExtendRate( double ExRateX, double ExRateY )
{
	// ExRateY がマイナスの値の場合は ExRateX の値を ExRateY でも使用する
	if( ExRateY <= 0.0 ) ExRateY = ExRateX ;

	// 今までと同じ場合は何もしない
	if( WinData.WindowSizeExRateX == ExRateX &&
		WinData.WindowSizeExRateY == ExRateY ) return 0 ;

	WinData.WindowSizeExRateX = ExRateX ;
	WinData.WindowSizeExRateY = ExRateY ;
	if( WinData.WindowSizeExRateX <= 0.0 )
	{
		WinData.WindowSizeExRateX = 1.0 ;
	}
	if( WinData.WindowSizeExRateY <= 0.0 )
	{
		WinData.WindowSizeExRateY = 1.0 ;
	}

	// SetWindowSize の設定は無効にする
	WinData.WindowSizeValid = FALSE ;

	// ウインドウモードの場合は新しい比率を反映する
	if( WinData.WindowModeFlag == TRUE )
	{
		SetWindowStyle() ;
	}

	// 終了
	return 0 ;
}

// ウインドウモード時のウインドウのクライアント領域のサイズを設定する
extern int NS_SetWindowSize( int Width, int Height )
{
	// ウインドウが作成済みでウインドウにフィットさせる設定の場合は拡大率を変更する
	if( WinData.MainWindow != NULL && WinData.ScreenNotFitWindowSize == FALSE )
	{
		double ExtendRateX ;
		double ExtendRateY ;
		int ScreenSizeX ;
		int ScreenSizeY ;

		// 描画領域のサイズを取得
		NS_GetDrawScreenSize( &ScreenSizeX , &ScreenSizeY ) ;

		// 拡大率を指定する
		ExtendRateX = ( double )Width  / ScreenSizeX ;
		ExtendRateY = ( double )Height / ScreenSizeY ;
		NS_SetWindowSizeExtendRate( ExtendRateX, ExtendRateY ) ;
	}
	else
	{
		// それ以外の場合はウインドウサイズを変更する
		WinData.WindowWidth = Width ;
		WinData.WindowHeight = Height ;
		WinData.WindowSizeValid = TRUE ;

		// 反映させる
		WM_SIZEProcess() ;
	}

	// 終了
	return 0 ;
}

// ウインドウモードのウインドウの位置を設定する( 枠も含めた左上座標 )
extern int NS_SetWindowPosition( int x, int y )
{
	WinData.WindowX = x ;
	WinData.WindowY = y ;
	WinData.WindowPosValid = TRUE ;

	// 反映させる
	WM_SIZEProcess() ;

	// 終了
	return 0 ;
}

// _KBDLLHOOKSTRUCT 構造体の定義
typedef struct tag_KBDLLHOOKSTRUCT
{
    DWORD   vkCode;
    DWORD   scanCode;
    DWORD   flags;
    DWORD   time;
    DWORD   dwExtraInfo;
} _KBDLLHOOKSTRUCT, FAR *LP_KBDLLHOOKSTRUCT, *P_KBDLLHOOKSTRUCT;

// フックされた時のコールバック関数
LRESULT CALLBACK LowLevelKeyboardProc (INT nCode, WPARAM wParam, LPARAM lParam)
{
    // フック プロシージャから非ゼロの値を返すことにより、
    // メッセージがターゲット ウィンドウに渡されなくなります
    _KBDLLHOOKSTRUCT *pkbhs = (_KBDLLHOOKSTRUCT *) lParam;
    BOOL bControlKeyDown = 0;

	if( WinData.ActiveFlag == TRUE && WinData.SysCommandOffFlag == TRUE )
	{
		switch (nCode)
		{
			case HC_ACTION:
			{
				// Ctrl キーが押されたかどうかをチェック
				bControlKeyDown = GetAsyncKeyState (VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);

				// Ctrl + Esc を無効にします
				if (pkbhs->vkCode == VK_ESCAPE && bControlKeyDown)
					return 1;

				// Alt + Tab を無効にします
				if (pkbhs->vkCode == VK_TAB && pkbhs->flags & LLKHF_ALTDOWN)
					return 1;

				// Alt + Esc を無効にします
				if (pkbhs->vkCode == VK_ESCAPE && pkbhs->flags & LLKHF_ALTDOWN)
					return 1;

				// Alt + F4 を無効にします
				if (pkbhs->vkCode == VK_F4 && pkbhs->flags & LLKHF_ALTDOWN )
					return 1 ;

				// Alt up を無効にします
				if (pkbhs->flags & LLKHF_UP )
					return 1 ;

				break;
			}

			default:
				break;
		}
	}
    return CallNextHookEx( WinData.TaskHookHandle, nCode, wParam, lParam);
}

#ifdef DX_THREAD_SAFE

// ProcessMessage をひたすら呼びつづけるスレッド
DWORD WINAPI ProcessMessageThreadFunction( LPVOID )
{
	int Result ;
	DWORD ThreadID ;
	HANDLE EventHandle ;
	int WaitFlag = 0 ;

	// スレッドのＩＤを得ておく
	ThreadID = GetCurrentThreadId() ;

	// ひたすら ProcessMessage を呼びつづける
	for(;;)
	{
		// ソフトの終了フラグが立ったらループから外れる
		if( WinData.QuitMessageFlag == TRUE ) break ;

		// CheckConflictAndWaitDxFunction が呼ばれていたら、呼ばれ終わるまで待つ
		CRITICALSECTION_LOCK( &WinData.DxConflictCheckCriticalSection ) ;
//		CheckConflict( &WinData.DxConflictCheckFlag ) ;

		// ＤＸライブラリが使用されているかどうかで処理を分岐
		if( WinData.DxUseThreadFlag == FALSE || WinData.DxUseThreadID == ThreadID )
		{
//RUN:
			// 待っているスレッドが居て、それが自分のスレッドではない場合は待ち
			if( WinData.DxUseThreadFlag == FALSE && WinData.DxConflictWaitThreadNum != 0 && WinData.DxUseThreadID != ThreadID )
			{
				if( WinData.DxConflictWaitThreadID[0][0] != ThreadID )
				{
					goto WAIT ;
				}
				else
				{
					// もし待っていたのが自分だったらリストをスライドさせる
					if( WinData.DxConflictWaitThreadNum != 1 )
					{
						EventHandle = (HANDLE)WinData.DxConflictWaitThreadID[0][1] ;
						_MEMCPY( &WinData.DxConflictWaitThreadID[0][0], &WinData.DxConflictWaitThreadID[1][0], sizeof( DWORD_PTR ) * 2 * ( WinData.DxConflictWaitThreadNum - 1 ) ) ;
						WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum - 1][1] = (DWORD_PTR)EventHandle ;
						WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum - 1][0] = (DWORD_PTR)0 ;
					}

					// 待っている数を減らす
					WinData.DxConflictWaitThreadNum -- ;
				}
			}

			// 情報をセット
			WinData.DxUseThreadFlag = TRUE ;
			WinData.DxUseThreadID = ThreadID ;

			// 使用中カウンタをインクリメントする
//			if( WinData.DxConflictCheckCounter != 0 )
//			{
//				DXST_ERRORLOG_ADD( _T( "エラー:衝突発生 No.2\n" ) ) ;
//			}
			WinData.DxConflictCheckCounter ++ ;

			// ロックフラグを倒す
			CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//			WinData.DxConflictCheckFlag -- ;

			// ProcessMessage を呼ぶ
			Result = NS_ProcessMessage() ;

			// CheckConflictAndWaitDxFunction が呼ばれていたら、呼ばれ終わるまで待つ
			CRITICALSECTION_LOCK( &WinData.DxConflictCheckCriticalSection ) ;
//			CheckConflict( &WinData.DxConflictCheckFlag ) ;

			// カウンタが０になってたら使用中状態を解除
			if( WinData.DxConflictCheckCounter == 1 )
			{
				WinData.DxUseThreadFlag = FALSE ;
				WinData.DxUseThreadID = 0xffffffff ;

				// もし待っているスレッドがある場合は、スリープするフラグを立てる
				if( WinData.DxConflictWaitThreadNum > 0 )
					WaitFlag = 1 ;
			}
//			else
//			{
//				DXST_ERRORLOG_ADD( _T( "エラー:衝突発生 No.3 \n" ) ) ;
//			}

			// カウンタをデクリメント
			WinData.DxConflictCheckCounter -- ;

			// ロックフラグを倒す
			CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//			WinData.DxConflictCheckFlag -- ;

			// スリープするフラグが立っていたらスリープする
			if( WaitFlag == 1 )
			{
				// 次に実行すべきスレッドのイベントをシグナル状態にする
				SetEvent( (HANDLE)WinData.DxConflictWaitThreadID[0][1] ) ;
				WaitFlag = 0 ;
			}

			// PostMessage の戻り値が -1 だったらループを抜ける
			if( Result < 0 ) break ;

			// 暫く寝る
			Sleep( 17 ) ;
		}
		else
		{
WAIT:
			// もし待っているスレッドが限界を越えていたら単純な待ち処理を行う
			if( WinData.DxConflictWaitThreadNum == MAX_THREADWAIT_NUM )
			{
				// とりあえずこの関数を使用中、フラグを倒す
				CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//				WinData.DxConflictCheckFlag -- ;

//				DXST_ERRORLOG_ADD( _T( "エラー:衝突発生 No.5 \n" ) ) ;

				// 少し寝る
				Sleep( 1 ) ;

				// 最初に戻る
				continue ;
			}

			// 待ってるスレッドがいますよという情報を追加する
			WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum][0] = ThreadID ;
			EventHandle = (HANDLE)WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum][1] ;
			WinData.DxConflictWaitThreadNum ++ ;

			// この関数を使用中、フラグを倒す
			CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//			WinData.DxConflictCheckFlag -- ;

//			// 待ってるスレッドがいますよカウンターをインクリメントする
//			WinData.DxConflictWaitThreadCounter ++ ;
//			if( WinData.DxConflictWaitThreadCounter <= 0 )
//				WinData.DxConflictWaitThreadCounter = 1 ;

			// 使用中フラグが倒れるか、QuitMessageFlag が立つまで待つ
//			while( WinData.DxUseThreadFlag == TRUE && WinData.QuitMessageFlag != TRUE )

			while( WaitForSingleObject( EventHandle, 0 ) == WAIT_TIMEOUT && WinData.QuitMessageFlag != TRUE )
			{
				WaitForSingleObject( EventHandle, 1000 ) ;
//				Sleep( 0 ) ;
			}
//			WaitForSingleObject( EventHandle, INFINITE ) ;
			ResetEvent( EventHandle ) ;
			// 待ってるスレッドがあるよカウンタをデクリメントする
//			WinData.DxConflictWaitThreadCounter -- ;
//			if( WinData.DxConflictWaitThreadCounter < 0 )
//				WinData.DxConflictWaitThreadCounter = 0 ;

			// CheckConflictAndWaitDxFunction が呼ばれていたら、呼ばれ終わるまで待つ
//			CRITICALSECTION_LOCK( &WinData.DxConflictCheckCriticalSection ) ;
//			CheckConflict( &WinData.DxConflictCheckFlag ) ;

//			goto RUN ;
		}
	}

	// もしスレッドの待ち順の中に自分がいたら詰める
	{
		int i ;

		// CheckConflictAndWaitDxFunction が呼ばれていたら、呼ばれ終わるまで待つ
		CRITICALSECTION_LOCK( &WinData.DxConflictCheckCriticalSection ) ;
//		CheckConflict( &WinData.DxConflictCheckFlag ) ;

		for( i = 0 ; i < WinData.DxConflictWaitThreadNum ; )
		{
			if( WinData.DxConflictWaitThreadID[i][0] != ThreadID )
			{
				i ++ ;
				continue ;
			}

			EventHandle = (HANDLE)WinData.DxConflictWaitThreadID[i][1] ;
			if( WinData.DxConflictWaitThreadNum - 1 != i )
			{
				_MEMCPY( &WinData.DxConflictWaitThreadID[i][0], &WinData.DxConflictWaitThreadID[i+1][0], sizeof( DWORD_PTR ) * 2 * ( WinData.DxConflictWaitThreadNum - i - 1 ) ) ;
				WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum - 1][1] = (DWORD_PTR)EventHandle ;
				WinData.DxConflictWaitThreadID[WinData.DxConflictWaitThreadNum - 1][0] = 0 ;
			}
			WinData.DxConflictWaitThreadNum -- ;

			break ;
		}

		// フラグを倒す
		CriticalSection_Unlock( &WinData.DxConflictCheckCriticalSection ) ;
//		WinData.DxConflictCheckFlag -- ;

		// もし元々自分の番だったら次のスレッドのイベントをシグナル状態にする
		if( i == 0 && WinData.DxConflictWaitThreadNum > 0 )
		{
			SetEvent( (HANDLE)WinData.DxConflictWaitThreadID[0][1] ) ;
		}
	}

	// スレッドが終了したことを示すフラグを立てる
	WinData.ProcessMessageThreadExitFlag = TRUE ;

	// スレッド終了
	ExitThread( 0 ) ;

	// 終了
	return 0 ;
}

#endif

// フックされた時のコールバック関数
LRESULT CALLBACK MsgHook(int nCnode, WPARAM wParam, LPARAM lParam)
{
	MSG *pmsg;

	if( WinData.ActiveFlag == TRUE && WinData.SysCommandOffFlag == TRUE )
	{
		pmsg = (MSG *)lParam;
		if(pmsg->message == WM_USER + 260) pmsg->message = WM_NULL;
	}

	return 0;
}

// タスクスイッチを有効にするかどうかを設定する
extern int NS_SetSysCommandOffFlag( int Flag, const TCHAR *HookDllPath )
{
#ifndef DX_NON_STOPTASKSWITCH
	if( WinData.SysCommandOffFlag == Flag ) return 0 ;

	// 有効にする指定の場合はファイルパスを保存する
	if( Flag == TRUE )
	{
		// ファイルパスを保存
		if( HookDllPath == NULL )
		{
			int Length, FileSize ;
			HANDLE FileHandle ;
			void *DestBuffer ;
			DWORD WriteSize ;

			// パス名が特に指定されなかった場合は内蔵のＤＬＬを
			// テンポラリファイルに出力して使用する

			// キーボードフックＤＬＬファイルのサイズを取得する
			FileSize = DXA_Decode( DxKeyHookBinary, NULL ) ;

			// メモリの確保
			DestBuffer = DXALLOC( FileSize ) ;
			if( DestBuffer == NULL )
				return -1 ;

			// 解凍
			DXA_Decode( DxKeyHookBinary, DestBuffer ) ;

			// 取得临时文件的目录路径
			if( GetTempPath( MAX_PATH, WinData.HookDLLFilePath ) == 0 )
			{
				DXFREE( DestBuffer ) ;
				return -1 ;
			}

			// 字符串的最后加上目录分隔符
			Length = lstrlen( WinData.HookDLLFilePath ) ;
			if( WinData.HookDLLFilePath[Length-1] != _T( '\\' ) ) 
			{
				WinData.HookDLLFilePath[Length]   = _T( '\\' ) ;
				WinData.HookDLLFilePath[Length+1] = _T( '\0' ) ;
			}

			// 誰も使いそうに無いファイル名を追加する
			lstrcat( WinData.HookDLLFilePath, _T( "ddxx_MesHoooooook.dll" ) );

			// 打开临时文件
			DeleteFile( WinData.HookDLLFilePath ) ;
			FileHandle = CreateFile( WinData.HookDLLFilePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL ) ;
			if( FileHandle == NULL )
			{
				DXFREE( DestBuffer ) ;
				return -1 ;
			}

			// テンポラリファイルにデータを書き出す
			WriteFile( FileHandle, DestBuffer, FileSize, &WriteSize, NULL ) ;

			// 閉じる
			CloseHandle( FileHandle ) ;

			// メモリの解放
			DXFREE( DestBuffer ) ;

			// ユーザー指定のフックＤＬＬを使っていないフラグを立てる
			WinData.NotUseUserHookDllFlag = TRUE ;
		}
		else
		{
			lstrcpy( WinData.HookDLLFilePath, HookDllPath ) ; 

			// ユーザー指定のフックＤＬＬを使っていないフラグを倒す
			WinData.NotUseUserHookDllFlag = FALSE ;
		}
	}

	// ウインドウズのバージョンによって処理を分岐
	if( WinData.WindowsVersion < DX_WINDOWSVERSION_NT31 )
	{
		// Win95 カーネルの場合の処理
		UINT nPreviousState;
//		SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, Flag, &nPreviousState, 0 ) ;
//		SystemParametersInfo( SPI_SCREENSAVERRUNNING, TRUE, &nPreviousState, 0 ) ;
		SystemParametersInfo( SPI_SETSCREENSAVERRUNNING/*SPI_SCREENSAVERRUNNING*/, WinData.ActiveFlag && Flag, &nPreviousState, 0 ) ;
	}
	else
	{
		// WinNT カーネルの場合の処理
		if( Flag == TRUE )
		{
			// キーボードフックのセット
			if( WinData.TaskHookHandle == NULL )
			{
//				WinData.TaskHookHandle = SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc, WinData.Instance, 0 ) ;
			}

			// メッセージフックのセット
			if( WinData.GetMessageHookHandle == NULL )
			{
//				WinData.MessageHookThredID = GetWindowThreadProcessId( WinData.MainWindow, NULL ) ;
//				WinData.MessageHookThredID = GetWindowThreadProcessId( GetDesktopWindow(), NULL ) ;
				WinData.MessageHookDLL = LoadLibrary( WinData.HookDLLFilePath ) ;

				// DLL が無かったら進まない
				if( WinData.MessageHookDLL != NULL )
				{
					WinData.MessageHookCallBadk = ( MSGFUNC )GetProcAddress( WinData.MessageHookDLL, "SetMSGHookDll" ) ;
					if( WinData.MessageHookCallBadk != NULL )
					{
						WinData.MessageHookCallBadk( WinData.MainWindow, &WinData.KeyboardHookHandle ) ;
//						WinData.GetMessageHookHandle = SetWindowsHookEx( WH_GETMESSAGE, WinData.MessageHookCallBadk, WinData.MessageHookDLL, WinData.MessageHookThredID ) ;
						WinData.GetMessageHookHandle = SetWindowsHookEx( WH_GETMESSAGE, MsgHook, WinData.Instance, 0 ) ;
					}
				}
			}
		}
	}

	// 全バージョン共通
/*	if( Flag == TRUE )
	{
		// 勝手に他のウインドウがアクティブにならないようにする
		while( WinData.ActiveFlag != TRUE )
		{
			if( NS_ProcessMessage() == -1 ) break ;
		}
		LockSetForegroundWindow( LSFW_LOCK ) ;
	}
*/
	// フラグを保存
	WinData.SysCommandOffFlag = Flag ;

	// 終了
	return 0 ;
#else
	return -1;
#endif
}

// メッセージをフックするウインドウプロージャを登録する
extern int NS_SetHookWinProc( WNDPROC WinProc )
{
	// 登録する
	WinData.UserWindowProc = WinProc ;

	// 終了
	return 0 ;
}

// SetHookWinProc で設定したウインドウプロージャの戻り値を使用するかどうかを設定する、SetHookWinProc で設定したウインドウプロージャの中でのみ使用可能( UseFlag TRUE:戻り値を使用して、ＤＸライブラリのウインドウプロージャの処理は行わない  FALSE:戻り値は使用せず、ウインドウプロージャから出た後、ＤＸライブラリのウインドウプロージャの処理を行う )
extern int NS_SetUseHookWinProcReturnValue( int UseFlag )
{
	// フラグを保存する
	WinData.UseUserWindowProcReturnValue = UseFlag ;

	// 終了
	return 0 ;
}

// ２重起動を許すかどうかのフラグをセットする
extern int NS_SetDoubleStartValidFlag( int Flag )
{
	WinData.DoubleStartValidFlag = Flag ;

	// 終了
	return 0 ;
}

// メッセージ処理をＤＸライブラリに肩代わりしてもらうウインドウを追加する
extern int NS_AddMessageTakeOverWindow( HWND Window )
{
	if( WinData.MesTakeOverWindowNum == MAX_MESTAKEOVERWIN_NUM ) return -1 ;

	// 追加する
	WinData.MesTakeOverWindow[WinData.MesTakeOverWindowNum] = Window ;
	WinData.MesTakeOverWindowNum ++ ;

	// 終了
	return 0 ;
}

// メッセージ処理をＤＸライブラリに肩代わりしてもらうウインドウを減らす
extern	int	NS_SubMessageTakeOverWindow( HWND Window )
{
	int i ;

	// 探す
	for( i = 0 ; i < MAX_MESTAKEOVERWIN_NUM && WinData.MesTakeOverWindow[i] != Window ; i ++ ){}
	if( i == MAX_MESTAKEOVERWIN_NUM ) return -1 ;

	// 減らす
	WinData.MesTakeOverWindowNum -- ;

	// つぶす
	_MEMMOVE( &WinData.MesTakeOverWindow[i], &WinData.MesTakeOverWindow[i+1], sizeof( HWND ) * ( WinData.MesTakeOverWindowNum - i ) ) ;

	// 終了
	return 0 ;
}

// ウインドウの初期位置を設定する
extern int NS_SetWindowInitPosition( int x, int y )
{
	WinData.WindowX = x ;
	WinData.WindowY = y ;
	WinData.WindowPosValid = TRUE ;

	return 0 ;
}

// ＤＸライブラリでメッセージ処理を行うダイアログボックスを登録する
extern int NS_SetDialogBoxHandle( HWND WindowHandle )
{
	WinData.DialogBoxHandle = WindowHandle ;
	
	return 0 ;
}

// メインウインドウを表示するかどうかのフラグをセットする
extern int NS_SetWindowVisibleFlag( int Flag )
{
	if( WinData.NotWindowVisibleFlag == !Flag ) return 0 ;

	WinData.NotWindowVisibleFlag = !Flag ;

	if( WinData.MainWindow == NULL )
	{
//		NS_SetNotDrawFlag( WinData.NotWindowVisibleFlag ) ;
		
		return 0 ;
	}

	if( WinData.NotWindowVisibleFlag == FALSE )
	{
		ShowWindow( WinData.MainWindow , SW_SHOW ) ;
		UpdateWindow( WinData.MainWindow ) ;
	}
	else
	{
		ShowWindow( WinData.MainWindow , SW_HIDE ) ;
		UpdateWindow( WinData.MainWindow ) ;
	}

	// 終了
	return 0 ;
}

// メインウインドウを最小化するかどうかのフラグをセットする
extern int NS_SetWindowMinimizeFlag( int Flag )
{
	if( WinData.WindowMinimizeFlag == Flag ) return 0 ;

	WinData.WindowMinimizeFlag = Flag ;

	if( WinData.MainWindow == NULL )
	{
//		NS_SetNotDrawFlag( WinData.NotWindowVisibleFlag ) ;
		
		return 0 ;
	}

	if( WinData.WindowMinimizeFlag == FALSE )
	{
		ShowWindow( WinData.MainWindow , SW_RESTORE ) ;
		UpdateWindow( WinData.MainWindow ) ;
	}
	else
	{
		ShowWindow( WinData.MainWindow , SW_MINIMIZE ) ;
		UpdateWindow( WinData.MainWindow ) ;
	}

	// 終了
	return 0 ;
}

// メインウインドウの×ボタンを押した時にライブラリが自動的にウインドウを閉じるかどうかのフラグをセットする
extern int NS_SetWindowUserCloseEnableFlag( int Flag )
{
	WinData.NonUserCloseEnableFlag = !Flag ;
	
	// 終了
	return 0 ;
}

// ＤＸライブラリ終了時に PostQuitMessage を呼ぶかどうかのフラグをセットする
extern int NS_SetDxLibEndPostQuitMessageFlag( int Flag )
{
	WinData.NonDxLibEndPostQuitMessageFlag = !Flag ;

	// 終了
	return 0 ;
}

// 表示に使用するウインドウのハンドルを取得する
extern HWND GetDisplayWindowHandle( void )
{
	if( WinData.UserWindowFlag )
	{
		return WinData.UserChildWindow ? WinData.UserChildWindow : WinData.MainWindow ;
	}

	return WinData.MainWindow ;
}

// ユーザーウインドウの情報を更新
static void UpdateUserWindowInfo( void )
{
	// 画面モードはウインドウモード
	NS_ChangeWindowMode( TRUE ) ;

	// 画面サイズはクライアント領域のサイズ
	{
		RECT ClientRect ;

		GetClientRect( GetDisplayWindowHandle(), &ClientRect ) ;
		NS_SetGraphMode( ClientRect.right - ClientRect.left,
							ClientRect.bottom - ClientRect.top, 32 ) ;
	}
}

// ＤＸライブラリで利用するウインドウのハンドルをセットする
extern int NS_SetUserWindow( HWND WindowHandle )
{
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	WinData.MainWindow = WindowHandle ;
	WinData.UserWindowFlag = WindowHandle == NULL ? FALSE : TRUE ;

	// 情報を更新
	UpdateUserWindowInfo() ;

	// 終了
	return 0 ;
}

// ＤＸライブラリで使用する表示用の子ウインドウのハンドルをセットする(DxLib_Init を実行する以前でのみ有効)
extern int NS_SetUserChildWindow( HWND WindowHandle )
{
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	WinData.UserChildWindow = WindowHandle ;

	// 情報を更新
	UpdateUserWindowInfo() ;

	// 終了
	return 0 ;
}

// SetUseWindow で設定したウインドウのメッセージループ処理をＤＸライブラリで行うかどうか、フラグをセットする
extern int NS_SetUserWindowMessageProcessDXLibFlag( int Flag )
{
	WinData.NotUserWindowMessageProcessDXLibFlag = !Flag ;

	// 終了
	return 0 ;
}

// 検索するＤＸアーカイブファイルの拡張子を変更する
extern int NS_SetDXArchiveExtension( const TCHAR *Extension )
{
#ifndef DX_NON_DXA
	return DXA_DIR_SetArchiveExtension( Extension ) ;
#else
	return -1;
#endif
}

// ＤＸアーカイブファイルと通常のフォルダのどちらも存在した場合、どちらを優先させるかを設定する( 1:フォルダを優先 0:ＤＸアーカイブファイルを優先(デフォルト) )
extern int NS_SetDXArchivePriority( int Priority )
{
#ifndef DX_NON_DXA
	return DXA_DIR_SetDXArchivePriority( Priority ) ;
#else
	return -1;
#endif
}

// ＤＸアーカイブファイルの鍵文字列を設定する
extern int NS_SetDXArchiveKeyString( const TCHAR *KeyString )
{
#ifndef DX_NON_DXA
#ifdef UNICODE
	char TempBuffer[ 1024 ] ;

	WCharToMBChar( _GET_CODEPAGE(), ( DXWCHAR * )KeyString, TempBuffer, 1024 ) ; 
	return DXA_DIR_SetKeyString( TempBuffer ) ;
#else
	return DXA_DIR_SetKeyString( KeyString ) ;
#endif
#else
	return -1;
#endif
}

// FPUの精度を落とさない設定を使用するかどうかを設定する、DxLib_Init を呼び出す前のみ有効( TRUE:使用する(精度が落ちない)  FALSE:使用しない(精度を落とす(デフォルト) )
extern int NS_SetUseFPUPreserveFlag( int Flag )
{
	// 初期化前のみ有効
	if( DxSysData.DxLib_InitializeFlag ) return -1 ;

	// フラグを保存
	WinData.UseFPUPreserve = Flag ;

	// 終了
	return 0 ;
}

// マウスポインタがウインドウのクライアントエリアの外にいけるかどうかを設定する( TRUE:いける( デフォルト設定 )  FALSE:いけない )
extern int NS_SetValidMousePointerWindowOutClientAreaMoveFlag( int Flag )
{
	Flag = Flag ? FALSE : TRUE ;

	// 今までとフラグが同じ場合は何もしない
	if( WinData.NotMoveMousePointerOutClientAreaFlag == Flag )
		return 0 ;

	// フラグを保存
	WinData.NotMoveMousePointerOutClientAreaFlag = Flag ;

	if( DxSysData.DxLib_InitializeFlag == TRUE )
	{
		if( WinData.NotMoveMousePointerOutClientAreaFlag && WinData.ActiveFlag && WinData.WindowModeFlag )
		{
			WinData.SetClipCursorFlag = FALSE ;
		}
		else
		{
			ClipCursor( NULL ) ;
		}
	}

	// 終了
	return 0 ;
}

// バックバッファの透過色の部分を透過させるかどうかを設定する( TRUE:透過させる  FALSE:透過させない )
extern int NS_SetUseBackBufferTransColorFlag( int Flag )
{
	// 今までとフラグが同じ場合は何もしない
	if( WinData.BackBufferTransColorFlag == Flag )
		return 0 ;

	// フラグを保存
	WinData.BackBufferTransColorFlag = Flag ;

	// ウインドウモードを変更する
	NS_SetWindowStyleMode( WinData.WindowStyle ) ;

	// 終了
	return 0 ;
}

// UpdateLayerdWindowForBaseImage や UpdateLayerdWindowForSoftImage を使用するかどうかを設定する( TRUE:使用する  FALSE:使用しない )
extern int NS_SetUseUpdateLayerdWindowFlag( int Flag )
{
	// 今までとフラグが同じ場合は何もしない
	if( WinData.UseUpdateLayerdWindowFlag == Flag )
		return 0 ;

	// フラグを保存
	WinData.UseUpdateLayerdWindowFlag = Flag ;

	// ウインドウモードを変更する
	NS_SetWindowStyleMode( WinData.WindowStyle ) ;

	// 終了
	return 0 ;
}

// リソースを読み込む際に使用するモジュールを設定する( NULL を指定すると初期状態に戻ります、デフォルトでは NULL )
extern int NS_SetResourceModule( HMODULE ResourceModule )
{
	WinData.LoadResourModule = ResourceModule ;

	// 終了
	return 0 ;
}













// クリップボード関係

// クリップボードに格納されているテキストデータを読み出す、-1 の場合はクリップボードにテキストデータは無いということ( DestBuffer に NULL を渡すと格納に必要なデータサイズが返ってくる )
extern int NS_GetClipboardText( TCHAR *DestBuffer )
{
	HGLOBAL Mem ;
	void *SrcBuffer ;

	// クリップボードをオープン
	if( OpenClipboard( WinData.MainWindow ) == 0 )
		return -1 ;

	// クリップボードに格納されているデータがテキストデータかどうかを取得する
	if( IsClipboardFormatAvailable( CLIPBOARD_TEXT ) == 0 )
	{
		CloseClipboard() ;
		return -1 ;
	}

	// クリップボードに格納されているテキストデータのメモリハンドルを取得する
	Mem = GetClipboardData( CLIPBOARD_TEXT ) ;

	// 出力バッファが NULL の場合はテキストデータのサイズを返す
	if( DestBuffer == NULL )
	{
		SIZE_T Size ;

		Size = GlobalSize( Mem ) + 1 ;
		CloseClipboard() ;
		return ( int )Size ;
	}

	// 出力バッファにテキストデータをコピーする
	SrcBuffer = GlobalLock( Mem ) ;
	lstrcpy( DestBuffer, ( TCHAR * )SrcBuffer ) ;
	GlobalUnlock( Mem ) ;
	CloseClipboard() ;

	// 終了
	return 0 ;
}

// クリップボードにテキストデータを格納する
extern int NS_SetClipboardText( const TCHAR *Text )
{
	HGLOBAL Mem ;
	void *Buffer ;
	int Len ;

	// 文字列の長さを取得
	Len = lstrlen( Text ) ;

	// 文字列を格納するメモリ領域の確保
	Mem = GlobalAlloc( GMEM_FIXED, ( Len + 1 ) * sizeof( TCHAR ) ) ;

	// 文字列を確保したメモリ領域に格納
	Buffer = GlobalLock( Mem ) ;
	lstrcpy( ( TCHAR * )Buffer, Text ) ;
	GlobalUnlock( Mem ) ;

	// クリップボードをオープン
	if( OpenClipboard( WinData.MainWindow ) )
	{
		// クリップボードに文字列を格納する
		EmptyClipboard() ;
		SetClipboardData( CLIPBOARD_TEXT, Mem ) ;
		CloseClipboard() ;
	}
	else
	{
		// メモリの解放
		GlobalFree( Mem ) ;
	}

	// 終了
	return 0 ;
}










// ドラッグ＆ドロップされたファイル関係

// ファイルのドラッグ＆ドロップ機能を有効にするかどうかの設定を再設定する
extern int RefreshDragFileValidFlag()
{
	// 有効状態を変更
	DragAcceptFiles( WinData.MainWindow, WinData.DragFileValidFlag ) ;

	// 終了
	return 0 ;
}

// ファイルのドラッグ＆ドロップ機能を有効にするかどうかのフラグをセットする
extern int NS_SetDragFileValidFlag( int Flag )
{
	// 指定と既に同じ場合は何もせず終了
	if( WinData.DragFileValidFlag == Flag ) return 0 ;
	
	// 有効状態を変更
	DragAcceptFiles( WinData.MainWindow, Flag ) ;
	
	// フラグを保存
	WinData.DragFileValidFlag = Flag ;

	// 終了
	return 0 ;
}

// ドラッグ＆ドロップされたファイルの情報を初期化する
extern int NS_DragFileInfoClear( void )
{
	int i ;
	
	// 全てのファイル名用に確保したメモリ領域を解放する
	for( i = 0 ; i < WinData.DragFileNum ; i ++ )
	{
		DXFREE( WinData.DragFileName[i] ) ;
		WinData.DragFileName[i] = NULL ;
	}
	
	// ファイルの数を０にする
	WinData.DragFileNum = 0 ;
	
	// 終了
	return 0 ;
}

// ドラッグ＆ドロップされたファイル名を取得する( -1:取得できなかった  0:取得できた )
extern int NS_GetDragFilePath( TCHAR *FilePathBuffer )
{
	int Result = 0 ;

	// ファイル名が一つも無かったら -1 を返す
	if( WinData.DragFileNum == 0 ) return -1 ;

	// NULL を渡されたら文字列格納に必要なサイズを返す
	if( FilePathBuffer == NULL ) 
	{
		Result = ( lstrlen( WinData.DragFileName[ WinData.DragFileNum - 1 ] ) + 1 ) * sizeof( TCHAR ) ;
	}
	
	// 文字列をコピーする
	lstrcpy( FilePathBuffer, WinData.DragFileName[ WinData.DragFileNum - 1 ] ) ;

	// 渡し終わった文字列は解放する
	DXFREE( WinData.DragFileName[ WinData.DragFileNum - 1 ] ) ;
	WinData.DragFileName[ WinData.DragFileNum - 1 ] = NULL ;

	// ドラッグ＆ドロップされたファイルの数を減らす
	WinData.DragFileNum -- ;
	
	// 終了
	return Result ;
}

// ドラッグ＆ドロップされたファイルの数を取得する
extern int NS_GetDragFileNum( void )
{
	return WinData.DragFileNum ;
}




















// ウインドウ描画領域設定系関数

// 任意のグラフィックからRGNハンドルを作成する
extern HRGN NS_CreateRgnFromGraph( int Width, int Height, const void *MaskData, int Pitch, int Byte )
{
	HRGN RgnTmp, Rgn ;
	int i, j, x, f = 0, AddPitch ;
	BYTE *Data ;
	int SizeX, SizeY ;

	// 大元のRGNハンドルを作成する
	NS_GetDrawScreenSize( &SizeX, &SizeY ) ;
	Rgn = CreateRectRgn( 0, 0, SizeX, SizeY ) ;
	RgnTmp = CreateRectRgn( 0, 0, SizeX, SizeY ) ;
	CombineRgn( Rgn, Rgn, RgnTmp, RGN_XOR ) ;
	DeleteObject( RgnTmp ) ;

	// 透過RGNの走査
	AddPitch = Pitch - Width * Byte ;
	Data = ( BYTE * )MaskData ;
	for( i = 0 ; i < Height ; i ++, Data += AddPitch )
	{
		x = -1 ;
		for( j = 0 ; j < Width ; j ++, Data += Byte )
		{
			switch( Byte )
			{
			case 1 : f = *Data == 0 ; break ;
			case 2 : f = *( ( WORD * )Data ) == 0 ; break ;
			case 3 : f = *( ( WORD * )Data ) == 0 || *( ( BYTE * )( Data + 2 ) ) == 0 ; break ;
			case 4 : f = *( ( DWORD * )Data ) == 0 ; break ;
			}

			if( !f && x == -1 )
			{
				x = j ;
			}
			else
			if( f && x != -1 )
			{
				RgnTmp = CreateRectRgn( x, i, j, i + 1 ) ;
				CombineRgn( Rgn, Rgn, RgnTmp, RGN_OR ) ; 
				DeleteObject( RgnTmp ) ;
				x = -1 ;
			}
		}

		if( x != -1 )
		{
			RgnTmp = CreateRectRgn( x, i, j, i + 1 ) ;
			CombineRgn( Rgn, Rgn, RgnTmp, RGN_OR ) ; 
			DeleteObject( RgnTmp ) ;
			x = -1 ;
		}
	}

	// 終了
	return Rgn ;
}

// 任意の基本イメージデータと透過色からRGNハンドルを作成する
extern HRGN NS_CreateRgnFromBaseImage( BASEIMAGE *BaseImage, int TransColorR, int TransColorG, int TransColorB )
{
	HRGN RgnTmp, Rgn ;
	int i, j, x, f, AddPitch, Byte ;
	BYTE *Data ;
	DWORD TransColor ;
	int Width, Height ;
	BASEIMAGE TempBaseImage, *UseBaseImage ;

	// 基本イメージデータのフォーマットがXRGB8以外の場合はXRGB8形式にする
	if( BaseImage->ColorData.AlphaMask != 0x00000000 ||
		BaseImage->ColorData.RedMask   != 0x00ff0000 ||
		BaseImage->ColorData.GreenMask != 0x0000ff00 ||
		BaseImage->ColorData.BlueMask  != 0x000000ff )
	{
		NS_CreateXRGB8ColorBaseImage( BaseImage->Width, BaseImage->Height, &TempBaseImage ) ;
		NS_BltBaseImage( 0, 0, BaseImage, &TempBaseImage ) ;
		UseBaseImage = &TempBaseImage ;
	}
	else
	{
		UseBaseImage = BaseImage ;
	}

	// 大元のRGNハンドルを作成する
	Width = UseBaseImage->Width ;
	Height = UseBaseImage->Height ;
	Rgn = CreateRectRgn( 0, 0, Width, Height ) ;
	RgnTmp = CreateRectRgn( 0, 0, Width, Height ) ;
	CombineRgn( Rgn, Rgn, RgnTmp, RGN_XOR ) ;
	DeleteObject( RgnTmp ) ;

	// 透過RGNの走査
	Byte = UseBaseImage->ColorData.PixelByte ;
	AddPitch = UseBaseImage->Pitch - UseBaseImage->Width * Byte ;
	Data = ( BYTE * )UseBaseImage->GraphData ;
	TransColor = NS_GetColor3( &UseBaseImage->ColorData, TransColorR, TransColorG, TransColorB, 255 ) & 0x00ffffff ;
	for( i = 0 ; i < Height ; i ++, Data += AddPitch )
	{
		x = -1 ;
		for( j = 0 ; j < Width ; j ++, Data += Byte )
		{
			f = ( *( ( DWORD * )Data ) & 0x00ffffff ) == TransColor ;

			if( !f && x == -1 )
			{
				x = j ;
			}
			else
			if( f && x != -1 )
			{
				RgnTmp = CreateRectRgn( x, i, j, i + 1 ) ;
				CombineRgn( Rgn, Rgn, RgnTmp, RGN_OR ) ; 
				DeleteObject( RgnTmp ) ;
				x = -1 ;
			}
		}

		if( x != -1 )
		{
			RgnTmp = CreateRectRgn( x, i, j, i + 1 ) ;
			CombineRgn( Rgn, Rgn, RgnTmp, RGN_OR ) ; 
			DeleteObject( RgnTmp ) ;
			x = -1 ;
		}
	}

	if( UseBaseImage == &TempBaseImage )
	{
		ReleaseBaseImage( &TempBaseImage ) ;
	}

	// 終了
	return Rgn ;
}


// 任意のグラフィックからＲＧＮをセットする
extern int NS_SetWindowRgnGraph( const TCHAR *FileName )
{
	HBITMAP bmp ;
	BITMAP bm ;

	if( FileName == NULL )
	{
		if( WinData.WindowRgn == NULL )
		{
			return 0 ;
		}
		else
		{
			DeleteObject( WinData.WindowRgn ) ;
			WinData.WindowRgn = NULL ; 
		}
	}
	else
	{
		// ファイルを読み込む
		bmp = NS_CreateDIBGraph( FileName, FALSE, NULL ) ;
		if( bmp == NULL )
		{
			return -1 ;
		}

		// グラフィックの情報を取得する
		GetObject( bmp, sizeof( BITMAP ), &bm ) ;

		// ピッチ補正
		bm.bmWidthBytes += bm.bmWidthBytes % 4 ? 4 - bm.bmWidthBytes % 4 : 0 ;

		// リージョン作成
		if( WinData.WindowRgn != NULL ) DeleteObject( WinData.WindowRgn ) ;
		WinData.WindowRgn = NS_CreateRgnFromGraph( bm.bmWidth, bm.bmHeight, bm.bmBits, bm.bmWidthBytes, bm.bmBitsPixel / 8 ) ;

		// リージョンを割り当てる
		if( WinData.MainWindow != NULL ) SetWindowRgn( WinData.MainWindow, WinData.WindowRgn, TRUE ) ;
	}

	// 終了
	return 0 ;
}

// 描画先の画面の透過色の部分を透過させるＲＧＮをセットする
extern int NS_UpdateTransColorWindowRgn( void )
{
	BASEIMAGE ScreenImage ;
	int Width, Height ;

	NS_GetDrawScreenSize( &Width, &Height ) ;
	NS_CreateXRGB8ColorBaseImage( Width, Height, &ScreenImage ) ;
	NS_GetDrawScreenBaseImage( 0, 0, Width, Height, &ScreenImage ) ;

	// リージョン作成
	if( WinData.WindowRgn != NULL ) DeleteObject( WinData.WindowRgn ) ;
	WinData.WindowRgn = NS_CreateRgnFromBaseImage( &ScreenImage, ( GBASE.TransColor >> 16 ) & 0xff, ( GBASE.TransColor >> 8 ) & 0xff, GBASE.TransColor & 0xff ) ;

	// リージョンを割り当てる
	if( WinData.MainWindow != NULL ) SetWindowRgn( WinData.MainWindow, WinData.WindowRgn, TRUE ) ;

	NS_ReleaseBaseImage( &ScreenImage ) ;

	// 終了
	return 0 ;
}















// ツールバー関係

// 指定のＩＤのボタンのインデックスを得る
static int SearchToolBarButton( int ID )
{
	int i ;
	WINTOOLBARITEMINFO *but ;

	// ツールバーのセットアップが完了していない場合は何もせずに終了
	if( WinData.ToolBarUseFlag == FALSE ) return -1 ;

	// 指定のＩＤのボタンを探す
	but = WinData.ToolBarItem ;
	for( i = 0 ; i < WinData.ToolBarItemNum ; i ++, but ++ )
	{
		if( but->Type == TOOLBUTTON_TYPE_SEP ) continue ;	// 隙間は除外
		if( but->ID == ID ) break ;							// ＩＤが一致した場合は抜ける
	}

	// 無かったらエラー
	if( i == WinData.ToolBarItemNum ) return -1;

	// あったらインデックスを返す
	return i ;
}

// ツールバーの高さを得る
extern int GetToolBarHeight( void )
{
	RECT rect ;

	if( WinData.ToolBarUseFlag == FALSE ) return 0 ;

	GetWindowRect( WinData.ToolBarHandle, &rect ) ;
	return rect.bottom - rect.top ;
}

// ツールバーの準備( NULL を指定するとツールバーを解除 )
extern int NS_SetupToolBar( const TCHAR *BitmapName, int DivNum, int ResourceID )
{
	HBITMAP NewBitmap = NULL ;
	BITMAP bm ;

	if( WinAPIData.Win32Func.WinMMDLL == NULL )
		return -1 ;

	// BitmapName が NULL で、且つツールバーを使っていない場合は何もせずに終了
	if( BitmapName == NULL && ResourceID < 0 && WinData.ToolBarUseFlag == FALSE ) return 0 ;

	// フルスクリーンで起動している場合も何もせずに終了
	if( WinData.WindowModeFlag == FALSE ) return 0 ;

	// とりあえず全てのボタンを削除
	NS_DeleteAllToolBarButton() ;

	// BitmapName が NULL で、ResouceID も -1 の場合はツールバーを削除する
	if( BitmapName == NULL && ResourceID < 0 )
	{
		// ツールバーウインドウに WM_CLOSE メッセージを送る
		SendMessage( WinData.ToolBarHandle, WM_CLOSE, 0, 0 ) ;
		WinData.ToolBarHandle = NULL ;

		// ビットマップファイルを削除する
		DeleteObject( WinData.ToolBarButtonImage ) ;
		WinData.ToolBarButtonImage = NULL ;

		// ツールバーを使用しているフラグを倒す
		WinData.ToolBarUseFlag = FALSE ;
	}
	else
	{
		// ビットマップファイルを読み込む
		if( BitmapName != NULL )
		{
			NewBitmap = NS_CreateDIBGraph( BitmapName, 0, NULL ) ;
		}

		if( NewBitmap == NULL && ResourceID != -1 )
		{
			HRSRC RSrc ;
			BYTE *DataP ;
			BITMAPINFO *BmpInfo ;
			HGLOBAL Global ;

			// リソースを取得
			RSrc = FindResource( WinData.LoadResourModule == NULL ? GetModuleHandle( NULL ) : WinData.LoadResourModule, MAKEINTRESOURCE( ResourceID ), RT_BITMAP ) ;
			if( RSrc )
			{
				// リソースが格納されているメモリ領域を取得
				Global = LoadResource( WinData.LoadResourModule == NULL ? GetModuleHandle( NULL ) : WinData.LoadResourModule, RSrc ) ;
				if( Global )
				{
					DataP = ( BYTE * )LockResource( Global ) ;
					if( DataP )
					{
						// イメージデータの位置を取得
						BmpInfo = ( BITMAPINFO * )DataP ;
						DataP += sizeof( BITMAPINFOHEADER ) ;

						// カラービット数が８以下の時はパレットがある
						if( BmpInfo->bmiHeader.biBitCount <= 8 )
						{
							DataP += ( size_t )( 1 << BmpInfo->bmiHeader.biBitCount ) * sizeof( RGBQUAD ) ;
						}
						else
						// カラービット数が３２か１６でカラーマスクがある
						if( BmpInfo->bmiHeader.biBitCount == 16 || BmpInfo->bmiHeader.biBitCount == 32 || BmpInfo->bmiHeader.biCompression == BI_BITFIELDS )
						{
							DataP += sizeof( RGBQUAD ) * 3 ;
						}

						// HBITMAP の作成
						NewBitmap = NS_CreateDIBGraphToMem( BmpInfo, DataP, FALSE, NULL ) ;
					}

					// リソース解放
					UnlockResource( Global ) ;
				}
			}
//			NewBitmap = LoadBitmap( WinData.Instance, MAKEINTRESOURCE( ResourceID ) ) ;
		}

		if( NewBitmap == NULL )
		{
			DXST_ERRORLOG_ADD( _T( "ツールバーのボタン用のビットマップファイルの読み込みに失敗しました\n" ) ) ;
			return -1 ;
		}

		// ビットマップのサイズを得る
		GetObject( NewBitmap, sizeof( bm ), &bm ) ;

		// ツールバーを既に使っているかどうかで処理を分岐
		if( WinData.ToolBarUseFlag == FALSE )
		{
			TBADDBITMAP AddBitmap ;

			// 今まで使っていなかった場合はツールバーウインドウを作成する
			WinAPIData.Win32Func.InitCommonControlsFunc();
			WinData.ToolBarHandle = CreateWindowEx( 
				0,
				TOOLBARCLASSNAME,
				NULL,
				WS_CHILD | WS_VISIBLE,
				0, 0,
				0, 0,
				WinData.MainWindow,
				NULL,
				WinData.Instance,
				NULL ) ;
			if( WinData.ToolBarHandle == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "ツールバーウインドウの作成に失敗しました\n" ) ) ;
				return -1 ;
			}

			// TBBUTTON 構造体のサイズを送っておく
			SendMessage( WinData.ToolBarHandle, TB_BUTTONSTRUCTSIZE,
						 (WPARAM)sizeof( TBBUTTON ), 0 ) ;

			// ツールバーを使用している状態にする
			WinData.ToolBarUseFlag = TRUE ;

			// ビットマップファイルを設定する
			AddBitmap.hInst = NULL ;
			AddBitmap.nID   = (UINT_PTR)NewBitmap ;
			SendMessage( WinData.ToolBarHandle, TB_ADDBITMAP, DivNum, (LPARAM)&AddBitmap ) ;
			WinData.ToolBarButtonImage = NewBitmap ;

			// 情報を初期化する
			WinData.ToolBarItemNum = 0 ;
		}
		else
		{
			// 既に作られている場合はビットマップファイルを交換する
			TBREPLACEBITMAP RepBitmap ;

			RepBitmap.hInstOld = NULL ;
			RepBitmap.nIDOld   = (UINT_PTR)WinData.ToolBarButtonImage ;
			RepBitmap.hInstNew = NULL ;
			RepBitmap.nIDNew   = (UINT_PTR)NewBitmap ;
			RepBitmap.nButtons = DivNum ;
			SendMessage( WinData.ToolBarHandle, TB_REPLACEBITMAP, 0, (LPARAM)&RepBitmap ) ;

			// 今までのビットマップを破棄する
			DeleteObject( WinData.ToolBarButtonImage ) ;
			WinData.ToolBarButtonImage = NewBitmap ;
		}

		// ビットマップのサイズを設定する
		SendMessage( WinData.ToolBarHandle, TB_SETBITMAPSIZE, 0, ( bm.bmWidth / DivNum ) | ( bm.bmHeight << 16 ) ) ;

		// スタイルを変更する
		NS_SetWindowStyleMode( 6 ) ;
	}

	WM_SIZEProcess() ;

	// 終了
	return 0 ;
}

// ツールバーにボタンを追加
// int Type   : TOOLBUTTON_TYPE_NORMAL 等
// int State  : TOOLBUTTON_STATE_ENABLE 等
extern int NS_AddToolBarButton( int Type, int State, int ImageIndex, int ID )
{
	WINTOOLBARITEMINFO *but ;
	TBBUTTON tbbut ;
	static const BYTE StateTable[2][TOOLBUTTON_STATE_NUM] =
	{
		{ TBSTATE_ENABLED,                   TBSTATE_ENABLED, TBSTATE_INDETERMINATE, TBSTATE_PRESSED },
		{ TBSTATE_ENABLED, TBSTATE_CHECKED | TBSTATE_ENABLED, TBSTATE_INDETERMINATE, TBSTATE_CHECKED },
	};

	static const BYTE TypeTable[] =
	{ TBSTYLE_BUTTON, TBSTYLE_CHECK, TBSTYLE_CHECKGROUP, TBSTYLE_SEP } ;

	// ツールバーのセットアップが完了していない場合は何もせずに終了
	if( WinData.ToolBarUseFlag == FALSE ) return -1 ;

	// ツールバーのボタンの数が最大数に達していたら何もせずに終了
	if( WinData.ToolBarItemNum == MAX_TOOLBARITEM_NUM )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "ツールバーのボタンの数が最大数の %d に達している為ボタンを追加できませんでした\n" ), MAX_TOOLBARITEM_NUM )) ;
		return -1 ;
	}

	// パラメータの値が異常な場合はエラー
	if( Type >= TOOLBUTTON_TYPE_NUM )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "ツールバーのボタン追加関数において State の値が不正な値 %d となっています\n" ), State )) ;
		return -1 ;
	}
	if( State >= TOOLBUTTON_STATE_NUM )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "ツールバーのボタン追加関数において Type の値が不正な値 %d となっています\n" ), Type )) ;
		return -1 ;
	}

	// 新しいボタンを追加
	_MEMSET( &tbbut, 0, sizeof( tbbut ) ) ;
	tbbut.iBitmap = ImageIndex ;
	tbbut.idCommand = TOOLBAR_COMMANDID_BASE + ID ;
	tbbut.fsState = StateTable[ Type == TOOLBUTTON_TYPE_CHECK || Type == TOOLBUTTON_TYPE_GROUP ? 1 : 0 ][ State ] ;
	tbbut.fsStyle = TypeTable[ Type ] ;
	SendMessage( WinData.ToolBarHandle, TB_ADDBUTTONS, 1, (LPARAM)&tbbut ) ;

	// 新しいボタンの情報をセット
	but = &WinData.ToolBarItem[ WinData.ToolBarItemNum ] ;
	but->ID         = ID ;
	but->ImageIndex = ImageIndex ;
	but->Type       = Type ;
	but->State      = State ;
	but->Click      = FALSE ;

	// ボタンの数を増やす
	WinData.ToolBarItemNum ++ ;

	// 終了
	return 0 ;
}

// ツールバーに隙間を追加
extern int NS_AddToolBarSep( void )
{
	WINTOOLBARITEMINFO *but ;
	TBBUTTON tbbut ;

	// ツールバーのセットアップが完了していない場合は何もせずに終了
	if( WinData.ToolBarUseFlag == FALSE ) return -1 ;

	// ツールバーのボタンの数が最大数に達していたら何もせずに終了
	if( WinData.ToolBarItemNum == MAX_TOOLBARITEM_NUM )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "ツールバーのアイテムの数が最大数の %d に達している為隙間を追加できませんでした\n" ), MAX_TOOLBARITEM_NUM )) ;
		return -1 ;
	}

	// 新しく隙間を追加
	_MEMSET( &tbbut, 0, sizeof( tbbut ) ) ;
	tbbut.iBitmap   = 0 ;
	tbbut.idCommand = 0 ;
	tbbut.fsState   = TBSTATE_ENABLED ;
	tbbut.fsStyle   = TBSTYLE_SEP ;
	SendMessage( WinData.ToolBarHandle, TB_ADDBUTTONS, 1, (LPARAM)&tbbut ) ;

	// 新しいボタンの情報をセット
	but = &WinData.ToolBarItem[ WinData.ToolBarItemNum ] ;
	but->ID         = 0 ;
	but->ImageIndex = 0 ;
	but->Type       = TOOLBUTTON_TYPE_SEP ;
	but->State      = TOOLBUTTON_STATE_ENABLE ;
	but->Click      = FALSE ;

	// ボタンの数を増やす
	WinData.ToolBarItemNum ++ ;

	// 終了
	return 0 ;
}

// ツールバーのボタンの状態を取得
extern int NS_GetToolBarButtonState( int ID )
{
	int i ;
	WINTOOLBARITEMINFO *but ;
	int State = 0 ;
	LRESULT Result ;

	// ツールバーのセットアップが完了していない場合は何もせずに終了
	if( WinData.ToolBarUseFlag == FALSE ) return -1 ;

	// 指定のＩＤのボタンを探す
	i = SearchToolBarButton( ID ) ;
	if( i == -1 )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "指定のＩＤ %d を持ったツールバーのボタンがありませんでした\n" ), ID )) ;
		return -1;
	}
	but = &WinData.ToolBarItem[i] ;

	// ボタンのタイプによって処理を分岐
	switch( but->Type )
	{
	case TOOLBUTTON_TYPE_NORMAL :	// 普通のボタンの場合
		if( but->State == TOOLBUTTON_STATE_ENABLE )
		{
			// 一度でも押されたら TRUE 状態になる変数を返す
			State = but->Click ;
			but->Click = FALSE ;
		}
		else
		{
			return but->State == TOOLBUTTON_STATE_PRESSED ? TRUE : FALSE ;
		}
		break ;

	case TOOLBUTTON_TYPE_CHECK :	// 押すごとにＯＮ／ＯＦＦが切り替わるボタン
	case TOOLBUTTON_TYPE_GROUP :	// 別の TOOLBUTTON_TYPE_GROUP タイプのボタンが押されるとＯＦＦになるタイプのボタン(グループの区切りは隙間で)
		// ボタンの状態を取得する
		Result = SendMessage( WinData.ToolBarHandle, TB_GETSTATE, TOOLBAR_COMMANDID_BASE + but->ID, 0 ) ;
		if( Result & ( TBSTATE_CHECKED | TBSTATE_PRESSED ) ) State = TRUE  ;
		else                                                 State = FALSE ;
		break ;
	}

	// 状態を返す
	return State ;
}

// ツールバーのボタンの状態を設定
extern int NS_SetToolBarButtonState( int ID, int State )
{
	int i ;
	int SetState = 0 ;
	WINTOOLBARITEMINFO *but ;

	// ツールバーのセットアップが完了していない場合は何もせずに終了
	if( WinData.ToolBarUseFlag == FALSE ) return -1 ;

	// 指定のＩＤのボタンを探す
	i = SearchToolBarButton( ID ) ;
	if( i == -1 )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "指定のＩＤ %d を持ったツールバーのボタンがありませんでした\n" ), ID )) ;
		return -1;
	}
	but = &WinData.ToolBarItem[i] ;

	// ボタンの状態を保存
	but->State = State ;
	but->Click = FALSE ;

	// ボタンの状態を設定する
	switch( State )
	{
	case TOOLBUTTON_STATE_ENABLE  :
		SetState = TBSTATE_ENABLED ;
		break ;

	case TOOLBUTTON_STATE_PRESSED :
		if( but->Type == TOOLBUTTON_TYPE_NORMAL ) SetState = TBSTATE_PRESSED | TBSTATE_ENABLED ;
		else                                      SetState = TBSTATE_CHECKED | TBSTATE_ENABLED ;
		break ;

	case TOOLBUTTON_STATE_DISABLE :
		SetState = TBSTATE_INDETERMINATE ;
		break ;

	case TOOLBUTTON_STATE_PRESSED_DISABLE :
		SetState = TBSTATE_PRESSED | TBSTATE_INDETERMINATE ;
		break ;
	}
	SendMessage( WinData.ToolBarHandle, TB_SETSTATE, TOOLBAR_COMMANDID_BASE + but->ID, SetState ) ;

	// 終了
	return 0 ;
}

// ツールバーのボタンを全て削除
extern int NS_DeleteAllToolBarButton( void )
{
	int i ;

	// ツールバーのセットアップが完了していない場合は何もせずに終了
	if( WinData.ToolBarUseFlag == FALSE ) return -1 ;

	// ボタンの数だけ削除メッセージを送る
	for( i = 0 ; i < WinData.ToolBarItemNum ; i ++ )
		SendMessage( WinData.ToolBarHandle, TB_DELETEBUTTON, 0, 0 ) ;

	// ボタンの数を０にする
	WinData.ToolBarItemNum = 0 ;

	// 終了
	return 0 ;
}

















// メニュー関係

// メニューを有効にするかどうかを設定する
extern int NS_SetUseMenuFlag( int Flag )
{
	// もし今までと同じ場合は何もせず終了
	if( Flag == WinData.MenuUseFlag ) return 0 ;

	// フラグによって処理を分岐
	if( Flag == FALSE )
	{
		// メニューを削除する
		if( WinData.Menu != NULL )
		{
			// メニューをウインドウから外す
			NS_SetDisplayMenuFlag( FALSE ) ;

			// メニューを削除
			DestroyMenu( WinData.Menu ) ;
			WinData.Menu = NULL ;
		}
		
		// メニューを無効にする
		WinData.MenuUseFlag = FALSE ;
	}
	else
	{
		// メニューが無い場合は空のメニューを作成する
		if( WinData.Menu == NULL )
		{
			WinData.Menu = CreateMenu() ;
			if( WinData.Menu == NULL ) return -1 ;
		}
		
		// メニューを有効にする
		WinData.MenuUseFlag = TRUE ;
		
		// 初期状態で表示
		WinData.MenuDisplayState = FALSE ;
		NS_SetDisplayMenuFlag( TRUE ) ;
	}

	// 選択されたメニュー項目の数を初期化
	WinData.SelectMenuItemNum = 0 ;

	// メニュー項目の数を初期化する
	WinData.MenuItemInfoNum = 0 ;

	// 終了
	return 0 ;
}

// キーボードアクセラレーターを使用するかどうかを設定する
extern int NS_SetUseKeyAccelFlag( int Flag )
{
	if( WinData.AltF4_EndFlag == TRUE ) Flag = FALSE ;

	WinData.UseAccelFlag = Flag ;
	if( Flag == FALSE ) NS_ClearKeyAccel() ;

	// 終了
	return 0 ;
}

// ショートカットキーを追加する
extern int NS_AddKeyAccel( const TCHAR *ItemName, int ItemID,
							int KeyCode, int CtrlFlag, int AltFlag, int ShiftFlag )
{
#ifdef DX_NON_INPUT

	return -1 ;

#else // DX_NON_INPUT

	HACCEL NewAccel ;
	ACCEL *Accel, *ac ;
	int AccelNum ;
	WINMENUITEMINFO *WinItemInfo ;

	if( WinData.MenuUseFlag == FALSE ) goto ERR ;

	// 指定の選択項目が見つからなかったら何もしない
	WinItemInfo = SearchMenuItemInfo( ItemName, ItemID ) ;
	if( WinItemInfo == NULL ) goto ERR ;

	if( WinData.UseAccelFlag == FALSE ) NS_SetUseKeyAccelFlag( TRUE ) ;
	if( WinData.UseAccelFlag == FALSE ) goto ERR ;

	// 既にアクセラレーターが設定されているかどうかで処理を分岐
	if( WinData.Accel != NULL )
	{
		// 既に設定されている場合は現在のアクセラレーターの情報を取得する
		AccelNum = CopyAcceleratorTable( WinData.Accel, NULL, 0 ) ;

		// データが一つ増えるので数を一つ増やす
		AccelNum ++ ;

		// データを格納するためのメモリ領域を確保
		Accel = (ACCEL *)DXALLOC( sizeof( ACCEL ) * AccelNum ) ;
		if( Accel == NULL ) goto ERR ;

		// 現在のデータを取得する
		CopyAcceleratorTable( WinData.Accel, Accel, AccelNum ) ;
	}
	else
	{
		// データがひとつだけ入るメモリ領域を確保
		AccelNum = 1 ;
		Accel = (ACCEL *)DXALLOC( sizeof( ACCEL ) * AccelNum ) ;
		if( Accel == NULL ) goto ERR ;
	}

	// アクセラレータの情報を作成
	ac = &Accel[AccelNum-1] ;
	ac->fVirt = FNOINVERT | FVIRTKEY ;
	if( CtrlFlag != FALSE ) ac->fVirt |= FCONTROL ;
	if( AltFlag != FALSE ) ac->fVirt |= FALT ;
	if( ShiftFlag != FALSE ) ac->fVirt |= FSHIFT ;
	ac->key = (WORD)NS_ConvertKeyCodeToVirtualKey( KeyCode ) ;
	ac->cmd = (WORD)WinItemInfo->ID ;

	// アクセラレータを作成
	NewAccel = CreateAcceleratorTable( Accel, AccelNum ) ;
	if( NewAccel == NULL ) goto ERR ;

	// メモリの解放
	_MEMSET( Accel, 0, sizeof( ACCEL ) * AccelNum ) ;
	DXFREE( Accel ) ;

	// 作成に成功したら現在のアクセラレータを削除する
	if( WinData.Accel != NULL )
		DestroyAcceleratorTable( WinData.Accel ) ;

	// 新しいアクセラレータをセットする
	WinData.Accel = NewAccel ;

	// 終了
	return 0 ;
	
ERR:
	return -1 ;
#endif // DX_NON_INPUT
}

// ショートカットキーを追加する
extern int NS_AddKeyAccel_Name( const TCHAR *ItemName, int KeyCode, int CtrlFlag, int AltFlag, int ShiftFlag )
{
	return NS_AddKeyAccel( ItemName, 0, KeyCode, CtrlFlag, AltFlag, ShiftFlag ) ;
}

// ショートカットキーを追加する
extern int NS_AddKeyAccel_ID( int ItemID, int KeyCode, int CtrlFlag, int AltFlag, int ShiftFlag )
{
	return NS_AddKeyAccel( NULL, ItemID, KeyCode, CtrlFlag, AltFlag, ShiftFlag ) ;
}

// ショートカットキーの情報を初期化する
extern int NS_ClearKeyAccel( void )
{
	// アクセラレータが有効になっていた場合は削除する
	if( WinData.Accel != NULL )
	{
		DestroyAcceleratorTable( WinData.Accel ) ;
		WinData.Accel = NULL ;
	}

	// 終了
	return 0 ;
}

// メニューに項目を追加する
extern int NS_AddMenuItem( int AddType, const TCHAR *ItemName, int ItemID,
						int SeparatorFlag, const TCHAR *NewItemName, int NewItemID )
{
	HMENU Menu = NULL ;
	int AddIndex = 0 ;
	MENUITEMINFO ItemInfo ;
	WINMENUITEMINFO *WItemInfo ;

	if( WinData.AltF4_EndFlag != 0 ) return -1 ;

	// 情報が一杯だったらエラー
	if( WinData.MenuItemInfoNum == MAX_MENUITEM_NUM ) return -1 ;

	// メニューが無効だったら有効にする
	if( WinData.MenuUseFlag == FALSE ) NS_SetUseMenuFlag( TRUE ) ;

	// 追加のタイプによって処理を分岐
	switch( AddType )
	{
	case MENUITEM_ADD_CHILD :	// 指定の項目の子として追加する場合
		{
			// 親の指定があるかどうかで処理を分岐
			if( ItemID == MENUITEM_IDTOP )
			{
				Menu = WinData.Menu ;
			}
			else
			{
				// 指定の選択項目のサブメニューを取得する(無かったら作成する)
				Menu = MenuItemSubMenuSetup( ItemName, ItemID ) ;

				// 指定の選択項目が無かったら一番上に追加する
				if( Menu == NULL )
				{
					Menu = WinData.Menu ;
				}
			}

			// 追加先をサブメニュー内の末尾にする
			AddIndex = GetMenuItemCount( Menu ) ;
		}
		break ;
		
	case MENUITEM_ADD_INSERT :	// 指定の項目と指定の項目より一つ上の項目の間に追加する場合
		{
			WINMENUITEMINFO *WinItemInfo ;

			// 指定の項目の情報を取得する
			WinItemInfo = SearchMenuItemInfo( ItemName, ItemID ) ;
			
			// 無かったら選択項目の一番上に追加する
			if( WinItemInfo == NULL )
			{
				Menu = WinData.Menu ;
				AddIndex = GetMenuItemCount( Menu ) ;
			}
			else
			{
				// 在ったらその項目とその項目より一つ上の項目の間に追加する
				Menu = WinItemInfo->Menu ;
				AddIndex = WinItemInfo->Index ;
			}
		}
		break ;
	}

	// 区切り線かどうかで処理を分岐
	if( SeparatorFlag == TRUE )
	{
		// 区切り線の場合

		// 区切り線の情報をセットする
		_MEMSET( &ItemInfo, 0, sizeof( ItemInfo ) ) ;
		ItemInfo.cbSize = sizeof( MENUITEMINFO ) ;	// 構造体のサイズ
		ItemInfo.fMask = MIIM_TYPE ;				// 取得または設定するメンバ
		ItemInfo.fType = MFT_SEPARATOR ; 			// アイテムのタイプ

		// 区切り線項目の追加
		if( InsertMenuItem( Menu, AddIndex, TRUE, &ItemInfo ) == 0 )
		{
			return -1 ;
		}
	}
	else
	{
		// 選択項目の場合

		// 新しいアイテムのＩＤが－１だった場合使われていないＩＤを付ける
		if( NewItemID == -1 )
			NewItemID = GetNewMenuItemID() ;
		WItemInfo = &WinData.MenuItemInfo[ WinData.MenuItemInfoNum ] ;

		// 選択項目を追加する
		{
			// 新しい項目の情報をセットする
			_MEMSET( &ItemInfo, 0, sizeof( ItemInfo ) ) ;
			ItemInfo.cbSize = sizeof( MENUITEMINFO ) ;		// 構造体のサイズ
			ItemInfo.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID ;	// 取得または設定するメンバ
			ItemInfo.fType = MFT_STRING ;					// アイテムのタイプ
			ItemInfo.fState = MFS_ENABLED ;					// アイテムの状態
			ItemInfo.wID = NewItemID ;						// アイテムID
			lstrcpy( WItemInfo->Name, NewItemName ) ;
			ItemInfo.dwTypeData = WItemInfo->Name ;			// アイテムの内容セット
			ItemInfo.cch = lstrlen( WItemInfo->Name ) ;		// アイテムの文字列の長さ

			// 項目の追加
			if( InsertMenuItem( Menu, AddIndex, TRUE, &ItemInfo ) == 0 )
			{
				return -1 ;
			}
		}

		// メニュー項目の情報を追加する
		AddMenuItemInfo( Menu, AddIndex, NewItemID, NewItemName ) ;
	}

	// 一番最初のメニューバーへの項目追加の場合はウインドウサイズを調整する
	if( WinData.WindowModeFlag == TRUE && Menu == WinData.Menu && GetMenuItemCount( WinData.Menu ) == 1 )
		SetWindowStyle() ;

	// メニューを再描画する
	if( GetDisplayMenuState() == TRUE )
	{
		if( GRA2.ValidHardWare )
		{
			SetD3DDialogBoxMode( TRUE )  ;
		}

		DrawMenuBar( WinData.MainWindow ) ;
	}

	// 挿入の場合はリストを再構築する
	if( AddType == MENUITEM_ADD_INSERT )
	{
		WinData.MenuItemInfoNum = 0 ;
		ListupMenuItemInfo( WinData.Menu ) ;
	}

	// 終了
	return 0 ;
}

// メニューから項目を削除する
extern int NS_DeleteMenuItem( const TCHAR *ItemName, int ItemID )
{
	HMENU Menu ;
	int Index ;

	if( WinData.MenuUseFlag == FALSE ) return -1 ;

	// 指定の項目を探す
	if( SearchMenuItem( ItemName, ItemID, WinData.Menu, &Menu, &Index ) != 1 ) return -1 ;

	// 選択項目を削除する
	DeleteMenu( Menu, Index, MF_BYPOSITION ) ;

	// リストを再構築する
	WinData.MenuItemInfoNum = 0 ;
	ListupMenuItemInfo( WinData.Menu ) ;

	// メニューを再描画する
	if( GetDisplayMenuState() == TRUE )
	{
		if( GRA2.ValidHardWare )
		{
			SetD3DDialogBoxMode( TRUE )  ;
		}

		DrawMenuBar( WinData.MainWindow ) ;
	}

	// 終了
	return 0 ;
}

// メニューが選択されたかどうかを取得する( 0:選択されていない  1:選択された )
extern int NS_CheckMenuItemSelect( const TCHAR *ItemName, int ItemID )
{
	int i, InfoNum ;

	if( WinData.MenuUseFlag == FALSE ) return -1 ;

	// 名前が指定されていた場合はＩＤを取得する
	if( ItemName != NULL )
	{
		ItemID = NS_GetMenuItemID( ItemName ) ;
	}

	// 指定のアイテムが存在するか調べる
	InfoNum = WinData.SelectMenuItemNum ;
	for( i = 0 ; i < InfoNum ; i ++ )
		if( WinData.SelectMenuItem[i] == ItemID ) break ;

	// 無かったら０を返す
	if( i == InfoNum ) return 0 ;
	
	// 在ったら、情報から外す
	if( i != InfoNum - 1 )
		_MEMMOVE( &WinData.SelectMenuItem[i], &WinData.SelectMenuItem[i+1], sizeof( int ) * ( InfoNum - i - 1 ) ) ;
	
	// 情報の数を減らす
	WinData.SelectMenuItemNum -- ;

	// あったので１を返す
	return 1 ;
}

// メニューの全ての選択項目を削除する
extern int NS_DeleteMenuItemAll( void )
{
	HMENU NewMenu ;
	int DispFlag ;

	if( WinData.MenuUseFlag == FALSE ) return -1 ;
	
	// 新しいメニューを作成
	NewMenu = CreateMenu() ;
	if( NewMenu == NULL ) return -1 ;

	DispFlag = NS_GetDisplayMenuFlag() ;
	
	// 大本のメニューを削除して作り直す
	if( WinData.Menu != NULL )
	{
		// とりあえずウインドウから外す
		NS_SetDisplayMenuFlag( FALSE ) ;
		
		// フルスクリーンの場合
		if( WinData.WindowModeFlag == FALSE )
		{
			// メニューを外す
			if( WinData.MenuSetupFlag == TRUE )
			{
				SetMenu( WinData.MainWindow, NULL ) ;
				WinData.MenuSetupFlag = FALSE ;
			}
		}

		// メニューを削除
		DestroyMenu( WinData.Menu ) ;
	}

	// 新しいメニューをセット
	WinData.Menu = NewMenu ;

	// 表示状態を元に戻す
	NS_SetDisplayMenuFlag( DispFlag ) ;

	// フルスクリーンの場合
	if( WinData.WindowModeFlag == FALSE )
	{
		// メニューのセット
		if( WinData.MenuSetupFlag == FALSE && WinData.MenuDisplayState != FALSE )
		{
			SetMenu( WinData.MainWindow, WinData.Menu ) ;
			WinData.MenuSetupFlag = TRUE ;
		}
	}

	// 選択されたメニュー項目の数を初期化
	WinData.SelectMenuItemNum = 0 ;

	// リストも初期化
	WinData.MenuItemInfoNum = 0 ;

	// 終了
	return 0 ;
}

// メニューが選択されたかどうかの情報を初期化
extern int NS_ClearMenuItemSelect( void )
{
	WinData.SelectMenuItemNum = 0 ;

	// 終了
	return 0 ;
}

// メニューの項目を選択出来るかどうかを設定する
extern int NS_SetMenuItemEnable( const TCHAR *ItemName, int ItemID, int EnableFlag )
{
	WINMENUITEMINFO *WinItemInfo ;
	
	if( WinData.MenuUseFlag == FALSE ) return -1 ;

	// 選択項目の情報を取得
	WinItemInfo = SearchMenuItemInfo( ItemName, ItemID ) ;
	if( WinItemInfo == NULL ) return -1 ;

	// 選択出来るかどうかを設定する
	EnableMenuItem( WinItemInfo->Menu, WinItemInfo->Index, MF_BYPOSITION | ( EnableFlag == TRUE ? MF_ENABLED : MF_GRAYED ) ) ;

	// 表示を更新する
	if( GetDisplayMenuState() == TRUE )
	{
		if( GRA2.ValidHardWare )
		{
			SetD3DDialogBoxMode( TRUE )  ;
		}

		DrawMenuBar( WinData.MainWindow ) ;
	}

	// 終了
	return 0 ;
}

// メニューの項目にチェックマークやラジオボタンを表示するかどうかを設定する
extern int NS_SetMenuItemMark( const TCHAR *ItemName, int ItemID, int Mark )
{
	MENUITEMINFO ItemInfo ;
	WINMENUITEMINFO *WinItemInfo ;

	if( WinData.MenuUseFlag == FALSE ) return -1 ;
	
	// 選択項目の情報を取得
	WinItemInfo = SearchMenuItemInfo( ItemName, ItemID ) ;
	if( WinItemInfo == NULL ) return -1 ;

	// 現在の情報を得る
	_GetMenuItemInfo( WinItemInfo->Menu, WinItemInfo->Index, &ItemInfo ) ;
	
	// 情報を書き換える
	switch( Mark )
	{
	case MENUITEM_MARK_NONE :	// 何も付けない
		ItemInfo.fMask |= MIIM_TYPE ;
		ItemInfo.fState &= ~MFS_CHECKED ;
		ItemInfo.fType &= ~MFT_RADIOCHECK ;
		break ;
		
	case MENUITEM_MARK_CHECK :	// 普通のチェック
		ItemInfo.fMask |= MIIM_TYPE ;
		ItemInfo.fType &= ~MFT_RADIOCHECK ;
		ItemInfo.fState |= MFS_CHECKED ;
		ItemInfo.hbmpChecked = NULL ;
		break ;

	case MENUITEM_MARK_RADIO :	// ラジオボタン
		ItemInfo.fMask |= MIIM_TYPE ;
		ItemInfo.fType = MFT_RADIOCHECK ;
		ItemInfo.fState |= MFS_CHECKED ;
		ItemInfo.hbmpChecked = NULL ;
		break ;
	}
	SetMenuItemInfo( WinItemInfo->Menu, WinItemInfo->Index, TRUE, &ItemInfo ) ;
	
	// 表示を更新する
	if( GetDisplayMenuState() == TRUE )
	{
		if( GRA2.ValidHardWare )
		{
			SetD3DDialogBoxMode( TRUE )  ;
		}

		DrawMenuBar( WinData.MainWindow ) ;
	}

	// 終了
	return 0 ;
}

// メニューの項目がどれか選択されたかどうかを取得する( 戻り値  TRUE:どれか選択された  FALSE:選択されていない )
extern int NS_CheckMenuItemSelectAll( void )
{
	return WinData.SelectMenuItemNum != 0 ? TRUE : FALSE ;
}

// メニューに項目を追加する
extern int NS_AddMenuItem_Name( const TCHAR *ParentItemName, const TCHAR *NewItemName )
{
	if( ParentItemName == NULL )
	{
		return NS_AddMenuItem( MENUITEM_ADD_CHILD, NULL, MENUITEM_IDTOP,
								 FALSE, NewItemName, -1 ) ;
	}
	else
	{
		return NS_AddMenuItem( MENUITEM_ADD_CHILD, ParentItemName, -1,
								 FALSE, NewItemName, -1 ) ;
	}
}

// 指定の項目と、指定の項目の一つ上の項目との間に新しい項目を追加する
extern int NS_InsertMenuItem_Name( const TCHAR *ItemName, const TCHAR *NewItemName )
{
	if( ItemName == NULL )
	{
		return NS_AddMenuItem( MENUITEM_ADD_INSERT, NULL, MENUITEM_IDTOP,
								FALSE, NewItemName, -1 ) ;
	}
	else
	{
		return NS_AddMenuItem( MENUITEM_ADD_INSERT, ItemName, -1,
								FALSE, NewItemName, -1 ) ;
	}
}

// メニューのリストに区切り線を追加する
extern int NS_AddMenuLine_Name( const TCHAR *ParentItemName )
{
	if( ParentItemName == NULL )
	{
		return NS_AddMenuItem( MENUITEM_ADD_CHILD, NULL, MENUITEM_IDTOP,
								TRUE, NULL, -1 ) ;
	}
	else
	{
		return NS_AddMenuItem( MENUITEM_ADD_CHILD, ParentItemName, -1,
								TRUE, NULL, -1 ) ;
	}
}

// 指定の項目と、指定の項目の一つ上の項目との間に区切り線を追加する
extern int NS_InsertMenuLine_Name( const TCHAR *ItemName )
{
	if( ItemName == NULL )
	{
		return NS_AddMenuItem( MENUITEM_ADD_INSERT, NULL, MENUITEM_IDTOP,
							TRUE, NULL, -1 ) ;
	}
	else
	{
		return NS_AddMenuItem( MENUITEM_ADD_INSERT, ItemName, -1,
								TRUE, NULL, -1 ) ;
	}
}

// メニューから項目を削除する
extern int NS_DeleteMenuItem_Name( const TCHAR *ItemName )
{
	return NS_DeleteMenuItem( ItemName, -1 ) ;
}

// メニューが選択されたかどうかを取得する( 0:選択されていない  1:選択された )
extern int NS_CheckMenuItemSelect_Name( const TCHAR *ItemName )
{
	return NS_CheckMenuItemSelect( ItemName, -1 ) ;
}

// メニューの項目を選択出来るかどうかを設定する
extern int NS_SetMenuItemEnable_Name( const TCHAR *ItemName, int EnableFlag )
{
	return NS_SetMenuItemEnable( ItemName, -1, EnableFlag ) ;
}

// メニューの項目にチェックマークやラジオボタンを表示するかどうかを設定する
extern int NS_SetMenuItemMark_Name( const TCHAR *ItemName, int Mark )
{
	return NS_SetMenuItemMark( ItemName, -1, Mark ) ;
}


// メニューに項目を追加する
extern int NS_AddMenuItem_ID( int ParentItemID, const TCHAR *NewItemName, int NewItemID )
{
	return NS_AddMenuItem( MENUITEM_ADD_CHILD, NULL, ParentItemID,
							FALSE, NewItemName, NewItemID ) ;
}

// 指定の項目と、指定の項目の一つ上の項目との間に新しい項目を追加する
extern int NS_InsertMenuItem_ID( int ItemID, int NewItemID )
{
	return NS_AddMenuItem( MENUITEM_ADD_INSERT, NULL, ItemID,
							FALSE, NULL, NewItemID ) ;
}

// メニューのリストに区切り線を追加する
extern int NS_AddMenuLine_ID( int ParentItemID )
{
	return NS_AddMenuItem( MENUITEM_ADD_CHILD, NULL, ParentItemID,
							TRUE, NULL, -1 ) ;
}

// 指定の項目と、指定の項目の一つ上の項目との間に区切り線を追加する
extern int NS_InsertMenuLine_ID( int ItemID, int NewItemID )
{
	return NS_AddMenuItem( MENUITEM_ADD_INSERT, NULL, ItemID,
							TRUE, NULL, NewItemID ) ;
}

// メニューから項目を削除する
extern int NS_DeleteMenuItem_ID( int ItemID )
{
	return NS_DeleteMenuItem( NULL, ItemID ) ;
}

// メニューが選択されたかどうかを取得する( 0:選択されていない  1:選択された )
extern int NS_CheckMenuItemSelect_ID( int ItemID )
{
	return NS_CheckMenuItemSelect( NULL, ItemID ) ;
}

// メニューの項目を選択出来るかどうかを設定する
extern int NS_SetMenuItemEnable_ID( int ItemID, int EnableFlag )
{
	return NS_SetMenuItemEnable( NULL, ItemID, EnableFlag ) ;
}

// メニューの項目にチェックマークやラジオボタンを表示するかどうかを設定する
extern int NS_SetMenuItemMark_ID( int ItemID, int Mark )
{
	return NS_SetMenuItemMark( NULL, ItemID, Mark ) ;
}


// メニューの項目名からＩＤを取得する
extern int NS_GetMenuItemID( const TCHAR *ItemName )
{
	WINMENUITEMINFO *WinItemInfo ;
	int Result ;

	if( WinData.MenuUseFlag == FALSE ) return -1 ;
	
	// 選択項目の情報を取得
	WinItemInfo = SearchMenuItemInfo( ItemName, -1 ) ;
	if( WinItemInfo == NULL ) return -1 ;
	Result = (int)WinItemInfo->ID ;
	
	// ＩＤを返す
	return Result ;
}


// メニューのＩＤから項目名を取得する
extern int NS_GetMenuItemName( int ItemID, TCHAR *NameBuffer )
{
	int Result ;
	MENUITEMINFO ItemInfo ;
	WINMENUITEMINFO *WinItemInfo ;

	if( WinData.MenuUseFlag == FALSE ) return -1 ;
	
	// 親のＩＤを持つ選択項目の情報を取得		
	WinItemInfo = SearchMenuItemInfo( NULL, ItemID ) ;
	if( WinItemInfo == NULL ) return -1 ;

	// 現在の情報を得る
	Result = _GetMenuItemInfo( WinItemInfo->Menu, WinItemInfo->Index, &ItemInfo ) ;
	if( Result != 0 ) return -1 ;

	// 項目名をコピー
	lstrcpy( NameBuffer, ItemInfo.dwTypeData ) ;

	// 終了
	return 0 ;
}

// メニューをリソースから読み込む
extern int NS_LoadMenuResource( int MenuResourceID )
{
	HMENU Menu ;

	// 指定のリソースを読み込む
	Menu = LoadMenu( WinData.LoadResourModule == NULL ? GetModuleHandle( NULL ) : WinData.LoadResourModule, MAKEINTRESOURCE( MenuResourceID ) ) ;
	if( Menu == NULL ) return -1 ;
	
	// メニューを有効にする
	NS_SetUseMenuFlag( TRUE ) ;
	
	// 既に作成されているメニューを削除する
	if( WinData.Menu != NULL )
	{
		// メニューをウインドウから外す
		NS_SetDisplayMenuFlag( FALSE ) ;

		// メニューを削除
		DestroyMenu( WinData.Menu ) ;
		WinData.Menu = NULL ;
	}

	// 新しいメニューをセット
	WinData.Menu = Menu ;
	
	// メニューを表示状態にセット
	WinData.MenuDisplayState = FALSE ;
	NS_SetDisplayMenuFlag( TRUE ) ;

	// 選択項目の情報を更新する
	WinData.MenuItemInfoNum = 0 ;
	ListupMenuItemInfo( WinData.Menu ) ;

	// 終了
	return 0 ;
}

// メニューの項目が選択されたときに呼ばれるコールバック関数を設定する
extern int NS_SetMenuItemSelectCallBackFunction( void (*CallBackFunction)( const TCHAR *ItemName, int ItemID ) )
{
	WinData.MenuCallBackFunction = CallBackFunction ;

	// 終了
	return 0 ;
}

// メニュー構造の中から、選択項目のＩＤを元に位置情報を得る( -1:エラー  0:見つからなかった  1:見つかった )
static int SearchMenuItem( const TCHAR *ItemName, int ItemID, HMENU SearchMenu, HMENU *Menu, int *Index )
{
	int ItemNum, i ;
	int Result ;
	HMENU SubMenu ;
	MENUITEMINFO ItemInfo ;

	// メニューが有効ではなかったら何もしない
	if( WinData.MenuUseFlag == FALSE ) return -1 ;

	// メニュー内の項目数を得る	
	ItemNum = GetMenuItemCount( SearchMenu ) ;
	if( ItemNum == -1 ) return -1 ;

	// メニュー内に指定のＩＤの選択項目が無いか調べる
	for( i = 0 ; i < ItemNum ; i ++ )
	{
		// 項目の情報を得る
		Result = _GetMenuItemInfo( SearchMenu, i, &ItemInfo ) ;
		
		// 区切り線だったらスキップ
		if( Result == 1 ) continue ;
		
		// 目的の項目だったらここで終了
		if( ItemName )
		{
			if( lstrcmp( ItemInfo.dwTypeData, ItemName ) == 0 ) break ;
		}
		else
		{
			if( ItemInfo.wID == (UINT)ItemID ) break ;
		}

		// サブメニューを持っていたらそちらも検索する
		SubMenu = ItemInfo.hSubMenu ;
		if( SubMenu != NULL )
		{
			Result = SearchMenuItem( ItemName, ItemID, SubMenu, Menu, Index ) ;
			
			// エラーが発生するか、目的のＩＤが見つかったらここで終了
			if( Result == -1 || Result == 1 ) return Result ;
		}
	}
	
	// 在ったら情報を格納して関数を出る
	if( i != ItemNum )
	{
		*Menu = SearchMenu ;
		*Index = i ;
		
		// 成功終了は１
		return 1 ;
	}
	
	// 無かったら０を返す
	return 0 ;
}



// (古い関数)ウインドウにメニューを設定する
extern int NS_SetWindowMenu( int MenuID, int (*MenuProc)( WORD ID ) )
{
	int DispFlag ;

	DispFlag = NS_GetDisplayMenuFlag() ;

	// もし既にメニューがロードされていたらメニューを削除する
	if( WinData.Menu != NULL )
	{
		// メニューを除去
		NS_SetDisplayMenuFlag( FALSE ) ;

		// メニューを削除
		DestroyMenu( WinData.Menu ) ;
		WinData.Menu = NULL ;
	}

	// もしメニューＩＤが -1 だったらメニューを削除
	if( MenuID == -1 )
	{
		// メニューを無効にする
		WinData.MenuUseFlag = FALSE ;
	}
	else
	{
		// メニューをロード
		WinData.Menu = LoadMenu( GetModuleHandle( NULL ), MAKEINTRESOURCE( MenuID ) ) ;
		if( WinData.Menu == NULL ) return -1 ;

		// メニューを有効にする
		WinData.MenuUseFlag = TRUE ;

		// メニューを表示状態を更新する
		NS_SetDisplayMenuFlag( DispFlag ) ;

		// コールバック関数のポインタを保存
		WinData.MenuProc = MenuProc ;
	}
	
	// 選択項目の情報を更新する
	WinData.MenuItemInfoNum = 0 ;
	ListupMenuItemInfo( WinData.Menu ) ;

	// 終了
	return 0 ;
}

// ウインドウメニュー表示開始に使用するキーを設定する
/*extern int SetWindowMenuStartKey( int KeyID )
{
	// キーを設定する
	WinData.MenuStartKey = KeyID ;

	// 終了
	return 0 ;
}
*/

// メニューを表示するかどうかをセットする
extern int NS_SetDisplayMenuFlag( int Flag )
{
	int DispState ;

	// メニューが有効ではない場合は何もしない
	if( WinData.MenuUseFlag == FALSE ) return 0 ;

	// フラグを保存
	WinData.NotMenuDisplayFlag = !Flag ;
	
	// ウインドウが作成されていない場合は何もしない
	if( WinData.MainWindow == NULL ) return 0 ;

	// 表示状態が同じ場合は何もせず終了
	DispState = GetDisplayMenuState() ;
	if( DispState == WinData.MenuDisplayState ) return 0 ;

	// メニューを表示すべきかどうかによって処理を分岐
	if( DispState == TRUE )
	{
		// 表示する場合の処理

		// メニューのセット
		if( WinData.MenuSetupFlag == FALSE )
		{
			SetMenu( WinData.MainWindow, WinData.Menu ) ;
			WinData.MenuSetupFlag = TRUE ;
		}

		// ウインドウの属性を変更
		if( WinData.WindowModeFlag ) SetWindowStyle() ;
	}
	else
	{
		// 非表示にする場合の処理

		// メニューを外す
		if( WinData.MenuSetupFlag == TRUE )
		{
			SetMenu( WinData.MainWindow, NULL ) ;
			WinData.MenuSetupFlag = FALSE ;
		}

		// ウインドウの属性を変更
		if( WinData.WindowModeFlag ) SetWindowStyle() ;
	}

	// メニューの再描画
	UpdateWindow( WinData.MainWindow ) ;
//	DrawMenuBar( WinData.MainWindow ) ;

	// フラグの保存
	WinData.MenuDisplayState = DispState ;

	// 終了
	return 0 ;
}

// メニューを表示しているかどうかを取得する
extern int NS_GetDisplayMenuFlag( void )
{
	return WinData.NotMenuDisplayFlag == FALSE ;
}

// メニューが表示するべきかどうかを取得する( FALSE:表示すべきでない  TRUE:表示すべき )
static int GetDisplayMenuState( void )
{
	if( WinData.MainWindow == NULL ) return FALSE ;
	if( WinData.MenuUseFlag == FALSE ) return FALSE ;
	if( WinData.NotMenuDisplayFlag == TRUE ) return FALSE ;
	if( WinData.NotMenuAutoDisplayFlag == TRUE ) return TRUE ;
	if( WinData.WindowModeFlag == FALSE && WinData.MousePosInMenuBarFlag == FALSE ) return FALSE ;
	return TRUE ;
}

// メニューアイテムの情報を取得する( 0:正常終了  -1:エラー  1:区切り線 )
static int _GetMenuItemInfo( HMENU Menu, int Index, MENUITEMINFO *Buffer )
{
	static TCHAR NameBuffer[128] ;

	// 区切り線かどうか調べる
	_MEMSET( Buffer, 0, sizeof( MENUITEMINFO ) ) ;
	Buffer->cbSize = sizeof( MENUITEMINFO ) ;
	Buffer->fMask = MIIM_TYPE ;
	if( GetMenuItemInfo( Menu, Index, TRUE, Buffer ) == 0 ) return -1 ;

	// 区切り線だったら１を返す
	if( Buffer->fType & MFT_SEPARATOR ) return 1 ;

	// 区切り線ではなかったらそれ以外のステータスを取得する
	_MEMSET( Buffer, 0, sizeof( MENUITEMINFO ) ) ;
	Buffer->cbSize = sizeof( MENUITEMINFO ) ;
	Buffer->fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_TYPE ;
	Buffer->fType = MFT_STRING ;
	Buffer->dwTypeData = NameBuffer ;
	Buffer->cch = 128 ;
	if( GetMenuItemInfo( Menu, Index, TRUE, Buffer ) == 0 ) return -1 ;
	
	return 0 ;
}


// 指定の選択項目にサブメニューを付けられるように準備をする
static HMENU MenuItemSubMenuSetup( const TCHAR *ItemName, int ItemID )
{
	HMENU SubMenu ;
	MENUITEMINFO ItemInfo ;
	WINMENUITEMINFO *WinItemInfo ;
	
	// 指定の項目の情報を得る
	WinItemInfo = SearchMenuItemInfo( ItemName, ItemID ) ;
	
	// 見つからなかったらエラー
	if( WinItemInfo == NULL ) return NULL ;

	// サブメニューが既にあるかどうか調べる
	SubMenu = GetSubMenu( WinItemInfo->Menu, WinItemInfo->Index ) ;

	// 既に在ったらそれを返す
	if( SubMenu != NULL ) return SubMenu ;

	// なかったらサブメニューを追加する
	SubMenu = CreateMenu() ;

	// 選択項目の情報にサブメニューを加える
	{
		// 現在の情報を取得
		_GetMenuItemInfo( WinItemInfo->Menu, WinItemInfo->Index, &ItemInfo ) ;
		
		// サブメニューを追加
		ItemInfo.hSubMenu = SubMenu ;

		// 新しい情報をセット
		SetMenuItemInfo( WinItemInfo->Menu, WinItemInfo->Index, TRUE, &ItemInfo ) ;
	}

	// 作成したサブメニューを返す
	return SubMenu ;
}

// メニューを使用しているかどうかを得る
extern int NS_GetUseMenuFlag( void )
{
	return WinData.MenuUseFlag ;
}

// フルスクリーン時にメニューを自動で表示したり非表示にしたり
// するかどうかのフラグをセットする
extern int NS_SetAutoMenuDisplayFlag( int Flag )
{
	WinData.NotMenuAutoDisplayFlag = !Flag ;

	return 0 ;
}

// メニューの選択項目の情報の一覧を作成する時に使用する関数
static int ListupMenuItemInfo( HMENU Menu )
{
	int i, Num, Result ;
	MENUITEMINFO ItemInfo ;
	WINMENUITEMINFO *WinItemInfo ;
	
	// 全ての項目のＩＤをリストに追加する
	Num = GetMenuItemCount( Menu ) ;
	if( Num == -1 ) return -1 ;
	
	for( i = 0 ; i < Num ; i ++ )
	{
		// 項目の情報を取得
		Result = _GetMenuItemInfo( Menu, i, &ItemInfo ) ;
		
		// エラーが発生したら終了
		if( Result == -1 ) return -1 ;
		
		// 区切り線以外だったら情報を追加
		if( Result != 1 )
		{
			WinItemInfo = &WinData.MenuItemInfo[ WinData.MenuItemInfoNum ] ;
			WinItemInfo->Menu = Menu ;
			WinItemInfo->Index = (unsigned short)i ;
			WinItemInfo->ID = (unsigned short)ItemInfo.wID ;
			lstrcpy( WinItemInfo->Name, ItemInfo.dwTypeData ) ;

			WinData.MenuItemInfoNum ++ ;

			// サブメニューがあったらそれも処理
			if( ItemInfo.hSubMenu != NULL )
				ListupMenuItemInfo( ItemInfo.hSubMenu ) ;
		}
	}

	// 終了
	return 0 ;
}

// メニューの選択項目の情報を追加する
static int AddMenuItemInfo( HMENU Menu, int Index, int ID, const TCHAR *Name )
{
	WINMENUITEMINFO *ItemInfo ;

	// 情報が一杯だったらエラー
	if( WinData.MenuItemInfoNum == MAX_MENUITEM_NUM ) return -1 ;

	// 情報の追加
	ItemInfo = &WinData.MenuItemInfo[WinData.MenuItemInfoNum] ;
	ItemInfo->Menu = Menu ;
	ItemInfo->Index = (unsigned short)Index ;
	ItemInfo->ID = (unsigned short)ID ;
	lstrcpy( ItemInfo->Name, Name ) ;

	// 数を増やす
	WinData.MenuItemInfoNum ++ ;
	
	// 終了
	return 0 ;
}

// メニューの選択項目の情報を削除する
static int DeleteMenuItemInfo( const TCHAR *Name, int ID )
{
	LONG_PTR Index ;
	WINMENUITEMINFO *WinItemInfo ;
	
	// 削除する項目の情報を得る
	WinItemInfo = SearchMenuItemInfo( Name, ID ) ;
	if( WinItemInfo == NULL ) return -1 ;
	
	// インデックスを算出
	Index = WinItemInfo - WinData.MenuItemInfo ;

	// 情報を詰める
	if( Index != ( LONG_PTR )( WinData.MenuItemInfoNum - 1 ) )
	{
		_MEMMOVE( &WinData.MenuItemInfo[Index],
				 &WinData.MenuItemInfo[Index+1],
				 sizeof( WINMENUITEMINFO ) * ( WinData.MenuItemInfoNum - Index - 1 ) ) ;
	}
	
	// 数を減らす
	WinData.MenuItemInfoNum -- ;
	
	// 終了
	return 0 ;
}

// メニューの選択項目の情報を取得する
static WINMENUITEMINFO *SearchMenuItemInfo( const TCHAR *Name, int ID )
{
	int i, ItemInfoNum ;
	WINMENUITEMINFO *WinItemInfo ;

	ItemInfoNum = WinData.MenuItemInfoNum ;
	WinItemInfo = WinData.MenuItemInfo ;

	// 名前が有効な場合は名前で調べる
	if( Name != NULL )
	{
		for( i = 0 ; i < ItemInfoNum ; i ++, WinItemInfo ++ )
		{
			if( lstrcmp( Name, WinItemInfo->Name ) == 0 ) break ;
		}
	}
	else
	{
		for( i = 0 ; i < ItemInfoNum ; i ++, WinItemInfo ++ )
		{
			if( (unsigned short)ID == WinItemInfo->ID ) break ;
		}
	}

	// 見つからなかった場合はＮＵＬＬを返す
	if( i == ItemInfoNum ) return NULL ;

	// 正常終了
	return WinItemInfo ;
}

// 新しい選択項目のＩＤを取得する
static int GetNewMenuItemID( void )
{
	int NewItemID ;

	NewItemID = WinData.MenuItemInfoNum ;
	for(;;)
	{
		// 同じＩＤが無かったらループを抜ける
		if( SearchMenuItemInfo( NULL, NewItemID ) == NULL ) break ;

		// 在ったら次のＩＤを試す
		NewItemID ++ ;
		
		// ＩＤの有効範囲を超えたら０にする
		if( NewItemID == 0x10000 ) NewItemID = 0 ;
	}

	// 新しいＩＤを返す
	return NewItemID ;
}

// メニューを自動的に表示したり非表示にしたりする処理を行う
extern int MenuAutoDisplayProcess( void )
{
//	MENUBARINFO MBInfo ;
	int MenuBarHeight ;
	int MouseX, MouseY ;

	// ウインドウモードの場合は何もしない
	if( WinData.WindowModeFlag == TRUE ) return 0 ;

	// メニューバーの高さを得る
/*	_MEMSET( &MBInfo, 0, sizeof( MBInfo ) ) ;
	MBInfo.cbSize = sizeof( MBInfo ) ;
	GetMenuBarInfo( WinData.MainWindow, OBJID_MENU, 0, &MBInfo ) ;
	MenuBarHeight = MBInfo.rcBar.bottom - MBInfo.rcBar.top ;
*/
	MenuBarHeight = 60 ;

	// マウスポインタの座標を得る
	NS_GetMousePoint( &MouseX, &MouseY ) ;

	// Direct3D9 を使用している場合はメニュー位置より下に来たらマウスポインタの表示状態を更新する
	if( GRA2.ValidHardWare )
	{
		// Direct3D9 を使用している場合はメニューが存在する場合は
		// フルスクリーンモードでも常にセットしておく
		if( WinData.Menu && WinData.MenuSetupFlag == FALSE )
		{
			SetMenu( WinData.MainWindow, WinData.Menu ) ;
			WinData.MenuSetupFlag = TRUE ;
		}

		// Ｙ座標が規定以上になったら非表示にする
		if( MouseY > MenuBarHeight )
		{
			NS_SetMouseDispFlag( NS_GetMouseDispFlag() ) ;
		}
	}
	else
	{
		// マウスポインタがメニューの位置にある場合はメニューバーを表示する
		WinData.MousePosInMenuBarFlag = TRUE ;
		if( MouseY < MenuBarHeight )
			WinData.MousePosInMenuBarFlag = TRUE ;
		else
			WinData.MousePosInMenuBarFlag = FALSE ;

		// ＧＤＩを使用するかどうかをセットする
		NS_SetUseGDIFlag( NS_GetUseGDIFlag() ) ;

		// メニューバーとマウスの表示状態を更新する
		NS_SetMouseDispFlag( NS_GetMouseDispFlag() ) ;
		NS_SetDisplayMenuFlag( NS_GetDisplayMenuFlag() ) ;
	}

	// 終了
	return 0 ;	
}










// マウス関係関数

// マウスの表示フラグのセット
extern int NS_SetMouseDispFlag( int DispFlag )
{
	int DispState ;

	// 今までとフラグが同じで、且つメニューが非表示だった場合なにもせず終了
//	if( DispFlag == WinData.MouseDispFlag &&
//		GetDisplayMenuState() == 0 )
//	{
//		
//		
//		return 0 ;
//	}

	// マウスの表示フラグをセットしてメッセージを生成する
	if( DispFlag != -1 )
	{
		WinData.MouseDispFlag = DispFlag ;
	}

	// マウスを表示するかどうかを取得
	DispState = WinData.MouseDispFlag == TRUE || GetDisplayMenuState() == TRUE ;

	// マウスの表示状態が今までと同じ場合は何もしない
	if( DispFlag != -1 && DispState == WinData.MouseDispState ) return 0 ;

	// マウスの表示状態をセット
	if( 1/*!WinData.WindowModeFlag && !WinData.MouseDispFlag*/ )
	{
		if( DispState == FALSE )
		{
			while( ShowCursor( FALSE ) > -1 ){} ;
		}
		else
		{
			while( ShowCursor( TRUE ) < 0 ){} ;
		}
	}

	// マウスのセット信号を出す
	PostMessage( WinData.MainWindow, WM_SETCURSOR, ( WPARAM )WinData.MainWindow, 0 ) ;

	// マウスの表示状態を保存する
	WinData.MouseDispState = DispState ;

	// 終了
	return 0 ;
}

// マウスの位置を取得する
extern int NS_GetMousePoint( int *XBuf, int *YBuf )
{
	POINT MousePos ;

	// スクリーン上での位置を取得
	GetCursorPos( &MousePos ) ; 

	// ウインドウモードの場合クライアント領域内に座標を変換
	if( WinData.WindowModeFlag )
	{
		MousePos.x -= WinData.WindowRect.left ;
		MousePos.y -= WinData.WindowRect.top + GetToolBarHeight() ;

		// 画面が拡大されている場合はその影響を考慮する
		MousePos.x = _DTOL( MousePos.x / WinData.WindowSizeExRateX ) ;
		MousePos.y = _DTOL( MousePos.y / WinData.WindowSizeExRateY ) ;
	}

	// バッファに書き込む
	if( XBuf ) *XBuf = MousePos.x ;
	if( YBuf ) *YBuf = MousePos.y ;

	// 終了
	return 0 ;
}

// マウスの位置をセットする
extern int NS_SetMousePoint( int PointX , int PointY )
{
	int ScreenSizeX , ScreenSizeY ;

	// 位置を補正する
	{
		// 画面外にカーソルが出ていた場合の補正
		NS_GetDrawScreenSize( &ScreenSizeX , &ScreenSizeY ) ;

		// その前にウインドウのスケーリングに応じて座標を補正
		PointX = _DTOL( PointX * WinData.WindowSizeExRateX ) ;
		PointY = _DTOL( PointY * WinData.WindowSizeExRateY ) ;
		ScreenSizeX = _DTOL( ScreenSizeX * WinData.WindowSizeExRateX ) ;
		ScreenSizeY = _DTOL( ScreenSizeY * WinData.WindowSizeExRateY ) ;

		if( PointX < 0 ) 			PointX = 0 ;
		else
		if( PointX > ScreenSizeX )	PointX = ScreenSizeX ;

		if( PointY < 0 )			PointY = 0 ;
		else
		if( PointY > ScreenSizeY )	PointY = ScreenSizeY ;

		// ウインドウモードの場合の位置補正
		if( WinData.WindowModeFlag ) 
		{
			PointX += WinData.WindowRect.left ;
			PointY += WinData.WindowRect.top ;
		}
	}

	// 位置をセットする
	SetCursorPos( PointX , PointY ) ;

	// 終了
	return 0 ;
}

#ifndef DX_NON_INPUT

// マウスホイールの回転量を得る
extern int NS_GetMouseWheelRotVol( int CounterReset )
{
	int Vol ;

	// DirectInput を使用しているかどうかで処理を分岐
	if( CheckUseDirectInputMouse() == FALSE )
	{
		Vol = WinData.MouseMoveZ / WHEEL_DELTA ;
		if( CounterReset )
		{
			WinData.MouseMoveZ -= Vol * WHEEL_DELTA ;
		}
	}
	else
	{
		Vol = GetDirectInputMouseMoveZ( CounterReset ) ;
	}

	return Vol ;
}

// 水平マウスホイールの回転量を得る
extern int NS_GetMouseHWheelRotVol( int CounterReset )
{
	int Vol ;

	Vol = WinData.MouseMoveHZ / WHEEL_DELTA ;
	if( CounterReset )
	{
		WinData.MouseMoveHZ -= Vol * WHEEL_DELTA ;
	}

	return Vol ;
}

// マウスホイールの回転量を得る( 戻り値が float 型 )
extern float NS_GetMouseWheelRotVolF( int CounterReset )
{
	float Vol ;

	// DirectInput を使用しているかどうかで処理を分岐
	if( CheckUseDirectInputMouse() == FALSE )
	{
		Vol = ( float )WinData.MouseMoveZ / WHEEL_DELTA ;
		if( CounterReset )
		{
			WinData.MouseMoveZ = 0 ;
		}
	}
	else
	{
		Vol = GetDirectInputMouseMoveZF( CounterReset ) ;
	}

	return Vol ;
}

// 水平マウスホイールの回転量を得る( 戻り値が float 型 )
extern float NS_GetMouseHWheelRotVolF( int CounterReset )
{
	float Vol ;

	Vol = ( float )WinData.MouseMoveHZ / WHEEL_DELTA ;
	if( CounterReset )
	{
		WinData.MouseMoveHZ = 0 ;
	}

	return Vol ;
}

// マウスの入力情報を保存する
static	void		StockMouseInputInfo( int Button )
{
	WINMOUSEINPUT *MInput ;

	// ログの数が最大数に達していたら何もせず終了
	if( WinData.MouseInputNum >= WIN_MOUSEINPUT_LOG_NUM ) return ;

	// 情報を保存
	MInput = &WinData.MouseInput[ WinData.MouseInputNum ] ;
	NS_GetMousePoint( &MInput->ClickX, &MInput->ClickY ) ;
	MInput->Button = Button ;

	// 情報の数を増やす
	WinData.MouseInputNum ++ ;
}

// マウス入力

// マウスのクリック情報を取得する( 戻り値  -1:ログが無かった )
extern int NS_GetMouseInputLog( int *Button, int *ClickX, int *ClickY, int LogDelete )
{
	WINMOUSEINPUT *MInput ;

	// ログが無かったら -1 を返す
	if( WinData.MouseInputNum == 0 ) return -1 ;

	// 情報を返す
	MInput = &WinData.MouseInput[ 0 ] ;
	if( Button ) *Button = MInput->Button ;
	if( ClickX ) *ClickX = MInput->ClickX ;
	if( ClickY ) *ClickY = MInput->ClickY ;

	// ログを削除する指定がある場合は削除する
	if( LogDelete )
	{
		WinData.MouseInputNum -- ;
		if( WinData.MouseInputNum )
			_MEMMOVE( WinData.MouseInput, &WinData.MouseInput[ 1 ], sizeof( WINMOUSEINPUT ) * WinData.MouseInputNum ) ;
	}

	// 終了
	return 0 ;
}

#endif // DX_NON_INPUT






// ウエイト系関数

// 指定の時間だけ処理をとめる
extern int NS_WaitTimer( int WaitTime )
{
	LONGLONG StartTime, EndTime ;

	StartTime = NS_GetNowHiPerformanceCount( FALSE ) ;

	// 4msec前まで寝る
	WaitTime *= 1000 ;
	if( WaitTime > 4000 )
	{
		// 指定時間の間メッセージループ
		EndTime = StartTime + WaitTime - 4000 ;
		while( ProcessMessage() == 0 && EndTime > NS_GetNowHiPerformanceCount( FALSE ) ) Sleep( 1 ) ;
	}

	// 4msec以下の分は正確に待つ
	EndTime = StartTime + WaitTime ;
	while( EndTime > NS_GetNowHiPerformanceCount( FALSE ) ){}

	// 終了
	return 0 ;
}

#ifndef DX_NON_INPUT

// キーの入力待ち
extern int NS_WaitKey( void )
{
	int BackCode = 0 ;

	while( ProcessMessage() == 0 && CheckHitKeyAll() != 0 ){Sleep(1);}
	while( ProcessMessage() == 0 && ( BackCode = CheckHitKeyAll() ) == 0 ){Sleep(1);}
//	while( ProcessMessage() == 0 && CheckHitKeyAll() != 0 ){Sleep(1);}

	return BackCode ;
}

#endif // DX_NON_INPUT































// 補助関数
extern void _FileTimeToLocalDateData( FILETIME *FileTime, DATEDATA *DateData )
{
	SYSTEMTIME SysTime;
	FILETIME LocalTime;

	FileTimeToLocalFileTime( FileTime, &LocalTime );
	FileTimeToSystemTime( &LocalTime, &SysTime );
	DateData->Year = SysTime.wYear ;
	DateData->Mon  = SysTime.wMonth ;
	DateData->Day  = SysTime.wDay ;
	DateData->Hour = SysTime.wHour ;
	DateData->Min  = SysTime.wMinute ;
	DateData->Sec  = SysTime.wSecond ;
}

// 指定の基本イメージを使用して UpdateLayeredWindow を行う
extern int UpdateBackBufferTransColorWindow( const BASEIMAGE *SrcImage, const RECT *SrcImageRect, HDC Direct3DDC, int NotColorKey, int PreMultipliedAlphaImage )
{
	HDC dc, ddc, memdc ;
	SIZE wsize ;
	POINT wpos, pos ;
	BYTE *Dst ;
	BYTE *Src ;
//	RECT wrect, wcrect ;
//	RECT wrect, wcrect ;
	BLENDFUNCTION blend ;
	HGDIOBJ old ;
	COLORREF trans ;
	int w, h ;
	int Systembar ;

	// システムバーを持つウインドウスタイルかどうかを算出
	Systembar = ( WStyle_WindowModeTable[ WinData.WindowStyle ] & WS_CAPTION ) != 0;

	// 共通処理
	ddc = GetDC( NULL ) ;
	dc = GetDC( GetDisplayWindowHandle() ) ;

	pos.x = 0 ;
	pos.y = 0 ;
//	GetWindowRect( GetDisplayWindowHandle(), &wrect ) ;
//	GetClientRect( GetDisplayWindowHandle(), &wcrect ) ;
//	wpos.x = wrect.left + wcrect.left ;
//	wpos.y = wrect.top  + wcrect.top  ;
//	wsize.cx = wcrect.right - wcrect.left ;
//	wsize.cy = wcrect.bottom - wcrect.top ;
	wpos.x = WinData.WindowEdgeRect.left ;
	wpos.y = WinData.WindowEdgeRect.top ;
	NS_GetDrawScreenSize( &w, &h ) ;
	wsize.cx = w ;
	wsize.cy = h ;

	// Direct3DDC が有効かどうかで処理を分岐
	trans = ( ( GBASE.TransColor & 0xff0000 ) >> 16 ) | 
			( ( GBASE.TransColor & 0x0000ff ) << 16 ) |
			  ( GBASE.TransColor & 0x00ff00 ) ;
	if( Direct3DDC )
	{
		WinData.UpdateLayeredWindow( GetDisplayWindowHandle(), ddc, &wpos, &wsize, Direct3DDC, &pos, trans, &blend, ULW_COLORKEY ) ;
	}
	else
	{
		RECT SrcRect ;
		BASEIMAGE SrcImageTemp ;

		// SrcImage が NULL 以外で、且つ SrcImageRect が NULL 以外の場合は仮の BASEIMAGE を用意する
		if( SrcImage != NULL && SrcImageRect != NULL )
		{
			if( NS_DerivationBaseImage( SrcImage, SrcImageRect->left, SrcImageRect->top, SrcImageRect->right, SrcImageRect->bottom, &SrcImageTemp ) < 0 )
			{
				return -1 ;
			}

			SrcImage = &SrcImageTemp ;
		}

		// ビットマップのサイズが違ったらビットマップの作り直し
		if( WinData.BackBufferTransBitmapSize.cx != SrcImage->Width ||
			WinData.BackBufferTransBitmapSize.cy != SrcImage->Height )
		{
			BITMAPINFO BHead ;
			HDC dc ;

			if( WinData.BackBufferTransBitmap )
			{
				DeleteObject( WinData.BackBufferTransBitmap ) ;
				WinData.BackBufferTransBitmap = NULL ;
			}

			_MEMSET( &BHead, 0, sizeof( BHead ) ) ;
			BHead.bmiHeader.biSize = sizeof( BHead ) ;
			BHead.bmiHeader.biWidth = SrcImage->Width ;
			BHead.bmiHeader.biHeight = -SrcImage->Height;
			if( Systembar )
			{
				BHead.bmiHeader.biHeight -= WinData.SystembarHeight;
			}
			BHead.bmiHeader.biBitCount = 32 ;
			BHead.bmiHeader.biPlanes = 1 ;
			dc = GetDC( GetDisplayWindowHandle() ) ;
			WinData.BackBufferTransBitmap = CreateDIBSection( dc, &BHead, DIB_RGB_COLORS, &WinData.BackBufferTransBitmapImage, NULL, 0 ) ;
			ReleaseDC( GetDisplayWindowHandle(), dc ) ;
			NS_CreateARGB8ColorData( &WinData.BackBufferTransBitmapColorData ) ;
			_MEMSET( WinData.BackBufferTransBitmapImage, 0, -BHead.bmiHeader.biHeight * SrcImage->Width * 4 ) ;

			WinData.BackBufferTransBitmapSize.cx = SrcImage->Width ;
			WinData.BackBufferTransBitmapSize.cy = SrcImage->Height ;
		}

		// イメージのアドレスをセット
		Dst = ( BYTE * )WinData.BackBufferTransBitmapImage ;
		if( Systembar )
		{
			Dst += WinData.SystembarHeight * SrcImage->Width * 4 ;
		}
		Src = ( BYTE * )SrcImage->GraphData ;

		// ビットマップにデータを転送
		SrcRect.left = 0 ;
		SrcRect.top = 0 ;
		SrcRect.right = SrcImage->Width ;
		SrcRect.bottom = SrcImage->Height ;
		if( NotColorKey == FALSE )
		{
			// VISTA以降の場合とそれ以外で処理を分岐
			if( WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA )
			{
				DWORD i, j, Size, PackNum, NokoriNum ;
				DWORD TransColor, SrcAddPitch ;

				PackNum = SrcImage->Width / 4 ;
				NokoriNum = SrcImage->Width - PackNum * 4 ;

				if( SrcImage->ColorData.AlphaMask == 0x00000000 &&
					SrcImage->ColorData.RedMask   == 0x00ff0000 &&
					SrcImage->ColorData.GreenMask == 0x0000ff00 &&
					SrcImage->ColorData.BlueMask  == 0x000000ff )
				{
					SrcAddPitch = SrcImage->Pitch - SrcImage->Width * 4 ;
					TransColor = GBASE.TransColor & 0x00ffffff ;
					for( i = SrcImage->Height ; i ; i --, Src += SrcAddPitch )
					{
						for( j = SrcImage->Width ; j ; j --, Src += 4, Dst += 4 )
						{
							if( ( *( ( DWORD * )Src ) & 0x00ffffff ) == TransColor )
							{
								*( ( DWORD * )Dst ) = 0 ;
							}
							else
							{
								*( ( DWORD * )Dst ) = *( ( DWORD * )Src ) | 0xff000000 ;
							}
						}
					}
				}
				else
				if( SrcImage->ColorData.AlphaMask == 0x00000000 &&
					SrcImage->ColorData.RedMask   == 0x0000f800 &&
					SrcImage->ColorData.GreenMask == 0x000007e0 &&
					SrcImage->ColorData.BlueMask  == 0x0000001f )
				{
					SrcAddPitch = SrcImage->Pitch - SrcImage->Width * 2 ;
					TransColor =	( ( ( GBASE.TransColor & 0x00ff0000 ) >> ( 16 + 3 ) ) << 11 ) |
									( ( ( GBASE.TransColor & 0x0000ff00 ) >> (  8 + 2 ) ) <<  5 ) |
									( ( ( GBASE.TransColor & 0x000000ff ) >> (  0 + 3 ) ) <<  0 ) ;
					for( i = SrcImage->Height ; i ; i --, Src += SrcAddPitch )
					{
						for( j = PackNum ; j ; j --, Src += 2 * 4, Dst += 4 * 4 )
						{
							if( ( ( WORD * )Src )[ 0 ] == TransColor )
							{
								( ( DWORD * )Dst )[ 0 ] = 0 ;
							}
							else
							{
								( ( DWORD * )Dst )[ 0 ] =
									( ( ( ( WORD * )Src )[ 0 ] & 0xf800 ) << ( 16 + 3 - 11 ) ) |
									( ( ( ( WORD * )Src )[ 0 ] & 0x07e0 ) << (  8 + 2 -  5 ) ) |
									( ( ( ( WORD * )Src )[ 0 ] & 0x001f ) << (  0 + 3 -  0 ) ) | 0xff000000 ;
							}

							if( ( ( WORD * )Src )[ 1 ] == TransColor )
							{
								( ( DWORD * )Dst )[ 1 ] = 0 ;
							}
							else
							{
								( ( DWORD * )Dst )[ 1 ] =
									( ( ( ( WORD * )Src )[ 1 ] & 0xf800 ) << ( 16 + 3 - 11 ) ) |
									( ( ( ( WORD * )Src )[ 1 ] & 0x07e0 ) << (  8 + 2 -  5 ) ) |
									( ( ( ( WORD * )Src )[ 1 ] & 0x001f ) << (  0 + 3 -  0 ) ) | 0xff000000 ;
							}

							if( ( ( WORD * )Src )[ 2 ] == TransColor )
							{
								( ( DWORD * )Dst )[ 2 ] = 0 ;
							}
							else
							{
								( ( DWORD * )Dst )[ 2 ] =
									( ( ( ( WORD * )Src )[ 2 ] & 0xf800 ) << ( 16 + 3 - 11 ) ) |
									( ( ( ( WORD * )Src )[ 2 ] & 0x07e0 ) << (  8 + 2 -  5 ) ) |
									( ( ( ( WORD * )Src )[ 2 ] & 0x001f ) << (  0 + 3 -  0 ) ) | 0xff000000 ;
							}

							if( ( ( WORD * )Src )[ 3 ] == TransColor )
							{
								( ( DWORD * )Dst )[ 3 ] = 0 ;
							}
							else
							{
								( ( DWORD * )Dst )[ 3 ] =
									( ( ( ( WORD * )Src )[ 3 ] & 0xf800 ) << ( 16 + 3 - 11 ) ) |
									( ( ( ( WORD * )Src )[ 3 ] & 0x07e0 ) << (  8 + 2 -  5 ) ) |
									( ( ( ( WORD * )Src )[ 3 ] & 0x001f ) << (  0 + 3 -  0 ) ) | 0xff000000 ;
							}
						}

						for( j = NokoriNum ; j ; j --, Src += 2, Dst += 4 )
						{
							if( *( ( WORD * )Src ) == TransColor )
							{
								*( ( DWORD * )Dst ) = 0 ;
							}
							else
							{
								*( ( DWORD * )Dst ) =
									( ( *( ( WORD * )Src ) & 0xf800 ) << ( 16 + 3 - 11 ) ) |
									( ( *( ( WORD * )Src ) & 0x07e0 ) << (  8 + 2 -  5 ) ) |
									( ( *( ( WORD * )Src ) & 0x001f ) << (  0 + 3 -  0 ) ) | 0xff000000 ;
							}
						}
					}
				}
				else
				{
					NS_GraphColorMatchBltVer2(
						Dst, SrcImage->Width * 4, &WinData.BackBufferTransBitmapColorData,
						Src, SrcImage->Pitch, &SrcImage->ColorData,
						NULL, 0, NULL,
						pos, &SrcRect, FALSE, FALSE, 0, 0, 0, 0, 0, 0 ) ;

					Size = WinData.BackBufferTransBitmapSize.cx * WinData.BackBufferTransBitmapSize.cy ;
					TransColor = GBASE.TransColor & 0x00ffffff ;
					for( i = Size ; i ; i --, Dst += 4 )
					{
						if( ( *( ( DWORD * )Dst ) & 0x00ffffff ) == TransColor )
						{
							*( ( DWORD * )Dst ) = 0 ;
						}
						else
						{
							Dst[ 3 ] = 255 ;
						}
					}
				}
			}
			else
			{
				COLORDATA XRGB8ColorData ;

				if( Systembar )
				{
					_MEMSETD( WinData.BackBufferTransBitmapImage, GBASE.TransColor & 0x00ffffff, WinData.SystembarHeight * SrcImage->Width ) ;
				}
				NS_CreateXRGB8ColorData( &XRGB8ColorData ) ;
				NS_GraphColorMatchBltVer2(
					Dst, SrcImage->Width * 4, &XRGB8ColorData,
					Src, SrcImage->Pitch, &SrcImage->ColorData,
					NULL, 0, NULL,
					pos, &SrcRect, FALSE, FALSE, 0, 0, 0, 0, 0, 0 ) ;
			}
		}
		else
		{
			DWORD i, j, Size ;
			DWORD TransColor, SrcAddPitch ;
			int UseTransColor ;

			UseTransColor = WinData.UseUpdateLayerdWindowFlag == FALSE ? TRUE : FALSE ;

			if( SrcImage->ColorData.AlphaMask == 0xff000000 &&
				SrcImage->ColorData.RedMask   == 0x00ff0000 &&
				SrcImage->ColorData.GreenMask == 0x0000ff00 &&
				SrcImage->ColorData.BlueMask  == 0x000000ff )
			{
				SrcAddPitch = SrcImage->Pitch - SrcImage->Width * 4 ;
				TransColor = GBASE.TransColor & 0x00ffffff ;
				if( PreMultipliedAlphaImage )
				{
					if( UseTransColor )
					{
						for( i = SrcImage->Height ; i ; i --, Src += SrcAddPitch )
						{
							for( j = SrcImage->Width ; j ; j --, Src += 4, Dst += 4 )
							{
								if( ( *( ( DWORD * )Src ) & 0x00ffffff ) == TransColor )
								{
									*( ( DWORD * )Dst ) = 0 ;
								}
								else
								{
									*( ( DWORD * )Dst ) = *( ( DWORD * )Src ) ;
								}
							}
						}
					}
					else
					{
						for( i = SrcImage->Height ; i ; i --, Src += SrcAddPitch )
						{
							for( j = SrcImage->Width ; j ; j --, Src += 4, Dst += 4 )
							{
								*( ( DWORD * )Dst ) = *( ( DWORD * )Src ) ;
							}
						}
					}
				}
				else
				{
					if( UseTransColor )
					{
						for( i = SrcImage->Height ; i ; i --, Src += SrcAddPitch )
						{
							for( j = SrcImage->Width ; j ; j --, Src += 4, Dst += 4 )
							{
								if( ( *( ( DWORD * )Src ) & 0x00ffffff ) == TransColor || Src[ 3 ] == 0 )
								{
									*( ( DWORD * )Dst ) = 0 ;
								}
								else
								if( Src[ 3 ] == 255 )
								{
									*( ( DWORD * )Dst ) = *( ( DWORD * )Src ) ;
								}
								else
								{
									Dst[ 0 ] = ( Src[ 0 ] * Src[ 3 ] ) >> 8 ;
									Dst[ 1 ] = ( Src[ 1 ] * Src[ 3 ] ) >> 8 ;
									Dst[ 2 ] = ( Src[ 2 ] * Src[ 3 ] ) >> 8 ;
									Dst[ 3 ] = Src[ 3 ] ;
								}
							}
						}
					}
					else
					{
						for( i = SrcImage->Height ; i ; i --, Src += SrcAddPitch )
						{
							for( j = SrcImage->Width ; j ; j --, Src += 4, Dst += 4 )
							{
								if( Src[ 3 ] == 0 )
								{
									*( ( DWORD * )Dst ) = 0 ;
								}
								else
								if( Src[ 3 ] == 255 )
								{
									*( ( DWORD * )Dst ) = *( ( DWORD * )Src ) ;
								}
								else
								{
									Dst[ 0 ] = ( Src[ 0 ] * Src[ 3 ] ) >> 8 ;
									Dst[ 1 ] = ( Src[ 1 ] * Src[ 3 ] ) >> 8 ;
									Dst[ 2 ] = ( Src[ 2 ] * Src[ 3 ] ) >> 8 ;
									Dst[ 3 ] = Src[ 3 ] ;
								}
							}
						}
					}
				}
			}
			else
			{
				NS_GraphColorMatchBltVer2(
					Dst, SrcImage->Width * 4, &WinData.BackBufferTransBitmapColorData,
					Src, SrcImage->Pitch, &SrcImage->ColorData,
					NULL, 0, NULL,
					pos, &SrcRect, FALSE, UseTransColor, GBASE.TransColor, 0, 0, 0, 0, 0 ) ;

				Size = WinData.BackBufferTransBitmapSize.cx * WinData.BackBufferTransBitmapSize.cy ;
				if( PreMultipliedAlphaImage == FALSE )
				{
					for( i = Size ; i ; i --, Dst += 4 )
					{
						if( Dst[ 3 ] == 0 )
						{
							*( ( DWORD * )Dst ) = 0 ;
						}
						else
						if( Dst[ 3 ] != 255 ) 
						{
							Dst[ 0 ] = ( Dst[ 0 ] * Dst[ 3 ] ) >> 8 ;
							Dst[ 1 ] = ( Dst[ 1 ] * Dst[ 3 ] ) >> 8 ;
							Dst[ 2 ] = ( Dst[ 2 ] * Dst[ 3 ] ) >> 8 ;
						}
					}
				}
			}
		}

		// 作成したビットマップを元に UpdateLeyeredWindow を行う
		memdc = CreateCompatibleDC( dc ) ;
		old = SelectObject( memdc, WinData.BackBufferTransBitmap ) ;

		// Vista 以降の場合は必ずアルファブレンドする
		if( Systembar )
		{
			wsize.cy += WinData.SystembarHeight ;
		}
		if( NotColorKey == FALSE && WinData.WindowsVersion < DX_WINDOWSVERSION_VISTA )
		{
			WinData.UpdateLayeredWindow( GetDisplayWindowHandle(), ddc, &wpos, &wsize, memdc, &pos, trans, NULL, ULW_COLORKEY ) ;
		}
		else
		{
			blend.BlendOp = AC_SRC_OVER ;
			blend.BlendFlags = 0 ;
			blend.SourceConstantAlpha = 255 ;
			blend.AlphaFormat = AC_SRC_ALPHA ;
			WinData.UpdateLayeredWindow( GetDisplayWindowHandle(), ddc, &wpos, &wsize, memdc, &pos, 0, &blend, ULW_ALPHA ) ;
		}

		SelectObject( memdc, old ) ;
		DeleteDC( memdc ) ;
	}

	ReleaseDC( GetDisplayWindowHandle(), dc ) ;
	ReleaseDC( NULL, ddc ) ;

	return 0 ;
}


























// 情報出力系

// ＣＰＵＩＤ命令定義
#define CPUID	__asm _emit 0fH  __asm _emit 0A2H


// ＯＳやＤｉｒｅｃｔＸのバージョンを出力する
extern int OutSystemInfo( void )
{
	TCHAR Str[256] ;

	DXST_ERRORLOG_ADD( _T( "システムの情報を出力します\n" ) ) ;
	DXST_ERRORLOG_TABADD ;

#ifndef DX_NON_LITERAL_STRING
	// ＤＸライブラリのバージョンを出力する
	{
		_TSPRINTF( _DXWTP( Str ), _DXWTR( "ＤＸライブラリ Ver%s\n" ), DXLIB_VERSION_STR ) ;

		DXST_ERRORLOG_ADD( Str ) ;
	}
#endif

	// ＣＰＵのコア数を取得する
	{
		SYSTEM_INFO SystemInfo ;

		GetSystemInfo( &SystemInfo ) ;
		WinData.ProcessorNum = SystemInfo.dwNumberOfProcessors ;

		DXST_ERRORLOGFMT_ADD(( _T( "論理プロセッサの数 : %d" ), WinData.ProcessorNum )) ;
	}

	// ＯＳのバージョンを出力する
	{
		OSVERSIONINFO OsVersionInfo ;

		DXST_ERRORLOG_ADD( _T( "ＯＳ  " ) ) ;
		lstrcpy( Str, _T( "Windows" ) ) ;

		_MEMSET( &OsVersionInfo, 0, sizeof( OsVersionInfo ) ) ;
		OsVersionInfo.dwOSVersionInfoSize = sizeof( OsVersionInfo ) ;

		GetVersionEx( &OsVersionInfo ) ;

		// プラットフォームによって処理を分ける
		switch( OsVersionInfo.dwPlatformId )
		{
		// Windows3.1系(まずここにくることはないと思うが…)
		case VER_PLATFORM_WIN32s :
			lstrcat( Str, _T( "3.1" ) ) ;
			WinData.WindowsVersion = DX_WINDOWSVERSION_31 ;
			break ;

		// Windows95, 98, Me
		case VER_PLATFORM_WIN32_WINDOWS :

			// マイナーバージョンによって分岐
			switch( OsVersionInfo.dwMinorVersion )
			{
			case 0 :	// Windows95 
				lstrcat( Str, _T( "95" ) ) ;
				WinData.WindowsVersion = DX_WINDOWSVERSION_95 ;
				if( OsVersionInfo.szCSDVersion[0] == _T( 'C' ) )
				{
					lstrcat( Str, _T( "OSR2" ) ) ;
				}
				break ;

			case 10 :	// Windows98
				lstrcat( Str, _T( "98" ) ) ;
				WinData.WindowsVersion = DX_WINDOWSVERSION_98 ;
				if( OsVersionInfo.szCSDVersion[0] == _T( 'A' ) )
				{
					lstrcat( Str, _T( "SE" ) ) ;
				}
				break ;

			case 90 :
				lstrcat( Str, _T( "Me" ) ) ;
				WinData.WindowsVersion = DX_WINDOWSVERSION_ME ;
				break ;
			}
			break ;

		// NT 系
		case VER_PLATFORM_WIN32_NT :

			// メジャーバージョンによって処理を分岐
			switch( OsVersionInfo.dwMajorVersion )
			{
			case 3 :
				lstrcat( Str, _T( "NT 3.51" ) ) ;
				WinData.WindowsVersion = DX_WINDOWSVERSION_NT31 ;
				break ;

			case 4 :
				lstrcat( Str, _T( "4.0" ) ) ;
				WinData.WindowsVersion = DX_WINDOWSVERSION_NT40 ;
				break ;

			case 5 :	
				switch( OsVersionInfo.dwMinorVersion )
				{
				case 0 :
					lstrcat( Str, _T( "2000" ) ) ;
					WinData.WindowsVersion = DX_WINDOWSVERSION_2000 ;
					break ;

				case 1 :
					lstrcat( Str, _T( "XP" ) ) ;
					WinData.WindowsVersion = DX_WINDOWSVERSION_XP ;
					break ;
				}
				break ;

			case 6 :
				switch( OsVersionInfo.dwMinorVersion )
				{
				case 0 :
					lstrcat( Str, _T( "Vista" ) ) ;
					WinData.WindowsVersion = DX_WINDOWSVERSION_VISTA ;
					break ;

				case 1 :
					lstrcat( Str, _T( "7" ) ) ;
					WinData.WindowsVersion = DX_WINDOWSVERSION_7 ;
					break ;

				case 2 :
					lstrcat( Str, _T( "8" ) ) ;
					WinData.WindowsVersion = DX_WINDOWSVERSION_8 ;
					break ;
				}
				break ;
			}
		}
		_TSPRINTF( _DXWTP( WinData.PcInfo.OSString ), _DXWTR( "%s ( Build %d %s )" ), Str, OsVersionInfo.dwBuildNumber, OsVersionInfo.szCSDVersion ) ;
		DXST_ERRORLOGFMT_ADD(( _T( "%s" ), WinData.PcInfo.OSString )) ;
	}

	// ＤｉｒｅｃｔＸのバージョンを出力

	// 今時 DirectX 7.0 がインストールされていない環境はまず無いので、起動時間を短くする為にチェックはしない
	WinData.DirectXVersion = DX_DIRECTXVERSION_7 ;
	if( 0 )
	{
		// それぞれＤＬＬのファンクションアドレスを取得するためのポインタ
		typedef HRESULT ( WINAPI *DIRECTDRAWCREATEFUNC )( GUID *, D_IDirectDraw **, IUnknown * ); 
		typedef HRESULT ( WINAPI *DIRECTDRAWCREATEEXFUNC )( GUID *, VOID **, REFIID, IUnknown * ); 
//		typedef HRESULT ( WINAPI *DIRECTINPUTCREATEFUNC )( HINSTANCE, DWORD, LPDIRECTINPUT *, IUnknown * ); 
		DIRECTDRAWCREATEFUNC DirectDrawCreateFunc = NULL ; 
		DIRECTDRAWCREATEEXFUNC DirectDrawCreateExFunc = NULL ; 
//		DIRECTINPUTCREATEFUNC DirectInputCreateFunc = NULL ; 
		FARPROC DirectInputCreateFunc = NULL ;

		// それぞれ各ＤＬＬのハンドルを格納する変数
		HINSTANCE DDrawDLL = NULL ;	
		HINSTANCE DInputDLL = NULL ;
		HINSTANCE D3D8DLL = NULL ;
//		HINSTANCE DPNHPASTDLL = NULL ;

		// テスト用のそれぞれのインターフェースのポインタ
		D_IDirectDraw			*DDraw = NULL ;
		D_IDirectDraw2			*DDraw2 = NULL ;
		D_IDirectDrawSurface	*DDrawSurf = NULL ;
		D_IDirectDrawSurface3	*DDrawSurf3 = NULL ;
		D_IDirectDrawSurface4	*DDrawSurf4 = NULL ;
		D_IDirectMusic			*DMusic = NULL ;
		D_IDirectDraw7			*DDraw7 = NULL ;

		D_DDSURFACEDESC			ddsd;


		// バージョン情報を初期化
		WinData.DirectXVersion = DX_DIRECTXVERSION_NON ;

		// 確認開始
		for(;;)
		{
/*			// DirectX9 以上のバージョンの確認
			{
				int ComInitializeFlag = FALSE ;
				DXDIAG_INIT_PARAMS DiagInitializeParam ;
				IDxDiagProvider *DxDiagProvider = NULL ;
				IDxDiagContainer *DxDiagRoot = NULL ;
				IDxDiagContainer *DxDiagSystemInfo = NULL ;
				VARIANT Var ;
				int VarInitializeFlag = FALSE ;
				unsigned int VersionMajor = 0 ;
				unsigned int VersionMinor = 0 ;
				unsigned int VersionLetter = 0 ;
				int GetVersionFlag = FALSE ;

				// Com の初期化
				if( FAILED( WinAPIData.Win32Func.CoInitializeExFunc( NULL ) ) ) goto DX9END ;
				ComInitializeFlag = TRUE ;

				// IDxDiagProvider インターフェースの作成
				if( FAILED( WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DxDiagProvider, 
												NULL,
												CLSCTX_INPROC_SERVER,
												IID_IDxDiagProvider,
												(void **)&DxDiagProvider ) ) ) goto DX9END ;

				// IDxDiagProvider の初期化
				_MEMSET( &DiagInitializeParam, 0, sizeof( DXDIAG_INIT_PARAMS ) ) ;
				DiagInitializeParam.dwSize = sizeof( DXDIAG_INIT_PARAMS ) ;
				DiagInitializeParam.dwDxDiagHeaderVersion = DXDIAG_DX9_SDK_VERSION ;
				DiagInitializeParam.bAllowWHQLChecks = false ;
				DiagInitializeParam.pReserved = NULL ;
				if( FAILED( DxDiagProvider->Initialize( &DiagInitializeParam ) ) ) goto DX9END ;

				// DxDiagProvider の ルート Container を得る
				if( FAILED( DxDiagProvider->GetRootContainer( &DxDiagRoot ) ) ) goto DX9END ;

				// DxDiag の システム情報 Container を得る
				if( FAILED( DxDiagRoot->GetChildContainer( L"DxDiag_SystemInfo", &DxDiagSystemInfo ) ) ) goto DX9END ;

				// Directx のメジャーバージョンを得る
				{
					// バージョン情報構造体を初期化する
					VariantInitialize( &Var ) ;
					VarInitializeFlag = TRUE ;

					if( FAILED( DxDiagSystemInfo->GetProp( L"dwDirectXVersionMajor", &Var ) ) ) goto DX9END ;

					// 取得したデータが符号なしダブルバイト形式ではなかったらエラー
					if( Var.vt  != VT_UI4 ) goto DX9END ;

					// メジャーバージョン値の保存
					VersionMajor = Var.ulVal ;

					// バージョン情報構造体の後始末
					VariantClear( &Var ) ;
					VarInitializeFlag = FALSE ;
				}

				// DirectX のマイナーバージョンを得る
				{
					// バージョン情報構造体を初期化する
					VariantInitialize( &Var ) ;
					VarInitializeFlag = TRUE ;

					if( FAILED( DxDiagSystemInfo->GetProp( L"dwDirectXVersionMinor", &Var ) ) ) goto DX9END ;

					// 取得したデータが符号なしダブルバイト形式ではなかったらエラー
					if( Var.vt  != VT_UI4 ) goto DX9END ;

					// マイナーバージョン値の保存
					VersionMinor = Var.ulVal ;

					// バージョン情報構造体の後始末
					VariantClear( &Var ) ;
				}
	
				// DirectX のバージョンの最後に文字があるか調べる
				{
					// バージョン情報構造体を初期化する
					VariantInitialize( &Var ) ;
					VarInitializeFlag = TRUE ;

					if( FAILED( DxDiagSystemInfo->GetProp( L"dwDirectXVersionLetter", &Var ) ) ) goto DX9END ;

					// 取得したデータが文字列ではなかったらエラー
					if( Var.vt  != VT_BSTR ) goto DX9END ;

					// ４バイト文字になっているので２バイト文字に変換する
					{
                        char Dest[10];
                        WCharToMBChar( CP_ACP, Var.bstrVal, Dest, 10 * sizeof( char ) );
						VersionLetter = (unsigned int)Dest[0] ;
					}

					// バージョン情報構造体の後始末
					VariantClear( &Var ) ;
				}

				// ここまで来ていたら DirectX のバージョンの取得に成功したことを意味する
				GetVersionFlag = TRUE ;

				// DirectX のバージョン情報を生成
				WinData.DirectXVersion = VersionMajor << 16 |
											VersionMinor << 8 |
											VersionLetter ;

DX9END :
				// 情報の後始末
				if( VarInitializeFlag == TRUE ) VariantClear( &Var ) ;
				if( DxDiagSystemInfo != NULL ) DxDiagSystemInfo->Release() ;
				if( DxDiagRoot != NULL ) DxDiagRoot->Release() ;
				if( DxDiagProvider != NULL ) DxDiagProvider->Release() ;
				if( ComInitializeFlag ) WinAPIData.Win32Func.CoUninitializeFunc() ;

				// バージョンが取得できていたらここで終了
				break ;
			}
*/
			// ＤｉｒｅｃｔＸ９かどうかを取得する
//			if( GetDirectX9Version( (unsigned int *)&WinData.DirectXVersion ) == 0 ) break ;

			// ＤｉｒｅｃｔＸがまずあるか調べる
			if( ( DDrawDLL = LoadLibrary( _T( "DDRAW.DLL" ) ) ) == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "DirectX がインストールされていません\n" ) ) ;
				break ;
			}

			// DirectDrawCreateファンクションがあるか調べる
			if( ( DirectDrawCreateFunc = ( DIRECTDRAWCREATEFUNC )GetProcAddress( DDrawDLL, "DirectDrawCreate" ) ) == NULL )
			{
				FreeLibrary( DDrawDLL );
				DXST_ERRORLOG_ADD( _T( "DirectDrawCreate ファンクションが見つかりませんでした\n" ) ) ;
				break ;
			}

			// 実際にDirectDrawCreateファンクションを使ってみる
//			if( FAILED( DirectDrawCreate( NULL, &DDraw, NULL ) ) )
			if( FAILED( DirectDrawCreateFunc( NULL, &DDraw, NULL ) ) )
			{
				DXST_ERRORLOG_ADD( _T( "DirectDrawCreate ファンクションが失敗しました\n" ) ) ;
				FreeLibrary( DDrawDLL );
				break ;
			}

			// ここまできてやっと DirectX1 の存在の確認完了
			WinData.DirectXVersion = DX_DIRECTXVERSION_1 ;

			// DirectDraw2 インターフェースの取得を試みる
			if( FAILED( DDraw->QueryInterface( IID_IDIRECTDRAW2, (VOID**)&DDraw2 ) ) )
			{
				DXST_ERRORLOG_ADD( _T( "DirectDraw2 インターフェイスの取得に失敗しました\n" ) ) ;
				DDraw->Release();
				FreeLibrary( DDrawDLL );
				break ;
			}

			// ここで DirectX2 の存在を確認
			DDraw2->Release();
			WinData.DirectXVersion = DX_DIRECTXVERSION_2 ;


			// DirectInput が存在するかどうかで DirectX3 の存在を確認する
			if( ( DInputDLL = LoadLibrary( _T( "DINPUT.DLL" ) ) ) == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "DirecInput.DLL のロードに失敗しました\n" ) ) ;
				DDraw->Release();
				FreeLibrary( DDrawDLL );
				break ;
			}

			// DirectInputCreate ファンクションが存在するか調べる
			DirectInputCreateFunc = GetProcAddress( DInputDLL, "DirectInputCreateA" ) ;
			if( DirectInputCreateFunc == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "DirecInputCreateA が見つかりませんでした\n" ) ) ;
				FreeLibrary( DInputDLL );
				DDraw->Release();
				FreeLibrary( DDrawDLL );
				break ;
			}

			// ここで初めてDirectX3の存在を確認
			WinData.DirectXVersion = DX_DIRECTXVERSION_3 ;
			FreeLibrary( DInputDLL );


			// DirectX5 は DirectDrawSurface3 を扱えるかどうかで確認
			{
				// 協調レベルをセット
				if( FAILED( DDraw->SetCooperativeLevel( NULL, D_DDSCL_NORMAL ) ) ) 
				{
					DXST_ERRORLOG_ADD( _T( "協調設定を変更できませんでした\n" ) ) ;
					DDraw->Release();
					FreeLibrary( DDrawDLL );
					break ;
				}

				// DirectDrawSurface1 のプライマリーサーフェスの作成
				_MEMSET( &ddsd, 0, sizeof(ddsd) ) ;
				ddsd.dwSize = sizeof(ddsd) ;
				ddsd.dwFlags = D_DDSD_CAPS ;
				ddsd.ddsCaps.dwCaps = D_DDSCAPS_PRIMARYSURFACE;
				if( FAILED( DDraw->CreateSurface( &ddsd, &DDrawSurf, NULL ) ) )
				{
					DXST_ERRORLOG_ADD( _T( "DirectDrawSurface オブジェクトの作成に失敗しました\n" ) ) ;
					DDraw->Release();
					FreeLibrary( DDrawDLL );
					break ;
				}

				// DirectDrawSurface3 の取得を試みる
				if( FAILED( DDrawSurf->QueryInterface( IID_IDIRECTDRAWSURFACE3, ( VOID ** )&DDrawSurf3 ) ) )
				{
					DXST_ERRORLOG_ADD( _T( "DirectDrawSurface3 インターフェイスの取得に失敗しました\n" ) ) ;
					DDrawSurf->Release();
					DDraw->Release();
					FreeLibrary( DDrawDLL );
					break ;
				}
			}

			// ここまできて DirectX5 の存在を確認
			DDrawSurf3->Release();
			WinData.DirectXVersion = DX_DIRECTXVERSION_5 ;


			// DirectX6 は DirectDrawSurface4 を扱えるかどうかで確認
			if( FAILED( DDrawSurf->QueryInterface( IID_IDIRECTDRAWSURFACE4, ( VOID** )&DDrawSurf4 ) ) )
			{
				DXST_ERRORLOG_ADD( _T( "DirectDrawSurface4 インターフェイスの取得に失敗しました\n" ) ) ;
				DDrawSurf->Release();
				DDraw->Release();
				FreeLibrary( DDrawDLL );
				break ;
			}

			DDrawSurf4->Release();
			DDrawSurf->Release();
			DDraw->Release();

			// DirectX6 の存在を確認
			WinData.DirectXVersion = DX_DIRECTXVERSION_6 ;


			// DirectX6.1 は DirectMusic が存在しているかどうかで判断
			{
				WinAPIData.Win32Func.CoInitializeExFunc( NULL, COINIT_APARTMENTTHREADED );
				if( FAILED( WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTMUSIC, NULL, CLSCTX_INPROC_SERVER,
									   IID_IDIRECTMUSIC, ( VOID** )&DMusic ) ) )
				{
					DXST_ERRORLOG_ADD( _T( "DirectMusic インターフェイスの取得に失敗しました\n" ) ) ;
					FreeLibrary( DDrawDLL );
					break ;
				}

				DMusic->Release();
				WinAPIData.Win32Func.CoUninitializeFunc();
			}

			// DirectX6.1 の存在を確認
			WinData.DirectXVersion = DX_DIRECTXVERSION_6_1 ;


			// DirectX7 は DirectDraw7 が存在するかどうかで判断
			{
				// まず DirectDrawCreateEx があるか調べる
				if( ( DirectDrawCreateExFunc = ( DIRECTDRAWCREATEEXFUNC )GetProcAddress( DDrawDLL, "DirectDrawCreateEx" ) ) == NULL )
				{
					DXST_ERRORLOG_ADD( _T( "DirectDrawCreateEx ファンクションが見つかりませんでした\n" ) ) ;
					FreeLibrary( DDrawDLL );
					break ;
				}

				// あった場合はそれを使って DirectDraw7 の作成を試みる
//				if( FAILED( DirectDrawCreateEx( NULL, (VOID**)&DDraw7, IID_IDirectDraw7, NULL ) ) )
				if( FAILED( DirectDrawCreateExFunc( NULL, (VOID**)&DDraw7, IID_IDIRECTDRAW7, NULL ) ) )
				{
					DXST_ERRORLOG_ADD( _T( "DirectDrawCreateEx ファンクションが失敗しました\n" ) ) ;
					FreeLibrary( DDrawDLL );
					break ;
				}
				DDraw7->Release();
			}
			FreeLibrary( DDrawDLL );


			// ここまでこれたら Direct7 の存在の確認完了
			WinData.DirectXVersion = DX_DIRECTXVERSION_7 ;


			// DirectX8 は Direct3D8 のＤＬＬがあるかどうかで判断
			if( ( D3D8DLL = LoadLibrary( _T( "D3D8.DLL" ) ) ) == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "D33D8.DLL はありませんでした\n" ) ) ;
				FreeLibrary( DDrawDLL );
				break ;
			}
			FreeLibrary( D3D8DLL );

			// あったら DirectX8 が扱える
			WinData.DirectXVersion = DX_DIRECTXVERSION_8 ;

/*
			// ネットワークにアクセスする可能性があるためコメントアウト

			// DirectX8.1 は dpnhpast.dll が存在するかどうかで判断が可能
			if( ( DPNHPASTDLL = LoadLibrary( "dpnhpast.dll" ) ) == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "dpnhpast.DLL はありませんでした\n" ) ) ;
				FreeLibrary( DPNHPASTDLL );
				break ;
			}
			FreeLibrary( DPNHPASTDLL );

			// あったら DirectX8.1 が扱える
			WinData.DirectXVersion = DX_DIRECTXVERSION_8_1 ;
*/
			// これにて検査終了
			break ;
		}

		// バージョン出力
		{
			TCHAR Str[256], Str2[10] ;

//			lstrcpy( Str, _T( "ＤｉｒｅｃｔＸ　Ver" ) ) ;
			Str[0] = _T( '\0' ) ;
			_TSPRINTF( _DXWTP( Str2 ), _DXWTR( "%d" ), ( WinData.DirectXVersion & 0xff0000 ) >> 16 ) ;
			lstrcat( Str, Str2 ) ;
			Str2[0] = _T( '.' ) ;
			_TSPRINTF( _DXWTP( &Str2[1] ), _DXWTR( "%d" ), ( WinData.DirectXVersion & 0xff00 ) >> 8 ) ;
			lstrcat( Str, Str2 ) ;
/*			if( ( WinData.DirectXVersion & 0xff ) >= _T( 'a' ) && ( WinData.DirectXVersion & 0xff ) <= _T( 'z' ) )
			{
				unsigned char let[2] ;
				
				let[0] = WinData.DirectXVersion & 0xff ;
				let[1] = 0 ;
				_STRCAT( Str, ( char * )let ) ;
			}
*/
			// DirectX のバージョンが 8.1 だとされている場合、それ以上のバージョンを
			// チェックしていないので、それ以上のバージョンの可能性がある、なので
			// バージョンが 8.1 だった場合は『以上』と付け加えておく
			if( WinData.DirectXVersion == DX_DIRECTXVERSION_8 )
			{
				lstrcat( Str, _T( "以上" ) ) ;
			}

			DXST_ERRORLOGFMT_ADD(( _T( "ＤｉｒｅｃｔＸ　Ver%s" ), Str )) ;
			lstrcpy( WinData.PcInfo.DirectXString, Str ) ;
//			DXST_ERRORLOG_ADD( _T( Str ) ;
//			DXST_ERRORLOG_ADD( _T( "\n" ) ) ;
		}
	}

	// タイマの精度を設定する
	{
		TIMECAPS tc ;
		WinAPIData.Win32Func.timeGetDevCapsFunc( &tc , sizeof(TIMECAPS) );

		// マルチメディアタイマーのサービス精度を最大に 
		WinAPIData.Win32Func.timeBeginPeriodFunc( tc.wPeriodMin ) ;
	}

	// CPU のチェック
	{
#ifndef DX_NON_INLINE_ASM
		int CPUCODE ;
#endif
		int ENDMODE = 0, RDTSCUse = 0 ;
		int Str1 = 0,Str2 = 0,Str3 = 0 ;
		char String[5], CpuName[4*4*3+1] ;

		CpuName[0] = '\0' ;
		CpuName[48] = '\0' ;
		WinData.UseMMXFlag = FALSE ;
#ifndef DX_NON_INLINE_ASM
		__asm{
			// CPUID が使えるか検査
			PUSHFD
			PUSHFD
			POP		EAX
			XOR		EAX , 00200000H
			PUSH	EAX
			POPFD
			PUSHFD
			POP		EBX
			MOV		ENDMODE , 0 
			CMP		EAX , EBX
			JNZ		MMXEND
			MOV		EAX , 0
		}
//			db 		0fh
//			db		0a2h
        cpuid
		__asm{
			CMP		EAX , 0

			// 使えるのでＣＰＵベンダ名でもロード
			MOV		CPUCODE , EAX
			MOV		Str1, EBX
			MOV		Str2, EDX
			MOV		Str3, ECX
			MOV		ENDMODE , 1

			// ついでにパフォーマンスカウンタがつかえるかチェック
			MOV		EAX , 1
		}
//			db 0fh
//			db 0a2h
        cpuid
		__asm{
			AND		EDX , 16
			SHR		EDX , 4
			MOV		RDTSCUse , EDX

			// 今度は拡張ＣＰＵＩＤが使えるか検査
			MOV		EAX , 80000000H
//			db 0fh
//			db 0a2h
		}
        cpuid
		__asm{
			CMP		EAX , 80000004H
			JB		CHECKMMX

			// 使えるのでＣＰＵ名を取得
			MOV		EAX , 80000002H
		}
//			db 0fh
//			db 0a2h
        cpuid
		__asm{
			MOV		DWORD PTR [CpuName + 0] , EAX
			MOV		DWORD PTR [CpuName + 4] , EBX
			MOV		DWORD PTR [CpuName + 8] , ECX
			MOV		DWORD PTR [CpuName + 12] , EDX
			MOV		EAX , 80000003H
		}
//			db 0fh
//			db 0a2h
        cpuid
		__asm{
			MOV		DWORD PTR [CpuName + 16] , EAX
			MOV		DWORD PTR [CpuName + 20] , EBX
			MOV		DWORD PTR [CpuName + 24] , ECX
			MOV		DWORD PTR [CpuName + 28] , EDX
			MOV		EAX , 80000004H
		}
//			db 0fh
//			db 0a2h
        cpuid
		__asm{
			MOV		DWORD PTR [CpuName + 32] , EAX
			MOV		DWORD PTR [CpuName + 36] , EBX
			MOV		DWORD PTR [CpuName + 40] , ECX
			MOV		DWORD PTR [CpuName + 44] , EDX

CHECKMMX:
			// 今度はＭＭＸが使えるか検査
			MOV		EAX , 1
		}
//			db 0fh
//			db 0a2h
        cpuid
		__asm{
			AND		EDX , 00800000H
			JZ		MMXEND
			MOV		ENDMODE , 3
			//SSEが使えるか検査
			MOV		EAX , 1
		}
		cpuid
		__asm{
			AND		EDX	, 02000000H
			JZ		MMXEND
			ADD		ENDMODE	, 1
			//SSE2が使えるか検査
			MOV		EAX	, 1
		}
		cpuid
		__asm{
			AND		EDX	, 04000000H
			JZ		MMXEND
			ADD		ENDMODE	, 1
MMXEND:
			POPFD
		}
#endif  // DX_NON_INLINE_ASM
			
		// パフォーマンスカウンタフラグを保存
		WinData.UseRDTSCFlag = RDTSCUse ;

#ifndef DX_NON_INLINE_ASM
		// パフォーマンスカウンタがつかえる場合はクロック数を簡単に計測
		if( RDTSCUse )
		{
			LARGE_INTEGER Clock1, Clock2 ;
			int Time ;
			DWORD Clock ;
#ifdef __BCC
			unsigned int low2, high2 ;
			unsigned int low1, high1 ;

			__asm
			{
				db 0fh
				db 031h
				MOV		low1, EAX
				MOV		high1, EDX
			}

			Time = NS_GetNowCount(FALSE) ;
			while( NS_GetNowCount(FALSE) - Time < 100 ){}

			__asm
			{
				db 0fh
				db 031h
				MOV		low2, EAX
				MOV		high2, EDX
			}

			Clock1.LowPart = low1 ; Clock1.HighPart = high1 ;
			Clock2.LowPart = low2 ; Clock2.HighPart = high2 ;
#else
			__asm
			{
				RDTSC
				MOV		Clock1.LowPart, EAX
				MOV		Clock1.HighPart, EDX
			}

			Time = NS_GetNowCount(FALSE) ;
			while( NS_GetNowCount(FALSE) - Time < 100 ){}

			__asm
			{
				RDTSC
				MOV		Clock2.LowPart, EAX
				MOV		Clock2.HighPart, EDX
			}
#endif

			Clock = _DTOL( (double)( Clock2.QuadPart - Clock1.QuadPart ) / 100000 ) ; 
			WinData.OneSecCount = ( Clock2.QuadPart - Clock1.QuadPart ) * 10 ;

			DXST_ERRORLOGFMT_ADD(( _T( "ＣＰＵ動作速度：大体%.2fGHz" ), (float)Clock / 1000.0f )) ;
			WinData.PcInfo.CPUSpeed = Clock ;
		}
#endif // DX_NON_INLINE_ASM

		switch( ENDMODE )
		{
		case 0 :
#ifndef DX_NON_INLINE_ASM
			DXST_ERRORLOG_ADD( _T( "ＣＰＵＩＤ命令は使えません\n" ) ) ;
#endif // DX_NON_INLINE_ASM
			break ;

		case 1 :
			DXST_ERRORLOG_ADD( _T( "ＭＭＸは使えません\n" ) ) ;
			break ;

		case 3 :
			DXST_ERRORLOG_ADD( _T( "ＭＭＸ命令を使用します\n" ) ) ;
			WinData.UseMMXFlag = TRUE ;
			break ;

		case 4 :
			DXST_ERRORLOG_ADD( _T( "ＭＭＸ命令を使用します\n" ) ) ;
			DXST_ERRORLOG_ADD( _T( "ＳＳＥ命令が使用可能です\n" ) ) ;
			WinData.UseMMXFlag = TRUE ;
			WinData.UseSSEFlag = TRUE ;
			break ;

		case 5 :
			DXST_ERRORLOG_ADD( _T( "ＭＭＸ命令を使用します\n" ) ) ;
			DXST_ERRORLOG_ADD( _T( "ＳＳＥ命令が使用可能です\n" ) ) ;
			DXST_ERRORLOG_ADD( _T( "ＳＳＥ２命令が使用可能です\n" ) ) ;
			WinData.UseMMXFlag = TRUE ;
			WinData.UseSSEFlag = TRUE ;
			WinData.UseSSE2Flag = TRUE ;
			break ;
		}

		if( ENDMODE > 0 )
		{
			DXST_ERRORLOG_ADD( _T( "ＣＰＵベンダ：" ) ) ;

			String[4] = 0 ;
			*( ( int * )&String[0] ) = Str1 ;
			DXST_ERRORLOG_ADDA( String ) ;

			*( ( int * )&String[0] ) = Str2 ;
			DXST_ERRORLOG_ADDA( String ) ;

			*( ( int * )&String[0] ) = Str3 ;
			DXST_ERRORLOG_ADDA( String ) ;

			DXST_ERRORLOG_ADD( _T( "\n" ) ) ;

			if( CpuName[0] != '\0' )
			{
				DXST_ERRORLOGFMT_ADDA(( "ＣＰＵ名：%s", CpuName )) ;
#ifdef UNICODE
				MBCharToWChar( CP_ACP, CpuName, ( DXWCHAR * )WinData.PcInfo.CPUString, 64 ) ;
#else
				lstrcpy( WinData.PcInfo.CPUString, CpuName ) ;
#endif
			}
		}
	}

	DXST_ERRORLOG_TABSUB ;

	// もし DirectX のバージョンが７以下だった場合はここでソフトを終了
	if( WinData.DirectXVersion < DX_DIRECTXVERSION_7 )
	{
#ifndef DX_NON_LITERAL_STRING
		if( WinData.DirectXVersion == DX_DIRECTXVERSION_NON )
		{
			MessageBox( NULL, _T( "DirectX がインストールされていないのでここで終了します" ), _T( "エラー" ), MB_OK ) ;
		}
		else
		{
			MessageBox( NULL, _T( "DirectX バージョン 7 以前なのでソフトを起動することが出来ません" ), _T( "エラー" ), MB_OK ) ;
		}
#endif
		ExitProcess( (DWORD)-1 ) ;
	}

	// 終了
	return 0 ;
}






















// メッセージ処理関数
int WM_SIZEProcess( void )
{
	RECT rect, ClientRect ;
	int Width, Height ;
	int CWidth, CHeight ;
	double ExRateX, ExRateY ;

	// ユーザーのウインドウを使用していない場合のみサイズの補正を行う
	if( WinData.UserWindowFlag == TRUE ) return 0 ;

	NS_GetDrawScreenSize( &Width, &Height ) ;
	if( WinData.WindowModeFlag == FALSE && ( GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) )
	{
		Width = 640 ;
		Height = 480 ;
	}

	// ウインドウへの出力サイズを反映させる
	NS_GetWindowSizeExtendRate( &ExRateX, &ExRateY ) ;
	Width  = _DTOL( Width  * ExRateX ) ;
	Height = _DTOL( Height * ExRateY ) ;

	GetWindowRect( WinData.MainWindow, &rect ) ;
	if( GetClientRect( WinData.MainWindow, &ClientRect ) == 0 ) return 0 ;
	WinData.WindowRect.right  = WinData.WindowRect.left + ClientRect.right ;
	WinData.WindowRect.bottom = WinData.WindowRect.top  + ClientRect.bottom ;

	// クライアント領域のサイズを得る
	CWidth = ClientRect.right  - ClientRect.left ;
	CHeight = ClientRect.bottom - ClientRect.top ;

	// クライアント領域外の部分の幅・高さを得る
	rect.right  -= CWidth ;
	rect.bottom -= CHeight ;

	// ウインドウに画面をフィットさせるかどうかで処理を分岐
	if( WinData.ScreenNotFitWindowSize == TRUE || WinData.WindowSizeValid == TRUE )
	{
		// ウインドウサイズ設定が指定されていたらそれを使用する
		if( WinData.WindowSizeValid == TRUE )
		{
			CWidth  = WinData.WindowWidth ;
			CHeight = WinData.WindowHeight ;

			if( WinData.WindowSizeChangeEnable == TRUE || WinData.ScreenNotFitWindowSize == TRUE )
			{
				if( DxSysData.DxLib_RunInitializeFlag == FALSE )
				{
					WinData.WindowSizeValid = FALSE ;
				}
				else
				{
					WinData.WindowSizeValidResetRequest = TRUE ;
				}
			}
		}

		// クライアント領域が画面領域より大きくなっている場合は補正する
		if( CWidth  > Width  ) CWidth  = Width  ;
		if( CHeight > Height ) CHeight = Height ;
	}
	else
	{
		// クライアント領域が画面領域と異なっている場合は補正する
		if( CWidth  != Width  ) CWidth  = Width  ;
		if( CHeight != Height ) CHeight = Height ;
	}
	rect.right  += CWidth ;
	rect.bottom += CHeight + GetToolBarHeight() ;

	// ウインドウの位置指定がある場合は反映
	if( WinData.WindowPosValid == TRUE )
	{
		int SX, SY;

		SX = WinData.WindowX - rect.left ;
		SY = WinData.WindowY - rect.top ;
		rect.left   += SX ;
		rect.top    += SY ;
		rect.right  += SX ;
		rect.bottom += SY ;

		if( DxSysData.DxLib_RunInitializeFlag == FALSE && GRA2.ChangeGraphModeFlag == FALSE )
		{
			WinData.WindowPosValid = FALSE ;
		}
		if( WinData.BackBufferTransColorFlag == TRUE || WinData.UseUpdateLayerdWindowFlag == TRUE )
		{
			int SW, SH ;

			NS_GetDrawScreenSize( &SW, &SH ) ;
			rect.right  = rect.left + SW ;
			rect.bottom = rect.top  + SH ;
			if( WStyle_WindowModeTable[ WinData.WindowStyle ] & WS_CAPTION )
			{
				rect.bottom += WinData.SystembarHeight ;
			}
		}
	}

	MoveWindow( WinData.MainWindow, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, TRUE ) ;
	if( WinData.WindowModeFlag == TRUE && WinData.VisibleFlag == FALSE )
	{
		WinData.VisibleFlag = TRUE ;
		SetWindowStyle() ;
	}

	// 終了
	return 0 ;
}

int WM_MOVEProcess( LPARAM lParam )
{
	RECT rect ;
	SIZE WindowSize ;

	// ユーザーのウインドウを使用している場合のみ移動を行う
	if( WinData.UserWindowFlag == TRUE ) return 0 ;

	// ウインドウの位置を保存
	if( GetClientRect( WinData.MainWindow , &rect ) == 0 ) return 0 ;

	WindowSize.cx = rect.right - rect.left ;
	WindowSize.cy = rect.bottom - rect.top ;

	WinData.WindowRect.left = ( LONG )( short )( lParam & 0xffff ) ;
	WinData.WindowRect.top  = ( LONG )( short )( ( lParam >> 16 ) & 0xffff ) ;

	WinData.WindowRect.right = WinData.WindowRect.left + WindowSize.cx ;
	WinData.WindowRect.bottom = WinData.WindowRect.top + WindowSize.cy ;

	if( NS_GetActiveGraph() == DX_SCREEN_FRONT )
	{
#ifndef DX_NON_MASK
		int Flag ;
#endif

		// マスクを使用終了する手続きを取る
#ifndef DX_NON_MASK
		Flag = NS_GetUseMaskScreenFlag() ;
		NS_SetUseMaskScreenFlag( FALSE ) ;
#endif

		// マスクを使用する手続きを取る
#ifndef DX_NON_MASK
		NS_SetUseMaskScreenFlag( Flag ) ;
#endif
	}

	// 終了
	return 0 ;
}

int WM_ACTIVATEProcessUseStock( WPARAM wParam, int APPMes )
{
	// ProcessMessage からここに来た場合( 且つまだリクエストがされていなく、ウインドウ作成直後でもない場合 )は
	// ProcessMessage の最後に WM_ACTIVATE を行うようにする
	if( WinData.ProcessMessageFlag == TRUE &&
		WinData.WindowCreateFlag == FALSE &&
		WinData.WM_ACTIVATE_StockNum < 512 )
	{
		WinData.WM_ACTIVATE_wParam[ WinData.WM_ACTIVATE_EndIndex ] = wParam ;
		WinData.WM_ACTIVATE_APPMes[ WinData.WM_ACTIVATE_EndIndex ] = APPMes ;
		WinData.WM_ACTIVATE_EndIndex = ( WinData.WM_ACTIVATE_EndIndex + 1 ) % 512 ;
		WinData.WM_ACTIVATE_StockNum ++ ;
	}
	else
	{
		WM_ACTIVATEProcess( wParam, APPMes ) ;
	}

	return 0 ;
}

int WM_ACTIVATEProcess( WPARAM wParam, int APPMes )
{
	int ActiveFlag ;

	// アクティブかどうかを取得
	ActiveFlag = LOWORD( wParam ) != 0 ;

	// ウインドウ作成直後だった場合は何もせず終了
	if( WinData.WindowCreateFlag == TRUE )
	{
		// アクティブかどうかを保存
		WinData.ActiveFlag = ActiveFlag ;
		return 0 ;
	}

	// 最小化された状態のアクティブ情報(wParam の HIWORD が 0 以外)は非アクティブとして処理をする
	if( HIWORD( wParam ) != 0 )
	{
		ActiveFlag = FALSE ;
	}

	// アクティブ情報が今までと同じ場合は何もしない
	if( WinData.ActiveFlag == ActiveFlag )
		return 0 ;

	// アクティブかどうかを保存
	WinData.ActiveFlag = ActiveFlag ;

	if( ActiveFlag == TRUE )
	{
//		DXST_ERRORLOG_ADD( _T( "アクティブになりました\n" ) ) ;

#ifndef DX_NON_INPUTSTRING
		// ＩＭＥで文字列を入力中だった場合は文字列を確定してしまう
		if( CharBuf.IMEUseFlag_OSSet == TRUE && CharBuf.IMESwitch == TRUE )
		{
			CharBuf.IMERefreshStep = 1;
		}
#endif // DX_NON_INPUTSTRING

		if( WinData.WindowModeFlag == TRUE )
		{
//			if( NS_ScreenFlip() != 0 )
			if( GraphicsDevice_IsValid() != 0 )
			{
				EndScene() ;
				if( GraphicsDevice_IsLost() != 0 )
				{
					NS_RestoreGraphSystem() ;
				}
			}
		}

		if( WinData.WindowModeFlag == FALSE && /*WinData.StopFlag == TRUE &&*/ DxSysData.DxLib_InitializeFlag == TRUE )
		{
			// グラフィックシステムの復帰処理
			NS_RestoreGraphSystem() ;

			WinData.StopFlag = FALSE ;

#ifndef DX_NON_MOVIE
			// ムービーグラフィックの再生状態を戻す
			PlayMovieAll() ;
#endif

#ifndef DX_NON_SOUND
			// サウンドの再生を再開する
			PauseSoundMemAll( FALSE ) ;
			PauseSoftSoundAll( FALSE ) ;
#endif // DX_NON_SOUND
		}
		else
		{
			if( WinData.NonActiveRunFlag == FALSE )
			{
#ifndef DX_NON_MOVIE
				// ムービーグラフィックの再生状態を戻す
				PlayMovieAll() ;
#endif

#ifndef DX_NON_SOUND
				// サウンドの再生を再開する
				PauseSoundMemAll( FALSE ) ;
				PauseSoftSoundAll( FALSE ) ;
#endif // DX_NON_SOUND
			}
		}

		// DirectInputデバイスオブジェクトの再取得
#ifndef DX_NON_INPUT
		RefreshInputDeviceAcquireState() ;
#endif // DX_NON_INPUT
	}
	else
	{
//		DXST_ERRORLOG_ADD( _T( "非アクティブになりました\n" ) ) ;

		if( WinData.WindowModeFlag == FALSE  )
		{
			// DirectX のオブジェクトを解放する
			ReleaseDirectXObject() ;

			if( WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA || GRA2.ValidHardWare == FALSE )
			{
				ChangeDisplaySettings( NULL, 0 ) ;
				ShowWindow( WinData.MainWindow, SW_MINIMIZE );
			}
		}

		// フルスクリーンモードの場合の処理
		if( WinData.WindowModeFlag == FALSE && NS_GetChangeDisplayFlag() && NS_GetUseDDrawObj() != NULL )
		{
			// フルスクリーンの場合は必ず DWM を有効にする
			SetEnableAero( TRUE ) ;

			// もし復元スレッドが登録されていない場合はソフト終了
			if( NS_GetValidRestoreShredPoint() == FALSE ) 
			{
				// ユーザー提供のウインドウだったら何もしない
				if( WinData.UserWindowFlag == FALSE )
				{
					DXST_ERRORLOG_ADD( _T( "復元関数が登録されていないため終了します\n" ) ) ;
				
					// クローズフラグが倒れていたらWM_CLOSEメッセージを送る
					if( WinData.CloseMessagePostFlag == FALSE )
					{
						WinData.CloseMessagePostFlag = TRUE ;
						PostMessage( WinData.MainWindow, WM_CLOSE, 0, 0 );
					}

					return -1 ;
				}
			}

#ifndef DX_NON_MOVIE
			// ムービーグラフィックの再生状態をとめる
			PauseMovieAll() ;
#endif

#ifndef DX_NON_SOUND
			// サウンドの再生を止める
			PauseSoundMemAll( TRUE ) ;
			PauseSoftSoundAll( TRUE ) ;
#endif // DX_NON_SOUND

			WinData.StopFlag = TRUE ;
		}
		else
		{
			if( WinData.NonActiveRunFlag == FALSE )
			{
#ifndef DX_NON_MOVIE
				// ムービーグラフィックの再生状態をとめる
				PauseMovieAll() ;
#endif

#ifndef DX_NON_SOUND
				// サウンドの再生を止める
				PauseSoundMemAll( TRUE ) ;
				PauseSoftSoundAll( TRUE ) ;
#endif // DX_NON_SOUND
			}

			// DirectInputデバイスオブジェクトの再取得
#ifndef DX_NON_INPUT
			RefreshInputDeviceAcquireState() ;
#endif // DX_NON_INPUT
		}

		if( WinData.NotMoveMousePointerOutClientAreaFlag )
		{
			ClipCursor( NULL ) ;
			WinData.SetClipCursorFlag = FALSE ;
		}

		// バックグラウンド描画
		{
			HDC hdc ;

			hdc = GetDC( GetDisplayWindowHandle() ) ;
			if( hdc != NULL )
			{
				DrawBackGraph( hdc ) ;
				ReleaseDC( GetDisplayWindowHandle() , hdc ) ;
			}
		}
	}

	// パッドのバイブレーション状態を更新
#ifndef DX_NON_INPUT
	RefreshEffectPlayState() ;
#endif // DX_NON_INPUT

	// アクティブ状態が変化した時に呼ぶコールバック関数が設定されていたら呼ぶ
	if( WinData.ActiveStateChangeCallBackFunction != NULL && APPMes == FALSE )
	{
		WinData.ActiveStateChangeCallBackFunction( ActiveFlag, WinData.ActiveStateChangeCallBackFunctionData ) ;
	}

	// ソフトプロセスの実行優先順位をセットする
//	SetPriorityClass( GetCurrentProcess() , ActiveFlag ? HIGH_PRIORITY_CLASS : IDLE_PRIORITY_CLASS );

	// タスクスイッチの有効無効をセット
	if( WinData.SysCommandOffFlag == TRUE )
	{
		if( WinData.WindowsVersion < DX_WINDOWSVERSION_NT31 )
		{
			// Win95 カーネルの場合の処理
			UINT nPreviousState;
//			SystemParametersInfo( SPI_SETSCREENSAVERRUNNING, ActiveFlag, &nPreviousState, 0 ) ;
			SystemParametersInfo( SPI_SETSCREENSAVERRUNNING/*SPI_SCREENSAVERRUNNING*/, ActiveFlag, &nPreviousState, 0 ) ;
		}
		else
		{
			// WinNT カーネルの場合の処理

			if( WinData.GetMessageHookHandle == NULL && ActiveFlag == TRUE )
			{
//				WinData.MessageHookThredID = GetWindowThreadProcessId( WinData.MainWindow, NULL ) ;
//				WinData.MessageHookThredID = GetWindowThreadProcessId( GetDesktopWindow(), NULL ) ;
				WinData.MessageHookDLL = LoadLibrary( WinData.HookDLLFilePath ) ;

				// DLL が無かったら進まない
				if( WinData.MessageHookDLL != NULL )
				{
					WinData.MessageHookCallBadk = ( MSGFUNC )GetProcAddress( WinData.MessageHookDLL, "SetMSGHookDll" ) ;
					if( WinData.MessageHookCallBadk != NULL )
					{
						WinData.MessageHookCallBadk( WinData.MainWindow, &WinData.KeyboardHookHandle ) ;
//						WinData.GetMessageHookHandle = SetWindowsHookEx( WH_GETMESSAGE, WinData.MessageHookCallBadk, WinData.MessageHookDLL, WinData.MessageHookThredID ) ;
						WinData.GetMessageHookHandle = SetWindowsHookEx( WH_GETMESSAGE, MsgHook, WinData.Instance, 0 ) ;
					}
				}
			}
			else
			if( WinData.GetMessageHookHandle != NULL && ActiveFlag == FALSE )
			{
				// メッセージフックを無効に
				UnhookWindowsHookEx( WinData.GetMessageHookHandle ) ;
				UnhookWindowsHookEx( WinData.KeyboardHookHandle ) ;
				FreeLibrary( WinData.MessageHookDLL ) ;
				WinData.GetMessageHookHandle = NULL ;
				WinData.KeyboardHookHandle = NULL ;
			}
		}

		// 共通
		WinData.LockInitializeFlag = TRUE ;
	}

	// 正常終了
	return 0 ;
}



// SPI_GETFOREGROUNDLOCKTIMEOUT の定義
#if !defined(SPI_GETFOREGROUNDLOCKTIMEOUT) 
#define SPI_GETFOREGROUNDLOCKTIMEOUT 0x2000 
#define SPI_SETFOREGROUNDLOCKTIMEOUT 0x2001 
#endif 

// ソフトのウインドウにフォーカスを移す
extern void SetAbsoluteForegroundWindow( HWND hWnd, int Flag )
{
    int nTargetID, nForegroundID;
    DWORD sp_time;

    // フォアグラウンドウィンドウを作成したスレッドのIDを取得
    nForegroundID = GetWindowThreadProcessId(GetForegroundWindow(), NULL);

    // 目的のウィンドウを作成したスレッドのIDを取得
    nTargetID = GetWindowThreadProcessId(hWnd, NULL );


    // スレッドのインプット状態を結び付ける
    AttachThreadInput(nTargetID, nForegroundID, TRUE );  // TRUE で結び付け


    // 現在の設定を sp_time に保存
    SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);

    // ウィンドウの切り替え時間を 0ms にする
    SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,(LPVOID)0,0);


    // ウィンドウをフォアグラウンドに持ってくる
    SetForegroundWindow(hWnd);


    // 設定を元に戻す
    SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT,0,&sp_time,0);


    // スレッドのインプット状態を切り離す
    AttachThreadInput(nTargetID, nForegroundID, FALSE );  // FALSE で切り離し

	// ウインドウを最前面に持ってくる
	if( Flag == TRUE ) SetWindowPos( WinData.MainWindow , HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW ) ;
}



// ＢＭＰリソースから BITMAPINFO と画像イメージを構築する
extern int GetBmpImageToResource( int ResourceID, BITMAPINFO **BmpInfoP, void **GraphDataP )
{
	BITMAPINFO *BmpInfo = NULL ;
	void *GraphData = NULL ;
	HRSRC RSrc ;
	void *DataBuf ;
	BYTE *DataP ;
	HGLOBAL Global ;
	int Er = FALSE ;

	// リソースを取得
	RSrc = FindResource( WinData.LoadResourModule == NULL ? GetModuleHandle( NULL ) : WinData.LoadResourModule, MAKEINTRESOURCE( ResourceID ), RT_BITMAP ) ;
	if( RSrc == NULL ) return -1 ;

	// リソースが格納されているメモリ領域を取得
	Global = LoadResource( WinData.LoadResourModule == NULL ? GetModuleHandle( NULL ) : WinData.LoadResourModule, RSrc ) ;
	DataP = ( BYTE * )( DataBuf = LockResource( Global ) ) ;
	if( DataBuf == NULL ) return -1 ;

	// データを解析
	{
		BITMAPINFO			BmpInfoT ;
		int					ImageSize ;

		// BITMAPINFOを読みこむ
		memcpy( &BmpInfoT, DataP, sizeof( BITMAPINFOHEADER ) ) ;
		DataP += sizeof( BITMAPINFOHEADER ) ;

		// カラービット数が８以下の時はパレットを読む
		if( BmpInfoT.bmiHeader.biBitCount <= 8 )
		{
			// ＢＭＰＩＮＦＯ構造体の格納用メモリを確保
			if( ( BmpInfo = ( BITMAPINFO * )DXCALLOC( ( int )sizeof( BITMAPINFOHEADER ) + ( 1 << BmpInfoT.bmiHeader.biBitCount ) * ( int )sizeof( RGBQUAD ) ) ) == NULL )
			{
				Er = TRUE ; goto END1 ;
			}

			// ＢＭＰＩＮＦＯ構造体の内容をコピー
			memcpy( BmpInfo, &BmpInfoT, sizeof( BITMAPINFOHEADER ) ) ;

			// 残りのカラーパレットの読みこみ
			memcpy( ( ( BYTE * )BmpInfo ) + ( int )sizeof( BITMAPINFOHEADER ), DataP, ( 1 << BmpInfoT.bmiHeader.biBitCount ) * ( int )sizeof( RGBQUAD ) ) ;
			DataP += ( 1 << BmpInfoT.bmiHeader.biBitCount ) * ( int )sizeof( RGBQUAD ) ;
		}
		else
		// カラービット数が３２か１６でカラーマスク使用時の処理
		if( BmpInfoT.bmiHeader.biBitCount == 16 || BmpInfoT.bmiHeader.biBitCount == 32 || BmpInfoT.bmiHeader.biCompression == BI_BITFIELDS )
		{
			// ＢＭＰＩＮＦＯ構造体の格納用メモリを確保
			if( ( BmpInfo = ( BITMAPINFO * )DXCALLOC( sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * 3 ) ) == NULL )
			{
				Er = TRUE ; goto END1 ;
			}

			// ＢＭＰＩＮＦＯＨＥＡＤＥＲ構造体の内容をコピー
			memcpy( BmpInfo, &BmpInfoT, sizeof( BITMAPINFOHEADER ) ) ;

			// カラーマスクの読みこみ
			memcpy( ( ( BYTE * )BmpInfo ) + sizeof( BITMAPINFOHEADER ), DataP, sizeof( RGBQUAD ) * 3 ) ;
			DataP += sizeof( RGBQUAD ) * 3 ;
		}
		else
		// それ以外の場合の処理
		{
			// ＢＭＰＩＮＦＯ構造体の格納用メモリを確保
			if( ( BmpInfo = ( BITMAPINFO * )DXCALLOC( sizeof( BITMAPINFO ) ) ) == NULL )
			{
				Er = TRUE ; goto END1 ;
			}

			// ＢＭＰＩＮＦＯＨＥＡＤＥＲ構造体の内容をコピー
			memcpy( BmpInfo, &BmpInfoT, sizeof( BITMAPINFOHEADER ) ) ;
		}

		// イメージサイズを計算する
		{
			int Byte ;

			Byte = BmpInfoT.bmiHeader.biWidth * BmpInfoT.bmiHeader.biBitCount / 8 ;
			Byte += Byte % 4 != 0 ? ( 4 - Byte % 4 ) : 0 ;

			ImageSize = Byte * _ABS( BmpInfoT.bmiHeader.biHeight ) ;
		}

		// グラフィックデータ領域を確保
		if( ( GraphData = DXALLOC( ImageSize ) ) == NULL )
		{
			DXFREE( BmpInfo ) ; BmpInfo = NULL ;
			goto END1 ;
		}

		// グラフィックデータの読みこみ
		memcpy( GraphData, DataP, ImageSize ) ;
		DataP += ImageSize ;
	}

END1 :
	// リソース解放
	UnlockResource( Global ) ;

	if( Er == TRUE ) return -1 ;

	*BmpInfoP = BmpInfo ;
	*GraphDataP = GraphData ;

	// 成功
	return 0 ;
}





}










