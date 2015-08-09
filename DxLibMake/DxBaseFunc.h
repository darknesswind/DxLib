// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		標準関数の互換関数プログラムヘッダファイル
// 
// 				Ver 3.14d
// 
// -------------------------------------------------------------------------------

#ifndef __DXBASEFUNC_H__
#define __DXBASEFUNC_H__

// インクルード ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxChar.h"
#include <stdarg.h>

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

#ifdef UNICODE
	#define _TMULT( strp, cset )					CheckUTF16H( (strp) )
#else
	#define _TMULT( strp, cset )					CheckMultiByteChar( (strp), (cset) )
#endif

#define DXWCHAR		WORD
#ifdef UNICODE
	#define _DXWTP( x )		( DXWCHAR * )( x )
	#define _DXWTR( x )		( DXWCHAR * )( _T( x ) )
#else
	#define _DXWTP( x )		x
	#define _DXWTR( x )		x
#endif


#define CHAR_TO_WCHAR_TEMPSTRINGLENGTH				(512)


#define CHAR_TO_WCHAR_T_STRING_BEGIN( str )													\
	BYTE    str##Buffer[ sizeof( wchar_t ) * CHAR_TO_WCHAR_TEMPSTRINGLENGTH ] ;				\
	void    *Alloc##str##Buffer = NULL ;													\
	wchar_t *Use##str##Buffer = NULL ;														\
	int      str##Length ;


#define CHAR_TO_WCHAR_T_STRING_SETUP( str, err, codepage )												\
	if( str != NULL )																					\
	{																									\
		str##Length = CL_strlen( codepage, str ) ;														\
		if( str##Length + 16 > CHAR_TO_WCHAR_TEMPSTRINGLENGTH )											\
		{																								\
			Alloc##str##Buffer = ( wchar_t * )DXALLOC( sizeof( wchar_t ) * ( str##Length + 16 ) ) ;		\
			if( Alloc##str##Buffer == NULL )															\
			{																							\
				err ;																					\
			}																							\
			Use##str##Buffer = ( wchar_t * )Alloc##str##Buffer ;										\
		}																								\
		else																							\
		{																								\
			Use##str##Buffer = ( wchar_t * )str##Buffer ;												\
		}																								\
																										\
		ConvString( ( const char * )str, codepage, ( char * )Use##str##Buffer, WCHAR_T_CODEPAGE ) ;		\
	}


#define CHAR_TO_WCHAR_T_STRING_END( str )			\
	if( Alloc##str##Buffer != NULL )				\
	{												\
		DXFREE( Alloc##str##Buffer ) ;				\
		Alloc##str##Buffer = NULL ;					\
	}



#define SHIFT_JIS_TO_WCHAR_T_STRING_BEGIN( str )		CHAR_TO_WCHAR_T_STRING_BEGIN( str )
#define SHIFT_JIS_TO_WCHAR_T_STRING_SETUP( str, err )	CHAR_TO_WCHAR_T_STRING_SETUP( str, err, DX_CODEPAGE_SHIFTJIS )
#define SHIFT_JIS_TO_WCHAR_T_STRING_END( str )			CHAR_TO_WCHAR_T_STRING_END( str )


#define SHIFT_JIS_TO_WCHAR_T_STRING_ONE_BEGIN( str, err )		\
	SHIFT_JIS_TO_WCHAR_T_STRING_BEGIN( str )					\
	SHIFT_JIS_TO_WCHAR_T_STRING_SETUP( str, err )



#define TCHAR_TO_WCHAR_T_STRING_BEGIN( str )			CHAR_TO_WCHAR_T_STRING_BEGIN( str )
#define TCHAR_TO_WCHAR_T_STRING_SETUP( str, err )		CHAR_TO_WCHAR_T_STRING_SETUP( str, err, _TCODEPAGE )
#define TCHAR_TO_WCHAR_T_STRING_END( str )				CHAR_TO_WCHAR_T_STRING_END( str )


#define TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( str, err )		\
	TCHAR_TO_WCHAR_T_STRING_BEGIN( str )					\
	TCHAR_TO_WCHAR_T_STRING_SETUP( str, err )




#define TCHAR_FORMATSTRING_SETUP					\
	va_list VaList ;								\
	TCHAR String[ 2048 ] ;							\
													\
	va_start( VaList, FormatString ) ;				\
													\
	_TVSPRINTF( String, FormatString, VaList ) ;	\
													\
	va_end( VaList ) ;


#define WCHAR_T_FORMATSTRING_SETUP					\
	va_list VaList ;								\
	wchar_t String[ 2048 ] ;						\
													\
	va_start( VaList, FormatString ) ;				\
													\
	_VSWPRINTF( String, FormatString, VaList ) ;	\
													\
	va_end( VaList ) ;


// 構造体定義 --------------------------------------------------------------------

// 標準関数の互換関数で使用する情報
struct BASEFUNCSYSTEM
{
	int					UseCharSet ;						// 使用する文字列セット
	int					Use_char_CodePage ;					// char で使用する文字コードページ
	int					Use_wchar_t_CodePage ;				// wchar_t 使用する文字コードページ
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

extern BASEFUNCSYSTEM g_BaseFuncSystem ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// TCHAR用定義
#ifdef UNICODE
	#define _TISWCHAR								(TRUE)
	#define _TSTRCAT								_WCSCAT
	#define _TSTRSTR								_WCSSTR
	#define _TSTRCPY								_WCSCPY
	#define _TSTRLEN								_WCSLEN
	#define _TSTRCMP								_WCSCMP
	#define _TSTRNCMP								_WCSNCMP
	#define _TSTRNCPY								_WCSNCPY
	#define _TSTRUPR								_WCSUPR
	#define _TSTRCHR								_WCSCHR
	#define _TSTRRCHR								_WCSRCHR
	#define _TVSPRINTF( buf, frm, vlst )			_VSWPRINTF( buf, frm, vlst )
	#define _TSPRINTF								_SWPRINTF
	#define _TTOF									_ATOFW
	#define _TTOI									_ATOIW
	#define _ITOT									_ITOAW
	#define _TCHDIR									_WCHDIR
	#define _TGETCWD								_WGETCWD
	#define _TCODEPAGE								WCHAR_T_CODEPAGE
	#define _TCHARSIZE								GetCodePageUnitSize( WCHAR_T_CODEPAGE )
#else
	#define _TISWCHAR								(FALSE)
	#define _TSTRCAT								_STRCAT
	#define _TSTRSTR								_STRSTR
	#define _TSTRCPY								_STRCPY
	#define _TSTRLEN								_STRLEN
	#define _TSTRCMP								_STRCMP
	#define _TSTRNCMP								_STRNCMP
	#define _TSTRNCPY								_STRNCPY
	#define _TSTRUPR								_STRUPR
	#define _TSTRCHR								_STRCHR
	#define _TSTRRCHR								_STRRCHR
	#define _TVSPRINTF								_VSPRINTF
	#define _TSPRINTF								_SPRINTF
	#define _TTOF									_ATOF
	#define _TTOI									_ATOI
	#define _ITOT									_ITOA
	#define _TCHDIR									_CHDIR
	#define _TGETCWD								_GETCWD
	#define _TCODEPAGE								CHAR_CODEPAGE
	#define _TCHARSIZE								GetCodePageUnitSize( CHAR_CODEPAGE )
#endif

#define CHAR_CODEPAGE		( g_BaseFuncSystem.Use_char_CodePage    == 0 ? ( _SET_DEFAULT_CODEPAGE(), g_BaseFuncSystem.Use_char_CodePage    ) : g_BaseFuncSystem.Use_char_CodePage    )
#define WCHAR_T_CODEPAGE	( g_BaseFuncSystem.Use_wchar_t_CodePage == 0 ? ( _SET_DEFAULT_CODEPAGE(), g_BaseFuncSystem.Use_wchar_t_CodePage ) : g_BaseFuncSystem.Use_wchar_t_CodePage )

extern	void			_SET_CHARSET( int CharSet ) ;
extern	int				_GET_CHARSET( void ) ;

extern	void			_SET_DEFAULT_CODEPAGE( void ) ;
extern	void			_SET_CHAR_CODEPAGE( int CodePage ) ;
extern	int				_GET_CHAR_CODEPAGE( void ) ;
extern	void			_SET_WCHAR_T_CODEPAGE( int CodePage ) ;
extern	int				_GET_WCHAR_T_CODEPAGE( void ) ;


// 自前標準関数系
extern	void			_STRCPY(  char    *Dest, const char    *Src ) ;
extern	void			_WCSCPY(  wchar_t *Dest, const wchar_t *Src ) ;
extern	void			_STRNCPY( char    *Dest, const char    *Src, int Num ) ;
extern	void			_WCSNCPY( wchar_t *Dest, const wchar_t *Src, int Num ) ;
extern	void			_STRCAT(  char    *Dest, const char    *Src ) ;
extern	void			_WCSCAT(  wchar_t *Dest, const wchar_t *Src ) ;
extern	const char *	_STRSTR(  const char    *Str1, const char    *Str2 ) ;
extern	const wchar_t *	_WCSSTR(  const wchar_t *Str1, const wchar_t *Str2 ) ;
extern	int				_STRLEN(  const char    *Str ) ;
extern	int				_WCSLEN(  const wchar_t *Str ) ;
extern	int				_STRCMP(  const char    *Str1, const char    *Str2 ) ;
extern	int				_WCSCMP(  const wchar_t *Str1, const wchar_t *Str2 ) ;
extern	int				_STRICMP( const char    *Str1, const char    *Str2 ) ;
extern	int				_WCSICMP( const wchar_t *Str1, const wchar_t *Str2 ) ;
extern	int				_STRNCMP( const char    *Str1, const char    *Str2, int Size ) ;
extern	int				_WCSNCMP( const wchar_t *Str1, const wchar_t *Str2, int Size ) ;
extern	const char *	_STRCHR(  const char    *Str, int     Char ) ;
extern	const wchar_t *	_WCSCHR(  const wchar_t *Str, wchar_t Char ) ;
extern	const char *	_STRRCHR( const char    *Str, int     Char ) ;
extern	const wchar_t *	_WCSRCHR( const wchar_t *Str, wchar_t Char ) ;
extern	char *			_STRUPR(  char    *Str ) ;
extern	wchar_t *		_WCSUPR(  wchar_t *Str ) ;
extern	int				_VSPRINTF(  char    *Buffer, const char    *FormatString, va_list Arg ) ;
extern	int				_VSWPRINTF( wchar_t *Buffer, const wchar_t *FormatString, va_list Arg ) ; // 実体は DxUseCLib.cpp の中
extern	int				_SPRINTF(   char    *Buffer, const char    *FormatString, ... ) ;
extern	int				_SWPRINTF(  wchar_t *Buffer, const wchar_t *FormatString, ... ) ;
extern	void			_MEMSET(  void *Memory, unsigned char Char, size_t Size ) ;
extern	void			_MEMSETD( void *Memory, unsigned int  Data, int Num ) ;
extern	int				_MEMCMP(  const void *Buffer1, const void *Buffer2, int Size ) ;
extern	void			_MEMCPY(  void *Buffer1, const void *Buffer2, size_t Size ) ;
extern	void			_MEMMOVE( void *Buffer1, const void *Buffer2, size_t Size ) ;

extern	char *			_GETCWD(  char    *Buffer, size_t Length ) ;
extern	wchar_t *		_WGETCWD( wchar_t *Buffer, size_t Length ) ;
extern	int				_CHDIR(   const char *    DirName ) ;
extern	int				_WCHDIR(  const wchar_t * DirName ) ;

extern	void			_SINCOS( float Angle, float *DestSin, float *DestCos ) ;
extern	void			_SINCOSD( double Angle, double *DestSin, double *DestCos ) ;
extern	int				_FPCLASS( double x ) ;
extern	float			_POW( float x, float y ) ;
extern	double			_POWD( double x, double y ) ;
extern	float			_ATAN2( float y, float x ) ;
extern	double			_ATAN2D( double y, double x ) ;
extern	float			_EXPF( float x ) ;
extern	double			_EXPFD( double x ) ;
extern	float			_ASIN( float Real ) ;
extern	double			_ASIND( double Real ) ;
extern	float			_ACOS( float Real ) ;
extern	double			_ACOSD( double Real ) ;
extern	double			_LOG10( double Real ) ;
extern	int				_ABS( int Number ) ;
extern	float			_FABS( float Real ) ;
extern	double			_DABS( double Real ) ;
extern	int				_FTOL( float Real ) ;
extern	int				_DTOL( double Real ) ;
extern	LONGLONG		_DTOL64( double Real ) ;
extern	void			_FTOL4( float *Real, int *Buffer4 ) ;
extern	void			_DTOL4( double *Real, int *Buffer4 ) ;
extern	float			_SQRT( float Real ) ;
extern	double			_SQRTD( double Real ) ;
extern	int				_ATOI( const char *String ) ;
extern	int				_ATOIW( const wchar_t *String ) ;
extern	double			_ATOF( const char *String ) ;
extern	double			_ATOFW( const wchar_t *String ) ;
extern	char *			_ITOA( int Value, char *Buffer, int Radix ) ;
extern	wchar_t	*		_ITOAW( int Value, wchar_t *Buffer, int Radix ) ;
extern	void			_SHR64( DWORD *Number64, int ShftNum ) ;
extern	void			_SHL64( DWORD *Number64, int ShftNum ) ;
extern	void			_MUL128_1( DWORD *Src64_1, DWORD *Src64_2, DWORD *Dest128 );
extern	void			_DIV128_1( DWORD *Src128,  DWORD *Div64,   DWORD *Dest64  );

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXBASEFUNC_H__

