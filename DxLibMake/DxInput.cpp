// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ＤｉｒｅｃｔＩｎｐｕｔ制御プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

#include "DxInput.h"

#ifndef DX_NON_INPUT

// Include ------------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxSystem.h"
#include "DxLog.h"
#include "Windows/DxWindow.h"
#include "Windows/DxWinAPI.h"
#include "Windows/DxGuid.h"

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

// ＤｉｒｅｃｔＸライブラリ処理ＯＫ時の識別子
#define DXLIB_OK  D_DI_OK

#define DEADZONE							(DWORD)( 0.35 * 65536 )
#define DEADZONE_DIRECTINPUT( ZONE )		(DWORD)( 10000 * (ZONE) / 65536)
#define DEADZONE_WINMM( ZONE )				(DWORD)(0x8000 * (ZONE) / 65536)
#define DEADZONE_XINPUT( ZONE )				(SHORT)( 32767 * (ZONE) / 65536)
#define DEADZONE_XINPUT_TRIGGER( ZONE )		(SHORT)(   255 * (ZONE) / 65536)
#define RANGE								(1000)
#define VALIDRANGE_WINMM( ZONE )			(0x8000 - DEADZONE_WINMM(ZONE))
#define VALIDRANGE_XINPUT( ZONE )			( 32767 - DEADZONE_XINPUT(ZONE))
#define VALIDRANGE_XINPUT_TRIGGER( ZONE )	(   255 - DEADZONE_XINPUT_TRIGGER(ZONE))

#ifndef VK_OEM_PLUS
#define VK_OEM_PLUS			(0xBB)
#endif

#ifndef VK_OEM_COMMA
#define VK_OEM_COMMA		(0xBC)
#endif

#ifndef VK_OEM_MINUS
#define VK_OEM_MINUS		(0xBD)
#endif

#ifndef VK_OEM_PERIOD
#define VK_OEM_PERIOD		(0xBE)
#endif

#ifndef VK_OEM_1
#define VK_OEM_1			(0xBA)
#endif

#ifndef VK_OEM_2
#define VK_OEM_2			(0xBF)
#endif

#ifndef VK_OEM_3
#define VK_OEM_3			(0xC0)
#endif

#ifndef VK_OEM_4
#define VK_OEM_4			(0xDB)
#endif

#ifndef VK_OEM_5
#define VK_OEM_5			(0xDC)
#endif

#ifndef VK_OEM_6
#define VK_OEM_6			(0xDD)
#endif

#ifndef VK_OEM_7
#define VK_OEM_7			(0xDE)
#endif

#ifndef VK_OEM_102
#define VK_OEM_102			(0xE2)
#endif

#ifndef WHEEL_DELTA
#define WHEEL_DELTA			(120)
#endif

const static unsigned short __KeyMap[][3 /* 0:Windows仮想キーコード  1:DirectInputキーコード  2:DirectInput使用時に使用するか否か */ ] =
{
	'A',			D_DIK_A,			0,	// Ａキー
	'B',			D_DIK_B,			0,	// Ｂキー
	'C',			D_DIK_C,			0,	// Ｃキー
	'D',			D_DIK_D,			0,	// Ｄキー
	'E',			D_DIK_E,			0,	// Ｅキー
	'F',			D_DIK_F,			0,	// Ｆキー
	'G',			D_DIK_G,			0,	// Ｇキー
	'H',			D_DIK_H,			0,	// Ｈキー
	'I',			D_DIK_I,			0,	// Ｉキー
	'J',			D_DIK_J,			0,	// Ｊキー
	'K',			D_DIK_K,			0,	// Ｋキー
	'L',			D_DIK_L,			0,	// Ｌキー
	'M',			D_DIK_M,			0,	// Ｍキー
	'N',			D_DIK_N,			0,	// Ｎキー
	'O',			D_DIK_O,			0,	// Ｏキー
	'P',			D_DIK_P,			0,	// Ｐキー
	'Q',			D_DIK_Q,			0,	// Ｑキー
	'R',			D_DIK_R,			0,	// Ｒキー
	'S',			D_DIK_S,			0,	// Ｓキー
	'T',			D_DIK_T,			0,	// Ｔキー
	'U',			D_DIK_U,			0,	// Ｕキー
	'V',			D_DIK_V,			0,	// Ｖキー
	'W',			D_DIK_W,			0,	// Ｗキー
	'X',			D_DIK_X,			0,	// Ｘキー
	'Y',			D_DIK_Y,			0,	// Ｙキー
	'Z',			D_DIK_Z,			0,	// Ｚキー
	'0',			D_DIK_0,			0,	// ０キー
	'1',			D_DIK_1,			0,	// １キー
	'2',			D_DIK_2,			0,	// ２キー
	'3',			D_DIK_3,			0,	// ３キー
	'4',			D_DIK_4,			0,	// ４キー
	'5',			D_DIK_5,			0,	// ５キー
	'6',			D_DIK_6,			0,	// ６キー
	'7',			D_DIK_7,			0,	// ７キー
	'8',			D_DIK_8,			0,	// ８キー
	'9',			D_DIK_9,			0,	// ９キー

	VK_LEFT,		D_DIK_LEFT,			1,	// 左キー
	VK_UP,			D_DIK_UP,			1,	// 上キー
	VK_RIGHT,		D_DIK_RIGHT,		1,	// 右キー
	VK_DOWN,		D_DIK_DOWN,			1,	// 下キー

	VK_OEM_PLUS,	D_DIK_SEMICOLON,	0,	// ；キー
	VK_OEM_1,		D_DIK_COLON,		0,	// ：キー
	VK_OEM_4,		D_DIK_LBRACKET,		0,	// ［キー
	VK_OEM_6,		D_DIK_RBRACKET,		0,	// ］キー
	VK_OEM_3,		D_DIK_AT,			0,	// ＠キー
	VK_OEM_102,		D_DIK_BACKSLASH,	0,	// ＼キー
	VK_OEM_COMMA,	D_DIK_COMMA,		0,	// ，キー
	VK_OEM_7,		D_DIK_PREVTRACK,	0,	// ＾きー

	VK_OEM_MINUS,	D_DIK_MINUS,		0,	// －キー
	VK_OEM_5,		D_DIK_YEN,			0,	// ￥キー
	VK_OEM_PERIOD,	D_DIK_PERIOD,		0,	// ．キー
	VK_OEM_2,		D_DIK_SLASH,		0,	// ／キー

	VK_DELETE,		D_DIK_DELETE,		0,	// デリートキー

	VK_ESCAPE,		D_DIK_ESCAPE,		0,	// エスケープキー
	VK_SPACE,		D_DIK_SPACE,		0,	// スペースキー

	VK_BACK, 		D_DIK_BACK,			0,	// バックスペースキー
	VK_TAB, 		D_DIK_TAB,			0,	// タブキー
	VK_RETURN,		D_DIK_RETURN,		0,	// エンターキー

	VK_NUMPAD0,		D_DIK_NUMPAD0,		1,	// テンキー０
	VK_NUMPAD1,		D_DIK_NUMPAD1,		1,	// テンキー１
	VK_NUMPAD2,		D_DIK_NUMPAD2,		1,	// テンキー２
	VK_NUMPAD3,		D_DIK_NUMPAD3,		1,	// テンキー３
	VK_NUMPAD4,		D_DIK_NUMPAD4,		1,	// テンキー４
	VK_NUMPAD5,		D_DIK_NUMPAD5,		1,	// テンキー５
	VK_NUMPAD6,		D_DIK_NUMPAD6,		1,	// テンキー６
	VK_NUMPAD7,		D_DIK_NUMPAD7,		1,	// テンキー７
	VK_NUMPAD8,		D_DIK_NUMPAD8,		1,	// テンキー８
	VK_NUMPAD9,		D_DIK_NUMPAD9,		1,	// テンキー９
	VK_MULTIPLY,	D_DIK_MULTIPLY,		1,	// テンキー＊キー
	VK_ADD,			D_DIK_ADD,			1,	// テンキー＋キー
	VK_SUBTRACT,	D_DIK_SUBTRACT,		1,	// テンキー－キー
	VK_DECIMAL,		D_DIK_DECIMAL,		1,	// テンキー．キー
	VK_DIVIDE,		D_DIK_DIVIDE,		1,	// テンキー／キー

	VK_RETURN,		D_DIK_NUMPADENTER,	1,	// テンキーのエンターキー
//	VK_NUMPADENTER,	D_DIK_NUMPADENTER,	1,	// テンキーのエンターキー

	VK_LSHIFT,		D_DIK_LSHIFT,		1,	// 左シフトキー
	VK_RSHIFT,		D_DIK_RSHIFT,		1,	// 右シフトキー
	VK_LCONTROL,	D_DIK_LCONTROL,		1,	// 左コントロールキー
	VK_RCONTROL,	D_DIK_RCONTROL,		1,	// 右コントロールキー
	VK_PRIOR,		D_DIK_PGUP,			1,	// ＰａｇｅＵＰキー
	VK_NEXT,		D_DIK_PGDN,			1,	// ＰａｇｅＤｏｗｎキー
	VK_END,			D_DIK_END,			1,	// エンドキー
	VK_HOME,		D_DIK_HOME,			1,	// ホームキー
	VK_INSERT,		D_DIK_INSERT,		1,	// インサートキー

	VK_LMENU,		D_DIK_LALT,			1,	// 左ＡＬＴキー
	VK_RMENU,		D_DIK_RALT,			1,	// 右ＡＬＴキー
	VK_SCROLL,		D_DIK_SCROLL,		1,	// ScrollLockキー
	VK_CAPITAL,		D_DIK_CAPSLOCK,		1,	// CaspLockキー
	VK_PAUSE,		D_DIK_PAUSE,		1,	// PauseBreakキー

	VK_F1,			D_DIK_F1,			1,	// Ｆ１キー
	VK_F2,			D_DIK_F2,			1,	// Ｆ２キー
	VK_F3,			D_DIK_F3,			1,	// Ｆ３キー
	VK_F4,			D_DIK_F4,			1,	// Ｆ４キー
	VK_F5,			D_DIK_F5,			1,	// Ｆ５キー
	VK_F6,			D_DIK_F6,			1,	// Ｆ６キー
	VK_F7,			D_DIK_F7,			1,	// Ｆ７キー
	VK_F8,			D_DIK_F8,			1,	// Ｆ８キー
	VK_F9,			D_DIK_F9,			1,	// Ｆ９キー
	VK_F10,			D_DIK_F10,			1,	// Ｆ１０キー
	VK_F11,			D_DIK_F11,			1,	// Ｆ１１キー
	VK_F12,			D_DIK_F12,			1,	// Ｆ１２キー

	0xffff,			0xffff,
} ;

#define XINPUT_TO_DIRECTINPUT_BUTTONNUM		(10)
WORD XInputButtonToDirectInputButtonNo[ XINPUT_TO_DIRECTINPUT_BUTTONNUM ] =
{
	D_XINPUT_GAMEPAD_A,
	D_XINPUT_GAMEPAD_B,
	D_XINPUT_GAMEPAD_X,
	D_XINPUT_GAMEPAD_Y,
	D_XINPUT_GAMEPAD_LEFT_SHOULDER,
	D_XINPUT_GAMEPAD_RIGHT_SHOULDER,
	D_XINPUT_GAMEPAD_BACK,
	D_XINPUT_GAMEPAD_START,
	D_XINPUT_GAMEPAD_LEFT_THUMB,
	D_XINPUT_GAMEPAD_RIGHT_THUMB
} ;

// 结构体定义 --------------------------------------------------------------------

// 定数定義 ----------------------------------------------------------------------

//#include "DxInputDef.h"
// キーボードデバイスのデータフォーマット定義 =============

// ＧＵＩＤ
extern GUID GUIDDIKEYBOARD ;

// デバイスオブジェクトデータ
extern D_DIOBJECTDATAFORMAT C_ODFDIKEYBOARD[256] ;

// デバイスデータフォーマット
extern D_DIDATAFORMAT C_DFDIKEYBOARD ;


// ジョイスティックデバイスのデータフォーマット定義 =======

// ＧＵＩＤ
extern GUID GUIDDIJOYSTICK[8] ;

// デバイスオブジェクトデータ
extern D_DIOBJECTDATAFORMAT C_ODFDIJOYSTICK[44] ;

// デバイスデータフォーマット
extern D_DIDATAFORMAT C_DFDIJOYSTICK ;
extern D_DIDATAFORMAT C_DFDIJOYSTICK2 ;
extern D_DIDATAFORMAT C_DFDIMOUSE2 ;
extern D_DIDATAFORMAT C_DFDIMOUSE ;


// 内部大域変数宣言 --------------------------------------------------------------

// ＤｉｒｅｃｔＩｎｐｕｔ管理用データ
DINPUTDATA DInputData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// ジョイパッド列挙用コールバック関数
		BOOL FAR PASCAL EnumJoypadProc( const D_DIDEVICEINSTANCEA *pdinst , LPVOID pvRef ) ;

static	void F10Process( void ) ;												// F10 処理
static	int UpdateJoypadInputState( int padno ) ;								// パッドの入力状態の更新

// プログラム --------------------------------------------------------------------

// ＤｉｒｅｃｔＩｎｐｕｔの初期化、終了関数

// ＤｉｒｅｃｔＩｎｐｕｔを初期化する
extern int InitializeDirectInput( void )
{
	HRESULT hr ;
	int NoUseXInputFlag ;
	int NoUseDirectInputFlag ;
	int UseDirectInputFlag ;
	int KeyToJoypadInputInitializeFlag ;
	int NoUseVibrationFlag ;
	int i, j;
	int KeyToJoypadInput[ MAX_JOYPAD_NUM ][ 32 ][ 4 ] ;
	static int NowInitialize = FALSE ;

	// 初期化判定
	if( DInputData.DirectInputObject != NULL || NowInitialize == TRUE ) return 0 ;
	NowInitialize = TRUE ;

	// ゼロ初期化
	NoUseXInputFlag								= DInputData.NoUseXInputFlag ;
	NoUseDirectInputFlag						= DInputData.NoUseDirectInputFlag ;
	UseDirectInputFlag							= DInputData.UseDirectInputFlag ;
	KeyToJoypadInputInitializeFlag				= DInputData.KeyToJoypadInputInitializeFlag ;
	NoUseVibrationFlag							= DInputData.NoUseVibrationFlag ;
	_MEMCPY( KeyToJoypadInput, DInputData.KeyToJoypadInput, sizeof( DInputData.KeyToJoypadInput ) ) ;
	_MEMSET( &DInputData, 0, sizeof( DInputData ) ) ; 
	DInputData.NoUseXInputFlag					= NoUseXInputFlag ;
	DInputData.NoUseDirectInputFlag				= NoUseDirectInputFlag ;
	DInputData.UseDirectInputFlag				= UseDirectInputFlag ;
	DInputData.KeyToJoypadInputInitializeFlag	= KeyToJoypadInputInitializeFlag ;
	DInputData.NoUseVibrationFlag				= NoUseVibrationFlag ;
	_MEMCPY( DInputData.KeyToJoypadInput, KeyToJoypadInput, sizeof( DInputData.KeyToJoypadInput ) ) ;

	// ＤｉｒｅｃｔＩｎｐｕｔ を使用するかどうかで処理を分岐
START:
	if( DInputData.NoUseDirectInputFlag == FALSE )
	{
		int i ;
		const TCHAR *XInputDllFileName[] = 
		{
			_T( "xinput1_4.dll" ),
			_T( "xinput1_3.dll" ),
			_T( "xinput9_1_0.dll" ),
			NULL
		} ;

		// DirectInput を使用する場合

		DXST_ERRORLOG_ADD( _T( "DirectInput関係初期化処理\n" ) ) ;
		DXST_ERRORLOG_TABADD ;

		// ＸＩｎｐｕｔを使用しないフラグが立っていなかったらＸＩｎｐｕｔのＤＬＬを読み込む
		if( DInputData.NoUseXInputFlag == FALSE )
		{
			DXST_ERRORLOG_ADD( _T( "XInput DLL の読み込み中... " ) ) ;
			for( i = 0 ; XInputDllFileName[ i ] != NULL ; i ++ )
			{
				DInputData.XInputDLL = LoadLibrary( XInputDllFileName[ i ] ) ;
				if( DInputData.XInputDLL != NULL )
					break ;
			}

			if( DInputData.XInputDLL == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "失敗、XInputは使用しません\n" ) ) ;
			}
			else
			{
				DXST_ERRORLOG_ADD( _T( "成功\n" ) ) ;

				DInputData.XInputGetStateFunc = ( DWORD ( WINAPI * )( DWORD, D_XINPUT_STATE*     ) )GetProcAddress( DInputData.XInputDLL, "XInputGetState" ) ;
				DInputData.XInputSetStateFunc = ( DWORD ( WINAPI * )( DWORD, D_XINPUT_VIBRATION* ) )GetProcAddress( DInputData.XInputDLL, "XInputSetState" ) ;
			}
		}

		// ＤｉｒｅｃｔＩｎｐｕｔオブジェクトを作成する
		DXST_ERRORLOG_ADD( _T( "DirectInput7 の取得中... " ) ) ;
#ifdef _DEBUG
		hr = E_FAIL;
#else
		hr = WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTINPUT , NULL, CLSCTX_INPROC_SERVER, IID_IDIRECTINPUT7, ( LPVOID * )&DInputData.DirectInputObject ) ;
#endif
		if( !FAILED( hr ) )
		{
			DXST_ERRORLOG_ADD( _T( "成功\n" ) ) ;
			DXST_ERRORLOG_TABADD ;
			DXST_ERRORLOG_ADD( _T( "引き続き初期化処理... " ) ) ;
			hr = DInputData.DirectInputObject->Initialize( GetModuleHandle( NULL ), 0x700 ) ;
			if( FAILED( hr ) ) 
			{
				DXST_ERRORLOG_ADD( _T( "初期化に失敗。" ) ) ;
				DInputData.DirectInputObject->Release() ;
				DInputData.DirectInputObject = NULL ;
				goto ER1;
			}
			DXST_ERRORLOG_ADD( _T( "初期化成功\n" ) ) ;
			DXST_ERRORLOG_TABSUB ;
		}
		else
		{
			DXST_ERRORLOG_ADD( _T( "取得に失敗。\n" ) ) ;
			DXST_ERRORLOGFMT_ADD( (_T( "エラーコード 0x%x\n" ), hr) );
ER1:
			DXST_ERRORLOG_ADD( _T( "DirectInput8 の取得を試みます..." ) ) ;
			hr = WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTINPUT8 , NULL, CLSCTX_INPROC_SERVER, IID_IDIRECTINPUT8, ( LPVOID * )&DInputData.DirectInputObject ) ;
			if( !FAILED( hr ) )
			{
				DXST_ERRORLOG_ADD( _T( "成功\n" ) ) ;
				DXST_ERRORLOG_TABADD ;
				DXST_ERRORLOG_ADD( _T( "引き続き初期化処理... " ) ) ;
				hr = DInputData.DirectInputObject->Initialize( GetModuleHandle( NULL ) , 0x800 ) ;
				if( FAILED( hr ) ) 
				{
					DXST_ERRORLOG_ADD( _T( "初期化に失敗。" ) ) ;
					DInputData.DirectInputObject->Release() ;
					DInputData.DirectInputObject = NULL ;
					goto ER2;
				}
				DXST_ERRORLOG_ADD( _T( "初期化成功\n" ) ) ;
				DInputData.UseDirectInput8Flag = TRUE;
				DXST_ERRORLOG_TABSUB ;
			}
			else
			{
				DXST_ERRORLOG_ADD( _T( "取得に失敗。" ) ) ;
ER2:
				DXST_ERRORLOG_ADD( _T( "DirectInput を使用しません。\n" ) ) ;
				DXST_ERRORLOG_TABSUB ;

				if( DInputData.DirectInputObject ) DInputData.DirectInputObject->Release() ;
				DInputData.DirectInputObject = NULL;
				DInputData.NoUseDirectInputFlag = TRUE;
//				return -1 ;
				goto START;
			}
		}

		// メインウインドウをアクティブにする
		// BringWindowToTop( NS_GetMainWindowHandle() ) ;

		// アクティブになるまで待つ
		NS_ProcessMessage() ;

		// ジョイパッドデバイスを初期化する
		SetupJoypad();

		// マウスデバイスを初期化する
		DInputData.MouseDeviceObject = NULL ;
		{
			// マウスデバイスを作成する
			DXST_ERRORLOG_ADD( _T( "マウスデバイスの初期化... " ) ) ; 
			if( DInputData.UseDirectInput8Flag == TRUE )
			{
				hr = DInputData.DirectInputObject->CreateDevice( GUID_SYSMOUSE, ( D_IDirectInputDevice ** )&DInputData.MouseDeviceObject , NULL ) ;
			}
			else
			{
				hr = DInputData.DirectInputObject->CreateDeviceEx( GUID_SYSMOUSE, IID_IDIRECTINPUTDEVICE7, ( void ** )&DInputData.MouseDeviceObject , NULL ) ;
			}
			if( hr != D_DI_OK )
			{
				DInputData.MouseDeviceObject = NULL ;
				goto MOUSEDEVICEINITEND ;
//				DInputData.DirectInputObject->Release() ;
//				DInputData.DirectInputObject = NULL ;

//				NowInitialize = FALSE ;
//				return DxLib_Error( DXSTRING( _T( "マウスデバイスの取得に失敗しました" ) ) ) ;
			}

			// マウスデバイスのデータ形式を設定する
	//		hr = DInputData.MouseDeviceObject->SetDataFormat( &c_dfDIMouse2 ) ;
			hr = DInputData.MouseDeviceObject->SetDataFormat( &C_DFDIMOUSE2 ) ;
			if( hr != D_DI_OK )
			{
				DInputData.MouseDeviceObject->Release() ;
				DInputData.MouseDeviceObject = NULL ;
				goto MOUSEDEVICEINITEND ;

//				DInputData.DirectInputObject->Release() ;
//				DInputData.DirectInputObject = NULL ;

//				NowInitialize = FALSE ;
//				return DxLib_Error( DXSTRING( _T( "マウスデバイスのデータ形式の設定に失敗しました" ) ) ) ;
			}

			// マウスの協調レベルを設定する
	//		hr = DInputData.MouseDeviceObject->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_EXCLUSIVE | D_DISCL_FOREGROUND ) ;
			hr = DInputData.MouseDeviceObject->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_NONEXCLUSIVE | D_DISCL_BACKGROUND ) ;
	//		hr = DInputData.MouseDeviceObject->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_NONEXCLUSIVE | D_DISCL_FOREGROUND ) ;
 			if( hr != D_DI_OK ) 
			{
				DInputData.MouseDeviceObject->Release() ;
				DInputData.MouseDeviceObject = NULL ;
				goto MOUSEDEVICEINITEND ;

//				DInputData.DirectInputObject->Release() ;
//				DInputData.DirectInputObject = NULL ;

//				NowInitialize = FALSE ;
//				return DxLib_Error( DXSTRING( _T( "マウスデバイスの協調レベルの設定に失敗しました" ) ) ) ;
			}

			DXST_ERRORLOG_ADD( _T( "初期化成功\n" ) ) ;

MOUSEDEVICEINITEND:
			if( DInputData.MouseDeviceObject == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "マウスデバイスの取得に失敗しました\n" ) ) ; 
			}

			DInputData.KeyInputGetTime = WinAPIData.Win32Func.timeGetTimeFunc() ;
		}

		// キーボードデバイスを初期化する
		DInputData.KeyboardDeviceObject = NULL ;
		if( DInputData.KeyboardNotDirectInputFlag == FALSE )
		{
			HANDLE Event ;

			// キーボードデバイスを作成する
			DXST_ERRORLOG_ADD( _T( "キーボードデバイスの初期化... " ) ) ; 
			if( DInputData.UseDirectInput8Flag == TRUE )
			{
				hr = DInputData.DirectInputObject->CreateDevice( GUID_SYSKEYBOARD, ( D_IDirectInputDevice ** )&DInputData.KeyboardDeviceObject , NULL ) ;
			}
			else
			{
				hr = DInputData.DirectInputObject->CreateDeviceEx( GUID_SYSKEYBOARD, IID_IDIRECTINPUTDEVICE7, ( void ** )&DInputData.KeyboardDeviceObject , NULL ) ;
			}
			if( hr != D_DI_OK )
			{
				if( DInputData.MouseDeviceObject )
				{
					DInputData.MouseDeviceObject->Release() ;
					DInputData.MouseDeviceObject = NULL ;
				}

				DInputData.DirectInputObject->Release() ;
				DInputData.DirectInputObject = NULL ;

				NowInitialize = FALSE ;
				return DxLib_Error( DXSTRING( _T( "キーボードデバイスの取得に失敗しました" ) ) ) ;
			}

			// キーボードデバイスのデータ形式を設定する
	//		hr = DInputData.KeyboardDeviceObject->SetDataFormat( &c_dfDIKeyboard ) ;
			hr = DInputData.KeyboardDeviceObject->SetDataFormat( &C_DFDIKEYBOARD ) ;
			if( hr != D_DI_OK )
			{
				if( DInputData.MouseDeviceObject )
				{
					DInputData.MouseDeviceObject->Release() ;
					DInputData.MouseDeviceObject = NULL ;
				}

				DInputData.KeyboardDeviceObject->Release() ;
				DInputData.KeyboardDeviceObject = NULL ;

				DInputData.DirectInputObject->Release() ;
				DInputData.DirectInputObject = NULL ;

				NowInitialize = FALSE ;
				return DxLib_Error( DXSTRING( _T( "キーボードデバイスのデータ形式の設定に失敗しました" ) ) ) ;
			}

			// キーボードの協調レベルを設定する
			DInputData.KeyExclusiveCooperativeLevelFlag = FALSE ;
	//		hr = DInputData.KeyboardDeviceObject->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_EXCLUSIVE | D_DISCL_FOREGROUND ) ;
			hr = DInputData.KeyboardDeviceObject->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_NONEXCLUSIVE | D_DISCL_BACKGROUND ) ;
	//		hr = DInputData.KeyboardDeviceObject->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_NONEXCLUSIVE | D_DISCL_FOREGROUND ) ;
 			if( hr != D_DI_OK ) 
			{
				if( DInputData.MouseDeviceObject )
				{
					DInputData.MouseDeviceObject->Release() ;
					DInputData.MouseDeviceObject = NULL ;
				}

				DInputData.KeyboardDeviceObject->Release() ;
				DInputData.KeyboardDeviceObject = NULL ;

				DInputData.DirectInputObject->Release() ;
				DInputData.DirectInputObject = NULL ;

				NowInitialize = FALSE ;
				return DxLib_Error( DXSTRING( _T( "キーボードデバイスの協調レベルの設定に失敗しました" ) ) ) ;
			}

			// バッファを設定してみる
			{
				D_DIPROPDWORD DIProp ;

				DIProp.diph.dwSize = sizeof( DIProp ) ;
				DIProp.diph.dwHeaderSize = sizeof( DIProp.diph ) ;
				DIProp.diph.dwObj = 0 ;
				DIProp.diph.dwHow = D_DIPH_DEVICE ;
				DIProp.dwData = 100 ;
				if( DInputData.KeyboardDeviceObject->SetProperty( D_DIPROP_BUFFERSIZE, &DIProp.diph ) != D_DI_OK )
				{
					DXST_ERRORLOG_ADD( _T( "キーボードのプロパティ設定に失敗しました\n" ) ) ;
				}
			}

			// 変化通知用のイベントを作成・アタッチする
			Event = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
			hr = DInputData.KeyboardDeviceObject->SetEventNotification( Event ) ;
			if( hr != D_DI_POLLEDDEVICE && hr != D_DI_OK )
			{
				DXST_ERRORLOG_ADD( _T( "キーボードイベントのセットアップが失敗しました\n" ) ) ;
				NowInitialize = FALSE ;
				return -1 ;
			}
			DInputData.KeyEvent = Event ;

			// キーボードのデバイスを取得する
			hr = DInputData.KeyboardDeviceObject->Acquire() ;
			if( hr != D_DI_OK )
			{
				hr = DInputData.KeyboardDeviceObject->Acquire() ;
				if( hr != D_DI_OK )
				{
					if( DInputData.MouseDeviceObject )
					{
						DInputData.MouseDeviceObject->Release() ;
						DInputData.MouseDeviceObject = NULL ;
					}

					DInputData.KeyboardDeviceObject->Release() ;
					DInputData.KeyboardDeviceObject = NULL ;

					DInputData.DirectInputObject->Release() ;
					DInputData.DirectInputObject = NULL ;

					NowInitialize = FALSE ;
					return DxLib_Error( DXSTRING( _T( "キーボードデバイスの取得に失敗しました" ) ) ) ;
				}
			}

			DXST_ERRORLOG_ADD( _T( "初期化成功\n" ) ) ;

			DInputData.KeyInputGetTime = WinAPIData.Win32Func.timeGetTimeFunc() ;
		}
		

		DXST_ERRORLOG_TABSUB ;
		DXST_ERRORLOG_ADD( _T( "DirectInput 関連の初期化は正常に終了しました\n" ) ) ;
	}
	else
	{
		// DirectInput を使用しない場合
		int i ;
		JOYCAPS joycaps ;
		DXST_ERRORLOG_TABADD ;

		// パッドの数を調べる
		for( i = 0 ; i < MAX_JOYPAD_NUM ; i ++ )
		{
//			_MEMSET( &joyex, 0, sizeof( joyex ) ) ;
//			joyex.dwSize  = sizeof( joyex ) ;
//			joyex.dwFlags = JOY_RETURNALL ;
//			if( WinAPIData.Win32Func.joyGetPosExFunc( i, &joyex ) != JOYERR_NOERROR ) break ;
			_MEMSET( &joycaps, 0, sizeof( joycaps ) ) ;
			hr = WinAPIData.Win32Func.joyGetDevCapsFunc( i, &joycaps, sizeof( joycaps ) ) ;
			if( hr != JOYERR_NOERROR ) break ;
			DInputData.Joypad[ i ].RightStickFlag = ( joycaps.wCaps & JOYCAPS_HASZ ) && ( joycaps.wCaps & JOYCAPS_HASR ) ;
			DInputData.Joypad[ i ].DeadZone = DEADZONE ;
		}
		DInputData.JoyPadNum = i ;

		DXST_ERRORLOGFMT_ADD(( _T( "パッドの数は %d 個です\n" ), i ));

		DXST_ERRORLOG_TABSUB ;
		DInputData.DirectInputObject = (D_IDirectInput7 *)(DWORD_PTR)0xffffffff ;
		DXST_ERRORLOG_ADD( _T( "入力関連の初期化をしました\n" ) ) ;
	}

	// キーボードとジョイパッドの入力のデフォルトの対応表を設定する
	if( DInputData.KeyToJoypadInputInitializeFlag == FALSE )
	{
		DInputData.KeyToJoypadInputInitializeFlag = TRUE ;

		for( i = 0 ; i < MAX_JOYPAD_NUM ; i ++ )
		{
			for ( j = 0 ; j < 32 ; j ++ )
			{
				DInputData.KeyToJoypadInput[ i ][ j ][ 0 ] = -1 ;
				DInputData.KeyToJoypadInput[ i ][ j ][ 1 ] = -1 ;
			}
		}
		DInputData.KeyToJoypadInput[ 0 ][  0 ][ 0 ] = D_DIK_NUMPAD2;
		DInputData.KeyToJoypadInput[ 0 ][  0 ][ 1 ] = D_DIK_DOWN;
		DInputData.KeyToJoypadInput[ 0 ][  1 ][ 0 ] = D_DIK_NUMPAD4;
		DInputData.KeyToJoypadInput[ 0 ][  1 ][ 1 ] = D_DIK_LEFT;
		DInputData.KeyToJoypadInput[ 0 ][  2 ][ 0 ] = D_DIK_NUMPAD6;
		DInputData.KeyToJoypadInput[ 0 ][  2 ][ 1 ] = D_DIK_RIGHT;
		DInputData.KeyToJoypadInput[ 0 ][  3 ][ 0 ] = D_DIK_NUMPAD8;
		DInputData.KeyToJoypadInput[ 0 ][  3 ][ 1 ] = D_DIK_UP;
		DInputData.KeyToJoypadInput[ 0 ][  4 ][ 0 ] = D_DIK_Z;
		DInputData.KeyToJoypadInput[ 0 ][  5 ][ 0 ] = D_DIK_X;
		DInputData.KeyToJoypadInput[ 0 ][  6 ][ 0 ] = D_DIK_C;
		DInputData.KeyToJoypadInput[ 0 ][  7 ][ 0 ] = D_DIK_A;
		DInputData.KeyToJoypadInput[ 0 ][  8 ][ 0 ] = D_DIK_S;
		DInputData.KeyToJoypadInput[ 0 ][  9 ][ 0 ] = D_DIK_D;
		DInputData.KeyToJoypadInput[ 0 ][ 10 ][ 0 ] = D_DIK_Q;
		DInputData.KeyToJoypadInput[ 0 ][ 11 ][ 0 ] = D_DIK_W;
		DInputData.KeyToJoypadInput[ 0 ][ 12 ][ 0 ] = D_DIK_ESCAPE;
		DInputData.KeyToJoypadInput[ 0 ][ 13 ][ 0 ] = D_DIK_SPACE;
	}

	// 終了
	NowInitialize = FALSE ;
	return 0 ;
}

// ＤｉｒｅｃｔＩｎｐｕｔの使用を終了する
extern int TerminateDirectInput( void )
{
	// 初期化判定
	if( DInputData.DirectInputObject == NULL ) return 0 ;

	// ＤｉｒｅｃｔＩｎｐｕｔ を使用していたかどうかで処理を分岐
	if( DInputData.DirectInputObject != (D_IDirectInput7 *)(DWORD_PTR)0xffffffff )
	{
		// DirectInput を使用した場合
		DXST_ERRORLOG_ADD( _T( "DirectInput 関連の終了処理... 完了\n" ) ) ;

		// ジョイパッドデバイスの解放
		TerminateJoypad() ;

		// キーボードデバイスの解放
		if( DInputData.KeyboardDeviceObject != NULL )
		{
			DInputData.KeyboardDeviceObject->Unacquire() ;
			DInputData.KeyboardDeviceObject->Release() ;
			DInputData.KeyboardDeviceObject = NULL ;
			CloseHandle( DInputData.KeyEvent ) ;
		}

		// マウスデバイスの解放
		if( DInputData.MouseDeviceObject != NULL )
		{
			DInputData.MouseDeviceObject->Unacquire() ;
			DInputData.MouseDeviceObject->Release() ;
			DInputData.MouseDeviceObject = NULL ;
		}

		// ＤｉｒｅｃｔＩｎｐｕｔオブジェクトの解放
		DInputData.DirectInputObject->Release() ;
		DInputData.DirectInputObject = NULL ;

		// ＸＩｎｐｕｔの後始末処理
		if( DInputData.XInputDLL )
		{
			FreeLibrary( DInputData.XInputDLL ) ;
			DInputData.XInputDLL = NULL ;
			DInputData.XInputGetStateFunc = NULL ;
			DInputData.XInputSetStateFunc = NULL ;
		}
	}
	else
	{
		// DirectInput を使用しなかった場合
		DXST_ERRORLOG_ADD( _T( "入力関連の終了処理... 完了\n" ) ) ;

		DInputData.DirectInputObject = NULL ;
	}

	// 終了
	return 0 ;
}

// ジョイパッドのセットアップを行う
extern int SetupJoypad( void )
{
	// 初期化判定
	if( DInputData.DirectInputObject == NULL || DInputData.DirectInputObject == (D_IDirectInput7 *)(DWORD_PTR)0xffffffff ) return -1 ;

	// 既に初期化されていた場合を考えて後始末を最初に行う
	TerminateJoypad() ;

	// ジョイパッドデバイスを初期化する
	DXST_ERRORLOG_ADD( _T( "ジョイパッドの初期化... \n" ) ) ;
	DInputData.JoyPadNum = 0 ;

	// XInput で取得できるパッドを先に検出
	if( DInputData.XInputDLL != NULL )
	{
		DWORD i ;
		DINPUTPAD *pad ;
		D_XINPUT_STATE state ;

		pad = &DInputData.Joypad[ DInputData.JoyPadNum ] ;
		for( i = 0 ; i < 4 ; i ++ )
		{
			if( DInputData.XInputGetStateFunc( i, &state ) != ERROR_SUCCESS )
				continue ;

			DXST_ERRORLOGFMT_ADD(( _T( "ジョイパッドナンバー %d は Xbox360 Controller です" ), DInputData.JoyPadNum )) ;

			// XInput でのデバイス番号を保存
			pad->XInputDeviceNo = i ;

			// 無効ゾーンのセット
			pad->DeadZone = DEADZONE ;

			// 振動関係の設定の初期化を行う
			pad->EffectPlayFlag			= FALSE ;
			pad->EffectPower			= D_DI_FFNOMINALMAX ;
			pad->EffectPlayStateLeft	= FALSE ;
			pad->EffectPlayStateRight	= FALSE ;

			// ステータスを初期化
			_MEMSET( &pad->State, 0, sizeof( D_DIJOYSTATE ) ) ;
			_MEMSET( &pad->XInputState, 0, sizeof( pad->XInputState ) ) ;

			// 名前を設定
			_TSTRCPY( pad->InstanceName, _T( "Controller (XBOX 360 For Windows)" ) );
			_TSTRCPY( pad->ProductName, _T( "Controller (XBOX 360 For Windows)" ) );

			pad ++ ;
			DInputData.JoyPadNum ++ ;
		}
	}

	DInputData.DirectInputObject->EnumDevices( D_DIDEVTYPE_JOYSTICK, EnumJoypadProc, NULL, D_DIEDFL_ATTACHEDONLY ) ;
	DXST_ERRORLOG_ADD( _T( "ジョイパッドの初期化は正常に終了しました\n" ) ) ;

	// 終了
	return 0;
}

// ジョイパッドの後始末を行う
extern int TerminateJoypad( void )
{
	int i ;
	DINPUTPAD *pad ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL || DInputData.DirectInputObject == (D_IDirectInput7 *)(DWORD_PTR)0xffffffff ) return -1 ;

	// ジョイパッドデバイスの解放
	pad = DInputData.Joypad ;
	for( i = 0 ; i < DInputData.JoyPadNum ; i ++ , pad ++ )
	{
		if( pad->XInputDeviceNo < 0 )
		{
			pad->Device->Unacquire() ;
			pad->Device->SetEventNotification( NULL ) ;

			if( pad->LeftEffect != NULL )
			{
				pad->LeftEffect->Stop() ;
				pad->LeftEffect->Release() ;
				pad->LeftEffect = NULL ;
			}

			if( pad->RightEffect != NULL )
			{
				pad->RightEffect->Stop() ;
				pad->RightEffect->Release() ;
				pad->RightEffect = NULL ;
			}

			pad->Device->Release() ;
			pad->Device = NULL ;
			CloseHandle( pad->Event ) ;
		}
		
		_MEMSET( pad, 0, sizeof( *pad ) ) ;
	}
	DInputData.JoyPadNum = 0 ;

	// 終了
	return 0;
}

BOOL CALLBACK EffectEnumCallBack( const D_DIEFFECTINFOA * /*Info*/, void * /*Data*/ )
{
	return D_DIENUM_CONTINUE ;
}

// ジョイパッド列挙用コールバック関数
BOOL FAR PASCAL EnumJoypadProc( const D_DIDEVICEINSTANCEA *pdinst , LPVOID /*pvRef*/ )
{
	HRESULT hr ;
	D_IDirectInputDevice7 *Joypad ;
	D_DIPROPRANGE diprg ;
	D_DIPROPDWORD dipdw ;
	HANDLE Event ;
	DINPUTPAD *pad ;

	pad = &DInputData.Joypad[ DInputData.JoyPadNum ] ;

	// ジョイスティックデバイスの作成
	pad->XInputDeviceNo = -1 ;
	if( DInputData.UseDirectInput8Flag == TRUE )
	{
		hr = DInputData.DirectInputObject->CreateDevice( pdinst->guidInstance, ( D_IDirectInputDevice ** )&Joypad , NULL ) ;
	}
	else
	{
		hr = DInputData.DirectInputObject->CreateDeviceEx( pdinst->guidInstance, IID_IDIRECTINPUTDEVICE7, ( void ** )&Joypad , NULL ) ;
	}
	if( hr != D_DI_OK )
	{
		DXST_ERRORLOG_ADD( _T( "入力装置の取得に失敗しました\n" ) ) ;
		return D_DIENUM_CONTINUE ;
	}

	// 情報表示
	{
		D_DIDEVICEINSTANCEA State ;

		_MEMSET( &State, 0, sizeof( State ) ) ;
		State.dwSize = sizeof( State ) ;
		Joypad->GetDeviceInfo( &State ) ;

		// Xbox360 コントローラの場合で、XInputで検出されていたら弾く
		if( _TSTRSTR( State.tszInstanceName, _T( "XBOX 360 For Windows" ) ) != NULL )
		{
			int i ;

			for( i = 0 ; i < DInputData.JoyPadNum ; i ++ )
			{
				if( DInputData.Joypad[ i ].XInputDeviceNo >= 0 )
					break ;
			}
			if( i != DInputData.JoyPadNum )
			{
				Joypad->Release() ;
				Joypad = NULL ;
				return D_DIENUM_CONTINUE ;
			}
		}

		DXST_ERRORLOG_ADD( _T( "入力装置を見つけました\n" ) ) ;

		_TSTRCPY( pad->InstanceName, State.tszInstanceName );
		_TSTRCPY( pad->ProductName,  State.tszProductName  );

		DXST_ERRORLOG_TABADD ;
		DXST_ERRORLOGFMT_ADD(( _T( "デバイスの登録名:%s" ), State.tszInstanceName )) ;

		DXST_ERRORLOGFMT_ADD(( _T( "デバイスの製品登録名:%s" ), State.tszProductName )) ;
		DXST_ERRORLOG_TABSUB ;
	}
	
	// ジョイスパッドのデータ形式を設定
//	hr = Joypad->SetDataFormat( &c_dfDIJoystick ) ;
	hr = Joypad->SetDataFormat( &C_DFDIJOYSTICK ) ;
	if( hr != D_DI_OK )
	{
		Joypad->Release() ;
		DXST_ERRORLOG_ADD( _T( "ジョイパッドデバイスではありませんでした、又は設定に失敗しました\n" ) ) ;
		return D_DIENUM_CONTINUE ;
	}

	// ジョイパッドの協調レベルを設定する
//	hr = Joypad->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_BACKGROUND | D_DISCL_NONEXCLUSIVE ) ;
	hr = Joypad->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_FOREGROUND | D_DISCL_EXCLUSIVE ) ;
	if( hr != D_DI_OK )
	{
//		Joypad->Release() ;
		DXST_ERRORLOG_ADD( _T( "ジョイパッドデバイスの協調レベルの設定に失敗しました\n" ) ) ;
//		return D_DIENUM_CONTINUE ;
	}

	// Ｘの範囲を設定
	diprg.diph.dwSize		= sizeof( diprg ) ;
	diprg.diph.dwHeaderSize	= sizeof( diprg.diph ) ;
	diprg.diph.dwObj		= D_DIJOFS_X ;
	diprg.diph.dwHow		= D_DIPH_BYOFFSET ;
	diprg.lMin				= -RANGE ;
	diprg.lMax				= +RANGE ;
	hr = Joypad->SetProperty( D_DIPROP_RANGE , &diprg.diph ) ;
	if( hr != D_DI_OK )
	{
//		Joypad->Release() ;
		DXST_ERRORLOG_ADD( _T( "ジョイパッドデバイスのＸ軸関係の設定に失敗しました\n" ) ) ;
//		return D_DIENUM_CONTINUE ;
	}

	// Ｙの範囲を設定
	diprg.diph.dwObj		= D_DIJOFS_Y ;
	hr = Joypad->SetProperty( D_DIPROP_RANGE , &diprg.diph ) ;
	if( hr != D_DI_OK )
	{
//		Joypad->Release() ;
		DXST_ERRORLOG_ADD( _T( "ジョイパッドデバイスのＹ軸関係の設定に失敗しました\n" ) ) ;
//		return D_DIENUM_CONTINUE ;
	}

	// Ｚの範囲を設定
	diprg.diph.dwObj		= D_DIJOFS_Z ;
	Joypad->SetProperty( D_DIPROP_RANGE , &diprg.diph ) ;

	// ＲＸＲＹＲＺの範囲を設定
	diprg.diph.dwObj		= D_DIJOFS_RX ;
	Joypad->SetProperty( D_DIPROP_RANGE , &diprg.diph ) ;
	diprg.diph.dwObj		= D_DIJOFS_RY ;
	Joypad->SetProperty( D_DIPROP_RANGE , &diprg.diph ) ;
	diprg.diph.dwObj		= D_DIJOFS_RZ ;
	Joypad->SetProperty( D_DIPROP_RANGE , &diprg.diph ) ;

	// 無効ゾーンのセット
	pad->DeadZone = DEADZONE ;

	// Ｘの無効ゾーンを設定
	dipdw.diph.dwSize		= sizeof( dipdw ) ;
	dipdw.diph.dwHeaderSize	= sizeof( dipdw.diph ) ;
	dipdw.diph.dwObj		= D_DIJOFS_X ;
	dipdw.diph.dwHow		= D_DIPH_BYOFFSET ;
	dipdw.dwData			= DEADZONE_DIRECTINPUT( pad->DeadZone ) ;
	hr = Joypad->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;
	if( hr != D_DI_OK )
	{
//		Joypad->Release() ;
		DXST_ERRORLOG_ADD( _T( "ジョイパッドデバイスのＸ軸の無効ゾーンの設定に失敗しました\n" ) ) ;
//		return D_DIENUM_CONTINUE ;
	}

	// Ｙの無効ゾーンを設定
	dipdw.diph.dwObj		= D_DIJOFS_Y ;
	hr = Joypad->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;
	if( hr != D_DI_OK )
	{
//		Joypad->Release() ;
		DXST_ERRORLOG_ADD( _T( "ジョイパッドデバイスのＹ軸の無効ゾーンの設定に失敗しました\n" ) ) ;
//		return D_DIENUM_CONTINUE ;
	}

	// Ｚの無効ゾーンを設定
	dipdw.diph.dwObj		= D_DIJOFS_Z ;
	Joypad->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;

	// ＲＸＲＹＲＺの無効ゾーンを設定
	dipdw.diph.dwObj		= D_DIJOFS_RX ;
	Joypad->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;
	dipdw.diph.dwObj		= D_DIJOFS_RY ;
	Joypad->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;
	dipdw.diph.dwObj		= D_DIJOFS_RZ ;
	Joypad->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;

	// フォースフェードバックオブジェクトの作成(PSパッド想定)
	{
		DWORD dwAxes[2] = { D_DIJOFS_X, D_DIJOFS_Y } ;
		LONG lDirection[2] = { 0, 0 } ;

//		Joypad->EnumEffects( EffectEnumCallBack, NULL, D_DIEFT_ALL ) ;

		// 右側モーター用(手元のパッドでは何故か GUID_SINE エフェクトは右モーターになってた)
		{
			D_DIPERIODIC diPeriodic ;	// タイプ固有パラメータ
			D_DIENVELOPE diEnvelope ;	// エンベロープ
			D_DIEFFECT diEffect ;		// 汎用パラメータ

			_MEMSET( &diEffect, 0, sizeof( diEffect ) ) ;
			_MEMSET( &diEnvelope, 0, sizeof( diEnvelope ) ) ;
			_MEMSET( &diPeriodic, 0, sizeof( diPeriodic ) ) ;

			// 次にタイプ固有パラメータを初期化する。次の例のような値を設定すると、1/20 秒周期の全力周期的エフェクトを生成することになる。

			diPeriodic.dwMagnitude	= D_DI_FFNOMINALMAX ;
			diPeriodic.lOffset		= 0;
			diPeriodic.dwPhase		= 0;
			diPeriodic.dwPeriod		= (DWORD) (1.5 * D_DI_SECONDS);
			// チェーンソーのモーターが始動しようとして、短い間咳のような騒音を出し、次第にそれが止んでいくというエフェクトを得るために、エンベロープに半秒間のアタック時間と 1 秒間のフェード時間を設定する。こうして、短期間の維持期間値が得られる。

			diEnvelope.dwSize			= sizeof(D_DIENVELOPE);
			diEnvelope.dwAttackLevel	= 0;
			diEnvelope.dwAttackTime		= (DWORD) (0.5 * D_DI_SECONDS);
			diEnvelope.dwFadeLevel		= 0;
			diEnvelope.dwFadeTime		= (DWORD) (1.0 * D_DI_SECONDS);
			// 次に、基本的エフェクトパラメータを設定する。これらのパラメータには、方向とデバイス オブジェクト (ボタンと軸) の識別方法を決定するフラグ、エフェクトのサンプル周期とゲイン、およびさきほど準備した他のデータへのポインタが含まれる。さらに、ジョイスティックの発射ボタンにエフェクトを割り付けて、ボタンを押せばエフェクトが自動的に再生するようにする。

			diEffect.dwSize						= sizeof(D_DIEFFECT);
			diEffect.dwFlags					= D_DIEFF_POLAR | D_DIEFF_OBJECTOFFSETS;
			diEffect.dwDuration					= INFINITE ;
 
			diEffect.dwSamplePeriod				= 0;               // デフォルト値
			diEffect.dwGain						= D_DI_FFNOMINALMAX;         // スケーリングなし
			diEffect.dwTriggerButton			= D_DIEB_NOTRIGGER ; // D_DIJOFS_BUTTON( 0 );
			diEffect.dwTriggerRepeatInterval	= 0;
			diEffect.cAxes						= 2;
			diEffect.rgdwAxes					= dwAxes;
			diEffect.rglDirection				= &lDirection[0];
			diEffect.lpEnvelope					= &diEnvelope;
			diEffect.cbTypeSpecificParams		= sizeof(diPeriodic);
			diEffect.lpvTypeSpecificParams		= &diPeriodic;
			// セットアップはここまで。やっとエフェクトを生成できる。

			pad->RightEffect = NULL ;
			if( DInputData.NoUseVibrationFlag == FALSE )
			{
				Joypad->CreateEffect(
								 GUID_SINE,     // 列挙からの GUID
								 &diEffect,      // データの場所
								 &pad->RightEffect,  // インターフェイス ポインタを置く場所
								 NULL ) ;          // 集合化なし
			}
		}

		// 左側モーター設定(手元のパッドでは何故か GUID_CONSTANTFORCE エフェクトは右モーターになってた)
		{
			DWORD    rgdwAxes[2] = { D_DIJOFS_X, D_DIJOFS_Y };
			LONG     rglDirection[2] = { 0, 0 };
			D_DICONSTANTFORCE cf = { D_DI_FFNOMINALMAX };

			D_DIEFFECT eff;
			_MEMSET( &eff, 0, sizeof(eff) );
			eff.dwSize                  = sizeof(D_DIEFFECT);
			eff.dwFlags                 = D_DIEFF_CARTESIAN | D_DIEFF_OBJECTOFFSETS;
			eff.dwDuration              = INFINITE;
			eff.dwSamplePeriod          = 0;
			eff.dwGain                  = D_DI_FFNOMINALMAX;
			eff.dwTriggerButton         = D_DIEB_NOTRIGGER;
			eff.dwTriggerRepeatInterval = 0;
			eff.cAxes                   = 2;
			eff.rgdwAxes                = rgdwAxes;
			eff.rglDirection            = rglDirection;
			eff.lpEnvelope              = 0;
			eff.cbTypeSpecificParams    = sizeof(D_DICONSTANTFORCE);
			eff.lpvTypeSpecificParams   = &cf;
			eff.dwStartDelay            = 0;

			pad->LeftEffect = NULL ;
			if( DInputData.NoUseVibrationFlag == FALSE )
			{
				hr = Joypad->CreateEffect(
								 GUID_CONSTANTFORCE,     // 列挙からの GUID
								 &eff,      // データの場所
								 &pad->LeftEffect,  // インターフェイス ポインタを置く場所
								 NULL ) ;          // 集合化なし
				if( hr != D_DI_OK )
				{
					DXST_ERRORLOG_ADD( _T( "周期的エフェクトの作成に失敗しました。\n" ) );
				}
			}
		}

		// 振動関係の設定の初期化を行う
		pad->EffectPlayFlag			= FALSE ;
		pad->EffectPower			= D_DI_FFNOMINALMAX ;
		pad->EffectPlayStateLeft	= FALSE ;
		pad->EffectPlayStateRight	= FALSE ;
	}

	// 変化通知用のイベントを作成・アタッチする
	Event = CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	hr = Joypad->SetEventNotification( Event ) ;
	if( hr != D_DI_POLLEDDEVICE && hr != D_DI_OK )
	{
		DXST_ERRORLOG_ADD( _T( "ジョイパッドイベントのセットアップが失敗しました\n" ) ) ;
		return D_DIENUM_CONTINUE ;
	}

	// デバイスを取得する
	Joypad->Acquire() ;

	// イベントハンドルを保存
	pad->Event = Event ;

	// ジョイパッドデバイスのアドレスを保存
	pad->Device = Joypad ;

	// ステータスを初期化
	_MEMSET( &pad->State, 0, sizeof( D_DIJOYSTATE ) ) ;

	// ジョイパッドの数を増やす
	DInputData.JoyPadNum ++ ;

	DXST_ERRORLOG_ADD( _T( "ジョイパッドの追加は正常に終了しました\n" ) ) ;

	// 終了
	return ( DInputData.JoyPadNum != MAX_JOYPAD_NUM ) ? D_DIENUM_CONTINUE : D_DIENUM_STOP ;
}










// 输入状态取得函数

// キーボードの状態取得
extern int NS_CheckHitKey( int KeyCode )
{
	int Result ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL )
	{
		if( NS_GetWindowCloseFlag() == FALSE )
			return InitializeDirectInput() ;
		else
			return 0;
	}

/*
	if( KeyCode == KEY_INPUT_F10 )
	{
		int Result ;
		
		F10Process() ;
		Result = DInputData.F10Flag ;

		
		
		return Result ;
	}
*/

	// キーボードの入力状態を更新
	UpdateKeyboardInputState() ;

	// 指定のキーの状態を返す
	Result = ( DInputData.KeyInputBuf[ KeyCode ] & 0x80 ) != 0 ;
	

	
	

	return Result ;
}

// 全キーの押下状態を取得
extern int NS_CheckHitKeyAll( int CheckType )
{
	int i ;
	unsigned char *KeyData ;

	if( WinData.MainWindow == NULL )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	// キーボードのチェック
	if( CheckType & DX_CHECKINPUT_KEY )
	{
		// キーボードの入力状態を更新
		UpdateKeyboardInputState() ;

		// どれか一つでもキーが押されているか判定
		KeyData = DInputData.KeyInputBuf ;
		for( i = 0 ; i < 256 ; i ++ , KeyData ++ )
		{
			if( ( *KeyData & 0x80 ) && i != D_DIK_KANJI && i != D_DIK_CAPITAL && i != D_DIK_KANA )
				return i ;
		}
		F10Process() ;
		if( DInputData.F10Flag )
		{
			return KEY_INPUT_F10 ;
		}
	}

	// ジョイパッドのチェック
	if( CheckType & DX_CHECKINPUT_PAD )
	{
		for( i = 0 ; i < DInputData.JoyPadNum ; i ++ )
		{
			if( NS_GetJoypadInputState( i + 1 ) != 0 )
				return -1 ;
		}
	}

	// マウスボタンのチェック
	if( CheckType & DX_CHECKINPUT_MOUSE )
	{
		int x, y ;
		RECT Rect ;

		NS_GetMousePoint( &x, &y ) ;
		NS_GetWindowCRect( &Rect ) ;
		if( x >= 0                      && y >= 0                      && NS_GetWindowActiveFlag() &&
			x <= Rect.right - Rect.left && y <= Rect.bottom - Rect.top && NS_GetMouseInput() != 0 )
			return -1 ;
	}

	// 終了
	return 0 ;
}

// マウスの情報取得にＤｉｒｅｃｔＩｎｐｕｔを使用しているかどうかを取得する( 戻り値  TRUE:DirectInputを使用している  FALSE:DirectInputを使用していない )
extern	int CheckUseDirectInputMouse( void )
{
	if( DInputData.NoUseDirectInputFlag == TRUE || DInputData.MouseDeviceObject == NULL || ( DInputData.UseDirectInputFlag == FALSE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA ) )
	{
		return FALSE ;
	}

	return TRUE ;
}

// ＤｉｒｅｃｔＩｎｐｕｔを使用したマウスホイールの移動値を取得する
extern	int GetDirectInputMouseMoveZ( int CounterReset )
{
	int Result ;

	NS_GetMouseInput() ;

	Result = DInputData.MouseMoveZ / WHEEL_DELTA ;
	if( CounterReset )
	{
		DInputData.MouseMoveZ -= Result * WHEEL_DELTA ;
	}

	return Result ;
}

// ＤｉｒｅｃｔＩｎｐｕｔを使用したマウスホイールの移動値を取得する( float版 )
extern	float GetDirectInputMouseMoveZF( int CounterReset )
{
	float Result ;

	NS_GetMouseInput() ;

	Result = ( float )DInputData.MouseMoveZ / WHEEL_DELTA ;
	if( CounterReset )
	{
		DInputData.MouseMoveZ = 0 ;
	}

	return Result ;
}

// マウスのボタンの状態を得る 
extern int NS_GetMouseInput( void )
{
	D_DIMOUSESTATE2 input ;
	HRESULT hr ;
	int res ;

	// DirectInput を使用しているかどうかで処理を分岐
	if( CheckUseDirectInputMouse() == FALSE )
	{
		// DirectInput を使用しない場合
		UpdateKeyboardInputState() ;
		if( DInputData.MouseInputBufValidFlag == TRUE )
		{
			res = 0 ;
			if( DInputData.MouseInputBuf[ 0 ] ) res |= MOUSE_INPUT_LEFT ;
			if( DInputData.MouseInputBuf[ 1 ] ) res |= MOUSE_INPUT_RIGHT ;
			if( DInputData.MouseInputBuf[ 2 ] ) res |= MOUSE_INPUT_MIDDLE ;
		}
		else
		{
			unsigned char KeyBuffer[256] ;

			GetKeyboardState( KeyBuffer ) ;
			res = 0 ;
			if( KeyBuffer[ VK_LBUTTON ] & 0x80 ) res |= MOUSE_INPUT_LEFT ;
			if( KeyBuffer[ VK_RBUTTON ] & 0x80 ) res |= MOUSE_INPUT_RIGHT ;
			if( KeyBuffer[ VK_MBUTTON ] & 0x80 ) res |= MOUSE_INPUT_MIDDLE ;
		}
	}
	else
	{
		// DirectInput を使用する場合

		hr = DInputData.MouseDeviceObject->GetDeviceState( sizeof( D_DIMOUSESTATE2 ), (LPVOID)&input ) ;
		if( hr != D_DI_OK )
		{
			if( DInputData.MouseDeviceObject->Acquire() == D_DI_OK )
			{
				hr = DInputData.MouseDeviceObject->GetDeviceState( sizeof( D_DIMOUSESTATE2 ) , (LPVOID)&input ) ;
				if( hr != D_DI_OK ) return 0 ;
			}
			else return 0 ;
		}

		res = 0 ;
		if( input.rgbButtons[0] & 0x80 ) res |= MOUSE_INPUT_1 ;
		if( input.rgbButtons[1] & 0x80 ) res |= MOUSE_INPUT_2 ;
		if( input.rgbButtons[2] & 0x80 ) res |= MOUSE_INPUT_3 ;
		if( input.rgbButtons[3] & 0x80 ) res |= MOUSE_INPUT_4 ;
		if( input.rgbButtons[4] & 0x80 ) res |= MOUSE_INPUT_5 ;
		if( input.rgbButtons[5] & 0x80 ) res |= MOUSE_INPUT_6 ;
		if( input.rgbButtons[6] & 0x80 ) res |= MOUSE_INPUT_7 ;
		if( input.rgbButtons[7] & 0x80 ) res |= MOUSE_INPUT_8 ;

		DInputData.MouseMoveZ += ( int )input.lZ ;
	}

	return res ;
}

// すべてのキーの押下状態を取得する
extern int NS_GetHitKeyStateAll( char *KeyStateBuf )
{
	int i ;
//	char *Buf = (char *)KeyStateBuf ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	// キーボードの入力状態を更新
	UpdateKeyboardInputState() ;

	// キーの入力値を変換
	for( i = 0 ; i < 256 ; i ++ , KeyStateBuf ++ )
	{
		*KeyStateBuf = ( BYTE )( DInputData.KeyInputBuf[i] ) >> 7 ;
	}
	F10Process() ;
//	Buf[KEY_INPUT_F10] = (char)DInputData.F10Flag ;
//	Buf[KEY_INPUT_F12] = (char)DInputData.F12Flag ;

	// 終了
	return 0 ;
}

// キーボードの協調レベルを排他レベルにするかどうかのフラグをセットする
extern int NS_SetKeyExclusiveCooperativeLevelFlag( int Flag )
{
	int hr ;

	// 初期化されていなかったら何もせず終了
	if( DInputData.KeyboardDeviceObject == NULL )
		return 0 ;

	// 状態が同じだったら何もせず終了
	if( DInputData.KeyExclusiveCooperativeLevelFlag == Flag )
		return 0 ;

	// 一度アクセス権を解放する
	DInputData.KeyboardDeviceObject->Unacquire() ;

	// 協調レベルをセット
	DInputData.KeyboardDeviceObject->SetCooperativeLevel( NS_GetMainWindowHandle(),
			Flag == TRUE ? ( D_DISCL_EXCLUSIVE | D_DISCL_FOREGROUND ) : ( D_DISCL_NONEXCLUSIVE | D_DISCL_BACKGROUND ) ) ;

	// アクセス権を取得する
	hr = DInputData.KeyboardDeviceObject->Acquire() ;
	if( hr != D_DI_OK )
	{
		hr = DInputData.KeyboardDeviceObject->Acquire() ;
		if( hr != D_DI_OK )
		{
			DXST_ERRORLOG_ADD( _T( "キーボードデバイスの取得に失敗しました" ) ) ;
			return -1 ;
		}
	}

	// フラグを保存する
	DInputData.KeyExclusiveCooperativeLevelFlag = Flag ;

	// 終了
	return 0 ;
}




// ジョイパッドが接続されている数を取得する
extern int NS_GetJoypadNum( void )
{
	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	return DInputData.JoyPadNum ;
}

// ジョイバッドの入力状態取得
extern int NS_GetJoypadInputState( int InputType )
{
	int BackData = 0 ;
	DINPUTPAD *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	int iX , iY ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	if( ( JoypadNum < -1 || JoypadNum >= DInputData.JoyPadNum ) && 
		( DInputData.JoyPadNum != 0 && ( InputType & DX_INPUT_KEY ) == 0 ) )
	{
		return 0 ;
	}

	if( JoypadNum != -1 && DInputData.JoyPadNum != 0 && JoypadNum < DInputData.JoyPadNum )
	{
		pad = &DInputData.Joypad[ JoypadNum ] ;

		// パッドの情報を取得する
		{
			// 情報の更新
			UpdateJoypadInputState( JoypadNum ) ;

			// 入力状態を保存
			iX = pad->State.lX ;
			iY = pad->State.lY ;

			// カーソルボタン判定
			     if( iX < 0 ) BackData |= PAD_INPUT_LEFT ;			// ←入力判定
			else if( iX > 0 ) BackData |= PAD_INPUT_RIGHT ;			// →入力判定

			     if( iY < 0 ) BackData |= PAD_INPUT_UP ;			// ↑入力判定
			else if( iY > 0 ) BackData |= PAD_INPUT_DOWN ;			// ↓入力判定

			// ボタン入力判定
			{
				int i ;

				// 入力状態を保存
				for( i = 0 ; i < 24 ; i ++ )
				{
					BackData |= ( pad->State.rgbButtons[ i ] & 0x80 ) != 0 ? 1 << ( i + 4 ) : 0 ;
				}
			}
		}
	}

	// キー入力指定もあった場合はキーの入力状態も反映させる
	if( InputType & DX_INPUT_KEY )
	{
		BYTE *KeyBuf ;
		int *Map ;
		int i, j ;
		unsigned int pad ;

		// キーボードの入力状態を更新
		UpdateKeyboardInputState() ;

		KeyBuf = DInputData.KeyInputBuf ;
		Map = DInputData.KeyToJoypadInput[ JoypadNum < 0 ? 0 : JoypadNum ][ 0 ] ;

		pad = 1 ;
		for( i = 0 ; i < 32 ; i ++, pad <<= 1, Map += 4 )
		{
			for( j = 0 ; j < 4 ; j ++ )
			{
				if( Map[ j ] > 0 && ( KeyBuf[ Map[ j ] ] & 0x80 ) )
				{
					BackData |= ( int )pad ;
					break ; 
				}
			}
		}
/*
		if( KeyBuf[ D_DIK_NUMPAD2 ] & 0x80 || KeyBuf[ D_DIK_DOWN  ] & 0x80 ) BackData |= PAD_INPUT_DOWN  ;		// ２キー↓キーチェック
		if( KeyBuf[ D_DIK_NUMPAD4 ] & 0x80 || KeyBuf[ D_DIK_LEFT  ] & 0x80 ) BackData |= PAD_INPUT_LEFT  ;		// ４キー←チェック
		if( KeyBuf[ D_DIK_NUMPAD6 ] & 0x80 || KeyBuf[ D_DIK_RIGHT ] & 0x80 ) BackData |= PAD_INPUT_RIGHT ;		// ６キー→キーチェック
		if( KeyBuf[ D_DIK_NUMPAD8 ] & 0x80 || KeyBuf[ D_DIK_UP    ] & 0x80 ) BackData |= PAD_INPUT_UP    ;		// ８キー↑キーチェック

		if( KeyBuf[ D_DIK_SPACE  ] & 0x80 ) BackData |= PAD_INPUT_M ;											// スペースチェック
		if( KeyBuf[ D_DIK_ESCAPE ] & 0x80 ) BackData |= PAD_INPUT_START ;										// エスケープチェック
		if( KeyBuf[ D_DIK_Z ] & 0x80 ) BackData |= PAD_INPUT_A ;												// Ｚキーチェック
		if( KeyBuf[ D_DIK_X ] & 0x80 ) BackData |= PAD_INPUT_B ;												// Ｘキーチェック
		if( KeyBuf[ D_DIK_C ] & 0x80 ) BackData |= PAD_INPUT_C ;												// Ｃキーチェック
		if( KeyBuf[ D_DIK_A ] & 0x80 ) BackData |= PAD_INPUT_X ;												// Ａキーチェック
		if( KeyBuf[ D_DIK_S ] & 0x80 ) BackData |= PAD_INPUT_Y ;												// Ｓキーチェック
		if( KeyBuf[ D_DIK_D ] & 0x80 ) BackData |= PAD_INPUT_Z ;												// Ｄキーチェック
		if( KeyBuf[ D_DIK_Q ] & 0x80 ) BackData |= PAD_INPUT_L ;												// Ｑキーチェック
		if( KeyBuf[ D_DIK_W ] & 0x80 ) BackData |= PAD_INPUT_R ;												// Ｗキーチェック
*/
	}

	// 終了
	return BackData ;
}

// ジョイパッドのアナログ的なスティック入力情報を得る
extern int NS_GetJoypadAnalogInput( int *XBuf , int *YBuf , int InputType )
{
	int BackData = 0 ;
	DINPUTPAD *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;
	
	if( XBuf != NULL ) *XBuf = 0 ;
	if( YBuf != NULL ) *YBuf = 0 ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	if( ( JoypadNum < -1 || JoypadNum >= DInputData.JoyPadNum ) && 
		( DInputData.JoyPadNum != 0 && ( InputType & DX_INPUT_KEY ) == 0 ) )
	{
		return 0 ;
	}

	if( JoypadNum != -1 && DInputData.JoyPadNum != 0 )
	{
		pad = &DInputData.Joypad[ JoypadNum ] ;

		// 入力状態の更新
		UpdateJoypadInputState( JoypadNum ) ;

		// 入力状態を保存
		if( XBuf ) *XBuf = pad->State.lX ;
		if( YBuf ) *YBuf = pad->State.lY ;
	}

	// キー入力指定もあった場合はキーの入力状態も反映させる
	if( InputType & DX_INPUT_KEY )
	{
		BYTE *KeyBuf ;

		// キーボードの入力状態を更新
		UpdateKeyboardInputState() ;

		KeyBuf = DInputData.KeyInputBuf ;
		if( XBuf )
		{
			if( KeyBuf[ D_DIK_NUMPAD4 ] & 0x80 || KeyBuf[ D_DIK_LEFT  ] & 0x80 ) *XBuf = -RANGE  ;		// ４キー←チェック
			if( KeyBuf[ D_DIK_NUMPAD6 ] & 0x80 || KeyBuf[ D_DIK_RIGHT ] & 0x80 ) *XBuf = RANGE	;		// ６キー→キーチェック
		}
		if( YBuf )
		{
			if( KeyBuf[ D_DIK_NUMPAD2 ] & 0x80 || KeyBuf[ D_DIK_DOWN  ] & 0x80 ) *YBuf = RANGE	;		// ２キー↓キーチェック
			if( KeyBuf[ D_DIK_NUMPAD8 ] & 0x80 || KeyBuf[ D_DIK_UP    ] & 0x80 ) *YBuf = -RANGE  ;		// ８キー↑キーチェック
		}
	}

	// 終了
	return BackData ;
}

// ジョイパッドのアナログ的なスティック入力情報を得る(右スティック用)
extern int NS_GetJoypadAnalogInputRight( int *XBuf, int *YBuf, int InputType )
{
	int BackData = 0 ;
	DINPUTPAD *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;
	
	if( XBuf != NULL ) *XBuf = 0 ;
	if( YBuf != NULL ) *YBuf = 0 ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	if( JoypadNum < -1 || JoypadNum >= DInputData.JoyPadNum )
		return 0 ;

	if( JoypadNum != -1 )
	{
		pad = &DInputData.Joypad[ JoypadNum ] ;

		// 入力状態の更新
		UpdateJoypadInputState( JoypadNum ) ;

		// 入力状態を保存
		if( pad->XInputDeviceNo >= 0 )
		{
			if( XBuf ) *XBuf = pad->State.lRx ;
			if( YBuf ) *YBuf = pad->State.lRy ;
		}
		else
		{
			if( XBuf ) *XBuf = pad->State.lZ ;
			if( YBuf ) *YBuf = pad->State.lRz ;
		}
	}

	// 終了
	return BackData ;
}

// ジョイパッドから DirectInput から得られる生のデータを取得する( DX_CHECKINPUT_KEY や DX_INPUT_KEY_PAD1 を引数に渡すとエラー )
extern	int	NS_GetJoypadDirectInputState( int InputType, DINPUT_JOYSTATE *DInputState )
{
	DINPUTPAD *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum )
	{
		_MEMSET( DInputState, 0, sizeof( DINPUT_JOYSTATE ) ) ;
		DInputState->POV[ 0 ] = 0xffffffff ;
		DInputState->POV[ 1 ] = 0xffffffff ;
		DInputState->POV[ 2 ] = 0xffffffff ;
		DInputState->POV[ 3 ] = 0xffffffff ;
		return -1 ;
	}

	pad = &DInputData.Joypad[ JoypadNum ] ;

	// 入力状態の更新
	UpdateJoypadInputState( JoypadNum ) ;

	// 入力状態を代入する
	if( DInputState )
	{
		DInputState->X = pad->State.lX ;
		DInputState->Y = pad->State.lY ;
		DInputState->Z = pad->State.lZ ;
		DInputState->Rx = pad->State.lRx ;
		DInputState->Ry = pad->State.lRy ;
		DInputState->Rz = pad->State.lRz ;
		DInputState->Slider[ 0 ] = pad->State.rglSlider[ 0 ] ;
		DInputState->Slider[ 1 ] = pad->State.rglSlider[ 1 ] ;
		DInputState->POV[ 0 ] = pad->State.rgdwPOV[ 0 ] ;
		DInputState->POV[ 1 ] = pad->State.rgdwPOV[ 1 ] ;
		DInputState->POV[ 2 ] = pad->State.rgdwPOV[ 2 ] ;
		DInputState->POV[ 3 ] = pad->State.rgdwPOV[ 3 ] ;
		DInputState->Buttons[ 0 ] = pad->State.rgbButtons[ 0 ] ;
		DInputState->Buttons[ 1 ] = pad->State.rgbButtons[ 1 ] ;
		DInputState->Buttons[ 2 ] = pad->State.rgbButtons[ 2 ] ;
		DInputState->Buttons[ 3 ] = pad->State.rgbButtons[ 3 ] ;
		DInputState->Buttons[ 4 ] = pad->State.rgbButtons[ 4 ] ;
		DInputState->Buttons[ 5 ] = pad->State.rgbButtons[ 5 ] ;
		DInputState->Buttons[ 6 ] = pad->State.rgbButtons[ 6 ] ;
		DInputState->Buttons[ 7 ] = pad->State.rgbButtons[ 7 ] ;
		DInputState->Buttons[ 8 ] = pad->State.rgbButtons[ 8 ] ;
		DInputState->Buttons[ 9 ] = pad->State.rgbButtons[ 9 ] ;
		DInputState->Buttons[ 10 ] = pad->State.rgbButtons[ 10 ] ;
		DInputState->Buttons[ 11 ] = pad->State.rgbButtons[ 11 ] ;
		DInputState->Buttons[ 12 ] = pad->State.rgbButtons[ 12 ] ;
		DInputState->Buttons[ 13 ] = pad->State.rgbButtons[ 13 ] ;
		DInputState->Buttons[ 14 ] = pad->State.rgbButtons[ 14 ] ;
		DInputState->Buttons[ 15 ] = pad->State.rgbButtons[ 15 ] ;
		DInputState->Buttons[ 16 ] = pad->State.rgbButtons[ 16 ] ;
		DInputState->Buttons[ 17 ] = pad->State.rgbButtons[ 17 ] ;
		DInputState->Buttons[ 18 ] = pad->State.rgbButtons[ 18 ] ;
		DInputState->Buttons[ 19 ] = pad->State.rgbButtons[ 19 ] ;
		DInputState->Buttons[ 20 ] = pad->State.rgbButtons[ 20 ] ;
		DInputState->Buttons[ 21 ] = pad->State.rgbButtons[ 21 ] ;
		DInputState->Buttons[ 22 ] = pad->State.rgbButtons[ 22 ] ;
		DInputState->Buttons[ 23 ] = pad->State.rgbButtons[ 23 ] ;
		DInputState->Buttons[ 24 ] = pad->State.rgbButtons[ 24 ] ;
		DInputState->Buttons[ 25 ] = pad->State.rgbButtons[ 25 ] ;
		DInputState->Buttons[ 26 ] = pad->State.rgbButtons[ 26 ] ;
		DInputState->Buttons[ 27 ] = pad->State.rgbButtons[ 27 ] ;
		DInputState->Buttons[ 28 ] = pad->State.rgbButtons[ 28 ] ;
		DInputState->Buttons[ 29 ] = pad->State.rgbButtons[ 29 ] ;
		DInputState->Buttons[ 30 ] = pad->State.rgbButtons[ 30 ] ;
		DInputState->Buttons[ 31 ] = pad->State.rgbButtons[ 31 ] ;
	}

	// 正常終了
	return 0 ;
}

// 指定の入力デバイスが XInput に対応しているかどうかを取得する
// ( 戻り値  TRUE:XInput対応の入力デバイス  FALSE:XInput非対応の入力デバイス   -1:エラー )
// ( DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キーボードが絡むタイプを InputType に渡すとエラーとなり -1 を返す )
extern int NS_CheckJoypadXInput( int InputType )
{
	DINPUTPAD *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum )
		return -1 ;

	pad = &DInputData.Joypad[ JoypadNum ] ;

	return pad->XInputDeviceNo < 0 ? FALSE : TRUE ;
}

// XInput から得られる入力デバイス( Xbox360コントローラ等 )の生のデータを取得する( XInput非対応のパッドの場合はエラーとなり -1 を返す、DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キーボードが絡むタイプを InputType に渡すとエラーとなり -1 を返す )
extern int NS_GetJoypadXInputState(	int InputType, XINPUT_STATE *XInputState )
{
	DINPUTPAD *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum )
	{
		_MEMSET( XInputState, 0, sizeof( XINPUT_STATE ) ) ;
		return -1 ;
	}

	pad = &DInputData.Joypad[ JoypadNum ] ;

	if( pad->XInputDeviceNo < 0 )
	{
		_MEMSET( XInputState, 0, sizeof( XINPUT_STATE ) ) ;
		return -1 ;
	}

	// 入力状態の更新
	UpdateJoypadInputState( JoypadNum ) ;

	// 入力状態を代入する
	if( XInputState )
	{
		XInputState->Buttons[ 0  ] = ( pad->XInputState.Gamepad.wButtons >> 0 ) & 1 ;
		XInputState->Buttons[ 1  ] = ( pad->XInputState.Gamepad.wButtons >> 1 ) & 1 ;
		XInputState->Buttons[ 2  ] = ( pad->XInputState.Gamepad.wButtons >> 2 ) & 1 ;
		XInputState->Buttons[ 3  ] = ( pad->XInputState.Gamepad.wButtons >> 3 ) & 1 ;
		XInputState->Buttons[ 4  ] = ( pad->XInputState.Gamepad.wButtons >> 4 ) & 1 ;
		XInputState->Buttons[ 5  ] = ( pad->XInputState.Gamepad.wButtons >> 5 ) & 1 ;
		XInputState->Buttons[ 6  ] = ( pad->XInputState.Gamepad.wButtons >> 6 ) & 1 ;
		XInputState->Buttons[ 7  ] = ( pad->XInputState.Gamepad.wButtons >> 7 ) & 1 ;
		XInputState->Buttons[ 8  ] = ( pad->XInputState.Gamepad.wButtons >> 8 ) & 1 ;
		XInputState->Buttons[ 9  ] = ( pad->XInputState.Gamepad.wButtons >> 9 ) & 1 ;
		XInputState->Buttons[ 10 ] = ( pad->XInputState.Gamepad.wButtons >> 10 ) & 1 ;
		XInputState->Buttons[ 11 ] = ( pad->XInputState.Gamepad.wButtons >> 11 ) & 1 ;
		XInputState->Buttons[ 12 ] = ( pad->XInputState.Gamepad.wButtons >> 12 ) & 1 ;
		XInputState->Buttons[ 13 ] = ( pad->XInputState.Gamepad.wButtons >> 13 ) & 1 ;
		XInputState->Buttons[ 14 ] = ( pad->XInputState.Gamepad.wButtons >> 14 ) & 1 ;
		XInputState->Buttons[ 15 ] = ( pad->XInputState.Gamepad.wButtons >> 15 ) & 1 ;
		XInputState->LeftTrigger = pad->XInputState.Gamepad.bLeftTrigger ;
		XInputState->RightTrigger = pad->XInputState.Gamepad.bRightTrigger ;
		XInputState->ThumbLX = pad->XInputState.Gamepad.sThumbLX ;
		XInputState->ThumbLY = pad->XInputState.Gamepad.sThumbLY ;
		XInputState->ThumbRX = pad->XInputState.Gamepad.sThumbRX ;
		XInputState->ThumbRY = pad->XInputState.Gamepad.sThumbRY ;
	}

	// 正常終了
	return 0 ;
}

// キーボードのバッファからデータを取得する処理(バッファを空にするだけだけど・・・)
extern int NS_KeyboradBufferProcess( void )
{
	D_DIDEVICEOBJECTDATA_DX7 data7[5] ;
	D_DIDEVICEOBJECTDATA_DX8 data8[5] ;
	DWORD num ;

	if( DInputData.KeyboardDeviceObject == NULL )
		return 0 ;

	num = 5 ;
	while( num == 5 )
	{
		if( DInputData.UseDirectInput8Flag == FALSE )
		{
			if( DInputData.KeyboardDeviceObject->GetDeviceData( sizeof( D_DIDEVICEOBJECTDATA_DX7 ), data7, &num, 0 ) != D_DI_OK ) return 0 ;
		}
		else
		{
			if( DInputData.KeyboardDeviceObject->GetDeviceData( sizeof( D_DIDEVICEOBJECTDATA_DX8 ), data8, &num, 0 ) != D_DI_OK ) return 0 ;
		}
//		if( num != 0 ) DXST_ERRORLOGFMT_ADD(( _T( "押されたか離されたかしたキー:0x%x  0x%x \n" ), data[0].dwOfs, data[0].dwData & 0xff )) ;
	}
	
	return 0 ;
}

// ジョイパッドのＧＵIＤを得る
extern int NS_GetJoypadGUID( int PadIndex, GUID *GuidBuffer )
{
	D_DIDEVICEINSTANCEA inst ;

	if( DInputData.JoyPadNum <= PadIndex )
		return -1 ;

	if( DInputData.Joypad[ PadIndex ].Device == NULL ) return -1 ;

	_MEMSET( &inst, 0, sizeof( inst ) ) ;
	inst.dwSize = sizeof( inst ) ;
	DInputData.Joypad[ PadIndex ].Device->GetDeviceInfo( &inst ) ;
	*GuidBuffer = inst.guidInstance ;

	return 0 ;
}

// ＤＸライブラリのキーコードから Windows の仮想キーコードを取得する
extern int NS_ConvertKeyCodeToVirtualKey( int KeyCode )
{
	int i ;

	for( i = 0 ; __KeyMap[i][0] != 0xffff ; i ++ )
	{
		if( KeyCode == __KeyMap[i][1] ) break ;
	}

	return __KeyMap[i][0] ;
}

//  Windows の仮想キーコード( VK_LEFT など ) に対応するＤＸライブラリのキーコード( KEY_INPUT_A など )を取得する( VirtualKey:変換したいWindowsの仮想キーコード　戻り値：ＤＸライブラリのキーコード )
extern int NS_ConvertVirtualKeyToKeyCode( int VirtualKey )
{
	int i ;

	for( i = 0 ; __KeyMap[i][0] != 0xffff ; i ++ )
	{
		if( VirtualKey == __KeyMap[i][0] ) break ;
	}

	return __KeyMap[i][1] ;
}

// ジョイパッドの入力に対応したキーボードの入力を設定する
extern	int NS_SetJoypadInputToKeyInput( int InputType, int PadInput, int KeyInput1, int KeyInput2 , int KeyInput3 , int KeyInput4 )
{
	unsigned int Bit , i ;

	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	if( JoypadNum < 0 || JoypadNum >= MAX_JOYPAD_NUM )
		return 0 ;

	Bit = (unsigned int)PadInput ;
	for( i = 0 ; i < 32 ; i ++ )
	{
		if( Bit & ( 1 << i ) )
		{
			DInputData.KeyToJoypadInput[ JoypadNum ][ i ][ 0 ] = KeyInput1 ;
			DInputData.KeyToJoypadInput[ JoypadNum ][ i ][ 1 ] = KeyInput2 ;
			DInputData.KeyToJoypadInput[ JoypadNum ][ i ][ 2 ] = KeyInput3 ;
			DInputData.KeyToJoypadInput[ JoypadNum ][ i ][ 3 ] = KeyInput4 ;
		}
	}

	// 正常終了
	return 0 ;
}

// ジョイパッドの無効ゾーンの設定を行う
extern	int NS_SetJoypadDeadZone( int InputType, double Zone )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	DINPUTPAD *pad = &DInputData.Joypad[ JoypadNum ] ;
	D_DIPROPDWORD dipdw ;
	DWORD ZoneI ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;
	
	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum || pad->Device == NULL  )
		return 0 ;

	// 今までと同じゾーンの場合は何もせず終了
	ZoneI = ( DWORD )_DTOL( Zone * 65536 ) ;
	if( pad->DeadZone == ZoneI )
		return 0 ;

	// ゾーンを保存
	pad->DeadZone = ZoneI ;

	// Ｘの無効ゾーンを設定
	dipdw.diph.dwSize		= sizeof( dipdw ) ;
	dipdw.diph.dwHeaderSize	= sizeof( dipdw.diph ) ;
	dipdw.diph.dwObj		= D_DIJOFS_X ;
	dipdw.diph.dwHow		= D_DIPH_BYOFFSET ;
	dipdw.dwData			= 10000 * pad->DeadZone / 65536 ;
	pad->Device->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;

	// Ｙの無効ゾーンを設定
	dipdw.diph.dwObj		= D_DIJOFS_Y ;
	pad->Device->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;

	// Ｚの無効ゾーンを設定
	dipdw.diph.dwObj		= D_DIJOFS_Z ;
	pad->Device->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;

	// Ｚ回転の無効ゾーンを設定
	dipdw.diph.dwObj		= D_DIJOFS_RZ ;
	pad->Device->SetProperty( D_DIPROP_DEADZONE , &dipdw.diph ) ;

	// 正常終了
	return 0 ;
}

// ジョイパッドの振動を開始する
extern	int NS_StartJoypadVibration( int InputType, int Power, int Time )
{
	D_DICONSTANTFORCE cf ;
	D_DIEFFECT eff;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	DINPUTPAD *pad = &DInputData.Joypad[ JoypadNum ] ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	
	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum )
		return 0 ;
	if( pad->XInputDeviceNo < 0 && ( pad->Device == NULL || pad->LeftEffect == NULL ) )
		return 0 ;

	// ゆれの大きさを DirectInput の型に合わせる
	if( Power == -1 ) Power = pad->EffectPower ;
	Power *= 10 ;
		 if( Power >  D_DI_FFNOMINALMAX ) Power = D_DI_FFNOMINALMAX ;
	else if( Power < -D_DI_FFNOMINALMAX ) Power = -D_DI_FFNOMINALMAX ;

	// 再生開始時刻などをセット
	pad->EffectBackTime	= NS_GetNowCount() ;
	pad->EffectTime		= Time ;
	pad->EffectCompTime	= 0 ;

	// 既に再生中でゆれの大きさも同じ場合は何もしない
	if( pad->EffectPlayFlag == TRUE && pad->EffectPower == Power )
		return 0 ;

	// ゆれの強さを変更する
	if( pad->XInputDeviceNo < 0 )
	{
		cf.lMagnitude = Power ;

		_MEMSET( &eff, 0, sizeof(eff) );
		eff.dwSize                = sizeof( D_DIEFFECT );
		eff.dwFlags               = D_DIEFF_CARTESIAN | D_DIEFF_OBJECTOFFSETS;
		eff.lpEnvelope            = 0;
		eff.cbTypeSpecificParams  = sizeof( D_DICONSTANTFORCE);
		eff.lpvTypeSpecificParams = &cf;
		eff.dwStartDelay          = 0;

		if( pad->EffectPlayStateLeft == TRUE )
			pad->LeftEffect->SetParameters( &eff, D_DIEP_TYPESPECIFICPARAMS | D_DIEP_START ) ;
		else
			pad->LeftEffect->SetParameters( &eff, D_DIEP_TYPESPECIFICPARAMS ) ;
	}

	// 再生設定を保存
	pad->EffectPlayFlag	= TRUE ;
	pad->EffectPower	= Power ;

	// 再生状態の設定を反映
	RefreshEffectPlayState() ;

	// 終了
	return 0 ;
}

// ジョイパッドの振動を停止する
extern	int NS_StopJoypadVibration( int InputType )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	DINPUTPAD *pad = &DInputData.Joypad[ JoypadNum ] ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;
	
	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum )
		return 0 ;
	if( pad->XInputDeviceNo < 0 && ( pad->Device == NULL || pad->LeftEffect == NULL ) )
		return 0 ;

	// 既に再生が停止していたら何もしない
	if( pad->EffectPlayFlag == FALSE )
		return 0 ;

	// 再生フラグを倒す
	pad->EffectPlayFlag = FALSE ;

	// 振動の強さを初期化
	pad->EffectPower = D_DI_FFNOMINALMAX ;

	// 再生状態の設定を反映
	RefreshEffectPlayState() ;

	// 終了
	return 0 ;
}

// ジョイパッドのＰＯＶ入力の状態を得る( 単位は角度の１００倍  中心位置にある場合は -1 が返る )
extern int NS_GetJoypadPOVState( int InputType, int POVNumber )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	DINPUTPAD *pad = &DInputData.Joypad[ JoypadNum ] ;
	DWORD pov;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;
	
	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum || pad->Device == NULL || POVNumber >= 4 )
		return -1 ;

	// 情報の更新
	UpdateJoypadInputState( JoypadNum ) ;
	pov = pad->State.rgdwPOV[POVNumber];

	// 中心チェック
	if( ( pov & 0xffff ) == 0xffff ) return -1;

	// 何らかの角度がある場合はそのまま返す
	return (int)pov;
}

// ジョイパッドのデバイス登録名と製品登録名を取得する
extern int NS_GetJoypadName( int InputType, TCHAR *InstanceNameBuffer, TCHAR *ProductNameBuffer )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	DINPUTPAD *pad = &DInputData.Joypad[ JoypadNum ] ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;
	
	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return InitializeDirectInput() ;

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= DInputData.JoyPadNum || ( pad->Device == NULL && pad->XInputDeviceNo < 0 ) )
		return -1 ;

	// 名前をコピー
	if( InstanceNameBuffer != NULL ) _TSTRCPY( InstanceNameBuffer, pad->InstanceName ) ;
	if( ProductNameBuffer  != NULL ) _TSTRCPY( ProductNameBuffer,  pad->ProductName  ) ;

	// 終了
	return 0;
}

// ジョイパッドの再セットアップを行う( 新たに接続されたジョイパッドがある場合に検出される )
extern int NS_ReSetupJoypad( void )
{
	// ジョイパッドの再セットアップを行う
	return SetupJoypad() ;
}

// F10 が押されたことにするフラグを立てる
extern int SetF10Input( void )
{
	DInputData.F10Flag = TRUE ;
//	DInputData.F10InputTime = NS_GetNowCount() ;

	// 終了
	return 0 ;
}

// F10 が離されたことにするフラグを立てる
extern int ResetF10Input( void )
{
	DInputData.F10Flag = FALSE ;

	// 終了
	return 0 ;
}

// F12 が押されたことにするフラグを立てる
extern int SetF12Input( void )
{
	DInputData.F12Flag = TRUE ;
//	DInputData.F12InputTime = NS_GetNowCount() ;

	// 終了
	return 0 ;
}

// F12 が離されたことにするフラグを立てる
extern int ResetF12Input( void )
{
	DInputData.F12Flag = FALSE ;

	// 終了
	return 0 ;
}


// F10 処理
static void F10Process( void )
{
/*
	if( DInputData.F10Flag == TRUE )
	{
		if( NS_GetNowCount() - DInputData.F10InputTime > 64 )
		{
			DInputData.F10Flag = FALSE ;
		}
	}
*/
}

// キーボードの入力状態の更新
extern int UpdateKeyboardInputState( int UseProcessMessage )
{
	static bool InFunction = false ;
	unsigned char KeyBuffer[256] ;
	int Time, hr, i ;
	int Result = 0 ;

	// 無限再帰防止
	if( InFunction == true ) return 0 ;
	InFunction = true ;

	// キーボード入力処理に DirectInput を使用するかどうかで処理を分岐
	if( DInputData.NoUseDirectInputFlag == TRUE || DInputData.KeyboardNotDirectInputFlag == TRUE || DInputData.KeyboardDeviceObject == NULL || ( DInputData.UseDirectInputFlag == FALSE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA ) )
	{
		// メインスレッド以外から呼ばれた場合は何もせずに終了
		if( GetCurrentThreadId() != WinData.MainThreadID ) goto ENDFUNCTION ;

		// まだ前回の取得から 1msec も経過していない場合は以前の状態を返す
		Time = WinAPIData.Win32Func.timeGetTimeFunc() ;
		if( DInputData.KeyInputGetTime == Time ) goto ENDFUNCTION ;
		DInputData.KeyInputGetTime = Time ;

		// キー入力のメッセージが来ている可能性があるので、メッセージ処理を行う
		if( UseProcessMessage != FALSE && NS_ProcessMessage() != 0 ) goto ENDFUNCTION ;

		// 標準ＡＰＩで入力状態を取得する
		GetKeyboardState( KeyBuffer ) ;
		_MEMSET( &DInputData.KeyInputBuf, 0, sizeof( DInputData.KeyInputBuf ) ) ;
		for( i = 0 ; __KeyMap[i][0] != 0xffff ; i ++ )
			DInputData.KeyInputBuf[__KeyMap[i][1]] = KeyBuffer[__KeyMap[i][0]] ;

		DInputData.MouseInputBuf[ 0 ] = ( KeyBuffer[ VK_LBUTTON ] & 0x80 ) ? 1 : 0 ;
		DInputData.MouseInputBuf[ 1 ] = ( KeyBuffer[ VK_RBUTTON ] & 0x80 ) ? 1 : 0 ;
		DInputData.MouseInputBuf[ 2 ] = ( KeyBuffer[ VK_MBUTTON ] & 0x80 ) ? 1 : 0 ;
		DInputData.MouseInputBufValidFlag = TRUE ;
	}
	else
	{
		// まだ前回の取得から 1msec も経過していない場合は以前の状態を返す
		Time = WinAPIData.Win32Func.timeGetTimeFunc() ;
		if( DInputData.KeyInputGetTime == Time && WaitForSingleObject( DInputData.KeyEvent, 0 ) != WAIT_OBJECT_0 ) goto ENDFUNCTION ;

		// キー入力のメッセージが来ている可能性があるので、メッセージ処理を行う
		if( UseProcessMessage != FALSE && NS_ProcessMessage() != 0 ) goto ENDFUNCTION ;

		// 初期化判定
		if( DInputData.DirectInputObject == NULL ) goto ENDFUNCTION ;

		// ステータス取得、ロストしていたら再取得
		hr = DInputData.KeyboardDeviceObject->GetDeviceState( sizeof( DInputData.KeyInputBuf ) , ( LPVOID )DInputData.KeyInputBuf ) ;
//		if( hr == DIERR_INPUTLOST )
		if( hr != D_DI_OK )
		{
			if( DInputData.KeyboardDeviceObject->Acquire() == D_DI_OK )
			{
				hr = DInputData.KeyboardDeviceObject->GetDeviceState( sizeof( DInputData.KeyInputBuf ) , ( LPVOID )DInputData.KeyInputBuf ) ;
				if( hr != D_DI_OK ) goto ENDFUNCTION ;
			}
			else goto ENDFUNCTION ;
		}

		DInputData.KeyInputGetTime = Time ;
		
		// イベントをリセット
		if( WaitForSingleObject( DInputData.KeyEvent, 0 ) == WAIT_OBJECT_0 ) ResetEvent( DInputData.KeyEvent ) ;

		// 右シフトキーを標準ＡＰＩから取る
		GetKeyboardState( KeyBuffer ) ;
		DInputData.KeyInputBuf[ KEY_INPUT_RSHIFT ] = KeyBuffer[ VK_RSHIFT ] ;

		// ******* ＤｉｒｅｃｔＩｎｐｕｔが信用できないので、一部のキーを除き標準ＡＰＩでキー状態を取得 ******* //
/*		if( DInputData.KeyExclusiveCooperativeLevelFlag == FALSE )
		{
			//メインスレッド以外だったらこの処理を行わない
			if( WinData.MainThreadID == GetCurrentThreadId() )
			{
				// 標準ＡＰＩで入力状態を取得する
				GetKeyboardState( KeyBuffer ) ;
				for( i = 0 ; __KeyMap[i][0] != 0xffff ; i ++ )
					if( __KeyMap[i][2] != 1 )
						DInputData.KeyInputBuf[__KeyMap[i][1]] = KeyBuffer[__KeyMap[i][0]] ;
			}
		}
*/	}

	Result = 1 ;

ENDFUNCTION:
	InFunction = false ;

	return Result ;
}

// パッドの入力状態の更新
static int UpdateJoypadInputState( int padno )
{
	DINPUTPAD *pad = &DInputData.Joypad[padno] ;
	D_DIJOYSTATE InputState ;
	JOYINFOEX joyex ;
	HRESULT hr ;
	int Time, i ;

	if( padno >= MAX_JOYPAD_NUM ) return -1 ;

	// DirectInput を使用しているかどうかで処理を分岐
	if( DInputData.NoUseDirectInputFlag == TRUE )
	{
		// DirectInput を使用しない場合

		// 前回の取得から 1msec も経過していない場合は以前の状態を返す
		Time = WinAPIData.Win32Func.timeGetTimeFunc() ;
		if( pad->GetTime == Time ) return 0 ;
		pad->GetTime = Time ;

		// 入力状態を取得する
		_MEMSET( &joyex, 0, sizeof( joyex ) ) ;
		joyex.dwSize  = sizeof( JOYINFOEX ) ;
		joyex.dwFlags = JOY_RETURNALL ;
		if( WinAPIData.Win32Func.joyGetPosExFunc( padno, &joyex ) != JOYERR_NOERROR )
		{
			// 取得できなかった場合は接続されていないということなので情報をゼロで埋めておく
			_MEMSET( &pad->State, 0, sizeof( pad->State ) ) ;
		}
		else
		{
			// マルチメディアＡＰＩ用の入力値を DirectInput 用に変換する

			switch( joyex.dwXpos )
			{
			case 0x7fff :	pad->State.lX =      0 ;	break ;
			case 0x0000 :	pad->State.lX = -RANGE ;	break ;
			case 0xffff :	pad->State.lX =  RANGE ;	break ;
			default :
				     if( joyex.dwXpos < 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lX = -( ( ( 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) - (LONG)joyex.dwXpos ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
				else if( joyex.dwXpos > 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lX =  ( ( (LONG)joyex.dwXpos - ( 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
				else                                              pad->State.lX = 0 ;
				break ;
			}

			switch( joyex.dwYpos )
			{
			case 0x7fff :	pad->State.lY =      0 ;	break ;
			case 0x0000 :	pad->State.lY = -RANGE ;	break ;
			case 0xffff :	pad->State.lY =  RANGE ;	break ;
			default :
				     if( joyex.dwYpos < 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lY = -( ( ( 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) - (LONG)joyex.dwYpos ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
				else if( joyex.dwYpos > 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lY =  ( ( (LONG)joyex.dwYpos - ( 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
				else                                              pad->State.lY = 0 ;
				break ;
			}

			if( pad->RightStickFlag )
			{
				switch( joyex.dwZpos )
				{
				case 0x7fff :	pad->State.lZ =      0 ;	break ;
				case 0x0000 :	pad->State.lZ = -RANGE ;	break ;
				case 0xffff :	pad->State.lZ =  RANGE ;	break ;
				default :
						 if( joyex.dwZpos < 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lZ = -( ( ( 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) - (LONG)joyex.dwZpos ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
					else if( joyex.dwZpos > 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lZ =  ( ( (LONG)joyex.dwZpos - ( 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
					else                                              pad->State.lZ = 0 ;
					break ;
				}

				switch( joyex.dwRpos )
				{
				case 0x7fff :	pad->State.lRz =      0 ;	break ;
				case 0x0000 :	pad->State.lRz = -RANGE ;	break ;
				case 0xffff :	pad->State.lRz =  RANGE ;	break ;
				default :
						 if( joyex.dwRpos < 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lRz = -( ( ( 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) - (LONG)joyex.dwRpos ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
					else if( joyex.dwRpos > 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lRz =  ( ( (LONG)joyex.dwRpos - ( 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
					else                                              pad->State.lRz = 0 ;
					break ;
				}

				switch( joyex.dwUpos )
				{
				case 0x7fff :	pad->State.lRx =      0 ;	break ;
				case 0x0000 :	pad->State.lRx = -RANGE ;	break ;
				case 0xffff :	pad->State.lRx =  RANGE ;	break ;
				default :
						 if( joyex.dwUpos < 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lRx = -( ( ( 0x8000 - DEADZONE_WINMM( pad->DeadZone ) ) - (LONG)joyex.dwUpos ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
					else if( joyex.dwUpos > 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) pad->State.lRx =  ( ( (LONG)joyex.dwUpos - ( 0x8000 + DEADZONE_WINMM( pad->DeadZone ) ) ) * RANGE / VALIDRANGE_WINMM( pad->DeadZone ) ) ;
					else                                              pad->State.lRx = 0 ;
					break ;
				}
			}
			else
			{
				pad->State.lZ = 0 ;
				pad->State.lRx = 0 ;
				pad->State.lRz = 0 ;
			}

			for( i = 0 ; i < 24 ; i ++ )
			{
				if( joyex.dwButtons & ( 1 << i ) ) pad->State.rgbButtons[ i ] = 0x80 ;
				else                               pad->State.rgbButtons[ i ] = 0x00 ;
			}
		}
	}
	else
	if( pad->XInputDeviceNo < 0 )
	{
		// DirectInput を使用する場合

		// 前回の取得から 1msec も経過していない場合は以前の状態を返す
		if( pad->GetTime == ( Time = WinAPIData.Win32Func.timeGetTimeFunc() ) &&
			WaitForSingleObject( pad->Event, 0 ) != WAIT_OBJECT_0 ) return 0 ;
		pad->GetTime = Time ;

		// 入力データのポーリング
		pad->Device->Poll() ;

		if( ( hr = pad->Device->GetDeviceState( sizeof( D_DIJOYSTATE ) , &InputState ) ) != D_DI_OK )
		{
			if( NS_ProcessMessage() != 0 ) return -1 ;
			if( DInputData.DirectInputObject == NULL ) return -1;

			if( pad->Device->Acquire() == D_DI_OK )
			{
				if( NS_ProcessMessage() ) return -1 ;
				if( DInputData.DirectInputObject == NULL ) return -1;

				if( pad->Device->GetDeviceState( sizeof( D_DIJOYSTATE ) , &InputState ) == D_DI_OK )
				{
					hr = D_DI_OK;
				}
			}
		}

		if( hr == D_DI_OK )
		{
			pad->State = InputState ;
		}

		// イベントのリセット
		if( WaitForSingleObject( pad->Event, 0 ) == WAIT_OBJECT_0 ) ResetEvent( pad->Event ) ;
	}
	else
	{
		// XInput を使用する場合

		// 前回の取得から 1msec も経過していない場合は以前の状態を返す
		Time = WinAPIData.Win32Func.timeGetTimeFunc() ;
		if( pad->GetTime == Time ) return 0 ;
		pad->GetTime = Time ;

		if( DInputData.XInputGetStateFunc == NULL || DInputData.XInputGetStateFunc( pad->XInputDeviceNo, &pad->XInputState ) != ERROR_SUCCESS )
		{
			// 取得できなかった場合は接続されていないということなので情報をゼロで埋めておく
			_MEMSET( &pad->XInputState, 0, sizeof( pad->XInputState ) ) ;
		}
		else
		{
			WORD Buttons ;

			// XInput用の入力値を DirectInput 用に変換する

			switch( pad->XInputState.Gamepad.sThumbLX )
			{
			case  32767 : pad->State.lX =  RANGE ; break ;
			case -32768 : pad->State.lX = -RANGE ; break ;
			default :
				     if( pad->XInputState.Gamepad.sThumbLX < -DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lX = -( ( -pad->XInputState.Gamepad.sThumbLX - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else if( pad->XInputState.Gamepad.sThumbLX >  DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lX =  ( (  pad->XInputState.Gamepad.sThumbLX - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else                                                                             pad->State.lX = 0 ;
				break ;
			}

			switch( pad->XInputState.Gamepad.sThumbLY )
			{
			case  32767 : pad->State.lY = -RANGE ; break ;
			case -32768 : pad->State.lY =  RANGE ; break ;
			default :
				     if( pad->XInputState.Gamepad.sThumbLY < -DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lY =  ( ( -pad->XInputState.Gamepad.sThumbLY - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else if( pad->XInputState.Gamepad.sThumbLY >  DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lY = -( (  pad->XInputState.Gamepad.sThumbLY - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else                                                                             pad->State.lY = 0 ;
				break ;
			}

			switch( pad->XInputState.Gamepad.sThumbRX )
			{
			case  32767 : pad->State.lRx =  RANGE ; break ;
			case -32768 : pad->State.lRx = -RANGE ; break ;
			default :
				     if( pad->XInputState.Gamepad.sThumbRX < -DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lRx = -( ( -pad->XInputState.Gamepad.sThumbRX - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else if( pad->XInputState.Gamepad.sThumbRX >  DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lRx =  ( (  pad->XInputState.Gamepad.sThumbRX - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else                                                                             pad->State.lRx = 0 ;
				break ;
			}

			switch( pad->XInputState.Gamepad.sThumbRY )
			{
			case  32767 : pad->State.lRy = -RANGE ; break ;
			case -32768 : pad->State.lRy =  RANGE ; break ;
			default :
				     if( pad->XInputState.Gamepad.sThumbRY < -DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lRy =  ( ( -pad->XInputState.Gamepad.sThumbRY - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else if( pad->XInputState.Gamepad.sThumbRY >  DEADZONE_XINPUT( pad->DeadZone ) ) pad->State.lRy = -( (  pad->XInputState.Gamepad.sThumbRY - DEADZONE_XINPUT( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT( pad->DeadZone ) ) ;
				else                                                                             pad->State.lRy = 0 ;
				break ;
			}

			Buttons = pad->XInputState.Gamepad.wButtons ;
			for( i = 0 ; i < XINPUT_TO_DIRECTINPUT_BUTTONNUM ; i ++ )
			{
				pad->State.rgbButtons[ i ] = ( Buttons & XInputButtonToDirectInputButtonNo[ i ] ) ? 0x80 : 0x00 ;
			}

			     if( ( Buttons & D_XINPUT_GAMEPAD_DPAD_LEFT  ) == 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_UP    ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_RIGHT ) == 0 ) pad->State.rgdwPOV[ 0 ] =     0 ;
			else if(                                                   ( Buttons & D_XINPUT_GAMEPAD_DPAD_UP    ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_RIGHT ) != 0 ) pad->State.rgdwPOV[ 0 ] =  4500 ;
			else if( ( Buttons & D_XINPUT_GAMEPAD_DPAD_UP    ) == 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_RIGHT ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_DOWN  ) == 0 ) pad->State.rgdwPOV[ 0 ] =  9000 ;
			else if(                                                   ( Buttons & D_XINPUT_GAMEPAD_DPAD_RIGHT ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_DOWN  ) != 0 ) pad->State.rgdwPOV[ 0 ] = 13500 ;
			else if( ( Buttons & D_XINPUT_GAMEPAD_DPAD_RIGHT ) == 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_DOWN  ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_LEFT  ) == 0 ) pad->State.rgdwPOV[ 0 ] = 18000 ;
			else if(                                                   ( Buttons & D_XINPUT_GAMEPAD_DPAD_DOWN  ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_LEFT  ) != 0 ) pad->State.rgdwPOV[ 0 ] = 22500 ;
			else if( ( Buttons & D_XINPUT_GAMEPAD_DPAD_DOWN  ) == 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_LEFT  ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_UP    ) == 0 ) pad->State.rgdwPOV[ 0 ] = 27000 ;
			else if(                                                   ( Buttons & D_XINPUT_GAMEPAD_DPAD_LEFT  ) != 0 && ( Buttons & D_XINPUT_GAMEPAD_DPAD_UP    ) != 0 ) pad->State.rgdwPOV[ 0 ] = 31500 ;
			else                                                                                                                                                          pad->State.rgdwPOV[ 0 ] = 0xffffffff ;
			pad->State.rgdwPOV[ 1 ] = 0xffffffff ;
			pad->State.rgdwPOV[ 2 ] = 0xffffffff ;
			pad->State.rgdwPOV[ 3 ] = 0xffffffff ;

			pad->State.lZ = 0 ;
			if( pad->XInputState.Gamepad.bLeftTrigger  > DEADZONE_XINPUT_TRIGGER( pad->DeadZone ) )
			{
				pad->State.lZ += ( pad->XInputState.Gamepad.bLeftTrigger  - DEADZONE_XINPUT_TRIGGER( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT_TRIGGER( pad->DeadZone ) ;
			}
			if( pad->XInputState.Gamepad.bRightTrigger > DEADZONE_XINPUT_TRIGGER( pad->DeadZone ) )
			{
				pad->State.lZ -= ( pad->XInputState.Gamepad.bRightTrigger - DEADZONE_XINPUT_TRIGGER( pad->DeadZone ) ) * RANGE / VALIDRANGE_XINPUT_TRIGGER( pad->DeadZone ) ;
			}
		}
	}

	// 終了
	return 0 ;
}

// キーボードの入力処理に DirectInput を使わないか、フラグをセットする
extern int NS_SetKeyboardNotDirectInputFlag( int Flag )
{
	DInputData.KeyboardNotDirectInputFlag = Flag ;

	// 終了
	return 0 ;
}

// 入力処理に DirectInput を使用するかどうかのフラグをセットする
extern int NS_SetUseDirectInputFlag( int Flag )
{
	DInputData.NoUseDirectInputFlag = !Flag ;
	DInputData.UseDirectInputFlag = Flag ;

	// 終了
	return 0 ;
}

// Xbox360コントローラの入力処理に XInput を使用するかどうかを設定する( TRUE:XInput を使用する( デフォルト )　　FALSE:XInput を使用しない )
extern int NS_SetUseXInputFlag(	int Flag )
{
	DInputData.NoUseXInputFlag = !Flag ;

	// 終了
	return 0 ;
}

// ジョイパッドの振動機能を使用するかどうかのフラグをセットする
extern int NS_SetUseJoypadVibrationFlag( int Flag )
{
	DInputData.NoUseVibrationFlag = !Flag ;

	// 終了
	return 0 ;
}


// ＤｉｒｅｃｔＩｎｐｕｔデバイスの取得状態を更新する
extern int RefreshInputDeviceAcquireState( void )
{
	int i, num ;
	DINPUTPAD *Pad ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return 0 ;

	// DirectInput を使用していない場合は何もせず終了
	if( DInputData.DirectInputObject == (D_IDirectInput7 *)(DWORD_PTR)0xffffffff ) return 0 ;

	// パッドデバイスの再取得
	num = DInputData.JoyPadNum ;
	Pad = DInputData.Joypad ;
	for( i = 0 ; i < num ; i ++, Pad ++ )
	{
		if( Pad->Device != NULL )
		{
			Pad->Device->Unacquire() ;

			// ソフトのアクティブ状態によって協調レベルを変更
			if( NS_GetActiveFlag() == TRUE )
			{
				Pad->Device->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_FOREGROUND | D_DISCL_EXCLUSIVE ) ;
			}
			else
			{
				Pad->Device->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DISCL_BACKGROUND | D_DISCL_NONEXCLUSIVE ) ;
			}

			Pad->Device->Acquire() ;
		}
	}

	// キーボードデバイスの再取得
	if( DInputData.KeyboardDeviceObject != NULL )
	{
		DInputData.KeyboardDeviceObject->Unacquire() ;
		DInputData.KeyboardDeviceObject->Acquire() ;
	}

	// 終了
	return 0 ;
}

// パッドのエフェクト処理に関するフレーム処理を行う
extern int JoypadEffectProcess( void )
{
	int i, num, time ;
	DINPUTPAD *Pad ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return 0 ;

	// ＤｉｒｅｃｔＩｎｐｕｔを使用していない場合はエフェクト処理は出来ないので終了
	if( DInputData.DirectInputObject == (D_IDirectInput7 *)(DWORD_PTR)0xffffffff ) return 0 ;

	// パッドデバイスの再取得
	num = DInputData.JoyPadNum ;
	Pad = DInputData.Joypad ;
	time = NS_GetNowCount() ;
	for( i = 0 ; i < num ; i ++, Pad ++ )
	{
		if( Pad->EffectPlayFlag == FALSE || ( Pad->XInputDeviceNo < 0 && Pad->Device == NULL ) ) continue ;

		// 無限再生の場合は特に何もしない
		if( Pad->EffectTime < 0 ) continue ;

		// 前回から時刻が変わっていなかったら何もしない
		if( Pad->EffectBackTime == time ) continue ;

		// 経過時間の加算
		if( time < Pad->EffectBackTime ) Pad->EffectBackTime = time ;
		Pad->EffectCompTime += time - Pad->EffectBackTime ;

		// 今回の時刻を保存
		Pad->EffectBackTime = time ;

		// 再生停止のチェック
		if( Pad->EffectCompTime >= Pad->EffectTime )
		{
			NS_StopJoypadVibration( i + 1 ) ;
		}
	}

	// 再生状態の更新
	RefreshEffectPlayState() ;

	// 終了
	return 0 ;
}

// パッドエフェクトの再生状態を更新する(ソフトが非アクティブになることもあるため)
extern int RefreshEffectPlayState( void )
{
	int i, num, time, PlayFlag, Flag ;
	DINPUTPAD *Pad ;
	D_XINPUT_VIBRATION VibState ;

	// 初期化判定
	if( DInputData.DirectInputObject == NULL && NS_GetWindowCloseFlag() == FALSE )
		return 0 ;

	// ＤｉｒｅｃｔＩｎｐｕｔを使用していない場合はエフェクト処理は出来ないので終了
	if( DInputData.DirectInputObject == (D_IDirectInput7 *)(DWORD_PTR)0xffffffff ) return 0 ;

	// パッドデバイスの再取得
	num = DInputData.JoyPadNum ;
	Pad = DInputData.Joypad ;
	time = NS_GetNowCount() ;

	// ソフトのアクティブ状態によって処理を分岐
	if( NS_GetActiveFlag() == TRUE )
	{
		// ソフトがアクティブな場合
		for( i = 0 ; i < num ; i ++, Pad ++ )
		{
			if( Pad->XInputDeviceNo >= 0 )
			{
				VibState.wLeftMotorSpeed  = 0 ;
				VibState.wRightMotorSpeed = 0 ;
				Flag = 0 ;
				
				if( Pad->EffectPlayStateLeft != Pad->EffectPlayFlag )
				{
					if( Pad->EffectPlayFlag == TRUE )
					{
						VibState.wLeftMotorSpeed = ( WORD )( Pad->EffectPower > D_DI_FFNOMINALMAX / 2 ? 65535 : Pad->EffectPower * 65535 / ( D_DI_FFNOMINALMAX / 2 ) ) ;
					}
					Pad->EffectPlayStateLeft = Pad->EffectPlayFlag ;
					Flag = 1 ;
				}

				PlayFlag = Pad->EffectPlayFlag == TRUE && Pad->EffectPower > D_DI_FFNOMINALMAX / 2 ;
				if( PlayFlag != Pad->EffectPlayStateRight )
				{
					if( PlayFlag == TRUE )
					{
						VibState.wRightMotorSpeed = ( WORD )( ( Pad->EffectPower - D_DI_FFNOMINALMAX / 2 ) * 65535 / ( D_DI_FFNOMINALMAX / 2 ) ) ;
					}
					Pad->EffectPlayStateRight = PlayFlag ;
					Flag = 1 ;
				}

				if( DInputData.XInputSetStateFunc != NULL && Flag == 1 )
				{
					DInputData.XInputSetStateFunc( Pad->XInputDeviceNo, &VibState ) ;
				}
			}
			else
			{
				if( Pad->Device == NULL ) continue ;

				if( Pad->LeftEffect != NULL && Pad->EffectPlayStateLeft != Pad->EffectPlayFlag )
				{
					if( Pad->EffectPlayFlag == TRUE )	Pad->LeftEffect->Start( 1, 0 ) ;
					else								Pad->LeftEffect->Stop() ;
					Pad->EffectPlayStateLeft = Pad->EffectPlayFlag ;
				}

				PlayFlag = Pad->EffectPlayFlag == TRUE && Pad->EffectPower > D_DI_FFNOMINALMAX / 2 ;
				if( Pad->RightEffect != NULL && PlayFlag != Pad->EffectPlayStateRight )
				{
					if( PlayFlag == TRUE )				Pad->RightEffect->Start( 1, 0 ) ;
					else								Pad->RightEffect->Stop() ;
					Pad->EffectPlayStateRight = PlayFlag ;
				}
			}
		}
	}
	else
	{
		// ソフトが非アクティブな場合
		for( i = 0 ; i < num ; i ++, Pad ++ )
		{
			if( Pad->XInputDeviceNo >= 0 )
			{
				VibState.wLeftMotorSpeed  = 0 ;
				VibState.wRightMotorSpeed = 0 ;
				Flag = 0 ;

				if( Pad->LeftEffect  != NULL && Pad->EffectPlayStateLeft  != FALSE ) Flag = 1 ;
				if( Pad->RightEffect != NULL && Pad->EffectPlayStateRight != FALSE ) Flag = 1 ;

				if( DInputData.XInputSetStateFunc != NULL && Flag == 1 )
				{
					DInputData.XInputSetStateFunc( Pad->XInputDeviceNo, &VibState ) ;
				}
			}
			else
			{
				if( Pad->Device == NULL ) continue ;

				if( Pad->LeftEffect  != NULL && Pad->EffectPlayStateLeft  != FALSE ) Pad->LeftEffect->Stop() ;
				if( Pad->RightEffect != NULL && Pad->EffectPlayStateRight != FALSE ) Pad->RightEffect->Stop() ;
			}
			Pad->EffectPlayStateLeft  = FALSE ;
			Pad->EffectPlayStateRight = FALSE ;
		}
	}

	// 終了
	return 0 ;
}

// キーボードデバイスのデータフォーマット定義 -----------------------

// ＧＵＩＤ
GUID GUIDDIKEYBOARD =
{
//	Data1		Data2	Data3	Data4
	0x55728220,	0xd33c,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00", 
} ;

// デバイスオブジェクトデータ
D_DIOBJECTDATAFORMAT C_ODFDIKEYBOARD[256] =
{
//		pguid				dwOfs	dwType 		dwFlags
	{ 	&GUIDDIKEYBOARD, 	0x00,	0x8000000c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x01,	0x8000010c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x02,	0x8000020c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x03,	0x8000030c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x04,	0x8000040c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x05,	0x8000050c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x06,	0x8000060c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x07,	0x8000070c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x08,	0x8000080c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x09,	0x8000090c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x0a,	0x80000a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x0b,	0x80000b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x0c,	0x80000c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x0d,	0x80000d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x0e,	0x80000e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x0f,	0x80000f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x10,	0x8000100c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x11,	0x8000110c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x12,	0x8000120c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x13,	0x8000130c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x14,	0x8000140c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x15,	0x8000150c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x16,	0x8000160c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x17,	0x8000170c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x18,	0x8000180c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x19,	0x8000190c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x1a,	0x80001a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x1b,	0x80001b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x1c,	0x80001c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x1d,	0x80001d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x1e,	0x80001e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x1f,	0x80001f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x20,	0x8000200c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x21,	0x8000210c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x22,	0x8000220c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x23,	0x8000230c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x24,	0x8000240c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x25,	0x8000250c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x26,	0x8000260c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x27,	0x8000270c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x28,	0x8000280c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x29,	0x8000290c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x2a,	0x80002a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x2b,	0x80002b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x2c,	0x80002c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x2d,	0x80002d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x2e,	0x80002e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x2f,	0x80002f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x30,	0x8000300c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x31,	0x8000310c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x32,	0x8000320c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x33,	0x8000330c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x34,	0x8000340c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x35,	0x8000350c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x36,	0x8000360c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x37,	0x8000370c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x38,	0x8000380c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x39,	0x8000390c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x3a,	0x80003a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x3b,	0x80003b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x3c,	0x80003c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x3d,	0x80003d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x3e,	0x80003e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x3f,	0x80003f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x40,	0x8000400c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x41,	0x8000410c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x42,	0x8000420c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x43,	0x8000430c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x44,	0x8000440c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x45,	0x8000450c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x46,	0x8000460c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x47,	0x8000470c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x48,	0x8000480c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x49,	0x8000490c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x4a,	0x80004a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x4b,	0x80004b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x4c,	0x80004c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x4d,	0x80004d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x4e,	0x80004e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x4f,	0x80004f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x50,	0x8000500c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x51,	0x8000510c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x52,	0x8000520c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x53,	0x8000530c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x54,	0x8000540c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x55,	0x8000550c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x56,	0x8000560c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x57,	0x8000570c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x58,	0x8000580c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x59,	0x8000590c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x5a,	0x80005a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x5b,	0x80005b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x5c,	0x80005c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x5d,	0x80005d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x5e,	0x80005e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x5f,	0x80005f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x60,	0x8000600c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x61,	0x8000610c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x62,	0x8000620c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x63,	0x8000630c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x64,	0x8000640c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x65,	0x8000650c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x66,	0x8000660c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x67,	0x8000670c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x68,	0x8000680c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x69,	0x8000690c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x6a,	0x80006a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x6b,	0x80006b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x6c,	0x80006c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x6d,	0x80006d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x6e,	0x80006e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x6f,	0x80006f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x70,	0x8000700c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x71,	0x8000710c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x72,	0x8000720c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x73,	0x8000730c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x74,	0x8000740c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x75,	0x8000750c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x76,	0x8000760c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x77,	0x8000770c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x78,	0x8000780c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x79,	0x8000790c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x7a,	0x80007a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x7b,	0x80007b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x7c,	0x80007c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x7d,	0x80007d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x7e,	0x80007e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x7f,	0x80007f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x80,	0x8000800c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x81,	0x8000810c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x82,	0x8000820c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x83,	0x8000830c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x84,	0x8000840c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x85,	0x8000850c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x86,	0x8000860c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x87,	0x8000870c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x88,	0x8000880c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x89,	0x8000890c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x8a,	0x80008a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x8b,	0x80008b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x8c,	0x80008c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x8d,	0x80008d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x8e,	0x80008e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x8f,	0x80008f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0x90,	0x8000900c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x91,	0x8000910c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x92,	0x8000920c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x93,	0x8000930c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x94,	0x8000940c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x95,	0x8000950c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x96,	0x8000960c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x97,	0x8000970c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x98,	0x8000980c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x99,	0x8000990c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x9a,	0x80009a0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x9b,	0x80009b0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x9c,	0x80009c0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x9d,	0x80009d0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x9e,	0x80009e0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0x9f,	0x80009f0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0xa0,	0x8000a00c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa1,	0x8000a10c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa2,	0x8000a20c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa3,	0x8000a30c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa4,	0x8000a40c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa5,	0x8000a50c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa6,	0x8000a60c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa7,	0x8000a70c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa8,	0x8000a80c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xa9,	0x8000a90c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xaa,	0x8000aa0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xab,	0x8000ab0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xac,	0x8000ac0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xad,	0x8000ad0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xae,	0x8000ae0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xaf,	0x8000af0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0xb0,	0x8000b00c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb1,	0x8000b10c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb2,	0x8000b20c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb3,	0x8000b30c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb4,	0x8000b40c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb5,	0x8000b50c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb6,	0x8000b60c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb7,	0x8000b70c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb8,	0x8000b80c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xb9,	0x8000b90c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xba,	0x8000ba0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xbb,	0x8000bb0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xbc,	0x8000bc0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xbd,	0x8000bd0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xbe,	0x8000be0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xbf,	0x8000bf0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0xc0,	0x8000c00c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc1,	0x8000c10c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc2,	0x8000c20c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc3,	0x8000c30c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc4,	0x8000c40c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc5,	0x8000c50c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc6,	0x8000c60c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc7,	0x8000c70c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc8,	0x8000c80c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xc9,	0x8000c90c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xca,	0x8000ca0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xcb,	0x8000cb0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xcc,	0x8000cc0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xcd,	0x8000cd0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xce,	0x8000ce0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xcf,	0x8000cf0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0xd0,	0x8000d00c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd1,	0x8000d10c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd2,	0x8000d20c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd3,	0x8000d30c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd4,	0x8000d40c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd5,	0x8000d50c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd6,	0x8000d60c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd7,	0x8000d70c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd8,	0x8000d80c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xd9,	0x8000d90c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xda,	0x8000da0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xdb,	0x8000db0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xdc,	0x8000dc0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xdd,	0x8000dd0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xde,	0x8000de0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xdf,	0x8000df0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0xe0,	0x8000e00c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe1,	0x8000e10c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe2,	0x8000e20c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe3,	0x8000e30c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe4,	0x8000e40c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe5,	0x8000e50c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe6,	0x8000e60c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe7,	0x8000e70c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe8,	0x8000e80c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xe9,	0x8000e90c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xea,	0x8000ea0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xeb,	0x8000eb0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xec,	0x8000ec0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xed,	0x8000ed0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xee,	0x8000ee0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xef,	0x8000ef0c,	0x0 },

	{ 	&GUIDDIKEYBOARD, 	0xf0,	0x8000f00c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf1,	0x8000f10c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf2,	0x8000f20c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf3,	0x8000f30c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf4,	0x8000f40c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf5,	0x8000f50c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf6,	0x8000f60c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf7,	0x8000f70c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf8,	0x8000f80c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xf9,	0x8000f90c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xfa,	0x8000fa0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xfb,	0x8000fb0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xfc,	0x8000fc0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xfd,	0x8000fd0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xfe,	0x8000fe0c,	0x0 },
	{ 	&GUIDDIKEYBOARD, 	0xff,	0x8000ff0c,	0x0 },
} ;

// デバイスデータフォーマット
D_DIDATAFORMAT C_DFDIKEYBOARD =
{
#ifdef _WIN64
	32, 	// dwSize
	24,		// dwObjSize
#else
	24, 	// dwSize
	16,		// dwObjSize
#endif
	0x02,	// dwFlags
	256,	// dwDataSize
	256,	// dwNumObjs
	C_ODFDIKEYBOARD, // rgodf
} ;


// ジョイスティックデバイスのデータフォーマット定義 -----------------

// ＧＵＩＤ
GUID GUID_X_AXIS	= { 0xa36d02e0, 0xc9f3, 0x11cf, "\xbf\xc7\x44\x45\x53\x54\x00" };
GUID GUID_Y_AXIS	= { 0xa36d02e1,	0xc9f3,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00" };
GUID GUID_Z_AXIS	= { 0xa36d02e2,	0xc9f3,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00" };
GUID GUID_RX_AXIS	= { 0xa36d02f4,	0xc9f3,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00" };
GUID GUID_RY_AXIS	= { 0xa36d02f5,	0xc9f3,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00" };
GUID GUID_RZ_AXIS	= { 0xa36d02e3,	0xc9f3,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00" };
GUID GUID_SLIDER	= { 0xa36d02e4,	0xc9f3,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00" };
GUID GUID_POV		= { 0xa36d02f2,	0xc9f3,	0x11cf,	"\xbf\xc7\x44\x45\x53\x54\x00" };

// ジョイスティック１デバイスオブジェクトデータ
D_DIOBJECTDATAFORMAT C_ODFDIJOYSTICK[44] =
{
//	pguid				dwOfs	dwType		dwFlags
	&GUID_X_AXIS,		0x00,	0x80ffff03,	0x100,
	&GUID_Y_AXIS,		0x04,	0x80ffff03,	0x100,
	&GUID_Z_AXIS,		0x08,	0x80ffff03,	0x100,
	&GUID_RX_AXIS,		0x0c,	0x80ffff03,	0x100,
	&GUID_RY_AXIS,		0x10,	0x80ffff03,	0x100,
	&GUID_RZ_AXIS,		0x14,	0x80ffff03,	0x100,
	&GUID_SLIDER,		0x18,	0x80ffff03,	0x100,
	&GUID_SLIDER,		0x1c,	0x80ffff03,	0x100,
	&GUID_POV,			0x20,	0x80ffff10,	0x000,
	&GUID_POV,			0x24,	0x80ffff10,	0x000,
	&GUID_POV,			0x28,	0x80ffff10,	0x000,
	&GUID_POV,			0x2c,	0x80ffff10,	0x000,
	NULL,				0x30,	0x80ffff0c,	0x000,
	NULL,				0x31,	0x80ffff0c,	0x000,
	NULL,				0x32,	0x80ffff0c,	0x000,
	NULL,				0x33,	0x80ffff0c,	0x000,
	NULL,				0x34,	0x80ffff0c,	0x000,
	NULL,				0x35,	0x80ffff0c,	0x000,
	NULL,				0x36,	0x80ffff0c,	0x000,
	NULL,				0x37,	0x80ffff0c,	0x000,
	NULL,				0x38,	0x80ffff0c,	0x000,
	NULL,				0x39,	0x80ffff0c,	0x000,
	NULL,				0x3a,	0x80ffff0c,	0x000,
	NULL,				0x3b,	0x80ffff0c,	0x000,
	NULL,				0x3c,	0x80ffff0c,	0x000,
	NULL,				0x3d,	0x80ffff0c,	0x000,
	NULL,				0x3e,	0x80ffff0c,	0x000,
	NULL,				0x3f,	0x80ffff0c,	0x000,
	NULL,				0x40,	0x80ffff0c,	0x000,
	NULL,				0x41,	0x80ffff0c,	0x000,
	NULL,				0x42,	0x80ffff0c,	0x000,
	NULL,				0x43,	0x80ffff0c,	0x000,
	NULL,				0x44,	0x80ffff0c,	0x000,
	NULL,				0x45,	0x80ffff0c,	0x000,
	NULL,				0x46,	0x80ffff0c,	0x000,
	NULL,				0x47,	0x80ffff0c,	0x000,
	NULL,				0x48,	0x80ffff0c,	0x000,
	NULL,				0x49,	0x80ffff0c,	0x000,
	NULL,				0x4a,	0x80ffff0c,	0x000,
	NULL,				0x4b,	0x80ffff0c,	0x000,
	NULL,				0x4c,	0x80ffff0c,	0x000,
	NULL,				0x4d,	0x80ffff0c,	0x000,
	NULL,				0x4e,	0x80ffff0c,	0x000,
	NULL,				0x4f,	0x80ffff0c,	0x000,
} ;

// ジョイスティック１デバイスデータフォーマット
D_DIDATAFORMAT C_DFDIJOYSTICK =
{
#ifdef _WIN64
	32, 	// dwSize
	24,		// dwObjSize
#else
	24, 	// dwSize
	16,		// dwObjSize
#endif
	0x01,	// dwFlags
	80,		// dwDataSize
	44,		// dwNumObjs
	C_ODFDIJOYSTICK, // rgodf
} ;

// ジョイスティック２デバイスオブジェクトデータ
D_DIOBJECTDATAFORMAT C_ODFDIJOYSTICK2[164] =
{
//	pguid				dwOfs	dwType		dwFlags
	&GUID_X_AXIS,		0x00,	0x80ffff03,	0x100,
	&GUID_Y_AXIS,		0x04,	0x80ffff03,	0x100,
	&GUID_Z_AXIS,		0x08,	0x80ffff03,	0x100,
	&GUID_RX_AXIS,		0x0c,	0x80ffff03,	0x100,
	&GUID_RY_AXIS,		0x10,	0x80ffff03,	0x100,
	&GUID_RZ_AXIS,		0x14,	0x80ffff03,	0x100,
	&GUID_SLIDER,		0x18,	0x80ffff03,	0x100,
	&GUID_SLIDER,		0x1c,	0x80ffff03,	0x100,
	&GUID_POV,			0x20,	0x80ffff10,	0x000,
	&GUID_POV,			0x24,	0x80ffff10,	0x000,
	&GUID_POV,			0x28,	0x80ffff10,	0x000,
	&GUID_POV,			0x2c,	0x80ffff10,	0x000,
	NULL,				0x30,	0x80ffff0c,	0x000,
	NULL,				0x31,	0x80ffff0c,	0x000,
	NULL,				0x32,	0x80ffff0c,	0x000,
	NULL,				0x33,	0x80ffff0c,	0x000,
	NULL,				0x34,	0x80ffff0c,	0x000,
	NULL,				0x35,	0x80ffff0c,	0x000,
	NULL,				0x36,	0x80ffff0c,	0x000,
	NULL,				0x37,	0x80ffff0c,	0x000,
	NULL,				0x38,	0x80ffff0c,	0x000,
	NULL,				0x39,	0x80ffff0c,	0x000,
	NULL,				0x3a,	0x80ffff0c,	0x000,
	NULL,				0x3b,	0x80ffff0c,	0x000,
	NULL,				0x3c,	0x80ffff0c,	0x000,
	NULL,				0x3d,	0x80ffff0c,	0x000,
	NULL,				0x3e,	0x80ffff0c,	0x000,
	NULL,				0x3f,	0x80ffff0c,	0x000,
	NULL,				0x40,	0x80ffff0c,	0x000,
	NULL,				0x41,	0x80ffff0c,	0x000,
	NULL,				0x42,	0x80ffff0c,	0x000,
	NULL,				0x43,	0x80ffff0c,	0x000,
	NULL,				0x44,	0x80ffff0c,	0x000,
	NULL,				0x45,	0x80ffff0c,	0x000,
	NULL,				0x46,	0x80ffff0c,	0x000,
	NULL,				0x47,	0x80ffff0c,	0x000,
	NULL,				0x48,	0x80ffff0c,	0x000,
	NULL,				0x49,	0x80ffff0c,	0x000,
	NULL,				0x4a,	0x80ffff0c,	0x000,
	NULL,				0x4b,	0x80ffff0c,	0x000,
	NULL,				0x4c,	0x80ffff0c,	0x000,
	NULL,				0x4d,	0x80ffff0c,	0x000,
	NULL,				0x4e,	0x80ffff0c,	0x000,
	NULL,				0x4f,	0x80ffff0c,	0x000,
	NULL,				0x50,	0x80ffff0c,	0x000,
	NULL,				0x51,	0x80ffff0c,	0x000,
	NULL,				0x52,	0x80ffff0c,	0x000,
	NULL,				0x53,	0x80ffff0c,	0x000,
	NULL,				0x54,	0x80ffff0c,	0x000,
	NULL,				0x55,	0x80ffff0c,	0x000,
	NULL,				0x56,	0x80ffff0c,	0x000,
	NULL,				0x57,	0x80ffff0c,	0x000,
	NULL,				0x58,	0x80ffff0c,	0x000,
	NULL,				0x59,	0x80ffff0c,	0x000,
	NULL,				0x5a,	0x80ffff0c,	0x000,
	NULL,				0x5b,	0x80ffff0c,	0x000,
	NULL,				0x5c,	0x80ffff0c,	0x000,
	NULL,				0x5d,	0x80ffff0c,	0x000,
	NULL,				0x5e,	0x80ffff0c,	0x000,
	NULL,				0x5f,	0x80ffff0c,	0x000,
	NULL,				0x60,	0x80ffff0c,	0x000,
	NULL,				0x61,	0x80ffff0c,	0x000,
	NULL,				0x62,	0x80ffff0c,	0x000,
	NULL,				0x63,	0x80ffff0c,	0x000,
	NULL,				0x64,	0x80ffff0c,	0x000,
	NULL,				0x65,	0x80ffff0c,	0x000,
	NULL,				0x66,	0x80ffff0c,	0x000,
	NULL,				0x67,	0x80ffff0c,	0x000,
	NULL,				0x68,	0x80ffff0c,	0x000,
	NULL,				0x69,	0x80ffff0c,	0x000,
	NULL,				0x6a,	0x80ffff0c,	0x000,
	NULL,				0x6b,	0x80ffff0c,	0x000,
	NULL,				0x6c,	0x80ffff0c,	0x000,
	NULL,				0x6d,	0x80ffff0c,	0x000,
	NULL,				0x6e,	0x80ffff0c,	0x000,
	NULL,				0x6f,	0x80ffff0c,	0x000,
	NULL,				0x70,	0x80ffff0c,	0x000,
	NULL,				0x71,	0x80ffff0c,	0x000,
	NULL,				0x72,	0x80ffff0c,	0x000,
	NULL,				0x73,	0x80ffff0c,	0x000,
	NULL,				0x74,	0x80ffff0c,	0x000,
	NULL,				0x75,	0x80ffff0c,	0x000,
	NULL,				0x76,	0x80ffff0c,	0x000,
	NULL,				0x77,	0x80ffff0c,	0x000,
	NULL,				0x78,	0x80ffff0c,	0x000,
	NULL,				0x79,	0x80ffff0c,	0x000,
	NULL,				0x7a,	0x80ffff0c,	0x000,
	NULL,				0x7b,	0x80ffff0c,	0x000,
	NULL,				0x7c,	0x80ffff0c,	0x000,
	NULL,				0x7d,	0x80ffff0c,	0x000,
	NULL,				0x7e,	0x80ffff0c,	0x000,
	NULL,				0x7f,	0x80ffff0c,	0x000,
	NULL,				0x80,	0x80ffff0c,	0x000,
	NULL,				0x81,	0x80ffff0c,	0x000,
	NULL,				0x82,	0x80ffff0c,	0x000,
	NULL,				0x83,	0x80ffff0c,	0x000,
	NULL,				0x84,	0x80ffff0c,	0x000,
	NULL,				0x85,	0x80ffff0c,	0x000,
	NULL,				0x86,	0x80ffff0c,	0x000,
	NULL,				0x87,	0x80ffff0c,	0x000,
	NULL,				0x88,	0x80ffff0c,	0x000,
	NULL,				0x89,	0x80ffff0c,	0x000,
	NULL,				0x8a,	0x80ffff0c,	0x000,
	NULL,				0x8b,	0x80ffff0c,	0x000,
	NULL,				0x8c,	0x80ffff0c,	0x000,
	NULL,				0x8d,	0x80ffff0c,	0x000,
	NULL,				0x8e,	0x80ffff0c,	0x000,
	NULL,				0x8f,	0x80ffff0c,	0x000,
	NULL,				0x90,	0x80ffff0c,	0x000,
	NULL,				0x91,	0x80ffff0c,	0x000,
	NULL,				0x92,	0x80ffff0c,	0x000,
	NULL,				0x93,	0x80ffff0c,	0x000,
	NULL,				0x94,	0x80ffff0c,	0x000,
	NULL,				0x95,	0x80ffff0c,	0x000,
	NULL,				0x96,	0x80ffff0c,	0x000,
	NULL,				0x97,	0x80ffff0c,	0x000,
	NULL,				0x98,	0x80ffff0c,	0x000,
	NULL,				0x99,	0x80ffff0c,	0x000,
	NULL,				0x9a,	0x80ffff0c,	0x000,
	NULL,				0x9b,	0x80ffff0c,	0x000,
	NULL,				0x9c,	0x80ffff0c,	0x000,
	NULL,				0x9d,	0x80ffff0c,	0x000,
	NULL,				0x9e,	0x80ffff0c,	0x000,
	NULL,				0x9f,	0x80ffff0c,	0x000,
	NULL,				0xa0,	0x80ffff0c,	0x000,
	NULL,				0xa1,	0x80ffff0c,	0x000,
	NULL,				0xa2,	0x80ffff0c,	0x000,
	NULL,				0xa3,	0x80ffff0c,	0x000,
	NULL,				0xa4,	0x80ffff0c,	0x000,
	NULL,				0xa5,	0x80ffff0c,	0x000,
	NULL,				0xa6,	0x80ffff0c,	0x000,
	NULL,				0xa7,	0x80ffff0c,	0x000,
	NULL,				0xa8,	0x80ffff0c,	0x000,
	NULL,				0xa9,	0x80ffff0c,	0x000,
	NULL,				0xaa,	0x80ffff0c,	0x000,
	NULL,				0xab,	0x80ffff0c,	0x000,
	NULL,				0xac,	0x80ffff0c,	0x000,
	NULL,				0xad,	0x80ffff0c,	0x000,
	NULL,				0xae,	0x80ffff0c,	0x000,
	NULL,				0xaf,	0x80ffff0c,	0x000,
	&GUID_X_AXIS,		0xb0,	0x80ffff03,	0x200,
	&GUID_Y_AXIS,		0xb4,	0x80ffff03,	0x200,
	&GUID_Z_AXIS,		0xb8,	0x80ffff03,	0x200,
	&GUID_RX_AXIS,		0xbc,	0x80ffff03,	0x200,
	&GUID_RY_AXIS,		0xc0,	0x80ffff03,	0x200,
	&GUID_RZ_AXIS,		0xc4,	0x80ffff03,	0x200,
	&GUID_SLIDER,		0x18,	0x80ffff03,	0x200,
	&GUID_SLIDER,		0x1c,	0x80ffff03,	0x200,
	&GUID_X_AXIS,		0xd0,	0x80ffff03,	0x300,
	&GUID_Y_AXIS,		0xd4,	0x80ffff03,	0x300,
	&GUID_Z_AXIS,		0xd8,	0x80ffff03,	0x300,
	&GUID_RX_AXIS,		0xdc,	0x80ffff03,	0x300,
	&GUID_RY_AXIS,		0xe0,	0x80ffff03,	0x300,
	&GUID_RZ_AXIS,		0xe4,	0x80ffff03,	0x300,
	&GUID_SLIDER,		0x18,	0x80ffff03,	0x300,
	&GUID_SLIDER,		0x1c,	0x80ffff03,	0x300,
	&GUID_X_AXIS,		0xf0,	0x80ffff03,	0x400,
	&GUID_Y_AXIS,		0xf4,	0x80ffff03,	0x400,
	&GUID_Z_AXIS,		0xf8,	0x80ffff03,	0x400,
	&GUID_RX_AXIS,		0xfc,	0x80ffff03,	0x400,
	&GUID_RY_AXIS,		0x00,	0x80ffff03,	0x400,
	&GUID_RZ_AXIS,		0x04,	0x80ffff03,	0x400,
	&GUID_SLIDER,		0x18,	0x80ffff03,	0x400,
	&GUID_SLIDER,		0x1c,	0x80ffff03,	0x400,
} ;

// ジョイスティック２デバイスデータフォーマット
D_DIDATAFORMAT C_DFDIJOYSTICK2 =
{
#ifdef _WIN64
	32, 	// dwSize
	24,		// dwObjSize
#else
	24, 	// dwSize
	16,		// dwObjSize
#endif
	0x01,	// dwFlags
	272,	// dwDataSize
	164,	// dwNumObjs
	C_ODFDIJOYSTICK2, // rgodf
} ;


GUID GUIDDIMOUSE2[3] =
{
	0xa36d02e0, 0xc9f3, 0x11cf, 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00, 
	0xa36d02e1, 0xc9f3, 0x11cf, 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00, 
	0xa36d02e2, 0xc9f3, 0x11cf, 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00, 
} ;

D_DIOBJECTDATAFORMAT C_ODFDIMOUSE2[11] =
{
	&GUIDDIMOUSE2[0],	0x00, 0x00ffff03, 0x000,
	&GUIDDIMOUSE2[1],	0x04, 0x00ffff03, 0x000,
	&GUIDDIMOUSE2[2],	0x08, 0x80ffff03, 0x000,
	NULL,				0x0c, 0x00ffff0c, 0x000,
	NULL,				0x0d, 0x00ffff0c, 0x000,
	NULL,				0x0e, 0x80ffff0c, 0x000,
	NULL, 				0x0f, 0x80ffff0c, 0x000,
	NULL, 				0x10, 0x80ffff0c, 0x000,
	NULL, 				0x11, 0x80ffff0c, 0x000,
	NULL, 				0x12, 0x80ffff0c, 0x000,
	NULL, 				0x13, 0x80ffff0c, 0x000,
} ;

D_DIDATAFORMAT C_DFDIMOUSE2 =
{
#ifdef _WIN64
	32, 	// dwSize
	24,		// dwObjSize
#else
	24, 	// dwSize
	16,		// dwObjSize
#endif
	2,		// dwFlags
	20,		// dwDataSize
	11,		// dwNumObjs
	C_ODFDIMOUSE2,
} ;


GUID GUIDDIMOUSE[3] =
{
	0xa36d02e0, 0xc9f3, 0x11cf, 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00, 
	0xa36d02e1, 0xc9f3, 0x11cf, 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00, 
	0xa36d02e2, 0xc9f3, 0x11cf, 0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00, 
} ;

D_DIOBJECTDATAFORMAT C_ODFDIMOUSE[7] =
{
	&GUIDDIMOUSE[0],	0x00, 0x00ffff03, 0x000,
	&GUIDDIMOUSE[1],	0x04, 0x00ffff03, 0x000,
	&GUIDDIMOUSE[2],	0x08, 0x80ffff03, 0x000,
	NULL,				0x0c, 0x00ffff0c, 0x000,
	NULL,				0x0d, 0x00ffff0c, 0x000,
	NULL,				0x0e, 0x80ffff0c, 0x000,
	NULL, 				0x0f, 0x80ffff0c, 0x000,
} ;

D_DIDATAFORMAT C_DFDIMOUSE =
{
#ifdef _WIN64
	32, 	// dwSize
	24,		// dwObjSize
#else
	24, 	// dwSize
	16,		// dwObjSize
#endif
	2,		// dwFlags
	16,		// dwDataSize
	7,		// dwNumObjs
	C_ODFDIMOUSE,
} ;

}

#endif // DX_NON_INPUT
