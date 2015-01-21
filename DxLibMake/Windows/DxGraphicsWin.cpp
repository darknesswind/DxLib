//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		WindowsOS用描画処理プログラム
// 
//  	Ver 3.11f
// 
//-----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ---------------------------------------------------------------
#include "DxGraphicsWin.h"
#include "DxWindow.h"
#include "DxWinAPI.h"
#include "../DxSystem.h"
#include "../DxLog.h"
#include "../DxModel.h"
#include "../DxBaseFunc.h"
#include "../DxGraphics.h"

namespace DxLib
{

// 宏定义 -----------------------------------------------------------------

// 结构体声明 -----------------------------------------------------------------

// 数据定义 -----------------------------------------------------------------

static int UseGDIFlag ;							// GDI を使用するかどうか

// 函数声明 -------------------------------------------------------------------

static int EnumDirect3DAdapter() ;												// Direct3D のアダプタ情報を列挙する
static int UseChangeDisplaySettings( void ) ;									// ChangeDisplaySettings を使用して画面モードを変更する

// Program -----------------------------------------------------------------

// 現在の画面のリフレッシュレートを取得する
extern int NS_GetRefreshRate( void )
{
	int RefreshRate = -1 ;

	// リフレッシュレートの取得
	if( WinData.WindowsVersion >= DX_WINDOWSVERSION_NT31 )
	{
		HDC hdc ;
		
		hdc = GetDC( NS_GetMainWindowHandle() ) ;
		if( hdc )
		{
			RefreshRate = GetDeviceCaps( hdc, VREFRESH ) ;
			ReleaseDC( NS_GetMainWindowHandle(), hdc ) ;

			if( RefreshRate < 50 ) RefreshRate = -1 ;
		}
	}

	return RefreshRate ;
}

// ChangeDisplaySettings を使用して画面モードを変更する
static int UseChangeDisplaySettings( void )
{
//	HRESULT hr ;
	int DesktopWidth, DesktopHeight, DesktopColorBitDepth ;
	DEVMODE DevMode;

	// 元の画面の大きさを得る
	NS_GetDefaultState( &DesktopWidth, &DesktopHeight, &DesktopColorBitDepth ) ;

	DXST_ERRORLOGFMT_ADD(( _T( "画面モードの変更処理を開始します %u x %u  %u bit " ), GRA2.MainScreenSizeX, GRA2.MainScreenSizeY, GRA2.MainScreenColorBitDepth )) ;
	DXST_ERRORLOG_TABADD ;

	// 対応しているカラービット深度は 16 と 32 のみ
	if( GRA2.MainScreenColorBitDepth != 16 &&
		GRA2.MainScreenColorBitDepth != 32 )
		return -1 ;

	// ディスプレイモードの変更
	DXST_ERRORLOG_ADD( _T( "画面解像度を変更します... " ) ) ;

	// Win32 API を使って画面モードを変更する
	{
		// パラメータセット
		_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
		DevMode.dmSize             = sizeof( DevMode ) ;
		DevMode.dmPelsWidth        = GRA2.MainScreenSizeX ;
		DevMode.dmPelsHeight       = GRA2.MainScreenSizeY ;
		DevMode.dmBitsPerPel       = GRA2.MainScreenColorBitDepth ;
		DevMode.dmFields           = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT ;
		if( GRA2.MainScreenRefreshRate != 0 )
		{
			DevMode.dmDisplayFrequency  = GRA2.MainScreenRefreshRate ;
			DevMode.dmFields           |= DM_DISPLAYFREQUENCY ;
		}

		// 画面モードチェンジ
		if( ChangeDisplaySettings( &DevMode, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
		{
			// 失敗したらリフレッシュレート指定無しでもう一度挑戦
			GRA2.MainScreenRefreshRate = 0 ;
			DevMode.dmDisplayFrequency = 0 ;
			DevMode.dmFields           &= ~DM_DISPLAYFREQUENCY ;
			if( ChangeDisplaySettings( &DevMode, 0/*CDS_FULLSCREEN*/ ) != DISP_CHANGE_SUCCESSFUL )
			{
				// それでも駄目で、且つ画面モードが 320x240 だった場合は
				// 擬似 320x240 モードで試す
				if( GRA2.MainScreenSizeX == 320 && GRA2.MainScreenSizeY == 240 )
				{
					SetMainScreenSize( 640, 480 ) ;
					NS_SetEmulation320x240( TRUE ) ;
					DevMode.dmPelsWidth        = GRA2.MainScreenSizeX ;
					DevMode.dmPelsHeight       = GRA2.MainScreenSizeY ;
					if( ChangeDisplaySettings( &DevMode, 0/*CDS_FULLSCREEN*/ ) != DISP_CHANGE_SUCCESSFUL )
					{
						goto ERRORLABEL ;
					}
					SetWindowStyle() ;
				}
				else
				{
ERRORLABEL:
					DXST_ERRORLOG_ADD( _T( "画面モードの変更に失敗しました\n" ) ) ;
					DXST_ERRORLOG_TABSUB ;
					return -1 ;
				}
			}
		}
	}

	GRA2.UseChangeDisplaySettings = TRUE ;
	DXST_ERRORLOG_ADD( _T( "成功しました\n" ) ) ;

	return 0 ;
}

// DirectX 関連の初期化( 0:成功  -1:失敗 )
extern int InitializeDirectX()
{
	int result ;

	// Direct3D9.DLL の読み込み
	if( Graphics_LoadDLL() < 0 )
		return -1 ;

	// DirectDraw7 の取得を試みる
	if( OldGraphicsInterface_Create() < 0 )
	{
		Graphics_FreeDLL() ;
		return -1 ;
	}

	// ハードウエアを使用しない、フラグが立っていない場合は Direct3D9 の取得を試みる
	GRA2.ValidHardWare = FALSE ;
	if( GRA2.NotUseHardWare == FALSE )
	{
		// フルスクリーンモードで且つ Vista 以降の場合で表画面と裏画面の大きさが違う場合はここで画面モードを変更する
		result = 0 ;
		if( NS_GetWindowModeFlag() == FALSE && WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA &&
			( GRA2.ValidGraphDisplayArea || GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) )
		{
			result = UseChangeDisplaySettings() ;
		}

		// 画面モードを変更していないか、画面モードの変更が成功した場合のみ Direct3DDevice9 を作成する
		if( result == 0 )
		{
CREATEDIRECT3D9OBJECT :
			if( Graphics_Create() == 0 )
			{
				int Ret ;

				// Direct3DDevice9 の取得を試みる
				Ret = CreateDirect3D9DeviceObject() ;
				if( Ret == -2 )
				{
					goto CREATEDIRECT3D9OBJECT ;
				}
				else
				if( Ret == 0 )
				{
					// Zバッファ の作成を試みる
					if( CreateDirect3D9ZBufferObject() == 0 )
					{
						GRA2.ValidHardWare = TRUE ;

						// シェーダーの作成
						SetupDirect3D9Shader() ;

						// シェーダーが使用できる場合とできない場合で処理を分岐
						if( GRH.UseShader )
						{
							// 頂点宣言の作成
							CreateDirect3D9VertexDeclaration() ;

#ifndef DX_NON_MODEL
							// ボリュームテクスチャの作成
							CreateRGBtoVMaxRGBVolumeTexture() ;
#endif // DX_NON_MODEL
						}
					}
				}
			}
		}
		if( GRA2.ValidHardWare == FALSE )
		{
			// どれか一つでも失敗した場合は Direct3D 周りのオブジェクトを解放する

			// Zバッファ の解放
			if( GRH.ZBufferSurface )
			{
				Graphics_ObjectRelease( GRH.ZBufferSurface ) ;
				GRH.ZBufferSurface = NULL ;
			}

			// Direct3DDevice9 の解放
			GraphicsDevice_Release() ;
			GRH.BeginSceneFlag = FALSE ;

			// Direct3D9 の解放
			Graphics_Release() ;
		}

#if 0
		// ディスプレイモードの列挙を行う
		{
			int i, Num16, Num32 ;
			D_D3DDISPLAYMODE Mode ;
			DISPLAYMODEDATA *Disp ;

			Num32 = Graphics_GetAdapterModeCount( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_X8R8G8B8 ) ;
			Num16 = Graphics_GetAdapterModeCount( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_R5G6B5 ) ;

			if( GRH.DisplayMode )
			{
				DXFREE( GRH.DisplayMode ) ;
				GRH.DisplayMode = NULL ;
			}
			GRH.DisplayModeNum = Num32 + Num16 ;
			GRH.DisplayMode = ( DISPLAYMODEDATA * )DXALLOC( sizeof( DISPLAYMODEDATA ) * ( Num32 + Num16 ) ) ;
			if( GRH.DisplayMode == NULL )
			{
				return DXST_ERRORLOG_ADD( _T( "ディスプレイモードを保存するメモリ領域の確保に失敗しました\n" ) ) ;
			}

			Disp = GRH.DisplayMode ;
			for( i = 0 ; i < Num32 ; i ++, Disp ++ )
			{
				Graphics_EnumAdapterModes( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_X8R8G8B8, i, &Mode ) ;
				Disp->Width = Mode.Width ;
				Disp->Height = Mode.Height ;
				Disp->ColorBitDepth = 32 ;
				Disp->RefreshRate = Mode.RefreshRate ;
			}
			for( i = 0 ; i < Num16 ; i ++, Disp ++ )
			{
				Graphics_EnumAdapterModes( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_R5G6B5, i, &Mode ) ;
				Disp->Width = Mode.Width ;
				Disp->Height = Mode.Height ;
				Disp->ColorBitDepth = 16 ;
				Disp->RefreshRate = Mode.RefreshRate ;
			}
		}
#endif
	}

	// ハードウエアアクセラレーションを使用しない場合はここで画面モードを変更する
	if( GRA2.ValidHardWare == FALSE )
	{
		// フルスクリーンの場合のみ処理
		if( NS_GetWindowModeFlag() == FALSE )
		{
			if( UseChangeDisplaySettings() < 0 )
				return -1 ;
		}

		// ＶＳＹＮＣ待ちの時間を取得する
		GraphicsDevice_WaitVSyncInitialize() ;

		// 終了
		return 0 ;
	}

	// 終了
	return 0 ;
}

// DirectX 関連の後始末
extern int TerminateDirectX()
{
#ifndef DX_NON_MODEL
	// モデルの頂点バッファを解放
	MV1TerminateVertexBufferAll() ;
#endif

	// 頂点宣言の解放
	TerminateDirect3D9VertexDeclaration() ;

	// シェーダーの解放
	TerminateDirect3D9Shader() ;

//	// ハードウエアアクセラレータを使用しない場合はここで画面モードを元に戻す
//	if( /*NS_GetWindowModeFlag() == FALSE &&*/
//		( GRA2.ValidHardWare == FALSE ||
//		 ( WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA && ( GRA2.ValidGraphDisplayArea || GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) ) ) )
	// ChangeDisplaySettings を使用して画面モードを変更した場合はここで画面モードを元に戻す
	if( GRA2.UseChangeDisplaySettings )
	{
		GRA2.UseChangeDisplaySettings = FALSE ;
		ChangeDisplaySettings( NULL, 0 ) ;
	}

	// Zバッファ の解放
	if( GRH.ZBufferSurface )
	{
		Graphics_ObjectRelease( GRH.ZBufferSurface ) ;
		GRH.ZBufferSurface = NULL ;
	}

	// サブバックバッファの解放
	if( GRH.SubBackBufferSurface )
	{
		Graphics_ObjectRelease( GRH.SubBackBufferSurface ) ;
		GRH.SubBackBufferSurface = NULL ;
	}

	// バックバッファの解放
	if( GRH.BackBufferSurface )
	{
		Graphics_ObjectRelease( GRH.BackBufferSurface ) ;
		GRH.BackBufferSurface = NULL ;
	}

#ifndef DX_NON_MODEL
	// ボリュームテクスチャの解放
	if( GRH.RGBtoVMaxRGBVolumeTexture )
	{
		Graphics_ObjectRelease( GRH.RGBtoVMaxRGBVolumeTexture ) ;
		GRH.RGBtoVMaxRGBVolumeTexture = NULL ;
	}
#endif // DX_NON_MODEL

	// Direct3DDevice9 の解放
	GraphicsDevice_Release() ;
	GRH.BeginSceneFlag = FALSE ;

	// Direct3D9 の解放
	Graphics_Release() ;

	// DirectDraw の解放
	OldGraphicsInterface_Release() ;

	// d3d9.dll の解放
	Graphics_FreeDLL() ;

	// ディスプレイモード用に確保したメモリの解放
	if( GRH.DisplayMode )
	{
		HeapFree( GetProcessHeap(), 0, GRH.DisplayMode ) ;
		GRH.DisplayMode = NULL ;
	}

	// 終了
	return 0 ;
}


















































// 機種依存関数のコード

// Direct3D のアダプタ情報を列挙する
static int EnumDirect3DAdapter()
{
	int i ;

	if( GRH.ValidAdapterInfo ) return 0 ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE || DxLib_GetEndRequest() )
	{
		// Direct3D9.DLL の読み込み
		if( Graphics_LoadDLL() < 0 )
			return -1 ;

		// Direct3D9 の作成
		if( Graphics_Create() == 0 )
		{
			GRH.AdapterInfoNum = Graphics_GetAdapterCount() ;
			for( i = 0 ; i < GRH.AdapterInfoNum ; i ++ )
				Graphics_GetAdapterIdentifier( i, 0, &GRH.AdapterInfo[ i ] ) ;

			// Direct3D9 の解放
			Graphics_Release() ;
		}

		// d3d9.dll の解放
		Graphics_FreeDLL() ;
	}
	else
	{
		GRH.AdapterInfoNum = Graphics_GetAdapterCount() ;
		for( i = 0 ; i < GRH.AdapterInfoNum ; i ++ )
			Graphics_GetAdapterIdentifier( i, 0, &GRH.AdapterInfo[ i ] ) ;
	}

	GRH.ValidAdapterInfo = TRUE ;

	// 正常終了
	return 0 ;
}

// ＤｉｒｅｃｔＤｒａｗが使用するＧＵＤＩを設定する
extern	int		NS_SetDDrawUseGuid( const GUID FAR *Guid )
{
	int i ;

	if( EnumDirect3DAdapter() != 0 ) return -1 ;

	// 指定のＧＵＩＤがあるかどうか調べる
	for( i = 0 ; i < GRH.AdapterInfoNum && _MEMCMP( Guid, &GRH.AdapterInfo[ i ].DeviceIdentifier, sizeof( GUID ) ) != 0 ; i ++ ){}
	if( i == GRH.AdapterInfoNum ) return -1 ;

	// 値を保存する
	GRH.ValidAdapterNumber = TRUE ;
	GRH.UseAdapterNumber = i ;

	// 終了
	return 0 ;
}

/*戻り値を IDirectDraw7 * にキャストして下さい*/			// 現在使用しているＤｉｒｅｃｔＤｒａｗオブジェクトのアドレスを取得する
extern	const void					*NS_GetUseDDrawObj( void )
{
	// 終了
	return OldGraphicsInterface_GetObject() ;
}

// 有効な DirectDraw デバイスの GUID を取得する
extern	const GUID *				NS_GetDirectDrawDeviceGUID( int Number )
{
	if( EnumDirect3DAdapter() != 0 || Number >= GRH.AdapterInfoNum ) return NULL ;

	return &GRH.AdapterInfo[ Number ].DeviceIdentifier ;
}

// 有効な DirectDraw デバイスの名前を得る
extern	int						NS_GetDirectDrawDeviceDescription( int Number, char *StringBuffer )
{
	if( EnumDirect3DAdapter() != 0 || Number >= GRH.AdapterInfoNum ) return -1 ;

	_STRCPY( StringBuffer, GRH.AdapterInfo[ Number ].Description ) ;

	// 終了
	return 0 ;
}

// 有効な DirectDraw デバイスの数を取得する
extern	int						NS_GetDirectDrawDeviceNum( void )
{
	if( EnumDirect3DAdapter() != 0 ) return -1 ;

	return GRH.AdapterInfoNum ;
}


// ＤｉｒｅｃｔＤｒａｗを使用するかどうかをセットする
extern	int		NS_SetUseDirectDrawFlag( int Flag )
{
	// 初期化前のみ使用可能
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	GRA2.NotUseHardWare = !Flag ;

	// 終了
	return 0 ;
}

// ＧＤＩ描画を必要とするか、を変更する
extern	int		NS_SetUseGDIFlag( int Flag )
{
	// フラグを保存
	UseGDIFlag = Flag ;

	// ハードウエアを使用しているかどうかで処理を分岐
	if( GRA2.ValidHardWare )
	{
		// ハードウエアレンダリングモードの場合
		if( GraphicsDevice_IsValid() == 0 ) return 0 ;

		// デバイスの設定
		if( Flag == FALSE )
		{
			GraphicsDevice_SetDialogBoxMode( FALSE ) ;
		}
		else
		{
			GraphicsDevice_SetDialogBoxMode( FALSE ) ;
			GraphicsDevice_SetDialogBoxMode( TRUE ) ;
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

// ディスプレイモードデータの情報をセットアップする
extern void SetupDisplayModeData( int *Num, DISPLAYMODEDATA **Buffer )
{
	int i, ModeNum ;
	DEVMODE DevMode ;
	DISPLAYMODEDATA *Disp ;

	// 既にメモリが確保されていたら解放
	if( *Buffer )
	{
		HeapFree( GetProcessHeap(), 0, *Buffer ) ;
		*Buffer = NULL ;
	}

	// 数を数える
	*Num = 0 ;
	ModeNum = 0 ;
	for( ModeNum ;  ; ModeNum ++ )
	{
		_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
		if( EnumDisplaySettings( NULL, ModeNum, &DevMode ) == 0 ) break ;

		// 16bit 以下のモードは除外する
		if( DevMode.dmBitsPerPel >= 16 )
		{
			( *Num ) ++ ;
		}
	}

	// 情報を格納するメモリ領域の確保
	*Buffer = ( DISPLAYMODEDATA * )HeapAlloc( GetProcessHeap(), 0, sizeof( DISPLAYMODEDATA ) * *Num ) ;
	if( *Buffer == NULL )
	{
		*Num = 0 ;
		return ;
	}

	// 情報を取得する
	Disp = *Buffer ;
	for( i = 0 ; i < ModeNum ; i ++ )
	{
		_MEMSET( &DevMode, 0, sizeof( DevMode ) ) ;
		EnumDisplaySettings( NULL, i, &DevMode ) ;
		if( DevMode.dmBitsPerPel >= 16 )
		{
			Disp->Width         = DevMode.dmPelsWidth ;
			Disp->Height        = DevMode.dmPelsHeight ;
			Disp->ColorBitDepth = DevMode.dmBitsPerPel ;
			Disp->RefreshRate   = DevMode.dmDisplayFrequency ;
			Disp ++ ;
		}
	}
}

}

