// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ＢａｓｅＩｍａｇｅプログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXBASEIMAGE_H__
#define __DXBASEIMAGE_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxThread.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// キューブマップの面の数
#define CUBEMAP_SURFACE_NUM					(6)

#define BASEIM								BaseImageManage

// 结构体定义 --------------------------------------------------------------------

// 汎用画像読み込みに必要なグローバルデータを集めた構造体
struct CREATEBASEIMAGETYPE2_GPARAM
{
	int						( *UserImageLoadFunc4[ MAX_USERIMAGEREAD_FUNCNUM ] )( STREAMDATA *Src, BASEIMAGE *Image ) ;	// ユーザー画像読み込み関数Ver4
	int						UserImageLoadFuncNum4 ;																		// ユーザー画像読み込み関数Ver4の数
} ;

// 画像の読み込みに必要なグローバルデータを纏めた構造体
struct LOADBASEIMAGE_GPARAM
{
	CREATEBASEIMAGETYPE2_GPARAM CreateGraphImageType2GParam ;		// CreateGraphImageType2_UseGParam で使用するデータ

	STREAMDATASHREDTYPE2	StreamDataShred2 ;						// ストリームデータアクセス用関数２
	STREAMDATASHRED			FileStreamDataShred ;					// ファイルデータアクセス用関数
	STREAMDATASHRED			MemStreamDataShred ;					// メモリデータアクセス用関数

	int						ConvertPremultipliedAlpha ;				// 読み込み処理後に乗算済みアルファの画像に変換するかどうかのフラグ( TRUE:変換処理を行う  FALSE:変換処理を行わない )
	int						AlphaTestImageCreateFlag ;				// アルファテストを使用するグラフィックを作成するかどうかのフラグ( TRUE:αテストを使用する  FALSE:αテストは使用しない )
	DWORD					TransColor ;							// 透過色( AlphaTestImageCreateFlag が TRUE の場合用 )
} ;

// BASEIMAGE + DIB 関係の情報構造体
struct BASEIMAGEMANAGE
{
	int						( *UserImageLoadFunc4[ MAX_USERIMAGEREAD_FUNCNUM ] )( STREAMDATA *Src, BASEIMAGE *Image ) ;																									// ユーザー画像読み込み関数Ver4
	int						UserImageLoadFuncNum4 ;																																										// ユーザー画像読み込み関数Ver4の数
/*
	int						( *UserImageLoadFunc3[ MAX_USERIMAGEREAD_FUNCNUM ] )( void *DataImage, int DataImageSize, int DataImageType, int BmpFlag, BASEIMAGE *Image, BITMAPINFO **BmpInfo, void **GraphData ) ;		// ユーザー画像読み込み関数Ver3
	int						UserImageLoadFuncNum3 ;																																										// ユーザー画像読み込み関数Ver3の数

	int						( *UserImageLoadFunc2[ MAX_USERIMAGEREAD_FUNCNUM ] )( void *Image, int ImageSize, int ImageType, BITMAPINFO **BmpInfo, void **GraphData ) ;													// ユーザー画像読み込み関数Ver2
	int						UserImageLoadFuncNum2 ;																																										// ユーザー画像読み込み関数Ver2の数

	int						( *UserImageLoadFunc[ MAX_USERIMAGEREAD_FUNCNUM ] )( FILE *fp, BITMAPINFO **BmpInfo, void **GraphData ) ;																					// ユーザー画像読み込み関数
	int						UserImageLoadFuncNum ;																																										// ユーザー画像読み込み関数の数
*/

	DX_CRITICAL_SECTION		CriticalProcessHandle ;					// データ処理衝突回避用クリティカルセクション
	int						InitializeFlag ;						// 初期化フラグ

	int						Bmp32AllZeroAlphaToXRGB8 ;				// 32bit bmp 画像のＡ成分がすべて０だったらＡ成分を無視するかどうか
	int						LowLoadFlag ;							// グラフィックの低速読み込みを行うかフラグ
	int						ImageShavedMode ;						// 減色時の拡散パターン
	int						ConvertPremultipliedAlpha ;				// 読み込み処理後に乗算済みアルファの画像に変換するかどうかのフラグ( TRUE:変換処理を行う  FALSE:変換処理を行わない )

	int						AlphaTestImageCreateFlag ;				// アルファテストを使用するグラフィックを作成するかどうかのフラグ( TRUE:αテストを使用する  FALSE:αテストは使用しない )
	DWORD					TransColor ;							// 透過色( AlphaTestImageCreateFlag が TRUE の場合用 )
} ;


// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 基本イメージ管理用データ
extern BASEIMAGEMANAGE BaseImageManage ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// 初期化、後始末
extern	int		InitializeBaseImageManage( void ) ;															// 基本イメージ管理情報の初期化
extern	int		TerminateBaseImageManage( void ) ;															// 基本イメージ管理情報の後始末

// 画像読み込み関数
extern	int		SetBmp32AllZeroAlphaToXRGB8( int Flag ) ;															// 32bit bmp 画像のＡ成分がすべて０だったらＡ成分を無視するかどうかのフラグをセットする

extern	int		ScalingBltBaseImage(
					int SrcX1,  int SrcY1,  int SrcX2,  int SrcY2,  BASEIMAGE *SrcBaseImage,
					int DestX1, int DestY1, int DestX2, int DestY2, BASEIMAGE *DestBaseImage, int Bilinear = TRUE ) ;		// 基本イメージデータを拡大転送する
extern	int		GraphHalfScaleBlt(	const COLORDATA *ColorData,
										      void *DestGraphData, int DestPitch,
										const void *SrcGraphData,  int SrcPitch,
										int DestX, int DestY, int SrcX, int SrcY, int SrcWidth, int SrcHeight ) ;	// ２分の１スケーリングしながらグラフィックデータ間転送を行う、そのため奇数倍数の転送矩形は指定できない
extern	int		NoneMaskFill( RECT *Rect, void *ImageData, int Pitch, COLORDATA *ColorData, unsigned int Fill = 0xff ) ;		// 使われていないビットを指定の値で埋める


extern	int		ConvertYV12ToXRGB32( void *YV12Image, int Width, int Height, BASEIMAGE *DestBaseImage ) ;				// YV12 フォーマットのイメージを XRGB32 のビットマップイメージに変換する
extern	int		ConvertNV11ToXRGB32( void *NV11Image, int Width, int Height, BASEIMAGE *DestBaseImage ) ;				// NV11 フォーマットのイメージを XRGB32 のビットマップイメージに変換する
extern	int		ConvertNV12ToXRGB32( void *NV12Image, int Width, int Height, BASEIMAGE *DestBaseImage ) ;				// NV12 フォーマットのイメージを XRGB32 のビットマップイメージに変換する
extern	int		ConvertYUY2ToXRGB32( void *YUY2Image, int Width, int Height, BASEIMAGE *DestBaseImage ) ;				// YUY2 フォーマットのイメージを XRGB32 のビットマップイメージに変換する
extern	int		ConvertUYVYToXRGB32( void *UYVYImage, int Width, int Height, BASEIMAGE *DestBaseImage ) ;				// UYVY フォーマットのイメージを XRGB32 のビットマップイメージに変換する
extern	int		ConvertYVYUToXRGB32( void *YVYUImage, int Width, int Height, BASEIMAGE *DestBaseImage ) ;				// YVYU フォーマットのイメージを XRGB32 のビットマップイメージに変換する

extern	void	InitCreateBaseImageType2GParam( CREATEBASEIMAGETYPE2_GPARAM *GParam ) ;											// CREATEBASEIMAGETYPE2_GPARAM のデータをセットする
extern	void	InitLoadBaseImageGParam( LOADBASEIMAGE_GPARAM *GParam ) ;														// LOADBASEIMAGE_GPARAM のデータをセットする

extern	int		CreateGraphImageType2_UseGParam( CREATEBASEIMAGETYPE2_GPARAM *GParam, STREAMDATA *Src, BASEIMAGE *Dest ) ;																																													// CreateGraphImageType2 のグローバル変数にアクセスしないバージョン
extern	int		CreateGraphImageOrDIBGraph_UseGParam( LOADBASEIMAGE_GPARAM *GParam, const TCHAR *FileName, const void *DataImage, int DataImageSize, int DataImageType, int BmpFlag, int ReverseFlag,
														BASEIMAGE *Image, BITMAPINFO **BmpInfo, void **GraphData ) ;																																														// CreateGraphImageOrDIBGraph のグローバル変数にアクセスしないバージョン
extern	int		CreateGraphImage_plus_Alpha_UseGParam( LOADBASEIMAGE_GPARAM *GParam, const TCHAR *FileName, const void *RgbImage, int RgbImageSize, int RgbImageType,
													const void *AlphaImage, int AlphaImageSize, int AlphaImageType,
													BASEIMAGE *RgbGraphImage, BASEIMAGE *AlphaGraphImage, int ReverseFlag ) ;																																												// CreateGraphImage_plus_Alpha のグローバル変数にアクセスしないバージョン

}

#endif // __DXBASEIMAGE_H__
