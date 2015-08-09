//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用GraphFilter系プログラム
// 
//  	Ver 3.14d
// 
//-----------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード ---------------------------------------------------------------
#include "DxGraphicsWin.h"
#include "DxGraphicsFilterWin.h"
#include "DxGraphicsFilterD3D9.h"
#include "DxGraphicsFilterD3D11.h"

#ifndef DX_NON_FILTER

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 -----------------------------------------------------------------

// 構造体宣言 -----------------------------------------------------------------

// データ定義 -----------------------------------------------------------------

// 関数宣言 -------------------------------------------------------------------

// プログラム -----------------------------------------------------------------

// 環境依存関係

extern int	GraphFilter_Mono_PF(       GRAPHFILTER_INFO *Info, float Cb, float Cr )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_Mono_PF(       Info,  Cb,  Cr ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_Mono_PF(       Info,  Cb,  Cr ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_Gauss_PF(      GRAPHFILTER_INFO *Info, int PixelWidth, float Param, float *Table )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_Gauss_PF(      Info,  PixelWidth,  Param, Table ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_Gauss_PF(      Info,  PixelWidth,  Param, Table ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_Down_Scale_PF(  GRAPHFILTER_INFO *Info, int DivNum )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_Down_Scale_PF(  Info,  DivNum ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_Down_Scale_PF(  Info,  DivNum ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_Bright_Clip_PF( GRAPHFILTER_INFO *Info, int CmpType, float CmpParam, int ClipFillFlag, COLOR_F *ClipFillColor )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_Bright_Clip_PF( Info,  CmpType,  CmpParam,  ClipFillFlag, ClipFillColor ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_Bright_Clip_PF( Info,  CmpType,  CmpParam,  ClipFillFlag, ClipFillColor ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_HSB_PF(         GRAPHFILTER_INFO *Info, int HueType, float Hue, float Saturation, float Bright )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_HSB_PF(         Info,  HueType,  Hue,  Saturation,  Bright ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_HSB_PF(         Info,  HueType,  Hue,  Saturation,  Bright ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_Invert_PF(      GRAPHFILTER_INFO *Info )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_Invert_PF(      Info ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_Invert_PF(      Info ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_Level_PF(       GRAPHFILTER_INFO *Info, float Min, float Max, float Gamma, float AfterMin, float AfterMax )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_Level_PF(       Info,  Min,  Max,  Gamma,  AfterMin,  AfterMax ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_Level_PF(       Info,  Min,  Max,  Gamma,  AfterMin,  AfterMax ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_TwoColor_PF(    GRAPHFILTER_INFO *Info, float Threshold, COLOR_F *LowColor, COLOR_F *HighColor )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_TwoColor_PF(    Info,  Threshold, LowColor, HighColor ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_TwoColor_PF(    Info,  Threshold, LowColor, HighColor ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_GradientMap_PF( GRAPHFILTER_INFO *Info, int MapGrHandle, int Reverse )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_GradientMap_PF( Info,  MapGrHandle,  Reverse ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_GradientMap_PF( Info,  MapGrHandle,  Reverse ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_PremulAlpha_PF( GRAPHFILTER_INFO *Info )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_PremulAlpha_PF( Info ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_PremulAlpha_PF( Info ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphFilter_InterpAlpha_PF( GRAPHFILTER_INFO *Info )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_InterpAlpha_PF( Info ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_InterpAlpha_PF( Info ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int GraphFilter_RectBltBase_Timing0_PF( GRAPHFILTER_INFO *Info, GRAPHFILTER_PARAM *Param )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_RectBltBase_Timing0_PF( Info, Param ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_RectBltBase_Timing0_PF( Info, Param ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int GraphFilter_RectBltBase_Timing1_PF( void )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_RectBltBase_Timing1_PF() ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_RectBltBase_Timing1_PF() ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int GraphFilter_DestGraphSetup_PF( GRAPHFILTER_INFO *Info, int *UseSrcGrHandle, int *UseDestGrHandle )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_DestGraphSetup_PF( Info, UseSrcGrHandle, UseDestGrHandle ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_DestGraphSetup_PF( Info, UseSrcGrHandle, UseDestGrHandle ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int GraphFilter_DestGraphUpdate_PF( GRAPHFILTER_INFO *Info, int UseDestGrHandle, int FilterResult )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphFilter_D3D11_DestGraphUpdate_PF( Info, UseDestGrHandle, FilterResult ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphFilter_D3D9_DestGraphUpdate_PF( Info, UseDestGrHandle, FilterResult ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}



extern int	GraphBlend_Basic_PF(           GRAPHFILTER_INFO *Info )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphBlend_D3D11_Basic_PF(           Info ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphBlend_D3D9_Basic_PF(           Info ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}

extern int	GraphBlend_RGBA_Select_Mix_PF( GRAPHFILTER_INFO *Info, int SelectR, int SelectG, int SelectB, int SelectA )
{
	switch( GRAWIN.Setting.UseGraphicsAPI )
	{
#ifndef DX_NON_DIRECT3D11
	case GRAPHICS_API_DIRECT3D11_WIN32 :
		return GraphBlend_D3D11_RGBA_Select_Mix_PF( Info,  SelectR,  SelectG,  SelectB,  SelectA ) ;
#endif // DX_NON_DIRECT3D11

#ifndef DX_NON_DIRECT3D9
	case GRAPHICS_API_DIRECT3D9_WIN32 :
		return GraphBlend_D3D9_RGBA_Select_Mix_PF( Info,  SelectR,  SelectG,  SelectB,  SelectA ) ;
#endif // DX_NON_DIRECT3D9

	default :
		return 0 ;
	}
}



#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // DX_NON_FILTER


