// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		メモリ関係プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXMEMORY_H__
#define __DXMEMORY_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxThread.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#ifdef _WIN64
#define ALLOCNAMELENGTH			(32)
#define DXALLOC_INFO_SIZE		(64)
#else
#define ALLOCNAMELENGTH			(16)
#define DXALLOC_INFO_SIZE		(32)
#endif

#define DXADDRESS	(DXALLOC_INFO_SIZE * 2)
#define DXADDSIZE	(DXALLOC_INFO_SIZE * 3)

// 内存分配系函数
//#ifdef DX_USE_DXLIB_MEM_DUMP
#if !defined( __BCC ) || defined( _DEBUG )
	#define DXALLOC( size )			NS_DxAlloc( (size), __FILE__, __LINE__ )
	#define DXCALLOC( size )		NS_DxCalloc( (size), __FILE__, __LINE__ )
	#define DXREALLOC( mem, size )	NS_DxRealloc( (mem), (size), __FILE__, __LINE__ )
	#define DXFREE( mem )			NS_DxFree( (mem) )
#else
	#define DXALLOC( size )			NS_DxAlloc( (size), "", 0 )
	#define DXCALLOC( size )		NS_DxCalloc( (size), "", 0 )
	#define DXREALLOC( mem, size )	NS_DxRealloc( (mem), (size), "", 0 )
	#define DXFREE( mem )			NS_DxFree( (mem) )
#endif

//#else
//	#define DXALLOC( size )			AllocWrap( (size) )
//	#define DXCALLOC( size )		CallocWrap( (size) )
//	#define DXREALLOC( mem, size )	ReallocWrap( (mem), (size) )
//	#define DXFREE( mem )			FreeWrap( (mem) )
//	#define DXALLOC( size )			malloc( (size) )
//	#define DXCALLOC( size )		calloc( 1, (size) )
//	#define DXREALLOC( mem, size )	realloc( (mem), (size) )
//	#define DXFREE( mem )			free( (mem) )
//#endif

// 结构体定义 --------------------------------------------------------------------

// メモリ確保の情報
#ifdef _WIN64
struct ALLOCMEM
{
	char					Name[ ALLOCNAMELENGTH ] ;			// ファイルパス
	int						ID ;								// ＩＤ
	int						Line ;								// 行番号
	size_t					Size ;								// 確保サイズ
	struct ALLOCMEM			*Back, *Next ;						// 次と前の確保メモリ情報へのポインタ
} ;
#else
struct ALLOCMEM
{
	char					Name[ ALLOCNAMELENGTH ] ;			// ファイルパス
	unsigned short			ID ;								// ＩＤ
	unsigned short			Line ;								// 行番号
	size_t					Size ;								// 確保サイズ
	struct ALLOCMEM			*Back, *Next ;						// 次と前の確保メモリ情報へのポインタ
} ;
#endif

// メモリ関係の情報構造体
struct MEMORYDATA
{
	ALLOCMEM				AllocMemoryAnchor ;					// メモリ確保情報の最後尾
	char					AllocMemoryAnchorDummy[DXALLOC_INFO_SIZE * 2] ;
	ALLOCMEM				*AllocMemoryFirst ;					// メモリ確保情報の先頭
	unsigned short			AllocMemoryID ;						// 確保したメモリに割り当てるＩＤ
	size_t					AllocMemorySize ;					// 確保しているメモリの総量
	int						AllocMemoryNum ;					// ライブラリで確保したメモリの数
	int						AllocMemoryPrintFlag ;				// 確保される時に情報を出力するかどうかのフラグ
	int						AllocMemorySizeOutFlag ;			// 確保や解放が行われる度に確保メモリの総量をログに出力するかどうかのフラグ
	int						AllocMemoryErrorCheckFlag ;			// 確保や解放が行われる度に確保メモリの情報が破損していないかをチェックするかどうかのフラグ
	size_t					AllocTrapSize ;						// 列挙対象にする確保サイズ

#if defined( DX_THREAD_SAFE ) || defined( DX_THREAD_SAFE_NETWORK_ONLY )
	int						InitMemoryAllocCriticalSection ;	// MemoryAllocCriticalSection の初期化が済んでいるかどうかのフラグ
	DX_CRITICAL_SECTION		MemoryAllocCriticalSection ;		// メモリ確保処理用クリティカルセクション
#endif
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// メモリ関係の情報
extern MEMORYDATA MemData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

extern void			MemoryDump( void *buffer, int size ) ;
extern int			DxCheckAlloc( ALLOCMEM *Alloc ) ;										// 確保情報が破壊されていないか調べる

// ラッパー関数
extern	void 		*AllocWrap( size_t AllocSize ) ;										// メモリを確保する
extern	void 		*CallocWrap( size_t AllocSize ) ;										// メモリを確保して０で初期化する
extern	void		*ReallocWrap( void *Memory, size_t AllocSize ) ;						// メモリの再確保を行う
extern	void		FreeWrap( void *Memory ) ;												// メモリを解放する

}

#endif // __DXMEMORY_H__
