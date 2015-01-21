// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		描画ＡＰＩプログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXGRAPHICSAPI_H__
#define __DXGRAPHICSAPI_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"

#if !defined( __ANDROID )
#include "Windows/DxGraphicsAPIWin.h"
#endif

#ifdef __ANDROID
#include "Android/DxGraphicsAPIAndroid.h"
#endif

#include "DxLib.h"
#include "DxStatic.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 列挙型定義 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 函数原型声明 ----------------------------------------------------------

// プラットフォーム非依存
extern	HRESULT Graphics_ObjectRelease_ASync						( void *pObject, int ASyncThread = FALSE ) ;

extern	HRESULT	GraphicsDevice_CreateRenderTarget_ASync				( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_CreateDepthStencilSurface_ASync		( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_CreateTexture_ASync					( UINT Width, UINT Height, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DTEXTURE9** ppTexture, HANDLE* pSharedHandle, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_CreateCubeTexture_ASync				( UINT EdgeLength, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DCUBETEXTURE9** ppCubeTexture, HANDLE* pSharedHandle, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_ColorFill_ASync						( DX_DIRECT3DSURFACE9* pSurface, CONST RECT* pRect, D_D3DCOLOR color, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_CreateOffscreenPlainSurface_ASync	( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_UpdateSurface_ASync					( DX_DIRECT3DSURFACE9* pSourceSurface, CONST RECT* pSourceRect, DX_DIRECT3DSURFACE9* pDestinationSurface, CONST POINT* pDestPoint, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_UpdateTexture_ASync					( DX_DIRECT3DBASETEXTURE9* pSourceTexture, DX_DIRECT3DBASETEXTURE9* pDestinationTexture, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_CreateVertexShader_ASync				( CONST DWORD* pFunction, DX_DIRECT3DVERTEXSHADER9** ppShader, int ASyncThread ) ;
extern	HRESULT	GraphicsDevice_CreatePixelShader_ASync				( CONST DWORD* pFunction, DX_DIRECT3DPIXELSHADER9** ppShader, int ASyncThread ) ;
extern	int		GraphicsDevice_CreateVertexBuffer_ASync				( DWORD Length, DWORD Usage, DWORD FVFFlag, D_D3DPOOL Pool, DX_DIRECT3DVERTEXBUFFER9 **BufferP, int ASyncThread = FALSE ) ;		// 頂点バッファを作成する
extern	int		GraphicsDevice_CreateIndexBuffer_ASync				( DWORD Length, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DINDEXBUFFER9 **BufferP, int ASyncThread = FALSE ) ;	// インデックスバッファを作成する

extern	HRESULT	GraphicsTexture_GetSurfaceLevel_ASync				( DX_DIRECT3DTEXTURE9 *Texture, UINT Level,  DX_DIRECT3DSURFACE9** ppSurfaceLevel, int ASyncThread ) ;
extern	HRESULT	GraphicsTexture_LockRect_ASync						( DX_DIRECT3DTEXTURE9 *Texture, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags, int ASyncThread ) ;
extern	HRESULT	GraphicsTexture_UnlockRect_ASync					( DX_DIRECT3DTEXTURE9 *Texture, UINT Level, int ASyncThread ) ;

extern	HRESULT GraphicsCubeTexture_GetCubeMapSurface_ASync			( DX_DIRECT3DCUBETEXTURE9 *CubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, DX_DIRECT3DSURFACE9** ppCubeMapSurface, int ASyncThread) ;
extern	HRESULT GraphicsCubeTexture_LockRect_ASync					( DX_DIRECT3DCUBETEXTURE9 *CubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags, int ASyncThread) ;
extern	HRESULT GraphicsCubeTexture_UnlockRect_ASync				( DX_DIRECT3DCUBETEXTURE9 *CubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, int ASyncThread) ;

extern	HRESULT	GraphicsSurface_LockRect_ASync						( DX_DIRECT3DSURFACE9* pSurface, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags, int ASyncThread ) ;
extern	HRESULT	GraphicsSurface_UnlockRect_ASync					( DX_DIRECT3DSURFACE9* pSurface, int ASyncThread ) ;

extern	HRESULT	GraphicsVertexBuffer_Lock_ASync						( DX_DIRECT3DVERTEXBUFFER9 *Buffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags, int ASyncThread = FALSE ) ;
extern	HRESULT GraphicsVertexBuffer_Unlock_ASync					( DX_DIRECT3DVERTEXBUFFER9 *Buffer, int ASyncThread = FALSE ) ;

extern	HRESULT	GraphicsIndexBuffer_Lock_ASync						( DX_DIRECT3DINDEXBUFFER9 *Buffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags, int ASyncThread = FALSE ) ;
extern	HRESULT GraphicsIndexBuffer_Unlock_ASync					( DX_DIRECT3DINDEXBUFFER9 *Buffer, int ASyncThread = FALSE ) ;



// プラットフォーム依存
extern	int		InitializeGraphicsDevice( void ) ;										// グラフィックスデバイスの初期化
extern	int		TerminateGraphicsDevice( void ) ;										// グラフィックスデバイスの後始末

extern	int		OldGraphicsInterface_Create				( void ) ;
extern	int		OldGraphicsInterface_Release			( void ) ;
extern	void *	OldGraphicsInterface_GetObject			( void ) ;
extern	int		OldGraphicsInterface_GetVideoMemorySize	( int *TotalSize, int *FreeSize ) ;


extern	ULONG	Graphics_ObjectRelease					( void *pObject ) ;


extern	int		Graphics_Create							( void ) ;
extern	int		Graphics_LoadDLL						( void ) ;
extern	int		Graphics_FreeDLL						( void ) ;
extern	ULONG	Graphics_Release						( void ) ;
extern	UINT	Graphics_GetAdapterCount				( void ) ;
extern	long	Graphics_GetAdapterIdentifier			( DWORD Adapter, DWORD Flags, D_D3DADAPTER_IDENTIFIER9* pIdentifier) ;
extern	UINT	Graphics_GetAdapterModeCount			( DWORD Adapter, D_D3DFORMAT Format) ;
extern	long	Graphics_EnumAdapterModes				( DWORD Adapter, D_D3DFORMAT Format, DWORD Mode,D_D3DDISPLAYMODE* pMode) ;
extern	long	Graphics_GetAdapterDisplayMode			( DWORD Adapter, D_D3DDISPLAYMODE* pMode) ;
extern	long	Graphics_CheckDeviceFormat				( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT AdapterFormat, DWORD Usage, D_D3DRESOURCETYPE RType, D_D3DFORMAT CheckFormat) ;
extern	long	Graphics_CheckDeviceMultiSampleType		( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT SurfaceFormat, BOOL Windowed, D_D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) ;
extern	long	Graphics_CheckDepthStencilMatch			( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT AdapterFormat, D_D3DFORMAT RenderTargetFormat, D_D3DFORMAT DepthStencilFormat) ;
extern	long	Graphics_GetDeviceCaps					( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DCAPS9* pCaps) ;
extern	int		Graphics_CreateDevice					( void ) ;
extern	int		Graphics_CheckMultiSampleParam			( D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE *Samples, DWORD *Quality, int SamplesFailedBreak ) ;	// マルチサンプルレンダリングのサンプル数とクオリティをチェック
extern	int		Graphics_IsValid						( void ) ;
extern	int		Graphics_IsExObject						( void ) ;


extern	int		GraphicsDevice_Release					( void ) ;
extern	UINT	GraphicsDevice_GetAvailableTextureMem	( void ) ;
extern	long	GraphicsDevice_GetDeviceCaps			( D_D3DCAPS9* pCaps) ;
extern	long	GraphicsDevice_GetBackBuffer			( UINT iSwapChain, UINT iBackBuffer, D_D3DBACKBUFFER_TYPE Type, DX_DIRECT3DSURFACE9** ppBackBuffer ) ;
extern	long	GraphicsDevice_SetDialogBoxMode			( BOOL bEnableDialogs) ;
extern	long	GraphicsDevice_CreateTexture			( UINT Width, UINT Height, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DTEXTURE9** ppTexture, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_CreateVolumeTexture		( UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DVOLUMETEXTURE9** ppVolumeTexture, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_CreateCubeTexture		( UINT EdgeLength, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DCUBETEXTURE9** ppCubeTexture, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_CreateVertexBuffer		( UINT Length, DWORD Usage, DWORD FVF, D_D3DPOOL Pool, DX_DIRECT3DVERTEXBUFFER9** ppVertexBuffer, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_CreateIndexBuffer		( UINT Length, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DINDEXBUFFER9** ppIndexBuffer, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_CreateRenderTarget		( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_CreateDepthStencilSurface( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_UpdateSurface			( DX_DIRECT3DSURFACE9* pSourceSurface, CONST RECT* pSourceRect, DX_DIRECT3DSURFACE9* pDestinationSurface, CONST POINT* pDestPoint) ;
extern	long	GraphicsDevice_UpdateTexture			( DX_DIRECT3DBASETEXTURE9* pSourceTexture, DX_DIRECT3DBASETEXTURE9* pDestinationTexture) ;
extern	long	GraphicsDevice_GetRenderTargetData		( DX_DIRECT3DSURFACE9* pRenderTarget, DX_DIRECT3DSURFACE9* pDestSurface) ;
extern	long	GraphicsDevice_StretchRect				( DX_DIRECT3DSURFACE9* pSourceSurface, CONST RECT* pSourceRect, DX_DIRECT3DSURFACE9* pDestSurface, CONST RECT* pDestRect, D_D3DTEXTUREFILTERTYPE Filter) ;
extern	long	GraphicsDevice_ColorFill				( DX_DIRECT3DSURFACE9* pSurface, CONST RECT* pRect, D_D3DCOLOR color) ;
extern	long	GraphicsDevice_CreateOffscreenPlainSurface( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DPOOL Pool, DX_DIRECT3DSURFACE9** ppSurface, HANDLE* pSharedHandle) ;
extern	long	GraphicsDevice_SetRenderTarget			( DWORD RenderTargetIndex, DX_DIRECT3DSURFACE9* pRenderTarget) ;
extern	long	GraphicsDevice_SetDepthStencilSurface	( DX_DIRECT3DSURFACE9* pNewZStencil) ;
extern	long	GraphicsDevice_GetDepthStencilSurface	( DX_DIRECT3DSURFACE9** ppZStencilSurface) ;
extern	long	GraphicsDevice_BeginScene				( void ) ;
extern	long	GraphicsDevice_EndScene					( void ) ;
extern	long	GraphicsDevice_Clear					( DWORD Count, CONST D_D3DRECT* pRects, DWORD Flags, D_D3DCOLOR Color, float Z, DWORD Stencil) ;
extern	long	GraphicsDevice_SetTransform				( D_D3DTRANSFORMSTATETYPE State, CONST D_D3DMATRIX* pMatrix) ;
extern	long	GraphicsDevice_SetViewport				( CONST D_D3DVIEWPORT9* pViewport) ;
extern	long	GraphicsDevice_SetMaterial				( CONST D_D3DMATERIAL9* pMaterial) ;
extern	long	GraphicsDevice_GetMaterial				( D_D3DMATERIAL9* pMaterial) ;
extern	long	GraphicsDevice_SetLight					( DWORD Index, CONST D_D3DLIGHT9* Param ) ;
extern	long	GraphicsDevice_LightEnable				( DWORD Index, BOOL Enable) ;
extern	long	GraphicsDevice_SetRenderState			( D_D3DRENDERSTATETYPE State, DWORD Value ) ;
extern	long	GraphicsDevice_SetTexture				( DWORD Stage, DX_DIRECT3DBASETEXTURE9* pTexture) ;
extern	long	GraphicsDevice_SetTextureStageState		( DWORD Stage, D_D3DTEXTURESTAGESTATETYPE Type, DWORD Value) ;
extern	long	GraphicsDevice_SetSamplerState			( DWORD Sampler, D_D3DSAMPLERSTATETYPE Type, DWORD Value ) ;
extern	long	GraphicsDevice_SetScissorRect			( CONST RECT* pRect) ;
extern	long	GraphicsDevice_DrawPrimitive			( D_D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount ) ;
extern	long	GraphicsDevice_DrawIndexedPrimitive		( D_D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount ) ;
extern	long	GraphicsDevice_DrawPrimitiveUP			( D_D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride ) ;
extern	long	GraphicsDevice_DrawIndexedPrimitiveUP	( D_D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D_D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride ) ;
extern	long	GraphicsDevice_CreateVertexDeclaration	( CONST D_D3DVERTEXELEMENT9* pVertexElements, DX_DIRECT3DVERTEXDECLARATION9** ppDecl) ;
extern	long	GraphicsDevice_SetVertexDeclaration		( DX_DIRECT3DVERTEXDECLARATION9* pDecl ) ;
extern	long	GraphicsDevice_SetFVF					( DWORD FVF ) ;
extern	long	GraphicsDevice_CreateVertexShader		( CONST DWORD* pFunction, DX_DIRECT3DVERTEXSHADER9** ppShader) ;
extern	long	GraphicsDevice_SetVertexShader			( DX_DIRECT3DVERTEXSHADER9* pShader) ;
extern	long	GraphicsDevice_SetVertexShaderConstantF	( UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) ;
extern	long	GraphicsDevice_SetVertexShaderConstantI	( UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) ;
extern	long	GraphicsDevice_SetVertexShaderConstantB	( UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) ;
extern	long	GraphicsDevice_SetStreamSource			( UINT StreamNumber, DX_DIRECT3DVERTEXBUFFER9* pStreamData, UINT OffsetInBytes, UINT Stride ) ;
extern	long	GraphicsDevice_SetIndices				( DX_DIRECT3DINDEXBUFFER9* pIndexData ) ;
extern	long	GraphicsDevice_CreatePixelShader		( CONST DWORD* pFunction, DX_DIRECT3DPIXELSHADER9** ppShader) ;
extern	long	GraphicsDevice_SetPixelShader			( DX_DIRECT3DPIXELSHADER9* pShader) ;
extern	long	GraphicsDevice_SetPixelShaderConstantF	( UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) ;
extern	long	GraphicsDevice_SetPixelShaderConstantI	( UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) ;
extern	long	GraphicsDevice_SetPixelShaderConstantB	( UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) ;
extern	int		GraphicsDevice_Present					( RECT *CopyRect ) ;
extern	int		GraphicsDevice_BltRectBackScreenToWindow( HWND Window, RECT BackScreenRect, RECT WindowClientRect ) ;
extern	void	GraphicsDevice_WaitVSyncInitialize		( void ) ;
extern	void	GraphicsDevice_LocalWaitVSync			( void ) ;
extern	int		GraphicsDevice_WaitVSync				( int SyncNum ) ;
extern	int		GraphicsDevice_IsValid					( void ) ;
extern	void *	GraphicsDevice_GetObject				( void ) ;
extern	int		GraphicsDevice_IsLost					( void ) ;
extern	int		GraphicsDevice_SetupTimerPresent		( int EnableFlag ) ;


extern	long	GraphicsTexture_GetSurfaceLevel			( DX_DIRECT3DTEXTURE9 *pTexture, UINT Level, DX_DIRECT3DSURFACE9 ** ppSurfaceLevel) ;
extern	long	GraphicsTexture_LockRect				( DX_DIRECT3DTEXTURE9 *pTexture, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) ;
extern	long	GraphicsTexture_UnlockRect				( DX_DIRECT3DTEXTURE9 *pTexture, UINT Level) ;

extern	long	GraphicsCubeTexture_GetCubeMapSurface	( DX_DIRECT3DCUBETEXTURE9 *pCubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, DX_DIRECT3DSURFACE9** ppCubeMapSurface) ;
extern	long	GraphicsCubeTexture_LockRect			( DX_DIRECT3DCUBETEXTURE9 *pCubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) ;
extern	long	GraphicsCubeTexture_UnlockRect			( DX_DIRECT3DCUBETEXTURE9 *pCubeTexture, D_D3DCUBEMAP_FACES FaceType, UINT Level) ;

extern	long	GraphicsSurface_GetDesc					( DX_DIRECT3DSURFACE9 *pSurface, D_D3DSURFACE_DESC *pDesc) ;
extern	long	GraphicsSurface_LockRect				( DX_DIRECT3DSURFACE9 *pSurface, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) ;
extern	long	GraphicsSurface_UnlockRect				( DX_DIRECT3DSURFACE9 *pSurface ) ;


extern	long	GraphicsVertexBuffer_Lock				( DX_DIRECT3DVERTEXBUFFER9 *pVertexBuffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags ) ;
extern	long	GraphicsVertexBuffer_Unlock				( DX_DIRECT3DVERTEXBUFFER9 *pVertexBuffer ) ;


extern	long	GraphicsIndexBuffer_Lock				( DX_DIRECT3DINDEXBUFFER9 *pIndexBuffer, UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags) ;
extern	long	GraphicsIndexBuffer_Unlock				( DX_DIRECT3DINDEXBUFFER9 *pIndexBuffer ) ;


}

#endif // __DXGRAPHICSAPI_H__




