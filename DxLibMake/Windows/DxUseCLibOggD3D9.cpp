//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Ogg関係プログラム( Direct3D9 )
// 
//  	Ver 3.14f
// 
//-----------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

#include "../DxCompileConfig.h"

#ifndef DX_NON_DIRECT3D9

#if !defined( DX_NON_OGGVORBIS ) || !defined( DX_NON_OGGTHEORA )

// インクルード----------------------------------------------------------------
#include "../DxLib.h"
#include "../DxStatic.h"
#include "../DxUseCLib.h"
#include "../DxGraphics.h"
#include "../DxLog.h"
#include "DxGraphicsAPIWin.h"
#include "DxUseCLibOggD3D9.h"
#include "DxUseCLibOggWin.h"

//namespace DxLib
//{

// マクロ定義------------------------------------------------------------------

// 型定義----------------------------------------------------------------------

// データ宣言------------------------------------------------------------------


// 関数プロトタイプ宣言 -------------------------------------------------------

// プログラム------------------------------------------------------------------

// 環境依存関数

// ハードウエア機能が有効な場合は YUV フォーマットの一時保存用テクスチャを作成する
extern	int	TheoraDecode_D3D9_CreateSurface_PF( DECODE_THEORA *DT )
{
	ogg_uint32_t w, h ;

	// ハードウエア機能が有効な場合は YUV フォーマットの一時保存用テクスチャを作成する
	if( DT->NotUseYUVFormatSurface || GSYS.Setting.ValidHardware == FALSE || Graphics_Hardware_CheckValid_PF() == 0 )
	{
		return 0 ;
	}

	DT->PF->D3D9.YUVSurface = NULL ;

	w = DT->TheoraInfo.width ;
	h = DT->TheoraInfo.height ;

	// 最初は YV12 形式を試す
	DT->YUVImageFourCC = MAKEFOURCC( 'Y', 'V', '1', '2' ) ;
	Direct3DDevice9_CreateOffscreenPlainSurface( w, h, ( D_D3DFORMAT )DT->YUVImageFourCC, D_D3DPOOL_DEFAULT, ( D_IDirect3DSurface9 ** )&DT->PF->D3D9.YUVSurface, NULL ) ;

	// 作成に失敗したら YUY2 フォーマットを試してみる
	if( DT->PF->D3D9.YUVSurface == NULL )
	{
		DT->YUVImageFourCC = MAKEFOURCC( 'Y', 'U', 'Y', '2' ) ;
		Direct3DDevice9_CreateOffscreenPlainSurface( w, h, ( D_D3DFORMAT )DT->YUVImageFourCC, D_D3DPOOL_DEFAULT, ( D_IDirect3DSurface9 ** )&DT->PF->D3D9.YUVSurface, NULL ) ;
	}

	// それでも駄目なら UYVY フォーマットを試す
	if( DT->PF->D3D9.YUVSurface == NULL )
	{
		DT->YUVImageFourCC = MAKEFOURCC( 'U', 'Y', 'V', 'Y' ) ;
		Direct3DDevice9_CreateOffscreenPlainSurface( w, h, ( D_D3DFORMAT )DT->YUVImageFourCC, D_D3DPOOL_DEFAULT, ( D_IDirect3DSurface9 ** )&DT->PF->D3D9.YUVSurface, NULL ) ;
	}

	// それでも駄目なら YUV フォーマットを諦める
	if( DT->PF->D3D9.YUVSurface == NULL )
	{
		DT->YUVImageFourCC = 0 ;
		DT->ValidYUVSurface = FALSE ;
	}
	else
	{
		DT->ValidYUVSurface = TRUE ;
	}

	// 正常終了
	return 0 ;
}

extern	void	TheoraDecode_D3D9_ReleaseSurface_PF( DECODE_THEORA *DT )
{
	DT->YUVSurfaceReleaseRequest = FALSE ;

	if( DT->PF->D3D9.YUVSurface != NULL )
	{
		DT->PF->D3D9.YUVSurface->Release() ;
		DT->PF->D3D9.YUVSurface = NULL ;
	}
}

// カレントフレームのRGB画像を作成する( 戻り値  1:作成された  0:されなかった )
extern	int		TheoraDecode_D3D9_SetupImage_PF( DECODE_THEORA *DT, volatile THEORA_STOCKFRAME *Stock, int ASyncThread )
{
	D_D3DLOCKED_RECT LockRect = { 0 } ;
	int              Ok ;
	unsigned char   *d ;
	unsigned char   *ys ;
	unsigned char   *us ;
	unsigned char   *vs ;
	int              i ;
	int              j ;
	int              ysadd ;
	int              dadd ;
	int              uvadd ;
	int              w ;
	int              h ;
	int              dpitch ;

	// ロックに失敗したらサーフェスの作成しなおし
	Ok = 0 ;
	for( i = 0 ; i < 3 ; i ++ )
	{
		if( DT->ValidYUVSurface == FALSE || DT->YUVSurfaceReleaseRequest || Direct3DSurface9_LockRect_ASync( DT->PF->D3D9.YUVSurface, &LockRect, NULL, 0, ASyncThread ) != D_DD_OK )
		{
			TheoraDecode_CreateSurface( DT, ASyncThread ) ;
		}
		else
		{
			Ok = 1 ;
			break ;
		}
	}

	// 作成しなおしても駄目だったら諦め
	if( Ok == 1 )
	{
		// フォーマットによって処理を分岐
		d  = ( unsigned char * )LockRect.pBits ;
		ys = ( unsigned char * )Stock->YBuffer ;
		us = ( unsigned char * )Stock->UBuffer ;
		vs = ( unsigned char * )Stock->VBuffer ;
		dpitch = LockRect.Pitch ;

		// YV12 の場合
		if( DT->YUVImageFourCC == MAKEFOURCC( 'Y', 'V', '1', '2' ) )
		{
			if( Stock->YWidth  == Stock->UVWidth  * 2 &&
				Stock->YHeight == Stock->UVHeight * 2 )
			{
				ysadd  = Stock->YStride  * 2 - Stock->YWidth ;
				uvadd  = Stock->UVStride     - Stock->UVWidth ; 
				dadd   = dpitch          * 2 - Stock->YWidth ;
				w = Stock->YWidth  / 2 ;
				h = Stock->YHeight / 2 ;

				for( i = 0 ; i < Stock->YHeight ; i ++, d += dpitch, ys += Stock->YStride )
				{
					_MEMCPY( d, ys, ( size_t )Stock->YWidth ) ; 
				}
				for( i = 0 ; i < Stock->UVHeight ; i ++, d += dpitch / 2, vs += Stock->UVStride )
				{
					_MEMCPY( d, vs, ( size_t )Stock->UVWidth ) ; 
				}
				for( i = 0 ; i < Stock->UVHeight ; i ++, d += dpitch / 2, us += Stock->UVStride )
				{
					_MEMCPY( d, us, ( size_t )Stock->UVWidth ) ; 
				}
			}
		}
		else
		// YUY2 の場合
		if( DT->YUVImageFourCC == MAKEFOURCC( 'Y', 'U', 'Y', '2' ) )
		{
			if( Stock->YWidth  == Stock->UVWidth  * 2 &&
				Stock->YHeight == Stock->UVHeight * 2 )
			{
				ysadd  = Stock->YStride  * 2 - Stock->YWidth ;
				uvadd  = Stock->UVStride     - Stock->UVWidth ;
				dadd   = dpitch     * 2 - Stock->YWidth * 2 ;
				w = Stock->YWidth  / 2 ;
				h = Stock->YHeight / 2 ;
				for( i = 0; i < h; i++, d += dadd, ys += ysadd, vs += uvadd, us += uvadd )
				{
					for( j = 0; j < w; j ++, d += 4, ys += 2, us ++, vs ++ )
					{
						d[          0 ] = ys[                0 ] ;
						d[          2 ] = ys[                1 ] ;
						d[ dpitch     ] = ys[ Stock->YStride     ] ;
						d[ dpitch + 2 ] = ys[ Stock->YStride + 1 ] ;

						d[          1 ] = *us ;
						d[ dpitch + 1 ] = *us ;

						d[          3 ] = *vs ;
						d[ dpitch + 3 ] = *vs ;
					}
				}
			}
		}
		else
		// UYVY の場合
		if( DT->YUVImageFourCC == MAKEFOURCC( 'U', 'Y', 'V', 'Y' ) )
		{
			if( Stock->YWidth  == Stock->UVWidth  * 2 &&
				Stock->YHeight == Stock->UVHeight * 2 )
			{
				ysadd  = Stock->YStride  * 2 - Stock->YWidth ;
				uvadd  = Stock->UVStride     - Stock->UVWidth ;
				dadd   = dpitch     * 2 - Stock->YWidth * 2 ;
				w = Stock->YWidth  / 2 ;
				h = Stock->YHeight / 2 ;
				for( i = 0; i < h; i++, d += dadd, ys += ysadd, vs += uvadd, us += uvadd )
				{
					for( j = 0; j < w; j ++, d += 4, ys += 2, us ++, vs ++ )
					{
						d[          1 ] = ys[                0 ] ;
						d[          3 ] = ys[                1 ] ;
						d[ dpitch + 1 ] = ys[ Stock->YStride     ] ;
						d[ dpitch + 3 ] = ys[ Stock->YStride + 1 ] ;

						d[          0 ] = *us ;
						d[ dpitch + 0 ] = *us ;

						d[          2 ] = *vs ;
						d[ dpitch + 2 ] = *vs ;

					}
				}
			}
		}

		// ロックを解除
		Direct3DSurface9_UnlockRect_ASync( DT->PF->D3D9.YUVSurface, ASyncThread ) ;

		// セットアップフラグを立てる
		DT->YUVImageSetup = 1 ;
	}

	// 正常終了
	return 0 ;
}

// 一時バッファの YUV フォーマットのテクスチャを得る
extern	const void *TheoraDecode_D3D9_GetYUVImage_PF( DECODE_THEORA *DT )
{
	// アドレスを返す
	return DT->PF->D3D9.YUVSurface ;
}




//}

#endif // #if !defined( DX_NON_OGGVORBIS ) || !defined( DX_NON_OGGTHEORA )

#endif // DX_NON_DIRECT3D9

