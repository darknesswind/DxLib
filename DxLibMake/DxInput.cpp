﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ＤｉｒｅｃｔＩｎｐｕｔ制御プログラム
// 
// 				Ver 3.14d
// 
// -------------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

#include "DxInput.h"

#ifndef DX_NON_INPUT

// インクルード ------------------------------------------------------------------
#include "DxLib.h"
#include "DxSystem.h"
#include "DxLog.h"

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 -------------------------------------

#define DX_FFNOMINALMAX						(10000)

// 構造体定義 --------------------------------------------------------------------

// 定数定義 ----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 入力システム用データ
INPUTSYSTEMDATA InputSysData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

static	int UpdateJoypadInputState( int padno ) ;								// パッドの入力状態の更新

// プログラム --------------------------------------------------------------------

// 入力システムの初期化、終了関数

// 入力システムを初期化する
extern int InitializeInputSystem( void )
{
	// 既に初期化されていたら何もせずに終了
	if( InputSysData.InitializeFlag )
	{
		return 0 ;
	}

	// 初期化完了フラグを立てる
	InputSysData.InitializeFlag = TRUE ;

	// 環境依存処理
	if( InitializeInputSystem_PF_Timing0() != 0 )
	{
		// 初期化完了フラグを倒す
		InputSysData.InitializeFlag = FALSE ;

		return -1 ;
	}

	// 正常終了
	return 0 ;
}

// 入力システムの使用を終了する
extern int TerminateInputSystem( void )
{
	// 既に後始末が終わっていたら何もせずに終了
	if( InputSysData.InitializeFlag == FALSE )
	{
		return 0 ;
	}

	// 環境依存処理
	if( TerminateInputSystem_PF_Timing0() != 0 )
	{
		return -1 ;
	}

	// 初期化完了フラグを倒す
	InputSysData.InitializeFlag = FALSE ;

	// 正常終了
	return 0 ;
}

// ジョイパッドのセットアップを行う
extern int SetupJoypad( void )
{
	// 初期化判定
	if( InputSysData.InitializeFlag == FALSE )
	{
		return -1 ;
	}

	// 環境依存処理
	if( SetupJoypad_PF() != 0 )
	{
		return -1 ;
	}

	DXST_ERRORLOG_ADDUTF16LE( "\xb8\x30\xe7\x30\xa4\x30\xd1\x30\xc3\x30\xc9\x30\x6e\x30\x1d\x52\x1f\x67\x16\x53\x6f\x30\x63\x6b\x38\x5e\x6b\x30\x42\x7d\x86\x4e\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"ジョイパッドの初期化は正常に終了しました\n" @*/ ) ;

	// 終了
	return 0 ;
}

// ジョイパッドの後始末を行う
extern int TerminateJoypad( void )
{
	// 初期化判定
	if( InputSysData.InitializeFlag == FALSE )
	{
		return -1 ;
	}

	// 環境依存処理
	if( TerminateJoypad_PF() != 0 )
	{
		return -1 ;
	}

	// 終了
	return 0 ;
}









// F10 が押されたことにするフラグを立てる
extern int SetF10Input( void )
{
	InputSysData.F10Flag = TRUE ;
//	InputSysData.F10InputTime = NS_GetNowCount() ;

	// 終了
	return 0 ;
}

// F10 が離されたことにするフラグを立てる
extern int ResetF10Input( void )
{
	InputSysData.F10Flag = FALSE ;

	// 終了
	return 0 ;
}

// F12 が押されたことにするフラグを立てる
extern int SetF12Input( void )
{
	InputSysData.F12Flag = TRUE ;
//	InputSysData.F12InputTime = NS_GetNowCount() ;

	// 終了
	return 0 ;
}

// F12 が離されたことにするフラグを立てる
extern int ResetF12Input( void )
{
	InputSysData.F12Flag = FALSE ;

	// 終了
	return 0 ;
}











// 入力状態取得関数

// キーボードの状態取得
extern int NS_CheckHitKey( int KeyCode )
{
	int Result ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	// キーボードの入力状態を更新
	UpdateKeyboardInputState() ;

	// 指定のキーの状態を返す
	Result = ( InputSysData.KeyInputBuf[ KeyCode ] & 0x80 ) != 0 ;
	
	return Result ;
}

// 全キーの押下状態を取得
extern int NS_CheckHitKeyAll( int CheckType )
{
	int i ;
	unsigned char *KeyData ;

#ifdef __WINDOWS__
	if( WinData.MainWindow == NULL )
		return -1 ;
#endif // __WINDOWS__

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	// キーボードのチェック
	if( CheckType & DX_CHECKINPUT_KEY )
	{
		// キーボードの入力状態を更新
		UpdateKeyboardInputState() ;

		// どれか一つでもキーが押されているか判定
		KeyData = InputSysData.KeyInputBuf ;
		for( i = 0 ; i < 256 ; i ++ , KeyData ++ )
		{
			if( ( *KeyData & 0x80 ) && i != KEY_INPUT_KANJI && i != KEY_INPUT_CAPSLOCK && i != KEY_INPUT_KANA )
				return i ;
		}
		if( InputSysData.F10Flag )
		{
			return KEY_INPUT_F10 ;
		}
	}

	// ジョイパッドのチェック
	if( CheckType & DX_CHECKINPUT_PAD )
	{
		for( i = 0 ; i < InputSysData.PadNum ; i ++ )
		{
			if( NS_GetJoypadInputState( i + 1 ) != 0 )
				return -1 ;
		}
	}

#ifdef __WINDOWS__
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
#endif // __WINDOWS__

	// 終了
	return 0 ;
}

// マウスのボタンの状態を得る 
extern int NS_GetMouseInput( void )
{
	// 環境依存処理に丸投げ
	return GetMouseInput_PF() ;
}

// すべてのキーの押下状態を取得する
extern int NS_GetHitKeyStateAll( char *KeyStateBuf )
{
	int i ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	// キーボードの入力状態を更新
	UpdateKeyboardInputState() ;

	// キーの入力値を変換
	for( i = 0 ; i < 256 ; i ++ , KeyStateBuf ++ )
	{
		*KeyStateBuf = ( BYTE )( InputSysData.KeyInputBuf[i] ) >> 7 ;
	}

	// 終了
	return 0 ;
}





// ジョイパッドが接続されている数を取得する
extern int NS_GetJoypadNum( void )
{
	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	return InputSysData.PadNum ;
}

// ジョイバッドの入力状態取得
extern int NS_GetJoypadInputState( int InputType )
{
	int BackData = 0 ;
	INPUTPADDATA *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	int iX , iY ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	if( ( JoypadNum < -1 || JoypadNum >= InputSysData.PadNum ) && 
		( InputSysData.PadNum != 0 && ( InputType & DX_INPUT_KEY ) == 0 ) )
	{
		return 0 ;
	}

	if( JoypadNum != -1 && InputSysData.PadNum != 0 && JoypadNum < InputSysData.PadNum )
	{
		pad = &InputSysData.Pad[ JoypadNum ] ;

		// パッドの情報を取得する
		{
			// 情報の更新
			UpdateJoypadInputState( JoypadNum ) ;

			// 入力状態を保存
			iX = pad->State.X ;
			iY = pad->State.Y ;

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
					BackData |= ( pad->State.Buttons[ i ] & 0x80 ) != 0 ? 1 << ( i + 4 ) : 0 ;
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

		KeyBuf = InputSysData.KeyInputBuf ;
		Map = InputSysData.KeyToJoypadInput[ JoypadNum < 0 ? 0 : JoypadNum ][ 0 ] ;

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
	}

	// 終了
	return BackData ;
}

// ジョイパッドのアナログ的なスティック入力情報を得る
extern int NS_GetJoypadAnalogInput( int *XBuf , int *YBuf , int InputType )
{
	int BackData = 0 ;
	INPUTPADDATA *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;
	
	if( XBuf != NULL ) *XBuf = 0 ;
	if( YBuf != NULL ) *YBuf = 0 ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	if( ( JoypadNum < -1 || JoypadNum >= InputSysData.PadNum ) && 
		( InputSysData.PadNum != 0 && ( InputType & DX_INPUT_KEY ) == 0 ) )
	{
		return 0 ;
	}

	if( JoypadNum != -1 && InputSysData.PadNum != 0 )
	{
		pad = &InputSysData.Pad[ JoypadNum ] ;

		// 入力状態の更新
		UpdateJoypadInputState( JoypadNum ) ;

		// 入力状態を保存
		if( XBuf ) *XBuf = pad->State.X ;
		if( YBuf ) *YBuf = pad->State.Y ;
	}

	// キー入力指定もあった場合はキーの入力状態も反映させる
	if( InputType & DX_INPUT_KEY )
	{
		BYTE *KeyBuf ;

		// キーボードの入力状態を更新
		UpdateKeyboardInputState() ;

		KeyBuf = InputSysData.KeyInputBuf ;
		if( XBuf )
		{
			if( KeyBuf[ KEY_INPUT_NUMPAD4 ] & 0x80 || KeyBuf[ KEY_INPUT_LEFT  ] & 0x80 ) *XBuf = -DIRINPUT_MAX ;		// ４キー←チェック
			if( KeyBuf[ KEY_INPUT_NUMPAD6 ] & 0x80 || KeyBuf[ KEY_INPUT_RIGHT ] & 0x80 ) *XBuf =  DIRINPUT_MAX	;		// ６キー→キーチェック
		}
		if( YBuf )
		{
			if( KeyBuf[ KEY_INPUT_NUMPAD2 ] & 0x80 || KeyBuf[ KEY_INPUT_DOWN  ] & 0x80 ) *YBuf =  DIRINPUT_MAX	;		// ２キー↓キーチェック
			if( KeyBuf[ KEY_INPUT_NUMPAD8 ] & 0x80 || KeyBuf[ KEY_INPUT_UP    ] & 0x80 ) *YBuf = -DIRINPUT_MAX ;		// ８キー↑キーチェック
		}
	}

	// 終了
	return BackData ;
}

// ジョイパッドのアナログ的なスティック入力情報を得る(右スティック用)
extern int NS_GetJoypadAnalogInputRight( int *XBuf, int *YBuf, int InputType )
{
	int BackData = 0 ;
	INPUTPADDATA *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;
	
	if( XBuf != NULL ) *XBuf = 0 ;
	if( YBuf != NULL ) *YBuf = 0 ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	if( JoypadNum < -1 || JoypadNum >= InputSysData.PadNum )
		return 0 ;

	if( JoypadNum != -1 )
	{
		pad = &InputSysData.Pad[ JoypadNum ] ;

		// 入力状態の更新
		UpdateJoypadInputState( JoypadNum ) ;

		// 入力状態を保存( XInput かどうかで処理を分岐 )
		if( NS_CheckJoypadXInput( InputType ) )
		{
			if( XBuf ) *XBuf = pad->State.Rx ;
			if( YBuf ) *YBuf = pad->State.Ry ;
		}
		else
		{
			if( XBuf ) *XBuf = pad->State.Z ;
			if( YBuf ) *YBuf = pad->State.Rz ;
		}
	}

	// 終了
	return BackData ;
}

// ジョイパッドから DirectInput から得られる生のデータを取得する( DX_CHECKINPUT_KEY や DX_INPUT_KEY_PAD1 を引数に渡すとエラー )
extern	int	NS_GetJoypadDirectInputState( int InputType, DINPUT_JOYSTATE *DInputState )
{
	INPUTPADDATA *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	if( JoypadNum < 0 || JoypadNum >= InputSysData.PadNum )
	{
		_MEMSET( DInputState, 0, sizeof( DINPUT_JOYSTATE ) ) ;
		DInputState->POV[ 0 ] = 0xffffffff ;
		DInputState->POV[ 1 ] = 0xffffffff ;
		DInputState->POV[ 2 ] = 0xffffffff ;
		DInputState->POV[ 3 ] = 0xffffffff ;
		return -1 ;
	}

	pad = &InputSysData.Pad[ JoypadNum ] ;

	// 入力状態の更新
	UpdateJoypadInputState( JoypadNum ) ;

	// 入力状態を代入する
	if( DInputState )
	{
		*DInputState = pad->State ;
	}

	// 正常終了
	return 0 ;
}

// 指定の入力デバイスが XInput に対応しているかどうかを取得する
// ( 戻り値  TRUE:XInput対応の入力デバイス  FALSE:XInput非対応の入力デバイス   -1:エラー )
// ( DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キーボードが絡むタイプを InputType に渡すとエラーとなり -1 を返す )
extern int NS_CheckJoypadXInput( int InputType )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	if( JoypadNum < 0 || JoypadNum >= InputSysData.PadNum )
	{
		return -1 ;
	}

	// 環境依存処理に任せる
	return CheckJoypadXInput_PF( InputType ) ;
}

// XInput から得られる入力デバイス( Xbox360コントローラ等 )の生のデータを取得する( XInput非対応のパッドの場合はエラーとなり -1 を返す、DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キーボードが絡むタイプを InputType に渡すとエラーとなり -1 を返す )
extern int NS_GetJoypadXInputState(	int InputType, XINPUT_STATE *XInputState )
{
	INPUTPADDATA *pad ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	if( JoypadNum < 0 || JoypadNum >= InputSysData.PadNum )
	{
		_MEMSET( XInputState, 0, sizeof( XINPUT_STATE ) ) ;
		return -1 ;
	}

	pad = &InputSysData.Pad[ JoypadNum ] ;

	// XInput に対応していなかったらエラー
	if( NS_CheckJoypadXInput( InputType ) == FALSE )
	{
		_MEMSET( XInputState, 0, sizeof( XINPUT_STATE ) ) ;
		return -1 ;
	}

	// 入力状態の更新
	UpdateJoypadInputState( JoypadNum ) ;

	// 入力状態を代入する
	if( XInputState )
	{
		*XInputState = pad->XInputState ;
	}

	// 正常終了
	return 0 ;
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
			InputSysData.KeyToJoypadInput[ JoypadNum ][ i ][ 0 ] = KeyInput1 ;
			InputSysData.KeyToJoypadInput[ JoypadNum ][ i ][ 1 ] = KeyInput2 ;
			InputSysData.KeyToJoypadInput[ JoypadNum ][ i ][ 2 ] = KeyInput3 ;
			InputSysData.KeyToJoypadInput[ JoypadNum ][ i ][ 3 ] = KeyInput4 ;
		}
	}

	// 正常終了
	return 0 ;
}

// ジョイパッドの無効ゾーンの設定を行う
extern	int NS_SetJoypadDeadZone( int InputType, double Zone )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	INPUTPADDATA *pad = &InputSysData.Pad[ JoypadNum ] ;
	DWORD ZoneI ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;
	
	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= InputSysData.PadNum )
	{
		return 0 ;
	}

	// 今までと同じゾーンの場合は何もせず終了
	ZoneI = ( DWORD )_DTOL( Zone * 65536 ) ;
	if( pad->DeadZone == ZoneI )
	{
		return 0 ;
	}

	// ゾーンを保存
	pad->DeadZone = ZoneI ;

	// 環境依存処理
	SetJoypadDeadZone_PF( pad ) ;

	// 正常終了
	return 0 ;
}

// ジョイパッドの振動を開始する
extern	int NS_StartJoypadVibration( int InputType, int Power, int Time, int EffectIndex )
{
	int               SetPower ;
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	INPUTPADDATA *pad = &InputSysData.Pad[ JoypadNum ] ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= InputSysData.PadNum )
	{
		return 0 ;
	}

	// EffectIndex がマイナスの場合は処理を分岐
	if( EffectIndex < 0 )
	{
		if( Power < 0 )
		{
			NS_StartJoypadVibration( InputType, pad->Effect[ DINPUTPAD_MOTOR_LEFT ].Power, Time, DINPUTPAD_MOTOR_LEFT ) ;
		}
		else
		{
			SetPower = Power * 2 ;
			if( SetPower > DX_FFNOMINALMAX / 10 )
			{
				SetPower = DX_FFNOMINALMAX / 10 ;
			}
			NS_StartJoypadVibration( InputType, SetPower, Time, DINPUTPAD_MOTOR_LEFT ) ;
		}

		if( Power < 0 )
		{
			NS_StartJoypadVibration( InputType, pad->Effect[ DINPUTPAD_MOTOR_RIGHT ].Power, Time, DINPUTPAD_MOTOR_RIGHT ) ;
		}
		else
		{
			SetPower = ( Power - ( DX_FFNOMINALMAX / 10 / 2 ) ) * 2 ;
			if( SetPower <= 0 )
			{
				SetPower = 0 ;
			}
			else
			if( SetPower > DX_FFNOMINALMAX / 10 )
			{
				SetPower = DX_FFNOMINALMAX / 10 ;
			}
			NS_StartJoypadVibration( InputType, SetPower, Time, DINPUTPAD_MOTOR_RIGHT ) ;
		}

		// 終了
		return 0 ;
	}

	if( EffectIndex >= DINPUTPAD_MOTOR_NUM )
	{
		return 0 ;
	}

	// 振動に対応していなければここで終了
	if( CheckJoypadVibrationEnable_PF( pad, EffectIndex ) == FALSE )
	{
		return 0 ;
	}

	// ゆれの大きさを DirectInput の型に合わせる
	if( Power < 0 )
	{
		Power = pad->Effect[ EffectIndex ].Power ;
	}
	else
	{
		Power *= 10 ;
		if( Power >  DX_FFNOMINALMAX )
		{
			Power =  DX_FFNOMINALMAX ;
		}
	}

	// 揺れの大きさが０の場合は振動をストップする
	if( Power == 0 )
	{
		NS_StopJoypadVibration( InputType, EffectIndex ) ;
	}
	else
	{
		// 再生開始時刻などをセット
		pad->Effect[ EffectIndex ].BackTime	= NS_GetNowCount() ;
		pad->Effect[ EffectIndex ].Time		= Time ;
		pad->Effect[ EffectIndex ].CompTime	= 0 ;

		// 既に再生中でゆれの大きさも同じ場合は何もしない
		if( pad->Effect[ EffectIndex ].PlayFlag == TRUE && pad->Effect[ EffectIndex ].Power == Power )
		{
			return 0 ;
		}

		// 再生設定を保存
		pad->Effect[ EffectIndex ].PlayFlag	= TRUE ;
		pad->Effect[ EffectIndex ].Power	= Power ;

		// 再生状態の設定を反映
		RefreshEffectPlayState() ;
	}

	// 終了
	return 0 ;
}

// ジョイパッドの振動を停止する
extern	int NS_StopJoypadVibration( int InputType, int EffectIndex )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	INPUTPADDATA *pad = &InputSysData.Pad[ JoypadNum ] ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;
	
	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= InputSysData.PadNum )
	{
		return 0 ;
	}

	// EffectIndex がマイナスの場合は処理を分岐
	if( EffectIndex < 0 )
	{
		// 左右モーターの振動を止める
		NS_StopJoypadVibration( InputType, DINPUTPAD_MOTOR_LEFT ) ;
		NS_StopJoypadVibration( InputType, DINPUTPAD_MOTOR_RIGHT ) ;

		// 終了
		return 0 ;
	}

	if( EffectIndex >= DINPUTPAD_MOTOR_NUM )
	{
		return 0 ;
	}

	// 振動に対応していなければここで終了
	if( CheckJoypadVibrationEnable_PF( pad, EffectIndex ) == FALSE )
	{
		return 0 ;
	}

	// 既に再生が停止していたら何もしない
	if( pad->Effect[ EffectIndex ].PlayFlag == FALSE )
	{
		return 0 ;
	}

	// 再生フラグを倒す
	pad->Effect[ EffectIndex ].PlayFlag = FALSE ;

	// 振動の強さを初期化
	pad->Effect[ EffectIndex ].Power = DX_FFNOMINALMAX ;

	// 再生状態の設定を反映
	RefreshEffectPlayState() ;

	// 終了
	return 0 ;
}

// ジョイパッドのＰＯＶ入力の状態を得る( 単位は角度の１００倍  中心位置にある場合は -1 が返る )
extern int NS_GetJoypadPOVState( int InputType, int POVNumber )
{
	int JoypadNum = ( InputType & ~DX_INPUT_KEY ) - 1 ;
	INPUTPADDATA *pad = &InputSysData.Pad[ JoypadNum ] ;
	DWORD pov;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;
	
	// 初期化されていなかったら条件付きで初期化を行う
	if( InputSysData.InitializeFlag == FALSE )
	{
		return AutoInitialize_PF() ;
	}

	// エラーチェック
	if( JoypadNum < 0 || JoypadNum >= InputSysData.PadNum || POVNumber >= 4 )
	{
		return -1 ;
	}

	// 情報の更新
	UpdateJoypadInputState( JoypadNum ) ;
	pov = pad->State.POV[ POVNumber ] ;

	// 中心チェック
	if( ( pov & 0xffff ) == 0xffff )
	{
		return -1 ;
	}

	// 何らかの角度がある場合はそのまま返す
	return ( int )pov ;
}

// ジョイパッドの再セットアップを行う( 新たに接続されたジョイパッドがある場合に検出される )
extern int NS_ReSetupJoypad( void )
{
	// ジョイパッドの再セットアップを行う
	return SetupJoypad() ;
}

// キーボードの入力状態の更新
extern int UpdateKeyboardInputState( int UseProcessMessage )
{
	static BOOL InFunction = FALSE ;
	int Result ;

	// 無限再帰防止
	if( InFunction == TRUE )
	{
		return 0 ;
	}
	InFunction = TRUE ;

	// 環境依存処理
	Result = UpdateKeyboardInputState_PF( UseProcessMessage ) ;

	InFunction = FALSE ;

	return Result ;
}

// パッドの入力状態の更新
static int UpdateJoypadInputState( int padno )
{
	int Result ;

	// パッド番号チェック
	if( padno < 0 || padno >= MAX_JOYPAD_NUM )
	{
		return -1 ;
	}

	// 環境依存処理
	Result = UpdateJoypadInputState_PF( padno ) ;

	// 正常終了
	return Result ;
}

// ジョイパッドの振動機能を使用するかどうかのフラグをセットする
extern int NS_SetUseJoypadVibrationFlag( int Flag )
{
	InputSysData.NoUseVibrationFlag = !Flag ;

	// 終了
	return 0 ;
}

// パッドのエフェクト処理に関するフレーム処理を行う
extern int JoypadEffectProcess( void )
{
	int        i ;
	int        j ;
	int        num ;
	int        time ;
	INPUTPADDATA *Pad ;

	// 初期化判定
	if( InputSysData.InitializeFlag == FALSE )
	{
		return 0 ;
	}

	// パッドデバイスの再取得
	num = InputSysData.PadNum ;
	Pad = InputSysData.Pad ;
	time = NS_GetNowCount() ;
	for( i = 0 ; i < num ; i ++, Pad ++ )
	{
		for( j = 0 ; j < DINPUTPAD_MOTOR_NUM ; j ++ )
		{
			if( CheckJoypadVibrationEnable_PF( Pad, j ) == FALSE )
			{
				continue ;
			}

			if( Pad->Effect[ j ].PlayFlag == FALSE )
			{
				continue ;
			}

			// 無限再生の場合は特に何もしない
			if( Pad->Effect[ j ].Time < 0 )
			{
				continue ;
			}

			// 前回から時刻が変わっていなかったら何もしない
			if( Pad->Effect[ j ].BackTime == time )
			{
				continue ;
			}

			// 経過時間の加算
			if( time < Pad->Effect[ j ].BackTime )
			{
				Pad->Effect[ j ].BackTime = time ;
			}
			Pad->Effect[ j ].CompTime += time - Pad->Effect[ j ].BackTime ;

			// 今回の時刻を保存
			Pad->Effect[ j ].BackTime = time ;

			// 再生停止のチェック
			if( Pad->Effect[ j ].CompTime >= Pad->Effect[ j ].Time )
			{
				NS_StopJoypadVibration( i + 1, j ) ;
			}
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
	// 初期化判定
	if( InputSysData.InitializeFlag == FALSE )
	{
		return 0 ;
	}

	// 環境依存処理
	RefreshEffectPlayState_PF() ;

	// 終了
	return 0 ;
}

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // DX_NON_INPUT
