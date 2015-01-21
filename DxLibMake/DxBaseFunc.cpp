// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		標準関数の互換関数プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ------------------------------------------------------------------
#include "DxBaseFunc.h"
#include "DxLib.h"
#include <wchar.h>
#include <math.h>


#ifdef DX_NON_INLINE_ASM
  #include <math.h>
#endif

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

static int UseCharSet ;		// 使用する文字列セット
static int UseCodePage ;	// 使用する文字コードページ

// 関数プロトタイプ宣言-----------------------------------------------------------

// プログラム --------------------------------------------------------------------

// 文字コード関連
extern void _SET_CHARSET( int CharSet )
{
	UseCharSet = CharSet ;
}

extern int _GET_CHARSET( void )
{
	return UseCharSet ;
}

extern void _SET_CODEPAGE( int CodePage )
{
	UseCodePage = CodePage ;
}

extern int _GET_CODEPAGE( void )
{
	return UseCodePage ;
}


// サロゲートペアかどうかの判定( TRUE:４バイト文字上位  FALSE:２バイト文字 )
extern int CheckUTF16H( wchar_t c )
{
	return ( DWORD )c >= 0xd800 && ( DWORD )c <= 0xdbff ? TRUE : FALSE ;
}

// 指定の文字列の指定の位置の文字のタイプを返す( 0:１キャラ文字  1:２キャラ文字の１キャラ目  2:２キャラ文字の２キャラ目 )
extern int CheckDoubleChar( const TCHAR *String, int CharPosition, int CharSet /* DX_CHARSET_SHFTJIS */ )
{
	int i ;

	for( i = 0 ; String[ i ] != _T( '\0' ) ; )
	{
		if( _TMULT( String[ i ], CharSet ) == TRUE )
		{
			if( i     == CharPosition ) return 1 ;
			if( i + 1 == CharPosition ) return 2 ;
			i += 2 ;
		}
		else
		{
			if( i == CharPosition ) return 0 ;
			i ++ ;
		}
	}

	return -1 ;
}

// ２バイト文字か調べる( TRUE:２バイト文字  FALSE:１バイト文字 )
extern int CheckMultiByteChar( char CharCode, int CharSet /* DX_CHARSET_SHFTJIS */ )
{
	switch( CharSet )
	{
	case DX_CHARSET_HANGEUL :
	case DX_CHARSET_BIG5 :
	case DX_CHARSET_GB2312 :
		return ( (unsigned char)CharCode & 0x80 ) != 0 ;
	}

	return (unsigned char)(( ((unsigned char)CharCode) ^ 0x20) - (unsigned char)0xa1) < 0x3c;
//	return ( (unsigned char)CharCode >= 0x81 && (unsigned char)CharCode <= 0x9F ) || ( (unsigned char)CharCode >= 0xE0 && (unsigned char)CharCode <= 0xFC ) ;
}

// 指定の文字列の指定の位置の文字のタイプを返す( 0:１バイト文字  1:２バイト文字の１バイト目  2:２バイト文字の２バイト目 )
extern int CheckMultiByteString( const char *String, int BytePosition, int CharSet /* DX_CHARSET_SHFTJIS */ )
{
	int i ;

	for( i = 0 ; String[ i ] != '\0' ; )
	{
		if( CheckMultiByteChar( String[ i ], CharSet ) == TRUE )
		{
			if( i     == BytePosition ) return 1 ;
			if( i + 1 == BytePosition ) return 2 ;
			i += 2 ;
		}
		else
		{
			if( i == BytePosition ) return 0 ;
			i ++ ;
		}
	}

	return -1 ;
}

// 指定の文字列の指定の位置の文字のタイプを返す( 0:非サロゲートペア文字  1:サロゲートペア文字の１キャラ目  2:サロゲートペア文字の２キャラ目 )
extern int CheckUTF16HChar( const wchar_t *String, int CharPosition )
{
	int i ;

	for( i = 0 ; String[ i ] != L'\0' ; )
	{
		if( CheckUTF16H( String[ i ] ) == TRUE )
		{
			if( i     == CharPosition ) return 1 ;
			if( i + 1 == CharPosition ) return 2 ;
			i += 2 ;
		}
		else
		{
			if( i == CharPosition ) return 0 ;
			i ++ ;
		}
	}

	return -1 ;
}


// 文字列処理関数
extern void _STRCPY( char *Dest, const char *Src )
{
	int i ;
	for( i = 0 ; Src[i] != '\0' ; i ++ ) Dest[i] = Src[i] ;
	Dest[i] = '\0' ;
}

extern void _WCSCPY( wchar_t *Dest, const wchar_t *Src )
{
	int i ;
	for( i = 0 ; Src[i] != L'\0' ; i ++ ) Dest[i] = Src[i] ;
	Dest[i] = L'\0' ;
}

extern void _STRNCPY( char *Dest, const char *Src, int Num )
{
	int i ;
	for( i = 0 ; i < Num && Src[i] != '\0' ; i ++ ) Dest[i] = Src[i] ;
	Dest[i] = '\0' ;
}

extern void _WCSNCPY( wchar_t *Dest, const wchar_t *Src, int Num )
{
	int i ;
	for( i = 0 ; i < Num && Src[i] != L'\0' ; i ++ ) Dest[i] = Src[i] ;
	Dest[i] = L'\0' ;
}

extern void _STRCAT( char *Dest, const char *Src )
{
	int i ;
	for( i = 0 ; Dest[i] != '\0' ; i ++ ){}
	_STRCPY( &Dest[i], Src ) ;
}

extern void _WCSCAT( wchar_t *Dest, const wchar_t *Src )
{
	int i ;
	for( i = 0 ; Dest[i] != L'\0' ; i ++ ){}
	_WCSCPY( &Dest[i], Src ) ;
}

extern char *_STRSTR( const char *Str1, const char *Str2 )
{
	int i, j ;
	for( i = 0 ; Str1[i] != '\0' ; i ++ )
	{
		for( j = 0 ; Str2[j] != '\0' && Str1[i+j] != '\0' && Str1[i+j] == Str2[j] ; j ++ ){}
		if( Str2[j] == '\0' ) return (char *)&Str1[i] ;
		if( Str1[i+j] == '\0' ) return NULL ;
	}
	return NULL ;
}

extern wchar_t *_WCSSTR( const wchar_t *Str1, const wchar_t *Str2 )
{
	int i, j ;
	for( i = 0 ; Str1[i] != L'\0' ; i ++ )
	{
		for( j = 0 ; Str2[j] != L'\0' && Str1[i+j] != L'\0' && Str1[i+j] == Str2[j] ; j ++ ){}
		if( Str2[j] == L'\0' ) return (wchar_t *)&Str1[i] ;
		if( Str1[i+j] == L'\0' ) return NULL ;
	}
	return NULL ;
}

extern int _STRLEN( const char *Str )
{
	int i ;

	for( i = 0 ; Str[ i ] ; i ++ ){}
	return i ;
}

extern int _WCSLEN( const wchar_t *Str )
{
	int i ;

	for( i = 0 ; Str[ i ] ; i ++ ){}
	return i ;
}

extern char *_STRCHR( const char *Str1, char Char )
{
	while( *Str1 != '\0' )
	{
		if( CheckMultiByteChar( *Str1, UseCharSet ) == TRUE )
		{
			Str1 += 2 ;
		}
		else
		{
			if( *Str1 == Char ) return (char *)Str1 ;
			Str1 ++ ;
		}
	}
	return NULL ;
}

extern wchar_t *_WCSCHR( const wchar_t *Str1, wchar_t Char )
{
	while( *Str1 != L'\0' )
	{
		if( CheckUTF16H( *Str1 ) == TRUE )
		{
			Str1 += 2 ;
		}
		else
		{
			if( *Str1 == Char ) return (wchar_t *)Str1 ;
			Str1 ++ ;
		}
	}
	return NULL ;
}

extern char *_STRRCHR( const char *Str1, char Char )
{
	char *lastp;

	lastp = NULL;
	while( *Str1 != '\0' )
	{
		if( CheckMultiByteChar( *Str1, UseCharSet ) == TRUE )
		{
			Str1 += 2 ;
		}
		else
		{
			if( *Str1 == Char )
			{
				lastp = (char *)Str1 ;
			}
			Str1 ++ ;
		}
	}
	return lastp ;
}

extern wchar_t *_WCSRCHR( const wchar_t *Str1, wchar_t Char )
{
	wchar_t *lastp;

	lastp = NULL;
	while( *Str1 != L'\0' )
	{
		if( CheckUTF16H( *Str1 ) == TRUE )
		{
			Str1 += 2 ;
		}
		else
		{
			if( *Str1 == Char )
			{
				lastp = (wchar_t *)Str1 ;
			}
			Str1 ++ ;
		}
	}
	return lastp ;
}

extern int _STRCMP( const char *Str1, const char *Str2 )
{
	int i ;
	for( i = 0 ; Str1[i] != '\0' && Str2[i] != '\0' && Str1[i] == Str2[i] ; i ++ ){}
	return ( Str1[i] != Str2[i] ) ? 1 : 0 ;
}

extern int _WCSCMP( const wchar_t *Str1, const wchar_t *Str2 )
{
	int i ;
	for( i = 0 ; Str1[i] != L'\0' && Str2[i] != L'\0' && Str1[i] == Str2[i] ; i ++ ){}
	return ( Str1[i] != Str2[i] ) ? 1 : 0 ;
}

extern int _STRCMPI( const char *Str1, const char *Str2 )
{
	int i ;
	char c1, c2;
	for( i = 0 ; Str1[i] != '\0' && Str2[i] != '\0'; i ++ )
	{
		c1 = Str1[i] >= 'a' && Str1[i] <= 'z' ? Str1[i] - 'a' + 'A' : Str1[i] ;
		c2 = Str2[i] >= 'a' && Str2[i] <= 'z' ? Str2[i] - 'a' + 'A' : Str2[i] ;
		if( c1 != c2 ) break;
	}
	return ( Str1[i] != Str2[i] ) ? 1 : 0 ;
}

extern char *_STRUPR( char *Str )
{
	while( *Str != '\0' )
	{
		if( CheckMultiByteChar( *Str, UseCharSet ) == TRUE )
		{
			Str += 2 ;
		}
		else
		{
			if( *Str >= 'a' && *Str <= 'z' ) *Str = *Str - 'a' + 'A' ;
			Str ++ ;
		}
	}

	return Str ;
}

extern wchar_t *_WCSUPR( wchar_t *Str )
{
	while( *Str != L'\0' )
	{
		if( CheckUTF16H( *Str ) == TRUE )
		{
			Str += 2 ;
		}
		else
		{
			if( *Str >= L'a' && *Str <= L'z' ) *Str = *Str - L'a' + L'A' ;
			Str ++ ;
		}
	}

	return Str ;
}

extern int _STRNCMP( const char *Str1, const char *Str2, int Size )
{
	int i ;
	for( i = 0 ; i < Size && Str1[i] == Str2[i] ; i ++ ){}
	return i != Size ? 1 : 0 ;
}

extern int _WCSNCMP( const wchar_t *Str1, const wchar_t *Str2, int Size )
{
	int i ;
	for( i = 0 ; i < Size && Str1[i] == Str2[i] ; i ++ ){}
	return i != Size ? 1 : 0 ;
}

extern int _VSPRINTF( char *Buffer, const char *FormatString, va_list Arg )
{
	return vsprintf( Buffer, FormatString, Arg ) ;
}

extern int _SPRINTF( char *Buffer, const char *FormatString, ... )
{
	va_list VaList ;

	va_start( VaList, FormatString ) ;
	vsprintf( Buffer, FormatString, VaList ) ;
	va_end( VaList ) ;
	
	return 0 ;
}

extern int _SWPRINTF( DXWCHAR *Buffer, const DXWCHAR *FormatWString, ... )
{
	va_list VaList ;

	va_start( VaList, FormatWString ) ;
	_VSWPRINTF( Buffer, FormatWString, VaList ) ;
	va_end( VaList ) ;
	
	return 0 ;
}

extern void _MEMSET( void *Memory, unsigned char Char, size_t Size )
{
	if( Size == 0 ) return ;

#ifndef DX_NON_INLINE_ASM
	DWORD s = (DWORD)Size ;
	DWORD DwordChar = (DWORD)Char ;
	__asm
	{
		CLD
		MOV EAX, DwordChar
		MOV ECX, s
		MOV EDI, Memory
		REP STOSB
	}
#else
	do
	{
		*( ( BYTE * )Memory ) = Char ;
		Memory = ( BYTE * )Memory + 1 ;
	}while( -- Size != 0 ) ;
#endif
}

extern void _MEMSETD( void *Memory, unsigned int Data, int Num )
{
	if( Num == 0 ) return ;

#ifndef DX_NON_INLINE_ASM
	DWORD s = (DWORD)Num ;
	__asm
	{
		CLD
		MOV EAX, Data
		MOV ECX, s
		MOV EDI, Memory
		REP STOSD
	}
#else
	do
	{
		*( ( DWORD * )Memory ) = Data ;
		Memory = ( BYTE * )Memory + 4 ;
	}while( -- Num != 0 ) ;
#endif
}


extern int _MEMCMP( const void *Buffer1, const void *Buffer2, int Size )
{
	if( Size == 0 ) return 0 ;

#ifndef DX_NON_INLINE_ASM
	DWORD s1 = (DWORD)Buffer1 ;
	DWORD s2 = (DWORD)Buffer2 ;
	DWORD s = (DWORD)Size ;
	DWORD res ;
	__asm
	{
		CLD
		XOR EAX, EAX
		MOV ESI, s1
		MOV EDI, s2
		MOV ECX, s
		REPZ CMPSB
		MOV res, EAX
		JE END
		MOV EAX, 1
		MOV res, EAX
	END:
	}
	return res ;
#else
	do
	{
		if( *( ( BYTE * )Buffer1 ) > *( ( BYTE * )Buffer2 ) ) return -1 ;
		if( *( ( BYTE * )Buffer1 ) < *( ( BYTE * )Buffer2 ) ) return  1 ;
		Buffer1 = ( BYTE * )Buffer1 + 1 ;
		Buffer2 = ( BYTE * )Buffer2 + 1 ;
	}while( -- Size != 0 ) ;
	return 0 ;
#endif
}

extern void _MEMCPY( void *Buffer1, const void *Buffer2, size_t Size )
{
	if( Size == 0 ) return ;

#ifndef DX_NON_INLINE_ASM
	DWORD s1 = (DWORD)Buffer1 ;
	DWORD s2 = (DWORD)Buffer2 ;
	DWORD s = (DWORD)Size ;
	__asm
	{
		CLD
		MOV ESI, s2
		MOV EDI, s1
		MOV ECX, s
		REP MOVSB
	}
#else
	do
	{
		*( ( BYTE * )Buffer1 ) = *( ( BYTE * )Buffer2 ) ;
		Buffer1 = ( BYTE * )Buffer1 + 1 ;
		Buffer2 = ( BYTE * )Buffer2 + 1 ;
	}while( -- Size != 0 ) ;
#endif
}

extern void _MEMMOVE( void *Buffer1, const void *Buffer2, size_t Size )
{
	if( Size == 0 ) return ;

#ifndef DX_NON_INLINE_ASM
	DWORD s1 = (DWORD)Buffer1 ;
	DWORD s2 = (DWORD)Buffer2 ;
	DWORD s = (DWORD)Size ;
	__asm
	{
		CLD
		MOV ESI, s2
		MOV EDI, s1
		MOV ECX, s
		CMP ESI, EDI
		JE LABEL_END
		JA LABEL_1
		
		STD
		DEC ECX
		ADD ESI, ECX
		ADD EDI, ECX
		INC ECX

LABEL_1:
		REP MOVSB
LABEL_END:
		CLD
	}
#else
	if( ( BYTE * )Buffer1 < ( BYTE * )Buffer2 )
	{
		do
		{
			*( ( BYTE * )Buffer1 ) = *( ( BYTE * )Buffer2 ) ;
			Buffer1 = ( BYTE * )Buffer1 + 1 ;
			Buffer2 = ( BYTE * )Buffer2 + 1 ;
		}while( -- Size != 0 ) ;
	}
	else
	{
		Buffer1 = ( BYTE * )Buffer1 + Size - 1 ;
		Buffer2 = ( BYTE * )Buffer2 + Size - 1 ;
		do
		{
			*( ( BYTE * )Buffer1 ) = *( ( BYTE * )Buffer2 ) ;
			Buffer1 = ( BYTE * )Buffer1 - 1 ;
			Buffer2 = ( BYTE * )Buffer2 - 1 ;
		}while( -- Size != 0 ) ;
	}
#endif
}

extern void _SINCOS_DOUBLE( double Angle, double *DestSin, double *DestCos )
{
#ifndef DX_NON_INLINE_ASM
	double TempSin, TempCos ;

	__asm
	{
		fld		Angle
		fsincos
		fstp	TempCos
		fstp	TempSin
	}

	*DestSin = TempSin ;
	*DestCos = TempCos ;
#else
	*DestSin = sin( Angle ) ;
	*DestCos = cos( Angle ) ;
#endif
}

extern void _SINCOS( float Angle, float *DestSin, float *DestCos )
{
#ifndef DX_NON_INLINE_ASM
	float TempSin, TempCos ;

	__asm
	{
		fld		Angle
		fsincos
		fstp	TempCos
		fstp	TempSin
	}

	*DestSin = TempSin ;
	*DestCos = TempCos ;
#else
	*DestSin = ( float )sin( Angle ) ;
	*DestCos = ( float )cos( Angle ) ;
#endif
}

extern float _POW( float x, float y )
{
	return ( float )pow( x, y ) ;
}

extern float _ATAN2( float y, float x )
{
	return ( float )atan2( ( double )y, ( double )x ) ;
}

extern float _EXPF( float x )
{
	return ( float )exp( ( double )x ) ;
}

extern float _ASIN( float Real )
{
	return ( float )asin( ( double )Real ) ;
}

extern float _ACOS( float Real )
{
#ifdef __BCC
	if( Real < -1.0f )
	{
		Real = -1.0f ;
	}
	else
	if( Real > 1.0f )
	{
		Real = 1.0f ;
	}
#endif

	return ( float )acos( ( double )Real ) ;
}

extern	double _LOG10( double Real )
{
	return log10( Real ) ;
}

extern int _ABS( int Number )
{
	return Number < 0 ? -Number : Number ;
}

extern float _FABS( float Real )
{
	return Real < 0.0F ? -Real : Real ;
}

extern int _FTOL( float Real )
{
#ifndef DX_NON_INLINE_ASM
	DWORD Result ;
	WORD STFlag, DSTFlag ;
	__asm
	{
		fnstcw	STFlag
		mov		ax, STFlag
		or		ax, 0xC00
		mov		DSTFlag, ax
		fld		Real
		fldcw	DSTFlag
		fistp	Result
		fldcw	STFlag
	}
	return Result ;
#else
	return ( int )Real ;
#endif
}

extern int _DTOL( double Real )
{
#ifndef DX_NON_INLINE_ASM
	DWORD Result ;
	WORD STFlag, DSTFlag ;
	__asm
	{
		fnstcw	STFlag
		mov		ax, STFlag
		or		ax, 0xC00
		mov		DSTFlag, ax
		fld		Real
		fldcw	DSTFlag
		fistp	Result
		fldcw	STFlag
	}
	return Result ;
#else
	return ( int )Real ;
#endif
}

extern LONGLONG _DTOL64( double Real )
{
#ifndef DX_NON_INLINE_ASM
	LONGLONG Result ;
	WORD STFlag, DSTFlag ;
	__asm
	{
		fnstcw	STFlag
		mov		ax, STFlag
		or		ax, 0xC00
		mov		DSTFlag, ax
		fld		Real
		fldcw	DSTFlag
		fistp	Result
		fldcw	STFlag
	}
	return Result ;
#else
	return ( LONGLONG )Real ;
#endif
}

extern void _FTOL4( float *Real, int *Buffer4 )
{
#ifndef DX_NON_INLINE_ASM
	DWORD src = (DWORD)Real ;
	DWORD dst = (DWORD)Buffer4 ;
	WORD STFlag, DSTFlag ;
	__asm
	{
		fnstcw	STFlag
		mov		ax, STFlag
		or		ax, 0xC00
		mov		DSTFlag, ax
		mov		eax, src
		mov		ebx, dst
		fld		dword ptr [eax]
		fld		dword ptr [eax+4]
		fld		dword ptr [eax+8]
		fld		dword ptr [eax+12]
		fldcw	DSTFlag
		fistp	dword ptr [ebx+12]
		fistp	dword ptr [ebx+8]
		fistp	dword ptr [ebx+4]
		fistp	dword ptr [ebx]
		fldcw	STFlag
	}
#else
	Buffer4[ 0 ] = ( int )Real[ 0 ] ;
	Buffer4[ 1 ] = ( int )Real[ 1 ] ;
	Buffer4[ 2 ] = ( int )Real[ 2 ] ;
	Buffer4[ 3 ] = ( int )Real[ 3 ] ;
#endif
}

extern void _DTOL4( double *Real, int *Buffer4 )
{
#ifndef DX_NON_INLINE_ASM
	DWORD src = (DWORD)Real ;
	DWORD dst = (DWORD)Buffer4 ;
	WORD STFlag, DSTFlag ;
	__asm
	{
		fnstcw	STFlag
		mov		ax, STFlag
		or		ax, 0xC00
		mov		DSTFlag, ax
		mov		eax, src
		mov		ebx, dst
		fld		qword ptr [eax]
		fld		qword ptr [eax+8]
		fld		qword ptr [eax+16]
		fld		qword ptr [eax+24]
		fldcw	DSTFlag
		fistp	dword ptr [ebx+12]
		fistp	dword ptr [ebx+8]
		fistp	dword ptr [ebx+4]
		fistp	dword ptr [ebx]
		fldcw	STFlag
	}
#else
	Buffer4[ 0 ] = ( int )Real[ 0 ] ;
	Buffer4[ 1 ] = ( int )Real[ 1 ] ;
	Buffer4[ 2 ] = ( int )Real[ 2 ] ;
	Buffer4[ 3 ] = ( int )Real[ 3 ] ;
#endif
}


extern float _SQRT( float Real )
{
#ifndef DX_NON_INLINE_ASM
	__asm
	{
		fld		Real
		fsqrt
		fstp	Real
	}

	return Real ;
#else
	return ( float )sqrt( ( double )Real ) ;
#endif
}

extern int _ATOI( const char *String )
{
	int i, AddNum, Num, Number[32], Total, Minus ;
	const unsigned char *p = (const unsigned char *)String ;
	
	// 数字を探す
	while( *p != '\0' )
	{
		if( ( *p >= '0' && *p <= '9' ) || *p == '-' ) break ;
		p ++ ;
	}
	if( *p == '\0' ) return -1 ;
	if( *p == '-' )
	{
		Minus = 1 ;
		p ++ ;
	}
	else
	{
		Minus = 0 ;
	}
	
	// 数字を取得
	for( Num = 0 ; *p != '\0' && ( *p >= '0' && *p <= '9' ) ; Num ++, p ++ )
	{
		Number[Num] = *p - '0' ;
	}
	
	// 数値に変換
	AddNum = 1 ;
	Total = 0 ;
	for( i = 0 ; i < Num ; i ++, AddNum *= 10 )
	{
		Total += Number[Num-i-1] * AddNum ;
	}

	if( Minus == 1 ) Total = -Total ;

	// 数値を返す
	return Total ;
}

extern int _ATOIW( const DXWCHAR *String )
{
	int i, AddNum, Num, Number[32], Total, Minus ;
	const DXWCHAR *p = (const DXWCHAR *)String ;
	
	// 数字を探す
	while( *p != L'\0' )
	{
		if( ( *p >= L'0' && *p <= L'9' ) || *p == L'-' ) break ;
		p ++ ;
	}
	if( *p == L'\0' ) return -1 ;
	if( *p == L'-' )
	{
		Minus = 1 ;
		p ++ ;
	}
	else
	{
		Minus = 0 ;
	}
	
	// 数字を取得
	for( Num = 0 ; *p != L'\0' && ( *p >= L'0' && *p <= L'9' ) ; Num ++, p ++ )
	{
		Number[Num] = *p - L'0' ;
	}
	
	// 数値に変換
	AddNum = 1 ;
	Total = 0 ;
	for( i = 0 ; i < Num ; i ++, AddNum *= 10 )
	{
		Total += Number[Num-i-1] * AddNum ;
	}

	if( Minus == 1 ) Total = -Total ;

	// 数値を返す
	return Total ;
}

extern double _ATOF( const char *String )
{
	int MinusFlag ;
	int MinusFlag2 ;
	int TenFlag ;
	int SisuuFlag ;
	int i, j, k ;
	int num, num2, num3 ;
	char Number[128], Number2[128], Number3[128] ;
	double doubleNum, doubleNum2, doubleNum3, doubleCount ;
	LONGLONG int64Num, int64Count ;

	MinusFlag = FALSE;
	TenFlag = FALSE;
	SisuuFlag = FALSE;
	MinusFlag2 = FALSE;

	if( *String == '\0' ) return 0.0f ;
	if( *String == '-' )
	{
		String ++ ;
		if( *String == '\0' ) return 0.0f ;
		MinusFlag = TRUE;
	}

	i = 0;	//自然数
	j = 0;	//小数
	k = 0;	//指数
	for( ; *String != '\0' ; String ++ )
	{
		if( *String == '.' )
		{
			if( TenFlag ) return 0.0f ;
			TenFlag = TRUE;
		}
		else if( *String == 'e' || *String == 'E' )
		{
			if( SisuuFlag ) return 0.0f ;
			SisuuFlag = TRUE;
		}
		else if( *String == '-' || *String == '+' )
		{
			if( SisuuFlag == FALSE || k != 0 ) return 0.0f ;
			if( *String == '-' ) MinusFlag2 = TRUE;
		}
		else if( *String >= '0' && *String <= '9' )
		{
			if( SisuuFlag )
			{
				if( k >= 127 ) return 0.0f ;
				Number3[ k ] = *String - '0';
				k ++ ;
			}
			else if( TenFlag )
			{
				if( j >= 127 ) return 0.0f ;
				Number2[j] = *String - '0';
				j ++ ;
			}
			else
			{
				if( i >= 127 ) return 0.0f ;
				Number[i] = *String - '0';
				i ++ ;
			}
		}
		else return 0.0f ;
	}
	if( i == 0 && j == 0 )
		return 0.0f ;
	num = i ;
	num2 = j ;
	num3 = k ;

	doubleCount = 1.0 ;
	doubleNum = 0 ;
	for( i = num - 1 ; i >= 0; i --, doubleCount *= 10.0 )
	{
		if( Number[i] != 0 )
			doubleNum += Number[i] * doubleCount ;
	}
	if( MinusFlag ) doubleNum = -doubleNum ;

	doubleCount = 0.1 ;
	doubleNum2 = 0 ;
	for( i = 0 ; i < num2 ; i ++, doubleCount /= 10.0 )
	{
		if( Number2[i] != 0 )
			doubleNum2 += Number2[i] * doubleCount ;
	}
	if( MinusFlag ) doubleNum2 = -doubleNum2 ;

	int64Count = 1 ;
	int64Num = 0 ;
	for( i = num3 - 1; i >= 0; i --, int64Count *= 10 )
	{
		int64Num += Number3[i] * int64Count ;
	}
	if( MinusFlag2 ) int64Num = -int64Num ;

	doubleNum3 = 1.0 ;
	if( int64Num != 0 )
	{
		if( int64Num < 0 )
		{
			int64Num = -int64Num ;
			for( i = 0 ; i < int64Num ; i ++ )
				doubleNum3 /= 10.0 ;
		}
		else
		{
			for( i = 0 ; i < int64Num ; i++ )
				doubleNum3 *= 10.0 ;
		}
	}

	return ( doubleNum + doubleNum2 ) * doubleNum3 ;
}

extern double _ATOFW( const DXWCHAR *String )
{
	int MinusFlag ;
	int MinusFlag2 ;
	int TenFlag ;
	int SisuuFlag ;
	int i, j, k ;
	int num, num2, num3 ;
	DXWCHAR Number[128], Number2[128], Number3[128] ;
	double doubleNum, doubleNum2, doubleNum3, doubleCount ;
	LONGLONG int64Num, int64Count ;

	MinusFlag = FALSE;
	TenFlag = FALSE;
	SisuuFlag = FALSE;
	MinusFlag2 = FALSE;

	if( *String == L'\0' ) return 0.0f ;
	if( *String == L'-' )
	{
		String ++ ;
		if( *String == L'\0' ) return 0.0f ;
		MinusFlag = TRUE;
	}

	i = 0;	//自然数
	j = 0;	//小数
	k = 0;	//指数
	for( ; *String != L'\0' ; String ++ )
	{
		if( *String == L'.' )
		{
			if( TenFlag ) return 0.0f ;
			TenFlag = TRUE;
		}
		else if( *String == L'e' || *String == L'E' )
		{
			if( SisuuFlag ) return 0.0f ;
			SisuuFlag = TRUE;
		}
		else if( *String == L'-' || *String == L'+' )
		{
			if( SisuuFlag == FALSE || k != 0 ) return 0.0f ;
			if( *String == L'-' ) MinusFlag2 = TRUE;
		}
		else if( *String >= L'0' && *String <= L'9' )
		{
			if( SisuuFlag )
			{
				if( k >= 127 ) return 0.0f ;
				Number3[ k ] = *String - L'0';
				k ++ ;
			}
			else if( TenFlag )
			{
				if( j >= 127 ) return 0.0f ;
				Number2[j] = *String - L'0';
				j ++ ;
			}
			else
			{
				if( i >= 127 ) return 0.0f ;
				Number[i] = *String - L'0';
				i ++ ;
			}
		}
		else return 0.0f ;
	}
	if( i == 0 && j == 0 )
		return 0.0f ;
	num = i ;
	num2 = j ;
	num3 = k ;

	doubleCount = 1.0 ;
	doubleNum = 0 ;
	for( i = num - 1 ; i >= 0; i --, doubleCount *= 10.0 )
	{
		if( Number[i] != 0 )
			doubleNum += Number[i] * doubleCount ;
	}
	if( MinusFlag ) doubleNum = -doubleNum ;

	doubleCount = 0.1 ;
	doubleNum2 = 0 ;
	for( i = 0 ; i < num2 ; i ++, doubleCount /= 10.0 )
	{
		if( Number2[i] != 0 )
			doubleNum2 += Number2[i] * doubleCount ;
	}
	if( MinusFlag ) doubleNum2 = -doubleNum2 ;

	int64Count = 1 ;
	int64Num = 0 ;
	for( i = num3 - 1; i >= 0; i --, int64Count *= 10 )
	{
		int64Num += Number3[i] * int64Count ;
	}
	if( MinusFlag2 ) int64Num = -int64Num ;

	doubleNum3 = 1.0 ;
	if( int64Num != 0 )
	{
		if( int64Num < 0 )
		{
			int64Num = -int64Num ;
			for( i = 0 ; i < int64Num ; i ++ )
				doubleNum3 /= 10.0 ;
		}
		else
		{
			for( i = 0 ; i < int64Num ; i++ )
				doubleNum3 *= 10.0 ;
		}
	}

	return ( doubleNum + doubleNum2 ) * doubleNum3 ;
}

extern char *_ITOA( int Value, char *Buffer, int Radix )
{
	int j, i, Number[64], *ip ;
	unsigned char *p = (unsigned char *)Buffer ;

	if( Value == 0 )
	{
		Buffer[0] = '0' ;
		Buffer[1] = '\0' ;
	}
	else
	{
		if( Value < 0 )
		{
			*p = '-' ;
			p ++ ;
			Value = -Value ;
		}
		
		for( i = 0 ; Value != 0 ; i ++, Value /= Radix )
		{
			Number[i] = Value % Radix ;
		}

		ip = &Number[i-1] ;	
		for( j = 0 ; j < i ; j ++, p ++, ip -- )
		{
			if( *ip <= 9 )	*p = ( unsigned char )( '0' + *ip ) ;
			else			*p = ( unsigned char )( 'a' + *ip - 10 ) ;
		}
		*p = '\0' ;
	}
	
	return Buffer ;
}

extern wchar_t *_ITOAW( int Value, wchar_t *Buffer, int Radix )
{
	int j, i, Number[64], *ip ;
	wchar_t *p = Buffer ;

	if( Value == 0 )
	{
		Buffer[0] = L'0' ;
		Buffer[1] = L'\0' ;
	}
	else
	{
		if( Value < 0 )
		{
			*p = L'-' ;
			p ++ ;
			Value = -Value ;
		}
		
		for( i = 0 ; Value != 0 ; i ++, Value /= Radix )
		{
			Number[i] = Value % Radix ;
		}

		ip = &Number[i-1] ;	
		for( j = 0 ; j < i ; j ++, p ++, ip -- )
		{
			if( *ip <= 9 )	*p = ( wchar_t )( L'0' + *ip ) ;
			else			*p = ( wchar_t )( L'a' + *ip - 10 ) ;
		}
		*p = L'\0' ;
	}
	
	return Buffer ;
}

extern void _SHR64( DWORD *Number64, int ShftNum )
{
	Number64[0] = ( Number64[0] >> ShftNum ) | ( Number64[1] << ( 32 - ShftNum ) ) ;
	Number64[1] >>= ShftNum ;
}

extern void _SHL64( DWORD *Number64, int ShftNum )
{
	Number64[1] = ( Number64[1] << ShftNum ) | ( Number64[0] >> ( 32 - ShftNum ) ) ;
	Number64[0] <<= ShftNum ;
}

extern void _MUL128_1( DWORD *Src64_1, DWORD *Src64_2, DWORD *Dest128 )
{
#ifndef DX_NON_INLINE_ASM
	__asm
	{
		xor eax, eax
		mov esi, [Src64_1]
		mov ecx, [esi]
		mov eax, ecx
		mov edi, [Src64_2]
		mul dword ptr [edi]
		mov ebx, [Dest128]
		mov dword ptr [ebx   ], 0
		mov dword ptr [ebx+4 ], 0
		mov dword ptr [ebx+8 ], 0
		mov dword ptr [ebx+12], 0
		mov [ebx   ], eax
		mov [ebx+4 ], edx
		mov eax, ecx
		mul dword ptr [edi+4]
		add [ebx+4 ], eax
		adc [ebx+8 ], edx
		adc dword ptr [ebx+12], 0
		mov ecx, [esi+4]
		mov eax, ecx
		mul dword ptr [edi]
		add [ebx+4 ], eax
		adc [ebx+8 ], edx
		adc dword ptr [ebx+12], 0
		mov eax, ecx
		mul dword ptr [edi+4]
		add [ebx+8 ], eax
		adc [ebx+12], edx
	}
#else
	ULONGLONG Temp1, Temp2, Temp3, Temp4;
	ULONGLONG Dest1, Dest2, Dest3, Dest4;

	Temp1 = ( ULONGLONG )Src64_1[ 0 ] * ( ULONGLONG )Src64_2[ 0 ] ;
	Temp2 = ( ULONGLONG )Src64_1[ 0 ] * ( ULONGLONG )Src64_2[ 1 ] ;

	Temp3 = ( ULONGLONG )Src64_1[ 1 ] * ( ULONGLONG )Src64_2[ 0 ] ;
	Temp4 = ( ULONGLONG )Src64_1[ 1 ] * ( ULONGLONG )Src64_2[ 1 ] ;

	Dest1 = 0;
	Dest2 = 0;
	Dest3 = 0;
	Dest4 = 0;

#ifdef DX_GCC_COMPILE
	if( Temp4 > 0xffffffffULL )
#else
	if( Temp4 > 0xffffffff )
#endif
	{
		Dest4 += Temp4 >> 32;
		Temp4 &= 0xffffffff;
	}
#ifdef DX_GCC_COMPILE
	if (Temp3 > 0xffffffffULL)
#else
	if (Temp3 > 0xffffffff)
#endif
	{
		Dest3 += Temp3 >> 32;
		Temp3 &= 0xffffffff;
	}
#ifdef DX_GCC_COMPILE
	if (Temp2 > 0xffffffffULL)
#else
	if (Temp2 > 0xffffffff)
#endif
	{
		Dest3 += Temp2 >> 32;
		Temp2 &= 0xffffffff;
	}
#ifdef DX_GCC_COMPILE
	if (Temp1 > 0xffffffffULL)
#else
	if (Temp1 > 0xffffffff)
#endif
	{
		Dest2 += Temp1 >> 32;
		Temp1 &= 0xffffffff;
	}

	Dest1 += Temp1;
	Dest2 += Temp2 + Temp3;
	Dest3 += Temp4;

#ifdef DX_GCC_COMPILE
	if (Dest1 > 0xffffffffULL)
#else
	if (Dest1 > 0xffffffff)
#endif
	{
		Dest2 += Dest1 >> 32;
		Dest1 &= 0xffffffff;
	}
#ifdef DX_GCC_COMPILE
	if (Dest2 > 0xffffffffULL)
#else
	if (Dest2 > 0xffffffff)
#endif
	{
		Dest3 += Dest2 >> 32;
		Dest2 &= 0xffffffff;
	}
#ifdef DX_GCC_COMPILE
	if (Dest3 > 0xffffffffULL)
#else
	if (Dest3 > 0xffffffff)
#endif
	{
		Dest4 += Dest3 >> 32;
		Dest3 &= 0xffffffff;
	}

	Dest128[ 0 ] = ( DWORD )Dest1 ;
	Dest128[ 1 ] = ( DWORD )Dest2 ;
	Dest128[ 2 ] = ( DWORD )Dest3 ;
	Dest128[ 3 ] = ( DWORD )Dest4 ;
#endif
}

// Src128 の最上位ビットが立っていると正常に処理できない
extern void _DIV128_1( DWORD *Src128, DWORD *Div64, DWORD *Dest64 )
{
#ifndef DX_NON_INLINE_ASM
	DWORD dat128_1[4], dat128_2[4], dat128_src[4], dat128_dst[4];
	DWORD dat128_1L,   dat128_2L,   dat128_srcL,   dat128_dstL;
	dat128_1L = (DWORD)dat128_1;
	dat128_2L = (DWORD)dat128_2;
	dat128_srcL = (DWORD)dat128_src;
	dat128_dstL = (DWORD)dat128_dst;
	__asm
	{
		pushf
		cld
		mov esi, [Src128]
		mov edi, dat128_1L
		mov ecx, 4
		rep movsd
		mov esi, [Div64]
		mov edi, dat128_srcL
		mov ecx, 2
		rep movsd
		xor eax, eax
		mov ecx, 2
		rep stosd
		mov edi, dat128_dstL
		mov ecx, 4
		rep stosd

LOOP2:
		mov ecx, 4
		mov esi, dat128_srcL
		mov edi, dat128_2L
		rep movsd
		mov edx, 0
LOOP1:
		std
		mov ecx, 4
		mov esi, dat128_1L
		add esi, 12
		mov edi, dat128_2L
		add edi, 12
		rep cmpsd
		cld
		jb LABEL1
		inc edx
		mov esi, dat128_2L
		mov ecx, 1
LSHFT1:
		shl dword ptr [esi   ], 1
		rcl dword ptr [esi+4 ], 1
		rcl dword ptr [esi+8 ], 1
		rcl dword ptr [esi+12], 1
		dec ecx
		jnz LSHFT1
		jmp LOOP1

LABEL1:
		cmp edx, 0
		je LOOPEND

		mov eax, 1
		dec edx
		mov ecx, edx

		cmp ecx, 32
		jae LSHFT2_1

		shl eax, cl
		or [dat128_dst], eax
		jmp LABEL2

LSHFT2_1:
		cmp ecx, 64
		jae LSHFT2_2

		sub ecx, 32
		shl eax, cl
		or [dat128_dst+4], eax
		jmp LABEL2

LSHFT2_2:
		cmp ecx, 96
		jae LSHFT2_3

		sub ecx, 64
		shl eax, cl
		or [dat128_dst+8], eax
		jmp LABEL2

LSHFT2_3:
		sub ecx, 96
		shl eax, cl
		or [dat128_dst+12], eax

LABEL2:
		shr dword ptr [dat128_2+12], 1
		rcr dword ptr [dat128_2+8 ], 1
		rcr dword ptr [dat128_2+4 ], 1
		rcr dword ptr [dat128_2   ], 1

		mov eax, [dat128_2]
		sub [dat128_1], eax
		mov eax, [dat128_2+4]
		sbb [dat128_1+4], eax
		mov eax, [dat128_2+8]
		sbb [dat128_1+8], eax
		mov eax, [dat128_2+12]
		sbb [dat128_1+12], eax

		jmp LOOP2

LOOPEND:
		mov esi, dat128_dstL
		mov edi, [Dest64]
		mov ecx, 2
		rep movsd
		popf
	}
#else
	bool cflag ;
	DWORD edx, ecx ;
	DWORD cf1, cf2, cf3 ;
	DWORD dat_1_0, dat_1_1, dat_1_2, dat_1_3 ;
	DWORD dat_2_0, dat_2_1, dat_2_2, dat_2_3 ;
	DWORD dat_src_0, dat_src_1, dat_src_2, dat_src_3 ;
	DWORD dat_dst_0, dat_dst_1, dat_dst_2, dat_dst_3 ;

	dat_1_0 = Src128[ 0 ] ;
	dat_1_1 = Src128[ 1 ] ;
	dat_1_2 = Src128[ 2 ] ;
	dat_1_3 = Src128[ 3 ] ;

	dat_src_0 = Div64[ 0 ] ;
	dat_src_1 = Div64[ 1 ] ;
	dat_src_2 = 0;
	dat_src_3 = 0;

	dat_dst_0 = 0;
	dat_dst_1 = 0;
	dat_dst_2 = 0;
	dat_dst_3 = 0;

	for (; ; )
	{
		dat_2_0 = dat_src_0;
		dat_2_1 = dat_src_1;
		dat_2_2 = dat_src_2;
		dat_2_3 = dat_src_3;
		edx = 0;

		for (; ; )
		{
			if (dat_1_3 == dat_2_3)
			{
				if (dat_1_2 == dat_2_2)
				{
					if (dat_1_1 == dat_2_1)
					{
						cflag = dat_1_0 < dat_2_0;
					}
					else
					{
						cflag = dat_1_1 < dat_2_1;
					}
				}
				else
				{
					cflag = dat_1_2 < dat_2_2;
				}
			}
			else
			{
				cflag = dat_1_3 < dat_2_3;
			}
			if (cflag == true) break;
			edx++;

			cf1 = dat_2_0 & 0x80000000;
			cf2 = dat_2_1 & 0x80000000;
			cf3 = dat_2_2 & 0x80000000;
			dat_2_0 <<= 1;
			dat_2_1 <<= 1;
			dat_2_2 <<= 1;
			dat_2_3 <<= 1;
			dat_2_1 |= cf1 >> 31;
			dat_2_2 |= cf2 >> 31;
			dat_2_3 |= cf3 >> 31;
		}

		if (edx == 0) break;

		edx--;

		ecx = edx;
		if (ecx < 32)
		{
			dat_dst_0 |= (DWORD)1 << (int)(ecx - 0);
		}
		else if (ecx < 64)
		{
			dat_dst_1 |= (DWORD)1 << (int)(ecx - 32);
		}
		else if (ecx < 96)
		{
			dat_dst_2 |= (DWORD)1 << (int)(ecx - 64);
		}
		else
		{
			dat_dst_3 |= (DWORD)1 << (int)(ecx - 96);
		}

		cf1 = dat_2_1 & 1;
		cf2 = dat_2_2 & 1;
		cf3 = dat_2_3 & 1;
		dat_2_0 >>= 1;
		dat_2_1 >>= 1;
		dat_2_2 >>= 1;
		dat_2_3 >>= 1;
		dat_2_0 |= cf1 << 31;
		dat_2_1 |= cf2 << 31;
		dat_2_2 |= cf3 << 31;

		cf1 = (DWORD)(dat_1_0 < dat_2_0 ? 1 : 0);
		dat_1_0 -= dat_2_0;
		cf2 = (DWORD)((cf1 != 0 && dat_1_1 == 0) || dat_1_1 - cf1 < dat_2_1 ? 1 : 0);
		dat_1_1 -= dat_2_1 + cf1;
		cf3 = (DWORD)((cf2 != 0 && dat_1_2 == 0) || dat_1_2 - cf1 < dat_2_2 ? 1 : 0);
		dat_1_2 -= dat_2_2 + cf2;
		dat_1_3 -= dat_2_3 + cf3;
	}

	Dest64[ 0 ] = dat_dst_0 ;
	Dest64[ 1 ] = dat_dst_1 ;
#endif
}



}
