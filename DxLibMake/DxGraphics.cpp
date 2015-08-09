﻿// ----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		描画プログラム
// 
// 				Ver 3.14f
// 
// ----------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード----------------------------------------------------------------
#include "DxGraphics.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxFile.h"
#include "DxSound.h"
#include "DxGraphicsFilter.h"
#include "DxMask.h"
#include "DxMemImg.h"
#include "DxMath.h"
#include "DxFont.h"
#include "DxLog.h"
#include "DxASyncLoad.h"
#include "DxUseCLib.h"
#include "DxSoftImage.h"

#include "DxModel.h"

#include "DxBaseImage.h"
#include "DxMovie.h"
#include "DxInput.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxMemory.h"

#ifdef __WINDOWS__
#include "Windows/DxGraphicsWin.h"
#endif // __WINDOWS__

#ifdef __PSVITA
#include "PSVita/DxGraphicsPSVita.h"
#endif // __PSVITA

#ifdef __PS4
#include "PS4/DxGraphicsPS4.h"
#endif // __PS4


#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 -----------------------------------------------------------------

// クリッピングフラグ
#define CLIP_XP							(0x01)				// X+方向にクリップ
#define CLIP_XM							(0x02)				// X-方向にクリップ
#define CLIP_YP							(0x04)				// Y+方向にクリップ
#define CLIP_YM							(0x08)				// Y-方向にクリップ
#define CLIP_ZP							(0x10)				// Z+方向にクリップ
#define CLIP_ZM							(0x20)				// Z-方向にクリップ

// シャドウマップの初期深度補正値
#define DEFAULT_SHADOWMAP_ADJUST_DEPTH		( 0.002f )
#define DEFAULT_SHADOWMAP_GRADATION_RANGE	( 0.0007f )
#define DEFAULT_SHADOWMAP_BLUR_PARAM		( 0 )

// テクスチャピクセルフォーマットのビットデプスインデックス定義
#define TEX_BITDEPTH_16				(0)
#define TEX_BITDEPTH_32 			(1)
#define PIXELBYTE( x )				( x == TEX_BITDEPTH_16 ? 2 : 4 )

// テクスチャーの分割有効時の最大サイズと最小サイズ
#define MIN_TEXTURE_SIZE			(1)

// パレットがある画像の有効性チェック
#define PALETTEIMAGECHK( HAND, MEMIMG )														\
{																							\
	IMAGEDATA *Image ;																	\
																							\
	if( GRAPHCHK( ( HAND ), Image ) )														\
		return -1 ;																			\
	if( Image->Orig->FormatDesc.TextureFlag ) return -1 ;									\
	MEMIMG = &Image->Soft.MemImg;															\
	if( MEMIMG->Base->UsePalette == 0 ) return -1;											\
}

// ライトハンドルの有効性チェック
#define LIGHTCHK( HAND, LPOINT, ERR )										\
	if( ( ( (HAND) & DX_HANDLEERROR_MASK ) != 0 ) ||											\
		( ( (HAND) & DX_HANDLETYPE_MASK ) != DX_HANDLETYPE_MASK_LIGHT ) ||							\
		( ( (HAND) & DX_HANDLEINDEX_MASK ) >= MAX_LIGHT_NUM ) ||								\
		( ( LPOINT = GSYS.Light.Data[ (HAND) & DX_HANDLEINDEX_MASK ] ) == NULL ) ||				\
		( (int)( (LPOINT)->ID << DX_HANDLECHECK_ADDRESS ) != ( (HAND) & DX_HANDLECHECK_MASK ) ) )\
	{																							\
		return (ERR) ;																			\
	}

// 読み込み時の透過色変更処理
#define LUTRANS_START( Img )	\
	int _TransColor = GSYS.CreateImage.TransColor ; \
	if( GSYS.CreateImage.LeftUpColorIsTransColorFlag == TRUE )\
	{\
		GSYS.CreateImage.TransColor = NS_GetGraphImageFullColorCode( Img, 0, 0 ) & 0xffffff ;\
		BASEIM.TransColor = GSYS.CreateImage.TransColor ;\
	}

// 読み込み時の透過色変更処理
#define LUTRANS_GPARAM( GParam, Img )	\
	if( GParam->LeftUpColorIsTransColorFlag == TRUE ) GParam->InitHandleGParam.TransColor = NS_GetGraphImageFullColorCode( Img, 0, 0 ) & 0xffffff ;

#define LUTRANS_RE_START( Img, GrH )		\
	LUTRANS_START( Img ) \

#define LUTRANS_RE_GPARAM( GParam, Img, GrH )		\
	LUTRANS_GPARAM( GParam, Img ) \

#define LUTRANS_RE_DIV_START( Img, GrH, Num )	\
	LUTRANS_START( Img ) \

#define LUTRANS_RE_DIV_GPARAM( GParam, Img, GrH, Num )	\
	LUTRANS_GPARAM( GParam, Img ) \

#define LUTRANS_END		\
	GSYS.CreateImage.TransColor = _TransColor ;	\
	BASEIM.TransColor = _TransColor ;

// 構造体宣言 -----------------------------------------------------------------

// 画像読み込み処理に必要な変数を纏めたもの
struct CREATEGRAPH_LOADBASEIMAGE_PARAM
{
	int						UseTempBaseImage ;
	int						LoadHr ;
	BASEIMAGE				TempRgbBaseImage ;
	BASEIMAGE				TempAlphaBaseImage ;
	BASEIMAGE *				UseRgbBaseImage ;
	BASEIMAGE *				UseAlphaBaseImage ;
} ;

// 値の大きい座標値を使った描画による計算精度の低下を抑える処理で行う行列設定で必要な情報
struct LARGE3DPOSITIONSUPPORT_DRAWINFO
{
	MATRIX_D				BackupWorldMatrix ;
	MATRIX_D				BackupViewMatrix ;
} ;

// データ定義 -----------------------------------------------------------------

// 8x8 の tgaテクスチャ
static BYTE WhiteTga8x8TextureFileImage[ 84 ] = 
{
	0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x08, 0x00,
	0x20, 0x08, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF,
	0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF,
	0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x54, 0x52, 0x55, 0x45, 0x56, 0x49, 0x53, 0x49, 0x4F, 0x4E, 0x2D, 0x58, 0x46, 0x49,
	0x4C, 0x45, 0x2E, 0x00
} ;

// 描画周りの基本的な情報
GRAPHICSSYSTEMDATA GraphicsSysData ;

// 関数宣言 -------------------------------------------------------------------

// 画像関係関数
__inline static int		Graphics_Image_CheckBlendGraphSize( IMAGEDATA *GraphData ) ;		// ブレンド画像との大きさチェック
		 static void	Graphics_Image_DefaultRestoreGraphFunction( void ) ;				// デフォルトのグラフィック復旧関数
		 static void	Graphics_Image_CreateGraph_LoadBaseImage(      LOADGRAPH_PARAM *Param, CREATEGRAPH_LOADBASEIMAGE_PARAM *LParam ) ;		// CreateGraph と CreateDivGraph の共通する BASEIMAGE 構築部分を関数化したもの
		 static void	Graphics_Image_CreateGraph_TerminateBaseImage( LOADGRAPH_PARAM *Param, CREATEGRAPH_LOADBASEIMAGE_PARAM *LParam ) ;		// CreateGraph と CreateDivGraph の共通する BASEIMAGE 後始末部分を関数化したもの







// 画面関係関数
static int Graphics_Screen_CheckDisplaySetting( int ScreenSizeX, int ScreenSizeY, int ColorBitDepth ) ;		// 指定の解像度が対応しているかどうかを調べる関数







// 描画関係関数
__inline static void	Graphics_Draw_VectorTransformToBillboard( VECTOR *Vector, VECTOR *Position ) ;										// ビルボード用の行列計算
		 static int		Graphics_Draw_ModiGraphBase( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int GrHandle, int TransFlag, bool SimpleDrawFlag ) ;
		 static int		Graphics_Draw_ModiGraphFBase( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int GrHandle, int TransFlag, bool SimpleDrawFlag ) ;
		 static void	Graphics_Draw_BeginLarge3DPositionSupportDraw( LARGE3DPOSITIONSUPPORT_DRAWINFO *DrawInfo, VECTOR_D BasePosition ) ;	// 値の大きい座標値を使った描画による計算精度の低下を抑える為の処理を行う
		 static void	Graphics_Draw_EndLarge3DPositionSupportDraw( const LARGE3DPOSITIONSUPPORT_DRAWINFO *DrawInfo ) ;					// 値の大きい座標値を使った描画による計算精度の低下を抑える為の処理で行った変更を元に戻す








// ハードウェアアクセラレータ使用レンダリング関係関数
static int	Graphics_Hardware_DrawChipMap( int Sx, int Sy, int XNum, int YNum, const int *MapData, int MapDataPitch, int ChipTypeNum, const int *ChipGrHandle, int TransFlag ) ;	// ハードウエアアクセラレータ使用版 DrawChipMap
static int	Graphics_Hardware_DrawTile( int x1, int y1, int x2, int y2, int Tx, int Ty, double ExtRate, double Angle, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag ) ;	// ハードウエアアクセラレータ使用版 DrawTile








// ソフトウエアレンダリング関数
static int	Graphics_Software_Initialize( void ) ;																										// ソフトウエアレンダリングの初期化を行う
static int	Graphics_Software_Terminate( void ) ;																										// ソフトウエアレンダリングの後始末を行う
static int  Graphics_Software_DrawGraph(            int x,  int y,                                                                  IMAGEDATA *Image, int TransFlag ) ;						// ソフトウエアレンダリング版 DrawGraph
static int  Graphics_Software_DrawExtendGraph(      int x1, int y1, int x2, int y2,                                                 IMAGEDATA *Image, int TransFlag ) ;						// ソフトウエアレンダリング版 DrawExtendGraph
static int  Graphics_Software_DrawRotaGraph(        float x, float y, double ExRate, double Angle,                                  IMAGEDATA *Image, int TransFlag, int TurnFlag ) ;		// ソフトウエアレンダリング版 DrawRotaGraph
static int  Graphics_Software_DrawPoly(             DX_POINTDATA *p,                                                                IMAGEDATA *Image, int TransFlag, unsigned int Color ) ;
static int  Graphics_Software_DrawModiGraph(        int   x1, int   y1, int   x2, int   y2, int   x3, int   y3, int   x4, int   y4, IMAGEDATA *Image, int TransFlag ) ;						// ソフトウエアレンダリング版 DrawModiGraph
static int  Graphics_Software_DrawModiGraphF(       float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, IMAGEDATA *Image, int TransFlag ) ;						// ソフトウエアレンダリング版 DrawModiGraphF
static int  Graphics_Software_DrawChipMap(          int Sx, int Sy, int XNum, int YNum, const int *MapData, int MapDataPitch, int ChipTypeNum, const int *ChipGrHandle, int TransFlag ) ;	// ソフトウエアレンダリング版 DrawChipMap
static int  Graphics_Software_DrawTile(             int x1, int y1, int x2, int y2, int Tx, int Ty, double ExtRate, double Angle,   IMAGEDATA *Image, int TransFlag ) ;						// ソフトウエアレンダリング版 DrawTile
static int  Graphics_Software_DrawFillBox(          int x1, int y1, int x2, int y2,                                 unsigned int Color ) ;						// ソフトウエアレンダリング版 DrawFillBox
static int  Graphics_Software_DrawLineBox(          int x1, int y1, int x2, int y2,                                 unsigned int Color ) ;						// ソフトウエアレンダリング版 DrawLineBox
static int  Graphics_Software_DrawLine(             int x1, int y1, int x2, int y2,                                 unsigned int Color ) ;						// ソフトウエアレンダリング版 DrawLine
static int  Graphics_Software_DrawCircle_Thickness( int x, int y, int r,                                            unsigned int Color, int Thickness ) ;		// ソフトウエアレンダリング版 DrawCircle( 太さ指定あり )
static int  Graphics_Software_DrawOval_Thickness(   int x, int y, int rx, int ry,                                   unsigned int Color, int Thickness ) ;		// ソフトウエアレンダリング版 DrawOval( 太さ指定あり )
static int  Graphics_Software_DrawCircle(           int x, int y, int r,                                            unsigned int Color, int FillFlag ) ;		// ソフトウエアレンダリング版 DrawCircle
static int  Graphics_Software_DrawOval(             int x, int y, int rx, int ry,                                   unsigned int Color, int FillFlag ) ;		// ソフトウエアレンダリング版 DrawOval
static int  Graphics_Software_DrawTriangle(         int x1, int y1, int x2, int y2, int x3, int y3,                 unsigned int Color, int FillFlag ) ;		// ソフトウエアレンダリング版 DrawTriangle
static int  Graphics_Software_DrawQuadrangle(       int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, int FillFlag ) ;		// ソフトウエアレンダリング版 DrawQuadrangle
static int  Graphics_Software_DrawPixel(            int x, int y,                                                   unsigned int Color ) ;						// ソフトウエアレンダリング版 DrawPixel
static int  Graphics_Software_DrawPixelSet(         const POINTDATA *PointData, int Num ) ;																// ソフトウエアレンダリング版 DrawPixelSet
static int  Graphics_Software_DrawLineSet(          const LINEDATA  *LineData,  int Num ) ;																// ソフトウエアレンダリング版 DrawLineSet

// プログラム -----------------------------------------------------------------

// 画像関係関数

// ブレンド画像との大きさチェック
__inline static int Graphics_Image_CheckBlendGraphSize( IMAGEDATA *GraphData )
{
	IMAGEDATA *BlendData ;

	if( GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendData ) )
		return -1 ;

	// ブレンド画像よりも大きかったらアウト
	return ( ( BlendData->Width  - GraphData->Width  ) |
		     ( BlendData->Height - GraphData->Height ) ) & 0x80000000 ? -1 : 0 ;
}

// デフォルトのグラフィック復旧関数
static void Graphics_Image_DefaultRestoreGraphFunction( void )
{
	IMAGEDATA *Image ;
	IMAGEDATA *FileBackImage ;
	IMAGEDATA *MemBackImage ;
	int         i ;
	int         ImageDataArea ;
	BASEIMAGE   FileRgbImage ;
	BASEIMAGE   FileAlphaImage ;
	int         FileRgbLoad ;
	int         FileAlphaLoad ;
	BASEIMAGE   MemRgbImage ;
	BASEIMAGE   MemAlphaImage ;
	int         MemRgbLoad ;
	int         MemAlphaLoad ;
	int         LoadHr ;

	DXST_ERRORLOG_ADDUTF16LE( "\xb0\x30\xe9\x30\xd5\x30\xa3\x30\xc3\x30\xaf\x30\x92\x30\xa9\x5f\x30\x5e\x57\x30\x7e\x30\x59\x30\x0a\x00\x00"/*@ L"グラフィックを復帰します\n" @*/ ) ;

	// すべてのグラフィックを検証
	ImageDataArea  = HandleManageArray[ DX_HANDLETYPE_GRAPH ].AreaMax ;
	FileBackImage  = NULL ;
	FileRgbLoad    = FALSE ;
	FileAlphaLoad  = FALSE ;
	MemBackImage   = NULL ;
	MemRgbLoad     = FALSE ;
	MemAlphaLoad   = FALSE ;
	if( HandleManageArray[ DX_HANDLETYPE_GRAPH ].InitializeFlag )
	{
		for( i = HandleManageArray[ DX_HANDLETYPE_GRAPH ].AreaMin ; i <= ImageDataArea ; i ++ )
		{
			Image = ( IMAGEDATA * )HandleManageArray[ DX_HANDLETYPE_GRAPH ].Handle[ i ] ;
			if( Image == NULL || Image->ReadBase == NULL ) continue ;

			// ファイルから読み込んだ場合はファイルから画像を復元する
			if( Image->ReadBase->FileName )
			{
				// 一つ前のグラフィックとパスとファイルが同じ場合は読み込みをしない
				if( FileBackImage == NULL ||
					FileRgbLoad == FALSE ||
					FileBackImage->ReadBase->FileName == NULL ||
					_WCSCMP( Image->ReadBase->FileName, FileBackImage->ReadBase->FileName ) != 0 ||
					Image->ReadBase->ReverseFlag != FileBackImage->ReadBase->ReverseFlag )
				{
					// BASEIMAGE の後始末
					if( FileRgbLoad == TRUE )
					{
						NS_ReleaseGraphImage( &FileRgbImage ) ;
						FileRgbLoad = FALSE ;
					}
					if( FileAlphaLoad == TRUE )
					{
						NS_ReleaseGraphImage( &FileAlphaImage ) ;
						FileAlphaLoad = FALSE ;
					}

					// 画像をファイルからロードした場合は画像をロードする
		//			DXST_ERRORLOGFMT_ADDW(( L"i:%d フルパス:%s がロードできませんでした", i, Image->FilePath )) ;
		//			DXST_ERRORLOGFMT_ADDW(( L"ファイル:%s をロードします", Image->FilePath->String )) ;
		//			DXST_ERRORLOGFMT_ADDW(( L"反転フラグ:%d  ファイル:%s をロードします", Image->ReadBase->ReverseFlag, Image->ReadBase->FileName )) ;
					LoadHr = CreateGraphImage_plus_Alpha_WCHAR_T(
								Image->ReadBase->FileName,
								NULL,
								0,
								LOADIMAGE_TYPE_FILE, 
								NULL,
								0,
								LOADIMAGE_TYPE_FILE,
								&FileRgbImage,
								&FileAlphaImage,
								Image->ReadBase->ReverseFlag ) ;
					if( LoadHr == -1 )
					{
						DXST_ERRORLOGFMT_ADDW(( L"Image File Load Error : %s", Image->ReadBase->FileName )) ;
						goto R1 ;
					}

					FileRgbLoad = TRUE ;
					if( LoadHr == 0 ) FileAlphaLoad = TRUE ;
				}

				// グラフィックの転送
		//		DXST_ERRORLOGFMT_ADDW(( L"UseFileX:%d UseFileY:%d", Image->UseFileX, Image->UseFileY )) ;
				Graphics_Image_BltBmpOrGraphImageToGraphBase(
					&FileRgbImage,
					FileAlphaLoad == TRUE ? &FileAlphaImage : NULL,
					Image->UseBaseX,
					Image->UseBaseY,
					Image->HandleInfo.Handle,
					GSYS.CreateImage.NotUseTransColor ? FALSE : TRUE
				) ;
		R1 :
				FileBackImage = Image ;
			}
			else
			// メモリから読み込んだ場合はメモリから画像を復元する
			if( Image->ReadBase->MemImage )
			{
				// 一つ前のグラフィックとメモリアドレスが同じ場合は読み込みをしない
				if( MemBackImage == NULL ||
					MemRgbLoad == FALSE ||
					Image->ReadBase->MemImage != MemBackImage->ReadBase->MemImage ||
					Image->ReadBase->AlphaMemImage != MemBackImage->ReadBase->AlphaMemImage ||
					Image->ReadBase->ReverseFlag != MemBackImage->ReadBase->ReverseFlag )
				{
					// BASEIMAGE の後始末
					if( MemRgbLoad == TRUE )
					{
						NS_ReleaseGraphImage( &MemRgbImage ) ;
						MemRgbLoad = FALSE ;
					}
					if( MemAlphaLoad == TRUE )
					{
						NS_ReleaseGraphImage( &MemAlphaImage ) ;
						MemAlphaLoad = FALSE ;
					}

					// 画像をメモリからロードした場合は画像をロードする
					LoadHr = NS_CreateGraphImage_plus_Alpha(
								NULL,
								Image->ReadBase->MemImage,
								Image->ReadBase->MemImageSize,
								LOADIMAGE_TYPE_MEM,
								Image->ReadBase->AlphaMemImage,
								Image->ReadBase->AlphaMemImageSize,
								LOADIMAGE_TYPE_MEM,
								&MemRgbImage,
								&MemAlphaImage,
								Image->ReadBase->ReverseFlag ) ;
					if( LoadHr == -1 )
					{
						DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe1\x30\xe2\x30\xea\x30\x4b\x30\x89\x30\x6e\x30\x3b\x75\xcf\x50\xc7\x30\xfc\x30\xbf\x30\x6e\x30\xed\x30\xfc\x30\x4c\x30\x67\x30\x4d\x30\x7e\x30\x5b\x30\x93\x30\x67\x30\x57\x30\x5f\x30\x00"/*@ L"メモリからの画像データのローができませんでした" @*/ )) ;
						goto R2 ;
					}

					MemRgbLoad = TRUE ;
					if( LoadHr == 0 ) MemAlphaLoad = TRUE ;
				}

				// グラフィックの転送
				Graphics_Image_BltBmpOrGraphImageToGraphBase( 
					&MemRgbImage,
					MemAlphaLoad == TRUE ? &MemAlphaImage : NULL,
					Image->UseBaseX,
					Image->UseBaseY,
					Image->HandleInfo.Handle,
					GSYS.CreateImage.NotUseTransColor ? FALSE : TRUE
				) ;
		R2 :
				MemBackImage = Image ;
			}
			else
			// BaseImage から読み込んだ場合は BaseImage から画像を復元する
			if( Image->ReadBase->BaseImage )
			{
				// グラフィックの転送
				Graphics_Image_BltBmpOrGraphImageToGraphBase( 
					Image->ReadBase->BaseImage,
					Image->ReadBase->AlphaBaseImage,
					Image->UseBaseX,
					Image->UseBaseY,
					Image->HandleInfo.Handle,
					GSYS.CreateImage.NotUseTransColor ? FALSE : TRUE
				) ;
			}
		}
	}

	// GraphImage の後始末
	if( FileRgbLoad == TRUE )
	{
		NS_ReleaseGraphImage( &FileRgbImage ) ;
		FileRgbLoad = FALSE ;
	}
	if( FileAlphaLoad == TRUE )
	{
		NS_ReleaseGraphImage( &FileAlphaImage ) ;
		FileAlphaLoad = FALSE ;
	}
	if( MemRgbLoad == TRUE )
	{
		NS_ReleaseGraphImage( &MemRgbImage ) ;
		MemRgbLoad = FALSE ;
	}
	if( MemAlphaLoad == TRUE )
	{
		NS_ReleaseGraphImage( &MemAlphaImage ) ;
		MemAlphaLoad = FALSE ;
	}

	DXST_ERRORLOG_ADDUTF16LE( "\xb0\x30\xe9\x30\xd5\x30\xa3\x30\xc3\x30\xaf\x30\x6e\x30\xa9\x5f\x30\x5e\x4c\x30\x8c\x5b\x86\x4e\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"グラフィックの復帰が完了しました\n" @*/ ) ;
}

// CreateGraph と CreateDivGraph の共通する BASEIMAGE 構築部分を関数化したもの
static void Graphics_Image_CreateGraph_LoadBaseImage(
	 LOADGRAPH_PARAM *Param,
	 CREATEGRAPH_LOADBASEIMAGE_PARAM *LParam
 )
{
	LParam->UseTempBaseImage             = FALSE ;
	LParam->LoadHr                       = -1 ;
	LParam->UseRgbBaseImage              = NULL ;
	LParam->UseAlphaBaseImage            = NULL ;
	LParam->TempRgbBaseImage.GraphData   = NULL ;
	LParam->TempAlphaBaseImage.GraphData = NULL ;

	if( Param->FileName != NULL )
	{
		// ファイルから読み込む場合
		LParam->LoadHr = CreateGraphImage_plus_Alpha_UseGParam(
					&Param->GParam.LoadBaseImageGParam,
					Param->FileName,
					NULL,
					0,
					LOADIMAGE_TYPE_FILE,
					NULL,
					0,
					LOADIMAGE_TYPE_FILE,
					&LParam->TempRgbBaseImage,
					Param->GParam.CreateGraphGParam.InitHandleGParam.NotUseAlphaImageLoadFlag ? NULL : &LParam->TempAlphaBaseImage,
					FALSE
				) ;
		LParam->UseRgbBaseImage = &LParam->TempRgbBaseImage ;
		if( LParam->LoadHr == 0 )
		{
			LParam->UseAlphaBaseImage = &LParam->TempAlphaBaseImage ;
		}
		LParam->UseTempBaseImage = TRUE ;
	}
	else
	if( Param->RgbMemImage != NULL )
	{
		// メモリ上のファイルイメージから読み込む場合
		LParam->LoadHr = CreateGraphImage_plus_Alpha_UseGParam(
					&Param->GParam.LoadBaseImageGParam,
					NULL,
					Param->RgbMemImage,
					Param->RgbMemImageSize,
					LOADIMAGE_TYPE_MEM,
					Param->AlphaMemImage,
					Param->AlphaMemImageSize,
					LOADIMAGE_TYPE_MEM,
					&LParam->TempRgbBaseImage,
					&LParam->TempAlphaBaseImage,
					FALSE
				) ;
		LParam->UseRgbBaseImage = &LParam->TempRgbBaseImage ;
		if( LParam->LoadHr == 0 )
		{
			LParam->UseAlphaBaseImage = &LParam->TempAlphaBaseImage ;
		}
		LParam->UseTempBaseImage = TRUE ;
	}
#ifdef __WINDOWS__
	else
	if( Param->RgbBmpInfo != NULL )
	{
		int RgbCopyHr ;
		int AlphaCopyHr ;

		// BMP から読み込む場合
		RgbCopyHr = NS_ConvBitmapToGraphImage( Param->RgbBmpInfo, Param->RgbBmpImage, &LParam->TempRgbBaseImage, TRUE ) ;
		if( RgbCopyHr == -1 )
			return ;

		if( Param->AlphaBmpInfo != NULL && Param->AlphaBmpImage != NULL )
		{
			AlphaCopyHr = NS_ConvBitmapToGraphImage( Param->AlphaBmpInfo, Param->AlphaBmpImage, &LParam->TempAlphaBaseImage, TRUE ) ;
			if( AlphaCopyHr == -1 )
			{
				if( RgbCopyHr == 1 ) NS_ReleaseGraphImage( &LParam->TempRgbBaseImage ) ;
				return ;
			}
			LParam->UseAlphaBaseImage = &LParam->TempAlphaBaseImage ;
		}
		else
		{
			_MEMSET( &LParam->TempAlphaBaseImage, 0, sizeof( LParam->TempAlphaBaseImage ) ) ;
		}
		LParam->UseRgbBaseImage = &LParam->TempRgbBaseImage ;
		LParam->UseTempBaseImage = TRUE ;

		LParam->LoadHr = 1 ;
	}
#endif // __WINDOWS__
	else
	if( Param->RgbBaseImage != NULL )
	{
		// 基本イメージから読み込む場合
		LParam->UseRgbBaseImage   = Param->RgbBaseImage ;
		LParam->UseAlphaBaseImage = Param->AlphaBaseImage ;

		LParam->LoadHr = 1 ;
	}

	if( LParam->LoadHr >= 0 )
	{
		// 反転フラグが立っていたらイメージを反転する
		if( Param->ReverseFlag == TRUE )
		{
			NS_ReverseGraphImage( LParam->UseRgbBaseImage ) ;
			if( LParam->UseAlphaBaseImage != NULL ) NS_ReverseGraphImage( LParam->UseAlphaBaseImage ) ;
		}
	}
}

// CreateGraph と CreateDivGraph の共通する BASEIMAGE 後始末部分を関数化したもの
static void Graphics_Image_CreateGraph_TerminateBaseImage(
	LOADGRAPH_PARAM *Param,
	CREATEGRAPH_LOADBASEIMAGE_PARAM *LParam
)
{
	// 読み込んだグラフィックの後始末をする
	if( LParam->UseTempBaseImage )
	{
		NS_ReleaseGraphImage( &LParam->TempRgbBaseImage ) ;
		NS_ReleaseGraphImage( &LParam->TempAlphaBaseImage ) ;
	}
	else
	{
		// 反転フラグが立っていたらイメージを元に戻す
		if( Param->ReverseFlag == TRUE )
		{
			NS_ReverseGraphImage( LParam->UseRgbBaseImage ) ;
			if( LParam->UseAlphaBaseImage != NULL ) NS_ReverseGraphImage( LParam->UseAlphaBaseImage ) ;
		}
	}
}





















// 画面関係関数

// 指定の解像度が対応しているかどうかを調べる関数
static int Graphics_Screen_CheckDisplaySetting( int ScreenSizeX, int ScreenSizeY, int ColorBitDepth )
{
	int Num, i ;
	DISPLAYMODEDATA Mode ;

	Num = NS_GetDisplayModeNum() ;
	for( i = 0 ; i < Num ; i ++ )
	{
		Mode = NS_GetDisplayMode( i ) ;
		if( Mode.Width         == ScreenSizeX &&
			Mode.Height        == ScreenSizeY &&
			Mode.ColorBitDepth == ColorBitDepth )
		{
			return TRUE ;
		}
	}

	return FALSE ;
}























// 描画関係関数

// ビルボード用の行列計算
__inline static void Graphics_Draw_VectorTransformToBillboard( VECTOR *Vector, VECTOR *Position )
{
	VECTOR invec = *Vector ;
	Vector->x = invec.x * GSYS.DrawSetting.BillboardMatrixF.m[0][0] + invec.y * GSYS.DrawSetting.BillboardMatrixF.m[1][0] + Position->x ;
	Vector->y = invec.x * GSYS.DrawSetting.BillboardMatrixF.m[0][1] + invec.y * GSYS.DrawSetting.BillboardMatrixF.m[1][1] + Position->y ;
	Vector->z = invec.x * GSYS.DrawSetting.BillboardMatrixF.m[0][2] + invec.y * GSYS.DrawSetting.BillboardMatrixF.m[1][2] + Position->z ;
}

#define SETDRAWRECTCODE\
	int x[ 4 ], y[ 4 ], MaxX, MaxY, MinX, MinY ;\
	int i, ix, iy ;\
\
	x[ 0 ] = x1 ; x[ 1 ] = x2 ; x[ 2 ] = x3 ; x[ 3 ] = x4 ;\
	y[ 0 ] = y1 ; y[ 1 ] = y2 ; y[ 2 ] = y3 ; y[ 3 ] = y4 ;\
\
	MaxX = 0 ; MaxY = 0 ;\
	MinX = 0xffffff ; MinY = 0xffffff ;\
\
	for( i = 0 ; i < 4 ; i ++ )\
	{\
		ix = x[ i ] ; iy = y[ i ] ;\
		if( ix > MaxX ) MaxX = ix ; if( iy > MaxY ) MaxY = iy ;\
		if( ix < MinX ) MinX = ix ; if( iy < MinY ) MinY = iy ;\
	}\
\
	SETRECT( DrawRect, MinX, MinY, MaxX, MaxY ) ;\
	DRAWRECT_DRAWAREA_CLIP

static int Graphics_Draw_ModiGraphBase( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int GrHandle, int TransFlag, bool SimpleDrawFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawModiGraph_PF( x1, y1, x2, y2, x4, y4, x3, y3, Image, BlendImage, TransFlag, SimpleDrawFlag ),
		Graphics_Software_DrawModiGraph(    x1, y1, x2, y2, x4, y4, x3, y3, Image,             TransFlag                 ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

#define SETDRAWRECTCODE\
	int x[4], y[4], MaxX, MaxY, MinX, MinY ;\
	int i, ix, iy ;\
\
	x[ 0 ] = _FTOL( x1 ) ; x[ 1 ] = _FTOL( x2 ) ; x[ 2 ] = _FTOL( x3 ) ; x[ 3 ] = _FTOL( x4 ) ;\
	y[ 0 ] = _FTOL( y1 ) ; y[ 1 ] = _FTOL( y2 ) ; y[ 2 ] = _FTOL( y3 ) ; y[ 3 ] = _FTOL( y4 ) ;\
\
	MaxX = 0 ; MaxY = 0 ;\
	MinX = 0xfffffff ; MinY = 0xfffffff ;\
\
	for( i = 0 ; i < 4 ; i ++ )\
	{\
		ix = x[ i ] ; iy = y[ i ] ;\
		if( ix > MaxX ) MaxX = ix ; if( iy > MaxY ) MaxY = iy ;\
		if( ix < MinX ) MinX = ix ; if( iy < MinY ) MinY = iy ;\
	}\
\
	SETRECT( DrawRect, MinX, MinY, MaxX, MaxY ) ;

static int Graphics_Draw_ModiGraphFBase( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int GrHandle, int TransFlag, bool SimpleDrawFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawModiGraphF_PF( x1, y1, x2, y2, x4, y4, x3, y3, Image, BlendImage, TransFlag, SimpleDrawFlag ),
		Graphics_Software_DrawModiGraphF(    x1, y1, x2, y2, x4, y4, x3, y3, Image,             TransFlag                 ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE



// 値の大きい座標値を使った描画による計算精度の低下を抑える為の処理を行う
static void	Graphics_Draw_BeginLarge3DPositionSupportDraw( LARGE3DPOSITIONSUPPORT_DRAWINFO *DrawInfo, VECTOR_D BasePosition )
{
	MATRIX_D TranslateMatrix ;
	MATRIX_D TempWorldMatrix ;
	MATRIX_D IdentityMatrix ;

	DrawInfo->BackupWorldMatrix = GSYS.DrawSetting.WorldMatrix ;
	DrawInfo->BackupViewMatrix  = GSYS.DrawSetting.ViewMatrix ;
	CreateTranslationMatrixD( &TranslateMatrix, BasePosition.x, BasePosition.y, BasePosition.z ) ;
	CreateIdentityMatrixD( &IdentityMatrix ) ;
	CreateMultiplyMatrixD( &TempWorldMatrix, &TranslateMatrix, &GSYS.DrawSetting.WorldViewMatrix ) ;
	NS_SetTransformToWorldD( &TempWorldMatrix ) ;
	NS_SetTransformToViewD( &IdentityMatrix ) ;
}

// 値の大きい座標値を使った描画による計算精度の低下を抑える為の処理で行った変更を元に戻す
static void	Graphics_Draw_EndLarge3DPositionSupportDraw( const LARGE3DPOSITIONSUPPORT_DRAWINFO *DrawInfo )
{
	NS_SetTransformToWorldD( &DrawInfo->BackupWorldMatrix ) ;
	NS_SetTransformToViewD( &DrawInfo->BackupViewMatrix ) ;
}






















// ハードウェアアクセラレータ使用レンダリング関係関数

// ハードウエアアクセラレータ使用版 DrawChipMap
static int	Graphics_Hardware_DrawChipMap( int Sx, int Sy, int XNum, int YNum, const int *MapData, int MapDataPitch, int /*ChipTypeNum*/, const int *ChipGrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int i, j ;
	int x, y, Width, Height ;
	const int *MP ;

	if( Graphics_Hardware_CheckValid_PF() == 0 ) return -1 ;

	// 描画の準備
	if( GRAPHCHK( ChipGrHandle[ 0 ], Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;
	}

	Width = Image->Width ;
	Height = Image->Height ;

	// 普通に for 文でループ描画
	y = Sy ;
	MP = MapData ;
	for( i = 0 ; i < YNum ; i ++, y += Height )
	{
		x = Sx ;
		for( j = 0 ; j < XNum ; j ++, x += Width, MP ++ )
		{
			if( *MP == -1 ) continue ;

			if( GRAPHCHK( ChipGrHandle[ *MP ], Image ) )
				return -1 ;

			Graphics_Hardware_DrawGraph_PF( x, y, ( float )x, ( float )y, Image, BlendImage, TransFlag, TRUE ) ;
		}
		MP += MapDataPitch - XNum ;
	}

	// 終了
	return 0 ;
}

/*
x1,y1,x2,y2　画面中、タイル貼りしたい領域
Tx,Ty　　　　変位　　　　　　　　　　　　元画像中の、どこの座標からタイル貼りを開始するか。
ExtRate　　　画像の拡大率　　　　　　　　これを「1」にすると、元画像のままの大きさでタイル貼りする。
Angle　　　　画像の回転角度　　　　　　　これを「0」にすると、回転なしでタイル貼りする。
GrHandle　　 画像のハンドル
TransFlag　　透過フラグ
*/

// ハードウエアアクセラレータ使用版 DrawTile
static int	Graphics_Hardware_DrawTile( int x1, int y1, int x2, int y2, int Tx, int Ty, double ExtRate, double Angle, IMAGEDATA *Image, IMAGEDATA *BlendImage, int TransFlag )
{
	double MaxLength ;
	float Width, Height, f, Sin, Cos, Sin2, Cos2 ;
	float CenX, CenY, x, y, BaseX, BaseY ;
	float xAddX, xAddY, yAddX, yAddY ;
	int temp, Num, i, j ;
	RECT DrawRect ;
	float FExtRate, FAngle ;

	FExtRate = (float)ExtRate ;
	FAngle = (float)Angle ;

	if( ExtRate <= 0.0 ) ExtRate = 0.0001 ;
	if( x1 > x2 ){ temp = x2 ; x2 = x1 ; x1 = temp ; }
	if( y1 > y2 ){ temp = y2 ; y2 = y1 ; y1 = temp ; }
	Tx %= Image->Width ;
	Ty %= Image->Height ;

	// 進む値を計算
	_SINCOS( FAngle, &Sin, &Cos ) ;
	xAddX = Image->Width * Cos * FExtRate ;
	xAddY = Image->Height * Sin * FExtRate ;

	_SINCOS( FAngle + DX_PI_F / 2.0F, &Sin2, &Cos2 ) ;
	yAddX = Image->Width * Cos2 * FExtRate ;
	yAddY = Image->Height * Sin2 * FExtRate ;

	// 描画幅を得る
	Width = (float)( x2 - x1 ) ;
	Height = (float)( y2 - y1 ) ;

	// 最大長を得る
	MaxLength = _SQRT( Width * Width + Height * Height ) ;

	// 描画個数を得る
	{
		int k ;

		k = Image->Width > Image->Height ? Image->Height : Image->Width ;
		Num = _DTOL( MaxLength / ( k * ExtRate ) + 2 ) ;
		if( Num % 2 == 0 ) Num ++ ;
	}

	// 描画中心点を得る
	CenX = x1 + Width / 2 ;
	CenY = y1 + Height / 2 ;

	// 描画可能範囲を変更する
	DrawRect = GSYS.DrawSetting.DrawArea ;
	NS_SetDrawArea( x1, y1, x2, y2 ) ;

	// 描画開始座標を得る
	BaseX = ( -Tx - ( Num * Image->Width  ) / 2 ) * FExtRate ;
	BaseY = ( -Ty - ( Num * Image->Height ) / 2 ) * FExtRate ;

	f = BaseX * Cos - BaseY * Sin ;
	BaseY = BaseX * Sin + BaseY * Cos + CenY ;
	BaseX = f + CenX ;

	// 描画開始
	for( i = 0 ; i < Num ; i ++, BaseX += yAddX, BaseY += yAddY )
	{
		x = BaseX ;
		y = BaseY ;
		for( j = 0 ; j < Num ; j ++, x += xAddX, y += xAddY )
		{
			Graphics_Hardware_DrawModiGraphF_PF(
				x, y,
				x + xAddX, y + xAddY,
				x + yAddX, y + yAddY,
				x + xAddX + yAddX, y + xAddY + yAddY,
				Image, BlendImage, TransFlag, true ) ;
		}
	}

	// 描画可能範囲を元に戻す
	NS_SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;

	return 0 ;
}

























// ソフトウエアレンダリング関数

// ソフトウエアレンダリングの初期化を行う
static int Graphics_Software_Initialize( void )
{
	// メイン画面用 MEMIMG の作成
	if( MakeMemImgScreen(
		&GSYS.SoftRender.MainBufferMemImg,
		GSYS.Screen.MainScreenSizeX,
		GSYS.Screen.MainScreenSizeY,
		GSYS.Screen.MainScreenColorBitDepth == 16 ? 0 : 1 ) < 0 )
	{
		return DxLib_ErrorUTF16LE( "\xee\x4e\x3b\x75\x62\x97\x28\x75\x6e\x30\x20\x00\x4d\x00\x45\x00\x4d\x00\x49\x00\x4d\x00\x47\x00\x20\x00\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"仮画面用の MEMIMG の作成に失敗しました\n" @*/ ) ;
	}

	// 補助画面用 MEMIMG の作成
	if( MakeMemImgScreen(
		&GSYS.SoftRender.SubBufferMemImg,
		GSYS.Screen.MainScreenSizeX,
		GSYS.Screen.MainScreenSizeY,
		GSYS.Screen.MainScreenColorBitDepth == 16 ? 0 : 1 ) < 0 )
	{
		return DxLib_ErrorUTF16LE( "\xdc\x88\xa9\x52\x28\x75\x6e\x30\x20\x00\x4d\x00\x45\x00\x4d\x00\x49\x00\x4d\x00\x47\x00\x20\x00\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"補助用の MEMIMG の作成に失敗しました\n" @*/ ) ;
	}

	// フォントの半透明描画用アルファつき画像の作成
	_MEMSET( &GSYS.SoftRender.FontScreenMemImgAlpha, 0, sizeof( MEMIMG ) ) ;
	_MEMSET( &GSYS.SoftRender.FontScreenMemImgNormal, 0, sizeof( MEMIMG ) ) ;
	InitializeMemImg(
		&GSYS.SoftRender.FontScreenMemImgNormal,
		GSYS.Screen.MainScreenSizeX,
		GSYS.Screen.MainScreenSizeY,
		-1,
		0,
		GSYS.Screen.MainScreenColorBitDepth == 16 ? 0 : 1,
		FALSE, FALSE, FALSE, NULL ) ;
	InitializeMemImg(
		&GSYS.SoftRender.FontScreenMemImgAlpha,
		GSYS.Screen.MainScreenSizeX,
		GSYS.Screen.MainScreenSizeY,
		-1,
		0,
		GSYS.Screen.MainScreenColorBitDepth == 16 ? 0 : 1,
		FALSE, TRUE, FALSE, NULL ) ;

	// バッファのクリア
	ClearMemImg( &GSYS.SoftRender.MainBufferMemImg, NULL, NS_GetColor3( GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP, GSYS.Screen.BackgroundRed, GSYS.Screen.BackgroundGreen, GSYS.Screen.BackgroundBlue, 0 ) ) ;

	// 描画対象のセット
	GSYS.SoftRender.TargetMemImg = &GSYS.SoftRender.MainBufferMemImg ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリングの後始末を行う
static int	Graphics_Software_Terminate( void )
{
	// メインバッファの解放
	TerminateMemImg( &GSYS.SoftRender.MainBufferMemImg ) ;

	// サブバッファの解放
	TerminateMemImg( &GSYS.SoftRender.SubBufferMemImg ) ;

	// フォントの半透明描画用アルファつき画像の解放
	TerminateMemImg( &GSYS.SoftRender.FontScreenMemImgNormal ) ;
	TerminateMemImg( &GSYS.SoftRender.FontScreenMemImgAlpha ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawGraph
static int  Graphics_Software_DrawGraph( int x, int y, IMAGEDATA *Image, int TransFlag )
{
	DrawMemImg( GSYS.SoftRender.TargetMemImg, &Image->Soft.MemImg, x, y, TransFlag, GSYS.SoftRender.BlendMemImg ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawExtendGraph
static int  Graphics_Software_DrawExtendGraph( int x1, int y1, int x2, int y2, IMAGEDATA *Image, int TransFlag )
{
	RECT Rect ;

	Rect.left   = x1 ;
	Rect.top    = y1 ;
	Rect.right  = x2 ;
	Rect.bottom = y2 ;

	DrawEnlargeMemImg( GSYS.SoftRender.TargetMemImg, &Image->Soft.MemImg, &Rect, TransFlag, GSYS.SoftRender.BlendMemImg ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawRotaGraph
static int  Graphics_Software_DrawRotaGraph( float x, float y, double ExRate, double Angle, IMAGEDATA *Image, int TransFlag, int TurnFlag )
{
	DrawRotationMemImg( GSYS.SoftRender.TargetMemImg, &Image->Soft.MemImg,
		_FTOL( x ), _FTOL( y ), ( float )Angle, ( float )ExRate, TransFlag, TurnFlag, GSYS.SoftRender.BlendMemImg ) ;

	// 終了
	return 0 ;
}

static int	Graphics_Software_DrawPoly( DX_POINTDATA *p, IMAGEDATA *Image, int TransFlag, unsigned int Color )
{
	DrawBasicPolygonMemImg( GSYS.SoftRender.TargetMemImg, Image ? &Image->Soft.MemImg : NULL, 
							p, TransFlag, GSYS.SoftRender.BlendMemImg, Color ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawModiGraph
static int  Graphics_Software_DrawModiGraph( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, IMAGEDATA *Image, int TransFlag )
{
	DX_POINTDATA p[4] ;

	p[0].x = x1 ;		p[0].y = y1 ;		p[0].u = 0 ;				p[0].v = 0 ;
	p[1].x = x2 ;		p[1].y = y2 ;		p[1].u = Image->Width ;	p[1].v = 0 ;
	p[2].x = x3 ;		p[2].y = y3 ;		p[2].u = 0 ;				p[2].v = Image->Height ;
	p[3].x = x4 ;		p[3].y = y4 ;		p[3].u = Image->Width ;	p[3].v = Image->Height ;

	Graphics_Software_DrawPoly( &p[0], Image, TransFlag, 0 ) ;
	Graphics_Software_DrawPoly( &p[1], Image, TransFlag, 0 ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawModiGraphF
static int  Graphics_Software_DrawModiGraphF( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, IMAGEDATA *Image, int TransFlag )
{
	return Graphics_Software_DrawModiGraph( _FTOL( x1 ), _FTOL( y1 ), _FTOL( x2 ), _FTOL( y2 ), _FTOL( x3 ), _FTOL( y3 ), _FTOL( x4 ), _FTOL( y4 ), Image, TransFlag ) ;
}

// ソフトウエアレンダリング版 DrawChipMap
static int	Graphics_Software_DrawChipMap( int Sx, int Sy, int XNum, int YNum, const int *MapData, int MapDataPitch, int /*ChipTypeNum*/, const int *ChipGrHandle, int TransFlag )
{
	int x, y, i, j, Width, Height, Pitch2 ;
	const int *MP ;
	IMAGEDATA *Image ;

	if( GRAPHCHK( ChipGrHandle[ 0 ], Image ) )
		return -1 ;

	Width = Image->Width ;
	Height = Image->Height ;

	Pitch2 = MapDataPitch - XNum ;

	// 普通に for 文でループ描画
	y = Sy ;
	MP = MapData ;
	for( i = 0 ; i < YNum ; i ++, y += Height )
	{
		x = Sx ;
		for( j = 0 ; j < XNum ; j ++, x += Width, MP ++ )
		{
			if( *MP == -1 ) continue ;

			if( GRAPHCHK( ChipGrHandle[ *MP ], Image ) )
				return -1 ;

			Graphics_Software_DrawGraph( x, y, Image, TransFlag ) ;
		}
		MP += Pitch2 ;
	}

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawTile
static int	Graphics_Software_DrawTile( int x1, int y1, int x2, int y2, int Tx, int Ty, double ExtRate, double Angle, IMAGEDATA *Image, int TransFlag )
{
	double MaxLength ;
	float Width, Height, f, Sin, Cos ;
	float CenX, CenY, x, y, BaseX, BaseY ;
	float xAddX, xAddY, yAddX, yAddY ;
	int temp, Num, i, j ;
	RECT DrawRect ;
	float FAngle, FExtRate, Sin2, Cos2 ;

	FAngle = ( float )Angle ;
	FExtRate = ( float )ExtRate ;

	if( ExtRate <= 0.0 ) ExtRate = 0.0001 ;
	if( x1 > x2 ){ temp = x2 ; x2 = x1 ; x1 = temp ; }
	if( y1 > y2 ){ temp = y2 ; y2 = y1 ; y1 = temp ; }
	Tx %= Image->Width ;
	Ty %= Image->Height ;

	_SINCOS( FAngle, &Sin, &Cos ) ;
//	Sin = (float)sin( Angle ) ;
//	Cos = (float)cos( Angle ) ;

	// 進む値を計算
	xAddX = Image->Width * Cos * FExtRate ;
	xAddY = Image->Width * Sin * FExtRate ;

	_SINCOS( FAngle + DX_PI_F / 2.0F, &Sin2, &Cos2 ) ;
	yAddX = Image->Height * Cos2 * FExtRate ;
	yAddY = Image->Height * Sin2 * FExtRate ;

	// 描画幅を得る
	Width = (float)( x2 - x1 ) ;
	Height = (float)( y2 - y1 ) ;

	// 最大長を得る
	MaxLength = _SQRT( Width * Width + Height * Height ) ;

	// 描画個数を得る
	{
		int k ;

		k = Image->Width > Image->Height ? Image->Height : Image->Width ;
		Num = _DTOL( MaxLength / ( k * ExtRate ) + 2 ) ;
		if( Num % 2 == 0 ) Num ++ ;
	}

	// 描画中心点を得る
	CenX = x1 + Width / 2 ;
	CenY = y1 + Height / 2 ;

	// 描画可能範囲を変更する
	DrawRect = GSYS.DrawSetting.DrawArea ;
	NS_SetDrawArea( x1, y1, x2, y2 ) ;

	// 描画開始座標を得る
	BaseX = ( -Tx - ( Num * Image->Width  ) / 2 ) * FExtRate ;
	BaseY = ( -Ty - ( Num * Image->Height ) / 2 ) * FExtRate ;

	f = BaseX * Cos - BaseY * Sin ;
	BaseY = BaseX * Sin + BaseY * Cos + CenY ;
	BaseX = f + CenX ;

	// 描画開始
	for( i = 0 ; i < Num ; i ++, BaseX += yAddX, BaseY += yAddY )
	{
		x = BaseX ;
		y = BaseY ;
		for( j = 0 ; j < Num ; j ++, x += xAddX, y += xAddY )
		{
			if( Angle == 0.0 )
			{
				if( ExtRate == 0.0 )
				{
					Graphics_Software_DrawGraph( _FTOL( x ), _FTOL( y ), Image, TransFlag ) ;
				}
				else
				{
					Graphics_Software_DrawExtendGraph( _FTOL( x ), _FTOL( y ),
											 _FTOL( x + xAddX ),
											 _FTOL( y + yAddY ), Image, TransFlag ) ;
				}
			}
			else
			{
				Graphics_Software_DrawModiGraph( _FTOL( x ), _FTOL( y ),
								_FTOL( x + xAddX ), _FTOL( y + xAddY ),
								_FTOL( x + yAddX ), _FTOL( y + yAddY ),
								_FTOL( x + xAddX + yAddX ), _FTOL( y + xAddY + yAddY ),
								Image, TransFlag ) ;
			}
		}
	}

	// 描画可能範囲を元に戻す
	NS_SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;

	return 0 ;
}

// ソフトウエアレンダリング版 DrawFillBox
static int  Graphics_Software_DrawFillBox( int x1, int y1, int x2, int y2, unsigned int Color )
{
	RECT DestRect ;

	DestRect.left   = x1 ;
	DestRect.top    = y1 ;
	DestRect.right  = x2 ;
	DestRect.bottom = y2 ;

	DrawFillBoxMemImg( GSYS.SoftRender.TargetMemImg, &DestRect, Color ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawLineBox
static int  Graphics_Software_DrawLineBox( int x1, int y1, int x2, int y2, unsigned int Color )
{
	LINEDATA Line[4] ;

	Line[0].x1 = x1 ;		Line[0].y1 = y1 ;		Line[0].x2 = x2 - 1 ;	Line[0].y2 = y1 ;
	Line[1].x1 = x2 - 1 ;	Line[1].y1 = y1 ;		Line[1].x2 = x2 - 1 ;	Line[1].y2 = y2 - 1 ;
	Line[2].x1 = x2 - 1 ;	Line[2].y1 = y2 - 1 ;	Line[2].x2 = x1 ;		Line[2].y2 = y2 - 1 ;
	Line[3].x1 = x1 ;		Line[3].y1 = y2 - 1 ;	Line[3].x2 = x1 ;		Line[3].y2 = y1 ;
	Line[3].color = Line[2].color = Line[1].color = Line[0].color = Color ;
	Line[3].pal   = Line[2].pal   = Line[1].pal   = Line[0].pal   = GSYS.DrawSetting.BlendParam ;

	return Graphics_Software_DrawLineSet( Line, 4 ) ;
}

// ソフトウエアレンダリング版 DrawLine
static int	Graphics_Software_DrawLine( int x1, int y1, int x2, int y2, unsigned int Color )
{
	DrawLineMemImg( GSYS.SoftRender.TargetMemImg, x1, y1, x2, y2, Color ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawCircle( 太さ指定あり )
static int	Graphics_Software_DrawCircle_Thickness( int x, int y, int r, unsigned int Color, int Thickness )
{
	int i ;
	RECT DrawRect ;
	short ( *CirclePos )[ 5 ] ;
	int x1, x2, y1 ;

	DrawRect = GSYS.DrawSetting.DrawArea ;

	// 円の座標を代入するバッファを取得
	if( Graphics_Other_AllocCommonBuffer( 1, sizeof( short ) * 5 * DrawRect.bottom ) < 0 )
	{
		return -1 ;
	}
	CirclePos = ( short (*)[ 5 ] )GSYS.Resource.CommonBuffer[ 1 ] ;
	_MEMSET( CirclePos, 0, sizeof( short ) * 5 * DrawRect.bottom ) ;

	// 円の座標リスト作成
	Graphics_Draw_GetCircle_ThicknessDrawPosition( x, y, r, Thickness, CirclePos ) ;

	// 描画処理
	for( i = DrawRect.top ; i < DrawRect.bottom ; i ++ )
	{
		if( CirclePos[ i ][ 4 ] == 0 )
		{
			continue ;
		}

		y1 = i ;

		if( ( CirclePos[ i ][ 4 ] & ( 4 | 8 ) ) == 0 || CirclePos[ i ][ 2 ] >= CirclePos[ i ][ 3 ] )
		{
			x1 = CirclePos[ i ][ 0 ] ;
			x2 = CirclePos[ i ][ 1 ] ;
			DrawLineMemImg( GSYS.SoftRender.TargetMemImg, x1, y1, x2 + 1, y1, Color ) ;
		}
		else
		{
			x1 = CirclePos[ i ][ 0 ] ;
			x2 = CirclePos[ i ][ 2 ] ;
			DrawLineMemImg( GSYS.SoftRender.TargetMemImg, x1, y1, x2 + 1, y1, Color ) ;

			x1 = CirclePos[ i ][ 3 ] ;
			x2 = CirclePos[ i ][ 1 ] ;
			DrawLineMemImg( GSYS.SoftRender.TargetMemImg, x1, y1, x2 + 1, y1, Color ) ;
		}
	}

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawOval( 太さ指定あり )
static int	Graphics_Software_DrawOval_Thickness( int x, int y, int rx, int ry, unsigned int Color, int Thickness )
{
	int i ;
	RECT DrawRect ;
	short ( *CirclePos )[ 5 ] ;
	int x1, x2, y1 ;

	DrawRect = GSYS.DrawSetting.DrawArea ;

	// 円の座標を代入するバッファを取得
	if( Graphics_Other_AllocCommonBuffer( 1, sizeof( short ) * 5 * DrawRect.bottom ) < 0 )
	{
		return -1 ;
	}
	CirclePos = ( short (*)[ 5 ] )GSYS.Resource.CommonBuffer[ 1 ] ;
	_MEMSET( CirclePos, 0, sizeof( short ) * 5 * DrawRect.bottom ) ;

	// 円の座標リスト作成
	Graphics_Draw_GetOval_ThicknessDrawPosition( x, y, rx, ry, Thickness, CirclePos ) ;

	// 描画処理
	for( i = DrawRect.top ; i < DrawRect.bottom ; i ++ )
	{
		if( CirclePos[ i ][ 4 ] == 0 )
		{
			continue ;
		}

		y1 = i ;

		if( ( CirclePos[ i ][ 4 ] & ( 4 | 8 ) ) == 0 || CirclePos[ i ][ 2 ] >= CirclePos[ i ][ 3 ] )
		{
			x1 = CirclePos[ i ][ 0 ] ;
			x2 = CirclePos[ i ][ 1 ] ;
			DrawLineMemImg( GSYS.SoftRender.TargetMemImg, x1, y1, x2 + 1, y1, Color ) ;
		}
		else
		{
			x1 = CirclePos[ i ][ 0 ] ;
			x2 = CirclePos[ i ][ 2 ] ;
			DrawLineMemImg( GSYS.SoftRender.TargetMemImg, x1, y1, x2 + 1, y1, Color ) ;

			x1 = CirclePos[ i ][ 3 ] ;
			x2 = CirclePos[ i ][ 1 ] ;
			DrawLineMemImg( GSYS.SoftRender.TargetMemImg, x1, y1, x2 + 1, y1, Color ) ;
		}
	}

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawCircle
static int	Graphics_Software_DrawCircle( int x, int y, int r, unsigned int Color, int FillFlag )
{
	DrawCircleMemImg( GSYS.SoftRender.TargetMemImg, x, y, r, Color, FillFlag ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawOval
static int	Graphics_Software_DrawOval( int x, int y, int rx, int ry, unsigned int Color, int FillFlag )
{
	DrawOvalMemImg( GSYS.SoftRender.TargetMemImg, x, y, rx, ry, Color, FillFlag ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawTriangle
static int	Graphics_Software_DrawTriangle( int x1, int y1, int x2, int y2, int x3, int y3, unsigned int Color, int FillFlag )
{
	// 塗りつぶしかそうでないかで処理を分岐
	if( FillFlag == TRUE )
	{
		// 塗りつぶしの場合
		DX_POINTDATA p[3] ;

		p[0].x = x1 ;		p[0].y = y1 ;
		p[1].x = x2 ;		p[1].y = y2 ;
		p[2].x = x3 ;		p[2].y = y3 ;
	
		return Graphics_Software_DrawPoly( p, NULL, FALSE, Color ) ;
	}
	else
	{
		LINEDATA Line[3] ;
		
		Line[0].x1 = x1 ;	Line[0].y1 = y1 ;	Line[0].x2 = x2 ;	Line[0].y2 = y2 ;
		Line[1].x1 = x2 ;	Line[1].y1 = y2 ;	Line[1].x2 = x3 ;	Line[1].y2 = y3 ;
		Line[2].x1 = x3 ;	Line[2].y1 = y3 ;	Line[2].x2 = x1 ;	Line[2].y2 = y1 ;
		Line[2].color = Line[1].color = Line[0].color = Color ;
		Line[2].pal   = Line[1].pal   = Line[0].pal   = GSYS.DrawSetting.BlendParam ;
		return Graphics_Software_DrawLineSet( Line, 3 ) ;
	}
}

// ソフトウエアレンダリング版 DrawQuadrangle
static int	Graphics_Software_DrawQuadrangle( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, int FillFlag )
{
	// 塗りつぶしかそうでないかで処理を分岐
	if( FillFlag == TRUE )
	{
		// 塗りつぶしの場合

		DX_POINTDATA p[3] ;

		p[0].x = x1 ;		p[0].y = y1 ;
		p[1].x = x2 ;		p[1].y = y2 ;
		p[2].x = x4 ;		p[2].y = y4 ;
		Graphics_Software_DrawPoly( p, NULL, FALSE, Color ) ;

		p[0].x = x3 ;		p[0].y = y3 ;
		p[1].x = x4 ;		p[1].y = y4 ;
		p[2].x = x2 ;		p[2].y = y2 ;
		return Graphics_Software_DrawPoly( p, NULL, FALSE, Color ) ;
	}
	else
	{
		LINEDATA Line[4] ;
		
		Line[0].x1 = x1 ;	Line[0].y1 = y1 ;	Line[0].x2 = x2 ;	Line[0].y2 = y2 ;
		Line[1].x1 = x2 ;	Line[1].y1 = y2 ;	Line[1].x2 = x3 ;	Line[1].y2 = y3 ;
		Line[2].x1 = x3 ;	Line[2].y1 = y3 ;	Line[2].x2 = x4 ;	Line[2].y2 = y4 ;
		Line[3].x1 = x4 ;	Line[3].y1 = y4 ;	Line[3].x2 = x1 ;	Line[3].y2 = y1 ;
		Line[3].color = Line[2].color = Line[1].color = Line[0].color = Color ;
		Line[3].pal   = Line[2].pal   = Line[1].pal   = Line[0].pal   = GSYS.DrawSetting.BlendParam ;
		return Graphics_Software_DrawLineSet( Line, 4 ) ;
	}
}


// ソフトウエアレンダリング版 DrawPixel
static int	Graphics_Software_DrawPixel( int x, int y, unsigned int Color )
{
	DrawPixelMemImg( GSYS.SoftRender.TargetMemImg, x, y, Color ) ;

	// 終了
	return 0 ;
}


// ソフトウエアレンダリング版 DrawPixelSet
static int	Graphics_Software_DrawPixelSet( const POINTDATA *PointData, int Num )
{
	DrawPixelSetMemImg( GSYS.SoftRender.TargetMemImg, PointData, Num ) ;

	// 終了
	return 0 ;
}

// ソフトウエアレンダリング版 DrawLineSet
static int	Graphics_Software_DrawLineSet( const LINEDATA *LineData, int Num )
{
	DrawLineSetMemImg( GSYS.SoftRender.TargetMemImg, LineData, Num ) ;

	// 終了
	return 0 ;
}




















// グラフィックハンドル作成関係関数

// 空のグラフィックを作成
extern	int NS_MakeGraph( int SizeX, int SizeY, int NotUse3DFlag )
{
	SETUP_GRAPHHANDLE_GPARAM GParam ;

	Graphics_Image_InitSetupGraphHandleGParam( &GParam ) ;

	return Graphics_Image_MakeGraph_UseGParam( &GParam, SizeX, SizeY, NotUse3DFlag, FALSE, 0, GetASyncLoadFlag() ) ;
}

// 描画可能な画面を作成
extern int NS_MakeScreen( int SizeX, int SizeY, int UseAlphaChannel )
{
	SETUP_GRAPHHANDLE_GPARAM GParam ;

	// ソフトウエアレンダリングモードではアルファチャンネル付きの描画可能画像は作れない
	if( UseAlphaChannel && NS_GetScreenMemToSystemMemFlag() == TRUE )
		return -1 ;

	Graphics_Image_InitSetupGraphHandleGParam( &GParam ) ;

	GParam.DrawValidImageCreateFlag = TRUE ;
	GParam.DrawValidAlphaImageCreateFlag = UseAlphaChannel ;
	return Graphics_Image_MakeGraph_UseGParam( &GParam, SizeX, SizeY, FALSE, FALSE, 0, GetASyncLoadFlag() ) ;
}

// 指定のグラフィックの指定部分だけを抜き出して新たなグラフィックハンドルを作成する
extern int NS_DerivationGraph( int SrcX, int SrcY, int Width, int Height, int SrcGraphHandle )
{
	return Graphics_Image_DerivationGraph_UseGParam( SrcX, SrcY, Width, Height, SrcGraphHandle, FALSE ) ;
}

// 指定のグラフィックデータを削除する
extern	int NS_DeleteGraph( int GrHandle, int )
{
	return SubHandle( GrHandle ) ;
}

// 指定のグラフィックハンドルと、同じグラフィックデータから派生しているグラフィックハンドル( DerivationGraph で派生したハンドル、LoadDivGraph 読み込んで作成された複数のハンドル )を一度に削除する
extern	int NS_DeleteSharingGraph( int GrHandle )
{
	IMAGEDATA_ORIG *Orig ;
	IMAGEDATA *Image, **TmpImage ;
	bool BreakFlag ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	// 同じグラフィックデータから派生しているハンドルを全て削除する
	Orig = Image->Orig ;
	BreakFlag = false ;
	for( TmpImage = ( IMAGEDATA ** )&HandleManageArray[ DX_HANDLETYPE_GRAPH ].Handle[ HandleManageArray[ DX_HANDLETYPE_GRAPH ].AreaMin ] ; ; TmpImage ++ )
	{
		if( *TmpImage == NULL ) continue ;

		if( ( *TmpImage )->Orig == Orig )
		{
			BreakFlag = Orig->RefCount == 1 ;
			NS_DeleteGraph( ( *TmpImage )->HandleInfo.Handle ) ;
			if( BreakFlag ) break ;
		}
	}

	// 終了
	return 0 ;
}

// 有効なグラフィックの数を取得する
extern	int NS_GetGraphNum( void )
{
	return HandleManageArray[ DX_HANDLETYPE_GRAPH ].Num ;
}

// グラフィックを特定の色で塗りつぶす
extern	int NS_FillGraph( int GrHandle, int Red, int Green, int Blue, int Alpha )
{
	return Graphics_Image_FillGraph_UseGParam( GrHandle, Red, Green, Blue, Alpha, FALSE ) ;
}

// 解放時に立てるフラグのポインタをセットする
extern	int NS_SetGraphLostFlag( int GrHandle, int *LostFlag )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	Image->LostFlag = LostFlag ;
	if( LostFlag != NULL ) *LostFlag = FALSE ;

	// 終了
	return 0 ;
}

// InitGraph でグラフィックハンドルを削除するかどうかをチェックする関数
static int DeleteCancelCheckInitGraphFunction( HANDLEINFO *HandleInfo )
{
	IMAGEDATA *Image = ( IMAGEDATA * )HandleInfo ;

	return Image->NotInitGraphDelete ;
}

// 画像データの初期化
extern	int NS_InitGraph( int )
{
	int Result ;

	// グラフィックハンドルを全て削除
	Result = AllHandleSub( DX_HANDLETYPE_GRAPH, DeleteCancelCheckInitGraphFunction ) ;

	// シャドウマップハンドルも全て削除
	Result = AllHandleSub( DX_HANDLETYPE_SHADOWMAP ) ;

	// 環境依存処理
	Graphics_Hardware_InitGraph_PF() ;

	// 終了
	return Result ;
}

// ファイルから読み込んだ画像情報を再度読み込む
extern	int NS_ReloadFileGraphAll( void )
{
	Graphics_Image_DefaultRestoreGraphFunction() ;

	// 終了
	return 0 ;
}






















// シャドウマップハンドル関係関数

// シャドウマップハンドルを作成する
extern int NS_MakeShadowMap( int SizeX, int SizeY )
{
	SETUP_SHADOWMAPHANDLE_GPARAM GParam ;

	// ハードウエアアクセラレータが無効だったらエラー
	if( Graphics_Hardware_CheckValid_PF() == 0 )
		return -1 ;

	GParam.Dummy = 0 ;

	return Graphics_ShadowMap_MakeShadowMap_UseGParam( &GParam, SizeX, SizeY, GetASyncLoadFlag() ) ;
}

// シャドウマップハンドルを削除する
extern int NS_DeleteShadowMap( int SmHandle )
{
	return SubHandle( SmHandle ) ;
}

// シャドウマップが想定するライトの方向を設定する
extern int NS_SetShadowMapLightDirection( int SmHandle, VECTOR Direction )
{
	VECTOR xvec ;
	VECTOR yvec ;
	VECTOR zvec ;
	SHADOWMAPDATA *ShadowMap ;

	// アドレスの取得
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	// ライトベクトル方向に回転するための行列を作成
	zvec = VNorm( Direction ) ;
	yvec = VGet( 0.0f, 1.0f, 0.0f ) ;
	xvec = VCross( yvec, zvec ) ;
	if( VSquareSize( xvec ) <= 0.0000001f )
	{
		yvec = VGet( 1.0f, 0.0f, 0.0f ) ;
		xvec = VCross( yvec, zvec ) ;
	}
	yvec = VNorm( VCross( zvec, xvec ) ) ;
	xvec = VNorm( xvec ) ;
	ShadowMap->ShadowMapViewMatrix.m[ 0 ][ 0 ] = ShadowMap->LightMatrix.m[ 0 ][ 0 ] = xvec.x ;
	ShadowMap->ShadowMapViewMatrix.m[ 1 ][ 0 ] = ShadowMap->LightMatrix.m[ 1 ][ 0 ] = xvec.y ;
	ShadowMap->ShadowMapViewMatrix.m[ 2 ][ 0 ] = ShadowMap->LightMatrix.m[ 2 ][ 0 ] = xvec.z ;

	ShadowMap->ShadowMapViewMatrix.m[ 0 ][ 1 ] = ShadowMap->LightMatrix.m[ 0 ][ 1 ] = yvec.x ;
	ShadowMap->ShadowMapViewMatrix.m[ 1 ][ 1 ] = ShadowMap->LightMatrix.m[ 1 ][ 1 ] = yvec.y ;
	ShadowMap->ShadowMapViewMatrix.m[ 2 ][ 1 ] = ShadowMap->LightMatrix.m[ 2 ][ 1 ] = yvec.z ;

	ShadowMap->ShadowMapViewMatrix.m[ 0 ][ 2 ] = ShadowMap->LightMatrix.m[ 0 ][ 2 ] = zvec.x ;
	ShadowMap->ShadowMapViewMatrix.m[ 1 ][ 2 ] = ShadowMap->LightMatrix.m[ 1 ][ 2 ] = zvec.y ;
	ShadowMap->ShadowMapViewMatrix.m[ 2 ][ 2 ] = ShadowMap->LightMatrix.m[ 2 ][ 2 ] = zvec.z ;

	// ライトの方向を保存
	ShadowMap->LightDirection = Direction ;

	// もしシャドウマップへの描画準備済みの場合はシャドウマップへの描画に必要な行列を更新する
	if( ShadowMap->DrawSetupFlag )
	{
		// 行列の更新
		Graphics_ShadowMap_RefreshMatrix( ShadowMap ) ;
	}

	// 終了
	return 0 ;
}

// シャドウマップへの描画の準備を行う
extern int NS_ShadowMap_DrawSetup( int SmHandle )
{
	int i ;
	SHADOWMAPDATA *ShadowMap ;

	// アドレスの取得
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	// 描画で使用するシャドウマップに設定されていたら解除
	for( i = 0 ; i < MAX_USE_SHADOWMAP_NUM ; i ++ )
	{
		if( GSYS.DrawSetting.ShadowMap[ i ] == SmHandle )
		{
			NS_SetUseShadowMap( i, -1 ) ;
		}
	}

	// 既に描画準備が完了している場合は何もしない
	if( ShadowMap->DrawSetupFlag == TRUE )
		return -1 ;

	// 別のシャドウマップに対する描画準備が完了していたら、そちらの描画完了状態を解除する
	if( GSYS.DrawSetting.ShadowMapDraw == TRUE )
	{
		NS_ShadowMap_DrawEnd() ;
	}

	// 元々の描画対象を保存
	for( i = 0 ; i < DX_RENDERTARGET_COUNT ; i ++ )
	{
		ShadowMap->RenderTargetScreen[ i ]        = GSYS.DrawSetting.TargetScreen[ i ] ;
		ShadowMap->RenderTargetScreenSurface[ i ] = GSYS.DrawSetting.TargetScreenSurface[ i ] ;
	}

	// 現在の描画座標計算用の情報を保存
#ifndef DX_NON_MASK
	ShadowMap->RenderMaskUseFlag          = MASKD.MaskUseFlag ;
#endif

	ShadowMap->RenderCameraPosition       = GSYS.Camera.Position ;
	ShadowMap->RenderCameraTarget         = GSYS.Camera.Target ;
	ShadowMap->RenderCameraUp             = GSYS.Camera.Up ;
	ShadowMap->RenderCameraHRotate        = GSYS.Camera.HRotate ;
	ShadowMap->RenderCameraVRotate        = GSYS.Camera.VRotate ;
	ShadowMap->RenderCameraTRotate        = GSYS.Camera.TRotate ;
	ShadowMap->RenderCameraMatrix         = GSYS.Camera.Matrix ;
	ShadowMap->RenderCameraScreenCenterX  = GSYS.Camera.ScreenCenterX ;
	ShadowMap->RenderCameraScreenCenterY  = GSYS.Camera.ScreenCenterY ;

	ShadowMap->RenderProjectionMatrixMode = GSYS.DrawSetting.ProjectionMatrixMode ;
	ShadowMap->RenderProjNear             = GSYS.DrawSetting.ProjNear ;
	ShadowMap->RenderProjFar              = GSYS.DrawSetting.ProjFar ;
	ShadowMap->RenderProjDotAspect        = GSYS.DrawSetting.ProjDotAspect ;
	ShadowMap->RenderProjFov              = GSYS.DrawSetting.ProjFov ;
	ShadowMap->RenderProjSize             = GSYS.DrawSetting.ProjSize ;
	ShadowMap->RenderProjMatrix           = GSYS.DrawSetting.ProjMatrix ;

	// 描画範囲が設定されていない場合に使用する描画範囲を算出する
	{
		VECTOR AddXVecP ;
		VECTOR AddXVecM ;
		VECTOR AddYVecP ;
		VECTOR AddYVecM ;
		VECTOR AddZVecP ;
		VECTOR AddZVecM ;

		// 描画範囲に入れる領域は視錐台の後方２倍、左右・上下１．５倍、にする
		AddXVecP = VSub( NS_ConvScreenPosToWorldPos( VGet( GSYS.DrawSetting.DrawAreaF.right,                           0.0f, 1.0f ) ), NS_ConvScreenPosToWorldPos( VGet( GSYS.DrawSetting.DrawAreaF.left,                              0.0f, 1.0f ) ) ) ;
		AddYVecP = VSub( NS_ConvScreenPosToWorldPos( VGet(                             0.0f, GSYS.DrawSetting.DrawAreaF.top, 1.0f ) ), NS_ConvScreenPosToWorldPos( VGet(                            0.0f, GSYS.DrawSetting.DrawAreaF.bottom, 1.0f ) ) ) ;
		AddZVecP = VSub( NS_ConvScreenPosToWorldPos( VGet(                             0.0f,                           0.0f, 1.0f ) ), NS_ConvScreenPosToWorldPos( VGet(                            0.0f,                              0.0f, 0.0f ) ) ) ;
		AddXVecM = VScale( AddXVecP, -1.0f ) ;
		AddYVecM = VScale( AddYVecP, -1.0f ) ;
		AddZVecM = VScale( AddZVecP, -1.0f ) ;

		AddXVecP = VScale( AddXVecP, 0.5f ) ;
		AddXVecM = VScale( AddXVecM, 0.5f ) ;
		AddYVecP = VScale( AddYVecP, 0.5f ) ;
		AddYVecM = VScale( AddYVecM, 0.5f ) ;
	//	AddZVecP = VScale( AddZVecP, 0.5f ) ;
	//	AddZVecM = VScale( AddZVecM, 0.5f ) ;

		// ライトベクトル方向にビュークリップ座標を回転して、最大値と最小値を算出
		for( i = 0 ; i < 8 ; i ++ )
		{
			ShadowMap->DefaultViewClipPos[ i ] = VConvDtoF( ( ( VECTOR_D * )GSYS.DrawSetting.ViewClipPos )[ i ] ) ;
		}
		
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 0 ], &ShadowMap->DefaultViewClipPos[ 0 ], &AddZVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 1 ], &ShadowMap->DefaultViewClipPos[ 1 ], &AddZVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 2 ], &ShadowMap->DefaultViewClipPos[ 2 ], &AddZVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 3 ], &ShadowMap->DefaultViewClipPos[ 3 ], &AddZVecM ) ;
		
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 0 ], &ShadowMap->DefaultViewClipPos[ 0 ], &AddXVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 2 ], &ShadowMap->DefaultViewClipPos[ 2 ], &AddXVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 4 ], &ShadowMap->DefaultViewClipPos[ 4 ], &AddXVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 6 ], &ShadowMap->DefaultViewClipPos[ 6 ], &AddXVecM ) ;

		VectorAdd( &ShadowMap->DefaultViewClipPos[ 1 ], &ShadowMap->DefaultViewClipPos[ 1 ], &AddXVecP ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 3 ], &ShadowMap->DefaultViewClipPos[ 3 ], &AddXVecP ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 5 ], &ShadowMap->DefaultViewClipPos[ 5 ], &AddXVecP ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 7 ], &ShadowMap->DefaultViewClipPos[ 7 ], &AddXVecP ) ;
		
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 0 ], &ShadowMap->DefaultViewClipPos[ 0 ], &AddYVecP ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 1 ], &ShadowMap->DefaultViewClipPos[ 1 ], &AddYVecP ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 4 ], &ShadowMap->DefaultViewClipPos[ 4 ], &AddYVecP ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 5 ], &ShadowMap->DefaultViewClipPos[ 5 ], &AddYVecP ) ;

		VectorAdd( &ShadowMap->DefaultViewClipPos[ 2 ], &ShadowMap->DefaultViewClipPos[ 2 ], &AddYVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 3 ], &ShadowMap->DefaultViewClipPos[ 3 ], &AddYVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 6 ], &ShadowMap->DefaultViewClipPos[ 6 ], &AddYVecM ) ;
		VectorAdd( &ShadowMap->DefaultViewClipPos[ 7 ], &ShadowMap->DefaultViewClipPos[ 7 ], &AddYVecM ) ;
	}

#ifndef DX_NON_MASK
	// マスク画面を使用しない設定に変更
	NS_SetUseMaskScreenFlag( FALSE ) ;
#endif

	// 描画対象をシャドウマップに変更
	GSYS.DrawSetting.ShadowMapDrawSetupRequest = TRUE ;
	NS_SetDrawScreen( SmHandle ) ;
	for( i = 1 ; i < DX_RENDERTARGET_COUNT ; i ++ )
	{
		NS_SetRenderTargetToShader( i, -1 ) ;
	}

	// シャドウマップへのレンダリングに必要な行列のセットアップ
	Graphics_ShadowMap_RefreshMatrix( ShadowMap ) ;

	// 描画準備をしてあるフラグを立てる
	ShadowMap->DrawSetupFlag = TRUE ;

	// 環境依存処理
	Graphics_Hardware_ShadowMap_DrawSetup_PF( ShadowMap ) ;

	// 画面を最深度でクリア
	{
		int Red, Green, Blue, Enable ;

		Red   = GSYS.Screen.BackgroundRed ;
		Green = GSYS.Screen.BackgroundGreen ;
		Blue  = GSYS.Screen.BackgroundBlue ;
		Enable = GSYS.Screen.EnableBackgroundColor ;

		NS_SetBackgroundColor( 255,255,255 ) ;
		NS_ClearDrawScreen() ;

		GSYS.Screen.BackgroundRed = Red ;
		GSYS.Screen.BackgroundGreen = Green ;
		GSYS.Screen.BackgroundBlue = Blue ;
		GSYS.Screen.EnableBackgroundColor = Enable ;
	}

	// 終了
	return 0 ;
}

// シャドウマップへの描画を終了する
extern int NS_ShadowMap_DrawEnd( void )
{
	SHADOWMAPDATA *ShadowMap ;
	int i ;

	// 描画準備をしていない場合は何もしない
	if( GSYS.DrawSetting.ShadowMapDraw == FALSE )
		return -1 ;

	// アドレスの取得
	if( SHADOWMAPCHK( GSYS.DrawSetting.ShadowMapDrawHandle, ShadowMap ) )
	{
		GSYS.DrawSetting.ShadowMapDraw = FALSE ;
		return -1 ;
	}

	// 描画準備をしていない場合は何もしない
	if( ShadowMap->DrawSetupFlag == FALSE )
		return -1 ;

	// 描画準備完了済みフラグを倒す
	ShadowMap->DrawSetupFlag = FALSE ;

	// シャドウマップに対する描画であるかどうかのフラグを倒す
	GSYS.DrawSetting.ShadowMapDraw = FALSE ;
	GSYS.DrawSetting.ShadowMapDrawHandle = 0 ;

	// 描画先を元に戻す
	for( i = 0 ; i < DX_RENDERTARGET_COUNT ; i ++ )
	{
		NS_SetRenderTargetToShader( i, ShadowMap->RenderTargetScreen[ i ], ShadowMap->RenderTargetScreenSurface[ i ] ) ; 
	}

#ifndef DX_NON_MASK
	// マスク画面を使用する設定を元に戻す
	NS_SetUseMaskScreenFlag( ShadowMap->RenderMaskUseFlag ) ;
#endif

	// カメラ設定を元に戻す
	GSYS.Camera.Position      = ShadowMap->RenderCameraPosition ;
	GSYS.Camera.Target        = ShadowMap->RenderCameraTarget ;
	GSYS.Camera.Up            = ShadowMap->RenderCameraUp ;
	GSYS.Camera.HRotate       = ShadowMap->RenderCameraHRotate ;
	GSYS.Camera.VRotate       = ShadowMap->RenderCameraVRotate ;
	GSYS.Camera.TRotate       = ShadowMap->RenderCameraTRotate ;
	GSYS.Camera.Matrix        = ShadowMap->RenderCameraMatrix ;
	NS_SetTransformToViewD( &ShadowMap->RenderCameraMatrix ) ;
	NS_SetCameraScreenCenterD( ShadowMap->RenderCameraScreenCenterX, ShadowMap->RenderCameraScreenCenterY ) ;

	// 射影パラメータをリセット
	GSYS.DrawSetting.ProjectionMatrixMode = ShadowMap->RenderProjectionMatrixMode ;
	GSYS.DrawSetting.ProjNear             = ShadowMap->RenderProjNear ;
	GSYS.DrawSetting.ProjFar              = ShadowMap->RenderProjFar ;
	GSYS.DrawSetting.ProjDotAspect        = ShadowMap->RenderProjDotAspect ;
	GSYS.DrawSetting.ProjFov              = ShadowMap->RenderProjFov ;
	GSYS.DrawSetting.ProjSize             = ShadowMap->RenderProjSize ;
	GSYS.DrawSetting.ProjMatrix           = ShadowMap->RenderProjMatrix ;
	Graphics_DrawSetting_SetTransformToProjection_Direct( &GSYS.DrawSetting.ProjMatrix ) ;

	// 環境依存処理
	Graphics_Hardware_ShadowMap_DrawEnd_PF( ShadowMap ) ;

#ifndef DX_NON_FILTER

	// シャドウマップにブラーを掛ける
	if( ShadowMap->BlurParam > 0 )
	{
		NS_GraphFilter( ShadowMap->HandleInfo.Handle, DX_GRAPH_FILTER_GAUSS, 8, ShadowMap->BlurParam ) ;
	}

#endif // DX_NON_FILTER

	// 終了
	return 0 ;
}

// 描画で使用するシャドウマップを指定する、スロットは０か１かを指定可能、SmHandle に -1 を渡すと指定のスロットのシャドウマップを解除
extern int NS_SetUseShadowMap( int SlotIndex, int SmHandle )
{
	SHADOWMAPDATA *ShadowMap = NULL ;

	// スロットのインデックスが不正な場合はエラー
	if( SlotIndex < 0 || SlotIndex >= MAX_USE_SHADOWMAP_NUM )
		return -1 ;

	// ハンドル値がマイナスの値の場合は解除
	if( SmHandle < 0 )
	{
		// 有効なシャドウマップを設定している数を更新
		if( GSYS.DrawSetting.ShadowMap[ SlotIndex ] != 0 )
		{
			GSYS.DrawSetting.UseShadowMapNum -- ;
		}

		// ハンドル値をリセット
		GSYS.DrawSetting.ShadowMap[ SlotIndex ] = 0 ;
	}
	else
	{
		// アドレスの取得
		if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
			return -1 ;

		// 有効なシャドウマップを設定している数を更新
		if( GSYS.DrawSetting.ShadowMap[ SlotIndex ] == 0 )
		{
			GSYS.DrawSetting.UseShadowMapNum ++ ;
		}

		// ハンドル値の保存
		GSYS.DrawSetting.ShadowMap[ SlotIndex ] = SmHandle ;
	}

	// 環境依存処理
	Graphics_Hardware_ShadowMap_SetUse_PF( SlotIndex, ShadowMap ) ;

	// シェーダーのパラメータに反映
	Graphics_ShadowMap_RefreshVSParam() ;
	Graphics_ShadowMap_RefreshPSParam() ;

	// 終了
	return 0 ;
}

// シャドウマップに描画する際の範囲を設定する( この関数で描画範囲を設定しない場合は視錐台を拡大した範囲が描画範囲となる )
extern int NS_SetShadowMapDrawArea( int SmHandle, VECTOR MinPosition, VECTOR MaxPosition )
{
	SHADOWMAPDATA *ShadowMap ;

	// エラー判定
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	// 座標を保存
	if( MinPosition.x < MaxPosition.x )
	{
		ShadowMap->DrawAreaMinPosition.x = MinPosition.x ;
		ShadowMap->DrawAreaMaxPosition.x = MaxPosition.x ;
	}
	else
	{
		ShadowMap->DrawAreaMinPosition.x = MaxPosition.x ;
		ShadowMap->DrawAreaMaxPosition.x = MinPosition.x ;
	}

	if( MinPosition.y < MaxPosition.y )
	{
		ShadowMap->DrawAreaMinPosition.y = MinPosition.y ;
		ShadowMap->DrawAreaMaxPosition.y = MaxPosition.y ;
	}
	else
	{
		ShadowMap->DrawAreaMinPosition.y = MaxPosition.y ;
		ShadowMap->DrawAreaMaxPosition.y = MinPosition.y ;
	}

	if( MinPosition.z < MaxPosition.z )
	{
		ShadowMap->DrawAreaMinPosition.z = MinPosition.z ;
		ShadowMap->DrawAreaMaxPosition.z = MaxPosition.z ;
	}
	else
	{
		ShadowMap->DrawAreaMinPosition.z = MaxPosition.z ;
		ShadowMap->DrawAreaMaxPosition.z = MinPosition.z ;
	}

	// 幅が無い場合は無効
	if( ShadowMap->DrawAreaMaxPosition.x - ShadowMap->DrawAreaMinPosition.x < 0.00000001f ||
		ShadowMap->DrawAreaMaxPosition.y - ShadowMap->DrawAreaMinPosition.y < 0.00000001f ||
		ShadowMap->DrawAreaMaxPosition.z - ShadowMap->DrawAreaMinPosition.z < 0.00000001f )
	{
		return -1 ;
	}

	// フラグを有効に設定
	ShadowMap->EnableDrawArea = TRUE ;

	// シャドウマップに描画する範囲の座標を更新
	ShadowMap->DrawAreaViewClipPos[ 0 ].x = ShadowMap->DrawAreaMinPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 0 ].y = ShadowMap->DrawAreaMinPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 0 ].z = ShadowMap->DrawAreaMinPosition.z ;

	ShadowMap->DrawAreaViewClipPos[ 1 ].x = ShadowMap->DrawAreaMaxPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 1 ].y = ShadowMap->DrawAreaMinPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 1 ].z = ShadowMap->DrawAreaMinPosition.z ;

	ShadowMap->DrawAreaViewClipPos[ 2 ].x = ShadowMap->DrawAreaMinPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 2 ].y = ShadowMap->DrawAreaMaxPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 2 ].z = ShadowMap->DrawAreaMinPosition.z ;

	ShadowMap->DrawAreaViewClipPos[ 3 ].x = ShadowMap->DrawAreaMaxPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 3 ].y = ShadowMap->DrawAreaMaxPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 3 ].z = ShadowMap->DrawAreaMinPosition.z ;

	ShadowMap->DrawAreaViewClipPos[ 4 ].x = ShadowMap->DrawAreaMinPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 4 ].y = ShadowMap->DrawAreaMinPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 4 ].z = ShadowMap->DrawAreaMaxPosition.z ;

	ShadowMap->DrawAreaViewClipPos[ 5 ].x = ShadowMap->DrawAreaMaxPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 5 ].y = ShadowMap->DrawAreaMinPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 5 ].z = ShadowMap->DrawAreaMaxPosition.z ;

	ShadowMap->DrawAreaViewClipPos[ 6 ].x = ShadowMap->DrawAreaMinPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 6 ].y = ShadowMap->DrawAreaMaxPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 6 ].z = ShadowMap->DrawAreaMaxPosition.z ;

	ShadowMap->DrawAreaViewClipPos[ 7 ].x = ShadowMap->DrawAreaMaxPosition.x ;
	ShadowMap->DrawAreaViewClipPos[ 7 ].y = ShadowMap->DrawAreaMaxPosition.y ;
	ShadowMap->DrawAreaViewClipPos[ 7 ].z = ShadowMap->DrawAreaMaxPosition.z ;

	// もしシャドウマップへの描画準備済みの場合はシャドウマップへの描画に必要な行列を更新する
	if( ShadowMap->DrawSetupFlag )
	{
		// 行列の更新
		Graphics_ShadowMap_RefreshMatrix( ShadowMap ) ;
	}

	// 終了
	return 0 ;
}

// SetShadowMapDrawArea の設定を解除する
extern int NS_ResetShadowMapDrawArea( int SmHandle )
{
	SHADOWMAPDATA *ShadowMap ;

	// エラー判定
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	// 既に設定が解除されていたら何もしない
	if( ShadowMap->EnableDrawArea == FALSE )
	{
		return 0 ;
	}

	// フラグを倒す
	ShadowMap->EnableDrawArea = FALSE ;

	// もしシャドウマップへの描画準備済みの場合はシャドウマップへの描画に必要な行列を更新する
	if( ShadowMap->DrawSetupFlag )
	{
		// 行列の更新
		Graphics_ShadowMap_RefreshMatrix( ShadowMap ) ;
	}

	// 終了
	return 0 ;
}

// シャドウマップを使用した描画時の補正深度を設定する
extern int NS_SetShadowMapAdjustDepth( int SmHandle, float Depth )
{
	SHADOWMAPDATA *ShadowMap ;
	int i ;

	// エラー判定
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	// 補正値を保存する
	ShadowMap->AdjustDepth = Depth ;

	// 既にシャドウマップを使用中の場合は補正値を変更する
	for( i = 0 ; i < MAX_USE_SHADOWMAP_NUM ; i ++ )
	{
		if( GSYS.DrawSetting.ShadowMap[ i ] == SmHandle )
		{
			Graphics_ShadowMap_RefreshPSParam() ;
			break ;
		}
	}

	// 終了
	return 0 ;
}

// シャドウマップに適用するぼかし度合いを設定する
extern int NS_SetShadowMapBlurParam( int SmHandle, int Param )
{
	SHADOWMAPDATA *ShadowMap ;

	// エラー判定
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	// 補正値を保存する
	ShadowMap->BlurParam = Param ;

	// 終了
	return 0 ;
}

// シャドウマップを使用した描画時の影をグラデーションさせる範囲を設定する
extern int NS_SetShadowMapGradationParam( int SmHandle, float Param )
{
	SHADOWMAPDATA *ShadowMap ;
	int i ;

	// エラー判定
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	// グラデーションさせる範囲を保存する
	ShadowMap->GradationParam = Param ;

	// 既にシャドウマップを使用中の場合は補正値を変更する
	for( i = 0 ; i < MAX_USE_SHADOWMAP_NUM ; i ++ )
	{
		if( GSYS.DrawSetting.ShadowMap[ i ] == SmHandle )
		{
			Graphics_ShadowMap_RefreshPSParam() ;
			break ;
		}
	}

	// 終了
	return 0 ;
}

// シャドウマップを画面にテスト描画する
#define SETDRAWRECTCODE_DRAWSHADOWMAP\
	if( x1 < x2 ){ DrawRect.left = x1 ; DrawRect.right = x2; }\
	if( y1 < y2 ){ DrawRect.top = y1 ; DrawRect.bottom = y2; }\
\
	SETRECT( DrawRect, x1, y1, x2, y2 ) ;\
	DRAWRECT_DRAWAREA_CLIP

extern int NS_TestDrawShadowMap( int SmHandle, int x1, int y1, int x2, int y2 )
{
	SHADOWMAPDATA *ShadowMap ;
	VERTEX_2D DrawVert[ 6 ] ;
	int Ret, Flag ;
	float AdjustPos ;

	CheckActiveState() ;

	// アドレスの取得
	if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
		return -1 ;

	Ret = -1 ;
	Flag = GSYS.Setting.ValidHardware ;

	AdjustPos = GSYS.HardInfo.DrawFloatCoordType == DX_DRAWFLOATCOORDTYPE_DIRECT3D9 ? -0.5f : 0.0f ;

	DrawVert[ 0 ].rhw = 1.0f ;
	DrawVert[ 0 ].pos.z = GSYS.DrawSetting.DrawZ ;
	DrawVert[ 0 ].color = 0xffffffff ;

	DrawVert[ 1 ] = DrawVert[ 0 ] ;
	DrawVert[ 2 ] = DrawVert[ 0 ] ;
	DrawVert[ 3 ] = DrawVert[ 0 ] ;

	DrawVert[ 0 ].pos.x = ( float )x1 + AdjustPos ;
	DrawVert[ 0 ].pos.y = ( float )y1 + AdjustPos ;
	DrawVert[ 0 ].u = 0.0f ;
	DrawVert[ 0 ].v = 0.0f ;

	DrawVert[ 1 ].pos.x = ( float )x2 + AdjustPos ;
	DrawVert[ 1 ].pos.y = ( float )y1 + AdjustPos ;
	DrawVert[ 1 ].u = 1.0f ;
	DrawVert[ 1 ].v = 0.0f ;

	DrawVert[ 2 ].pos.x = ( float )x1 + AdjustPos ;
	DrawVert[ 2 ].pos.y = ( float )y2 + AdjustPos ;
	DrawVert[ 2 ].u = 0.0f ;
	DrawVert[ 2 ].v = 1.0f ;

	DrawVert[ 3 ].pos.x = ( float )x2 + AdjustPos ;
	DrawVert[ 3 ].pos.y = ( float )y2 + AdjustPos ;
	DrawVert[ 3 ].u = 1.0f ;
	DrawVert[ 3 ].v = 1.0f ;

	DrawVert[ 4 ] = DrawVert[ 2 ] ;
	DrawVert[ 5 ] = DrawVert[ 1 ] ;

	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive2D_PF( DrawVert, 6, DX_PRIMTYPE_TRIANGLELIST, ( IMAGEDATA * )SmHandle, FALSE, FALSE, FALSE, FALSE, 0 ),
		0,
		SETDRAWRECTCODE_DRAWSHADOWMAP,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}


























// グラフィックハンドルへの画像転送関数

// 画像データの転送
extern	int NS_BltBmpToGraph( const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, int CopyPointX, int CopyPointY, int GrHandle )
{
	return NS_BltBmpOrGraphImageToGraph( BmpColorData, RgbBmp, AlphaBmp,
											TRUE, NULL, NULL,
											CopyPointX, CopyPointY, GrHandle ) ;
}

// 分割画像へのＢＭＰの転送
extern	int NS_BltBmpToDivGraph( const COLORDATA *BmpColorData, HBITMAP RgbBmp, HBITMAP AlphaBmp, int AllNum, int XNum, int YNum, int Width, int Height, const int *GrHandle, int ReverseFlag )
{
	return NS_BltBmpOrGraphImageToDivGraph( BmpColorData, RgbBmp, AlphaBmp,
												TRUE, NULL, NULL,
												AllNum, XNum, YNum, Width, Height, GrHandle, ReverseFlag ) ;
}

// ＢＭＰ か GraphImage を画像に転送
extern int NS_BltBmpOrGraphImageToGraph(
	const COLORDATA	*BmpColorData,
	      HBITMAP	RgbBmp,
	      HBITMAP	AlphaBmp,
	      int		BmpFlag,
	const BASEIMAGE	*RgbBaseImage,
	const BASEIMAGE	*AlphaBaseImage,
	      int		CopyPointX,
	      int		CopyPointY,
	      int		GrHandle
)
{
	int			Result ;
#ifdef __WINDOWS__
	BITMAP		Bmp ;
	BASEIMAGE	TempBaseRGB ;
	BASEIMAGE	TempBaseAlpha = { 0 } ;
#endif // __WINDOWS__

	if( GSYS.InitializeFlag == FALSE ) return -1 ;

	// Bitmap の場合は、BASEIMAGE の情報を作成する
	if( BmpFlag )
	{
#ifdef __WINDOWS__
		GetObject( RgbBmp, sizeof( BITMAP ), ( void * )&Bmp ) ;
		TempBaseRGB.GraphData      = Bmp.bmBits ;
		TempBaseRGB.Width          = Bmp.bmWidth ;
		TempBaseRGB.Height         = Bmp.bmHeight ;
		TempBaseRGB.Pitch          = ( Bmp.bmWidthBytes + 3 ) / 4 * 4 ;
		TempBaseRGB.ColorData      = *BmpColorData ;
		TempBaseRGB.MipMapCount    = 0 ;
		TempBaseRGB.GraphDataCount = 0 ;

		RgbBaseImage = &TempBaseRGB ;

		if( AlphaBmp )
		{
			GetObject( AlphaBmp, sizeof( BITMAP ), ( void * )&Bmp ) ;
			TempBaseAlpha.GraphData      = Bmp.bmBits ;
			TempBaseAlpha.Width          = Bmp.bmWidth ;
			TempBaseAlpha.Height         = Bmp.bmHeight ;
			TempBaseAlpha.Pitch          = ( Bmp.bmWidthBytes + 3 ) / 4 * 4 ;
			TempBaseAlpha.ColorData      = *BmpColorData ;
			TempBaseAlpha.MipMapCount    = 0 ;
			TempBaseAlpha.GraphDataCount = 0 ;

			AlphaBaseImage = &TempBaseAlpha ;
		}
		else
		{
			AlphaBaseImage = NULL ;
		}
#else // __WINDOWS__
		return -1 ;
#endif // __WINDOWS__
	}

	Result = Graphics_Image_BltBmpOrGraphImageToGraphBase( 
				RgbBaseImage,
				AlphaBaseImage,
				CopyPointX,
				CopyPointY,
				GrHandle,
				GSYS.CreateImage.NotUseTransColor ? FALSE : TRUE
			) ;

	if( Result == 0 )
	{
		if( BmpFlag )
		{
			Graphics_Image_SetGraphBaseInfo( GrHandle, NULL, BmpColorData, RgbBmp, AlphaBmp, NULL, 0, NULL, 0, NULL, NULL, FALSE, -1, FALSE ) ;
		}
		else
		{
			Graphics_Image_SetGraphBaseInfo( GrHandle, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, RgbBaseImage, AlphaBaseImage, FALSE, -1, FALSE ) ;
		}
	}

	return Result ;
}

// ＢＭＰ か GraphImage を画像に転送
extern	int NS_BltBmpOrGraphImageToGraph2(
	const COLORDATA	*BmpColorData,
	      HBITMAP	RgbBmp,
	      HBITMAP	AlphaBmp,
	      int		BmpFlag,
	const BASEIMAGE	*RgbBaseImage,
	const BASEIMAGE	*AlphaBaseImage,
	const RECT		*SrcRect,
	      int		DestX,
	      int		DestY,
	      int		GrHandle
)
{
#ifdef __WINDOWS__
	BITMAP			Bmp ;
	BASEIMAGE		TempBaseRGB ;
	BASEIMAGE		TempBaseAlpha = { 0 } ;
#endif // __WINDOWS__

	// Bitmap の場合は、BASEIMAGE の情報を作成する
	if( BmpFlag )
	{
#ifdef __WINDOWS__
		GetObject( RgbBmp, sizeof( BITMAP ), ( void * )&Bmp ) ;
		TempBaseRGB.GraphData      = Bmp.bmBits ;
		TempBaseRGB.Width          = Bmp.bmWidth ;
		TempBaseRGB.Height         = Bmp.bmHeight ;
		TempBaseRGB.Pitch          = ( Bmp.bmWidthBytes + 3 ) / 4 * 4 ;
		TempBaseRGB.ColorData      = *BmpColorData ;
		TempBaseRGB.MipMapCount    = 0 ;
		TempBaseRGB.GraphDataCount = 0 ;

		RgbBaseImage = &TempBaseRGB ;

		if( AlphaBmp )
		{
			GetObject( AlphaBmp, sizeof( BITMAP ), ( void * )&Bmp ) ;
			TempBaseAlpha.GraphData      = Bmp.bmBits ;
			TempBaseAlpha.Width          = Bmp.bmWidth ;
			TempBaseAlpha.Height         = Bmp.bmHeight ;
			TempBaseAlpha.Pitch          = ( Bmp.bmWidthBytes + 3 ) / 4 * 4 ;
			TempBaseAlpha.ColorData      = *BmpColorData ;
			TempBaseAlpha.MipMapCount    = 0 ;
			TempBaseAlpha.GraphDataCount = 0 ;

			AlphaBaseImage = &TempBaseAlpha ;
		}
		else
		{
			AlphaBaseImage = NULL ;
		}
#else // __WINDOWS__
		return -1 ;
#endif // __WINDOWS__
	}

	// 画像の転送
	return Graphics_Image_BltBmpOrGraphImageToGraph2Base(
				RgbBaseImage,
				AlphaBaseImage,
				SrcRect,
				DestX,
				DestY,
				GrHandle,
				GSYS.CreateImage.NotUseTransColor ? FALSE : TRUE ) ;
}

// 分割画像への ＢＭＰ か GraphImage の転送
extern	int NS_BltBmpOrGraphImageToDivGraph(
	const COLORDATA	*BmpColorData,
	      HBITMAP	RgbBmp,
	      HBITMAP	AlphaBmp,
	      int		BmpFlag,
	const BASEIMAGE	*RgbBaseImage,
	const BASEIMAGE	*AlphaBaseImage,
	      int		AllNum,
	      int		XNum,
	      int		YNum,
	      int		Width,
	      int		Height,
	const int		*GrHandle,
	      int		ReverseFlag
)
{
	int Result ;
	int i ;
#ifdef __WINDOWS__
	BITMAP		Bmp ;
	BASEIMAGE	TempBaseRGB ;
	BASEIMAGE	TempBaseAlpha = { 0 } ;
#endif // __WINDOWS__

	if( GSYS.InitializeFlag == FALSE ) return -1 ;

	// Bitmap の場合は、BASEIMAGE の情報を作成する
	if( BmpFlag )
	{
#ifdef __WINDOWS__
		GetObject( RgbBmp, sizeof( BITMAP ), ( void * )&Bmp ) ;
		TempBaseRGB.GraphData      = Bmp.bmBits ;
		TempBaseRGB.Width          = Bmp.bmWidth ;
		TempBaseRGB.Height         = Bmp.bmHeight ;
		TempBaseRGB.Pitch          = ( Bmp.bmWidthBytes + 3 ) / 4 * 4 ;
		TempBaseRGB.ColorData      = *BmpColorData ;
		TempBaseRGB.MipMapCount    = 0 ;
		TempBaseRGB.GraphDataCount = 0 ;

		RgbBaseImage = &TempBaseRGB ;

		if( AlphaBmp )
		{
			GetObject( AlphaBmp, sizeof( BITMAP ), ( void * )&Bmp ) ;
			TempBaseAlpha.GraphData      = Bmp.bmBits ;
			TempBaseAlpha.Width          = Bmp.bmWidth ;
			TempBaseAlpha.Height         = Bmp.bmHeight ;
			TempBaseAlpha.Pitch          = ( Bmp.bmWidthBytes + 3 ) / 4 * 4 ;
			TempBaseAlpha.ColorData      = *BmpColorData ;
			TempBaseAlpha.MipMapCount    = 0 ;
			TempBaseAlpha.GraphDataCount = 0 ;

			AlphaBaseImage = &TempBaseAlpha ;
		}
		else
		{
			AlphaBaseImage = NULL ;
		}
#else // __WINDOWS__
		return -1 ;
#endif // __WINDOWS__
	}

	// 転送処理
	Result = Graphics_Image_BltBmpOrGraphImageToDivGraphBase(
		RgbBaseImage,
		AlphaBaseImage,
		AllNum,
		XNum,
		YNum,
		Width,
		Height,
		GrHandle,
		ReverseFlag,
		GSYS.CreateImage.NotUseTransColor ? FALSE : TRUE,
		FALSE
	) ;

	// 最初の画像にファイル情報をセット
	if( BmpFlag )
	{
		Graphics_Image_SetGraphBaseInfo( GrHandle[ 0 ], NULL, BmpColorData, RgbBmp, AlphaBmp, NULL, 0, NULL, 0, NULL, NULL, FALSE, -1, FALSE ) ;
	}
	else
	{
		Graphics_Image_SetGraphBaseInfo( GrHandle[ 0 ], NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, RgbBaseImage, AlphaBaseImage, FALSE, -1, FALSE ) ;
	}
	for( i = 1 ; i < AllNum ; i ++ )
	{
		Graphics_Image_SetGraphBaseInfo( GrHandle[ i ], NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, NULL, NULL, FALSE, GrHandle[ 0 ], FALSE ) ;
	}

	// 終了
	return Result ;
}



















// 画像からグラフィックハンドルを作成する関数

// 画像を読みこむ
extern int NS_LoadBmpToGraph( const TCHAR *FileName, int TextureFlag, int ReverseFlag, int SurfaceMode )
{
#ifdef UNICODE
	return LoadBmpToGraph_WCHAR_T(
		FileName, TextureFlag, ReverseFlag, SurfaceMode
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = LoadBmpToGraph_WCHAR_T(
		UseFileNameBuffer, TextureFlag, ReverseFlag, SurfaceMode
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// 画像を読みこむ
extern int LoadBmpToGraph_WCHAR_T( const wchar_t *FileName, int TextureFlag, int ReverseFlag, int SurfaceMode )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_LoadBmpToGraph_UseGParam( &GParam, FALSE, -1, FileName, TextureFlag, ReverseFlag, SurfaceMode, GetASyncLoadFlag() ) ;
}

// ＢＭＰファイルのメモリへの読みこみ（フラグつき)
extern int NS_LoadGraph( const TCHAR *GraphName, int NotUse3DFlag )
{
	return NS_LoadBmpToGraph( GraphName, !NotUse3DFlag, FALSE ) ;
}

// ＢＭＰファイルのメモリへの読みこみ（フラグつき)
extern int LoadGraph_WCHAR_T( const wchar_t *GraphName, int NotUse3DFlag )
{
	return LoadBmpToGraph_WCHAR_T( GraphName, !NotUse3DFlag, FALSE ) ;
}

// ＢＭＰファイルのメモリへの反転読みこみ（フラグつき)
extern int NS_LoadReverseGraph( const TCHAR *GraphName, int NotUse3DFlag )
{
	return NS_LoadBmpToGraph( GraphName, !NotUse3DFlag, TRUE ) ;
}

// ＢＭＰファイルのメモリへの反転読みこみ（フラグつき)
extern int LoadReverseGraph_WCHAR_T( const wchar_t *GraphName, int NotUse3DFlag )
{
	return LoadBmpToGraph_WCHAR_T( GraphName, !NotUse3DFlag, TRUE ) ;
}

// ＢＭＰの分割読みこみ（フラグつき）
extern int NS_LoadDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf, int NotUse3DFlag )
{
	return NS_LoadDivBmpToGraph( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, !NotUse3DFlag, FALSE ) ;
}

// ＢＭＰの分割読みこみ（フラグつき）
extern int LoadDivGraph_WCHAR_T( const wchar_t *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf, int NotUse3DFlag )
{
	return LoadDivBmpToGraph_WCHAR_T( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, !NotUse3DFlag, FALSE ) ;
}

// ＢＭＰの分割読みこみ
extern int NS_LoadDivBmpToGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag )
{
#ifdef UNICODE
	return LoadDivBmpToGraph_WCHAR_T(
		FileName, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = LoadDivBmpToGraph_WCHAR_T(
		UseFileNameBuffer, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// ＢＭＰの分割読みこみ
extern int LoadDivBmpToGraph_WCHAR_T( const wchar_t *FileName, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_LoadDivBmpToGraph_UseGParam( &GParam, FALSE, FileName, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, GetASyncLoadFlag(), FALSE ) ;
}

// ＢＭＰの反転分割読みこみ（フラグつき）
extern int NS_LoadReverseDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf, int NotUse3DFlag )
{
	return NS_LoadDivBmpToGraph( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, !NotUse3DFlag, TRUE ) ;
}

// ＢＭＰの反転分割読みこみ（フラグつき）
extern int LoadReverseDivGraph_WCHAR_T( const wchar_t *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf, int NotUse3DFlag )
{
	return LoadDivBmpToGraph_WCHAR_T( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, !NotUse3DFlag, TRUE ) ;
}

// 画像ファイルからブレンド用画像を読み込む
extern int NS_LoadBlendGraph( const TCHAR *FileName )
{
#ifdef UNICODE
	return LoadBlendGraph_WCHAR_T(
		FileName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = LoadBlendGraph_WCHAR_T(
		UseFileNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// 画像ファイルからブレンド用画像を読み込む
extern int LoadBlendGraph_WCHAR_T( const wchar_t *FileName )
{
	int Result, UseBlendGraphFlag ;

	UseBlendGraphFlag = NS_GetUseBlendGraphCreateFlag() ;
	NS_SetUseBlendGraphCreateFlag( TRUE ) ;
	
	Result = LoadGraph_WCHAR_T( FileName ) ;
	
	NS_SetUseBlendGraphCreateFlag( UseBlendGraphFlag ) ;
	
	return Result ;
}

#ifdef __WINDOWS__

// リソースからＢＭＰファイルを読み込む
extern int NS_LoadGraphToResource( int ResourceID )
{
	BITMAPINFO *BmpInfo ;
	void *GraphData ;
	int NewHandle ;

	// リソースから画像ハンドル作成に必要なデータを取得する
	if( GetBmpImageToResource( ResourceID, &BmpInfo, &GraphData ) == -1 )
		return -1 ;

	// グラフィックの作成
	NewHandle = NS_CreateGraphFromBmp( BmpInfo, GraphData ) ;

	// メモリの解放
	DXFREE( BmpInfo ) ;
	DXFREE( GraphData ) ;

	// ハンドルを返す
	return NewHandle ;
}

// リソースから画像データを分割読み込みする
extern int NS_LoadDivGraphToResource( int ResourceID, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf )
{
	BITMAPINFO *BmpInfo ;
	void *GraphData ;

	// リソースから画像ハンドル作成に必要なデータを取得する
	if( GetBmpImageToResource( ResourceID, &BmpInfo, &GraphData ) == -1 )
		return -1 ;

	// グラフィックの作成
	NS_CreateDivGraphFromBmp( BmpInfo, GraphData, AllNum, XNum, YNum, XSize, YSize, HandleBuf ) ;

	// メモリの解放
	DXFREE( BmpInfo ) ;
	DXFREE( GraphData ) ;

	// 成功
	return 0 ;
}

// リソースからグラフィックデータを読み込む
extern int NS_LoadGraphToResource( const TCHAR *ResourceName, const TCHAR *ResourceType )
{
#ifdef UNICODE
	return LoadGraphToResource_WCHAR_T(
		ResourceName, ResourceType
	) ;
#else
	int Result = -1 ;

	TCHAR_TO_WCHAR_T_STRING_BEGIN( ResourceName )
	TCHAR_TO_WCHAR_T_STRING_BEGIN( ResourceType )

	TCHAR_TO_WCHAR_T_STRING_SETUP( ResourceName, goto ERR )
	TCHAR_TO_WCHAR_T_STRING_SETUP( ResourceType, goto ERR )

	Result = LoadGraphToResource_WCHAR_T(
		UseResourceNameBuffer, UseResourceTypeBuffer
	) ;

ERR :

	TCHAR_TO_WCHAR_T_STRING_END( ResourceName )
	TCHAR_TO_WCHAR_T_STRING_END( ResourceType )

	return Result ;
#endif
}

// リソースからグラフィックデータを読み込む
extern int LoadGraphToResource_WCHAR_T( const wchar_t *ResourceName, const wchar_t *ResourceType )
{
	void *Image ;
	int ImageSize ;
	int Result ;

	// リソースの情報を取得
	if( GetResourceInfo_WCHAR_T( ResourceName, ResourceType, &Image, &ImageSize ) < 0 )
	{
		return -1 ;
	}

	// ハンドルの作成
	Result = NS_CreateGraphFromMem( Image, ImageSize ) ;

	return Result ;
}

// リソースからグラフィックデータを分割読み込みする
extern int NS_LoadDivGraphToResource( const TCHAR *ResourceName, const TCHAR *ResourceType, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf )
{
#ifdef UNICODE
	return LoadDivGraphToResource_WCHAR_T(
		ResourceName, ResourceType, AllNum, XNum, YNum, XSize, YSize, HandleBuf
	) ;
#else
	int Result = -1 ;

	TCHAR_TO_WCHAR_T_STRING_BEGIN( ResourceName )
	TCHAR_TO_WCHAR_T_STRING_BEGIN( ResourceType )

	TCHAR_TO_WCHAR_T_STRING_SETUP( ResourceName, goto ERR )
	TCHAR_TO_WCHAR_T_STRING_SETUP( ResourceType, goto ERR )

	Result = LoadDivGraphToResource_WCHAR_T(
		UseResourceNameBuffer, UseResourceTypeBuffer, AllNum, XNum, YNum, XSize, YSize, HandleBuf
	) ;

ERR :

	TCHAR_TO_WCHAR_T_STRING_END( ResourceName )
	TCHAR_TO_WCHAR_T_STRING_END( ResourceType )

	return Result ;
#endif
}

// リソースからグラフィックデータを分割読み込みする
extern int LoadDivGraphToResource_WCHAR_T( const wchar_t *ResourceName, const wchar_t *ResourceType, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf )
{
	void *Image ;
	int ImageSize ;
	int Result ;

	// リソースの情報を取得
	if( GetResourceInfo_WCHAR_T( ResourceName, ResourceType, &Image, &ImageSize ) < 0 ) return -1 ;

	// ハンドルの作成
	Result = NS_CreateDivGraphFromMem( Image, ImageSize, AllNum, XNum, YNum, XSize, YSize, HandleBuf ) ;

	return Result ;
}

#endif // __WINDOWS__

// メモリ上のグラフィックイメージからグラフィックハンドルを作成する
extern int NS_CreateGraphFromMem( const void *MemImage, int MemImageSize, const void *AlphaImage, int AlphaImageSize, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, FALSE, -1, MemImage, MemImageSize, AlphaImage, AlphaImageSize, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// メモリ上のグラフィックイメージから既存のグラフィックハンドルにデータを転送する
extern int NS_ReCreateGraphFromMem( const void *MemImage, int MemImageSize, int GrHandle, const void *AlphaImage, int AlphaImageSize, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, TRUE, GrHandle, MemImage, MemImageSize, AlphaImage, AlphaImageSize, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// メモリ上のグラフィックイメージから分割グラフィックハンドルを作成する
extern int NS_CreateDivGraphFromMem( const void *MemImage, int MemImageSize, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag, const void *AlphaImage, int AlphaImageSize )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateDivGraphFromMem_UseGParam( &GParam, FALSE, MemImage, MemImageSize, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, AlphaImage, AlphaImageSize, GetASyncLoadFlag(), FALSE ) ;
}

// メモリ上のグラフィックイメージから既存の分割グラフィックハンドルにデータを転送する
extern int NS_ReCreateDivGraphFromMem( const void *MemImage, int MemImageSize, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag, const void *AlphaImage, int AlphaImageSize )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateDivGraphFromMem_UseGParam( &GParam, TRUE, MemImage, MemImageSize, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, AlphaImage, AlphaImageSize, GetASyncLoadFlag(), FALSE ) ;
}

// ビットマップデータからグラフィックハンドルを作成する
extern int NS_CreateGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, const BITMAPINFO *AlphaInfo, const void *AlphaData, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateGraphFromBmp_UseGParam( &GParam, FALSE, -1, BmpInfo, GraphData, AlphaInfo, AlphaData, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// ビットマップデータから既存のグラフィックハンドルにデータを転送する
extern int NS_ReCreateGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, int GrHandle, const BITMAPINFO *AlphaInfo, const void *AlphaData, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateGraphFromBmp_UseGParam( &GParam, TRUE, GrHandle, BmpInfo, GraphData, AlphaInfo, AlphaData, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// ビットマップデータから分割グラフィックハンドルを作成する
extern int NS_CreateDivGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag, const BITMAPINFO *AlphaInfo, const void *AlphaData )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateDivGraphFromBmp_UseGParam( &GParam, FALSE, BmpInfo, GraphData, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, AlphaInfo, AlphaData, GetASyncLoadFlag() ) ;
}

// ビットマップデータから既存の分割グラフィックハンドルにデータを転送する
extern int NS_ReCreateDivGraphFromBmp( const BITMAPINFO *BmpInfo, const void *GraphData, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag, const BITMAPINFO *AlphaInfo, const void *AlphaData )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateDivGraphFromBmp_UseGParam( &GParam, TRUE, BmpInfo, GraphData, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, AlphaInfo, AlphaData, GetASyncLoadFlag() ) ;
}

// GraphImage データからサイズを割り出し、それに合ったグラフィックハンドルを作成する
extern int NS_CreateDXGraph( const BASEIMAGE *RgbBaseImage, const BASEIMAGE *AlphaBaseImage, int TextureFlag )
{
	SETUP_GRAPHHANDLE_GPARAM GParam ;
	int NewGraphHandle ;
	int Result ;

	CheckActiveState() ;

	// ハンドルの作成
	NewGraphHandle = Graphics_Image_AddHandle( FALSE ) ;
	if( NewGraphHandle == -1 )
	{
		return -1 ;
	}

	Graphics_Image_InitSetupGraphHandleGParam( &GParam ) ;

	Result = Graphics_Image_CreateDXGraph_UseGParam( &GParam, NewGraphHandle, RgbBaseImage, AlphaBaseImage, TextureFlag ) ;
	if( Result < 0 )
	{
		NS_DeleteGraph( NewGraphHandle ) ;
		return -1 ;
	}

	return NewGraphHandle ;
}

// GraphImageデータからグラフィックハンドルを作成する
extern int NS_CreateGraphFromGraphImage( const BASEIMAGE *BaseImage, int TextureFlag, int ReverseFlag )
{
	return NS_CreateGraphFromGraphImage( BaseImage, NULL, TextureFlag, ReverseFlag ) ;
}

// GraphImageデータからグラフィックハンドルを作成する
extern int NS_CreateGraphFromGraphImage( const BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, FALSE, -1, BaseImage, AlphaBaseImage, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ; 
}

// GraphImageデータから既存のグラフィックハンドルにデータを転送する
extern int NS_ReCreateGraphFromGraphImage( const BASEIMAGE *BaseImage, int GrHandle, int TextureFlag, int ReverseFlag )
{
	return NS_ReCreateGraphFromGraphImage( BaseImage, NULL, GrHandle, TextureFlag, ReverseFlag ) ;
}

// GraphImageデータから既存のグラフィックハンドルにデータを転送する
extern int NS_ReCreateGraphFromGraphImage( const BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int GrHandle, int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, TRUE, GrHandle, BaseImage, AlphaBaseImage, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// GraphImageデータから分割グラフィックハンドルを作成する
extern int NS_CreateDivGraphFromGraphImage( BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	return NS_CreateDivGraphFromGraphImage( BaseImage, NULL, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag ) ;
}

// GraphImageデータから分割グラフィックハンドルを作成する
extern int NS_CreateDivGraphFromGraphImage( BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateDivGraphFromGraphImage_UseGParam( &GParam, FALSE, BaseImage, AlphaBaseImage, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// GraphImageデータから既存の分割グラフィックハンドルにデータを転送する
extern int NS_ReCreateDivGraphFromGraphImage( BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	return NS_ReCreateDivGraphFromGraphImage( BaseImage, NULL, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag ) ;
}

// GraphImageデータから既存の分割グラフィックハンドルにデータを転送する
extern int NS_ReCreateDivGraphFromGraphImage( BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf,int TextureFlag, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_CreateDivGraphFromGraphImage_UseGParam( &GParam, TRUE, BaseImage, AlphaBaseImage, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, GetASyncLoadFlag() ) ;
}

// メモリ上のグラフィックデータからグラフィックハンドルを作成する
extern int NS_CreateGraph( int Width, int Height, int Pitch, const void *GraphData, const void *AlphaData, int GrHandle )
{
	BITMAPINFO	BmpInfo ;
	BYTE *DData, *AData, *SData, *DestData ;
	int i ;
	int DPitch, SPitch ;
	int NewGraph ;

	// ピッチ作成
	{
		int Byte ; 
		
		Byte = Width * 3 ;

		SPitch = ( Pitch == 0 ) ? Width * 3 : Pitch ;
		DPitch = ( ( Byte + 3 ) / 4 ) * 4 ;
	}

	// もしピッチが同じならグラフィックデータをコピーする必要なし
	AData = NULL ;
	if( SPitch != DPitch )
	{
		// RGBデータの作成
		{
			// キャスト
			SData = ( BYTE * )GraphData ;

			// グラフィックデータを複製
			if( ( DestData = DData = ( BYTE *)DXCALLOC( ( size_t )( Height * DPitch ) ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, DData += DPitch )
				_MEMCPY( DData, SData, ( size_t )SPitch ) ;
			DData = DestData ;
		}

		// アルファデータの作成
		if( AlphaData != NULL )
		{
			// キャスト
			SData = ( BYTE * )AlphaData ;

			// グラフィックデータを複製
			if( ( DestData = AData = ( BYTE *)DXCALLOC( ( size_t )( Height * DPitch ) ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, AData += DPitch )
				_MEMCPY( AData, SData, ( size_t )SPitch ) ;
			AData = DestData ;
		}
	}
	else
	{
		DData = ( BYTE * )GraphData ;
		AData = ( BYTE * )AlphaData ;
	}

	// 新グラフィックを作成
	_MEMSET( &BmpInfo, 0, sizeof( BmpInfo ) ) ;
	BmpInfo.bmiHeader.biSize	= sizeof( BITMAPINFOHEADER ) ;
	BmpInfo.bmiHeader.biWidth	= Width ;
	BmpInfo.bmiHeader.biHeight	= -Height ;
	BmpInfo.bmiHeader.biPlanes	= 1 ;
	BmpInfo.bmiHeader.biBitCount= 24 ;

	if( GrHandle != -1 )
	{
		NS_ReCreateGraphFromBmp( &BmpInfo, DData, GrHandle, AData != NULL ? &BmpInfo : NULL, AData ) ;
		NewGraph = 0 ;
	}
	else
	{
		NewGraph = NS_CreateGraphFromBmp( &BmpInfo, DData, AData != NULL ? &BmpInfo : NULL, AData ) ;
	}

	// メモリを解放
	if( SPitch != DPitch )
	{
		DXFREE( DData ) ;
		DXFREE( AData ) ;
	}

	// 終了
	return NewGraph ;
}

// メモリ上のグラフィックデータから分割グラフィックハンドルを作成する
extern int NS_CreateDivGraph( int Width, int Height, int Pitch, const void *GraphData, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, const void *AlphaData )
{
	BITMAPINFO	BmpInfo ;
	BYTE *DData, *AData, *SData, *DestData ;
	int i, Result ;
	int DPitch, SPitch ;

	// ピッチ作成
	{
		int Byte ; 
		
		Byte = Width * 3 ;

		SPitch = ( Pitch == 0 ) ? Width * 3 : Pitch ;
		DPitch = ( ( Byte + 3 ) / 4 ) * 4 ;
	}

	// もしピッチが同じならグラフィックデータをコピーする必要なし
	AData = NULL ;
	if( SPitch != DPitch )
	{
		// RGBデータの作成
		{
			// キャスト
			SData = ( BYTE * )GraphData ;

			// グラフィックデータを複製
			if( ( DestData = DData = ( BYTE *)DXCALLOC( ( size_t )( Height * DPitch ) ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, DData += DPitch )
				_MEMCPY( DData, SData, ( size_t )SPitch ) ;
			DData = DestData ;
		}

		// アルファデータの作成
		if( AlphaData != NULL )
		{
			// キャスト
			SData = ( BYTE * )AlphaData ;

			// グラフィックデータを複製
			if( ( DestData = AData = ( BYTE *)DXCALLOC( ( size_t )( Height * DPitch ) ) ) == NULL ) return -1 ;
			for( i = 0 ; i < Height ; i ++, SData += SPitch, AData += DPitch )
				_MEMCPY( AData, SData, ( size_t )SPitch ) ;
			AData = DestData ;
		}
	}
	else
	{
		DData = ( BYTE * )GraphData ;
		AData = ( BYTE * )AlphaData ;
	}

	// 新グラフィックを作成
	_MEMSET( &BmpInfo, 0, sizeof( BmpInfo ) ) ;
	BmpInfo.bmiHeader.biSize	= sizeof( BITMAPINFOHEADER ) ;
	BmpInfo.bmiHeader.biWidth	= Width ;
	BmpInfo.bmiHeader.biHeight	= -Height ;
	BmpInfo.bmiHeader.biPlanes	= 1 ;
	BmpInfo.bmiHeader.biBitCount= 24 ;

	Result = NS_CreateDivGraphFromBmp( &BmpInfo, DData, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TRUE, FALSE, &BmpInfo, AData ) ;

	// メモリを解放
	if( SPitch != DPitch )
	{
		DXFREE( DData ) ;
		DXFREE( AData ) ;
	}

	// 終了
	return Result ;
}

// メモリ上のグラフィックデータからグラフィックハンドルを再作成する
extern int NS_ReCreateGraph( int Width, int Height, int Pitch, const void *GraphData, int GrHandle, const void *AlphaData )
{
	return NS_CreateGraph( Width, Height, Pitch, GraphData, AlphaData, GrHandle ) ;
}

#ifndef DX_NON_SOFTIMAGE

// ソフトウエアで扱うイメージからブレンド用画像グラフィックハンドルを作成する
extern	int		NS_CreateBlendGraphFromSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;
	LOADGRAPH_GPARAM GParam ;

	// アドレスの取得
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;
	GParam.CreateGraphGParam.InitHandleGParam.BlendImageCreateFlag = TRUE ;

	return Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, FALSE, -1, &SoftImg->BaseImage, NULL, TRUE, FALSE, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱うイメージからグラフィックハンドルを作成する
extern	int		NS_CreateGraphFromSoftImage( int SIHandle )
{
	SOFTIMAGE *SoftImg ;

	// アドレスの取得
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_CreateGraphFromBaseImage( &SoftImg->BaseImage ) ;
}

// ソフトウエアで扱うイメージの指定の領域を使ってグラフィックハンドルを作成する( -1:エラー  -1以外:グラフィックハンドル )
extern	int		NS_CreateGraphFromRectSoftImage( int SIHandle, int x, int y, int SizeX, int SizeY )
{
	SOFTIMAGE *SoftImg ;

	// アドレスの取得
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_CreateGraphFromRectBaseImage( &SoftImg->BaseImage, x, y, SizeX, SizeY ) ;
}

// ソフトウエアで扱うイメージから既存のグラフィックハンドルに画像データを転送する
extern	int		NS_ReCreateGraphFromSoftImage( int SIHandle, int GrHandle )
{
	SOFTIMAGE *SoftImg ;

	// アドレスの取得
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ReCreateGraphFromBaseImage( &SoftImg->BaseImage, GrHandle ) ;
}

// ソフトウエアで扱うイメージから既存のグラフィックハンドルに画像データを転送する
extern	int		NS_ReCreateGraphFromRectSoftImage( int SIHandle, int x, int y, int SizeX, int SizeY, int GrHandle )
{
	SOFTIMAGE *SoftImg ;

	// アドレスの取得
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_ReCreateGraphFromRectBaseImage( &SoftImg->BaseImage, x, y, SizeX, SizeY, GrHandle ) ;
}

// ソフトウエアで扱うイメージから分割グラフィックハンドルを作成する
extern	int		NS_CreateDivGraphFromSoftImage( int SIHandle, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf )
{
	SOFTIMAGE *SoftImg ;

	// アドレスの取得
	if( SFTIMGCHK( SIHandle, SoftImg ) )
		return -1 ;

	return NS_CreateDivGraphFromBaseImage( &SoftImg->BaseImage, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf ) ;
}

#endif // DX_NON_SOFTIMAGE

// 基本イメージデータからグラフィックハンドルを作成する
extern	int		NS_CreateGraphFromBaseImage( const BASEIMAGE *BaseImage )
{
	return NS_CreateGraphFromGraphImage( BaseImage ) ;
}

// 基本イメージの指定の領域を使ってグラフィックハンドルを作成する
extern	int		NS_CreateGraphFromRectBaseImage( const BASEIMAGE *BaseImage, int x, int y, int SizeX, int SizeY )
{
	BASEIMAGE DummyImage ;

	// 位置やサイズが不正だったらエラー
	if( SizeX <= 0 || SizeY <= 0 ||
		x < 0 || x + SizeX >  BaseImage->Width ||
		y < 0 || y + SizeY >  BaseImage->Height ) 
		return -1 ;

	// ダミーの基本イメージをでっち上げてグラフィックハンドルを作成する
	DummyImage.ColorData = BaseImage->ColorData ;
	DummyImage.Width = SizeX ;
	DummyImage.Height = SizeY ;
	DummyImage.Pitch = BaseImage->Pitch ;
	DummyImage.MipMapCount = 0 ;
	DummyImage.GraphDataCount = 0 ;
	DummyImage.GraphData = ( BYTE * )BaseImage->GraphData + x * BaseImage->ColorData.PixelByte + y * BaseImage->Pitch ;
	return NS_CreateGraphFromBaseImage( &DummyImage ) ;
}

// 基本イメージデータから既存のグラフィックハンドルに画像データを転送する
extern	int		NS_ReCreateGraphFromBaseImage( const BASEIMAGE *BaseImage, int GrHandle )
{
	return NS_ReCreateGraphFromGraphImage( BaseImage, GrHandle ) ;
}

// 基本イメージの指定の領域を使って既存のグラフィックハンドルに画像データを転送する
extern	int		NS_ReCreateGraphFromRectBaseImage( const BASEIMAGE *BaseImage, int x, int y, int SizeX, int SizeY, int GrHandle )
{
	BASEIMAGE DummyImage ;

	// 位置やサイズが不正だったらエラー
	if( SizeX <= 0 || SizeY <= 0 ||
		x < 0 || x + SizeX >  BaseImage->Width ||
		y < 0 || y + SizeY >  BaseImage->Height ) 
		return -1 ;

	// ダミーの基本イメージをでっち上げてグラフィックハンドルを作成する
	DummyImage.ColorData = BaseImage->ColorData ;
	DummyImage.Width = SizeX ;
	DummyImage.Height = SizeY ;
	DummyImage.Pitch = BaseImage->Pitch ;
	DummyImage.MipMapCount = 0 ;
	DummyImage.GraphDataCount = 0 ;
	DummyImage.GraphData = ( BYTE * )BaseImage->GraphData + x * BaseImage->ColorData.PixelByte + y * BaseImage->Pitch ;
	return NS_ReCreateGraphFromGraphImage( &DummyImage, GrHandle ) ;
}

// 基本イメージデータから分割グラフィックハンドルを作成する
extern int NS_CreateDivGraphFromBaseImage( BASEIMAGE *BaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf )
{
	return NS_CreateDivGraphFromGraphImage( BaseImage,  AllNum,  XNum,  YNum,  SizeX,  SizeY, HandleBuf ) ;
}

// グラフィックデータへのグラフィックの読み込み
extern int NS_ReloadGraph( const TCHAR *FileName, int GrHandle, int ReverseFlag )
{
#ifdef UNICODE
	return ReloadGraph_WCHAR_T(
		FileName, GrHandle, ReverseFlag
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = ReloadGraph_WCHAR_T(
		UseFileNameBuffer, GrHandle, ReverseFlag
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// グラフィックデータへのグラフィックの読み込み
extern int ReloadGraph_WCHAR_T( const wchar_t *FileName, int GrHandle, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_LoadBmpToGraph_UseGParam( &GParam, TRUE, GrHandle, FileName, TRUE, ReverseFlag, DX_MOVIESURFACE_NORMAL, GetASyncLoadFlag() ) ;
}

// グラフィックデータへのグラフィックの分割読み込み
extern int NS_ReloadDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, const int *HandleBuf, int ReverseFlag )
{
#ifdef UNICODE
	return ReloadDivGraph_WCHAR_T(
		FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, ReverseFlag
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = ReloadDivGraph_WCHAR_T(
		UseFileNameBuffer, AllNum, XNum, YNum, XSize, YSize, HandleBuf, ReverseFlag
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// グラフィックデータへのグラフィックの分割読み込み
extern int ReloadDivGraph_WCHAR_T( const wchar_t *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, const int *HandleBuf, int ReverseFlag )
{
	LOADGRAPH_GPARAM GParam ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	return Graphics_Image_LoadDivBmpToGraph_UseGParam( &GParam, TRUE, FileName, AllNum, XNum, YNum, XSize, YSize, ( int * )HandleBuf, TRUE, ReverseFlag, GetASyncLoadFlag(), FALSE ) ;
}

// グラフィックデータへのグラフィックの反転読み込み
extern int NS_ReloadReverseGraph( const TCHAR *FileName, int GrHandle )
{
	return NS_ReloadGraph( FileName, GrHandle, TRUE ) ;
}

// グラフィックデータへのグラフィックの反転読み込み
extern int ReloadReverseGraph_WCHAR_T( const wchar_t *FileName, int GrHandle )
{
	return ReloadGraph_WCHAR_T( FileName, GrHandle, TRUE ) ;
}

// グラフィックデータへのグラフィックの反転分割読み込み
extern int NS_ReloadReverseDivGraph( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, const int *HandleBuf )
{
	return NS_ReloadDivGraph( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, TRUE ) ;
}

// グラフィックデータへのグラフィックの反転分割読み込み
extern int ReloadReverseDivGraph_WCHAR_T( const wchar_t *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, const int *HandleBuf )
{
	return ReloadDivGraph_WCHAR_T( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, TRUE ) ;
}
























// グラフィックハンドル作成時設定係関数

// SetCreateGraphColorBitDepth の旧名称
extern int NS_SetGraphColorBitDepth( int ColorBitDepth )
{
	return NS_SetCreateGraphColorBitDepth( ColorBitDepth ) ;
}

// GetCreateGraphColorBitDepth の旧名称
extern int NS_GetGraphColorBitDepth( void )
{
	return NS_GetCreateGraphColorBitDepth() ;
}

// 作成するグラフィックの色深度を設定する
extern int NS_SetCreateGraphColorBitDepth( int BitDepth )
{
	// 値を保存する
	GSYS.CreateImage.ColorBitDepth = BitDepth ;
	
	// 終了
	return 0 ;
}

// 作成するグラフィックの色深度を取得する
extern int NS_GetCreateGraphColorBitDepth( void )
{
	return GSYS.CreateImage.ColorBitDepth ;
}

// 作成するグラフィックの１チャンネル辺りのビット深度を設定する
extern int NS_SetCreateGraphChannelBitDepth( int BitDepth )
{
	// 値を保存する
	GSYS.CreateImage.ChannelBitDepth = BitDepth ;
	
	// 終了
	return 0 ;
}

// 作成するグラフィックの１チャンネル辺りのビット深度を取得する
extern int NS_GetCreateGraphChannelBitDepth( void )
{
	return GSYS.CreateImage.ChannelBitDepth ;
}

// 描画可能なグラフィックを作成するかどうかのフラグをセットする( TRUE:作成する  FALSE:作成しない )
extern int NS_SetDrawValidGraphCreateFlag( int Flag )
{
	// フラグを保存する
	GSYS.CreateImage.DrawValidFlag = Flag ;
	
	// 終了
	return 0 ;
}

// 描画可能なグラフィックを作成するかどうかのフラグを取得する
extern int NS_GetDrawValidGraphCreateFlag( void )
{
	return GSYS.CreateImage.DrawValidFlag ;
}

// SetDrawValidGraphCreateFlag の旧名称
extern int NS_SetDrawValidFlagOf3DGraph( int Flag )
{
	return NS_SetDrawValidGraphCreateFlag( Flag ) ;
}

// 画像左上の色を透過色にするかどうかのフラグをセットする
extern int NS_SetLeftUpColorIsTransColorFlag( int Flag )
{
	// フラグをセットする
	GSYS.CreateImage.LeftUpColorIsTransColorFlag = Flag ;

	// 終了
	return 0 ;
}

// ブレンド処理用画像を作成するかどうかのフラグをセットする
extern int NS_SetUseBlendGraphCreateFlag( int Flag )
{
	// フラグを保存する
	GSYS.CreateImage.BlendImageFlag = Flag ;
	
	// 終了
	return 0 ;
}

// ブレンド処理用画像を作成するかどうかのフラグを取得する
extern int NS_GetUseBlendGraphCreateFlag( void )
{
	return GSYS.CreateImage.BlendImageFlag ;
}

// アルファテストを使用するグラフィックを作成するかどうかのフラグをセットする
extern int NS_SetUseAlphaTestGraphCreateFlag( int Flag )
{
	// フラグをセットする
	GSYS.CreateImage.AlphaTestFlag = Flag ;
	BASEIM.AlphaTestImageCreateFlag = Flag ;
	
	// 終了
	return 0 ;
}

// アルファテストを使用するグラフィックを作成するかどうかのフラグを取得する
extern int NS_GetUseAlphaTestGraphCreateFlag( void )
{
	return GSYS.CreateImage.AlphaTestFlag ;
}

// SetUseAlphaTestGraphCreateFlag の旧名称
extern int NS_SetUseAlphaTestFlag( int Flag )
{
	return NS_SetUseAlphaTestGraphCreateFlag( Flag ) ;
}

// GetUseAlphaTestGraphCreateFlag の旧名称
extern int NS_GetUseAlphaTestFlag( void )
{
	return NS_GetUseAlphaTestGraphCreateFlag() ;
}

// キューブマップテクスチャを作成するかどうかのフラグを設定する
extern int NS_SetCubeMapTextureCreateFlag( int Flag )
{
	GSYS.CreateImage.CubeMapFlag = Flag == FALSE ? FALSE : TRUE ;

	// 正常終了
	return 0 ;
}

// キューブマップテクスチャを作成するかどうかのフラグを取得する
extern int NS_GetCubeMapTextureCreateFlag( void )
{
	return 	GSYS.CreateImage.CubeMapFlag ;
}

// SetDrawBlendMode 関数の第一引数に DX_BLENDMODE_NOBLEND を代入した際に、デフォルトでは第二引数は内部で２５５を指定したことになるが、その自動２５５化をしないかどうかを設定する( TRUE:しない(第二引数の値が使用される)   FALSE:する(第二引数の値は無視されて 255 が常に使用される)(デフォルト) )αチャンネル付き画像に対して描画を行う場合のみ意味がある関数
extern int NS_SetUseNoBlendModeParam( int Flag )
{
	GSYS.DrawSetting.UseNoBlendModeParam = Flag ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// 描画可能なαチャンネル付き画像を作成するかどうかのフラグをセットする,SetDrawValidGraphCreateFlag 関数で描画可能画像を作成するように設定されていないと効果がない( TRUE:αチャンネル付き FALSE:αチャンネルなし )
extern int NS_SetDrawValidAlphaChannelGraphCreateFlag( int Flag )
{
	// フラグを保存する
	GSYS.CreateImage.DrawValidAlphaFlag = Flag ;
	
	// 終了
	return 0 ;
}

// 描画可能なαチャンネル付き画像を作成するかどうかのフラグを取得する
extern int NS_GetDrawValidAlphaChannelGraphCreateFlag( void )
{
	// フラグを返す
	return GSYS.CreateImage.DrawValidAlphaFlag ;
}

// 描画可能な浮動小数点型の画像を作成するかどうかのフラグをセットする,SetDrawValidGraphCreateFlag 関数で描画可能画像を作成するように設定されていないと効果がない、ＧＰＵが Float型の画像に対応していない場合は失敗する( TRUE:Float型 FALSE:Int型 )
extern int NS_SetDrawValidFloatTypeGraphCreateFlag( int Flag )
{
	// フラグを保存する
	GSYS.CreateImage.DrawValidFloatTypeFlag = Flag ;

	// 終了
	return 0 ;
}

// 描画可能な浮動小数点型の画像を作成するかどうかのフラグを取得する
extern int NS_GetDrawValidFloatTypeGraphCreateFlag( void )
{
	// フラグを返す
	return GSYS.CreateImage.DrawValidFloatTypeFlag ;
}

// 描画可能画像を作成する際に専用のＺバッファも作成するかどうかを設定する( デフォルトでは TRUE( 作成する ) )
extern int NS_SetDrawValidGraphCreateZBufferFlag( int Flag )
{
	// フラグを保存する
	GSYS.CreateImage.NotDrawValidCreateZBufferFlag = Flag ? FALSE : TRUE ;

	// 終了
	return 0 ;
}

// 描画可能画像を作成する際に専用のＺバッファも作成するかどうかを取得する
extern int NS_GetDrawValidGraphCreateZBufferFlag( void )
{
	// フラグを返す
	return GSYS.CreateImage.NotDrawValidCreateZBufferFlag ? FALSE : TRUE ;
}

// SetDrawScreen の引数として渡せる( 描画対象として使用できる )グラフィックハンドルに適用するＺバッファのビット深度を設定する( BitDepth:ビット深度( 指定可能な値は 16, 24, 32 の何れか( SetDrawValidGraphCreateFlag 関数で描画対象として使用できるグラフィックハンドルを作成するように設定されていないと効果ありません )
extern int NS_SetCreateDrawValidGraphZBufferBitDepth( int BitDepth )
{
	// ビット深度を保存する
	GSYS.CreateImage.DrawValidZBufferBitDepth = BitDepth ;

	// 終了
	return 0 ;
}

// SetDrawScreen の引数として渡せる( 描画対象として使用できる )グラフィックハンドルに適用するＺバッファのビット深度を取得する
extern int NS_GetCreateDrawValidGraphZBufferBitDepth( void )
{
	// ビット深度を返す
	return GSYS.CreateImage.DrawValidZBufferBitDepth == 0 ? 16 : GSYS.CreateImage.DrawValidZBufferBitDepth ;
}

// 作成する描画可能な画像のチャンネル数を設定する,SetDrawValidGraphCreateFlag 関数で描画可能画像を作成するように設定されていないと効果がない
extern int NS_SetCreateDrawValidGraphChannelNum( int ChannelNum )
{
	// チャンネル数を保存する
	GSYS.CreateImage.DrawValidChannelNum = ChannelNum ;

	// 終了
	return 0 ;
}

// 作成する描画可能な画像のチャンネル数を取得する
extern int NS_GetCreateDrawValidGraphChannelNum( void )
{
	// チャンネル数を返す
	return GSYS.CreateImage.DrawValidChannelNum ;
}

// SetDrawScreen の引数として渡せる( 描画対象として使用できる )グラフィックハンドルに適用するマルチサンプリング( アンチエイリアシング )設定を行う( Samples:マルチサンプル処理に使用するドット数( 多いほど重くなります )  Quality:マルチサンプル処理の品質 )
extern int NS_SetCreateDrawValidGraphMultiSample( int Samples, int Quality )
{
	if( Samples > 16 )
		Samples = 16 ;

	// パラメータを保存する
	GSYS.CreateImage.DrawValidMSSamples = Samples ;
	GSYS.CreateImage.DrawValidMSQuality = Quality ;

	// 終了
	return 0 ;
}

// 描画可能な画像のマルチサンプリング設定を行う
extern int NS_SetDrawValidMultiSample( int Samples, int Quality )
{
	return NS_SetCreateDrawValidGraphMultiSample( Samples, Quality ) ;
}

// 指定のマルチサンプル数で使用できる最大クオリティ値を取得する
extern int NS_GetMultiSampleQuality( int Samples )
{
	return Graphics_Hardware_GetMultiSampleQuality_PF( Samples ) ;
}

// 透過色機能を使用するかどうかを設定する
extern int NS_SetUseTransColor( int Flag )
{
	GSYS.CreateImage.NotUseTransColor = !Flag;

	// 終了
	return 0;
}

// 透過色機能を使用することを前提とした画像の読み込み処理を行うかどうかを設定する( TRUE にすると SetDrawMode( DX_DRAWMODE_BILINEAR ); をした状態で DrawGraphF 等の浮動小数点型座標を受け取る関数で小数点以下の値を指定した場合に発生する描画結果の不自然を緩和する効果がある ( デフォルトは FALSE ) )
extern int NS_SetUseTransColorGraphCreateFlag( int Flag )
{
	GSYS.CreateImage.UseTransColorFlag = Flag;

	return 0;
}

// SetUseAlphaChannelGraphCreateFlag の旧名称
extern int NS_SetUseGraphAlphaChannel( int Flag )
{
	return NS_SetUseAlphaChannelGraphCreateFlag( Flag ) ;
}

// GetUseAlphaChannelGraphCreateFlag の旧名称
extern int NS_GetUseGraphAlphaChannel( void )
{
	return NS_GetUseAlphaChannelGraphCreateFlag() ;
}

// αチャンネル付きグラフィックを作成するかどうかのフラグをセットする( TRUE:αチャンネル付き   FALSE:αチャンネル無し )
extern int NS_SetUseAlphaChannelGraphCreateFlag( int Flag )
{
	// フラグを保存する
	GSYS.CreateImage.AlphaChannelFlag = Flag ;
	
	// 終了
	return 0 ;
}

// αチャンネル付きグラフィックを作成するかどうかのフラグを取得する( TRUE:αチャンネル付き   FALSE:αチャンネル無し )
extern int NS_GetUseAlphaChannelGraphCreateFlag( void )
{
	return GSYS.CreateImage.AlphaChannelFlag ;
}

// 非管理テクスチャを使用するか、のフラグをセット( TRUE:使用する  FALSE:使用しない )
extern int NS_SetUseNotManageTextureFlag( int Flag )
{
	GSYS.CreateImage.NotUseManagedTextureFlag = Flag ;

	// 終了
	return 0 ;
}

// 非管理テクスチャを使用するか、のフラグを取得する( TRUE:使用する  FALSE:使用しない )
extern int NS_GetUseNotManageTextureFlag( void )
{
	return GSYS.CreateImage.NotUseManagedTextureFlag ;
}

// グラフィックに設定する透過色をセットする
extern int NS_SetTransColor( int Red, int Green, int Blue )
{
	if( NS_GetColorBitDepth() == 8 ) return 0 ;
	GSYS.CreateImage.TransColor = ( DWORD)( ( ( ( BYTE )Red ) << 16 ) | ( ( ( BYTE )Green ) << 8 ) | ( ( BYTE )Blue ) ) ;
	BASEIM.TransColor = GSYS.CreateImage.TransColor ;

	// 終了
	return 0 ;
}

// 透過色を得る
extern int NS_GetTransColor( int *Red, int *Green, int *Blue )
{
	DWORD TransColor ;

	if( NS_GetColorBitDepth() == 8 ) return -1 ;

	TransColor = GSYS.CreateImage.TransColor ;

	*Red   = ( int )( ( TransColor >> 16 ) & 0xff ) ;
	*Green = ( int )( ( TransColor >> 8  ) & 0xff ) ;
	*Blue  = ( int )(   TransColor         & 0xff ) ;
 
	// 終了
	return 0 ;
}

// 必要ならグラフィックの分割を行うか、フラグのセット
extern	int		NS_SetUseDivGraphFlag( int Flag ) 
{
	GSYS.CreateImage.NotUseDivFlag = !Flag ;

	// 終了
	return 0 ;
}

// LoadGraph などの際にファイル名の末尾に _a が付いたアルファチャンネル用の画像ファイルを追加で読み込む処理を行うかどうかを設定する( TRUE:行う( デフォルト )  FALSE:行わない )
extern	int		NS_SetUseAlphaImageLoadFlag( int Flag )
{
	GSYS.CreateImage.NotUseAlphaImageLoadFlag = Flag ? FALSE : TRUE ;

	// 終了
	return 0 ;
}

// 使用するテクスチャーの最大サイズをセット(0でデフォルト)
extern	int		NS_SetUseMaxTextureSize( int Size )
{
	int i ;

	if( Size == 0 )
	{
		GSYS.CreateImage.UserMaxTextureSize = 0 ;
		return 0 ;
	}
	
	// 指定のサイズが収まる最小の２のｎ乗のサイズを求める
	for( i = MIN_TEXTURE_SIZE ; i < Size ; i <<= 1 ){}

	// 最大サイズより大きかったら補正
	if( Size > GSYS.HardInfo.MaxTextureSize ) Size = GSYS.HardInfo.MaxTextureSize ; 
	
	// 値をセット
	GSYS.CreateImage.UserMaxTextureSize = i ;
	
	// 終了
	return 0 ;
}

// 画像を作成する際に使用した画像データのバックアップをしてデバイスロスト時に使用するかどうかのフラグをセットする( TRUE:バックアップをする( デフォルト )  FALSE:バックアップをしない )
extern int NS_SetUseGraphBaseDataBackup( int Flag )
{
	GSYS.CreateImage.NotGraphBaseDataBackupFlag = Flag ? FALSE : TRUE ;

	// 終了
	return 0 ;
}

// 画像を作成する際に使用した画像データのバックアップをしてデバイスロスト時に使用するかどうかのフラグを取得する
extern int NS_GetUseGraphBaseDataBackup( void )
{
	return GSYS.CreateImage.NotGraphBaseDataBackupFlag ? FALSE : TRUE ;
}

// システムメモリ上にグラフィックを作成するかどうかのフラグをセットする( TRUE:システムメモリ上に作成  FALSE:ＶＲＡＭ上に作成 )
extern	int NS_SetUseSystemMemGraphCreateFlag( int /* Flag */ )
{
	// フラグを保存する
//	GSYS.SystemMemImageCreateFlag = Flag ;

	// 終了
	return 0 ;
}

// システムメモリ上にグラフィックを作成するかどうかのフラグを取得する( TRUE:システムメモリ上に作成  FALSE:ＶＲＡＭ上に作成 )
extern int NS_GetUseSystemMemGraphCreateFlag( void )
{
//	return GSYS.SystemMemImageCreateFlag ;
	return FALSE ;
}


























// 画像情報関係関数

// 指定の画像のＡＲＧＢ８のフルカラーイメージを取得する
extern const unsigned int *NS_GetFullColorImage( int GrHandle )
{
	IMAGEDATA *Image ;

//	if( GSYS.NotDrawFlag ) return 0 ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return NULL ;

	// フルカラーイメージを格納するメモリ領域をまだ確保していない場合は確保する
	if( Image->FullColorImage == NULL )
	{
		Image->FullColorImage = ( DWORD * )DXALLOC( ( size_t )( Image->Width * 4 * Image->Height ) ) ;
		if( Image->FullColorImage == NULL )
		{
			DXST_ERRORLOG_ADDUTF16LE( "\xd5\x30\xeb\x30\xab\x30\xe9\x30\xfc\x30\x6e\x30\x3b\x75\xcf\x50\xa4\x30\xe1\x30\xfc\x30\xb8\x30\xc7\x30\xfc\x30\xbf\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x92\x30\xba\x78\xdd\x4f\xfa\x51\x65\x67\x7e\x30\x5b\x30\x93\x30\x67\x30\x57\x30\x5f\x30\x02\x30\x0a\x00\x00"/*@ L"フルカラーの画像イメージデータを格納するメモリ領域を確保出来ませんでした。\n" @*/ ) ;
			return NULL;
		}
	}

#ifndef DX_NON_MOVIE

	// ムービーの場合は処理を分岐
	if( Image->MovieHandle >= 0 )
	{
		MOVIEGRAPH * Movie ;
		Movie = GetMovieData( Image->MovieHandle ) ;
		if( Movie == NULL ) return NULL ;
		static BASEIMAGE Src, Dest ;

		// サーフェスモードがオーバーレイの場合は無理
		if( Movie->SurfaceMode == DX_MOVIESURFACE_OVERLAY ) return NULL ;

		// それ以外の場合はムービーから直接データを取得する

		// ムービーのフレームを更新
		if( NS_GetMovieStateToGraph( GrHandle ) == FALSE )
		{
			int Time ;

			Time = NS_TellMovieToGraph( GrHandle ) ;
			NS_PlayMovieToGraph( GrHandle ) ;
			UpdateMovie( Image->MovieHandle, TRUE ) ;
			NS_PauseMovieToGraph( GrHandle ) ;
			NS_SeekMovieToGraph( GrHandle, Time ) ;
		}
		else
		{
//			UpdateMovieToGraph( GrHandle ) ;
		}

		// 転送先基本イメージデータの情報を準備する
		CreateARGB8ColorData( &Dest.ColorData ) ;
		Dest.Width     = Image->Width ;
		Dest.Height    = Image->Height ;
		Dest.GraphData = Image->FullColorImage ;
		Dest.Pitch     = Image->Width * 4 ;

		// 転送
		BltBaseImage( 0, 0, Image->Width, Image->Height, 0, 0, &Movie->NowImage, &Dest ) ;
	}
	else
#endif
	{
	}

	// イメージのポインタを返す
	return (unsigned int *)Image->FullColorImage ;
}

// グラフィックメモリ領域のロック
extern int NS_GraphLock( int GrHandle, int *PitchBuf, void **DataPointBuf, COLORDATA **ColorDataPP, int WriteOnly )
{
	// 画像かどうかで処理を分岐
	if( GrHandle == DX_SCREEN_BACK || GrHandle == DX_SCREEN_FRONT )
	{
		// ハードウエアを使用しているかどうかで処理を分岐
		if( GSYS.Setting.ValidHardware )
		{
			// ハードウエアでのロックは対応していない
			return -1 ;
		}
		else
		{
			// ソフトウエアの場合は画面イメージの情報を返す
			if( ColorDataPP  ) *ColorDataPP  =        GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP ;
			if( PitchBuf     ) *PitchBuf     = ( int )GSYS.SoftRender.MainBufferMemImg.Base->Pitch ;
			if( DataPointBuf ) *DataPointBuf =        GSYS.SoftRender.MainBufferMemImg.UseImage ;
		}
	}
	else
	{
		IMAGEDATA *Image ;

		// エラー判定
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;

		// 既にロックしてあったらエラー
		if( Image->LockFlag == TRUE ) return -1 ;

		// ロックフラグを立てる
		Image->LockFlag = TRUE ;

		// テクスチャかどうかで処理を分岐
		if( Image->Orig->FormatDesc.TextureFlag )
		{
			COLORDATA *ColorData ;

			// テクスチャの場合
			if( Graphics_Hardware_GraphLock_PF( Image, &ColorData, WriteOnly ) < 0 )
			{
				return -1 ;
			}

			// 情報の格納
			if( PitchBuf     ) *PitchBuf     = ( int )Image->LockImagePitch ;
			if( DataPointBuf ) *DataPointBuf = ( void * )Image->LockImage ;
			if( ColorDataPP  ) *ColorDataPP  = ColorData ;
		}
		else
		{
			// テクスチャではない場合
			if( PitchBuf     ) *PitchBuf     = ( int )Image->Soft.MemImg.Base->Pitch ;
			if( DataPointBuf ) *DataPointBuf =        Image->Soft.MemImg.UseImage ;
			if( ColorDataPP  ) *ColorDataPP  =        Image->Soft.MemImg.Base->ColorDataP ;
		}
	}

	// 終了
	return 0 ;
}

// グラフィックメモリ領域のロック解除
extern	int NS_GraphUnLock( int GrHandle )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	// ロックしてなかったらエラー
	if( Image->LockFlag == FALSE ) return -1 ;

	// ロックフラグを倒す
	Image->LockFlag = FALSE ;

	// テクスチャかどうかで処理を分岐
	if( Image->Orig->FormatDesc.TextureFlag )
	{
		// テクスチャの場合
		if( Graphics_Hardware_GraphUnlock_PF( Image ) < 0 )
		{
			return -1 ;
		}
	}
	else
	{
		// テクスチャではない場合
	}

	// 終了
	return 0 ;
}

// グラフィックにＺバッファを使用するかどうかを設定する
extern	int NS_SetUseGraphZBuffer( int GrHandle, int UseFlag, int BitDepth )
{
	IMAGEDATA *Image ;
	int DepthIndex ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	// テクスチャではない場合は何もしない
	if( Image->Orig->FormatDesc.TextureFlag == FALSE )
		return -1 ;

	// 描画可能ではない場合も何もしない
	if( Image->Orig->FormatDesc.DrawValidFlag == FALSE )
		return -1 ;

	// ビット深度のセット
	if( Image->Orig->ZBufferFlag == TRUE && BitDepth < 0 )
	{
		DepthIndex = Image->Orig->ZBufferBitDepthIndex ;
	}
	else
	{
		switch( BitDepth )
		{
		default :
		case 16 : DepthIndex = ZBUFFER_FORMAT_16BIT ; break ;
		case 24 : DepthIndex = ZBUFFER_FORMAT_24BIT ; break ;
		case 32 : DepthIndex = ZBUFFER_FORMAT_32BIT ; break ;
		}
	}

	// Ｚバッファを使うフラグとビット深度が同じだったら何もしない
	if( Image->Orig->ZBufferFlag == UseFlag && Image->Orig->ZBufferBitDepthIndex == DepthIndex )
		return 0 ;

	// フラグを保存
	Image->Orig->ZBufferFlag = UseFlag ;

	// ビット深度インデックスを保存
	Image->Orig->ZBufferBitDepthIndex = DepthIndex ;

	// テクスチャの作り直し
	Graphics_Hardware_CreateOrigTexture_PF( Image->Orig ) ;

	// 終了
	return 0 ;
}

// グラフィックのＺバッファの状態を別のグラフィックのＺバッファにコピーする( DestGrHandle も SrcGrHandle もＺバッファを持っている描画可能画像で、且つアンチエイリアス画像ではないことが条件 )
extern int NS_CopyGraphZBufferImage( int DestGrHandle, int SrcGrHandle )
{
	IMAGEDATA *SrcImage ;
	IMAGEDATA *DestImage ;

	// エラー判定
	if( GRAPHCHK( SrcGrHandle, SrcImage ) )
		return -1 ;

	if( GRAPHCHK( DestGrHandle, DestImage ) )
		return -1 ;

	// テクスチャではない場合は何もしない
	if( SrcImage->Orig->FormatDesc.TextureFlag == FALSE ||
		DestImage->Orig->FormatDesc.TextureFlag == FALSE )
		return -1 ;

	// 環境依存処理
	if( Graphics_Hardware_CopyGraphZBufferImage_PF( DestImage, SrcImage ) < 0 )
	{
		return -1 ;
	}

	// 正常終了
	return 0 ;
}

// グラフィックスデバイスのデバイスロスト発生時に指定のグラフィックハンドルを削除するかどうかを設定する( TRUE:デバイスロスト時に削除する  FALSE:デバイスロストが発生しても削除しない )
extern int NS_SetDeviceLostDeleteGraphFlag(	int GrHandle, int DeleteFlag )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK_ASYNC( GrHandle, Image ) )
		return -1 ;

	Image->DeviceLostDeleteFlag = DeleteFlag ? TRUE : FALSE ;

	return 0 ;
}

// グラフィックのサイズを得る
extern	int NS_GetGraphSize( int GrHandle, int *SizeXBuf, int *SizeYBuf )
{
	IMAGEDATA *Image ;
	SHADOWMAPDATA *ShadowMap ;

	switch( GrHandle )
	{
	case DX_SCREEN_BACK	 :
	case DX_SCREEN_FRONT :
		if( SizeXBuf ) *SizeXBuf = GSYS.Screen.MainScreenSizeX ;
		if( SizeYBuf ) *SizeYBuf = GSYS.Screen.MainScreenSizeY ;
		break ;

	default :
		// エラー判定
		if( !GRAPHCHK_ASYNC( GrHandle, Image ) )
		{
			if( SizeXBuf ) *SizeXBuf = Image->Width ;
			if( SizeYBuf ) *SizeYBuf = Image->Height ;
		}
		else
		if( !SHADOWMAPCHK( GrHandle, ShadowMap ) )
		{
			if( SizeXBuf ) *SizeXBuf = ShadowMap->BaseSizeX ;
			if( SizeYBuf ) *SizeYBuf = ShadowMap->BaseSizeY ;
		}
		else
		{
			return -1 ;
		}
		break ;
	}

	return 0 ;
}

// グラフィックに使用されているテクスチャーのサイズを得る
extern int NS_GetGraphTextureSize( int GrHandle, int *SizeXBuf, int *SizeYBuf )
{
	IMAGEDATA *Image = NULL ;
	SHADOWMAPDATA *ShadowMap = NULL ;

	// エラー判定
	if( GRAPHCHK(     GrHandle, Image    ) &&
		SHADOWMAPCHK( GrHandle, ShadowMap ) )
		return -1 ;

	// シャドウマップかどうかで処理を分岐
	if( ShadowMap != NULL )
	{
		if( SizeXBuf ) *SizeXBuf = ShadowMap->BaseSizeX ;
		if( SizeYBuf ) *SizeYBuf = ShadowMap->BaseSizeY ;
	}
	else
	{
		// テクスチャかどうかで処理を分岐
		if( Image->Orig->FormatDesc.TextureFlag )
		{
			if( SizeXBuf ) *SizeXBuf = Image->Orig->Hard.Tex[ 0 ].TexWidth ;
			if( SizeYBuf ) *SizeYBuf = Image->Orig->Hard.Tex[ 0 ].TexHeight ;
		}
		else
		{
			if( SizeXBuf ) *SizeXBuf = Image->Width ;
			if( SizeYBuf ) *SizeYBuf = Image->Height ;
		}
	}

	// 終了
	return 0 ;
}

// グラフィックが持つミップマップレベルの数を取得する
extern int NS_GetGraphMipmapCount( int GrHandle )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	// ハードウェアを使用していない場合は１
	if( Image->Orig->FormatDesc.TextureFlag == FALSE )
		return 1 ;

	// ミップマップの数を返す
	return Image->Orig->Hard.MipMapCount ;
}

// グラフィックハンドルが画像ファイルから読み込まれていた場合、その画像のファイルパスを取得する
extern int NS_GetGraphFilePath( int GrHandle, TCHAR *FilePathBuffer )
{
#ifdef UNICODE
	return GetGraphFilePath_WCHAR_T( GrHandle, FilePathBuffer ) ;
#else
	IMAGEDATA *Image ;
	TCHAR TempBuffer[ 2048 ] ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
	{
		return -1 ;
	}

	// 画像ファイルパスが無い場合はエラー
	if( Image->ReadBase->FileName == NULL )
	{
		return -1 ;
	}

	// 画像ファイルパスをコピー
	ConvString( ( const char * )Image->ReadBase->FileName, WCHAR_T_CHARCODEFORMAT, TempBuffer, _TCHARCODEFORMAT ) ;
	if( FilePathBuffer != NULL )
	{
		_TSTRCPY( FilePathBuffer, TempBuffer ) ;
	}

	// 正常終了の場合は文字列サイズを返す
	return _TSTRLEN( TempBuffer ) ;
#endif
}

// グラフィックハンドルが画像ファイルから読み込まれていた場合、その画像のファイルパスを取得する
extern int GetGraphFilePath_WCHAR_T( int GrHandle, wchar_t *FilePathBuffer )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	// 画像ファイルパスが無い場合はエラー
	if( Image->ReadBase->FileName == NULL )
		return -1 ;

	// 画像ファイルパスをコピー
	if( FilePathBuffer != NULL )
	{
		_WCSCPY( FilePathBuffer, Image->ReadBase->FileName ) ;
	}

	// 正常終了の場合は文字列サイズを返す
	return _WCSLEN( Image->ReadBase->FileName ) ;
}

// カラーデータを得る
extern const COLORDATA * NS_GetTexColorData( int AlphaCh, int AlphaTest, int ColorBitDepth, int DrawValid )
{
	IMAGEFORMATDESC Format ;
	
	Format.DrawValidFlag   = ( unsigned char )DrawValid ;
	Format.AlphaChFlag     = ( unsigned char )AlphaCh ;
	Format.AlphaTestFlag   = ( unsigned char )AlphaTest ;
	Format.ColorBitDepth   = ( BYTE )( ColorBitDepth == TEX_BITDEPTH_16 ? 16 : 32 ) ;
	Format.ChannelNum      = 0 ;
	Format.ChannelBitDepth = 0 ;
	Format.FloatTypeFlag   = FALSE ;
	Format.BaseFormat      = DX_BASEIMAGE_FORMAT_NORMAL ;
	return NS_GetTexColorData( &Format ) ;
}

// フォーマットに基づいたカラーデータを得る
extern const COLORDATA * NS_GetTexColorData( const IMAGEFORMATDESC *Format )
{
	const COLORDATA * Result ;

	Result = NS_GetTexColorData( NS_GetTexFormatIndex( Format ) ) ;

	return Result ;
}

// 指定のフォーマットインデックスのカラーデータを得る
extern const COLORDATA * NS_GetTexColorData( int FormatIndex )
{
	static int Initialize = FALSE ;
	static COLORDATA ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_NUM ] ;

	// 値が範囲外だったらエラー
	if( FormatIndex >= DX_GRAPHICSIMAGE_FORMAT_3D_NUM ) return NULL ;

	// 最初に来たときに初期化
	if( Initialize == FALSE )
	{
		Initialize = TRUE ;

		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_RGB16                 ],  16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_RGB32                 ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB16           ],  16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB32           ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_RGB16       ],  16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_RGB32       ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT1                  ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT2                  ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT3                  ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT4                  ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT5                  ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_RGB16       ],  16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_RGB32       ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ALPHA_RGB32 ],  32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_I16    ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 4, 16, FALSE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F16    ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 4, 16, TRUE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F16    ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 4, 32, TRUE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I8      ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 1,  8, FALSE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I16     ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 1, 16, FALSE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F16     ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 1, 16, TRUE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F32     ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 1, 32, TRUE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I8      ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 2,  8, FALSE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I16     ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 2, 16, FALSE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F16     ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 2, 16, TRUE ) ;
		NS_CreateColorData( &ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F32     ],   0, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 2, 32, TRUE ) ;

		ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT1 ].Format = DX_BASEIMAGE_FORMAT_DXT1 ; ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT1 ].ColorBitDepth = 4 ;
		ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT2 ].Format = DX_BASEIMAGE_FORMAT_DXT2 ; ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT2 ].ColorBitDepth = 8 ;
		ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT3 ].Format = DX_BASEIMAGE_FORMAT_DXT3 ; ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT3 ].ColorBitDepth = 8 ;
		ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT4 ].Format = DX_BASEIMAGE_FORMAT_DXT4 ; ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT4 ].ColorBitDepth = 8 ;
		ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT5 ].Format = DX_BASEIMAGE_FORMAT_DXT5 ; ColorData[ DX_GRAPHICSIMAGE_FORMAT_3D_DXT5 ].ColorBitDepth = 8 ;
	}

	// カラーフォーマットを返す
	return &ColorData[ FormatIndex ] ;
}

// グラフィックに使用される最大テクスチャサイズを取得する
extern int NS_GetMaxGraphTextureSize( int *SizeX, int *SizeY )
{
	if( SizeX != NULL ) *SizeX = GSYS.HardInfo.MaxTextureWidth ;
	if( SizeY != NULL ) *SizeY = GSYS.HardInfo.MaxTextureHeight ;

	// 終了
	return 0 ;
}

// グラフィック復元関数の有無を取得	
extern	int NS_GetValidRestoreShredPoint( void )
{
	return GSYS.Setting.GraphRestoreShred != NULL ;
}

// これから新たにグラフィックを作成する場合に使用するカラー情報を取得する
extern	int NS_GetCreateGraphColorData( COLORDATA * /*ColorData*/, IMAGEFORMATDESC * /*Format*/ )
{
	// 終了
	return 0 ;
}























// 画像パレット操作関係関数

// メモリ上に読み込んだ画像のパレットを取得する(フルカラー画像の場合は無効)
extern	int		NS_GetGraphPalette( int GrHandle, int ColorIndex, int *Red, int *Green, int *Blue )
{
	MEMIMG *MemImg ;
	unsigned int Color;

	// エラーチェック
	if( ColorIndex < 0 || ColorIndex > 255 ) return -1 ;
	PALETTEIMAGECHK( GrHandle, MemImg );
	
	// パレットを取得する
	switch( MemImg->Base->ColorType )
	{
	case 0: Color = ((WORD  *)MemImg->Base->Palette)[ ColorIndex ]; break;
	case 1: Color = ((DWORD *)MemImg->Base->Palette)[ ColorIndex ]; break;
	default : return -1 ;
	}
	NS_GetColor5( MemImg->Base->ColorDataP, Color, Red, Green, Blue ) ;
	
	// 終了
	return 0 ;
}

// メモリ上に読み込んだ画像の SetGraphPalette で変更する前のパレットを取得する
extern  int		NS_GetGraphOriginalPalette( int GrHandle, int ColorIndex, int *Red, int *Green, int *Blue )
{
	MEMIMG *MemImg ;
	unsigned int Color ;
	
	// エラーチェック
	if( ColorIndex < 0 || ColorIndex > 255 ) return -1 ;
	PALETTEIMAGECHK( GrHandle, MemImg );

	// パレットを取得する
	switch( MemImg->Base->ColorType )
	{
	case 0: Color = ((WORD  *)MemImg->Base->OriginalPalette)[ColorIndex]; break;
	case 1: Color = ((DWORD *)MemImg->Base->OriginalPalette)[ColorIndex]; break;
	default : return -1 ;
	}
	NS_GetColor5( MemImg->Base->ColorDataP, Color, Red, Green, Blue ) ;
	
	// 終了
	return 0 ;
}

// メモリ上に読み込んだ画像のパレットを変更する(フルカラー画像の場合は無効)
extern	int		NS_SetGraphPalette( int GrHandle, int ColorIndex, unsigned int Color )
{
	MEMIMG *MemImg ;
	int Red, Green, Blue ;
	COLORDATA *ColorData ;

	// エラーチェック
	if( ColorIndex < 0 || ColorIndex > 255 ) return -1 ;
	PALETTEIMAGECHK( GrHandle, MemImg );

	// 色コードをセットする
	NS_GetColor2( Color, &Red, &Green, &Blue ) ;
	ColorData = MemImg->Base->ColorDataP ;
	Color = ColorData->NoneMask +
				( ( ( BYTE )Red   >> ( 8 - ColorData->RedWidth   ) ) << ColorData->RedLoc   ) +
				( ( ( BYTE )Green >> ( 8 - ColorData->GreenWidth ) ) << ColorData->GreenLoc ) +
				( ( ( BYTE )Blue  >> ( 8 - ColorData->BlueWidth  ) ) << ColorData->BlueLoc  ) ;
	switch( MemImg->Base->ColorType )
	{
	case 0: ((WORD  *)MemImg->Base->Palette)[ColorIndex] = (WORD)Color;  break;
	case 1: ((DWORD *)MemImg->Base->Palette)[ColorIndex] = (DWORD)Color; break;
	}
	
	// 終了
	return 0 ;
}

// SetGraphPalette で変更したパレットを全て元に戻す
extern int NS_ResetGraphPalette( int GrHandle )
{
	MEMIMG *MemImg ;
	int Size;
	
	// エラーチェック
	PALETTEIMAGECHK( GrHandle, MemImg );

	// 元のパレットをコピーする
	switch( MemImg->Base->ColorType )
	{
	case 0 : Size = 1 ; break ;
	case 1 : Size = 2 ; break ;
	default : return -1 ;
	}
	_MEMCPY( MemImg->Base->Palette, MemImg->Base->OriginalPalette, ( size_t )( MemImg->Base->ColorNum << Size ) ) ;
	
	// 終了
	return 0 ;
}

























// 図形描画関数

// 太さ指定付き線の描画
static int DrawLine_Thickness( int x1, int y1, int x2, int y2, unsigned int Color, int Thickness )
{
	VERTEX vert[4];
	VECTOR v, p1, p2;
	int r, g, b;
	float f;

	if( Thickness <= 0 || Thickness == 1 )
	{
		return NS_DrawLine( x1, y1, x2, y2, Color, 1 ) ;
	}
	else
	{
		if( x1 == x2 && Thickness % 2 == 1 )
		{
			int x ;

			x = x1 - Thickness / 2 ;
			return NS_DrawBox( x, y1, x + Thickness, y2, Color, TRUE ) ;
		}
		else
		if( y1 == y2 && Thickness % 2 == 1 )
		{
			int y ;

			y = y1 - Thickness / 2 ;
			return NS_DrawBox( x1, y, x2, y + Thickness, Color, TRUE ) ;
		}
	}

	if( GSYS.Setting.ValidHardware )
	{
		p1.x = (float)x1;
		p1.y = (float)y1;
		p2.x = (float)x2;
		p2.y = (float)y2;

		v.x = (    p2.y - p1.y   );
		v.y = ( -( p2.x - p1.x ) );
		f = ( 1.0f / _SQRT( v.x * v.x + v.y * v.y ) ) * ( (float)Thickness / 2.0f );
		v.x *= f;
		v.y *= f;

		NS_GetColor2( Color, &r, &g, &b );
		vert[0].u = 0.0f;
		vert[0].v = 0.0f;
		vert[0].r = (unsigned char)r;
		vert[0].g = (unsigned char)g;
		vert[0].b = (unsigned char)b;
		vert[0].a = (unsigned char)255;
		vert[1] =
		vert[2] =
		vert[3] = vert[0];

		vert[0].x = p1.x + v.x;
		vert[0].y = p1.y + v.y;
		vert[1].x = p2.x + v.x;
		vert[1].y = p2.y + v.y;
		vert[2].x = p1.x - v.x;
		vert[2].y = p1.y - v.y;
		vert[3].x = p2.x - v.x;
		vert[3].y = p2.y - v.y;

		NS_DrawPolygonBase( vert, 4, DX_PRIMTYPE_TRIANGLESTRIP, DX_NONE_GRAPH, FALSE );
	}
	else
	{
		v.x = ( float )(    y2 - y1   );
		v.y = ( float )( -( x2 - x1 ) );
		f = ( 1.0f / _SQRT( v.x * v.x + v.y * v.y ) ) * ( (float)Thickness / 2.0f ) ;
		v.x *= f ;
		v.y *= f ;

		Graphics_Software_DrawQuadrangle( 
			_FTOL( x1 + v.x ),
			_FTOL( y1 + v.y ),
			_FTOL( x2 + v.x ),
			_FTOL( y2 + v.y ),
			_FTOL( x2 - v.x ),
			_FTOL( y2 - v.y ),
			_FTOL( x1 - v.x ),
			_FTOL( y1 - v.y ), Color, TRUE ) ;
	}

	// 終了
	return 0;
}

// 線を描画
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, x1, y1, x2, y2 ) ;\
	if( x1 > x2 ){ DrawRect.left = x2 ; DrawRect.right  = x1; }\
	if( y1 > y2 ){ DrawRect.top  = y2 ; DrawRect.bottom = y1; }\
	DrawRect.left   -= Thickness ;\
	DrawRect.top    -= Thickness ;\
	DrawRect.right  += 1 + Thickness ;\
	DrawRect.bottom += 1 + Thickness ;\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawLine( int x1, int y1, int x2, int y2, unsigned int Color, int Thickness )
{
	int Ret = -1 ;
	int Flag ;

	// 太さが１以上の場合は太さ指定付き線描画を行う
	if( Thickness > 1 )
		return DrawLine_Thickness( x1, y1, x2, y2, Color, Thickness ) ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	DRAW_DEF(
		Graphics_Hardware_DrawLine_PF( x1, y1, x2, y2, Color ),
		Graphics_Software_DrawLine(    x1, y1, x2, y2, Color ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// 四角形の描画
extern	int NS_DrawBox( int x1, int y1, int x2, int y2, unsigned int Color, int FillFlag )
{
	int Ret ;
	
	if( FillFlag )	Ret = NS_DrawFillBox( x1, y1, x2, y2, Color ) ;
	else			Ret = NS_DrawLineBox( x1, y1, x2, y2, Color ) ;

	// 終了
	return Ret ;
}

// 中身のある四角を描画
#define SETDRAWRECTCODE\
	if( x1 < x2 ){ DrawRect.left = x1 ; DrawRect.right = x2; }\
	if( y1 < y2 ){ DrawRect.top = y1 ; DrawRect.bottom = y2; }\
\
	SETRECT( DrawRect, x1, y1, x2, y2 ) ;\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawFillBox( int x1, int y1, int x2, int y2, unsigned int Color )
{
	int Ret = -1 ;
	int Flag ;
	
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;
	
	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawFillBox_PF( x1, y1, x2, y2, Color ),
		Graphics_Software_DrawFillBox(    x1, y1, x2, y2, Color ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

// 四角形の描画 
extern	int NS_DrawLineBox( int x1, int y1, int x2, int y2, unsigned int Color )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;
	
	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawLineBox_PF( x1, y1, x2, y2, Color ),
		Graphics_Software_DrawLineBox(    x1, y1, x2, y2, Color ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// 円を描く
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, x - r - 1, y - r - 1, x + r + 1, y + r + 1 )\
	DRAWRECT_DRAWAREA_CLIP
extern	int NS_DrawCircle( int x, int y, int r, unsigned int Color, int FillFlag, int Thickness )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;
	
	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	if( r < 0 ) return 0 ;

	// 描画
	if( Thickness == 1 )
	{
		DRAW_DEF(
			Graphics_Hardware_DrawCircle_PF( x, y, r, Color, FillFlag ),
			Graphics_Software_DrawCircle(    x, y, r, Color, FillFlag ),
			SETDRAWRECTCODE,
			Ret,
			Flag
		)
	}
	else
	{
		DRAW_DEF(
			Graphics_Hardware_DrawCircle_Thickness_PF( x, y, r, Color, Thickness ),
			Graphics_Software_DrawCircle_Thickness(    x, y, r, Color, Thickness ),
			SETDRAWRECTCODE,
			Ret,
			Flag
		)
	}

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// 楕円を描く
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, x - rx - 1, y - ry - 1, x + rx + 1, y + ry + 1 )\
	DRAWRECT_DRAWAREA_CLIP
extern	int NS_DrawOval( int x, int y, int rx, int ry, unsigned int Color, int FillFlag, int Thickness )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	if( rx < 0 || ry < 0 ) return 0 ;

	// 描画
	if( Thickness == 1 )
	{
		DRAW_DEF(
			Graphics_Hardware_DrawOval_PF( x, y, rx, ry, Color, FillFlag ),
			Graphics_Software_DrawOval(    x, y, rx, ry, Color, FillFlag ),
			SETDRAWRECTCODE,
			Ret,
			Flag
		)
	}
	else
	{
		DRAW_DEF(
			Graphics_Hardware_DrawOval_Thickness_PF( x, y, rx, ry, Color, Thickness ),
			Graphics_Software_DrawOval_Thickness(    x, y, rx, ry, Color, Thickness ),
			SETDRAWRECTCODE,
			Ret,
			Flag
		)
	}

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// 三角形の描画
#define SETDRAWRECTCODE\
	int x[ 3 ], y[ 3 ], MaxX, MaxY, MinX, MinY ;\
	int i, ix, iy ;\
\
	x[ 0 ] = x1 ; x[ 1 ] = x2 ; x[ 2 ] = x3 ;\
	y[ 0 ] = y1 ; y[ 1 ] = y2 ; y[ 2 ] = y3 ;\
\
	MaxX = 0 ; MaxY = 0 ;\
	MinX = 0xffffff ; MinY = 0xffffff ;\
\
	for( i = 0 ; i < 3 ; i ++ )\
	{\
		ix = x[ i ] ; iy = y[ i ] ;\
		if( ix > MaxX ) MaxX = ix ; if( iy > MaxY ) MaxY = iy ;\
		if( ix < MinX ) MinX = ix ; if( iy < MinY ) MinY = iy ;\
	}\
\
	SETRECT( DrawRect, MinX, MinY, MaxX, MaxY ) ;\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawTriangle( int x1, int y1, int x2, int y2, int x3, int y3, unsigned int Color, int FillFlag )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;
	
	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawTriangle_PF( x1, y1, x2, y2, x3, y3, Color, FillFlag ),
		Graphics_Software_DrawTriangle(    x1, y1, x2, y2, x3, y3, Color, FillFlag ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// 四角形の描画
#define SETDRAWRECTCODE\
	int x[ 4 ], y[ 4 ], MaxX, MaxY, MinX, MinY ;\
	int i, ix, iy ;\
\
	x[ 0 ] = x1 ; x[ 1 ] = x2 ; x[ 2 ] = x3 ; x[ 3 ] = x4 ;\
	y[ 0 ] = y1 ; y[ 1 ] = y2 ; y[ 2 ] = y3 ; y[ 3 ] = y4 ;\
\
	MaxX = 0 ; MaxY = 0 ;\
	MinX = 0xffffff ; MinY = 0xffffff ;\
\
	for( i = 0 ; i < 4 ; i ++ )\
	{\
		ix = x[ i ] ; iy = y[ i ] ;\
		if( ix > MaxX ) MaxX = ix ; if( iy > MaxY ) MaxY = iy ;\
		if( ix < MinX ) MinX = ix ; if( iy < MinY ) MinY = iy ;\
	}\
\
	SETRECT( DrawRect, MinX, MinY, MaxX, MaxY ) ;\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawQuadrangle( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, unsigned int Color, int FillFlag )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;
	
	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawQuadrangle_PF( x1, y1, x2, y2, x3, y3, x4, y4, Color, FillFlag ),
		Graphics_Software_DrawQuadrangle(    x1, y1, x2, y2, x3, y3, x4, y4, Color, FillFlag ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// 角の丸い四角形を描画する
extern int NS_DrawRoundRect( int x1, int y1, int x2, int y2, int rx, int ry, unsigned int Color, int FillFlag )
{
	RECT  BackupDrawRect ;
	RECTF BackupDrawRectF ;
	int   DrawSizeX ;
	int   DrawSizeY ;
	int   Temp ;

	// 描画矩形を保存
	BackupDrawRect  = GSYS.DrawSetting.DrawArea ;
	BackupDrawRectF = GSYS.DrawSetting.DrawAreaF ;

	if( rx <= 0 )
	{
		rx = 1 ;
	}

	if( ry <= 0 )
	{
		ry = 1 ;
	}

	if( x2 < x1 )
	{
		Temp = x1 ;
		x1 = x2 ;
		x2 = Temp ;
	}

	if( y2 < y1 )
	{
		Temp = y1 ;
		y1 = y2 ;
		y2 = Temp ;
	}

	DrawSizeX = x2 - x1 ;
	DrawSizeY = y2 - y1 ;

	if( rx * 2 > DrawSizeX )
	{
		rx = DrawSizeX / 2 ;
	}

	if( ry * 2 > DrawSizeY )
	{
		ry = DrawSizeY / 2 ;
	}

	if( FillFlag )
	{
		NS_DrawBox( x1 + rx, y1,      x2 - rx, y2,      Color, FillFlag ) ;
		NS_DrawBox( x1,      y1 + ry, x1 + rx, y2 - ry, Color, FillFlag ) ;
		NS_DrawBox( x2 - rx, y1 + ry, x2,      y2 - ry, Color, FillFlag ) ;
	}
	else
	{
		NS_SetDrawArea( x1 + rx, y1, x2 - rx, y2 ) ;
		NS_DrawBox( x1, y1, x2, y2, Color, FillFlag ) ;

		NS_SetDrawArea( x1, y1 + ry, x1 + rx, y2 - ry ) ;
		NS_DrawBox( x1, y1, x2, y2, Color, FillFlag ) ;

		NS_SetDrawArea( x2 - rx, y1 + ry, x2, y2 - ry ) ;
		NS_DrawBox( x1, y1, x2, y2, Color, FillFlag ) ;

		NS_SetDrawArea( BackupDrawRect.left, BackupDrawRect.top, BackupDrawRect.right, BackupDrawRect.bottom ) ; 
	}

	SETRECT( GSYS.DrawSetting.DrawArea, x1, y1, x1 + rx, y1 + ry ) ;
	RectClipping( &GSYS.DrawSetting.DrawArea, &BackupDrawRect ) ;
	SetMemImgDrawArea( &GSYS.DrawSetting.DrawArea ) ;
	GSYS.DrawSetting.DrawAreaF.left   = ( float )GSYS.DrawSetting.DrawArea.left ;
	GSYS.DrawSetting.DrawAreaF.top    = ( float )GSYS.DrawSetting.DrawArea.top ;
	GSYS.DrawSetting.DrawAreaF.right  = ( float )GSYS.DrawSetting.DrawArea.right ;
	GSYS.DrawSetting.DrawAreaF.bottom = ( float )GSYS.DrawSetting.DrawArea.bottom ;
	NS_DrawOval( x1 + rx, y1 + ry, rx, ry, Color, FillFlag ) ;

	SETRECT( GSYS.DrawSetting.DrawArea, x2 - rx, y1, x2, y1 + ry ) ;
	RectClipping( &GSYS.DrawSetting.DrawArea, &BackupDrawRect ) ;
	SetMemImgDrawArea( &GSYS.DrawSetting.DrawArea ) ;
	GSYS.DrawSetting.DrawAreaF.left   = ( float )GSYS.DrawSetting.DrawArea.left ;
	GSYS.DrawSetting.DrawAreaF.top    = ( float )GSYS.DrawSetting.DrawArea.top ;
	GSYS.DrawSetting.DrawAreaF.right  = ( float )GSYS.DrawSetting.DrawArea.right ;
	GSYS.DrawSetting.DrawAreaF.bottom = ( float )GSYS.DrawSetting.DrawArea.bottom ;
	NS_DrawOval( x2 - rx - 1, y1 + ry, rx, ry, Color, FillFlag ) ;

	SETRECT( GSYS.DrawSetting.DrawArea, x1, y2 - ry, x1 + rx, y2 ) ;
	RectClipping( &GSYS.DrawSetting.DrawArea, &BackupDrawRect ) ;
	SetMemImgDrawArea( &GSYS.DrawSetting.DrawArea ) ;
	GSYS.DrawSetting.DrawAreaF.left   = ( float )GSYS.DrawSetting.DrawArea.left ;
	GSYS.DrawSetting.DrawAreaF.top    = ( float )GSYS.DrawSetting.DrawArea.top ;
	GSYS.DrawSetting.DrawAreaF.right  = ( float )GSYS.DrawSetting.DrawArea.right ;
	GSYS.DrawSetting.DrawAreaF.bottom = ( float )GSYS.DrawSetting.DrawArea.bottom ;
	NS_DrawOval( x1 + rx, y2 - ry - 1, rx, ry, Color, FillFlag ) ;

	SETRECT( GSYS.DrawSetting.DrawArea, x2 - rx, y2 - ry, x2, y2 ) ;
	RectClipping( &GSYS.DrawSetting.DrawArea, &BackupDrawRect ) ;
	SetMemImgDrawArea( &GSYS.DrawSetting.DrawArea ) ;
	GSYS.DrawSetting.DrawAreaF.left   = ( float )GSYS.DrawSetting.DrawArea.left ;
	GSYS.DrawSetting.DrawAreaF.top    = ( float )GSYS.DrawSetting.DrawArea.top ;
	GSYS.DrawSetting.DrawAreaF.right  = ( float )GSYS.DrawSetting.DrawArea.right ;
	GSYS.DrawSetting.DrawAreaF.bottom = ( float )GSYS.DrawSetting.DrawArea.bottom ;
	NS_DrawOval( x2 - rx - 1, y2 - ry - 1, rx, ry, Color, FillFlag ) ;

	// 描画矩形を元に戻す
	GSYS.DrawSetting.DrawArea = BackupDrawRect ;
	SetMemImgDrawArea( &BackupDrawRect ) ;
	GSYS.DrawSetting.DrawAreaF = BackupDrawRectF ;

	// 終了
	return 0 ;
}

// 点を描画する
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, x, y, x + 1, y + 1 )\
	DRAWRECT_DRAWAREA_CLIP
extern 	int NS_DrawPixel( int x, int y, unsigned int Color )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;
	
	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	// 描画
//	if( GSYS.DrawSetting.NotUseBasicGraphDraw3DDeviceMethodFlag ) Flag = FALSE ;
	DRAW_DEF(
		Graphics_Hardware_DrawPixel_PF( x, y, Color ),
		Graphics_Software_DrawPixel(    x, y, Color ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// 指定点から境界色があるところまで塗りつぶす
extern int NS_Paint( int x, int y, unsigned int FillColor, ULONGLONG BoundaryColor )
{
	// ハードウェアを使用しているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		// ハードウエアの機能を使用している場合
		if( Graphics_Hardware_Paint_PF( x, y, FillColor, BoundaryColor ) == 2 )
		{
			// ２が返ってきたらデフォルトの処理を行う

			MEMIMG          MemImg ;
			BASEIMAGE       ScreenImage ;
			int             DrawMode ;
			int             DrawBlendMode ;
			int             DrawBlendParam ;
			RGBCOLOR        DrawBright ;
			RECT            DrawArea ;
			const COLORDATA *MemImageColorData ;
			int				Red, Green, Blue ;
//			const COLORDATA *HardwareMainColorData ;

			// 画面のイメージを取得
			NS_CreateXRGB8ColorBaseImage( GSYS.DrawSetting.DrawSizeX, GSYS.DrawSetting.DrawSizeY, &ScreenImage ) ;
			NS_GetDrawScreenBaseImage( 0, 0, GSYS.DrawSetting.DrawSizeX, GSYS.DrawSetting.DrawSizeY, &ScreenImage ) ;

			// Paint 処理用の MEMIMG をセットアップする
			_MEMSET( &MemImg, 0, sizeof( MEMIMG ) ) ;
			InitializeMemImg(
				&MemImg,
				GSYS.DrawSetting.DrawSizeX,
				GSYS.DrawSetting.DrawSizeY,
				ScreenImage.Pitch,
				0,
				1,
				FALSE, FALSE, FALSE, ScreenImage.GraphData ) ;

			// Paint 処理を行う
			MemImageColorData     = GetMemImgColorData( 1, FALSE, FALSE ) ;
//			HardwareMainColorData = Graphics_Hardware_GetMainColorData_PF() ;
//			FillColor             = NS_GetColor4( MemImageColorData, HardwareMainColorData, FillColor ) ;
			NS_GetColor2( FillColor, &Red, &Green, &Blue ) ; 
			FillColor             = NS_GetColor3( MemImageColorData, Red, Green, Blue ) ;
#if defined( DX_GCC_COMPILE ) || defined( __ANDROID )
			if( BoundaryColor != 0xffffffffffffffffULL )
#else // defined( DX_GCC_COMPILE ) || defined( __ANDROID )
			if( BoundaryColor != 0xffffffffffffffff )
#endif // defined( DX_GCC_COMPILE ) || defined( __ANDROID )
			{
//				BoundaryColor = ( ULONGLONG )NS_GetColor4( MemImageColorData, HardwareMainColorData, ( unsigned int )BoundaryColor ) ;
				NS_GetColor2( ( unsigned int )BoundaryColor, &Red, &Green, &Blue ) ; 
				BoundaryColor = NS_GetColor3( MemImageColorData, Red, Green, Blue ) ;
			}
			PaintMemImg( &MemImg, x, y, FillColor, BoundaryColor ) ;

			// MEMIMG の後始末
			TerminateMemImg( &MemImg ) ;

			// 画面にペイント後の画像を描画
			DrawBlendMode  = GSYS.DrawSetting.BlendMode ;
			DrawBlendParam = GSYS.DrawSetting.BlendParam ;
			DrawMode       = GSYS.DrawSetting.DrawMode ;
			DrawBright     = GSYS.DrawSetting.DrawBright ;
			DrawArea       = GSYS.DrawSetting.DrawArea ;

			NS_SetDrawBlendMode( DX_BLENDMODE_SRCCOLOR, 255 ) ;
			NS_SetDrawMode( DX_DRAWMODE_NEAREST ) ;
			NS_SetDrawBright( 255, 255, 255 ) ;
			NS_SetDrawArea( 0, 0, GSYS.DrawSetting.DrawSizeX, GSYS.DrawSetting.DrawSizeY ) ;

			NS_DrawBaseImage( 0, 0, &ScreenImage ) ;

			NS_SetDrawBlendMode( DrawBlendMode, DrawBlendParam ) ;
			NS_SetDrawMode( DrawMode ) ;
			NS_SetDrawBright( DrawBright.Red, DrawBright.Green, DrawBright.Blue ) ;
			NS_SetDrawArea( DrawArea.left, DrawArea.top, DrawArea.right, DrawArea.bottom ) ;

			NS_ReleaseBaseImage( &ScreenImage ) ;

			// 正常終了
			return 0 ;
		}
	}
	else
	{
		MEMIMG *Img ;
		IMAGEDATA *Image ;

		// ハードウエアの機能を使用していない場合

		// エラー判定
		if( GRAPHCHKFULL( GSYS.DrawSetting.TargetScreen[ 0 ], Image ) )
			Image = NULL ;

		if( Image )
		{
			Img = &Image->Soft.MemImg ;
		}
		else
		{
			Img = &GSYS.SoftRender.MainBufferMemImg ;
		}

		PaintMemImg( Img, x, y, FillColor, BoundaryColor ) ;
	}
	
	// 終了
	return 0 ;
}

// 点の集合を描く
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, 0, 0, GSYS.DrawSetting.DrawSizeX, GSYS.DrawSetting.DrawSizeY )\
	DRAWRECT_DRAWAREA_CLIP
extern 	int NS_DrawPixelSet( const POINTDATA *PointData, int Num )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawPixelSet_PF( PointData, Num  ),
		Graphics_Software_DrawPixelSet(    PointData, Num  ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// 線の集合を描く
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, 0, 0, GSYS.DrawSetting.DrawSizeX, GSYS.DrawSetting.DrawSizeY )\
	DRAWRECT_DRAWAREA_CLIP
extern	int NS_DrawLineSet( const LINEDATA *LineData, int Num )
{
	int Ret = -1 ;
	int Flag ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;
	
	Flag = GSYS.Setting.ValidHardware ;

	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawLineSet_PF( LineData, Num ),
		Graphics_Software_DrawLineSet(    LineData, Num ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// ３Ｄの点を描画する
extern int NS_DrawPixel3D( VECTOR Pos, unsigned int Color )
{
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;
	int Ret ;
	int Flag ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ブレンドテクスチャは使用できない
	if( GSYS.DrawSetting.BlendGraph > 0 )
		return -1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, VConvFtoD( Pos ) ) ; 

		Pos = VGet( 0.0f, 0.0f, 0.0f ) ;
	}

	Flag = GSYS.Setting.ValidHardware ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawPixel3D_PF( Pos, Color, TRUE, NULL ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return Ret ;
}

// ３Ｄの点を描画する
extern int NS_DrawPixel3DD( VECTOR_D Pos, unsigned int Color )
{
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;
	VECTOR PosF ;
	int Ret ;
	int Flag ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ブレンドテクスチャは使用できない
	if( GSYS.DrawSetting.BlendGraph > 0 )
		return -1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, Pos ) ; 

		Pos = VGetD( 0.0, 0.0, 0.0 ) ;
	}

	PosF = VConvDtoF( Pos ) ;

	Flag = GSYS.Setting.ValidHardware ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawPixel3D_PF( PosF, Color, TRUE, NULL ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return Ret ;
}

// ３Ｄの線分を描画する
extern int NS_DrawLine3D( VECTOR Pos1, VECTOR Pos2, unsigned int Color )
{
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;
	int Ret ;
	int Flag ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ブレンドテクスチャは使用できない
	if( GSYS.DrawSetting.BlendGraph > 0 )
		return -1 ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, VConvFtoD( Pos1 ) ) ; 

		Pos2 = VSub( Pos2, Pos1 ) ;
		Pos1 = VGet( 0.0f, 0.0f, 0.0f ) ;
	}

	Flag = GSYS.Setting.ValidHardware ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawLine3D_PF( Pos1, Pos2, Color, TRUE, NULL ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return Ret ;
}

// ３Ｄの線分を描画する
extern int NS_DrawLine3DD( VECTOR_D Pos1, VECTOR_D Pos2, unsigned int Color )
{
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;
	VECTOR Pos1F, Pos2F ;
	int Ret ;
	int Flag ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ブレンドテクスチャは使用できない
	if( GSYS.DrawSetting.BlendGraph > 0 )
		return -1 ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, Pos1 ) ; 

		Pos2 = VSubD( Pos2, Pos1 ) ;
		Pos1 = VGetD( 0.0, 0.0, 0.0 ) ;
	}

	Pos1F = VConvDtoF( Pos1 ) ;
	Pos2F = VConvDtoF( Pos2 ) ;

	Flag = GSYS.Setting.ValidHardware ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawLine3D_PF( Pos1F, Pos2F, Color, TRUE, NULL ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return Ret ;
}

// ３Ｄの三角形を描画する
extern int NS_DrawTriangle3D( VECTOR Pos1, VECTOR Pos2, VECTOR Pos3, unsigned int Color, int FillFlag )
{
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;
	int Ret ;
	int Flag ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// ブレンドテクスチャは使用できない
	if( GSYS.DrawSetting.BlendGraph > 0 )
		return -1 ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, VConvFtoD( Pos1 ) ) ; 

		Pos2 = VSub( Pos2, Pos1 ) ;
		Pos3 = VSub( Pos3, Pos1 ) ;
		Pos1 = VGet( 0.0f, 0.0f, 0.0f ) ;
	}

	Flag = GSYS.Setting.ValidHardware ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawTriangle3D_PF( Pos1, Pos2, Pos3, Color, FillFlag, TRUE, NULL ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return Ret ;
}

// ３Ｄの三角形を描画する
extern int NS_DrawTriangle3DD( VECTOR_D Pos1, VECTOR_D Pos2, VECTOR_D Pos3, unsigned int Color, int FillFlag )
{
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;
	int Ret ;
	int Flag ;
	VECTOR Pos1F, Pos2F, Pos3F ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// ブレンドテクスチャは使用できない
	if( GSYS.DrawSetting.BlendGraph > 0 )
		return -1 ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, Pos1 ) ; 

		Pos2 = VSubD( Pos2, Pos1 ) ;
		Pos3 = VSubD( Pos3, Pos1 ) ;
		Pos1 = VGetD( 0.0, 0.0, 0.0 ) ;
	}

	Pos1F = VConvDtoF( Pos1 ) ;
	Pos2F = VConvDtoF( Pos2 ) ;
	Pos3F = VConvDtoF( Pos3 ) ;

	Flag = GSYS.Setting.ValidHardware ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawTriangle3D_PF( Pos1F, Pos2F, Pos3F, Color, FillFlag, TRUE, NULL ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return Ret ;
}

// ３Ｄの立方体を描画する
extern int NS_DrawCube3D( VECTOR Pos1, VECTOR Pos2, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	return NS_DrawCube3DD( VConvFtoD( Pos1 ), VConvFtoD( Pos2 ), DifColor, SpcColor, FillFlag ) ;
}

// ３Ｄの立方体を描画する
extern int NS_DrawCube3DD( VECTOR_D Pos1, VECTOR_D Pos2, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;
	VECTOR Pos1F, Pos2F ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, Pos1 ) ; 

		Pos2 = VSubD( Pos2, Pos1 ) ;
		Pos1 = VGetD( 0.0, 0.0, 0.0 ) ;
	}

	Pos1F = VConvDtoF( Pos1 ) ;
	Pos2F = VConvDtoF( Pos2 ) ;

	if( GSYS.Light.ProcessDisable == FALSE && FillFlag == TRUE )
	{
		static WORD FillIndex[ 36 ] =
		{
			 0,  1,  2,   3,  2,  1,
			 4,  5,  6,   7,  6,  5,
			 8,  9, 10,  11, 10,  9,
			12, 13, 14,  15, 14, 13,
			16, 17, 18,  19, 18, 17,
			20, 21, 22,  23, 22, 21,
		} ;
		VERTEX3D Vertex[ 6 ][ 4 ] ;
		int i, dr, dg, db, a, sr, sg, sb ;

		// 頂点データの作成
		NS_GetColor2( DifColor, &dr, &dg, &db ) ;
		NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
		a = GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GSYS.DrawSetting.BlendParam ;
		Vertex[ 0 ][ 0 ].pos.x = Pos1F.x ; Vertex[ 0 ][ 0 ].pos.y = Pos2F.y ; Vertex[ 0 ][ 0 ].pos.z = Pos1F.z ;
		Vertex[ 0 ][ 1 ].pos.x = Pos2F.x ; Vertex[ 0 ][ 1 ].pos.y = Pos2F.y ; Vertex[ 0 ][ 1 ].pos.z = Pos1F.z ;
		Vertex[ 0 ][ 2 ].pos.x = Pos1F.x ; Vertex[ 0 ][ 2 ].pos.y = Pos1F.y ; Vertex[ 0 ][ 2 ].pos.z = Pos1F.z ;
		Vertex[ 0 ][ 3 ].pos.x = Pos2F.x ; Vertex[ 0 ][ 3 ].pos.y = Pos1F.y ; Vertex[ 0 ][ 3 ].pos.z = Pos1F.z ;
		Vertex[ 0 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 0 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 0 ][ 0 ].norm.z = -1.0f ;
		Vertex[ 0 ][ 1 ].norm = Vertex[ 0 ][ 0 ].norm ;
		Vertex[ 0 ][ 2 ].norm = Vertex[ 0 ][ 0 ].norm ;
		Vertex[ 0 ][ 3 ].norm = Vertex[ 0 ][ 0 ].norm ;

		Vertex[ 1 ][ 0 ].pos.x = Pos2F.x ; Vertex[ 1 ][ 0 ].pos.y = Pos2F.y ; Vertex[ 1 ][ 0 ].pos.z = Pos2F.z ;
		Vertex[ 1 ][ 1 ].pos.x = Pos1F.x ; Vertex[ 1 ][ 1 ].pos.y = Pos2F.y ; Vertex[ 1 ][ 1 ].pos.z = Pos2F.z ;
		Vertex[ 1 ][ 2 ].pos.x = Pos2F.x ; Vertex[ 1 ][ 2 ].pos.y = Pos1F.y ; Vertex[ 1 ][ 2 ].pos.z = Pos2F.z ;
		Vertex[ 1 ][ 3 ].pos.x = Pos1F.x ; Vertex[ 1 ][ 3 ].pos.y = Pos1F.y ; Vertex[ 1 ][ 3 ].pos.z = Pos2F.z ;
		Vertex[ 1 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 1 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 1 ][ 0 ].norm.z = 1.0f ;
		Vertex[ 1 ][ 1 ].norm = Vertex[ 1 ][ 0 ].norm ;
		Vertex[ 1 ][ 2 ].norm = Vertex[ 1 ][ 0 ].norm ;
		Vertex[ 1 ][ 3 ].norm = Vertex[ 1 ][ 0 ].norm ;

		Vertex[ 2 ][ 0 ].pos.x = Pos1F.x ; Vertex[ 2 ][ 0 ].pos.y = Pos2F.y ; Vertex[ 2 ][ 0 ].pos.z = Pos2F.z ;
		Vertex[ 2 ][ 1 ].pos.x = Pos1F.x ; Vertex[ 2 ][ 1 ].pos.y = Pos2F.y ; Vertex[ 2 ][ 1 ].pos.z = Pos1F.z ;
		Vertex[ 2 ][ 2 ].pos.x = Pos1F.x ; Vertex[ 2 ][ 2 ].pos.y = Pos1F.y ; Vertex[ 2 ][ 2 ].pos.z = Pos2F.z ;
		Vertex[ 2 ][ 3 ].pos.x = Pos1F.x ; Vertex[ 2 ][ 3 ].pos.y = Pos1F.y ; Vertex[ 2 ][ 3 ].pos.z = Pos1F.z ;
		Vertex[ 2 ][ 0 ].norm.x = -1.0f ;
		Vertex[ 2 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 2 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 2 ][ 1 ].norm = Vertex[ 2 ][ 0 ].norm ;
		Vertex[ 2 ][ 2 ].norm = Vertex[ 2 ][ 0 ].norm ;
		Vertex[ 2 ][ 3 ].norm = Vertex[ 2 ][ 0 ].norm ;

		Vertex[ 3 ][ 0 ].pos.x = Pos2F.x ; Vertex[ 3 ][ 0 ].pos.y = Pos2F.y ; Vertex[ 3 ][ 0 ].pos.z = Pos1F.z ;
		Vertex[ 3 ][ 1 ].pos.x = Pos2F.x ; Vertex[ 3 ][ 1 ].pos.y = Pos2F.y ; Vertex[ 3 ][ 1 ].pos.z = Pos2F.z ;
		Vertex[ 3 ][ 2 ].pos.x = Pos2F.x ; Vertex[ 3 ][ 2 ].pos.y = Pos1F.y ; Vertex[ 3 ][ 2 ].pos.z = Pos1F.z ;
		Vertex[ 3 ][ 3 ].pos.x = Pos2F.x ; Vertex[ 3 ][ 3 ].pos.y = Pos1F.y ; Vertex[ 3 ][ 3 ].pos.z = Pos2F.z ;
		Vertex[ 3 ][ 0 ].norm.x = 1.0f ;
		Vertex[ 3 ][ 0 ].norm.y = 0.0f ;
		Vertex[ 3 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 3 ][ 1 ].norm = Vertex[ 3 ][ 0 ].norm ;
		Vertex[ 3 ][ 2 ].norm = Vertex[ 3 ][ 0 ].norm ;
		Vertex[ 3 ][ 3 ].norm = Vertex[ 3 ][ 0 ].norm ;

		Vertex[ 4 ][ 0 ].pos.x = Pos1F.x ; Vertex[ 4 ][ 0 ].pos.y = Pos2F.y ; Vertex[ 4 ][ 0 ].pos.z = Pos2F.z ;
		Vertex[ 4 ][ 1 ].pos.x = Pos2F.x ; Vertex[ 4 ][ 1 ].pos.y = Pos2F.y ; Vertex[ 4 ][ 1 ].pos.z = Pos2F.z ;
		Vertex[ 4 ][ 2 ].pos.x = Pos1F.x ; Vertex[ 4 ][ 2 ].pos.y = Pos2F.y ; Vertex[ 4 ][ 2 ].pos.z = Pos1F.z ;
		Vertex[ 4 ][ 3 ].pos.x = Pos2F.x ; Vertex[ 4 ][ 3 ].pos.y = Pos2F.y ; Vertex[ 4 ][ 3 ].pos.z = Pos1F.z ;
		Vertex[ 4 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 4 ][ 0 ].norm.y = 1.0f ;
		Vertex[ 4 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 4 ][ 1 ].norm = Vertex[ 4 ][ 0 ].norm ;
		Vertex[ 4 ][ 2 ].norm = Vertex[ 4 ][ 0 ].norm ;
		Vertex[ 4 ][ 3 ].norm = Vertex[ 4 ][ 0 ].norm ;

		Vertex[ 5 ][ 0 ].pos.x = Pos1F.x ; Vertex[ 5 ][ 0 ].pos.y = Pos1F.y ; Vertex[ 5 ][ 0 ].pos.z = Pos1F.z ;
		Vertex[ 5 ][ 1 ].pos.x = Pos2F.x ; Vertex[ 5 ][ 1 ].pos.y = Pos1F.y ; Vertex[ 5 ][ 1 ].pos.z = Pos1F.z ;
		Vertex[ 5 ][ 2 ].pos.x = Pos1F.x ; Vertex[ 5 ][ 2 ].pos.y = Pos1F.y ; Vertex[ 5 ][ 2 ].pos.z = Pos2F.z ;
		Vertex[ 5 ][ 3 ].pos.x = Pos2F.x ; Vertex[ 5 ][ 3 ].pos.y = Pos1F.y ; Vertex[ 5 ][ 3 ].pos.z = Pos2F.z ;
		Vertex[ 5 ][ 0 ].norm.x = 0.0f ;
		Vertex[ 5 ][ 0 ].norm.y = -1.0f ;
		Vertex[ 5 ][ 0 ].norm.z = 0.0f ;
		Vertex[ 5 ][ 1 ].norm = Vertex[ 5 ][ 0 ].norm ;
		Vertex[ 5 ][ 2 ].norm = Vertex[ 5 ][ 0 ].norm ;
		Vertex[ 5 ][ 3 ].norm = Vertex[ 5 ][ 0 ].norm ;
		for( i = 0 ; i < 4 * 6 ; i ++ )
		{
			( ( VERTEX3D * )Vertex )[ i ].dif.b = ( BYTE )db ;
			( ( VERTEX3D * )Vertex )[ i ].dif.g = ( BYTE )dg ;
			( ( VERTEX3D * )Vertex )[ i ].dif.r = ( BYTE )dr ;
			( ( VERTEX3D * )Vertex )[ i ].dif.a = ( BYTE )a ;
			( ( VERTEX3D * )Vertex )[ i ].spc.b = ( BYTE )sb ;
			( ( VERTEX3D * )Vertex )[ i ].spc.g = ( BYTE )sg ;
			( ( VERTEX3D * )Vertex )[ i ].spc.r = ( BYTE )sr ;
			( ( VERTEX3D * )Vertex )[ i ].spc.a = 0 ;
			( ( VERTEX3D * )Vertex )[ i ].u = 0.0f ;
			( ( VERTEX3D * )Vertex )[ i ].v = 0.0f ;
			( ( VERTEX3D * )Vertex )[ i ].su = 0.0f ;
			( ( VERTEX3D * )Vertex )[ i ].sv = 0.0f ;
		}

		NS_DrawPrimitiveIndexed3D( ( VERTEX3D * )Vertex, 6 * 4, FillIndex, 36, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		static WORD FillIndex[ 36 ] =
		{
			0, 1, 3,  3, 1, 2,  3, 2, 7,
			7, 2, 6,  2, 1, 6,  6, 1, 5,
			1, 0, 5,  5, 0, 4,  0, 3, 4,
			4, 3, 7,  7, 6, 4,  4, 6, 5,
		} ;
		static WORD LineIndex[ 24 ] =
		{
			0, 1,   1, 2,   2, 3,   3, 0,
			0, 4,   1, 5,   2, 6,   3, 7,
			4, 5,   5, 6,   6, 7,   7, 4,
		} ;
		VERTEX3D Vertex[ 8 ] ;
		int i, dr, dg, db, a, sr, sg, sb ;

		// 頂点データの作成
		NS_GetColor2( DifColor, &dr, &dg, &db ) ;
		NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
		a = GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GSYS.DrawSetting.BlendParam ;
		Vertex[ 0 ].pos.x = Pos1F.x ; Vertex[ 0 ].pos.y = Pos2F.y ; Vertex[ 0 ].pos.z = Pos2F.z ;
		Vertex[ 1 ].pos.x = Pos2F.x ; Vertex[ 1 ].pos.y = Pos2F.y ; Vertex[ 1 ].pos.z = Pos2F.z ;
		Vertex[ 2 ].pos.x = Pos2F.x ; Vertex[ 2 ].pos.y = Pos2F.y ; Vertex[ 2 ].pos.z = Pos1F.z ;
		Vertex[ 3 ].pos.x = Pos1F.x ; Vertex[ 3 ].pos.y = Pos2F.y ; Vertex[ 3 ].pos.z = Pos1F.z ;
		Vertex[ 4 ].pos.x = Pos1F.x ; Vertex[ 4 ].pos.y = Pos1F.y ; Vertex[ 4 ].pos.z = Pos2F.z ;
		Vertex[ 5 ].pos.x = Pos2F.x ; Vertex[ 5 ].pos.y = Pos1F.y ; Vertex[ 5 ].pos.z = Pos2F.z ;
		Vertex[ 6 ].pos.x = Pos2F.x ; Vertex[ 6 ].pos.y = Pos1F.y ; Vertex[ 6 ].pos.z = Pos1F.z ;
		Vertex[ 7 ].pos.x = Pos1F.x ; Vertex[ 7 ].pos.y = Pos1F.y ; Vertex[ 7 ].pos.z = Pos1F.z ;
		for( i = 0 ; i < 8 ; i ++ )
		{
			Vertex[ i ].dif.b = ( BYTE )db ;
			Vertex[ i ].dif.g = ( BYTE )dg ;
			Vertex[ i ].dif.r = ( BYTE )dr ;
			Vertex[ i ].dif.a = ( BYTE )a ;
			Vertex[ i ].spc.b = ( BYTE )sb ;
			Vertex[ i ].spc.g = ( BYTE )sg ;
			Vertex[ i ].spc.r = ( BYTE )sr ;
			Vertex[ i ].spc.a = 0 ;
			Vertex[ i ].u = 0.0f ;
			Vertex[ i ].v = 0.0f ;
			Vertex[ i ].su = 0.0f ;
			Vertex[ i ].sv = 0.0f ;
		}

		if( FillFlag == TRUE ) 
		{
			NS_DrawPrimitiveIndexed3D( Vertex, 8, FillIndex, 36, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
		}
		else
		{
			NS_DrawPrimitiveIndexed3D( Vertex, 8, LineIndex, 24, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
		}
	}

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return 0 ;
}

// ３Ｄの球体を描画する
extern int NS_DrawSphere3D( VECTOR CenterPos, float r, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	return NS_DrawSphere3DD( VConvFtoD( CenterPos ), r, DivNum, DifColor, SpcColor, FillFlag ) ;
}

// ３Ｄの球体を描画する
extern int NS_DrawSphere3DD( VECTOR_D CenterPos, double r, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	VERTEX3D *Vertex, *vert ;
	WORD *Index, *ind ;
	float *SinCosTable1, *SinCosTable2, *t1, *t2, rf ;
	int CirVertNumH, CirVertNumV, i, j, k, l, indexnum, vertnum ;
	int dr, dg, db, a, sr, sg, sb ;
	VECTOR CenterPosF ;
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, CenterPos ) ; 

		CenterPos.x = 0.0 ;
		CenterPos.y = 0.0 ;
		CenterPos.z = 0.0 ;
	}

	CenterPosF = VConvDtoF( CenterPos ) ;
	rf         = ( float )r ;

	NS_GetColor2( DifColor, &dr, &dg, &db ) ;
	NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
	a = GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GSYS.DrawSetting.BlendParam ;

	CirVertNumH = DivNum + 4 ;
	CirVertNumV = DivNum + 1 ;
	indexnum = FillFlag ? ( CirVertNumV - 1 ) * CirVertNumH * 6 + CirVertNumH * 3 * 2 : CirVertNumH * ( CirVertNumV + 1 ) * 2 + CirVertNumV * CirVertNumH * 2 ;
	vertnum = CirVertNumV * CirVertNumH + 2 ;
	Vertex = ( VERTEX3D * )DXALLOC( sizeof( WORD ) * indexnum + sizeof( VERTEX3D ) * vertnum + sizeof( float ) * ( CirVertNumV + CirVertNumH ) * 2 ) ;
	if( Vertex == NULL )
	{
		if( GSYS.DrawSetting.Large3DPositionSupport )
		{
			Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
		}
		return 0 ;
	}
	SinCosTable1 = ( float * )( Vertex + vertnum ) ;
	SinCosTable2 = SinCosTable1 + CirVertNumV * 2 ;
	Index = ( WORD * )( SinCosTable2 + CirVertNumH * 2 ) ;

	// 頂点データの作成
	t1 = SinCosTable1 ;
	t2 = SinCosTable2 ;
	for( i = 0 ; i < CirVertNumH ; i ++, t2 += 2 )
	{
		_SINCOS( 2 * ( float )DX_PI / CirVertNumH * i, t2, t2 + 1 ) ;
	}
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		_SINCOS( ( float )DX_PI / 2.0f - ( float )DX_PI / ( CirVertNumV + 1 ) * ( i + 1 ), t1, t1 + 1 ) ;
	}
	vert = Vertex ;
	t1 = SinCosTable1 ;
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		t2 = SinCosTable2 ;
		for( j = 0 ; j < CirVertNumH ; j ++, vert ++, t2 += 2 )
		{
			vert->pos.x = t2[ 1 ] * t1[ 1 ] * rf + CenterPosF.x ;
			vert->pos.y = t1[ 0 ] * rf           + CenterPosF.y ;
			vert->pos.z = t2[ 0 ] * t1[ 1 ] * rf + CenterPosF.z ;
			vert->norm.x = t2[ 1 ] * t1[ 1 ] ;
			vert->norm.y = t1[ 0 ] ;
			vert->norm.z = t2[ 0 ] * t1[ 1 ] ;

			vert->dif.b = ( BYTE )db ;
			vert->dif.g = ( BYTE )dg ;
			vert->dif.r = ( BYTE )dr ;
			vert->dif.a = ( BYTE )a ;
			vert->spc.b = ( BYTE )sb ;
			vert->spc.g = ( BYTE )sg ;
			vert->spc.r = ( BYTE )sr ;
			vert->spc.a = 0 ;
			vert->u = 0.0f ;
			vert->v = 0.0f ;
			vert->su = 0.0f ;
			vert->sv = 0.0f ;
		}
	}
	vert->pos.x = 0.0f + CenterPosF.x ;
	vert->pos.y =   rf + CenterPosF.y ;
	vert->pos.z = 0.0f + CenterPosF.z ;
	vert->norm.x = 0.0f ;
	vert->norm.y = 1.0f ;
	vert->norm.z = 0.0f ;

	vert->dif.b = ( BYTE )db ;
	vert->dif.g = ( BYTE )dg ;
	vert->dif.r = ( BYTE )dr ;
	vert->dif.a = ( BYTE )a ;
	vert->spc.b = ( BYTE )sb ;
	vert->spc.g = ( BYTE )sg ;
	vert->spc.r = ( BYTE )sr ;
	vert->spc.a = 0 ;
	vert->u = 0.0f ;
	vert->v = 0.0f ;
	vert->su = 0.0f ;
	vert->sv = 0.0f ;

	vert ++ ;

	vert->pos.x = 0.0f + CenterPosF.x ;
	vert->pos.y =  -rf + CenterPosF.y ;
	vert->pos.z = 0.0f + CenterPosF.z ;
	vert->norm.x = 0.0f ;
	vert->norm.y = -1.0f ;
	vert->norm.z = 0.0f ;

	vert->dif.b = ( BYTE )db ;
	vert->dif.g = ( BYTE )dg ;
	vert->dif.r = ( BYTE )dr ;
	vert->dif.a = ( BYTE )a ;
	vert->spc.b = ( BYTE )sb ;
	vert->spc.g = ( BYTE )sg ;
	vert->spc.r = ( BYTE )sr ;
	vert->spc.a = 0 ;
	vert->u = 0.0f ;
	vert->v = 0.0f ;
	vert->su = 0.0f ;
	vert->sv = 0.0f ;

	vert ++ ;

	// 塗りつぶすかどうかで処理を分岐
	if( FillFlag )
	{
		ind = Index ;
		l = CirVertNumV * CirVertNumH ;
		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )l ;
			ind[ 2 ] = ( WORD )( j + 1 ) ;
		}
		ind[ 0 ] = ( WORD )j ;
		ind[ 1 ] = ( WORD )l ;
		ind[ 2 ] = 0 ;

		ind += 3 ;

		k = 0 ;
		for( i = 0 ; i < CirVertNumV - 1 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 6 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ( WORD )( ind[ 0 ] + 1 ) ;
				ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
				
				ind[ 3 ] = ind[ 2 ] ;
				ind[ 4 ] = ind[ 1 ] ;
				ind[ 5 ] = ( WORD )( ind[ 3 ] + 1 ) ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
			
			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ( WORD )( k + CirVertNumH ) ;

			ind += 6 ;
		}

		l = CirVertNumV * CirVertNumH + 1 ;
		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )( j + 1 + k ) ;
			ind[ 1 ] = ( WORD )l ;
			ind[ 2 ] = ( WORD )( j + k ) ;
		}
		ind[ 0 ] = ( WORD )k ;
		ind[ 1 ] = ( WORD )l ;
		ind[ 2 ] = ( WORD )( j + k ) ;

		ind += 3 ;
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNumH ; i ++ )
		{
			ind[ 0 ] = ( WORD )( CirVertNumV * CirVertNumH ) ;
			ind[ 1 ] = ( WORD )i ;
			ind += 2 ;

			k = i ;
			for( j = 0 ; j < CirVertNumV - 1 ; j ++, k += CirVertNumH, ind += 2 )
			{
				ind[ 0 ] = ( WORD )k ;
				ind[ 1 ] = ( WORD )( k + CirVertNumH ) ;
			}
			
			ind[ 0 ] = ( WORD )k ;
			ind[ 1 ] = ( WORD )( CirVertNumV * CirVertNumH + 1 ) ;
			ind += 2 ;
		}

		k = 0 ;
		for( i = 0 ; i < CirVertNumV ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 2 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ( WORD )( j + k + 1 ) ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind += 2 ;
		}

		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
	}

	// メモリの解放
	DXFREE( Vertex ) ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return 0 ;
}

// ３Ｄのカプセルを描画
extern int NS_DrawCapsule3D( VECTOR Pos1, VECTOR Pos2, float r, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	return NS_DrawCapsule3DD( VConvFtoD( Pos1 ), VConvFtoD( Pos2 ), r, DivNum, DifColor, SpcColor, FillFlag ) ;
}

// ３Ｄのカプセルを描画
extern int NS_DrawCapsule3DD( VECTOR_D Pos1, VECTOR_D Pos2, double r, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	VERTEX3D *Vertex, *vert1, *vert2 ;
	WORD *Index, *ind ;
	VECTOR_D SubV, xv, yv, zv ;
	VECTOR xvf, yvf, zvf, Pos1F ;
	double Length ;
	float *SinCosTable1, *SinCosTable2, *t1, *t2, x, y, z, LengthF, rf ;
	int CirVertNumH, CirVertNumV, i, j, k, topind, botind, ettopind, etbotind, indexnum ;
	int dr, dg, db, a, sr, sg, sb, vertnum ;
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, Pos1 ) ; 

		Pos2 = VSubD( Pos2, Pos1 ) ;
		Pos1 = VGetD( 0.0, 0.0, 0.0 ) ;
	}

	rf = ( float )r ;
	Pos1F = VConvDtoF( Pos1 ) ;

	NS_GetColor2( DifColor, &dr, &dg, &db ) ;
	NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
	a = GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GSYS.DrawSetting.BlendParam ;

	SubV   = VSubD( Pos2, Pos1 ) ;
	Length = VectorInnerProductD( &SubV, &SubV ) ;
	if( Length < 0.00000000001 )
	{
		xvf = VGet( 1.0, 0.0, 0.0 ) ;
		yvf = VGet( 0.0, 1.0, 0.0 ) ;
		zvf = VGet( 0.0, 0.0, 1.0 ) ;
	}
	else
	{
		Length = _SQRTD( Length ) ;
		yv.x = SubV.x / Length ;
		yv.y = SubV.y / Length ;
		yv.z = SubV.z / Length ;

		xv = VGetD( 1.0, 0.0, 0.0 ) ;
		VectorOuterProductD( &zv, &xv, &yv ) ;
		if( VSquareSizeD( zv ) < 0.0000000001f )
		{
			xv = VGetD( 0.0, 0.0, 1.0 ) ;
			VectorOuterProductD( &zv, &xv, &yv ) ;
		}
		VectorOuterProductD( &xv, &yv, &zv ) ;
		VectorNormalizeD( &xv, &xv ) ;
		VectorNormalizeD( &zv, &zv ) ;

		xvf = VConvDtoF( xv ) ;
		yvf = VConvDtoF( yv ) ;
		zvf = VConvDtoF( zv ) ;
	}
	LengthF = ( float )Length ;

	CirVertNumH = DivNum + 4 ;
	CirVertNumV = DivNum + 2 ;
	indexnum = FillFlag ? ( CirVertNumH * 3 + CirVertNumH * 6 * ( CirVertNumV - 1 ) ) * 2 + CirVertNumH * 6 : CirVertNumH * ( CirVertNumV * 2 + 1 ) * 2 + CirVertNumH * CirVertNumV * 2 * 2 ;
	vertnum  = ( CirVertNumV * CirVertNumH + 1 ) * 2 ;
	Vertex   = ( VERTEX3D * )DXALLOC( sizeof( WORD ) * indexnum + sizeof( VERTEX3D ) * vertnum + sizeof( float ) * ( CirVertNumH + CirVertNumV ) * 2 ) ;
	if( Vertex == NULL )
	{
		if( GSYS.DrawSetting.Large3DPositionSupport )
		{
			Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
		}
		return 0 ;
	}
	SinCosTable1 = ( float * )( Vertex + vertnum ) ;
	SinCosTable2 = SinCosTable1 + CirVertNumV * 2 ;
	Index        = ( WORD * )( SinCosTable2 + CirVertNumH * 2 ) ;
	topind       = CirVertNumV * CirVertNumH * 2 ;
	botind       = CirVertNumV * CirVertNumH * 2 + 1 ;
	ettopind     = ( CirVertNumV - 1 ) * CirVertNumH ;
	etbotind     = CirVertNumV * CirVertNumH ;

	// 頂点データの作成
	t1 = SinCosTable1 ;
	t2 = SinCosTable2 ;
	for( i = 0 ; i < CirVertNumH ; i ++, t2 += 2 )
	{
		_SINCOS( 2 * ( float )DX_PI / CirVertNumH * i, t2, t2 + 1 ) ;
	}
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		_SINCOS( ( float )DX_PI / 2.0f - ( float )( DX_PI / 2.0f ) / CirVertNumV * ( i + 1 ), t1, t1 + 1 ) ;
	}
	vert1 = Vertex ;
	t1 = SinCosTable1 ;
	for( i = 0 ; i < CirVertNumV ; i ++, t1 += 2 )
	{
		t2 = SinCosTable2 ;
		vert2 = Vertex + etbotind + ( CirVertNumV - 1 - i ) * CirVertNumH ;
		for( j = 0 ; j < CirVertNumH ; j ++, vert1 ++, vert2 ++, t2 += 2 )
		{
			vert1->pos.x = t2[ 1 ] * t1[ 1 ] * rf ;
			vert1->pos.y = t1[ 0 ] * rf + LengthF ;
			vert1->pos.z = t2[ 0 ] * t1[ 1 ] * rf ;
			vert1->norm.x = t2[ 1 ] * t1[ 1 ] ;
			vert1->norm.y = t1[ 0 ] ;
			vert1->norm.z = t2[ 0 ] * t1[ 1 ] ;

			vert1->dif.b = ( BYTE )db ;
			vert1->dif.g = ( BYTE )dg ;
			vert1->dif.r = ( BYTE )dr ;
			vert1->dif.a = ( BYTE )a ;
			vert1->spc.b = ( BYTE )sb ;
			vert1->spc.g = ( BYTE )sg ;
			vert1->spc.r = ( BYTE )sr ;
			vert1->spc.a = 0 ;
			vert1->u = 0.0f ;
			vert1->v = 0.0f ;
			vert1->su = 0.0f ;
			vert1->sv = 0.0f ;

			vert2->pos.x =    vert1->pos.x ;
			vert2->pos.y = -( vert1->pos.y - LengthF ) ;
			vert2->pos.z =    vert1->pos.z ;
			vert2->norm.x =  vert1->norm.x ;
			vert2->norm.y = -vert1->norm.y ;
			vert2->norm.z =  vert1->norm.z ;

			vert2->dif.b = ( BYTE )db ;
			vert2->dif.g = ( BYTE )dg ;
			vert2->dif.r = ( BYTE )dr ;
			vert2->dif.a = ( BYTE )a ;
			vert2->spc.b = ( BYTE )sb ;
			vert2->spc.g = ( BYTE )sg ;
			vert2->spc.r = ( BYTE )sr ;
			vert2->spc.a = 0 ;
			vert2->u = 0.0f ;
			vert2->v = 0.0f ;
			vert2->su = 0.0f ;
			vert2->sv = 0.0f ;
		}
	}

	vert1 = Vertex + topind ;
	vert1->pos.x = 0.0f ;
	vert1->pos.y = rf + LengthF ;
	vert1->pos.z = 0.0f ;
	vert1->norm.x = 0.0f ;
	vert1->norm.y = 1.0f ;
	vert1->norm.z = 0.0f ;

	vert1->dif.b = ( BYTE )db ;
	vert1->dif.g = ( BYTE )dg ;
	vert1->dif.r = ( BYTE )dr ;
	vert1->dif.a = ( BYTE )a ;
	vert1->spc.b = ( BYTE )sb ;
	vert1->spc.g = ( BYTE )sg ;
	vert1->spc.r = ( BYTE )sr ;
	vert1->spc.a = 0 ;
	vert1->u = 0.0f ;
	vert1->v = 0.0f ;
	vert1->su = 0.0f ;
	vert1->sv = 0.0f ;

	vert1 ++ ;

	vert1->pos.x = 0.0f ;
	vert1->pos.y = -rf ;
	vert1->pos.z = 0.0f ;
	vert1->norm.x = 0.0f ;
	vert1->norm.y = -1.0f ;
	vert1->norm.z = 0.0f ;

	vert1->dif.b = ( BYTE )db ;
	vert1->dif.g = ( BYTE )dg ;
	vert1->dif.r = ( BYTE )dr ;
	vert1->dif.a = ( BYTE )a ;
	vert1->spc.b = ( BYTE )sb ;
	vert1->spc.g = ( BYTE )sg ;
	vert1->spc.r = ( BYTE )sr ;
	vert1->spc.a = 0 ;
	vert1->u = 0.0f ;
	vert1->v = 0.0f ;
	vert1->su = 0.0f ;
	vert1->sv = 0.0f ;

	vert1 ++ ;

	vert1 = Vertex ;
	for( i = 0 ; i < vertnum ; i ++, vert1 ++ )
	{
		x = vert1->pos.x ;
		y = vert1->pos.y ;
		z = vert1->pos.z ;
		vert1->pos.x = x * xvf.x + y * yvf.x + z * zvf.x + Pos1F.x ;
		vert1->pos.y = x * xvf.y + y * yvf.y + z * zvf.y + Pos1F.y ;
		vert1->pos.z = x * xvf.z + y * yvf.z + z * zvf.z + Pos1F.z ;
		x = vert1->norm.x ;
		y = vert1->norm.y ;
		z = vert1->norm.z ;
		vert1->norm.x = x * xvf.x + y * yvf.x + z * zvf.x ;
		vert1->norm.y = x * xvf.y + y * yvf.y + z * zvf.y ;
		vert1->norm.z = x * xvf.z + y * yvf.z + z * zvf.z ;
	}

	// 塗りつぶすかどうかで処理を分岐
	if( FillFlag )
	{
		ind = Index ;
		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )topind ;
			ind[ 2 ] = ( WORD )( j + 1 ) ;
		}
		ind[ 0 ] = ( WORD )j ;
		ind[ 1 ] = ( WORD )topind ;
		ind[ 2 ] = 0 ;

		ind += 3 ;

		// 上部の球部分のインデックスリストを作成
		k = 0 ;
		for( i = 0 ; i < CirVertNumV - 1 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 6 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ( WORD )( ind[ 0 ] + 1 ) ;
				ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
				
				ind[ 3 ] = ind[ 2 ] ;
				ind[ 4 ] = ind[ 1 ] ;
				ind[ 5 ] = ( WORD )( ind[ 3 ] + 1 ) ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
			
			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ( WORD )( k + CirVertNumH ) ;

			ind += 6 ;
		}

		// 中心の円柱部分のインデックスリストを作成
		for( i = 0 ; i < CirVertNumH - 1 ; i ++, ind += 6 )
		{
			ind[ 0 ] = ( WORD )( ettopind + i ) ;
			ind[ 1 ] = ( WORD )( ind[ 0 ] + 1 ) ;
			ind[ 2 ] = ( WORD )( etbotind + i ) ;

			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ( WORD )( ind[ 3 ] + 1 ) ;
		}
		ind[ 0 ] = ( WORD )( ettopind + i ) ;
		ind[ 1 ] = ( WORD )ettopind ;
		ind[ 2 ] = ( WORD )( etbotind + i ) ;

		ind[ 3 ] = ind[ 2 ] ;
		ind[ 4 ] = ind[ 1 ] ;
		ind[ 5 ] = ( WORD )etbotind ;

		ind += 6 ;
		k += CirVertNumH ;

		// 下部の球部分のインデックスリストを作成
		for( i = 0 ; i < CirVertNumV - 1 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 6 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ( WORD )( ind[ 0 ] + 1 ) ;
				ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
				
				ind[ 3 ] = ind[ 2 ] ;
				ind[ 4 ] = ind[ 1 ] ;
				ind[ 5 ] = ( WORD )( ind[ 3 ] + 1 ) ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind[ 2 ] = ( WORD )( ind[ 0 ] + CirVertNumH ) ;
			
			ind[ 3 ] = ind[ 2 ] ;
			ind[ 4 ] = ind[ 1 ] ;
			ind[ 5 ] = ( WORD )( k + CirVertNumH ) ;

			ind += 6 ;
		}

		for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )( j + 1 + k ) ;
			ind[ 1 ] = ( WORD )botind ;
			ind[ 2 ] = ( WORD )( j + k ) ;
		}
		ind[ 0 ] = ( WORD )k ;
		ind[ 1 ] = ( WORD )botind ;
		ind[ 2 ] = ( WORD )( j + k ) ;

		ind += 3 ;
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNumH ; i ++ )
		{
			ind[ 0 ] = ( WORD )topind ;
			ind[ 1 ] = ( WORD )i ;
			ind += 2 ;

			k = i ;
			for( j = 0 ; j < CirVertNumV * 2 - 1 ; j ++, k += CirVertNumH, ind += 2 )
			{
				ind[ 0 ] = ( WORD )k ;
				ind[ 1 ] = ( WORD )( k + CirVertNumH ) ;
			}
			
			ind[ 0 ] = ( WORD )k ;
			ind[ 1 ] = ( WORD )botind ;
			ind += 2 ;
		}

		k = 0 ;
		for( i = 0 ; i < CirVertNumV * 2 ; i ++, k += CirVertNumH )
		{
			for( j = 0 ; j < CirVertNumH - 1 ; j ++, ind += 2 )
			{
				ind[ 0 ] = ( WORD )( j + k ) ;
				ind[ 1 ] = ( WORD )( j + k + 1 ) ;
			}
			ind[ 0 ] = ( WORD )( j + k ) ;
			ind[ 1 ] = ( WORD )k ;
			ind += 2 ;
		}

		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
	}

	// メモリの解放
	DXFREE( Vertex ) ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return 0 ;
}

// ３Ｄの円錐を描画する
extern int NS_DrawCone3D( VECTOR TopPos, VECTOR BottomPos, float r, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	return NS_DrawCone3DD( VConvFtoD( TopPos ), VConvFtoD( BottomPos ), r, DivNum, DifColor, SpcColor, FillFlag ) ;
}

// ３Ｄの円錐を描画する
extern int NS_DrawCone3DD( VECTOR_D TopPos, VECTOR_D BottomPos, double r, int DivNum, unsigned int DifColor, unsigned int SpcColor, int FillFlag )
{
	VERTEX3D *Vertex, *vert1, *vert2, *vert3 ;
	MATRIX   Axis ;
	WORD *Index, *ind ;
	VECTOR_D SubV, xv, yv, zv ;
	VECTOR bottomvec, norm, TopPosF ;
	double Length ;
	float *SinCosTable, *t, rf ;
	int CirVertNum, vertnum, indexnum ;
	int dr, dg, db, a, sr, sg, sb, i, j, k, topind, bottomind, num ;
	LARGE3DPOSITIONSUPPORT_DRAWINFO Large3DPosDrawInfo ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_BeginLarge3DPositionSupportDraw( &Large3DPosDrawInfo, TopPos ) ; 

		BottomPos = VSubD( BottomPos, TopPos ) ;
		TopPos    = VGetD( 0.0, 0.0, 0.0 ) ;
	}

	TopPosF = VConvDtoF( TopPos ) ;
	rf      = ( float )r ;

	NS_GetColor2( DifColor, &dr, &dg, &db ) ;
	NS_GetColor2( SpcColor, &sr, &sg, &sb ) ;
	a = GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ? 255 : GSYS.DrawSetting.BlendParam ;

	SubV   = VSubD( TopPos, BottomPos ) ;
	Length = VSizeD( SubV ) ;
	zv     = VScaleD( SubV, 1.0 / Length ) ;
	xv     = VGetD( 1.0, 0.0, 0.0 ) ;
	VectorOuterProductD( &yv, &xv, &zv ) ;
	if( VSquareSizeD( yv ) < 0.000000001 )
	{
		xv = VGetD( 0.0, 0.0, 1.0 ) ;
		VectorOuterProductD( &yv, &xv, &zv ) ;
	}
	xv     = VNormD( VCrossD( yv, zv ) ) ;
	yv     = VNormD( yv ) ;
	Axis   = MGetAxis1( VConvDtoF( xv ), VConvDtoF( yv ), VConvDtoF( zv ), VConvDtoF( BottomPos ) ) ;

	norm = VConvDtoF( VNormD( VCrossD( VGetD( 0.0, 0.0, 1.0 ), VSubD( VGetD( r, 0.0, 0.0 ), VGetD( 0.0, Length, 0.0 ) ) ) ) ) ;

	CirVertNum = DivNum + 4 ;
	vertnum    = CirVertNum * 3 ;
	topind     = CirVertNum ;
	bottomind  = CirVertNum * 2 ;
	indexnum   = FillFlag ? CirVertNum * 3 + ( CirVertNum - 2 ) * 3 : CirVertNum * 2 + CirVertNum * 2 ;
	Vertex     = ( VERTEX3D * )DXALLOC( sizeof( WORD ) * indexnum + sizeof( VERTEX3D ) * vertnum + sizeof( float ) * CirVertNum * 2 ) ;
	if( Vertex == NULL )
	{
		if( GSYS.DrawSetting.Large3DPositionSupport )
		{
			Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
		}
		return 0 ;
	}
	SinCosTable = ( float * )( Vertex + vertnum ) ;
	Index       = ( WORD * )( SinCosTable + CirVertNum * 2 ) ;

	// 頂点データの作成
	t = SinCosTable ;
	for( i = 0 ; i < CirVertNum ; i ++, t += 2 )
	{
		_SINCOS( 2 * DX_PI_F / CirVertNum * i, t, t + 1 ) ;
	}
	vert1 = Vertex ;
	vert2 = Vertex + CirVertNum ;
	vert3 = Vertex + CirVertNum * 2 ;
	t = SinCosTable ;
	bottomvec = VTransformSR( VGet( 0.0f, 0.0f, -1.0f ), Axis ) ;
	for( i = 0 ; i < CirVertNum ; i ++, vert1 ++, vert2 ++, vert3 ++, t += 2 )
	{
		vert1->pos = VTransform( VGet( t[ 0 ] * rf, t[ 1 ] * rf, 0.0f ), Axis ) ;
		vert2->pos = TopPosF ;
		vert3->pos = vert1->pos ;
		vert1->norm = VTransformSR( VGet( t[ 0 ] * norm.x, t[ 1 ] * norm.x, norm.y ), Axis ) ;
		vert2->norm = vert1->norm ;
		vert3->norm = bottomvec ;

		vert1->dif.b = ( BYTE )db ; vert1->dif.g = ( BYTE )dg ; vert1->dif.r = ( BYTE )dr ; vert1->dif.a = ( BYTE )a ;
		vert2->dif.b = ( BYTE )db ; vert2->dif.g = ( BYTE )dg ; vert2->dif.r = ( BYTE )dr ; vert2->dif.a = ( BYTE )a ;
		vert3->dif.b = ( BYTE )db ; vert3->dif.g = ( BYTE )dg ; vert3->dif.r = ( BYTE )dr ; vert3->dif.a = ( BYTE )a ;
		vert1->spc.b = ( BYTE )sb ; vert1->spc.g = ( BYTE )sg ; vert1->spc.r = ( BYTE )sr ; vert1->spc.a = 0 ;
		vert2->spc.b = ( BYTE )sb ; vert2->spc.g = ( BYTE )sg ; vert2->spc.r = ( BYTE )sr ; vert2->spc.a = 0 ;
		vert3->spc.b = ( BYTE )sb ; vert3->spc.g = ( BYTE )sg ; vert3->spc.r = ( BYTE )sr ; vert3->spc.a = 0 ;
		vert1->u = 0.0f ; vert1->v = 0.0f ;
		vert2->u = 0.0f ; vert2->v = 0.0f ;
		vert3->u = 0.0f ; vert3->v = 0.0f ;
		vert1->su = 0.0f ; vert1->sv = 0.0f ;
		vert2->su = 0.0f ; vert2->sv = 0.0f ;
		vert3->su = 0.0f ; vert3->sv = 0.0f ;
	}

	// 塗りつぶすかどうかで処理を分岐
	if( FillFlag )
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNum - 1 ; i ++, ind += 3 )
		{
			ind[ 0 ] = ( WORD )i ;
			ind[ 1 ] = ( WORD )( topind + i ) ;
			ind[ 2 ] = ( WORD )( i + 1 ) ;
		}
		ind[ 0 ] = ( WORD )i ;
		ind[ 1 ] = ( WORD )( topind + i ) ;
		ind[ 2 ] = 0 ;
		ind += 3 ;

		j = bottomind + CirVertNum - 1 ;
		i = bottomind ;
		ind[ 0 ] = ( WORD )i ;
		ind[ 1 ] = ( WORD )( i + 1 ) ;
		ind[ 2 ] = ( WORD )j ;
		ind += 3 ;

		num = CirVertNum - 2 ;
		for( k = 1 ; k < num ; )
		{
			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )( i + 1 ) ;
			ind[ 2 ] = ( WORD )( i + 2 ) ;
			ind += 3 ;
			k ++ ;
			i ++ ;
			if( k >= num ) break ;

			ind[ 0 ] = ( WORD )j ;
			ind[ 1 ] = ( WORD )( i + 1 ) ;
			ind[ 2 ] = ( WORD )( j - 1 ) ;
			ind += 3 ;

			j -- ;
			k ++ ;
			if( k >= num ) break ;
		}
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_TRIANGLELIST, DX_NONE_GRAPH, TRUE ) ;
	}
	else
	{
		ind = Index ;
		for( i = 0 ; i < CirVertNum ; i ++, ind += 2 )
		{
			ind[ 0 ] = ( WORD )i ;
			ind[ 1 ] = ( WORD )( topind + i ) ;
		}
		for( i = 0 ; i < CirVertNum - 1 ; i ++, ind += 2 )
		{
			ind[ 0 ] = ( WORD )( bottomind + i ) ;
			ind[ 1 ] = ( WORD )( bottomind + i + 1 ) ;
		}
		ind[ 0 ] = ( WORD )( bottomind + i ) ;
		ind[ 1 ] = ( WORD )bottomind ;
		NS_DrawPrimitiveIndexed3D( Vertex, vertnum, Index, indexnum, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, TRUE ) ;
	}

	// メモリの解放
	DXFREE( Vertex ) ;

	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Graphics_Draw_EndLarge3DPositionSupportDraw( &Large3DPosDrawInfo ) ;
	}

	// 終了
	return 0 ;
}


























// 画像描画関数

// ＢＭＰファイルを読みこんで画面に描画する
extern	int NS_LoadGraphScreen( int x, int y, const TCHAR *GraphName, int TransFlag )
{
#ifdef UNICODE
	return LoadGraphScreen_WCHAR_T(
		x, y, GraphName, TransFlag
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( GraphName, return -1 )

	Result = LoadGraphScreen_WCHAR_T(
		x, y, UseGraphNameBuffer, TransFlag
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( GraphName )

	return Result ;
#endif
}

// ＢＭＰファイルを読みこんで画面に描画する
extern	int LoadGraphScreen_WCHAR_T( int x, int y, const wchar_t *GraphName, int TransFlag )
{
	int GrHandle ;
	LOADGRAPH_GPARAM GParam ;
	
//	if( GSYS.NotDrawFlag ) return 0 ;

	// グラフィックを読みこむ
	Graphics_Image_InitLoadGraphGParam( &GParam ) ;
	GrHandle = Graphics_Image_LoadBmpToGraph_UseGParam( &GParam, FALSE, -1, GraphName, TRUE, FALSE, DX_MOVIESURFACE_NORMAL, FALSE ) ;
	if( GrHandle < 0 )
	{
		return -1 ;
	}

	// グラフィックの描画
	NS_DrawGraph( x, y, GrHandle, TransFlag ) ;

	// グラフィックの削除
	NS_DeleteGraph( GrHandle ) ;

	// 終了
	return 0 ;
}

// グラフィックの描画
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, x, y, x + Image->Width, y + Image->Height )\
	DRAWRECT_DRAWAREA_CLIP
extern	int NS_DrawGraph( int x, int y, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF( 
		Graphics_Hardware_DrawGraph_PF( x, y, ( float )x, ( float )y, Image, BlendImage, TransFlag, TRUE ),
		Graphics_Software_DrawGraph(    x, y,                         Image,             TransFlag       ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

// グラフィックの描画
extern	int NS_DrawGraphF( float xf, float yf, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;
	int x, y ;

	x = _FTOL( xf ) ;
	y = _FTOL( yf ) ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawGraph_PF( x, y, xf, yf, Image, BlendImage, TransFlag, FALSE ),
		Graphics_Software_DrawGraph(    x, y,         Image,             TransFlag        ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// グラフィックの拡大縮小描画
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, x1, y1, x2, y2 ) ;\
	if( x1 > x2 ){ DrawRect.left = x2 ; DrawRect.right = x1; }\
	if( y1 > y2 ){ DrawRect.top = y2 ; DrawRect.bottom = y1; }\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawExtendGraph( int x1, int y1, int x2, int y2, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;
	
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// テクスチャの場合とそれ以外で処理を分岐
	DRAW_DEF(
		Graphics_Hardware_DrawExtendGraph_PF( x1, y1, x2, y2, (float)x1, (float)y1, (float)x2, (float)y2, Image, BlendImage, TransFlag, TRUE ),
		Graphics_Software_DrawExtendGraph(    x1, y1, x2, y2,                                             Image,             TransFlag       ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

// グラフィックの拡大縮小描画
extern	int NS_DrawExtendGraphF( float x1f, float y1f, float x2f, float y2f, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;
	int x1, y1, x2, y2 ;
	
	x1 = _FTOL( x1f ) ;
	y1 = _FTOL( y1f ) ;
	x2 = _FTOL( x2f ) ;
	y2 = _FTOL( y2f ) ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawExtendGraph_PF( x1, y1, x2, y2, x1f, y1f, x2f, y2f, Image, BlendImage, TransFlag, FALSE ),
		Graphics_Software_DrawExtendGraph(    x1, y1, x2, y2,                     Image,             TransFlag        ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// グラフィックの回転描画
#define SETDRAWRECTCODE\
	int Width, Height ;\
	int i ;\
	float SinF, CosF ;\
	double xb, Lx, Ly ;\
	double px[4], py[4] ;\
	double Sin, Cos ;\
	double MaxMin[4] ; /* 0:MinX 1:MinY 2:MaxX 3:MaxY */\
\
	Lx = x ; Ly = y ;\
\
	_SINCOS( (float)Angle, &SinF, &CosF ) ;\
	Sin = (double)SinF ;\
	Cos = (double)CosF ;\
\
	Width = Image->Width / 2 ;\
	Height = Image->Height / 2 ;\
\
	px[0] = -Width ;				py[0] = -Height ;\
	px[1] = Image->Width - Width ;	py[1] = -Height ;\
	px[2] = -Width ;				py[2] = Image->Height - Height ;\
	px[3] = Image->Width - Width ;	py[3] = Image->Height - Height ;\
\
	xb = px[0] ;\
	px[0] = ( xb * Cos - py[0] * Sin ) * ExRate + Lx ;\
	py[0] = ( xb * Sin + py[0] * Cos ) * ExRate + Ly ;\
	MaxMin[0] = px[0] ;\
	MaxMin[1] = py[0] ;\
	MaxMin[2] = px[0] ;\
	MaxMin[3] = py[0] ;\
	for( i = 1 ; i < 4 ; i ++ )\
	{\
		xb = px[i] ;\
		px[i] = ( xb * Cos - py[i] * Sin ) * ExRate + Lx ;\
		py[i] = ( xb * Sin + py[i] * Cos ) * ExRate + Ly ;\
\
		if( MaxMin[0] > px[i] ) MaxMin[0] = px[i];\
		if( MaxMin[2] < px[i] ) MaxMin[2] = px[i];\
		if( MaxMin[1] > py[i] ) MaxMin[1] = py[i];\
		if( MaxMin[3] < py[i] ) MaxMin[3] = py[i];\
	}\
\
	MaxMin[0] -= 5.0F ; MaxMin[1] -= 5.0F ;\
	MaxMin[2] += 5.0F ; MaxMin[3] += 5.0F ; \
\
	_DTOL4( MaxMin, (int *)&DrawRect ) ;\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawRotaGraph( int x, int y, double ExRate, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawRotaGraph_PF( x, y, (float)x, (float)y, ExRate, Angle, Image, BlendImage, TransFlag, TurnFlag, TRUE ),
		Graphics_Software_DrawRotaGraph(          (float)x, (float)y, ExRate, Angle, Image,             TransFlag, TurnFlag       ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

// グラフィックの回転描画
extern	int NS_DrawRotaGraphF( float xf, float yf, double ExRate, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;
	int x, y ;
	
	x = _FTOL( xf ) ;
	y = _FTOL( yf ) ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawRotaGraph_PF( x, y, xf, yf, ExRate, Angle, Image, BlendImage, TransFlag, TurnFlag, FALSE ),
		Graphics_Software_DrawRotaGraph(          xf, yf, ExRate, Angle, Image,             TransFlag, TurnFlag        ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// グラフィックの回転描画２
extern	int NS_DrawRotaGraph2( int x, int y, int cx, int cy, double ExtRate, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	int w, h ;

	NS_GetGraphSize( GrHandle, &w,&h ) ;

	if( Angle == 0.0 && ExtRate == 1.0 )
	{
		if( TurnFlag )	return NS_DrawTurnGraph( x - cx, y - cy, GrHandle, TransFlag ) ;
		else			return NS_DrawGraph( x - cx, y - cy, GrHandle, TransFlag ) ;
	}
	else
	{
		return NS_DrawRotaGraph2F( ( float )x, ( float )y, ( float )cx, ( float )cy, ExtRate, Angle, GrHandle, TransFlag, TurnFlag ) ;
	}
}

// グラフィックの回転描画３
extern	int NS_DrawRotaGraph3( int x, int y, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	if( Angle == 0.0 && ExtRateX == 1.0 && ExtRateY == 1.0 )
	{
		int w, h ;

		NS_GetGraphSize( GrHandle, &w,&h ) ;

		if( TurnFlag )	return NS_DrawTurnGraph( x - cx, y - cy, GrHandle, TransFlag ) ;
		else			return NS_DrawGraph( x - cx, y - cy, GrHandle, TransFlag ) ;
	}
	else
	{
		return NS_DrawRotaGraph3F( ( float )x, ( float )y, ( float )cx, ( float )cy, ExtRateX, ExtRateY, Angle, GrHandle, TransFlag, TurnFlag ) ;
	}
}

// 画像の自由変形描画
extern	int NS_DrawModiGraph( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int GrHandle, int TransFlag )
{
	return Graphics_Draw_ModiGraphBase( x1, y1, x2, y2, x3, y3, x4, y4, GrHandle, TransFlag, false ) ;
}

// 画像の左右反転描画
extern	int NS_DrawTurnGraph( int x, int y, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	return NS_DrawExtendGraph( Image->Width + x, y, x, y + Image->Height, GrHandle, TransFlag ) ;
}





// グラフィックの回転描画２
extern	int NS_DrawRotaGraph2F( float xf, float yf, float cxf, float cyf, double ExtRate, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
#if 1
	int w, h ;

	NS_GetGraphSize( GrHandle, &w,&h ) ;

	if( Angle == 0.0 )
	{
		if( ExtRate == 1.0 )
		{
			if( TurnFlag )	return NS_DrawTurnGraphF( xf - cxf, yf - cyf, GrHandle, TransFlag ) ;
			else			return NS_DrawGraphF( xf - cxf, yf - cyf, GrHandle, TransFlag ) ;
		}
		else
		{
			float numF[ 4 ] ;

			numF[ 0 ] = xf - ( cxf * ( float )ExtRate ) ;
			numF[ 1 ] = yf - ( cyf * ( float )ExtRate ) ;
			numF[ 2 ] = numF[ 0 ] + ( w * ( float )ExtRate ) ;
			numF[ 3 ] = numF[ 1 ] + ( h * ( float )ExtRate ) ;
			if( TurnFlag )
			{
				float d ;

				d = numF[ 0 ] ; numF[ 0 ] = numF[ 2 ] ; numF[ 2 ] = d ;
			}
			return NS_DrawExtendGraphF( numF[0], numF[1],
										numF[2], numF[3],
										 GrHandle, TransFlag ) ;
		}
	}
	else
	{
		float dcx, dcy ;
		float Sin, Cos ;
		float bcx, bcy ;

		bcx = ( -cxf + ( float )w * 0.5F ) * ( float )ExtRate ;
		bcy = ( -cyf + ( float )h * 0.5F ) * ( float )ExtRate ;

		_SINCOS( ( float )Angle, &Sin, &Cos ) ;
		dcx = bcx * Cos - bcy * Sin ;
		dcy = bcx * Sin + bcy * Cos ;

		return NS_DrawRotaGraphF( xf + dcx, yf + dcy, ExtRate, Angle, GrHandle, TransFlag, TurnFlag ) ; 
	}
#else
/*
	double xx[4],yy[4];
	int xxi[4], yyi[4] ;
	int w, h, i, Result ;
	double xb, yb, xd, yd ;
	double Sin, Cos ;
	float SinF, CosF ;

	NS_GetGraphSize( GrHandle, &w,&h ) ;

	xx[0] = (   - cx )	* ExtRate ;	yy[0] = (   - cy )	* ExtRate ;
	xx[1] = ( w - cx )	* ExtRate ;	yy[1] = yy[0] ;
	xx[2] = xx[0] ;					yy[2] = ( h - cy )	* ExtRate ;
	xx[3] = xx[1] ;					yy[3] = yy[2] ;

	_SINCOS( (float)Angle, &SinF, &CosF ) ;
	Sin = (double)SinF ;
	Cos = (double)CosF ;
//	Sin = sin( Angle ) ;
//	Cos = cos( Angle ) ;
	xd = (double)x ;
	yd = (double)y ;
	for( i = 0 ; i < 4 ; i ++ )
	{
		xb = xx[i] * Cos - yy[i] * Sin ;
		yb = xx[i] * Sin + yy[i] * Cos ;
		xx[i] = xb + xd ;
		yy[i] = yb + yd ;
	}

	if( TurnFlag )
	{
		double d ;

		d = xx[0] ; xx[0] = xx[1] ; xx[1] = d ;
		d = xx[2] ; xx[2] = xx[3] ; xx[3] = d ;
		d = yy[0] ; yy[0] = yy[1] ; yy[1] = d ;
		d = yy[2] ; yy[2] = yy[3] ; yy[3] = d ;
	}

	_DTOL4( xx, xxi ) ;
	_DTOL4( yy, yyi ) ;
	Result = NS_DrawModiGraph( xxi[0],yyi[0], xxi[1],yyi[1], xxi[3],yyi[3], xxi[2],yyi[2], GrHandle, TransFlag ) ;

	return Result ;
*/
#endif
}


// グラフィックの回転描画３
extern	int NS_DrawRotaGraph3F( float xf, float yf, float cxf, float cyf, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	int w, h ;

	NS_GetGraphSize( GrHandle, &w,&h ) ;

	if( Angle == 0.0 )
	{
		if( ExtRateX == 1.0 && ExtRateY == 1.0 )
		{
			if( TurnFlag )	return NS_DrawTurnGraphF( xf - cxf, yf - cyf, GrHandle, TransFlag ) ;
			else			return NS_DrawGraphF( xf - cxf, yf - cyf, GrHandle, TransFlag ) ;
		}
		else
		{
			float numF[ 4 ] ;

			numF[ 0 ] = xf - ( cxf * ( float )ExtRateX ) ;
			numF[ 1 ] = yf - ( cyf * ( float )ExtRateY ) ;
			numF[ 2 ] = numF[ 0 ] + ( w * ( float )ExtRateX ) ;
			numF[ 3 ] = numF[ 1 ] + ( h * ( float )ExtRateY ) ;
			if( TurnFlag )
			{
				float d ;

				d = numF[ 0 ] ; numF[ 0 ] = numF[ 2 ] ; numF[ 2 ] = d ;
			}
			return NS_DrawExtendGraphF( numF[0], numF[1],
										numF[2], numF[3],
										 GrHandle, TransFlag ) ;
		}
	}
	else
	{
		if( ExtRateX == 1.0 && ExtRateY == 1.0 )
		{
			float dcx, dcy ;
			float Sin, Cos ;
			float bcx, bcy ;

			bcx = ( -cxf + ( float )w * 0.5F ) ;
			bcy = ( -cyf + ( float )h * 0.5F ) ;

			_SINCOS( ( float )Angle, &Sin, &Cos ) ;
			dcx = bcx * Cos - bcy * Sin ;
			dcy = bcx * Sin + bcy * Cos ;

			return NS_DrawRotaGraphF( xf + dcx, yf + dcy, 1.0, Angle, GrHandle, TransFlag, TurnFlag ) ; 
		}
		else
		{
			float xx[ 4 ], yy[ 4 ] ;
			int w, h, i ;
			float SinF, CosF ;
			float xb, yb ;

			NS_GetGraphSize( GrHandle, &w,&h ) ;

			xx[0] = (   - cxf )	* ( float )ExtRateX ;	yy[0] = (   - cyf )	* ( float )ExtRateY ;
			xx[1] = ( w - cxf )	* ( float )ExtRateX ;	yy[1] = yy[0] ;
			xx[2] = xx[0] ;								yy[2] = ( h - cyf )	* ( float )ExtRateY ;
			xx[3] = xx[1] ;								yy[3] = yy[2] ;

			_SINCOS( ( float )Angle, &SinF, &CosF ) ;
			for( i = 0 ; i < 4 ; i ++ )
			{
				xb = xx[ i ] * CosF - yy[ i ] * SinF ;
				yb = xx[ i ] * SinF + yy[ i ] * CosF ;
				xx[ i ] = xb + xf ;
				yy[ i ] = yb + yf ;
			}

			if( TurnFlag )
			{
				float d ;

				d = xx[ 0 ] ; xx[ 0 ] = xx[ 1 ] ; xx[ 1 ] = d ;
				d = xx[ 2 ] ; xx[ 2 ] = xx[ 3 ] ; xx[ 3 ] = d ;
				d = yy[ 0 ] ; yy[ 0 ] = yy[ 1 ] ; yy[ 1 ] = d ;
				d = yy[ 2 ] ; yy[ 2 ] = yy[ 3 ] ; yy[ 3 ] = d ;
			}

			return Graphics_Draw_ModiGraphFBase( xx[0], yy[0], xx[1], yy[1], xx[3], yy[3], xx[2], yy[2], GrHandle, TransFlag, true ) ;
		}
	}
}

// 画像の自由変形描画( float 版 )
extern	int NS_DrawModiGraphF( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int GrHandle, int TransFlag )
{
	return Graphics_Draw_ModiGraphFBase( x1, y1, x2, y2, x3, y3, x4, y4, GrHandle, TransFlag, false ) ;
}


// 画像の左右反転描画
extern	int NS_DrawTurnGraphF( float xf, float yf, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	return NS_DrawExtendGraphF( Image->Width + xf, yf, xf, yf + Image->Height, GrHandle, TransFlag ) ;
}

// チップグラフィックを使ったマップ描画
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, Sx, Sy, Sx + XNum * Image->Width , Sy + YNum * Image->Height )\
	DRAWRECT_DRAWAREA_CLIP
extern	int NS_DrawChipMap( int Sx, int Sy, int XNum, int YNum, const int *MapData, int ChipTypeNum, int MapDataPitch, const int *ChipGrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret = -1 ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( ChipGrHandle[0], Image ) )
		return -1 ;

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawChipMap( Sx, Sy, XNum, YNum, MapData, MapDataPitch, ChipTypeNum, ChipGrHandle, TransFlag ),
		Graphics_Software_DrawChipMap( Sx, Sy, XNum, YNum, MapData, MapDataPitch, ChipTypeNum, ChipGrHandle, TransFlag ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// チップグラフィックを使ったマップ描画
// int MapWidth, MapHeight : マップデータ全体の幅と高さ
// int *MapData : マップデータが格納されたアドレス
// int ChipTypeNum : マップに使うチップの種類の数
// int *ChipGrHandle : チップのグラフィック配列のアドレス
// int TransFlag : 透過色処理の有無
// int MapDrawX, MapDrawY : 描画するマップの起点となる左上座標
// int MapDrawWidth, MapDrawHeight : 起点座標から描画するマップの幅と高さ
// int ScreenX, ScreenY : マップを描画する画面上の座標
extern	int NS_DrawChipMap( int MapWidth, int MapHeight, const int *MapData, int ChipTypeNum, const int *ChipGrHandle, int TransFlag, int MapDrawPointX, int MapDrawPointY, int MapDrawWidth, int MapDrawHeight, int ScreenX, int ScreenY )
{
	int Width, Height ;
	int Result ;
	
	// チップグラフィックのサイズを得る
	NS_GetGraphSize( ChipGrHandle[0], &Width, &Height ) ;

	// マップ情報がはみ出る場合の補正処理
	if( MapDrawPointX + MapDrawWidth > MapWidth ) 
		MapDrawWidth -= ( MapDrawPointX + MapDrawWidth ) - MapWidth ;

	if( MapDrawPointY + MapDrawHeight > MapHeight ) 
		MapDrawHeight -= ( MapDrawPointY + MapDrawHeight ) - MapHeight ; 

	if( MapDrawPointX < 0 )
	{
		ScreenX = -MapDrawPointX * Width ;
		MapDrawWidth += MapDrawPointX ; 
		MapDrawPointX = 0 ;
	}
	if( MapDrawPointY < 0 )
	{
		ScreenY = -MapDrawPointY * Height ;
		MapDrawHeight += MapDrawPointY ; 
		MapDrawPointY = 0 ;
	}

	// マップを描く
	Result = NS_DrawChipMap( ScreenX, ScreenY, MapDrawWidth, MapDrawHeight,
							 MapData + MapDrawPointX + MapDrawPointY * MapWidth, MapWidth, ChipTypeNum, ChipGrHandle, TransFlag ) ;
						 
	return Result ;
}

// グラフィックを指定領域にタイル状に描画する
#define SETDRAWRECTCODE\
	SETRECT( DrawRect, x1, y1, x2, y2 )\
	DRAWRECT_DRAWAREA_CLIP
extern	int NS_DrawTile( int x1, int y1, int x2, int y2, int Tx, int Ty, double ExtRate, double Angle, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret = -1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	if( x1 == x2 || y1 == y2 ) return 0 ;

	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;
	}

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawTile( x1, y1, x2, y2, Tx, Ty, ExtRate, Angle, Image, BlendImage, TransFlag ),
		Graphics_Software_DrawTile( x1, y1, x2, y2, Tx, Ty, ExtRate, Angle, Image,             TransFlag ),
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}
#undef SETDRAWRECTCODE

// グラフィックの指定矩形部分のみを描画
extern	int NS_DrawRectGraph( int DestX, int DestY, int SrcX, int SrcY, int Width, int Height, int GraphHandle, int TransFlag, int TurnFlag )
{
	RECT  BackupDrawRect ;
	RECTF BackupDrawRectF ;
	int hr ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 指定矩形だけしか描画出来ないようにする
	BackupDrawRect  = GSYS.DrawSetting.DrawArea ;
	BackupDrawRectF = GSYS.DrawSetting.DrawAreaF ;
	SETRECT( GSYS.DrawSetting.DrawArea, DestX, DestY, DestX + Width, DestY + Height ) ;
	RectClipping( &GSYS.DrawSetting.DrawArea, &BackupDrawRect ) ;
	SetMemImgDrawArea( &GSYS.DrawSetting.DrawArea ) ;
	GSYS.DrawSetting.DrawAreaF.left   = ( float )GSYS.DrawSetting.DrawArea.left ;
	GSYS.DrawSetting.DrawAreaF.top    = ( float )GSYS.DrawSetting.DrawArea.top ;
	GSYS.DrawSetting.DrawAreaF.right  = ( float )GSYS.DrawSetting.DrawArea.right ;
	GSYS.DrawSetting.DrawAreaF.bottom = ( float )GSYS.DrawSetting.DrawArea.bottom ;

	// グラフィックの描画
	if( TurnFlag )
	{
		IMAGEDATA *Image ;

		// エラー判定
		if( GRAPHCHK( GraphHandle, Image ) )
			return -1 ;

		hr = NS_DrawTurnGraph( DestX - ( Image->Width - SrcX - Width ), DestY - SrcY, GraphHandle, TransFlag ) ;
	}
	else
	{
		hr = NS_DrawGraph( DestX - SrcX, DestY - SrcY, GraphHandle, TransFlag ) ;
	}

	// 描画矩形を元に戻す
#ifndef DX_NON_MASK
	if( MASKD.MaskValidFlag )
	{
		// マスクを使用している場合はマスク処理内部で色々変更が加えられているので通常の SetDrawArea を使用する
		NS_SetDrawArea( BackupDrawRect.left, BackupDrawRect.top, BackupDrawRect.right, BackupDrawRect.bottom ) ;
	}
	else 
#endif
	{
		// マスクを使用していない場合は簡易的な設定復帰処理
		GSYS.DrawSetting.DrawArea = BackupDrawRect ;
		SetMemImgDrawArea( &BackupDrawRect ) ;
		GSYS.DrawSetting.DrawAreaF = BackupDrawRectF ;
	}

	// 終了
	return hr ;
}

// グラフィックの指定矩形部分のみを拡大描画
extern	int NS_DrawRectExtendGraph( int DestX1, int DestY1, int DestX2, int DestY2, int SrcX, int SrcY, int SrcWidth, int SrcHeight, int GraphHandle, int TransFlag )
{
	RECT MotoDrawRect ;
	RECTF MotoDrawRectF ;
	int hr, tmp ;
	int x1, y1, x2, y2 ;	
	bool RevX, RevY ;
	int DestWidth, DestHeight ;
	double exX, exY ;
	IMAGEDATA *Image ;

	// サイズ判定
	if( SrcWidth <= 0 || SrcHeight <= 0 )
		return -1 ;
	
	// エラー判定
	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	// 反転判定
	{
		if( DestX2 < DestX1 )
		{
			RevX = true ;
			tmp = DestX1 ; DestX1 = DestX2 ; DestX2 = tmp ;
		}
		else
		{
			RevX = false ;
		}
		
		if( DestY2 < DestY1 )
		{
			RevY = true ;
			tmp = DestY1 ; DestY1 = DestY2 ; DestY2 = tmp ;
		}
		else
		{
			RevY = false ;
		}
	}

	// 指定矩形だけしか描画出来ないようにする
	MotoDrawRect = GSYS.DrawSetting.DrawArea ;
	MotoDrawRectF = GSYS.DrawSetting.DrawAreaF ;
	SETRECT( GSYS.DrawSetting.DrawArea, DestX1, DestY1, DestX2, DestY2 ) ;
	RectClipping( &GSYS.DrawSetting.DrawArea, &MotoDrawRect ) ;
	SetMemImgDrawArea( &GSYS.DrawSetting.DrawArea ) ;
	GSYS.DrawSetting.DrawAreaF.left   = (float)GSYS.DrawSetting.DrawArea.left ;
	GSYS.DrawSetting.DrawAreaF.top    = (float)GSYS.DrawSetting.DrawArea.top ;
	GSYS.DrawSetting.DrawAreaF.right  = (float)GSYS.DrawSetting.DrawArea.right ;
	GSYS.DrawSetting.DrawAreaF.bottom = (float)GSYS.DrawSetting.DrawArea.bottom ;

	// グラフィックの描画
	{
		DestWidth = DestX2 - DestX1 ;
		DestHeight = DestY2 - DestY1 ;
		exX = (double)DestWidth / SrcWidth ;
		exY = (double)DestHeight / SrcHeight ;
		if( RevX == true )
		{
			x1 = DestX2 + _DTOL( SrcX * exX ) ;
			x2 = DestX2 - _DTOL( ( Image->Width - SrcX ) * exX ) ;
		}
		else
		{
			x1 = DestX1 - _DTOL( SrcX * exX ) ;
			x2 = DestX1 + _DTOL( ( Image->Width - SrcX ) * exX ) ;
		}

		if( RevY == true )
		{
			y1 = DestY2 + _DTOL( SrcY * exY ) ;
			y2 = DestY2 - _DTOL( ( Image->Height - SrcY ) * exY ) ;
		}
		else
		{
			y1 = DestY1 - _DTOL( SrcY * exY ) ;
			y2 = DestY1 + _DTOL( ( Image->Height - SrcY ) * exY ) ;
		}
		hr = NS_DrawExtendGraph( x1, y1, x2, y2, GraphHandle, TransFlag ) ;
	}

	// 描画矩形を元に戻す
	GSYS.DrawSetting.DrawArea = MotoDrawRect ;
	SetMemImgDrawArea( &MotoDrawRect ) ;
	GSYS.DrawSetting.DrawAreaF = MotoDrawRectF ;

	// 終了
	return hr ;
}

extern	int NS_DrawRectRotaGraph( int X, int Y, int SrcX, int SrcY, int Width, int Height, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag ) 
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 回転描画
	NS_DrawRotaGraph( X, Y, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

extern int NS_DrawRectRotaGraph2( int x, int y, int SrcX, int SrcY, int Width, int Height, int cx, int cy, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 回転描画
	NS_DrawRotaGraph2( x, y, cx, cy, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

extern int NS_DrawRectRotaGraph3(  int x,   int y,   int SrcX, int SrcY, int Width, int Height, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 回転描画
	NS_DrawRotaGraph3( x, y, cx, cy, ExtRateX, ExtRateY, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

// 画像の指定矩形部分のみを自由変形描画
extern int NS_DrawRectModiGraph( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int SrcX, int SrcY, int Width, int Height, int GraphHandle, int TransFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 自由変形描画
	NS_DrawModiGraph( x1, y1, x2, y2, x3, y3, x4, y4, TempHandle, TransFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

// グラフィックの指定矩形部分のみを描画
extern int NS_DrawRectGraphF( float DestX, float DestY, int SrcX, int SrcY, int Width, int Height, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 描画
	if( TurnFlag )
	{
		NS_DrawTurnGraphF( DestX, DestY, TempHandle, TransFlag ) ;
	}
	else
	{
		NS_DrawGraphF( DestX, DestY, TempHandle, TransFlag ) ;
	}

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

// グラフィックの指定矩形部分のみを拡大描画
extern int NS_DrawRectExtendGraphF( float DestX1, float DestY1, float DestX2, float DestY2, int SrcX, int SrcY, int SrcWidth, int SrcHeight, int GraphHandle, int TransFlag )
{
	int TempHandle ;

	// サイズ判定
	if( SrcWidth <= 0 || SrcHeight <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, SrcWidth, SrcHeight, GraphHandle ) ;

	// 拡大描画
	NS_DrawExtendGraphF( DestX1, DestY1, DestX2, DestY2, TempHandle, TransFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

extern int NS_DrawRectRotaGraphF( float X, float Y, int SrcX, int SrcY, int Width, int Height, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 回転描画
	NS_DrawRotaGraphF( X, Y, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

extern int NS_DrawRectRotaGraph2F( float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, double ExtRate, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 回転描画
	NS_DrawRotaGraph2F( x, y, cxf, cyf, ExtRate, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

extern int NS_DrawRectRotaGraph3F( float x, float y, int SrcX, int SrcY, int Width, int Height, float cxf, float cyf, double ExtRateX, double ExtRateY, double Angle, int GraphHandle, int TransFlag, int TurnFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 回転描画
	NS_DrawRotaGraph3F( x, y, cxf, cyf, ExtRateX, ExtRateY, Angle, TempHandle, TransFlag, TurnFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

// 画像の指定矩形部分のみを自由変形描画( 座標指定が float 版 )
extern int NS_DrawRectModiGraphF( float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int SrcX, int SrcY, int Width, int Height,         int GraphHandle, int TransFlag )
{
	int TempHandle ;

	// サイズ判定
	if( Width <= 0 || Height <= 0 )
		return -1 ;

	// 切り取ったグラフィックを作成
	TempHandle = NS_DerivationGraph( SrcX, SrcY, Width, Height, GraphHandle ) ;

	// 自由変形描画
	NS_DrawModiGraphF( x1, y1, x2, y2, x3, y3, x4, y4, TempHandle, TransFlag ) ;

	// 削除
	NS_DeleteGraph( TempHandle ) ;

	// 終了
	return 0 ;
}

// ブレンド画像と合成して画像を描画する
extern	int NS_DrawBlendGraph( int x, int y, int GrHandle, int TransFlag, int BlendGraph, int BorderParam, int BorderRange )
{
	int B_BlendGraph, B_BorderParam, B_BorderRange ;

	B_BlendGraph = GSYS.DrawSetting.BlendGraph ;
	B_BorderParam = GSYS.DrawSetting.BlendGraphBorderParam ;
	B_BorderRange = GSYS.DrawSetting.BlendGraphBorderRange ;

	NS_SetBlendGraph( BlendGraph, BorderParam, BorderRange ) ;
	NS_DrawGraph( x, y, GrHandle, TransFlag ) ;
	NS_SetBlendGraph( B_BlendGraph, B_BorderParam, B_BorderRange ) ;

	// 終了
	return 0 ;
}

// ブレンド画像と合成して画像を描画する( ブレンド画像の起点座標を指定する版 )
extern	int NS_DrawBlendGraphPos( int x, int y, int GrHandle, int TransFlag, int bx, int by, int BlendGraph, int BorderParam, int BorderRange )
{
	int B_BlendGraph, B_BorderParam, B_BorderRange, B_PosX, B_PosY ;

	B_BlendGraph = GSYS.DrawSetting.BlendGraph ;
	B_BorderParam = GSYS.DrawSetting.BlendGraphBorderParam ;
	B_BorderRange = GSYS.DrawSetting.BlendGraphBorderRange ;
	B_PosX = GSYS.DrawSetting.BlendGraphX ;
	B_PosY = GSYS.DrawSetting.BlendGraphY ;

	NS_SetBlendGraph( BlendGraph, BorderParam, BorderRange ) ;
	NS_SetBlendGraphPosition( bx, by ) ;
	NS_DrawGraph( x, y, GrHandle, TransFlag ) ;
	NS_SetBlendGraphPosition( B_PosX, B_PosY ) ;
	NS_SetBlendGraph( B_BlendGraph, B_BorderParam, B_BorderRange ) ;

	// 終了
	return 0 ;
}

// DrawCircleGauge 関数の補助関数
void DrawCircleGaugePolygon(
	int GraphHandle,
	float CenterX,
	float CenterY,
	VECTOR *pVector,
	float SizeR,
	float UScale,
	float VScale,
	float UStart,
	float VStart
)
{
	int i ;
	VERTEX vert[ 3 ] ;
	float AdjustPosition ;

	AdjustPosition = GSYS.HardInfo.DrawFloatCoordType == DX_DRAWFLOATCOORDTYPE_DIRECT3D9 ? 0.5f : 0.0f ;
	
	for( i = 0; i < 3; i ++ )
	{
		vert[i].x = pVector[i].x + CenterX + AdjustPosition ;
		vert[i].y = pVector[i].y + CenterY + AdjustPosition ;
		vert[i].u = ( pVector[i].x * SizeR + 0.5f ) * UScale + UStart ;
		vert[i].v = ( pVector[i].y * SizeR + 0.5f ) * VScale + VStart ;
		vert[i].b = 255 ;
		vert[i].g = 255 ;
		vert[i].r = 255 ;
		vert[i].a = 255 ;
	}
	NS_DrawPolygon( vert, 1, GraphHandle, TRUE );
}

// 円グラフ的な描画を行う
int NS_DrawCircleGauge( int CenterX, int CenterY, double Percent, int GrHandle, double StartPercent )
{
	VECTOR vertex[3] ;
	VECTOR position[4] ;
	VECTOR positionR ;
	VECTOR positionR2 ;
	float radian ;
	float radian2 ;
	float length ;
	float Sin ;
	float Cos ;
	float CenterXF ;
	float CenterYF ;
	float SizeR ;
	int SizeX ;
	int SizeY ;
	int i ;
	float StartRad ;
	float EndRad ;
	IMAGEDATA *Image ;
	float UScale ;
	float VScale ;
	float UStart ;
	float VStart ;
	RECT DrawRect ;
	RECT GaugeDrawRect ;

	Image = Graphics_Image_GetData( GrHandle ) ;
	if( Image == NULL )
	{
		return -1 ;
	}

	if( Image->Orig->FormatDesc.TextureFlag )
	{
		UStart = ( float )Image->Hard.Draw[ 0 ].UsePosX / Image->Hard.Draw[ 0 ].Tex->TexWidth ;
		VStart = ( float )Image->Hard.Draw[ 0 ].UsePosY / Image->Hard.Draw[ 0 ].Tex->TexHeight ;
		UScale = ( float )Image->Width                  / Image->Hard.Draw[ 0 ].Tex->TexWidth ;
		VScale = ( float )Image->Height                 / Image->Hard.Draw[ 0 ].Tex->TexHeight ;
	}
	else
	{
		UScale = 1.0f ;
		VScale = 1.0f ;
		UStart = 0.0f ;
		VStart = 0.0f ;
	}

	if( StartPercent > 200.0 )
	{
		StartPercent = 200.0 ;
	}
	else
	if( StartPercent < -100.0 )
	{
		StartPercent = -100.0 ;
	}

	if( Percent > 200.0 )
	{
		Percent = 200.0 ;
	}
	else
	if( Percent < -100.0 )
	{
		Percent = -100.0 ;
	}

	if( Percent - StartPercent > 100.0 )
	{
		StartPercent = 0.0 ;
		Percent = 100.0 ;
	}

	if( StartPercent == Percent ||
		StartPercent > Percent )
	{
		return 0 ;
	}

	NS_GetGraphSize( GrHandle, &SizeX, &SizeY );

	radian   = ( float )( Percent      * 2 * DX_PI / 100.0 );
	radian2  = ( float )( StartPercent * 2 * DX_PI / 100.0 );
	length   = SizeX * 0.75f;
	CenterXF = ( float )CenterX;
	CenterYF = ( float )CenterY;
	SizeR    = 1.0f / SizeX;

	vertex[0].x = 0.0f;
	vertex[0].y = 0.0f;
	vertex[1] = vertex[0];
	vertex[2] = vertex[0];

	position[0].x = 0.0f;
	position[0].y = -length;

	position[1].x = length;
	position[1].y = 0.0f;

	position[2].x = 0.0f;
	position[2].y = length;

	position[3].x = -length;
	position[3].y = 0.0f;

	_SINCOS( radian2 - DX_PI_F * 0.5f, &Sin, &Cos ) ;
	positionR2.x = Cos * length;
	positionR2.y = Sin * length;

	_SINCOS( radian - DX_PI_F * 0.5f, &Sin, &Cos ) ;
	positionR.x = Cos * length;
	positionR.y = Sin * length;

	DrawRect = GSYS.DrawSetting.DrawArea ;
	GaugeDrawRect.left   = CenterX - SizeX / 2 ;
	GaugeDrawRect.top    = CenterY - SizeY / 2 ;
	GaugeDrawRect.right  = CenterX + SizeX / 2 ;
	GaugeDrawRect.bottom = CenterY + SizeY / 2 ;
	RectClipping( &GaugeDrawRect, &DrawRect ) ;
	NS_SetDrawArea( GaugeDrawRect.left, GaugeDrawRect.top, GaugeDrawRect.right, GaugeDrawRect.bottom ) ; 

	// 90度分筒描画
	for( i = 0 ; i < 12 ; i ++ )
	{
		StartRad = DX_PI_F * 0.5f * ( i - 4 ) ;
		EndRad   = DX_PI_F * 0.5f * ( i - 4 + 1 ) ;

		if( ( radian2 >= StartRad && radian2 <= EndRad ) || ( StartRad >= radian2 && StartRad <= radian ) )
		{
			vertex[1] = radian2 <= StartRad ? position[ i         % 4 ] : positionR2;
			vertex[2] = radian  >= EndRad   ? position[ ( i + 1 ) % 4 ] : positionR;
			DrawCircleGaugePolygon( GrHandle, CenterXF, CenterYF, vertex, SizeR, UScale, VScale, UStart, VStart );
		}
	}

	// 描画可能範囲を元に戻す
	NS_SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ;

	// 終了
	return 0;
}

static struct
{
	float DrawZ;
	int BlendMode, BlendParam;
	int AlphaTestMode, AlphaTestParam;
	int ZBufferCmpType2D, WriteZBufferFlag2D;
	int ZBufferCmpType3D, WriteZBufferFlag3D;
} DrawZBuffer_Static;

static void FASTCALL DrawZBuffer_Pre( int WriteZMode )
{
	DrawZBuffer_Static.DrawZ              = GSYS.DrawSetting.DrawZ;
	DrawZBuffer_Static.BlendMode          = GSYS.DrawSetting.BlendMode;
	DrawZBuffer_Static.BlendParam         = GSYS.DrawSetting.BlendParam;
	DrawZBuffer_Static.AlphaTestMode      = GSYS.DrawSetting.AlphaTestMode;
	DrawZBuffer_Static.AlphaTestParam     = GSYS.DrawSetting.AlphaTestParam;
	DrawZBuffer_Static.WriteZBufferFlag2D = GSYS.DrawSetting.WriteZBufferFlag2D;
	DrawZBuffer_Static.WriteZBufferFlag3D = GSYS.DrawSetting.WriteZBufferFlag3D;
	DrawZBuffer_Static.ZBufferCmpType2D   = GSYS.DrawSetting.ZBufferCmpType2D;
	DrawZBuffer_Static.ZBufferCmpType3D   = GSYS.DrawSetting.ZBufferCmpType3D;

	NS_SetDrawZ( WriteZMode == DX_ZWRITE_MASK ? 0.0f : 0.9f );
	NS_SetDrawBlendMode( DX_BLENDMODE_DESTCOLOR, 255 );
	NS_SetDrawAlphaTest( DX_CMP_GREATER, 0 );
	NS_SetWriteZBufferFlag( TRUE );
	NS_SetZBufferCmpType( DX_CMP_ALWAYS );
}

static void FASTCALL DrawZBuffer_Post( void )
{
	NS_SetZBufferCmpType( DrawZBuffer_Static.ZBufferCmpType2D );
	NS_SetWriteZBufferFlag( DrawZBuffer_Static.WriteZBufferFlag2D );
	NS_SetDrawAlphaTest( DrawZBuffer_Static.AlphaTestMode, DrawZBuffer_Static.AlphaTestParam );
	NS_SetDrawBlendMode( DrawZBuffer_Static.BlendMode, DrawZBuffer_Static.BlendParam );
	NS_SetDrawZ( DrawZBuffer_Static.DrawZ );

	NS_SetZBufferCmpType3D( DrawZBuffer_Static.ZBufferCmpType3D );
	NS_SetWriteZBuffer3D( DrawZBuffer_Static.WriteZBufferFlag3D );
}

// Ｚバッファに対して画像の描画を行う
extern	int NS_DrawGraphToZBuffer( int X, int Y, int GrHandle, int WriteZMode )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawGraph( X, Y, GrHandle, TRUE );
	DrawZBuffer_Post();

	// 終了
	return 0 ;
}

// Ｚバッファに対して画像の左右反転描画
extern	int NS_DrawTurnGraphToZBuffer( int x, int y, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawTurnGraph( x, y, GrHandle, TRUE );
	DrawZBuffer_Post();

	// 終了
	return 0 ;
}

// Ｚバッファに対して画像の拡大縮小描画
extern	int NS_DrawExtendGraphToZBuffer( int x1, int y1, int x2, int y2, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendGraph( x1, y1, x2, y2, GrHandle, TRUE );
	DrawZBuffer_Post();

	// 終了
	return 0 ;
}

// Ｚバッファに対して画像の回転描画
extern	int NS_DrawRotaGraphToZBuffer( int x, int y, double ExRate, double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */, int TurnFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaGraph(  x,  y,  ExRate,  Angle,  GrHandle,  TRUE, TurnFlag );
	DrawZBuffer_Post();

	// 終了
	return 0 ;
}

// Ｚバッファに対して画像の回転描画２
extern	int NS_DrawRotaGraph2ToZBuffer(  int x, int y, int cx, int cy, double ExtRate,                   double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int TurnFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaGraph2( x,  y,  cx,  cy,  ExtRate,  Angle,  GrHandle,  TRUE, TurnFlag ) ;
	DrawZBuffer_Post();

	// 終了
	return 0 ;
}

// Ｚバッファに対して画像の回転描画３
extern	int NS_DrawRotaGraph3ToZBuffer(  int x, int y, int cx, int cy, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int TurnFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaGraph3(   x,  y,  cx,  cy,  ExtRateX,  ExtRateY,  Angle,  GrHandle, TRUE, TurnFlag );
	DrawZBuffer_Post();

	// 終了
	return 0 ;
}

// Ｚバッファに対して画像の自由変形描画
extern	int NS_DrawModiGraphToZBuffer( int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int GrHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawModiGraph(  x1,  y1,  x2,  y2,  x3,  y3,  x4,  y4,  GrHandle,  TRUE );
	DrawZBuffer_Post();

	// 終了
	return 0 ;
}

// Ｚバッファに対して矩形の描画を行う
extern int NS_DrawBoxToZBuffer( int x1, int y1, int x2, int y2, int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawBox( x1, y1, x2, y2, NS_GetColor(255,255,255), FillFlag );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して円の描画を行う
extern int NS_DrawCircleToZBuffer( int x, int y, int r, int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawCircle( x, y, r, GetColor(255,255,255), FillFlag );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して三角形を描画する
extern int NS_DrawTriangleToZBuffer( int x1, int y1, int x2, int y2, int x3, int y3, int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawTriangle( x1, y1, x2, y2, x3, y3, GetColor(255,255,255), FillFlag ) ;
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して四角形を描画する
extern int NS_DrawQuadrangleToZBuffer(  int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawQuadrangle( x1, y1, x2, y2, x3, y3, x4, y4, GetColor(255,255,255), FillFlag ) ;
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して角の丸い四角形を描画する
extern int NS_DrawRoundRectToZBuffer(   int x1, int y1, int x2, int y2, int rx, int ry, int FillFlag, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRoundRect( x1, y1, x2, y2, rx, ry, GetColor(255,255,255), FillFlag ) ;
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// ２Ｄポリゴンを描画する
extern	int NS_DrawPolygon( const VERTEX *Vertex, int PolygonNum, int GrHandle, int TransFlag, int UVScaling )
{
	return NS_DrawPolygonBase( Vertex, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag,  UVScaling ) ;
}

// ２Ｄポリゴンを描画する
extern int NS_DrawPolygon2D( const VERTEX2D *Vertex, int PolygonNum, int GrHandle, int TransFlag )
{
	return NS_DrawPrimitive2D( Vertex, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag ) ;
}

// ３Ｄポリゴンを描画する
extern	int NS_DrawPolygon3D( const VERTEX_3D *Vertex, int PolygonNum, int GrHandle, int TransFlag )
{
	return NS_DrawPolygon3DBase( Vertex, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag ) ;
}

// ２Ｄポリゴンを描画する( 頂点インデックスを使用 )
extern int NS_DrawPolygonIndexed2D( const VERTEX2D  *Vertex, int VertexNum, const unsigned short *Indices, int PolygonNum, int GrHandle, int TransFlag )
{
	return NS_DrawPrimitiveIndexed2D( Vertex, VertexNum, Indices, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag ) ;
}

// ３Ｄポリゴンを描画する(インデックス)
extern int NS_DrawPolygonIndexed3D( const VERTEX3D *Vertex, int VertexNum, const unsigned short *Indices, int PolygonNum, int GrHandle, int TransFlag )
{
	return NS_DrawPrimitiveIndexed3D( Vertex, VertexNum, Indices, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag ) ;
}

// ３Ｄポリゴンを描画する(インデックス)
extern	int NS_DrawPolygonIndexed3DBase( const VERTEX_3D *Vertex, int VertexNum, const unsigned short *Indices, int IndexNum, int PrimitiveType, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret, i, Flag ;
	const VERTEX_3D *UseVertex ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 色の変換が必要な場合はここで処理する
	if( GSYS.DrawSetting.DrawBright.Red   != 255 ||
		GSYS.DrawSetting.DrawBright.Green != 255 ||
		GSYS.DrawSetting.DrawBright.Blue  != 255 ||
		( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND && GSYS.DrawSetting.BlendParam != 255 ) )
	{
		DWORD *RedTable, *GreenTable, *BlueTable, *AlphaTable ;
		const VERTEX_3D *sv ;
		VERTEX_3D *dv ;
		unsigned int Size ;

		// テーブルを用意する
		RedTable   = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Red] ;
		GreenTable = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Green] ;
		BlueTable  = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Blue] ;
		AlphaTable = MemImgManage.RateTable[GSYS.DrawSetting.BlendParam] ;
		if( GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ) AlphaTable = MemImgManage.RateTable[255] ;

		// 頂点データを格納するメモリ領域が足りない場合は確保する
		Size = sizeof( VERTEX_3D ) * VertexNum ;
		if( GSYS.Resource.TempVertexBufferSize < ( int )Size )
		{
			if( GSYS.Resource.TempVertexBuffer )
			{
				DXFREE( GSYS.Resource.TempVertexBuffer ) ;
				GSYS.Resource.TempVertexBufferSize = 0 ;
			}
			GSYS.Resource.TempVertexBuffer = DXALLOC( Size ) ;
			if( GSYS.Resource.TempVertexBuffer == NULL )
				return -1 ;
			GSYS.Resource.TempVertexBufferSize = ( int )Size ;
		}

		// 一時バッファに変換しながら転送
		sv = Vertex;
		dv = ( VERTEX_3D * )GSYS.Resource.TempVertexBuffer ;
		for( i = 0 ; i < VertexNum ; i ++, sv ++, dv ++ )
		{
			dv->pos = sv->pos ;
			dv->u = sv->u ;
			dv->v = sv->v ;
			dv->b = ( BYTE )BlueTable [ sv->b ] ;
			dv->g = ( BYTE )GreenTable[ sv->g ] ;
			dv->r = ( BYTE )RedTable  [ sv->r ] ;
			dv->a = ( BYTE )AlphaTable[ sv->a ] ;
		}
		UseVertex = ( VERTEX_3D * )GSYS.Resource.TempVertexBuffer ;
	}
	else
	{
		UseVertex = Vertex ;
	}
	
	DRAW_DEF(
		Graphics_Hardware_DrawIndexedPrimitive_PF( UseVertex, VertexNum, Indices, IndexNum, PrimitiveType, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// ３Ｄポリゴンを描画する
extern	int NS_DrawPolygon3DBase( const VERTEX_3D *Vertex, int VertexNum, int PrimitiveType, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret, i, Flag ;
	const VERTEX_3D *UseVertex ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 色の変換が必要な場合はここで処理する
	if( GSYS.DrawSetting.DrawBright.Red   != 255 ||
		GSYS.DrawSetting.DrawBright.Green != 255 ||
		GSYS.DrawSetting.DrawBright.Blue  != 255 ||
		( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND && GSYS.DrawSetting.BlendParam != 255 ) )
	{
		DWORD *RedTable, *GreenTable, *BlueTable, *AlphaTable ;
		const VERTEX_3D *sv ;
		VERTEX_3D *dv ;
		unsigned int Size ;

		// テーブルを用意する
		RedTable   = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Red] ;
		GreenTable = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Green] ;
		BlueTable  = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Blue] ;
		AlphaTable = MemImgManage.RateTable[GSYS.DrawSetting.BlendParam] ;
		if( GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ) AlphaTable = MemImgManage.RateTable[255] ;

		// 頂点データを格納するメモリ領域が足りない場合は確保する
		Size = sizeof( VERTEX_3D ) * VertexNum ;
		if( GSYS.Resource.TempVertexBufferSize < ( int )Size )
		{
			if( GSYS.Resource.TempVertexBuffer )
			{
				DXFREE( GSYS.Resource.TempVertexBuffer ) ;
				GSYS.Resource.TempVertexBufferSize = 0 ;
			}
			GSYS.Resource.TempVertexBuffer = DXALLOC( Size ) ;
			if( GSYS.Resource.TempVertexBuffer == NULL )
				return -1 ;
			GSYS.Resource.TempVertexBufferSize = ( int )Size ;
		}

		// 一時バッファに変換しながら転送
		sv = Vertex;
		dv = ( VERTEX_3D * )GSYS.Resource.TempVertexBuffer ;
		for( i = 0 ; i < VertexNum ; i ++, sv ++, dv ++ )
		{
			dv->pos = sv->pos ;
			dv->u = sv->u ;
			dv->v = sv->v ;
			dv->b = ( BYTE )BlueTable [ sv->b ] ;
			dv->g = ( BYTE )GreenTable[ sv->g ] ;
			dv->r = ( BYTE )RedTable  [ sv->r ] ;
			dv->a = ( BYTE )AlphaTable[ sv->a ] ;
		}
		UseVertex = ( VERTEX_3D * )GSYS.Resource.TempVertexBuffer ;
	}
	else
	{
		UseVertex = Vertex ;
	}
	
	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive_PF( UseVertex, VertexNum, PrimitiveType, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// ３Ｄポリゴンを描画する
extern int NS_DrawPolygon3D( const VERTEX3D *Vertex, int PolygonNum, int GrHandle, int TransFlag )
{
	return NS_DrawPrimitive3D( Vertex, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag ) ;
}

// ２Ｄポリゴンを描画する
#define SETDRAWRECTCODE\
	float MinX, MinY, MaxX, MaxY ;\
	const VERTEX *Vert ;\
\
	MinX =  100000000.0F ;\
	MinY =  100000000.0F ;\
	MaxX = -100000000.0F ;\
	MaxY = -100000000.0F ;\
	Vert = Vertex ;\
	for( i = 0 ; i < VertexNum ; i ++, Vert ++ )\
	{\
		if( Vert->x - 1.0F < MinX ) MinX = Vert->x - 1.0F ;\
		if( Vert->x + 1.0F > MaxX ) MaxX = Vert->x + 1.0F ;\
\
		if( Vert->y - 1.0F < MinY ) MinY = Vert->y - 1.0F ;\
		if( Vert->y + 1.0F > MaxY ) MaxY = Vert->y + 1.0F ;\
	}\
\
	DrawRect.left   = _FTOL( MinX ) ;\
	DrawRect.top    = _FTOL( MinY ) ;\
	DrawRect.right  = _FTOL( MaxX ) ;\
	DrawRect.bottom = _FTOL( MaxY ) ;\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawPolygonBase( const VERTEX *Vertex, int VertexNum, int PrimitiveType, int GrHandle, int TransFlag, int UVScaling )
{
	IMAGEDATA *Image ;
	VERTEX_2D *TransVert = NULL ;
	VERTEX_2D *DestP ;
	int Ret, i, Flag ;
	float UPos = 0.0f, VPos = 0.0f, UScale = 1.0f, VScale = 1.0f ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;

		Flag = Image->Orig->FormatDesc.TextureFlag ;

		// 画像のサイズが最初から２のｎ乗だったらＵＶ補正はしない
		if( UVScaling )
		{
			if( Image->Orig->FormatDesc.TextureFlag == FALSE ||
				( Image->Hard.DrawNum == 1 && 
				  Image->Hard.Draw->Tex->TexWidth  == Image->Hard.Draw->Width &&
				  Image->Hard.Draw->Tex->TexHeight == Image->Hard.Draw->Height ) )
			{
				UVScaling = FALSE ;
			}
		}
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
		UVScaling = FALSE ;
	}

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// UVスケーリングを行う場合はここでデータを算出する
	if( UVScaling )
	{
		UPos = ( float )Image->Hard.Draw->UsePosX  / Image->Hard.Draw->Tex->TexWidth ;
		VPos = ( float )Image->Hard.Draw->UsePosY  / Image->Hard.Draw->Tex->TexHeight ;
		UScale = ( float )Image->Hard.Draw->Width  / Image->Hard.Draw->Tex->TexWidth ;
		VScale = ( float )Image->Hard.Draw->Height / Image->Hard.Draw->Tex->TexHeight ;
	}

	// 頂点の出力先を確保する
	TransVert = ( VERTEX_2D * )DXALLOC( sizeof( VERTEX_2D ) * VertexNum ) ;
	if( TransVert == NULL ) return -1 ;

	// 頂点変換を行う
	if( GSYS.DrawSetting.DrawBright.Red != 255 || GSYS.DrawSetting.DrawBright.Green != 255 || GSYS.DrawSetting.DrawBright.Blue != 255 ||
		( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND && GSYS.DrawSetting.BlendParam != 255 ) )
	{
		DWORD *RedTable, *GreenTable, *BlueTable, *AlphaTable ;

		// テーブルを用意する
		RedTable   = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Red] ;
		GreenTable = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Green] ;
		BlueTable  = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Blue] ;
		AlphaTable = MemImgManage.RateTable[GSYS.DrawSetting.BlendParam] ;
		if( GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ) AlphaTable = MemImgManage.RateTable[255] ;

		// 頂点変換する
		if( UVScaling )
		{
			DestP = TransVert ;
			for( i = 0 ; i < VertexNum ; i ++, DestP ++, Vertex ++ )
			{
				DestP->pos.x = Vertex->x ;
				DestP->pos.y = Vertex->y ;
				DestP->pos.z = GSYS.DrawSetting.DrawZ ;
				DestP->rhw = 1.0F ;
				((BYTE *)&DestP->color)[0] = (BYTE)BlueTable [Vertex->b] ;
				((BYTE *)&DestP->color)[1] = (BYTE)GreenTable[Vertex->g] ;
				((BYTE *)&DestP->color)[2] = (BYTE)RedTable  [Vertex->r] ;
				((BYTE *)&DestP->color)[3] = (BYTE)AlphaTable[Vertex->a] ;
				DestP->u = Vertex->u * UScale + UPos ;
				DestP->v = Vertex->v * VScale + VPos ;
			}
		}
		else
		{
			DestP = TransVert ;
			for( i = 0 ; i < VertexNum ; i ++, DestP ++, Vertex ++ )
			{
				DestP->pos.x = Vertex->x ;
				DestP->pos.y = Vertex->y ;
				DestP->pos.z = GSYS.DrawSetting.DrawZ ;
				DestP->rhw = 1.0F ;
				((BYTE *)&DestP->color)[0] = (BYTE)BlueTable [Vertex->b] ;
				((BYTE *)&DestP->color)[1] = (BYTE)GreenTable[Vertex->g] ;
				((BYTE *)&DestP->color)[2] = (BYTE)RedTable  [Vertex->r] ;
				((BYTE *)&DestP->color)[3] = (BYTE)AlphaTable[Vertex->a] ;
				DestP->u = Vertex->u ;
				DestP->v = Vertex->v ;
			}
		}
	}
	else
	{
		// 頂点変換する
		if( UVScaling )
		{
			DestP = TransVert ;
			for( i = 0 ; i < VertexNum ; i ++, DestP ++, Vertex ++ )
			{
				DestP->pos.x = Vertex->x ;
				DestP->pos.y = Vertex->y ;
				DestP->pos.z = GSYS.DrawSetting.DrawZ ;
				DestP->rhw = 1.0F ;
				DestP->color = *( ( DWORD * )&Vertex->b ) ;
				DestP->u = Vertex->u * UScale + UPos ;
				DestP->v = Vertex->v * VScale + VPos ;
			}
		}
		else
		{
			DestP = TransVert ;
			for( i = 0 ; i < VertexNum ; i ++, DestP ++, Vertex ++ )
			{
				DestP->pos.x = Vertex->x ;
				DestP->pos.y = Vertex->y ;
				DestP->pos.z = GSYS.DrawSetting.DrawZ ;
				DestP->rhw = 1.0F ;
				DestP->color = *( ( DWORD * )&Vertex->b ) ;
				DestP->u = Vertex->u ;
				DestP->v = Vertex->v ;
			}
		}
	}

	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive2D_PF( TransVert, VertexNum, PrimitiveType, Image, TransFlag, FALSE, FALSE, FALSE, 0 ),
		0,
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	if( TransVert != NULL )
	{
		DXFREE( TransVert ) ;
		TransVert = NULL ;
	}

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// ２Ｄプリミティブを描画する
extern int NS_DrawPrimitive2D( const VERTEX2D *Vertex, int VertexNum, int PrimitiveType, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret, i, Flag ;
	const VERTEX2D *UseVertex ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 色の変換が必要な場合はここで処理する
	if( GSYS.DrawSetting.DrawBright.Red   != 255 ||
		GSYS.DrawSetting.DrawBright.Green != 255 ||
		GSYS.DrawSetting.DrawBright.Blue  != 255 ||
		( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND && GSYS.DrawSetting.BlendParam != 255 ) )
	{
		DWORD *RedTable, *GreenTable, *BlueTable, *AlphaTable ;
		const VERTEX2D *sv ;
		VERTEX2D *dv ;
		unsigned int Size ;

		// テーブルを用意する
		RedTable   = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Red] ;
		GreenTable = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Green] ;
		BlueTable  = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Blue] ;
		AlphaTable = MemImgManage.RateTable[GSYS.DrawSetting.BlendParam] ;
		if( GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ) AlphaTable = MemImgManage.RateTable[255] ;

		// 頂点データを格納するメモリ領域が足りない場合は確保する
		Size = sizeof( VERTEX2D ) * VertexNum ;
		if( GSYS.Resource.TempVertexBufferSize < ( int )Size )
		{
			if( GSYS.Resource.TempVertexBuffer )
			{
				DXFREE( GSYS.Resource.TempVertexBuffer ) ;
				GSYS.Resource.TempVertexBufferSize = 0 ;
			}
			GSYS.Resource.TempVertexBuffer = DXALLOC( Size ) ;
			if( GSYS.Resource.TempVertexBuffer == NULL )
				return -1 ;
			GSYS.Resource.TempVertexBufferSize = ( int )Size ;
		}

		// 一時バッファに変換しながら転送
		sv = Vertex;
		dv = ( VERTEX2D * )GSYS.Resource.TempVertexBuffer ;
		for( i = 0 ; i < VertexNum ; i ++, sv ++, dv ++ )
		{
			dv->pos = sv->pos ;
			dv->rhw = sv->rhw ;
			dv->dif.b = ( BYTE )BlueTable [ sv->dif.b ] ;
			dv->dif.g = ( BYTE )GreenTable[ sv->dif.g ] ;
			dv->dif.r = ( BYTE )RedTable  [ sv->dif.r ] ;
			dv->dif.a = ( BYTE )AlphaTable[ sv->dif.a ] ;
			dv->u = sv->u ;
			dv->v = sv->v ;
		}
		UseVertex = ( VERTEX2D * )GSYS.Resource.TempVertexBuffer ;
	}
	else
	{
		UseVertex = Vertex ;
	}
	
	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive2DUser_PF( UseVertex, VertexNum, PrimitiveType, Image, TransFlag, FALSE, FALSE, 0 ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// ３Ｄポリゴンを描画する
extern int NS_DrawPrimitive3D( const VERTEX3D *Vertex, int VertexNum, int PrimitiveType, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret, i, Flag ;
	const VERTEX3D *UseVertex ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 色の変換が必要な場合はここで処理する
	if( GSYS.DrawSetting.DrawBright.Red   != 255 ||
		GSYS.DrawSetting.DrawBright.Green != 255 ||
		GSYS.DrawSetting.DrawBright.Blue  != 255 ||
		( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND && GSYS.DrawSetting.BlendParam != 255 ) )
	{
		DWORD *RedTable, *GreenTable, *BlueTable, *AlphaTable ;
		const VERTEX3D *sv ;
		VERTEX3D *dv ;
		unsigned int Size ;

		// テーブルを用意する
		RedTable   = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Red] ;
		GreenTable = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Green] ;
		BlueTable  = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Blue] ;
		AlphaTable = MemImgManage.RateTable[GSYS.DrawSetting.BlendParam] ;
		if( GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ) AlphaTable = MemImgManage.RateTable[255] ;

		// 頂点データを格納するメモリ領域が足りない場合は確保する
		Size = sizeof( VERTEX3D ) * VertexNum ;
		if( GSYS.Resource.TempVertexBufferSize < ( int )Size )
		{
			if( GSYS.Resource.TempVertexBuffer )
			{
				DXFREE( GSYS.Resource.TempVertexBuffer ) ;
				GSYS.Resource.TempVertexBufferSize = 0 ;
			}
			GSYS.Resource.TempVertexBuffer = DXALLOC( Size ) ;
			if( GSYS.Resource.TempVertexBuffer == NULL )
				return -1 ;
			GSYS.Resource.TempVertexBufferSize = ( int )Size ;
		}

		// 一時バッファに変換しながら転送
		sv = Vertex;
		dv = ( VERTEX3D * )GSYS.Resource.TempVertexBuffer ;
		for( i = 0 ; i < VertexNum ; i ++, sv ++, dv ++ )
		{
			dv->pos = sv->pos ;
			dv->norm = sv->norm ;
			dv->dif.b = ( BYTE )BlueTable [ sv->dif.b ] ;
			dv->dif.g = ( BYTE )GreenTable[ sv->dif.g ] ;
			dv->dif.r = ( BYTE )RedTable  [ sv->dif.r ] ;
			dv->dif.a = ( BYTE )AlphaTable[ sv->dif.a ] ;
			dv->spc.b = ( BYTE )BlueTable [ sv->spc.b ] ;
			dv->spc.g = ( BYTE )GreenTable[ sv->spc.g ] ;
			dv->spc.r = ( BYTE )RedTable  [ sv->spc.r ] ;
			dv->spc.a = ( BYTE )AlphaTable[ sv->spc.a ] ;
			dv->u = sv->u ;
			dv->v = sv->v ;
			dv->su = sv->su ;
			dv->sv = sv->sv ;
		}
		UseVertex = ( VERTEX3D * )GSYS.Resource.TempVertexBuffer ;
	}
	else
	{
		UseVertex = Vertex ;
	}
	
	DRAW_DEF(
		Graphics_Hardware_DrawPrimitiveLight_PF( UseVertex, VertexNum, PrimitiveType, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// ２Ｄプリミティブを描画する(インデックス)
extern int NS_DrawPrimitiveIndexed2D( const VERTEX2D *Vertex, int VertexNum, const unsigned short *Indices, int IndexNum, int PrimitiveType, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret, i, Flag ;
	const VERTEX2D *UseVertex ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 色の変換が必要な場合はここで処理する
	if( GSYS.DrawSetting.DrawBright.Red   != 255 ||
		GSYS.DrawSetting.DrawBright.Green != 255 ||
		GSYS.DrawSetting.DrawBright.Blue  != 255 ||
		( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND && GSYS.DrawSetting.BlendParam != 255 ) )
	{
		DWORD *RedTable, *GreenTable, *BlueTable, *AlphaTable ;
		const VERTEX2D *sv ;
		VERTEX2D *dv ;
		unsigned int Size ;

		// テーブルを用意する
		RedTable   = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Red] ;
		GreenTable = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Green] ;
		BlueTable  = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Blue] ;
		AlphaTable = MemImgManage.RateTable[GSYS.DrawSetting.BlendParam] ;
		if( GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ) AlphaTable = MemImgManage.RateTable[255] ;

		// 頂点データを格納するメモリ領域が足りない場合は確保する
		Size = sizeof( VERTEX2D ) * VertexNum ;
		if( GSYS.Resource.TempVertexBufferSize < ( int )Size )
		{
			if( GSYS.Resource.TempVertexBuffer )
			{
				DXFREE( GSYS.Resource.TempVertexBuffer ) ;
				GSYS.Resource.TempVertexBufferSize = 0 ;
			}
			GSYS.Resource.TempVertexBuffer = DXALLOC( Size ) ;
			if( GSYS.Resource.TempVertexBuffer == NULL )
				return -1 ;
			GSYS.Resource.TempVertexBufferSize = ( int )Size ;
		}

		// 一時バッファに変換しながら転送
		sv = Vertex;
		dv = ( VERTEX2D * )GSYS.Resource.TempVertexBuffer ;
		for( i = 0 ; i < VertexNum ; i ++, sv ++, dv ++ )
		{
			dv->pos = sv->pos ;
			dv->rhw = sv->rhw ;
			dv->dif.b = ( BYTE )BlueTable [ sv->dif.b ] ;
			dv->dif.g = ( BYTE )GreenTable[ sv->dif.g ] ;
			dv->dif.r = ( BYTE )RedTable  [ sv->dif.r ] ;
			dv->dif.a = ( BYTE )AlphaTable[ sv->dif.a ] ;
			dv->u = sv->u ;
			dv->v = sv->v ;
		}
		UseVertex = ( VERTEX2D * )GSYS.Resource.TempVertexBuffer ;
	}
	else
	{
		UseVertex = Vertex ;
	}
	
	DRAW_DEF(
		Graphics_Hardware_DrawIndexedPrimitive2DUser_PF( UseVertex, VertexNum, Indices, IndexNum, PrimitiveType, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// ３Ｄポリゴンを描画する(インデックス)
extern int NS_DrawPrimitiveIndexed3D( const VERTEX3D *Vertex, int VertexNum, const unsigned short *Indices, int IndexNum, int PrimitiveType, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret, i, Flag ;
	const VERTEX3D *UseVertex ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 色の変換が必要な場合はここで処理する
	if( GSYS.DrawSetting.DrawBright.Red   != 255 ||
		GSYS.DrawSetting.DrawBright.Green != 255 ||
		GSYS.DrawSetting.DrawBright.Blue  != 255 ||
		( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND && GSYS.DrawSetting.BlendParam != 255 ) )
	{
		DWORD *RedTable, *GreenTable, *BlueTable, *AlphaTable ;
		const VERTEX3D *sv ;
		VERTEX3D *dv ;
		unsigned int Size ;

		// テーブルを用意する
		RedTable   = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Red] ;
		GreenTable = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Green] ;
		BlueTable  = MemImgManage.RateTable[GSYS.DrawSetting.DrawBright.Blue] ;
		AlphaTable = MemImgManage.RateTable[GSYS.DrawSetting.BlendParam] ;
		if( GSYS.DrawSetting.BlendMode == DX_BLENDMODE_NOBLEND ) AlphaTable = MemImgManage.RateTable[255] ;

		// 頂点データを格納するメモリ領域が足りない場合は確保する
		Size = sizeof( VERTEX3D ) * VertexNum ;
		if( GSYS.Resource.TempVertexBufferSize < ( int )Size )
		{
			if( GSYS.Resource.TempVertexBuffer )
			{
				DXFREE( GSYS.Resource.TempVertexBuffer ) ;
				GSYS.Resource.TempVertexBufferSize = 0 ;
			}
			GSYS.Resource.TempVertexBuffer = DXALLOC( Size ) ;
			if( GSYS.Resource.TempVertexBuffer == NULL )
				return -1 ;
			GSYS.Resource.TempVertexBufferSize = ( int )Size ;
		}

		// 一時バッファに変換しながら転送
		sv = Vertex;
		dv = ( VERTEX3D * )GSYS.Resource.TempVertexBuffer ;
		for( i = 0 ; i < VertexNum ; i ++, sv ++, dv ++ )
		{
			dv->pos = sv->pos ;
			dv->norm = sv->norm ;
			dv->dif.b = ( BYTE )BlueTable [ sv->dif.b ] ;
			dv->dif.g = ( BYTE )GreenTable[ sv->dif.g ] ;
			dv->dif.r = ( BYTE )RedTable  [ sv->dif.r ] ;
			dv->dif.a = ( BYTE )AlphaTable[ sv->dif.a ] ;
			dv->spc.b = ( BYTE )BlueTable [ sv->spc.b ] ;
			dv->spc.g = ( BYTE )GreenTable[ sv->spc.g ] ;
			dv->spc.r = ( BYTE )RedTable  [ sv->spc.r ] ;
			dv->spc.a = ( BYTE )AlphaTable[ sv->spc.a ] ;
			dv->u = sv->u ;
			dv->v = sv->v ;
			dv->su = sv->su ;
			dv->sv = sv->sv ;
		}
		UseVertex = ( VERTEX3D * )GSYS.Resource.TempVertexBuffer ;
	}
	else
	{
		UseVertex = Vertex ;
	}
	
	DRAW_DEF(
		Graphics_Hardware_DrawIndexedPrimitiveLight_PF( UseVertex, VertexNum, Indices, IndexNum, PrimitiveType, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// 頂点バッファを使用して３Ｄポリゴンを描画する
extern int NS_DrawPolygon3D_UseVertexBuffer( int VertexBufHandle, int GrHandle, int TransFlag )
{
	return NS_DrawPrimitive3D_UseVertexBuffer( VertexBufHandle, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag ) ;
}

// 頂点バッファを使用して３Ｄプリミティブを描画する
extern int NS_DrawPrimitive3D_UseVertexBuffer( int VertexBufHandle, int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;
	int Ret, Flag ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}
	VertexBuffer = NULL ;
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	DRAW_DEF(
		Graphics_Hardware_DrawPrimitiveLight_UseVertexBuffer_PF( VertexBuffer, PrimitiveType, 0, VertexBuffer->Num, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// 頂点バッファを使用して３Ｄプリミティブを描画する
extern int NS_DrawPrimitive3D_UseVertexBuffer2( int VertexBufHandle, int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */, int StartVertex, int UseVertexNum, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;
	int Ret, Flag ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}
	VertexBuffer = NULL ;
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	DRAW_DEF(
		Graphics_Hardware_DrawPrimitiveLight_UseVertexBuffer_PF( VertexBuffer, PrimitiveType, StartVertex, UseVertexNum, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// 頂点バッファとインデックスバッファを使用して３Ｄポリゴンを描画する
extern int NS_DrawPolygonIndexed3D_UseVertexBuffer( int VertexBufHandle, int IndexBufHandle, int GrHandle, int TransFlag )
{
	return NS_DrawPrimitiveIndexed3D_UseVertexBuffer( VertexBufHandle, IndexBufHandle, DX_PRIMTYPE_TRIANGLELIST, GrHandle, TransFlag ) ;
}

// 頂点バッファとインデックスバッファを使用して３Ｄプリミティブを描画する
extern int NS_DrawPrimitiveIndexed3D_UseVertexBuffer( int VertexBufHandle, int IndexBufHandle, int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;
	INDEXBUFFERHANDLEDATA *IndexBuffer ;
	int Ret, Flag ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}
	VertexBuffer = NULL ;
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
		return -1 ;
	if( INDEXBUFFERCHK( IndexBufHandle, IndexBuffer ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	DRAW_DEF(
		Graphics_Hardware_DrawIndexedPrimitiveLight_UseVertexBuffer_PF( VertexBuffer, IndexBuffer, PrimitiveType, 0, 0, VertexBuffer->Num, 0, IndexBuffer->Num, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// 頂点バッファとインデックスバッファを使用して３Ｄプリミティブを描画する
extern int NS_DrawPrimitiveIndexed3D_UseVertexBuffer2( int VertexBufHandle, int IndexBufHandle, int PrimitiveType  /* DX_PRIMTYPE_TRIANGLELIST 等 */, int BaseVertex, int StartVertex, int UseVertexNum, int StartIndex, int UseIndexNum, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;
	INDEXBUFFERHANDLEDATA *IndexBuffer ;
	int Ret, Flag ;

	Ret = -1;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	Image = NULL ;
	if( GrHandle != DX_NONE_GRAPH )
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		Flag = Image->Orig->FormatDesc.TextureFlag ;
	}
	else
	{
		Flag = GSYS.Setting.ValidHardware ;
	}
	VertexBuffer = NULL ;
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
		return -1 ;
	if( INDEXBUFFERCHK( IndexBufHandle, IndexBuffer ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	DRAW_DEF(
		Graphics_Hardware_DrawIndexedPrimitiveLight_UseVertexBuffer_PF( VertexBuffer, IndexBuffer, PrimitiveType, BaseVertex, StartVertex, UseVertexNum, StartIndex, UseIndexNum, Image, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Flag
	)

	return Ret ;
}

// グラフィックの３Ｄ描画
#define SETDRAWRECTCODE\
	float MinX, MinY, MaxX, MaxY ;\
\
	MinX =  100000000.0F ;\
	MinY =  100000000.0F ;\
	MaxX = -100000000.0F ;\
	MaxY = -100000000.0F ;\
	for( i = 0 ; i < 4 ; i ++ )\
	{\
		if( TransVert[i].pos.x - 1.0F < MinX ) MinX = TransVert[i].pos.x - 1.0F ;\
		if( TransVert[i].pos.x + 1.0F > MaxX ) MaxX = TransVert[i].pos.x + 1.0F ;\
\
		if( TransVert[i].pos.y - 1.0F < MinY ) MinY = TransVert[i].pos.y - 1.0F ;\
		if( TransVert[i].pos.y + 1.0F > MaxY ) MaxY = TransVert[i].pos.y + 1.0F ;\
	}\
\
	DrawRect.left   = _FTOL( MinX ) ;\
	DrawRect.top    = _FTOL( MinY ) ;\
	DrawRect.right  = _FTOL( MaxX ) ;\
	DrawRect.bottom = _FTOL( MaxY ) ;\
	DRAWRECT_DRAWAREA_CLIP

extern	int NS_DrawGraph3D( float x, float y, float z, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret = -1, i ;
	VERTEX_2D TransVert[6] ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 座標変換を行う
	{
		MATRIX *TransMatrix ;

		TransMatrix = &GSYS.DrawSetting.BlendMatrixF ;

		// 頂点情報をセットする
		{
			VECTOR SrcVec[4], Pos ;
			VERTEX_2D DestVec[4] ;
			float SizeX, SizeY ;
			
			// ビルボードの４頂点を得る
			SizeX = ( float )Image->Width  * 0.5F ;
			SizeY = ( float )Image->Height * 0.5F ;
			Pos.x = x ;	Pos.y = y ;	Pos.z = z ;
			SrcVec[0].x = -SizeX ;	SrcVec[0].y =  SizeY ;	SrcVec[0].z = 0.0F ;
			SrcVec[1].x =  SizeX ;	SrcVec[1].y =  SizeY ;	SrcVec[1].z = 0.0F ;
			SrcVec[2].x = -SizeX ;	SrcVec[2].y = -SizeY ;	SrcVec[2].z = 0.0F ;
			SrcVec[3].x =  SizeX ;	SrcVec[3].y = -SizeY ;	SrcVec[3].z = 0.0F ;

			// 座標変換
			for( i = 0 ; i < 4 ; i ++ )
			{
				// ビルボード座標をワールド座標へ変換
				Graphics_Draw_VectorTransformToBillboard( &SrcVec[i], &Pos ) ;

				// スクリーン座標に変換
				DestVec[i].rhw = 1.0F ;
				VectorTransform4( &DestVec[i].pos,  &DestVec[i].rhw,
								  &SrcVec[i],       &DestVec[i].rhw, TransMatrix ) ;
				DestVec[i].rhw = 1.0F / DestVec[i].rhw ;
				DestVec[i].pos.x *= DestVec[i].rhw ;
				DestVec[i].pos.y *= DestVec[i].rhw ;
				DestVec[i].pos.z *= DestVec[i].rhw ;

				i = i ;
			}
									            
			// 頂点情報のセット
			TransVert[0].pos = DestVec[0].pos ;
			TransVert[1].pos = DestVec[1].pos ;
			TransVert[2].pos = DestVec[2].pos ;
			TransVert[3].pos = DestVec[3].pos ;
			TransVert[0].rhw = DestVec[0].rhw ;
			TransVert[1].rhw = DestVec[1].rhw ;
			TransVert[2].rhw = DestVec[2].rhw ;
			TransVert[3].rhw = DestVec[3].rhw ;

			((BYTE *)&TransVert[0].color)[0] = (BYTE)GSYS.DrawSetting.DrawBright.Blue ;
			((BYTE *)&TransVert[0].color)[1] = (BYTE)GSYS.DrawSetting.DrawBright.Green ;
			((BYTE *)&TransVert[0].color)[2] = (BYTE)GSYS.DrawSetting.DrawBright.Red ;
			((BYTE *)&TransVert[0].color)[3] = ( BYTE )( ( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND || GSYS.DrawSetting.UseNoBlendModeParam ) ? GSYS.DrawSetting.BlendParam : 255 ) ;
			TransVert[1].color =
			TransVert[2].color =
			TransVert[3].color = TransVert[0].color ;

			TransVert[0].u = 0.0F ;	TransVert[0].v = 0.0F ;
			TransVert[1].u = 1.0F ;	TransVert[1].v = 0.0F ;
			TransVert[2].u = 0.0F ;	TransVert[2].v = 1.0F ;
			TransVert[3].u = 1.0F ;	TransVert[3].v = 1.0F ;

			TransVert[4] = TransVert[2] ;
			TransVert[5] = TransVert[1] ;
		}
	}

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive2D_PF( TransVert, 6, DX_PRIMTYPE_TRIANGLELIST, Image, TransFlag, TRUE, TRUE, FALSE, 0 ),
		0,
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

// グラフィックの拡大３Ｄ描画
extern	int NS_DrawExtendGraph3D( float x, float y, float z, double ExRateX, double ExRateY, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	int Ret = -1, i ;
	VERTEX_2D TransVert[6] ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 座標変換を行う
	{
		MATRIX *TransMatrix ;

		TransMatrix = &GSYS.DrawSetting.BlendMatrixF ;

		// 頂点情報をセットする
		{
			VECTOR SrcVec[4], Pos ;
			VERTEX_2D DestVec[4] ;
			float SizeX, SizeY ;
			
			// ビルボードの４頂点を得る
			SizeX = ( float )Image->Width  * ( float )ExRateX * 0.5F ;
			SizeY = ( float )Image->Height * ( float )ExRateY * 0.5F ;
			Pos.x = x ;	Pos.y = y ;	Pos.z = z ;
			SrcVec[0].x = -SizeX ;	SrcVec[0].y =  SizeY ;	SrcVec[0].z = 0.0F ;
			SrcVec[1].x =  SizeX ;	SrcVec[1].y =  SizeY ;	SrcVec[1].z = 0.0F ;
			SrcVec[2].x = -SizeX ;	SrcVec[2].y = -SizeY ;	SrcVec[2].z = 0.0F ;
			SrcVec[3].x =  SizeX ;	SrcVec[3].y = -SizeY ;	SrcVec[3].z = 0.0F ;

			// 座標変換
			for( i = 0 ; i < 4 ; i ++ )
			{
				// ビルボード座標をワールド座標へ変換
				Graphics_Draw_VectorTransformToBillboard( &SrcVec[i], &Pos ) ;

				// スクリーン座標に変換
				DestVec[i].rhw = 1.0F ;
				VectorTransform4( &DestVec[i].pos,  &DestVec[i].rhw,
									      &SrcVec[i],  &DestVec[i].rhw, TransMatrix ) ;
				DestVec[i].rhw = 1.0F / DestVec[i].rhw ;
				DestVec[i].pos.x *= DestVec[i].rhw ;
				DestVec[i].pos.y *= DestVec[i].rhw ;
				DestVec[i].pos.z *= DestVec[i].rhw ;
			}
									            
			// 頂点情報のセット
			TransVert[0].pos = DestVec[0].pos ;
			TransVert[1].pos = DestVec[1].pos ;
			TransVert[2].pos = DestVec[2].pos ;
			TransVert[3].pos = DestVec[3].pos ;
			TransVert[0].rhw = DestVec[0].rhw ;
			TransVert[1].rhw = DestVec[1].rhw ;
			TransVert[2].rhw = DestVec[2].rhw ;
			TransVert[3].rhw = DestVec[3].rhw ;

			((BYTE *)&TransVert[0].color)[0] = (BYTE)GSYS.DrawSetting.DrawBright.Blue ;
			((BYTE *)&TransVert[0].color)[1] = (BYTE)GSYS.DrawSetting.DrawBright.Green ;
			((BYTE *)&TransVert[0].color)[2] = (BYTE)GSYS.DrawSetting.DrawBright.Red ;
			((BYTE *)&TransVert[0].color)[3] = ( BYTE )( ( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND || GSYS.DrawSetting.UseNoBlendModeParam ) ? GSYS.DrawSetting.BlendParam : 255 ) ;
			TransVert[1].color =
			TransVert[2].color =
			TransVert[3].color = TransVert[0].color ;

			TransVert[0].u = 0.0F ;	TransVert[0].v = 0.0F ;
			TransVert[1].u = 1.0F ;	TransVert[1].v = 0.0F ;
			TransVert[2].u = 0.0F ;	TransVert[2].v = 1.0F ;
			TransVert[3].u = 1.0F ;	TransVert[3].v = 1.0F ;

			TransVert[4] = TransVert[2] ;
			TransVert[5] = TransVert[1] ;
		}
	}

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive2D_PF( TransVert, 6, DX_PRIMTYPE_TRIANGLELIST, Image, TransFlag, TRUE, TRUE, FALSE, 0 ),
		0,
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

// グラフィックの回転３Ｄ描画
extern	int NS_DrawRotaGraph3D( float x, float y, float z, double ExRate, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	IMAGEDATA *Image ;
	int Ret = -1, i ;
	VERTEX_2D TransVert[6] ;
	float Sin, Cos ;
	float ExtendRate = ( float )ExRate ; 
	float ExtendRateX ;
	float XBuf, YBuf ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 座標変換を行う
	{
		MATRIX *TransMatrix ;

		TransMatrix = &GSYS.DrawSetting.BlendMatrixF ;

		// 頂点情報をセットする
		{
			VECTOR SrcVec[4], Pos ;
			VERTEX_2D DestVec[4]/*, VectBuf */ ;
			float SizeX, SizeY ;

			// 反転を考慮した横方向の拡大率をセット
			ExtendRateX = ( ( TurnFlag == TRUE ) ? -1.0f : 1.0f ) * ExtendRate ;

			// ビルボードの４頂点を得る
			{
				SizeX = Image->Width  * 0.5F * ExtendRateX ;
				SizeY = Image->Height * 0.5F * ExtendRate  ;
				Pos.x = x ;	Pos.y = y ;	Pos.z = z ;
				SrcVec[0].x = -SizeX ;	SrcVec[0].y =  SizeY ;	SrcVec[0].z = 0.0F ;
				SrcVec[1].x =  SizeX ;	SrcVec[1].y =  SizeY ;	SrcVec[1].z = 0.0F ;
				SrcVec[2].x = -SizeX ;	SrcVec[2].y = -SizeY ;	SrcVec[2].z = 0.0F ;
				SrcVec[3].x =  SizeX ;	SrcVec[3].y = -SizeY ;	SrcVec[3].z = 0.0F ;

				// 回転
				_SINCOS( (float)Angle, &Sin, &Cos ) ;
				for( i = 0 ; i < 4 ; i ++ )
				{
					XBuf = SrcVec[i].x ;
					YBuf = SrcVec[i].y ;

					SrcVec[i].x = XBuf * Cos - YBuf * Sin ;	
					SrcVec[i].y = XBuf * Sin + YBuf * Cos ;
				}
			}

			// 座標変換
			for( i = 0 ; i < 4 ; i ++ )
			{
				// ビルボード座標をワールド座標へ変換
				Graphics_Draw_VectorTransformToBillboard( &SrcVec[i], &Pos ) ;

				// スクリーン座標に変換
				DestVec[i].rhw = 1.0F ;
				VectorTransform4( &DestVec[i].pos,  &DestVec[i].rhw,
									      &SrcVec[i],  &DestVec[i].rhw, TransMatrix ) ;
				DestVec[i].rhw = 1.0F / DestVec[i].rhw ;
				DestVec[i].pos.x *= DestVec[i].rhw ;
				DestVec[i].pos.y *= DestVec[i].rhw ;
				DestVec[i].pos.z *= DestVec[i].rhw ;
			}
									            
			// 頂点情報のセット
			TransVert[0].pos = DestVec[0].pos ;
			TransVert[1].pos = DestVec[1].pos ;
			TransVert[2].pos = DestVec[2].pos ;
			TransVert[3].pos = DestVec[3].pos ;
			TransVert[0].rhw = DestVec[0].rhw ;
			TransVert[1].rhw = DestVec[1].rhw ;
			TransVert[2].rhw = DestVec[2].rhw ;
			TransVert[3].rhw = DestVec[3].rhw ;

			((BYTE *)&TransVert[0].color)[0] = (BYTE)GSYS.DrawSetting.DrawBright.Blue ;
			((BYTE *)&TransVert[0].color)[1] = (BYTE)GSYS.DrawSetting.DrawBright.Green ;
			((BYTE *)&TransVert[0].color)[2] = (BYTE)GSYS.DrawSetting.DrawBright.Red ;
			((BYTE *)&TransVert[0].color)[3] = ( BYTE )( ( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND || GSYS.DrawSetting.UseNoBlendModeParam ) ? GSYS.DrawSetting.BlendParam : 255 ) ;
			TransVert[1].color =
			TransVert[2].color =
			TransVert[3].color = TransVert[0].color ;

			TransVert[0].u = 0.0F ;	TransVert[0].v = 0.0F ;
			TransVert[1].u = 1.0F ;	TransVert[1].v = 0.0F ;
			TransVert[2].u = 0.0F ;	TransVert[2].v = 1.0F ;
			TransVert[3].u = 1.0F ;	TransVert[3].v = 1.0F ;

			// 反転処理
//			if( TurnFlag == TRUE )
//			{
//				VectBuf = TransVert[ 1 ] ; TransVert[ 1 ] = TransVert[ 2 ] ; TransVert[ 2 ] = VectBuf ;
//			}

			TransVert[4] = TransVert[2] ;
			TransVert[5] = TransVert[1] ;
		}
	}

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive2D_PF( TransVert, 6, DX_PRIMTYPE_TRIANGLELIST, Image, TransFlag, TRUE, TRUE, FALSE, 0 ),
		0,
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

// グラフィックの回転３Ｄ描画(回転中心指定型)
extern	int NS_DrawRota2Graph3D( float x, float y, float z, float cx, float cy, double ExtRateX, double ExtRateY, double Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	IMAGEDATA *Image ;
	int Ret = -1, i ;
	VERTEX_2D TransVert[6] ;
	float Sin, Cos ;
	float ExtendRateX = ( float )ExtRateX ; 
	float ExtendRateY = ( float )ExtRateY ; 
	float XBuf, YBuf ;

	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

#ifndef DX_NON_MOVIE
	if( GrHandle != DX_NONE_GRAPH )
		UpdateMovie( Image->MovieHandle ) ;
#endif

	// 行列をハードウエアに反映する
	Graphics_DrawSetting_ApplyLibMatrixToHardware() ;

	// 座標変換を行う
	{
		MATRIX *TransMatrix ;

		TransMatrix = &GSYS.DrawSetting.BlendMatrixF ;

		// 頂点情報をセットする
		{
			VECTOR SrcVec[4], Pos ;
			VERTEX_2D DestVec[4]/* , VectBuf */ ;
			float SizeX, SizeY ;
			
			// ビルボードの４頂点を得る
			{
				SizeX = (float)Image->Width ;
				SizeY = (float)Image->Height ;
				Pos.x = x ;	Pos.y = y ;	Pos.z = z ;
				if( TurnFlag == TRUE )
				{
					SrcVec[0].x = ( -cx + SizeX ) * ExtendRateX ;	SrcVec[0].y = ( -cy + SizeY ) * ExtendRateY ;	SrcVec[0].z = 0.0F ;
					SrcVec[1].x = -cx             * ExtendRateX ;	SrcVec[1].y = ( -cy + SizeY ) * ExtendRateY ;	SrcVec[1].z = 0.0F ;
					SrcVec[2].x = ( -cx + SizeX ) * ExtendRateX ;	SrcVec[2].y = -cy             * ExtendRateY ;	SrcVec[2].z = 0.0F ;
					SrcVec[3].x = -cx             * ExtendRateX ;	SrcVec[3].y = -cy             * ExtendRateY ;	SrcVec[3].z = 0.0F ;
				}
				else
				{
					SrcVec[0].x = -cx             * ExtendRateX ;	SrcVec[0].y = ( -cy + SizeY ) * ExtendRateY ;	SrcVec[0].z = 0.0F ;
					SrcVec[1].x = ( -cx + SizeX ) * ExtendRateX ;	SrcVec[1].y = ( -cy + SizeY ) * ExtendRateY ;	SrcVec[1].z = 0.0F ;
					SrcVec[2].x = -cx             * ExtendRateX ;	SrcVec[2].y = -cy             * ExtendRateY ;	SrcVec[2].z = 0.0F ;
					SrcVec[3].x = ( -cx + SizeX ) * ExtendRateX ;	SrcVec[3].y = -cy             * ExtendRateY ;	SrcVec[3].z = 0.0F ;
				}

				// 回転
				_SINCOS( (float)Angle, &Sin, &Cos ) ;
				for( i = 0 ; i < 4 ; i ++ )
				{
					XBuf = SrcVec[i].x ;
					YBuf = SrcVec[i].y ;

					SrcVec[i].x = XBuf * Cos - YBuf * Sin ;	
					SrcVec[i].y = XBuf * Sin + YBuf * Cos ;
				}
			}

			// 座標変換
			for( i = 0 ; i < 4 ; i ++ )
			{
				// ビルボード座標をワールド座標へ変換
				Graphics_Draw_VectorTransformToBillboard( &SrcVec[i], &Pos ) ;

				// スクリーン座標に変換
				DestVec[i].rhw = 1.0F ;
				VectorTransform4( &DestVec[i].pos,  &DestVec[i].rhw,
									      &SrcVec[i],  &DestVec[i].rhw, TransMatrix ) ;
				DestVec[i].rhw = 1.0F / DestVec[i].rhw ;
				DestVec[i].pos.x *= DestVec[i].rhw ;
				DestVec[i].pos.y *= DestVec[i].rhw ;
				DestVec[i].pos.z *= DestVec[i].rhw ;
			}
									            
			// 頂点情報のセット
			TransVert[0].pos = DestVec[0].pos ;
			TransVert[1].pos = DestVec[1].pos ;
			TransVert[2].pos = DestVec[2].pos ;
			TransVert[3].pos = DestVec[3].pos ;
			TransVert[0].rhw = DestVec[0].rhw ;
			TransVert[1].rhw = DestVec[1].rhw ;
			TransVert[2].rhw = DestVec[2].rhw ;
			TransVert[3].rhw = DestVec[3].rhw ;

			((BYTE *)&TransVert[0].color)[0] = (BYTE)GSYS.DrawSetting.DrawBright.Blue ;
			((BYTE *)&TransVert[0].color)[1] = (BYTE)GSYS.DrawSetting.DrawBright.Green ;
			((BYTE *)&TransVert[0].color)[2] = (BYTE)GSYS.DrawSetting.DrawBright.Red ;
			((BYTE *)&TransVert[0].color)[3] = ( BYTE )( ( GSYS.DrawSetting.BlendMode != DX_BLENDMODE_NOBLEND || GSYS.DrawSetting.UseNoBlendModeParam ) ? GSYS.DrawSetting.BlendParam : 255 ) ;
			TransVert[1].color =
			TransVert[2].color =
			TransVert[3].color = TransVert[0].color ;

			TransVert[0].u = 0.0F ;	TransVert[0].v = 0.0F ;
			TransVert[1].u = 1.0F ;	TransVert[1].v = 0.0F ;
			TransVert[2].u = 0.0F ;	TransVert[2].v = 1.0F ;
			TransVert[3].u = 1.0F ;	TransVert[3].v = 1.0F ;

			TransVert[4] = TransVert[2] ;
			TransVert[5] = TransVert[1] ;

			// 反転処理
//			if( TurnFlag == TRUE )
//			{
//				VectBuf = TransVert[ 1 ] ; TransVert[ 1 ] = TransVert[ 2 ] ; TransVert[ 2 ] = VectBuf ;
//			}

			TransVert[4] = TransVert[2] ;
			TransVert[5] = TransVert[1] ;
		}
	}

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawPrimitive2D_PF( TransVert, 6, DX_PRIMTYPE_TRIANGLELIST, Image, TransFlag, TRUE, TRUE, FALSE, 0 ),
		0,
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE

// グラフィックの自由変形３Ｄ描画
extern int NS_DrawModiBillboard3D( VECTOR Pos, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, int GrHandle, int TransFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawModiBillboard3D_PF( Pos, x1, y1, x2, y2, x3, y3, x4, y4, Image, BlendImage, TransFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

// ３Ｄ空間上にグラフィックを描画
extern int NS_DrawBillboard3D( VECTOR Pos, float cx, float cy, float Size, float Angle, int GrHandle, int TransFlag, int TurnFlag )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE ) return -1 ;

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) ) return -1 ;
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
		UpdateMovie( Image->MovieHandle, FALSE ) ;
#endif

	// 描画
	DRAW_DEF(
		Graphics_Hardware_DrawBillboard3D_PF( Pos, cx, cy, Size, Angle, Image, BlendImage, TransFlag, TurnFlag ),
		0,
		DrawRect = GSYS.DrawSetting.DrawArea ;,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

























// 描画設定関係関数

// 描画モードをセットする
extern	int NS_SetDrawMode( int DrawMode )
{
	if( /* GSYS.NotDrawFlag || */ DrawMode == GSYS.DrawSetting.DrawMode )
		return 0 ;

	// 描画モードの保存
	GSYS.DrawSetting.DrawMode = DrawMode ;
	GSYS.ChangeSettingFlag = TRUE ;

	// ハードウエアアクセラレーションの設定に反映
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetDrawMode_PF( DrawMode ) ;
	}

	// 終了
	return 0 ;
}

// 描画モードを取得する
extern int NS_GetDrawMode( void )
{
	return GSYS.DrawSetting.DrawMode ;
}

// 描画ブレンドモードをセットする
extern	int NS_SetDrawBlendMode( int BlendMode, int BlendParam )
{
	if( /* GSYS.NotDrawFlag || */ ( GSYS.DrawSetting.BlendMode == BlendMode && GSYS.DrawSetting.BlendParam == BlendParam ) )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// ブレンドモードの保存
	if( GSYS.DrawSetting.BlendMode != BlendMode )
	{
		GSYS.DrawSetting.BlendMode = BlendMode ;
		GSYS.ChangeSettingFlag = TRUE ;
	}

	if( BlendParam > 255 ) BlendParam = 255 ;
	if( BlendParam < 0	 ) BlendParam = 0 ;
	GSYS.DrawSetting.BlendParam = BlendParam ;
	SetMemImgDrawBlendMode( GSYS.DrawSetting.BlendMode, GSYS.DrawSetting.BlendParam ) ;

	// ハードウエアアクセラレーションの設定に反映
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetDrawBlendMode_PF( BlendMode, BlendParam ) ;
	}

	// 終了
	return 0 ;
}

// 描画ブレンドモードを取得する
extern int NS_GetDrawBlendMode( int *BlendMode, int *BlendParam )
{
	if( BlendMode	!= NULL ) *BlendMode =	GSYS.DrawSetting.BlendMode ;
	if( BlendParam	!= NULL ) *BlendParam =	GSYS.DrawSetting.BlendParam ;

	// 終了
	return 0 ;
}

// 描画時のアルファテストの設定を行う( TestMode:DX_CMP_GREATER等( -1:デフォルト動作に戻す )  TestParam:描画アルファ値との比較に使用する値 )
extern	int	NS_SetDrawAlphaTest( int TestMode, int TestParam )
{
	if( /* GSYS.NotDrawFlag || */ ( GSYS.DrawSetting.AlphaTestMode == TestMode && GSYS.DrawSetting.AlphaTestParam == TestParam ) )
		return 0 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// アルファテストパラメータを保存
	GSYS.DrawSetting.AlphaTestMode = TestMode ;
	if( TestParam > 255 ) TestParam = 255 ;
	if( TestParam < 0	 ) TestParam = 0 ;
	GSYS.DrawSetting.AlphaTestParam = TestParam ;
	GSYS.ChangeSettingFlag = TRUE ;

	// ハードウエアアクセラレーションの設定に反映
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetDrawAlphaTest_PF( TestMode, TestParam ) ;
	}

	// 終了
	return 0 ;
}

// 描画処理時に描画する画像とブレンドするαチャンネル付き画像をセットする
extern	int NS_SetBlendGraph( int BlendGraph, int BorderParam, int BorderRange )
{
	return NS_SetBlendGraphParam( BlendGraph, DX_BLENDGRAPHTYPE_WIPE, BorderParam, BorderRange ) ;
}

// 描画処理時に描画する画像とブレンドする画像のブレンド設定を行う
extern	int NS_SetBlendGraphParam( int BlendGraph, int BlendType, ... )
{
	int Result ;
	va_list VaList ;

	va_start( VaList, BlendType ) ;

	Result = Graphics_DrawSetting_SetBlendGraphParamBase(  BlendGraph,  BlendType, VaList ) ;

	va_end( VaList ) ;

	return Result ;
}

// ブレンド画像の起点座標をセットする
extern	int NS_SetBlendGraphPosition( int x, int y )
{
	// 今までと値が同じ場合は何もせず終了
	if( GSYS.DrawSetting.BlendGraphX == x && GSYS.DrawSetting.BlendGraphY == y ) return 0 ;

	// 座標を保存
	GSYS.DrawSetting.BlendGraphX = x ;
	GSYS.DrawSetting.BlendGraphY = y ;

	// 終了
	return 0 ;
}

// 描画輝度をセット
extern	int NS_SetDrawBright( int RedBright, int GreenBright, int BlueBright )
{
	bool OldWhiteFlag ;
	bool NewWhiteFlag ;

	if( GSYS.DrawSetting.DrawBright.Red   == RedBright  &&
		GSYS.DrawSetting.DrawBright.Green == GreenBright &&
		GSYS.DrawSetting.DrawBright.Blue  == BlueBright )
	{
		return 0 ;
	}

	if( (unsigned int)RedBright   > 255 )
	{
		     if( RedBright   > 255 ) RedBright   = 255 ;
		else if( RedBright   < 0   ) RedBright   = 0   ;
	}
	if( (unsigned int)GreenBright > 255 )
	{
		     if( GreenBright > 255 ) GreenBright = 255 ;
		else if( GreenBright < 0   ) GreenBright = 0   ;
	}
	if( (unsigned int)BlueBright  > 255 )
	{
		     if( BlueBright  > 255 ) BlueBright  = 255 ;
		else if( BlueBright  < 0   ) BlueBright  = 0   ;
	}

	// 輝度を保存
	OldWhiteFlag = ( GSYS.DrawSetting.bDrawBright & 0xffffff ) == 0xffffff ;
	GSYS.DrawSetting.DrawBright.Red   = ( BYTE )RedBright   ;
	GSYS.DrawSetting.DrawBright.Blue  = ( BYTE )BlueBright  ;
	GSYS.DrawSetting.DrawBright.Green = ( BYTE )GreenBright ;
	NewWhiteFlag = ( GSYS.DrawSetting.bDrawBright & 0xffffff ) == 0xffffff ;
	SetMemImgDrawBright( GSYS.DrawSetting.bDrawBright ) ;

	if( OldWhiteFlag != NewWhiteFlag )
	{
		GSYS.ChangeSettingFlag = TRUE ;
	}

	// ハードウエアアクセラレーションの設定に反映
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetDrawBright_PF( RedBright, GreenBright, BlueBright ) ;
	}

	// 終了
	return 0 ;
}

// 描画輝度を取得する
extern int NS_GetDrawBright( int *Red, int *Green, int *Blue )
{
	if( Red		!= NULL ) *Red =	GSYS.DrawSetting.DrawBright.Red ;
	if( Green	!= NULL ) *Green =	GSYS.DrawSetting.DrawBright.Green ;
	if( Blue	!= NULL ) *Blue =	GSYS.DrawSetting.DrawBright.Blue ;

	// 終了
	return 0 ;
}

// 描画する画像の色成分を無視するかどうかを指定する( EnableFlag:この機能を使うかどうか( TRUE:使う  FALSE:使わない ) )
extern int NS_SetIgnoreDrawGraphColor( int EnableFlag )
{
	if( GSYS.DrawSetting.IgnoreGraphColorFlag == EnableFlag )
		return 0 ;

	// 設定を保存
	GSYS.DrawSetting.IgnoreGraphColorFlag = EnableFlag ;
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// 最大異方性の値をセットする
extern int NS_SetMaxAnisotropy( int MaxAnisotropy )
{
	if( /* GSYS.NotDrawFlag || */ MaxAnisotropy == GSYS.DrawSetting.MaxAnisotropy )
		return 0 ;

	// 最大異方性の保存
	GSYS.DrawSetting.MaxAnisotropy = MaxAnisotropy ;
	GSYS.ChangeSettingFlag = TRUE ;

	// ハードウエアアクセラレーションの設定に反映
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetMaxAnisotropy_PF( MaxAnisotropy ) ;
	}

	// 終了
	return 0 ;
}

// ３Ｄ処理で使用する座標値が 10000000.0f などの大きな値になっても描画の崩れを小さく抑える処理を使用するかどうかを設定する、DxLib_Init の呼び出し前でのみ使用可能( TRUE:描画の崩れを抑える処理を使用する( CPU負荷が上がります )　　FALSE:描画の崩れを抑える処理は使用しない( デフォルト ) )
extern int NS_SetUseLarge3DPositionSupport( int UseFlag )
{
	// 初期化前のみ有効
	if( DxSysData.DxLib_InitializeFlag )
	{
		return -1 ;
	}

	// フラグを保存
	GSYS.DrawSetting.Large3DPositionSupport = UseFlag ;

#ifdef __WINDOWS__
	// TRUE の場合は SetUseFPUPreserveFlag も有効にする
	NS_SetUseFPUPreserveFlag( TRUE ) ;
#endif // __WINDOWS__

	// 終了
	return 0 ;
}


// Ｚバッファを有効にするか、フラグをセットする
extern	int NS_SetUseZBufferFlag( int Flag )
{
	if( GSYS.DrawSetting.EnableZBufferFlag2D == Flag &&
		GSYS.DrawSetting.EnableZBufferFlag3D == Flag ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.EnableZBufferFlag2D = Flag ;
	GSYS.DrawSetting.EnableZBufferFlag3D = Flag ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// Ｚバッファに書き込みを行うか、フラグをセットする
extern	int NS_SetWriteZBufferFlag( int Flag )
{
	if( GSYS.DrawSetting.WriteZBufferFlag2D == Flag &&
		GSYS.DrawSetting.WriteZBufferFlag3D == Flag ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.WriteZBufferFlag2D = Flag ;
	GSYS.DrawSetting.WriteZBufferFlag3D = Flag ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// Ｚ値の比較モードをセットする
extern	int NS_SetZBufferCmpType( int CmpType /* DX_CMP_NEVER 等 */ )
{
	if( GSYS.DrawSetting.ZBufferCmpType2D == CmpType &&
		GSYS.DrawSetting.ZBufferCmpType3D == CmpType ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.ZBufferCmpType2D = CmpType ;
	GSYS.DrawSetting.ZBufferCmpType3D = CmpType ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// Ｚバイアスをセットする
extern	int NS_SetZBias( int Bias )
{
	if( GSYS.DrawSetting.ZBias2D == Bias &&
		GSYS.DrawSetting.ZBias3D == Bias ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.ZBias2D = Bias ;
	GSYS.DrawSetting.ZBias3D = Bias ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// Ｚバッファを有効にするか、フラグをセットする( ３Ｄ図形描画のみに影響 )
extern int NS_SetUseZBuffer3D( int Flag )
{
	if( GSYS.DrawSetting.EnableZBufferFlag3D == Flag ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.EnableZBufferFlag3D = Flag ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// Ｚバッファに書き込みを行うか、フラグをセットする( ３Ｄ図形描画のみに影響 )
extern int NS_SetWriteZBuffer3D( int Flag )
{
	if( GSYS.DrawSetting.WriteZBufferFlag3D == Flag ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.WriteZBufferFlag3D = Flag ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// Ｚ値の比較モードをセットする( ３Ｄ図形描画のみに影響 )
extern int NS_SetZBufferCmpType3D( int CmpType /* DX_CMP_NEVER 等 */ )
{
	if( GSYS.DrawSetting.ZBufferCmpType3D == CmpType ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.ZBufferCmpType3D = CmpType ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// Ｚバイアスをセットする( ３Ｄ図形描画のみに影響 )
extern int NS_SetZBias3D( int Bias )
{
	if( GSYS.DrawSetting.ZBias3D == Bias ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.ZBias3D = Bias ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ２Ｄ描画時にＺバッファに書き込むＺ値を変更する
extern int NS_SetDrawZ( float Z )
{
	GSYS.DrawSetting.DrawZ = Z;

	// 終了
	return 0;
}



// 描画可能領域のセット
extern	int NS_SetDrawArea( int x1, int y1, int x2, int y2 )
{
	const char *HandleString = NULL ;
	int Buf, SizeX, SizeY ;
	IMAGEDATA *Image ;
	SHADOWMAPDATA *ShadowMap ;

//	if( GSYS.NotDrawFlag ) return 0 ;

	// フルスクリーンマスク処理を行っている場合はマスク描画の結果を反映させる
	MASK_FULLSCREEN_MASKUPDATE

	// ソフトが非アクティブの場合はアクティブになるまで待つ
//	CheckActiveState() ;

	// オリジナルの描画エリアを保存
	GSYS.DrawSetting.OriginalDrawRect.left   = x1 ;
	GSYS.DrawSetting.OriginalDrawRect.top    = y1 ;
	GSYS.DrawSetting.OriginalDrawRect.right  = x2 ;
	GSYS.DrawSetting.OriginalDrawRect.bottom = y2 ;
	
	// 描画先の描画可能サイズを取得
	switch( GSYS.DrawSetting.TargetScreen[ 0 ] )
	{
	case DX_SCREEN_BACK :
	case DX_SCREEN_FRONT :
		HandleString = GSYS.DrawSetting.TargetScreen[ 0 ] == DX_SCREEN_BACK ? "Back Buffer" : "Front Buffer" ;
		SizeX = GSYS.Screen.MainScreenSizeX ;
		SizeY = GSYS.Screen.MainScreenSizeY ;
		break ;

	default :
		HandleString = "Image Buffer" ;
		// エラー判定
		if( !GRAPHCHK( GSYS.DrawSetting.TargetScreen[ 0 ], Image ) )
		{
			SizeX = Image->Width ;
			SizeY = Image->Height ;
		}
		else
		if( !SHADOWMAPCHK( GSYS.DrawSetting.TargetScreen[ 0 ], ShadowMap ) )
		{
			SizeX = ShadowMap->BaseSizeX ;
			SizeY = ShadowMap->BaseSizeY ;
		}
		else
		{
			return -1 ;
		}
		break ;
	}

	// 左右逆補正とはみ出し補正
	if( x1 > x2 ){ Buf = x1 ; x1 = x2 ; x2 = Buf ; }
	if( y1 > y2 ){ Buf = y1 ; y1 = y2 ; y2 = Buf ; }

	if( x1 < 0 )		x1 = 0 ;
	if( x1 > SizeX )	x1 = SizeX ; 
	if( x2 < 0 )		x2 = 0 ;
	if( x2 > SizeX )	x2 = SizeX ;

	if( y1 < 0 )		y1 = 0 ;
	if( y1 > SizeY )	y1 = SizeY ; 
	if( y2 < 0 )		y2 = 0 ;
	if( y2 > SizeY )	y2 = SizeY ;

	// 描画可能矩形セット
	GSYS.DrawSetting.DrawAreaF.left   = ( float )( GSYS.DrawSetting.DrawArea.left   = x1 ) ;
	GSYS.DrawSetting.DrawAreaF.top    = ( float )( GSYS.DrawSetting.DrawArea.top    = y1 ) ;
	GSYS.DrawSetting.DrawAreaF.right  = ( float )( GSYS.DrawSetting.DrawArea.right  = x2 ) ;
	GSYS.DrawSetting.DrawAreaF.bottom = ( float )( GSYS.DrawSetting.DrawArea.bottom = y2 ) ;
	SetMemImgDrawArea( &GSYS.DrawSetting.DrawArea ) ;

	// 描画可能フラグをセットする
	GSYS.DrawSetting.NotDrawFlagInSetDrawArea = x1 == x2 || y1 == y2 ;
	if( GSYS.DrawSetting.NotDrawFlagInSetDrawArea == FALSE )
	{
		// Direct3D のビューポート行列の更新
		CreateViewportMatrixD(
			&GSYS.DrawSetting.Direct3DViewportMatrix,
			( double )( GSYS.DrawSetting.DrawArea.right  - GSYS.DrawSetting.DrawArea.left ) / 2.0 + GSYS.DrawSetting.DrawArea.left,
			( double )( GSYS.DrawSetting.DrawArea.bottom - GSYS.DrawSetting.DrawArea.top  ) / 2.0 + GSYS.DrawSetting.DrawArea.top,
			( double )( GSYS.DrawSetting.DrawArea.right  - GSYS.DrawSetting.DrawArea.left ),
			( double )( GSYS.DrawSetting.DrawArea.bottom - GSYS.DrawSetting.DrawArea.top  ) ) ;
		CreateInverseMatrixD( &GSYS.DrawSetting.Direct3DViewportMatrixAnti, &GSYS.DrawSetting.Direct3DViewportMatrix ) ;
		ConvertMatrixDtoF( &GSYS.DrawSetting.Direct3DViewportMatrixAntiF, &GSYS.DrawSetting.Direct3DViewportMatrixAnti ) ;
		ConvertMatrixDtoF( &GSYS.DrawSetting.Direct3DViewportMatrixF,     &GSYS.DrawSetting.Direct3DViewportMatrix ) ;

		// ハードウエアアクセラレータを使用する場合の処理
		if( GSYS.Setting.ValidHardware )
		{
			Graphics_Hardware_SetDrawArea_PF( x1, y1, x2, y2 ) ;
		}
	}

	// 終了
	return 0 ;
}

// 描画可能領域を得る
extern int NS_GetDrawArea( RECT *Rect )
{
	// セット
	*Rect = GSYS.DrawSetting.DrawArea ;

	// 終了
	return 0 ;
}

// 描画可能領域を描画対象画面全体にする
extern int NS_SetDrawAreaFull( void )
{
	int Width, Height ;

	NS_GetGraphSize( GSYS.DrawSetting.TargetScreen[ 0 ], &Width, &Height ) ;
	return NS_SetDrawArea( 0, 0, Width, Height ) ;
}

// ３Ｄ描画のスケールをセットする
extern int NS_SetDraw3DScale( float Scale )
{
	// スケール値を補正
	if( Scale < 0.00001f && Scale > -0.00001f )
		Scale = 1.0f ;

	// スケール値を保存
	GSYS.DrawSetting.Draw3DScale = Scale ;

	// 消失点の再計算
	NS_SetCameraScreenCenterD( GSYS.Camera.ScreenCenterX, GSYS.Camera.ScreenCenterY ) ;

	// 終了
	return 0 ;
}

// SetRestoreGraphCallback の旧名
extern int NS_SetRestoreShredPoint( void (* ShredPoint )( void ) )
{
	return NS_SetRestoreGraphCallback( ShredPoint ) ;
}

// グラフィックハンドル復元関数の登録
extern	int NS_SetRestoreGraphCallback( void (* Callback )( void ) )
{
	// グラフィック復元スレッドアドレスの登録
	if( Callback == NULL )
	{
		GSYS.Setting.GraphRestoreShred = Graphics_Image_DefaultRestoreGraphFunction ;
	}
	else
	{
		GSYS.Setting.GraphRestoreShred = Callback ;
	}

	// 終了
	return 0 ;
}

// グラフィック復元関数の実行
extern	int NS_RunRestoreShred( void )
{
#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込みは一時的に無効にする
	int UseASyncThread = GetASyncLoadFlag() ;
	NS_SetUseASyncLoadFlag( FALSE ) ;
#endif

	// グラフィック復元スレッドの実行
	if( GSYS.Setting.GraphRestoreShred == NULL )
	{
		Graphics_Image_DefaultRestoreGraphFunction() ;
	}
	else
	{
		GSYS.Setting.GraphRestoreShred() ;
	}

#ifndef DX_NON_MODEL
	// モデルのテクスチャを再読み込み
	MV1ReloadTexture() ;
#endif

#ifndef DX_NON_FONT
	// フォントハンドルが持つキャッシュを初期化
	InitCacheFontToHandle() ;
#endif

#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込みの設定を元に戻す
	NS_SetUseASyncLoadFlag( UseASyncThread ) ;
#endif

	// 終了
	return 0 ;
}

// グラフィックスデバイスがロストから復帰した際に呼ばれるコールバック関数を設定する
extern int NS_SetGraphicsDeviceRestoreCallbackFunction( void (* Callback )( void *Data ), void *CallbackData )
{
	return Graphics_Hardware_SetGraphicsDeviceRestoreCallbackFunction_PF( Callback, CallbackData ) ;
}

// グラフィックスデバイスがロストから復帰する前に呼ばれるコールバック関数を設定する
extern int NS_SetGraphicsDeviceLostCallbackFunction( void (* Callback )( void *Data ), void *CallbackData )
{
	return Graphics_Hardware_SetGraphicsDeviceLostCallbackFunction_PF( Callback, CallbackData ) ;
}

// ワールド変換用行列をセットする
extern	int NS_SetTransformToWorld( const MATRIX *Matrix )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// データを保存
	GSYS.DrawSetting.WorldMatrixF = *Matrix ;
	ConvertMatrixFtoD( &GSYS.DrawSetting.WorldMatrix, &GSYS.DrawSetting.WorldMatrixF ) ;
	GSYS.DrawSetting.ValidBlendMatrix = FALSE ;

	// 全ての行列を掛け合わせた行列を更新
	Graphics_DrawSetting_RefreshBlendTransformMatrix() ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTransformToWorld_PF( &GSYS.DrawSetting.WorldMatrixF ) ;

		GSYS.DrawSetting.MatchHardwareWorldMatrix = TRUE ;
	}

	// 終了
	return 0 ;
}

// ワールド変換用行列をセットする
extern	int NS_SetTransformToWorldD( const MATRIX_D *Matrix )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// データを保存
	GSYS.DrawSetting.WorldMatrix = *Matrix ;
	ConvertMatrixDtoF( &GSYS.DrawSetting.WorldMatrixF, &GSYS.DrawSetting.WorldMatrix ) ;
	GSYS.DrawSetting.ValidBlendMatrix = FALSE ;

	// 全ての行列を掛け合わせた行列を更新
	Graphics_DrawSetting_RefreshBlendTransformMatrix() ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTransformToWorld_PF( &GSYS.DrawSetting.WorldMatrixF ) ;

		GSYS.DrawSetting.MatchHardwareWorldMatrix = TRUE ;
	}

	// 終了
	return 0 ;
}

// ワールド行列を取得する
extern	int NS_GetTransformToWorldMatrix( MATRIX *MatBuf )
{
	ConvertMatrixDtoF( MatBuf, &GSYS.DrawSetting.WorldMatrix ) ;

	// 終了
	return 0 ;
}

// ワールド行列を取得する
extern	int NS_GetTransformToWorldMatrixD( MATRIX_D *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.WorldMatrix ;

	// 終了
	return 0 ;
}

// ビュー変換用行列をセットする
extern	int NS_SetTransformToView( const MATRIX *Matrix )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// データを保存
	GSYS.DrawSetting.ViewMatrixF      = *Matrix ;
	ConvertMatrixFtoD( &GSYS.DrawSetting.ViewMatrix, &GSYS.DrawSetting.ViewMatrixF ) ;
	GSYS.DrawSetting.ValidBlendMatrix = FALSE ;

	// 全ての行列を掛け合わせた行列を更新
	Graphics_DrawSetting_RefreshBlendTransformMatrix() ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTransformToView_PF( &GSYS.DrawSetting.ViewMatrixF ) ;

		GSYS.DrawSetting.MatchHardwareViewMatrix = TRUE ;
	}

	// 終了
	return 0 ;
}

// ビュー変換用行列をセットする
extern	int NS_SetTransformToViewD( const MATRIX_D *Matrix )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// データを保存
	GSYS.DrawSetting.ViewMatrix       = *Matrix ;
	ConvertMatrixDtoF( &GSYS.DrawSetting.ViewMatrixF, &GSYS.DrawSetting.ViewMatrix ) ;
	GSYS.DrawSetting.ValidBlendMatrix = FALSE ;

	// 全ての行列を掛け合わせた行列を更新
	Graphics_DrawSetting_RefreshBlendTransformMatrix() ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTransformToView_PF( &GSYS.DrawSetting.ViewMatrixF ) ;

		GSYS.DrawSetting.MatchHardwareViewMatrix = TRUE ;
	}

	// 終了
	return 0 ;
}

// ビュー行列を取得する
extern	int NS_GetTransformToViewMatrix( MATRIX *MatBuf )
{
	ConvertMatrixDtoF( MatBuf, &GSYS.DrawSetting.ViewMatrix ) ;

	// 終了
	return 0 ;
}

// ビュー行列を取得する
extern	int NS_GetTransformToViewMatrixD( MATRIX_D *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.ViewMatrix ;

	// 終了
	return 0 ;
}

// 射影行列を設定する
extern int NS_SetTransformToProjection( const MATRIX *Matrix )
{
	return NS_SetupCamera_ProjectionMatrix( *Matrix ) ;
}

// 射影行列を設定する
extern int NS_SetTransformToProjectionD( const MATRIX_D *Matrix )
{
	return NS_SetupCamera_ProjectionMatrixD( *Matrix ) ;
}

// 射影行列を取得する
extern	int NS_GetTransformToProjectionMatrix( MATRIX *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.ProjectionMatrixF ;

	// 終了
	return 0 ;
}

// 射影行列を取得する
extern	int NS_GetTransformToProjectionMatrixD( MATRIX_D *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.ProjectionMatrix ;

	// 終了
	return 0 ;
}

// ビューポート行列をセットする
extern	int NS_SetTransformToViewport( const MATRIX *Matrix )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// データを保存
	GSYS.DrawSetting.ViewportMatrixF   = *Matrix ;
	ConvertMatrixFtoD( &GSYS.DrawSetting.ViewportMatrix, &GSYS.DrawSetting.ViewportMatrixF ) ;
	GSYS.DrawSetting.ValidBlendMatrix = FALSE ;

	// 全ての行列を掛け合わせた行列を更新
	Graphics_DrawSetting_RefreshBlendTransformMatrix() ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTransformToViewport_PF( &GSYS.DrawSetting.ViewportMatrixF ) ;

		GSYS.DrawSetting.MatchHardwareProjectionMatrix = TRUE ;
	}

	// 終了
	return 0 ;
}

// ビューポート行列をセットする
extern	int NS_SetTransformToViewportD( const MATRIX_D *Matrix )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// データを保存
	GSYS.DrawSetting.ViewportMatrix   = *Matrix ;
	ConvertMatrixDtoF( &GSYS.DrawSetting.ViewportMatrixF, &GSYS.DrawSetting.ViewportMatrix ) ;
	GSYS.DrawSetting.ValidBlendMatrix = FALSE ;

	// 全ての行列を掛け合わせた行列を更新
	Graphics_DrawSetting_RefreshBlendTransformMatrix() ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTransformToViewport_PF( &GSYS.DrawSetting.ViewportMatrixF ) ;

		GSYS.DrawSetting.MatchHardwareProjectionMatrix = TRUE ;
	}

	// 終了
	return 0 ;
}

// ビューポート行列を取得する
extern	int NS_GetTransformToViewportMatrix( MATRIX *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.ViewportMatrixF ;

	// 終了
	return 0 ;
}

// ビューポート行列を取得する
extern	int NS_GetTransformToViewportMatrixD( MATRIX_D *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.ViewportMatrix ;

	// 終了
	return 0 ;
}

// Direct3Dで自動適用されるビューポート行列を取得する
extern	int NS_GetTransformToAPIViewportMatrix( MATRIX *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.Direct3DViewportMatrixF ;

	// 終了
	return 0 ;
}

// Direct3Dで自動適用されるビューポート行列を取得する
extern	int NS_GetTransformToAPIViewportMatrixD( MATRIX_D *MatBuf )
{
	*MatBuf = GSYS.DrawSetting.Direct3DViewportMatrix ;

	// 終了
	return 0 ;
}

// デフォルトの変換行列をセットする
#define FOV				(60.0 * DX_PI / 180.0)
#define NEARZ			(0.0)
#define FARZ			(1000.0)
#define TAN_FOV_HALF	(0.52359877559829887307710723054658) // /*(0.57735026918962573)*/ tan( FOV * 0.5 )
extern	int NS_SetDefTransformMatrix( void )
{
	MATRIX_D mat ;
	double D ;
	
	// ワールド変換行列は単位行列
	CreateIdentityMatrixD( &mat ) ;
	NS_SetTransformToWorldD( &mat ) ;

	// ビューポート行列のセット
	CreateViewportMatrixD(
		&mat,
		( double )GSYS.DrawSetting.DrawSizeX / 2,
		( double )GSYS.DrawSetting.DrawSizeY / 2,
		( double )( GSYS.DrawSetting.DrawSizeX * GSYS.DrawSetting.Draw3DScale ),
		( double )( GSYS.DrawSetting.DrawSizeY * GSYS.DrawSetting.Draw3DScale )
	) ;
	NS_SetTransformToViewportD( &mat ) ;

	// ビュー行列は z = 0.0 の時に丁度スクリーン全体が写る位置と方向を持つカメラを
	{
		VECTOR_D up, at, eye ;
		int w, h ;

		w = GSYS.DrawSetting.DrawSizeX ;
		h = GSYS.DrawSetting.DrawSizeY ;
		D = ( double )( ( h / 2 ) / TAN_FOV_HALF ) ;
		eye.x = ( double )w / 2 ;
		eye.y = ( double )h / 2 ;
		eye.z = -D ;

		at.x = eye.x ;
		at.y = eye.y ;
		at.z = eye.z + 1.0 ;

		up.x = 0.0 ;
		up.y = 1.0 ;
		up.z = 0.0 ;

		CreateLookAtMatrixD( &mat, &eye, &at, &up ) ;
		NS_SetTransformToViewD( &mat ) ;
	}

	// プロジェクション行列は普通に
	CreatePerspectiveFovMatrixD( &mat, FOV, D * 0.1, D + FARZ ) ;
	NS_SetTransformToProjectionD( &mat ) ;

	// 終了
	return 0 ;
}

// ローカル座標から画面座標を取得する
extern	int NS_GetTransformPosition( VECTOR *LocalPos, float *x, float *y )
{
	float w ;
	VECTOR ScreenPos ;

	if( LocalPos == NULL ) return -1 ;

	// 座標変換
	w = 1.0F ;
	VectorTransform4(
		&ScreenPos, &w,
		LocalPos,   &w,
		&GSYS.DrawSetting.BlendMatrixF
	) ;
	w = 1.0F / w ;

	// Ｚ値が０以下の場合は正常なＸＹ値は導けないのでエラー
	if( ScreenPos.z * w < 0.0F )
	{
		return -1 ;
	}

	if( x != NULL )
	{
		*x = ScreenPos.x * w ;
	}

	if( y != NULL )
	{
		*y = ScreenPos.y * w ;
	}

	// 正常終了
	return 0 ;
}

// ローカル座標から画面座標を取得する
extern	int NS_GetTransformPositionD( VECTOR_D *LocalPos, double *x, double *y )
{
	double w ;
	VECTOR_D ScreenPos ;

	if( LocalPos == NULL ) return -1 ;

	// 座標変換
	w = 1.0 ;
	VectorTransform4D(
		&ScreenPos, &w,
		LocalPos,   &w,
		&GSYS.DrawSetting.BlendMatrix
	) ;
	w = 1.0 / w ;

	// Ｚ値が０以下の場合は正常なＸＹ値は導けないのでエラー
	if( ScreenPos.z * w < 0.0 )
	{
		return -1 ;
	}

	if( x != NULL )
	{
		*x = ScreenPos.x * w ;
	}

	if( y != NULL )
	{
		*y = ScreenPos.y * w ;
	}

	// 正常終了
	return 0 ;
}

// ワールド空間上のビルボードのサイズからスクリーンに投影した場合のピクセル単位のサイズを取得する
extern float NS_GetBillboardPixelSize( VECTOR WorldPos, float WorldSize )
{
	return ( float )NS_GetBillboardPixelSizeD( VConvFtoD( WorldPos ), WorldSize ) ;
}

// ワールド空間上のビルボードのサイズからスクリーンに投影した場合のピクセル単位のサイズを取得する
extern double NS_GetBillboardPixelSizeD( VECTOR_D WorldPos, double WorldSize )
{
	VECTOR_D ScreenPos, ScreenLeft, ScreenRightDirection ;

	ScreenPos            = NS_ConvWorldPosToScreenPosD( WorldPos ) ;
	ScreenRightDirection = VNormD( VGetD( GSYS.DrawSetting.ViewMatrix.m[ 0 ][ 0 ], GSYS.DrawSetting.ViewMatrix.m[ 1 ][ 0 ], GSYS.DrawSetting.ViewMatrix.m[ 2 ][ 0 ] ) ) ;
	ScreenLeft           = NS_ConvWorldPosToScreenPosD( VSubD( WorldPos, VScaleD( ScreenRightDirection, WorldSize / 2.0 ) ) ) ;
	return VSubD( ScreenPos, ScreenLeft ).x * 2.0 ;
}

// ワールド座標をカメラ座標に変換する
extern VECTOR NS_ConvWorldPosToViewPos( VECTOR WorldPos )
{
	return VConvDtoF( NS_ConvWorldPosToViewPosD( VConvFtoD( WorldPos ) ) ) ;
}

// ワールド座標をカメラ座標に変換する
extern VECTOR_D NS_ConvWorldPosToViewPosD( VECTOR_D WorldPos )
{
	VECTOR_D ViewPos ;

	VectorTransformD( &ViewPos, &WorldPos, &GSYS.DrawSetting.ViewMatrix ) ;

	return ViewPos ;
}

// ワールド座標をスクリーン座標に変換する
extern VECTOR NS_ConvWorldPosToScreenPos( VECTOR WorldPos )
{
	float w ;
	VECTOR ScreenPos ;

	// 座標変換
	w = 1.0F ;
	VectorTransform4( &ScreenPos, &w, &WorldPos, &w, &GSYS.DrawSetting.BlendMatrixF ) ;

	// 画面座標を返す
	w = 1.0F / w ;
	ScreenPos.x *= w ;
	ScreenPos.y *= w ;
	ScreenPos.z *= w ;

	return ScreenPos ;
}

// ワールド座標をスクリーン座標に変換する
extern VECTOR_D NS_ConvWorldPosToScreenPosD( VECTOR_D WorldPos )
{
	double   w ;
	VECTOR_D ScreenPos ;

	// 座標変換
	w = 1.0 ;
	VectorTransform4D( &ScreenPos, &w, &WorldPos, &w, &GSYS.DrawSetting.BlendMatrix ) ;

	// 画面座標を返す
	w = 1.0 / w ;
	ScreenPos.x *= w ;
	ScreenPos.y *= w ;
	ScreenPos.z *= w ;

	return ScreenPos ;
}

// ワールド座標をスクリーン座標に変換する、最後のＸＹＺ座標をＷで割る前の値を得る
extern FLOAT4 NS_ConvWorldPosToScreenPosPlusW( VECTOR WorldPos )
{
	VECTOR ScreenPos ;
	float w ;
	FLOAT4 Result ;

	// 座標変換
	w = 1.0f ;
	VectorTransform4( &ScreenPos, &w, &WorldPos, &w, &GSYS.DrawSetting.BlendMatrixF ) ;

	Result.x = ScreenPos.x ;
	Result.y = ScreenPos.y ;
	Result.z = ScreenPos.z ;
	Result.w = w ;

	return Result ;
}

// ワールド座標をスクリーン座標に変換する、最後のＸＹＺ座標をＷで割る前の値を得る
extern DOUBLE4 NS_ConvWorldPosToScreenPosPlusWD( VECTOR_D WorldPos )
{
	VECTOR_D ScreenPos ;
	double   w ;
	DOUBLE4 Result ;

	// 座標変換
	w = 1.0 ;
	VectorTransform4D( &ScreenPos, &w, &WorldPos, &w, &GSYS.DrawSetting.BlendMatrix ) ;

	Result.x = ScreenPos.x ;
	Result.y = ScreenPos.y ;
	Result.z = ScreenPos.z ;
	Result.w = w ;

	return Result ;
}

// スクリーン座標をワールド座標に変換する
extern VECTOR NS_ConvScreenPosToWorldPos( VECTOR ScreenPos )
{
	float w ;
	VECTOR WorldPos ;

	// ブレンド行列の逆行列が有効ではなかったら逆行列を構築する
	if( GSYS.DrawSetting.ValidInverseBlendMatrix == FALSE )
	{
		CreateInverseMatrixD( &GSYS.DrawSetting.InverseBlendMatrix, &GSYS.DrawSetting.BlendMatrix ) ;
		ConvertMatrixDtoF( &GSYS.DrawSetting.InverseBlendMatrixF, &GSYS.DrawSetting.InverseBlendMatrix ) ;
		GSYS.DrawSetting.ValidInverseBlendMatrix = TRUE ;
	}

	// 座標変換
	w = 1.0F ;
	VectorTransform4( &WorldPos, &w, &ScreenPos, &w, &GSYS.DrawSetting.InverseBlendMatrixF ) ;

	// ワールド座標を返す
	w = 1.0F / w ;
	WorldPos.x *= w ;
	WorldPos.y *= w ;
	WorldPos.z *= w ;
	return WorldPos ;
}

// スクリーン座標をワールド座標に変換する
extern VECTOR_D NS_ConvScreenPosToWorldPosD( VECTOR_D ScreenPos )
{
	double w ;
	VECTOR_D WorldPos ;

	// ブレンド行列の逆行列が有効ではなかったら逆行列を構築する
	if( GSYS.DrawSetting.ValidInverseBlendMatrix == FALSE )
	{
		CreateInverseMatrixD( &GSYS.DrawSetting.InverseBlendMatrix, &GSYS.DrawSetting.BlendMatrix ) ;
		ConvertMatrixDtoF( &GSYS.DrawSetting.InverseBlendMatrixF, &GSYS.DrawSetting.InverseBlendMatrix ) ;
		GSYS.DrawSetting.ValidInverseBlendMatrix = TRUE ;
	}

	// 座標変換
	w = 1.0 ;
	VectorTransform4D( &WorldPos, &w, &ScreenPos, &w, &GSYS.DrawSetting.InverseBlendMatrix ) ;

	// ワールド座標を返す
	w = 1.0 / w ;
	WorldPos.x *= w ;
	WorldPos.y *= w ;
	WorldPos.z *= w ;
	return WorldPos ;
}

// スクリーン座標をワールド座標に変換する( Z座標が線形 )
extern VECTOR NS_ConvScreenPosToWorldPos_ZLinear( VECTOR ScreenPos )
{
	return VConvDtoF( NS_ConvScreenPosToWorldPos_ZLinearD( VConvFtoD( ScreenPos ) ) ) ;
}

// スクリーン座標をワールド座標に変換する( Z座標が線形 )
extern VECTOR_D NS_ConvScreenPosToWorldPos_ZLinearD( VECTOR_D ScreenPos )
{
	double w, inz ;
	VECTOR_D WorldPos ;

	// ブレンド行列の逆行列が有効ではなかったら逆行列を構築する
	if( GSYS.DrawSetting.ValidInverseBlendMatrix == FALSE )
	{
		CreateInverseMatrixD( &GSYS.DrawSetting.InverseBlendMatrix, &GSYS.DrawSetting.BlendMatrix ) ;
		ConvertMatrixDtoF( &GSYS.DrawSetting.InverseBlendMatrixF, &GSYS.DrawSetting.InverseBlendMatrix ) ;
		GSYS.DrawSetting.ValidInverseBlendMatrix = TRUE ;
	}

	// スクリーン座標をワールド座標に変換する際に使用する行列が有効ではなかったら構築する
	if( GSYS.DrawSetting.ValidConvScreenPosToWorldPosMatrix == FALSE )
	{
		MATRIX_D TempMatrix ;

		CreateMultiplyMatrixD( &TempMatrix, &GSYS.DrawSetting.ProjectionMatrix, &GSYS.DrawSetting.ViewportMatrix ) ;
		CreateInverseMatrixD( &GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix, &TempMatrix ) ;
		GSYS.DrawSetting.ValidConvScreenPosToWorldPosMatrix = TRUE ;
	}

	// VectorTransform4 に渡すべき z 値を求める
	{
		double k1, k2, resz, testz, minz, maxz, sa, target, sikii_plus, sikii_minus ;
		int num ;

		k1 = ScreenPos.x * GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[0][2] + ScreenPos.y * GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[1][2] + GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[3][2] ;
		k2 = ScreenPos.x * GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[0][3] + ScreenPos.y * GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[1][3] + GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[3][3] ;
		target = ScreenPos.z * ( GSYS.DrawSetting.ProjFar - GSYS.DrawSetting.ProjNear ) + GSYS.DrawSetting.ProjNear ;
		testz = 0.5 ;
		sikii_plus  =  ( GSYS.DrawSetting.ProjFar - GSYS.DrawSetting.ProjNear ) / 100000.0 ;
		sikii_minus = -sikii_plus ;
		minz = 0.0 ;
		maxz = 1.0 ;
		for( num = 0 ; num < 20 ; num ++ )
		{
			resz = ( k1 + testz * GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[2][2] ) / ( k2 + testz * GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix.m[2][3] ) ;
			if( resz > target )
			{
				maxz = testz ;
			}
			else
			{
				minz = testz ;
			}
			sa = resz - target ;
			if( sa < 0.0 )
			{
				if( sa > sikii_minus ) break ;
			}
			else
			{
				if( sa < sikii_plus  ) break ;
			}
			testz = ( maxz + minz ) * 0.5 ;
		}
		inz = testz ;
	}

	// 座標変換
	w = 1.0 ;
	ScreenPos.z = inz ;
	VectorTransform4D( &WorldPos, &w, &ScreenPos, &w, &GSYS.DrawSetting.InverseBlendMatrix ) ;

	// ワールド座標を返す
	w = 1.0 / w ;
	WorldPos.x *= w ;
	WorldPos.y *= w ;
	WorldPos.z *= w ;
	return WorldPos ;
}



// ポリゴンカリングの有効、無効をセットする
extern int NS_SetUseCullingFlag( int Flag )
{
	if( GSYS.DrawSetting.CullMode == Flag ) return 0 ;

	// カリングモードのフラグを保存
	GSYS.DrawSetting.CullMode = Flag ;
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ポリゴンカリングの有効、無効をセットする
extern int NS_SetUseBackCulling( int Flag )
{
	return NS_SetUseCullingFlag( Flag ) ;
}

// ポリゴンカリングモードを取得する
extern int NS_GetUseBackCulling( void )
{
	return GSYS.DrawSetting.CullMode ;
}


// テクスチャアドレスモードを設定する
extern	int NS_SetTextureAddressMode( int Mode /* DX_TEXADDRESS_WRAP 等 */, int Stage )
{
	int i ;

	if( Stage == -1 )
	{
		for( i = 0 ; i < USE_TEXTURESTAGE_NUM ; i ++ )
		{
			if( GSYS.DrawSetting.TexAddressModeU[ i ] != Mode ||
				GSYS.DrawSetting.TexAddressModeV[ i ] != Mode ||
				GSYS.DrawSetting.TexAddressModeW[ i ] != Mode )
			{
				break ;
			}
		}

		if( i == USE_TEXTURESTAGE_NUM )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		for( i = 0 ; i < USE_TEXTURESTAGE_NUM ; i ++ )
		{
			GSYS.DrawSetting.TexAddressModeU[ i ] = Mode ;
			GSYS.DrawSetting.TexAddressModeV[ i ] = Mode ;
			GSYS.DrawSetting.TexAddressModeW[ i ] = Mode ;
		}

		GSYS.ChangeSettingFlag = TRUE ;
	}
	else
	{
		if( Stage < 0 || Stage >= USE_TEXTURESTAGE_NUM ) return 0 ;
		if( GSYS.DrawSetting.TexAddressModeU[ Stage ] == Mode &&
			GSYS.DrawSetting.TexAddressModeV[ Stage ] == Mode &&
			GSYS.DrawSetting.TexAddressModeW[ Stage ] == Mode ) return 0;

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		GSYS.DrawSetting.TexAddressModeU[ Stage ] = Mode ;
		GSYS.DrawSetting.TexAddressModeV[ Stage ] = Mode ;
		GSYS.DrawSetting.TexAddressModeW[ Stage ] = Mode ;
		GSYS.ChangeSettingFlag = TRUE ;
	}

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTextureAddressMode_PF( Mode, Stage ) ;
	}

	// 終了
	return 0 ;
}

// テクスチャアドレスモードを設定する
extern	int NS_SetTextureAddressModeUV( int ModeU, int ModeV, int Stage )
{
	int i ;

	if( Stage == -1 )
	{
		for( i = 0 ; i < USE_TEXTURESTAGE_NUM ; i ++ )
		{
			if( GSYS.DrawSetting.TexAddressModeU[ i ] != ModeU ||
				GSYS.DrawSetting.TexAddressModeV[ i ] != ModeV )
			{
				break ;
			}
		}

		if( i == USE_TEXTURESTAGE_NUM )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		for( i = 0 ; i < USE_TEXTURESTAGE_NUM ; i ++ )
		{
			GSYS.DrawSetting.TexAddressModeU[ i ] = ModeU ;
			GSYS.DrawSetting.TexAddressModeV[ i ] = ModeV ;
		}

		GSYS.ChangeSettingFlag = TRUE ;
	}
	else
	{
		if( Stage < 0 || Stage >= USE_TEXTURESTAGE_NUM ) return 0 ;
		if( GSYS.DrawSetting.TexAddressModeU[ Stage ] == ModeU &&
			GSYS.DrawSetting.TexAddressModeV[ Stage ] == ModeV ) return 0;

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		GSYS.DrawSetting.TexAddressModeU[ Stage ] = ModeU ;
		GSYS.DrawSetting.TexAddressModeV[ Stage ] = ModeV ;
		GSYS.ChangeSettingFlag = TRUE ;
	}

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTextureAddressModeUV_PF( ModeU, ModeV, Stage ) ;
	}

	// 終了
	return 0 ;
}

// テクスチャ座標変換パラメータをセットする
extern int NS_SetTextureAddressTransform( float TransU, float TransV, float ScaleU, float ScaleV, float RotCenterU, float RotCenterV, float Rotate )
{
	MATRIX Transform, Temp1, Temp2, Temp3 ;
	int UseFlag ;

	UseFlag = FALSE ;

	if( Rotate != 0.0f )
	{
		CreateTranslationMatrix( &Temp1, -RotCenterU, -RotCenterV, 0.0f ) ; 
		CreateRotationZMatrix( &Temp2, Rotate ) ;
		CreateTranslationMatrix( &Temp3,  RotCenterU,  RotCenterV, 0.0f ) ; 
		CreateMultiplyMatrix( &Transform, &Temp1, &Temp2 ) ;
		CreateMultiplyMatrix( &Transform, &Transform, &Temp3 ) ;
		UseFlag = TRUE ;
	}
	else
	{
		CreateIdentityMatrix( &Transform ) ;
	}

	if( TransU != 0.0f || TransV != 0.0f )
	{
		CreateTranslationMatrix( &Temp1, TransU, TransV, 0.0f ) ;
		CreateMultiplyMatrix( &Transform, &Transform, &Temp1 ) ;
		UseFlag = TRUE ;
	}

	if( ScaleU != 1.0f || ScaleV != 1.0f )
	{
		CreateScalingMatrix( &Temp1, ScaleU, ScaleV, 1.0f ) ;
		CreateMultiplyMatrix( &Transform, &Transform, &Temp1 ) ;
		UseFlag = TRUE ;
	}

	// 元から使用しない場合は何もしない
	if( GSYS.DrawSetting.TextureAddressTransformUse == UseFlag && UseFlag == FALSE )
		return 0 ;

	// 設定を保存
	GSYS.DrawSetting.TextureAddressTransformUse = UseFlag ;
	GSYS.DrawSetting.TextureAddressTransformMatrix = Transform ;
//	GSYS.DrawSetting.TextureMatrixValid = FALSE ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTextureAddressTransformMatrix_PF( UseFlag, &Transform ) ;

		GSYS.DrawSetting.MatchHardwareTextureAddressTransformMatrix = TRUE ;
	}

	// 正常終了
	return 0 ;
}

// テクスチャ座標変換行列をセットする
extern int NS_SetTextureAddressTransformMatrix( MATRIX Matrix )
{
	// 設定を保存
	GSYS.DrawSetting.TextureAddressTransformUse    = TRUE ;
//	GSYS.DrawSetting.TextureMatrixValid     = TRUE ;
//	GSYS.TextureMatrix          = Matrix ;
	GSYS.DrawSetting.TextureAddressTransformMatrix = Matrix ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTextureAddressTransformMatrix_PF( TRUE, &Matrix ) ;

		GSYS.DrawSetting.MatchHardwareTextureAddressTransformMatrix = TRUE ;
	}

	// 正常終了
	return 0 ;
}

// テクスチャ座標変換パラメータをリセットする
extern int NS_ResetTextureAddressTransform( void )
{
	return NS_SetTextureAddressTransform( 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f ) ;
}

// フォグを有効にするかどうかを設定する( TRUE:有効  FALSE:無効 )
extern int NS_SetFogEnable( int Flag )
{
	if( GSYS.DrawSetting.FogEnable == Flag ) return 0;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.FogEnable = Flag ;
	GSYS.ChangeSettingFlag     = TRUE ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetFogEnable_PF( Flag ) ;
	}

	// 終了
	return 0 ;
}

// フォグが有効かどうかを取得する( TRUE:有効  FALSE:無効 )
extern int NS_GetFogEnable( void )
{
	return GSYS.DrawSetting.FogEnable ;
}

// フォグモードを設定する
extern	int NS_SetFogMode( int Mode /* DX_FOGMODE_NONE 等 */ )
{
	if( GSYS.DrawSetting.FogMode == Mode ) return 0;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.FogMode = Mode ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetFogMode_PF( Mode ) ;
	}

	// 終了
	return 0 ;
}

// フォグモードを取得する
extern int NS_GetFogMode( void )
{
	return GSYS.DrawSetting.FogMode ;
}

// フォグカラーを変更する
extern	int NS_SetFogColor( int r, int g, int b )
{
	DWORD color = ( DWORD )( ( ( 0xff ) << 24 ) | ( ( r & 0xff ) << 16 ) | ( ( g & 0xff ) << 8 ) | ( b & 0xff ) ) ;

	if( GSYS.DrawSetting.FogColor == color ) return 0;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.FogColor = color ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetFogColor_PF( color ) ;
	}

	// 終了
	return 0 ;
}

// フォグカラーを取得する
extern	int NS_GetFogColor( int *r, int *g, int *b )
{
	if( r != NULL )
	{
		*r = ( int )( ( GSYS.DrawSetting.FogColor >> 16 ) & 0xff ) ;
	}

	if( g != NULL )
	{
		*g = ( int )( ( GSYS.DrawSetting.FogColor >> 8  ) & 0xff ) ;
	}

	if( b != NULL )
	{
		*b = ( int )( ( GSYS.DrawSetting.FogColor       ) & 0xff ) ;
	}

	return 0 ;
}

// フォグが始まる距離と終了する距離を設定する( 0.0f 〜 1.0f )
extern	int	NS_SetFogStartEnd( float start, float end )
{
	if( GSYS.DrawSetting.FogStart == start && GSYS.DrawSetting.FogEnd == end ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.FogStart = start ;
	GSYS.DrawSetting.FogEnd = end ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetFogStartEnd_PF( start, end ) ;
	}

	// 終了
	return 0 ;
}

// フォグが始まる距離と終了する距離を取得する( 0.0f 〜 1.0f )
extern	int NS_GetFogStartEnd( float *start, float *end )
{
	if( start != NULL )
	{
		*start = GSYS.DrawSetting.FogStart ;
	}

	if( end != NULL )
	{
		*end = GSYS.DrawSetting.FogEnd ;
	}

	return 0 ;
}

// フォグの密度を設定する( 0.0f 〜 1.0f )
extern	int	NS_SetFogDensity( float density )
{
	if( GSYS.DrawSetting.FogDensity == density ) return 0;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	GSYS.DrawSetting.FogDensity = density ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetFogDensity_PF( density ) ;
	}

	// 終了
	return 0 ;
}

// フォグの密度を取得する( 0.0f 〜 1.0f )
extern float NS_GetFogDensity( void )
{
	return GSYS.DrawSetting.FogDensity ;
}


























// 画面関係関数

// 指定座標の色を取得する
extern unsigned int NS_GetPixel( int x, int y )
{
	unsigned int Ret = 0xffffffff ;

	// 描画座標チェック
	if( x < 0 || y < 0 || x >= GSYS.DrawSetting.DrawSizeX || y >= GSYS.DrawSetting.DrawSizeY ) return 0xffffffff ;

	// ハードウエア機能を使用するかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		// ハードウェアを使用する場合

		Ret = Graphics_Hardware_GetPixel_PF( x, y ) ;
	}
	else
	{
		MEMIMG    *Img ;
		IMAGEDATA *Image ;

		// ハードウェアを使用しない場合

		// エラー判定
		if( GRAPHCHKFULL( GSYS.DrawSetting.TargetScreen[ 0 ], Image ) )
			Image = NULL ;

		// 描画先によって処理を分岐
		if( Image )
		{
			Img = &Image->Soft.MemImg ;
		}
		else
		{
			Img = &GSYS.SoftRender.MainBufferMemImg ;
		}

		switch( GSYS.Screen.MainScreenColorBitDepth )
		{
		case 16 : Ret = *( (  WORD * )( ( char * )Img->UseImage + 2 * x + Img->Base->Pitch * y ) ) ; break ;
		case 32 : Ret = *( ( DWORD * )( ( char * )Img->UseImage + 4 * x + Img->Base->Pitch * y ) ) ; break ;
		}
	}

	// 終了
	return Ret ;
}

// メインウインドウのバックグラウンドカラーを設定する
extern int NS_SetBackgroundColor( int Red, int Green, int Blue )
{
	// 色の保存
	GSYS.Screen.BackgroundRed   = Red ;
	GSYS.Screen.BackgroundGreen = Green ;
	GSYS.Screen.BackgroundBlue  = Blue ;

	// バックグラウンドカラーが有効かどうかのフラグを立てる
	GSYS.Screen.EnableBackgroundColor = TRUE ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetBackgroundColor_PF( Red, Green, Blue ) ;
	}

	// 終了
	return 0;
}

// アクティブになっている画面から指定領域のグラフィックを取得する
extern	int NS_GetDrawScreenGraph( int x1, int y1, int x2, int y2, int GrHandle, int /*UseClientFlag*/ )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	// 取り込みサイズと画像サイズが違う場合はエラー
	if( Image->Width != x2 - x1 || Image->Height != y2 - y1 ) return -1 ;

	return Graphics_Image_GetDrawScreenGraphBase( GSYS.DrawSetting.TargetScreen[ 0 ], GSYS.DrawSetting.TargetScreenSurface[ 0 ], x1, y1, x2, y2, 0, 0, GrHandle ) ;
}

// 描画可能画像から指定領域のグラフィックを取得する
extern int NS_BltDrawValidGraph( int TargetDrawValidGrHandle, int x1, int y1, int x2, int y2, int DestX, int DestY, int DestGrHandle )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHK( TargetDrawValidGrHandle, Image ) )
		return -1 ;

	return Graphics_Image_GetDrawScreenGraphBase( TargetDrawValidGrHandle, 0, x1, y1, x2, y2, DestX, DestY, DestGrHandle ) ;
}

// 裏画面と表画面を交換する
extern int NS_ScreenFlip( void )
{
	return Graphics_Screen_FlipBase() ;
}

// ScreenCopy のベース関数
extern int Graphics_Screen_ScreenCopyBase( int DrawTargetFrontScreenMode_Copy )
{
	if( GSYS.Setting.ValidHardware == TRUE )
	{
		Graphics_Hardware_ScreenCopy_PF( DrawTargetFrontScreenMode_Copy ) ;
	}

	// フリップ
	return Graphics_Screen_FlipBase() ;
}

// 裏画面の内容を表画面に描画する
extern int NS_ScreenCopy( void )
{
	return Graphics_Screen_ScreenCopyBase( FALSE ) ;
}

// 垂直同期信号を待つ
extern	int		NS_WaitVSync( int SyncNum )
{
	return Graphics_Hardware_WaitVSync_PF( SyncNum ) ;
}

// 画面の状態を初期化する
extern	int NS_ClearDrawScreen( const RECT *ClearRect )
{
//	HRESULT hr ;
//	RECT Rect ;

	// フルスクリーンマスク処理を行っている場合はマスク描画の結果を反映させる
	MASK_FULLSCREEN_MASKUPDATE

	// チェック
	if( ClearRect && ( ClearRect->left < 0 || ClearRect->top < 0 ) )
		ClearRect = NULL ;

	// ハードウエア機能を使っているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		// ハードウエアの機能を使っている場合
		Graphics_Hardware_ClearDrawScreen_PF( ClearRect ) ;
	}
	else
	{
		IMAGEDATA *Image = NULL ;
		MEMIMG    *ClearTarget ;

		// ソフトウエアレンダリングの場合

		// 描画先画像の画像情報を取得
		GRAPHCHKFULL( GSYS.DrawSetting.TargetScreen[ 0 ], Image ) ;

		// 描画可能画像の場合はその画像をクリア
		ClearTarget = Image ? &Image->Soft.MemImg : &GSYS.SoftRender.MainBufferMemImg ;
		ClearMemImg( ClearTarget, ClearRect, NS_GetColor3( ClearTarget->Base->ColorDataP, GSYS.Screen.BackgroundRed, GSYS.Screen.BackgroundGreen, GSYS.Screen.BackgroundBlue, 0 ) ) ;
	}

	// 終了
	return 0 ;
}

// 画面のＺバッファの状態を初期化する
extern int NS_ClearDrawScreenZBuffer( const RECT *ClearRect )
{
	// チェック
	if( ClearRect && ( ClearRect->left < 0 || ClearRect->top < 0 ) )
		ClearRect = NULL ;

	// ハードウエア機能を使っているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		// ハードウエアの機能を使っている場合
		Graphics_Hardware_ClearDrawScreenZBuffer_PF( ClearRect ) ;
	}
	else
	{
		// ソフトウエアレンダリングの場合は何もしない
	}

	// 終了
	return 0 ;
}

// 画面の状態を初期化する(ClearDrawScreenの旧名称)
extern int NS_ClsDrawScreen( void )
{
	return NS_ClearDrawScreen() ;
}

// 描画先画面のセット
extern	int NS_SetDrawScreen( int DrawScreen )
{
	int OldScreen ;
	int OldScreenSurface ;
#ifndef DX_NON_MASK
	int MaskUseFlag ;
#endif
//	int Result ;
	IMAGEDATA     *Image        = NULL ;
	IMAGEDATA     *OldImage     = NULL ;
	SHADOWMAPDATA *ShadowMap    = NULL ;
	SHADOWMAPDATA *OldShadowMap = NULL ;
	
	if( GSYS.InitializeFlag == FALSE ) return -1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// グラフィック描画先が今までと同じだった場合何もせず終了
//	if( DrawScreen == GSYS.DrawSetting.TargetScreen[ 0 ] && GSYS.DrawSetting.TargetScreenSurface == 0 ) return 0 ;

	// 画像情報の取得
	if( GSYS.DrawSetting.ShadowMapDrawSetupRequest )
	{
		// アドレスの取得
		if( SHADOWMAPCHK( DrawScreen, ShadowMap ) )
			return -1 ;
	}
	else
	{
		Image = NULL ;
		if( !GRAPHCHKFULL( DrawScreen, Image ) )
		{
			// 画像だった場合は描画可能では無い場合はエラー
			if( Image->Orig->FormatDesc.DrawValidFlag == FALSE )
				return -1 ;
		}
		else
		{
			if( Image != NULL )
			{
				return -1 ;
			}
		}
	}

	// 今までの描画先の画像情報の取得
	if( GRAPHCHKFULL( GSYS.DrawSetting.TargetScreen[ 0 ], OldImage ) )
	{
		OldImage = NULL ;
		if( SHADOWMAPCHKFULL( GSYS.DrawSetting.TargetScreen[ 0 ], OldShadowMap ) )
		{
			OldShadowMap = NULL ;
		}
	}

	// 指定したスクリーンが無効だったら何もせず終了
	if( DrawScreen != DX_SCREEN_FRONT && DrawScreen != DX_SCREEN_BACK &&
		DrawScreen != DX_SCREEN_WORK  && DrawScreen != DX_SCREEN_TEMPFRONT &&
		ShadowMap == NULL &&
		( ( DrawScreen & DX_HANDLEERROR_MASK ) != 0                   ||
		  ( DrawScreen & DX_HANDLETYPE_MASK  ) != DX_HANDLETYPE_MASK_GRAPH || 
		  GetHandleInfo( DrawScreen ) == NULL ) )
	{
		return -1 ;
	}

	// 今までの画面が表画面だった場合は一度 ScreenCopy を行う
	if( GSYS.DrawSetting.TargetScreen[ 0 ] == DX_SCREEN_FRONT )
	{
		Graphics_Screen_ScreenCopyBase( TRUE ) ;
	}

	// シャドウマップへの描画設定がされていたら解除
	if( GSYS.DrawSetting.ShadowMapDraw )
	{
		NS_ShadowMap_DrawEnd() ;
	}

#ifndef DX_NON_MASK
	// マスクを使用終了する手続きを取る
	MaskUseFlag = MASKD.MaskUseFlag ;
	NS_SetUseMaskScreenFlag( FALSE ) ;

	// 描画先のサイズでマスク画面を作成しなおす
	if( GSYS.DrawSetting.ShadowMapDrawSetupRequest == FALSE && MASKD.CreateMaskFlag )
	{
		int w, h ;
		NS_GetGraphSize( DrawScreen, &w, &h ) ;
		Mask_CreateScreenFunction( TRUE, w, h ) ;
	}
#endif

	// 描画先を保存
	OldScreen                                 = GSYS.DrawSetting.TargetScreen[ 0 ] ;
	OldScreenSurface                          = GSYS.DrawSetting.TargetScreenSurface[ 0 ] ;
	GSYS.DrawSetting.TargetScreen[ 0 ]        = DrawScreen ;
	GSYS.DrawSetting.TargetScreenSurface[ 0 ] = 0 ;

	// 描画可能サイズの取得
	NS_GetGraphSize( DrawScreen, &GSYS.DrawSetting.DrawSizeX, &GSYS.DrawSetting.DrawSizeY ) ;

	// 描画先に正しいα値を書き込むかどうかのフラグを更新する
	Graphics_DrawSetting_RefreshAlphaChDrawMode() ;

	// ３Ｄ描画関係の描画先をセット
	if( GSYS.Setting.ValidHardware )
	{
		// ハードウエアアクセラレータを使用する場合の処理
		Graphics_Hardware_SetDrawScreen_PF( DrawScreen, OldScreenSurface, Image, OldImage, ShadowMap, OldShadowMap ) ;
	}
	else
	{
		// ソフトウエアレンダリングの場合の処理

		// 描画先の決定
#ifndef DX_NON_MASK
		if( MASKD.MaskValidFlag )
		{
			// マスクが有効な場合はマスクを描画先にする
			GSYS.SoftRender.TargetMemImg = &MASKD.MaskDrawMemImg ;
		}
		else
#endif
		{
			// 画像が描画対象かどうかで処理を分岐
			if( Image != NULL )
			{
				// アルファチャンネル付きの画像の場合は描画対象にできない
				if( Image->Soft.MemImg.Base->UseAlpha == TRUE )
					return -1 ;

				// 描画可能画像を描画先に
				GSYS.SoftRender.TargetMemImg = &Image->Soft.MemImg ;
			}
			else
			{
				// メインバッファを描画先に
				GSYS.SoftRender.TargetMemImg = &GSYS.SoftRender.MainBufferMemImg ;
			}
		}
	}

	// ２Ｄ描画用の行列を作成
//	{
//		CreateViewportMatrix(
//			&GSYS.DrawSetting.ViewportMatrix2D,
//			GSYS.DrawSetting.DrawSizeX / 2.0f,
//			GSYS.DrawSetting.DrawSizeY / 2.0f, 
//			( float )GSYS.DrawSetting.DrawSizeX,
//			( float )GSYS.DrawSetting.DrawSizeY
//		) ;
//
//		CreateIdentityMatrix( &GSYS.DrawSetting.ProjectionMatrix2D ) ;
//		GSYS.DrawSetting.ProjectionMatrix2D.m[ 0 ][ 0 ] =  1.0f / ( ( float )GSYS.DrawSetting.DrawSizeX / 2.0f ) ;
//		GSYS.DrawSetting.ProjectionMatrix2D.m[ 1 ][ 1 ] = -1.0f / ( ( float )GSYS.DrawSetting.DrawSizeY / 2.0f ) ;
//		GSYS.DrawSetting.ProjectionMatrix2D.m[ 3 ][ 0 ] = -1.0f ;
//		GSYS.DrawSetting.ProjectionMatrix2D.m[ 3 ][ 1 ] =  1.0f ;
//	}

	// 描画領域を更新
	{
//		RECT SrcRect, ClippuRect ;
		RECT SrcRect ;
		int NewWidth, NewHeight, OldWidth, OldHeight ;

		NS_GetGraphSize( DrawScreen, &NewWidth, &NewHeight ) ;
		NS_GetGraphSize( OldScreen,  &OldWidth, &OldHeight ) ;

		SrcRect = GSYS.DrawSetting.DrawArea ;

//		SETRECT( ClippuRect, 0, 0, NewWidth, NewHeight ) ;
//		RectClipping( &SrcRect, &ClippuRect ) ;

//		if( NewWidth != GSYS.DrawSetting.DrawArea.right || NewHeight != GSYS.DrawSetting.DrawArea.bottom )
		if( GSYS.DrawSetting.ShadowMapDrawSetupRequest || ( NewWidth != OldWidth ) || ( NewHeight != OldHeight ) )
		{
			NS_SetDrawArea( 0, 0, NewWidth, NewHeight ) ;
		}
		else
		{
			NS_SetDrawArea( SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom ) ;
		}
	}
		
	// ３Ｄ描画処理用の行列設定の初期化を行う
	{
		MATRIX mat ;
		float D ;

		// ドットアスペクトを１．０に戻す
		NS_SetCameraDotAspect( 1.0f ) ;

		// ワールド変換行列は単位行列
		CreateIdentityMatrix( &mat ) ;
		NS_SetTransformToWorld( &mat ) ;

		// ライブラリ管理のビューポート行列を更新
		GSYS.Camera.ScreenCenterX = ( double )GSYS.DrawSetting.DrawSizeX / 2.0 ;
		GSYS.Camera.ScreenCenterY = ( double )GSYS.DrawSetting.DrawSizeY / 2.0 ;
		NS_SetCameraScreenCenterD( GSYS.Camera.ScreenCenterX, GSYS.Camera.ScreenCenterY ) ;

		// ビュー行列は z = 0.0 の時に丁度スクリーン全体が写る位置と方向を持つカメラを
		{
			VECTOR up, at, eye ;

			D = (float)( ( GSYS.DrawSetting.DrawSizeY / 2 ) / DEFAULT_TAN_FOV_HALF ) ;

			eye.x = (float)GSYS.DrawSetting.DrawSizeX / 2 ;
			eye.y = (float)GSYS.DrawSetting.DrawSizeY / 2 ;
			eye.z = -D;
			
			at.x = eye.x ;
			at.y = eye.y ;
			at.z = eye.z + 1.0F ;
			
			up.x = 0.0f ;
			up.y = 1.0F ;
			up.z = 0.0f ;
			
			NS_SetCameraPositionAndTargetAndUpVec( eye, at, up ) ;
//				CreateLookAtMatrix( &mat, &eye, &at, &up ) ;
//				NS_SetTransformToView( &mat ) ;
		}

		// プロジェクション行列は普通に
		NS_SetupCamera_Perspective( DEFAULT_FOV ) ;
		NS_SetCameraNearFar( D * 0.1f + DEFAULT_NEAR, D + DEFAULT_FAR ) ;
	}

#ifndef DX_NON_MASK
	// マスクを使用する手続きを取る
	NS_SetUseMaskScreenFlag( MaskUseFlag ) ;
#endif

	// シャドウマップに対する描画準備リクエストだった場合はリクエストフラグを倒し、描画準備状態を保存する
	if( GSYS.DrawSetting.ShadowMapDrawSetupRequest )
	{
		GSYS.DrawSetting.ShadowMapDrawSetupRequest = FALSE ;

		// シャドウマップに対する描画であるかどうかのフラグを立てる
		GSYS.DrawSetting.ShadowMapDraw = TRUE ;

		// シャドウマップに対する描画の場合の、対象となるシャドウマップのハンドルを保存
		GSYS.DrawSetting.ShadowMapDrawHandle = DrawScreen ;
	}

	// SetDrawScreen の後に行う環境依存処理を実行する
	Graphics_Hardware_SetDrawScreen_Post_PF( DrawScreen ) ;

	// 終了
	return 0 ;
}

// 描画先画面の取得
extern int NS_GetDrawScreen( void )
{
	return GSYS.DrawSetting.TargetScreen[ 0 ] ;
}

// アクティブになっているグラフィックのハンドルを得る
extern int NS_GetActiveGraph( void )
{
	return NS_GetDrawScreen() ;
}

// 描画先Ｚバッファのセット
extern	int	NS_SetDrawZBuffer( int DrawScreen )
{
	IMAGEDATA *Image ;
	
	if( GSYS.InitializeFlag == FALSE ) return -1 ;

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// 画像情報の取得
	Image = NULL ;
	GRAPHCHKFULL( DrawScreen, Image ) ;

	// 環境依存処理
	if( Graphics_Hardware_SetDrawZBuffer_PF( DrawScreen, Image ) < 0 )
	{
		return -1 ;
	}

	// 描画先Ｚバッファのハンドルを保存
	GSYS.DrawSetting.TargetZBuffer = DrawScreen ;

	// 描画先Ｚバッファを更新
	Graphics_Screen_SetupUseZBuffer() ;

	// 終了
	return 0 ;
}

// 画面モードのセット
extern int NS_SetGraphMode( int ScreenSizeX, int ScreenSizeY, int ColorBitDepth, int RefreshRate )
{
	int Width ;
	int Height ;
	int CDepth ;
	int RRate ;
//	int MaxScreenSizeX ;
//	int MaxScreenSizeY ;

	if( ColorBitDepth == 0 )
	{
		ColorBitDepth = DEFAULT_COLOR_BITDEPTH ;
	}

	// ３２０×２４０の画面をエミュレーションするフラグが立っている場合は強制的に解像度を３２０×２４０にする
	if( GSYS.Screen.Emulation320x240Flag )
	{
		ScreenSizeX = 320 ;
		ScreenSizeY = 240 ;
	}

	// エラーチェック
	if( ScreenSizeX <= 0 ||
		ScreenSizeY <= 0 ||
		( ColorBitDepth != 0 && ColorBitDepth != 8 && ColorBitDepth != 16 && ColorBitDepth != 32 ) )
	{
		return DX_CHANGESCREEN_RETURN ;
	}

	// モニタが対応している解像度では無く、且つモニタが対応している最大解像度よりも大きい解像度が指定された場合はエラー
//	if( Graphics_Screen_CheckDisplaySetting( ScreenSizeX, ScreenSizeY, ColorBitDepth ) == FALSE )
//	{
//		NS_GetDisplayMaxResolution( &MaxScreenSizeX, &MaxScreenSizeY ) ;
//		if( MaxScreenSizeX < ScreenSizeX || MaxScreenSizeY < ScreenSizeY )
//		{
//			return -1 ;
//		}
//	}

	// 初期化前の場合は設定値だけ保存する
	if( DxSysData.DxLib_InitializeFlag == FALSE )
	{
		Graphics_Screen_SetMainScreenSize( ScreenSizeX, ScreenSizeY ) ;
		GSYS.Screen.MainScreenColorBitDepth = ColorBitDepth ;
		GSYS.Screen.MainScreenRefreshRate   = RefreshRate ;
		SetMemImgDefaultColorType( ColorBitDepth == 32 ? 1 : 0 ) ;
		return 0 ;
	}

	// 元の値を取っておく
	Width	= GSYS.Screen.MainScreenSizeX ;
	Height	= GSYS.Screen.MainScreenSizeX ;
	CDepth	= GSYS.Screen.MainScreenColorBitDepth ;
	RRate	= GSYS.Screen.MainScreenRefreshRate ;

	// 画面モードを変更
	Graphics_Screen_ChangeMode( ScreenSizeX, ScreenSizeY, ColorBitDepth, FALSE, RefreshRate ) ;

	// 戻り値のセット
	if( ScreenSizeX == GSYS.Screen.MainScreenSizeX &&
		ScreenSizeY == GSYS.Screen.MainScreenSizeY 
#ifdef __WINDOWS__
		&&
		(
		  NS_GetWindowModeFlag() == FALSE ||
		  (
		    NS_GetWindowModeFlag() == TRUE &&
		    ColorBitDepth	== GSYS.Screen.MainScreenColorBitDepth &&
		    RefreshRate		== GSYS.Screen.MainScreenRefreshRate
		  )
		)
#endif // __WINDOWS__
	  )
	{
		return DX_CHANGESCREEN_OK ;
	}
	else
	{
		if( Width	== GSYS.Screen.MainScreenSizeX &&
			Height	== GSYS.Screen.MainScreenSizeY &&
			CDepth	== GSYS.Screen.MainScreenColorBitDepth &&
			RRate	== GSYS.Screen.MainScreenRefreshRate )
		{
			return DX_CHANGESCREEN_RETURN ;
		}
		else
		{
			if( GSYS.Screen.MainScreenSizeX == DEFAULT_SCREEN_SIZE_X && 
				GSYS.Screen.MainScreenSizeY == DEFAULT_SCREEN_SIZE_Y 
#ifdef __WINDOWS__
				&& 
				(
				  NS_GetWindowModeFlag() == FALSE ||
				  (
					NS_GetWindowModeFlag() == TRUE &&
				    GSYS.Screen.MainScreenColorBitDepth == DEFAULT_COLOR_BITDEPTH &&
				    GSYS.Screen.MainScreenRefreshRate   == 0
				  )
				)
#endif // __WINDOWS__
			  )
			{
				return DX_CHANGESCREEN_DEFAULT ;
			}
			else
			{
				return -1 ;
			}
		}
	}
}

// フルスクリーン解像度モードを設定する
extern int NS_SetFullScreenResolutionMode( int ResolutionMode /* DX_FSRESOLUTIONMODE_NATIVE 等 */ )
{
	// 初期後の場合は何もせず終了
	if( DxSysData.DxLib_InitializeFlag == TRUE )
	{
		return -1 ;
	}

	// フルスクリーン解像度モードを保存する
	GSYS.Screen.FullScreenResolutionMode    = ResolutionMode ;
	GSYS.Screen.FullScreenResolutionModeAct = ResolutionMode ;

	// 終了
	return 0 ;
}

// フルスクリーンモード時の画面拡大モードを設定する
extern int NS_SetFullScreenScalingMode( int ScalingMode /* DX_FSSCALINGMODE_NEAREST 等 */ )
{
	// フルスクリーンスケーリングモードを保存する
	GSYS.Screen.FullScreenScalingMode = ScalingMode ;

	// 終了
	return 0 ;
}

// ６４０ｘ４８０の画面で３２０ｘ２４０の画面解像度にするかどうかのフラグをセットする、６４０ｘ４８０以外の解像度では無効( TRUE:有効  FALSE:無効 )
extern int NS_SetEmulation320x240( int Flag )
{
	// 初期後の場合は何もせず終了
	if( DxSysData.DxLib_InitializeFlag == TRUE )
	{
		return -1 ;
	}

	// フラグを保存
	GSYS.Screen.Emulation320x240Flag = Flag;

	// フラグが TRUE の場合は画面モードを 320x240 に設定する
	if( Flag != FALSE )
	{
		NS_SetGraphMode( 320, 240, GSYS.Screen.MainScreenColorBitDepth, GSYS.Screen.MainScreenRefreshRate ) ;
	}

	// 終了
	return 0;
}

// 画面用のＺバッファのサイズを設定する
extern int NS_SetZBufferSize( int ZBufferSizeX, int ZBufferSizeY )
{
	return Graphics_Screen_SetZBufferMode( ZBufferSizeX, ZBufferSizeY, -1 ) ;
}

// 画面用のＺバッファのビット深度を設定する( 16 or 24 or 32 )
extern int NS_SetZBufferBitDepth( int BitDepth )
{
	return Graphics_Screen_SetZBufferMode( -1, -1, BitDepth ) ;
}

// ＶＳＹＮＣ待ちをするかのフラグセット
extern int NS_SetWaitVSyncFlag( int Flag )
{
	// 初期後の場合は何もせず終了
	if( DxSysData.DxLib_InitializeFlag == TRUE )
	{
		return -1 ;
	}

	GSYS.Screen.NotWaitVSyncFlag = !Flag ;

//	if( DxSysData.DxLib_InitializeFlag == FALSE )
//	{
//		GSYS.Screen.PreSetWaitVSyncFlag = Flag ;
//	}

	return 0 ;
}

// ＶＳＹＮＣ待ちをする設定になっているかどうかを取得する
extern int NS_GetWaitVSyncFlag( void )
{
	return !GSYS.Screen.NotWaitVSyncFlag ;
}

// 画面のフルスクリーンアンチエイリアスモードの設定をする
extern int NS_SetFullSceneAntiAliasingMode( int Samples, int Quality )
{
	return Graphics_Hardware_SetFullSceneAntiAliasingMode_PF( Samples, Quality ) ;
}

// ScreenFlip 時に表画面全体に転送する裏画面の領域を設定する( DxLib_Init の前でのみ使用可能 )
extern int NS_SetGraphDisplayArea( int x1, int y1, int x2, int y2 )
{
	// 初期後の場合は何もせず終了
	if( DxSysData.DxLib_InitializeFlag == TRUE )
		return -1 ;

	// パラメータチェック
	if( x1 >= x2 || y1 >= y2 )
	{
		GSYS.Screen.ValidGraphDisplayArea = FALSE ;
		return -1 ;
	}

	// パラメータ保存
	GSYS.Screen.GraphDisplayArea.left   = x1 ;
	GSYS.Screen.GraphDisplayArea.right  = x2 ;
	GSYS.Screen.GraphDisplayArea.top    = y1 ;
	GSYS.Screen.GraphDisplayArea.bottom = y2 ;

	GSYS.Screen.ValidGraphDisplayArea   = TRUE ;

	// 終了
	return 0 ;
}

// 画面モード変更時( とウインドウモード変更時 )にグラフィックスシステムの設定やグラフィックハンドルをリセットするかどうかを設定する( TRUE:リセットする  FALSE:リセットしない )
extern int NS_SetChangeScreenModeGraphicsSystemResetFlag( int Flag )
{
	GSYS.Setting.ChangeScreenModeNotGraphicsSystemFlag = Flag ? FALSE : TRUE ;

	return 0 ;
}

// 現在の画面の大きさとカラービット数を得る 
extern	int NS_GetScreenState( int *SizeX, int *SizeY, int *ColorBitDepth )
{
	if( SizeX ) *SizeX = GSYS.Screen.MainScreenSizeX ;
	if( SizeY ) *SizeY = GSYS.Screen.MainScreenSizeY ;

	if( ColorBitDepth ) *ColorBitDepth = GSYS.Screen.MainScreenColorBitDepth ;

	// 終了
	return 0 ;
}

// 描画サイズを取得する
extern int NS_GetDrawScreenSize( int *XBuf, int *YBuf )
{
	if( XBuf ) *XBuf = GSYS.Screen.MainScreenSizeX_Result == 0 ? DEFAULT_SCREEN_SIZE_X : GSYS.Screen.MainScreenSizeX_Result ;
	if( YBuf ) *YBuf = GSYS.Screen.MainScreenSizeY_Result == 0 ? DEFAULT_SCREEN_SIZE_Y : GSYS.Screen.MainScreenSizeY_Result ;

	// 終了
	return 0 ;
}

// 使用色ビット数を返す
extern int NS_GetScreenBitDepth( void )
{
	// 終了
	return NS_GetColorBitDepth() ;
}

// 画面の色ビット深度を得る
extern int NS_GetColorBitDepth( void )
{
	// 終了
	return GSYS.Screen.MainScreenColorBitDepth == 0 ? DEFAULT_COLOR_BITDEPTH : GSYS.Screen.MainScreenColorBitDepth ;
}

// 画面モードが変更されているかどうかのフラグを取得する
extern	int						NS_GetChangeDisplayFlag( void )
{
#ifdef __WINDOWS__
	// 終了
	return NS_GetWindowModeFlag() == FALSE ;
#else // __WINDOWS__
	return FALSE ;
#endif // __WINDOWS__
}

// ビデオメモリの容量を得る
extern	int						NS_GetVideoMemorySize( int *AllSize, int *FreeSize )
{
	return Graphics_Hardware_GetVideoMemorySize_PF( AllSize, FreeSize ) ;
}

// 現在の画面のリフレッシュレートを取得する
extern int NS_GetRefreshRate( void )
{
	return Graphics_GetRefreshRate_PF() ;
}

// ディスプレイの数を取得
extern int NS_GetDisplayNum( void )
{
	// ディスプレイ情報のセットアップが行われていない場合はセットアップをする
	if( GSYS.Screen.DisplayInfo == NULL )
	{
		if( Graphics_SetupDisplayInfo_PF() < 0 )
		{
			return -1 ;
		}
	}

	return GSYS.Screen.DisplayNum ;
}

// 変更可能なディスプレイモードの数を取得する
extern int NS_GetDisplayModeNum( int DisplayIndex )
{
	// ディスプレイ情報のセットアップが行われていない場合はセットアップをする
	if( GSYS.Screen.DisplayInfo == NULL )
	{
		if( Graphics_SetupDisplayInfo_PF() < 0 )
		{
			return -1 ;
		}
	}

	if( DisplayIndex < 0 || DisplayIndex >= GSYS.Screen.DisplayNum )
	{
		return -1 ;
	}

	return GSYS.Screen.DisplayInfo[ DisplayIndex ].ModeNum ;
}

// 変更可能なディスプレイモードの情報を取得する( ModeIndex は 0 〜 GetDisplayModeNum の戻り値-1 )
extern DISPLAYMODEDATA NS_GetDisplayMode( int ModeIndex, int DisplayIndex )
{
	static DISPLAYMODEDATA ErrorResult = { -1, -1, -1, -1 } ;

	// ディスプレイ情報のセットアップが行われていない場合はセットアップをする
	if( GSYS.Screen.DisplayInfo == NULL )
	{
		if( Graphics_SetupDisplayInfo_PF() < 0 )
		{
			return ErrorResult ;
		}
	}

	if( DisplayIndex < 0 || DisplayIndex >= GSYS.Screen.DisplayNum )
	{
		return ErrorResult ;
	}

	if( GSYS.Screen.DisplayInfo[ DisplayIndex ].ModeNum <= ModeIndex || ModeIndex < 0 )
	{
		return ErrorResult ;
	}

	return GSYS.Screen.DisplayInfo[ DisplayIndex ].ModeData[ ModeIndex ] ;
}

// ディスプレイの最大解像度を取得する
extern int NS_GetDisplayMaxResolution( int *SizeX, int *SizeY, int DisplayIndex )
{
	int MaxSizeX ;
	int MaxSizeY ;
	DISPLAYMODEDATA *DisplayModeData ;
	int i ;

	// ディスプレイ情報のセットアップが行われていない場合はセットアップをする
	if( GSYS.Screen.DisplayInfo == NULL )
	{
		if( Graphics_SetupDisplayInfo_PF() < 0 )
		{
			return -1 ;
		}
	}

	if( DisplayIndex < 0 || DisplayIndex >= GSYS.Screen.DisplayNum )
	{
		return -1 ;
	}

	// 最大の面積を持つ解像度を調べる
	MaxSizeX = 0 ;
	MaxSizeY = 0 ;
	DisplayModeData = GSYS.Screen.DisplayInfo[ DisplayIndex ].ModeData ;
	for( i = 0 ; i < GSYS.Screen.DisplayInfo[ DisplayIndex ].ModeNum ; i ++, DisplayModeData ++ )
	{
		if( DisplayModeData->Width * DisplayModeData->Height > MaxSizeX * MaxSizeY )
		{
			MaxSizeX = DisplayModeData->Width ;
			MaxSizeY = DisplayModeData->Height ;
		}
	}

	// サイズを保存
	if( SizeX != NULL )
	{
		*SizeX = MaxSizeX ;
	}

	if( SizeY != NULL )
	{
		*SizeY = MaxSizeY ;
	}

	// 正常終了
	return 0 ;
}

// ディスプレーのカラーデータポインタを得る
extern	const COLORDATA *NS_GetDispColorData( void )
{
	if( GSYS.Setting.ValidHardware )
	{
		return Graphics_Hardware_GetDispColorData_PF() ;
	}
	else
	{
		return GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP ;
	}
}

// 同時に描画を行うことができる画面の数を取得する
extern int NS_GetMultiDrawScreenNum( void )
{
	return GSYS.HardInfo.RenderTargetNum ;
}

// DrawGraphF 等の浮動小数点値で座標を指定する関数における座標タイプを取得する( 戻り値 : DX_DRAWFLOATCOORDTYPE_DIRECT3D9 など )
extern int NS_GetDrawFloatCoordType( void )
{
	return GSYS.HardInfo.DrawFloatCoordType ;
}

























// その他設定関係関数

// 通常描画にプログラマブルシェーダーを使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )
extern	int		NS_SetUseNormalDrawShader( int Flag )
{
	return Graphics_Hardware_SetUseNormalDrawShader_PF( Flag ) ;
}

// ソフトウエアレンダリングモードを使用するかどうかをセットする
extern int NS_SetUseSoftwareRenderModeFlag( int Flag )
{
	return NS_SetScreenMemToVramFlag( Flag == TRUE ? FALSE : TRUE ) ;
}

// ３Ｄ機能を使わないフラグのセット
extern	int		NS_SetNotUse3DFlag( int Flag )
{
	// 初期化前のみ使用可能　
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	GSYS.Setting.NotUseHardware = Flag ;

	// 終了
	return 0 ;
}

// ３Ｄ機能を使うか、のフラグをセット
extern	int NS_SetUse3DFlag( int Flag )
{
	// 初期化前のみ有効
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	GSYS.Setting.NotUseHardware = !Flag ;

	// 終了
	return 0 ;
}

// 描画に３Ｄ機能を使うかフラグを取得
extern	int NS_GetUse3DFlag( void )
{
	// 終了
	return !GSYS.Setting.NotUseHardware ;
}

// 画面データをＶＲＡＭに置くか、フラグ
extern	int		NS_SetScreenMemToVramFlag( int Flag )
{
	// 初期化前のみ使用可能　
	if( DxSysData.DxLib_InitializeFlag == TRUE ) return -1 ;

	GSYS.Setting.NotUseHardware = !Flag ;

	// 終了
	return 0 ;
}

// 画面グラフィックデータがシステムメモリに存在するかフラグの取得
extern	int						NS_GetScreenMemToSystemMemFlag( void )
{
	// 終了
	return GSYS.Setting.ValidHardware == FALSE ;
}

// 通常使用しない
extern int NS_SetWindowDrawRect( const RECT * /* DrawRect */ )
{
//	GSYS.WindowDrawRect = *DrawRect ;

	return 0 ;
}

// ＤＸライブラリのグラフィック関連の復帰処理を行う
extern	int NS_RestoreGraphSystem( void )
{
	Graphics_RestoreOrChangeSetupGraphSystem( FALSE ) ;

	// 終了
	return 0 ;
}

// ハードウエアの頂点演算処理機能を使用するかどうかを設定する
extern	int NS_SetUseHardwareVertexProcessing( int Flag )
{
	return Graphics_Hardware_SetUseHardwareVertexProcessing_PF( Flag ) ;
}

// ピクセル単位でライティングを行うかどうかを設定する、要 ShaderModel 3.0( TRUE:ピクセル単位のライティングを行う  FALSE:頂点単位のライティングを行う( デフォルト ) )
extern int NS_SetUsePixelLighting( int Flag )
{
	return Graphics_Hardware_SetUsePixelLighting_PF( Flag ) ;
}

// 以前の DrawModiGraph 関数のコードを使用するかどうかのフラグをセットする
extern int NS_SetUseOldDrawModiGraphCodeFlag( int Flag )
{
	return Graphics_Hardware_SetUseOldDrawModiGraphCodeFlag_PF( Flag ) ;
}

// ＶＲＡＭを使用するかのフラグをセットする
extern	int NS_SetUseVramFlag( int Flag )
{
	return NS_SetUseSystemMemGraphCreateFlag( Flag == TRUE ? FALSE : TRUE ) ;
}

// ２Ｄグラフィックサーフェス作成時にシステムメモリーを使用するかのフラグ取得
extern	int NS_GetUseVramFlag( void )
{
	return NS_GetUseSystemMemGraphCreateFlag() == TRUE ? FALSE : TRUE ;
}

// 簡略化ブレンド処理を行うか否かのフラグをセットする
extern	int		NS_SetBasicBlendFlag( int /*Flag*/ )
{
	// 終了
	return 0 ;
}

// 単純図形の描画に３Ｄデバイスの機能を使用するかどうかのフラグをセットする
extern int NS_SetUseBasicGraphDraw3DDeviceMethodFlag( int /* Flag */ )
{
//	GSYS.DrawSetting.NotUseBasicGraphDraw3DDeviceMethodFlag = !Flag ;

	return 0 ;
}

// ＤＸライブラリのウインドウを表示するディスプレイデバイスを設定する
extern int NS_SetUseDisplayIndex( int Index )
{
	if( GSYS.Screen.DisplayInfo == NULL )
	{
		Graphics_SetupDisplayInfo_PF() ;
	}

	if( Index < 0 || Index >= GSYS.Screen.DisplayNum )
	{
		return -1 ;
	}

	GSYS.Screen.ValidUseDisplayIndex = TRUE ;
	GSYS.Screen.UseDisplayIndex      = Index ;

	return 0 ;
}

// 頂点バッファに溜まった頂点データを吐き出す
extern int NS_RenderVertex( void )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	return 0 ;
}

























// 描画先画面保存関数

#ifndef DX_NON_SAVEFUNCTION

// Jpeg_Quality         = 0:低画質〜100:高画質
// Png_CompressionLevel = 0:無圧縮〜  9:最高圧縮
// 現在描画対象になっている画面を保存する
extern int NS_SaveDrawScreen( int x1, int y1, int x2, int y2, const TCHAR *FileName, int SaveType, int Jpeg_Quality, int Jpeg_Sample2x1, int Png_CompressionLevel )
{
#ifdef UNICODE
	return SaveDrawScreen_WCHAR_T(
		x1, y1, x2, y2, FileName, SaveType, Jpeg_Quality, Jpeg_Sample2x1, Png_CompressionLevel
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = SaveDrawScreen_WCHAR_T(
		x1, y1, x2, y2, UseFileNameBuffer, SaveType, Jpeg_Quality, Jpeg_Sample2x1, Png_CompressionLevel
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// Jpeg_Quality         = 0:低画質〜100:高画質
// Png_CompressionLevel = 0:無圧縮〜  9:最高圧縮
// 現在描画対象になっている画面を保存する
extern int SaveDrawScreen_WCHAR_T( int x1, int y1, int x2, int y2, const wchar_t *FileName, int SaveType, int Jpeg_Quality, int Jpeg_Sample2x1, int Png_CompressionLevel )
{
	BASEIMAGE BaseImage ;
	RECT      LockRect ;

	// サイズが不正な場合は何もせずに終了
	if( x1 >= x2 || y1 >= y2 || x1 < 0 || y1 < 0 || x2 > GSYS.DrawSetting.DrawSizeX || y2 > GSYS.DrawSetting.DrawSizeY )
	{
		return -1 ;
	}

	// 描画先をロック
	LockRect.left   = x1 ;
	LockRect.top    = y1 ;
	LockRect.right  = x2 ;
	LockRect.bottom = y2 ;
	if( Graphics_Screen_LockDrawScreen( &LockRect, &BaseImage, -1, -1, TRUE, 0 ) < 0 )
	{
		return -1 ;
	}

	// 保存形式によって処理を変更する
	switch( SaveType )
	{
	case DX_IMAGESAVETYPE_JPEG:
#ifndef DX_NON_JPEGREAD
		SaveBaseImageToJpeg_WCHAR_T( FileName, &BaseImage, Jpeg_Quality, Jpeg_Sample2x1 );
#endif
		break;

	case DX_IMAGESAVETYPE_PNG:
#ifndef DX_NON_PNGREAD
		SaveBaseImageToPng_WCHAR_T( FileName, &BaseImage, Png_CompressionLevel );
#endif
		break;

	case DX_IMAGESAVETYPE_BMP:	// Bitmap 
		SaveBaseImageToBmp_WCHAR_T( FileName, &BaseImage ) ;
		break ;
	}

	// 描画先をアンロック
	Graphics_Screen_UnlockDrawScreen() ;

	// 終了
	return 0 ;
}

// 現在描画対象になっている画面をＢＭＰ形式で保存する
extern int NS_SaveDrawScreenToBMP( int x1, int y1, int x2, int y2, const TCHAR *FileName )
{
	return NS_SaveDrawScreen( x1, y1, x2, y2, FileName, DX_IMAGESAVETYPE_BMP );
}

// 現在描画対象になっている画面をＢＭＰ形式で保存する
extern int SaveDrawScreenToBMP_WCHAR_T( int x1, int y1, int x2, int y2, const wchar_t *FileName )
{
	return SaveDrawScreen_WCHAR_T( x1, y1, x2, y2, FileName, DX_IMAGESAVETYPE_BMP );
}

// 現在描画対象になっている画面をＪＰＥＧ形式で保存する Quality = 画質、値が大きいほど低圧縮高画質,0〜100 
extern int NS_SaveDrawScreenToJPEG( int x1, int y1, int x2, int y2, const TCHAR *FileName, int Quality, int Sample2x1 )
{
	return NS_SaveDrawScreen( x1, y1, x2, y2, FileName, DX_IMAGESAVETYPE_JPEG, Quality, Sample2x1 );
}

// 現在描画対象になっている画面をＪＰＥＧ形式で保存する Quality = 画質、値が大きいほど低圧縮高画質,0〜100 
extern int SaveDrawScreenToJPEG_WCHAR_T( int x1, int y1, int x2, int y2, const wchar_t *FileName, int Quality, int Sample2x1 )
{
	return SaveDrawScreen_WCHAR_T( x1, y1, x2, y2, FileName, DX_IMAGESAVETYPE_JPEG, Quality, Sample2x1 );
}

// 現在描画対象になっている画面をＰＮＧ形式で保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0〜9
extern int NS_SaveDrawScreenToPNG( int x1, int y1, int x2, int y2, const TCHAR *FileName, int CompressionLevel )
{
	return NS_SaveDrawScreen( x1, y1, x2, y2, FileName, DX_IMAGESAVETYPE_PNG, 80, CompressionLevel );
}

// 現在描画対象になっている画面をＰＮＧ形式で保存する CompressionLevel = 圧縮率、値が大きいほど高圧縮率高負荷、０は無圧縮,0〜9
extern int SaveDrawScreenToPNG_WCHAR_T( int x1, int y1, int x2, int y2, const wchar_t *FileName, int CompressionLevel )
{
	return SaveDrawScreen_WCHAR_T( x1, y1, x2, y2, FileName, DX_IMAGESAVETYPE_PNG, 80, CompressionLevel );
}

#endif // DX_NON_SAVEFUNCTION

























// 頂点バッファ関係関数

// 頂点バッファを作成する( -1:エラー  0以上:頂点バッファハンドル )
extern int NS_CreateVertexBuffer( int VertexNum, int VertexType /* DX_VERTEX_TYPE_NORMAL_3D 等 */ )
{
	return Graphics_VertexBuffer_Create( VertexNum, VertexType, FALSE ) ;
}

// 頂点バッファを削除する
extern int NS_DeleteVertexBuffer( int VertexBufHandle )
{
	return SubHandle( VertexBufHandle ) ;
}

// すべての頂点バッファを削除する
extern int NS_InitVertexBuffer()
{
	return AllHandleSub( DX_HANDLETYPE_VERTEX_BUFFER ) ;
}

// 頂点バッファに頂点データを転送する
extern int NS_SetVertexBufferData( int SetIndex, const void *VertexData, int VertexNum, int VertexBufHandle )
{
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;

	// エラー判定
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
		return -1 ;

	// エラーチェック
	if( VertexBuffer->Num < SetIndex + VertexNum || SetIndex < 0 )
		return -1 ;

	// 環境依存の頂点データ転送処理
	if( Graphics_Hardware_VertexBuffer_SetData_PF( VertexBuffer, SetIndex, VertexData, VertexNum ) < 0 )
	{
		return -1 ;
	}

	// システムメモリのバッファへも転送
	_MEMCPY( ( BYTE * )VertexBuffer->Buffer + SetIndex * VertexBuffer->UnitSize, VertexData, ( size_t )( VertexNum * VertexBuffer->UnitSize ) ) ;

	// 終了
	return 0 ;
}

// インデックスバッファを作成する( -1:エラー　0以上：インデックスバッファハンドル )
extern int NS_CreateIndexBuffer( int IndexNum, int IndexType /* DX_INDEX_TYPE_16BIT 等 */  )
{
	return Graphics_IndexBuffer_Create( IndexNum, IndexType, FALSE ) ;
}

// インデックスバッファを削除する
extern int NS_DeleteIndexBuffer( int IndexBufHandle )
{
	return SubHandle( IndexBufHandle ) ;
}

// すべてのインデックスバッファを削除する
extern int NS_InitIndexBuffer()
{
	return AllHandleSub( DX_HANDLETYPE_INDEX_BUFFER ) ;
}

// インデックスバッファにインデックスデータを転送する
extern int NS_SetIndexBufferData( int SetIndex, const void *IndexData, int IndexNum, int IndexBufHandle )
{
	INDEXBUFFERHANDLEDATA *IndexBuffer ;

	// エラー判定
	if( INDEXBUFFERCHK( IndexBufHandle, IndexBuffer ) )
		return -1 ;

	// エラーチェック
	if( IndexBuffer->Num < SetIndex + IndexNum || SetIndex < 0 )
		return -1 ;

	// 環境依存のインデックスデータ転送処理
	if( Graphics_Hardware_IndexBuffer_SetData_PF( IndexBuffer, SetIndex, IndexData, IndexNum ) < 0 )
	{
		return -1 ;
	}

	// システムメモリのバッファへも転送
	_MEMCPY( ( BYTE * )IndexBuffer->Buffer + SetIndex * IndexBuffer->UnitSize, IndexData, ( size_t )( IndexNum * IndexBuffer->UnitSize ) ) ;

	// 終了
	return 0 ;
}

// グラフィックスデバイスが対応している一度に描画できるプリミティブの最大数を取得する
extern int NS_GetMaxPrimitiveCount( void )
{
	return GSYS.HardInfo.MaxPrimitiveCount ;
}

// グラフィックスデバイスが対応している一度に使用することのできる最大頂点数を取得する
extern int NS_GetMaxVertexIndex( void )
{
	return GSYS.HardInfo.MaxVertexIndex ;
}






















// シェーダー関係関数

// 使用できるシェーダーのバージョンを取得する( 0=使えない  200=シェーダーモデル２．０が使用可能  300=シェーダーモデル３．０が使用可能 )
extern int NS_GetValidShaderVersion( void )
{
	return Graphics_Hardware_Shader_GetValidShaderVersion_PF() ;
}

// 頂点シェーダーバイナリをファイルから読み込み頂点シェーダーハンドルを作成する( 戻り値 -1:エラー  -1以外:シェーダーハンドル )
extern int NS_LoadVertexShader( const TCHAR *FileName )
{
#ifdef UNICODE
	return LoadVertexShader_WCHAR_T(
		FileName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = LoadVertexShader_WCHAR_T(
		UseFileNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// 頂点シェーダーバイナリをファイルから読み込み頂点シェーダーハンドルを作成する( 戻り値 -1:エラー  -1以外:シェーダーハンドル )
extern int LoadVertexShader_WCHAR_T( const wchar_t *FileName )
{
	return Graphics_Shader_LoadShader_UseGParam( TRUE, FileName, GetASyncLoadFlag() ) ;
}

// ピクセルシェーダーバイナリをファイルから読み込みピクセルシェーダーハンドルを作成する( 戻り値 -1:エラー  -1以外:シェーダーハンドル )
extern int NS_LoadPixelShader( const TCHAR *FileName )
{
#ifdef UNICODE
	return LoadPixelShader_WCHAR_T(
		FileName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = LoadPixelShader_WCHAR_T(
		UseFileNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// ピクセルシェーダーバイナリをファイルから読み込みピクセルシェーダーハンドルを作成する( 戻り値 -1:エラー  -1以外:シェーダーハンドル )
extern int LoadPixelShader_WCHAR_T( const wchar_t *FileName )
{
	return Graphics_Shader_LoadShader_UseGParam( FALSE, FileName, GetASyncLoadFlag() ) ;
}

// メモリ空間上に存在する頂点シェーダーバイナリから頂点シェーダーハンドルを作成する( 戻り値 -1:エラー  -1以外:シェーダーハンドル )
extern int NS_LoadVertexShaderFromMem( const void *ImageAddress, int ImageSize )
{
	return Graphics_Shader_CreateHandle_UseGParam( TRUE, ( void * )ImageAddress, ImageSize, FALSE, GetASyncLoadFlag() ) ;
}

// メモリ空間上に存在するピクセルシェーダーバイナリからピクセルシェーダーハンドルを作成する( 戻り値 -1:エラー  -1以外:シェーダーハンドル )
extern int NS_LoadPixelShaderFromMem( const void *ImageAddress, int ImageSize )
{
	return Graphics_Shader_CreateHandle_UseGParam( FALSE, ( void * )ImageAddress, ImageSize, FALSE, GetASyncLoadFlag() ) ;
}

// シェーダーハンドルの削除
extern int NS_DeleteShader( int ShaderHandle )
{
	return SubHandle( ShaderHandle ) ;
}

// シェーダーハンドルを全て削除する
extern int NS_InitShader( void )
{
	if( GSYS.InitializeFlag == FALSE ) return 0 ;

	return AllHandleSub( DX_HANDLETYPE_SHADER ) ;
}

// 指定の名前を持つ定数が使用するシェーダー定数の番号を取得する
extern int NS_GetConstIndexToShader( const TCHAR *ConstantName, int ShaderHandle )
{
#ifdef UNICODE
	return GetConstIndexToShader_WCHAR_T(
		ConstantName, ShaderHandle
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( ConstantName, return -1 )

	Result = GetConstIndexToShader_WCHAR_T(
		UseConstantNameBuffer, ShaderHandle
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( ConstantName )

	return Result ;
#endif
}

// 指定の名前を持つ定数が使用するシェーダー定数の番号を取得する
extern int GetConstIndexToShader_WCHAR_T( const wchar_t *ConstantName, int ShaderHandle )
{
	SHADERHANDLEDATA *Shader ;

	// エラー判定
	if( SHADERCHK( ShaderHandle, Shader ) )
	{
		return -1 ;
	}

	// 環境依存処理
	return Graphics_Hardware_Shader_GetConstIndex_PF( ConstantName, Shader ) ;
}

// 指定の名前を持つ定数が使用するシェーダー定数の数を取得する
extern int NS_GetConstCountToShader( const TCHAR *ConstantName, int ShaderHandle )
{
#ifdef UNICODE
	return GetConstCountToShader_WCHAR_T(
		ConstantName, ShaderHandle
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( ConstantName, return -1 )

	Result = GetConstCountToShader_WCHAR_T(
		UseConstantNameBuffer, ShaderHandle
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( ConstantName )

	return Result ;
#endif
}

// 指定の名前を持つ定数が使用するシェーダー定数の数を取得する
extern int GetConstCountToShader_WCHAR_T( const wchar_t *ConstantName, int ShaderHandle )
{
	SHADERHANDLEDATA *Shader ;

	// エラー判定
	if( SHADERCHK( ShaderHandle, Shader ) )
		return -1 ;

	// 環境依存処理
	return Graphics_Hardware_Shader_GetConstCount_PF( ConstantName, Shader ) ;
}

// 指定の名前を持つ浮動小数点定数のデフォルトパラメータが格納されているメモリアドレスを取得する
extern	const FLOAT4 *NS_GetConstDefaultParamFToShader( const TCHAR *ConstantName, int ShaderHandle )
{
#ifdef UNICODE
	return GetConstDefaultParamFToShader_WCHAR_T(
		ConstantName, ShaderHandle
	) ;
#else
	const FLOAT4 * Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( ConstantName, return NULL )

	Result = GetConstDefaultParamFToShader_WCHAR_T(
		UseConstantNameBuffer, ShaderHandle
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( ConstantName )

	return Result ;
#endif
}

// 指定の名前を持つ浮動小数点定数のデフォルトパラメータが格納されているメモリアドレスを取得する
extern	const FLOAT4 *GetConstDefaultParamFToShader_WCHAR_T( const wchar_t *ConstantName, int ShaderHandle )
{
	SHADERHANDLEDATA *Shader ;

	// エラー判定
	if( SHADERCHK( ShaderHandle, Shader ) )
		return NULL ;

	// 環境依存処理
	return Graphics_Hardware_Shader_GetConstDefaultParamF_PF( ConstantName, Shader ) ;
}

// 頂点シェーダーの float 型定数を設定する
extern int NS_SetVSConstSF( int ConstantIndex, float Param )
{
	FLOAT4 ParamF4 ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 256 )
		return -1 ;

	// 定数更新
	ParamF4.x = Param ;
	ParamF4.y = Param ;
	ParamF4.z = Param ;
	ParamF4.w = Param ;
	NS_SetVSConstF( ConstantIndex, ParamF4 ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数を設定する
extern int NS_SetVSConstF( int ConstantIndex, FLOAT4 Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 256 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, &Param, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数に行列を設定する
extern	int			NS_SetVSConstFMtx( int ConstantIndex, MATRIX Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + 4 > 256 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ( FLOAT4 * )&Param, 4, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数に転置した行列を設定する
extern	int			NS_SetVSConstFMtxT( int ConstantIndex, MATRIX Param )
{
	MATRIX Transpose ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + 4 > 256 )
		return -1 ;

	// 定数更新
	CreateTransposeMatrix( &Transpose, &Param ) ;
	NS_SetVSConstFMtx( ConstantIndex, Transpose ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの int 型定数を設定する
extern	int			NS_SetVSConstSI( int ConstantIndex, int Param )
{
	INT4 ParamI4 ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 16 )
		return -1 ;

	// 定数更新
	ParamI4.x = Param ;
	ParamI4.y = Param ;
	ParamI4.z = Param ;
	ParamI4.w = Param ;
	return NS_SetVSConstI( ConstantIndex, ParamI4 ) ;
}

// 頂点シェーダーの int 型定数を設定する
extern	int			NS_SetVSConstI( int ConstantIndex, INT4 Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, &Param, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの BOOL 型定数を設定する
extern	int			NS_SetVSConstB( int ConstantIndex, BOOL Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_BOOL, DX_SHADERCONSTANTSET_USER, ConstantIndex, &Param, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetVSConstSFArray( int ConstantIndex, const float *ParamArray, int ParamNum )
{
	FLOAT4 ParamArrayF4[ 256 ] ;
	int i ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 256 )
		return -1 ;

	// 配列にデータをセット
	for( i = 0 ; i < ParamNum ; i ++ )
	{
		ParamArrayF4[ i ].x = ParamArray[ i ] ;
		ParamArrayF4[ i ].y = ParamArray[ i ] ;
		ParamArrayF4[ i ].z = ParamArray[ i ] ;
		ParamArrayF4[ i ].w = ParamArray[ i ] ;
	}

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArrayF4, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetVSConstFArray( int ConstantIndex, const FLOAT4 *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 256 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数に行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetVSConstFMtxArray( int ConstantIndex, const MATRIX *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum * 4 > 256 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum * 4, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数に転置した行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetVSConstFMtxTArray( int ConstantIndex, const MATRIX *ParamArray, int ParamNum )
{
	MATRIX Transpose[ 256 / 4 ] ;
	int i ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum * 4 > 256 )
		return -1 ;

	// 定数更新
	for( i = 0 ; i < ParamNum ; i ++ )
	{
		Transpose[ i ].m[ 0 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 3 ] ;

		Transpose[ i ].m[ 0 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 3 ] ;

		Transpose[ i ].m[ 0 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 3 ] ;

		Transpose[ i ].m[ 0 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 3 ] ;
	}
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, Transpose, ParamNum * 4, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの int 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetVSConstSIArray( int ConstantIndex, const int *ParamArray, int ParamNum )
{
	INT4 ParamArrayI4[ 16 ] ;
	int i ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 配列のセット
	for( i = 0 ; i < ParamNum ; i ++ )
	{
		ParamArrayI4[ i ].x = ParamArray[ i ] ;
		ParamArrayI4[ i ].y = ParamArray[ i ] ;
		ParamArrayI4[ i ].z = ParamArray[ i ] ;
		ParamArrayI4[ i ].w = ParamArray[ i ] ;
	}

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArrayI4, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの int 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetVSConstIArray( int ConstantIndex, const INT4 *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの BOOL 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetVSConstBArray( int ConstantIndex, const BOOL *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_VS_BOOL, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの float 型定数の設定をリセットする
extern	int			NS_ResetVSConstF( int ConstantIndex, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 256 )
		return -1 ;

	// 定数リセット
	Graphics_Hardware_Shader_ResetConst_PF( DX_SHADERCONSTANTTYPE_VS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamNum ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの int 型定数の設定をリセットする
extern	int			NS_ResetVSConstI( int ConstantIndex, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数リセット
	Graphics_Hardware_Shader_ResetConst_PF( DX_SHADERCONSTANTTYPE_VS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamNum ) ;

	// 終了
	return 0 ;
}

// 頂点シェーダーの BOOL 型定数の設定をリセットする
extern	int			NS_ResetVSConstB( int ConstantIndex, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数リセット
	Graphics_Hardware_Shader_ResetConst_PF( DX_SHADERCONSTANTTYPE_VS_BOOL, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamNum ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数を設定する
extern	int			NS_SetPSConstSF( int ConstantIndex, float Param )
{
	FLOAT4 ParamF4 ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 224 )
		return -1 ;

	// 定数更新
	ParamF4.x = Param ;
	ParamF4.y = Param ;
	ParamF4.z = Param ;
	ParamF4.w = Param ;
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, &ParamF4, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数を設定する
extern	int			NS_SetPSConstF( int ConstantIndex, FLOAT4 Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 224 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, &Param, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数に行列を設定する
extern	int			NS_SetPSConstFMtx( int ConstantIndex, MATRIX Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + 4 > 224 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ( FLOAT4 * )&Param, 4, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数に転置した行列を設定する
extern	int			NS_SetPSConstFMtxT( int ConstantIndex, MATRIX Param )
{
	MATRIX Transpose ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + 4 > 224 )
		return -1 ;

	// 定数更新
	CreateTransposeMatrix( &Transpose, &Param ) ;
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ( FLOAT4 * )&Transpose, 4, TRUE ) ;

	// 終了
	return 0 ;
}


// ピクセルシェーダーの int 型定数を設定する
extern	int			NS_SetPSConstSI( int ConstantIndex, int Param )
{
	INT4 ParamI4 ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 16 )
		return -1 ;

	// 定数更新
	ParamI4.x = Param ;
	ParamI4.y = Param ;
	ParamI4.z = Param ;
	ParamI4.w = Param ;
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, &ParamI4, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの int 型定数を設定する
extern	int			NS_SetPSConstI( int ConstantIndex, INT4 Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, &Param, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの BOOL 型定数を設定する
extern	int			NS_SetPSConstB( int ConstantIndex, BOOL Param )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex >= 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_BOOL, DX_SHADERCONSTANTSET_USER, ConstantIndex, &Param, 1, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetPSConstSFArray( int ConstantIndex, const float *ParamArray, int ParamNum )
{
	FLOAT4 ParamArrayF4[ 256 ] ;
	int i ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 224 )
		return -1 ;

	// 配列にセット
	for( i = 0 ; i < ParamNum ; i ++ )
	{
		ParamArrayF4[ i ].x = ParamArray[ i ] ;
		ParamArrayF4[ i ].y = ParamArray[ i ] ;
		ParamArrayF4[ i ].z = ParamArray[ i ] ;
		ParamArrayF4[ i ].w = ParamArray[ i ] ;
	}

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArrayF4, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetPSConstFArray( int ConstantIndex, const FLOAT4 *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 224 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数に行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetPSConstFMtxArray( int ConstantIndex, const MATRIX *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum * 4 > 224 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum * 4, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数に転置した行列を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetPSConstFMtxTArray( int ConstantIndex, const MATRIX *ParamArray, int ParamNum )
{
	int i ;
	MATRIX Transpose[ 224 / 4 ] ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum * 4 > 224 )
		return -1 ;

	// 定数更新
	for( i = 0 ; i < ParamNum ; i ++ )
	{
		Transpose[ i ].m[ 0 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 0 ] = ParamArray[ i ].m[ 0 ][ 3 ] ;

		Transpose[ i ].m[ 0 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 1 ] = ParamArray[ i ].m[ 1 ][ 3 ] ;

		Transpose[ i ].m[ 0 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 2 ] = ParamArray[ i ].m[ 2 ][ 3 ] ;

		Transpose[ i ].m[ 0 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 0 ] ;
		Transpose[ i ].m[ 1 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 1 ] ;
		Transpose[ i ].m[ 2 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 2 ] ;
		Transpose[ i ].m[ 3 ][ 3 ] = ParamArray[ i ].m[ 3 ][ 3 ] ;
	}
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, Transpose, ParamNum * 4, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの int 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetPSConstSIArray( int ConstantIndex, const int *ParamArray, int ParamNum )
{
	INT4 ParamArrayI4[ 16 ] ;
	int i ;

	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 配列にパラメータをセット
	for( i = 0 ; i < ParamNum ; i ++ )
	{
		ParamArrayI4[ i ].x = ParamArray[ i ] ;
		ParamArrayI4[ i ].y = ParamArray[ i ] ;
		ParamArrayI4[ i ].z = ParamArray[ i ] ;
		ParamArrayI4[ i ].w = ParamArray[ i ] ;
	}

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArrayI4, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの int 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetPSConstIArray( int ConstantIndex, const INT4 *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの BOOL 型定数を設定する( 配列を使って連番インデックスに一度に設定 )
extern	int			NS_SetPSConstBArray( int ConstantIndex, const BOOL *ParamArray, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数更新
	Graphics_Hardware_Shader_SetConst_PF( DX_SHADERCONSTANTTYPE_PS_BOOL, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamArray, ParamNum, TRUE ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの float 型定数の設定をリセットする
extern	int			NS_ResetPSConstF( int ConstantIndex, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 256 )
		return -1 ;

	// 定数リセット
	Graphics_Hardware_Shader_ResetConst_PF( DX_SHADERCONSTANTTYPE_PS_FLOAT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamNum ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの int 型定数の設定をリセットする
extern	int			NS_ResetPSConstI( int ConstantIndex, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数リセット
	Graphics_Hardware_Shader_ResetConst_PF( DX_SHADERCONSTANTTYPE_PS_INT, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamNum ) ;

	// 終了
	return 0 ;
}

// ピクセルシェーダーの BOOL 型定数の設定をリセットする
extern	int			NS_ResetPSConstB( int ConstantIndex, int ParamNum )
{
	// シェーダーが使えない場合は何もしない
	if( GSYS.HardInfo.UseShader == FALSE )
		return 0 ;

	// 範囲越えチェック
	if( ConstantIndex < 0 || ConstantIndex + ParamNum > 16 )
		return -1 ;

	// 定数リセット
	Graphics_Hardware_Shader_ResetConst_PF( DX_SHADERCONSTANTTYPE_PS_BOOL, DX_SHADERCONSTANTSET_USER, ConstantIndex, ParamNum ) ;

	// 終了
	return 0 ;
}

// シェーダー描画での描画先を設定する
extern int NS_SetRenderTargetToShader( int TargetIndex, int DrawScreen, int SurfaceIndex )
{
	// ターゲットインデックスの範囲チェック
	if( TargetIndex < 0 || TargetIndex >= GSYS.HardInfo.RenderTargetNum )
		return -1 ;

	// 値が同じ場合は何もせず終了
//	if( GSYS.DrawSetting.TargetScreen[ TargetIndex ] == DrawScreen && GSYS.DrawSetting.TargetScreenSurface[ TargetIndex ] == SurfaceIndex )
//		return 0 ;

	// ターゲットが０でサーフェスインデックスも０の場合は SetDrawScreen に任せる
	if( TargetIndex == 0 && SurfaceIndex == 0 )
	{
		return NS_SetDrawScreen( DrawScreen ) ;
	}

	// ハードウエアで動作していない場合は何もしない
	if( GSYS.Setting.ValidHardware == FALSE )
	{
		return -1 ;
	}

	// ハードウエアアクセラレーション別の処理を実行する
	if( Graphics_Hardware_SetRenderTargetToShader_PF( TargetIndex, DrawScreen, SurfaceIndex ) < 0 )
	{
		return -1 ;
	}

	// 画像ハンドルを保存
	GSYS.DrawSetting.TargetScreen[ TargetIndex ] = DrawScreen ;
	GSYS.DrawSetting.TargetScreenSurface[ TargetIndex ] = SurfaceIndex ;

	// 描画先に正しいα値を書き込むかどうかのフラグを更新する
	Graphics_DrawSetting_RefreshAlphaChDrawMode() ;

	// 正常終了
	return 0 ;
}

// シェーダー描画で使用するグラフィックを設定する
extern	int			NS_SetUseTextureToShader( int StageIndex, int GraphHandle )
{
	// 有効範囲チェック
	if( StageIndex < 0 || StageIndex >= USE_TEXTURESTAGE_NUM )
		return -1 ;

	// グラフィックハンドルチェック
	if( GraphHandle != -1 )
	{
		IMAGEDATA *Image ;
		SHADOWMAPDATA *ShadowMap ;

		if( GRAPHCHKFULL(     GraphHandle, Image    ) &&
			SHADOWMAPCHKFULL( GraphHandle, ShadowMap ) )
			return -1 ;

		// ハンドルを保存
		GSYS.DrawSetting.UserShaderRenderInfo.SetTextureGraphHandle[ StageIndex ] = GraphHandle ;
	}
	else
	{
		// ハンドルをリセット
		GSYS.DrawSetting.UserShaderRenderInfo.SetTextureGraphHandle[ StageIndex ] = 0 ;
	}

	// 終了
	return 0 ;
}

// シェーダー描画に使用する頂点シェーダーを設定する
extern	int			NS_SetUseVertexShader( int ShaderHandle )
{
	SHADERHANDLEDATA *VertexShader ;

	if( ShaderHandle > 0 && ( SHADERCHK( ShaderHandle, VertexShader ) || VertexShader->IsVertexShader == FALSE ) )
		return -1 ;

	// ハンドルをセット
	GSYS.DrawSetting.UserShaderRenderInfo.SetVertexShaderHandle = ShaderHandle <= 0 ? 0 : ShaderHandle ;

	// 終了
	return 0 ;
}

// シェーダー描画に使用するピクセルシェーダーを設定する
extern	int			NS_SetUsePixelShader( int ShaderHandle )
{
	SHADERHANDLEDATA *PixelShader ;

	if( ShaderHandle > 0 && ( SHADERCHK( ShaderHandle, PixelShader ) || PixelShader->IsVertexShader == TRUE ) )
		return -1 ;

	// ハンドルをセット
	GSYS.DrawSetting.UserShaderRenderInfo.SetPixelShaderHandle = ShaderHandle <= 0 ? 0 : ShaderHandle  ;

	// 終了
	return 0 ;
}

// ポリゴンの頂点の接線と従法線をＵＶ座標から計算してセットする
extern int NS_CalcPolygonBinormalAndTangentsToShader( VERTEX3DSHADER *Vertex, int PolygonNum )
{
	unsigned short *Indices ;
	int IndexNum ;
	int i ;
	int Result ;

	IndexNum = PolygonNum * 3 ;

	Indices = ( unsigned short * )DXALLOC( IndexNum * sizeof( unsigned short ) ) ;
	if( Indices == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x02\x98\xb9\x70\xa4\x30\xf3\x30\xc7\x30\xc3\x30\xaf\x30\xb9\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"頂点インデックスを格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
		return -1 ;
	}

	// 頂点インデックスをセット
	for( i = 0 ; i < IndexNum ; i ++ )
	{
		Indices[ i ] = ( unsigned short )i ;
	}

	// 接線と従法線を計算
	Result = NS_CalcPolygonIndexedBinormalAndTangentsToShader( Vertex, IndexNum, Indices, PolygonNum ) ;

	// 頂点インデックスを格納していたメモリ領域を開放
	DXFREE( Indices ) ;

	// 終了
	return Result ;
}

// ポリゴンの頂点の接線と従法線をＵＶ座標から計算してセットする(インデックス)
extern int NS_CalcPolygonIndexedBinormalAndTangentsToShader( VERTEX3DSHADER *Vertex, int VertexNum, const unsigned short *Indices, int PolygonNum )
{
	int i ;
	VECTOR v1, v2, vt, du, dv, vb, vn ;
	BYTE *UseFlag ;
	const unsigned short *Index ;
	int IndexNum ;
	VERTEX3DSHADER *Vert[ 3 ] ;

	// 使用している頂点のテーブルを作成する
	{
		UseFlag = ( BYTE * )DXALLOC( sizeof( BYTE ) * VertexNum ) ;
		if( UseFlag == NULL )
		{
			DXST_ERRORLOGFMT_ADDUTF16LE(( "\x02\x98\xb9\x70\x6e\x30\xa5\x63\xda\x7d\x68\x30\x93\x5f\xd5\x6c\xda\x7d\x6e\x30\x5c\x4f\x10\x62\x5c\x4f\x6d\x69\x6b\x30\xc5\x5f\x81\x89\x6a\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"頂点の接線と従法線の作成作業に必要なメモリ領域の確保に失敗しました\n" @*/ )) ;
			return -1 ;
		}
		_MEMSET( UseFlag, 0, ( size_t )VertexNum ) ;

		IndexNum = PolygonNum * 3 ;
		for( i = 0 ; i < IndexNum ; i ++ )
		{
			UseFlag[ Indices[ i ] ] = 1 ;
		}
	}

	// 接線と従法線の初期化
	for( i = 0 ; i < VertexNum ; i ++ )
	{
		if( UseFlag[ i ] == 0 ) continue ;
		Vertex[ i ].binorm.x = 0.0f ;
		Vertex[ i ].binorm.y = 0.0f ;
		Vertex[ i ].binorm.z = 0.0f ;
		Vertex[ i ].tan.x = 0.0f ;
		Vertex[ i ].tan.y = 0.0f ;
		Vertex[ i ].tan.z = 0.0f ;
	}

	// 全ての面の数だけ繰り返し
	Index = Indices ;
	for( i = 0 ; i < PolygonNum ; i ++, Index += 3 )
	{
		Vert[ 0 ] = &Vertex[ Index[ 0 ] ] ;
		Vert[ 1 ] = &Vertex[ Index[ 1 ] ] ;
		Vert[ 2 ] = &Vertex[ Index[ 2 ] ] ;

		v1.x = Vert[ 1 ]->pos.x - Vert[ 0 ]->pos.x ;
		v1.y = Vert[ 1 ]->u - Vert[ 0 ]->u ;
		v1.z = Vert[ 1 ]->v - Vert[ 0 ]->v ;

		v2.x = Vert[ 2 ]->pos.x - Vert[ 0 ]->pos.x ;
		v2.y = Vert[ 2 ]->u - Vert[ 0 ]->u ;
		v2.z = Vert[ 2 ]->v - Vert[ 0 ]->v ;

		vt = VCross( v1, v2 ) ;
		du.x = 1.0f ;
		if( VDot( vt, vt ) >= 0.0000001f )
		{
			du.x = -vt.y / vt.x ;
			dv.x = -vt.z / vt.x ;
		}

		v1.x = Vert[ 1 ]->pos.y - Vert[ 0 ]->pos.y ;
		v2.x = Vert[ 2 ]->pos.y - Vert[ 0 ]->pos.y ;

		vt = VCross( v1, v2 ) ;
		du.y = 1.0f ;
		if( VDot( vt, vt ) >= 0.0000001f )
		{
			du.y = -vt.y / vt.x ;
			dv.y = -vt.z / vt.x ;
		}

		v1.x = Vert[ 1 ]->pos.z - Vert[ 0 ]->pos.z ;
		v2.x = Vert[ 2 ]->pos.z - Vert[ 0 ]->pos.z ;

		vt = VCross( v1, v2 ) ;
		du.z = 1.0f ;
		if( VDot( vt, vt ) >= 0.0000001f )
		{
			du.z = -vt.y / vt.x ;
			dv.z = -vt.z / vt.x ;
		}

		VectorAdd( &Vert[ 0 ]->tan, &Vert[ 0 ]->tan, &du ) ;
		VectorAdd( &Vert[ 1 ]->tan, &Vert[ 1 ]->tan, &du ) ;
		VectorAdd( &Vert[ 2 ]->tan, &Vert[ 2 ]->tan, &du ) ;

		VectorAdd( &Vert[ 0 ]->binorm, &Vert[ 0 ]->binorm, &dv ) ;
		VectorAdd( &Vert[ 1 ]->binorm, &Vert[ 1 ]->binorm, &dv ) ;
		VectorAdd( &Vert[ 2 ]->binorm, &Vert[ 2 ]->binorm, &dv ) ;
	}

	// 法線の算出と正規化
	for( i = 0 ; i < VertexNum ; i ++ )
	{
		if( UseFlag[ i ] == 0 ) continue ;

		vt = VNorm( Vertex[ i ].tan ) ;
		vn = VNorm( VCross( vt, Vertex[ i ].binorm ) ) ;
		vb = VNorm( VCross( vn, vt ) ) ;

		// 正規化
		Vertex[ i ].tan    = vt ;
		Vertex[ i ].binorm = vb ;
	}

	// メモリの解放
	DXFREE( UseFlag ) ;

	// 終了
	return 0 ;
}



// シェーダーを使って２Ｄポリゴンを描画する
extern	int			NS_DrawPolygon2DToShader( const VERTEX2DSHADER *Vertex, int PolygonNum )
{
	return NS_DrawPrimitive2DToShader( Vertex, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST ) ;
}

// シェーダーを使って３Ｄポリゴンを描画する
extern	int			NS_DrawPolygon3DToShader( const VERTEX3DSHADER *Vertex, int PolygonNum )
{
	return NS_DrawPrimitive3DToShader( Vertex, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST ) ;
}

// シェーダーを使って２Ｄポリゴンを描画する(インデックス)
extern	int			NS_DrawPolygonIndexed2DToShader( const VERTEX2DSHADER *Vertex, int VertexNum, const unsigned short *Indices, int PolygonNum )
{
	return NS_DrawPrimitiveIndexed2DToShader( Vertex, VertexNum, Indices, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST ) ;
}

// シェーダーを使って３Ｄポリゴンを描画する(インデックス)
extern	int			NS_DrawPolygonIndexed3DToShader( const VERTEX3DSHADER *Vertex, int VertexNum, const unsigned short *Indices, int PolygonNum )
{
	return NS_DrawPrimitiveIndexed3DToShader( Vertex, VertexNum, Indices, PolygonNum * 3, DX_PRIMTYPE_TRIANGLELIST ) ;
}

// シェーダーを使って２Ｄプリミティブを描画する
extern	int			NS_DrawPrimitive2DToShader( const VERTEX2DSHADER *Vertex, int VertexNum, int PrimitiveType )
{
	return Graphics_Hardware_DrawPrimitive2DToShader_PF( Vertex, VertexNum, PrimitiveType ) ;
}

// シェーダーを使って３Ｄプリミティブを描画する
extern	int			NS_DrawPrimitive3DToShader( const VERTEX3DSHADER *Vertex, int VertexNum, int PrimitiveType )
{
	return Graphics_Hardware_DrawPrimitive3DToShader_PF( Vertex, VertexNum, PrimitiveType ) ;
}

// シェーダーを使って２Ｄプリミティブを描画する(インデックス)
extern	int			NS_DrawPrimitiveIndexed2DToShader( const VERTEX2DSHADER *Vertex, int VertexNum, const unsigned short *Indices, int IndexNum, int PrimitiveType )
{
	return Graphics_Hardware_DrawPrimitiveIndexed2DToShader_PF( Vertex, VertexNum, Indices, IndexNum, PrimitiveType ) ;
}

// シェーダーを使って３Ｄプリミティブを描画する(インデックス)
extern	int			NS_DrawPrimitiveIndexed3DToShader( const VERTEX3DSHADER *Vertex, int VertexNum, const unsigned short *Indices, int IndexNum, int PrimitiveType )
{
	return Graphics_Hardware_DrawPrimitiveIndexed3DToShader_PF( Vertex, VertexNum, Indices, IndexNum, PrimitiveType ) ;
}

// シェーダーを使って３Ｄポリゴンを描画する( 頂点バッファ使用版 )
extern	int			NS_DrawPolygon3DToShader_UseVertexBuffer( int VertexBufHandle )
{
	return NS_DrawPrimitive3DToShader_UseVertexBuffer( VertexBufHandle, DX_PRIMTYPE_TRIANGLELIST ) ;
}

// シェーダーを使って３Ｄポリゴンを描画する( 頂点バッファとインデックスバッファ使用版 )
extern	int			NS_DrawPolygonIndexed3DToShader_UseVertexBuffer( int VertexBufHandle, int IndexBufHandle )
{
	return NS_DrawPrimitiveIndexed3DToShader_UseVertexBuffer( VertexBufHandle, IndexBufHandle, DX_PRIMTYPE_TRIANGLELIST ) ;
}

// シェーダーを使って３Ｄプリミティブを描画する( 頂点バッファ使用版 )
extern	int			NS_DrawPrimitive3DToShader_UseVertexBuffer( int VertexBufHandle, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ )
{
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;

	// エラー判定
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
	{
		return -1 ;
	}

	return NS_DrawPrimitive3DToShader_UseVertexBuffer2( VertexBufHandle, PrimitiveType, 0, VertexBuffer->Num ) ;
}

// シェーダーを使って３Ｄプリミティブを描画する( 頂点バッファ使用版 )
extern	int			NS_DrawPrimitive3DToShader_UseVertexBuffer2( int VertexBufHandle, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int StartVertex, int UseVertexNum )
{
	return Graphics_Hardware_DrawPrimitive3DToShader_UseVertexBuffer2_PF( VertexBufHandle, PrimitiveType, StartVertex, UseVertexNum ) ;
}

// シェーダーを使って３Ｄプリミティブを描画する( 頂点バッファとインデックスバッファ使用版 )
extern	int			NS_DrawPrimitiveIndexed3DToShader_UseVertexBuffer( int VertexBufHandle, int IndexBufHandle, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */ )
{
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;
	INDEXBUFFERHANDLEDATA  *IndexBuffer ;

	// エラー判定
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
	{
		return -1 ;
	}
	if( INDEXBUFFERCHK( IndexBufHandle, IndexBuffer ) )
	{
		return -1 ;
	}

	return NS_DrawPrimitiveIndexed3DToShader_UseVertexBuffer2( VertexBufHandle, IndexBufHandle, PrimitiveType, 0, 0, VertexBuffer->Num, 0, IndexBuffer->Num ) ;
}

// シェーダーを使って３Ｄプリミティブを描画する( 頂点バッファとインデックスバッファ使用版 )
extern	int			NS_DrawPrimitiveIndexed3DToShader_UseVertexBuffer2( int VertexBufHandle, int IndexBufHandle, int PrimitiveType /* DX_PRIMTYPE_TRIANGLELIST 等 */, int BaseVertex, int StartVertex, int UseVertexNum, int StartIndex, int UseIndexNum )
{
	return Graphics_Hardware_DrawPrimitiveIndexed3DToShader_UseVertexBuffer2_PF( VertexBufHandle, IndexBufHandle, PrimitiveType, BaseVertex, StartVertex, UseVertexNum, StartIndex, UseIndexNum ) ;
}






















// シェーダー用定数バッファ関係関数

// 全てのシェーダー用定数バッファハンドルを削除する
extern int NS_InitShaderConstantBuffer( void )
{
	if( GSYS.InitializeFlag == FALSE ) return 0 ;

	return AllHandleSub( DX_HANDLETYPE_SHADER_CONSTANT_BUFFER ) ;
}

// シェーダー用定数バッファハンドルを初期化する
extern int NS_CreateShaderConstantBuffer(	int BufferSize )
{
	return Graphics_ShaderConstantBuffer_Create( BufferSize, GetASyncLoadFlag() ) ;
}

// シェーダー用定数バッファハンドルの後始末
extern int NS_DeleteShaderConstantBuffer( int SConstBufHandle )
{
	return SubHandle( SConstBufHandle ) ;
}

// シェーダー用定数バッファハンドルの定数バッファのアドレスを取得する
extern void *NS_GetBufferShaderConstantBuffer(	int SConstBufHandle )
{
	SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer ;

	// エラー判定
	if( SHADERCONSTANTBUFFERCHK( SConstBufHandle, ShaderConstantBuffer ) )
		return NULL ;

	// 環境依存処理
	return Graphics_Hardware_ShaderConstantBuffer_GetBuffer_PF( ShaderConstantBuffer ) ;
}

// シェーダー用定数バッファハンドルの定数バッファへの変更を適用する
extern int NS_UpdateShaderConstantBuffer( int SConstBufHandle )
{
	SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer ;

	// エラー判定
	if( SHADERCONSTANTBUFFERCHK( SConstBufHandle, ShaderConstantBuffer ) )
		return -1 ;

	// 環境依存処理
	return Graphics_Hardware_ShaderConstantBuffer_Update_PF( ShaderConstantBuffer ) ;
}

// シェーダー用定数バッファハンドルの定数バッファを指定のシェーダーの指定のスロットにセットする
extern int NS_SetShaderConstantBuffer( int SConstBufHandle, int TargetShader /* DX_SHADERTYPE_VERTEX など */ , int Slot )
{
	SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer ;

	// エラー判定
	if( SHADERCONSTANTBUFFERCHK( SConstBufHandle, ShaderConstantBuffer ) )
		return -1 ;

	// 環境依存処理
	return Graphics_Hardware_ShaderConstantBuffer_Set_PF( ShaderConstantBuffer, TargetShader, Slot ) ;
}
























#ifndef DX_NON_MOVIE

// ムービーグラフィック関係関数
// 動画ファイルの再生
extern int NS_PlayMovie( const TCHAR *FileName, int ExRate, int PlayType )
{
#ifdef UNICODE
	return PlayMovie_WCHAR_T(
		FileName, ExRate, PlayType
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = PlayMovie_WCHAR_T(
		UseFileNameBuffer, ExRate, PlayType
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// ムービーグラフィック関係関数
// 動画ファイルの再生
extern int PlayMovie_WCHAR_T( const wchar_t *FileName, int ExRate, int PlayType )
{
	int MovieHandle ;
	LOADGRAPH_GPARAM GParam ;
	int x1, y1, x2, y2, w, h ;
	int DrawScreen ;
	int DrawScreenSurface ;
#ifndef DX_NON_INPUT
	int KeyOffFlag ;
	int State ;
#endif // DX_NON_INPUT
	int ScreenW, ScreenH ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;

	MovieHandle = Graphics_Image_LoadBmpToGraph_UseGParam( &GParam, FALSE, -1, FileName, FALSE, FALSE, DX_MOVIESURFACE_NORMAL, FALSE ) ;
	if( MovieHandle < 0 )
		return -1 ;

#ifndef DX_NON_INPUT
	KeyOffFlag = NS_CheckHitKeyAll() ;
#endif // DX_NON_INPUT
	if( MovieHandle != -1 )
	{
		NS_PlayMovieToGraph( MovieHandle ) ;

		NS_GetDrawScreenSize( &ScreenW, &ScreenH ) ;
		NS_GetGraphSize( MovieHandle, &w, &h ) ;
		x1 = ( ScreenW - w * ExRate ) >> 1 ;
		y1 = ( ScreenH - h * ExRate ) >> 1 ;

		x2 = x1 + w * ExRate ;
		y2 = y1 + h * ExRate ;

		DrawScreen = GSYS.DrawSetting.TargetScreen[ 0 ] ;
		DrawScreenSurface = GSYS.DrawSetting.TargetScreenSurface[ 0 ] ;

		NS_SetDrawScreen( DX_SCREEN_BACK ) ;
		NS_ClearDrawScreen() ;

		while( NS_GetMovieStateToGraph( MovieHandle ) == 1 )
		{
//			NS_DrawBox( 0,0,32,32, GetColor( 255,255,255 ), TRUE ) ;
			NS_DrawExtendGraph( x1, y1, x2, y2, MovieHandle, FALSE ) ;

			NS_ScreenFlip() ;

#ifndef DX_NON_INPUT
			State = NS_CheckHitKeyAll() ;
			if( State == FALSE ) 
			{
				KeyOffFlag = FALSE ;
			}
#endif // DX_NON_INPUT

			if( ProcessMessage() != 0 
#ifndef DX_NON_INPUT
				|| ( PlayType == DX_MOVIEPLAYTYPE_BCANCEL &&
#ifdef __WINDOWS__
				GetActiveFlag() && 
#endif // __WINDOWS__
				State && KeyOffFlag == FALSE )
#endif // DX_NON_INPUT
				) break ;
		}
		NS_DeleteGraph( MovieHandle ) ;

		NS_SetRenderTargetToShader( 0, DrawScreen, DrawScreenSurface ) ;
	}

	// 終了
	return 0;
}

// ムービーを開く
extern int NS_OpenMovieToGraph( const TCHAR *FileName, int FullColor )
{
	return NS_LoadBmpToGraph( FileName, FALSE, FALSE, FullColor ? DX_MOVIESURFACE_FULLCOLOR : DX_MOVIESURFACE_NORMAL );
}

// ムービーを開く
extern int OpenMovieToGraph_WCHAR_T( const wchar_t *FileName, int FullColor )
{
	return LoadBmpToGraph_WCHAR_T( FileName, FALSE, FALSE, FullColor ? DX_MOVIESURFACE_FULLCOLOR : DX_MOVIESURFACE_NORMAL );
}

// ムービーグラフィックに含まれるムービーの再生を開始する
extern 	int		NS_PlayMovieToGraph( int GraphHandle, int PlayType, int SysPlay  )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return  0 ;

	return PlayMovie_( Image->MovieHandle, PlayType, SysPlay ) ;
}

// ムービーグラフィックに含まれるムービーの再生をストップする
extern 	int		NS_PauseMovieToGraph( int GraphHandle, int SysPause  )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return  0 ;

	return PauseMovie( Image->MovieHandle, SysPause ) ;
}

// ムービーのフレームを進める、戻すことは出来ない( ムービーが停止状態で、且つ Ogg Theora のみ有効 )
extern	int		NS_AddMovieFrameToGraph( int GraphHandle, unsigned int FrameNum )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return 0 ;

	return AddMovieFrame( Image->MovieHandle, FrameNum ) ;
}

// ムービーの再生位置を設定する(ミリ秒単位)
extern	int		NS_SeekMovieToGraph( int GraphHandle, int Time )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return 0 ;

	return SeekMovie( Image->MovieHandle, Time ) ;
}

// 動画ファイルの再生速度を設定する( 1.0 = 等倍速  2.0 = ２倍速 )、一部のファイルフォーマットのみで有効な機能です
extern	int		NS_SetPlaySpeedRateMovieToGraph( int GraphHandle, double SpeedRate )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return 0 ;

	return SetPlaySpeedRateMovie( Image->MovieHandle, SpeedRate ) ;
}

// ムービーの再生状態を得る
extern 	int		NS_GetMovieStateToGraph( int GraphHandle )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return 0 ;

	return GetMovieState( Image->MovieHandle ) ;
}

// ムービーのボリュームをセットする(0〜10000)
extern	int		NS_SetMovieVolumeToGraph( int Volume, int GraphHandle )	
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return 0 ;

	return SetMovieVolume( Volume, Image->MovieHandle ) ;
}

// ムービーのボリュームをセットする(0〜255)
extern	int		NS_ChangeMovieVolumeToGraph( int Volume, int GraphHandle )
{
	IMAGEDATA *Image ;
	int temp ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return 0 ;

	if( Volume < 0 )
	{
		Volume = 0 ;
	}
	else
	if( Volume > 255 )
	{
		Volume = 255 ;
	}

	if( Volume == 0 )
	{
		temp = 0 ;
	}
	else
	{
		if( SoundSysData.OldVolumeTypeFlag )
		{
			temp = _DTOL( _LOG10( Volume / 255.0f ) * 10.0 * 100.0 ) + 10000;
		}
		else
		{
			temp = _DTOL( _LOG10( Volume / 255.0f ) * 50.0 * 100.0 ) + 10000;
		}
	}

	return SetMovieVolume( temp, Image->MovieHandle ) ;
}

// ムービーの基本イメージデータを取得する
extern	const BASEIMAGE *NS_GetMovieBaseImageToGraph( int GraphHandle, int *ImageUpdateFlag )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return NULL ;

	if( Image->MovieHandle < 0 ) return  NULL ;

	return GetMovieBaseImage( Image->MovieHandle, ImageUpdateFlag ) ;
}

// ムービーの総フレーム数を得る( Ogg Theora でのみ有効 )
extern	int		NS_GetMovieTotalFrameToGraph( int GraphHandle )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return 0 ;

	return GetMovieTotalFrame( Image->MovieHandle ) ;
}

// ムービーの再生位置を取得する(ミリ秒単位)
extern	int		NS_TellMovieToGraph( int GraphHandle )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return  0 ;

	return TellMovie( Image->MovieHandle ) ;
}

// ムービーの再生位置を取得する(フレーム単位)
extern	int		NS_TellMovieToGraphToFrame( int GraphHandle )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return  0 ;

	return TellMovieToFrame( Image->MovieHandle ) ;
}

// ムービーの再生位置を設定する(フレーム単位)
extern	int		NS_SeekMovieToGraphToFrame( int GraphHandle, int Frame )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return  0 ;

	return SeekMovieToFrame( Image->MovieHandle, Frame ) ;
}

// ムービーの１フレームあたりの時間を得る
extern	LONGLONG NS_GetOneFrameTimeMovieToGraph( int GraphHandle )
{
	IMAGEDATA *Image ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return  0 ;

	return GetOneFrameTimeMovie( Image->MovieHandle ) ;
}

// ムービーのイメージを最後に更新した時間を得る(ミリ秒単位)
extern int NS_GetLastUpdateTimeMovieToGraph( int GraphHandle )
{
	IMAGEDATA *Image ;
	MOVIEGRAPH *Movie ;

	if( GRAPHCHK( GraphHandle, Image ) )
		return -1 ;

	if( Image->MovieHandle < 0 ) return  -1 ;

	Movie = GetMovieData( Image->MovieHandle ) ;
	if( Movie == NULL ) return -1 ;

	return Movie->RefreshTime ;
}

#endif // DX_NON_MOVIE



















// カメラ関係関数

// カメラの Nearクリップ面と Farクリップ面の距離を設定する
extern int NS_SetCameraNearFar( float Near, float Far )
{
	return NS_SetCameraNearFarD( Near, Far ) ;
}

// カメラの Nearクリップ面と Farクリップ面の距離を設定する
extern int NS_SetCameraNearFarD( double Near, double Far )
{
	// パラメータの保存
	GSYS.DrawSetting.ProjNear = Near ;
	GSYS.DrawSetting.ProjFar = Far ;

	// 射影行列の更新
	Graphics_DrawSetting_RefreshProjectionMatrix() ;

	// 終了
	return 0 ;
}

// カメラの設定をする、視点、注視点、アップベクトルはＹ軸版
extern int NS_SetCameraPositionAndTarget_UpVecY( VECTOR Position, VECTOR Target )
{
	return NS_SetCameraPositionAndTarget_UpVecYD( VConvFtoD( Position ), VConvFtoD( Target ) ) ;
}

// カメラの設定をする、視点、注視点、アップベクトルはＹ軸版
extern int NS_SetCameraPositionAndTarget_UpVecYD( VECTOR_D Position, VECTOR_D Target )
{
	VECTOR_D Up = { 0.0, 1.0, 0.0 }, Side, Dir ;
	MATRIX_D LookAt ;

	// 視点と注視点を保存
	GSYS.Camera.Position = Position ;
	GSYS.Camera.Target   = Target ;

	// アップベクトルを算出
	VectorSubD( &Dir, &GSYS.Camera.Target, &GSYS.Camera.Position ) ;
	VectorOuterProductD( &Side, &Dir, &Up ) ;
	VectorOuterProductD( &GSYS.Camera.Up, &Side, &Dir ) ;
	VectorNormalizeD( &GSYS.Camera.Up, &GSYS.Camera.Up ) ;

	// 行列の作成
	CreateLookAtMatrixD( &LookAt, &GSYS.Camera.Position, &GSYS.Camera.Target, &GSYS.Camera.Up ) ;

	// ビュー行列にセット
	NS_SetTransformToViewD( &LookAt ) ;
	GSYS.Camera.Matrix = LookAt ;

	// 回転角度を算出
	Graphics_Camera_CalcCameraRollViewMatrix() ;

	// 終了
	return 0 ;
}

// カメラの設定をする、視点、注視点、アップベクトル指定版
extern int NS_SetCameraPositionAndTargetAndUpVec( VECTOR Position, VECTOR Target, VECTOR Up )
{
	return NS_SetCameraPositionAndTargetAndUpVecD( VConvFtoD( Position ), VConvFtoD( Target ), VConvFtoD( Up ) ) ;
}

// カメラの設定をする、視点、注視点、アップベクトル指定版
extern int NS_SetCameraPositionAndTargetAndUpVecD( VECTOR_D Position, VECTOR_D Target, VECTOR_D Up )
{
	VECTOR_D Side, Dir ;
	MATRIX_D LookAt ;

	// 視点と注視点を保存
	GSYS.Camera.Position = Position ;
	GSYS.Camera.Target   = Target ;

	// アップベクトルを算出
	VectorSubD( &Dir, &Target, &Position ) ;
	VectorOuterProductD( &Side, &Dir, &Up ) ;
	VectorOuterProductD( &GSYS.Camera.Up, &Side, &Dir ) ;
	VectorNormalizeD( &GSYS.Camera.Up, &GSYS.Camera.Up ) ;

	// 行列の作成
	CreateLookAtMatrixD( &LookAt, &GSYS.Camera.Position, &GSYS.Camera.Target, &GSYS.Camera.Up ) ;

	// ビュー行列にセット
	NS_SetTransformToViewD( &LookAt ) ;
	GSYS.Camera.Matrix = LookAt ;

	// 回転角度を算出
	Graphics_Camera_CalcCameraRollViewMatrix() ;

	// 終了
	return 0 ;
}

// カメラの設定をする、垂直回転角度、水平回転角度、捻り回転角度、視点指定版
extern int NS_SetCameraPositionAndAngle( VECTOR Position, float VRotate, float HRotate, float TRotate )
{
	return NS_SetCameraPositionAndAngleD( VConvFtoD( Position ), VRotate, HRotate, TRotate ) ;
}

// カメラの設定をする、垂直回転角度、水平回転角度、捻り回転角度、視点指定版
extern int NS_SetCameraPositionAndAngleD( VECTOR_D Position, double VRotate, double HRotate, double TRotate )
{
	VECTOR_D ZVec, YVec, XVec, TVec, Target ;
	MATRIX_D LookAt ;
	double Sin, Cos ;

	// 水平回転角度、垂直回転角度、捻り回転角度、視点を保存する
	GSYS.Camera.HRotate = HRotate ;
	GSYS.Camera.VRotate = VRotate ;
	GSYS.Camera.TRotate = TRotate ;
	GSYS.Camera.Position = Position ;

	// 行列の作成
	_SINCOSD( VRotate, &Sin, &Cos ) ;
	ZVec.x  =  0.0 ;
	ZVec.y  = -Sin ;
	ZVec.z  =  Cos ;

	YVec.z  =  Sin ;
	YVec.y  =  Cos ;
	YVec.x  =  0.0 ;

	_SINCOSD( HRotate, &Sin, &Cos ) ;
	ZVec.x  =  Sin * ZVec.z ;
	ZVec.z *=  Cos ;

	YVec.x  =  Sin * YVec.z ;
	YVec.z *=  Cos ;

	XVec.x  =  Cos ;
	XVec.y  =  0.0 ;
	XVec.z  = -Sin ;

	_SINCOSD( TRotate, &Sin, &Cos ) ;
	TVec.x = -Sin * XVec.x + Cos * YVec.x ;
	TVec.y = -Sin * XVec.y + Cos * YVec.y ;
	TVec.z = -Sin * XVec.z + Cos * YVec.z ;

	VectorAddD( &Target, &Position, &ZVec ) ;
	CreateLookAtMatrixD( &LookAt, &Position, &Target, &TVec ) ;

	// ビュー行列にセット
	NS_SetTransformToViewD( &LookAt ) ;
	GSYS.Camera.Matrix = LookAt ;

	// 視点と注視点を保存
	GSYS.Camera.Position = Position ;
	GSYS.Camera.Target   = Target ;
	GSYS.Camera.Up       = TVec ;

//	Graphics_Camera_CalcCameraRollViewMatrix() ;

	// 終了
	return 0 ;
}

// ビュー行列を直接設定する
extern int NS_SetCameraViewMatrix( MATRIX ViewMatrix )
{
	MATRIX_D ViewMatrixD ;

	ConvertMatrixFtoD( &ViewMatrixD, &ViewMatrix ) ;

	// ビュー行列にセット
	NS_SetCameraViewMatrixD( ViewMatrixD ) ;

	// 終了
	return 0 ;
}

// ビュー行列を直接設定する
extern int NS_SetCameraViewMatrixD( MATRIX_D ViewMatrix )
{
	// パラメータを保存
	GSYS.Camera.Matrix = ViewMatrix ;

	// ビュー行列にセット
	NS_SetTransformToViewD( &ViewMatrix ) ;

	// 終了
	return 0 ;
}

// 画面上におけるカメラが見ている映像の中心の座標を設定する
extern int NS_SetCameraScreenCenter( float x, float y )
{
	return NS_SetCameraScreenCenterD( x, y ) ;
}

// 画面上におけるカメラが見ている映像の中心の座標を設定する
extern int NS_SetCameraScreenCenterD( double x, double y )
{
	MATRIX_D ViewportMatrix ;

	// 値を保存
	GSYS.Camera.ScreenCenterX = x ;
	GSYS.Camera.ScreenCenterY = y ;

	// ビューポート行列の作成
	CreateViewportMatrixD(
		&ViewportMatrix,
		x, y, 
		GSYS.DrawSetting.DrawSizeX * GSYS.DrawSetting.Draw3DScale,
		GSYS.DrawSetting.DrawSizeY * GSYS.DrawSetting.Draw3DScale ) ;

	// ビューポー行列としてセット
	NS_SetTransformToViewportD( &ViewportMatrix ) ;

	// 終了
	return 0 ;
}






// 遠近法カメラをセットアップする
extern int NS_SetupCamera_Perspective( float Fov )
{
	return NS_SetupCamera_PerspectiveD( Fov ) ;
}

// 遠近法カメラをセットアップする
extern int NS_SetupCamera_PerspectiveD( double Fov )
{
	// パラメータの保存
	GSYS.DrawSetting.ProjectionMatrixMode = 0 ;
	GSYS.DrawSetting.ProjFov = Fov ;

	// 射影行列の更新
	Graphics_DrawSetting_RefreshProjectionMatrix() ;

	// 終了
	return 0 ;
}

// 正射影カメラをセットアップする
extern int NS_SetupCamera_Ortho( float Size )
{
	return NS_SetupCamera_OrthoD( Size ) ;
}

// 正射影カメラをセットアップする
extern int NS_SetupCamera_OrthoD( double Size )
{
	// パラメータの保存
	GSYS.DrawSetting.ProjectionMatrixMode = 1 ;
	GSYS.DrawSetting.ProjSize = Size ;

	// 射影行列の更新
	Graphics_DrawSetting_RefreshProjectionMatrix() ;

	// 終了
	return 0 ;
}

// 射影行列を設定する
extern int NS_SetupCamera_ProjectionMatrix( MATRIX ProjectionMatrix )
{
	MATRIX_D ProjectionMatrixD ;

	ConvertMatrixFtoD( &ProjectionMatrixD, &ProjectionMatrix ) ;

	return NS_SetupCamera_ProjectionMatrixD( ProjectionMatrixD ) ;
}

// 射影行列を設定する
extern int NS_SetupCamera_ProjectionMatrixD( MATRIX_D ProjectionMatrix )
{
	// パラメータの保存
	GSYS.DrawSetting.ProjectionMatrixMode = 2 ;
	GSYS.DrawSetting.ProjMatrix = ProjectionMatrix ;

	// 射影行列の更新
	Graphics_DrawSetting_RefreshProjectionMatrix() ;

	// 終了
	return 0 ;
}

// カメラのドットアスペクト比を設定する
extern int NS_SetCameraDotAspect( float DotAspect )
{
	return NS_SetCameraDotAspectD( DotAspect ) ;
}

// カメラのドットアスペクト比を設定する
extern int NS_SetCameraDotAspectD( double DotAspect )
{
	// パラメータの保存
	GSYS.DrawSetting.ProjDotAspect = DotAspect ;

	// 射影行列の更新
	Graphics_DrawSetting_RefreshProjectionMatrix() ;

	// 終了
	return 0 ;
}





// 指定の座標がカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )
extern int NS_CheckCameraViewClip( VECTOR CheckPos )
{
	if( GSYS.DrawSetting.ClippingPlane[ 0 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 0 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 0 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 0 ].w < 0.0f ||
		GSYS.DrawSetting.ClippingPlane[ 1 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 1 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 1 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 1 ].w < 0.0f ||
		GSYS.DrawSetting.ClippingPlane[ 2 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 2 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 2 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 2 ].w < 0.0f ||
		GSYS.DrawSetting.ClippingPlane[ 3 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 3 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 3 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 3 ].w < 0.0f ||
		GSYS.DrawSetting.ClippingPlane[ 4 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 4 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 4 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 4 ].w < 0.0f ||
		GSYS.DrawSetting.ClippingPlane[ 5 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 5 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 5 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 5 ].w < 0.0f )
		return TRUE ;

	return FALSE ;
}

// 指定の座標がカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )
extern int NS_CheckCameraViewClipD( VECTOR_D CheckPos )
{
	if( GSYS.DrawSetting.ClippingPlane[ 0 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 0 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 0 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 0 ].w < 0.0 ||
		GSYS.DrawSetting.ClippingPlane[ 1 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 1 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 1 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 1 ].w < 0.0 ||
		GSYS.DrawSetting.ClippingPlane[ 2 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 2 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 2 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 2 ].w < 0.0 ||
		GSYS.DrawSetting.ClippingPlane[ 3 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 3 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 3 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 3 ].w < 0.0 ||
		GSYS.DrawSetting.ClippingPlane[ 4 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 4 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 4 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 4 ].w < 0.0 ||
		GSYS.DrawSetting.ClippingPlane[ 5 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 5 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 5 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 5 ].w < 0.0 )
		return TRUE ;

	return FALSE ;
}

// 指定の座標がカメラの視界に入っているかどうかを判定する、戻り値で外れている方向も知ることができる( 戻り値 0:視界に入っている  0以外:視界に入っていない( DX_CAMERACLIP_LEFT や DX_CAMERACLIP_RIGHT が or 演算で混合されたもの、and 演算で方向を確認できる ) )
extern int NS_CheckCameraViewClip_Dir( VECTOR CheckPos )
{
	int clip ;

	clip = 0 ;
	if( GSYS.DrawSetting.ClippingPlane[ 0 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 0 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 0 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 0 ].w < 0.0f ) clip |= DX_CAMERACLIP_LEFT ;
	if( GSYS.DrawSetting.ClippingPlane[ 1 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 1 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 1 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 1 ].w < 0.0f ) clip |= DX_CAMERACLIP_RIGHT ;
	if( GSYS.DrawSetting.ClippingPlane[ 2 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 2 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 2 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 2 ].w < 0.0f ) clip |= DX_CAMERACLIP_BOTTOM ;
	if( GSYS.DrawSetting.ClippingPlane[ 3 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 3 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 3 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 3 ].w < 0.0f ) clip |= DX_CAMERACLIP_TOP ;
	if( GSYS.DrawSetting.ClippingPlane[ 4 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 4 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 4 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 4 ].w < 0.0f ) clip |= DX_CAMERACLIP_BACK ;
	if( GSYS.DrawSetting.ClippingPlane[ 5 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 5 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 5 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 5 ].w < 0.0f ) clip |= DX_CAMERACLIP_FRONT ;

	return clip ;
}

// 指定の座標がカメラの視界に入っているかどうかを判定する、戻り値で外れている方向も知ることができる( 戻り値 0:視界に入っている  0以外:視界に入っていない( DX_CAMERACLIP_LEFT や DX_CAMERACLIP_RIGHT が or 演算で混合されたもの、and 演算で方向を確認できる ) )
extern int NS_CheckCameraViewClip_DirD( VECTOR_D CheckPos )
{
	int clip ;

	clip = 0 ;
	if( GSYS.DrawSetting.ClippingPlane[ 0 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 0 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 0 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 0 ].w < 0.0 ) clip |= DX_CAMERACLIP_LEFT ;
	if( GSYS.DrawSetting.ClippingPlane[ 1 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 1 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 1 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 1 ].w < 0.0 ) clip |= DX_CAMERACLIP_RIGHT ;
	if( GSYS.DrawSetting.ClippingPlane[ 2 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 2 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 2 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 2 ].w < 0.0 ) clip |= DX_CAMERACLIP_BOTTOM ;
	if( GSYS.DrawSetting.ClippingPlane[ 3 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 3 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 3 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 3 ].w < 0.0 ) clip |= DX_CAMERACLIP_TOP ;
	if( GSYS.DrawSetting.ClippingPlane[ 4 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 4 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 4 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 4 ].w < 0.0 ) clip |= DX_CAMERACLIP_BACK ;
	if( GSYS.DrawSetting.ClippingPlane[ 5 ].x * CheckPos.x + GSYS.DrawSetting.ClippingPlane[ 5 ].y * CheckPos.y + GSYS.DrawSetting.ClippingPlane[ 5 ].z * CheckPos.z + GSYS.DrawSetting.ClippingPlane[ 5 ].w < 0.0 ) clip |= DX_CAMERACLIP_FRONT ;

	return clip ;
}


// 二つの座標で表されるボックスがカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )
extern int NS_CheckCameraViewClip_Box( VECTOR BoxPos1, VECTOR BoxPos2 )
{
	VECTOR CheckBoxPos[ 8 ] ;
	VECTOR Tmp ;

	Tmp   = BoxPos2   ; CheckBoxPos[ 0 ] = Tmp ;
	Tmp.x = BoxPos1.x ; CheckBoxPos[ 1 ] = Tmp ;
	Tmp.y = BoxPos1.y ; CheckBoxPos[ 2 ] = Tmp ;
	Tmp.x = BoxPos2.x ; CheckBoxPos[ 3 ] = Tmp ;
	Tmp.z = BoxPos1.z ; CheckBoxPos[ 4 ] = Tmp ;
	Tmp.y = BoxPos2.y ; CheckBoxPos[ 5 ] = Tmp ;
	Tmp.x = BoxPos1.x ; CheckBoxPos[ 6 ] = Tmp ;
	Tmp.y = BoxPos1.y ; CheckBoxPos[ 7 ] = Tmp ;

	return Graphics_Camera_CheckCameraViewClip_Box_PosDim( CheckBoxPos ) ;
}

// 二つの座標で表されるボックスがカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )
extern int NS_CheckCameraViewClip_BoxD( VECTOR_D BoxPos1, VECTOR_D BoxPos2 )
{
	VECTOR_D CheckBoxPos[ 8 ] ;
	VECTOR_D Tmp ;

	Tmp   = BoxPos2   ; CheckBoxPos[ 0 ] = Tmp ;
	Tmp.x = BoxPos1.x ; CheckBoxPos[ 1 ] = Tmp ;
	Tmp.y = BoxPos1.y ; CheckBoxPos[ 2 ] = Tmp ;
	Tmp.x = BoxPos2.x ; CheckBoxPos[ 3 ] = Tmp ;
	Tmp.z = BoxPos1.z ; CheckBoxPos[ 4 ] = Tmp ;
	Tmp.y = BoxPos2.y ; CheckBoxPos[ 5 ] = Tmp ;
	Tmp.x = BoxPos1.x ; CheckBoxPos[ 6 ] = Tmp ;
	Tmp.y = BoxPos1.y ; CheckBoxPos[ 7 ] = Tmp ;

	return Graphics_Camera_CheckCameraViewClip_Box_PosDimD( CheckBoxPos ) ;
}

// カメラの Near クリップ面の距離を取得する
extern float NS_GetCameraNear( void )
{
	return ( float )GSYS.DrawSetting.ProjNear ;
}

// カメラの Near クリップ面の距離を取得する
extern double NS_GetCameraNearD( void )
{
	return GSYS.DrawSetting.ProjNear ;
}

// カメラの Far クリップ面の距離を取得する
extern float NS_GetCameraFar( void )
{
	return ( float )GSYS.DrawSetting.ProjFar ;
}

// カメラの Far クリップ面の距離を取得する
extern double NS_GetCameraFarD( void )
{
	return GSYS.DrawSetting.ProjFar ;
}

// カメラの位置を取得する
extern	VECTOR	NS_GetCameraPosition( void )
{
	return VConvDtoF( GSYS.Camera.Position ) ;
}

// カメラの位置を取得する
extern	VECTOR_D NS_GetCameraPositionD( void )
{
	return GSYS.Camera.Position ;
}

// カメラの注視点を取得する
extern	VECTOR	NS_GetCameraTarget( void )
{
	return VConvDtoF( GSYS.Camera.Target ) ;
}

// カメラの注視点を取得する
extern	VECTOR_D NS_GetCameraTargetD( void )
{
	return GSYS.Camera.Target ;
}

// カメラの視線に垂直な上方向のベクトルを得る
extern	VECTOR	NS_GetCameraUpVector( void )
{
	return VConvDtoF( GSYS.Camera.Up ) ;
}

// カメラの視線に垂直な上方向のベクトルを得る
extern	VECTOR_D NS_GetCameraUpVectorD( void )
{
	return GSYS.Camera.Up ;
}

// カメラの水平方向の向きを取得する
extern	float	NS_GetCameraAngleHRotate( void )
{
	return ( float )GSYS.Camera.HRotate ;
}

// カメラの水平方向の向きを取得する
extern	double NS_GetCameraAngleHRotateD( void )
{
	return GSYS.Camera.HRotate ;
}

// カメラの垂直方向の向きを取得する
extern	float	NS_GetCameraAngleVRotate( void )
{
	return ( float )GSYS.Camera.VRotate ;
}

// カメラの垂直方向の向きを取得する
extern	double NS_GetCameraAngleVRotateD( void )
{
	return GSYS.Camera.VRotate ;
}

// カメラの向きの捻り角度を取得する
extern	float	NS_GetCameraAngleTRotate( void )
{
	return ( float )GSYS.Camera.TRotate ;
}

// カメラの向きの捻り角度を取得する
extern	double NS_GetCameraAngleTRotateD( void )
{
	return GSYS.Camera.TRotate ;
}




// ビュー行列を取得する
extern MATRIX NS_GetCameraViewMatrix( void )
{
	MATRIX Result ;

	ConvertMatrixDtoF( &Result, &GSYS.DrawSetting.ViewMatrix ) ;
	return Result ;
}

// ビュー行列を取得する
extern MATRIX_D NS_GetCameraViewMatrixD( void )
{
	return GSYS.DrawSetting.ViewMatrix ;
}

// ビルボード行列を取得する
extern MATRIX NS_GetCameraBillboardMatrix( void )
{
	return GSYS.DrawSetting.BillboardMatrixF ;
}

// ビルボード行列を取得する
extern MATRIX_D NS_GetCameraBillboardMatrixD( void )
{
	return GSYS.DrawSetting.BillboardMatrix ;
}

// 画面上におけるカメラが見ている映像の中心の座標を取得する
extern int NS_GetCameraScreenCenter( float  *x, float  *y )
{
	if( x != NULL )
	{
		*x = ( float )GSYS.Camera.ScreenCenterX ;
	}

	if( y != NULL )
	{
		*y = ( float )GSYS.Camera.ScreenCenterY ;
	}

	return 0 ;
}

// 画面上におけるカメラが見ている映像の中心の座標を取得する
extern int NS_GetCameraScreenCenterD( double *x, double *y )
{
	if( x != NULL )
	{
		*x = GSYS.Camera.ScreenCenterX ;
	}

	if( y != NULL )
	{
		*y = GSYS.Camera.ScreenCenterY ;
	}

	return 0 ;
}

// カメラの視野角を取得する
extern float NS_GetCameraFov( void )
{
	return ( float )GSYS.DrawSetting.ProjFov ;
}

// カメラの視野角を取得する
extern double NS_GetCameraFovD( void )
{
	return GSYS.DrawSetting.ProjFov ;
}

// カメラの視野サイズを取得する
extern float NS_GetCameraSize( void )
{
	return ( float )GSYS.DrawSetting.ProjSize ;
}

// カメラの視野サイズを取得する
extern double NS_GetCameraSizeD( void )
{
	return GSYS.DrawSetting.ProjSize ;
}

// 射影行列を取得する
extern MATRIX NS_GetCameraProjectionMatrix( void )
{
	return GSYS.DrawSetting.ProjectionMatrixF ;
}

// 射影行列を取得する
extern MATRIX_D NS_GetCameraProjectionMatrixD( void )
{
	return GSYS.DrawSetting.ProjectionMatrix ;
}

// カメラのドットアスペクト比を得る
extern float NS_GetCameraDotAspect( void )
{
	return ( float )GSYS.DrawSetting.ProjDotAspect ;
}

// カメラのドットアスペクト比を得る
extern double NS_GetCameraDotAspectD( void )
{
	return GSYS.DrawSetting.ProjDotAspect ;
}

// ビューポート行列を取得する
extern MATRIX NS_GetCameraViewportMatrix( void )
{
	return GSYS.DrawSetting.ViewportMatrixF ;
}

// ビューポート行列を取得する
extern MATRIX_D NS_GetCameraViewportMatrixD( void )
{
	return GSYS.DrawSetting.ViewportMatrix ;
}

// Direct3Dで自動適用されるビューポート行列を取得する
extern MATRIX NS_GetCameraAPIViewportMatrix( void )
{
	return GSYS.DrawSetting.Direct3DViewportMatrixF ;
}

// Direct3Dで自動適用されるビューポート行列を取得する
extern MATRIX_D NS_GetCameraAPIViewportMatrixD( void )
{
	return GSYS.DrawSetting.Direct3DViewportMatrix ;
}



























// ライト関係関数

// ライティング計算機能を使用するかどうかを設定する
extern int NS_SetUseLighting( int Flag )
{
//	if( GSYS.Light.ProcessDisable == !Flag )
//		return 0 ;
	GSYS.Light.ProcessDisable = Flag ? FALSE : TRUE ;

	// セット
	Graphics_Hardware_Light_SetUse_PF( Flag ) ;

	// ライトの設定が変更されたフラグを立てる
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライティング計算に頂点カラーのディフューズカラーを使用するかどうかを設定する
extern int NS_SetMaterialUseVertDifColor( int UseFlag )
{
	GSYS.Light.MaterialNotUseVertexDiffuseColor = UseFlag ? FALSE : TRUE ;
	
	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライティング計算に頂点カラーのスペキュラカラーを使用するかどうかを設定する
extern int NS_SetMaterialUseVertSpcColor( int UseFlag )
{
	GSYS.Light.MaterialNotUseVertexSpecularColor = UseFlag ? FALSE : TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライティング計算に使用するマテリアルパラメータを設定する
extern int NS_SetMaterialParam( MATERIALPARAM Material )
{
	GSYS.Light.ChangeMaterial = 1 ;
	GSYS.Light.Material = Material ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ３Ｄ図形描画にスペキュラを使用するかどうかを設定する
extern int NS_SetUseSpecular( int UseFlag )
{
	GSYS.DrawSetting.NotUseSpecular = UseFlag ? FALSE : TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// グローバルアンビエントライトカラーを設定する
extern int NS_SetGlobalAmbientLight( COLOR_F Color )
{
	     if( Color.r < 0.0f ) Color.r = 0.0f ;
	else if( Color.r > 1.0f ) Color.r = 1.0f ;
	     if( Color.g < 0.0f ) Color.g = 0.0f ;
	else if( Color.g > 1.0f ) Color.g = 1.0f ;
	     if( Color.b < 0.0f ) Color.b = 0.0f ;
	else if( Color.b > 1.0f ) Color.b = 1.0f ;

	// ハードウエアに設定する
	Graphics_Hardware_Light_GlobalAmbient_PF( &Color ) ;

	// 終了
	return 0 ;
}

// ライトのタイプをディレクショナルライトにする
extern int NS_ChangeLightTypeDir( VECTOR Direction )
{
	LIGHT_HANDLE *pLH ;
	float Length ;

	LIGHTCHK( GSYS.Light.DefaultHandle, pLH, -1 ) ;

	// タイプを変更
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_DIRECTIONAL ;

	// 向きをセット
	Length = _SQRT( VectorInnerProduct( &Direction, &Direction ) ) ;
	Direction.x /= Length ;
	Direction.y /= Length ;
	Direction.z /= Length ;
	pLH->Param.Direction = Direction ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトのタイプをスポットライトにする
extern int NS_ChangeLightTypeSpot( VECTOR Position, VECTOR Direction, float OutAngle, float InAngle/*, float Falloff*/, float Range, float Atten0, float Atten1, float Atten2 )
{
	float Falloff = 1.0f ;

	LIGHT_HANDLE *pLH ;
	LIGHTCHK( GSYS.Light.DefaultHandle, pLH, -1 ) ;

	// タイプをセット
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_SPOT ;

	// パラメータをセット
	pLH->Param.Position = Position ;
	pLH->Param.Direction = Direction ;
	pLH->Param.Phi = OutAngle ;
	pLH->Param.Theta = InAngle ;
	pLH->Param.Falloff = Falloff ;
	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトのタイプをポイントライトにする
extern int NS_ChangeLightTypePoint( VECTOR Position, float Range, float Atten0, float Atten1, float Atten2 )
{
	LIGHT_HANDLE *pLH ;
	LIGHTCHK( GSYS.Light.DefaultHandle, pLH, -1 ) ;

	// タイプをセット
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_POINT ;

	// パラメータをセット
	pLH->Param.Range = Range ;
	pLH->Param.Position = Position ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトのタイプを取得する( 戻り値は DX_LIGHTTYPE_DIRECTIONAL 等 )
extern	int NS_GetLightType( void )
{
	return NS_GetLightTypeHandle( GSYS.Light.DefaultHandle ) ;
}

// ライトの有効、無効をセットする
extern int NS_SetLightEnable( int EnableFlag )
{
	return NS_SetLightEnableHandle( GSYS.Light.DefaultHandle, EnableFlag ) ;
}

// ライトハンドルのライト効果の有効、無効を設定する( TRUE:有効  FALSE:無効 )
extern	int NS_GetLightEnable( void )
{
	return NS_GetLightEnableHandle( GSYS.Light.DefaultHandle ) ;
}

// ライトのディフューズカラーを設定する
extern int NS_SetLightDifColor( COLOR_F Color )
{
	return NS_SetLightDifColorHandle( GSYS.Light.DefaultHandle, Color ) ;
}

// ライトハンドルのライトのディフューズカラーを設定する
extern COLOR_F NS_GetLightDifColor( void )
{
	return NS_GetLightDifColorHandle( GSYS.Light.DefaultHandle ) ;
}

// ライトのスペキュラカラーを設定する
extern int NS_SetLightSpcColor( COLOR_F Color )
{
	return NS_SetLightSpcColorHandle( GSYS.Light.DefaultHandle, Color ) ;
}

// ライトハンドルのライトのスペキュラカラーを設定する
extern COLOR_F NS_GetLightSpcColor(  void )
{
	return NS_GetLightSpcColorHandle( GSYS.Light.DefaultHandle ) ;
}

// ライトのアンビエントカラーを設定する
extern int NS_SetLightAmbColor( COLOR_F Color )
{
	return NS_SetLightAmbColorHandle( GSYS.Light.DefaultHandle, Color ) ;
}

// ライトハンドルのライトのアンビエントカラーを設定する
extern COLOR_F NS_GetLightAmbColor(  void )
{
	return NS_GetLightAmbColorHandle( GSYS.Light.DefaultHandle ) ;
}

// ライトの方向を設定する
extern int NS_SetLightDirection( VECTOR Direction )
{
	return NS_SetLightDirectionHandle( GSYS.Light.DefaultHandle, Direction ) ;
}

// ライトハンドルのライトの方向を設定する
extern	VECTOR NS_GetLightDirection( void )
{
	return NS_GetLightDirectionHandle( GSYS.Light.DefaultHandle ) ;
}

// ライトの位置を設定する
extern int NS_SetLightPosition( VECTOR Position )
{
	return NS_SetLightPositionHandle( GSYS.Light.DefaultHandle, Position ) ;
}

// ライトハンドルのライトの位置を設定する
extern	VECTOR NS_GetLightPosition( void )
{
	return NS_GetLightPositionHandle( GSYS.Light.DefaultHandle ) ;
}

// ライトの距離減衰パラメータを設定する( 有効距離、距離減衰係数０、１、２ )
extern int NS_SetLightRangeAtten( float Range, float Atten0, float Atten1, float Atten2 )
{
	return NS_SetLightRangeAttenHandle( GSYS.Light.DefaultHandle, Range,  Atten0,  Atten1,  Atten2 ) ;
}

// ライトの距離減衰パラメータを取得する( 有効距離、距離減衰係数０、１、２ )
extern int NS_GetLightRangeAtten( float *Range, float *Atten0, float *Atten1, float *Atten2 )
{
	return NS_GetLightRangeAttenHandle( GSYS.Light.DefaultHandle, Range, Atten0, Atten1, Atten2 ) ;
}

// ライトのスポットライトのパラメータを設定する( 外部コーン角度、内部コーン角度、フォールオフ( 1.0f を推奨 ) )
extern int NS_SetLightAngle( float OutAngle, float InAngle /*, float Falloff */ )
{
	return NS_SetLightAngleHandle( GSYS.Light.DefaultHandle,  OutAngle,  InAngle /*,  Falloff */ ) ;
}

// ライトのスポットライトのパラメータを取得する( 外部コーン角度、内部コーン角度、フォールオフ )
extern int NS_GetLightAngle( float *OutAngle, float *InAngle /*, float *Falloff */ )
{
	return NS_GetLightAngleHandle( GSYS.Light.DefaultHandle, OutAngle, InAngle /*, Falloff */ ) ;
}

// デフォルトライトに SetUseShadowMap で指定したシャドウマップを適用するかどうかを設定する( SmSlotIndex:シャドウマップスロット( SetUseShadowMap の第一引数に設定する値 ) UseFlag:適用にするかどうかのフラグ( TRUE:適用する( デフォルト )  FALSE:適用しない ) )
extern int NS_SetLightUseShadowMap( int SmSlotIndex, int UseFlag )
{
	return NS_SetLightUseShadowMapHandle( GSYS.Light.DefaultHandle, SmSlotIndex, UseFlag ) ;
}

// ディレクショナルライトハンドルを作成する
extern int NS_CreateDirLightHandle( VECTOR Direction )
{
	int LHandle ;
	LIGHT_HANDLE *pLH ;
	float Length ;

	// 新しいハンドルの追加
	LHandle = Graphics_Light_AddHandle() ;
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// タイプをセット
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_DIRECTIONAL ;

	// 向きをセット
	Length = _SQRT( VectorInnerProduct( &Direction, &Direction ) ) ;
	Direction.x /= Length ;
	Direction.y /= Length ;
	Direction.z /= Length ;
	pLH->Param.Direction = Direction ;

	// ハンドルを返す
	return LHandle ;
}

// スポットライトハンドルを作成する
extern int NS_CreateSpotLightHandle( VECTOR Position, VECTOR Direction, float OutAngle, float InAngle/*, float Falloff*/, float Range, float Atten0, float Atten1, float Atten2 )
{
	float Falloff = 1.0f ;
	int LHandle ;
	LIGHT_HANDLE *pLH ;

	// 新しいハンドルの追加
	LHandle = Graphics_Light_AddHandle() ;
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// タイプをセット
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_SPOT ;

	// パラメータをセット
	pLH->Param.Position = Position ;
	pLH->Param.Direction = Direction ;
	pLH->Param.Phi = OutAngle ;
	pLH->Param.Theta = InAngle ;
	pLH->Param.Falloff = Falloff ;
	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// ハンドルを返す
	return LHandle ;
}

// ポイントライトハンドルを作成する
extern int NS_CreatePointLightHandle( VECTOR Position, float Range, float Atten0, float Atten1, float Atten2 )
{
	int LHandle ;
	LIGHT_HANDLE *pLH ;

	// 新しいハンドルの追加
	LHandle = Graphics_Light_AddHandle() ;
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// タイプをセット
	pLH->Param.LightType = DX_LIGHTTYPE_D3DLIGHT_POINT ;

	// パラメータをセット
	pLH->Param.Position = Position ;
	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// ハンドルを返す
	return LHandle ;
}

// ライトハンドルを削除する
extern int NS_DeleteLightHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// ライトハンドルの総数を減らす
	GSYS.Light.Num -- ;

	// 有効なライトハンドルが存在する範囲の更新
	if( ( LHandle & DX_HANDLEINDEX_MASK ) == GSYS.Light.Area - 1 )
	{
		if( GSYS.Light.Num == 0 )
		{
			GSYS.Light.Area = 0 ;
		}
		else
		{
			while( GSYS.Light.Data[ GSYS.Light.Area - 1 ] == NULL )
				GSYS.Light.Area -- ; 
		}
	}

	// データ領域を解放する
	DXFREE( pLH ) ;

	// テーブルに NULL をセットする
	GSYS.Light.Data[ LHandle & DX_HANDLEINDEX_MASK ] = NULL ;

	// 変更フラグを立てる
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルを全て削除する
extern int NS_DeleteLightHandleAll( void )
{
	int i ;
	LIGHT_HANDLE **ppLH ;

	// 標準ライト以外のすべてのライトデータを削除する
	ppLH = GSYS.Light.Data ;
	for( i = 0 ; i < GSYS.Light.Area ; i ++, ppLH ++ )
	{
		if( *ppLH && ( *ppLH )->Handle != GSYS.Light.DefaultHandle )
			NS_DeleteLightHandle( ( *ppLH )->Handle ) ;
	}

	// 終了
	return 0 ;
}

// ライトハンドルのライトのタイプを変更する( DX_LIGHTTYPE_DIRECTIONAL 等 )
extern int NS_SetLightTypeHandle( int LHandle, int LightType )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// ライトタイプが不正だった場合はエラー
	if( LightType != DX_LIGHTTYPE_DIRECTIONAL &&
		LightType != DX_LIGHTTYPE_SPOT &&
		LightType != DX_LIGHTTYPE_POINT )
		return -1 ;

	// タイプをセット
	pLH->Param.LightType = LightType ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトの有効、無効を設定する( TRUE:有効  FALSE:無効 )
extern int NS_SetLightEnableHandle( int LHandle, int EnableFlag )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// 今までと同じ場合は何もせずに終了
	if( pLH->EnableFlag == EnableFlag )
		return 0 ;

	// フラグを保存
	pLH->EnableFlag = EnableFlag ;
	if( EnableFlag == FALSE )
	{
		pLH->SetHardwareIndex = -1 ;
	}

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトのディフューズカラーを設定する
extern int NS_SetLightDifColorHandle( int LHandle, COLOR_F Color )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	Color.a = 0.0f ;
	pLH->Param.Diffuse = Color ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトのスペキュラカラーを設定する
extern int NS_SetLightSpcColorHandle( int LHandle, COLOR_F Color )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	Color.a = 0.0f ;
	pLH->Param.Specular = Color ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトのアンビエントカラーを設定する
extern int NS_SetLightAmbColorHandle( int LHandle, COLOR_F Color )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	Color.a = 0.0f ;
	pLH->Param.Ambient = Color ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトの方向を設定する
extern int NS_SetLightDirectionHandle( int LHandle, VECTOR Direction )
{
	LIGHT_HANDLE *pLH ;
	float Length ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// 向きを保存する
	Length = _SQRT( VectorInnerProduct( &Direction, &Direction ) ) ;
	Direction.x /= Length ;
	Direction.y /= Length ;
	Direction.z /= Length ;
	pLH->Param.Direction = Direction ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトの位置を設定する
extern int NS_SetLightPositionHandle( int LHandle, VECTOR Position )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	pLH->Param.Position = Position ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトの距離減衰パラメータを設定する( 有効距離、距離減衰係数０、１、２ )
extern int NS_SetLightRangeAttenHandle( int LHandle, float Range, float Atten0, float Atten1, float Atten2 )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	pLH->Param.Range = Range ;
	pLH->Param.Attenuation0 = Atten0 ;
	pLH->Param.Attenuation1 = Atten1 ;
	pLH->Param.Attenuation2 = Atten2 ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトのスポットライトのパラメータを設定する( 外部コーン角度、内部コーン角度、フォールオフ( 1.0f を推奨 ) )
extern int NS_SetLightAngleHandle( int LHandle, float OutAngle, float InAngle /*, float Falloff */ )
{
	LIGHT_HANDLE *pLH ;
	float Falloff = 1.0f ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	pLH->Param.Phi = OutAngle ;
	pLH->Param.Theta = InAngle ;
	pLH->Param.Falloff = Falloff ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトに SetUseShadowMap で指定したシャドウマップを適用するかどうかを設定する
// ( SmSlotIndex:シャドウマップスロット( SetUseShadowMap の第一引数に設定する値 )
//   UseFlag:適用にするかどうかのフラグ( TRUE:適用する( デフォルト )  FALSE:適用しない ) )
extern int NS_SetLightUseShadowMapHandle( int LHandle, int SmSlotIndex, int UseFlag )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	// エラーチェック
	if( SmSlotIndex < 0 || SmSlotIndex >= MAX_USE_SHADOWMAP_NUM )
	{
		return -1 ;
	}

	// フラグセット
	pLH->ShadowMapSlotDisableFlag[ SmSlotIndex ] = UseFlag == FALSE ? TRUE : FALSE ;

	// 変更フラグを立てる
	pLH->HardwareChangeFlag = TRUE ;
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトのタイプを取得する( 戻り値は DX_LIGHTTYPE_DIRECTIONAL 等 )
extern int NS_GetLightTypeHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	return pLH->Param.LightType ;
}

// ライトハンドルのライト効果の有効、無効を取得する( TRUE:有効  FALSE:無効 )
extern int NS_GetLightEnableHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	return pLH->EnableFlag ;
}

// ライトハンドルのライトのディフューズカラーを取得する
extern COLOR_F NS_GetLightDifColorHandle( int LHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	// 色を返す
	return pLH->Param.Diffuse ;
}

// ライトハンドルのライトのスペキュラカラーを取得する
extern COLOR_F NS_GetLightSpcColorHandle( int LHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	// 色を返す
	return pLH->Param.Specular ;
}

// ライトハンドルのライトのアンビエントカラーを取得する
extern COLOR_F NS_GetLightAmbColorHandle( int LHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	// 色を返す
	return pLH->Param.Ambient ;
}

// ライトハンドルのライトの方向を取得する
extern VECTOR NS_GetLightDirectionHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;
	VECTOR ErrorResult = { -1.0f, -1.0f, -1.0f } ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	return pLH->Param.Direction ;
}

// ライトハンドルのライトの位置を取得する
extern VECTOR NS_GetLightPositionHandle( int LHandle )
{
	LIGHT_HANDLE *pLH ;
	VECTOR ErrorResult = { -1.0f, -1.0f, -1.0f } ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, ErrorResult ) ;

	return pLH->Param.Position ;
}

// ライトハンドルのライトの距離減衰パラメータを取得する( 有効距離、距離減衰係数０、１、２ )
extern int NS_GetLightRangeAttenHandle( int LHandle, float *Range, float *Atten0, float *Atten1, float *Atten2 )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	if( Range ) *Range = pLH->Param.Range ;
	if( Atten0 ) *Atten0 = pLH->Param.Attenuation0 ;
	if( Atten1 ) *Atten1 = pLH->Param.Attenuation1 ;
	if( Atten2 ) *Atten2 = pLH->Param.Attenuation2 ;

	// 終了
	return 0 ;
}

// ライトハンドルのライトのスポットライトのパラメータを取得する( 外部コーン角度、内部コーン角度、フォールオフ )
extern int NS_GetLightAngleHandle( int LHandle, float *OutAngle, float *InAngle /*, float *Falloff */ )
{
	LIGHT_HANDLE *pLH ;

	// エラー判定
	LIGHTCHK( LHandle, pLH, -1 ) ;

	if( OutAngle ) *OutAngle = pLH->Param.Phi ;
	if( InAngle  ) *InAngle  = pLH->Param.Theta ;
//	if( Falloff  ) *Falloff  = pLH->Param.Falloff ;

	// 終了
	return 0 ;
}

// 有効になっているライトハンドルの数を取得する
extern int NS_GetEnableLightHandleNum( void )
{
	// 更新フラグが立っていたら情報を更新する
	if( GSYS.Light.HardwareChange )
	{
		Graphics_Light_RefreshState() ;
	}

	// 有効になっているライトハンドルの数を返す
	return GSYS.Light.EnableNum ;
}

// 有効になっているライトハンドルを取得する
extern int NS_GetEnableLightHandle( int Index )
{
	// インデックスチェック
	if( Index < 0 || Index >= MAX_HARDWAREENABLELIGHTINDEX_NUM )
		return -1 ;

	// 更新フラグが立っていたら情報を更新する
	if( GSYS.Light.HardwareChange )
	{
		Graphics_Light_RefreshState() ;
	}

	// 範囲外の場合はエラー
	if( Index >= GSYS.Light.EnableNum )
		return -1 ;

	// ハンドル値を返す
	return GSYS.Light.Data[ GSYS.Light.EnableHardwareIndex[ Index ] ]->Handle ;
}

























// 色情報取得用関数

// テクスチャフォーマットのインデックスを得る
extern int NS_GetTexFormatIndex( const IMAGEFORMATDESC *Format )
{
	int Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB32 ;

	switch( Format->BaseFormat )
	{
	case DX_BASEIMAGE_FORMAT_DXT1 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT1 ;
	case DX_BASEIMAGE_FORMAT_DXT2 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT2 ;
	case DX_BASEIMAGE_FORMAT_DXT3 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT3 ;
	case DX_BASEIMAGE_FORMAT_DXT4 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT4 ;
	case DX_BASEIMAGE_FORMAT_DXT5 : return DX_GRAPHICSIMAGE_FORMAT_3D_DXT5 ;

	default:
		switch( Format->ChannelNum )
		{
		case 1 :
			if( Format->FloatTypeFlag )
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F32 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_F32 ;
					}
				}
			}
			else
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 16 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I16 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 16 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ONE_I16 ;
					}
				}
			}

		case 2 :
			if( Format->FloatTypeFlag )
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F32 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 64 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_F32 ;
					}
				}
			}
			else
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 16 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I16 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I8 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_TWO_I16 ;
					}
				}
			}

		case 4 :
		default :
			if( Format->FloatTypeFlag )
			{
				if( Format->ChannelBitDepth != 0 )
				{
					if( Format->ChannelBitDepth < 32 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F32 ;
					}
				}
				else
				{
					if( Format->ColorBitDepth < 128 )
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F16 ;
					}
					else
					{
						return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_F32 ;
					}
				}
			}
			else
			{
				if( Format->ChannelBitDepth >= 9 )
				{
					return DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ABGR_I16 ;
				}
				else
				if( Format->ColorBitDepth == 4 )
				{
					     if( Format->AlphaChFlag   ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_PAL4 ;
					else if( Format->AlphaTestFlag ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_PAL4 ;
					else                             Result = DX_GRAPHICSIMAGE_FORMAT_3D_PAL4 ;
				}
				else
				if( Format->ColorBitDepth == 8 )
				{
					     if( Format->AlphaChFlag   ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_PAL8 ;
					else if( Format->AlphaTestFlag ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_PAL8 ;
					else                             Result = DX_GRAPHICSIMAGE_FORMAT_3D_PAL8 ;
				}
				else
				{
					int ColorBitDepthIndex = Format->ColorBitDepth == 16 ? 0 : 1 ;

					if( Format->DrawValidFlag )
					{
						if( Format->AlphaChFlag == TRUE || Format->AlphaTestFlag == TRUE )
						{
							Result = DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_ALPHA_RGB32 ;
						}
						else
						{
							Result = DX_GRAPHICSIMAGE_FORMAT_3D_DRAWVALID_RGB16 + ColorBitDepthIndex ;
						}
					}
					else if( Format->AlphaChFlag   ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHA_RGB16     + ColorBitDepthIndex ;
					else if( Format->AlphaTestFlag ) Result = DX_GRAPHICSIMAGE_FORMAT_3D_ALPHATEST_RGB16 + ColorBitDepthIndex ;
					else                             Result = DX_GRAPHICSIMAGE_FORMAT_3D_RGB16           + ColorBitDepthIndex ;
				}
			}
			break ;
		}
		break ;
	}

	return Result ;
}

#ifdef __WINDOWS__

// 色情報解析
extern int NS_ColorKaiseki( const void *PixelData, LPCOLORDATA ColorData )
{
	D_DDPIXELFORMAT *PData = ( D_DDPIXELFORMAT * )PixelData ;
	DWORD ColorMask ;
	int ColorWidth, ColorLoc ;
	int i, j ;

	unsigned int  ColorMaskBuf[ 5 ] ;
	unsigned char *ColorWidthBuf[ 5 ] ;
	unsigned char *ColorLocBuf[ 5 ] ;

	// ビットデプスを保存
	ColorData->ColorBitDepth = ( unsigned char )PData->dwRGBBitCount ;

	// ピクセルあたりのバイト数を保存
	ColorData->PixelByte = ( unsigned char )( ColorData->ColorBitDepth / 8 ) ;

	// 色情報を解析する
	{
		// カラーマスク情報を得る準備をする
		ColorData->RedMask		= PData->dwRBitMask;
		ColorMaskBuf[ 0 ]		= ColorData->RedMask ;
		ColorWidthBuf[ 0 ]		= &ColorData->RedWidth ;
		ColorLocBuf[ 0 ]		= &ColorData->RedLoc ;

		ColorData->GreenMask	= PData->dwGBitMask;
		ColorMaskBuf[ 1 ]		= ColorData->GreenMask ;
		ColorWidthBuf[ 1 ]		= &ColorData->GreenWidth ;
		ColorLocBuf[ 1 ]		= &ColorData->GreenLoc ;

		ColorData->BlueMask		= PData->dwBBitMask;
		ColorMaskBuf[ 2 ]		= ColorData->BlueMask ;
		ColorWidthBuf[ 2 ]		= &ColorData->BlueWidth ;
		ColorLocBuf[ 2 ]		= &ColorData->BlueLoc ;

		ColorData->AlphaMask	= PData->dwRGBAlphaBitMask ;
		ColorMaskBuf[ 3 ]		= ColorData->AlphaMask ;
		ColorWidthBuf[ 3 ]		= &ColorData->AlphaWidth ;
		ColorLocBuf[ 3 ]		= &ColorData->AlphaLoc ;

		if( ColorData->ColorBitDepth == 32 ) ColorData->NoneMask = 0xffffffff;
		else                                 ColorData->NoneMask = ( unsigned int )( ( 1 << ColorData->ColorBitDepth ) - 1 ) ;
		ColorData->NoneMask		&= ~( ColorData->RedMask | ColorData->GreenMask | ColorData->BlueMask | ColorData->AlphaMask ) ;
		ColorMaskBuf[ 4 ]		= ColorData->NoneMask ;
		ColorWidthBuf[ 4 ]		= &ColorData->NoneWidth ;
		ColorLocBuf[ 4 ]		= &ColorData->NoneLoc ;
	}

	// 色の解析開始
 	for( j = 0 ; j < 5 ; j ++ )
	{
		// カラーマスクを取得
		ColorMask = ColorMaskBuf[ j ] ;

		if( ColorMask == 0 )
		{
			*ColorLocBuf[ j ] = 0 ;
			*ColorWidthBuf[ j ] = 0 ;
			continue ;
		}

		// カラー情報の先頭ビットまでのビット数とビット幅の初期化
		ColorLoc = 0 ;
		ColorWidth = 0 ;

		// カラー情報のある先頭ビットまでのビット数を解析
		for( i = 0 ; i < 32 ; i ++ )
		{
			// 先頭ビットに到達しない間は繰り返す
			if( ColorMask & 1 )
			{	
				ColorLoc = i ;
				break ;
			}
			ColorMask >>= 1 ;
		}
		// もしカラー情報がなければ異常終了
		if( i == 32 ) return -1 ;

		// カラー情報のビット幅を調べる
		for( i = 0 ; i < 32 ; i ++ )
		{
			// ビットが途切れない間繰り返す
			if( !( ColorMask & 1 ) ) 
			{
				ColorWidth = i ;
				break ;
			}
			ColorMask >>= 1 ;
		}

		// 解析結果を保存する
		*ColorLocBuf[ j ] = ( unsigned char )ColorLoc ;
		*ColorWidthBuf[ j ] = ( unsigned char )ColorWidth ;
	}

	// なんの機能も割り付けられていないビットのマスクをセットする
//	NS_SetColorDataNoneMask( ColorData ) ;

	// 終了
	return 0 ;
}

#endif // __WINDOWS__





















// 文字列描画関数

#ifndef DX_NON_FONT

// Ｚバッファに対して文字列の描画
extern int NS_DrawStringToZBuffer( int x, int y, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawString(  x,  y, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して文字列の描画
extern int NS_DrawVStringToZBuffer( int x, int y, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVString(  x,  y, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して文字列を描画する
extern int NS_DrawStringToHandleToZBuffer( int x, int y, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawStringToHandle(  x,  y, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) ,  VerticalFlag );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して文字列を描画する
extern int NS_DrawVStringToHandleToZBuffer( int x, int y, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVStringToHandle(  x,  y, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を描画する
extern int NS_DrawFormatStringToZBuffer( int x, int y, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawString(  x,  y, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を描画する
extern int NS_DrawFormatVStringToZBuffer( int x, int y, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVString(  x,  y, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を描画する
extern int NS_DrawFormatStringToHandleToZBuffer( int x, int y, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawStringToHandle(  x,  y, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を描画する
extern int NS_DrawFormatVStringToHandleToZBuffer( int x, int y, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawVStringToHandle(  x,  y, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して文字列の拡大描画
extern int NS_DrawExtendStringToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendString(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して文字列の拡大描画
extern int NS_DrawExtendVStringToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVString(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して文字列を拡大描画する
extern int NS_DrawExtendStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendStringToHandle(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) ,  VerticalFlag );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して文字列を拡大描画する
extern int NS_DrawExtendVStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVStringToHandle(  x,  y,  ExRateX,  ExRateY, String,  GetColor( 255,255,255 ),  FontHandle,  GetColor( 0,0,0 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を拡大描画する
extern int NS_DrawExtendFormatStringToZBuffer( int x, int y, double ExRateX, double ExRateY, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendString(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を拡大描画する
extern int NS_DrawExtendFormatVStringToZBuffer( int x, int y, double ExRateX, double ExRateY, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVString(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ) );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を拡大描画する
extern int NS_DrawExtendFormatStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendStringToHandle(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// Ｚバッファに対して書式指定文字列を拡大描画する
extern int NS_DrawExtendFormatVStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode );
	NS_DrawExtendVStringToHandle(  x,  y, ExRateX, ExRateY, String,  GetColor( 255,255,255 ), FontHandle );
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// デフォルトフォントハンドルを使用してＺバッファに対して文字列を回転描画する
extern int NS_DrawRotaStringToZBuffer( int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *String )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaString(  x,  y,  ExRateX,  ExRateY, RotCenterX, RotCenterY, RotAngle,  GetColor( 255,255,255 ), GetColor( 0,0,0 ), VerticalFlag, String ) ;
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// フォントハンドルを使用してＺバッファに対して文字列を回転描画する
extern int NS_DrawRotaStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag , const TCHAR *String )
{
	DrawZBuffer_Pre( WriteZMode );
	NS_DrawRotaStringToHandle(  x,  y,  ExRateX,  ExRateY, RotCenterX, RotCenterY, RotAngle,  GetColor( 255,255,255 ), FontHandle, GetColor( 0,0,0 ), VerticalFlag, String ) ;
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// デフォルトフォントハンドルを使用してＺバッファに対して書式指定文字列を回転描画する
extern int NS_DrawRotaFormatStringToZBuffer( int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *FormatString , ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode ) ;
	NS_DrawRotaString(  x,  y,  ExRateX,  ExRateY, RotCenterX, RotCenterY, RotAngle,  GetColor( 255,255,255 ), GetColor( 0,0,0 ), VerticalFlag, String ) ;
	DrawZBuffer_Post();

	// 終了
	return 0;
}

// フォントハンドルを使用してＺバッファに対して書式指定文字列を回転描画する
extern int NS_DrawRotaFormatStringToHandleToZBuffer( int x, int y, double ExRateX, double ExRateY, double RotCenterX, double RotCenterY, double RotAngle, int FontHandle, int WriteZMode /* DX_ZWRITE_MASK 等 */ , int VerticalFlag, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	va_start( VaList, FormatString ) ;
	_TVSPRINTF( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	DrawZBuffer_Pre( WriteZMode ) ;
	NS_DrawRotaStringToHandle(  x,  y,  ExRateX,  ExRateY, RotCenterX, RotCenterY, RotAngle,  GetColor( 255,255,255 ), FontHandle, GetColor( 0,0,0 ), VerticalFlag, String ) ;
	DrawZBuffer_Post();

	// 終了
	return 0;
}

#endif // DX_NON_FONT
























// 基本イメージ描画

// 基本イメージデータを描画する
extern	int		NS_DrawBaseImage( int x, int y, BASEIMAGE *BaseImage )
{
	int TempHandle ;

/*
	if( NS_GetActiveGraph() == DX_SCREEN_BACK )
	{
		DWORD Pitch;
		int Width, Height;
		void *Image;
		COLORDATA *ColorData;
		BASEIMAGE DestImage;

		NS_GetDrawScreenSize( &Width, &Height ) ;

		NS_GraphLock( DX_SCREEN_BACK, (int *)&Pitch, &Image, &ColorData ) ;
		DestImage.ColorData = *ColorData;
		DestImage.Width     = Width;
		DestImage.Height    = Height;
		DestImage.Pitch     = Pitch;
		DestImage.GraphData = Image;

		NS_BltBaseImage( 0, 0, BaseImage->Width, BaseImage->Height, x, y, BaseImage, &DestImage ) ;

		NS_GraphUnLock( DX_SCREEN_BACK ) ;
	}
	else
*/
	{
		TempHandle = Graphics_Image_CreateGraphFromGraphImageBase( BaseImage, NULL, TRUE, FALSE ) ;
		NS_DrawGraph( x, y, TempHandle, BaseImage->ColorData.AlphaWidth ? TRUE : FALSE ) ;
		NS_DeleteGraph( TempHandle ) ;
	}

	return 0 ;
}




















// 基本イメージデータ構造体関係

// 描画対象の画面から指定領域のグラフィックを基本イメージデータに転送する
extern int NS_GetDrawScreenBaseImage( int x1, int y1, int x2, int y2, BASEIMAGE *BaseImage )
{
	return NS_GetDrawScreenBaseImageDestPos( x1, y1, x2, y2, BaseImage, 0, 0 ) ;
}

// 描画対象の画面から指定領域を基本イメージデータに転送する( 転送先座標指定版 )
extern	int NS_GetDrawScreenBaseImageDestPos( int x1, int y1, int x2, int y2, BASEIMAGE *BaseImage, int DestX, int DestY )
{
	RECT SrcRect ;
	BASEIMAGE BufferImage ;

	// 座標チェック
	if( x1 < 0 || y1 < 0 || x1 >= x2 || y1 >= y2 )
	{
		return -1 ;
	}

	// 取り込みサイズと取り込み先が画像サイズより大きい場合はエラー
	if( BaseImage->Width < DestX + x2 - x1 || BaseImage->Height < DestY + y2 - y1 )
	{
		return -1 ;
	}

	// 描画先バッファをロック
	SrcRect.left   = x1 ;
	SrcRect.right  = x2 ;
	SrcRect.top    = y1 ;
	SrcRect.bottom = y2 ;
	if( Graphics_Screen_LockDrawScreen( &SrcRect, &BufferImage, -1, -1, TRUE, 0 ) < 0 )
	{
		return -1 ;
	}

//	// 取り込みサイズと画像サイズが違う場合はエラー
//	if( BaseImage->Width != BufferImage.Width || BaseImage->Height != BufferImage.Height ) return -1 ;

	// データを転送
	NS_BltBaseImage( DestX, DestY, &BufferImage, BaseImage ) ;

	// 描画先バッファをアンロック
	Graphics_Screen_UnlockDrawScreen() ;

	// 終了
	return 0 ;
}























// グラフィック関連の初期化と後始末

// グラフィックスシステムの初期化
extern int Graphics_Initialize( void )
{
	// 既に初期化済みの場合は何もしない
	if( GSYS.InitializeFlag == TRUE ) return 0 ;

	// 画面の設定を初期化
	if( GSYS.Screen.MainScreenSizeX == 0 || GSYS.Screen.MainScreenSizeY == 0 )
	{
		Graphics_Screen_SetMainScreenSize( DEFAULT_SCREEN_SIZE_X, DEFAULT_SCREEN_SIZE_Y ) ;
	}

	if( GSYS.Screen.MainScreenColorBitDepth == 0 )
	{
		GSYS.Screen.MainScreenColorBitDepth = DEFAULT_COLOR_BITDEPTH ;
		SetMemImgDefaultColorType( 0 ) ;
	}

	// グラフィックハンドル管理情報の初期化
	InitializeHandleManage( DX_HANDLETYPE_GRAPH, sizeof( IMAGEDATA ), MAX_IMAGE_NUM, Graphics_Image_InitializeHandle, Graphics_Image_TerminateHandle, L"Graph" ) ;

	// シェーダーハンドル管理情報の初期化
	InitializeHandleManage( DX_HANDLETYPE_SHADER, sizeof( SHADERHANDLEDATA ) + sizeof( SHADERHANDLEDATA_PF ) , MAX_SHADER_NUM, Graphics_Shader_InitializeHandle, Graphics_Shader_TerminateHandle, L"Shader" ) ;

	// シェーダー用定数バッファハンドル管理情報の初期化
	InitializeHandleManage( DX_HANDLETYPE_SHADER_CONSTANT_BUFFER, sizeof( SHADERCONSTANTBUFFERHANDLEDATA ) + sizeof( SHADERCONSTANTBUFFERHANDLEDATA_PF ) , MAX_CONSTANT_BUFFER_NUM, Graphics_ShaderConstantBuffer_InitializeHandle, Graphics_ShaderConstantBuffer_TerminateHandle, L"ShaderConstantBuffer" ) ;

	// 頂点バッファハンドル管理情報の初期化
	InitializeHandleManage( DX_HANDLETYPE_VERTEX_BUFFER, sizeof( VERTEXBUFFERHANDLEDATA ) + sizeof( VERTEXBUFFERHANDLEDATA_PF ), MAX_VERTEX_BUFFER_NUM, Graphics_VertexBuffer_InitializeHandle, Graphics_VertexBuffer_TerminateHandle, L"VertexBuffer" ) ;

	// インデックスバッファハンドル管理情報の初期化
	InitializeHandleManage( DX_HANDLETYPE_INDEX_BUFFER, sizeof( INDEXBUFFERHANDLEDATA ) + sizeof( INDEXBUFFERHANDLEDATA_PF ), MAX_INDEX_BUFFER_NUM, Graphics_IndexBuffer_InitializeHandle, Graphics_IndexBuffer_TerminateHandle, L"IndexBuffer" ) ;

	// シャドウマップハンドル管理情報の初期化
	InitializeHandleManage( DX_HANDLETYPE_SHADOWMAP, sizeof( SHADOWMAPDATA ) + sizeof( SHADOWMAPDATA_PF ), MAX_SHADOWMAP_NUM, Graphics_ShadowMap_InitializeHandle, Graphics_ShadowMap_TerminateHandle, L"ShadowMap" ) ;

	// 描画処理の環境依存部分の初期化その１
	if( Graphics_Initialize_Timing0_PF() < 0 )
	{
		goto ERR ;
	}

	// 初期化済みフラグを立てる
	GSYS.InitializeFlag = TRUE ;

	// ハードウエアアクセラレータが使用できる場合
	// ハードウエアアクセラレータ周りの初期化を行う
	if( GSYS.Setting.ValidHardware == TRUE )
	{
		Graphics_Hardware_Initialize_PF() ;
	}
	else
	// それ以外の場合はソフトウエアレンダリング周りの初期化を行う
	{
		Graphics_Software_Initialize() ;
	}

	// グラフィック描画設定関係の情報を初期化
	Graphics_DrawSetting_Initialize() ;

	// MEMIMG の初期化
	InitializeMemImgManage() ;

#ifndef DX_NON_FONT
	// フォントの初期化を行う
	InitFontManage() ;
#endif

#ifndef DX_NON_MASK
	// マスク処理の初期化
	Mask_Initialize() ;
#endif

	// 描画処理の環境依存部分の初期化その２
	if( Graphics_Initialize_Timing1_PF() < 0 )
	{
		goto ERR ;
	}

	// 終了
	return 0 ;

	// エラー処理
ERR:
	Graphics_Terminate() ;

	return -1;
}

// グラフィックシステムの後始末
extern int Graphics_Terminate( void )
{
#ifndef DX_NON_FONT
	// フォントキャッシュの管理系を終了
	TermFontManage() ;
#endif

	// 既に後始末済みの場合は何もしない
	if( GSYS.InitializeFlag == FALSE ) return 0 ;

#ifndef DX_NON_MASK
	// マスク処理の後始末
	Mask_Terminate() ;
#endif

#ifndef DX_NON_MODEL
	// すべてのモデルを削除
	NS_MV1InitModel() ;
	MV1InitModelBase() ;
#endif

	// 全ての画像を削除
	NS_InitGraph() ;

	// すべての頂点データとインデックスデータを削除
	NS_InitVertexBuffer() ;
	NS_InitIndexBuffer() ;

	// 全てのシェーダー用定数バッファを削除
	NS_InitShaderConstantBuffer() ;

	// 全てのシェーダーを削除
	NS_InitShader() ;

	// ライトを全て削除
	NS_DeleteLightHandleAll() ;
	NS_DeleteLightHandle( GSYS.Light.DefaultHandle ) ;
	GSYS.Light.DefaultHandle = -1 ;

	// ソフトウエアレンダリング機能の後始末
	Graphics_Software_Terminate() ;

	// 描画処理の環境依存部分の後始末
	Graphics_Terminate_PF() ;

	// 頂点を一時的に保存するメモリ領域の解放
	if( GSYS.Resource.TempVertexBuffer )
	{
		DXFREE( GSYS.Resource.TempVertexBuffer ) ;
		GSYS.Resource.TempVertexBuffer = NULL ;
		GSYS.Resource.TempVertexBufferSize = 0 ;
	}

	// シャドウマップハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_SHADOWMAP ) ;

	// 頂点バッファハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_VERTEX_BUFFER ) ;

	// インデックスバッファハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_INDEX_BUFFER ) ;

	// シェーダーハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_SHADER ) ;

	// シェーダー用定数バッファハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_SHADER_CONSTANT_BUFFER ) ;

	// グラフィックハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_GRAPH ) ;

	// 汎用バッファの解放
	Graphics_Other_TerminateCommonBuffer() ;

	// 初期化済みフラグを倒す
	GSYS.InitializeFlag = FALSE ;

	// 終了
	return 0 ;
}

// グラフィックスシステムの復帰、又は変更付きの再セットアップを行う
extern int Graphics_RestoreOrChangeSetupGraphSystem( int Change, int ScreenSizeX, int ScreenSizeY, int ColorBitDepth, int RefreshRate )
{
	int Result ;

	// 環境依存の処理を行う
	Result = Graphics_RestoreOrChangeSetupGraphSystem_PF( Change, ScreenSizeX, ScreenSizeY, ColorBitDepth, RefreshRate ) ;

	// ムービーグラフィックの再生
	if( DxSysData.NotDrawFlag == FALSE )
	{
#ifndef DX_NON_MOVIE
		RestoreMovieAll() ;
#endif
	}


	// 終了
	return Result ;
}























// 画面関係関数

// フルスクリーンモードのモードのチェックや使用する解像度をの決定を行う
extern	int		Graphics_Screen_SetupFullScreenModeInfo( void )
{
	int             DispModeNum ;
	int             i ;
	int				IntMulNum ;
	int             ColorBitDepthResolutionMatchIndex ;
	int             ColorBitDepthResolutionMatchIntMulNum ;
	int             ResolutionMatchIndex ;
	int             ResolutionMatchIntMulNum ;
	int             RefreshRateColorBitDepthResolutionMatchIndex ;
	int             RefreshRateColorBitDepthResolutionMatchIntMulNum ;
	DISPLAYMODEDATA DispModeData = { 0 } ;
	int				UseDisplayIndex ;

	// 画面解像度が設定されていなかったらデフォルト値をセット
	if( GSYS.Screen.MainScreenSizeX == 0 || GSYS.Screen.MainScreenSizeY == 0 )
	{
		Graphics_Screen_SetMainScreenSize( DEFAULT_SCREEN_SIZE_X, DEFAULT_SCREEN_SIZE_Y ) ;
	}

	// 使用するディスプレイ番号を準備
	UseDisplayIndex = GSYS.Screen.ValidUseDisplayIndex ? GSYS.Screen.UseDisplayIndex : 0 ;

	// 画面のカラービット深度がが設定されていなかったらデフォルト値をセット
	if( GSYS.Screen.MainScreenColorBitDepth == 0 )
	{
		GSYS.Screen.MainScreenColorBitDepth = DEFAULT_COLOR_BITDEPTH ;
		SetMemImgDefaultColorType( 0 ) ;
	}

	GSYS.Screen.FullScreenResolutionModeAct = GSYS.Screen.FullScreenResolutionMode ;

LABEL_FULLSCREENMODE_SWITCH :
	switch( GSYS.Screen.FullScreenResolutionModeAct )
	{
	case DX_FSRESOLUTIONMODE_NATIVE :
		// 指定の画面モードに対応しているか調べる
		DispModeNum = NS_GetDisplayModeNum( UseDisplayIndex ) ;
		ResolutionMatchIndex                               = -1 ;
		ResolutionMatchIntMulNum                           = -1 ;
		ColorBitDepthResolutionMatchIndex                  = -1 ;
		ColorBitDepthResolutionMatchIntMulNum              = -1 ;
		RefreshRateColorBitDepthResolutionMatchIndex       = -1 ;
		RefreshRateColorBitDepthResolutionMatchIntMulNum   = -1 ;
		for( i = 0 ; i < DispModeNum ; i ++ )
		{
			DispModeData = NS_GetDisplayMode( i, UseDisplayIndex ) ;

			// 指定の解像度と一致しているか、若しくは指定の解像度の整数倍の解像度かどうかで分岐
			if( DispModeData.Width  % GSYS.Screen.MainScreenSizeX == 0 &&
				DispModeData.Height % GSYS.Screen.MainScreenSizeY == 0 &&
				( DispModeData.Width  / GSYS.Screen.MainScreenSizeX == DispModeData.Height / GSYS.Screen.MainScreenSizeY ) )
			{
				// 指定の解像度の何倍かを算出
				IntMulNum = DispModeData.Width  / GSYS.Screen.MainScreenSizeX ;

				// 最初に見つけた画面モードか、若しくは今まで見つけた画面モードよりも解像度倍数が小さい場合は記憶
				if( ResolutionMatchIntMulNum < 0 || IntMulNum < ResolutionMatchIntMulNum )
				{
					ResolutionMatchIndex     = i ;
					ResolutionMatchIntMulNum = IntMulNum ;
				}

				// 指定のカラービット深度と一致しているかどうかで分岐
				if( GSYS.Screen.MainScreenColorBitDepth == DispModeData.ColorBitDepth )
				{
					// 最初に見つけた画面モードか、若しくは今まで見つけた画面モードよりも解像度倍数が小さい場合は記憶
					if( ColorBitDepthResolutionMatchIntMulNum < 0 || IntMulNum < ColorBitDepthResolutionMatchIntMulNum )
					{
						ColorBitDepthResolutionMatchIndex     = i ;
						ColorBitDepthResolutionMatchIntMulNum = IntMulNum ;
					}

					// リフレッシュレートの指定が無いか、指定のリフレッシュレートと一致しているかどうかで分岐
					if( GSYS.Screen.MainScreenRefreshRate == 0 ||
						GSYS.Screen.MainScreenRefreshRate == DispModeData.RefreshRate )
					{
						// 最初に見つけた画面モードか、若しくは今まで見つけた画面モードよりも解像度倍数が小さい場合は記憶
						if( RefreshRateColorBitDepthResolutionMatchIntMulNum < 0 || IntMulNum < RefreshRateColorBitDepthResolutionMatchIntMulNum )
						{
							RefreshRateColorBitDepthResolutionMatchIndex     = i ;
							RefreshRateColorBitDepthResolutionMatchIntMulNum = IntMulNum ;

							// もし解像度倍数が１のときは、指定の画面モードに完全に一致しているということなので、ループから抜ける
							if( IntMulNum == 1 )
							{
								break ;
							}
						}
					}
				}
			}
		}

		// 解像度が指定通り、若しくは整数倍で、リフレッシュレート・カラービット深度が指定の通りの画面モードがあった場合は、それにする
		if( RefreshRateColorBitDepthResolutionMatchIndex != -1 )
		{
			GSYS.Screen.FullScreenUseDispModeData = NS_GetDisplayMode( RefreshRateColorBitDepthResolutionMatchIndex, UseDisplayIndex ) ;
		}
		else
		// 解像度が指定通り、若しくは整数倍で、カラービット深度も指定通りの画面モードがあった場合は、それにする
		if( ColorBitDepthResolutionMatchIndex != -1 )
		{
			GSYS.Screen.FullScreenUseDispModeData = NS_GetDisplayMode( ColorBitDepthResolutionMatchIndex, UseDisplayIndex ) ;
		}
		else
		// 解像度が指定通り、若しくは整数倍の画面モードがあった場合は、それにする
		if( ResolutionMatchIndex != -1 )
		{
			GSYS.Screen.FullScreenUseDispModeData = NS_GetDisplayMode( ResolutionMatchIndex, UseDisplayIndex ) ;
		}
		else
		// 解像度が指定通り、若しくは整数倍の画面モードが無かった場合はフルスクリーン解像度モードを DX_FSRESOLUTIONMODE_DESKTOP に変更する
		{
			GSYS.Screen.FullScreenResolutionModeAct = DX_FSRESOLUTIONMODE_DESKTOP ;
			goto LABEL_FULLSCREENMODE_SWITCH ;
		}

		// 画面モードが指定に完全一致していない場合は、フルスクリーン解像度モードを DX_FSRESOLUTIONMODE_MAXIMUM にする
		if( RefreshRateColorBitDepthResolutionMatchIndex == -1 || 
			RefreshRateColorBitDepthResolutionMatchIntMulNum != 1 )
		{
			GSYS.Screen.FullScreenResolutionModeAct = DX_FSRESOLUTIONMODE_MAXIMUM ;
		}
		break ;

	case DX_FSRESOLUTIONMODE_DESKTOP :
		if( GSYS.Screen.DisplayInfo == NULL )
		{
			// ディスプレイ情報の取得に失敗したら DX_FSRESOLUTIONMODE_MAXIMUM を使用する
			if( Graphics_SetupDisplayInfo_PF() < 0 )
			{
				GSYS.Screen.FullScreenResolutionModeAct = DX_FSRESOLUTIONMODE_MAXIMUM ;
				goto LABEL_FULLSCREENMODE_SWITCH ;
			}
		}

		// 指定の解像度がデスクトップ画面より高い場合は DX_FSRESOLUTIONMODE_MAXIMUM に変更
		if( GSYS.Screen.DisplayInfo[ UseDisplayIndex ].DesktopSizeX < GSYS.Screen.MainScreenSizeX ||
			GSYS.Screen.DisplayInfo[ UseDisplayIndex ].DesktopSizeY < GSYS.Screen.MainScreenSizeY )
		{
			GSYS.Screen.FullScreenResolutionModeAct = DX_FSRESOLUTIONMODE_MAXIMUM ;
			goto LABEL_FULLSCREENMODE_SWITCH ;
		}

		// デスクトップのサイズをセット
		GSYS.Screen.FullScreenUseDispModeData.Width         = GSYS.Screen.DisplayInfo[ UseDisplayIndex ].DesktopSizeX ;
		GSYS.Screen.FullScreenUseDispModeData.Height        = GSYS.Screen.DisplayInfo[ UseDisplayIndex ].DesktopSizeY ;
		GSYS.Screen.FullScreenUseDispModeData.ColorBitDepth = GSYS.Screen.DisplayInfo[ UseDisplayIndex ].DesktopColorBitDepth ;
		GSYS.Screen.FullScreenUseDispModeData.RefreshRate   = GSYS.Screen.DisplayInfo[ UseDisplayIndex ].DesktopRefreshRate ;
		break ;

	case DX_FSRESOLUTIONMODE_MAXIMUM :
		// 最大解像度を取得する
		{
			int MatchDispModeIndex ;
			int MatchDispModeWidth ;
			int MatchDispModeHeight ;
			int MaxResDispModeIndex ;
			int MaxResDispModeWidth ;
			int MaxResDispModeHeight ;
			int MaxResFullColorDispModeIndex ;
			int MaxResFullColorDispModeWidth ;
			int MaxResFullColorDispModeHeight ;

			MatchDispModeIndex            = -1 ;
			MaxResDispModeIndex           = -1 ;
			MaxResFullColorDispModeIndex  = -1 ;
			MatchDispModeWidth            = 0 ;
			MatchDispModeHeight           = 0 ;
			MaxResDispModeWidth           = 0 ;
			MaxResDispModeHeight          = 0 ;
			MaxResFullColorDispModeWidth  = 0 ;
			MaxResFullColorDispModeHeight = 0 ;

			DispModeNum = NS_GetDisplayModeNum() ;
			for( i = 0 ; i < DispModeNum ; i ++ )
			{
				DispModeData = NS_GetDisplayMode( i, UseDisplayIndex ) ;

				if(   GSYS.Screen.MainScreenColorBitDepth == DispModeData.ColorBitDepth &&
					( GSYS.Screen.MainScreenRefreshRate == 0 ||
					  GSYS.Screen.MainScreenRefreshRate == DispModeData.RefreshRate ) && 
					( MatchDispModeIndex == -1 ||
					DispModeData.Width * DispModeData.Height > MatchDispModeWidth * MatchDispModeHeight ) )
				{
					MatchDispModeIndex = i ;
					MatchDispModeWidth  = DispModeData.Width ;
					MatchDispModeHeight = DispModeData.Height ;
				}

				if( MaxResDispModeIndex == -1 ||
					DispModeData.Width * DispModeData.Height > MaxResDispModeWidth * MaxResDispModeHeight )
				{
					MaxResDispModeIndex = i ;
					MaxResDispModeWidth  = DispModeData.Width ;
					MaxResDispModeHeight = DispModeData.Height ;
				}

				if( DispModeData.ColorBitDepth == GSYS.Screen.MainScreenColorBitDepth &&
					( MaxResFullColorDispModeIndex == -1 ||
					DispModeData.Width * DispModeData.Height > MaxResFullColorDispModeWidth * MaxResFullColorDispModeHeight ) )
				{
					MaxResFullColorDispModeIndex = i ;
					MaxResFullColorDispModeWidth  = DispModeData.Width ;
					MaxResFullColorDispModeHeight = DispModeData.Height ;
				}
			}

			if( MaxResDispModeWidth * MaxResDispModeHeight <= MaxResFullColorDispModeWidth * MaxResFullColorDispModeHeight )
			{
				GSYS.Screen.FullScreenUseDispModeData = NS_GetDisplayMode( MaxResFullColorDispModeIndex, UseDisplayIndex ) ;
			}
			else
			if( MaxResDispModeWidth * MaxResDispModeHeight <= MatchDispModeWidth * MatchDispModeHeight )
			{
				GSYS.Screen.FullScreenUseDispModeData = NS_GetDisplayMode( MatchDispModeIndex, UseDisplayIndex ) ;
			}
			else
			if( MaxResDispModeIndex > 0 )
			{
				GSYS.Screen.FullScreenUseDispModeData = NS_GetDisplayMode( MaxResDispModeIndex, UseDisplayIndex ) ;
			}
			else
			{
				GSYS.Screen.FullScreenUseDispModeData = NS_GetDisplayMode( 0, UseDisplayIndex ) ;
			}
		}
		break ;
	}

	// 正常終了
	return 0 ;
}

// GSYS.Screen.FullScreenScalingDestRect の値をセットアップする
extern	int		Graphics_Screen_SetupFullScreenScalingDestRect( void )
{
	// DX_FSRESOLUTIONMODE_NATIVE 以外の場合はここで転送先矩形の算出
	switch( GSYS.Screen.FullScreenResolutionModeAct )
	{
	case DX_FSRESOLUTIONMODE_NATIVE :
		break ;

	case DX_FSRESOLUTIONMODE_MAXIMUM :
	case DX_FSRESOLUTIONMODE_DESKTOP :
		// サブバックバッファの内容をピクセルの縦横比を１：１に保ちつつバックバッファに最大まで拡大して転送する矩形の算出
		{
			int DestSizeX ;
			int DestSizeY ;
			int ScalingSizeX ;
			int ScalingSizeY ;
			static int DestSizeXBackup = -1000 ;
			static int DestSizeYBackup = -1000 ;
			static int ScalingSizeXBackup = -1000 ;
			static int ScalingSizeYBackup = -1000 ;
			static int MainScreenSizeXBackup = -1000 ;
			static int MainScreenSizeYBackup = -1000 ;

			// 転送先のサイズをセット
			if( GSYS.Screen.FullScreenResolutionModeAct == DX_FSRESOLUTIONMODE_DESKTOP )
			{
				GRAPHICSSYS_DISPLAYINFO *DisplayInfo ;

				if( GSYS.Screen.DisplayInfo == NULL )
				{
					return -1 ;
				}

				DisplayInfo = &GSYS.Screen.DisplayInfo[ GSYS.Screen.ValidUseDisplayIndex ? GSYS.Screen.UseDisplayIndex : 0 ] ;
				DestSizeX = DisplayInfo->DesktopSizeX ;
				DestSizeY = DisplayInfo->DesktopSizeY ;

				if( DestSizeXBackup != DestSizeX || DestSizeYBackup != DestSizeY )
				{
					DestSizeXBackup = DestSizeX ;
					DestSizeYBackup = DestSizeY ;
//					DXST_ERRORLOGFMT_ADDUTF16LE(( "\x11\xff\x20\x00\x20\x00\x44\x00\x65\x00\x73\x00\x74\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x58\x00\x3a\x00\x25\x00\x64\x00\x20\x00\x20\x00\x44\x00\x65\x00\x73\x00\x74\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x59\x00\x3a\x00\x25\x00\x64\x00\x00"/*@ L"１  DestSizeX:%d  DestSizeY:%d" @*/, DestSizeX, DestSizeY )) ;
				}
			}
			else
			{
				DestSizeX = GSYS.Screen.FullScreenUseDispModeData.Width ;
				DestSizeY = GSYS.Screen.FullScreenUseDispModeData.Height ;

				if( DestSizeXBackup != DestSizeX || DestSizeYBackup != DestSizeY )
				{
					DestSizeXBackup = DestSizeX ;
					DestSizeYBackup = DestSizeY ;
//					DXST_ERRORLOGFMT_ADDUTF16LE(( "\x12\xff\x20\x00\x20\x00\x44\x00\x65\x00\x73\x00\x74\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x58\x00\x3a\x00\x25\x00\x64\x00\x20\x00\x20\x00\x44\x00\x65\x00\x73\x00\x74\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x59\x00\x3a\x00\x25\x00\x64\x00\x00"/*@ L"２  DestSizeX:%d  DestSizeY:%d" @*/, DestSizeX, DestSizeY )) ;
				}
			}

			// 転送後のサイズを決定
			ScalingSizeX = DestSizeY * GSYS.Screen.MainScreenSizeX / GSYS.Screen.MainScreenSizeY ;
			if( ScalingSizeX < DestSizeX )
			{
				ScalingSizeY = DestSizeY ;
			}
			else
			{
				ScalingSizeX = DestSizeX ;
				ScalingSizeY = DestSizeX * GSYS.Screen.MainScreenSizeY / GSYS.Screen.MainScreenSizeX ;
			}

			if( ScalingSizeXBackup != ScalingSizeX ||
				ScalingSizeYBackup != ScalingSizeY ||
				MainScreenSizeXBackup != GSYS.Screen.MainScreenSizeX ||
				MainScreenSizeYBackup != GSYS.Screen.MainScreenSizeY )
			{
				ScalingSizeXBackup = ScalingSizeX ;
				ScalingSizeYBackup = ScalingSizeY ;
				MainScreenSizeXBackup = GSYS.Screen.MainScreenSizeX ;
				MainScreenSizeYBackup = GSYS.Screen.MainScreenSizeY ;
//				DXST_ERRORLOGFMT_ADDUTF16LE(( "\x53\x00\x63\x00\x61\x00\x6c\x00\x69\x00\x6e\x00\x67\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x58\x00\x3a\x00\x25\x00\x64\x00\x00\x30\x53\x00\x63\x00\x61\x00\x6c\x00\x69\x00\x6e\x00\x67\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x59\x00\x3a\x00\x25\x00\x64\x00\x20\x00\x20\x00\x4d\x00\x61\x00\x69\x00\x6e\x00\x53\x00\x63\x00\x72\x00\x65\x00\x65\x00\x6e\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x58\x00\x3a\x00\x25\x00\x64\x00\x20\x00\x20\x00\x4d\x00\x61\x00\x69\x00\x6e\x00\x53\x00\x63\x00\x72\x00\x65\x00\x65\x00\x6e\x00\x53\x00\x69\x00\x7a\x00\x65\x00\x59\x00\x3a\x00\x25\x00\x64\x00\x00"/*@ L"ScalingSizeX:%d　ScalingSizeY:%d  MainScreenSizeX:%d  MainScreenSizeY:%d" @*/,
//					ScalingSizeX, ScalingSizeY,
//					GSYS.Screen.MainScreenSizeX, GSYS.Screen.MainScreenSizeY )) ;
			}

			// 転送先の矩形を決定
			GSYS.Screen.FullScreenScalingDestRect.left   = ( DestSizeX - ScalingSizeX ) / 2 ;
			GSYS.Screen.FullScreenScalingDestRect.top    = ( DestSizeY - ScalingSizeY ) / 2 ;
			GSYS.Screen.FullScreenScalingDestRect.right  = GSYS.Screen.FullScreenScalingDestRect.left + ScalingSizeX ;
			GSYS.Screen.FullScreenScalingDestRect.bottom = GSYS.Screen.FullScreenScalingDestRect.top  + ScalingSizeY ;
		}
		break ;
	}

	// 正常終了
	return 0 ;
}

// スクリーン座標をサブバックバッファー座標に変換する
extern int Graphics_Screen_ScreenPosConvSubBackbufferPos( int ScreenPosX, int ScreenPosY, int *BackBufferPosX, int *BackBufferPosY )
{
	ScreenPosX -= GSYS.Screen.FullScreenDesktopRect.left ;
	ScreenPosY -= GSYS.Screen.FullScreenDesktopRect.top ;

	switch( GSYS.Screen.FullScreenResolutionModeAct )
	{
	case DX_FSRESOLUTIONMODE_NATIVE :
		if( BackBufferPosX != NULL )
		{
			*BackBufferPosX = ScreenPosX ;
		}

		if( BackBufferPosY != NULL )
		{
			*BackBufferPosY = ScreenPosY ;
		}
		break ;

	case DX_FSRESOLUTIONMODE_MAXIMUM :
	case DX_FSRESOLUTIONMODE_DESKTOP :
		if( Graphics_Screen_SetupFullScreenScalingDestRect() < 0 )
		{
			return -1 ;
		}

		if( BackBufferPosX != NULL )
		{
			*BackBufferPosX = ( ScreenPosX - GSYS.Screen.FullScreenScalingDestRect.left ) * GSYS.Screen.MainScreenSizeX / ( GSYS.Screen.FullScreenScalingDestRect.right  - GSYS.Screen.FullScreenScalingDestRect.left ) ;
		}

		if( BackBufferPosY != NULL )
		{
			*BackBufferPosY = ( ScreenPosY - GSYS.Screen.FullScreenScalingDestRect.top  ) * GSYS.Screen.MainScreenSizeY / ( GSYS.Screen.FullScreenScalingDestRect.bottom - GSYS.Screen.FullScreenScalingDestRect.top ) ;
		}
		break ;
	}

	return 0 ;
}

// サブバックバッファー座標をスクリーン座標に変換する
extern int Graphics_Screen_SubBackbufferPosConvScreenPos( int BackBufferPosX, int BackBufferPosY, int *ScreenPosX, int *ScreenPosY )
{
	switch( GSYS.Screen.FullScreenResolutionModeAct )
	{
	case DX_FSRESOLUTIONMODE_NATIVE :
		if( ScreenPosX != NULL )
		{
			*ScreenPosX = BackBufferPosX ;
		}

		if( ScreenPosY != NULL )
		{
			*ScreenPosY = BackBufferPosY ;
		}
		break ;

	case DX_FSRESOLUTIONMODE_MAXIMUM :
	case DX_FSRESOLUTIONMODE_DESKTOP :
		if( Graphics_Screen_SetupFullScreenScalingDestRect() < 0 )
		{
			return -1 ;
		}

		if( ScreenPosX != NULL )
		{
			*ScreenPosX = BackBufferPosX * ( GSYS.Screen.FullScreenScalingDestRect.right  - GSYS.Screen.FullScreenScalingDestRect.left ) / GSYS.Screen.MainScreenSizeX + GSYS.Screen.FullScreenScalingDestRect.left ;
		}

		if( ScreenPosY != NULL )
		{
			*ScreenPosY = BackBufferPosY * ( GSYS.Screen.FullScreenScalingDestRect.bottom - GSYS.Screen.FullScreenScalingDestRect.top  ) / GSYS.Screen.MainScreenSizeY + GSYS.Screen.FullScreenScalingDestRect.top ;
		}
		break ;
	}

	return 0 ;
}

// メイン画面のＺバッファの設定を変更する
extern int Graphics_Screen_SetZBufferMode( int ZBufferSizeX, int ZBufferSizeY, int ZBufferBitDepth )
{
	return Graphics_Hardware_SetZBufferMode_PF( ZBufferSizeX, ZBufferSizeY, ZBufferBitDepth ) ;
}

// 設定に基づいて使用するＺバッファをセットする
extern int Graphics_Screen_SetupUseZBuffer( void )
{
	return Graphics_Hardware_SetupUseZBuffer_PF() ;
}

// メイン画面のサイズ値を変更する
extern void Graphics_Screen_SetMainScreenSize( int SizeX, int SizeY )
{
	GSYS.Screen.MainScreenSizeX = SizeX ;
	GSYS.Screen.MainScreenSizeY = SizeY ;

	GSYS.Screen.MainScreenSizeX_Result = GSYS.Screen.MainScreenSizeX == 0 ? DEFAULT_SCREEN_SIZE_X : GSYS.Screen.MainScreenSizeX ;
	GSYS.Screen.MainScreenSizeY_Result = GSYS.Screen.MainScreenSizeY == 0 ? DEFAULT_SCREEN_SIZE_Y : GSYS.Screen.MainScreenSizeY ;

	SetMathScreenSize( GSYS.Screen.MainScreenSizeX_Result, GSYS.Screen.MainScreenSizeY_Result ) ;
}

// 画面モードの変更２
extern int Graphics_Screen_ChangeMode( int ScreenSizeX, int ScreenSizeY, int ColorBitDepth, int ChangeWindowFlag, int RefreshRate )
{
	int BackScSizeX, BackScSizeY, BackScColorBitDepth, BackRefreshRate ;
	int Ret = DX_CHANGESCREEN_OK ;
	int InitFlag ;
#ifndef DX_NON_FONT
	wchar_t FontName[ 128 ] ;
	int FontSize     = 0 ;
	int FontThick    = 0 ;
	int FontType     = 0 ;
	int FontCharSet  = 0 ;
	int FontEdgeSize = 0 ;
	int FontItalic   = 0 ;
	int FontSpace    = 0 ;
	int FontUse      = 0 ;
#endif // DX_NON_FONT
	int Result ;
	int i ;

	// Graphics_Screen_ChangeMode実行中フラグを立てる
	GSYS.Screen.Graphics_Screen_ChangeModeFlag = TRUE ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 既に初期化済みかどうかを取得しておく
	InitFlag = GSYS.InitializeFlag ;

	// -1 だった部分は既存の値で代用
	if( ScreenSizeX		== -1 ) ScreenSizeX   = GSYS.Screen.MainScreenSizeX ;
	if( ScreenSizeY		== -1 ) ScreenSizeY   = GSYS.Screen.MainScreenSizeY ;
	if( ColorBitDepth	== -1 ) ColorBitDepth = GSYS.Screen.MainScreenColorBitDepth ;
	if( RefreshRate		== -1 ) RefreshRate   = GSYS.Screen.MainScreenRefreshRate ;

	// もし以前と画面モードが全く同じ場合は何もせず終了
	if( ScreenSizeX      == GSYS.Screen.MainScreenSizeX &&
		ScreenSizeY      == GSYS.Screen.MainScreenSizeY &&
		ColorBitDepth    == GSYS.Screen.MainScreenColorBitDepth &&
		ChangeWindowFlag == FALSE &&
		RefreshRate      == GSYS.Screen.MainScreenRefreshRate )
	{
		goto END ;
	}

	DXST_ERRORLOG_ADDUTF16LE( "\x3b\x75\x62\x97\xe2\x30\xfc\x30\xc9\x30\x09\x59\xf4\x66\xe6\x51\x06\x74\x92\x30\x8b\x95\xcb\x59\x57\x30\x7e\x30\x59\x30\x0a\x00\x00"/*@ L"画面モード変更処理を開始します\n" @*/ ) ;
	DXST_ERRORLOG_TABADD ;

#ifndef DX_NON_FONT

	// 既に初期化されていたらフォントの情報をとっておく
	if( InitFlag && NS_CheckFontHandleValid( NS_GetDefaultFontHandle() ) )
	{
		GetFontStateToHandle_WCHAR_T( FontName, &FontSize, &FontThick, DX_DEFAULT_FONT_HANDLE, &FontType, &FontCharSet, &FontEdgeSize, &FontItalic ) ;
		FontSpace = NS_GetFontSpaceToHandle( DX_DEFAULT_FONT_HANDLE ) ;
		FontUse = TRUE ;
	}

#endif // DX_NON_FONT

	// 既存の画面モードのパラメータをとっておく
	BackScSizeX			= GSYS.Screen.MainScreenSizeX ; 
	BackScSizeY			= GSYS.Screen.MainScreenSizeY ;
	BackScColorBitDepth	= GSYS.Screen.MainScreenColorBitDepth ;
	BackRefreshRate		= GSYS.Screen.MainScreenRefreshRate ;

	// ＤＸライブラリの初期化前は実行できない
	if( DxSysData.DxLib_RunInitializeFlag == FALSE && DxSysData.DxLib_InitializeFlag == FALSE )
	{
		goto END ;
	}

	// 画面モード変更時にグラフィックスシステムをリセットするかどうかで処理を分岐
	if( GSYS.Setting.ChangeScreenModeNotGraphicsSystemFlag )
	{
		// リセットしない場合

#ifdef __WINDOWS__
		// ウインドウモードの場合は指定の解像度がデスクトップのサイズを超えていないかどうかを調べる
		if( NS_GetWindowModeFlag() == TRUE )
		{
			GRAPHICSSYS_DISPLAYINFO *DisplayInfo ;

			DisplayInfo = &GSYS.Screen.DisplayInfo[ GSYS.Screen.ValidUseDisplayIndex ? GSYS.Screen.UseDisplayIndex : 0 ] ;

			if( DisplayInfo->DesktopSizeX < ScreenSizeX || DisplayInfo->DesktopSizeY < ScreenSizeY )
			{
				goto END ;
			}
		}
#endif // __WINDOWS__

		// チェンジ
		Graphics_RestoreOrChangeSetupGraphSystem( TRUE, ScreenSizeX, ScreenSizeY, ColorBitDepth, RefreshRate ) ;

		// ソフトウエアレンダリングの場合ここで後始末と初期化を行う
		if( GSYS.Setting.ValidHardware == FALSE )
		{
			Graphics_Software_Terminate() ;
			Graphics_Software_Initialize() ;
		}

#ifdef __WINDOWS__
		// ウインドウスタイルセット
		if( NS_GetWindowModeFlag() == TRUE )
		{
			SetWindowStyle() ;
		}
#endif // __WINDOWS__

		DXST_ERRORLOG_TABSUB ;
		DXST_ERRORLOG_ADDUTF16LE( "\x3b\x75\x62\x97\xe2\x30\xfc\x30\xc9\x30\x09\x59\xf4\x66\xe6\x51\x06\x74\x6f\x30\x63\x6b\x38\x5e\x6b\x30\x42\x7d\x86\x4e\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"画面モード変更処理は正常に終了しました\n" @*/ ) ;
	}
	else
	{
		// リセットする場合

		// グラフィクスシステムの終了
		if( GSYS.InitializeFlag )
		{
			Graphics_Terminate() ;
		}

		// 画面モードのセット
		Graphics_Screen_SetMainScreenSize( ScreenSizeX, ScreenSizeY ) ;
		GSYS.Screen.FullScreenResolutionModeAct = GSYS.Screen.FullScreenResolutionMode ;
		GSYS.Screen.MainScreenColorBitDepth     = ColorBitDepth ;
		GSYS.Screen.MainScreenRefreshRate       = RefreshRate ;
		SetMemImgDefaultColorType( ColorBitDepth == 32 ? 1 : 0 ) ;

#ifndef DX_NON_FONT
		// デフォルトフォントの設定をセット
		if( FontUse == TRUE )
		{
			SetDefaultFontState_WCHAR_T( FontName, FontSize, FontThick, FontType, FontCharSet, FontEdgeSize, FontItalic ) ;
			NS_SetFontSpace( FontSpace ) ;
		}
#endif // DX_NON_FONT

		// グラフィックシステムの再初期化
		Result = Graphics_Initialize() ;

		// 失敗した場合の処理
		if( Result == -1 )
		{
			// エラーが発生した場合で画面モードが 320x240 だった場合は
			// 擬似 320x240 モードで試す
			if( GSYS.Screen.MainScreenSizeX == 320 && GSYS.Screen.MainScreenSizeY == 240 )
			{
				Graphics_Screen_SetMainScreenSize( 640, 480 ) ;
				NS_SetEmulation320x240( TRUE ) ;
				Result = Graphics_Initialize() ;
			}

			// エラーが発生した場合は元の画面モードに戻す
			if( Result == -1 )
			{
				Graphics_Screen_SetMainScreenSize( BackScSizeX, BackScSizeY ) ;
				GSYS.Screen.MainScreenColorBitDepth = BackScColorBitDepth ;
				GSYS.Screen.MainScreenRefreshRate   = BackRefreshRate ;
				SetMemImgDefaultColorType( BackScColorBitDepth == 32 ? 1 : 0 ) ;
				if( Graphics_Initialize() == -1 )
				{
					// それでも駄目だった場合はＤＸライブラリのデフォルトの画面モードに変更
					Graphics_Screen_SetMainScreenSize( DEFAULT_SCREEN_SIZE_X, DEFAULT_SCREEN_SIZE_Y ) ;
					GSYS.Screen.MainScreenColorBitDepth = DEFAULT_COLOR_BITDEPTH ;
					GSYS.Screen.MainScreenRefreshRate   = 0 ;
					SetMemImgDefaultColorType( 0 ) ;
					if( Graphics_Initialize() == -1 )
					{
						// それでも駄目だった場合は640x480 32bit を試す
						Graphics_Screen_SetMainScreenSize( DEFAULT_SCREEN_SIZE_X, DEFAULT_SCREEN_SIZE_Y ) ;
						GSYS.Screen.MainScreenColorBitDepth = 32 ;
						GSYS.Screen.MainScreenRefreshRate   = 0 ;
						SetMemImgDefaultColorType( 0 ) ;
						if( Graphics_Initialize() == -1 )
						{
							Ret = -1 ;
							DxLib_ErrorUTF16LE( "\x88\x4e\x1f\x67\x5b\x30\x6c\x30\x3b\x75\x62\x97\xe2\x30\xfc\x30\xc9\x30\x09\x59\xf4\x66\x0d\x4e\xfd\x80\xfe\x73\x61\x8c\x4c\x30\x77\x8d\x4d\x30\x7e\x30\x57\x30\x5f\x30\x32\x00\x0a\x00\x00"/*@ L"予期せぬ画面モード変更不能現象が起きました2\n" @*/ ) ;
							goto END ;
						}
					}
				}
			}
		}

#ifdef __WINDOWS__
		// ウインドウスタイルセット
		if( NS_GetWindowModeFlag() == TRUE /*|| GSYS.Setting.ValidHardware == FALSE*/ )
		{
			SetWindowStyle() ;
		}
#endif // __WINDOWS__

		DXST_ERRORLOG_TABSUB ;
		DXST_ERRORLOG_ADDUTF16LE( "\x3b\x75\x62\x97\xe2\x30\xfc\x30\xc9\x30\x09\x59\xf4\x66\xe6\x51\x06\x74\x6f\x30\x63\x6b\x38\x5e\x6b\x30\x42\x7d\x86\x4e\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"画面モード変更処理は正常に終了しました\n" @*/ ) ;

		// メッセージ処理
		NS_ProcessMessage() ;

		// 描画先の設定しなおし
		NS_SetRenderTargetToShader( 0, GSYS.DrawSetting.TargetScreen[ 0 ], GSYS.DrawSetting.TargetScreenSurface[ 0 ] ) ;
		for( i = 1 ; i < GSYS.HardInfo.RenderTargetNum ; i ++ )
		{
			NS_SetRenderTargetToShader( i, -1 ) ;
		}
	}

#ifdef __WINDOWS__
	// ドラッグ＆ドロップの設定をし直す
	RefreshDragFileValidFlag() ;
#endif // __WINDOWS__

END:
	// Graphics_Screen_ChangeMode 実行中フラグを倒す
	GSYS.Screen.Graphics_Screen_ChangeModeFlag = FALSE ;

	// 終了
	return Ret ;
}

// 描画先バッファをロックする
extern int Graphics_Screen_LockDrawScreen( RECT *LockRect, BASEIMAGE *BaseImage, int TargetScreen, int TargetScreenSurface, int ReadOnly, int TargetScreenTextureNo )
{
	IMAGEDATA *TargImage ;
	int TargetScreenWidth ;
	int TargetScreenHeight ;

	// 描画先の画像データアドレスを取得する
	if( TargetScreen == -1 )
	{
		TargetScreen        = GSYS.DrawSetting.TargetScreen[ 0 ] ;
		TargetScreenSurface = GSYS.DrawSetting.TargetScreenSurface[ 0 ] ;
	}
	else
	{
		if( TargetScreenSurface == -1 )
		{
			TargetScreenSurface = 0 ;
		}
	}
	if( GRAPHCHKFULL( TargetScreen, TargImage ) )
	{
		TargImage          = NULL ;
		TargetScreenWidth  = GSYS.DrawSetting.DrawSizeX ;
		TargetScreenHeight = GSYS.DrawSetting.DrawSizeY ;
	}
	else
	{
		TargetScreenWidth  = TargImage->Width ;
		TargetScreenHeight = TargImage->Height ;
	}

	// 矩形のデータが不正だった場合エラー
	if( LockRect->left < 0 || LockRect->left >= LockRect->right  ||
		LockRect->top  < 0 || LockRect->top  >= LockRect->bottom ||
		LockRect->right  > TargetScreenWidth ||
		LockRect->bottom > TargetScreenHeight ) return -1 ;

	// ハードウエアを使用しているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		// ハードウェアアクセラレータを使用する場合
		Graphics_Hardware_LockDrawScreenBuffer_PF( LockRect, BaseImage, TargetScreen, TargImage, TargetScreenSurface, ReadOnly, TargetScreenTextureNo ) ;
	}
	else
	{
		int Width, Height ;

		// ソフトウエアレンダリングの場合

		Width  = LockRect->right  - LockRect->left ;
		Height = LockRect->bottom - LockRect->top ;

		// データをセット
		if( TargImage )
		{
			BaseImage->ColorData = *TargImage->Soft.MemImg.Base->ColorDataP ;
			BaseImage->Pitch     = ( int )TargImage->Soft.MemImg.Base->Pitch ;
			BaseImage->GraphData = ( BYTE * )TargImage->Soft.MemImg.UseImage + BaseImage->ColorData.PixelByte * LockRect->left + BaseImage->Pitch * LockRect->top ;
		}
		else
		{
			BaseImage->ColorData = *GSYS.SoftRender.MainBufferMemImg.Base->ColorDataP ;
			BaseImage->Pitch     = ( int )GSYS.SoftRender.MainBufferMemImg.Base->Pitch ;
			BaseImage->GraphData = ( BYTE * )GSYS.SoftRender.MainBufferMemImg.UseImage + BaseImage->ColorData.PixelByte * LockRect->left + BaseImage->Pitch * LockRect->top ;
		}
		BaseImage->Width          = Width ;
		BaseImage->Height         = Height ;
		BaseImage->MipMapCount    = 0 ;
		BaseImage->GraphDataCount = 0 ;
	}

	// ロック中フラグを立てる
	GSYS.Screen.DrawScreenBufferLockFlag = TRUE ;

	// 終了
	return 0 ;
}

// 描画先バッファをアンロックする
extern int Graphics_Screen_UnlockDrawScreen( void )
{
	// ロックしていなかったら何もせずに終了
	if( GSYS.Screen.DrawScreenBufferLockFlag == FALSE ) return 0 ;

	// ハードウエアを使用しているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_UnlockDrawScreenBuffer_PF() ;
	}
	else
	{
	}

	// ロック中フラグを倒す
	GSYS.Screen.DrawScreenBufferLockFlag = FALSE ;

	return 0 ;
}

// 裏画面と表画面を交換する
// 表画面が裏画面からのスケーリングの場合は、CopyRect はコピー先矩形 )
extern int Graphics_Screen_FlipBase( void )
{
	static int EndFlag = FALSE ;
	int Result = 0 ;

	GSYS.Screen.ScreenFlipFlag = TRUE ;

	// もしフラグがたっていたらなにもせず終了
	if( EndFlag )
	{
		goto END ;
	}

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE || DxLib_GetEndRequest() )
	{
		goto ERR ;
	}

	// フルスクリーンマスク処理を行っている場合はマスク描画の結果を反映させる
	MASK_FULLSCREEN_MASKUPDATE

#if !defined( DX_NON_LOG ) && !defined( DX_NON_PRINTF_DX )
	// ログ出力を行う設定の場合のみログ出力を行う
	if( NS_GetLogDrawFlag() )
	{
		int BlendMode, BlendParam, RedBright, GreenBright, BlueBright ;
		RECT DrawRect ;

		BlendMode   = GSYS.DrawSetting.BlendMode ;
		BlendParam  = GSYS.DrawSetting.BlendParam ;
		DrawRect    = GSYS.DrawSetting.DrawArea ;
		RedBright   = GSYS.DrawSetting.DrawBright.Red ;
		GreenBright = GSYS.DrawSetting.DrawBright.Green ;
		BlueBright  = GSYS.DrawSetting.DrawBright.Blue ;

		NS_SetDrawArea( 0, 0, GSYS.Screen.MainScreenSizeX, GSYS.Screen.MainScreenSizeY ) ;

		// 描画モードをノーブレンドにセット
		NS_SetDrawBlendMode( DX_BLENDMODE_NOBLEND, 0 ) ;

		// 輝度を１００％にセット
		NS_SetDrawBright( 255,255,255 ) ;

		// フラグをたてる
		EndFlag = TRUE ;

		// ログ描画
		DrawLog() ;

		// フラグを倒す
		EndFlag = FALSE ;

		// 設定を元に戻す
		NS_SetDrawArea( DrawRect.left, DrawRect.top, DrawRect.right, DrawRect.bottom ) ; 
		NS_SetDrawBlendMode( BlendMode, BlendParam ) ;
		NS_SetDrawBright( RedBright, GreenBright, BlueBright ) ;
	}
#endif

	// 環境依存処理
	Result = Graphics_ScreenFlipBase_PF() ;


END:
	GSYS.Screen.ScreenFlipFlag = FALSE ;

	// 終了
	return Result ;


ERR:
	GSYS.Screen.ScreenFlipFlag = FALSE ;

	// エラー終了
	return -1 ;
}




















// 画像関係関数

// フォーマット情報をセットアップする
extern int Graphics_Image_SetupFormatDesc( 
	IMAGEFORMATDESC *Format,
	SETUP_GRAPHHANDLE_GPARAM *GParam,
	int Width,
	int Height,
	int AlphaValidFlag,
	int UsePaletteFlag,
	int PaletteBitDepth,
	int BaseFormat,
	int MipMapCount
)
{
	// フォーマット情報を初期化
	_MEMSET( Format, 0, sizeof( *Format ) ) ;

	// テクスチャフォーマットかどうかをセット
	Format->TextureFlag = ( unsigned char )GSYS.Setting.ValidHardware ;

	// DirectX が管理するテクスチャにしないかどうかをセット
	Format->UseManagedTextureFlag = ( unsigned char )GParam->UseManagedTextureFlag ;

	// キューブマップかどうかをセット
	Format->CubeMapTextureFlag = ( unsigned char )GParam->CubeMapTextureCreateFlag ;

	// 基本フォーマットのセット
	Format->BaseFormat = ( unsigned char )BaseFormat ;

	// ミップマップの数を保存
	if( MipMapCount != -1 )
	{
		Format->MipMapCount = ( unsigned char )MipMapCount ;
	}
	else
	{
		if( GParam->MipMapCount == -1 )
		{
			int tx, ty ;

			tx = Width ;
			ty = Height ;
			Format->MipMapCount = 1 ;
			for(;;)
			{
				if( tx == 1 && ty == 1 ) break ;
				Format->MipMapCount ++ ;
				tx /= 2 ;
				ty /= 2 ;
				if( tx == 0 ) tx = 1 ;
				if( ty == 0 ) ty = 1 ;
			}
		}
		else
		{
			Format->MipMapCount = ( unsigned char )( GParam->MipMapCount == 0 ? 1 : GParam->MipMapCount ) ;
		}
	}

	// 描画可能画像かどうかで処理を分岐
	if( GParam->DrawValidImageCreateFlag == TRUE )
	{
		// 描画可能な場合はアルファテスト、ブレンドグラフィックは無しで、
		// アルファチャンネルが着くかどうかは GSYS.CreateImage.DrawValidAlphaFlag の値に従う
		// 基本フォーマットは強制的に DX_BASEIMAGE_FORMAT_NORMAL
		Format->DrawValidFlag         = TRUE ;
		Format->AlphaChFlag           = ( unsigned char )GParam->DrawValidAlphaImageCreateFlag ;
		Format->AlphaTestFlag         = FALSE ;		// 描画可能な場合はアルファテストは無し
		Format->BlendGraphFlag        = FALSE ;		// 描画可能な場合はブレンド画像も無し
		Format->UseManagedTextureFlag = FALSE ;
		Format->MipMapCount           = 1 ;
		Format->MSSamples             = ( unsigned char )GParam->DrawValidMSSamples ;
		Format->MSQuality             = ( unsigned char )GParam->DrawValidMSQuality ;
	}
	else
	{
		// 描画可能指定が無い場合はアルファチャンネルを付けるかどうかは引数に
		// アルファテストを付けるかどうかは GSYS.CreateImage.AlphaTestFlag に
		// ブレンド画像にするかどうかは GSYS.CreateImage.BlendImageFlag に従う
		Format->DrawValidFlag  = FALSE ;
		Format->AlphaChFlag    = ( unsigned char )AlphaValidFlag ;
		Format->AlphaTestFlag  = ( unsigned char )GParam->AlphaTestImageCreateFlag ;
		Format->BlendGraphFlag = ( unsigned char )GParam->BlendImageCreateFlag ;
	}

	// テクスチャかどうかで処理を分岐
	if( Format->TextureFlag == TRUE )
	{
		// テクスチャの場合

		// カラービット深度の決定
		Format->ColorBitDepth   = ( unsigned char )( GParam->CreateImageColorBitDepth == 0 ? ( AlphaValidFlag == TRUE ? 32 : GSYS.Screen.MainScreenColorBitDepth ) : GParam->CreateImageColorBitDepth ) ;
		Format->ChannelNum      = 0 ;
		Format->ChannelBitDepth = 0 ;
		Format->FloatTypeFlag   = FALSE ;
		Format->SystemMemFlag   = FALSE ;

		// アルファチャンネルありの場合はアルファテストは無し
		if( Format->AlphaChFlag )
		{
			Format->AlphaTestFlag = FALSE ;
		}

		// ブレンド画像の場合はフォーマットは固定される
		if( Format->BlendGraphFlag == TRUE )
		{
			Format->ColorBitDepth = 32 ;
			Format->AlphaTestFlag = FALSE ;
			Format->AlphaChFlag   = TRUE ;
			Format->DrawValidFlag = FALSE ;
		}
		else
		// 描画可能画像の場合はチャンネル数とチャンネル辺りのビット数、浮動小数点型かどうかを設定する
		if( GParam->DrawValidImageCreateFlag == TRUE )
		{
			Format->ChannelNum      = ( unsigned char )GParam->CreateDrawValidGraphChannelNum ;
			Format->ChannelBitDepth = ( unsigned char )GParam->CreateImageChannelBitDepth ;
			Format->FloatTypeFlag   = ( unsigned char )GParam->DrawValidFloatTypeGraphCreateFlag ;
		}
		else
		// ブレンド画像でも描画可能画像でもない場合のみパレット画像を使用できる
		if( UsePaletteFlag )
		{
			Format->ColorBitDepth = ( unsigned char )( PaletteBitDepth == 4 ? 4 : 8 ) ;
		}
	}
	else
	{
		// テクスチャではない場合
		Format->BaseFormat            = DX_BASEIMAGE_FORMAT_NORMAL ;
		Format->ColorBitDepth         = ( unsigned char )( GParam->CreateImageColorBitDepth == 0 ? GSYS.Screen.MainScreenColorBitDepth : GParam->CreateImageColorBitDepth ) ;
		Format->ChannelNum            = 0 ;
		Format->ChannelBitDepth       = 0 ;
		Format->FloatTypeFlag         = FALSE ;
		Format->SystemMemFlag         = TRUE ;
		Format->CubeMapTextureFlag    = FALSE ;
		Format->UsePaletteFlag        = ( unsigned char )UsePaletteFlag ;
		Format->UseManagedTextureFlag = FALSE ;		// DirectX の管理も無し
		Format->AlphaTestFlag         = FALSE ;		// アルファテストは無し
		Format->DrawValidFlag         = ( unsigned char )( Format->BlendGraphFlag ? FALSE : TRUE ) ;	// 普通の画像の場合はブレンド画像ではなければ描画可能　 
		Format->MipMapCount           = 0 ;
	}

	// 正常終了
	return 0 ;
}

// デバイスロスト発生時に削除するフラグが立っているグラフィックを削除する
extern int Graphics_Image_DeleteDeviceLostDelete( void )
{
	int i ;
	IMAGEDATA *Image ;

	if( HandleManageArray[ DX_HANDLETYPE_GRAPH ].InitializeFlag )
	{
		for( i = HandleManageArray[ DX_HANDLETYPE_GRAPH ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_GRAPH ].AreaMax ; i ++ )
		{
			Image = ( IMAGEDATA * )HandleManageArray[ DX_HANDLETYPE_GRAPH ].Handle[ i ] ;

			if( Image == NULL ) continue ;
			if( Image->Orig == NULL ) continue ;
			if( Image->DeviceLostDeleteFlag == FALSE ) continue ;

			SubHandle( Image->HandleInfo.Handle ) ;
		}
	}

	// 終了
	return 0 ;
}

// ＭＳＡＡを使用する描画可能画像かどうかを調べる( TRUE:MSAA画像  FALSE:MSAA画像ではない )
extern int Graphics_Image_CheckMultiSampleDrawValid( int GrHandle )
{
	IMAGEDATA *Image ;

	if( GRAPHCHKFULL( GrHandle, Image ) )
		return FALSE ;

	if( Image->Orig->FormatDesc.TextureFlag == FALSE )
		return FALSE ;

	return Image->Orig->FormatDesc.MSSamples != 0 ? TRUE : FALSE ;
}

// 新しいグラフィックハンドルを確保する
extern int Graphics_Image_AddHandle( int ASyncThread )
{
	return AddHandle( DX_HANDLETYPE_GRAPH, ASyncThread, -1 );
}

// グラフィックハンドルを初期化する
extern int Graphics_Image_SetupHandle_UseGParam(
	SETUP_GRAPHHANDLE_GPARAM *GParam,
	int GrHandle,
	int Width,
	int Height,
	int /*TextureFlag*/,
	int AlphaValidFlag,
	int UsePaletteFlag,
	int PaletteBitDepth,
	int BaseFormat,
	int MipMapCount,
	int ASyncThread
) 
{
	IMAGEDATA					*Image ;
	IMAGEDATA_ORIG				*Orig ;
	int							IsTex ;
	int							DivXNum ;
	int							DivYNum ;
	int							DivNum ;
	short						TexSizeX[ 256 ] ;
	short						TexSizeY[ 256 ] ;
	short						TexPosX[ 256 ] ;
	short						TexPosY[ 256 ] ;
	short						XList[ 32 ] ;
	short						YList[ 32 ] ;
	int							XPos ;
	int							YPos ;
	int							BaseAllocSize ;
	unsigned int				AllocSize ;
	int							i ;
	int							j ;
	IMAGEDATA_ORIG_HARD_TEX	*OrigTex ;
	IMAGEFORMATDESC				Format ;
	IMAGEDATA_ORIG_HARD_TEX_PF	*OrigTexPF ;

	// アドレスの取得
	if( GRAPHCHK_ASYNC( GrHandle, Image ) )
	{
		return -1 ;
	}

	// テクスチャかどうかを決定する
	IsTex = GSYS.Setting.ValidHardware ;

	// フォーマット情報をセットする
	Graphics_Image_SetupFormatDesc( &Format, GParam, Width, Height, AlphaValidFlag, UsePaletteFlag, PaletteBitDepth, BaseFormat, MipMapCount ) ;

	// テクスチャの場合は何分割になるか調べる
	if( IsTex )
	{
		// 作成するテクスチャのサイズをリストアップ
//		DivXNum = Graphics_Image_ListUpTexSize( Width,  XList, Format.DrawValidFlag | Format.BlendGraphFlag | GParam->NotUseDivFlag, Format.DrawValidFlag == FALSE, GParam->UserMaxTextureSize ) ;
//		DivYNum = Graphics_Image_ListUpTexSize( Height, YList, Format.DrawValidFlag | Format.BlendGraphFlag | GParam->NotUseDivFlag, Format.DrawValidFlag == FALSE, GParam->UserMaxTextureSize ) ;
		// 分割する方がデメリットが大きくなってきたので、基本的に最大テクスチャーサイズに収まる限りは１枚で済ますように処理を変更
		DivXNum = Graphics_Image_ListUpTexSize( Width,  XList, TRUE, Format.DrawValidFlag == FALSE, GParam->UserMaxTextureSize ) ;
		DivYNum = Graphics_Image_ListUpTexSize( Height, YList, TRUE, Format.DrawValidFlag == FALSE, GParam->UserMaxTextureSize ) ;

		// 分割数をセット
		DivNum = DivXNum * DivYNum ;

		// 位置とサイズをセットする
		YPos = 0 ;
		for( i = 0 ; i < DivYNum ; i ++ )
		{
			XPos = 0 ;
			for( j = 0 ; j < DivXNum ; j ++ )
			{
				TexPosX[ i * DivXNum + j ] = ( short )XPos ;
				TexPosY[ i * DivXNum + j ] = ( short )YPos ;

				TexSizeX[ i * DivXNum + j ] = XList[ j ] ;
				TexSizeY[ i * DivXNum + j ] = YList[ i ] ;

				XPos += XList[ j ] ;
			}
			YPos += YList[ i ] ;
		}

		// ハードウェア制限に応じてテクスチャを正方形にする
		if( GSYS.HardInfo.TextureSquareOnly )
		{
			int Size ;
			int DNum ;
			int MotoDivNum ;
			int PosX ;
			int PosY ;

			MotoDivNum = DivXNum * DivYNum ;
			for( i = 0 ; i < MotoDivNum ; i ++ )
			{
				if( TexSizeX[ i ] != TexSizeY[ i ] )
				{
					if( TexSizeX[ i ] < TexSizeY[ i ] )
					{
						Size = TexSizeX[ i ] ;
						DNum = TexSizeY[ i ] / TexSizeX[ i ] ;
						TexSizeY[ i ] = ( short )Size ;
						PosX = TexPosX[ i ] ;
						PosY = TexPosY[ i ] ;
						for( j = 0 ; j < DNum - 1 ; j ++ )
						{
							TexSizeX[ DivNum + j ] = ( short )Size ;
							TexSizeY[ DivNum + j ] = ( short )Size ;
							TexPosX[ DivNum + j ] = ( short )PosX ;
							TexPosY[ DivNum + j ] = ( short )( PosY + Size * ( j + 1 ) ) ;
						}
						DivNum += DNum - 1 ;
					}
					else
					{
						Size = TexSizeY[ i ] ;
						DNum = TexSizeX[ i ] / TexSizeY[ i ] ;
						TexSizeX[ i ] = ( short )Size ;
						PosX = TexPosX[ i ] ;
						PosY = TexPosY[ i ] ;
						for( j = 0 ; j < DNum - 1 ; j ++ )
						{
							TexSizeY[ DivNum + j ] = ( short )Size ;
							TexSizeX[ DivNum + j ] = ( short )Size ;
							TexPosY[ DivNum + j ] = ( short )PosY ;
							TexPosX[ DivNum + j ] = ( short )( PosX + Size * ( j + 1 ) ) ;
						}
						DivNum += DNum - 1 ;
					}
				}
			}
		}
	}

	// 確保メモリの確定
	BaseAllocSize = sizeof( IMAGEDATA_ORIG ) - ( ( sizeof( IMAGEDATA_ORIG_SOFT ) > sizeof( IMAGEDATA_ORIG_HARD ) ? sizeof( IMAGEDATA_ORIG_SOFT ) : sizeof( IMAGEDATA_ORIG_HARD ) ) ) ;
	if( IsTex )
	{
		AllocSize = sizeof( IMAGEDATA_ORIG_HARD ) + ( DivNum - 4 ) * sizeof( IMAGEDATA_ORIG_HARD_TEX ) + DivNum * sizeof( IMAGEDATA_ORIG_HARD_TEX_PF ) ;
	}
	else
	{
		AllocSize = sizeof( IMAGEDATA_ORIG_SOFT ) ;
	}

	// オリジナル画像情報構造体用のメモリの確保
	Image->Orig = ( IMAGEDATA_ORIG * )DXCALLOC( AllocSize + BaseAllocSize ) ;
	if( Image->Orig == NULL )
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xb0\x30\xe9\x30\xd5\x30\xa3\x30\xc3\x30\xaf\x30\x6e\x30\xaa\x30\xea\x30\xb8\x30\xca\x30\xeb\x30\x3b\x75\xcf\x50\xc7\x30\xfc\x30\xbf\x30\xdd\x4f\x58\x5b\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"グラフィックのオリジナル画像データ保存用メモリの確保に失敗しました\n" @*/ ) ;
		goto ERR ;
	}
	Orig = Image->Orig ;

	// テクスチャの場合は環境依存データのアドレスをセット
	if( IsTex )
	{
		OrigTexPF = ( IMAGEDATA_ORIG_HARD_TEX_PF * )( ( BYTE * )Orig + BaseAllocSize + sizeof( IMAGEDATA_ORIG_HARD ) + ( DivNum - 4 ) * sizeof( IMAGEDATA_ORIG_HARD_TEX ) ) ;
		OrigTex   = Orig->Hard.Tex ;
		for( i = 0; i < DivNum; i++, OrigTex++, OrigTexPF++ )
		{
			OrigTex->PF = OrigTexPF ;
		}
	}

	// フォーマットをセット
	Orig->FormatDesc = Format ;

	// 透過色をセット
	Orig->TransCode = GParam->TransColor ;

	// InitGraph で削除するかどうかをセット
	Image->NotInitGraphDelete = GParam->NotInitGraphDelete ;

	// 幅と高さを保存
	Image->UseOrigX     = 0 ;
	Image->UseOrigY     = 0 ;
	Image->Width        = Width ;
	Image->Height       = Height ;
	Image->Orig->Width  = Width ;
	Image->Orig->Height = Height ;

	// 参照数を初期化
	Image->Orig->RefCount = 1 ;

	// オリジナル画像情報の初期化
	if( Orig->FormatDesc.TextureFlag )
	{
		// テクスチャの場合

		// フォーマットの決定
		Orig->ColorFormat = NS_GetTexFormatIndex( &Orig->FormatDesc ) ;

		// テクスチャ分割数に合わせてテクスチャ情報リスト用のメモリを確保する
		Orig->Hard.TexNum = DivNum ;

		// ミップマップの数をセット
//		Orig->Hard.MipMap = Orig->FormatDesc.DrawValidFlag ? FALSE : GRH.MipMapFlag ;
/*		if( Orig->ColorFormat == DX_GRAPHICSIMAGE_FORMAT_3D_DXT1 ||
			Orig->ColorFormat == DX_GRAPHICSIMAGE_FORMAT_3D_DXT2 ||
			Orig->ColorFormat == DX_GRAPHICSIMAGE_FORMAT_3D_DXT3 ||
			Orig->ColorFormat == DX_GRAPHICSIMAGE_FORMAT_3D_DXT4 ||
			Orig->ColorFormat == DX_GRAPHICSIMAGE_FORMAT_3D_DXT5 )
		{
			Orig->Hard.MipMapCount = 1 ;
		}
		else*/
		{
			Orig->Hard.MipMapCount = Format.MipMapCount ;
		}

		// 決定したテクスチャーサイズでテクスチャー情報を初期化する
		OrigTex = Orig->Hard.Tex ;
		for( i = 0 ; i < DivNum ; i ++, OrigTex ++ )
		{
			OrigTex->UseWidth  = Orig->Width  - TexPosX[ i ] ;
			OrigTex->UseHeight = Orig->Height - TexPosY[ i ] ;
			if( OrigTex->UseWidth  > TexSizeX[ i ] ) OrigTex->UseWidth  = TexSizeX[ i ] ; 
			if( OrigTex->UseHeight > TexSizeY[ i ] ) OrigTex->UseHeight = TexSizeY[ i ] ;

			OrigTex->OrigPosX = TexPosX[ i ] ;
			OrigTex->OrigPosY = TexPosY[ i ] ;

			OrigTex->TexWidth  = TexSizeX[ i ] ;
			OrigTex->TexHeight = TexSizeY[ i ] ;
		}

/*
		// 描画可能で解像度がバックバッファ以上の場合は専用のＺバッファを持つ
		if( Orig->FormatDesc.DrawValidFlag &&
			( Orig->Hard.Tex[ 0 ].TexWidth  > GSYS.Screen.MainScreenSizeX ||
			  Orig->Hard.Tex[ 0 ].TexHeight > GSYS.Screen.MainScreenSizeY ) )
		{
			Orig->ZBufferFlag = TRUE ;
		}
*/
		// 描画可能な場合でＺバッファを作成するフラグが立っている場合はＺバッファを持つ
		if( Orig->FormatDesc.DrawValidFlag && GParam->DrawValidGraphCreateZBufferFlag )
		{
			Orig->ZBufferFlag = TRUE ;
			switch( GParam->CreateDrawValidGraphZBufferBitDepth )
			{
			default :
			case 16 : Orig->ZBufferBitDepthIndex = ZBUFFER_FORMAT_16BIT ; break ;
			case 24 : Orig->ZBufferBitDepthIndex = ZBUFFER_FORMAT_24BIT ; break ;
			case 32 : Orig->ZBufferBitDepthIndex = ZBUFFER_FORMAT_32BIT ; break ;
			}
		}

		// テクスチャの作成
		if( Graphics_Hardware_CreateOrigTexture_PF( Orig, ASyncThread ) == -1 )
		{
			goto ERR ;
		}
	}
	else
	{
		// テクスチャではない場合

		// ブレンド用画像の場合とそれ以外で処理を分岐
		if( Orig->FormatDesc.BlendGraphFlag == TRUE )
		{
			// イメージの作成
			if( InitializeMemImg( &Orig->Soft.MemImg, Orig->Width, Orig->Height, -1, 0, 2, FALSE, FALSE, FALSE ) < 0 )
			{
				DXST_ERRORLOG_ADDUTF16LE( "\xd6\x30\xec\x30\xf3\x30\xc9\x30\x3b\x75\xcf\x50\x28\x75\xe1\x30\xe2\x30\xea\x30\xa4\x30\xe1\x30\xfc\x30\xb8\x30\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"ブレンド画像用メモリイメージの作成に失敗しました\n" @*/ ) ;
				goto ERR ;
			}
		}
		else
		{
			DWORD TransColor ;

			// ピクセルフォーマットのインデックスをセット
			if( Orig->FormatDesc.ColorBitDepth == 16 )
			{
				if( Orig->FormatDesc.AlphaChFlag )	Orig->ColorFormat = DX_GRAPHICSIMAGE_FORMAT_X8A8R5G6B5 ;
				else								Orig->ColorFormat = DX_GRAPHICSIMAGE_FORMAT_R5G6B5 ;
			}
			else
			{
				if( Orig->FormatDesc.AlphaChFlag )	Orig->ColorFormat = DX_GRAPHICSIMAGE_FORMAT_A8R8G8B8 ;
				else								Orig->ColorFormat = DX_GRAPHICSIMAGE_FORMAT_X8R8G8B8 ;
			}
			
			if( Orig->FormatDesc.UsePaletteFlag == TRUE )
			{
				TransColor = 0xffff ;
			}
			else
			{
				TransColor = NS_GetColor3( 
						GetMemImgColorData( Orig->FormatDesc.ColorBitDepth == 16 ? 0 : 1, ( int )Orig->FormatDesc.AlphaChFlag, FALSE ),
						( int )( ( GSYS.CreateImage.TransColor >> 16 ) & 0xff ),
						( int )( ( GSYS.CreateImage.TransColor >> 8  ) & 0xff ),
						( int )( ( GSYS.CreateImage.TransColor >> 0  ) & 0xff ),
						0xff ) ;
			}

			// イメージの作成
			if( InitializeMemImg(
					&Orig->Soft.MemImg,
					Orig->Width,
					Orig->Height,
					-1,
					TransColor,
					Orig->FormatDesc.ColorBitDepth == 16 ? 0 : 1,
					Orig->FormatDesc.UsePaletteFlag,
					Orig->FormatDesc.AlphaChFlag,
					FALSE
				) < 0 )
			{
				DXST_ERRORLOG_ADDUTF16LE( "\xe1\x30\xe2\x30\xea\x30\xa4\x30\xe1\x30\xfc\x30\xb8\x30\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"メモリイメージの作成に失敗しました\n" @*/ ) ;
				goto ERR ;
			}
		}
	}

	// 描画情報の初期化
	if( Graphics_Image_InitializeDrawInfo( GrHandle, ASyncThread ) < 0 )
	{
		goto ERR ;
	}

	// 終了
	return 0 ;

	// エラー終了
ERR :
//	NS_DeleteGraph( GrHandle ) ;

	return -1 ;
}

// 指定のテクスチャーサイズを上手く分割する
extern int Graphics_Image_ListUpTexSize( int Size, short *SizeList, int NotDivFlag, int Pow2Flag, int MaxTextureSize )
{
	int DivNum ;
	int TexSize, GraphSize, MaxTexSize ;

	// 最大サイズをセット
	MaxTexSize = MaxTextureSize != 0 ? MaxTextureSize : GSYS.HardInfo.MaxTextureSize ;

	// ２のｎ乗ではなくて良い場合は指定のサイズが最大テクスチャサイズ以下だったらそのまま返す
	if( Pow2Flag == FALSE && ( GSYS.HardInfo.TextureSizePow2 == FALSE || GSYS.HardInfo.TextureSizeNonPow2Conditional ) )
	{
		if( Size <= MaxTexSize )
		{
			if( SizeList ) SizeList[ 0 ] = ( short )Size ;
			return 1 ;
		}
	}

	// 分割を行う最小サイズのテクスチャより小さい場合は最小サイズをセットして１を返す
	if( MIN_TEXTURE_SIZE >= Size )
	{
		if( SizeList ) SizeList[ 0 ] = MIN_TEXTURE_SIZE ;
		return 1 ;
	}

	// １番最初のテクスチャーは指定サイズに収まる出きるだけ大きいテクスチャーにする
	DivNum = 0 ;
	TexSize = MaxTexSize ;
	while( TexSize > MIN_TEXTURE_SIZE )
	{
		if( TexSize <= Size ) break ;
		TexSize >>= 1 ;
	}

	// 分割を行わない場合は一枚で収められる場合は収めて終了
	if( NotDivFlag != FALSE )
	{
		if( ( TexSize << 1 ) <= MaxTexSize )
		{
			if( TexSize < Size ) TexSize <<= 1 ;
			if( SizeList ) SizeList[ 0 ] = ( short )TexSize ;
			return 1 ;
		}
	}
	else
	// 分割を行う指定の場合でも作成する画像のサイズが
	// 画面と同じかそれよりも小さく、且つユーザー指定のサイズが無い場合は
	// なるべく分割を行わない
	if( MaxTextureSize == 0 )
	{
		if( Size <= GSYS.Screen.MainScreenSizeX )
		{
			if( ( TexSize << 1 ) <= MaxTexSize )
			{
				if( TexSize < Size ) TexSize <<= 1 ;
				if( SizeList ) SizeList[ 0 ] = ( short )TexSize ;
				return 1 ;
			}
		}
	}

	// 分割処理
	GraphSize = Size ;
	for(;;)
	{
		if( SizeList ) SizeList[ DivNum ] = ( short )TexSize ;
		GraphSize -= TexSize ;

		if( GraphSize <= 0 ) break ;

		// もう一つテクスチャーを作るよりサイズを２倍にしたほうがいい場合を判定
		if( ( GraphSize > TexSize * 3 / 4 ) && ( TexSize < MaxTexSize ) )
		{
			if( SizeList ) SizeList[ DivNum ] = ( short )( TexSize << 1 ) ;
			break ;
		}
		else
		{
			// テクスチャーのサイズを残りグラフィックサイズに合わせて補正する
			if( GraphSize < TexSize )
			{
				while( GraphSize < TexSize  &&  TexSize > MIN_TEXTURE_SIZE )
				{
					TexSize = TexSize >> 1 ;
				}
				DivNum ++ ; 
				continue ;
			}
		}
		DivNum ++ ;
	}

	// 最後のサイズが、もし最後の一つ前のサイズと等しい場合は
	// 最後の一つ前のサイズを２倍にして分割情報を一つ減らす
	if( DivNum > 0 && SizeList[ DivNum - 1 ] == SizeList[ DivNum ] &&
		( SizeList[ DivNum - 1 ] << 1 ) <= MaxTexSize )
	{
		DivNum -- ;
		SizeList[ DivNum ] <<= 1 ;
	}

	// 分割数を返す
	return DivNum + 1 ;
}

// グラフィックハンドルの初期化
extern int Graphics_Image_InitializeHandle( HANDLEINFO *HandleInfo )
{
	IMAGEDATA *Image = ( IMAGEDATA * )HandleInfo ;

#ifndef DX_NON_MOVIE
	// ムービーハンドルは無し
	Image->MovieHandle = -1 ;
#endif

	// 終了
	return 0 ;
}

// グラフィックハンドルの後始末
extern int Graphics_Image_TerminateHandle( HANDLEINFO *HandleInfo )
{
	IMAGEDATA *Image = ( IMAGEDATA * )HandleInfo ;
	int i ;

	// ムービーグラフィックだった場合その後始末を行う
#ifndef DX_NON_MOVIE
	CloseMovie( Image->MovieHandle ) ;
#endif

	// 描画先になっていたら描画先を表画面にする
	for( i = 0 ; i < GSYS.HardInfo.RenderTargetNum ; i ++ )
	{
		if( GSYS.DrawSetting.TargetScreen[ i ] == Image->HandleInfo.Handle )
		{
			NS_SetRenderTargetToShader( i, ( int )( i == 0 ? ( int )DX_SCREEN_FRONT : -1 ) ) ;
		}
	}

	// 描画先Ｚバッファになっていたら描画先を裏画面にする
	if( GSYS.DrawSetting.TargetZBuffer == Image->HandleInfo.Handle )
	{
		NS_SetDrawZBuffer( DX_SCREEN_BACK ) ;
	}

	// オリジナルデータがある場合はオリジナルデータの解放処理を行う
	if( Image->Orig )
	{
		// テクスチャかどうかで処理を分岐
		if( Image->Orig->FormatDesc.TextureFlag )
		{
			// テクスチャの場合
		}
		else
		{
			// テクスチャでは無い場合

			// 描画用 MEMIMG の後始末
			TerminateMemImg( &Image->Soft.MemImg ) ;
		}

		// オリジナルデータの参照数をデクリメント
		Image->Orig->RefCount -- ;

		// ０になったらオリジナルデータの解放処理を行う
		if( Image->Orig->RefCount == 0 )
		{
			// Ｚバッファを解放する
			//ReleaseOrigZBuffer_PF( Image->Orig ) ;

			// テクスチャかどうかで処理を分岐
			if( Image->Orig->FormatDesc.TextureFlag )
			{
				// テクスチャの場合

				// テクスチャオブジェクトの解放
				Graphics_Hardware_ReleaseOrigTexture_PF( Image->Orig ) ;
			}
			else
			{
				// テクスチャでは無い場合

				// MEMIMG を解放する
				TerminateMemImg( &Image->Orig->Soft.MemImg ) ;
			}

			// オリジナル画像データ用に確保したメモリ領域を開放
			DXFREE( Image->Orig ) ;
		}

		Image->Orig = NULL ;
	}

	// フルカラーイメージがある場合は解放
	if( Image->FullColorImage != NULL )
	{
		DXFREE( Image->FullColorImage ) ;
		Image->FullColorImage = NULL ;
	}

	// ロックイメージがある場合は解放
	if( Image->LockImage != NULL )
	{
		DXFREE( Image->LockImage ) ;
		Image->LockImage = NULL ;
	}

	// ファイル名保存用にメモリを確保していた場合は解放
	Graphics_Image_SetGraphBaseInfo( Image->HandleInfo.Handle, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, NULL, NULL, FALSE, -1, FALSE ) ;

	// 解放時に立てるフラグへのポインタが有効である場合は立てる
	if( Image->LostFlag != NULL )
		*Image->LostFlag = TRUE ;

	// 終了
	return 0 ;
}

// 指定部分だけを抜き出したグラフィックハンドルを初期化する
extern int Graphics_Image_InitializeDerivationHandle( int GrHandle, int SrcX, int SrcY, int Width, int Height, int SrcGrHandle, int ASyncThread )
{
	IMAGEDATA *Image, *SrcImage ;
	IMAGEDATA_ORIG *Orig ;

	// アドレスの取得
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) ||
			GRAPHCHK_ASYNC( SrcGrHandle, SrcImage ) )
			return -1 ;
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;

		if( GRAPHCHK( SrcGrHandle, SrcImage ) )
			return -1 ;
	}
	Orig = SrcImage->Orig ;

	// オリジナル画像の参照数をインクリメントする
	Image->Orig = Orig ;
	Orig->RefCount ++ ;

	// ファイルの情報をコピーする
	Graphics_Image_SetName( GrHandle, NULL, FALSE, SrcGrHandle, ASyncThread ) ;
	Image->UseBaseX = SrcImage->UseBaseX + SrcX ;
	Image->UseBaseY = SrcImage->UseBaseY + SrcY ;

	// オリジナル画像中の参照座標をセット
	Image->UseOrigX = SrcImage->UseOrigX + SrcX ;
	Image->UseOrigY = SrcImage->UseOrigY + SrcY ;

	// サイズを保存
	Image->Width = Width ;
	Image->Height = Height ;

	// InitGraph で削除するかどうかを保存
	Image->NotInitGraphDelete = SrcImage->NotInitGraphDelete ;

	//描画情報の初期化
	Graphics_Image_InitializeDrawInfo( GrHandle, ASyncThread ) ;

	// 終了
	return 0 ;
}

// グラフィックハンドルの描画情報を初期化する
extern int Graphics_Image_InitializeDrawInfo( int GrHandle, int ASyncThread )
{
	IMAGEDATA					*Image ;
	IMAGEDATA_ORIG				*Orig ;
	IMAGEDATA_ORIG_HARD_TEX	*OrigTex ;
	IMAGEDATA_HARD_DRAW		*DrawTex ;
	int							i ;
	int							Num ;
	int							Size ;
	int							Handle ;
	bool						List[ 256 ] ;
	double						d ;

	// アドレスの取得
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) )
		{
			return -1 ;
		}
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
		{
			return -1 ;
		}
	}
	Orig   = Image->Orig ;
	Handle = Image->HandleInfo.Handle ;

	// 基本確保メモリの算出
	Size = sizeof( IMAGEDATA ) - ( ( sizeof( IMAGEDATA_SOFT ) > sizeof( IMAGEDATA_HARD ) ? sizeof( IMAGEDATA_SOFT ) : sizeof( IMAGEDATA_HARD ) ) ) ;

	// テクスチャかどうかで処理を分岐
	if( Orig->FormatDesc.TextureFlag )
	{
		// テクスチャの場合

		// 頂点タイプ計算に使用する値の準備
		d = ( double )Image->Height / ( double )Image->Width ;

		// 元画像から被る部分の数を数える
		OrigTex = Orig->Hard.Tex ;
		Num = 0 ;
		for( i = 0 ; i < Orig->Hard.TexNum ; i ++, OrigTex ++ )
		{
			if(
				(
				  (
				    OrigTex->OrigPosX <= Image->UseOrigX  &&
					OrigTex->OrigPosX + OrigTex->UseWidth  > Image->UseOrigX
				  ) ||
				  (
				    Image->UseOrigX  <= OrigTex->OrigPosX &&
					Image->UseOrigX  + Image->Width      > OrigTex->OrigPosX
				  )
				) &&
				(
				  (
				    OrigTex->OrigPosY <= Image->UseOrigY  &&
					OrigTex->OrigPosY + OrigTex->UseHeight > Image->UseOrigY
				  ) ||
				  (
				    Image->UseOrigY  <= OrigTex->OrigPosY &&
					Image->UseOrigY  + Image->Height     > OrigTex->OrigPosY
				  )
				)
			  )
			{
				Num ++ ;
				List[ i ] = true ;
			}
			else
			{
				List[ i ] = false ;
			}
		}

		// 被る部分が０の場合はエラー
		if( Num == 0 )
		{
			DXST_ERRORLOG_ADDUTF16LE( "\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\xcf\x63\x3b\x75\xc5\x60\x31\x58\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"テクスチャ描画情報の作成に失敗しました\n" @*/ ) ;
			return -1 ;
		}

		// 描画情報を格納するメモリ領域が足りない場合は再確保
		if( Num > 4 )
		{
			if( ReallocHandle( Handle, Size + sizeof( IMAGEDATA_HARD ) + ( Num - 4 ) * sizeof( IMAGEDATA_HARD_DRAW ) ) < 0 )
			{
				DXST_ERRORLOG_ADDUTF16LE( "\x3b\x75\xcf\x50\xcf\x63\x3b\x75\xc5\x60\x31\x58\x3c\x68\x0d\x7d\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"画像描画情報格納用メモリの確保に失敗しました\n" @*/ ) ;
				return -1 ;
			}
			Image = ( IMAGEDATA * )GetHandleInfo( Handle ) ;
			Orig = Image->Orig ;
		}

		Image->Hard.DrawNum = Num ;

		// 描画情報をセットする
		OrigTex = Orig->Hard.Tex ;
		DrawTex = Image->Hard.Draw ;
		Num     = 0 ;
		for( i = 0 ; i < Orig->Hard.TexNum ; i ++, OrigTex ++ )
		{
			if( List[ i ] == false ) continue ;

			// 使用するオリジナル画像テクスチャ情報のアドレスをセット
			DrawTex->Tex = OrigTex ;

			// オリジナル画像テクスチャ中のどの部分を使用するか、などの情報をセット
			{
				if( OrigTex->OrigPosX > Image->UseOrigX )
				{
					DrawTex->DrawPosX = OrigTex->OrigPosX - Image->UseOrigX ;
					DrawTex->UsePosX  = 0 ;

					if( OrigTex->OrigPosX + OrigTex->UseWidth > Image->UseOrigX + Image->Width )
					{
						DrawTex->Width = Image->UseOrigX + Image->Width - OrigTex->OrigPosX ;
					}
					else
					{
						DrawTex->Width = OrigTex->UseWidth ;
					}
				}
				else
				{
					DrawTex->DrawPosX = 0 ;
					DrawTex->UsePosX  = Image->UseOrigX - OrigTex->OrigPosX ;

					if( OrigTex->OrigPosX + OrigTex->UseWidth > Image->UseOrigX + Image->Width )
					{
						DrawTex->Width = Image->Width ;
					}
					else
					{
						DrawTex->Width = OrigTex->OrigPosX + OrigTex->UseWidth - Image->UseOrigX ;
					}
				}

				if( OrigTex->OrigPosY > Image->UseOrigY )
				{
					DrawTex->DrawPosY = OrigTex->OrigPosY - Image->UseOrigY ;
					DrawTex->UsePosY  = 0 ;

					if( OrigTex->OrigPosY + OrigTex->UseHeight > Image->UseOrigY + Image->Height )
					{
						DrawTex->Height = Image->UseOrigY + Image->Height - OrigTex->OrigPosY ;
					}
					else
					{
						DrawTex->Height = OrigTex->UseHeight ;
					}
				}
				else
				{
					DrawTex->DrawPosY = 0 ;
					DrawTex->UsePosY  = Image->UseOrigY - OrigTex->OrigPosY ;

					if( OrigTex->OrigPosY + OrigTex->UseHeight > Image->UseOrigY + Image->Height )
					{
						DrawTex->Height = Image->Height ;
					}
					else
					{
						DrawTex->Height = OrigTex->OrigPosY + OrigTex->UseHeight - Image->UseOrigY ;
					}
				}
			}

			// 算出した使用域情報から頂点データに使用する値の割り出し
			DrawTex->Vertex[ 0 ].x = ( float )  DrawTex->DrawPosX ;
			DrawTex->Vertex[ 1 ].x = ( float )( DrawTex->DrawPosX + DrawTex->Width ) ;
			DrawTex->Vertex[ 2 ].x = ( float )  DrawTex->DrawPosX ;
			DrawTex->Vertex[ 3 ].x = ( float )( DrawTex->DrawPosX + DrawTex->Width ) ;

			DrawTex->Vertex[ 0 ].y = ( float )  DrawTex->DrawPosY ;
			DrawTex->Vertex[ 1 ].y = ( float )  DrawTex->DrawPosY ;
			DrawTex->Vertex[ 2 ].y = ( float )( DrawTex->DrawPosY + DrawTex->Height ) ;
			DrawTex->Vertex[ 3 ].y = ( float )( DrawTex->DrawPosY + DrawTex->Height ) ;

			DrawTex->Vertex[ 0 ].u = ( float )  DrawTex->UsePosX                    / ( float )OrigTex->TexWidth ;
			DrawTex->Vertex[ 1 ].u = ( float )( DrawTex->UsePosX + DrawTex->Width ) / ( float )OrigTex->TexWidth ;
			DrawTex->Vertex[ 2 ].u = ( float )  DrawTex->UsePosX                    / ( float )OrigTex->TexWidth ;
			DrawTex->Vertex[ 3 ].u = ( float )( DrawTex->UsePosX + DrawTex->Width ) / ( float )OrigTex->TexWidth ;

			DrawTex->Vertex[ 0 ].v = ( float )  DrawTex->UsePosY                     / ( float )OrigTex->TexHeight ;
			DrawTex->Vertex[ 1 ].v = ( float )  DrawTex->UsePosY                     / ( float )OrigTex->TexHeight ;
			DrawTex->Vertex[ 2 ].v = ( float )( DrawTex->UsePosY + DrawTex->Height ) / ( float )OrigTex->TexHeight ;
			DrawTex->Vertex[ 3 ].v = ( float )( DrawTex->UsePosY + DrawTex->Height ) / ( float )OrigTex->TexHeight ;

			// 頂点タイプのセット
			DrawTex->VertType[ 0 ] = ( unsigned char )( DrawTex->Vertex[ 0 ].y <= Image->Height - ( d * DrawTex->Vertex[ 0 ].x ) ? 1 : 0 ) ;
			DrawTex->VertType[ 1 ] = ( unsigned char )( DrawTex->Vertex[ 1 ].y <= Image->Height - ( d * DrawTex->Vertex[ 1 ].x ) ? 1 : 0 ) ;
			DrawTex->VertType[ 2 ] = ( unsigned char )( DrawTex->Vertex[ 2 ].y <= Image->Height - ( d * DrawTex->Vertex[ 2 ].x ) ? 1 : 0 ) ;
			DrawTex->VertType[ 3 ] = ( unsigned char )( DrawTex->Vertex[ 3 ].y <= Image->Height - ( d * DrawTex->Vertex[ 3 ].x ) ? 1 : 0 ) ;

			DrawTex ++ ;
		}
	}
	else
	{
		// テクスチャではない場合

		// 描画用 MEMIMG の準備
		DerivationMemImg( &Image->Soft.MemImg, &Orig->Soft.MemImg, Image->UseOrigX, Image->UseOrigY, Image->Width, Image->Height ) ;
	}

	// 終了
	return 0 ;
}

// グラフィックハンドルが有効かどうかを調べる( TRUE:有効  FALSE:無効 )
extern	int Graphics_Image_IsValidHandle( int GrHandle )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( GRAPHCHKFULL( GrHandle, Image ) )
		return FALSE ;

	// 終了
	return TRUE ;
}

// GraphImage データからサイズを割り出し、それに合ったグラフィックハンドルを作成する
extern int Graphics_Image_CreateDXGraph_UseGParam(
	SETUP_GRAPHHANDLE_GPARAM *GParam,
	int GrHandle,
	const BASEIMAGE *RgbBaseImage,
	const BASEIMAGE *AlphaBaseImage,
	int TextureFlag,
	int ASyncThread
)
{
	if( GSYS.InitializeFlag == FALSE ) return -1 ;

	if( ASyncThread == FALSE )
	{
		CheckActiveState() ;
	}

	// 二つのグラフィックのサイズが違った場合はエラー
	if( AlphaBaseImage != NULL && ( RgbBaseImage->Width != AlphaBaseImage->Width || RgbBaseImage->Height != AlphaBaseImage->Height ) )
		return -1 ;

	// キューブマップテクスチャを作成するかどうかをセット
	GParam->CubeMapTextureCreateFlag = RgbBaseImage->GraphDataCount == CUBEMAP_SURFACE_NUM ? TRUE : FALSE ;

	// ハンドルの初期化
	{
		int UsePaletteFlag ;
		int PaletteColorBitDepth = 0 ;

		// パレットテクスチャを使用するかどうかをセット
		UsePaletteFlag = RgbBaseImage->ColorData.PixelByte  == 1 && 
						 RgbBaseImage->ColorData.AlphaWidth == 0 &&
						 AlphaBaseImage == NULL ;

		// グラフィックスデバイスが4bitパレットテクスチャに対応している場合で、且つ8bitカラーの場合は
		// 使用されている最大パレット番号が 4bit に収まるかどうかを調べ、収まる場合は 4bit パレットテクスチャにする
		if( UsePaletteFlag )
		{
			if( GSYS.HardInfo.Support4bitPaletteTexture &&
				RgbBaseImage->ColorData.ColorBitDepth == 8 )
			{
				if( RgbBaseImage->ColorData.MaxPaletteNo == 0 || RgbBaseImage->ColorData.MaxPaletteNo == 255 )
				{
					PaletteColorBitDepth = NS_GetBaseImageUseMaxPaletteNo( RgbBaseImage ) < 16 ? 4 : 8 ;
				}
				else
				{
					PaletteColorBitDepth = RgbBaseImage->ColorData.MaxPaletteNo < 16 ? 4 : 8 ;
				}
			}
			else
			{
				PaletteColorBitDepth = 8 ;
			}
		}

		if( Graphics_Image_SetupHandle_UseGParam(
				GParam,
				GrHandle,
				RgbBaseImage->Width, RgbBaseImage->Height, TextureFlag,
				RgbBaseImage->ColorData.AlphaWidth != 0 || AlphaBaseImage != NULL,
				UsePaletteFlag,
				PaletteColorBitDepth,
				RgbBaseImage->ColorData.Format,
				RgbBaseImage->MipMapCount == 0 ? -1 : RgbBaseImage->MipMapCount,
				ASyncThread ) == -1 )
			return -1 ;
	}

	// 終了
	return 0 ;
}

// DerivationGraph のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_DerivationGraph_UseGParam(
	int SrcX,
	int SrcY,
	int Width,
	int Height,
	int SrcGraphHandle,
	int ASyncThread
)
{
	int NewGraphHandle ;
	int Result ;

	// 新たなグラフィックデータの追加
	NewGraphHandle = Graphics_Image_AddHandle( ASyncThread ) ;
	if( NewGraphHandle == -1 )
	{
		return -1 ;
	}

	Result = Graphics_Image_DerivationGraphBase( NewGraphHandle, SrcX, SrcY, Width, Height, SrcGraphHandle, ASyncThread ) ;
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( NewGraphHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD
	if( Result < 0 )
	{
		NS_DeleteGraph( NewGraphHandle ) ;
		return -1 ;
	}

	// ハンドルを返す
	return NewGraphHandle ;
}

#ifndef DX_NON_MOVIE

extern int Graphics_Image_OpenMovie_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int GrHandle,
	const wchar_t *GraphName,
	int TextureFlag,
	int SurfaceMode,
	int ASyncThread
)
{
	int MovieHandle ;
	int Width, Height ;
	MOVIEGRAPH *Movie;
	IMAGEDATA *Image ;

	// ムービーグラフィックとして扱おうとしてみる
	MovieHandle = OpenMovie_UseGParam( &GParam->OpenMovieGParam, GraphName, &Width, &Height, SurfaceMode, ASyncThread ) ;
	if( MovieHandle < 0 )
	{
		DXST_ERRORLOGFMT_ADDW(( L"Image File Load Error : %s \n", GraphName )) ;
		return -1 ;
	}

	Movie = GetMovieData( MovieHandle );

	// 画面のカラービットデプスと対応するようにセット、及びＶＲＡＭ未使用セット
	GParam->CreateGraphGParam.InitHandleGParam.CreateImageColorBitDepth = NS_GetColorBitDepth() ;
#ifndef DX_NON_DSHOW_MOVIE
	#ifndef DX_NON_OGGTHEORA
		GParam->CreateGraphGParam.InitHandleGParam.AlphaChannelImageCreateFlag = Movie->A8R8G8B8Flag && Movie->TheoraFlag == FALSE && Movie->PF.pMovieImage->ImageType == 1 ;
	#else // DX_NON_OGGTHEORA
		GParam->CreateGraphGParam.InitHandleGParam.AlphaChannelImageCreateFlag = Movie->A8R8G8B8Flag && Movie->PF.pMovieImage->ImageType == 1 ;
	#endif // DX_NON_OGGTHEORA
#else // DX_NON_DSHOW_MOVIE
	GParam->CreateGraphGParam.InitHandleGParam.AlphaChannelImageCreateFlag = FALSE ;
#endif // DX_NON_DSHOW_MOVIE
	GParam->CreateGraphGParam.InitHandleGParam.CreateImageChannelBitDepth = 0 ;
	GParam->CreateGraphGParam.InitHandleGParam.NotUseDivFlag = TRUE ;
	GParam->CreateGraphGParam.InitHandleGParam.AlphaTestImageCreateFlag = FALSE ;
	GParam->CreateGraphGParam.InitHandleGParam.CubeMapTextureCreateFlag = FALSE ;
	GParam->CreateGraphGParam.InitHandleGParam.UseManagedTextureFlag = FALSE ;
	GParam->CreateGraphGParam.InitHandleGParam.MipMapCount = 0 ;
#ifndef DX_NON_OGGTHEORA
	if( Movie->TheoraFlag && Movie->RightAlpha == FALSE )
	{
		GParam->CreateGraphGParam.InitHandleGParam.DrawValidImageCreateFlag = TRUE ;
		GParam->CreateGraphGParam.InitHandleGParam.DrawValidAlphaImageCreateFlag = FALSE ;
		GParam->CreateGraphGParam.InitHandleGParam.DrawValidFloatTypeGraphCreateFlag = FALSE ;
		GParam->CreateGraphGParam.InitHandleGParam.DrawValidGraphCreateZBufferFlag = FALSE ;
		GParam->CreateGraphGParam.InitHandleGParam.CreateDrawValidGraphChannelNum = 0 ;
		GParam->CreateGraphGParam.InitHandleGParam.CreateDrawValidGraphZBufferBitDepth = 0 ;
		GParam->CreateGraphGParam.InitHandleGParam.DrawValidMSSamples = 0 ;
		GParam->CreateGraphGParam.InitHandleGParam.DrawValidMSQuality = 0 ;
	}
#endif // DX_NON_OGGTHEORA

	// 読みこんだビットマップのサイズのグラフィックデータを作成する
	// 右側半分をαチャンネルとして扱う場合はαチャンネル付き画像にする
	if( Graphics_Image_SetupHandle_UseGParam(
			&GParam->CreateGraphGParam.InitHandleGParam,
			GrHandle,
			Movie->RightAlpha ? Width / 2 : Width,
			Height,
			TextureFlag,
			Movie->RightAlpha || GParam->CreateGraphGParam.InitHandleGParam.AlphaChannelImageCreateFlag,
			FALSE,
			0,
			DX_BASEIMAGE_FORMAT_NORMAL,
			-1,
			ASyncThread
		) == -1 )
	{
		DXST_ERRORLOGFMT_ADDW(( L"Graph Handle Setup Error : %s", GraphName )) ;
		return -1 ;
	}

	Image = Graphics_Image_GetData( GrHandle, ASyncThread ) ;
	if( Image )
	{
		// ムービーのハンドルをセット
		Image->MovieHandle = MovieHandle ;

		// コールバック関数をセット
		SetCallbackMovie( MovieHandle, ( void (*)( MOVIEGRAPH *, void * ) )Graphics_Image_UpdateGraphMovie, ( void * )( DWORD_PTR )GrHandle ) ;
	}

	// 正常終了
	return 0 ;
}

#endif // DX_NON_MOVIE

// グラフィックハンドルを作成しない DerivationGraph
extern int Graphics_Image_DerivationGraphBase( int GrHandle, int SrcX, int SrcY, int Width, int Height, int SrcGraphHandle, int ASyncThread )
{
	IMAGEDATA *Image ;

//	if( GSYS.NotDrawFlag == TRUE ) return 0 ;

	// エラー判定
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( SrcGraphHandle, Image ) )
			return -1 ;
	}
	else
	{
		if( GRAPHCHK( SrcGraphHandle, Image ) )
			return -1 ;
	}

	// 補正
	if( SrcX < 0 || SrcY < 0 || SrcX >= Image->Width || SrcY >= Image->Height ) return -1 ;
	if( SrcX + Width  > Image->Width  ) Width  = Image->Width  - SrcX ;
	if( SrcY + Height > Image->Height ) Height = Image->Height - SrcY ;

	// 抜き出しグラフィックの初期化
	if( Graphics_Image_InitializeDerivationHandle( GrHandle, SrcX, SrcY, Width, Height, SrcGraphHandle, ASyncThread ) == -1 )
		return -1 ;

	// 正常終了
	return 0 ;
}

// Graphics_Image_SetGraphBaseInfo のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_SetBaseInfo_UseGParam(
	SETGRAPHBASEINFO_GPARAM *GParam,
	int GrHandle,
	const wchar_t *FileName,
	const COLORDATA *BmpColorData,
	HBITMAP RgbBmp,
	HBITMAP AlphaBmp,
	const void *MemImage,
	int MemImageSize,
	const void *AlphaMemImage,
	int AlphaMemImageSize,
	const BASEIMAGE *BaseImage,
	const BASEIMAGE *AlphaBaseImage,
	int ReverseFlag,
	int UnionGrHandle,
	int ASyncThread
)
{
	IMAGEDATA *Image, *SrcImage = NULL ;
	wchar_t Path[ 1024 ] ;
	int Len ;
	BASEIMAGE BmpImage, AlphaBmpImage ;

	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) )
			return -1 ;
		GRAPHCHKFULL_ASYNC( UnionGrHandle, SrcImage ) ;
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
		GRAPHCHKFULL( UnionGrHandle, SrcImage ) ;
	}

	// 既にファイルパス用のメモリが確保されていたら解放
	if( Image->ReadBase )
	{
		Image->ReadBase->RefCount -- ;
		if( Image->ReadBase->RefCount == 0 )
		{
			DXFREE( Image->ReadBase ) ;
			Image->ReadBase = NULL ;
		}
	}

	// ビットマップが有効な場合は BASEIMAGE 構造体を構築する
	if( RgbBmp )
	{
#ifdef __WINDOWS__
		BITMAP BitmapInfo ;
		BITMAP AlphaBitmapInfo ;

		GetObject( RgbBmp, sizeof( BITMAP ), ( void * )&BitmapInfo ) ;
		_MEMSET( &BmpImage, 0, sizeof( BASEIMAGE ) ) ;
		BmpImage.GraphData = BitmapInfo.bmBits ;
		BmpImage.Width = BitmapInfo.bmWidth ;
		BmpImage.Height = BitmapInfo.bmHeight ;
		BmpImage.Pitch = ( BmpImage.Pitch + 3 ) / 4 * 4 ;
		_MEMCPY( &BmpImage.ColorData, BmpColorData, sizeof( COLORDATA ) ) ; 
		BaseImage = &BmpImage ;

		if( AlphaBmp )
		{
			GetObject( AlphaBmp, sizeof( BITMAP ), ( void * )&AlphaBitmapInfo ) ;
			_MEMSET( &AlphaBmpImage, 0, sizeof( BASEIMAGE ) ) ;
			AlphaBmpImage.GraphData = AlphaBitmapInfo.bmBits ;
			AlphaBmpImage.Width = AlphaBitmapInfo.bmWidth ;
			AlphaBmpImage.Height = AlphaBitmapInfo.bmHeight ;
			AlphaBmpImage.Pitch = ( AlphaBmpImage.Pitch + 3 ) / 4 * 4 ;
			_MEMCPY( &AlphaBmpImage.ColorData, BmpColorData, sizeof( COLORDATA ) ) ; 
			AlphaBaseImage = &AlphaBmpImage ;
		}
#else // __WINDOWS__
		return -1 ;
#endif // __WINDOWS__
	}

	// 元データの情報が一切ないか、バックアップを拒否するフラグが立っていた場合はバックアップをしない
	if( GParam->NotGraphBaseDataBackupFlag || ( FileName == NULL && MemImage == NULL && BaseImage == NULL && SrcImage == NULL ) )
	{
		Image->ReadBase = NULL ;
	}
	else
	{
		// 共有ハンドルの指定がある場合は共有情報をセットする
		if( SrcImage != NULL )
		{
			// ファイル名情報のアドレスを貰う
			Image->ReadBase = SrcImage->ReadBase ;

			// 参照数を増やす
			if( SrcImage->ReadBase )
			{
				SrcImage->ReadBase->RefCount ++ ;
			}
		}
		else
		{
			// ファイル名が有効な場合はファイル名を保存
			if( FileName )
			{
				// フルパスを取得
				ConvertFullPathW_( FileName, Path ) ;

				// ファイル名保存用のメモリを確保
				Len = _WCSLEN( Path ) ;
				Image->ReadBase = ( IMAGEDATA_READBASE * )DXCALLOC( sizeof( IMAGEDATA_READBASE ) + ( Len + 1 ) * sizeof( wchar_t ) ) ;
				if( Image->ReadBase == NULL )
				{
					DXST_ERRORLOG_ADDUTF16LE( "\xe2\x8e\x01\x90\x59\x30\x8b\x30\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\x6e\x30\xd5\x30\xa1\x30\xa4\x30\xeb\x30\x0d\x54\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"転送する画像ファイルのファイル名を保存するメモリ領域の確保に失敗しました\n" @*/ ) ;
					return -1 ;
				}
				Image->ReadBase->FileName = ( wchar_t * )( Image->ReadBase + 1 ) ;

				// ファイルパスを保存
				_MEMCPY( Image->ReadBase->FileName, Path, ( Len + 1 ) * sizeof( wchar_t ) ) ;
			}
			else
			// メモリイメージが有効な場合はメモリイメージを保存
			if( MemImage )
			{
				// メモリイメージ保存用のメモリを確保
				Image->ReadBase = ( IMAGEDATA_READBASE * )DXALLOC( sizeof( IMAGEDATA_READBASE ) + MemImageSize + AlphaMemImageSize ) ;
				if( Image->ReadBase == NULL )
				{
					DXST_ERRORLOG_ADDUTF16LE( "\xe2\x8e\x01\x90\x59\x30\x8b\x30\x3b\x75\xcf\x50\xc7\x30\xfc\x30\xbf\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"転送する画像データを保存するメモリ領域の確保に失敗しました\n" @*/ ) ;
					return -1 ;
				}
				_MEMSET( Image->ReadBase, 0, sizeof( IMAGEDATA_READBASE ) ) ;

				// メモリイメージを保存
				Image->ReadBase->MemImage = Image->ReadBase + 1 ;
				Image->ReadBase->MemImageSize = MemImageSize ;
				_MEMCPY( Image->ReadBase->MemImage, MemImage, ( size_t )MemImageSize ) ;

				if( AlphaMemImage )
				{
					Image->ReadBase->AlphaMemImage = ( BYTE * )Image->ReadBase->MemImage + MemImageSize ;
					Image->ReadBase->AlphaMemImageSize = AlphaMemImageSize ;
					_MEMCPY( Image->ReadBase->AlphaMemImage, AlphaMemImage, ( size_t )AlphaMemImageSize ) ;
				}
			}
			else
			// 基本イメージデータが有効な場合は基本イメージデータを保存
			if( BaseImage )
			{
				unsigned int ImageSize, AlphaImageSize = 0 ;
				unsigned int AllocSize ;

				ImageSize =  ( unsigned int )NS_GetBaseImageGraphDataSize( BaseImage ) ;
				AllocSize = ImageSize + sizeof( BASEIMAGE ) ;
				if( AlphaBaseImage )
				{
					AlphaImageSize = ( unsigned int )NS_GetBaseImageGraphDataSize( AlphaBaseImage ) ;
					AllocSize += AlphaImageSize + sizeof( BASEIMAGE ) ;
				}

				// 基本イメージ保存用のメモリを確保
				Image->ReadBase = ( IMAGEDATA_READBASE * )DXALLOC( sizeof( IMAGEDATA_READBASE ) + AllocSize ) ;
				if( Image->ReadBase == NULL )
				{
					DXST_ERRORLOG_ADDUTF16LE( "\xe2\x8e\x01\x90\x59\x30\x8b\x30\xfa\x57\x2c\x67\xa4\x30\xe1\x30\xfc\x30\xb8\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"転送する基本イメージを保存するメモリ領域の確保に失敗しました\n" @*/ ) ;
					return -1 ;
				}
				_MEMSET( Image->ReadBase, 0, sizeof( IMAGEDATA_READBASE ) ) ;

				// 基本イメージをメモリに保存
				Image->ReadBase->BaseImage = ( BASEIMAGE * )( Image->ReadBase + 1 ) ;
				_MEMCPY( Image->ReadBase->BaseImage, BaseImage, sizeof( BASEIMAGE ) ) ;
				Image->ReadBase->BaseImage->GraphData = Image->ReadBase->BaseImage + 1 ;
				_MEMCPY( Image->ReadBase->BaseImage->GraphData, BaseImage->GraphData, ImageSize ) ;

				if( AlphaBaseImage )
				{
					Image->ReadBase->AlphaBaseImage = ( BASEIMAGE * )( ( BYTE * )Image->ReadBase->BaseImage->GraphData + ImageSize ) ;
					_MEMCPY( Image->ReadBase->AlphaBaseImage, AlphaBaseImage, sizeof( BASEIMAGE ) ) ;
					Image->ReadBase->AlphaBaseImage->GraphData = Image->ReadBase->AlphaBaseImage + 1 ;
					_MEMCPY( Image->ReadBase->AlphaBaseImage->GraphData, AlphaBaseImage->GraphData, AlphaImageSize ) ;
				}
			}

			// 反転フラグを保存する
			Image->ReadBase->ReverseFlag = ReverseFlag ;

			// 参照数を１にする
			Image->ReadBase->RefCount = 1 ;
		}
	}

	// 終了
	return 0 ;
}

// 画像の元データの情報を保存する
extern int Graphics_Image_SetGraphBaseInfo(
	int GrHandle,
	const wchar_t *FileName,
	const COLORDATA *BmpColorData,
	HBITMAP RgbBmp,
	HBITMAP AlphaBmp,
	const void *MemImage,
	int MemImageSize,
	const void *AlphaMemImage,
	int AlphaMemImageSize,
	const BASEIMAGE *BaseImage,
	const BASEIMAGE *AlphaBaseImage,
	int ReverseFlag,
	int UnionGrHandle,
	int ASyncThread
)
{
	SETGRAPHBASEINFO_GPARAM GParam ;

	Graphics_Image_InitSetGraphBaseInfoGParam( &GParam ) ;

	return Graphics_Image_SetBaseInfo_UseGParam( &GParam, GrHandle, FileName, BmpColorData, RgbBmp, AlphaBmp, MemImage, MemImageSize,
										AlphaMemImage, AlphaMemImageSize, BaseImage, AlphaBaseImage, ReverseFlag, UnionGrHandle, ASyncThread ) ;
}

// 特定のファイルから画像を読み込んだ場合のファイルパスをセットする
extern int Graphics_Image_SetName( int GrHandle, const wchar_t *GraphName, int ReverseFlag, int UnionGrHandle, int ASyncThread )
{
	return Graphics_Image_SetGraphBaseInfo( GrHandle, GraphName, NULL, NULL, NULL, NULL, 0, NULL, 0, NULL, NULL, ReverseFlag, UnionGrHandle, ASyncThread ) ;
}

// FillGraph のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_FillGraph_UseGParam(
	int GrHandle,
	int Red,
	int Green,
	int Blue,
	int Alpha,
	int ASyncThread
)
{
	IMAGEDATA *Image ;
	IMAGEDATA_ORIG *Orig ;
	RECT Rect ;

	// エラー判定
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) )
			return -1 ;
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
	}
	Orig = Image->Orig ;

	// 値の補正
	if( Red   < 0 ) Red   = 0 ; else if( Red   > 255 ) Red   = 255 ;
	if( Green < 0 ) Green = 0 ; else if( Green > 255 ) Green = 255 ;
	if( Blue  < 0 ) Blue  = 0 ; else if( Blue  > 255 ) Blue  = 255 ;
	if( Alpha < 0 ) Alpha = 0 ; else if( Alpha > 255 ) Alpha = 255 ;

	// テクスチャかどうかで処理を分岐
	if( Image->Orig->FormatDesc.TextureFlag )
	{
		// テクスチャの場合
		if( Graphics_Hardware_FillGraph_PF( Image, Red, Green, Blue, Alpha, ASyncThread ) < 0 )
		{
			return -1 ;
		}
	}
	else
	{
		// テクスチャではない場合

		// DrawFillBoxMemImg で塗りつぶす
		Rect.left   = 0 ;
		Rect.top    = 0 ;
		Rect.right  = Image->Width ;
		Rect.bottom = Image->Height ;
		DrawFillBoxMemImg( &Image->Soft.MemImg, &Rect, NS_GetColor( Red, Green, Blue ) ) ; 
	}

	// 終了
	return 0 ;
}

#ifndef DX_NON_MOVIE

// ムービー画像を更新する
extern void Graphics_Image_UpdateGraphMovie( MOVIEGRAPH *Movie, DWORD_PTR GrHandle )
{
	IMAGEDATA *Image ;
	IMAGEDATA_ORIG *Orig ;

	if( GRAPHCHK( ( LONG_PTR )GrHandle, Image ) )
		return;
	Orig = Image->Orig ;

	// テクスチャかどうかで処理を分岐
	if( Image->Orig->FormatDesc.TextureFlag )
	{
		// テクスチャの場合

	//	D_DDSURFACEDESC2 SrcDesc ;
		BASEIMAGE MovieImage, AlphaBaseImage ;
		RECT SrcRect ;

		// ムービーイメージの構築
#ifndef DX_NON_OGGTHEORA
		// Ogg Theora を使用している場合はここで更新処理
		if( Movie->TheoraFlag )
		{
			if( Movie->RightAlpha == TRUE || TheoraDecode_GetYUVImage( Movie->TheoraHandle ) == NULL )
			{
				if( TheoraDecode_SetupImage( Movie->TheoraHandle, 1, 0 ) == 0 )
				{
					return ;
				}
			}
			else
			{
				if( TheoraDecode_SetupImage( Movie->TheoraHandle, 0, 1 ) == 0 )
				{
					return ;
				}
			}
		}

		// Theora を使用していて、且つ YUY2 フォーマットを使用しているかで処理を分岐
		if( Movie->TheoraFlag && Movie->RightAlpha == FALSE && TheoraDecode_GetYUVImage( Movie->TheoraHandle ) != NULL )
		{
			Graphics_Hardware_UpdateGraphMovie_TheoraYUV_PF( Movie, Image ) ;
		}
		else
#endif
		{
			// ムービーのイメージ情報を作成する
			MovieImage = *Movie->UseNowImage ;
			
			// 右側半分がアルファ情報の場合はアルファ情報を転送する準備をする
			if( Movie->RightAlpha )
			{
				MovieImage.Width /= 2;
				AlphaBaseImage = MovieImage;
				AlphaBaseImage.GraphData = (BYTE *)MovieImage.GraphData + MovieImage.Width * MovieImage.ColorData.PixelByte ;
			}

			// 転送元矩形をセット
			SETRECT( SrcRect, Image->UseOrigX, Image->UseOrigY, Image->UseOrigX + Image->Width, Image->UseOrigY + Image->Height ) ;

			// 分割されているテクスチャーに貼り付ける
			Graphics_Image_BltBmpOrBaseImageToGraph3(
				&SrcRect,
				0,
				0,
				( int )GrHandle,
				&MovieImage,
				Movie->RightAlpha ? &AlphaBaseImage : NULL,
				FALSE,
				FALSE,
				FALSE ) ;
		}
	}
	else
	{
		// テクスチャでは無い場合
		MEMIMG *MemImg ;

		MemImg = &Image->Soft.MemImg ;

#ifndef DX_NON_OGGTHEORA
		// Ogg Theora を使用している場合はここで更新
		if( Movie->TheoraFlag )
		{
			if( TheoraDecode_SetupImage( Movie->TheoraHandle, 1, 0 ) == 0 )
				return ;
		}
#endif

		// ピクセルフォーマットが違うか、右側半分をアルファ情報として扱う場合はカラーマッチング転送を行う
		if( Movie->SurfaceMode == DX_MOVIESURFACE_FULLCOLOR ||
			MemImg->Base->ColorDataP->ColorBitDepth != Movie->UseNowImage->ColorData.ColorBitDepth ||
			Movie->RightAlpha == 1 )
		{
			// 右半分がアルファ情報ではない場合は簡易転送
			if( Movie->RightAlpha == 0 )
			{
				BltBaseImageToMemImg(
					Movie->UseNowImage,
					NULL,
					MemImg,
					0,
					0,
					Image->Width,
					Image->Height,
					0,
					0,
					FALSE ) ;
			}
			else
			{
				// 右半分がアルファ情報の場合は通常転送
				BASEIMAGE MovieImage, AlphaBaseImage;
				RECT SrcRect ;

				SETRECT( SrcRect, 0, 0, Image->Width, Image->Height ) ;
				MovieImage.ColorData	= Movie->UseNowImage->ColorData ;
				MovieImage.GraphData	= Movie->UseNowImage->GraphData ;
				MovieImage.Pitch		= Movie->UseNowImage->Pitch ;
				MovieImage.Width		= Movie->UseNowImage->Width / 2;
				MovieImage.Height		= Movie->UseNowImage->Height ;

				AlphaBaseImage = MovieImage;
				AlphaBaseImage.GraphData = (BYTE *)MovieImage.GraphData + MovieImage.Width * MovieImage.ColorData.PixelByte ;

				// グラフィックの転送
				BltBaseImageToMemImg(
					&MovieImage,
					&AlphaBaseImage,
					MemImg,
					0,
					0,
					Image->Width,
					Image->Height,
					0,
					0,
					FALSE ) ;
			}
		}
		else
		{
			// 高速転送処理プログラム
			void *DestP, *SrcP ;
			int SrcAddPitch, DestAddPitch ;
			int DwMoveSetNum, NokoriMoveSetNum ;
			int MoveLineByte, ColorBitDepth ;
			int PixelByte ;
			int BltHeight ;

			// 転送するライン数をセット
			BltHeight = Movie->Height ;

			// 色ビット数をセット
			ColorBitDepth = NS_GetColorBitDepth() ;

			// １ドット当たりのバイト数取得
			PixelByte = ColorBitDepth / 8 ;

			// １ライン当たりの転送サイズをセット
			MoveLineByte = Movie->Width * PixelByte ;

			// 転送元、先アドレスセット
			DestP = MemImg->UseImage ;
			SrcP = Movie->UseNowImage->GraphData ;

			// １ライン転送後、次の転送元アドレスまでのバイト数計算
			DestAddPitch = ( int )( MemImg->Base->Pitch - MoveLineByte ) ;
			SrcAddPitch  = Movie->UseNowImage->Pitch - MoveLineByte ;

			// ４バイト転送何回、その後あまる転送分が何ピクセル分あるか算出
			DwMoveSetNum = MoveLineByte / 4 ;
			NokoriMoveSetNum = ( MoveLineByte - DwMoveSetNum * 4 ) / PixelByte ;

			// 転送処理
#ifndef DX_NON_INLINE_ASM
			_asm{
				PUSHF
				CLD
				MOV		EDI, DestP
				MOV		ESI, SrcP
				MOV		EAX, DestAddPitch
				MOV		EBX, SrcAddPitch
				MOV		EDX, BltHeight
				CMP		ColorBitDepth, 8
				JZ		CB8
				CMP		ColorBitDepth, 16
				JZ		CB16


			CB32:
				MOV		ECX, DwMoveSetNum
				REP		MOVSD
				ADD		EDI, DestAddPitch
				ADD		ESI, SrcAddPitch
				DEC		EDX
				JNZ		CB32
				JMP		END_


			CB16:
				MOV		ECX, DwMoveSetNum
				TEST	ECX, 0xffffffff
				JZ		B161
				REP		MOVSD
			B161:
				MOV		ECX, NokoriMoveSetNum
				TEST	ECX, 0xffffffff
				JZ		B162
				REP		MOVSW
			B162:
				ADD		EDI, DestAddPitch
				ADD		ESI, SrcAddPitch
				DEC		EDX
				JNZ		CB16
				JMP		END_


			CB8:
				MOV		ECX, DwMoveSetNum
				TEST	ECX, 0xffffffff
				JZ		B81
				REP		MOVSD
			B81:
				MOV		ECX, NokoriMoveSetNum
				TEST	ECX, 0xffffffff
				JZ		B82
				REP		MOVSB
			B82:
				ADD		EDI, DestAddPitch
				ADD		ESI, SrcAddPitch
				DEC		EDX
				JNZ		CB8
				JMP		END_

			END_:
				POPF
			}
#else
			int i, j ;
			BYTE *DestBP, *SrcBP ;

			DestBP = ( BYTE * )DestP ;
			SrcBP = ( BYTE * )SrcP ;
			switch( ColorBitDepth )
			{
			case 8 :
				for( i = BltHeight ; i > 0 ; i -- )
				{
					for( j = DwMoveSetNum ; j > 0 ; j --, DestBP += 4, SrcBP += 4 )
					{
						*( ( DWORD * )DestBP ) = *( ( DWORD * )SrcBP ) ;
					}
					for( j = NokoriMoveSetNum ; j > 0 ; j --, DestBP ++, SrcBP ++ )
					{
						*DestBP = *SrcBP ;
					}
					DestBP += DestAddPitch ;
					SrcBP += SrcAddPitch ;
				}
				break ;

			case 16 :
				for( i = BltHeight ; i > 0 ; i -- )
				{
					for( j = DwMoveSetNum ; j > 0 ; j --, DestBP += 4, SrcBP += 4 )
					{
						*( ( DWORD * )DestBP ) = *( ( DWORD * )SrcBP ) ;
					}
					for( j = NokoriMoveSetNum ; j > 0 ; j --, DestBP += 2, SrcBP += 2 )
					{
						*( ( WORD * )DestBP ) = *( ( WORD * )SrcBP ) ;
					}
					DestBP += DestAddPitch ;
					SrcBP += SrcAddPitch ;
				}
				break ;

			case 32 :
				for( i = BltHeight ; i > 0 ; i -- )
				{
					for( j = DwMoveSetNum ; j > 0 ; j --, DestBP += 4, SrcBP += 4 )
					{
						*( ( DWORD * )DestBP ) = *( ( DWORD * )SrcBP ) ;
					}
					DestBP += DestAddPitch ;
					SrcBP += SrcAddPitch ;
				}
				break ;
			}
#endif
		}
	}
}

#endif

// BltBmpOrGraphImageToGraph の内部関数
extern int Graphics_Image_BltBmpOrGraphImageToGraphBase(
	const BASEIMAGE		*RgbBaseImage,
	const BASEIMAGE		*AlphaBaseImage,
	      int			CopyPointX,
	      int			CopyPointY,
	      int			GrHandle,
	      int			UseTransColorConvAlpha,
	      int			ASyncThread
)
{
	IMAGEDATA		*Image ;
	IMAGEDATA_ORIG	*Orig ;
	RECT			SrcRect ;

	if( GSYS.InitializeFlag == FALSE )
	{
		return -1 ;
	}

	// エラー判定
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) )
			return -1 ;
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
	}
	Orig = Image->Orig ;

	// 転送元矩形の準備
	SrcRect.left   = CopyPointX ;
	SrcRect.top    = CopyPointY ;
	SrcRect.right  = CopyPointX + Image->Width ;
	SrcRect.bottom = CopyPointY + Image->Height ;

	// 使用する領域を保存
	Image->UseBaseX = SrcRect.left ;
	Image->UseBaseY = SrcRect.top ; 

	// 画像の転送
	return Graphics_Image_BltBmpOrBaseImageToGraph3(
				&SrcRect,
				0,
				0,
				GrHandle,
				RgbBaseImage,
				AlphaBaseImage,
				Orig->FormatDesc.BlendGraphFlag,
				UseTransColorConvAlpha,
				FALSE,
				ASyncThread
			) ;
}

// BltBmpOrGraphImageToGraph2 の内部関数
extern int Graphics_Image_BltBmpOrGraphImageToGraph2Base(
	const BASEIMAGE		*RgbBaseImage,
	const BASEIMAGE		*AlphaBaseImage,
	const RECT			*SrcRect,
	      int			DestX,
	      int			DestY,
	      int			GrHandle,
	      int			UseTransColorConvAlpha,
	      int			ASyncThread
)
{
	IMAGEDATA		*Image ;
	IMAGEDATA_ORIG *Orig ;

	if( GSYS.InitializeFlag == FALSE )
	{
		return -1 ;
	}

	// エラー判定
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) )
			return -1 ;
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
	}
	Orig = Image->Orig ;

	// 使用する領域を保存
	Image->UseBaseX = SrcRect->left - DestX ;
	Image->UseBaseY = SrcRect->top  - DestY ; 

	// 画像の転送
	return Graphics_Image_BltBmpOrBaseImageToGraph3(
				SrcRect,
				DestX,
				DestY,
				GrHandle,
				RgbBaseImage,
				AlphaBaseImage, 
				Orig->FormatDesc.BlendGraphFlag,
				UseTransColorConvAlpha,
				FALSE,
				ASyncThread
			) ;
}

// BltBmpOrGraphImageToDivGraph の内部関数
extern int Graphics_Image_BltBmpOrGraphImageToDivGraphBase(
	const BASEIMAGE	*RgbBaseImage,
	const BASEIMAGE	*AlphaBaseImage,
	      int		AllNum,
	      int		XNum,
	      int		/*YNum*/,
	      int		Width,
	      int		Height,
	const int		*GrHandle,
	      int		ReverseFlag,
	      int		UseTransColorConvAlpha,
	      int		ASyncThread
)
{
	IMAGEDATA		*Image ;
	IMAGEDATA		*DivImage ;
	IMAGEDATA_ORIG	*Orig ;
	RECT			Rect ;
	int				i ;
	int				Count ;
	int				x ;
	int				y ;

	if( GSYS.InitializeFlag == FALSE )
	{
		return -1 ;
	}

	// エラー判定
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle[ 0 ], Image ) )
			return -1 ;
	}
	else
	{
		if( GRAPHCHK( GrHandle[ 0 ], Image ) )
			return -1 ;
	}
	Orig = Image->Orig ;

	// 最初の画像にファイル情報をセット
	Image->UseBaseX = 0 ;
	Image->UseBaseY = 0 ;

	// ファイルに情報をセットしながらすべての画像が同じ
	// オリジナル画像を使用しているか調べる
	Count = 1 ;
	x     = 0 ;
	y     = 0 ;
	for( i = 1 ; i < AllNum ; i ++ )
	{
		// 画像情報のアドレスを取得
		if( ASyncThread )
		{
			if( GRAPHCHK_ASYNC( GrHandle[ i ], DivImage ) )
				return -1 ;
		}
		else
		{
			if( GRAPHCHK( GrHandle[ i ], DivImage ) )
				return -1 ;
		}

		// 同じオリジナル画像を使用しているかどうか調べる
		if( DivImage->Orig == Orig ) Count ++ ;

		// ファイル情報をセット
		DivImage->UseBaseX = ReverseFlag ? ( ( XNum - 1 ) - x ) * Width : x * Width ;
		DivImage->UseBaseY = y * Height ;
		x ++ ;
		if( x == XNum )
		{
			x = 0 ;
			y ++ ;
		}
	}

	// すべてのグラフィックハンドルが同じオリジナル画像を
	// 使用しているかどうかで処理を分岐
	if( AllNum == Count )
	{
		// 同じオリジナル画像を使用している場合

		// オリジナル画像を最初のグラフィックハンドルに転送
		Rect.left   = 0 ;
		Rect.right  = Image->Orig->Width ;
		Rect.top    = 0 ;
		Rect.bottom = Image->Orig->Height ;
		Graphics_Image_BltBmpOrBaseImageToGraph3(
			&Rect,
			0, 0,
			GrHandle[ 0 ],
			RgbBaseImage, AlphaBaseImage,
			Orig->FormatDesc.BlendGraphFlag,
			UseTransColorConvAlpha,
			TRUE,
			ASyncThread
		) ;
	}
	else
	{
		// 同じオリジナル画像を使用していない場合

		// すべてのグラフィックハンドルに個別に転送する
		for( i = 0 ; i < AllNum ; i ++ )
		{
			// 画像情報のアドレスを取得
			if( ASyncThread )
			{
				if( GRAPHCHK_ASYNC( GrHandle[ i ], DivImage ) )
					return -1 ;
			}
			else
			{
				if( GRAPHCHK( GrHandle[ i ], DivImage ) )
					return -1 ;
			}

			// 転送元矩形のセット
			Rect.left   = DivImage->UseBaseX ;
			Rect.top    = DivImage->UseBaseY ;
			Rect.right  = Rect.left + DivImage->Width ;
			Rect.bottom = Rect.top  + DivImage->Height ;

			// 画像を転送
			Graphics_Image_BltBmpOrBaseImageToGraph3(
				&Rect,
				0, 0,
				GrHandle[ i ],
				RgbBaseImage, AlphaBaseImage,
				Orig->FormatDesc.BlendGraphFlag,
				UseTransColorConvAlpha,
				FALSE,
				ASyncThread
			) ;
		}
	}

	// 終了
	return 0 ;
}

// グラフィックハンドルに画像データを転送するための関数
extern int Graphics_Image_BltBmpOrBaseImageToGraph3(
	const RECT		*SrcRect,
	      int		DestX,
	      int		DestY,
	      int		GrHandle,
	const BASEIMAGE	*RgbBaseImage,
	const BASEIMAGE	*AlphaBaseImage,
	      int		RedIsAlphaFlag,
	      int		UseTransColorConvAlpha,
	      int		TargetOrig,
	      int		ASyncThread
)
{
	IMAGEDATA		*Image ;
	IMAGEDATA_ORIG	*Orig ;
	int				SrcWidth ;
	int				SrcHeight ;
	BASEIMAGE		TempBaseRGB ;
	BASEIMAGE		TempBaseAlpha = { 0 } ;
	int				RequiredReleaseBaseRGB = FALSE ;
	int				RequiredReleaseBaseAlpha = FALSE ;

	if( GSYS.InitializeFlag == FALSE )
	{
		return -1 ;
	}

	// エラー判定
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) )
			return -1 ;
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return -1 ;
	}
	Orig = Image->Orig ;

	// 転送幅を計算しておく
	SrcWidth  = SrcRect->right  - SrcRect->left ;
	SrcHeight = SrcRect->bottom - SrcRect->top ;
	if( SrcWidth <= 0 || SrcHeight <= 0 )
	{
		return 0 ;
	}

	// 転送元の情報をセット
	TempBaseAlpha.GraphData  = NULL ;
	RequiredReleaseBaseRGB   = FALSE ;
	RequiredReleaseBaseAlpha = FALSE ;

	TempBaseRGB.GraphData      = RgbBaseImage->GraphData ;
	TempBaseRGB.Width          = RgbBaseImage->Width ;
	TempBaseRGB.Height         = RgbBaseImage->Height ;
	TempBaseRGB.Pitch          = RgbBaseImage->Pitch ;
	TempBaseRGB.ColorData      = RgbBaseImage->ColorData ;
	TempBaseRGB.MipMapCount    = RgbBaseImage->MipMapCount ;
	TempBaseRGB.GraphDataCount = RgbBaseImage->GraphDataCount ;

	if( AlphaBaseImage )
	{
		TempBaseAlpha.GraphData      = AlphaBaseImage->GraphData ;
		TempBaseAlpha.Width          = AlphaBaseImage->Width ;
		TempBaseAlpha.Height         = AlphaBaseImage->Height ;
		TempBaseAlpha.Pitch          = AlphaBaseImage->Pitch ;
		TempBaseAlpha.ColorData      = AlphaBaseImage->ColorData ;
		TempBaseAlpha.MipMapCount    = AlphaBaseImage->MipMapCount ;
		TempBaseAlpha.GraphDataCount = AlphaBaseImage->GraphDataCount ;
	}

	// 転送元がキューブマップで転送先がキューブマップではない場合はエラー
	if( ( TempBaseRGB.GraphDataCount          != 0 ) !=
		( Orig->FormatDesc.CubeMapTextureFlag != 0 ) )
	{
		return -1 ;
	}

	// BASEIMAGE が DXT で、転送先が DXT では無い場合はここで標準フォーマットに変換する
	{
		int RgbConvFlag   = FALSE ;
		int AlphaConvFlag = FALSE ;
		int Hr1 = 0 ;
		int Hr2 = 0 ;

		Graphics_CheckRequiredNormalImageConv_BaseImageFormat_PF(
			Orig,
			TempBaseRGB.ColorData.Format,
			&RgbConvFlag,
			TempBaseAlpha.GraphData != NULL ? TempBaseAlpha.ColorData.Format : -1,
			&AlphaConvFlag
		) ;

		if( RgbConvFlag == TRUE )
		{
			Hr1 = ConvertNormalFormatBaseImage( &TempBaseRGB ) ;
//			if( Hr1 == 0 )
//			{
//				RequiredReleaseBaseRGB = TRUE ;
//			}
		}

		if( AlphaConvFlag == TRUE )
		{
			Hr2 = ConvertNormalFormatBaseImage( &TempBaseAlpha ) ;
//			if( Hr2 == 0 )
//			{
//				RequiredReleaseBaseAlpha = TRUE ;
//			}
		}

		if( Hr1 < 0 || Hr2 < 0 )
		{
			DXST_ERRORLOG_ADDUTF16LE( "\x19\x6a\x96\x6e\xa4\x30\xe1\x30\xfc\x30\xb8\x30\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"標準イメージ用のメモリ確保に失敗しました\n" @*/ ) ;
			goto ERR ;
		}
	}

	// テクスチャかどうかで処理を分岐
	if( Orig->FormatDesc.TextureFlag )
	{
		// テクスチャの場合機種依存関数を呼ぶ
		if( Graphics_Hardware_BltBmpOrBaseImageToGraph3_PF(
			SrcRect,
			DestX,
			DestY,
			GrHandle,
			&TempBaseRGB,
			&TempBaseAlpha,
			RedIsAlphaFlag,
			UseTransColorConvAlpha,
			TargetOrig,
			ASyncThread ) < 0 )
		{
			goto ERR ;
		}
	}
	else
	{
		// テクスチャではない場合

		// MEMIMG の転送処理関数を使うための準備を行う

		// オリジナル画像が対象かどうかで処理を分岐
		if( TargetOrig == TRUE )
		{
			// オリジナル画像が対象の場合

			// サイズの補正
			if( DestX + SrcWidth  > ( int )Orig->Soft.MemImg.Width  )
			{
				SrcWidth  = ( int )( Orig->Soft.MemImg.Width  - DestX ) ;
			}

			if( DestY + SrcHeight > ( int )Orig->Soft.MemImg.Height )
			{
				SrcHeight = ( int )( Orig->Soft.MemImg.Height - DestY ) ;
			}

			if( SrcWidth <= 0 || SrcHeight <= 0 )
			{
				goto ERR ;
			}

			// 画像の転送
			BltBaseImageToMemImg(
				&TempBaseRGB,
				TempBaseAlpha.GraphData == NULL ? NULL : &TempBaseAlpha,
				&Orig->Soft.MemImg,
				SrcRect->left,
				SrcRect->top,
				SrcWidth,
				SrcHeight,
				DestX,
				DestY,
				UseTransColorConvAlpha
			) ;
		}
		else
		{
			// オリジナル画像が対象ではない場合

			// サイズの補正
			if( DestX + SrcWidth  > Image->Width  )
			{
				SrcWidth  = Image->Width  - DestX ;
			}

			if( DestY + SrcHeight > Image->Height )
			{
				SrcHeight = Image->Height - DestY ;
			}

			if( SrcWidth <= 0 || SrcHeight <= 0 ) 
			{
				goto ERR ;
			}

			// 画像の転送
			BltBaseImageToMemImg(
				&TempBaseRGB,
				TempBaseAlpha.GraphData == NULL ? NULL : &TempBaseAlpha,
				&Orig->Soft.MemImg,
				SrcRect->left,
				SrcRect->top,
				SrcWidth,
				SrcHeight,
				Image->UseOrigX + DestX,
				Image->UseOrigY + DestY,
				UseTransColorConvAlpha
			) ;
		}

		// パレット画像の場合は透過色コードを調整
		if( Orig->FormatDesc.UsePaletteFlag == TRUE && Orig->Soft.MemImg.Base->Palette )
		{
			int   i ;
			DWORD TransCode ;

			// 透過色の取得
			TransCode = NS_GetColor3( 
					GetMemImgColorData( Orig->FormatDesc.ColorBitDepth == 16 ? 0 : 1, Orig->FormatDesc.AlphaChFlag, FALSE ),
					( int )( ( GSYS.CreateImage.TransColor >> 16 ) & 0xff ),
					( int )( ( GSYS.CreateImage.TransColor >> 8  ) & 0xff ),
					( int )( ( GSYS.CreateImage.TransColor >> 0  ) & 0xff ),
					0xff ) ;

			// 指定のカラーがパレットの中にあったらそれを透過色にする
			if( Orig->Soft.MemImg.Base->ColorType == 1 )
			{
				TransCode &= 0xffffff;
				for( i = 0 ; i < 256 ; i ++ )
				{
					if( ( Orig->Soft.MemImg.Base->Palette[i] & 0xffffff ) == TransCode )
					{
						break ;
					}
				}
			}
			else
			{
				TransCode &= 0xffff;
				for( i = 0 ; i < 256 ; i ++ )
				{
					if( ((WORD *)Orig->Soft.MemImg.Base->Palette)[i] == TransCode )
					{
						break ;
					}
				}
			}

			if( i == 256 )
			{
				i = 0 ;
			}

			Orig->Soft.MemImg.Base->TransColor = ( unsigned int )i ;
		}
	}

	if( RequiredReleaseBaseRGB )
	{
		ReleaseBaseImage( &TempBaseRGB ) ;
	}

	if( RequiredReleaseBaseAlpha )
	{
		ReleaseBaseImage( &TempBaseAlpha ) ;
	}

	// 終了
	return 0 ;

	// エラー終了
ERR :

	if( RequiredReleaseBaseRGB )
	{
		ReleaseBaseImage( &TempBaseRGB ) ;
	}

	if( RequiredReleaseBaseAlpha )
	{
		ReleaseBaseImage( &TempBaseAlpha ) ;
	}

	return -1 ;
}

// 指定のオリジナル画像情報に転送する矩形情報を作成する
// 戻り値  -1:範囲外   0:正常終了
extern	int		Graphics_Image_BltBmpOrBaseImageToGraph3_Make_OrigTex_MoveRect(
	const IMAGEDATA_ORIG_HARD_TEX	*OrigTex,
	const RECT						*SrcRect,
		  int						SrcWidth,
		  int						SrcHeight,
	      int						DestX,
	      int						DestY,
	      RECT						*DestRect,
	      RECT						*MoveRect
)
{
	// 範囲外だったら何もしない
	if( OrigTex->OrigPosX >= DestX + SrcWidth  ||
		OrigTex->OrigPosY >= DestY + SrcHeight ||
		DestX >= OrigTex->OrigPosX + OrigTex->UseWidth ||
		DestY >= OrigTex->OrigPosY + OrigTex->UseHeight )
	{
		return -1 ;
	}

	// ロックする転送範囲と転送する範囲の確定
	if( OrigTex->OrigPosX > DestX )
	{
		DestRect->left = 0 ;
		MoveRect->left = OrigTex->OrigPosX - DestX ;
	}
	else
	{
		DestRect->left = DestX - OrigTex->OrigPosX ;
		MoveRect->left = 0 ;
	}

	if( OrigTex->OrigPosX + OrigTex->UseWidth < DestX + SrcWidth )
	{
		DestRect->right = OrigTex->UseWidth ;
		MoveRect->right = ( OrigTex->OrigPosX + OrigTex->UseWidth ) - DestX ;
	}
	else
	{
		DestRect->right = ( DestX + SrcWidth ) - OrigTex->OrigPosX ;
		MoveRect->right = SrcWidth ;
	}

	if( OrigTex->OrigPosY > DestY )
	{
		DestRect->top = 0 ;
		MoveRect->top = OrigTex->OrigPosY - DestY ;
	}
	else
	{
		DestRect->top = DestY - OrigTex->OrigPosY ;
		MoveRect->top = 0 ;
	}

	if( OrigTex->OrigPosY + OrigTex->UseHeight < DestY + SrcHeight )
	{
		DestRect->bottom = OrigTex->UseHeight ;
		MoveRect->bottom = ( OrigTex->OrigPosY + OrigTex->UseHeight ) - DestY ;
	}
	else
	{
		DestRect->bottom = ( DestY + SrcHeight ) - OrigTex->OrigPosY ;
		MoveRect->bottom = SrcHeight ;
	}

	// 元画像の転送領域値を加算
	MoveRect->left   += SrcRect->left ;
	MoveRect->top    += SrcRect->top ;
	MoveRect->right  += SrcRect->left ;
	MoveRect->bottom += SrcRect->top ;

	// 正常終了
	return 0 ;
}

// 指定の描画用画像情報に転送する矩形情報を作成する
// 戻り値  -1:範囲外   0:正常終了
extern	int		Graphics_Image_BltBmpOrBaseImageToGraph3_Make_DrawTex_MoveRect(
	const IMAGEDATA_HARD_DRAW		*DrawTex,
	const RECT						*SrcRect,
		  int						SrcWidth,
		  int						SrcHeight,
	      int						DestX,
	      int						DestY,
	      RECT						*DestRect,
	      RECT						*MoveRect
)
{
	// 範囲外だったら何もしない
	if( DrawTex->DrawPosX >= DestX + SrcWidth ||
		DrawTex->DrawPosY >= DestY + SrcHeight ||
		DestX >= DrawTex->DrawPosX + DrawTex->Width ||
		DestY >= DrawTex->DrawPosY + DrawTex->Height )
	{
		return -1 ;
	}

	// ロックする転送範囲と転送する範囲の確定
	if( DrawTex->DrawPosX > DestX )
	{
		DestRect->left = 0 ;
		MoveRect->left = DrawTex->DrawPosX - DestX ;
	}
	else
	{
		DestRect->left = DestX - DrawTex->DrawPosX ;
		MoveRect->left = 0 ;
	}

	if( DrawTex->DrawPosX + DrawTex->Width < DestX + SrcWidth )
	{
		DestRect->right = DrawTex->Width ;
		MoveRect->right = ( DrawTex->DrawPosX + DrawTex->Width ) - DestX ;
	}
	else
	{
		DestRect->right = ( DestX + SrcWidth ) - DrawTex->DrawPosX ;
		MoveRect->right = SrcWidth ;
	}

	if( DrawTex->DrawPosY > DestY )
	{
		DestRect->top = 0 ;
		MoveRect->top = DrawTex->DrawPosY - DestY ;
	}
	else
	{
		DestRect->top = DestY - DrawTex->DrawPosY ;
		MoveRect->top = 0 ;
	}

	if( DrawTex->DrawPosY + DrawTex->Height < DestY + SrcHeight )
	{
		DestRect->bottom = DrawTex->Height ;
		MoveRect->bottom = ( DrawTex->DrawPosY + DrawTex->Height ) - DestY ;
	}
	else
	{
		DestRect->bottom = ( DestY + SrcHeight ) - DrawTex->DrawPosY ;
		MoveRect->bottom = SrcHeight ;
	}

	// テクスチャの指定領域値を加算
	DestRect->left   += DrawTex->UsePosX ;
	DestRect->top    += DrawTex->UsePosY ;
	DestRect->right  += DrawTex->UsePosX ;
	DestRect->bottom += DrawTex->UsePosY ;

	// 元画像の転送領域値を加算
	MoveRect->left   += SrcRect->left ;
	MoveRect->top    += SrcRect->top ;
	MoveRect->right  += SrcRect->left ;
	MoveRect->bottom += SrcRect->top ;

	// 正常終了
	return 0 ;
}

// 描画可能画像やバックバッファから指定領域のグラフィックを取得する
extern	int Graphics_Image_GetDrawScreenGraphBase( int TargetScreen, int TargetScreenSurface, int x1, int y1, int x2, int y2, int destX, int destY, int GrHandle )
{
	IMAGEDATA *Image, *TargImage ;
//	IMAGEDATA_HARD_DRAW *DrawTex ;
	int Width, Height ;
//	int i ;
	int TargetScreenWidth ;
	int TargetScreenHeight ;
	RECT Rect ;
	RECT DestBaseRect ;
//	HRESULT hr ;

	// エラー判定
	if( GRAPHCHK( GrHandle, Image ) )
		return -1 ;

	// 描画先の画像データアドレスを取得する
	if( GRAPHCHKFULL( TargetScreen, TargImage ) )
	{
		TargImage = NULL ;
		TargetScreenWidth  = GSYS.DrawSetting.DrawSizeX ;
		TargetScreenHeight = GSYS.DrawSetting.DrawSizeY ;
	}
	else
	{
		TargetScreenWidth  = TargImage->Width ;
		TargetScreenHeight = TargImage->Height ;
	}

	// 矩形のデータが不正だった場合エラー
	if( x1 < 0 || x1 >= x2 ||
		y1 < 0 || y1 >= y2 ||
		x2 > TargetScreenWidth ||
		y2 > TargetScreenHeight ) return -1 ;

	// 取り込み先が画像をはみ出ていたらエラー
	Width  = x2 - x1 ;
	Height = y2 - y1 ;
	if( Image->Width < Width + destX || Image->Height < Height + destY ) return -1 ;
	DestBaseRect.left   = destX ;
	DestBaseRect.top    = destY ;
	DestBaseRect.right  = destX + Width ;
	DestBaseRect.bottom = destY + Height ;

	// テクスチャかどうかで処理を分岐
	if( Image->Orig->FormatDesc.TextureFlag )
	{
		// テクスチャの場合
		if( Graphics_Hardware_GetDrawScreenGraphBase_PF( Image, TargImage, TargetScreen, TargetScreenSurface, TargetScreenWidth, TargetScreenHeight, x1, y1, x2, y2, destX, destY ) < 0 )
		{
			return -1 ;
		}
	}
	else
	{
		// ソフトウエアレンダリングの場合

		// 描画対象から転送
		Rect.left   = x1 ;
		Rect.top    = y1 ;
		Rect.right  = x2 ;
		Rect.bottom = y2 ;
		BltMemImg( &Image->Soft.MemImg, TargImage ? &TargImage->Soft.MemImg : &GSYS.SoftRender.MainBufferMemImg, &Rect, destX, destY ) ;
	}

	// 終了
	return 0 ;
}

// グラフィックのデータをインデックス値から取り出す
extern IMAGEDATA *Graphics_Image_GetData( int GrHandle, int ASyncThread )
{
	IMAGEDATA *Image ;

	// エラー判定
	if( ASyncThread )
	{
		if( GRAPHCHK_ASYNC( GrHandle, Image ) )
			return NULL ;
	}
	else
	{
		if( GRAPHCHK( GrHandle, Image ) )
			return NULL ;
	}

	return Image ;
}

// 真っ白のテクスチャのハンドルを取得する
extern int Graphics_Image_GetWhiteTexHandle( void )
{
	if( GSYS.Resource.WhiteTexHandle <= 0 )
	{
		LOADGRAPH_GPARAM GParam ;

		Graphics_Image_InitLoadGraphGParam( &GParam ) ;
		Graphics_Image_InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam.CreateGraphGParam.InitHandleGParam, 16, TRUE, FALSE ) ;
		GParam.CreateGraphGParam.InitHandleGParam.TransColor = ( BYTE )255 << 16 | ( BYTE )0 << 8 | ( BYTE )255 ;
		GSYS.Resource.WhiteTexHandle = Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, FALSE, GSYS.Resource.WhiteTexHandle, WhiteTga8x8TextureFileImage, 84 ) ;
		NS_SetDeleteHandleFlag( GSYS.Resource.WhiteTexHandle, &GSYS.Resource.WhiteTexHandle ) ;
		NS_SetDeviceLostDeleteGraphFlag( GSYS.Resource.WhiteTexHandle, TRUE ) ;
	}

	return GSYS.Resource.WhiteTexHandle ;
}


// MakeGraph の実処理関数
static int Graphics_Image_MakeGraph_Static(
	SETUP_GRAPHHANDLE_GPARAM *GParam,
	int GrHandle,
	int SizeX,
	int SizeY,
	int NotUse3DFlag,
	int UsePaletteFlag,
	int PaletteBitDepth,
	int AlphaValidFlag,
	int ASyncThread
)
{
	// ハンドルの初期化
	if( Graphics_Image_SetupHandle_UseGParam(
			GParam,
			GrHandle,
			SizeX,
			SizeY,
			!NotUse3DFlag,
			AlphaValidFlag,
			UsePaletteFlag,
			PaletteBitDepth,
			DX_BASEIMAGE_FORMAT_NORMAL,
			-1,
			ASyncThread
		) == -1 )
		return -1 ;

	// 終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// MakeGraph の非同期読み込みスレッドから呼ばれる関数
static void Graphics_Image_MakeGraph_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	SETUP_GRAPHHANDLE_GPARAM *GParam ;
	int GrHandle ;
	int SizeX ;
	int SizeY ;
	int NotUse3DFlag ;
	int UsePaletteFlag ;
	int PaletteBitDepth ;
	int AlphaValidFlag ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( SETUP_GRAPHHANDLE_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	GrHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeX = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeY = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	NotUse3DFlag = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	UsePaletteFlag = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	PaletteBitDepth = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	AlphaValidFlag = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = Graphics_Image_MakeGraph_Static( GParam, GrHandle, SizeX, SizeY, NotUse3DFlag, UsePaletteFlag, PaletteBitDepth, AlphaValidFlag, TRUE ) ;

	DecASyncLoadCount( GrHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteGraph( GrHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// 空のグラフィックハンドルを作成する関数
extern int Graphics_Image_MakeGraph_UseGParam(
	SETUP_GRAPHHANDLE_GPARAM *GParam,
	int SizeX,
	int SizeY,
	int NotUse3DFlag,
	int UsePaletteFlag,
	int PaletteBitDepth,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	int GrHandle ;

	CheckActiveState() ;

	// ハンドルの作成
	GrHandle = Graphics_Image_AddHandle( ASyncThread ) ;
	if( GrHandle == -1 )
	{
		return -1 ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, GrHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeX ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeY ) ;
		AddASyncLoadParamInt( NULL, &Addr, NotUse3DFlag ) ;
		AddASyncLoadParamInt( NULL, &Addr, UsePaletteFlag ) ;
		AddASyncLoadParamInt( NULL, &Addr, PaletteBitDepth ) ;
		AddASyncLoadParamInt( NULL, &Addr, GParam->AlphaChannelImageCreateFlag ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = Graphics_Image_MakeGraph_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, GrHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeX ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeY ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, NotUse3DFlag ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, UsePaletteFlag ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, PaletteBitDepth ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, GParam->AlphaChannelImageCreateFlag ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( GrHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( Graphics_Image_MakeGraph_Static( GParam, GrHandle, SizeX, SizeY, NotUse3DFlag, UsePaletteFlag, PaletteBitDepth, GParam->AlphaChannelImageCreateFlag, ASyncThread ) < 0 )
		{
			goto ERR ;
		}
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( GrHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	// 終了
	return GrHandle ;

ERR :
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( GrHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	NS_DeleteGraph( GrHandle ) ;

	return -1 ;
}

// CreateGraph の実処理関数
static int Graphics_Image_CreateGraph_Static(
	LOADGRAPH_PARAM *Param,
	int ASyncThread
)
{
	char    FullPath_UTF16LE[ 2048 ] ;
	int Result = 0 ;
	CREATEGRAPH_LOADBASEIMAGE_PARAM LParam ;

	// 画像データの読み込み
	Graphics_Image_CreateGraph_LoadBaseImage( Param, &LParam ) ;

	if( LParam.LoadHr == -1 ) 
	{
		// 再読み込みではない場合でファイルからの読み込みの場合はムービーの可能性を見る
		if( Param->ReCreateFlag == FALSE && Param->FileName != NULL )
		{
#ifndef DX_NON_MOVIE
			// ファイルがあるか確認
			{
				DWORD_PTR fp ;

				fp = FOPEN( Param->FileName ) ;
				if( fp == 0 )
				{
					wchar_t FullPath_WCHAR_T[ 1024 ] ;

					ConvertFullPathW_( Param->FileName, FullPath_WCHAR_T ) ;
					ConvString( ( const char * )FullPath_WCHAR_T, WCHAR_T_CHARCODEFORMAT, FullPath_UTF16LE, DX_CHARCODEFORMAT_UTF16LE ) ;

					DXST_ERRORLOGFMT_ADDUTF16LE(( "\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\x20\x00\x25\x00\x73\x00\x20\x00\x4c\x30\x42\x30\x8a\x30\x7e\x30\x5b\x30\x93\x30\x00"/*@ L"画像ファイル %s がありません" @*/, FullPath_UTF16LE )) ;
					return -1 ;
				}
				FCLOSE( fp ) ;
			}

			// 動画のオープンを試みる
			if( Graphics_Image_OpenMovie_UseGParam( &Param->GParam, Param->GrHandle, Param->FileName, Param->TextureFlag, Param->SurfaceMode, ASyncThread ) < 0 )
			{
				return -1 ;
			}

			// 正常終了
			return 0 ;
#else	// DX_NON_MOVIE
			{
				ConvString( ( const char * )Param->FileName, WCHAR_T_CHARCODEFORMAT, FullPath_UTF16LE, DX_CHARCODEFORMAT_UTF16LE ) ;
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\x20\x00\x25\x00\x73\x00\x20\x00\x6e\x30\xaa\x30\xfc\x30\xd7\x30\xf3\x30\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"画像ファイル %s のオープンに失敗しました" @*/, FullPath_UTF16LE )) ;
				return -1 ;
			}
#endif	// DX_NON_MOVIE
		}
		else
		{
			return -1 ;
		}
	}

	// グラフィックハンドルを作成する
	Result = Graphics_Image_CreateGraphFromGraphImageBase_UseGParam(
				&Param->GParam.CreateGraphGParam,
				Param->ReCreateFlag,
				Param->GrHandle,
				LParam.UseRgbBaseImage,
				LParam.UseAlphaBaseImage,
				Param->TextureFlag,
				ASyncThread
			) ;

	// ファイルパスをセットする
	if( Result == 0 )
	{
		Graphics_Image_SetBaseInfo_UseGParam(
			&Param->GParam.SetGraphBaseInfoGParam,
			Param->GrHandle,
			Param->FileName,
			NULL,
			NULL,
			NULL,
			Param->RgbMemImage,
			Param->RgbMemImageSize,
			Param->AlphaMemImage,
			Param->AlphaMemImageSize,
			LParam.UseRgbBaseImage,
			LParam.UseAlphaBaseImage,
			Param->ReverseFlag,
			-1,
			ASyncThread
		) ;
	}

	// 読み込んだグラフィックの後始末をする
	Graphics_Image_CreateGraph_TerminateBaseImage( Param, &LParam ) ;

	// 終了
	return Result ;
}

#ifndef DX_NON_ASYNCLOAD

// CreateGraph の非同期読み込みスレッドから呼ばれる関数
static void Graphics_Image_CreateGraph_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADGRAPH_PARAM *Param ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	Param = ( LOADGRAPH_PARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	if( Param->FileName != NULL )
	{
		Param->FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	}

	if( Param->RgbBaseImage != NULL )
	{
		Param->RgbBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	if( Param->AlphaBaseImage != NULL )
	{
		Param->AlphaBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	Result = Graphics_Image_CreateGraph_Static( Param, TRUE ) ;
	DecASyncLoadCount( Param->GrHandle ) ;
	if( Param->ReCreateFlag == FALSE )
	{
		if( Result < 0 )
		{
			NS_DeleteGraph( Param->GrHandle ) ;
		}
	}
}
#endif // DX_NON_ASYNCLOAD

// CreateGraph のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateGraph_UseGParam( 
	LOADGRAPH_PARAM *Param,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	if( ASyncThread == FALSE )
	{
		CheckActiveState() ;
	}

	if( Param->ReCreateFlag == FALSE )
	{
		Param->GrHandle = Graphics_Image_AddHandle( ASyncThread ) ;
		if( Param->GrHandle < 0 )
		{
			return -1 ;
		}
	}
	else
	{
#ifndef DX_NON_ASYNCLOAD
		if( NS_CheckHandleASyncLoad( Param->GrHandle ) > 0 )
		{
			return -1 ;
		}

		// 非同期読み込みカウントをインクリメント
		if( ASyncThread )
		{
			IncASyncLoadCount( Param->GrHandle, -1 ) ;
		}
#endif // DX_NON_ASYNCLOAD
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread == FALSE && ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		wchar_t FullPath[ 1024 ] ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, Param, sizeof( *Param ) ) ;
		if( Param->FileName != NULL )
		{
			ConvertFullPathW_( Param->FileName, FullPath ) ;
			AddASyncLoadParamString( NULL, &Addr, FullPath ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = Graphics_Image_CreateGraph_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, Param, sizeof( *Param ) ) ;
		if( Param->FileName != NULL )
		{
			AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( Param->GrHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( Graphics_Image_CreateGraph_Static( Param, ASyncThread ) < 0 )
		{
			goto ERR ;
		}
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( Param->GrHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	// 正常終了
	return Param->ReCreateFlag == FALSE ? Param->GrHandle : 0 ;

ERR :
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( Param->GrHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	if( Param->ReCreateFlag == FALSE )
	{
		NS_DeleteGraph( Param->GrHandle ) ;
		Param->GrHandle = -1 ;
	}

	return -1 ;
}


// Graphics_Image_CreateDivGraph の実処理関数
static int Graphics_Image_CreateDivGraph_Static(
	LOADGRAPH_PARAM *Param,
	int ASyncThread
)
{
	int Result ;
	int i ;
	CREATEGRAPH_LOADBASEIMAGE_PARAM LParam ;

	// 画像データの読み込み
	Graphics_Image_CreateGraph_LoadBaseImage( Param, &LParam ) ;
	if( LParam.LoadHr == -1 )
	{
		if( Param->FileName != NULL )
		{
			wchar_t FullPath_WCHAR_T[ 1024 ] ;
			char    FullPath_UTF16LE[ 2048 ] ;

			ConvertFullPathW_( Param->FileName, FullPath_WCHAR_T ) ;
			ConvString( ( const char * )FullPath_WCHAR_T, WCHAR_T_CHARCODEFORMAT, FullPath_UTF16LE, DX_CHARCODEFORMAT_UTF16LE ) ;

			DXST_ERRORLOGFMT_ADDUTF16LE(( "\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\x20\x00\x25\x00\x73\x00\x20\x00\x6e\x30\xed\x30\xfc\x30\xc9\x30\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"画像ファイル %s のロードに失敗しました" @*/, FullPath_UTF16LE )) ;
		}
		return -1 ;
	}

	// 分割グラフィックハンドルを作成する
	Result = Graphics_Image_CreateDivGraphFromGraphImageBase_UseGParam(
				&Param->GParam.CreateGraphGParam,
				Param->ReCreateFlag,
				Param->BaseHandle,
				LParam.UseRgbBaseImage,
				LParam.UseAlphaBaseImage,
				Param->AllNum,
				Param->XNum,
				Param->YNum,
				Param->SizeX,
				Param->SizeY,
				Param->HandleBuf,
				Param->TextureFlag,
				Param->ReverseFlag,
				ASyncThread
			) ;

	// ファイルパスをセットする
	if( Result == 0 )
	{
		Graphics_Image_SetBaseInfo_UseGParam(
			&Param->GParam.SetGraphBaseInfoGParam,
			Param->HandleBuf[ 0 ],
			Param->FileName,
			NULL,
			NULL,
			NULL,
			Param->RgbMemImage,
			Param->RgbMemImageSize,
			Param->AlphaMemImage,
			Param->AlphaMemImageSize,
			LParam.UseRgbBaseImage,
			LParam.UseAlphaBaseImage,
			Param->ReverseFlag,
			-1,
			ASyncThread
		) ;
		for( i = 1 ; i < Param->AllNum ; i ++ )
		{
			Graphics_Image_SetBaseInfo_UseGParam(
				&Param->GParam.SetGraphBaseInfoGParam,
				Param->HandleBuf[ i ],
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				0,
				NULL,
				0,
				NULL,
				NULL,
				FALSE,
				Param->HandleBuf[ 0 ],
				ASyncThread
			) ;
		}
	}

	// 読み込んだグラフィックの後始末をする
	Graphics_Image_CreateGraph_TerminateBaseImage( Param, &LParam ) ;

	// 終了
	return Result ;
}

#ifndef DX_NON_ASYNCLOAD

// Graphics_Image_CreateDivGraph の非同期読み込みスレッドから呼ばれる関数
static void Graphics_Image_CreateDivGraph_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADGRAPH_PARAM *Param ;
	int Addr ;
	int i ;
	int Result ;

	Addr = 0 ;
	Param = ( LOADGRAPH_PARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	Param->HandleBuf = ( int * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	if( Param->FileName != NULL )
	{
		Param->FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	}

	if( Param->RgbBaseImage != NULL )
	{
		Param->RgbBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	if( Param->AlphaBaseImage != NULL )
	{
		Param->AlphaBaseImage = ( BASEIMAGE * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	}

	Result = Graphics_Image_CreateDivGraph_Static( Param, TRUE ) ;

	DecASyncLoadCount( Param->BaseHandle ) ;
	for( i = 0 ; i < Param->AllNum ; i ++ )
		DecASyncLoadCount( Param->HandleBuf[ i ] ) ;

	if( Param->ReCreateFlag == FALSE )
	{
		NS_DeleteGraph( Param->BaseHandle ) ;
		if( Result < 0 )
		{
			for( i = 0 ; i < Param->AllNum ; i ++ )
				NS_DeleteGraph( Param->HandleBuf[ i ] ) ;
		}
	}
}
#endif // DX_NON_ASYNCLOAD

// 画像を分割してグラフィックハンドルを作成する関数
extern int Graphics_Image_CreateDivGraph_UseGParam( 
	LOADGRAPH_PARAM *Param,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	int i ;

	CheckActiveState() ;

	if( Param->AllNum == 0 )
		return -1 ;

	Param->BaseHandle = -1 ;
	if( Param->ReCreateFlag == FALSE )
	{
		// グラフィックハンドルの作成
		_MEMSET( Param->HandleBuf, 0, sizeof( int ) * Param->AllNum ) ;
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			Param->HandleBuf[ i ] = Graphics_Image_AddHandle( ASyncThread ) ;
			if( Param->HandleBuf[ i ] < 0 )
			{
				goto ERR ;
			}
		}
		Param->BaseHandle = Graphics_Image_AddHandle( ASyncThread ) ;
		if( Param->BaseHandle < 0 )
		{
			goto ERR ;
		}
	}
	else
	{
#ifndef DX_NON_ASYNCLOAD
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			if( NS_CheckHandleASyncLoad( Param->HandleBuf[ i ] ) > 0 )
			{
				return -1 ;
			}
		}

		// 非同期読み込みカウントをインクリメント
		if( ASyncThread )
		{
			IncASyncLoadCount( Param->BaseHandle, -1 ) ;
			for( i = 0 ; i < Param->AllNum ; i ++ )
			{
				IncASyncLoadCount( Param->HandleBuf[ i ], -1 ) ;
			}
		}
#endif // DX_NON_ASYNCLOAD
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, Param, sizeof( *Param ) ) ;
		AddASyncLoadParamStruct( NULL, &Addr, Param->HandleBuf, ( int )( sizeof( int ) * Param->AllNum ) ) ;
		if( Param->FileName != NULL )
		{
			AddASyncLoadParamString( NULL, &Addr, Param->FileName ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( NULL, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = Graphics_Image_CreateDivGraph_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, Param, sizeof( *Param ) ) ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, Param->HandleBuf, ( int )( sizeof( int ) * Param->AllNum ) ) ;
		if( Param->FileName != NULL )
		{
			AddASyncLoadParamString( AParam->Data, &Addr, Param->FileName ) ; 
		}
		if( Param->RgbBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->RgbBaseImage, sizeof( *Param->RgbBaseImage ) ) ;
		}
		if( Param->AlphaBaseImage != NULL )
		{
			AddASyncLoadParamStruct( AParam->Data, &Addr, Param->AlphaBaseImage, sizeof( *Param->AlphaBaseImage ) ) ;
		}

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		for( i = 0 ; i < Param->AllNum ; i ++ )
			IncASyncLoadCount( Param->HandleBuf[ i ], AParam->Index ) ;
		IncASyncLoadCount( Param->BaseHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( Graphics_Image_CreateDivGraph_Static( Param, FALSE ) < 0 )
			goto ERR ;

		if( Param->ReCreateFlag == FALSE )
		{
			NS_DeleteGraph( Param->BaseHandle ) ;
		}
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		// 非同期読み込みカウントをデクリメント
		DecASyncLoadCount( Param->BaseHandle ) ;
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			DecASyncLoadCount( Param->HandleBuf[ i ] ) ;
		}
	}
#endif // DX_NON_ASYNCLOAD

	// 正常終了
	return 0 ;

ERR :
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		// 非同期読み込みカウントをデクリメント
		DecASyncLoadCount( Param->BaseHandle ) ;
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			DecASyncLoadCount( Param->HandleBuf[ i ] ) ;
		}
	}
#endif // DX_NON_ASYNCLOAD

	if( Param->ReCreateFlag == FALSE )
	{
		for( i = 0 ; i < Param->AllNum ; i ++ )
		{
			NS_DeleteGraph( Param->HandleBuf[ i ] ) ;
		}
		NS_DeleteGraph( Param->BaseHandle ) ;
	}

	return -1 ;
}


// LoadBmpToGraph のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_LoadBmpToGraph_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const wchar_t *GraphName,
	int TextureFlag,
	int ReverseFlag,
	int SurfaceMode,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.FileName = GraphName ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = SurfaceMode ;

	return Graphics_Image_CreateGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// LoadDivBmpToGraph のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_LoadDivBmpToGraph_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const wchar_t *FileName,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.FileName = FileName ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return Graphics_Image_CreateDivGraph_UseGParam( &Param, ASyncLoadFlag, ASyncThread ) ;
}

// CreateGraphFromMem のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateGraphFromMem_UseGParam( 
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const void *MemImage,
	int MemImageSize,
	const void *AlphaImage,
	int AlphaImageSize,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.RgbMemImage = MemImage ;
	Param.RgbMemImageSize = MemImageSize ;
	Param.AlphaMemImage = AlphaImage ;
	Param.AlphaMemImageSize = AlphaImageSize ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = DX_MOVIESURFACE_NORMAL ;

	return Graphics_Image_CreateGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// CreateDivGraphFromMem のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateDivGraphFromMem_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const void *MemImage,
	int MemImageSize,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	const void *AlphaImage,
	int AlphaImageSize,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.RgbMemImage = MemImage ;
	Param.RgbMemImageSize = MemImageSize ;
	Param.AlphaMemImage = AlphaImage ;
	Param.AlphaMemImageSize = AlphaImageSize ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return Graphics_Image_CreateDivGraph_UseGParam( &Param, ASyncLoadFlag, ASyncThread ) ;
}

// CreateGraphFromBmp のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateGraphFromBmp_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const BITMAPINFO *BmpInfo,
	const void *GraphData,
	const BITMAPINFO *AlphaInfo,
	const void *AlphaData,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.RgbBmpInfo = BmpInfo ;
	Param.RgbBmpImage = ( void * )GraphData ;
	Param.AlphaBmpInfo = AlphaInfo ;
	Param.AlphaBmpImage = ( void * )AlphaData ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = DX_MOVIESURFACE_NORMAL ;

	return Graphics_Image_CreateGraph_UseGParam( &Param, ASyncLoadFlag ) ;
}

// CreateDivGraphFromBmp のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateDivGraphFromBmp_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const BITMAPINFO *BmpInfo,
	const void *GraphData,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	const BITMAPINFO *AlphaInfo,
	const void *AlphaData,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.RgbBmpInfo = BmpInfo ;
	Param.RgbBmpImage = ( void * )GraphData ;
	Param.AlphaBmpInfo = AlphaInfo ;
	Param.AlphaBmpImage = ( void * )AlphaData ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return Graphics_Image_CreateDivGraph_UseGParam( &Param, ASyncLoadFlag, FALSE ) ;
}

// CreateGraphFromGraphImage のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateGraphFromGraphImage_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	const BASEIMAGE *BaseImage,
	const BASEIMAGE *AlphaBaseImage,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;
	Param.GrHandle = GrHandle ;

	Param.RgbBaseImage = ( BASEIMAGE * )BaseImage ;
	Param.AlphaBaseImage = ( BASEIMAGE * )AlphaBaseImage ;

	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;
	Param.SurfaceMode = DX_MOVIESURFACE_NORMAL ;

	return Graphics_Image_CreateGraph_UseGParam( &Param, ASyncLoadFlag, ASyncThread ) ;
}

// CreateDivGraphFromGraphImage のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateDivGraphFromGraphImage_UseGParam(
	LOADGRAPH_GPARAM *GParam,
	int ReCreateFlag,
	const BASEIMAGE *BaseImage,
	const BASEIMAGE *AlphaBaseImage,
	int AllNum,
	int XNum,
	int YNum,
	int SizeX,
	int SizeY,
	int *HandleBuf,
	int TextureFlag,
	int ReverseFlag,
	int ASyncLoadFlag
)
{
	LOADGRAPH_PARAM Param ;

	_MEMSET( &Param, 0, sizeof( Param ) ) ;
	Param.GParam = *GParam ;
	Param.ReCreateFlag = ReCreateFlag ;

	Param.RgbBaseImage = ( BASEIMAGE * )BaseImage ;
	Param.AlphaBaseImage = ( BASEIMAGE * )AlphaBaseImage ;

	Param.AllNum = AllNum ;
	Param.XNum = XNum ;
	Param.YNum = YNum ;
	Param.SizeX = SizeX ;
	Param.SizeY = SizeY ;
	Param.HandleBuf = HandleBuf ;
	Param.TextureFlag = TextureFlag ;
	Param.ReverseFlag = ReverseFlag ;

	return Graphics_Image_CreateDivGraph_UseGParam( &Param, ASyncLoadFlag, FALSE ) ;
}

// Graphics_Image_CreateGraphFromGraphImageBase のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateGraphFromGraphImageBase_UseGParam(
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam,
	int ReCreateFlag,
	int GrHandle,
	BASEIMAGE *BaseImage,
	const BASEIMAGE *AlphaBaseImage,
	int TextureFlag,
	int ASyncThread
)
{
	int SizeX, SizeY ;
	int UseTempImage = FALSE ;
	BASEIMAGE TempImage ;
	int Result ;

	// 再作成ではない場合は画像の透過色の処理を行う
	if( ReCreateFlag == FALSE )
	{
		// 透過色カラー使用を前提とした画像にする場合は、ここで透過色カラー処理前提画像を作成する
		if( GParam->UseTransColorGraphCreateFlag && GParam->NotUseTransColor == FALSE )
		{
			int w, h, i, j, r, g, b, a, r2, g2, b2, a2, tr, tg, tb, useAlpha;

			w = BaseImage->Width;
			h = BaseImage->Height;
			tr = ( int )( ( GParam->InitHandleGParam.TransColor >> 16 ) & 0xff ) ;
			tg = ( int )( ( GParam->InitHandleGParam.TransColor >>  8 ) & 0xff ) ;
			tb = ( int )( ( GParam->InitHandleGParam.TransColor >>  0 ) & 0xff ) ;
			useAlpha = BaseImage->ColorData.AlphaWidth != 0;
			if( NS_CreateARGB8ColorBaseImage( w, h, &TempImage ) == 0 )
			{
				// 透過色のチェックを行いながら画像を転送
				for( i = 0; i < h; i++ )
				{
					for( j = 0; j < w; j++ )
					{
						NS_GetPixelBaseImage(      BaseImage, j, i, &r, &g, &b, &a );
						NS_SetPixelBaseImage( &TempImage, j, i,  r,  g,  b, r == tr && g == tg && b == tb ? 0 : ( useAlpha ? a : 255 ) );
					}
				}

				// 透過色ではない部分の色を透過色の部分に代入する
				for( i = 0; i < h; i++ )
				{
					for( j = 0; j < w; j++ )
					{
						NS_GetPixelBaseImage( BaseImage, j, i, &r, &g, &b, &a );
						if( r != tr || g != tg || b != tb )
						{
							if( j >    0 ){ NS_GetPixelBaseImage( BaseImage, j-1, i  ,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j-1, i  ,r,g,b,0); }
							if( i >    0 ){ NS_GetPixelBaseImage( BaseImage, j  , i-1,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j  , i-1,r,g,b,0); }
							if( j < w -1 ){ NS_GetPixelBaseImage( BaseImage, j+1, i  ,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j+1, i  ,r,g,b,0); }
							if( i < h -1 ){ NS_GetPixelBaseImage( BaseImage, j  , i+1,&r2,&g2,&b2,&a2); if( r2 == tr && g2 == tg && b2 == tb ) NS_SetPixelBaseImage( &TempImage, j  , i+1,r,g,b,0); }
						}
					}
				}

				// 新しいイメージの情報をセット
				BaseImage = &TempImage;

				// 仮画像を使用しているフラグを立てる
				UseTempImage = TRUE ;
			}
		}
	}

	// なんの機能も割り付けられていないビットのマスクをセットする
	// (NoneMask は途中から入れたメンバ変数なので、下位互換性を持たせるために・・・)
	NS_SetColorDataNoneMask( ( COLORDATA * )&BaseImage->ColorData ) ;
	if( AlphaBaseImage != NULL ) NS_SetColorDataNoneMask( ( COLORDATA * )&AlphaBaseImage->ColorData ) ;

	// 再作成の場合はサイズが同一かチェックする
	if( ReCreateFlag )
	{
		// サイズが違ったらエラー
		if( NS_GetGraphSize( GrHandle, &SizeX, &SizeY ) == -1 ) return -1 ;
		if( SizeX != BaseImage->Width || SizeY != BaseImage->Height ) return -1 ;
		if( AlphaBaseImage != NULL && ( SizeX != AlphaBaseImage->Width || SizeY != AlphaBaseImage->Height ) ) return -1 ;

		LUTRANS_RE_GPARAM( GParam, BaseImage, GrHandle )

		Result = 0 ;
	}
	else
	{
		LUTRANS_GPARAM( GParam, BaseImage ) ;

		// グラフィックハンドルを作成する
		Result = Graphics_Image_CreateDXGraph_UseGParam( &GParam->InitHandleGParam, GrHandle, BaseImage, AlphaBaseImage, TextureFlag, ASyncThread ) ;
	}

	if( Result != -1 )
	{
		Graphics_Image_BltBmpOrGraphImageToGraphBase(
			BaseImage,
			AlphaBaseImage,
			0,
			0,
			GrHandle,
			GParam->NotUseTransColor ? FALSE : TRUE,
			ASyncThread
		) ;
	}

	// 仮画像を使用していた場合は解放する
	if( UseTempImage )
	{
		NS_ReleaseBaseImage( &TempImage );
	}

	// 成功か失敗かを返す
	return Result ;
}

// Graphics_Image_CreateDivGraphFromGraphImageBase のグローバル変数にアクセスしないバージョン
extern int Graphics_Image_CreateDivGraphFromGraphImageBase_UseGParam( CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam, int ReCreateFlag, int BaseHandle, BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag, int ASyncThread )
{
	int Result ;
	int GrSizeX, GrSizeY ;

	if( ReCreateFlag == FALSE )
	{
		int i, j, k, x, y, basex, addx ;

		// 普通にグラフィックを作成する
		if( Graphics_Image_CreateGraphFromGraphImageBase_UseGParam( GParam, FALSE, BaseHandle, BaseImage, AlphaBaseImage, TextureFlag, ASyncThread ) < 0 )
			return -1 ;

		if( ReverseFlag == TRUE )
		{
			basex = ( XNum - 1 ) * SizeX ;
			addx = -SizeX ;
		}
		else
		{
			basex = 0 ;
			addx = SizeX ;
		}
		
		// 指定の枚数に分割する
		k = 0 ;
		y = 0 ;
		for( i = 0 ; k < AllNum && i < YNum ; i ++, y += SizeY )
		{
			x = basex ;
			for( j = 0 ; k < AllNum && j < XNum ; j ++, k ++, x += addx )
			{
				if( Graphics_Image_DerivationGraphBase( HandleBuf[ k ], x, y, SizeX, SizeY, BaseHandle, ASyncThread ) < 0 )
				{
					DXST_ERRORLOG_ADDUTF16LE( "\xb0\x30\xe9\x30\xd5\x30\xa3\x30\xc3\x30\xaf\x30\x6e\x30\x06\x52\x72\x52\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x20\x00\x69\x00\x6e\x00\x20\x00\x43\x00\x72\x00\x65\x00\x61\x00\x74\x00\x65\x00\x44\x00\x69\x00\x76\x00\x47\x00\x72\x00\x61\x00\x70\x00\x68\x00\x46\x00\x72\x00\x6f\x00\x6d\x00\x47\x00\x72\x00\x61\x00\x70\x00\x68\x00\x49\x00\x6d\x00\x61\x00\x67\x00\x65\x00\x0a\x00\x00"/*@ L"グラフィックの分割に失敗しました in CreateDivGraphFromGraphImage\n" @*/ ) ;
					return -1 ;
				}
			}
		}
		Result = 0 ;
	}
	else
	{
		// なんの機能も割り付けられていないビットのマスクをセットする
		// (NoneMask は途中から入れたメンバ変数なので、下位互換性を持たせるために・・・)
		NS_SetColorDataNoneMask( ( COLORDATA * )&BaseImage->ColorData ) ;
		if( AlphaBaseImage != NULL ) NS_SetColorDataNoneMask( ( COLORDATA * )&AlphaBaseImage->ColorData ) ;

		// サイズが違ったらエラー
		if( NS_GetGraphSize( HandleBuf[ 0 ], &GrSizeX, &GrSizeY ) == -1 ) return -1 ;
		if( BaseImage->Width < XNum * SizeX || BaseImage->Height < YNum * SizeY ) return -1 ;
		if( AlphaBaseImage != NULL && ( AlphaBaseImage->Width != BaseImage->Width || AlphaBaseImage->Height != BaseImage->Height ) ) return -1 ;

		LUTRANS_RE_DIV_GPARAM( GParam, BaseImage, HandleBuf, AllNum )

		// 分割グラフィックをグラフィックハンドルに転送する
		Result = Graphics_Image_BltBmpOrGraphImageToDivGraphBase(
				BaseImage,
				AlphaBaseImage,
				AllNum,
				XNum,
				YNum,
				SizeX,
				SizeY,
				HandleBuf,
				ReverseFlag,
				GParam->NotUseTransColor ? FALSE : TRUE,
				ASyncThread
		) ; 
	}

	// 終了
	return Result ;
}

// CreateGraphFromGraphImage の内部関数
extern int Graphics_Image_CreateGraphFromGraphImageBase( BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int TextureFlag, int ASyncThread )
{
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;
	int GrHandle ;
	int Result ;

	CheckActiveState() ;

	GrHandle = Graphics_Image_AddHandle( ASyncThread ) ;
	if( GrHandle == -1 )
	{
		return -1 ;
	}

	Graphics_Image_InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	Result = Graphics_Image_CreateGraphFromGraphImageBase_UseGParam( &GParam, FALSE, GrHandle, BaseImage, AlphaBaseImage, TextureFlag, FALSE ) ;
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( GrHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD
	if( Result < 0 )
	{
		NS_DeleteGraph( GrHandle ) ;
		return -1 ;
	}

	return GrHandle ;
}

// CreateDivGraphFromGraphImage の内部関数
extern int Graphics_Image_CreateDivGraphFromGraphImageBase( BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag )
{
	int BaseHandle, i ;
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;

	// 分割グラフィックハンドルの値を初期化
	for( i = 0 ; i < AllNum ; i ++ )
	{
		HandleBuf[ i ] = -1 ;
	}

	// 基本グラフィックハンドルの作成
	BaseHandle = Graphics_Image_AddHandle( FALSE ) ;
	if( BaseHandle == -1 )
	{
		return -1 ;
	}

	// 分割グラフィックハンドルの作成
	_MEMSET( HandleBuf, 0, AllNum * sizeof( int ) ) ;
	for( i = 0 ; i < AllNum ; i ++ )
	{
		HandleBuf[ i ] = Graphics_Image_AddHandle( FALSE ) ;
		if( HandleBuf[ i ] == -1 )
		{
			goto ERR ;
		}
	}

	// 初期化用データの準備
	Graphics_Image_InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	// 分割グラフィックハンドルの作成
	if( Graphics_Image_CreateDivGraphFromGraphImageBase_UseGParam( &GParam, FALSE, BaseHandle, BaseImage, AlphaBaseImage, AllNum, XNum, YNum, SizeX, SizeY, HandleBuf, TextureFlag, ReverseFlag, FALSE ) < 0 )
		goto ERR ;
	
	// 元となったハンドルを解放
	NS_DeleteGraph( BaseHandle ) ;

	// 終了
	return 0 ;
	
ERR :
	for( i = 0 ; i < AllNum ; i ++ )
	{
		NS_DeleteGraph( HandleBuf[ i ] ) ;
	}
	NS_DeleteGraph( BaseHandle ) ;

	// エラー終了	
	return -1 ;
}

// ReCreateGraphFromGraphImage の内部関数
extern int Graphics_Image_ReCreateGraphFromGraphImageBase( BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int GrHandle, int TextureFlag )
{
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;
	int Result ;

	CheckActiveState() ;

	Graphics_Image_InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	Result = Graphics_Image_CreateGraphFromGraphImageBase_UseGParam( &GParam, TRUE, GrHandle, BaseImage, AlphaBaseImage, TextureFlag, FALSE ) ;

	return Result ;
}

// ReCreateDivGraphFromGraphImage の内部関数
extern int Graphics_Image_ReCreateDivGraphFromGraphImageBase( BASEIMAGE *BaseImage, const BASEIMAGE *AlphaBaseImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, const int *HandleBuf, int TextureFlag, int ReverseFlag )
{
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM GParam ;
	int Result ;

	CheckActiveState() ;

	Graphics_Image_InitCreateGraphHandleAndBltGraphImageGParam( &GParam ) ;

	Result = Graphics_Image_CreateDivGraphFromGraphImageBase_UseGParam( &GParam, TRUE, -1, BaseImage, AlphaBaseImage, AllNum, XNum, YNum, SizeX, SizeY, ( int * )HandleBuf, TextureFlag, ReverseFlag, FALSE ) ;

	// 終了
	return Result ;
}

// CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM のデータをセットする
extern void Graphics_Image_InitCreateGraphHandleAndBltGraphImageGParam( CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam )
{
	Graphics_Image_InitSetupGraphHandleGParam( &GParam->InitHandleGParam ) ;

	GParam->UseTransColorGraphCreateFlag	= GSYS.CreateImage.UseTransColorFlag ;
	GParam->NotUseTransColor				= GSYS.CreateImage.NotUseTransColor ;
	GParam->LeftUpColorIsTransColorFlag		= GSYS.CreateImage.LeftUpColorIsTransColorFlag ;
}

// SETUP_GRAPHHANDLE_GPARAM のデータをセットする
extern void Graphics_Image_InitSetupGraphHandleGParam( SETUP_GRAPHHANDLE_GPARAM *GParam )
{
	GParam->TransColor							= GSYS.CreateImage.TransColor ;

	GParam->CreateImageColorBitDepth			= GSYS.CreateImage.ColorBitDepth ;
	GParam->CreateImageChannelBitDepth			= GSYS.CreateImage.ChannelBitDepth ;
	GParam->AlphaTestImageCreateFlag			= GSYS.CreateImage.AlphaTestFlag ;
	GParam->AlphaChannelImageCreateFlag			= GSYS.CreateImage.AlphaChannelFlag ;
	GParam->CubeMapTextureCreateFlag			= GSYS.CreateImage.CubeMapFlag ;
	GParam->BlendImageCreateFlag				= GSYS.CreateImage.BlendImageFlag ;
	GParam->UseManagedTextureFlag				= GSYS.CreateImage.NotUseManagedTextureFlag ? FALSE : TRUE ;

	GParam->DrawValidImageCreateFlag			= GSYS.CreateImage.DrawValidFlag ;
	GParam->DrawValidAlphaImageCreateFlag		= GSYS.CreateImage.DrawValidAlphaFlag ;
	GParam->DrawValidFloatTypeGraphCreateFlag	= GSYS.CreateImage.DrawValidFloatTypeFlag ;
	GParam->DrawValidGraphCreateZBufferFlag		= GSYS.CreateImage.NotDrawValidCreateZBufferFlag ? FALSE : TRUE ;
	GParam->CreateDrawValidGraphZBufferBitDepth	= GSYS.CreateImage.DrawValidZBufferBitDepth ;
	GParam->CreateDrawValidGraphChannelNum		= GSYS.CreateImage.DrawValidChannelNum ;
	GParam->DrawValidMSSamples					= GSYS.CreateImage.DrawValidMSSamples ;
	GParam->DrawValidMSQuality					= GSYS.CreateImage.DrawValidMSQuality ;

	GParam->MipMapCount							= GSYS.CreateImage.MipMapCount ;
	GParam->UserMaxTextureSize					= GSYS.CreateImage.UserMaxTextureSize ;
	GParam->NotUseDivFlag						= GSYS.CreateImage.NotUseDivFlag ;
	GParam->NotUseAlphaImageLoadFlag			= GSYS.CreateImage.NotUseAlphaImageLoadFlag ;
	GParam->NotInitGraphDelete					= FALSE ;
}

extern void Graphics_Image_InitSetupGraphHandleGParam_Normal_NonDrawValid( SETUP_GRAPHHANDLE_GPARAM *GParam, int BitDepth, int AlphaChannel, int AlphaTest )
{
	GParam->TransColor							= 0x00ff00ff ;

	GParam->CreateImageColorBitDepth			= BitDepth ;
	GParam->CreateImageChannelBitDepth			= 0 ;
	GParam->AlphaTestImageCreateFlag			= AlphaTest ;
	GParam->AlphaChannelImageCreateFlag			= AlphaChannel ;
	GParam->CubeMapTextureCreateFlag			= FALSE ;
	GParam->BlendImageCreateFlag				= FALSE ;
	GParam->UseManagedTextureFlag				= TRUE ;

	GParam->DrawValidImageCreateFlag			= FALSE ;
	GParam->DrawValidAlphaImageCreateFlag		= FALSE ;
	GParam->DrawValidFloatTypeGraphCreateFlag	= FALSE ;
	GParam->DrawValidGraphCreateZBufferFlag		= FALSE ;
	GParam->CreateDrawValidGraphZBufferBitDepth	= 0 ;
	GParam->CreateDrawValidGraphChannelNum		= 0 ;
	GParam->DrawValidMSSamples					= 0 ;
	GParam->DrawValidMSQuality					= 0 ;

	GParam->MipMapCount							= 1 ;
	GParam->UserMaxTextureSize					= 0 ;
	GParam->NotUseDivFlag						= TRUE ;
	GParam->NotUseAlphaImageLoadFlag			= TRUE ;
	GParam->NotInitGraphDelete					= FALSE ;
}

extern void Graphics_Image_InitSetupGraphHandleGParam_Normal_DrawValid_NoneZBuffer( SETUP_GRAPHHANDLE_GPARAM *GParam, int BitDepth, int AlphaChannel )
{
	GParam->TransColor							= 0x00ff00ff ;

	GParam->CreateImageColorBitDepth			= BitDepth ;
	GParam->CreateImageChannelBitDepth			= 0 ;
	GParam->AlphaTestImageCreateFlag			= FALSE ;
	GParam->AlphaChannelImageCreateFlag			= AlphaChannel ;
	GParam->CubeMapTextureCreateFlag			= FALSE ;
	GParam->BlendImageCreateFlag				= FALSE ;
	GParam->UseManagedTextureFlag				= FALSE ;

	GParam->DrawValidImageCreateFlag			= TRUE ;
	GParam->DrawValidAlphaImageCreateFlag		= AlphaChannel ;
	GParam->DrawValidFloatTypeGraphCreateFlag	= FALSE ;
	GParam->DrawValidGraphCreateZBufferFlag		= FALSE ;
	GParam->CreateDrawValidGraphZBufferBitDepth	= 0 ;
	GParam->CreateDrawValidGraphChannelNum		= 0 ;
	GParam->DrawValidMSSamples					= 0 ;
	GParam->DrawValidMSQuality					= 0 ;

	GParam->MipMapCount							= 1 ;
	GParam->UserMaxTextureSize					= 0 ;
	GParam->NotUseDivFlag						= TRUE ;
	GParam->NotUseAlphaImageLoadFlag			= TRUE ;
	GParam->NotInitGraphDelete					= FALSE ;
}

// SETGRAPHBASEINFO_GPARAM のデータをセットする
extern void Graphics_Image_InitSetGraphBaseInfoGParam( SETGRAPHBASEINFO_GPARAM *GParam )
{
	GParam->NotGraphBaseDataBackupFlag = GSYS.CreateImage.NotGraphBaseDataBackupFlag ;
}

// LOADGRAPH_GPARAM のデータをセットする
extern void Graphics_Image_InitLoadGraphGParam( LOADGRAPH_GPARAM *GParam )
{
	InitLoadBaseImageGParam( &GParam->LoadBaseImageGParam, FALSE ) ;
#ifndef DX_NON_MOVIE
	Graphics_Image_InitOpenMovieGParam( &GParam->OpenMovieGParam ) ;
#endif
	Graphics_Image_InitCreateGraphHandleAndBltGraphImageGParam( &GParam->CreateGraphGParam ) ;
	Graphics_Image_InitSetGraphBaseInfoGParam( &GParam->SetGraphBaseInfoGParam ) ;
}























// 描画設定関係関数

// グラフィック描画設定関係の情報を初期化
extern	int		Graphics_DrawSetting_Initialize( void )
{
	int i ;

//	int NotDrawFlag                            = GSYS.NotDrawFlag ;
//	int NotUseBasicGraphDraw3DDeviceMethodFlag = GSYS.DrawSetting.NotUseBasicGraphDraw3DDeviceMethodFlag ;
//	int PreSetWaitVSyncFlag                    = GSYS.Screen.PreSetWaitVSyncFlag ;
//	int NotGraphBaseDataBackupFlag             = GSYS.CreateImage.NotGraphBaseDataBackupFlag ;
//	int NotUseManagedTextureFlag               = GSYS.CreateImage.NotUseManagedTextureFlag ;
	RECT OriginalDrawRect                      = GSYS.DrawSetting.OriginalDrawRect ;
	int Large3DPositionSupport                 = GSYS.DrawSetting.Large3DPositionSupport ;

	// ゼロ初期化
	_MEMSET( &GSYS.DrawSetting, 0, sizeof( GSYS.DrawSetting ) ) ;
	_MEMSET( &GSYS.Camera,      0, sizeof( GSYS.Camera      ) ) ;
//	_MEMSET( &GSYS.CreateImage, 0, sizeof( GSYS.CreateImage ) ) ;

//	GSYS.NotDrawFlag						     = NotDrawFlag ;
//	GSYS.DrawSetting.NotUseBasicGraphDraw3DDeviceMethodFlag = NotUseBasicGraphDraw3DDeviceMethodFlag ;
//	GSYS.Screen.PreSetWaitVSyncFlag             = PreSetWaitVSyncFlag ;
//	GSYS.CreateImage.NotGraphBaseDataBackupFlag = NotGraphBaseDataBackupFlag ;
//	GSYS.CreateImage.NotUseManagedTextureFlag   = NotUseManagedTextureFlag ;
	GSYS.DrawSetting.OriginalDrawRect           = OriginalDrawRect ;
	GSYS.DrawSetting.Large3DPositionSupport     = Large3DPositionSupport ;

	// テクスチャアドレスモードの初期設定
	for( i = 0 ; i < USE_TEXTURESTAGE_NUM ; i ++ )
	{
		if( GSYS.DrawSetting.TexAddressModeU[ i ] == 0 )
			GSYS.DrawSetting.TexAddressModeU[ i ] = DX_TEXADDRESS_CLAMP ;
		if( GSYS.DrawSetting.TexAddressModeV[ i ] == 0 )
			GSYS.DrawSetting.TexAddressModeV[ i ] = DX_TEXADDRESS_CLAMP ;
		if( GSYS.DrawSetting.TexAddressModeW[ i ] == 0 )
			GSYS.DrawSetting.TexAddressModeW[ i ] = DX_TEXADDRESS_CLAMP ;
	}

	// ブレンド画像は設定されていない状態にする
	GSYS.DrawSetting.BlendGraph = -1 ;

	// Ｚバッファの初期設定
	GSYS.DrawSetting.EnableZBufferFlag2D = FALSE ;
	GSYS.DrawSetting.WriteZBufferFlag2D  = FALSE ;
	GSYS.DrawSetting.EnableZBufferFlag3D = FALSE ;
	GSYS.DrawSetting.WriteZBufferFlag3D  = FALSE ;

	// ライトの設定をハードウエアアクセラレータに反映するようにフラグをセット
	GSYS.Light.HardwareChange = TRUE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 描画Ｚ値をセット
	GSYS.DrawSetting.DrawZ = 0.2f;

	// 初期Ｚ比較モードをセット
	GSYS.DrawSetting.ZBufferCmpType2D = DX_CMP_LESSEQUAL;
	GSYS.DrawSetting.ZBufferCmpType3D = DX_CMP_LESSEQUAL;

	// 初期はテクスチャサーフェスを作成する方向で
//	GSYS.TextureImageCreateFlag = TRUE ;

	// 透過色にアルファテストを使用する
	GSYS.CreateImage.AlphaTestFlag = TRUE ;
	BASEIM.AlphaTestImageCreateFlag = TRUE ;

	// 描画モードをネアレストネイバーにセット
	GSYS.DrawSetting.DrawMode = DX_DRAWMODE_NEAREST ;

	// 最大異方性を１にセット
	GSYS.DrawSetting.MaxAnisotropy = 1 ;

	// フィルモードはポリゴン描画
	GSYS.DrawSetting.FillMode = DX_FILL_SOLID ;

	// ブレンドモードをαブレンドにセット
	GSYS.DrawSetting.BlendMode = DX_BLENDMODE_NOBLEND ;

	// アルファテストモードはデフォルト動作
	GSYS.DrawSetting.AlphaTestMode = -1 ;

	// 描画輝度をセット
	NS_SetDrawBright( 255, 255, 255 ) ;
//	GSYS.DrawSetting.bDrawBright = 0xffffff ;

	// フォグの初期設定
	NS_SetFogMode( DX_FOGMODE_LINEAR ) ;
	NS_SetFogColor( 255, 255, 255 ) ;
	NS_SetFogStartEnd( 0.0f, 1500.0f ) ;

	// 使用するピクセルフォーマットをセット
//	SetCreateGraphColorBitDepth( COLORBITDEPTH ) ;
	NS_SetCreateGraphColorBitDepth( 0 ) ;

#ifndef DX_NON_MASK
	// マスクは使用するにセット
	MASKD.MaskUseFlag = TRUE ;
	MASKD.MaskValidFlag = FALSE ;
#endif

	// 描画先スクリーン領域をセット
	GSYS.DrawSetting.TargetScreen[ 0 ] = DX_SCREEN_BACK ;
	GSYS.DrawSetting.TargetScreenSurface[ 0 ] = 0 ;
	NS_SetDrawScreen( DX_SCREEN_FRONT ) ;

	// 描画可能領域を設定
	{
		int Width, Height ;

		NS_GetGraphSize( GSYS.DrawSetting.TargetScreen[ 0 ], &Width, &Height ) ;
		NS_SetDrawArea( 0, 0, Width, Height ) ;
	}

	// 標準をαブレンドにする
//	NS_SetDrawBlendMode( DX_BLENDMODE_ALPHA, 255 ) ;
	
	// 描画を可能な状態にする
//	GSYS.NotDrawFlag = FALSE ;

	// デフォルトのライトを作成
	{
		VECTOR Dir ;

		Dir.x =  1.0f ;
		Dir.y = -1.0f ;
		Dir.z =  1.0f ;
		GSYS.Light.DefaultHandle = NS_CreateDirLightHandle( Dir ) ;
	}

	// 各種行列に単位行列のセット
	CreateIdentityMatrixD( &GSYS.DrawSetting.WorldMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.WorldMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.ViewMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.ViewMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.ProjectionMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.ProjectionMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.Direct3DViewportMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.Direct3DViewportMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.Direct3DViewportMatrixAnti ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.Direct3DViewportMatrixAntiF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.ViewportMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.ViewportMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.BlendMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.BlendMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.InverseBlendMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.InverseBlendMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.WorldViewMatrix ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.ViewProjectionViewportMatrix ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.BillboardMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.BillboardMatrixF ) ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.ConvScreenPosToWorldPosMatrix ) ;
	CreateIdentityMatrix(  &GSYS.DrawSetting.TextureAddressTransformMatrix ) ;

	// 射影行列の設定を初期化
	GSYS.DrawSetting.ProjectionMatrixMode = 0 ;
	GSYS.DrawSetting.ProjNear             = 10.0 ;
	GSYS.DrawSetting.ProjFar              = 10000.0 ;
	GSYS.DrawSetting.ProjDotAspect        = 1.0 ;
	GSYS.DrawSetting.ProjFov              = DEFAULT_FOV ;
	GSYS.DrawSetting.ProjSize             = 1.0 ;
	CreateIdentityMatrixD( &GSYS.DrawSetting.ProjMatrix ) ;

	// マテリアルの初期値をセット
	GSYS.Light.ChangeMaterial    = 1 ;
	GSYS.Light.Material.Diffuse  = GetColorF( 0.8f, 0.8f, 0.8f, 1.0f ) ;
	GSYS.Light.Material.Specular = GetColorF( 0.8f, 0.8f, 0.8f, 0.0f ) ;
	GSYS.Light.Material.Ambient  = GetColorF( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	GSYS.Light.Material.Emissive = GetColorF( 0.0f, 0.0f, 0.0f, 0.0f ) ;
	GSYS.Light.Material.Power    = 20.0f ;

	// ３Ｄ描画スケール値を初期化
	GSYS.DrawSetting.Draw3DScale = 1.0f ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 終了
	return 0 ;
}

// SetDrawBright の引数が一つ版
extern void FASTCALL Graphics_DrawSetting_SetDrawBrightToOneParam( DWORD Bright )
{
	Bright &= 0xffffff ;
	if( GSYS.DrawSetting.bDrawBright == Bright ) return ;

	// 輝度を保存
	GSYS.DrawSetting.bDrawBright = Bright ;
	SetMemImgDrawBright( Bright ) ;
	GSYS.ChangeSettingFlag = TRUE ;

	// ハードウエアアクセラレーションの設定に反映
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetDrawBrightToOneParam_PF( Bright ) ;
	}
}

// 補助関数
extern void FASTCALL Graphics_DrawSetting_BlendModeSub_Pre( RECT *DrawRect )
{
	DWORD Bright ;
	DWORD Cr ;
	int   BlendParam ;

	Cr         = NS_GetColor( 255,255,255 ) ;
	Bright     = GSYS.DrawSetting.bDrawBright ;
	BlendParam = GSYS.DrawSetting.BlendParam ;

	Graphics_DrawSetting_SetDrawBrightToOneParam( 0xffffff ) ;
	NS_SetDrawBlendMode( DX_BLENDMODE_SUB, 255 ) ;

	Graphics_Hardware_DrawFillBox_PF( DrawRect->left, DrawRect->top, DrawRect->right, DrawRect->bottom, Cr ) ;

	Graphics_DrawSetting_SetDrawBrightToOneParam( Bright ) ;
	NS_SetDrawBlendMode( DX_BLENDMODE_SUB2, BlendParam ) ;
}

extern void FASTCALL Graphics_DrawSetting_BlendModeSub_Post( RECT *DrawRect )
{
	DWORD Bright ;
	DWORD Cr ;
	int   BlendParam ;

	Cr         = NS_GetColor( 255,255,255 ) ;
	Bright     = GSYS.DrawSetting.bDrawBright ;
	BlendParam = GSYS.DrawSetting.BlendParam ;

	Graphics_DrawSetting_SetDrawBrightToOneParam( 0xffffff ) ;
	NS_SetDrawBlendMode( DX_BLENDMODE_SUB, 255 ) ; 
	
	Graphics_Hardware_DrawFillBox_PF( DrawRect->left, DrawRect->top, DrawRect->right, DrawRect->bottom, Cr ) ;

	Graphics_DrawSetting_SetDrawBrightToOneParam( Bright ) ;
	NS_SetDrawBlendMode( DX_BLENDMODE_SUB, BlendParam ) ;
}

// SetBlendGraphParam の可変長引数パラメータ付き
extern int Graphics_DrawSetting_SetBlendGraphParamBase( int BlendGraph, int BlendType, va_list ParamList )
{
	IMAGEDATA *BlendImage ;
	int Param[ 16 ] ;

	// ブレンドする画像が -1 だったらブレンド画像処理を止めるということ
	if( BlendGraph <= 0 )
	{
		BlendImage = NULL ;
		GSYS.DrawSetting.BlendGraph = -1 ;
	}
	else
	{
		// エラー判定
		if( GRAPHCHK( BlendGraph, BlendImage ) )
			return -1 ;
		GSYS.DrawSetting.BlendGraph = BlendGraph ;
	}

	CheckActiveState() ;

	if( BlendImage != NULL )
	{
		switch( BlendType )
		{
		case DX_BLENDGRAPHTYPE_NORMAL :
			int FadeRatio ;

			FadeRatio = va_arg( ParamList, int ) ;

			if( FadeRatio > 255 ) FadeRatio = 255 ;
			else
			if( FadeRatio < 0   ) FadeRatio = 0   ;

			Param[ 0 ] = FadeRatio ;
			break ;

		case DX_BLENDGRAPHTYPE_WIPE :
			int BorderRange ;
			int BorderParam ;

			BorderParam = va_arg( ParamList, int ) ;
			BorderRange = va_arg( ParamList, int ) ;

			if( BorderRange > 255 ) BorderRange = 255 ;
			else
			if( BorderRange < 0   ) BorderRange = 0   ;

			if( BorderParam > 255 ) BorderParam = 255 ;
			else
			if( BorderParam < 0	  ) BorderParam = 0   ;
			
			Param[ 0 ] = GSYS.DrawSetting.BlendGraphBorderParam = BorderParam ;
			Param[ 1 ] = GSYS.DrawSetting.BlendGraphBorderRange = BorderRange ;

			// Direct3D を使用していない場合のみ処理を分岐
			if( GSYS.Setting.ValidHardware == FALSE )
			{
				// MEMIMG システムへの設定
				SetBlendGraphParamMemImg( BorderParam, BorderRange ) ;
			}
			break ;

		case DX_BLENDGRAPHTYPE_ALPHA :
			break ;
		}
	}

	// ハードウエアアクセラレーションを使用しているかどうかで処理を分岐
	if( GSYS.Setting.ValidHardware )
	{
		// ハードウエアアクセラレーションを使用している場合の処理
		Graphics_Hardware_SetBlendGraphParamBase_PF( BlendImage, BlendType, Param ) ;
	}
	else
	{
		// ハードウエアアクセラレーションを使用していない場合の処理

		// ブレンドグラフィックのアドレスを設定
		GSYS.SoftRender.BlendMemImg = GSYS.DrawSetting.BlendGraph <= 0 ? NULL : &BlendImage->Orig->Soft.MemImg ;
	}

	// 終了
	return 0 ;
}

// 描画先に正しいα値を書き込むかどうかのフラグを更新する
extern	int Graphics_DrawSetting_RefreshAlphaChDrawMode( void )
{
	return Graphics_Hardware_RefreshAlphaChDrawMode_PF() ;
}

// 基本データに設定されている行列をハードウエアに反映する
extern void Graphics_DrawSetting_ApplyLibMatrixToHardware()
{
	if( GSYS.DrawSetting.MatchHardwareMatrix == TRUE ) return ;

	if( GSYS.DrawSetting.MatchHardwareWorldMatrix == FALSE )
	{
		Graphics_Hardware_DeviceDirect_SetWorldMatrix_PF( &GSYS.DrawSetting.WorldMatrixF ) ;
		GSYS.DrawSetting.MatchHardwareWorldMatrix = TRUE ;
	}

	if( GSYS.DrawSetting.MatchHardwareViewMatrix == FALSE )
	{
		Graphics_Hardware_DeviceDirect_SetViewMatrix_PF( &GSYS.DrawSetting.ViewMatrixF ) ;
		GSYS.DrawSetting.MatchHardwareViewMatrix = TRUE ;
	}

	if( GSYS.DrawSetting.MatchHardwareProjectionMatrix == FALSE )
	{
		Graphics_Hardware_DeviceDirect_SetProjectionMatrix_PF( &GSYS.DrawSetting.ProjectionMatrixF ) ;
		GSYS.DrawSetting.MatchHardwareProjectionMatrix = TRUE ;
	}

	if( GSYS.DrawSetting.MatchHardwareTextureAddressTransformMatrix == FALSE )
	{
		Graphics_Hardware_SetTextureAddressTransformMatrix_PF( GSYS.DrawSetting.TextureAddressTransformUse, &GSYS.DrawSetting.TextureAddressTransformMatrix ) ;
		GSYS.DrawSetting.MatchHardwareTextureAddressTransformMatrix = TRUE ;
	}

	GSYS.DrawSetting.MatchHardwareMatrix = TRUE ;
}

// 基本データに設定されているフォグ情報をハードウェアに反映する
extern void Graphics_DrawSetting_ApplyLibFogToHardware( void )
{
	Graphics_Hardware_ApplyLigFogToHardware_PF() ;
}

// テクスチャ座標変換行列をセットする
extern int Graphics_DrawSetting_SetTextureAddressTransformMatrix_Direct( int Use, MATRIX *Matrix, int Stage )
{
	Graphics_Hardware_SetTextureAddressTransformMatrix_PF( Use, Matrix, Stage ) ;
	GSYS.DrawSetting.MatchHardwareTextureAddressTransformMatrix = FALSE ;
	GSYS.DrawSetting.MatchHardwareMatrix = FALSE ;

	// 終了
	return 0 ;
}

// ローカル→ワールド行列を変更する
extern int Graphics_DrawSetting_SetTransformToWorld_Direct( MATRIX *Matrix )
{
	Graphics_Hardware_DeviceDirect_SetWorldMatrix_PF( Matrix ) ;
	GSYS.DrawSetting.MatchHardwareWorldMatrix = FALSE ;
	GSYS.DrawSetting.MatchHardwareMatrix = FALSE ;

	// 終了
	return 0 ;
}

// 投影変換用行列をセットする
extern	int Graphics_DrawSetting_SetTransformToProjection_Direct( const MATRIX_D *Matrix )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// データを保存
	GSYS.DrawSetting.ProjectionMatrix = *Matrix ;
	ConvertMatrixDtoF( &GSYS.DrawSetting.ProjectionMatrixF, &GSYS.DrawSetting.ProjectionMatrix ) ;
	GSYS.DrawSetting.ValidBlendMatrix = FALSE ;

	// 全ての行列を掛け合わせた行列を更新
	Graphics_DrawSetting_RefreshBlendTransformMatrix() ;

	// ハードウエアアクセラレーションに設定
	if( GSYS.Setting.ValidHardware )
	{
		Graphics_Hardware_SetTransformToProjection_PF( &GSYS.DrawSetting.ProjectionMatrixF ) ;

		GSYS.DrawSetting.MatchHardwareProjectionMatrix = TRUE ;
	}

	// 終了
	return 0 ;
}

// 射影行列パラメータに基づいて射影行列を構築する
extern void Graphics_DrawSetting_RefreshProjectionMatrix( void )
{
	double Aspect = 1.0 ;

	if( GSYS.DrawSetting.ProjectionMatrixMode != 2 )
	{
		Aspect = ( double )GSYS.DrawSetting.DrawSizeY / ( double )GSYS.DrawSetting.DrawSizeX * GSYS.DrawSetting.ProjDotAspect ;
	}

	switch( GSYS.DrawSetting.ProjectionMatrixMode )
	{
	case 0 :	// 遠近法
		CreatePerspectiveFovMatrixD( &GSYS.DrawSetting.ProjMatrix, GSYS.DrawSetting.ProjFov, GSYS.DrawSetting.ProjNear, GSYS.DrawSetting.ProjFar, Aspect ) ;
		break ;

	case 1 :	// 正射影
		CreateOrthoMatrixD( &GSYS.DrawSetting.ProjMatrix, GSYS.DrawSetting.ProjSize, GSYS.DrawSetting.ProjNear, GSYS.DrawSetting.ProjFar, Aspect ) ;
		break ;

	case 2 :	// 行列直指定
		break ;
	}

	// ハードウエアに反映する
	Graphics_DrawSetting_SetTransformToProjection_Direct( &GSYS.DrawSetting.ProjMatrix ) ;
}

// 頂点変換行列を掛け合わせた行列を更新する
extern void Graphics_DrawSetting_RefreshBlendTransformMatrix( void )
{
	if( GSYS.DrawSetting.ValidBlendMatrix == TRUE ) return ;

	CreateMultiplyMatrixD( &GSYS.DrawSetting.WorldViewMatrix, &GSYS.DrawSetting.WorldMatrix, &GSYS.DrawSetting.ViewMatrix ) ;
	CreateMultiplyMatrixD( &GSYS.DrawSetting.ViewProjectionViewportMatrix, &GSYS.DrawSetting.ViewMatrix, &GSYS.DrawSetting.ProjectionMatrix ) ;
	CreateMultiplyMatrixD( &GSYS.DrawSetting.ViewProjectionViewportMatrix, &GSYS.DrawSetting.ViewProjectionViewportMatrix, &GSYS.DrawSetting.ViewportMatrix ) ;
	CreateMultiplyMatrixD( &GSYS.DrawSetting.BlendMatrix, &GSYS.DrawSetting.WorldMatrix, &GSYS.DrawSetting.ViewProjectionViewportMatrix ) ;
	ConvertMatrixDtoF( &GSYS.DrawSetting.BlendMatrixF, &GSYS.DrawSetting.BlendMatrix ) ;

	GSYS.DrawSetting.BillboardMatrix.m[0][0] = GSYS.DrawSetting.ViewMatrix.m[0][0] ;
	GSYS.DrawSetting.BillboardMatrix.m[0][1] = GSYS.DrawSetting.ViewMatrix.m[1][0] ;
	GSYS.DrawSetting.BillboardMatrix.m[0][2] = GSYS.DrawSetting.ViewMatrix.m[2][0] ;
	GSYS.DrawSetting.BillboardMatrix.m[0][3] = 0.0F ;

	GSYS.DrawSetting.BillboardMatrix.m[1][0] = GSYS.DrawSetting.ViewMatrix.m[0][1] ;
	GSYS.DrawSetting.BillboardMatrix.m[1][1] = GSYS.DrawSetting.ViewMatrix.m[1][1] ;
	GSYS.DrawSetting.BillboardMatrix.m[1][2] = GSYS.DrawSetting.ViewMatrix.m[2][1] ;
	GSYS.DrawSetting.BillboardMatrix.m[1][3] = 0.0F ;

	GSYS.DrawSetting.BillboardMatrix.m[2][0] = GSYS.DrawSetting.ViewMatrix.m[0][2] ;
	GSYS.DrawSetting.BillboardMatrix.m[2][1] = GSYS.DrawSetting.ViewMatrix.m[1][2] ;
	GSYS.DrawSetting.BillboardMatrix.m[2][2] = GSYS.DrawSetting.ViewMatrix.m[2][2] ;
	GSYS.DrawSetting.BillboardMatrix.m[2][3] = 0.0F ;

	GSYS.DrawSetting.BillboardMatrix.m[3][0] = 
	GSYS.DrawSetting.BillboardMatrix.m[3][1] = 
	GSYS.DrawSetting.BillboardMatrix.m[3][2] = 
	GSYS.DrawSetting.BillboardMatrix.m[3][3] = 0.0F ;

	ConvertMatrixDtoF( &GSYS.DrawSetting.BillboardMatrixF, &GSYS.DrawSetting.BillboardMatrix ) ;

	GSYS.DrawSetting.ValidInverseBlendMatrix            = FALSE ;
	GSYS.DrawSetting.ValidBlendMatrix                   = TRUE ;
	GSYS.DrawSetting.ValidConvScreenPosToWorldPosMatrix = FALSE ;

	if( GSYS.DrawSetting.DrawArea.left == GSYS.DrawSetting.DrawArea.right ||
		GSYS.DrawSetting.DrawArea.top  == GSYS.DrawSetting.DrawArea.bottom )
	{
		_MEMSET( GSYS.DrawSetting.ViewClipPos,   0, sizeof( GSYS.DrawSetting.ViewClipPos   ) ) ;
		_MEMSET( GSYS.DrawSetting.ClippingPlane, 0, sizeof( GSYS.DrawSetting.ClippingPlane ) ) ;
	}
	else
	{
		GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.left,  GSYS.DrawSetting.DrawAreaF.top,    0.0 ) ) ;
		GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 1 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.right, GSYS.DrawSetting.DrawAreaF.top,    0.0 ) ) ;
		GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 0 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.left,  GSYS.DrawSetting.DrawAreaF.bottom, 0.0 ) ) ;
		GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 1 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.right, GSYS.DrawSetting.DrawAreaF.bottom, 0.0 ) ) ;
		GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 0 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.left,  GSYS.DrawSetting.DrawAreaF.top,    1.0 ) ) ;
		GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 1 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.right, GSYS.DrawSetting.DrawAreaF.top,    1.0 ) ) ;
		GSYS.DrawSetting.ViewClipPos[ 1 ][ 1 ][ 0 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.left,  GSYS.DrawSetting.DrawAreaF.bottom, 1.0 ) ) ;
		GSYS.DrawSetting.ViewClipPos[ 1 ][ 1 ][ 1 ] = NS_ConvScreenPosToWorldPosD( VGetD( GSYS.DrawSetting.DrawAreaF.right, GSYS.DrawSetting.DrawAreaF.bottom, 1.0 ) ) ;

		VECTOR_D Normal ;

		// ＸＺ平面
		Normal = VCrossD( VSubD( GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 0 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ] ), VSubD( GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 0 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ] ) ) ;
		CreateNormalizePlaneD( &GSYS.DrawSetting.ClippingPlane[ 0 ], &GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ], &Normal ) ;

		Normal = VCrossD( VSubD( GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 1 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 1 ] ), VSubD( GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 1 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 1 ] ) ) ;
		CreateNormalizePlaneD( &GSYS.DrawSetting.ClippingPlane[ 1 ], &GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 1 ], &Normal ) ;

		// ＹＺ平面
		Normal = VCrossD( VSubD( GSYS.DrawSetting.ViewClipPos[ 1 ][ 1 ][ 0 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 0 ] ), VSubD( GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 1 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 0 ] ) ) ;
		CreateNormalizePlaneD( &GSYS.DrawSetting.ClippingPlane[ 2 ], &GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 0 ], &Normal ) ;

		Normal = VCrossD( VSubD( GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 1 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ] ), VSubD( GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 0 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ] ) ) ;
		CreateNormalizePlaneD( &GSYS.DrawSetting.ClippingPlane[ 3 ], &GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ], &Normal ) ;

		// ＸＹ平面
		Normal = VCrossD( VSubD( GSYS.DrawSetting.ViewClipPos[ 0 ][ 1 ][ 0 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ] ), VSubD( GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 1 ], GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ] ) ) ;
		CreateNormalizePlaneD( &GSYS.DrawSetting.ClippingPlane[ 4 ], &GSYS.DrawSetting.ViewClipPos[ 0 ][ 0 ][ 0 ], &Normal ) ;

		Normal = VCrossD( VSubD( GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 1 ], GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 0 ] ), VSubD( GSYS.DrawSetting.ViewClipPos[ 1 ][ 1 ][ 0 ], GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 0 ] ) ) ;
		CreateNormalizePlaneD( &GSYS.DrawSetting.ClippingPlane[ 5 ], &GSYS.DrawSetting.ViewClipPos[ 1 ][ 0 ][ 0 ], &Normal ) ;
	}
}

// 使用する射影行列を３Ｄ用にするか２Ｄ用にするかを設定する
//extern void Graphics_DrawSetting_SetUse2DProjectionMatrix( int Use2DProjectionMatrix )
//{
//	if( Use2DProjectionMatrix == GSYS.DrawSetting.UseProjectionMatrix2D )
//	{
//		return ;
//	}
//
//	// 描画待機している描画物を描画
//	DRAWSTOCKINFO
//
//	// フラグを保存
//	GSYS.DrawSetting.UseProjectionMatrix2D = Use2DProjectionMatrix ;
//
//	// ハードウエアアクセラレーションに設定
//	if( GSYS.Setting.ValidHardware )
//	{
//		Graphics_Hardware_SetUse2DProjectionMatrix_PF( Use2DProjectionMatrix ) ;
//	}
//}

// ＤＸライブラリ内部で SetDrawScreen を使用して描画先を変更する際の、元のカメラ設定や描画領域を復元する為の情報を取得する処理を行う
extern int Graphics_DrawSetting_GetScreenDrawSettingInfo( SCREENDRAWSETTINGINFO *ScreenDrawSettingInfo )
{
	// 各種データ保存
	ScreenDrawSettingInfo->Use3DFlag			= NS_GetUse3DFlag() ;
	ScreenDrawSettingInfo->DrawMode				= NS_GetDrawMode() ;
	ScreenDrawSettingInfo->WaitVSync			= NS_GetWaitVSyncFlag() ;
	ScreenDrawSettingInfo->UseSysMemFlag		= NS_GetUseSystemMemGraphCreateFlag() ;
	ScreenDrawSettingInfo->DrawScreen			= NS_GetActiveGraph() ;
	ScreenDrawSettingInfo->ViewMatrix           = GSYS.DrawSetting.ViewMatrix ;
	ScreenDrawSettingInfo->ProjectionMatrixMode = GSYS.DrawSetting.ProjectionMatrixMode ;
	ScreenDrawSettingInfo->ProjFov              = GSYS.DrawSetting.ProjFov ;
	ScreenDrawSettingInfo->ProjSize             = GSYS.DrawSetting.ProjSize ;
	ScreenDrawSettingInfo->ProjMatrix           = GSYS.DrawSetting.ProjMatrix ;
	ScreenDrawSettingInfo->ProjNear             = GSYS.DrawSetting.ProjNear ;
	ScreenDrawSettingInfo->ProjFar              = GSYS.DrawSetting.ProjFar ;
	ScreenDrawSettingInfo->ProjDotAspect        = GSYS.DrawSetting.ProjDotAspect ;
	ScreenDrawSettingInfo->ProjectionMatrix     = GSYS.DrawSetting.ProjectionMatrix ;
	ScreenDrawSettingInfo->ViewportMatrix       = GSYS.DrawSetting.ViewportMatrix ;
	ScreenDrawSettingInfo->WorldMatrix          = GSYS.DrawSetting.WorldMatrix ;
	
	// 描画領域を得る
	NS_GetDrawArea( &ScreenDrawSettingInfo->DrawRect ) ;

	// 正常終了
	return 0 ;
}

// ＤＸライブラリ内部で SetDrawScreen を使用して描画先を変更する際の、元のカメラ設定や描画領域を復元する処理を行う
extern int Graphics_DrawSetting_SetScreenDrawSettingInfo( const SCREENDRAWSETTINGINFO *ScreenDrawSettingInfo )
{
	// 保存しておいた設定を反映させる
	NS_SetDrawMode( ScreenDrawSettingInfo->DrawMode ) ;
	NS_SetWaitVSyncFlag( ScreenDrawSettingInfo->WaitVSync ) ;
	NS_SetDrawScreen( ScreenDrawSettingInfo->DrawScreen ) ;
	NS_SetUse3DFlag( ScreenDrawSettingInfo->Use3DFlag ) ;
	NS_SetUseSystemMemGraphCreateFlag( ScreenDrawSettingInfo->UseSysMemFlag ) ;
	NS_SetDrawArea( ScreenDrawSettingInfo->DrawRect.left, ScreenDrawSettingInfo->DrawRect.top, ScreenDrawSettingInfo->DrawRect.right, ScreenDrawSettingInfo->DrawRect.bottom ) ;

	NS_SetTransformToWorldD( &ScreenDrawSettingInfo->WorldMatrix );
	NS_SetTransformToViewD( &ScreenDrawSettingInfo->ViewMatrix );
	switch( ScreenDrawSettingInfo->ProjectionMatrixMode )
	{
	case 0 :	// 遠近法
		NS_SetupCamera_PerspectiveD( ScreenDrawSettingInfo->ProjFov ) ;
		break ;

	case 1 :	// 正射影
		NS_SetupCamera_OrthoD( ScreenDrawSettingInfo->ProjSize ) ;
		break ;

	case 2 :	// 行列直指定
		NS_SetupCamera_ProjectionMatrixD( ScreenDrawSettingInfo->ProjMatrix ) ;
		break ;
	}
	NS_SetCameraNearFarD( ScreenDrawSettingInfo->ProjNear, ScreenDrawSettingInfo->ProjFar ) ;
	NS_SetCameraDotAspectD( ScreenDrawSettingInfo->ProjDotAspect ) ;
//	NS_SetTransformToProjection( &ScreenDrawSettingInfo->ProjectionMatrix );
	NS_SetTransformToViewportD( &ScreenDrawSettingInfo->ViewportMatrix );

	// 正常終了
	return 0 ;
}
























// 描画関係関数

#define DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK									\
	if( y1 >= DrawRect.top && y1 < DrawRect.bottom )							\
	{																			\
		if( ( CirclePos[ y1 ][ 4 ] & 1 ) == 0 || CirclePos[ y1 ][ 0 ] > x1 )	\
		{																		\
			CirclePos[ y1 ][ 4 ] |= 1 ;											\
			CirclePos[ y1 ][ 0 ] = ( short )x1 ;								\
		}																		\
	}

#define DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK									\
	if( y1 >= DrawRect.top && y1 < DrawRect.bottom )							\
	{																			\
		if( ( CirclePos[ y1 ][ 4 ] & 2 ) == 0 || CirclePos[ y1 ][ 1 ] < x1 )	\
		{																		\
			CirclePos[ y1 ][ 4 ] |= 2 ;											\
			CirclePos[ y1 ][ 1 ] = ( short )x1 ;								\
		}																		\
	}

#define DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK									\
	if( y1 >= DrawRect.top && y1 < DrawRect.bottom )							\
	{																			\
		if( ( CirclePos[ y1 ][ 4 ] & 4 ) == 0 || CirclePos[ y1 ][ 2 ] < x1 )	\
		{																		\
			CirclePos[ y1 ][ 4 ] |= 4 ;											\
			CirclePos[ y1 ][ 2 ] = ( short )x1 ;								\
		}																		\
	}

#define DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK									\
	if( y1 >= DrawRect.top && y1 < DrawRect.bottom )							\
	{																			\
		if( ( CirclePos[ y1 ][ 4 ] & 8 ) == 0 || CirclePos[ y1 ][ 3 ] > x1 )	\
		{																		\
			CirclePos[ y1 ][ 4 ] |= 8 ;											\
			CirclePos[ y1 ][ 3 ] = ( short )x1 ;								\
		}																		\
	}

// 線の幅付き円の描画用頂点を取得する関数
extern int	Graphics_Draw_GetCircle_ThicknessDrawPosition( int x, int y, int r, int Thickness, short ( *CirclePos )[ 5 ] )
{
	int rmax, rmin ;
	int x1, y1 ;
	int Dx, Dy, F ;
	RECT DrawRect ;

	DrawRect = GSYS.DrawSetting.DrawArea ;

	// 各線の太さを算出
	rmax = r + Thickness / 2 ;
	rmin = r - Thickness / 2 ;
	if( rmax - rmin + 1 > Thickness )
	{
		rmin ++ ;
	}

	// 外側円の座標リスト作成
	{
		// 初期値セット
		Dx = rmax ; Dy = 0 ; F = -2 * rmax + 3 ;

		// 最初の点
		{
			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
			x1 = Dx + x ;  
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

			x1 = Dy + x ; y1 = Dx + y ;
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

			x1 = Dy + x ; y1 = -Dx + y;
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

			// 座標データを進める
			if( F >= 0 )
			{
				Dx -- ;
				F -= 4 * Dx ;
			}	

			Dy ++ ;
			F += 4 * Dy + 2 ;
		}

		while( Dx >= Dy )
		{
			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
			x1 = Dx + x ;  
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

			x1 = Dy + x ; y1 = Dx + y ;
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;
			x1 = -Dy + x ; 
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
		
			x1 = Dx + x ; y1 = -Dy + y;
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;
			x1 = -Dx + x ; 
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;

			x1 = Dy + x ; y1 = -Dx + y;
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;
			x1 = -Dy + x ; 
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;

			// 座標データを進める
			if( F >= 0 )
			{
				Dx -- ;
				F -= 4 * Dx ;
			}

			Dy ++ ;
			F += 4 * Dy + 2 ;
		}
	}

	// 内側円の座標リスト作成
	{
		// 初期値セット
		Dx = rmin ; Dy = 0 ; F = -2 * rmin + 3 ;

		// 最初の点
		{
			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
			x1 = Dx + x ;  
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

			x1 = Dy + x ; y1 = Dx + y ;
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

			x1 = Dy + x ; y1 = -Dx + y;
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

			// 座標データを進める
			if( F >= 0 )
			{
				Dx -- ;
				F -= 4 * Dx ;
			}	

			Dy ++ ;
			F += 4 * Dy + 2 ;
		}

		while( Dx >= Dy )
		{
			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
			x1 = Dx + x ;  
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

			x1 = Dy + x ; y1 = Dx + y ;
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;
			x1 = -Dy + x ; 
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
		
			x1 = Dx + x ; y1 = -Dy + y;
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;
			x1 = -Dx + x ; 
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;

			x1 = Dy + x ; y1 = -Dx + y;
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;
			x1 = -Dy + x ; 
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;

			// 座標データを進める
			if( F >= 0 )
			{
				Dx -- ;
				F -= 4 * Dx ;
			}

			Dy ++ ;
			F += 4 * Dy + 2 ;
		}
	}

	// 終了
	return 0 ;
}

// 線の幅付き楕円の描画用頂点を取得する関数
extern int	Graphics_Draw_GetOval_ThicknessDrawPosition( int x, int y, int rx, int ry, int Thickness, short ( *CirclePos )[ 5 ] )
{
	int x1, y1 ;
	int Dx, Dy, F, H, Df, yc, xc, rc, r ;
	int rxmax, rymax ;
	int rxmin, rymin ;
	RECT DrawRect ;

	DrawRect = GSYS.DrawSetting.DrawArea ;

	// 各線の太さを算出
	rxmax = rx + Thickness / 2 ;
	rxmin = rx - Thickness / 2 ;
	if( rxmax - rxmin + 1 > Thickness )
	{
		rxmin ++ ;
	}
	rymax = ry + Thickness / 2 ;
	rymin = ry - Thickness / 2 ;
	if( rymax - rymin + 1 > Thickness )
	{
		rymin ++ ;
	}

	// 外側円の座標リスト作成
	{
		if( rxmax >= rymax )
		{
			// 初期値をセット
			rc = ( rxmax * rxmax ) / rymax ;

			xc = 0 ; yc = 0 ;
			r = rxmax ;
			Df = 2 ;
	
			Dx = r ;
			Dy = 0;
			F = -2 * r + 1 + 2 * 1;
			H = -4 * r + 2 + 1;

			// 最初の点を描く
			{
				x1 = -Dx + x ; y1 = Dy + y ;
				DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
				x1 = Dx + x ;  
				DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					yc += rymax ;
					if( yc >= rc )
					{
						Dy ++ ;
						yc -= rc ;
						Df = 1 ;
					}
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					Df = 1 ;

					if( H>=0 )
					{
						Dx--;
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Dx--;	
						yc += rymax ;
						if( yc >= rc )
						{
							Dy ++ ;
							yc -= rc ;
							Df = 1 ;
						}
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			while( Dx > 0 )
			{
				if( Df )
				{
					x1 = -Dx + x ; y1 = Dy + y ;
					DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

					x1 = -Dx + x ; y1 = -Dy + y ;
					DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;
				}

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					yc += rymax ;
					if( yc >= rc )
					{
						Dy ++ ;
						yc -= rc ;
						Df = 1 ;
					}
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					Df = 1 ;

					if( H>=0 )
					{
						Dx--;
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Dx--;	
						yc += rymax ;
						if( yc >= rc )
						{
							Dy ++ ;
							yc -= rc ;
							Df = 1 ;
						}
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			// 最後の点を描く
			x1 = Dx + x ; y1 = -Dy + y;
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
		}
		else
		{
			// 初期値をセット
			rc = ( rymax * rymax ) / rxmax ;

			xc = 0 ; yc = 0 ;
			r = rxmax ;
			Df = 2 ;
	
			Dx = r ;
			Dy = 0;
			F = -2 * r + 1 + 2 * 1;
			H = -4 * r + 2 + 1;

			// 最初の点を描く
			{
				x1 = -Dx + x ; y1 = Dy + y ;
				DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
				x1 = Dx + x ;  
				DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					Df = 1 ;

					Dy ++ ;
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					if( H>=0 )
					{
						xc += rxmax ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
							Df = 1 ;
						}
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Df = 1 ;

						xc += rxmax ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
						}

						Dy ++ ;
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			while( Dx > 0 )
			{
				if( Df )
				{
					x1 = -Dx + x ; y1 = Dy + y ;
					DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

					x1 = -Dx + x ; y1 = -Dy + y ;
					DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;
				}

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					Df = 1 ;

					Dy ++ ;
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					if( H>=0 )
					{
						xc += rxmax ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
							Df = 1 ;
						}
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Df = 1 ;

						xc += rxmax ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
						}

						Dy ++ ;
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			// 最後の点を描く
			x1 = Dx + x ; y1 = -Dy + y;
			DX_CIRCLE_THICKNESS_R_MAX_RIGHT_CHECK ;

			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MAX_LEFT_CHECK ;
		}
	}

	// 内側円の座標リスト作成
	{
		if( rxmin >= rymin )
		{
			// 初期値をセット
			rc = ( rxmin * rxmin ) / rymin ;

			xc = 0 ; yc = 0 ;
			r = rxmin ;
			Df = 2 ;
	
			Dx = r ;
			Dy = 0;
			F = -2 * r + 1 + 2 * 1;
			H = -4 * r + 2 + 1;

			// 最初の点を描く
			{
				x1 = -Dx + x ; y1 = Dy + y ;
				DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
				x1 = Dx + x ;  
				DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					yc += rymin ;
					if( yc >= rc )
					{
						Dy ++ ;
						yc -= rc ;
						Df = 1 ;
					}
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					Df = 1 ;

					if( H>=0 )
					{
						Dx--;
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Dx--;	
						yc += rymin ;
						if( yc >= rc )
						{
							Dy ++ ;
							yc -= rc ;
							Df = 1 ;
						}
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			while( Dx > 0 )
			{
				if( Df )
				{
					x1 = -Dx + x ; y1 = Dy + y ;
					DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

					x1 = -Dx + x ; y1 = -Dy + y ;
					DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;
				}

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					yc += rymin ;
					if( yc >= rc )
					{
						Dy ++ ;
						yc -= rc ;
						Df = 1 ;
					}
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					Df = 1 ;

					if( H>=0 )
					{
						Dx--;
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Dx--;	
						yc += rymin ;
						if( yc >= rc )
						{
							Dy ++ ;
							yc -= rc ;
							Df = 1 ;
						}
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			// 最後の点を描く
			x1 = Dx + x ; y1 = -Dy + y;
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
		}
		else
		{
			// 初期値をセット
			rc = ( rymin * rymin ) / rxmin ;

			xc = 0 ; yc = 0 ;
			r = rxmin ;
			Df = 2 ;
	
			Dx = r ;
			Dy = 0;
			F = -2 * r + 1 + 2 * 1;
			H = -4 * r + 2 + 1;

			// 最初の点を描く
			{
				x1 = -Dx + x ; y1 = Dy + y ;
				DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
				x1 = Dx + x ;  
				DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					Df = 1 ;

					Dy ++ ;
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					if( H>=0 )
					{
						xc += rxmin ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
							Df = 1 ;
						}
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Df = 1 ;

						xc += rxmin ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
						}

						Dy ++ ;
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			while( Dx > 0 )
			{
				if( Df )
				{
					x1 = -Dx + x ; y1 = Dy + y ;
					DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

					x1 = -Dx + x ; y1 = -Dy + y ;
					DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
					x1 = Dx + x ;  
					DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;
				}

				Df = 0 ;

				// 座標データを進める
				if( F < 0 )
				{
					Df = 1 ;

					Dy ++ ;
					F += 4 * Dy + 2 ;
					H += 4 * Dy;
				}
				else
				{
					if( H>=0 )
					{
						xc += rxmin ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
							Df = 1 ;
						}
						F -= 4 * Dx;
						H -= 4 * Dx - 2 ;
					}
					else
					{
						Df = 1 ;

						xc += rxmin ;
						if( xc >= rc )
						{
							Dx--;
							xc -= rc ;
						}

						Dy ++ ;
						F += 4 * Dy - 4 * Dx + 2 ;
						H += 4 * Dy - 4 * Dx + 2 ;
					}
				}
			}

			// 最後の点を描く
			x1 = Dx + x ; y1 = -Dy + y;
			DX_CIRCLE_THICKNESS_R_MIN_RIGHT_CHECK ;

			x1 = -Dx + x ; y1 = Dy + y ;
			DX_CIRCLE_THICKNESS_R_MIN_LEFT_CHECK ;
		}
	}

	// 終了
	return 0 ;
}

// 座標補正を行わない２ポリゴン描画を行う( １テクスチャ画像のみ有効 )

#define SETDRAWRECTCODE\
	int x[ 4 ], y[ 4 ], MaxX, MaxY, MinX, MinY ;\
	int i, ix, iy ;\
\
	x[ 0 ] = _FTOL( Param->Vertex[ 0 ].x ) ; x[ 1 ] = _FTOL( Param->Vertex[ 1 ].x ) ; x[ 2 ] = _FTOL( Param->Vertex[ 2 ].x ) ; x[ 3 ] = _FTOL( Param->Vertex[ 3 ].x ) ;\
	y[ 0 ] = _FTOL( Param->Vertex[ 0 ].y ) ; y[ 1 ] = _FTOL( Param->Vertex[ 1 ].y ) ; y[ 2 ] = _FTOL( Param->Vertex[ 2 ].y ) ; y[ 3 ] = _FTOL( Param->Vertex[ 3 ].y ) ;\
\
	MaxX = 0 ; MaxY = 0 ;\
	MinX = 0xfffffff ; MinY = 0xfffffff ;\
\
	for( i = 0 ; i < 4 ; i ++ )\
	{\
		ix = x[ i ] ; iy = y[ i ] ;\
		if( ix > MaxX ) MaxX = ix ; if( iy > MaxY ) MaxY = iy ;\
		if( ix < MinX ) MinX = ix ; if( iy < MinY ) MinY = iy ;\
	}\
\
	SETRECT( DrawRect, MinX, MinY, MaxX, MaxY ) ;\

extern int Graphics_Draw_DrawSimpleTwoTriangleGraphF( const GRAPHICS_DRAW_DRAWSIMPLETWOTRIANGLEGRAPHF_PARAM *Param )
{
	IMAGEDATA *Image ;
	IMAGEDATA *BlendImage ;
	int Ret ;

	// 初期化判定
	if( DxSysData.DxLib_InitializeFlag == FALSE )
	{
		return -1 ;
	}

	// 非描画設定がされている場合は何もせず終了
	if( /* GSYS.NotDrawFlag || */ GSYS.DrawSetting.NotDrawFlagInSetDrawArea )
	{
		return 0 ;
	}

	// ソフトが非アクティブの場合はアクティブになるまで待つ
	CheckActiveState() ;

	// エラー判定
	if( GRAPHCHK( Param->GraphHandle, Image ) )
	{
		return -1 ;
	}
	BlendImage = NULL ;
	if( GSYS.DrawSetting.BlendGraph > 0 )
	{
		GRAPHCHK( GSYS.DrawSetting.BlendGraph, BlendImage ) ;

		// ブレンドテクスチャを使用する場合、描画画像よりも小さかったらエラー
		if( Graphics_Image_CheckBlendGraphSize( Image ) )
		{
			return -1 ;
		}
	}

#ifndef DX_NON_MOVIE
	if( Image->MovieHandle != -1 )
	{
		UpdateMovie( Image->MovieHandle, FALSE ) ;
	}
#endif

	// 描画
	DRAW_DEF( 
		Graphics_Hardware_DrawSimpleTwoTriangleGraphF_PF( Param, Image, BlendImage ),
		0,
		SETDRAWRECTCODE,
		Ret,
		Image->Orig->FormatDesc.TextureFlag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE
























// カメラ関係関数

// ８座標で形作るボックスがカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )( CheckPosは VECTOR 8個分の配列の先頭アドレス、配列の各要素番号の内容 0:+x +y +z   1:-x +y +z   2:-x -y +z   3:+x -y +z   4:+x -y -z   5:+x +y -z   6:-x +y -z   7:-x -y -z )
extern int Graphics_Camera_CheckCameraViewClip_Box_PosDim( VECTOR *CheckBoxPos )
{
	BYTE clip[ 8 ] ;
	VECTOR *pos ;
	int i ;

	// 各頂点が可視領域に入っているかどうかを調べる
	pos = CheckBoxPos ;
	for( i = 0 ; i < 8 ; i ++, pos ++ )
	{
		clip[ i ] = 0 ;
		if( GSYS.DrawSetting.ClippingPlane[ 0 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 0 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 0 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 0 ].w < 0.0f ) clip[ i ] |= CLIP_XM ;
		if( GSYS.DrawSetting.ClippingPlane[ 1 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 1 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 1 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 1 ].w < 0.0f ) clip[ i ] |= CLIP_XP ;
		if( GSYS.DrawSetting.ClippingPlane[ 2 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 2 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 2 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 2 ].w < 0.0f ) clip[ i ] |= CLIP_YM ;
		if( GSYS.DrawSetting.ClippingPlane[ 3 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 3 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 3 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 3 ].w < 0.0f ) clip[ i ] |= CLIP_YP ;
		if( GSYS.DrawSetting.ClippingPlane[ 4 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 4 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 4 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 4 ].w < 0.0f ) clip[ i ] |= CLIP_ZM ;
		if( GSYS.DrawSetting.ClippingPlane[ 5 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 5 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 5 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 5 ].w < 0.0f ) clip[ i ] |= CLIP_ZP ;
		if( clip[ i ] == 0 ) return FALSE ;
	}

	// どれかの側に全ての頂点が寄っている場合は１００％可視領域に入っていない
	if( ( ( clip[0] & CLIP_XP ) && ( clip[1] & CLIP_XP ) &&
		  ( clip[2] & CLIP_XP ) && ( clip[3] & CLIP_XP ) &&
		  ( clip[4] & CLIP_XP ) && ( clip[5] & CLIP_XP ) &&
		  ( clip[6] & CLIP_XP ) && ( clip[7] & CLIP_XP ) ) ||
		( ( clip[0] & CLIP_XM ) && ( clip[1] & CLIP_XM ) &&
		  ( clip[2] & CLIP_XM ) && ( clip[3] & CLIP_XM ) &&
		  ( clip[4] & CLIP_XM ) && ( clip[5] & CLIP_XM ) &&
		  ( clip[6] & CLIP_XM ) && ( clip[7] & CLIP_XM ) ) ||
		( ( clip[0] & CLIP_YP ) && ( clip[1] & CLIP_YP ) &&
		  ( clip[2] & CLIP_YP ) && ( clip[3] & CLIP_YP ) &&
		  ( clip[4] & CLIP_YP ) && ( clip[5] & CLIP_YP ) &&
		  ( clip[6] & CLIP_YP ) && ( clip[7] & CLIP_YP ) ) ||
		( ( clip[0] & CLIP_YM ) && ( clip[1] & CLIP_YM ) &&
		  ( clip[2] & CLIP_YM ) && ( clip[3] & CLIP_YM ) &&
		  ( clip[4] & CLIP_YM ) && ( clip[5] & CLIP_YM ) &&
		  ( clip[6] & CLIP_YM ) && ( clip[7] & CLIP_YM ) ) ||
		( ( clip[0] & CLIP_ZP ) && ( clip[1] & CLIP_ZP ) &&
		  ( clip[2] & CLIP_ZP ) && ( clip[3] & CLIP_ZP ) &&
		  ( clip[4] & CLIP_ZP ) && ( clip[5] & CLIP_ZP ) &&
		  ( clip[6] & CLIP_ZP ) && ( clip[7] & CLIP_ZP ) ) ||
		( ( clip[0] & CLIP_ZM ) && ( clip[1] & CLIP_ZM ) &&
		  ( clip[2] & CLIP_ZM ) && ( clip[3] & CLIP_ZM ) &&
		  ( clip[4] & CLIP_ZM ) && ( clip[5] & CLIP_ZM ) &&
		  ( clip[6] & CLIP_ZM ) && ( clip[7] & CLIP_ZM ) ) )
		  return TRUE ;

	// ここにきたら見えているということ
	return FALSE ;
}

// ８座標で形作るボックスがカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )( CheckPosは VECTOR 8個分の配列の先頭アドレス、配列の各要素番号の内容 0:+x +y +z   1:-x +y +z   2:-x -y +z   3:+x -y +z   4:+x -y -z   5:+x +y -z   6:-x +y -z   7:-x -y -z )
extern int Graphics_Camera_CheckCameraViewClip_Box_PosDimD( VECTOR_D *CheckBoxPos )
{
	BYTE clip[ 8 ] ;
	VECTOR_D *pos ;
	int i ;

	// 各頂点が可視領域に入っているかどうかを調べる
	pos = CheckBoxPos ;
	for( i = 0 ; i < 8 ; i ++, pos ++ )
	{
		clip[ i ] = 0 ;
		if( GSYS.DrawSetting.ClippingPlane[ 0 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 0 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 0 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 0 ].w < 0.0 ) clip[ i ] |= CLIP_XM ;
		if( GSYS.DrawSetting.ClippingPlane[ 1 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 1 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 1 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 1 ].w < 0.0 ) clip[ i ] |= CLIP_XP ;
		if( GSYS.DrawSetting.ClippingPlane[ 2 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 2 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 2 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 2 ].w < 0.0 ) clip[ i ] |= CLIP_YM ;
		if( GSYS.DrawSetting.ClippingPlane[ 3 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 3 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 3 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 3 ].w < 0.0 ) clip[ i ] |= CLIP_YP ;
		if( GSYS.DrawSetting.ClippingPlane[ 4 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 4 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 4 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 4 ].w < 0.0 ) clip[ i ] |= CLIP_ZM ;
		if( GSYS.DrawSetting.ClippingPlane[ 5 ].x * pos->x + GSYS.DrawSetting.ClippingPlane[ 5 ].y * pos->y + GSYS.DrawSetting.ClippingPlane[ 5 ].z * pos->z + GSYS.DrawSetting.ClippingPlane[ 5 ].w < 0.0 ) clip[ i ] |= CLIP_ZP ;
		if( clip[ i ] == 0 ) return FALSE ;
	}

	// どれかの側に全ての頂点が寄っている場合は１００％可視領域に入っていない
	if( ( ( clip[0] & CLIP_XP ) && ( clip[1] & CLIP_XP ) &&
		  ( clip[2] & CLIP_XP ) && ( clip[3] & CLIP_XP ) &&
		  ( clip[4] & CLIP_XP ) && ( clip[5] & CLIP_XP ) &&
		  ( clip[6] & CLIP_XP ) && ( clip[7] & CLIP_XP ) ) ||
		( ( clip[0] & CLIP_XM ) && ( clip[1] & CLIP_XM ) &&
		  ( clip[2] & CLIP_XM ) && ( clip[3] & CLIP_XM ) &&
		  ( clip[4] & CLIP_XM ) && ( clip[5] & CLIP_XM ) &&
		  ( clip[6] & CLIP_XM ) && ( clip[7] & CLIP_XM ) ) ||
		( ( clip[0] & CLIP_YP ) && ( clip[1] & CLIP_YP ) &&
		  ( clip[2] & CLIP_YP ) && ( clip[3] & CLIP_YP ) &&
		  ( clip[4] & CLIP_YP ) && ( clip[5] & CLIP_YP ) &&
		  ( clip[6] & CLIP_YP ) && ( clip[7] & CLIP_YP ) ) ||
		( ( clip[0] & CLIP_YM ) && ( clip[1] & CLIP_YM ) &&
		  ( clip[2] & CLIP_YM ) && ( clip[3] & CLIP_YM ) &&
		  ( clip[4] & CLIP_YM ) && ( clip[5] & CLIP_YM ) &&
		  ( clip[6] & CLIP_YM ) && ( clip[7] & CLIP_YM ) ) ||
		( ( clip[0] & CLIP_ZP ) && ( clip[1] & CLIP_ZP ) &&
		  ( clip[2] & CLIP_ZP ) && ( clip[3] & CLIP_ZP ) &&
		  ( clip[4] & CLIP_ZP ) && ( clip[5] & CLIP_ZP ) &&
		  ( clip[6] & CLIP_ZP ) && ( clip[7] & CLIP_ZP ) ) ||
		( ( clip[0] & CLIP_ZM ) && ( clip[1] & CLIP_ZM ) &&
		  ( clip[2] & CLIP_ZM ) && ( clip[3] & CLIP_ZM ) &&
		  ( clip[4] & CLIP_ZM ) && ( clip[5] & CLIP_ZM ) &&
		  ( clip[6] & CLIP_ZM ) && ( clip[7] & CLIP_ZM ) ) )
		  return TRUE ;

	// ここにきたら見えているということ
	return FALSE ;
}

// ビュー行列から水平、垂直、捻り角度を算出する
extern void Graphics_Camera_CalcCameraRollViewMatrix( void )
{
	VECTOR_D XVec, YVec, ZVec ;
	double Sin, Cos, f ;

	XVec.x = GSYS.DrawSetting.ViewMatrix.m[ 0 ][ 0 ] ;
	XVec.y = GSYS.DrawSetting.ViewMatrix.m[ 1 ][ 0 ] ;
	XVec.z = GSYS.DrawSetting.ViewMatrix.m[ 2 ][ 0 ] ;

	YVec.x = GSYS.DrawSetting.ViewMatrix.m[ 0 ][ 1 ] ;
	YVec.y = GSYS.DrawSetting.ViewMatrix.m[ 1 ][ 1 ] ;
	YVec.z = GSYS.DrawSetting.ViewMatrix.m[ 2 ][ 1 ] ;

	ZVec.x = GSYS.DrawSetting.ViewMatrix.m[ 0 ][ 2 ] ;
	ZVec.y = GSYS.DrawSetting.ViewMatrix.m[ 1 ][ 2 ] ;
	ZVec.z = GSYS.DrawSetting.ViewMatrix.m[ 2 ][ 2 ] ;

	if( ZVec.x < 0.00000001 && ZVec.x > -0.00000001 &&
		ZVec.z < 0.00000001 && ZVec.z > -0.00000001 )
	{
		GSYS.Camera.HRotate = 0.0 ;
		if( ZVec.y < 0.0 )
		{
			GSYS.Camera.VRotate =  DX_PI / 2.0 ;
			GSYS.Camera.TRotate = _ATAN2D( -YVec.x, YVec.z ) ;
		}
		else
		{
			GSYS.Camera.VRotate = -DX_PI / 2.0 ;
			GSYS.Camera.TRotate = _ATAN2D( -YVec.x, -YVec.z ) ;
		}
	}
	else
	{
		if( YVec.y < 0.0 )
		{
			GSYS.Camera.HRotate = _ATAN2D( -ZVec.x, -ZVec.z ) ;

			_SINCOSD( -GSYS.Camera.HRotate, &Sin, &Cos ) ;
			f      = XVec.z * Cos - XVec.x * Sin ;
			XVec.x = XVec.z * Sin + XVec.x * Cos ;
			XVec.z = f ;

			f      = ZVec.z * Cos - ZVec.x * Sin ;
			ZVec.x = ZVec.z * Sin + ZVec.x * Cos ;
			ZVec.z = f ;

			if( ZVec.y > 0.0 )
			{
				GSYS.Camera.VRotate = -_ATAN2D( -ZVec.z,  ZVec.y ) - DX_PI / 2.0 ;
			}
			else
			{
				GSYS.Camera.VRotate =  _ATAN2D( -ZVec.z, -ZVec.y ) + DX_PI / 2.0 ;
			}

			_SINCOSD( -GSYS.Camera.VRotate, &Sin, &Cos ) ;
			f      = XVec.y * Cos - XVec.z * Sin ;
			XVec.z = XVec.y * Sin + XVec.z * Cos ;
			XVec.y = f ;

			f      = ZVec.y * Cos - ZVec.z * Sin ;
			ZVec.z = ZVec.y * Sin + ZVec.z * Cos ;
			ZVec.y = f ;

			GSYS.Camera.TRotate = _ATAN2D( XVec.y, XVec.x ) ;
		}
		else
		{
			GSYS.Camera.HRotate = _ATAN2D( ZVec.x, ZVec.z ) ;

			_SINCOSD( -GSYS.Camera.HRotate, &Sin, &Cos ) ;
			f      = XVec.z * Cos - XVec.x * Sin ;
			XVec.x = XVec.z * Sin + XVec.x * Cos ;
			XVec.z = f ;

			f      = ZVec.z * Cos - ZVec.x * Sin ;
			ZVec.x = ZVec.z * Sin + ZVec.x * Cos ;
			ZVec.z = f ;

			GSYS.Camera.VRotate = _ATAN2D( -ZVec.y, ZVec.z ) ;

			_SINCOSD( -GSYS.Camera.VRotate, &Sin, &Cos ) ;
			f      = XVec.y * Cos - XVec.z * Sin ;
			XVec.z = XVec.y * Sin + XVec.z * Cos ;
			XVec.y = f ;

			f      = ZVec.y * Cos - ZVec.z * Sin ;
			ZVec.z = ZVec.y * Sin + ZVec.z * Cos ;
			ZVec.y = f ;

			GSYS.Camera.TRotate = _ATAN2D( XVec.y, XVec.x ) ;
		}
	}

//	NS_DrawFormatString( 0, 20, NS_GetColor( 255,255,255 ), "H:%.3f V:%.3f T:%.3f", GSYS.Camera.HRotate, GSYS.Camera.VRotate, GSYS.Camera.TRotate ) ;

}






















// ライト関係関数

// ライトハンドルの追加
extern int Graphics_Light_AddHandle( void )
{
	LIGHT_HANDLE *pLH, **ppLH ;
	int i ;

	// 使用されていないライトハンドルを探す
	if( GSYS.Light.Area != MAX_LIGHT_NUM )
	{
		ppLH = &GSYS.Light.Data[ GSYS.Light.Area ] ;
		i = GSYS.Light.Area ;
	}
	else
	{
		ppLH = GSYS.Light.Data ;
		for( i = 0 ; *ppLH != NULL ; i ++, ppLH ++ ){}
	}

	// データ領域を確保する
	if( ( pLH = *ppLH = ( LIGHT_HANDLE * )DXCALLOC( sizeof( LIGHT_HANDLE ) ) ) == NULL )
		return DxLib_ErrorUTF16LE( "\xe9\x30\xa4\x30\xc8\x30\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"ライト用のメモリの確保に失敗しました" @*/ ) ;

	// ゼロ初期化
	_MEMSET( pLH, 0, sizeof( LIGHT_HANDLE ) ) ;

	// 初期値をセット
	pLH->Param.Ambient.r = 0.33f ;
	pLH->Param.Ambient.g = 0.33f ;
	pLH->Param.Ambient.b = 0.33f ;
	pLH->Param.Ambient.a = 0.33f ;
	pLH->Param.Diffuse.r = 1.00f ;
	pLH->Param.Diffuse.g = 1.00f ;
	pLH->Param.Diffuse.b = 1.00f ;
	pLH->Param.Diffuse.a = 1.00f ;
	pLH->Param.Specular.r = 1.00f ;
	pLH->Param.Specular.g = 1.00f ;
	pLH->Param.Specular.b = 1.00f ;
	pLH->Param.Specular.a = 1.00f ;
	pLH->Param.Attenuation0 = 0.1f ;
	pLH->Param.Attenuation1 = 0.0f ;
	pLH->Param.Attenuation2 = 0.0f ;
	pLH->Param.Falloff = 1.0f ;
	pLH->Param.Theta = 2.0f ;
	pLH->Param.Phi = 2.0f ;
	pLH->Param.Range = 2000.0f ;
	pLH->EnableFlag = FALSE ;
	pLH->SetHardwareIndex = -1 ;
	pLH->HardwareChangeFlag = FALSE ;

	// エラーチェック用ＩＤの設定
	pLH->ID = ( unsigned int )GSYS.Light.HandleID ;
	GSYS.Light.HandleID ++ ;
	if( GSYS.Light.HandleID >= ( DX_HANDLECHECK_MASK >> DX_HANDLECHECK_ADDRESS ) )
		GSYS.Light.HandleID = 0 ;

	// ハンドル値をセット
	pLH->Handle = ( int )( i | DX_HANDLETYPE_MASK_LIGHT | ( pLH->ID << DX_HANDLECHECK_ADDRESS ) ) ;

	// ライトの数を増やす
	GSYS.Light.Num ++ ;

	// 使用されているライトハンドルが存在する範囲を更新する
	if( GSYS.Light.Area == i ) GSYS.Light.Area ++ ;

	// 有効にする
	NS_SetLightEnableHandle( pLH->Handle, TRUE ) ;

	// ハンドルを返す
	return pLH->Handle ;
}

// ライトの変更を反映する
extern int Graphics_Light_RefreshState( void )
{
	int i, j, k, OldEnableNum, OldMaxIndex ;
	LIGHT_HANDLE *pLH ;

	// ライトの設定を反映する
	Graphics_Hardware_Light_SetUse_PF( GSYS.Light.ProcessDisable ? FALSE : TRUE ) ;

	// 設定が変更されていなかったら何もしない
	if( GSYS.Light.HardwareChange == FALSE && GSYS.Light.HardwareRefresh == FALSE )
	{
		return 0 ;
	}

	// 有効なライトを順番に反映する
	OldEnableNum                      = GSYS.Light.EnableNum ;
	OldMaxIndex                       = GSYS.Light.MaxHardwareEnableIndex ;
	GSYS.Light.EnableNum              = 0 ;
	GSYS.Light.MaxHardwareEnableIndex = -1 ;
	for( k = 3 ; k >= 1 ; k -- )
	{
		for( i = 0, j = 0 ; i < GSYS.Light.Num ; j ++ )
		{
			if( GSYS.Light.Data[ j ] == NULL )
			{
				continue ;
			}
			i ++ ;

			pLH = GSYS.Light.Data[ j ] ;
			if( pLH->EnableFlag == FALSE || pLH->Param.LightType != k )
			{
				continue ;
			}

			if( MAX_HARDWAREENABLELIGHTINDEX_NUM > GSYS.Light.EnableNum )
			{
				GSYS.Light.EnableHardwareIndex[ GSYS.Light.EnableNum ] = j ;
			}

			if( pLH->SetHardwareIndex != GSYS.Light.EnableNum || pLH->HardwareChangeFlag == TRUE || GSYS.Light.HardwareRefresh == TRUE )
			{
				pLH->SetHardwareIndex   = GSYS.Light.EnableNum ;
				pLH->HardwareChangeFlag = FALSE ;
//				D_SetLightParam( pLH->SetHardwareIndex, pLH->EnableFlag, &pLH->Param ) ;
				if( pLH->EnableFlag )
				{
					Graphics_Hardware_Light_SetState_PF( pLH->SetHardwareIndex, &pLH->Param ) ;
				}
				Graphics_Hardware_Light_SetEnable_PF( pLH->SetHardwareIndex, pLH->EnableFlag ) ;
			}
			GSYS.Light.MaxHardwareEnableIndex = GSYS.Light.EnableNum ;
			GSYS.Light.EnableNum ++ ;
		}
	}

	// 前回より有効なライトの数が少ない場合は有効なライトをOFFにする
	for( i = GSYS.Light.MaxHardwareEnableIndex + 1 ; i <= OldMaxIndex ; i ++ )
	{
//		D_SetLightParam( i, FALSE, NULL ) ;
		Graphics_Hardware_Light_SetEnable_PF( i, FALSE ) ;
	}

	// 設定が変更されたフラグを倒す
	GSYS.Light.HardwareChange = FALSE ;

	// 設定が変更されたかどうかのフラグを立てる
	GSYS.ChangeSettingFlag = TRUE ;

	// 設定の際反映が必要かどうかのフラグを倒す
	GSYS.Light.HardwareRefresh = FALSE ;

	// ピクセルシェーダーに設定するシャドウマップの情報を更新する
	Graphics_ShadowMap_RefreshPSParam() ;

	// 終了
	return 0 ;
}






















// シェーダー関係関数

// シェーダーハンドルを作成する
extern int Graphics_Shader_CreateHandle( int ASyncThread )
{
	if( GSYS.InitializeFlag == FALSE )
		return -1 ;

	// シェーダが使えない場合は失敗
	if( GSYS.HardInfo.UseShader == FALSE )
		return -1 ;

	return AddHandle( DX_HANDLETYPE_SHADER, ASyncThread, -1 ) ;
}

// CreateShader の実処理関数
static int Graphics_Shader_CreateHandle_Static(
	int ShaderHandle,
	int IsVertexShader,
	void *Image,
	int ImageSize,
	int ImageAfterFree,
	int ASyncThread
)
{
	return Graphics_Hardware_Shader_Create_PF( ShaderHandle, IsVertexShader, Image, ImageSize, ImageAfterFree, ASyncThread ) ;
}

#ifndef DX_NON_ASYNCLOAD

// CreateShader の非同期読み込みスレッドから呼ばれる関数
static void Graphics_Shader_CreateHandle_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int   ShaderHandle ;
	int   IsVertexShader ;
	void *Image ;
	int   ImageSize ;
	int   ImageAfterFree ;
	int   Addr ;
	int   Result ;

	Addr = 0 ;
	ShaderHandle   = GetASyncLoadParamInt(   AParam->Data, &Addr ) ;
	IsVertexShader = GetASyncLoadParamInt(   AParam->Data, &Addr ) ;
	Image          = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	ImageSize      = GetASyncLoadParamInt(   AParam->Data, &Addr ) ;
	ImageAfterFree = GetASyncLoadParamInt(   AParam->Data, &Addr ) ;

	Result = Graphics_Shader_CreateHandle_Static( ShaderHandle, IsVertexShader, Image, ImageSize, ImageAfterFree, TRUE ) ;
	DecASyncLoadCount( ShaderHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteShader( ShaderHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// シェーダーハンドルを作成する
extern int Graphics_Shader_CreateHandle_UseGParam(
	int IsVertexShader,
	void *Image,
	int ImageSize,
	int ImageAfterFree,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	int ShaderHandle = -1 ;

	// シェーダが使えない場合は失敗
	if( Graphics_Hardware_CheckValid_PF() == 0 || GSYS.HardInfo.UseShader == FALSE )
		return -1 ;

	ShaderHandle = Graphics_Shader_CreateHandle( ASyncThread ) ;
	if( ShaderHandle == -1 )
		goto ERR ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, ShaderHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, IsVertexShader ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, Image ) ;
		AddASyncLoadParamInt( NULL, &Addr, ImageSize ) ;
		AddASyncLoadParamInt( NULL, &Addr, ImageAfterFree ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = Graphics_Shader_CreateHandle_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ShaderHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, IsVertexShader ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, Image ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ImageSize ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ImageAfterFree ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( ShaderHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( Graphics_Shader_CreateHandle_Static( ShaderHandle, IsVertexShader, Image, ImageSize, ImageAfterFree, ASyncThread ) < 0 )
			goto ERR ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( ShaderHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	// ハンドルを返す
	return ShaderHandle ;

ERR :
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( ShaderHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	NS_DeleteShader( ShaderHandle ) ;
	ShaderHandle = -1 ;

	// 終了
	return -1 ;
}

// LoadShader の実処理関数
static int Graphics_Shader_LoadShader_Static( int ShaderHandle, int IsVertexShader, const wchar_t *FileName, int ASyncThread )
{
	SHADERHANDLEDATA *pShader ;
	DWORD_PTR fp ;
	LONGLONG size ;
	void *buffer ;

	// エラー判定
	if( ASyncThread )
	{
		if( SHADERCHK_ASYNC( ShaderHandle, pShader ) )
			return -1 ;
	}
	else
	{
		if( SHADERCHK( ShaderHandle, pShader ) )
			return -1 ;
	}

	fp = FOPEN( FileName ) ;
	if( fp == 0 ) return -1 ;
	FSEEK( fp, 0, SEEK_END ) ;
	size = FTELL( fp ) ;
	FSEEK( fp, 0, SEEK_SET ) ;
	buffer = DXALLOC( ( size_t )size ) ;
	if( buffer == NULL )
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xb7\x30\xa7\x30\xfc\x30\xc0\x30\xfc\x30\x6e\x30\xd0\x30\xa4\x30\xca\x30\xea\x30\xc7\x30\xfc\x30\xbf\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"シェーダーのバイナリデータを格納するメモリ領域の確保に失敗しました\n" @*/ ) ;
		return -1 ;
	}
	FREAD( buffer, size, 1, fp ) ;
	FCLOSE( fp ) ;

	return Graphics_Shader_CreateHandle_Static( ShaderHandle, IsVertexShader, buffer, ( int )size, TRUE, ASyncThread ) ;
}

#ifndef DX_NON_ASYNCLOAD

// Graphics_Shader_LoadShader の非同期読み込みスレッドから呼ばれる関数
static void Graphics_Shader_LoadShader_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int          ShaderHandle ;
	int          IsVertexShader ;
	const wchar_t *FileName ;
	int          Addr ;
	int          Result ;

	Addr = 0 ;
	ShaderHandle   = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	IsVertexShader = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileName       = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = Graphics_Shader_LoadShader_Static( ShaderHandle, IsVertexShader, FileName, TRUE ) ;
	DecASyncLoadCount( ShaderHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteShader( ShaderHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// シェーダーバイナリをファイルから読み込む
extern int Graphics_Shader_LoadShader_UseGParam( int IsVertexShader, const wchar_t *FileName, int ASyncLoadFlag )
{
	int ShaderHandle = -1 ;

	// シェーダが使えない場合は失敗
	if( Graphics_Hardware_CheckValid_PF() == 0 || GSYS.HardInfo.UseShader == FALSE )
		return -1 ;

	ShaderHandle = Graphics_Shader_CreateHandle( FALSE ) ;
	if( ShaderHandle == -1 )
		goto ERR ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		wchar_t FullPath[ 1024 ] ;

		ConvertFullPathW_( FileName, FullPath ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, ShaderHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, IsVertexShader ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = Graphics_Shader_LoadShader_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ShaderHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, IsVertexShader ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( ShaderHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( Graphics_Shader_LoadShader_Static( ShaderHandle, IsVertexShader, FileName, FALSE ) < 0 )
			goto ERR ;
	}

	// ハンドルを返す
	return ShaderHandle ;

ERR :
	NS_DeleteShader( ShaderHandle ) ;
	ShaderHandle = -1 ;

	// 終了
	return -1 ;
}

// シェーダーハンドルの初期化
extern int Graphics_Shader_InitializeHandle( HANDLEINFO *HandleInfo )
{
	SHADERHANDLEDATA *Shader = ( SHADERHANDLEDATA * )HandleInfo ;

	// 環境依存データのアドレスをセット
	Shader->PF = ( SHADERHANDLEDATA_PF * )( Shader + 1 ) ;

	// 正常終了
	return 0 ;
}

// シェーダーハンドルの後始末
extern int Graphics_Shader_TerminateHandle( HANDLEINFO *HandleInfo )
{
	SHADERHANDLEDATA *Shader = ( SHADERHANDLEDATA * )HandleInfo ;

	// 使用するシェーダーとしてセットされていたら外す
	if( GSYS.DrawSetting.UserShaderRenderInfo.SetVertexShaderHandle == HandleInfo->Handle )
	{
		NS_SetUseVertexShader( -1 ) ;
	}
	if( GSYS.DrawSetting.UserShaderRenderInfo.SetPixelShaderHandle == HandleInfo->Handle )
	{
		NS_SetUsePixelShader( -1 ) ;
	}

	// シェーダーのバイナリデータを解放
	if( Shader->FunctionCode != NULL )
	{
		DXFREE( Shader->FunctionCode ) ;
		Shader->FunctionCode = NULL ;
	}

	// 環境依存の後始末処理を実行
	Graphics_Hardware_Shader_TerminateHandle_PF( Shader ) ;

	// 終了
	return 0 ;
}

// シェーダーデータをハンドル値から取り出す
extern	SHADERHANDLEDATA *Graphics_Shader_GetData( int ShaderHandle, int ASyncThread )
{
	SHADERHANDLEDATA *Shader ;

	// エラー判定
	if( ASyncThread )
	{
		if( SHADERCHK_ASYNC( ShaderHandle, Shader ) )
		{
			return NULL ;
		}
	}
	else
	{
		if( SHADERCHK( ShaderHandle, Shader ) )
		{
			return NULL ;
		}
	}

	return Shader ;
}



















// 定数バッファ関係関数

// シェーダー用定数バッファハンドルを作成する
extern int Graphics_ShaderConstantBuffer_CreateHandle( int ASyncThread )
{
	if( GSYS.InitializeFlag == FALSE )
		return -1 ;

	// シェーダが使えない場合は失敗
	if( GSYS.HardInfo.UseShader == FALSE )
		return -1 ;

	return AddHandle( DX_HANDLETYPE_SHADER_CONSTANT_BUFFER, ASyncThread, -1 ) ;
}


// Graphics_ShaderConstantBuffer_CreateHandle の実処理関数
static int Graphics_ShaderConstantBuffer_CreateHandle_Static(
	int ShaderConstantBufferHandle,
	int BufferSize,
	int ASyncThread
)
{
	return Graphics_Hardware_ShaderConstantBuffer_Create_PF( ShaderConstantBufferHandle, BufferSize, ASyncThread ) ;
}

#ifndef DX_NON_ASYNCLOAD

// Graphics_ShaderConstantBuffer_CreateHandle の非同期読み込みスレッドから呼ばれる関数
static void Graphics_ShaderConstantBuffer_CreateHandle_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int   ShaderConstantBufferHandle ;
	int   BufferSize ;
	int   Addr ;
	int   Result ;

	Addr = 0 ;
	ShaderConstantBufferHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	BufferSize                 = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = Graphics_ShaderConstantBuffer_CreateHandle_Static( ShaderConstantBufferHandle, BufferSize, TRUE ) ;
	DecASyncLoadCount( ShaderConstantBufferHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteShaderConstantBuffer( ShaderConstantBufferHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// シェーダー用定数バッファハンドルを作成する
extern int Graphics_ShaderConstantBuffer_Create( int BufferSize, int ASyncLoadFlag, int ASyncThread )
{
	int ShaderConstantBufferHandle = -1 ;

	// シェーダが使えない場合は失敗
	if( Graphics_Hardware_CheckValid_PF() == 0 || GSYS.HardInfo.UseShader == FALSE )
		return -1 ;

	ShaderConstantBufferHandle = Graphics_ShaderConstantBuffer_CreateHandle( ASyncThread ) ;
	if( ShaderConstantBufferHandle == -1 )
		goto ERR ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, ShaderConstantBufferHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, BufferSize ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = Graphics_ShaderConstantBuffer_CreateHandle_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, ShaderConstantBufferHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, BufferSize ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( ShaderConstantBufferHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( Graphics_ShaderConstantBuffer_CreateHandle_Static( ShaderConstantBufferHandle, BufferSize, ASyncThread ) < 0 )
			goto ERR ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( ShaderConstantBufferHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	// ハンドルを返す
	return ShaderConstantBufferHandle ;

ERR :
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( ShaderConstantBufferHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	NS_DeleteShaderConstantBuffer( ShaderConstantBufferHandle ) ;
	ShaderConstantBufferHandle = -1 ;

	// 終了
	return -1 ;
}

// シェーダー用定数バッファハンドルの初期化
extern int Graphics_ShaderConstantBuffer_InitializeHandle( HANDLEINFO *HandleInfo )
{
	SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer = ( SHADERCONSTANTBUFFERHANDLEDATA * )HandleInfo ;

	// 環境依存データのアドレスをセット
	ShaderConstantBuffer->PF = ( SHADERCONSTANTBUFFERHANDLEDATA_PF * )( ShaderConstantBuffer + 1 ) ;

	// 正常終了
	return 0 ;
}

// シェーダー用定数バッファハンドルの後始末
extern int Graphics_ShaderConstantBuffer_TerminateHandle( HANDLEINFO *HandleInfo )
{
	SHADERCONSTANTBUFFERHANDLEDATA *ShaderConstantBuffer = ( SHADERCONSTANTBUFFERHANDLEDATA * )HandleInfo ;

	// 環境依存の後始末処理を実行
	Graphics_Hardware_ShaderConstantBuffer_TerminateHandle_PF( ShaderConstantBuffer ) ;

	// 終了
	return 0 ;
}




























// 頂点バッファ・インデックスバッファ関係関数

// 頂点バッファを作成する
extern int Graphics_VertexBuffer_Create( int VertexNum, int VertexType /* DX_VERTEX_TYPE_NORMAL_3D 等 */, int ASyncThread )
{
	int NewHandle ;
	int Result ;

	// 新しいハンドルを作成する
	NewHandle = AddHandle( DX_HANDLETYPE_VERTEX_BUFFER, ASyncThread, -1 ) ;
	if( NewHandle < 0 )
	{
		return -1 ;
	}

	// ハンドルのセットアップ
	Result = Graphics_VertexBuffer_SetupHandle( NewHandle, VertexNum, VertexType ) ;
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( NewHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD
	if( Result < 0 )
	{
		SubHandle( NewHandle ) ;
		return -1 ;
	}

	// ハンドルを返す
	return NewHandle ;
}

// 頂点バッファハンドルのセットアップを行う
extern int Graphics_VertexBuffer_SetupHandle( int VertexBufHandle, int VertexNum, int VertexType /* DX_VERTEX_TYPE_NORMAL_3D 等 */ )
{
	VERTEXBUFFERHANDLEDATA *VertexBuffer ;
	int VertexSize ;

	// エラー判定
	if( VERTEXBUFFERCHK( VertexBufHandle, VertexBuffer ) )
		return -1 ;

	// 対応していないタイプの場合はエラー
	switch( VertexType )
	{
	case DX_VERTEX_TYPE_NORMAL_3D :
		VertexSize = sizeof( VERTEX3D ) ;
		break ;

	case DX_VERTEX_TYPE_SHADER_3D :
		VertexSize = sizeof( VERTEX3DSHADER ) ;

		// シェーダーが使えない場合は失敗
		if( GSYS.HardInfo.UseShader == FALSE )
			return -1 ;
		break ;

	default :
		DXST_ERRORLOG_ADDUTF16LE( "\x02\x98\xb9\x70\xbf\x30\xa4\x30\xd7\x30\x6e\x30\x24\x50\x4c\x30\x0d\x4e\x63\x6b\x67\x30\x59\x30\x0a\x00\x00"/*@ L"頂点タイプの値が不正です\n" @*/ ) ;
		return -1 ;
	}

	// タイプを保存
	VertexBuffer->Type = VertexType ;

	// 頂点数を保存
	VertexBuffer->Num = VertexNum ;

	// 頂点データ一つ辺りのバイト数を保存
	VertexBuffer->UnitSize = VertexSize ;

	// 環境依存の頂点バッファ作成処理
	if( Graphics_Hardware_VertexBuffer_Create_PF( VertexBuffer ) < 0 )
	{
		return -1 ;
	}

	// データ領域を確保する
	if( ReallocHandle( VertexBufHandle, sizeof( VERTEXBUFFERHANDLEDATA ) + sizeof( VERTEXBUFFERHANDLEDATA_PF ) + VertexSize * VertexNum + 32 ) < 0 )
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"メモリの確保に失敗しました" @*/ ) ;
		return -1 ;
	}
	VertexBuffer     = ( VERTEXBUFFERHANDLEDATA *    )GetHandleInfo( VertexBufHandle ) ;
	VertexBuffer->PF = ( VERTEXBUFFERHANDLEDATA_PF * )( VertexBuffer + 1 ) ;

	// システムメモリ上の頂点データを保存するアドレスをセット
	VertexBuffer->Buffer = ( void * )( ( ( DWORD_PTR )VertexBuffer + sizeof( VERTEXBUFFERHANDLEDATA ) + sizeof( VERTEXBUFFERHANDLEDATA_PF ) + 31 ) / 32 * 32 ) ;

	// 正常終了
	return 0 ;
}

// 頂点バッファハンドルの初期化
extern int Graphics_VertexBuffer_InitializeHandle( HANDLEINFO *HandleInfo )
{
	VERTEXBUFFERHANDLEDATA *VertexBuffer = ( VERTEXBUFFERHANDLEDATA * )HandleInfo ;

	VertexBuffer->PF = ( VERTEXBUFFERHANDLEDATA_PF * )( VertexBuffer + 1 ) ;

	// 正常終了
	return 0 ;
}

// 頂点バッファハンドルの後始末
extern int Graphics_VertexBuffer_TerminateHandle( HANDLEINFO *HandleInfo )
{
	VERTEXBUFFERHANDLEDATA *VertexBuffer = ( VERTEXBUFFERHANDLEDATA * )HandleInfo ;

	// 環境依存後始末処理
	Graphics_Hardware_VertexBuffer_Terminate_PF( VertexBuffer ) ;

	// 終了
	return 0 ;
}

// インデックスバッファを作成する
extern int Graphics_IndexBuffer_Create( int IndexNum, int IndexType /* DX_INDEX_TYPE_16BIT 等 */, int ASyncThread )
{
	int NewHandle ;
	int Result ;

	// 新しいハンドルを作成する
	NewHandle = AddHandle( DX_HANDLETYPE_INDEX_BUFFER, ASyncThread, -1 ) ;
	if( NewHandle < 0 )
	{
		return -1 ;
	}

	// ハンドルのセットアップ
	Result = Graphics_IndexBuffer_SetupHandle( NewHandle, IndexNum, IndexType ) ;
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( NewHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD
	if( Result < 0 )
	{
		SubHandle( NewHandle ) ;
		return -1 ;
	}

	// ハンドルを返す
	return NewHandle ;
}

// インデックスバッファハンドルのセットアップを行う
extern int Graphics_IndexBuffer_SetupHandle( int IndexBufHandle, int IndexNum, int IndexType /* DX_INDEX_TYPE_16BIT 等 */ )
{
	INDEXBUFFERHANDLEDATA *IndexBuffer ;
	int IndexSize ;

	// エラー判定
	if( INDEXBUFFERCHK( IndexBufHandle, IndexBuffer ) )
		return -1 ;

	// 対応していないタイプの場合はエラー
	switch( IndexType )
	{
	case DX_INDEX_TYPE_16BIT :
		IndexSize = sizeof( WORD ) ;
		break ;

	case DX_INDEX_TYPE_32BIT :
		IndexSize = sizeof( DWORD ) ;
		break ;

	default :
		DXST_ERRORLOG_ADDUTF16LE( "\xa4\x30\xf3\x30\xc7\x30\xc3\x30\xaf\x30\xb9\x30\xbf\x30\xa4\x30\xd7\x30\x6e\x30\x24\x50\x4c\x30\x0d\x4e\x63\x6b\x67\x30\x59\x30\x0a\x00\x00"/*@ L"インデックスタイプの値が不正です\n" @*/ ) ;
		return -1 ;
	}

	// タイプを保存
	IndexBuffer->Type = IndexType ;

	// インデックスデータ一つ辺りのバイト数を保存
	IndexBuffer->UnitSize = IndexSize ;

	// インデックス数を保存
	IndexBuffer->Num = IndexNum ;

	// 環境依存のインデックスバッファ作成処理
	if( Graphics_Hardware_IndexBuffer_Create_PF( IndexBuffer ) < 0 )
	{
		return -1 ;
	}

	// データ領域を確保する
	if( ReallocHandle( IndexBufHandle, sizeof( INDEXBUFFERHANDLEDATA ) + sizeof( INDEXBUFFERHANDLEDATA_PF ) + IndexSize * IndexNum + 32 ) < 0 )
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"メモリの確保に失敗しました" @*/ ) ;
		return -1 ;
	}
	IndexBuffer     = ( INDEXBUFFERHANDLEDATA *    )GetHandleInfo( IndexBufHandle ) ;
	IndexBuffer->PF = ( INDEXBUFFERHANDLEDATA_PF * )( IndexBuffer + 1 ) ;

	// システムメモリ上のインデックスデータを保存するアドレスをセット
	IndexBuffer->Buffer = ( void * )( ( ( DWORD_PTR )IndexBuffer + sizeof( INDEXBUFFERHANDLEDATA ) + sizeof( INDEXBUFFERHANDLEDATA_PF ) + 31 ) / 32 * 32 ) ;

	// 正常終了
	return 0 ;
}

// インデックスバッファハンドルの初期化
extern int Graphics_IndexBuffer_InitializeHandle( HANDLEINFO *HandleInfo )
{
	INDEXBUFFERHANDLEDATA *IndexBuffer = ( INDEXBUFFERHANDLEDATA * )HandleInfo ;

	IndexBuffer->PF = ( INDEXBUFFERHANDLEDATA_PF * )( IndexBuffer + 1 ) ;

	// 正常終了
	return 0 ;
}

// インデックスバッファハンドルの後始末
extern int Graphics_IndexBuffer_TerminateHandle( HANDLEINFO *HandleInfo )
{
	INDEXBUFFERHANDLEDATA *IndexBuffer = ( INDEXBUFFERHANDLEDATA * )HandleInfo ;

	// 環境依存後始末処理
	Graphics_Hardware_IndexBuffer_Terminate_PF( IndexBuffer ) ;

	// 終了
	return 0 ;
}



























// シャドウマップ関係関数

// MakeShadowMap の実処理関数
static int Graphics_ShadowMap_MakeShadowMap_Static(
	SETUP_SHADOWMAPHANDLE_GPARAM *GParam,
	int SmHandle,
	int SizeX,
	int SizeY,
	int ASyncThread
)
{
	// ハンドルの初期化
	if( Graphics_ShadowMap_SetupHandle_UseGParam(
			GParam,
			SmHandle,
			SizeX,
			SizeY,
			TRUE,
			16,
			ASyncThread
		) == -1 )
		return -1 ;

	// 終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// MakeShadowMap の非同期読み込みスレッドから呼ばれる関数
static void Graphics_ShadowMap_MakeShadowMap_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	SETUP_SHADOWMAPHANDLE_GPARAM *GParam ;
	int SmHandle ;
	int SizeX ;
	int SizeY ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( SETUP_SHADOWMAPHANDLE_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	SmHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeX = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SizeY = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = Graphics_ShadowMap_MakeShadowMap_Static( GParam, SmHandle, SizeX, SizeY, TRUE ) ;

	DecASyncLoadCount( SmHandle ) ;
	if( Result < 0 )
	{
		NS_DeleteGraph( SmHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// シャドウマップハンドルを作成する関数
extern int Graphics_ShadowMap_MakeShadowMap_UseGParam(
	SETUP_SHADOWMAPHANDLE_GPARAM *GParam,
	int SizeX,
	int SizeY,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	int SmHandle ;

	CheckActiveState() ;

	// ハンドルの作成
	SmHandle = Graphics_ShadowMap_AddHandle( ASyncThread ) ;
	if( SmHandle == -1 )
	{
		return -1 ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, SmHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeX ) ;
		AddASyncLoadParamInt( NULL, &Addr, SizeY ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = Graphics_ShadowMap_MakeShadowMap_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SmHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeX ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SizeY ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( SmHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( Graphics_ShadowMap_MakeShadowMap_Static( GParam, SmHandle, SizeX, SizeY, ASyncThread ) < 0 )
			goto ERR ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( SmHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	// 終了
	return SmHandle ;

ERR :
#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( SmHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	NS_DeleteGraph( SmHandle ) ;

	return -1 ;
}

// 新しいシャドウマップハンドルを確保する
extern int Graphics_ShadowMap_AddHandle( int ASyncThread )
{
	return AddHandle( DX_HANDLETYPE_SHADOWMAP, ASyncThread, -1 );
}

// シャドウマップハンドルのセットアップを行う
extern int Graphics_ShadowMap_SetupHandle_UseGParam( SETUP_SHADOWMAPHANDLE_GPARAM * /* GParam */, int SmHandle, int SizeX, int SizeY, int TexFormat_Float, int TexFormat_BitDepth, int /* ASyncThread */ )
{
	SHADOWMAPDATA *ShadowMap ;

	// アドレスの取得
	if( SHADOWMAPCHK_ASYNC( SmHandle, ShadowMap ) )
		return -1 ;

	// パラメータの保存
	ShadowMap->TexFormat_Float = TexFormat_Float ;
	ShadowMap->TexFormat_BitDepth = TexFormat_BitDepth ;
	ShadowMap->BaseSizeX = SizeX ;
	ShadowMap->BaseSizeY = SizeY ;
	ShadowMap->ShadowMapViewMatrix.m[ 3 ][ 3 ] = 1.0f ;

	// テクスチャの作成
	if( Graphics_ShadowMap_CreateTexture( ShadowMap ) == -1 )
	{
		goto ERR ;
	}

	// 終了
	return 0 ;

	// エラー終了
ERR :
	NS_DeleteGraph( SmHandle ) ;

	return -1 ;
}

// シャドウマップデータに必要なテクスチャを作成する
extern int Graphics_ShadowMap_CreateTexture( SHADOWMAPDATA *ShadowMap, int ASyncThread )
{
	return Graphics_Hardware_ShadowMap_CreateTexture_PF( ShadowMap, ASyncThread ) ;
}

// シャドウマップデータに必要なテクスチャを解放する
extern int Graphics_ShadowMap_ReleaseTexture( SHADOWMAPDATA *ShadowMap )
{
	return Graphics_Hardware_ShadowMap_ReleaseTexture_PF( ShadowMap ) ;
}

// シャドウマップデータをハンドル値から取り出す
extern SHADOWMAPDATA *Graphics_ShadowMap_GetData( int SmHandle, int ASyncThread )
{
	SHADOWMAPDATA *ShadowMap ;

	// エラー判定
	if( ASyncThread )
	{
		if( SHADOWMAPCHK_ASYNC( SmHandle, ShadowMap ) )
			return NULL ;
	}
	else
	{
		if( SHADOWMAPCHK( SmHandle, ShadowMap ) )
			return NULL ;
	}

	return ShadowMap ;
}

// 頂点シェーダーに設定するシャドウマップの情報を更新する
extern void Graphics_ShadowMap_RefreshVSParam( void )
{
	Graphics_Hardware_ShadowMap_RefreshVSParam_PF() ;
}

// ピクセルシェーダーに設定するシャドウマップの情報を更新する
extern void Graphics_ShadowMap_RefreshPSParam( void )
{
	Graphics_Hardware_ShadowMap_RefreshPSParam_PF() ;
}

// シャドウマップへのレンダリングで使用する行列の情報を更新する
extern int Graphics_ShadowMap_RefreshMatrix( SHADOWMAPDATA *ShadowMap )
{
	VECTOR TempPos ;
	VECTOR MaxPos ;
	VECTOR MinPos ;
	VECTOR *SrcPos ;
	VECTOR EyePos ;
	VECTOR AreaSize ;
	int i ;

	// ライト始点からの最大座標と最小座標を割り出す
	SrcPos = ShadowMap->EnableDrawArea ? ShadowMap->DrawAreaViewClipPos : ShadowMap->DefaultViewClipPos ;
	MinPos.x =  100000000.0f ;
	MinPos.y =  100000000.0f ;
	MinPos.z =  100000000.0f ;
	MaxPos.x = -100000000.0f ;
	MaxPos.y = -100000000.0f ;
	MaxPos.z = -100000000.0f ;
	for( i = 0 ; i < 8 ; i ++, SrcPos ++ )
	{
		VectorTransformSR( &TempPos, SrcPos, &ShadowMap->LightMatrix ) ;

		if( MinPos.x > TempPos.x ) MinPos.x = TempPos.x ;
		if( MinPos.y > TempPos.y ) MinPos.y = TempPos.y ;
		if( MinPos.z > TempPos.z ) MinPos.z = TempPos.z ;

		if( MaxPos.x < TempPos.x ) MaxPos.x = TempPos.x ;
		if( MaxPos.y < TempPos.y ) MaxPos.y = TempPos.y ;
		if( MaxPos.z < TempPos.z ) MaxPos.z = TempPos.z ;
	}

	// シャドウマップ描画用ビュー行列の平行移動成分を設定
	EyePos.x = ( MaxPos.x + MinPos.x ) / 2.0f ;
	EyePos.y = ( MaxPos.y + MinPos.y ) / 2.0f ;
	EyePos.z = MinPos.z ;
	ShadowMap->ShadowMapViewMatrix.m[ 3 ][ 0 ] = -EyePos.x ;
	ShadowMap->ShadowMapViewMatrix.m[ 3 ][ 1 ] = -EyePos.y ;
	ShadowMap->ShadowMapViewMatrix.m[ 3 ][ 2 ] = -EyePos.z ;

	// シャドウマップ描画用の射影行列の拡大成分を設定
	AreaSize = VSub( MaxPos, MinPos ) ;
	ShadowMap->ShadowMapProjectionMatrix.m[ 0 ][ 0 ] = 2.0f / AreaSize.x ;
	ShadowMap->ShadowMapProjectionMatrix.m[ 1 ][ 1 ] = 2.0f / AreaSize.y ;
	ShadowMap->ShadowMapProjectionMatrix.m[ 2 ][ 2 ] = 1.0f / ( MaxPos.z - MinPos.z ) ;

	// シャドウマップ描画用ビュー行列とシャドウマップ描画用射影行列を乗算したものを保存
	CreateMultiplyMatrix( &ShadowMap->ShadowMapViewProjectionMatrix, &ShadowMap->ShadowMapViewMatrix, &ShadowMap->ShadowMapProjectionMatrix ) ;

	// ビュー行列と射影行列のセット
	NS_SetTransformToView( &ShadowMap->ShadowMapViewMatrix ) ;
	NS_SetTransformToProjection( &ShadowMap->ShadowMapProjectionMatrix ) ;

	// 終了
	return 0 ;
}

// シャドウマップハンドルの初期化
extern int Graphics_ShadowMap_InitializeHandle( HANDLEINFO *HandleInfo )
{
	SHADOWMAPDATA *ShadowMap = ( SHADOWMAPDATA * )HandleInfo ;

	ShadowMap->PF             = ( SHADOWMAPDATA_PF * )( ShadowMap + 1 ) ;

	ShadowMap->BaseSizeX      = -1 ;
	ShadowMap->BaseSizeY      = -1 ;

	ShadowMap->AdjustDepth    = DEFAULT_SHADOWMAP_ADJUST_DEPTH ;
	ShadowMap->BlurParam      = DEFAULT_SHADOWMAP_BLUR_PARAM ;
	ShadowMap->GradationParam = DEFAULT_SHADOWMAP_GRADATION_RANGE ;

	ShadowMap->LightMatrix.m[ 3 ][ 3 ]               = 1.0f ;
	ShadowMap->ShadowMapProjectionMatrix.m[ 3 ][ 3 ] = 1.0f ;
	CreateMultiplyMatrix( &ShadowMap->ShadowMapViewProjectionMatrix, &ShadowMap->ShadowMapViewMatrix, &ShadowMap->ShadowMapProjectionMatrix ) ;

	// 初期ライト方向の設定
	NS_SetShadowMapLightDirection( HandleInfo->Handle, VGet( 0.0f, -1.0f, 0.0f ) ) ;

	// 終了
	return 0 ;
}

// シャドウマップハンドルの後始末
extern int Graphics_ShadowMap_TerminateHandle( HANDLEINFO *HandleInfo )
{
	SHADOWMAPDATA *ShadowMap = ( SHADOWMAPDATA * )HandleInfo ;
	int i ;

	// 描画で使用するシャドウマップに設定されていたら解除
	for( i = 0 ; i < MAX_USE_SHADOWMAP_NUM ; i ++ )
	{
		if( GSYS.DrawSetting.ShadowMap[ i ] == HandleInfo->Handle )
		{
			NS_SetUseShadowMap( i, -1 ) ;
		}
	}

	// シャドウマップの描画準備が行われていたら解除する
	if( ShadowMap->DrawSetupFlag )
	{
		NS_ShadowMap_DrawEnd() ;
	}

	// サーフェスやテクスチャを解放
	Graphics_ShadowMap_ReleaseTexture( ShadowMap ) ;

	// 解放時に立てるフラグへのポインタが有効である場合は立てる
	if( ShadowMap->LostFlag != NULL )
		*ShadowMap->LostFlag = TRUE ;

	// 終了
	return 0 ;
}




























// 補助関係関数

// 共有メモリの確保
extern int Graphics_Other_AllocCommonBuffer( int Index, DWORD Size )
{
	if( Index >= COMMON_BUFFER_NUM || Index < 0 )
	{
		return -1 ;
	}

	if( GSYS.Resource.CommonBufferSize[ Index ] >= Size )
	{
		return 0 ;
	}

	GSYS.Resource.CommonBuffer[ Index ] = DXREALLOC( GSYS.Resource.CommonBuffer[ Index ], Size ) ;
	if( GSYS.Resource.CommonBuffer[ Index ] == NULL )
	{
		return -1 ;
	}
	GSYS.Resource.CommonBufferSize[ Index ] = Size ;

	return 0 ;
}

// 共有メモリの解放
extern int Graphics_Other_TerminateCommonBuffer( void ) 
{
	int i ;

	for( i = 0 ; i < COMMON_BUFFER_NUM ; i ++ )
	{
		if( GSYS.Resource.CommonBuffer[ i ] != NULL )
		{
			DXFREE( GSYS.Resource.CommonBuffer[ i ] ) ;
			GSYS.Resource.CommonBuffer[ i ] = 0 ;
		}

		GSYS.Resource.CommonBufferSize[ i ] = 0 ;
	}

	return 0 ;
}

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE


