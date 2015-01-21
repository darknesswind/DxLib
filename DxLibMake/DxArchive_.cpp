﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		アーカイブ制御プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------


// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// インクルード-------------------------------------------------------------------
#include "DxArchive_.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxFile.h"
#include "DxFont.h"
#include "DxLog.h"
#include "DxSystem.h"
#include <stdio.h>

#ifdef __WINDOWS__
#include "Windows/DxWindow.h"
#endif // __WINDOWS__

namespace DxLib
{

#ifndef DX_NON_DXA

// 宏定义 --------------------------------------------------------------------

#define DXARCD						DX_ArchiveDirData
//#define CHECKMULTIBYTECHAR(CP)		(( (unsigned char)*(CP) >= 0x81 && (unsigned char)*(CP) <= 0x9F ) || ( (unsigned char)*(CP) >= 0xE0 && (unsigned char)*(CP) <= 0xFC ))	// TRUE:２バイト文字  FALSE:１バイト文字
#define CHARUP(C)					((C) >= _T( 'a' ) && (C) <= _T( 'z' ) ? (C) - _T( 'a' ) + _T( 'A' ) : (C))

#define DXARC_ID_AND_VERSION_SIZE	(sizeof( WORD ) * 2)

#define DXARC_HEAD_VER3_SIZE		(24)
#define DXARC_HEAD_VER4_SIZE		(28)
#define DXARC_HEAD_VER6_SIZE		(48)

#define DXARC_FILEHEAD_VER1_SIZE	(40)			// Ver0x0001 の DXARC_FILEHEAD 構造体のサイズ
#define DXARC_FILEHEAD_VER2_SIZE	(44)			// Ver0x0002 の DXARC_FILEHEAD 構造体のサイズ
#define DXARC_FILEHEAD_VER6_SIZE	(64)			// Ver0x0006 の DXARC_FILEHEAD 構造体のサイズ

#if defined( DX_GCC_COMPILE ) || defined( __ANDROID )
#define NONE_PAL		(0xffffffffffffffffULL)
#else
#define NONE_PAL		(0xffffffffffffffff)
#endif

// 结构体定义 --------------------------------------------------------------------

// DXA_DIR_FindFirst 等の処理で使用する構造体
struct DXA_DIR_FINDDATA
{
	int							UseArchiveFlag;					// アーカイブファイルを使用しているかフラグ
	int							UseArchiveIndex;				// アーカイブを使用している場合、使用しているアーカイブファイルデータのインデックス
	DWORD_PTR					FindHandle;						// ファイル検索用ハンドル
} ;

// DXA_FindFirst 等の処理で使用する構造体
struct DXA_FINDDATA
{
	DXARC						*Container;						// 検索対象のＤＸＡファイル
	char						SearchStr[MAX_PATH];			// 検索文字列
	union
	{
		DXARC_DIRECTORY			*Directory;						// 検索対象のディレクトリ
		DXARC_DIRECTORY_VER5	*DirectoryV5;					// 検索対象のディレクトリ(Ver5以前用)
	};
	DWORD						ObjectCount;					// 次に渡すディレクトリ内オブジェクトのインデックス
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// アーカイブをディレクトリに見立てる為のデータ
DXARC_DIR DX_ArchiveDirData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

static DXARC_FILEHEAD_VER5 *DXA_GetFileInfoV5(		DXARC *DXA, const char *FilePath ) ;													// ファイルの情報を得る
static DXARC_FILEHEAD      *DXA_GetFileInfo(		DXARC *DXA, const char *FilePath ) ;													// ファイルの情報を得る
static int		DXA_ConvSearchData(					DXARC *DXA, DXARC_SEARCHDATA *Dest, const char *Src, int *Length ) ;					// 文字列を検索用のデータに変換( ヌル文字か \ があったら終了 )
static int		DXA_ChangeCurrentDirectoryFast(		DXARC *DXA, DXARC_SEARCHDATA *SearchData ) ;											// アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
static int		DXA_ChangeCurrentDirectoryBase(		DXARC *DXA, const char *DirectoryPath, bool ErrorIsDirectoryReset, DXARC_SEARCHDATA *LastSearchData = NULL ) ;		// アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
//static int	DXA_Decode(							void *Src, void *Dest ) ;																// データを解凍する( 戻り値:解凍後のデータサイズ )
static void		DXA_KeyCreate(						const char *Source, unsigned char *Key ) ;												// 鍵文字列を作成
static void		DXA_KeyConv(						void *Data, LONGLONG  SizeLL, LONGLONG  PositionLL,  unsigned char *Key ) ;				// 鍵文字列を使用して Xor 演算( Key は必ず DXA_KEYSTR_LENGTH の長さがなければならない )
static void		DXA_KeyConvFileRead(				void *Data, ULONGLONG Size,   DWORD_PTR FilePointer, unsigned char *Key, LONGLONG Position = -1 ) ;	// ファイルから読み込んだデータを鍵文字列を使用して Xor 演算する関数( Key は必ず DXA_KEYSTR_LENGTH の長さがなければならない )
static int		DXA_FindProcess(					DXA_FINDDATA *FindData, FILEINFO *FileInfo );											// 条件に適合するオブジェクトを検索する(検索対象は ObjectCount をインデックスとしたところから)(戻り値 -1:エラー 0:成功)

static int		DXA_DIR_OpenArchive(				const TCHAR *FilePath, void *FileImage = NULL, int FileSize = -1, int FileImageCopyFlag = FALSE, int FileImageReadOnly = FALSE, int ArchiveIndex = -1, int OnMemory = FALSE, int ASyncThread = FALSE ) ;	// アーカイブファイルを開く
static int		DXA_DIR_GetArchive(					const TCHAR *FilePath, void *FileImage = NULL ) ;										// 既に開かれているアーカイブのハンドルを取得する( 戻り値: -1=無かった 0以上:ハンドル )
static int		DXA_DIR_CloseArchive(				int ArchiveHandle ) ;																	// アーカイブファイルを閉じる
static void		DXA_DIR_CloseWaitArchive(			void ) ;																				// 使用されるのを待っているアーカイブファイルを全て閉じる
static int		DXA_DIR_ConvertFullPath(			const TCHAR *Src, TCHAR *Dest ) ;														// 全ての英字小文字を大文字にしながら、フルパスに変換する
static int		DXA_DIR_AnalysisFileNameAndDirPath( DXARC *DXA, const char *Src, char *FileName = 0, char *DirPath = 0 ) ;					// ファイル名も一緒になっていると分かっているパス中からファイル名とディレクトリパスを分割する。フルパスである必要は無い、ファイル名だけでも良い、DirPath の終端に ￥ マークは付かない
static int		DXA_DIR_FileNameCmp(				DXARC *DXA, const char *Src, const char *CmpStr );										// CmpStr の条件に Src が適合するかどうかを調べる( 0:適合する  -1:適合しない )
static int		DXA_DIR_OpenTest(					const TCHAR *FilePath, int *ArchiveIndex, char *ArchivePath, char *ArchiveFilePath );	// アーカイブファイルをフォルダに見立ててファイルを開く時の情報を得る( -1:アーカイブとしては存在しなかった  0:存在した )

static int		DXA_DirectoryKeyConv(				DXARC *DXA, DXARC_DIRECTORY *Dir ) ;													// 指定のディレクトリデータの暗号化を解除する( 丸ごとメモリに読み込んだ場合用 )
static int		DXA_DirectoryKeyConvV5(				DXARC *DXA, DXARC_DIRECTORY_VER5 *Dir ) ;												// 指定のディレクトリデータの暗号化を解除する( 丸ごとメモリに読み込んだ場合用 )

// プログラム --------------------------------------------------------------------

// ファイルの情報を得る
static DXARC_FILEHEAD_VER5 *DXA_GetFileInfoV5( DXARC *DXA, const char *FilePath )
{
	DXARC_DIRECTORY_VER5 *OldDir ;
	DXARC_FILEHEAD_VER5 *FileH ;
	DWORD FileHeadSize ;
	BYTE *NameData ;
	int i, j, k, Num ;
	DXARC_SEARCHDATA SearchData ;

	// 元のディレクトリを保存しておく
	OldDir = DXA->CurrentDirectoryV5 ;

	// ファイルパスに \ or / が含まれている場合、ディレクトリ変更を行う
	if( _STRCHR( FilePath, '\\' ) != NULL || _STRCHR( FilePath, '/' ) != NULL )
	{
		// カレントディレクトリを目的のファイルがあるディレクトリに変更する
		if( DXA_ChangeCurrentDirectoryBase( DXA, FilePath, false, &SearchData ) >= 0 )
		{
			// エラーが起きなかった場合はファイル名もディレクトリだったことになるのでエラー
			goto ERR ;
		}
	}
	else
	{
		// ファイル名を検索用データに変換する
		DXA_ConvSearchData( DXA, &SearchData, FilePath, NULL ) ;
	}

	// 同名のファイルを探す
	FileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
	FileH        = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + DXA->CurrentDirectoryV5->FileHeadAddress ) ;
	Num          = ( int )DXA->CurrentDirectoryV5->FileHeadNum ;
	for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD_VER5 *)( (BYTE *)FileH + FileHeadSize ) )
	{
		// ディレクトリチェック
		if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) continue ;

		// 文字列数とパリティチェック
		NameData = DXA->Table.NameTable + FileH->NameAddress ;
		if( SearchData.PackNum != ((WORD *)NameData)[0] || SearchData.Parity != ((WORD *)NameData)[1] ) continue ;

		// 文字列チェック
		NameData += 4 ;
		for( j = 0, k = 0 ; j < SearchData.PackNum ; j ++, k += 4 )
			if( *((DWORD *)&SearchData.FileName[k]) != *((DWORD *)&NameData[k]) ) break ;

		// 適合したファイルがあったらここで終了
		if( SearchData.PackNum == j ) break ;
	}

	// 無かったらエラー
	if( i == Num ) goto ERR ;
	
	// ディレクトリを元に戻す
	DXA->CurrentDirectoryV5 = OldDir ;
	
	// 目的のファイルのアドレスを返す
	return FileH ;
	
ERR :
	// ディレクトリを元に戻す
	DXA->CurrentDirectoryV5 = OldDir ;
	
	// エラー終了
	return NULL ;
}

// ファイルの情報を得る
static DXARC_FILEHEAD *DXA_GetFileInfo( DXARC *DXA, const char *FilePath )
{
	DXARC_DIRECTORY *OldDir ;
	DXARC_FILEHEAD *FileH ;
	DWORD FileHeadSize ;
	BYTE *NameData ;
	int i, j, k, Num ;
	DXARC_SEARCHDATA SearchData ;

	// 元のディレクトリを保存しておく
	OldDir = DXA->CurrentDirectory ;

	// ファイルパスに \ or / が含まれている場合、ディレクトリ変更を行う
	if( _STRCHR( FilePath, '\\' ) != NULL || _STRCHR( FilePath, '/' ) != NULL )
	{
		// カレントディレクトリを目的のファイルがあるディレクトリに変更する
		if( DXA_ChangeCurrentDirectoryBase( DXA, FilePath, false, &SearchData ) >= 0 )
		{
			// エラーが起きなかった場合はファイル名もディレクトリだったことになるのでエラー
			goto ERR ;
		}
	}
	else
	{
		// ファイル名を検索用データに変換する
		DXA_ConvSearchData( DXA, &SearchData, FilePath, NULL ) ;
	}

	// 同名のファイルを探す
	FileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;
	FileH        = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + DXA->CurrentDirectory->FileHeadAddress ) ;
	Num          = ( int )DXA->CurrentDirectory->FileHeadNum ;
	for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD *)( (BYTE *)FileH + FileHeadSize ) )
	{
		// ディレクトリチェック
		if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 ) continue ;

		// 文字列数とパリティチェック
		NameData = DXA->Table.NameTable + FileH->NameAddress ;
		if( SearchData.PackNum != ((WORD *)NameData)[0] || SearchData.Parity != ((WORD *)NameData)[1] ) continue ;

		// 文字列チェック
		NameData += 4 ;
		for( j = 0, k = 0 ; j < SearchData.PackNum ; j ++, k += 4 )
			if( *((DWORD *)&SearchData.FileName[k]) != *((DWORD *)&NameData[k]) ) break ;

		// 適合したファイルがあったらここで終了
		if( SearchData.PackNum == j ) break ;
	}

	// 無かったらエラー
	if( i == Num ) goto ERR ;
	
	// ディレクトリを元に戻す
	DXA->CurrentDirectory = OldDir ;
	
	// 目的のファイルのアドレスを返す
	return FileH ;
	
ERR :
	// ディレクトリを元に戻す
	DXA->CurrentDirectory = OldDir ;
	
	// エラー終了
	return NULL ;
}


// 文字列を検索用のデータに変換( ヌル文字か \ があったら終了 )
static int DXA_ConvSearchData( DXARC *DXA, DXARC_SEARCHDATA *Dest, const char *Src, int *Length )
{
	int i, StringLength ;
	WORD ParityData ;

	ParityData = 0 ;
	for( i = 0 ; Src[i] != '\0' && Src[i] != '\\' && Src[i] != '/' ; )
	{
		if( CheckMultiByteChar( Src[i], DXA->CharSet ) == TRUE )
		{
			// ２バイト文字の場合はそのままコピー
			*((WORD *)&Dest->FileName[i]) = *((WORD *)&Src[i]) ;
			ParityData += (BYTE)Src[i] + (BYTE)Src[i+1] ;
			i += 2 ;
		}
		else
		{
			// 小文字の場合は大文字に変換
			if( Src[i] >= 'a' && Src[i] <= 'z' )	Dest->FileName[i] = (BYTE)Src[i] - 'a' + 'A' ;
			else									Dest->FileName[i] = Src[i] ;
			ParityData += ( WORD )Dest->FileName[i] ;
			i ++ ;
		}
	}

	// 文字列の長さを保存
	if( Length != NULL ) *Length = i ;

	// ４の倍数の位置まで０を代入
	StringLength = ( ( i + 1 ) + 3 ) / 4 * 4 ;
	_MEMSET( &Dest->FileName[i], 0, StringLength - i ) ;

	// パリティデータの保存
	Dest->Parity = ParityData ;

	// パックデータ数の保存
	Dest->PackNum = ( WORD )( StringLength / 4 ) ;

	// 正常終了
	return 0 ;
}






// 鍵文字列を作成
void DXA_KeyCreate( const char *Source, unsigned char *Key )
{
	size_t Len ;

	if( Source == NULL )
	{
		_MEMSET( Key, 0xaa, DXA_KEYSTR_LENGTH ) ;
	}
	else
	{
		Len = strlen( Source ) ;
		if( Len > DXA_KEYSTR_LENGTH )
		{
			_MEMCPY( Key, Source, DXA_KEYSTR_LENGTH ) ;
		}
		else
		{
			// 鍵文字列が DXA_KEYSTR_LENGTH より短かったらループする
			size_t i ;

			for( i = 0 ; i + Len <= DXA_KEYSTR_LENGTH ; i += Len )
				_MEMCPY( Key + i, Source, Len ) ;
			if( i < DXA_KEYSTR_LENGTH )
				_MEMCPY( Key + i, Source, DXA_KEYSTR_LENGTH - i ) ;
		}
	}

	Key[0] = ( unsigned char )( ~Key[0] ) ;
	Key[1] = ( unsigned char )( ( Key[1] >> 4 ) | ( Key[1] << 4 ) ) ;
	Key[2] = ( unsigned char )( Key[2] ^ 0x8a ) ;
	Key[3] = ( unsigned char )( ~( ( Key[3] >> 4 ) | ( Key[3] << 4 ) ) ) ;
	Key[4] = ( unsigned char )( ~Key[4] ) ;
	Key[5] = ( unsigned char )( Key[5] ^ 0xac ) ;
	Key[6] = ( unsigned char )( ~Key[6] ) ;
	Key[7] = ( unsigned char )( ~( ( Key[7] >> 3 ) | ( Key[7] << 5 ) ) ) ;
	Key[8] = ( unsigned char )( ( Key[8] >> 5 ) | ( Key[8] << 3 ) ) ;
	Key[9] = ( unsigned char )( Key[9] ^ 0x7f ) ;
	Key[10] = ( unsigned char )( ( ( Key[10] >> 4 ) | ( Key[10] << 4 ) ) ^ 0xd6 ) ;
	Key[11] = ( unsigned char )( Key[11] ^ 0xcc ) ;
}

// 鍵文字列を使用して Xor 演算( Key は必ず DXA_KEYSTR_LENGTH の長さがなければならない )
void DXA_KeyConv( void *Data, LONGLONG SizeLL, LONGLONG PositionLL, unsigned char *Key )
{
	int Position ;

	Position = ( int )( PositionLL % DXA_KEYSTR_LENGTH ) ;

	if( SizeLL <= 0xffffffff )
	{
		DWORD SizeT ;
		SizeT = ( DWORD )SizeLL ;

#ifndef DX_NON_INLINE_ASM
		DWORD DataT ;
		DataT = (DWORD)Data ;

		__asm
		{
			MOV EDI, DataT
			MOV ESI, Key

			MOV EAX, SizeT
			CMP EAX, DXA_KEYSTR_LENGTH
			JB LABEL2


			MOV EAX, Position
			CMP EAX, 0
			JE LABEL1


			MOV EDX, SizeT
	LOOP1:
			MOV BL, [ESI+EAX]
			XOR [EDI], BL
			INC EAX
			INC EDI
			DEC EDX
			CMP EAX, DXA_KEYSTR_LENGTH
			JB LOOP1
			XOR ECX, ECX
			MOV Position, ECX

			MOV SizeT, EDX
			CMP EDX, DXA_KEYSTR_LENGTH
			JB LABEL2


	LABEL1:
			MOV EAX, SizeT
			XOR EDX, EDX
			MOV ECX, DXA_KEYSTR_LENGTH
			DIV ECX
			MOV SizeT, EDX
			MOV ECX, EAX

			MOV EAX, [ESI]
			MOV EBX, [ESI+4]
			MOV EDX, [ESI+8]
	LOOP2:
			XOR [EDI],    EAX
			XOR [EDI+4],  EBX
			XOR [EDI+8],  EDX
			ADD EDI, DXA_KEYSTR_LENGTH
			DEC ECX
			JNZ LOOP2


	LABEL2:
			MOV EDX, SizeT
			CMP EDX, 0
			JE LABEL3


			MOV EAX, Position
	LOOP3:
			MOV BL, [ESI+EAX]
			XOR [EDI], BL
			INC EAX
			CMP EAX, DXA_KEYSTR_LENGTH
			JNE LABEL4
			XOR EAX, EAX
	LABEL4:
			INC EDI
			DEC EDX
			JNZ LOOP3
	LABEL3:
		} ;

#else // DX_NON_INLINE_ASM

		BYTE *DataBP ;
		BYTE *KeyBP ;

		DataBP = ( BYTE * )Data ;
		KeyBP = ( BYTE * )Key ;
		if( SizeT >= DXA_KEYSTR_LENGTH )
		{
			if( Position != 0 )
			{
				do
				{
					*DataBP ^= KeyBP[ Position ] ;
					Position ++ ;
					DataBP ++ ;
					SizeT -- ;
				}while( Position < DXA_KEYSTR_LENGTH ) ;
				Position = 0 ;
			}

			if( SizeT >= DXA_KEYSTR_LENGTH )
			{
				DWORD SetNum ;
				DWORD Key1, Key2, Key3 ;

				SetNum = SizeT / DXA_KEYSTR_LENGTH ;
				SizeT -= SetNum * DXA_KEYSTR_LENGTH ;
				Key1 = ( ( DWORD * )KeyBP )[ 0 ] ;
				Key2 = ( ( DWORD * )KeyBP )[ 1 ] ;
				Key3 = ( ( DWORD * )KeyBP )[ 2 ] ;

				do
				{
					( ( DWORD * )DataBP )[ 0 ] ^= Key1 ;
					( ( DWORD * )DataBP )[ 1 ] ^= Key2 ;
					( ( DWORD * )DataBP )[ 2 ] ^= Key3 ;
					DataBP += DXA_KEYSTR_LENGTH ;
					SetNum -- ;
				}while( SetNum > 0 ) ;
			}
		}

		if( SizeT > 0 )
		{
			do
			{
				*DataBP ^= KeyBP[ Position ] ;
				DataBP ++ ;
				Position ++ ;
				if( Position == DXA_KEYSTR_LENGTH )
					Position = 0 ;
				SizeT -- ;
			}while( SizeT > 0 ) ;
		}
#endif // DX_NON_INLINE_ASM
	}
	else
	{
		BYTE *DataBP ;
		BYTE *KeyBP ;

		DataBP = ( BYTE * )Data ;
		KeyBP = ( BYTE * )Key ;
		if( SizeLL >= DXA_KEYSTR_LENGTH )
		{
			if( Position != 0 )
			{
				do
				{
					*DataBP ^= KeyBP[ Position ] ;
					Position ++ ;
					DataBP ++ ;
					SizeLL -- ;
				}while( Position < DXA_KEYSTR_LENGTH ) ;
				Position = 0 ;
			}

			if( SizeLL >= DXA_KEYSTR_LENGTH )
			{
				DWORD SetNum ;
				DWORD Key1, Key2, Key3 ;

				SetNum = ( DWORD )( SizeLL / DXA_KEYSTR_LENGTH ) ;
				SizeLL -= SetNum * DXA_KEYSTR_LENGTH ;
				Key1 = ( ( DWORD * )KeyBP )[ 0 ] ;
				Key2 = ( ( DWORD * )KeyBP )[ 1 ] ;
				Key3 = ( ( DWORD * )KeyBP )[ 2 ] ;

				do
				{
					( ( DWORD * )DataBP )[ 0 ] ^= Key1 ;
					( ( DWORD * )DataBP )[ 1 ] ^= Key2 ;
					( ( DWORD * )DataBP )[ 2 ] ^= Key3 ;
					DataBP += DXA_KEYSTR_LENGTH ;
					SetNum -- ;
				}while( SetNum > 0 ) ;
			}
		}

		if( SizeLL > 0 )
		{
			do
			{
				*DataBP ^= KeyBP[ Position ] ;
				DataBP ++ ;
				Position ++ ;
				if( Position == DXA_KEYSTR_LENGTH )
					Position = 0 ;
				SizeLL -- ;
			}while( SizeLL > 0 ) ;
		}
	}
}

// ファイルから読み込んだデータを鍵文字列を使用して Xor 演算する関数( Key は必ず DXA_KEYSTR_LENGTH の長さがなければならない )
void DXA_KeyConvFileRead( void *Data, ULONGLONG  Size, DWORD_PTR FilePointer, unsigned char *Key, LONGLONG Position )
{
	LONGLONG pos ;

	// ファイルの位置を取得しておく
	if( Position == -1 ) pos = ReadOnlyFileAccessTell( FilePointer ) ;
	else                 pos = Position ;

	// 読み込む
	ReadOnlyFileAccessRead( Data, ( size_t )Size, 1, FilePointer ) ;
	while( ReadOnlyFileAccessIdleCheck( FilePointer ) == FALSE ) Sleep(1);

	// データを鍵文字列を使って Xor 演算
	DXA_KeyConv( Data, Size, pos, Key ) ;
}

// 条件に適合するオブジェクトを検索する(検索対象は ObjectCount をインデックスとしたところから)(戻り値 -1:エラー 0:成功)
static int DXA_FindProcess( DXA_FINDDATA *FindData, FILEINFO *FileInfo )
{
	BYTE *nameTable;
	DXARC *DXA;
	char *str, *name;
	DWORD fileHeadSize;
	char TempName[ 1024 ] ;

	DXA = FindData->Container;
	nameTable = DXA->Table.NameTable;
	str = FindData->SearchStr;

	if( DXA->V5Flag )
	{
		int i, num, addnum;

		DXARC_DIRECTORY_VER5 *dir;
		DXARC_FILEHEAD_VER5 *file;

		dir = FindData->DirectoryV5;
		num = dir->FileHeadNum;
		addnum = dir->ParentDirectoryAddress == 0xffffffff ? 1 : 2;
		fileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;

		if( FindData->ObjectCount == (DWORD)( num + addnum ) ) return -1;

		file = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + dir->FileHeadAddress + fileHeadSize * ( FindData->ObjectCount - addnum ) ) ;
		for( i = FindData->ObjectCount; i < num + addnum; i ++, file = (DXARC_FILEHEAD_VER5 *)( (BYTE *)file + fileHeadSize ) )
		{
			if( i < addnum )
			{
					 if( i == 0 ){ if( DXA_DIR_FileNameCmp( DXA, ".",  str ) == 0 ) break; }
				else if( i == 1 ){ if( DXA_DIR_FileNameCmp( DXA, "..", str ) == 0 ) break; }
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress + 4 );
				if( DXA_DIR_FileNameCmp( DXA, name, str ) == 0 ) break;
			}
		}
		FindData->ObjectCount = i;
		if( i == num + addnum )
			return -1;

		if( FileInfo )
		{
			if( i < addnum )
			{
				switch( i )
				{
				case 0: _STRCPY( TempName, "."  ); break;
				case 1: _STRCPY( TempName, ".." ); break;
				}
				FileInfo->DirFlag = 1;
				FileInfo->Size    = 0;
				_MEMSET( &FileInfo->CreationTime,  0, sizeof( FileInfo->CreationTime  ) );
				_MEMSET( &FileInfo->LastWriteTime, 0, sizeof( FileInfo->LastWriteTime ) );
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress );
				_STRCPY( TempName, name + ((WORD *)name)[0] * 4 + 4 );
				FileInfo->DirFlag = (file->Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;
				FileInfo->Size = (LONGLONG)file->DataSize;
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.Create,    &FileInfo->CreationTime  );
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.LastWrite, &FileInfo->LastWriteTime );
			}

#ifdef UNICODE
			MBCharToWChar( DXA->HeadV5.CodePage, TempName, ( DXWCHAR * )FileInfo->Name, 260 ) ;
#else
			lstrcpy( FileInfo->Name, TempName );
#endif
		}
	}
	else
	{
		ULONGLONG i, num, addnum;
		DXARC_DIRECTORY *dir;
		DXARC_FILEHEAD *file;

		dir = FindData->Directory;
		num = dir->FileHeadNum;
		addnum = dir->ParentDirectoryAddress == NONE_PAL ? 1 : 2;
		fileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;

		if( FindData->ObjectCount == (DWORD)( num + addnum ) ) return -1;

		file = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + dir->FileHeadAddress + fileHeadSize * ( FindData->ObjectCount - addnum ) ) ;
		for( i = FindData->ObjectCount; i < num + addnum; i ++, file = (DXARC_FILEHEAD *)( (BYTE *)file + fileHeadSize ) )
		{
			if( i < addnum )
			{
					 if( i == 0 ){ if( DXA_DIR_FileNameCmp( DXA, ".",  str ) == 0 ) break; }
				else if( i == 1 ){ if( DXA_DIR_FileNameCmp( DXA, "..", str ) == 0 ) break; }
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress + 4 );
				if( DXA_DIR_FileNameCmp( DXA, name, str ) == 0 ) break;
			}
		}
		FindData->ObjectCount = ( DWORD )i;
		if( i == num + addnum )
			return -1;

		if( FileInfo )
		{
			if( i < addnum )
			{
				switch( i )
				{
				case 0: _STRCPY( TempName, "."  ); break;
				case 1: _STRCPY( TempName, ".." ); break;
				}
				FileInfo->DirFlag = 1;
				FileInfo->Size    = 0;
				_MEMSET( &FileInfo->CreationTime,  0, sizeof( FileInfo->CreationTime  ) );
				_MEMSET( &FileInfo->LastWriteTime, 0, sizeof( FileInfo->LastWriteTime ) );
			}
			else
			{
				name = (char *)( nameTable + file->NameAddress );
				_STRCPY( TempName, name + ((WORD *)name)[0] * 4 + 4 );
				FileInfo->DirFlag = (file->Attributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ? TRUE : FALSE;
				FileInfo->Size = (LONGLONG)file->DataSize;
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.Create,    &FileInfo->CreationTime  );
				_FileTimeToLocalDateData( (FILETIME *)&file->Time.LastWrite, &FileInfo->LastWriteTime );
			}

#ifdef UNICODE
			MBCharToWChar( DXA->Head.CodePage, TempName, ( DXWCHAR * )FileInfo->Name, 260 ) ;
#else
			lstrcpy( FileInfo->Name, TempName );
#endif
		}
	}

	return 0;
}



// アーカイブファイルを扱う為の構造体を初期化する
extern	int	DXA_Initialize( DXARC *DXA )
{
	_MEMSET( DXA, 0, sizeof( DXARC ) ) ;

	// 終了
	return 0 ;
}

// アーカイブファイルを扱う為の構造体の後始末をする
extern int DXA_Terminate( DXARC *DXA )
{
	DXA_CloseArchive( DXA ) ;

	_MEMSET( DXA, 0, sizeof( DXARC ) ) ;

	// 終了
	return 0 ;
}

// 指定のディレクトリデータの暗号化を解除する( 丸ごとメモリに読み込んだ場合用 )
static int DXA_DirectoryKeyConv( DXARC *DXA, DXARC_DIRECTORY *Dir )
{
	// メモリイメージではない場合はエラー
	if( DXA->MemoryOpenFlag == FALSE )
		return -1 ;

	// バージョン 0x0006 より前では何もしない
	if( DXA->V5Flag )
		return 0 ;

	// 暗号化解除処理開始
	{
		DWORD i, FileHeadSize ;
		DXARC_FILEHEAD *File ;

		// 格納されているファイルの数だけ繰り返す
		FileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;
		File = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + Dir->FileHeadAddress ) ;
		for( i = 0 ; i < Dir->FileHeadNum ; i ++, File = ( DXARC_FILEHEAD * )( ( BYTE * )File + FileHeadSize ) )
		{
			// ディレクトリかどうかで処理を分岐
			if( File->Attributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// ディレクトリの場合は再帰をかける
				DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + File->DataAddress ) ) ;
			}
			else
			{
				BYTE *DataP ;

				// ファイルの場合は暗号化を解除する
				
				// データがある場合のみ処理
				if( File->DataSize != 0 )
				{
					// データ位置をセットする
					DataP = ( BYTE * )DXA->MemoryImage + DXA->Head.DataStartAddress + File->DataAddress ;

					// データが圧縮されているかどうかで処理を分岐
					if( File->PressDataSize != NONE_PAL )
					{
						// 圧縮されている場合
						DXA_KeyConv( DataP, File->PressDataSize, File->DataSize, DXA->Key ) ;
					}
					else
					{
						// 圧縮されていない場合
						DXA_KeyConv( DataP, File->DataSize, File->DataSize, DXA->Key ) ;
					}
				}
			}
		}
	}

	// 終了
	return 0 ;
}

// 指定のディレクトリデータの暗号化を解除する( 丸ごとメモリに読み込んだ場合用 )
static int DXA_DirectoryKeyConvV5( DXARC *DXA, DXARC_DIRECTORY_VER5 *Dir )
{
	// メモリイメージではない場合はエラー
	if( DXA->MemoryOpenFlag == FALSE )
		return -1 ;

	// バージョン 0x0005 より前では何もしない
	if( DXA->HeadV5.Version < 0x0005 )
		return 0 ;
	
	// 暗号化解除処理開始
	{
		DWORD i, FileHeadSize ;
		DXARC_FILEHEAD_VER5 *File ;

		// 格納されているファイルの数だけ繰り返す
		FileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
		File = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + Dir->FileHeadAddress ) ;
		for( i = 0 ; i < Dir->FileHeadNum ; i ++, File = ( DXARC_FILEHEAD_VER5 * )( ( BYTE * )File + FileHeadSize ) )
		{
			// ディレクトリかどうかで処理を分岐
			if( File->Attributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				// ディレクトリの場合は再帰をかける
				DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + File->DataAddress ) ) ;
			}
			else
			{
				BYTE *DataP ;

				// ファイルの場合は暗号化を解除する
				
				// データがある場合のみ処理
				if( File->DataSize != 0 )
				{
					// データ位置をセットする
					DataP = ( BYTE * )DXA->MemoryImage + DXA->HeadV5.DataStartAddress + File->DataAddress ;

					// データが圧縮されているかどうかで処理を分岐
					if( DXA->HeadV5.Version >= 0x0002 && File->PressDataSize != 0xffffffff )
					{
						// 圧縮されている場合
						DXA_KeyConv( DataP, File->PressDataSize, File->DataSize, DXA->Key ) ;
					}
					else
					{
						// 圧縮されていない場合
						DXA_KeyConv( DataP, File->DataSize, File->DataSize, DXA->Key ) ;
					}
				}
			}
		}
	}

	// 終了
	return 0 ;
}

// メモリ上にあるアーカイブファイルイメージを開く( 0:成功  -1:失敗 )
extern int DXA_OpenArchiveFromMem( DXARC *DXA, void *ArchiveImage, int ArchiveSize, int ArchiveImageCopyFlag, int ArchiveImageReadOnlyFlag, const char *KeyString, const TCHAR *EmulateArchivePath )
{
	BYTE *datp ;

	// メモリイメージをコピーして使用する場合は読み取り専用にはしない
	if( ArchiveImageCopyFlag )
	{
		ArchiveImageReadOnlyFlag = FALSE ;
	}

	// 既になんらかのアーカイブを開いていた場合はエラー
	if( DXA->WinFilePointer__ != 0 || DXA->MemoryImage != NULL ) return -1 ;

	// 鍵の作成
	DXA_KeyCreate( KeyString, DXA->Key ) ;

	// ファイルパスを保存
	if( EmulateArchivePath != NULL )
	{
		lstrcpy( DXA->FilePath, EmulateArchivePath ) ;
	}
	else
	{
		_MEMSET( DXA->FilePath, 0, sizeof( DXA->FilePath ) ) ;
	}

	DXA->Table.Top = NULL ;
	DXA->MemoryImage = NULL ;
	if( ArchiveImageCopyFlag )
	{
		// イメージをコピーするフラグが立っている場合はコピー先のメモリ領域を確保
		DXA->MemoryImage = DXALLOC( ( size_t )ArchiveSize ) ;
		if( DXA->MemoryImage == NULL )
			return -1 ;

		// アーカイブファイルイメージの内容を確保したメモリにコピー
		_MEMCPY( DXA->MemoryImage, ArchiveImage, ArchiveSize ) ;

		// コピーしたメモリイメージを使用する
		DXA->MemoryImageOriginal = ArchiveImage ;
		ArchiveImage             = DXA->MemoryImage ;
	}
	else
	{
		// ポインタを保存
		DXA->MemoryImage         = ArchiveImage ;
		DXA->MemoryImageOriginal = NULL ;
	}

	DXA->V5Flag = FALSE ;

	// 最初にヘッダの部分を反転する
	_MEMCPY( &DXA->Head, ArchiveImage, DXARC_ID_AND_VERSION_SIZE ) ;
	DXA_KeyConv( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, 0, DXA->Key ) ;

	// ＩＤが違う場合はバージョン２以前か調べる
	if( DXA->Head.Head != DXAHEAD )
	{
		// バージョン２以前か調べる
		_MEMSET( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;

		_MEMCPY( &DXA->Head, ArchiveImage, DXARC_ID_AND_VERSION_SIZE ) ;
		DXA_KeyConv( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, 0, DXA->Key ) ;

		// バージョン２以前でもない場合はエラー
		if( DXA->Head.Head != DXAHEAD )
			goto ERR ;
	}

	// バージョン6以降かどうかで処理を分岐
	if( DXA->Head.Version >= 0x0006 )
	{
		DXA->V5Flag = FALSE ;

		DXA->Head.CodePage = 0 ;

		// ヘッダを解析する
		{
			_MEMCPY( &DXA->Head, ArchiveImage, DXARC_HEAD_VER6_SIZE ) ;
			DXA_KeyConv( &DXA->Head, DXARC_HEAD_VER6_SIZE, 0, DXA->Key ) ;

			// ＩＤの検査
			if( DXA->Head.Head != DXAHEAD ) goto ERR ;
			
			// バージョン検査
			if( DXA->Head.Version > DXAVER ) goto ERR ;

			// コードページをセット
			switch( DXA->Head.CodePage )
			{
			case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
			case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
			case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
			case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
			default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
			}

			// 読み取り専用の場合は情報テーブルのサイズ分のメモリを確保する
			if( ArchiveImageReadOnlyFlag )
			{
				DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
				if( DXA->Table.Top == NULL ) goto ERR ;

				_MEMCPY( DXA->Table.Top,  (BYTE *)DXA->MemoryImage + DXA->Head.FileNameTableStartAddress, DXA->Head.HeadSize ) ;
			}
			else
			{
				// 情報テーブルのアドレスをセットする
				DXA->Table.Top          = (BYTE *)DXA->MemoryImage + DXA->Head.FileNameTableStartAddress ;
			}
			DXA_KeyConv( DXA->Table.Top, DXA->Head.HeadSize, 0, DXA->Key ) ;

			DXA->Table.NameTable		= DXA->Table.Top ;
			DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
			DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;
		}

		// カレントディレクトリのセット
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;

		DXA->MemoryOpenFlag					= TRUE ;						// メモリイメージから開いているフラグを立てる
		DXA->UserMemoryImageFlag			= TRUE ;						// ユーザーのイメージから開いたフラグを立てる
		DXA->MemoryImageSize				= ArchiveSize ;					// サイズも保存しておく
		DXA->MemoryImageCopyFlag			= ArchiveImageCopyFlag ;		// メモリイメージをコピーして使用しているかどうかのフラグを保存
		DXA->MemoryImageReadOnlyFlag        = ArchiveImageReadOnlyFlag ;	// メモリイメージを読み取り専用にするかどうかのフラグを保存

		// 全てのファイルデータの暗号化を解除する
		if( ArchiveImageReadOnlyFlag == FALSE )
		{
			DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ) ;
		}
	}
	else
	{
		DXA->V5Flag = TRUE ;

		// バージョン検査
		if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

		// メモリを読み取り専用とするかどうかで処理を分岐
		if( ArchiveImageReadOnlyFlag )
		{
			// バージョンが４以上かどうかで読み込む残りのヘッダサイズを変更
			if( DXA->HeadV5.Version >= 0x0004 )
			{
				_MEMCPY( &DXA->HeadV5, ArchiveImage, DXARC_HEAD_VER4_SIZE ) ;
				DXA_KeyConv( &DXA->HeadV5, DXARC_HEAD_VER4_SIZE, 0, DXA->Key ) ;
				switch( DXA->HeadV5.CodePage )
				{
				case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
				case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
				case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
				case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
				default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
				}
			}
			else
			{
				_MEMCPY( &DXA->HeadV5, ArchiveImage, DXARC_HEAD_VER3_SIZE ) ;
				DXA_KeyConv( &DXA->HeadV5, DXARC_HEAD_VER3_SIZE, 0, DXA->Key ) ;

				DXA->HeadV5.CodePage = 0 ;
				DXA->CharSet = DX_CHARSET_DEFAULT ;
			}

			// 情報テーブルのサイズ分のメモリを確保する
			DXA->Table.Top = ( BYTE * )DXALLOC( DXA->HeadV5.HeadSize ) ;
			if( DXA->Table.Top == NULL ) goto ERR ;
			
			// 情報テーブルをメモリに読み込む
			_MEMCPY( DXA->Table.Top, (BYTE *)DXA->MemoryImage + DXA->HeadV5.FileNameTableStartAddress, DXA->HeadV5.HeadSize ) ;
			if( DXA->HeadV5.Version >= 0x0005 )
			{
				DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize,                                     0, DXA->Key ) ;
			}
			else
			{
				DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->HeadV5.FileNameTableStartAddress, DXA->Key ) ;
			}
		}
		else
		{
			// すべてのデータを反転する
			if( DXA->HeadV5.Version < 0x0005 )
			{
				DXA_KeyConv( ArchiveImage, ArchiveSize, 0, DXA->Key ) ;
			}

			datp = (BYTE *)ArchiveImage ;

			// ヘッダを解析する
			{
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					_MEMCPY( &DXA->HeadV5, datp, DXARC_HEAD_VER3_SIZE ) ;
					DXA_KeyConv( &DXA->HeadV5, DXARC_HEAD_VER3_SIZE, 0, DXA->Key ) ;
				}
				else
				{
					_MEMCPY( &DXA->HeadV5, datp, DXARC_HEAD_VER3_SIZE ) ;
				}
				datp += DXARC_HEAD_VER3_SIZE ;

				// ＩＤの検査
				if( DXA->HeadV5.Head != DXAHEAD ) goto ERR ;
				
				// バージョン検査
				if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

				// バージョンが 4以上だったらコードページを読み込む
				if( DXA->HeadV5.Version >= 0x0004 )
				{
					DXA->HeadV5.CodePage = *( ( DWORD * )datp ) ;
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( &DXA->HeadV5.CodePage, 4, DXARC_HEAD_VER3_SIZE, DXA->Key ) ;
					}
					switch( DXA->HeadV5.CodePage )
					{
					case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
					case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
					case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
					case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
					default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
					}
				}
				else
				{
					DXA->HeadV5.CodePage = 0 ;
					DXA->CharSet = DX_CHARSET_DEFAULT ;
				}

				// 情報テーブルのアドレスをセットする
				DXA->Table.Top = (BYTE *)DXA->MemoryImage + DXA->HeadV5.FileNameTableStartAddress ;
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize, 0, DXA->Key ) ;
				}
			}
		}

		// 情報テーブルのアドレスをセットする
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->HeadV5.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->HeadV5.DirectoryTableStartAddress ;

		// カレントディレクトリのセット
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;

		DXA->MemoryOpenFlag					= TRUE ;						// メモリイメージから開いているフラグを立てる
		DXA->UserMemoryImageFlag			= TRUE ;						// ユーザーのイメージから開いたフラグを立てる
		DXA->MemoryImageSize				= ArchiveSize ;					// サイズも保存しておく
		DXA->MemoryImageCopyFlag			= ArchiveImageCopyFlag ;		// メモリイメージをコピーして使用しているかどうかのフラグを保存
		DXA->MemoryImageReadOnlyFlag        = ArchiveImageReadOnlyFlag ;	// メモリイメージを読み取り専用にするかどうかのフラグを保存

		if( ArchiveImageReadOnlyFlag == FALSE )
		{
			// 全てのファイルデータの暗号化を解除する
			if( DXA->HeadV5.Version >= 0x0005 )
			{
				DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ) ;
			}
		}
	}

	// 終了
	return 0 ;

ERR :
	if( ArchiveImageCopyFlag )
	{
		if( DXA->MemoryImage != NULL )
		{
			DXFREE( DXA->MemoryImage ) ;
			DXA->MemoryImage = NULL ;
		}
	}
	else
	if( ArchiveImageReadOnlyFlag )
	{
		if( DXA->Table.Top != NULL )
		{
			DXFREE( DXA->Table.Top ) ;
			DXA->Table.Top = NULL ;
		}
	}
	else
	{
		if( DXA->V5Flag )
		{
			// 反転したデータを元に戻す
			if( DXA->HeadV5.Version < 0x0005 )
			{
				DXA_KeyConv( ArchiveImage, ArchiveSize, 0, DXA->Key ) ;
			}
		}
	}
	
	// 終了
	return -1 ;
}

// アーカイブファイルを扱う準備が整ったかを得る( TRUE:整っている  FALSE:整っていない )
extern int DXA_CheckIdle( DXARC *DXA )
{
	// 非同期オープン中ではなければ特にやることはない
	if( DXA->ASyncOpenFlag == FALSE ) return TRUE ;

	// ファイル読み込みが完了しているか調べる
	if( ReadOnlyFileAccessIdleCheck( DXA->ASyncOpenFilePointer ) == FALSE ) return FALSE ;

	// ファイルを閉じる
	ReadOnlyFileAccessClose( DXA->ASyncOpenFilePointer ) ;
	DXA->ASyncOpenFilePointer = 0;

	// すべてのデータを反転する
	if( DXA->V5Flag == FALSE )
	{
		DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ) ;
	}
	else
	{
		if( DXA->HeadV5.Version >= 0x0005 )
		{
			DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ) ;
		}
		else
		{
			DXA_KeyConv( DXA->MemoryImage, DXA->MemoryImageSize, 0, DXA->Key ) ;
		}
	}

	// 非同期オープン中フラグを倒す
	DXA->ASyncOpenFlag = FALSE ;

	// 終了
	return TRUE ;
}

// アーカイブファイルを開き最初にすべてメモリ上に読み込んでから処理する( 0:成功  -1:失敗 )
extern int DXA_OpenArchiveFromFileUseMem( DXARC *DXA, const TCHAR *ArchivePath, const char *KeyString , int ASyncThread )
{
	// 既になんらかのアーカイブを開いていた場合はエラー
	if( DXA->WinFilePointer__ != 0 || DXA->MemoryImage ) return -1 ;

	// 鍵の作成
	DXA_KeyCreate( KeyString, DXA->Key ) ;

	// ヘッダ部分だけ先に読み込む
	DXA->ASyncOpenFilePointer = 0 ;
	DXA->MemoryImage = NULL ;
	DXA->ASyncOpenFilePointer = ReadOnlyFileAccessOpen( ArchivePath, FALSE, TRUE, FALSE ) ;
	if( DXA->ASyncOpenFilePointer == 0 ) return -1 ;

	// ファイルパスを保存
	lstrcpy( DXA->FilePath, ArchivePath ) ;

	// ファイルのサイズを取得する
	ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_END ) ;
	DXA->MemoryImageSize = ReadOnlyFileAccessTell( DXA->ASyncOpenFilePointer ) ;
	ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;

	// ファイルの内容を全てメモリに読み込む為のメモリ領域の確保
	DXA->MemoryImage = DXALLOC( ( size_t )DXA->MemoryImageSize ) ;

	// IDとバージョンを読み込み
	DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;

	// ＩＤが違う場合はバージョン２以前か調べる
	if( DXA->Head.Head != DXAHEAD )
	{
		// バージョン２以前か調べる
		_MEMSET( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;

		ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;
		DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;

		// バージョン２以前でもない場合はエラー
		if( DXA->Head.Head != DXAHEAD )
			goto ERR ;
	}

	// バージョン6以降かどうかで処理を分岐
	if( DXA->Head.Version >= 0x0006 )
	{
		DXA->V5Flag = FALSE ;

		// バージョン検査
		if( DXA->Head.Version > DXAVER ) goto ERR ;

		// バージョンが４以上かどうかで読み込む残りのヘッダサイズを変更
		DXA_KeyConvFileRead( ( BYTE * )&DXA->Head + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER6_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
		switch( DXA->Head.CodePage )
		{
		case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
		case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
		case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
		case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
		default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
		}
		
		// 情報テーブルのサイズ分のメモリを確保する
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// 情報テーブルをメモリに読み込む
		ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, DXA->Head.FileNameTableStartAddress, SEEK_SET ) ;
		DXA_KeyConvFileRead( DXA->Table.Top, DXA->Head.HeadSize, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;

		// 情報テーブルのアドレスをセットする
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;

		// カレントディレクトリのセット
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
	}
	else
	{
		DXA->V5Flag = TRUE ;

		// バージョン検査
		if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

		// バージョンが４以上かどうかで読み込む残りのヘッダサイズを変更
		if( DXA->HeadV5.Version >= 0x0004 )
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER4_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			switch( DXA->HeadV5.CodePage )
			{
			case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
			case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
			case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
			case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
			default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
			}
		}
		else
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER3_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->ASyncOpenFilePointer, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			DXA->HeadV5.CodePage = 0 ;
			DXA->CharSet = DX_CHARSET_DEFAULT ;
		}
		
		// 情報テーブルのサイズ分のメモリを確保する
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->HeadV5.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// 情報テーブルをメモリに読み込む
		ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, DXA->HeadV5.FileNameTableStartAddress, SEEK_SET ) ;
		if( DXA->HeadV5.Version >= 0x0005 )
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->ASyncOpenFilePointer, DXA->Key, 0 ) ;
		}
		else
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->ASyncOpenFilePointer, DXA->Key ) ;
		}

		// 情報テーブルのアドレスをセットする
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->HeadV5.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->HeadV5.DirectoryTableStartAddress ;

		// カレントディレクトリのセット
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;
	}

	// 改めてファイルを丸ごと読み込む
	ReadOnlyFileAccessSeek( DXA->ASyncOpenFilePointer, 0L, SEEK_SET ) ;
	ReadOnlyFileAccessRead( DXA->MemoryImage, ( size_t )DXA->MemoryImageSize, 1, DXA->ASyncOpenFilePointer );

	// ファイル非同期オープン中だということをセットしておく
	DXA->ASyncOpenFlag = TRUE ;

	DXA->MemoryOpenFlag					= TRUE ;			// メモリイメージから開いているフラグを立てる
	DXA->UserMemoryImageFlag			= FALSE ;			// ユーザーのイメージから開いたわけではないのでフラグを倒す
	DXA->MemoryImageCopyFlag			= FALSE ;			// メモリイメージをコピーして使用しているフラグを倒す
	DXA->MemoryImageReadOnlyFlag        = FALSE ;			// メモリイメージを読み取り専用にするかどうかのフラグを倒す

	// 同期オープンの場合はここで開き終わるのを待つ
	if( ASyncThread == FALSE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// 終了
	return 0 ;

ERR :
	if( DXA->ASyncOpenFilePointer )
	{
		ReadOnlyFileAccessClose( DXA->ASyncOpenFilePointer );
		DXA->ASyncOpenFilePointer = 0;
	}

	if( DXA->MemoryImage )
	{
		DXFREE( DXA->MemoryImage );
		DXA->MemoryImage = 0;
	}

	DXA->ASyncOpenFlag = FALSE ;

	// 終了
	return -1 ;
}

// アーカイブファイルを開く( 0:成功  -1:失敗 )
extern int DXA_OpenArchiveFromFile( DXARC *DXA, const TCHAR *ArchivePath, const char *KeyString )
{
	// 既になんらかのアーカイブを開いていた場合はエラー
	if( DXA->WinFilePointer__ != 0 || DXA->MemoryImage ) return -1 ;

	// ヘッダの初期化
	_MEMSET( &DXA->Head, 0, sizeof( DXA->Head ) ) ;

	// アーカイブファイルを開こうと試みる
	DXA->WinFilePointer__ = ReadOnlyFileAccessOpen( ArchivePath, FALSE, TRUE, FALSE ) ;
	if( DXA->WinFilePointer__ == 0 ) return -1 ;

	// ファイルパスを保存
	lstrcpy( DXA->FilePath, ArchivePath ) ;

	// 鍵文字列の作成
	DXA_KeyCreate( KeyString, DXA->Key ) ;

	// ＩＤとバージョン番号部分だけを読み込み
	DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, 0 ) ;

	// ＩＤの検査
	if( DXA->Head.Head != DXAHEAD )
	{
		// バージョン２以前か調べる
		_MEMSET( DXA->Key, 0xff, DXA_KEYSTR_LENGTH ) ;

		ReadOnlyFileAccessSeek( DXA->WinFilePointer__, 0L, SEEK_SET ) ;
		DXA_KeyConvFileRead( &DXA->Head, DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, 0 ) ;

		// バージョン２以前でもない場合はエラー
		if( DXA->Head.Head != DXAHEAD )
			goto ERR ;
	}

	// バージョン6以降かどうかで処理を分岐
	if( DXA->Head.Version >= 0x0006 )
	{
		DXA->V5Flag = FALSE ;

		// バージョン検査
		if( DXA->Head.Version > DXAVER ) goto ERR ;

		// バージョンが４以上かどうかで読み込む残りのヘッダサイズを変更
		DXA_KeyConvFileRead( ( BYTE * )&DXA->Head + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER6_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
		switch( DXA->Head.CodePage )
		{
		case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
		case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
		case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
		case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
		default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
		}

		// 情報テーブルのサイズ分のメモリを確保する
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->Head.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// 情報テーブルをメモリに読み込む
		ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->Head.FileNameTableStartAddress, SEEK_SET ) ;
		DXA_KeyConvFileRead( DXA->Table.Top, DXA->Head.HeadSize, DXA->WinFilePointer__, DXA->Key, 0 ) ;

		// 情報テーブルのアドレスをセットする
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->Head.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->Head.DirectoryTableStartAddress ;

		// カレントディレクトリのセット
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ;
	}
	else
	{
		DXA->V5Flag = TRUE ;

		// バージョン検査
		if( DXA->HeadV5.Version > DXAVER_VER5 ) goto ERR ;

		// バージョンが４以上かどうかで読み込む残りのヘッダサイズを変更
		if( DXA->HeadV5.Version >= 0x0004 )
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER4_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			switch( DXA->HeadV5.CodePage )
			{
			case 949 : DXA->CharSet = DX_CHARSET_HANGEUL ; break ;
			case 950 : DXA->CharSet = DX_CHARSET_BIG5 ;    break ;
			case 936 : DXA->CharSet = DX_CHARSET_GB2312 ;  break ;
			case 932 : DXA->CharSet = DX_CHARSET_SHFTJIS ; break ;
			default :  DXA->CharSet = DX_CHARSET_DEFAULT ; break ;
			}
		}
		else
		{
			DXA_KeyConvFileRead( ( BYTE * )&DXA->HeadV5 + DXARC_ID_AND_VERSION_SIZE, DXARC_HEAD_VER3_SIZE - DXARC_ID_AND_VERSION_SIZE, DXA->WinFilePointer__, DXA->Key, DXARC_ID_AND_VERSION_SIZE ) ;
			DXA->HeadV5.CodePage = 0 ;
			DXA->CharSet = DX_CHARSET_DEFAULT ;
		}

		// 情報テーブルのサイズ分のメモリを確保する
		DXA->Table.Top = ( BYTE * )DXALLOC( DXA->HeadV5.HeadSize ) ;
		if( DXA->Table.Top == NULL ) goto ERR ;
		
		// 情報テーブルをメモリに読み込む
		ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->HeadV5.FileNameTableStartAddress, SEEK_SET ) ;
		if( DXA->HeadV5.Version >= 0x0005 )
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->WinFilePointer__, DXA->Key, 0 ) ;
		}
		else
		{
			DXA_KeyConvFileRead( DXA->Table.Top, DXA->HeadV5.HeadSize, DXA->WinFilePointer__, DXA->Key ) ;
		}

		// 情報テーブルのアドレスをセットする
		DXA->Table.NameTable		= DXA->Table.Top ;
		DXA->Table.FileTable		= DXA->Table.NameTable + DXA->HeadV5.FileTableStartAddress ;
		DXA->Table.DirectoryTable	= DXA->Table.NameTable + DXA->HeadV5.DirectoryTableStartAddress ;

		// カレントディレクトリのセット
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;
	}

	DXA->MemoryOpenFlag					= FALSE ;			// メモリイメージから開いているフラグを倒す
	DXA->UserMemoryImageFlag			= FALSE ;			// ユーザーのイメージから開いたわけではないのでフラグを倒す
	DXA->MemoryImageCopyFlag			= FALSE ;			// メモリイメージをコピーして使用しているフラグを倒す
	DXA->MemoryImageReadOnlyFlag        = FALSE ;			// メモリイメージを読み取り専用にするかどうかのフラグを倒す

	// 終了
	return 0 ;

ERR :
	if( DXA->WinFilePointer__ != 0 )
	{
		ReadOnlyFileAccessClose( DXA->WinFilePointer__ ) ;
		DXA->WinFilePointer__ = 0 ;
	}

	if( DXA->Table.Top != NULL )
	{
		DXFREE( DXA->Table.Top ) ;
		DXA->Table.Top = NULL ;
	}
	
	// 終了
	return -1 ;
}

// アーカイブファイルを閉じる
extern int DXA_CloseArchive( DXARC *DXA )
{
	// 既に閉じていたら何もせず終了
	if( DXA->WinFilePointer__ == 0 && DXA->MemoryImage == NULL ) return 0 ;

	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// メモリから開いているかどうかで処理を分岐
	if( DXA->MemoryOpenFlag == TRUE )
	{
		// アーカイブプログラムがメモリに読み込んだ場合とそうでない場合で処理を分岐
		if( DXA->UserMemoryImageFlag == TRUE )
		{
			// ユーザーから渡されたデータだった場合

			// アーカイブイメージをコピーして使用していた場合は単純に解放する
			if( DXA->MemoryImageCopyFlag )
			{
				if( DXA->MemoryImage != NULL )
				{
					DXFREE( DXA->MemoryImage ) ;
					DXA->MemoryImage = NULL ;
				}
			}
			else
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// 読み取り専用の場合は情報テーブル用に確保していたメモリを解放
					if( DXA->Table.Top != NULL )
					{
						DXFREE( DXA->Table.Top ) ;
						DXA->Table.Top = NULL ;
					}
				}
				else
				{
					// 渡されたメモリアドレス先の内容を直接使用していた場合は
					// 反転したデータを元に戻す
					if( DXA->V5Flag == FALSE )
					{
						DXA_DirectoryKeyConv( DXA, ( DXARC_DIRECTORY * )DXA->Table.DirectoryTable ) ;
						DXA_KeyConv( DXA->Table.Top, DXA->Head.HeadSize, 0, DXA->Key ) ;
					}
					else
					{
						if( DXA->HeadV5.Version >= 0x0005 )
						{
							DXA_DirectoryKeyConvV5( DXA, ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ) ;
							DXA_KeyConv( DXA->Table.Top, DXA->HeadV5.HeadSize, 0, DXA->Key ) ;
						}
						else
						{
							DXA_KeyConv( DXA->MemoryImage, DXA->MemoryImageSize, 0, DXA->Key ) ;
						}
					}
				}
			}
		}
		else
		{
			// アーカイブプログラムがメモリに読み込んだ場合

			// 確保していたメモリを開放する
			DXFREE( DXA->Table.Top ) ;
			DXFREE( DXA->MemoryImage ) ;
		}
	}
	else
	{
		// アーカイブファイルを閉じる
		ReadOnlyFileAccessClose( DXA->WinFilePointer__ ) ;

		// 情報テーブルを格納していたメモリ領域も解放
		DXFREE( DXA->Table.Top ) ;
	}

	// 初期化
	_MEMSET( DXA, 0, sizeof( DXARC ) ) ;

	// 終了
	return 0 ;
}

// アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
static int DXA_ChangeCurrentDirectoryFast( DXARC *DXA, DXARC_SEARCHDATA *SearchData )
{
	int i, j, k, Num ;
	BYTE *NameData, *PathData ;
	WORD PackNum, Parity ;
	DWORD FileHeadSize ;

	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	PackNum  = SearchData->PackNum ;
	Parity   = SearchData->Parity ;
	PathData = SearchData->FileName ;

	// カレントディレクトリから同名のディレクトリを探す
	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		FileH = ( DXARC_FILEHEAD_VER5 * )( DXA->Table.FileTable + DXA->CurrentDirectoryV5->FileHeadAddress ) ;
		Num = (int)DXA->CurrentDirectoryV5->FileHeadNum ;
		FileHeadSize = DXA->HeadV5.Version >= 0x0002 ? DXARC_FILEHEAD_VER2_SIZE : DXARC_FILEHEAD_VER1_SIZE ;
		for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD_VER5 *)( (BYTE *)FileH + FileHeadSize ) )
		{
			// ディレクトリチェック
			if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) continue ;

			// 文字列数とパリティチェック
			NameData = DXA->Table.NameTable + FileH->NameAddress ;
			if( PackNum != ((WORD *)NameData)[0] || Parity != ((WORD *)NameData)[1] ) continue ;

			// 文字列チェック
			NameData += 4 ;
			for( j = 0, k = 0 ; j < PackNum ; j ++, k += 4 )
				if( *((DWORD *)&PathData[k]) != *((DWORD *)&NameData[k]) ) break ;

			// 適合したディレクトリがあったらここで終了
			if( PackNum == j ) break ;
		}

		// 無かったらエラー
		if( i == Num ) return -1 ;

		// 在ったらカレントディレクトリを変更
		DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + FileH->DataAddress ) ;
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		FileH = ( DXARC_FILEHEAD * )( DXA->Table.FileTable + DXA->CurrentDirectory->FileHeadAddress ) ;
		Num = (int)DXA->CurrentDirectory->FileHeadNum ;
		FileHeadSize = DXARC_FILEHEAD_VER6_SIZE ;
		for( i = 0 ; i < Num ; i ++, FileH = (DXARC_FILEHEAD *)( (BYTE *)FileH + FileHeadSize ) )
		{
			// ディレクトリチェック
			if( ( FileH->Attributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 ) continue ;

			// 文字列数とパリティチェック
			NameData = DXA->Table.NameTable + FileH->NameAddress ;
			if( PackNum != ((WORD *)NameData)[0] || Parity != ((WORD *)NameData)[1] ) continue ;

			// 文字列チェック
			NameData += 4 ;
			for( j = 0, k = 0 ; j < PackNum ; j ++, k += 4 )
				if( *((DWORD *)&PathData[k]) != *((DWORD *)&NameData[k]) ) break ;

			// 適合したディレクトリがあったらここで終了
			if( PackNum == j ) break ;
		}

		// 無かったらエラー
		if( i == Num ) return -1 ;

		// 在ったらカレントディレクトリを変更
		DXA->CurrentDirectory = ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + FileH->DataAddress ) ;
	}

	// 正常終了
	return 0 ;
}

// アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
extern int DXA_ChangeCurrentDir( DXARC *DXA, const char *DirPath )
{
	return DXA_ChangeCurrentDirectoryBase( DXA, DirPath, true ) ;
}

// アーカイブ内のディレクトリパスを変更する( 0:成功  -1:失敗 )
static int DXA_ChangeCurrentDirectoryBase( DXARC *DXA, const char *DirectoryPath, bool ErrorIsDirectoryReset, DXARC_SEARCHDATA *LastSearchData )
{
	DXARC_DIRECTORY *OldDir ;
	DXARC_SEARCHDATA SearchData ;

	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// ここに留まるパスだったら無視
	if( _STRCMP( DirectoryPath, "." ) == 0 ) return 0 ;

	// 『\ or /』だけの場合はルートディレクトリに戻る
	if( _STRCMP( DirectoryPath, "\\" ) == 0 || _STRCMP( DirectoryPath, "/" ) == 0 )
	{
		if( DXA->V5Flag )
		{
			DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )DXA->Table.DirectoryTable ;
		}
		else
		{
			DXA->CurrentDirectory   = ( DXARC_DIRECTORY      * )DXA->Table.DirectoryTable ;
		}
		return 0 ;
	}

	// 下に一つ下がるパスだったら処理を分岐
	if( _STRCMP( DirectoryPath, ".." ) == 0 )
	{
		if( DXA->V5Flag )
		{
			// ルートディレクトリに居たらエラー
			if( DXA->CurrentDirectoryV5->ParentDirectoryAddress == 0xffffffff ) return -1 ;
			
			// 親ディレクトリがあったらそちらに移る
			DXA->CurrentDirectoryV5 = ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + DXA->CurrentDirectoryV5->ParentDirectoryAddress ) ;
		}
		else
		{
			// ルートディレクトリに居たらエラー
			if( DXA->CurrentDirectory->ParentDirectoryAddress == NONE_PAL ) return -1 ;
			
			// 親ディレクトリがあったらそちらに移る
			DXA->CurrentDirectory   = ( DXARC_DIRECTORY      * )( DXA->Table.DirectoryTable + DXA->CurrentDirectory->ParentDirectoryAddress ) ;
		}
		return 0 ;
	}

	// それ以外の場合は指定の名前のディレクトリを探す
	
	// 変更以前のディレクトリを保存しておく
	OldDir = DXA->CurrentDirectory ;

	// パス中に『\』があるかどうかで処理を分岐
	if( _STRCHR( DirectoryPath, '\\' ) == NULL && _STRCHR( DirectoryPath, '/' ) == NULL )
	{
		// ファイル名を検索専用の形式に変換する
		DXA_ConvSearchData( DXA, &SearchData, DirectoryPath, NULL ) ;

		// ディレクトリを変更
		if( DXA_ChangeCurrentDirectoryFast( DXA, &SearchData ) < 0 ) goto ERR ;
	}
	else
	{
		// \ or / がある場合は繋がったディレクトリを一つづつ変更してゆく
		int Point, StrLength ;

		Point = 0 ;
		// ループ
		for(;;)
		{
			// 文字列を取得する
			DXA_ConvSearchData( DXA, &SearchData, &DirectoryPath[Point], &StrLength ) ;
			Point += StrLength ;

			// もし初っ端が \ or / だった場合はルートディレクトリに落とす
			if( StrLength == 0 && ( DirectoryPath[Point] == '\\' || DirectoryPath[Point] == '/' ) )
			{
				DXA_ChangeCurrentDirectoryBase( DXA, "\\", false ) ;
			}
			else
			{
				// それ以外の場合は普通にディレクトリ変更
				if( DXA_ChangeCurrentDirectoryFast( DXA, &SearchData ) < 0 )
				{
					// エラーが起きて、更にエラーが起きた時に元のディレクトリに戻せの
					// フラグが立っている場合は元のディレクトリに戻す
					if( ErrorIsDirectoryReset == true ) DXA->CurrentDirectory = OldDir ;

					// エラー終了
					goto ERR ;
				}
			}

			// もし終端文字で終了した場合はループから抜ける
			// 又はあと \ or / しかない場合もループから抜ける
			if( DirectoryPath[Point] == '\0' ||
				( ( DirectoryPath[Point] == '\\' && DirectoryPath[Point+1] == '\0' ) ||
				  ( DirectoryPath[Point] == '/' && DirectoryPath[Point+1] == '\0' ) ) ) break ;
			Point ++ ;
		}
	}

	if( LastSearchData != NULL )
	{
		_MEMCPY( LastSearchData->FileName, SearchData.FileName, SearchData.PackNum * 4 ) ;
		LastSearchData->Parity  = SearchData.Parity ;
		LastSearchData->PackNum = SearchData.PackNum ;
	}

	// 正常終了
	return 0 ;

ERR:
	if( LastSearchData != NULL )
	{
		_MEMCPY( LastSearchData->FileName, SearchData.FileName, SearchData.PackNum * 4 ) ;
		LastSearchData->Parity  = SearchData.Parity ;
		LastSearchData->PackNum = SearchData.PackNum ;
	}

	// エラー終了
	return -1 ;
}
		
// アーカイブ内のカレントディレクトリパスを取得する
extern int DXA_GetCurrentDir( DXARC *DXA, char *DirPathBuffer, int BufferSize )
{
	char DirPath[256] ;
	int Depth, i ;

	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	if( DXA->V5Flag )
	{
		DXARC_DIRECTORY_VER5 *Dir[200], *DirTempP ;

		// ルートディレクトリに着くまで検索する
		Depth = 0 ;
		DirTempP = DXA->CurrentDirectoryV5 ;
		while( DirTempP->DirectoryAddress != 0xffffffff && DirTempP->DirectoryAddress != 0 )
		{
			Dir[Depth] = DirTempP ;
			DirTempP = ( DXARC_DIRECTORY_VER5 * )( DXA->Table.DirectoryTable + DirTempP->ParentDirectoryAddress ) ;
			Depth ++ ;
		}
		
		// パス名を連結する
		DirPath[0] = '\0' ;
		for( i = Depth - 1 ; i >= 0 ; i -- )
		{
			_STRCAT( DirPath, "\\" ) ;
			_STRCAT( DirPath, (char *)DXA->Table.NameTable + ((DXARC_FILEHEAD_VER5 *)( DXA->Table.FileTable + Dir[i]->DirectoryAddress ))->NameAddress ) ;
		}
	}
	else
	{
		DXARC_DIRECTORY *Dir[200], *DirTempP ;

		// ルートディレクトリに着くまで検索する
		Depth = 0 ;
		DirTempP = DXA->CurrentDirectory ;
		while( DirTempP->DirectoryAddress != 0xffffffff && DirTempP->DirectoryAddress != 0 )
		{
			Dir[Depth] = DirTempP ;
			DirTempP = ( DXARC_DIRECTORY * )( DXA->Table.DirectoryTable + DirTempP->ParentDirectoryAddress ) ;
			Depth ++ ;
		}
		
		// パス名を連結する
		DirPath[0] = '\0' ;
		for( i = Depth - 1 ; i >= 0 ; i -- )
		{
			_STRCAT( DirPath, "\\" ) ;
			_STRCAT( DirPath, (char *)DXA->Table.NameTable + ((DXARC_FILEHEAD *)( DXA->Table.FileTable + Dir[i]->DirectoryAddress ))->NameAddress ) ;
		}
	}

	// バッファの長さが０か、長さが足りないときはディレクトリ名の長さを返す
	if( BufferSize == 0 || BufferSize < (int)lstrlenA( DirPath ) )
	{
		return lstrlenA( DirPath ) + 1 ;
	}
	else
	{
		// ディレクトリ名をバッファに転送する
		_STRCPY( DirPathBuffer, DirPath ) ;
	}

	// 終了
	return 0 ;
}

// アーカイブ内のオブジェクトを検索する( -1:エラー -1以外:DXA検索ハンドル )
extern DWORD_PTR DXA_FindFirst( DXARC *DXA, const char *FilePath, FILEINFO *Buffer )
{
	DXA_FINDDATA *find;
	char Dir[256], Name[256];

	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// メモリの確保
	find = (DXA_FINDDATA *)DXALLOC( sizeof( *find ) );
	if( find == NULL ) return ( DWORD_PTR )-1;

	find->Container = DXA;
	DXA_DIR_AnalysisFileNameAndDirPath( DXA, FilePath, Name, Dir );

	// 全て大文字にする
	_STRUPR( Dir );
	_STRUPR( Name );

	// 検索対象のディレクトリを取得
	if( Dir[0] == '\0' )
	{
		find->Directory = DXA->CurrentDirectory;
	}
	else
	{
		DXARC_DIRECTORY *OldDir;

		OldDir = DXA->CurrentDirectory;

		// 指定のディレクトリが無い場合はエラー
		if( DXA_ChangeCurrentDirectoryBase( DXA, Dir, false ) == -1 )
		{
			DXFREE( find );
			return ( DWORD_PTR )-1;
		}

		find->Directory = DXA->CurrentDirectory;
		DXA->CurrentDirectory = OldDir;
	}

	find->ObjectCount = 0;
	_STRCPY( find->SearchStr, Name );

	// 適合する最初のファイルを検索する
	if( DXA_FindProcess( find, Buffer ) == -1 )
	{
		DXFREE( find );
		return ( DWORD_PTR )-1;
	}
	find->ObjectCount ++ ;

	// 返回句柄
	return (DWORD_PTR)find;
}

// アーカイブ内のオブジェクトを検索する( -1:エラー 0:成功 )
extern int DXA_FindNext( DWORD_PTR DxaFindHandle, FILEINFO *Buffer )
{
	DXA_FINDDATA *find;

	find = (DXA_FINDDATA *)DxaFindHandle;
	if( DXA_FindProcess( find, Buffer ) == -1 ) return -1;
	find->ObjectCount ++ ;

	return 0;
}

// アーカイブ内のオブジェクト検索を終了する
extern int DXA_FindClose( DWORD_PTR DxaFindHandle )
{
	DXA_FINDDATA *find;

	find = (DXA_FINDDATA *)DxaFindHandle;
	DXFREE( find );

	return 0;
}

// アーカイブファイル中の指定のファイルをメモリに読み込む( -1:エラー 0以上:ファイルサイズ )
extern int DXA_LoadFile( DXARC *DXA, const char *FilePath, void *Buffer, ULONGLONG BufferSize )
{
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		// 指定のファイルの情報を得る
		FileH = DXA_GetFileInfoV5( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// ファイルサイズが足りているか調べる、足りていないか、バッファ、又はサイズが０だったらサイズを返す
		if( BufferSize < FileH->DataSize || BufferSize == 0 || Buffer == NULL )
		{
			return ( int )FileH->DataSize ;
		}
		
		// 足りている場合はバッファーに読み込む

		// ファイルが圧縮されているかどうかで処理を分岐
		if( DXA->HeadV5.Version >= 0x0002 && FileH->PressDataSize != 0xffffffff )
		{
			// 圧縮されている場合

			// メモリ上に読み込んでいるかどうかで処理を分岐
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					void *temp ;

					// 圧縮データをメモリに読み込んでから解凍する

					// 圧縮データが収まるメモリ領域の確保
					temp = DXALLOC( FileH->PressDataSize ) ;

					// 圧縮データの転送
					_MEMCPY( temp, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->DataSize ) ;
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( temp, FileH->PressDataSize,                                   FileH->DataSize, DXA->Key ) ;
					}
					else
					{
						DXA_KeyConv( temp, FileH->PressDataSize, DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXA->Key ) ;
					}
					
					// 解凍
					DXA_Decode( temp, Buffer ) ;
					
					// メモリの解放
					DXFREE( temp ) ;
				}
				else
				{
					// メモリ上の圧縮データを解凍する
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, Buffer ) ;
				}
			}
			else
			{
				void *temp ;

				// 圧縮データをメモリに読み込んでから解凍する

				// 圧縮データが収まるメモリ領域の確保
				temp = DXALLOC( FileH->PressDataSize ) ;

				// 圧縮データの読み込み
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->HeadV5.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					DXA_KeyConvFileRead( temp, FileH->PressDataSize, DXA->WinFilePointer__, DXA->Key, FileH->DataSize ) ;
				}
				else
				{
					DXA_KeyConvFileRead( temp, FileH->PressDataSize, DXA->WinFilePointer__, DXA->Key ) ;
				}
				
				// 解凍
				DXA_Decode( temp, Buffer ) ;
				
				// メモリの解放
				DXFREE( temp ) ;
			}
		}
		else
		{
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// ファイルポインタを移動
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->DataSize ) ;

					// 读取
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( Buffer, FileH->DataSize,                                   FileH->DataSize, DXA->Key ) ;
					}
					else
					{
						DXA_KeyConv( Buffer, FileH->DataSize, DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXA->Key ) ;
					}
				}
				else
				{
					// コピー
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->DataSize ) ;
				}
			}
			else
			{
				// ファイルポインタを移動
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->HeadV5.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;

				// 读取
				if( DXA->HeadV5.Version >= 0x0005 )
				{
					DXA_KeyConvFileRead( Buffer, FileH->DataSize, DXA->WinFilePointer__, DXA->Key, FileH->DataSize ) ;
				}
				else
				{
					DXA_KeyConvFileRead( Buffer, FileH->DataSize, DXA->WinFilePointer__, DXA->Key ) ;
				}
			}
		}
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		// 指定のファイルの情報を得る
		FileH = DXA_GetFileInfo( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// ファイルサイズが足りているか調べる、足りていないか、バッファ、又はサイズが０だったらサイズを返す
		if( BufferSize < FileH->DataSize || BufferSize == 0 || Buffer == NULL )
		{
			return ( int )FileH->DataSize ;
		}
		
		// 足りている場合はバッファーに読み込む

		// ファイルが圧縮されているかどうかで処理を分岐
		if( FileH->PressDataSize != NONE_PAL )
		{
			// 圧縮されている場合

			// メモリ上に読み込んでいるかどうかで処理を分岐
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					void *temp ;

					// 圧縮データをメモリに読み込んでから解凍する

					// 圧縮データが収まるメモリ領域の確保
					temp = DXALLOC( ( size_t )FileH->PressDataSize ) ;

					// 圧縮データの読み込み
					_MEMCPY( temp, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;
					DXA_KeyConv( temp, FileH->DataSize, ( LONGLONG )FileH->DataSize, DXA->Key ) ;
					
					// 解凍
					DXA_Decode( temp, Buffer ) ;
					
					// メモリの解放
					DXFREE( temp ) ;
				}
				else
				{
					// メモリ上の圧縮データを解凍する
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, Buffer ) ;
				}
			}
			else
			{
				void *temp ;

				// 圧縮データをメモリに読み込んでから解凍する

				// 圧縮データが収まるメモリ領域の確保
				temp = DXALLOC( ( size_t )FileH->PressDataSize ) ;

				// 圧縮データの読み込み
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->Head.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;
				DXA_KeyConvFileRead( temp, FileH->PressDataSize, DXA->WinFilePointer__, DXA->Key, ( LONGLONG )FileH->DataSize ) ;
				
				// 解凍
				DXA_Decode( temp, Buffer ) ;
				
				// メモリの解放
				DXFREE( temp ) ;
			}
		}
		else
		{
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// コピー
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;

					DXA_KeyConv( Buffer, FileH->DataSize, FileH->DataSize, DXA->Key ) ;
				}
				else
				{
					// コピー
					_MEMCPY( Buffer, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;
				}
			}
			else
			{
				// ファイルポインタを移動
				ReadOnlyFileAccessSeek( DXA->WinFilePointer__, DXA->Head.DataStartAddress + FileH->DataAddress, SEEK_SET ) ;

				// 读取
				DXA_KeyConvFileRead( Buffer, FileH->DataSize, DXA->WinFilePointer__, DXA->Key, FileH->DataSize ) ;
			}
		}
	}
	
	// 終了
	return 0 ;
}

// アーカイブファイルをメモリに読み込んだ場合のファイルイメージが格納されている先頭アドレスを取得する( DXA_OpenArchiveFromFileUseMem 若しくは DXA_OpenArchiveFromMem で開いた場合に有効 )
extern void *DXA_GetFileImage( DXARC *DXA )
{
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// メモリイメージから開いていなかったらエラー
	if( DXA->MemoryOpenFlag == FALSE ) return NULL ;

	// 先頭アドレスを返す
	return DXA->MemoryImage ;
}

// アーカイブファイル中の指定のファイルのファイル内の位置とファイルの大きさを得る( -1:エラー )
extern int DXA_GetFileInfo( DXARC *DXA, const char *FilePath, int *Position, int *Size )
{
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		// 指定のファイルの情報を得る
		FileH = DXA_GetFileInfoV5( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// ファイルのデータがある位置とファイルサイズを保存する
		if( Position != NULL ) *Position = DXA->HeadV5.DataStartAddress + FileH->DataAddress ;
		if( Size     != NULL ) *Size     = FileH->DataSize ;
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		// 指定のファイルの情報を得る
		FileH = DXA_GetFileInfo( DXA, FilePath ) ;
		if( FileH == NULL ) return -1 ;

		// ファイルのデータがある位置とファイルサイズを保存する
		if( Position != NULL ) *Position = ( int )( DXA->Head.DataStartAddress + FileH->DataAddress ) ;
		if( Size     != NULL ) *Size     = ( int )( FileH->DataSize ) ;
	}

	// 成功終了
	return 0 ;
}



// アーカイブファイル内のファイルを開く(ファイル閉じる作業は必要なし)
extern int DXA_STREAM_Initialize( DXARC_STREAM *DXAStream, DXARC *DXA, const char *FilePath, int UseASyncReadFlag )
{
	// 非同期同期オープン中の場合はここで開き終わるのを待つ
	if( DXA->ASyncOpenFlag == TRUE )
	{
		while( DXA_CheckIdle( DXA ) == FALSE ) Sleep(0);
	}

	// データのセット
	DXAStream->Archive          = DXA ;
	DXAStream->EOFFlag          = FALSE ;
	DXAStream->FilePoint        = 0 ;
	DXAStream->DecodeDataBuffer = NULL ;
	DXAStream->DecodeTempBuffer = NULL ;
	DXAStream->UseASyncReadFlag = UseASyncReadFlag ;
	DXAStream->ASyncState       = DXARC_STREAM_ASYNCSTATE_IDLE ;

	// ファイルから開いている場合はアーカイブファイルのファイルポインタを作成
	if( DXA->MemoryOpenFlag == FALSE )
	{
		DXAStream->WinFilePointer = ReadOnlyFileAccessOpen( DXA->FilePath, FALSE, TRUE, FALSE ) ;
		if( DXAStream->WinFilePointer == 0 )
			return -1 ;
	}

	if( DXA->V5Flag )
	{
		DXARC_FILEHEAD_VER5 *FileH ;

		// 指定のファイルの情報を得る
		FileH = DXA_GetFileInfoV5( DXA, FilePath ) ;
		if( FileH == NULL )
		{
			if( DXA->MemoryOpenFlag == FALSE )
			{
				ReadOnlyFileAccessClose( DXAStream->WinFilePointer ) ;
				DXAStream->WinFilePointer = 0 ;
			}
			return -1 ;
		}

		// ファイル情報をセット
		DXAStream->FileHeadV5 = FileH ;

		// ファイルが圧縮されている場合はここで読み込んで解凍してしまう
		if( DXA->HeadV5.Version >= 0x0002 && FileH->PressDataSize != 0xffffffff )
		{
			// 解凍データが収まるメモリ領域の確保
			DXAStream->DecodeDataBuffer = DXALLOC( FileH->DataSize ) ;

			// メモリ上に読み込まれているかどうかで処理を分岐
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// 圧縮データが収まるメモリ領域の確保
					DXAStream->DecodeTempBuffer = DXALLOC( FileH->PressDataSize ) ;

					// 圧縮データの読み込み
					_MEMCPY( DXAStream->DecodeTempBuffer, (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, FileH->PressDataSize ) ;
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( DXAStream->DecodeTempBuffer, FileH->PressDataSize,                                   FileH->DataSize, DXA->Key ) ;
					}
					else
					{
						DXA_KeyConv( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXA->Key ) ;
					}

					// 解凍
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// メモリの解放
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
				else
				{
					// 解凍
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->HeadV5.DataStartAddress + FileH->DataAddress, DXAStream->DecodeDataBuffer ) ;
				}
			}
			else
			{
				// 圧縮データが収まるメモリ領域の確保
				DXAStream->DecodeTempBuffer = DXALLOC( FileH->PressDataSize ) ;

				// 圧縮データの読み込み
				DXAStream->ASyncReadFileAddress = DXA->HeadV5.DataStartAddress + FileH->DataAddress;
				ReadOnlyFileAccessSeek( DXAStream->WinFilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;

				// 非同期の場合は読み込みと鍵解除を別々に行う
				if( DXAStream->UseASyncReadFlag == TRUE )
				{
					// ファイルから読み込み
					ReadOnlyFileAccessRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, 1, DXAStream->WinFilePointer ) ;
					DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_PRESSREAD ;
				}
				else
				{
					if( DXA->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConvFileRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXAStream->WinFilePointer, DXA->Key, FileH->DataSize ) ;
					}
					else
					{
						DXA_KeyConvFileRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXAStream->WinFilePointer, DXA->Key ) ;
					}

					// 解凍
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// メモリの解放
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
			}
		}
	}
	else
	{
		DXARC_FILEHEAD *FileH ;

		// 指定のファイルの情報を得る
		FileH = DXA_GetFileInfo( DXA, FilePath ) ;
		if( FileH == NULL )
		{
			if( DXA->MemoryOpenFlag == FALSE )
			{
				ReadOnlyFileAccessClose( DXAStream->WinFilePointer ) ;
				DXAStream->WinFilePointer = 0 ;
			}
			return -1 ;
		}

		// ファイル情報をセット
		DXAStream->FileHead = FileH ;

		// ファイルが圧縮されている場合はここで読み込んで解凍してしまう
		if( FileH->PressDataSize != NONE_PAL )
		{
			// 解凍データが収まるメモリ領域の確保
			DXAStream->DecodeDataBuffer = DXALLOC( ( size_t )FileH->DataSize ) ;

			// メモリ上に読み込まれているかどうかで処理を分岐
			if( DXA->MemoryOpenFlag == TRUE )
			{
				if( DXA->MemoryImageReadOnlyFlag )
				{
					// 圧縮データが収まるメモリ領域の確保
					DXAStream->DecodeTempBuffer = DXALLOC( ( size_t )FileH->PressDataSize ) ;

					// 圧縮データの読み込み
					_MEMCPY( DXAStream->DecodeTempBuffer, (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, ( size_t )FileH->DataSize ) ;
					DXA_KeyConv( DXAStream->DecodeTempBuffer, FileH->PressDataSize, FileH->DataSize, DXA->Key ) ;

					// 解凍
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// メモリの解放
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
				else
				{
					// 解凍
					DXA_Decode( (BYTE *)DXA->MemoryImage + DXA->Head.DataStartAddress + FileH->DataAddress, DXAStream->DecodeDataBuffer ) ;
				}
			}
			else
			{
				// 圧縮データが収まるメモリ領域の確保
				DXAStream->DecodeTempBuffer = DXALLOC( ( size_t )FileH->PressDataSize ) ;

				// 圧縮データの読み込み
				DXAStream->ASyncReadFileAddress = DXA->Head.DataStartAddress + FileH->DataAddress;
				ReadOnlyFileAccessSeek( DXAStream->WinFilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;

				// 非同期の場合は読み込みと鍵解除を別々に行う
				if( DXAStream->UseASyncReadFlag == TRUE )
				{
					// ファイルから読み込み
					ReadOnlyFileAccessRead( DXAStream->DecodeTempBuffer, ( size_t )FileH->PressDataSize, 1, DXAStream->WinFilePointer ) ;
					DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_PRESSREAD ;
				}
				else
				{
					DXA_KeyConvFileRead( DXAStream->DecodeTempBuffer, FileH->PressDataSize, DXAStream->WinFilePointer, DXA->Key, FileH->DataSize ) ;

					// 解凍
					DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
				
					// メモリの解放
					DXFREE( DXAStream->DecodeTempBuffer ) ;
					DXAStream->DecodeTempBuffer = NULL ;
				}
			}
		}
	}

	// 終了
	return 0 ;
}

// アーカイブファイル内のファイルを閉じる
extern int DXA_STREAM_Terminate( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	// メモリの解放
	if( DXAStream->DecodeDataBuffer != NULL )
	{
		DXFREE( DXAStream->DecodeDataBuffer ) ;
		DXAStream->DecodeDataBuffer = NULL ;
	}

	if( DXAStream->DecodeTempBuffer != NULL )
	{
		DXFREE( DXAStream->DecodeTempBuffer ) ;
		DXAStream->DecodeTempBuffer = NULL ;
	}

	// ファイルを閉じる
	if( DXAStream->Archive->MemoryOpenFlag == FALSE )
	{
		ReadOnlyFileAccessClose( DXAStream->WinFilePointer ) ;
		DXAStream->WinFilePointer = 0 ;
	}

	// ゼロ初期化
	_MEMSET( DXAStream, 0, sizeof( DXARC_STREAM ) ) ;

	// 終了
	return 0 ;
}

// ファイルの内容を読み込む
extern int DXA_STREAM_Read( DXARC_STREAM *DXAStream, void *Buffer, size_t ReadLength )
{
	size_t ReadSize ;
	ULONGLONG DataSize ;
	ULONGLONG DataStartAddress ;
	ULONGLONG DataAddress ;

	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	if( DXAStream->Archive->V5Flag )
	{
		DataSize = DXAStream->FileHeadV5->DataSize ;
		DataAddress = DXAStream->FileHeadV5->DataAddress ;
		DataStartAddress = DXAStream->Archive->HeadV5.DataStartAddress ;
	}
	else
	{
		DataSize = DXAStream->FileHead->DataSize ;
		DataAddress = DXAStream->FileHead->DataAddress ;
		DataStartAddress = DXAStream->Archive->Head.DataStartAddress ;
	}

	// EOF フラグが立っていたら０を返す
	if( DXAStream->EOFFlag == TRUE ) return 0 ;

	// EOF 検出
	if( DataSize == DXAStream->FilePoint )
	{
		DXAStream->EOFFlag = TRUE ;
		return 0 ;
	}

	// データを読み込む量を設定する
	ReadSize = ReadLength < DataSize - DXAStream->FilePoint ? ReadLength : ( size_t )( DataSize - DXAStream->FilePoint ) ;

	// データが圧縮されていたかどうかで処理を分岐
	if( DXAStream->DecodeDataBuffer != NULL )
	{
		// データをコピーする
		_MEMCPY( Buffer, (BYTE *)DXAStream->DecodeDataBuffer + DXAStream->FilePoint, ReadSize ) ;
	}
	else
	{
		// メモリ上にデータがあるかどうかで処理を分岐
		if( DXAStream->Archive->MemoryOpenFlag == TRUE )
		{
			// メモリ上にある場合

			// データをコピーする
			_MEMCPY( Buffer, (BYTE *)DXAStream->Archive->MemoryImage + DataStartAddress + DataAddress + DXAStream->FilePoint, ReadSize ) ;

			if( DXAStream->Archive->MemoryImageReadOnlyFlag )
			{
				if( DXAStream->Archive->V5Flag )
				{
					if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConv( Buffer, ( int )ReadSize,                       DataSize + DXAStream->FilePoint, DXAStream->Archive->Key ) ;
					}
					else
					{
						DXA_KeyConv( Buffer, ( int )ReadSize, DataStartAddress + DataAddress + DXAStream->FilePoint, DXAStream->Archive->Key ) ;
					}
				}
				else
				{
					DXA_KeyConv( Buffer, ( int )ReadSize,                       DataSize + DXAStream->FilePoint, DXAStream->Archive->Key ) ;
				}
			}
		}
		else
		{
			// ファイルから読み込んでいる場合

			// アーカイブファイルポインタと、仮想ファイルポインタが一致しているか調べる
			// 一致していなかったらアーカイブファイルポインタを移動する
			DXAStream->ASyncReadFileAddress = DataAddress + DataStartAddress + DXAStream->FilePoint ;
			if( ( ULONGLONG )ReadOnlyFileAccessTell( DXAStream->WinFilePointer ) != DXAStream->ASyncReadFileAddress )
			{
				ReadOnlyFileAccessSeek( DXAStream->WinFilePointer, DXAStream->ASyncReadFileAddress, SEEK_SET ) ;
			}

			// 非同期読み込みの場合と同期読み込みの場合で処理を分岐
			if( DXAStream->UseASyncReadFlag )
			{
				// ファイルから読み込み
				ReadOnlyFileAccessRead( Buffer, ReadSize, 1, DXAStream->WinFilePointer ) ;
				DXAStream->ReadBuffer = Buffer;
				DXAStream->ReadSize = ( int )ReadSize;
				DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_READ ;
			}
			else
			{
				// データを読み込む
				if( DXAStream->Archive->V5Flag )
				{
					if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
					{
						DXA_KeyConvFileRead( Buffer, ( int )ReadSize, DXAStream->WinFilePointer, DXAStream->Archive->Key, DataSize + DXAStream->FilePoint ) ;
					}
					else
					{
						DXA_KeyConvFileRead( Buffer, ( int )ReadSize, DXAStream->WinFilePointer, DXAStream->Archive->Key ) ;
					}
				}
				else
				{
					DXA_KeyConvFileRead( Buffer, ( int )ReadSize, DXAStream->WinFilePointer, DXAStream->Archive->Key, DataSize + DXAStream->FilePoint ) ;
				}
			}
		}
	}
	
	// EOF フラグを倒す
	DXAStream->EOFFlag = FALSE ;

	// 読み込んだ分だけファイルポインタを移動する
	DXAStream->FilePoint += ( int )ReadSize ;
	
	// 読み込んだ容量を返す
	return ( int )ReadSize ;
}
	
// ファイルポインタを変更する
extern	int DXA_STREAM_Seek( DXARC_STREAM *DXAStream, LONGLONG SeekPoint, int SeekMode )
{
	ULONGLONG DataSize ;

	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	if( DXAStream->Archive->V5Flag )
	{
		DataSize = DXAStream->FileHeadV5->DataSize ;
	}
	else
	{
		DataSize = DXAStream->FileHead->DataSize ;
	}

	// シークタイプによって処理を分岐
	switch( SeekMode )
	{
	case SEEK_SET : break ;		
	case SEEK_CUR : SeekPoint += ( LONGLONG )( DXAStream->FilePoint ) ; break ;
	case SEEK_END :	SeekPoint  = ( LONGLONG )( DataSize + SeekPoint ) ; break ;
	}
	
	// 補正
	if( SeekPoint > ( LONGLONG )DataSize ) SeekPoint = ( LONGLONG )DataSize ;
	if( SeekPoint < 0 ) SeekPoint = 0 ;
	
	// セット
	DXAStream->FilePoint = SeekPoint ;
	
	// EOFフラグを倒す
	DXAStream->EOFFlag = FALSE ;
	
	// 終了
	return 0 ;
}

// 現在のファイルポインタを得る
extern	LONGLONG DXA_STREAM_Tell( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	return ( LONGLONG )DXAStream->FilePoint ;
}

// ファイルの終端に来ているか、のフラグを得る
extern	int DXA_STREAM_Eof( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みで状態がまだ待機状態ではなかったら待機状態になるまで待つ
	if( DXAStream->UseASyncReadFlag == TRUE && DXAStream->ASyncState != DXARC_STREAM_ASYNCSTATE_IDLE )
	{
		while( DXA_STREAM_IdleCheck( DXAStream ) == FALSE ) Sleep(1);
	}

	return DXAStream->EOFFlag ? EOF : 0 ;
}

// 読み込み処理が完了しているかどうかを調べる
extern	int	DXA_STREAM_IdleCheck( DXARC_STREAM *DXAStream )
{
	// 非同期読み込みではない場合は何もせず TRUE を返す
	if( DXAStream->UseASyncReadFlag == FALSE ) return TRUE ;

	// 状態によって処理を分岐
	switch( DXAStream->ASyncState )
	{
	case DXARC_STREAM_ASYNCSTATE_IDLE:			// 待機状態
		return TRUE;

	case DXARC_STREAM_ASYNCSTATE_PRESSREAD:		// 圧縮データ読み込み待ち

		// 読み込み終了待ち
		if( ReadOnlyFileAccessIdleCheck( DXAStream->WinFilePointer ) == FALSE ) return FALSE;

		// 読み込み終わったらまず鍵を外す
		if( DXAStream->Archive->V5Flag )
		{
			if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
			{
				DXA_KeyConv( DXAStream->DecodeTempBuffer, DXAStream->FileHeadV5->PressDataSize, DXAStream->FileHeadV5->DataSize, DXAStream->Archive->Key ) ;
			}
			else
			{
				DXA_KeyConv( DXAStream->DecodeTempBuffer, DXAStream->FileHeadV5->PressDataSize, DXAStream->ASyncReadFileAddress, DXAStream->Archive->Key ) ;
			}
		}
		else
		{
			DXA_KeyConv( DXAStream->DecodeTempBuffer, DXAStream->FileHead->PressDataSize, DXAStream->FileHead->DataSize, DXAStream->Archive->Key ) ;
		}

		// 解凍
		DXA_Decode( DXAStream->DecodeTempBuffer, DXAStream->DecodeDataBuffer ) ;
	
		// メモリの解放
		DXFREE( DXAStream->DecodeTempBuffer ) ;
		DXAStream->DecodeTempBuffer = NULL ;

		// 状態を待機状態にする
		DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_IDLE;
		return TRUE;

	case DXARC_STREAM_ASYNCSTATE_READ:			// 読み込み待ち

		// 読み込み終了待ち
		if( ReadOnlyFileAccessIdleCheck( DXAStream->WinFilePointer ) == FALSE ) return FALSE;

		// 読み込み終わったら鍵を外す
		if( DXAStream->Archive->V5Flag )
		{
			if( DXAStream->Archive->HeadV5.Version >= 0x0005 )
			{
				DXA_KeyConv( DXAStream->ReadBuffer, DXAStream->ReadSize, DXAStream->FileHeadV5->DataSize + ( DXAStream->ASyncReadFileAddress - ( DXAStream->FileHeadV5->DataAddress + DXAStream->Archive->HeadV5.DataStartAddress ) ), DXAStream->Archive->Key ) ;
			}
			else
			{
				DXA_KeyConv( DXAStream->ReadBuffer, DXAStream->ReadSize, DXAStream->ASyncReadFileAddress, DXAStream->Archive->Key ) ;
			}
		}
		else
		{
			DXA_KeyConv( DXAStream->ReadBuffer, DXAStream->ReadSize, DXAStream->FileHead->DataSize + ( DXAStream->ASyncReadFileAddress - ( DXAStream->FileHead->DataAddress + DXAStream->Archive->Head.DataStartAddress ) ), DXAStream->Archive->Key ) ;
		}

		// 状態を待機状態にする
		DXAStream->ASyncState = DXARC_STREAM_ASYNCSTATE_IDLE;
		return TRUE;
	}

	return TRUE ;
}

// ファイルのサイズを取得する
extern	LONGLONG DXA_STREAM_Size( DXARC_STREAM *DXAStream )
{
	if( DXAStream->Archive->V5Flag )
	{
		return DXAStream->FileHeadV5->DataSize ;
	}
	else
	{
		return ( LONGLONG )DXAStream->FileHead->DataSize ;
	}
}







// フルパスではないパス文字列をフルパスに変換する
static int DXA_DIR_ConvertFullPath( const TCHAR *Src, TCHAR *Dest )
{
	int i, j, k ;
	TCHAR iden[256], CurrentDir[MAX_PATH] ;

	// カレントディレクトリを得る
	GetCurrentDirectory( MAX_PATH, CurrentDir ) ;
	_TSTRUPR( CurrentDir ) ;

	if( Src == NULL )
	{
		lstrcpy( Dest, CurrentDir ) ;
		goto END ;
	}

	i = 0 ;
	j = 0 ;
	k = 0 ;

	// 最初に『\』又は『/』が２回連続で続いている場合はネットワークを介していると判断
	if( ( Src[0] == _T( '\\' ) && Src[1] == _T( '\\' ) ) ||
		( Src[0] == _T( '/' )  && Src[1] == _T( '/'  ) ) )
	{
		Dest[0] = _T( '\\' ) ;
		Dest[1] = _T( '\0' ) ;

		i += 2;
		j ++ ;
	}
	else
	// 最初が『\』又は『/』の場合はカレントドライブのルートディレクトリまで落ちる
	if( Src[0] == _T( '\\' ) || Src[0] == _T( '/' ) )
	{
		Dest[0] = CurrentDir[0] ;
		Dest[1] = CurrentDir[1] ;
		Dest[2] = _T( '\0' ) ;

		i ++ ;
		j = 2 ;
	}
	else
	// ドライブ名が書かれていたらそのドライブへ
	if( Src[1] == _T( ':' ) )
	{
		Dest[0] = CHARUP(Src[0]) ;
		Dest[1] = Src[1] ;
		Dest[2] = _T( '\0' ) ;

		i = 2 ;
		j = 2 ;

		if( Src[i] == _T( '\\' ) ) i ++ ;
	}
	else
	// それ以外の場合はカレントディレクトリ
	{
		lstrcpy( Dest, CurrentDir ) ;
		j = lstrlen( Dest ) ;
		if( Dest[j-1] == _T( '\\' ) || Dest[j-1] == _T( '/' ) )
		{
			Dest[j-1] = _T( '\0' ) ;
			j -- ;
		}
	}

	for(;;)
	{
		switch( Src[i] )
		{
		case _T( '\0' ) :
			if( k != 0 )
			{
				Dest[j] = _T( '\\' ) ; j ++ ;
				lstrcpy( &Dest[j], iden ) ;
				j += k ;
				k = 0 ;
			}
			goto END ;

		case _T( '\\' ) :
		case _T( '/' ) :
			// 文字列が無かったらスキップ
			if( k == 0 )
			{
				i ++ ;
				break;
			}
			if( lstrcmp( iden, _T( "." ) ) == 0 )
			{
				// なにもしない
			}
			else
			if( lstrcmp( iden, _T( ".." ) ) == 0 )
			{
				// 一つ下のディレクトリへ
				j -- ;
//				while( Dest[j] != _T( '\\' ) && Dest[j] != _T( '/' ) && Dest[j] != _T( ':' ) ) j -- ;
				for(;;)
				{
					if( Dest[j] == _T( '\\' ) || Dest[j] == _T( '/' ) || Dest[j] == _T( ':' ) ) break ;
					j -= CheckDoubleChar( Dest, j - 1, _GET_CHARSET() ) == 2 ? 2 : 1 ;
				}
				if( Dest[j] != _T( ':' ) ) Dest[j] = _T( '\0' ) ;
				else j ++ ;
			}
			else
			{
				Dest[j] = _T( '\\' ) ; j ++ ;
				lstrcpy( &Dest[j], iden ) ;
				j += k ;
			}

			k = 0 ;
			i ++ ;
			break ;
		
		default :
			if( _TMULT( Src[i], _GET_CHARSET() ) == FALSE  )
			{
				iden[k] = CHARUP(Src[i]) ;
				iden[k+1] = _T( '\0' ) ; 
				k ++ ;
				i ++ ;
			}
			else
			{
				iden[k]   = Src[i] ;
				iden[k+1] = Src[i+1] ;
				iden[k+2] = _T( '\0' ) ;
				k += 2 ;
				i += 2 ;
			}
			break ;
		}
	}
	
END :
	// 正常終了
	return 0 ;
}

// ファイル名も一緒になっていると分かっているパス中からファイル名とディレクトリパスを分割する
// フルパスである必要は無い、ファイル名だけでも良い
// DirPath の終端に ￥ マークは付かない
static int DXA_DIR_AnalysisFileNameAndDirPath( DXARC *DXA, const char *Src, char *FileName, char *DirPath )
{
	int i, Last ;
	
	// ファイル名を抜き出す
	i = 0 ;
	Last = -1 ;
	while( Src[i] != '\0' )
	{
		if( CheckMultiByteChar( Src[i], DXA->CharSet ) == FALSE )
		{
			if( Src[i] == '\\' || Src[i] == '/' || Src[i] == '\0' || Src[i] == ':' ) Last = i ;
			i ++ ;
		}
		else
		{
			i += 2 ;
		}
	}
	if( FileName != NULL )
	{
		if( Last != -1 ) _STRCPY( FileName, &Src[Last+1] ) ;
		else             _STRCPY( FileName, Src ) ;
	}
	
	// ディレクトリパスを抜き出す
	if( DirPath != NULL )
	{
		if( Last != -1 )
		{
			_MEMCPY( DirPath, Src, Last ) ;
			DirPath[Last] = '\0' ;
		}
		else
		{
			DirPath[0] = '\0' ;
		}
	}
	
	// 終了
	return 0 ;
}

// CmpStr の条件に Src が適合するかどうかを調べる( 0:適合する  -1:適合しない )
static int DXA_DIR_FileNameCmp( DXARC *DXA, const char *Src, const char *CmpStr )
{
	const char *s, *c;

	s = Src;
	c = CmpStr;
	while( *c != '\0' || *s != '\0' )
	{
		if( CheckMultiByteChar( *c, DXA->CharSet ) == TRUE )
		{
			if( *((WORD *)s) != *((WORD *)c) ) return -1;
			c += 2;
			s += 2;
		}
		else
		{
			switch( *c )
			{
			case '?':
				c ++ ;
				s ++ ;
				break;

			case '*':
				while( *c == '*' ) c ++ ;
				if( *c == '\0' ) return 0;
				while( *s != '\0' && *s != *c ) s ++ ;
				if( *s == '\0' ) return -1;
				c ++ ;
				s ++ ;
				break;

			default:
				if( *c != *s ) return -1;
				c ++ ;
				s ++ ;
				break;
			}
		}
		if( ( *c == '\0' && *s != '\0' ) || ( *c != '\0' && *s == '\0' ) ) return -1;
	}

	return 0;
}

// アーカイブファイルをフォルダに見立ててファイルを開く時の情報を得る( -1:アーカイブとしては存在しなかった  0:存在した )
static int DXA_DIR_OpenTest( const TCHAR *FilePath, int *ArchiveIndex, char *ArchivePath, char *ArchiveFilePath )
{
	int i, len, arcindex ;
	TCHAR fullpath[256], path[256], temp[256], dir[256], *p ;

	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// 前回と使用するアーカイブのパスが同じ場合は同じアーカイブを使用する
	if( DXARCD.BackUseDirectoryPathLength != 0 && _TSTRNCMP( fullpath, DXARCD.BackUseDirectory, DXARCD.BackUseDirectoryPathLength ) == 0 &&
		( fullpath[ DXARCD.BackUseDirectoryPathLength ] == _T( '\\' ) || fullpath[ DXARCD.BackUseDirectoryPathLength ] == _T( '/' ) ) )
	{
		// 前回使用したＤＸＡファイルを開く
		arcindex = DXA_DIR_OpenArchive( DXARCD.BackUseDirectory, NULL, -1, FALSE, FALSE, DXARCD.BackUseArchiveIndex ) ;
		if( arcindex == -1 ) return -1 ;

		// ＤＸＡファイルがある場所以降のパスを作成する
		p = &fullpath[ DXARCD.BackUseDirectoryPathLength + 1 ] ;
	}
	else
	{
		// 前回とは違うパスの場合は一から調べる

		// ディレクトリを一つ一つ追って行く
		p = fullpath ;
		len = 0 ;
		for(;;)
		{
			// ネットワークを介していた場合の専用処理
			if( p - fullpath == 0 && fullpath[0] == _T( '\\' ) && fullpath[1] == _T( '\\' ) )
			{
				path[0] = _T( '\\' );
				path[1] = _T( '\\' );
				path[2] = _T( '\0' );
				len += 2;
				p += 2;
			}

			// ディレクトリを一つ取る
			for( i = 0 ; *p != _T( '\0' ) && *p !=  _T( '/' ) && *p != _T( '\\' ) ; )
			{
				if( _TMULT( *p, _GET_CHARSET() ) )
				{
					dir[i  ] = path[len+i  ] = p[ 0 ] ;
					dir[i+1] = path[len+i+1] = p[ 1 ] ;
					i += 2 ;
					p += 2 ;
				}
				else
				{
					dir[i] = path[len+i] = *p ;
					i ++ ;
					p ++ ;
				}
			}
			if( *p == _T( '\0' ) || i == 0 ) return -1;
			p ++ ;
			dir[i] = path[len+i] = _T( '\0' ) ;
			len += i ;

			// フォルダ名をDXアーカイブファイル名にする
			_MEMCPY( temp, path, len * 2 ) ;
			temp[len] = _T( '.' ) ;
			if( DXARCD.ArchiveExtensionLength == 0 )	_MEMCPY( &temp[len+1], _T( "DXA" ), 4 * sizeof( TCHAR ) ) ;
			else										_MEMCPY( &temp[len+1], DXARCD.ArchiveExtension, ( DXARCD.ArchiveExtensionLength + 1 ) * sizeof( TCHAR ) ) ;

			// ＤＸＡファイルとして開いてみる
			arcindex = DXA_DIR_OpenArchive( temp ) ;
			if( arcindex != -1 ) break ;

			// 開けなかったら次の階層へ
			path[len] = _T( '\\' ) ;
			len ++ ;
		}

		// 開けたら今回の情報を保存する
		if( DXARCD.ArchiveExtensionLength == 0 )
			_MEMCPY( DXARCD.BackUseDirectory, temp, ( len + 3 + 2 ) * sizeof( TCHAR ) ) ;
		else
			_MEMCPY( DXARCD.BackUseDirectory, temp, ( len + DXARCD.ArchiveExtensionLength + 2 ) * sizeof( TCHAR ) ) ;
		DXARCD.BackUseDirectoryPathLength = len ;
		DXARCD.BackUseArchiveIndex        = arcindex ;
	}

	// 情報をセットする
	*ArchiveIndex = arcindex;
#ifdef UNICODE
	int CodePage ;
	CodePage = DXARCD.Archive[ arcindex ]->Archive.HeadV5.CodePage ;
	if( CodePage == 0 ) CodePage = _GET_CODEPAGE() ;
	if( ArchivePath )		WCharToMBChar( CodePage, ( DXWCHAR * )DXARCD.BackUseDirectory, ArchivePath,     512 ) ;
	if( ArchiveFilePath )	WCharToMBChar( CodePage, ( DXWCHAR * )p,                       ArchiveFilePath, 512 ) ;
#else
	if( ArchivePath     ) lstrcpy( ArchivePath,     DXARCD.BackUseDirectory );
	if( ArchiveFilePath ) lstrcpy( ArchiveFilePath, p                       );
#endif

	// 終了
	return 0;
}

// アーカイブファイルを開く
static int DXA_DIR_OpenArchive( const TCHAR *FilePath, void *FileImage, int FileSize, int FileImageCopyFlag, int FileImageReadOnly, int ArchiveIndex, int OnMemory, int ASyncThread )
{
	int					i ;
	int					index ;
	int					newindex ;
	DXARC_DIR_ARCHIVE *	arc ;
	DXARC_DIR_ARCHIVE *	tarc ;
	DXARC				temparc ;

	// アーカイブの指定がある場合はそのまま使用する
	if( ArchiveIndex != -1 )
	{
		tarc = DXARCD.Archive[ ArchiveIndex ] ;
		if( tarc != NULL )
		{
			if(	lstrcmp( FilePath, tarc->Path ) == 0 )
			{
				DXARCD.Archive[ArchiveIndex]->UseCounter ++ ;
				return ArchiveIndex ;
			}
		}
	}

	// 既に開かれているか調べる
	newindex = -1 ;
	index    = 0 ;
	for( i = 0 ; i < DXARCD.ArchiveNum ; index ++ )
	{
		arc = DXARCD.Archive[index] ;
		if( arc == NULL )
		{
			newindex = index ;
			continue ;
		}
		
		i ++ ;

		if( lstrcmp( arc->Path, FilePath ) == 0 )
		{
			// 既に開かれていた場合はそのインデックスを返す
			arc->UseCounter ++ ;
			return index ;
		}
	}
	
	// なかった場合は新規にデータを追加する

	// ハンドルが既に一杯の場合は使用されていないアーカイブハンドルを解放する
	if( DXARCD.ArchiveNum == DXA_DIR_MAXARCHIVENUM )
	{
		// 未使用のハンドルを解放
		DXA_DIR_CloseWaitArchive() ;
		
		// それでも一杯である場合はエラー
		if( DXARCD.ArchiveNum == DXA_DIR_MAXARCHIVENUM )
			return -1 ;
	} 
	if( newindex == -1 )
	{
		for( newindex = 0 ; DXARCD.Archive[newindex] != NULL ; newindex ++ ){}
	}

	// アーカイブファイルが存在しているか確認がてら初期化する
	DXA_Initialize( &temparc ) ;
	if( FileImage != NULL )
	{
		// メモリ上に展開されたファイルイメージを使用する場合
		if( DXA_OpenArchiveFromMem( &temparc, FileImage, FileSize, FileImageCopyFlag, FileImageReadOnly, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL, FilePath ) < 0 )
			return -1 ;
	}
	else
	if( OnMemory == TRUE )
	{
		// メモリに読み込む場合
		if( DXA_OpenArchiveFromFileUseMem( &temparc, FilePath, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL, ASyncThread ) < 0 )
			return -1 ;
	}
	else
	{
		// ファイルから読み込む場合
		if( DXA_OpenArchiveFromFile( &temparc, FilePath, DXARCD.ValidKeyString == TRUE ? DXARCD.KeyString : NULL ) < 0 )
			return -1 ;
	}

	// 新しいアーカイブデータ用のメモリを確保する
	arc = DXARCD.Archive[ newindex ] = (DXARC_DIR_ARCHIVE *)DXALLOC( sizeof( DXARC_DIR_ARCHIVE ) ) ;
	if( DXARCD.Archive[ newindex ] == NULL )
	{
		DXA_CloseArchive( &temparc ) ;
		DXA_Terminate( &temparc ) ;
		return -1 ;
	}

	// 情報セット
	_MEMCPY( &arc->Archive, &temparc, sizeof( DXARC ) ) ;
	arc->UseCounter = 1 ;
	lstrcpy( arc->Path, FilePath ) ;

	// 使用中のアーカイブの数を増やす
	DXARCD.ArchiveNum ++ ;

	// インデックスを返す
	return newindex ;
}

// 既に開かれているアーカイブのハンドルを取得する( 戻り値: -1=無かった 0以上:ハンドル )
static int DXA_DIR_GetArchive( const TCHAR *FilePath, void *FileImage )
{
	int i, index ;
	DXARC_DIR_ARCHIVE *arc ;

	index = 0 ;
	for( i = 0 ; i < DXARCD.ArchiveNum ; index ++ )
	{
		arc = DXARCD.Archive[index] ;
		if( arc == NULL ) continue ;

		i ++ ;

		if( FilePath )
		{
			if( lstrcmp( arc->Path, FilePath ) == 0 )
				return index ;
		}
		else
		{
			if( arc->Archive.MemoryImageCopyFlag )
			{
				if( arc->Archive.MemoryImageOriginal == FileImage )
					return index ;
			}
			else
			{
				if( arc->Archive.MemoryImage == FileImage )
					return index ;
			}
		}
	}

	return -1 ;
}

// アーカイブファイルを閉じる
static int DXA_DIR_CloseArchive( int ArchiveHandle )
{
	DXARC_DIR_ARCHIVE *arc ;

	// 使用されていなかったら何もせず終了
	arc = DXARCD.Archive[ArchiveHandle] ;
	if( arc == NULL || arc->UseCounter == 0 ) return -1 ;

	// 参照カウンタを減らす
	arc->UseCounter -- ;

	// 終了
	return 0 ;
}

// 使用されるのを待っているアーカイブファイルを全て閉じる
static void DXA_DIR_CloseWaitArchive( void )
{
	int i, Num, index ;
	DXARC_DIR_ARCHIVE *arc ;
	
	Num = DXARCD.ArchiveNum ;
	for( i = 0, index = 0 ; i < Num ; index ++ )
	{
		if( DXARCD.Archive[index] == NULL ) continue ;
		i ++ ;

		arc = DXARCD.Archive[index] ;

		// 使われていたら解放しない
		if( arc->UseCounter > 0 ) continue ;

		// 後始末
		DXA_CloseArchive( &arc->Archive ) ;
		DXA_Terminate( &arc->Archive ) ;
		DXFREE( arc ) ;
		DXARCD.Archive[index] = NULL ;
		
		// アーカイブの数を減らす
		DXARCD.ArchiveNum -- ;
	}
}

// アーカイブをディレクトリに見立てる処理の初期化
extern	int DXA_DIR_Initialize( void )
{
	// 既に初期化済みの場合は何もしない
	if( DXARCD.InitializeFlag )
		return -1 ;

	// クリティカルセクションの初期化
	CriticalSection_Initialize( &DXARCD.CriticalSection ) ;

//	_MEMSET( &DXARCD, 0, sizeof( DXARC_DIR ) ) ;
//	DXA_DIR_Terminate() ;

	// 使用されていないアーカイブファイルを解放する
	DXA_DIR_CloseWaitArchive() ;

	// 初期化完了フラグを立てる
	DXARCD.InitializeFlag = TRUE ;

	// 終了
	return 0 ;
}


// アーカイブをディレクトリに見立てる処理の後始末
extern int DXA_DIR_Terminate( void )
{
	// 既に後始末済みの場合は何もしない
	if( DXARCD.InitializeFlag == FALSE )
		return -1 ;

	// 使用されていないアーカイブファイルを解放する
	DXA_DIR_CloseWaitArchive() ;

	// クリティカルセクションの後始末
	CriticalSection_Delete( &DXARCD.CriticalSection ) ;

	// 初期化完了フラグを倒す
	DXARCD.InitializeFlag = FALSE ;

	// 終了
	return 0 ;
}

// アーカイブファイルの拡張子を設定する
extern int DXA_DIR_SetArchiveExtension( const TCHAR *Extension )
{
	int Length ;
	
	Length = lstrlen( Extension ) ;

	if( Length >= 64 || Extension == NULL || Extension[0] == _T( '\0' ) )
	{
		DXARCD.ArchiveExtension[0] = 0 ;
		DXARCD.ArchiveExtensionLength = 0 ;
	}
	else
	{
		DXARCD.ArchiveExtensionLength = Length ;
		lstrcpy( DXARCD.ArchiveExtension, Extension ) ;
	}

	// 終了
	return 0 ;
}

// アーカイブファイルと通常のフォルダのどちらも存在した場合、どちらを優先させるかを設定する( 1:フォルダを優先 0:ＤＸアーカイブファイルを優先(デフォルト) )
extern int DXA_DIR_SetDXArchivePriority( int Priority )
{
	DXARCD.DXAPriority = Priority ;

	// 終了
	return 0 ;
}

// アーカイブファイルの鍵文字列を設定する
extern int DXA_DIR_SetKeyString( const char *KeyString )
{
	if( KeyString == NULL )
	{
		DXARCD.ValidKeyString = FALSE ;
	}
	else
	{
		DXARCD.ValidKeyString = TRUE ;
		if( lstrlenA( KeyString ) > DXA_KEYSTR_LENGTH )
		{
			_MEMCPY( DXARCD.KeyString, KeyString, DXA_KEYSTR_LENGTH ) ;
			DXARCD.KeyString[ DXA_KEYSTR_LENGTH ] = '\0' ;
		}
		else
		{
			_STRCPY( DXARCD.KeyString, KeyString ) ;
		}
	}

	// 終了
	return 0 ;
}

// ファイルを丸ごと読み込む関数
extern LONGLONG DXA_DIR_LoadFile( const TCHAR *FilePath, void *Buffer, int BufferSize )
{
	LONGLONG siz ;
	DWORD_PTR handle ;

	handle = DXA_DIR_Open( FilePath ) ;
	if( handle == -1 ) return false ;

	DXA_DIR_Seek( handle, 0L, SEEK_END ) ;
	siz = DXA_DIR_Tell( handle ) ;
	DXA_DIR_Seek( handle, 0L, SEEK_SET ) ;

	if( siz <= BufferSize )
	{
		DXA_DIR_Read( Buffer, ( size_t )siz, 1, handle ) ;
	}

	DXA_DIR_Close( handle ) ;

	// 終了
	return siz ;
}

// DXA_DIR_Open の基本関数
extern DWORD_PTR DXA_DIR_Open( const TCHAR *FilePath, int UseCacheFlag, int BlockReadFlag, int UseASyncReadFlag )
{
	int index ;
	DXARC_DIR_FILE *file ;
	char DXAErrorStr[256], DxaInFilePath[256] ;

	// 初期化されていなかったら初期化する
	if( DXARCD.InitializeFlag == FALSE )
	{
		DXA_DIR_Initialize() ;
	}

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &DXARCD.CriticalSection ) ;

	UseCacheFlag  = UseCacheFlag ;
	BlockReadFlag = BlockReadFlag ;
	DXAErrorStr[0] = 0 ;

	// 空きデータを探す
	if( DXARCD.FileNum == DXA_DIR_MAXFILENUM )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

		DXST_ERRORLOG_ADD( _T( "同時に開けるファイルの数が限界を超えました\n" ) ) ;
		return 0 ;
	}
	for( index = 0 ; DXARCD.File[index] != NULL ; index ++ ){}

	// メモリの確保
	DXARCD.File[index] = (DXARC_DIR_FILE *)DXALLOC( sizeof( DXARC_DIR_FILE ) ) ;
	if( DXARCD.File[index] == NULL )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

		DXST_ERRORLOG_ADD( _T( "ファイルの情報を格納するメモリの確保に失敗しました\n" ) ) ;
		return 0 ;
	}
	file = DXARCD.File[index] ;

	// アーカイブファイルとフォルダのどちらを優先するかで処理を分岐
	if( DXARCD.DXAPriority == 0 )
	{
		// アーカイブを優先する場合

		// アーカイブファイルが無いか調べる
		if( DXA_DIR_OpenTest( FilePath, (int *)&file->UseArchiveIndex, NULL, DxaInFilePath ) == 0 )
		{
			// アーカイブを使用しているフラグを立てる
			file->UseArchiveFlag = 1 ;

			// ディレクトリ名と同名のＤＸＡファイルを開けたらその中から指定のファイルを読み込もうとする
			if( DXA_STREAM_Initialize( &file->DXAStream, &DXARCD.Archive[ file->UseArchiveIndex ]->Archive, DxaInFilePath, UseASyncReadFlag ) < 0 )
			{
				_STRCPY( DXAErrorStr, DXSTRING( "ＤＸＡファイルの中に指定のファイルは見つかりませんでした\n" ) ) ;
				DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;
				goto ERR ;
			}
		}
		else
		{
			// アーカイブファイルが無かったら普通のファイルから読み込む設定を行う
			file->UseArchiveFlag = 0 ;

			// 普通のファイルが無いか調べる
			file->WinFilePointer_ = ReadOnlyFileAccessOpen( FilePath, UseCacheFlag, TRUE, UseASyncReadFlag ) ;
			if( file->WinFilePointer_ == 0 ) goto ERR ;
		}
	}
	else
	{
		// 普通のファイルを優先する場合

		// 普通のファイルが無いか調べる
		if( ( file->WinFilePointer_ = ReadOnlyFileAccessOpen( FilePath, UseCacheFlag, TRUE, UseASyncReadFlag ) ) != 0 )
		{
			// 開いたら普通のファイルから読み込む設定を行う
			file->UseArchiveFlag = 0 ;
		}
		else
		{
			// アーカイブファイルが無いか調べる
			if( DXA_DIR_OpenTest( FilePath, (int *)&file->UseArchiveIndex, NULL, DxaInFilePath ) == 0 )
			{
				// アーカイブを使用しているフラグを立てる
				file->UseArchiveFlag = 1 ;

				// ディレクトリ名と同名のＤＸＡファイルを開けたらその中から指定のファイルを読み込もうとする
				if( DXA_STREAM_Initialize( &file->DXAStream, &DXARCD.Archive[ file->UseArchiveIndex ]->Archive, DxaInFilePath, UseASyncReadFlag ) < 0 )
				{
					_STRCPY( DXAErrorStr, DXSTRING( "ＤＸＡファイルの中に指定のファイルは見つかりませんでした\n" ) ) ;
					DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;
					goto ERR ;
				}
			}
			else
			{
				// 無かったらエラー
				goto ERR;
			}
		}
	}

	// ハンドルの数を増やす
	DXARCD.FileNum ++ ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

	// インデックスを返す
	return index | 0xF0000000 ;

ERR:
	// メモリの解放
	if( DXARCD.File[index] != NULL ) DXFREE( DXARCD.File[index] ) ;
	DXARCD.File[index] = NULL ;
	
	// エラー文字列出力
//	DXST_ERRORLOGFMT_ADD(( "ファイル %s のオープンに失敗しました\n", FilePath )) ;
//	if( DXAErrorStr[0] != '\0' ) DXST_ERRORLOGFMT_ADD(( "ＤＸＡエラー：%s", DXAErrorStr )) ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

	// エラー終了
	return 0 ;
}

// ファイルを閉じる
extern int DXA_DIR_Close( DWORD_PTR Handle )
{
	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &DXARCD.CriticalSection ) ;

	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;

	// 使用されていなかったら何もせず終了
	if( file == NULL )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

		return -1 ;
	}
	
	// アーカイブを使用しているかどうかで分岐
	if( file->UseArchiveFlag == FALSE )
	{
		// 使用していない場合は標準入出力のファイルポインタを解放する
		ReadOnlyFileAccessClose( file->WinFilePointer_ ) ;
		file->WinFilePointer_ = 0 ;
	}
	else
	{
		// アーカイブを使用していた場合はアーカイブの参照数を減らす
		
		// アーカイブファイルの参照数を減らす
		DXA_DIR_CloseArchive( file->UseArchiveIndex ) ;

		// アーカイブファイルの後始末
		DXA_STREAM_Terminate( &file->DXAStream ) ;
	}

	// メモリを解放する
	DXFREE( file ) ;
	DXARCD.File[Handle & 0x0FFFFFFF] = NULL ;
	
	// 数を減らす
	DXARCD.FileNum -- ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &DXARCD.CriticalSection ) ;

	// 終了
	return 0 ;
}

// ファイルポインタの位置を取得する
extern	LONGLONG DXA_DIR_Tell( DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessTell( file->WinFilePointer_ ) ;
	else							return DXA_STREAM_Tell( &file->DXAStream ) ;
}

// ファイルポインタの位置を変更する
extern int DXA_DIR_Seek( DWORD_PTR Handle, LONGLONG SeekPoint, int SeekType )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessSeek( file->WinFilePointer_, SeekPoint, SeekType ) ;
	else							return DXA_STREAM_Seek( &file->DXAStream, SeekPoint, SeekType ) ;
}

// ファイルからデータを読み込む
extern size_t DXA_DIR_Read( void *Buffer, size_t BlockSize, size_t BlockNum, DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return ( size_t )-1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessRead( Buffer, BlockSize, BlockNum, file->WinFilePointer_ ) ;
	else							return DXA_STREAM_Read( &file->DXAStream, Buffer, BlockSize * BlockNum ) / BlockSize ;
}

// ファイルの終端を調べる
extern int DXA_DIR_Eof( DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 ) return ReadOnlyFileAccessEof( file->WinFilePointer_ ) ;
	else							return DXA_STREAM_Eof( &file->DXAStream ) ;
}

extern	int DXA_DIR_ChDir( const TCHAR *Path )
{
	::SetCurrentDirectory( Path ) ;

	return 0 ;
}

extern	int DXA_DIR_GetDir( TCHAR *Buffer )
{
	::GetCurrentDirectory( 256, Buffer ) ;

	return 0 ;
}


extern	int DXA_DIR_IdleCheck( DWORD_PTR Handle )
{
	DXARC_DIR_FILE *file = DXARCD.File[Handle & 0x0FFFFFFF] ;
	if( file == NULL ) return -1 ;
	if( file->UseArchiveFlag == 0 )	return ReadOnlyFileAccessIdleCheck( file->WinFilePointer_ ) ;
	else							return DXA_STREAM_IdleCheck( &file->DXAStream ) ;
}

// 戻り値: -1=エラー  -1以外=FindHandle
extern DWORD_PTR DXA_DIR_FindFirst( const TCHAR *FilePath, FILEINFO *Buffer )
{
	DXA_DIR_FINDDATA *find;
	char nPath[256];

	// メモリの確保
	find = ( DXA_DIR_FINDDATA * )DXALLOC( sizeof( DXA_DIR_FINDDATA ) );
	if( find == NULL ) return ( DWORD_PTR )-1;
	_MEMSET( find, 0, sizeof( *find ) );

	// 指定のオブジェクトがアーカイブファイル内か調べる
	if( DXA_DIR_OpenTest( FilePath, &find->UseArchiveIndex, NULL, nPath ) == -1 )
	{
		// アーカイブファイル内ではなかった場合はファイルから検索する
		find->UseArchiveFlag = 0;
		find->FindHandle = ReadOnlyFileAccessFindFirst( FilePath, Buffer );
	}
	else
	{
		// アーカイブファイル内の場合はアーカイブファイル内から検索する
		find->UseArchiveFlag = 1;
		find->FindHandle = DXA_FindFirst( &DXARCD.Archive[ find->UseArchiveIndex ]->Archive, nPath, Buffer );
	}

	// 検索ハンドルが取得できなかった場合はエラー
	if( find->FindHandle == ( DWORD_PTR )-1 )
	{
		DXFREE( find );
		return ( DWORD_PTR )-1;
	}

	// 返回句柄
	return (DWORD_PTR)find;
}

// 戻り値: -1=エラー  0=成功
extern int DXA_DIR_FindNext( DWORD_PTR FindHandle, FILEINFO *Buffer )
{
	DXA_DIR_FINDDATA *find;

	find = (DXA_DIR_FINDDATA *)FindHandle;
	if( find->UseArchiveFlag == 0 )
		return ReadOnlyFileAccessFindNext( find->FindHandle, Buffer );
	else
		return DXA_FindNext( find->FindHandle, Buffer );
}

// 戻り値: -1=エラー  0=成功
extern int DXA_DIR_FindClose( DWORD_PTR FindHandle )
{
	DXA_DIR_FINDDATA *find;

	find = (DXA_DIR_FINDDATA *)FindHandle;
	if( find->UseArchiveFlag == 0 )
	{
		ReadOnlyFileAccessFindClose( find->FindHandle );
	}
	else
	{
		DXA_FindClose( find->FindHandle );
		DXA_DIR_CloseArchive( find->UseArchiveIndex ) ;
	}

	DXFREE( find );

	return 0;
}




// 指定のＤＸＡファイルを丸ごとメモリに読み込む( 戻り値: -1=エラー  0=成功 )
extern int NS_DXArchivePreLoad( const TCHAR *FilePath , int ASyncThread )
{
	TCHAR fullpath[256];

	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	return DXA_DIR_OpenArchive( fullpath, NULL, -1, FALSE, FALSE, -1, TRUE, ASyncThread ) == -1 ? -1 : 0;
}

// 指定のＤＸＡファイルの事前読み込みが完了したかどうかを取得する( 戻り値： TRUE=完了した FALSE=まだ )
extern int NS_DXArchiveCheckIdle( const TCHAR *FilePath )
{
	int handle;
	TCHAR fullpath[256];

	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// ファイルパスからハンドルを取得する
	handle = DXA_DIR_GetArchive( fullpath );
	if( handle == -1 ) return 0 ;

	// 準備が完了したかどうかを得る
	return DXA_CheckIdle( &DXARCD.Archive[handle]->Archive );
}

// 指定のＤＸＡファイルをメモリから解放する
extern int NS_DXArchiveRelease( const TCHAR *FilePath )
{
	int handle;
	TCHAR fullpath[256];

	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// ファイルパスからハンドルを取得する
	handle = DXA_DIR_GetArchive( fullpath ) ;
	if( handle == -1 ) return 0 ;

	// 閉じる
	DXA_DIR_CloseArchive( handle ) ;
	DXA_DIR_CloseWaitArchive() ;

	// 終了
	return 0 ;
}

// ＤＸＡファイルの中に指定のファイルが存在するかどうかを調べる、TargetFilePath はＤＸＡファイルをカレントフォルダとした場合のパス( 戻り値:  -1=エラー  0:無い  1:ある )
extern int NS_DXArchiveCheckFile( const TCHAR *FilePath, const TCHAR *TargetFilePath )
{
	int index, ret ;
	DXARC_DIR_ARCHIVE *Archive ;
	TCHAR fullpath[256];

	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( FilePath, fullpath ) ;

	// アーカイブファイルがあるかどうか調べる
	index = DXA_DIR_OpenArchive( FilePath ) ;
	if( index == -1 ) return -1 ;

	// アーカイブの中に指定のファイルがあるかどうかを調べる
	Archive = DXARCD.Archive[ index ] ;

#ifdef UNICODE
	// マルチバイト文字列に変換
	char TargetFilePathA[1024];
	WCharToMBChar( Archive->Archive.HeadV5.CodePage == 0 ? _GET_CODEPAGE() : Archive->Archive.HeadV5.CodePage, ( DXWCHAR * )TargetFilePath, TargetFilePathA, 256 ) ;
	ret = DXA_GetFileInfo( &Archive->Archive, TargetFilePathA, NULL, NULL ) ;
#else
	ret = DXA_GetFileInfo( &Archive->Archive, TargetFilePath, NULL, NULL ) ;
#endif

	DXA_DIR_CloseArchive( index ) ;

	// 結果を返す
	return ret == -1 ? 0 : 1 ;
}

// メモリ上に展開されたＤＸＡファイルを指定のファイルパスにあることにする
extern int NS_DXArchiveSetMemImage(		void *ArchiveImage, int ArchiveImageSize, const TCHAR *EmulateFilePath, int ArchiveImageCopyFlag, int ArchiveImageReadOnly )
{
	TCHAR fullpath[256];

	// フルパスを得る(ついでに全ての文字を大文字にする)
	DXA_DIR_ConvertFullPath( EmulateFilePath, fullpath ) ;

	return DXA_DIR_OpenArchive( fullpath, ArchiveImage, ArchiveImageSize, ArchiveImageCopyFlag, ArchiveImageReadOnly, -1, FALSE, FALSE ) == -1 ? -1 : 0;
}

// DXArchiveSetMemImage の設定を解除する
extern int NS_DXArchiveReleaseMemImage(	void *ArchiveImage )
{
	int handle;

	// ファイルパスからハンドルを取得する
	handle = DXA_DIR_GetArchive( NULL, ArchiveImage ) ;
	if( handle == -1 ) return 0 ;

	// 閉じる
	DXA_DIR_CloseArchive( handle ) ;
	DXA_DIR_CloseWaitArchive() ;

	// 終了
	return 0 ;
}

#endif


// データを解凍する( 戻り値:解凍後のデータサイズ )
#define MIN_COMPRESS		(4)						// 最低圧縮バイト数
#define MAX_SEARCHLISTNUM	(64)					// 最大一致長を探す為のリストを辿る最大数
#define MAX_SUBLISTNUM		(65536)					// 圧縮時間短縮のためのサブリストの最大数
#define MAX_COPYSIZE 		(0x1fff + MIN_COMPRESS)	// 参照アドレスからコピー出切る最大サイズ( 圧縮コードが表現できるコピーサイズの最大値 + 最低圧縮バイト数 )
#define MAX_ADDRESSLISTNUM	(1024 * 1024 * 1)		// スライド辞書の最大サイズ
#define MAX_POSITION		(1 << 24)				// 参照可能な最大相対アドレス( 16MB )

// 圧縮時間短縮用リスト
typedef struct LZ_LIST
{
	LZ_LIST *next, *prev ;
	DWORD address ;
} LZ_LIST ;

// データを圧縮する( 戻り値:圧縮後のデータサイズ )
extern	int	DXA_Encode( void *Src, DWORD SrcSize, void *Dest )
{
	int dstsize ;
	int    bonus,    conbo,    conbosize,    address,    addresssize ;
	int maxbonus, maxconbo, maxconbosize, maxaddress, maxaddresssize ;
	BYTE keycode, *srcp, *destp, *dp, *sp, *sp2, *sp1 ;
	DWORD srcaddress, nextprintaddress, code ;
	int j ;
	DWORD i, m ;
	DWORD maxlistnum, maxlistnummask, listaddp ;
	DWORD sublistnum, sublistmaxnum ;
	LZ_LIST *listbuf, *listtemp, *list, *newlist ;
	BYTE *listfirsttable, *usesublistflagtable, *sublistbuf ;
	
	// サブリストのサイズを決める
	{
			 if( SrcSize < 100 * 1024 )			sublistmaxnum = 1 ;
		else if( SrcSize < 3 * 1024 * 1024 )	sublistmaxnum = MAX_SUBLISTNUM / 3 ;
		else									sublistmaxnum = MAX_SUBLISTNUM ;
	}

	// リストのサイズを決める
	{
		maxlistnum = MAX_ADDRESSLISTNUM ;
		if( maxlistnum > SrcSize )
		{
			while( ( maxlistnum >> 1 ) > 0x100 && ( maxlistnum >> 1 ) > SrcSize )
				maxlistnum >>= 1 ;
		}
		maxlistnummask = maxlistnum - 1 ;
	}

	// メモリの確保
	usesublistflagtable   = (BYTE *)DXALLOC(
		sizeof( DWORD_PTR )	* 65536 +					// メインリストの先頭オブジェクト用領域
		sizeof( LZ_LIST   )	* maxlistnum +				// メインリスト用領域
		sizeof( BYTE      )	* 65536 +					// サブリストを使用しているかフラグ用領域
		sizeof( DWORD_PTR )	* 256 * sublistmaxnum ) ;	// サブリスト用領域
		
	// アドレスのセット
	listfirsttable =     usesublistflagtable + sizeof( BYTE      ) * 65536 ;
	sublistbuf     =          listfirsttable + sizeof( DWORD_PTR ) * 65536 ;
	listbuf        = (LZ_LIST *)( sublistbuf + sizeof( DWORD_PTR ) * 256 * sublistmaxnum ) ;
	
	// 初期化
	_MEMSET( usesublistflagtable, 0, sizeof( BYTE      ) * 65536               ) ;
	_MEMSET(          sublistbuf, 0, sizeof( DWORD_PTR ) * 256 * sublistmaxnum ) ;
	_MEMSET(      listfirsttable, 0, sizeof( DWORD_PTR ) * 65536               ) ;
	list = listbuf ;
	for( i = maxlistnum / 8 ; i ; i --, list += 8 )
	{
		list[0].address =
		list[1].address =
		list[2].address =
		list[3].address =
		list[4].address =
		list[5].address =
		list[6].address =
		list[7].address = 0xffffffff ;
	}

	srcp  = (BYTE *)Src ;
	destp = (BYTE *)Dest ;

	// 圧縮元データの中で一番出現頻度が低いバイトコードを検索する
	{
		DWORD qnum, table[256], mincode ;

		for( i = 0 ; i < 256 ; i ++ )
			table[i] = 0 ;
		
		sp   = srcp ;
		qnum = SrcSize / 8 ;
		i    = qnum * 8 ;
		for( ; qnum ; qnum --, sp += 8 )
		{
			table[sp[0]] ++ ;
			table[sp[1]] ++ ;
			table[sp[2]] ++ ;
			table[sp[3]] ++ ;
			table[sp[4]] ++ ;
			table[sp[5]] ++ ;
			table[sp[6]] ++ ;
			table[sp[7]] ++ ;
		}
		for( ; i < SrcSize ; i ++, sp ++ )
			table[*sp] ++ ;
			
		keycode = 0 ;
		mincode = table[0] ;
		for( i = 1 ; i < 256 ; i ++ )
		{
			if( mincode < table[i] ) continue ;
			mincode = table[i] ;
			keycode = (BYTE)i ;
		}
	}

	// 圧縮元のサイズをセット
	((DWORD *)destp)[0] = SrcSize ;

	// キーコードをセット
	destp[8] = keycode ;

	// 圧縮処理
	dp               = destp + 9 ;
	sp               = srcp ;
	srcaddress       = 0 ;
	dstsize          = 0 ;
	listaddp         = 0 ;
	sublistnum       = 0 ;
	nextprintaddress = 1024 * 100 ;
	while( srcaddress < SrcSize )
	{
		// 残りサイズが最低圧縮サイズ以下の場合は圧縮処理をしない
		if( srcaddress + MIN_COMPRESS >= SrcSize ) goto NOENCODE ;

		// リストを取得
		code = *((WORD *)sp) ;
		list = (LZ_LIST *)( listfirsttable + code * sizeof( DWORD_PTR ) ) ;
		if( usesublistflagtable[code] == 1 )
		{
			list = (LZ_LIST *)( (DWORD_PTR *)list->next + sp[2] ) ;
		}
		else
		{
			if( sublistnum < sublistmaxnum )
			{
				list->next = (LZ_LIST *)( sublistbuf + sizeof( DWORD_PTR ) * 256 * sublistnum ) ;
				list       = (LZ_LIST *)( (DWORD_PTR *)list->next + sp[2] ) ;
			
				usesublistflagtable[code] = 1 ;
				sublistnum ++ ;
			}
		}

		// 一番一致長の長いコードを探す
		maxconbo   = -1 ;
		maxaddress = -1 ;
		maxbonus   = -1 ;
		for( m = 0, listtemp = list->next ; /*m < MAX_SEARCHLISTNUM &&*/ listtemp != NULL ; listtemp = listtemp->next, m ++ )
		{
			address = srcaddress - listtemp->address ;
			if( address >= MAX_POSITION )
			{
				if( listtemp->prev ) listtemp->prev->next = listtemp->next ;
				if( listtemp->next ) listtemp->next->prev = listtemp->prev ;
				listtemp->address = 0xffffffff ;
				continue ;
			}
			
			sp2 = &sp[-address] ;
			sp1 = sp ;
			if( srcaddress + MAX_COPYSIZE < SrcSize )
			{
				conbo = MAX_COPYSIZE / 4 ;
				while( conbo && *((DWORD *)sp2) == *((DWORD *)sp1) )
				{
					sp2 += 4 ;
					sp1 += 4 ;
					conbo -- ;
				}
				conbo = MAX_COPYSIZE - ( MAX_COPYSIZE / 4 - conbo ) * 4 ;

				while( conbo && *sp2 == *sp1 )
				{
					sp2 ++ ;
					sp1 ++ ;
					conbo -- ;
				}
				conbo = MAX_COPYSIZE - conbo ;
			}
			else
			{
				for( conbo = 0 ;
						conbo < MAX_COPYSIZE &&
						conbo + srcaddress < SrcSize &&
						sp[conbo - address] == sp[conbo] ;
							conbo ++ ){}
			}

			if( conbo >= 4 )
			{
				conbosize   = ( conbo - MIN_COMPRESS ) < 0x20 ? 0 : 1 ;
				addresssize = address < 0x100 ? 0 : ( address < 0x10000 ? 1 : 2 ) ;
				bonus       = conbo - ( 3 + conbosize + addresssize ) ;

				if( bonus > maxbonus )
				{
					maxconbo       = conbo ;
					maxaddress     = address ;
					maxaddresssize = addresssize ;
					maxconbosize   = conbosize ;
					maxbonus       = bonus ;
				}
			}
		}

		// リストに登録
		newlist = &listbuf[listaddp] ;
		if( newlist->address != 0xffffffff )
		{
			if( newlist->prev ) newlist->prev->next = newlist->next ;
			if( newlist->next ) newlist->next->prev = newlist->prev ;
			newlist->address = 0xffffffff ;
		}
		newlist->address = srcaddress ;
		newlist->prev    = list ;
		newlist->next    = list->next ;
		if( list->next != NULL ) list->next->prev = newlist ;
		list->next       = newlist ;
		listaddp         = ( listaddp + 1 ) & maxlistnummask ;

		// 一致コードが見つからなかったら非圧縮コードとして出力
		if( maxconbo == -1 )
		{
NOENCODE:
			// キーコードだった場合は２回連続で出力する
			if( *sp == keycode )
			{
				if( destp != NULL )
				{
					dp[0]  =
					dp[1]  = keycode ;
					dp += 2 ;
				}
				dstsize += 2 ;
			}
			else
			{
				if( destp != NULL )
				{
					*dp = *sp ;
					dp ++ ;
				}
				dstsize ++ ;
			}
			sp ++ ;
			srcaddress ++ ;
		}
		else
		{
			// 見つかった場合は見つけた位置と長さを出力する
			
			// キーコードと見つけた位置と長さを出力
			if( destp != NULL )
			{
				// キーコードの出力
				*dp++ = keycode ;

				// 出力する連続長は最低 MIN_COMPRESS あることが前提なので - MIN_COMPRESS したものを出力する
				maxconbo -= MIN_COMPRESS ;

				// 連続長０～４ビットと連続長、相対アドレスのビット長を出力
				*dp = (BYTE)( ( ( maxconbo & 0x1f ) << 3 ) | ( maxconbosize << 2 ) | maxaddresssize ) ;

				// キーコードの連続はキーコードと値の等しい非圧縮コードと
				// 判断するため、キーコードの値以上の場合は値を＋１する
				if( *dp >= keycode ) dp[0] += 1 ;
				dp ++ ;

				// 連続長５～１２ビットを出力
				if( maxconbosize == 1 )
					*dp++ = (BYTE)( ( maxconbo >> 5 ) & 0xff ) ;

				// maxconbo はまだ使うため - MIN_COMPRESS した分を戻す
				maxconbo += MIN_COMPRESS ;

				// 出力する相対アドレスは０が( 現在のアドレス－１ )を挿すので、－１したものを出力する
				maxaddress -- ;

				// 相対アドレスを出力
				*dp++ = (BYTE)( maxaddress ) ;
				if( maxaddresssize > 0 )
				{
					*dp++ = (BYTE)( maxaddress >> 8 ) ;
					if( maxaddresssize == 2 )
						*dp++ = (BYTE)( maxaddress >> 16 ) ;
				}
			}
			
			// 出力サイズを加算
			dstsize += 3 + maxaddresssize + maxconbosize ;
			
			// リストに情報を追加
			if( srcaddress + maxconbo < SrcSize )
			{
				sp2 = &sp[1] ;
				for( j = 1 ; j < maxconbo && (DWORD_PTR)&sp2[2] - (DWORD_PTR)srcp < SrcSize ; j ++, sp2 ++ )
				{
					code = *((WORD *)sp2) ;
					list = (LZ_LIST *)( listfirsttable + code * sizeof( DWORD_PTR ) ) ;
					if( usesublistflagtable[code] == 1 )
					{
						list = (LZ_LIST *)( (DWORD_PTR *)list->next + sp2[2] ) ;
					}
					else
					{
						if( sublistnum < sublistmaxnum )
						{
							list->next = (LZ_LIST *)( sublistbuf + sizeof( DWORD_PTR ) * 256 * sublistnum ) ;
							list       = (LZ_LIST *)( (DWORD_PTR *)list->next + sp2[2] ) ;
						
							usesublistflagtable[code] = 1 ;
							sublistnum ++ ;
						}
					}

					newlist = &listbuf[listaddp] ;
					if( newlist->address != 0xffffffff )
					{
						if( newlist->prev ) newlist->prev->next = newlist->next ;
						if( newlist->next ) newlist->next->prev = newlist->prev ;
						newlist->address = 0xffffffff ;
					}
					newlist->address = srcaddress + j ;
					newlist->prev = list ;
					newlist->next = list->next ;
					if( list->next != NULL ) list->next->prev = newlist ;
					list->next = newlist ;
					listaddp = ( listaddp + 1 ) & maxlistnummask ;
				}
			}
			
			sp         += maxconbo ;
			srcaddress += maxconbo ;
		}
	}

	// 圧縮後のデータサイズを保存する
	*((DWORD *)&destp[4]) = dstsize + 9 ;

	// 確保したメモリの解放
	DXFREE( usesublistflagtable ) ;

	// データのサイズを返す
	return dstsize + 9 ;
}

extern int DXA_Decode( void *Src, void *Dest )
{
	DWORD srcsize, destsize, code, indexsize, keycode, conbo, index = 0 ;
	BYTE *srcp, *destp, *dp, *sp ;

	destp = (BYTE *)Dest ;
	srcp  = (BYTE *)Src ;
	
	// 解凍後のデータサイズを得る
	destsize = *((DWORD *)&srcp[0]) ;

	// 圧縮データのサイズを得る
	srcsize = *((DWORD *)&srcp[4]) - 9 ;

	// キーコード
	keycode = srcp[8] ;
	
	// 出力先がない場合はサイズだけ返す
	if( Dest == NULL )
		return destsize ;
	
	// 展開開始
	sp  = srcp + 9 ;
	dp  = destp ;
	while( srcsize )
	{
		// キーコードか同かで処理を分岐
		if( sp[0] != keycode )
		{
			// 非圧縮コードの場合はそのまま出力
			*dp = *sp ;
			dp      ++ ;
			sp      ++ ;
			srcsize -- ;
			continue ;
		}
	
		// キーコードが連続していた場合はキーコード自体を出力
		if( sp[1] == keycode )
		{
			*dp = (BYTE)keycode ;
			dp      ++ ;
			sp      += 2 ;
			srcsize -= 2 ;
			
			continue ;
		}

		// 第一バイトを得る
		code = sp[1] ;

		// もしキーコードよりも大きな値だった場合はキーコード
		// とのバッティング防止の為に＋１しているので－１する
		if( code > keycode ) code -- ;

		sp      += 2 ;
		srcsize -= 2 ;

		// 連続長を取得する
		conbo = code >> 3 ;
		if( code & ( 0x1 << 2 ) )
		{
			conbo |= *sp << 5 ;
			sp      ++ ;
			srcsize -- ;
		}
		conbo += MIN_COMPRESS ;	// 保存時に減算した最小圧縮バイト数を足す

		// 参照相対アドレスを取得する
		indexsize = code & 0x3 ;
		switch( indexsize )
		{
		case 0 :
			index = *sp ;
			sp      ++ ;
			srcsize -- ;
			break ;
			
		case 1 :
			index = *((WORD *)sp) ;
			sp      += 2 ;
			srcsize -= 2 ;
			break ;
			
		case 2 :
			index = *((WORD *)sp) | ( sp[2] << 16 ) ;
			sp      += 3 ;
			srcsize -= 3 ;
			break ;
		}
		index ++ ;		// 保存時に－１しているので＋１する

		// 展開
		if( index < conbo )
		{
			DWORD num ;

			num  = index ;
			while( conbo > num )
			{
				_MEMCPY( dp, dp - num, num ) ;
				dp    += num ;
				conbo -= num ;
				num   += num ;
			}
			if( conbo != 0 )
			{
				_MEMCPY( dp, dp - num, conbo ) ;
				dp += conbo ;
			}
		}
		else
		{
			_MEMCPY( dp, dp - index, conbo ) ;
			dp += conbo ;
		}
	}

	// 解凍後のサイズを返す
	return (int)destsize ;
}

}
