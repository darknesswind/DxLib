﻿//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		WindowsOS用描画処理プログラム
// 
//  	Ver 3.14f
// 
//-----------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード ---------------------------------------------------------------
#include "DxGraphicsWin.h"
#include "DxWindow.h"
#include "DxWinAPI.h"
#include "../DxSystem.h"
#include "../DxLog.h"
#include "../DxModel.h"
#include "../DxBaseFunc.h"
#include "../DxGraphics.h"
#include "../DxASyncLoad.h"
#include "../DxMask.h"

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 -----------------------------------------------------------------

// 構造体宣言 -----------------------------------------------------------------

// データ定義 -----------------------------------------------------------------

static int UseGDIFlag ;							// GDI を使用するかどうか

GRAPHICSMANAGE_WIN GraphicsManage_Win ; 

// 関数宣言 -------------------------------------------------------------------

static int UseChangeDisplaySettings( void ) ;									// ChangeDisplaySettings を使用して画面モードを変更する

// プログラム -----------------------------------------------------------------

// DirectX のオブジェクトを解放する
extern void Graphics_ReleaseDirectXObject( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		Graphics_D3D11_ReleaseDirectXObject_PF() ;
		break ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		Graphics_D3D9_ReleaseDirectXObject_PF() ;
		break ;
#endif // DX_NON_DIRECT3D9

	default :
		break ;
	}
}

// メッセージループ時に行うべき処理を実行する
extern void Graphics_Win_MessageLoop_Process( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		break ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		// 管理テクスチャへの転送用のシステムメモリテクスチャの定期処理を行う
		Graphics_D3D9_SysMemTextureProcess() ;

		// 管理テクスチャへの転送用のシステムメモリサーフェスの定期処理を行う
		Graphics_D3D9_SysMemSurfaceProcess() ;
		break ;
#endif // DX_NON_DIRECT3D9

	default :
		break ;
	}
}

// WM_ACTIVATE メッセージの処理で、ウインドウモード時にアクティブになった際に呼ばれる関数
extern void Graphics_Win_WM_ACTIVATE_ActiveProcess( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		Graphics_D3D11_WM_ACTIVATE_ActiveProcess_PF() ;
		break ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		Graphics_D3D9_WM_ACTIVATE_ActiveProcess_PF() ;
		break ;
#endif // DX_NON_DIRECT3D9

	default :
		break ;
	}
}





// 設定関係関数

// ダイアログボックスモードを変更する
extern	int		Graphics_Win_SetDialogBoxMode( int Flag )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_D3D9_SetDialogBoxMode_PF( Flag ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}








// 裏画面の内容を指定のウインドウに転送する
extern int NS_BltBackScreenToWindow( HWND Window, int ClientX, int ClientY )
{
	RECT SrcRect, DestRect, WinRect ;

	GetClientRect( Window, &WinRect ) ;

	SrcRect.left   = 0 ;
	SrcRect.top    = 0 ;
	SrcRect.right  = GSYS.Screen.MainScreenSizeX ;
	SrcRect.bottom = GSYS.Screen.MainScreenSizeY ;

	DestRect.left   = WinRect.left + ClientX ;
	DestRect.top    = WinRect.top  + ClientY ;
	DestRect.right  = WinRect.left + ClientX + GSYS.Screen.MainScreenSizeX ;
	DestRect.bottom = WinRect.top  + ClientY + GSYS.Screen.MainScreenSizeY ;

	if( DestRect.left < WinRect.left )
	{
		SrcRect.left += WinRect.left - DestRect.left ;
		DestRect.left = WinRect.left ;
	}

	if( DestRect.top < WinRect.top )
	{
		SrcRect.top += WinRect.top - DestRect.top ;
		DestRect.top = WinRect.top ;
	}

	if( DestRect.right > WinRect.right )
	{
		SrcRect.right -= DestRect.right - WinRect.right ;
		DestRect.right = WinRect.right ;
	}

	if( DestRect.bottom > WinRect.bottom )
	{
		SrcRect.bottom -= DestRect.bottom - WinRect.bottom ;
		DestRect.bottom = WinRect.bottom ;
	}

	return NS_BltRectBackScreenToWindow( Window, SrcRect, DestRect ) ;
}

// 裏画面の指定の領域をウインドウのクライアント領域の指定の領域に転送する
extern int NS_BltRectBackScreenToWindow( HWND Window, RECT BackScreenRect, RECT WindowClientRect )
{
	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE || DxLib_GetEndRequest() ) return -1 ;

	// フルスクリーンモードでは使用できない
	if( NS_GetWindowModeFlag() == FALSE ) return -1 ;

	// 実行
	return Graphics_BltRectBackScreenToWindow_PF( Window, BackScreenRect, WindowClientRect ) ;
}

// ScreenFlip で画像を転送する先のウインドウを設定する( NULL を指定すると設定解除 )
extern int NS_SetScreenFlipTargetWindow( HWND TargetWindow )
{
	return Graphics_SetScreenFlipTargetWindow_PF( TargetWindow ) ;
}

// 使用する DirectDraw デバイスのインデックスを設定する
extern int NS_SetUseDirectDrawDeviceIndex( int Index )
{
	return NS_SetUseDisplayIndex( Index ) ;
}

// DirectDraw や Direct3D の協調レベルをマルチスレッド対応にするかどうかをセットする
extern	int		NS_SetMultiThreadFlag( int Flag )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_Hardware_D3D9_SetMultiThreadFlag_PF( Flag ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// Vista以降の Windows Aero を無効にするかどうかをセットする、TRUE:無効にする  FALSE:有効にする( DxLib_Init の前に呼ぶ必要があります )
extern int NS_SetAeroDisableFlag( int Flag )
{
	return Graphics_SetAeroDisableFlag_PF( Flag ) ;
}

// Vista以降の環境で Direct3D9Ex を使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )
extern int NS_SetUseDirect3D9Ex( int Flag )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_D3D9_SetUseDirect3D9Ex_PF( Flag ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// Direct3D11 を使用するかどうかを設定する( TRUE:使用する  FALSE:使用しない )
extern int NS_SetUseDirect3D11( int Flag )
{
	// 初期化前のみ使用可能 
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	// フラグを保存
	GRAWIN.Setting.NotUseDirect3D11 = Flag ? FALSE : TRUE ;

	// 終了
	return 0 ;
}

// Direct3D11 で使用する最低機能レベルを指定する関数です、尚、DX_DIRECT3D_11_FEATURE_LEVEL_11_0 より低い機能レベルでの正常な動作は保証しません( デフォルトは DX_DIRECT3D_11_FEATURE_LEVEL_11_0 )
extern int NS_SetUseDirect3D11MinFeatureLevel( int Level /* DX_DIRECT3D_11_FEATURE_LEVEL_10_0 など */ )
{
#ifndef DX_NON_DIRECT3D11
	return Graphics_D3D11_SetUseDirect3D11MinFeatureLevel_PF( Level ) ;
#else // DX_NON_DIRECT3D11
	return 0 ;
#endif // DX_NON_DIRECT3D11
}

// 使用する Direct3D のバージョンを設定する
extern int NS_SetUseDirect3DVersion( int Version )
{
	// 初期化前のみ有効
	if( DxSysData.DxLib_InitializeFlag )
	{
		return -1 ;
	}

	// バージョンによって処理を変更
	switch( Version )
	{
	case DX_DIRECT3D_NONE :
		NS_SetUseDirectDrawFlag( FALSE ) ;
		NS_SetUseDirect3D11( FALSE ) ;
		break ;

	case DX_DIRECT3D_9 :
		NS_SetUseDirect3D9Ex( FALSE ) ;
		break ;

	case DX_DIRECT3D_9EX :
		NS_SetUseDirect3D9Ex( TRUE ) ;
		break ;

	case DX_DIRECT3D_11 :
		NS_SetUseDirectDrawFlag( TRUE ) ;
		NS_SetUseDirect3D11( TRUE ) ;
		break ;

	default :
		return -1 ;
	}

	// 終了
	return 0 ;
}

// 使用している Direct3D のバージョンを取得する( DX_DIRECT3D_9 など )
extern int NS_GetUseDirect3DVersion( void )
{
#ifndef DX_NON_DIRECT3D9
	if( Direct3D9_IsValid() )
	{
		if( Direct3D9_IsExObject() )
		{
			return DX_DIRECT3D_9EX ;
		}
		else
		{
			return DX_DIRECT3D_9 ;
		}
	}
#endif // DX_NON_DIRECT3D9

#ifndef DX_NON_DIRECT3D11
	if( D3D11Device_IsValid() )
	{
		return DX_DIRECT3D_11 ;
	}
#endif // DX_NON_DIRECT3D11

	return DX_DIRECT3D_NONE ;
}

// 使用している Direct3D11 の FeatureLevel ( DX_DIRECT3D_11_FEATURE_LEVEL_9_1 等 )を取得する( 戻り値　-1：エラー　-1以外：Feature Level )
extern int NS_GetUseDirect3D11FeatureLevel(	void )
{
#ifndef DX_NON_DIRECT3D11
	if( D3D11Device_IsValid() == FALSE )
	{
		return -1 ;
	}

	return GD3D11.Setting.FeatureLevel ;
#else  // DX_NON_DIRECT3D11
	return -1 ;
#endif // DX_NON_DIRECT3D11
}

// ＤｉｒｅｃｔＤｒａｗを使用するかどうかをセットする
extern int NS_SetUseDirectDrawFlag( int Flag )
{
	// 初期化前のみ使用可能 
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	GSYS.Setting.NotUseHardware = !Flag ;

	// 終了
	return 0 ;
}

// ＧＤＩ描画を必要とするか、を変更する
extern	int		NS_SetUseGDIFlag( int Flag )
{
	// フラグを保存
	UseGDIFlag = Flag ;

	// ハードウエアを使用しているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		switch( GRAWIN.Setting.UseGraphicsAPI )
		{
#ifndef DX_NON_DIRECT3D11
		case GRAPHICS_API_DIRECT3D11_WIN32 :
			break  ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
		case GRAPHICS_API_DIRECT3D9_WIN32 :
			// ハードウエアレンダリングモードの場合
			if( Direct3DDevice9_IsValid() == 0 ) return 0 ;

			// デバイスの設定
			if( Flag == FALSE )
			{
				Direct3DDevice9_SetDialogBoxMode( FALSE ) ;
			}
			else
			{
				Direct3DDevice9_SetDialogBoxMode( FALSE ) ;
				Direct3DDevice9_SetDialogBoxMode( TRUE ) ;
			}
			break ;
#endif // DX_NON_DIRECT3D9

		default :
			break ;
	}
	}
	else
	{
		// ソフトウエアレンダリングモードでは特にすること無し
	}

	// 終了
	return 0 ;
}

// ＧＤＩ描画を必要とするかの状態を取得する
extern int NS_GetUseGDIFlag( void )
{
	return UseGDIFlag ;
}

// ＤｉｒｅｃｔＤｒａｗが使用するＧＵＩＤを設定する
extern	int		NS_SetDDrawUseGuid( const GUID FAR *Guid )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_D3D9_SetDDrawUseGuid_PF( Guid ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

/*戻り値を IDirectDraw7 * にキャストして下さい*/			// 現在使用しているＤｉｒｅｃｔＤｒａｗオブジェクトのアドレスを取得する
extern	const void *NS_GetUseDDrawObj( void )
{
	// 終了
	return DirectDraw7_GetObject() ;
}

// 有効な DirectDraw デバイスの GUID を取得する
extern	const GUID *				NS_GetDirectDrawDeviceGUID( int Number )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_D3D9_GetDirectDrawDeviceGUID_PF( Number ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// 有効な DirectDraw デバイスの名前を得る
extern	int						NS_GetDirectDrawDeviceDescription( int Number, char *StringBuffer )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_D3D9_GetDirectDrawDeviceDescription_PF( Number, StringBuffer ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// 有効な DirectDraw デバイスの数を取得する
extern	int						NS_GetDirectDrawDeviceNum( void )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_D3D9_GetDirectDrawDeviceNum_PF() ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// 使用中のＤｉｒｅｃｔ３ＤＤｅｖｉｃｅ９オブジェクトを得る
extern	const void * NS_GetUseDirect3DDevice9( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return NULL ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GetUseDirect3DDevice9_PF(  ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return NULL ;
	}
}

// 使用中のバックバッファのDirect3DSurface9オブジェクトを取得する
extern const void * NS_GetUseDirect3D9BackBufferSurface( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return NULL ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GetUseDirect3D9BackBufferSurface_PF(  ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return NULL ;
	}
}

// ＤＸライブラリのＤｉｒｅｃｔ３Ｄ設定をしなおす
extern int NS_RefreshDxLibDirect3DSetting( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_RefreshDxLibDirect3DSetting_PF(  ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_RefreshDxLibDirect3DSetting_PF(  ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}























// 環境依存初期化関係

// ChangeDisplaySettings を使用して画面モードを変更する
static int UseChangeDisplaySettings( void )
{
//	HRESULT hr ;
//	int     DesktopWidth ;
//	int     DesktopHeight ;
//	int     DesktopColorBitDepth ;
//	int     ScreenSizeX ;
//	int     ScreenSizeY ;
	DEVMODE DevMode ;

	// 元の画面の大きさを得る
//	NS_GetDefaultState( &DesktopWidth, &DesktopHeight, &DesktopColorBitDepth ) ;

	DXST_ERRORLOGFMT_ADDUTF16LE(( "\x3b\x75\x62\x97\xe2\x30\xfc\x30\xc9\x30\x6e\x30\x09\x59\xf4\x66\xe6\x51\x06\x74\x92\x30\x8b\x95\xcb\x59\x57\x30\x7e\x30\x59\x30\x20\x00\x25\x00\x75\x00\x20\x00\x78\x00\x20\x00\x25\x00\x75\x00\x20\x00\x20\x00\x25\x00\x75\x00\x20\x00\x62\x00\x69\x00\x74\x00\x20\x00\x00"/*@ L"画面モードの変更処理を開始します %u x %u  %u bit " @*/, GSYS.Screen.MainScreenSizeX, GSYS.Screen.MainScreenSizeY, GSYS.Screen.MainScreenColorBitDepth )) ;
	DXST_ERRORLOG_TABADD ;

	// フルスクリーンモードの情報をセットアップ
	Graphics_Screen_SetupFullScreenModeInfo() ;

	// 対応しているカラービット深度は 16 と 32 のみ
//	if( GSYS.Screen.MainScreenColorBitDepth != 16 &&
//		GSYS.Screen.MainScreenColorBitDepth != 32 )
	if( GSYS.Screen.FullScreenUseDispModeData.ColorBitDepth != 16 &&
		GSYS.Screen.FullScreenUseDispModeData.ColorBitDepth != 32 )
	{
		return -1 ;
	}

	// ディスプレイモードの変更
	DXST_ERRORLOG_ADDUTF16LE( "\x3b\x75\x62\x97\xe3\x89\xcf\x50\xa6\x5e\x92\x30\x09\x59\xf4\x66\x57\x30\x7e\x30\x59\x30\x2e\x00\x2e\x00\x2e\x00\x20\x00\x00"/*@ L"画面解像度を変更します... " @*/ ) ;

	// Win32 API を使って画面モードを変更する
	{
		// パラメータセット
		_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
		DevMode.dmSize             = sizeof( DevMode ) ;
		DevMode.dmBitsPerPel       = ( DWORD )GSYS.Screen.MainScreenColorBitDepth ;
		DevMode.dmFields           = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT ;
		if( GSYS.Screen.MainScreenRefreshRate != 0 )
		{
			DevMode.dmDisplayFrequency  = ( DWORD )GSYS.Screen.MainScreenRefreshRate ;
			DevMode.dmFields           |= DM_DISPLAYFREQUENCY ;
		}
		DevMode.dmPelsWidth        = ( DWORD )GSYS.Screen.FullScreenUseDispModeData.Width ;
		DevMode.dmPelsHeight       = ( DWORD )GSYS.Screen.FullScreenUseDispModeData.Height ;

		// 画面モードチェンジ
		if( ChangeDisplaySettings( &DevMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
		{
			// 失敗したらリフレッシュレート指定無しでもう一度挑戦
			GSYS.Screen.MainScreenRefreshRate = 0 ;
			DevMode.dmDisplayFrequency        = 0 ;
			DevMode.dmFields                 &= ~DM_DISPLAYFREQUENCY ;
			if( ChangeDisplaySettings( &DevMode, 0/*CDS_FULLSCREEN*/ ) != DISP_CHANGE_SUCCESSFUL )
			{
#if 0
				// それでも駄目で、且つ画面モードが 320x240 だった場合は
				// 擬似 320x240 モードで試す
				if( GSYS.Screen.MainScreenSizeX == 320 && GSYS.Screen.MainScreenSizeY == 240 )
				{
					Graphics_Screen_SetMainScreenSize( 640, 480 ) ;
					NS_SetEmulation320x240( TRUE ) ;
					DevMode.dmPelsWidth        = GSYS.Screen.MainScreenSizeX ;
					DevMode.dmPelsHeight       = GSYS.Screen.MainScreenSizeY ;
					if( ChangeDisplaySettings( &DevMode, 0/*CDS_FULLSCREEN*/ ) != DISP_CHANGE_SUCCESSFUL )
					{
						goto ERRORLABEL ;
					}
					SetWindowStyle() ;
				}
				else
#endif
				{
//ERRORLABEL:
					DXST_ERRORLOG_ADDUTF16LE( "\x3b\x75\x62\x97\xe2\x30\xfc\x30\xc9\x30\x6e\x30\x09\x59\xf4\x66\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"画面モードの変更に失敗しました\n" @*/ ) ;
					DXST_ERRORLOG_TABSUB ;
					return -1 ;
				}
			}
		}
	}

	GRAWIN.UseChangeDisplaySettings = TRUE ;
	DXST_ERRORLOG_ADDUTF16LE( "\x10\x62\x9f\x52\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"成功しました\n" @*/ ) ;

	// 10回画面全体を黒で塗りつぶす指定を残す
	GRAWIN.FullScreenBlackFillCounter = 10 ;

	return 0 ;
}

// 描画処理の環境依存部分の初期化を行う関数( 実行箇所区別０ )
extern int Graphics_Initialize_Timing0_PF( void )
{
	int Result ;

	if( GSYS.Setting.NotUseHardware )
	{
		Result = -1 ;
	}
	else
	{
		Result                        = -1 ;
		GRAWIN.Setting.UseGraphicsAPI = GRAPHICS_API_NONE ;

#ifndef DX_NON_DIRECT3D11
		if( GRAWIN.Setting.NotUseDirect3D11 == FALSE )
		{
			if( Result < 0 )
			{
				GRAWIN.Setting.UseGraphicsAPI = GRAPHICS_API_DIRECT3D11_WIN32 ;
				Result = Graphics_D3D11_Initialize_Timing0_PF() ;
				if( Result < 0 )
				{
					GRAWIN.Setting.UseGraphicsAPI = GRAPHICS_API_NONE ;
				}
			}
		}
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
		if( Result < 0 )
		{
			GRAWIN.Setting.UseGraphicsAPI = GRAPHICS_API_DIRECT3D9_WIN32 ;
			Result = Graphics_D3D9_Initialize_Timing0_PF() ;
			if( Result < 0 )
			{
				GRAWIN.Setting.UseGraphicsAPI = GRAPHICS_API_NONE ;
			}
		}
#endif // DX_NON_DIRECT3D9
	}

	// ハードウェア機能が使用できない( しない )場合はソフトウエアレンダリングモードの処理を行う
	if( Result < 0 )
	{
		// DirectDraw7 を作成する
		if( DirectDraw7_Create() < 0 )
		{
			return -1 ;
		}

		// フルスクリーンモードの場合はここで画面モードを変更する
		if( NS_GetWindowModeFlag() == FALSE )
		{
			if( UseChangeDisplaySettings() < 0 )
			{
				DirectDraw7_Release() ;
				return -1 ;
			}
		}

		// ＶＳＹＮＣ待ちの時間を取得する
		DirectDraw7_WaitVSyncInitialize() ;
	}

	// 正常終了
	return 0 ;
}

// 描画処理の環境依存部分の初期化を行う関数( 実行箇所区別１ )
extern int Graphics_Initialize_Timing1_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		if( Graphics_D3D11_Initialize_Timing1_PF() < 0 )
		{
			return -1 ;
		}
		break ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		if( Graphics_D3D9_Initialize_Timing1_PF() < 0 )
		{
			return -1 ;
		}
#endif // DX_NON_DIRECT3D9
		break ;

	default :
		break ;
	}

	// 正常終了
	return 0 ;
}

// ハードウエアアクセラレータを使用する場合の環境依存の初期化処理を行う
extern	int		Graphics_Hardware_Initialize_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_Hardware_Initialize_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_Hardware_Initialize_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画処理の環境依存部分の後始末を行う関数
extern	int		Graphics_Terminate_PF( void )
{
	int Result ;

	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		Result = Graphics_D3D11_Terminate_PF() ;
		break ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		Result = Graphics_D3D9_Terminate_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		Result = 0 ;
		break ;
	}

	// ディスプレイ情報用に確保したメモリの解放
	if( GSYS.Screen.DisplayInfo )
	{
		HeapFree( GetProcessHeap(), 0, GSYS.Screen.DisplayInfo ) ;
		GSYS.Screen.DisplayInfo = NULL ;
	}

	return Result ;
}

// グラフィックスシステムの復帰、又は変更付きの再セットアップを行う
extern int Graphics_RestoreOrChangeSetupGraphSystem_PF(
	int Change,
	int ScreenSizeX,
	int ScreenSizeY,
	int ColorBitDepth,
	int RefreshRate
)
{
#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込みの途中の場合は、非同期読み込みが終了するまで待つ
	while( NS_GetASyncLoadNum() > 0 )
	{
		ProcessASyncLoadRequestMainThread() ;
		if( WinData.ProcessorNum <= 1 )
		{
			Thread_Sleep( 2 ) ;
		}
		else
		{
			Thread_Sleep( 0 ) ;
		}
	}
#endif // DX_NON_ASYNCLOAD

	// ハードウエアの機能を使用するかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware == TRUE )
	{
		switch( GRAWIN.Setting.UseGraphicsAPI )
		{
#ifndef DX_NON_DIRECT3D11
		case GRAPHICS_API_DIRECT3D11_WIN32 :
			return Graphics_D3D11_RestoreOrChangeSetupGraphSystem_PF( Change, ScreenSizeX, ScreenSizeY, ColorBitDepth, RefreshRate ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
		case GRAPHICS_API_DIRECT3D9_WIN32 :
			return Graphics_D3D9_RestoreOrChangeSetupGraphSystem_PF( Change, ScreenSizeX, ScreenSizeY, ColorBitDepth, RefreshRate ) ;
#endif // DX_NON_DIRECT3D9

		default :
			return 0 ;
		}
	}
	else
	{
		// ソフトウエアレンダリングモード

		if( DxSysData.NotDrawFlag == FALSE )
		{
			// DirectDraw7 の解放
			DirectDraw7_Release() ;
		}

		if( Change == TRUE )
		{
			// 画面モードのセット
			Graphics_Screen_SetMainScreenSize( ScreenSizeX, ScreenSizeY ) ;
			GSYS.Screen.MainScreenColorBitDepth = ColorBitDepth ;
			GSYS.Screen.MainScreenRefreshRate   = RefreshRate ;
	//		SetMemImgDefaultColorType( ColorBitDepth == 32 ? 1 : 0 ) ;
		}

		if( DxSysData.NotDrawFlag == FALSE )
		{
			GSYS.Screen.FullScreenResolutionModeAct = GSYS.Screen.FullScreenResolutionMode ;

			// DirectDraw7 を作成する
			if( DirectDraw7_Create() < 0 )
			{
				return -1 ;
			}

			// フルスクリーンモードの場合はここで画面モードを変更する
			if( NS_GetWindowModeFlag() == FALSE )
			{
				if( UseChangeDisplaySettings() < 0 )
				{
					DirectDraw7_Release() ;
					return -1 ;
				}
			}

			// ＶＳＹＮＣ待ちの時間を取得する
			DirectDraw7_WaitVSyncInitialize() ;
		}
	}

	// 正常終了
	return 0 ;
}

// 描画用デバイスが有効かどうかを取得する
extern int Graphics_Hardware_CheckValid_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_Hardware_CheckValid_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_Hardware_CheckValid_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return -1 ;
	}
}



























// 環境依存描画設定関係

// メインウインドウの背景色を設定する( Red,Green,Blue:それぞれ ０〜２５５ )
extern	int		Graphics_Hardware_SetBackgroundColor_PF( int Red, int Green, int Blue )
{
	return 0 ;
}

// シェーダー描画での描画先を設定する
extern	int		Graphics_Hardware_SetRenderTargetToShader_PF( int TargetIndex, int DrawScreen, int SurfaceIndex )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetRenderTargetToShader_PF(  TargetIndex,  DrawScreen,  SurfaceIndex ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetRenderTargetToShader_PF(  TargetIndex,  DrawScreen,  SurfaceIndex ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// SetDrawBright の引数が一つ版
extern	int		Graphics_Hardware_SetDrawBrightToOneParam_PF( DWORD Bright )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawBrightToOneParam_PF( Bright ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawBrightToOneParam_PF( Bright ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画ブレンドモードをセットする
extern	int		Graphics_Hardware_SetDrawBlendMode_PF( int BlendMode, int BlendParam )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawBlendMode_PF(  BlendMode,  BlendParam ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawBlendMode_PF(  BlendMode,  BlendParam ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画時のアルファテストの設定を行う( TestMode:DX_CMP_GREATER等( -1:デフォルト動作に戻す )  TestParam:描画アルファ値との比較に使用する値 )
extern	int		Graphics_Hardware_SetDrawAlphaTest_PF( int TestMode, int TestParam )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawAlphaTest_PF(  TestMode,  TestParam ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawAlphaTest_PF(  TestMode,  TestParam ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画モードをセットする
extern	int		Graphics_Hardware_SetDrawMode_PF( int DrawMode )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawMode_PF(  DrawMode ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawMode_PF(  DrawMode ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画輝度をセット
extern	int		Graphics_Hardware_SetDrawBright_PF( int RedBright, int GreenBright, int BlueBright )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawBright_PF(  RedBright,  GreenBright,  BlueBright ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawBright_PF(  RedBright,  GreenBright,  BlueBright ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// SetBlendGraphParam の可変長引数パラメータ付き
extern	int		Graphics_Hardware_SetBlendGraphParamBase_PF( IMAGEDATA *BlendImage, int BlendType, int *Param )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetBlendGraphParamBase_PF(  BlendImage,  BlendType, Param ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetBlendGraphParamBase_PF(  BlendImage,  BlendType, Param ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 最大異方性の値をセットする
extern	int		Graphics_Hardware_SetMaxAnisotropy_PF( int MaxAnisotropy )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetMaxAnisotropy_PF(  MaxAnisotropy ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetMaxAnisotropy_PF(  MaxAnisotropy ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ワールド変換用行列をセットする
extern	int		Graphics_Hardware_SetTransformToWorld_PF( const MATRIX *Matrix )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetTransformToWorld_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetTransformToWorld_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ビュー変換用行列をセットする
extern	int		Graphics_Hardware_SetTransformToView_PF( const MATRIX *Matrix )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetTransformToView_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetTransformToView_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 投影変換用行列をセットする
extern	int		Graphics_Hardware_SetTransformToProjection_PF( const MATRIX *Matrix )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetTransformToProjection_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetTransformToProjection_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ビューポート行列をセットする
extern	int		Graphics_Hardware_SetTransformToViewport_PF( const MATRIX *Matrix )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetTransformToViewport_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetTransformToViewport_PF( Matrix ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// テクスチャアドレスモードを設定する
extern	int		Graphics_Hardware_SetTextureAddressMode_PF( int Mode /* DX_TEXADDRESS_WRAP 等 */, int Stage )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetTextureAddressMode_PF( Mode /* DX_TEXADDRESS_WRAP 等 */,  Stage ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetTextureAddressMode_PF( Mode /* DX_TEXADDRESS_WRAP 等 */,  Stage ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// テクスチャアドレスモードを設定する
extern	int		Graphics_Hardware_SetTextureAddressModeUV_PF( int ModeU, int ModeV, int Stage )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetTextureAddressModeUV_PF(  ModeU,  ModeV,  Stage ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetTextureAddressModeUV_PF(  ModeU,  ModeV,  Stage ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// テクスチャ座標変換行列をセットする
extern	int		Graphics_Hardware_SetTextureAddressTransformMatrix_PF( int UseFlag, MATRIX *Matrix, int Sampler )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetTextureAddressTransformMatrix_PF( UseFlag, Matrix, Sampler ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetTextureAddressTransformMatrix_PF( UseFlag, Matrix, Sampler ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// フォグを有効にするかどうかを設定する( TRUE:有効  FALSE:無効 )
extern	int		Graphics_Hardware_SetFogEnable_PF( int Flag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetFogEnable_PF(  Flag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetFogEnable_PF(  Flag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// フォグモードを設定する
extern	int		Graphics_Hardware_SetFogMode_PF( int Mode /* DX_FOGMODE_NONE 等 */ )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetFogMode_PF(  Mode /* DX_FOGMODE_NONE 等 */ ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetFogMode_PF(  Mode /* DX_FOGMODE_NONE 等 */ ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// フォグカラーを変更する
extern	int		Graphics_Hardware_SetFogColor_PF( DWORD FogColor )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetFogColor_PF( FogColor ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetFogColor_PF( FogColor ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// フォグが始まる距離と終了する距離を設定する( 0.0f 〜 1.0f )
extern	int		Graphics_Hardware_SetFogStartEnd_PF( float start, float end )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetFogStartEnd_PF(  start,  end ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetFogStartEnd_PF(  start,  end ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// フォグの密度を設定する( 0.0f 〜 1.0f )
extern	int		Graphics_Hardware_SetFogDensity_PF( float density )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetFogDensity_PF(  density ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetFogDensity_PF(  density ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ワールド変換用行列をセットする
extern	int		Graphics_Hardware_DeviceDirect_SetWorldMatrix_PF( const MATRIX *Matrix )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DeviceState_SetWorldMatrix( Matrix ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DeviceState_SetWorldMatrix( Matrix ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ビュー変換用行列をセットする
extern	int		Graphics_Hardware_DeviceDirect_SetViewMatrix_PF( const MATRIX *Matrix )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DeviceState_SetViewMatrix( Matrix ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DeviceState_SetViewMatrix( Matrix ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 投影変換用行列をセットする
extern	int		Graphics_Hardware_DeviceDirect_SetProjectionMatrix_PF( const MATRIX *Matrix )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DeviceState_SetProjectionMatrix( Matrix ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DeviceState_SetProjectionMatrix( Matrix ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 基本データに設定されているフォグ情報をハードウェアに反映する
extern	int		Graphics_Hardware_ApplyLigFogToHardware_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ApplyLigFogToHardware_PF( ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ApplyLigFogToHardware_PF( ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 以前の DrawModiGraph 関数のコードを使用するかどうかのフラグをセットする
extern	int		Graphics_Hardware_SetUseOldDrawModiGraphCodeFlag_PF( int Flag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetUseOldDrawModiGraphCodeFlag_PF( Flag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetUseOldDrawModiGraphCodeFlag_PF( Flag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画先に正しいα値を書き込むかどうかのフラグを更新する
extern	int		Graphics_Hardware_RefreshAlphaChDrawMode_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_RefreshAlphaChDrawMode_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_RefreshAlphaChDrawMode_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}























// 環境依存設定関係

// ハードウエアの頂点演算処理機能を使用するかどうかを設定する
extern	int		Graphics_Hardware_SetUseHardwareVertexProcessing_PF( int Flag )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_Hardware_D3D9_SetUseHardwareVertexProcessing_PF( Flag ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// ピクセル単位でライティングを行うかどうかを設定する、要 ShaderModel 3.0( TRUE:ピクセル単位のライティングを行う  FALSE:頂点単位のライティングを行う( デフォルト ) )
extern	int		Graphics_Hardware_SetUsePixelLighting_PF( int Flag )
{
#ifndef DX_NON_DIRECT3D11
	Graphics_Hardware_D3D11_SetUsePixelLighting_PF( Flag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	Graphics_Hardware_D3D9_SetUsePixelLighting_PF( Flag ) ;
#endif // DX_NON_DIRECT3D9

	return 0 ;
}

// グラフィックスデバイスがロストから復帰した際に呼ばれるコールバック関数を設定する
extern	int		Graphics_Hardware_SetGraphicsDeviceRestoreCallbackFunction_PF( void (* Callback )( void *Data ), void *CallbackData )
{
#ifndef DX_NON_DIRECT3D11
	Graphics_Hardware_D3D11_SetGraphicsDeviceRestoreCallbackFunction_PF( Callback, CallbackData ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	Graphics_Hardware_D3D9_SetGraphicsDeviceRestoreCallbackFunction_PF( Callback, CallbackData ) ;
#endif // DX_NON_DIRECT3D9

	return 0 ;
}

// グラフィックスデバイスがロストから復帰する前に呼ばれるコールバック関数を設定する
extern	int		Graphics_Hardware_SetGraphicsDeviceLostCallbackFunction_PF( void (* Callback )( void *Data ), void *CallbackData )
{
#ifndef DX_NON_DIRECT3D11
	Graphics_Hardware_D3D11_SetGraphicsDeviceLostCallbackFunction_PF( Callback, CallbackData ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	Graphics_Hardware_D3D9_SetGraphicsDeviceLostCallbackFunction_PF( Callback, CallbackData ) ;
#endif // DX_NON_DIRECT3D9

	return 0 ;
}

// 通常描画にプログラマブルシェーダーを使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )
extern	int		Graphics_Hardware_SetUseNormalDrawShader_PF( int Flag )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_Hardware_D3D9_SetUseNormalDrawShader_PF( Flag ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// ビデオメモリの容量を得る
extern	int		Graphics_Hardware_GetVideoMemorySize_PF( int *AllSize, int *FreeSize )
{
#ifndef DX_NON_DIRECT3D9
	return Graphics_Hardware_D3D9_GetVideoMemorySize_PF( AllSize, FreeSize ) ;
#else // DX_NON_DIRECT3D9
	return 0 ;
#endif // DX_NON_DIRECT3D9
}

// Vista以降の Windows Aero を無効にするかどうかをセットする、TRUE:無効にする  FALSE:有効にする( DxLib_Init の前に呼ぶ必要があります )
extern	int		Graphics_SetAeroDisableFlag_PF( int Flag )
{
	// 初期化前のみ有効
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	if( Flag )
	{
		GRAWIN.Setting.DisableAeroFlag = 2 ;
	}
	else
	{
		GRAWIN.Setting.DisableAeroFlag = 1 ;
	}

	// 終了
	return 0 ;
}























// 環境依存画面関係

// 設定に基づいて使用するＺバッファをセットする
extern	int		Graphics_Hardware_SetupUseZBuffer_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetupUseZBuffer_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetupUseZBuffer_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 画面のＺバッファの状態を初期化する
extern	int		Graphics_Hardware_ClearDrawScreenZBuffer_PF( const RECT *ClearRect )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ClearDrawScreenZBuffer_PF( ClearRect ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ClearDrawScreenZBuffer_PF( ClearRect ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 画面の状態を初期化する
extern	int		Graphics_Hardware_ClearDrawScreen_PF( const RECT *ClearRect )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ClearDrawScreen_PF( ClearRect ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ClearDrawScreen_PF( ClearRect ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画先画面のセット
extern	int		Graphics_Hardware_SetDrawScreen_PF( int DrawScreen, int OldScreenSurface, IMAGEDATA *NewTargetImage, IMAGEDATA *OldTargetImage, SHADOWMAPDATA *NewTargetShadowMap, SHADOWMAPDATA *OldTargetShadowMap )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawScreen_PF(  DrawScreen,  OldScreenSurface, NewTargetImage, OldTargetImage, NewTargetShadowMap, OldTargetShadowMap ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawScreen_PF(  DrawScreen,  OldScreenSurface, NewTargetImage, OldTargetImage, NewTargetShadowMap, OldTargetShadowMap ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// SetDrawScreen の最後で呼ばれる関数
extern	int		Graphics_Hardware_SetDrawScreen_Post_PF( int DrawScreen )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawScreen_Post_PF(  DrawScreen ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawScreen_Post_PF(  DrawScreen ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画可能領域のセット
extern	int		Graphics_Hardware_SetDrawArea_PF( int x1, int y1, int x2, int y2 )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawArea_PF(  x1,  y1,  x2,  y2 ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawArea_PF(  x1,  y1,  x2,  y2 ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画先バッファをロックする
extern	int		Graphics_Hardware_LockDrawScreenBuffer_PF( RECT *LockRect, BASEIMAGE *BaseImage, int TargetScreen, IMAGEDATA *TargetImage, int TargetScreenSurface, int ReadOnly, int TargetScreenTextureNo )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_LockDrawScreenBuffer_PF( LockRect, BaseImage,  TargetScreen, TargetImage, TargetScreenSurface,  ReadOnly,  TargetScreenTextureNo ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_LockDrawScreenBuffer_PF( LockRect, BaseImage,  TargetScreen, TargetImage, TargetScreenSurface,  ReadOnly,  TargetScreenTextureNo ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画先バッファをアンロックする
extern	int		Graphics_Hardware_UnlockDrawScreenBuffer_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_UnlockDrawScreenBuffer_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_UnlockDrawScreenBuffer_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 裏画面の内容を表画面に描画する
extern	int		Graphics_Hardware_ScreenCopy_PF( int DrawTargetFrontScreenMode_Copy )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ScreenCopy_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ScreenCopy_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ディスプレイ列挙用のコールバック関数
BOOL CALLBACK Graphics_MonitorEnumProc(
	HMONITOR hMonitor,    // ディスプレイモニタのハンドル
	HDC      hdcMonitor,  // モニタに適したデバイスコンテキストのハンドル
	LPRECT   lprcMonitor, // モニタ上の交差部分を表す長方形領域へのポインタ
	LPARAM   dwData       // EnumDisplayMonitors から渡されたデータ
)
{
	MONITORINFOEXW          MonitorInfoEx ;
	GRAPHICSSYS_DISPLAYINFO *DisplayInfo ;
	HDC                     hdc ;

	if( GSYS.Screen.DisplayInfo != NULL )
	{
		DisplayInfo = &GSYS.Screen.DisplayInfo[ GSYS.Screen.DisplayNum ] ;

		_MEMSET( &MonitorInfoEx, 0, sizeof( MonitorInfoEx ) ) ;
		MonitorInfoEx.cbSize = sizeof( MonitorInfoEx ) ;
		GetMonitorInfoW( hMonitor, &MonitorInfoEx ) ;

		DisplayInfo->DesktopRect = MonitorInfoEx.rcMonitor ;
		_WCSCPY( DisplayInfo->Name, MonitorInfoEx.szDevice ) ;

		hdc = CreateDCW( DisplayInfo->Name, NULL, NULL, NULL ) ;
		DisplayInfo->DesktopSizeX         = GetDeviceCaps( hdc, HORZRES ) ;
		DisplayInfo->DesktopSizeY         = GetDeviceCaps( hdc, VERTRES ) ;
		DisplayInfo->DesktopColorBitDepth = GetDeviceCaps( hdc , PLANES ) * GetDeviceCaps( hdc , BITSPIXEL ) ;
		DisplayInfo->DesktopRefreshRate   = GetDeviceCaps( hdc , VREFRESH ) ;
		DeleteDC( hdc ) ;

		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4e\x00\x6f\x00\x2e\x00\x25\x00\x64\x00\x20\x00\x20\x00\xe2\x30\xcb\x30\xbf\x30\xfc\x30\x0d\x54\x1a\xff\x25\x00\x73\x00\x00\x30\x25\x00\x64\x00\x78\x00\x25\x00\x64\x00\x00\x30\x25\x00\x64\x00\x62\x00\x69\x00\x74\x00\x00\x30\x25\x00\x64\x00\x48\x00\x7a\x00\x00"/*@ L"No.%d  モニター名：%s　%dx%d　%dbit　%dHz" @*/, GSYS.Screen.DisplayNum, DisplayInfo->Name, DisplayInfo->DesktopSizeX, DisplayInfo->DesktopSizeY, DisplayInfo->DesktopColorBitDepth, DisplayInfo->DesktopRefreshRate )) ;
	}

	GSYS.Screen.DisplayNum ++ ;

	return TRUE ;
}

// ディスプレイの情報をセットアップする
extern int Graphics_SetupDisplayInfo_PF( void )
{
	DWORD                    i ;
	DWORD                    j ;
	DISPLAY_DEVICEW          DisplayDevice ;
	GRAPHICSSYS_DISPLAYINFO *DisplayInfo ;
	wchar_t *                DeviceName ;
	DWORD                    DisplayDeviceNum ;

	OSVERSIONINFO OsVersionInfo ;

	DXST_ERRORLOGFMT_ADDUTF16LE(( "\xc7\x30\xa3\x30\xb9\x30\xd7\x30\xec\x30\xa4\x30\xc5\x60\x31\x58\x6e\x30\xbb\x30\xc3\x30\xc8\x30\xa2\x30\xc3\x30\xd7\x30\x8b\x95\xcb\x59\x00"/*@ L"ディスプレイ情報のセットアップ開始" @*/ )) ;
	DXST_ERRORLOG_TABADD ;

	_MEMSET( &OsVersionInfo, 0, sizeof( OsVersionInfo ) ) ;
	OsVersionInfo.dwOSVersionInfoSize = sizeof( OsVersionInfo ) ;

	GetVersionEx( &OsVersionInfo ) ;

	if( OsVersionInfo.dwPlatformId >= VER_PLATFORM_WIN32_NT &&
		OsVersionInfo.dwMajorVersion >= 5 )
	{
		_MEMSET( &DisplayDevice, 0, sizeof( DisplayDevice ) ) ;
		DisplayDevice.cb = sizeof( DisplayDevice ) ;

		GSYS.Screen.DisplayNum = 0 ;
		EnumDisplayMonitors( NULL, NULL, Graphics_MonitorEnumProc, NULL ) ;

		// API を読み込む
		if( WinAPIData.Win32Func.EnumDisplayDevicesWFunc == NULL )
		{
			LoadWinAPI() ;
		}

		DisplayDeviceNum = 0 ;
		while( WinAPIData.Win32Func.EnumDisplayDevicesWFunc( NULL, DisplayDeviceNum, &DisplayDevice, 0 ) != 0 )
		{
			DisplayDeviceNum ++ ;
		}
	}
	else
	{
		GSYS.Screen.DisplayNum = 1 ;
		DisplayDeviceNum       = 1 ;
	}

	DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe2\x30\xcb\x30\xbf\x30\xfc\x30\x6e\x30\x70\x65\x1a\xff\x25\x00\x64\x00\x20\x00\x20\x00\xc7\x30\xa3\x30\xb9\x30\xd7\x30\xec\x30\xa4\x30\xc7\x30\xd0\x30\xa4\x30\xb9\x30\x6e\x30\x70\x65\x1a\xff\x25\x00\x64\x00\x00"/*@ L"モニターの数：%d  ディスプレイデバイスの数：%d" @*/, GSYS.Screen.DisplayNum, DisplayDeviceNum )) ;

	GSYS.Screen.DisplayInfo = ( GRAPHICSSYS_DISPLAYINFO * )HeapAlloc( GetProcessHeap(), 0, sizeof( GRAPHICSSYS_DISPLAYINFO ) * GSYS.Screen.DisplayNum ) ;
	if( GSYS.Screen.DisplayInfo == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x5b\x00\xa8\x30\xe9\x30\xfc\x30\x5d\x00\xc7\x30\xa3\x30\xb9\x30\xd7\x30\xec\x30\xa4\x30\xc5\x60\x31\x58\xdd\x4f\x58\x5b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"[エラー]ディスプレイ情報保存用のメモリ確保に失敗しました" @*/ )) ;
		DXST_ERRORLOG_TABSUB ;
		return -1 ;
	}
	_MEMSET( GSYS.Screen.DisplayInfo, 0, sizeof( GRAPHICSSYS_DISPLAYINFO ) * GSYS.Screen.DisplayNum ) ;

	if( OsVersionInfo.dwPlatformId >= VER_PLATFORM_WIN32_NT &&
		OsVersionInfo.dwMajorVersion >= 5 )
	{
		GSYS.Screen.DisplayNum = 0 ;
		EnumDisplayMonitors( NULL, NULL, Graphics_MonitorEnumProc, NULL ) ;
	}
	else
	{
		HDC hdc ;

		DisplayInfo = GSYS.Screen.DisplayInfo ;

		hdc = GetDC( NULL ) ;

		DisplayInfo->DesktopSizeX         = GetDeviceCaps( hdc, HORZRES ) ;
		DisplayInfo->DesktopSizeY         = GetDeviceCaps( hdc, VERTRES ) ;
		DisplayInfo->DesktopColorBitDepth = GetDeviceCaps( hdc , PLANES ) * GetDeviceCaps( hdc , BITSPIXEL ) ;
		DisplayInfo->DesktopRefreshRate   = GetDeviceCaps( hdc , VREFRESH ) ;

		ReleaseDC( NULL, hdc ) ;

		DisplayInfo->DesktopRect.left   = 0 ;
		DisplayInfo->DesktopRect.top    = 0 ;
		DisplayInfo->DesktopRect.right  = DisplayInfo->DesktopSizeX ;
		DisplayInfo->DesktopRect.bottom = DisplayInfo->DesktopSizeY ;
		_WCSCPY( DisplayInfo->Name, L"DISPLAY" ) ;

		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4e\x00\x6f\x00\x2e\x00\x25\x00\x64\x00\x20\x00\x20\x00\xe2\x30\xcb\x30\xbf\x30\xfc\x30\x0d\x54\x1a\xff\x25\x00\x73\x00\x00\x30\x25\x00\x64\x00\x78\x00\x25\x00\x64\x00\x00\x30\x25\x00\x64\x00\x62\x00\x69\x00\x74\x00\x00\x30\x25\x00\x64\x00\x48\x00\x7a\x00\x00"/*@ L"No.%d  モニター名：%s　%dx%d　%dbit　%dHz" @*/, 0, DisplayInfo->Name, DisplayInfo->DesktopSizeX, DisplayInfo->DesktopSizeY, DisplayInfo->DesktopColorBitDepth, DisplayInfo->DesktopRefreshRate )) ;
	}

	for( j = 0 ; j < DisplayDeviceNum ; j ++ )
	{
		DWORD           ModeNum ;
		DEVMODEW        DevMode ;
		DISPLAYMODEDATA *ModeData ;

		if( OsVersionInfo.dwPlatformId >= VER_PLATFORM_WIN32_NT &&
			OsVersionInfo.dwMajorVersion >= 5 )
		{
			WinAPIData.Win32Func.EnumDisplayDevicesWFunc( NULL, j, &DisplayDevice, 0 ) ;
			DeviceName = DisplayDevice.DeviceName ;

			DisplayInfo = GSYS.Screen.DisplayInfo ;
			for( i = 0 ; i < ( DWORD )GSYS.Screen.DisplayNum ; i ++, DisplayInfo ++ )
			{
				if( _WCSCMP( DisplayInfo->Name, DeviceName ) == 0 )
				{
					break ;
				}
			}
			if( i == ( DWORD )GSYS.Screen.DisplayNum )
			{
				continue ;
			}
		}
		else
		{
			DisplayInfo = GSYS.Screen.DisplayInfo ;
			DeviceName = NULL ;
		}

//		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4e\x00\x6f\x00\x2e\x00\x25\x00\x64\x00\x20\x00\x20\x00\xc7\x30\xa3\x30\xb9\x30\xd7\x30\xec\x30\xa4\x30\xc7\x30\xd0\x30\xa4\x30\xb9\x30\x0d\x54\x1a\xff\x25\x00\x73\x00\x00"/*@ L"No.%d  ディスプレイデバイス名：%s" @*/, j, DeviceName == NULL ? L"None" : DeviceName )) ;

		// 数を数える
		DisplayInfo->ModeNum = 0 ;
		ModeNum              = 0 ;
		for( ModeNum ;  ; ModeNum ++ )
		{
			_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
			if( EnumDisplaySettingsW( DeviceName, ModeNum, &DevMode ) == 0 )
			{
				break ;
			}

			// 16bit 以下のモードは除外する
			if( DevMode.dmBitsPerPel >= 16 )
			{
				DisplayInfo->ModeNum ++ ;
			}
		}

		// 情報を格納するメモリ領域の確保
		DisplayInfo->ModeData = ( DISPLAYMODEDATA * )HeapAlloc( GetProcessHeap(), 0, sizeof( DISPLAYMODEDATA ) * DisplayInfo->ModeNum ) ;
		if( DisplayInfo->ModeData == NULL )
		{
			DisplayInfo->ModeNum = 0 ;
			return -1 ;
		}

		// 情報を取得する
		ModeData = DisplayInfo->ModeData ;
		for( i = 0 ; i < ModeNum ; i ++ )
		{
			_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
			EnumDisplaySettingsW( DeviceName, i, &DevMode ) ;
			if( DevMode.dmBitsPerPel >= 16 )
			{
				ModeData->Width         = ( int )DevMode.dmPelsWidth ;
				ModeData->Height        = ( int )DevMode.dmPelsHeight ;
				ModeData->ColorBitDepth = ( int )DevMode.dmBitsPerPel ;
				ModeData->RefreshRate   = ( int )DevMode.dmDisplayFrequency ;
//				DXST_ERRORLOGFMT_ADDUTF16LE(( "\x3b\x75\x62\x97\xe2\x30\xfc\x30\xc9\x30\x4e\x00\x6f\x00\x2e\x00\x25\x00\x64\x00\x20\x00\x20\x00\x25\x00\x64\x00\x78\x00\x25\x00\x64\x00\x20\x00\x25\x00\x64\x00\x62\x00\x69\x00\x74\x00\x20\x00\x25\x00\x64\x00\x48\x00\x7a\x00\x00"/*@ L"画面モードNo.%d  %dx%d %dbit %dHz" @*/, i, ModeData->Width, ModeData->Height, ModeData->ColorBitDepth, ModeData->RefreshRate )) ;
				ModeData ++ ;
			}
		}
	}

	DXST_ERRORLOG_TABSUB ;
	DXST_ERRORLOGFMT_ADDUTF16LE(( "\xc7\x30\xa3\x30\xb9\x30\xd7\x30\xec\x30\xa4\x30\xc5\x60\x31\x58\x6e\x30\xbb\x30\xc3\x30\xc8\x30\xa2\x30\xc3\x30\xd7\x30\x8c\x5b\x86\x4e\x00"/*@ L"ディスプレイ情報のセットアップ完了" @*/ )) ;

	return 0 ;
}

// 垂直同期信号を待つ
extern	int		Graphics_Hardware_WaitVSync_PF( int SyncNum )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_WaitVSync_PF( SyncNum ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_WaitVSync_PF( SyncNum ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 裏画面と表画面を交換する
extern	int		Graphics_ScreenFlipBase_PF( void )
{
	// ハードウエア機能を使っているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		switch( GRAWIN.Setting.UseGraphicsAPI )
		{
#ifndef DX_NON_DIRECT3D11
		case GRAPHICS_API_DIRECT3D11_WIN32 :
			return Graphics_Hardware_D3D11_ScreenFlipBase_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
		case GRAPHICS_API_DIRECT3D9_WIN32 :
			return Graphics_Hardware_D3D9_ScreenFlipBase_PF() ;
#endif // DX_NON_DIRECT3D9

		default :
			return 0 ;
		}
	}
	else
	{
		// ソフトウエアレンダリングの場合
		RECT        WindRect ;
		char        Buffer[ sizeof( BITMAPINFOHEADER ) + sizeof( DWORD ) * 3 + 4 ] ;
		BITMAPINFO *pBmpInfo ;
		HDC         Dc ;
		int         DrawScreenWidth ;
		int         DrawScreenHeight ;

		// ウインドウモードの場合は転送先ウインドウのデスクトップ上の座標を割り出す
		if( NS_GetWindowModeFlag() == TRUE )
		{
			int    ClientHeight ;
			double ExRateX ;
			double ExRateY ;

			WindRect.left   = 0 ;
			WindRect.top    = 0 ;

			// 現在のウインドウのクライアント領域のサイズを得る
			NS_GetWindowSizeExtendRate( &ExRateX, &ExRateY ) ;
			NS_GetDrawScreenSize( &DrawScreenWidth, &DrawScreenHeight ) ;
			WindRect.right  = _DTOL( DrawScreenWidth  * ExRateX ) ;
			WindRect.bottom = _DTOL( DrawScreenHeight * ExRateY ) ;

			if( WinData.ToolBarUseFlag )
			{
				ClientHeight    = WinData.WindowRect.bottom - WinData.WindowRect.top ;
				WindRect.top    = ClientHeight - DrawScreenHeight ;
				WindRect.bottom = ClientHeight ;
			}
		}
		else
		{
			WindRect.left   = 0 ;
			WindRect.top    = 0 ;
			WindRect.right  = GSYS.Screen.MainScreenSizeX ;
			WindRect.bottom = GSYS.Screen.MainScreenSizeY ;
		}

		if( CheckValidMemImg( &GSYS.SoftRender.MainBufferMemImg ) == FALSE ||
			CheckValidMemImg( &GSYS.SoftRender.SubBufferMemImg  ) == FALSE )
		{
			return 0 ;
		}

		// ＶＳＹＮＣを待つ	
		if( GSYS.Screen.NotWaitVSyncFlag == FALSE )
		{
			DirectDraw7_LocalWaitVSync() ;
		}

		// バックバッファの透過色の部分を透過するフラグか、UpdateLayerdWindow を使用するフラグが立っている場合は処理を分岐
		if( WinData.BackBufferTransColorFlag && WinData.UseUpdateLayerdWindowFlag )
		{
			BASEIMAGE BackBufferImage ;
			RECT      LockRect ;
			int       OldTargetScreen ;
			int       OldTargetScreenSurface ;

			// ＶＳＹＮＣを待つ	
			if( GSYS.Screen.NotWaitVSyncFlag == FALSE ) DirectDraw7_LocalWaitVSync() ;

			// Graphics_Screen_LockDrawScreen を使う方法
			OldTargetScreen                           = GSYS.DrawSetting.TargetScreen[ 0 ] ;
			OldTargetScreenSurface                    = GSYS.DrawSetting.TargetScreenSurface[ 0 ] ;
			GSYS.DrawSetting.TargetScreen[ 0 ]        = DX_SCREEN_BACK ;
			GSYS.DrawSetting.TargetScreenSurface[ 0 ] = 0 ;
			LockRect.left                             = 0 ;
			LockRect.top                              = 0 ;
			LockRect.right                            = GSYS.Screen.MainScreenSizeX ;
			LockRect.bottom                           = GSYS.Screen.MainScreenSizeY ;
			if( Graphics_Screen_LockDrawScreen( &LockRect, &BackBufferImage, -1, -1, TRUE, 0 ) < 0 )
			{
				return -1 ;
			}
			UpdateBackBufferTransColorWindow( &BackBufferImage ) ;
			Graphics_Screen_UnlockDrawScreen() ;
			GSYS.DrawSetting.TargetScreen[ 0 ]        = OldTargetScreen ;
			GSYS.DrawSetting.TargetScreenSurface[ 0 ] = OldTargetScreenSurface ;
		}
		else
		{
			MEMIMG *UseScreenMemImg ;

			// 画面に転送する領域を限定する場合は最初にサブバッファに転送する
			if( GSYS.Screen.ValidGraphDisplayArea )
			{
				RECT   SrcRect ;
				RECT   ClipRect ;
				MEMIMG BltSrcMemImg ;

				// サブバッファに転送する領域を設定する
				ClipRect.left   = 0 ;
				ClipRect.top    = 0 ;
				ClipRect.right  = GSYS.Screen.MainScreenSizeX ;
				ClipRect.bottom = GSYS.Screen.MainScreenSizeY ;

				SrcRect = GSYS.Screen.GraphDisplayArea ;
				RectClipping( &SrcRect, &ClipRect ) ;
				if( SrcRect.right  - SrcRect.left <= 0 ||
					SrcRect.bottom - SrcRect.top  <= 0 )
				{
					return -1 ;
				}

				// 抜き出す部分のみの MEMIMG を作成
				DerivationMemImg(
					&BltSrcMemImg,
					&GSYS.SoftRender.MainBufferMemImg,
					SrcRect.left,
					SrcRect.top,
					SrcRect.right  - SrcRect.left,
					SrcRect.bottom - SrcRect.top
				) ;

				// サブバッファ全体に転送
				DrawEnlargeMemImg(
					&GSYS.SoftRender.SubBufferMemImg,
					&BltSrcMemImg,
					&ClipRect,
					FALSE,
					NULL
				) ;

				UseScreenMemImg = &GSYS.SoftRender.SubBufferMemImg ;
			}
			else
			{
				UseScreenMemImg = &GSYS.SoftRender.MainBufferMemImg ;
			}

			// メインウインドウのＤＣを取得して転送する
			pBmpInfo = ( BITMAPINFO * )Buffer ;
			_MEMSET( Buffer, 0, sizeof( Buffer ) ) ;
			pBmpInfo->bmiHeader.biSize               = sizeof( pBmpInfo->bmiHeader ) ;
			pBmpInfo->bmiHeader.biWidth              =  GSYS.Screen.MainScreenSizeX ;
			pBmpInfo->bmiHeader.biHeight             = -GSYS.Screen.MainScreenSizeY ;
			pBmpInfo->bmiHeader.biPlanes             = 1 ;
			pBmpInfo->bmiHeader.biBitCount           = UseScreenMemImg->Base->ColorDataP->ColorBitDepth ;
			pBmpInfo->bmiHeader.biCompression        = BI_BITFIELDS ;
			( ( DWORD * )&pBmpInfo->bmiColors )[ 0 ] = UseScreenMemImg->Base->ColorDataP->RedMask ;
			( ( DWORD * )&pBmpInfo->bmiColors )[ 1 ] = UseScreenMemImg->Base->ColorDataP->GreenMask ;
			( ( DWORD * )&pBmpInfo->bmiColors )[ 2 ] = UseScreenMemImg->Base->ColorDataP->BlueMask ;

			// ピッチが１６バイトの倍数なので補正
			if( UseScreenMemImg->Base->ColorDataP->ColorBitDepth == 16 )
			{
				// 16bit
				pBmpInfo->bmiHeader.biWidth = ( pBmpInfo->bmiHeader.biWidth + 16 / 2 - 1 ) / ( 16 / 2 ) * ( 16 / 2 ) ;
			}
			else
			{
				// 32bit
				pBmpInfo->bmiHeader.biWidth = ( pBmpInfo->bmiHeader.biWidth + 16 / 4 - 1 ) / ( 16 / 4 ) * ( 16 / 4 ) ;
			}

			Dc = GetDC( GRAWIN.Setting.ScreenFlipTargetWindow ? GRAWIN.Setting.ScreenFlipTargetWindow : GetDisplayWindowHandle() ) ;
			if( Dc )
			{
				// ウインドウモードで３２０×２４０エミュレーションモードの場合は転送元と転送先のパラメータは固定
				if( NS_GetWindowModeFlag() && GSYS.Screen.Emulation320x240Flag )
				{
					StretchDIBits(
						Dc,
						0,
						0,
						640,
						480, 
						0,
						0,
						320,
						240,
						UseScreenMemImg->UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
				}
				else
				{
					if( NS_GetWindowModeFlag() == FALSE )
					{
						// フルスクリーンモードの場合

						// 画面全体を黒く塗りつぶす指定があったら実行する
						if( GRAWIN.FullScreenBlackFillCounter > 0 )
						{
							GRAWIN.FullScreenBlackFillCounter -- ;

							// ウインドウの座標を左上端に移動する
							NS_SetWindowPosition( 0, 0 ) ;

							// 画面全体を黒で塗りつぶす
							{
								HBRUSH OldBrush;

								OldBrush = ( HBRUSH )SelectObject( Dc, GetStockObject( BLACK_BRUSH ) ) ;
								Rectangle(
									Dc,
									0,
									0,
									GSYS.Screen.FullScreenUseDispModeData.Width,
									GSYS.Screen.FullScreenUseDispModeData.Height
								) ;
								SelectObject( Dc, OldBrush) ;
							}
						}

						switch( GSYS.Screen.FullScreenResolutionModeAct )
						{
						case DX_FSRESOLUTIONMODE_NATIVE :
							SetDIBitsToDevice(
								Dc,
								0, 0,
								( DWORD )GSYS.Screen.MainScreenSizeX, ( DWORD )GSYS.Screen.MainScreenSizeY,
								0, 0,
								0, ( UINT )GSYS.Screen.MainScreenSizeY, 
								UseScreenMemImg->UseImage, pBmpInfo, DIB_RGB_COLORS );
							break ;

						case DX_FSRESOLUTIONMODE_MAXIMUM :
						case DX_FSRESOLUTIONMODE_DESKTOP :
							// 転送先矩形のセットアップ
							Graphics_Screen_SetupFullScreenScalingDestRect() ;

//							DXST_ERRORLOGFMT_ADDUTF16LE(( L"GSYS.Screen.FullScreenScalingDestRect %d %d %d %d",
//								GSYS.Screen.FullScreenScalingDestRect.left,
//								GSYS.Screen.FullScreenScalingDestRect.top,
//								GSYS.Screen.FullScreenScalingDestRect.right,
//								GSYS.Screen.FullScreenScalingDestRect.bottom
//							)) ;

							StretchDIBits(
								Dc,
								GSYS.Screen.FullScreenScalingDestRect.left,
								GSYS.Screen.FullScreenScalingDestRect.top,
								GSYS.Screen.FullScreenScalingDestRect.right  - GSYS.Screen.FullScreenScalingDestRect.left,
								GSYS.Screen.FullScreenScalingDestRect.bottom - GSYS.Screen.FullScreenScalingDestRect.top,
								0,
								0,
								GSYS.Screen.MainScreenSizeX,
								GSYS.Screen.MainScreenSizeY,
								UseScreenMemImg->UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
							break ;
						}
					}
					else
					{
						int WindowSizeX ;
						int WindowSizeY ;

						WindowSizeX = WindRect.right  - WindRect.left ;
						WindowSizeY = WindRect.bottom - WindRect.top ;

						// ウインドウモードの場合
						if( GSYS.Screen.MainScreenSizeX == WindowSizeX && GSYS.Screen.MainScreenSizeY == WindowSizeY )
						{
							SetDIBitsToDevice(
								Dc,
								0, 0,
								( DWORD )WindowSizeX, ( DWORD )WindowSizeY,
								0, 0,
								0, ( UINT )WindowSizeY, 
								UseScreenMemImg->UseImage, pBmpInfo, DIB_RGB_COLORS );
						}
						else
						{
							StretchDIBits(
								Dc,
								0, 0,
								WindowSizeX, WindowSizeY, 
								0, 0,
								GSYS.Screen.MainScreenSizeX, GSYS.Screen.MainScreenSizeY,
								UseScreenMemImg->UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
						}
					}
				}

				ReleaseDC( GetDisplayWindowHandle(), Dc );
			}
		}
	}

	return 0 ;
}

// 裏画面の指定の領域をウインドウのクライアント領域の指定の領域に転送する
extern	int		Graphics_BltRectBackScreenToWindow_PF( HWND Window, RECT BackScreenRect, RECT WindowClientRect )
{
	// 矩形チェック
	if( BackScreenRect.left   == BackScreenRect.right    ||
		BackScreenRect.top    == BackScreenRect.bottom   ||
		WindowClientRect.left == WindowClientRect.right  ||
		WindowClientRect.top  == WindowClientRect.bottom )
	{
		return -1 ;
	}

	// ハードウエア機能を使っているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		// 使っている場合

		switch( GRAWIN.Setting.UseGraphicsAPI )
		{
#ifndef DX_NON_DIRECT3D11
		case GRAPHICS_API_DIRECT3D11_WIN32 :
			return Graphics_Hardware_D3D11_BltRectBackScreenToWindow_PF(  Window, BackScreenRect, WindowClientRect ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
		case GRAPHICS_API_DIRECT3D9_WIN32 :
			return Graphics_Hardware_D3D9_BltRectBackScreenToWindow_PF(  Window, BackScreenRect, WindowClientRect ) ;
#endif // DX_NON_DIRECT3D9

		default :
			return 0 ;
		}
	}
	else
	{
		// ソフトウエアレンダリングの場合
		char Buffer[ sizeof( BITMAPINFOHEADER ) + sizeof( DWORD ) * 3 + 4 ];
		BITMAPINFO *pBmpInfo;
		HDC Dc;

		if( CheckValidMemImg( &GSYS.SoftRender.MainBufferMemImg ) == FALSE ) return 0 ;

		// ＶＳＹＮＣを待つ	
		if( GSYS.Screen.NotWaitVSyncFlag == FALSE )
		{
			DirectDraw7_LocalWaitVSync() ;
		}

		// ウインドウのＤＣを取得して転送する
		pBmpInfo = (BITMAPINFO *)Buffer ;
		_MEMSET( Buffer, 0, sizeof( Buffer ) ) ;
		pBmpInfo->bmiHeader.biSize        = sizeof( pBmpInfo->bmiHeader ) ;
		pBmpInfo->bmiHeader.biWidth       =  GSYS.Screen.MainScreenSizeX ;
		pBmpInfo->bmiHeader.biHeight      = -GSYS.Screen.MainScreenSizeY ;
		pBmpInfo->bmiHeader.biPlanes      = 1;
		pBmpInfo->bmiHeader.biBitCount    = GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP->ColorBitDepth ;
		pBmpInfo->bmiHeader.biCompression = BI_BITFIELDS ;
		( ( DWORD * )&pBmpInfo->bmiColors )[ 0 ] = GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP->RedMask ;
		( ( DWORD * )&pBmpInfo->bmiColors )[ 1 ] = GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP->GreenMask ;
		( ( DWORD * )&pBmpInfo->bmiColors )[ 2 ] = GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP->BlueMask ;
		Dc = GetDC( Window );
		if( Dc )
		{
			StretchDIBits(
				Dc,
				WindowClientRect.left, WindowClientRect.top,
				WindowClientRect.right - WindowClientRect.left,
				WindowClientRect.bottom - WindowClientRect.top,
				BackScreenRect.left, GSYS.Screen.MainScreenSizeY - BackScreenRect.bottom,
				BackScreenRect.right  - BackScreenRect.left,
				BackScreenRect.bottom - BackScreenRect.top,
				( BYTE * )GSYS.SoftRender.MainBufferMemImg.UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
			ReleaseDC( Window, Dc );
		}
	}

	// 終了
	return 0 ;
}

// ScreenFlip で画像を転送する先のウインドウを設定する( NULL を指定すると設定解除 )
extern	int		Graphics_SetScreenFlipTargetWindow_PF( HWND TargetWindow )
{
	GRAWIN.Setting.ScreenFlipTargetWindow = TargetWindow ;

	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetScreenFlipTargetWindow_PF( TargetWindow ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetScreenFlipTargetWindow_PF( TargetWindow ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// メイン画面のＺバッファの設定を変更する
extern	int		Graphics_Hardware_SetZBufferMode_PF( int ZBufferSizeX, int ZBufferSizeY, int ZBufferBitDepth )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetZBufferMode_PF(  ZBufferSizeX,  ZBufferSizeY,  ZBufferBitDepth ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetZBufferMode_PF(  ZBufferSizeX,  ZBufferSizeY,  ZBufferBitDepth ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 画面のフルシーンアンチエイリアスモードの設定をする
extern	int		Graphics_Hardware_SetFullSceneAntiAliasingMode_PF( int Samples, int Quality )
{
	// 設定できるのは初期化前のみ
	if( DxSysData.DxLib_InitializeFlag == TRUE )
		return 0 ;

	GRAWIN.Setting.FSAAMultiSampleCount   = Samples ;
	GRAWIN.Setting.FSAAMultiSampleQuality = Quality ;

	// 終了
	return 0 ;
}

// 描画先Ｚバッファのセット
extern	int		Graphics_Hardware_SetDrawZBuffer_PF( int DrawScreen, IMAGEDATA *Image )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_SetDrawZBuffer_PF( DrawScreen, Image ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_SetDrawZBuffer_PF( DrawScreen, Image ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 現在の画面のリフレッシュレートを取得する
extern	int		Graphics_GetRefreshRate_PF( void )
{
	int RefreshRate = -1 ;

	// リフレッシュレートの取得
//	if( WinData.WindowsVersion >= DX_WINDOWSVERSION_NT31 )
	{
		HDC hdc ;
		
		hdc = GetDC( NS_GetMainWindowHandle() ) ;
		if( hdc )
		{
			RefreshRate = GetDeviceCaps( hdc, VREFRESH ) ;
			ReleaseDC( NS_GetMainWindowHandle(), hdc ) ;

			if( RefreshRate < 20 ) RefreshRate = -1 ;
		}
	}

	return RefreshRate ;
}






















// 環境依存情報取得関係

// GetColor や GetColor2 で使用するカラーデータを取得する
extern	const COLORDATA *	Graphics_Hardware_GetMainColorData_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_GetMainColorData_PF(  ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GetMainColorData_PF(  ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ディスプレーのカラーデータポインタを得る
extern	const COLORDATA *	Graphics_Hardware_GetDispColorData_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_GetDispColorData_PF(  ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GetDispColorData_PF(  ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 指定座標の色を取得する
extern	DWORD				Graphics_Hardware_GetPixel_PF( int x, int y )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_GetPixel_PF(  x,  y ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GetPixel_PF(  x,  y ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}


























// 環境依存画像関係

// YUVサーフェスを使った Theora 動画の内容をグラフィックスハンドルのテクスチャに転送する
extern	int		Graphics_Hardware_UpdateGraphMovie_TheoraYUV_PF( MOVIEGRAPH *Movie, IMAGEDATA *Image )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_UpdateGraphMovie_TheoraYUV_PF( Movie, Image ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_UpdateGraphMovie_TheoraYUV_PF( Movie, Image ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// グラフィックメモリ領域のロック
extern	int		Graphics_Hardware_GraphLock_PF( IMAGEDATA *Image, COLORDATA **ColorDataP, int WriteOnly )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_GraphLock_PF( Image, ColorDataP, WriteOnly ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GraphLock_PF( Image, ColorDataP, WriteOnly ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// グラフィックメモリ領域のロック解除
extern	int		Graphics_Hardware_GraphUnlock_PF( IMAGEDATA *Image )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_GraphUnlock_PF( Image ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GraphUnlock_PF( Image ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// グラフィックのＺバッファの状態を別のグラフィックのＺバッファにコピーする( DestGrHandle も SrcGrHandle もＺバッファを持っている描画可能画像で、且つアンチエイリアス画像ではないことが条件 )
extern	int		Graphics_Hardware_CopyGraphZBufferImage_PF( IMAGEDATA *DestImage, IMAGEDATA *SrcImage )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_CopyGraphZBufferImage_PF( DestImage, SrcImage ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_CopyGraphZBufferImage_PF( DestImage, SrcImage ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 画像データの初期化
extern	int		Graphics_Hardware_InitGraph_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_InitGraph_PF( ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_InitGraph_PF( ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// グラフィックを特定の色で塗りつぶす
extern	int		Graphics_Hardware_FillGraph_PF( IMAGEDATA *Image, int Red, int Green, int Blue, int Alpha, int ASyncThread )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_FillGraph_PF( Image,  Red,  Green,  Blue,  Alpha,  ASyncThread ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_FillGraph_PF( Image,  Red,  Green,  Blue,  Alpha,  ASyncThread ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画可能画像やバックバッファから指定領域のグラフィックを取得する
extern	int		Graphics_Hardware_GetDrawScreenGraphBase_PF( IMAGEDATA *Image, IMAGEDATA *TargetImage, int TargetScreen, int TargetScreenSurface, int TargetScreenWidth, int TargetScreenHeight, int x1, int y1, int x2, int y2, int destX, int destY )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_GetDrawScreenGraphBase_PF( Image, TargetImage,  TargetScreen,  TargetScreenSurface,  TargetScreenWidth,  TargetScreenHeight,  x1,  y1,  x2,  y2,  destX,  destY ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GetDrawScreenGraphBase_PF( Image, TargetImage,  TargetScreen,  TargetScreenSurface,  TargetScreenWidth,  TargetScreenHeight,  x1,  y1,  x2,  y2,  destX,  destY ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// Graphics_Image_BltBmpOrBaseImageToGraph3 の機種依存部分用関数
extern int Graphics_Hardware_BltBmpOrBaseImageToGraph3_PF(
	const RECT		*SrcRect,
	      int		DestX,
	      int		DestY,
	      int		GrHandle,
	const BASEIMAGE	*RgbBaseImage,
	const BASEIMAGE	*AlphaBaseImage,
	      int		RedIsAlphaFlag,
	      int		UseTransColorConvAlpha,
	      int		TargetOrig,
	      int		ASyncThread
)
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_BltBmpOrBaseImageToGraph3_PF(
			SrcRect,
			DestX,
			DestY,
			GrHandle,
			RgbBaseImage,
			AlphaBaseImage,
			RedIsAlphaFlag,
			UseTransColorConvAlpha,
			TargetOrig,
			ASyncThread
		);
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_BltBmpOrBaseImageToGraph3_PF(
			SrcRect,
			DestX,
			DestY,
			GrHandle,
			RgbBaseImage,
			AlphaBaseImage,
			RedIsAlphaFlag,
			UseTransColorConvAlpha,
			TargetOrig,
			ASyncThread
		);
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 基本イメージのフォーマットを DX_BASEIMAGE_FORMAT_NORMAL に変換する必要があるかどうかをチェックする
// ( RequiredRgbBaseImageConvFlag と RequiredAlphaBaseImageConvFlag に入る値  TRUE:変換する必要がある  FALSE:変換する必要は無い )
extern	int		Graphics_CheckRequiredNormalImageConv_BaseImageFormat_PF(
	IMAGEDATA_ORIG *Orig,
	int             RgbBaseImageFormat,
	int            *RequiredRgbBaseImageConvFlag,
	int             AlphaBaseImageFormat,
	int            *RequiredAlphaBaseImageConvFlag
)
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_CheckRequiredNormalImageConv_BaseImageFormat_PF(
			Orig,
			RgbBaseImageFormat,
			RequiredRgbBaseImageConvFlag,
			AlphaBaseImageFormat,
			RequiredAlphaBaseImageConvFlag
		) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_CheckRequiredNormalImageConv_BaseImageFormat_PF(
			Orig,
			RgbBaseImageFormat,
			RequiredRgbBaseImageConvFlag,
			AlphaBaseImageFormat,
			RequiredAlphaBaseImageConvFlag
		) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// オリジナル画像情報中のテクスチャを作成する( 0:成功  -1:失敗 )
extern int Graphics_Hardware_CreateOrigTexture_PF( IMAGEDATA_ORIG *Orig, int ASyncThread )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_CreateOrigTexture_PF( Orig, ASyncThread ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_CreateOrigTexture_PF( Orig, ASyncThread ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// オリジナル画像情報中のテクスチャを解放する
extern int Graphics_Hardware_ReleaseOrigTexture_PF( IMAGEDATA_ORIG *Orig )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ReleaseOrigTexture_PF( Orig ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ReleaseOrigTexture_PF( Orig ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 指定のマルチサンプル数で使用できる最大クオリティ値を取得する
extern int Graphics_Hardware_GetMultiSampleQuality_PF( int Samples )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_GetMultiSampleQuality_PF( Samples ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_GetMultiSampleQuality_PF( Samples ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}
























// 環境依存頂点バッファ・インデックスバッファ関係

// 頂点バッファハンドルの頂点バッファを作成する
extern	int		Graphics_Hardware_VertexBuffer_Create_PF( VERTEXBUFFERHANDLEDATA *VertexBuffer )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_VertexBuffer_Create_PF( VertexBuffer ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_VertexBuffer_Create_PF( VertexBuffer ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 頂点バッファハンドルの後始末
extern	int		Graphics_Hardware_VertexBuffer_Terminate_PF( VERTEXBUFFERHANDLEDATA *VertexBuffer )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_VertexBuffer_Terminate_PF( VertexBuffer ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_VertexBuffer_Terminate_PF( VertexBuffer ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 頂点バッファに頂点データを転送する
extern	int		Graphics_Hardware_VertexBuffer_SetData_PF( VERTEXBUFFERHANDLEDATA *VertexBuffer, int SetIndex, const void *VertexData, int VertexNum )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_VertexBuffer_SetData_PF( VertexBuffer, SetIndex, VertexData, VertexNum ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_VertexBuffer_SetData_PF( VertexBuffer, SetIndex, VertexData, VertexNum ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// インデックスバッファハンドルのセットアップを行う
extern	int		Graphics_Hardware_IndexBuffer_Create_PF( INDEXBUFFERHANDLEDATA *IndexBuffer )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_IndexBuffer_Create_PF( IndexBuffer ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_IndexBuffer_Create_PF( IndexBuffer ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// インデックスバッファハンドルの後始末
extern	int		Graphics_Hardware_IndexBuffer_Terminate_PF( INDEXBUFFERHANDLEDATA *IndexBuffer )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_IndexBuffer_Terminate_PF( IndexBuffer ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_IndexBuffer_Terminate_PF( IndexBuffer ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// インデックスバッファにインデックスデータを転送する
extern	int		Graphics_Hardware_IndexBuffer_SetData_PF( INDEXBUFFERHANDLEDATA *IndexBuffer, int SetIndex, const void *IndexData, int IndexNum )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_IndexBuffer_SetData_PF( IndexBuffer,  SetIndex, IndexData,  IndexNum ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_IndexBuffer_SetData_PF( IndexBuffer,  SetIndex, IndexData,  IndexNum ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}





















// 環境依存ライト関係

// ライティングを行うかどうかを設定する
extern	int		Graphics_Hardware_Light_SetUse_PF( int Flag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Light_SetUse_PF( Flag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Light_SetUse_PF( Flag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// グローバルアンビエントライトカラーを設定する
extern	int		Graphics_Hardware_Light_GlobalAmbient_PF( COLOR_F *Color )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Light_GlobalAmbient_PF( Color ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Light_GlobalAmbient_PF( Color ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ライトパラメータをセット
extern	int		Graphics_Hardware_Light_SetState_PF( int LightNumber, LIGHTPARAM *LightParam )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Light_SetState_PF(  LightNumber, LightParam ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Light_SetState_PF(  LightNumber, LightParam ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ライトの有効、無効を変更
extern	int		Graphics_Hardware_Light_SetEnable_PF( int LightNumber, int EnableState )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Light_SetEnable_PF(  LightNumber,  EnableState ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Light_SetEnable_PF(  LightNumber,  EnableState ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

























// 環境依存シャドウマップ関係

// シャドウマップデータに必要なテクスチャを作成する
extern	int		Graphics_Hardware_ShadowMap_CreateTexture_PF( SHADOWMAPDATA *ShadowMap, int ASyncThread )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShadowMap_CreateTexture_PF( ShadowMap, ASyncThread ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShadowMap_CreateTexture_PF( ShadowMap, ASyncThread ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シャドウマップデータに必要なテクスチャを解放する
extern	int		Graphics_Hardware_ShadowMap_ReleaseTexture_PF( SHADOWMAPDATA *ShadowMap )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShadowMap_ReleaseTexture_PF( ShadowMap ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShadowMap_ReleaseTexture_PF( ShadowMap ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 頂点シェーダーに設定するシャドウマップの情報を更新する
extern	void	Graphics_Hardware_ShadowMap_RefreshVSParam_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		Graphics_Hardware_D3D11_ShadowMap_RefreshVSParam_PF(  ) ;
		break ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		Graphics_Hardware_D3D9_ShadowMap_RefreshVSParam_PF(  ) ;
		break ;
#endif // DX_NON_DIRECT3D9

	default :
		break ;
	}
}

// ピクセルシェーダーに設定するシャドウマップの情報を更新する
extern	void	Graphics_Hardware_ShadowMap_RefreshPSParam_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		Graphics_Hardware_D3D11_ShadowMap_RefreshPSParam_PF(  ) ;
		break ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		Graphics_Hardware_D3D9_ShadowMap_RefreshPSParam_PF(  ) ;
		break ;
#endif // DX_NON_DIRECT3D9

	default :
		break ;
	}
}

// シャドウマップへの描画の準備を行う
extern	int		Graphics_Hardware_ShadowMap_DrawSetup_PF( SHADOWMAPDATA *ShadowMap )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShadowMap_DrawSetup_PF( ShadowMap ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShadowMap_DrawSetup_PF( ShadowMap ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シャドウマップへの描画を終了する
extern	int		Graphics_Hardware_ShadowMap_DrawEnd_PF( SHADOWMAPDATA *ShadowMap )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShadowMap_DrawEnd_PF( ShadowMap ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShadowMap_DrawEnd_PF( ShadowMap ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 描画で使用するシャドウマップを指定する( スロットは０か１かを指定可能 )
extern int Graphics_Hardware_ShadowMap_SetUse_PF( int SlotIndex, SHADOWMAPDATA *ShadowMap )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShadowMap_SetUse_PF( SlotIndex, ShadowMap ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShadowMap_SetUse_PF( SlotIndex, ShadowMap ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}
























// 環境依存シェーダー関係

// シェーダーハンドルを初期化する
extern	int		Graphics_Hardware_Shader_Create_PF( int ShaderHandle, int IsVertexShader, void *Image, int ImageSize, int ImageAfterFree, int ASyncThread )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_Create_PF(  ShaderHandle,  IsVertexShader, Image,  ImageSize,  ImageAfterFree,  ASyncThread ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_Create_PF(  ShaderHandle,  IsVertexShader, Image,  ImageSize,  ImageAfterFree,  ASyncThread ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダーハンドルの後始末
extern	int		Graphics_Hardware_Shader_TerminateHandle_PF( SHADERHANDLEDATA *Shader )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_TerminateHandle_PF( Shader ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_TerminateHandle_PF( Shader ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 使用できるシェーダーのバージョンを取得する( 0=使えない  200=シェーダーモデル２．０が使用可能  300=シェーダーモデル３．０が使用可能 )
extern	int		Graphics_Hardware_Shader_GetValidShaderVersion_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_GetValidShaderVersion_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_GetValidShaderVersion_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 指定の名前を持つ定数が使用するシェーダー定数の番号を取得する
extern	int		Graphics_Hardware_Shader_GetConstIndex_PF( const wchar_t *ConstantName, SHADERHANDLEDATA *Shader )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_GetConstIndex_PF( ConstantName, Shader ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_GetConstIndex_PF( ConstantName, Shader ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 指定の名前を持つ定数が使用するシェーダー定数の数を取得する
extern	int		Graphics_Hardware_Shader_GetConstCount_PF( const wchar_t *ConstantName, SHADERHANDLEDATA *Shader )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_GetConstCount_PF( ConstantName, Shader ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_GetConstCount_PF( ConstantName, Shader ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 指定の名前を持つ浮動小数点定数のデフォルトパラメータが格納されているメモリアドレスを取得する
extern	const FLOAT4 *Graphics_Hardware_Shader_GetConstDefaultParamF_PF( const wchar_t *ConstantName, SHADERHANDLEDATA *Shader )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_GetConstDefaultParamF_PF( ConstantName, Shader ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_GetConstDefaultParamF_PF( ConstantName, Shader ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return NULL ;
	}
}

// シェーダー定数情報を設定する
extern	int		Graphics_Hardware_Shader_SetConst_PF( int TypeIndex, int SetIndex, int ConstantIndex, const void *Param, int ParamNum, int UpdateUseArea )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_SetConst_PF( TypeIndex,  SetIndex,  ConstantIndex, Param,  ParamNum,  UpdateUseArea ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_SetConst_PF( TypeIndex,  SetIndex,  ConstantIndex, Param,  ParamNum,  UpdateUseArea ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 指定領域のシェーダー定数情報をリセットする
extern	int		Graphics_Hardware_Shader_ResetConst_PF( int TypeIndex, int SetIndex, int ConstantIndex, int ParamNum )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_ResetConst_PF(  TypeIndex,  SetIndex,  ConstantIndex,  ParamNum ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_ResetConst_PF(  TypeIndex,  SetIndex,  ConstantIndex,  ParamNum ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

#ifndef DX_NON_MODEL
// ３Ｄモデル用のシェーダーコードの初期化を行う
extern	int		Graphics_Hardware_Shader_ModelCode_Init_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Shader_ModelCode_Init_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Shader_ModelCode_Init_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}
#endif // DX_NON_MODEL























// 環境依存シェーダー用定数バッファ関係

// シェーダー用定数バッファハンドルを初期化する
extern int Graphics_Hardware_ShaderConstantBuffer_Create_PF( int ShaderConstantBufferHandle, int BufferSize, int ASyncThread )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShaderConstantBuffer_Create_PF( ShaderConstantBufferHandle, BufferSize, ASyncThread ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShaderConstantBuffer_Create_PF( ShaderConstantBufferHandle, BufferSize, ASyncThread ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダー用定数バッファハンドルの後始末
extern int Graphics_Hardware_ShaderConstantBuffer_TerminateHandle_PF( SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShaderConstantBuffer_TerminateHandle_PF( ShaderConstantBuffer ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShaderConstantBuffer_TerminateHandle_PF( ShaderConstantBuffer ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダー用定数バッファハンドルの定数バッファのアドレスを取得する
extern void *Graphics_Hardware_ShaderConstantBuffer_GetBuffer_PF( SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShaderConstantBuffer_GetBuffer_PF( ShaderConstantBuffer ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShaderConstantBuffer_GetBuffer_PF( ShaderConstantBuffer ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return NULL ;
	}
}

// シェーダー用定数バッファハンドルの定数バッファへの変更を適用する
extern int Graphics_Hardware_ShaderConstantBuffer_Update_PF( SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShaderConstantBuffer_Update_PF( ShaderConstantBuffer ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShaderConstantBuffer_Update_PF( ShaderConstantBuffer ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダー用定数バッファハンドルの定数バッファを指定のシェーダーの指定のスロットにセットする
extern int Graphics_Hardware_ShaderConstantBuffer_Set_PF( SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer, int TargetShader /* DX_SHADERTYPE_VERTEX など */, int Slot )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_ShaderConstantBuffer_Set_PF( ShaderConstantBuffer, TargetShader, Slot ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_ShaderConstantBuffer_Set_PF( ShaderConstantBuffer, TargetShader, Slot ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}


























// 環境依存描画関係

// 頂点バッファに溜まった頂点データをレンダリングする
extern	int		Graphics_Hardware_RenderVertex( int ASyncThread )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_RenderVertex( -1, ASyncThread ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_RenderVertex( ASyncThread ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawBillboard3D
extern	int		Graphics_Hardware_DrawBillboard3D_PF( VECTOR Pos, float cx, float cy, float Size, float Angle, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag, int TurnFlag, int DrawFlag, RECT *DrawArea )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawBillboard3D( Pos,  cx,  cy,  Size,  Angle, Image, BlendImage,  TransFlag,  TurnFlag,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawBillboard3D( Pos,  cx,  cy,  Size,  Angle, Image, BlendImage,  TransFlag,  TurnFlag,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawModiBillboard3D
extern	int		Graphics_Hardware_DrawModiBillboard3D_PF( VECTOR Pos, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag, int DrawFlag, RECT *DrawArea )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawModiBillboard3D(  Pos,  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4, Image, BlendImage,  TransFlag,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawModiBillboard3D(  Pos,  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4, Image, BlendImage,  TransFlag,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawGraph
extern	int		Graphics_Hardware_DrawGraph_PF( int x, int y, float xf, float yf, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag, int IntFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawGraph( x, y, xf, yf, Image, BlendImage, TransFlag, IntFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawGraph( x, y, xf, yf, Image, BlendImage, TransFlag, IntFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawExtendGraph
extern	int		Graphics_Hardware_DrawExtendGraph_PF( int x1, int y1, int x2, int y2, float x1f, float y1f, float x2f, float y2f, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag, int IntFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawExtendGraph(  x1,  y1,  x2,  y2,  x1f,  y1f,  x2f,  y2f, Image, BlendImage,  TransFlag,  IntFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawExtendGraph(  x1,  y1,  x2,  y2,  x1f,  y1f,  x2f,  y2f, Image, BlendImage,  TransFlag,  IntFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawRotaGraph
extern	int		Graphics_Hardware_DrawRotaGraph_PF( int x, int y, float xf, float yf, double ExRate, double Angle, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag, int TurnFlag, int IntFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawRotaGraph(  x,  y,  xf,  yf,  ExRate,  Angle, Image, BlendImage,  TransFlag,  TurnFlag,  IntFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawRotaGraph(  x,  y,  xf,  yf,  ExRate,  Angle, Image, BlendImage,  TransFlag,  TurnFlag,  IntFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawModiGraph
extern	int		Graphics_Hardware_DrawModiGraph_PF( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag, bool SimpleDrawFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawModiGraph(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4, Image, BlendImage,  TransFlag, SimpleDrawFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawModiGraph(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4, Image, BlendImage,  TransFlag, SimpleDrawFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawModiGraphF
extern	int		Graphics_Hardware_DrawModiGraphF_PF( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag, bool SimpleDrawFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawModiGraphF(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4, Image, BlendImage,  TransFlag,  SimpleDrawFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawModiGraphF(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4, Image, BlendImage,  TransFlag,  SimpleDrawFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawSimpleTwoTriangleGraphF
extern	int		Graphics_Hardware_DrawSimpleTwoTriangleGraphF_PF( const GRAPHICS_DRAW_DRAWSIMPLETWOTRIANGLEGRAPHF_PARAM *Param, IMAGEDATA *Image, IMAGEDATA *BlendImage )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawSimpleTwoTriangleGraphF( Param, Image, BlendImage ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawSimpleTwoTriangleGraphF( Param, Image, BlendImage ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawFillBox
extern	int		Graphics_Hardware_DrawFillBox_PF( int x1, int y1, int x2, int y2, unsigned int Color )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawFillBox(  x1,  y1,  x2,  y2,  Color ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawFillBox(  x1,  y1,  x2,  y2,  Color ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawLineBox
extern	int		Graphics_Hardware_DrawLineBox_PF( int x1, int y1, int x2, int y2, unsigned int Color )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawLineBox(  x1,  y1,  x2,  y2,  Color ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawLineBox(  x1,  y1,  x2,  y2,  Color ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}


// ハードウエアアクセラレータ使用版 DrawLine
extern	int		Graphics_Hardware_DrawLine_PF( int x1, int y1, int x2, int y2, unsigned int Color )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawLine(  x1,  y1,  x2,  y2,  Color ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawLine(  x1,  y1,  x2,  y2,  Color ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawLine3D
extern	int		Graphics_Hardware_DrawLine3D_PF( VECTOR Pos1, VECTOR Pos2, unsigned int Color, int DrawFlag, RECT *DrawArea )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawLine3D(  Pos1,  Pos2,  Color,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawLine3D(  Pos1,  Pos2,  Color,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawCircle( 太さ指定あり )
extern	int		Graphics_Hardware_DrawCircle_Thickness_PF( int x, int y, int r, unsigned int Color, int Thickness )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawCircle_Thickness(  x,  y,  r,  Color,  Thickness ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawCircle_Thickness(  x,  y,  r,  Color,  Thickness ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawOval( 太さ指定あり )
extern	int		Graphics_Hardware_DrawOval_Thickness_PF( int x, int y, int rx, int ry, unsigned int Color, int Thickness )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawOval_Thickness(  x,  y,  rx,  ry,  Color,  Thickness ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawOval_Thickness(  x,  y,  rx,  ry,  Color,  Thickness ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawCircle
extern	int		Graphics_Hardware_DrawCircle_PF( int x, int y, int r, unsigned int Color, int FillFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawCircle(  x,  y,  r,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawCircle(  x,  y,  r,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawOval
extern	int		Graphics_Hardware_DrawOval_PF( int x, int y, int rx, int ry, unsigned int Color, int FillFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawOval(  x,  y,  rx,  ry,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawOval(  x,  y,  rx,  ry,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawTriangle
extern	int		Graphics_Hardware_DrawTriangle_PF( int x1, int y1, int x2, int y2, int x3, int y3, unsigned int Color, int FillFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawTriangle(  x1,  y1,  x2,  y2,  x3,  y3,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawTriangle(  x1,  y1,  x2,  y2,  x3,  y3,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawTriangle3D
extern	int		Graphics_Hardware_DrawTriangle3D_PF( VECTOR Pos1, VECTOR Pos2, VECTOR Pos3, unsigned int Color, int FillFlag, int DrawFlag, RECT *DrawArea )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawTriangle3D(  Pos1,  Pos2,  Pos3,  Color,  FillFlag,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawTriangle3D(  Pos1,  Pos2,  Pos3,  Color,  FillFlag,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawQuadrangle
extern	int		Graphics_Hardware_DrawQuadrangle_PF( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, int FillFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawQuadrangle(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawQuadrangle(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4,  Color,  FillFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawPixel
extern	int		Graphics_Hardware_DrawPixel_PF( int x, int y, unsigned int Color )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPixel(  x,  y,  Color ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPixel(  x,  y,  Color ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawPixel3D
extern	int		Graphics_Hardware_DrawPixel3D_PF( VECTOR Pos, unsigned int Color, int DrawFlag, RECT *DrawArea )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPixel3D(  Pos,  Color,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPixel3D(  Pos,  Color,  DrawFlag, DrawArea ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawPixelSet
extern	int		Graphics_Hardware_DrawPixelSet_PF( const POINTDATA *PointData, int Num )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPixelSet( PointData,  Num ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPixelSet( PointData,  Num ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// ハードウエアアクセラレータ使用版 DrawLineSet
extern	int		Graphics_Hardware_DrawLineSet_PF( const LINEDATA *LineData, int Num )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawLineSet( LineData,  Num ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawLineSet( LineData,  Num ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}


extern	int		Graphics_Hardware_DrawPrimitive_PF(                             const VERTEX_3D *Vertex, int VertexNum,                                    int PrimitiveType, IMAGEDATA *Image, int TransFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitive( Vertex, VertexNum, PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitive( Vertex, VertexNum, PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawIndexedPrimitive_PF(                      const VERTEX_3D *Vertex, int VertexNum, const WORD *Indices, int IndexNum, int PrimitiveType, IMAGEDATA *Image, int TransFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawIndexedPrimitive( Vertex,  VertexNum, Indices, IndexNum,  PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawIndexedPrimitive( Vertex,  VertexNum, Indices, IndexNum,  PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawPrimitiveLight_PF(                        const VERTEX3D  *Vertex, int VertexNum,                                    int PrimitiveType, IMAGEDATA *Image, int TransFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitiveLight(                        Vertex,  VertexNum,                                     PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitiveLight(                        Vertex,  VertexNum,                                     PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawIndexedPrimitiveLight_PF(                 const VERTEX3D  *Vertex, int VertexNum, const WORD *Indices, int IndexNum, int PrimitiveType, IMAGEDATA *Image, int TransFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawIndexedPrimitiveLight(                 Vertex, VertexNum, Indices,  IndexNum,  PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawIndexedPrimitiveLight(                 Vertex, VertexNum, Indices,  IndexNum,  PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawPrimitiveLight_UseVertexBuffer_PF(        VERTEXBUFFERHANDLEDATA *VertexBuffer,                                      int PrimitiveType,                 int StartVertex, int UseVertexNum, IMAGEDATA *Image, int TransFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitiveLight_UseVertexBuffer(        VertexBuffer,                                       PrimitiveType,                  StartVertex,  UseVertexNum, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitiveLight_UseVertexBuffer(        VertexBuffer,                                       PrimitiveType,                  StartVertex,  UseVertexNum, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawIndexedPrimitiveLight_UseVertexBuffer_PF( VERTEXBUFFERHANDLEDATA *VertexBuffer, INDEXBUFFERHANDLEDATA *IndexBuffer,  int PrimitiveType, int BaseVertex, int StartVertex, int UseVertexNum, int StartIndex, int UseIndexNum, IMAGEDATA *Image, int TransFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawIndexedPrimitiveLight_UseVertexBuffer( VertexBuffer, IndexBuffer,   PrimitiveType,  BaseVertex,  StartVertex,  UseVertexNum,  StartIndex,  UseIndexNum, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawIndexedPrimitiveLight_UseVertexBuffer( VertexBuffer, IndexBuffer,   PrimitiveType,  BaseVertex,  StartVertex,  UseVertexNum,  StartIndex,  UseIndexNum, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawPrimitive2D_PF(                                 VERTEX_2D *Vertex, int VertexNum,                                    int PrimitiveType, IMAGEDATA *Image, int TransFlag, int BillboardFlag, int Is3D, int TurnFlag, int TextureNo )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitive2D(                                 Vertex,  VertexNum,                                     PrimitiveType, Image,  TransFlag,  BillboardFlag,  Is3D,  TurnFlag,  TextureNo ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitive2D(                                 Vertex,  VertexNum,                                     PrimitiveType, Image,  TransFlag,  BillboardFlag,  Is3D,  TurnFlag,  TextureNo ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawPrimitive2DUser_PF(                       const VERTEX2D  *Vertex, int VertexNum,                                    int PrimitiveType, IMAGEDATA *Image, int TransFlag, int Is3D, int TurnFlag, int TextureNo )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitive2DUser(                       Vertex,  VertexNum,                                     PrimitiveType, Image,  TransFlag,  Is3D,  TurnFlag,  TextureNo ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitive2DUser(                       Vertex,  VertexNum,                                     PrimitiveType, Image,  TransFlag,  Is3D,  TurnFlag,  TextureNo ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern	int		Graphics_Hardware_DrawIndexedPrimitive2DUser_PF(                const VERTEX2D  *Vertex, int VertexNum, const WORD *Indices, int IndexNum, int PrimitiveType, IMAGEDATA *Image, int TransFlag )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawIndexedPrimitive2DUser(                Vertex,  VertexNum, Indices,  IndexNum,  PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawIndexedPrimitive2DUser(                Vertex,  VertexNum, Indices,  IndexNum,  PrimitiveType, Image,  TransFlag ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダーを使って２Ｄプリミティブを描画する
extern	int		Graphics_Hardware_DrawPrimitive2DToShader_PF(        const VERTEX2DSHADER *Vertex, int VertexNum,                                              int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitive2DToShader(        Vertex,  VertexNum,                                               PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitive2DToShader(        Vertex,  VertexNum,                                               PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダーを使って３Ｄプリミティブを描画する
extern	int		Graphics_Hardware_DrawPrimitive3DToShader_PF(        const VERTEX3DSHADER *Vertex, int VertexNum,                                              int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitive3DToShader(        Vertex,  VertexNum,                                               PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitive3DToShader(        Vertex,  VertexNum,                                               PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダーを使って２Ｄプリミティブを描画する( 頂点インデックスを使用する )
extern	int		Graphics_Hardware_DrawPrimitiveIndexed2DToShader_PF( const VERTEX2DSHADER *Vertex, int VertexNum, const unsigned short *Indices, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitiveIndexed2DToShader( Vertex,  VertexNum, Indices,  IndexNum,  PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitiveIndexed2DToShader( Vertex,  VertexNum, Indices,  IndexNum,  PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダーを使って３Ｄプリミティブを描画する( 頂点インデックスを使用する )
extern	int		Graphics_Hardware_DrawPrimitiveIndexed3DToShader_PF( const VERTEX3DSHADER *Vertex, int VertexNum, const unsigned short *Indices, int IndexNum, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitiveIndexed3DToShader( Vertex,  VertexNum, Indices,  IndexNum,  PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitiveIndexed3DToShader( Vertex,  VertexNum, Indices,  IndexNum,  PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダーを使って３Ｄプリミティブを描画する( 頂点バッファ使用版 )
extern	int		Graphics_Hardware_DrawPrimitive3DToShader_UseVertexBuffer2_PF(        int VertexBufHandle,                     int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int StartVertex, int UseVertexNum )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitive3DToShader_UseVertexBuffer2(         VertexBufHandle,                      PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */,  StartVertex,  UseVertexNum ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitive3DToShader_UseVertexBuffer2(         VertexBufHandle,                      PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */,  StartVertex,  UseVertexNum ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// シェーダーを使って３Ｄプリミティブを描画する( 頂点バッファとインデックスバッファ使用版 )
extern	int		Graphics_Hardware_DrawPrimitiveIndexed3DToShader_UseVertexBuffer2_PF( int VertexBufHandle, int IndexBufHandle, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int BaseVertex, int StartVertex, int UseVertexNum, int StartIndex, int UseIndexNum )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_D3D11_DrawPrimitiveIndexed3DToShader_UseVertexBuffer2(  VertexBufHandle,  IndexBufHandle,  PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */,  BaseVertex,  StartVertex,  UseVertexNum,  StartIndex,  UseIndexNum ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_D3D9_DrawPrimitiveIndexed3DToShader_UseVertexBuffer2(  VertexBufHandle,  IndexBufHandle,  PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */,  BaseVertex,  StartVertex,  UseVertexNum,  StartIndex,  UseIndexNum ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

// 指定点から境界色があるところまで塗りつぶす
extern	int		Graphics_Hardware_Paint_PF( int x, int y, unsigned int FillColor, ULONGLONG BoundaryColor )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return Graphics_Hardware_D3D11_Paint_PF(  x,  y,  FillColor,  BoundaryColor ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return Graphics_Hardware_D3D9_Paint_PF(  x,  y,  FillColor,  BoundaryColor ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}






























#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

