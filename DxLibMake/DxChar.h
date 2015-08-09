// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		文字コード関係プログラムヘッダファイル
// 
// 				Ver 3.14d
// 
// -------------------------------------------------------------------------------

#ifndef __DXCHAR_H__
#define __DXCHAR_H__

// インクルード ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxLib.h"
#include "DxStatic.h"

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

#define DX_CODEPAGE_SHIFTJIS	(932)
#define DX_CODEPAGE_GB2312		(936)
#define DX_CODEPAGE_UHC			(949)
#define DX_CODEPAGE_BIG5		(950)
#define DX_CODEPAGE_UTF16LE		(1200)
#define DX_CODEPAGE_UTF16BE		(1201)
#define DX_CODEPAGE_ASCII		(1252)
#define DX_CODEPAGE_UTF8		(65001)
#define DX_CODEPAGE_UTF32LE		(32766)			// 正式なコードが無かったので勝手に定義
#define DX_CODEPAGE_UTF32BE		(32767)			// 正式なコードが無かったので勝手に定義

// シフトJIS２バイト文字判定
#define CHECK_SHIFTJIS_2BYTE( x )			( ( BYTE )( ( ( ( BYTE )(x) ) ^ 0x20) - ( BYTE )0xa1 ) < 0x3c )

// UTF16LEサロゲートペア判定( リトルエンディアン環境用 )
#define CHECK_CPU_LE_UTF16LE_4BYTE( x )		( ( ( x ) & 0xfc00 ) == 0xd800 )

// UTF16LEサロゲートペア判定( ビッグエンディアン環境用 )
#define CHECK_CPU_BE_UTF16LE_4BYTE( x )		( ( ( ( ( ( ( WORD )( x ) ) >> 8 ) & 0xff ) | ( ( ( WORD )( x ) << 8 ) & 0xff00 ) ) & 0xfc00 ) == 0xd800 )

// UTF16BEサロゲートペア判定( リトルエンディアン環境用 )
#define CHECK_CPU_LE_UTF16BE_4BYTE( x )		CHECK_CPU_BE_UTF16LE_4BYTE( x )

// UTF16BEサロゲートペア判定( ビッグエンディアン環境用 )
#define CHECK_CPU_BE_UTF16BE_4BYTE( x )		CHECK_CPU_LE_UTF16LE_4BYTE( x )

// wchar_t サロゲートペア判定( UTF-32 or UTF-16 想定 )
#ifdef WCHAR_T_BE
	#define CHECK_WCHAR_T_DOUBLE( x )	( sizeof( wchar_t ) == 2 && ( ( ( WORD )( x ) & 0x00fc ) == 0x00d8 ) )
#else
	#define CHECK_WCHAR_T_DOUBLE( x )	( sizeof( wchar_t ) == 2 && ( ( ( WORD )( x ) & 0xfc00 ) == 0xd800 ) )
#endif

// 構造体定義 --------------------------------------------------------------------

// UTF-16と各文字コードの対応表の情報
struct CHARCODETABLEINFO
{
	WORD				MultiByteToUTF16[ 0x10000 ] ;		// 各文字コードからUTF-16に変換するためのテーブル
	WORD				UTF16ToMultiByte[ 0x10000 ] ;		// UTF-16から各文字コードに変換するためのテーブル
} ;

// 文字コード処理関係で使用する情報
struct CHARCODESYSTEM
{
	int					InitializeFlag ;					// 初期化処理を行ったかどうか( TRUE:行った  FALSE:行っていない )

	CHARCODETABLEINFO	CharCodeCP932Info ;					// Shift-JISの文字コード情報
	CHARCODETABLEINFO	CharCodeCP936Info ;					// GB2312の文字コード情報
	CHARCODETABLEINFO	CharCodeCP949Info ;					// UHCの文字コード情報
	CHARCODETABLEINFO	CharCodeCP950Info ;					// BIG5の文字コード情報
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

extern CHARCODESYSTEM g_CharCodeSystem ;

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	int				InitCharCode( void ) ;																			// 文字コード関連処理の初期化を行う

extern	int				GetCodePageUnitSize(	int CodePage ) ;														// 指定のコードページの情報最少サイズを取得する( 戻り値：バイト数 )
extern	int				GetCharBytes(			const char *CharCode, int CodePage ) ;									// １文字のバイト数を取得する( 戻り値：１文字のバイト数 )
extern	DWORD			GetCharCode(			const char *CharCode, int CodePage, int *CharBytes ) ;					// １文字の文字コードと文字のバイト数を取得する
extern	int				PutCharCode(			DWORD CharCode, int CodePage, char *Dest ) ;							// 文字コードを通常の文字列に変換する、終端にヌル文字は書き込まない( 戻り値：書き込んだバイト数 )
extern	DWORD			ConvCharCode(			DWORD SrcCharCode, int SrcCodePage, int DestCodePage ) ;				// 文字コードを指定のコードページの文字に変換する
extern	int				ConvCharCodeString(		const DWORD *Src, int SrcCodePage, DWORD *Dest, int DestCodePage ) ;	// １文字４バイトの配列を、別コードページの１文字４バイトの配列に変換する( 戻り値：変換後のサイズ、ヌル文字含む( 単位：バイト ) )
extern	int				StringToCharCodeString( const char  *Src, int SrcCodePage, DWORD  *Dest ) ;						// 文字列を１文字４バイトの配列に変換する( 戻り値：変換後のサイズ、ヌル文字含む( 単位：バイト ) )
extern	int				CharCodeStringToString( const DWORD *Src, char *Dest, int DestCodePage ) ;						// １文字４バイトの配列を文字列に変換する( 戻り値：変換後のサイズ、ヌル文字含む( 単位：バイト ) )
extern	int				ConvString(				const char *Src, int SrcCodePage, char *Dest, int DestCodePage ) ;		// 文字列を指定のコードページの文字列に変換する( 戻り値：変換後のサイズ、ヌル文字含む( 単位：バイト ) )
extern	int				GetStringCharNum(		const char *String, int CodePage ) ;									// 文字列に含まれる文字数を取得する
extern	const char *	GetStringCharAddress(	const char *String, int CodePage, int Index ) ;							// 指定番号の文字のアドレスを取得する
extern	DWORD			GetStringCharCode(		const char *String, int CodePage, int Index ) ;							// 指定番号の文字のコードを取得する

extern	void			CL_strcpy(            int CodePage, char *Dest, const char *Src ) ;
extern	void			CL_strncpy(           int CodePage, char *Dest, const char *Src, int Num ) ;
extern	void			CL_strcat(            int CodePage, char *Dest, const char *Src ) ;
extern	const char *	CL_strstr(            int CodePage, const char *Str1, const char *Str2 ) ;
extern	int				CL_strlen(            int CodePage, const char *Str ) ;
extern	int				CL_strcmp(            int CodePage, const char *Str1, const char *Str2 ) ;
extern	int				CL_stricmp(           int CodePage, const char *Str1, const char *Str2 ) ;
extern	int				CL_strcmp_str2_ascii( int CodePage, const char *Str1, const char *Str2 ) ;
extern	int				CL_strncmp(           int CodePage, const char *Str1, const char *Str2, int Size ) ;
extern	const char *	CL_strchr(            int CodePage, const char *Str, DWORD CharCode ) ;
extern	const char *	CL_strrchr(           int CodePage, const char *Str, DWORD CharCode ) ;
extern	char *			CL_strupr(            int CodePage, char *Str ) ;
extern	int				CL_vsprintf(          int CodePage, int IsWChar, int CharCodePage, int WCharCodePage, char *Buffer, const char *FormatString, va_list Arg ) ;
extern	int				CL_sprintf(           int CodePage, int IsWChar, int CharCodePage, int WCharCodePage, char *Buffer, const char *FormatString, ... ) ;
extern	char *			CL_itoa(              int CodePage, int Value, char *Buffer, int Radix ) ;
extern	int				CL_atoi(              int CodePage, const char *Str ) ;
extern	double			CL_atof(              int CodePage, const char *Str ) ;
extern	int				CL_vsscanf(           int CodePage, int IsWChar, int CharCodePage, int WCharCodePage, const char *String, const char *FormatString, va_list Arg ) ;
extern	int				CL_sscanf(            int CodePage, int IsWChar, int CharCodePage, int WCharCodePage, const char *String, const char *FormatString, ... ) ;

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXCHAR_H__
