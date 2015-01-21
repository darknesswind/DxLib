// -------------------------------------------------------------------------------
// 
// 		ＤＸLibrary		Font处理用ProgramHeaderFile
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXFONT_H__
#define __DXFONT_H__

#include "DxCompileConfig.h"

#ifndef DX_NON_FONT

// Include ------------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxMemImg.h"
#include "DxHandle.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// CreateFontToHandle で使用されるグローバル変数を纏めたもの
struct CREATEFONTTOHANDLE_GPARAM
{
	int						NotTextureFlag ;					// 作成するフォントキャッシュをテクスチャにするか、フラグ
	int						TextureCacheColorBitDepth16Flag ;	// 作成するフォントキャッシュテクスチャのカラービット数を16ビットにするかどうかのフラグ
	int						CacheCharNum ;						// フォントキャッシュでキャッシュできる文字の数
	int						UsePremulAlphaFlag ;				// 乗算済みαを使用するかどうかのフラグ( TRUE:使用する  FALSE:使用しない )
} ;

// フォント１文字のキャッシュ情報
struct FONTDATA
{
	BYTE					ValidFlag ;			// 有効フラグ
	BYTE					WCharaFlag ;		// ワイド文字フラグ
	short					DrawX, DrawY ;		// 文字画像を描画すべき座標
	short					AddX ;				// 次の文字を描画すべき相対座標
	int						GraphIndex ;		// 文字データのインデックスナンバー
	short					SizeX, SizeY ;		// 文字のサイズ
	struct FONTCODEDATA *	CodeData ;			// このフォントを管理しているデータのポインタ
} ;

// フォントキャッシュ存在フラグ構造体
struct FONTCODEDATA
{
	int						ExistFlag ;			// キャッシュの存在フラグ
	FONTDATA *				DataPoint ;			// キャッシュデータの存在位置
} ;

// フォントキャッシュの管理データ
struct FONTMANAGE
{
	HANDLEINFO				HandleInfo ;						// ハンドル共通データ

	int						UseTextOut ;						// TextOut を使用するタイプのフォントかどうか( TRUE:TextOutを使用する  FALSE:GetGlyphOutline を使用する )

	HBITMAP					CacheBitmap ;						// テキストキャッシュ用ビットマップ
	unsigned char			*CacheBitmapMem ;					// テキストキャッシュ用ビットマップのメモリ
	int						CacheBitmapMemPitch ;				// テキストキャッシュ用ビットマップのピッチ

	int						*LostFlag ;							// 解放時に TRUE にするフラグへのポインタ

	FONTDATA				FontData[ FONT_CACHE_MAXNUM + 1 ];	// キャッシュデータ
	FONTCODEDATA			FontCodeData[ 0x10000 ] ;			// 存在データを含むデータ配列
	unsigned int			Index ;								// 次データ追加時の配列インデックスナンバー
	int						MaxCacheCharNum ;					// キャッシュできる最大数
	unsigned char			*CacheMem ;							// テキストキャッシュメモリ
	int						CachePitch ;						// テキストキャッシュメモリのピッチ	
	int						CacheDataBitNum ;					// テキストキャッシュ上の１ピクセルのビット数

	TCHAR					FontName[ 256 ] ;					// フォントの名前
	int						FontThickness ;						// フォントの太さ
	int						FontSize ;							// 描画するフォントのサイズ
	int						FontHeight ;						// 描画するフォントの最大縦幅
	int						FontAddHeight ;						// サイズ調整の為に足した高さ
	int						Italic ;							// イタリックフォントかどうか(TRUE:イタリック FALSE:非イタリック)
	int						Space ;								// 次の文字を表示する座標に加算ドット数
	int						MaxWidth ;							// フォントの最大幅
	HFONT					FontObj ;							// フォントのオブジェクトデータ

	int						FontType ;							// フォントのタイプ
	int						CharSet ;							// キャラクタセット
	int						EdgeSize ;							// エッジの太さ

	SIZE					SurfaceSize ;						// テキストサーフェスのサイズ 
	int						LengthCharNum ;						// サーフェスの縦に並べられる文字の数

	int						TextureCache ;						// テクスチャにキャッシュする場合のテクスチャグラフィックハンドル
	int						TextureCacheSub ;					// テクスチャにキャッシュする場合のテクスチャグラフィックハンドル(縁用)
	int						TextureCacheLostFlag ;				// TextureCache が無効になったときに TRUE になる変数
//	BASEIMAGE				TextureTempCache ;					// テクスチャに転送する前に一時的にグラフィックデータを保存するイメージデータ
	BASEIMAGE				TextureCacheBaseImage ;				// TextureCache に転送したものと同じものを格納した基本イメージ
	int						TextureCacheColorBitDepth ;			// テクスチャキャッシュのカラービット数
	int						TextureCacheUsePremulAlpha ;		// テクスチャキャッシュのαチャンネルを乗算済みαにするかどうか

	int						TextureCacheFlag ;					// テクスチャキャッシュをしているか、フラグ
} ;

// フォントシステム用構造体
struct FONTSYSTEM
{
	int						InitializeFlag ;					// 初期化フラグ

	int						NotTextureFlag ;					// 作成するフォントキャッシュをテクスチャにするか、フラグ
	int						TextureCacheColorBitDepth16Flag ;	// 作成するフォントキャッシュテクスチャのカラービット数を16ビットにするかどうかのフラグ
	int						CacheCharNum ;						// フォントキャッシュでキャッシュできる文字の数
	int						UsePremulAlphaFlag ;				// 乗算済みαを使用するかどうかのフラグ( TRUE:使用する  FALSE:使用しない )
	BYTE					BitCountTable[ 256 ] ;				// ビットカウントテーブル

	int						DefaultFontHandle ;					// デフォルトで使用するフォントのハンドル
	int						EnableInitDefaultFontName ;			// デフォルトで使用するフォントの設定
	TCHAR					DefaultFontName[ 128 ] ;
	int						EnableInitDefaultFontSize ;
	int						DefaultFontSize ;
	int						EnableInitDefaultFontThick ;
	int						DefaultFontThick ;
	int						EnableInitDefaultFontType ;
	int						DefaultFontType ;
	int						EnableInitDefaultFontCharSet ;
	int						DefaultFontCharSet ;
	int						EnableInitDefaultFontEdgeSize ;
	int						DefaultFontEdgeSize ;
	int						EnableInitDefaultFontItalic ;
	int						DefaultFontItalic ;
	int						EnableInitDefaultFontSpace ;
	int						DefaultFontSpace ;
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	int			InitFontManage( void ) ;																					// フォントシステムの初期化
extern	int			TermFontManage( void ) ;																					// フォント制御の終了
extern	int			InitCacheFontToHandle( void ) ;																				// フォントのキャッシュ情報を初期化する
extern	int			InitFontCacheToHandle( int FontHandle, int ASyncThread = FALSE ) ;											// 特定のフォントのキャッシュ情報を初期化する

extern	int			InitializeFontHandle( HANDLEINFO *HandleInfo ) ;															// フォントハンドルを初期化する
extern	int			TerminateFontHandle( HANDLEINFO *HandleInfo ) ;																// フォントハンドルの後始末をする

extern	int			RefreshFontDrawResourceToHandle( int FontHandle, int ASyncThread = FALSE ) ;								// フォントハンドルが使用する描画バッファやテクスチャキャッシュを再初期化する
extern	int			FontCacheStringDrawToHandleST(
							int DrawFlag, int xi, int yi, float xf, float yf, int PosIntFlag,
							bool ExRateValidFlag, double ExRateX, double ExRateY,
							const TCHAR *StrData,
							int Color, MEMIMG *DestMemImg, const RECT *ClipRect,
							int TransFlag, int FontHandle, int EdgeColor,
							int StrLen, int VerticalFlag, SIZE *DrawSize ) ;
extern	int			RefreshDefaultFont( void ) ;																				// デフォルトフォントを再作成する
extern	int			InitFontToHandleBase( int Terminate = FALSE ) ;																// InitFontToHandle の内部関数
/*
extern	int			FontCacheStringDrawToHandleST(  int DrawFlag, int x, int y, const char *StrData, int Color, void *DSuf,
													void *ASuf, int APitch, RECT *ClipRect, int ColorBitDepth ,
													int TransFlag, int FontHandle, int EdgeColor, int StrLen, int VerticalFlag,
													SIZE *DrawSize ) ;															// 文字列の描画（キャッシュ使用版）
*/

extern	FONTMANAGE *GetFontManageDataToHandle( int FontHandle ) ;																// フォント管理データの取得

extern	void		InitCreateFontToHandleGParam( CREATEFONTTOHANDLE_GPARAM *GParam ) ;											// CREATEFONTTOHANDLE_GPARAM のデータをセットする

extern	int			CreateFontToHandle_UseGParam( CREATEFONTTOHANDLE_GPARAM *GParam, const TCHAR *FontName, int Size, int Thick, int FontType, int CharSet, int EdgeSize, int Italic, int Handle, int ASyncLoadFlag = FALSE ) ;			// CreateFontToHandle のグローバル変数にアクセスしないバージョン

}

#endif // DX_NON_FONT

#endif // __DXFONT_H__
