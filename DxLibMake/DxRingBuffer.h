﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		リングバッファプログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXRINGBUFFER_H__
#define __DXRINGBUFFER_H__

// Include ------------------------------------------------------------------

#include "DxCompileConfig.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// リングバッファデータ構造体
struct RINGBUF
{
	char					*DataBuffer ;						// データ領域のポインタ
	int						Start, End ;						// 有効データの始まりと終り
	int						DataLength ;						// 有効なデータの長さ
	int						BufferSize ;						// リングバッファの大きさ
} ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// リングバッファ
extern	int			RingBufInitialize( RINGBUF * RingBuf ) ;									// リングバッファを初期化する
extern	int			RingBufTerminate( RINGBUF * RingBuf ) ;										// リングバッファの後始末をする
extern	int			RingBufDataAdd( RINGBUF * RingBuf, const void *Data, int Len ) ;			// リングバッファにデータを追加する
extern	int			RingBufDataGet( RINGBUF * RingBuf, void *Buf, int Len, int PeekFlag) ;		// リングバッファからデータを読みこむ
extern	int			RingBufGetDataLength( RINGBUF * RingBuf ) ;									// リングバッファの有効データ量の取得 
extern	int			RingBufGetRestDataLength( RINGBUF * RingBuf ) ;								// リングバッファの残り空きデータ領域を取得
extern	int			RingBufReSize( RINGBUF * RingBuf, int AddSize ) ;							// リングバッファの容量を指定量のデータを追加しても収まるサイズに調整する

// リングバッファのアドレスに不正がないかチェックする
__inline void RingBufAddressCheck( RINGBUF * RingBuf )
{
	if( RingBuf->Start == RingBuf->BufferSize ) RingBuf->Start = 0 ;
	if( RingBuf->End == RingBuf->BufferSize ) RingBuf->End = 0 ;
}

}

#endif // __DXRINGBUFFER_H__

