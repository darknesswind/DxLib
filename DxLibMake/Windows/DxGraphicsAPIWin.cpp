// ----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		描画ＡＰＩプログラム
// 
// 				Ver 3.11f
// 
// ----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// インクルード----------------------------------------------------------------
#include "DxGraphicsAPIWin.h"
#include "../DxGraphics.h"
#include "DxWinAPI.h"
#include "DxWindow.h"
#include "DxGuid.h"
#include "../DxLog.h"
#include "../DxBaseFunc.h"

namespace DxLib
{

// 宏定义 -----------------------------------------------------------------

// タイマーＩＤ
#define SCREENFLIPTIMER_ID		(32767)

// 结构体声明 -----------------------------------------------------------------

// 数据定义 -----------------------------------------------------------------

GRAPHICSAPIINFO_WIN GAPIWin ;

// 函数声明 -------------------------------------------------------------------

#if _MSC_VER > 1200
static  VOID CALLBACK ScreenFlipTimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime );							// テンポラリプライマリバッファの内容をプライマリバッファに転送するコールバック関数
#else
static  VOID CALLBACK ScreenFlipTimerProc( HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime );								// テンポラリプライマリバッファの内容をプライマリバッファに転送するコールバック関数
#endif

// Program -----------------------------------------------------------------

// グラフィックスデバイスの初期化
extern int InitializeGraphicsDevice( void )
{
	// 終了
	return 0 ;
}

// グラフィックスデバイスの後始末
extern int TerminateGraphicsDevice( void )
{
	// 終了
	return 0 ;
}



extern int OldGraphicsInterface_Create( void )
{
	HRESULT hr ;

	// 既に作成済みの場合は一度削除する
	if( GAPIWin.DirectDraw7Object )
	{
		GAPIWin.DirectDraw7Object->Release() ;
		GAPIWin.DirectDraw7Object = NULL ;
	}

	// ＤｉｒｅｃｔＤｒａｗオブジェクトの作成
	DXST_ERRORLOG_ADD( _T( "DirectDraw オブジェクトの取得を行います....  " ) ) ;
	hr = WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTDRAW7, NULL, CLSCTX_ALL, IID_IDIRECTDRAW7, ( LPVOID *)&GAPIWin.DirectDraw7Object );
	if( !FAILED( hr ) )
	{
		DXST_ERRORLOG_ADD( _T( "成功\n" )) ;
		DXST_ERRORLOG_ADD( _T( "引き続き初期化処理... " ) ) ; 
		hr = GAPIWin.DirectDraw7Object->Initialize( NULL ) ;
		if( FAILED( hr ) ) 
		{
			DXST_ERRORLOG_ADD( _T( "DirectDrawオブジェクトの初期化に失敗しました\n" ) );
			GAPIWin.DirectDraw7Object->Release() ;
			return -1 ;
		}
		DXST_ERRORLOG_ADD( _T( "初期化に成功しました\n" ) ) ;
	}
	else
	{
		DXST_ERRORLOG_ADD( _T( "オブジェクトの取得に失敗しました\n" ) );
		return -1 ;
	}

	// 協調レベルを適当にセット
	GAPIWin.DirectDraw7Object->SetCooperativeLevel( NS_GetMainWindowHandle(), D_DDSCL_NORMAL | ( WinData.UseFPUPreserve ? D_DDSCL_FPUPRESERVE : 0 ) ) ;

	// Aero无效化处理
	if( GRH.DisableAeroFlag == 2 )
	{
		// 关闭DWM
		SetEnableAero( FALSE ) ;
#if 0
		D_DDSURFACEDESC2 ddsd ;
		DX_DIRECTDRAWSURFACE7 *prmsuf ;
		HDC prmdc ;
		HRESULT hr ;

		//  作成パラメータのセット	
		_MEMSET( &ddsd, 0, sizeof( ddsd ) ) ; 
		ddsd.dwSize				= sizeof( ddsd ) ;
		ddsd.dwFlags			= D_DDSD_CAPS ;	
		ddsd.ddsCaps.dwCaps		= D_DDSCAPS_PRIMARYSURFACE | D_DDSCAPS_3DDEVICE ;

		// プライマリサーフェスの作成
		hr = GAPIWin.DirectDraw7Object->CreateSurface( &ddsd, &prmsuf, NULL ) ;
		if( hr == D_DD_OK )
		{
			// GetDC をして Aero 無効
			hr = prmsuf->GetDC( &prmdc ) ;
			if( hr == D_DD_OK )
			{
				prmsuf->ReleaseDC( prmdc ) ;
				prmdc = NULL ;
			}
			prmsuf->Release() ;
			prmsuf = NULL ;
		}
#endif
	}

	// 終了
	return 0 ;
}

extern int OldGraphicsInterface_Release( void )
{
	if( GAPIWin.DirectDraw7Object )
	{
		GAPIWin.DirectDraw7Object->Release() ;
		GAPIWin.DirectDraw7Object = NULL ;
	}

	// 終了
	return 0 ;
}

extern void *OldGraphicsInterface_GetObject( void )
{
	return GAPIWin.DirectDraw7Object ;
}

extern int OldGraphicsInterface_GetVideoMemorySize( int *TotalSize, int *FreeSize )
{
	D_DDSCAPS2 caps ;
	DWORD TotalMem, FreeMem ;

	if( GAPIWin.DirectDraw7Object == NULL ) return -1 ;

	_MEMSET( &caps, 0, sizeof( caps ) );
	caps.dwCaps = D_DDSCAPS_VIDEOMEMORY | D_DDSCAPS_LOCALVIDMEM ;
	GAPIWin.DirectDraw7Object->GetAvailableVidMem( &caps, &TotalMem, &FreeMem ) ;

	if( TotalSize ) *TotalSize = TotalMem ;
	if( FreeSize ) *FreeSize = FreeMem ;

	// 終了
	return 0 ;
}








extern ULONG Graphics_ObjectRelease( void *pObject )
{
	return ( ( IUnknown * )pObject )->Release() ;
}






extern int Graphics_Create( void )
{
	D_IDirect3D9 * ( WINAPI *DF_Direct3DCreate9 )( UINT SDKVersion ) ;
	HRESULT ( WINAPI * DF_Direct3DCreate9Ex )(  UINT SDKVersion, D_IDirect3D9Ex **ppD3D ) ;

	// 既に作成済みの場合は一度削除する
	if( GAPIWin.Direct3D9Object )
	{
		Graphics_Release() ;
	}
	GAPIWin.Direct3D9ExObject = NULL ;

	// 先に Direct3DCreate9Ex のアドレス取得を試みる
	if( GRH.NotUseDirect3D9Ex == FALSE )
	{
		DXST_ERRORLOG_ADD( _T( "IDirect3D9Ex オブジェクトを取得します.... " ) ) ;

		DF_Direct3DCreate9Ex = ( HRESULT ( WINAPI * )( UINT SDKVersion, D_IDirect3D9Ex ** ) )GetProcAddress( GAPIWin.Direct3D9DLL, "Direct3DCreate9Ex" ) ;
		if( DF_Direct3DCreate9Ex != NULL )
		{
			// IDirect3D9Ex オブジェクトの作成
			if( DF_Direct3DCreate9Ex( 32, &GAPIWin.Direct3D9ExObject ) == S_OK )
			{
				GAPIWin.Direct3D9Object = GAPIWin.Direct3D9ExObject ;
			}
		}
	}

	// IDirect3D9Ex が作成できなかったら IDirect3D9 を作成する
	if( GAPIWin.Direct3D9Object == NULL )
	{
		DXST_ERRORLOG_ADD( _T( "IDirect3D9 オブジェクトを取得します.... " ) ) ;

		// Direct3DCreate9 API のアドレス取得
		DF_Direct3DCreate9 = ( D_IDirect3D9 * ( WINAPI * )( UINT SDKVersion ) )GetProcAddress( GAPIWin.Direct3D9DLL, "Direct3DCreate9" ) ;
		if( DF_Direct3DCreate9 == NULL )
		{
			return DXST_ERRORLOG_ADD( _T( "Direct3DCreate9 APIのアドレス取得に失敗しました\n" ) ) ;
		}

		// IDirect3D9 オブジェクトの作成
		GAPIWin.Direct3D9Object = DF_Direct3DCreate9( 32 ) ;
		if( GAPIWin.Direct3D9Object == NULL )
		{
			return DXST_ERRORLOG_ADD( _T( "IDirect3D9 オブジェクトの取得に失敗しました\n" ) ) ;
		}
	}

	DXST_ERRORLOG_ADD( _T( "成功\n" )) ;

	// 終了
	return 0 ;
}

extern int Graphics_LoadDLL( void )
{
	// Direct3D9.DLL の読み込み
	GAPIWin.Direct3D9DLL = LoadLibrary( _T( "d3d9.dll" ) ) ;
	if( GAPIWin.Direct3D9DLL == NULL )
	{
		return DXST_ERRORLOG_ADD( _T( "d3d9.dll の読み込みに失敗しました\n" ) ) ;
	}

	// 正常終了
	return 0 ;
}

extern int Graphics_FreeDLL( void )
{
	// d3d9.dll の解放
	if( GAPIWin.Direct3D9DLL )
	{
		DXST_ERRORLOG_ADD( _T( "Direct3D9 DLL の解放 1\n" ) ) ;
		FreeLibrary( GAPIWin.Direct3D9DLL ) ;
		GAPIWin.Direct3D9DLL = NULL ;
	}

	// 正常終了
	return 0 ;
}

extern ULONG Graphics_Release( void )
{
	ULONG Result ;

	if( GAPIWin.Direct3D9Object == NULL )
	{
		return 0 ;
	}
	Result = GAPIWin.Direct3D9Object->Release() ;
	GAPIWin.Direct3D9Object = NULL ;

	return Result ;
}

extern UINT Graphics_GetAdapterCount( void )
{
	return GAPIWin.Direct3D9Object->GetAdapterCount() ;
}

extern long Graphics_GetAdapterIdentifier( DWORD Adapter, DWORD Flags, D_D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	return GAPIWin.Direct3D9Object->GetAdapterIdentifier( Adapter, Flags, pIdentifier) ;
}

extern UINT Graphics_GetAdapterModeCount( DWORD Adapter, D_D3DFORMAT Format)
{
	return GAPIWin.Direct3D9Object->GetAdapterModeCount( Adapter, Format) ;
}

extern long Graphics_EnumAdapterModes( DWORD Adapter, D_D3DFORMAT Format, DWORD Mode,D_D3DDISPLAYMODE* pMode)
{
	return GAPIWin.Direct3D9Object->EnumAdapterModes( Adapter, Format, Mode,pMode) ;
}

extern long Graphics_GetAdapterDisplayMode( DWORD Adapter, D_D3DDISPLAYMODE* pMode)
{
	return GAPIWin.Direct3D9Object->GetAdapterDisplayMode( Adapter, pMode) ;
}

extern long Graphics_CheckDeviceFormat( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT AdapterFormat, DWORD Usage, D_D3DRESOURCETYPE RType, D_D3DFORMAT CheckFormat)
{
	return GAPIWin.Direct3D9Object->CheckDeviceFormat( Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat) ;
}

extern long Graphics_CheckDeviceMultiSampleType( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT SurfaceFormat, BOOL Windowed, D_D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return GAPIWin.Direct3D9Object->CheckDeviceMultiSampleType( Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels) ;
}

extern long Graphics_CheckDepthStencilMatch( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT AdapterFormat, D_D3DFORMAT RenderTargetFormat, D_D3DFORMAT DepthStencilFormat)
{
	return GAPIWin.Direct3D9Object->CheckDepthStencilMatch( Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat) ;
}

extern long Graphics_GetDeviceCaps( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DCAPS9* pCaps)
{
	return GAPIWin.Direct3D9Object->GetDeviceCaps( Adapter, DeviceType, pCaps) ;
}

extern int Graphics_CreateDevice( void )
{
	D_D3DCAPS9 DevCaps ;
	D_D3DPRESENT_PARAMETERS param ;
	HRESULT hr ;
	int ModeCount ;
	D_D3DDISPLAYMODE DisplayMode ;
	D_D3DDISPLAYMODEEX DisplayModeEx ;
	int ErrorRet = -1 ;

	// パラメータのセット
	_MEMSET( &param, 0, sizeof( D_D3DPRESENT_PARAMETERS ) ) ;
	param.BackBufferWidth = GRA2.MainScreenSizeX ;
	param.BackBufferHeight = GRA2.MainScreenSizeY ;
	if( NS_GetWindowModeFlag() == TRUE )
	{
		// ウインドウモード
		Graphics_GetAdapterDisplayMode( 0, &DisplayMode ) ;
		param.BackBufferFormat = DisplayMode.Format ;
		param.BackBufferCount = 1 ;
		param.Windowed = TRUE ;
		param.SwapEffect = GRH.FSAAMultiSampleType == D_D3DMULTISAMPLE_NONE ? D_D3DSWAPEFFECT_COPY : D_D3DSWAPEFFECT_DISCARD ;

		switch( DisplayMode.Format )
		{
		case D_D3DFMT_X8R8G8B8 :
			GRA2.MainScreenColorBitDepth = 32 ;
			SetMemImgDefaultColorType( 1 ) ;
			break ;

		case D_D3DFMT_X1R5G5B5 :
		case D_D3DFMT_A1R5G5B5 :
		case D_D3DFMT_R5G6B5 :
			GRA2.MainScreenColorBitDepth = 16 ;
			SetMemImgDefaultColorType( 0 ) ;
			break ;
		}

		// Aero の有効・無効を設定する
		SetEnableAero( GRH.DisableAeroFlag == 2 ? FALSE : TRUE ) ;
	}
	else
	{
		// フルスクリーンモード

		// VISTA 以降の場合で GraphDisplayArea かエミュレーション320x240 が有効な場合はウインドウモードで作成する
		if( WinData.WindowsVersion >= DX_WINDOWSVERSION_VISTA && 
			( GRA2.ValidGraphDisplayArea || GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) )
		{
			Graphics_GetAdapterDisplayMode( 0, &DisplayMode ) ;
			param.BackBufferFormat = DisplayMode.Format ;
			param.BackBufferCount = 1 ;
			param.Windowed = TRUE ;
			param.SwapEffect = GRH.FSAAMultiSampleType == D_D3DMULTISAMPLE_NONE ? D_D3DSWAPEFFECT_COPY : D_D3DSWAPEFFECT_DISCARD ;

			switch( GRA2.MainScreenColorBitDepth )
			{
			case 16 :
				ModeCount = Graphics_GetAdapterModeCount( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_R5G6B5 ) ;
				param.BackBufferFormat = ModeCount != 0 ? D_D3DFMT_R5G6B5 : D_D3DFMT_X1R5G5B5 ;
				break ;

			case 32 :
				param.BackBufferFormat = D_D3DFMT_X8R8G8B8 ;
				break ;

			default :
				DXST_ERRORLOG_ADD( _T( "非対応のバックバッファービット深度が指定されました\n" ) ) ;
				goto ERR ;
			}
		}
		else
		{
			// 真のフルスクリーンモード
			switch( GRA2.MainScreenColorBitDepth )
			{
			case 16 :
				ModeCount = Graphics_GetAdapterModeCount( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DFMT_R5G6B5 ) ;
				param.BackBufferFormat = ModeCount != 0 ? D_D3DFMT_R5G6B5 : D_D3DFMT_X1R5G5B5 ;
				break ;

			case 32 :
				param.BackBufferFormat = D_D3DFMT_X8R8G8B8 ;
				break ;

			default :
				DXST_ERRORLOG_ADD( _T( "非対応のバックバッファービット深度が指定されました\n" ) ) ;
				goto ERR ;
			}
			param.BackBufferCount = 1 ;
			param.Windowed = FALSE ;
			param.FullScreen_RefreshRateInHz = GRA2.MainScreenRefreshRate ;
//			param.SwapEffect = GBASE.Emulation320x240Flag ? D_D3DSWAPEFFECT_COPY : D_D3DSWAPEFFECT_DISCARD ;
			param.SwapEffect = GRH.FSAAMultiSampleType == D_D3DMULTISAMPLE_NONE ? D_D3DSWAPEFFECT_COPY : D_D3DSWAPEFFECT_DISCARD ;

			// ディスプレイモードの設定
			DisplayModeEx.Size = sizeof( DisplayModeEx ) ;
			DisplayModeEx.Format = param.BackBufferFormat ;
			DisplayModeEx.Width = param.BackBufferWidth ;
			DisplayModeEx.Height = param.BackBufferHeight ;
			DisplayModeEx.RefreshRate = param.FullScreen_RefreshRateInHz ;
			DisplayModeEx.ScanLineOrdering = D_D3DSCANLINEORDERING_PROGRESSIVE ;

			// フルスクリーンの場合で明示的に Aero のＯＮを設定していない場合は必ず DWM を無効にする
			if( GRH.DisableAeroFlag != 1 )
			{
				SetEnableAero( FALSE ) ;
			}
			if( WinAPIData.DF_DwmEnableComposition )
			{
				DXST_ERRORLOG_ADD( _T( "Desktop Window Manager を無効にしました\n" ) ) ;
			}
		}
	}
	param.MultiSampleType = GRH.FSAAMultiSampleType ;
	param.MultiSampleQuality = GRH.FSAAMultiSampleQuality ;
	param.hDeviceWindow = GetDisplayWindowHandle() ;

//	param.PresentationInterval = GBASE.PreSetWaitVSyncFlag ? D_D3DPRESENT_INTERVAL_ONE : D_D3DPRESENT_INTERVAL_IMMEDIATE ;
	param.PresentationInterval = GBASE.NotWaitVSyncFlag ? D_D3DPRESENT_INTERVAL_IMMEDIATE : D_D3DPRESENT_INTERVAL_ONE ;
	param.EnableAutoDepthStencil = FALSE ;
	param.Flags = GRH.FSAAMultiSampleType == D_D3DMULTISAMPLE_NONE ? D_D3DPRESENTFLAG_LOCKABLE_BACKBUFFER/* D_D3DPRESENTFLAG_DEVICECLIP */ : 0 ;

	// FSAAの設定値を調べる
	if( GRH.FSAAMultiSampleType != 0 )
	{
		param.MultiSampleType = GRH.FSAAMultiSampleType ;
		param.MultiSampleQuality = GRH.FSAAMultiSampleQuality ;
		Graphics_CheckMultiSampleParam( param.BackBufferFormat, &param.MultiSampleType, &param.MultiSampleQuality, FALSE ) ;
		GRH.FSAAMultiSampleType = param.MultiSampleType ;
		GRH.FSAAMultiSampleQuality = param.MultiSampleQuality ;
	}
	else
	{
		GRH.FSAAMultiSampleQuality = 0 ;
		param.MultiSampleQuality = 0 ;
	}

	if( GRH.NonUseVertexHardwareProcess == TRUE )
	{
		if( GAPIWin.Direct3D9ExObject )
			goto NOTUSEHARDWARE_VERTEXPROCESSINGEX ;
		else
			goto NOTUSEHARDWARE_VERTEXPROCESSING ;
	}

	// シェーダーバージョン２．０を使用できない場合は
	// 頂点処理はすべてソフトウエアで行う
	Graphics_GetDeviceCaps( GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT, D_D3DDEVTYPE_HAL, &DevCaps ) ;
	if( ( DevCaps.VertexShaderVersion & 0xffff ) < 0x200 || 
		( DevCaps.PixelShaderVersion  & 0xffff ) < 0x200 )
	{
		GRH.VertexHardwareProcess = FALSE ;
	}
	else
	{
		GRH.VertexHardwareProcess = TRUE ;
	}
	GRH.NativeVertexShaderVersion = DevCaps.VertexShaderVersion ;
	if( GAPIWin.Direct3D9ExObject )
	{
		DXST_ERRORLOG_ADD( _T( "IDirect3DDevice9Ex オブジェクトを取得します.... " ) ) ;

		hr = GAPIWin.Direct3D9ExObject->CreateDeviceEx(
			GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT,
			D_D3DDEVTYPE_HAL,
			NS_GetMainWindowHandle(),
//			D_D3DCREATE_MIXED_VERTEXPROCESSING
			( GRH.VertexHardwareProcess ? D_D3DCREATE_MIXED_VERTEXPROCESSING : D_D3DCREATE_SOFTWARE_VERTEXPROCESSING )
			| ( WinData.UseFPUPreserve ? D_D3DCREATE_FPU_PRESERVE : 0 )
			| ( GRH.UseMultiThread ? D_D3DCREATE_MULTITHREADED : 0 ),
			&param,
			param.Windowed ? NULL : &DisplayModeEx,
			&GAPIWin.Direct3DDevice9ExObject
		) ;
		if( FAILED( hr ) )
		{
NOTUSEHARDWARE_VERTEXPROCESSINGEX:
			GRH.VertexHardwareProcess = FALSE ;

			// だめだった場合はソフトウエアプロセッシング
			hr = GAPIWin.Direct3D9ExObject->CreateDeviceEx(
				GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT,
				D_D3DDEVTYPE_HAL,
				NS_GetMainWindowHandle(),
				D_D3DCREATE_SOFTWARE_VERTEXPROCESSING
				| ( WinData.UseFPUPreserve ? D_D3DCREATE_FPU_PRESERVE  : 0 )
				| ( GRH.UseMultiThread     ? D_D3DCREATE_MULTITHREADED : 0 ),
				&param,
				param.Windowed ? NULL : &DisplayModeEx,
				&GAPIWin.Direct3DDevice9ExObject
			) ;
			if( FAILED( hr ) )
			{
				GRH.NotUseDirect3D9Ex = TRUE ;

				DXST_ERRORLOG_ADD( _T( "Direct3DDevice9Ex の作成に失敗しました\n" ) ) ;
				ErrorRet = -2 ;
				goto ERR ;
			}
			else
			{
				DXST_ERRORLOG_ADD( _T( "ソフトウエア頂点演算を使用します\n" ) ) ;
			}
		}
		else
		{
			DXST_ERRORLOG_ADD( _T( "ハードウエア頂点演算を使用します\n" ) ) ;
		}
		GAPIWin.Direct3DDevice9Object = GAPIWin.Direct3DDevice9ExObject ;

		// レンダリング保持できるフレーム数を最小にする
		GAPIWin.Direct3DDevice9ExObject->SetMaximumFrameLatency( 1 ) ;
	}
	else
	{
		DXST_ERRORLOG_ADD( _T( "IDirect3DDevice9 オブジェクトを取得します.... " ) ) ;

		hr = GAPIWin.Direct3D9Object->CreateDevice(
			GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT,
			D_D3DDEVTYPE_HAL,
			NS_GetMainWindowHandle(),
//			D_D3DCREATE_MIXED_VERTEXPROCESSING/*D_D3DCREATE_HARDWARE_VERTEXPROCESSING*/
			( GRH.VertexHardwareProcess ? D_D3DCREATE_MIXED_VERTEXPROCESSING : D_D3DCREATE_SOFTWARE_VERTEXPROCESSING )
			| ( WinData.UseFPUPreserve ? D_D3DCREATE_FPU_PRESERVE : 0 )
			| ( GRH.UseMultiThread ? D_D3DCREATE_MULTITHREADED : 0 ),
			&param,
			&GAPIWin.Direct3DDevice9Object
		) ;
		if( FAILED( hr ) )
		{
NOTUSEHARDWARE_VERTEXPROCESSING:
			GRH.VertexHardwareProcess = FALSE ;

			// だめだった場合はソフトウエアプロセッシング
			hr = GAPIWin.Direct3D9Object->CreateDevice(
				GRH.ValidAdapterNumber ? GRH.UseAdapterNumber : D_D3DADAPTER_DEFAULT,
				D_D3DDEVTYPE_HAL,
				NS_GetMainWindowHandle(),
				D_D3DCREATE_SOFTWARE_VERTEXPROCESSING
				| ( WinData.UseFPUPreserve ? D_D3DCREATE_FPU_PRESERVE : 0 )
				| ( GRH.UseMultiThread ? D_D3DCREATE_MULTITHREADED : 0 ),
				&param,
				&GAPIWin.Direct3DDevice9Object
			) ;
			if( FAILED( hr ) )
			{
				DXST_ERRORLOG_ADD( _T( "Direct3DDevice9 の作成に失敗しました\n" ) ) ;
				goto ERR ;
			}
			else
			{
				DXST_ERRORLOG_ADD( _T( "ソフトウエア頂点演算を使用します\n" ) ) ;
			}
		}
		else
		{
			DXST_ERRORLOG_ADD( _T( "ハードウエア頂点演算を使用します\n" ) ) ;
		}
	}

	// バックバッファの初期化
	GraphicsDevice_SetRenderState( D_D3DRS_ZENABLE, D_D3DZB_TRUE ) ;
	GraphicsDevice_Clear( 0, NULL, D_D3DCLEAR_TARGET, ( GRA2.BackgroundRed << 16 ) | ( GRA2.BackgroundGreen << 8 ) | GRA2.BackgroundBlue, 1.0f, 0 ) ;
	GAPIWin.Direct3DDevice9Object->Present( NULL, NULL, GetDisplayWindowHandle(), NULL ) ;
	GraphicsDevice_Clear( 0, NULL, D_D3DCLEAR_TARGET, ( GRA2.BackgroundRed << 16 ) | ( GRA2.BackgroundGreen << 8 ) | GRA2.BackgroundBlue, 1.0f, 0 ) ;
	GAPIWin.Direct3DDevice9Object->Present( NULL, NULL, GetDisplayWindowHandle(), NULL ) ;
	GraphicsDevice_Clear( 0, NULL, D_D3DCLEAR_TARGET, ( GRA2.BackgroundRed << 16 ) | ( GRA2.BackgroundGreen << 8 ) | GRA2.BackgroundBlue, 1.0f, 0 ) ;
	GAPIWin.Direct3DDevice9Object->Present( NULL, NULL, GetDisplayWindowHandle(), NULL ) ;
	GraphicsDevice_Clear( 0, NULL, D_D3DCLEAR_TARGET, ( GRA2.BackgroundRed << 16 ) | ( GRA2.BackgroundGreen << 8 ) | GRA2.BackgroundBlue, 1.0f, 0 ) ;

	// スワップチェインのアドレスを取得
	GAPIWin.Direct3DDevice9Object->GetSwapChain( 0, &GAPIWin.Direct3DSwapChain9Object ) ;

	// 正常終了
	return 0 ;

	// エラー処理
ERR:
	return ErrorRet ;
}

// マルチサンプルレンダリングのサンプル数とクオリティをチェック
extern int Graphics_CheckMultiSampleParam( D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE *Samples, DWORD *Quality, int SamplesFailedBreak )
{
	DWORD ColorBufferQuality, ZBufferQuality, MaxQuality ;

	// カラーバッファで使用できるマルチサンプルタイプとその際のクオリティを取得する
	ColorBufferQuality = 0 ;
	for(;;)
	{
		if( Graphics_CheckDeviceMultiSampleType( 0, D_D3DDEVTYPE_HAL, Format, NS_GetWindowModeFlag(), *Samples, &ColorBufferQuality ) == D_D3D_OK )
			break ;

		if( SamplesFailedBreak )
		{
			*Quality = 0 ;
			return -1 ;
		}

		*Samples = ( D_D3DMULTISAMPLE_TYPE )( ( int )*Samples - 1 ) ;
		if( *Samples == D_D3DMULTISAMPLE_NONE ) break ;
	}

	// 使用できるマルチサンプルタイプが無かったら終了
	if( *Samples == D_D3DMULTISAMPLE_NONE )
	{
		*Quality = 0 ;
		return 0 ;
	}

	// Ｚバッファで使用できるマルチサンプルタイプとその際のクオリティを取得する
	ZBufferQuality = 0 ;
	for(;;)
	{
		if( Graphics_CheckDeviceMultiSampleType( 0, D_D3DDEVTYPE_HAL, D_D3DFMT_D16, NS_GetWindowModeFlag(), *Samples, &ZBufferQuality ) == D_D3D_OK )
			break ;

		if( SamplesFailedBreak )
		{
			*Quality = 0 ;
			return -1 ;
		}

		*Samples = ( D_D3DMULTISAMPLE_TYPE )( ( int )*Samples - 1 ) ;
		if( *Samples == D_D3DMULTISAMPLE_NONE ) break ;
	}

	// 使用できるマルチサンプルタイプが無かったら終了
	if( *Samples == D_D3DMULTISAMPLE_NONE )
	{
		*Quality = 0 ;
		return 0 ;
	}

	// クオリティは低いほうに合わせる
	MaxQuality = ( ZBufferQuality < ColorBufferQuality ? ZBufferQuality : ColorBufferQuality ) - 1 ;
	if( *Quality > MaxQuality ) *Quality = MaxQuality ;

	// 終了
	return 0 ;
}

extern int Graphics_IsValid( void )
{
	return GAPIWin.Direct3D9Object != NULL ? 1 : 0 ;
}

extern int Graphics_IsExObject( void )
{
	return GAPIWin.Direct3D9ExObject != NULL ? 1 : 0 ;
}






extern long GraphicsDevice_SetDialogBoxMode( BOOL bEnableDialogs )
{
	return GAPIWin.Direct3DDevice9Object->SetDialogBoxMode( bEnableDialogs ) ;
}



extern int GraphicsDevice_Release( void )
{
	// スワップチェインの解放
	if( GAPIWin.Direct3DSwapChain9Object )
	{
		GAPIWin.Direct3DSwapChain9Object->Release() ;
		GAPIWin.Direct3DSwapChain9Object = NULL ;
	}

	if( GAPIWin.Direct3DDevice9Object )
	{
		DXST_ERRORLOG_ADD( _T( "Direct3DDevice9 の解放 2\n" ) ) ;

		GAPIWin.Direct3DDevice9Object->Release() ;
		GAPIWin.Direct3DDevice9Object = NULL ;
	}

	// 正常終了
	return 0 ;
}

extern UINT GraphicsDevice_GetAvailableTextureMem( void )
{
	return GAPIWin.Direct3DDevice9Object->GetAvailableTextureMem();
}

extern long GraphicsDevice_GetDeviceCaps( D_D3DCAPS9* pCaps)
{
	return GAPIWin.Direct3DDevice9Object->GetDeviceCaps( pCaps );
}

extern long GraphicsDevice_GetBackBuffer( UINT iSwapChain, UINT iBackBuffer, D_D3DBACKBUFFER_TYPE Type, DX_DIRECT3DSURFACE9** ppBackBuffer )
{
	return GAPIWin.Direct3DDevice9Object->GetBackBuffer( iSwapChain, iBackBuffer, Type, ppBackBuffer ) ;
}

extern long GraphicsDevice_CreateTexture( UINT Width, UINT Height, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DTEXTURE9** ppTexture, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateTexture( Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle) ;
}

extern long GraphicsDevice_CreateVolumeTexture( UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DVOLUMETEXTURE9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateVolumeTexture( Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle) ;
}

extern long GraphicsDevice_CreateCubeTexture( UINT EdgeLength, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DCUBETEXTURE9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateCubeTexture( EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle) ;
}

extern long GraphicsDevice_CreateVertexBuffer( UINT Length, DWORD Usage, DWORD FVF, D_D3DPOOL Pool, DX_DIRECT3DVERTEXBUFFER9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateVertexBuffer( Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle) ;
}

extern long GraphicsDevice_CreateIndexBuffer( UINT Length, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DINDEXBUFFER9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateIndexBuffer( Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle) ;
}

extern long GraphicsDevice_CreateRenderTarget( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateRenderTarget( Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle) ;
}

extern long GraphicsDevice_CreateDepthStencilSurface( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateDepthStencilSurface( Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle) ;
}

extern long GraphicsDevice_UpdateSurface( DX_DIRECT3DSURFACE9* pSourceSurface, CONST RECT* pSourceRect, DX_DIRECT3DSURFACE9* pDestinationSurface, CONST POINT* pDestPoint)
{
	return GAPIWin.Direct3DDevice9Object->UpdateSurface( pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint) ;
}

extern long GraphicsDevice_UpdateTexture( DX_DIRECT3DBASETEXTURE9* pSourceTexture, DX_DIRECT3DBASETEXTURE9* pDestinationTexture)
{
	return GAPIWin.Direct3DDevice9Object->UpdateTexture( pSourceTexture, pDestinationTexture) ;
}

extern long GraphicsDevice_GetRenderTargetData( DX_DIRECT3DSURFACE9* pRenderTarget, DX_DIRECT3DSURFACE9* pDestSurface)
{
	return GAPIWin.Direct3DDevice9Object->GetRenderTargetData( pRenderTarget, pDestSurface) ;
}

extern long GraphicsDevice_StretchRect( DX_DIRECT3DSURFACE9* pSourceSurface, CONST RECT* pSourceRect, DX_DIRECT3DSURFACE9* pDestSurface, CONST RECT* pDestRect, D_D3DTEXTUREFILTERTYPE Filter)
{
	return GAPIWin.Direct3DDevice9Object->StretchRect( pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter) ;
}

extern long GraphicsDevice_ColorFill( DX_DIRECT3DSURFACE9* pSurface, CONST RECT* pRect, D_D3DCOLOR color)
{
	return GAPIWin.Direct3DDevice9Object->ColorFill( pSurface, pRect, color) ;
}

extern long GraphicsDevice_CreateOffscreenPlainSurface( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle)
{
	return GAPIWin.Direct3DDevice9Object->CreateOffscreenPlainSurface( Width, Height, Format, Pool, ppSurface, pSharedHandle) ;
}

extern long GraphicsDevice_SetRenderTarget( DWORD RenderTargetIndex, DX_DIRECT3DSURFACE9* pRenderTarget)
{
	return GAPIWin.Direct3DDevice9Object->SetRenderTarget( RenderTargetIndex, pRenderTarget) ;
}

extern long GraphicsDevice_SetDepthStencilSurface( DX_DIRECT3DSURFACE9* pNewZStencil)
{
	return GAPIWin.Direct3DDevice9Object->SetDepthStencilSurface( pNewZStencil) ;
}

extern long GraphicsDevice_GetDepthStencilSurface( DX_DIRECT3DSURFACE9** ppZStencilSurface)
{
	return GAPIWin.Direct3DDevice9Object->GetDepthStencilSurface( ppZStencilSurface) ;
}

extern long GraphicsDevice_BeginScene( void )
{
	return GAPIWin.Direct3DDevice9Object->BeginScene() ;
}

extern long GraphicsDevice_EndScene( void )
{
	return GAPIWin.Direct3DDevice9Object->EndScene() ;
}

extern long GraphicsDevice_Clear( DWORD Count, CONST D_D3DRECT* pRects, DWORD Flags, D_D3DCOLOR Color, float Z, DWORD Stencil)
{
	return GAPIWin.Direct3DDevice9Object->Clear( Count, pRects, Flags, Color, Z, Stencil) ;
}

extern long GraphicsDevice_SetTransform( D_D3DTRANSFORMSTATETYPE State, CONST D_D3DMATRIX* pMatrix)
{
	return GAPIWin.Direct3DDevice9Object->SetTransform( State, pMatrix) ;
}

extern long GraphicsDevice_SetViewport( CONST D_D3DVIEWPORT9* pViewport)
{
	return GAPIWin.Direct3DDevice9Object->SetViewport( pViewport) ;
}

extern long GraphicsDevice_SetMaterial( CONST D_D3DMATERIAL9* pMaterial)
{
	return GAPIWin.Direct3DDevice9Object->SetMaterial( pMaterial) ;
}

extern long GraphicsDevice_GetMaterial( D_D3DMATERIAL9* pMaterial)
{
	return GAPIWin.Direct3DDevice9Object->GetMaterial( pMaterial) ;
}

extern long GraphicsDevice_SetLight( DWORD Index, CONST D_D3DLIGHT9* Param )
{
	return GAPIWin.Direct3DDevice9Object->SetLight( Index, Param ) ;
}

extern long GraphicsDevice_LightEnable( DWORD Index, BOOL Enable)
{
	return GAPIWin.Direct3DDevice9Object->LightEnable( Index, Enable) ;
}

extern long GraphicsDevice_SetRenderState( D_D3DRENDERSTATETYPE State, DWORD Value )
{
	return GAPIWin.Direct3DDevice9Object->SetRenderState( State, Value ) ;
}

extern long GraphicsDevice_SetTexture( DWORD Stage, DX_DIRECT3DBASETEXTURE9* pTexture)
{
	return GAPIWin.Direct3DDevice9Object->SetTexture( Stage, pTexture) ;
}

extern long GraphicsDevice_SetTextureStageState( DWORD Stage, D_D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return GAPIWin.Direct3DDevice9Object->SetTextureStageState( Stage, Type, Value) ;
}

extern long GraphicsDevice_SetSamplerState( DWORD Sampler, D_D3DSAMPLERSTATETYPE Type, DWORD Value )
{
	return GAPIWin.Direct3DDevice9Object->SetSamplerState( Sampler, Type, Value ) ;
}

extern long GraphicsDevice_SetScissorRect( CONST RECT* pRect)
{
	return GAPIWin.Direct3DDevice9Object->SetScissorRect( pRect) ;
}

extern long GraphicsDevice_DrawPrimitive( D_D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount )
{
	return GAPIWin.Direct3DDevice9Object->DrawPrimitive( PrimitiveType,  StartVertex,  PrimitiveCount ) ;
}

extern long GraphicsDevice_DrawIndexedPrimitive( D_D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount )
{
	return GAPIWin.Direct3DDevice9Object->DrawIndexedPrimitive( PrimitiveType,  BaseVertexIndex,  MinVertexIndex,  NumVertices,  startIndex,  primCount ) ;
}

extern long GraphicsDevice_DrawPrimitiveUP( D_D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride )
{
	return GAPIWin.Direct3DDevice9Object->DrawPrimitiveUP( PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride ) ;
}

extern long GraphicsDevice_DrawIndexedPrimitiveUP( D_D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D_D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride )
{
	return GAPIWin.Direct3DDevice9Object->DrawIndexedPrimitiveUP( PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride ) ;
}

extern long GraphicsDevice_CreateVertexDeclaration( CONST D_D3DVERTEXELEMENT9* pVertexElements, DX_DIRECT3DVERTEXDECLARATION9** ppDecl)
{
	return GAPIWin.Direct3DDevice9Object->CreateVertexDeclaration( pVertexElements, ppDecl) ;
}

extern long GraphicsDevice_SetVertexDeclaration( DX_DIRECT3DVERTEXDECLARATION9* pDecl )
{
	return GAPIWin.Direct3DDevice9Object->SetVertexDeclaration( pDecl ) ;
}

extern long GraphicsDevice_SetFVF( DWORD FVF)
{
	return GAPIWin.Direct3DDevice9Object->SetFVF( FVF ) ;
}

extern long GraphicsDevice_CreateVertexShader( CONST DWORD* pFunction, DX_DIRECT3DVERTEXSHADER9** ppShader)
{
	return GAPIWin.Direct3DDevice9Object->CreateVertexShader( pFunction, ppShader) ;
}

extern long GraphicsDevice_SetVertexShader( DX_DIRECT3DVERTEXSHADER9* pShader)
{
	return GAPIWin.Direct3DDevice9Object->SetVertexShader( pShader) ;
}

extern long GraphicsDevice_SetVertexShaderConstantF( UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return GAPIWin.Direct3DDevice9Object->SetVertexShaderConstantF( StartRegister, pConstantData, Vector4fCount) ;
}

extern long GraphicsDevice_SetVertexShaderConstantI( UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return GAPIWin.Direct3DDevice9Object->SetVertexShaderConstantI( StartRegister, pConstantData, Vector4iCount) ;
}

extern long GraphicsDevice_SetVertexShaderConstantB( UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return GAPIWin.Direct3DDevice9Object->SetVertexShaderConstantB( StartRegister, pConstantData,  BoolCount) ;
}

extern long GraphicsDevice_SetStreamSource( UINT StreamNumber, DX_DIRECT3DVERTEXBUFFER9* pStreamData, UINT OffsetInBytes, UINT Stride )
{
	return GAPIWin.Direct3DDevice9Object->SetStreamSource( StreamNumber, pStreamData, OffsetInBytes, Stride ) ;
}

extern long GraphicsDevice_SetIndices( DX_DIRECT3DINDEXBUFFER9* pIndexData )
{
	return GAPIWin.Direct3DDevice9Object->SetIndices( pIndexData ) ;
}

extern long GraphicsDevice_CreatePixelShader( CONST DWORD* pFunction, DX_DIRECT3DPIXELSHADER9** ppShader)
{
	return GAPIWin.Direct3DDevice9Object->CreatePixelShader( pFunction, ppShader) ;
}

extern long GraphicsDevice_SetPixelShader( DX_DIRECT3DPIXELSHADER9* pShader)
{
	return GAPIWin.Direct3DDevice9Object->SetPixelShader( pShader) ;
}

extern long GraphicsDevice_SetPixelShaderConstantF( UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return GAPIWin.Direct3DDevice9Object->SetPixelShaderConstantF( StartRegister, pConstantData, Vector4fCount) ;
}

extern long GraphicsDevice_SetPixelShaderConstantI( UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return GAPIWin.Direct3DDevice9Object->SetPixelShaderConstantI( StartRegister, pConstantData, Vector4iCount) ;
}

extern long GraphicsDevice_SetPixelShaderConstantB( UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return GAPIWin.Direct3DDevice9Object->SetPixelShaderConstantB( StartRegister, pConstantData,  BoolCount) ;
}

// テンポラリプライマリバッファの内容をプライマリバッファに転送するコールバック関数
#if _MSC_VER > 1200
static VOID CALLBACK ScreenFlipTimerProc( HWND /*hwnd*/, UINT /*uMsg*/, UINT_PTR /*idEvent*/, DWORD /*dwTime*/ )
#else
static VOID CALLBACK ScreenFlipTimerProc( HWND /*hwnd*/, UINT /*uMsg*/, UINT /*idEvent*/, DWORD /*dwTime*/ )
#endif
{
	KillTimer( NS_GetMainWindowHandle(), SCREENFLIPTIMER_ID );
	if( GBASE.TargetScreen[ 0 ] == DX_SCREEN_FRONT &&
		GBASE.TargetScreenSurface[ 0 ] == 0 &&
		WinData.BackBufferTransColorFlag == FALSE &&
		WinData.UseUpdateLayerdWindowFlag == FALSE )
	{
		if( NS_GetActiveFlag() == TRUE )
		{
			NS_ScreenCopy() ;
		}
//		BltTempPrimaryToPrimary();
		SetTimer( NS_GetMainWindowHandle(), SCREENFLIPTIMER_ID, 32, ScreenFlipTimerProc );
	}
}

extern int GraphicsDevice_Present( RECT *CopyRect )
{
	RECT WindRect, ScreenRect ;
	int DrawScreenWidth, DrawScreenHeight, ValidEmulation320x240Flag = FALSE ;
	RECT TCopyRect ;

	// フィットさせない場合は転送元を計算
	if( CopyRect )
	{
		int LCW, LCH ;

		LCW = GRA2.LastCopySrcRect.right - GRA2.LastCopySrcRect.left ;
		LCH = GRA2.LastCopySrcRect.bottom - GRA2.LastCopySrcRect.top ;
		TCopyRect.left   = CopyRect->left   * GRA2.MainScreenSizeX / LCW + GRA2.LastCopySrcRect.left ;
		TCopyRect.right  = CopyRect->right  * GRA2.MainScreenSizeX / LCW + GRA2.LastCopySrcRect.left ;
		TCopyRect.top    = CopyRect->top    * GRA2.MainScreenSizeY / LCH + GRA2.LastCopySrcRect.top ;
		TCopyRect.bottom = CopyRect->bottom * GRA2.MainScreenSizeY / LCH + GRA2.LastCopySrcRect.top ;
		if( TCopyRect.left == TCopyRect.right ||
			TCopyRect.top  == TCopyRect.bottom )
			goto ERR ;
	}

	// 転送元となる画面の矩形をセット
	{
		NS_GetDrawScreenSize( &DrawScreenWidth, &DrawScreenHeight ) ;
		if( GRA2.ValidGraphDisplayArea )
		{
			ScreenRect = GRA2.GraphDisplayArea ;
		}
		else
		{
			ScreenRect.left   = 0 ;
			ScreenRect.top    = 0 ;
			ScreenRect.right  = GRA2.MainScreenSizeX ;
			ScreenRect.bottom = GRA2.MainScreenSizeY ;
		}

		if( ( GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) && GRA2.MainScreenSizeX == 640 && GRA2.MainScreenSizeY == 480 )
		{
			ScreenRect.left   = 0 ;
			ScreenRect.top    = 0 ;
			ScreenRect.right  = 320 ;
			ScreenRect.bottom = 240 ;
			ValidEmulation320x240Flag = TRUE ;
		}
	}

	// ウインドウモードの場合は転送先ウインドウのデスクトップ上の座標を割り出す
	if( NS_GetWindowModeFlag() == TRUE )
	{
		int ClientHeight ;

		WindRect.left   = 0 ;
		WindRect.top    = 0 ;
		if( ( GBASE.Emulation320x240Flag || GRH.FullScreenEmulation320x240 ) && GRA2.MainScreenSizeX == 640 && GRA2.MainScreenSizeY == 480 )
		{
			NS_GetDrawScreenSize( &DrawScreenWidth, &DrawScreenHeight ) ;
			ClientHeight = WinData.WindowRect.bottom - WinData.WindowRect.top ;
			WindRect.right  = DrawScreenWidth ;
			WindRect.bottom = DrawScreenHeight ;
		}
		else
		{
			double ExRateX, ExRateY ;

			// 現在のウインドウのクライアント領域のサイズを得る
			NS_GetWindowSizeExtendRate( &ExRateX, &ExRateY ) ;
			WindRect.right  = _DTOL( DrawScreenWidth  * ExRateX ) ;
			WindRect.bottom = _DTOL( DrawScreenHeight * ExRateY ) ;
		}

		if( WinData.ToolBarUseFlag )
		{
			ClientHeight = WinData.WindowRect.bottom - WinData.WindowRect.top ;
			WindRect.top    = ClientHeight - DrawScreenHeight ;
			WindRect.bottom = ClientHeight ;
		}
	}
	else
	{
		WindRect.left   = 0 ;
		WindRect.top    = 0 ;
		WindRect.right  = GRA2.MainScreenSizeX ;
		WindRect.bottom = GRA2.MainScreenSizeY ;
	}

	// ハードウエア機能を使っているかどうかで処理を分岐
	if( GRA2.ValidHardWare )
	{
		// 使っている場合
		RenderVertexHardware() ;
		EndScene();

		// バックバッファの透過色の部分を透過するフラグか、UpdateLayerdWindow を使用するフラグが立っている場合は処理を分岐
		if( WinData.BackBufferTransColorFlag && WinData.UseUpdateLayerdWindowFlag )
		{
			BASEIMAGE BackBufferImage ;
			RECT LockRect ;
			int OldTargetScreen ;
			int OldTargetScreenSurfaceIndex ;
//			HDC BackBufferDC ;

			// ＶＳＹＮＣを待つ	
			if( GBASE.NotWaitVSyncFlag == FALSE ) GraphicsDevice_LocalWaitVSync() ;

			// GetDC を使う方法
//			if( WinData.WindowsVersion < DX_WINDOWSVERSION_VISTA && GRH.BackBufferSurface->GetDC( &BackBufferDC ) == D_D3D_OK )
//			{
//				UpdateBackBufferTransColorWindow( NULL, BackBufferDC ) ;
//				GRH.BackBufferSurface->ReleaseDC( BackBufferDC ) ;
//			}
//			else
			{
				// 駄目だったら GetDrawScreenBaseImage を使う方法
//				NS_CreateXRGB8ColorBaseImage( GRA2.MainScreenSizeX, GRA2.MainScreenSizeY, &BackBufferImage ) ;
//				NS_GetDrawScreenBaseImage( 0, 0, GRA2.MainScreenSizeX, GRA2.MainScreenSizeY, &BackBufferImage ) ;
//				UpdateBackBufferTransColorWindow( &BackBufferImage ) ;
//				NS_ReleaseBaseImage( &BackBufferImage ) ;

				// 駄目だったら LockDrawScreenBuffer を使う方法
				OldTargetScreen = GBASE.TargetScreen[ 0 ] ;
				OldTargetScreenSurfaceIndex = GBASE.TargetScreenSurface [ 0 ] ;
				GBASE.TargetScreen[ 0 ] = DX_SCREEN_BACK ;
				GBASE.TargetScreenSurface[ 0 ] = 0 ;
				LockRect.left = 0 ;
				LockRect.top = 0 ;
				LockRect.right = GRA2.MainScreenSizeX ;
				LockRect.bottom = GRA2.MainScreenSizeY ;
				if( LockDrawScreenBuffer( &LockRect, &BackBufferImage, -1, -1, TRUE, 0 ) < 0 )
					goto ERR ;
				UpdateBackBufferTransColorWindow( &BackBufferImage ) ;
				UnlockDrawScreenBuffer() ;
				GBASE.TargetScreen[ 0 ] = OldTargetScreen ;
				GBASE.TargetScreenSurface[ 0 ] = OldTargetScreenSurfaceIndex ;
			}
		}
		else
		{
			// 320x240画面エミュレーションモードの場合は処理を分岐
			if( ValidEmulation320x240Flag )
			{
				WindRect.left = 0 ;
				WindRect.top = 0 ;
				WindRect.right = 640 ;
				WindRect.bottom = 480 ;

				ScreenRect.left = 0 ;
				ScreenRect.top = 0 ;
				ScreenRect.right = 320 ;
				ScreenRect.bottom = 240 ;
			}

			// サブバックバッファを使用している場合は内容をメインバックバッファに転送する
			if( GRH.SubBackBufferSurface != NULL )
			{
				// 使っている場合
				if( GraphicsDevice_StretchRect( GRH.SubBackBufferSurface, NULL, GRH.BackBufferSurface, NULL, D_D3DTEXF_NONE ) != D_D3D_OK )
					goto ERR ;
			}

			// フリップ
			if( GAPIWin.Direct3DSwapChain9Object )
			{
				LONGLONG Time ;
/*
				// ＶＳＹＮＣ待ち
				if( GBASE.PreSetWaitVSyncFlag == FALSE && GBASE.NotWaitVSyncFlag == FALSE )
				{
					GraphicsDevice_LocalWaitVSync() ;
				}
*/

				// 経過時間を計測
				Time = NS_GetNowHiPerformanceCount() ;

				// 最小化していないときだけ Present を実行
				if( WinData.WindowMinSizeFlag == FALSE )
				{
					// ウインドウモードかどうかで処理を分岐( とりあえずエミュレーションモードの場合は全ての場合でサブバックバッファを使用することにしました )
					if( NS_GetWindowModeFlag() )
					{
						// ウインドウモードの場合は転送矩形を指定できる
						if( CopyRect == NULL )
						{
							GRA2.LastCopySrcRect = ScreenRect ;
							GRA2.LastCopyDestRect = WindRect ;
	/*
							DXST_ERRORLOGFMT_ADD(( _T( "src rect left:%d top:%d right:%d bottom:%d" ),
								ScreenRect.left,
								ScreenRect.top,
								ScreenRect.right,
								ScreenRect.bottom )) ;

							DXST_ERRORLOGFMT_ADD(( _T( "dest rect left:%d top:%d right:%d bottom:%d" ),
								WindRect.left,
								WindRect.top,
								WindRect.right,
								WindRect.bottom )) ;
	*/
							if( GAPIWin.Direct3DSwapChain9Object->Present( &GRA2.LastCopySrcRect, &GRA2.LastCopyDestRect, GetDisplayWindowHandle(), NULL, 0 ) != D_D3D_OK )
								goto ERR ;
						}
						else
						{
							// フィットさせない場合は転送元を計算
							int LCW, LCH ;
							LCW = WindRect.right - WindRect.left ;
							LCH = WindRect.bottom - WindRect.top ;
							TCopyRect.left   = CopyRect->left   * GRA2.MainScreenSizeX / LCW + WindRect.left ;
							TCopyRect.right  = CopyRect->right  * GRA2.MainScreenSizeX / LCW + WindRect.left ;
							TCopyRect.top    = CopyRect->top    * GRA2.MainScreenSizeY / LCH + WindRect.top ;
							TCopyRect.bottom = CopyRect->bottom * GRA2.MainScreenSizeY / LCH + WindRect.top ;

	/*
							DXST_ERRORLOGFMT_ADD(( _T( "copy rect left:%d top:%d right:%d bottom:%d" ),
								CopyRect->left,
								CopyRect->top,
								CopyRect->right,
								CopyRect->bottom )) ;
							DXST_ERRORLOGFMT_ADD(( _T( "tcopy rect left:%d top:%d right:%d bottom:%d" ),
								TCopyRect.left,
								TCopyRect.top,
								TCopyRect.right,
								TCopyRect.bottom )) ;
	*/

							if( GAPIWin.Direct3DSwapChain9Object->Present( &TCopyRect, CopyRect, GetDisplayWindowHandle(), NULL, 0 ) != D_D3D_OK )
								goto ERR ;
						}
					}
					else
					{
						if( ValidEmulation320x240Flag || GRA2.ValidGraphDisplayArea )
						{
							if( GAPIWin.Direct3DSwapChain9Object->Present( &ScreenRect, &WindRect, GetDisplayWindowHandle(), NULL, 0 ) != D_D3D_OK )
								goto ERR ;
						}
						else
						{
							if( GAPIWin.Direct3DSwapChain9Object->Present( NULL, NULL, GetDisplayWindowHandle(), NULL, 0 ) != D_D3D_OK )
								goto ERR ;
						}
					}
				}

				// VSYNC待ちをする指定をしているのに殆ど時間が経過していなかったらVSYNC待ちをする
				Time = NS_GetNowHiPerformanceCount() - Time ;
				if( NS_GetAlwaysRunFlag() &&
					GBASE.NotWaitVSyncFlag == FALSE &&
					Time < 500 )
				{
					GraphicsDevice_LocalWaitVSync() ;
				}
			}
		}
	}
	else
	{
		// ソフトウエアレンダリングの場合
		char Buffer[ sizeof( BITMAPINFOHEADER ) + sizeof( DWORD ) * 3 + 4 ];
		BITMAPINFO *pBmpInfo;
		HDC Dc;

		if( CheckValidMemImg( &GRS.MainBufferMemImg ) == FALSE ) goto END ;

		// ＶＳＹＮＣを待つ	
		if( GBASE.NotWaitVSyncFlag == FALSE ) GraphicsDevice_LocalWaitVSync() ;

		// バックバッファの透過色の部分を透過するフラグか、UpdateLayerdWindow を使用するフラグが立っている場合は処理を分岐
		if( WinData.BackBufferTransColorFlag && WinData.UseUpdateLayerdWindowFlag )
		{
			BASEIMAGE BackBufferImage ;
			RECT LockRect ;
			int OldTargetScreen ;
			int OldTargetScreenSurface ;

			// ＶＳＹＮＣを待つ	
			if( GBASE.NotWaitVSyncFlag == FALSE ) GraphicsDevice_LocalWaitVSync() ;

			// 駄目だったら GetDrawScreenBaseImage を使う方法
//			NS_CreateXRGB8ColorBaseImage( GRA2.MainScreenSizeX, GRA2.MainScreenSizeY, &BackBufferImage ) ;
//			NS_GetDrawScreenBaseImage( 0, 0, GRA2.MainScreenSizeX, GRA2.MainScreenSizeY, &BackBufferImage ) ;
//			UpdateBackBufferTransColorWindow( &BackBufferImage ) ;
//			NS_ReleaseBaseImage( &BackBufferImage ) ;

			// 駄目だったら LockDrawScreenBuffer を使う方法
			OldTargetScreen = GBASE.TargetScreen[ 0 ] ;
			OldTargetScreenSurface = GBASE.TargetScreenSurface[ 0 ] ;
			GBASE.TargetScreen[ 0 ] = DX_SCREEN_BACK ;
			GBASE.TargetScreenSurface[ 0 ] = 0 ;
			LockRect.left = 0 ;
			LockRect.top = 0 ;
			LockRect.right = GRA2.MainScreenSizeX ;
			LockRect.bottom = GRA2.MainScreenSizeY ;
			if( LockDrawScreenBuffer( &LockRect, &BackBufferImage, -1, -1, TRUE, 0 ) < 0 )
				goto ERR ;
			UpdateBackBufferTransColorWindow( &BackBufferImage ) ;
			UnlockDrawScreenBuffer() ;
			GBASE.TargetScreen[ 0 ] = OldTargetScreen ;
			GBASE.TargetScreenSurface[ 0 ] = OldTargetScreenSurface ;
		}
		else
		{
			// メインウインドウのＤＣを取得して転送する
			pBmpInfo = (BITMAPINFO *)Buffer ;
			_MEMSET( Buffer, 0, sizeof( Buffer ) ) ;
			pBmpInfo->bmiHeader.biSize        = sizeof( pBmpInfo->bmiHeader ) ;
			pBmpInfo->bmiHeader.biWidth       =  GRA2.MainScreenSizeX ;
			pBmpInfo->bmiHeader.biHeight      = -GRA2.MainScreenSizeY ;
			pBmpInfo->bmiHeader.biPlanes      = 1;
			pBmpInfo->bmiHeader.biBitCount    = GRS.MainBufferMemImg.Base->ColorDataP->ColorBitDepth ;
			pBmpInfo->bmiHeader.biCompression = BI_BITFIELDS ;
			( ( DWORD * )&pBmpInfo->bmiColors )[ 0 ] = GRS.MainBufferMemImg.Base->ColorDataP->RedMask ;
			( ( DWORD * )&pBmpInfo->bmiColors )[ 1 ] = GRS.MainBufferMemImg.Base->ColorDataP->GreenMask ;
			( ( DWORD * )&pBmpInfo->bmiColors )[ 2 ] = GRS.MainBufferMemImg.Base->ColorDataP->BlueMask ;

			// ピッチが１６バイトの倍数なので補正
			if( GRS.MainBufferMemImg.Base->ColorDataP->ColorBitDepth == 16 )
			{
				// 16bit
				pBmpInfo->bmiHeader.biWidth = ( pBmpInfo->bmiHeader.biWidth + 16 / 2 - 1 ) / ( 16 / 2 ) * ( 16 / 2 ) ;
			}
			else
			{
				// 32bit
				pBmpInfo->bmiHeader.biWidth = ( pBmpInfo->bmiHeader.biWidth + 16 / 4 - 1 ) / ( 16 / 4 ) * ( 16 / 4 ) ;
			}

			Dc = GetDC( GetDisplayWindowHandle() );
			if( Dc )
			{
				int w, h;

				w = WindRect.right  - WindRect.left ;
				h = WindRect.bottom - WindRect.top ;
				if( ValidEmulation320x240Flag )
				{
					GRA2.LastCopySrcRect.left = 0 ;
					GRA2.LastCopySrcRect.top = 0 ;
					GRA2.LastCopySrcRect.right = 640 ;
					GRA2.LastCopySrcRect.bottom = 480 ;
					GRA2.LastCopyDestRect.left = 0 ;
					GRA2.LastCopyDestRect.top = 0 ;
					GRA2.LastCopyDestRect.right = 640 ;
					GRA2.LastCopyDestRect.bottom = 480 ;
					StretchDIBits(
						Dc,
						0, 0,
						640, 480, 
						0, 240,
						320, 240,
						GRS.MainBufferMemImg.UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
				}
				else
				if( CopyRect )
				{
					HRESULT hrr ;

					hrr = 
					StretchDIBits(
						Dc,
						CopyRect->left, CopyRect->top,
						CopyRect->right - CopyRect->left,
						CopyRect->bottom - CopyRect->top,
						TCopyRect.left, GRA2.MainScreenSizeY - TCopyRect.bottom,
						TCopyRect.right  - TCopyRect.left,
						TCopyRect.bottom - TCopyRect.top,
						( BYTE * )GRS.MainBufferMemImg.UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
					hrr = 0 ;
				}
				else
				{
					if( GRA2.ValidGraphDisplayArea )
					{
						GRA2.LastCopySrcRect = GRA2.GraphDisplayArea ;
						GRA2.LastCopyDestRect = WindRect ;
						StretchDIBits(
							Dc,
							0, 0,
							w, h,
							CopyRect->left, GRA2.MainScreenSizeY - CopyRect->bottom,
							CopyRect->right  - CopyRect->left,
							CopyRect->bottom - CopyRect->top,
							( BYTE * )GRS.MainBufferMemImg.UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
					}
					else
					{
						GRA2.LastCopySrcRect.left = 0 ;
						GRA2.LastCopySrcRect.top = 0 ;
						GRA2.LastCopySrcRect.right = w ;
						GRA2.LastCopySrcRect.bottom = h ;
						if( GRA2.MainScreenSizeX == w && GRA2.MainScreenSizeY == h )
						{
							SetDIBitsToDevice(
								Dc,
								0, 0,
								w, h,
								0, 0,
								0, h, 
								GRS.MainBufferMemImg.UseImage, pBmpInfo, DIB_RGB_COLORS );
						}
						else
						{
							StretchDIBits(
								Dc,
								0, 0,
								w, h, 
								0, 0,
								GRA2.MainScreenSizeX, GRA2.MainScreenSizeY,
								GRS.MainBufferMemImg.UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
						}
					}
				}
				ReleaseDC( GetDisplayWindowHandle(), Dc );
			}
		}
	}

END:
	// 終了
	return 0 ;

ERR:
	// エラー終了
	return -1;
}

extern int GraphicsDevice_BltRectBackScreenToWindow( HWND Window, RECT BackScreenRect, RECT WindowClientRect )
{
	// 矩形チェック
	if( BackScreenRect.left   == BackScreenRect.right    ||
		BackScreenRect.top    == BackScreenRect.bottom   ||
		WindowClientRect.left == WindowClientRect.right  ||
		WindowClientRect.top  == WindowClientRect.bottom ) return -1 ;

	// ハードウエア機能を使っているかどうかで処理を分岐
	if( GRA2.ValidHardWare )
	{
		// 使っている場合
		RenderVertexHardware() ;
		EndScene();

		// サブバックバッファを使用している場合は内容をメインバックバッファに転送する
		if( GRH.SubBackBufferSurface != NULL )
		{
			// 使っている場合
			if( GraphicsDevice_StretchRect( GRH.SubBackBufferSurface, NULL, GRH.BackBufferSurface, NULL, D_D3DTEXF_NONE ) != D_D3D_OK )
				return -1 ;
		}

		// フリップ
		if( GAPIWin.Direct3DSwapChain9Object )
		{
			if( GAPIWin.Direct3DSwapChain9Object->Present( &BackScreenRect, &WindowClientRect, Window, NULL, 0 ) != D_D3D_OK )
				return -1 ;
		}
	}
	else
	{
		// ソフトウエアレンダリングの場合
		char Buffer[ sizeof( BITMAPINFOHEADER ) + sizeof( DWORD ) * 3 + 4 ];
		BITMAPINFO *pBmpInfo;
		HDC Dc;

		if( CheckValidMemImg( &GRS.MainBufferMemImg ) == FALSE ) return 0 ;

		// ＶＳＹＮＣを待つ	
		if( GBASE.NotWaitVSyncFlag == FALSE ) GraphicsDevice_LocalWaitVSync() ;

		// ウインドウのＤＣを取得して転送する
		pBmpInfo = (BITMAPINFO *)Buffer ;
		_MEMSET( Buffer, 0, sizeof( Buffer ) ) ;
		pBmpInfo->bmiHeader.biSize        = sizeof( pBmpInfo->bmiHeader ) ;
		pBmpInfo->bmiHeader.biWidth       =  GRA2.MainScreenSizeX ;
		pBmpInfo->bmiHeader.biHeight      = -GRA2.MainScreenSizeY ;
		pBmpInfo->bmiHeader.biPlanes      = 1;
		pBmpInfo->bmiHeader.biBitCount    = GRS.MainBufferMemImg.Base->ColorDataP->ColorBitDepth ;
		pBmpInfo->bmiHeader.biCompression = BI_BITFIELDS ;
		( ( DWORD * )&pBmpInfo->bmiColors )[ 0 ] = GRS.MainBufferMemImg.Base->ColorDataP->RedMask ;
		( ( DWORD * )&pBmpInfo->bmiColors )[ 1 ] = GRS.MainBufferMemImg.Base->ColorDataP->GreenMask ;
		( ( DWORD * )&pBmpInfo->bmiColors )[ 2 ] = GRS.MainBufferMemImg.Base->ColorDataP->BlueMask ;
		Dc = GetDC( Window );
		if( Dc )
		{
			StretchDIBits(
				Dc,
				WindowClientRect.left, WindowClientRect.top,
				WindowClientRect.right - WindowClientRect.left,
				WindowClientRect.bottom - WindowClientRect.top,
				BackScreenRect.left, GRA2.MainScreenSizeY - BackScreenRect.bottom,
				BackScreenRect.right  - BackScreenRect.left,
				BackScreenRect.bottom - BackScreenRect.top,
				( BYTE * )GRS.MainBufferMemImg.UseImage, pBmpInfo, DIB_RGB_COLORS, SRCCOPY );
			ReleaseDC( Window, Dc );
		}
	}

	// 終了
	return 0 ;
}

// ＶＳＹＮＣの前にSleepする時間を取得する
extern void GraphicsDevice_WaitVSyncInitialize( void )
{
	GRA2.VSyncWaitTime = -1 ;
	if( GAPIWin.DirectDraw7Object )
	{
//		D_DDSURFACEDESC2 Desc2 ;

//		GAPIWin.DirectDraw7Object->GetDisplayMode( &Desc2 ) ;
//		if( Desc2.dwRefreshRate == 0 )
		{
			LONGLONG Time1, Time2, DeltaTime1, DeltaTime2, DeltaTimeTotal, DeltaTimeSize ;
			int Count, LoopCount ;

			DeltaTime1 = 0 ;
			DeltaTime2 = 0 ;
			DeltaTimeTotal = 0 ;
			Count = 0 ;
			LoopCount = 0 ;
			GAPIWin.DirectDraw7Object->WaitForVerticalBlank( D_DDWAITVB_BLOCKBEGIN, 0 ) ;
			Time1 = NS_GetNowHiPerformanceCount() ;
			while( Count < 10 && LoopCount < 60 )
			{
				GAPIWin.DirectDraw7Object->WaitForVerticalBlank( D_DDWAITVB_BLOCKBEGIN, 0 ) ;
				Time2 = NS_GetNowHiPerformanceCount() ;
				DeltaTime2 = Time2 - Time1 ;
				DeltaTimeSize = DeltaTime1 - DeltaTime2 ;
				if( DeltaTimeSize < 0 ) DeltaTimeSize = -DeltaTimeSize ;
				if( DeltaTimeSize < 1000 )
				{
					DeltaTimeTotal += DeltaTime2 ;
					Count ++ ;
				}
				else
				{
					DeltaTimeTotal = 0 ;
					Count = 0 ;
				}
				Time1 = Time2 ;
				DeltaTime1 = DeltaTime2 ;
				LoopCount ++ ;
				if( Count >= 10 )
				{
					GRA2.VSyncWaitTime = ( LONGLONG )( ( int )DeltaTimeTotal / Count ) ;
				}
				Sleep( 5 ) ;
			}
		}
//		else
//		{
//		}
	}

	// １フレーム待つ
	GraphicsDevice_LocalWaitVSync() ;
}

extern void GraphicsDevice_LocalWaitVSync( void )
{
//	BOOL flag ;
	LONGLONG WaitTime ;

	// 前回ＶＳＹＮＣ待ちをしてから最低限待つ時間が過ぎているかどうかで処理を分岐
	WaitTime = GRA2.VSyncWaitTime != -1 ? GRA2.VSyncWaitTime - 3000 : 4000 ; 
	if( NS_GetNowHiPerformanceCount() - GRA2.VSyncTime < WaitTime )
	{
		Sleep( 0 ) ;
		while( NS_GetNowHiPerformanceCount() - GRA2.VSyncTime < WaitTime ) Sleep( 1 ) ;
		GAPIWin.DirectDraw7Object->WaitForVerticalBlank( D_DDWAITVB_BLOCKBEGIN, 0 ) ;
		GRA2.VSyncTime = NS_GetNowHiPerformanceCount() ;
	}
	else
	{
//		GAPIWin.DirectDraw7Object->GetVerticalBlankStatus( &flag ) ;
//		if( flag == FALSE )
		GAPIWin.DirectDraw7Object->WaitForVerticalBlank( D_DDWAITVB_BLOCKBEGIN, 0 ) ;
		GRA2.VSyncTime = NS_GetNowHiPerformanceCount() ;
		Sleep( 0 ) ;
	}
}

extern int GraphicsDevice_WaitVSync( int SyncNum )
{
	int i ;
	BOOL vsync ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	if( GAPIWin.DirectDraw7Object == NULL ) return -1 ;
	if( GBASE.NotDrawFlag ) return 0 ;
	
	if( SyncNum == 0 )
	{
		vsync = FALSE ;
		while( NS_ProcessMessage() == 0 && vsync == FALSE && GAPIWin.DirectDraw7Object != NULL )
			GAPIWin.DirectDraw7Object->GetVerticalBlankStatus( &vsync ) ;
	}
	else
	{
		for( i = 0 ; i < SyncNum ; i ++ )
		{
			vsync = TRUE ;
			while( NS_ProcessMessage() == 0 && vsync == TRUE && GAPIWin.DirectDraw7Object != NULL )
			{
				GAPIWin.DirectDraw7Object->GetVerticalBlankStatus( &vsync ) ;
			}
		
	//		GAPIWin.DirectDraw7Object->WaitForVerticalBlank( DDWAITVB_BLOCKBEGIN, 0 ) ;
			vsync = FALSE ;
			while( NS_ProcessMessage() == 0 && vsync == FALSE && GAPIWin.DirectDraw7Object != NULL )
				GAPIWin.DirectDraw7Object->GetVerticalBlankStatus( &vsync ) ;
			if( NS_ProcessMessage() < 0 )
			{
				return -1 ; 
			}
		}
	}

	// 終了
	return 0 ;
}

extern int GraphicsDevice_IsValid( void )
{
	return GAPIWin.Direct3DDevice9Object != NULL ? 1 : 0 ;
}

extern void *GraphicsDevice_GetObject( void )
{
	return GAPIWin.Direct3DDevice9Object ;
}

extern int GraphicsDevice_IsLost( void )
{
	return GAPIWin.Direct3DDevice9Object->TestCooperativeLevel() != D_D3D_OK ? 1 : 0 ;
}

extern int GraphicsDevice_SetupTimerPresent( int EnableFlag )
{
	if( EnableFlag == FALSE )
		return 0 ;

	SetTimer( NS_GetMainWindowHandle(), SCREENFLIPTIMER_ID, 32, ScreenFlipTimerProc ) ;

	// 終了
	return 0 ;
}















extern long GraphicsTexture_GetSurfaceLevel( DX_DIRECT3DTEXTURE9 *pTexture, UINT Level, DX_DIRECT3DSURFACE9** ppSurfaceLevel)
{
	return pTexture->GetSurfaceLevel( Level, ppSurfaceLevel) ;
}

extern long GraphicsTexture_LockRect( DX_DIRECT3DTEXTURE9 *pTexture, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	return pTexture->LockRect( Level, pLockedRect, pRect, Flags) ;
}

extern long GraphicsTexture_UnlockRect( DX_DIRECT3DTEXTURE9 *pTexture, UINT Level)
{
	return pTexture->UnlockRect( Level) ;
}











extern long GraphicsCubeTexture_GetCubeMapSurface( DX_DIRECT3DCUBETEXTURE9 *pCubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, DX_DIRECT3DSURFACE9** ppCubeMapSurface)
{
	return pCubeTexture->GetCubeMapSurface( FaceType, Level, ppCubeMapSurface ) ;
}

extern long GraphicsCubeTexture_LockRect( DX_DIRECT3DCUBETEXTURE9 *pCubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	return pCubeTexture->LockRect( FaceType, Level, pLockedRect, pRect, Flags) ;
}

extern long GraphicsCubeTexture_UnlockRect( DX_DIRECT3DCUBETEXTURE9 *pCubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level)
{
	return pCubeTexture->UnlockRect( FaceType, Level) ;
}













extern long GraphicsSurface_GetDesc( DX_DIRECT3DSURFACE9 *pSurface, D_D3DSURFACE_DESC *pDesc)
{
	return pSurface->GetDesc( pDesc) ;
}

extern long GraphicsSurface_LockRect( DX_DIRECT3DSURFACE9 *pSurface, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
	return pSurface->LockRect( pLockedRect, pRect, Flags) ;
}

extern long GraphicsSurface_UnlockRect( DX_DIRECT3DSURFACE9 *pSurface )
{
	return pSurface->UnlockRect() ;
}






extern long GraphicsVertexBuffer_Lock( DX_DIRECT3DVERTEXBUFFER9 *pVertexBuffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags )
{
	return pVertexBuffer->Lock( OffsetToLock, SizeToLock, ppbData, Flags ) ;
}

extern long GraphicsVertexBuffer_Unlock( DX_DIRECT3DVERTEXBUFFER9 *pVertexBuffer )
{
	return pVertexBuffer->Unlock() ;
}



extern long GraphicsIndexBuffer_Lock( DX_DIRECT3DINDEXBUFFER9 *pIndexBuffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)
{
	return pIndexBuffer->Lock( OffsetToLock, SizeToLock, ppbData, Flags) ;
}

extern long GraphicsIndexBuffer_Unlock( DX_DIRECT3DINDEXBUFFER9 *pIndexBuffer )
{
	return pIndexBuffer->Unlock() ;
}






}


