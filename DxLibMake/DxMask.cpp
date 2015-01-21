//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		マスクデータ管理プログラム
// 
//  	Ver 3.11f
// 
//-----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

#include "DxCompileConfig.h"

#ifndef DX_NON_MASK

// インクルード----------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxMask.h"
#include "DxFont.h"
#include "DxUseCLib.h"
#include "DxFile.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxGraphicsBase.h"
#include "DxLog.h"
#include "Windows/DxWindow.h"
#include "DxGraphics.h"
#include "DxASyncLoad.h"

namespace DxLib
{

// マクロ定義------------------------------------------------------------------

// マスクハンドルの有効性チェック
#define MASKHCHK( HAND, MPOINT )		HANDLECHK(       DX_HANDLETYPE_GMASK, HAND, *( ( HANDLEINFO ** )&MPOINT ) )
#define MASKHCHK_ASYNC( HAND, MPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_GMASK, HAND, *( ( HANDLEINFO ** )&MPOINT ) )

// FVFの変更
#define SETFVF( type )	\
	if( GRH.SetFVF != ( type ) )\
	{\
		GraphicsDevice_SetFVF( ( type ) ) ;\
		GRH.SetFVF = ( type ) ;\
	}

// 型定義----------------------------------------------------------------------

// データ宣言------------------------------------------------------------------

MASKMANAGEDATA MaskManageData ;

// 関数プロトタイプ宣言 -------------------------------------------------------

static int UpdateMaskImageTexture( RECT Rect ) ;						// 指定領域のマスクイメージテクスチャを更新する

// プログラム------------------------------------------------------------------

// マスク処理の初期化
extern int InitializeMask( void )
{
	if( MASKD.InitializeFlag == TRUE )
		return -1 ;

	// マスクハンドル管理データの初期化
	InitializeHandleManage( DX_HANDLETYPE_GMASK, sizeof( MASKDATA ), MAX_MASK_NUM, InitializeMaskHandle, TerminateMaskHandle, DXSTRING( _T( "マスク" ) ) ) ;

	// 初期化フラグを立てる
	MASKD.InitializeFlag = TRUE ;

	// 終了
	return 0 ;
}

// マスク処理の後始末
extern int TerminateMask( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// マスクスクリーンを削除する
	NS_DeleteMaskScreen() ;

	// 作成したマスクグラフィックを削除
	NS_InitMask() ;

	// 初期化フラグを倒す
	MASKD.InitializeFlag = FALSE ;

	// マスクハンドルの後始末
	TerminateHandleManage( DX_HANDLETYPE_GMASK ) ;

	// 終了
	return 0 ;
}

// マスクデータを初期化する
extern int NS_InitMask( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// ハンドルを初期化
	return AllHandleSub( DX_HANDLETYPE_GMASK ) ;
}

// マスクスクリーンを作成する
extern int NS_CreateMaskScreen( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// マスク保存用サーフェスとマスク描画用サーフェスの作成
	CreateMaskScreenFunction( TRUE, GBASE.DrawSizeX, GBASE.DrawSizeY ) ; 

	// マスクサーフェスが作成されているかフラグをたてる
	MASKD.CreateMaskFlag = TRUE ;
	MASKD.MaskValidFlag = MASKD.MaskUseFlag && MASKD.CreateMaskFlag ;

//	// もしマスク使用フラグが立っていたらマスクを有効にする作業を行う
//	if( MASKD.MaskUseFlag )
		NS_SetUseMaskScreenFlag( TRUE ) ;

	// 終了
	return 0 ;
}

// マスクサーフェスの作成
extern int CreateMaskSurface( BYTE **MaskBuffer, int *BufferPitch, int Width, int Height, int *TransModeP )
{
	// ピッチの算出
	*BufferPitch = ( Width + 3 ) / 4 * 4 ;

	// バッファの作成
	*MaskBuffer = ( BYTE * )DXALLOC( *BufferPitch * Height ) ;
	if( *MaskBuffer == NULL )
		return -1 ;

	// バッファの初期化
	_MEMSET( *MaskBuffer, 0, *BufferPitch * Height ) ;

	// 透過色モードのセット
	*TransModeP = DX_MASKTRANS_BLACK ;

	// 終了
	return 0 ;
}

// マスクハンドルの初期化
extern int InitializeMaskHandle( HANDLEINFO * )
{
	// 特に何もせず終了
	return 0 ;
}

// マスクハンドルの後始末
extern int TerminateMaskHandle( HANDLEINFO *HandleInfo )
{
	MASKDATA *Mask ;

	Mask = ( MASKDATA * )HandleInfo ;

	// マスク用データ領域の解放
	if( Mask->SrcData )
	{
		DXFREE( Mask->SrcData ) ;
		Mask->SrcData = NULL ;
	}

	// 終了
	return 0 ;
}

// MakeMask の実処理関数
static int MakeMask_Static(
	int MaskHandle,
	int Width,
	int Height,
	int /*ASyncThread*/
)
{
	MASKDATA *Mask ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK_ASYNC( MaskHandle, Mask ) )
		return -1 ;

	// マスク保存用サーフェスの作成
	if( CreateMaskSurface( &Mask->SrcData, &Mask->SrcDataPitch, Width, Height, &Mask->TransMode ) < 0 )
		return -1 ;

	// データのセット
	Mask->MaskWidth = Width ;
	Mask->MaskHeight = Height ;

	// 正常終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// MakeMask の非同期読み込みスレッドから呼ばれる関数
static void MakeMask_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MaskHandle ;
	int Width ;
	int Height ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MaskHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Width = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Height = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = MakeMask_Static( MaskHandle, Width, Height, TRUE ) ;

	DecASyncLoadCount( MaskHandle ) ;
	if( Result < 0 )
	{
		SubHandle( MaskHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// マスクデータの追加
extern int MakeMask_UseGParam( int Width, int Height, int ASyncLoadFlag )
{
	int MaskHandle ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	MaskHandle = AddHandle( DX_HANDLETYPE_GMASK ) ;
	if( MaskHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, MaskHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, Width ) ;
		AddASyncLoadParamInt( NULL, &Addr, Height ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = MakeMask_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MaskHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Width ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Height ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MaskHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MakeMask_Static( MaskHandle, Width, Height, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return MaskHandle ;

ERR :
	SubHandle( MaskHandle ) ;

	return -1 ;
}

// マスクデータの追加
extern int NS_MakeMask( int Width, int Height )
{
	return MakeMask_UseGParam( Width, Height, GetASyncLoadFlag() ) ;
}

// マスクデータを削除
extern int NS_DeleteMask( int MaskHandle )
{
	return SubHandle( MaskHandle ) ;
}

// マスクデータサーフェスにＢＭＰデータをマスクデータと見たてて転送
extern int NS_BmpBltToMask( HBITMAP Bmp, int BmpPointX, int BmpPointY, int MaskHandle )
{
	MASKDATA * MaskData ;
	BITMAP bm ;
	int ColorBitDepth ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK_ASYNC( MaskHandle, MaskData ) )
		return -1 ;

	// ビットマップデータの取得
	GetObject( Bmp, sizeof( BITMAP ), ( void * )&bm ) ;
	bm.bmWidthBytes += bm.bmWidthBytes % 4 ? 4 - bm.bmWidthBytes % 4 : 0 ;

	// 転送処理
	ColorBitDepth = NS_GetScreenBitDepth() ;
	{
		BYTE *SrcPoint, *DestPoint ;
		int SrcAddIndex, DestAddIndex ;
		int DestHeight, DestWidth ;

		SrcPoint = ( BYTE *)bm.bmBits + BmpPointX * bm.bmBitsPixel / 8 + BmpPointY * bm.bmWidthBytes ;
		DestPoint = ( BYTE * )MaskData->SrcData ;

		SrcAddIndex = bm.bmWidthBytes - MaskData->MaskWidth * bm.bmBitsPixel / 8 ;
		DestAddIndex = MaskData->SrcDataPitch - MaskData->MaskWidth ;

		DestHeight = MaskData->MaskHeight ;
		DestWidth = MaskData->MaskWidth ;

#ifdef DX_NON_INLINE_ASM
		int i ;
		do
		{
			i = DestWidth ;
			do
			{
				*DestPoint = *SrcPoint ;
				DestPoint ++ ;
				SrcPoint += 3 ;
			}while( -- i != 0 ) ;
			DestPoint += DestAddIndex ;
			SrcPoint += SrcAddIndex ;
		}while( -- DestHeight != 0 ) ;
#else
		__asm{
			PUSH	EDI
			PUSH	ESI
			PUSHF
			CLD
			MOV		EDI, DestPoint
			MOV		ESI, SrcPoint
			MOV		EDX, DestHeight
			MOV		EBX, DestWidth
		LOOP81:
			MOV		ECX, EBX
		LOOP82:
			MOV		AL	, [ESI]
			MOV		[EDI], AL
			INC		EDI
			ADD		ESI, 3
			LOOP	LOOP82

			ADD		ESI, SrcAddIndex
			ADD		EDI, DestAddIndex
			DEC		EDX
			JNZ		LOOP81

			POPF
			POP		ESI
			POP		EDI
		}
#endif
	}

	// 終了
	return 0 ;
}

// マスクの大きさを得る 
extern int NS_GetMaskSize( int *WidthBuf, int *HeightBuf, int MaskHandle )
{
	MASKDATA * Mask ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	*WidthBuf = Mask->MaskWidth ;
	*HeightBuf = Mask->MaskHeight ;

	// 終了
	return 0 ;
}

// LoadMask の実処理関数
static int LoadMask_Static(
	int MaskHandle,
	const TCHAR *FileName,
	int /*ASyncThread*/
)
{
	HBITMAP Bmp ;
	BITMAP bm ;
	COLORDATA SrcColor;
	MASKDATA * Mask ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK_ASYNC( MaskHandle, Mask ) )
		return -1 ;

	// ロードを試みる
	NS_CreateFullColorData( &SrcColor );
	if( ( Bmp = NS_CreateDIBGraph( FileName, FALSE, &SrcColor ) ) == NULL ) return -1 ;

	// グラフィックのデータを取得
	GetObject( Bmp, sizeof( BITMAP ), ( void * )&bm ) ;
	
	// マスク保存用サーフェスの作成
	if( CreateMaskSurface( &Mask->SrcData, &Mask->SrcDataPitch, bm.bmWidth, bm.bmHeight, &Mask->TransMode ) < 0 )
	{
		DeleteObject( Bmp ) ;
		return -1 ;
	}

	// データのセット
	Mask->MaskWidth = bm.bmWidth ;
	Mask->MaskHeight = bm.bmHeight ;

	// マスクデータを転送
	NS_BmpBltToMask( Bmp, 0, 0, MaskHandle ) ;

	// ＢＭＰオブジェクトを削除
	DeleteObject( Bmp ) ;

	// 終了
	return MaskHandle ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadMask の非同期読み込みスレッドから呼ばれる関数
static void LoadMask_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MaskHandle ;
	const TCHAR *FileName ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MaskHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = LoadMask_Static( MaskHandle, FileName, TRUE ) ;

	DecASyncLoadCount( MaskHandle ) ;
	if( Result < 0 )
	{
		SubHandle( MaskHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// マスクデータをロードする 
extern int LoadMask_UseGParam( const TCHAR *FileName, int ASyncLoadFlag )
{
	int MaskHandle ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	MaskHandle = AddHandle( DX_HANDLETYPE_GMASK ) ;
	if( MaskHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FileName, FullPath ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, MaskHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadMask_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MaskHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MaskHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadMask_Static( MaskHandle, FileName, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return MaskHandle ;

ERR :
	SubHandle( MaskHandle ) ;

	return -1 ;
}

// マスクデータをロードする
extern int NS_LoadMask( const TCHAR *FileName )
{
	return LoadMask_UseGParam( FileName, GetASyncLoadFlag() ) ;
}

// LoadDivMask の実処理関数
static int LoadDivMask_Static(
	const TCHAR *FileName,
	int AllNum,
	int XNum,
	int YNum,
	int XSize,
	int YSize,
	int *HandleBuf,
	int /*ASyncThread*/
)
{
	MASKDATA * Mask ;
	HBITMAP Bmp ;
	BITMAP bm ;
	int i, j, k ;
	COLORDATA SrcColor;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// ロードを試みる
	Bmp = NULL ;
	NS_CreateFullColorData( &SrcColor );
	if( ( Bmp = NS_CreateDIBGraph( FileName, FALSE, &SrcColor ) ) == NULL )
		return -1 ;

	// グラフィックのデータを取得
	GetObject( Bmp, sizeof( BITMAP ), ( void * )&bm ) ;

	// サイズ確認
	if( XNum * XSize > bm.bmWidth || YNum * YSize > bm.bmHeight )
		goto ERR ;

	// 分割転送開始
	k = 0 ;
	for( i = 0 ; k != AllNum && i < YNum ; i ++ )
	{
		for( j = 0 ; k != AllNum && j < XNum ; j ++, k ++ )
		{
			if( MASKHCHK_ASYNC( HandleBuf[ k ], Mask ) )
				goto ERR ;

			// マスクの作成
			if( CreateMaskSurface( &Mask->SrcData, &Mask->SrcDataPitch, XSize, YSize, &Mask->TransMode ) < 0 )
				goto ERR ;

			// データのセット
			Mask->MaskWidth = XSize ;
			Mask->MaskHeight = YSize ;

			// マスクデータの転送
			NS_BmpBltToMask( Bmp, XSize * j, YSize * i, HandleBuf[ k ] ) ;
		}
	}

	// ＢＭＰの削除
	DeleteObject( Bmp ) ;

	// 終了
	return 0 ;

	// エラー終了
ERR :
	if( Bmp != NULL )
		DeleteObject( Bmp ) ;
	
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadDivMask の非同期読み込みスレッドから呼ばれる関数
static void LoadDivMask_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	const TCHAR *FileName ;
	int AllNum ;
	int XNum ;
	int YNum ;
	int XSize ;
	int YSize ;
	int *HandleBuf ;
	int Addr ;
	int i ;
	int Result ;

	Addr = 0 ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	AllNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	XNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	YNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	XSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	YSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	HandleBuf = ( int * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;

	Result = LoadDivMask_Static( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, TRUE ) ;

	for( i = 0 ; i < AllNum ; i ++ )
		DecASyncLoadCount( HandleBuf[ i ] ) ;

	if( Result < 0 )
	{
		for( i = 0 ; i < AllNum ; i ++ )
			NS_DeleteMask( HandleBuf[ i ] ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// マスクを画像から分割読みこみ
extern int LoadDivMask_UseGParam(
	const TCHAR *FileName,
	int AllNum,
	int XNum,
	int YNum,
	int XSize,
	int YSize,
	int *HandleBuf,
	int ASyncLoadFlag
)
{
	int i ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	if( AllNum == 0 )
		return -1 ;

	// グラフィックハンドルの作成
	_MEMSET( HandleBuf, 0, sizeof( int ) * AllNum ) ;
	for( i = 0 ; i < AllNum ; i ++ )
	{
		HandleBuf[ i ] = AddHandle( DX_HANDLETYPE_GMASK ) ;
		if( HandleBuf[ i ] < 0 )
			goto ERR ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FileName, FullPath ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ; 
		AddASyncLoadParamInt( NULL, &Addr, AllNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, XNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, YNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, XSize ) ;
		AddASyncLoadParamInt( NULL, &Addr, YSize ) ;
		AddASyncLoadParamStruct( NULL, &Addr, HandleBuf, sizeof( int ) * AllNum ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadDivMask_ASync ;
		Addr = 0 ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ; 
		AddASyncLoadParamInt( AParam->Data, &Addr, AllNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, XNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, YNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, XSize ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, YSize ) ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, HandleBuf, sizeof( int ) * AllNum ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		for( i = 0 ; i < AllNum ; i ++ )
			IncASyncLoadCount( HandleBuf[ i ], AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadDivMask_Static( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, FALSE ) < 0 )
			goto ERR ;
	}

	// 正常終了
	return 0 ;

ERR :
	for( i = 0 ; i < AllNum ; i ++ )
	{
		NS_DeleteMask( HandleBuf[ i ] ) ;
	}

	return -1 ;
}

// マスクを画像から分割読みこみ
extern int NS_LoadDivMask( const TCHAR *FileName, int AllNum, int XNum, int YNum, int XSize, int YSize, int *HandleBuf )
{
	return LoadDivMask_UseGParam( FileName, AllNum, XNum, YNum, XSize, YSize, HandleBuf, GetASyncLoadFlag() ) ;
}

#ifndef DX_NON_FONT

// 書式指定ありの文字列をマスクスクリーンに描画する
extern int NS_DrawFormatStringMask( int x, int y, int Flag, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// 編集後の文字列を取得する
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawStringMaskToHandle( x, y, Flag, NS_GetDefaultFontHandle(), String ) ;
}

// 書式指定ありの文字列をマスクスクリーンに描画する
extern int NS_DrawFormatStringMaskToHandle( int x, int y, int Flag, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;
	
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// 編集後の文字列を取得する
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawStringMaskToHandle( x, y, Flag, FontHandle, String ) ;
}

// 文字列をマスクスクリーンに描画する(フォントハンドル指定版)
extern int NS_DrawStringMaskToHandle( int x, int y, int Flag, int FontHandle, const TCHAR *String )
{
	BASEIMAGE Image ;
	int Color ;
	int Result ;
	SIZE DrawSize ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// マスクスクリーンが作成されていなかったらエラー
	if( MASKD.MaskBuffer == NULL )
		return -1 ;

	RefreshDefaultFont() ;

	// BASEIMAGEデータの作成
	NS_CreatePaletteColorData( &Image.ColorData ) ;
	NS_GetDrawScreenSize( &Image.Width, &Image.Height ) ;
	Image.Pitch = MASKD.MaskBufferPitch ;
	Image.GraphData = MASKD.MaskBuffer ;

	// マスクスクリーン上に文字を描画
	Color = Flag == 0 ? 0 : 0xff ;
	Result = NS_FontCacheStringDrawToHandle( x, y, String, Color, Color, 
											&Image, &GBASE.DrawArea, FontHandle,
											FALSE, &DrawSize ) ;

	RECT Rect ;

	if( GRA2.ValidHardWare )
	{
		// 更新
		Rect.left = x ;
		Rect.top = y ;
		Rect.right = x + DrawSize.cx ;
		Rect.bottom = y + DrawSize.cy ;
		UpdateMaskImageTexture( Rect ) ;
	}

	return Result ;
}

// 文字列をマスクスクリーンに描画する
extern int NS_DrawStringMask( int x, int y, int Flag, const TCHAR *String )
{
	return NS_DrawStringMaskToHandle( x, y, Flag, NS_GetDefaultFontHandle(), String ) ;
}

#endif // DX_NON_FONT

// マスクスクリーンを使用中かフラグの取得
extern int NS_GetMaskUseFlag( void )
{
	return MASKD.MaskUseFlag ;
}

// マスクの数値に対する効果を逆転させる
extern int NS_SetMaskReverseEffectFlag( int ReverseFlag )
{
	MASKD.MaskReverseEffectFlag = ReverseFlag ;

	// 終了
	return 0 ;
}

// マスク使用モードの取得
extern int NS_GetUseMaskScreenFlag( void )
{
	return MASKD.MaskUseFlag ;
}

// マスクスクリーンを復旧させる
extern int ReCreateMaskSurface( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// メインのマスクサーフェスを作成する
	CreateMaskScreenFunction( MASKD.CreateMaskFlag, GBASE.DrawSizeX, GBASE.DrawSizeY ) ;

	// もしマスク使用フラグが立っていたらマスクを有効にする作業を行う
	if( MASKD.MaskUseFlag )
		NS_SetUseMaskScreenFlag( TRUE ) ;

	// 終了
	return 0 ;
}

// マスクの作成
extern	int CreateMaskOn2D( int DrawPointX, int DrawPointY ,int DestWidth, int DestHeight, void *DestBufP, int DestPitch, int DestBitDepth, 
					   int SrcWidth, int SrcHeight, void *SrcData )
{
	int SrcPitch ;
	BYTE *DestP, *SrcDataTemp ;
	BYTE *StartSrcPointY ;
	DWORD StartSrcPointX ;
	int TempHeight ;
	int PixelByte ;
	int StartSrcWidth, StartSrcHeight ;

	// 転送準備
	PixelByte = DestBitDepth / 8 ;
	SrcPitch = SrcWidth ;
	DestP = ( BYTE * )DestBufP + DrawPointX * PixelByte + DrawPointY * DestPitch ;
	SrcDataTemp = ( BYTE * )SrcData ;
	DestPitch -= DestBitDepth / 8 * DestWidth ;

	StartSrcPointY = ( BYTE * )SrcData + ( DrawPointY % SrcHeight ) * SrcWidth ; 
	StartSrcPointX = DrawPointX % SrcWidth ;
	StartSrcWidth = SrcWidth - DrawPointX % SrcWidth ;
	StartSrcHeight = SrcHeight - DrawPointY % SrcHeight ; 

	// 転送処理
#ifdef DX_NON_INLINE_ASM
	int i ;
	DWORD TempWidth ;
	BYTE *SrcPoint, *SrcPointX ;

	TempHeight = StartSrcHeight ;
	SrcPointX = StartSrcPointY ;
	switch( DestBitDepth )
	{
	case 8 :
		do
		{
			i = DestWidth ;
			TempWidth = StartSrcWidth ;
			SrcPoint = SrcPointX + StartSrcPointX ;
			do
			{
				*DestP = *SrcPoint ;
				DestP ++ ;
				SrcPoint ++ ;

				if( -- TempWidth == 0 )
				{
					SrcPoint = SrcPointX ;
					TempWidth = SrcWidth ;
				}
			}while( -- i != 0 ) ;

			DestP += DestPitch ;
			if( -- TempHeight == 0 )
			{
				SrcPointX = ( BYTE * )SrcData ;
				TempHeight = SrcHeight ;
			}
			else
			{
				SrcPointX += SrcPitch ;
			}
		}while( -- DestHeight != 0 ) ;
		break ;

	case 16 :
		do
		{
			i = DestWidth ;
			TempWidth = StartSrcWidth ;
			SrcPoint = SrcPointX + StartSrcPointX ;
			do
			{
				DestP[ 0 ] = *SrcPoint ;
				DestP[ 1 ] = *SrcPoint ;
				DestP += 2 ;
				SrcPoint ++ ;

				if( -- TempWidth == 0 )
				{
					SrcPoint = SrcPointX ;
					TempWidth = SrcWidth ;
				}
			}while( -- i != 0 ) ;

			DestP += DestPitch ;
			if( -- TempHeight == 0 )
			{
				SrcPointX = ( BYTE * )SrcData ;
				TempHeight = SrcHeight ;
			}
			else
			{
				SrcPointX += SrcPitch ;
			}
		}while( -- DestHeight != 0 ) ;
		break ;

	case 32 :
		do
		{
			i = DestWidth ;
			TempWidth = StartSrcWidth ;
			SrcPoint = SrcPointX + StartSrcPointX ;
			do
			{
				*( ( DWORD * )DestP ) = *SrcPoint | ( *SrcPoint << 8 ) | ( *SrcPoint << 16 ) ;
				DestP += 4 ;
				SrcPoint ++ ;

				if( -- TempWidth == 0 )
				{
					SrcPoint = SrcPointX ;
					TempWidth = SrcWidth ;
				}
			}while( -- i != 0 ) ;

			DestP += DestPitch ;
			if( -- TempHeight == 0 )
			{
				SrcPointX = ( BYTE * )SrcData ;
				TempHeight = SrcHeight ;
			}
			else
			{
				SrcPointX += SrcPitch ;
			}
		}while( -- DestHeight != 0 ) ;
		break ;
	}
#else
	switch( DestBitDepth ) 
	{
	case 8 :
		__asm{
			PUSHA
			PUSHF
			CLD
			MOV		EDI, DestP
			MOV		EAX, StartSrcHeight
			MOV		TempHeight, EAX
			MOV		ESI, StartSrcPointY
			ADD		ESI, StartSrcPointX
			MOV		EDX, StartSrcPointY
			JMP		LOOP84_a
		LOOP81_a:
			MOV		ESI, SrcData
			MOV		EDX, ESI
			ADD		ESI, StartSrcPointX
			MOV		EAX, SrcHeight
			MOV		TempHeight, EAX
		LOOP84_a:
			MOV		EBX, DestWidth
			MOV		ECX, StartSrcWidth
			JMP		LOOP83_a
		LOOP82_a:
			MOV		ESI, EDX
			MOV		ECX, SrcWidth
		LOOP83_a:
			MOVSB
			DEC		EBX
			JZ		R81_a
			LOOP	LOOP83_a
			JMP		LOOP82_a
		R81_a:
			ADD		EDX, SrcPitch
			MOV		ESI, EDX
			ADD		ESI, StartSrcPointX
			ADD		EDI, DestPitch
			DEC		DestHeight
			JZ		END8_a
			DEC		TempHeight
			JNZ		LOOP84_a
			JMP		LOOP81_a
		END8_a:
			POPF
			POPA
		}
		break ;

	case 16 :
		__asm{
			PUSHA
			PUSHF
			CLD
			MOV		EDI, DestP
			MOV		EAX, StartSrcHeight
			MOV		TempHeight, EAX
			MOV		ESI, StartSrcPointY
			ADD		ESI, StartSrcPointX
			MOV		EDX, StartSrcPointY
			JMP		LOOP164_a
		LOOP161_a:
			MOV		ESI, SrcData
			MOV		EDX, ESI
			ADD		ESI, StartSrcPointX
			MOV		EAX, SrcHeight
			MOV		TempHeight, EAX
		LOOP164_a:
			MOV		EBX, DestWidth
			MOV		ECX, StartSrcWidth
			JMP		LOOP163_a
		LOOP162_a:
			MOV		ESI, EDX
			MOV		ECX, SrcWidth
		LOOP163_a:
			MOV		AL, [ESI]
			MOV		AH, AL
			STOSW
			INC		ESI
			DEC		EBX
			JZ		R161_a
			LOOP	LOOP163_a
			JMP		LOOP162_a
		R161_a:
			ADD		EDX, SrcPitch
			MOV		ESI, EDX
			ADD		ESI, StartSrcPointX
			ADD		EDI, DestPitch
			DEC		DestHeight
			JZ		END16_a
			DEC		TempHeight
			JNZ		LOOP164_a
			JMP		LOOP161_a
		END16_a:
			POPF
			POPA
		}
		break ;

	case 32 :
		__asm{
			PUSHA
			PUSHF
			CLD
			MOV		EDI, DestP
			MOV		EAX, StartSrcHeight
			MOV		TempHeight, EAX
			MOV		ESI, StartSrcPointY
			ADD		ESI, StartSrcPointX
			MOV		EDX, StartSrcPointY
			JMP		LOOP324_a
		LOOP321_a:
			MOV		ESI, SrcData
			MOV		EDX, ESI
			ADD		ESI, StartSrcPointX
			MOV		EAX, SrcHeight
			MOV		TempHeight, EAX
		LOOP324_a:
			MOV		EBX, DestWidth
			MOV		ECX, StartSrcWidth
			JMP		LOOP323_a
		LOOP322_a:
			MOV		ESI, EDX
			MOV		ECX, SrcWidth
		LOOP323_a:
			XOR		EAX, EAX
			MOV		AL, [ESI]
			MOV		AH, AL
			SHL		EAX, 8
			MOV		AL, AH
			STOSD
			INC		ESI
			DEC		EBX
			JZ		R321_a
			LOOP	LOOP323_a
			JMP		LOOP322_a
		R321_a:
			ADD		EDX, SrcPitch
			MOV		ESI, EDX
			ADD		ESI, StartSrcPointX
			ADD		EDI, DestPitch
			DEC		DestHeight
			JZ		END32_a
			DEC		TempHeight
			JNZ		LOOP324_a
			JMP		LOOP321_a
		END32_a:
			POPF
			POPA
		}
		break ;
	}
#endif

	// 終了
	return 0 ;
}


// マスクパターンの展開
extern int DrawMaskToDirectData_Base( int DrawPointX, int DrawPointY, void *DestBufP, int DestPitch, 
											int SrcWidth, int SrcHeight, const void *SrcData, int TransMode  )
{
	BYTE *DestP ,*SrcP ;
	BYTE TransColor ;
	int DestAddPoint ;
	int SrcAddPoint ;
	int BltWidth, BltHeight ;
	RECT Rect ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKD.MaskBuffer == NULL ) return 0 ;

	SETRECT( Rect, DrawPointX, DrawPointY, DrawPointX + SrcWidth, DrawPointY + SrcHeight ) ;
	RectClipping( &Rect, &GBASE.DrawArea ) ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;
	Rect.left -= DrawPointX ;
	Rect.right -= DrawPointX ;
	Rect.top -= DrawPointY ;
	Rect.bottom -= DrawPointY ;
	if( Rect.left > 0 ) DrawPointX += Rect.left ;
	if( Rect.top > 0 ) DrawPointY += Rect.top ;

	// 転送準備
	BltWidth = Rect.right - Rect.left ;
	BltHeight = Rect.bottom - Rect.top ;
	SrcP = ( BYTE * )SrcData + Rect.left + Rect.top * SrcWidth ;
	SrcAddPoint = SrcWidth - BltWidth ;

	DestP = ( BYTE * )DestBufP + DrawPointX + DrawPointY * DestPitch ;
	DestAddPoint = DestPitch - BltWidth ;

	TransColor = TransMode == DX_MASKTRANS_WHITE ? 0xff : 0 ;

	// 透過色があるかどうかで処理を分岐
	if( TransMode == DX_MASKTRANS_NONE )
	{
#ifdef DX_NON_INLINE_ASM
		int i ;
		do
		{
			i = BltWidth ;

			do
			{
				*DestP = *SrcP ;
				DestP ++ ;
				SrcP ++ ;
			}while( -- i != 0 ) ;

			DestP += DestAddPoint ;
			SrcP += SrcAddPoint ;

		}while( -- BltHeight != 0 ) ;
#else
		// 透過色ないバージョン
		__asm{
			CLD
			MOV		EDI, DestP
			MOV		ESI, SrcP
			MOV		EDX, BltWidth
			MOV		EBX, BltHeight
			MOV		EAX, SrcAddPoint
			MOV		ECX, DestAddPoint
			PUSH	EBP
			MOV		EBP, ECX
		LOOP811:
			MOV		ECX, EDX
			REP		MOVSB
			ADD		EDI, EBP
			ADD		ESI, EAX
			DEC		EBX
			JNZ		LOOP811
			POP		EBP
		}
#endif
	}
	else
	{
		// 透過色ありバージョン
		
#ifdef DX_NON_INLINE_ASM
		int i ;
		TransColor &= 0xff ;
		do
		{
			i = BltWidth ;

			do
			{
				if( *SrcP != TransColor )
					*DestP = *SrcP ;
				DestP ++ ;
				SrcP ++ ;
			}while( -- i != 0 ) ;

			DestP += DestAddPoint ;
			SrcP += SrcAddPoint ;

		}while( -- BltHeight != 0 ) ;
#else
		__asm{
			MOV		EDI, DestP
			MOV		ESI, SrcP
			MOV		BL, TransColor
	LOOP821:
			MOV		ECX, BltWidth
	LOOP822:
			MOV		AL, [ESI]
			TEST	AL, BL
			JZ		R821
			MOV		[EDI], AL
	R821:
			INC		EDI
			INC		ESI
			LOOP	LOOP822

			ADD		EDI, DestAddPoint
			ADD		ESI, SrcAddPoint
			DEC		BltHeight
			JNZ		LOOP821
		}
#endif
	}

	// 終了
	return 0 ;
}

// マスクのデータを直接マスク画面全体に描画する
extern int NS_DrawFillMaskToDirectData( int x1, int y1, int x2, int y2,  int Width, int Height, const void *MaskData )
{
	int MaskHandle ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 空のマスクを作成
	if( ( MaskHandle = NS_MakeMask( Width, Height ) ) == -1 ) return -1 ;

	// マスクにデータをセット
	NS_SetDataToMask( Width, Height, MaskData, MaskHandle ) ;

	// マスクを画面全体に描画
	NS_DrawFillMask( x1, y1, x2, y2, MaskHandle ) ;

	// マスクハンドルを削除
	NS_DeleteMask( MaskHandle ) ;

	// 終了
	return 0 ;
}

// マスクのデータをマスクに転送する
extern int NS_SetDataToMask( int Width, int Height, const void *MaskData, int MaskHandle )
{
	MASKDATA * Mask ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;
	
	if( Mask->MaskWidth != Width || Mask->MaskHeight != Height ) return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// マスクデータがなかった場合は処理を終了
	if( MaskData == NULL ) return 0 ;

	// マスクグラフィックにマスクデータを展開
	DrawMaskToDirectData_Base( 0, 0, Mask->SrcData, Mask->SrcDataPitch,
							Width, Height, MaskData, DX_MASKTRANS_NONE ) ;

	// 終了
	return 0 ;
}



// マスク画面上の描画状態を取得する
extern int NS_GetMaskScreenData( int x1, int y1, int x2, int y2, int MaskHandle )
{
	MASKDATA * Mask ; 
	RECT Rect, Rect2 ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	if( MASKD.MaskBuffer == NULL )
		return 0 ;

	// 取得矩形のセット
	SETRECT( Rect, x1, y1, x2, y2 ) ;
	NS_GetDrawScreenSize( ( int * )&Rect2.right, ( int * )&Rect2.bottom ) ;
	Rect2.left = 0 ; Rect2.top = 0 ;
	RectClipping( &Rect, &Rect2 ) ;

	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return -1 ;
	if( Rect.right - Rect.left != Mask->MaskWidth ||
		Rect.bottom - Rect.top != Mask->MaskHeight ) return -1 ;

	// データの転送
	{
		BYTE *Dest, *Src ;
		int DestAddPitch, SrcAddPitch ;
		int DestWidth, DestHeight ;

		Rect.left -= x1 ;
		Rect.right -= x1 ;
		Rect.top -= y1 ;
		Rect.bottom -= y1 ;
		if( Rect.left > 0 ){ x1 += Rect.left ; }
		if( Rect.top  > 0 ){ y1 += Rect.top  ; }

		DestWidth = Rect.right - Rect.left ;
		DestHeight = Rect.bottom - Rect.top ;
		Dest = Mask->SrcData + Rect.left + ( Rect.top * Mask->SrcDataPitch ) ;
		Src = MASKD.MaskBuffer + x1 + ( y1 * MASKD.MaskBufferPitch ) ;

		SrcAddPitch = Mask->SrcDataPitch - DestWidth ;
		DestAddPitch = MASKD.MaskBufferPitch - DestWidth ;

#ifdef DX_NON_INLINE_ASM
		int i ;
		do
		{
			i = DestWidth ;
			do
			{
				*Dest = *Src ;
				Dest ++ ;
				Src ++ ;
			}while( -- i != 0 ) ;

			Dest += DestAddPitch ;
			Src += SrcAddPitch ;

		}while( -- DestHeight != 0 ) ;
#else
		__asm{
			CLD
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		EDX, DestWidth
			MOV		EBX, DestHeight
			MOV		EAX, SrcAddPitch
			MOV		ECX, DestAddPitch
			PUSH	EBP
			MOV		EBP, ECX
		LOOP1_b:
			MOV		ECX, EDX
			REP		MOVSB
			ADD		EDI, EBP
			ADD		ESI, EAX
			DEC		EBX
			JNZ		LOOP1_b
			POP		EBP
		}
#endif
	}

	// 終了
	return 0 ;
}

// マスクスクリーンを削除する
extern int NS_DeleteMaskScreen( void )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

//	if( WinData.ActiveFlag == FALSE )
//		DxActiveWait() ;

	// もしマスク使用フラグが立っていたらマスクを無効にする作業を行う
	if( MASKD.MaskUseFlag )
		NS_SetUseMaskScreenFlag( FALSE ) ;

	// マスク用サーフェスを削除する
	ReleaseMaskSurface() ;

	// マスクサーフェスが作成されているかフラグを倒す
	MASKD.CreateMaskFlag = FALSE ;
	MASKD.MaskValidFlag = MASKD.MaskUseFlag && MASKD.CreateMaskFlag ;

	// 終了
	return 0 ;
}














// DirectX バージョン依存を含む関数

// マスクスクリーンを作成する関数
extern int CreateMaskScreenFunction( int MaskSurfaceFlag, int Width, int Height )
{
	bool OldEnable = false ;
	BYTE *MaskBufferOld ;
	int MaskBufferPitchOld ;
	int MaskBufferSizeXOld ;
	int MaskBufferSizeYOld ;
	D_IDirect3DTexture9 *MaskImageTextureOld ;
	D_IDirect3DTexture9 *MaskScreenTextureOld ;
	D_IDirect3DSurface9 *MaskScreenSurfaceOld ;
	MEMIMG MaskDrawMemImgOld ;

	if( MaskSurfaceFlag == FALSE ) return 0 ;

	// サイズの調整
	if( MASKD.MaskBuffer != NULL )
	{
		// サイズは大きいほうに合わせる
		if( Width  < MASKD.MaskBufferSizeX ) Width  = MASKD.MaskBufferSizeX ;
		if( Height < MASKD.MaskBufferSizeY ) Height = MASKD.MaskBufferSizeY ;
	}

	// 既にマスクバッファが存在して、且つ今までよりも大きなサイズのバッファが指定された場合はバッファを大きく作り直す
	if( MASKD.MaskBuffer != NULL && ( Width > MASKD.MaskBufferSizeX || Height > MASKD.MaskBufferSizeY ) )
	{
		OldEnable = true ;

		MaskBufferOld = MASKD.MaskBuffer ;
		MASKD.MaskBuffer = NULL ;
		MaskBufferPitchOld = MASKD.MaskBufferPitch ;

		MaskBufferSizeXOld = MASKD.MaskBufferSizeX ;
		MaskBufferSizeYOld = MASKD.MaskBufferSizeY ;

		MaskImageTextureOld = MASKD.MaskImageTexture ;
		MASKD.MaskImageTexture = NULL ;

		MaskScreenTextureOld = MASKD.MaskScreenTexture ;
		MASKD.MaskScreenTexture = NULL ;

		MaskScreenSurfaceOld = MASKD.MaskScreenSurface ;
		MASKD.MaskScreenSurface = NULL ;

		MaskDrawMemImgOld = MASKD.MaskDrawMemImg ;
		_MEMSET( &MASKD.MaskDrawMemImg, 0, sizeof( MEMIMG ) ) ;
	}

	MASKD.MaskBufferSizeX = Width ;
	MASKD.MaskBufferSizeY = Height ;

	// マスク保存用バッファの作成
	if( MASKD.MaskBuffer == NULL )
	{
		// マスクバッファのピッチを算出( 4の倍数にする )
		MASKD.MaskBufferPitch = ( Width + 3 ) / 4 * 4 ;

		// メモリの確保
		MASKD.MaskBuffer = ( BYTE * )DXALLOC( MASKD.MaskBufferPitch * Height ) ;

		// マスクのクリア
		_MEMSET( MASKD.MaskBuffer, 0, MASKD.MaskBufferPitch * Height ) ;
	}

	int w, h ;

	// ハードウエアの機能を使用する場合はテクスチャも作成する
	if( GRA2.ValidHardWare == TRUE )
	{
		// カラーバッファかアルファバッファが作成不可能な場合はえエラー
		if( GRH.MaskColorFormat == D_D3DFMT_UNKNOWN )
		{
			return DxLib_Error( DXSTRING( _T( "マスク描画用カラーバッファで使用できるテクスチャフォーマットがありませんでした" ) ) ) ;
		}

		if( GRH.MaskAlphaFormat == D_D3DFMT_UNKNOWN )
		{
			return DxLib_Error( DXSTRING( _T( "マスク描画用アルファチャンネルバッファで使用できるテクスチャフォーマットがありませんでした" ) ) ) ;
		}

		// 画面サイズが収まる 2 のn乗の値を割り出す
		for( w = 1 ; w < Width  ; w <<= 1 ){}
		for( h = 1 ; h < Height ; h <<= 1 ){}
		MASKD.MaskTextureSizeX = w ;
		MASKD.MaskTextureSizeY = h ;

		// マスク用イメージテクスチャの作成
		if( MASKD.MaskImageTexture == NULL )
		{
			if( GraphicsDevice_CreateTexture( w, h, 1, D_D3DUSAGE_DYNAMIC, GRH.MaskAlphaFormat, D_D3DPOOL_DEFAULT, &MASKD.MaskImageTexture, NULL ) != D_D3D_OK )
				return DxLib_Error( DXSTRING( _T( "マスク用イメージテクスチャの作成に失敗しました" ) ) ) ;

			// マスク用イメージテクスチャの初期化
			{
				D_D3DLOCKED_RECT LockRect ;
				int i, Width ;
				BYTE *Dest ;

				if( MASKD.MaskImageTexture->LockRect( 0, &LockRect, NULL, 0 ) == D_D3D_OK )
				{
					Dest = ( BYTE * )LockRect.pBits ;
					Width = GetD3DFormatColorData( GRH.MaskAlphaFormat )->PixelByte * GRA2.MainScreenSizeX ;
					for( i = 0 ; i < GRA2.MainScreenSizeY ; i ++, Dest += LockRect.Pitch )
						_MEMSET( Dest, 0, Width ) ;

					MASKD.MaskImageTexture->UnlockRect( 0 ) ;
				}
			}
		}

		// マスク用スクリーンテクスチャの作成
		if( MASKD.MaskScreenTexture == NULL )
		{
			if( GraphicsDevice_CreateTexture( w, h, 1, D_D3DUSAGE_RENDERTARGET, GRH.ScreenFormat, D_D3DPOOL_DEFAULT, &MASKD.MaskScreenTexture, NULL ) != D_D3D_OK )
				return DxLib_Error( DXSTRING( _T( "マスク用スクリーンテクスチャの作成に失敗しました" ) ) ) ;
		}

		// マスク用スクリーンサーフェスの取得
		if( MASKD.MaskScreenTexture && MASKD.MaskScreenSurface == NULL )
		{
			MASKD.MaskScreenTexture->GetSurfaceLevel( 0, &MASKD.MaskScreenSurface ) ;
		}
	}
	else
	{
		// 描画先用サーフェスの作成
		if( CheckValidMemImg( &MASKD.MaskDrawMemImg ) == FALSE )
		{
			// MEMIMG を使用する場合は画面と同じ大きさの MEMIMG を作成する
			if( MakeMemImgScreen( &MASKD.MaskDrawMemImg, Width, Height, -1 ) < 0 )
				return DxLib_Error( DXSTRING( _T( "マスク用 MEMIMG の作成に失敗しました" ) ) ) ;

			MASKD.ValidMaskDrawMemImg = TRUE ;
		}
	}

	// 作り直した場合は以前の内容をコピーする
	if( OldEnable )
	{
		int i ;

		// マスクバッファの内容をコピー
		for( i = 0 ; i < MaskBufferSizeYOld ; i ++ )
			_MEMCPY( MASKD.MaskBuffer + MASKD.MaskBufferPitch * i, MaskBufferOld + MaskBufferPitchOld * i, MaskBufferSizeXOld ) ;
		DXFREE( MaskBufferOld ) ;
		MaskBufferOld = NULL ;

		// ハードウエアの機能を使用する場合はテクスチャも作成する
		if( GRA2.ValidHardWare == TRUE )
		{
			RECT UpdateRect ;

			// マスク用イメージテクスチャに内容を転送
			UpdateRect.left = 0 ;
			UpdateRect.top = 0 ;
			UpdateRect.right = MaskBufferSizeXOld ;
			UpdateRect.bottom = MaskBufferSizeYOld ;
			UpdateMaskImageTexture( UpdateRect ) ;

			MaskImageTextureOld->Release() ;
			MaskImageTextureOld = NULL ;

			// マスク用スクリーンテクスチャに今までの内容を転送
			GraphicsDevice_StretchRect(
				MaskScreenSurfaceOld,    &UpdateRect,
				MASKD.MaskScreenSurface, &UpdateRect, D_D3DTEXF_NONE ) ;

			// レンダーターゲットにされていたら変更する
			for( i = 0 ; i < DX_RENDERTARGET_COUNT ; i ++ )
			{
				if( GRH.TargetSurface[ i ] == MaskScreenSurfaceOld )
				{
					SetRenderTargetHardware( MASKD.MaskScreenSurface, i ) ;
				}
			}

			MaskScreenSurfaceOld->Release() ;
			MaskScreenSurfaceOld = NULL ;

			MaskScreenTextureOld->Release() ;
			MaskScreenTextureOld = NULL ;
		}
		else
		{
			// 古い描画先用MEMIMG の内容を新しい MEMIMG にコピーする
			DrawMemImg( &MASKD.MaskDrawMemImg, &MaskDrawMemImgOld, 0, 0, FALSE, NULL ) ;
			TerminateMemImg( &MaskDrawMemImgOld ) ;
		}
	}

	// 終了
	return 0 ;
}

// マスクスクリーンを一時削除する
extern int ReleaseMaskSurface( void )
{
	// マスク保存用バッファの解放
	if( MASKD.MaskBuffer != NULL )
	{
		DXFREE( MASKD.MaskBuffer ) ;
		MASKD.MaskBuffer = NULL ;
	}

	// マスク描画用の MEMIMG を解放
	if( MASKD.ValidMaskDrawMemImg == TRUE )
	{
		TerminateMemImg( &MASKD.MaskDrawMemImg ) ;
		MASKD.ValidMaskDrawMemImg = FALSE ;
	}

	// マスクイメージ用テクスチャの解放
	if( MASKD.MaskImageTexture )
	{
		MASKD.MaskImageTexture->Release() ;
		MASKD.MaskImageTexture = NULL ;
	}

	// マスクスクリーンサーフェスの解放
	if( MASKD.MaskScreenSurface )
	{
		MASKD.MaskScreenSurface->Release() ;
		MASKD.MaskScreenSurface = NULL ;
	}

	// マスクスクリーンテクスチャの解放
	if( MASKD.MaskScreenTexture )
	{
		MASKD.MaskScreenTexture->Release() ;
		MASKD.MaskScreenTexture = NULL ;
	}

	// 終了
	return 0 ;
}

// マスク使用モードを変更
extern int NS_SetUseMaskScreenFlag( int ValidFlag )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// マスクスクリーンがない場合はここで処理を終了
	if( MASKD.MaskBuffer == NULL ) return -1 ;

	// フルスクリーンマスク処理を行っている場合はマスク描画の結果を反映させる
	MASK_FULLSCREEN_MASKUPDATE

	// 今までとフラグが同じ場合は無視
//	if( ValidFlag == MASKD.MaskUseFlag ) return 0 ;

	// マスクを使用するかのフラグをセット
	MASKD.MaskUseFlag = ValidFlag ;
	MASKD.MaskValidFlag = MASKD.MaskUseFlag && MASKD.CreateMaskFlag ;

	IMAGEDATA2 *Image2 ;

	// 描画先の画像データアドレスを取得する
	Image2 = GetGraphData2( GBASE.TargetScreen[ 0 ] ) ;

	// ハードウエアアクセラレータが有効かどうかで処理を分岐
	if( GRA2.ValidHardWare )
	{
		// ハードウエアアクセラレータが有効な場合

		// 溜まった頂点データを描画する
		RenderVertexHardware() ;

		// 描画先を変更する

		// マスクサーフェスが存在していて且つ有効な場合はマスクサーフェスを描画対称にする
		if( MASKD.MaskValidFlag && MASKD.MaskScreenSurface )
		{
			SetRenderTargetHardware( MASKD.MaskScreenSurface ) ;
		}
		else
		// 描画可能画像が有効な場合は描画可能画像を描画対象にする
		if( Image2 )
		{
			if( Image2->Hard.Draw[ 0 ].Tex->RenderTargetSurface )
			{
				SetRenderTargetHardware( Image2->Hard.Draw[ 0 ].Tex->RenderTargetSurface ) ;
			}
			else
			{
				SetRenderTargetHardware( Image2->Hard.Draw[ 0 ].Tex->Surface[ GBASE.TargetScreenSurface[ 0 ] ] ) ;
			}
		}
		else
		{
			// それ以外の場合はサブバックバッファが有効な場合はサブバックバッファを描画対称にする
			SetRenderTargetHardware( GRH.SubBackBufferSurface ? GRH.SubBackBufferSurface : GRH.BackBufferSurface ) ;
		}

		// 使用するＺバッファのセットアップ
		SetupUseZBuffer() ;

		// ビューポートを元に戻す
		GRH.InitializeFlag = TRUE ;
		SetViewportHardwareEasy( GBASE.DrawArea.left, GBASE.DrawArea.top, GBASE.DrawArea.right, GBASE.DrawArea.bottom ) ;
		GRH.InitializeFlag = FALSE ;
	}
	else
	{
		// ソフトウエアレンダリングの場合

		// 描画先の決定
		if( MASKD.MaskValidFlag )
		{
			// マスクが有効な場合はマスクを描画先にする
			GRS.TargetMemImg = &MASKD.MaskDrawMemImg ;
		}
		else
		{
			// それ以外の場合は描画可能画像かメインバッファ
			GRS.TargetMemImg = Image2 ? &Image2->Soft.MemImg : &GRS.MainBufferMemImg ;
		}
	}

	// 終了
	return 0 ;
}

static 	DX_DIRECT3DSURFACE9 *UseBackupSurface, *DestTargetSurface ;

// マスクを使用した描画の前に呼ぶ関数
// ( ついでにサブバッファを使用した描画エリア機能を使用している場合の処理もいれてしまっているよ )
extern int MaskDrawBeginFunction( RECT Rect )
{
	// フルスクリーンマスク処理を行っている場合はマスク描画の結果を反映させる
	MASK_FULLSCREEN_MASKUPDATE

	MASKD.MaskBeginFunctionCount ++ ;
	if( MASKD.MaskBeginFunctionCount == 1 )
	{
		IMAGEDATA2 *Image2 ;

		// 描画先の画像データアドレスを取得する
		Image2 = GetGraphData2( GBASE.TargetScreen[ 0 ] ) ;

		// 描画エリアでクリッピング
		RectClipping( &Rect, &GBASE.DrawArea ) ;

		// ハードウエアを使用するかどうかで処理を分岐
		if( GRA2.ValidHardWare )
		{
			// ハードウエアを使用する場合

			// マスクを使用しているときのみ特別な処理をする
			if( MASKD.MaskValidFlag && MASKD.MaskScreenSurface )
			{
				// マスクを使用している場合は描画先をマスクスクリーンにする
				UseBackupSurface = MASKD.MaskScreenSurface ;

				// 最終出力先の決定
				
				// 描画可能画像が描画先の場合はそれを、
				// それ以外の場合はサブバックバッファを
				// 使用している場合はサブバックバッファを、
				// そうではない場合はバックバッファを出力先にする
				if( Image2 )
				{
					DestTargetSurface = Image2->Orig->Hard.Tex[ 0 ].Surface[ GBASE.TargetScreenSurface[ 0 ] ] ;
				}
				else
				{
					DestTargetSurface = GRH.SubBackBufferSurface ? GRH.SubBackBufferSurface : GRH.BackBufferSurface ;
				}

				// 最終出力先からマスクスクリーンに現在の描画状況をコピーする
				GraphicsDevice_StretchRect(
					DestTargetSurface,       &Rect,
					MASKD.MaskScreenSurface, &Rect, D_D3DTEXF_NONE ) ; 
			}
		}
		else
		{
			// ハードウエアを使用しない場合

			// 描画対象 MEMIMG からマスク処理用 MEMIMG にイメージを転送
			if( MASKD.MaskValidFlag )
			{
				BltMemImg(
					&MASKD.MaskDrawMemImg, 
					Image2 ? &Image2->Soft.MemImg : &GRS.MainBufferMemImg,
					&Rect,
					Rect.left,
					Rect.top ) ;
			}
		}
	}

	// 終了
	return 0 ;
}

// マスクを使用した描画の後に呼ぶ関数
// ( ついでにサブバッファを使用した描画エリア機能を使用している場合の処理もいれてしまっているよ )
extern int MaskDrawAfterFunction( RECT Rect )
{
	MASKD.MaskBeginFunctionCount -- ;
	if( MASKD.MaskBeginFunctionCount == 0 )
	{
		IMAGEDATA2 *Image2 ;

		// 描画先の画像データアドレスを取得する
		Image2 = GetGraphData2( GBASE.TargetScreen[ 0 ] ) ;

		// 描画エリアでクリッピング
		RectClipping( &Rect, &GBASE.DrawArea ) ;
		if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;

		// Direct3D を使用しているかどうかで処理を分岐
		// ハードウエアを使用するかどうかで処理を分岐
		if( GRA2.ValidHardWare )
		{
			// ハードウエアを使用する場合

			// マスクを使用している場合としない場合で処理を分岐
			if( MASKD.MaskValidFlag && MASKD.MaskScreenSurface )
			{
#if 1
				int UseZBufferFlag ;
				VERTEX_2D Vert[ 4 ] ;
				DIRECT3DBLENDINFO BlendInfo ;

				// 描画先を変更
				SetRenderTargetHardware( DestTargetSurface ) ;
				BeginScene() ;

				// Ｚバッファを使用しない設定にする
				UseZBufferFlag = GRH.EnableZBufferFlag ;
				D_SetUseZBufferFlag( FALSE ) ;

				// シェーダーがセットされていたらはずす
				if( GRH.SetVS )
				{
					GraphicsDevice_SetVertexShader( NULL ) ;
					GRH.SetVS = NULL ;
					GRH.SetVD = NULL ;
					GRH.SetFVF = 0 ;
				}
				if( GRH.SetPS )
				{
					GraphicsDevice_SetPixelShader( NULL ) ;
					GRH.SetPS = NULL ;
				}
				GRH.SetIB = NULL ;
				GRH.SetVB = NULL ;

				// ビューポートを元に戻す
				GRH.InitializeFlag = TRUE ;
				SetViewportHardwareEasy( GBASE.DrawArea.left, GBASE.DrawArea.top, GBASE.DrawArea.right, GBASE.DrawArea.bottom ) ;
				GRH.InitializeFlag = FALSE ;

				// マスクアルファチャンネルとマスクカラーバッファを融合して描画
				BlendInfo.AlphaTestEnable = TRUE ;
				BlendInfo.AlphaRef = 0 ;
		//		BlendInfo.AlphaFunc = MASKD.MaskReverseEffectFlag ? D_D3DCMP_GREATER : D_D3DCMP_LESS ;
				BlendInfo.AlphaFunc = MASKD.MaskReverseEffectFlag ? D_D3DCMP_NOTEQUAL : D_D3DCMP_EQUAL ;
				BlendInfo.AlphaBlendEnable = FALSE ;
				BlendInfo.FactorColor = 0xffffffff ;
				BlendInfo.SrcBlend = -1 ;
				BlendInfo.DestBlend = -1 ;
				BlendInfo.UseTextureStageNum = 3 ;

				BlendInfo.SeparateAlphaBlendEnable = FALSE ;
				BlendInfo.SrcBlendAlpha = -1 ;
				BlendInfo.DestBlendAlpha = -1 ;
				BlendInfo.BlendOpAlpha = -1 ;

				BlendInfo.TextureStageInfo[ 0 ].ResultTempARG = -1 ;
				BlendInfo.TextureStageInfo[ 0 ].Texture = ( void * )MASKD.MaskScreenTexture ;
				BlendInfo.TextureStageInfo[ 0 ].TextureCoordIndex = 0 ;
				BlendInfo.TextureStageInfo[ 0 ].AlphaARG1 = D_D3DTA_CURRENT ;
				BlendInfo.TextureStageInfo[ 0 ].AlphaARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 0 ].AlphaOP = D_D3DTOP_SELECTARG1 ;
				BlendInfo.TextureStageInfo[ 0 ].ColorARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 0 ].ColorARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 0 ].ColorOP = D_D3DTOP_SELECTARG1 ;

				BlendInfo.TextureStageInfo[ 1 ].ResultTempARG = -1 ;
				BlendInfo.TextureStageInfo[ 1 ].Texture = ( void * )MASKD.MaskImageTexture ;
				BlendInfo.TextureStageInfo[ 1 ].TextureCoordIndex = 0 ;
				BlendInfo.TextureStageInfo[ 1 ].AlphaARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 1 ].AlphaARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 1 ].AlphaOP = D_D3DTOP_SELECTARG1 ;
				BlendInfo.TextureStageInfo[ 1 ].ColorARG1 = D_D3DTA_CURRENT ;
				BlendInfo.TextureStageInfo[ 1 ].ColorARG2 = -1 ;
				BlendInfo.TextureStageInfo[ 1 ].ColorOP = D_D3DTOP_SELECTARG1 ;

				BlendInfo.TextureStageInfo[ 2 ].ResultTempARG = -1 ;
				BlendInfo.TextureStageInfo[ 2 ].Texture   = NULL ;
				BlendInfo.TextureStageInfo[ 2 ].TextureCoordIndex = 0 ;
				BlendInfo.TextureStageInfo[ 2 ].ColorARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 2 ].ColorARG2 = D_D3DTA_DIFFUSE ;
				BlendInfo.TextureStageInfo[ 2 ].ColorOP   = D_D3DTOP_DISABLE ;
				BlendInfo.TextureStageInfo[ 2 ].AlphaARG1 = D_D3DTA_TEXTURE ;
				BlendInfo.TextureStageInfo[ 2 ].AlphaARG2 = D_D3DTA_DIFFUSE ;
				BlendInfo.TextureStageInfo[ 2 ].AlphaOP   = D_D3DTOP_DISABLE ;

				// ブレンド情報の変更
				D_SetUserBlendInfo( &BlendInfo, FALSE, FALSE ) ;

				// シェーダーを使用する場合はここでピクセルシェーダーを設定する
				if( GRH.UseShader && GRH.NormalDraw_NotUseShader == FALSE )
				{
					GRH.SetPS = GRH.MaskEffectPixelShader ;
					GraphicsDevice_SetPixelShader( GRH.SetPS ) ;
					GRH.NormalPS = FALSE ;
					GRH.DrawPrepAlwaysFlag = TRUE ;
				}

				// 描画準備
				BeginScene() ;

				// 頂点の準備
				Vert[ 2 ].pos.x = Vert[ 0 ].pos.x = ( float )Rect.left   - 0.5f ;
				Vert[ 1 ].pos.y = Vert[ 0 ].pos.y = ( float )Rect.top    - 0.5f ;

				Vert[ 3 ].pos.x = Vert[ 1 ].pos.x = ( float )Rect.right  - 0.5f ;
				Vert[ 3 ].pos.y = Vert[ 2 ].pos.y = ( float )Rect.bottom - 0.5f ;

				Vert[ 2 ].u = Vert[ 0 ].u = ( float )Rect.left   / ( float )MASKD.MaskTextureSizeX ;
				Vert[ 1 ].v = Vert[ 0 ].v = ( float )Rect.top    / ( float )MASKD.MaskTextureSizeY ;
				Vert[ 3 ].u = Vert[ 1 ].u = ( float )Rect.right  / ( float )MASKD.MaskTextureSizeX ;
				Vert[ 3 ].v = Vert[ 2 ].v = ( float )Rect.bottom / ( float )MASKD.MaskTextureSizeY ;

				Vert[ 0 ].color =
				Vert[ 1 ].color =
				Vert[ 2 ].color =
				Vert[ 3 ].color = 0xffffffff ;

				Vert[ 0 ].pos.z = 
				Vert[ 1 ].pos.z = 
				Vert[ 2 ].pos.z = 
				Vert[ 3 ].pos.z = 0.0f ;

				Vert[ 0 ].rhw = 
				Vert[ 1 ].rhw = 
				Vert[ 2 ].rhw = 
				Vert[ 3 ].rhw = 1.0f ;

				// 描画
				SETFVF( VERTEXFVF_2D ) ;
				GraphicsDevice_DrawPrimitiveUP( D_D3DPT_TRIANGLESTRIP, 2, Vert, sizeof( VERTEX_2D ) ) ;
				EndScene() ;
				BeginScene() ;

				// ブレンド情報を元に戻す
				SetUserBlendInfo( NULL ) ;

				// シェーダーを使用する場合はここでピクセルシェーダーを無効にする
				if( GRH.UseShader && GRH.NormalDraw_NotUseShader == FALSE )
				{
					GRH.SetPS = NULL ;
					GraphicsDevice_SetPixelShader( GRH.SetPS ) ;
				}

				// Ｚバッファの設定を元に戻す
				D_SetUseZBufferFlag( UseZBufferFlag ) ;

#else
				// 書き出す
				RenderVertexHardware() ;
				EndScene() ;

				GraphicsDevice_StretchRect(
					UseBackupSurface,  &Rect,
					DestTargetSurface, &Rect, D_D3DTEXF_NONE ) ;
#endif

				// 描画先をマスクサーフェスにする
				SetRenderTargetHardware( MASKD.MaskScreenSurface ) ;

				// ビューポートを元に戻す
				GRH.InitializeFlag = TRUE ;
				SetViewportHardwareEasy( GBASE.DrawArea.left, GBASE.DrawArea.top, GBASE.DrawArea.right, GBASE.DrawArea.bottom ) ;
				GRH.InitializeFlag = FALSE ;
			}
		}
		else
		{
			// ハードウエアを使用しない場合

			// マスクが掛かっていない部分を描画先に転送する
			if( MASKD.MaskValidFlag )
			{
				BYTE *Dest, *Src, *Mask ;
				int DestAddPitch, SrcAddPitch, MaskAddPitch ;
				int DestWidth, DestHeight ;
				MEMIMG *TargetMemImg ;
				int PixelByte ;
				
				TargetMemImg = Image2 ? &Image2->Soft.MemImg : &GRS.MainBufferMemImg ;
				PixelByte = TargetMemImg->Base->ColorDataP->PixelByte ;

				DestWidth = Rect.right - Rect.left ;
				DestHeight = Rect.bottom - Rect.top ;

				MaskAddPitch = MASKD.MaskBufferPitch            - DestWidth ;
				SrcAddPitch  = MASKD.MaskDrawMemImg.Base->Pitch - PixelByte * DestWidth ;
				DestAddPitch = TargetMemImg->Base->Pitch        - PixelByte * DestWidth ;

				Mask = ( BYTE * )MASKD.MaskBuffer              + Rect.left             + Rect.top * MASKD.MaskBufferPitch ;
				Src  = ( BYTE * )MASKD.MaskDrawMemImg.UseImage + Rect.left * PixelByte + Rect.top * MASKD.MaskDrawMemImg.Base->Pitch ;
				Dest = ( BYTE * )TargetMemImg->UseImage        + Rect.left * PixelByte + Rect.top * TargetMemImg->Base->Pitch ;

				if( MASKD.MaskReverseEffectFlag == TRUE )
				{
#ifdef DX_NON_INLINE_ASM
					int i ;
					// 効果反転版
					switch( PixelByte )
					{
					case 2 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask != 0 )
									*( ( WORD * )Dest ) = *( ( WORD * )Src ) ;

								Dest += 2 ;
								Src += 2 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;

					case 4 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask != 0 )
									*( ( DWORD * )Dest ) = *( ( DWORD * )Src ) ;

								Dest += 4 ;
								Src += 4 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;
					}
#else
					// 効果反転版
					switch( PixelByte )
					{
					case 2 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_2_1:
							MOV		ECX, DestWidth
	LOOP_2_2:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JZ		LABEL_2_1
							MOV		BX, [ ESI ]
							MOV		[ EDI ], BX
	LABEL_2_1:
							ADD		ESI, 2
							ADD		EDI, 2
							INC		EDX
							DEC		ECX
							JNZ		LOOP_2_2

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_2_1
						}
						break ;

					case 4 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_4_1:
							MOV		ECX, DestWidth
	LOOP_4_2:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JZ		LABEL_4_1
							MOV		EBX, [ ESI ]
							MOV		[ EDI ], EBX
	LABEL_4_1:
							ADD		ESI, 4
							ADD		EDI, 4
							INC		EDX
							DEC		ECX
							JNZ		LOOP_4_2

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_4_1
						}
						break ;
					}
#endif
				}
				else
				{
#ifdef DX_NON_INLINE_ASM
					int i ;
					// 効果反転してない版
					switch( PixelByte )
					{
					case 2 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask == 0 )
									*( ( WORD * )Dest ) = *( ( WORD * )Src ) ;

								Dest += 2 ;
								Src += 2 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;

					case 4 :
						do
						{
							i = DestWidth ;
							do
							{
								if( *Mask == 0 )
									*( ( DWORD * )Dest ) = *( ( DWORD * )Src ) ;

								Dest += 4 ;
								Src += 4 ;
								Mask ++ ;

							}while( -- i != 0 ) ;

							Dest += DestAddPitch ;
							Src += SrcAddPitch ;
							Mask += MaskAddPitch ;

						}while( -- DestHeight != 0 ) ;
						break ;
					}
#else
					// 効果反転してない版
					switch( PixelByte )
					{
					case 2 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_2_1R:
							MOV		ECX, DestWidth
	LOOP_2_2R:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JNZ		LABEL_2_1R
							MOV		BX, [ ESI ]
							MOV		[ EDI ], BX
	LABEL_2_1R:
							ADD		ESI, 2
							ADD		EDI, 2
							INC		EDX
							DEC		ECX
							JNZ		LOOP_2_2R

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_2_1R
						}
						break ;

					case 4 :
						__asm
						{
							MOV		EDI, Dest
							MOV		ESI, Src
							MOV		EDX, Mask
	LOOP_4_1R:
							MOV		ECX, DestWidth
	LOOP_4_2R:
							MOV		AL, [ EDX ]
							TEST	AL, 0xff
							JNZ		LABEL_4_1R
							MOV		EBX, [ ESI ]
							MOV		[ EDI ], EBX
	LABEL_4_1R:
							ADD		ESI, 4
							ADD		EDI, 4
							INC		EDX
							DEC		ECX
							JNZ		LOOP_4_2R

							ADD		ESI, SrcAddPitch
							ADD		EDI, DestAddPitch
							ADD		EDX, MaskAddPitch

							DEC		DestHeight
							JNZ		LOOP_4_1R
						}
						break ;
					}
#endif
				}
			}
		}
	}

	// 終了
	return 0 ;
}

// マスクをセットする
extern int NS_DrawMask( int x, int y, int MaskHandle, int TransMode )
{
	MASKDATA * Mask ; 
	RECT Rect, MovRect ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	if( MASKD.MaskBuffer == NULL )
		return 0 ;

	// クリッピング処理
	SETRECT( Rect, x, y, x + Mask->MaskWidth, y + Mask->MaskHeight ) ;
	RectClipping( &Rect, &GBASE.DrawArea ) ;
	MovRect = Rect ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;
	Rect.left -= x ;
	Rect.right -= x ;
	Rect.top -= y ;
	Rect.bottom -= y ;
	if( Rect.left > 0 ){ x += Rect.left ; }
	if( Rect.top  > 0 ){ y += Rect.top  ; }

	// 転送処理
	{
		BYTE *Dest, *Src ;
		int DestAddPitch, SrcAddPitch ;
		int DestWidth, DestHeight, DWordNum, BNum ;
#ifdef DX_NON_INLINE_ASM
		int i ;
#endif

		DestWidth = Rect.right - Rect.left ;
		DestHeight = Rect.bottom - Rect.top ;
		DWordNum = DestWidth / 4 ;
		BNum = DestWidth % 4 ;
		Src = Mask->SrcData + Rect.left + ( Rect.top * Mask->SrcDataPitch ) ;
		Dest = MASKD.MaskBuffer + x + ( y * MASKD.MaskBufferPitch ) ;

		SrcAddPitch = Mask->SrcDataPitch - DestWidth ;
		DestAddPitch = MASKD.MaskBufferPitch - DestWidth ;


		switch( TransMode )
		{
		case DX_MASKTRANS_NONE :
			// 透過色なし転送
#ifdef DX_NON_INLINE_ASM
			do
			{
				i = DestWidth ;
				do
				{
					*Dest = *Src ;
					Dest ++ ;
					Src ++ ;
				}while( -- i != 0 ) ;
				Dest += DestAddPitch ;
				Src += SrcAddPitch ;
			}while( -- DestHeight != 0 ) ;
#else
			__asm{
				CLD
				MOV		EDI, Dest
				MOV		ESI, Src
				MOV		EDX, DestWidth
				MOV		EBX, DestHeight
				MOV		EAX, SrcAddPitch
				MOV		ECX, DestAddPitch
				PUSH	EBP
				MOV		EBP, ECX
			LOOP1_a:
				MOV		ECX, EDX
				REP		MOVSB
				ADD		EDI, EBP
				ADD		ESI, EAX
				DEC		EBX
				JNZ		LOOP1_a
				POP		EBP
			}
#endif
			break ;

		case DX_MASKTRANS_BLACK :
			// 黒透過色
#ifdef DX_NON_INLINE_ASM
			do
			{
				if( DWordNum != 0 )
				{
					i = DWordNum ;
					do
					{
						*( ( DWORD * )Dest ) |= *( ( DWORD * )Src ) ;
						Dest += 4 ;
						Src += 4 ;
					}while( -- i != 0 ) ;
				}

				if( BNum != 0 )
				{
					i = BNum ;
					do
					{
						*Dest |= *Src ;
						Dest ++ ;
						Src ++ ;
					}while( -- i != 0 ) ;
				}

				Dest += DestAddPitch ;
				Src += SrcAddPitch ;
			}while( -- DestHeight != 0 ) ;
#else
			__asm{
				MOV		EDI, Dest
				MOV		ESI, Src
				MOV		EDX, BNum
				SHL		EDX, 30
				OR		EDX, DWordNum
				MOV		EBX, DestHeight

			LOOP1B:
				MOV		ECX, EDX
				AND		ECX, 0x3fffffff // ((1 << 30) - 1)
				CMP		ECX, 0
				JE		R10B

			LOOP2B:
				MOV		EAX, [ESI]
				OR		[EDI], EAX

				ADD		EDI, 4
				ADD		ESI, 4

				DEC		ECX
				JNZ		LOOP2B

			R10B:

				MOV		ECX, EDX
				SHR		ECX, 30
				CMP		ECX, 0
				JE		R11B

			LOOP3B:
				MOV		AL, [ESI]
				OR		[EDI], AL
				INC		ESI
				INC		EDI
				DEC		ECX
				JNZ		LOOP3B

			R11B:

				ADD		EDI, DestAddPitch
				ADD		ESI, SrcAddPitch
				DEC		EBX
				JNZ		LOOP1B
			}
#endif
			break ;

		case DX_MASKTRANS_WHITE :
			// 白透過色
#ifdef DX_NON_INLINE_ASM
			do
			{
				if( DWordNum != 0 )
				{
					i = DWordNum ;
					do
					{
						*( ( DWORD * )Dest ) &= *( ( DWORD * )Src ) ;
						Dest += 4 ;
						Src += 4 ;
					}while( -- i != 0 ) ;
				}

				if( BNum != 0 )
				{
					i = BNum ;
					do
					{
						*Dest &= *Src ;
						Dest ++ ;
						Src ++ ;
					}while( -- i != 0 ) ;
				}

				Dest += DestAddPitch ;
				Src += SrcAddPitch ;
			}while( -- DestHeight != 0 ) ;
#else
			__asm
			{
				MOV		EDI, Dest
				MOV		ESI, Src
				MOV		EDX, BNum
				SHL		EDX, 30
				OR		EDX, DWordNum
				MOV		EBX, DestHeight

			LOOP1W:
				MOV		ECX, EDX
				AND		ECX, 0x3fffffff // ((1 << 30) - 1)
				CMP		ECX, 0
				JE		R10W

			LOOP2W:
				MOV		EAX, [ESI]
				AND		[EDI], EAX
				ADD		EDI, 4
				ADD		ESI, 4

				DEC		ECX
				JNZ		LOOP2W

			R10W:
				MOV		ECX, EDX
				SHR		ECX, 30
				CMP		ECX, 0
				JE		R11W

			LOOP3W:
				MOV		AL, [ESI]
				AND		[EDI], AL
				INC		ESI
				INC		EDI
				DEC		ECX
				JNZ		LOOP3W

			R11W:
				ADD		EDI, DestAddPitch
				ADD		ESI, SrcAddPitch
				DEC		EBX
				JNZ		LOOP1W
			}
#endif
			break ;
		}
	}

	// 更新
	if( GRA2.ValidHardWare )
	{
		UpdateMaskImageTexture( MovRect ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマスクを画面いっぱいに展開する
extern int NS_DrawFillMask( int x1, int y1, int x2, int y2, int MaskHandle )
{
	MASKDATA * Mask ;
	RECT Rect, MovRect ;

	if( MASKD.InitializeFlag == FALSE )
		return -1 ;


	if( MASKHCHK( MaskHandle, Mask ) )
		return -1 ;

	if( MASKD.MaskBuffer == NULL )
		return 0 ;

	if( x1 == x2 || y1 == y2 ) return 0 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// クリッピング処理
	SETRECT( Rect, x1, y1, x2, y2 ) ;
	RectClipping( &Rect, &GBASE.DrawArea ) ;
	MovRect = Rect ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;
	Rect.left -= x1 ;
	Rect.right -= x1 ;
	Rect.top -= y1 ;
	Rect.bottom -= y1 ;
	if( Rect.left > 0 ){ x1 += Rect.left ; }
	if( Rect.top  > 0 ){ y1 += Rect.top  ; }

	{
		BYTE *Dest, *Src, *Src2, *Src3 ;
		int DestWidth, DestHeight, SrcWidth, SrcHeight ;
		DWORD SrcAddPitch, DestAddPitch ;
		DWORD FirstX, FirstY ;
		DWORD CounterY ;

		SrcWidth = Mask->MaskWidth ;
		SrcHeight = Mask->MaskHeight ;

		DestWidth = Rect.right - Rect.left ;
		DestHeight = Rect.bottom - Rect.top ;

		FirstX = Mask->MaskWidth - Rect.left % Mask->MaskWidth ;
		FirstY = Mask->MaskHeight - Rect.top % Mask->MaskHeight ;

		Dest = MASKD.MaskBuffer + x1 + y1 * MASKD.MaskBufferPitch ;
		Src = Mask->SrcData + ( Rect.left % Mask->MaskWidth ) + ( Rect.top % Mask->MaskHeight ) * Mask->SrcDataPitch ;
		Src2 = Mask->SrcData ;
		Src3 = Mask->SrcData + ( Rect.left % Mask->MaskWidth ) ;

		SrcAddPitch = Mask->SrcDataPitch - ( ( DestWidth + Rect.left % Mask->MaskWidth ) % Mask->MaskWidth == 0 ? 0 : ( DestWidth + Rect.left % Mask->MaskWidth ) % Mask->MaskWidth ) + Rect.left % Mask->MaskWidth ;
		DestAddPitch = MASKD.MaskBufferPitch - DestWidth ;

		CounterY = FirstY ;

#ifdef DX_NON_INLINE_ASM
		int i, TempWidth, TempHeight ;

		TempHeight = FirstY ;
		do
		{
			TempWidth = FirstX ;
			i = DestWidth ;
			do
			{
				*Dest = *Src ;
				Dest ++ ;
				Src ++ ;
				if( -- TempWidth == 0 )
				{
					TempWidth = SrcWidth ;
					Src -= SrcWidth ;
				}
			}while( -- i != 0 ) ;

			Dest += DestAddPitch ;
			Src += SrcAddPitch ;

			if( -- TempHeight == 0 )
			{
				Src = Src3 ;
				TempHeight = SrcHeight ;
			}

		}while( -- DestHeight != 0 ) ;
#else
		__asm{
			CLD
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		EAX, SrcWidth
			MOV		EBX, FirstY
		LOOP1:
			MOV		ECX, FirstX
			MOV		EDX, DestWidth
		LOOP2:
			MOVSB
			DEC		ECX
			JZ		R2
		R1:
			DEC		EDX
			JZ		RE
			JMP		LOOP2

		R2:
			MOV		ECX, EAX
			SUB		ESI, ECX
			JMP		R1

		RE:
			ADD		ESI, SrcAddPitch
			ADD		EDI, DestAddPitch
			DEC		DestHeight
			JZ		RE2
			DEC		EBX
			JZ		R3
			JMP		LOOP1

		R3:
			MOV		ESI, Src3
			MOV		ECX, SrcHeight
			MOV		EBX, ECX
			JMP		LOOP1

		RE2:
		}
#endif
	}

	if( GRA2.ValidHardWare )
	{
		UpdateMaskImageTexture( MovRect ) ;
	}

	// 終了
	return 0 ;
}

// マスクのセット
extern int NS_DrawMaskToDirectData( int x, int y, int Width, int Height, const void *MaskData, int TransMode )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// マスクスクリーンがない場合はここで処理を終了
	if( MASKD.MaskBuffer == NULL ) return 0 ;

	// マスクデータがなかった場合は処理を終了
	if( MaskData == NULL ) return 0 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// マスクスクリーンにマスクを展開
	DrawMaskToDirectData_Base( x, y, MASKD.MaskBuffer, MASKD.MaskBufferPitch,
							Width, Height, MaskData, TransMode ) ;

	RECT Rect ;

	if( GRA2.ValidHardWare )
	{
		// 更新
		Rect.left = x ;
		Rect.top = y ;
		Rect.right = x + Width ;
		Rect.bottom = y + Height ;
		UpdateMaskImageTexture( Rect ) ;
	}

	// 終了
	return 0 ;
}

// マスクスクリーンを指定の色で塗りつぶす
extern int NS_FillMaskScreen( int Flag )
{
	if( MASKD.InitializeFlag == FALSE )
		return -1 ;

	// マスクスクリーンがない場合はここで処理を終了
	if( MASKD.MaskBuffer == NULL ) return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// マスクスクリーンをクリアする
	_MEMSET( MASKD.MaskBuffer, Flag ? 0xff : 0, MASKD.MaskBufferPitch * GBASE.DrawSizeY ) ;

	if( GRA2.ValidHardWare )
	{
		D_D3DLOCKED_RECT LockRect ;
		int i, Width = 0 ;
		BYTE *Dest ;

		if( MASKD.MaskImageTexture->LockRect( 0, &LockRect, NULL, 0 ) == D_D3D_OK )
		{
			Flag = Flag ? 0xff : 0 ;

			switch( GRH.MaskAlphaFormat )
			{
			case D_D3DFMT_A8R8G8B8 : Width = GRA2.MainScreenSizeX * 4 ; break ;
			case D_D3DFMT_A4R4G4B4 : Width = GRA2.MainScreenSizeX * 2 ; break ;
			case D_D3DFMT_A1R5G5B5 : Width = GRA2.MainScreenSizeX * 2 ; break ;
			}

			Dest = ( BYTE * )LockRect.pBits ;
			for( i = 0 ; i < GRA2.MainScreenSizeY ; i ++, Dest += LockRect.Pitch )
			{
				_MEMSET( Dest, ( unsigned char )Flag, Width ) ;
			}

			MASKD.MaskImageTexture->UnlockRect( 0 ) ;
		}
	}

	// 終了
	return 0 ;
}












// DirectX バージョン依存がある関数

// DirectX9 用

// 指定領域のマスクイメージテクスチャを更新する
static int UpdateMaskImageTexture( RECT Rect )
{
	D_D3DLOCKED_RECT LockRect ;
	int Width, Height ;
	BYTE *Dest, *Src ;
	DWORD DestAdd, SrcAdd ;
#ifdef DX_NON_INLINE_ASM
	int i ;
#endif

	RectClipping( &Rect, &GBASE.DrawArea ) ;
	if( Rect.left == Rect.right || Rect.top == Rect.bottom ) return 0 ;

	Width = Rect.right - Rect.left ;
	Height = Rect.bottom - Rect.top ;

	// マスクイメージテクスチャをロックする
	if( MASKD.MaskImageTexture->LockRect( 0, &LockRect, &Rect, 0 ) != D_D3D_OK )
		return -1 ;

	// 転送元の準備
	Src = MASKD.MaskBuffer + MASKD.MaskBufferPitch * Rect.top + Rect.left ;
	SrcAdd = MASKD.MaskBufferPitch - Width ;

	// 指定部分イメージの転送
	Dest = ( BYTE * )LockRect.pBits ;
	switch( GRH.MaskAlphaFormat )
	{
	case D_D3DFMT_A8R8G8B8 :
		DestAdd = LockRect.Pitch - Width * 4 ;
#ifdef DX_NON_INLINE_ASM
		do
		{
			i = Width ;
			do
			{
				Dest[ 3 ] = *Src ;
				Dest += 4 ;
				Src ++ ;
			}while( -- i != 0 ) ;
			Dest += DestAdd ;
			Src += SrcAdd ;
		}while( -- Height != 0 ) ;
#else
		__asm
		{
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		ECX, Height
LOOP_A8R8G8B8_1:
			MOV		EDX, Width
LOOP_A8R8G8B8_2:
			MOV		AL, [ ESI ]
			MOV		[ EDI + 3 ], AL
			INC		ESI
			ADD		EDI, 4
			DEC		EDX
			JNZ		LOOP_A8R8G8B8_2

			ADD		EDI, DestAdd
			ADD		ESI, SrcAdd
			DEC		Height
			JNZ		LOOP_A8R8G8B8_1
		}
#endif
		break ;

	case D_D3DFMT_A1R5G5B5 :
	case D_D3DFMT_A4R4G4B4 :
		DestAdd = LockRect.Pitch - Width * 2 ;
#ifdef DX_NON_INLINE_ASM
		do
		{
			i = Width ;
			do
			{
				Dest[ 1 ] = *Src ;
				Dest += 2 ;
				Src ++ ;
			}while( -- i != 0 ) ;
			Dest += DestAdd ;
			Src += SrcAdd ;
		}while( -- Height != 0 ) ;
#else
		__asm
		{
			MOV		EDI, Dest
			MOV		ESI, Src
			MOV		ECX, Height
LOOP_A1R5G5B5_1:
			MOV		EDX, Width
LOOP_A1R5G5B5_2:
			MOV		AL, [ ESI ]
			MOV		[ EDI + 1 ], AL
			INC		ESI
			ADD		EDI, 2
			DEC		EDX
			JNZ		LOOP_A1R5G5B5_2

			ADD		EDI, DestAdd
			ADD		ESI, SrcAdd
			DEC		Height
			JNZ		LOOP_A1R5G5B5_1
		}
#endif
		break ;
	}

	// ロックを解除する
	MASKD.MaskImageTexture->UnlockRect( 0 ) ;

	// 終了
	return 0 ;
}

}

#endif


