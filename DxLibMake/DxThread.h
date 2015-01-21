// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		スレッド関係プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXTHREAD_H__
#define __DXTHREAD_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"

#if !defined( __ANDROID )
#include "Windows/DxThreadWin.h"
#endif

#ifdef __ANDROID
#include "Android/DxThreadAndroid.h"
#endif

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 優先順位
#define DX_THREAD_PRIORITY_LOWEST			(0)
#define DX_THREAD_PRIORITY_BELOW_NORMAL		(1)
#define DX_THREAD_PRIORITY_NORMAL			(2)
#define DX_THREAD_PRIORITY_HIGHEST			(3)

#if !defined( __BCC ) || defined( _DEBUG )
	#define CRITICALSECTION_LOCK( csection )			CriticalSection_Lock( (csection), __FILE__, __LINE__ )
#else
	#define CRITICALSECTION_LOCK( csection )			CriticalSection_Lock( (csection) )
#endif

// 型定義 ------------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	void		Thread_Initialize( void ) ;																				// スレッドの処理を初期化する
extern	int			Thread_Create( THREAD_INFO *pThreadInfo, void ( *pFunction )( THREAD_INFO *, void * ), void *pParam ) ;	// スレッドを作成する
extern	void		Thread_Delete( THREAD_INFO *pThreadInfo ) ;																// スレッドの後始末を行う
extern	int			Thread_IsValid( THREAD_INFO *pThreadInfo ) ;															// スレッドが有効かどうかを取得する( 1:有効  0:無効 )
extern	void		Thread_SetPriority( THREAD_INFO *pThreadInfo, int Priority /* DX_THREAD_PRIORITY_LOWEST など */ ) ;		// スレッドの実行優先順位を設定する
extern	DWORD		Thread_GetCurrentId( void ) ;																			// カレントスレッドのＩＤを取得する
extern	DWORD		Thread_GetId( THREAD_INFO *pThreadInfo ) ;																// スレッドのＩＤを取得する
extern	void		Thread_Suspend( void ) ;																				// スレッドを休止状態にする
extern	int			Thread_Resume( THREAD_INFO *pThreadInfo ) ;																// スレッドの休止状態を解除する( 0:休止状態じゃなかった  1:休止状態だった )

extern	void		Thread_Sleep( DWORD MiliSecond ) ;																		// 指定時間スレッドを停止する

extern	int			CriticalSection_Initialize( DX_CRITICAL_SECTION *pCSection ) ;									// クリティカルセクションの初期化
extern	int			CriticalSection_Delete( DX_CRITICAL_SECTION *pCSection ) ;										// クリティカルセクションの削除

#if !defined( __BCC ) || defined( _DEBUG )
extern	int			CriticalSection_Lock( DX_CRITICAL_SECTION *pCSection, const char *FilePath, int LineNo ) ;		// クリティカルセクションのロックの取得
#else
extern	int			CriticalSection_Lock( DX_CRITICAL_SECTION *pCSection ) ;										// クリティカルセクションのロックの取得
#endif
extern	int			CriticalSection_Unlock( DX_CRITICAL_SECTION *pCSection ) ;										// クリティカルセクションのロックを解放する

}

#endif // __DXTHREAD_H__
