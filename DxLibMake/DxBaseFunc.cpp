﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		標準関数の互換関数プログラム
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード ------------------------------------------------------------------
#include "DxBaseFunc.h"
#include "DxArchive_.h"
#include "DxLib.h"
#include "DxUseCLib.h"
#include "DxChar.h"
#include <wchar.h>
#include <math.h>


#ifdef DX_NON_INLINE_ASM
  #include <math.h>
#endif

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

// 構造体定義 --------------------------------------------------------------------

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

BASEFUNCSYSTEM g_BaseFuncSystem ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// プログラム --------------------------------------------------------------------

extern void _SET_CHARSET( int CharSet )
{
	g_BaseFuncSystem.UseCharSet = CharSet ;
}

extern int _GET_CHARSET( void )
{
	return g_BaseFuncSystem.UseCharSet ;
}

extern void _SET_CHAR_CHARCODEFORMAT( int CharCodeFormat )
{
	g_BaseFuncSystem.Use_char_CharCodeFormat = CharCodeFormat ;
}

extern int _GET_CHAR_CHARCODEFORMAT( void )
{
	return g_BaseFuncSystem.Use_char_CharCodeFormat ;
}

extern void _SET_WCHAR_T_CHARCODEFORMAT( int CharCodeFormat )
{
	g_BaseFuncSystem.Use_wchar_t_CharCodeFormat = CharCodeFormat ;
}

extern int _GET_WCHAR_T_CHARCODEFORMAT( void )
{
	return g_BaseFuncSystem.Use_wchar_t_CharCodeFormat ;
}

// 文字列処理関数
extern void _STRCPY( char *Dest, const char *Src )
{
	CL_strcpy( CHAR_CHARCODEFORMAT, Dest, Src ) ;
//	int i ;
//	for( i = 0 ; Src[i] != '\0' ; i ++ ) Dest[i] = Src[i] ;
//	Dest[i] = '\0' ;
}

extern void _WCSCPY( wchar_t *Dest, const wchar_t *Src )
{
	CL_strcpy( WCHAR_T_CHARCODEFORMAT, ( char * )Dest, ( const char * )Src ) ;
//	int i ;
//	for( i = 0 ; Src[i] != L'\0' ; i ++ ) Dest[i] = Src[i] ;
//	Dest[i] = L'\0' ;
}

extern void _STRNCPY( char *Dest, const char *Src, int Num )
{
	CL_strncpy( CHAR_CHARCODEFORMAT, Dest, Src, Num ) ;
//	int i ;
//	for( i = 0 ; i < Num && Src[i] != '\0' ; i ++ ) Dest[i] = Src[i] ;
//	Dest[i] = '\0' ;
}

extern void _WCSNCPY( wchar_t *Dest, const wchar_t *Src, int Num )
{
	CL_strncpy( WCHAR_T_CHARCODEFORMAT, ( char * )Dest, ( const char * )Src, Num ) ;
//	int i ;
//	for( i = 0 ; i < Num && Src[i] != L'\0' ; i ++ ) Dest[i] = Src[i] ;
//	Dest[i] = L'\0' ;
}

extern void _STRCAT( char *Dest, const char *Src )
{
	CL_strcat( CHAR_CHARCODEFORMAT, Dest, Src ) ;
//	int i ;
//	for( i = 0 ; Dest[i] != '\0' ; i ++ ){}
//	_STRCPY( &Dest[i], Src ) ;
}

extern void _WCSCAT( wchar_t *Dest, const wchar_t *Src )
{
	CL_strcat( WCHAR_T_CHARCODEFORMAT, ( char * )Dest, ( const char * )Src ) ;
//	int i ;
//	for( i = 0 ; Dest[i] != L'\0' ; i ++ ){}
//	_WCSCPY( &Dest[i], Src ) ;
}

extern const char *_STRSTR( const char *Str1, const char *Str2 )
{
	return CL_strstr( CHAR_CHARCODEFORMAT, Str1, Str2 ) ;
//	int i, j ;
//	for( i = 0 ; Str1[i] != '\0' ; i ++ )
//	{
//		for( j = 0 ; Str2[j] != '\0' && Str1[i+j] != '\0' && Str1[i+j] == Str2[j] ; j ++ ){}
//		if( Str2[j] == '\0' ) return (char *)&Str1[i] ;
//		if( Str1[i+j] == '\0' ) return NULL ;
//	}
//	return NULL ;
}

extern const wchar_t *_WCSSTR( const wchar_t *Str1, const wchar_t *Str2 )
{
	return ( const wchar_t * )CL_strstr( WCHAR_T_CHARCODEFORMAT, ( const char * )Str1, ( const char * )Str2 ) ;
//	int i, j ;
//	for( i = 0 ; Str1[i] != L'\0' ; i ++ )
//	{
//		for( j = 0 ; Str2[j] != L'\0' && Str1[i+j] != L'\0' && Str1[i+j] == Str2[j] ; j ++ ){}
//		if( Str2[j] == L'\0' ) return (wchar_t *)&Str1[i] ;
//		if( Str1[i+j] == L'\0' ) return NULL ;
//	}
//	return NULL ;
}

extern int _STRLEN( const char *Str )
{
	return CL_strlen( CHAR_CHARCODEFORMAT, Str ) ;
//	int i ;
//
//	for( i = 0 ; Str[ i ] ; i ++ ){}
//	return i ;
}

extern int _WCSLEN( const wchar_t *Str )
{
	return CL_strlen( WCHAR_T_CHARCODEFORMAT, ( const char * )Str ) ;
//	int i ;
//
//	for( i = 0 ; Str[ i ] ; i ++ ){}
//	return i ;
}

extern const char *_STRCHR( const char *Str, int Char )
{
	return CL_strchr( CHAR_CHARCODEFORMAT, Str, ( DWORD )Char ) ;
//	while( *Str1 != '\0' )
//	{
//		if( CheckMultiByteChar( *Str1, g_BaseFuncSystem.UseCharSet ) == TRUE )
//		{
//			Str1 += 2 ;
//		}
//		else
//		{
//			if( *Str1 == Char ) return (char *)Str1 ;
//			Str1 ++ ;
//		}
//	}
//	return NULL ;
}

extern const wchar_t *_WCSCHR( const wchar_t *Str, wchar_t Char )
{
	return ( const wchar_t * )CL_strchr( WCHAR_T_CHARCODEFORMAT, ( const char * )Str, ( DWORD )Char ) ;
//	while( *Str1 != L'\0' )
//	{
//		if( CheckUTF16H( *Str1 ) == TRUE )
//		{
//			Str1 += 2 ;
//		}
//		else
//		{
//			if( *Str1 == Char ) return (wchar_t *)Str1 ;
//			Str1 ++ ;
//		}
//	}
//	return NULL ;
}

extern const char *_STRRCHR( const char *Str, int Char )
{
	return CL_strrchr( CHAR_CHARCODEFORMAT, Str, ( DWORD )Char ) ;
//	char *lastp;
//
//	lastp = NULL;
//	while( *Str1 != '\0' )
//	{
//		if( CheckMultiByteChar( *Str1, g_BaseFuncSystem.UseCharSet ) == TRUE )
//		{
//			Str1 += 2 ;
//		}
//		else
//		{
//			if( *Str1 == Char )
//			{
//				lastp = (char *)Str1 ;
//			}
//			Str1 ++ ;
//		}
//	}
//	return lastp ;
}

extern const wchar_t *_WCSRCHR( const wchar_t *Str, wchar_t Char )
{
	return ( const wchar_t * )CL_strrchr( WCHAR_T_CHARCODEFORMAT, ( const char * )Str, ( DWORD )Char ) ;
//	wchar_t *lastp;
//
//	lastp = NULL;
//	while( *Str1 != L'\0' )
//	{
//		if( CheckUTF16H( *Str1 ) == TRUE )
//		{
//			Str1 += 2 ;
//		}
//		else
//		{
//			if( *Str1 == Char )
//			{
//				lastp = (wchar_t *)Str1 ;
//			}
//			Str1 ++ ;
//		}
//	}
//	return lastp ;
}

extern int _STRCMP( const char *Str1, const char *Str2 )
{
	return CL_strcmp( CHAR_CHARCODEFORMAT, Str1, Str2 ) ;
//	int i ;
//	for( i = 0 ; Str1[i] != '\0' && Str2[i] != '\0' && Str1[i] == Str2[i] ; i ++ ){}
//	return ( Str1[i] != Str2[i] ) ? 1 : 0 ;
}

extern int _WCSCMP( const wchar_t *Str1, const wchar_t *Str2 )
{
	return CL_strcmp( WCHAR_T_CHARCODEFORMAT, ( const char * )Str1, ( const char * )Str2 ) ;
//	int i ;
//	for( i = 0 ; Str1[i] != L'\0' && Str2[i] != L'\0' && Str1[i] == Str2[i] ; i ++ ){}
//	return ( Str1[i] != Str2[i] ) ? 1 : 0 ;
}

extern int _STRICMP( const char *Str1, const char *Str2 )
{
	return CL_stricmp( CHAR_CHARCODEFORMAT, Str1, Str2 ) ;
//	int i ;
//	char c1, c2;
//	for( i = 0 ; Str1[i] != '\0' && Str2[i] != '\0'; i ++ )
//	{
//		c1 = Str1[i] >= 'a' && Str1[i] <= 'z' ? Str1[i] - 'a' + 'A' : Str1[i] ;
//		c2 = Str2[i] >= 'a' && Str2[i] <= 'z' ? Str2[i] - 'a' + 'A' : Str2[i] ;
//		if( c1 != c2 ) break;
//	}
//	return ( Str1[i] != Str2[i] ) ? 1 : 0 ;
}

extern int _WCSICMP( const wchar_t *Str1, const wchar_t *Str2 )
{
	return CL_stricmp( WCHAR_T_CHARCODEFORMAT, ( const char * )Str1, ( const char * )Str2 ) ;
}

extern char *_STRUPR( char *Str )
{
	return CL_strupr( CHAR_CHARCODEFORMAT, Str ) ;
//	char *OrigStr = Str ;
//
//	while( *Str != '\0' )
//	{
//		if( CheckMultiByteChar( *Str, g_BaseFuncSystem.UseCharSet ) == TRUE )
//		{
//			Str += 2 ;
//		}
//		else
//		{
//			if( *Str >= 'a' && *Str <= 'z' ) *Str = *Str - 'a' + 'A' ;
//			Str ++ ;
//		}
//	}
//
//	return OrigStr ;
}

extern wchar_t *_WCSUPR( wchar_t *Str )
{
	return ( wchar_t * )CL_strupr( WCHAR_T_CHARCODEFORMAT, ( char * )Str ) ;
//	while( *Str != L'\0' )
//	{
//		if( CheckUTF16H( *Str ) == TRUE )
//		{
//			Str += 2 ;
//		}
//		else
//		{
//			if( *Str >= L'a' && *Str <= L'z' ) *Str = ( wchar_t )( *Str - L'a' + L'A' ) ;
//			Str ++ ;
//		}
//	}
//
//	return Str ;
}

extern int _STRNCMP( const char *Str1, const char *Str2, int Size )
{
	return CL_strncmp( CHAR_CHARCODEFORMAT, Str1, Str2, Size ) ;
//	int i ;
//	for( i = 0 ; i < Size && Str1[i] == Str2[i] ; i ++ ){}
//	return i != Size ? 1 : 0 ;
}

extern int _WCSNCMP( const wchar_t *Str1, const wchar_t *Str2, int Size )
{
	return CL_strncmp( WCHAR_T_CHARCODEFORMAT, ( const char * )Str1, ( const char * )Str2, Size ) ;
//	int i ;
//	for( i = 0 ; i < Size && Str1[i] == Str2[i] ; i ++ ){}
//	return i != Size ? 1 : 0 ;
}

extern int _VSPRINTF( char *Buffer, const char *FormatString, va_list Arg )
{
	return CL_vsprintf( CHAR_CHARCODEFORMAT, FALSE, CHAR_CHARCODEFORMAT, WCHAR_T_CHARCODEFORMAT, Buffer, FormatString, Arg ) ;
//	return vsprintf( Buffer, FormatString, Arg ) ;
}

extern int _VSWPRINTF( wchar_t *Buffer, const wchar_t *FormatString, va_list Arg )
{
	return CL_vsprintf( WCHAR_T_CHARCODEFORMAT, TRUE, CHAR_CHARCODEFORMAT, WCHAR_T_CHARCODEFORMAT, ( char * )Buffer, ( const char * )FormatString, Arg ) ;
//#ifdef __ANDROID
//	return vswprintf( ( wchar_t * )Buffer, 4096, ( wchar_t * )FormatWString, Arg ) ;
//#else
//	return vswprintf( ( wchar_t * )Buffer, ( wchar_t * )FormatWString, Arg ) ;
//#endif
}

extern int _SPRINTF( char *Buffer, const char *FormatString, ... )
{
	va_list VaList ;
	int Result ;

	va_start( VaList, FormatString ) ;
	Result = CL_vsprintf( CHAR_CHARCODEFORMAT, FALSE, CHAR_CHARCODEFORMAT, WCHAR_T_CHARCODEFORMAT, Buffer, FormatString, VaList ) ;
	va_end( VaList ) ;
	
	return Result ;
//	va_list VaList ;
//
//	va_start( VaList, FormatString ) ;
//	vsprintf( Buffer, FormatString, VaList ) ;
//	va_end( VaList ) ;
//	
//	return 0 ;
}

extern int _SWPRINTF( wchar_t *Buffer, const wchar_t *FormatString, ... )
{
	va_list VaList ;
	int Result ;

	va_start( VaList, FormatString ) ;
	Result = CL_vsprintf( WCHAR_T_CHARCODEFORMAT, TRUE, CHAR_CHARCODEFORMAT, WCHAR_T_CHARCODEFORMAT, ( char * )Buffer, ( const char * )FormatString, VaList ) ;
	va_end( VaList ) ;
	
	return Result ;
//	va_list VaList ;
//
//	va_start( VaList, FormatWString ) ;
//	_VSWPRINTF( Buffer, FormatWString, VaList ) ;
//	va_end( VaList ) ;
//	
//	return 0 ;
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
	return ( int )res ;
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

extern void _SINCOSD( double Angle, double *DestSin, double *DestCos )
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

extern double _POWD( double x, double y )
{
	return pow( x, y ) ;
}

extern float _ATAN2( float y, float x )
{
	return ( float )atan2( ( double )y, ( double )x ) ;
}

extern double _ATAN2D( double y, double x )
{
	return atan2( y, x ) ;
}

extern float _EXPF( float x )
{
	return ( float )exp( ( double )x ) ;
}

extern double _EXPF( double x )
{
	return exp( x ) ;
}

extern float _ASIN( float Real )
{
	return ( float )asin( ( double )Real ) ;
}

extern double _ASIND( double Real )
{
	return asin( Real ) ;
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

extern double _ACOSD( double Real )
{
#ifdef __BCC
	if( Real < -1.0 )
	{
		Real = -1.0 ;
	}
	else
	if( Real > 1.0 )
	{
		Real = 1.0 ;
	}
#endif

	return acos( Real ) ;
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

extern double _DABS( double Real )
{
	return Real < 0.0 ? -Real : Real ;
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
	return ( int )Result ;
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
	return ( int )Result ;
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

extern double _SQRTD( double Real )
{
	return sqrt( Real ) ;
}

extern int _ATOI( const char *String )
{
	return CL_atoi( CHAR_CHARCODEFORMAT, String ) ;
}

extern int _ATOIW( const wchar_t *String )
{
	return CL_atoi( WCHAR_T_CHARCODEFORMAT, ( const char * )String ) ;
}

extern double _ATOF( const char *String )
{
	return CL_atof( CHAR_CHARCODEFORMAT, String ) ;
}

extern double _ATOFW( const wchar_t *String )
{
	return CL_atof( WCHAR_T_CHARCODEFORMAT, ( const char * )String ) ;
}

extern char *_ITOA( int Value, char *Buffer, int Radix )
{
	return CL_itoa( CHAR_CHARCODEFORMAT, Value, Buffer, Radix ) ;
}

extern wchar_t *_ITOAW( int Value, wchar_t *Buffer, int Radix )
{
	return ( wchar_t * )CL_itoa( WCHAR_T_CHARCODEFORMAT, Value, ( char * )Buffer, Radix ) ;
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













// 文字コード関係

// 文字列の文字コード形式を別の文字コード形式に変換する
extern int NS_ConvertStringCharCodeFormat( int SrcCharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */, const void *SrcString, int DestCharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */, void *DestStringBuffer )
{
	return ConvString( ( const char * )SrcString, SrcCharCodeFormat, ( char * )DestStringBuffer, DestCharCodeFormat ) ;
}

// 文字列の引数の文字コード形式を設定する( 文字列描画系関数とその他一部関数を除く )( UNICODE版では無効 )
extern int NS_SetUseCharCodeFormat( int CharCodeFormat /* DX_CHARCODEFORMAT_SHIFTJIS 等 */ )
{
	_SET_CHAR_CHARCODEFORMAT( CharCodeFormat ) ;

	return 0 ;
}






// 文字列関係

// strcpy と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strcpyDx(   TCHAR *Dest, const TCHAR *Src )
{
#ifdef UNICODE
	CL_strcpy( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src ) ;
#else // UNICODE
	CL_strcpy( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src ) ;
#endif // UNICODE
}

// 位置指定付き strcpy、Pos はコピー開始位置　( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strpcpyDx(    TCHAR *Dest, const TCHAR *Src, int Pos )
{
#ifdef UNICODE
	CL_strpcpy( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Pos ) ;
#else // UNICODE
	CL_strpcpy( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Pos ) ;
#endif // UNICODE
}

// 位置指定付き strcpy、Pos はコピー開始位置( 全角文字も 1 扱い )　( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strpcpy2Dx(   TCHAR *Dest, const TCHAR *Src, int Pos )
{
#ifdef UNICODE
	CL_strpcpy2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Pos ) ;
#else // UNICODE
	CL_strpcpy2( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Pos ) ;
#endif // UNICODE
}

// strncpy と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strncpyDx(  TCHAR *Dest, const TCHAR *Src, int Num )
{
#ifdef UNICODE
	CL_strncpy( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Num ) ;
#else // UNICODE
	CL_strncpy( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Num ) ;
#endif // UNICODE
}

// strncpy の Num が文字数( 全角文字も 1 扱い )になったもの、終端に必ずヌル文字が代入される( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strncpy2Dx( TCHAR *Dest, const TCHAR *Src, int Num )
{
#ifdef UNICODE
	CL_strncpy2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Num ) ;
#else // UNICODE
	CL_strncpy2( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Num ) ;
#endif // UNICODE
}

// strncpy の文字列の終端からの文字数指定版( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strrncpyDx(   TCHAR *Dest, const TCHAR *Src, int Num )
{
#ifdef UNICODE
	CL_strrncpy( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Num ) ;
#else // UNICODE
	CL_strrncpy( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Num ) ;
#endif // UNICODE
}

// strncpy の文字列の終端からの文字数( 全角文字も 1 扱い )指定版、終端に必ずヌル文字が代入される( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strrncpy2Dx(  TCHAR *Dest, const TCHAR *Src, int Num )
{
#ifdef UNICODE
	CL_strrncpy2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Num ) ;
#else // UNICODE
	CL_strrncpy2( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Num ) ;
#endif // UNICODE
}

// strncpy のコピー開始位置指定版、Pos はコピー開始位置、Num は文字数( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strpncpyDx(   TCHAR *Dest, const TCHAR *Src, int Pos, int Num )
{
#ifdef UNICODE
	CL_strpncpy( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Pos, Num ) ;
#else // UNICODE
	CL_strpncpy( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Pos, Num ) ;
#endif // UNICODE
}

// strncpy のコピー開始位置指定版、Pos はコピー開始位置( 全角文字も 1 扱い )、Num は文字数( 全角文字も 1 扱い )、終端に必ずヌル文字が代入される( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strpncpy2Dx(  TCHAR *Dest, const TCHAR *Src, int Pos, int Num )
{
#ifdef UNICODE
	CL_strpncpy2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src, Pos, Num ) ;
#else // UNICODE
	CL_strpncpy2( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src, Pos, Num ) ;
#endif // UNICODE
}

// strcat と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern void NS_strcatDx(   TCHAR *Dest, const TCHAR *Src )
{
#ifdef UNICODE
	CL_strcat( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Dest, ( const char * )Src ) ;
#else // UNICODE
	CL_strcat( g_BaseFuncSystem.Use_char_CharCodeFormat, Dest, Src ) ;
#endif // UNICODE
}

// strlen と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strlenDx(   const TCHAR *Str )
{
#ifdef UNICODE
	return CL_strlen( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str ) ;
#else // UNICODE
	return CL_strlen( g_BaseFuncSystem.Use_char_CharCodeFormat, Str ) ;
#endif // UNICODE
}

// strlen の戻り値が文字数( 全角文字も 1 扱い )になったもの( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strlen2Dx(  const TCHAR *Str )
{
#ifdef UNICODE
	return CL_strlen2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str ) ;
#else // UNICODE
	return CL_strlen2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str ) ;
#endif // UNICODE
}

// strcmp と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strcmpDx(   const TCHAR *Str1, const TCHAR *Str2 )
{
#ifdef UNICODE
	return CL_strcmp( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2 ) ;
#else // UNICODE
	return CL_strcmp( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2 ) ;
#endif // UNICODE
}

// stricmp と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_stricmpDx(  const TCHAR *Str1, const TCHAR *Str2 )
{
#ifdef UNICODE
	return CL_stricmp( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2 ) ;
#else // UNICODE
	return CL_stricmp( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2 ) ;
#endif // UNICODE
}

// strncmp と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strncmpDx(  const TCHAR *Str1, const TCHAR *Str2, int Num )
{
#ifdef UNICODE
	return CL_strncmp( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2, Num ) ;
#else // UNICODE
	return CL_strncmp( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2, Num ) ;
#endif // UNICODE
}

// strncmp2 の Num が文字数( 全角文字も 1 扱い )になったもの( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strncmp2Dx( const TCHAR *Str1, const TCHAR *Str2, int Num )
{
#ifdef UNICODE
	return CL_strncmp2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2, Num ) ;
#else // UNICODE
	return CL_strncmp2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2, Num ) ;
#endif // UNICODE
}

// strncmp の比較開始位置指定版、Pos が比較開始位置、Num が文字数( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strpncmpDx(   const TCHAR *Str1, const TCHAR *Str2, int Pos, int Num )
{
#ifdef UNICODE
	return CL_strpncmp( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2, Pos, Num ) ;
#else // UNICODE
	return CL_strpncmp( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2, Pos, Num ) ;
#endif // UNICODE
}

// strncmp の比較開始位置指定版、Pos が比較開始位置( 全角文字も 1 扱い )、Num が文字数( 全角文字も 1 扱い )( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strpncmp2Dx(  const TCHAR *Str1, const TCHAR *Str2, int Pos, int Num )
{
#ifdef UNICODE
	return CL_strpncmp2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2, Pos, Num ) ;
#else // UNICODE
	return CL_strpncmp2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2, Pos, Num ) ;
#endif // UNICODE
}

// 文字列の指定の位置の文字コードを取得する、Pos は取得する位置、CharNums は文字数を代入する変数のアドレス、戻り値は文字コード( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern DWORD NS_strgetchrDx(  const TCHAR *Str, int Pos, int *CharNums )
{
#ifdef UNICODE
	return CL_strgetchr( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, Pos, CharNums ) ;
#else // UNICODE
	return CL_strgetchr( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, Pos, CharNums ) ;
#endif // UNICODE
}

// 文字列の指定の位置の文字コードを取得する、Pos は取得する位置( 全角文字も 1 扱い )、CharNums は文字数を代入する変数のアドレス、戻り値は文字コード( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern DWORD NS_strgetchr2Dx( const TCHAR *Str, int Pos, int *CharNums )
{
#ifdef UNICODE
	return CL_strgetchr2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, Pos, CharNums ) ;
#else // UNICODE
	return CL_strgetchr2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, Pos, CharNums ) ;
#endif // UNICODE
}

// 文字列の指定の位置に文字コードを書き込む、Pos は書き込む位置、CharCode は文字コード、戻り値は書き込んだ文字数( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strputchrDx(  TCHAR *Str, int Pos, DWORD CharCode )
{
#ifdef UNICODE
	return CL_strputchr( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Str, Pos, CharCode ) ;
#else // UNICODE
	return CL_strputchr( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, Pos, CharCode ) ;
#endif // UNICODE
}

// 文字列の指定の位置に文字コードを書き込む、Pos は書き込む位置( 全角文字も 1 扱い )、CharCode は文字コード、戻り値は書き込んだ文字数( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strputchr2Dx( TCHAR *Str, int Pos, DWORD CharCode )
{
#ifdef UNICODE
	return CL_strputchr2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Str, Pos, CharCode ) ;
#else // UNICODE
	return CL_strputchr2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, Pos, CharCode ) ;
#endif // UNICODE
}

// 文字列の指定の位置のアドレスを取得する、Pos は取得する位置　( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern const TCHAR * NS_strposDx(     const TCHAR *Str, int Pos )
{
#ifdef UNICODE
	return ( const TCHAR * )CL_strpos( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, Pos ) ;
#else // UNICODE
	return CL_strpos( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, Pos ) ;
#endif // UNICODE
}

// 文字列の指定の位置のアドレスを取得する、Pos は取得する位置( 全角文字も 1 扱い )　( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern const TCHAR * NS_strpos2Dx(    const TCHAR *Str, int Pos )
{
#ifdef UNICODE
	return ( const TCHAR * )CL_strpos2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, Pos ) ;
#else // UNICODE
	return CL_strpos2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, Pos ) ;
#endif // UNICODE
}

// strstr と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern const TCHAR * NS_strstrDx(   const TCHAR *Str1, const TCHAR *Str2 )
{
#ifdef UNICODE
	return ( wchar_t * )CL_strstr( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2 ) ;
#else // UNICODE
	return CL_strstr( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2 ) ;
#endif // UNICODE
}

// strstr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strstr2Dx(    const TCHAR *Str1, const TCHAR *Str2 )
{
#ifdef UNICODE
	return CL_strstr2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2 ) ;
#else // UNICODE
	return CL_strstr2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2 ) ;
#endif // UNICODE
}

// strrstr と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern const TCHAR * NS_strrstrDx( const TCHAR *Str1, const TCHAR *Str2 )
{
#ifdef UNICODE
	return ( wchar_t * )CL_strrstr( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2 ) ;
#else // UNICODE
	return CL_strrstr( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2 ) ;
#endif // UNICODE
}

// strrstr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strrstr2Dx(   const TCHAR *Str1, const TCHAR *Str2 )
{
#ifdef UNICODE
	return CL_strrstr2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str1, ( const char * )Str2 ) ;
#else // UNICODE
	return CL_strrstr2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str1, Str2 ) ;
#endif // UNICODE
}

// strchr と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern const TCHAR * NS_strchrDx(   const TCHAR *Str, DWORD CharCode )
{
#ifdef UNICODE
	return ( const TCHAR * )CL_strchr( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, CharCode ) ;
#else // UNICODE
	return CL_strchr( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, CharCode ) ;
#endif // UNICODE
}

// strchr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strchr2Dx( const TCHAR *Str, DWORD CharCode )
{
#ifdef UNICODE
	return CL_strchr2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, CharCode ) ;
#else // UNICODE
	return CL_strchr2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, CharCode ) ;
#endif // UNICODE
}

// strrchr と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern const TCHAR * NS_strrchrDx(  const TCHAR *Str, DWORD CharCode )
{
#ifdef UNICODE
	return ( const TCHAR * )CL_strrchr( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, CharCode ) ;
#else // UNICODE
	return CL_strrchr( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, CharCode ) ;
#endif // UNICODE
}

// strrchr の戻り値が文字列先頭からの文字数( 全角文字も 1 扱い ) になったもの( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_strrchr2Dx(   const TCHAR *Str, DWORD CharCode )
{
#ifdef UNICODE
	return CL_strrchr2( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str, CharCode ) ;
#else // UNICODE
	return CL_strrchr2( g_BaseFuncSystem.Use_char_CharCodeFormat, Str, CharCode ) ;
#endif // UNICODE
}

// strupr と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern TCHAR *NS_struprDx(   TCHAR *Str )
{
#ifdef UNICODE
	return ( wchar_t * )CL_strupr( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Str ) ;
#else // UNICODE
	return CL_strupr( g_BaseFuncSystem.Use_char_CharCodeFormat, Str ) ;
#endif // UNICODE
}

// vsprintf と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_vsprintfDx( TCHAR *Buffer, const TCHAR *FormatString, va_list Arg )
{
#ifdef UNICODE
	return CL_vsprintf( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, TRUE,  g_BaseFuncSystem.Use_char_CharCodeFormat, g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( char * )Buffer, ( const char * )FormatString, Arg ) ;
#else // UNICODE
	return CL_vsprintf( g_BaseFuncSystem.Use_char_CharCodeFormat,    FALSE, g_BaseFuncSystem.Use_char_CharCodeFormat, g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, Buffer, FormatString, Arg ) ;
#endif // UNICODE
}

// sprintf と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_sprintfDx(  TCHAR *Buffer, const TCHAR *FormatString, ... )
{
	int Result ;
	va_list VaList ;

	va_start( VaList, FormatString ) ;
	Result = NS_vsprintfDx( Buffer, FormatString, VaList ) ;
	va_end( VaList ) ;

	return Result ;
}

// itoa と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern TCHAR *NS_itoaDx(     int Value, TCHAR *Buffer, int Radix )
{
#ifdef UNICODE
	return ( TCHAR * )CL_itoa( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, Value, ( char * )Buffer, Radix ) ;
#else // UNICODE
	return CL_itoa( g_BaseFuncSystem.Use_char_CharCodeFormat, Value, Buffer, Radix ) ;
#endif // UNICODE
}

// atoi と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_atoiDx(     const TCHAR *Str )
{
#ifdef UNICODE
	return CL_atoi( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str ) ;
#else // UNICODE
	return CL_atoi( g_BaseFuncSystem.Use_char_CharCodeFormat, Str ) ;
#endif // UNICODE
}

// atof と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern double NS_atofDx(     const TCHAR *Str )
{
#ifdef UNICODE
	return CL_atof( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )Str ) ;
#else // UNICODE
	return CL_atof( g_BaseFuncSystem.Use_char_CharCodeFormat, Str ) ;
#endif // UNICODE
}

// vsscanf と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_vsscanfDx(  const TCHAR *String, const TCHAR *FormatString, va_list Arg )
{
#ifdef UNICODE
	return CL_vsscanf( g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, TRUE,  g_BaseFuncSystem.Use_char_CharCodeFormat, g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, ( const char * )String, ( const char * )FormatString, Arg ) ;
#else // UNICODE
	return CL_vsscanf( g_BaseFuncSystem.Use_char_CharCodeFormat,    FALSE, g_BaseFuncSystem.Use_char_CharCodeFormat, g_BaseFuncSystem.Use_wchar_t_CharCodeFormat, String, FormatString, Arg ) ;
#endif // UNICODE
}

// sscanf と同等の機能( マルチバイト文字列版では文字コード形式として SetUseCharCodeFormat で設定した形式が使用されます )
extern int NS_sscanfDx(   const TCHAR *String, const TCHAR *FormatString, ... )
{
	int Result ;
	va_list VaList ;

	va_start( VaList, FormatString ) ;
	Result = NS_vsscanfDx( String, FormatString, VaList ) ;
	va_end( VaList ) ;

	return Result ;
}











#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE
