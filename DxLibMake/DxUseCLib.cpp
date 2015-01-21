// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		標準Ｃライブラリ使用コード
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxUseCLib.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxFile.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxMemory.h"
#include "DxLog.h"

#ifndef DX_NON_MOVIE
#include "Windows/DxWinAPI.h"
#include "Windows/DxGuid.h"
#endif

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef __ANDROID
#include <wchar.h>
#endif

#ifndef DX_NON_BULLET_PHYSICS
	#include "btBulletDynamicsCommon.h"
#endif

#ifndef DX_NON_PNGREAD
	#include "png.h"
    #include "pngpriv.h"
#endif

#ifndef DX_NON_JPEGREAD
	#ifdef DX_GCC_COMPILE
	typedef unsigned char boolean;
	#endif	
	#ifdef __BCC
	typedef unsigned char boolean;
	#endif	
	extern "C"
	{
	#include "jpeglib.h"
	}
#endif

namespace DxLib
{

// 構造体型宣言 ------------------------------------------------------------------

// 函数原型声明 ----------------------------------------------------------

// extern int LoadPngImage( STREAMDATA *Src, BASEIMAGE *Image ) ;
// extern int LoadJpegImage( STREAMDATA *Src, BASEIMAGE *Image ) ;
// extern int NS_SaveBaseImageToPng( const char *pFilePath, BASEIMAGE *Image, int CompressionLevel ) ;
// extern int NS_SaveBaseImageToJpeg( const char *pFilePath, BASEIMAGE *Image, int Quality ) ;
// extern int NS_SRand( int Seed ) ;
// extern int NS_GetRand( int RandMax ) ;

// プログラム --------------------------------------------------------------------

extern int _VSWPRINTF( DXWCHAR *Buffer, const DXWCHAR *FormatWString, va_list Arg )
{
#ifdef __ANDROID
	return vswprintf( ( wchar_t * )Buffer, 4096, ( wchar_t * )FormatWString, Arg ) ;
#else
	return vswprintf( ( wchar_t * )Buffer, ( wchar_t * )FormatWString, Arg ) ;
#endif
}


// 汎用データ読み込み処理からのＰＮＧ画像読み込みのためのプログラム

#ifndef DX_NON_PNGREAD

// 汎用データ読み込み処理からの読み込みをするためのデータ型
typedef struct tagPNGGENERAL
{
	STREAMDATA *Data ;
	unsigned int DataSize ;
} PNGGENERAL ;

// 汎用データ読み込み処理からデータを読み込むコールバック関数
static void png_general_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PNGGENERAL *PGen ;

	PGen = (PNGGENERAL *)/*CVT_PTR*/(png_ptr->io_ptr) ;

	// 残りのサイズが足りなかったらエラー
	if( PGen->DataSize - ( int )STTELL( PGen->Data ) < length )
	{
		png_error(png_ptr, "Read Error");
	}
	else
	{
		STREAD( data, length, 1, PGen->Data ) ;
	}
}

// 汎用データ読み込み処理からの読み込みを設定する関数
int png_general_read_set( png_structp png_ptr, PNGGENERAL *PGen, STREAMDATA *Data )
{
	PGen->Data = Data ;

	// 現在のファイルポインタから終端までのサイズを取得する
	{
		long pos ;
		pos = ( long )STTELL( PGen->Data ) ;
		STSEEK( PGen->Data, 0, STREAM_SEEKTYPE_END ) ;
		PGen->DataSize = ( unsigned int )STTELL( PGen->Data ) - pos ;
		STSEEK( PGen->Data, pos, STREAM_SEEKTYPE_SET ) ;
	}

	// コールバック関数のセット
	png_set_read_fn( png_ptr, PGen, png_general_read_data ) ;

	// 終了
	return 0 ;
}

// ＰＮＧ画像の読みこみ
extern int LoadPngImage( STREAMDATA *Src, BASEIMAGE *Image )
{
	png_bytep *row_pointers;
	size_t row, rowbytes ;
	bool Expand ;
	BYTE *GData ;
	PNGGENERAL PGen ;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	void *GraphPoint ;
//	int i ;
	png_bytep BufPoint ;
	BYTE Check[ 8 ] ;

	Expand = false ;

	// 最初の８バイトでPNGファイルかどうかをチェック
	STREAD( Check, 8, 1, Src ) ;
	STSEEK( Src, 0, STREAM_SEEKTYPE_SET ) ;
	if( Check[ 0 ] != 0x89 || Check[ 1 ] != 0x50 || Check[ 2 ] != 0x4E || Check[ 3 ] != 0x47 ||
		Check[ 4 ] != 0x0D || Check[ 5 ] != 0x0A || Check[ 6 ] != 0x1A || Check[ 7 ] != 0x0A )
	{
		return -1 ;
	}

	// ＰＮＧ管理情報の作成
	if( ( png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL ) ) == NULL ) return -1 ;

	// 画像情報構造体の作成
	if( ( info_ptr = png_create_info_struct( png_ptr ) ) == NULL ) 
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return -1 ;
	}

	// エラー時の処理セットアップおよび処理
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
		png_destroy_read_struct( &png_ptr, &info_ptr, (png_infopp)NULL );
		return -1 ;
	}

	// 汎用データ読み込み処理から読み込む場合の設定を行う
	png_general_read_set( png_ptr, &PGen, Src ) ;

	// 設定処理郡
	png_set_sig_bytes(		png_ptr, sig_read ) ;												// よくわから無い処理(汗)
	png_read_info(			png_ptr, info_ptr );												// 画像情報を得る
	png_get_IHDR(			png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,		// 画像の基本ステータスを取得する
							&interlace_type, NULL, NULL ) ;
	png_set_strip_16(		png_ptr ) ;															// １６ビットの画像でも８ビットで収納するように設定
//	if( BmpFlag == TRUE )										png_set_strip_alpha( png_ptr ) ;// アルファチャンネル無視
	png_set_packing(		png_ptr ) ;															// １バイト以下のパレット画像をバイト単位で展開するよう指定
	png_set_packswap(		png_ptr ) ;															// よくわから無い処理

//	if( color_type == PNG_COLOR_TYPE_PALETTE )					png_set_expand( png_ptr ) ;		// パレット使用画像データの自動展開指定

	// αチャンネル付きパレットか８ビット以下のグレースケール画像の場合は
	// 出力画像のピクセルフォーマットをフルカラーにする
	if( ( color_type == PNG_COLOR_TYPE_GRAY && bit_depth <= 8 ) ||
	    png_get_valid( png_ptr, info_ptr, PNG_INFO_tRNS ) )
	{
		png_set_expand( png_ptr );		
		Expand = true ;
	}
	png_set_bgr(			png_ptr ) ;															// カラー配列をＲＧＢからＢＧＲに反転

	// 変更した設定を反映させる
	png_read_update_info(	png_ptr, info_ptr ) ;

	// １ラインあたりに必要なデータ量を得る
	rowbytes = png_get_rowbytes( png_ptr, info_ptr ) ;

	// グラフィックデータを格納するメモリ領域を作成する
	{
		png_bytep BufP ;

		row_pointers = ( png_bytep * )DXALLOC( height * sizeof( png_bytep * ) ) ;
		if( ( BufPoint = ( png_bytep )png_malloc( png_ptr, rowbytes * height ) ) == NULL )
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return -1 ;
		}
		BufP = BufPoint ;
		for (row = 0; row < height; row++, BufP += rowbytes )
			row_pointers[row] = BufP ;
	}

	// 画像データの読み込み
	png_read_image( png_ptr, row_pointers );

	// 実際に使うグラフィックデータ領域の作成・転送
	GraphPoint = DXALLOC( rowbytes * height ) ;
	GData = ( BYTE * )GraphPoint ;
	for (row = 0; row < height; row++, GData += rowbytes )
		_MEMCPY( GData, row_pointers[row], rowbytes ) ;

	// メモリの解放
	png_free( png_ptr, BufPoint ) ;
	DXFREE( row_pointers ) ;

	// 読み込み処理の終了
	png_read_end(png_ptr, info_ptr);

	// BASEIMAGE データの情報をセットする
	{
		Image->Width		= width ;
		Image->Height		= height ;
		Image->Pitch		= ( int )rowbytes ;
		Image->GraphData	= GraphPoint ;

		// カラー情報をセットする
		if( color_type == PNG_COLOR_TYPE_PALETTE && Expand == false )
		{
			png_colorp SrcPalette ;
			int PaletteNum ;
			int i ;
			COLORPALETTEDATA *Palette ;
			
			// パレットカラーの場合
			NS_CreatePaletteColorData( &Image->ColorData ) ;

			// パレットを取得
			png_get_PLTE( png_ptr, info_ptr, &SrcPalette, &PaletteNum ) ;

			// パレットの数が２５６以上だった場合は２５６に補正
			if( PaletteNum < 256 ) PaletteNum = 256 ;

			// パレットのコピー
			Palette = Image->ColorData.Palette ;
			for( i = 0 ; i < PaletteNum ; i ++, Palette ++, SrcPalette ++ )
			{
				Palette->Blue  = SrcPalette->blue ;
				Palette->Green = SrcPalette->green ;
				Palette->Red   = SrcPalette->red ;
				Palette->Alpha = 0 ;
			}
		}
		else
		if( color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA )
		{
			// もしグレースケールなのに１ピクセル２バイトだったら下位８ビットだけを使用する
			if( rowbytes / 2 >= width )
			{
				Image->ColorData.ColorBitDepth	= 16 ;
				Image->ColorData.PixelByte		= 2 ;

				Image->ColorData.NoneMask		= 0x0000ff00 ;

				Image->ColorData.AlphaLoc		= 0 ;
				Image->ColorData.AlphaMask		= 0 ;
				Image->ColorData.AlphaWidth		= 0 ;

				Image->ColorData.RedLoc			= 0 ;
				Image->ColorData.RedMask		= 0x000000ff ;
				Image->ColorData.RedWidth		= 8 ;

				Image->ColorData.GreenLoc		= 0 ;
				Image->ColorData.GreenMask		= 0x000000ff ;
				Image->ColorData.GreenWidth		= 8 ;

				Image->ColorData.BlueLoc		= 0  ;
				Image->ColorData.BlueMask		= 0x000000ff ;
				Image->ColorData.BlueWidth		= 8 ;
			}
			else
			{
				NS_CreateGrayColorData( &Image->ColorData ) ;
			}
		}
		else
		{
			if( info_ptr->channels == 4 )
			{
				NS_CreateFullColorData( &Image->ColorData ) ;
				Image->ColorData.ColorBitDepth	= 32 ;
				Image->ColorData.PixelByte		= 4 ;
				Image->ColorData.AlphaLoc		= 24 ;
				Image->ColorData.AlphaWidth		= 8 ;
				Image->ColorData.AlphaMask		= 0xff000000 ;
			}
			else
			{
				NS_CreateFullColorData( &Image->ColorData ) ;
			}
		}
	}

	// 読み込み処理用構造体の破棄
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	// 終了
	return 0;
}

#ifndef DX_NON_SAVEFUNCTION

extern int SaveBaseImageToPngBase( const DXWCHAR *pFilePathW, const char *pFilePathA, BASEIMAGE *Image, int CompressionLevel )
{
	FILE       *fp;
	png_structp png_ptr;
	png_infop   info_ptr;
	png_bytepp  buffer;
	png_bytep   sample;
	int r, g, b, a, i, j;
	
	// 保存用のファイルを開く
	if( pFilePathW )
	{
		fp = _wfopen( ( wchar_t * )pFilePathW, L"wb");
	}
	else
	{
		fp = fopen( pFilePathA, "wb");
	}
	if( fp == NULL ) return -1;

	// ＰＮＧ管理情報の作成
	png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if( png_ptr == NULL ) return -1;

	// 画像情報構造体の作成
	info_ptr = png_create_info_struct(png_ptr);             // info_ptr構造体を確保・初期化します
	if( info_ptr == NULL )
	{
		png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
		return -1 ;
	}

	// エラー時の処理セットアップおよび処理
	if( setjmp( png_jmpbuf( png_ptr ) ) )
	{
ERR:
		if( buffer )
		{
			for( i = 0; i < Image->Height; i++ )
				if( buffer[i] ) DXFREE( buffer[i] );
			DXFREE( buffer );
		}
		if( fp ) fclose( fp );
		png_destroy_write_struct( &png_ptr, &info_ptr );
		return -1 ;
	}

	// 開いたファイルポインタをセット
	png_init_io( png_ptr, fp );

	// 使用するフィルタのセット
	png_set_filter( png_ptr, 0, PNG_ALL_FILTERS );

	// 圧縮レベルのセット
	png_set_compression_level( png_ptr, CompressionLevel );

	// IHDRチャンク情報の設定
	png_set_IHDR(
		png_ptr,
		info_ptr,
		Image->Width,
		Image->Height,
		8,
		Image->ColorData.AlphaWidth ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);

	// ヘッダ部分の書き出し
	png_write_info( png_ptr, info_ptr );

	// 圧縮用データの用意
	{
		// バッファの確保
		buffer = (png_bytepp)DXALLOC( sizeof( png_bytep ) * Image->Height );
		if( buffer == NULL ) goto ERR;
		_MEMSET( buffer, 0, sizeof( png_bytep ) * Image->Height );
		for( i = 0; i < Image->Height; i++ )
		{
			buffer[i] = (png_bytep)DXALLOC( sizeof( png_byte ) * ( Image->ColorData.AlphaWidth ? 4 : 3 ) * Image->Width );
			if( buffer[i] == NULL ) goto ERR;
			sample = buffer[i];
			if( Image->ColorData.AlphaWidth )
			{
				for( j = 0; j < Image->Width; j ++, sample += 4 )
				{
					NS_GetPixelBaseImage( Image, j, i, &r, &g, &b, &a );
					sample[0] = (png_byte)r;
					sample[1] = (png_byte)g;
					sample[2] = (png_byte)b;
					sample[3] = (png_byte)a;
				}
			}
			else
			{
				for( j = 0; j < Image->Width; j ++, sample += 3 )
				{
					NS_GetPixelBaseImage( Image, j, i, &r, &g, &b, &a );
					sample[0] = (png_byte)r;
					sample[1] = (png_byte)g;
					sample[2] = (png_byte)b;
				}
			}
		}
	}

	// イメージの書き出し
	png_write_image( png_ptr, buffer );

	// フッダ部分の書き出し
	png_write_end( png_ptr, info_ptr );

	// 構造体の後始末
	png_destroy_write_struct( &png_ptr, &info_ptr );

	// ファイルを閉じる
	fclose(fp);

	// メモリの解放
	for( i = 0; i < Image->Height; i++ )
		if( buffer[i] ) DXFREE( buffer[i] );
	DXFREE( buffer );

	// 正常終了
	return 0 ;
}

#endif // DX_NON_SAVEFUNCTION
#endif // DX_NON_PNGREAD








// ＪＰＥＧをメモリから読み込むためのプログラム
#ifndef DX_NON_JPEGREAD

// (殆ど jdatasrc.c の流用)
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"

// ＪＰＥＧ読みこみエラー時処理ルーチン用構造体
typedef struct my_error_mgr {
	struct jpeg_error_mgr pub;	// 標準エラーデータ

	jmp_buf setjmp_buffer;		// ロングジャンプ用構造体
} *my_error_ptr ;

// エラー時に呼ばれる関数
void my_error_exit( j_common_ptr cinfo )
{
	// cinfo->errが示す標準エラーデータの先頭アドレスをmy_error_mgr構造体の先頭アドレスに変換
	my_error_ptr myerr = (my_error_ptr) cinfo->err;

	// すいませんよくわかりません、とりあえずエラーメッセージ標準関数？
	(*cinfo->err->output_message) (cinfo);

	// ユーザーが決めたエラー処理コードの位置まで飛ぶ
	longjmp( myerr->setjmp_buffer, 1 ) ;
}


// 汎用データ形式の転送用構造体
typedef struct
{
	struct jpeg_source_mgr pub;		/* public fields */

	JOCTET *buffer;					/* start of buffer */
	boolean start_of_file;			/* have we gotten any data yet? */

	STREAMDATA *Data ;				// 汎用データ形式読み込み処理用構造体
	int DataSize ;					// データのサイズ
} my_source_general_mgr;

typedef my_source_general_mgr	*my_src_general_ptr;

#define INPUT_BUF_SIZE		(4096)	// 作業用バッファのサイズ

// 読み込むソースを初期化する関数
METHODDEF(void)
init_source_general( j_decompress_ptr cinfo )
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;

	// ファイル読み込み開始のフラグを立てる
	src->start_of_file = TRUE;
}

// データバッファにデータを転送する
METHODDEF(boolean)
fill_input_buffer_general (j_decompress_ptr cinfo)
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;
	size_t nbytes;

	// 転送するデータの量をコピーする
	nbytes = ( size_t )( ( src->DataSize - STTELL( src->Data ) < INPUT_BUF_SIZE ) ?
			 src->DataSize - STTELL( src->Data ) : INPUT_BUF_SIZE ) ;
	if( nbytes != 0 ) STREAD( src->buffer, nbytes, 1, src->Data ) ;

	// 読み込みに失敗したらエラー
	if( nbytes <= 0 )
	{
		if (src->start_of_file)	/* Treat empty input file as fatal error */
			ERREXIT(cinfo, JERR_INPUT_EMPTY);
		WARNMS(cinfo, JWRN_JPEG_EOF);

		/* Insert a fake EOI marker */
	    src->buffer[0] = (JOCTET) 0xFF;
		src->buffer[1] = (JOCTET) JPEG_EOI;
		nbytes = 2;
	}

	// その他の処理
	src->pub.next_input_byte = src->buffer;
	src->pub.bytes_in_buffer = nbytes;
	src->start_of_file = FALSE;

	return TRUE;
}

// 指定されたサイズのデータをスキップする処理
METHODDEF(void)
skip_input_data_general( j_decompress_ptr cinfo, long num_bytes)
{
	my_src_general_ptr src = (my_src_general_ptr) cinfo->src;

	// データスキップ処理
	if( num_bytes > 0 )
	{
		while( num_bytes > (long) src->pub.bytes_in_buffer )
		{
			num_bytes -= (long) src->pub.bytes_in_buffer;
			(void) fill_input_buffer_general( cinfo ) ;
		}
		src->pub.next_input_byte += (size_t) num_bytes;
		src->pub.bytes_in_buffer -= (size_t) num_bytes;
	}
}

// データを閉じるときの処理
METHODDEF(void)
term_source_general( j_decompress_ptr /*cinfo*/ )
{
  /* no work necessary here */
}

// 汎用データ読み込み処理からデータを読み込むようにする実際の設定を行う関数
GLOBAL(void)
jpeg_general_src (j_decompress_ptr cinfo, STREAMDATA *Data )
{
	my_src_general_ptr src;

	// まだＪＰＥＧデータを一時的に格納するバッファを確保していなかったら確保する
	if (cinfo->src == NULL)
	{
		/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
						(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
						SIZEOF(my_source_general_mgr));
		src = (my_src_general_ptr) cinfo->src;
		src->buffer = (JOCTET *)
					(*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
					INPUT_BUF_SIZE * SIZEOF(JOCTET));
	}

	// 関数ポインタなどをセットする
	src = (my_src_general_ptr) cinfo->src;
	src->pub.init_source			= init_source_general ;
	src->pub.fill_input_buffer		= fill_input_buffer_general ;
	src->pub.skip_input_data		= skip_input_data_general ;
	src->pub.resync_to_restart		= jpeg_resync_to_restart ; /* use default method */
	src->pub.term_source			= term_source_general ;

	src->Data = Data ;

	// 現在のファイルポインタから終端までのサイズを取得する
	{
		long pos ;
		pos = ( long )STTELL( src->Data ) ;
		STSEEK( src->Data, 0, STREAM_SEEKTYPE_END ) ;
		src->DataSize = ( int )STTELL( src->Data ) - pos ;
		STSEEK( src->Data, pos, STREAM_SEEKTYPE_SET ) ;
	}

	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

#ifdef DX_TEST	// テスト用
extern LONGLONG time2;
#endif

// ＪＰＥＧ画像の読みこみ
extern int LoadJpegImage( STREAMDATA *Src, BASEIMAGE *Image )
{
	struct jpeg_decompress_struct cinfo ;
	struct my_error_mgr jerr ;
	JSAMPARRAY buffer ;
	void *GraphPoint ;
	int InPitch ;
	int i ;
	int imgSize ;
	unsigned char *pImg ;
	int OutPitch ;
	unsigned char Check ;

	// 先頭の１バイトが 0xFF ではなかったらJPEGファイルではない
	STREAD( &Check, 1, 1, Src ) ;
	STSEEK( Src, 0, STREAM_SEEKTYPE_SET ) ;
	if( Check != 0xff )
	{
		return -1 ;
	}
	
	// 通常ＪＰＥＧエラールーチンのセットアップ
	_MEMSET( &cinfo, 0, sizeof( cinfo ) );
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if( setjmp( jerr.setjmp_buffer ) )
	{
		jpeg_destroy_decompress( &cinfo );
		return -1;
	}

#ifdef DX_TEST	// テスト用
	time2 = NS_GetNowHiPerformanceCount();
#endif

	// cinfo情報のアロケートと初期化を行う
	jpeg_create_decompress(&cinfo);

	// 汎用データ読み込み処理からデータを読み込む設定を行う
	jpeg_general_src( &cinfo, Src ) ;

	// ＪＰＥＧファイルのパラメータ情報の読みこみ
	(void)jpeg_read_header(&cinfo, TRUE);

	// ＪＰＥＧファイルの解凍の開始
	(void)jpeg_start_decompress(&cinfo);

	// １ライン当たりのデータバイト数を計算
	InPitch = cinfo.output_width * cinfo.output_components ;
	OutPitch = cinfo.output_width * cinfo.output_components ;

	// データバッファの確保
	buffer = (*cinfo.mem->alloc_sarray)
				((j_common_ptr) &cinfo, JPOOL_IMAGE, InPitch, 1 );

	// 出力イメージデータサイズのセット、及びデータ領域の確保
	imgSize = cinfo.output_height * OutPitch ;
	if( ( GraphPoint = ( unsigned char * ) DXALLOC( imgSize ) ) == NULL )
	{
		jpeg_destroy_decompress( &cinfo );
		return -1 ;
	}
	
	// 画像データの読みこみ
	pImg = ( unsigned char * )GraphPoint ; 
	while( cinfo.output_scanline < cinfo.output_height )
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);

		// データを出力データに変換して、またはそのまま転送
		for( i = 0 ; i < InPitch ; )
		{
			if ( cinfo.output_components == 1 )
			{
				*pImg = *( buffer[0] + i ) ; pImg ++ ;
				i ++ ;
			}
			else
			{
				*pImg = *( buffer[0] + i + 2 ) ; pImg ++ ;
				*pImg = *( buffer[0] + i + 1 ) ; pImg ++ ;
				*pImg = *( buffer[0] + i     ) ; pImg ++ ;
				i += 3 ;
			}
		}
	}

	// 解凍処理の終了
	(void) jpeg_finish_decompress(&cinfo);

	// BASEIMAGE 構造体のデータを詰める
	{
		Image->Width = cinfo.output_width ;
		Image->Height = cinfo.output_height ;
		Image->Pitch = OutPitch ;
		Image->GraphData = GraphPoint ;

		// カラー情報をセットする
		if( cinfo.output_components == 1 )
			NS_CreateGrayColorData( &Image->ColorData ) ;
		else
			NS_CreateFullColorData( &Image->ColorData ) ;
	}

	// cinfo構造体の解放
	jpeg_destroy_decompress(&cinfo);

#ifdef DX_TEST	// テスト用
	time2 = NS_GetNowHiPerformanceCount() - time2 ;
#endif

	// 終了
	return 0 ;
}

#ifndef DX_NON_SAVEFUNCTION

extern int SaveBaseImageToJpegBase( const DXWCHAR *pFilePathW, const char *pFilePathA, BASEIMAGE *Image, int Quality, int /*Sample2x1*/ )
{
	struct jpeg_compress_struct cinfo ;
	struct my_error_mgr jerr ;
	FILE *fp = 0;
	JSAMPARRAY buffer = 0;
	JSAMPROW sample;
	int i, j, r, g, b, a;

	// 保存用のファイルを開く
	if( pFilePathW )
	{
		fp = _wfopen( ( wchar_t * )pFilePathW, L"wb");
	}
	else
	{
		fp = fopen( pFilePathA, "wb");
	}
	if( fp == NULL ) return -1;

#ifdef DX_TEST	// テスト用
	time2 = NS_GetNowHiPerformanceCount();
#endif

	// 通常ＪＰＥＧエラールーチンのセットアップ
	_MEMSET( &cinfo, 0, sizeof( cinfo ) );
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if( setjmp( jerr.setjmp_buffer ) )
	{
ERR:
		if( buffer )
		{
			for( i = 0; i < Image->Height; i++ )
				if( buffer[i] ) DXFREE( buffer[i] );
			DXFREE( buffer );
		}

		if( fp ) fclose( fp );
		jpeg_destroy_compress( &cinfo );
		return -1;
	}

	// cinfo情報のアロケートと初期化を行う
	jpeg_create_compress( &cinfo );

	// 出力ファイルのポインタをセット
	jpeg_stdio_dest( &cinfo, fp );

	// 出力画像の情報を設定する
	cinfo.image_width      = Image->Width;
	cinfo.image_height     = Image->Height;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;
	jpeg_set_defaults( &cinfo );

	// 画質を設定
	jpeg_set_quality( &cinfo, Quality, TRUE );

	// 圧縮処理開始
	jpeg_start_compress( &cinfo, TRUE );

	// 圧縮用データの用意
	{
		// バッファの確保
		buffer = (JSAMPARRAY)DXALLOC( sizeof( JSAMPROW ) * Image->Height );
		if( buffer == NULL ) goto ERR;
		_MEMSET( buffer, 0, sizeof( JSAMPROW ) * Image->Height );
		for( i = 0; i < Image->Height; i++ )
		{
			buffer[i] = (JSAMPROW)DXALLOC( sizeof( JSAMPLE ) * 3 * Image->Width );
			if( buffer[i] == NULL ) goto ERR;
			sample = buffer[i];
			for( j = 0; j < Image->Width; j ++, sample += 3 )
			{
				NS_GetPixelBaseImage( Image, j, i, &r, &g, &b, &a );
				sample[0] = (JSAMPLE)r;
				sample[1] = (JSAMPLE)g;
				sample[2] = (JSAMPLE)b;
			}
		}
	}

	// 圧縮
	jpeg_write_scanlines( &cinfo, buffer, Image->Height );

	// 圧縮終了
	jpeg_finish_compress( &cinfo );

	// 後始末
	jpeg_destroy_compress( &cinfo );

#ifdef DX_TEST	// テスト用
	time2 = NS_GetNowHiPerformanceCount() - time2 ;
#endif

	// ファイルを閉じる
	fclose( fp );

	// データの解放
	for( i = 0; i < Image->Height; i++ )
		DXFREE( buffer[i] );
	DXFREE( buffer );

	// 正常終了
	return 0;
}

#endif // DX_NON_SAVEFUNCTION

#endif // DX_NON_JPEGREAD






#ifndef DX_NON_MERSENNE_TWISTER

//=====================================================================================
//   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
// 
// 下記のプログラムは松本様と西村様が作成されたプログラムを元に SYN様 が
// ＭＭＸを使用する改良を行われた Mersenne Twister 法疑似乱数発生プログラムを
// 更に BorlandC++ Compiler でもコンパイル出来るようにしたバージョン

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

/* Static member */
int mti;                   /* index number */
unsigned long mt[N + 1];   /* the array for the state vector */
unsigned long mtr[N];      /* the array for the random number */
unsigned long bInitialized = 0;
unsigned long bMMX = 0;

/* Prototype */
#ifndef DX_NON_INLINE_ASM
unsigned long CheckMMX(void);
#endif
void srandMT(unsigned long seed);
#ifndef DX_NON_INLINE_ASM
void generateMT(void);
#else
void generateMT_C(void);
#endif
unsigned long randMT(void);


void srandMT(unsigned long seed)
{
    int i;

    for(i = 0; i < N; i++){
         mt[i] = seed & 0xffff0000;
         seed = 69069 * seed + 1;
         mt[i] |= (seed & 0xffff0000) >> 16;
         seed = 69069 * seed + 1;
    }

#ifndef DX_NON_INLINE_ASM
    bMMX = CheckMMX();
#else
	bMMX = 0;
#endif
    bInitialized = 1;
#ifndef DX_NON_INLINE_ASM
    generateMT();
#else
    generateMT_C();
#endif
}

#ifndef DX_NON_INLINE_ASM

#ifdef __BCC
	#define cpuid __emit__(0xf) ; __emit__(0xa2) ;
#else
	#define cpuid __asm __emit 0fh __asm __emit 0a2h
#endif

unsigned long CheckMMX(void)
{
    unsigned long flag = 0;
	
    _asm{
        push    edx
        push    ecx
        pushfd
        pop     eax
        xor     eax, 00200000h
        push    eax
        popfd
        pushfd
        pop     ebx
        cmp     eax, ebx
        jnz     non_mmx
        mov     eax, 0
	}
//		db 0fh
//		db 0a2h
        cpuid
	__asm{
        cmp     eax, 0
        jz      non_mmx
        mov     eax, 1
	}
//		db 0fh
//		db 0a2h
	    cpuid
    __asm{
        and     edx, 00800000h
        jz      non_mmx
        mov     flag, 1
        non_mmx:
        pop     ecx
        pop     edx
    }

    return(flag);
}

/* MMX version */
void MMX_generateMT(void)
{
    _asm{
#ifdef __BCC
//PMMX // Borland C++ 5.6.4 for Win32 Copyright (c) 1993, 2002 Borland	では要らなくなったようです
#endif
        mov         eax, MATRIX_A
        movd        mm4, eax
        punpckldq   mm4, mm4
        mov         eax, 1
        movd        mm5, eax
        punpckldq   mm5, mm5
        movq        mm6, mm5
        psllq       mm6, 31     /* UPPER_MASK */
        movq        mm7, mm6
        psubd       mm7, mm5    /* LOWER_MASK */

        lea         esi, mt
        add         esi, ((N-M)/2)*8
        mov         edi, esi
        add         edi, M*4
        
        mov         ecx, -((N-M)/2)
        /* 8clocks */
        movq        mm0, [esi+ecx*8]
        pand        mm0, mm6
        movq        mm1, [esi+ecx*8+4]
        movq        mm2, [edi+ecx*8]
        pand        mm1, mm7
    loop0:
        por         mm0, mm1
        movq        mm3, mm1
        psrld       mm0, 1
        pand        mm3, mm5
        pxor        mm2, mm0
        psubd       mm3, mm5
        movq        mm0, [esi+ecx*8+8]
        pandn       mm3, mm4
        movq        mm1, [esi+ecx*8+12]
        pxor        mm3, mm2
        movq        mm2, [edi+ecx*8+8]
        pand        mm0, mm6
        movq        [esi+ecx*8], mm3
        pand        mm1, mm7
        inc         ecx
        jnz         loop0
        /**/
        mov         eax, [esi]
        mov         ebx, [esi+4]
        and         eax, UPPER_MASK
        and         ebx, LOWER_MASK
        mov         edx, [edi]
        or          eax, ebx
        shr         eax, 1
        or          ebx, 0xfffffffe
        xor         eax, edx
        add         ebx, 1
        xor         eax, MATRIX_A
        and         ebx, MATRIX_A
        xor         ebx, eax
        mov         [esi], ebx
        /**/

        lea         esi, mt
        mov         eax, [esi] /* mt[N] = mt[0]; */
        add         esi, N*4
        mov         [esi], eax /* mt[N] = mt[0]; */

        mov         edi, esi
        sub         edi, (N-M)*4
        add         esi, 4
        add         edi, 4
        
        mov         ecx, -((M+1)/2) /* overrun */
        /* 8clocks */
        movq        mm0, [esi+ecx*8]
        pand        mm0, mm6
        movq        mm1, [esi+ecx*8+4]
        movq        mm2, [edi+ecx*8]
        pand        mm1, mm7
    loop1:
        por         mm0, mm1
        movq        mm3, mm1
        psrld       mm0, 1
        pand        mm3, mm5
        pxor        mm2, mm0
        psubd       mm3, mm5
        movq        mm0, [esi+ecx*8+8]
        pandn       mm3, mm4
        movq        mm1, [esi+ecx*8+12]
        pxor        mm3, mm2
        movq        mm2, [edi+ecx*8+8]
        pand        mm0, mm6
        movq        [esi+ecx*8], mm3
        pand        mm1, mm7
        inc         ecx
        jnz         loop1
        /**/

        push        ebp

        mov         eax, TEMPERING_MASK_B
        movd        mm6, eax
        punpckldq   mm6, mm6
        mov         eax, TEMPERING_MASK_C
        movd        mm7, eax
        punpckldq   mm7, mm7

        lea         esi, mt
        lea         edi, mtr
        add         esi, N*4
        add         edi, N*4
        mov         ecx, -N/2
        /* 17clocks */
        movq        mm0, [esi+ecx*8]
        movq        mm1, mm0
        psrld       mm0, 11
        pxor        mm0, mm1
        movq        mm2, [esi+ecx*8+8]
    loop2:
        movq        mm1, mm0
        pslld       mm0, 7
        movq        mm3, mm2
        psrld       mm2, 11
        pand        mm0, mm6
        pxor        mm2, mm3
        pxor        mm0, mm1
        movq        mm3, mm2
        pslld       mm2, 7
        movq        mm1, mm0
        pslld       mm0, 15
        pand        mm2, mm6
        pand        mm0, mm7
        pxor        mm2, mm3
        pxor        mm0, mm1
        movq        mm3, mm2
        pslld       mm2, 15
        movq        mm1, mm0
        pand        mm2, mm7
        psrld       mm0, 18
        pxor        mm2, mm3
        pxor        mm1, mm0
        movq        mm0, [esi+ecx*8+16]
        movq        mm3, mm2
        movq        [edi+ecx*8], mm1
        psrld       mm2, 18
        movq        mm1, mm0
        pxor        mm3, mm2
        movq        mm2, [esi+ecx*8+24]
        psrld       mm0, 11
        movq        [edi+ecx*8+8], mm3
        pxor        mm0, mm1
        add         ecx, 2
        jnz         loop2
        /**/

        pop         ebp
        emms
#ifdef __BCC
//PNOMMX // Borland C++ 5.6.4 for Win32 Copyright (c) 1993, 2002 Borland	では要らなくなったようです
#endif
    }
    mti = 0;
}

/* Non MMX version */
void generateMT(void)
{
    if(bMMX){
        MMX_generateMT();
        return;
    }

    _asm{
        lea     esi, mt
        add     esi, (N-M)*4
        mov     edi, esi
        add     edi, M*4
        
        mov     ecx, -(N-M)
        /* 8clocks */
        mov     eax, [esi+ecx*4]
        mov     ebx, [esi+ecx*4+4]
    loop0_a:
        and     eax, UPPER_MASK
        and     ebx, LOWER_MASK
        mov     edx, [edi+ecx*4]
        or      eax, ebx
        shr     eax, 1
        or      ebx, 0xfffffffe
        xor     eax, edx
        add     ebx, 1
        xor     eax, MATRIX_A
        and     ebx, MATRIX_A
        xor     ebx, eax
        mov     eax, [esi+ecx*4+4]
        mov     [esi+ecx*4], ebx
        mov     ebx, [esi+ecx*4+8]
        inc     ecx
        jnz     loop0_a
        /**/

        lea     esi, mt
        mov     eax, [esi] /* mt[N] = mt[0]; */
        add     esi, N*4
        mov     [esi], eax /* mt[N] = mt[0]; */
        mov     edi, esi
        sub     edi, (N-M)*4
        
        mov     ecx, -M
        /* 8clocks */
        mov     eax, [esi+ecx*4]
        mov     ebx, [esi+ecx*4+4]
    loop1_a:
        and     eax, UPPER_MASK
        and     ebx, LOWER_MASK
        mov     edx, [edi+ecx*4]
        or      eax, ebx
        shr     eax, 1
        or      ebx, 0xfffffffe
        xor     eax, edx
        add     ebx, 1
        xor     eax, MATRIX_A
        and     ebx, MATRIX_A
        xor     ebx, eax
        mov     eax, [esi+ecx*4+4]
        mov     [esi+ecx*4], ebx
        mov     ebx, [esi+ecx*4+8]
        inc     ecx
        jnz     loop1_a
        /**/

        push    ebp

        lea     esi, mt
        lea     edi, mtr
        add     esi, N*4
        add     edi, N*4
        mov     ecx, -N/2
        /* 17clocks */
        mov     eax, [esi+ecx*8]
    loop2_a:
        mov     edx, [esi+ecx*8+4]
        mov     ebx, eax
        shr     eax, 11
        mov     ebp, edx
        shr     edx, 11
        xor     eax, ebx
        xor     edx, ebp
        mov     ebx, eax
        shl     eax, 7
        mov     ebp, edx
        shl     edx, 7
        and     eax, TEMPERING_MASK_B
        and     edx, TEMPERING_MASK_B
        xor     eax, ebx
        xor     edx, ebp
        mov     ebx, eax
        shl     eax, 15
        mov     ebp, edx
        shl     edx, 15
        and     eax, TEMPERING_MASK_C
        and     edx, TEMPERING_MASK_C
        xor     eax, ebx
        xor     edx, ebp
        mov     ebx, eax
        shr     eax, 18
        mov     ebp, edx
        shr     edx, 18
        xor     eax, ebx
        xor     edx, ebp
        mov     [edi+ecx*8],   eax
        mov     [edi+ecx*8+4], edx
        mov     eax, [esi+ecx*8+8]
        inc     ecx
        jnz     loop2_a
        /**/

        pop     ebp
    }
    mti = 0;
}

#else

/* C version */
void generateMT_C(void)
{
    int kk;
    unsigned long y;
    static unsigned long mag01[2] = {0x0, MATRIX_A}; /* mag01[x] = x * MATRIX_A  for x=0,1 */
    
    for(kk = 0; kk < N - M; kk++){
        y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
        mt[kk] = mt[kk + M] ^ (y >> 1) ^ mag01[y & 0x1];
    }

    mt[N] = mt[0];

    for(; kk < N; kk++){
        y = (mt[kk] & UPPER_MASK) | (mt[kk + 1] & LOWER_MASK);
        mt[kk] = mt[kk + (M - N)] ^ (y >> 1) ^ mag01[y & 0x1];
    }

    for(kk = 0; kk < N; kk++){
        y = mt[kk];
        y ^= TEMPERING_SHIFT_U(y);
        y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
        y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
        y ^= TEMPERING_SHIFT_L(y);
        mtr[kk] = y;
    }
    mti = 0;
}

#endif


#ifdef __inline
__inline unsigned long randMT(void)
#else
unsigned long randMT(void)
#endif
{
    if(mti >= N){
        if(!bInitialized) srandMT(4357);
#ifndef DX_NON_INLINE_ASM
        generateMT();
#else
		generateMT_C();
#endif
    }
    return mtr[mti++]; 
}
//=============================================================================

// 获取随机数

// 乱数の初期値を設定する
extern int NS_SRand( int Seed )
{
	// 初期値セット
	srandMT( ( unsigned long )Seed ) ;

	// 終了
	return 0 ;
}

// 乱数を取得する( RandMax : 返って来る値の最大値 )
extern int NS_GetRand( int RandMax )
{
	int Result ;

	RandMax ++ ;
	Result = ( int )( ( ( LONGLONG )randMT() * ( LONGLONG )RandMax ) >> 32 ) ;

	return Result ;
}

#else // DX_NON_MERSENNE_TWISTER

// 乱数の初期値を設定する
extern int NS_SRand( int Seed )
{
	// 初期値セット
	srand( Seed ) ;

	// 終了
	return 0 ;
}

// 乱数を取得する( RandMax : 返って来る値の最大値 )
extern int NS_GetRand( int RandMax )
{
	int Result ;

	RandMax ++ ;
	Result = ( int )( ( ( LONGLONG )rand() * ( LONGLONG )RandMax ) / ( RAND_MAX + 1 ) ) ;

	return Result ;
}

#endif // DX_NON_MERSENNE_TWISTER









//#define CHECKMULTIBYTECHAR(CP)		(( (unsigned char)*(CP) >= 0x81 && (unsigned char)*(CP) <= 0x9F ) || ( (unsigned char)*(CP) >= 0xE0 && (unsigned char)*(CP) <= 0xFC ))	// TRUE:２バイト文字  FALSE:１バイト文字

static void FileRead_SkipSpace( DWORD_PTR FileHandle, int *Eof )
{
	char c;
	size_t res;

	if( Eof ) *Eof = FALSE;
	for(;;)
	{
		res = FREAD( &c, sizeof( char ), 1, FileHandle );
		FSYNC( FileHandle )
		if( res == 0 )
		{
			if( Eof ) *Eof = TRUE;
			return;
		}
		if( c != ' ' && c != '\n' && c != '\r' ) break;
	}
	FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
}

// ファイルから書式化されたデータを読み出す
extern int FileRead_scanf_base( DWORD_PTR FileHandle, const char *Format, va_list Param )
{
	char c, c2[2], tstr[512], str[256], Number[128], Number2[128], Number3[128], VStr[1024];
	int ReadNum, Width, i, j, k, num, num2, num3;
	int SkipFlag, VStrRFlag, I64Flag, lFlag, hFlag, Eof, MinusFlag, UIntFlag;
	int TenFlag, SisuuFlag, MinusFlag2 ;
	size_t res;
	LONGLONG int64Num, int64Count;
	double doubleNum, doubleNum2, doubleNum3, doubleCount;
	char *pStr;
	DXWCHAR *pStrW ;

	if( FEOF( FileHandle ) ) return EOF;

	ReadNum = 0;
	Eof = FALSE;
	while( *Format != '\0' && FEOF( FileHandle ) == 0 )
	{
		if( Format[0] == '%' && Format[1] != '%' )
		{
			Width = -1;
			I64Flag = FALSE;
			lFlag = FALSE;
			hFlag = FALSE;
			UIntFlag = FALSE;
			SkipFlag = FALSE;
			Format ++ ;
			if( *Format == '\0' ) break;
			if( *Format == '*' )
			{
				SkipFlag = TRUE;
				Format ++ ;
			}
			if( *Format == '\0' ) break;
			if( *Format >= '0' && *Format <= '9' )
			{
				for( i = 0; Format[i] >= '0' && Format[i] <= '9'; i++ )
					str[i] = Format[i];
				str[i] = '\0';
				Format += i;
				Width = _ATOI( str );
				if( Width == 0 ) break;
			}
			if( *Format == '\0' ) break;
			switch( *Format )
			{
			case 'l': case 'H': lFlag = TRUE; Format++ ; break;
			case 'h': case 'L': hFlag = TRUE; Format++ ; break;
			case 'I':
				if( Format[1] == '6' && Format[2] == '4' )
				{
					I64Flag = TRUE;
					Format += 3;
				}
				break;
			}
			if( *Format == '\0' ) break;

			if( *Format == '[' )
			{
				if( lFlag || hFlag || I64Flag ) break;

				Format ++ ;
				VStrRFlag = FALSE;
				if( *Format == '^' )
				{
					VStrRFlag = TRUE;
					Format++;
				}
				j = 0;
				_MEMSET( VStr, 0, sizeof( VStr ) );
				c = '[';
				while( *Format != '\0' && *Format != ']' )
				{
					if( CheckMultiByteChar( *Format, _GET_CHARSET() ) == TRUE )
					{
						if( Format[1] == '\0' )
						{
							Format++;
							break;
						}
						if( Format[1] == ']' ) break;
						VStr[j]   = Format[0];
						VStr[j+1] = Format[1];
						j += 2 ;
						Format += 2 ;
						c = '[';
					}
					else
					{
						if( *Format == '-' && c != '[' && Format[1] != '\0' && Format[1] != ']' )
						{
							num  = (int)(unsigned char)c;
							num2 = (int)(unsigned char)Format[1];
							if( num2 < num )
							{
								k = num2; num2 = num; num = k;
							}
							for( k = num; k <= num2; k++ )
							{
								if( c != k )
								{
									*((unsigned char *)&VStr[j]) = (unsigned char)k;
									j++;
								}
							}
							Format += 2;
							c = '[';
						}
						else
						{
							VStr[j] = *Format;
							c = *Format;
							j ++ ;
							Format ++ ;
						}
					}
				}
				if( *Format == '\0' ) break;
				Format ++ ;
				pStr = NULL;
				if( SkipFlag == FALSE )
				{
					pStr = va_arg( Param, char * );
				}
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;
				i = 0;
				for(;;)
				{
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( CheckMultiByteChar( c, _GET_CHARSET() ) == TRUE )
					{
						c2[0] = c;
						res = FREAD( &c, sizeof( char ), 1, FileHandle );
						FSYNC( FileHandle )
						if( res == 0 )
						{
							Eof = TRUE;
							break;
						}
						c2[1] = c;

						j = 0;
						while( VStr[j] != '\0' )
						{
							if( CheckMultiByteChar( VStr[j], _GET_CHARSET() ) == TRUE )
							{
								if( VStr[j] == c2[0] && VStr[j+1] == c2[1] ) break;
								j += 2;
							}
							else
							{
								j ++ ;
							}
						}

						if( ( VStrRFlag == TRUE  && VStr[j] != '\0' ) || ( VStrRFlag == FALSE && VStr[j] == '\0' ) )
						{
							FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
							break;
						}

						if( Width - i == 1 )
						{
							FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
							if( pStr )
							{
								pStr[0] = c2[0];
								pStr ++ ;
							}
							i ++ ;
						}
						else
						{
							if( pStr )
							{
								pStr[0] = c2[0];
								pStr[1] = c2[1];
								pStr += 2 ;
							}
							i += 2 ;
						}
					}
					else
					{
						j = 0;
						while( VStr[j] != '\0' )
						{
							if( CheckMultiByteChar( VStr[j], _GET_CHARSET() ) == TRUE )
							{
								j += 2;
							}
							else
							{
								if( VStr[j] == c ) break;
								j ++ ;
							}
						}

						if( ( VStrRFlag == TRUE  && VStr[j] != '\0' ) || ( VStrRFlag == FALSE && VStr[j] == '\0' ) ) break;
						if( pStr )
						{
							*pStr = c;
							pStr ++ ;
						}
						i ++ ;
					}
					if( Width != 0 && Width == i ) break;
				}
				if( pStr ) *pStr = '\0';
				if( Eof == FALSE && Width != i )
				{
					FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
				}
			}
			else if( *Format == 'd' || *Format == 'D' || *Format == 'u' || *Format == 'U' )
			{
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				UIntFlag = *Format == 'u' || *Format == 'U';
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( char ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == '-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
STR_10INT:
				for( i = 0; i < 127 && ( Width == -1 || Width != i ) && c >= '0' && c <= '9'; i ++ )
				{
					Number[i] = c - '0';
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
				}
				num = i;
				if( num == 0 ) break;
				int64Count = 1;
				int64Num = 0;
				for( i = num - 1; i >= 0; i --, int64Count *= 10 )
				{
					if( UIntFlag )
						int64Num = (int)( (ULONGLONG)int64Num + (ULONGLONG)Number[i] * (ULONGLONG)int64Count );
					else 
						int64Num += (LONGLONG)Number[i] * int64Count;
				}
				if( MinusFlag ) int64Num = -int64Num;
				if( SkipFlag == FALSE )
				{
					if( I64Flag )
					{
						if( UIntFlag ) *va_arg( Param,      ULONGLONG * ) =      (ULONGLONG)int64Num;
						else           *va_arg( Param,       LONGLONG * ) =       (LONGLONG)int64Num;
					}
					else if( hFlag )
					{
						if( UIntFlag ) *va_arg( Param, unsigned short * ) = (unsigned short)int64Num;
						else           *va_arg( Param,          short * ) =          (short)int64Num;
					}
					else
					{
						if( UIntFlag ) *va_arg( Param,   unsigned int * ) =   (unsigned int)int64Num;
						else           *va_arg( Param,            int * ) =            (int)int64Num;
					}
				}
			}
			else if( *Format == 'x' || *Format == 'X' )
			{
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( char ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == '-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
STR_16INT:
				i = 0;
				for(;;)
				{
					if( i >= 127 ) break;
					if( Width != -1 && Width == i ) break;
					else if( c >= '0' && c <= '9' ) Number[i] = c - '0';
					else if( c >= 'a' && c <= 'f' ) Number[i] = c - 'a' + 10;
					else if( c >= 'A' && c <= 'F' ) Number[i] = c - 'A' + 10;
					else break;
					i ++ ;
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
				}
				num = i;
				if( num == 0 ) break;
				int64Count = 1;
				int64Num = 0;
				for( i = num - 1; i >= 0; i --, int64Count *= 16 )
				{
					int64Num += Number[i] * int64Count;
				}
				if( MinusFlag ) int64Num = -int64Num;
				if( SkipFlag == FALSE )
				{
					if( I64Flag )    *va_arg( Param, LONGLONG * ) = (LONGLONG)int64Num;
					else if( hFlag ) *va_arg( Param, short *    ) =    (short)int64Num;
					else             *va_arg( Param, int *      ) =      (int)int64Num;
				}
			}
			else if( *Format == 'o' || *Format == 'O' )
			{
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( char ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == '-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
STR_8INT:
				for( i = 0; i < 127 && ( Width == -1 || Width != i ) && c >= '0' && c <= '7'; i ++ )
				{
					Number[i] = c - '0';
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
				}
				num = i;
				if( num == 0 ) break;
				int64Count = 1;
				int64Num = 0;
				for( i = num - 1; i >= 0; i --, int64Count *= 8 )
				{
					int64Num += Number[i] * int64Count;
				}
				if( MinusFlag ) int64Num = -int64Num;
				if( SkipFlag == FALSE )
				{
					if( I64Flag )    *va_arg( Param, LONGLONG * ) = (LONGLONG)int64Num;
					else if( hFlag ) *va_arg( Param, short *    ) =    (short)int64Num;
					else             *va_arg( Param, int *      ) =      (int)int64Num;
				}
			}
			else if( *Format == 'i' || *Format == 'I' )
			{
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( char ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == '-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( c == '0' )
				{
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( c == 'x' )
					{
						if( Width != -1 )
						{
							Width -- ;
							if( Width == 0 ) break;
						}
						res = FREAD( &c, sizeof( char ), 1, FileHandle );
						FSYNC( FileHandle )
						if( res == 0 )
						{
							Eof = TRUE;
							break;
						}
						goto STR_16INT;
					}
					else
					{
						goto STR_8INT;
					}
				}
				else
				{
					goto STR_10INT;
				}
			}
			else if( *Format == 'c' || *Format == 'C' )
			{
				Format ++ ;
				if( Width == -1 ) Width = 1;
				pStr = NULL;
				if( SkipFlag == FALSE )
				{
					if( Format[ -1 ] == 'c' )
					{
						pStr = va_arg( Param, char * );
					}
					else
					{
						pStrW = va_arg( Param, DXWCHAR * );
					}
				}
				for( i = 0; i < Width; i ++ )
				{
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					tstr[ i ] = c ;
				}
				tstr[ i ] = '\0' ;
				if( pStr )
				{
					_STRCPY( pStr, tstr ) ;
				}
				if( pStrW )
				{
					MBCharToWChar( _GET_CODEPAGE(), tstr, pStrW, 1024 ) ;
				}
			}
			else if( *Format == 's' || *Format == 'S' )
			{
				Format ++ ;
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				pStr = NULL;
				pStrW = NULL;
				if( SkipFlag == FALSE )
				{
					if( Format[ -1 ] == 's' )
					{
						pStr = va_arg( Param, char * );
					}
					else
					{
						pStrW = va_arg( Param, DXWCHAR * );
					}
				}

				i = 0;
				for(;;)
				{
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE; 
						break;
					}
					if( CheckMultiByteChar( c, _GET_CHARSET() ) == FALSE && ( c == ' ' || c == '\n' || c == '\r' ) )
					{
						FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
						break;
					}
					tstr[ i ] = c ;
					i ++ ;
					if( Width != -1 && i >= Width ) break;

					if( CheckMultiByteChar( c, _GET_CHARSET() ) == TRUE )
					{
						res = FREAD( &c, sizeof( char ), 1, FileHandle );
						FSYNC( FileHandle )
						if( res == 0 )
						{
							Eof = TRUE; 
							break;
						}
						tstr[ i ] = c ;
						i ++ ;
						if( Width != -1 && i >= Width ) break;
					}
				}
				tstr[ i ] = '\0' ;
				if( pStr )
				{
					_STRCPY( pStr, tstr ) ;
				}
				if( pStrW )
				{
					MBCharToWChar( _GET_CODEPAGE(), tstr, pStrW, 1024 ) ;
				}
			}
			else if( *Format == 'f' || *Format == 'F' || *Format == 'g' || *Format == 'G' )
			{
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				TenFlag = FALSE;
				SisuuFlag = FALSE;
				MinusFlag2 = FALSE;
				res = FREAD( &c, sizeof( char ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == '-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}

				i = 0;	//自然数
				j = 0;	//小数
				k = 0;	//指数
				for(;;)
				{
					if( c == '.' )
					{
						if( TenFlag ) break;
						TenFlag = TRUE;
					}
					else if( c == 'e' || c == 'E' )
					{
						if( SisuuFlag ) break;
						SisuuFlag = TRUE;
					}
					else if( c == '-' || c == '+' )
					{
						if( SisuuFlag == FALSE || k != 0 ) break;
						if( c == '-' ) MinusFlag2 = TRUE;
					}
					else if( c >= '0' && c <= '9' )
					{
						if( SisuuFlag )
						{
							if( k >= 127 ) break ;
							Number3[k] = c - '0';
							k ++ ;
						}
						else if( TenFlag )
						{
							if( j >= 127 ) break ;
							Number2[j] = c - '0';
							j ++ ;
						}
						else
						{
							if( i >= 127 ) break ;
							Number[i] = c - '0';
							i ++ ;
						}
					}
					else break;

					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( char ), SEEK_CUR );
				}
				if( i == 0 && j == 0 ) break;
				num = i;
				num2 = j;
				num3 = k;
				if( num == 0 && num2 == 0 ) break;

				doubleCount = 1.0;
				doubleNum = 0;
				for( i = num - 1; i >= 0; i --, doubleCount *= 10.0 )
				{
					if( Number[i] != 0 )
						doubleNum += Number[i] * doubleCount;
				}
				if( MinusFlag ) doubleNum = -doubleNum;

				doubleCount = 0.1;
				doubleNum2 = 0;
				for( i = 0; i < num2; i ++, doubleCount /= 10.0 )
				{
					if( Number2[i] != 0 )
						doubleNum2 += Number2[i] * doubleCount;
				}
				if( MinusFlag ) doubleNum2 = -doubleNum2;

				int64Count = 1;
				int64Num = 0;
				for( i = num3 - 1; i >= 0; i --, int64Count *= 10 )
				{
					int64Num += Number3[i] * int64Count;
				}
				if( MinusFlag2 ) int64Num = -int64Num;

				doubleNum3 = 1.0;
				if( int64Num != 0 )
				{
					if( int64Num < 0 )
					{
						int64Num = -int64Num;
						for( i = 0; i < int64Num; i++ )
							doubleNum3 /= 10.0;
					}
					else
					{
						for( i = 0; i < int64Num; i++ )
							doubleNum3 *= 10.0;
					}
				}

				doubleNum = ( doubleNum + doubleNum2 ) * doubleNum3;

				if( SkipFlag == FALSE )
				{
					if( lFlag ) *va_arg( Param, double * ) = doubleNum;
					else        *va_arg( Param, float *  ) = (float)doubleNum;
				}
			}
			if( SkipFlag == FALSE ) ReadNum ++ ;
		}
		else
		{
			if( *Format == ' ' || *Format == '\n' || *Format == '\r' )
			{
				while( *Format != '\0' && ( *Format == ' ' || *Format == '\n' || *Format == '\r' ) ) Format ++ ;
				FileRead_SkipSpace( FileHandle, &Eof );
				if( Eof == TRUE ) break;
			}
			else
			{
				for( i = 0; *Format != '\0' && *Format != ' ' && *Format != '\n' && *Format != '\r' && *Format != '%'; i++, Format ++ )
					str[i] = *Format;
				str[i] = '\0';
				num = i;
				for( i = 0; i < num; i ++ )
				{
					res = FREAD( &c, sizeof( char ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( str[i] != c ) break;
				}
			}
		}
	}

	if( FEOF( FileHandle ) && ReadNum == 0 ) return EOF;

	return ReadNum;
}

static void FileRead_SkipSpaceW( DWORD_PTR FileHandle, int *Eof )
{
	DXWCHAR c;
	size_t res;

	if( Eof ) *Eof = FALSE;
	for(;;)
	{
		res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
		FSYNC( FileHandle )
		if( res == 0 )
		{
			if( Eof ) *Eof = TRUE;
			return;
		}
		if( c != L' ' && c != L'\n' && c != L'\r' ) break;
	}
	FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
}

// サロゲートペアかどうかの判定( TRUE:４バイト文字上位  FALSE:２バイト文字 )
static int _CheckUTF16H( DXWCHAR c )
{
	return ( DWORD )c >= 0xd800 && ( DWORD )c <= 0xdbff ? TRUE : FALSE ;
}

static void __WCSCPY( DXWCHAR *Dest, const DXWCHAR *Src )
{
	int i ;
	for( i = 0 ; Src[i] != L'\0' ; i ++ ) Dest[i] = Src[i] ;
	Dest[i] = L'\0' ;
}

// ファイルから書式化されたデータを読み出す
extern int FileRead_scanf_baseW( DWORD_PTR FileHandle, const DXWCHAR *Format, va_list Param )
{
	DXWCHAR c, c2[2], tstr[512], str[256], Number[128], Number2[128], Number3[128], VStr[1024];
	int ReadNum, Width, i, j, k, num, num2, num3;
	int SkipFlag, VStrRFlag, I64Flag, lFlag, hFlag, Eof, MinusFlag, UIntFlag;
	int TenFlag, SisuuFlag, MinusFlag2 ;
	size_t res;
	LONGLONG int64Num, int64Count;
	double doubleNum, doubleNum2, doubleNum3, doubleCount;
	DXWCHAR *pStr;
	char *pStrA ;

	if( FEOF( FileHandle ) ) return EOF;

	if( FTELL( FileHandle ) == 0 )
	{
		FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle ) ;
		FSYNC( FileHandle )
		if( *( ( WORD * )&c ) != 0xfeff )
		{
			FSEEK( FileHandle, 0, SEEK_SET ) ;
		}
	}

	ReadNum = 0;
	Eof = FALSE;
	while( *Format != L'\0' && FEOF( FileHandle ) == 0 )
	{
		if( Format[0] == L'%' && Format[1] != L'%' )
		{
			Width = -1;
			I64Flag = FALSE;
			lFlag = FALSE;
			hFlag = FALSE;
			UIntFlag = FALSE;
			SkipFlag = FALSE;
			Format ++ ;
			if( *Format == L'\0' ) break;
			if( *Format == L'*' )
			{
				SkipFlag = TRUE;
				Format ++ ;
			}
			if( *Format == L'\0' ) break;
			if( *Format >= L'0' && *Format <= L'9' )
			{
				for( i = 0; Format[i] >= L'0' && Format[i] <= L'9'; i++ )
					str[i] = Format[i];
				str[i] = L'\0';
				Format += i;
				Width = _ATOIW( str );
				if( Width == 0 ) break;
			}
			if( *Format == L'\0' ) break;
			switch( *Format )
			{
			case L'l': case L'H': lFlag = TRUE; Format++ ; break;
			case L'h': case L'L': hFlag = TRUE; Format++ ; break;
			case L'I':
				if( Format[1] == L'6' && Format[2] == L'4' )
				{
					I64Flag = TRUE;
					Format += 3;
				}
				break;
			}
			if( *Format == L'\0' ) break;

			if( *Format == L'[' )
			{
				if( lFlag || hFlag || I64Flag ) break;

				Format ++ ;
				VStrRFlag = FALSE;
				if( *Format == L'^' )
				{
					VStrRFlag = TRUE;
					Format++;
				}
				j = 0;
				_MEMSET( VStr, 0, sizeof( VStr ) );
				c = L'[';
				while( *Format != L'\0' && *Format != L']' )
				{
					if( _CheckUTF16H( *Format ) == TRUE )
					{
						if( Format[1] == L'\0' )
						{
							Format++;
							break;
						}
						if( Format[1] == L']' ) break;
						VStr[j]   = Format[0];
						VStr[j+1] = Format[1];
						j += 2 ;
						Format += 2 ;
						c = L'[';
					}
					else
					{
						if( *Format == L'-' && c != L'[' && Format[1] != L'\0' && Format[1] != L']' )
						{
							num  = (int)(unsigned short)c;
							num2 = (int)(unsigned short)Format[1];
							if( num2 < num )
							{
								k = num2; num2 = num; num = k;
							}
							for( k = num; k <= num2; k++ )
							{
								if( c != k )
								{
									*((unsigned short *)&VStr[j]) = (unsigned short)k;
									j++;
								}
							}
							Format += 2;
							c = L'[';
						}
						else
						{
							VStr[j] = *Format;
							c = *Format;
							j ++ ;
							Format ++ ;
						}
					}
				}
				if( *Format == L'\0' ) break;
				Format ++ ;
				pStr = NULL;
				if( SkipFlag == FALSE )
				{
					pStr = va_arg( Param, DXWCHAR * );
				}
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;
				i = 0;
				for(;;)
				{
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( _CheckUTF16H( c ) == TRUE )
					{
						c2[0] = c;
						res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
						FSYNC( FileHandle )
						if( res == 0 )
						{
							Eof = TRUE;
							break;
						}
						c2[1] = c;

						j = 0;
						while( VStr[j] != L'\0' )
						{
							if( _CheckUTF16H( VStr[j] ) == TRUE )
							{
								if( VStr[j] == c2[0] && VStr[j+1] == c2[1] ) break;
								j += 2;
							}
							else
							{
								j ++ ;
							}
						}

						if( ( VStrRFlag == TRUE  && VStr[j] != L'\0' ) || ( VStrRFlag == FALSE && VStr[j] == L'\0' ) )
						{
							FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
							break;
						}

						if( Width - i == 1 )
						{
							FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
							if( pStr )
							{
								pStr[0] = c2[0];
								pStr ++ ;
							}
							i ++ ;
						}
						else
						{
							if( pStr )
							{
								pStr[0] = c2[0];
								pStr[1] = c2[1];
								pStr += 2 ;
							}
							i += 2 ;
						}
					}
					else
					{
						j = 0;
						while( VStr[j] != L'\0' )
						{
							if( _CheckUTF16H( VStr[j] ) == TRUE )
							{
								j += 2;
							}
							else
							{
								if( VStr[j] == c ) break;
								j ++ ;
							}
						}

						if( ( VStrRFlag == TRUE  && VStr[j] != L'\0' ) || ( VStrRFlag == FALSE && VStr[j] == L'\0' ) ) break;
						if( pStr )
						{
							*pStr = c;
							pStr ++ ;
						}
						i ++ ;
					}
					if( Width != 0 && Width == i ) break;
				}
				if( pStr ) *pStr = L'\0';
				if( Eof == FALSE && Width != i )
				{
					FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
				}
			}
			else if( *Format == L'd' || *Format == L'D' || *Format == L'u' || *Format == L'U' )
			{
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				UIntFlag = *Format == L'u' || *Format == L'U';
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == L'-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
STR_10INT:
				for( i = 0; i < 127 && ( Width == -1 || Width != i ) && c >= L'0' && c <= L'9'; i ++ )
				{
					Number[i] = c - L'0';
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
				}
				num = i;
				if( num == 0 ) break;
				int64Count = 1;
				int64Num = 0;
				for( i = num - 1; i >= 0; i --, int64Count *= 10 )
				{
					if( UIntFlag )
						int64Num = (int)( (ULONGLONG)int64Num + (ULONGLONG)Number[i] * (ULONGLONG)int64Count );
					else 
						int64Num += (LONGLONG)Number[i] * int64Count;
				}
				if( MinusFlag ) int64Num = -int64Num;
				if( SkipFlag == FALSE )
				{
					if( I64Flag )
					{
						if( UIntFlag ) *va_arg( Param,      ULONGLONG * ) =      (ULONGLONG)int64Num;
						else           *va_arg( Param,       LONGLONG * ) =       (LONGLONG)int64Num;
					}
					else if( hFlag )
					{
						if( UIntFlag ) *va_arg( Param, unsigned short * ) = (unsigned short)int64Num;
						else           *va_arg( Param,          short * ) =          (short)int64Num;
					}
					else
					{
						if( UIntFlag ) *va_arg( Param,   unsigned int * ) =   (unsigned int)int64Num;
						else           *va_arg( Param,            int * ) =            (int)int64Num;
					}
				}
			}
			else if( *Format == L'x' || *Format == L'X' )
			{
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == L'-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
STR_16INT:
				i = 0;
				for(;;)
				{
					if( i >= 127 ) break;
					if( Width != -1 && Width == i ) break;
					else if( c >= L'0' && c <= L'9' ) Number[i] = c - L'0';
					else if( c >= L'a' && c <= L'f' ) Number[i] = c - L'a' + 10;
					else if( c >= L'A' && c <= L'F' ) Number[i] = c - L'A' + 10;
					else break;
					i ++ ;
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
				}
				num = i;
				if( num == 0 ) break;
				int64Count = 1;
				int64Num = 0;
				for( i = num - 1; i >= 0; i --, int64Count *= 16 )
				{
					int64Num += Number[i] * int64Count;
				}
				if( MinusFlag ) int64Num = -int64Num;
				if( SkipFlag == FALSE )
				{
					if( I64Flag )    *va_arg( Param, LONGLONG * ) = (LONGLONG)int64Num;
					else if( hFlag ) *va_arg( Param, short *    ) =    (short)int64Num;
					else             *va_arg( Param, int *      ) =      (int)int64Num;
				}
			}
			else if( *Format == L'o' || *Format == L'O' )
			{
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == L'-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
STR_8INT:
				for( i = 0; i < 127 && ( Width == -1 || Width != i ) && c >= L'0' && c <= L'7'; i ++ )
				{
					Number[i] = c - L'0';
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
				}
				num = i;
				if( num == 0 ) break;
				int64Count = 1;
				int64Num = 0;
				for( i = num - 1; i >= 0; i --, int64Count *= 8 )
				{
					int64Num += Number[i] * int64Count;
				}
				if( MinusFlag ) int64Num = -int64Num;
				if( SkipFlag == FALSE )
				{
					if( I64Flag )    *va_arg( Param, LONGLONG * ) = (LONGLONG)int64Num;
					else if( hFlag ) *va_arg( Param, short *    ) =    (short)int64Num;
					else             *va_arg( Param, int *      ) =      (int)int64Num;
				}
			}
			else if( *Format == L'i' || *Format == L'I' )
			{
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == L'-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}
				if( c == L'0' )
				{
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( c == L'x' )
					{
						if( Width != -1 )
						{
							Width -- ;
							if( Width == 0 ) break;
						}
						res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
						FSYNC( FileHandle )
						if( res == 0 )
						{
							Eof = TRUE;
							break;
						}
						goto STR_16INT;
					}
					else
					{
						goto STR_8INT;
					}
				}
				else
				{
					goto STR_10INT;
				}
			}
			else if( *Format == L'c' )
			{
				Format ++ ;
				if( Width == -1 ) Width = 1;
				pStr = NULL;
				pStrA = NULL;
				if( SkipFlag == FALSE )
				{
					if( Format[ -1 ] == 'c' )
					{
						pStr = va_arg( Param, DXWCHAR * );
					}
					else
					{
						pStrA = va_arg( Param, char * );
					}
				}
				for( i = 0; i < Width; i ++ )
				{
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					tstr[ i ] = c ;
				}
				tstr[ i ] = L'\0' ;
				if( pStr )
				{
					__WCSCPY( pStr, tstr ) ;
				}
				if( pStrA )
				{
					WCharToMBChar( _GET_CODEPAGE(), tstr, pStrA, 512 ) ;
				}
			}
			else if( *Format == L's' || *Format == L'S' )
			{
				Format ++ ;
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				pStr = NULL;
				pStrA = NULL;
				if( SkipFlag == FALSE )
				{
					if( Format[ -1 ] == 's' )
					{
						pStr = va_arg( Param, DXWCHAR * );
					}
					else
					{
						pStrA = va_arg( Param, char * );
					}
				}

				i = 0;
				for(;;)
				{
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE; 
						break;
					}
					if( _CheckUTF16H( c ) == FALSE && ( c == L' ' || c == L'\n' || c == L'\r' ) )
					{
						FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
						break;
					}
					tstr[ i ] = c ;
					i ++ ;
					if( Width != -1 && i >= Width ) break;

					if( _CheckUTF16H( c ) == TRUE )
					{
						res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
						FSYNC( FileHandle )
						if( res == 0 )
						{
							Eof = TRUE; 
							break;
						}
						tstr[ i ] = c ;
						i ++ ;
						if( Width != -1 && i >= Width ) break;
					}
				}
				tstr[ i ] = '\0' ;
				if( pStr )
				{
					__WCSCPY( pStr, tstr ) ;
				}
				if( pStrA )
				{
					WCharToMBChar( _GET_CODEPAGE(), tstr, pStrA, 512 ) ;
				}
			}
			else if( *Format == L'f' || *Format == L'F' || *Format == L'g' || *Format == L'G' )
			{
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;

				Format ++ ;
				MinusFlag = FALSE;
				TenFlag = FALSE;
				SisuuFlag = FALSE;
				MinusFlag2 = FALSE;
				res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
				FSYNC( FileHandle )
				if( res == 0 )
				{
					Eof = TRUE;
					break;
				}
				if( c == L'-' )
				{
					MinusFlag = TRUE;
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
				}

				i = 0;	//自然数
				j = 0;	//小数
				k = 0;	//指数
				for(;;)
				{
					if( c == L'.' )
					{
						if( TenFlag ) break;
						TenFlag = TRUE;
					}
					else if( c == L'e' || c == L'E' )
					{
						if( SisuuFlag ) break;
						SisuuFlag = TRUE;
					}
					else if( c == L'-' || c == L'+' )
					{
						if( SisuuFlag == FALSE || k != 0 ) break;
						if( c == L'-' ) MinusFlag2 = TRUE;
					}
					else if( c >= L'0' && c <= L'9' )
					{
						if( SisuuFlag )
						{
							if( k >= 127 ) break ;
							Number3[k] = c - L'0';
							k ++ ;
						}
						else if( TenFlag )
						{
							if( j >= 127 ) break ;
							Number2[j] = c - L'0';
							j ++ ;
						}
						else
						{
							if( i >= 127 ) break ;
							Number[i] = c - L'0';
							i ++ ;
						}
					}
					else break;

					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( Width != -1 )
					{
						Width -- ;
						if( Width == 0 ) break;
					}
				}
				if( Eof == FALSE )
				{
					FSEEK( FileHandle, -( int )sizeof( DXWCHAR ), SEEK_CUR );
				}
				if( i == 0 && j == 0 ) break;
				num = i;
				num2 = j;
				num3 = k;
				if( num == 0 && num2 == 0 ) break;

				doubleCount = 1.0;
				doubleNum = 0;
				for( i = num - 1; i >= 0; i --, doubleCount *= 10.0 )
				{
					if( Number[i] != 0 )
						doubleNum += Number[i] * doubleCount;
				}
				if( MinusFlag ) doubleNum = -doubleNum;

				doubleCount = 0.1;
				doubleNum2 = 0;
				for( i = 0; i < num2; i ++, doubleCount /= 10.0 )
				{
					if( Number2[i] != 0 )
						doubleNum2 += Number2[i] * doubleCount;
				}
				if( MinusFlag ) doubleNum2 = -doubleNum2;

				int64Count = 1;
				int64Num = 0;
				for( i = num3 - 1; i >= 0; i --, int64Count *= 10 )
				{
					int64Num += Number3[i] * int64Count;
				}
				if( MinusFlag2 ) int64Num = -int64Num;

				doubleNum3 = 1.0;
				if( int64Num != 0 )
				{
					if( int64Num < 0 )
					{
						int64Num = -int64Num;
						for( i = 0; i < int64Num; i++ )
							doubleNum3 /= 10.0;
					}
					else
					{
						for( i = 0; i < int64Num; i++ )
							doubleNum3 *= 10.0;
					}
				}

				doubleNum = ( doubleNum + doubleNum2 ) * doubleNum3;

				if( SkipFlag == FALSE )
				{
					if( lFlag ) *va_arg( Param, double * ) = doubleNum;
					else        *va_arg( Param, float *  ) = (float)doubleNum;
				}
			}
			if( SkipFlag == FALSE ) ReadNum ++ ;
		}
		else
		{
			if( *Format == L' ' || *Format == L'\n' || *Format == L'\r' )
			{
				while( *Format != L'\0' && ( *Format == L' ' || *Format == L'\n' || *Format == L'\r' ) ) Format ++ ;
				FileRead_SkipSpaceW( FileHandle, &Eof );
				if( Eof == TRUE ) break;
			}
			else
			{
				for( i = 0; *Format != L'\0' && *Format != L' ' && *Format != L'\n' && *Format != L'\r' && *Format != L'%'; i++, Format ++ )
					str[i] = *Format;
				str[i] = L'\0';
				num = i;
				for( i = 0; i < num; i ++ )
				{
					res = FREAD( &c, sizeof( DXWCHAR ), 1, FileHandle );
					FSYNC( FileHandle )
					if( res == 0 )
					{
						Eof = TRUE;
						break;
					}
					if( str[i] != c ) break;
				}
			}
		}
	}

	if( FEOF( FileHandle ) && ReadNum == 0 ) return EOF;

	return ReadNum;
}

#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MP3
#ifndef DX_NON_DSHOW_MOVIE

HRESULT SoundCallback_DSMP3(  D_IMediaSample * pSample, D_REFERENCE_TIME * /*StartTime*/, D_REFERENCE_TIME * /*StopTime*/, BOOL /*TypeChanged*/, void *CallbackData )
{
	SOUNDCONV_DSMP3 *dsmp3 = ( SOUNDCONV_DSMP3 * )( ( ( SOUNDCONV * ) CallbackData )->ConvFunctionBuffer ) ;
	BYTE *pBuffer ;
	long BufferLen ;

	// バッファが解放されていたら何もせずに終了
	if( dsmp3->PCMBuffer == NULL )
		return 0 ;

	// サンプルのデータサイズを取得する
	pSample->GetPointer( &pBuffer ) ;
	BufferLen = pSample->GetActualDataLength() ;

	// サンプルがバッファに収まらない場合はバッファのサイズを大きくする
	if( dsmp3->PCMValidDataSize + BufferLen > dsmp3->PCMBufferSize )
	{
		void *OldBuffer ;

		// 今までのバッファのサイズを保存
		OldBuffer = dsmp3->PCMBuffer ;

		// バッファのサイズを１ＭＢ増やす
		dsmp3->PCMBufferSize += 1 * 1024 * 1024 ;
		dsmp3->PCMBuffer = DXALLOC( dsmp3->PCMBufferSize ) ;

		// 確保に失敗したらエラー
		if( dsmp3->PCMBuffer == NULL )
		{
			DXFREE( OldBuffer ) ;
			return 0 ;
		}

		// 今までのデータを転送
		_MEMCPY( dsmp3->PCMBuffer, OldBuffer, dsmp3->PCMValidDataSize ) ;

		// 今までのバッファを解放
		DXFREE( OldBuffer ) ;
	}

	_MEMCPY( ( BYTE * )dsmp3->PCMBuffer + dsmp3->PCMValidDataSize, pBuffer, BufferLen ) ;
	dsmp3->PCMValidDataSize += BufferLen ;
	return 0 ;
}

// フィルターの指定方向の最初のピンを取得する
D_IPin *_GetFilterPin( D_IBaseFilter *Filter, D_PIN_DIRECTION Direction )
{
	D_IEnumPins *EnumPin ;
	D_IPin *Pin ;
	D_PIN_DIRECTION PinDir ;

	Filter->EnumPins( &EnumPin ) ;
	while( EnumPin->Next( 1, &Pin, NULL ) == S_OK )
	{
		Pin->QueryDirection( &PinDir ) ;
		if( PinDir == Direction )
		{
			EnumPin->Release() ;
			return Pin ;
		}
		Pin->Release() ;
	}
	EnumPin->Release() ;
	return NULL ;
}

// DirectShowを使ってMP3ファイルをコンバートするセットアップ処理を行う( [戻] -1:エラー )
extern int SetupSoundConvert_DSMP3( SOUNDCONV *SoundConv )
{
	D_CMediaType ccmt, cmt, csmpmt;
	D_CMemStream *MemStream = NULL ;
	D_CMemReader *MemReader = NULL ;
	D_IBaseFilter *NullRenderer = NULL ;
	D_CSampleGrabber *SampleGrabber = NULL ;
	D_IGraphBuilder *GraphBuilder = NULL ;
	D_IMediaControl *MediaControl = NULL ;
	D_IMediaFilter *MediaFilter = NULL ;
	D_IMediaEvent *MediaEvent = NULL ;

	SOUNDCONV_DSMP3 *dsmp3 = ( SOUNDCONV_DSMP3 * )SoundConv->ConvFunctionBuffer ;
	STREAMDATA *Stream = &SoundConv->Stream ;
	void *TempBuffer ;
	HRESULT hr ;
	WAVEFORMATEX *smpfmt;
	DWORD TempSize ;

	// メンバ変数初期化
	_MEMSET( dsmp3, 0, sizeof( SOUNDCONV_DSMP3 ) ) ;

	// ファイルのサイズを取得する
	Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_END ) ;
	TempSize = ( DWORD )Stream->ReadShred.Tell( Stream->DataPoint ) ;
	Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_SET ) ;

	// ファイルを丸ごと格納できるメモリ領域を取得してそこにファイルを丸ごと読み込む
	TempBuffer = DXALLOC( TempSize ) ;
	if( TempBuffer == NULL )
	{
		goto ERR ;
	}
	Stream->ReadShred.Read( TempBuffer, TempSize, 1, Stream->DataPoint ) ;

	// メモリソースフィルタの準備
	MemStream = ( D_CMemStream * )D_CMemStream::CreateInstance( ( BYTE * )TempBuffer, TempSize ) ;
	if( MemStream == NULL )
	{
		DXST_ERRORLOG_ADDA( "MP3変換用 D_CMemStream の作成に失敗しました" );
		goto ERR ;
	}
	hr = S_OK ;
	ccmt.majortype = MEDIATYPE_STREAM ;
	ccmt.subtype = D_MEDIASUBTYPE_MPEG1AUDIO ;
	MemReader = ( D_CMemReader * )D_CMemReader::CreateInstance( MemStream, &ccmt, &hr ) ;
	if( MemReader == NULL )
	{
		DXST_ERRORLOG_ADDA( "MP3変換用 D_CMemReader の作成に失敗しました" );
		goto ERR ;
	}
	MemReader->AddRef() ;

	// Nullレンダラーフィルタの作成
	if( ( FAILED( WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_NULLRENDERER, 0, CLSCTX_INPROC_SERVER, IID_IBASEFILTER, ( void ** )&NullRenderer ) ) ) )
	{
		DXST_ERRORLOG_ADDA( "MP3変換用 NullRender の作成に失敗しました" );
		goto ERR ;
	}

	// グラフィックビルダーオブジェクトの作成
	if( ( FAILED( WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_FILTERGRAPH, NULL, CLSCTX_INPROC, IID_IGRAPHBUILDER, ( void ** )&GraphBuilder ) ) ) )
	{
		DXST_ERRORLOG_ADDA( "MP3変換用 IGraphBuilder の作成に失敗しました" );
		goto ERR ;
	}

	// サンプルグラバフィルタ生成
	hr = TRUE ;
	SampleGrabber = ( D_CSampleGrabber * )D_CSampleGrabber::CreateInstance( NULL, &hr ) ;
	if( SampleGrabber == NULL )
	{
		DXST_ERRORLOG_ADDA( "MP3変換用 D_CSampleGrabber の作成に失敗しました" );
		goto ERR ;
	}

	// サンプルグラバフィルタの設定
	cmt.majortype = MEDIATYPE_AUDIO ;
	cmt.subtype = D_MEDIASUBTYPE_PCM ;
	hr = SampleGrabber->SetAcceptedMediaType( &cmt ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "サンプルグラバフィルタのメディアタイプの設定に失敗しました" );
		goto ERR ;
	}

	// サンプルグラバフィルタをグラフに追加
	hr = GraphBuilder->AddFilter( SampleGrabber, L"SampleGrabber") ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "サンプルグラバフィルタのグラフへの追加に失敗しました" );
		goto ERR ;
	}

	// メモリソースフィルタをグラフに追加
	hr = GraphBuilder->AddFilter( MemReader, L"MemFile" ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "メモリファイルフィルタのグラフへの追加に失敗しました" );
		goto ERR ;
	}

	// フィルタ連結
	hr = GraphBuilder->Render( MemReader->GetPin( 0 ) ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "グラフの構築に失敗しました" );
		goto ERR ;
	}

	// ヌルレンダラーフィルタをグラフに追加
	hr = GraphBuilder->AddFilter( NullRenderer, L"File Renderer" ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "ヌルレンダラーフィルタのグラフへの追加に失敗しました" );
		goto ERR ;
	}

	// ヌルレンダラーを元のレンダラーと交換する
	{
		D_IEnumFilters *EnumFilter ;
		D_IPin *LastOutput, *LastInput, *InputPin, *OutputPin ;
		D_IBaseFilter *TmpFilter ;

		// フィルターの列挙
		GraphBuilder->EnumFilters( &EnumFilter ) ;

		// フィルター群の中からレンダラーフィルタを捜す
		while( EnumFilter->Next( 1, &TmpFilter, NULL ) == S_OK )
		{
			InputPin  = _GetFilterPin( TmpFilter, D_PINDIR_INPUT  ) ;
			OutputPin = _GetFilterPin( TmpFilter, D_PINDIR_OUTPUT ) ;

			// 出力のないフィルターはレンダラーフィルタという判断
			if( InputPin != NULL && OutputPin == NULL )
			{
				// 入力ピンに繋がれている出力ピンを取得する
				InputPin->ConnectedTo( &LastOutput ) ;
				if( LastOutput != NULL )
				{
					// 現在のレンダラーフィルタと出力ピンの接続を解除
					GraphBuilder->Disconnect( LastOutput ) ;
					GraphBuilder->Disconnect( InputPin ) ;

					// 列挙用に取得したオブジェクトを解放
					InputPin->Release() ;
					TmpFilter->Release() ;
					break ;
				}
			}

			// 列挙用に取得したオブジェクトを解放
			if( InputPin  ) InputPin->Release() ;
			if( OutputPin ) OutputPin->Release() ; 
			TmpFilter->Release() ;
		}

		// 列挙用インターフェイスの解放
		EnumFilter->Release() ;

		// ヌルレンダラーの入力ピンを取得
		LastInput = _GetFilterPin( NullRenderer, D_PINDIR_INPUT ) ;

		// ヌルレンダラーの入力ピンと最初にレンダラーと接続していた出力ピンを接続
		hr = GraphBuilder->Connect( LastOutput, LastInput ) ;

		// 一時的なインターフェイスを解放
		LastOutput->Release() ;
		LastInput->Release() ;

		// エラー判定
		if( hr != S_OK )
		{
			DXST_ERRORLOG_ADDA( "既存レンダラーをNullレンダラーと交換する処理が失敗しました" );
			goto ERR ;
		}
	}

	// オーディオフォーマットの取得
	_MEMSET( &csmpmt, 0, sizeof( csmpmt ) ) ;
	hr = SampleGrabber->GetConnectedMediaType( &csmpmt ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "音声が含まれていないか、オーディオフォーマットの取得に失敗しました。" );
		goto ERR ;
	}
	if( csmpmt.formattype != FORMAT_WAVEFORMATEX )
	{
		DXST_ERRORLOG_ADDA( "サポート外のストリームフォーマットです。" );
		goto ERR ;;
	}
	smpfmt = ( WAVEFORMATEX * )csmpmt.pbFormat ;
	if( smpfmt->wFormatTag != WAVE_FORMAT_PCM )
	{
		DXST_ERRORLOG_ADDA( "サポート外のオーディオフォーマットです。" );
		goto ERR ;
	}
	SoundConv->OutFormat.cbSize = 0 ;
	SoundConv->OutFormat.wFormatTag			= smpfmt->wFormatTag ;
	SoundConv->OutFormat.nChannels			= smpfmt->nChannels ;
	SoundConv->OutFormat.nBlockAlign		= smpfmt->nBlockAlign ;
	SoundConv->OutFormat.nSamplesPerSec		= smpfmt->nSamplesPerSec ;
	SoundConv->OutFormat.nAvgBytesPerSec	= SoundConv->OutFormat.nBlockAlign * smpfmt->nSamplesPerSec ;
	SoundConv->OutFormat.wBitsPerSample		= smpfmt->nBlockAlign * 8 / smpfmt->nChannels ;

	// サンプリング準備
	hr = SampleGrabber->SetCallback( SoundCallback_DSMP3, SoundConv ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "コールバック関数を設定できません" );
		goto ERR ;;
	}

	// とりあえず元データの１０倍サイズのメモリを確保する
	dsmp3->PCMBufferSize = TempSize * 10 ;
	dsmp3->PCMBuffer = DXALLOC( dsmp3->PCMBufferSize ) ;
	if( dsmp3->PCMBuffer == NULL )
	{
		DXST_ERRORLOG_ADDA( "PCMデータを格納するメモリ領域の確保に失敗しました" );
		goto ERR ;
	}

	// バッファ中の有効なデータのサイズを初期化
	dsmp3->PCMValidDataSize = 0 ;

	// メディアフィルターの取得
	hr = GraphBuilder->QueryInterface( IID_IMEDIAFILTER, ( void ** )&MediaFilter ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "IMediaFilter の取得に失敗しました" );
		goto ERR ;
	}

	// 同期ソースを無くす
	MediaFilter->SetSyncSource( NULL ) ;

	// メディアコントロールの取得
	hr = GraphBuilder->QueryInterface( IID_IMEDIACONTROL, ( void ** )&MediaControl ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "IMediaControl の取得に失敗しました" );
		goto ERR ;
	}

	// メディアイベントの取得
	hr = GraphBuilder->QueryInterface( IID_IMEDIAEVENT, ( void ** )&MediaEvent ) ;
	if( hr != S_OK )
	{
		DXST_ERRORLOG_ADDA( "IMediaEvent の取得に失敗しました" );
		goto ERR ;
	}

	// 再生開始
	MediaControl->Run() ;

	// 再生が終了するのを待つ
	for(;;)
	{
		long EvCode ;

		if( NS_ProcessMessage() != 0 )
		{
			DXST_ERRORLOG_ADDA( "MP3変換の途中でソフトが終了しました" );
			goto ERR ;
		}

		// メモリ転送が完了したらループを抜ける
		if( MediaEvent->WaitForCompletion( 100, &EvCode ) == S_OK )
		{
			if( EvCode == D_EC_ERRORABORT )
			{
				DXST_ERRORLOG_ADDA( "MP3変換の途中でエラーが発生しました" );
				goto ERR ;
			}
			if( EvCode == D_EC_COMPLETE )
			{
				break ;
			}
		}
	}

	// バッファが解放されていたらエラー
	if( dsmp3->PCMBuffer == NULL )
	{
		DXST_ERRORLOG_ADDA( "PCMデータを格納するメモリ領域の再確保に失敗しました" );
		goto ERR ;
	}

	// DirectShow を使用したＭＰ３再生の印を保存
	SoundConv->MethodType = SOUND_METHODTYPE_DSMP3 ;

	// 変換後のＰＣＭデータを一時的に保存するメモリ領域のサイズは１秒
	SoundConv->DestDataSize = SoundConv->OutFormat.nAvgBytesPerSec ;

	// 各種インターフェイスとメモリの解放
	if( MediaEvent   ){ MediaEvent->Release() ; MediaEvent = NULL ; }
	if( MediaFilter  ){ MediaFilter->Release() ; MediaFilter = NULL ; }
	if( MediaControl ){ MediaControl->Release() ; MediaControl = NULL ; }
	if( GraphBuilder ){ GraphBuilder->Release() ; GraphBuilder = NULL ; }
	if( NullRenderer ){ NullRenderer->Release() ; NullRenderer = NULL ; }
	if( MemStream ){ MemStream->DeleteInstance() ; MemStream = NULL ; }
	if( MemReader ){ MemReader->DeleteInstance() ; MemReader = NULL ; }
	if( TempBuffer ){ DXFREE( TempBuffer ) ; TempBuffer = NULL ; }

	// 正常終了
	return 0 ;

ERR :
	if( MediaEvent   ){ MediaEvent->Release() ; MediaEvent = NULL ; }
	if( MediaFilter ){ MediaFilter->Release() ; MediaFilter = NULL ; }
	if( MediaControl ){ MediaControl->Release() ; MediaControl = NULL ; }
	if( GraphBuilder ){ GraphBuilder->Release() ; GraphBuilder = NULL ; }
	if( NullRenderer ){ NullRenderer->Release() ; NullRenderer = NULL ; }
	if( MemStream ){ MemStream->DeleteInstance() ; MemStream = NULL ; }
	if( MemReader ){ MemReader->DeleteInstance() ; MemReader = NULL ; }
	if( TempBuffer ){ DXFREE( TempBuffer ) ; TempBuffer = NULL ; }
	if( dsmp3->PCMBuffer ){ DXFREE( dsmp3->PCMBuffer ) ; dsmp3->PCMBuffer = NULL ; }

	// エラー終了
	return -1 ;
}

#endif // DX_NON_MOVIE
#endif // DX_NON_DSHOW_MP3
#endif // DX_NON_DSHOW_MOVIE




}












