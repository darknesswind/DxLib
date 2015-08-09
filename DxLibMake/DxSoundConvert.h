﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		サウンドデータ変換プログラムヘッダファイル
// 
// 				Ver 3.14f
// 
// -------------------------------------------------------------------------------

#ifndef __DXSOUNDCONVERT_H__
#define __DXSOUNDCONVERT_H__

#include "DxCompileConfig.h"

// インクルード ------------------------------------------------------------------

#include "DxLib.h"

#ifdef __WINDOWS__
	#include "Windows/DxSoundConvertWin.h"
#endif // __WINDOWS__

#ifdef __PSVITA
	#include "PSVita/DxSoundConvertPSVita.h"
#endif // __PSVITA

#ifdef __PS4
	#include "PS4/DxSoundConvertPS4.h"
#endif // __PS4

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

// 処理タイプ
#define SOUND_METHODTYPE_NORMAL				(0)		// 通常の処理(特別な処理は挟まない)
#define SOUND_METHODTYPE_OGG				(1)		// ＯＧＧを使用
#define SOUND_METHODTYPE_DEFAULT_NUM		(2)		// 環境に依存しない処理タイプの数

// チャンクＩＤセットマクロ
#define RIFFCHUNKID( C1 , C2 , C3 , C4 )	( DWORD )( ((( DWORD )C4)<<24) | ((( DWORD )C3)<<16) | ((( DWORD )C2)<<8) | (C1) ) 

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT		0x0003
#endif

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM				0x0001
#endif

// 構造体定義 --------------------------------------------------------------------

// RIFFチャンクデータ
struct ___RIFFCHUNK
{
	DWORD						fcc ;
	DWORD						 cb ;
} ;

// ＷＡＶＥデータ変換用構造体
struct SOUNDCONV_WAVE
{
	BYTE						*SrcBuffer ;		// 変換元のデータ
	DWORD						SrcBufferSize ;		// 変換元バッファのデータサイズ
	DWORD						SrcSampleNum ;		// 変換元バッファのサンプル数
	WAVEFORMATEX				SrcFormat ;			// 変換元バッファのフォーマット
} ;

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


// 環境依存初期化・終了関数
extern	int InitializeSoundConvert_PF( void ) ;														// サウンドデータ変換処理の環境依存の初期化を行う
extern	int TerminateSoundConvert_PF( void ) ;														// サウンドデータ変換処理の環境依存の終了処理を行う

extern	int SetupSoundConvert_PF( SOUNDCONV *SoundConv, STREAMDATA *Stream, int DisableReadSoundFunctionMask ) ;	// (環境依存処理)変換処理のセットアップ( [戻] -1:エラー )
extern	int SetSampleTimeSoundConvert_PF(    SOUNDCONV *SoundConv, int SampleTime ) ;					// (環境依存処理)変換処理の位置を変更する( サンプル単位 )
extern	int ConvertProcessSoundConvert_PF(   SOUNDCONV *SoundConv ) ;									// (環境依存処理)変換後のバッファにデータを補充する
extern	int TerminateSoundConvert_PF(        SOUNDCONV *SoundConv ) ;									// (環境依存処理)変換処理の後始末を行う
extern	int GetSoundConvertDestSize_Fast_PF( SOUNDCONV *SoundConv ) ;									// (環境依存処理)変換後の大凡のデータサイズを得る

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXSOUNDCONVERT_H__
