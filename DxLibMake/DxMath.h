// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		演算プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXMATH_H__
#define __DXMATH_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxLib.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	void	TriangleBarycenter_Base( const VECTOR *TrianglePos1, const VECTOR *TrianglePos2, const VECTOR *TrianglePos3, const VECTOR *Position, float *TrianglePos1Weight, float *TrianglePos2Weight, float *TrianglePos3Weight ) ;	// 指定の座標から三角形の重心を求める

extern VECTOR	Get_Triangle_Point_MinPosition( VECTOR Point, VECTOR TrianglePos1, VECTOR TrianglePos2, VECTOR TrianglePos3 ) ;			// 点に一番近い三角形上の座標を得る
extern VECTOR	Get_Line_Point_MinPosition( VECTOR Point, VECTOR LinePos1, VECTOR LinePos2 ) ;											// 点に一番近い線上の座標を得る
extern int		Get_TwoPlane_Line( VECTOR Normal1, float Dist1, VECTOR Normal2, float Dist2, VECTOR *OutDir, VECTOR *OutPos ) ;			// 二つの平面が交差する線を得る( 戻り値が -1 の場合は、二つの面は平行 )
extern int		Get_TwoLine_MinLength_Rate( VECTOR Direction1, VECTOR Position1, float *OutRate1, VECTOR Direction2, VECTOR Position2, float *OutRate2 ) ;		// 二つの線の最近点のそれぞれの方向ベクトルの拡大値を得る

extern void		CreateNormalizePlane( FLOAT4 *Plane, VECTOR *Position, VECTOR *Normal ) ;												// 平面上の点と平面の法線から正規化された平面パラメータを算出する
extern void		SetMathScreenSize( int SizeX, int SizeY ) ;																				// DxMath.cpp で使用する画面のサイズを設定する

}

#endif // __DXMATH_H__
