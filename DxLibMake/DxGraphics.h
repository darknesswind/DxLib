// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		描画プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXGRAPHICS_H__
#define __DXGRAPHICS_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxGraphicsBase.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxHandle.h"
#include "DxMemImg.h"
#include "DxGraphicsAPI.h"
#include "DxArchive_.h"
#include <stdarg.h>

#if !defined( __ANDROID )
#include "Windows/DxGraphicsWin.h"
#endif

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#define GRA2						GraphicsManage2
#define GRH							GraphicsManage2.Hard
#define GRS							GraphicsManage2.Soft

// Ｚバッファフォーマット
#define ZBUFFER_FORMAT_16BIT		(0)
#define ZBUFFER_FORMAT_24BIT		(1)
#define ZBUFFER_FORMAT_32BIT		(2)
#define ZBUFFER_FORMAT_NUM			(3)

// DrawPreparation 関数に渡すフラグ
#define DRAWPREP_TRANS				(0x00001)
#define DRAWPREP_VECTORINT			(0x00002)
#define DRAWPREP_GOURAUDSHADE		(0x00008)
#define DRAWPREP_PERSPECTIVE		(0x00010)
#define DRAWPREP_DIFFUSERGB			(0x00020)
#define DRAWPREP_DIFFUSEALPHA		(0x00040)
#define DRAWPREP_FOG				(0x00080)
#define DRAWPREP_NOBLENDSETTING		(0x00100)
#define DRAWPREP_LIGHTING			(0x00200)
#define DRAWPREP_SPECULAR			(0x00400)
#define DRAWPREP_3D					(0x00800)
#define DRAWPREP_TEXADDRESS			(0x01000)
#define DRAWPREP_NOTSHADERRESET		(0x02000)
#define DRAWPREP_CULLING			(0x04000)
#define DRAWPREP_TEXALPHACH			(0x08000)
#define DRAWPREP_TEXALPHATEST		(0x10000)
#define DRAWPREP_EDGEFONT			(0x20000)

// 頂点タイプ
#define VERTEXTYPE_NOTEX			(0)				// テクスチャを使用しない
#define VERTEXTYPE_TEX				(1)				// テクスチャを一つ使用する
#define VERTEXTYPE_BLENDTEX			(2)				// ブレンドテクスチャを使用する
#define VERTEXTYPE_NUM				(3)

// マスク定型処理
#ifdef DX_NON_MASK
#define MASK_FULLSCREEN_MASKUPDATE
#else
#define MASK_FULLSCREEN_MASKUPDATE					\
	if( MASKD.FullScreenMaskUpdateFlag )			\
	{												\
		RECT MaskRect ;								\
		MaskRect.left   = GBASE.DrawArea.left ;		\
		MaskRect.right  = GBASE.DrawArea.right ;	\
		MaskRect.top    = GBASE.DrawArea.top ;		\
		MaskRect.bottom = GBASE.DrawArea.bottom ;	\
		MASK_END( MaskRect )						\
		MASKD.FullScreenMaskUpdateFlag = FALSE ;	\
	}
#endif

// 描画範囲に入っているかどうかの判定( 0 だったら描画領域に入っていない部分がある )
#define IN_DRAWAREA_CHECK( x1, y1, x2, y2 )	\
		( (DWORD)( ( (x2) - ( GBASE.DrawArea.right  + 1 ) ) & ( GBASE.DrawArea.left - ( (x1) + 1 ) ) &					\
		           ( (y2) - ( GBASE.DrawArea.bottom + 1 ) ) & ( GBASE.DrawArea.top  - ( (y1) + 1 ) ) ) & 0x80000000 )

// 描画範囲から完全に外れているかどうかの判定( 0 じゃなかったら完全にはずれている )
#define OUT_DRAWAREA_CHECK( x1, y1, x2, y2 ) \
		( (DWORD)( ( GBASE.DrawArea.right  - ( (x1) + 1 ) ) & ( (x2) - ( GBASE.DrawArea.left + 1 ) ) &		\
		           ( GBASE.DrawArea.bottom - ( (y1) + 1 ) ) & ( (y2) - ( GBASE.DrawArea.top  + 1 ) ) ) & 0x80000000 )

#define DRAWRECT_DRAWAREA_CLIP\
		if( IN_DRAWAREA_CHECK( GRH.DrawRect.left, GRH.DrawRect.top, GRH.DrawRect.right, GRH.DrawRect.bottom ) == 0 )\
		{\
			if( GRH.DrawRect.left   < GBASE.DrawArea.left   ) GRH.DrawRect.left   = GBASE.DrawArea.left   ;\
			if( GRH.DrawRect.right  < GBASE.DrawArea.left   ) GRH.DrawRect.right  = GBASE.DrawArea.left   ;\
			if( GRH.DrawRect.left   > GBASE.DrawArea.right  ) GRH.DrawRect.left   = GBASE.DrawArea.right  ;\
			if( GRH.DrawRect.right  > GBASE.DrawArea.right  ) GRH.DrawRect.right  = GBASE.DrawArea.right  ;\
			if( GRH.DrawRect.top    < GBASE.DrawArea.top    ) GRH.DrawRect.top    = GBASE.DrawArea.top    ;\
			if( GRH.DrawRect.bottom < GBASE.DrawArea.top    ) GRH.DrawRect.bottom = GBASE.DrawArea.top    ;\
			if( GRH.DrawRect.top    > GBASE.DrawArea.bottom ) GRH.DrawRect.top    = GBASE.DrawArea.bottom ;\
			if( GRH.DrawRect.bottom > GBASE.DrawArea.bottom ) GRH.DrawRect.bottom = GBASE.DrawArea.bottom ;\
		}

#ifdef DX_NON_MASK
#define MASK_BEGIN( RECT )
#else
#define MASK_BEGIN( RECT )		if( MASKD.MaskValidFlag ) MaskDrawBeginFunction( (RECT) ) ;
#endif

#ifdef DX_NON_MASK
#define MASK_END( RECT )
#else
#define MASK_END( RECT )		if( MASKD.MaskValidFlag ) MaskDrawAfterFunction( (RECT) ) ;
#endif

// 描画定型処理
#ifdef DX_NON_2DDRAW
	#ifdef DX_NON_MASK
		#define DRAW_DEF( FUNC3D, FUNC2D, SETRECT, RET, USE3DFLAG )\
			{\
				RECT Rect ;\
				if( ( GBASE.BlendMode == DX_BLENDMODE_SUB && GRH.ValidDestBlendOp == FALSE ) || GBASE.AlphaChDrawMode )\
				{\
					SETRECT\
					Rect = GRH.DrawRect ;\
				}\
				if( GBASE.BlendMode == DX_BLENDMODE_SUB && GRH.ValidDestBlendOp == FALSE )\
				{\
					BlendModeSub_Pre( &Rect ) ;\
					RET = (FUNC3D) ;\
					BlendModeSub_Post( &Rect ) ;\
				}\
				else\
				{\
					RET = (FUNC3D) ;\
				}\
			}
	#else // DX_NON_MASK
		#define DRAW_DEF( FUNC3D, FUNC2D, SETRECT, RET, USE3DFLAG )\
			{\
				RECT Rect ;\
				bool UseSubMode = GBASE.BlendMode == DX_BLENDMODE_SUB && GRH.ValidDestBlendOp == FALSE ;\
				if( GBASE.AlphaChDrawMode || UseSubMode || MASKD.MaskValidFlag )\
				{\
					SETRECT\
					Rect = GRH.DrawRect ;\
				}\
				if( UseSubMode || MASKD.MaskValidFlag )\
				{\
					MASK_BEGIN( Rect ) ;\
					if( UseSubMode )\
					{\
						BlendModeSub_Pre( &Rect ) ;\
						RET = (FUNC3D) ;\
						BlendModeSub_Post( &Rect ) ;\
					}\
					else\
					{\
						RET = (FUNC3D) ;\
					}\
					\
					MASK_END( Rect ) ;\
				}\
				else\
				{\
					RET = (FUNC3D) ;\
				}\
			}
	#endif // DX_NON_MASK
#else // DX_NON_2DDRAW
	#ifdef DX_NON_MASK
		#define DRAW_DEF( FUNC3D, FUNC2D, SETRECT, RET, USE3DFLAG )\
			{\
				RECT Rect ;\
				if( ( GBASE.BlendMode == DX_BLENDMODE_SUB && GRH.ValidDestBlendOp == FALSE && (USE3DFLAG) ) || GBASE.AlphaChDrawMode )\
				{\
					SETRECT\
					Rect = GRH.DrawRect ;\
				}\
				if( GBASE.BlendMode == DX_BLENDMODE_SUB && GRH.ValidDestBlendOp == FALSE && (USE3DFLAG) )\
				{\
					BlendModeSub_Pre( &Rect ) ;\
					RET = (FUNC3D) ;\
					BlendModeSub_Post( &Rect ) ;\
				}\
				else\
				{\
					RET = (USE3DFLAG) ? (FUNC3D) : (FUNC2D) ;\
				}\
			}
	#else // DX_NON_MASK
		#define DRAW_DEF( FUNC3D, FUNC2D, SETRECT, RET, USE3DFLAG )\
			{\
				RECT Rect ;\
				bool UseSubMode = GBASE.BlendMode == DX_BLENDMODE_SUB && GRH.ValidDestBlendOp == FALSE && (USE3DFLAG) ;\
				if( GBASE.AlphaChDrawMode || UseSubMode || MASKD.MaskValidFlag )\
				{\
					SETRECT\
					Rect = GRH.DrawRect ;\
				}\
				if( UseSubMode || MASKD.MaskValidFlag )\
				{\
					MASK_BEGIN( Rect ) ;\
					if( UseSubMode )\
					{\
						BlendModeSub_Pre( &Rect ) ;\
						RET = (FUNC3D) ;\
						BlendModeSub_Post( &Rect ) ;\
					}\
					else\
					{\
						RET = (USE3DFLAG) ? (FUNC3D) : (FUNC2D) ;\
					}\
					\
					MASK_END( Rect ) ;\
				}\
				else\
				{\
					RET = (USE3DFLAG) ? (FUNC3D) : (FUNC2D) ;\
				}\
			}
	#endif // DX_NON_MASK
#endif // DX_NON_2DDRAW


// 頂点フォーマット
#define VERTEXFVF_2D_USER			( D_D3DFVF_XYZRHW | D_D3DFVF_DIFFUSE | D_D3DFVF_TEX1 )										// トランスフォーム済み頂点フォーマット
#define VERTEXFVF_NOTEX_2D			( D_D3DFVF_XYZRHW | D_D3DFVF_DIFFUSE | D_D3DFVF_TEX1 )										// ライン、ボックス描画用頂点フォーマット
#define VERTEXFVF_2D				( D_D3DFVF_XYZRHW | D_D3DFVF_DIFFUSE | D_D3DFVF_TEX2 )										// トランスフォーム済み頂点フォーマット
#define VERTEXFVF_BLENDTEX_2D		( D_D3DFVF_XYZRHW | D_D3DFVF_DIFFUSE | D_D3DFVF_SPECULAR | D_D3DFVF_TEX3 )					// αブレンドテクスチャ付きトランスフォーム済み頂点フォーマット
#define VERTEXFVF_SHADER_2D			( D_D3DFVF_XYZRHW | D_D3DFVF_DIFFUSE | D_D3DFVF_SPECULAR | D_D3DFVF_TEX2 )					// シェーダー描画用頂点フォーマット
#define VERTEXFVF_NOTEX_3D			( D_D3DFVF_XYZ | D_D3DFVF_DIFFUSE /*| D_D3DFVF_SPECULAR*/ )									// ライン、ボックス描画用頂点フォーマット、３Ｄ用( 旧バージョン用 )
#define VERTEXFVF_3D				( D_D3DFVF_XYZ | D_D3DFVF_DIFFUSE /*| D_D3DFVF_SPECULAR*/ | D_D3DFVF_TEX1 )					// グラフィックス描画用頂点フォーマット、３Ｄ用( 旧バージョン用 )
#define VERTEXFVF_3D_LIGHT			( D_D3DFVF_XYZ | D_D3DFVF_NORMAL | D_D3DFVF_DIFFUSE | D_D3DFVF_SPECULAR | D_D3DFVF_TEX2 )	// グラフィックス描画用頂点フォーマット
#define VERTEXFVF_SHADER_3D			( D_D3DFVF_XYZ | D_D3DFVF_NORMAL | D_D3DFVF_DIFFUSE | D_D3DFVF_SPECULAR | D_D3DFVF_TEX2 )	// シェーダー描画用頂点フォーマット

// 頂点バッファのサイズ
#define D3DDEV_NOTEX_VERTBUFFERSIZE		(63 * 1024)		// 63KB
#define D3DDEV_TEX_VERTBUFFERSIZE		(64 * 1024)		// 64KB
#define D3DDEV_BLENDTEX_VERTBUFFERSIZE	(64 * 1024)		// 64KB
#define D3DDEV_NOTEX_3D_VERTBUFFERSIZE	(64 * 1024)		// 64KB
#define D3DDEV_TEX_3D_VERTBUFFERSIZE	(64 * 1024)		// 64KB
#define D3DDEV_NOTEX_VERTMAXNUM			(D3DDEV_NOTEX_VERTBUFFERSIZE     / sizeof( VERTEX_NOTEX_2D    ))
#define D3DDEV_TEX_VERTMAXNUM			(D3DDEV_TEX_VERTBUFFERSIZE       / sizeof( VERTEX_2D          ))
#define D3DDEV_BLENDTEX_VERTMAXNUM		(D3DDEV_BLENDTEX_VERTBUFFERSIZE  / sizeof( VERTEX_BLENDTEX_2D ))
#define D3DDEV_NOTEX_3D_VERTMAXNUM		(D3DDEV_NOTEX_3D_VERTBUFFERSIZE  / sizeof( VERTEX_NOTEX_3D    ))
#define D3DDEV_TEX_3D_VERTMAXNUM		(D3DDEV_TEX_3D_VERTBUFFERSIZE    / sizeof( VERTEX_3D          ))

// 頂点インデックスバッファのサイズ
#define D3DDEV_INDEX_NUM			( D3DDEV_NOTEX_VERTMAXNUM / 4 * 6 )

// フィルタータイプ定義
#define DX_D3DTEXF_NONE				D_D3DTEXF_NONE
#define DX_D3DTEXF_POINT			D_D3DTEXF_POINT
#define DX_D3DTEXF_LINEAR			D_D3DTEXF_LINEAR
#define DX_D3DTEXF_ANISOTROPIC		D_D3DTEXF_ANISOTROPIC
#define DX_D3DTEXF_PYRAMIDALQUAD	D_D3DTEXF_PYRAMIDALQUAD
#define DX_D3DTEXF_GAUSSIANQUAD		D_D3DTEXF_GAUSSIANQUAD

// フィルターターゲット定義
#define DX_D3DSAMP_MAGFILTER		D_D3DTSS_MAGFILTER
#define DX_D3DSAMP_MINFILTER		D_D3DTSS_MINFILTER
#define DX_D3DSAMP_MIPFILTER		D_D3DTSS_MIPFILTER

// シェーダー定数セット
#define DX_SHADERCONSTANTSET_LIB		0
#define DX_SHADERCONSTANTSET_LIB_SUB	1
#define DX_SHADERCONSTANTSET_MV1		2
#define DX_SHADERCONSTANTSET_USER		3
#define DX_SHADERCONSTANTSET_NUM		4

// シェーダー定数セットマスク
#define DX_SHADERCONSTANTSET_MASK_LIB		0x0001
#define DX_SHADERCONSTANTSET_MASK_LIB_SUB	0x0002
#define DX_SHADERCONSTANTSET_MASK_MV1		0x0004
#define DX_SHADERCONSTANTSET_MASK_USER		0x0008

// シェーダー定数タイプ
#define DX_SHADERCONSTANTTYPE_VS_FLOAT		0
#define DX_SHADERCONSTANTTYPE_VS_INT		1
#define DX_SHADERCONSTANTTYPE_VS_BOOL		2
#define DX_SHADERCONSTANTTYPE_PS_FLOAT		3
#define DX_SHADERCONSTANTTYPE_PS_INT		4
#define DX_SHADERCONSTANTTYPE_PS_BOOL		5
#define DX_SHADERCONSTANTTYPE_NUM			6

// ピクセル単位ライティングタイプの頂点シェーダー識別コード作成用マクロ
#define PIXELLIGHTING_VERTEXSHADER_SHADOWMAP( use )			( ( use )  * 3 * 2 * 4 )
#define PIXELLIGHTING_VERTEXSHADER_SKINMESH( use )			( ( use )  * 2 * 4 )
#define PIXELLIGHTING_VERTEXSHADER_BUMPMAP( use )			( ( use )  * 4 )
#define PIXELLIGHTING_VERTEXSHADER_FOGMODE( mode )			( ( mode ) )

// ピクセル単位ライティングタイプの頂点シェーダー識別コードから各要素を取得するためのマクロ
#define PIXELLIGHTING_VERTEXSHADER_GET_SHADOWMAP( index )	( ( index ) / ( 3 * 2 * 4 ) % 2 )
#define PIXELLIGHTING_VERTEXSHADER_GET_SKINMESH( index )	( ( index ) / ( 2 * 4 ) % 3 )
#define PIXELLIGHTING_VERTEXSHADER_GET_BUMPMAP( index )		( ( index ) / ( 4 ) % 2 )
#define PIXELLIGHTING_VERTEXSHADER_GET_FOGMODE( index )		( ( index ) % 4 )

// ピクセル単位ライティングタイプのピクセルシェーダータイプ
#define PIXELLIGHTING_PIXELSHADER_TYPE_NORMAL				0	
#define PIXELLIGHTING_PIXELSHADER_TYPE_TOON					1

// ピクセル単位ライティングタイプのピクセルシェーダー識別コード作成用マクロ
#define PIXELLIGHTING_PIXELSHADER_TYPE( type )				( ( type  ) * 2 * 5 * 2 * 3 * 2 * 2 * 2 * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_SHADOWMAP( use )			( ( use   ) * 5 * 2 * 3 * 2 * 2 * 2 * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_MULTITEX( type )			( ( type  ) * 2 * 3 * 2 * 2 * 2 * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_TOONTYPE( type )			( ( type  ) * 3 * 2 * 2 * 2 * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_TOONSPHEREOP( type )		( ( type  ) * 2 * 2 * 2 * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_TOONDIFBLDOP( type )		( ( type  ) * 2 * 2 * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_TOONSPCBLDOP( type )		( ( type  ) * 2 * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_SPECULARMAP( use )		( ( use   ) * 2 * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_BUMPMAP( use )			( ( use   ) * 84 * 2 )
#define PIXELLIGHTING_PIXELSHADER_LIGHTINDEX( index )		( ( index ) * 2 )
#define PIXELLIGHTING_PIXELSHADER_SPECULAR( use )			( ( use   ) )

// ピクセル単位ライティングタイプのピクセルシェーダー識別コードから各要素を取得するためのマクロ
#define PIXELLIGHTING_PIXELSHADER_GET_TYPE( index )			( ( index ) / ( 2 * 5 * 2 * 3 * 2 * 2 * 2 * 2 * 84 * 2 ) % 2 )
#define PIXELLIGHTING_PIXELSHADER_GET_SHADOWMAP( index )	( ( index ) / ( 5 * 2 * 3 * 2 * 2 * 2 * 2 * 84 * 2 ) % 2 )
#define PIXELLIGHTING_PIXELSHADER_GET_MULTITEX( index )		( ( index ) / ( 2 * 3 * 2 * 2 * 2 * 2 * 84 * 2 ) % 5 )
#define PIXELLIGHTING_PIXELSHADER_GET_TOONTYPE( index )		( ( index ) / ( 3 * 2 * 2 * 2 * 2 * 84 * 2 ) % 2 )
#define PIXELLIGHTING_PIXELSHADER_GET_TOONSPHEREOP( index )	( ( index ) / ( 2 * 2 * 2 * 2 * 84 * 2 ) % 3 )
#define PIXELLIGHTING_PIXELSHADER_GET_TOONDIFBLDOP( index )	( ( index ) / ( 2 * 2 * 2 * 84 * 2 ) % 2 )
#define PIXELLIGHTING_PIXELSHADER_GET_TOONSPCBLDOP( index )	( ( index ) / ( 2 * 2 * 84 * 2 ) % 2 )
#define PIXELLIGHTING_PIXELSHADER_GET_SPECULARMAP( index )	( ( index ) / ( 2 * 84 * 2 ) % 2 )
#define PIXELLIGHTING_PIXELSHADER_GET_BUMPMAP( index )		( ( index ) / ( 84 * 2 ) % 2 )
#define PIXELLIGHTING_PIXELSHADER_GET_LIGHTINDEX( index )	( ( index ) / ( 2 ) % 84 )
#define PIXELLIGHTING_PIXELSHADER_GET_SPECULAR( index )		( ( index ) % 2 )





// 頂点シェーダータイプ
#define VERTEXSHADER_TYPE_NORMAL					0
#define VERTEXSHADER_TYPE_DRAW_SHADOWMAP			1
#define VERTEXSHADER_TYPE_TOON_OUTLINE				2

// 頂点シェーダー識別コード作成用マクロ
#define VERTEXSHADER_TYPE( type )					( ( type  ) * 2 * 2 * 3 * 2 * 4 * 20 * 2 )
#define VERTEXSHADER_SHADERMODEL( ver )				( ( ver   ) * 2 * 3 * 2 * 4 * 20 * 2 )
#define VERTEXSHADER_SHADOWMAP( use )				( ( use   ) * 3 * 2 * 4 * 20 * 2 )
#define VERTEXSHADER_MESHTYPE( type )				( ( type  ) * 2 * 4 * 20 * 2 )
#define VERTEXSHADER_BUMPMAP( use )					( ( use   ) * 4 * 20 * 2 )
#define VERTEXSHADER_FOGMODE( mode )				( ( mode  ) * 20 * 2 )
#define VERTEXSHADER_LIGHTINDEX( index )			( ( index ) * 2 )
#define VERTEXSHADER_SPECULAR( use )				( ( use   ) )

// 頂点シェーダー識別コードから各要素を取得するためのマクロ
#define VERTEXSHADER_GET_TYPE( index )				( ( index ) / ( 2 * 2 * 3 * 2 * 4 * 20 * 2 ) % 3 )
#define VERTEXSHADER_GET_SHADERMODEL( index )		( ( index ) / ( 2 * 3 * 2 * 4 * 20 * 2 ) % 2 )
#define VERTEXSHADER_GET_SHADOWMAP( index )			( ( index ) / ( 3 * 2 * 4 * 20 * 2 ) % 2 )
#define VERTEXSHADER_GET_MESHTYPE( index )			( ( index ) / ( 2 * 4 * 20 * 2 ) % 3 )
#define VERTEXSHADER_GET_BUMPMAP( index )			( ( index ) / ( 4 * 20 * 2 ) % 2 )
#define VERTEXSHADER_GET_FOGMODE( index )			( ( index ) / ( 20 * 2 ) % 4 )
#define VERTEXSHADER_GET_LIGHTINDEX( index )		( ( index ) / ( 2 ) % 20 )
#define VERTEXSHADER_GET_SPECULAR( index )			( ( index ) % 2 )

// ピクセルシェーダータイプ
#define PIXELSHADER_TYPE_NORMAL						0
#define PIXELSHADER_TYPE_DRAW_SHADOWMAP				1
#define PIXELSHADER_TYPE_TOON_OUTLINE				2

// ピクセルシェーダー識別コード作成用マクロ
#define PIXELSHADER_TYPE( type )					( ( type  ) * 2 * 2 * 5 * 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_SHADERMODEL( ver )				( ( ver   ) * 2 * 5 * 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_SHADOWMAP( use )				( ( use   ) * 5 * 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_MULTITEX( type )				( ( type  ) * 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_TOON( use )						( ( use   ) * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_TOONTYPE( type )				( ( type  ) * 3 * 2 * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_TOONSPHEREOP( type )			( ( type  ) * 2 * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_TOONDIFBLDOP( type )			( ( type  ) * 2 * 2 * 2 * 10 * 2 )
#define PIXELSHADER_TOONSPCBLDOP( type )			( ( type  ) * 2 * 2 * 10 * 2 )
#define PIXELSHADER_SPECULARMAP( use )				( ( use   ) * 2 * 10 * 2 )
#define PIXELSHADER_BUMPMAP( use )					( ( use   ) * 10 * 2 )
#define PIXELSHADER_LIGHTINDEX( index )				( ( index ) * 2 )
#define PIXELSHADER_SPECULAR( use )					( ( use ) )

// ピクセルシェーダー識別コードから各要素を取得するためのマクロ
#define PIXELSHADER_GET_TYPE( index )				( ( index ) / ( 2 * 2 * 5 * 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 ) % 3 )
#define PIXELSHADER_GET_SHADERMODEL( index )		( ( index ) / ( 2 * 5 * 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 ) % 2 )
#define PIXELSHADER_GET_SHADOWMAP( index )			( ( index ) / ( 5 * 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 ) % 2 )
#define PIXELSHADER_GET_MULTITEX( index )			( ( index ) / ( 2 * 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 ) % 5 )
#define PIXELSHADER_GET_TOON( index )				( ( index ) / ( 2 * 3 * 2 * 2 * 2 * 2 * 10 * 2 ) % 2 )
#define PIXELSHADER_GET_TOONTYPE( index )			( ( index ) / ( 3 * 2 * 2 * 2 * 2 * 10 * 2 ) % 2 )
#define PIXELSHADER_GET_TOONSPHEREOP( index )		( ( index ) / ( 2 * 2 * 2 * 2 * 10 * 2 ) % 3 )
#define PIXELSHADER_GET_TOONDIFBLDOP( index )		( ( index ) / ( 2 * 2 * 2 * 10 * 2 ) % 2 )
#define PIXELSHADER_GET_TOONSPCBLDOP( index )		( ( index ) / ( 2 * 2 * 10 * 2 ) % 2 )
#define PIXELSHADER_GET_SPECULARMAP( index )		( ( index ) / ( 2 * 10 * 2 ) % 2 )
#define PIXELSHADER_GET_BUMPMAP( index )			( ( index ) / ( 10 * 2 ) % 2 )
#define PIXELSHADER_GET_LIGHTINDEX( index )			( ( index ) / ( 2 ) % 10 )
#define PIXELSHADER_GET_SPECULAR( index )			( ( index ) % 2 )

/*
// 頂点シェーダー配列インデックス作成用マクロ
#define VS_MULTITEX( use )					( ( use )  * 2 * 2 * 3 * 2 * 2 * 4 * 20 * 2 )
#define VS_TOONOUTLINE( use )				( ( use )  * 2 * 3 * 2 * 2 * 4 * 20 * 2 )
#define VS_LIGHTUSE( use )					( ( use )  * 3 * 2 * 2 * 4 * 20 * 2 )
#define VS_SKINMESH( use )					( ( use )  * 2 * 2 * 4 * 20 * 2 )
#define VS_PHONG( use )						( ( use )  * 2 * 4 * 20 * 2 )
#define VS_BUMPMAP( use )					( ( use )  * 4 * 20 * 2 )
#define VS_FOGMODE( mode )					( ( mode ) * 20 * 2 )
#define VS_LIGHTINDEX( type )				( ( type ) * 2 )
#define VS_SPECULAR( use )					( ( use ) )

// 頂点シェーダー配列インデックスから各要素を取得するためのマクロ
#define VS_GET_MULTITEX( index )			( ( index ) / ( 2 * 2 * 3 * 2 * 2 * 4 * 20 * 2 ) % 2 )
#define VS_GET_TOONOUTLINE( index )			( ( index ) / ( 2 * 3 * 2 * 2 * 4 * 20 * 2 ) % 2 )
#define VS_GET_LIGHTUSE( index )			( ( index ) / ( 3 * 2 * 2 * 4 * 20 * 2 ) % 2 )
#define VS_GET_SKINMESH( index )			( ( index ) / ( 2 * 2 * 4 * 20 * 2 ) % 3 )
#define VS_GET_PHONG( index )				( ( index ) / ( 2 * 4 * 20 * 2 ) % 2 )
#define VS_GET_BUMPMAP( index )				( ( index ) / ( 4 * 20 * 2 ) % 2 )
#define VS_GET_FOGMODE( index )				( ( index ) / ( 20 * 2 ) % 4 )
#define VS_GET_LIGHTINDEX( index )			( ( index ) / ( 2 ) % 20 )
#define VS_GET_SPECULAR( index )			( ( index ) % 2 )

// ピクセルシェーダー配列インデックス作成用マクロ
#define PS_MULTITEX( use )					( ( use )  * 3 * 3 * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 )
#define PS_TOON( use )						( ( use )  * 3 * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 )
#define PS_TOONSPHEREOP( type )				( ( type ) * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 )
#define PS_TOONDIFBLDOP( type )				( ( type ) * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 )
#define PS_TOONSPCBLDOP( type )				( ( type ) * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 )
#define PS_LIGHTUSE( use )					( ( use )  * 2 * 2 * 2 * 3 * 3 * 3 * 2 )
#define PS_SPECULARMAP( use )				( ( use )  * 2 * 2 * 3 * 3 * 3 * 2 )
#define PS_PHONG( use )						( ( use )  * 2 * 3 * 3 * 3 * 2 )
#define PS_BUMPMAP( use )					( ( use )  * 3 * 3 * 3 * 2 )
#define PS_LIGHT0TYPE( type )				( ( type ) * 3 * 3 * 2 )
#define PS_LIGHT1TYPE( type )				( ( type ) * 3 * 2 )
#define PS_LIGHT2TYPE( type )				( ( type ) * 2 )
#define PS_SPECULAR( use )					( ( use ) )

// ピクセルシェーダー配列インデックスから各要素を取得するためのマクロ
#define PS_GET_MULTITEX( index )			( ( index ) / ( 3 * 3 * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 ) % 5 )
#define PS_GET_TOON( index )				( ( index ) / ( 3 * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 ) % 3 )
#define PS_GET_TOONSPHEREOP( index )		( ( index ) / ( 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 ) % 3 )
#define PS_GET_TOONDIFBLDOP( index )		( ( index ) / ( 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 ) % 2 )
#define PS_GET_TOONSPCBLDOP( index )		( ( index ) / ( 2 * 2 * 2 * 2 * 3 * 3 * 3 * 2 ) % 2 )
#define PS_GET_LIGHTUSE( index )			( ( index ) / ( 2 * 2 * 2 * 3 * 3 * 3 * 2 ) % 2 )
#define PS_GET_SPECULARMAP( index )			( ( index ) / ( 2 * 2 * 3 * 3 * 3 * 2 ) % 2 )
#define PS_GET_PHONG( index )				( ( index ) / ( 2 * 3 * 3 * 3 * 2 ) % 2 )
#define PS_GET_BUMPMAP( index )				( ( index ) / ( 3 * 3 * 3 * 2 ) % 2 )
#define PS_GET_LIGHT0TYPE( index )			( ( index ) / ( 3 * 3 * 2 ) % 3 )
#define PS_GET_LIGHT1TYPE( index )			( ( index ) / ( 3 * 2 ) % 3 )
#define PS_GET_LIGHT2TYPE( index )			( ( index ) / ( 2 ) % 3 )
#define PS_GET_SPECULAR( index )			( ( index ) % 2 )
*/

// フィルタータイプ定義
#define DX_D3DTEXFILTER_TYPE		D_D3DTEXTUREFILTERTYPE

// 各データへのアクセス記述簡略化用マクロ
#define GSYSTEM						DX_GraphicsSystemData

// デバイス列挙の最大数
#define MAX_DEVICE_LISTUP					(32)

// 非管理テクスチャへのデータ転送用システムメモリ配置テクスチャの数
#define SYSMEMTEXTURE_NUM					(2048)

// 非管理テクスチャへのデータ転送用システムメモリ配置サーフェスの数
#define SYSMEMSURFACE_NUM					(2048)

// 共有バッファの数
#define COMMON_BUFFER_NUM					(3)


// 结构体定义 --------------------------------------------------------------------

// 浮動小数点型色構造体
struct COLOR
{
	float					r, g, b, a ;
} ;

// ２Ｄ描画用頂点構造体(テクスチャ無し)
typedef struct tagVERTEX_NOTEX_2D
{
	VECTOR					pos ;
	float					rhw ;
	int						color ;
	float					sc_u, sc_v ;
} VERTEX_NOTEX_2D, *LPVERTEX_NOTEX_2D ; 

// 主に２Ｄ描画に使用する頂点データ型
typedef struct tagVERTEX_2D
{
	VECTOR					pos ;
	float					rhw ;
	int						color ;
	float					u, v ;
	float					sc_u, sc_v ;
} VERTEX_2D, *LPVERTEX_2D ; 

// ブレンド画像付き２Ｄ画像描画用頂点構造体
struct VERTEX_BLENDTEX_2D
{
	VECTOR					pos ;
	float					rhw ;
	int						color ;
	unsigned int			specular ;
	float					u1, v1 ;
	float					u2, v2 ;
	float					sc_u, sc_v ;
} ;

// マテリアル構造体
struct MATERIAL
{
	COLOR					Diffuse ;		// ディフューズ色
	COLOR					Ambient ;		// アンビエント色
	COLOR					Specular ;		// スペキュラー色
	COLOR					Emissive ;		// エミッション色
	float					Power ;			// パワー
} ;

// ライト構造体
struct LIGHT
{
	int						Type ;
	COLOR					Diffuse ;
	COLOR					Specular ;
	COLOR					Ambient ;
	VECTOR					Position ;
	VECTOR					Direction ;
	float					Range ;
	float					Falloff ;
	float					Attenuation0 ;
	float					Attenuation1 ;
	float					Attenuation2 ;
	float					Theta ;
	float					Phi ;
} ;


// テクスチャステージの情報
struct DIRECT3DTEXTURESTAGEINFO
{
	void					*Texture ;							// ステージにセットされているテクスチャサーフェス
	int						TextureCoordIndex ;					// 使用するテクスチャ座標インデックス
	int						ResultTempARG ;						// 出力先レジスタをテンポラリにするかどうか
	short					AlphaARG1, AlphaARG2, AlphaOP ;		// Direct3DDevice の D3DTSS_ALPHAOP と D3DTSS_ALPHAARG1 と D3DTSS_ALPHAARG2 の設定値
	short					ColorARG1, ColorARG2, ColorOP ;		// Direct3DDevice の D3DTSS_COLOROP と D3DTSS_COLORARG1 と D3DTSS_COLORARG2 の設定値
} ;

// Ｄｉｒｅｃｔ３Ｄ のブレンド設定に関る設定の情報
struct DIRECT3DBLENDINFO
{
	int						AlphaTestEnable ;					// Direct3DDevice の D3DRENDERSTATE_ALPHATESTENABLE の設定値
	int						AlphaRef ;							// Direct3DDevice の D3DRENDERSTATE_ALPHAREF の設定値
	int						AlphaFunc ;							// Direct3DDevice の D3DRENDERSTATE_ALPHAFUNC の設定値
	int						AlphaBlendEnable ;					// Direct3DDevice の D3DRENDERSTATE_ALPHABLENDENABLE の設定値
	int						SeparateAlphaBlendEnable ;			// Direct3DDevice の D3DRENDERSTATE_SEPARATEALPHABLENDENABLE の設定値

	DWORD					FactorColor ;						// Direct3DDevice の D3DRENDERSTATE_TEXTUREFACTOR の設定値
	int						SrcBlend, DestBlend ;				// Direct3DDevice の D3DRENDERSTATE_DESTBLEND と D3DRENDERSTATE_SRCBLEND の設定値
	int						BlendOp ;							// Direct3DDevice の D3DRENDERSTATE_BLENDOP の設定値
	int						SrcBlendAlpha, DestBlendAlpha ;		// Direct3DDevice の D3DRENDERSTATE_DESTBLENDALPHA と D3DRENDERSTATE_SRCBLENDALPHA の設定値
	int						BlendOpAlpha ;						// Direct3DDevice の D3DRENDERSTATE_BLENDOPALPHA の設定値
	int						UseTextureStageNum ;				// 使用しているテクスチャステージの数
	DIRECT3DTEXTURESTAGEINFO TextureStageInfo[USE_TEXTURESTAGE_NUM] ;	// Direct3DDevice のテクスチャステージ設定値( Direct3DDevice にこれから適応する予定の設定値 )
} ;


// ソフトウエアレンダリング用オリジナル画像情報
struct IMAGEDATA2_ORIG_SOFT
{
	MEMIMG					MemImg ;						// MEMIMG
	MEMIMG					*ZBuffer ;						// ZBuffer
} ;

// 非管理テクスチャへのデータ転送用システムメモリ配置テクスチャの情報
struct IMAGEDATA2_SYSMEMTEXTURE
{
	unsigned char			InitializeFlag ;				// 使用可能な状態かどうかのフラグ
	unsigned char			UseFlag ;						// 使用中かどうかのフラグ
	unsigned char			CubeMap ;						// キューブマップかどうか( 1:キューブマップテクスチャ  0:通常テクスチャ )
	unsigned char			MipMapCount ;					// ミップマップの数
	short					Width ;							// 幅
	short					Height ;						// 高さ
	D_D3DFORMAT				Format ;						// フォーマット
	unsigned int			UseCount ;						// 使用回数
	int						UseTime ;						// 最後に使用した時間
	DX_DIRECT3DBASETEXTURE9	*MemTexture ;					// システムメモリテクスチャ
} ;

// 非管理テクスチャへのデータ転送用システムメモリ配置サーフェスの情報
struct IMAGEDATA2_SYSMEMSURFACE
{
	unsigned char			InitializeFlag ;				// 使用可能な状態かどうかのフラグ
	unsigned char			UseFlag ;						// 使用中かどうかのフラグ
	short					Width ;							// 幅
	short					Height ;						// 高さ
	D_D3DFORMAT				Format ;						// フォーマット
	unsigned int			UseCount ;						// 使用回数
	int						UseTime ;						// 最後に使用した時間
	DX_DIRECT3DSURFACE9		*MemSurface ;					// システムメモリサーフェス
} ;

// ハードウエアレンダリング用オリジナル画像テクスチャ情報
struct IMAGEDATA2_ORIG_HARD_TEX
{
	union
	{
		DX_DIRECT3DCUBETEXTURE9	*CubeTexture ;				// キューブテクスチャ
		DX_DIRECT3DTEXTURE9		*Texture ;					// テクスチャ
	} ;
//	DX_DIRECT3DTEXTURE9		*MemTexture ;					// D3DPOOL_MANAGED が使用できない画像用メモリテクスチャ
	DX_DIRECT3DSURFACE9		*Surface[ CUBEMAP_SURFACE_NUM ] ;	// サーフェス
	DX_DIRECT3DSURFACE9		*RenderTargetSurface ;			// レンダリングターゲットサーフェス
	DX_DIRECT3DSURFACE9		*ZBuffer ;						// Ｚバッファ

	int						OrigPosX, OrigPosY ;			// 元画像で使用している領域の左上座標
	int						UseWidth, UseHeight ;			// 元画像で使用している領域のサイズ
	int						TexWidth, TexHeight ;			// テクスチャ自体の幅と高さ
} ;

// ハードウエアレンダリング用オリジナル画像情報
struct IMAGEDATA2_ORIG_HARD
{
	int						MipMapCount ;					// ミップマップの数
	int						TexNum ;						// テクスチャの数
	IMAGEDATA2_ORIG_HARD_TEX Tex[ 4 ] ;						// テクスチャリストへのポインタ
} ;

// オリジナル画像情報構造体
struct IMAGEDATA2_ORIG
{
	int						RefCount ;						// 参照されている数
	int						ColorFormat ;					// カラーフォーマット( DX_GRAPHICSIMAGE_FORMAT_3D_RGB16 等 )
	IMAGEFORMATDESC			FormatDesc ;					// フォーマット
	unsigned int			TransCode ;						// 透過色コード
	int						Width, Height ;					// 画像のサイズ
	int						ZBufferFlag ;					// Ｚバッファがあるかどうか( TRUE:ある  FALSE:ない )
	int						ZBufferBitDepthIndex ;			// Ｚバッファビット深度インデックス

	union
	{
		IMAGEDATA2_ORIG_SOFT	Soft ;						// ソフトウエアレンダリング用情報
		IMAGEDATA2_ORIG_HARD	Hard ;						// ハードウエアレンダリング用情報
	} ;
} ;

// ソフトウエアレンダリング版イメージデータ構造体
struct IMAGEDATA2_SOFT
{
	MEMIMG					MemImg ;						// 描画用 MEMIMG
} ;

// ハードウエアレンダリング版座標情報構造体
struct IMAGEDATA2_HARD_VERT
{
	float					x, y ;							// 位置
	float					u, v ;							// ＵＶ位置
} ;

// ハードウエアレンダリング版描画用構造体
struct IMAGEDATA2_HARD_DRAW
{
	int						DrawPosX, DrawPosY ;			// 描画時に指定される座標からの相対座標
	int						UsePosX, UsePosY ;				// テクスチャ内で使用している矩形の左上座標
	int						Width, Height ;					// テクスチャ内で使用しているサイズ

	IMAGEDATA2_HARD_VERT	Vertex[ 4 ] ;					// テクスチャの描画座標情報
	unsigned char			VertType[ 4 ] ;					// 頂点タイプ( TRUE=三角の半分より上　FALSE=下 )

	IMAGEDATA2_ORIG_HARD_TEX *Tex ;							// 使用するテクスチャへのポインタ
} ;

// ハードウエアレンダリング版イメージデータ構造体
struct IMAGEDATA2_HARD
{
	int						DrawNum ;						// 描画用情報の数
	IMAGEDATA2_HARD_DRAW	Draw[ 4 ] ;						// 描画用情報
} ;

// イメージ元データ構造体
struct IMAGEDATA_READBASE
{
	int						Type ;							// タイプ( 0:ファイル 1:メモリイメージ 2:BaseImage )

	TCHAR					*FileName ;						// ファイル名
	BASEIMAGE				*BaseImage ;					// 基本イメージデータ
	BASEIMAGE				*AlphaBaseImage ;				// αチャンネル用基本イメージデータ
	void					*MemImage ;						// メモリイメージ
	int						MemImageSize ;					// メモリイメージサイズ
	void					*AlphaMemImage ;				// αチャンネル用メモリイメージ
	int						AlphaMemImageSize ;				// αチャンネル用メモリイメージサイズ
	int						ReverseFlag ;					// 反転読み込みしたかどうか( TRUE:した  FALSE:してない )、再読み込みの際に読み込み元を反転するかどうかで使用される、BASEIMAGE に関しては読み込み元保存時に反転するためこのフラグは無視される

	int						RefCount ;						// 参照数
} ;

// ImageData结构体
struct IMAGEDATA2
{
	HANDLEINFO				HandleInfo ;					// ハンドル共通データ

	int						*LostFlag ;						// 解放時に立てるフラグのポインタ

#ifndef DX_NON_MOVIE
	int						MovieHandle ;					// 动画句柄
#endif

	IMAGEDATA_READBASE		*ReadBase ;						// 元数据信息
	int						UseBaseX, UseBaseY ;			// 元データ中で使用している範囲の左上座標

	IMAGEDATA2_ORIG			*Orig ;							// オリジナル画像情報構造体へのポインタ( オリジナル画像ではない場合は NULL )
	int						UseOrigX, UseOrigY ;			// オリジナル画像中の使用している矩形の左上座標
	int						Width, Height ;					// オリジナル画像中の使用している矩形の幅と高さ

	DWORD					*FullColorImage ;				// フルカラーイメージ

	int						LockFlag ;						// ロックしているかどうかフラグ
	BYTE					*LockImage ;					// ロック時に作成するテンポラリイメージ
	DWORD					LockImagePitch ;				// ロックイメージのピッチ

	int						DeviceLostDeleteFlag ;			// デバイスロスト時に削除するかどうかのフラグ( TRUE:デバイスロスト時に削除する  FALSE:デバイスロスト時に削除しない )

	union
	{
		IMAGEDATA2_SOFT		Soft ;							// ソフトウエアレンダリング用構造体へのポインタ
		IMAGEDATA2_HARD		Hard ;							// ハードウエアレンダリング用構造体へのポインタ
	} ;
} ;

// シェーダーハンドルで使用する情報の構造体
struct SHADERHANDLEDATA
{
	HANDLEINFO				HandleInfo ;						// ハンドル共通データ

	int						IsVertexShader ;					// 頂点シェーダーかどうか( TRUE:頂点シェーダー  FALSE:ピクセルシェーダー )
	void					*FunctionCode ;						// シェーダーを作成する際に使用した中間コードを格納したメモリ領域へのポインタ
	union
	{
		DX_DIRECT3DPIXELSHADER9		*PixelShader ;				// IDirect3DPixelShader9 のポインタ
		DX_DIRECT3DVERTEXSHADER9	*VertexShader ;				// IDirect3DVertexShader9 のポインタ
	} ;
	int						ConstantNum ;						// 定数の数
	D_D3DXSHADER_CONSTANTINFO	*ConstantInfo ;					// 定数情報配列の先頭アドレス
} ;

// 頂点バッファハンドルで使用する情報の構造体
struct VERTEXBUFFERHANDLEDATA
{
	HANDLEINFO				HandleInfo ;						// ハンドル共通データ

	int						Type ;								// 頂点データのタイプ( DX_VERTEXTYPE_NORMAL_3D 等 )
	int						UnitSize ;							// 頂点データ一つ辺りのバイト数
	int						Num ;								// 頂点の数
	void					*Buffer ;							// システムメモリに確保されたバッファ
	DX_DIRECT3DVERTEXBUFFER9	*Direct3DBuffer ;				// IDirect3DVertexBuffer9 のポインタ
} ;

// インデックスバッファハンドルで使用する情報の構造体
struct INDEXBUFFERHANDLEDATA
{
	HANDLEINFO				HandleInfo ;						// ハンドル共通データ

	int						Type ;								// インデックスデータのタイプ( DX_INDEX_TYPE_NORMAL_3D 等 )
	int						UnitSize ;							// インデックスデータ一つ辺りのバイト数
	int						Num ;								// インデックスの数
	void					*Buffer ;							// システムメモリに確保されたバッファ
	DX_DIRECT3DINDEXBUFFER9	*Direct3DBuffer ;					// IDirect3DIndexBuffer9 のポインタ
} ;

#ifndef DX_NON_FILTER

// シェーダーコードハンドル
struct SHADERCODE_HANDLE
{
	int						Gauss_PS[ 3 ] ;							// ガウスフィルタのピクセルシェーダー
	int						BrightClipPS[ 2 ][ 2 ] ;				// 明るさでクリップするフィルタのピクセルシェーダー( [ 1:一定以上クリップ  0:一定以下クリップ ][ 1:クリップ塗りつぶしあり 0:塗りつぶしなし ] )
	int						DownScalePS[ 3 ] ;						// 縮小フィルター( 0:X2 1:X4 2:X8 )
	int						HsbPS[ 3 ] ;							// ＨＳＢフィルター( 0:RGB to HSI  1:HSI to RGB  2:HSB の HLock )
	int						MonoPS ;								// モノトーンフィルター
	int						InvertPS ;								// 階調反転フィルター
	int						LevelPS ;								// レベル補正フィルター
	int						GammaTex ;								// ガンマ補正に使用する画像
	float					PrevGamma ;								// 前回のガンマフィルタの際のガンマ値
	int						TwoColorPS ;							// ２値化フィルター
	int						GradientMapPS[ 2 ] ;					// グラデーションマップフィルター
	int						PreMulAlphaPS ;							// 通常画像から乗算済みアルファ画像を作成する為のフィルター
	int						InterpAlphaPS ;							// 乗算済みアルファ画像から通常画像を作成する為のフィルター

	int						BasicBlendPS[ 14 ] ;					// 基本的なブレンドフィルター
	int						RgbaMixBasePS ;							// RGBAMixブレンド基本
	int						RgbaMixSRGBB[ 4 ] ;						// RGBAMixブレンドの A だけ BRGBA の４つ
	int						RgbaMixSRRRB[ 4 ][ 4 ] ;				// RGBAMixブレンドの SYYY BX の１６こ [ Y ][ X ]
	int						RgbaMixS[ 4 ][ 4 ][ 4 ][ 4 ] ;			// RGBAMixブレンドの S だけの組み合わせ256個[ R ][ G ][ B ][ A ]
} ;

#endif // DX_NON_FILTER


// シェーダー定数の一つの使用領域情報構造体
struct SHADERCONST_ONE_USEAREA
{
	WORD					Start ;									// 使用領域の開始番号
	WORD					EndPlusOne ;							// 使用領域の終了番号＋１
} ;

// シェーダー定数使用領域情報構造体
struct SHADERCONSTANT_USEAREA
{
	SHADERCONST_ONE_USEAREA	AreaInfo[ 256 ] ;						// 使用領域情報
	int						AreaInfoNum ;							// 使用領域情報の数
	int						TotalSize ;								// 管理する領域のサイズ
} ;

// シェーダー定数情報構造体
struct SHADERCONSTANTINFO
{
	SHADERCONSTANT_USEAREA	UseArea ;								// ユーザー用頂点シェーダー定数の使用領域情報
	int						UnitSize ;								// データ一つあたりのサイズ

	union
	{
		BYTE					Data[ 256 * 16 ] ;					// サイズ保証用データ

		FLOAT4					Float4[ 256 ] ;						// ユーザー用頂点シェーダー float 型定数
		INT4					Int4[ 16 ] ;						// ユーザー用頂点シェーダー int 型定数
		BOOL					Bool[ 16 ] ;						// ユーザー用頂点シェーダー BOOL 型定数
	} ;
} ;

// シェーダー定数情報セット構造体
struct SHADERCONSTANTINFOSET
{
	int						IsApply[ DX_SHADERCONSTANTSET_NUM ] ;			// 定数情報を適用するかどうか( TRUE:適用する  FALSE:適用しない )
	int						ApplyMask ;										// 適用マスク( DX_SHADERCONSTANTSET_MASK_LIB | DX_SHADERCONSTANTSET_MASK_LIB_SUB 等 )
	SHADERCONSTANTINFO		Info[ DX_SHADERCONSTANTTYPE_NUM ][ DX_SHADERCONSTANTSET_NUM ] ;				// 定数情報実体
	BYTE					UseMap[ DX_SHADERCONSTANTTYPE_NUM ][ DX_SHADERCONSTANTSET_NUM ][ 256 ] ;	// 定数使用マップ

	SHADERCONSTANTINFO		FixInfo[ DX_SHADERCONSTANTTYPE_NUM ] ;			// 実際にシェーダーに適用されている定数情報

	BYTE					SetMap[ DX_SHADERCONSTANTTYPE_NUM ][ 256 ] ;	// シェーダー定数の使用中セットマップ( DX_SHADERCONSTANTSET_LIB 等、0xff 未使用 )
} ;

// ユーザーの描画設定情報構造体
struct USERRENDERINFO
{
	int						SetTextureGraphHandle[ 20 ] ;			// ユーザー設定の各ステージのテクスチャ
	int						SetRenderTargetGraphHandle[ 4 ] ;		// ユーザー設定の各レンダリングターゲット

	int						SetVertexShaderHandle ;					// ユーザー設定で使用する頂点シェーダー
	int						SetPixelShaderHandle ;					// ユーザー設定で使用するピクセルシェーダー
} ;

// シャドウマップデータ構造体
struct SHADOWMAPDATA
{
	HANDLEINFO				HandleInfo ;					// ハンドル共通データ

	int						*LostFlag ;						// 解放時に立てるフラグのポインタ

	int						TexFormat_Float ;				// 浮動小数点型フォーマットかどうか( TRUE:浮動小数点型  FALSE:固定小数点型 )
	int						TexFormat_BitDepth ;			// ビット深度
	int						ColorFormat ;					// カラーフォーマット
	int						ZBufferFormat ;					// Ｚバッファフォーマット

	int						EnableDrawArea ;				// DrawAreaMinPosition と DrawAreaMaxPosition が有効かどうか
	VECTOR					DrawAreaMinPosition ;			// シャドウマップの描画の際に入れる範囲の小さい座標
	VECTOR					DrawAreaMaxPosition ;			// シャドウマップの描画の際に入れる範囲の大きい座標
	VECTOR					DrawAreaViewClipPos[ 8 ] ;		// シャドウマップに描画する範囲を囲む座標

	VECTOR					DefaultViewClipPos[ 8 ] ;		// EnableDrawArea が FALSE の際に使用するシャドウマップに描画する範囲を囲む座標

	int						BaseSizeX ;						// シャドウマップのサイズ
	int						BaseSizeY ;

	int						DrawSetupFlag ;					// 描画準備をしているかどうかのフラグ

	MATRIX					ShadowMapViewMatrix ;			// シャドウマップを描画した際のビュー行列
	MATRIX					ShadowMapProjectionMatrix ;		// シャドウマップを描画した際の射影行列

	float					AdjustDepth ;					// シャドウマップを使用した描画時の深度補正値
	int						BlurParam ;						// シャドウマップに適用するぼかしフィルターの値
	float					GradationParam ;				// シャドウマップ値のグラデーションの範囲

	int						RenderTargetScreen[ DX_RENDERTARGET_COUNT ] ;			// 元々のレンダリングターゲット
	int						RenderTargetScreenSurface[ DX_RENDERTARGET_COUNT ] ;	// 元々のレンダリングターゲットサーフェス

	int						RenderMaskUseFlag ;				// マスク画面を使用するかどうかのフラグ

	VECTOR					LightDirection ;				// シャドウマップが想定するライトの方向
	MATRIX					LightMatrix ;					// シャドウマップが想定するライトの行列

	VECTOR					RenderCameraPosition ;			// レンダリングを行う際のカメラの位置
	VECTOR					RenderCameraTarget ;			// レンダリングを行う際のカメラの注視点
	VECTOR					RenderCameraUp ;				// レンダリングを行う際のカメラのアップベクトル
	float					RenderCameraHRotate ;			// レンダリングを行う際のカメラの水平角度
	float					RenderCameraVRotate ;			// レンダリングを行う際のカメラの垂直角度
	float					RenderCameraTRotate ;			// レンダリングを行う際のカメラの捻り角度
	MATRIX					RenderCameraMatrix ;			// レンダリングを行う際のビュー行列
	float					RenderCameraScreenCenterX ;		// レンダリングを行う際のカメラの消失点
	float					RenderCameraScreenCenterY ;

	int						RenderProjectionMatrixMode ;	// レンダリングを行う際の射影行列作成モード
	float					RenderProjNear ;				// レンダリングを行う際のカメラの Nearクリップ面
	float					RenderProjFar ;					// レンダリングを行う際のカメラの Farクリップ面
	float					RenderProjDotAspect ;			// レンダリングを行う際のドットアスペクト比( 縦 / 横 )
	float					RenderProjFov ;					// レンダリングを行う際の遠近法時の視野角
	float					RenderProjSize ;				// レンダリングを行う際の正射影時のサイズ
	MATRIX					RenderProjMatrix ;				// レンダリングを行う際の射影行列

	DX_DIRECT3DTEXTURE9		*Texture ;						// テクスチャ

	DX_DIRECT3DSURFACE9		*Surface ;						// サーフェス
	DX_DIRECT3DSURFACE9		*ZBufferSurface ;				// Ｚバッファサーフェス
} ;

// ハードウエアアクセラレータを使用するグラフィックス処理で使用する情報の構造体
struct GRAPHICS_HARDDATA
{
	int						NotUseDirect3D9Ex ;						// Direct3D9Ex を使用しないかどうか
	int						UseMultiThread ;						// マルチスレッド対応フラグを使うかどうか
	int						ValidAdapterNumber ;					// UseAdapterNumber が有効かどうか( TRUE:有効  FALSE:無効 )
	int						UseAdapterNumber ;						// 使用するデバイスアダプタ番号
	int						NonUseVertexHardwareProcess ;			// 強制的に頂点演算をハードウエアで処理しないかどうか( TRUE:処理しない  FALSE:処理する )
	int						UsePixelLightingShader ;				// ピクセル単位でライティングを行うタイプのシェーダーを使用するかどうか( TRUE:使用する  FALSE:使用しない )
	int						VertexHardwareProcess ;					// 頂点演算をハードウエアで処理するかどうか( TRUE:処理する  FALSE:処理しない )
	int						ValidVertexShader ;						// 頂点シェーダーが使用可能かどうか( TRUE:使用可能  FALSE:不可能 )
	int						ValidPixelShader ;						// ピクセルシェーダーが使用可能かどうか( TRUE:使用可能  FALSE:不可能 )
	int						ValidVertexShader_SM3 ;					// Shader Model 3.0 の頂点シェーダーが使用可能かどうか( TRUE:使用可能  FALSE:不可能 )
	int						ValidPixelShader_SM3 ;					// Shader Model 3.0 のピクセルシェーダーが使用可能かどうか( TRUE:使用可能  FALSE:不可能 )
	int						DisableAeroFlag ;						// Aero を無効にするかどうかのフラグ( TRUE:無効にする  FALSE:無効にしない )
	DWORD					NativeVertexShaderVersion ;				// エミュレーション無しの頂点シェーダーのバージョン

	int						UseBaseVertexShaderIndex ;				// 使用する頂点シェーダーのライト・フォグ・フォンシェーディングの有無のみ設定した値
	int						UseBasePixelShaderIndex ;				// 使用するピクセルシェーダーのライト・フォンシェーディングの有無のみ設定した値
	int						UseBaseVertexShaderIndex_PL ;			// ピクセル単位ライティングタイプで使用する頂点シェーダーのライト・フォグ・フォンシェーディングの有無のみ設定した値
	int						UseBasePixelShaderIndex_PL ;			// ピクセル単位ライティングタイプで使用するピクセルシェーダーのライト・フォンシェーディングの有無のみ設定した値
	int						UseOnlyPixelLightingTypeCode ;			// ピクセル単位ライティングタイプのシェーダーコードを使用する指定になっているかどうか

	D_D3DMULTISAMPLE_TYPE	FSAAMultiSampleType ;					// FSAA用マルチサンプリングタイプ
	int						FSAAMultiSampleQuality ;				// FSAA用マルチサンプリングクオリティ
	int						MipMapCount ;							// 自動で作成するミップマップの数( -1:最大レベルまで作成する )
	int						FullScreenEmulation320x240 ;			// フルスクリーンモードで 320x240 を指定されているかどうか( TRUE:指定されている  FALSE:指定されていない )
	int						UseRenderTargetLock ;					// 描画先サーフェスのロックを行うかどうか( TRUE:行う  FALSE:行わない )

	int						ValidAdapterInfo ;						// アダプタの情報が有効かどうか( TRUE:有効  FALSE:無効 )
	int						AdapterInfoNum ;						// アダプタ情報の数
	D_D3DADAPTER_IDENTIFIER9	AdapterInfo[ MAX_DEVICE_LISTUP ] ;	// アダプタの情報

	D_D3DFORMAT				ScreenFormat ;										// 画面カラーフォーマット
	D_D3DFORMAT				TextureFormat[ DX_GRAPHICSIMAGE_FORMAT_3D_NUM ] ;	// テクスチャフォーマット
	D_D3DFORMAT				MaskColorFormat ;									// マスクカラーバッファ用フォーマット
	D_D3DFORMAT				MaskAlphaFormat ;									// マスクアルファチャンネル用フォーマット
	D_D3DFORMAT				ZBufferFormat[ ZBUFFER_FORMAT_NUM ] ;				// Ｚバッファフォーマット

	IMAGEDATA2_SYSMEMTEXTURE SysMemTexture[ SYSMEMTEXTURE_NUM ] ;	// 非管理テクスチャへのデータ転送用システムメモリ配置テクスチャ
	int						SysMemTextureInitNum ;					// 初期化済みのシステムメモリテクスチャの数

	IMAGEDATA2_SYSMEMSURFACE SysMemSurface[ SYSMEMSURFACE_NUM ] ;	// 非管理テクスチャへのデータ転送用システムメモリ配置サーフェス
	int						SysMemSurfaceInitNum ;					// 初期化済みのシステムメモリサーフェスの数

	int						UserMaxTextureSize ;					// ユーザー指定のテクスチャ最大サイズ
	int						NotUseDivFlag ;							// 画像分割を行わないかどうか( TRUE:行わない  FALSE:行う )
	int						UseOldDrawModiGraphCodeFlag ;			// 以前の DrawModiGraph 関数コードを使用するかどうかのフラグ

	int						FlipRunScanline[ 2 ] ;					// ScreenFlip を実行したときのスキャンライン値
	DWORD					FlipRunTime[ 2 ] ;						// ScreenFlip を実行したときの timeGetTime の値
	int						FlipSkipFlag ;							// Flipスキップ中か、フラグ( TRUE:スキップ中  FALSE:スキップ中ではない )

	int						ValidTexTempRegFlag ;					// テクスチャステージのテンポラリレジスタが使用できるかどうかのフラグ

	DX_DIRECT3DSURFACE9		*BackBufferSurface ;					// デバイスが持つバックバッファサーフェス
	DX_DIRECT3DSURFACE9		*SubBackBufferSurface ;					// ScreenCopy や GetDrawScreen を実現するために使用する描画可能サーフェス
	DX_DIRECT3DSURFACE9		*ZBufferSurface ;						// メインで使用するＺバッファ
	int						ZBufferSizeX, ZBufferSizeY ;			// Ｚバッファサーフェスのサイズ
	int						UserZBufferSizeSet ;					// 外部からＺバッファサイズの指定があったかどうか
	int						ZBufferBitDepth ;						// Ｚバッファサーフェスのビット深度
	int						UserZBufferBitDepthSet ;				// 会部からＺバッファのビット深度指定があったかどうか

	IMAGEFORMATDESC			*DrawPrepFormat ;						// 前回 DrawPreparation に入ったときの Format パラメータ
	DX_DIRECT3DTEXTURE9		*DrawPrepTexture ;						// 前回 DrawPreparation に入ったときの Texture パラメータ
	int						DrawPrepParamFlag ;						// 前回 DrawPreparation に入ったときの ParamFlag パラメータ
	int						DrawPrepAlwaysFlag ;					// 必ず DrawPreparation を行うかどうかのフラグ

	D_D3DCAPS9				DeviceCaps ;							// デバイス情報
	int						TextureSizePow2 ;						// テクスチャのサイズが２のｎ乗である必要があるかどうか
	int						TextureSizeNonPow2Conditional ;			// 条件付でテクスチャのサイズが２のｎ乗でなくても大丈夫かどうか
	int						MaxTextureSize ;						// 最大テクスチャサイズ
	int						ValidDestBlendOp ;						// D3DBLENDOP_ADD 以外が使用可能かどうか( TRUE:使用可能  FALSE:使用不可能 )
	int						WhiteTexHandle ;						// 8x8の白いテクスチャのハンドル
	int						MaxPrimitiveCount ;						// 一度に描画できるプリミティブの最大数

	DX_DIRECT3DSURFACE9		*TargetSurface[DX_RENDERTARGET_COUNT] ;	// 描画対象のサーフェス
	int						RenderTargetNum ;						// 同時に設定できるレンダリングターゲットの数

	D_D3DVIEWPORT9			Viewport ;								// ビューポート情報
	RECT					ScissorRect ;							// シザー矩形
	int						ScissorTestEnable ;						// シザー矩形の有効、無効( TRUE:有効 FALSE:無効 )

	BYTE					*VertexBufferPoint[2][3] ;				// 各頂点バッファへのポインタ( [ ３Ｄ頂点かどうか ][ 頂点タイプ ] )
	int						Use3DVertex ;							// ３Ｄ頂点を使用しているかどうか( 1:使用している  0:使用していない )
	int						VertexNum ; 							// 頂点バッファに格納されている頂点の数
	int						VertexType ;							// 頂点バッファに格納されている頂点データ( 0:テクスチャなし  1:テクスチャあり  2:ブレンドテクスチャあり )
	D_D3DPRIMITIVETYPE		PrimitiveType ;							// 頂点バッファに格納されている頂点データのプリミティブタイプ

	float					BlendTextureWidth ;						// ブレンドテクスチャの幅
	float					BlendTextureHeight ;					// ブレンドテクスチャの高さ
	float					InvBlendTextureWidth ;					// ブレンドテクスチャの幅の逆数
	float					InvBlendTextureHeight ;					// ブレンドテクスチャの高さの逆数
	DWORD					DiffuseColor ;							// ディフューズカラー

	int						GaussPass1_VS ;							// ガウスフィルタシェーダーの１パス目の頂点シェーダー
	int						GaussPass1_PS ;							// ガウスフィルタシェーダーの１パス目のピクセルシェーダー
	int						GaussPass2_VS ;							// ガウスフィルタシェーダーの２パス目の頂点シェーダー
	int						GaussPass2_PS ;							// ガウスフィルタシェーダーの２パス目のピクセルシェーダー

	BYTE					*VertexBufferNextAddress ;				// 次のデータを格納すべき頂点バッファ内のアドレス
	BYTE					NoTexVertexBuffer[     D3DDEV_NOTEX_VERTBUFFERSIZE     ] ;	// テクスチャを使用しない頂点バッファ
	BYTE					TexVertexBuffer[       D3DDEV_TEX_VERTBUFFERSIZE       ] ;	// テクスチャを使用する頂点バッファ
	BYTE					BlendTexVertexBuffer[  D3DDEV_BLENDTEX_VERTBUFFERSIZE  ] ;	// ブレンドテクスチャを使用する頂点バッファ
	BYTE					Vertex3DBuffer[	       D3DDEV_NOTEX_3D_VERTBUFFERSIZE  ] ;	// ３Ｄ用頂点バッファ

	void					*CommonBuffer[ COMMON_BUFFER_NUM ] ;	// 汎用バッファ
	DWORD					CommonBufferSize[ COMMON_BUFFER_NUM ] ;	// 汎用バッファのメモリ確保サイズ

	RECT					DrawRect ;							// 描画範囲

	DX_DIRECT3DTEXTURE9		*RenderTexture ;					// 描画時に使用するテクスチャー
	DX_DIRECT3DTEXTURE9		*BlendTexture ;						// 描画時に描画テクスチャーとブレンドするαチャンネルテクスチャー
	DX_DIRECT3DTEXTURE9		*RenderTargetTexture ;				// 描画対象となっているテクスチャー
	DX_DIRECT3DSURFACE9		*RenderTargetSurface ;				// 描画対象となっているテクスチャーのサーフェス
	DX_DIRECT3DSURFACE9		*RenderTargetCopySurface ;			// 描画対象となっているテクスチャーのコピーサーフェス
	float					RenderTargetTextureInvWidth ;		// 描画対象となっているテクスチャーの幅で1.0fを割ったもの
	float					RenderTargetTextureInvHeight ;		// 描画対象となっているテクスチャーの高さで1.0fを割ったもの
#ifndef DX_NON_MODEL
	DX_DIRECT3DVOLUMETEXTURE9	*RGBtoVMaxRGBVolumeTexture ;	// RGBカラーを輝度を最大にしたRGB値に変換するためのボリュームテクスチャ
#endif // DX_NON_MODEL

	DIRECT3DBLENDINFO		BlendInfo ;							// Direct3DDevice のブレンドに関係する設定値
	float					FactorColorPSConstantF[ 4 ] ;		// ピクセルシェーダーを使ったＤＸライブラリ標準処理用の FactorColor の値
	DIRECT3DBLENDINFO		UserBlendInfo ;						// Direct3D 管理プログラム外から設定されるブレンドに関係する設定値
	int						UserBlendInfoFlag ;					// UserBlendInfo が有効かどうかフラグ( TRUE:有効  FALSE:無効 )
	int						UserBlendInfoTextureStageIsTextureAndTextureCoordOnlyFlag ;	// UserBlendInfo.TextureStageInfo の中で有効なパラメータが Texture と TextureCoordIndex だけかどうか( TRUE:その通り  FALSE:違う )

	int						DrawMode ;							// 描画モード
	int						MaxAnisotropy ;						// 最大異方性
	int						MaxAnisotropyDim[ 8 ] ;				// 各サンプラの最大異方性
	DX_D3DTEXFILTER_TYPE	MagFilter[ 8 ] ;					// 各サンプラの拡大フィルタ
	DX_D3DTEXFILTER_TYPE	MinFilter[ 8 ] ;					// 各サンプラの縮小フィルタ
	DX_D3DTEXFILTER_TYPE	MipFilter[ 8 ] ;					// 各サンプラのミップマップフィルタ
	int						EnableZBufferFlag ;					// Ｚバッファの有効フラグ
	int						WriteZBufferFlag ;					// Ｚバッファの更新を行うか、フラグ
	int						ZBufferCmpType ;					// Ｚ値の比較モード
	int						ZBias ;								// Ｚバイアス
	int						TextureTransformUse[ 8 ] ;				// テクスチャ座標変換行列を使用するかどうか( TRUE:使用する  FALSE:使用しない )
	MATRIX					TextureTransformMatrix[ 8 ] ;			// テクスチャ座標変換行列
	int						TextureTransformMatrixDirectChange ;	// テクスチャ座標変換行列を直接変更されたかどうか( TRUE:された  FALSE:されてない )

	int						ShadowMapDrawSetupRequest ;			// シャドウマップに対する描画準備リクエスト
	int						ShadowMapDraw ;						// シャドウマップに対する描画かどうかのフラグ
	int						ShadowMapDrawHandle ;				// シャドウマップに対する描画の際の、シャドウマップハンドル
	int						NormalizeNormals ;					// 法線自動正規化を行うかどうか
	int						ShadeMode ;							// シェーディングモード
	int						FillMode ;							// フィルモード
	int						CullMode ;							// カリングモード
	int						TexAddressModeU[ USE_TEXTURESTAGE_NUM ] ;	// テクスチャアドレスモードＵ
	int						TexAddressModeV[ USE_TEXTURESTAGE_NUM ] ;	// テクスチャアドレスモードＶ
	int						TexAddressModeW[ USE_TEXTURESTAGE_NUM ] ;	// テクスチャアドレスモードＷ
	int						FogEnable ;							// フォグが有効かどうか( TRUE:有効  FALSE:無効 )
	int						FogMode ;							// フォグモード
	DWORD					FogColor ;							// フォグカラー
	float					FogStart, FogEnd ;					// フォグ開始アドレスと終了アドレス
	float					FogDensity ;						// フォグ密度
	int						PerspectiveFlag ;					// パースペクティブ補正有効フラグ

	int						LightFlag ;							// ライトを使用するかフラグ
	int						LightEnableMaxIndex ;				// 有効なライトの最大インデックス
	int						LightEnableFlag[ 256 ] ;			// ライトが有効かどうかフラグ( TRUE:有効  FALSE:無効 )
	LIGHT					LightParam[ 256 ] ;					// ライトのパラメータ

	COLOR					GlobalAmbientLightColor ;			// グローバルアンビエントライトカラー
	MATERIAL				Material ;							// デバイスにセットされているマテリアル
	int						MaterialUseVertexDiffuseColor ;		// 頂点のディフューズカラーをマテリアルのディフューズカラーとして使用するかどうか
	int						MaterialUseVertexSpecularColor ;	// 頂点のスペキュラカラーをマテリアルのスペキュラカラーとして使用するかどうか
	int						UseSpecular ;						// スペキュラを使用するかどうか

	int						UseDiffuseRGBColor ;				// ディフューズカラーのＲＧＢ値を使用するか、フラグ
	int						UseDiffuseAlphaColor ;				// ディフューズカラーのα値を使用するか、フラグ
	int						BeginSceneFlag ;					// BeginScene を実行してあるかどうか

	int						IgnoreGraphColorFlag ;				// 描画する画像の色成分を無視するかどうかのフラグ

	int						UseAlphaChDrawShader ;				// 描画先に正しいα値を書き込む為のシェーダーを使用するかどうか( TRUE:使用する  FALSE:使用しない )
	int						BlendMode ;							// ブレンドモード
	int						BlendGraphType ;					// ブレンド画像タイプ
	int						BlendGraphFadeRatio ;				// ブレンド画像のフェードパラメータ
	int						BlendGraphBorderParam ;				// ブレンド画像の境界パラメータ(０(ブレンド画像の影響０)　←　(ブレンド画像の影響少ない)　←　１２８(ブレンド画像の影響１００％)　→　(ブレンド画像の影響を超えて非描画部分が増える)　→２５５(全く描画されない) )
	int						BlendGraphBorderRange ;				// ブレンド画像の境界幅(０～２５５　狭い～広い　しかし４段階)
	int						AlphaChannelValidFlag ;				// αチャンネル有効フラグ

	int						AlphaTestMode ;						// アルファテストモード
	int						AlphaTestParam ;					// アルファテストパラメータ

	int						AlphaTestValidFlag ;				// αテスト有効フラグ( Func は必ず D_D3DCMP_GREATEREQUAL )
	int						ChangeBlendParamFlag ;				// ブレンド設定に関わる部分の変更があったか、フラグ
	int						ChangeTextureFlag ;					// テクスチャが変更されたか、フラグ
	int						BlendMaxNotDrawFlag ;				// ブレンド値が最大であることにより描画を行わないかどうか、フラグ
	int						EdgeFontDrawFlag ;					// 縁付きフォントを描画するかどうかのフラグ


	int						DrawScreenBufferLockFlag ;			// バックバッファをロックしているかどうかフラグ
	DX_DIRECT3DSURFACE9		*DrawScreenBufferLockSMSurface ;	// バックバッファをロックした際に直接ロックできない場合に使用するサーフェスのポインタ
	int						DrawScreenBufferLockSMSurfaceIndex ;// バックバッファをロックした際に直接ロックできない場合に使用するサーフェスキャッシュのインデックス
	DX_DIRECT3DSURFACE9		*DrawScreenBufferLockSurface ;		// ロックしたバックバッファ

	void					(*DeviceRestoreCallbackFunction)( void *Data ) ;	// デバイスロストから復帰したときに呼ぶ関数
	void					*DeviceRestoreCallbackData ;		// デバイスロストから復帰したときに呼ぶ関数に渡すポインタ

	void					(*DeviceLostCallbackFunction)( void *Data ) ;	// デバイスロストから復帰する前に呼ぶ関数
	void					*DeviceLostCallbackData ;			// デバイスロストから復帰する前に呼ぶ関数に渡すポインタ

	int						UseShader ;							// プログラマブルシェーダーを使用するかどうか( TRUE:使用する  FALSE:使用しない )
	int						NormalDraw_NotUseShader ;			// 通常描画にシェーダーを使用しないかどうか( TRUE:使用しない  FALSE:使用する )
#ifndef DX_NON_FILTER
	int						ValidRenderTargetInputTexture ;		// 描画先を入力テクスチャとして使用できるかどうか( TRUE:使用できる  FALSE:使用できない )
#endif // DX_NON_FILTER

	DX_DIRECT3DVERTEXSHADER9		*SetVS ;					// 現在 Direct3DDevice9 にセットしてある頂点シェーダー
	DX_DIRECT3DPIXELSHADER9			*SetPS ;					// 現在 Direct3DDevice9 にセットしてあるピクセルシェーダー
	int								NormalPS ;					// 通常描画用ピクセルシェーダーがセットされているかどうか
	DX_DIRECT3DVERTEXDECLARATION9	*SetVD ;					// 現在 Direct3DDevice9 にセットしてある頂点シェーダ宣言
	DWORD							SetFVF ;					// 現在 Direct3DDevice9 にセットしてある FVF コード
	DX_DIRECT3DVERTEXBUFFER9		*SetVB ;					// 現在 Direct3DDevice9 にセットしてある頂点バッファ
	DX_DIRECT3DINDEXBUFFER9			*SetIB ;					// 現在 Direct3DDevice9 にセットしてあるインデックスバッファ

	// 固定機能パイプライン互換のシェーダー( テクスチャなし )
	// [ 特殊効果( 0:通常描画  1:乗算描画  2:RGB反転  3:描画輝度４倍  4:乗算済みαブレンドモードの通常描画  5:乗算済みαブレンドモードのRGB反転  6:乗算済みαブレンドモードの描画輝度4倍 ) ]
	// [ αチャンネル考慮版かどうか( 0:考慮しない  1:考慮する ) ]
	// [ 乗算済みαブレンド用かどうか( 0:乗算済みαブレンド用ではない  1:乗算済みαブレンド用 ) ]
	DX_DIRECT3DPIXELSHADER9          *BaseNoneTexPixelShader[ 7 ][ 2 ] ;

	// 固定機能パイプライン互換のシェーダー( テクスチャあり )
	// [ ブレンド画像とのブレンドタイプ( 0:なし  1:DX_BLENDGRAPHTYPE_NORMAL  2:DX_BLENDGRAPHTYPE_WIPE  3:DX_BLENDGRAPHTYPE_ALPHA ) ]
	// [ 特殊効果( 0:通常描画  1:乗算描画  2:RGB反転  3:描画輝度４倍  4:乗算済みαブレンドモードの通常描画  5:乗算済みαブレンドモードのRGB反転  6:乗算済みαブレンドモードの描画輝度4倍 ) ]
	// [ テクスチャRGB無視( 0:無視しない  1:無視する ) ]
	// [ テクスチャAlpha無視( 0:無視しない  1:無視する ) ]
	// [ αチャンネル考慮版かどうか( 0:考慮しない  1:考慮する ) ]
	DX_DIRECT3DPIXELSHADER9          *BaseUseTexPixelShader[ 4 ][ 7 ][ 2 ][ 2 ][ 2 ] ;

	// マスク処理用のシェーダー
	DX_DIRECT3DPIXELSHADER9          *MaskEffectPixelShader ;

#ifndef DX_NON_MODEL
	// モデル描画用ピクセル単位ライティングタイプの頂点シェーダー( リストは配列の左から )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ 2:8ボーン内スキニングメッシュ ) ]
	// [ バンプマップの有無( 0:無し 1:有り ) ]
	// [ フォグタイプ ]
	DX_DIRECT3DVERTEXSHADER9         *MV1_PixelLighting_VertexShader[ 2 ][ 3 ][ 2 ][ 4 ] ;

	// モデル描画用ピクセル単位ライティングタイプのトゥーン用ピクセルシェーダー( リストは配列の左から )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ トゥーンレンダリングタイプ( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング Type 2 ) ]
	// [ トゥーンレンダリングのスフィアマップの有無とブレンドタイプ( 0:スフィアマップは無い   1:MV1_LAYERBLEND_TYPE_MODULATE  2:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	// [ トゥーンレンダリングのスペキュラグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	DX_DIRECT3DPIXELSHADER9         *MV1_PixelLighting_ToonPixelShader[ 2 ][ 2 ][ 3 ][ 2 ][ 2 ][ 2 ][ 2 ][ 84 ][ 2 ] ;

	// モデル描画用ピクセル単位ライティングタイプのピクセルシェーダー( リストは配列の左から )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ マルチテクスチャのブレンドモード、MV1_LAYERBLEND_TYPE_TRANSLUCENT などに +1 したもの ( 0:マルチテクスチャ無し  1:αブレンド  2:加算ブレンド  3:乗算ブレンド  4:乗算ブレンド×2 ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	DX_DIRECT3DPIXELSHADER9         *MV1_PixelLighting_PixelShader[ 2 ][ 5 ][ 2 ][ 2 ][ 84 ][ 2 ] ;





	// モデル描画用のトゥーンレンダリングの輪郭線描画用頂点シェーダー
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	// [ フォグタイプ ]
	DX_DIRECT3DVERTEXSHADER9         *MV1_ToonOutLineVertexShader[ 3 ][ 4 ] ;

	// モデル描画用のシャドウマップへのレンダリング用頂点シェーダー
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	DX_DIRECT3DVERTEXSHADER9         *MV1_ShadowMapVertexShader[ 3 ] ;

	// モデル描画用のライティングなし頂点シェーダー
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	// [ フォグタイプ ]
	DX_DIRECT3DVERTEXSHADER9         *MV1_NoLightingVertexShader[ 3 ][ 4 ] ;

	// モデル描画用のライティングあり頂点シェーダー
	// [ シェーダーモデル( 0:SM2  1:SM3 )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	// [ バンプマップの有無( 0:無し 1:有り ) ]
	// [ フォグタイプ ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	DX_DIRECT3DVERTEXSHADER9         *MV1_LightingVertexShader[ 2 ][ 2 ][ 3 ][ 2 ][ 4 ][ 20 ][ 2 ] ;


	// モデル描画用のトゥーンレンダリングの輪郭線描画用ピクセルシェーダー
	DX_DIRECT3DPIXELSHADER9         *MV1_ToonOutLinePixelShader ;

	// モデル描画用のシャドウマップへのトゥーンレンダリング用ピクセルシェーダー
	// [ トゥーンレンダリングタイプ( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング type 2 ) ]
	// [ トゥーンレンダリングのスフィアマップの有無とブレンドタイプ( 0:スフィアマップは無い   1:MV1_LAYERBLEND_TYPE_MODULATE  2:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	DX_DIRECT3DPIXELSHADER9         *MV1_ShadowMapToonPixelShader[ 2 ][ 3 ][ 2 ] ;

	// モデル描画用のシャドウマップへのレンダリング用ピクセルシェーダー
	DX_DIRECT3DPIXELSHADER9         *MV1_ShadowMapPixelShader ;

	// モデル描画用のライティングなしトゥーン用ピクセルシェーダー( リストは配列の左から )
	// [ トゥーンレンダリングタイプ( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング type 2 ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	DX_DIRECT3DPIXELSHADER9         *MV1_ToonNoLightingPixelShader[ 2 ][ 2 ] ;

	// モデル描画用のライティングなしピクセルシェーダー
	// [ マルチテクスチャのブレンドモード、MV1_LAYERBLEND_TYPE_TRANSLUCENT などに +1 したもの ( 0:マルチテクスチャ無し  1:αブレンド  2:加算ブレンド  3:乗算ブレンド  4:乗算ブレンド×2 ) ]
	DX_DIRECT3DPIXELSHADER9         *MV1_NoLightingPixelShader[ 5 ] ;

	// モデル描画用のライティングありトゥーン用ピクセルシェーダー
	// [ シェーダーモデル( 0:SM2  1:SM3 )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ トゥーンレンダリングタイプ( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング type 2 ) ]
	// [ トゥーンレンダリングのスフィアマップの有無とブレンドタイプ( 0:スフィアマップは無い   1:MV1_LAYERBLEND_TYPE_MODULATE  2:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	// [ トゥーンレンダリングのスペキュラグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	DX_DIRECT3DPIXELSHADER9         *MV1_ToonLightingPixelShader[ 2 ][ 2 ][ 2 ][ 3 ][ 2 ][ 2 ][ 2 ][ 2 ][ 10 ][ 2 ] ;

	// モデル描画用のライティングありピクセルシェーダー
	// [ シェーダーモデル( 0:SM2  1:SM3 )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ マルチテクスチャのブレンドモード、MV1_LAYERBLEND_TYPE_TRANSLUCENT などに +1 したもの ( 0:マルチテクスチャ無し  1:αブレンド  2:加算ブレンド  3:乗算ブレンド  4:乗算ブレンド×2 ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	DX_DIRECT3DPIXELSHADER9         *MV1_LightingPixelShader[ 2 ][ 2 ][ 5 ][ 2 ][ 2 ][ 10 ][ 2 ] ;




	// 頂点シェーダ宣言( リストは配列の左から )
	// [ バンプマップ情報付きかどうか( 1:バンプマップ付き 0:付いてない ) ]
	// [ スキニングメッシュかどうか( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ 2:8ボーン内スキニングメッシュ ) ]
	// [ UVの数 ]
	DX_DIRECT3DVERTEXDECLARATION9    *DefaultVertexDeclaration[ 2 ][ 3 ][ 9 ] ;

#endif // DX_NON_MODEL

	int						InitializeFlag ;					// このフラグが立っている時は現在のステータスと変更
																// 指定のステータスが同じでも必ず実行される
	int						DisplayModeNum ;					// 変更可能なディスプレイモードの数
	DISPLAYMODEDATA			*DisplayMode ;						// ディスプレイモードリスト

	SHADERCONSTANTINFOSET	ShaderConstantInfo ;				// シェーダーの定数情報

	DX_DIRECT3DVERTEXDECLARATION9	*UserShaderDeclaration[ DX_VERTEX_TYPE_NUM ] ;	// ユーザーシェーダモードで使用する頂点データフォーマット

	USERRENDERINFO			UserShaderRenderInfo ;				// SetUseTextureToShader で設定された各ステージのテクスチャ情報や、ユーザー設定のシェーダー定数情報など

	int						WorkDrawValidGrHandle[ 2 ][ 16 ][ 2 ] ;	// フィルター処理作業用描画可能テクスチャ[ 0:整数テクスチャ 1:浮動小数点テクスチャ ][ ２のｎ乗 ][ 作業用２枚 ]
#ifndef DX_NON_FILTER
	SHADERCODE_HANDLE		ShaderCode ;						// ライブラリが用意するシェーダーコード
#endif // DX_NON_FILTER
} ;

// ソフトウエアレンダリングで使用する情報の構造体
struct GRAPHICS_SOFTDATA
{
	MEMIMG					MainBufferMemImg ;					// メイン画面用 MEMIMG
	MEMIMG					FontScreenMemImgNormal ;			// 半透明描画などの時に使うフォント用 MEMIMG ( アルファなし )
	MEMIMG					FontScreenMemImgAlpha ;				// 半透明描画などの時に使うフォント用 MEMIMG ( アルファつき )
	MEMIMG					*TargetMemImg ;						// 描画対象の MEMIMG
	MEMIMG					*BlendMemImg ;						// ブレンド描画用 MEMIMG
} ;

// グラフィック処理管理データ構造体
struct GRAPHICSMANAGE2
{
	int						InitializeFlag ;						// 初期化フラグ

	int						MainScreenSizeX, MainScreenSizeY ;		// メイン画面のサイズ
	int						MainScreenSizeX_Result, MainScreenSizeY_Result ;	// GetDrawScreenSize の返り値になるサイズ
	int						MainScreenColorBitDepth ;				// メイン画面のカラービット深度
	int						MainScreenRefreshRate ;					// メイン画面のリフレッシュレート
	RECT					OriginalDrawRect ;						// ウインドウの位置によって改変される前の使用者が意図する正しい描画矩形
	int						UseChangeDisplaySettings ;				// ChangeDisplaySettings を使用して画面モードを変更したかどうか( TRUE:ChangeDisplaySettings を使用した  FALSE:ChangeDisplaySettings は使用していない )
	int						ChangeGraphModeFlag ;					// ChangeGraphMode を実行中かどうかのフラグ

	int						ValidGraphDisplayArea ;					// GraphDisplayArea に有効な値が入っているかどうか
	RECT					GraphDisplayArea ;						// 表画面に転送する裏画面の領域
	RECT					LastCopySrcRect ;						// 最後にフィット転送したときのコピー元矩形
	RECT					LastCopyDestRect ;						// 最後にフィット転送したときのコピー先矩形
	int						EnableBackgroundColor;					// バックグラウンド塗り潰し用カラーが有効かどうかのフラグ
	int						BackgroundRed, BackgroundGreen, BackgroundBlue;	// バックグラウンド塗り潰し用カラー

	LONGLONG				VSyncWaitTime ;							// 前回ＶＳＹＮＣ待ちをしてから次にチェックするまでに待つ時間
	LONGLONG				VSyncTime ;								// 前回ＶＳＹＮＣ待ちをした時間

	int						ValidHardWare ;							// ハードウエア描画が可能かどうか( TRUE:可能  FALSE:不可能 )
	int						NotUseHardWare ;						// ハードウエア描画の機能を使用しないかどうか( TRUE:使用しない  FALSE:使用する )
	int						ChangeScreenModeNotGraphicsSystemFlag ;	// 画面モードの変更時に画像ハンドルを削除しないかどうか( TRUE:しない  FALSE:する )


	int						ShaderInitializeFlag ;					// シェーダーバイナリのセットアップが完了しているかどうかのフラグ( TRUE:完了している  FALSE:完了していない )

	DXARC					BaseShaderBinDxa ;						// 基本シェーダーオブジェクトファイルＤＸＡ構造体
	void					*BaseShaderBinDxaImage ;				// 基本シェーダーオブジェクトファイルＤＸＡのバイナリイメージ

#ifndef DX_NON_FILTER
	DXARC					FilterShaderBinDxa ;					// フィルターシェーダーオブジェクトファイルＤＸＡ構造体
	void					*FilterShaderBinDxaImage ;				// フィルターシェーダーオブジェクトファイルＤＸＡのバイナリイメージ

	void					*RGBAMixS_ShaderPackImage ;				// RGBAMix の S だけの組み合わせ２５６個のシェーダーパッケージバイナリイメージ

	// RGBAMix の S だけの組み合わせ２５６個のシェーダー[ R ][ G ][ B ][ A ]
	void					*RGBAMixS_ShaderAddress[ 4 ][ 4 ][ 4 ][ 4 ] ;
	short					RGBAMixS_ShaderSize[ 4 ][ 4 ][ 4 ][ 4 ] ;
#endif // DX_NON_FILTER

	// ライトインデックスリスト
	short					LightIndexList84[ 4 ][ 4 ][ 4 ][ 4 ][ 4 ][ 4 ] ;
	short					LightIndexList20[ 4 ][ 4 ][ 4 ] ;

	short					LightIndexList10[ 3 ][ 3 ][ 3 ] ;

#ifndef DX_NON_MODEL
	void					*ModelShaderPackImage ;					// ３Ｄモデル用シェーダーパッケージバイナリイメージ

	// モデル描画用ピクセル単位ライティングタイプの頂点シェーダー( リストは配列の左から )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ 2:8ボーン内スキニングメッシュ ) ]
	// [ バンプマップの有無( 0:無し 1:有り ) ]
	// [ フォグタイプ ]
	void					*MV1_PixelLighting_VertexShaderAddress[ 2 ][ 3 ][ 2 ][ 4 ] ;

	// モデル描画用ピクセル単位ライティングタイプのトゥーン用ピクセルシェーダー( リストは配列の左から )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ トゥーンレンダリングかどうか( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング Type 2 ) ]
	// [ トゥーンレンダリングのスフィアマップの有無とブレンドタイプ( 0:スフィアマップは無い   1:MV1_LAYERBLEND_TYPE_MODULATE  2:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	// [ トゥーンレンダリングのスペキュラグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	void					*MV1_PixelLighting_ToonPixelShaderAddress[ 2 ][ 2 ][ 3 ][ 2 ][ 2 ][ 2 ][ 2 ][ 84 ][ 2 ] ;

	// モデル描画用ピクセル単位ライティングタイプのピクセルシェーダー( リストは配列の左から )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ マルチテクスチャのブレンドモード、MV1_LAYERBLEND_TYPE_TRANSLUCENT などに +1 したもの ( 0:マルチテクスチャ無し  1:αブレンド  2:加算ブレンド  3:乗算ブレンド  4:乗算ブレンド×2 ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	void					*MV1_PixelLighting_PixelShaderAddress[ 2 ][ 5 ][ 2 ][ 2 ][ 84 ][ 2 ] ;






	// モデル描画用のトゥーンレンダリングの輪郭線描画用頂点シェーダー
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	// [ フォグタイプ ]
	void					*MV1_ToonOutLineVertexShaderAddress[ 3 ][ 4 ] ;

	// モデル描画用のシャドウマップへのレンダリング用頂点シェーダー
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	void					*MV1_ShadowMapVertexShaderAddress[ 3 ] ;

	// モデル描画用のライティングなし頂点シェーダー
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	// [ フォグタイプ ]
	void					*MV1_NoLightingVertexShaderAddress[ 3 ][ 4 ] ;

	// モデル描画用のライティングあり頂点シェーダー
	// [ シェーダーモデル( 0:SM2  1:SM3 )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ メッシュタイプ( 0:剛体メッシュ 1:4ボーン内スキニングメッシュ  2:8ボーン内スキニングメッシュ ) ]
	// [ バンプマップの有無( 0:無し 1:有り ) ]
	// [ フォグタイプ ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	void					*MV1_LightingVertexShaderAddress[ 2 ][ 2 ][ 3 ][ 2 ][ 4 ][ 20 ][ 2 ] ;


	// モデル描画用のトゥーンレンダリングの輪郭線描画用ピクセルシェーダー
	void					*MV1_ToonOutLinePixelShaderAddress ;

	// モデル描画用のシャドウマップへのトゥーンレンダリング用ピクセルシェーダー
	// [ トゥーンレンダリングタイプ( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング type 2 ) ]
	// [ トゥーンレンダリングのスフィアマップの有無とブレンドタイプ( 0:スフィアマップは無い   1:MV1_LAYERBLEND_TYPE_MODULATE  2:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	void					*MV1_ShadowMapToonPixelShaderAddress[ 2 ][ 3 ][ 2 ] ;

	// モデル描画用のシャドウマップへのレンダリング用ピクセルシェーダー
	void					*MV1_ShadowMapPixelShaderAddress ;

	// モデル描画用のライティングなしトゥーン用ピクセルシェーダー( リストは配列の左から )
	// [ トゥーンレンダリングタイプ( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング type 2 ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT or トゥーンレンダリングではない  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	void					*MV1_ToonNoLightingPixelShaderAddress[ 2 ][ 2 ] ;

	// モデル描画用のライティングなしピクセルシェーダー
	// [ マルチテクスチャのブレンドモード、MV1_LAYERBLEND_TYPE_TRANSLUCENT などに +1 したもの ( 0:マルチテクスチャ無し  1:αブレンド  2:加算ブレンド  3:乗算ブレンド  4:乗算ブレンド×2 ) ]
	void					*MV1_NoLightingPixelShaderAddress[ 5 ] ;

	// モデル描画用のライティングありトゥーン用ピクセルシェーダー
	// [ シェーダーモデル( 0:SM2  1:SM3 )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ トゥーンレンダリングタイプ( 0:トゥーンレンダリング type 1   1:トゥーンレンダリング type 2 ) ]
	// [ トゥーンレンダリングのスフィアマップの有無とブレンドタイプ( 0:スフィアマップは無い   1:MV1_LAYERBLEND_TYPE_MODULATE  2:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ トゥーンレンダリングのディフューズグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_MODULATE ) ]
	// [ トゥーンレンダリングのスペキュラグラデーションのブレンドタイプ( 0:MV1_LAYERBLEND_TYPE_TRANSLUCENT  1:MV1_LAYERBLEND_TYPE_ADDITIVE ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	void					*MV1_ToonLightingPixelShaderAddress[ 2 ][ 2 ][ 2 ][ 3 ][ 2 ][ 2 ][ 2 ][ 2 ][ 10 ][ 2 ] ;

	// モデル描画用のライティングありピクセルシェーダー
	// [ シェーダーモデル( 0:SM2  1:SM3 )
	// [ シャドウマップの有無( 0:無し 1:有り ) ]
	// [ マルチテクスチャのブレンドモード、MV1_LAYERBLEND_TYPE_TRANSLUCENT などに +1 したもの ( 0:マルチテクスチャ無し  1:αブレンド  2:加算ブレンド  3:乗算ブレンド  4:乗算ブレンド×2 ) ]
	// [ スペキュラマップ ( 0:無い 1:ある ) ]
	// [ バンプマップ ( 0:なし 1:あり ) ]
	// [ ライトインデックス ]
	// [ スペキュラの有無( 0:無し 1:有り ) ]
	void					*MV1_LightingPixelShaderAddress[ 2 ][ 2 ][ 5 ][ 2 ][ 2 ][ 10 ][ 2 ] ;

#endif // DX_NON_MODEL

	GRAPHICS_HARDDATA		Hard ;									// ハードウェアアクセラレータを使用する処理に必要な情報
	GRAPHICS_SOFTDATA		Soft ;									// ソフトウエアレンダリングを使用する際に必要な情報
} ;

// 内部大域変数宣言---------------------------------------------------------------

// グラフィックス管理データ構造体
extern GRAPHICSMANAGE2 GraphicsManage2 ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// グラフィック関連の初期化と後始末
extern	int		InitializeGraphics2( void ) ;
extern	int		TerminateGraphics2( void ) ;

extern	int		SetZBufferMode( int ZBufferSizeX, int ZBufferSizeY, int ZBufferBitDepth ) ;	// メイン画面のＺバッファの設定を変更する
extern	void	SetMainScreenSize( int SizeX, int SizeY ) ;									// メイン画面のサイズ値を変更する
extern	void	ReleaseDirectXObject( void ) ;												// DirectX のオブジェクトを解放する
extern	void	BeginScene( void ) ;														// ビギンシーンを行う	
extern	void	EndScene( void ) ;															// エンドシーンを行う	
extern	int		RenderVertexHardware( int ASyncThread = FALSE ) ;							// 頂点バッファに溜まった頂点データをレンダリングする
extern	int		SetRenderTargetHardware( DX_DIRECT3DSURFACE9 *TargetSurface, int TargetIndex = 0 ) ;	// 描画対象の変更
extern	int		SetViewportHardwareEasy( int x1, int y1, int x2, int y2 ) ;					// ビューポートをセットする( 簡易版 )
extern	int		SetupUseZBuffer( void ) ;													// 設定に基づいて使用するＺバッファをセットする
extern	int		CreateSubBackBufferSurface( void ) ;										// ScreenCopy や GetDrawScreen を実現するために使用するテンポラリバッファの作成( 0:成功  -1:失敗 )
extern COLORDATA *GetD3DFormatColorData( D_D3DFORMAT Format ) ;								// D3DFORMAT のフォーマットに合わせたカラーフォーマット情報を取得する
extern	void FASTCALL SetDrawBrightToOneParam( DWORD Bright ) ;								// SetDrawBright の引数が一つ版
extern	void FASTCALL BlendModeSub_Pre( RECT *DrawRect ) ;
extern	void FASTCALL BlendModeSub_Post( RECT *DrawRect ) ;
extern	void FASTCALL AlphaChDraw_Pre( RECT *DrawRect ) ;
extern	int		GetWorkTexture( int IsFloatType, DWORD TexSizeW, DWORD TexSizeH, DWORD HandleIndex ) ;
extern	int		SetUseAutoMipMap( int UseFlag, int MaxLevel = -1 ) ;
extern	int		ChangeGraphMode( int ScreenSizeX, int ScreenSizeY, int ColorBitDepth, int ChangeWindowFlag, int RefreshRate ) ;				// 画面モードの変更２
extern	int		SetD3DDialogBoxMode( int Flag ) ;
#ifndef DX_NON_MODEL
extern	int		SetupShader( int VertexShaderIndex, int VertexShaderIndex_SM3, int PixelShaderIndex, int PixelShaderIndex_SM3 ) ;	// 指定のシェーダーをセットアップする( TRUE:成功  FALSE:失敗 )
extern	int		SetupVertexDeclaration( int BumpMap, int SkinMesh, int UVNum ) ;											// 指定の頂点データ定義をセットアップ
#endif // DX_NON_MODEL
extern	int		SetDeviceVertexShaderToHandle( int ShaderHandle ) ;															// 指定の頂点シェーダーをデバイスにセットする
extern	int		SetDevicePixelShaderToHandle( int ShaderHandle ) ;															// 指定のピクセルシェーダーをデバイスにセットする
extern	int		LockDrawScreenBuffer( RECT *LockRect, BASEIMAGE *BaseImage, int TargetScreen/* = -1*/, int TargetScreenSurface/* = -1*/, int ReadOnly/* = TRUE*/, int TargetScreenTextureNo/* = 0*/ ) ;	// 描画先バッファをロックする
extern	int		UnlockDrawScreenBuffer( void ) ;																			// 描画先バッファをアンロックする
extern	int		SetBlendGraphParamBase( int BlendGraph, int BlendType, va_list ParamList ) ;								// SetBlendGraphParam の可変長引数パラメータ付き
extern	int		GraphFilterRectBltBase( int IsBlend, int SrcGrHandle, int BlendGrHandle, int DestGrHandle, int BlendRatio, int FilterOrBlendType, int SrcX1, int SrcY1, int SrcX2, int SrcY2, int BlendX, int BlendY, int BlendPosEnable, int DestX, int DestY, va_list ParamList ) ;		// 画像のフィルター付き転送を行う( 可変引数情報付き )

extern	int		SysMemTextureProcess( void ) ;																				// 管理テクスチャへの転送用のシステムメモリテクスチャの定期処理を行う
extern	int		SysMemSurfaceProcess( void ) ;																				// 管理テクスチャへの転送用のシステムメモリサーフェスの定期処理を行う

extern	int		CreateShaderHandle( void ) ;																				// シェーダーハンドルを作成する
extern	int		CreateShader_UseGParam( int IsVertexShader, void *Image, int ImageSize, int ImageAfterFree, int ASyncLoadFlag = FALSE, int ASyncThread = FALSE ) ;		// シェーダーハンドルを作成する
extern	int		LoadShader_UseGParam( int IsVertexShader, const TCHAR *FileName, int ASyncLoadFlag = FALSE ) ;				// シェーダーバイナリをファイルから読み込む

extern	int		InitializeShaderHandle( HANDLEINFO *HandleInfo ) ;															// シェーダーハンドルの初期化
extern	int		TerminateShaderHandle( HANDLEINFO *HandleInfo ) ;															// シェーダーハンドルの後始末

extern	int		SetupVertexBufferHandle( int VertexBufHandle, int VertexNum, int VertexType /* DX_VERTEX_TYPE_NORMAL_3D 等 */ ) ;	// 頂点バッファハンドルのセットアップを行う
extern	int		InitializeVertexBufferHandle( HANDLEINFO *HandleInfo ) ;													// 頂点バッファハンドルの初期化
extern	int		TerminateVertexBufferHandle( HANDLEINFO *HandleInfo ) ;														// 頂点バッファハンドルの後始末

extern	int		SetupIndexBufferHandle( int IndexBufHandle, int IndexNum, int IndexType /* DX_INDEX_TYPE_16BIT 等 */ ) ;	// インデックスバッファハンドルのセットアップを行う
extern	int		InitializeIndexBufferHandle( HANDLEINFO *HandleInfo ) ;														// インデックスバッファハンドルの初期化
extern	int		TerminateIndexBufferHandle( HANDLEINFO *HandleInfo ) ;														// インデックスバッファハンドルの後始末

// DirectX 関連関数
extern	int		InitializeDirectX() ;									// DirectX 関連の初期化( 0:成功  -1:失敗 )
extern	int		TerminateDirectX() ;									// DirectX 関連の後始末

extern	int		CreateDirect3D9DeviceObject() ;							// Direct3DDevice9 オブジェクトの作成( 0:成功  -1:失敗 )

extern	int		SetupDirect3D9Shader() ;								// Direct3D9 のシェーダーのセットアップを行う( 0:成功  -1:失敗 )
extern	int		TerminateDirect3D9Shader() ;							// Direct3D9 のシェーダーの後始末をする( 0:成功  -1:失敗 )

extern	int		CreateDirect3D9VertexDeclaration() ;					// Direct3DVertexDeclaration9 オブジェクトの作成( 0:成功  -1:失敗 )
extern	int		TerminateDirect3D9VertexDeclaration() ;					// Direct3DVertexDeclaration9 オブジェクトの削除( 0:成功  -1:失敗 )

#ifndef DX_NON_MODEL
extern	int		CreateRGBtoVMaxRGBVolumeTexture() ;						// RGBカラーを輝度を最大にしたRGB値に変換するためのボリュームテクスチャを作成する( 0:成功  -1:失敗 )
#endif // DX_NON_MODEL

extern	int		CreateDirect3D9ZBufferObject() ;						// Ｚバッファオブジェクトの作成( 0:成功  -1:失敗 )


// ハードウエア設定関係関数
extern	void	ApplyLibMatrixToHardware( void ) ;						// 基本データに設定されている行列をハードウエアに反映する
extern	void	ApplyLigFogToHardware( void ) ;							// 基本データに設定されているフォグ情報をハードウエアに反映する

extern	int		D_SetVertexBlend( int WeightNum ) ;					// 行列ブレンドの数を設定する( DrawGraph などを呼ぶ前に 0 を引数にして呼び出さないとその後の描画が正常に行われなくなります )
extern	int		D_DrawIndexedPrimitive( void *VertexBuffer9, DWORD VertexStride, DWORD FVFFlag, void *IndexBuffer9, int PrimitiveType, int BaseVertexIndex, DWORD MinIndex, DWORD NumVertices, DWORD StartIndex, DWORD PrimitiveCount ) ;						// Direct3DDevice9->DrawIndexedPrimitive のラッピング関数
extern	int		D_DrawIndexedPrimitiveUP( DWORD FVFFlag, int PrimitiveType, DWORD MinVertexIndex, DWORD NumVertexIndices, DWORD PrimitiveCount, void *pIndexData, D_D3DFORMAT IndexDataFormat, void *pVertexStreamZeroData, DWORD VertexStreamZeroStride ) ;		// Direct3DDevice9->DrawIndexedPrimitiveUP のラッピング関数
extern	int		D_SetLightEnable( int Flag ) ;											// ライトを有効にするかどうかをセットする
extern	int		D_SetLightParam( int No, int EnableFlag, LIGHTPARAM *Param = NULL ) ;	// ライトのパラメータをセットする
extern	int		D_SetAmbientLight( int r, int g, int b ) ;								// アンビエントライトを設定する
extern	int		D_SetUseSpecular( int UseFlag ) ;										// スペキュラを使用するかどうかを設定する
extern	int		D_SetMaterial( MATERIALPARAM *Param ) ;									// マテリアルパラメータをセットする
extern	int		D_SetMaterialUseVertexDiffuseColor( int UseFlag ) ;						// 頂点ディフューズカラーをマテリアルのディフューズカラーとして使用するかどうかを設定する
extern	int		D_SetMaterialUseVertexSpecularColor( int UseFlag ) ;						// 頂点スペキュラカラーをマテリアルのスペキュラカラーとして使用するかどうかを設定する
extern	int		D_SetShadeMode( int ShadeMode ) ;										// シェードモードをセットする
extern	int		D_GetShadeMode( void ) ;													// シェードモードを取得する
extern	int		D_SetMaxAnisotropy( int TexStage, int MaxAnisotropy ) ;					// 最大異方性をセットする
extern	int		D_SetUseCullingFlag( int Flag ) ;										// ポリゴンカリングの有効、無効をセットする
extern	int		D_SetupZBuffer3D( void ) ;												// ３Ｄ描画用のＺバッファ設定をハードに反映する
extern	int		D_SetupUseVertColor( void ) ;											// 頂点カラーを使用するかどうかの設定をハードに反映する
extern	int		D_SetUseZBufferFlag( int Flag ) ;										// Ｚバッファを有効にするか、フラグをセットする
extern	int		D_SetWriteZBufferFlag( int Flag ) ;										// Ｚバッファに書き込みを行うか、フラグをセットする
extern	int		D_SetDrawAlphaTest( int TestMode, int TestParam ) ;						// 描画時のアルファテストのモードをセットする
extern	int		D_SetZBufferCmpType( int CmpType /* DX_CMP_NEVER 等 */ )	;				// Ｚ値の比較モードをセットする
extern	int		D_SetZBias( int Bias ) ;													// Ｚバイアスをセットする
extern	int		D_SetFillMode( int FillMode ) ;												// フィルモードをセットする
extern	int		D_SetTextureAddressMode( int Mode /* DX_TEXADDRESS_WRAP 等 */, int Stage = -1 ) ;	// テクスチャアドレスモードを設定する
extern	int		D_SetTextureAddressModeUVW( int ModeU, int ModeV, int ModeW, int Stage ) ;			// テクスチャアドレスモードを設定する
extern	int		D_SetTextureAddressTransformMatrix( int Use, MATRIX *Matrix, int Stage = -1 ) ;		// テクスチャ座標変換行列をセットする
extern	int		D_SetTransformToWorld( MATRIX *Matrix ) ;								// ローカル→ワールド行列を変更する
extern	int		D_SetTransformToView( MATRIX *Matrix ) ;									// ビュー変換用行列をセットする
extern	int		D_SetTransformToProjection( MATRIX *Matrix ) ;							// 投影変換用行列をセットする
extern	int		D_ResetTextureCoord( void ) ;											// 各ステージが使用するテクスチャアドレスをステージ番号と同じにする
extern	int		_SetTransformToProjection( const MATRIX *Matrix ) ;						// 投影変換用行列をセットする
//extern	int		RenderVertexBuffer( void ) ;										// 頂点バッファに溜まった頂点データを吐き出す
extern	void	_DrawPreparation( int GrHandle = -1 , int ParamFlag = 0 ) ;										// 描画準備を行う( ParamFlag は DRAWPREP_TRANS 等 )



extern	int		AddGraphHandle( void ) ;																						// 新しいグラフィックハンドルを確保する
extern	int		SetupGraphHandle_UseGParam( SETUP_GRAPHHANDLE_GPARAM *GParam, int GrHandle, int Width, int Height, int TextureFlag, int AlphaValidFlag, int UsePaletteFlag, int BaseFormat/* = DX_BASEIMAGE_FORMAT_NORMAL*/, int MipMapCount, int ASyncThread ) ;							// SetupGraphHandle のグローバル変数にアクセスしないバージョン

extern	int		InitializeGraphHandle( HANDLEINFO *HandleInfo ) ;																// グラフィックハンドルの初期化
extern	int		TerminateGraphHandle( HANDLEINFO *HandleInfo ) ;																// グラフィックハンドルの後始末

extern	int		AddShadowMapHandle( void ) ;																						// 新しいシャドウマップハンドルを確保する
extern	int		SetupShadowMapHandle_UseGParam( SETUP_SHADOWMAPHANDLE_GPARAM *GParam, int SmHandle, int SizeX, int SizeY, int TexFormat_Float, int TexFormat_BitDepth, int ASyncThread ) ;	// シャドウマップハンドルのセットアップを行う
extern	int		CreateShadowMapTexture( SHADOWMAPDATA *ShadowMap, int ASyncThread = FALSE ) ;										// シャドウマップデータに必要なテクスチャを作成する
extern	int		ReleaseShadowMapTexture( SHADOWMAPDATA *ShadowMap ) ;																// シャドウマップデータに必要なテクスチャを解放する
extern	SHADOWMAPDATA	*GetShadowMapData( int SmHandle, int ASyncThread = FALSE ) ;												// シャドウマップデータをハンドル値から取り出す
extern	void	RefreshShadowMapVSParam( void ) ;																					// 頂点シェーダーに設定するシャドウマップの情報を更新する
extern	void	RefreshShadowMapPSParam( void ) ;																					// ピクセルシェーダーに設定するシャドウマップの情報を更新する

extern	int		InitializeShadowMapHandle( HANDLEINFO *HandleInfo ) ;																// シャドウマップハンドルの初期化
extern	int		TerminateShadowMapHandle( HANDLEINFO *HandleInfo ) ;																// シャドウマップハンドルの後始末

extern	int		AllocCommonBuffer( int Index, DWORD Size ) ;																	// 共有メモリの確保
extern	int		TerminateCommonBuffer( void ) ;																					// 共有メモリの解放

extern	int		CreateDXGraph_UseGParam( SETUP_GRAPHHANDLE_GPARAM *GParam, int GrHandle, const BASEIMAGE *RgbImage, const BASEIMAGE *AlphaImage, int TextureFlag, int ASyncThread = FALSE ) ;																															// CreateDXGraph のグローバル変数にアクセスしないバージョン
extern	int		DerivationGraph_UseGParam( int SrcX, int SrcY, int Width, int Height, int SrcGraphHandle, int ASyncThread = FALSE ) ;																																												// DerivationGraph のグローバル変数にアクセスしないバージョン
#ifndef DX_NON_MOVIE
extern	int		LoadBmpToGraph_OpenMovie_UseGParam( LOADGRAPH_GPARAM *GParam, int GrHandle, const TCHAR *GraphName, int TextureFlag, int SurfaceMode = DX_MOVIESURFACE_NORMAL, int ASyncThread = FALSE ) ;
#endif

extern	int		DerivationGraphBase( int GrHandle, int SrcX, int SrcY, int Width, int Height, int SrcGraphHandle, int ASyncThread = FALSE ) ;																																								// グラフィックハンドルを作成しない DerivationGraph
extern	int		SetGraphBaseInfo_UseGParam( SETGRAPHBASEINFO_GPARAM *GParam, int GrHandle, const TCHAR *FileName, const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, const void *MemImage, int MemImageSize,
										   const void *AlphaMemImage, int AlphaMemImageSize, const BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int ReverseFlag, int UnionGrHandle, int ASyncThread ) ;																														// SetGraphBaseInfo のグローバル変数にアクセスしないバージョン
extern	int		SetGraphName_UseGParam( SETGRAPHBASEINFO_GPARAM *GParam, int Handle, const TCHAR *GraphName, int ReverseFlag, int UnionGrHandle, int ASyncThread ) ;																																		// SetGraphName のグローバル変数にアクセスしないバージョン

extern	int		FillGraph_UseGParam( int GrHandle, int Red, int Green, int Blue, int Alpha, int ASyncThread ) ;																																																// FillGraph のグローバル変数にアクセスしないバージョン

extern	int		SetGraphName( int Handle, const TCHAR *GraphName, int ReverseFlag, int UnionGrHandle, int ASyncThread ) ;	// 特定のファイルから画像を読み込んだ場合のファイルパスをセットする
extern	int		IsValidGraphHandle( int GrHandle ) ;															// グラフィックハンドルが有効かどうかを調べる( TRUE:有効  FALSE:無効 )


extern	int		ScreenFlipBase( RECT *CopyRect ) ;											// ScreenFlip のベース関数
extern	int		SetGraphBaseInfo( int GrHandle, const TCHAR *FileName, const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, const void *MemImage, int MemImageSize, const void *AlphaMemImage, int AlphaMemImageSize, const BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int ReverseFlag, int UnionGrHandle, int ASyncThread ) ;		// 画像の元データの情報を保存する

extern	int		BltBmpOrGraphImageToGraphBase(    const COLORDATA *SrcColorData, HBITMAP Bmp, HBITMAP AlphaMask, int BmpFlag, const BASEIMAGE *RgbImage, const BASEIMAGE *AlphaImage, int CopyPointX, int CopyPointY, int GrHandle, int UseTransColorConvAlpha = TRUE, int ASyncThread = FALSE ) ;											// BltBmpOrGraphImageToGraph の内部関数
extern	int		BltBmpOrGraphImageToGraph2Base(   const COLORDATA *SrcColorData, HBITMAP Bmp, HBITMAP AlphaMask, int BmpFlag, const BASEIMAGE *RgbImage, const BASEIMAGE *AlphaImage, const RECT *SrcRect, int DestX, int DestY, int GrHandle ) ;																							// BltBmpOrGraphImageToGraph2 の内部関数
extern	int		BltBmpOrGraphImageToDivGraphBase( const COLORDATA *SrcColor,     HBITMAP Bmp, HBITMAP AlphaMask, int BmpFlag, const BASEIMAGE *RgbImage, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int Width, int Height, const int *GrHandle, int ReverseFlag, int UseTransColorConvAlpha = TRUE, int ASyncThread = FALSE ) ;	// BltBmpOrGraphImageToDivGraph の内部関数

extern	int		SetUserBlendInfo( DIRECT3DBLENDINFO *BlendInfo, int TextureStageIsTextureAndTextureCoordOnlyFlag = FALSE ) ;	// テクスチャステージステートを直接指定する( NULL で無効 )
extern	int		D_SetSampleFilterMode( int TexStage, int SetTarget, DX_D3DTEXFILTER_TYPE FilterType ) ;	// テクスチャフィルタリングモードを設定する
extern	DX_D3DTEXFILTER_TYPE D_GetSampleFilterMode( int TexStage, int SetTarget ) ;		// テクスチャフィルタリングモードを取得する
extern	int		D_SetUserBlendInfo( DIRECT3DBLENDINFO *BlendInfo, int TextureStageIsTextureAndTextureCoordOnlyFlag, int TextureIsGraphHandleFlag = TRUE ) ;	// ユーザーのブレンドインフォを適応する
extern IMAGEDATA2 *GetGraphData2( int GrHandle, int ASyncThread = FALSE ) ;					// グラフィックのデータをインデックス値から取り出す
extern	D_D3DXSHADER_CONSTANTINFO *GetShaderConstInfo( SHADERHANDLEDATA *Shader, const TCHAR *ConstantName ) ;				// シェーダーの定数情報を得る

extern	int		InitializeShaderConstantUseArea( SHADERCONSTANT_USEAREA *UseArea, int TotalSize ) ;							// シェーダー定数使用領域情報を初期化する
extern	int		SetShaderConstantUseArea( SHADERCONSTANT_USEAREA *UseArea, int IsUse, int Index, int Num ) ;				// シェーダー定数使用領域情報を変更する
extern	int		CreateUseAreaMap( SHADERCONSTANT_USEAREA *UseArea, BYTE *Map, BYTE SetNumber ) ;							// シェーダー定数使用領域情報から使用マップを作成する

extern	int		InitializeShaderConstantInfoSet( SHADERCONSTANTINFOSET *ConstInfoSet ) ;																	// シェーダー定数情報の初期化
extern	int		SetUseShaderContantInfoState( SHADERCONSTANTINFOSET *ConstInfoSet, int ApplyMask ) ;														// 指定のシェーダー定数セットを適用するかどうかを設定する DX_SHADERCONSTANTSET_MASK_LIB | DX_SHADERCONSTANTSET_MASK_LIB_SUB 等
extern	int		SetShaderConstantSet(   SHADERCONSTANTINFOSET *ConstInfoSet, int TypeIndex, int SetIndex, int ConstantIndex, const void *Param, int ParamNum, int UpdateUseArea ) ;	// シェーダー定数情報を設定する
extern	int		ResetShaderConstantSet( SHADERCONSTANTINFOSET *ConstInfoSet, int TypeIndex, int SetIndex, int ConstantIndex, int ParamNum ) ;										// 指定領域のシェーダー定数情報をリセットする


}

#endif // __DXGRAPHICS_H__


