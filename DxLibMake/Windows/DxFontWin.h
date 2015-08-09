﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用フォント関係プログラムヘッダファイル
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

#ifndef __DXFONTWIN_H__
#define __DXFONTWIN_H__

// インクルード ------------------------------------------------------------------
#include "../DxCompileConfig.h"
#include "../DxFile.h"

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

// 構造体定義 --------------------------------------------------------------------

// Windows用 フォントキャッシュの環境依存管理データ
struct FONTMANAGE_PF
{
	int						UseTextOut ;						// TextOut を使用するタイプのフォントかどうか( TRUE:TextOutを使用する  FALSE:GetGlyphOutline を使用する )

	HBITMAP					CacheBitmap ;						// テキストキャッシュ用ビットマップ
	unsigned char *			CacheBitmapMem ;					// テキストキャッシュ用ビットマップのメモリ
	DWORD					CacheBitmapMemPitch ;				// テキストキャッシュ用ビットマップのピッチ

	HFONT					FontObj ;							// フォントのオブジェクトデータ

	void *					GetGlyphOutlineBuffer ;				// GetGlyphOutline のデータ取得用に使用するバッファ
	DWORD					GetGlyphOutlineBufferSize ;			// GetGlyphOutline のデータ取得用に使用するバッファのサイズ
} ;

// Windows用 フォントシステム用構造体
struct FONTSYSTEM_WIN
{
	HDC						Devicecontext ;						// FontCacheCharAddToHandle の処理で使用するデバイスコンテキスト
	HFONT					OldFont ;							// FontCacheCharAddToHandle の処理で使用するフォント
	HBITMAP					OldBitmap ;							// FontCacheCharAddToHandle の処理で使用するビットマップ
	TEXTMETRIC				TextMetric ;						// FontCacheCharAddToHandle の処理で使用するフォントの情報
} ;

// 内部大域変数宣言 --------------------------------------------------------------

extern FONTSYSTEM_WIN FontSystem_Win ;

// 関数プロトタイプ宣言-----------------------------------------------------------

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXFONTWIN_H__
