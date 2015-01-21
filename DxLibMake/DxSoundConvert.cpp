// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		サウンドデータ変換プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

// Include ------------------------------------------------------------------
#include "DxSoundConvert.h"
#include "DxStatic.h"

#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxUseCLib.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// チャンクＩＤセットマクロ
#define RIFFCHUNKID( C1 , C2 , C3 , C4 )	( DWORD )( ((( DWORD )C4)<<24) | ((( DWORD )C3)<<16) | ((( DWORD )C2)<<8) | (C1) ) 

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT		0x0003
#endif

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM				0x0001
#endif

// 结构体定义 --------------------------------------------------------------------

// サウンド変換処理全体で使用するデータ構造体
struct SOUNDCONVERTDATA
{
	int							InitializeFlag ;				// 初期化フラグ

#ifndef DX_NON_ACM
	HMODULE						msacm32DLL ;					// msacm32DLL
	MMRESULT					( WINAPI *acmStreamOpenFunc )( LPHACMSTREAM phas, HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, LPWAVEFILTER pwfltr, DWORD dwCallback, DWORD dwInstance, DWORD fdwOpen );
	MMRESULT					( WINAPI *acmFormatSuggestFunc )( HACMDRIVER had, LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst, DWORD cbwfxDst, DWORD fdwSuggest );
	MMRESULT					( WINAPI *acmStreamCloseFunc )( HACMSTREAM has, DWORD fdwClose );
	MMRESULT					( WINAPI *acmMetricsFunc )( HACMOBJ hao, UINT uMetric, LPVOID pMetric );
	MMRESULT					( WINAPI *acmStreamPrepareHeaderFunc )( HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwPrepare );
	MMRESULT					( WINAPI *acmStreamConvertFunc )( HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwConvert );
	MMRESULT					( WINAPI *acmStreamUnprepareHeaderFunc )( HACMSTREAM has, LPACMSTREAMHEADER pash, DWORD fdwUnprepare );
	MMRESULT					( WINAPI *acmStreamSizeFunc )( HACMSTREAM has, DWORD cbInput, LPDWORD pdwOutputBytes, DWORD fdwSize );
#endif
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// サウンド変換処理全体で使用するデータ
SOUNDCONVERTDATA GSoundConvertData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

static	int CheckNormalWaveFormat( STREAMDATA *Stream ) ;											// 通常のＰＣＭ形式か調べる( TRUE:ＰＣＭ形式  FALSE:違う )

static	int SetupSoundConvert_WAVE(            SOUNDCONV *SoundConv ) ;								// ＷＡＶＥファイルのセットアップ処理を行う( [戻] -1:エラー )
static	int TerminateSoundConvert_WAVE(        SOUNDCONV *SoundConv ) ;								// ＷＡＶＥファイルの後始末処理を行う
static	int ConvertProcessSoundConvert_WAVE(   SOUNDCONV *SoundConv ) ;								// 変換後のバッファにデータを補充する
//static	int SetTimeSoundConvert_WAVE(      SOUNDCONV *SoundConv, int Time ) ;					// 変換処理の位置を変更する( ミリ秒単位 )
static	int SetSampleTimeSoundConvert_WAVE(    SOUNDCONV *SoundConv, int SampleTime ) ;				// 変換処理の位置を変更する( サンプル単位 )
static	int GetSoundConvertDestSize_Fast_WAVE( SOUNDCONV *SoundConv ) ;								// 変換後の大凡のデータサイズを得る

#ifndef DX_NON_ACM
static	int SetupSoundConvert_ACM(            SOUNDCONV *SoundConv ) ;								// ＡＣＭを使用したファイルのセットアップ処理を行う( [戻] -1:エラー )
static	int SetupSoundConvert_MP3(            SOUNDCONV *SoundConv ) ;								// ＭＰ３ファイルのセットアップ処理を行う( [戻] -1:エラー )
static	int TerminateSoundConvert_ACM(        SOUNDCONV *SoundConv ) ;								// ＡＣＭを使用したファイルの後始末処理を行う
static	int ConvertProcessSoundConvert_ACM(   SOUNDCONV *SoundConv ) ;								// 変換後のバッファにデータを補充する
//static	int SetTimeSoundConvert_ACM(      SOUNDCONV *SoundConv, int Time ) ;					// 変換処理を位置を変更する( ミリ秒単位 )
static	int SetSampleTimeSoundConvert_ACM(    SOUNDCONV *SoundConv, int SampleTime ) ;				// 変換処理の位置を変更する( サンプル単位 )
static	int GetSoundConvertDestSize_Fast_ACM( SOUNDCONV *SoundConv ) ;								// 変換後の大凡のデータサイズを得る
#endif

#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3
static	int TerminateSoundConvert_DSMP3(        SOUNDCONV *SoundConv ) ;							// MP3を使用したファイルの後始末処理を行う
static	int ConvertProcessSoundConvert_DSMP3(   SOUNDCONV *SoundConv ) ;							// 変換後のバッファにデータを補充する
//extern	int SetTimeSoundConvert_DSMP3(      SOUNDCONV *SoundConv, int Time ) ;					// 変換処理を位置を変更する( ミリ秒単位 )
static	int SetSampleTimeSoundConvert_DSMP3(    SOUNDCONV *SoundConv, int SampleTime ) ;			// 変換処理の位置を変更する( サンプル単位 )
static	int GetSoundConvertDestSize_Fast_DSMP3( SOUNDCONV *SoundConv ) ;							// 変換後の大凡のデータサイズを得る
#endif
#endif
#endif

// プログラム --------------------------------------------------------------------

// 初期化・終了関数

// サウンドデータ変換処理の初期化を行う
extern	int InitializeSoundConvert( void )
{
	// 既に初期化済みの場合は何もしない
	if( GSoundConvertData.InitializeFlag == TRUE )
		return -1 ;

#ifndef DX_NON_ACM
	// MSACM32DLL をロードする
	if( GSoundConvertData.msacm32DLL == NULL )
	{
		GSoundConvertData.msacm32DLL = LoadLibrary( _T( "msacm32.dll" ) ) ;
		if( GSoundConvertData.msacm32DLL == NULL )
			return -1 ;

		GSoundConvertData.acmFormatSuggestFunc = ( MMRESULT ( WINAPI * )( HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, DWORD, DWORD ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmFormatSuggest" ) ;
		if( GSoundConvertData.acmFormatSuggestFunc == NULL )
			return -1 ;

		GSoundConvertData.acmStreamOpenFunc = ( MMRESULT ( WINAPI * )( LPHACMSTREAM, HACMDRIVER, LPWAVEFORMATEX, LPWAVEFORMATEX, LPWAVEFILTER, DWORD, DWORD, DWORD ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmStreamOpen" ) ;
		if( GSoundConvertData.acmStreamOpenFunc == NULL )
			return -1 ;

		GSoundConvertData.acmStreamCloseFunc = ( MMRESULT ( WINAPI * )( HACMSTREAM, DWORD ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmStreamClose" ) ;
		if( GSoundConvertData.acmStreamCloseFunc == NULL )
			return -1 ;

		GSoundConvertData.acmMetricsFunc = ( MMRESULT ( WINAPI * )( HACMOBJ, UINT, LPVOID ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmMetrics" ) ;
		if( GSoundConvertData.acmMetricsFunc == NULL )
			return -1 ;

		GSoundConvertData.acmStreamPrepareHeaderFunc = ( MMRESULT ( WINAPI * )( HACMSTREAM, LPACMSTREAMHEADER, DWORD ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmStreamPrepareHeader" ) ;
		if( GSoundConvertData.acmStreamPrepareHeaderFunc == NULL )
			return -1 ;

		GSoundConvertData.acmStreamConvertFunc = ( MMRESULT ( WINAPI * )( HACMSTREAM, LPACMSTREAMHEADER, DWORD ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmStreamConvert" ) ;
		if( GSoundConvertData.acmStreamConvertFunc == NULL )
			return -1 ;

		GSoundConvertData.acmStreamUnprepareHeaderFunc = ( MMRESULT ( WINAPI * )( HACMSTREAM, LPACMSTREAMHEADER, DWORD ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmStreamUnprepareHeader" ) ;
		if( GSoundConvertData.acmStreamUnprepareHeaderFunc == NULL )
			return -1 ;

		GSoundConvertData.acmStreamSizeFunc = ( MMRESULT  ( WINAPI * )( HACMSTREAM, DWORD, LPDWORD, DWORD ) )GetProcAddress( GSoundConvertData.msacm32DLL, "acmStreamSize" ) ;
		if( GSoundConvertData.acmStreamSizeFunc == NULL )
			return -1 ;
	}
#endif

	// 初期化完了フラグを立てる
	GSoundConvertData.InitializeFlag = TRUE ;

	// 正常終了
	return 0 ;
}

// サウンドデータ変換処理の終了処理を行う
extern	int TerminateSoundConvert( void )
{
	// 既に後始末処理済みの場合は何もしない
	if( GSoundConvertData.InitializeFlag == FALSE )
		return -1 ;

#ifndef DX_NON_ACM
	// msacm32.DLL の解放
	if( GSoundConvertData.msacm32DLL )
	{
		FreeLibrary( GSoundConvertData.msacm32DLL ) ;
		GSoundConvertData.msacm32DLL = NULL ;
		GSoundConvertData.acmFormatSuggestFunc = NULL ;
		GSoundConvertData.acmStreamOpenFunc = NULL ;
		GSoundConvertData.acmStreamCloseFunc = NULL ;
		GSoundConvertData.acmMetricsFunc = NULL ;
		GSoundConvertData.acmStreamPrepareHeaderFunc = NULL ;
		GSoundConvertData.acmStreamConvertFunc = NULL ;
		GSoundConvertData.acmStreamUnprepareHeaderFunc = NULL ;
		GSoundConvertData.acmStreamSizeFunc = NULL ;
	}
#endif

	// 初期化完了フラグを倒す
	GSoundConvertData.InitializeFlag = FALSE ;

	// 正常終了
	return 0 ;
}

// 変換処理のセットアップ( [戻] -1:エラー )
extern	int SetupSoundConvert( SOUNDCONV *SoundConv, STREAMDATA *Stream, int DisableReadSoundFunctionMask, int OggVorbisBitDepth, int OggVorbisFromTheoraFile )
{
	// 試す
#ifndef DX_NON_OGGVORBIS
	SoundConv->OggVorbisBitDepth = OggVorbisBitDepth ;
	SoundConv->OggVorbisFromTheoraFile = OggVorbisFromTheoraFile ;
#endif
	SoundConv->Stream = *Stream ;
	SoundConv->SeekLockPosition = 0 ;
	Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_SET ) ;
	if( ( DisableReadSoundFunctionMask & DX_READSOUNDFUNCTION_PCM ) == 0 )
	{
		if( SetupSoundConvert_WAVE( SoundConv ) == 0 ) goto R1 ;
		Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_SET ) ;
	}
#ifndef DX_NON_ACM
	if( ( DisableReadSoundFunctionMask & DX_READSOUNDFUNCTION_ACM ) == 0 )
	{
		if( SetupSoundConvert_ACM( SoundConv ) == 0 ) goto R1 ;
		Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_SET ) ;
	}
#endif
#ifndef DX_NON_OGGVORBIS
	if( ( DisableReadSoundFunctionMask & DX_READSOUNDFUNCTION_OGG ) == 0 )
	{
		if( SetupSoundConvert_OGG( SoundConv ) == 0 ) goto R1 ;
		Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_SET ) ;
	}
#endif
#ifndef DX_NON_ACM
	if( ( DisableReadSoundFunctionMask & DX_READSOUNDFUNCTION_MP3 ) == 0 )
	{
		if( SetupSoundConvert_MP3( SoundConv ) == 0 ) goto R1 ;
	}
#endif
#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3
	if( ( DisableReadSoundFunctionMask & DX_READSOUNDFUNCTION_DSMP3 ) == 0 )
	{
		if( SetupSoundConvert_DSMP3( SoundConv ) == 0 ) goto R1 ;
	}
#endif
#endif
#endif
	return -1 ;
	
R1 :
	// 共通初期化処理を行う
	SoundConv->InitializeFlag = FALSE ;
	SoundConv->EndFlag = FALSE ;
	SoundConv->DestData = DXALLOC( SoundConv->DestDataSize ) ;
	if( SoundConv->DestData == NULL )
	{
		TerminateSoundConvert( SoundConv ) ;
		return -1 ;
	}
	SoundConv->DestDataValidSize = 0 ;
	SoundConv->DestDataCompSize = 0 ;
	SoundConv->DestDataCompSizeAll = 0 ;

	// 初期化完了
	SoundConv->InitializeFlag = TRUE ;
	
	// 終了
	return 0 ;
}

/*
// 変換処理を初期状態に戻す
extern	int SetTimeSoundConvert( SOUNDCONV *SoundConv, int Time )
{
	int res ;
	int BytePos ;

	BytePos = ( SoundConv->OutFormat.nSamplesPerSec * Time / 1000 ) * SoundConv->OutFormat.nBlockAlign ;
	if( _ABS( BytePos - SoundConv->DestDataCompSizeAll ) < 16 ) return 0 ;

	switch( SoundConv->MethodType )
	{
	case SOUND_METHODTYPE_NORMAL : res = SetTimeSoundConvert_WAVE( SoundConv, Time ) ; break ;
#ifndef DX_NON_ACM
	case SOUND_METHODTYPE_ACM : res = SetTimeSoundConvert_ACM( SoundConv, Time ) ; break ;
#endif
#ifndef DX_NON_OGGVORBIS
	case SOUND_METHODTYPE_OGG : res = SetTimeSoundConvert_OGG( SoundConv, Time ) ; break ;
#endif
	}
	SoundConv->EndFlag = FALSE ;
	return res ;
}
*/

// 変換処理の位置を変更する( サンプル単位 )
extern int SetSampleTimeSoundConvert( SOUNDCONV *SoundConv, int SampleTime )
{
	int res = -1 ;
	int BytePos ;

	BytePos = SampleTime * SoundConv->OutFormat.nBlockAlign ;
	if( _ABS( BytePos - SoundConv->DestDataCompSizeAll ) < 16 ) return 0 ;

	switch( SoundConv->MethodType )
	{
	case SOUND_METHODTYPE_NORMAL : res = SetSampleTimeSoundConvert_WAVE( SoundConv, SampleTime ) ; break ;
#ifndef DX_NON_ACM
	case SOUND_METHODTYPE_ACM : res = SetSampleTimeSoundConvert_ACM( SoundConv, SampleTime ) ; break ;
#endif
#ifndef DX_NON_OGGVORBIS
	case SOUND_METHODTYPE_OGG : res = SetSampleTimeSoundConvert_OGG( SoundConv, SampleTime ) ; break ;
#endif
#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3
	case SOUND_METHODTYPE_DSMP3 : res = SetSampleTimeSoundConvert_DSMP3( SoundConv, SampleTime ) ; break ;
#endif
#endif
#endif
	}
	SoundConv->EndFlag = FALSE ;
	return res ;
}

// シーク予定の位置をセットしておく(ＡＣＭの為に)
extern	int SetupSeekPosSoundConvert( SOUNDCONV *SoundConv, int SeekPos )
{
	SoundConv->SeekLockPosition = SeekPos * SoundConv->OutFormat.nBlockAlign ;

	return 0 ;
}


// 指定のサイズ分だけ音声を解凍( [戻] -1:エラー  0以上:変換したサイズ )
extern	int RunSoundConvert( SOUNDCONV *SoundConv, void *DestBuffer, int DestSize )
{
	int pos, movesize, res = -1, copysize ;
	
	if( SoundConv->InitializeFlag == FALSE ) return -1 ;
	if( SoundConv->EndFlag == TRUE ) return 0 ;
	
	pos = 0 ;
	copysize = 0 ;
	while( DestSize != 0 )
	{
		if( SoundConv->DestDataValidSize == 0 )
		{
			SoundConv->DestDataCompSize = 0 ;
			switch( SoundConv->MethodType )
			{
			case SOUND_METHODTYPE_NORMAL : res = ConvertProcessSoundConvert_WAVE( SoundConv ) ; break ;
#ifndef DX_NON_ACM
			case SOUND_METHODTYPE_ACM : res = ConvertProcessSoundConvert_ACM( SoundConv ) ; break ;
#endif
#ifndef DX_NON_OGGVORBIS
			case SOUND_METHODTYPE_OGG : res = ConvertProcessSoundConvert_OGG( SoundConv ) ; break ;
#endif
#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3
			case SOUND_METHODTYPE_DSMP3 : res = ConvertProcessSoundConvert_DSMP3( SoundConv ) ; break ;
#endif
#endif
#endif
			}
			if( res == -1 ) break ;
		}
	
		movesize = DestSize ;
		if( movesize > SoundConv->DestDataValidSize ) movesize = SoundConv->DestDataValidSize ;
/*
		int i ;
		int tmp ;
		short *dst, *src ;

		for( i = 0 ; i < movesize / 4 ; i ++ )
		{
			dst = ( short * )( (BYTE *)DestBuffer + pos + i * 4 ) ;
			src = ( short * )( (BYTE *)SoundConv->DestData + SoundConv->DestDataCompSize + i * 4 ) ;

			tmp = src[ 0 ] * 3 / 2 ;
			if( tmp >  32767 ) tmp =  32767 ;
			if( tmp < -32768 ) tmp = -32768 ;
			dst[ 0 ] = ( short )tmp ;

//			NS_DrawPixel( i, ( tmp + 32768 ) / 256, GetColor( 255,255,255 ) ) ;

			tmp = src[ 1 ] * 3 / 2 ;
			if( tmp >  32767 ) tmp =  32767 ;
			if( tmp < -32768 ) tmp = -32768 ;
			dst[ 1 ] = ( short )tmp ;
		}
//		NS_ScreenFlip() ;
//		WaitKey() ;
*/
		_MEMCPY( (BYTE *)DestBuffer + pos,
				(BYTE *)SoundConv->DestData + SoundConv->DestDataCompSize,
				movesize ) ;
		SoundConv->DestDataValidSize -= movesize ;
		SoundConv->DestDataCompSize += movesize ;
		SoundConv->DestDataCompSizeAll += movesize ;
		
		DestSize -= movesize ;
		pos += movesize ;
		copysize += movesize ;
	}
	if( DestSize != 0 )
	{
		SoundConv->EndFlag = TRUE ;
	}

	// 終了
	return copysize ;
}

// 変換処理の後始末を行う
extern	int TerminateSoundConvert( SOUNDCONV *SoundConv )
{
	if( SoundConv->InitializeFlag == FALSE ) return -1 ;
	
	if( SoundConv->DestData != NULL )
	{
		DXFREE( SoundConv->DestData ) ;
		SoundConv->DestData = NULL ;
	}
	
	switch( SoundConv->MethodType )
	{
	case SOUND_METHODTYPE_NORMAL : TerminateSoundConvert_WAVE( SoundConv ) ; break ;
#ifndef DX_NON_ACM
	case SOUND_METHODTYPE_ACM : TerminateSoundConvert_ACM( SoundConv ) ; break ;
#endif
#ifndef DX_NON_OGGVORBIS
	case SOUND_METHODTYPE_OGG : TerminateSoundConvert_OGG( SoundConv ) ; break ;
#endif
#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3
	case SOUND_METHODTYPE_DSMP3 : TerminateSoundConvert_DSMP3( SoundConv ) ; break ;
#endif
#endif
#endif
	}

	SoundConv->InitializeFlag = FALSE ;
	
	// 終了
	return 0 ;	
}

// 変換後の音声形式を取得( [戻] -1:エラー  0以上:変換後のサイズ )
extern	int GetOutSoundFormatInfo( SOUNDCONV *SoundConv, WAVEFORMATEX *OutWaveFormat )
{
	*OutWaveFormat = SoundConv->OutFormat ;
	return 0 ;
}

// 変換後の音声形式を取得( [戻] -1:エラー  0以上:変換後のサイズ )
extern	int GetOutSoundFormat( SOUNDCONV *SoundConv, WAVEFORMATEX *OutWaveFormat )
{
	*OutWaveFormat = SoundConv->OutFormat ;
	
	return 0 ;
}

// 変換処理が終了しているかどうかを取得する( [戻] TRUE:終了した  FALSE:まだ終了していない )
extern	int GetSoundConvertEndState( SOUNDCONV *SoundConv )
{
	return SoundConv->EndFlag ;
}

// 変換後の大凡のデータサイズを得る
extern	int GetSoundConvertDestSize_Fast( SOUNDCONV *SoundConv )
{
	switch( SoundConv->MethodType )
	{
	case SOUND_METHODTYPE_NORMAL : return GetSoundConvertDestSize_Fast_WAVE( SoundConv ) ; break ;
#ifndef DX_NON_ACM
	case SOUND_METHODTYPE_ACM : return GetSoundConvertDestSize_Fast_ACM( SoundConv ) ; break ;
#endif
#ifndef DX_NON_OGGVORBIS
	case SOUND_METHODTYPE_OGG : return GetSoundConvertDestSize_Fast_OGG( SoundConv ) ; break ;
#endif
#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3
	case SOUND_METHODTYPE_DSMP3 : return GetSoundConvertDestSize_Fast_DSMP3( SoundConv ) ; break ;
#endif
#endif
#endif
	}
	return 0 ;
}

//extern void *SoundBuf ;
//extern int SoundSize ;

// 高速な一括変換
extern	int SoundConvertFast( SOUNDCONV *SoundConv, 
									WAVEFORMATEX *FormatP, void **DestBufferP, int *DestSizeP )
{
	void *Buffer ;
	int BufferSize ;

	// バッファの確保
	BufferSize = GetSoundConvertDestSize_Fast( SoundConv ) ;

	Buffer = DXALLOC( BufferSize ) ;
	if( Buffer == NULL ) return -1 ;
	
	// 変換位置のセット
//	SetTimeSoundConvert( SoundConv, 0 ) ;
	SetSampleTimeSoundConvert( SoundConv, 0 ) ;


	// データ形式が無圧縮ＰＣＭの場合のみそのままコピー
	if( SoundConv->MethodType == SOUND_METHODTYPE_NORMAL &&
		( ( SOUNDCONV_WAVE * )SoundConv->ConvFunctionBuffer )->SrcFormat.wFormatTag == WAVE_FORMAT_PCM )
	{
		SoundConv->Stream.ReadShred.Read( Buffer, BufferSize, 1, SoundConv->Stream.DataPoint ) ;
	}
	else
	{
		// それ以外の場合は普通に展開
		BufferSize = RunSoundConvert( SoundConv, Buffer, BufferSize ) ;
	}

	// 情報を保存
	*FormatP = SoundConv->OutFormat ;
	*DestBufferP = Buffer ;
	*DestSizeP = BufferSize ;
	
	// 終了
	return 0 ;
}















// フォーマット変換用

// 通常のＰＣＭ形式か調べる( TRUE:ＰＣＭ形式  FALSE:違う )
extern	int CheckNormalWaveFormat( STREAMDATA *Stream )
{
	___RIFFCHUNK chunk ;
	char type[5] ;
	WAVEFORMAT format ;

	// RIFFファイルかどうか調べる
	Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
	if( chunk.fcc != RIFFCHUNKID( 'R', 'I', 'F', 'F' ) )
		return FALSE ;
		
	// WAVEファイルかどうか調べる
	type[4] = '\0' ;
	Stream->ReadShred.Read( type, 4, 1, Stream->DataPoint ) ;
	if( _STRCMP( type, "WAVE" ) != 0 )
		return FALSE ;
		
	// ノーマルなＰＣＭ形式か調べる
	Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
	if( chunk.fcc != RIFFCHUNKID( 'f', 'm', 't', ' ' ) )
		return FALSE ;

	Stream->ReadShred.Read( &format, sizeof( WAVEFORMAT ), 1, Stream->DataPoint ) ;
	if( format.wFormatTag != WAVE_FORMAT_PCM )
		return FALSE ;
		
	// ここまで来たら通常のフォーマットということ
	return TRUE ;
}

// ＷＡＶＥファイルのセットアップ処理を行う( [戻] -1:エラー )
static	int SetupSoundConvert_WAVE( SOUNDCONV *SoundConv )
{
	___RIFFCHUNK chunk ;
	char type[5] ;
	WAVEFORMATEX *format = NULL ;
	STREAMDATA *Stream = &SoundConv->Stream ;
	SOUNDCONV_WAVE *Wave = ( SOUNDCONV_WAVE * )SoundConv->ConvFunctionBuffer ;

	Wave->SrcBuffer = NULL ;

	// RIFFファイルかどうか調べる
	Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
	if( chunk.fcc != RIFFCHUNKID( 'R', 'I', 'F', 'F' ) )
		goto ERR ;

	// WAVEファイルかどうか調べる
	type[4] = '\0' ;
	Stream->ReadShred.Read( type, 4, 1, Stream->DataPoint ) ;
	if( _STRCMP( type, "WAVE" ) != 0 )
		goto ERR ;

	// ノーマルなＰＣＭ形式か調べる
	{
		// 拡張情報があったらスキップする
		Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
		if( chunk.fcc == RIFFCHUNKID( 'b', 'e', 'x', 't' ) )
		{
			Stream->ReadShred.Seek( Stream->DataPoint, chunk.cb, SEEK_CUR ) ;
			Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
		}
		if( chunk.fcc != RIFFCHUNKID( 'f', 'm', 't', ' ' ) )
			goto ERR ;

		format = (WAVEFORMATEX *)DXALLOC( chunk.cb ) ;
		if( format == NULL ) goto ERR ;

		// ヘッダの位置とサイズをセット
		SoundConv->HeaderPos = ( int )Stream->ReadShred.Tell( Stream->DataPoint ) ;
		SoundConv->HeaderSize = chunk.cb ;

		_MEMSET( SoundConv->ConvFunctionBuffer, 0, sizeof( SoundConv->ConvFunctionBuffer ) ) ;

		_MEMSET( &SoundConv->OutFormat, 0, sizeof( SoundConv->OutFormat ) ) ;
		Stream->ReadShred.Read( format, chunk.cb, 1, Stream->DataPoint ) ;
		switch( format->wFormatTag )
		{
		case WAVE_FORMAT_PCM :

			// 形式が WAVEFORMATEX ではなかった場合は補完する
			_MEMCPY( &SoundConv->OutFormat, format, chunk.cb ) ;
			if( chunk.cb == 16 )
			{
				SoundConv->OutFormat.wBitsPerSample = format->nBlockAlign / format->nChannels * 8 ;
				SoundConv->OutFormat.cbSize = 0 ;
			}
			break ;


		case WAVE_FORMAT_IEEE_FLOAT :

			// 出力フォーマットをセット
			SoundConv->OutFormat.nSamplesPerSec = format->nSamplesPerSec ;
			SoundConv->OutFormat.nChannels = format->nChannels ;
			SoundConv->OutFormat.wBitsPerSample = 16 ;
			SoundConv->OutFormat.wFormatTag = WAVE_FORMAT_PCM ;
			SoundConv->OutFormat.nBlockAlign = SoundConv->OutFormat.wBitsPerSample  * SoundConv->OutFormat.nChannels / 8 ;
			SoundConv->OutFormat.nAvgBytesPerSec = SoundConv->OutFormat.nBlockAlign * SoundConv->OutFormat.nSamplesPerSec ;

			Wave->SrcSampleNum = SoundConv->OutFormat.nSamplesPerSec ;
			Wave->SrcBufferSize = format->nAvgBytesPerSec ;
			Wave->SrcBuffer = ( BYTE * )DXALLOC( Wave->SrcBufferSize ) ;
			if( Wave->SrcBuffer == NULL )
				goto ERR ;
			break ;


		default :
			goto ERR ;
		}

		// フォーマットのコピー
		_MEMCPY( &Wave->SrcFormat, format, chunk.cb ) ;

		// メモリの解放
		DXFREE( format ) ;
		format = NULL ;
	}

	// データチャンクを探す
	while( Stream->ReadShred.Eof( Stream->DataPoint ) == 0 )
	{
		Stream->ReadShred.Read( &chunk , sizeof( chunk ) , 1 , Stream->DataPoint ) ;
		if( chunk.fcc == RIFFCHUNKID( 'd', 'a', 't', 'a' ) ) break ;
		Stream->ReadShred.Seek( Stream->DataPoint, chunk.cb + ( ( chunk.cb % 2 != 0 ) ? 1 : 0 ) , SEEK_CUR ) ; 
	}
	if( Stream->ReadShred.Eof( Stream->DataPoint ) != 0 )
		goto ERR ;

	SoundConv->DataPos = ( int )Stream->ReadShred.Tell( Stream->DataPoint ) ;
	SoundConv->DataSize = ( int )chunk.cb ;

	// 変換後のＰＣＭデータを一時的に保存するメモリ領域のサイズをセット
	SoundConv->DestDataSize = SoundConv->OutFormat.nAvgBytesPerSec ;

	// タイプセット
	SoundConv->MethodType = SOUND_METHODTYPE_NORMAL ;

	// 正常終了
	return 0 ;

ERR :
	if( Wave->SrcBuffer != NULL )
	{
		DXFREE( Wave->SrcBuffer ) ;
		Wave->SrcBuffer = NULL ;
	}

	if( format != NULL )
	{
		DXFREE( format ) ;
		format = NULL ;
	}

	return -1 ;
}

#ifndef DX_NON_ACM

#ifdef DX_GCC_COMPILE
#define ACM_FORMATSUGGESTF_WFORMATTAG       0x00010000L
#define ACM_FORMATSUGGESTF_NCHANNELS        0x00020000L
#define ACM_FORMATSUGGESTF_NSAMPLESPERSEC   0x00040000L
#define ACM_FORMATSUGGESTF_WBITSPERSAMPLE   0x00080000L

#define ACM_STREAMOPENF_QUERY           0x00000001
#define ACM_STREAMOPENF_ASYNC           0x00000002
#define ACM_STREAMOPENF_NONREALTIME     0x00000004
#endif

// ＡＣＭファイルのセットアップ処理を行う( [戻] -1:エラー )
static	int SetupSoundConvert_ACM( SOUNDCONV *SoundConv )
{
	___RIFFCHUNK chunk ;
	char type[5] ;
	STREAMDATA *Stream = &SoundConv->Stream ;
//	SOUNDCONV_ACM *acmdata = &SoundConv->AcmTypeData ;
	SOUNDCONV_ACM *acmdata = (SOUNDCONV_ACM *)SoundConv->ConvFunctionBuffer ;
	int res, i ;
	
	acmdata->Format = NULL ;
	acmdata->AcmStreamHandle[0] = NULL ;
	acmdata->AcmStreamHandle[1] = NULL ;
	acmdata->SrcData = NULL ;
	acmdata->DestDataSampleNum = -1 ;
	acmdata->BeginSeekPosition = 0 ;
	acmdata->BeginSeekCompSrcSize = 0 ;
	acmdata->BeginSeekBuffer = NULL ;

	// RIFFファイルかどうか調べる
	Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
	if( chunk.fcc != RIFFCHUNKID( 'R', 'I', 'F', 'F' ) )
		goto ERR ;

	// WAVEファイルかどうか調べる
	type[4] = '\0' ;
	Stream->ReadShred.Read( type, 4, 1, Stream->DataPoint ) ;
	if( _STRCMP( type, "WAVE" ) != 0 )
		goto ERR ;

	// フォーマットを取得する
	{
		// 拡張情報があったらスキップする
		Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
		if( chunk.fcc == RIFFCHUNKID( 'b', 'e', 'x', 't' ) )
		{
			Stream->ReadShred.Seek( Stream->DataPoint, chunk.cb, SEEK_CUR ) ;
			Stream->ReadShred.Read( &chunk, sizeof( chunk ), 1, Stream->DataPoint ) ;
		}
		if( chunk.fcc != RIFFCHUNKID( 'f', 'm', 't', ' ' ) ) goto ERR ;

		acmdata->Format = (WAVEFORMATEX *)DXALLOC( chunk.cb ) ;
		if( acmdata->Format == NULL ) goto ERR ;

		// ヘッダの位置とサイズをセット
		SoundConv->HeaderPos = ( int )Stream->ReadShred.Tell( Stream->DataPoint ) ;
		SoundConv->HeaderSize = chunk.cb ;

		Stream->ReadShred.Read( acmdata->Format, chunk.cb, 1, Stream->DataPoint ) ;

		// 形式が WAVEFORMATEX ではなかった場合は補完する
		if( acmdata->Format->wFormatTag == WAVE_FORMAT_PCM && chunk.cb == 16 )
		{
			acmdata->Format->wBitsPerSample = acmdata->Format->nBlockAlign / acmdata->Format->nChannels * 8 ;
			acmdata->Format->cbSize = 0 ;
		}
	}
	
	// データチャンクを探す
	while( Stream->ReadShred.Eof( Stream->DataPoint ) == 0 )
	{
		Stream->ReadShred.Read( &chunk , sizeof( chunk ) , 1 , Stream->DataPoint ) ;
		if( chunk.fcc == RIFFCHUNKID( 'd', 'a', 't', 'a' ) ) break ;
		Stream->ReadShred.Seek( Stream->DataPoint, chunk.cb + ( ( chunk.cb % 2 != 0 ) ? 1 : 0 ) , SEEK_CUR ) ; 
	}
	
	if( Stream->ReadShred.Eof( Stream->DataPoint ) != 0 )
		goto ERR ;

	// データチャンクの情報をセット
	SoundConv->DataPos = ( int )Stream->ReadShred.Tell( Stream->DataPoint ) ;
	SoundConv->DataSize = chunk.cb ;

	// 変換後の推奨フォーマットを得る
	_MEMSET( &SoundConv->OutFormat, 0, sizeof( SoundConv->OutFormat ) ) ;
	SoundConv->OutFormat.wFormatTag = WAVE_FORMAT_PCM ;
	res = GSoundConvertData.acmFormatSuggestFunc( NULL, acmdata->Format, &SoundConv->OutFormat, sizeof( WAVEFORMATEX ), ACM_FORMATSUGGESTF_WFORMATTAG ) ;
	if( res != 0 ) goto ERR ;
	
	// 圧縮データを一時的に保存するメモリ領域の確保
//	acmdata->SrcDataSize = AdjustSoundDataBlock( acmdata->Format->nAvgBytesPerSec / STS_CACHEBUFFERSEC, acmdata->Format ) ;
	acmdata->SrcDataSize = SoundConv->DataSize ;  // ループ処理の関係で、丸々データが収まる領域を確保する
	acmdata->SrcData = DXALLOC( acmdata->SrcDataSize ) ;
	if( acmdata->SrcData == NULL ) goto ERR ;
//	acmdata->SrcDataValidSize = 0 ;
	acmdata->SrcDataPosition = 0 ;

	// 圧縮データを一括で読み込む
	Stream->ReadShred.Read( acmdata->SrcData, SoundConv->DataSize, 1, Stream->DataPoint ) ;
	Stream->ReadShred.Seek( Stream->DataPoint, SoundConv->DataPos, STREAM_SEEKTYPE_SET ) ;
//	acmdata->SrcDataValidSize = SoundConv->DataSize ;
	
	// ＡＣＭハンドルの作成
	for( i = 0 ; i < 2 ; i ++ )
	{
		res = GSoundConvertData.acmStreamOpenFunc( &acmdata->AcmStreamHandle[i],
								NULL,
								acmdata->Format,
								&SoundConv->OutFormat,
								NULL,
								0,
								0,
								ACM_STREAMOPENF_NONREALTIME ) ;
		if( res != 0 ) goto ERR ;
	}

	// 変換後のＰＣＭデータを一時的に保存するメモリ領域のサイズをセット
	SoundConv->DestDataSize = SoundConv->OutFormat.nAvgBytesPerSec ;

	// シーク予定位置まで変換しておく処理で使用するメモリ領域の確保
	acmdata->BeginSeekBuffer = DXALLOC( SoundConv->DestDataSize ) ;
	if( acmdata->BeginSeekBuffer == NULL ) goto ERR ;

	// タイプセット
	SoundConv->MethodType = SOUND_METHODTYPE_ACM ;

	// 正常終了
	return 0 ;

ERR :
	if( acmdata->AcmStreamHandle[0] != NULL )
	{
		GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[0], 0 ) ;
		acmdata->AcmStreamHandle[0] = NULL ;
	}

	if( acmdata->AcmStreamHandle[1] != NULL )
	{
		GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[1], 0 ) ;
		acmdata->AcmStreamHandle[1] = NULL ;
	}

	if( acmdata->BeginSeekBuffer != NULL )
	{
		DXFREE( acmdata->BeginSeekBuffer ) ;
		acmdata->BeginSeekBuffer = NULL ;
	}

	if( acmdata->Format != NULL )
	{
		DXFREE( acmdata->Format ) ;
		acmdata->Format = NULL ;
	}

	if( acmdata->SrcData != NULL )
	{
		DXFREE( acmdata->SrcData ) ;
		acmdata->SrcData = NULL ;
	}
	
	return -1 ;
}

#endif


#ifndef DX_NON_ACM

// ＭＰ３のヘッダファイルを作成するときに必要なデータの定義

// ビットレート情報の取得
int MP3BitRateTable[2][3][14] =
{
	// ＭＰＥＧ－１
	{
		// Layer-1
		32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448,

		// Layer-2
		32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384,

		// Layer-3
		32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320,
	},

	// ＭＰＥＧ－２
	{
		// Layer-1
		32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256,

		// Layer-2
		8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 114, 160,

		// Later-3
		8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 114, 160,
	},
};

// サンプリング周波数
int MP3FreqTable[2][3] =
{
	// ＭＰＥＧ－１
	44100,48000,32000,

	// ＭＰＥＧ－２
	22050,24000,16000,
};

// フレームヘッダ構造体
struct MP3_FRAMEHEADER
{
	BYTE Layer ;			// 0:Layer-1 1:Layer-2 2:Layer-3
	BYTE MpegV ;			// 0:MPEG1 1:MPEG2
	BYTE BitRateIndex ;
	BYTE SampleRateIndex ;
	BYTE PaddingBit ;
	BYTE ChannelNum ;
	DWORD SampleRate ;
	DWORD BitRate ;
	DWORD FrameSize ;
	DWORD SampleNum ;
} ;

static int AnalysisMP3_FrameHeader( BYTE *Header, MP3_FRAMEHEADER *FrHead )
{
	// ＭＰ３データか調べる
	if( Header[0] != 0xff || ( Header[1] & 0xf0 ) != 0xf0 ) return -2 ;

	// ＭＰＥＧのバージョンを得る(0:Ver1 1:Ver2)
	FrHead->MpegV = ( Header[1] & 0x8 ) == 0x8 ? 0 : 1 ;

	// レイヤを得る(0:Layer-1 1:Layer-2 2:Layer-3)
	FrHead->Layer = 3 - ( ( Header[1] >> 1 ) & 0x3 ) ;
	if( FrHead->Layer > 2 ) return -1 ;

	// ビットレートのインデックス値を得る
	FrHead->BitRateIndex = ( Header[2] >> 4 ) & 0xf ;
	if( FrHead->BitRateIndex > 14 ) return -1 ;

	// ビットレートを得る
	FrHead->BitRate = MP3BitRateTable[FrHead->MpegV][FrHead->Layer][FrHead->BitRateIndex-1] ;

	// サンプリング周波数のインデックス値を得る
	FrHead->SampleRateIndex = ( Header[2] >> 2 ) & 0x3 ;
	if( FrHead->SampleRateIndex > 2 ) return -1 ;

	// サンプリング周波数を得る
	FrHead->SampleRate = MP3FreqTable[FrHead->MpegV][FrHead->SampleRateIndex] ;

	// パディングビットを得る
	FrHead->PaddingBit = ( Header[2] >> 1 ) & 1 ;

	// チャンネルモードからチャンネル数を得る
	FrHead->ChannelNum = ( ( Header[3] >> 6 ) & 0x3 ) == 3 ? 1 : 2 ;
	
	// フレームサイズを計算する
	switch( FrHead->MpegV )
	{
	case 0 : // Mapg-1
		switch( FrHead->Layer )
		{
		case 0 :		// Layer-1
			FrHead->FrameSize = ( ( 12 * ( FrHead->BitRate * 1000 ) / FrHead->SampleRate ) + FrHead->PaddingBit ) * 4 ;
			FrHead->SampleNum = 12 * 8 ;
			break ;
			
		case 1 :		// Layer-2 Layer-3
		case 2 :
			FrHead->FrameSize = ( 144 * ( FrHead->BitRate * 1000 ) / FrHead->SampleRate ) + FrHead->PaddingBit ;
			FrHead->SampleNum = 144 * 8 ;
			break ;
		}
		break ;

	case 1 : // Mapg-2
		switch( FrHead->Layer )
		{
		case 0 :		// Layer-1
			FrHead->FrameSize = ( ( 12 * ( FrHead->BitRate * 1000 ) / FrHead->SampleRate ) + FrHead->PaddingBit ) * 4 ;
			FrHead->SampleNum = 12 * 8 ;
			break ;

		case 1 :		// Layer-2 Layer-3
		case 2 :
			FrHead->FrameSize = ( 72 * ( FrHead->BitRate * 1000 ) / FrHead->SampleRate ) + FrHead->PaddingBit ;
			FrHead->SampleNum = 72 * 8 ;
			break ;
		}
		break ;
	}

	// 正常終了	
	return 0 ;
}

// ＭＰ３ファイルのセットアップ処理を行う( [戻] -1:エラー )
static int SetupSoundConvert_MP3( SOUNDCONV *SoundConv )
{
	BYTE Header[4] ;
//	BYTE Layer, MpegV, BitRateIndex, SampleRateIndex ;
//	BYTE PaddingBit, ChannelNum ;
//	DWORD SampleRate, BitRate, FrameSize ;
	MPEGLAYER3WAVEFORMAT *MP3Format = NULL ;
	MP3_FRAMEHEADER MP3Frame ;
	int DataSize, FileSize, /*StartPoint,*/ res, i;
	DWORD LastPoint ;
	DWORD SkipSize, EndSkipSize, StartPos ;
//	SOUNDCONV_ACM *acmdata = &SoundConv->AcmTypeData ;
	SOUNDCONV_ACM *acmdata = (SOUNDCONV_ACM *)SoundConv->ConvFunctionBuffer ;
	STREAMDATA *Stream = &SoundConv->Stream ;
	

	acmdata->Format = NULL ;
	acmdata->SrcData = NULL ;
	acmdata->DestDataSampleNum = 0 ;
	acmdata->BeginSeekPosition = 0 ;
	acmdata->BeginSeekCompSrcSize = 0 ;

	// ファイルサイズを得る
	Stream->ReadShred.Seek( Stream->DataPoint, 0, STREAM_SEEKTYPE_END ) ;
	FileSize = ( int )Stream->ReadShred.Tell( Stream->DataPoint ) ;
	Stream->ReadShred.Seek( Stream->DataPoint, 0, STREAM_SEEKTYPE_SET ) ;

	// ID3v2 タグがある場合のスキップ処理を行う
	{
		char Tag[4] ;
		BYTE MainVer, SubVer, Flag, SizeData[4] ;

		Tag[3] = '\0' ;
		Stream->ReadShred.Read( Tag, 1, 3, Stream->DataPoint ) ;
		if( _STRCMP( Tag, "ID3" ) == 0 )
		{
			// バージョンを得る
			Stream->ReadShred.Read( &MainVer, 1, 1, Stream->DataPoint ) ;
			Stream->ReadShred.Read( &SubVer, 1, 1, Stream->DataPoint ) ;

			// フラグを得る
			Stream->ReadShred.Read( &Flag, 1, 1, Stream->DataPoint ) ;

			// 容量情報を得る
			Stream->ReadShred.Read( SizeData, 1, 4, Stream->DataPoint ) ;
			SkipSize = ( SizeData[0] << 21 ) + ( SizeData[1] << 14 ) + ( SizeData[2] << 7 ) + SizeData[3] + 10 ;

			// フッタ情報がある場合は更に１０バイト切る
			if( MainVer == 4 && ( Flag & 0x10 ) != 0 ) SkipSize += 10 ;
		}
		else
		{
			SkipSize = 0 ;
		}
	}

	// ファイルの終端を調べる。主に ID3v1 タグがあるかどうかで変化する
	if( FileSize > 128 )
	{
		char Tag[4] ;

		Tag[3] = '\0' ;
		Stream->ReadShred.Seek( Stream->DataPoint, -128, STREAM_SEEKTYPE_END ) ;
		Stream->ReadShred.Read( Tag, 1, 3, Stream->DataPoint ) ;

		if( _STRCMP( Tag, "TAG" ) == 0 )
		{
			EndSkipSize = 128 ;
		}
		else
		{
			EndSkipSize = 0 ;
		}
	}

	// データの末端を検索する
	{
		// シークする
		Stream->ReadShred.Seek( Stream->DataPoint, SkipSize, STREAM_SEEKTYPE_SET ) ;

		StartPos = 0xffffffff ;

		i = 0;
		for(;;)
		{
			for(;;)
			{
				if( Stream->ReadShred.Read( &Header[0], 1, 1, Stream->DataPoint ) == 0 ) goto R1 ;
				if( Header[0] == 0xff )
				{
					if( Stream->ReadShred.Read( &Header[1], 1, 1, Stream->DataPoint ) == 0 ) goto R1 ;
					if( ( Header[1] & 0xf0 ) == 0xf0 )
					{
						break ;
					}
				}
			}
			
			// 残りのヘッダを読み込む
			Stream->ReadShred.Read( &Header[2], 1, 2, Stream->DataPoint ) ;

			// ヘッダを解析
			if( AnalysisMP3_FrameHeader( Header, &MP3Frame ) < 0 ) return -1 ;

			// もしフレームサイズが0だったらエラー
			if( MP3Frame.FrameSize <= 0 ) return -1 ;

			// もし最初のフレームだったら位置を保存しておく
			if( StartPos == 0xffffffff )
			{
				StartPos = ( DWORD )Stream->ReadShred.Tell( Stream->DataPoint ) - 4 ;
			}
/*
			// ＭＰ３データか調べる
			if( Header[0] != 0xff || ( Header[1] & 0xf0 ) != 0xf0 ) continue ;

			// ＭＰＥＧのバージョンを得る(0:Ver1 1:Ver2)
			MpegV = ( Header[1] & 0x8 ) == 0x8 ? 0 : 1 ;

			// レイヤを得る(0:Layer-1 1:Layer-2 2:Layer-3)
			Layer = 3 - ( ( Header[1] >> 1 ) & 0x3 ) ;
			if( Layer > 2 ) return -1 ;

			// ビットレートのインデックス値を得る
			BitRateIndex = ( Header[2] >> 4 ) & 0xf ;
			if( BitRateIndex > 14 ) return -1 ;

			// ビットレートを得る
			BitRate = MP3BitRateTable[MpegV][Layer][BitRateIndex-1] ;

			// サンプリング周波数のインデックス値を得る
			SampleRateIndex = ( Header[2] >> 2 ) & 0x3 ;
			if( SampleRateIndex > 2 ) return -1 ;

			// サンプリング周波数を得る
			SampleRate = MP3FreqTable[MpegV][SampleRateIndex] ;

			// パディングビットを得る
			PaddingBit = ( Header[2] >> 1 ) & 1 ;

			// チャンネルモードからチャンネル数を得る
			ChannelNum = ( ( Header[3] >> 6 ) & 0x3 ) == 3 ? 1 : 2 ;
			
			// もし最初のフレームだったら位置を保存しておく
			if( StartPos == 0xffffffff )
			{
				StartPos = Stream->ReadShred.Tell( Stream->DataPoint ) - 4 ;
			}

			// フレームサイズを計算する
			switch( MpegV )
			{
			case 0 : // Mapg-1
				switch( Layer )
				{
				case 0 : FrameSize = ( ( 12000 * BitRate / SampleRate ) + PaddingBit ) * 4 ; break ;		// Layer-1
				case 1 :
				case 2 : FrameSize = ( 144000 * BitRate / SampleRate ) + PaddingBit ; break ;				// Layer-2 Layer-3
				}
				break ;

			case 1 : // Mapg-2
				switch( Layer )
				{
				case 0 : FrameSize = ( ( 12000 * BitRate / SampleRate ) + PaddingBit ) * 4 ; break ;		// Layer-1
				case 1 :
				case 2 : FrameSize = ( 72000 * BitRate / SampleRate ) + PaddingBit ; break ;				// Layer-2 Layer-3
				}
				break ;
			}
//			FrameSize += 4 ;
*/
			// 総サンプル数を増やす
			acmdata->DestDataSampleNum += MP3Frame.SampleNum ;

			// データの末端アドレスを取得
			{
				LastPoint = ( DWORD )( Stream->ReadShred.Tell( Stream->DataPoint ) /*- 8*/ + MP3Frame.FrameSize - 4 ) ;
				if( LastPoint >= FileSize - EndSkipSize )
				{
					if( LastPoint > FileSize - EndSkipSize ) LastPoint = FileSize - EndSkipSize;
					break ;
				}
				Stream->ReadShred.Seek( Stream->DataPoint, MP3Frame.FrameSize - 4 /*- 8*/, STREAM_SEEKTYPE_CUR ) ;
				i ++ ;
			}
		}
	}

R1 :
	// フレームデータが見つからなかったらエラー
	if( StartPos == 0xffffffff ) return -1 ;

	// 検索位置をセットする
/*	Stream->ReadShred.Seek( Stream->DataPoint, SkipSize, STREAM_SEEKTYPE_SET ) ;

	// フレームデータを探す
	{
		while( 1 )
		{
			if( Stream->ReadShred.Read( &Header[0], 1, 1, Stream->DataPoint ) == 0 ) return -1 ;
			if( Header[0] == 0xff )
			{
				if( Stream->ReadShred.Read( &Header[1], 1, 1, Stream->DataPoint ) == 0 ) return -1 ;
				if( ( Header[1] & 0xf0 ) == 0xf0 )
				{
					break ;
				}
			}
		}
	}

	// 残りのヘッダを読み込む
	Stream->ReadShred.Read( &Header[2], 1, 2, Stream->DataPoint ) ;
*/
	// 最初のフレームの位置へ移動
	Stream->ReadShred.Seek( Stream->DataPoint, StartPos, STREAM_SEEKTYPE_SET ) ;

	// ヘッダを読み込む
	Stream->ReadShred.Read( &Header[0], 1, 4, Stream->DataPoint ) ;

	// ヘッダを解析
	if( AnalysisMP3_FrameHeader( Header, &MP3Frame ) < 0 ) return -1 ;
/*
	// ＭＰ３データか調べる
	if( Header[0] != 0xff || ( Header[1] & 0xf0 ) != 0xf0 ) return -1 ;

	// ＭＰＥＧのバージョンを得る(0:Ver1 1:Ver2)
	MpegV = ( Header[1] & 0x8 ) == 0x8 ? 0 : 1 ;

	// レイヤを得る(0:Layer-1 1:Layer-2 2:Layer-3)
	Layer = 3 - ( ( Header[1] >> 1 ) & 0x3 ) ;

	// ビットレートのインデックス値を得る
	BitRateIndex = ( Header[2] >> 4 ) & 0xf ;

	// ビットレートを得る
	BitRate = MP3BitRateTable[MpegV][Layer][BitRateIndex-1] ;

	// サンプリング周波数のインデックス値を得る
	SampleRateIndex = ( Header[2] >> 2 ) & 0x3 ;

	// サンプリング周波数を得る
	SampleRate = MP3FreqTable[MpegV][SampleRateIndex] ;

	// パディングビットを得る
	PaddingBit = ( Header[2] >> 1 ) & 1 ;

	// チャンネルモードからチャンネル数を得る
	ChannelNum = ( ( Header[3] >> 6 ) & 0x3 ) == 3 ? 1 : 2 ;

	// フレームサイズを計算する
	switch( MpegV )
	{
	case 0 : // Mapg-1
		switch( Layer )
		{
		case 0 : FrameSize = ( ( 12000 * BitRate / SampleRate ) + PaddingBit ) * 4 ; break ;		// Layer-1
		case 1 :
		case 2 : FrameSize = ( 144000 * BitRate / SampleRate ) + PaddingBit ; break ;				// Layer-2 Layer-3
		}
		break ;

	case 1 : // Mapg-2
		switch( Layer )
		{
		case 0 : FrameSize = ( ( 12000 * BitRate / SampleRate ) + PaddingBit ) * 4 ; break ;		// Layer-1
		case 1 :
		case 2 : FrameSize = ( 72000 * BitRate / SampleRate ) + PaddingBit ; break ;				// Layer-2 Layer-3
		}
		break ;
	}
//	FrameSize += 4 ;
*/
	// ヘッダのセット
	{
		// メモリの確保
		int headerSize = sizeof( MPEGLAYER3WAVEFORMAT );
		GSoundConvertData.acmMetricsFunc( NULL, ACM_METRIC_MAX_SIZE_FORMAT, &headerSize );

		MP3Format = ( MPEGLAYER3WAVEFORMAT * )DXALLOC( headerSize ) ;
		if( MP3Format == NULL ) goto ERR ;

		// ＭＰ３ＷＡＶＥフォーマットのデータをセットする
		_MEMSET( MP3Format, 0, headerSize ) ;
		MP3Format->wfx.cbSize = MPEGLAYER3_WFX_EXTRA_BYTES ;
		MP3Format->wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3 ;
		MP3Format->wfx.nChannels = MP3Frame.ChannelNum ;
		MP3Format->wfx.nSamplesPerSec = MP3Frame.SampleRate ;
		MP3Format->wfx.nAvgBytesPerSec = MP3Frame.BitRate * 1000 / 8 ;
		MP3Format->wfx.nBlockAlign = 1 ;
		MP3Format->wfx.wBitsPerSample = 0 ;
		MP3Format->wID = MPEGLAYER3_ID_MPEG ;
		MP3Format->fdwFlags = MP3Frame.PaddingBit/*0*//*MPEGLAYER3_FLAG_PADDING_OFF*/ ;
		MP3Format->nBlockSize = ( WORD )MP3Frame.FrameSize ;
		MP3Format->nFramesPerBlock = 1 ;
		MP3Format->nCodecDelay = 700/*0x0571*/ ;
	}
/*
	// データのスタートアドレスを取得
	StartPoint = Stream->ReadShred.Tell( Stream->DataPoint ) - 4 ;
*/

	// 有効データサイズを取得
	DataSize = LastPoint - StartPos ;
//	DataSize = LastPoint - StartPoint ;
//	DataSize = FileSize - SkipSize - EndSkipSize ;

	// ファイルポインタの操作
	Stream->ReadShred.Seek( Stream->DataPoint, StartPos, STREAM_SEEKTYPE_SET ) ;
//	Stream->ReadShred.Seek( Stream->DataPoint, StartPoint, STREAM_SEEKTYPE_SET ) ;

	// 情報のセット
	acmdata->Format = ( WAVEFORMATEX * )MP3Format ;
//	SoundConv->DataPos = StartPoint ;
	SoundConv->DataPos = StartPos ;
	SoundConv->DataSize = DataSize ;

	// 変換後の推奨フォーマットを得る
	_MEMSET( &SoundConv->OutFormat, 0, sizeof( SoundConv->OutFormat ) ) ;
	SoundConv->OutFormat.wFormatTag = WAVE_FORMAT_PCM ;
	res = GSoundConvertData.acmFormatSuggestFunc( NULL, acmdata->Format, &SoundConv->OutFormat, sizeof( WAVEFORMATEX ), ACM_FORMATSUGGESTF_WFORMATTAG ) ;
	if( res != 0 ) goto ERR ;

	// 圧縮データを一時的に保存するメモリ領域の確保
//	acmdata->SrcDataSize = AdjustSoundDataBlock( acmdata->Format->nAvgBytesPerSec * 8/* / STS_CACHEBUFFERSEC*/, acmdata->Format ) ;
	acmdata->SrcDataSize = SoundConv->DataSize ;  // ループ処理の関係で、丸々データが収まる領域を確保する
	acmdata->SrcData = DXALLOC( acmdata->SrcDataSize ) ;
	if( acmdata->SrcData == NULL ) goto ERR ;
//	acmdata->SrcDataValidSize = 0 ;

	// 圧縮データを一括で読み込む
	Stream->ReadShred.Read( acmdata->SrcData, SoundConv->DataSize, 1, Stream->DataPoint ) ;
	Stream->ReadShred.Seek( Stream->DataPoint, StartPos, STREAM_SEEKTYPE_SET ) ;
//	acmdata->SrcDataValidSize = SoundConv->DataSize ;
	acmdata->SrcDataPosition = 0 ;

	// ＡＣＭハンドルの作成
	for( i = 0 ; i < 2 ; i ++ )
	{
		res = GSoundConvertData.acmStreamOpenFunc( &acmdata->AcmStreamHandle[i],
								NULL,
								acmdata->Format,
								&SoundConv->OutFormat,
								NULL,
								0,
								0,
								ACM_STREAMOPENF_NONREALTIME ) ;
		if( res != 0 ) goto ERR ;
	}

	// 変換後のＰＣＭデータを一時的に保存するメモリ領域のサイズをセット(１秒分よりも少し大きめにとっておく)
	SoundConv->DestDataSize = SoundConv->OutFormat.nAvgBytesPerSec * 4 / 3;
	
	// シーク予定位置まで変換しておく処理で使用するメモリ領域の確保
	acmdata->BeginSeekBuffer = DXALLOC( SoundConv->DestDataSize ) ;
	if( acmdata->BeginSeekBuffer == NULL ) goto ERR ;

	// タイプセット
	SoundConv->MethodType = SOUND_METHODTYPE_ACM ;

	// 終了
	return 0 ;
	
ERR :
	if( acmdata->AcmStreamHandle[0] != NULL )
	{
		GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[0], 0 ) ;
		acmdata->AcmStreamHandle[0] = NULL ;
	}

	if( acmdata->AcmStreamHandle[1] != NULL )
	{
		GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[1], 0 ) ;
		acmdata->AcmStreamHandle[1] = NULL ;
	}

	if( acmdata->BeginSeekBuffer != NULL )
	{
		DXFREE( acmdata->BeginSeekBuffer ) ;
		acmdata->BeginSeekBuffer = NULL ;
	}

	if( acmdata->Format != NULL )
	{
		DXFREE( acmdata->Format ) ;
		acmdata->Format = NULL ;
	}

	if( acmdata->SrcData != NULL )
	{
		DXFREE( acmdata->SrcData ) ;
		acmdata->SrcData = NULL ;
	}
	
	return -1 ;
}

#endif

// ＷＡＶＥファイルの後始末処理を行う
static	int TerminateSoundConvert_WAVE( SOUNDCONV *SoundConv )
{
	SOUNDCONV_WAVE *Wave = ( SOUNDCONV_WAVE * )SoundConv->ConvFunctionBuffer ;

	// メモリ解放
	if( Wave->SrcBuffer )
	{
		DXFREE( Wave->SrcBuffer ) ;
		Wave->SrcBuffer = NULL ;
	}

	// 終了
	return 0 ;
}

#ifndef DX_NON_ACM

// ＡＣＭを使用したファイルの後始末処理を行う
static	int TerminateSoundConvert_ACM( SOUNDCONV *SoundConv )
{
//	SOUNDCONV_ACM *acmdata = &SoundConv->AcmTypeData ;
	SOUNDCONV_ACM *acmdata = (SOUNDCONV_ACM *)SoundConv->ConvFunctionBuffer ;

	if( acmdata->Format != NULL )
	{
		DXFREE( acmdata->Format ) ;
		acmdata->Format = NULL ;
	}

	if( acmdata->BeginSeekBuffer != NULL )
	{
		DXFREE( acmdata->BeginSeekBuffer ) ;
		acmdata->BeginSeekBuffer = NULL ;
	}
	
	if( acmdata->AcmStreamHandle[0] != NULL )
	{
		GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[0], 0 ) ;
		acmdata->AcmStreamHandle[0] = NULL ;
	}

	if( acmdata->AcmStreamHandle[1] != NULL )
	{
		GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[1], 0 ) ;
		acmdata->AcmStreamHandle[1] = NULL ;
	}
	
	if( acmdata->SrcData != NULL )
	{
		DXFREE( acmdata->SrcData ) ;
		acmdata->SrcData = NULL ;
	}

	return 0 ;
}

#endif

#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3

// MP3を使用したファイルの後始末処理を行う
static int TerminateSoundConvert_DSMP3( SOUNDCONV *SoundConv )
{
	SOUNDCONV_DSMP3 *dsmp3 = ( SOUNDCONV_DSMP3 * )SoundConv->ConvFunctionBuffer ;

	if( dsmp3->PCMBuffer )
	{
		DXFREE( dsmp3->PCMBuffer ) ;
		dsmp3->PCMBuffer = NULL ;
	}

	return 0 ;
}

#endif
#endif
#endif

// 変換後のバッファにデータを補充する
static	int ConvertProcessSoundConvert_WAVE( SOUNDCONV *SoundConv )
{
	int readsize, pos ;
	STREAMDATA *Stream = &SoundConv->Stream ;
	SOUNDCONV_WAVE *Wave = ( SOUNDCONV_WAVE * )SoundConv->ConvFunctionBuffer ;

	switch( Wave->SrcFormat.wFormatTag )
	{
	case WAVE_FORMAT_PCM :
		pos = ( int )( Stream->ReadShred.Tell( Stream->DataPoint ) - SoundConv->DataPos ) ;
		if( pos == SoundConv->DataSize ) return -1 ;

		// 読み込むデータサイズを決定する
		readsize = SoundConv->DataSize - pos ;
		if( SoundConv->DestDataSize < readsize ) readsize = SoundConv->DestDataSize ;

		// 読み込む
		Stream->ReadShred.Read( SoundConv->DestData,
								readsize, 1, Stream->DataPoint ) ;
		SoundConv->DestDataValidSize = readsize ;
		break ;

	case WAVE_FORMAT_IEEE_FLOAT :
		{
			short *Dest ;
			float *Src ;
			DWORD i ;
			DWORD SampleNum ;
			int j ;
			int DestI ;

			pos = ( int )( Stream->ReadShred.Tell( Stream->DataPoint ) - SoundConv->DataPos ) ;
			if( pos == SoundConv->DataSize ) return -1 ;

			// 読み込むデータサイズを決定する
			readsize = SoundConv->DataSize - pos ;
			if( Wave->SrcBufferSize < ( DWORD )readsize ) readsize = Wave->SrcBufferSize ;

			// 読み込む
			Stream->ReadShred.Read( Wave->SrcBuffer,
									readsize, 1, Stream->DataPoint ) ;

			// 整数型に変換
			Dest = ( short * )SoundConv->DestData ;
			Src = ( float * )Wave->SrcBuffer ;
			SampleNum = readsize / Wave->SrcFormat.nBlockAlign ;
			for( i = 0 ; i < SampleNum ; i ++ )
			{
				for( j = 0 ; j < Wave->SrcFormat.nChannels ; j ++, Dest ++, Src ++ )
				{
					DestI = _FTOL( *Src * 32768.0f ) ;
					if( DestI < -32768 )
					{
						*Dest = -32768 ;
					}
					else
					if( DestI > 32767 )
					{
						*Dest = 32767 ;
					}
					else
					{
						*Dest = ( short )DestI ;
					}
				}
			}
			SoundConv->DestDataValidSize = SampleNum * SoundConv->OutFormat.nBlockAlign ;
		}
		break ;
	}

	return 0 ;
}

#ifndef DX_NON_ACM

// 変換後のバッファにデータを補充する
static	int ConvertProcessSoundConvert_ACM( SOUNDCONV *SoundConv )
{
	ACMSTREAMHEADER header ;
//	SOUNDCONV_ACM *acmdata = &SoundConv->AcmTypeData ;
	SOUNDCONV_ACM *acmdata = (SOUNDCONV_ACM *)SoundConv->ConvFunctionBuffer ;
//	STREAMDATA *Stream = &SoundConv->Stream ;
	DWORD ConvSize = 0 ;

	// シーク予定位置とサブACMハンドルで変換が完了している位置が違う場合は処理を行う
	if( SoundConv->SeekLockPosition != acmdata->BeginSeekPosition )
	{
		// 追い越してしまっている場合は最初からやり直す
		if( acmdata->BeginSeekPosition > SoundConv->SeekLockPosition )
		{
			GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[1], 0 ) ;
			GSoundConvertData.acmStreamOpenFunc( &acmdata->AcmStreamHandle[1], NULL,
							acmdata->Format, &SoundConv->OutFormat,
							NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ) ;
			acmdata->BeginSeekPosition = 0 ;
			acmdata->BeginSeekCompSrcSize = 0 ;
		}

		if( SoundConv->SeekLockPosition - 1024 * 10 > acmdata->BeginSeekPosition )
		{
			// 変換済みの位置を進める
			_MEMSET( &header, 0, sizeof( ACMSTREAMHEADER ) ) ;
			header.cbStruct		= sizeof( ACMSTREAMHEADER ) ;
			header.pbSrc		= (BYTE *)acmdata->SrcData + acmdata->BeginSeekCompSrcSize ;
			if( acmdata->SrcDataSize - acmdata->BeginSeekCompSrcSize > 2048 ) header.cbSrcLength = 2048 ;
			else 	                                                          header.cbSrcLength = acmdata->SrcDataSize - acmdata->BeginSeekCompSrcSize ;
			header.pbDst		= (BYTE *)acmdata->BeginSeekBuffer ;
			header.cbDstLength	= SoundConv->DestDataSize ;
			if( (signed)( header.cbDstLength + acmdata->BeginSeekPosition ) > SoundConv->SeekLockPosition ) 
				header.cbDstLength = SoundConv->SeekLockPosition - acmdata->BeginSeekPosition ;

			if( GSoundConvertData.acmStreamPrepareHeaderFunc( acmdata->AcmStreamHandle[1], &header, 0 ) != 0 )
				return -1 ;

			// 展開
			GSoundConvertData.acmStreamConvertFunc( acmdata->AcmStreamHandle[1], &header, ACM_STREAMCONVERTF_BLOCKALIGN ) ;

			// 展開が成功した分だけ座標を進める
			acmdata->BeginSeekPosition += header.cbDstLengthUsed ;
			acmdata->BeginSeekCompSrcSize += header.cbSrcLengthUsed ;

			// ヘッダの解除
			GSoundConvertData.acmStreamUnprepareHeaderFunc( acmdata->AcmStreamHandle[1], &header, 0 ) ;
		}
	}

	// 終端まで来ていたら終了
	if( acmdata->SrcDataPosition == acmdata->SrcDataSize )
		return -1 ;

	// 変換元のデータが無かったらストリームから読み込みむ
//(ループ処理の関係でデータを丸々読み込んでいるためこの処理は必要なくなった)
/*	if( acmdata->SrcDataValidSize < acmdata->SrcDataSize )
	{
		int pos, readsize ;

		pos = Stream->ReadShred.Tell( Stream->DataPoint ) - SoundConv->DataPos ; 
		if( pos == SoundConv->DataSize && acmdata->SrcDataValidSize == 0 )
			return -1 ;

		readsize = SoundConv->DataSize - pos ;
		if( readsize != 0 )
		{
			if( readsize > acmdata->SrcDataSize - acmdata->SrcDataValidSize )
				readsize = acmdata->SrcDataSize - acmdata->SrcDataValidSize ;

			Stream->ReadShred.Read( (BYTE *)acmdata->SrcData + acmdata->SrcDataValidSize,
									readsize, 1, Stream->DataPoint ) ;
			acmdata->SrcDataValidSize += readsize ;
		}
	}
*/

	// 展開の準備
	_MEMSET( &header, 0, sizeof( ACMSTREAMHEADER ) ) ;
	header.cbStruct		= sizeof( ACMSTREAMHEADER ) ;
//	header.pbSrc		= (BYTE *)acmdata->SrcData ;
//	header.cbSrcLength	= acmdata->SrcDataValidSize ;
	header.pbSrc		= (BYTE *)acmdata->SrcData + acmdata->SrcDataPosition ;
	if( acmdata->SrcDataSize - acmdata->SrcDataPosition > 2048 ) header.cbSrcLength = 2048 ;
	else 	                                                     header.cbSrcLength = acmdata->SrcDataSize - acmdata->SrcDataPosition ;
	header.pbDst		= (BYTE *)SoundConv->DestData ;
	header.cbDstLength	= SoundConv->DestDataSize ;


	if( GSoundConvertData.acmStreamPrepareHeaderFunc( acmdata->AcmStreamHandle[0], &header, 0 ) != 0 )
		return -1 ;

	// 展開
	GSoundConvertData.acmStreamConvertFunc( acmdata->AcmStreamHandle[0], &header, ACM_STREAMCONVERTF_BLOCKALIGN ) ;

	// データのセット
	ConvSize = header.cbDstLengthUsed ;
/*	if( acmdata->DestDataSampleNum != -1 )
	{
		if( ( SoundConv->DestDataCompSize + ConvSize ) / SoundConv->OutFormat.nBlockAlign >= acmdata->DestDataSampleNum )
		{
			ConvSize = ( acmdata->DestDataSampleNum - SoundConv->DestDataCompSize / SoundConv->OutFormat.nBlockAlign ) * SoundConv->OutFormat.nBlockAlign ;
		}
	}
*/
	SoundConv->DestDataValidSize = header.cbDstLengthUsed ;
	acmdata->SrcDataPosition += header.cbSrcLengthUsed ;
//	acmdata->SrcDataValidSize -= header.cbSrcLengthUsed ;

	// ヘッダの解除
	GSoundConvertData.acmStreamUnprepareHeaderFunc( acmdata->AcmStreamHandle[0], &header, 0 ) ;

	// 使用されたデータのサイズが０で、且つコンバートサイズが０だったら音声が終了している
	// 可能性があるので、その場合はコンバート終了
	if( ConvSize == 0 )
	{
		SoundConv->DestDataValidSize = 0 ;
		acmdata->SrcDataPosition = acmdata->SrcDataSize ;
//		acmdata->SrcDataValidSize = 0 ;
		return -1 ;
	}

	// 残った部分をスライドさせる
//(ループ処理の関係でデータを丸々読み込んでいるためこの処理は必要なくなった)
/*	if( acmdata->SrcDataValidSize != 0 )
		_MEMMOVE( acmdata->SrcData,
				 (BYTE *)acmdata->SrcData + header.cbSrcLengthUsed,
				 acmdata->SrcDataValidSize ) ;
*/

	// 終了
	return 0 ;
}

#endif

#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3

// 変換後のバッファにデータを補充する
static int ConvertProcessSoundConvert_DSMP3( SOUNDCONV *SoundConv )
{
	SOUNDCONV_DSMP3 *dsmp3 = ( SOUNDCONV_DSMP3 * )SoundConv->ConvFunctionBuffer ;
	int readsize ;

	// 全て転送しきっていたら終了
	if( dsmp3->PCMValidDataSize == dsmp3->PCMDestCopySize ) return -1 ;
	
	// 転送するデータサイズを決定する
	readsize = dsmp3->PCMValidDataSize - dsmp3->PCMDestCopySize ;
	if( SoundConv->DestDataSize < readsize ) readsize = SoundConv->DestDataSize ;
	
	// 転送する
	_MEMCPY( SoundConv->DestData, ( BYTE * )dsmp3->PCMBuffer + dsmp3->PCMDestCopySize, readsize ) ;
	SoundConv->DestDataValidSize = readsize ;

	// 転送した分オフセットを進める
	dsmp3->PCMDestCopySize += readsize ;

	return 0 ;
}

#endif
#endif
#endif

// 変換処理の位置を変更する( サンプル単位 )
static int SetSampleTimeSoundConvert_WAVE( SOUNDCONV *SoundConv, int SampleTime )
{
	int BytePos ;
	STREAMDATA *Stream = &SoundConv->Stream ;
	
	BytePos = SampleTime * SoundConv->OutFormat.nBlockAlign + SoundConv->DataPos ;
	Stream->ReadShred.Seek( Stream->DataPoint, BytePos, SEEK_SET ) ;

	SoundConv->DestDataValidSize = 0 ;
	SoundConv->DestDataCompSize = 0 ;
	SoundConv->DestDataCompSizeAll = BytePos ;

	return 0 ;
}

#ifndef DX_NON_ACM

// 変換処理の位置を変更する( サンプル単位 )
static int SetSampleTimeSoundConvert_ACM( SOUNDCONV *SoundConv, int SampleTime )
{
	int BytePos, CurPos, res, DelByte ;
	STREAMDATA *Stream = &SoundConv->Stream ;
//	SOUNDCONV_ACM *acmdata = &SoundConv->AcmTypeData ;
	SOUNDCONV_ACM *acmdata = (SOUNDCONV_ACM *)SoundConv->ConvFunctionBuffer ;

	BytePos = SampleTime * SoundConv->OutFormat.nBlockAlign ;
	CurPos = SoundConv->DestDataCompSizeAll ;
	
	// 丁度現在解凍してある部分の指定だった場合は処理を分岐
	if( BytePos >= CurPos && BytePos <= CurPos + SoundConv->DestDataValidSize )
	{
		DelByte = BytePos - CurPos ;
		SoundConv->DestDataCompSize += DelByte ;
		SoundConv->DestDataCompSizeAll += DelByte ;
		SoundConv->DestDataValidSize -= DelByte ;
		
		// 終了
		return 0 ;
	}
	
	// もう変換処理が済んでしまっている地点に戻る場合はＡＣＭハンドルを作成しなおし、
	// ストリームポイントもデータの最初に戻す
	if( BytePos < CurPos )
	{
		// もう一つのＡＣＭハンドルが指定の位置と同じか、それより位置的に手前である場合はそちらを使用する
		if( BytePos >= acmdata->BeginSeekPosition )
		{
			HACMSTREAM temp ;

			// ＡＣＭハンドルの交換
			temp = acmdata->AcmStreamHandle[0] ;
			acmdata->AcmStreamHandle[0] = acmdata->AcmStreamHandle[1] ;
			acmdata->AcmStreamHandle[1] = temp ;

			// 情報交換
			acmdata->SrcDataPosition = acmdata->BeginSeekCompSrcSize ;
			SoundConv->DestDataCompSize = acmdata->BeginSeekPosition ;
			SoundConv->DestDataCompSizeAll = acmdata->BeginSeekPosition ;
			SoundConv->DestDataValidSize = 0 ;

			// 今までメインだったＡＣＭハンドルは初期化
			GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[1], 0 ) ;
			GSoundConvertData.acmStreamOpenFunc( &acmdata->AcmStreamHandle[1], NULL,
							acmdata->Format, &SoundConv->OutFormat,
							NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ) ;
			acmdata->BeginSeekCompSrcSize = 0 ;
			acmdata->BeginSeekPosition = 0 ;
		}
		else
		{
			// サブも駄目な場合はメインを作り直して一から追うしかない
			GSoundConvertData.acmStreamCloseFunc( acmdata->AcmStreamHandle[0], 0 ) ;
			GSoundConvertData.acmStreamOpenFunc( &acmdata->AcmStreamHandle[0], NULL,
							acmdata->Format, &SoundConv->OutFormat,
							NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ) ;
			Stream->ReadShred.Seek( Stream->DataPoint, SoundConv->DataPos, SEEK_SET ) ;
			acmdata->SrcDataPosition = 0 ;
//			acmdata->SrcDataValidSize = 0 ;

			SoundConv->DestDataValidSize = 0 ;
			SoundConv->DestDataCompSize = 0 ;
			SoundConv->DestDataCompSizeAll = 0 ;
		}
	}
	
	// 指定の位置まで解凍処理を進める
	res = 0 ;
	while( SoundConv->DestDataCompSizeAll + SoundConv->DestDataValidSize < BytePos )
	{
		SoundConv->DestDataCompSizeAll += SoundConv->DestDataValidSize ;
		SoundConv->DestDataValidSize = 0 ;
		SoundConv->DestDataCompSize = 0 ;
		res = ConvertProcessSoundConvert_ACM( SoundConv ) ;
		if( res == -1 ) break ;
	}
	
	// 終端に到達してしまったら変換終了状態にする
	if( res == -1 )
	{
		SoundConv->EndFlag = TRUE ;
		SoundConv->DestDataValidSize = 0 ;
		return 0 ;
	}
	
	DelByte = BytePos - SoundConv->DestDataCompSizeAll ;
	SoundConv->DestDataValidSize -= DelByte ;
	SoundConv->DestDataCompSize += DelByte ;
	SoundConv->DestDataCompSizeAll += DelByte ;

	// 終了
	return 0 ;
}

#endif

#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3

// 変換処理の位置を変更する( サンプル単位 )
static	int SetSampleTimeSoundConvert_DSMP3( SOUNDCONV *SoundConv, int SampleTime )
{
	int BytePos ;
	SOUNDCONV_DSMP3 *dsmp3 = ( SOUNDCONV_DSMP3 * )SoundConv->ConvFunctionBuffer ;
	
	BytePos = SampleTime * SoundConv->OutFormat.nBlockAlign ;

	SoundConv->DestDataValidSize = 0 ;
	SoundConv->DestDataCompSize = 0 ;
	SoundConv->DestDataCompSizeAll = BytePos ;
	dsmp3->PCMDestCopySize = BytePos ;

	return 0 ;
}

#endif
#endif
#endif


// 変換後の大凡のデータサイズを得る
static	int GetSoundConvertDestSize_Fast_WAVE( SOUNDCONV *SoundConv )
{
	SOUNDCONV_WAVE *Wave = ( SOUNDCONV_WAVE * )SoundConv->ConvFunctionBuffer ;

	switch( Wave->SrcFormat.wFormatTag )
	{
	case WAVE_FORMAT_PCM :
		return SoundConv->DataSize ;

	case WAVE_FORMAT_IEEE_FLOAT :
		return SoundConv->DataSize / ( Wave->SrcFormat.wBitsPerSample / SoundConv->OutFormat.wBitsPerSample ) ;
	}

	return -1 ;
}

#ifndef DX_NON_ACM

// 変換後の大凡のデータサイズを得る
static	int GetSoundConvertDestSize_Fast_ACM( SOUNDCONV *SoundConv )
{
//	SOUNDCONV_ACM *acmdata = &SoundConv->AcmTypeData ;
	SOUNDCONV_ACM *acmdata = (SOUNDCONV_ACM *)SoundConv->ConvFunctionBuffer ;
	DWORD size ;
	
	if( acmdata->DestDataSampleNum != -1 )
	{
		size = (DWORD)( acmdata->DestDataSampleNum * SoundConv->OutFormat.nBlockAlign ) ;
	}
	else
	{
		GSoundConvertData.acmStreamSizeFunc( acmdata->AcmStreamHandle[0], SoundConv->DataSize, &size, ACM_STREAMSIZEF_SOURCE ) ;
	}
	return size ;
}

#endif


#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_DSHOW_MP3

// 変換後の大凡のデータサイズを得る
static int GetSoundConvertDestSize_Fast_DSMP3( SOUNDCONV *SoundConv )
{
	SOUNDCONV_DSMP3 *dsmp3 = ( SOUNDCONV_DSMP3 * )SoundConv->ConvFunctionBuffer ;
	return ( int )dsmp3->PCMValidDataSize ;
}

#endif
#endif
#endif





}
