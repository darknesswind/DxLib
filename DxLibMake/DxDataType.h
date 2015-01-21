// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		データタイプ定義ヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXDATATYPE_H__
#define __DXDATATYPE_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include <stdio.h>

#if !defined( __ANDROID )
#include "DxDataTypeWin.h"
#endif

#if defined( __ANDROID )
#include "DxDataTypeAndroid.h"
#endif

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#define SETRECT( Dest, Left, Top, Right, Bottom )	\
	( Dest ).left   = Left ;\
	( Dest ).top    = Top ;\
	( Dest ).right  = Right ;\
	( Dest ).bottom = Bottom ;

// 结构体定义 --------------------------------------------------------------------

// ＲＧＢＡ色構造体
struct RGBCOLOR
{
	unsigned char			Blue, Green, Red, Alpha ;
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

}

#endif // __DXDATATYPE_H__
