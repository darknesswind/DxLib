// ----------------------------------------------------------------------------
// 
// 		ＤＸLibrary		ＤｉｒｅｃｔＤｒａｗ控制函数
// 
// 				Ver 3.11f
// 
// ----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

#include "DxCompileConfig.h"

#ifndef DX_NON_FONT

// インクルード----------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxFont.h"
#include "DxMask.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxUseCLib.h"
#include "DxBaseImage.h"
#include "DxGraphicsBase.h"
#include "DxGraphics.h"
#include "DxASyncLoad.h"
#include "DxLog.h"

#ifdef __WINDOWS__
#include "Windows/DxWindow.h"
#endif // __WINDOWS__


namespace DxLib
{

// マクロ定義------------------------------------------------------------------

// Font句柄有效性检测
#define FONTHCHK( HAND, HPOINT )		HANDLECHK(       DX_HANDLETYPE_FONT, HAND, *( ( HANDLEINFO ** )&HPOINT ) )
#define FONTHCHK_ASYNC( HAND, HPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_FONT, HAND, *( ( HANDLEINFO ** )&HPOINT ) )

// デフォルトフォントハンドル定型処理
#define DEFAULT_FONT_HANDLE_SETUP		\
	if( FontHandle == DX_DEFAULT_FONT_HANDLE )\
	{\
		RefreshDefaultFont() ;\
		FontHandle = FSYS.DefaultFontHandle ;\
	}

#define FONTEDGE_PATTERN_NUM	(4)			// 用意する文字の縁のパターンの数

#define FSYS FontSystem

// 構造体型宣言----------------------------------------------------------------

// フォント列挙時用データ構造体
typedef struct tugENUMFONTDATA
{
	TCHAR *FontBuffer ;
	int FontNum ;
	int BufferNum ;
	int JapanOnlyFlag ;
	int Valid ;
	const TCHAR *CheckFontName ;
} ENUMFONTDATA, *LPENUMFONTDATA ;

// データ宣言------------------------------------------------------------------

// キャラセットテーブル
const DWORD CharSetTable[] =
{
	DEFAULT_CHARSET,
	SHIFTJIS_CHARSET,
	HANGEUL_CHARSET,
	CHINESEBIG5_CHARSET,
	GB2312_CHARSET
} ;

const unsigned char Japanese1[ 7 ]      = { 0x93, 0xfa, 0x96, 0x7b, 0x8c, 0xea, 0x00 } ;													// 日本語
const unsigned char Japanese2[ 5 ]      = { 0xa4, 0xe9, 0xa4, 0xe5, 0x00 } ;																// 日文

// 文字の縁のパターンテーブル
static unsigned char _FontEdgePattern[FONTEDGE_PATTERN_NUM][FONTEDGE_PATTERN_NUM*2+1][FONTEDGE_PATTERN_NUM*2+1] =
{
	// 1
	{
		{0,1,0},
		{1,1,1},
		{0,1,0},
	},
	
	// 2
	{
		{0,1,1,1,0},
		{1,1,1,1,1},
		{1,1,1,1,1},
		{1,1,1,1,1},
		{0,1,1,1,0},
	},
	
	// 3
	{
		{0,0,0,1,0,0,0},
		{0,1,1,1,1,1,0},
		{0,1,1,1,1,1,0},
		{1,1,1,1,1,1,1},
		{0,1,1,1,1,1,0},
		{0,1,1,1,1,1,0},
		{0,0,0,1,0,0,0},
	},
	
	// 4
	{
		{0,0,1,1,1,1,1,0,0},
		{0,1,1,1,1,1,1,1,0},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1},
		{0,1,1,1,1,1,1,1,0},
		{0,0,1,1,1,1,1,0,0},
	},
} ;

FONTSYSTEM FontSystem ;

// 関数プロトタイプ宣言--------------------------------------------------------

static	FONTDATA *FontCacheCharAddToHandle( int AddNum, const TCHAR *CharData, int FontHandle ) ;	// 文字キャッシュに新しい文字を加える
static	int FontCacheStringAddToHandle(		int FontHandle, const TCHAR *String, int StrLen = -1,
											TCHAR *DrawStrBuffer = NULL, int *DrawCharNumP = NULL ) ;	// 文字キャッシュに新しい文字を加える

static int DrawStringHardware(				int xi, int yi, float xf, float yf, int PosIntFlag,                                 const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag ) ;
static int DrawExtendStringHardware(		int xi, int yi, float xf, float yf, int PosIntFlag, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag ) ;

static int DrawStringSoftware(				int x, int y,                                 const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag ) ;
static int DrawExtendStringSoftware(		int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag ) ;

// プログラムコード------------------------------------------------------------

static int DrawStringHardware( int xi, int yi, float xf, float yf, int PosIntFlag, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	FONTMANAGE *Font ;

	// 描画先のグラフィックの情報と使用するフォントの情報を取得する
	Font = GetFontManageDataToHandle( FontHandle ) ;

	// フォントにテクスチャキャッシュが使用されている場合
	if( Font->TextureCacheFlag )
	{
		RefreshFontDrawResourceToHandle( FontHandle ) ;
		FontCacheStringDrawToHandleST(
			TRUE,
			xi,
			yi,
			xf,
			yf,
			PosIntFlag, 
			false,
			1.0,
			1.0,
			String,
			Color,
			NULL,
			&GBASE.DrawArea,
			TRUE,
			FontHandle,
			EdgeColor,
			lstrlen( String ),
			VerticalFlag,
			NULL
		) ;
		return 0 ;
	}
	else
	{
	}

	// 終了
	return 0 ;
}


static int DrawExtendStringHardware( int xi, int yi, float xf, float yf, int PosIntFlag, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	FONTMANAGE * Font ;

	// 描画先のグラフィックの情報と使用するフォントの情報を取得する
	Font = GetFontManageDataToHandle( FontHandle ) ;

	// フォントにテクスチャキャッシュが使用されている場合
	if( Font->TextureCacheFlag )
	{
		// 描画先が３Ｄデバイスによる描画が出来ない場合はエラー
		RefreshFontDrawResourceToHandle( FontHandle ) ;
		FontCacheStringDrawToHandleST(
			TRUE,
			xi,
			yi,
			xf,
			yf,
			PosIntFlag,
			true,
			ExRateX,
			ExRateY,
			String,
			Color,
			NULL,
			&GBASE.DrawArea,
			TRUE,
			FontHandle,
			EdgeColor,
			lstrlen( String ),
			VerticalFlag,
			NULL
		) ;
		return 0 ;
	}
	else
	{
	}

	// 終了
	return 0 ;
}

static int DrawStringSoftware( int x, int y, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	SIZE DrawSize ;
	FONTMANAGE * ManageData ;
	RECT DrawArea, ClipRect, BRect, Rect ;
	DWORD bright ;
	MEMIMG *ScreenImg ;

	ManageData = GetFontManageDataToHandle( FontHandle ) ;

	// 描画領域を得る
	FontCacheStringDrawToHandleST(
		FALSE,
		0,
		0,
		0.0f,
		0.0f,
		TRUE,
		false,
		1.0,
		1.0,
		String,
		0,
		NULL,
		NULL,
		FALSE,
		FontHandle,
		0,
		-1,
		VerticalFlag,
		&DrawSize
	) ;

	// ブレンドモードによって処理を分岐
	if( GBASE.BlendMode != DX_BLENDMODE_NOBLEND || ( ManageData->FontType & DX_FONTTYPE_ANTIALIASING ) )
	{
		// アンチエイリアスでもなくアルファブレンドで且つアルファ値が最大のときは通常描画
		if( ( ManageData->FontType & DX_FONTTYPE_ANTIALIASING ) == 0 &&
			GBASE.BlendMode == DX_BLENDMODE_ALPHA && GBASE.BlendParam == 255 ) goto NORMALDRAW ;

		// 使用する仮スクリーンの決定
		ScreenImg = ( ManageData->FontType & DX_FONTTYPE_ANTIALIASING ) ? &GRS.FontScreenMemImgAlpha : &GRS.FontScreenMemImgNormal ;

		// 元の描画矩形と描画輝度を保存
		BRect  = GBASE.DrawArea ;
		bright = GBASE.bDrawBright ;

		// 仮バッファは画面サイズ分しかないので、そこでクリップされるようにする
		{
			ClipRect.top    = 0 ;
			ClipRect.left   = 0 ;
			ClipRect.right  = GRA2.MainScreenSizeX ;
			ClipRect.bottom = GRA2.MainScreenSizeY ;

			DrawArea.left   = 0 ; 
			DrawArea.right  = DrawSize.cx ; 
			DrawArea.top    = 0 ; 
			DrawArea.bottom = DrawSize.cy ;
			RectClipping( &DrawArea, &ClipRect ) ;

			GBASE.DrawArea  = DrawArea ;
			SetMemImgDrawArea( &DrawArea ) ;
		}

		// 描画バッファを透過色で塗りつぶす
		{
			// 透過色の調整
			if( ManageData->FontType & DX_FONTTYPE_ANTIALIASING )
			{
				ScreenImg->Base->TransColor = 0 ;
			}
			else
			{
				while(  Color     == (int)ScreenImg->Base->TransColor ||
						EdgeColor == (int)ScreenImg->Base->TransColor )
				{
					ScreenImg->Base->TransColor ++ ;
				}
			}

			ClearMemImg( ScreenImg, &DrawArea, ScreenImg->Base->TransColor ) ;
		}

		// アンチエイリアスを使用する場合

		// 一時バッファに描画
		FontCacheStringDrawToHandleST(
			TRUE,
			0,
			0,
			0.0f,
			0.0f,
			TRUE,
			false,
			1.0,
			1.0,
			String,
			Color,
			ScreenImg,
			&DrawArea,
			TRUE,
			FontHandle,
			EdgeColor,
			-1,
			VerticalFlag,
			NULL
		) ;

		// 本描画先に描画
		{
			GBASE.bDrawBright = 0xffffff ;
			Rect.left   = x ;
			Rect.top    = y ;
			Rect.right  = x + DrawArea.right ;
			Rect.bottom = y + DrawArea.bottom ;
			RectClipping( &Rect, &BRect ) ;
			GBASE.DrawArea = Rect ;
			SetMemImgDrawArea( &Rect ) ;

			DrawMemImg( GRS.TargetMemImg, ScreenImg, x, y, TRUE, GRS.BlendMemImg ) ;
		}

		// パラメータを元に戻す
		GBASE.DrawArea    = BRect ;
		SetMemImgDrawArea( &BRect ) ;
		GBASE.bDrawBright = bright ;

		// 終了
		return 0 ;
	}

NORMALDRAW:

	FontCacheStringDrawToHandleST(
		TRUE,
		x,
		y,
		( float )x, 
		( float )y,
		TRUE,
		false,
		1.0,
		1.0,
		String,
		Color,
		GRS.TargetMemImg,
		&GBASE.DrawArea,
		TRUE,
		FontHandle,
		EdgeColor,
		-1,
		VerticalFlag,
		NULL
	) ;

	// 終了
	return 0 ;
}

static int DrawExtendStringSoftware( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	SIZE DrawSize ;
	FONTMANAGE * ManageData ;
	RECT DrawArea, ClipRect, BRect, Rect, DrawRect ;
	DWORD bright ;
	MEMIMG *ScreenImg ;

	ManageData = GetFontManageDataToHandle( FontHandle ) ;

	// 描画領域を得る
	FontCacheStringDrawToHandleST(
		FALSE,
		0,
		0,
		0.0f,
		0.0f,
		TRUE,
		false,
		1.0,
		1.0,
		String,
		0,
		NULL,
		NULL,
		FALSE,
		FontHandle,
		0,
		-1,
		VerticalFlag,
		&DrawSize
	) ;

	// 使用する仮スクリーンの決定
	ScreenImg = ( ManageData->FontType & DX_FONTTYPE_ANTIALIASING ) ? &GRS.FontScreenMemImgAlpha : &GRS.FontScreenMemImgNormal ;

	// 元の描画矩形と描画輝度を保存
	BRect  = GBASE.DrawArea ;
	bright = GBASE.bDrawBright ;

	// 仮バッファは画面サイズ分しかないので、そこでクリップされるようにする
	{
		ClipRect.top    = 0 ;
		ClipRect.left   = 0 ;
		ClipRect.right  = GRA2.MainScreenSizeX ;
		ClipRect.bottom = GRA2.MainScreenSizeY ;

		DrawArea.left   = 0 ; 
		DrawArea.right  = DrawSize.cx ; 
		DrawArea.top    = 0 ; 
		DrawArea.bottom = DrawSize.cy ;
		RectClipping( &DrawArea, &ClipRect ) ;

		GBASE.DrawArea  = DrawArea ;
		SetMemImgDrawArea( &DrawArea ) ;
	}

	// 描画バッファを透過色で塗りつぶす
	{
		RECT ClearRect ;

		// 透過色の調整
		if( ManageData->FontType & DX_FONTTYPE_ANTIALIASING )
		{
			ScreenImg->Base->TransColor = 0 ;
		}
		else
		{
			while(  Color     == (int)ScreenImg->Base->TransColor ||
					EdgeColor == (int)ScreenImg->Base->TransColor )
			{
				ScreenImg->Base->TransColor ++ ;
			}
		}

		ClearRect = DrawArea ;
		ClearRect.right  += 4 ;
		ClearRect.bottom += 4 ;
		RectClipping( &ClearRect, &ClipRect ) ;
		ClearMemImg( ScreenImg, &ClearRect, ScreenImg->Base->TransColor ) ;
	}

	// アンチエイリアスを使用する場合

	// 一時バッファに描画
	FontCacheStringDrawToHandleST(
		TRUE,
		0,
		0,
		0.0f,
		0.0f,
		TRUE,
		false,
		1.0,
		1.0,
		String,
		Color,
		ScreenImg,
		&DrawArea,
		TRUE,
		FontHandle,
		EdgeColor,
		-1,
		VerticalFlag,
		NULL
	) ;

	// 本描画先に描画
	{
		GBASE.bDrawBright = 0xffffff ;
		Rect.left   = x ;
		Rect.top    = y ;
		Rect.right  = x + _DTOL( DrawArea.right * ExRateX ) + 3 ;
		Rect.bottom = y + _DTOL( DrawArea.bottom * ExRateY ) + 3  ;
		RectClipping( &Rect, &BRect ) ;
		GBASE.DrawArea = Rect ;
		SetMemImgDrawArea( &Rect ) ;

		DrawRect.left   = x ;
		DrawRect.top    = y ;
		DrawRect.right  = x + _DTOL( GRA2.MainScreenSizeX * ExRateX ) + 3 ;
		DrawRect.bottom = y + _DTOL( GRA2.MainScreenSizeY * ExRateY ) + 3 ;

		DrawEnlargeMemImg( GRS.TargetMemImg, ScreenImg, &DrawRect, TRUE, GRS.BlendMemImg ) ;
	}

	// パラメータを元に戻す
	GBASE.DrawArea    = BRect ;
	SetMemImgDrawArea( &BRect ) ;
	GBASE.bDrawBright = bright ;

	// 終了
	return 0 ;
}

// フォントシステムの初期化
extern int InitFontManage( void )
{
	int i ;
	int j ;
	int k ;

	if( FSYS.InitializeFlag == TRUE )
		return -1 ;

	// 初期化フラグを立てる
	FSYS.InitializeFlag = TRUE ;

	// フォントハンドル管理情報を初期化する
	InitializeHandleManage( DX_HANDLETYPE_FONT, sizeof( FONTMANAGE ), MAX_FONT_NUM, InitializeFontHandle, TerminateFontHandle, DXSTRING( _T( "フォント" ) ) ) ;

	// デフォルトフォントの設定をセット
	if( FSYS.EnableInitDefaultFontName == FALSE )
	{
		_MEMSET( FSYS.DefaultFontName, 0, sizeof( FSYS.DefaultFontName ) );
	}
	FSYS.EnableInitDefaultFontName = FALSE ;

	if( FSYS.EnableInitDefaultFontType == FALSE )
	{
		FSYS.DefaultFontType     = DEFAULT_FONT_TYPE ;
	}
	FSYS.EnableInitDefaultFontType = FALSE ;

	if( FSYS.EnableInitDefaultFontSize == FALSE )
	{
		FSYS.DefaultFontSize     = DEFAULT_FONT_SIZE ;
	}
	FSYS.EnableInitDefaultFontSize = FALSE ;

	if( FSYS.EnableInitDefaultFontThick == FALSE )
	{
		FSYS.DefaultFontThick    = DEFAULT_FONT_THINCK ;
	}
	FSYS.EnableInitDefaultFontThick = FALSE ;

	if( FSYS.EnableInitDefaultFontCharSet == FALSE )
	{
		FSYS.DefaultFontCharSet  = _GET_CHARSET() ;
	}
	FSYS.EnableInitDefaultFontCharSet = FALSE ;

	if( FSYS.EnableInitDefaultFontEdgeSize == FALSE )
	{
		FSYS.DefaultFontEdgeSize = DEFAULT_FONT_EDGESIZE ;
	}
	FSYS.EnableInitDefaultFontEdgeSize = FALSE ;

	if( FSYS.EnableInitDefaultFontItalic == FALSE )
	{
		FSYS.DefaultFontItalic   = FALSE ;
	}
	FSYS.EnableInitDefaultFontItalic = FALSE ;

	if( FSYS.EnableInitDefaultFontSpace == FALSE )
	{
		FSYS.DefaultFontSpace    = 0 ;
	}
	FSYS.EnableInitDefaultFontSpace = FALSE ;

	// テーブルを作成する
	for( i = 0 ; i < 256 ; i ++ )
	{
		j = i ;
		for( k = 0 ; j != 0 ; k ++, j &= j - 1 ){}
		FSYS.BitCountTable[ i ] = ( BYTE )k ;
	}

	// フォントハンドルの初期化
	InitFontToHandleBase() ;

	// 終了
	return 0 ;
}

// フォント制御の終了
extern int TermFontManage( void )
{
	if( FSYS.InitializeFlag == FALSE )
		return -1 ;

	// 全てのフォントを削除
	InitFontToHandleBase( TRUE ) ;

	// フォントハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_FONT ) ;

	// 初期化フラグを倒す
	FSYS.InitializeFlag = FALSE ;

	// 終了
	return 0 ;
}

// フォント、文字列描画関係関数

// フォント列挙用コールバック関数
int CALLBACK EnumFontFamExProc( ENUMLOGFONTEX *lpelf, NEWTEXTMETRICEX * /*lpntm*/, int nFontType, LPARAM lParam )
{
	ENUMFONTDATA *FontData = ( LPENUMFONTDATA )lParam ;

	// JapanOnlyフラグが立っていたら TrueType 、日本語フォントだけを列挙する
	// 横向きフォント(@付)はいずれもはじく
	if( ( FontData->JapanOnlyFlag == TRUE && ( nFontType & TRUETYPE_FONTTYPE ) &&
#ifdef UNICODE
		( lstrcmp( _T( "日本語" ), ( TCHAR * )&lpelf->elfScript[0] ) == 0 ||
		  lstrcmp( _T( "日文" ),   ( TCHAR * )&lpelf->elfScript[0] ) == 0 )
#else
		( lstrcmp( ( char * )Japanese1, ( char * )&lpelf->elfScript[0] ) == 0 ||
		  lstrcmp( ( char * )Japanese2, ( char * )&lpelf->elfScript[0] ) == 0 )
#endif
		  && lpelf->elfFullName[0] != _T( '@' ) ) || 
		( FontData->JapanOnlyFlag == FALSE && lpelf->elfFullName[0] != _T( '@' ) ) )
	{
		// 同じフォント名が以前にもあった場合は弾く
		{
			int i ;

			for( i = 0 ; i < FontData->FontNum ; i ++ )
				if( lstrcmp( ( TCHAR * )&lpelf->elfFullName[0], &FontData->FontBuffer[64 * i] ) == 0 ) return TRUE ;
		}

		// ネームを保存する
		lstrcpy( &FontData->FontBuffer[ 64 * FontData->FontNum ], ( TCHAR * )&lpelf->elfFullName[0] ) ;

		// フォントの数を増やす
		FontData->FontNum ++ ;

		// もしバッファの数が限界に来ていたら列挙終了
		if( FontData->BufferNum != 0 && FontData->BufferNum == FontData->FontNum ) return FALSE ;
	}

	// 終了
	return TRUE ;
}

// フォント列挙用コールバック関数
int CALLBACK EnumFontFamExProcEx( ENUMLOGFONTEX *lpelf, NEWTEXTMETRICEX * /*lpntm*/, int /*nFontType*/, LPARAM lParam )
{
	ENUMFONTDATA *FontData = ( LPENUMFONTDATA )lParam ;

	// チェック用フォント名がある場合はフォント名をチェックする
	if( FontData->CheckFontName != NULL )
	{
		if( lstrcmp( ( TCHAR * )&lpelf->elfFullName[ 0 ], FontData->CheckFontName ) == 0 )
		{
			FontData->Valid = TRUE ;
		}
	}

	// 横向きフォント(@付)はいずれもはじく
	if( lpelf->elfFullName[0] != _T( '@' ) )
	{
		// フォントバッファが有効な場合のみフォント名を保存する
		if( FontData->FontBuffer != NULL )
		{
			int i ;

			// 同じフォント名が以前にもあった場合は弾く
			for( i = 0 ; i < FontData->FontNum ; i ++ )
				if( lstrcmp( ( TCHAR * )&lpelf->elfFullName[0], &FontData->FontBuffer[64 * i] ) == 0 ) return TRUE ;

			// ネームを保存する
			lstrcpy( &FontData->FontBuffer[ 64 * FontData->FontNum ], ( TCHAR * )&lpelf->elfFullName[0] ) ;

			// フォントの数を増やす
			FontData->FontNum ++ ;

			// もしバッファの数が限界に来ていたら列挙終了
			if( FontData->BufferNum != 0 && FontData->BufferNum == FontData->FontNum ) return FALSE ;
		}
	}

	// 終了
	return TRUE ;
}

// フォントのキャッシュ情報を初期化する
extern int InitCacheFontToHandle( void )
{
	int i, Handle ;
	FONTMANAGE * ManageData ;
	TCHAR FontName[256] ;
	CREATEFONTTOHANDLE_GPARAM GParam ;
	int Space, Size, Thick, FontType, CharSet, EdgeSize, Italic ;

	if( HandleManageArray[ DX_HANDLETYPE_FONT ].InitializeFlag == FALSE )
		return -1 ;

	for( i = HandleManageArray[ DX_HANDLETYPE_FONT ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_FONT ].AreaMax ; i ++ )
	{
		ManageData = ( FONTMANAGE * )HandleManageArray[ DX_HANDLETYPE_FONT ].Handle[ i ] ;
		if( ManageData == NULL ) continue ;

		lstrcpy( FontName, ManageData->FontName ) ;
		Size		= ManageData->FontSize ;
		Thick		= ManageData->FontThickness ;
		FontType	= ManageData->FontType ;
		CharSet		= ManageData->CharSet ;
		EdgeSize	= ManageData->EdgeSize ;
		Italic		= ManageData->Italic ;
		Space       = ManageData->Space ;

//		DXST_ERRORLOG_ADD( _T( "Tarpu" ) ) ;

		// フォントハンドルの作り直し
		Handle = ManageData->HandleInfo.Handle ;
		NS_DeleteFontToHandle( Handle ) ;
		InitCreateFontToHandleGParam( &GParam ) ;
		CreateFontToHandle_UseGParam( &GParam, FontName, Size, Thick, FontType, CharSet, EdgeSize, Italic, Handle, FALSE ) ;
		NS_SetFontSpaceToHandle( Space, Handle ) ;
	}

	// 終了
	return 0 ;
}

// 特定のフォントのキャッシュ情報を初期化する
extern int InitFontCacheToHandle( int FontHandle, int ASyncThread )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( ASyncThread )
	{
		if( FONTHCHK_ASYNC( FontHandle, ManageData ) )
			return -1  ;
	}
	else
	{
		if( FONTHCHK( FontHandle, ManageData ) )
			return -1  ;
	}

	// テキストキャッシュデータの初期化
	{
		int i ;
		FONTCODEDATA *CodeData ;
		FONTDATA * FontData ;

		_MEMSET( ManageData->FontCodeData, 0, sizeof( FONTCODEDATA ) * 0x10000 ) ;
		_MEMSET( ManageData->FontData, 0, sizeof( FONTDATA ) * ( FONT_CACHE_MAXNUM + 1 ) ) ;

		CodeData = ManageData->FontCodeData ;
		FontData = ManageData->FontData ;
		for( i = 0 ; i < ManageData->MaxCacheCharNum ; i ++, FontData ++ )
		{
			FontData->GraphIndex = i ;	
		}
		FontData->GraphIndex = -1 ;

		ManageData->Index = 0 ;
	}

	// 終了
	return 0 ;
}

// フォントハンドルが使用する描画バッファやテクスチャキャッシュを再初期化する
extern int RefreshFontDrawResourceToHandle( int FontHandle, int ASyncThread )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( ASyncThread )
	{
		if( FONTHCHK_ASYNC( FontHandle, ManageData ) )
			return -1  ;
	}
	else
	{
		if( FONTHCHK( FontHandle, ManageData ) )
			return -1  ;
	}

	// テクスチャキャッシュを使用するフォントハンドルで、何時の間にか画面が
	// MEMIMG を使用するモードになっていたらテクスチャキャッシュを使用しないハンドルとして作り直す
/*	if( USEMEMIMG && ManageData->TextureCacheFlag == TRUE )
	{
		int ID, DataIndex, Size, Thick, FontType, CharSet, EdgeSize ;
		char FontName[128] ;

		ID        = ManageData->ID                   ;
		DataIndex = FontHandle & DX_HANDLEINDEX_MASK ;
		Size      = ManageData->FontSize             ;
		Thick     = ManageData->FontThickness        ;
		FontType  = ManageData->FontType             ;
		CharSet   = ManageData->CharSet              ;
		EdgeSize  = ManageData->EdgeSize             ;
		_STRCPY( FontName, ManageData->FontName ) ;

		DeleteFontToHandle( FontHandle ) ;
		FontHandle = CreateFontToHandle( FontName, Size, Thick, FontType, CharSet, EdgeSize, ID, DataIndex ) ;

		// エラー判定
		if( FONTHCHK( FontHandle, ManageData ) )
		return -1  ;
	}
*/

	// TextureCache のロスト判定
	if( ManageData->TextureCacheLostFlag == TRUE )
	{
		int Use3D, w ;
		SETUP_GRAPHHANDLE_GPARAM GParam ;

		ManageData->TextureCacheLostFlag = FALSE ;

		if( ( ManageData->FontType & DX_FONTTYPE_ANTIALIASING ) == 0 )
		{
			InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam, 16, FALSE, TRUE ) ;
		}
		else
		{
			InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam, ManageData->TextureCacheColorBitDepth, TRUE, FALSE ) ;
		}

		Use3D = NS_GetUse3DFlag() ;
		NS_SetUse3DFlag( TRUE ) ;

		// エッジつきの場合は横幅を倍にする
		w = ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 ? ManageData->SurfaceSize.cx * 2 : ManageData->SurfaceSize.cx ;
		ManageData->TextureCache = MakeGraph_UseGParam( &GParam, w, ManageData->SurfaceSize.cy, FALSE, FALSE, ASyncThread ) ;
		ManageData->TextureCacheSub = -1 ;
		
		// エッジ付きの場合は派生させる
		if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
		{
			ManageData->TextureCacheSub = DerivationGraph_UseGParam( ManageData->SurfaceSize.cx, 0, ManageData->SurfaceSize.cx * 2, ManageData->SurfaceSize.cy, ManageData->TextureCache, ASyncThread ) ;
		}

		NS_SetUse3DFlag( Use3D ) ;

		// エラーチェック
		if( ManageData->TextureCache < 0 ) return -1 ;
		NS_SetGraphLostFlag( ManageData->TextureCache, &ManageData->TextureCacheLostFlag ) ;

		// 透明色で塗りつぶす
/*		if( ManageData->TextureCacheUsePremulAlpha )
		{
			FillGraph_UseGParam( ManageData->TextureCache, 0,0,0,0, ASyncThread ) ;
			if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
			{
				FillGraph_UseGParam( ManageData->TextureCacheSub, 0,0,0,0, ASyncThread ) ;
			}
		}
		else
		{
			FillGraph_UseGParam( ManageData->TextureCache, 255,255,255,0, ASyncThread ) ;
			if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
			{
				FillGraph_UseGParam( ManageData->TextureCacheSub, 255,255,255,0, ASyncThread ) ;
			}
		}
*/
		// フォントのキャッシュ情報を初期化する
		InitFontCacheToHandle( FontHandle, ASyncThread ) ;
	}

	// 終了
	return 0 ;
}

// デフォルトフォントを再作成する
extern int RefreshDefaultFont( void )
{
	FONTMANAGE * ManageData ;
	
	// 設定が変わっていない場合は再作成しない
	if( FSYS.DefaultFontHandle > 0 )
	{
		int FontType, Size, Thick, CharSet, EdgeSize, Italic ;
		TCHAR *FontName ;

		ManageData = GetFontManageDataToHandle( FSYS.DefaultFontHandle ) ;

		FontName	= FSYS.DefaultFontName ;
		FontType 	= FSYS.DefaultFontType ;
		Size 		= FSYS.DefaultFontSize ;
		Thick 		= FSYS.DefaultFontThick ;
		CharSet 	= FSYS.DefaultFontCharSet ;
		EdgeSize	= ( FSYS.DefaultFontType & DX_FONTTYPE_EDGE ) ? FSYS.DefaultFontEdgeSize : -1 ;
		Italic		= FSYS.DefaultFontItalic ;

		if( FontType 	== -1 ) FontType  = DEFAULT_FONT_TYPE ;
		if( EdgeSize	> 1   )	FontType |= DX_FONTTYPE_EDGE ;
		if( Size 		== -1 ) Size      = DEFAULT_FONT_SIZE ;
		if( Thick 		== -1 ) Thick     = DEFAULT_FONT_THINCK ;
		if( CharSet		== -1 ) CharSet   = _GET_CHARSET() ;
		if( EdgeSize	== -1 ) EdgeSize  = DEFAULT_FONT_EDGESIZE ;
		
		if( lstrcmp( FSYS.DefaultFontName, ManageData->FontName ) == 0 &&
			FontType 	== ManageData->FontType &&
			Size 		== ManageData->FontSize &&
			Thick 		== ManageData->FontThickness &&
			CharSet		== ManageData->CharSet &&
			EdgeSize	== ManageData->EdgeSize &&
			Italic		== ManageData->Italic )
			return 0 ;

		NS_DeleteFontToHandle( FSYS.DefaultFontHandle ) ;
	}

	// 設定が変わっているかフォントハンドルが無効になっている場合は再作成する	
	CREATEFONTTOHANDLE_GPARAM GParam ;
	InitCreateFontToHandleGParam( &GParam ) ;
	FSYS.DefaultFontHandle = CreateFontToHandle_UseGParam(
		&GParam,
		FSYS.DefaultFontName,
		FSYS.DefaultFontSize,
		FSYS.DefaultFontThick,
		FSYS.DefaultFontType,
		FSYS.DefaultFontCharSet,
		( FSYS.DefaultFontType & DX_FONTTYPE_EDGE ) ? FSYS.DefaultFontEdgeSize : -1,
		FSYS.DefaultFontItalic,
		-1,
		FALSE ) ;
	NS_SetDeleteHandleFlag( FSYS.DefaultFontHandle, &FSYS.DefaultFontHandle ) ;
	NS_SetFontSpaceToHandle( FSYS.DefaultFontSpace, FSYS.DefaultFontHandle ) ;

	return 0 ;
}



// 文字キャッシュに新しい文字を加える
static int FontCacheStringAddToHandle( int FontHandle, const TCHAR *String, int StrLen,
										TCHAR *DrawStrBufferP, int *DrawCharNumP )
{
	static TCHAR addbuf[ 256 * 3 ] ;	// キャッシュに追加する際に使用する文字列データ（３バイトで１文字を表記）
	static TCHAR temp[256 * 3] ;
	static WORD resetExist[256] ;

	FONTMANAGE * ManageData ;
	FONTCODEDATA * fontcode ;
	int i ;						// 繰り返しと汎用変数
//	bool Hangeul ;				// ＵＮＩコードかどうか
	TCHAR *drstr, *adstr ;		// それぞれDrawStrBuf,CacheAddBufのポインタ
	WORD *rexist ;
	int addnum ;				// キャッシュに追加する文字の数
	int drawnum ;				// ２バイト文字１バイト文字全部ひっくるめて何文字あるか保存する変数
	int len ;					// 文字列の長さ
	bool UseAlloc ;
	TCHAR *AllocDrawStr = NULL ;
	WORD *AllocResetExist = NULL ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1  ;

//	Hangeul = ManageData->CharSet == DX_CHARSET_HANGEUL ;
	UseAlloc = false ;

	// 文字列の長さをセット
//	len = /*Hangeul ? wcslen( ( wchar_t * )String ) :*/ lstrlen( String ) ;
	len = lstrlen( String ) ;
	if( StrLen != -1 && StrLen < len ) len = StrLen ;

	// デフォルトバッファのサイズを超える場合はテンポラリバッファの確保
	if( len > 256 )
	{
		UseAlloc = true ;
		AllocDrawStr = ( TCHAR * )DXALLOC( ( len * 3 ) * sizeof( TCHAR ) + len * sizeof( WORD ) ) ;
		AllocResetExist = ( WORD * )( AllocDrawStr + len * 3 ) ;
		if( AllocDrawStr == NULL )
			return -1 ;
	}
	
	addnum = 0 ;
	drawnum = 0 ;
	drstr = DrawStrBufferP != NULL ? DrawStrBufferP : ( UseAlloc ? AllocDrawStr : temp ) ;
	adstr = UseAlloc ? AllocDrawStr : addbuf ;
	rexist = UseAlloc ? AllocResetExist : resetExist ;
	fontcode = ManageData->FontCodeData ;
	for( i = 0 ; i < len ; drstr += 3 )
	{
#ifdef UNICODE
		// ４バイト文字か２バイト文字か判断、バッファに保存
		if( _TMULT( *String, ManageData->CharSet ) == TRUE )
		{
			// ４バイト文字には非対応
			String += 2 ;
			i += 2 ;
			continue ;
		}
		else
		{
			drstr[ 0 ] = *String ;
			drstr[ 1 ] = 0 ;
			drstr[ 2 ] = 0 ;
			String ++ ;
			i ++ ;
			drawnum ++ ;
		}
#else
		// ２バイト文字か１バイト文字か判断、バッファに保存
		if( _TMULT( *String, ManageData->CharSet ) == TRUE )
		{
			drstr[ 0 ] = String[ 0 ] ;
			drstr[ 1 ] = String[ 1 ] ;
			drstr[ 2 ] = 0 ;

			String += 2 ;
			i += 2 ;
			drawnum ++ ;
		}
		else
		{
			drstr[ 0 ] = *String ;
			drstr[ 1 ] = 0 ;
			drstr[ 2 ] = 0 ;

			String ++ ;
			i ++ ;
			drawnum ++ ;
		}
#endif

		// キャッシュに存在しない文字だった場合キャッシュ文字の一括追加をするためのバッファに保存、キャッシュに追加する文字数を増やす
		if( fontcode[ *((WORD *)drstr) ].ExistFlag == false &&
			addnum != ManageData->MaxCacheCharNum )
		{
			fontcode[ *((WORD *)drstr) ].ExistFlag = true ;
			rexist[ addnum ] = *((WORD *)drstr) ;

			addnum ++ ;
			*((WORD *)adstr) = *((WORD *)drstr) ;
			adstr[2] = 0 ;
			adstr += 3 ;
		}
	}

	for( i = 0 ; i < addnum ; i ++ )
	{
		fontcode[ rexist[ i ] ].ExistFlag = false ;
	}

	// キャッシュ文字の一括追加を行う
	if( addnum != 0 )
	{
		FontCacheCharAddToHandle( addnum, UseAlloc ? AllocDrawStr : addbuf, FontHandle ) ;
	}

	// 描画する文字の数を保存する
	if( DrawCharNumP != NULL ) *DrawCharNumP = drawnum ;

	// メモリを確保していたら解放
	if( UseAlloc )
	{
		DXFREE( AllocDrawStr ) ;
	}

	// 終了
	return 0 ;
}

// 文字キャッシュに新しい文字を加える
static FONTDATA * FontCacheCharAddToHandle( int AddNum, const TCHAR *CharData, int FontHandle )
{
	HDC 			DC ;
	FONTMANAGE * 	ManageData ;
	FONTCODEDATA * 	CodeData ;
	FONTDATA * 		FontData = NULL ;
	const TCHAR 	*StrPoint ;
	HFONT 			OldFont ;
	HBITMAP			OldBmp = NULL ;
	TEXTMETRIC 		met ;
	int 			i ;
	int				SampleScale = 1 ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( GBASE.NotDrawFlag || NS_GetUseDDrawObj() == NULL ) return NULL ;
	if( FONTHCHK( FontHandle, ManageData ) )
		return NULL ;
//	DXST_ERRORLOG_ADD( _T( "フォントハンドル値が異常です終了します\n" ) ) ;
//	DXST_ERRORLOGFMT_ADD(( _T( "	in FontCacheCharAddToHandle  AddNum = %d   CharData = %s  FontHandle = %d  " ), AddNum, CharData, FontHandle )) ;

	// ＤＣの作成
	DC = CreateCompatibleDC( NULL ) ;
	if( DC == NULL )
	{
		DxLib_Error( DXSTRING( _T( "テキストキャッシュサーフェスのＤＣの取得に失敗しました" ) ) ) ;
		return NULL ;
	}

	// フォントをセット
	OldFont = ( HFONT )SelectObject( DC, ManageData->FontObj ) ;
	if( OldFont == NULL )
	{
		DeleteDC( DC ) ;
		DxLib_Error( DXSTRING( _T( "テキストキャッシュサーフェスのＤＣの取得に失敗しました" ) ) ) ;
		return NULL ;
	}

	// フォントの情報を取得
	GetTextMetrics( DC, &met ) ;

	// TextOut を使用するかどうかで処理を分岐
	if( ManageData->UseTextOut )
	{
		// 描画先ビットマップをセット
		OldBmp = ( HBITMAP )SelectObject( DC , ManageData->CacheBitmap ) ;

		// 文字の描画設定を行う
		{
			SetTextColor( DC , RGB( 255 , 255 , 255 ) ) ; 		// 色をセット	

			// 背景色をセット
			SetBkColor( DC , 0 ) ;
			SetBkMode( DC , OPAQUE ) ;							// 背景を塗りつぶす指定
		}
	}

	// 追加する文字の数だけループ
	{
		int AddX, AddY, AddSize ;
		int DestX, DestY ;

		if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) )
		{
			AddX = AddY = ManageData->EdgeSize ;
			AddSize = ManageData->EdgeSize * 2 ;
		}
		else
		{
			AddX = AddY = 0 ;
			AddSize = 0 ;
		}

		StrPoint = CharData ;
		for( i = 0 ; i < AddNum ; i ++, StrPoint += 3 )
		{
			// 次の文字を追加するインデックスを取得
			FontData = &ManageData->FontData[ ManageData->Index ] ;

			// もし追加しようとしているデータが以前に何かあったならば
			// そのデータを無効にする
			if( FontData->ValidFlag )
			{
				// コードデータを無効にする
				FontData->CodeData->ExistFlag = FALSE ;
				FontData->CodeData->DataPoint = NULL ;

				// フォントのデータも無効にする
				FontData->ValidFlag = FALSE ;
			}

			// コードのリスト型データに登録
			CodeData = &ManageData->FontCodeData[ *( (WORD *)StrPoint ) ] ; 

			// 文字を追加する座標のセット
			DestX =   FontData->GraphIndex / ManageData->LengthCharNum   * ManageData->MaxWidth ;
			DestY = ( FontData->GraphIndex % ManageData->LengthCharNum ) * ManageData->MaxWidth ;

			// キャッシュに文字イメージを転送する
			{
				int type = 0 ;
				int gettype = 0 ;
				unsigned int Code ;
				int Space ;
				int StrSize ;

				StrSize = lstrlen( StrPoint ) ;
#ifdef UNICODE
				// スペースかどうかを取得しておく
				Space = ( ( wchar_t * )StrPoint )[ 0 ] == L' ' || ( ( wchar_t * )StrPoint )[ 0 ] == L'　' ? 1 : 0 ;
				Code = *( ( WORD * )StrPoint ) ;
#else
				// スペースかどうかを取得しておく
				Space = StrPoint[0] == ' ' ? 1 : ( *( ( WORD * )StrPoint ) == *( ( WORD * )"　" ) ? 2 : 0 ) ;

				Code = 0 ;
				if( StrPoint[1] != 0 )
				{
					( ( BYTE * )&Code )[1] = StrPoint[0] ;
					( ( BYTE * )&Code )[0] = StrPoint[1] ;
					StrSize = 2 ;
				}
				else
				{
					( ( BYTE * )&Code )[0] = StrPoint[0] ;
					( ( BYTE * )&Code )[1] = StrPoint[1] ;
				}
#endif

				// 取得するイメージのスケールを取得しておく
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

				// 文字イメージを一時的に保存するメモリ領域を初期化
				if( ManageData->TextureCacheFlag == FALSE )
				{
					_MEMSET(	ManageData->CacheMem + DestY * ManageData->CachePitch,
								0,
								ManageData->CachePitch * ManageData->MaxWidth ) ;
				}
				else
				{
//					_MEMSET( ManageData->TextureTempCache.GraphData, 0, ManageData->TextureTempCache.Height * ManageData->TextureTempCache.Pitch ) ;

					if( ManageData->TextureCacheUsePremulAlpha || ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
					{
						NS_ClearRectBaseImage( &ManageData->TextureCacheBaseImage, DestX, DestY, ManageData->MaxWidth, ManageData->MaxWidth ) ;
						if( ManageData->FontType & DX_FONTTYPE_EDGE )
						{
							NS_ClearRectBaseImage( &ManageData->TextureCacheBaseImage, DestX + ManageData->SurfaceSize.cx, DestY, ManageData->MaxWidth, ManageData->MaxWidth ) ;
						}
					}
					else
					{
						NS_FillRectBaseImage( &ManageData->TextureCacheBaseImage, DestX, DestY, ManageData->MaxWidth, ManageData->MaxWidth, 255,255,255,0 ) ;
						if( ManageData->FontType & DX_FONTTYPE_EDGE )
						{
							NS_FillRectBaseImage( &ManageData->TextureCacheBaseImage, DestX + ManageData->SurfaceSize.cx, DestY, ManageData->MaxWidth, ManageData->MaxWidth, 255,255,255,0 ) ;
						}
					}
				}

				// テクスチャキャッシュを使用する場合は転送先座標を１ドット分ずらす
				if( ManageData->TextureCacheFlag == TRUE )
				{
					DestX ++ ;
					DestY ++ ;
				}

				// TextOut を使用するかどうかで処理を分岐
				if( ManageData->UseTextOut )
				{
					BYTE *Src ;
					int i, j ;
					int Height, Width ;
					DWORD SrcPitch ;
					DWORD DestPitch ;
					SIZE TempSize ;

					// 追加する文字の大きさを取得
					GetTextExtentPoint32( DC , StrPoint , StrSize , &TempSize );
					FontData->SizeX = ( short )TempSize.cx ;
					FontData->SizeY = ( short )TempSize.cy ;

					// 文字イメージを出力
					TextOut( DC , 0 , 0 , StrPoint , StrSize ) ;

					// 情報をセット
					FontData->DrawX = 0 ;
					FontData->DrawY = 0 ;
					FontData->AddX = FontData->SizeX ;

					// 取得したデータをキャッシュに転送
					Width = FontData->SizeX ;
					Height = FontData->SizeY ;
					Src = ManageData->CacheBitmapMem ;
					SrcPitch = ManageData->CacheBitmapMemPitch ;
					if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) == 0 )
					{
						if( ManageData->TextureCacheFlag == FALSE )
						{
							BYTE *Dest ;
							BYTE *dp ;
							BYTE dat ;
							BYTE bit ;

							DestPitch = ManageData->CachePitch ;
							Dest = ( BYTE * )ManageData->CacheMem + DestY * DestPitch ;
							for( i = 0 ; i < Height ; i ++ )
							{
								dp = Dest ;
								bit = 0x80 ;
								dat = 0 ;
								for( j = 0 ; j < Width ; j ++, bit >>= 1 )
								{
									if( j != 0 && j % 8 == 0 )
									{
										bit = 0x80 ;
										*dp = dat ;
										dp ++ ;
										dat = 0 ;
									}

									if( Src[ j ] != 0 )
									{
										dat |= bit ;
									}
								}
								*dp = dat ;

								Src  += SrcPitch ;
								Dest += DestPitch ;
							}
						}
						else
						{
							BYTE 		*Dest ;
							BASEIMAGE 	&im 	= ManageData->TextureCacheBaseImage ;
							COLORDATA 	&cl 	= im.ColorData ;
							WORD 		RGBMask ;
							WORD 		RGBAMask ;

							DestPitch = im.Pitch ;
							Dest = (BYTE *)im.GraphData + DestX * im.ColorData.PixelByte + DestY * im.Pitch ;

							RGBMask = ( WORD )( cl.RedMask | cl.GreenMask | cl.BlueMask ) ;
							RGBAMask = ( WORD )( RGBMask | cl.AlphaMask ) ;

							if( ManageData->TextureCacheUsePremulAlpha )
							{
								for( i = 0 ; i < Height ; i ++ )
								{
									for( j = 0 ; j < Width ; j ++ )
									{
										*( ( WORD * )Dest + j ) = Src[ j ] != 0 ? RGBAMask : 0 ;
									}

									Src  += SrcPitch ;
									Dest += DestPitch ;
								}
							}
							else
							{
								for( i = 0 ; i < Height ; i ++ )
								{
									for( j = 0 ; j < Width ; j ++ )
									{
										*( ( WORD * )Dest + j ) = Src[ j ] != 0 ? RGBAMask : RGBMask ;
									}

									Src  += SrcPitch ;
									Dest += DestPitch ;
								}
							}

							// テクスチャキャッシュに転送
							{
								RECT srect ;

								SETRECT( srect, DestX - 1, DestY - 1, ManageData->MaxWidth + DestX, ManageData->MaxWidth + DestY ) ;
								BltBmpOrGraphImageToGraph2Base( &im.ColorData, NULL, NULL, 
																FALSE, &im, NULL,
																&srect, DestX - 1, DestY - 1,
																ManageData->TextureCache ) ;
							}
						}
					}
					else
					{
						BYTE 	*Dest, *DestT ;
						int 	DestPointX,	DestPointY ;
						int 	i, j, l, n, m, o, EdgeSize ;
						int 	DestPitch ;
						unsigned char (*EdgePat)[FONTEDGE_PATTERN_NUM*2+1] ;

						DestPointX 	= AddX ;
						DestPointY 	= AddY ;

						EdgeSize = ManageData->EdgeSize ;
						EdgePat = _FontEdgePattern[ EdgeSize - 1 ] ;
						
						if( ManageData->TextureCacheFlag == FALSE )
						{
							DestPitch = ManageData->CachePitch ;

							switch( ManageData->FontType )
							{
							case DX_FONTTYPE_EDGE :
								Dest = ( BYTE * )ManageData->CacheMem + ( DestX + DestPointX ) + ( DestY + DestPointY ) * DestPitch ;

								if( EdgeSize == 1 )
								{
									for( i = 0 ; i < Height ; i ++ )
									{
										for( j = 0 ; j < Width ; j ++ )
										{
											if( Src[ j ] != 0 )
											{
												Dest[j] = 1 ;
												if( Dest[j-1]         != 1 ) Dest[j-1]         = 2 ;
												if( Dest[j+1]         != 1 ) Dest[j+1]         = 2 ;
												if( Dest[j-DestPitch] != 1 ) Dest[j-DestPitch] = 2 ;
												if( Dest[j+DestPitch] != 1 ) Dest[j+DestPitch] = 2 ;
											}
										}
										
										Src += SrcPitch ;
										Dest += DestPitch ;
									}
								}
								else
								if( EdgeSize <= FONTEDGE_PATTERN_NUM )
								{
									for( i = 0 ; i < Height ; i ++ )
									{
										for( j = 0 ; j < Width ; j ++ )
										{
											if( Src[ j ] != 0 )
											{
												Dest[j] = 1 ;
												for( n = 0, l = -EdgeSize ; l < EdgeSize + 1 ; n ++, l ++ )
												{
													for( o = 0, m = -EdgeSize ; m < EdgeSize + 1 ; o ++, m ++ )
													{
														if( EdgePat[n][o] == 0 ) continue ;
																		
														DestT = ( Dest + j + m ) + ( l * DestPitch ) ;
														if( *DestT != 1 ) *DestT = 2 ;
													}
												}
											}
										}
											
										Src  += SrcPitch ;
										Dest += DestPitch ;
									}
								}
								else
								{
									for( i = 0 ; i < Height ; i ++ )
									{
										for( j = 0 ; j < Width ; j ++ )
										{
											if( Src[ j ] != 0 )
											{
												Dest[j] = 1 ;
												for( l = -EdgeSize ; l < EdgeSize + 1 ; l ++ )
												{
													for( m = -EdgeSize ; m < EdgeSize + 1 ; m ++ )
													{
														DestT = Dest + j + m + l * DestPitch ;
														if( *DestT != 1 ) *DestT = 2 ;
													}
												}
											}
										}
										
										Src  += SrcPitch ;
										Dest += DestPitch ;
									}
								}
								break ;
							}
						}
						else
						{
							BYTE 		*Dest ;
							BASEIMAGE 	&im 	= ManageData->TextureCacheBaseImage ;
							COLORDATA 	&cl 	= im.ColorData ;
							DWORD 		RGBMask, AlphaRGBMask, adp ;

							DestPitch = im.Pitch ;
							Dest = ( BYTE * )im.GraphData + im.ColorData.PixelByte * ( DestPointX + DestX ) + DestPitch * ( DestPointY + DestY ) ;
							adp = im.Width / 2 ;

	#define D0		*( (WORD *)dp + j )
	#define D1		*( (WORD *)( dp + ( j << 1 ) - DestPitch ) )
	#define D2		*( (WORD *)( dp + ( j << 1 ) + DestPitch ) )
	#define D3		*( (WORD *)dp + j - 1 )
	#define D4		*( (WORD *)dp + j + 1 )

	#define E0		*( (WORD *)dp + adp + j )
	#define E1		*( (WORD *)( dp + ( ( j + adp ) << 1 ) - DestPitch ) )
	#define E2		*( (WORD *)( dp + ( ( j + adp ) << 1 ) + DestPitch ) )
	#define E3		*( (WORD *)dp + adp + j - 1 )
	#define E4		*( (WORD *)dp + adp + j + 1 )
	#define E5		*( (WORD *)( dp + ( ( j + adp - 1 ) << 1 ) - DestPitch ) )
	#define E6		*( (WORD *)( dp + ( ( j + adp + 1 ) << 1 ) - DestPitch ) )
	#define E7		*( (WORD *)( dp + ( ( j + adp - 1 ) << 1 ) + DestPitch ) )
	#define E8		*( (WORD *)( dp + ( ( j + adp + 1 ) << 1 ) + DestPitch ) )

	#define DD0		*( (DWORD *)dp + j )
	#define DD1		*( (DWORD *)( dp + ( j << 2 ) - DestPitch ) )
	#define DD2		*( (DWORD *)( dp + ( j << 2 ) + DestPitch ) )
	#define DD3		*( (DWORD *)dp + j - 1 )
	#define DD4		*( (DWORD *)dp + j + 1 )

	#define ED0		*( (DWORD *)dp + adp + j )
	#define ED1		*( (DWORD *)( dp + ( ( j + adp ) << 2 ) - DestPitch ) )
	#define ED2		*( (DWORD *)( dp + ( ( j + adp ) << 2 ) + DestPitch ) )
	#define ED3		*( (DWORD *)dp + adp + j - 1 )
	#define ED4		*( (DWORD *)dp + adp + j + 1 )
	#define ED5		*( (DWORD *)( dp + ( ( j + adp - 1 ) << 2 ) - DestPitch ) )
	#define ED6		*( (DWORD *)( dp + ( ( j + adp + 1 ) << 2 ) - DestPitch ) )
	#define ED7		*( (DWORD *)( dp + ( ( j + adp - 1 ) << 2 ) + DestPitch ) )
	#define ED8		*( (DWORD *)( dp + ( ( j + adp + 1 ) << 2 ) + DestPitch ) )

							RGBMask = cl.RedMask | cl.GreenMask | cl.BlueMask ;
							AlphaRGBMask = RGBMask | cl.AlphaMask ;
							{
								BYTE *sp, *dp ;
								WORD *p ;
								
								sp = Src ;
								dp = Dest ;
								
								if( EdgeSize == 1 )
								{
									for( i = 0 ; i < Height ; i ++ )
									{
										for( j = 0 ; j < Width ; j ++ )
										{
											if( sp[ j ] != 0 )
											{
												D0 = 1 ;
												E1 = 1 ;
												E2 = 1 ;
												E3 = 1 ;
												E4 = 1 ;
												E5 = 1 ;
												E6 = 1 ;
												E7 = 1 ;
												E8 = 1 ;
											}
										}
										
										sp += SrcPitch ;
										dp += DestPitch ;
									}
								}
								else
								if( EdgeSize <= FONTEDGE_PATTERN_NUM )
								{
									for( i = 0 ; i < Height ; i ++ )
									{
										for( j = 0 ; j < Width ; j ++ )
										{
											if( sp[ j ] != 0 )
											{
												D0 = 1 ;
												for( n = 0, l = -EdgeSize ; l < EdgeSize + 1 ; n ++, l ++ )
												{
													for( o = 0, m = -EdgeSize ; m < EdgeSize + 1 ; o ++, m ++ )
													{
														if( EdgePat[n][o] == 0 ) continue ;
													
														p = (WORD *)( dp + (j + m + adp) * 2 + (l * DestPitch) ) ;
														if( *p == 0 ) *p = 1 ;
													}
												}
											}
										}
										
										sp += SrcPitch ;
										dp += DestPitch ;
									}
								}
								else
								{
									for( i = 0 ; i < Height ; i ++ )
									{
										for( j = 0 ; j < Width ; j ++ )
										{
											if( sp[ j ] != 0 )
											{
												D0 = 1 ;
												for( l = -EdgeSize ; l < EdgeSize + 1 ; l ++ )
												{
													for( m = -EdgeSize ; m < EdgeSize + 1 ; m ++ )
													{
														p = (WORD *)( dp + (j + m + adp) * 2 + (l * DestPitch) ) ;
														if( *p == 0 ) *p = 1 ;
													}
												}
											}
										}
										
										sp += SrcPitch ;
										dp += DestPitch ;
									}
								}

								sp = Src - AddX - AddY * SrcPitch ;
								dp = Dest - AddX * 2 - AddY * DestPitch ;
								if( ManageData->TextureCacheUsePremulAlpha )
								{
									for( i = 0 ; i < Height + AddSize ; i ++ )
									{
										for( j = 0 ; j < Width + AddSize ; j ++ )
										{
											if( D0 != 0 )
											{
												D0 = ( WORD )AlphaRGBMask ;
												E0 = ( WORD )0 ;
											}
											else
											{
												D0 = ( WORD )0 ;
												E0 = ( WORD )( E0 != 0 ? AlphaRGBMask : 0 ) ;
											}
										}

										sp += SrcPitch ;
										dp += DestPitch ;
									}
								}
								else
								{
									for( i = 0 ; i < Height + AddSize ; i ++ )
									{
										for( j = 0 ; j < Width + AddSize ; j ++ )
										{
											if( D0 != 0 )
											{
												D0 = ( WORD )AlphaRGBMask ;
												E0 = ( WORD )RGBMask ;
											}
											else
											{
												D0 = ( WORD )RGBMask ;
												E0 = ( WORD )( E0 != 0 ? AlphaRGBMask : RGBMask ) ;
											}
										}

										sp += SrcPitch ;
										dp += DestPitch ;
									}
								}
							}
	#undef ED0
	#undef ED1
	#undef ED2
	#undef ED3
	#undef ED4
	#undef ED5
	#undef ED6
	#undef ED7
	#undef ED8

	#undef DD0
	#undef DD1
	#undef DD2
	#undef DD3
	#undef DD4

	#undef E0
	#undef E1
	#undef E2
	#undef E3
	#undef E4
	#undef E5
	#undef E6
	#undef E7
	#undef E8

	#undef D0
	#undef D1
	#undef D2
	#undef D3
	#undef D4
							// テクスチャキャッシュに転送
							{
								RECT srect ;

								SETRECT( srect, DestX - 1, DestY - 1, FontData->SizeX + AddSize + DestX, FontData->SizeY + AddSize + DestY ) ;
								BltBmpOrGraphImageToGraph2Base( &im.ColorData, NULL, NULL, 
																FALSE, &im, NULL,
																&srect, DestX - 1, DestY - 1, ManageData->TextureCache ) ;

								srect.left += adp ;
								srect.right += adp ;
								BltBmpOrGraphImageToGraph2Base( &im.ColorData, NULL, NULL, 
																FALSE, &im, NULL,
																&srect, DestX - 1 + ManageData->SurfaceSize.cx, DestY - 1, ManageData->TextureCache ) ;
							}
						}
					}
				}
				else
				{
					// 取得するイメージ形式を決定する
					switch( ManageData->FontType )
					{
					case DX_FONTTYPE_NORMAL :
					case DX_FONTTYPE_EDGE :
					case DX_FONTTYPE_ANTIALIASING_4X4 :
					case DX_FONTTYPE_ANTIALIASING_8X8 :
					case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
					case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
						gettype = GGO_BITMAP ;
						break ;
						
					case DX_FONTTYPE_ANTIALIASING :
					case DX_FONTTYPE_ANTIALIASING_EDGE :
						gettype = ManageData->TextureCache == FALSE || ManageData->TextureCacheColorBitDepth == 16 ? GGO_GRAY4_BITMAP : GGO_GRAY8_BITMAP ;
						break ;
					}

					// 最終的なイメージ形式を決定する
					switch( ManageData->FontType )
					{
					case DX_FONTTYPE_NORMAL :
					case DX_FONTTYPE_EDGE :
						type = GGO_BITMAP ;
						break ;
						
					case DX_FONTTYPE_ANTIALIASING :
					case DX_FONTTYPE_ANTIALIASING_EDGE :
					case DX_FONTTYPE_ANTIALIASING_4X4 :
					case DX_FONTTYPE_ANTIALIASING_8X8 :
					case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
					case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
						type = ManageData->TextureCache == FALSE || ManageData->TextureCacheColorBitDepth == 16 ? GGO_GRAY4_BITMAP : GGO_GRAY8_BITMAP ;
						break ;
					}

					{
						BYTE *DataBuffer, *AllocAddress ;
						DWORD DataSize, DataHeight, DataMDrawY ;
						GLYPHMETRICS gm ;
						MAT2 mt = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } } ;
						int SrcPitch ; 

						// 文字情報の取得
						_MEMSET( &gm, 0, sizeof( GLYPHMETRICS ) ) ;
						DataSize = GetGlyphOutline( DC, Code, gettype, &gm, 0, NULL, &mt ) ;
						if( DataSize == GDI_ERROR )
						{
							DXST_ERRORLOG_ADD( _T( "GetGlyphOutline 関数が失敗しました\n" ) ) ;
							continue ;
						}

						// もしスペース文字だった場合は次の文字を描画する座標だけ代入して終了
						if( Space != 0 )
						{
							FontData->AddX = ( short )( ( gm.gmCellIncX + ( SampleScale >> 1 ) ) / SampleScale ) ;
							FontData->SizeX = 0 ;
							FontData->SizeY = 0 ;
						}
						else
						{
							// 文字イメージを取得
							{
								SrcPitch = ( DataSize / gm.gmBlackBoxY ) / 4 * 4 ;
								DataBuffer = ( BYTE * )DXALLOC( DataSize + SrcPitch * ( 2 + gm.gmBlackBoxY ) ) ;
								if( DataBuffer == NULL )
								{
									DxLib_Error( DXSTRING( _T( "アンチエイリアス文字取得用バッファの確保に失敗しました" ) ) ) ;
									return NULL ;
								}
								_MEMSET( DataBuffer, 0, DataSize + SrcPitch * ( 2 + gm.gmBlackBoxY ) ) ;
								AllocAddress = DataBuffer ;

								DataBuffer += SrcPitch * ( 1 + gm.gmBlackBoxY ) ;
								DataSize = GetGlyphOutline( DC, Code, gettype, &gm, DataSize, ( LPVOID )DataBuffer, &mt ) ;
								if( DataSize == GDI_ERROR )
								{
									DXST_ERRORLOG_ADD( _T( "GetGlyphOutline 関数が失敗しました\n" ) ) ;
									continue ;
								}
							}

							// 大きさ調整
							DataMDrawY = ( met.tmAscent - gm.gmptGlyphOrigin.y ) % SampleScale ;
							DataHeight = gm.gmBlackBoxY + DataMDrawY ;
							FontData->DrawX = ( short )( ( gm.gmptGlyphOrigin.x + ( SampleScale >> 2 ) ) / SampleScale ) ;
							FontData->DrawY = ( short )( ( met.tmAscent - gm.gmptGlyphOrigin.y ) / SampleScale ) ;
							FontData->SizeX = ( short )( ( gm.gmBlackBoxX + SampleScale - 1 ) / SampleScale ) ;
							FontData->SizeY = ( short )( ( DataHeight + SampleScale - 1 ) / SampleScale ) ;
							FontData->AddX = ( short )( ( gm.gmCellIncX + ( SampleScale >> 2 ) ) / SampleScale ) ;

							// リサンプリング設定の場合はここでリサンプリングを行う
							if( SampleScale > 1 )
							{
								BYTE *RDataBuffer ;
								BYTE *RAllocAddress ;
								int RSrcPitch ;
								DWORD SrcAddPitch ;
								DWORD SrcPitch2 ;
								DWORD SrcPitch3 ;
								DWORD SrcPitch4 ;
								DWORD SrcPitch5 ;
								DWORD SrcPitch6 ;
								DWORD SrcPitch7 ;
								BYTE *RSrc, *RDest ;
								int RWidth, RHeight, MHeight, HWidth ;
								int i, j ;

								RWidth  = ( FontData->SizeX + 1 ) / 2 * 2 ;
								HWidth = RWidth / 2 ;
								RHeight = DataHeight / SampleScale ;
								MHeight = DataHeight % SampleScale ;

								RSrcPitch = RWidth + 4 ;

								// リサンプル後のデータを格納するメモリを確保
								RDataBuffer = ( BYTE * )DXALLOC( RSrcPitch * ( FontData->SizeY + 2 ) ) ;
								if( RDataBuffer == NULL )
								{
									DxLib_Error( DXSTRING( _T( "文字イメージリサンプリング用バッファの確保に失敗しました" ) ) ) ;
									return NULL ;
								}
								_MEMSET( RDataBuffer, 0, RSrcPitch * ( FontData->SizeY + 2 ) ) ;
								RAllocAddress = RDataBuffer ;
								RDataBuffer += RSrcPitch ;

								RSrc  = DataBuffer - DataMDrawY * SrcPitch ;
								RDest = RDataBuffer ;
								SrcAddPitch = SrcPitch * SampleScale ;

								SrcPitch2 = SrcPitch * 2 ;
								SrcPitch3 = SrcPitch * 3 ;
								SrcPitch4 = SrcPitch * 4 ;
								SrcPitch5 = SrcPitch * 5 ;
								SrcPitch6 = SrcPitch * 6 ;
								SrcPitch7 = SrcPitch * 7 ;

								// リサンプルスケールによって処理を分岐
								switch( SampleScale )
								{
									// ４倍の場合
								case 4 :
									switch( type )
									{
									case GGO_GRAY4_BITMAP:
										for( i = 0 ; i < RHeight ; i ++ )
										{
											for( j = 0 ; j < HWidth ; j ++ )
											{
												RDest[ j * 2     ] =
													FSYS.BitCountTable[ RSrc[ j             ] & 0xf0 ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch  ] & 0xf0 ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch2 ] & 0xf0 ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch3 ] & 0xf0 ] ;

												RDest[ j * 2 + 1 ] =
													FSYS.BitCountTable[ RSrc[ j             ] & 0x0f ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch  ] & 0x0f ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch2 ] & 0x0f ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch3 ] & 0x0f ] ;
											}

											RSrc  += SrcAddPitch ;
											RDest += RSrcPitch ;
										}

										if( MHeight != 0 )
										{
											for( i = 0 ; i < MHeight ; i ++ )
											{
												for( j = 0 ; j < HWidth ; j ++ )
												{
													RDest[ j * 2     ] += FSYS.BitCountTable[ RSrc[ j ] & 0xf0 ] ;
													RDest[ j * 2 + 1 ] += FSYS.BitCountTable[ RSrc[ j ] & 0x0f ] ;
												}

												RSrc += SrcPitch ;
											}
										}
										break;

									case GGO_GRAY8_BITMAP:
										for( i = 0 ; i < RHeight ; i ++ )
										{
											for( j = 0 ; j < HWidth ; j ++ )
											{
												RDest[ j * 2     ] =
												  (	FSYS.BitCountTable[ RSrc[ j             ] & 0xf0 ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch  ] & 0xf0 ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch2 ] & 0xf0 ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch3 ] & 0xf0 ] ) << 2 ;

												RDest[ j * 2 + 1 ] =
												  (	FSYS.BitCountTable[ RSrc[ j             ] & 0x0f ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch  ] & 0x0f ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch2 ] & 0x0f ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch3 ] & 0x0f ] ) << 2 ;
											}

											RSrc  += SrcAddPitch ;
											RDest += RSrcPitch ;
										}

										if( MHeight != 0 )
										{
											for( i = 0 ; i < MHeight ; i ++ )
											{
												for( j = 0 ; j < HWidth ; j ++ )
												{
													RDest[ j * 2     ] += FSYS.BitCountTable[ RSrc[ j ] & 0xf0 ] << 2 ;
													RDest[ j * 2 + 1 ] += FSYS.BitCountTable[ RSrc[ j ] & 0x0f ] << 2 ;
												}

												RSrc += SrcPitch ;
											}
										}
										break ;
									}
									break ;

									// ８倍の場合
								case 8 :
									switch( type )
									{
									case GGO_GRAY4_BITMAP:
										for( i = 0 ; i < RHeight ; i ++ )
										{
											for( j = 0 ; j < RWidth ; j ++ )
											{
												RDest[ j ] =
												(	FSYS.BitCountTable[ RSrc[ j ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch2 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch3 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch4 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch5 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch6 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch7 ] ] ) >> 2 ;
											}

											RSrc  += SrcAddPitch ;
											RDest += RSrcPitch ;
										}

										if( MHeight != 0 )
										{
											for( i = 0 ; i < MHeight ; i ++ )
											{
												for( j = 0 ; j < RWidth ; j ++ )
												{
													RDest[ j ] += FSYS.BitCountTable[ RSrc[ j ] ] ;
												}

												RSrc += SrcPitch ;
											}
											for( j = 0 ; j < RWidth ; j ++ )
											{
												RDest[ j ] >>= 2 ;
											}
										}
										break;

									case GGO_GRAY8_BITMAP:
										for( i = 0 ; i < RHeight ; i ++ )
										{
											for( j = 0 ; j < RWidth ; j ++ )
											{
												RDest[ j ] =
												(	FSYS.BitCountTable[ RSrc[ j ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch2 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch3 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch4 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch5 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch6 ] ] +
													FSYS.BitCountTable[ RSrc[ j + SrcPitch7 ] ] ) ;
											}

											RSrc  += SrcAddPitch ;
											RDest += RSrcPitch ;
										}

										if( MHeight != 0 )
										{
											for( i = 0 ; i < MHeight ; i ++ )
											{
												for( j = 0 ; j < RWidth ; j ++ )
												{
													RDest[ j ] += FSYS.BitCountTable[ RSrc[ j ] ] ;
												}

												RSrc += SrcPitch ;
											}
										}
										break ;
									}
									break ;
								}

								DXFREE( AllocAddress ) ;
								DataBuffer = RDataBuffer ;
								AllocAddress = RAllocAddress ;
								SrcPitch = RSrcPitch ;
							}

							// 取得したデータをキャッシュに転送
							if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) == 0 )
							{
								BYTE *Src, dat = 0 ;
								int Height, Width ;
								int i, j ;
								DWORD DestPitch ;

								Width = FontData->SizeX ;
								Height = FontData->SizeY ;

								Src = DataBuffer ;

								if( ManageData->TextureCacheFlag == FALSE )
								{
									BYTE *Dest ;

									DestPitch = ManageData->CachePitch ;
									Dest = ( BYTE * )ManageData->CacheMem + DestY * DestPitch ;

									switch( ManageData->FontType )
									{
									case DX_FONTTYPE_NORMAL :
										for( i = 0 ; i < Height ; i ++ )
										{
											_MEMCPY( Dest, Src, SrcPitch ) ;

											Src  += SrcPitch ;
											Dest += DestPitch ;
										}
										break ;

									case DX_FONTTYPE_ANTIALIASING : 
									case DX_FONTTYPE_ANTIALIASING_4X4 : 
									case DX_FONTTYPE_ANTIALIASING_8X8 : 
										for( i = 0 ; i < Height ; i ++ )
										{
											for( j = 0 ; j < Width ; j ++ )
											{
												if( Src[j] ) Dest[j] = Src[j] - 1 ;
											}

											Src  += SrcPitch ;
											Dest += DestPitch ;
										}
										break ;
									}
								}
								else
								{
									BYTE 		*Dest ;
	//								BASEIMAGE 	&im 	= ManageData->TextureTempCache ;
									BASEIMAGE 	&im 	= ManageData->TextureCacheBaseImage ;
									COLORDATA 	&cl 	= im.ColorData ;
									DWORD 		RGBMask ;

	//								_MEMSET( im.GraphData, 0, im.Height * im.Pitch );
									DestPitch = im.Pitch ;
									Dest = (BYTE *)im.GraphData + DestX * im.ColorData.PixelByte + DestY * im.Pitch ;

									RGBMask = cl.RedMask | cl.GreenMask | cl.BlueMask ;

									switch( type )
									{
									case GGO_BITMAP :
										if( ManageData->TextureCacheUsePremulAlpha )
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++, dat <<= 1 )
												{
													if( ( j & 0x7 ) == 0 ) dat = Src[j>>3] ;

													if( dat & 0x80 )
													{
														*( (WORD *)Dest + j ) = (WORD)( RGBMask | cl.AlphaMask ) ;
													}
													else
													{
														*( (WORD *)Dest + j ) = 0 ;
													}
												}

												Src += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										else
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++, dat <<= 1 )
												{
													if( ( j & 0x7 ) == 0 ) dat = Src[j>>3] ;
													*( (WORD *)Dest + j ) = (WORD)( RGBMask | ( ( dat & 0x80 ) ? cl.AlphaMask : 0 ) ) ;
												}

												Src += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										break ;
										
									case GGO_GRAY4_BITMAP :
										if( cl.AlphaMask == 0xf000 &&
											cl.RedMask   == 0x0f00 &&
											cl.GreenMask == 0x00f0 &&
											cl.BlueMask  == 0x000f )
										{
											if( ManageData->TextureCacheUsePremulAlpha )
											{
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++ )
													{
														if( Src[j] == 16 )
														{
															*( (WORD *)Dest + j ) = ( WORD )0xffff ;
														}
														else
														if( Src[j] )
														{
															*( (WORD *)Dest + j ) = ( WORD )( ( Src[j] << 0 ) | ( Src[j] << 4 ) | ( Src[j] << 8 ) | ( Src[j] << 12 ) ) ;
														}
														else
														{
															*( (WORD *)Dest + j ) = (WORD)0x0000 ;
														}
													}

													Src += SrcPitch ;
													Dest += DestPitch ;
												}
											}
											else
											{
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++ )
													{
														if( Src[j] )
															*( (WORD *)Dest + j ) = ( WORD )( ( ( (DWORD)Src[j] << 12 ) - 1 ) | 0x0fff ) ;
														else
															*( (WORD *)Dest + j ) = (WORD)0x0fff ;
													}

													Src += SrcPitch ;
													Dest += DestPitch ;
												}
											}
										}
										else
										if( cl.ColorBitDepth == 16 )
										{
											if( ManageData->TextureCacheUsePremulAlpha )
											{
												BYTE aloc, rloc, gloc, bloc ;

												aloc = cl.AlphaLoc + cl.AlphaWidth - 4 ;
												rloc = cl.RedLoc   + cl.RedWidth   - 4 ;
												gloc = cl.GreenLoc + cl.GreenWidth - 4 ;
												bloc = cl.BlueLoc  + cl.BlueWidth  - 4 ;
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++ )
													{
														if( Src[j] == 16 )
														{
															*( (WORD *)Dest + j ) = ( WORD )0xffff ;
														}
														else
														if( Src[j] )
														{
															*( (WORD *)Dest + j ) = ( WORD )( ( Src[j] << aloc ) | ( Src[j] << rloc ) | ( Src[j] << gloc ) | ( Src[j] << bloc ) ) ;
														}
														else
														{
															*( (WORD *)Dest + j ) = (WORD)0x0000 ;
														}
													}

													Src += SrcPitch ;
													Dest += DestPitch ;
												}
											}
											else
											{
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++ )
													{
														if( Src[j] )
															*( (WORD *)Dest + j ) = ( WORD )( ( ( (DWORD)Src[j] << ( ( cl.AlphaLoc + cl.AlphaWidth ) - 4 ) ) - 1 ) | RGBMask ) ;
														else
															*( (WORD *)Dest + j ) = (WORD)RGBMask ;
													}

													Src += SrcPitch ;
													Dest += DestPitch ;
												}
											}
										}
										else
										if( cl.ColorBitDepth == 32 )
										{
											if( ManageData->TextureCacheUsePremulAlpha )
											{
												BYTE aloc, rloc, gloc, bloc ;

												aloc = cl.AlphaLoc + cl.AlphaWidth - 4 ;
												rloc = cl.RedLoc   + cl.RedWidth   - 4 ;
												gloc = cl.GreenLoc + cl.GreenWidth - 4 ;
												bloc = cl.BlueLoc  + cl.BlueWidth  - 4 ;
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++ )
													{
														if( Src[j] == 16 )
														{
															*( (DWORD *)Dest + j ) = ( DWORD )0xffffffff ;
														}
														else
														if( Src[j] )
														{
															*( (DWORD *)Dest + j ) = ( DWORD )( ( Src[j] << aloc ) | ( Src[j] << rloc ) | ( Src[j] << gloc ) | ( Src[j] << bloc ) ) ;
														}
														else
														{
															*( (DWORD *)Dest + j ) = (DWORD)0x00000000 ;
														}
													}

													Src += SrcPitch ;
													Dest += DestPitch ;
												}
											}
											else
											{
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++ )
													{
														if( Src[j] )
															*( (DWORD *)Dest + j ) = ( DWORD )( ( ( (DWORD)Src[j] << ( ( cl.AlphaLoc + cl.AlphaWidth ) - 4 ) ) - 1 ) | RGBMask ) ;
														else
															*( (DWORD *)Dest + j ) = (DWORD)RGBMask ;
													}

													Src += SrcPitch ;
													Dest += DestPitch ;
												}
											}
										}
										break ;

									case GGO_GRAY8_BITMAP :
										if( ManageData->TextureCacheUsePremulAlpha )
										{
											BYTE aloc, rloc, gloc, bloc ;

											aloc = cl.AlphaLoc + cl.AlphaWidth - 6 ;
											rloc = cl.RedLoc   + cl.RedWidth   - 6 ;
											gloc = cl.GreenLoc + cl.GreenWidth - 6 ;
											bloc = cl.BlueLoc  + cl.BlueWidth  - 6 ;
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++ )
												{
													if( Src[j] == 64 )
													{
														*( (DWORD *)Dest + j ) = ( DWORD )0xffffffff ;
													}
													else
													if( Src[j] )
													{
														*( (DWORD *)Dest + j ) = ( DWORD )( ( Src[j] << aloc ) | ( Src[j] << rloc ) | ( Src[j] << gloc ) | ( Src[j] << bloc ) ) ;
													}
													else
													{
														*( (DWORD *)Dest + j ) = (DWORD)0x00000000 ;
													}
												}

												Src += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										else
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++ )
												{
													if( Src[j] )
													{
														*( (DWORD *)Dest + j ) = ( DWORD )( ( ( DWORD )( Src[j] - 1 ) << ( ( cl.AlphaLoc + cl.AlphaWidth ) - 6 ) ) | RGBMask ) ;
													}
													else
													{
														*( (DWORD *)Dest + j ) = RGBMask ;
													}
												}

												Src  += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										break ;
									}

									// テクスチャキャッシュに転送
									{
										RECT srect ;

	//									SETRECT( srect, 0, 0, FontData->SizeX, FontData->SizeY ) ;
	//									SETRECT( srect, 0, 0, ManageData->MaxWidth, ManageData->MaxWidth ) ;
										SETRECT( srect, DestX - 1, DestY - 1, ManageData->MaxWidth + DestX, ManageData->MaxWidth + DestY ) ;
										BltBmpOrGraphImageToGraph2Base( &im.ColorData, NULL, NULL, 
																		FALSE, &im, NULL,
																		&srect, DestX - 1, DestY - 1,
																		ManageData->TextureCache ) ;
									}
								}
							}
							else
							{
								BYTE	*Src, dat = 0 ;
								BYTE 	*Dest, *DestT ;
								int 	Height, Width ;
								int 	DestPointX,	DestPointY ;
								int 	i, j, k, l, n, m, o, d, EdgeSize ;
								int 	DestPitch ;
								unsigned char (*EdgePat)[FONTEDGE_PATTERN_NUM*2+1] ;
								
								Width 		= FontData->SizeX ;
								Height 		= FontData->SizeY ;
								DestPointX 	= AddX ;
								DestPointY 	= AddY ;

								Src = DataBuffer ;
								EdgeSize = ManageData->EdgeSize ;
								EdgePat = _FontEdgePattern[ EdgeSize - 1 ] ;
								
								if( ManageData->TextureCacheFlag == FALSE )
								{
									DestPitch = ManageData->CachePitch ;

									switch( ManageData->FontType )
									{
									case DX_FONTTYPE_EDGE :
										Dest = ( BYTE * )ManageData->CacheMem + ( DestX + DestPointX ) + ( DestY + DestPointY ) * DestPitch ;

										if( EdgeSize == 1 )
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++, dat <<= 1 )
												{
													if( ( j & 0x7 ) == 0 ) dat = Src[ j / 8 ] ;
													if( dat & 0x80 )
													{
														Dest[j] = 1 ;
														if( Dest[j-1]         != 1 ) Dest[j-1]         = 2 ;
														if( Dest[j+1]         != 1 ) Dest[j+1]         = 2 ;
														if( Dest[j-DestPitch] != 1 ) Dest[j-DestPitch] = 2 ;
														if( Dest[j+DestPitch] != 1 ) Dest[j+DestPitch] = 2 ;
													}
												}
												
												Src += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										else
										if( EdgeSize <= FONTEDGE_PATTERN_NUM )
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++, dat <<= 1 )
												{
													if( ( j % 8 ) == 0 ) dat = Src[ j / 8 ] ;
													if( dat & 0x80 )
													{
														Dest[j] = 1 ;
														for( n = 0, l = -EdgeSize ; l < EdgeSize + 1 ; n ++, l ++ )
														{
															for( o = 0, m = -EdgeSize ; m < EdgeSize + 1 ; o ++, m ++ )
															{
																if( EdgePat[n][o] == 0 ) continue ;
																				
																DestT = ( Dest + j + m ) + ( l * DestPitch ) ;
																if( *DestT != 1 ) *DestT = 2 ;
															}
														}
													}
												}
													
												Src  += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										else
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++, dat <<= 1 )
												{
													if( ( j % 8 ) == 0 ) dat = Src[ j / 8 ] ;
													if( dat & 0x80 )
													{
														Dest[j] = 1 ;
														for( l = -EdgeSize ; l < EdgeSize + 1 ; l ++ )
														{
															for( m = -EdgeSize ; m < EdgeSize + 1 ; m ++ )
															{
																DestT = Dest + j + m + l * DestPitch ;
																if( *DestT != 1 ) *DestT = 2 ;
															}
														}
													}
												}
												
												Src  += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										break ;


									case DX_FONTTYPE_ANTIALIASING_EDGE :
									case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
									case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
										Dest = ( BYTE * )ManageData->CacheMem + DestX + DestPointX + ( DestY + DestPointY ) * DestPitch ;

										k = 0 ;
										if( EdgeSize == 1 )
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++ )
												{
													if( Src[j] != 0 )
													{
														d = Src[j] - 1 ;
														Dest[j] |= d ;
														d <<= 4 ;
														if( ( Dest[j - DestPitch] & 0xf0 ) < d ) Dest[j - DestPitch] = ( BYTE )( ( Dest[j - DestPitch] & 0x0f ) | d ) ;
														if( ( Dest[j + DestPitch] & 0xf0 ) < d ) Dest[j + DestPitch] = ( BYTE )( ( Dest[j + DestPitch] & 0x0f ) | d ) ;
														if( ( Dest[j + 1] & 0xf0 )         < d ) Dest[j + 1]         = ( BYTE )( ( Dest[j + 1]         & 0x0f ) | d ) ;
														if( ( Dest[j - 1] & 0xf0 )         < d ) Dest[j - 1]         = ( BYTE )( ( Dest[j - 1]         & 0x0f ) | d ) ;
													}
												}

												Src  += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										else
										if( EdgeSize <= FONTEDGE_PATTERN_NUM )
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++ )
												{
													if( Src[j] != 0 )
													{
														d = Src[j] - 1 ;
														Dest[j] |= d ;
														d <<= 4 ;
														for( n = 0, l = -EdgeSize ; l < EdgeSize + 1 ; n ++, l ++ )
														{
															for( o = 0, m = -EdgeSize ; m < EdgeSize + 1 ; o ++, m ++ )
															{
																if( EdgePat[n][o] == 0 ) continue ;

																DestT = Dest + j + l * DestPitch + m ;
																if( ( *DestT & 0xf0 ) < d ) *DestT = ( BYTE )( ( *DestT & 0x0f ) | d ) ;
															}
														}
													}
												}

												Src += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										else
										{
											for( i = 0 ; i < Height ; i ++ )
											{
												for( j = 0 ; j < Width ; j ++ )
												{
													if( Src[j] != 0 )
													{
														d = Src[j] - 1 ;
														Dest[j] |= d ;
														d <<= 4 ;
														for( l = -EdgeSize ; l < EdgeSize + 1 ; l ++ )
														{
															for( m = -EdgeSize ; m < EdgeSize + 1 ; m ++ )
															{
																DestT = Dest + j + l * DestPitch + m ;
																if( ( *DestT & 0xf0 ) < d ) *DestT = ( BYTE )( ( *DestT & 0x0f ) | d ) ;
															}
														}
													}
												}

												Src += SrcPitch ;
												Dest += DestPitch ;
											}
										}
										break ;
									}
								}
								else
								{
									BYTE 		*Dest, dat = 0 ;
	//								BASEIMAGE 	&im 	= ManageData->TextureTempCache ;
									BASEIMAGE 	&im 	= ManageData->TextureCacheBaseImage ;
									COLORDATA 	&cl 	= im.ColorData ;
									DWORD 		RGBMask, AlphaRGBMask, adp ;

									DestPitch = im.Pitch ;
									Dest = ( BYTE * )im.GraphData + im.ColorData.PixelByte * ( DestPointX + DestX ) + DestPitch * ( DestPointY + DestY ) ;
									adp = im.Width / 2 ;

	#define D0		*( (WORD *)dp + j )
	#define D1		*( (WORD *)( dp + ( j << 1 ) - DestPitch ) )
	#define D2		*( (WORD *)( dp + ( j << 1 ) + DestPitch ) )
	#define D3		*( (WORD *)dp + j - 1 )
	#define D4		*( (WORD *)dp + j + 1 )

	#define E0		*( (WORD *)dp + adp + j )
	#define E1		*( (WORD *)( dp + ( ( j + adp ) << 1 ) - DestPitch ) )
	#define E2		*( (WORD *)( dp + ( ( j + adp ) << 1 ) + DestPitch ) )
	#define E3		*( (WORD *)dp + adp + j - 1 )
	#define E4		*( (WORD *)dp + adp + j + 1 )
	#define E5		*( (WORD *)( dp + ( ( j + adp - 1 ) << 1 ) - DestPitch ) )
	#define E6		*( (WORD *)( dp + ( ( j + adp + 1 ) << 1 ) - DestPitch ) )
	#define E7		*( (WORD *)( dp + ( ( j + adp - 1 ) << 1 ) + DestPitch ) )
	#define E8		*( (WORD *)( dp + ( ( j + adp + 1 ) << 1 ) + DestPitch ) )

	#define DD0		*( (DWORD *)dp + j )
	#define DD1		*( (DWORD *)( dp + ( j << 2 ) - DestPitch ) )
	#define DD2		*( (DWORD *)( dp + ( j << 2 ) + DestPitch ) )
	#define DD3		*( (DWORD *)dp + j - 1 )
	#define DD4		*( (DWORD *)dp + j + 1 )

	#define ED0		*( (DWORD *)dp + adp + j )
	#define ED1		*( (DWORD *)( dp + ( ( j + adp ) << 2 ) - DestPitch ) )
	#define ED2		*( (DWORD *)( dp + ( ( j + adp ) << 2 ) + DestPitch ) )
	#define ED3		*( (DWORD *)dp + adp + j - 1 )
	#define ED4		*( (DWORD *)dp + adp + j + 1 )
	#define ED5		*( (DWORD *)( dp + ( ( j + adp - 1 ) << 2 ) - DestPitch ) )
	#define ED6		*( (DWORD *)( dp + ( ( j + adp + 1 ) << 2 ) - DestPitch ) )
	#define ED7		*( (DWORD *)( dp + ( ( j + adp - 1 ) << 2 ) + DestPitch ) )
	#define ED8		*( (DWORD *)( dp + ( ( j + adp + 1 ) << 2 ) + DestPitch ) )

									RGBMask = cl.RedMask | cl.GreenMask | cl.BlueMask ;
									AlphaRGBMask = RGBMask | cl.AlphaMask ;
									{
										BYTE *sp, *dp ;
										WORD *p ;
										DWORD *pd ;
										
										sp = Src ;
										dp = Dest ;
										
										switch( type )
										{
										case GGO_BITMAP :
											if( EdgeSize == 1 )
											{
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++, dat <<= 1 )
													{
														if( ( j & 0x7 ) == 0 ) dat = sp[j>>3] ;
														if( dat & 0x80 )
														{
															D0 = 1 ;
															E1 = 1 ;
															E2 = 1 ;
															E3 = 1 ;
															E4 = 1 ;
															E5 = 1 ;
															E6 = 1 ;
															E7 = 1 ;
															E8 = 1 ;
														}
													}
													
													sp += SrcPitch ;
													dp += DestPitch ;
												}
											}
											else
											if( EdgeSize <= FONTEDGE_PATTERN_NUM )
											{
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++, dat <<= 1 )
													{
														if( ( j % 8 ) == 0 ) dat = sp[j/8] ;
														if( dat & 0x80 )
														{
															D0 = 1 ;
															for( n = 0, l = -EdgeSize ; l < EdgeSize + 1 ; n ++, l ++ )
															{
																for( o = 0, m = -EdgeSize ; m < EdgeSize + 1 ; o ++, m ++ )
																{
																	if( EdgePat[n][o] == 0 ) continue ;
																
																	p = (WORD *)( dp + (j + m + adp) * 2 + (l * DestPitch) ) ;
																	if( *p == 0 ) *p = 1 ;
																}
															}
														}
													}
													
													sp += SrcPitch ;
													dp += DestPitch ;
												}
											}
											else
											{
												for( i = 0 ; i < Height ; i ++ )
												{
													for( j = 0 ; j < Width ; j ++, dat <<= 1 )
													{
														if( ( j % 8 ) == 0 ) dat = sp[j/8] ;
														if( dat & 0x80 )
														{
															D0 = 1 ;
															for( l = -EdgeSize ; l < EdgeSize + 1 ; l ++ )
															{
																for( m = -EdgeSize ; m < EdgeSize + 1 ; m ++ )
																{
																	p = (WORD *)( dp + (j + m + adp) * 2 + (l * DestPitch) ) ;
																	if( *p == 0 ) *p = 1 ;
																}
															}
														}
													}
													
													sp += SrcPitch ;
													dp += DestPitch ;
												}
											}

											sp = Src - AddX - AddY * SrcPitch ;
											dp = Dest - AddX * 2 - AddY * DestPitch ;
											if( ManageData->TextureCacheUsePremulAlpha )
											{
												for( i = 0 ; i < Height + AddSize ; i ++ )
												{
													for( j = 0 ; j < Width + AddSize ; j ++ )
													{
														if( D0 != 0 )
														{
															D0 = ( WORD )AlphaRGBMask ;
															E0 = ( WORD )0 ;
														}
														else
														{
															D0 = ( WORD )0 ;
															E0 = ( WORD )( E0 != 0 ? AlphaRGBMask : 0 ) ;
														}
													}

													sp += SrcPitch ;
													dp += DestPitch ;
												}
											}
											else
											{
												for( i = 0 ; i < Height + AddSize ; i ++ )
												{
													for( j = 0 ; j < Width + AddSize ; j ++ )
													{
														if( D0 != 0 )
														{
															D0 = ( WORD )AlphaRGBMask ;
															E0 = ( WORD )RGBMask ;
														}
														else
														{
															D0 = ( WORD )RGBMask ;
															E0 = ( WORD )( E0 != 0 ? AlphaRGBMask : RGBMask ) ;
														}
													}

													sp += SrcPitch ;
													dp += DestPitch ;
												}
											}
											break ;
											
										case GGO_GRAY4_BITMAP :
											{
												DWORD s ;
												
												if( EdgeSize == 1 )
												{
													for( i = 0 ; i < Height ; i ++ )
													{
														for( j = 0 ; j < Width ; j ++ )
														{
															s = (DWORD)sp[j] ;
															if( s > 0 )
															{
																D0 = (WORD)s ;
																E1 = (WORD)( E1 + ( ( ( 0x10 - E1 ) * s ) >> 4 ) ) ;
																E2 = (WORD)( E2 + ( ( ( 0x10 - E2 ) * s ) >> 4 ) ) ;
																E3 = (WORD)( E3 + ( ( ( 0x10 - E3 ) * s ) >> 4 ) ) ;
																E4 = (WORD)( E4 + ( ( ( 0x10 - E4 ) * s ) >> 4 ) ) ;
																E5 = (WORD)( E5 + ( ( ( 0x10 - E5 ) * s ) >> 4 ) ) ;
																E6 = (WORD)( E6 + ( ( ( 0x10 - E6 ) * s ) >> 4 ) ) ;
																E7 = (WORD)( E7 + ( ( ( 0x10 - E7 ) * s ) >> 4 ) ) ;
																E8 = (WORD)( E8 + ( ( ( 0x10 - E8 ) * s ) >> 4 ) ) ;
															}
														}

														sp += SrcPitch ;
														dp += DestPitch ;
													}
												}
												else
												if( EdgeSize <= FONTEDGE_PATTERN_NUM )
												{
													for( i = 0 ; i < Height ; i ++ )
													{
														for( j = 0 ; j < Width ; j ++ )
														{
															s = (DWORD)sp[j] ;
															if( s > 0 )
															{
																D0 = (WORD)s ;
																for( n = 0, l = -EdgeSize ; l < EdgeSize + 1 ; n ++, l ++ )
																{
																	for( o = 0, m = -EdgeSize ; m < EdgeSize + 1 ; o ++, m ++ )
																	{
																		if( EdgePat[n][o] == 0 ) continue ;
																		p = (WORD *)( dp + (j + m + adp) * 2 + (l * DestPitch) ) ;
																		*p = (WORD)( ( 0x100 - ( ( 0x10 - *p ) * ( 0x10 - s ) ) ) >> 4 ) ;
																	}
																}
															}
														}

														sp += SrcPitch ;
														dp += DestPitch ;
													}
												}
												else
												{
													for( i = 0 ; i < Height ; i ++ )
													{
														for( j = 0 ; j < Width ; j ++ )
														{
															s = (DWORD)sp[j] ;
															if( s > 0 )
															{
																D0 = (WORD)s ;
																for( l = -EdgeSize ; l < EdgeSize + 1 ; l ++ )
																{
																	for( m = -EdgeSize ; m < EdgeSize + 1 ; m ++ )
																	{
																		p = (WORD *)( dp + (j + m + adp) * 2 + (l * DestPitch) ) ;
																		*p = (WORD)( ( 0x100 - ( ( 0x10 - *p ) * ( 0x10 - s ) ) ) >> 4 ) ;
																	}
																}
															}
														}

														sp += SrcPitch ;
														dp += DestPitch ;
													}
												}

												// テクスチャのピクセルフォーマットに変換
												{
													BYTE aloc, rloc, gloc, bloc ;

													aloc = ( cl.AlphaLoc + cl.AlphaWidth ) - 4 ;
													rloc = ( cl.RedLoc   + cl.RedWidth   ) - 4 ;
													gloc = ( cl.GreenLoc + cl.GreenWidth ) - 4 ;
													bloc = ( cl.BlueLoc  + cl.BlueWidth  ) - 4 ;

													sp = Src - AddX - AddY * SrcPitch ;
													dp = Dest - AddX * 2 - AddY * DestPitch ;
													if( ManageData->TextureCacheUsePremulAlpha )
													{
														for( i = 0 ; i < Height + AddSize ; i ++ )
														{
															for( j = 0 ; j < Width + AddSize ; j ++ )
															{
																if( D0 == 16 )
																{
																	D0 = ( WORD )0xffff ;
																	E0 = 0 ;
																}
																else
																{
																	if( D0 )
																	{
																		D0 = ( WORD )( ( (DWORD)D0 << aloc ) | ( (DWORD)D0 << rloc ) | ( (DWORD)D0 << gloc ) | ( (DWORD)D0 << bloc ) ) ;
																	}

																	if( E0 == 16 )
																	{
																		E0 = ( WORD )0xffff ;
																	}
																	else
																	if( E0 )
																	{
																		E0 = ( WORD )( ( (DWORD)E0 << aloc ) | ( (DWORD)E0 << rloc ) | ( (DWORD)E0 << gloc ) | ( (DWORD)E0 << bloc ) ) ;
																	}
																}
															}

															sp += SrcPitch ;
															dp += DestPitch ;
														}
													}
													else
													{
														for( i = 0 ; i < Height + AddSize ; i ++ )
														{
															for( j = 0 ; j < Width + AddSize ; j ++ )
															{
																if( D0 == 16 )
																{
																	D0 = ( WORD )( ( ( (DWORD)D0 << aloc ) - 1 ) | RGBMask ) ;
																	E0 = (WORD)RGBMask ;
																}
																else
																{
																	if( D0 )
																	{
																		D0 = ( WORD )( ( ( (DWORD)D0 << aloc ) - 1 ) | RGBMask ) ;
																	}
																	else
																	{
																		D0 = (WORD)RGBMask ;
																	}

																	if( E0 )
																	{
																		E0 = ( WORD )( ( ( (DWORD)E0 << aloc ) - 1 ) | RGBMask ) ;
																	}
																	else
																	{
																		E0 = (WORD)RGBMask ;
																	}
																}
															}

															sp += SrcPitch ;
															dp += DestPitch ;
														}
													}
												}
											}
											break ;

										case GGO_GRAY8_BITMAP :
											{
												DWORD s ;
												
												if( EdgeSize == 1 )
												{
													for( i = 0 ; i < Height ; i ++ )
													{
														for( j = 0 ; j < Width ; j ++ )
														{
															s = (DWORD)sp[j] ;
															if( s > 0 )
															{
																DD0 = (DWORD)s ;
																ED1 = (DWORD)( ED1 + ( ( ( 0x40 - ED1 ) * s ) >> 6 ) ) ;
																ED2 = (DWORD)( ED2 + ( ( ( 0x40 - ED2 ) * s ) >> 6 ) ) ;
																ED3 = (DWORD)( ED3 + ( ( ( 0x40 - ED3 ) * s ) >> 6 ) ) ;
																ED4 = (DWORD)( ED4 + ( ( ( 0x40 - ED4 ) * s ) >> 6 ) ) ;
																ED5 = (DWORD)( ED5 + ( ( ( 0x40 - ED5 ) * s ) >> 6 ) ) ;
																ED6 = (DWORD)( ED6 + ( ( ( 0x40 - ED6 ) * s ) >> 6 ) ) ;
																ED7 = (DWORD)( ED7 + ( ( ( 0x40 - ED7 ) * s ) >> 6 ) ) ;
																ED8 = (DWORD)( ED8 + ( ( ( 0x40 - ED8 ) * s ) >> 6 ) ) ;
															}
														}

														sp += SrcPitch ;
														dp += DestPitch ;
													}
												}
												else
												if( EdgeSize <= FONTEDGE_PATTERN_NUM )
												{
													for( i = 0 ; i < Height ; i ++ )
													{
														for( j = 0 ; j < Width ; j ++ )
														{
															s = (DWORD)sp[j] ;
															if( s > 0 )
															{
																DD0 = (DWORD)s ;
																for( n = 0, l = -EdgeSize ; l < EdgeSize + 1 ; n ++, l ++ )
																{
																	for( o = 0, m = -EdgeSize ; m < EdgeSize + 1 ; o ++, m ++ )
																	{
																		if( EdgePat[n][o] == 0 ) continue ;
																		pd = (DWORD *)( dp + (j + m + adp) * 4 + (l * DestPitch) ) ;
																		*pd = (DWORD)( *pd + ( ( ( 0x40 - *pd ) * s ) >> 6 ) ) ;
																	}
																}
															}
														}

														sp += SrcPitch ;
														dp += DestPitch ;
													}
												}
												else
												{
													for( i = 0 ; i < Height ; i ++ )
													{
														for( j = 0 ; j < Width ; j ++ )
														{
															s = (DWORD)sp[j] ;
															if( s > 0 )
															{
																DD0 = (DWORD)s ;
																for( l = -EdgeSize ; l < EdgeSize + 1 ; l ++ )
																{
																	for( m = -EdgeSize ; m < EdgeSize + 1 ; m ++ )
																	{
																		pd = (DWORD *)( dp + (j + m + adp) * 4 + (l * DestPitch) ) ;
																		*pd = (DWORD)( *pd + ( ( ( 0x40 - *pd ) * s ) >> 6 ) ) ;
																	}
																}
															}
														}

														sp += SrcPitch ;
														dp += DestPitch ;
													}
												}

												// テクスチャのピクセルフォーマットに変換
												{
													static BYTE NumConvTable[ 0x41 ] =
													{
														  0,   3,   7,  11,  15,  19,  23,  27,  31,  35,  39,  43,  47,  51,  55,  59, 
														 63,  67,  71,  75,  79,  83,  87,  91,  95,  99, 103, 107, 111, 115, 119, 123, 
														127, 131, 135, 139, 143, 147, 151, 155, 159, 163, 167, 171, 175, 179, 183, 187, 
														191, 195, 199, 203, 207, 211, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 
														255, 
													} ;

													sp = Src - AddX - AddY * SrcPitch ;
													dp = Dest - AddX * 4 - AddY * DestPitch ;
													if( ManageData->TextureCacheUsePremulAlpha )
													{
														DWORD tmp ;

														for( i = 0 ; i < Height + AddSize ; i ++ )
														{
															for( j = 0 ; j < Width + AddSize ; j ++ )
															{
																if( DD0 == 64 )
																{
																	DD0 = 0xffffffff ;
																	ED0 = 0 ;
																}
																else
																{
																	if( DD0 )
																	{
																		tmp = NumConvTable[ DD0 ] ;
																		DD0 = ( DWORD )( ( tmp << cl.AlphaLoc ) | ( tmp << cl.RedLoc ) | ( tmp << cl.GreenLoc ) | ( tmp << cl.BlueLoc ) ) ;
																	}

																	if( ED0 == 64 )
																	{
																		ED0 = 0xffffffff ;
																	}
																	else
																	if( ED0 )
																	{
																		tmp = NumConvTable[ ED0 ] ;
																		ED0 = ( DWORD )( ( tmp << cl.AlphaLoc ) | ( tmp << cl.RedLoc ) | ( tmp << cl.GreenLoc ) | ( tmp << cl.BlueLoc ) ) ;
																	}
																}
															}

															sp += SrcPitch ;
															dp += DestPitch ;
														}
													}
													else
													{
														for( i = 0 ; i < Height + AddSize ; i ++ )
														{
															for( j = 0 ; j < Width + AddSize ; j ++ )
															{
																if( DD0 == 64 )
																{
																	DD0 = AlphaRGBMask ;
																	ED0 = (DWORD)RGBMask ;
																}
																else
																{
																	if( DD0 )
																	{
																		DD0 = ( DWORD )( ( ( DWORD )NumConvTable[ DD0 ] << cl.AlphaLoc ) | RGBMask ) ;
																	}
																	else
																	{
																		DD0 = (DWORD)RGBMask ;
																	}

																	if( ED0 )
																	{
																		ED0 = ( DWORD )( ( ( DWORD )NumConvTable[ ED0 ] << cl.AlphaLoc ) | RGBMask ) ;
																	}
																	else
																	{
																		ED0 = (DWORD)RGBMask ;
																	}
																}
															}

															sp += SrcPitch ;
															dp += DestPitch ;
														}
													}
												}
											}
											break ;
										}
									}
	#undef ED0
	#undef ED1
	#undef ED2
	#undef ED3
	#undef ED4
	#undef ED5
	#undef ED6
	#undef ED7
	#undef ED8

	#undef DD0
	#undef DD1
	#undef DD2
	#undef DD3
	#undef DD4

	#undef E0
	#undef E1
	#undef E2
	#undef E3
	#undef E4
	#undef E5
	#undef E6
	#undef E7
	#undef E8

	#undef D0
	#undef D1
	#undef D2
	#undef D3
	#undef D4
									// テクスチャキャッシュに転送
									{
										RECT srect ;

	//									SETRECT( srect, 0, 0, FontData->SizeX + AddSize, FontData->SizeY + AddSize ) ;
										SETRECT( srect, DestX - 1, DestY - 1, FontData->SizeX + AddSize + DestX, FontData->SizeY + AddSize + DestY ) ;
										BltBmpOrGraphImageToGraph2Base( &im.ColorData, NULL, NULL, 
																		FALSE, &im, NULL,
																		&srect, DestX - 1, DestY - 1, ManageData->TextureCache ) ;

										srect.left += adp ;
										srect.right += adp ;
										BltBmpOrGraphImageToGraph2Base( &im.ColorData, NULL, NULL, 
																		FALSE, &im, NULL,
																		&srect, DestX - 1 + ManageData->SurfaceSize.cx, DestY - 1, ManageData->TextureCache ) ;
									}
								}
							}

							// データ解放
							DXFREE( AllocAddress ) ;
						}
					}
				}
			}

			// サイズ補正
			FontData->AddX  += ( short )AddSize ;
			FontData->SizeX += ( short )AddSize ;
			FontData->SizeY += ( short )AddSize ;

			// 成功したら情報を有効にする
			CodeData->ExistFlag = TRUE ;
			CodeData->DataPoint = FontData ;
			FontData->CodeData = CodeData ;
			FontData->ValidFlag = TRUE ;

			// 次の文字を登録する際のインデックスをセットする
			ManageData->Index ++ ;
			if( ManageData->Index ==  ( unsigned int )ManageData->MaxCacheCharNum )
				ManageData->Index = 0 ;
		}
	}

	// フォントを元に戻す
	SelectObject( DC, OldFont ) ;

	if( ManageData->UseTextOut )
	{
		// ビットマップを元に戻す
		SelectObject( DC, OldBmp ) ;
	}

	// ＤＣの削除
	DeleteDC( DC ) ;

	// 終了
	return FontData ;
}

// フォント管理データの取得
extern FONTMANAGE * GetFontManageDataToHandle( int FontHandle )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	if( FONTHCHK( FontHandle, ManageData ) )
		return NULL ;

	// 終了
	return ManageData ;
}

// 使用可能なフォントの名前をすべて列挙する
extern int NS_EnumFontName( TCHAR *NameBuffer, int NameBufferNum, int JapanOnlyFlag )
{
	HDC hdc ;
	ENUMFONTDATA FontData ;
	LOGFONT LogFont ;
	TCHAR *buffer;

	if( NameBuffer == NULL )
	{
		buffer = (TCHAR *)DXALLOC( 1024 * 256 );
	}
	else
	{
		buffer = NameBuffer;
	}

	// ネームバッファをセット
	FontData.FontBuffer = buffer ;
	FontData.FontNum = 0 ;
	FontData.BufferNum = NameBufferNum ;
	FontData.Valid = FALSE ;
	FontData.CheckFontName = NULL ;
	FontData.JapanOnlyFlag = JapanOnlyFlag ;

	// デバイスコンテキストの作成
	hdc = GetDC( NULL );

	// 列挙開始
	_MEMSET( &LogFont, 0, sizeof( LOGFONT ) ) ;
	LogFont.lfCharSet = DEFAULT_CHARSET ;
	LogFont.lfFaceName[0] = '\0' ;
	LogFont.lfPitchAndFamily = 0 ;
	EnumFontFamiliesEx( hdc, &LogFont, (FONTENUMPROC)EnumFontFamExProc, ( LPARAM )&FontData, 0  ) ;

	// デバイスコンテキストの解放
	ReleaseDC( NULL, hdc ) ;

	// メモリの解放
	if( NameBuffer == NULL ) DXFREE( buffer );

	// フォントデータ領域数を返す
	return FontData.FontNum ;
}

// 使用可能なフォントの名前をすべて列挙する
extern int NS_EnumFontNameEx( TCHAR *NameBuffer, int NameBufferNum, int CharSet )
{
	HDC hdc ;
	ENUMFONTDATA FontData ;
	LOGFONT LogFont ;
	TCHAR *buffer;

	if( NameBuffer == NULL )
	{
		buffer = (TCHAR *)DXALLOC( 1024 * 256 );
	}
	else
	{
		buffer = NameBuffer;
	}

	// ネームバッファをセット
	FontData.FontBuffer = buffer ;
	FontData.FontNum = 0 ;
	FontData.BufferNum = NameBufferNum ;
	FontData.Valid = FALSE ;
	FontData.CheckFontName = NULL ;
	FontData.JapanOnlyFlag = FALSE ;

	// デバイスコンテキストの作成
	hdc = GetDC( NULL );

	// 列挙開始
	_MEMSET( &LogFont, 0, sizeof( LOGFONT ) ) ;
	LogFont.lfCharSet = CharSet < 0 ? DEFAULT_CHARSET : ( BYTE )CharSetTable[ CharSet ] ;
	LogFont.lfFaceName[0] = '\0' ;
	LogFont.lfPitchAndFamily = 0 ;
	EnumFontFamiliesEx( hdc, &LogFont, (FONTENUMPROC)EnumFontFamExProcEx, ( LPARAM )&FontData, 0  ) ;

	// デバイスコンテキストの解放
	ReleaseDC( NULL, hdc ) ;

	// メモリの解放
	if( NameBuffer == NULL ) DXFREE( buffer );

	// フォントデータ領域数を返す
	return FontData.FontNum ;
}

// 指定のフォント名のフォントを列挙する
extern int NS_EnumFontNameEx2( TCHAR *NameBuffer, int NameBufferNum, const TCHAR *EnumFontName, int CharSet )
{
	HDC hdc ;
	ENUMFONTDATA FontData ;
	LOGFONT LogFont ;
	TCHAR *buffer;

	if( NameBuffer == NULL )
	{
		buffer = (TCHAR *)DXALLOC( 1024 * 256 );
	}
	else
	{
		buffer = NameBuffer;
	}

	// ネームバッファをセット
	FontData.FontBuffer = buffer ;
	FontData.FontNum = 0 ;
	FontData.BufferNum = NameBufferNum ;
	FontData.Valid = FALSE ;
	FontData.CheckFontName = NULL ;
	FontData.JapanOnlyFlag = FALSE ;

	// デバイスコンテキストの作成
	hdc = GetDC( NULL );

	// 列挙開始
	_MEMSET( &LogFont, 0, sizeof( LOGFONT ) ) ;
	LogFont.lfCharSet = CharSet < 0 ? DEFAULT_CHARSET : ( BYTE )CharSetTable[ CharSet ] ;
	_TSTRNCPY( LogFont.lfFaceName, EnumFontName, 31 ) ;
	LogFont.lfPitchAndFamily = 0 ;
	EnumFontFamiliesEx( hdc, &LogFont, (FONTENUMPROC)EnumFontFamExProcEx, ( LPARAM )&FontData, 0  ) ;

	// デバイスコンテキストの解放
	ReleaseDC( NULL, hdc ) ;

	// メモリの解放
	if( NameBuffer == NULL ) DXFREE( buffer );

	// フォントデータ領域数を返す
	return FontData.FontNum ;
}

// 指定のフォント名のフォントが存在するかどうかをチェックする( 戻り値  TRUE:存在する  FALSE:存在しない )
extern int NS_CheckFontName( const TCHAR *FontName, int CharSet /* DX_CHARSET_DEFAULT 等 */ )
{
	HDC hdc ;
	ENUMFONTDATA FontData ;
	LOGFONT LogFont ;

	// ネームバッファをセット
	FontData.FontBuffer = NULL ;
	FontData.FontNum = 0 ;
	FontData.BufferNum = 0 ;
	FontData.Valid = FALSE ;
	FontData.CheckFontName = FontName ;
	FontData.JapanOnlyFlag = FALSE ;

	// デバイスコンテキストの作成
	hdc = GetDC( NULL );

	// 列挙開始
	_MEMSET( &LogFont, 0, sizeof( LOGFONT ) ) ;
	LogFont.lfCharSet = CharSet < 0 ? DEFAULT_CHARSET : ( BYTE )CharSetTable[ CharSet ] ;
	_TSTRNCPY( LogFont.lfFaceName, FontName, 31 ) ;
	LogFont.lfPitchAndFamily = 0 ;
	EnumFontFamiliesEx( hdc, &LogFont, ( FONTENUMPROC )EnumFontFamExProcEx, ( LPARAM )&FontData, 0  ) ;

	// デバイスコンテキストの解放
	ReleaseDC( NULL, hdc ) ;

	// フォントが存在したかどうかを返す
	return FontData.Valid ;
}

// InitFontToHandle の内部関数
extern int InitFontToHandleBase( int Terminate )
{
	FONTMANAGE * DefFont ;
	TCHAR DefFontName[128] ;
	int DefFontThickness ;
	int	DefFontSize ;
	int DefFontEdgeSize ;
	int	DefFontType ;
	int	DefFontCharSet ;
	int DefFontItalic ;
	int UseDefaultFont ;

	if( NS_GetUseDDrawObj() == NULL ) return -1 ;

	DXST_ERRORLOG_ADD( _T( "フォントの初期化を行います\n" ) ) ;
	DXST_ERRORLOG_TABADD ;

	// デフォルトフォントのデータを保存
	UseDefaultFont = FSYS.DefaultFontHandle > 0 ? TRUE : FALSE ;
	if( UseDefaultFont )
	{
		DefFont = GetFontManageDataToHandle( FSYS.DefaultFontHandle ) ;
		lstrcpy( DefFontName, DefFont->FontName ) ;
		DefFontSize      =	DefFont->FontSize      ;
		DefFontEdgeSize  =  DefFont->EdgeSize      ;
		DefFontThickness = 	DefFont->FontThickness ;
		DefFontType      =	DefFont->FontType      ;
		DefFontCharSet   = 	DefFont->CharSet       ;
		DefFontItalic    =  DefFont->Italic        ;
	}

	// すべてのフォントデータを削除
	AllHandleSub( DX_HANDLETYPE_FONT ) ;

	// 後始末中ではない場合はデフォルトフォントを作成
	if( Terminate == FALSE )
	{
		CREATEFONTTOHANDLE_GPARAM GParam ;
		InitCreateFontToHandleGParam( &GParam ) ;

		if( UseDefaultFont )
		{
			FSYS.DefaultFontHandle = CreateFontToHandle_UseGParam( &GParam, DefFontName,          DefFontSize,          DefFontThickness,      DefFontType,          DefFontCharSet,          DefFontEdgeSize,          DefFontItalic,          -1, FALSE ) ;
		}
		else
		{
			FSYS.DefaultFontHandle = CreateFontToHandle_UseGParam( &GParam, FSYS.DefaultFontName, FSYS.DefaultFontSize, FSYS.DefaultFontThick, FSYS.DefaultFontType, FSYS.DefaultFontCharSet, FSYS.DefaultFontEdgeSize, FSYS.DefaultFontItalic, -1, FALSE ) ;
		}
		NS_SetDeleteHandleFlag( FSYS.DefaultFontHandle, &FSYS.DefaultFontHandle ) ;
	}

	DXST_ERRORLOG_TABSUB ;
	DXST_ERRORLOG_ADD( _T( "フォントの初期化は正常に終了しました\n" ) ) ;

	// 終了
	return 0 ;
}

// フォントのステータスをデフォルトに戻す
extern int NS_InitFontToHandle( void )
{
	return InitFontToHandleBase() ;
}

// フォントハンドルを初期化する
extern int InitializeFontHandle( HANDLEINFO *HandleInfo )
{
	FONTMANAGE * ManageData = ( FONTMANAGE * )HandleInfo ;

	// キャッシュハンドルを初期化
	ManageData->TextureCache = -1 ;
	ManageData->TextureCacheSub = -1 ;

	// 終了
	return 0 ;
}

// フォントハンドルの後始末をする
extern int TerminateFontHandle( HANDLEINFO *HandleInfo )
{
	FONTMANAGE * ManageData = ( FONTMANAGE * )HandleInfo ;

	// フォントオブジェクトを削除
	if( ManageData->FontObj != NULL )
	{
		DeleteObject( ManageData->FontObj ) ;
		ManageData->FontObj = NULL ;
	}
	
	// テキストキャッシュ用メモリの解放
	if( ManageData->CacheMem != NULL )
	{
		DXFREE( ManageData->CacheMem ) ;
		ManageData->CacheMem = NULL ;
	}

	// テクスチャグラフィック削除
	if( ManageData->TextureCache >= 0 )
	{
		NS_DeleteGraph( ManageData->TextureCache ) ;
		NS_DeleteGraph( ManageData->TextureCacheSub ) ;
		ManageData->TextureCache = -1 ;
		ManageData->TextureCacheSub = -1 ;
	}

	// テクスチャキャッシュと同じイメージの基本イメージの解放
	if( ManageData->TextureCacheBaseImage.GraphData != NULL )
	{
		DXFREE( ManageData->TextureCacheBaseImage.GraphData ) ;
		ManageData->TextureCacheBaseImage.GraphData = NULL ;
	}

	// TextOut を使用するフォントで使用するビットマップを解放
	if( ManageData->CacheBitmap != NULL )
	{
		DeleteObject( ManageData->CacheBitmap ) ;
		ManageData->CacheBitmap = NULL ;
		ManageData->CacheBitmapMem = NULL ;
	}

	// ロストフラグが設定されている場合は TRUE にする
	if( ManageData->LostFlag != NULL )
	{
		*ManageData->LostFlag = TRUE ;
	}

	// 終了
	return 0 ;
}

// CREATEFONTTOHANDLE_GPARAM のデータをセットする
extern void InitCreateFontToHandleGParam( CREATEFONTTOHANDLE_GPARAM *GParam )
{
	GParam->CacheCharNum = FSYS.CacheCharNum ;
	GParam->TextureCacheColorBitDepth16Flag = FSYS.TextureCacheColorBitDepth16Flag ;
	GParam->NotTextureFlag = FSYS.NotTextureFlag ;
	GParam->UsePremulAlphaFlag = FSYS.UsePremulAlphaFlag ;
}

// CreateFontToHandle の実処理関数
static int CreateFontToHandle_Static(
	CREATEFONTTOHANDLE_GPARAM *GParam,
	int FontHandle,
	const TCHAR *FontName,
	int Size,
	int Thick,
	int FontType,
	int CharSet,
	int EdgeSize,
	int Italic,
	int ASyncThread
)
{
	HFONT NewFont = NULL ;
	FONTMANAGE * ManageData = NULL ;
	int AddHeight = 0 ;
	int OrigHeight = 0 ;
	int EnableAddHeight = FALSE ;
	int DefaultCharSet = FALSE ;
	int SampleScale = 1 ;
	int CreateFontSize ;

	if( WinData.DestroyMessageCatchFlag == TRUE ) return -1 ;

	if( FontType 	== -1 ) FontType 	= DEFAULT_FONT_TYPE ;
	if( EdgeSize	> 1   )	FontType	|= DX_FONTTYPE_EDGE ;

	if( Size 		== -1 ) Size 		= DEFAULT_FONT_SIZE ;
	if( Thick 		== -1 ) Thick 		= DEFAULT_FONT_THINCK ;
	if( EdgeSize	== -1 ) EdgeSize	= DEFAULT_FONT_EDGESIZE ;
	if( CharSet		== -1 )
	{
		DefaultCharSet = TRUE ;
		CharSet = _GET_CHARSET() ;
	}

	FONTHCHK( FontHandle, ManageData ) ;

	if( NS_GetColorBitDepth() == 8 && ( FontType & DX_FONTTYPE_ANTIALIASING ) ) FontType &= ~DX_FONTTYPE_ANTIALIASING ;

	switch( FontType )
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
	CreateFontSize = Size * SampleScale ;

	// フォントのパラメータのセット
	ManageData->FontSize 		= Size ;
	ManageData->FontThickness 	= Thick ;
	ManageData->FontType		= FontType ;
	ManageData->EdgeSize		= EdgeSize ;
	ManageData->Italic			= Italic ;
	ManageData->Space			= 0 ;

	// Direct3D を使用する場合はテクスチャキャッシュを使用する
	ManageData->TextureCacheFlag = GRA2.ValidHardWare && GParam->NotTextureFlag == FALSE ? TRUE : FALSE ;
	ManageData->TextureCacheColorBitDepth = ManageData->TextureCacheFlag == FALSE || ( FontType & DX_FONTTYPE_ANTIALIASING ) == 0 || GParam->TextureCacheColorBitDepth16Flag ? 16 : 32 ;
	ManageData->TextureCacheUsePremulAlpha = GParam->UsePremulAlphaFlag ;

CREATEFONTLABEL:

	// フォントの作成
	if( FontName == NULL )
		ManageData->FontName[0] = _T( '\0' ) ;
	else
		lstrcpy( ManageData->FontName, FontName ) ;

	if( ManageData->FontName[0] != _T( '\0' ) )
	{
		// 特に文字セットの指定が無い場合で、且つ指定のフォント名の指定の文字セットが無い場合は文字セットを DEFAULT_CHARSET にする
		if( DefaultCharSet == TRUE )
		{
			TCHAR TempNameBuffer[ 16 ][ 64 ] ;
			TCHAR *TempNameBufferP ;
			int TempNameNum ;
			int TempNameBufferAlloc ;
			int i ;

			TempNameNum = NS_EnumFontNameEx2( TempNameBuffer[ 0 ], 16, ManageData->FontName, CharSet ) ;
			if( TempNameNum >= 16 )
			{
				TempNameNum = NS_EnumFontNameEx2( NULL, 0, ManageData->FontName, CharSet ) ;
				TempNameBufferP = ( TCHAR * )DXALLOC( TempNameNum * 64 * sizeof( TCHAR ) ) ;
				TempNameNum = NS_EnumFontNameEx2( TempNameBufferP, TempNameNum, ManageData->FontName, CharSet ) ;
				TempNameBufferAlloc = TRUE ;
			}
			else
			{
				TempNameBufferAlloc = FALSE ;
				TempNameBufferP = TempNameBuffer[ 0 ] ;
			}

			for( i = 0 ; i < TempNameNum && _TSTRCMP( TempNameBufferP + i * 64, ManageData->FontName ) != 0 ; i ++ ){}
			if( i == TempNameNum )
			{
				CharSet = DX_CHARSET_DEFAULT ;
			}

			if( TempNameBufferAlloc )
			{
				DXFREE( TempNameBufferP ) ;
				TempNameBufferP = NULL ;
			}
		}

//		NewFont = CreateFont( CreateFontSize, 0, 0, 0, ManageData->FontThickness * 100, FALSE,
//								FALSE, FALSE, HANGEUL_CHARSET, /*OUT_TT_ONLY_PRECIS*/ /*OUT_RASTER_PRECIS*/ OUT_DEFAULT_PRECIS,
//								CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,  FIXED_PITCH, ManageData->FontName );
		NewFont = CreateFont( CreateFontSize + AddHeight, 0, 0, 0, ManageData->FontThickness * 100, Italic,
								FALSE, FALSE, CharSetTable[CharSet], /*OUT_TT_ONLY_PRECIS*/ /*OUT_RASTER_PRECIS*/ OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,  FIXED_PITCH, ManageData->FontName );
		if( NewFont == NULL )
		{
			NewFont = CreateFont( CreateFontSize + AddHeight, 0, 0, 0, ManageData->FontThickness * 100, Italic,
									FALSE, FALSE, DEFAULT_CHARSET,	/*OUT_TT_ONLY_PRECIS*/ /*OUT_RASTER_PRECIS*/ OUT_DEFAULT_PRECIS,
									CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,  FIXED_PITCH, ManageData->FontName );
			if( NewFont == NULL )
			{
				NewFont = CreateFont( CreateFontSize + AddHeight, 0, 0, 0, ManageData->FontThickness * 100, Italic,
										FALSE, FALSE, SHIFTJIS_CHARSET, OUT_DEFAULT_PRECIS,
										CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,  FIXED_PITCH, ManageData->FontName );
				if( NewFont == NULL )
				{
					DXST_ERRORLOG_ADD( _T( "指定のフォントの作成に失敗しました\n" ) ) ;
					goto ERR ;
				}
			}
		}
	}
	else
	{
		NewFont = CreateFont( CreateFontSize + AddHeight, 0, 0, 0, ManageData->FontThickness * 100, Italic, FALSE, FALSE, CharSetTable[CharSet]/*DEFAULT_CHARSET*/,
											OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,  FIXED_PITCH, _T( "ＭＳ ゴシック" ) );
		ManageData->FontName[0] = _T( '\0' ) ;
	}

	if( NewFont == NULL )
	{
		NewFont = CreateFont( CreateFontSize + AddHeight, 0, 0, 0, ManageData->FontThickness * 100, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
											OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,  FIXED_PITCH, _T( "ＭＳ ゴシック" ) );
		if( NewFont == NULL )
		{
			NewFont = CreateFont( CreateFontSize + AddHeight, 0, 0, 0, ManageData->FontThickness * 100, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
												OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,  FIXED_PITCH, NULL );
		}
		ManageData->FontName[0] = _T( '\0' ) ;

		if( NewFont == NULL )
		{
			DXST_ERRORLOG_ADD( _T( "フォントの作成に失敗しました\n" ) ) ;
			goto ERR ;
		}
	}

	// 文字のサイズを取得する
	{
		HDC DC ;
		HFONT OldFont ;
		TEXTMETRIC TextInfo ;

		// プライマリサーフェスのデバイスコンテキストを取得
		if( ( DC = CreateCompatibleDC( NULL ) ) == NULL )
		{
			DXST_ERRORLOG_ADD( _T( "ＤＣの取得に失敗しました\n" ) ) ;
			goto ERR ;
		}

		OldFont = ( HFONT )SelectObject( DC, NewFont ) ;										// フォントのセット
		GetTextMetrics( DC, &TextInfo ) ;														// フォントのステータスを取得

		// もし TextInfo.tmInternalLeading + TextInfo.tmExternalLeading が 0 ではなかったらその高さを追加して作成しなおす
		if( EnableAddHeight == FALSE )
		{
			if( TextInfo.tmInternalLeading + TextInfo.tmExternalLeading > 0 )
			{
				OrigHeight = TextInfo.tmHeight ;
				AddHeight = _FTOL( CreateFontSize / ( 1.0f - ( float )( TextInfo.tmInternalLeading + TextInfo.tmExternalLeading ) / TextInfo.tmHeight ) ) - CreateFontSize ;
				EnableAddHeight = TRUE ;
				SelectObject( DC, OldFont ) ;
				DeleteDC( DC ) ;
				DeleteObject( NewFont ) ;
				NewFont = NULL ;
				goto CREATEFONTLABEL ;
			}
			
			ManageData->FontAddHeight = 0 ;
		}
		else
		{
			ManageData->FontAddHeight = ( TextInfo.tmHeight - OrigHeight ) / SampleScale ;
		}

		ManageData->MaxWidth = TextInfo.tmMaxCharWidth / SampleScale + 4 ;						// フォントの最大サイズを取得
		if( FontType & DX_FONTTYPE_EDGE ) ManageData->MaxWidth += ManageData->EdgeSize * 2 ;	// フォントにエッジをつける場合は最大サイズ＋(エッジのサイズ×２)にする
		ManageData->MaxWidth = ( ManageData->MaxWidth + 7 ) / 8 * 8 ;							// フォントの最大幅を 8 の倍数にする

		ManageData->FontHeight = TextInfo.tmHeight / SampleScale + 1 ;
		if( FontType & DX_FONTTYPE_EDGE ) ManageData->FontHeight += ManageData->EdgeSize * 2 ;	// フォントにエッジをつける場合は最大サイズ＋(エッジのサイズ×２)にする
		if( ManageData->FontHeight & 1 ) ManageData->FontHeight ++ ;
		if( ManageData->MaxWidth < ManageData->FontHeight ) ManageData->MaxWidth = ManageData->FontHeight;

		// GetGlyphOutline が使用できるかどうかを調べる
		{
			GLYPHMETRICS gm ;
			MAT2 mt = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } } ;
			unsigned int Code ;
			DWORD DataSize ;

			_MEMSET( &gm, 0, sizeof( GLYPHMETRICS ) ) ;
			Code = ' ' ;
			DataSize = GetGlyphOutline( DC, Code, GGO_BITMAP, &gm, 0, NULL, &mt ) ;

			// 失敗した場合は TextOut 方式を使用する
			if( DataSize == GDI_ERROR )
			{
				ManageData->UseTextOut = TRUE ;

				// 使用するテクスチャキャッシュは 16bit 固定
				ManageData->TextureCacheColorBitDepth = 16 ;
			}
		}

		SelectObject( DC, OldFont ) ;															// フォントを元に戻す
		DeleteDC( DC ) ;																		// ＤＣを削除する
	}

	// フォントのデータを保存
	if( ManageData->FontObj != NULL ) DeleteObject( ManageData->FontObj ) ;
	ManageData->FontObj = NewFont ;
	NewFont = NULL ;

	// テキストキャッシュサーフェスの作成
	{
		// キャッシュできる最大文字数を決定
R1 :
		if( ManageData->TextureCacheFlag == TRUE )
		{
			// テクスチャキャッシュを使用する場合

			ManageData->MaxCacheCharNum = GParam->CacheCharNum ;
			if( ManageData->MaxCacheCharNum == 0 )
			{
				ManageData->SurfaceSize.cx = 512 ;
				ManageData->SurfaceSize.cy = 512 ;
				for(;;)
				{
					ManageData->MaxCacheCharNum = ( ManageData->SurfaceSize.cx / ManageData->MaxWidth ) *
													( ManageData->SurfaceSize.cy / ManageData->MaxWidth ) ;
					if( ManageData->MaxCacheCharNum >= 10 ) break ;

					if( ManageData->SurfaceSize.cx > ManageData->SurfaceSize.cy )
						ManageData->SurfaceSize.cy <<= 1 ;
					else
						ManageData->SurfaceSize.cx <<= 1 ;
				}
			}
			else
			{
				int x, num ;

				// 指定の数が収まるテクスチャサイズを計算する
				num = _FTOL( _SQRT( (float)ManageData->MaxCacheCharNum ) ) + 1 ;
				ManageData->SurfaceSize.cy = ManageData->SurfaceSize.cx = num * ManageData->MaxWidth ;
				x = 8 ;	while( x < ManageData->SurfaceSize.cx ) x <<= 1 ;
				ManageData->SurfaceSize.cy = ManageData->SurfaceSize.cx = x ;

				ManageData->MaxCacheCharNum = ( ManageData->SurfaceSize.cx / ManageData->MaxWidth ) *
												( ManageData->SurfaceSize.cy / ManageData->MaxWidth ) ;
			}
			if( ManageData->MaxCacheCharNum > FONT_CACHE_MAXNUM ) 
				ManageData->MaxCacheCharNum = FONT_CACHE_MAXNUM ;
			
			ManageData->LengthCharNum = ManageData->SurfaceSize.cy / ManageData->MaxWidth ;

			// キャッシュに使用するテクスチャグラフィックを作成する
			ManageData->TextureCacheLostFlag = TRUE ;
			if( RefreshFontDrawResourceToHandle( FontHandle, ASyncThread ) < 0 )
			{
				DXST_ERRORLOG_ADD( _T( "テクスチャ文字キャッシュの作成に失敗しました\n" ) ) ;
				ManageData->TextureCacheFlag = FALSE ;
				goto R1 ;
			}

			// 基本イメージ関係の初期化
			{
				int alpha, test, edge ;
				BASEIMAGE *im ;

				if( FontType & DX_FONTTYPE_ANTIALIASING )
				{
					alpha = TRUE ;
					test = FALSE ;
				}
				else
				{
					alpha = FALSE ;
					test = TRUE ;
				}
				edge = ( FontType & DX_FONTTYPE_EDGE ) != 0 ? TRUE : FALSE ;

				// キャッシュに転送するイメージと同じものを格納しておく基本イメージの作成
				{
					im = &ManageData->TextureCacheBaseImage ;
					im->ColorData = *( NS_GetTexColorData( alpha, test, ManageData->TextureCacheColorBitDepth == 16 ? 0 : 1 ) ) ;

					im->MipMapCount = 0 ;
					im->GraphDataCount = 0 ;
					im->Width  = ManageData->SurfaceSize.cx * ( edge ? 2 : 1 ) ;
					im->Height = ManageData->SurfaceSize.cy ;
					im->Pitch  = im->Width * im->ColorData.PixelByte ;
					im->GraphData = DXALLOC( im->Pitch * im->Height ) ;
					if( im->GraphData == NULL )
					{
						DXST_ERRORLOG_ADD( _T( "文字キャッシュ基本イメージ用のメモリの確保に失敗しました\n" ) ) ;
						goto ERR ;
					}

					// 乗算済みアルファを使用する場合はRGBA=0に、使用しない場合はAだけを0にして初期化
					if( ManageData->TextureCacheUsePremulAlpha )
					{
						NS_FillBaseImage( im, 0,0,0,0 ) ;
					}
					else
					{
						NS_FillBaseImage( im, 255,255,255,0 ) ;
					}
				}

				// 一時的にキャッシュに使用するグラフィックイメージデータの初期化
/*				{
					im = &ManageData->TextureTempCache ;
					im->ColorData = *( NS_GetTexColorData( alpha, test, 0 ) ) ;

					im->MipMapCount = 0 ;
					im->GraphDataCount = 0 ;
					im->Width  = ManageData->MaxWidth ;	if( edge == TRUE ) im->Width   = im->Width * 2 ;
					im->Height = ManageData->MaxWidth ;	if( edge == TRUE ) im->Height += EdgeSize  * 2 ;
					im->Pitch = im->Width * im->ColorData.PixelByte ;
					im->GraphData = DXCALLOC( im->Width * im->Height * im->ColorData.PixelByte ) ;
					if( im->GraphData == NULL )
					{
						DXST_ERRORLOG_ADD( _T( "文字キャッシュ処理用メモリの確保に失敗しました\n" ) ) ;
						goto ERR ;
					}
				}
*/			}
		}
		else
		{
			// メモリ上のキャッシュを使用する場合
			int BitNum = 0 ;
			
			// １ピクセル分のデータを保存するに当たり必要なビット数をセット
			switch( FontType )
			{
			case DX_FONTTYPE_NORMAL :                 BitNum = 1 ; break ;
			case DX_FONTTYPE_EDGE :                   BitNum = 8 ; break ;
			case DX_FONTTYPE_ANTIALIASING :           BitNum = 8 ; break ;
			case DX_FONTTYPE_ANTIALIASING_4X4 :       BitNum = 8 ; break ;
			case DX_FONTTYPE_ANTIALIASING_8X8 :       BitNum = 8 ; break ;
			case DX_FONTTYPE_ANTIALIASING_EDGE :      BitNum = 8 ; break ;
			case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :  BitNum = 8 ; break ;
			case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :  BitNum = 8 ; break ;
			}

			// キャッシュ文字の数を調整
			ManageData->MaxCacheCharNum = GParam->CacheCharNum ;
			if( ManageData->MaxCacheCharNum == 0 )
			{
				ManageData->MaxCacheCharNum = FONT_CACHE_MEMORYSIZE / ( ManageData->MaxWidth * ManageData->MaxWidth ) ;
				if( ManageData->MaxCacheCharNum < 20 ) ManageData->MaxCacheCharNum = 20 ;
			}
			if( ManageData->MaxCacheCharNum > FONT_CACHE_MAXNUM ) 
				ManageData->MaxCacheCharNum = FONT_CACHE_MAXNUM ;

			// キャッシュイメージのサイズをセット
			ManageData->SurfaceSize.cx = ManageData->MaxWidth ;
			ManageData->SurfaceSize.cy = ManageData->MaxCacheCharNum * ManageData->MaxWidth ;

			// キャッシュ文字数のセット
			ManageData->MaxCacheCharNum = ManageData->SurfaceSize.cy / ManageData->MaxWidth ;
			ManageData->LengthCharNum = ManageData->MaxCacheCharNum ;
			ManageData->CacheDataBitNum = BitNum ;

			// キャッシュ用メモリの確保
			ManageData->CachePitch = ( BitNum * ManageData->SurfaceSize.cx + 7 ) / 8 ;
			ManageData->CacheMem = (unsigned char *)DXALLOC( ManageData->CachePitch * ManageData->SurfaceSize.cy ) ;
			if( ManageData->CacheMem == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "フォントのキャッシュ用メモリの確保に失敗しました in CreateFontToHandle" ) ) ;
				goto ERR ;
			}

			// ブレンド描画時用フォントグラフィックの作成
			if( RefreshFontDrawResourceToHandle( FontHandle, ASyncThread ) < 0 )
			{
				DXST_ERRORLOG_ADD( _T( "フォント用描画スクリーンの作成に失敗しました\n" ) ) ;
				goto ERR ;
			}
		}
	}

	// TextOut 方式を使用する場合は DIB を作成しておく
	ManageData->CacheBitmap = NULL ;
	ManageData->CacheBitmapMem = NULL ;
	ManageData->CacheBitmapMemPitch = 0 ;
	if( ManageData->UseTextOut )
	{
		HDC hdc ;
		BITMAPINFO *BmpInfoPlus ;
		BITMAP BmpData ; 

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
			return DxLib_Error( _T( "メモリの確保に失敗しました in CFont" ) ) ;
		}
		BmpInfoPlus->bmiHeader.biSize	= sizeof( BITMAPINFOHEADER ) ;
		BmpInfoPlus->bmiHeader.biWidth	= ManageData->MaxWidth ;
		BmpInfoPlus->bmiHeader.biHeight	= -ManageData->MaxWidth ;
		BmpInfoPlus->bmiHeader.biPlanes	= 1 ;
		BmpInfoPlus->bmiHeader.biBitCount= 8 ;
		BmpInfoPlus->bmiHeader.biCompression = BI_RGB ;
		BmpInfoPlus->bmiHeader.biSizeImage = ManageData->MaxWidth * ManageData->MaxWidth ;

		// カラーパレットのセット
		{
			RGBQUAD *Color ;
			int i ;

			Color = &BmpInfoPlus->bmiColors[0] ;
			for( i = 0 ; i < 256 ; i ++ )
			{
				Color->rgbBlue = i ;
				Color->rgbRed = i ;
				Color->rgbBlue = i ;
				Color->rgbReserved = 0 ;

				Color ++ ;
			}
		}

		// ＤＣ取得
		hdc = CreateCompatibleDC( NULL ) ;

		// ＤＩＢデータを作成する
		ManageData->CacheBitmapMem = NULL ; 
		ManageData->CacheBitmap = CreateDIBSection( hdc , BmpInfoPlus , DIB_PAL_COLORS , ( void ** )&ManageData->CacheBitmapMem , NULL , 0 ) ;

		// ピッチを得る
		GetObject( ManageData->CacheBitmap , sizeof( BITMAP ) , &BmpData ) ;
		ManageData->CacheBitmapMemPitch = BmpData.bmWidthBytes ;

		// ＤＣ削除
		DeleteDC( hdc ) ;

		// メモリの解放
		DXFREE( BmpInfoPlus ) ;
	}

	// テキストキャッシュデータの初期化
	InitFontCacheToHandle( FontHandle, ASyncThread ) ; 

	// フォントの文字セットを保存
	ManageData->CharSet = CharSet ;

	// ロストフラグへのポインタを NULL にしておく
	ManageData->LostFlag = NULL ;

	// 正常終了
	return 0 ;

	// エラー処理
ERR :
	if( NewFont != NULL ) DeleteObject( NewFont ) ; 

	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD

// CreateFontToHandle の非同期読み込みスレッドから呼ばれる関数
static void CreateFontToHandle_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	CREATEFONTTOHANDLE_GPARAM *GParam ;
	int FontHandle ;
	const TCHAR *FontName ;
	int Size ;
	int Thick ;
	int FontType ;
	int CharSet ;
	int EdgeSize ;
	int Italic ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( CREATEFONTTOHANDLE_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	FontHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FontName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	Size = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Thick = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FontType = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	CharSet = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	EdgeSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Italic = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = CreateFontToHandle_Static( GParam, FontHandle, FontName, Size, Thick, FontType, CharSet, EdgeSize, Italic, TRUE ) ;

	DecASyncLoadCount( FontHandle ) ;
	if( Result < 0 )
	{
		SubHandle( FontHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// CreateFontToHandle のグローバル変数にアクセスしないバージョン
extern int CreateFontToHandle_UseGParam(
	CREATEFONTTOHANDLE_GPARAM *GParam,
	const TCHAR *FontName,
	int Size,
	int Thick,
	int FontType,
	int CharSet,
	int EdgeSize,
	int Italic,
	int Handle,
	int ASyncLoadFlag
)
{
	int FontHandle ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	FontHandle = AddHandle( DX_HANDLETYPE_FONT, Handle ) ;
	if( FontHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, FontHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FontName ) ;
		AddASyncLoadParamInt( NULL, &Addr, Size ) ;
		AddASyncLoadParamInt( NULL, &Addr, Thick ) ;
		AddASyncLoadParamInt( NULL, &Addr, FontType ) ;
		AddASyncLoadParamInt( NULL, &Addr, CharSet ) ;
		AddASyncLoadParamInt( NULL, &Addr, EdgeSize ) ;
		AddASyncLoadParamInt( NULL, &Addr, Italic ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = CreateFontToHandle_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FontHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FontName ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Size ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Thick ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FontType ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, CharSet ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, EdgeSize ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Italic ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( FontHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( CreateFontToHandle_Static( GParam, FontHandle, FontName, Size, Thick, FontType, CharSet, EdgeSize, Italic, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return FontHandle ;

ERR :
	SubHandle( FontHandle ) ;

	return -1 ;
}

// 新しいフォントデータを作成
extern int NS_CreateFontToHandle( const TCHAR *FontName,
									int Size, int Thick,
									int FontType, int CharSet,
									int EdgeSize, int Italic,
									int Handle )
{
	CREATEFONTTOHANDLE_GPARAM GParam ;

	InitCreateFontToHandleGParam( &GParam ) ;
	return CreateFontToHandle_UseGParam( &GParam, FontName, Size, Thick, FontType, CharSet, EdgeSize, Italic, Handle, GetASyncLoadFlag() ) ;
}

// 字間を変更する
extern int NS_SetFontSpaceToHandle( int Point, int FontHandle )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;
	
	ManageData->Space = Point ;
	
	// 終了
	return 0 ;
}

// デフォルトフォントのステータスを一括設定する
extern int NS_SetDefaultFontState( const TCHAR *FontName, int Size, int Thick, int FontType, int CharSet, int EdgeSize, int Italic )
{
	if( FontType 	== -1 ) FontType 	= DEFAULT_FONT_TYPE ;
	if( EdgeSize	> 1   )	FontType	|= DX_FONTTYPE_EDGE ;

	if( Size 		== -1 ) Size 		= DEFAULT_FONT_SIZE ;
	if( Thick 		== -1 ) Thick 		= DEFAULT_FONT_THINCK ;
	if( EdgeSize	== -1 ) EdgeSize	= DEFAULT_FONT_EDGESIZE ;
//	if( CharSet		== -1 ) CharSet     = _GET_CHARSET() ;

	lstrcpy( FSYS.DefaultFontName, FontName != NULL ? FontName : _T( "" ) ) ;
	FSYS.DefaultFontType     = FontType ;
	FSYS.DefaultFontSize     = Size ;
	FSYS.DefaultFontThick    = Thick ;
	FSYS.DefaultFontCharSet  = CharSet ;
	FSYS.DefaultFontEdgeSize = EdgeSize ;
	FSYS.DefaultFontItalic   = Italic ;

	// 初期化前の場合はここで終了
	if( FSYS.InitializeFlag == FALSE )
	{
		FSYS.EnableInitDefaultFontName     = TRUE ;
		FSYS.EnableInitDefaultFontType     = TRUE ;
		FSYS.EnableInitDefaultFontSize     = TRUE ;
		FSYS.EnableInitDefaultFontThick    = TRUE ;
		FSYS.EnableInitDefaultFontCharSet  = TRUE ;
		FSYS.EnableInitDefaultFontEdgeSize = TRUE ;
		FSYS.EnableInitDefaultFontItalic   = TRUE ;

		return 0 ;
	}
	
	RefreshDefaultFont() ;

	// 終了
	return 0 ;
}


// フォントキャッシュの制御を終了する
extern int NS_DeleteFontToHandle( int FontHandle )
{
	return SubHandle( FontHandle ) ;
}

// 解放時に TRUE にするフラグへのポインタを設定する
extern int NS_SetFontLostFlag( int FontHandle, int *LostFlag )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;

	ManageData->LostFlag = LostFlag ;
	if( LostFlag != NULL ) *LostFlag = FALSE ;

	return 0 ;
}

// 描画するフォントのサイズをセットする
extern int NS_SetFontSize( int FontSize )
{
	if( FontSize == -1 ) FontSize = DEFAULT_FONT_SIZE ;

	FSYS.DefaultFontSize = FontSize ;

	// 初期化前の場合はここで終了
	if( FSYS.InitializeFlag == FALSE )
	{
		FSYS.EnableInitDefaultFontSize = TRUE ;

		return 0 ;
	}

	RefreshDefaultFont() ;

	// 終了
	return 0 ;
}


// フォントの太さをセット
extern int NS_SetFontThickness( int ThickPal )
{
	if( ThickPal == -1 ) ThickPal = DEFAULT_FONT_THINCK ;

	FSYS.DefaultFontThick = ThickPal ;

	// 初期化前の場合はここで終了
	if( FSYS.InitializeFlag == FALSE )
	{
		FSYS.EnableInitDefaultFontThick = TRUE ;

		return 0 ;
	}

	RefreshDefaultFont() ;

	// 終了
	return 0 ;
}

// 字間を変更する
extern int NS_SetFontSpace( int Point )
{
	// 初期化前の場合はここで終了
	if( FSYS.InitializeFlag == FALSE )
	{
		FSYS.DefaultFontSpace = Point ;

		FSYS.EnableInitDefaultFontSpace = TRUE ;

		return 0 ;
	}

	RefreshDefaultFont() ;

	FSYS.DefaultFontSpace = Point ;

	NS_SetFontSpaceToHandle( Point, FSYS.DefaultFontHandle ) ;

	// 終了
	return 0 ;
}

// デフォルトフォントハンドルの字間を取得する
extern int NS_GetFontSpace( void )
{
	return NS_GetFontSpaceToHandle( DX_DEFAULT_FONT_HANDLE ) ;
}

// フォントのキャッシュにテクスチャを使用するか、フラグをセットする
extern int NS_SetFontCacheToTextureFlag( int Flag )
{
	// フラグを保存
	FSYS.NotTextureFlag = !Flag ;
	
	// 終了
	return 0 ;
}

// フォントのキャッシュにテクスチャを使用するか、フラグをセットする(誤字版)
extern int NS_SetFontChacheToTextureFlag( int Flag )
{
	return NS_SetFontCacheToTextureFlag( Flag ) ;
}

// フォントキャッシュでキャッシュできる文字数を指定する
extern int NS_SetFontCacheCharNum( int CharNum )
{
	FSYS.CacheCharNum = CharNum ;

	// 終了
	return 0 ;
}



// フォントを変更
extern int NS_ChangeFont( const TCHAR *FontName, int CharSet )
{
	lstrcpy( FSYS.DefaultFontName, FontName != NULL ? FontName : _T( "" ) ) ;

//	if( CharSet == -1 )
//		CharSet = _GET_CHARSET() ;

	FSYS.DefaultFontCharSet = CharSet ;

	// 初期化前の場合はここで終了
	if( FSYS.InitializeFlag == FALSE )
	{
		FSYS.EnableInitDefaultFontName = TRUE ;
		FSYS.EnableInitDefaultFontCharSet = TRUE ;

		return 0 ;
	}

	RefreshDefaultFont() ;

	// 終了
	return 0 ;
}

// フォントタイプの変更
extern int NS_ChangeFontType( int FontType )
{
	if( FontType == -1 ) FontType = DEFAULT_FONT_TYPE ;

	FSYS.DefaultFontType = FontType ;

	// 初期化前の場合はここで終了
	if( FSYS.InitializeFlag == FALSE )
	{
		FSYS.EnableInitDefaultFontType = TRUE ;

		return 0 ;
	}

	RefreshDefaultFont() ;

	// 終了
	return 0 ;
}







// 文字列を描画
extern int FontCacheStringDrawToHandleST(
	int				DrawFlag,
	int				xi,
	int				yi,
	float			xf,
	float			yf,
	int				PosIntFlag,
	bool			ExRateValidFlag,
	double			ExRateX,
	double			ExRateY,
	const			TCHAR *StrData,
	int				Color,
	MEMIMG *		DestMemImg,
	const RECT *	ClipRect,
	int				TransFlag,
	int				FontHandle,
	int				EdgeColor,
	int				StrLen,
	int				VerticalFlag,
	SIZE *			DrawSize
)
{
	static TCHAR	DrawStrBuf[ 256 * 3 ] ;		// 描画する際に使用する文字列データ（３バイトで１文字を表現）
	FONTMANAGE *	ManageData ;
	FONTDATA *		FontData ;
	FONTCODEDATA *	FontCodeData ;
	RECT			SrcRect ;
	RECT			DstRect ;
	int				DrawCharNum ;				// ２バイト文字１バイト文字全部ひっくるめて何文字あるか保存する変数
	int				i ;							// 繰り返しと汎用変数
	int				SrcX ;						// テキストキャッシュ中の目的テキストの座標決定に使用
	int				SrcY ;
	const TCHAR *	StrPoint ;
	double			DrawPos ;
	double			ExRate ;
	DWORD			FColor ;
	DWORD			FEdgeColor ;
	DWORD			DstPitch = 0 ;
	DWORD			SrcPitch ;
	bool			UseAlloc = false ;
	TCHAR *			AllocDrawStrBuf ;
	TCHAR *			UseDrawStrBuf ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( GBASE.NotDrawFlag || NS_GetUseDDrawObj() == NULL ) return -1 ;
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1  ;
//	DXST_ERRORLOG_ADD( _T( "フォントハンドル値が異常です終了します\n" ) ) ;
//	DXST_ERRORLOGFMT_ADD(( _T( "	in FontCacheStringDrawToHandle  StrData = %s   FontHandle = %d  " ), StrData, FontHandle )) ;

	if( DestMemImg != NULL || ManageData->TextureCache == FALSE )
	{
		PosIntFlag = TRUE ;
	}

	// 描画座標の補正
	if( VerticalFlag )
	{
		if( PosIntFlag )
		{
			xi -=    _DTOL( ManageData->FontAddHeight * ExRateY ) / 2 ;
		}
		else
		{
			xf -= ( float )( ManageData->FontAddHeight * ExRateY ) / 2.0f ;
		}
	}
	else
	{
		if( PosIntFlag )
		{
			yi -=    _DTOL( ManageData->FontAddHeight * ExRateY ) / 2 ;
		}
		else
		{
			yf -= ( float )( ManageData->FontAddHeight * ExRateY ) / 2.0f ;
		}
	}

	if( DrawFlag && ExRateValidFlag == false || ManageData->TextureCacheFlag == FALSE )
	{
		ExRateValidFlag = false ;
		ExRateX = 1.0 ;
		ExRateY = 1.0 ;
	}

	ExRate = VerticalFlag ? ExRateY : ExRateX ;

	// ３２ビット色カラー値を取得しておく
	{
		int r, g, b ;
		RGBCOLOR *bright ;
	
		bright = &GBASE.DrawBright ;
		
		// ＤＸライブラリに設定されている描画輝度と乗算する
		NS_GetColor2( Color, &r, &g, &b ) ;
		r          = bright->Red   * r / 255 ;
		g          = bright->Green * g / 255 ;
		b          = bright->Blue  * b / 255 ;
		Color      = NS_GetColor( r, g, b ) & ~NS_GetColor( 0, 0, 0 ) ;
		FColor     = ( 0xff000000 ) | ( r << 16 ) | ( g << 8 ) | ( b ) ;

		NS_GetColor2( EdgeColor, &r, &g, &b ) ;
		r          = bright->Red   * r / 255 ;
		g          = bright->Green * g / 255 ;
		b          = bright->Blue  * b / 255 ;
		EdgeColor  = NS_GetColor( r, g, b ) & ~NS_GetColor( 0, 0, 0 ) ;;
		FEdgeColor = ( 0xff000000 ) | ( r << 16 ) | ( g << 8 ) | ( b ) ;
	}

	// 文字列がない場合は終了
	if( StrData == NULL ) return 0 ;
	if( StrData[ 0 ] == 0 ) return 0 ;

	// 描画する文字列の長さを保存
	{
		int len ;

		len = lstrlen( StrData ) ;
		if( StrLen == -1 ) StrLen = len ;
		if( StrLen > len ) StrLen = len ;

		if( len > 256 )
		{
			UseAlloc = true ;
			AllocDrawStrBuf = ( TCHAR * )DXALLOC( ( len * 3 ) * sizeof( TCHAR ) ) ;
			if( AllocDrawStrBuf == NULL )
				return -1 ;
			UseDrawStrBuf = AllocDrawStrBuf ;
		}
		else
		{
			UseDrawStrBuf = DrawStrBuf ;
		}
	}
	
	// ピッチのセット
	if( DestMemImg != NULL ) DstPitch = DestMemImg->Base->Pitch ;
	SrcPitch = ManageData->CachePitch ;

	// キャッシュに入っていない文字列をキャッシュに入れる
	FontCacheStringAddToHandle( FontHandle, StrData, StrLen, UseDrawStrBuf, &DrawCharNum ) ;
	FontCodeData = ManageData->FontCodeData ;

	if( DrawFlag == TRUE )
	{
		// 文字の数だけ繰り返し
		StrPoint = UseDrawStrBuf ;			// 描画用文字データの先頭アドレスをセット
		DrawPos = 0.0 ;
		for( i = 0 ; i < DrawCharNum ; i ++, DrawPos += ( FontData->AddX + ManageData->Space ) * ExRate, StrPoint += 3 )
		{
			// キャッシュの中に希望の文字があるか検査
			if( FontCodeData[ *( ( WORD *)StrPoint ) ].ExistFlag )
			{
				// あった場合そのフォントデータのポインタを保存
				FontData = FontCodeData[ *( ( WORD *)StrPoint ) ].DataPoint ; 
			}
			else
			{
				// 無かった場合キャッシュに追加

				// 新たにキャッシュ文字を作成
				FontData = FontCacheCharAddToHandle( 1, StrPoint, FontHandle ) ;
				if( FontData == NULL )
				{
					DXST_ERRORLOG_ADD( _T( "テキストキャッシュの作成に失敗しました" ) ) ;
					goto ERR ;
				}
			}

			// 画像データが存在する矩形を算出
			SrcY = ( FontData->GraphIndex % ManageData->LengthCharNum ) * ManageData->MaxWidth ;
			SrcX =   FontData->GraphIndex / ManageData->LengthCharNum   * ManageData->MaxWidth ;
			if( ManageData->TextureCacheFlag == TRUE )
			{
				SrcX ++ ;
				SrcY ++ ;
			}
			SETRECT( SrcRect, SrcX, SrcY, SrcX + FontData->SizeX, SrcY + FontData->SizeY ) ;

			// スペース文字だった場合はキャンセルする
			if( *StrPoint == _T( ' ' ) ||
#ifdef UNICODE
				*StrPoint                  ==             L'　'
#else
				*( ( WORD * )StrPoint )    == *( ( WORD * )"　" )
#endif
				)
				continue ;

			// サイズが無かったら何もしない
			if( FontData->SizeX == 0 && FontData->SizeY == 0 ) 
				continue ;

			// 文字の描画
			if( ManageData->TextureCacheFlag == TRUE )
			{
				DWORD color ;
				int anti ;
				int BlendGraph, BorderParam, BorderRange ;

				// 既存のブレンド画像の情報を保存しておく
				BlendGraph  = GBASE.BlendGraph ;
				BorderParam = GBASE.BlendGraphBorderParam ;
				BorderRange = GBASE.BlendGraphBorderRange ;

				// ブレンド画像を無効にする
				NS_SetBlendGraph( -1, 0, 0 ) ;

				// 描画色を保存しておく
				color = GBASE.bDrawBright ;
				
				// アンチエイリアスフォントか
				anti = ( ManageData->FontType & DX_FONTTYPE_ANTIALIASING ) != 0 ;

				// 縦書きかどうかで処理を分岐
				if( VerticalFlag == TRUE )
				{
					// 縦書き

					RECT MotoDrawRect, TempRect ;
					int GraphSizeX, GraphSizeY ;
					int TempGraph ;

					// 元の描画可能矩形を保存しておく
					MotoDrawRect = GBASE.DrawArea ;

					// キャッシュテクスチャのサイズを取得する
					NS_GetGraphSize( ManageData->TextureCache, &GraphSizeX, &GraphSizeY ) ;

					// 拡大描画かどうかで処理を分岐
					if( ExRateValidFlag == false )
					{
						// 等倍描画

						if( PosIntFlag )
						{
							int DrawX, DrawY, x1, y1, x2, y2 ;

							// 描画元座標の算出
							DrawX = xi + ManageData->FontHeight - FontData->DrawY ;
							DrawY = yi + _DTOL( DrawPos )       + FontData->DrawX ;

							x1 = DrawX + SrcY ;
							y1 = DrawY - SrcX ;
							x2 = DrawX + SrcY - GraphSizeY ;
							y2 = DrawY - SrcX + GraphSizeX ;

							// 指定矩形だけしか描画出来ないようにする
							TempRect.left   = ( DrawX - FontData->SizeY ) - 1 ;
							TempRect.top    = DrawY ;
							TempRect.right  = DrawX + 1 ;
							TempRect.bottom = DrawY + FontData->SizeX ;
							RectClipping( &TempRect, &MotoDrawRect ) ;
							NS_SetDrawArea( TempRect.left, TempRect.top, TempRect.right, TempRect.bottom ) ;

							// エッジがある場合はそれを先に描画する
							if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
							{
								SetDrawBrightToOneParam( FEdgeColor ) ;
								NS_DrawModiGraphF(
									x1 - 0.5F, y1                       - 0.5F,
									x1 - 0.5F, y2 - ( GraphSizeX >> 1 ) - 0.5F,
									x2 - 0.5F, y2 - ( GraphSizeX >> 1 ) - 0.5F,
									x2 - 0.5F, y1                       - 0.5F,
									ManageData->TextureCacheSub, TRUE
								) ;
							}

							// 本体を描画
							SetDrawBrightToOneParam( FColor ) ;
							NS_DrawModiGraphF(
								x1 - 0.5F, y1 - 0.5F,
								x1 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y1 - 0.5F,
								 ManageData->TextureCache,
								 TRUE/*anti ? FALSE : TRUE*/
							) ;

							// 描画可能矩形を元に戻す
							NS_SetDrawArea( MotoDrawRect.left,  MotoDrawRect.top,
											MotoDrawRect.right, MotoDrawRect.bottom ) ;
						}
						else
						{
							float DrawX, DrawY, x1, y1, x2, y2 ;

							// 描画元座標の算出
							DrawX = xf +                    ManageData->FontHeight - FontData->DrawY ;
							DrawY = yf + ( float )DrawPos +                          FontData->DrawX ;

							x1 = DrawX ;
							y1 = DrawY ;
							x2 = DrawX + -FontData->SizeY ;
							y2 = DrawY +  FontData->SizeX ;

							// エッジがある場合はそれを先に描画する
							if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
							{
								TempGraph = NS_DerivationGraph( SrcX, SrcY, FontData->SizeX, FontData->SizeY, ManageData->TextureCacheSub ) ;
								SetDrawBrightToOneParam( FEdgeColor ) ;
								NS_DrawModiGraphF(
									x1 - 0.5F, y1 - 0.5F,
									x1 - 0.5F, y2 - 0.5F,
									x2 - 0.5F, y2 - 0.5F,
									x2 - 0.5F, y1 - 0.5F,
									TempGraph, TRUE
								) ;
								NS_DeleteGraph( TempGraph ) ;
							}

							// 本体を描画
							TempGraph = NS_DerivationGraph( SrcX, SrcY, FontData->SizeX, FontData->SizeY, ManageData->TextureCache ) ;
							SetDrawBrightToOneParam( FColor ) ;
							NS_DrawModiGraphF(
								x1 - 0.5F, y1 - 0.5F,
								x1 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y1 - 0.5F,
								TempGraph,
								TRUE
							) ;
							NS_DeleteGraph( TempGraph ) ;
						}
					}
					else
					{
						float DrawX, DrawY, x1, y1, x2, y2 ;

						// 拡大描画

						if( PosIntFlag )
						{
							// 描画元座標の算出
							DrawX = ( float )( xi + ( ManageData->FontHeight - FontData->DrawY ) * ExRateX ) ;
							DrawY = ( float )( yi + DrawPos + FontData->DrawX * ExRateY ) ;

							x1 = DrawX ;
							y1 = DrawY ;
							x2 = ( float )( DrawX + -FontData->SizeY * ExRateX ) ;
							y2 = ( float )( DrawY +  FontData->SizeX * ExRateY ) ;

							// 指定矩形だけしか描画出来ないようにする
							TempRect.left   = _FTOL( ( float )( DrawX - _DTOL( FontData->SizeY * ExRateX ) ) ) - 1 ;
							TempRect.top    = _FTOL( DrawY ) ;
							TempRect.right  = _FTOL( DrawX ) + 1 ;
							TempRect.bottom = _FTOL( ( float )( DrawY + FontData->SizeX * ExRateY ) ) ;
							RectClipping( &TempRect, &MotoDrawRect ) ;
							NS_SetDrawArea( TempRect.left, TempRect.top, TempRect.right, TempRect.bottom ) ;

							// エッジがある場合はそれを先に描画する
							if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
							{
								TempGraph = NS_DerivationGraph( SrcX, SrcY, FontData->SizeX, FontData->SizeY, ManageData->TextureCacheSub ) ;
								SetDrawBrightToOneParam( FEdgeColor ) ;
								NS_DrawModiGraphF(
									x1 - 0.5F, y1 - 0.5F,
									x1 - 0.5F, y2 - 0.5F,
									x2 - 0.5F, y2 - 0.5F,
									x2 - 0.5F, y1 - 0.5F,
									TempGraph, TRUE
								) ;
								NS_DeleteGraph( TempGraph ) ;
							}

							// 本体を描画
							TempGraph = NS_DerivationGraph( SrcX, SrcY, FontData->SizeX, FontData->SizeY, ManageData->TextureCache ) ;
							SetDrawBrightToOneParam( FColor ) ;
							NS_DrawModiGraphF(
								x1 - 0.5F, y1 - 0.5F,
								x1 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y1 - 0.5F,
								TempGraph,
								TRUE/*anti ? FALSE : TRUE*/
							) ;
							NS_DeleteGraph( TempGraph ) ;

							// 描画可能矩形を元に戻す
							NS_SetDrawArea( MotoDrawRect.left,  MotoDrawRect.top,
											MotoDrawRect.right, MotoDrawRect.bottom ) ;
						}
						else
						{
							// 描画元座標の算出
							DrawX = ( float )( xf +           ( ManageData->FontHeight - FontData->DrawY ) * ExRateX ) ;
							DrawY = ( float )( yf + DrawPos +                            FontData->DrawX   * ExRateY ) ;

							x1 = DrawX ;
							y1 = DrawY ;
							x2 = DrawX + -FontData->SizeY * ( float )ExRateX ;
							y2 = DrawY +  FontData->SizeX * ( float )ExRateY ;

							// エッジがある場合はそれを先に描画する
							if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
							{
								TempGraph = NS_DerivationGraph( SrcX, SrcY, FontData->SizeX, FontData->SizeY, ManageData->TextureCacheSub ) ;
								SetDrawBrightToOneParam( FEdgeColor ) ;
								NS_DrawModiGraphF(
									x1 - 0.5F, y1 - 0.5F,
									x1 - 0.5F, y2 - 0.5F,
									x2 - 0.5F, y2 - 0.5F,
									x2 - 0.5F, y1 - 0.5F,
									TempGraph, TRUE
								) ;
								NS_DeleteGraph( TempGraph ) ;
							}

							// 本体を描画
							TempGraph = NS_DerivationGraph( SrcX, SrcY, FontData->SizeX, FontData->SizeY, ManageData->TextureCache ) ;
							SetDrawBrightToOneParam( FColor ) ;
							NS_DrawModiGraphF(
								x1 - 0.5F, y1 - 0.5F,
								x1 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y2 - 0.5F,
								x2 - 0.5F, y1 - 0.5F,
								TempGraph,
								TRUE/*anti ? FALSE : TRUE*/
							) ;
							NS_DeleteGraph( TempGraph ) ;
						}
					}
				}
				else
				{
					// 横書き

					// 拡大描画かどうかで処理を分岐
					if( ExRateValidFlag == false )
					{
						// 等倍描画

						// エッジがある場合はそれを先に描画する
						if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
						{
							SetDrawBrightToOneParam( FEdgeColor ) ;
							if( PosIntFlag )
							{
								NS_DrawRectGraph(
									xi + _DTOL( DrawPos ) + FontData->DrawX,
									yi                    + FontData->DrawY,
									SrcX,
									SrcY,
									FontData->SizeX,
									FontData->SizeY,
									ManageData->TextureCacheSub,
									TRUE,
									FALSE
								) ;
							}
							else
							{
								NS_DrawRectGraphF(
									xf + ( float )DrawPos + FontData->DrawX,
									yf                    + FontData->DrawY,
									SrcX,
									SrcY,
									FontData->SizeX,
									FontData->SizeY,
									ManageData->TextureCacheSub,
									TRUE,
									FALSE
								) ;
							}
						}

						// 本体を描画
						SetDrawBrightToOneParam( FColor ) ;
						if( PosIntFlag )
						{
							NS_DrawRectGraph(
								xi + _DTOL( DrawPos ) + FontData->DrawX,
								yi                    + FontData->DrawY,
								SrcX,
								SrcY,
								FontData->SizeX,
								FontData->SizeY,
								ManageData->TextureCache,
								TRUE,
								FALSE
							) ;
						}
						else
						{
							NS_DrawRectGraphF(
								xf + ( float )DrawPos + FontData->DrawX,
								yf                    + FontData->DrawY,
								SrcX,
								SrcY,
								FontData->SizeX,
								FontData->SizeY,
								ManageData->TextureCache,
								TRUE,
								FALSE
							) ;
						}
					}
					else
					{
						float x1, y1, x2, y2 ;

						// 拡大描画

						if( PosIntFlag )
						{
							x1 = ( float )( xi  + DrawPos + FontData->DrawX * ExRateX ) ;
							y1 = ( float )( yi  +           FontData->DrawY * ExRateY ) ;
						}
						else
						{
							x1 = ( float )( xf  + DrawPos + FontData->DrawX * ExRateX ) ;
							y1 = ( float )( yf  +           FontData->DrawY * ExRateY ) ;
						}
						x2 = ( float )( x1 +           FontData->SizeX * ExRateX ) ;
						y2 = ( float )( y1 +           FontData->SizeY * ExRateY ) ;

						// エッジがある場合はそれを先に描画する
						if( ( ManageData->FontType & DX_FONTTYPE_EDGE ) != 0 )
						{
							SetDrawBrightToOneParam( FEdgeColor ) ;
							NS_DrawRectExtendGraphF(
								x1,								y1,
								x2,								y2,
								SrcX,							SrcY,
								FontData->SizeX,				FontData->SizeY,
								ManageData->TextureCacheSub,
								TRUE ) ;
						}

						// 本体を描画
						SetDrawBrightToOneParam( FColor ) ;
						NS_DrawRectExtendGraphF(
							x1,							y1,
							x2,							y2,
							SrcX,						SrcY,
							FontData->SizeX,			FontData->SizeY,
							ManageData->TextureCache,
							TRUE
						) ;
					}
				}

				// ブレンド画像の情報を元に戻す
				NS_SetBlendGraph( BlendGraph, BorderParam, BorderRange ) ;
				
				// 描画色を元に戻す
				SetDrawBrightToOneParam( color ) ;
			}
			else
			if( DestMemImg != NULL )
			{
				// 縦書きかどうかで処理を分岐
				if( VerticalFlag == TRUE )
				{
					DWORD SizeX, SizeY ;
					RECT TempRect ;

					// 描画矩形のセット
					DstRect.left   = xi + ManageData->FontHeight - FontData->DrawY - FontData->SizeY ;
					DstRect.right  = xi + ManageData->FontHeight - FontData->DrawY ;
					DstRect.top    = yi + _DTOL( DrawPos )       + FontData->DrawX ;
					DstRect.bottom = yi + _DTOL( DrawPos )       + FontData->DrawX + FontData->SizeX ;
					TempRect = DstRect ;
					RectClipping( &DstRect, ClipRect ) ;

					// 描画する文字が無かった場合ここで次ループに移る
					if( DstRect.right == DstRect.left || DstRect.bottom == DstRect.top ) continue ;

					// 転送元矩形と描画サイズの調整
					SrcRect.left   += DstRect.top     - TempRect.top ;
					SrcRect.top    += DstRect.right   - TempRect.right ;
					SrcRect.right  -= TempRect.bottom - DstRect.bottom ;
					SrcRect.bottom -= TempRect.left   - DstRect.left ;
					SizeX = SrcRect.right  - SrcRect.left ;
					SizeY = SrcRect.bottom - SrcRect.top ;

					// 传送
					{
						int SrcPitch2, DstPitch2 ;
						WORD SizeXWord = ( WORD )SizeX, SizeYWord = ( WORD )SizeY ;

						// カラービットモードによって処理を分岐

						switch( NS_GetColorBitDepth() )
						{
						case 8 :
							// 8ビットモードの時の処理
							{
								BYTE *Dest ;
								BYTE *Src ;
								BYTE ColorData = ( BYTE )( Color & 0xff ) ;

								Dest = DestMemImg->UseImage + DstRect.right + DstRect.top * DestMemImg->Base->Pitch ;
								Src = ManageData->CacheMem + SrcRect.left + SrcRect.top * ManageData->CachePitch ;

								SrcPitch2 = ManageData->CachePitch - SizeX ;
								DstPitch2 = -1 - SizeX * DestMemImg->Base->Pitch ;

								switch( ManageData->FontType )
								{
								case DX_FONTTYPE_NORMAL :
									{
										WORD SizeXTemp ;
										WORD Data ;
										int StX ;
										const WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

										StX = SrcRect.left % 8 ;
										Src = ManageData->CacheMem + ( SrcRect.left / 8 ) + SrcRect.top * ManageData->CachePitch ;
										SrcPitch2 = ManageData->CachePitch - ( ( SizeX + 7 ) / 8 ) ;

										if( TransFlag )
										{
											do{
												SizeXTemp = SizeXWord ;
												do{
													Data = *( ( WORD * )Src ) ;
													do{
														if( Data & Table[StX] )   *Dest = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+1] ) *Dest = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+2] ) *Dest = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+3] ) *Dest = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+4] ) *Dest = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+5] ) *Dest = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+6] ) *Dest = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+7] ) *Dest = ColorData ;	Dest += DstPitch ;	--SizeXTemp ;
													}while(0);

													Src += 1 ;
												}while( SizeXTemp );

												Src += SrcPitch2 ;
												Dest += DstPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do{
												SizeXTemp = SizeXWord ;
												do{
													Data = *( ( WORD * )Src ) ;
													do{
														*Dest = ( Data & Table[StX] ) ? ColorData : 0 ;  	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*Dest = ( Data & Table[StX+1] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*Dest = ( Data & Table[StX+2] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*Dest = ( Data & Table[StX+3] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*Dest = ( Data & Table[StX+4] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*Dest = ( Data & Table[StX+5] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*Dest = ( Data & Table[StX+6] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*Dest = ( Data & Table[StX+7] ) ? ColorData : 0 ;	Dest += DstPitch ;	--SizeXTemp ;
													}while(0);

													Src += 1 ;
												}while( SizeXTemp );

												Src += SrcPitch2 ;
												Dest += DstPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;


								case DX_FONTTYPE_EDGE :
									{
										WORD Width ;
										BYTE EColor ;

										EColor = ( BYTE )( EdgeColor & 0xff ) ;
										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src ) *Dest = *Src == 1 ? ColorData : EColor ;
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													*Dest = *Src ? ( *Src == 1 ? ColorData : EColor ) : 0 ;
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;
								}
							}
							break ;

						case 16 :
							// 16ビットモードの時の処理
							{
								BYTE *Dest ;
								BYTE *Src ;
								WORD ColorData = ( WORD )Color ;

								Dest = DestMemImg->UseImage + ( DstRect.right * 2 ) + DstRect.top * DstPitch ;
								Src = ManageData->CacheMem + SrcRect.left + SrcRect.top * SrcPitch ;

								SrcPitch2 = SrcPitch - SizeX ;
								DstPitch2 = -1 * 2 - SizeX * DstPitch ;

								switch( ManageData->FontType )
								{
								case DX_FONTTYPE_ANTIALIASING_EDGE :
								case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
								case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
									{
										BYTE Red1, Green1, Blue1 ;
										BYTE Red2, Green2, Blue2 ;
										DWORD AAA ;

										WORD Width ;

										Red1   = ( BYTE )( ( ColorData & MEMIMG_R5G6B5_R ) >> MEMIMG_R5G6B5_LR ) ;
										Green1 = ( BYTE )( ( ColorData & MEMIMG_R5G6B5_G ) >> MEMIMG_R5G6B5_LG ) ;
										Blue1  = ( BYTE )( ( ColorData & MEMIMG_R5G6B5_B ) >> MEMIMG_R5G6B5_LB ) ;

										Red2   = ( BYTE )( ( EdgeColor & MEMIMG_R5G6B5_R ) >> MEMIMG_R5G6B5_LR ) ;
										Green2 = ( BYTE )( ( EdgeColor & MEMIMG_R5G6B5_G ) >> MEMIMG_R5G6B5_LG ) ;
										Blue2  = ( BYTE )( ( EdgeColor & MEMIMG_R5G6B5_B ) >> MEMIMG_R5G6B5_LB ) ;

										Dest = DestMemImg->UseImage + ( DstRect.right * 4 ) + DstRect.top * DstPitch ;
										DstPitch2 = -1 * 4 - SizeX * DstPitch ;

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src & 0x0f )
													{
														if( ( *Src & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )Dest ) = 0xff0000 | ColorData ;
														}
														else
														{
															Dest[2] = ( ( *Src & 0x0f ) << 4 ) + ( ( 0x0f - ( *Src & 0x0f ) ) * ( ( *Src & 0xf0 ) >> 4 ) ) ;
															AAA = ( ( WORD )( *Src & 0x0f ) << 12 ) / Dest[2] ;
															*( ( WORD * )Dest ) = ( WORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_R5G6B5_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_R5G6B5_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_R5G6B5_LB ) ) ; 
														}
													}
													else
													{
														if( *Src & 0xf0 )
														{
															*( ( DWORD * )Dest ) = EdgeColor | ( ( *Src & 0xf0 ) << 16 ) ;
														}
													}

													Dest += DstPitch ;
													Src  += 1 ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src & 0x0f )
													{
														if( ( *Src & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )Dest ) = 0xff0000 | ColorData ;
														}
														else
														{
															Dest[2] = ( ( *Src & 0x0f ) << 4 ) + ( ( 0x0f - ( *Src & 0x0f ) ) * ( ( *Src & 0xf0 ) >> 4 ) ) ;
															AAA = ( ( WORD )( *Src & 0x0f ) << 12 ) / Dest[2] ;
															*( ( WORD * )Dest ) = ( WORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_R5G6B5_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_R5G6B5_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_R5G6B5_LB ) ) ; 
														}
													}
													else
													{
														if( *Src & 0xf0 )
														{
															*( ( DWORD * )Dest ) = EdgeColor | ( ( *Src & 0xf0 ) << 16 ) ;
														}
														else
														{
															*( ( DWORD * )Dest ) = 0 ;
														}
													}

													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_ANTIALIASING :
								case DX_FONTTYPE_ANTIALIASING_4X4 :
								case DX_FONTTYPE_ANTIALIASING_8X8 :
									{
										WORD Width ;

										Dest = DestMemImg->UseImage + ( DstRect.right * 4 ) + DstRect.top * DstPitch ;
										DstPitch2 = -1 * 4 - SizeX * DstPitch ;

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src )
													{
														*( ( DWORD * )Dest ) = ColorData | ( *Src << 20 ) ;
													}
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src )
													{
														*( ( DWORD * )Dest ) = ColorData | ( *Src << 20 ) ;
													}
													else
													{
														*( ( DWORD * )Dest ) = 0 ;
													}

													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_NORMAL :
									{
										WORD SizeXTemp ;
										WORD Data ;
										int StX = SrcRect.left % 8 ;
										WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

										Src = ManageData->CacheMem + ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
										SrcPitch2 = SrcPitch - ( ( SizeX + 7 ) / 8 ) ;

										if( TransFlag )
										{
											do{
												SizeXTemp = SizeXWord ;
												do{
													Data = *( ( WORD * )Src ) ;
													do{
														if( Data & Table[StX] )   *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+1] ) *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+2] ) *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+3] ) *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+4] ) *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+5] ) *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+6] ) *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+7] ) *( (WORD *)Dest ) = ColorData ;	Dest += DstPitch ;	--SizeXTemp ;
													}while(0);

													Src += 1 ;
												}while( SizeXTemp );

												Src += SrcPitch2 ;
												Dest += DstPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do{
												SizeXTemp = SizeXWord ;
												do{
													Data = *( ( WORD * )Src ) ;
													do{
														*( (WORD *)Dest ) = ( Data & Table[StX] ) ? ColorData : 0 ;  	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*( (WORD *)Dest ) = ( Data & Table[StX+1] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*( (WORD *)Dest ) = ( Data & Table[StX+2] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*( (WORD *)Dest ) = ( Data & Table[StX+3] ) ? ColorData : 0 ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														*( (WORD *)Dest ) = ( Data & Table[StX+4] ) ? ColorData : 0 ;	Dest += DstPitch ;  if( --SizeXTemp == 0 ) break ;
														*( (WORD *)Dest ) = ( Data & Table[StX+5] ) ? ColorData : 0 ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														*( (WORD *)Dest ) = ( Data & Table[StX+6] ) ? ColorData : 0 ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														*( (WORD *)Dest ) = ( Data & Table[StX+7] ) ? ColorData : 0 ;	Dest += DstPitch ; 	--SizeXTemp ;
													}while(0);

													Src += 1 ;
												}while( SizeXTemp );

												Src += SrcPitch2 ;
												Dest += DstPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_EDGE :
									{
										WORD Width ;
										WORD EColor ;

										EColor = ( WORD )( EdgeColor & 0xffff ) ;
										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src ) *( ( WORD * )Dest ) = *Src == 1 ? ColorData : EColor ;
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													*( ( WORD * )Dest ) = *Src ? ( *Src == 1 ? ColorData : EColor ) : 0 ;
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;
								}
							}
							break ;

						case 32 :
							// 32ビットモードの場合
							{
								BYTE *Dest ;
								BYTE *Src ;
								DWORD ColorData = ( DWORD )Color ;
   
								Dest = DestMemImg->UseImage + ( DstRect.right * 4 ) + DstRect.top * DstPitch ;
								Src = ManageData->CacheMem + SrcRect.left + SrcRect.top * SrcPitch ;

								SrcPitch2 = SrcPitch - SizeX ;
								DstPitch2 = -1 * 4 - ( SizeX * DstPitch ) ;

								switch( ManageData->FontType )
								{
								case DX_FONTTYPE_ANTIALIASING_EDGE :
								case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
								case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
									{
										BYTE Red1, Green1, Blue1 ;
										BYTE Red2, Green2, Blue2 ;
										DWORD AAA ;

										WORD Width ;

										Red1   = ( BYTE )( ( ColorData & MEMIMG_XRGB8_R ) >> MEMIMG_XRGB8_LR ) ;
										Green1 = ( BYTE )( ( ColorData & MEMIMG_XRGB8_G ) >> MEMIMG_XRGB8_LG ) ;
										Blue1  = ( BYTE )( ( ColorData & MEMIMG_XRGB8_B ) >> MEMIMG_XRGB8_LB ) ;

										Red2   = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_R ) >> MEMIMG_XRGB8_LR ) ;
										Green2 = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_G ) >> MEMIMG_XRGB8_LG ) ;
										Blue2  = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_B ) >> MEMIMG_XRGB8_LB ) ;

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src & 0x0f )
													{
														if( ( *Src & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )Dest ) = 0xff000000 | ColorData ;
														}
														else
														{
															*( ( DWORD * )Dest ) = ( ( ( *Src & 0x0f ) << 4 ) + ( ( 0x0f - ( *Src & 0x0f ) ) * ( ( *Src & 0xf0 ) >> 4 ) ) ) << 24 ;
															AAA = ( ( WORD )( *Src & 0x0f ) << 12 ) / Dest[3] ;
															*( ( DWORD * )Dest ) |= ( DWORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_XRGB8_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_XRGB8_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_XRGB8_LB ) ) ; 
														}
													}
													else
													{
														if( *Src & 0xf0 )
														{
															*( ( DWORD * )Dest ) = EdgeColor | ( ( *Src & 0xf0 ) << 24 ) ;
														}
													}

													Dest += DstPitch ;
													Src  ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src & 0x0f )
													{
														if( ( *Src & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )Dest ) = 0xff000000 | ColorData ;
														}
														else
														{
															*( ( DWORD * )Dest ) = ( ( ( *Src & 0x0f ) << 4 ) + ( ( 0x0f - ( *Src & 0x0f ) ) * ( ( *Src & 0xf0 ) >> 4 ) ) ) << 24 ;
															AAA = ( ( WORD )( *Src & 0x0f ) << 12 ) / Dest[3] ;
															*( ( DWORD * )Dest ) |= ( DWORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_XRGB8_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_XRGB8_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_XRGB8_LB ) ) ; 
														}
													}
													else
													{
														if( *Src & 0xf0 )
														{
															*( ( DWORD * )Dest ) = EdgeColor | ( ( *Src & 0xf0 ) << 24 ) ;
														}
														else
														{
															*( ( DWORD * )Dest ) = 0 ;
														}
													}

													Dest += DstPitch ;
													Src  ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;


								case DX_FONTTYPE_ANTIALIASING :
								case DX_FONTTYPE_ANTIALIASING_4X4 :
								case DX_FONTTYPE_ANTIALIASING_8X8 :
									{
										WORD Width ;

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src )
													{
														*( ( DWORD * )Dest ) = ColorData | ( ( *Src & 0x0f ) << 28 ) ;
													}
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src )
													{
														*( ( DWORD * )Dest ) = ColorData | ( ( *Src & 0x0f ) << 28 ) ;
													}
													else
													{
														*( ( DWORD * )Dest ) = 0 ;
													}

													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_NORMAL :
									{
										WORD SizeXTemp ;
										WORD Data ;
										int StX = SrcRect.left % 8 ;
										WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

										Src = ManageData->CacheMem + ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
										SrcPitch2 = SrcPitch - ( ( SizeX + 7 ) / 8 ) ;

										if( TransFlag )
										{
											do{
												SizeXTemp = SizeXWord ;
												do{
													Data = *( ( WORD * )Src ) ;
													do{
														if( Data & Table[StX] )   *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+1] ) *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+2] ) *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+3] ) *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+4] ) *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+5] ) *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+6] ) *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														if( Data & Table[StX+7] ) *( (DWORD *)Dest ) = ColorData ;	Dest += DstPitch ;	--SizeXTemp ;
													}while(0);

													Src += 1 ;
												}while( SizeXTemp );

												Src += SrcPitch2 ;
												Dest += DstPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do{
												SizeXTemp = SizeXWord ;
												do{
													Data = *( ( WORD * )Src ) ;
													do{
														*( (DWORD *)Dest ) = ( Data & Table[StX] ) ? ColorData : 0 ;   	Dest += DstPitch ;	if( --SizeXTemp == 0 ) break ;
														*( (DWORD *)Dest ) = ( Data & Table[StX+1] ) ? ColorData : 0 ;	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*( (DWORD *)Dest ) = ( Data & Table[StX+2] ) ? ColorData : 0 ;	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*( (DWORD *)Dest ) = ( Data & Table[StX+3] ) ? ColorData : 0 ;	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*( (DWORD *)Dest ) = ( Data & Table[StX+4] ) ? ColorData : 0 ;	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*( (DWORD *)Dest ) = ( Data & Table[StX+5] ) ? ColorData : 0 ;	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*( (DWORD *)Dest ) = ( Data & Table[StX+6] ) ? ColorData : 0 ;	Dest += DstPitch ; 	if( --SizeXTemp == 0 ) break ;
														*( (DWORD *)Dest ) = ( Data & Table[StX+7] ) ? ColorData : 0 ;	Dest += DstPitch ; 	--SizeXTemp ;
													}while(0);

													Src += 1 ;
												}while( SizeXTemp );

												Src += SrcPitch2 ;
												Dest += DstPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_EDGE :
									{
										WORD Width ;
										DWORD EColor ;

										EColor = ( DWORD )( EdgeColor & 0xffffff ) ;
										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *Src ) *( ( DWORD * )Dest ) = *Src == 1 ? ColorData : EColor ;
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													*( ( DWORD * )Dest ) = *Src ? ( *Src == 1 ? ColorData : EColor ) : 0 ;
													Dest += DstPitch ;
													Src ++ ;
												}while( --Width ) ;

												Dest += DstPitch2 ;
												Src += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;
								}
							}
							break ;
						}
					}
				}
				else
				{
					int DrawX, DrawY ;
					DWORD SizeX, SizeY ;

					// 描画座標のセット
					DrawX = xi + _DTOL( DrawPos ) + FontData->DrawX ;
					DrawY = yi + FontData->DrawY ;

					// 実際の描画領域を作成
					SETRECT( DstRect, DrawX, DrawY, DrawX + FontData->SizeX, DrawY + FontData->SizeY ) ;
					RectClipping( &DstRect, ClipRect ) ;

					// 描画する文字が無かった場合ここで次ループに移る
					if( DstRect.right == DstRect.left || DstRect.bottom == DstRect.top ) continue ;

					// 描画元矩形と描画先矩形をセット
					SrcRect.left   += DstRect.left - DrawX ;
					SrcRect.top    += DstRect.top  - DrawY ;
					SrcRect.right  -= ( DrawX + FontData->SizeX ) - DstRect.right ;
					SrcRect.bottom -= ( DrawY + FontData->SizeY ) - DstRect.bottom ;
					SizeX = SrcRect.right - SrcRect.left ;
					SizeY = SrcRect.bottom - SrcRect.top ;

					// 传送
					{
						DWORD SrcPitch2, DstPitch2 ;
						WORD SizeXWord = ( WORD )SizeX, SizeYWord = ( WORD )SizeY ;

						// カラービットモードによって処理を分岐

						switch( NS_GetColorBitDepth() )
						{
						case 8 :
							// 8ビットモードの時の処理
							{
								BYTE *DrawDstPoint = ( BYTE * )DestMemImg->UseImage ;
								BYTE *DrawSrcPoint = ManageData->CacheMem ;
								BYTE ColorData     = ( BYTE )( Color & 0xff ) ;

								DrawDstPoint += ( DstRect.left ) + DstRect.top * DstPitch ;
								DrawSrcPoint += ( SrcRect.left ) + SrcRect.top * SrcPitch ;

								SrcPitch2 = SrcPitch - SizeX ;
								DstPitch2 = DstPitch - SizeX ;

								switch( ManageData->FontType )
								{
								case DX_FONTTYPE_NORMAL :
									{
										WORD SizeXTemp ;
										WORD Data ;

										if( SrcRect.left % 8 == 0 )
										{
											DrawSrcPoint = ManageData->CacheMem ;
											DrawSrcPoint += ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
											SrcPitch2 = SrcPitch - ( SizeX + 15 ) / 16 * 2 ;
											DstPitch2 = DstPitch - ( SizeX / 16 * 16 ) ;

											if( TransFlag == TRUE )
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															if( Data & 0x80 ) *( DrawDstPoint ) = ColorData ;			if( --SizeXTemp == 0 ) break ;
															if( Data & 0x40 ) *( DrawDstPoint + 1 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x20 ) *( DrawDstPoint + 2 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x10 ) *( DrawDstPoint + 3 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x8 ) *( DrawDstPoint + 4 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x4 ) *( DrawDstPoint + 5 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x2 ) *( DrawDstPoint + 6 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x1 ) *( DrawDstPoint + 7 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x8000 ) *( DrawDstPoint + 8 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x4000 ) *( DrawDstPoint + 9 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x2000 ) *( DrawDstPoint + 10 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x1000 ) *( DrawDstPoint + 11 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x800 ) *( DrawDstPoint + 12 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x400 ) *( DrawDstPoint + 13 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x200 ) *( DrawDstPoint + 14 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x100 ) *( DrawDstPoint + 15 ) = ColorData ;
															DrawDstPoint += 16 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 2 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
											else
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															*( DrawDstPoint ) = ( Data & 0x80 ) ? ColorData : 0 ;			if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 1 ) = ( Data & 0x40 ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 2 ) = ( Data & 0x20 ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 3 ) = ( Data & 0x10 ) ? ColorData : 0 ; 		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 4 ) = ( Data & 0x8 ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 5 ) = ( Data & 0x4 ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 6 ) = ( Data & 0x2 ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 7 ) = ( Data & 0x1 ) ? ColorData : 0 ; 		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 8 ) = ( Data & 0x8000 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 9 ) = ( Data & 0x4000 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 10 ) = ( Data & 0x2000 ) ? ColorData : 0 ; 	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 11 ) = ( Data & 0x1000 ) ? ColorData : 0 ;	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 12 ) = ( Data & 0x800 ) ? ColorData : 0 ; 	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 13 ) = ( Data & 0x400 ) ? ColorData : 0 ; 	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 14 ) = ( Data & 0x200 ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 15 ) = ( Data & 0x100 ) ? ColorData : 0 ;  
															DrawDstPoint += 16 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 2 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
										}
										else
										{
											int StX = SrcRect.left % 8 ;
											WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

											DrawSrcPoint = ManageData->CacheMem ;
											DrawSrcPoint += ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
											SrcPitch2 = SrcPitch - ( ( SizeX / 8 ) + ( SizeX % 8 != 0 ? 1 : 0 ) ) ;

											DstPitch2 = DstPitch - ( ( ( SizeX >> 3 ) << 3 ) ) ;

											if( TransFlag )
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															if( Data & Table[StX] ) *( DrawDstPoint ) = ColorData ;			if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+1] ) *( DrawDstPoint + 1 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+2] ) *( DrawDstPoint + 2 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+3] ) *( DrawDstPoint + 3 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+4] ) *( DrawDstPoint + 4 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+5] ) *( DrawDstPoint + 5 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+6] ) *( DrawDstPoint + 6 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+7] ) *( DrawDstPoint + 7 ) = ColorData ;
															DrawDstPoint += 8 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 1 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
											else
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															*( DrawDstPoint ) = ( Data & Table[StX] ) ? ColorData : 0 ;   		if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 1 ) = ( Data & Table[StX+1] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 2 ) = ( Data & Table[StX+2] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 3 ) = ( Data & Table[StX+3] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 4 ) = ( Data & Table[StX+4] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 5 ) = ( Data & Table[StX+5] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 6 ) = ( Data & Table[StX+6] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( DrawDstPoint + 7 ) = ( Data & Table[StX+7] ) ? ColorData : 0 ;  
															DrawDstPoint += 8 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 1 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
										}
									}
									break ;


								case DX_FONTTYPE_EDGE :
									{
										WORD Width ;
										BYTE EColor ;

										EColor = ( BYTE )( EdgeColor & 0xff ) ;
										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint )
													{
														*DrawDstPoint = *DrawSrcPoint == 1 ? ColorData : EColor ;
													}
													DrawDstPoint ++ ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													*DrawDstPoint = ( *DrawSrcPoint ) ? ( *DrawSrcPoint == 1 ? ColorData : EColor ) : 0 ;
													DrawDstPoint ++ ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;
								}
							}
							break ;

						case 16 :
							// 16ビットモードの時の処理
							{
								BYTE *DrawDstPoint = DestMemImg->UseImage ;
								BYTE *DrawSrcPoint = ManageData->CacheMem ;
								WORD ColorData = ( WORD )Color ;

								if( ManageData->FontType & DX_FONTTYPE_ANTIALIASING )
								{
									DrawDstPoint += DstRect.left * 4 + DstRect.top * DstPitch ;
									DstPitch2     = DstPitch - SizeX * 4 ;
								}
								else
								{
									DrawDstPoint += DstRect.left * 2 + DstRect.top * DstPitch ;
									DstPitch2     = DstPitch - SizeX * 2 ;
								}

								DrawSrcPoint += SrcRect.left + SrcRect.top * SrcPitch ;
								SrcPitch2     = SrcPitch - SizeX ;

								switch( ManageData->FontType )
								{
								case DX_FONTTYPE_ANTIALIASING_EDGE :
								case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
								case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
									{
										BYTE Red1, Green1, Blue1 ;
										BYTE Red2, Green2, Blue2 ;
										DWORD AAA ;
										WORD Width ;

										Red1   = ( BYTE )( ( ColorData & MEMIMG_R5G6B5_R ) >> MEMIMG_R5G6B5_LR ) ;
										Green1 = ( BYTE )( ( ColorData & MEMIMG_R5G6B5_G ) >> MEMIMG_R5G6B5_LG ) ;
										Blue1  = ( BYTE )( ( ColorData & MEMIMG_R5G6B5_B ) >> MEMIMG_R5G6B5_LB ) ;

										Red2   = ( BYTE )( ( EdgeColor & MEMIMG_R5G6B5_R ) >> MEMIMG_R5G6B5_LR ) ;
										Green2 = ( BYTE )( ( EdgeColor & MEMIMG_R5G6B5_G ) >> MEMIMG_R5G6B5_LG ) ;
										Blue2  = ( BYTE )( ( EdgeColor & MEMIMG_R5G6B5_B ) >> MEMIMG_R5G6B5_LB ) ;

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint & 0x0f )
													{
														if( ( *DrawSrcPoint & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )DrawDstPoint ) = 0xff0000 | ColorData ;
														}
														else
														{
															DrawDstPoint[2] = ( ( *DrawSrcPoint & 0x0f ) << 4 ) + ( ( 0x0f - ( *DrawSrcPoint & 0x0f ) ) * ( ( *DrawSrcPoint & 0xf0 ) >> 4 ) ) ;
															AAA = ( ( WORD )( *DrawSrcPoint & 0x0f ) << 12 ) / DrawDstPoint[2] ;
															*( ( WORD * )DrawDstPoint ) = ( WORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_R5G6B5_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_R5G6B5_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_R5G6B5_LB ) ) ; 
														}
													}
													else
													{
														if( *DrawSrcPoint & 0xf0 )
														{
															*( ( DWORD * )DrawDstPoint ) = EdgeColor | ( ( *DrawSrcPoint & 0xf0 ) << 16 ) ;
														}
													}

													DrawDstPoint += 4 ;
													DrawSrcPoint += 1 ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint & 0x0f )
													{
														if( ( *DrawSrcPoint & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )DrawDstPoint ) = 0xff0000 | ColorData ;
														}
														else
														{
															DrawDstPoint[2] = ( ( *DrawSrcPoint & 0x0f ) << 4 ) + ( ( 0x0f - ( *DrawSrcPoint & 0x0f ) ) * ( ( *DrawSrcPoint & 0xf0 ) >> 4 ) ) ;
															AAA = ( ( WORD )( *DrawSrcPoint & 0x0f ) << 12 ) / DrawDstPoint[2] ;
															*( ( WORD * )DrawDstPoint ) = ( WORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_R5G6B5_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_R5G6B5_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_R5G6B5_LB ) ) ; 
														}
													}
													else
													{
														if( *DrawSrcPoint & 0xf0 )
														{
															*( ( DWORD * )DrawDstPoint ) = EdgeColor | ( ( *DrawSrcPoint & 0xf0 ) << 16 ) ;
														}
														else
														{
															*( ( DWORD * )DrawDstPoint ) = 0 ;
														}
													}

													DrawDstPoint += 4 ;
													DrawSrcPoint += 1 ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_ANTIALIASING :
								case DX_FONTTYPE_ANTIALIASING_4X4 :
								case DX_FONTTYPE_ANTIALIASING_8X8 :
									{
										WORD Width ;

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint )
													{
														*( ( DWORD * )DrawDstPoint ) = ColorData | ( *DrawSrcPoint << 20 ) ;
													}
													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint )
													{
														*( ( DWORD * )DrawDstPoint ) = ColorData | ( *DrawSrcPoint << 20 ) ;
													}
													else
													{
														*( ( DWORD * )DrawDstPoint ) = 0 ;
													}

													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_NORMAL :
									{
										WORD SizeXTemp ;
										WORD Data ;

										if( SrcRect.left % 8 == 0 )
										{
											DrawSrcPoint = ManageData->CacheMem ;
											DrawSrcPoint += ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
											SrcPitch2 = SrcPitch - ( ( SizeX / 16 ) * 2 + ( SizeX % 16 != 0 ? 2 : 0 ) ) ;
											DstPitch2 = DstPitch - ( ( ( SizeX >> 4 ) << 5 ) ) ;

											if( TransFlag )
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															if( Data & 0x80 ) *( ( WORD * )DrawDstPoint ) = ColorData ;			if( --SizeXTemp == 0 ) break ;
															if( Data & 0x40 ) *( ( WORD * )DrawDstPoint + 1 ) = ColorData ; 	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x20 ) *( ( WORD * )DrawDstPoint + 2 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x10 ) *( ( WORD * )DrawDstPoint + 3 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x8 ) *( ( WORD * )DrawDstPoint + 4 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x4 ) *( ( WORD * )DrawDstPoint + 5 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x2 ) *( ( WORD * )DrawDstPoint + 6 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x1 ) *( ( WORD * )DrawDstPoint + 7 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x8000 ) *( ( WORD * )DrawDstPoint + 8 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x4000 ) *( ( WORD * )DrawDstPoint + 9 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x2000 ) *( ( WORD * )DrawDstPoint + 10 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x1000 ) *( ( WORD * )DrawDstPoint + 11 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x800 ) *( ( WORD * )DrawDstPoint + 12 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x400 ) *( ( WORD * )DrawDstPoint + 13 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x200 ) *( ( WORD * )DrawDstPoint + 14 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x100 ) *( ( WORD * )DrawDstPoint + 15 ) = ColorData ;
															DrawDstPoint += 32 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 2 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
											else
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															*( ( WORD * )DrawDstPoint ) = ( Data & 0x80 ) ? ColorData : 0 ;			if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 1 ) = ( Data & 0x40 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 2 ) = ( Data & 0x20 ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 3 ) = ( Data & 0x10 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 4 ) = ( Data & 0x8 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 5 ) = ( Data & 0x4 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 6 ) = ( Data & 0x2 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 7 ) = ( Data & 0x1 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 8 ) = ( Data & 0x8000 ) ? ColorData : 0 ;	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 9 ) = ( Data & 0x4000 ) ? ColorData : 0 ;	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 10 ) = ( Data & 0x2000 ) ? ColorData : 0 ;	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 11 ) = ( Data & 0x1000 ) ? ColorData : 0 ;	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 12 ) = ( Data & 0x800 ) ? ColorData : 0 ;	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 13 ) = ( Data & 0x400 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 14 ) = ( Data & 0x200 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 15 ) = ( Data & 0x100 ) ? ColorData : 0 ;  
															DrawDstPoint += 32 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 2 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
										}
										else
										{
											int StX = SrcRect.left % 8 ;
											WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

											DrawSrcPoint = ManageData->CacheMem ;
											DrawSrcPoint += ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
											SrcPitch2 = SrcPitch - ( ( SizeX / 8 ) + ( SizeX % 8 != 0 ? 1 : 0 ) ) ;
											DstPitch2 = DstPitch - ( ( ( SizeX >> 3 ) << 4 ) ) /*- ( SizeX % 4 == 0 ? 8 : 0 )*/ ;

											if( TransFlag )
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															if( Data & Table[StX] ) *( ( WORD * )DrawDstPoint ) = ColorData ; 			if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+1] ) *( ( WORD * )DrawDstPoint + 1 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+2] ) *( ( WORD * )DrawDstPoint + 2 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+3] ) *( ( WORD * )DrawDstPoint + 3 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+4] ) *( ( WORD * )DrawDstPoint + 4 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+5] ) *( ( WORD * )DrawDstPoint + 5 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+6] ) *( ( WORD * )DrawDstPoint + 6 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+7] ) *( ( WORD * )DrawDstPoint + 7 ) = ColorData ;
 															--SizeXTemp ;
															DrawDstPoint += 16 ;
														}while(0);

														DrawSrcPoint += 1 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
											else
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															*( ( WORD * )DrawDstPoint ) = ( Data & Table[StX] ) ? ColorData : 0 ;   		if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 1 ) = ( Data & Table[StX+1] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 2 ) = ( Data & Table[StX+2] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 3 ) = ( Data & Table[StX+3] ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 4 ) = ( Data & Table[StX+4] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 5 ) = ( Data & Table[StX+5] ) ? ColorData : 0 ;		if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 6 ) = ( Data & Table[StX+6] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( WORD * )DrawDstPoint + 7 ) = ( Data & Table[StX+7] ) ? ColorData : 0 ;  
 															--SizeXTemp ;
															DrawDstPoint += 16 ;
														}while(0);

														DrawSrcPoint += 1 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
										}
									}
									break ;

								case DX_FONTTYPE_EDGE :
									{
										WORD Width ;
										WORD EColor ;

										EColor = ( WORD )( EdgeColor & 0xffff ) ;
										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint )
													{
														*( ( WORD * )DrawDstPoint ) = *DrawSrcPoint == 1 ? ColorData : EColor ;
													}
													DrawDstPoint += 2 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													*( ( WORD * )DrawDstPoint ) = *DrawSrcPoint ? ( *DrawSrcPoint == 1 ? ColorData : EColor ) : 0 ;
													DrawDstPoint += 2 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;
								}
							}
							break ;

						case 32 :
							// 32ビットモードの場合
							{
								BYTE *DrawDstPoint = DestMemImg->UseImage ;
								BYTE *DrawSrcPoint = ManageData->CacheMem ;
								DWORD ColorData = ( DWORD )Color ;
   
								DrawDstPoint += ( DstRect.left << 2 ) + DstRect.top * DstPitch ;
								DrawSrcPoint += ( SrcRect.left ) + SrcRect.top * SrcPitch ;
								SrcPitch2 = SrcPitch - SizeX ;
								DstPitch2 = DstPitch - ( SizeX << 2 ) ;

								switch( ManageData->FontType )
								{
								case DX_FONTTYPE_ANTIALIASING_EDGE :
								case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
								case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
									{
										BYTE Red1, Green1, Blue1 ;
										BYTE Red2, Green2, Blue2 ;
										DWORD AAA ;

										WORD Width ;

										Red1   = ( BYTE )( ( ColorData & MEMIMG_XRGB8_R ) >> MEMIMG_XRGB8_LR ) ;
										Green1 = ( BYTE )( ( ColorData & MEMIMG_XRGB8_G ) >> MEMIMG_XRGB8_LG ) ;
										Blue1  = ( BYTE )( ( ColorData & MEMIMG_XRGB8_B ) >> MEMIMG_XRGB8_LB ) ;

										Red2   = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_R ) >> MEMIMG_XRGB8_LR ) ;
										Green2 = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_G ) >> MEMIMG_XRGB8_LG ) ;
										Blue2  = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_B ) >> MEMIMG_XRGB8_LB ) ;

										Red1 = ( BYTE )( ( ColorData & MEMIMG_XRGB8_R ) >> MEMIMG_XRGB8_LR );
										Green1 = ( BYTE )( ( ColorData & MEMIMG_XRGB8_G ) >> MEMIMG_XRGB8_LG );
										Blue1 = ( BYTE )( ( ColorData & MEMIMG_XRGB8_B ) >> MEMIMG_XRGB8_LB );

										Red2 = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_R ) >> MEMIMG_XRGB8_LR );
										Green2 = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_G ) >> MEMIMG_XRGB8_LG );
										Blue2 = ( BYTE )( ( EdgeColor & MEMIMG_XRGB8_B ) >> MEMIMG_XRGB8_LB );

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint & 0x0f )
													{
														if( ( *DrawSrcPoint & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )DrawDstPoint ) = 0xff000000 | ColorData ;
														}
														else
														{
															*( ( DWORD * )DrawDstPoint ) = ( ( ( *DrawSrcPoint & 0x0f ) << 4 ) + ( ( 0x0f - ( *DrawSrcPoint & 0x0f ) ) * ( ( *DrawSrcPoint & 0xf0 ) >> 4 ) ) ) << 24 ;
															AAA = ( ( WORD )( *DrawSrcPoint & 0x0f ) << 12 ) / DrawDstPoint[3] ;
															*( ( DWORD * )DrawDstPoint ) |= ( DWORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_XRGB8_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_XRGB8_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_XRGB8_LB ) ) ; 
														}
													}
													else
													{
														if( *DrawSrcPoint & 0xf0 )
														{
															*( ( DWORD * )DrawDstPoint ) = EdgeColor | ( ( *DrawSrcPoint & 0xf0 ) << 24 ) ;
														}
													}

													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint & 0x0f )
													{
														if( ( *DrawSrcPoint & 0x0f ) == 0x0f )
														{
															*( ( DWORD * )DrawDstPoint ) = 0xff000000 | ColorData ;
														}
														else
														{
															*( ( DWORD * )DrawDstPoint ) = ( ( ( *DrawSrcPoint & 0x0f ) << 4 ) + ( ( 0x0f - ( *DrawSrcPoint & 0x0f ) ) * ( ( *DrawSrcPoint & 0xf0 ) >> 4 ) ) ) << 24 ;
															AAA = ( ( WORD )( *DrawSrcPoint & 0x0f ) << 12 ) / DrawDstPoint[3] ;
															*( ( DWORD * )DrawDstPoint ) |= ( DWORD )( 
																( ( ( ( ( Red1   - Red2   ) * AAA ) >> 8 ) + Red2   ) << MEMIMG_XRGB8_LR ) | 
																( ( ( ( ( Green1 - Green2 ) * AAA ) >> 8 ) + Green2 ) << MEMIMG_XRGB8_LG ) | 
																( ( ( ( ( Blue1  - Blue2  ) * AAA ) >> 8 ) + Blue2  ) << MEMIMG_XRGB8_LB ) ) ; 
														}
													}
													else
													{
														if( *DrawSrcPoint & 0xf0 )
														{
															*( ( DWORD * )DrawDstPoint ) = EdgeColor | ( ( *DrawSrcPoint & 0xf0 ) << 24 ) ;
														}
														else
														{
															*( ( DWORD * )DrawDstPoint ) = 0 ;
														}
													}

													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;


								case DX_FONTTYPE_ANTIALIASING :
								case DX_FONTTYPE_ANTIALIASING_4X4 :
								case DX_FONTTYPE_ANTIALIASING_8X8 :
									{
										WORD Width ;

										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint )
													{
														*( ( DWORD * )DrawDstPoint ) = ColorData | ( ( *DrawSrcPoint & 0x0f ) << 28 ) ;
													}
													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint )
													{
														*( ( DWORD * )DrawDstPoint ) = ColorData | ( ( *DrawSrcPoint & 0x0f ) << 28 ) ;
													}
													else
													{
														*( ( DWORD * )DrawDstPoint ) = 0 ;
													}

													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;

								case DX_FONTTYPE_NORMAL :
									{
										WORD SizeXTemp ;
										WORD Data ;

										if( SrcRect.left % 8 == 0 )
										{
											DrawSrcPoint = ManageData->CacheMem ;
											DrawSrcPoint += ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
											SrcPitch2 = SrcPitch - ( ( SizeX / 16 ) * 2 + ( SizeX % 16 != 0 ? 2 : 0 ) ) ;

											DstPitch2 = DstPitch - ( ( ( SizeX >> 4 ) << 6 ) ) ;

											if( TransFlag )
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															if( Data & 0x80 ) *( ( DWORD * )DrawDstPoint ) = ColorData ; 		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x40 ) *( ( DWORD * )DrawDstPoint + 1 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x20 ) *( ( DWORD * )DrawDstPoint + 2 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x10 ) *( ( DWORD * )DrawDstPoint + 3 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x8 ) *( ( DWORD * )DrawDstPoint + 4 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x4 ) *( ( DWORD * )DrawDstPoint + 5 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x2 ) *( ( DWORD * )DrawDstPoint + 6 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x1 ) *( ( DWORD * )DrawDstPoint + 7 ) = ColorData ;		if( --SizeXTemp == 0 ) break ;
															if( Data & 0x8000 ) *( ( DWORD * )DrawDstPoint + 8 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x4000 ) *( ( DWORD * )DrawDstPoint + 9 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x2000 ) *( ( DWORD * )DrawDstPoint + 10 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x1000 ) *( ( DWORD * )DrawDstPoint + 11 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x800 ) *( ( DWORD * )DrawDstPoint + 12 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x400 ) *( ( DWORD * )DrawDstPoint + 13 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x200 ) *( ( DWORD * )DrawDstPoint + 14 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & 0x100 ) *( ( DWORD * )DrawDstPoint + 15 ) = ColorData ;
															DrawDstPoint += 64 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 2 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
											else
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															*( ( DWORD * )DrawDstPoint ) = ( Data & 0x80 ) ? ColorData : 0 ;  		if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 1 ) = ( Data & 0x40 ) ? ColorData : 0 ;	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 2 ) = ( Data & 0x20 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 3 ) = ( Data & 0x10 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 4 ) = ( Data & 0x8 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 5 ) = ( Data & 0x4 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 6 ) = ( Data & 0x2 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 7 ) = ( Data & 0x1 ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 8 ) = ( Data & 0x8000 ) ? ColorData : 0 ;  if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 9 ) = ( Data & 0x4000 ) ? ColorData : 0 ;  if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 10 ) = ( Data & 0x2000 ) ? ColorData : 0 ; if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 11 ) = ( Data & 0x1000 ) ? ColorData : 0 ; if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 12 ) = ( Data & 0x800 ) ? ColorData : 0 ;  if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 13 ) = ( Data & 0x400 ) ? ColorData : 0 ;  if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 14 ) = ( Data & 0x200 ) ? ColorData : 0 ;  if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 15 ) = ( Data & 0x100 ) ? ColorData : 0 ;  
															DrawDstPoint += 64 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 2 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
										}
										else
										{
											int StX = SrcRect.left % 8 ;
											WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

											DrawSrcPoint = ManageData->CacheMem ;
											DrawSrcPoint += ( SrcRect.left / 8 ) + SrcRect.top * SrcPitch ;
											SrcPitch2 = SrcPitch - ( ( SizeX / 8 ) + ( SizeX % 8 != 0 ? 1 : 0 ) ) ;

											DstPitch2 = DstPitch - ( ( ( SizeX >> 3 ) << 5 ) ) ;

											if( TransFlag )
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															if( Data & Table[StX] ) *( ( DWORD * )DrawDstPoint ) = ColorData ;			if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+1] ) *( ( DWORD * )DrawDstPoint + 1 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+2] ) *( ( DWORD * )DrawDstPoint + 2 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+3] ) *( ( DWORD * )DrawDstPoint + 3 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+4] ) *( ( DWORD * )DrawDstPoint + 4 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+5] ) *( ( DWORD * )DrawDstPoint + 5 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+6] ) *( ( DWORD * )DrawDstPoint + 6 ) = ColorData ;	if( --SizeXTemp == 0 ) break ;
															if( Data & Table[StX+7] ) *( ( DWORD * )DrawDstPoint + 7 ) = ColorData ;
															DrawDstPoint += 32 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 1 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
											else
											{
												do{
													SizeXTemp = SizeXWord ;
													do{
														Data = *( ( WORD * )DrawSrcPoint ) ;
														do{
															*( ( DWORD * )DrawDstPoint ) = ( Data & Table[StX] ) ? ColorData : 0 ;   		if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 1 ) = ( Data & Table[StX+1] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 2 ) = ( Data & Table[StX+2] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 3 ) = ( Data & Table[StX+3] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 4 ) = ( Data & Table[StX+4] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 5 ) = ( Data & Table[StX+5] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 6 ) = ( Data & Table[StX+6] ) ? ColorData : 0 ;  	if( --SizeXTemp == 0 ) break ;
															*( ( DWORD * )DrawDstPoint + 7 ) = ( Data & Table[StX+7] ) ? ColorData : 0 ;  
															DrawDstPoint += 32 ;
 															--SizeXTemp ;
														}while(0);

														DrawSrcPoint += 1 ;
													}while( SizeXTemp );

													DrawSrcPoint += SrcPitch2 ;
													DrawDstPoint += DstPitch2 ;
												}while( --SizeYWord ) ;
											}
										}
									}
									break ;

								case DX_FONTTYPE_EDGE :
									{
										WORD Width ;
										DWORD EColor ;

										EColor = ( DWORD )( EdgeColor & 0xffffff ) ;
										if( TransFlag )
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													if( *DrawSrcPoint )
													{
														*( ( DWORD * )DrawDstPoint ) = *DrawSrcPoint == 1 ? ColorData : EColor ;
													}
													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
										else
										{
											do
											{
												Width = SizeXWord ;
												do
												{
													*( ( DWORD * )DrawDstPoint ) = *DrawSrcPoint ? ( *DrawSrcPoint == 1 ? ColorData : EColor ) : 0 ;
													DrawDstPoint += 4 ;
													DrawSrcPoint ++ ;
												}while( --Width ) ;

												DrawDstPoint += DstPitch2 ;
												DrawSrcPoint += SrcPitch2 ;
											}while( --SizeYWord ) ;
										}
									}
									break ;
								}
							}
							break ;
						}
					}
				}
			}
		}
	}
	else
	{
		// 文字の数だけ繰り返し
		StrPoint = UseDrawStrBuf ;			// 描画用文字データの先頭アドレスをセット
		DrawPos = 0 ;
		for( i = 0 ; i < DrawCharNum ; i ++, DrawPos += ( FontData->AddX + ManageData->Space ) * ExRate, StrPoint += 3 )
		{
			// キャッシュの中に希望の文字があるか検査
			if( FontCodeData[ *( (WORD *)StrPoint ) ].ExistFlag )
			{
				// あった場合そのフォントデータのポインタを保存
				FontData = FontCodeData[ *( (WORD *)StrPoint ) ].DataPoint ; 
			}
			else
			{
				// 無かった場合キャッシュに追加

				// 新たにキャッシュ文字を作成
				FontData = FontCacheCharAddToHandle( 1, StrPoint, FontHandle ) ;
				if( FontData == NULL )
				{
					DXST_ERRORLOG_ADD( _T( "テキストキャッシュの作成に失敗しました" ) ) ;
					goto ERR ;
				}
			}
		}
	}

	if( DrawSize != NULL )
	{
		if( VerticalFlag == TRUE ) 
		{
			DrawSize->cx = ManageData->FontHeight ;
			DrawSize->cy = _DTOL( DrawPos ) ;
		}
		else
		{
			DrawSize->cx = _DTOL( DrawPos ) ;
			DrawSize->cy = ManageData->FontHeight ;
		}
	}

	if( UseAlloc )
	{
		DXFREE( AllocDrawStrBuf ) ;
	}

	// 終了
	return _DTOL( DrawPos ) ;

ERR:
	if( UseAlloc )
	{
		DXFREE( AllocDrawStrBuf ) ;
	}
	return -1 ;
}

extern int NS_FontCacheStringDrawToHandle( int x, int y, const TCHAR *StrData, int Color, int EdgeColor,
											BASEIMAGE *DestImage, const RECT *ClipRect, int FontHandle,
											int VerticalFlag, SIZE *DrawSizeP )
{
	static TCHAR DrawStrBuf[ 256 * 3 ] ;		// 描画する際に使用する文字列データ（３バイトで１文字を表現）
	FONTMANAGE * cmanage ;
	FONTDATA * cache ;
	FONTCODEDATA *fontcodedata ;
	RECT SrcRect ;
	RECT DstRect ;
	int drawnum ;							// ２バイト文字１バイト文字全部ひっくるめて何文字あるか保存する変数
	int i ;									// 繰り返しと汎用変数
	const TCHAR *strp ;
	LONG dpitch, spitch ;					// 描画先と描画もとのサーフェスの１ラインのバイト数
	int TotalWidth, TotalHeight ;
	int r, g, b, er, eg, eb ;
//	bool Hangeul ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, cmanage ) )
		return -1 ;

	// テクスチャをキャッシュに使用しているフォントは無理
	if( cmanage->TextureCacheFlag == TRUE ) return -1 ;

	// 描画座標の補正
	if( VerticalFlag )
	{
		x -= cmanage->FontAddHeight / 2 ;
	}
	else
	{
		y -= cmanage->FontAddHeight / 2 ;
	}

	// 色カラー値を取得しておく
	NS_GetColor5( &DestImage->ColorData, Color, &r, &g, &b ) ;
	NS_GetColor5( &DestImage->ColorData, EdgeColor, &er, &eg, &eb ) ;

	// ＵＮＩコードかどうかを取得しておく
//	Hangeul = cmanage->CharSet == DX_CHARSET_HANGEUL ;

	// 文字列がない場合は終了
	if( StrData  == NULL || *StrData  == 0 ) return 0 ;

	// キャッシュに入っていない文字列をキャッシュに入れる
	FontCacheStringAddToHandle( FontHandle, StrData, -1, DrawStrBuf, &drawnum ) ;

	// ピッチをセット
	spitch = cmanage->CachePitch ;
	dpitch = DestImage->Pitch ;

	// 文字の数だけ繰り返し
	strp = DrawStrBuf ;			// 描画用文字データの先頭アドレスをセット
	TotalWidth = 0 ;
	TotalHeight = 0 ;
	fontcodedata = cmanage->FontCodeData ;
	for( i = 0 ; i < drawnum ; i ++,
//			TotalHeight = VerticalFlag ? TotalHeight + cache->Size.cy : ( TotalHeight < cache->Size.cy ? cache->Size.cy : TotalHeight ),
//			TotalWidth = VerticalFlag ? ( TotalWidth < cache->Size.cx ? cache->Size.cx : TotalWidth ) : TotalWidth + cache->Size.cx ,
			TotalHeight = VerticalFlag ? TotalHeight + cache->AddX + cmanage->Space : ( TotalHeight < cache->DrawY + cache->SizeY ? cache->DrawY + cache->SizeY : TotalHeight ),
			TotalWidth  = VerticalFlag ? ( TotalWidth < cache->DrawX + cache->SizeX ? cache->DrawX + cache->SizeX : TotalWidth ) : TotalWidth + cache->AddX + cmanage->Space,
			y += VerticalFlag ? cache->AddX + cmanage->Space : 0, x += VerticalFlag ? 0 : cache->AddX + cmanage->Space, strp += 3 )
	{
		// キャッシュの中に希望の文字があるか検査
		if( fontcodedata[ *(( WORD *)strp) ].ExistFlag )
		{
			// あった場合そのフォントデータのポインタを保存
			cache = fontcodedata[ *(( WORD *)strp) ].DataPoint ; 
		}
		else
		{
			// 無かった場合キャッシュに追加
			cache = FontCacheCharAddToHandle( 1, strp, FontHandle ) ;
			if( cache == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "テキストキャッシュの追加に失敗しました" ) ) ;
				return -1 ;
			}
		}

		// スペース文字だった場合はキャンセルする
#ifdef UNICODE
		if( StrData == NULL && ( *( ( wchar_t * )strp ) == L' ' || *( ( wchar_t * )strp ) == L'　'            ) ) continue ;
#else
		if( StrData != NULL && ( *strp                  ==  ' ' || *(( WORD *)strp )      == *(( WORD *)"　") ) ) continue ;
#endif

		// サイズが無かったら何もしない
		if( cache->SizeX == 0 && cache->SizeY == 0 ) continue ;

		// 転送先の描画領域を作成
		DstRect.left	= x + cache->DrawX ;
		DstRect.top		= y + cache->DrawY ;
		DstRect.right	= x + cache->DrawX + cache->SizeX ;
		DstRect.bottom	= y + cache->DrawY + cache->SizeY ;
		if( ClipRect == NULL )
		{
			RECT BaseRect ;
			BaseRect.left = 0 ;
			BaseRect.top = 0 ;
			BaseRect.right = DestImage->Width ;
			BaseRect.bottom = DestImage->Height ;
			RectClipping( &DstRect, &BaseRect ) ;
		}
		else
		{
			RectClipping( &DstRect, ClipRect ) ;
		}

		// 描画する文字が無かった場合ここで次ループに移る
		if( DstRect.right == DstRect.left || DstRect.bottom == DstRect.top ) continue ;

		// 転送元の矩形をセット
		SrcRect.left	=   cache->GraphIndex / cmanage->LengthCharNum   * cmanage->MaxWidth ;
		SrcRect.top		= ( cache->GraphIndex % cmanage->LengthCharNum ) * cmanage->MaxWidth ;
		if( cmanage->TextureCacheFlag == TRUE )
		{
			SrcRect.left ++ ;
			SrcRect.top ++ ;
		}
		SrcRect.right	= SrcRect.left + cache->SizeX ;
		SrcRect.bottom	= SrcRect.top + cache->SizeY ;

		// 転送元の矩形を補正
		SrcRect.left   += DstRect.left - ( x + cache->DrawX ) ;
		SrcRect.top    += DstRect.top  - ( y + cache->DrawY ) ;
		SrcRect.right  -= ( ( x + cache->DrawX ) + cache->SizeX ) - DstRect.right ;
		SrcRect.bottom -= ( ( y + cache->DrawY ) + cache->SizeY ) - DstRect.bottom ;

		// 传送
		{
			DWORD spitch2, dpitch2 ;
			DWORD SizeX, SizeY ;
			WORD SizeXWord, SizeYWord ;

			SizeX = SrcRect.right - SrcRect.left ;
			SizeY = SrcRect.bottom - SrcRect.top ;
			SizeXWord = ( WORD )SizeX ;
			SizeYWord = ( WORD )SizeY ;

			// 色のビット深度によって処理を分岐
			switch( DestImage->ColorData.ColorBitDepth )
			{
			case 8 :
				// 8ビットモードの時の処理
				{
					BYTE *dest = ( BYTE * )DestImage->GraphData ;
					BYTE *src = cmanage->CacheMem ;
					BYTE cr = ( BYTE )( Color & 0xff ) ;

					dest += ( DstRect.left ) + DstRect.top * dpitch ;
					src += ( SrcRect.left ) + SrcRect.top * spitch ;

					spitch2 = spitch - SizeX ;
					dpitch2 = dpitch - SizeX ;

					switch( cmanage->FontType )
					{
					case DX_FONTTYPE_NORMAL :
						{
							DWORD wtemp ;
							WORD dat ;
							DWORD WordNum, NokoriNum ;

							if( SrcRect.left % 8 == 0 )
							{
								src = cmanage->CacheMem ;
								src += ( SrcRect.left / 8 ) + SrcRect.top * spitch ;
								spitch2 = spitch - ( ( SizeX / 16 ) * 2 + ( SizeX % 16 != 0 ? 2 : 0 ) ) ;
								dpitch2 = dpitch - ( ( ( SizeX >> 4 ) << 4 ) ) ;
								WordNum = SizeXWord / 16 ;
								NokoriNum = SizeXWord - WordNum * 16 ;

								do
								{
									if( WordNum != 0 )
									{
										wtemp = WordNum ;
										do
										{
											dat = *(( WORD *)src) ;
											if( dat & 0x80 ) dest[0] = cr ;
											if( dat & 0x40 ) dest[1] = cr ;
											if( dat & 0x20 ) dest[2] = cr ;
											if( dat & 0x10 ) dest[3] = cr ;
											if( dat & 0x8 ) dest[4] = cr ;
											if( dat & 0x4 ) dest[5] = cr ;
											if( dat & 0x2 ) dest[6] = cr ;
											if( dat & 0x1 ) dest[7] = cr ;
											if( dat & 0x8000 ) dest[8] = cr ;
											if( dat & 0x4000 ) dest[9] = cr ;
											if( dat & 0x2000 ) dest[10] = cr ;
											if( dat & 0x1000 ) dest[11] = cr ;
											if( dat & 0x800 ) dest[12] = cr ;
											if( dat & 0x400 ) dest[13] = cr ;
											if( dat & 0x200 ) dest[14] = cr ;
											if( dat & 0x100 ) dest[15] = cr ;
											dest += 16 ;
											src += 2 ;
										}while( --wtemp ) ;
									}

									if( NokoriNum != 0 )
									{
										wtemp = NokoriNum ;
										do
										{
											dat = *(( WORD *)src ) ;
											do{
												if( dat & 0x80 ) dest[0] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x40 ) dest[1] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x20 ) dest[2] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x10 ) dest[3] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x8 ) dest[4] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x4 ) dest[5] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x2 ) dest[6] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x1 ) dest[7] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x8000 ) dest[8] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x4000 ) dest[9] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x2000 ) dest[10] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x1000 ) dest[11] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x800 ) dest[12] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x400 ) dest[13] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x200 ) dest[14] = cr ;
 												if( --wtemp == 0 ) break ;
												if( dat & 0x100 ) dest[15] = cr ;
												dest += 16 ;
 												--wtemp ;
											}while(0);

											src += 2 ;
										}while( wtemp );
									}

									src += spitch2 ;
									dest += dpitch2 ;
								}while( --SizeYWord ) ;
/*
								do{
									wtemp = SizeXWord ;
									do{
										dat = *(( WORD *)src) ;
										do{
											if( dat & 0x80 ) dest[0] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x40 ) dest[1] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x20 ) dest[2] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x10 ) dest[3] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x8 ) dest[4] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x4 ) dest[5] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x2 ) dest[6] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x1 ) dest[7] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x8000 ) dest[8] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x4000 ) dest[9] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x2000 ) dest[10] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x1000 ) dest[11] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x800 ) dest[12] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x400 ) dest[13] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x200 ) dest[14] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x100 ) dest[15] = cr ;
											dest += 16 ;
 											--wtemp ;
										}while(0);

										src += 2 ;
									}while( wtemp );

									src += spitch2 ;
									dest += dpitch2 ;
								}while( --SizeYWord ) ;
*/							}
							else
							{
								int StX = SrcRect.left % 8 ;
								WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

								src = cmanage->CacheMem ;
								src += ( SrcRect.left / 8 ) + SrcRect.top * spitch ;
								spitch2 = spitch - ( ( SizeX / 8 ) + ( SizeX % 8 != 0 ? 1 : 0 ) ) ;
								dpitch2 = dpitch - ( ( ( SizeX >> 3 ) << 3 ) ) ;

								do{
									wtemp = SizeXWord ;
									do{
										dat = *(( WORD *)src) ;
										do{
											if( dat & Table[StX] ) dest[0] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+1] ) dest[1] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+2] ) dest[2] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+3] ) dest[3] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+4] ) dest[4] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+5] ) dest[5] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+6] ) dest[6] = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+7] ) dest[7] = cr ;
											dest += 8 ;
 											--wtemp ;
										}while(0);

										src += 1 ;
									}while( wtemp );

									src += spitch2 ;
									dest += dpitch2 ;
								}while( --SizeYWord ) ;
							}
						}
						break ;


					case DX_FONTTYPE_EDGE :
						{
							WORD wtemp ;
							BYTE ecr ;

							ecr = (BYTE)( EdgeColor & 0xff ) ;
							do
							{
								wtemp = SizeXWord ;
								do
								{
									if( *src ) *dest = *src == 1 ? cr : ecr ;

									dest ++ ;
									src ++ ;
								}while( --wtemp ) ;

								dest += dpitch2 ;
								src += spitch2 ;
							}while( --SizeYWord ) ;
						}
						break ;
					}
				}
				break ;

			case 16 :
				// 16ビットモードの時の処理
				{
					BYTE *dest = ( BYTE * )DestImage->GraphData ;
					BYTE *src = cmanage->CacheMem ;
					WORD cr = (WORD)( Color & 0xffff ), ecr = (WORD)( EdgeColor & 0xffff );

					dest += ( DstRect.left << 1 ) + DstRect.top * dpitch ;
					src += SrcRect.left + SrcRect.top * spitch ;

					spitch2 = spitch - SizeX ;
					dpitch2 = dpitch - ( SizeX << 1 ) ;

					switch( cmanage->FontType )
					{
					case DX_FONTTYPE_ANTIALIASING_EDGE :
					case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
					case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
						{
							BYTE r1, g1, b1 ;
							BYTE r2, g2, b2 ;
							BYTE r3, g3, b3 ;
							DWORD AAA, AAA2 ;
							WORD wtemp ;

							src = cmanage->CacheMem + ( SrcRect.left << 1 ) + SrcRect.top * spitch ;
							spitch2 = spitch - ( SizeX << 1 ) ;

							if( ( DestImage->ColorData.RedMask & MEMIMG_R5G6B5_R ) &&
								( DestImage->ColorData.GreenMask & MEMIMG_R5G6B5_G ) &&
								( DestImage->ColorData.BlueMask & MEMIMG_R5G6B5_B ) )
							{
								r1 = ( BYTE )( r >> 3 ) ;
								g1 = ( BYTE )( g >> 2 ) ;
								b1 = ( BYTE )( b >> 3 ) ;

								r2 = ( BYTE )( er >> 3 ) ;
								g2 = ( BYTE )( eg >> 2 ) ;
								b2 = ( BYTE )( eb >> 3 ) ;

								do
								{
									wtemp = SizeXWord ;
									do
									{
										if( src[0] )
										{
											if( src[0] == 255 )
											{
												*((WORD *)dest) = cr ;
											}
											else
											{
												r3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_R) >> MEMIMG_R5G6B5_LR );
												g3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_G) >> MEMIMG_R5G6B5_LG );
												b3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_B) >> MEMIMG_R5G6B5_LB );

												AAA = ((WORD)src[0] << 8) / (AAA2 = src[0] + (((256 - src[0]) * src[1]) >> 8))  ;
												*((WORD *)dest) = ( WORD )( 
													(((((((((r1 - r2) * AAA) >> 8) + r2) - r3) * AAA2) >> 8) + r3) << MEMIMG_R5G6B5_LR ) | 
													(((((((((g1 - g2) * AAA) >> 8) + g2) - g3) * AAA2) >> 8) + g3) << MEMIMG_R5G6B5_LG ) | 
													(((((((((b1 - b2) * AAA) >> 8) + b2) - b3) * AAA2) >> 8) + b3) << MEMIMG_R5G6B5_LB ) ) ;
											}
										}
										else
										if( src[1] )
										{
											if( src[1] == 255 )
											{
												*((WORD *)dest) = ecr ;
											}
											else
											{
												r3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_R) >> MEMIMG_R5G6B5_LR );
												g3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_G) >> MEMIMG_R5G6B5_LG );
												b3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_B) >> MEMIMG_R5G6B5_LB );

												*((WORD *)dest) = ( WORD )( 
													(((((r2 - r3) * src[1]) >> 8) + r3) << MEMIMG_R5G6B5_LR ) | 
													(((((g2 - g3) * src[1]) >> 8) + g3) << MEMIMG_R5G6B5_LG ) | 
													(((((b2 - b3) * src[1]) >> 8) + b3) << MEMIMG_R5G6B5_LB ) ) ;
											}
										}

										dest += 2 ;
										src += 2 ;
									}while( --wtemp ) ;

									dest += dpitch2 ;
									src += spitch2 ;
								}while( --SizeYWord ) ;
							}
							else
							if( ( DestImage->ColorData.RedMask & MEMIMG_RGB5_R ) &&
								( DestImage->ColorData.GreenMask & MEMIMG_RGB5_G ) &&
								( DestImage->ColorData.BlueMask & MEMIMG_RGB5_B ) )
							{
								r1 = ( BYTE )( r >> 3 ) ;
								g1 = ( BYTE )( g >> 3 ) ;
								b1 = ( BYTE )( b >> 3 ) ;

								r2 = ( BYTE )( er >> 3 ) ;
								g2 = ( BYTE )( eg >> 3 ) ;
								b2 = ( BYTE )( eb >> 3 ) ;

								do
								{
									wtemp = SizeXWord ;
									do
									{
										if( src[0] )
										{
											if( src[0] == 255 )
											{
												*((WORD *)dest) = cr ;
											}
											else
											{
												r3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_R) >> MEMIMG_RGB5_LR );
												g3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_G) >> MEMIMG_RGB5_LG );
												b3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_B) >> MEMIMG_RGB5_LB );

												AAA = ((WORD)src[0] << 8) / (AAA2 = src[0] + (((256 - src[0]) * src[1]) >> 8))  ;
												*((WORD *)dest) = ( WORD )( 
													(((((((((r1 - r2) * AAA) >> 8) + r2) - r3) * AAA2) >> 8) + r3) << MEMIMG_RGB5_LR ) | 
													(((((((((g1 - g2) * AAA) >> 8) + g2) - g3) * AAA2) >> 8) + g3) << MEMIMG_RGB5_LG ) | 
													(((((((((b1 - b2) * AAA) >> 8) + b2) - b3) * AAA2) >> 8) + b3) << MEMIMG_RGB5_LB ) ) ;
											}
										}
										else
										if( src[1] )
										{
											if( src[1] == 255 )
											{
												*((WORD *)dest) = ecr ;
											}
											else
											{
												r3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_R) >> MEMIMG_RGB5_LR );
												g3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_G) >> MEMIMG_RGB5_LG );
												b3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_B) >> MEMIMG_RGB5_LB );

												*((WORD *)dest) = ( WORD )( 
													(((((r2 - r3) * src[1]) >> 8) + r3) << MEMIMG_RGB5_LR ) | 
													(((((g2 - g3) * src[1]) >> 8) + g3) << MEMIMG_RGB5_LG ) | 
													(((((b2 - b3) * src[1]) >> 8) + b3) << MEMIMG_RGB5_LB ) ) ;
											}
										}

										dest += 2 ;
										src += 2 ;
									}while( --wtemp ) ;

									dest += dpitch2 ;
									src += spitch2 ;
								}while( --SizeYWord ) ;
							}
						}
						break ;

					case DX_FONTTYPE_ANTIALIASING :
					case DX_FONTTYPE_ANTIALIASING_4X4 :
					case DX_FONTTYPE_ANTIALIASING_8X8 :
						{
							WORD wtemp ;
							BYTE r1, g1, b1 ;
							BYTE r3, g3, b3 ;

							src = cmanage->CacheMem + ( SrcRect.left << 1 ) + SrcRect.top * spitch ;
							spitch2 = spitch - ( SizeX << 1 ) ;

							if( ( DestImage->ColorData.RedMask & MEMIMG_R5G6B5_R ) &&
								( DestImage->ColorData.GreenMask & MEMIMG_R5G6B5_G ) &&
								( DestImage->ColorData.BlueMask & MEMIMG_R5G6B5_B ) )
							{
								r1 = ( BYTE )( r >> 3 ) ;
								g1 = ( BYTE )( g >> 2 ) ;
								b1 = ( BYTE )( b >> 3 ) ;

								do
								{
									wtemp = SizeXWord ;
									do
									{
										if( src[0] )
										{
											if( src[0] == 255 )
											{
												*((WORD *)dest) = cr ;
											}
											else
											{
												r3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_R) >> MEMIMG_R5G6B5_LR );
												g3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_G) >> MEMIMG_R5G6B5_LG );
												b3 = (BYTE)( (*((WORD *)dest) & MEMIMG_R5G6B5_B) >> MEMIMG_R5G6B5_LB );

												*((WORD *)dest) = ( WORD )( 
													(((((r1 - r3) * src[0]) >> 8) + r3) << MEMIMG_R5G6B5_LR ) | 
													(((((g1 - g3) * src[0]) >> 8) + g3) << MEMIMG_R5G6B5_LG ) | 
													(((((b1 - b3) * src[0]) >> 8) + b3) << MEMIMG_R5G6B5_LB ) ) ;
											}
										}
										dest += 2 ;
										src ++ ;
									}while( --wtemp ) ;

									dest += dpitch2 ;
									src += spitch2 ;
								}while( --SizeYWord ) ;
							}
							else
							if( ( DestImage->ColorData.RedMask & MEMIMG_RGB5_R ) &&
								( DestImage->ColorData.GreenMask & MEMIMG_RGB5_G ) &&
								( DestImage->ColorData.BlueMask & MEMIMG_RGB5_B ) )
							{
								r1 = ( BYTE )( r >> 3 ) ;
								g1 = ( BYTE )( g >> 3 ) ;
								b1 = ( BYTE )( b >> 3 ) ;

								do
								{
									wtemp = SizeXWord ;
									do
									{
										if( src[0] )
										{
											if( src[0] == 255 )
											{
												*((WORD *)dest) = cr ;
											}
											else
											{
												r3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_R) >> MEMIMG_RGB5_LR );
												g3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_G) >> MEMIMG_RGB5_LG );
												b3 = (BYTE)( (*((WORD *)dest) & MEMIMG_RGB5_B) >> MEMIMG_RGB5_LB );

												*((WORD *)dest) = ( WORD )( 
													(((((r1 - r3) * src[0]) >> 8) + r3) << MEMIMG_RGB5_LR ) | 
													(((((g1 - g3) * src[0]) >> 8) + g3) << MEMIMG_RGB5_LG ) | 
													(((((b1 - b3) * src[0]) >> 8) + b3) << MEMIMG_RGB5_LB ) ) ;
											}
										}
										dest += 2 ;
										src ++ ;
									}while( --wtemp ) ;

									dest += dpitch2 ;
									src += spitch2 ;
								}while( --SizeYWord ) ;
							}
						}
						break ;

					case DX_FONTTYPE_NORMAL :
						{
							DWORD wtemp ;
							WORD dat ;

							if( SrcRect.left % 8 == 0 )
							{
								src = cmanage->CacheMem ;
								src += ( SrcRect.left / 8 ) + SrcRect.top * spitch ;
								spitch2 = spitch - ( ( SizeX / 16 ) * 2 + ( SizeX % 16 != 0 ? 2 : 0 ) ) ;
								dpitch2 = dpitch - ( ( ( SizeX >> 4 ) << 5 ) ) ;

								do{
									wtemp = SizeXWord ;
									do{
										dat = *(( WORD *)src) ;
										do{
											if( dat & 0x80 ) *( ( WORD * )dest ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x40 ) *( ( WORD * )dest + 1 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x20 ) *( ( WORD * )dest + 2 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x10 ) *( ( WORD * )dest + 3 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x8 ) *( ( WORD * )dest + 4 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x4 ) *( ( WORD * )dest + 5 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x2 ) *( ( WORD * )dest + 6 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x1 ) *( ( WORD * )dest + 7 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x8000 ) *( ( WORD * )dest + 8 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x4000 ) *( ( WORD * )dest + 9 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x2000 ) *( ( WORD * )dest + 10 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x1000 ) *( ( WORD * )dest + 11 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x800 ) *( ( WORD * )dest + 12 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x400 ) *( ( WORD * )dest + 13 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x200 ) *( ( WORD * )dest + 14 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x100 ) *( ( WORD * )dest + 15 ) = cr ;
											dest += 32 ;
 											--wtemp ;
										}while(0);

										src += 2 ;
									}while( wtemp );

									src += spitch2 ;
									dest += dpitch2 ;
								}while( --SizeYWord ) ;
							}
							else
							{
								int StX = SrcRect.left % 8 ;
								WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;
								src = cmanage->CacheMem ;
								src += ( SrcRect.left / 8 ) + SrcRect.top * spitch ;
								spitch2 = spitch - ( ( SizeX / 8 ) + ( SizeX % 8 != 0 ? 1 : 0 ) ) ;
								dpitch2 = dpitch - ( ( ( SizeX >> 3 ) << 4 ) ) ;

								do{
									wtemp = SizeXWord ;
									do{
										dat = *( ( WORD * )src ) ;
										do{
											if( dat & Table[StX] ) *( ( WORD * )dest ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+1] ) *( ( WORD * )dest + 1 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+2] ) *( ( WORD * )dest + 2 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+3] ) *( ( WORD * )dest + 3 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+4] ) *( ( WORD * )dest + 4 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+5] ) *( ( WORD * )dest + 5 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+6] ) *( ( WORD * )dest + 6 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+7] ) *( ( WORD * )dest + 7 ) = cr ;
 											--wtemp ;
											dest += 16 ;
										}while(0);

										src += 1 ;
									}while( wtemp );

									src += spitch2 ;
									dest += dpitch2 ;
								}while( --SizeYWord ) ;
							}
						}
						break ;

					case DX_FONTTYPE_EDGE :
						{
							WORD wtemp ;
							WORD ecr ;

							ecr = (WORD)( EdgeColor & 0xffff ) ;
							do
							{
								wtemp = SizeXWord ;
								do
								{
									if( *src ) *((WORD *)dest) = (*src & 2) ? cr : ecr ;
									dest += 2 ;
									src ++ ;
								}while( --wtemp ) ;

								dest += dpitch2 ;
								src += spitch2 ;
							}while( --SizeYWord ) ;
						}
						break ;
					}
				}
				break ;

			case 32 :
				// 32ビットモードの場合
				{
					BYTE *dest = (BYTE *)DestImage->GraphData ;
					BYTE *src = cmanage->CacheMem ;
					DWORD cr = ( DWORD )Color, ecr = (DWORD)EdgeColor ;

					dest += ( DstRect.left << 2 ) + DstRect.top * dpitch ;
					src += ( SrcRect.left ) + SrcRect.top * spitch ;

					spitch2 = spitch - SizeX ;
					dpitch2 = dpitch - ( SizeX << 2 ) ;

					switch( cmanage->FontType )
					{
					case DX_FONTTYPE_ANTIALIASING_EDGE :
					case DX_FONTTYPE_ANTIALIASING_EDGE_4X4 :
					case DX_FONTTYPE_ANTIALIASING_EDGE_8X8 :
						{
							BYTE r1, g1, b1 ;
							BYTE r2, g2, b2 ;
							DWORD AAA, AAA2 ;
							WORD wtemp ;

							spitch2 = spitch - ( SizeX << 1 ) ;
							src = cmanage->CacheMem + ( SrcRect.left << 1 ) + SrcRect.top * spitch ;

							r1 = ( BYTE )r;
							g1 = ( BYTE )g;
							b1 = ( BYTE )b;

							r2 = ( BYTE )er;
							g2 = ( BYTE )eg;
							b2 = ( BYTE )eb;

							do
							{
								wtemp = SizeXWord ;
								do
								{
									if( src[0] )
									{
										if( src[0] == 255 )
										{
											*((DWORD *)dest) = cr ;
										}
										else
										{
											AAA = ((WORD)src[0] << 8) / (AAA2 = src[0] + (((256 - src[0]) * src[1]) >> 8))  ;
											*((DWORD *)dest) = ( DWORD )( 
												(((((((((r1 - r2) * AAA) >> 8) + r2) - dest[2]) * AAA2) >> 8) + dest[2]) << MEMIMG_XRGB8_LR ) | 
												(((((((((g1 - g2) * AAA) >> 8) + g2) - dest[1]) * AAA2) >> 8) + dest[1]) << MEMIMG_XRGB8_LG ) | 
												(((((((((b1 - b2) * AAA) >> 8) + b2) - dest[0]) * AAA2) >> 8) + dest[0]) << MEMIMG_XRGB8_LB ) ) ;
										}
									}
									else
									if( src[1] )
									{
										if( src[1] == 255 )
										{
											*((DWORD *)dest) = ecr ;
										}
										else
										{
											*((DWORD *)dest) = ( DWORD )( 
												(((((r2 - dest[2]) * src[1]) >> 8) + dest[2]) << MEMIMG_XRGB8_LR ) | 
												(((((g2 - dest[1]) * src[1]) >> 8) + dest[1]) << MEMIMG_XRGB8_LG ) | 
												(((((b2 - dest[0]) * src[1]) >> 8) + dest[0]) << MEMIMG_XRGB8_LB ) ) ;
										}
									}

									dest += 4 ;
									src += 2 ;
								}while( --wtemp ) ;

								dest += dpitch2 ;
								src += spitch2 ;
							}while( --SizeYWord ) ;
						}
						break ;


					case DX_FONTTYPE_ANTIALIASING :
					case DX_FONTTYPE_ANTIALIASING_4X4 :
					case DX_FONTTYPE_ANTIALIASING_8X8 :
						{
							BYTE r1, g1, b1 ;
							WORD wtemp ;

							r1 = ( BYTE )r;
							g1 = ( BYTE )g;
							b1 = ( BYTE )b;
							do
							{
								wtemp = SizeXWord ;
								do
								{
									if( src[0] == 255 )
									{
										*((DWORD *)dest) = ecr ;
									}
									else
									{
										*((DWORD *)dest) = ( DWORD )( 
											(((((r1 - dest[2]) * src[1]) >> 8) + dest[2]) << MEMIMG_XRGB8_LR ) | 
											(((((g1 - dest[1]) * src[1]) >> 8) + dest[1]) << MEMIMG_XRGB8_LG ) | 
											(((((b1 - dest[0]) * src[1]) >> 8) + dest[0]) << MEMIMG_XRGB8_LB ) ) ;
									}

									dest += 4 ;
									src ++ ;
								}while( --wtemp ) ;

								dest += dpitch2 ;
								src += spitch2 ;
							}while( --SizeYWord ) ;
						}
						break ;

					case DX_FONTTYPE_NORMAL :
						{
							WORD wtemp ;
							WORD dat ;

							if( SrcRect.left % 8 == 0 )
							{
								src = cmanage->CacheMem ;
								src += ( SrcRect.left / 8 ) + SrcRect.top * spitch ;
								spitch2 = spitch - ( ( SizeX / 16 ) * 2 + ( SizeX % 16 != 0 ? 2 : 0 ) ) ;
								dpitch2 = dpitch - ( ( ( SizeX >> 4 ) << 6 ) ) ;

								do{
									wtemp = SizeXWord ;
									do{
										dat = *( ( WORD * )src ) ;
										do{
											if( dat & 0x80 ) *( ( DWORD * )dest ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x40 ) *( ( DWORD * )dest + 1 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x20 ) *( ( DWORD * )dest + 2 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x10 ) *( ( DWORD * )dest + 3 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x8 ) *( ( DWORD * )dest + 4 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x4 ) *( ( DWORD * )dest + 5 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x2 ) *( ( DWORD * )dest + 6 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x1 ) *( ( DWORD * )dest + 7 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x8000 ) *( ( DWORD * )dest + 8 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x4000 ) *( ( DWORD * )dest + 9 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x2000 ) *( ( DWORD * )dest + 10 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x1000 ) *( ( DWORD * )dest + 11 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x800 ) *( ( DWORD * )dest + 12 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x400 ) *( ( DWORD * )dest + 13 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x200 ) *( ( DWORD * )dest + 14 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & 0x100 ) *( ( DWORD * )dest + 15 ) = cr ;
											dest += 64 ;
 											--wtemp ;
										}while(0);

										src += 2 ;
									}while( wtemp );

									src += spitch2 ;
									dest += dpitch2 ;
								}while( --SizeYWord ) ;
							}
							else
							{
								int StX = SrcRect.left % 8 ;
								WORD Table[16] = { 0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1, 0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100 } ;

								src = cmanage->CacheMem ;
								src += ( SrcRect.left / 8 ) + SrcRect.top * spitch ;
								spitch2 = spitch - ( ( SizeX / 8 ) + ( SizeX % 8 != 0 ? 1 : 0 ) ) ;
								dpitch2 = dpitch - ( ( ( SizeX >> 3 ) << 5 ) ) ;

								do{
									wtemp = SizeXWord ;
									do{
										dat = *( ( WORD * )src ) ;
										do{
											if( dat & Table[StX] ) *( ( DWORD * )dest ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+1] ) *( ( DWORD * )dest + 1 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+2] ) *( ( DWORD * )dest + 2 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+3] ) *( ( DWORD * )dest + 3 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+4] ) *( ( DWORD * )dest + 4 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+5] ) *( ( DWORD * )dest + 5 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+6] ) *( ( DWORD * )dest + 6 ) = cr ;
 											if( --wtemp == 0 ) break ;
											if( dat & Table[StX+7] ) *( ( DWORD * )dest + 7 ) = cr ;
											dest += 32 ;
 											--wtemp ;
										}while(0);

										src += 1 ;
									}while( wtemp );

									src += spitch2 ;
									dest += dpitch2 ;
								}while( --SizeYWord ) ;
							}
						}
						break ;

					case DX_FONTTYPE_EDGE :
						{
							WORD wtemp ;
							DWORD ecr ;
							BYTE dat ;

							ecr = ( DWORD )( EdgeColor & 0xffffff ) ;
							do
							{
								wtemp = SizeXWord ;
								do
								{
									dat = *src ;
									if( dat )
									{
										*( ( DWORD * )dest ) = ( dat & 2 ) ? cr : ecr ;
									}
									dest += 4 ;
									src ++ ;
								}while( --wtemp ) ;

								dest += dpitch2 ;
								src += spitch2 ;
							}while( --SizeYWord ) ;
						}
						break ;
					}
				}
				break ;
			}
		}
	}

	if( DrawSizeP != NULL )
	{
		DrawSizeP->cx = TotalWidth ;
		DrawSizeP->cy = TotalHeight ;
	}

	// 終了
	return VerticalFlag ? TotalHeight : TotalWidth ;
}


// 基本イメージに文字列イメージを転送する
extern int NS_FontBaseImageBlt( int x, int y, const TCHAR *StrData, BASEIMAGE *DestImage, BASEIMAGE *DestEdgeImage, int VerticalFlag )
{
	return NS_FontBaseImageBltToHandle( x, y, StrData, DestImage, DestEdgeImage, DX_DEFAULT_FONT_HANDLE, VerticalFlag ) ;
}

// 基本イメージに文字列を転送する
extern int NS_FontBaseImageBltToHandle( int x, int y, const TCHAR *StrData, BASEIMAGE *DestImage, BASEIMAGE *DestEdgeImage, int FontHandle, int VerticalFlag )
{
	TCHAR DrawStrBuf[ 256 * 3 ] ;		// 描画する際に使用する文字列データ（３バイトで１文字を表現）
	FONTCODEDATA *fontcodedata ;
	FONTMANAGE * cmanage ;
	FONTDATA * cache ;
	RECT SrcRect ;
	RECT BaseRect ;
	RECT DstRect ;
	POINT DstPoint ;
	int drawnum ;							// ２バイト文字１バイト文字全部ひっくるめて何文字あるか保存する変数
	int i ;									// 繰り返しと汎用変数
	const TCHAR *strp ;
	LONG dpitch, spitch ;					// 描画先と描画もとのサーフェスの１ラインのバイト数
	int TotalWidth, TotalHeight ;
//	bool Hangeul ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, cmanage ) )
		return -1 ;

	// 縁用画像が有効で、且つ本体用画像とサイズが違った場合はエラー
	if( DestEdgeImage != NULL && ( DestEdgeImage->Width != DestImage->Width || DestEdgeImage->Height != DestImage->Height ) )
		return -1 ;

	// テクスチャをキャッシュに使用していないフォントは無理
	if( cmanage->TextureCacheFlag == FALSE ) return -1 ;

	// ＵＮＩコードかどうかを取得しておく
//	Hangeul = cmanage->CharSet == DX_CHARSET_HANGEUL ;

	// 文字列がない場合は終了
	if( StrData  == NULL || *StrData  == 0 ) return 0 ;

	// キャッシュに入っていない文字列をキャッシュに入れる
	FontCacheStringAddToHandle( FontHandle, StrData, -1, DrawStrBuf, &drawnum ) ;

	// 描画座標の補正
	if( VerticalFlag )
	{
		x -= cmanage->FontAddHeight / 2 ;
	}
	else
	{
		y -= cmanage->FontAddHeight / 2 ;
	}

	// ピッチをセット
	spitch = cmanage->CachePitch ;
	dpitch = DestImage->Pitch ;

	// 文字の数だけ繰り返し
	strp = DrawStrBuf ;			// 描画用文字データの先頭アドレスをセット
	fontcodedata = cmanage->FontCodeData ;
	TotalWidth = 0 ;
	TotalHeight = 0 ;
	for( i = 0 ; i < drawnum ; i ++,
			TotalHeight = VerticalFlag ? TotalHeight + cache->AddX + cmanage->Space : ( TotalHeight < cache->DrawY + cache->SizeY ? cache->DrawY + cache->SizeY : TotalHeight ),
			TotalWidth  = VerticalFlag ? ( TotalWidth < cache->DrawX + cache->SizeX ? cache->DrawX + cache->SizeX : TotalWidth ) : TotalWidth + cache->AddX + cmanage->Space,
			y += VerticalFlag ? cache->AddX + cmanage->Space : 0, x += VerticalFlag ? 0 : cache->AddX + cmanage->Space, strp += 3 )
	{
		// キャッシュの中に希望の文字があるか検査
		if( fontcodedata[ *(( WORD *)strp) ].ExistFlag )
		{
			// あった場合そのフォントデータのポインタを保存
			cache = fontcodedata[ *(( WORD *)strp) ].DataPoint ; 
		}
		else
		{
			// 無かった場合キャッシュに追加
			cache = FontCacheCharAddToHandle( 1, strp, FontHandle ) ;
			if( cache == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "テキストキャッシュの追加に失敗しました" ) ) ;
				return -1 ;
			}
		}

		// スペース文字だった場合はキャンセルする
#ifdef UNICODE
		if( *( ( wchar_t * )strp ) == L' ' || *( ( wchar_t * )strp ) == L'　'            ) continue ;
#else
		if( *strp                  ==  ' ' || *(( WORD *)strp )      == *(( WORD *)"　") ) continue ;
#endif

		// サイズが無かったら何もしない
		if( cache->SizeX == 0 && cache->SizeY == 0 ) continue ;

		// 転送先の描画領域を作成
		DstRect.left	= x + cache->DrawX ;
		DstRect.top		= y + cache->DrawY ;
		DstRect.right	= x + cache->DrawX + cache->SizeX ;
		DstRect.bottom	= y + cache->DrawY + cache->SizeY ;
		BaseRect.left = 0 ;
		BaseRect.top = 0 ;
		BaseRect.right = DestImage->Width ;
		BaseRect.bottom = DestImage->Height ;
		RectClipping( &DstRect, &BaseRect ) ;

		// 描画する文字が無かった場合ここで次ループに移る
		if( DstRect.right == DstRect.left || DstRect.bottom == DstRect.top ) continue ;

		// 転送元の矩形をセット
		SrcRect.left	=   cache->GraphIndex / cmanage->LengthCharNum   * cmanage->MaxWidth ;
		SrcRect.top		= ( cache->GraphIndex % cmanage->LengthCharNum ) * cmanage->MaxWidth ;
		if( cmanage->TextureCacheFlag == TRUE )
		{
			SrcRect.left ++ ;
			SrcRect.top  ++ ;
		}
		SrcRect.right	= SrcRect.left + cache->SizeX ;
		SrcRect.bottom	= SrcRect.top  + cache->SizeY ;

		// 転送元の矩形を補正
		SrcRect.left   += DstRect.left - ( x + cache->DrawX ) ;
		SrcRect.top    += DstRect.top  - ( y + cache->DrawY ) ;
		SrcRect.right  -= ( ( x + cache->DrawX ) + cache->SizeX ) - DstRect.right ;
		SrcRect.bottom -= ( ( y + cache->DrawY ) + cache->SizeY ) - DstRect.bottom ;

		// 传送
		DstPoint.x = DstRect.left ;
		DstPoint.y = DstRect.top ;
		NS_GraphColorMatchBltVer2(
			DestImage->GraphData, DestImage->Pitch, &DestImage->ColorData,
			cmanage->TextureCacheBaseImage.GraphData, cmanage->TextureCacheBaseImage.Pitch, &cmanage->TextureCacheBaseImage.ColorData,
			NULL, 0, NULL,
			DstPoint, &SrcRect, FALSE,
			FALSE, 0,
			BASEIM.ImageShavedMode, FALSE,
			FALSE, FALSE,
			FALSE ) ;
		if( DestEdgeImage && ( cmanage->FontType & DX_FONTTYPE_EDGE ) != 0 )
		{
			SrcRect.left  += cmanage->TextureCacheBaseImage.Width / 2 ;
			SrcRect.right += cmanage->TextureCacheBaseImage.Width / 2 ;
			NS_GraphColorMatchBltVer2(
				DestEdgeImage->GraphData, DestEdgeImage->Pitch, &DestEdgeImage->ColorData,
				cmanage->TextureCacheBaseImage.GraphData, cmanage->TextureCacheBaseImage.Pitch, &cmanage->TextureCacheBaseImage.ColorData,
				NULL, 0, NULL,
				DstPoint, &SrcRect, FALSE,
				FALSE, 0,
				BASEIM.ImageShavedMode, FALSE,
				FALSE, FALSE,
				FALSE ) ;
		}
	}

	// 終了
	return 0 ;
}

// 文字の最大幅を得る
extern int NS_GetFontMaxWidth( void )
{
	return NS_GetFontMaxWidthToHandle( DX_DEFAULT_FONT_HANDLE ) ;
}

// 文字の最大幅を得る
extern int NS_GetFontMaxWidthToHandle( int FontHandle )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;

	return ManageData->MaxWidth ;
}

// 指定の文字の描画情報を取得する
extern int NS_GetFontCharInfo( int FontHandle, const TCHAR *Char, int *DrawX, int *DrawY, int *NextCharX, int *SizeX, int *SizeY )
{
	FONTMANAGE * ManageData ;
	FONTDATA * FontData ;
	TCHAR CharString[ 3 ] ;
	int Len ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;

	if( _TMULT( *Char, ManageData->CharSet ) == TRUE )
	{
		CharString[ 0 ] = Char[ 0 ] ;
		CharString[ 1 ] = Char[ 1 ] ;
		CharString[ 2 ] = 0 ;
		Len = 2 ;
	}
	else
	{
		CharString[ 0 ] = Char[ 0 ] ;
		CharString[ 1 ] = 0 ;
		CharString[ 2 ] = 0 ;
		Len = 1 ;
	}

	// キャッシュに登録
	NS_GetDrawStringWidthToHandle( CharString, Len, FontHandle ) ;

	// フォントデータを取得
	if( ManageData->FontCodeData[ *( ( WORD * )CharString ) ].ExistFlag )
	{
		FontData = ManageData->FontCodeData[ *( ( WORD * )CharString ) ].DataPoint ;
	}
	else
	{
		// キャッシュに追加できなかったらエラー
		return -1 ;
	}

	// フォントデータを返す
	if( DrawX ) *DrawX = FontData->DrawX ;
	if( DrawY ) *DrawY = FontData->DrawY ;
	if( NextCharX ) *NextCharX = FontData->AddX ;
	if( SizeX ) *SizeX = FontData->SizeX ;
	if( SizeY ) *SizeY = FontData->SizeY ;

	// 正常終了
	return 0 ;
}

// 文字列の幅を得る
extern int NS_GetDrawStringWidth( const TCHAR *String ,int StrLen, int VerticalFlag )
{
	return NS_GetDrawStringWidthToHandle( String, StrLen, DX_DEFAULT_FONT_HANDLE, VerticalFlag ) ;
}

// 書式付き文字列の描画幅を得る
extern int NS_GetDrawFormatStringWidth( const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 文字列の幅を得る
	return NS_GetDrawStringWidthToHandle( String, lstrlen( String ), DX_DEFAULT_FONT_HANDLE ) ;
}


// 書式付き文字列の描画幅を得る
extern int NS_GetDrawFormatStringWidthToHandle( int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 文字列の幅を得る
	return NS_GetDrawStringWidthToHandle( String, lstrlen( String ), FontHandle ) ;
}


// 文字列の幅を得る
extern int NS_GetDrawStringWidthToHandle( const TCHAR *String, int StrLen, int FontHandle, int VerticalFlag )
{
	return FontCacheStringDrawToHandleST(
				FALSE,
				0,
				0,
				0.0f,
				0.0f,
				TRUE,
				false,
				1.0,
				1.0,
				String,
				0,
				NULL,
				NULL,
				FALSE,
				FontHandle,
				0,
				StrLen,
				VerticalFlag,
				NULL
			) ;
}



// 文字列の幅を得る
extern int NS_GetDrawExtendStringWidth( double ExRateX, const TCHAR *String ,int StrLen, int VerticalFlag )
{
	return NS_GetDrawExtendStringWidthToHandle( ExRateX, String, StrLen, DX_DEFAULT_FONT_HANDLE, VerticalFlag ) ;
}

// 書式付き文字列の描画幅を得る
extern int NS_GetDrawExtendFormatStringWidth( double ExRateX, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 文字列の幅を得る
	return NS_GetDrawExtendStringWidthToHandle( ExRateX, String, lstrlen( String ), DX_DEFAULT_FONT_HANDLE ) ;
}


// 書式付き文字列の描画幅を得る
extern int NS_GetDrawExtendFormatStringWidthToHandle( double ExRateX, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 文字列の幅を得る
	return NS_GetDrawExtendStringWidthToHandle( ExRateX, String, lstrlen( String ), FontHandle ) ;
}

// 文字列の幅を得る
extern int NS_GetDrawExtendStringWidthToHandle( double ExRateX, const TCHAR *String, int StrLen, int FontHandle, int VerticalFlag )
{
	return FontCacheStringDrawToHandleST(
				FALSE,
				0,
				0,
				0.0f,
				0.0f,
				TRUE,
				true,
				ExRateX,
				1.0,
				String,
				0,
				NULL,
				NULL,
				FALSE,
				FontHandle,
				0,
				StrLen,
				VerticalFlag,
				NULL
			) ;
}

// フォントの情報を得る
extern int NS_GetFontStateToHandle( TCHAR *FontName, int *Size, int *Thick, int FontHandle, int *FontType , int *CharSet , int *EdgeSize , int *Italic )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;

	if( FontName ) lstrcpy( FontName, ManageData->FontName ) ;
	if( Thick    ) *Thick    = ManageData->FontThickness ;
	if( Size     ) *Size     = ManageData->FontSize ;
	if( FontType ) *FontType = ManageData->FontType ;
	if( CharSet  ) *CharSet  = ManageData->CharSet ;
	if( EdgeSize ) *EdgeSize = ManageData->EdgeSize ;
	if( Italic   ) *Italic   = ManageData->Italic ;

	// 終了
	return 0;
}

// デフォルトのフォントのハンドルを得る
extern int NS_GetDefaultFontHandle( void )
{
	return FSYS.DefaultFontHandle ;
}

// 指定のフォントがテクスチャキャッシュを使用しているかどうかを得る
extern int NS_CheckFontCacheToTextureFlag( int FontHandle )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	// エラー判定
	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;

	// フラグを返す
	return ManageData->TextureCacheFlag ;
}

// 指定のフォントがテクスチャキャッシュを使用しているかどうかを得る(誤字版)
extern int NS_CheckFontChacheToTextureFlag( int FontHandle )
{
	return NS_CheckFontCacheToTextureFlag( FontHandle );
}

// フォントにテクスチャキャッシュを使用するかどうかを取得する
extern int NS_GetFontCacheToTextureFlag( void )
{
	return !FSYS.NotTextureFlag ;
}

// フォントにテクスチャキャッシュを使用するかどうかを取得する(誤字版)
extern int NS_GetFontChacheToTextureFlag( void )
{
	return NS_GetFontCacheToTextureFlag() ;
}

// フォントのキャッシュとして使用するテクスチャのカラービット深度を設定する( 16 又は 32 のみ指定可能  デフォルトは 32 )
extern int NS_SetFontCacheTextureColorBitDepth( int ColorBitDepth )
{
	// フラグを保存
	FSYS.TextureCacheColorBitDepth16Flag = ColorBitDepth == 16 ? TRUE : FALSE ;
	
	// 終了
	return 0 ;
}

// フォントのキャッシュとして使用するテクスチャのカラービット深度を取得する
extern int NS_GetFontCacheTextureColorBitDepth( void )
{
	return FSYS.TextureCacheColorBitDepth16Flag ? 16 : 32 ;
}

// 指定のフォントハンドルが有効か否か調べる
extern int NS_CheckFontHandleValid( int FontHandle )
{
	FONTMANAGE * ManageData ;

	DEFAULT_FONT_HANDLE_SETUP

	if( FONTHCHK( FontHandle, ManageData ) )
		return 0 ;

	return 1 ;
}

// ２バイト文字か調べる( TRUE:２バイト文字  FALSE:１バイト文字 )
extern int NS_MultiByteCharCheck( const char *Buf, int CharSet )
{
	return CheckMultiByteChar( *Buf, CharSet ) ;
}


// フォントキャッシュでキャッシュできる文字数を取得する( 戻り値  0:デフォルト  1以上:指定文字数 )
extern int NS_GetFontCacheCharNum( void )
{
	return FSYS.CacheCharNum ;
}

// フォントのサイズを得る
extern int NS_GetFontSize( void )
{
	return NS_GetFontSizeToHandle( DX_DEFAULT_FONT_HANDLE ) ;
}

// フォントのサイズを得る
extern int NS_GetFontSizeToHandle( int FontHandle )
{
	FONTMANAGE * ManageData ;
	
	DEFAULT_FONT_HANDLE_SETUP

	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;

	return ManageData->FontSize ;
}

// フォントハンドルの字間を取得する
extern int NS_GetFontSpaceToHandle( int FontHandle )
{
	FONTMANAGE * ManageData ;
	
	DEFAULT_FONT_HANDLE_SETUP

	if( FONTHCHK( FontHandle, ManageData ) )
		return -1 ;

	return ManageData->Space ;
}

// フォントキャッシュとして保存する画像の形式を乗算済みαチャンネル付き画像にするかどうかを設定する( TRUE:乗算済みαを使用する  FLASE:乗算済みαを使用しない( デフォルト ) )
extern int NS_SetFontCacheUsePremulAlphaFlag( int Flag )
{
	FSYS.UsePremulAlphaFlag = Flag == FALSE ? FALSE : TRUE ;

	return 0 ;
}

// フォントキャッシュとして保存する画像の形式を乗算済みαチャンネル付き画像にするかどうかを取得する
extern int NS_GetFontCacheUsePremulAlphaFlag( void )
{
	return FSYS.UsePremulAlphaFlag ;
}

// 文字列を描画する
extern int NS_DrawString( int x, int y, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawStringToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, FALSE ) ;
}

// 文字列を描画する
extern int NS_DrawStringF( float x, float y, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawStringFToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, FALSE ) ;
}

// 文字列を描画する
extern int NS_DrawVString( int x, int y, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawStringToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, TRUE ) ;
}

// 文字列を描画する
extern int NS_DrawVStringF( float x, float y, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawStringFToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, TRUE ) ;
}

// 文字列を描画する
#define SETDRAWRECTCODE\
	if( VerticalFlag )\
	{\
		x += font->FontAddHeight / 2;\
		SETRECT( GRH.DrawRect, x, y, x + NS_GetFontSizeToHandle( FontHandle ) + 3, GBASE.DrawArea.bottom ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
		x -= font->FontAddHeight / 2;\
	}\
	else\
	{\
		y -= font->FontAddHeight / 2 ;\
		SETRECT( GRH.DrawRect, x, y, GBASE.DrawArea.right, y + NS_GetFontSizeToHandle( FontHandle ) + 3 ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
		y += font->FontAddHeight / 2 ;\
	}\
	DRAWRECT_DRAWAREA_CLIP


extern int NS_DrawStringToHandle( int x, int y, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	int Ret = -1 ;
	int Flag ;
	FONTMANAGE *font ;

	DEFAULT_FONT_HANDLE_SETUP

	if( String == NULL || String[0] == _T( '\0' ) ||
		GBASE.NotDrawFlag || GBASE.NotDrawFlagInSetDrawArea )
		return 0 ;

	if( NS_CheckFontHandleValid( FontHandle ) == 0 )
	{
		DXST_ERRORLOG_ADD( _T( "フォントハンドル値が異常です\n" ) ) ;
		return Ret ;
	}
	font = GetFontManageDataToHandle( FontHandle ) ;

	Flag = font->TextureCacheFlag ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 描画
	DRAW_DEF(
		DrawStringHardware( x, y, ( float )x, ( float )y, TRUE, String, Color, FontHandle, EdgeColor, VerticalFlag ),
		DrawStringSoftware( x, y,                               String, Color, FontHandle, EdgeColor, VerticalFlag ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE



// 文字列を描画する
#define SETDRAWRECTCODE\
	if( VerticalFlag )\
	{\
		x += ( float )font->FontAddHeight / 2;\
		SETRECT( GRH.DrawRect, _FTOL( x ), _FTOL( y ), _FTOL( x ) + NS_GetFontSizeToHandle( FontHandle ) + 3, GBASE.DrawArea.bottom ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
		x -= ( float )font->FontAddHeight / 2;\
	}\
	else\
	{\
		y -= ( float )font->FontAddHeight / 2 ;\
		SETRECT( GRH.DrawRect, _FTOL( x ), _FTOL( y ), GBASE.DrawArea.right, _FTOL( y ) + NS_GetFontSizeToHandle( FontHandle ) + 3 ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
		y += ( float )font->FontAddHeight / 2 ;\
	}\
	DRAWRECT_DRAWAREA_CLIP


extern int NS_DrawStringFToHandle( float x, float y, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	int Ret = -1 ;
	int Flag ;
	FONTMANAGE *font ;

	DEFAULT_FONT_HANDLE_SETUP

	if( String == NULL || String[0] == _T( '\0' ) ||
		GBASE.NotDrawFlag || GBASE.NotDrawFlagInSetDrawArea )
		return 0 ;

	if( NS_CheckFontHandleValid( FontHandle ) == 0 )
	{
		DXST_ERRORLOG_ADD( _T( "フォントハンドル値が異常です\n" ) ) ;
		return Ret ;
	}
	font = GetFontManageDataToHandle( FontHandle ) ;

	Flag = font->TextureCacheFlag ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 描画
	DRAW_DEF(
		DrawStringHardware( _FTOL( x ), _FTOL( y ), x, y, FALSE, String, Color, FontHandle, EdgeColor, VerticalFlag ),
		DrawStringSoftware( _FTOL( x ), _FTOL( y ),              String, Color, FontHandle, EdgeColor, VerticalFlag ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE




// 文字列を描画する
extern int NS_DrawVStringToHandle( int x, int y, const TCHAR *String, int Color, int FontHandle, int EdgeColor )
{
	return NS_DrawStringToHandle( x, y, String, Color, FontHandle, EdgeColor, TRUE ) ;
}

// 文字列を描画する
extern int NS_DrawVStringFToHandle( float x, float y, const TCHAR *String, int Color, int FontHandle, int EdgeColor )
{
	return NS_DrawStringFToHandle( x, y, String, Color, FontHandle, EdgeColor, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatString( int x, int y, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// 編集後の文字列を取得する
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawStringToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatStringF( float x, float y, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// 編集後の文字列を取得する
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawStringFToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatVString( int x, int y, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	return NS_DrawStringToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, Color, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatVStringF( float x, float y, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	return NS_DrawStringFToHandle( x, y, String, Color, DX_DEFAULT_FONT_HANDLE, Color, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatStringToHandle( int x, int y, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawStringToHandle( x, y, String, Color, FontHandle, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatStringFToHandle( float x, float y, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawStringFToHandle( x, y, String, Color, FontHandle, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatVStringToHandle( int x, int y, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawStringToHandle( x, y, String, Color, FontHandle, Color, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawFormatVStringFToHandle( float x, float y, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 2048 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawStringFToHandle( x, y, String, Color, FontHandle, Color, TRUE ) ;
}


// 文字列を描画する
extern int NS_DrawExtendString( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawExtendStringToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, FALSE ) ;
}

// 文字列を描画する
extern int NS_DrawExtendStringF( float x, float y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawExtendStringFToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, FALSE ) ;
}

// 文字列を描画する
extern int NS_DrawExtendVString( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawExtendStringToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, TRUE ) ;
}

// 文字列を描画する
extern int NS_DrawExtendVStringF( float x, float y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int EdgeColor )
{
	return NS_DrawExtendStringFToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor, TRUE ) ;
}

// 文字列を描画する
#define SETDRAWRECTCODE\
	if( VerticalFlag )\
	{\
		SETRECT( GRH.DrawRect, x, y, x + _DTOL( NS_GetFontSizeToHandle( FontHandle ) * ExRateY ) + 3, GBASE.DrawArea.bottom ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
	}\
	else\
	{\
		SETRECT( GRH.DrawRect, x, y, GBASE.DrawArea.right, y + _DTOL( NS_GetFontSizeToHandle( FontHandle ) * ExRateY ) + 3 ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
	}

extern int NS_DrawExtendStringToHandle( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	int Ret = -1 ;
	int Flag ;
	FONTMANAGE *font ;

	DEFAULT_FONT_HANDLE_SETUP

	if( String == NULL || String[0] == _T( '\0' ) ||
		GBASE.NotDrawFlag || GBASE.NotDrawFlagInSetDrawArea )
		return 0 ;

	if( NS_CheckFontHandleValid( FontHandle ) == 0 )
	{
		DXST_ERRORLOG_ADD( _T( "フォントハンドル値が異常です\n" ) ) ;
		return Ret ;
	}
	font = GetFontManageDataToHandle( FontHandle ) ;

	Flag = font->TextureCacheFlag ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 描画
	DRAW_DEF(
		DrawExtendStringHardware( x, y, ( float )x, ( float )y, TRUE, ExRateX, ExRateY, String, Color, FontHandle, EdgeColor, VerticalFlag ),
		DrawExtendStringSoftware( x, y,                               ExRateX, ExRateY, String, Color, FontHandle, EdgeColor, VerticalFlag ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE



// 文字列を描画する
#define SETDRAWRECTCODE\
	if( VerticalFlag )\
	{\
		SETRECT( GRH.DrawRect, _FTOL( x ), _FTOL( y ), _FTOL( x ) + _DTOL( NS_GetFontSizeToHandle( FontHandle ) * ExRateY ) + 3, GBASE.DrawArea.bottom ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
	}\
	else\
	{\
		SETRECT( GRH.DrawRect, _FTOL( x ), _FTOL( y ), GBASE.DrawArea.right, _FTOL( y ) + _DTOL( NS_GetFontSizeToHandle( FontHandle ) * ExRateY ) + 3 ) ;\
		if( GRH.DrawRect.left >= GBASE.DrawArea.right ) return 0 ;\
	}

extern int NS_DrawExtendStringFToHandle( float x, float y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor, int VerticalFlag )
{
	int Ret = -1 ;
	int Flag ;
	FONTMANAGE *font ;

	DEFAULT_FONT_HANDLE_SETUP

	if( String == NULL || String[0] == _T( '\0' ) ||
		GBASE.NotDrawFlag || GBASE.NotDrawFlagInSetDrawArea )
		return 0 ;

	if( NS_CheckFontHandleValid( FontHandle ) == 0 )
	{
		DXST_ERRORLOG_ADD( _T( "フォントハンドル値が異常です\n" ) ) ;
		return Ret ;
	}
	font = GetFontManageDataToHandle( FontHandle ) ;

	Flag = font->TextureCacheFlag ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// 描画
	DRAW_DEF(
		DrawExtendStringHardware( _FTOL( x ), _FTOL( y ), x, y, FALSE, ExRateX, ExRateY, String, Color, FontHandle, EdgeColor, VerticalFlag ),
		DrawExtendStringSoftware( _FTOL( x ), _FTOL( y ),              ExRateX, ExRateY, String, Color, FontHandle, EdgeColor, VerticalFlag ),
		SETDRAWRECTCODE,
		Ret,
		Flag
	)

	// 終了
	return Ret ;
}

#undef SETDRAWRECTCODE



// 文字列を描画する
extern int NS_DrawExtendVStringToHandle( int x, int y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor )
{
	return NS_DrawExtendStringToHandle( x, y, ExRateX, ExRateY, String, Color, FontHandle, EdgeColor, TRUE ) ;
}

// 文字列を描画する
extern int NS_DrawExtendVStringFToHandle( float x, float y, double ExRateX, double ExRateY, const TCHAR *String, int Color, int FontHandle, int EdgeColor )
{
	return NS_DrawExtendStringFToHandle( x, y, ExRateX, ExRateY, String, Color, FontHandle, EdgeColor, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatString( int x, int y, double ExRateX, double ExRateY, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// 編集後の文字列を取得する
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawExtendStringToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatStringF( float x, float y, double ExRateX, double ExRateY, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// 編集後の文字列を取得する
	{
		va_start( VaList, FormatString ) ;
		_TVSPRINTF( String, FormatString, VaList ) ;
		va_end( VaList ) ;
	}

	return NS_DrawExtendStringFToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatVString( int x, int y, double ExRateX, double ExRateY, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	return NS_DrawExtendStringToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, Color, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatVStringF( float x, float y, double ExRateX, double ExRateY, int Color, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	return NS_DrawExtendStringFToHandle( x, y, ExRateX, ExRateY, String, Color, DX_DEFAULT_FONT_HANDLE, Color, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatStringToHandle( int x, int y, double ExRateX, double ExRateY, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawExtendStringToHandle( x, y, ExRateX, ExRateY, String, Color, FontHandle, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatStringFToHandle( float x, float y, double ExRateX, double ExRateY, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawExtendStringFToHandle( x, y, ExRateX, ExRateY, String, Color, FontHandle, 0, FALSE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatVStringToHandle( int x, int y, double ExRateX, double ExRateY, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawExtendStringToHandle( x, y, ExRateX, ExRateY, String, Color, FontHandle, Color, TRUE ) ;
}

// 書式指定文字列を描画する
extern int NS_DrawExtendFormatVStringFToHandle( float x, float y, double ExRateX, double ExRateY, int Color, int FontHandle, const TCHAR *FormatString, ... )
{
	va_list VaList ;
	TCHAR String[ 1024 ] ;

	// ログ出力用のリストをセットする
	va_start( VaList, FormatString ) ;

	// 編集後の文字列を取得する
	_TVSPRINTF( String, FormatString, VaList ) ;
	
	// 可変長リストのポインタをリセットする
	va_end( VaList ) ;

	// 描画する
	return NS_DrawExtendStringFToHandle( x, y, ExRateX, ExRateY, String, Color, FontHandle, Color, TRUE ) ;
}


// 整数型の数値を描画する
extern int NS_DrawNumberToI( int x, int y, int Num, int RisesNum, int Color ,int EdgeColor )
{
	return NS_DrawNumberToIToHandle( x, y, Num, RisesNum, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor ) ;
}

// 浮動小数点型の数値を描画する
extern int NS_DrawNumberToF( int x, int y, double Num, int Length, int Color ,int EdgeColor  )
{
	return NS_DrawNumberToFToHandle( x, y, Num, Length, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor ) ;
}

// 整数型の数値とその説明の文字列を一度に描画する
extern int NS_DrawNumberPlusToI( int x, int y, const TCHAR *NoteString, int Num, int RisesNum, int Color ,int EdgeColor )
{
	return NS_DrawNumberPlusToIToHandle( x, y, NoteString, Num, RisesNum, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor ) ;
}

// 浮動小数点型の数値とその説明の文字列を一度に描画する
extern int NS_DrawNumberPlusToF( int x, int y, const TCHAR *NoteString, double Num, int Length, int Color ,int EdgeColor )
{
	return NS_DrawNumberPlusToFToHandle( x, y, NoteString, Num, Length, Color, DX_DEFAULT_FONT_HANDLE, EdgeColor ) ;
}



// 整数型の数値を描画する
extern int NS_DrawNumberToIToHandle( int x, int y, int Num, int RisesNum, int Color, int FontHandle ,int EdgeColor )
{
	TCHAR Str[50] ;

	_ITOT( Num, Str, RisesNum ) ;
	return NS_DrawStringToHandle( x, y, Str, Color, FontHandle, EdgeColor ) ;
}

// 浮動小数点型の数値を描画する
extern int NS_DrawNumberToFToHandle( int x, int y, double Num, int Length, int Color, int FontHandle,int EdgeColor  )
{
	TCHAR Str[256], Str2[50] ;

	lstrcpy( Str2, _T( "%." ) ) ;
	_ITOT( Length, Str, 10 ) ;
	lstrcat( Str2, Str ) ;
	lstrcat( Str2, _T( "f" ) ) ;
	_TSPRINTF( _DXWTP( Str ), _DXWTP( Str2 ), Num ) ;
	return NS_DrawStringToHandle( x, y, Str, Color, FontHandle, EdgeColor ) ;
}



// 整数型の数値とその説明の文字列を一度に描画する
extern int NS_DrawNumberPlusToIToHandle( int x, int y, const TCHAR *NoteString, int Num, int RisesNum, int Color, int FontHandle,int EdgeColor )
{
	TCHAR Str[256], Str2[30] ;

//	_SPRINTF( Str, "%s=%d", NoteString, Num ) ;

	lstrcpy( Str, NoteString ) ;
	lstrcat( Str, _T( "=" ) ) ;

	_ITOT( Num, Str2, RisesNum ) ;
	lstrcat( Str, Str2 ) ;

	// 描画
	return NS_DrawStringToHandle( x, y, Str, Color, FontHandle, EdgeColor ) ;
}

// 浮動小数点型の数値とその説明の文字列を一度に描画する
extern int NS_DrawNumberPlusToFToHandle( int x, int y, const TCHAR *NoteString, double Num, int Length, int Color, int FontHandle ,int EdgeColor )
{
	TCHAR Str[256], Str2[30], Str3[80] ;

//	_STRCPY( Str2, "%." ) ;
//	_SPRINTF( Str, "%d", Length ) ;

//	_SPRINTF( Str3, "%s=%s%df", NoteString, Str2, Str ) ;
//	_SPRINTF( Str, Str3, Num ) ;

	lstrcpy( Str, NoteString ) ;
	lstrcat( Str, _T( "=" ) ) ;

	lstrcpy( Str2, _T( "%." ) ) ;
	_ITOT( Length, Str3, 10 ) ;
	lstrcat( Str2, Str3 ) ;
	lstrcat( Str2, _T( "f" ) ) ;
	_TSPRINTF( _DXWTP( Str3 ), _DXWTP( Str2 ), Num ) ;

	lstrcat( Str, Str3 ) ;

	// 描画
	return NS_DrawStringToHandle( x, y, Str, Color, FontHandle, EdgeColor ) ;
}


}

#endif // DX_NON_FONT
