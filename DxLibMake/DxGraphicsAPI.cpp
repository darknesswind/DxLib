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
#include "DxGraphicsAPI.h"
#include "DxASyncLoad.h"

namespace DxLib
{

// 宏定义 -----------------------------------------------------------------

// 结构体声明 -----------------------------------------------------------------

// 数据定义 -----------------------------------------------------------------

// 函数声明 -------------------------------------------------------------------


// Program -----------------------------------------------------------------

#ifndef DX_NON_ASYNCLOAD
extern int Graphics_ObjectRelease_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return Graphics_ObjectRelease_ASync(
				( void * )Info->Data[ 0 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT Graphics_ObjectRelease_ASync( void *pObject, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = Graphics_ObjectRelease_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pObject ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return Graphics_ObjectRelease( pObject ) ;
}






#ifndef DX_NON_ASYNCLOAD
int GraphicsDevice_CreateRenderTarget_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateRenderTarget_ASync(
				( UINT )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( D_D3DFORMAT )Info->Data[ 2 ],
				( D_D3DMULTISAMPLE_TYPE )Info->Data[ 3 ],
				( DWORD )Info->Data[ 4 ],
				( BOOL )Info->Data[ 5 ],
				( DX_DIRECT3DSURFACE9** )Info->Data[ 6 ],
				( HANDLE* )Info->Data[ 7 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_CreateRenderTarget_ASync( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateRenderTarget_ASyncCallback ;
		AInfo.Data[ 0 ] = Width ;
		AInfo.Data[ 1 ] = Height ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )Format ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )MultiSample ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )MultisampleQuality ;
		AInfo.Data[ 5 ] = ( DWORD_PTR )Lockable ;
		AInfo.Data[ 6 ] = ( DWORD_PTR )ppSurface ;
		AInfo.Data[ 7 ] = ( DWORD_PTR )pSharedHandle ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_CreateRenderTarget(
					Width,					// 幅
					Height,					// 高さ
					Format,					// フォーマット
					MultiSample,			// マルチサンプリングタイプ
					MultisampleQuality,		// マルチサンプリングクオリティ
					Lockable,				// ロックはできない
					ppSurface,				// DX_DIRECT3DSURFACE9 のポインタを受け取るアドレス
					pSharedHandle			// 絶対 NULL
				) ;
}

#ifndef DX_NON_ASYNCLOAD
int GraphicsDevice_CreateDepthStencilSurface_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateDepthStencilSurface_ASync(
				( UINT )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( D_D3DFORMAT )Info->Data[ 2 ],
				( D_D3DMULTISAMPLE_TYPE )Info->Data[ 3 ],
				( DWORD )Info->Data[ 4 ],
				( BOOL )Info->Data[ 5 ],
				( DX_DIRECT3DSURFACE9** )Info->Data[ 6 ],
				( HANDLE* )Info->Data[ 7 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_CreateDepthStencilSurface_ASync( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateDepthStencilSurface_ASyncCallback ;
		AInfo.Data[ 0 ] = Width ;
		AInfo.Data[ 1 ] = Height ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )Format ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )MultiSample ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )MultisampleQuality ;
		AInfo.Data[ 5 ] = ( DWORD_PTR )Discard ;
		AInfo.Data[ 6 ] = ( DWORD_PTR )ppSurface ;
		AInfo.Data[ 7 ] = ( DWORD_PTR )pSharedHandle ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_CreateDepthStencilSurface(
					Width,					// 幅
					Height,					// 高さ
					Format,					// フォーマット
					MultiSample,			// マルチサンプリングタイプ
					MultisampleQuality,		// マルチサンプリングクオリティ
					Discard,				// ロックはできない
					ppSurface,				// DX_DIRECT3DSURFACE9 のポインタを受け取るアドレス
					pSharedHandle			// 絶対 NULL
				) ;
}

#ifndef DX_NON_ASYNCLOAD
int GraphicsDevice_CreateTexture_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateTexture_ASync(
				( UINT )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( UINT )Info->Data[ 2 ],
				( DWORD )Info->Data[ 3 ],
				( D_D3DFORMAT )Info->Data[ 4 ],
				( D_D3DPOOL )Info->Data[ 5 ],
				( DX_DIRECT3DTEXTURE9** )Info->Data[ 6 ],
				( HANDLE* )Info->Data[ 7 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_CreateTexture_ASync( UINT Width, UINT Height, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DTEXTURE9** ppTexture, HANDLE* pSharedHandle, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateTexture_ASyncCallback ;
		AInfo.Data[ 0 ] = Width ;
		AInfo.Data[ 1 ] = Height ;
		AInfo.Data[ 2 ] = Levels ;
		AInfo.Data[ 3 ] = Usage ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )Format ;
		AInfo.Data[ 5 ] = ( DWORD_PTR )Pool ;
		AInfo.Data[ 6 ] = ( DWORD_PTR )ppTexture ;
		AInfo.Data[ 7 ] = ( DWORD_PTR )pSharedHandle ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_CreateTexture(
				Width,				// 幅
				Height,				// 高さ
				Levels,				// MipMap レベル
				Usage,				// フラグ
				Format,				// フォーマット
				Pool,				// テクスチャを DirectX が管理するかどうか
				ppTexture,			// DX_DIRECT3DTEXTURE9 のポインタを受け取るアドレス
				pSharedHandle		// 絶対 NULL
			) ;
}

#ifndef DX_NON_ASYNCLOAD
int GraphicsDevice_CreateCubeTexture_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateCubeTexture_ASync(
				( UINT )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( DWORD )Info->Data[ 2 ],
				( D_D3DFORMAT )Info->Data[ 3 ],
				( D_D3DPOOL )Info->Data[ 4 ],
				( DX_DIRECT3DCUBETEXTURE9** )Info->Data[ 5 ],
				( HANDLE* )Info->Data[ 6 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_CreateCubeTexture_ASync( UINT EdgeLength, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DCUBETEXTURE9** ppCubeTexture, HANDLE* pSharedHandle, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateCubeTexture_ASyncCallback ;
		AInfo.Data[ 0 ] = EdgeLength ;
		AInfo.Data[ 1 ] = Levels ;
		AInfo.Data[ 2 ] = Usage ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )Format ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )Pool ;
		AInfo.Data[ 5 ] = ( DWORD_PTR )ppCubeTexture ;
		AInfo.Data[ 6 ] = ( DWORD_PTR )pSharedHandle ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_CreateCubeTexture(
				EdgeLength,			// エッジサイズ
				Levels,				// MipMap レベル
				Usage,				// フラグ
				Format,				// フォーマット
				Pool,				// テクスチャを DirectX が管理するかどうか
				ppCubeTexture,		// DX_DIRECT3DCUBETEXTURE9 のポインタを受け取るアドレス
				pSharedHandle		// 絶対 NULL
			) ;
}

#ifndef DX_NON_ASYNCLOAD
int GraphicsDevice_ColorFill_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_ColorFill_ASync(
				( DX_DIRECT3DSURFACE9* )Info->Data[ 0 ],
				( RECT* )Info->Data[ 1 ],
				( D_D3DCOLOR )Info->Data[ 2 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_ColorFill_ASync( DX_DIRECT3DSURFACE9* pSurface, CONST RECT* pRect, D_D3DCOLOR color, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_ColorFill_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pSurface ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )pRect ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )color ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_ColorFill( pSurface, pRect, color ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsDevice_CreateOffscreenPlainSurface_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateOffscreenPlainSurface_ASync(
				( UINT )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( D_D3DFORMAT )Info->Data[ 2 ],
				( D_D3DPOOL )Info->Data[ 3 ],
				( DX_DIRECT3DSURFACE9** )Info->Data[ 4 ],
				( HANDLE* )Info->Data[ 5 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_CreateOffscreenPlainSurface_ASync( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateOffscreenPlainSurface_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Width ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )Height ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )Format ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )Pool ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )ppSurface ;
		AInfo.Data[ 5 ] = ( DWORD_PTR )pSharedHandle ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_CreateOffscreenPlainSurface( Width, Height, Format, Pool, ppSurface, pSharedHandle ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsDevice_UpdateSurface_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_UpdateSurface_ASync(
				( DX_DIRECT3DSURFACE9* )Info->Data[ 0 ],
				( CONST RECT* )Info->Data[ 1 ],
				( DX_DIRECT3DSURFACE9* )Info->Data[ 2 ],
				( CONST POINT* )Info->Data[ 3 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_UpdateSurface_ASync( DX_DIRECT3DSURFACE9* pSourceSurface, CONST RECT* pSourceRect, DX_DIRECT3DSURFACE9* pDestinationSurface, CONST POINT* pDestPoint, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_UpdateSurface_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pSourceSurface ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )pSourceRect ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )pDestinationSurface ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )pDestPoint ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_UpdateSurface( pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsDevice_UpdateTexture_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_UpdateTexture_ASync(
				( DX_DIRECT3DBASETEXTURE9* )Info->Data[ 0 ],
				( DX_DIRECT3DBASETEXTURE9* )Info->Data[ 1 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_UpdateTexture_ASync( DX_DIRECT3DBASETEXTURE9* pSourceTexture, DX_DIRECT3DBASETEXTURE9* pDestinationTexture, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_UpdateTexture_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pSourceTexture ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )pDestinationTexture ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_UpdateTexture( pSourceTexture, pDestinationTexture ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsDevice_CreateVertexShader_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateVertexShader_ASync(
				( CONST DWORD* )Info->Data[ 0 ],
				( DX_DIRECT3DVERTEXSHADER9** )Info->Data[ 1 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_CreateVertexShader_ASync( CONST DWORD* pFunction, DX_DIRECT3DVERTEXSHADER9** ppShader, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateVertexShader_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pFunction ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )ppShader ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_CreateVertexShader( pFunction, ppShader ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsDevice_CreatePixelShader_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreatePixelShader_ASync(
				( CONST DWORD* )Info->Data[ 0 ],
				( DX_DIRECT3DPIXELSHADER9** )Info->Data[ 1 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsDevice_CreatePixelShader_ASync( CONST DWORD* pFunction, DX_DIRECT3DPIXELSHADER9** ppShader, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreatePixelShader_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pFunction ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )ppShader ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsDevice_CreatePixelShader( pFunction, ppShader ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsDevice_CreateVertexBuffer_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateVertexBuffer_ASync(
				( DWORD )Info->Data[ 0 ],
				( DWORD )Info->Data[ 1 ],
				( DWORD )Info->Data[ 2 ],
				( D_D3DPOOL )Info->Data[ 3 ],
				( DX_DIRECT3DVERTEXBUFFER9 ** )Info->Data[ 4 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

// 頂点バッファを作成する
extern int GraphicsDevice_CreateVertexBuffer_ASync( DWORD Length, DWORD Usage, DWORD FVFFlag, D_D3DPOOL Pool, DX_DIRECT3DVERTEXBUFFER9 **BufferP, int ASyncThread )
{
	HRESULT Result ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateVertexBuffer_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Length ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )Usage ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )FVFFlag ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )Pool ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )BufferP ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	if( GraphicsDevice_IsValid() == 0 ) return -1 ;
	
	Result = GraphicsDevice_CreateVertexBuffer( Length, Usage, FVFFlag, Pool, BufferP, NULL ) ;

	return Result == D_D3D_OK ? 0 : -1 ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsDevice_CreateIndexBuffer_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsDevice_CreateIndexBuffer_ASync(
				( DWORD )Info->Data[ 0 ],
				( DWORD )Info->Data[ 1 ],
				( D_D3DFORMAT )Info->Data[ 2 ],
				( D_D3DPOOL )Info->Data[ 3 ],
				( DX_DIRECT3DINDEXBUFFER9 ** )Info->Data[ 4 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

// インデックスバッファを作成する
extern int GraphicsDevice_CreateIndexBuffer_ASync( DWORD Length, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DINDEXBUFFER9 **BufferP, int ASyncThread )
{
	HRESULT Result ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsDevice_CreateIndexBuffer_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Length ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )Usage ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )Format ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )Pool ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )BufferP ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	if( GraphicsDevice_IsValid() == 0 ) return -1 ;

	Result = GraphicsDevice_CreateIndexBuffer( Length, Usage, Format, Pool, BufferP, NULL ) ;

	return Result == D_D3D_OK ? 0 : -1 ;
}









#ifndef DX_NON_ASYNCLOAD
int GraphicsTexture_GetSurfaceLevel_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsTexture_GetSurfaceLevel_ASync(
				( DX_DIRECT3DTEXTURE9 * )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( DX_DIRECT3DSURFACE9** )Info->Data[ 2 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsTexture_GetSurfaceLevel_ASync( DX_DIRECT3DTEXTURE9 *Texture, UINT Level,  DX_DIRECT3DSURFACE9** ppSurfaceLevel, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsTexture_GetSurfaceLevel_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Texture ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )Level ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )ppSurfaceLevel ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsTexture_GetSurfaceLevel( Texture, Level, ppSurfaceLevel ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsTexture_LockRect_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsTexture_LockRect_ASync(
				( DX_DIRECT3DTEXTURE9 * )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( D_D3DLOCKED_RECT* )Info->Data[ 2 ],
				( CONST RECT* )Info->Data[ 3 ],
				( DWORD )Info->Data[ 4 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsTexture_LockRect_ASync( DX_DIRECT3DTEXTURE9 *Texture, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsTexture_LockRect_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Texture ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )Level ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )pLockedRect ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )pRect ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )Flags ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsTexture_LockRect( Texture, Level, pLockedRect, pRect, Flags ) ;
}

#ifndef DX_NON_ASYNCLOAD

extern int GraphicsTexture_UnlockRect_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsTexture_UnlockRect_ASync(
				( DX_DIRECT3DTEXTURE9 * )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsTexture_UnlockRect_ASync( DX_DIRECT3DTEXTURE9 *Texture, UINT Level, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsTexture_UnlockRect_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Texture ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )Level ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsTexture_UnlockRect( Texture, Level ) ;
}









#ifndef DX_NON_ASYNCLOAD
int GraphicsCubeTexture_GetCubeMapSurface_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsCubeTexture_GetCubeMapSurface_ASync(
				( DX_DIRECT3DCUBETEXTURE9 * )Info->Data[ 0 ],
				( D_D3DCUBEMAP_FACES )Info->Data[ 1 ],
				( UINT )Info->Data[ 2 ],
				( DX_DIRECT3DSURFACE9** )Info->Data[ 3 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsCubeTexture_GetCubeMapSurface_ASync( DX_DIRECT3DCUBETEXTURE9 *CubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, DX_DIRECT3DSURFACE9** ppCubeMapSurface, int ASyncThread)
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsCubeTexture_GetCubeMapSurface_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )CubeTexture ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )FaceType ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )Level ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )ppCubeMapSurface ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsCubeTexture_GetCubeMapSurface( CubeTexture, FaceType, Level, ppCubeMapSurface ) ;
}










#ifndef DX_NON_ASYNCLOAD
extern int GraphicsCubeTexture_LockRect_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsCubeTexture_LockRect_ASync(
				( DX_DIRECT3DCUBETEXTURE9 * )Info->Data[ 0 ],
				( D_D3DCUBEMAP_FACES )Info->Data[ 1 ],
				( UINT )Info->Data[ 2 ],
				( D_D3DLOCKED_RECT* )Info->Data[ 3 ],
				( CONST RECT* )Info->Data[ 4 ],
				( DWORD )Info->Data[ 5 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsCubeTexture_LockRect_ASync( DX_DIRECT3DCUBETEXTURE9 *CubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsCubeTexture_LockRect_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )CubeTexture ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )FaceType ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )Level ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )pLockedRect ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )pRect ;
		AInfo.Data[ 5 ] = ( DWORD_PTR )Flags ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsCubeTexture_LockRect( CubeTexture, FaceType, Level, pLockedRect, pRect, Flags ) ;
}

#ifndef DX_NON_ASYNCLOAD

extern int GraphicsCubeTexture_UnlockRect_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsCubeTexture_UnlockRect_ASync(
				( DX_DIRECT3DCUBETEXTURE9 * )Info->Data[ 0 ],
				( D_D3DCUBEMAP_FACES )Info->Data[ 1 ],
				( UINT )Info->Data[ 2 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsCubeTexture_UnlockRect_ASync( DX_DIRECT3DCUBETEXTURE9 *CubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, int ASyncThread)
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsCubeTexture_UnlockRect_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )CubeTexture ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )FaceType ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )Level ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsCubeTexture_UnlockRect( CubeTexture, FaceType, Level ) ;
}














#ifndef DX_NON_ASYNCLOAD
extern int GraphicsSurface_LockRect_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsSurface_LockRect_ASync(
				( DX_DIRECT3DSURFACE9* )Info->Data[ 0 ],
				( D_D3DLOCKED_RECT* )Info->Data[ 1 ],
				( CONST RECT* )Info->Data[ 2 ],
				( DWORD )Info->Data[ 3 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsSurface_LockRect_ASync( DX_DIRECT3DSURFACE9* pSurface, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsSurface_LockRect_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pSurface ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )pLockedRect ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )pRect ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )Flags ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsSurface_LockRect( pSurface, pLockedRect, pRect, Flags ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsSurface_UnlockRect_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsSurface_UnlockRect_ASync(
				( DX_DIRECT3DSURFACE9* )Info->Data[ 0 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern HRESULT GraphicsSurface_UnlockRect_ASync( DX_DIRECT3DSURFACE9* pSurface, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsSurface_UnlockRect_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )pSurface ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsSurface_UnlockRect( pSurface ) ;
}








#ifndef DX_NON_ASYNCLOAD
extern int GraphicsVertexBuffer_Lock_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsVertexBuffer_Lock_ASync(
				( DX_DIRECT3DVERTEXBUFFER9 * )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( UINT )Info->Data[ 2 ],
				( void ** )Info->Data[ 3 ],
				( DWORD )Info->Data[ 4 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern	HRESULT	GraphicsVertexBuffer_Lock_ASync( DX_DIRECT3DVERTEXBUFFER9 *Buffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsVertexBuffer_Lock_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Buffer ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )OffsetToLock ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )SizeToLock ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )ppbData ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )Flags ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsVertexBuffer_Lock( Buffer, OffsetToLock, SizeToLock, ppbData, Flags ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsVertexBuffer_Unlock_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsVertexBuffer_Unlock_ASync(
				( DX_DIRECT3DVERTEXBUFFER9 * )Info->Data[ 0 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern	HRESULT GraphicsVertexBuffer_Unlock_ASync( DX_DIRECT3DVERTEXBUFFER9 *Buffer, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsVertexBuffer_Unlock_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Buffer ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsVertexBuffer_Unlock( Buffer ) ;
}










#ifndef DX_NON_ASYNCLOAD
extern int GraphicsIndexBuffer_Lock_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsIndexBuffer_Lock_ASync(
				( DX_DIRECT3DINDEXBUFFER9 * )Info->Data[ 0 ],
				( UINT )Info->Data[ 1 ],
				( UINT )Info->Data[ 2 ],
				( void ** )Info->Data[ 3 ],
				( DWORD )Info->Data[ 4 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern	HRESULT	GraphicsIndexBuffer_Lock_ASync( DX_DIRECT3DINDEXBUFFER9 *Buffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsIndexBuffer_Lock_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Buffer ;
		AInfo.Data[ 1 ] = ( DWORD_PTR )OffsetToLock ;
		AInfo.Data[ 2 ] = ( DWORD_PTR )SizeToLock ;
		AInfo.Data[ 3 ] = ( DWORD_PTR )ppbData ;
		AInfo.Data[ 4 ] = ( DWORD_PTR )Flags ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsIndexBuffer_Lock( Buffer, OffsetToLock, SizeToLock, ppbData, Flags ) ;
}

#ifndef DX_NON_ASYNCLOAD
extern int GraphicsIndexBuffer_Unlock_ASyncCallback( ASYNCLOAD_MAINTHREAD_REQUESTINFO *Info )
{
	return GraphicsIndexBuffer_Unlock_ASync(
				( DX_DIRECT3DINDEXBUFFER9 * )Info->Data[ 0 ],
				FALSE ) ;
}
#endif // DX_NON_ASYNCLOAD

extern	HRESULT GraphicsIndexBuffer_Unlock_ASync( DX_DIRECT3DINDEXBUFFER9 *Buffer, int ASyncThread )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		ASYNCLOAD_MAINTHREAD_REQUESTINFO AInfo ;

		AInfo.Function = GraphicsIndexBuffer_Unlock_ASyncCallback ;
		AInfo.Data[ 0 ] = ( DWORD_PTR )Buffer ;
		return AddASyncLoadRequestMainThreadInfo( &AInfo ) ;
	}
#endif // DX_NON_ASYNCLOAD

	return GraphicsIndexBuffer_Unlock( Buffer ) ;
}



}


