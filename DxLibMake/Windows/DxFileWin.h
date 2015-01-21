// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用ファイル関係プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXFILEWIN_H__
#define __DXFILEWIN_H__

// Include ------------------------------------------------------------------
#include "../DxCompileConfig.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// ファイルアクセス専用スレッド用構造体
struct FILEACCESSTHREAD
{
	DWORD					ThreadID ;							// スレッドＩＤ
	HANDLE					ThreadHandle ;						// スレッドハンドル
	HANDLE					Handle ;							// ファイルアクセス用ハンドル
	HANDLE					FuncEvent ;							// 指令受け取り用ハンドル
	HANDLE					CompEvent ;							// 指令完了用ハンドル
	int						Function ;							// 指令( FILEACCESSTHREAD_FUNCTION_OPEN 等 )

	int						EndFlag ;							// 終了したか、フラグ
	int						ErrorFlag ;							// エラー発生フラグ

	TCHAR					FilePath[MAX_PATH] ;				// ファイルパス
	void					*ReadBuffer ;						// 読み込むデータを格納するバッファー
	ULONGLONG				ReadPosition ;						// 読み込むデータの位置
	DWORD					ReadSize ;							// 読み込むデータのサイズ(読み込めたデータのサイズ)
	ULONGLONG				SeekPoint ;							// ファイルポインタを移動する位置 

	BYTE					*CacheBuffer ;						// キャッシュバッファへのポインタ
	ULONGLONG				CachePosition ;						// キャッシュしているファイル位置
	ULONGLONG				CacheSize ;							// キャッシュバッファの有効データサイズ
} ;

// ファイルアクセス処理用構造体
struct WINFILEACCESS
{
	HANDLE					Handle ;							// ファイルアクセス用ハンドル
	int						UseThread ;							// スレッドを使用するかどうか
	int						UseCacheFlag ;						// キャッシュを使用するかどうか
	int						UseASyncReadFlag ;					// 非同期読み込みを行うかどうか
	int						EofFlag ;							// 終端チェックフラグ
	ULONGLONG				Position ;							// アクセス位置
	ULONGLONG				Size ;								// サイズ

	FILEACCESSTHREAD		ThreadData ;						// 読み込み専用スレッドのデータ
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	HANDLE		CreateTemporaryFile( TCHAR *TempFileNameBuffer ) ;								// 生成临时文件

}

#endif // __DXFILEWIN_H__
