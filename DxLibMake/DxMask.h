// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		マスク処理ヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXMASK_H__
#define __DXMASK_H__

#include "DxCompileConfig.h"

#ifndef DX_NON_MASK

// Include ------------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxGraphicsAPI.h"
#include "DxMemImg.h"
#include "DxHandle.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// マスクデータ用構造体
struct MASKDATA
{
	HANDLEINFO				HandleInfo ;							// ハンドル共通情報

	BYTE					*SrcData ;								// マスクソースデータ
	int						SrcDataPitch ;							// ソースデータピッチ
	int						MaskWidth, MaskHeight ;					// マスクの幅と高さ
	int						ValidFlag ;								// 有効フラグ
	int						TransMode ;								// 透過色処理モード
} ;

// マスクデータ管理用構造体
struct MASKMANAGEDATA
{
	int						InitializeFlag ;						// 初期化フラグ

	int						MaskBufferSizeX, MaskBufferSizeY ;		// マスクデータバッファのサイズ
	BYTE					*MaskBuffer ;							// マスクデータバッファ
	int						MaskBufferPitch ;						// マスクデータバッファのピッチ

	int						MaskReverseEffectFlag ;					// マスクの数値に対する効果を逆転させる
	int						FullScreenMaskUpdateFlag ;				// 全画面マスク更新を行っているかのフラグ
	int						CreateMaskFlag ;						// マスクサーフェスが作成されているか、のフラグ
	int						MaskUseFlag ;							// マスクを使用するか、フラグ
	int						MaskValidFlag ;							// MaskUseFlag と CreateMaskFlag を & したもの

	int						MaskBeginFunctionCount ;				// マスクを使用した描画の前に呼ぶ関数が呼ばれた回数

	int						ValidMaskDrawMemImg ;					// MaskDrawMemImg が有効かどうか、のフラグ
	MEMIMG					MaskDrawMemImg ;						// マスク使用時の描画先サーフェスの代わりの MEMIMG

	// DirectX バージョン依存定義
	int						MaskTextureSizeX, MaskTextureSizeY ;	// マスクテクスチャのサイズ
	DX_DIRECT3DTEXTURE9		*MaskImageTexture ;						// マスクイメージテクスチャ
	DX_DIRECT3DTEXTURE9		*MaskScreenTexture ;					// マスクスクリーンテクスチャ
	DX_DIRECT3DSURFACE9		*MaskScreenSurface ;					// マスクスクリーンサーフェス

} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// マスク関係の情報
extern MASKMANAGEDATA MaskManageData ;


// 関数プロトタイプ宣言-----------------------------------------------------------

// DxMask.cpp 関連
extern	int			InitializeMask( void ) ;																				// マスク処理の初期化
extern	int			TerminateMask( void ) ;																					// マスク処理の後始末

extern	int			ReleaseMaskSurface( void ) ;																			// マスクスクリーンを一時削除する
extern	int			ReCreateMaskSurface( void ) ;																			// マスクスクリーンを復旧させる
extern	int			CreateMaskScreenFunction( int MaskSurfaceFlag, int Width, int Height ) ;									// マスクスクリーンを作成する関数
extern	int			SetGraphToMask( RECT MaskRect, DX_DIRECTDRAWSURFACE7 *SrcSurface, BASEIMAGE *SrcImage ) ;				// マスクデータサーフェスにグラフィックをロードする
extern	int			OutGraphToMask( RECT MaskRect, DX_DIRECTDRAWSURFACE7 *DestSurface, BASEIMAGE *DestImage ) ;				// マスキングを行って指定サーフェスに転送
extern	int			CreateMaskOn2D( int DrawPointX, int DrawPointY, int DestWidth, int DestHeight, void *DestBufP ,
								   int DestPitch, int DestBitDepth, int SrcWidth, int SrcHeight, void *SrcData ) ;			// マスクパターンの作成

extern	int			CreateMaskSurface( BYTE **MaskBuffer, int *BufferPitch, int Width, int Height, int *TransModeP ) ;		// マスクサーフェスの作成

extern	int			MaskDrawBeginFunction( RECT Rect ) ;																	// マスクを使用した描画の前に呼ぶ関数
extern	int			MaskDrawAfterFunction( RECT Rect ) ;																	// マスクを使用した描画の後に呼ぶ関数

extern	int			InitializeMaskHandle( HANDLEINFO *HandleInfo ) ;														// マスクハンドルの初期化
extern	int			TerminateMaskHandle( HANDLEINFO *HandleInfo ) ;															// マスクハンドルの後始末

extern	int			MakeMask_UseGParam( int Width, int Height, int ASyncLoadFlag = FALSE ) ;								// マスクデータの追加
extern	int			LoadMask_UseGParam( const TCHAR *FileName, int ASyncLoadFlag = FALSE ) ;								// マスクデータをロードする 
extern	int			LoadDivMask_UseGParam( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf, int ASyncLoadFlag = FALSE ) ;	// マスクを画像から分割読みこみ

}

#endif //	DX_NON_MASK

#endif // __DXMASK_H__
