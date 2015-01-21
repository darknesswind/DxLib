// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		文字列入力プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXINPUTSTRING_H__
#define __DXINPUTSTRING_H__

#include "DxCompileConfig.h"

#ifndef DX_NON_INPUTSTRING

// Include ------------------------------------------------------------------
#include "DxLib.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#define CHARBUFFER_SIZE				(1024)				// 文字コードバッファ容量

// 结构体定义 --------------------------------------------------------------------

#ifndef DX_NON_KEYEX

// 文字列入力中データ構造体
struct INPUTDATA
{
	int						UseFlag ;							// 使用中かフラグ

	int						ID ;								// エラーチェックに使用するＩＤ

	int						EndFlag ;							// 入力が終了しているかフラグ
	int						CancellFlag ;						// 入力がキャンセルされたかのフラグ

//	RECT					DrawRect ;							// 描画領域

	int						StrLength ;							// 入力中文字列の長さ
	int						MaxStrLength ;						// 文字列の最大長
	int						SelectStart ;						// 選択範囲開始位置
	int						SelectEnd ;							// 選択範囲終了位置
	int						DrawStartPos ;						// 描画を開始する位置
	int						Point ;								// カーソルの位置
	TCHAR					*Buffer ;							// 実行中に編集する文字列バッファ
//	char					*DestBuffer ;						// 入力終了時に転送する文字列バッファへのポインタ

	int						CancelValidFlag ;					// キャンセル有効フラグ
	int						SingleCharOnlyFlag ;				// ダブルバイト文字は扱わないかフラグ
	int						NumCharOnlyFlag ;					// 半角英数字しか使わないかフラグ
	int						DoubleCharOnlyFlag ;				// ダブルバイト文字しか扱わないかフラグ
} ;

#endif // DX_NON_KEYEX

// 文字コードバッファ構造体
struct CHARBUFFER
{
	TCHAR					CharBuffer[ CHARBUFFER_SIZE + 1 ] ;	// 入力された文字列
	int						StPoint , EdPoint ;					// リングバッファ用ポインタ

	TCHAR					SecondString[CHARBUFFER_SIZE + 1 ] ;
	int						IMEInputFlag ;						// 入力処理中か、フラグ
	int						InputPoint ;						// 入力中の編集文字列上の位置
	TCHAR					InputString[ CHARBUFFER_SIZE + 1 ] ;// ＩＭＥによる入力中の文字列
	TCHAR					InputTempString[ CHARBUFFER_SIZE + 1 ] ;// ＩＭＥによる入力中の文字列のテンポラリバッファ
	PCANDIDATELIST			CandidateList ;						// 変換候補のリストデータ
	int						CandidateListSize ;					// 変換候補のリストデータに必要なデータ容量
	int						IMEUseFlag ;						// ＩＭＥの使用状態
	int						IMEUseFlag_System ;					// ＩＭＥの使用状態( ＤＸライブラリ内部用 )
	int						IMEUseFlag_OSSet ;					// ＯＳに対して設定しているＩＭＥの使用状態
	int						IMESwitch ;							// ＩＭＥの起動状態
	int						IMEInputStringMaxLengthIMESync ;	// ＩＭＥで入力できる最大文字数を MakeKeyInput の最大数に同期させるかどうか( TRUE:同期させる  FALSE:させない )
	int						IMEInputMaxLength ;					// ＩＭＥで一度に入力できる文字数( 0:特に制限なし  1以上:文字数制限 )
	int						IMERefreshStep ;					// ＩＭＥの入力文字列リフレッシュ用ステップ変数
	IMEINPUTDATA			*IMEInputData ;						// ＩＭＥの入力中の情報( ユーザー用 )

	int						IMEProcessNum ;						// ＩＭＥの文字取得プロセス番号
	int						ClauseData[ 1024 ] ;				// 文節データ
	int						ClauseNum ;							// 文節数
	BYTE					CharAttr[ 1024 ] ;					// 各文字の属性情報
	int						CharAttrNum ;						// 各文字の属性情報の要素数
	int						ChangeFlag ;						// ＩＭＥ入力に変化があったかフラグ

	int						StrColor1 ;							// 入力文字列の色
	int						StrColor2 ;							// ＩＭＥ未使用時のカーソルの色
	int						StrColor3 ;							// ＩＭＥ使用時の入力文字列の周りの色
	int						StrColor4 ;							// ＩＭＥ使用時のカーソルの色
	int						StrColor5 ;							// ＩＭＥ使用時の変換文字列の下線
	int						StrColor6 ;							// ＩＭＥ使用時の選択対象の変換候補文字列の色
	int						StrColor7 ;							// ＩＭＥ使用時の入力モード文字列の色(「全角ひらがな」等)
	int						StrColor8 ;							// 入力文字列の縁の色
	int						StrColor9 ;							// ＩＭＥ使用時の選択対象の変換候補文字列の縁の色
	int						StrColor10 ;						// ＩＭＥ使用時の入力モード文字列の縁の色
	int						StrColor11 ;						// ＩＭＥ使用時の変換候補ウインドウの縁の色
	int						StrColor12 ;						// ＩＭＥ使用時の変換候補ウインドウの下地の色
	int						StrColor13 ;						// 選択された入力文字列の背景の色
	int						StrColor14 ;						// 選択された入力文字列の色
	int						StrColor15 ;						// 選択された入力文字列の縁の色
	int						StrColor16 ;						// ＩＭＥ使用時の入力文字列の色

	int						StrColor17Enable ;					// StrColor17 が有効かどうか( TRUE:有効  FALSE:無効 )
	int						StrColor17 ;						// ＩＭＥ使用時の入力文字列の縁の色
	
	int						CBrinkFlag ;						// カーソルを点滅させるか、フラグ
	int						CBrinkCount ;						// カーソルを点滅させる場合のカウンタ
	int						CBrinkWait ;						// カーソルの点滅の速度
	int						CBrinkDrawFlag ;					// カーソルを描画するかどうかのフラグ

	int						ActiveInputHandle ;					// 入力がアクティブになっている入力ハンドル
	int						UseFontHandle ;						// 文字列描画に使用するフォントのハンドル(-1でデフォルトのハンドル)
#ifndef DX_NON_KEYEX
	INPUTDATA				InputData[ MAX_INPUT_NUM ] ;		// インプットデータ
	int						HandleID ;							// ハンドルに割り当てるＩＤ
#endif
} ;

// 入力モード構造体
struct INPUTMODE
{
	DWORD					InputState ;						// 入力モード値
	TCHAR					InputName[ 16 ] ;					// 入力モードネーム
} ;

// テーブル-----------------------------------------------------------------------

// 仮想キーコードをコントロール文字コードに変換するためのテーブル
extern char CtrlCode[ 10 ][ 2 ] ;

// 内部大域変数宣言 --------------------------------------------------------------

// 文字列入力データ
extern CHARBUFFER CharBuf ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// 字符编码缓存操作相关
extern	int			InitializeInputCharBuf( void ) ;										// 文字コードバッファ関係の初期化
#ifndef DX_NON_KEYEX
extern	LRESULT		IMEProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam ) ;			// ＩＭＥメッセージのコールバック関数
extern	void		RefreshIMEFlag( int Always = FALSE ) ;										// ＩＭＥを使用するかどうかの状態を更新する
#endif

}

#endif // DX_NON_INPUTSTRING

#endif // __DXINPUTSTRING_H__
