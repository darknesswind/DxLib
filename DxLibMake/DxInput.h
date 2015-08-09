﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		入力情報プログラムヘッダファイル
// 
// 				Ver 3.14d
// 
// -------------------------------------------------------------------------------

#ifndef __DXINPUT_H__
#define __DXINPUT_H__

#include "DxCompileConfig.h"

#ifndef DX_NON_INPUT

// インクルード ------------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"

#ifdef __WINDOWS__
#include "Windows/DxInputWin.h"
#endif // __WINDOWS__

#ifdef __PSVITA
#include "PSVita/DxInputPSVita.h"
#endif // __PSVITA

#ifdef __PS4
#include "PS4/DxInputPS4.h"
#endif // __PS4

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

// 方向入力の最大値
#define DIRINPUT_MAX				(1000)

// モーターの番号定義
#define DINPUTPAD_MOTOR_LEFT		0
#define DINPUTPAD_MOTOR_RIGHT		1
#define DINPUTPAD_MOTOR_NUM			2

// 構造体定義 --------------------------------------------------------------------

// 振動の情報
struct INPUTVIBRATIONDATA
{
	int						PlayFlag ;							// エフェクトが再生中か、フラグ
	int						Power ;								// エフェクトの強さ
	int						Time ;								// エフェクトの長さ
	int						CompTime ;							// エフェクトを開始してから経過した時間
	int						BackTime ;							// エフェクトの前回調べた時刻

	int						PlayState ;							// 振動の再生状態(ソフトが非アクティブになれば再生停止する為)
	int						SetPower ;							// 振動の設定パワー

	INPUTVIBRATIONDATA_PF	PF ;								// 環境依存情報
} ;

// ゲームパッドの情報
struct INPUTPADDATA
{
	int						GetTime ;							// 取得時間
	INPUTVIBRATIONDATA		Effect[ DINPUTPAD_MOTOR_NUM ] ;		// モーターの情報
	DWORD					DeadZone ;							// 無効ゾーン( 65536 = 100% )

	DINPUT_JOYSTATE			State ;								// 状態
	XINPUT_STATE			XInputState ;						// XInputでのパッドの状態

	INPUTPADDATA_PF			PF ;								// 環境依存情報
} ;

// 入力システム用データ構造体型
struct INPUTSYSTEMDATA
{
	volatile int			InitializeFlag ;					// 初期化完了フラグ

	int						NoUseVibrationFlag ;				// ＤｉｒｅｃｔＩｎｐｕｔの振動機能を使用しないかどうかのフラグ
	int						KeyInputGetTime ;					// 一つ前に状態を取得した時間
	unsigned char			KeyInputBuf[ 256 ] ;				// キーボードの入力状態

	int						KeyToJoypadInputInitializeFlag ;	// KeyToJoypadInput を一度でも初期化したかどうかのフラグ
	int						KeyToJoypadInput[ MAX_JOYPAD_NUM ][ 32 ][ 4 ] ;	// ジョイパッドの入力に対応したキーボードの入力

	int						MouseInputBufValidFlag ;			// MouseInputBuf が有効かどうか( TRUE:有効  FALSE:無効 )
	BYTE					MouseInputBuf[ 4 ] ;				// マウスの入力状態( 0:LEFT 1:RIGHT 2:MIDDLE )
	int						MouseMoveZ ;						// マウスホイールの移動値

	int						PadNum ;							// ジョイパッドの数
	INPUTPADDATA			Pad[ MAX_JOYPAD_NUM ] ;				// パッドデバイス情報

	int						EndFlag ;

	int						F10Flag ;							// F10 が押されたかフラグ
	int						F10InputTime ;						// F10 が押された時間
	int						F12Flag ;							// F12 が押されたかフラグ
	int						F12InputTime ;						// F12 が押された時間

	INPUTSYSTEMDATA_PF		PF ;								// 環境依存情報
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 入力システム用データ
extern INPUTSYSTEMDATA InputSysData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// 入力システムの初期化、終了関数
extern	int InitializeInputSystem( void ) ;											// 入力システムを初期化する 
extern	int TerminateInputSystem( void ) ;											// 入力システムの後始末をする

extern	int JoypadEffectProcess( void ) ;											// パッドエフェクトに関するフレーム処理を行う
extern	int RefreshEffectPlayState( void ) ;										// パッドエフェクトの再生状態を更新する(ソフトが非アクティブになることもあるため)
extern	int UpdateKeyboardInputState( int UseProcessMessage = TRUE ) ;				// 入力状態の更新
extern	int SetupJoypad( void ) ;													// ジョイパッドのセットアップを行う
extern	int TerminateJoypad( void ) ;												// ジョイパッドの後始末を行う

extern	int SetF10Input( void ) ;													// F10 が押されたことにするフラグを立てる
extern	int ResetF10Input( void ) ;													// F10 が離されたことにするフラグを立てる
extern	int SetF12Input( void ) ;													// F12 が押されたことにするフラグを立てる
extern	int ResetF12Input( void ) ;													// F12 が離されたことにするフラグを立てる

// wchar_t版関数
extern int GetJoypadName_WCHAR_T( int InputType, wchar_t *InstanceNameBuffer, wchar_t *ProductNameBuffer ) ;


// 環境依存関係関数
extern	int InitializeInputSystem_PF_Timing0( void ) ;								// 入力システムを初期化する処理の環境依存処理
extern	int TerminateInputSystem_PF_Timing0( void ) ;								// 入力システムの後始末をする処理の環境依存処理

extern	int	AutoInitialize_PF( void ) ;												// 自動初期化を行う環境依存処理

extern	int SetupJoypad_PF( void ) ;												// ジョイパッドのセットアップの環境依存処理
extern	int TerminateJoypad_PF( void ) ;											// ジョイパッドの後始末を行う処理の環境依存処理

extern	int UpdateKeyboardInputState_PF( int UseProcessMessage = TRUE ) ;			// 入力状態の更新の環境依存処理
extern	int UpdateJoypadInputState_PF( int PadNo ) ;								// パッドの入力状態の更新の環境依存処理
extern	int RefreshEffectPlayState_PF( void ) ;										// パッドエフェクトの再生状態を更新する関数の環境依存処理

extern	int CheckJoypadVibrationEnable_PF( INPUTPADDATA *pad, int EffectIndex ) ;	// 指定のパッドが振動に対応しているかどうかを取得する( TRUE:対応している  FALSE:対応していない )
extern	int CheckJoypadXInput_PF( int InputType ) ;									// 指定の入力デバイスが XInput に対応しているかどうかを取得する処理の環境依存処理( 戻り値  TRUE:XInput対応の入力デバイス  FALSE:XInput非対応の入力デバイス   -1:エラー )( DX_INPUT_KEY や DX_INPUT_KEY_PAD1 など、キーボードが絡むタイプを InputType に渡すとエラーとなり -1 を返す )
extern	int GetMouseInput_PF( void ) ;												// マウスのボタンの状態を得る処理の環境依存処理
extern	int SetJoypadDeadZone_PF( INPUTPADDATA *pad ) ;								// ジョイパッドの無効ゾーンの設定を行う関数の環境依存処理


#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // DX_NON_INPUT

#endif // __DXINPUT_H__
