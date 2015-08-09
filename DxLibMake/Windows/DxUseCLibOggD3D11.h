﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Ogg関係ヘッダファイル( Direct3D11 )
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

#ifndef __DXUSECLIBOGGD3D11_H__
#define __DXUSECLIBOGGD3D11_H__

#include "../DxCompileConfig.h"

#ifndef DX_NON_DIRECT3D11

// インクルード ------------------------------------------------------------------
#include "../DxLib.h"
#include "../DxUseCLibOgg.h"

#if !defined( DX_NON_OGGVORBIS ) || !defined( DX_NON_OGGTHEORA )

//namespace DxLib
//{

// マクロ定義 --------------------------------------------------------------------

// 構造体定義 --------------------------------------------------------------------

// Direct3D11用 Ogg Theora デコード処理用環境依存データ構造体
struct DECODE_THEORA_DIRECT3D11
{
	int Dummy ;
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

// 環境依存関数
extern	int		TheoraDecode_D3D11_CreateSurface_PF(	 DECODE_THEORA *DT ) ;																// ハードウエア機能が有効な場合は YUV フォーマットの一時保存用テクスチャを作成する
extern	void	TheoraDecode_D3D11_ReleaseSurface_PF( DECODE_THEORA *DT ) ;
extern	int		TheoraDecode_D3D11_SetupImage_PF( DECODE_THEORA *DT, volatile THEORA_STOCKFRAME *Stock, int ASyncThread ) ;				// カレントフレームのRGB画像を作成する( 戻り値  1:作成された  0:されなかった )
extern	const void *TheoraDecode_D3D11_GetYUVImage_PF( DECODE_THEORA *DT ) ;												// 一時バッファの YUV フォーマットのテクスチャを得る

//}

#endif // #if !defined( DX_NON_OGGVORBIS ) || !defined( DX_NON_OGGTHEORA )

#endif // DX_NON_DIRECT3D11

#endif // __DXUSECLIBOGGD3D11_H__
