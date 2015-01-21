// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ハンドル管理プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ------------------------------------------------------------------
#include "DxHandle.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxASyncLoad.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxLog.h"

#ifndef DX_NON_ASYNCLOAD
#include "DxGraphics.h"
#include "DxSoftImage.h"
#include "DxSound.h"
#include "DxMask.h"
#include "DxNetwork.h"
#include "DxModel.h"
#include "DxFile.h"
#include "DxFont.h"
#endif // DX_NON_ASYNCLOAD

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

HANDLEMANAGE HandleManageArray[ DX_HANDLETYPE_MAX ] ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// プログラム --------------------------------------------------------------------


// ハンドル共通関係

// ハンドル管理情報を初期化する
// ( InitializeFlag には FALSE が入っている必要がある )
extern int InitializeHandleManage(
	int HandleType,
	int OneSize,
	int MaxNum,
	int ( *InitializeFunction )( HANDLEINFO *HandleInfo ),
	int ( *TerminateFunction )( HANDLEINFO *HandleInfo ),
	const TCHAR *Name
)
{
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	// 既に初期化されていたら何もしない
	if( HandleManage->InitializeFlag )
		return -1 ;

	// NextID以外ゼロ初期化
	int NextID = HandleManage->NextID ;
	_MEMSET( HandleManage, 0, sizeof( *HandleManage ) ) ;
	HandleManage->NextID = NextID ;

	// パラメータセット
	HandleManage->HandleTypeMask = HandleType << DX_HANDLETYPE_ADDRESS ;
	HandleManage->OneSize = OneSize ;
	HandleManage->MaxNum = MaxNum ;
	HandleManage->InitializeFunction = InitializeFunction ;
	HandleManage->TerminateFunction = TerminateFunction ;
	HandleManage->Name = Name ;

	// ハンドルのデータポインタアドレスを格納するメモリアドレス配列の確保
	HandleManage->Handle = ( HANDLEINFO ** )DXCALLOC( sizeof( HANDLEINFO * ) * MaxNum ) ;
	if( HandleManage->Handle == NULL )
		return -1 ;

	// ハンドルリストの初期化
	InitializeHandleList( &HandleManage->ListFirst, &HandleManage->ListLast ) ;

	// クリティカルセクションの初期化
	CriticalSection_Initialize( &HandleManage->CriticalSection ) ;

	// 初期化フラグを立てる
	HandleManage->InitializeFlag = TRUE ;

	// 終了
	return 0 ;
}

// ハンドル管理情報の後始末を行う
extern int TerminateHandleManage( int HandleType )
{
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	// 既に後始末されていたら何もしない
	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// すべてのハンドルを削除
	AllHandleSub( HandleType ) ;

	// ハンドルポインタ格納用のメモリアドレス配列を解放
	if( HandleManage->Handle != NULL )
	{
		DXFREE( HandleManage->Handle ) ;
		HandleManage->Handle = NULL ;
	}

	// クリティカルセクションの削除
	CriticalSection_Delete( &HandleManage->CriticalSection ) ;

	// NextID以外ゼロ初期化
	int NextID = HandleManage->NextID ;
	_MEMSET( HandleManage, 0, sizeof( *HandleManage ) ) ;
	HandleManage->NextID = NextID ;

	// 初期化フラグを倒す
	HandleManage->InitializeFlag = FALSE ;

	// 終了
	return 0 ;
}

// ハンドルを追加する
extern int AddHandle( int HandleType, int Handle )
{
	int NextIndex ;
	int NewHandle ;
	HANDLEINFO **ppHandleInfo ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

	// 追加できない場合は終了
	if( HandleManage->Num == HandleManage->MaxNum )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		DXST_ERRORLOGFMT_ADD(( _T( "%sハンドルの数が限界数( %d )に達していて新たなハンドルを作成できません" ), HandleManage->Name, HandleManage->MaxNum )) ;
		return -1 ;
	}

	// 空き配列番号の検索
	if( Handle != -1 && HandleManage->Handle[ Handle & DX_HANDLEINDEX_MASK ] == NULL )
	{
		NextIndex = Handle & DX_HANDLEINDEX_MASK ;
		ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
	}
	else
	{
		if( HandleManage->Num == 0 )
		{
			NextIndex = 0 ;
			ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
		}
		else
		{
			if( HandleManage->AreaMax + 1 < HandleManage->MaxNum )
			{
				NextIndex = HandleManage->AreaMax + 1 ;
				ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
			}
			else
			if( HandleManage->AreaMin - 1 > 0 )
			{
				NextIndex = HandleManage->AreaMin - 1 ;
				ppHandleInfo = &HandleManage->Handle[ NextIndex ] ;
			}
			else
			{
				ppHandleInfo = HandleManage->Handle ;
				for( NextIndex = 0 ; *ppHandleInfo != NULL ; NextIndex ++, ppHandleInfo ++ ){}
			}
		}
	}

	// データ領域を確保する
	if( ( *ppHandleInfo = ( HANDLEINFO * )DXCALLOC( HandleManage->OneSize ) ) == NULL )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		DXST_ERRORLOGFMT_ADD(( _T( "%sハンドルのデータを格納するメモリ領域の確保に失敗しました" ), HandleManage->Name )) ;
		return -1 ;
	}

	// エラーチェック用ＩＤの設定
	if( Handle != -1 )
	{
		(*ppHandleInfo)->ID = ( Handle & DX_HANDLECHECK_MASK ) >> DX_HANDLECHECK_ADDRESS ;
	}
	else
	{
		HandleManage->NextID ++ ;
		if( HandleManage->NextID >= ( DX_HANDLECHECK_MASK >> DX_HANDLECHECK_ADDRESS ) )
			HandleManage->NextID = 0 ;

		(*ppHandleInfo)->ID = HandleManage->NextID ;
	}

	// メモリの確保サイズを保存
	( *ppHandleInfo )->AllocSize = HandleManage->OneSize ;

	// ハンドル値を保存
	NewHandle = (*ppHandleInfo)->Handle = NextIndex | HandleManage->HandleTypeMask | ( (*ppHandleInfo)->ID << DX_HANDLECHECK_ADDRESS ) ;

	// ハンドルの数を増やす
	HandleManage->Num ++ ;

	// 使用されているハンドルが存在する範囲を更新する
	if( HandleManage->Num == 1 )
	{
		HandleManage->AreaMax = NextIndex ;
		HandleManage->AreaMin = NextIndex ;
	}
	else
	{
		if( HandleManage->AreaMax < NextIndex ) HandleManage->AreaMax = NextIndex ;
		if( HandleManage->AreaMin > NextIndex ) HandleManage->AreaMin = NextIndex ;
	}

	// リストへ要素を追加
	AddHandleList( &HandleManage->ListFirst, &(*ppHandleInfo)->List, (*ppHandleInfo)->Handle, *ppHandleInfo ) ;

	// 初期化関数を呼ぶ
	if( HandleManage->InitializeFunction )
	{
		HandleManage->InitializeFunction( *ppHandleInfo ) ;
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// 返回句柄
	return NewHandle ;
}

// ハンドルを削除する
extern int SubHandle( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int Index ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

	Index = Handle & DX_HANDLEINDEX_MASK ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		return -1 ;
	}

	// 削除フラグに-1を代入する
	if( HandleInfo->DeleteFlag != NULL )
	{
		*HandleInfo->DeleteFlag = -1 ;
	}

#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込み中である場合でまだ処理が走っていなかったら処理をキャンセルする
	if( HandleInfo->ASyncLoadCount != 0 )
	{
		if( DeleteASyncLoadData( HandleInfo->ASyncDataNumber, TRUE ) < -1 )
		{
			// クリティカルセクションの解放
			CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

			// 削除に失敗したら読み込み処理が終わるまで待つ
			while( HandleInfo->ASyncLoadCount != 0 )
			{
				ProcessASyncLoadRequestMainThread() ;
				Sleep( 0 );
			}

			// クリティカルセクションの取得
			CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;
		}
	}
#endif // DX_NON_ASYNCLOAD

	// ハンドルタイプ個々の後始末処理
	if( HandleManage->TerminateFunction )
	{
		// 戻り値が１の場合は削除キャンセル
		if( HandleManage->TerminateFunction( HandleInfo ) == 1 )
			goto END ;
	}

	// リストから要素を外す
	SubHandleList( &HandleInfo->List ) ;

	// データ領域を解放する
	DXFREE( HandleInfo ) ;

	// テーブルに NULL をセットする
	HandleManage->Handle[ Index ] = NULL ;

	// ハンドルの総数を減らす
	HandleManage->Num -- ;

	// 有効なハンドルが存在する範囲の更新
	if( HandleManage->Num == 0 )
	{
		HandleManage->AreaMax = 0 ;
		HandleManage->AreaMin = 0 ;
	}
	else
	{
		if( Index == HandleManage->AreaMax )
		{
			while( HandleManage->Handle[ HandleManage->AreaMax ] == NULL )
				HandleManage->AreaMax -- ; 
		}
		else
		if( Index == HandleManage->AreaMin )
		{
			while( HandleManage->Handle[ HandleManage->AreaMin ] == NULL )
				HandleManage->AreaMin ++ ; 
		}
	}

END :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// 終了
	return 0 ;
}

// ハンドルの情報を格納するメモリ領域のサイズを変更する、非同期読み込み中でないことが前提
extern int ReallocHandle( int Handle, int NewSize )
{
	HANDLEINFO *HandleInfo ;
	int Index ;
	void *NewBuffer ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	Index = Handle & DX_HANDLEINDEX_MASK ;

	// 確保メモリのサイズが変化しない場合は何もせず終了
	if( HandleInfo->AllocSize == NewSize )
		return 0 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

	// メモリの再確保
	NewBuffer = ( HANDLEINFO * )DXREALLOC( HandleInfo, NewSize ) ;

	// メモリの再確保成功チェック
	if( NewBuffer == NULL )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

		// エラー終了
		return -1 ;
	}

	// 確保メモリアドレスが変化した場合はリストのつなぎ直し
	if( NewBuffer != ( void * )HandleInfo )
	{
		// 新しいメモリアドレスをセット
		HandleInfo = HandleManage->Handle[ Index ] = ( HANDLEINFO * )NewBuffer ;

		// リストの繋ぎ直し
		NewMemoryHandleList( &HandleInfo->List, HandleInfo ) ;
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// 終了
	return 0 ;
}

// ハンドルの情報を取得する
extern HANDLEINFO *GetHandleInfo( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return NULL ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return NULL ;

	// ハンドルの情報を返す
	return HandleInfo ;
}

// ハンドル管理情報に登録されているすべてのハンドルを削除
extern int AllHandleSub( int HandleType, int (*DeleteCancelCheckFunction)( HANDLEINFO *HandleInfo ) )
{
	int i ;
	HANDLEINFO **ppHandleInfo ;
	HANDLEMANAGE *HandleManage = &HandleManageArray[ HandleType ] ;

	if( HandleManage->InitializeFlag == FALSE )
		return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManage->CriticalSection ) ;

#ifndef DX_NON_ASYNCLOAD
	// 非同期読み込み依頼があったら削除しておく
	ppHandleInfo = &HandleManage->Handle[ HandleManage->AreaMin ] ;
	for( i = HandleManage->AreaMin ; i <= HandleManage->AreaMax ; i ++, ppHandleInfo ++ )
	{
		if( *ppHandleInfo )
		{
			if( DeleteCancelCheckFunction == NULL ||
				DeleteCancelCheckFunction( *ppHandleInfo ) == FALSE )
			{
				if( (*ppHandleInfo)->ASyncLoadCount != 0 )
				{
					if( DeleteASyncLoadData( (*ppHandleInfo)->ASyncDataNumber, TRUE ) >= -1 )
					{
						(*ppHandleInfo)->ASyncLoadCount = 0 ;
					}
				}
			}
		}
	}
#endif // DX_NON_ASYNCLOAD

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManage->CriticalSection ) ;

	// すべてのハンドルデータを削除する
	ppHandleInfo = &HandleManage->Handle[ HandleManage->AreaMin ] ;
	for( i = HandleManage->AreaMin ; i <= HandleManage->AreaMax ; i ++, ppHandleInfo ++ )
	{
		if( *ppHandleInfo )
		{
			if( DeleteCancelCheckFunction == NULL ||
				DeleteCancelCheckFunction( *ppHandleInfo ) == FALSE )
			{
				SubHandle( (*ppHandleInfo)->Handle ) ;
			}
		}
	}

	// 終了
	return 0 ;
}









#ifndef DX_NON_ASYNCLOAD

// 非同期読み込み関係

// ハンドルの非同期読み込みが完了しているかどうかを取得する( TRUE:完了している  FALSE:まだ完了していない  -1:エラー )
extern int NS_CheckHandleASyncLoad( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	return HandleInfo->ASyncLoadCount != 0 ? TRUE : FALSE ;
}

// ハンドルの非同期読み込み処理の戻り値を取得する( 非同期読み込み中の場合は一つ前の非同期読み込み処理の戻り値が返ってきます )
extern int NS_GetHandleASyncLoadResult( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	return HandleInfo->ASyncLoadResult ;
}

// ハンドルの非同期読み込み中カウントをインクリメントする
extern int IncASyncLoadCount( int Handle, int ASyncDataNumber )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	HandleInfo->ASyncLoadCount ++ ;
	HandleInfo->ASyncDataNumber = ASyncDataNumber ;

	// 終了
	return 0 ;
}

// ハンドルの非同期読み込み中カウントをデクリメントする
extern int DecASyncLoadCount( int Handle )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	HandleInfo->ASyncLoadCount -- ;

	// 終了
	return 0 ;
}

#endif // DX_NON_ASYNCLOAD









// ハンドルリスト

// リストの初期化
extern int InitializeHandleList( HANDLELIST *First, HANDLELIST *Last )
{
	First->Handle = -1 ;
	First->Data   = NULL ;
	First->Prev   = NULL ;
	First->Next   = Last ;
	Last->Handle  = -1 ;
	Last->Data    = NULL ;
	Last->Prev    = First ;
	Last->Next    = NULL ;

	// 終了
	return 0 ;
}

// リストへ要素を追加
extern int AddHandleList( HANDLELIST *First, HANDLELIST *List, int Handle, void *Data )
{
	List->Handle      = Handle ;
	List->Data        = Data ;
	List->Prev        = First ;
	List->Next        = First->Next ;
	First->Next->Prev = List ;
	First->Next       = List ;

	// 終了
	return 0 ;
}

// リストから要素を外す
extern int SubHandleList( HANDLELIST *List )
{
	List->Prev->Next = List->Next ;
	List->Next->Prev = List->Prev ;

	return 0 ;
}

// リストが存在するメモリが変更された場合にリストの前後を更新する
extern int NewMemoryHandleList( HANDLELIST *List, void *Data )
{
	List->Data = Data ;

	List->Prev->Next = List ;
	List->Next->Prev = List ;

	// 終了
	return 0 ;
}







// ハンドルが削除されたときに－１が設定される変数を登録する
extern int NS_SetDeleteHandleFlag( int Handle, int *DeleteFlag )
{
	HANDLEINFO *HandleInfo ;
	int HandleType = ( int )( ( ( DWORD )Handle & DX_HANDLETYPE_MASK ) >> DX_HANDLETYPE_ADDRESS ) ;

	// エラー判定
	if( HANDLECHK_ASYNC( HandleType, Handle, HandleInfo ) )
		return -1 ;

	HandleInfo->DeleteFlag = DeleteFlag  ;

	// 終了
	return 0 ;
}










}
