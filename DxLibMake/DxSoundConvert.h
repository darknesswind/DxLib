// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		サウンドデータ変換プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXSOUNDCONVERT_H__
#define __DXSOUNDCONVERT_H__

// Include ------------------------------------------------------------------

#include "DxCompileConfig.h"
#ifndef DX_NON_ACM
#ifndef DX_GCC_COMPILE
	#include <mmreg.h>
#endif	
	#include <msacm.h>
#endif
#include "DxLib.h"
#include "DxDirectX.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

// 结构体定义 --------------------------------------------------------------------

// RIFFチャンクデータ
struct ___RIFFCHUNK
{
	DWORD						fcc ;
	DWORD						 cb ;
} ;

// 処理タイプ
#define SOUND_METHODTYPE_NORMAL				(0)		// 通常の処理(特別な処理は挟まない)
#define SOUND_METHODTYPE_ACM				(1)		// ＡＣＭを使用
#define SOUND_METHODTYPE_OGG				(2)		// ＯＧＧを使用
#define SOUND_METHODTYPE_DSMP3				(3)		// ＤｉｒｅｃｔＳｈｏｗを使用したＭＰ３再生

// ＷＡＶＥデータ変換用構造体
struct SOUNDCONV_WAVE
{
	BYTE						*SrcBuffer ;		// 変換元のデータ
	DWORD						SrcBufferSize ;		// 変換元バッファのデータサイズ
	DWORD						SrcSampleNum ;		// 変換元バッファのサンプル数
	WAVEFORMATEX				SrcFormat ;			// 変換元バッファのフォーマット
} ;

#ifndef DX_NON_ACM

#ifdef DX_GCC_COMPILE
struct HACMSTREAM__ { int unused; }; typedef struct HACMSTREAM__ *HACMSTREAM ;
typedef HACMSTREAM *LPHACMSTREAM ;

struct HACMOBJ__ { int unused; }; typedef struct HACMOBJ__ *HACMOBJ ;
typedef HACMOBJ            *PHACMOBJ;
typedef HACMOBJ        FAR *LPHACMOBJ;

typedef struct wavefilter_tag {
    DWORD   cbStruct;           /* Size of the filter in bytes */
    DWORD   dwFilterTag;        /* filter type */
    DWORD   fdwFilter;          /* Flags for the filter (Universal Dfns) */
    DWORD   dwReserved[5];      /* Reserved for system use */
} WAVEFILTER;
typedef WAVEFILTER       *PWAVEFILTER;
typedef WAVEFILTER NEAR *NPWAVEFILTER;
typedef WAVEFILTER FAR  *LPWAVEFILTER;

#define MPEGLAYER3_WFX_EXTRA_BYTES   12
#define WAVE_FORMAT_MPEGLAYER3       0x0055 /* ISO/MPEG Layer3 Format Tag */

#define MPEGLAYER3_ID_UNKNOWN            0
#define MPEGLAYER3_ID_MPEG               1
#define MPEGLAYER3_ID_CONSTANTFRAMESIZE  2

typedef struct mpeglayer3waveformat_tag {
  WAVEFORMATEX  wfx;
  WORD          wID;
  DWORD         fdwFlags;
  WORD          nBlockSize;
  WORD          nFramesPerBlock;
  WORD          nCodecDelay;
} MPEGLAYER3WAVEFORMAT;

typedef MPEGLAYER3WAVEFORMAT          *PMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT NEAR    *NPMPEGLAYER3WAVEFORMAT;
typedef MPEGLAYER3WAVEFORMAT FAR     *LPMPEGLAYER3WAVEFORMAT;

#define ACM_STREAMSIZEF_SOURCE          0x00000000L
#define ACM_STREAMSIZEF_DESTINATION     0x00000001L
#define ACM_STREAMSIZEF_QUERYMASK       0x0000000FL

#define ACM_METRIC_MAX_SIZE_FORMAT          50

#define ACM_STREAMCONVERTF_BLOCKALIGN   0x00000004
#define ACM_STREAMCONVERTF_START        0x00000010
#define ACM_STREAMCONVERTF_END          0x00000020

#ifdef  _WIN64
#define _DRVRESERVED    15
#else
#define _DRVRESERVED    10
#endif  // _WIN64

typedef struct tACMSTREAMHEADER
{
    DWORD           cbStruct;               // sizeof(ACMSTREAMHEADER)
    DWORD           fdwStatus;              // ACMSTREAMHEADER_STATUSF_*
    DWORD_PTR       dwUser;                 // user instance data for hdr
    LPBYTE          pbSrc;
    DWORD           cbSrcLength;
    DWORD           cbSrcLengthUsed;
    DWORD_PTR       dwSrcUser;              // user instance data for src
    LPBYTE          pbDst;
    DWORD           cbDstLength;
    DWORD           cbDstLengthUsed;
    DWORD_PTR       dwDstUser;              // user instance data for dst
    DWORD           dwReservedDriver[_DRVRESERVED];   // driver reserved work space

} ACMSTREAMHEADER, *PACMSTREAMHEADER, FAR *LPACMSTREAMHEADER;
#endif

// ＡＣＭデータ変換用構造体
struct SOUNDCONV_ACM
{
	WAVEFORMATEX				*Format ;				// 変換元のフォーマット
	HACMSTREAM					AcmStreamHandle[2]	;	// 圧縮されたデータを扱う際に使うＡＣＭのハンドル

	void						*SrcData ;				// 変換元のデータを一時的に保存しておくメモリ領域
	int							SrcDataSize ;			// 変換元のデータを一時的に保存しておくメモリ領域のサイズ
//	int							SrcDataValidSize ;		// 変換元のデータの有効なサイズ
	int							SrcDataPosition ;		// 変換元のデータの変換が完了したサイズ
	int							DestDataSampleNum ;		// 変換後のデータのサンプル数( -1:以外の場合のみ有効 )

	int							BeginSeekCompSrcSize ;	// シーク予定位置までサブACMハンドルで変換しておく処理で、既に変換が完了している圧縮データのサイズ
	int							BeginSeekPosition ;		// シーク予定位置までサブACMハンドルで変換しておく処理で、既に変換が完了している展開後データのサイズ(シーク予定位置は SOUNDCONV 関数の SeekLockPosition メンバー)
	void						*BeginSeekBuffer ;		// シーク予定位置までサブACMハンドルで変換しておく処理で使用するバッファ
} ;

#endif

#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE

// ＤｉｒｅｃｔＳｈｏｗを使ったＭＰ３データ変換処理で使用するバッファクラス
class SOUNDCONV_DSMP3_BUF : public D_ISampleGrabberCB
{
public :
	struct SOUNDCONV			*SoundConvData ;		// 音声データ変換処理用構造体へのポインタ

	ULONG __stdcall				AddRef() ;
	ULONG __stdcall				Release() ;

	HRESULT __stdcall			QueryInterface( REFIID riid, void ** ppv ) ;
	HRESULT __stdcall			SampleCB( double SampleTime, D_IMediaSample *pSample ) ;
	HRESULT __stdcall			BufferCB( double SampleTime, BYTE *pBuffer, long BufferLen ) ;
} ;

// ＤｉｒｅｃｔＳｈｏｗを使ったＭＰ３データ変換用構造体
struct SOUNDCONV_DSMP3
{
	void						*PCMBuffer ;			// ＰＣＭに変換されたデータを格納するメモリ領域
	DWORD						PCMBufferSize ;			// ＰＣＭバッファのサイズ
	DWORD						PCMValidDataSize ;		// 有効なＰＣＭのサイズ
	DWORD						PCMDestCopySize ;		// 本バッファへのコピーが完了したサイズ
} ;

#endif  // DX_NON_DSHOW_MOVIE
#endif	// DX_NON_MOVIE

// 音声データ変換処理用構造体
struct SOUNDCONV
{
	int							InitializeFlag ;		// 初期化されている場合に立つフラグ
	int							EndFlag ;				// 変換が終了したら立つフラグ
	
	int 						MethodType ;			// 処理タイプ( SOUND_METHODTYPE_NORMAL 等 )
	STREAMDATA 					Stream ;				// データ読み込み用ストリームデータ

	int							HeaderPos ;				// ヘッダのある位置
	int							HeaderSize ;			// ヘッダのサイズ
	int							DataPos ;				// 実データのある位置
	int							DataSize ;				// 実データのサイズ

	WAVEFORMATEX				OutFormat ;				// 変換後のフォーマット

	char						ConvFunctionBuffer[ 1024 ] ;	// MethodType 別データ保存用バッファ領域

	void						*DestData ;				// 変換後のデータを一時的に保存するメモリ領域
	int							DestDataSize ;			// 変換後のデータを一時的に保存するメモリ領域のサイズ
	int							DestDataValidSize ;		// 変換後のデータの有効なサイズ
	int							DestDataCompSize ;		// 変換後のデータを一時的に保存するメモリ領域中の転送済みのサイズ
	int							DestDataCompSizeAll ;	// 変換開始から現在に至るまでに転送したデータのサイズ

	int							SeekLockPosition ;		// 予約シーク位置(ＡＣＭタイプでループする場合の為のデータ)
#ifndef DX_NON_OGGVORBIS
	int							OggVorbisBitDepth ;			// ＯｇｇＶｏｒｂｉｓ使用時のビット深度(1:8bit 2:16bit)
	int							OggVorbisFromTheoraFile ;	// Ogg Theora ファイル中の Vorbis データを参照するかどうかのフラグ( TRUE:Theora ファイル中の Vorbis データを参照する )
#endif
} ;

// 関数プロトタイプ宣言-----------------------------------------------------------

// 初期化・終了関数
extern	int InitializeSoundConvert( void ) ;														// サウンドデータ変換処理の初期化を行う
extern	int TerminateSoundConvert( void ) ;															// サウンドデータ変換処理の終了処理を行う

// フォーマット変換用
extern	int SetupSoundConvert( SOUNDCONV *SoundConv, STREAMDATA *Stream, int DisableReadSoundFunctionMask = 0, int OggVorbisBitDepth = 2, int OggVorbisFromTheoraFile = FALSE ) ;	// 変換処理のセットアップ( [戻] -1:エラー )
//extern	int SetTimeSoundConvert(      SOUNDCONV *SoundConv, int Time ) ;						// 変換処理の位置を変更する( ミリ秒単位 )
extern	int SetSampleTimeSoundConvert(    SOUNDCONV *SoundConv, int SampleTime ) ;					// 変換処理の位置を変更する( サンプル単位 )
extern	int SetupSeekPosSoundConvert(     SOUNDCONV *SoundConv, int SeekPos ) ;						// シーク予定の位置をセットしておく(ＡＣＭの為に)
extern	int RunSoundConvert(              SOUNDCONV *SoundConv, void *DestBuffer, int DestSize ) ;	// 指定のサイズ分だけ音声を解凍( [戻] -1:エラー  0以上:変換したサイズ )
extern	int TerminateSoundConvert(        SOUNDCONV *SoundConv ) ;									// 変換処理の後始末を行う
extern	int GetOutSoundFormatInfo(        SOUNDCONV *SoundConv, WAVEFORMATEX *OutWaveFormat ) ;		// 変換後の音声形式を取得( [戻] -1:エラー  0以上:変換後のサイズ )
extern	int GetSoundConvertEndState(      SOUNDCONV *SoundConv ) ;									// 変換処理が終了しているかどうかを取得する( [戻] TRUE:終了した  FALSE:まだ終了していない )
extern	int GetSoundConvertDestSize_Fast( SOUNDCONV *SoundConv ) ;									// 変換後の大凡のデータサイズを得る
extern	int SoundConvertFast(             SOUNDCONV *SoundConv, WAVEFORMATEX *FormatP, void **DestBufferP, int *DestSizeP ) ;	// 高速な一括変換

}

#endif // __DXSOUNDCONVERT_H__
