﻿//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用スレッド関係プログラム
// 
//  	Ver 3.11f
// 
//-----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ---------------------------------------------------------------
#include "DxThreadWin.h"
#include "../DxMemory.h"

#if !defined( __BCC ) || defined( _DEBUG )
#include "../DxBaseFunc.h"
#endif

namespace DxLib
{

// 宏定义 -----------------------------------------------------------------

// 结构体声明 -----------------------------------------------------------------

// 数据定义 -----------------------------------------------------------------

// 函数声明 -------------------------------------------------------------------

// Program -----------------------------------------------------------------


// スレッドの処理を初期化する
extern void Thread_Initialize( void )
{
	// Windows では特に何もしない
	return ;
}

// スレッドを実行する関数
DWORD WINAPI ThreadRunFunction( void *pParam )
{
	THREAD_INFO *pInfo = ( THREAD_INFO * )pParam ;

	Thread_Suspend() ;

	pInfo->pFunction( pInfo, pInfo->pParam ) ;

	ExitThread( 0 ) ;

	return 0 ;
}

// スレッドを作成する
extern int Thread_Create( THREAD_INFO *pThreadInfo, void ( *pFunction )( THREAD_INFO *, void * ), void *pParam )
{
	pThreadInfo->pFunction = pFunction ;
	pThreadInfo->pParam = pParam ;
	pThreadInfo->ThreadHandle = CreateThread( NULL, 0, ThreadRunFunction, pThreadInfo, 0, &pThreadInfo->ThreadID ) ;

	return pThreadInfo->ThreadHandle == NULL ? -1 : 0 ;
}

// スレッドの後始末を行う
extern void Thread_Delete( THREAD_INFO *pThreadInfo )
{
	CloseHandle( pThreadInfo->ThreadHandle ) ;
	pThreadInfo->ThreadHandle = NULL ;
}

// スレッドが有効かどうかを取得する( 1:有効  0:無効 )
extern int Thread_IsValid( THREAD_INFO *pThreadInfo )
{
	return pThreadInfo->ThreadHandle != NULL ? 1 : 0 ;
}


// スレッドの実行優先順位を設定する
extern void Thread_SetPriority( THREAD_INFO *pThreadInfo, int Priority /* DX_THREAD_PRIORITY_LOWEST など */ )
{
	int winPriority = -1 ;

	switch( Priority )
	{
	case DX_THREAD_PRIORITY_LOWEST :
		winPriority = THREAD_PRIORITY_LOWEST ;
		break ;

	case DX_THREAD_PRIORITY_BELOW_NORMAL :
		winPriority = THREAD_PRIORITY_BELOW_NORMAL ;
		break ;

	case DX_THREAD_PRIORITY_NORMAL :
		winPriority = THREAD_PRIORITY_NORMAL ;
		break ;

	case DX_THREAD_PRIORITY_HIGHEST :
		winPriority = THREAD_PRIORITY_HIGHEST ;
		break ;

	default :
		return ;
	}

	SetThreadPriority( pThreadInfo->ThreadHandle, winPriority ) ;
}

// カレントスレッドのＩＤを取得する
extern DWORD Thread_GetCurrentId( void )
{
	return GetCurrentThreadId() ;
}

// スレッドのＩＤを取得する
extern DWORD Thread_GetId( THREAD_INFO *pThreadInfo )
{
	return pThreadInfo->ThreadID ;
}

// スレッドを休止状態にする
extern void Thread_Suspend( void )
{
	SuspendThread( GetCurrentThread() ) ;
}

// スレッドの休止状態を解除する( 0:休止状態じゃなかった  1:休止状態だった )
extern int Thread_Resume( THREAD_INFO *pThreadInfo )
{
	return ResumeThread( pThreadInfo->ThreadHandle ) == 0 ? 0 : 1 ;
}



// 指定時間スレッドを停止する
extern void Thread_Sleep( DWORD MiliSecond )
{
	Sleep( MiliSecond ) ;
}





// クリティカルセクションの初期化
extern int CriticalSection_Initialize( DX_CRITICAL_SECTION *pCSection )
{
	InitializeCriticalSection( &pCSection->CriticalSection ) ;

#if !defined( __BCC ) || defined( _DEBUG )
	int i ;
	for( i = 0 ; i < 256 ; i ++ )
	{
		pCSection->FilePath[ 0 ] = '\0' ;
	}
#endif

	return 0 ;
}

// クリティカルセクションの削除
extern int CriticalSection_Delete( DX_CRITICAL_SECTION *pCSection )
{
	DeleteCriticalSection( &pCSection->CriticalSection ) ;

	return 0 ;
}

// クリティカルセクションのロックの取得
#if !defined( __BCC ) || defined( _DEBUG )
extern int CriticalSection_Lock( DX_CRITICAL_SECTION *pCSection, const char *FilePath, int LineNo )
#else
extern int CriticalSection_Lock( DX_CRITICAL_SECTION *pCSection )
#endif
{
#if !defined( __BCC ) || defined( _DEBUG )
	DWORD ThreadID = GetCurrentThreadId() ;
#endif

	EnterCriticalSection( &pCSection->CriticalSection ) ;

#if !defined( __BCC ) || defined( _DEBUG )
	int Length = _STRLEN( FilePath ) ;
	if( Length >= 256 ) Length = 255 ;
	_MEMCPY( pCSection->FilePath, FilePath, Length ) ;
	pCSection->FilePath[ Length ] = '\0' ;
	pCSection->LineNo = LineNo ;
	pCSection->ThreadID = ThreadID ;
#endif

	return 0 ;
}

// クリティカルセクションのロックを解放する
extern int CriticalSection_Unlock( DX_CRITICAL_SECTION *pCSection )
{
	LeaveCriticalSection( &pCSection->CriticalSection ) ;

	return 0 ;
}




}

