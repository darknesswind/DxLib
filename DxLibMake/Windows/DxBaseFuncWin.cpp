//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用標準関数の互換関数プログラム
// 
//  	Ver 3.14d
// 
//-----------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード ---------------------------------------------------------------
#include "DxBaseFuncWin.h"
#include "../DxBaseFunc.h"
#include "../DxChar.h"
#include "../DxLog.h"
#include <windows.h>
#include <float.h>
#include <math.h>

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 -----------------------------------------------------------------

// 構造体宣言 -----------------------------------------------------------------

// データ定義 -----------------------------------------------------------------

// 関数宣言 -------------------------------------------------------------------

// プログラム -----------------------------------------------------------------

extern void	_SET_DEFAULT_CODEPAGE( void )
{
	if( _GET_CHARSET() == 0 )
	{
		_SET_WCHAR_T_CODEPAGE( sizeof( wchar_t ) > 2 ? DX_CODEPAGE_UTF32LE : DX_CODEPAGE_UTF16LE ) ;

		switch( GetOEMCP() )
		{
		case DX_CODEPAGE_UHC :      _SET_CHARSET( DX_CHARSET_HANGEUL ) ; _SET_CHAR_CODEPAGE( DX_CODEPAGE_UHC      ) ; break ;
		case DX_CODEPAGE_BIG5 :     _SET_CHARSET( DX_CHARSET_BIG5    ) ; _SET_CHAR_CODEPAGE( DX_CODEPAGE_BIG5     ) ; break ;
		case DX_CODEPAGE_GB2312 :   _SET_CHARSET( DX_CHARSET_GB2312  ) ; _SET_CHAR_CODEPAGE( DX_CODEPAGE_GB2312   ) ; break ;
		case DX_CODEPAGE_SHIFTJIS : _SET_CHARSET( DX_CHARSET_SHFTJIS ) ; _SET_CHAR_CODEPAGE( DX_CODEPAGE_SHIFTJIS ) ; break ;
		default :                   _SET_CHARSET( DX_CHARSET_SHFTJIS ) ; _SET_CHAR_CODEPAGE( DX_CODEPAGE_SHIFTJIS ) ; break ;
		}
	}
}

extern char *_GETCWD( char *Buffer, size_t Length )
{
	wchar_t TempBuffer[ 1024 ] ;

	GetCurrentDirectoryW( 1024, TempBuffer ) ;
	ConvString( ( const char * )TempBuffer, WIN32_WCHAR_CODEPAGE, Buffer, CHAR_CODEPAGE ) ;

	return Buffer ;
}

extern wchar_t *_WGETCWD( wchar_t *Buffer, size_t Length )
{
	wchar_t TempBuffer[ 1024 ] ;

	GetCurrentDirectoryW( 1024, TempBuffer ) ;
	ConvString( ( const char * )TempBuffer, WIN32_WCHAR_CODEPAGE, ( char * )Buffer, WCHAR_T_CODEPAGE ) ;

	return Buffer ;
}

extern int _CHDIR( const char *DirName )
{
	wchar_t TempBuffer[ 1024 ] ;
	int Result ;

	ConvString( DirName, CHAR_CODEPAGE, ( char * )TempBuffer, WIN32_WCHAR_CODEPAGE ) ;
	Result = SetCurrentDirectoryW( TempBuffer ) ;

	return Result != 0 ? 0 : -1 ;
}

extern int _WCHDIR(  const wchar_t * DirName )
{
	wchar_t TempBuffer[ 1024 ] ;
	int Result ;

	ConvString( ( const char * )DirName, WCHAR_T_CODEPAGE, ( char * )TempBuffer, WIN32_WCHAR_CODEPAGE ) ;
	Result = SetCurrentDirectoryW( TempBuffer ) ;

	return Result != 0 ? 0 : -1 ;
}

extern int _FPCLASS( double x )
{
	return _fpclass( x ) ;
}

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

