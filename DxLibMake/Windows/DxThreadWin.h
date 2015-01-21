// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用スレッド関係プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXTHREADWIN_H__
#define __DXTHREADWIN_H__

// Include ------------------------------------------------------------------
#include "../DxCompileConfig.h"

#include <windows.h>

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// クリティカルセクション
struct DX_CRITICAL_SECTION
{
	CRITICAL_SECTION		CriticalSection ;
#if !defined( __BCC ) || defined( _DEBUG )
	char					FilePath[ 256 ] ;
	int						LineNo ;
	DWORD					ThreadID ;
#endif
} ;

// 一つのスレッドの情報
struct THREAD_INFO
{
	void					( *pFunction )( THREAD_INFO *, void * ) ;
	void					*pParam ;
	void					*LocalValueAddr ;
	HANDLE					ThreadHandle ;
	DWORD					ThreadID ;
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

}

#endif // __DXTHREADWIN_H__
