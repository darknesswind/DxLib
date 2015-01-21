//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用メモリ関係プログラム
// 
//  	Ver 3.11f
// 
//-----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ---------------------------------------------------------------
#include "DxMemoryWin.h"
#include <windows.h>

namespace DxLib
{

// 宏定义 -----------------------------------------------------------------

// 结构体声明 -----------------------------------------------------------------

// 数据定义 -----------------------------------------------------------------

// 函数声明 -------------------------------------------------------------------

// Program -----------------------------------------------------------------

// ラッパー関数

// メモリを確保する
extern void *AllocWrap( size_t AllocSize )
{
	void *AllocMemory ;

//	AllocMemory = malloc( AllocSize ) ;
	AllocMemory = HeapAlloc( GetProcessHeap(), 0, AllocSize ) ;

	return AllocMemory ;
}

// メモリを確保して０で初期化する
extern void *CallocWrap( size_t AllocSize )
{
	void *AllocMemory ;

//	AllocMemory = calloc( 1, AllocSize ) ;
	AllocMemory = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, AllocSize ) ;
	
	return AllocMemory ;
}

// メモリの再確保を行う
extern void *ReallocWrap( void *Memory, size_t AllocSize )
{
	void *AllocMemory ;

//	AllocMemory = realloc( Memory, AllocSize ) ;
	AllocMemory = HeapReAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, Memory, AllocSize ) ;
	
	return AllocMemory ;
}

// メモリを解放する
extern void FreeWrap( void *Memory )
{
//	free( Memory ) ;
	HeapFree( GetProcessHeap(), 0, Memory ) ;
}

}

