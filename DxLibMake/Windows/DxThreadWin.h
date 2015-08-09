// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用スレッド関係プログラムヘッダファイル
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

#ifndef __DXTHREADWIN_H__
#define __DXTHREADWIN_H__

// インクルード ------------------------------------------------------------------
#include "../DxCompileConfig.h"

#include <windows.h>

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

// 構造体定義 --------------------------------------------------------------------

// クリティカルセクション
struct DX_CRITICAL_SECTION
{
	CRITICAL_SECTION		CriticalSection ;
#if !defined( __BCC ) || defined( _DEBUG )
	char					FilePath[ 512 ] ;
	int						LineNo ;
	DWORD					ThreadID ;
#endif
} ;

// 一つのスレッドの情報
struct THREAD_INFO
{
	void					( *pFunction )( THREAD_INFO *, void * ) ;
	void					*pParam ;
	HANDLE					ThreadHandle ;
	DWORD					ThreadID ;
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXTHREADWIN_H__
