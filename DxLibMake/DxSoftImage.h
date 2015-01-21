// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ソフトウェアで扱う画像プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXSOFTIMAGE_H__
#define __DXSOFTIMAGE_H__

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"

#ifndef DX_NON_SOFTIMAGE
#include "DxHandle.h"
#include "DxBaseImage.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// ソフトイメージハンドルの有効性チェック
#define SFTIMGCHK( HAND, SPOINT )			HANDLECHK(       DX_HANDLETYPE_SOFTIMAGE, HAND, *( ( HANDLEINFO ** )&SPOINT ) )
#define SFTIMGCHK_ASYNC( HAND, SPOINT )		HANDLECHK_ASYNC( DX_HANDLETYPE_SOFTIMAGE, HAND, *( ( HANDLEINFO ** )&SPOINT ) )

// 结构体定义 --------------------------------------------------------------------

// ソフトイメージの情報構造体
struct SOFTIMAGE
{
	HANDLEINFO				HandleInfo ;			// ハンドル共通データ
	BASEIMAGE				BaseImage ;				// 基本イメージ構造体
} ;

// ソフトウエアで扱う画像の情報構造体
struct SOFTIMAGEMANAGE
{
	int						InitializeFlag ;			// 初期化フラグ
} ;

// 内部大域変数宣言 --------------------------------------------------------------

extern SOFTIMAGEMANAGE SoftImageManage ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// 初期化、後始末
extern	int		InitializeSoftImageManage( void ) ;										// ソフトイメージ管理情報の初期化
extern	int		TerminateSoftImageManage( void ) ;										// ソフトイメージ管理情報の後始末

// 読み込み関連
extern	int		InitializeSoftImageHandle( HANDLEINFO *HandleInfo ) ;															// ソフトウエアイメージハンドルの初期化
extern	int		TerminateSoftImageHandle( HANDLEINFO *HandleInfo ) ;															// ソフトウエアイメージハンドルの後始末
extern	int		LoadSoftImage_UseGParam( const TCHAR *FileName, int ASyncLoadFlag = FALSE ) ;									// LoadSoftImage のグローバル変数にアクセスしないバージョン
extern	int		LoadSoftImageToMem_UseGParam( void *FileImage, int FileImageSize, int ASyncLoadFlag = FALSE ) ;					// LoadSoftImageToMem のグローバル変数にアクセスしないバージョン

}

#endif // DX_NON_SOFTIMAGE

#endif // __DXSOFTIMAGE_H__

