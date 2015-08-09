﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		WindowsOS用Ogg関係ヘッダファイル
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

#ifndef __DXUSECLIBOGGWIN_H__
#define __DXUSECLIBOGGWIN_H__

// インクルード ------------------------------------------------------------------
#include "../DxCompileConfig.h"
#include "../DxLib.h"
#include "DxUseCLibOggD3D9.h"
#include "DxUseCLibOggD3D11.h"

#if !defined( DX_NON_OGGVORBIS ) || !defined( DX_NON_OGGTHEORA )

//namespace DxLib
//{

// マクロ定義 --------------------------------------------------------------------

// 構造体定義 --------------------------------------------------------------------

// Ogg Theora デコード処理用環境依存データ構造体
struct DECODE_THEORA_PF
{
	union
	{
		int									Dummy ;
#ifndef DX_NON_DIRECT3D11
		DECODE_THEORA_DIRECT3D11			D3D11 ;					// Direct3D11用 Ogg Theora デコード処理用データ構造体
#endif // DX_NON_DIRECT3D11
#ifndef DX_NON_DIRECT3D9
		DECODE_THEORA_DIRECT3D9				D3D9 ;					// Direct3D9用 Ogg Theora デコード処理用データ構造体
#endif // DX_NON_DIRECT3D9
	} ;
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

//}

#endif // #if !defined( DX_NON_OGGVORBIS ) || !defined( DX_NON_OGGTHEORA )

#endif // __DXUSECLIBOGGWIN_H__
