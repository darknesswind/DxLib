﻿//-----------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用フォント関係プログラム
// 
//  	Ver 3.14d
// 
//-----------------------------------------------------------------------------

// ＤＸライブラリ作成時用定義
#define __DX_MAKE

// インクルード ---------------------------------------------------------------
#include "DxFontWin.h"
#include "DxWindow.h"
#include "DxBaseFuncWin.h"
#include "../DxFont.h"
#include "../DxBaseFunc.h"
#include "../DxSystem.h"
#include "../DxArchive_.h"
#include "../DxLog.h"
#include <windows.h>

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 -----------------------------------------------------------------

// 構造体宣言 -----------------------------------------------------------------

// データ定義 -----------------------------------------------------------------

// キャラセットテーブル
DWORD CharSetTable[ DX_CHARSET_NUM ] =
{
	DEFAULT_CHARSET,
	SHIFTJIS_CHARSET,
	HANGEUL_CHARSET,
	CHINESEBIG5_CHARSET,
	GB2312_CHARSET
} ;

const unsigned char Japanese1[ 7 ] = { 0x93, 0xfa, 0x96, 0x7b, 0x8c, 0xea, 0x00 } ;		// 日本語
const unsigned char Japanese2[ 5 ] = { 0xa4, 0xe9, 0xa4, 0xe5, 0x00 } ;					// 日文

FONTSYSTEM_WIN FontSystem_Win ;

// 関数宣言 -------------------------------------------------------------------

// プログラム -----------------------------------------------------------------

// CreateFontToHandle の環境依存処理を行う関数
extern int CreateFontToHandle_PF( FONTMANAGE *ManageData, int DefaultCharSet )
{
	int CreateFontSize ;
	int SampleScale ;
	int EnableAddHeight	= FALSE ;
	int	AddHeight		= 0 ;
	int	OrigHeight		= 0 ;

	if( WinData.DestroyMessageCatchFlag == TRUE )
	{
		return -1 ;
	}

	// フォントデータファイルを使用する場合は何もせずに終了
	if( ManageData->UseFontDataFile )
	{
		return 0 ;
	}

	switch( ManageData->FontType )
	{
	case DX_FONTTYPE_NORMAL:
	case DX_FONTTYPE_EDGE:
	case DX_FONTTYPE_ANTIALIASING:
	case DX_FONTTYPE_ANTIALIASING_EDGE:
		SampleScale = 1 ;
		break ;

	case DX_FONTTYPE_ANTIALIASING_4X4:
	case DX_FONTTYPE_ANTIALIASING_EDGE_4X4:
		SampleScale = 4 ;
		break ;

	case DX_FONTTYPE_ANTIALIASING_8X8:
	case DX_FONTTYPE_ANTIALIASING_EDGE_8X8:
		SampleScale = 8 ;
		break ;
	}
	CreateFontSize = ManageData->BaseInfo.FontSize * SampleScale ;

CREATEFONTLABEL:

	// 既にフォントが作成されていたら削除
	if( ManageData->PF->FontObj != NULL )
	{
		DeleteObject( ManageData->PF->FontObj ) ;
	}

	if( ManageData->FontName[0] != L'\0' )
	{
		// 特に文字セットの指定が無い場合で、且つ指定のフォント名の指定の文字セットが無い場合は文字セットを DEFAULT_CHARSET にする
		if( DefaultCharSet == TRUE )
		{
			wchar_t	TempNameBuffer[ 16 ][ 64 ] ;
			wchar_t	*TempNameBufferP ;
			int		TempNameNum ;
			int		TempNameBufferAlloc ;
			int		i ;

			TempNameNum = EnumFontNameEx2_WCHAR_T( TempNameBuffer[ 0 ], 16, ManageData->FontName, ManageData->BaseInfo.CharSet ) ;
			if( TempNameNum >= 16 )
			{
				TempNameNum			= EnumFontNameEx2_WCHAR_T( NULL, 0, ManageData->FontName, ManageData->BaseInfo.CharSet ) ;
				TempNameBufferP		= ( wchar_t * )DXALLOC( TempNameNum * 64 * sizeof( wchar_t ) ) ;
				TempNameNum			= EnumFontNameEx2_WCHAR_T( TempNameBufferP, TempNameNum, ManageData->FontName, ManageData->BaseInfo.CharSet ) ;
				TempNameBufferAlloc = TRUE ;
			}
			else
			{
				TempNameBufferAlloc = FALSE ;
				TempNameBufferP		= TempNameBuffer[ 0 ] ;
			}

			for( i = 0 ; i < TempNameNum && _WCSCMP( TempNameBufferP + i * 64, ManageData->FontName ) != 0 ; i ++ ){}
			if( i == TempNameNum )
			{
				ManageData->BaseInfo.CharSet = DX_CHARSET_DEFAULT ;
			}

			if( TempNameBufferAlloc )
			{
				DXFREE( TempNameBufferP ) ;
				TempNameBufferP = NULL ;
			}
		}

		ManageData->PF->FontObj = CreateFontW(
			CreateFontSize + AddHeight, 0, 0, 0,
			ManageData->BaseInfo.FontThickness * 100,
			ManageData->BaseInfo.Italic, FALSE, FALSE,
			CharSetTable[ ManageData->BaseInfo.CharSet ],
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			NONANTIALIASED_QUALITY, FIXED_PITCH,
			ManageData->FontName
		) ;

		if( ManageData->PF->FontObj == NULL )
		{
			ManageData->PF->FontObj = CreateFontW(
				CreateFontSize + AddHeight, 0, 0, 0,
				ManageData->BaseInfo.FontThickness * 100,
				ManageData->BaseInfo.Italic, FALSE, FALSE,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				NONANTIALIASED_QUALITY, FIXED_PITCH,
				ManageData->FontName
			) ;

			if( ManageData->PF->FontObj == NULL )
			{
				ManageData->PF->FontObj = CreateFontW(
					CreateFontSize + AddHeight, 0, 0, 0,
					ManageData->BaseInfo.FontThickness * 100,
					ManageData->BaseInfo.Italic, FALSE, FALSE,
					SHIFTJIS_CHARSET,
					OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
					NONANTIALIASED_QUALITY, FIXED_PITCH,
					ManageData->FontName
				) ;

				if( ManageData->PF->FontObj == NULL )
				{
					DXST_ERRORLOG_ADDUTF16LE( "\x07\x63\x9a\x5b\x6e\x30\xd5\x30\xa9\x30\xf3\x30\xc8\x30\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"指定のフォントの作成に失敗しました\n" @*/ ) ;
					goto ERR ;
				}
			}
		}
	}
	else
	{
		ManageData->PF->FontObj = CreateFontW(
			CreateFontSize + AddHeight, 0, 0, 0,
			ManageData->BaseInfo.FontThickness * 100,
			ManageData->BaseInfo.Italic, FALSE, FALSE,
			CharSetTable[ ManageData->BaseInfo.CharSet ],
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			NONANTIALIASED_QUALITY, FIXED_PITCH,
			( wchar_t * )"\x2d\xff\x33\xff\x20\x00\xb4\x30\xb7\x30\xc3\x30\xaf\x30\x00"/*@ L"ＭＳ ゴシック" @*/
		) ;
		ManageData->FontName[ 0 ] = L'\0' ;
	}

	if( ManageData->PF->FontObj == NULL )
	{
		ManageData->PF->FontObj = CreateFontW(
			CreateFontSize + AddHeight, 0, 0, 0,
			ManageData->BaseInfo.FontThickness * 100,
			FALSE, FALSE, FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			NONANTIALIASED_QUALITY, FIXED_PITCH,
			( wchar_t * )"\x2d\xff\x33\xff\x20\x00\xb4\x30\xb7\x30\xc3\x30\xaf\x30\x00"/*@ L"ＭＳ ゴシック" @*/
		) ;

		if( ManageData->PF->FontObj == NULL )
		{
			ManageData->PF->FontObj = CreateFontW(
				CreateFontSize + AddHeight, 0, 0, 0,
				ManageData->BaseInfo.FontThickness * 100,
				FALSE, FALSE, FALSE,
				DEFAULT_CHARSET,
				OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				NONANTIALIASED_QUALITY, FIXED_PITCH,
				NULL
			) ;
		}
		ManageData->FontName[ 0 ] = L'\0' ;

		if( ManageData->PF->FontObj == NULL )
		{
			DXST_ERRORLOG_ADDUTF16LE( "\xd5\x30\xa9\x30\xf3\x30\xc8\x30\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"フォントの作成に失敗しました\n" @*/ ) ;
			goto ERR ;
		}
	}

	// 文字のサイズを取得する
	{
		HDC			DC ;
		HFONT		OldFont ;
		TEXTMETRIC	TextInfo ;

		// デバイスコンテキストを取得
		DC = CreateCompatibleDC( NULL ) ;
		if( DC == NULL )
		{
			DXST_ERRORLOG_ADDUTF16LE( "\xc7\x30\xd0\x30\xa4\x30\xb9\x30\xb3\x30\xf3\x30\xc6\x30\xad\x30\xb9\x30\xc8\x30\x6e\x30\xd6\x53\x97\x5f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"デバイスコンテキストの取得に失敗しました\n" @*/ ) ;
			goto ERR ;
		}

		// フォントのセット
		OldFont = ( HFONT )SelectObject( DC, ManageData->PF->FontObj ) ;

		// フォントのステータスを取得
		GetTextMetrics( DC, &TextInfo ) ;

		// もし TextInfo.tmInternalLeading + TextInfo.tmExternalLeading が 0 ではなかったらその高さを追加してフォントを作成しなおす
		if( EnableAddHeight == FALSE )
		{
			if( TextInfo.tmInternalLeading + TextInfo.tmExternalLeading > 0 )
			{
				OrigHeight		= TextInfo.tmHeight ;
				AddHeight		= _FTOL( CreateFontSize / ( 1.0f - ( float )( TextInfo.tmInternalLeading + TextInfo.tmExternalLeading ) / TextInfo.tmHeight ) ) - CreateFontSize ;
				EnableAddHeight	= TRUE ;

				SelectObject( DC, OldFont ) ;
				DeleteDC( DC ) ;

				DeleteObject( ManageData->PF->FontObj ) ;
				ManageData->PF->FontObj = NULL ;

				goto CREATEFONTLABEL ;
			}
			
			ManageData->BaseInfo.FontAddHeight = 0 ;
		}
		else
		{
			ManageData->BaseInfo.FontAddHeight = ( WORD )( ( TextInfo.tmHeight - OrigHeight ) / SampleScale ) ;
		}

		// フォントの最大サイズを取得
		if( ManageData->BaseInfo.Italic )
		{
			// イタリック体の場合は最大幅が 1.35倍になる
			ManageData->BaseInfo.MaxWidth = ( WORD )( ( TextInfo.tmMaxCharWidth * 135 / SampleScale + 4 * 135 ) / 100 ) ;
		}
		else
		{
			ManageData->BaseInfo.MaxWidth = ( WORD )( TextInfo.tmMaxCharWidth / SampleScale + 4 ) ;
		}

		// フォントの高さを保存
		ManageData->BaseInfo.FontHeight = ( WORD )( TextInfo.tmHeight / SampleScale + 1 ) ;

		// GetGlyphOutline が使用できるかどうかを調べる
		{
			GLYPHMETRICS	gm ;
			MAT2			mt = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } } ;
			DWORD			Code ;
			DWORD			DataSize ;

			_MEMSET( &gm, 0, sizeof( GLYPHMETRICS ) ) ;
			Code		= L' ' ;
			DataSize	= GetGlyphOutlineW( DC, Code, GGO_BITMAP, &gm, 0, NULL, &mt ) ;

			// 失敗した場合は TextOut 方式を使用する
			if( DataSize == GDI_ERROR )
			{
				ManageData->PF->UseTextOut = TRUE ;

				// 使用するテクスチャキャッシュは 16bit 固定
				ManageData->TextureCacheColorBitDepth = 16 ;
			}
		}

		// TextOut 方式を使用する場合は DIB を作成しておく
		ManageData->PF->CacheBitmap			= NULL ;
		ManageData->PF->CacheBitmapMem		= NULL ;
		ManageData->PF->CacheBitmapMemPitch	= 0 ;
		if( ManageData->PF->UseTextOut )
		{
			BITMAPINFO	*BmpInfoPlus ;
			BITMAP		BmpData ;

			// フォントタイプも DX_FONTTYPE_NORMAL か DX_FONTTYPE_EDGE に限られる
			switch( ManageData->FontType )
			{
			case DX_FONTTYPE_ANTIALIASING :
			case DX_FONTTYPE_ANTIALIASING_4X4 :
			case DX_FONTTYPE_ANTIALIASING_8X8 :
				ManageData->FontType = DX_FONTTYPE_NORMAL ;
				break ;

			case DX_FONTTYPE_ANTIALIASING_EDGE :
			case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
			case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
				ManageData->FontType = DX_FONTTYPE_EDGE ;
				break ;
			}

			// キャッシュ領域のステータスの初期化
			BmpInfoPlus = ( BITMAPINFO * )DXCALLOC( sizeof( BITMAPINFO ) + sizeof( RGBQUAD ) * 256 ) ;
			if( BmpInfoPlus == NULL )
			{
				return DxLib_ErrorUTF16LE( "\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x20\x00\x69\x00\x6e\x00\x20\x00\x43\x00\x46\x00\x6f\x00\x6e\x00\x74\x00\x00"/*@ L"メモリの確保に失敗しました in CFont" @*/ ) ;
			}
			BmpInfoPlus->bmiHeader.biSize			= sizeof( BITMAPINFOHEADER ) ;
			BmpInfoPlus->bmiHeader.biWidth			=  ManageData->BaseInfo.MaxWidth ;
			BmpInfoPlus->bmiHeader.biHeight			= -ManageData->BaseInfo.MaxWidth ;
			BmpInfoPlus->bmiHeader.biPlanes			= 1 ;
			BmpInfoPlus->bmiHeader.biBitCount		= 8 ;
			BmpInfoPlus->bmiHeader.biCompression	= BI_RGB ;
			BmpInfoPlus->bmiHeader.biSizeImage		= ( DWORD )( ManageData->BaseInfo.MaxWidth * ManageData->BaseInfo.MaxWidth ) ;

			// カラーパレットのセット
			{
				RGBQUAD *Color ;
				int		i ;

				Color = &BmpInfoPlus->bmiColors[0] ;
				for( i = 0 ; i < 256 ; i ++ )
				{
					Color->rgbBlue     = ( BYTE )i ;
					Color->rgbRed      = ( BYTE )i ;
					Color->rgbBlue     = ( BYTE )i ;
					Color->rgbReserved = 0 ;

					Color ++ ;
				}
			}

			// ＤＩＢデータを作成する
			ManageData->PF->CacheBitmapMem	= NULL ; 
			ManageData->PF->CacheBitmap		= CreateDIBSection( DC, BmpInfoPlus, DIB_PAL_COLORS, ( void ** )&ManageData->PF->CacheBitmapMem, NULL, 0 ) ;

			// ピッチを得る
			GetObject( ManageData->PF->CacheBitmap, sizeof( BITMAP ), &BmpData ) ;
			ManageData->PF->CacheBitmapMemPitch = BmpData.bmWidthBytes ;

			// メモリの解放
			DXFREE( BmpInfoPlus ) ;
		}

		// フォントを元に戻す
		SelectObject( DC, OldFont ) ;

		// デバイスコンテキストを削除する
		DeleteDC( DC ) ;
	}

	// 正常終了
	return 0 ;

	// エラー処理
ERR :

	return -1 ;
}

// CreateFontToHandle の環境依存エラー処理を行う関数
extern int CreateFontToHandle_Error_PF( FONTMANAGE * ManageData )
{
	if( ManageData->PF->FontObj != NULL )
	{
		DeleteObject( ManageData->PF->FontObj ) ;
	}

	// 終了
	return 0 ;
}

// TerminateFontHandle の環境依存エラー処理を行う関数
extern int TerminateFontHandle_PF( FONTMANAGE *ManageData )
{
	// フォントイメージ取得用に使用したメモリの解放
	if( ManageData->PF->GetGlyphOutlineBuffer != NULL )
	{
		DXFREE( ManageData->PF->GetGlyphOutlineBuffer ) ;
		ManageData->PF->GetGlyphOutlineBuffer = NULL ;
		ManageData->PF->GetGlyphOutlineBufferSize = 0 ;
	}

	// フォントオブジェクトを削除
	if( ManageData->PF->FontObj != NULL )
	{
		DeleteObject( ManageData->PF->FontObj ) ;
		ManageData->PF->FontObj = NULL ;
	}

	// TextOut を使用するフォントで使用するビットマップを解放
	if( ManageData->PF->CacheBitmap != NULL )
	{
		DeleteObject( ManageData->PF->CacheBitmap ) ;
		ManageData->PF->CacheBitmap = NULL ;
		ManageData->PF->CacheBitmapMem = NULL ;
	}

	// 終了
	return 0 ;
}

// FontCacheCharaAddToHandleの環境依存処理を行う関数( 実行箇所区別 0 )
extern int FontCacheCharAddToHandle_Timing0_PF( FONTMANAGE *ManageData )
{
	// デバイスコンテキストの作成
	FontSystem_Win.Devicecontext = CreateCompatibleDC( NULL ) ;
	if( FontSystem_Win.Devicecontext == NULL )
	{
		DxLib_ErrorUTF16LE( "\xc6\x30\xad\x30\xb9\x30\xc8\x30\xad\x30\xe3\x30\xc3\x30\xb7\x30\xe5\x30\xb5\x30\xfc\x30\xd5\x30\xa7\x30\xb9\x30\x6e\x30\xc7\x30\xd0\x30\xa4\x30\xb9\x30\xb3\x30\xf3\x30\xc6\x30\xad\x30\xb9\x30\xc8\x30\x6e\x30\xd6\x53\x97\x5f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"テキストキャッシュサーフェスのデバイスコンテキストの取得に失敗しました" @*/ ) ;
		return -1 ;
	}

	// フォントをセット
	FontSystem_Win.OldFont = ( HFONT )SelectObject( FontSystem_Win.Devicecontext, ManageData->PF->FontObj ) ;
	if( FontSystem_Win.OldFont == NULL )
	{
		DeleteDC( FontSystem_Win.Devicecontext ) ;
		DxLib_ErrorUTF16LE( "\xc6\x30\xad\x30\xb9\x30\xc8\x30\xad\x30\xe3\x30\xc3\x30\xb7\x30\xe5\x30\xb5\x30\xfc\x30\xd5\x30\xa7\x30\xb9\x30\x6e\x30\xc7\x30\xd0\x30\xa4\x30\xb9\x30\xb3\x30\xf3\x30\xc6\x30\xad\x30\xb9\x30\xc8\x30\x6e\x30\xd6\x53\x97\x5f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"テキストキャッシュサーフェスのデバイスコンテキストの取得に失敗しました" @*/ ) ;
		return -1 ;
	}

	// フォントの情報を取得
	GetTextMetrics( FontSystem_Win.Devicecontext, &FontSystem_Win.TextMetric ) ;

	// TextOut を使用するかどうかで処理を分岐
	if( ManageData->PF->UseTextOut )
	{
		// 描画先ビットマップをセット
		FontSystem_Win.OldBitmap = ( HBITMAP )SelectObject( FontSystem_Win.Devicecontext , ManageData->PF->CacheBitmap ) ;

		// 文字の描画設定を行う
		{
			SetTextColor( FontSystem_Win.Devicecontext , RGB( 255 , 255 , 255 ) ) ; 		// 色をセット	

			// 背景色をセット
			SetBkColor( FontSystem_Win.Devicecontext , 0 ) ;
			SetBkMode( FontSystem_Win.Devicecontext , OPAQUE ) ;							// 背景を塗りつぶす指定
		}
	}

	// 正常終了
	return 0 ;
}

// FontCacheCharaAddToHandleの環境依存処理を行う関数( 実行箇所区別 1 )
extern int FontCacheCharAddToHandle_Timing1_PF( FONTMANAGE *ManageData, FONTCHARDATA *CharData, DWORD CharCode, int TextureCacheUpdate )
{
	int				ImageType = 0 ;
	int				gettype = 0 ;
	int				Space ;

	// スペースかどうかを取得しておく
	Space = CharCode == L' ' ? 1 : ( CharCode == FSYS.DoubleByteSpaceCharCode ? 2 : 0 ) ;

	// TextOut を使用するかどうかで処理を分岐
	if( ManageData->PF->UseTextOut )
	{
		SIZE    TempSize ;
		wchar_t AddStr[ 4 ] ;
		int     CharNum ;

		CharNum = PutCharCode( CharCode, WIN32_WCHAR_CODEPAGE, ( char * )AddStr ) / sizeof( wchar_t ) ;

		// 追加する文字の大きさを取得
		GetTextExtentPoint32W( FontSystem_Win.Devicecontext, AddStr, CharNum, &TempSize );

		// 文字イメージを出力
		TextOutW( FontSystem_Win.Devicecontext, 0, 0, AddStr, CharNum ) ;

		// イメージを転送
		FontCacheCharImageBltToHandle(
			ManageData,
			CharData,
			CharCode, 
			FALSE,
			DX_FONT_SRCIMAGETYPE_8BIT_ON_OFF,
			ManageData->PF->CacheBitmapMem,
			TempSize.cx,
			TempSize.cy,
			ManageData->PF->CacheBitmapMemPitch,
			0,
			0,
			TempSize.cx,
			TextureCacheUpdate
		) ;
	}
	else
	{
		DWORD			DataSize ;
		GLYPHMETRICS	gm ;
		GCP_RESULTSW	gcp ;
		wchar_t			gcpBuffer[ 2 ] ;
		MAT2			mt = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } } ;
		DWORD			SrcPitch ; 

		// 取得するイメージ形式を決定する
		switch( ManageData->FontType )
		{
		case DX_FONTTYPE_NORMAL :
		case DX_FONTTYPE_EDGE :
		case DX_FONTTYPE_ANTIALIASING_4X4 :
		case DX_FONTTYPE_ANTIALIASING_8X8 :
		case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
		case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
			gettype		= GGO_BITMAP ;

			switch( ManageData->FontType )
			{
			case DX_FONTTYPE_NORMAL :
			case DX_FONTTYPE_EDGE :
				ImageType	= DX_FONT_SRCIMAGETYPE_1BIT ;
				break ;

			case DX_FONTTYPE_ANTIALIASING_4X4 :
			case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
				ImageType	= DX_FONT_SRCIMAGETYPE_1BIT_SCALE4 ;
				break ;

			case DX_FONTTYPE_ANTIALIASING_8X8 :
			case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
				ImageType	= DX_FONT_SRCIMAGETYPE_1BIT_SCALE8 ;
				break ;
			}
			break ;
			
		case DX_FONTTYPE_ANTIALIASING :
		case DX_FONTTYPE_ANTIALIASING_EDGE :
			if( ManageData->TextureCacheFlag == FALSE || ManageData->TextureCacheColorBitDepth == 16 )
			{
				gettype		= GGO_GRAY4_BITMAP ;
				ImageType	= DX_FONT_SRCIMAGETYPE_8BIT_MAX16 ;
			}
			else
			{
				gettype		= GGO_GRAY8_BITMAP ;
				ImageType	= DX_FONT_SRCIMAGETYPE_8BIT_MAX64 ;
			}
			break ;
		}

		// サロゲートペアの場合は文字のグリフインデックスを取得する
		if( CharCode > 0xffff )
		{
			wchar_t CodeWString[ 4 ] ;
			int Bytes ;
			DWORD Result ;

			_MEMSET( &gcp, 0, sizeof( gcp ) ) ;
			gcp.lStructSize = sizeof( gcp ) ;
			gcp.lpGlyphs    = gcpBuffer ;
			gcp.nGlyphs     = 2 ;
			Bytes = PutCharCode( CharCode, DX_CODEPAGE_UTF16LE, ( char * )CodeWString ) ;
			Result = GetCharacterPlacementW( FontSystem_Win.Devicecontext, CodeWString, Bytes / 2, 0, &gcp, GCP_GLYPHSHAPE ) ;
			if( Result == 0 )
			{
				DXST_ERRORLOG_ADDUTF16LE( "\x57\x00\x69\x00\x6e\x00\x33\x00\x32\x00\x41\x00\x50\x00\x49\x00\x20\x00\x47\x00\x65\x00\x74\x00\x43\x00\x68\x00\x61\x00\x72\x00\x61\x00\x63\x00\x74\x00\x65\x00\x72\x00\x50\x00\x6c\x00\x61\x00\x63\x00\x65\x00\x6d\x00\x65\x00\x6e\x00\x74\x00\x20\x00\x4c\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Win32API GetCharacterPlacement が失敗しました\n" @*/ ) ;
				return -1 ;
			}
		}

		// 文字情報の取得
		_MEMSET( &gm, 0, sizeof( GLYPHMETRICS ) ) ;
		if( CharCode > 0xffff )
		{
			DataSize = GetGlyphOutlineW( FontSystem_Win.Devicecontext, ( UINT )gcp.lpGlyphs[ 0 ], ( UINT )gettype | GGO_GLYPH_INDEX, &gm, 0, NULL, &mt ) ;
		}
		else
		{
			DataSize = GetGlyphOutlineW( FontSystem_Win.Devicecontext, ( UINT )CharCode, ( UINT )gettype, &gm, 0, NULL, &mt ) ;
		}
		if( DataSize == GDI_ERROR )
		{
			DXST_ERRORLOG_ADDUTF16LE( "\x57\x00\x69\x00\x6e\x00\x33\x00\x32\x00\x41\x00\x50\x00\x49\x00\x20\x00\x47\x00\x65\x00\x74\x00\x47\x00\x6c\x00\x79\x00\x70\x00\x68\x00\x4f\x00\x75\x00\x74\x00\x6c\x00\x69\x00\x6e\x00\x65\x00\x20\x00\x4c\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Win32API GetGlyphOutline が失敗しました\n" @*/ ) ;
			return -1 ;
		}

		// スペース文字だった場合
		if( Space != 0 )
		{
			FontCacheCharImageBltToHandle(
				ManageData,
				CharData,
				CharCode, 
				TRUE,
				ImageType,
				NULL,
				0,
				0,
				0,
				0,
				0,
				gm.gmCellIncX,
				FALSE
			) ;
		}
		else
		// スペース以外でデータサイズが０の場合
		if( DataSize == 0 )
		{
			FontCacheCharImageBltToHandle(
				ManageData,
				CharData,
				CharCode, 
				FALSE,
				DX_FONT_SRCIMAGETYPE_1BIT,
				NULL,
				0,
				0,
				0,
				0,
				0,
				0,
				FALSE
			) ;
		}
		else
		{
			BYTE *DataBuffer ;
			DWORD BufferSize ;

			SrcPitch   = ( DataSize / gm.gmBlackBoxY ) / 4 * 4 ;
			BufferSize = DataSize + SrcPitch * ( 2 + gm.gmBlackBoxY ) ;

			if( ManageData->PF->GetGlyphOutlineBufferSize < BufferSize )
			{
				ManageData->PF->GetGlyphOutlineBufferSize = BufferSize ;

				if( ManageData->PF->GetGlyphOutlineBuffer != NULL )
				{
					DXFREE( ManageData->PF->GetGlyphOutlineBuffer ) ;
					ManageData->PF->GetGlyphOutlineBuffer = NULL ;
				}

				ManageData->PF->GetGlyphOutlineBuffer = DXALLOC( ManageData->PF->GetGlyphOutlineBufferSize ) ;
				if( ManageData->PF->GetGlyphOutlineBuffer == NULL )
				{
					DxLib_ErrorUTF16LE( "\xa2\x30\xf3\x30\xc1\x30\xa8\x30\xa4\x30\xea\x30\xa2\x30\xb9\x30\x87\x65\x57\x5b\xd6\x53\x97\x5f\x28\x75\xd0\x30\xc3\x30\xd5\x30\xa1\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"アンチエイリアス文字取得用バッファの確保に失敗しました" @*/ ) ;
					return -1 ;
				}
			}

			_MEMSET( ManageData->PF->GetGlyphOutlineBuffer, 0, BufferSize ) ;

			DataBuffer	= ( BYTE * )ManageData->PF->GetGlyphOutlineBuffer + SrcPitch * ( 1 + gm.gmBlackBoxY ) ;
			if( CharCode > 0xffff )
			{
				DataSize	= GetGlyphOutlineW( FontSystem_Win.Devicecontext, ( UINT )gcp.lpGlyphs[ 0 ], ( UINT )gettype | GGO_GLYPH_INDEX, &gm, DataSize, ( LPVOID )DataBuffer, &mt ) ;
			}
			else
			{
				DataSize	= GetGlyphOutlineW( FontSystem_Win.Devicecontext, ( UINT )CharCode, ( UINT )gettype, &gm, DataSize, ( LPVOID )DataBuffer, &mt ) ;
			}
			if( DataSize == GDI_ERROR )
			{
				DXST_ERRORLOG_ADDUTF16LE( "\x57\x00\x69\x00\x6e\x00\x33\x00\x32\x00\x41\x00\x50\x00\x49\x00\x20\x00\x47\x00\x65\x00\x74\x00\x47\x00\x6c\x00\x79\x00\x70\x00\x68\x00\x4f\x00\x75\x00\x74\x00\x6c\x00\x69\x00\x6e\x00\x65\x00\x20\x00\x4c\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Win32API GetGlyphOutline が失敗しました\n" @*/ ) ;
				return -1 ;
			}

			// イメージを転送
			FontCacheCharImageBltToHandle(
				ManageData,
				CharData,
				CharCode, 
				FALSE,
				ImageType,
				DataBuffer,
				gm.gmBlackBoxX,
				gm.gmBlackBoxY,
				( DataSize / gm.gmBlackBoxY ) / 4 * 4,
				gm.gmptGlyphOrigin.x ,
				FontSystem_Win.TextMetric.tmAscent - gm.gmptGlyphOrigin.y,
				gm.gmCellIncX,
				TextureCacheUpdate
			) ;
		}
	}

	// 正常終了
	return 0 ;
}

// FontCacheCharaAddToHandleの環境依存処理を行う関数( 実行箇所区別 2 )
extern int FontCacheCharAddToHandle_Timing2_PF( FONTMANAGE *ManageData )
{
	// フォントを元に戻す
	SelectObject( FontSystem_Win.Devicecontext, FontSystem_Win.OldFont ) ;

	if( ManageData->PF->UseTextOut )
	{
		// ビットマップを元に戻す
		SelectObject( FontSystem_Win.Devicecontext, FontSystem_Win.OldBitmap ) ;
		FontSystem_Win.OldBitmap = NULL ;
	}

	// デバイスコンテキストの削除
	DeleteDC( FontSystem_Win.Devicecontext ) ;

	// 終了
	return 0 ;
}



#ifndef DX_NON_SAVEFUNCTION

// フォントデータファイルを作成する
extern int NS_CreateFontDataFile( const TCHAR *SaveFilePath, const TCHAR *FontName, int Size, int BitDepth /* DX_FONTIMAGE_BIT_1等 */ , int Thick, int Italic, int CharSet, const TCHAR *SaveCharaList )
{
	FONTMANAGE				*ManageData ;
	FONTDATAFILEHEADER		*FontFileHeader ;
	FONTDATAFILECHARADATA	*FontCharaData ;
	BYTE					*FileHeaderBuffer = NULL ;
	DWORD					 FileHeaderBufferSize ;
	BYTE					*FilePressHeaderBuffer = NULL ;
	DWORD					 FilePressHeaderSize ;
	BYTE					*FileImageBuffer = NULL ;
	DWORD					 FileImageBufferSize ;
	DWORD					 FileImageBufferAddress ;
	BYTE					*FontTempImageBuffer = NULL ;
	DWORD					 FontTempImageBufferSize = 0 ;
	DWORD					 FontTempImageSize ;
	DWORD					 FontImagePressSize ;
	FONTCHARDATA			*CharData ;
	int						 FontHandle = -1 ;
	int						 FontType = 0 ;
	int						 FontCacheToTextureFlag ;
	int						 FontCacheTextureColorBitDepth ;
	int						 FontCacheCharaNum ;
	int						 FontCacheUsePremulAlphaFlag ;
	int						 i ;
	DWORD					*CharaList = NULL ;
	DWORD					 CharaIndex ;
	DWORD					 CharaNum ;
	const int				 CacheCharNum = 16 ;
	int						 CodePage ;

	// 指定のパラメータに合ったフォントハンドルを作成する
	FontCacheToTextureFlag        = NS_GetFontCacheToTextureFlag() ;
	FontCacheTextureColorBitDepth = NS_GetFontCacheTextureColorBitDepth() ;
	FontCacheCharaNum             = NS_GetFontCacheCharNum() ;
	FontCacheUsePremulAlphaFlag   = NS_GetFontCacheUsePremulAlphaFlag() ;
	NS_SetFontCacheToTextureFlag( TRUE ) ;
	NS_SetFontCacheCharNum( CacheCharNum ) ;
	NS_SetFontCacheUsePremulAlphaFlag( FALSE ) ;
	switch( BitDepth )
	{
	case DX_FONTIMAGE_BIT_1 :
		FontType = DX_FONTTYPE_NORMAL ;
		NS_SetFontCacheTextureColorBitDepth( 16 ) ;
		break ;

	case DX_FONTIMAGE_BIT_4 :
	case DX_FONTIMAGE_BIT_8 :
		FontType = DX_FONTTYPE_ANTIALIASING_8X8 ;
		NS_SetFontCacheTextureColorBitDepth( 32 ) ;
		break ;
	}
	FontHandle = NS_CreateFontToHandle( FontName, Size, Thick, FontType, CharSet, 0, Italic ) ;
	NS_SetFontCacheToTextureFlag(        FontCacheToTextureFlag ) ;
	NS_SetFontCacheTextureColorBitDepth( FontCacheTextureColorBitDepth ) ;
	NS_SetFontCacheCharNum(              FontCacheCharaNum ) ;
	NS_SetFontCacheUsePremulAlphaFlag(   FontCacheUsePremulAlphaFlag ) ;
	if( FontHandle < 0 )
	{
		return -1 ;
	}

	ManageData = GetFontManageDataToHandle( FontHandle ) ;

	// コードページの取得
	CodePage = GetFontHandleCharCode( FontHandle ) ;

	// SaveCharaList が NULL の場合は、全ての文字を変換の対象にする
	CharaNum = 0 ;
	if( SaveCharaList == NULL )
	{
		CharaList = ( DWORD * )DXALLOC( sizeof( DWORD ) * 0x10000 ) ;
		_MEMSET( CharaList, 0, sizeof( DWORD ) * 0x10000 ) ;

		for( i = 1 ; i < 0x10000 ; i ++ )
		{
			CharaList[ CharaNum ] = i ;
			CharaNum ++ ;
		}
	}
	else
	{
		DWORD CharCode ;
		int   CharBytes ;
		int   SrcAddr ;
		int   StringCharNum ;
		DWORD j ;

		StringCharNum = GetStringCharNum( ( const char * )SaveCharaList, CodePage ) ;
		CharaList = ( DWORD * )DXALLOC( sizeof( DWORD ) * StringCharNum ) ;

		SrcAddr = 0 ;
		for( i = 0 ; i < StringCharNum ; i ++ )
		{
			CharCode = GetCharCode( ( const char * )&( ( BYTE * )SaveCharaList )[ SrcAddr ], CodePage, &CharBytes ) ;
			CharCode = ConvCharCode( CharCode, CodePage, DX_CODEPAGE_UTF32LE ) ;

			for( j = 0 ; j < CharaNum ; j ++ )
			{
				if( CharaList[ j ] == CharCode )
				{
					break ;
				}
			}
			if( j == CharaNum )
			{
				CharaList[ CharaNum ] = CharCode ;
				CharaNum ++ ;
			}
		}
	}

	// テクスチャキャッシュが使用できない場合はエラー
	if( ManageData->TextureCacheFlag == FALSE )
	{
		goto ERR ;
	}

	// TextOut を使用する場合は強制的に DX_FONTIMAGE_BIT_1 になる
	if( ManageData->PF->UseTextOut )
	{
		FontType = DX_FONTTYPE_NORMAL ;
		BitDepth = DX_FONTIMAGE_BIT_1 ;
	}

	// ヘッダバッファを確保
	FileHeaderBufferSize = sizeof( FONTDATAFILEHEADER ) + CharaNum * sizeof( FONTDATAFILECHARADATA ) ; 
	FileHeaderBuffer = ( BYTE * )DXALLOC( FileHeaderBufferSize * 3 ) ;
	if( FileHeaderBuffer == NULL )
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xd5\x30\xa9\x30\xf3\x30\xc8\x30\xc7\x30\xfc\x30\xbf\x30\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xd8\x30\xc3\x30\xc0\x30\xc5\x60\x31\x58\x00\x4e\x42\x66\x18\x8a\xb6\x61\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"フォントデータファイルヘッダ情報一時記憶用メモリの確保に失敗しました\n" @*/ ) ;
		goto ERR ;
	}
	_MEMSET( FileHeaderBuffer, 0, FileHeaderBufferSize * 3 ) ;
	FilePressHeaderBuffer = FileHeaderBuffer + FileHeaderBufferSize ;

	// イメージバッファを確保
	FileImageBufferSize = 1024 * 1024 * 32 ; 
	FileImageBuffer = ( BYTE * )DXALLOC( FileImageBufferSize ) ;
	if( FileImageBuffer == NULL )
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xd5\x30\xa9\x30\xf3\x30\xc8\x30\xc7\x30\xfc\x30\xbf\x30\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xd8\x30\xc3\x30\xc0\x30\xc5\x60\x31\x58\x00\x4e\x42\x66\x18\x8a\xb6\x61\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"フォントデータファイルヘッダ情報一時記憶用メモリの確保に失敗しました\n" @*/ ) ;
		goto ERR ;
	}
	_MEMSET( FileImageBuffer, 0, FileImageBufferSize ) ;

	FontFileHeader = ( FONTDATAFILEHEADER * )FileHeaderBuffer ;
	FontFileHeader->Magic[ 0 ]              = 'F' ;
	FontFileHeader->Magic[ 1 ]              = 'N' ;
	FontFileHeader->Magic[ 2 ]              = 'T' ;
	FontFileHeader->Magic[ 3 ]              = 'F' ;
	FontFileHeader->Version                 = 0 ;
	FontFileHeader->Press.BaseInfo          = ManageData->BaseInfo ;
	FontFileHeader->Press.BaseInfo.CodePage = ( WORD )CodePage ;
	FontFileHeader->Press.ImageBitDepth     = ( BYTE )BitDepth ;

	ConvString( ( const char * )ManageData->FontName, WCHAR_T_CODEPAGE, ( char * )FontFileHeader->Press.FontName, DX_CODEPAGE_UTF16LE ) ;

	FontCharaData = ( FONTDATAFILECHARADATA * )( FileHeaderBuffer + 1 ) ;
	FileImageBufferAddress = 0 ;

	for( CharaIndex = 0 ; CharaIndex < CharaNum ; CharaIndex ++ )
	{
		// キャッシュに文字を追加
		if( CharaIndex % CacheCharNum == 0 )
		{
			int AddNum ;

			AddNum = CharaNum - CharaIndex ;
			if( AddNum > CacheCharNum )
			{
				AddNum = CacheCharNum ;
			}

			FontCacheCharAddToHandle( AddNum, &CharaList[ CharaIndex ], FontHandle, FALSE ) ;
		}

		// キャッシュに存在しない場合は追加する
		CharData = GetFontCacheChar( ManageData, CharaList[ CharaIndex ], FALSE ) ;
		if( CharData == NULL )
		{
			continue ;
		}

		// 画像サイズが０の場合は追加しない
		if( CharData->SizeX == 0 &&
			CharData->SizeY == 0 &&
			CharData->AddX  == 0 )
		{
			continue ;
		}

		// フォントの情報をセット
		FontCharaData->CodeUnicode   = CharaList[ CharaIndex ] ;
		FontCharaData->DrawX         = CharData->DrawX ;
		FontCharaData->DrawY         = CharData->DrawY ;
		FontCharaData->AddX          = CharData->AddX ;
		FontCharaData->SizeX         = CharData->SizeX ;
		FontCharaData->SizeY         = CharData->SizeY ;
		FontCharaData->ImageAddress  = FileImageBufferAddress ;

		// イメージデータの構築
		{
			DWORD		 DestPitch ;
			DWORD		 i ;
			DWORD		 j ;
			BYTE		*Dest ;
			BYTE		*Src ;
			DWORD		 SrcAlphaMask ;
			DWORD		 SrcAlphaLoc ;
			DWORD		 SrcAddPitch ;
			BASEIMAGE	*SrcBaseImage ;
			BYTE		 DestData ;

			SrcBaseImage = &ManageData->TextureCacheBaseImage ;
			SrcAlphaMask = SrcBaseImage->ColorData.AlphaMask ;
			SrcAlphaLoc  = SrcBaseImage->ColorData.AlphaLoc ;
			SrcAddPitch  = ( DWORD )( SrcBaseImage->Pitch - SrcBaseImage->ColorData.PixelByte * CharData->SizeX ) ;

			switch( BitDepth )
			{
			case DX_FONTIMAGE_BIT_1 :
				DestPitch = ( DWORD )( ( CharData->SizeX + 7 ) / 8 ) ;
				break ;

			case DX_FONTIMAGE_BIT_4 :
				DestPitch = ( DWORD )( ( CharData->SizeX + 1 ) / 2 ) ;
				break ;

			case DX_FONTIMAGE_BIT_8 :
				DestPitch = CharData->SizeX ;
				break ;
			}

			FontCharaData->ImagePitch = DestPitch ;

			FontTempImageSize = DestPitch * CharData->SizeY ;
			if( FontTempImageBufferSize < FontTempImageSize )
			{
				FontTempImageBufferSize = FontTempImageSize + 1024 * 32 ;
				FontTempImageBuffer = ( BYTE * )DXREALLOC( FontTempImageBuffer, FontTempImageBufferSize ) ;
				if( FontTempImageBuffer == NULL )
				{
					DXST_ERRORLOG_ADDUTF16LE( "\xd5\x30\xa9\x30\xf3\x30\xc8\x30\xc7\x30\xfc\x30\xbf\x30\x3b\x75\xcf\x50\x00\x4e\x42\x66\x18\x8a\xb6\x61\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"フォントデータ画像一時記憶用メモリの確保に失敗しました\n" @*/ ) ;
					goto ERR ;
				}
			}

			Dest = FontTempImageBuffer ;
			Src  = ( BYTE * )SrcBaseImage->GraphData + SrcBaseImage->ColorData.PixelByte * 1 + SrcBaseImage->Pitch * 1 ;
			switch( BitDepth )
			{
			case DX_FONTIMAGE_BIT_1 :
				for( i = 0 ; i < CharData->SizeY ; i ++, Src += SrcAddPitch )
				{
					for( j = 0 ; j < CharData->SizeX ; j += 8 )
					{
						DestData = 0 ;

						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x80 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						if( CharData->SizeX == j + 1 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}
						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x40 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						if( CharData->SizeX == j + 2 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}
						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x20 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						if( CharData->SizeX == j + 3 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}
						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x10 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						if( CharData->SizeX == j + 4 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}
						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x08 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						if( CharData->SizeX == j + 5 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}
						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x04 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						if( CharData->SizeX == j + 6 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}
						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x02 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						if( CharData->SizeX == j + 7 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}
						if( *( ( DWORD * )Src ) & SrcAlphaMask ) DestData |= 0x01 ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						*Dest = DestData ;
						Dest ++ ;
					}
				}
				break ;

			case DX_FONTIMAGE_BIT_4 :
				for( i = 0 ; i < CharData->SizeY ; i ++, Src += SrcAddPitch )
				{
					for( j = 0 ; j < CharData->SizeX ; j += 2 )
					{
						DestData = ( BYTE )( ( ( *( ( DWORD * )Src ) & SrcAlphaMask ) >> ( SrcAlphaLoc + 4 ) ) << 4 ) ;
						Src += SrcBaseImage->ColorData.PixelByte ;
						if( CharData->SizeX == j + 1 )
						{
							*Dest = DestData ;
							Dest ++ ;
							continue ;
						}

						DestData |= ( BYTE )( ( *( ( DWORD * )Src ) & SrcAlphaMask ) >> ( SrcAlphaLoc + 4 ) ) ;
						Src += SrcBaseImage->ColorData.PixelByte ;

						*Dest = DestData ;
						Dest ++ ;
					}
				}
				break ;

			case DX_FONTIMAGE_BIT_8 :
				for( i = 0 ; i < CharData->SizeY ; i ++, Src += SrcAddPitch )
				{
					for( j = 0 ; j < CharData->SizeX ; j ++, Dest ++, Src += SrcBaseImage->ColorData.PixelByte )
					{
						*Dest = ( BYTE )( ( *( ( DWORD * )Src ) & SrcAlphaMask ) >> SrcAlphaLoc ) ;
					}
				}
				break ;
			}

			// イメージを圧縮する
			if( FileImageBufferSize < FileImageBufferAddress + FontTempImageSize * 2 )
			{
				FileImageBufferSize += FontTempImageSize * 2 + 1024 * 1024 * 32 ;
				FileImageBuffer = ( BYTE * )DXREALLOC( FileImageBuffer, FileImageBufferSize ) ;
				if( FileImageBuffer == NULL )
				{
					DXST_ERRORLOG_ADDUTF16LE( "\xd5\x30\xa9\x30\xf3\x30\xc8\x30\xc7\x30\xfc\x30\xbf\x30\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xd8\x30\xc3\x30\xc0\x30\xc5\x60\x31\x58\x00\x4e\x42\x66\x18\x8a\xb6\x61\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xe1\x62\x35\x5f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"フォントデータファイルヘッダ情報一時記憶用メモリの拡張に失敗しました\n" @*/ ) ;
					goto ERR ;
				}
			}
			FontImagePressSize = ( DWORD )DXA_Encode( FontTempImageBuffer, FontTempImageSize, FileImageBuffer + FileImageBufferAddress ) ;

			if( FontImagePressSize >= FontTempImageSize )
			{
				FontCharaData->Press = 0 ;

				_MEMCPY( FileImageBuffer + FileImageBufferAddress, FontTempImageBuffer, FontTempImageSize ) ;
			}
			else
			{
				FontCharaData->Press = 1 ;
			}

			// 既に登録している文字とまったく同じ画像があるか調べる
			{
				FONTDATAFILECHARADATA	*FontCharaDataSub ;

				FontCharaDataSub = ( FONTDATAFILECHARADATA * )( FontFileHeader + 1 ) ;
				for( i = 0 ; i < FontFileHeader->CharaNum ; i ++, FontCharaDataSub ++ )
				{
					if( FontCharaDataSub->Press != FontCharaData->Press ||
						FontCharaDataSub->DrawX != FontCharaData->DrawX ||
						FontCharaDataSub->DrawY != FontCharaData->DrawY ||
						FontCharaDataSub->AddX  != FontCharaData->AddX ||
						FontCharaDataSub->SizeX != FontCharaData->SizeX ||
						FontCharaDataSub->SizeY != FontCharaData->SizeY )
					{
						continue ;
					}

					if( FontCharaData->Press )
					{
						int DecodeSize ;

						DecodeSize = DXA_Decode( FileImageBuffer + FontCharaDataSub->ImageAddress, NULL ) ;
						if( DecodeSize != FontImagePressSize )
						{
							continue ;
						}
						if( _MEMCMP( FileImageBuffer + FontCharaDataSub->ImageAddress, FileImageBuffer + FontCharaData->ImageAddress, FontImagePressSize ) == 0 )
						{
							break ;
						}
					}
					else
					{
						if( _MEMCMP( FileImageBuffer + FontCharaDataSub->ImageAddress, FileImageBuffer + FontCharaData->ImageAddress, FontTempImageSize ) == 0 )
						{
							break ;
						}
					}
				}

				// 全く同じ画像があった場合は画像を共有する
				if( i != FontFileHeader->CharaNum )
				{
					FontCharaData->ImageAddress = FontCharaDataSub->ImageAddress ;
				}
				else
				{
					// それ以外の場合は画像のアドレスを進める
					if( FontCharaData->Press )
					{
						FileImageBufferAddress += FontImagePressSize ;
					}
					else
					{
						FileImageBufferAddress += FontTempImageSize ;
					}
				}
			}
		}

		FontCharaData ++ ;
		FontFileHeader->CharaNum ++ ;

		if( CharaList[ CharaIndex ] >= 0x10000 )
		{
			FontFileHeader->CharaExNum ++ ;
		}
	}

	// ファイルヘッダ部分の圧縮
	{
		int NotPressSize ;

		NotPressSize = sizeof( FONTDATAFILEHEADER ) - sizeof( FONTDATAFILEPRESSHEADER ) ;

		FilePressHeaderSize = DXA_Encode(
			FileHeaderBuffer + NotPressSize,
			( DWORD )( ( BYTE * )FontCharaData - FileHeaderBuffer ) - NotPressSize,
			FilePressHeaderBuffer + NotPressSize ) + NotPressSize ;

		// 画像データ開始アドレスをセット
		FontFileHeader->ImageAddress = FilePressHeaderSize ;

		// 圧縮しない部分をコピー
		_MEMCPY( FilePressHeaderBuffer, FileHeaderBuffer, NotPressSize ) ;
	}

	// ファイルに保存
	{
		HANDLE FileHandle ;
		DWORD WriteSize ;

		// 既にファイルがあった場合用に、ファイル削除処理
		DeleteFile( SaveFilePath ) ;

		// ファイルを開く
		FileHandle = CreateFile( SaveFilePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( FileHandle == NULL ) 
		{
			DXST_ERRORLOG_ADDUTF16LE( "\xd5\x30\xa9\x30\xf3\x30\xc8\x30\xc7\x30\xfc\x30\xbf\x30\xd5\x30\xa1\x30\xa4\x30\xeb\x30\x6e\x30\xaa\x30\xfc\x30\xd7\x30\xf3\x30\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"フォントデータファイルのオープンに失敗しました\n" @*/ ) ;
			goto ERR ;
		}

		// ファイルヘッダの書き出し
		WriteFile( FileHandle, FilePressHeaderBuffer, FilePressHeaderSize, &WriteSize, NULL ) ;
		
		// イメージの書き出し
		WriteFile( FileHandle, FileImageBuffer, FileImageBufferAddress, &WriteSize, NULL ) ;

		// ファイルを閉じる
		CloseHandle( FileHandle ) ;
	}

	// 確保していたメモリを解放
	if( FontTempImageBuffer != NULL )
	{
		DXFREE( FontTempImageBuffer ) ;
		FontTempImageBuffer = NULL ;
	}

	if( FileHeaderBuffer != NULL )
	{
		DXFREE( FileHeaderBuffer ) ;
		FileHeaderBuffer = NULL ;
	}

	if( FileImageBuffer != NULL )
	{
		DXFREE( FileImageBuffer ) ;
		FileImageBuffer = NULL ;
	}

	if( CharaList != NULL )
	{
		DXFREE( CharaList ) ;
		CharaList = NULL ;
	}

	// フォントデータ作成処理用に作成したフォントハンドルを削除
	NS_DeleteFontToHandle( FontHandle ) ;

	// 正常終了
	return 0 ;

	// エラー処理
ERR :
	if( FontTempImageBuffer != NULL )
	{
		DXFREE( FontTempImageBuffer ) ;
		FontTempImageBuffer = NULL ;
	}

	if( FileHeaderBuffer != NULL )
	{
		DXFREE( FileHeaderBuffer ) ;
		FileHeaderBuffer = NULL ;
	}

	if( FileImageBuffer != NULL )
	{
		DXFREE( FileImageBuffer ) ;
		FileImageBuffer = NULL ;
	}

	if( CharaList != NULL )
	{
		DXFREE( CharaList ) ;
		CharaList = NULL ;
	}

	if( FontHandle >= 0 )
	{
		NS_DeleteFontToHandle( FontHandle ) ;
	}

	return -1 ;
}

#endif // DX_NON_SAVEFUNCTION

// フォント列挙用コールバック関数
int CALLBACK EnumFontFamExProc( ENUMLOGFONTEXW *lpelf, NEWTEXTMETRICEXW * /*lpntm*/, int nFontType, LPARAM lParam )
{
	ENUMFONTDATA *EnumFontData = ( ENUMFONTDATA * )lParam ;

	// JapanOnlyフラグが立っていたら TrueType 、日本語フォントだけを列挙する
	// 横向きフォント(@付)はいずれもはじく
	if( ( EnumFontData->JapanOnlyFlag == TRUE && ( nFontType & TRUETYPE_FONTTYPE ) &&
		( _WCSCMP( ( wchar_t * )"\xe5\x65\x2c\x67\x9e\x8a\x00"/*@ L"日本語" @*/, &lpelf->elfScript[0] ) == 0 ||
		  _WCSCMP( ( wchar_t * )"\xe5\x65\x87\x65\x00"/*@ L"日文" @*/,   &lpelf->elfScript[0] ) == 0 )
		  && lpelf->elfFullName[0] != L'@' ) || 
		( EnumFontData->JapanOnlyFlag == FALSE && lpelf->elfFullName[0] != L'@' ) )
	{
		// 同じフォント名が以前にもあった場合は弾く
		{
			int i ;

			for( i = 0 ; i < EnumFontData->FontNum ; i ++ )
			{
				if( _WCSCMP( &lpelf->elfFullName[0], &EnumFontData->FontBuffer[64 * i] ) == 0 )
				{
					return TRUE ;
				}
			}
		}

		// ネームを保存する
		_WCSCPY( &EnumFontData->FontBuffer[ 64 * EnumFontData->FontNum ], &lpelf->elfFullName[0] ) ;

		// フォントの数を増やす
		EnumFontData->FontNum ++ ;

		// もしバッファの数が限界に来ていたら列挙終了
		if( EnumFontData->BufferNum != 0 && EnumFontData->BufferNum == EnumFontData->FontNum )
		{
			return FALSE ;
		}
	}

	// 終了
	return TRUE ;
}

// フォント列挙用コールバック関数
int CALLBACK EnumFontFamExProcEx( ENUMLOGFONTEXW *lpelf, NEWTEXTMETRICEXW * /*lpntm*/, int /*nFontType*/, LPARAM lParam )
{
	ENUMFONTDATA *EnumFontData = ( ENUMFONTDATA * )lParam ;

	// フォントバッファが無効な場合はフォント名のみチェックする
	if( EnumFontData->FontBuffer == NULL )
	{
		if( EnumFontData->EnumFontName != NULL )
		{
			if( _WCSCMP( &lpelf->elfFullName[ 0 ], EnumFontData->EnumFontName ) == 0 )
			{
				EnumFontData->Valid = TRUE ;
			}
		}
	}
	else
	{
		// 横向きフォント(@付)はいずれもはじく
		if( lpelf->elfFullName[0] != L'@' )
		{
			int i ;

			// 同じフォント名が以前にもあった場合は弾く
			for( i = 0 ; i < EnumFontData->FontNum ; i ++ )
			{
				if( _WCSCMP( &lpelf->elfFullName[0], &EnumFontData->FontBuffer[64 * i] ) == 0 )
				{
					return TRUE ;
				}
			}

			// ネームを保存する
			_WCSCPY( &EnumFontData->FontBuffer[ 64 * EnumFontData->FontNum ], &lpelf->elfFullName[0] ) ;

			// フォントの数を増やす
			EnumFontData->FontNum ++ ;

			// もしバッファの数が限界に来ていたら列挙終了
			if( EnumFontData->BufferNum != 0 && EnumFontData->BufferNum == EnumFontData->FontNum )
			{
				return FALSE ;
			}
		}
	}

	// 終了
	return TRUE ;
}

// EnumFontName の環境依存処理を行う関数
extern int EnumFontName_PF( ENUMFONTDATA *EnumFontData, int IsEx, int CharSet )
{
	HDC			hdc ;
	LOGFONTW	LogFont ;

	// デバイスコンテキストを取得
	hdc = GetDC( NULL );

	// 列挙開始
	_MEMSET( &LogFont, 0, sizeof( LOGFONT ) ) ;
	if( IsEx )
	{
		LogFont.lfCharSet		= ( BYTE )( CharSet < 0 ? DEFAULT_CHARSET : CharSetTable[ CharSet ] ) ;
	}
	else
	{
		LogFont.lfCharSet		= DEFAULT_CHARSET ;
	}
	if( EnumFontData->EnumFontName != NULL )
	{
		_WCSNCPY( LogFont.lfFaceName, EnumFontData->EnumFontName, 31 ) ;
	}
	else
	{
		LogFont.lfFaceName[0]	= L'\0' ;
	}
	LogFont.lfPitchAndFamily	= 0 ;
	EnumFontFamiliesExW( hdc, &LogFont, ( FONTENUMPROCW )( IsEx ? EnumFontFamExProcEx : EnumFontFamExProc ), ( LPARAM )EnumFontData, 0  ) ;

	// デバイスコンテキストの解放
	ReleaseDC( NULL, hdc ) ;

	// 正常終了
	return 0 ;
}



#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

