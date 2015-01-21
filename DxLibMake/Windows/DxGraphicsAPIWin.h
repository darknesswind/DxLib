// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		描画ＡＰＩプログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXGRAPHICSAPIWIN_H__
#define __DXGRAPHICSAPIWIN_H__

// Include ------------------------------------------------------------------
#include "../DxCompileConfig.h"
#include "../DxDirectX.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#define DX_DIRECTDRAWSURFACE7				class D_IDirectDrawSurface7
#define DX_DIRECT3DVERTEXSHADER9			class D_IDirect3DVertexShader9
#define DX_DIRECT3DPIXELSHADER9				class D_IDirect3DPixelShader9 
#define DX_DIRECT3DBASETEXTURE9				class D_IDirect3DBaseTexture9
#define DX_DIRECT3DTEXTURE9					class D_IDirect3DTexture9
#define DX_DIRECT3DCUBETEXTURE9				class D_IDirect3DCubeTexture9
#define	DX_DIRECT3DVOLUMETEXTURE9			class D_IDirect3DVolumeTexture9
#define DX_DIRECT3DSURFACE9					class D_IDirect3DSurface9
#define DX_DIRECT3DVERTEXDECLARATION9		class D_IDirect3DVertexDeclaration9
#define DX_DIRECT3DVERTEXBUFFER9			class D_IDirect3DVertexBuffer9
#define	DX_DIRECT3DINDEXBUFFER9				class D_IDirect3DIndexBuffer9

// 结构体定义 --------------------------------------------------------------------

struct GRAPHICSAPIINFO_WIN
{
	HINSTANCE				Direct3D9DLL ;							// Ｄｉｒｅｃｔ３Ｄ９．ＤＬＬ
	D_IDirectDraw7			*DirectDraw7Object ;					// ＤｉｒｅｃｔＤｒａｗインターフェイス
	D_IDirect3D9			*Direct3D9Object ;						// Ｄｉｒｅｃｔ３Ｄ９インターフェイス
	D_IDirect3D9Ex			*Direct3D9ExObject ;					// Ｄｉｒｅｃｔ３Ｄ９Ｅｘインターフェイス
	D_IDirect3DDevice9		*Direct3DDevice9Object ;				// Ｄｉｒｅｃｔ３ＤＤｅｖｉｃｅ９インターフェイス
	D_IDirect3DDevice9Ex	*Direct3DDevice9ExObject ;				// Ｄｉｒｅｃｔ３ＤＤｅｖｉｃｅ９Ｅｘインターフェイス
	D_IDirect3DSwapChain9	*Direct3DSwapChain9Object ;				// Ｄｉｒｅｃｔ３ＤＳｗａｐＣｈａｉｎ９インターフェイス
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

}

#endif // __DXGRAPHICSAPIWIN_H__




