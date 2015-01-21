// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ＤｉｒｅｃｔＳｏｕｎｄ制御プログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

#include "DxSound.h"

#ifndef DX_NON_SOUND

// Include ------------------------------------------------------------------
#include "DxFile.h"
#include "DxBaseFunc.h"
#include "DxSystem.h"
#include "DxMemory.h"
#include "DxUseCLib.h"
#include "DxASyncLoad.h"
#include "DxLog.h"
#include "Windows/DxWindow.h"
#include "Windows/DxWinAPI.h"
#include "Windows/DxGuid.h"
#include "Windows/DxFileWin.h"

namespace DxLib
{

// マクロ定義 -------------------------------------

// 簡易アクセス用マクロ
#define DSOUND								DX_DirectSoundData
#define MIDI								DX_MidiData

// ソフトサウンド関係の定義
#define SSND_PLAYER_SEC_DIVNUM				(256)		// １秒の分割数
#define SSND_PLAYER_STRM_BUFSEC				(256)		// プレイヤーの DirectSoundBuffer のサイズ( 1 / SSND_PLAYER_SEC_DIVNUM　秒 )
#define SSND_PLAYER_STRM_ONECOPYSEC			(8)			// プレイヤーの DirectSoundBuffer のサイズ( 1 / SSND_PLAYER_SEC_DIVNUM　秒 )
#define SSND_PLAYER_STRM_SAKICOPYSEC		(256 / 5)	// プレイヤーの再生時に音声データを先行展開しておく最大時間( 単位は SSND_PLAYER_SEC_DIVNUM分の1秒 )
#define SSND_PLAYER_STRM_MINSAKICOPYSEC		(256 / 20)	// プレイヤーの再生時に音声データを先行展開しておく最小時間( 単位は SSND_PLAYER_SEC_DIVNUM分の1秒 )

// ストリーム関係の定義
#define STS_DIVNUM							(256)		// １秒の分割数
#define STS_BUFSEC							(256)		// ストリーム風サウンド再生時に確保するサウンドバッファの時間( 単位は STS_DIVNUM分の1秒 )
#define STS_ONECOPYSEC						(8)			// ストリーム風サウンド再生時に一度に転送するサウンドデータの長さ( 1/STS_ONECOPYSEC 秒 )
#define STS_SAKICOPYSEC						(192)		// ストリーム風サウンド再生時に音声データを先行展開しておく最大時間( 単位は STS_DIVNUM分の1秒 )
#define STS_MINSAKICOPYSEC					(96)		// ストリーム風サウンド再生時に音声データを先行展開しておく最小時間( 単位は STS_DIVNUM分の1秒 )
#define STS_CACHEBUFFERSEC					(8)			// ストリーム風サウンド再生時に、ファイルからのストリームのときにメモリ上に確保する一時バッファの長さ( 1/STS_CACHEBUFFERSEC 秒 )

#define DSMP3ONEFILLSEC						(8)			// DirectShow を使ったＭＰ３変換のフラグ一つ辺りがカバーする時間( 1/DSMP3ONEFILLSEC 秒 )

#define SOUNDSIZE( size, align )			( ( ( size ) + ( align - 1 ) ) / ( align ) * ( align ) )

// キャプチャで保存するサウンドのサンプリングレート
#define CAPTURE_SAMPLERATE					(44100)

// 標準ＷＡＶＥファイルのヘッダのサイズと標準ＷＡＶＥファイルの WAVEFORMATEX 構造体のサイズ
#define NORMALWAVE_HEADERSIZE				(46)
#define NORMALWAVE_FORMATSIZE				(18)

// ＭＩＤＩ最小ボリューム
#define DM_MIN_VOLUME						(-10000)


// エラーチェック用マクロ
#define SOUNDHCHK( HAND, SPOINT )			HANDLECHK(       DX_HANDLETYPE_SOUND, HAND, *( ( HANDLEINFO ** )&SPOINT ) )
#define SOUNDHCHK_ASYNC( HAND, SPOINT )		HANDLECHK_ASYNC( DX_HANDLETYPE_SOUND, HAND, *( ( HANDLEINFO ** )&SPOINT ) )

// エラーチェック用マクロ
#define SSND_MASKHCHK( HAND, SPOINT )		HANDLECHK(       DX_HANDLETYPE_SOFTSOUND, HAND, *( ( HANDLEINFO ** )&SPOINT ) )
#define SSND_MASKHCHK_ASYNC( HAND, SPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_SOFTSOUND, HAND, *( ( HANDLEINFO ** )&SPOINT ) )

// エラーチェック用マクロ
#define MIDI_MASKHCHK( HAND, SPOINT )		HANDLECHK(       DX_HANDLETYPE_MUSIC, HAND, *( ( HANDLEINFO ** )&SPOINT ) )
#define MIDI_MASKHCHK_ASYNC( HAND, SPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_MUSIC, HAND, *( ( HANDLEINFO ** )&SPOINT ) )

// デシベル値から XAudio2 の率値に変換する関数
__inline float D_XAudio2DecibelsToAmplitudeRatio( float Decibels )
{
    return _POW( 10.0f, Decibels / 20.0f ) ;
}

// XAudio2 の率値からデシベル値に変換する関数
__inline float D_XAudio2AmplitudeRatioToDecibels( float Volume )
{
    if( Volume == 0 )
    {
        return -3.402823466e+38f ;
    }
    return 20.0f * ( float )_LOG10( Volume ) ;
}

// 结构体定义 --------------------------------------------------------------------

static const D_X3DAUDIO_DISTANCE_CURVE_POINT D_X3DAudioDefault_LinearCurvePoints[2] = { 0.0f, 1.0f, 1.0f, 0.0f };
static const D_X3DAUDIO_DISTANCE_CURVE       D_X3DAudioDefault_LinearCurve          = { ( D_X3DAUDIO_DISTANCE_CURVE_POINT * )& D_X3DAudioDefault_LinearCurvePoints[ 0 ], 2 } ;

static const D_X3DAUDIO_CONE Listener_DirectionalCone = { DX_PI_F * 5.0f / 6.0f, DX_PI_F * 11.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f } ;

static const D_X3DAUDIO_DISTANCE_CURVE_POINT D_Emitter_LFE_CurvePoints[3]			= { 0.0f, 1.0f, 0.25f, 0.0f, 1.0f, 0.0f } ;
static const D_X3DAUDIO_DISTANCE_CURVE       D_Emitter_LFE_Curve					= { ( D_X3DAUDIO_DISTANCE_CURVE_POINT * )&D_Emitter_LFE_CurvePoints[ 0 ], 3 } ;

static const D_X3DAUDIO_DISTANCE_CURVE_POINT D_Emitter_Reverb_CurvePoints[3]		= { 0.0f, 0.5f, 0.75f, 1.0f, 1.0f, 0.0f } ;
static const D_X3DAUDIO_DISTANCE_CURVE       D_Emitter_Reverb_Curve					= { ( D_X3DAUDIO_DISTANCE_CURVE_POINT * )&D_Emitter_Reverb_CurvePoints[ 0 ], 3 } ;

static const D_XAUDIO2FX_REVERB_I3DL2_PARAMETERS D_PRESET_PARAMS[ D_XAUDIO2FX_PRESET_NUM ] =
{
	D_XAUDIO2FX_I3DL2_PRESET_FOREST,
	D_XAUDIO2FX_I3DL2_PRESET_DEFAULT,
	D_XAUDIO2FX_I3DL2_PRESET_GENERIC,
	D_XAUDIO2FX_I3DL2_PRESET_PADDEDCELL,
	D_XAUDIO2FX_I3DL2_PRESET_ROOM,
	D_XAUDIO2FX_I3DL2_PRESET_BATHROOM,
	D_XAUDIO2FX_I3DL2_PRESET_LIVINGROOM,
	D_XAUDIO2FX_I3DL2_PRESET_STONEROOM,
	D_XAUDIO2FX_I3DL2_PRESET_AUDITORIUM,
	D_XAUDIO2FX_I3DL2_PRESET_CONCERTHALL,
	D_XAUDIO2FX_I3DL2_PRESET_CAVE,
	D_XAUDIO2FX_I3DL2_PRESET_ARENA,
	D_XAUDIO2FX_I3DL2_PRESET_HANGAR,
	D_XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY,
	D_XAUDIO2FX_I3DL2_PRESET_HALLWAY,
	D_XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR,
	D_XAUDIO2FX_I3DL2_PRESET_ALLEY,
	D_XAUDIO2FX_I3DL2_PRESET_CITY,
	D_XAUDIO2FX_I3DL2_PRESET_MOUNTAINS,
	D_XAUDIO2FX_I3DL2_PRESET_QUARRY,
	D_XAUDIO2FX_I3DL2_PRESET_PLAIN,
	D_XAUDIO2FX_I3DL2_PRESET_PARKINGLOT,
	D_XAUDIO2FX_I3DL2_PRESET_SEWERPIPE,
	D_XAUDIO2FX_I3DL2_PRESET_UNDERWATER,
	D_XAUDIO2FX_I3DL2_PRESET_SMALLROOM,
	D_XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM,
	D_XAUDIO2FX_I3DL2_PRESET_LARGEROOM,
	D_XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL,
	D_XAUDIO2FX_I3DL2_PRESET_LARGEHALL,
	D_XAUDIO2FX_I3DL2_PRESET_PLATE,
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// ＤｉｒｅｃｔＳｏｕｎｄデータ
DIRECTSOUNDDATA DX_DirectSoundData ;

// ＭＩＤＩ管理用データ
MIDIDATA DX_MidiData ;	

// 関数プロトタイプ宣言-----------------------------------------------------------

static	int CreateDirectSoundBuffer( WAVEFORMATEX *WaveFormat, DWORD BufferSize ,
									 int SoundType, int BufferNum,
									 int SoundHandle, int SrcSoundHandle = -1, int ASyncThread = FALSE ) ;	// 新しい DirectSoundBuffer オブジェクトの作成
static  int _GetDirectSoundBuffer( int SoundHandle, SOUND * Sound, SOUNDBUFFER **BufferP = NULL, bool BufferGet = true ) ;	// 使用可能なサウンドバッファを取得する(-1:サウンドバッファの使用権を得られなかった)
static	DWORD WINAPI StreamSoundThreadFunction( void * ) ;											// ストリームサウンド処理用スレッド



// 補助系関数
static	int AdjustSoundDataBlock( int Length, SOUND * Sound ) ;										// サイズ補正

// ソフトウエアで制御するサウンドデータハンドル関係
//static int AddSoftSoundData( int IsPlayer, int Channels, int BitsPerSample, int SamplesPerSec, int SampleNum ) ;	// ソフトウエアで制御するサウンドデータハンドルの追加
//static int SubSoftSoundData( int SoftSoundHandle, int CheckIsPlayer ) ;											// ソフトウエアで制御するサウンドデータハンドルの削除
static	int _SoftSoundPlayerProcess( SOFTSOUND *SPlayer ) ;														// ソフトウエアで制御するサウンドデータプレイヤーの定期処理

// 補助系
extern	int CreateWaveFileImage( 	void **DestBufferP, int *DestBufferSizeP,
									const WAVEFORMATEX *Format, int FormatSize,
									const void *WaveData, int WaveDataSize ) ;						// 音声フォーマットと波形イメージからＷＡＶＥファイルイメージを作成する
static	int FileFullRead( const TCHAR *FileName, void **BufferP, int *SizeP ) ;						// ファイルを丸まるメモリに読み込む
extern	int StreamFullRead( STREAMDATA *Stream, void **BufferP, int *SizeP ) ;						// ストリームデータを丸まるメモリに読み込む
static	int SoundTypeChangeToStream( int SoundHandle ) ;											// サウンドハンドルのタイプがストリームハンドルではない場合にストリームハンドルに変更する
static	int MilliSecPositionToSamplePosition( int SamplesPerSec, int MilliSecTime ) ;				// ミリ秒単位の数値をサンプル単位の数値に変換する
static	int SamplePositionToMilliSecPosition( int SamplesPerSec, int SampleTime ) ;					// サンプル単位の数値をミリ秒単位の数値に変換する
static	int _PlaySetupSoundMem( SOUND * Sound, int TopPositionFlag ) ;								// サウンドハンドルの再生準備を行う( -1:エラー 0:正常終了 1:再生する必要なし )
static	int	_PlaySetupStreamSoundMem( int SoundHandle, SOUND * Sound, int PlayType, int TopPositionFlag, SOUNDBUFFER **DestSBuffer ) ;	// ストリームサウンドハンドルの再生準備を行う( -1:エラー 0:正常終了 1:再生する必要なし )
static	int _CreateSoundHandle( int Is3DSound ) ;													// サウンドハンドルを作成する

// 波形データ用
static	WAVEDATA	*AllocWaveData( int Size, int UseDoubleSizeBuffer = FALSE ) ;
static	int			ReleaseWaveData( WAVEDATA *Data ) ;
static	WAVEDATA	*DuplicateWaveData( WAVEDATA *Data ) ;

//サウンドバッファ用
static HRESULT SoundBuffer_Initialize(          SOUNDBUFFER *Buffer, D_DSBUFFERDESC *Desc, SOUNDBUFFER *Src, int Is3DSound ) ;
static HRESULT SoundBuffer_Duplicate(           SOUNDBUFFER *Buffer, SOUNDBUFFER *Src, int Is3DSound ) ;
static HRESULT SoundBuffer_Terminate(           SOUNDBUFFER *Buffer ) ;
static HRESULT SoundBuffer_Play(                SOUNDBUFFER *Buffer, int Loop ) ;
static HRESULT SoundBuffer_Stop(                SOUNDBUFFER *Buffer, int EffectStop = FALSE ) ;
static HRESULT SoundBuffer_GetStatus(           SOUNDBUFFER *Buffer, DWORD *Status ) ;
static HRESULT SoundBuffer_Lock(                SOUNDBUFFER *Buffer, DWORD WritePos, DWORD WriteSize, void **LockPos1, DWORD *LockSize1, void **LockPos2, DWORD *LockSize2 ) ;
static HRESULT SoundBuffer_Unlock(              SOUNDBUFFER *Buffer, void *LockPos1, DWORD LockSize1, void *LockPos2, DWORD LockSize2 ) ;
static HRESULT SoundBuffer_SetFrequency(        SOUNDBUFFER *Buffer, DWORD Frequency ) ;
static HRESULT SoundBuffer_GetFrequency(        SOUNDBUFFER *Buffer, LPDWORD Frequency ) ;
static HRESULT SoundBuffer_SetPan(              SOUNDBUFFER *Buffer, LONG Pan ) ;
static HRESULT SoundBuffer_GetPan(              SOUNDBUFFER *Buffer, LPLONG Pan ) ;
static HRESULT SoundBuffer_RefreshVolume(       SOUNDBUFFER *Buffer ) ;
static HRESULT SoundBuffer_SetVolumeAll(        SOUNDBUFFER *Buffer, LONG Volume ) ;
static HRESULT SoundBuffer_SetVolume(           SOUNDBUFFER *Buffer, int Channel, LONG Volume ) ;
static HRESULT SoundBuffer_GetVolume(           SOUNDBUFFER *Buffer, int Channel, LPLONG Volume ) ;
static HRESULT SoundBuffer_GetCurrentPosition(  SOUNDBUFFER *Buffer, LPDWORD PlayPos, LPDWORD WritePos ) ;
static HRESULT SoundBuffer_SetCurrentPosition(  SOUNDBUFFER *Buffer, DWORD NewPos ) ;
static HRESULT SoundBuffer_CycleProcess(        SOUNDBUFFER *Buffer ) ;
static HRESULT SoundBuffer_FrameProcess(        SOUNDBUFFER *Buffer, int Sample, short *DestBuf ) ;
static HRESULT SoundBuffer_Set3DPosition(       SOUNDBUFFER *Buffer, VECTOR *Position ) ;
static HRESULT SoundBuffer_Set3DRadius(         SOUNDBUFFER *Buffer, float Radius ) ;
static HRESULT SoundBuffer_Set3DInnerRadius(    SOUNDBUFFER *Buffer, float Radius ) ;
static HRESULT SoundBuffer_Set3DVelocity(       SOUNDBUFFER *Buffer, VECTOR *Velocity ) ;
static HRESULT SoundBuffer_Set3DFrontPosition(  SOUNDBUFFER *Buffer, VECTOR *FrontPosition, VECTOR *UpVector ) ;
static HRESULT SoundBuffer_Set3DConeAngle(      SOUNDBUFFER *Buffer, float InnerAngle, float OuterAngle ) ;
static HRESULT SoundBuffer_Set3DConeVolume(     SOUNDBUFFER *Buffer, float InnerAngleVolume, float OuterAngleVolume ) ;
static HRESULT SoundBuffer_Refresh3DSoundParam( SOUNDBUFFER *Buffer, int AlwaysFlag = FALSE ) ;
static HRESULT SoundBuffer_SetReverbParam(      SOUNDBUFFER *Buffer, SOUND3D_REVERB_PARAM *Param ) ;
static HRESULT SoundBuffer_SetPresetReverbParam( SOUNDBUFFER *Buffer, int PresetNo ) ;

// XAudio2関係
static HRESULT D_XAudio2CreateReverb( IUnknown** ppApo, DWORD Flags = 0 ) ;
static HRESULT D_XAudio2CreateVolumeMeter( IUnknown** ppApo, DWORD Flags = 0 ) ;
static void D_ReverbConvertI3DL2ToNative( const D_XAUDIO2FX_REVERB_I3DL2_PARAMETERS* pI3DL2, D_XAUDIO2FX_REVERB_PARAMETERS* pNative ) ;
static void D_ReverbConvertI3DL2ToNative2_8( const D_XAUDIO2FX_REVERB_I3DL2_PARAMETERS* pI3DL2, D_XAUDIO2FX_REVERB_PARAMETERS2_8* pNative ) ;

// プログラム --------------------------------------------------------------------

// ＤｉｒｅｃｔＳｏｕｎｄ初期化、終了関数

BOOL CALLBACK DSEnum( LPGUID /*lpGuid*/, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID /*lpContext*/ )
{
	DXST_ERRORLOGFMT_ADD(( _T( "モジュール名:%15s  ドライバ記述:%s " ), lpcstrModule , lpcstrDescription )) ;

	return TRUE ;
}


// ＤｉｒｅｃｔＳｏｕｎｄを初期化する
extern int InitializeDirectSound( void )
{
	HRESULT hr ;
	WAVEFORMATEX wfmtx ;
	D_DSCAPS caps ;

	if( DSOUND.InitializeFlag ) return -1 ;
	if( DSOUND.DirectSoundObject != NULL ) return -1 ;

INITSTART:

	// 出力レートをセット
	if( DSOUND.OutputSmaplesPerSec == 0 ) DSOUND.OutputSmaplesPerSec = 44100 ;

	// ３Ｄサウンドの１メートルを設定
	if( DSOUND._3DSoundOneMetreEnable == FALSE )
	{
		DSOUND._3DSoundOneMetre = 1.0f ;
	}

	// 再生モードによって処理を分岐
	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		if( DSOUND.DisableXAudioFlag == FALSE )
		{
			int i ;
			const TCHAR *X3DAudioDLLName[] =
			{
				_T( "X3DAudio1_7.dll" ),
				//_T( "X3DAudio1_6.dll" ),
				//_T( "X3DAudio1_5.dll" ),
				//_T( "X3DAudio1_4.dll" ),
				//_T( "X3DAudio1_3.dll" ),
				//_T( "X3DAudio1_2.dll" ),
				//_T( "X3DAudio1_1.dll" ),
				//_T( "X3DAudio1_0.dll" ),
				NULL,
			} ;
			GUID *XAudioGuidList[][ 2 ] = 
			{
				{
					&CLSID_XAUDIO2_7,
					&CLSID_XAUDIO2_7_DEBUG,
				},
				//{
				//	&CLSID_XAUDIO2_6,
				//	&CLSID_XAUDIO2_6_DEBUG,
				//},
				//{
				//	&CLSID_XAUDIO2_5,
				//	&CLSID_XAUDIO2_5_DEBUG,
				//},
				//{
				//	&CLSID_XAUDIO2_4,
				//	&CLSID_XAUDIO2_4_DEBUG,
				//},
				//{
				//	&CLSID_XAUDIO2_3,
				//	&CLSID_XAUDIO2_3_DEBUG,
				//},
				//{
				//	&CLSID_XAUDIO2_2,
				//	&CLSID_XAUDIO2_2_DEBUG,
				//},
				//{
				//	&CLSID_XAUDIO2_1,
				//	&CLSID_XAUDIO2_1_DEBUG,
				//},
				//{
				//	&CLSID_XAUDIO2_0,
				//	&CLSID_XAUDIO2_0_DEBUG,
				//},
				{
					NULL,
					NULL
				},
			} ;
			DXST_ERRORLOG_ADD( _T( "XAudio2 の初期化を行います\n" ) ) ;

			DXST_ERRORLOG_TABADD ;

			// 最初にXAudio2_8.dllが無いか調べる
			DSOUND.XAudio2_8DLL = LoadLibrary( _T( "XAudio2_8.dll" ) ) ;
			if( DSOUND.XAudio2_8DLL != NULL )
			{
				DXST_ERRORLOG_ADD( _T( "XAudio2_8.dll を使用します\n" ) ) ;

				// あった場合の処理
				DSOUND.XAudio2CreateFunc          = ( HRESULT ( WINAPI * )( D_IXAudio2_8 **, DWORD, D_XAUDIO2_PROCESSOR ) )GetProcAddress( DSOUND.XAudio2_8DLL, "XAudio2Create" ) ;
				DSOUND.CreateAudioVolumeMeterFunc = ( HRESULT ( WINAPI * )( IUnknown** ppApo ) )GetProcAddress( DSOUND.XAudio2_8DLL, "CreateAudioVolumeMeter" ) ;
				DSOUND.CreateAudioReverbFunc      = ( HRESULT ( WINAPI * )( IUnknown** ppApo ) )GetProcAddress( DSOUND.XAudio2_8DLL, "CreateAudioReverb" ) ;
				DSOUND.X3DAudioInitializeFunc     = ( void ( __cdecl * )( DWORD, float, D_X3DAUDIO_HANDLE ) )GetProcAddress( DSOUND.XAudio2_8DLL, "X3DAudioInitialize" ) ;
				DSOUND.X3DAudioCalculateFunc      = ( void ( __cdecl * )( const D_X3DAUDIO_HANDLE, const D_X3DAUDIO_LISTENER *, const D_X3DAUDIO_EMITTER *, DWORD, D_X3DAUDIO_DSP_SETTINGS * ) )GetProcAddress( DSOUND.XAudio2_8DLL, "X3DAudioCalculate" ) ;

				DXST_ERRORLOG_ADD( _T( "XAudio2 インターフェースの取得と初期化を行います....  " ) ) ;

				hr = DSOUND.XAudio2CreateFunc( &DSOUND.XAudio2_8Object, 0, D_XAUDIO2_DEFAULT_PROCESSOR ) ;
				if( FAILED( hr ) )
				{
					DXST_ERRORLOG_ADD( _T( "初期化に失敗しました\n" ) ) ;
					DXST_ERRORLOGFMT_ADDA(( "エラーコード %x", hr )) ;
					DSOUND.XAudio2_8Object->Release() ;
					DSOUND.XAudio2_8Object = NULL ;

					DSOUND.DisableXAudioFlag = TRUE ;
					goto INITSTART ;
				}
				DXST_ERRORLOG_ADD( _T( "成功 Ver2.8\n" ) ) ;

				hr = DSOUND.XAudio2_8Object->CreateMasteringVoice( &DSOUND.XAudio2_8MasteringVoiceObject, D_XAUDIO2_DEFAULT_CHANNELS, DSOUND.OutputSmaplesPerSec ) ;
				if( FAILED( hr ) )
				{
					DXST_ERRORLOG_ADD( _T( "XAudio2MasteringVoiceの作成に失敗しました\n" ) ) ;
					DSOUND.XAudio2_8Object->Release() ;
					DSOUND.XAudio2_8Object = NULL ;

					DSOUND.DisableXAudioFlag = TRUE ;
					goto INITSTART ;
				}

				// リバーブ計算用パラメータ準備
				for( i = 0 ; i < D_XAUDIO2FX_PRESET_NUM ; i ++ )
				{
					D_ReverbConvertI3DL2ToNative2_8( &D_PRESET_PARAMS[ i ], &DSOUND.XAudio2_8ReverbParameters[ i ] ) ;
				}
			}
			else
			{
				DXST_ERRORLOG_ADD( _T( "XAudio2 インターフェースの取得を行います....  " ) ) ;
				for( i = 0 ; XAudioGuidList[ i ][ 0 ] != NULL ; i ++ )
				{
					hr = WinAPIData.Win32Func.CoCreateInstanceFunc( *XAudioGuidList[ i ][ 0 ], NULL, CLSCTX_ALL, IID_IXAUDIO2, ( LPVOID *)&DSOUND.XAudio2Object );
					if( !FAILED( hr ) )
					{
						break ;
					}
				}
				if( XAudioGuidList[ i ][ 0 ] != NULL )
				{
					DXST_ERRORLOGFMT_ADD(( _T( "成功 Ver2.%d\n" ), 7 - i )) ;
					DXST_ERRORLOG_ADD( _T( "引き続きインターフェースの初期化処理...  " ) ) ;
					hr = DSOUND.XAudio2Object->Initialize( 0 ) ;
					if( FAILED( hr ) )
					{
						DXST_ERRORLOG_ADD( _T( "初期化に失敗しました\n" ) ) ;
						DXST_ERRORLOGFMT_ADDA(( "エラーコード %x", hr )) ;
						DSOUND.XAudio2Object->Release() ;
						DSOUND.XAudio2Object = NULL ;

						DSOUND.DisableXAudioFlag = TRUE ;
						goto INITSTART ;
					}
					DXST_ERRORLOG_ADD( _T( "成功\n" ) ) ;

					DXST_ERRORLOG_ADD( _T( "X3DAudio.DLL の読み込み中... " ) ) ;
					for( i = 0 ; X3DAudioDLLName[ i ] != NULL ; i ++ )
					{
						DSOUND.X3DAudioDLL = LoadLibrary( X3DAudioDLLName[ i ] ) ;
						if( DSOUND.X3DAudioDLL != NULL )
							break ;
					}
					if( DSOUND.X3DAudioDLL == NULL )
					{
						DXST_ERRORLOG_ADD( _T( "失敗、X3DAudio、XAudio2は使用しません\n" ) ) ;
						DSOUND.XAudio2Object->Release() ;
						DSOUND.XAudio2Object = NULL ;

						DSOUND.DisableXAudioFlag = TRUE ;
						goto INITSTART ;
					}
					else
					{
						DXST_ERRORLOG_ADD( _T( "成功\n" ) ) ;

						DSOUND.X3DAudioInitializeFunc = ( void ( __cdecl * )( DWORD, float, D_X3DAUDIO_HANDLE ) )GetProcAddress( DSOUND.X3DAudioDLL, "X3DAudioInitialize" ) ;
						DSOUND.X3DAudioCalculateFunc  = ( void ( __cdecl * )( const D_X3DAUDIO_HANDLE, const D_X3DAUDIO_LISTENER *, const D_X3DAUDIO_EMITTER *, DWORD, D_X3DAUDIO_DSP_SETTINGS * ) )GetProcAddress( DSOUND.X3DAudioDLL, "X3DAudioCalculate" ) ;
					}
				}
				else
				{
					DSOUND.XAudio2Object = NULL ;
					DXST_ERRORLOG_ADD( _T( "XAudio2インターフェースの取得に失敗しました\n" ) ) ;

					DSOUND.DisableXAudioFlag = TRUE ;
					goto INITSTART ;
				}

				hr = DSOUND.XAudio2Object->CreateMasteringVoice( &DSOUND.XAudio2MasteringVoiceObject, D_XAUDIO2_DEFAULT_CHANNELS, DSOUND.OutputSmaplesPerSec ) ;
				if( FAILED( hr ) )
				{
					DXST_ERRORLOG_ADD( _T( "XAudio2MasteringVoiceの作成に失敗しました\n" ) ) ;

					if( DSOUND.X3DAudioDLL != NULL )
					{
						FreeLibrary( DSOUND.X3DAudioDLL ) ;
						DSOUND.X3DAudioInitializeFunc = NULL ;
						DSOUND.X3DAudioCalculateFunc = NULL ;
						DSOUND.X3DAudioDLL = NULL ;
					}

					DSOUND.XAudio2Object->Release() ;
					DSOUND.XAudio2Object = NULL ;

					DSOUND.DisableXAudioFlag = TRUE ;
					goto INITSTART ;
				}

				// リバーブ計算用パラメータ準備
				for( i = 0 ; i < D_XAUDIO2FX_PRESET_NUM ; i ++ )
				{
					D_ReverbConvertI3DL2ToNative( &D_PRESET_PARAMS[ i ], &DSOUND.XAudio2ReverbParameters[ i ] ) ;
				}
			}
		}
		else
		{
			DXST_ERRORLOG_ADD( _T( "DirectSound の初期化を行います\n" ) ) ;
			DXST_ERRORLOG_ADD( _T( "DirectSound インターフェースの取得を行います....  " ) ) ; 
			hr = WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTSOUND, NULL, CLSCTX_ALL, IID_IDIRECTSOUND, ( LPVOID *)&DSOUND.DirectSoundObject );
			if( !FAILED( hr ) )
			{
				DXST_ERRORLOG_ADD( _T( "成功\n" ) ) ;
				DXST_ERRORLOG_ADD( _T( "引き続きインターフェースの初期化処理...  " ) ) ;
				hr = DSOUND.DirectSoundObject->Initialize( NULL ) ;
				if( FAILED( hr ) )
				{
					DXST_ERRORLOG_ADD( _T( "初期化に失敗しました\n" ) ) ;
					DXST_ERRORLOGFMT_ADDA(( "エラーコード %x", hr )) ;
					DSOUND.DirectSoundObject->Release() ;
					DSOUND.DirectSoundObject = NULL ;
					return -1 ;
				}
				DXST_ERRORLOG_ADD( _T( "成功\n" ) ) ;
			}
			else
			{
				DSOUND.DirectSoundObject = NULL ;
				DXST_ERRORLOG_ADD( _T( "DirectSoundインターフェースの取得に失敗しました\n" ) ) ;
				return -1 ;
			}
	/*
			DXST_ERRORLOG_ADD( _T( "DirectSound の初期化を行います\n" ) ) ; 
			hr = DirectSoundCreate( NULL , &DSOUND.DirectSoundObject , NULL ) ;
			if( hr != D_DS_OK )
			{
				DSOUND.DirectSoundObject = NULL ;
				DXST_ERRORLOG_ADD( _T( "DirectSoundの初期化に失敗しました\n" ) ) ;
				return -1 ;
			}
	*/

			// 強調レベルをセットする
			hr = DSOUND.DirectSoundObject->SetCooperativeLevel( GetDesktopWindow(), D_DSSCL_PRIORITY ) ;
			if( hr != D_DS_OK )
			{
				DXST_ERRORLOG_ADD( _T( "DirectSoundの協調レベルの設定に失敗しました\n" ) ) ;
				DSOUND.DirectSoundObject->Release() ;
				DSOUND.DirectSoundObject = NULL ;
				return -1 ;
			}

			// プライマリサウンドバッファを作成する
			{
				D_DSBUFFERDESC dsbdesc ;
				int ChNum , Rate , Byte ;

				// 作成ステータスをセットする
				_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
				dsbdesc.dwSize = sizeof( dsbdesc ) ;
				dsbdesc.dwFlags = D_DSBCAPS_PRIMARYBUFFER | ( DSOUND.UseSoftwareMixing ? D_DSBCAPS_LOCSOFTWARE : 0 ) ;
				hr = DSOUND.DirectSoundObject->CreateSoundBuffer( &dsbdesc , &DSOUND.PrimarySoundBuffer , NULL ) ;
				if( hr != D_DS_OK )
				{
					DXST_ERRORLOG_ADD( _T( "DirectSoundのプライマリサウンドバッファの作成に失敗しました\n" ) ) ;
					DSOUND.DirectSoundObject->Release() ;
					DSOUND.DirectSoundObject = NULL ;
					return -1 ;
				}

				// 情報を得る
				_MEMSET( &caps, 0, sizeof( caps ) ) ;
				caps.dwSize = sizeof( caps ) ;
				DSOUND.DirectSoundObject->GetCaps( &caps ) ;


				// プライマリバッファのフォーマットをセットする
				ChNum = caps.dwFlags & D_DSCAPS_PRIMARYSTEREO ? 2 : 1 ;
				Rate = caps.dwMaxSecondarySampleRate < 44100 ? caps.dwMaxSecondarySampleRate : 44100  ;
				Byte = caps.dwFlags & D_DSCAPS_PRIMARY16BIT  ? 2 : 1 ;

				_MEMSET( &wfmtx, 0, sizeof( wfmtx ) ) ;
				wfmtx.wFormatTag		= WAVE_FORMAT_PCM ;										// PCMフォーマット
				wfmtx.nChannels			= ( WORD )ChNum ;										// チャンネル２つ＝ステレオ
				wfmtx.nSamplesPerSec	= Rate ;												// 再生レート
				wfmtx.wBitsPerSample	= ( WORD )( Byte * 8 ) ;								// １音にかかるデータビット数
				wfmtx.nBlockAlign		= wfmtx.wBitsPerSample / 8 * wfmtx.nChannels  ;			// １ヘルツにかかるデータバイト数
				wfmtx.nAvgBytesPerSec	= wfmtx.nSamplesPerSec * wfmtx.nBlockAlign ;			// １秒にかかるデータバイト数
				hr = DSOUND.PrimarySoundBuffer->SetFormat( &wfmtx ) ;
				if( hr != D_DS_OK )
				{
					DXST_ERRORLOG_ADD( _T( "DirectSoundのプライマリサウンドバッファのフォーマット設定に失敗しました\n" ) ) ;
				}

			}
		}
		break ;

	case DX_MIDIMODE_DM :
		DSOUND.DisableXAudioFlag = TRUE ;

		// ＤｉｒｅｃｔＭｕｓｉｃによる再生の場合の処理
		{
			D_IDirectMusic *DMusic ;
			D_IDirectMusic8 *DMusic8 ;
			GUID MidiGuid = {0};
			bool MidiGuidValid ;
				
			DXST_ERRORLOG_ADD( _T( "DirectSound8 の初期化を行います\n" ) ) ; 

			// DirectMusic オブジェクトの作成
//			if( FAILED(	WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DirectMusicLoader , NULL , 
//											CLSCTX_INPROC , IID_IDirectMusicLoader8 , ( void ** )&DSOUND.DirectMusicLoaderObject ) ) )
			if( FAILED(	WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTMUSICLOADER, NULL , 
											CLSCTX_INPROC , IID_IDIRECTMUSICLOADER8 , ( void ** )&DSOUND.DirectMusicLoaderObject ) ) )
			{
				DXST_ERRORLOG_ADD( _T( "DirectMusicLoader8 オブジェクトの作成に失敗しました\n" ) ) ;
				DXST_ERRORLOG_ADD( _T( "前バージョンでの初期化処理に移ります\n" ) ) ;
				DSOUND.SoundMode = DX_MIDIMODE_MCI ;
				return InitializeDirectSound() ;
			}

//			if( FAILED( WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DirectMusicPerformance , NULL , 
//											CLSCTX_INPROC , IID_IDirectMusicPerformance8 , ( void ** )&DSOUND.DirectMusicPerformanceObject ) ) )
			if( FAILED( WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTMUSICPERFORMANCE, NULL , 
											CLSCTX_INPROC , IID_IDIRECTMUSICPERFORMANCE8 , ( void ** )&DSOUND.DirectMusicPerformanceObject ) ) )
			{
				DXST_ERRORLOG_ADD( _T( "DirectMusicPerformance オブジェクトの作成に失敗しました\n" ) ) ;
				TerminateDirectSound() ;
				return -1 ;
			}

			MidiGuidValid = false ;
			// MIDI音源の列挙
			{
				D_DMUS_PORTCAPS Param ;
				WCHAR wStr[D_DMUS_MAX_DESCRIPTION+3] ;
				int i ;

				if( FAILED( WinAPIData.Win32Func.CoCreateInstanceFunc( CLSID_DIRECTMUSIC, NULL,
												CLSCTX_INPROC_SERVER, IID_IDIRECTMUSIC, (LPVOID*)&DMusic ) ) )
				{
					DXST_ERRORLOG_ADD( _T( "DirectMusic オブジェクトの取得に失敗しました\n" ) ) ;
					TerminateDirectSound() ;
					return -1 ;
				}
			
//				if( FAILED( DMusic->QueryInterface( IID_IDirectMusic8, (void **)&DMusic8 ) ) )
				if( FAILED( DMusic->QueryInterface( IID_IDIRECTMUSIC8, (void **)&DMusic8 ) ) )
				{
					DMusic->Release() ;
					DXST_ERRORLOG_ADD( _T( "DirectMusic8 オブジェクトの取得に失敗しました\n" ) ) ;
					TerminateDirectSound() ;
					return -1 ;
				}
				
				_MEMSET( &Param, 0, sizeof( Param ) ) ;
				Param.dwSize = sizeof(D_DMUS_PORTCAPS) ;
				
				for( i = 0 ; DMusic8->EnumPort( i, &Param ) != S_FALSE ; i ++ )
				{
					lstrcpyW( wStr, Param.wszDescription ) ;
					wStr[D_DMUS_MAX_DESCRIPTION] = L'\0' ;
#ifdef UNICODE
					DXST_ERRORLOGFMT_ADD(( _T( "%s  Device%d:%s " ), Param.dwClass == D_DMUS_PC_OUTPUTCLASS ? _T( "出力ポート" ) : _T( "入力ポート" ), i, wStr )) ;
#else
					char Str[256] ;
					WCharToMBChar( CP_ACP, ( DXWCHAR * )wStr, Str, 256 ) ;
					DXST_ERRORLOGFMT_ADD(( _T( "%s  Device%d:%s " ), Param.dwClass == D_DMUS_PC_OUTPUTCLASS ? _T( "出力ポート" ) : _T( "入力ポート" ), i, Str )) ;
#endif
/*					if( strstr( Str, _T( "Microsoft MIDI マッパー" ) ) != NULL && Param.dwClass == D_DMUS_PC_OUTPUTCLASS )
					{
						MidiGuid = Param.guidPort ;
						MidiGuidValid = true ;
					}
*/				}
				
				DMusic8->Release() ;
				DMusic->Release() ;
			}
	
			// DirectMusicPerformanceオブジェクトの初期化
			{
				D_DMUS_AUDIOPARAMS AudioParams ;

				_MEMSET( &AudioParams, 0, sizeof( AudioParams ) ) ;
				AudioParams.dwSize = sizeof( AudioParams ) ;
				AudioParams.fInitNow = TRUE ;
				AudioParams.dwValidData = D_DMUS_AUDIOPARAMS_SAMPLERATE | D_DMUS_AUDIOPARAMS_FEATURES ;
				AudioParams.dwSampleRate = 44100 ;
				AudioParams.dwFeatures = D_DMUS_AUDIOF_ALL ;
				if( MidiGuidValid == true )
				{
					AudioParams.dwValidData |= D_DMUS_AUDIOPARAMS_DEFAULTSYNTH  ;
					AudioParams.clsidDefaultSynth = MidiGuid ;
				}

				DSOUND.DirectSoundObject = NULL ;
				DMusic = NULL ;
				if( DSOUND.DirectMusicPerformanceObject->InitAudio(
											&DMusic ,									// IDirectMusicインターフェイスは不要。
											&DSOUND.DirectSoundObject,					// IDirectSoundインターフェイスポインタを渡す。
											NULL ,										// ウィンドウのハンドル。
											D_DMUS_APATH_SHARED_STEREOPLUSREVERB ,		// デフォルトのオーディオパスタイプ
											64 ,										// パフォーマンスチャンネルの数。
											D_DMUS_AUDIOF_ALL ,							// シンセサイザの機能。
											&AudioParams								// オーディオパラメータにはデフォルトを使用。
										) != S_OK )
				{
					DXST_ERRORLOG_ADD( _T( "DirectMusicPerformanceオブジェクトの初期化に失敗しました\n" ) ) ;
					TerminateDirectSound() ;
					return -1 ;
				}
			}

			// 最大音量の取得
			DSOUND.DirectMusicPerformanceObject->GetGlobalParam( GUID_PERFMASTERVOLUME , ( void * )&DSOUND.MaxVolume , sizeof( long ) ) ;
		}
		break ;
	}

#ifndef DX_NON_BEEP
	// ビープ音のセット
	NS_SetBeepFrequency( 8000 ) ;
#endif

	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		// 情報出力
		if( DSOUND.XAudio2_8DLL == NULL )
		{
			D_XAUDIO2_DEVICE_DETAILS Details ;

			DSOUND.XAudio2Object->GetDeviceDetails( 0, &Details ) ;

			DXST_ERRORLOG_TABADD ;

#ifndef DX_GCC_COMPILE
			DXST_ERRORLOGFMT_ADDW(( L"デバイス名：%s", Details.DisplayName )) ;
#endif
			DXST_ERRORLOGFMT_ADD(( _T( "チャンネル数:%d" ), Details.OutputFormat.Format.nChannels )) ;
			DXST_ERRORLOGFMT_ADD(( _T( "ビット精度:%dbit" ), Details.OutputFormat.Format.wBitsPerSample )) ;
			DXST_ERRORLOGFMT_ADD(( _T( "サンプリングレート:%dHz" ), Details.OutputFormat.Format.nSamplesPerSec )) ;

			DXST_ERRORLOG_TABSUB ;

			DSOUND.XAudio2OutputChannelMask = Details.OutputFormat.dwChannelMask ;
			DSOUND.OutputChannels = Details.OutputFormat.Format.nChannels ;
		}
		else
		{
			int i ;

			DSOUND.XAudio2_8MasteringVoiceObject->GetChannelMask( &DSOUND.XAudio2OutputChannelMask ) ;
			DSOUND.OutputChannels = 0 ;
			for( i = 0 ; i < 32 ; i ++ )
			{
				if( ( DSOUND.XAudio2OutputChannelMask & ( 1 << i ) ) != 0 )
					DSOUND.OutputChannels ++ ;
			}
		}

		// X3DAudio の初期化
		DSOUND.X3DAudioInitializeFunc( DSOUND.XAudio2OutputChannelMask, D_X3DAUDIO_SPEED_OF_SOUND, DSOUND.X3DAudioInstance ) ;
	}
	else
	{
		// 無音バッファの初期化
		{
			D_DSBUFFERDESC dsbdesc ;
			WAVEFORMATEX wfmtx ;
			HRESULT hr ;

			// バッファ生成ステータスのセット
			_MEMSET( &wfmtx, 0, sizeof( wfmtx ) ) ;
			wfmtx.wFormatTag		= WAVE_FORMAT_PCM ;										// PCMフォーマット
			wfmtx.nChannels			= 2 ;													// チャンネル２つ＝ステレオ
			wfmtx.nSamplesPerSec	= 44100 ;												// 再生レート
			wfmtx.wBitsPerSample	= 2 * 8 ;												// １音にかかるデータビット数
			wfmtx.nBlockAlign		= wfmtx.wBitsPerSample / 8 * wfmtx.nChannels  ;			// １ヘルツにかかるデータバイト数
			wfmtx.nAvgBytesPerSec	= wfmtx.nSamplesPerSec * wfmtx.nBlockAlign ;			// １秒にかかるデータバイト数

			_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
			dsbdesc.dwSize			= sizeof( dsbdesc ) ;
			dsbdesc.dwFlags			= D_DSBCAPS_GLOBALFOCUS | ( DSOUND.UseSoftwareMixing ? D_DSBCAPS_LOCSOFTWARE : D_DSBCAPS_STATIC ) ;
			dsbdesc.dwBufferBytes	= wfmtx.nBlockAlign * 100 ;
			dsbdesc.lpwfxFormat		= &wfmtx ;

			hr = DSOUND.DirectSoundObject->CreateSoundBuffer( &dsbdesc , &DSOUND.NoSoundBuffer , NULL ) ;
			if( hr != D_DS_OK )
			{
				DXST_ERRORLOG_ADD( _T( "無音バッファの作成に失敗しました" ) ) ;
				DSOUND.NoSoundBuffer = NULL ;
				goto R1 ;
			}

			// 無音で埋める
			{
				LPVOID write1 ;
				DWORD length1 ;
				LPVOID write2 ;
				DWORD length2 ;
				DWORD ltemp, i , j ;
				signed short *p ;

				hr = DSOUND.NoSoundBuffer->Lock( 0 , dsbdesc.dwBufferBytes, &write1 , &length1 , &write2 , &length2 , 0 ) ;		// バッファのロック
				if( hr != D_DS_OK )
				{
					DXST_ERRORLOG_ADD( _T( "無音バッファのロックに失敗しました" ) ) ;
					DSOUND.NoSoundBuffer->Release() ;
					DSOUND.NoSoundBuffer = NULL ;
				}

				j = 0 ;
				p = ( signed short * )write1 ;
				ltemp = length1 >> 1 ;
				for( i = 0 ; i < ltemp ; i += 1, j ++, p ++ ) *p = 0 ;

				if( write2 != 0 )
				{
					p = ( signed short * )write2 ;
					ltemp = length2 >> 1 ;
					for( i = 0 ; i < ltemp ; i ++, j ++, p ++ ) *p = 0 ;
				}

				hr = DSOUND.NoSoundBuffer->Unlock( write1, length1, write2, length2 ) ;								// バッファのロック解除
			}

			// 無音バッファの再生
			DSOUND.NoSoundBuffer->Play( 0, 0, D_DSBPLAY_LOOPING ) ;
		}

R1 :
		// 性能表示
		if( DSOUND.DirectSoundObject )
		{
	/*
	#ifdef UNICODE
			typedef HRESULT ( WINAPI *DIRECTSOUNDENUMERATEFUNC )( LPD_DSENUMCALLBACKW, LPVOID lpContext ) ;
	#else
	*/
			typedef HRESULT ( WINAPI *DIRECTSOUNDENUMERATEFUNC )( LPD_DSENUMCALLBACKA, LPVOID lpContext ) ;
	//#endif
			DIRECTSOUNDENUMERATEFUNC DirectSoundEnumerateFunc = NULL ;
			HINSTANCE DSoundDLL = NULL ;
	 
			DXST_ERRORLOG_TABADD ;
			_MEMSET( &caps, 0, sizeof( caps ) ) ;
			caps.dwSize = sizeof( caps ) ;
			DSOUND.DirectSoundObject->GetCaps( &caps ) ;

			DXST_ERRORLOG_ADD( _T( "DirectSound デバイスを列挙します\n" ) ) ;
			DXST_ERRORLOG_TABADD ;
			
			// DirectSound DLL をロードする
			if( ( DSoundDLL = LoadLibraryA( "DSOUND.DLL" ) ) == NULL )
			{
				DXST_ERRORLOG_ADD( _T( "DSOUND.DLL のロードに失敗しました\n" ) ) ;
				DXST_ERRORLOG_TABSUB ;
				goto ENUMEND ;
			}
		
			// 列挙用関数の取得
	#ifdef UNICODE
			if( ( DirectSoundEnumerateFunc = ( DIRECTSOUNDENUMERATEFUNC )GetProcAddress( DSoundDLL, "DirectSoundEnumerateW" ) ) == NULL )
	#else
			if( ( DirectSoundEnumerateFunc = ( DIRECTSOUNDENUMERATEFUNC )GetProcAddress( DSoundDLL, "DirectSoundEnumerateA" ) ) == NULL )
	#endif
			{
				FreeLibrary( DSoundDLL );
				DXST_ERRORLOG_ADD( _T( "DirectSound デバイスの列挙用関数のポインタ取得に失敗しました\n" ) ) ;
				DXST_ERRORLOG_TABSUB ;
				goto ENUMEND ;
			}

			// 列挙
	//		DirectSoundEnumerate( DSEnum , NULL ) ;
			DirectSoundEnumerateFunc( DSEnum , NULL ) ;

			DXST_ERRORLOG_TABSUB ;

			DXST_ERRORLOGFMT_ADD(( _T( "最大サンプリングレート:%.2fKHz 最小サンプリングレート:%.2fKHz " ),
						( double )caps.dwMaxSecondarySampleRate / 1000 ,
						( double )caps.dwMinSecondarySampleRate / 1000 )) ;

			DXST_ERRORLOGFMT_ADD(( _T( "総サウンドメモリ領域:%.2fKB 空きサウンドメモリ領域:%.2fKB \n" ),
						( double )caps.dwTotalHwMemBytes / 0x100 ,
						( double )caps.dwFreeHwMemBytes / 0x100 )) ;

			DXST_ERRORLOG_ADD( _T( "利用可能サンプリング精度\n" ) ) ;
			DXST_ERRORLOGFMT_ADD(( _T( " プライマリ １６bit = %s  ８bit = %s " ),
				( caps.dwFlags & D_DSCAPS_PRIMARY16BIT ? _T( "OK" ) : _T( "NO" ) ) ,
				( caps.dwFlags & D_DSCAPS_PRIMARY8BIT ? _T( "OK" ) : _T( "NO" ) ) )) ;

			DXST_ERRORLOGFMT_ADD(( _T( " セカンダリ １６bit = %s  ８bit = %s \n" ),
				( caps.dwFlags & D_DSCAPS_SECONDARY16BIT  ? _T( "OK" ) : _T( "NO" ) ) ,
				( caps.dwFlags & D_DSCAPS_SECONDARY8BIT  ? _T( "OK" ) : _T( "NO" ) ) )) ;

			DXST_ERRORLOG_ADD( _T( "利用可能チャンネル\n" ) ) ;
			DXST_ERRORLOGFMT_ADD(( _T( " プライマリ MONO = %s  STEREO = %s " ) ,
				( caps.dwFlags & D_DSCAPS_PRIMARYMONO   ? _T( "OK" ) : _T( "NO" ) ) ,
				( caps.dwFlags & D_DSCAPS_PRIMARYSTEREO ? _T( "OK" ) : _T( "NO" ) ) )) ;

			DXST_ERRORLOGFMT_ADD(( _T( " セカンダリ MONO = %s  STEREO = %s \n" ) ,
				( caps.dwFlags & D_DSCAPS_SECONDARYMONO ? _T( "OK" ) : _T( "NO" ) ) ,
				( caps.dwFlags & D_DSCAPS_SECONDARYSTEREO ? _T( "OK" ) : _T( "NO" ) ) )) ;

			DXST_ERRORLOG_TABSUB ;

			// DirectSound DLL を解放する
			FreeLibrary( DSoundDLL );
		}
	}
ENUMEND :
	// 作成する音のデータタイプをセット
	DSOUND.CreateSoundDataType = DX_SOUNDDATATYPE_MEMNOPRESS ;

#ifndef DX_NON_OGGVORBIS
	// ＯｇｇＶｏｒｂｉｓのＰＣＭデコード時の、ビット深度を１６bitにセット
	DSOUND.OggVorbisBitDepth = 2 ;
#endif

	// サウンドハンドル管理情報初期化
	InitializeHandleManage( DX_HANDLETYPE_SOUND, sizeof( SOUND ), MAX_SOUND_NUM, InitializeSoundHandle, TerminateSoundHandle, DXSTRING( _T( "サウンド" ) ) ) ;

	// ソフトウエアで扱う波形データハンドル管理情報初期化
	InitializeHandleManage( DX_HANDLETYPE_SOFTSOUND, sizeof( SOFTSOUND ), MAX_SOFTSOUND_NUM, InitializeSoftSoundHandle, TerminateSoftSoundHandle, DXSTRING( _T( "ソフトサウンド" ) ) ) ;

	// ＭＩＤＩハンドル管理情報初期化
	InitializeHandleManage( DX_HANDLETYPE_MUSIC, sizeof( MIDIHANDLEDATA ), MAX_MUSIC_NUM, InitializeMidiHandle, TerminateMidiHandle, DXSTRING( _T( "ミュージック" ) ) ) ;

	// ハンドルリストを初期化
	InitializeHandleList( &DSOUND._3DSoundListFirst, &DSOUND._3DSoundListLast ) ;
	InitializeHandleList( &DSOUND.StreamSoundListFirst, &DSOUND.StreamSoundListLast ) ;
	InitializeHandleList( &DSOUND.SoftSoundPlayerListFirst, &DSOUND.SoftSoundPlayerListLast ) ;
	InitializeHandleList( &DSOUND.PlayFinishDeleteSoundListFirst, &DSOUND.PlayFinishDeleteSoundListLast ) ;
	InitializeHandleList( &DSOUND.Play3DSoundListFirst, &DSOUND.Play3DSoundListLast ) ;

	// ３Ｄサウンド処理用のリスナー情報を初期化
	DSOUND.X3DAudioListenerConeData = Listener_DirectionalCone ;
	DSOUND.X3DAudioListenerData.Position.x = 0.0f ;
	DSOUND.X3DAudioListenerData.Position.y = 0.0f ;
	DSOUND.X3DAudioListenerData.Position.z = 0.0f ;
	DSOUND.X3DAudioListenerData.Velocity.x = 0.0f ;
	DSOUND.X3DAudioListenerData.Velocity.y = 0.0f ;
	DSOUND.X3DAudioListenerData.Velocity.z = 0.0f ;
	DSOUND.X3DAudioListenerData.OrientFront.x = 0.0f ;
	DSOUND.X3DAudioListenerData.OrientFront.y = 0.0f ;
	DSOUND.X3DAudioListenerData.OrientFront.z = 1.0f ;
	DSOUND.X3DAudioListenerData.OrientTop.x = 0.0f ;
	DSOUND.X3DAudioListenerData.OrientTop.y = 1.0f ;
	DSOUND.X3DAudioListenerData.OrientTop.z = 0.0f ;
	DSOUND.X3DAudioListenerData.pCone = &DSOUND.X3DAudioListenerConeData ;
	DSOUND.ListenerInfo.Position = VGet( 0.0f, 0.0f, 0.0f ) ;
	DSOUND.ListenerInfo.FrontDirection = VGet( 0.0f, 0.0f, 1.0f ) ;
	DSOUND.ListenerInfo.Velocity = VGet( 0.0f, 0.0f, 0.0f ) ;
	DSOUND.ListenerInfo.InnerAngle = DSOUND.X3DAudioListenerConeData.InnerAngle ;
	DSOUND.ListenerInfo.OuterAngle = DSOUND.X3DAudioListenerConeData.OuterAngle ;
	DSOUND.ListenerInfo.InnerVolume = DSOUND.X3DAudioListenerConeData.InnerVolume ;
	DSOUND.ListenerInfo.OuterVolume = DSOUND.X3DAudioListenerConeData.OuterVolume ;
	DSOUND.ListenerSideDirection = VGet( 1.0f, 0.0f, 0.0f ) ;

	// 初期化フラグを立てる
	DSOUND.InitializeFlag = TRUE ;

	NS_InitSoundMem() ;
	NS_InitSoftSound() ;
	NS_InitSoftSoundPlayer() ;

#ifndef DX_NON_MULTITHREAD

	// ストリームサウンド再生用の処理を行うスレッドの作成
	DSOUND.StreamSoundThreadHandle = NULL ;
	if( WinData.ProcessorNum > 1 )
	{
		DSOUND.StreamSoundThreadHandle = CreateThread( NULL, 0, StreamSoundThreadFunction, NULL, CREATE_SUSPENDED, &DSOUND.StreamSoundThreadID ) ;
		SetThreadPriority( DSOUND.StreamSoundThreadHandle, THREAD_PRIORITY_TIME_CRITICAL ) ;
		ResumeThread( DSOUND.StreamSoundThreadHandle ) ;
	}

#endif // DX_NON_MULTITHREAD
	
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		DXST_ERRORLOG_TABSUB ;
		DXST_ERRORLOG_ADD( _T( "XAudio2 の初期化は正常に終了しました\n" ) ) ;
	}
	else
	{
		DXST_ERRORLOG_ADD( _T( "DirectSound の初期化は正常に終了しました\n" ) ) ;
	}

	// 終了
	return 0 ;
}

// ＤｉｒｅｃｔＳｏｕｎｄの使用を終了する
extern int TerminateDirectSound( void )
{
	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

#ifndef DX_NON_MULTITHREAD

	// ストリームサウンド再生用スレッドを終了する
	if( DSOUND.StreamSoundThreadHandle != NULL )
	{
		DSOUND.StreamSoundThreadEndFlag = 1 ;
		while( NS_ProcessMessage() == 0 && DSOUND.StreamSoundThreadEndFlag == 1 ) Sleep( 2 ) ;
	}

#endif // DX_NON_MULTITHREAD

	// サウンドキャプチャ処理を終了する
	EndSoundCapture() ;

	// すべてのサウンドデータを解放する
	NS_InitSoundMem() ;

	// 全てのソフトサウンドデータを解放する
	NS_InitSoftSound() ;
	NS_InitSoftSoundPlayer() ;

	// すべてのＭＩＤＩデータを解放する
	NS_InitMusicMem() ;

	// テンポラリファイルの削除
	DeleteFile( MIDI.FileName ) ;

	// サウンドハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_SOUND ) ;

	// ソフトサウンドハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_SOFTSOUND ) ;

	// ＭＩＤＩハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_MUSIC ) ;

	// 再生モードによって処理を分岐
	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		// 通常の再生モードの時の処理

		// PlayWav関数で再生されているサウンドはないので-1をセットしておく
		DSOUND.PlayWavSoundHandle = -1 ;

		break ;

	case DX_MIDIMODE_DM :
		// DirectMusic による演奏を行っていた場合の処理

		// すべてのサウンドの再生を止める
		if( DSOUND.DirectMusicPerformanceObject )
		{
			DSOUND.DirectMusicPerformanceObject->Stop( NULL , NULL , 0 , 0 ) ;
			DSOUND.DirectMusicPerformanceObject->CloseDown() ;
			DSOUND.DirectMusicPerformanceObject->Release() ;

			DSOUND.DirectMusicPerformanceObject = NULL ;
		}

		// ローダーを解放する
		if( DSOUND.DirectMusicLoaderObject )
		{
			DSOUND.DirectMusicLoaderObject->Release() ; 
			DSOUND.DirectMusicLoaderObject = NULL ;
		}

		break ;
	}

	// XAudio2 を使用しているかどうかで処理を分岐
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		// マスタリングボイスの解放
		if( DSOUND.XAudio2MasteringVoiceObject != NULL )
		{
			DSOUND.XAudio2MasteringVoiceObject->DestroyVoice() ;
			DSOUND.XAudio2MasteringVoiceObject = NULL ;
		}
		if( DSOUND.XAudio2_8MasteringVoiceObject != NULL )
		{
			DSOUND.XAudio2_8MasteringVoiceObject->DestroyVoice() ;
			DSOUND.XAudio2_8MasteringVoiceObject = NULL ;
		}

		// XAudio2の解放
		if( DSOUND.XAudio2Object != NULL )
		{
			DSOUND.XAudio2Object->Release() ;
			DSOUND.XAudio2Object = NULL ;
		}

		// XAudio2.dllの後始末処理
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			FreeLibrary( DSOUND.XAudio2_8DLL ) ;
			DSOUND.XAudio2_8DLL = NULL ;
			DSOUND.XAudio2CreateFunc = NULL ;
			DSOUND.CreateAudioVolumeMeterFunc = NULL ;
			DSOUND.CreateAudioReverbFunc = NULL ;
			DSOUND.X3DAudioInitializeFunc = NULL ;
			DSOUND.X3DAudioCalculateFunc = NULL ;
		}

		// X3DAudioの後始末処理
		if( DSOUND.X3DAudioDLL != NULL )
		{
			FreeLibrary( DSOUND.X3DAudioDLL ) ;
			DSOUND.X3DAudioDLL = NULL ;
			DSOUND.X3DAudioInitializeFunc = NULL ;
			DSOUND.X3DAudioCalculateFunc = NULL ;
		}
	}
	else
	{
		// プライマリサウンドバッファの再生停止、破棄
		if( DSOUND.PrimarySoundBuffer )
		{
			DSOUND.PrimarySoundBuffer->Stop() ;
			DSOUND.PrimarySoundBuffer->Release() ;
			DSOUND.PrimarySoundBuffer = NULL ;
		}

		// 無音バッファの再生停止、破棄
		if( DSOUND.NoSoundBuffer )
		{
			DSOUND.NoSoundBuffer->Stop() ;
			DSOUND.NoSoundBuffer->Release() ;
			DSOUND.NoSoundBuffer = NULL ;
		}
		
		// ビープ音バッファの解放
		if( DSOUND.BeepSoundBuffer )
		{
			DSOUND.BeepSoundBuffer->Release() ;
			DSOUND.BeepSoundBuffer = NULL ;
		}

		// ＤｉｒｅｃｔＳｏｕｎｄオブジェクトの解放
		if( DSOUND.DirectSoundObject > (D_IDirectSound *)1 )
		{
			DSOUND.DirectSoundObject->Release() ;
		}
		DSOUND.DirectSoundObject = NULL ;


		DXST_ERRORLOG_ADD( _T( "DirectSound の終了処理は正常に終了しました\n" ) ) ;
	}

	// 初期化フラグを倒す
	DSOUND.InitializeFlag = FALSE ;

	// 終了
	return 0 ;
}









// 音频数据管理类函数

// メモリに読みこんだWAVEデータを削除し、初期化する
extern int NS_InitSoundMem( int /*LogOutFlag*/ )
{
	int Ret ;

	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	// ハンドルを初期化
	Ret = AllHandleSub( DX_HANDLETYPE_SOUND ) ;

	// PlayWav用サウンドハンドルも初期化
	DSOUND.PlayWavSoundHandle = -1 ;

	// 終了
	return 0 ;
}

// 新しいサウンドデータ領域を確保する
extern int NS_AddSoundData( int Handle )
{
	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	return AddHandle( DX_HANDLETYPE_SOUND, Handle ) ;
}

// サウンドハンドルの初期化
extern int InitializeSoundHandle( HANDLEINFO *HandleInfo )
{
	SOUND *Sound = ( SOUND * )HandleInfo ;
	int i ;

	// 初期化
	Sound->PresetReverbParam = 0 ;
	NS_Get3DPresetReverbParamSoundMem( &Sound->ReverbParam, DX_REVERB_PRESET_DEFAULT ) ;

	for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
	{
		Sound->BaseVolume[ i ] = 0 ;
	}
	Sound->BasePan = 0 ;
	Sound->BaseFrequency = 0 ;
	Sound->Base3DPosition = VGet( 0.0f, 0.0f, 0.0f ) ;
	Sound->Base3DRadius = 14.0f ;
	Sound->Base3DVelocity = VGet( 0.0f, 0.0f, 0.0f ) ;

	// サウンドの再生が終了したらハンドルを自動的に削除するかどうかのフラグを倒す
	Sound->PlayFinishDeleteFlag = FALSE ;

	// データの数を初期化
	Sound->Stream.FileNum = 0 ;

	// 終了
	return 0 ;
}

// サウンドハンドルの後始末
extern int TerminateSoundHandle( HANDLEINFO *HandleInfo )
{
	SOUND *Sound = ( SOUND * )HandleInfo ;
	int i ;

	// 音の再生を止める
	if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		NS_StopStreamSoundMem( HandleInfo->Handle ) ;
	}
	else
	{
		if( Sound->Buffer[0].Valid ) NS_StopSoundMem( HandleInfo->Handle ) ;
	}

	if( Sound->Is3DSound != FALSE )
	{
		// ３Ｄサウンドだった場合は３Ｄサウンドリストから外す
		SubHandleList( &Sound->_3DSoundList ) ;

		// 再生中リストに追加されていたら外す
		if( Sound->AddPlay3DSoundList )
		{
			Sound->AddPlay3DSoundList = FALSE ;
			SubHandleList( &Sound->Play3DSoundList ) ;
		}
	}

	// 再生が終了したら削除する設定になっていたら、再生が終了したら削除するサウンドのリストから外す
	if( Sound->PlayFinishDeleteFlag )
	{
		SubHandleList( &Sound->PlayFinishDeleteSoundList ) ;
	}

	// サウンドタイプによって処理を分岐
	switch( Sound->Type )
	{
	case DX_SOUNDTYPE_NORMAL :
		for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
		{
			if( Sound->Buffer[ i ].Valid ) SoundBuffer_Terminate( &Sound->Buffer[ i ] ) ;
		}

		// サウンドデータを解放する
		if( Sound->Normal.WaveDataUseCount )
		{
			*Sound->Normal.WaveDataUseCount = *Sound->Normal.WaveDataUseCount - 1 ;
			if( *Sound->Normal.WaveDataUseCount == 0 )
			{
				if( Sound->Normal.WaveData != NULL )
				{
					DXFREE( Sound->Normal.WaveData ) ;
				}
				DXFREE( Sound->Normal.WaveDataUseCount ) ;
			}
		}
		Sound->Normal.WaveDataUseCount = NULL ;
		Sound->Normal.WaveData = NULL ;
		break ;

	case DX_SOUNDTYPE_STREAMSTYLE :
		{
			int i ;
			STREAMFILEDATA *PlayData ;
			SOUND *UniSound = NULL ;
			
			// サウンドバッファを共有している場合は処理を分岐
			if( Sound->Stream.BufferBorrowSoundHandle != -1 &&
				SOUNDHCHK( Sound->Stream.BufferBorrowSoundHandle, UniSound ) == false )
			{
				// サウンドバッファを使用しているの数を減らす
				UniSound->Stream.BufferBorrowSoundHandleCount-- ;
				
				// カウントが０になっていて、且つ自分以外のサウンドハンドルで、
				// 削除待ちをしていた場合は削除する
				if( Sound->Stream.BufferBorrowSoundHandle != HandleInfo->Handle &&
					UniSound->Stream.DeleteWaitFlag == TRUE )
				{
					NS_DeleteSoundMem( Sound->Stream.BufferBorrowSoundHandle ) ;
				}
			}
			
			// サウンドバッファが他のハンドルで使われていたら削除待ち状態にする
			if( Sound->Stream.BufferBorrowSoundHandleCount != 0 )
			{
				Sound->Stream.DeleteWaitFlag = TRUE ;

				// 削除キャンセルの戻り値
				return 1 ;
			}

			// ストリームサウンドリストから外す
			SubHandleList( &Sound->Stream.StreamSoundList ) ;

			// 登録されているすべてのサウンドデータの領域を解放
			PlayData = Sound->Stream.File ;
			for( i = 0 ; i < Sound->Stream.FileNum ; i ++, PlayData ++ )
			{
				TerminateSoundConvert( &PlayData->ConvData ) ;
			
				if( PlayData->FileImage != NULL )
					DXFREE( PlayData->FileImage ) ;

				if( Sound->Stream.FileLibraryLoadFlag[ i ] == TRUE &&
					PlayData->DataType == DX_SOUNDDATATYPE_FILE &&
					PlayData->ConvData.Stream.DataPoint )
					PlayData->ConvData.Stream.ReadShred.Close( PlayData->ConvData.Stream.DataPoint ) ;

				if( PlayData->MemStream.DataPoint != NULL )
					PlayData->MemStream.ReadShred.Close( PlayData->MemStream.DataPoint ) ;
			}

			if( Sound->Buffer[0].Valid ) SoundBuffer_Terminate( &Sound->Buffer[0] ) ;
		}
		break ;
	}

	// 終了
	return 0 ;
}

// 新しい DirectSoundBuffer オブジェクトの作成
static int CreateDirectSoundBuffer( WAVEFORMATEX *WaveFormat , DWORD BufferSize, int SoundType , int BufferNum , int SoundHandle, int SrcSoundHandle, int ASyncThread )
{
	int i ;
	SOUND *Sound ;
	SOUND *SrcSound = NULL ;
	D_DSBUFFERDESC dsbdesc ;
	HRESULT hr ;
	
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, Sound ) )
			return -1 ;
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, Sound ) )
			return -1 ;
	}

	// 元ハンドルがある場合の処理
	if( SrcSoundHandle != -1 )
	{
		if( SOUNDHCHK( SrcSoundHandle, SrcSound ) )
			return -1 ;

		// タイプがストリームだったらエラー
		if( SoundType != DX_SOUNDTYPE_NORMAL ) return -1 ;

		// パラメータは元ハンドルからもってくる
		WaveFormat = &SrcSound->BufferFormat ;
		BufferSize = SrcSound->Normal.WaveSize ;
	}

	// タイプによって処理を分岐
	switch( SoundType )
	{
	case DX_SOUNDTYPE_NORMAL :	// ノーマルサウンド
		{
			// セカンダリサウンドバッファを作成

			// 数分作成
			for( i = 0 ; i < BufferNum ; i ++ )
			{
				// ソースハンドルが無い場合は１番目以降は同じバッファを複製する
				if( i == 0 && SrcSound == NULL )
				{
NORMAL_CREATEBUFFER :
					// バッファ生成ステータスのセット
					_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
					dsbdesc.dwSize			= sizeof( dsbdesc ) ;
					dsbdesc.dwFlags			= D_DSBCAPS_GLOBALFOCUS | D_DSBCAPS_CTRLPAN | D_DSBCAPS_CTRLVOLUME | D_DSBCAPS_CTRLFREQUENCY | ( DSOUND.UseSoftwareMixing ? D_DSBCAPS_LOCSOFTWARE : D_DSBCAPS_STATIC ) ;
					dsbdesc.dwBufferBytes	= BufferSize ;
					dsbdesc.lpwfxFormat		= WaveFormat ;
//					hr = DSOUND.DirectSoundObject->CreateSoundBuffer( &dsbdesc , &Sound->Buffer[ i ] , NULL ) ;
					hr = SoundBuffer_Initialize( &Sound->Buffer[ i ], &dsbdesc, NULL, Sound->Is3DSound ) ;
					
					// 作成に失敗した場合は、含める機能を減らして再度作成する
					if( hr != D_DS_OK )
					{
						// バッファ生成ステータスのセット
						_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
						dsbdesc.dwSize			= sizeof( dsbdesc ) ;
						dsbdesc.dwFlags			= D_DSBCAPS_CTRLPAN | D_DSBCAPS_CTRLVOLUME | ( DSOUND.UseSoftwareMixing ? D_DSBCAPS_LOCSOFTWARE : D_DSBCAPS_STATIC ) ;
						dsbdesc.dwBufferBytes	= BufferSize ;
						dsbdesc.lpwfxFormat		= WaveFormat ;

						if( SoundBuffer_Initialize( &Sound->Buffer[ i ], &dsbdesc, NULL, Sound->Is3DSound ) != D_DS_OK )
						{
							i -- ;
							while( i != -1 )
							{
								if( Sound->Buffer[ i ].Valid ) SoundBuffer_Terminate( &Sound->Buffer[ i ] ) ;

								i -- ;
							}
							DXST_ERRORLOG_ADD( _T( "DirectSoundBuffer の作成に失敗しました\n" ) ) ;
							return -1 ;
						}
					}
					
					// 複製かどうかフラグを倒す
					Sound->Normal.BufferDuplicateFlag[ i ] = FALSE ;
				}
				else
				{
					if( SrcSound )
					{
						hr = SoundBuffer_Duplicate( &Sound->Buffer[ i ], &SrcSound->Buffer[ 0 ], Sound->Is3DSound ) ;
//						hr = DSOUND.DirectSoundObject->DuplicateSoundBuffer( Sound->Buffer[ 0 ], &Sound->Buffer[ i ] ) ;
					}
					else
					{
						hr = SoundBuffer_Duplicate( &Sound->Buffer[ i ], &Sound->Buffer[ 0 ], Sound->Is3DSound ) ;
//						hr = DSOUND.DirectSoundObject->DuplicateSoundBuffer( Sound->Buffer[ 0 ], &Sound->Buffer[ i ] ) ;
					}

					// 複製かどうかフラグをとりあえず倒しておく
					Sound->Normal.BufferDuplicateFlag[ i ] = FALSE ;
//					Sound->Normal.BufferDuplicateFlag[ i ] = hr == D_DS_OK ? TRUE : FALSE ;

					// 複製に失敗した場合は複数のサウンドバッファを使用しない
					if( hr != D_DS_OK )
					{
						// ソースサウンドがあって、且つ０で失敗したら実ハンドルを一つ作る
						if( SrcSound != NULL && i == 0 )
						{
							goto NORMAL_CREATEBUFFER ;
						}
						else
						{
							BufferNum = i ;
							break ;
						}
					}
					
					// 複製に失敗した場合は普通のバッファを作成する
/*					if( hr != D_DS_OK )
					{
						// 複製かどうかフラグを立てる
						Sound->Normal.BufferDuplicateFlag[ i ] = TRUE ;
						
						// バッファ生成ステータスのセット
						_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
						dsbdesc.dwSize			= sizeof( dsbdesc ) ;
						dsbdesc.dwFlags			= D_DSBCAPS_GLOBALFOCUS | D_DSBCAPS_STATIC | D_DSBCAPS_CTRLPAN | D_DSBCAPS_CTRLVOLUME | D_DSBCAPS_CTRLFREQUENCY ;
						dsbdesc.dwBufferBytes	= BufferSize ;
						dsbdesc.lpwfxFormat		= WaveFormat ;
						hr = DSOUND.DirectSoundObject->CreateSoundBuffer( &dsbdesc , &Sound->Buffer[ i ] , NULL ) ;
						
						// 作成に失敗した場合は、含める機能を減らして再度作成する
						if( hr != D_DS_OK )
						{
							// バッファ生成ステータスのセット
							_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
							dsbdesc.dwSize			= sizeof( dsbdesc ) ;
							dsbdesc.dwFlags			= D_DSBCAPS_STATIC | D_DSBCAPS_CTRLPAN | D_DSBCAPS_CTRLVOLUME ;
							dsbdesc.dwBufferBytes	= BufferSize ;
							dsbdesc.lpwfxFormat		= WaveFormat ;

							if( DSOUND.DirectSoundObject->CreateSoundBuffer( &dsbdesc , &Sound->Buffer[ i ] , NULL ) != D_DS_OK )
							{
								i -- ;
								while( i != -1 )
								{
									if( Sound->Buffer[ i ] ) Sound->Buffer[ i ]->Release() ;
									Sound->Buffer[ i ] = NULL ;

									i -- ;
								}
								return DxLib_Error( _T( "DirectSoundBuffer の作成に失敗しました" ) ) ;
							}
						}
					}
*/				}
			}

			// サウンドバッファ数を保存
			Sound->ValidBufferNum = BufferNum ;
		}
		break ;

	case DX_SOUNDTYPE_STREAMSTYLE :		// ストリーム風サウンド
		// バッファ生成ステータスのセット
		_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
		dsbdesc.dwSize			= sizeof( dsbdesc ) ;
		dsbdesc.dwFlags			= D_DSBCAPS_GLOBALFOCUS | D_DSBCAPS_CTRLPAN | D_DSBCAPS_CTRLVOLUME | D_DSBCAPS_CTRLFREQUENCY | D_DSBCAPS_GETCURRENTPOSITION2 | ( DSOUND.UseSoftwareMixing ? D_DSBCAPS_LOCSOFTWARE : D_DSBCAPS_STATIC )  ;
		dsbdesc.dwBufferBytes	= SOUNDSIZE( STS_BUFSEC * WaveFormat->nAvgBytesPerSec / STS_DIVNUM, WaveFormat->nBlockAlign ) ;
		dsbdesc.lpwfxFormat		= WaveFormat ;

//		if( DSOUND.DirectSoundObject->CreateSoundBuffer( &dsbdesc , &Sound->Buffer[ 0 ] , NULL ) != D_DS_OK )
		if( SoundBuffer_Initialize( &Sound->Buffer[ 0 ], &dsbdesc, NULL, Sound->Is3DSound ) != D_DS_OK )
		{
			DXST_ERRORLOG_ADD( _T( "ストリーム風サウンドバッファの作成に失敗しました\n" ) ) ;
			return -1 ;
		}

		// サウンドバッファ数を保存
		Sound->ValidBufferNum = 1 ;
		break ;
	}

	// データを保存
//	Sound->BufferFormat.nAvgBytesPerSec = WaveFormat->nAvgBytesPerSec ;
	Sound->Type = SoundType ;

	// ストリーム形式の場合はストリームサウンドハンドルリストに追加
	if( SoundType == DX_SOUNDTYPE_STREAMSTYLE )
	{
		// クリティカルセクションの取得
		CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		AddHandleList( &DSOUND.StreamSoundListFirst, &Sound->Stream.StreamSoundList, SoundHandle, Sound ) ;

		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;
	}

	// 終了
	return 0 ;
}

// 使用可能なサウンドバッファを取得する(-1:サウンドバッファの使用権を得られなかった)
static int _GetDirectSoundBuffer( int SoundHandle, SOUND * Sound, SOUNDBUFFER **BufferP, bool BufferGet )
{
	SOUND * UniSound = NULL ;

	// ストリームではなかったら使用権は得られる
	if( Sound->Type != DX_SOUNDTYPE_STREAMSTYLE )
	{
		if( BufferP ) *BufferP = &Sound->Buffer[ 0 ] ;
		return 0 ;
	}
	
	// 共有バッファを使用しているかどうかで分岐
	if( BufferP ) *BufferP = &Sound->Buffer[0] ;
	if( Sound->Stream.BufferBorrowSoundHandle == -1 )
	{
		// 使用していない場合は、使用されていなければ使用権を得られる
		if( Sound->Stream.BufferUseSoundHandle == -1 )
		{
			Sound->Stream.BufferUseSoundHandle = SoundHandle ;
			return 0 ;
		}
		
		// 使用されている場合で使用しているのが自分ではなく、
		// 使用権を得る指定も無い場合は使用権を得られない
		if( SoundHandle != Sound->Stream.BufferUseSoundHandle && BufferGet == false ) return -1 ;
		
		// 使用権を持っているのが自分ではない場合は再生をストップする
		if( SoundHandle != Sound->Stream.BufferUseSoundHandle )
			NS_StopStreamSoundMem( Sound->Stream.BufferUseSoundHandle ) ;

		// 使用権を得る
		Sound->Stream.BufferUseSoundHandle = SoundHandle ;
		return 0 ;
	}
	else
	{
		// 借り先が無効だったら使用権を得られない
		if( SOUNDHCHK( Sound->Stream.BufferBorrowSoundHandle, UniSound ) == true ) return -1 ;
		if( BufferP ) *BufferP = &UniSound->Buffer[0] ;
		
		// 誰も使用していない場合は使用権を得ることが出来る
		if( UniSound->Stream.BufferUseSoundHandle == -1 )
		{
			UniSound->Stream.BufferUseSoundHandle = SoundHandle ;
			return 0 ;
		}
		
		// 使用されている場合で使用しているのが自分ではなく、
		// 使用権を得る指定も無い場合は使用権を得られない
		if( SoundHandle != UniSound->Stream.BufferUseSoundHandle && BufferGet == false ) return -1 ;

		// 使用権を持っているのが自分ではない場合は再生をストップする
		if( SoundHandle != UniSound->Stream.BufferUseSoundHandle )
			NS_StopStreamSoundMem( UniSound->Stream.BufferUseSoundHandle ) ;

		// 使用権を得る
		UniSound->Stream.BufferUseSoundHandle = SoundHandle ;
		return 0 ;
	}
}


// AddStreamSoundMem のグローバル変数にアクセスしないバージョン
extern int AddStreamSoundMem_UseGParam(
	LOADSOUND_GPARAM *GParam,
	STREAMDATA *Stream,
	int LoopNum,
	int SoundHandle,
	int StreamDataType,
	int *CanStreamCloseFlag,
	int UnionHandle,
	int ASyncThread
)
{
	DWORD State ;
	SOUND * Sound ;
	STREAMFILEDATA *PlayData ;
	WAVEFORMATEX Format ;
	SOUND * UniSound;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, Sound ) )
			goto ERR ;
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, Sound ) )
			goto ERR ;
	}

	// 一つでもデータが存在する場合はチェック処理を入れる
	if( Sound->Stream.FileNum > 0 )
	{
		// ストリーム風サウンドではなかった場合は終了
		if( Sound->Type != DX_SOUNDTYPE_STREAMSTYLE )
		{
			DXST_ERRORLOG_ADD( _T( "ストリーム風サウンドではないサウンドデータが渡されました in AddSreamSound\n" ) ) ;
			goto ERR ;
		}

		// 既に再生中の場合も終了
		if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == 0 )
		{
//			SBuffer->GetStatus( &State ) ;
			SoundBuffer_GetStatus( SBuffer, &State ) ;

			if( State & D_DSBSTATUS_PLAYING )
			{
				DXST_ERRORLOG_ADD( _T( "既に再生中のストリームサウンドデータにデータを追加しようとしました\n" ) ) ;
				goto ERR ;
			}
		}

		// もうストリームサウンド制御データの領域に余裕がない場合も終了
		if( Sound->Stream.FileNum == STS_SETSOUNDNUM - 1 )
		{
			DXST_ERRORLOG_ADD( _T( "ストリーム風サウンドの制御データ領域がいっぱいです\n" ) ) ;
			goto ERR ;
		}
	}

	PlayData = &Sound->Stream.File[ Sound->Stream.FileNum ] ;
	_MEMSET( PlayData, 0, sizeof( STREAMFILEDATA ) ) ;

	// データタイプを保存
	PlayData->DataType = StreamDataType ;

	// ループ開始位置は無効
	PlayData->LoopStartSamplePosition = -1 ;

	// ストリームポインタを先頭位置へ
	Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_SET ) ;

	// 新しいＷＡＶＥファイルのロード、データのタイプによって処理を分岐
	switch( StreamDataType )
	{
	case DX_SOUNDDATATYPE_MEMNOPRESS :
		// メモリ上に直接再生可能なデータを保持する場合
		{
			STREAMDATA MemStream ;
			void *WaveImage ;
			int WaveSize ;

			// ＰＣＭ形式に変換
			{
				SOUNDCONV ConvData ;
				int res ;
				void *SrcBuffer ;
				int SrcSize ;
				
				// 丸々メモリに読み込む
				if( StreamFullRead( Stream, &SrcBuffer, &SrcSize ) < 0 )
				{
					DXST_ERRORLOGFMT_ADD(( _T( "音声データの読み込みに失敗しました" ) )) ;
					goto ERR ;
				}
				
				MemStream.DataPoint = MemStreamOpen( SrcBuffer, SrcSize ) ;
				MemStream.ReadShred = *GetMemStreamDataShredStruct() ;
			
				SetupSoundConvert( &ConvData, &MemStream, GParam->DisableReadSoundFunctionMask
#ifndef DX_NON_OGGVORBIS
									,GParam->OggVorbisBitDepth, GParam->OggVorbisFromTheoraFile
#endif
				) ;
				res = SoundConvertFast( &ConvData, &Format, &PlayData->FileImage, &PlayData->FileImageSize ) ;

				TerminateSoundConvert( &ConvData ) ;
				MemStreamClose( MemStream.DataPoint ) ;
				DXFREE( SrcBuffer ) ;

				if( res < 0 )
				{
					DXST_ERRORLOGFMT_ADD(( _T( "音声データの変換に失敗しました" ) )) ;
					goto ERR ;
				}
			}

			// ＷＡＶＥファイルをでっち上げる
			if( CreateWaveFileImage( &WaveImage, &WaveSize,
								 &Format, sizeof( WAVEFORMATEX ),
								 PlayData->FileImage, PlayData->FileImageSize ) < 0 )
				goto ERR ;

			// 展開されたデータをストリームとして再度開く				
			PlayData->MemStream.DataPoint = MemStreamOpen( WaveImage, WaveSize ) ;
			PlayData->MemStream.ReadShred = *GetMemStreamDataShredStruct() ;
			SetupSoundConvert( &PlayData->ConvData, &PlayData->MemStream, GParam->DisableReadSoundFunctionMask
#ifndef DX_NON_OGGVORBIS
				,GParam->OggVorbisBitDepth, GParam->OggVorbisFromTheoraFile
#endif
			) ;

			// メモリアドレスの入れ替え
			DXFREE( PlayData->FileImage ) ;
			PlayData->FileImage = WaveImage ;
			PlayData->FileImageSize = WaveSize ;

			// ストリーム解放フラグを立てる
			*CanStreamCloseFlag = TRUE ;
		}
		break ;

	case DX_SOUNDDATATYPE_MEMPRESS :
		// データはメモリ上に存在して、圧縮されている場合
		{
			// 丸々メモリに読み込む
			if( StreamFullRead( Stream, &PlayData->FileImage, &PlayData->FileImageSize ) < 0 )
			{
				DXST_ERRORLOGFMT_ADD(( _T( "音声データの読み込みに失敗しました" ) )) ;
				goto ERR ;
			}

			// 展開されたデータをストリームとして再度開く				
			PlayData->MemStream.DataPoint = MemStreamOpen( PlayData->FileImage, PlayData->FileImageSize ) ;
			PlayData->MemStream.ReadShred = *GetMemStreamDataShredStruct() ;
			if( SetupSoundConvert( &PlayData->ConvData, &PlayData->MemStream, GParam->DisableReadSoundFunctionMask
#ifndef DX_NON_OGGVORBIS
									,GParam->OggVorbisBitDepth, GParam->OggVorbisFromTheoraFile
#endif
									) < 0 )
			{
				MemStreamClose( PlayData->MemStream.DataPoint ) ;
				DXFREE( PlayData->FileImage ) ;
				PlayData->FileImage = NULL ;
				
				DXST_ERRORLOGFMT_ADD(( _T( "音声データのオープンに失敗しました" ) )) ;
				goto ERR ;
			}


			// ストリーム解放フラグを立てる
			*CanStreamCloseFlag = TRUE ;
		}
		break ;

	case DX_SOUNDDATATYPE_FILE :
		// データをファイルから逐次読み込んでくる場合
		{
			// 使わないデータに NULL をセットしておく
			PlayData->FileImage = NULL ;
			PlayData->FileImageSize = 0 ;
			PlayData->MemStream.DataPoint = NULL ;

			// 展開されたデータをストリームとして再度開く				
			if( SetupSoundConvert( &PlayData->ConvData, Stream, GParam->DisableReadSoundFunctionMask
#ifndef DX_NON_OGGVORBIS
									,GParam->OggVorbisBitDepth, GParam->OggVorbisFromTheoraFile
#endif
								) < 0 )
			{
				DXST_ERRORLOGFMT_ADD(( _T( "音声データのオープンに失敗しました" ) )) ;
				goto ERR ;
			}

			// ストリーム解放フラグを倒す
			*CanStreamCloseFlag = FALSE ;
		}
		break ;
	}
	
	// 一番最初のデータの場合は処理を分岐
	if( Sound->Stream.FileNum == 0 )
	{
		// フォーマットを取得
		GetOutSoundFormatInfo( &PlayData->ConvData, &Sound->BufferFormat ) ;

		// サウンドバッファを共有するかどうかを調べる
		UniSound = NULL;
		while( UnionHandle != -1 )
		{
			// 無効なハンドルの場合は無視
			if( SOUNDHCHK( UnionHandle, UniSound ) )
			{
				UniSound = NULL;
				UnionHandle = -1;
				break;
			}

			// フォーマットが違うか、オリジナルのバッファがないか、ストリーム形式でない場合もアウト
			if( UniSound->Type != DX_SOUNDTYPE_STREAMSTYLE ||
				UniSound->Stream.BufferBorrowSoundHandle != -1 ||
				_MEMCMP( &Sound->BufferFormat, &UniSound->BufferFormat, sizeof( WAVEFORMATEX ) ) != 0 )
			{
				UniSound = NULL;
				UnionHandle = -1;
				break;
			}
			break;
		}

		// サウンドバッファを共有するかどうかで処理を分岐
		if( UniSound == NULL )
		{
			// サウンドバッファを作成
			CreateDirectSoundBuffer( &Sound->BufferFormat, SOUNDSIZE( STS_BUFSEC * Sound->BufferFormat.nAvgBytesPerSec / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ), DX_SOUNDTYPE_STREAMSTYLE, 1, SoundHandle, -1, ASyncThread ) ;
			Sound->Stream.SoundBufferSize = SOUNDSIZE( STS_BUFSEC * Sound->BufferFormat.nAvgBytesPerSec / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
			Sound->BaseFrequency = Sound->BufferFormat.nSamplesPerSec ;

			// 共有情報をセット
			Sound->Stream.BufferBorrowSoundHandle = -1 ;
		}
		else
		{
			// クリティカルセクションの取得
			CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

			// ストリームサウンドハンドルリストに追加
			Sound->Type = DX_SOUNDTYPE_STREAMSTYLE ;
			Sound->ValidBufferNum = 1 ;
			AddHandleList( &DSOUND.StreamSoundListFirst, &Sound->Stream.StreamSoundList, SoundHandle, Sound ) ;

			// クリティカルセクションの解放
			CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

			// 共有情報をセット
			Sound->Stream.BufferBorrowSoundHandle = UnionHandle ;
			UniSound->Stream.BufferBorrowSoundHandleCount ++ ;
		}
		Sound->Stream.BufferBorrowSoundHandleCount = 0 ;
		Sound->Stream.BufferUseSoundHandle = -1 ;
		Sound->Stream.DeleteWaitFlag = FALSE ;
	}
	else
	{
		// 二つ目以降の場合はバッファのフォーマットと一致しているか調べる
		GetOutSoundFormatInfo( &PlayData->ConvData, &Format ) ;
		if( _MEMCMP( &Format, &Sound->BufferFormat, sizeof( WAVEFORMATEX ) ) != 0 )
		{
			TerminateSoundConvert( &PlayData->ConvData ) ;
			if( PlayData->MemStream.DataPoint != NULL )
			{
				MemStreamClose( PlayData->MemStream.DataPoint ) ;
				PlayData->MemStream.DataPoint = NULL ;
			}
			if( PlayData->FileImage != NULL )
			{
				DXFREE( PlayData->FileImage ) ;
				PlayData->FileImage = NULL ;
			}

			DXST_ERRORLOGFMT_ADD(( _T( "フォーマットの違う音声データは同時に使うことは出来ません" ) )) ;
			goto ERR ;
		}
	}

	// ループの数を保存
	PlayData->LoopNum = ( char )LoopNum ;

	// プレイデータの数を増やす
	Sound->Stream.FileNum ++ ;
	
	// ストリームデータの総サンプル数を更新
	{
		int i ;
		Sound->Stream.TotalSample = 0 ;
		for( i = 0 ; i < Sound->Stream.FileNum ; i ++ )
			Sound->Stream.TotalSample += GetSoundConvertDestSize_Fast( &Sound->Stream.File[i].ConvData ) / Sound->BufferFormat.nBlockAlign ;
	}

	// 終了
	return 0 ;

ERR :

	return -1 ;
}

// ストリーム風サウンドデータにサウンドデータを追加する
extern int NS_AddStreamSoundMem( STREAMDATA *Stream, int LoopNum, int SoundHandle, int StreamDataType, int *CanStreamCloseFlag, int UnionHandle )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;

	return AddStreamSoundMem_UseGParam( &GParam, Stream, LoopNum, SoundHandle, StreamDataType, CanStreamCloseFlag, UnionHandle, FALSE ) ;
}

// AddStreamSoundMemToFile のグローバル変数にアクセスしないバージョン
extern int AddStreamSoundMemToFile_UseGParam(
	LOADSOUND_GPARAM *GParam,
	const TCHAR *WaveFile,
	int LoopNum,
	int SoundHandle,
	int StreamDataType,
	int UnionHandle,
	int ASyncThread
)
{
	DWORD_PTR fp ;
	SOUND * Sound ;
	int CanStreamCloseFlag ;
	STREAMDATA Stream ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, Sound ) )
			goto ERR ;
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, Sound ) )
			goto ERR ;
	}

	// 打开文件
	fp = FOPEN( WaveFile ) ;
	if( fp == 0 )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "右記のＷＡＶファイルのロードに失敗しました：%s \n" ) , WaveFile )) ;
		goto ERR ;
	}

	// ストリームデータ情報をセットする
	Stream.DataPoint = (void *)fp ;
	Stream.ReadShred = *GetFileStreamDataShredStruct() ;

	// ストリームハンドルを追加
	if( AddStreamSoundMem_UseGParam( GParam, &Stream, LoopNum, SoundHandle, StreamDataType, &CanStreamCloseFlag, UnionHandle, ASyncThread ) == -1 )
	{
		FCLOSE( fp ) ;
		DXST_ERRORLOGFMT_ADD(( _T( "右記のＷＡＶファイルのセットアップに失敗しました：%s \n" ) , WaveFile )) ;
		goto ERR ;
	}

	// ファイルを閉じても良いとのお達しが来たらファイルを閉じる
	if( CanStreamCloseFlag == TRUE )
	{
		FCLOSE( fp ) ;
	}

	// ライブラリでロードした、フラグを立てる
	Sound->Stream.FileLibraryLoadFlag[ Sound->Stream.FileNum - 1 ] = TRUE ;

	// 終了
	return 0 ;

ERR :

	// エラー終了
	return -1 ;
}

// AddStreamSoundMemToMem のグローバル変数にアクセスしないバージョン
extern int AddStreamSoundMemToMem_UseGParam(
	LOADSOUND_GPARAM *GParam,
	const void *FileImageBuffer,
	int ImageSize,
	int LoopNum,
	int SoundHandle,
	int StreamDataType,
	int UnionHandle,
	int ASyncThread
)
{
	void *StreamHandle ;
	SOUND * Sound ;
	int CanStreamCloseFlag ;
	STREAMDATA Stream ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, Sound ) )
			goto ERR ;
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, Sound ) )
			goto ERR ;
	}

	// データタイプがファイルからの逐次読み込みだった場合はメモリにイメージを置く、に変更
	if( StreamDataType == DX_SOUNDDATATYPE_FILE ) StreamDataType = DX_SOUNDDATATYPE_MEMPRESS ;

	// メモリイメージストリームを開く
	if( ( StreamHandle = MemStreamOpen( FileImageBuffer, ImageSize ) ) == NULL )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "ＷＡＶファイルのストリームイメージハンドルの作成に失敗しました \n" ) )) ;
		goto ERR ;
	}

	// ストリームデータ情報をセット
	Stream.DataPoint = StreamHandle ;
	Stream.ReadShred = *GetMemStreamDataShredStruct() ;

	// ストリームハンドルを追加
	if( AddStreamSoundMem_UseGParam( GParam, &Stream, LoopNum, SoundHandle, StreamDataType, &CanStreamCloseFlag, UnionHandle, ASyncThread ) == -1 )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "ＷＡＶファイルのストリームイメージのセットアップに失敗しました \n" ) )) ;
		goto ERR ;
	}

	// ストリームを閉じても良いとのお達しが来たらストリームを閉じる
	if( CanStreamCloseFlag == TRUE )
	{
		MemStreamClose( StreamHandle ) ;
	}

	// ライブラリでロードした、フラグを立てる
	Sound->Stream.FileLibraryLoadFlag[ Sound->Stream.FileNum - 1 ] = TRUE ;

	// 終了
	return 0 ;

ERR:

	return -1 ;
}

// ストリーム風サウンドデータにサウンドデータを追加する
extern int NS_AddStreamSoundMemToFile( const TCHAR *WaveFile, int LoopNum,  int SoundHandle, int StreamDataType, int UnionHandle )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;
	return AddStreamSoundMemToFile_UseGParam( &GParam, WaveFile, LoopNum, SoundHandle, StreamDataType, UnionHandle, FALSE ) ;
}

// ストリーム風サウンドデータにサウンドデータを追加する
extern int NS_AddStreamSoundMemToMem( const void *FileImageBuffer, int ImageSize, int LoopNum, int SoundHandle, int StreamDataType, int UnionHandle )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;
	return AddStreamSoundMemToMem_UseGParam( &GParam, FileImageBuffer, ImageSize, LoopNum, SoundHandle, StreamDataType, UnionHandle, FALSE ) ;
}



// SetupStreamSoundMem のグローバル変数にアクセスしないバージョン
extern int SetupStreamSoundMem_UseGParam( int SoundHandle, int ASyncThread )
{
	SOUND * Sound ;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, Sound ) )
			goto ERR ;
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, Sound ) )
			goto ERR ;
	}

	// ストリームじゃなかったら何もせず終了
	if( Sound->Type != DX_SOUNDTYPE_STREAMSTYLE ) goto END ;

	// 再生中だったら何もせず終了
	if( ASyncThread == FALSE && NS_CheckStreamSoundMem( SoundHandle ) == 1 ) goto END ;

	// 使用権が無い場合は何もせず終了
	if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == -1 ) goto END ;

	// 準備が完了していたら何もせず終了
	if( Sound->Stream.StartSetupCompFlag == TRUE ) goto END ;

	// 再生位置を先頭にセット
//	SBuffer->SetCurrentPosition( 0 ) ;
	SoundBuffer_SetCurrentPosition( SBuffer, 0 ) ;

	// パラメータを初期化
	Sound->Stream.SoundBufferCompCopyOffset	 = -800 ;
	Sound->Stream.SoundBufferCopyStartOffset = -1 ;
	Sound->Stream.PrevCheckPlayPosition      = 0 ;
	Sound->Stream.EndWaitFlag                = FALSE ;

	// 圧縮データを展開する準備を行う
	{
		STREAMFILEDATA *PlayData ;

		PlayData = &Sound->Stream.File[ Sound->Stream.FileActive ] ;
//		SetTimeSoundConvert( &PlayData->ConvData, Sound->Stream.FileCompCopyLength * 1000 / Sound->BufferFormat.nAvgBytesPerSec ) ;
		SetSampleTimeSoundConvert( &PlayData->ConvData, Sound->Stream.FileCompCopyLength / Sound->BufferFormat.nBlockAlign ) ;
	}

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 準備完了
	Sound->Stream.StartSetupCompFlag = TRUE ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 最初のサウンドデータを書きこむ
	ProcessStreamSoundMem_UseGParam( SoundHandle, ASyncThread ) ;

END :

	// 終了
	return 0 ;

ERR :

	// 終了
	return 0 ;
}

// ストリーム風サウンドデータの再生準備を行う
extern int NS_SetupStreamSoundMem( int SoundHandle )
{
	return SetupStreamSoundMem_UseGParam( SoundHandle, FALSE ) ;
}


// ストリームサウンドハンドルの再生準備を行う( -1:エラー 0:正常終了 1:再生する必要なし )
static	int	_PlaySetupStreamSoundMem( int SoundHandle, SOUND * Sound, int PlayType, int TopPositionFlag, SOUNDBUFFER **DestSBuffer )
{
	SOUNDBUFFER *SBuffer ;
	int i ;

	// 使用するサウンドバッファを取得
	_GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer );
	if( DestSBuffer ) *DestSBuffer = SBuffer ;
	
	// 再生中で頭だしでもない場合は此処で終了
	if( TopPositionFlag == FALSE && NS_CheckStreamSoundMem( SoundHandle ) == 1 )
		return 1 ;

	// 再生中の場合は止める
	NS_StopStreamSoundMem( SoundHandle ) ;

	// 再生タイプが違ったら分岐
	if( Sound->PlayType != PlayType )
	{
		// 場合によっては頭だし再生にする
		if( ( Sound->PlayType & DX_PLAYTYPE_LOOPBIT ) != ( PlayType & DX_PLAYTYPE_LOOPBIT ) &&
			Sound->Stream.AllPlayFlag == TRUE )
		{
			Sound->Stream.AllPlayFlag = FALSE ;
			Sound->Stream.FileCompCopyLength = 0 ;
			Sound->Stream.CompPlayWaveLength = 0 ;
			Sound->Stream.LoopPositionValidFlag = FALSE ;
			Sound->Stream.FileLoopCount = 0 ;
			Sound->Stream.FileActive = 0 ;
			Sound->Stream.StartSetupCompFlag = FALSE ;		// 準備フラグを倒す
		}
	}

	// 頭からの再生だった場合の処理
	if( TopPositionFlag == TRUE )
	{
		Sound->Stream.AllPlayFlag = FALSE ;
		if( Sound->Stream.CompPlayWaveLength != 0 )
		{
			Sound->Stream.FileCompCopyLength = 0 ;
			Sound->Stream.CompPlayWaveLength = 0 ;
			Sound->Stream.LoopPositionValidFlag = FALSE ;
			Sound->Stream.FileLoopCount = 0 ;
			Sound->Stream.FileActive = 0 ;
			Sound->Stream.StartSetupCompFlag = FALSE ;		// 準備フラグを倒す
		}
	}

	// 再生タイプを保存
	Sound->PlayType = PlayType ;

	// 再生準備
	NS_SetupStreamSoundMem( SoundHandle ) ;

	// ボリュームをセットする
	for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
	{
		if( Sound->ValidNextPlayVolume[ i ] )
		{
			SoundBuffer_SetVolumeAll( &Sound->Buffer[ 0 ], ( LONG )Sound->NextPlayVolume[ i ] ) ;
			Sound->ValidNextPlayVolume[ i ] = 0 ;
		}
		else
		{
			SoundBuffer_SetVolumeAll( &Sound->Buffer[ 0 ], ( LONG )Sound->BaseVolume[ i ] ) ;
		}
	}

	// パンをセットする
	if( Sound->ValidNextPlayPan )
	{
		SoundBuffer_SetPan( &Sound->Buffer[ 0 ], ( LONG )Sound->NextPlayPan ) ;
		Sound->ValidNextPlayPan = 0 ;
	}
	else
	{
		SoundBuffer_SetPan( &Sound->Buffer[ 0 ], ( LONG )Sound->BasePan ) ;
	}

	// 周波数をセットする
	if( Sound->ValidNextPlayFrequency )
	{
		SoundBuffer_SetFrequency( &Sound->Buffer[ 0 ], ( DWORD )Sound->NextPlayFrequency ) ;
		Sound->ValidNextPlayFrequency = 0 ;
	}
	else
	{
		SoundBuffer_SetFrequency( &Sound->Buffer[ 0 ], ( DWORD )Sound->BaseFrequency ) ;
	}

	// ３Ｄサウンドの場合は３Ｄサウンドパラメータの更新を行う
	if( Sound->Is3DSound != FALSE )
	{
		VECTOR *Position ;
		float Radius ;
		VECTOR *Velocity ;

		if( Sound->ValidNextPlay3DPosition )
		{
			Position = &Sound->NextPlay3DPosition ;
			Sound->ValidNextPlay3DPosition = 0 ;
		}
		else
		{
			Position = &Sound->Base3DPosition ;
		}

		if( Sound->ValidNextPlay3DRadius )
		{
			Radius = Sound->NextPlay3DRadius ;
			Sound->ValidNextPlay3DRadius = 0 ;
		}
		else
		{
			Radius = Sound->Base3DRadius ;
		}

		if( Sound->ValidNextPlay3DVelocity )
		{
			Velocity = &Sound->NextPlay3DVelocity ;
			Sound->ValidNextPlay3DVelocity = 0 ;
		}
		else
		{
			Velocity = &Sound->Base3DVelocity ;
		}
		SoundBuffer_Set3DPosition( &Sound->Buffer[ 0 ], Position ) ;
		SoundBuffer_Set3DRadius(   &Sound->Buffer[ 0 ], Radius ) ;
		SoundBuffer_Set3DVelocity( &Sound->Buffer[ 0 ], Velocity ) ;

		SoundBuffer_Refresh3DSoundParam( &Sound->Buffer[ 0 ] ) ;
	}

	// 正常終了
	return 0 ;
}

// サウンドハンドルを作成する
static int _CreateSoundHandle( int Is3DSound )
{
	int SoundHandle ;
	SOUND *Sound ;

	SoundHandle = AddHandle( DX_HANDLETYPE_SOUND ) ;
	if( SoundHandle == -1 )
		return -1 ;

	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	Sound->Is3DSound = Is3DSound ;

	if( Sound->Is3DSound != FALSE )
	{
		// ３Ｄサウンド形式の場合は３Ｄサウンドハンドルリストに追加
		AddHandleList( &DSOUND._3DSoundListFirst, &Sound->_3DSoundList, SoundHandle, Sound ) ;
	}

	return SoundHandle ;
}

// ストリーム風サウンドデータの再生開始
extern int NS_PlayStreamSoundMem( int SoundHandle , int PlayType, int TopPositionFlag )
{
	SOUND * Sound ;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		// エラー終了
		return -1 ;
	}

	// 再生の準備
	if( _PlaySetupStreamSoundMem( SoundHandle, Sound, PlayType, TopPositionFlag, &SBuffer ) != 0 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		// エラー終了
		return -1 ;
	}

	// 再生開始
//	SBuffer->Play( 0 , 0 , D_DSBPLAY_LOOPING ) ; 
	SoundBuffer_Play( SBuffer, TRUE ) ;

	// ３Ｄサウンドの場合は再生中の３Ｄサウンドリストに追加する
	if( Sound->Is3DSound != FALSE )
	{
		if( Sound->AddPlay3DSoundList == FALSE )
		{
			Sound->AddPlay3DSoundList = TRUE ;
			AddHandleList( &DSOUND.Play3DSoundListFirst, &Sound->Play3DSoundList, SoundHandle, Sound ) ;
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 再生ステータスによっては再生終了を待つ
	if( PlayType == DX_PLAYTYPE_NORMAL )
	{
		while( NS_ProcessMessage() == 0 && NS_CheckSoundMem( SoundHandle ) == 1 ){ Sleep( 1 ) ; }
	}

	// 終了
	return 0 ;
}

// ストリーム風サウンドデータの再生状態を得る
extern int NS_CheckStreamSoundMem( int SoundHandle )
{
	DWORD State ;
	SOUND * Sound ;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// サウンドバッファの使用権が無い場合は少なくとも再生は行っていない
	if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == -1 ) return 0 ;

	// システム側で止めている場合は止める前の状態を返す
	if( Sound->BufferPlayStateBackupFlagValid[ 0 ] )
	{
		return Sound->BufferPlayStateBackupFlag[ 0 ] ? 1 : 0 ;
	}
	else
	{
		// ステータスを取得して調べる
//		SBuffer->GetStatus( &State ) ;
		SoundBuffer_GetStatus( SBuffer, &State ) ;
		if( State & D_DSBSTATUS_PLAYING ) return 1 ;
	}

	// ここまできていれば再生はされていない
	return 0 ;
}



// ストリーム風サウンドデータの再生終了
extern int NS_StopStreamSoundMem( int SoundHandle )
{
	SOUND *Sound, *UniSound = NULL ;
	DWORD State ;
	HRESULT hr ;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		// エラー終了
		return -1 ;
	}

	// 再生時間を更新する
	RefreshStreamSoundPlayCompLength( SoundHandle ) ;

	// ハンドルの使用権が無い場合は少なくとも再生中ではない
	if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == -1 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		return 0 ;
	}
	
	// 再生中の場合は止める
	if( Sound->Stream.BufferBorrowSoundHandle != -1 )
	{
		if( SOUNDHCHK( Sound->Stream.BufferBorrowSoundHandle, UniSound ) == false )
		{
			UniSound->Stream.BufferUseSoundHandle = -1;
		}
	}

//	hr = SBuffer->GetStatus( &State ) ;
	hr = SoundBuffer_GetStatus( SBuffer, &State ) ;
	if( hr != D_DS_OK ) return -1 ;
	if( State & D_DSBSTATUS_PLAYING )
	{
//		SBuffer->Stop() ;
		SoundBuffer_Stop( SBuffer, TRUE ) ;

		Sound->BufferPlayStateBackupFlagValid[ 0 ] = FALSE ;
		Sound->BufferPlayStateBackupFlag[ 0 ] = FALSE ;
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// サウンドハンドルの再生位置をバイト単位で変更する(再生が止まっている時のみ有効)
extern int NS_SetStreamSoundCurrentPosition( int Byte, int SoundHandle )
{
	SOUND * sd ;
//	STREAMFILEDATA * pl ;
//	int i, pos ;
//	int datasize ;
//	WAVEFORMATEX *wf ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		// エラー終了
		return -1 ;
	}

	if( Byte / sd->BufferFormat.nBlockAlign > sd->Stream.TotalSample )
		goto ERR ;

	// ファイルが二つ以上の場合はまだ未対応
	if( sd->Stream.FileNum > 1 ) goto ERR ;

/*
***	圧縮データの場合は正常に計算できなかった ****

	// 再生位置の算出
	pl = sd->Stream.File ;
	pos = Byte ;
	for( i = 0 ; i < sd->Stream.FileNum ; i ++, pl ++ )
	{
		wf = &sd->BufferFormat ;
		datasize = pl->ConvData.DataSize ;
		if( pos < datasize ) break ;
		pos -= datasize ;
	}
	if( i == sd->Stream.FileNum ) return -1 ;
*/
	sd->Stream.FileLoopCount = 0 ;
	sd->Stream.LoopPositionValidFlag = FALSE ;
//	sd->Stream.FileActive = i ;
	sd->Stream.FileActive = 0 ;
	sd->Stream.CompPlayWaveLength = Byte ;
	sd->Stream.FileCompCopyLength = Byte ;

	// 準備完了フラグを倒す
	sd->Stream.StartSetupCompFlag = FALSE ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;

ERR :
	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	return -1 ;
}

// サウンドハンドルの再生位置をバイト単位で取得する
extern int NS_GetStreamSoundCurrentPosition( int SoundHandle )
{
	SOUND * sd ;
//	STREAMFILEDATA * pl ;
	int /*i,*/ pos ;
//	WAVEFORMATEX *wf ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		// エラー終了
		return -1 ;
	}

	// 再生位置の算出
/*	pl = sd->Stream.File ;
	pos = 0 ;
	for( i = 0 ; i < sd->Stream.FileNum ; i ++, pl ++ )
	{
		wf = &sd->BufferFormat ;
		pos += pl->ConvData.DataSize ;
	}
	pos += sd->Stream.FileCompCopyLength ;
*/
	// 再生時間の更新
	RefreshStreamSoundPlayCompLength( SoundHandle ) ;

	// 再生時間の取得
	pos = sd->Stream.CompPlayWaveLength ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 値を返す
	return pos ;
}

// サウンドハンドルの再生位置をミリ秒単位で設定する(圧縮形式の場合は正しく設定されない場合がある)
extern int NS_SetStreamSoundCurrentTime( int Time, int SoundHandle )
{
	SOUND * sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// 再生位置の変更
	return NS_SetStreamSoundCurrentPosition( MilliSecPositionToSamplePosition( sd->BufferFormat.nSamplesPerSec, Time ) * sd->BufferFormat.nBlockAlign, SoundHandle ) ;
}

// サウンドハンドルの再生位置をミリ秒単位で取得する(圧縮形式の場合は正しい値が返ってこない場合がある)
extern int NS_GetStreamSoundCurrentTime( int SoundHandle )
{
	SOUND * sd ;
//	STREAMFILEDATA * pl ;
	int /*i,*/ time ;
//	int datasize ;
//	WAVEFORMATEX *wf ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		// エラー終了
		return -1 ;
	}

	// 再生時間の更新
	RefreshStreamSoundPlayCompLength( SoundHandle ) ;

	// 再生時間をミリ秒単位に変換
//	time = _DTOL( (double)sd->Stream.CompPlayWaveLength / sd->BufferFormat.nAvgBytesPerSec * 1000 ) ;
	time = SamplePositionToMilliSecPosition( sd->BufferFormat.nSamplesPerSec, sd->Stream.CompPlayWaveLength / sd->BufferFormat.nBlockAlign ) ;
	
	// 再生時間が最初に取得した音の長さよりも長いことがあるので
	// もし再生時間の方が長くなってしまった場合は補正する
	if( time > NS_GetSoundTotalTime( SoundHandle ) )
		time = NS_GetSoundTotalTime( SoundHandle ) ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 値を返す
	return time ;
}

// サウンドハンドルの再生完了時間を更新する
extern int RefreshStreamSoundPlayCompLength( int SoundHandle, int CurrentPosition, int ASyncThread )
{
	SOUND * sd ;
	DWORD CurPosition ;
	int UpdateFlag ;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, sd ) )
			return -1 ;
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, sd ) )
			return -1 ;
	}

	// ストリーム再生用データではない場合は何もせず終了
	if( sd->Type != DX_SOUNDTYPE_STREAMSTYLE ) return 0 ;
	
	// サウンドバッファの使用権が得られない場合は少なくとも再生中ではない
	if( _GetDirectSoundBuffer( SoundHandle, sd, &SBuffer, false ) == -1 ) return 0 ;
	
	// 再生準備が完了していない時も終了
	if( sd->Stream.StartSetupCompFlag == FALSE ) return 0 ;

	// 現在の再生位置を取得する
	if( CurrentPosition == -1 )
//		SBuffer->GetCurrentPosition( &CurPosition , NULL );
		SoundBuffer_GetCurrentPosition( SBuffer, &CurPosition , NULL ) ;
	else
		CurPosition = CurrentPosition ;

	// 前回の位置との差分を加算する
	if( sd->Stream.PrevCheckPlayPosition != CurPosition )
	{
		UpdateFlag = 0 ;

		// ループ指定がある場合と無い場合で処理を分岐
		if( sd->Stream.LoopPositionValidFlag == TRUE )
		{
			if( sd->Stream.PrevCheckPlayPosition < CurPosition )
			{
				if( sd->Stream.LoopPosition > sd->Stream.PrevCheckPlayPosition &&
					sd->Stream.LoopPosition <= CurPosition )
				{
					sd->Stream.CompPlayWaveLength = CurPosition - sd->Stream.LoopPosition ;
					UpdateFlag = 1 ;
				}
			}
			else
			{
				if( sd->Stream.LoopPosition > sd->Stream.PrevCheckPlayPosition )
				{
					sd->Stream.CompPlayWaveLength = sd->Stream.SoundBufferSize - sd->Stream.LoopPosition + CurPosition ;
					UpdateFlag = 1 ;
				}
				else
				if( sd->Stream.LoopPosition < CurPosition )
				{
					sd->Stream.CompPlayWaveLength = CurPosition - sd->Stream.LoopPosition ;
					UpdateFlag = 1 ;
				}
			}
		}
		if( UpdateFlag == 1 )
		{
			sd->Stream.CompPlayWaveLength += sd->Stream.LoopAfterCompPlayWaveLength ;
			sd->Stream.LoopPositionValidFlag = FALSE ;
		}
		else
		{
			if( sd->Stream.PrevCheckPlayPosition < CurPosition )
			{
				sd->Stream.CompPlayWaveLength += CurPosition - sd->Stream.PrevCheckPlayPosition ;
			}
			else
			{
				sd->Stream.CompPlayWaveLength += ( sd->Stream.SoundBufferSize - sd->Stream.PrevCheckPlayPosition ) + CurPosition ;
			}
		}
		
		// 今回取得した位置をセットする
		sd->Stream.PrevCheckPlayPosition = CurPosition ;
	}

	// 終了
	return 0 ;
}

// ストリームサウンド処理用スレッド
static	DWORD WINAPI StreamSoundThreadFunction( void * )
{
	for(;;)
	{
		if( DSOUND.StreamSoundThreadEndFlag == 1 ) break ;
		if( DSOUND.InitializeFlag == FALSE ) break ;

		// クリティカルセクションの取得
		CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		// ストリーミング処理
		NS_ProcessStreamSoundMemAll() ;

		// 再生が終了したらハンドルを削除する処理を行う
		ProcessPlayFinishDeleteSoundMemAll() ;

		// ３Ｄサウンドを再生しているサウンドハンドルに対する処理を行う
		ProcessPlay3DSoundMemAll() ;

		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;


		// クリティカルセクションの取得
		CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

		// ストリーミング処理
		ST_SoftSoundPlayerProcessAll() ;

		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

		// 待ち
		Sleep( 10 ) ;
	}

	// スレッド終了
	DSOUND.StreamSoundThreadEndFlag = 2 ;
	ExitThread( 0 ) ;

	return 0 ;
}


// ProcessStreamSoundMem 関数の補助関数
// 無音データを書き込む
void NoneSoundDataCopy( SOUND * Sound, SOUNDBUFFERLOCKDATA *LockData, DWORD MoveLength )
{
	DWORD MLen, MOff ;
	BYTE *WOff ;

	MOff = 0 ;

	while( MoveLength != 0 )
	{
		MLen = MoveLength ;

		if( LockData->Valid != 0 )
		{
			WOff = LockData->WriteP + LockData->Offset ;

			if( MLen > LockData->Valid ) MLen = LockData->Valid ;

			if( Sound->BufferFormat.wBitsPerSample == 8 )	_MEMSET( WOff, 127, MLen ) ;
			else											_MEMSET( WOff, 0,   MLen ) ;

			LockData->Valid -= MLen ;
			LockData->Offset += MLen ;
			MoveLength -= MLen ;
			MOff += MLen ;
		}
		else
		{
			if( LockData->Valid2 == 0 ) break ;

			WOff = LockData->WriteP2 + LockData->Offset2 ;

			if( MLen > LockData->Valid2 ) MLen = LockData->Valid2 ;

			if( Sound->BufferFormat.wBitsPerSample == 8 )	_MEMSET( WOff, 127, MLen ) ;
			else											_MEMSET( WOff, 0,   MLen ) ;

			LockData->Valid2 -= MLen ;
			LockData->Offset2 += MLen ;
			MoveLength -= MLen ;
			MOff += MLen ;
		}
	}
}


// ProcessStreamSoundMem 関数の補助関数
// 音声データを書き込む
int SoundDataCopy( SOUNDBUFFERLOCKDATA *LockData, SOUNDCONV *ConvData, DWORD MoveLength )
{
	DWORD MLen, MAllLen ;
	BYTE *WOff ;

	MAllLen = 0 ;

	while( MoveLength != 0 )
	{
		MLen = MoveLength ;

		if( LockData->Valid != 0 )
		{
			WOff = LockData->WriteP + LockData->Offset ;

			if( MLen > LockData->Valid ) MLen = LockData->Valid ;
			MLen = RunSoundConvert( ConvData, WOff, MLen ) ;
			if( MLen == 0 ) break ;

			LockData->Valid -= MLen ;
			LockData->Offset += MLen ;
			MoveLength -= MLen ;
			MAllLen += MLen ;
		}
		else
		{
			WOff = LockData->WriteP2 + LockData->Offset2 ;

			if( MLen > LockData->Valid2 ) MLen = LockData->Valid2 ;
			MLen = RunSoundConvert( ConvData, WOff, MLen ) ;
			if( MLen == 0 ) break ;

			LockData->Valid2 -= MLen ;
			LockData->Offset2 += MLen ;
			MoveLength -= MLen ;
			MAllLen += MLen ;
		}
	}
	
	return MAllLen ;
}

// ProcessStreamSoundMem関数の補助関数
// 次のファイルに移るかどうかをチェックする
int StreamSoundNextData( SOUND * Sound, SOUNDBUFFERLOCKDATA *LockData, int CurrentPosition )
{
	STREAMFILEDATA *PlayData ;
		
	// ループ数を増やす
	Sound->Stream.FileLoopCount ++ ;

	// 無限ループかどうかで処理を分岐
	if( Sound->Stream.File[Sound->Stream.FileActive].LoopNum == -1 )
	{
		WAVEFORMATEX *wfmt = &Sound->BufferFormat ;

		PlayData = &Sound->Stream.File[ Sound->Stream.FileActive ] ;

		// 既にループ位置をセットしてある場合は一度ループするまで待つ
		if( Sound->Stream.LoopPositionValidFlag == TRUE )
			return -1 ;

		// 再生位置をセットする
		Sound->Stream.FileCompCopyLength = PlayData->LoopSamplePosition * wfmt->nBlockAlign ;

		// 再生アドレス更新用情報をセット
		Sound->Stream.LoopAfterCompPlayWaveLength = PlayData->LoopSamplePosition * wfmt->nBlockAlign ;
		Sound->Stream.LoopPositionValidFlag = TRUE ;
		if( LockData->Valid == 0 )
		{
			Sound->Stream.LoopPosition = LockData->Length2 - LockData->Valid2 ;
		}
		else
		{
			Sound->Stream.LoopPosition = LockData->StartOffst + LockData->Length - LockData->Valid ;
		}
	}
	else
	{
		// ループ回数が規定値にきていたら次のデータへ
		if( Sound->Stream.FileLoopCount > Sound->Stream.File[Sound->Stream.FileActive].LoopNum  )
		{
			// ループ再生指定でループする条件が揃った場合、既にループ位置をセットしてある場合は一度ループするまで待つ
			if( Sound->Stream.FileNum - 1 == Sound->Stream.FileActive &&
				Sound->PlayType == DX_PLAYTYPE_LOOP &&
				Sound->Stream.LoopPositionValidFlag == TRUE )
				return -1 ;

			Sound->Stream.FileLoopCount = 0 ;
			Sound->Stream.FileActive ++ ;
			if( Sound->Stream.FileNum == Sound->Stream.FileActive )
			{
				Sound->Stream.AllPlayFlag = TRUE ;
			
				if( Sound->PlayType == DX_PLAYTYPE_LOOP )
				{
					Sound->Stream.FileActive = 0  ;
	//				Sound->Stream.CompPlayWaveLength = 0 ;

					Sound->Stream.LoopAfterCompPlayWaveLength = 0 ;
					Sound->Stream.LoopPositionValidFlag = TRUE ;
					if( LockData->Valid == 0 )
					{
						Sound->Stream.LoopPosition = LockData->Length2 - LockData->Valid2 ;
					}
					else
					{
						Sound->Stream.LoopPosition = LockData->StartOffst + LockData->Length - LockData->Valid ;
					}
				}
				else
				{
					// 終了フラグを立てる
					Sound->Stream.EndWaitFlag = TRUE ;

					// 終了オフセットをセットする
					if( LockData->Valid == 0 )
					{
						Sound->Stream.EndOffset = LockData->Length2 - LockData->Valid2 ;
					}
					else
					{
						Sound->Stream.EndOffset = LockData->StartOffst + LockData->Length - LockData->Valid ;
					}
					Sound->Stream.EndStartOffset = CurrentPosition ;
					NoneSoundDataCopy( Sound, LockData, AdjustSoundDataBlock( Sound->BufferFormat.nAvgBytesPerSec / STS_ONECOPYSEC, Sound ) ) ;

					return 0 ;
				}
			}
		}

		// 再生情報の初期化
		Sound->Stream.FileCompCopyLength = 0 ;

	}

	// 圧縮データを展開する準備を行う
	PlayData = &Sound->Stream.File[ Sound->Stream.FileActive ] ;
//	SetTimeSoundConvert( &PlayData->ConvData,  Sound->Stream.FileCompCopyLength * 1000 / Sound->BufferFormat.nAvgBytesPerSec ) ;
	SetSampleTimeSoundConvert( &PlayData->ConvData,  Sound->Stream.FileCompCopyLength / Sound->BufferFormat.nBlockAlign ) ;

	// 終了
	return 0 ;
}


// ProcessStreamSoundMem のグローバル変数にアクセスしないバージョン
extern int ProcessStreamSoundMem_UseGParam( int SoundHandle, int ASyncThread )
{
	SOUND * Sound ;
	STREAMFILEDATA *PlayData ;
	DWORD State ;
	DWORD CurPosition = 0 ;
	SOUNDBUFFERLOCKDATA LockData ;
	int MoveByte, MoveByte2, MoveStartOffset ;
	int MoveB ;
	SOUNDBUFFER *SBuffer ;
	int BreakFlag ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;
	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// エラーチェック
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, Sound ) )
			return -1 ;
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, Sound ) )
			return -1 ;
	}

	// 再生形式がストリームタイプじゃなかったらここで終了
	if( Sound->Type != DX_SOUNDTYPE_STREAMSTYLE ) return 0 ;
	
	// サウンドバッファの使用権がない場合もここで終了
	if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == -1 ) return 0 ;
	
	// 再生準備が完了していなかったら何もせず終了
	if( Sound->Stream.StartSetupCompFlag == FALSE ) return 0 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 再度ハンドルチェック
	if( ASyncThread )
	{
		if( SOUNDHCHK_ASYNC( SoundHandle, Sound ) )
		{
			// クリティカルセクションの解放
			CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

			// エラー終了
			return -1 ;
		}
	}
	else
	{
		if( SOUNDHCHK( SoundHandle, Sound ) )
		{
			// クリティカルセクションの解放
			CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

			// エラー終了
			return -1 ;
		}
	}

	// 再生完了時間の更新
	RefreshStreamSoundPlayCompLength( SoundHandle, -1, ASyncThread ) ;

	// エンドフラグが立っている場合は終了判定処理を行う
	if( Sound->Stream.EndWaitFlag )
	{
		// 現在の再生位置を得る
//		SBuffer->GetCurrentPosition( &CurPosition , NULL );
		SoundBuffer_GetCurrentPosition( SBuffer, &CurPosition, NULL ) ;
  
		// 終了判定
		if(
		    (
			  Sound->Stream.EndStartOffset   < Sound->Stream.EndOffset &&
			  (
			    Sound->Stream.EndStartOffset > CurPosition ||
				Sound->Stream.EndOffset      < CurPosition
			  )
			) ||
			(
			  Sound->Stream.EndOffset        < CurPosition &&
			  Sound->Stream.EndStartOffset   > CurPosition &&
			  Sound->Stream.EndStartOffset   > Sound->Stream.EndOffset
			)
		  )
		{
			// 再生をとめる
//			SBuffer->Stop() ;
			SoundBuffer_Stop( SBuffer ) ;

			// エンドフラグを倒す
			Sound->Stream.EndWaitFlag = FALSE ;

			// 再生時間情報を初期化する
			Sound->Stream.FileCompCopyLength = 0 ;
			Sound->Stream.CompPlayWaveLength = 0 ;
			Sound->Stream.LoopPositionValidFlag = FALSE ;
			Sound->Stream.FileLoopCount = 0 ;
			Sound->Stream.FileActive = 0 ;
			Sound->Stream.StartSetupCompFlag = FALSE ;		// 準備フラグを倒す
		}
		else
		{
			MoveByte = AdjustSoundDataBlock( Sound->BufferFormat.nAvgBytesPerSec / STS_ONECOPYSEC, Sound ) ;
			MoveStartOffset = Sound->Stream.SoundBufferCompCopyOffset ;

			// 転送すべきかどうかの判定
			{
				DWORD C, S ;

//				SBuffer->GetCurrentPosition( &CurPosition , NULL );
				SoundBuffer_GetCurrentPosition( SBuffer, &CurPosition, NULL ) ;

				C = Sound->Stream.SoundBufferCompCopyOffset ;
				S = Sound->Stream.SoundBufferCopyStartOffset ;
				if( ( C < S && ( S > CurPosition && C < CurPosition ) ) || 
					( C > S && ( S > CurPosition || C < CurPosition ) ) )
				{
					goto ERR ;
				}
			}

			MoveB = MoveByte ;

			// ロック処理
			{
				LockData.StartOffst = MoveStartOffset ;

				if( Sound->Stream.SoundBufferCompCopyOffset > (int)SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_BUFSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) )
					Sound->Stream.SoundBufferCompCopyOffset = Sound->Stream.SoundBufferCompCopyOffset ;

//				SBuffer->Lock( MoveStartOffset, MoveByte,
//								( void ** )&(LockData.WriteP), &LockData.Length,
//								( void ** )&(LockData.WriteP2), &LockData.Length2, 0 ) ; 
				SoundBuffer_Lock( SBuffer, MoveStartOffset, MoveByte,
								( void ** )&(LockData.WriteP), &LockData.Length,
								( void ** )&(LockData.WriteP2), &LockData.Length2 ) ; 
				LockData.Offset  = 0 ;
				LockData.Offset2 = 0 ;
				LockData.Valid  = LockData.Length ;
				LockData.Valid2 = LockData.Length2 ;
			}

			// データ転送処理
			while( MoveByte != 0 )
			{
				// 使用するデータをセット
				PlayData = &Sound->Stream.File[ Sound->Stream.FileActive ] ;

				// 転送バイトのセット
				MoveByte2 = MoveByte ;

				// 传送
				NoneSoundDataCopy( Sound, &LockData, MoveByte2 ) ;
				MoveByte -= MoveByte2 ;
			}

			// ロック解除
//			SBuffer->Unlock( ( void * )LockData.WriteP, LockData.Length,
//							( void * )LockData.WriteP2, LockData.Length2 ) ; 
			SoundBuffer_Unlock( SBuffer, ( void * )LockData.WriteP, LockData.Length,
										( void * )LockData.WriteP2, LockData.Length2 ) ; 

			// 次に転送処理を行うオフセットの変更
			{
				Sound->Stream.SoundBufferCompCopyOffset += MoveB ;
				if( Sound->Stream.SoundBufferCompCopyOffset >= (int)SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_BUFSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) )
					Sound->Stream.SoundBufferCompCopyOffset -= (int)SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_BUFSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;

				Sound->Stream.SoundBufferCopyStartOffset = Sound->Stream.SoundBufferCompCopyOffset - SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_SAKICOPYSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
				if( Sound->Stream.SoundBufferCopyStartOffset < 0 )
					Sound->Stream.SoundBufferCopyStartOffset += SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_BUFSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
			}
		}

		// 終了
		goto END ;
	}
	else
	{
		// 転送処理準備
		{
			// 現在演奏されているかチェック
			State = 0 ;
			//SBuffer->GetStatus( &State ) ;
			SoundBuffer_GetStatus( SBuffer, &State ) ;

			// 初期化中でもなく、演奏中でもない場合はここで終了
			if( ( State & D_DSBSTATUS_PLAYING ) == 0 )
			{
				if( Sound->Stream.SoundBufferCompCopyOffset == -800 )
				{
					MoveByte = AdjustSoundDataBlock( Sound->BufferFormat.nAvgBytesPerSec * STS_SAKICOPYSEC / STS_DIVNUM, Sound ) ;
					MoveStartOffset = 0 ;
				}
				else
				{
					goto END ;
				}
			}
			else
			{
				DWORD SakiyomiSec ;
				MoveByte = AdjustSoundDataBlock( Sound->BufferFormat.nAvgBytesPerSec / STS_ONECOPYSEC, Sound ) ;
				MoveStartOffset = Sound->Stream.SoundBufferCompCopyOffset ;

				// 転送すべきかどうかの判定、現在の先読みの秒数の算出
				{
					DWORD C, S ;

//					SBuffer->GetCurrentPosition( &CurPosition , NULL );
					SoundBuffer_GetCurrentPosition( SBuffer, &CurPosition, NULL ) ;
					
					C = Sound->Stream.SoundBufferCompCopyOffset ;
					S = Sound->Stream.SoundBufferCopyStartOffset ;
					if( ( C < S && ( S > CurPosition && C < CurPosition ) ) || 
						( C > S && ( S > CurPosition || C < CurPosition ) ) )
					{
						goto ERR ;
					}

					if( C < S )
					{
						if( CurPosition < C ) CurPosition += SOUNDSIZE( STS_BUFSEC * Sound->BufferFormat.nAvgBytesPerSec / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
						C += SOUNDSIZE( STS_BUFSEC * Sound->BufferFormat.nAvgBytesPerSec / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
					}
					SakiyomiSec = ( C - CurPosition ) * 0x200 / Sound->BufferFormat.nAvgBytesPerSec ;
				}

				// 先読み秒数が規定を下回っていたら規定の秒数まで一気に転送
				if( SakiyomiSec < STS_MINSAKICOPYSEC * 0x200 / STS_DIVNUM )
				{
					MoveByte = AdjustSoundDataBlock( Sound->BufferFormat.nAvgBytesPerSec * ( STS_MINSAKICOPYSEC * 0x200 / STS_DIVNUM - SakiyomiSec ) / 0x200, Sound ) ;
				}
			}
		}

		MoveB = MoveByte ;

		// ロック処理
		{
			LockData.StartOffst = MoveStartOffset ;

//			SBuffer->Lock( MoveStartOffset, MoveByte,
//							( void ** )&(LockData.WriteP), &LockData.Length,
//							( void ** )&(LockData.WriteP2), &LockData.Length2, 0 ) ; 
			SoundBuffer_Lock( SBuffer, MoveStartOffset, MoveByte,
							( void ** )&(LockData.WriteP), &LockData.Length,
							( void ** )&(LockData.WriteP2), &LockData.Length2 ) ; 
			LockData.Offset  = 0 ;
			LockData.Offset2 = 0 ;
			LockData.Valid  = LockData.Length ;
			LockData.Valid2 = LockData.Length2 ;
		}

		// データ転送処理
		while( MoveByte != 0 && Sound->Stream.EndWaitFlag == FALSE )
		{
			// 使用するデータをセット
			PlayData = &Sound->Stream.File[ Sound->Stream.FileActive ] ;

			// 転送バイトのセット
			MoveByte2 = MoveByte ;

			// ループ開始位置が設定されている場合はその位置より先まではデータをセットしない
			if( PlayData->LoopStartSamplePosition != -1 && MoveByte2 + Sound->Stream.FileCompCopyLength > PlayData->LoopStartSamplePosition * Sound->BufferFormat.nBlockAlign )
			{
				MoveByte2 = PlayData->LoopStartSamplePosition * Sound->BufferFormat.nBlockAlign - Sound->Stream.FileCompCopyLength ;
				if( MoveByte2 <= 0 )
					MoveByte2 = 0 ;
			}

			// 传送
			MoveByte2 = SoundDataCopy( &LockData, &PlayData->ConvData, MoveByte2 ) ;

			// 転送量が０バイトの場合は次のファイルに移る
			BreakFlag = FALSE ;
			if( MoveByte2 == 0 )
			{
//				CurPosition = 0;
				if( StreamSoundNextData( Sound, &LockData, CurPosition ) < 0 )
				{
					BreakFlag = TRUE ;
				}
			}
			else
			{
				MoveByte -= MoveByte2 ;
				Sound->Stream.FileCompCopyLength += MoveByte2 ;
			}

			// ループすべき位置に来ているのに次の再生データに移行していない場合は再生ループ待ち( Sound->Stream.LoopPositionValidFlag が TRUE )の状態なので、ループから抜ける
			if( ( PlayData == &Sound->Stream.File[ Sound->Stream.FileActive ] &&
				  PlayData->LoopStartSamplePosition != -1 &&
				  Sound->Stream.FileCompCopyLength >= PlayData->LoopStartSamplePosition * Sound->BufferFormat.nBlockAlign ) ||
				BreakFlag )
				break ;
		}
		MoveB -= MoveByte ;

		// ロック解除
//		SBuffer->Unlock( ( void * )LockData.WriteP,  LockData.Length,
//						  ( void * )LockData.WriteP2, LockData.Length2 ) ; 
		SoundBuffer_Unlock( SBuffer, ( void * )LockData.WriteP,  LockData.Length,
						  			( void * )LockData.WriteP2, LockData.Length2 ) ; 

		// 次に転送処理を行うオフセットの変更
		if( Sound->Stream.SoundBufferCompCopyOffset == -800 )
		{
//			Sound->Stream.SoundBufferCompCopyOffset = SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_SAKICOPYSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
			Sound->Stream.SoundBufferCompCopyOffset = MoveB ;
			Sound->Stream.SoundBufferCopyStartOffset = 0 ;
		}
		else
		{
			Sound->Stream.SoundBufferCompCopyOffset += MoveB ;
			if( Sound->Stream.SoundBufferCompCopyOffset >= (int)SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_BUFSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) )
				Sound->Stream.SoundBufferCompCopyOffset -= (int)SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_BUFSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;

			Sound->Stream.SoundBufferCopyStartOffset = Sound->Stream.SoundBufferCompCopyOffset - SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_SAKICOPYSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
			if( Sound->Stream.SoundBufferCopyStartOffset < 0 )
				Sound->Stream.SoundBufferCopyStartOffset += SOUNDSIZE( Sound->BufferFormat.nAvgBytesPerSec * STS_BUFSEC / STS_DIVNUM, Sound->BufferFormat.nBlockAlign ) ;
		}
	}

END :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;

ERR :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー終了
	return -1 ;
}

// 全ての音の一時停止状態を変更する
extern int PauseSoundMemAll( int PauseFlag )
{
	HANDLELIST *List ;
	SOUND *Sound ;
	DWORD State ;
	int i ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 停止するか開始するかで処理を分岐
	if( PauseFlag )
	{
		// 停止する場合

		for( List = HandleManageArray[ DX_HANDLETYPE_SOUND ].ListFirst.Next ; List->Next != NULL ; List = List->Next )
		{
			Sound = ( SOUND * )List->Data ;

			for( i = 0 ; i < MAX_SOUNDBUFFER_NUM ; i ++ )
			{
				if( Sound->Buffer[ i ].Valid == FALSE || Sound->Buffer[ i ].DSBuffer == NULL )
					continue ;

				// 既に状態保存済みの場合は何もしない
				if( Sound->BufferPlayStateBackupFlagValid[ i ] )
					continue ;

				// サウンドバッファの再生状態を保存
				if( SoundBuffer_GetStatus( &Sound->Buffer[ i ], &State ) == D_DS_OK )
				{
					Sound->BufferPlayStateBackupFlagValid[ i ] = TRUE ;

					if( State & D_DSBSTATUS_PLAYING )
					{
						Sound->BufferPlayStateBackupFlag[ i ] = TRUE ;

						// 再生されていたら再生を止める
						SoundBuffer_Stop( &Sound->Buffer[ i ], TRUE ) ;
					}
					else
					{
						Sound->BufferPlayStateBackupFlag[ i ] = FALSE ;
					}
				}
				else
				{
					Sound->BufferPlayStateBackupFlagValid[ i ] = FALSE ;
				}
			}
		}
	}
	else
	{
		// 再開する場合

		for( List = HandleManageArray[ DX_HANDLETYPE_SOUND ].ListFirst.Next ; List->Next != NULL ; List = List->Next )
		{
			Sound = ( SOUND * )List->Data ;

			for( i = 0 ; i < MAX_SOUNDBUFFER_NUM ; i ++ )
			{
				if( Sound->Buffer[ i ].Valid == FALSE || Sound->Buffer[ i ].DSBuffer == NULL ) continue ;

				// サウンドバッファの再生状態が有効で、且つ再生していた場合は再生を再開する
				if( Sound->BufferPlayStateBackupFlagValid[ i ] &&
					Sound->BufferPlayStateBackupFlag[ i ] )
				{
					SoundBuffer_Play( &Sound->Buffer[ i ], Sound->Buffer[ i ].Loop ) ;
				}

				Sound->BufferPlayStateBackupFlagValid[ i ] = FALSE ;
			}
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// ストリームサウンドの再生処理関数
extern int NS_ProcessStreamSoundMem( int SoundHandle )
{
	return ProcessStreamSoundMem_UseGParam( SoundHandle, FALSE ) ;
}

// 有効なストリームサウンドのすべて再生処理関数にかける
extern int NS_ProcessStreamSoundMemAll( void )
{
	HANDLELIST *List ;
	SOUND *Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	for( List = DSOUND.StreamSoundListFirst.Next ; List->Next != NULL ; List = List->Next )
	{
		// 再生準備が完了していなかったら何もせず次へ
		Sound = ( SOUND * )List->Data ;
		if( Sound->Stream.StartSetupCompFlag == FALSE ) continue ;

		NS_ProcessStreamSoundMem( List->Handle ) ;
	}

/*	int i , Num ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	Num = 0 ;
	for( i = 0 ; i < MAX_SOUND_NUM && Num < DSOUND.SoundNum ; i ++ )
	{
		if( DSOUND.Sound[ i ] )
		{
			if( DSOUND.Sound[ i ]->Type == DX_SOUNDTYPE_STREAMSTYLE )
				NS_ProcessStreamSoundMem( i | DX_HANDLETYPE_MASK_SOUND | ( DSOUND.Sound[ i ]->ID << DX_HANDLECHECK_ADDRESS ) ) ;
			Num ++ ;
		}
	}
*/

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// 再生中のすべての３Ｄサウンドのパラメータを更新する
extern int Refresh3DSoundParamAll()
{
	HANDLELIST *List ;
	SOUND *Sound ;
	DWORD State ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	for( List = DSOUND._3DSoundListFirst.Next ; List->Next != NULL ; List = List->Next )
	{
		Sound = ( SOUND * )List->Data ;

		// バッファーの数だけ繰り返し
		for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
		{
			// パラメータ更新予約
			Sound->Buffer[ i ].EmitterDataChangeFlag = TRUE ;

			// 再生中だったら即座に更新
			SoundBuffer_GetStatus( &Sound->Buffer[ i ], &State ) ;
			if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
			{
				SoundBuffer_Refresh3DSoundParam( &Sound->Buffer[ i ] ) ;
			}
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// 再生が終了したらサウンドハンドルを削除するサウンドの処理を行う
extern int ProcessPlayFinishDeleteSoundMemAll( void )
{
	HANDLELIST *List ;
//	SOUND *Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

LOOPSTART:

	for( List = DSOUND.PlayFinishDeleteSoundListFirst.Next ; List->Next != NULL ; List = List->Next )
	{
//		Sound = ( SOUND * )List->Data ;

		// 再生中ではなかったら削除
		if( NS_CheckSoundMem( List->Handle ) == 0 )
		{
			NS_DeleteSoundMem( List->Handle ) ;
			goto LOOPSTART ;
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// ３Ｄサウンドを再生しているサウンドハンドルに対する処理を行う
extern int ProcessPlay3DSoundMemAll( void )
{
	HANDLELIST *List ;
	SOUND *Sound ;
	int i ;
	int Valid ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

LOOPSTART:
	for( List = DSOUND.Play3DSoundListFirst.Next ; List->Next != NULL ; List = List->Next )
	{
		Sound = ( SOUND * )List->Data ;

		Valid = FALSE ;
		for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
		{
			if( Sound->Buffer[ i ].Valid == 0 )
			{
				continue ;
			}

			if( SoundBuffer_CycleProcess( &Sound->Buffer[ i ] ) == D_DS_OK )
			{
				Valid = TRUE ;
			}
		}

		if( Valid == FALSE )
		{
			Sound->AddPlay3DSoundList = FALSE ;
			SubHandleList( &Sound->Play3DSoundList ) ;
			goto LOOPSTART ;
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// LoadSoundMemBase の実処理関数
extern int LoadSoundMem2_Static(
	LOADSOUND_GPARAM *GParam,
	int SoundHandle,
	const TCHAR *WaveName1,
	const TCHAR *WaveName2,
	int ASyncThread
)
{
	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	// 一つ目のサウンドデータの追加
	if( AddStreamSoundMemToFile_UseGParam( GParam, WaveName1, 0,  SoundHandle, GParam->CreateSoundDataType, -1, ASyncThread ) == -1 ) goto ERR ;

	// ２つ目のサウンドデータの追加
	if( AddStreamSoundMemToFile_UseGParam( GParam, WaveName2, -1, SoundHandle, GParam->CreateSoundDataType, -1, ASyncThread ) == -1 ) goto ERR ;

	// 再生準備
	SetupStreamSoundMem_UseGParam( SoundHandle, ASyncThread ) ;

	// 正常終了
	return 0 ;

ERR :
	// エラー終了
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD

// LoadSoundMem2 の非同期読み込みスレッドから呼ばれる関数
static void LoadSoundMem2_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADSOUND_GPARAM *GParam ;
	int SoundHandle ;
	const TCHAR *WaveName1 ;
	const TCHAR *WaveName2 ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( LOADSOUND_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	SoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	WaveName1 = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	WaveName2 = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = LoadSoundMem2_Static( GParam, SoundHandle, WaveName1, WaveName2, TRUE ) ;
	DecASyncLoadCount( SoundHandle ) ;
	if( Result < 0 )
	{
		SubHandle( SoundHandle ) ;
	}
}

#endif // DX_NON_ASYNCLOAD

// LoadSoundMem2 のグローバル変数にアクセスしないバージョン
extern int LoadSoundMem2_UseGParam(
	LOADSOUND_GPARAM *GParam,
	const TCHAR *WaveName1,
	const TCHAR *WaveName2,
	int ASyncLoadFlag
)
{
	int SoundHandle = -1 ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	SoundHandle = _CreateSoundHandle( GParam->Create3DSoundFlag ) ;
	if( SoundHandle == -1 )
		goto ERR ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath1[ 1024 ] ;
		TCHAR FullPath2[ 1024 ] ;

		ConvertFullPathT_( WaveName1, FullPath1 ) ;
		ConvertFullPathT_( WaveName2, FullPath2 ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, SoundHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath1 ) ; 
		AddASyncLoadParamString( NULL, &Addr, FullPath2 ) ; 

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadSoundMem2_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SoundHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath1 ) ; 
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath2 ) ; 

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( SoundHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadSoundMem2_Static( GParam, SoundHandle, WaveName1, WaveName2, FALSE ) < 0 )
			goto ERR ;
	}

	// 返回句柄
	return SoundHandle ;

ERR :
	SubHandle( SoundHandle ) ;
	SoundHandle = -1 ;

	// 終了
	return -1 ;
}

// 前奏部とループ部に分かれたサウンドデータの作成
extern int NS_LoadSoundMem2( const TCHAR *WaveName1 , const TCHAR *WaveName2 )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;

	return LoadSoundMem2_UseGParam( &GParam, WaveName1, WaveName2, GetASyncLoadFlag() ) ;
}

// 主にＢＧＭを読み込むのに適した関数
extern int NS_LoadBGM( const TCHAR *WaveName )
{
	int Type = DSOUND.CreateSoundDataType, SoundHandle ;

/*
#ifdef __BCC
	if( stricmp( WaveName + lstrlen( WaveName ) - 3, _T( "wav" ) ) == 0 )
#else
	if( _stricmp( WaveName + lstrlen( WaveName ) - 3, _T( "wav" ) ) == 0 )
#endif
*/
	if( lstrcmpi( WaveName + lstrlen( WaveName ) - 3, _T( "wav" ) ) == 0 )
		NS_SetCreateSoundDataType( DX_SOUNDDATATYPE_FILE ) ;
	else
		NS_SetCreateSoundDataType( DX_SOUNDDATATYPE_MEMPRESS ) ;

	SoundHandle = NS_LoadSoundMem( WaveName, 1 ) ;
	NS_SetCreateSoundDataType( Type ) ;

	return SoundHandle ;
}

// LoadSoundMem2ByMemImage の実処理関数
static int LoadSoundMem2ByMemImage_Static( 
	LOADSOUND_GPARAM *GParam,
	int SoundHandle,
	const void *FileImageBuffer1,
	int ImageSize1,
	const void *FileImageBuffer2,
	int ImageSize2,
	int ASyncThread
)
{
	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	// 一つ目のサウンドデータの追加
	if( AddStreamSoundMemToMem_UseGParam( GParam, FileImageBuffer1, ImageSize1,  0, SoundHandle, GParam->CreateSoundDataType, -1, ASyncThread ) == -1 ) goto ERR ;

	// ２つ目のサウンドデータの追加
	if( AddStreamSoundMemToMem_UseGParam( GParam, FileImageBuffer2, ImageSize2, -1, SoundHandle, GParam->CreateSoundDataType, -1, ASyncThread ) == -1 ) goto ERR ;

	// 再生準備
	SetupStreamSoundMem_UseGParam( SoundHandle, ASyncThread ) ;

	// 正常終了
	return 0 ;

ERR :
	// エラー終了
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadSoundMem2ByMemImage の非同期読み込みスレッドから呼ばれる関数
static void LoadSoundMem2ByMemImage_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADSOUND_GPARAM *GParam ;
	int SoundHandle ;
	const void *FileImageBuffer1 ;
	int ImageSize1 ;
	const void *FileImageBuffer2 ;
	int ImageSize2 ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( LOADSOUND_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	SoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileImageBuffer1 = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	ImageSize1 = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileImageBuffer2 = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	ImageSize2 = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = LoadSoundMem2ByMemImage_Static( GParam, SoundHandle, FileImageBuffer1, ImageSize1, FileImageBuffer2, ImageSize2, TRUE ) ;
	DecASyncLoadCount( SoundHandle ) ;
	if( Result < 0 )
	{
		SubHandle( SoundHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// LoadSoundMem2ByMemImage のグローバル変数にアクセスしないバージョン
extern int LoadSoundMem2ByMemImage_UseGParam(
	LOADSOUND_GPARAM *GParam,
	const void *FileImageBuffer1,
	int ImageSize1,
	const void *FileImageBuffer2,
	int ImageSize2,
	int ASyncLoadFlag
)
{
	int SoundHandle = -1 ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	SoundHandle = _CreateSoundHandle( GParam->Create3DSoundFlag ) ;
	if( SoundHandle == -1 )
		goto ERR ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, SoundHandle ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, FileImageBuffer1 ) ; 
		AddASyncLoadParamInt( NULL, &Addr, ImageSize1 ) ; 
		AddASyncLoadParamConstVoidP( NULL, &Addr, FileImageBuffer2 ) ; 
		AddASyncLoadParamInt( NULL, &Addr, ImageSize2 ) ; 

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadSoundMem2ByMemImage_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SoundHandle ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, FileImageBuffer1 ) ; 
		AddASyncLoadParamInt( AParam->Data, &Addr, ImageSize1 ) ; 
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, FileImageBuffer2 ) ; 
		AddASyncLoadParamInt( AParam->Data, &Addr, ImageSize2 ) ; 

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( SoundHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadSoundMem2ByMemImage_Static( GParam, SoundHandle, FileImageBuffer1, ImageSize1, FileImageBuffer2, ImageSize2, FALSE ) < 0 )
			goto ERR ;
	}

	// 返回句柄
	return SoundHandle ;

ERR :
	SubHandle( SoundHandle ) ;
	SoundHandle = -1 ;

	// 終了
	return -1 ;
}

// 前奏部とループ部に分かれたサウンドデータの作成
extern int NS_LoadSoundMem2ByMemImage( const void *FileImageBuffer1, int ImageSize1, const void *FileImageBuffer2, int ImageSize2 )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;

	return LoadSoundMem2ByMemImage_UseGParam( &GParam, FileImageBuffer1, ImageSize1, FileImageBuffer2, ImageSize2, GetASyncLoadFlag() ) ;
}


	
// LOADSOUND_GPARAM のデータをセットする
extern void InitLoadSoundGParam( LOADSOUND_GPARAM *GParam )
{
	GParam->Create3DSoundFlag = DSOUND.Create3DSoundFlag ;								// 3Dサウンドを作成するかどうかのフラグ( TRUE:３Ｄサウンドを作成する  FALSE:３Ｄサウンドを作成しない )
	GParam->CreateSoundDataType = DSOUND.CreateSoundDataType ;							// 作成するサウンドデータのデータタイプ
	GParam->DisableReadSoundFunctionMask = DSOUND.DisableReadSoundFunctionMask ;		// 使用しない読み込み処理のマスク
#ifndef DX_NON_OGGVORBIS
	GParam->OggVorbisBitDepth = DSOUND.OggVorbisBitDepth ;								// ＯｇｇＶｏｒｂｉｓ使用時のビット深度(1:8bit 2:16bit)
	GParam->OggVorbisFromTheoraFile = DSOUND.OggVorbisFromTheoraFile ;					// Ogg Theora ファイル中の Vorbis データを参照するかどうかのフラグ( TRUE:Theora ファイル中の Vorbis データを参照する )
#endif
}

// LoadSoundMemBase の実処理関数
static int LoadSoundMemBase_Static(
	LOADSOUND_GPARAM *GParam,
	int SoundHandle,
	const TCHAR *WaveName,
	int BufferNum,
	int UnionHandle,
	int ASyncThread
)
{
	void *SrcBuffer = NULL ;
	int SrcSize ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	if( BufferNum > MAX_SOUNDBUFFER_NUM ) BufferNum = MAX_SOUNDBUFFER_NUM ;

	// データタイプがメモリに展開する以外の場合、ストリームデータとしてサウンドデータを作成する
	if( GParam->CreateSoundDataType != DX_SOUNDDATATYPE_MEMNOPRESS )
	{
		// ストリーム系のサウンドデータの追加
		if( AddStreamSoundMemToFile_UseGParam( GParam, WaveName, 0, SoundHandle, GParam->CreateSoundDataType, UnionHandle, ASyncThread ) == -1 )
			goto ERR ;

		// 再生準備
		SetupStreamSoundMem_UseGParam( SoundHandle, ASyncThread ) ;
	}
	else
	{
		// 丸々メモリに読み込む
		if( FileFullRead( WaveName, &SrcBuffer, &SrcSize ) < 0 )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "右記の音声ファイルのロードに失敗しました_1：%s" ) , WaveName )) ;
			goto ERR ;
		}

		// メモリから読み込む関数に渡す
		if( LoadSoundMemByMemImageBase_UseGParam( GParam, FALSE, SoundHandle, SrcBuffer, SrcSize, BufferNum, -1, FALSE, ASyncThread ) < 0 )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "右記の音声ファイルのロードに失敗しました_2：%s" ) , WaveName )) ;
			goto ERR ;
		}

		// メモリの解放
		DXFREE( SrcBuffer ) ;
		SrcBuffer = NULL ;
	}

	// 正常終了
	return 0 ;

ERR :
	if( SrcBuffer != NULL ) DXFREE( SrcBuffer ) ;

	return -1 ;
}


#ifndef DX_NON_ASYNCLOAD
// LoadSoundMemBase の非同期読み込みスレッドから呼ばれる関数
static void LoadSoundMemBase_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADSOUND_GPARAM *GParam ;
	int SoundHandle ;
	const TCHAR *WaveName ;
	int BufferNum ;
	int UnionHandle ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( LOADSOUND_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	SoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	WaveName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	BufferNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	UnionHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = LoadSoundMemBase_Static( GParam, SoundHandle, WaveName, BufferNum, UnionHandle, TRUE ) ;
	DecASyncLoadCount( SoundHandle ) ;
	if( Result < 0 )
	{
		SubHandle( SoundHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// LoadSoundMemBase のグローバル変数にアクセスしないバージョン
extern int LoadSoundMemBase_UseGParam(
	LOADSOUND_GPARAM *GParam,
	const TCHAR *WaveName,
	int BufferNum,
	int UnionHandle,
	int ASyncLoadFlag,
	int ASyncThread 
)
{
	int SoundHandle = -1 ;
	TCHAR FullPath[ 1024 ] ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	SoundHandle = _CreateSoundHandle( GParam->Create3DSoundFlag ) ;
	if( SoundHandle == -1 )
		goto ERR ;

	ConvertFullPathT_( WaveName, FullPath ) ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, SoundHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ; 
		AddASyncLoadParamInt( NULL, &Addr, BufferNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, UnionHandle ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadSoundMemBase_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SoundHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ; 
		AddASyncLoadParamInt( AParam->Data, &Addr, BufferNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, UnionHandle ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( SoundHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadSoundMemBase_Static( GParam, SoundHandle, FullPath, BufferNum, UnionHandle, ASyncThread ) < 0 )
			goto ERR ;
	}

	// 返回句柄
	return SoundHandle ;

ERR :
	SubHandle( SoundHandle ) ;
	SoundHandle = -1 ;

	// 終了
	return -1 ;
}


// サウンドデータを追加する
extern int NS_LoadSoundMemBase( const TCHAR *WaveName, int BufferNum, int UnionHandle )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;

	return LoadSoundMemBase_UseGParam( &GParam, WaveName, BufferNum, UnionHandle, GetASyncLoadFlag() ) ;
}

// サウンドデータを追加する
extern int NS_LoadSoundMem( const TCHAR *WaveName, int BufferNum, int UnionHandle )
{
	return LoadSoundMemBase( WaveName, BufferNum, UnionHandle ) ;
}

// 同時再生数指定型サウンド追加関数
extern int NS_LoadSoundMemToBufNumSitei( const TCHAR *WaveName, int BufferNum )
{
	return LoadSoundMemBase( WaveName, BufferNum, -1 ) ;
}

// サウンドをリソースから読み込む
extern int NS_LoadSoundMemByResource( const TCHAR *ResourceName, const TCHAR *ResourceType, int BufferNum )
{
	void *Image ;
	int ImageSize ;

	// リソースの情報を取得
	if( NS_GetResourceInfo( ResourceName, ResourceType, &Image, &ImageSize ) < 0 )
		return -1 ;

	// ハンドルの作成
	return NS_LoadSoundMemByMemImageBase( Image, ImageSize, BufferNum ) ;
}

// 同じサウンドデータを使用するサウンドハンドルを作成する
extern int NS_DuplicateSoundMem( int SrcSoundHandle, int BufferNum )
{
	int i ;
	int Handle = -1 ;
	HRESULT hr ;
	LPVOID write1, write2 ;
	DWORD length1, length2 ;
	SOUND *Sound, *SrcSound ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	// エラー判定
	if( SOUNDHCHK( SrcSoundHandle, SrcSound ) )
		goto ERR ;

	// サウンドタイプがストリーム風サウンドだった場合は複製できない
	if( SrcSound->Type == DX_SOUNDTYPE_STREAMSTYLE )
		goto ERR ;

	if( BufferNum > MAX_SOUNDBUFFER_NUM ) BufferNum = MAX_SOUNDBUFFER_NUM ;

	// 新しいサウンドデータの追加
	if( ( Handle = _CreateSoundHandle( SrcSound->Is3DSound ) ) == -1 )
		goto ERR ;

	Sound = ( SOUND * )HandleManageArray[ DX_HANDLETYPE_SOUND ].Handle[ Handle & DX_HANDLEINDEX_MASK ] ;

	// フォーマットを保存
	Sound->BufferFormat = SrcSound->BufferFormat ;

	// WaveData のポインタを保存
	Sound->Normal.WaveData = SrcSound->Normal.WaveData ;
	Sound->Normal.WaveSize = SrcSound->Normal.WaveSize ;

	// WaveData の参照数を増やす
	Sound->Normal.WaveDataUseCount = SrcSound->Normal.WaveDataUseCount ;
	*Sound->Normal.WaveDataUseCount = *Sound->Normal.WaveDataUseCount + 1 ;

	// ３Ｄサウンドの場合はバッファ数は１
	if( Sound->Is3DSound )
		BufferNum = 1 ;

	// DirectSoundBuffer を作成
	if( ( CreateDirectSoundBuffer( &Sound->BufferFormat, Sound->Normal.WaveSize, DX_SOUNDTYPE_NORMAL, BufferNum, Handle, SrcSoundHandle, FALSE ) ) == -1 )
	{
		DXST_ERRORLOG_ADD( _T( "複製サウンドの DirectSoundBuffer の作成に失敗しました\n" ) ) ;
		goto ERR ;
	}
	Sound->BaseFrequency = Sound->BufferFormat.nSamplesPerSec ;

	// サウンドデータをサウンドバッファにコピーする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
		// 別のサウンドバッファの複製だった場合は転送作業の必要が無いので転送しない
		// ( 正常に動作しない環境があったためとりあえずフラグは無視 )
//		if( Sound->Normal.BufferDuplicateFlag[ i ] == TRUE ) continue ;

		hr = SoundBuffer_Lock( &Sound->Buffer[ i ], 0, Sound->Normal.WaveSize,
								&write1, &length1,
								&write2, &length2 ) ;
		if( hr != D_DS_OK )
		{
			DXST_ERRORLOG_ADD( _T( "サウンドデータの転送に失敗しました_1" ) ) ;
			goto ERR ;
		}

		_MEMCPY( write1, Sound->Normal.WaveData, length1 ) ;
		if( write2 != 0 ) _MEMCPY( write2, (BYTE *)Sound->Normal.WaveData + length1, length2 ) ;

		hr = SoundBuffer_Unlock( &Sound->Buffer[ i ], write1, length1, write2, length2 ) ;
		if( hr != D_DS_OK )
		{
			DXST_ERRORLOG_ADD( _T( "サウンドデータの転送に失敗しました_2" ) ) ;
			goto ERR ;
		}
	}
	
	// 終了、サウンドデータハンドルを返す
	return Handle ;

ERR :
	if( Handle != -1 ) NS_DeleteSoundMem( Handle ) ;
	
	return -1 ;
}

// LoadSoundMemByMemImageBase の実処理関数
static int LoadSoundMemByMemImageBase_Static(
	LOADSOUND_GPARAM *GParam,
	int SoundHandle,
	const void *FileImageBuffer,
	int ImageSize,
	int BufferNum,
	int UnionHandle,
	int ASyncThread
)
{
	int i ;
	WAVEFORMATEX Format ;
	void *SoundBuffer ;
	int SoundSize ;
	HRESULT hr ;
	LPVOID write1, write2 ;
	DWORD length1, length2 ;
	SOUND * Sound ;
	SOUNDCONV ConvData ;
	STREAMDATA Stream ;

	if( DSOUND.DirectSoundObject == NULL || BufferNum > MAX_SOUNDBUFFER_NUM )
		return -1 ;

//	_MEMSET( &ConvData, 0, sizeof( ConvData ) );
	ConvData.InitializeFlag = FALSE ;
	Stream.DataPoint = NULL ;
	SoundBuffer = NULL ;

	Sound = ( SOUND * )HandleManageArray[ DX_HANDLETYPE_SOUND ].Handle[ SoundHandle & DX_HANDLEINDEX_MASK ] ;

//	// ３Ｄサウンドの場合はバッファの数は一つだけ
//	if( Sound->Is3DSound != FALSE )
//		BufferNum = 1 ;

	// データタイプがメモリに展開する以外の場合、ストリームデータとしてサウンドデータを作成する
	if( GParam->CreateSoundDataType != DX_SOUNDDATATYPE_MEMNOPRESS )
	{
STREAM_TYPE :
		if( AddStreamSoundMemToMem_UseGParam(
				GParam,
				FileImageBuffer,
				ImageSize,
				0,
				SoundHandle,
				GParam->CreateSoundDataType,
				UnionHandle,
				ASyncThread
			) == -1 )
			goto ERR ;

		// 再生準備
		SetupStreamSoundMem_UseGParam( SoundHandle, ASyncThread ) ;
	}
	else
	{
		// ＰＣＭ形式に変換
		{
			Stream.DataPoint = MemStreamOpen( FileImageBuffer, ImageSize ) ;
			Stream.ReadShred = *GetMemStreamDataShredStruct() ;
			if( SetupSoundConvert( &ConvData, &Stream, GParam->DisableReadSoundFunctionMask
#ifndef DX_NON_OGGVORBIS
								,GParam->OggVorbisBitDepth, GParam->OggVorbisFromTheoraFile
#endif
								) < 0 )
			{
				DXST_ERRORLOGFMT_ADD(( _T( "音声ファイルのＰＣＭへの変換に失敗しました" ) )) ;
				goto ERR ;
			}
			if( SoundConvertFast( &ConvData, &Format, &SoundBuffer, &SoundSize ) < 0 )
			{
				DXST_ERRORLOGFMT_ADD(( _T( "音声ファイルを格納するメモリ領域の確保に失敗しました" ) )) ;
				goto ERR ;
			}

			TerminateSoundConvert( &ConvData ) ;
			MemStreamClose( Stream.DataPoint ) ;
			Stream.DataPoint = NULL ;
		}

		// 3分以上のサウンドデータはストリーム形式で再生する
		if( SoundSize / Format.nAvgBytesPerSec > 60 * 3 )
		{
			if( SoundBuffer != NULL )
			{
				DXFREE( SoundBuffer ) ;
				SoundBuffer = NULL ;
			}

			GParam->CreateSoundDataType = DX_SOUNDDATATYPE_MEMPRESS ;
			goto STREAM_TYPE ;
		}

		// DirectSoundBuffer を作成
//		DXST_ERRORLOGFMT_ADD(( "format:%d  channel:%d  samplespersec:%d  AvgBytes:%d  blockalign:%d  BitsPerSample:%d  cbsize:%d\n",
//				Format.wFormatTag, Format.nChannels, Format.nSamplesPerSec, Format.nAvgBytesPerSec, Format.nBlockAlign, Format.wBitsPerSample, Format.cbSize )) ;
		if( ( CreateDirectSoundBuffer( &Format, SoundSize, DX_SOUNDTYPE_NORMAL, BufferNum, SoundHandle, -1, ASyncThread ) ) == -1 )
		{
			// DirectSoundBuffer の作成に失敗したらストリーム形式で開こうとしてみる
			if( SoundBuffer != NULL )
			{
				DXFREE( SoundBuffer ) ;
				SoundBuffer = NULL ;
			}

			GParam->CreateSoundDataType = DX_SOUNDDATATYPE_MEMPRESS ;
			goto STREAM_TYPE ;
		}
		Sound->BaseFrequency = Format.nSamplesPerSec ;

		// 参照数を保存するメモリの確保
		Sound->Normal.WaveDataUseCount = ( int * )DXALLOC( sizeof( int ) ) ;
		if( Sound->Normal.WaveDataUseCount == NULL )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "サウンドデータの参照数を保存するメモリ領域の確保に失敗しました" ) )) ;
			goto ERR ;
		}

		Sound->BufferFormat = Format ;			// フォーマットを保存
		Sound->Normal.WaveData = SoundBuffer ;	// データポインタを保存
		Sound->Normal.WaveSize = SoundSize ;	// データサイズを保存
		*Sound->Normal.WaveDataUseCount = 1 ;
		SoundBuffer = NULL ;

		// サウンドデータをサウンドバッファにコピーする
		{
			for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
			{
				// 別のサウンドバッファの複製だった場合は転送作業の必要が無いので転送しない
				if( Sound->Normal.BufferDuplicateFlag[ i ] == TRUE ) continue ;
			
				hr = SoundBuffer_Lock( &Sound->Buffer[ i ], 0, SoundSize,
										&write1, &length1,
										&write2, &length2 ) ;
				if( hr != D_DS_OK )
				{
					DXST_ERRORLOG_ADD( _T( "サウンドデータの転送に失敗しました_1" ) ) ;
					goto ERR ;
				}

				_MEMCPY( write1, Sound->Normal.WaveData, length1 ) ;
				if( write2 != 0 ) _MEMCPY( write2, (BYTE *)Sound->Normal.WaveData + length1, length2 ) ;

				hr = SoundBuffer_Unlock( &Sound->Buffer[ i ], write1, length1, write2, length2 ) ;
				if( hr != D_DS_OK )
				{
					DXST_ERRORLOG_ADD( _T( "サウンドデータの転送に失敗しました_2" ) ) ;
					goto ERR ;
				}
			}
		}
	}
	
	// 正常終了
	return 0 ;

ERR :
	if( SoundBuffer != NULL ) DXFREE( SoundBuffer ) ;
	if( Stream.DataPoint != NULL ) MemStreamClose( Stream.DataPoint ) ;
	TerminateSoundConvert( &ConvData ) ;
	
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadSoundMemByMemImageBase の非同期読み込みスレッドから呼ばれる関数
static void LoadSoundMemByMemImageBase_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADSOUND_GPARAM *GParam ;
	int CreateSoundHandle ;
	int SoundHandle ;
	const void *FileImageBuffer ;
	int ImageSize ;
	int BufferNum ;
	int UnionHandle ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( LOADSOUND_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	CreateSoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileImageBuffer = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	ImageSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	BufferNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	UnionHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = LoadSoundMemByMemImageBase_Static( GParam, SoundHandle, FileImageBuffer, ImageSize, BufferNum, UnionHandle, TRUE ) ;
	DecASyncLoadCount( SoundHandle ) ;
	if( Result < 0 )
	{
		if( CreateSoundHandle )
		{
			SubHandle( SoundHandle ) ;
		}
	}
}
#endif // DX_NON_ASYNCLOAD

// LoadSoundMemByMemImageBase のグローバル変数にアクセスしないバージョン
extern int LoadSoundMemByMemImageBase_UseGParam(
	LOADSOUND_GPARAM *GParam,
	int CreateSoundHandle,
	int SoundHandle,
	const void *FileImageBuffer,
	int ImageSize,
	int BufferNum,
	int UnionHandle,
	int ASyncLoadFlag,
	int ASyncThread
)
{
	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	if( CreateSoundHandle )
	{
		SoundHandle = _CreateSoundHandle( GParam->Create3DSoundFlag ) ;
		if( SoundHandle == -1 )
			goto ERR ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag && ASyncThread == FALSE )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, CreateSoundHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, SoundHandle ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, FileImageBuffer ) ; 
		AddASyncLoadParamInt( NULL, &Addr, ImageSize ) ;
		AddASyncLoadParamInt( NULL, &Addr, BufferNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, UnionHandle ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadSoundMemByMemImageBase_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, CreateSoundHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SoundHandle ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, FileImageBuffer ) ; 
		AddASyncLoadParamInt( AParam->Data, &Addr, ImageSize ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, BufferNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, UnionHandle ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( SoundHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadSoundMemByMemImageBase_Static( GParam, SoundHandle, FileImageBuffer, ImageSize, BufferNum, UnionHandle, ASyncThread ) < 0 )
			goto ERR ;
	}

	// 返回句柄
	return CreateSoundHandle ? SoundHandle : 0 ;

ERR :
	if( CreateSoundHandle )
	{
		SubHandle( SoundHandle ) ;
		SoundHandle = -1 ;
	}

	// 終了
	return -1 ;
}

// メモリ上に展開されたファイルイメージからハンドルを作成する(ベース関数)
extern int NS_LoadSoundMemByMemImageBase( const void *FileImageBuffer, int ImageSize, int BufferNum, int UnionHandle )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;

	return LoadSoundMemByMemImageBase_UseGParam( &GParam, TRUE, -1, FileImageBuffer, ImageSize, BufferNum, UnionHandle, GetASyncLoadFlag(), FALSE ) ;
}

// メモリ上に展開されたファイルイメージからハンドルを作成する
extern int NS_LoadSoundMemByMemImage( const void *FileImageBuffer , int ImageSize, int UnionHandle ) 
{
	return NS_LoadSoundMemByMemImageBase( FileImageBuffer, ImageSize, 1, UnionHandle ) ;
}

// メモリ上に展開されたファイルイメージからハンドルを作成する2
extern int NS_LoadSoundMemByMemImage2( const void *UData, int UDataSize, const WAVEFORMATEX *UFormat, int UHeaderSize )
{
	void *WaveImage ;
	int WaveImageSize ;
	int res, Handle ;
	
	res = CreateWaveFileImage( &WaveImage, &WaveImageSize,
								 UFormat, UHeaderSize,
								 UData, UDataSize ) ;
	if( res < 0 ) return -1 ;
	
	Handle = NS_LoadSoundMemByMemImageBase( WaveImage, WaveImageSize, 1 ) ;
	
	DXFREE( WaveImage ) ;
	
	return Handle ;
}

// メモリ上に展開されたファイルイメージからハンドルを作成する(ベース関数)
extern int NS_LoadSoundMemByMemImageToBufNumSitei( const void *FileImageBuffer, int ImageSize, int BufferNum )
{
	return NS_LoadSoundMemByMemImageBase( FileImageBuffer, ImageSize, BufferNum ) ;
}







// メモリに読み込んだWAVEデータを削除する
extern int NS_DeleteSoundMem( int SoundHandle, int )
{
	return SubHandle( SoundHandle ) ;
}

// サウンドハンドルの再生準備を行う( -1:エラー 0:正常終了 1:再生する必要なし )
static	int _PlaySetupSoundMem( SOUND * Sound, int TopPositionFlag )
{
	DWORD State ;
	ULONGLONG Position ;
	HRESULT hr ;
	int i ;
	int j ;
//	D_XAUDIO2_VOICE_STATE XAState ;
	LONG Volume[ SOUNDBUFFER_MAX_CHANNEL_NUM ] ;
	LONG Pan ;
	DWORD Frequency ;

	// 前回の再生位置を取得する
	{
		DWORD Temp ;

//		Sound->Buffer[ Sound->Normal.BackPlayBufferNo ]->GetCurrentPosition( &Position, NULL ) ;
		SoundBuffer_GetCurrentPosition( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], &Temp, NULL ) ;
		Position = Temp ;
	}

	// 再生するサウンドバッファのセット
	i = Sound->Normal.BackPlayBufferNo ;

	// 今回再生するサウンドバッファのセット
	i ++ ;
	if( i >= Sound->ValidBufferNum )
		i = 0 ;

	// 前回の再生位置の情報を更新する
	Sound->Normal.BackPlayBufferNo = i ;

	// 再生中だった場合は止める
	{
//		hr = Sound->Buffer[ i ]->GetStatus( &State ) ;
		hr = SoundBuffer_GetStatus( &Sound->Buffer[ i ], &State ) ;
		if( hr != D_DS_OK ) return -1 ;
		if( State & D_DSBSTATUS_PLAYING )
		{
			// 先頭からの再生指定ではない場合は何もせず終了
			if( TopPositionFlag == FALSE )
				return 1 ;

			// 再生停止
//			Sound->Buffer[ i ]->Stop() ;
			SoundBuffer_Stop( &Sound->Buffer[ i ] ) ;
		}

		// 再生位置を先頭にする
//		if( TopPositionFlag == TRUE )	Sound->Buffer[i]->SetCurrentPosition( 0 ) ;
//		else							Sound->Buffer[i]->SetCurrentPosition( Position ) ;
		if( TopPositionFlag == TRUE )	SoundBuffer_SetCurrentPosition( &Sound->Buffer[i], 0 ) ;
		else							SoundBuffer_SetCurrentPosition( &Sound->Buffer[i], ( DWORD )Position ) ;
	}

	// ボリュームをセットする
	for( j = 0 ; j < SOUNDBUFFER_MAX_CHANNEL_NUM ; j ++ )
	{
		if( Sound->ValidNextPlayVolume[ j ] )
		{
			Volume[ j ] = ( LONG )Sound->NextPlayVolume[ j ] ;
			Sound->ValidNextPlayVolume[ j ] = 0 ;
		}
		else
		{
			Volume[ j ] = ( LONG )Sound->BaseVolume[ j ] ;
		}
	}

	// パンをセットする
	if( Sound->ValidNextPlayPan )
	{
		Pan = ( LONG )Sound->NextPlayPan ;
		Sound->ValidNextPlayPan = 0 ;
	}
	else
	{
		Pan = ( LONG )Sound->BasePan ;
	}

	// 周波数をセットする
	if( Sound->ValidNextPlayFrequency )
	{
		Frequency = ( DWORD )Sound->NextPlayFrequency ;
		Sound->ValidNextPlayFrequency = 0 ;
	}
	else
	{
		Frequency = ( DWORD )Sound->BaseFrequency ;
	}

	for( j = 0 ; j < SOUNDBUFFER_MAX_CHANNEL_NUM ; j ++ )
	{
		SoundBuffer_SetVolume( &Sound->Buffer[ i ], j, Volume[ j ] ) ;
	}
	SoundBuffer_SetPan(       &Sound->Buffer[ i ], Pan ) ;
	SoundBuffer_SetFrequency( &Sound->Buffer[ i ], Frequency ) ;

	// ３Ｄサウンドの場合は３Ｄサウンドパラメータの更新を行う
	if( Sound->Is3DSound != FALSE )
	{
		VECTOR *Position ;
		float Radius ;
		VECTOR *Velocity ;

		if( Sound->ValidNextPlay3DPosition )
		{
			Position = &Sound->NextPlay3DPosition ;
			Sound->ValidNextPlay3DPosition = 0 ;
		}
		else
		{
			Position = &Sound->Base3DPosition ;
		}

		if( Sound->ValidNextPlay3DRadius )
		{
			Radius = Sound->NextPlay3DRadius ;
			Sound->ValidNextPlay3DRadius = 0 ;
		}
		else
		{
			Radius = Sound->Base3DRadius ;
		}

		if( Sound->ValidNextPlay3DVelocity )
		{
			Velocity = &Sound->NextPlay3DVelocity ;
			Sound->ValidNextPlay3DVelocity = 0 ;
		}
		else
		{
			Velocity = &Sound->Base3DVelocity ;
		}
		SoundBuffer_Set3DPosition( &Sound->Buffer[ i ], Position ) ;
		SoundBuffer_Set3DRadius(   &Sound->Buffer[ i ], Radius ) ;
		SoundBuffer_Set3DVelocity( &Sound->Buffer[ i ], Velocity ) ;

		SoundBuffer_Refresh3DSoundParam( &Sound->Buffer[ i ] ) ;
	}

	// 正常終了
	return 0 ;
}

// メモリに読みこんだWAVEデータを再生する
extern int NS_PlaySoundMem( int SoundHandle , int PlayType, int TopPositionFlag )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		return NS_PlayStreamSoundMem( SoundHandle , PlayType, TopPositionFlag ) ;
	}

	// 再生準備を行う
	if( _PlaySetupSoundMem( Sound, TopPositionFlag ) == 1 )
		return 0 ;

	// 再生
	{
//		if( Sound->Buffer[ i ]->Play( 0 , 0 , PlayType == DX_PLAYTYPE_LOOP ? D_DSBPLAY_LOOPING : 0 ) != D_DS_OK )
		if( SoundBuffer_Play( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], PlayType == DX_PLAYTYPE_LOOP ) != D_DS_OK )
			return -1 ;
	}

	// ３Ｄサウンドの場合は再生中の３Ｄサウンドリストに追加する
	if( Sound->Is3DSound != FALSE )
	{
		// クリティカルセクションの取得
		CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

		if( Sound->AddPlay3DSoundList == FALSE )
		{
			Sound->AddPlay3DSoundList = TRUE ;
			AddHandleList( &DSOUND.Play3DSoundListFirst, &Sound->Play3DSoundList, SoundHandle, Sound ) ;
		}

		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;
	}

	// 再生ステータスによっては再生終了を待つ
	if( PlayType == DX_PLAYTYPE_NORMAL )
	{
		while( NS_ProcessMessage() == 0 && NS_CheckSoundMem( SoundHandle ) == 1 ){ Sleep( 1 ) ; }
	}

	// 終了
	return 0 ;
}


// メモリに読み込んだ複数のWAVEデータをなるべく同時に再生する
extern int NS_MultiPlaySoundMem( const int *SoundHandleList, int SoundHandleNum, int PlayType, int TopPositionFlag )
{
	SOUND **SoundList, *SoundBuf[ 256 ], **SoundTempBuf = NULL, *Sound ;
	SOUNDBUFFER **SBufferList, *SBufferBuf[ 256 ], **SBufferTempBuf = NULL, *SBuffer ;
	int i, Loop ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// サウンドポインタリストが足りなかったらメモリを確保する
	if( SoundHandleNum > 256 )
	{
		SoundTempBuf = ( SOUND ** )DXALLOC( ( sizeof( SOUND * ) + sizeof( SOUNDBUFFER * ) ) * SoundHandleNum ) ;
		if( SoundTempBuf == NULL )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "%d 個のサウンドデータのポインタを格納するメモリ領域の確保に失敗しました" ) , SoundHandleNum )) ;
			goto END ;
		}
		SBufferTempBuf = ( SOUNDBUFFER ** )( SoundTempBuf + SoundHandleNum ) ;

		SBufferList = SBufferTempBuf ;
		SoundList = SoundTempBuf ;
	}
	else
	{
		SBufferList = SBufferBuf ;
		SoundList = SoundBuf ;
	}

	// ループかどうかを取得しておく
	Loop = PlayType == DX_PLAYTYPE_LOOP ;

	// 再生の準備
	for( i = 0 ; i < SoundHandleNum ; i ++ )
	{
		// エラー判定
		if( SOUNDHCHK( SoundHandleList[ i ], Sound ) )
		{
			SoundList[ i ] = NULL ;
		}
		else
		{
			SoundList[ i ] = Sound ;

			if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
			{
				_PlaySetupStreamSoundMem( SoundHandleList[ i ], Sound, PlayType, TopPositionFlag, &SBufferList[ i ] ) ;
			}
			else
			{
				_PlaySetupSoundMem( Sound, TopPositionFlag ) ;
				SBufferList[ i ] = &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ] ;
			}
		}
	}

	// 一斉に再生
	if( DSOUND.EnableSoundCaptureFlag )
	{
		for( i = 0 ; i < SoundHandleNum ; i ++ )
		{
			Sound = SoundList[ i ] ;
			if( Sound == NULL ) continue ;
			SBuffer = SBufferList[ i ] ;
			SBuffer->State = TRUE ;
			SBuffer->Loop = ( Loop || Sound->Type == DX_SOUNDTYPE_STREAMSTYLE ) ? 1 : 0 ;
		}
	}
	else
	{
		for( i = 0 ; i < SoundHandleNum ; i ++ )
		{
			Sound = SoundList[ i ] ;
			if( Sound == NULL ) continue ;
			SBuffer = SBufferList[ i ] ;
			SBuffer->DSBuffer->Play( 0, 0, ( Loop || Sound->Type == DX_SOUNDTYPE_STREAMSTYLE ) ? D_DSBPLAY_LOOPING : 0 ) ;
		}
	}

	// 再生ステータスによっては再生終了を待つ
	if( PlayType == DX_PLAYTYPE_NORMAL )
	{
		while( NS_ProcessMessage() == 0 )
		{
			for( i = 0 ; i < SoundHandleNum ; i ++ )
			{
				if( SoundList[ i ] == NULL ) continue ;
				if( NS_CheckSoundMem( SoundHandleList[ i ] ) == 1 ) break ;
			}
			if( i == SoundHandleNum ) break ;
			Sleep( 1 ) ;
		}
	}

END :

	// メモリの解放
	if( SoundTempBuf )
	{
		DXFREE( SoundTempBuf ) ;
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}


// メモリに読み込んだWAVEデータの再生を止める
extern int NS_StopSoundMem( int SoundHandle )
{
	SOUND * Sound ;
	HRESULT hr ;
	DWORD state ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		return NS_StopStreamSoundMem( SoundHandle ) ;
	}

	// 再生停止
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
		Sound->BufferPlayStateBackupFlagValid[ i ] = FALSE ;
		Sound->BufferPlayStateBackupFlag[ i ] = FALSE ;

//		hr = Sound->Buffer[ i ]->GetStatus( &state ) ;
		hr = SoundBuffer_GetStatus( &Sound->Buffer[ i ], &state ) ;
		if( hr != D_DS_OK ) return -1 ;
		if( state & D_DSBSTATUS_PLAYING  )
		{
//			Sound->Buffer[ i ]->Stop() ;
			SoundBuffer_Stop( &Sound->Buffer[ i ], TRUE ) ;
		}
	}

	// 終了
	return 0 ;
}

// メモリに読みこんだWAVEデータが再生中か調べる
extern int NS_CheckSoundMem( int SoundHandle )
{
	DWORD State ;
	SOUND * Sound ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		return NS_CheckStreamSoundMem( SoundHandle ) ;
	}
	
	// ステータスを取得して調べる
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
		// システム側で止めている場合は止める前の状態を返す
		if( Sound->BufferPlayStateBackupFlagValid[ i ] )
		{
			return Sound->BufferPlayStateBackupFlag[ i ] ? 1 : 0 ;
		}
		else
		{
//			Sound->Buffer[ i ]->GetStatus( &State ) ;
			SoundBuffer_GetStatus( &Sound->Buffer[ i ], &State ) ;
			if( State & D_DSBSTATUS_PLAYING ) return 1 ;
		}
	}

	// ここまできていれば再生はされていない
	return 0 ;
}

// メモリに読みこんだWAVEデータの再生にパンを設定する
extern int NS_SetPanSoundMem( int PanPal , int SoundHandle )
{
	int i ;
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// 再生がされていない場合は SetNextPlayPanSoundMem を呼ぶ
//	if( NS_CheckSoundMem( SoundHandle ) == 0 )
//	{
//		return NS_SetNextPlayPanSoundMem( PanPal, SoundHandle ) ;
//	}

	if( PanPal >  10000 ) PanPal =  10000 ;
	if( PanPal < -10000 ) PanPal = -10000 ;

	// パンをセットする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
//		Sound->Buffer[ i ]->SetPan( ( LONG )PanPal ) ;
		SoundBuffer_SetPan( &Sound->Buffer[ i ], ( LONG )PanPal ) ;
	}
//	SoundBuffer_SetPan( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], ( LONG )PanPal ) ;
	Sound->BasePan = PanPal ;

	// 終了
	return 0 ;
}

// サウンドハンドルのパンを設定する( 0 ～ 255 )
extern int NS_ChangePanSoundMem( int PanPal, int SoundHandle )
{
	int i ;
	int pan ;
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	if( PanPal >  255 ) PanPal =  255 ;
	if( PanPal < -255 ) PanPal = -255 ;

	if( PanPal == 0 )
	{
		pan = 0 ;
	}
	else
	{
		if( PanPal > 0 )
		{
			if( PanPal == 255 )
			{
				pan = 10000 ;
			}
			else
			{
				if( DSOUND.OldVolumeTypeFlag )
				{
					pan = -(LONG)( _DTOL( _LOG10( ( 255 - PanPal ) / 255.0 ) * 10.0 * 100.0 ) ) ;
				}
				else
				{
					pan = -(LONG)( _DTOL( _LOG10( ( 255 - PanPal ) / 255.0 ) * 50.0 * 100.0 ) ) ;
				}
				if( pan > 10000 )
				{
					pan = 10000 ;
				}
			}
		}
		else
		{
			if( PanPal == -255 )
			{
				pan = -10000 ;
			}
			else
			{
				if( DSOUND.OldVolumeTypeFlag )
				{
					pan = (LONG)( _DTOL( _LOG10( ( 255 + PanPal ) / 255.0 ) * 10.0 * 100.0 ) ) ;
				}
				else
				{
					pan = (LONG)( _DTOL( _LOG10( ( 255 + PanPal ) / 255.0 ) * 50.0 * 100.0 ) ) ;
				}
				if( pan < -10000 )
				{
					pan = -10000 ;
				}
			}
		}
	}

	// パンをセットする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
		SoundBuffer_SetPan( &Sound->Buffer[ i ], ( LONG )pan ) ;
	}

	Sound->BasePan = pan ;

	// 終了
	return 0 ;
}

// メモリに読みこんだWAVEデータのパンを取得する
extern int NS_GetPanSoundMem( int SoundHandle )
{
	SOUND * Sound ;
	LONG Result ;
	HRESULT hr ;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		// 使用権が無い場合はエラー
		if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == -1 ) return -1 ;
	
		// パンを取得する
		hr = SoundBuffer_GetPan( SBuffer, &Result ) ;
	}
	else
	{
		hr = SoundBuffer_GetPan( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], &Result ) ;
	}

	// 終了
	return Result ; 
}

// メモリに読みこんだWAVEデータの再生にボリュームを設定する
extern int NS_SetVolumeSoundMem( int VolumePal , int SoundHandle )
{
	SOUND * Sound ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// 再生がされていない場合は SetNextPlayVolumeSoundMem を呼ぶ
//	if( NS_CheckSoundMem( SoundHandle ) == 0 )
//	{
//		return NS_SetNextPlayVolumeSoundMem( VolumePal, SoundHandle ) ;
//	}

	VolumePal -= 10000 ;
//	if( VolumePal >  0 ) VolumePal =  0 ;
	if( VolumePal < -10000 ) VolumePal = -10000 ;

	// ボリュームをセットする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
//		Sound->Buffer[ i ]->SetVolume( ( LONG )VolumePal ) ;
		SoundBuffer_SetVolumeAll( &Sound->Buffer[ i ], ( LONG )VolumePal ) ;
	}
//	SoundBuffer_SetVolumeAll( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], ( LONG )VolumePal ) ;

	for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
	{
		Sound->BaseVolume[ i ] = VolumePal ;
	}

	// 終了
	return 0 ;
}

// メモリに読みこんだWAVEデータの再生にボリュームを設定する
extern int NS_ChangeVolumeSoundMem( int VolumePal, int SoundHandle )
{
	SOUND * Sound ;
	int i ;
	LONG vol ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

//	     if( VolumePal > 255 ) VolumePal = 255 ;
//	else if( VolumePal < 0   ) VolumePal = 0   ;
	if( VolumePal < 0   ) VolumePal = 0   ;

	if( VolumePal == 0 )
	{
		vol = D_DSBVOLUME_MIN ;
	}
	else
	{
		if( DSOUND.OldVolumeTypeFlag )
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 10.0 * 100.0 ) ) ;
		}
		else
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 50.0 * 100.0 ) ) ;
		}
		if( vol < D_DSBVOLUME_MIN ) vol = D_DSBVOLUME_MIN ;
	}

	// ボリュームをセットする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
//		Sound->Buffer[ i ]->SetVolume( vol ) ;
		SoundBuffer_SetVolumeAll( &Sound->Buffer[ i ], vol ) ;
	}
//	SoundBuffer_SetVolumeAll( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], vol ) ;

	for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
	{
		Sound->BaseVolume[ i ] = vol ;
	}

	// 終了
	return 0 ;
}

// メモリに読みこんだWAVEデータの再生のボリュームを取得する
extern int NS_GetVolumeSoundMem( int SoundHandle )
{
	return NS_GetChannelVolumeSoundMem( 0, SoundHandle ) ;
}


// サウンドハンドルの指定のチャンネルのボリュームを設定する( 100分の1デシベル単位 0 ～ 10000 )
extern int NS_SetChannelVolumeSoundMem( int Channel, int VolumePal, int SoundHandle )
{
	SOUND * Sound ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	VolumePal -= 10000 ;
//	if( VolumePal >  0 ) VolumePal =  0 ;
	if( VolumePal < -10000 ) VolumePal = -10000 ;

	// ボリュームをセットする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
		SoundBuffer_SetVolume( &Sound->Buffer[ i ], Channel, ( LONG )VolumePal ) ;
	}
	Sound->BaseVolume[ Channel ] = VolumePal ;

	// 終了
	return 0 ;
}

// サウンドハンドルの指定のチャンネルのボリュームを設定する( 0 ～ 255 )
extern int NS_ChangeChannelVolumeSoundMem( int Channel, int VolumePal, int SoundHandle )
{
	SOUND * Sound ;
	int i ;
	LONG vol ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

//	     if( VolumePal > 255 ) VolumePal = 255 ;
//	else if( VolumePal < 0   ) VolumePal = 0   ;
	if( VolumePal < 0   ) VolumePal = 0   ;

	if( VolumePal == 0 )
	{
		vol = D_DSBVOLUME_MIN ;
	}
	else
	{
		if( DSOUND.OldVolumeTypeFlag )
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 10.0 * 100.0 ) ) ;
		}
		else
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 50.0 * 100.0 ) ) ;
		}
		if( vol < D_DSBVOLUME_MIN ) vol = D_DSBVOLUME_MIN ;
	}

	// ボリュームをセットする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
		SoundBuffer_SetVolume( &Sound->Buffer[ i ], Channel, vol ) ;
	}

	Sound->BaseVolume[ Channel ] = vol ;

	// 終了
	return 0 ;
}

// サウンドハンドルの指定のチャンネルのボリュームを取得する
extern int NS_GetChannelVolumeSoundMem( int Channel, int SoundHandle )
{
	SOUND * Sound ;
	LONG Result ;
	SOUNDBUFFER *SBuffer ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		// 使用権が無い場合はエラー
		if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == -1 ) return -1 ;
	
		// ボリュームを取得する
		SoundBuffer_GetVolume( SBuffer, Channel, &Result ) ;
	}
	else
	{
		SoundBuffer_GetVolume( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], Channel, &Result ) ;
	}

	// 終了
	return Result + 10000 ; 
}

// メモリに読み込んだWAVEデータの再生周波数を設定する
extern int NS_SetFrequencySoundMem( int FrequencyPal , int SoundHandle )
{
	SOUND * Sound ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	if( FrequencyPal == -1 ) FrequencyPal = D_DSBFREQUENCY_ORIGINAL ;

	// 周波数をセットする
	for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
	{
//		Sound->Buffer[ i ]->SetFrequency( ( DWORD )FrequencyPal ) ;
		SoundBuffer_SetFrequency( &Sound->Buffer[ i ], ( DWORD )FrequencyPal ) ;
	}
//	SoundBuffer_SetFrequency( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], ( DWORD )FrequencyPal ) ;
	Sound->BaseFrequency = FrequencyPal ;

	// 終了
	return 0 ;
}

// メモリに読み込んだWAVEデータの再生周波数を取得する
extern int NS_GetFrequencySoundMem( int SoundHandle )
{
	SOUND * Sound ;
	SOUNDBUFFER *SBuffer ;
	HRESULT hr ;
	DWORD Result ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// 周波数を返す
//	return (int)Sound->BufferFormat.nSamplesPerSec ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( Sound->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		// 使用権が無い場合はエラー
		if( _GetDirectSoundBuffer( SoundHandle, Sound, &SBuffer, false ) == -1 ) return -1 ;
	
		// パンを取得する
		hr = SoundBuffer_GetFrequency( SBuffer, &Result ) ;
	}
	else
	{
		hr = SoundBuffer_GetFrequency( &Sound->Buffer[ Sound->Normal.BackPlayBufferNo ], &Result ) ;
	}

	// 終了
	return Result ; 
}

// メモリに読み込んだWAVEデータの再生周波数を読み込み直後の状態に戻す
extern	int NS_ResetFrequencySoundMem( int SoundHandle )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// 周波数を初期状態に戻す
	return NS_SetFrequencySoundMem( (int)Sound->BufferFormat.nSamplesPerSec, SoundHandle ) ;
}


// メモリに読みこんだWAVEデータの再生にパンを設定する
extern	int	NS_SetNextPlayPanSoundMem( int PanPal, int SoundHandle )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	if( PanPal >  10000 ) PanPal =  10000 ;
	if( PanPal < -10000 ) PanPal = -10000 ;

	Sound->NextPlayPan = PanPal ;
	Sound->ValidNextPlayPan = 1 ;

	// 終了
	return 0 ;
}

// サウンドハンドルの次の再生にのみ使用するパンを設定する( -255 ～ 255 )
extern int NS_ChangeNextPlayPanSoundMem( int PanPal, int SoundHandle )
{
	SOUND * Sound ;
	int pan ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	if( PanPal >  255 ) PanPal =  255 ;
	if( PanPal < -255 ) PanPal = -255 ;

	if( PanPal == 0 )
	{
		pan = 0 ;
	}
	else
	{
		if( PanPal > 0 )
		{
			if( PanPal == 255 )
			{
				pan = 10000 ;
			}
			else
			{
				if( DSOUND.OldVolumeTypeFlag )
				{
					pan = -(LONG)( _DTOL( _LOG10( ( 255 - PanPal ) / 255.0 ) * 10.0 * 100.0 ) ) ;
				}
				else
				{
					pan = -(LONG)( _DTOL( _LOG10( ( 255 - PanPal ) / 255.0 ) * 50.0 * 100.0 ) ) ;
				}
				if( pan > 10000 )
				{
					pan = 10000 ;
				}
			}
		}
		else
		{
			if( PanPal == -255 )
			{
				pan = -10000 ;
			}
			else
			{
				if( DSOUND.OldVolumeTypeFlag )
				{
					pan = (LONG)( _DTOL( _LOG10( ( 255 + PanPal ) / 255.0 ) * 10.0 * 100.0 ) ) ;
				}
				else
				{
					pan = (LONG)( _DTOL( _LOG10( ( 255 + PanPal ) / 255.0 ) * 50.0 * 100.0 ) ) ;
				}
				if( pan < -10000 )
				{
					pan = -10000 ;
				}
			}
		}
	}

	Sound->NextPlayPan = pan ;
	Sound->ValidNextPlayPan = 1 ;

	// 終了
	return 0 ;
}

// メモリに読みこんだWAVEデータの再生にボリュームを設定する( 100分の1デシベル単位 )
extern	int	NS_SetNextPlayVolumeSoundMem( int VolumePal, int SoundHandle )
{
	SOUND * Sound ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	VolumePal -= 10000 ;
	if( VolumePal >  0 ) VolumePal =  0 ;
	if( VolumePal < -10000 ) VolumePal = -10000 ;

	for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
	{
		Sound->NextPlayVolume[ i ] = VolumePal ;
		Sound->ValidNextPlayVolume[ i ] = 1 ;
	}

	// 終了
	return 0 ;
}

// メモリに読みこんだWAVEデータの再生にボリュームを設定する( パーセント指定 )
extern	int	NS_ChangeNextPlayVolumeSoundMem( int VolumePal, int SoundHandle )
{
	SOUND * Sound ;
	LONG vol ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	     if( VolumePal > 255 ) VolumePal = 255 ;
	else if( VolumePal < 0   ) VolumePal = 0   ;

	if( VolumePal == 0 )
	{
		vol = D_DSBVOLUME_MIN ;
	}
	else
	{
		if( DSOUND.OldVolumeTypeFlag )
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 10.0 * 100.0 ) ) ;
		}
		else
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 50.0 * 100.0 ) ) ;
		}
		if( vol < D_DSBVOLUME_MIN ) vol = D_DSBVOLUME_MIN ;
	}

	for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
	{
		Sound->NextPlayVolume[ i ] = vol ;
		Sound->ValidNextPlayVolume[ i ] = 1 ;
	}

	// 終了
	return 0 ;
}

// サウンドハンドルの次の再生にのみ使用するチャンネルのボリュームを設定する( 100分の1デシベル単位 0 ～ 10000 )
extern int NS_SetNextPlayChannelVolumeSoundMem( int Channel, int VolumePal, int SoundHandle )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	VolumePal -= 10000 ;
	if( VolumePal >  0 ) VolumePal =  0 ;
	if( VolumePal < -10000 ) VolumePal = -10000 ;

	Sound->NextPlayVolume[ Channel ] = VolumePal ;
	Sound->ValidNextPlayVolume[ Channel ] = 1 ;

	// 終了
	return 0 ;
}

// サウンドハンドルの次の再生にのみ使用するチャンネルのボリュームを設定する( 0 ～ 255 )
extern int NS_ChangeNextPlayChannelVolumeSoundMem( int Channel, int VolumePal, int SoundHandle )
{
	SOUND * Sound ;
	LONG vol ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	     if( VolumePal > 255 ) VolumePal = 255 ;
	else if( VolumePal < 0   ) VolumePal = 0   ;

	if( VolumePal == 0 )
	{
		vol = D_DSBVOLUME_MIN ;
	}
	else
	{
		if( DSOUND.OldVolumeTypeFlag )
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 10.0 * 100.0 ) ) ;
		}
		else
		{
			vol = (LONG)( _DTOL( _LOG10( VolumePal / 255.0 ) * 50.0 * 100.0 ) ) ;
		}
		if( vol < D_DSBVOLUME_MIN ) vol = D_DSBVOLUME_MIN ;
	}

	Sound->NextPlayVolume[ Channel ] = vol ;
	Sound->ValidNextPlayVolume[ Channel ] = 1 ;

	// 終了
	return 0 ;
}

// メモリに読み込んだWAVEデータの再生周波数を設定する
extern	int	NS_SetNextPlayFrequencySoundMem( int FrequencyPal, int SoundHandle )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	if( FrequencyPal == -1 ) FrequencyPal = D_DSBFREQUENCY_ORIGINAL ;

	Sound->NextPlayFrequency = FrequencyPal ;
	Sound->ValidNextPlayFrequency = 1 ;

	// 終了
	return 0 ;
}


// サウンドハンドルの再生位置をサンプル単位で変更する(再生が止まっている時のみ有効)
extern int NS_SetCurrentPositionSoundMem( int SamplePosition, int SoundHandle )
{
	SOUND * sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// 変更
	return NS_SetSoundCurrentPosition( sd->BufferFormat.nBlockAlign * SamplePosition, SoundHandle ) ;
}

// サウンドハンドルの再生位置をサンプル単位で取得する
extern int NS_GetCurrentPositionSoundMem( int SoundHandle )
{
	SOUND * sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// 取得
	return NS_GetSoundCurrentPosition( SoundHandle ) / sd->BufferFormat.nBlockAlign ;
}

// サウンドハンドルの再生位置をバイト単位で変更する(再生が止まっている時のみ有効)
extern int NS_SetSoundCurrentPosition( int Byte, int SoundHandle )
{
	SOUND * sd ;
//	int i ;
//	DWORD State ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( sd->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		return NS_SetStreamSoundCurrentPosition( Byte, SoundHandle ) ;
	}

	// 再生位置を変更する
//	sd->Buffer[ i ]->SetCurrentPosition( Byte ) ;
//	SoundBuffer_SetCurrentPosition( &sd->Buffer[ i ], Byte ) ;
	SoundBuffer_SetCurrentPosition( &sd->Buffer[ sd->Normal.BackPlayBufferNo ], Byte ) ;

	// 終了
	return 0 ;
}

// サウンドハンドルの再生位置をバイト単位で取得する
extern int NS_GetSoundCurrentPosition( int SoundHandle )
{
	SOUND * sd ;
	DWORD pos ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( sd->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		return NS_GetStreamSoundCurrentPosition( SoundHandle ) ;
	}

	// 再生中のバッファの再生位置を返す
//	sd->Buffer[ Sound->Normal.BackPlayBufferNo ]->GetCurrentPosition( &pos, NULL ) ;
	SoundBuffer_GetCurrentPosition( &sd->Buffer[ sd->Normal.BackPlayBufferNo ], &pos, NULL ) ;

	return (int)pos ;
}

// サウンドハンドルの再生位置をミリ秒単位で設定する(圧縮形式の場合は正しく設定されない場合がある)
extern int NS_SetSoundCurrentTime( int Time, int SoundHandle )
{
	SOUND * sd ;
	int /*i,*/ time ;
//	DWORD State ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( sd->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		return NS_SetStreamSoundCurrentTime( Time, SoundHandle ) ;
	}

	// 再生位置を変更する
//	time = _DTOL( ( (double)sd->BufferFormat.nSamplesPerSec * Time / 1000 ) * sd->BufferFormat.nBlockAlign ) ;
//	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	time = MilliSecPositionToSamplePosition( sd->BufferFormat.nSamplesPerSec, Time ) * sd->BufferFormat.nBlockAlign ;

//	sd->Buffer[ i ]->SetCurrentPosition( time ) ;
//	SoundBuffer_SetCurrentPosition( &sd->Buffer[ i ], time ) ;
	SoundBuffer_SetCurrentPosition( &sd->Buffer[ sd->Normal.BackPlayBufferNo ], time ) ;

	// 終了
	return 0 ;
}

// サウンドハンドルの音の総時間を取得する(単位はサンプル)
extern int NS_GetSoundTotalSample( int SoundHandle )
{
	SOUND * sd ;
	int sample = -1 ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// サウンドタイプによって処理を分岐
	switch( sd->Type )
	{
	case DX_SOUNDTYPE_STREAMSTYLE : // ストリームタイプの場合
		sample = sd->Stream.TotalSample ;
		break ;
		
	case DX_SOUNDTYPE_NORMAL :		// ノーマルタイプの場合
		sample = sd->Normal.WaveSize / sd->BufferFormat.nBlockAlign ;
		break ;
	}
	
	// 終了
	return sample ;
}

// サウンドハンドルの音の総時間を取得する(単位はミリ秒)
extern int NS_GetSoundTotalTime( int SoundHandle )
{
	SOUND * sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;
	
	// ミリ秒に変換して返す
	return SamplePositionToMilliSecPosition( sd->BufferFormat.nSamplesPerSec, NS_GetSoundTotalSample( SoundHandle ) ) ;
}

// サウンドハンドルの再生位置をミリ秒単位で取得する(圧縮形式の場合は正しい値が返ってこない場合がある)
extern int NS_GetSoundCurrentTime( int SoundHandle )
{
	SOUND * sd ;
	DWORD time ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// サウンドタイプがストリーム風サウンドだった場合はそっちに処理を移す
	if( sd->Type == DX_SOUNDTYPE_STREAMSTYLE )
	{
		return NS_GetStreamSoundCurrentTime( SoundHandle ) ;
	}

	// サウンドバッファ０の再生位置を取得する
//	sd->Buffer[ 0 ]->GetCurrentPosition( &time, NULL ) ;
	SoundBuffer_GetCurrentPosition( &sd->Buffer[ sd->Normal.BackPlayBufferNo ], &time, NULL ) ;
//	return _DTOL( (double)time * 1000 / ( sd->BufferFormat.nSamplesPerSec * sd->BufferFormat.nBlockAlign ) ) ;
	return SamplePositionToMilliSecPosition( sd->BufferFormat.nSamplesPerSec, time / sd->BufferFormat.nBlockAlign ) ;
}


// サウンドハンドルにループ位置を設定する
extern int NS_SetLoopPosSoundMem( int LoopTime, int SoundHandle )
{
	SOUND * sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ループサンプル位置をセット
	NS_SetLoopSamplePosSoundMem( LoopTime == -1 ? -1 : MilliSecPositionToSamplePosition( sd->BufferFormat.nSamplesPerSec, LoopTime ), SoundHandle ) ;

	// 終了
	return 0 ;
}

// サウンドハンドルにループ位置を設定する
extern int NS_SetLoopTimePosSoundMem( int LoopTime, int SoundHandle )
{
	SOUND * sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;
	
	// ループサンプル位置をセット
	NS_SetLoopSamplePosSoundMem( LoopTime == -1 ? -1 : MilliSecPositionToSamplePosition( sd->BufferFormat.nSamplesPerSec, LoopTime ), SoundHandle ) ;

	// 終了
	return 0 ;
}

// サウンドハンドルにループ位置を設定する
extern int NS_SetLoopSamplePosSoundMem( int LoopSamplePosition, int SoundHandle )
{
	SOUND * sd ;
	STREAMFILEDATA * pl ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		goto ERR ;

	// 指定の位置が音声ファイル全体よりも長い場合はエラー
	if( NS_GetSoundTotalSample( SoundHandle ) <= LoopSamplePosition )
		goto ERR ;

	// ループ位置が -1 だった場合はループ位置設定を解除する
	if( LoopSamplePosition == -1 )
	{
		pl = &sd->Stream.File[0] ;
		pl->LoopSamplePosition = 0 ;
		pl->LoopNum = 0 ;
		goto END ;
	}

	// サウンドハンドルがストリーム再生ではなかったらストリーム再生のハンドルに変更する
	if( sd->Type == DX_SOUNDTYPE_NORMAL )
	{
		if( SoundTypeChangeToStream( SoundHandle ) < 0 )
			return -1 ;

		// エラー判定
		if( SOUNDHCHK( SoundHandle, sd ) )
			return -1 ;
	}

	// 今の所ストリームデータが２つ以上あったら無理
	if( sd->Stream.FileNum > 1 )
	{
		DXST_ERRORLOG_ADD( _T( "二つ以上の音声ファイルから作成されたサウンドハンドルにループポイントを指定する事は出来ません\n" ) ) ;
		goto ERR ;
	}

	pl = &sd->Stream.File[0] ;

/*	// 無圧縮 wave か OGG 以外は無理
	if( pl->DataType != DX_SOUNDDATATYPE_MEMNOPRESS &&
#ifndef DX_NON_OGGVORBIS
		pl->OggFlag == FALSE && 
#endif
		( pl->DataType == DX_SOUNDDATATYPE_MEMPRESS ||
			( pl->DataType == DX_SOUNDDATATYPE_FILE && pl->FileData.FormatMatchFlag == FALSE ) ) )
	{
		DXST_ERRORLOG_ADD( _T( "無圧縮ＷＡＶＥファイル若しくはＯＧＧファイル以外はループポイントを指定する事は出来ません\n" ) ) ;

		
		
		return -1 ;
	}
*/
	// ループ位置を設定
	pl->LoopSamplePosition = LoopSamplePosition ;
	SetupSeekPosSoundConvert( &pl->ConvData, LoopSamplePosition ) ;

	// 無限ループに設定
	pl->LoopNum = -1 ;

END :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;

ERR :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー終了
	return -1 ;
}

// サウンドハンドルにループ開始位置を設定する
extern int NS_SetLoopStartTimePosSoundMem( int LoopStartTime, int SoundHandle )
{
	SOUND * sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ループサンプル位置をセット
	NS_SetLoopStartSamplePosSoundMem( LoopStartTime == -1 ? -1 : MilliSecPositionToSamplePosition( sd->BufferFormat.nSamplesPerSec, LoopStartTime ), SoundHandle ) ;

	// 終了
	return 0 ;
}

// サウンドハンドルにループ開始位置を設定する
extern int NS_SetLoopStartSamplePosSoundMem( int LoopStartSamplePosition, int SoundHandle )
{
	SOUND * sd ;
	STREAMFILEDATA * pl ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		goto ERR ;

	// 指定の位置が音声ファイル全体よりも長い場合はエラー
	if( NS_GetSoundTotalSample( SoundHandle ) <= LoopStartSamplePosition )
		goto ERR ;

	// ループ位置が -1 だった場合はループ位置設定を解除する
	if( LoopStartSamplePosition == -1 )
	{
		pl = &sd->Stream.File[0] ;
		pl->LoopStartSamplePosition = -1 ;
		goto END ;
	}

	// サウンドハンドルがストリーム再生ではなかったらストリーム再生のハンドルに変更する
	if( sd->Type == DX_SOUNDTYPE_NORMAL )
	{
		if( SoundTypeChangeToStream( SoundHandle ) < 0 )
			return -1 ;

		// エラー判定
		if( SOUNDHCHK( SoundHandle, sd ) )
			return -1 ;
	}

	// 今の所ストリームデータが２つ以上あったら無理
	if( sd->Stream.FileNum > 1 )
	{
		DXST_ERRORLOG_ADD( _T( "二つ以上の音声ファイルから作成されたサウンドハンドルにループ開始位置を指定する事は出来ません\n" ) ) ;
		goto ERR ;
	}

	pl = &sd->Stream.File[0] ;

	// ループ位置を設定
	pl->LoopStartSamplePosition = LoopStartSamplePosition ;

	// 転送完了位置が既にループ開始位置を越えてしまっていたらストリームサウンドデータのセットアップをやり直す
	if( sd->Stream.FileCompCopyLength > pl->LoopStartSamplePosition )
	{
		RefreshStreamSoundPlayCompLength( SoundHandle ) ;
		NS_SetStreamSoundCurrentPosition( sd->Stream.CompPlayWaveLength, SoundHandle ) ;
	}

END :
	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;

ERR :
	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// エラー終了
	return -1 ;
}

// サウンドハンドルの再生が終了したら自動的にハンドルを削除するかどうかを設定する
extern int NS_SetPlayFinishDeleteSoundMem( int DeleteFlag, int SoundHandle )
{
	SOUND *sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// 今までとフラグが同じ場合は何もしない
	if( sd->PlayFinishDeleteFlag == DeleteFlag )
	{
		return 0 ;
	}

	// フラグを保存
	sd->PlayFinishDeleteFlag = DeleteFlag ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// フラグが立てられる場合はリストに追加する、倒される場合はリストから外す
	if( DeleteFlag )
	{
		AddHandleList( &DSOUND.PlayFinishDeleteSoundListFirst, &sd->PlayFinishDeleteSoundList, SoundHandle, sd ) ;
	}
	else
	{
		SubHandleList( &sd->PlayFinishDeleteSoundList ) ;
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 正常終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用のリバーブパラメータを設定する
extern int NS_Set3DReverbParamSoundMem( const SOUND3D_REVERB_PARAM *Param, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	// パラメータを保存
	sd->PresetReverbParam = -1 ;
	_MEMCPY( &sd->ReverbParam, Param, sizeof( sd->ReverbParam ) ) ;

	// バッファの数だけ繰り返し
	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_SetReverbParam( &sd->Buffer[ i ], &sd->ReverbParam ) ;
	}

	// 終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用のリバーブパラメータをプリセットを使用して設定する
extern int NS_Set3DPresetReverbParamSoundMem( int PresetNo /* DX_REVERB_PRESET_DEFAULT 等 */ , int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;
	if( PresetNo < 0 || PresetNo >= DX_REVERB_PRESET_NUM )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	// パラメータを保存
	sd->PresetReverbParam = PresetNo ;

	// バッファの数だけ繰り返し
	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_SetPresetReverbParam( &sd->Buffer[ i ], PresetNo ) ;
	}

	// 終了
	return 0 ;
}

// 全ての３Ｄサウンドのサウンドハンドルにリバーブパラメータを設定する
extern int NS_Set3DReverbParamSoundMemAll( const SOUND3D_REVERB_PARAM *Param, int PlaySoundOnly )
{
	HANDLELIST *List ;
	SOUND *Sound ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	if( PlaySoundOnly )
	{
		List = DSOUND.Play3DSoundListFirst.Next ;
	}
	else
	{
		List = DSOUND._3DSoundListFirst.Next ;
	}
	for( ; List->Next != NULL ; List = List->Next )
	{
		Sound = ( SOUND * )List->Data ;

		// パラメータを保存
		Sound->PresetReverbParam = -1 ;
		_MEMCPY( &Sound->ReverbParam, Param, sizeof( Sound->ReverbParam ) ) ;

		// バッファの数だけ繰り返し
		for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
		{
			// パラメータの更新
			SoundBuffer_SetReverbParam( &Sound->Buffer[ i ], &Sound->ReverbParam ) ;
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// 全ての３Ｄサウンドのサウンドハンドルにリバーブパラメータをプリセットを使用して設定する
extern int NS_Set3DPresetReverbParamSoundMemAll( int PresetNo /* DX_REVERB_PRESET_DEFAULT 等 */, int PlaySoundOnly )
{
	HANDLELIST *List ;
	SOUND *Sound ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	if( PresetNo < 0 || PresetNo >= DX_REVERB_PRESET_NUM )
		return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	if( PlaySoundOnly )
	{
		List = DSOUND.Play3DSoundListFirst.Next ;
	}
	else
	{
		List = DSOUND._3DSoundListFirst.Next ;
	}
	for( ; List->Next != NULL ; List = List->Next )
	{
		Sound = ( SOUND * )List->Data ;

		// パラメータを保存
		Sound->PresetReverbParam = PresetNo ;

		// バッファの数だけ繰り返し
		for( i = 0 ; i < Sound->ValidBufferNum ; i ++ )
		{
			// パラメータの更新
			SoundBuffer_SetPresetReverbParam( &Sound->Buffer[ i ], PresetNo ) ;
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// サウンドハンドルに設定されている３Ｄサウンド用のリバーブパラメータを取得する
extern int NS_Get3DReverbParamSoundMem( SOUND3D_REVERB_PARAM *ParamBuffer, int SoundHandle )
{
	SOUND *sd ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	// パラメータをコピー
	if( ParamBuffer != NULL ) *ParamBuffer = sd->ReverbParam ;

	// 正常終了
	return 0 ;
}

// プリセットの３Ｄサウンド用のリバーブパラメータを取得する
extern int NS_Get3DPresetReverbParamSoundMem( SOUND3D_REVERB_PARAM *ParamBuffer, int PresetNo /* DX_REVERB_PRESET_DEFAULT 等 */ )
{
	// エラー判定
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	if( DSOUND.DisableXAudioFlag )
		return -1 ;

	if( PresetNo < 0 || PresetNo >= DX_REVERB_PRESET_NUM )
		return -1 ;

	// パラメータをコピー
	if( ParamBuffer != NULL )
	{
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			D_XAUDIO2FX_REVERB_PARAMETERS2_8 *Param2_8 ;

			Param2_8 = &DSOUND.XAudio2_8ReverbParameters[ PresetNo ] ;

			ParamBuffer->WetDryMix           = Param2_8->WetDryMix ;

			ParamBuffer->ReflectionsDelay    = Param2_8->ReflectionsDelay;
			ParamBuffer->ReverbDelay         = Param2_8->ReverbDelay ;
			ParamBuffer->RearDelay           = Param2_8->RearDelay ;

			ParamBuffer->PositionLeft        = Param2_8->PositionLeft ;
			ParamBuffer->PositionRight       = Param2_8->PositionRight ;
			ParamBuffer->PositionMatrixLeft  = Param2_8->PositionMatrixLeft ;
			ParamBuffer->PositionMatrixRight = Param2_8->PositionMatrixRight ;
			ParamBuffer->EarlyDiffusion      = Param2_8->EarlyDiffusion ;
			ParamBuffer->LateDiffusion       = Param2_8->LateDiffusion ;
			ParamBuffer->LowEQGain           = Param2_8->LowEQGain ;
			ParamBuffer->LowEQCutoff         = Param2_8->LowEQCutoff ;
			ParamBuffer->HighEQGain          = Param2_8->HighEQGain ;
			ParamBuffer->HighEQCutoff        = Param2_8->HighEQCutoff ;

			ParamBuffer->RoomFilterFreq      = Param2_8->RoomFilterFreq ;
			ParamBuffer->RoomFilterMain      = Param2_8->RoomFilterMain ;
			ParamBuffer->RoomFilterHF        = Param2_8->RoomFilterHF ;
			ParamBuffer->ReflectionsGain     = Param2_8->ReflectionsGain ;
			ParamBuffer->ReverbGain          = Param2_8->ReverbGain ;
			ParamBuffer->DecayTime           = Param2_8->DecayTime ;
			ParamBuffer->Density             = Param2_8->Density ;
			ParamBuffer->RoomSize            = Param2_8->RoomSize ;
		}
		else
		{
			D_XAUDIO2FX_REVERB_PARAMETERS *Param ;

			Param = &DSOUND.XAudio2ReverbParameters[ PresetNo ] ;

			ParamBuffer->WetDryMix           = Param->WetDryMix ;

			ParamBuffer->ReflectionsDelay    = Param->ReflectionsDelay;
			ParamBuffer->ReverbDelay         = Param->ReverbDelay ;
			ParamBuffer->RearDelay           = Param->RearDelay ;

			ParamBuffer->PositionLeft        = Param->PositionLeft ;
			ParamBuffer->PositionRight       = Param->PositionRight ;
			ParamBuffer->PositionMatrixLeft  = Param->PositionMatrixLeft ;
			ParamBuffer->PositionMatrixRight = Param->PositionMatrixRight ;
			ParamBuffer->EarlyDiffusion      = Param->EarlyDiffusion ;
			ParamBuffer->LateDiffusion       = Param->LateDiffusion ;
			ParamBuffer->LowEQGain           = Param->LowEQGain ;
			ParamBuffer->LowEQCutoff         = Param->LowEQCutoff ;
			ParamBuffer->HighEQGain          = Param->HighEQGain ;
			ParamBuffer->HighEQCutoff        = Param->HighEQCutoff ;

			ParamBuffer->RoomFilterFreq      = Param->RoomFilterFreq ;
			ParamBuffer->RoomFilterMain      = Param->RoomFilterMain ;
			ParamBuffer->RoomFilterHF        = Param->RoomFilterHF ;
			ParamBuffer->ReflectionsGain     = Param->ReflectionsGain ;
			ParamBuffer->ReverbGain          = Param->ReverbGain ;
			ParamBuffer->DecayTime           = Param->DecayTime ;
			ParamBuffer->Density             = Param->Density ;
			ParamBuffer->RoomSize            = Param->RoomSize ;
		}
	}

	// 正常終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用の再生位置を設定する
extern int NS_Set3DPositionSoundMem( VECTOR Position, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	// 1.0f = １メートルの単位に変換する
	VectorScale( &Position, &Position, 1.0f / DSOUND._3DSoundOneMetre ) ;

	// バッファの数だけ繰り返し
	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_Set3DPosition( &sd->Buffer[ i ], &Position ) ;
	}
	sd->Base3DPosition = Position ;

	// 終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用の音が聞こえる距離を設定する
extern int NS_Set3DRadiusSoundMem( float Radius, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	Radius /= DSOUND._3DSoundOneMetre ;

	// バッファの数だけ繰り返し
	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_Set3DRadius( &sd->Buffer[ i ], Radius ) ;
	}
	sd->Base3DRadius = Radius ;

	// 終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用の音量１００％で聞こえる距離を設定する
extern int NS_Set3DInnerRadiusSoundMem( float Radius, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	Radius /= DSOUND._3DSoundOneMetre ;

	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_Set3DInnerRadius( &sd->Buffer[ i ], Radius ) ;
	}

	// 終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用の移動速度を設定する
extern int NS_Set3DVelocitySoundMem( VECTOR Velocity, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	// 1.0f = １メートルの単位に変換する
	VectorScale( &Velocity, &Velocity, 1.0f / DSOUND._3DSoundOneMetre ) ;

	// バッファの数だけ繰り返し
	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_Set3DVelocity( &sd->Buffer[ i ], &Velocity ) ;
	}
	sd->Base3DVelocity = Velocity ;

	// 終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用の方向を決定するための前方座標と上方向を設定する
extern int NS_Set3DFrontPositionSoundMem( VECTOR FrontPosition, VECTOR UpVector, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	// バッファの数だけ繰り返し
	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_Set3DFrontPosition( &sd->Buffer[ i ], &FrontPosition, &UpVector ) ;
	}

	// 終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用の方向を決定するための前方座標を設定する( 上方向はY軸 )
extern int NS_Set3DFrontPosition_UpVecYSoundMem( VECTOR FrontPosition, int SoundHandle )
{
	VECTOR UpVec = { 0.0f, 1.0f, 0.0f } ;

	return NS_Set3DFrontPositionSoundMem( FrontPosition, UpVec, SoundHandle ) ;
}

// サウンドハンドルの３Ｄサウンド用の方向の角度範囲を設定する
extern int NS_Set3DConeAngleSoundMem( float InnerAngle, float OuterAngle, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	if( OuterAngle < 0.0f )
	{
		OuterAngle = 0.0f ;
	}
	else
	if( OuterAngle > DX_PI_F * 2.0f )
	{
		OuterAngle = DX_PI_F * 2.0f ;
	}

	if( InnerAngle < 0.0f )
	{
		InnerAngle = 0.0f ;
	}
	else
	if( InnerAngle > DX_PI_F * 2.0f )
	{
		InnerAngle = DX_PI_F * 2.0f ;
	}

	if( OuterAngle < InnerAngle )
	{
		float Temp ;
		Temp = InnerAngle ;
		InnerAngle = OuterAngle ;
		OuterAngle = Temp ;
	}

	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_Set3DConeAngle( &sd->Buffer[ i ], InnerAngle, OuterAngle ) ;
	}

	// 終了
	return 0 ;
}

// サウンドハンドルの３Ｄサウンド用の方向の角度範囲の音量倍率を設定する
extern int NS_Set3DConeVolumeSoundMem( float InnerAngleVolume, float OuterAngleVolume, int SoundHandle )
{
	SOUND *sd ;
	int i ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( sd->Is3DSound == FALSE )
		return -1 ;

	// バッファーの数だけ繰り返し
	for( i = 0 ; i < sd->ValidBufferNum ; i ++ )
	{
		// パラメータの更新
		SoundBuffer_Set3DConeVolume( &sd->Buffer[ i ], InnerAngleVolume, OuterAngleVolume ) ;
	}

	// 終了
	return 0 ;
}

// サウンドハンドルの次の再生のみに使用する３Ｄサウンド用の再生位置を設定する
extern int NS_SetNextPlay3DPositionSoundMem( VECTOR Position, int SoundHandle )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( Sound->Is3DSound == FALSE )
		return -1 ;

	// 1.0f = １メートルの単位に変換する
	VectorScale( &Position, &Position, 1.0f / DSOUND._3DSoundOneMetre ) ;

	Sound->NextPlay3DPosition = Position ;
	Sound->ValidNextPlay3DPosition = 1 ;

	// 終了
	return 0 ;
}

// サウンドハンドルの次の再生のみに使用する３Ｄサウンド用の音が聞こえる距離を設定する
extern int NS_SetNextPlay3DRadiusSoundMem( float Radius, int SoundHandle )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( Sound->Is3DSound == FALSE )
		return -1 ;

	Radius /= DSOUND._3DSoundOneMetre ;

	Sound->NextPlay3DRadius = Radius ;
	Sound->ValidNextPlay3DRadius = 1 ;

	// 終了
	return 0 ;
}

// サウンドハンドルの次の再生のみに使用する３Ｄサウンド用の移動速度を設定する
extern int NS_SetNextPlay3DVelocitySoundMem( VECTOR Velocity, int SoundHandle )
{
	SOUND * Sound ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, Sound ) )
		return -1 ;

	// ３Ｄサウンドではない場合は何もしない
	if( Sound->Is3DSound == FALSE )
		return -1 ;

	// 1.0f = １メートルの単位に変換する
	VectorScale( &Velocity, &Velocity, 1.0f / DSOUND._3DSoundOneMetre ) ;

	Sound->NextPlay3DVelocity = Velocity ;
	Sound->ValidNextPlay3DVelocity = 1 ;

	// 終了
	return 0 ;
}










// 設定関係関数

// 作成するサウンドのデータ形式を設定する
extern int NS_SetCreateSoundDataType( int SoundDataType )
{
	// 値が範囲外のデータ形式かどうか調べる
	if( SoundDataType >= DX_SOUNDDATATYPE_MEMNOPRESS && SoundDataType <= DX_SOUNDDATATYPE_FILE )
	{
		// 現在 DX_SOUNDDATATYPE_MEMNOPRESS_PLUS は非対応
		if( SoundDataType == DX_SOUNDDATATYPE_MEMNOPRESS_PLUS )
		{
			DSOUND.CreateSoundDataType = DX_SOUNDDATATYPE_MEMNOPRESS ;
		}
		else
		{
			DSOUND.CreateSoundDataType = SoundDataType ;
		}
	}
	else
	{
		return -1 ; 
	}

	// 終了
	return 0 ;
}

// 作成するサウンドのデータ形式を取得する( DX_SOUNDDATATYPE_MEMNOPRESS 等 )
extern	int NS_GetCreateSoundDataType( void )
{
	return DSOUND.CreateSoundDataType ;
}

// 使用しないサウンドデータ読み込み処理のマスクを設定する
extern	int NS_SetDisableReadSoundFunctionMask( int Mask )
{
	DSOUND.DisableReadSoundFunctionMask = Mask ;

	// 終了
	return 0 ;
}

// 使用しないサウンドデータ読み込み処理のマスクを取得する
extern	int	NS_GetDisableReadSoundFunctionMask( void )
{
	return DSOUND.DisableReadSoundFunctionMask ;
}

// サウンドキャプチャを前提とした動作をするかどうかを設定する
extern	int NS_SetEnableSoundCaptureFlag( int Flag )
{
	// フラグが同じ場合は何もしない
	if( DSOUND.EnableSoundCaptureFlag == Flag ) return 0 ;

	// 全てのサウンドハンドルを削除する
	NS_InitSoundMem() ;
	
	// フラグをセットする
	DSOUND.EnableSoundCaptureFlag = Flag ;
	
	// 終了
	return 0 ;
}

// サウンドの処理をソフトウエアで行うかどうかを設定する( TRUE:ソフトウエア  FALSE:ハードウエア( デフォルト ) )
extern int NS_SetUseSoftwareMixingSoundFlag( int Flag )
{
	// フラグを保存
	DSOUND.UseSoftwareMixing = Flag ;

	// 終了
	return 0 ;
}

// サウンドの再生にXAudioを使用するかどうかを設定する( TRUE:使用する( デフォルト )  FALSE:使用しない )
extern int NS_SetEnableXAudioFlag( int Flag )
{
	// 初期化済みの場合はエラー
	if( DSOUND.InitializeFlag != FALSE )
		return -1 ;

	// フラグを保存する
	DSOUND.DisableXAudioFlag = Flag ? FALSE : TRUE ;
	
	// 終了
	return 0 ;
}

// ChangeVolumeSoundMem, ChangeNextPlayVolumeSoundMem, ChangeMovieVolumeToGraph の音量計算式を Ver3.10c以前のものを使用するかどうかを設定する( TRUE:Ver3.10c以前の計算式を使用  FALSE:3.10d以降の計算式を使用( デフォルト ) )
extern int NS_SetUseOldVolumeCalcFlag( int Flag )
{
	// フラグを保存する
	DSOUND.OldVolumeTypeFlag = Flag ;
	
	// 終了
	return 0 ;
}

// 次に作成するサウンドを３Ｄサウンド用にするかどうかを設定する( TRUE:３Ｄサウンド用にする  FALSE:３Ｄサウンド用にしない( デフォルト ) )
extern int NS_SetCreate3DSoundFlag( int Flag )
{
	// フラグを保存する
	DSOUND.Create3DSoundFlag = Flag != FALSE ? TRUE : FALSE ;

	// 終了
	return 0 ;
}

// ３Ｄ空間の１メートルに当る距離を設定する( デフォルト:1.0f )
extern int NS_Set3DSoundOneMetre( float Distance )
{
	if( DSOUND.DirectSoundObject != NULL ) return -1 ;

	if( Distance <= 0.0f )
	{
		DSOUND._3DSoundOneMetreEnable = FALSE ;
		DSOUND._3DSoundOneMetre = 1.0f ;
	}
	else
	{
		DSOUND._3DSoundOneMetreEnable = TRUE ;
		DSOUND._3DSoundOneMetre = Distance ;
	}

	// 終了
	return 0 ;
}

// ３Ｄサウンドのリスナーの位置とリスナーの前方位置を設定する( リスナーの上方向位置はＹ軸固定 )
extern int NS_Set3DSoundListenerPosAndFrontPos_UpVecY( VECTOR Position, VECTOR FrontPosition )
{
	VECTOR UpVec = { 0.0f, 1.0f, 0.0f } ;

	return NS_Set3DSoundListenerPosAndFrontPos( Position, FrontPosition, UpVec ) ;
}

// ３Ｄサウンドのリスナーの位置とリスナーの前方位置とリスナーの上方向位置を設定する
extern int NS_Set3DSoundListenerPosAndFrontPos( VECTOR Position, VECTOR FrontPosition, VECTOR UpVector )
{
	VECTOR SideVec ;
	VECTOR DirVec ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	VectorSub( &DirVec, &FrontPosition, &Position ) ;
	VectorNormalize( &DirVec, &DirVec ) ;

	VectorScale( &Position, &Position, 1.0f / DSOUND._3DSoundOneMetre ) ;

	VectorOuterProduct( &SideVec, &DirVec,  &UpVector ) ;
	VectorOuterProduct( &UpVector, &SideVec, &DirVec ) ;
	VectorNormalize( &UpVector, &UpVector ) ;
	VectorNormalize( &SideVec, &SideVec ) ;

	// 値がほとんど変化しない場合場合は何もしない
	if( _FABS( DSOUND.X3DAudioListenerData.Position.x - Position.x ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.Position.y - Position.y ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.Position.z - Position.z ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.OrientFront.x - DirVec.x ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.OrientFront.y - DirVec.y ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.OrientFront.z - DirVec.z ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.OrientTop.x - UpVector.x ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.OrientTop.y - UpVector.y ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.OrientTop.z - UpVector.z ) < 0.001f )
		return 0 ;

	DSOUND.X3DAudioListenerData.Position.x = Position.x ;
	DSOUND.X3DAudioListenerData.Position.y = Position.y ;
	DSOUND.X3DAudioListenerData.Position.z = Position.z ;

	DSOUND.X3DAudioListenerData.OrientFront.x = DirVec.x ;
	DSOUND.X3DAudioListenerData.OrientFront.y = DirVec.y ;
	DSOUND.X3DAudioListenerData.OrientFront.z = DirVec.z ;

	DSOUND.X3DAudioListenerData.OrientTop.x = UpVector.x ;
	DSOUND.X3DAudioListenerData.OrientTop.y = UpVector.y ;
	DSOUND.X3DAudioListenerData.OrientTop.z = UpVector.z ;

	DSOUND.ListenerInfo.Position       = Position ;
	DSOUND.ListenerInfo.FrontDirection = DirVec ;
	DSOUND.ListenerInfo.UpDirection    = UpVector ;
	DSOUND.ListenerSideDirection       = SideVec ;

	// 再生中の３Ｄサウンドのパラメータを更新する
	Refresh3DSoundParamAll() ;

	// 終了
	return 0 ;
}

// ３Ｄサウンドのリスナーの移動速度を設定する
extern int NS_Set3DSoundListenerVelocity( VECTOR Velocity )
{
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	VectorScale( &Velocity, &Velocity, 1.0f / DSOUND._3DSoundOneMetre ) ;

	// 値がほとんど変化しない場合場合は何もしない
	if( _FABS( DSOUND.X3DAudioListenerData.Velocity.x - Velocity.x ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.Velocity.y - Velocity.y ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerData.Velocity.z - Velocity.z ) < 0.001f )
		return 0 ;

	DSOUND.X3DAudioListenerData.Velocity.x = Velocity.x ;
	DSOUND.X3DAudioListenerData.Velocity.y = Velocity.y ;
	DSOUND.X3DAudioListenerData.Velocity.z = Velocity.z ;

	DSOUND.ListenerInfo.Velocity = Velocity ;


	// 再生中の３Ｄサウンドのパラメータを更新する
	Refresh3DSoundParamAll() ;

	// 終了
	return 0 ;
}

// ３Ｄサウンドのリスナーの可聴角度範囲を設定する
extern int NS_Set3DSoundListenerConeAngle( float InnerAngle, float OuterAngle )
{
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	if( OuterAngle < 0.0f )
	{
		OuterAngle = 0.0f ;
	}
	else
	if( OuterAngle > DX_PI_F * 2.0f )
	{
		OuterAngle = DX_PI_F * 2.0f ;
	}

	if( InnerAngle < 0.0f )
	{
		InnerAngle = 0.0f ;
	}
	else
	if( InnerAngle > DX_PI_F * 2.0f )
	{
		InnerAngle = DX_PI_F * 2.0f ;
	}

	if( OuterAngle < InnerAngle )
	{
		float Temp ;
		Temp = InnerAngle ;
		InnerAngle = OuterAngle ;
		OuterAngle = Temp ;
	}

	// 値がほとんど変化しない場合場合は何もしない
	if( _FABS( DSOUND.X3DAudioListenerConeData.InnerAngle - InnerAngle ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerConeData.OuterAngle - OuterAngle ) < 0.001f )
		return 0 ;

	DSOUND.X3DAudioListenerConeData.InnerAngle = InnerAngle ;
	DSOUND.X3DAudioListenerConeData.OuterAngle = OuterAngle ;

	DSOUND.ListenerInfo.InnerAngle = InnerAngle ;
	DSOUND.ListenerInfo.OuterAngle = OuterAngle ;

	// 再生中の３Ｄサウンドのパラメータを更新する
	Refresh3DSoundParamAll() ;

	// 終了
	return 0 ;
}

// ３Ｄサウンドのリスナーの可聴角度範囲の音量倍率を設定する
extern int NS_Set3DSoundListenerConeVolume( float InnerAngleVolume, float OuterAngleVolume )
{
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// 値がほとんど変化しない場合場合は何もしない
	if( _FABS( DSOUND.X3DAudioListenerConeData.InnerVolume - InnerAngleVolume ) < 0.001f &&
		_FABS( DSOUND.X3DAudioListenerConeData.OuterVolume - OuterAngleVolume ) < 0.001f )
		return 0 ;

	DSOUND.X3DAudioListenerConeData.InnerVolume = InnerAngleVolume ;
	DSOUND.X3DAudioListenerConeData.OuterVolume = OuterAngleVolume ;

	DSOUND.ListenerInfo.InnerVolume = InnerAngleVolume ;
	DSOUND.ListenerInfo.OuterVolume = OuterAngleVolume ;

	// 再生中の３Ｄサウンドのパラメータを更新する
	Refresh3DSoundParamAll() ;

	// 終了
	return 0 ;
}










// 補助系関数

// サイズ補正
static	int AdjustSoundDataBlock( int Length, SOUND * Sound ) 
{
	return Length / Sound->BufferFormat.nBlockAlign * Sound->BufferFormat.nBlockAlign  ;
}

// 信息采集系函数

// ＤＸライブラリが使用している DirectSound オブジェクトを取得する
extern const void *NS_GetDSoundObj( void )
{
	return DSOUND.DirectSoundObject ;
}


// 波形データ用
static WAVEDATA *AllocWaveData( int Size, int UseDoubleSizeBuffer )
{
	WAVEDATA *Data ;
	int AllocSize ;

	AllocSize = sizeof( WAVEDATA ) + 16 + Size ;
	if( UseDoubleSizeBuffer )
	{
		AllocSize += Size * 2 + 32 ;
	}

	Data = ( WAVEDATA * )DXALLOC( AllocSize ) ;
	if( Data == NULL )
		return NULL ;
	Data->Buffer = ( void * )( ( ( DWORD_PTR )Data + sizeof( WAVEDATA ) + 15 ) / 16 * 16 ) ;
	Data->Bytes = Size ;
	Data->RefCount = 1 ;

	if( UseDoubleSizeBuffer )
	{
		Data->DoubleSizeBuffer = ( void * )( ( ( DWORD_PTR )Data->Buffer + Size + 15 ) / 16 * 16 ) ;
	}
	else
	{
		Data->DoubleSizeBuffer = NULL ;
	}

	return Data ;
}

static int ReleaseWaveData( WAVEDATA *Data )
{
	Data->RefCount -- ;
	if( Data->RefCount > 0 )
		return 0 ;

	if( Data->RefCount < 0 )
		return -1 ;

	DXFREE( Data ) ;

	return 0 ;
}

static WAVEDATA *DuplicateWaveData( WAVEDATA *Data )
{
	Data->RefCount ++ ;
	return Data ;
}


void __stdcall SOUNDBUFFER_CALLBACK::OnVoiceProcessingPassStart( DWORD BytesRequired )
{
	int CompPos = Buffer->CompPos ;
	int Loop = Buffer->Loop ;
	D_XAUDIO2_BUFFER XBuffer = { 0 } ;
	DWORD MoveData ;
	DWORD MaxSendSamples ;
	void *SampleBuffer ;
	int BlockAlign ;

	MaxSendSamples = Buffer->Format.nSamplesPerSec / 60 ;
	SampleBuffer = Buffer->Wave->DoubleSizeBuffer ? Buffer->Wave->DoubleSizeBuffer : Buffer->Wave->Buffer ;
	BlockAlign = Buffer->Format.wBitsPerSample * Buffer->XAudioChannels / 8 ;

	MoveData = 0 ;
	while( MoveData < BytesRequired )
	{
		if( CompPos >= Buffer->SampleNum )
		{
			if( Loop )
			{
				CompPos = 0 ;
				goto COPYDATA ;
			}
			else
			{
				SoundBuffer_Stop( Buffer ) ;
				break ;
			}
		}
		else
		{
			DWORD NowBytes ;
			DWORD AddSamples ;

COPYDATA :
			NowBytes = CompPos * BlockAlign ;
			XBuffer.pAudioData = ( BYTE * )SampleBuffer + NowBytes ;
			AddSamples = Buffer->SampleNum - CompPos ;
			if( AddSamples > MaxSendSamples )
			{
				AddSamples = MaxSendSamples ;
			}
			XBuffer.AudioBytes = AddSamples * BlockAlign ;
			if( XBuffer.AudioBytes > BytesRequired )
			{
				AddSamples = BytesRequired / BlockAlign ;
				XBuffer.AudioBytes = AddSamples * BlockAlign ;
			}
			CompPos += AddSamples ;
			MoveData += XBuffer.AudioBytes ;

			XBuffer.pContext = ( void * )Buffer ;

			if( DSOUND.XAudio2_8DLL != NULL )
			{
				Buffer->XA2_8SourceVoice->SubmitSourceBuffer( &XBuffer ) ;
			}
			else
			{
				Buffer->XA2SourceVoice->SubmitSourceBuffer( &XBuffer ) ;
			}
		}
	}
	Buffer->CompPos = CompPos ;
}

void __stdcall SOUNDBUFFER_CALLBACK::OnVoiceProcessingPassEnd()
{
}

void __stdcall SOUNDBUFFER_CALLBACK::OnStreamEnd()
{
}

void __stdcall SOUNDBUFFER_CALLBACK::OnBufferStart( void* /*pBufferContext*/ )
{
}

void __stdcall SOUNDBUFFER_CALLBACK::OnBufferEnd( void* pBufferContext )
{
	SOUNDBUFFER *Buffer = ( SOUNDBUFFER * )pBufferContext ;

	Buffer->Pos = Buffer->CompPos ;
}

void __stdcall SOUNDBUFFER_CALLBACK::OnLoopEnd( void* /*pBufferContext*/ )
{
}

void __stdcall SOUNDBUFFER_CALLBACK::OnVoiceError( void* /*pBufferContext*/, HRESULT /*Error*/ )
{
}


//サウンドバッファ用
static HRESULT SoundBuffer_Initialize( SOUNDBUFFER *Buffer, D_DSBUFFERDESC *Desc, SOUNDBUFFER *Src, int Is3DSound )
{
	int i ;
	HRESULT hr ;
	WAVEFORMATEX waveformat ;
	int UseDoubleSizeBuffer = FALSE ;

	// 初期化済みの場合はエラー
	if( Buffer->Valid ) return -1 ;

	Buffer->Wave = NULL ;
	Buffer->DSBuffer = NULL ;
	Buffer->XA2SourceVoice = NULL ;
	Buffer->XA2_8SourceVoice = NULL ;
	Buffer->XA2SubmixVoice = NULL ;
	Buffer->XA2_8SubmixVoice = NULL ;
	Buffer->XA2ReverbEffect = NULL ;
	Buffer->XA2Callback = NULL ;
	Buffer->StopTimeState = 0 ;
	Buffer->StopTime = 0 ;
	Buffer->Is3DSound = Is3DSound ;

	waveformat = Src != NULL ? Src->Format : *Desc->lpwfxFormat ;

	if( DSOUND.EnableSoundCaptureFlag )
	{
		if( Src != NULL )
		{
			Buffer->Wave = DuplicateWaveData( Src->Wave ) ;
		}
		else
		{
			Buffer->Wave = AllocWaveData( Desc->dwBufferBytes ) ;
		}
		if( Buffer->Wave == NULL )
			goto ERR ;
	}
	else
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		// ３Ｄサウンドかどうかで分岐
		if( Is3DSound != FALSE )
		{
			D_XAUDIO2_EFFECT_DESCRIPTOR effects[ 1 ] ;
			D_XAUDIO2_EFFECT_CHAIN effectChain ;

			// ３Ｄサウンドの場合はサブミックスボイスも作成する
			hr = D_XAudio2CreateReverb( &Buffer->XA2ReverbEffect, 0 ) ;
			if( FAILED( hr ) )
				goto ERR ;

			effects[ 0 ].pEffect = Buffer->XA2ReverbEffect ;
			effects[ 0 ].InitialState = TRUE ;
			effects[ 0 ].OutputChannels = waveformat.nChannels ;

			effectChain.EffectCount = 1 ;
			effectChain.pEffectDescriptors = effects ;

			if( DSOUND.XAudio2_8DLL != NULL )
			{
				if( FAILED( DSOUND.XAudio2_8Object->CreateSubmixVoice(
					&Buffer->XA2_8SubmixVoice,
					waveformat.nChannels,
					DSOUND.OutputSmaplesPerSec,
					0,
					0,
					NULL,
					&effectChain ) ) )
					goto ERR ;
				_MEMCPY( &Buffer->XAudio2_8ReverbParameter, &DSOUND.XAudio2_8ReverbParameters[ D_XAUDIO2FX_PRESET_DEFAULT ], sizeof( DSOUND.XAudio2_8ReverbParameters[ D_XAUDIO2FX_PRESET_DEFAULT ] ) ) ;
				Buffer->XA2_8SubmixVoice->SetEffectParameters( 0, &Buffer->XAudio2_8ReverbParameter, sizeof( Buffer->XAudio2_8ReverbParameter ) ) ;
				Buffer->XA2_8SubmixVoice->DisableEffect( 0 ) ;
			}
			else
			{
				if( FAILED( DSOUND.XAudio2Object->CreateSubmixVoice(
					&Buffer->XA2SubmixVoice,
					waveformat.nChannels,
					DSOUND.OutputSmaplesPerSec,
					0,
					0,
					NULL,
					&effectChain ) ) )
					goto ERR ;
				_MEMCPY( &Buffer->XAudio2ReverbParameter, &DSOUND.XAudio2ReverbParameters[ D_XAUDIO2FX_PRESET_DEFAULT ], sizeof( DSOUND.XAudio2ReverbParameters[ D_XAUDIO2FX_PRESET_DEFAULT ] ) ) ;
				Buffer->XA2SubmixVoice->SetEffectParameters( 0, &Buffer->XAudio2ReverbParameter, sizeof( Buffer->XAudio2ReverbParameter ) ) ;
				Buffer->XA2SubmixVoice->DisableEffect( 0 ) ;
			}
		}
		else
		{
			// モノラルサウンドでも３Ｄサウンドではない場合はパンのためにステレオに変更
			if( waveformat.nChannels == 1 )
			{
				waveformat.nChannels = 2 ;
				waveformat.nBlockAlign = waveformat.nChannels * waveformat.wBitsPerSample / 8 ;
				waveformat.nAvgBytesPerSec = waveformat.nBlockAlign * waveformat.nSamplesPerSec ;
				UseDoubleSizeBuffer = TRUE ;
			}
		}
		Buffer->XAudioChannels = waveformat.nChannels ;

		if( Src != NULL )
		{
			Buffer->Wave = DuplicateWaveData( Src->Wave ) ;
		}
		else
		{
			Buffer->Wave = AllocWaveData( Desc->dwBufferBytes, UseDoubleSizeBuffer ) ;
		}
		if( Buffer->Wave == NULL )
			goto ERR ;

		Buffer->XA2Callback = new SOUNDBUFFER_CALLBACK ;
		if( Buffer->XA2Callback == NULL )
			goto ERR ;

		Buffer->XA2Callback->Buffer = Buffer ;

		if( DSOUND.XAudio2_8DLL != NULL )
		{
			if( Is3DSound != FALSE )
			{
				D_XAUDIO2_VOICE_SENDS2_8 sendList ;
				D_XAUDIO2_SEND_DESCRIPTOR2_8 sendDescriptors[ 2 ] ;

				sendDescriptors[0].Flags = D_XAUDIO2_SEND_USEFILTER ;
				sendDescriptors[0].pOutputVoice = DSOUND.XAudio2_8MasteringVoiceObject ;
				sendDescriptors[1].Flags = D_XAUDIO2_SEND_USEFILTER ;
				sendDescriptors[1].pOutputVoice = Buffer->XA2_8SubmixVoice ;
				sendList.SendCount = 2 ;
				sendList.pSends = sendDescriptors ;

				hr = DSOUND.XAudio2_8Object->CreateSourceVoice( &Buffer->XA2_8SourceVoice, &waveformat, 0, D_XAUDIO2_DEFAULT_FREQ_RATIO, Buffer->XA2Callback, &sendList ) ;
			}
			else
			{
				hr = DSOUND.XAudio2_8Object->CreateSourceVoice( &Buffer->XA2_8SourceVoice, &waveformat, 0, D_XAUDIO2_DEFAULT_FREQ_RATIO, Buffer->XA2Callback ) ;
			}
		}
		else
		{
			if( Is3DSound != FALSE )
			{
				D_XAUDIO2_VOICE_SENDS sendList ;
				D_XAUDIO2_SEND_DESCRIPTOR sendDescriptors[ 2 ] ;

				sendDescriptors[0].Flags = D_XAUDIO2_SEND_USEFILTER ;
				sendDescriptors[0].pOutputVoice = DSOUND.XAudio2MasteringVoiceObject ;
				sendDescriptors[1].Flags = D_XAUDIO2_SEND_USEFILTER ;
				sendDescriptors[1].pOutputVoice = Buffer->XA2SubmixVoice ;
				sendList.SendCount = 2 ;
				sendList.pSends = sendDescriptors ;

				hr = DSOUND.XAudio2Object->CreateSourceVoice( &Buffer->XA2SourceVoice, &waveformat, 0, D_XAUDIO2_DEFAULT_FREQ_RATIO, Buffer->XA2Callback, &sendList ) ;
			}
			else
			{
				hr = DSOUND.XAudio2Object->CreateSourceVoice( &Buffer->XA2SourceVoice, &waveformat, 0, D_XAUDIO2_DEFAULT_FREQ_RATIO, Buffer->XA2Callback ) ;
			}
		}
		
		if( FAILED( hr ) )
			goto ERR ;

		if( Src != NULL )
		{
			for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
			{
				SoundBuffer_SetVolume( Buffer, i, Src->Volume[ i ] ) ;
			}
			SoundBuffer_SetFrequency( Buffer, Src->Frequency ) ;
			SoundBuffer_SetPan( Buffer, Src->Pan ) ;
		}
	}
	else
	{
		if( DSOUND.DirectSoundObject->CreateSoundBuffer( Desc, &Buffer->DSBuffer, NULL ) != D_DS_OK )
			goto ERR ;
	}

	Buffer->State = FALSE ;
	Buffer->Pos = 0 ;
	Buffer->CompPos = 0 ;
	Buffer->Loop = FALSE ;

	if( Src != NULL )
	{
		Buffer->Pan = Src->Pan ;
		for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
		{
			Buffer->Volume[ i ] = Src->Volume[ i ] ;
		}
		Buffer->Frequency = Src->Frequency ;
		Buffer->SampleNum = Src->SampleNum ;
		Buffer->Format = Src->Format ;
	}
	else
	{
		Buffer->Pan = 0 ;
		for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
		{
			Buffer->Volume[ i ] = 0 ;
		}
		Buffer->Frequency = -1 ;
		Buffer->SampleNum = Desc->dwBufferBytes / Desc->lpwfxFormat->nBlockAlign ;
		Buffer->Format = *Desc->lpwfxFormat ;
	}

	// ３Ｄサウンドの場合は３Ｄサウンドパラメータを初期化
	if( Is3DSound != FALSE )
	{
		// 最初にセットアップを行うために変更したフラグを立てる
		Buffer->EmitterDataChangeFlag = TRUE ;

		// エミッターの基本的な情報をセットする
		if( Src != NULL )
		{
			Buffer->X3DAudioEmitterConeData              = Src->X3DAudioEmitterConeData ;
			Buffer->X3DAudioEmitterData                  = Src->X3DAudioEmitterData ;
			Buffer->X3DAudioEmitterData.pCone            = &Buffer->X3DAudioEmitterConeData ;
			Buffer->X3DAudioEmitterData.pChannelAzimuths = Buffer->X3DAudioEmitterChannelAzimuths ;
			Buffer->EmitterInfo                          = Src->EmitterInfo ;
			Buffer->EmitterRadius                        = Src->EmitterRadius ;
		}
		else
		{
			Buffer->X3DAudioEmitterData.pCone = &Buffer->X3DAudioEmitterConeData ;
			Buffer->X3DAudioEmitterData.pCone->InnerAngle = 0.0f ;
			Buffer->X3DAudioEmitterData.pCone->OuterAngle = 0.0f ;
			Buffer->X3DAudioEmitterData.pCone->InnerVolume = 0.0f ;
			Buffer->X3DAudioEmitterData.pCone->OuterVolume = 1.0f ;
			Buffer->X3DAudioEmitterData.pCone->InnerLPF = 0.0f ;
			Buffer->X3DAudioEmitterData.pCone->OuterLPF = 1.0f ;
			Buffer->X3DAudioEmitterData.pCone->InnerReverb = 0.0f ;
			Buffer->X3DAudioEmitterData.pCone->OuterReverb = 1.0f ;

			Buffer->X3DAudioEmitterData.Position.x = 0.0f ;
			Buffer->X3DAudioEmitterData.Position.y = 0.0f ;
			Buffer->X3DAudioEmitterData.Position.z = 0.0f ;
			Buffer->X3DAudioEmitterData.OrientFront.x = 0.0f ;
			Buffer->X3DAudioEmitterData.OrientFront.y = 0.0f ;
			Buffer->X3DAudioEmitterData.OrientFront.z = 1.0f ;
			Buffer->X3DAudioEmitterData.OrientTop.x = 0.0f ;
			Buffer->X3DAudioEmitterData.OrientTop.y = 1.0f ;
			Buffer->X3DAudioEmitterData.OrientTop.z = 0.0f ;
			Buffer->X3DAudioEmitterData.Velocity.x = 0.0f ;
			Buffer->X3DAudioEmitterData.Velocity.y = 0.0f ;
			Buffer->X3DAudioEmitterData.Velocity.z = 0.0f ;
			Buffer->X3DAudioEmitterData.ChannelRadius = 0.1f ;
			Buffer->X3DAudioEmitterData.pChannelAzimuths = Buffer->X3DAudioEmitterChannelAzimuths ;

			Buffer->X3DAudioEmitterData.InnerRadius = 2.0f;
			Buffer->X3DAudioEmitterData.InnerRadiusAngle = DX_PI_F / 4.0f ;

			Buffer->X3DAudioEmitterData.pVolumeCurve = ( D_X3DAUDIO_DISTANCE_CURVE * )&D_X3DAudioDefault_LinearCurve ;
			Buffer->X3DAudioEmitterData.pLFECurve    = ( D_X3DAUDIO_DISTANCE_CURVE * )&D_Emitter_LFE_Curve ;
			Buffer->X3DAudioEmitterData.pLPFDirectCurve = NULL ;
			Buffer->X3DAudioEmitterData.pLPFReverbCurve = NULL ;
			Buffer->X3DAudioEmitterData.pReverbCurve    = ( D_X3DAUDIO_DISTANCE_CURVE * )&D_Emitter_Reverb_Curve ;
			Buffer->X3DAudioEmitterData.CurveDistanceScaler = 14.0f ;
			Buffer->X3DAudioEmitterData.DopplerScaler = 1.0f ;

			Buffer->EmitterInfo.Position = VGet( 0.0f, 0.0f, 0.0f ) ;
			Buffer->EmitterInfo.FrontDirection = VGet( 0.0f, 0.0f, 1.0f ) ;
			Buffer->EmitterInfo.UpDirection = VGet( 0.0f, 1.0f, 0.0f ) ;
			Buffer->EmitterInfo.Velocity = VGet( 0.0f, 0.0f, 0.0f ) ;
			Buffer->EmitterInfo.InnerAngle = Buffer->X3DAudioEmitterConeData.InnerAngle ;
			Buffer->EmitterInfo.OuterAngle = Buffer->X3DAudioEmitterConeData.OuterAngle ;
			Buffer->EmitterInfo.InnerVolume = Buffer->X3DAudioEmitterConeData.InnerVolume ;
			Buffer->EmitterInfo.OuterVolume = Buffer->X3DAudioEmitterConeData.OuterVolume ;
			Buffer->EmitterRadius = Buffer->X3DAudioEmitterData.CurveDistanceScaler ;

			// チャンネル数を保存する
			Buffer->X3DAudioEmitterData.ChannelCount = Buffer->Format.nChannels/*D_X3DAUDIO_INPUTCHANNELS*/ ;
			for( i = 0 ; i < Buffer->Format.nChannels ; i ++ )
			{
				Buffer->X3DAudioEmitterChannelAzimuths[ i ] = 0.0f ;
			}
		}
	}
	
	Buffer->Valid = TRUE ;
	
	return D_DS_OK ;

ERR :
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			if( Buffer->XA2_8SourceVoice != NULL )
			{
				Buffer->XA2_8SourceVoice->DestroyVoice() ;
				Buffer->XA2_8SourceVoice = NULL ;
			}

			if( Buffer->XA2_8SubmixVoice != NULL )
			{
				Buffer->XA2_8SubmixVoice->DestroyVoice() ;
				Buffer->XA2_8SubmixVoice = NULL ;
			}
		}
		else
		{
			if( Buffer->XA2SourceVoice != NULL )
			{
				Buffer->XA2SourceVoice->DestroyVoice() ;
				Buffer->XA2SourceVoice = NULL ;
			}

			if( Buffer->XA2SubmixVoice != NULL )
			{
				Buffer->XA2SubmixVoice->DestroyVoice() ;
				Buffer->XA2SubmixVoice = NULL ;
			}
		}

		if( Buffer->XA2Callback != NULL )
		{
			delete Buffer->XA2Callback ;
			Buffer->XA2Callback = NULL ;
		}
	}
	else
	{
		if( Buffer->DSBuffer != NULL )
		{
			Buffer->DSBuffer->Release() ;
			Buffer->DSBuffer = NULL ;
		}
	}

	if( Buffer->Wave != NULL )
	{
		ReleaseWaveData( Buffer->Wave ) ;
		Buffer->Wave = NULL ;
	}

	return -1 ;
}

static HRESULT SoundBuffer_Duplicate( SOUNDBUFFER *Buffer, SOUNDBUFFER *Src, int Is3DSound )
{
	return SoundBuffer_Initialize( Buffer, NULL, Src, Is3DSound ) ;
}


static HRESULT SoundBuffer_Terminate(          SOUNDBUFFER *Buffer )
{
	if( Buffer->Valid == FALSE ) return -1 ;

	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			if( Buffer->XA2_8SourceVoice != NULL )
			{
				Buffer->XA2_8SourceVoice->DestroyVoice() ;
				Buffer->XA2_8SourceVoice = NULL ;
			}

			if( Buffer->XA2_8SubmixVoice != NULL )
			{
				Buffer->XA2_8SubmixVoice->DestroyVoice() ;
				Buffer->XA2_8SubmixVoice = NULL ;
			}
		}
		else
		{
			if( Buffer->XA2SourceVoice != NULL )
			{
				Buffer->XA2SourceVoice->DestroyVoice() ;
				Buffer->XA2SourceVoice = NULL ;
			}

			if( Buffer->XA2SubmixVoice != NULL )
			{
				Buffer->XA2SubmixVoice->DestroyVoice() ;
				Buffer->XA2SubmixVoice = NULL ;
			}
		}

		if( Buffer->XA2Callback != NULL )
		{
			delete Buffer->XA2Callback ;
			Buffer->XA2Callback = NULL ;
		}
	}
	else
	{
		if( Buffer->DSBuffer != NULL )
		{
			Buffer->DSBuffer->Release() ;
			Buffer->DSBuffer = NULL ;
		}
	}

	if( Buffer->Wave != NULL )
	{
		ReleaseWaveData( Buffer->Wave ) ;
		Buffer->Wave = NULL ;
	}
	
	Buffer->Valid = FALSE ;
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_Play(               SOUNDBUFFER *Buffer, int Loop )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	Buffer->Loop = Loop ;
	if( DSOUND.EnableSoundCaptureFlag )
	{
		Buffer->State = TRUE ;
	}
	else
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		Buffer->State = TRUE ;
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			Buffer->XA2_8SourceVoice->Start( 0 ) ;
			if( Buffer->XA2_8SubmixVoice )
			{
				Buffer->XA2_8SubmixVoice->EnableEffect( 0 ) ;
			}
		}
		else
		{
			Buffer->XA2SourceVoice->Start( 0 ) ;
			if( Buffer->XA2SubmixVoice )
			{
				Buffer->XA2SubmixVoice->EnableEffect( 0 ) ;
			}
		}
		Buffer->StopTimeState = 1 ;
		Buffer->StopTime = 0 ;
	}
	else
	{
		return Buffer->DSBuffer->Play( 0, 0, Loop ? D_DSBPLAY_LOOPING : 0 ) ;
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_Stop(               SOUNDBUFFER *Buffer, int EffectStop )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( DSOUND.EnableSoundCaptureFlag )
	{
		Buffer->State = FALSE ;
	}
	else
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			Buffer->XA2_8SourceVoice->Stop( 0 ) ;
			if( Buffer->XA2_8SubmixVoice && EffectStop )
			{
				Buffer->XA2_8SubmixVoice->DisableEffect( 0 ) ;
			}
		}
		else
		{
			Buffer->XA2SourceVoice->Stop( 0 ) ;
			if( Buffer->XA2SubmixVoice && EffectStop )
			{
				Buffer->XA2SubmixVoice->DisableEffect( 0 ) ;
			}
		}
		Buffer->State = FALSE ;
	}
	else
	{
		Buffer->DSBuffer->Stop() ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_GetStatus(          SOUNDBUFFER *Buffer, DWORD *Status )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( DSOUND.EnableSoundCaptureFlag ||
		DSOUND.DisableXAudioFlag == FALSE )
	{
		if( Status ) *Status = ( Buffer->State ? D_DSBSTATUS_PLAYING : 0 ) ; 
	}
	else
	{
		if( Buffer->DSBuffer->GetStatus( Status ) != D_DS_OK ) return -1 ;
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_Lock(               SOUNDBUFFER *Buffer, DWORD WritePos , DWORD WriteSize, void **LockPos1, DWORD *LockSize1, void **LockPos2, DWORD *LockSize2 )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( DSOUND.EnableSoundCaptureFlag ||
		DSOUND.DisableXAudioFlag == FALSE )
	{
		int pos, sample, sample1 ;

		pos    = WritePos  / Buffer->Format.nBlockAlign ;
		sample = WriteSize / Buffer->Format.nBlockAlign ;
		if( pos + sample > Buffer->SampleNum )
		{
			sample1 = Buffer->SampleNum - pos ;
			*LockPos1  = (BYTE *)Buffer->Wave->Buffer + WritePos ;
			*LockSize1 = sample1 * Buffer->Format.nBlockAlign ;
			*LockPos2  = (BYTE *)Buffer->Wave->Buffer ;
			*LockSize2 = ( sample - sample1 ) * Buffer->Format.nBlockAlign ;
		}
		else
		{
			*LockPos1  = (BYTE *)Buffer->Wave->Buffer + WritePos ;
			*LockSize1 = WriteSize ;
			*LockPos2  = NULL ;
			*LockSize2 = 0 ;
		}
	}
	else
	{
		if( Buffer->DSBuffer->Lock( WritePos, WriteSize, LockPos1, LockSize1, LockPos2, LockSize2, 0 ) != D_DS_OK ) return -1 ;
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_Unlock(             SOUNDBUFFER *Buffer, void *LockPos1, DWORD LockSize1, void *LockPos2, DWORD LockSize2 )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( DSOUND.EnableSoundCaptureFlag )
	{
		// 不需要特别处理
	}
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		DWORD i ;

		// ダブルサイズの場合はステレオデータにする
		if( Buffer->Wave->DoubleSizeBuffer != NULL )
		{
			switch( Buffer->Format.wBitsPerSample )
			{
			case 8 :
				{
					BYTE *Src8bit ;
					WORD *Dest8bit ;
					DWORD SampleNum ;

					Src8bit = ( BYTE * )LockPos1 ;
					Dest8bit = ( WORD * )Buffer->Wave->DoubleSizeBuffer + ( ( BYTE * )LockPos1 - ( BYTE * )Buffer->Wave->Buffer ) ;
					SampleNum = LockSize1 ;
					for( i = 0 ; i < SampleNum ; i ++ )
					{
						Dest8bit[ i ] = ( WORD )( Src8bit[ i ] + ( Src8bit[ i ] << 8 ) ) ;
					}

					Src8bit = ( BYTE * )LockPos2 ;
					Dest8bit = ( WORD * )Buffer->Wave->DoubleSizeBuffer + ( ( BYTE * )LockPos2 - ( BYTE * )Buffer->Wave->Buffer ) ;
					SampleNum = LockSize2 ;
					for( i = 0 ; i < SampleNum ; i ++ )
					{
						Dest8bit[ i ] = ( WORD )( Src8bit[ i ] + ( Src8bit[ i ] << 8 ) ) ;
					}
				}
				break ;

			case 16 :
				{
					WORD *Src16bit ;
					DWORD *Dest16bit ;
					DWORD SampleNum ;

					Src16bit = ( WORD * )LockPos1 ;
					Dest16bit = ( DWORD * )Buffer->Wave->DoubleSizeBuffer + ( ( WORD * )LockPos1 - ( WORD * )Buffer->Wave->Buffer ) ;
					SampleNum = LockSize1 / 2 ;
					for( i = 0 ; i < SampleNum ; i ++ )
					{
						Dest16bit[ i ] = ( DWORD )( Src16bit[ i ] + ( Src16bit[ i ] << 16 ) ) ;
					}

					Src16bit = ( WORD * )LockPos2 ;
					Dest16bit = ( DWORD * )Buffer->Wave->DoubleSizeBuffer + ( ( WORD * )LockPos2 - ( WORD * )Buffer->Wave->Buffer ) ;
					SampleNum = LockSize2 / 2 ;
					for( i = 0 ; i < SampleNum ; i ++ )
					{
						Dest16bit[ i ] = ( DWORD )( Src16bit[ i ] + ( Src16bit[ i ] << 16 ) ) ;
					}
				}
				break ;
			}
		}
	}
	else
	{
		if( Buffer->DSBuffer->Unlock( LockPos1, LockSize1, LockPos2, LockSize2 ) != D_DS_OK ) return -1 ;
	}
	
	return D_DS_OK ;
}


static HRESULT SoundBuffer_SetFrequency(       SOUNDBUFFER *Buffer, DWORD Frequency )
{
	float SetFreq ;

	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( Frequency == D_DSBFREQUENCY_ORIGINAL ) Buffer->Frequency = -1 ;
	else                                       Buffer->Frequency = Frequency ;

	if( DSOUND.EnableSoundCaptureFlag )
	{
	}
	else
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		if( Frequency == D_DSBFREQUENCY_ORIGINAL )
		{
			SetFreq = 1.0f ;
		}
		else
		{
			SetFreq = ( float )Frequency / Buffer->Format.nSamplesPerSec ;
		}

		if( DSOUND.XAudio2_8DLL != NULL )
		{
			Buffer->XA2_8SourceVoice->SetFrequencyRatio( SetFreq ) ;
		}
		else
		{
			Buffer->XA2SourceVoice->SetFrequencyRatio( SetFreq ) ;
		}
	}
	else
	{
		Buffer->DSBuffer->SetFrequency( Frequency ) ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_GetFrequency(             SOUNDBUFFER *Buffer, LPDWORD Frequency )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( DSOUND.EnableSoundCaptureFlag ||
		DSOUND.DisableXAudioFlag == FALSE )
	{
		if( Buffer->Frequency < 0 )
		{
			*Frequency = Buffer->Format.nSamplesPerSec ;
		}
		else
		{
			*Frequency = Buffer->Frequency ;
		}
	}
	else
	{
		return Buffer->DSBuffer->GetFrequency( Frequency ) ;
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_SetPan(             SOUNDBUFFER *Buffer, LONG Pan )
{
	if( Buffer->Valid == FALSE ) return -1 ;

	if( Pan < -10000 )
	{
		Pan = -10000 ;
	}
	else
	if( Pan > 10000 ) 
	{
		Pan = 10000 ;
	}
	Buffer->Pan = Pan ;

	return SoundBuffer_RefreshVolume( Buffer ) ;
}

static HRESULT SoundBuffer_GetPan(             SOUNDBUFFER *Buffer, LPLONG Pan )
{
	if( Buffer->Valid == FALSE ) return -1 ;

	if( Pan != NULL )
	{
		*Pan = Buffer->Pan ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_RefreshVolume( SOUNDBUFFER *Buffer )
{
	int i ;
	int ChannelNum ;

	ChannelNum = Buffer->Format.nChannels > SOUNDBUFFER_MAX_CHANNEL_NUM ? SOUNDBUFFER_MAX_CHANNEL_NUM : Buffer->Format.nChannels ;

	if( Buffer->Valid == FALSE ) return -1 ;

	if( DSOUND.EnableSoundCaptureFlag )
	{
		if( Buffer->Volume[ 0 ] <= -10000 )
		{
			Buffer->CalcVolume = 0 ;
		}
		else
		if( Buffer->Volume[ 0 ] >= 0 )
		{
			Buffer->CalcVolume = 256 ;
		}
		else
		{
			const double Min = 0.0000000001 ;
			const double Max = 1.0 ;
			
			Buffer->CalcVolume = _DTOL( ( _POW( (float)10, Buffer->Volume[ 0 ] / 10.0f / 100.0f ) / ( Max - Min ) ) * 256 ) ;
		}

		if( Buffer->Pan == -10000 )
		{
			Buffer->CalcPan = -256 ;
		}
		else
		if( Buffer->Pan == 10000 ) 
		{
			Buffer->CalcPan = 256 ;
		}
		else
		if( Buffer->Pan == 0 )
		{
			Buffer->CalcPan = 0 ;
		}
		else
		{
			const double Min = 0.0000000001 ;
			const double Max = 1.0 ;
			int temp ;
			
			temp = _DTOL( ( _POW( (float)10, -( _ABS( Buffer->Pan ) ) / 10.0f / 100.0f ) / ( Max - Min ) ) * 256 ) ;
			Buffer->CalcPan = Buffer->Pan < 0 ? -temp : temp ;
		}
	}
	else
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		float ChannelVolume[ 16 ] ;

		for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
		{
			ChannelVolume[ i ] = D_XAudio2DecibelsToAmplitudeRatio( Buffer->Volume[ i ] / 100.0f ) ;
		}

		if( ChannelNum <= 2 )
		{
			if( Buffer->Pan < 0 )
			{
				ChannelVolume[ 1 ] *= D_XAudio2DecibelsToAmplitudeRatio( Buffer->Pan / 100.0f ) ;
			}
			else
			if( Buffer->Pan > 0 )
			{
				ChannelVolume[ 0 ] *= D_XAudio2DecibelsToAmplitudeRatio( -Buffer->Pan / 100.0f ) ;
			}
		}

		if( DSOUND.XAudio2_8DLL != NULL )
		{
			Buffer->XA2_8SourceVoice->SetChannelVolumes( Buffer->XAudioChannels, ChannelVolume ) ;
		}
		else
		{
			Buffer->XA2SourceVoice->SetChannelVolumes( Buffer->XAudioChannels, ChannelVolume ) ;
		}
	}
	else
	{
		if( ChannelNum > 2 )
		{
			LONG Volume ;

			Volume = ( LONG )Buffer->Volume[ 0 ] ;
			if( Volume > 0 )
			{
				Volume = 0 ;
			}
			else
			if( Volume < -10000 )
			{
				Volume = -10000 ;
			}
			Buffer->DSBuffer->SetVolume( Volume ) ;
			Buffer->DSBuffer->SetPan( Buffer->Pan ) ;
		}
		else
		{
			LONG CalcVolume[ 2 ] ;
			LONG TempVolume[ 2 ] ;
			FLOAT OrigVolume[ 2 ] ;
			LONG Volume ;
			LONG Pan ;

			if( Buffer->Is3DSound )
			{
				OrigVolume[ 0 ] = D_XAudio2DecibelsToAmplitudeRatio( Buffer->Volume[ 0 ] / 100.0f ) ;
				OrigVolume[ 1 ] = D_XAudio2DecibelsToAmplitudeRatio( Buffer->Volume[ 1 ] / 100.0f ) ;

				if( Buffer->DSound_Calc3DPan < 0.0f )
				{
					OrigVolume[ 0 ] *= 1.0f + Buffer->DSound_Calc3DPan ;
				}
				else
				if( Buffer->DSound_Calc3DPan > 0.0f )
				{
					OrigVolume[ 1 ] *= 1.0f - Buffer->DSound_Calc3DPan ;
				}

				TempVolume[ 0 ] = ( LONG )_DTOL( D_XAudio2AmplitudeRatioToDecibels( OrigVolume[ 0 ] * Buffer->DSound_Calc3DVolume ) * 100.0f ) ;
				TempVolume[ 1 ] = ( LONG )_DTOL( D_XAudio2AmplitudeRatioToDecibels( OrigVolume[ 1 ] * Buffer->DSound_Calc3DVolume ) * 100.0f ) ;
			}
			else
			{
				TempVolume[ 0 ] = Buffer->Volume[ 0 ] ;
				TempVolume[ 1 ] = Buffer->Volume[ 1 ] ;
			}

			if( Buffer->Pan < 0 )
			{
				CalcVolume[ 0 ] = 10000 ;
				CalcVolume[ 1 ] = 10000 + Buffer->Pan ;
			}
			else
			{
				CalcVolume[ 0 ] = 10000 - Buffer->Pan ;
				CalcVolume[ 1 ] = 10000 ;
			}

			if( TempVolume[ 0 ] > 0 )
			{
				TempVolume[ 0 ] = 0 ;
			}
			else
			if( TempVolume[ 0 ] < -10000 )
			{
				TempVolume[ 0 ] = -10000 ;
			}
			if( TempVolume[ 1 ] > 0 )
			{
				TempVolume[ 1 ] = 0 ;
			}
			else
			if( TempVolume[ 1 ] < -10000 )
			{
				TempVolume[ 1 ] = -10000 ;
			}

			CalcVolume[ 0 ] = CalcVolume[ 0 ] * ( TempVolume[ 0 ] + 10000 ) / 10000 ;
			CalcVolume[ 1 ] = CalcVolume[ 1 ] * ( TempVolume[ 1 ] + 10000 ) / 10000 ;

			if( CalcVolume[ 0 ] > CalcVolume[ 1 ] )
			{
				Volume = CalcVolume[ 0 ] - 10000 ;
				Pan =    _FTOL( CalcVolume[ 1 ] * ( 10000.0f / CalcVolume[ 0 ] ) ) - 10000 ;
			}
			else
			if( CalcVolume[ 0 ] < CalcVolume[ 1 ] )
			{
				Volume = CalcVolume[ 1 ] - 10000 ;
				Pan = -( _FTOL( CalcVolume[ 0 ] * ( 10000.0f / CalcVolume[ 1 ] ) ) - 10000 ) ;
			}
			else
			{
				Volume = CalcVolume[ 0 ] - 10000 ;
				Pan = 0 ;
			}

			Buffer->DSBuffer->SetPan( ( LONG )Pan ) ;
			Buffer->DSBuffer->SetVolume( ( LONG )Volume ) ;
		}
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_SetVolumeAll( SOUNDBUFFER *Buffer, LONG Volume )
{
	int i ;

	if( Buffer->Valid == FALSE ) return -1 ;

//	if( Volume > 0 )
//	{
//		Volume = 0 ;
//	}
//	else
//	if( Volume < -10000 )
//	{
//		Volume = -10000 ;
//	}

	for( i = 0 ; i < SOUNDBUFFER_MAX_CHANNEL_NUM ; i ++ )
	{
		Buffer->Volume[ i ] = Volume ;
	}

	return SoundBuffer_RefreshVolume( Buffer ) ;
}

static HRESULT SoundBuffer_SetVolume( SOUNDBUFFER *Buffer, int Channel, LONG Volume )
{
	if( Buffer->Valid == FALSE ) return -1 ;

	if( Channel >= SOUNDBUFFER_MAX_CHANNEL_NUM ) return -1 ;

//	if( Volume > 0 )
//	{
//		Volume = 0 ;
//	}
//	else
//	if( Volume < -10000 )
//	{
//		Volume = -10000 ;
//	}

	Buffer->Volume[ Channel ] = Volume ;

	return SoundBuffer_RefreshVolume( Buffer ) ;
}

static HRESULT SoundBuffer_GetVolume( SOUNDBUFFER *Buffer, int Channel, LPLONG Volume )
{
	if( Buffer->Valid == FALSE ) return -1 ;

	if( Channel >= SOUNDBUFFER_MAX_CHANNEL_NUM ) return -1 ;

	if( Volume )
	{
		*Volume = Buffer->Volume[ Channel ] ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_GetCurrentPosition( SOUNDBUFFER *Buffer, LPDWORD PlayPos, LPDWORD WritePos )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( DSOUND.EnableSoundCaptureFlag ||
		DSOUND.DisableXAudioFlag == FALSE )
	{
		if( PlayPos  ) *PlayPos  = (DWORD)Buffer->Pos * Buffer->Format.nBlockAlign ;
		if( WritePos ) *WritePos = (DWORD)Buffer->Pos * Buffer->Format.nBlockAlign ;
	}
	else
	{
		Buffer->DSBuffer->GetCurrentPosition( PlayPos, WritePos ) ;
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_SetCurrentPosition( SOUNDBUFFER *Buffer, DWORD NewPos )
{
	if( Buffer->Valid == FALSE ) return -1 ;
	
	if( DSOUND.EnableSoundCaptureFlag ||
		DSOUND.DisableXAudioFlag == FALSE )
	{
		Buffer->Pos = NewPos / Buffer->Format.nBlockAlign ;
		Buffer->CompPos = Buffer->Pos ;

		if( DSOUND.DisableXAudioFlag == FALSE )
		{
			if( DSOUND.XAudio2_8DLL != NULL )
			{
				Buffer->XA2_8SourceVoice->FlushSourceBuffers() ;
			}
			else
			{
				Buffer->XA2SourceVoice->FlushSourceBuffers() ;
			}
		}
	}
	else
	{
		Buffer->DSBuffer->SetCurrentPosition( NewPos ) ;
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_CycleProcess( SOUNDBUFFER *Buffer )
{
	int NowCount ;
	int Time ;

	if( DSOUND.EnableSoundCaptureFlag )
	{
		return -1 ;
	}
	else
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		if( Buffer->XA2SubmixVoice == NULL )
		{
			return -1 ;
		}

		if( Buffer->StopTimeState == 0 )
		{
			return -1 ;
		}

		if( Buffer->State == TRUE )
		{
			return D_DS_OK ;
		}

		if( Buffer->StopTimeState == 1 )
		{
			Buffer->StopTimeState = 2 ;
			Buffer->StopTime = NS_GetNowCount() ;

			return D_DS_OK ;
		}

		NowCount = NS_GetNowCount() ;

		// 再生が停止してから３秒後にエフェクトも停止する
		if( NowCount < Buffer->StopTime )
		{
			Time = 0x7ffffff - Buffer->StopTime + NowCount ;
		}
		else
		{
			Time = NowCount - Buffer->StopTime ;
		}
		if( Time < 3000 )
		{
			return D_DS_OK ;
		}

		Buffer->StopTimeState = 0 ;

		if( DSOUND.XAudio2_8DLL != NULL )
		{
			if( Buffer->XA2_8SubmixVoice )
			{
				Buffer->XA2_8SubmixVoice->DisableEffect( 0 ) ;
			}
		}
		else
		{
			if( Buffer->XA2SubmixVoice )
			{
				Buffer->XA2SubmixVoice->DisableEffect( 0 ) ;
			}
		}

		return -1 ;
	}
	else
	{
		return -1 ;
	}
	
	return D_DS_OK ;
}


#define DESTADD	\
{									\
	if( vol != 255 )				\
	{								\
		d1 = ( d1 * vol ) >> 8 ;	\
		d2 = ( d2 * vol ) >> 8 ;	\
	}								\
	if( pan != 0 )					\
	{								\
		if( pan > 0 ) d2 = ( d2 * ( 256 -   pan  ) ) >> 8 ;		\
		else          d1 = ( d1 * ( 256 - (-pan) ) ) >> 8 ;		\
	}								\
									\
		 if( DestBuf[0] + d1 >  32767 ) DestBuf[0] =  32767 ;				\
	else if( DestBuf[0] + d1 < -32768 ) DestBuf[0] = -32768 ;				\
	else                                DestBuf[0] += ( short )d1 ; 		\
									\
		 if( DestBuf[1] + d2 >  32767 ) DestBuf[1] =  32767 ;				\
	else if( DestBuf[1] + d2 < -32768 ) DestBuf[1] = -32768 ;				\
	else                                DestBuf[1] += ( short )d2 ; 		\
}

#define CNV( S )		((int)((S) * 65535 / 255) - 32768)

static HRESULT SoundBuffer_FrameProcess(       SOUNDBUFFER *Buffer, int Sample, short *DestBuf )
{
	if( Buffer->Valid == FALSE ) return -1 ;

	if( DSOUND.EnableSoundCaptureFlag )
	{
		int i, pos, d1, d2, b, ch, rate, bit, vol, pan ;
		short *s ;
		BYTE *sb ;
		
		ch = Buffer->Format.nChannels ;
		rate = Buffer->Format.nSamplesPerSec ;
		bit = Buffer->Format.wBitsPerSample ;
		vol = Buffer->Volume[ 0 ] ;
		pan = Buffer->Pan ;
		s = (short *)( (BYTE *)Buffer->Wave->Buffer + Buffer->Pos * Buffer->Format.nBlockAlign ) ;
		sb = (BYTE *)s ;

		if( DestBuf != NULL &&
			( rate == 44100 || rate == 22050 ) &&
			( bit  == 16    || bit  == 8     ) &&
			( ch   == 2     || ch   == 1     ) )
		{
			if( rate == 44100 )
			{
				pos = Buffer->Pos ;
				for( i = 0 ; i < Sample ; i ++, DestBuf += 2 )
				{
					if( bit == 16 )
					{
						if( ch == 2 ){      d1 = s[0]; d2 = s[1]; s += 2 ; }
						else         { d2 = d1 = s[0];            s += 1 ; }
					}
					else
					{
						if( ch == 2 ){      d1 = CNV(sb[0]); d2 = CNV(sb[1]); sb += 2 ; }
						else         { d2 = d1 = CNV(sb[0]);                  sb += 1 ; }
					}
					
					DESTADD
					pos ++ ;
					if( pos >= Buffer->SampleNum )
					{
						if( Buffer->Loop == FALSE ) break ;
						
						s = (short *)Buffer->Wave->Buffer ;
						sb = (BYTE *)s ;
						pos = 0 ;
					}
				}
			}
			else
			if( rate == 22050 )
			{
				pos = Buffer->Pos ;
				b = Buffer->Pos & 1;
				for( i = 0 ; i < Sample ; i ++, DestBuf += 2 )
				{
					if( b )
					{
						if( pos + 1 >= Buffer->SampleNum )
						{
							if( Buffer->Loop == FALSE )
							{
								if( bit == 16 )
								{
									if( ch == 2 )
									{
										d1=s[0]+(s[0]-s[-2])/2;
										d2=s[1]+(s[1]-s[-3])/2;
									}
									else
									{
										d2 = d1=s[0]+(s[0]-s[-1])/2;
									}
								}
								else
								{
									if( ch == 2 )
									{
										d1=CNV(sb[0])+(CNV(sb[0])-CNV(sb[-2]))/2;
										d2=CNV(sb[1])+(CNV(sb[1])-CNV(sb[-3]))/2;
									}
									else
									{
										d2 = d1=CNV(sb[0])+(CNV(sb[0])-CNV(sb[-1]))/2;
									}
								}
								break;
							}
							else
							{
								if( bit == 16 )
								{
									if( ch == 2 )
									{
										d1 = s[0] ;
										d2 = s[1] ;
										s = (short *)Buffer->Wave->Buffer ;
										d1=d1+(s[0]-d1)/2;
										d2=d2+(s[1]-d2)/2;
									}
									else
									{
										d1 = s[0] ;
										s = (short *)Buffer->Wave->Buffer ;
										d2 = d1=d1+(s[0]-d1)/2;
									}
								}
								else
								{
									if( ch == 2 )
									{
										d1 = CNV(sb[0]) ;
										d2 = CNV(sb[1]) ;
										sb = (BYTE *)Buffer->Wave->Buffer ;
										d1=d1+(CNV(sb[0])-d1)/2;
										d2=d2+(CNV(sb[1])-d2)/2;
									}
									else
									{
										d1 = CNV(sb[0]) ;
										sb = (BYTE *)Buffer->Wave->Buffer ;
										d2 = d1=d1+(CNV(sb[0])-d1)/2;
									}
								}

								pos = 0 ;
								b = 0;
							}
						}
						else
						{
							if( bit == 16 )
							{
								if( ch == 2 )
								{
									d1=s[0]+(s[2]-s[0])/2;
									d2=s[1]+(s[3]-s[1])/2;
									s += 2 ;
								}
								else
								{
									d2 = d1=s[0]+(s[1]-s[0])/2;
									s += 1 ;
								}
							}
							else
							{
								if( ch == 2 )
								{
									d1=CNV(sb[0]);d1+=(CNV(sb[2])-d1)/2;
									d2=CNV(sb[1]);d2+=(CNV(sb[3])-d2)/2;
									sb += 2 ;
								}
								else
								{
									d1=CNV(sb[0]);d1+=(CNV(sb[1])-d1)/2;
									d2 = d1;
									sb += 1 ;
								}
							}
							pos ++ ;
							b = b ? 0 : 1;
						}
						DESTADD
					}
					else
					{
						b = b ? 0 : 1;
						if( bit == 16 )
						{
							if( ch == 2 ){      d1 = s[0]; d2 = s[1]; }
							else         { d2 = d1 = s[0];            }
						}
						else
						{
							if( ch == 2 ){      d1 = CNV(sb[0]); d2 = CNV(sb[1]); }
							else         { d2 = d1 = CNV(sb[0]);	              }
						}
						DESTADD
					}
				}
				Sample >>= 1;
			}
		}

		if( Buffer->Pos + Sample >= Buffer->SampleNum )
		{
			if( Buffer->Loop == TRUE )
			{
				Buffer->Pos = ( Buffer->Pos + Sample ) - Buffer->SampleNum ;
			}
			else
			{
				Buffer->Pos = Buffer->SampleNum ;
				Buffer->State = FALSE ;
			}
		}
		else
		{
			Buffer->Pos += Sample ;
		}
		Buffer->CompPos = Buffer->Pos ;
	}
	else
	{
	}
	
	return D_DS_OK ;
}

static HRESULT SoundBuffer_Set3DPosition( SOUNDBUFFER *Buffer, VECTOR *Position )
{
	DWORD State ;

	// ３Ｄサウンドではない場合は何もしない
	if( Buffer->Is3DSound == FALSE )
		return -1 ;

	// 値がほぼ変化しない場合は何もしない
	if( _FABS( Buffer->X3DAudioEmitterData.Position.x - Position->x ) < 0.001f &&
		_FABS( Buffer->X3DAudioEmitterData.Position.y - Position->y ) < 0.001f &&
		_FABS( Buffer->X3DAudioEmitterData.Position.z - Position->z ) < 0.001f )
	{
		return D_DS_OK ;
	}

	// 値を保存
	Buffer->X3DAudioEmitterData.Position.x = Position->x ;
	Buffer->X3DAudioEmitterData.Position.y = Position->y ;
	Buffer->X3DAudioEmitterData.Position.z = Position->z ;

	Buffer->EmitterInfo.Position = *Position ;

	// データが変更されたフラグを立てる
	Buffer->EmitterDataChangeFlag = TRUE ;

	// 再生中だった場合はパラメータを更新
	SoundBuffer_GetStatus( Buffer, &State ) ;
	if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
	{
		SoundBuffer_Refresh3DSoundParam( Buffer ) ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_Set3DRadius( SOUNDBUFFER *Buffer, float Radius )
{
	DWORD State ;

	// ３Ｄサウンドではない場合は何もしない
	if( Buffer->Is3DSound == FALSE )
		return -1 ;

	// 値がほぼ変化しない場合は何もしない
	if( _FABS( Buffer->X3DAudioEmitterData.CurveDistanceScaler - Radius ) < 0.001f )
	{
		return D_DS_OK ;
	}

	// 聞こえる距離を保存
	Buffer->X3DAudioEmitterData.CurveDistanceScaler = Radius ;

	Buffer->EmitterRadius = Radius ;

	// データが変更されたフラグを立てる
	Buffer->EmitterDataChangeFlag = TRUE ;

	// 再生中だった場合はパラメータを更新
	SoundBuffer_GetStatus( Buffer, &State ) ;
	if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
	{
		SoundBuffer_Refresh3DSoundParam( Buffer ) ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_Set3DInnerRadius( SOUNDBUFFER *Buffer, float Radius )
{
	DWORD State ;

	// ３Ｄサウンドではない場合は何もしない
	if( Buffer->Is3DSound == FALSE )
		return -1 ;

	// 値がほぼ変化しない場合は何もしない
	if( _FABS( Buffer->X3DAudioEmitterData.InnerRadius - Radius ) < 0.001f )
	{
		return D_DS_OK ;
	}

	Buffer->X3DAudioEmitterData.InnerRadius = Radius ;

	// データが変更されたフラグを立てる
	Buffer->EmitterDataChangeFlag = TRUE ;

	// 再生中だった場合はパラメータを更新
	SoundBuffer_GetStatus( Buffer, &State ) ;
	if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
	{
		SoundBuffer_Refresh3DSoundParam( Buffer ) ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_Set3DVelocity( SOUNDBUFFER *Buffer, VECTOR *Velocity )
{
	DWORD State ;

	// ３Ｄサウンドではない場合は何もしない
	if( Buffer->Is3DSound == FALSE )
		return -1 ;

	// 値がほとんど変化しない場合場合は何もしない
	if( _FABS( Buffer->X3DAudioEmitterData.Velocity.x - Velocity->x ) < 0.001f &&
		_FABS( Buffer->X3DAudioEmitterData.Velocity.y - Velocity->y ) < 0.001f &&
		_FABS( Buffer->X3DAudioEmitterData.Velocity.z - Velocity->z ) < 0.001f )
	{
		return D_DS_OK ;
	}

	// 値を保存
	Buffer->X3DAudioEmitterData.Velocity.x = Velocity->x ;
	Buffer->X3DAudioEmitterData.Velocity.y = Velocity->y ;
	Buffer->X3DAudioEmitterData.Velocity.z = Velocity->z ;

	Buffer->EmitterInfo.Velocity = *Velocity ;

	// データが変更されたフラグを立てる
	Buffer->EmitterDataChangeFlag = TRUE ;

	// 再生中だった場合はパラメータを更新
	SoundBuffer_GetStatus( Buffer, &State ) ;
	if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
	{
		SoundBuffer_Refresh3DSoundParam( Buffer ) ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_Set3DFrontPosition( SOUNDBUFFER *Buffer, VECTOR *FrontPosition, VECTOR *UpVector )
{
	DWORD State ;
	VECTOR SideVec ;
	VECTOR DirVec ;
	VECTOR Position ;
	VECTOR UpVectorT ;

	// ３Ｄサウンドではない場合は何もしない
	if( Buffer->Is3DSound == FALSE )
		return -1 ;

	Position.x = Buffer->X3DAudioEmitterData.Position.x * DSOUND._3DSoundOneMetre ;
	Position.y = Buffer->X3DAudioEmitterData.Position.y * DSOUND._3DSoundOneMetre ;
	Position.z = Buffer->X3DAudioEmitterData.Position.z * DSOUND._3DSoundOneMetre ;

	VectorSub( &DirVec, FrontPosition, &Position ) ;
	VectorNormalize( &DirVec, &DirVec ) ;

	VectorOuterProduct( &SideVec, &DirVec,  UpVector ) ;
	VectorOuterProduct( &UpVectorT, &SideVec, &DirVec ) ;
	VectorNormalize( &UpVectorT, &UpVectorT ) ;

	// 値がほとんど変化しない場合場合は何もしない
	if( _FABS( Buffer->X3DAudioEmitterData.OrientFront.x - DirVec.x ) < 0.0001f &&
		_FABS( Buffer->X3DAudioEmitterData.OrientFront.y - DirVec.y ) < 0.0001f &&
		_FABS( Buffer->X3DAudioEmitterData.OrientFront.z - DirVec.z ) < 0.0001f &&
		_FABS( Buffer->X3DAudioEmitterData.OrientTop.x - UpVectorT.x ) < 0.0001f &&
		_FABS( Buffer->X3DAudioEmitterData.OrientTop.y - UpVectorT.y ) < 0.0001f &&
		_FABS( Buffer->X3DAudioEmitterData.OrientTop.z - UpVectorT.z ) < 0.0001f )
	{
		return D_DS_OK ;
	}

	// 値を保存
	Buffer->X3DAudioEmitterData.OrientFront.x = DirVec.x ;
	Buffer->X3DAudioEmitterData.OrientFront.y = DirVec.y ;
	Buffer->X3DAudioEmitterData.OrientFront.z = DirVec.z ;

	Buffer->X3DAudioEmitterData.OrientTop.x = UpVectorT.x ;
	Buffer->X3DAudioEmitterData.OrientTop.y = UpVectorT.y ;
	Buffer->X3DAudioEmitterData.OrientTop.z = UpVectorT.z ;

	Buffer->EmitterInfo.FrontDirection = DirVec ;
	Buffer->EmitterInfo.UpDirection = UpVectorT ;

	// データが変更されたフラグを立てる
	Buffer->EmitterDataChangeFlag = TRUE ;

	// 再生中だった場合はパラメータを更新
	SoundBuffer_GetStatus( Buffer, &State ) ;
	if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
	{
		SoundBuffer_Refresh3DSoundParam( Buffer ) ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_Set3DConeAngle( SOUNDBUFFER *Buffer, float InnerAngle, float OuterAngle )
{
	DWORD State ;

	// ３Ｄサウンドではない場合は何もしない
	if( Buffer->Is3DSound == FALSE )
		return -1 ;

	// 値がほぼ変化しない場合は何もしない
	if( _FABS( Buffer->X3DAudioEmitterConeData.InnerAngle - InnerAngle ) < 0.001f &&
		_FABS( Buffer->X3DAudioEmitterConeData.OuterAngle - OuterAngle ) < 0.001f )
	{
		return D_DS_OK ;
	}

	Buffer->X3DAudioEmitterConeData.InnerAngle = InnerAngle ;
	Buffer->X3DAudioEmitterConeData.OuterAngle = OuterAngle ;

	Buffer->EmitterInfo.InnerAngle = InnerAngle ;
	Buffer->EmitterInfo.OuterAngle = OuterAngle ;

	// データが変更されたフラグを立てる
	Buffer->EmitterDataChangeFlag = TRUE ;

	// 再生中だった場合はパラメータを更新
	SoundBuffer_GetStatus( Buffer, &State ) ;
	if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
	{
		SoundBuffer_Refresh3DSoundParam( Buffer ) ;
	}

	return D_DS_OK ;
}

static HRESULT SoundBuffer_Set3DConeVolume( SOUNDBUFFER *Buffer, float InnerAngleVolume, float OuterAngleVolume )
{
	DWORD State ;

	// ３Ｄサウンドではない場合は何もしない
	if( Buffer->Is3DSound == FALSE )
		return -1 ;

	// 値がほぼ変化しない場合は何もしない
	if( _FABS( Buffer->X3DAudioEmitterConeData.InnerVolume - InnerAngleVolume ) < 0.001f &&
		_FABS( Buffer->X3DAudioEmitterConeData.OuterVolume - OuterAngleVolume ) < 0.001f )
	{
		return D_DS_OK ;
	}

	Buffer->X3DAudioEmitterConeData.InnerVolume = InnerAngleVolume ;
	Buffer->X3DAudioEmitterConeData.OuterVolume = OuterAngleVolume ;

	Buffer->EmitterInfo.InnerVolume = InnerAngleVolume ;
	Buffer->EmitterInfo.OuterVolume = OuterAngleVolume ;

	// データが変更されたフラグを立てる
	Buffer->EmitterDataChangeFlag = TRUE ;

	// 再生中だった場合はパラメータを更新
	SoundBuffer_GetStatus( Buffer, &State ) ;
	if( ( State & D_DSBSTATUS_PLAYING ) != 0 )
	{
		SoundBuffer_Refresh3DSoundParam( Buffer ) ;
	}

	return D_DS_OK ;
}


static HRESULT SoundBuffer_Refresh3DSoundParam( SOUNDBUFFER *Buffer, int AlwaysFlag )
{
	DWORD CalcFlags ;
	D_X3DAUDIO_DSP_SETTINGS	DspSettings ;
	float MatrixCoefficients[ D_X3DAUDIO_INPUTCHANNELS * 32 ] ;
	D_XAUDIO2_FILTER_PARAMETERS FilterParametersDirect ;
	D_XAUDIO2_FILTER_PARAMETERS FilterParametersReverb ;
	float Sin, Cos ;
	int i ;
	int Num ;

	if( Buffer->Is3DSound == FALSE || Buffer->Valid == FALSE ) return -1 ;

	// 必ず実行するフラグが倒れていて、データが変更されたフラグも倒れていたら何もしない
	if( AlwaysFlag == FALSE && Buffer->EmitterDataChangeFlag == FALSE )
	{
		return D_DS_OK ;
	}

	if( DSOUND.EnableSoundCaptureFlag )
	{
	}
	else
	// XAudio2 を使用するかどうかで処理を分岐
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		// XAudio2 を使用する場合
		CalcFlags =
			D_X3DAUDIO_CALCULATE_MATRIX |
			D_X3DAUDIO_CALCULATE_DOPPLER |
			D_X3DAUDIO_CALCULATE_LPF_DIRECT |
			D_X3DAUDIO_CALCULATE_LPF_REVERB |
			D_X3DAUDIO_CALCULATE_REVERB ;
		if( ( DSOUND.XAudio2OutputChannelMask & D_SPEAKER_LOW_FREQUENCY ) != 0 )
		{
			CalcFlags |= D_X3DAUDIO_CALCULATE_REDIRECT_TO_LFE ;
		}

		_MEMSET( MatrixCoefficients, 0, sizeof( MatrixCoefficients ) ) ;
		_MEMSET( &DspSettings, 0, sizeof( DspSettings ) ) ;
	//	DspSettings.SrcChannelCount = D_X3DAUDIO_INPUTCHANNELS ;
		DspSettings.SrcChannelCount = Buffer->Format.nChannels ;
		DspSettings.DstChannelCount = DSOUND.OutputChannels ;
		DspSettings.pMatrixCoefficients = MatrixCoefficients ;

		DSOUND.X3DAudioCalculateFunc(
			DSOUND.X3DAudioInstance,
			&DSOUND.X3DAudioListenerData,
			&Buffer->X3DAudioEmitterData,
			CalcFlags,
			&DspSettings ) ;

		_SINCOS( DX_PI_F / 6.0f * DspSettings.LPFDirectCoefficient, &Sin, &Cos ) ;
		FilterParametersDirect.Type = D_LowPassFilter ;
		FilterParametersDirect.Frequency = 2.0f * Sin ;
		FilterParametersDirect.OneOverQ = 1.0f ;

		_SINCOS( DX_PI_F / 6.0f * DspSettings.LPFReverbCoefficient, &Sin, &Cos ) ;
		FilterParametersReverb.Type = D_LowPassFilter ;
		FilterParametersReverb.Frequency = 2.0f * Sin ;
		FilterParametersReverb.OneOverQ = 1.0f ;
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			Buffer->XA2_8SourceVoice->SetFrequencyRatio( DspSettings.DopplerFactor ) ;

			Buffer->XA2_8SourceVoice->SetOutputMatrix(
				DSOUND.XAudio2_8MasteringVoiceObject,
				Buffer->Format.nChannels,
				DSOUND.OutputChannels,
				MatrixCoefficients
			) ;

			Num = Buffer->Format.nChannels * Buffer->Format.nChannels ;
			for( i = 0 ; i < Num ; i ++ )
			{
				MatrixCoefficients[ i ] = DspSettings.ReverbLevel ;
			}
			Buffer->XA2_8SourceVoice->SetOutputMatrix(
				Buffer->XA2_8SubmixVoice,
				Buffer->Format.nChannels,
				Buffer->Format.nChannels,
				MatrixCoefficients
			) ;

			Buffer->XA2_8SourceVoice->SetOutputFilterParameters( DSOUND.XAudio2_8MasteringVoiceObject, &FilterParametersDirect ) ;
			Buffer->XA2_8SourceVoice->SetOutputFilterParameters( Buffer->XA2_8SubmixVoice,             &FilterParametersReverb ) ;
		}
		else
		{
			Buffer->XA2SourceVoice->SetFrequencyRatio( DspSettings.DopplerFactor ) ;
			Buffer->XA2SourceVoice->SetOutputMatrix(
				DSOUND.XAudio2MasteringVoiceObject,
				Buffer->Format.nChannels,
				DSOUND.OutputChannels,
				MatrixCoefficients
			) ;

			Num = Buffer->Format.nChannels * Buffer->Format.nChannels ;
			for( i = 0 ; i < Num ; i ++ )
			{
				MatrixCoefficients[ i ] = DspSettings.ReverbLevel ;
			}
			Buffer->XA2SourceVoice->SetOutputMatrix(
				Buffer->XA2SubmixVoice,
				Buffer->Format.nChannels,
				Buffer->Format.nChannels,
				MatrixCoefficients
			) ;

			Buffer->XA2SourceVoice->SetOutputFilterParameters( DSOUND.XAudio2MasteringVoiceObject, &FilterParametersDirect ) ;
			Buffer->XA2SourceVoice->SetOutputFilterParameters( Buffer->XA2SubmixVoice,             &FilterParametersReverb ) ;
		}
	}
	else
	{
		float Distance ;
		float Angle ;
		float DistanceVolumeRatio ;
		float AngleVolumeRatio ;
		// LONG Volume ;
		// LONG Pan ;
		float fVolume ;
		float fPan ;
		VECTOR ListenerToEmitterVec ;
		VECTOR PanVec ;

		// 距離での減衰率を計算
		ListenerToEmitterVec = VSub( Buffer->EmitterInfo.Position, DSOUND.ListenerInfo.Position ) ;
		Distance = VSize( ListenerToEmitterVec ) ;
		if( Distance > Buffer->EmitterRadius )
		{
			DistanceVolumeRatio = 0.0f ;
		}
		else
		{
			DistanceVolumeRatio = 1.0f - Distance / Buffer->EmitterRadius ;
		}
		ListenerToEmitterVec = VScale( ListenerToEmitterVec, 1.0f / Distance ) ;

		// 角度での減衰率を計算
		Angle = _ACOS( VDot( DSOUND.ListenerInfo.FrontDirection, ListenerToEmitterVec ) ) * 2.0f ;
		if( Angle < DSOUND.ListenerInfo.InnerAngle )
		{
			AngleVolumeRatio = DSOUND.ListenerInfo.InnerVolume ;
		}
		else
		if( Angle > DSOUND.ListenerInfo.OuterAngle )
		{
			AngleVolumeRatio = DSOUND.ListenerInfo.OuterVolume ;
		}
		else
		{
			AngleVolumeRatio = ( Angle - DSOUND.ListenerInfo.InnerAngle ) / ( DSOUND.ListenerInfo.OuterAngle - DSOUND.ListenerInfo.InnerAngle ) ;
			AngleVolumeRatio = ( DSOUND.ListenerInfo.OuterVolume - DSOUND.ListenerInfo.InnerVolume ) * AngleVolumeRatio + DSOUND.ListenerInfo.InnerVolume ;
		}
		// 要素の掛け合わせ
		fVolume = DistanceVolumeRatio * AngleVolumeRatio ;
//		Volume = ( LONG )_DTOL( _LOG10( DistanceVolumeRatio * AngleVolumeRatio ) * 20.0f * 100.0f ) ;
//		if( Volume > 0 )
//		{
//			Volume = 0 ;
//		}
//		else 
//		if( Volume < D_DSBVOLUME_MIN )
//		{
//			Volume = D_DSBVOLUME_MIN ;
//		}

		// 左右バランスを計算
		PanVec.x = VDot( ListenerToEmitterVec, DSOUND.ListenerSideDirection ) ;
		PanVec.y = VDot( ListenerToEmitterVec, DSOUND.ListenerInfo.UpDirection ) ;
		PanVec.z = VDot( ListenerToEmitterVec, DSOUND.ListenerInfo.FrontDirection ) ;
		fPan = PanVec.x < 0.0f ? -PanVec.x : PanVec.x ;
		if( fPan > 0.80f ) fPan = 0.80f ;
//		Pan = ( LONG )_DTOL( _LOG10( 1.0f - fPan ) * 20.0f * 100.0f ) ;
//		if( Pan > 0 )
//		{
//			Pan = 0 ;
//		}
//		else 
//		if( Pan < D_DSBVOLUME_MIN )
//		{
//			Pan = D_DSBVOLUME_MIN ;
//		}
//		if( PanVec.x < 0.0f )
//		{
//			Pan = -Pan ;
//		}
		if( PanVec.x < 0.0f )
		{
			fPan = -fPan ;
		}

		Buffer->DSound_Calc3DVolume = fVolume ;
		Buffer->DSound_Calc3DPan = fPan ;

		SoundBuffer_RefreshVolume( Buffer ) ;
	}

	// データが変更されたフラグを倒す
	Buffer->EmitterDataChangeFlag = FALSE ;

	// 終了
	return D_DS_OK ;
}

static HRESULT SoundBuffer_SetReverbParam( SOUNDBUFFER *Buffer, SOUND3D_REVERB_PARAM *Param )
{
	if( Buffer->Is3DSound == FALSE || Buffer->Valid == FALSE ) return -1 ;

	if( DSOUND.EnableSoundCaptureFlag )
	{
	}
	else
	// XAudio2 を使用する場合のみ有効
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			Buffer->XAudio2_8ReverbParameter.WetDryMix           = Param->WetDryMix ;

			Buffer->XAudio2_8ReverbParameter.ReflectionsDelay    = Param->ReflectionsDelay;
			Buffer->XAudio2_8ReverbParameter.ReverbDelay         = Param->ReverbDelay ;
			Buffer->XAudio2_8ReverbParameter.RearDelay           = Param->RearDelay ;

			Buffer->XAudio2_8ReverbParameter.PositionLeft        = Param->PositionLeft ;
			Buffer->XAudio2_8ReverbParameter.PositionRight       = Param->PositionRight ;
			Buffer->XAudio2_8ReverbParameter.PositionMatrixLeft  = Param->PositionMatrixLeft ;
			Buffer->XAudio2_8ReverbParameter.PositionMatrixRight = Param->PositionMatrixRight ;
			Buffer->XAudio2_8ReverbParameter.EarlyDiffusion      = Param->EarlyDiffusion ;
			Buffer->XAudio2_8ReverbParameter.LateDiffusion       = Param->LateDiffusion ;
			Buffer->XAudio2_8ReverbParameter.LowEQGain           = Param->LowEQGain ;
			Buffer->XAudio2_8ReverbParameter.LowEQCutoff         = Param->LowEQCutoff ;
			Buffer->XAudio2_8ReverbParameter.HighEQGain          = Param->HighEQGain ;
			Buffer->XAudio2_8ReverbParameter.HighEQCutoff        = Param->HighEQCutoff ;

			Buffer->XAudio2_8ReverbParameter.RoomFilterFreq      = Param->RoomFilterFreq ;
			Buffer->XAudio2_8ReverbParameter.RoomFilterMain      = Param->RoomFilterMain ;
			Buffer->XAudio2_8ReverbParameter.RoomFilterHF        = Param->RoomFilterHF ;
			Buffer->XAudio2_8ReverbParameter.ReflectionsGain     = Param->ReflectionsGain ;
			Buffer->XAudio2_8ReverbParameter.ReverbGain          = Param->ReverbGain ;
			Buffer->XAudio2_8ReverbParameter.DecayTime           = Param->DecayTime ;
			Buffer->XAudio2_8ReverbParameter.Density             = Param->Density ;
			Buffer->XAudio2_8ReverbParameter.RoomSize            = Param->RoomSize ;

			Buffer->XAudio2_8ReverbParameter.DisableLateField    = FALSE ;

			Buffer->XA2_8SubmixVoice->SetEffectParameters( 0, &Buffer->XAudio2_8ReverbParameter, sizeof( Buffer->XAudio2_8ReverbParameter ) ) ;
		}
		else
		{
			Buffer->XAudio2ReverbParameter.WetDryMix           = Param->WetDryMix ;

			Buffer->XAudio2ReverbParameter.ReflectionsDelay    = Param->ReflectionsDelay;
			Buffer->XAudio2ReverbParameter.ReverbDelay         = Param->ReverbDelay ;
			Buffer->XAudio2ReverbParameter.RearDelay           = Param->RearDelay ;

			Buffer->XAudio2ReverbParameter.PositionLeft        = Param->PositionLeft ;
			Buffer->XAudio2ReverbParameter.PositionRight       = Param->PositionRight ;
			Buffer->XAudio2ReverbParameter.PositionMatrixLeft  = Param->PositionMatrixLeft ;
			Buffer->XAudio2ReverbParameter.PositionMatrixRight = Param->PositionMatrixRight ;
			Buffer->XAudio2ReverbParameter.EarlyDiffusion      = Param->EarlyDiffusion ;
			Buffer->XAudio2ReverbParameter.LateDiffusion       = Param->LateDiffusion ;
			Buffer->XAudio2ReverbParameter.LowEQGain           = Param->LowEQGain ;
			Buffer->XAudio2ReverbParameter.LowEQCutoff         = Param->LowEQCutoff ;
			Buffer->XAudio2ReverbParameter.HighEQGain          = Param->HighEQGain ;
			Buffer->XAudio2ReverbParameter.HighEQCutoff        = Param->HighEQCutoff ;

			Buffer->XAudio2ReverbParameter.RoomFilterFreq      = Param->RoomFilterFreq ;
			Buffer->XAudio2ReverbParameter.RoomFilterMain      = Param->RoomFilterMain ;
			Buffer->XAudio2ReverbParameter.RoomFilterHF        = Param->RoomFilterHF ;
			Buffer->XAudio2ReverbParameter.ReflectionsGain     = Param->ReflectionsGain ;
			Buffer->XAudio2ReverbParameter.ReverbGain          = Param->ReverbGain ;
			Buffer->XAudio2ReverbParameter.DecayTime           = Param->DecayTime ;
			Buffer->XAudio2ReverbParameter.Density             = Param->Density ;
			Buffer->XAudio2ReverbParameter.RoomSize            = Param->RoomSize ;

			Buffer->XA2SubmixVoice->SetEffectParameters( 0, &Buffer->XAudio2ReverbParameter, sizeof( Buffer->XAudio2ReverbParameter ) ) ;
		}
	}
	else
	{
	}

	// 終了
	return D_DS_OK ;
}

static HRESULT SoundBuffer_SetPresetReverbParam( SOUNDBUFFER *Buffer, int PresetNo )
{
	if( Buffer->Is3DSound == FALSE || Buffer->Valid == FALSE ) return -1 ;

	if( DSOUND.EnableSoundCaptureFlag )
	{
	}
	else
	// XAudio2 を使用する場合のみ有効
	if( DSOUND.DisableXAudioFlag == FALSE )
	{
		if( DSOUND.XAudio2_8DLL != NULL )
		{
			Buffer->XAudio2_8ReverbParameter = DSOUND.XAudio2_8ReverbParameters[ PresetNo ] ;
			Buffer->XA2_8SubmixVoice->SetEffectParameters( 0, &Buffer->XAudio2_8ReverbParameter, sizeof( Buffer->XAudio2_8ReverbParameter ) ) ;
		}
		else
		{
			Buffer->XAudio2ReverbParameter = DSOUND.XAudio2ReverbParameters[ PresetNo ] ;
			Buffer->XA2SubmixVoice->SetEffectParameters( 0, &Buffer->XAudio2ReverbParameter, sizeof( Buffer->XAudio2ReverbParameter ) ) ;
		}
	}
	else
	{
	}

	// 終了
	return D_DS_OK ;
}













// XAudio2関係
static HRESULT D_XAudio2CreateVolumeMeter( IUnknown** ppApo, DWORD Flags )
{
	if( DSOUND.XAudio2_8DLL != NULL )
	{
		return DSOUND.CreateAudioVolumeMeterFunc( ppApo ) ;
	}
	else
	{
		return WinAPIData.Win32Func.CoCreateInstanceFunc((Flags & D_XAUDIO2FX_DEBUG) ? CLSID_AUDIOVOLUMEMeter2_7_DEBUG : CLSID_AUDIOVOLUMEMeter2_7, NULL, CLSCTX_INPROC_SERVER, IID_IUNKNOWN, (void**)ppApo ) ;
	}
}

static HRESULT D_XAudio2CreateReverb( IUnknown** ppApo, DWORD Flags )
{
	if( DSOUND.XAudio2_8DLL != NULL )
	{
		return DSOUND.CreateAudioReverbFunc( ppApo ) ;
	}
	else
	{
	    return WinAPIData.Win32Func.CoCreateInstanceFunc( ( Flags & D_XAUDIO2FX_DEBUG) ? CLSID_AUDIOREVERB2_7_DEBUG : CLSID_AUDIOREVERB2_7, NULL, CLSCTX_INPROC_SERVER, IID_IUNKNOWN, (void**)ppApo ) ;
	}
}

static void D_ReverbConvertI3DL2ToNative( const D_XAUDIO2FX_REVERB_I3DL2_PARAMETERS* pI3DL2, D_XAUDIO2FX_REVERB_PARAMETERS* pNative )
{
    float reflectionsDelay;
    float reverbDelay;

    // RoomRolloffFactor is ignored

    // These parameters have no equivalent in I3DL2
    pNative->RearDelay = D_XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY; // 5
    pNative->PositionLeft = D_XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionRight = D_XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionMatrixLeft = D_XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->PositionMatrixRight = D_XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->RoomSize = D_XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE; // 100
    pNative->LowEQCutoff = 4;
    pNative->HighEQCutoff = 6;

    // The rest of the I3DL2 parameters map to the native property set
    pNative->RoomFilterMain = (float)pI3DL2->Room / 100.0f;
    pNative->RoomFilterHF = (float)pI3DL2->RoomHF / 100.0f;

    if (pI3DL2->DecayHFRatio >= 1.0f)
    {
        int index = _DTOL(-4.0 * _LOG10(pI3DL2->DecayHFRatio));
        if (index < -8) index = -8;
        pNative->LowEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->HighEQGain = 8;
        pNative->DecayTime = pI3DL2->DecayTime * pI3DL2->DecayHFRatio;
    }
    else
    {
        int index = _DTOL(4.0 * _LOG10(pI3DL2->DecayHFRatio));
        if (index < -8) index = -8;
        pNative->LowEQGain = 8;
        pNative->HighEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->DecayTime = pI3DL2->DecayTime;
    }

    reflectionsDelay = pI3DL2->ReflectionsDelay * 1000.0f;
    if (reflectionsDelay >= D_XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY) // 300
    {
        reflectionsDelay = (float)(D_XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY - 1);
    }
    else if (reflectionsDelay <= 1)
    {
        reflectionsDelay = 1;
    }
    pNative->ReflectionsDelay = (DWORD)reflectionsDelay;

    reverbDelay = pI3DL2->ReverbDelay * 1000.0f;
    if (reverbDelay >= D_XAUDIO2FX_REVERB_MAX_REVERB_DELAY) // 85
    {
        reverbDelay = (float)(D_XAUDIO2FX_REVERB_MAX_REVERB_DELAY - 1);
    }
    pNative->ReverbDelay = (BYTE)reverbDelay;

    pNative->ReflectionsGain = pI3DL2->Reflections / 100.0f;
    pNative->ReverbGain = pI3DL2->Reverb / 100.0f;
    pNative->EarlyDiffusion = (BYTE)(15.0f * pI3DL2->Diffusion / 100.0f);
    pNative->LateDiffusion = pNative->EarlyDiffusion;
    pNative->Density = pI3DL2->Density;
    pNative->RoomFilterFreq = pI3DL2->HFReference;

    pNative->WetDryMix = pI3DL2->WetDryMix;
}

static void D_ReverbConvertI3DL2ToNative2_8( const D_XAUDIO2FX_REVERB_I3DL2_PARAMETERS* pI3DL2, D_XAUDIO2FX_REVERB_PARAMETERS2_8* pNative )
{
    float reflectionsDelay;
    float reverbDelay;

    // RoomRolloffFactor is ignored

    // These parameters have no equivalent in I3DL2
    pNative->RearDelay = D_XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY; // 5
    pNative->PositionLeft = D_XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionRight = D_XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionMatrixLeft = D_XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->PositionMatrixRight = D_XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->RoomSize = D_XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE; // 100
    pNative->LowEQCutoff = 4;
    pNative->HighEQCutoff = 6;

    // The rest of the I3DL2 parameters map to the native property set
    pNative->RoomFilterMain = (float)pI3DL2->Room / 100.0f;
    pNative->RoomFilterHF = (float)pI3DL2->RoomHF / 100.0f;

    if (pI3DL2->DecayHFRatio >= 1.0f)
    {
        int index = _DTOL(-4.0 * _LOG10(pI3DL2->DecayHFRatio));
        if (index < -8) index = -8;
        pNative->LowEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->HighEQGain = 8;
        pNative->DecayTime = pI3DL2->DecayTime * pI3DL2->DecayHFRatio;
    }
    else
    {
        int index = _DTOL(4.0 * _LOG10(pI3DL2->DecayHFRatio));
        if (index < -8) index = -8;
        pNative->LowEQGain = 8;
        pNative->HighEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->DecayTime = pI3DL2->DecayTime;
    }

    reflectionsDelay = pI3DL2->ReflectionsDelay * 1000.0f;
    if (reflectionsDelay >= D_XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY) // 300
    {
        reflectionsDelay = (float)(D_XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY - 1);
    }
    else if (reflectionsDelay <= 1)
    {
        reflectionsDelay = 1;
    }
    pNative->ReflectionsDelay = (DWORD)reflectionsDelay;

    reverbDelay = pI3DL2->ReverbDelay * 1000.0f;
    if (reverbDelay >= D_XAUDIO2FX_REVERB_MAX_REVERB_DELAY) // 85
    {
        reverbDelay = (float)(D_XAUDIO2FX_REVERB_MAX_REVERB_DELAY - 1);
    }
    pNative->ReverbDelay = (BYTE)reverbDelay;

    pNative->ReflectionsGain = pI3DL2->Reflections / 100.0f;
    pNative->ReverbGain = pI3DL2->Reverb / 100.0f;
    pNative->EarlyDiffusion = (BYTE)(15.0f * pI3DL2->Diffusion / 100.0f);
    pNative->LateDiffusion = pNative->EarlyDiffusion;
    pNative->Density = pI3DL2->Density;
    pNative->RoomFilterFreq = pI3DL2->HFReference;

    pNative->WetDryMix = pI3DL2->WetDryMix;
    pNative->DisableLateField = FALSE;
}









#ifndef DX_NON_BEEP

// BEEP音播放用命令

// ビープ音周波数設定関数
extern int NS_SetBeepFrequency( int Freq )
{
	WAVEFORMATEX wfmtx ;
	D_DSBUFFERDESC dsbdesc ;
	HRESULT hr ;
	LPVOID write1 ;
	DWORD length1 ;
	LPVOID write2 ;
	DWORD length2 ;
	int ChNum , Rate , Byte ;

	// ビープ音用のバッファを作成する
	{
		// プライマリバッファのフォーマットをセットする
		ChNum = 1 ;
		Rate = 44100 ;
		Byte = 2 ;

		_MEMSET( &wfmtx, 0, sizeof( wfmtx ) ) ;
		wfmtx.cbSize			= 0 ;
		wfmtx.wFormatTag		= WAVE_FORMAT_PCM ;										// PCMフォーマット
		wfmtx.nChannels			= ChNum ;												// チャンネル２つ＝ステレオ
		wfmtx.nSamplesPerSec	= Rate ;												// 再生レート
		wfmtx.wBitsPerSample	= Byte * 8 ;											// １音にかかるデータビット数
		wfmtx.nBlockAlign		= Byte * wfmtx.nChannels ;								// １ヘルツにかかるデータバイト数
		wfmtx.nAvgBytesPerSec	= wfmtx.nSamplesPerSec * wfmtx.nBlockAlign ;			// １秒にかかるデータバイト数

		// バッファ生成ステータスのセット
		_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
		dsbdesc.dwSize			= sizeof( dsbdesc ) ;
		dsbdesc.dwFlags			= D_DSBCAPS_GLOBALFOCUS | D_DSBCAPS_STATIC | D_DSBCAPS_CTRLPAN | D_DSBCAPS_CTRLVOLUME | D_DSBCAPS_CTRLFREQUENCY ;
		dsbdesc.dwBufferBytes	= Byte * Rate ;
		dsbdesc.lpwfxFormat		= &wfmtx ;

		hr = DSOUND.DirectSoundObject->CreateSoundBuffer( &dsbdesc , &DSOUND.BeepSoundBuffer , NULL ) ;
		if( hr != D_DS_OK )
		{
			DXST_ERRORLOG_ADD( _T( "ビープ用DirectSoundのプライマリサウンドバッファの作成に失敗しました\n" ) ) ;
			return -1 ;
		}
	}

	// データをセットする
	{
		DWORD i , j, ltemp ;
		DWORD k ;
		WORD *p ;
		int TempI ;
		float Sin, Cos, Temp ;

		hr = DSOUND.BeepSoundBuffer->Lock( 0 , dsbdesc.dwBufferBytes, &write1 , &length1 , &write2 , &length2 , 0 ) ;		// バッファのロック
		if( hr != D_DS_OK ) return -1 ;

		j = 0 ;
		p = ( WORD * )write1 ;
		ltemp = length1 >> 1 ;
		Temp  = (float)Rate / Freq ;
		TempI = _FTOL( Temp ) ;
		for( i = 0 ; i < ltemp ; i += 1 , j ++ )
		{
			k = j % TempI ;
			_SINCOS( ( PI_F * 2 ) * k / Temp, &Sin, &Cos ) ;
			*p = _FTOL( Sin * 32764 ) ;
			p += 1 ;
		}

		if( write2 != 0 )
		{
			p = ( WORD * )write2 ;
			ltemp = length2 >> 1 ;
			for( i = 0 ; i < ltemp ; i ++ , j ++ )
			{
				k = j % TempI ;
				_SINCOS( ( PI_F * 2 ) / Temp * k, &Sin, &Cos ) ;
				*p = _FTOL( Sin * 32764 ) ;
				p ++ ;
			}
		}
	}

	DSOUND.BeepSoundBuffer->Unlock( write1 , length1 , write2 , length2 ) ;								// バッファのロック解除

	// 終了
	return 0 ;
}

// ビープ音を再生する
extern int NS_PlayBeep( void )
{
	DWORD State ;
	HRESULT hr ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// 再生中だった場合はなにもしないで終了
	hr = DSOUND.BeepSoundBuffer->GetStatus( &State ) ;
	if( hr != D_DS_OK ) return -1 ;
	if( State & D_DSBSTATUS_PLAYING  ) return -1 ;

	// 再生
	DSOUND.BeepSoundBuffer->SetCurrentPosition( 0 ) ;
	hr = DSOUND.BeepSoundBuffer->Play( 0 , 0 , D_DSBPLAY_LOOPING ) ;
	if( hr != D_DS_OK ) return -1 ;

	// 終了
	return 0 ;
}

// ビープ音を止める	
extern int NS_StopBeep( void )
{
	HRESULT hr ;
	DWORD state ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// 再生停止
	if( DSOUND.BeepSoundBuffer )
	{
		hr = DSOUND.BeepSoundBuffer->GetStatus( &state ) ;
		if( hr != D_DS_OK ) return -1 ;
		if( state & D_DSBSTATUS_PLAYING  )
		{
			DSOUND.BeepSoundBuffer->Stop() ;
		}
	}

	// 終了
	return 0 ;
}

#endif








// ラッパー関数

// PlaySoundFile の旧名称
extern int NS_PlaySound( const TCHAR *FileName, int PlayType )
{
	return NS_PlaySoundFile( FileName, PlayType ) ;
}

// CheckSoundFile の旧名称
extern int NS_CheckSound( void )
{
	return NS_CheckSoundFile() ;
}

// StopSoundFile の旧名称
extern int NS_StopSound( void )
{
	return NS_StopSoundFile() ;
}

// SetVolumeSound の旧名称
extern int NS_SetVolumeSound( int VolumePal )
{
	return NS_SetVolumeSoundFile( VolumePal ) ;
}

// WAVEファイルを再生する
extern int NS_PlaySoundFile( const TCHAR *FileName , int PlayType )
{
	LOADSOUND_GPARAM GParam ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// 以前再生中だったデータを止める
	if( DSOUND.PlayWavSoundHandle != -1 )
		NS_DeleteSoundMem( DSOUND.PlayWavSoundHandle ) ;

	// サウンドデータを読み込む
	InitLoadSoundGParam( &GParam ) ;
	DSOUND.PlayWavSoundHandle = LoadSoundMemBase_UseGParam( &GParam, FileName, 1, -1, FALSE, FALSE ) ;
	if( DSOUND.PlayWavSoundHandle == -1 ) return -1 ;

	// サウンドを再生する
	NS_PlaySoundMem( DSOUND.PlayWavSoundHandle , PlayType ) ;

	// 終了
	return 0 ;
}

// WAVEファイルが再生中か調べる
extern int NS_CheckSoundFile( void )
{
	int Result ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;
	if( DSOUND.PlayWavSoundHandle == -1 ) return 0 ;

	Result = NS_CheckSoundMem( DSOUND.PlayWavSoundHandle ) ;

	return Result ;
}

// WAVEファイルの再生を止める
extern int NS_StopSoundFile( void )
{
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;
	if( DSOUND.PlayWavSoundHandle == -1 ) return 0 ;

	return NS_StopSoundMem( DSOUND.PlayWavSoundHandle ) ;
}

// WAVEファイルの音量をセットする
extern int NS_SetVolumeSoundFile( int VolumePal )
{
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;
	if( DSOUND.PlayWavSoundHandle == -1 ) return 0 ;

	return NS_SetVolumeSoundMem( VolumePal, DSOUND.PlayWavSoundHandle ) ;
}

// サウンドキャプチャの開始
extern	int StartSoundCapture( const char *SaveFilePath )
{
	DWORD size ;
	BYTE temp[NORMALWAVE_HEADERSIZE] ;
	
	// サウンドキャプチャが無効な場合は何もせず終了
	if( DSOUND.EnableSoundCaptureFlag == FALSE ) return -1 ;

	// 既にキャプチャを開始している場合は何もせず終了
	if( DSOUND.SoundCaptureFlag == TRUE ) return -1 ;
	DSOUND.SoundCaptureFlag = TRUE;
	
	// 保存用のファイルを開く
	DSOUND.SoundCaptureFileHandle = CreateFileA( SaveFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( DSOUND.SoundCaptureFileHandle == INVALID_HANDLE_VALUE )
	{
		DXST_ERRORLOG_ADD( _T( "サウンドキャプチャ保存用のファイルが開けませんでした。" ) ) ;
		return -1 ;
	}
	
	// ヘッダー分の空データを書き出す
	_MEMSET( temp, 0, sizeof( temp ) ) ;
	WriteFile( DSOUND.SoundCaptureFileHandle, temp, NORMALWAVE_HEADERSIZE, &size, NULL ) ;
	
	// サウンドキャプチャのフラグを立てる
	DSOUND.SoundCaptureFlag = TRUE ;
	
	// キャプチャしたサンプル数を０にする
	DSOUND.SoundCaptureSample = 0 ;
	
	// 終了
	return 0 ;
}

// サウンドキャプチャの周期的処理
extern	int SoundCaptureProcess( int CaptureSample )
{
	int i, j, num, k ;
	DWORD size ;
	short *Temp = NULL ;
	SOUND *sound ;

	if( DSOUND.EnableSoundCaptureFlag == FALSE ) return -1 ;
	
	// キャプチャを行う場合はメモリの確保
	if( DSOUND.SoundCaptureFlag == TRUE )
	{
		Temp = (short *)DXALLOC( CaptureSample * 4 ) ;
		_MEMSET( Temp, 0, CaptureSample * 4 ) ;
	}
	
	// サウンドバッファの進行処理を行う
	num = HandleManageArray[ DX_HANDLETYPE_SOUND ].Num ;
	for( i = 0, j = HandleManageArray[ DX_HANDLETYPE_SOUND ].AreaMin ; i < num ; j ++ )
	{
		if( HandleManageArray[ DX_HANDLETYPE_SOUND ].Handle[ j ] == NULL ) continue ;
		i ++ ;
		
		sound = ( SOUND * )HandleManageArray[ DX_HANDLETYPE_SOUND ].Handle[ j ] ;
		switch( sound->Type )
		{
		case DX_SOUNDTYPE_NORMAL :
			for( k = 0 ; k < MAX_SOUNDBUFFER_NUM ; k ++ )
			{
				if( sound->Buffer[ k ].Valid == FALSE || sound->Buffer[ k ].State == FALSE ) continue ;
				SoundBuffer_FrameProcess( &sound->Buffer[ k ], CaptureSample, Temp ) ;
			}
			break ;
		
		case DX_SOUNDTYPE_STREAMSTYLE :
			if( sound->Buffer[ 0 ].Valid && sound->Buffer[ 0 ].State )
				SoundBuffer_FrameProcess( &sound->Buffer[ 0 ], CaptureSample, Temp ) ;
			break ;
		}
	}
	
	// キャプチャ用のデータを書き出す
	if( DSOUND.SoundCaptureFlag == TRUE )
	{
		WriteFile( DSOUND.SoundCaptureFileHandle, Temp, CaptureSample * 4, &size, NULL ) ;
		DSOUND.SoundCaptureSample += CaptureSample ;
		
		// メモリの解放
		DXFREE( Temp ) ;
	}
	
	// 終了
	return 0 ;
}

// サウンドキャプチャの終了
extern	int EndSoundCapture( void )
{
	BYTE Header[NORMALWAVE_HEADERSIZE], *p ;
	WAVEFORMATEX *format;
	DWORD size ;

	// サウンドキャプチャを実行していなかった場合は何もせず終了
	if( DSOUND.SoundCaptureFlag == FALSE ) return -1 ;
	
	// フォーマットをセット
	format = (WAVEFORMATEX *)&Header[20]; 
	format->wFormatTag      = WAVE_FORMAT_PCM ;
	format->nChannels       = 2 ;
	format->nSamplesPerSec  = 44100 ;
	format->wBitsPerSample  = 16 ;
	format->nBlockAlign     = format->wBitsPerSample / 8 * format->nChannels ;
	format->nAvgBytesPerSec = format->nSamplesPerSec * format->nBlockAlign ;
	format->cbSize          = 0 ;
	
	// ヘッダを書き出してファイルを閉じる
	p = Header;
	_MEMCPY( (char *)p, "RIFF", 4 ) ;									p += 4 ;
	*((DWORD *)p) = DSOUND.SoundCaptureSample * format->nBlockAlign + NORMALWAVE_HEADERSIZE - 8 ;	p += 4 ;
	_MEMCPY( (char *)p, "WAVE", 4 ) ;									p += 4 ;

	_MEMCPY( (char *)p, "fmt ", 4 ) ;									p += 4 ;
	*((DWORD *)p) = NORMALWAVE_FORMATSIZE ;								p += 4 + NORMALWAVE_FORMATSIZE ;

	_MEMCPY( (char *)p, "data", 4 ) ;									p += 4 ;
	*((DWORD *)p) = DSOUND.SoundCaptureSample * format->nBlockAlign ;	p += 4 ;
	
	SetFilePointer( DSOUND.SoundCaptureFileHandle, 0, NULL, FILE_BEGIN ) ;
	WriteFile( DSOUND.SoundCaptureFileHandle, Header, NORMALWAVE_HEADERSIZE, &size, NULL ) ;
	CloseHandle( DSOUND.SoundCaptureFileHandle ) ;
	DSOUND.SoundCaptureFileHandle = NULL ;
	
	// キャプチャ終了
	DSOUND.SoundCaptureFlag = FALSE ;
	
	// 終了
	return 0 ;
}



















// 控制SoftWave的音频类函数

// ソフトサウンドハンドルをセットアップする
extern int SetupSoftSoundHandle(
	int SoftSoundHandle,
	int IsPlayer,
	int Channels,
	int BitsPerSample,
	int SamplesPerSec,
	int SampleNum
)
{
	SOFTSOUND * SSound ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// エラー判定
	if( SSND_MASKHCHK_ASYNC( SoftSoundHandle, SSound ) )
		return -1 ;

	// パラメータの初期化
	SSound->IsPlayer = IsPlayer ;
	SSound->BufferFormat.wFormatTag      = WAVE_FORMAT_PCM ;
	SSound->BufferFormat.nChannels       = ( WORD )Channels ;
	SSound->BufferFormat.nSamplesPerSec  = SamplesPerSec ;
	SSound->BufferFormat.wBitsPerSample  = ( WORD )BitsPerSample ;
	SSound->BufferFormat.nBlockAlign     = SSound->BufferFormat.wBitsPerSample / 8 * SSound->BufferFormat.nChannels ;
	SSound->BufferFormat.nAvgBytesPerSec = SSound->BufferFormat.nSamplesPerSec * SSound->BufferFormat.nBlockAlign ;
	SSound->BufferFormat.cbSize = 0 ;

	// プレイヤーかどうかで処理を分岐
	if( IsPlayer )
	{
		D_DSBUFFERDESC dsbdesc ;

		// プレイヤーの場合

		// リングバッファの初期化
		RingBufInitialize( &SSound->Player.StockSample ) ;
		SSound->Player.StockSampleNum = 0 ;

		// 再生用サウンドバッファの作成
		_MEMSET( &dsbdesc, 0, sizeof( dsbdesc ) ) ;
		dsbdesc.dwSize = sizeof( dsbdesc ) ;
		dsbdesc.dwFlags = D_DSBCAPS_GLOBALFOCUS | D_DSBCAPS_CTRLPAN | D_DSBCAPS_CTRLVOLUME | D_DSBCAPS_CTRLFREQUENCY | D_DSBCAPS_GETCURRENTPOSITION2 | ( DSOUND.UseSoftwareMixing ? D_DSBCAPS_LOCSOFTWARE : D_DSBCAPS_STATIC )  ;
		dsbdesc.dwBufferBytes	= SOUNDSIZE( SSND_PLAYER_STRM_BUFSEC * SSound->BufferFormat.nAvgBytesPerSec / SSND_PLAYER_SEC_DIVNUM, SSound->BufferFormat.nBlockAlign ) ;
		dsbdesc.lpwfxFormat		= &SSound->BufferFormat ;

		if( SoundBuffer_Initialize( &SSound->Player.SoundBuffer, &dsbdesc, NULL, FALSE ) != D_DS_OK )
		{
			DXST_ERRORLOG_ADD( _T( "ソフトサウンドプレイヤー用サウンドバッファの作成に失敗しました" ) ) ;
			return -1 ;
		}
		SSound->Player.SoundBufferSize = dsbdesc.dwBufferBytes ;
		SSound->Player.DataSetCompOffset = 0 ;

		SSound->Player.NoneDataSetCompOffset = -1 ;
		SSound->Player.NoneDataPlayCheckBackPlayOffset = 0 ;
		SSound->Player.NoneDataPlayStartFlag = FALSE ;

		SSound->Player.IsPlayFlag = FALSE ;

		// 先行バッファサイズをセット
		SSound->Player.MaxDataSetSize = SOUNDSIZE( SSND_PLAYER_STRM_SAKICOPYSEC    * SSound->BufferFormat.nAvgBytesPerSec / SSND_PLAYER_SEC_DIVNUM, SSound->BufferFormat.nBlockAlign ) ;
		SSound->Player.MinDataSetSize = SOUNDSIZE( SSND_PLAYER_STRM_MINSAKICOPYSEC * SSound->BufferFormat.nAvgBytesPerSec / SSND_PLAYER_SEC_DIVNUM, SSound->BufferFormat.nBlockAlign ) ;

		// クリティカルセクションの取得
		CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

		// ハンドルリストに追加
		AddHandleList( &DSOUND.SoftSoundPlayerListFirst, &SSound->Player.SoftSoundPlayerList, -1, SSound ) ;

		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;
	}
	else
	{
		// サウンドデータの場合

		// データを格納するメモリ領域を確保
		SSound->Wave.BufferSampleNum = SampleNum ;
		SSound->Wave.Buffer = DXALLOC( SampleNum * SSound->BufferFormat.nBlockAlign ) ;
		if( SSound->Wave.Buffer == NULL )
		{
			DXST_ERRORLOG_ADD( _T( "ソフトハンドルの波形を格納するメモリ領域の確保に失敗しました in AddSoftSoundData" ) ) ;
			return -1 ;
		}
	}

	// 正常終了
	return 0 ;
}

// ソフトサウンドハンドルの初期化
extern int InitializeSoftSoundHandle( HANDLEINFO * )
{
	// 不需要特别处理
	return 0 ;
}

// ソフトサウンドハンドルの後始末
extern int TerminateSoftSoundHandle( HANDLEINFO *HandleInfo )
{
	SOFTSOUND *SSound = ( SOFTSOUND * )HandleInfo ;

	// プレイヤーかどうかで処理を分岐
	if( SSound->IsPlayer == TRUE )
	{
		// リングバッファの解放
		RingBufTerminate( &SSound->Player.StockSample ) ;

		// サウンドバッファーの解放
		SoundBuffer_Terminate( &SSound->Player.SoundBuffer ) ;

		// ソフトサウンドプレイヤーリストから外す
		SubHandleList( &SSound->Player.SoftSoundPlayerList ) ;
	}
	else
	{
		// サウンドデータを格納していたメモリ領域を開放
		if( SSound->Wave.Buffer )
		{
			DXFREE( SSound->Wave.Buffer ) ;
			SSound->Wave.Buffer = NULL ;
		}
	}

	// 終了
	return 0 ;
}

// ソフトサウンドハンドルを削除するかどうかをチェックする関数
extern int DeleteCancelCheckSoftSoundFunction( HANDLEINFO *HandleInfo )
{
	SOFTSOUND *SSound = ( SOFTSOUND * )HandleInfo ;

	return SSound->IsPlayer != FALSE ;
}

// ソフトサウンドプレーヤーハンドルを削除するかどうかをチェックする関数
extern int DeleteCancelCheckSoftSoundPlayerFunction( HANDLEINFO *HandleInfo )
{
	SOFTSOUND *SSound = ( SOFTSOUND * )HandleInfo ;

	return SSound->IsPlayer == FALSE ;
}


// ソフトウエアで扱う波形データをすべて解放する
extern int NS_InitSoftSound( void )
{
	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	return AllHandleSub( DX_HANDLETYPE_SOFTSOUND, DeleteCancelCheckSoftSoundFunction );
}

// LoadSoftSoundBase の実処理関数
static int LoadSoftSoundBase_Static(
	LOADSOUND_GPARAM *GParam,
	int SoftSoundHandle,
	const TCHAR *FileName,
	const void *FileImage,
	int FileImageSize,
	int /*ASyncThread*/
)
{
	SOFTSOUND * SSound ;
	STREAMDATA Stream ;
	SOUNDCONV ConvData ;
	WAVEFORMATEX Format ;
	int SampleNum ;
	void *SrcBuffer = NULL ;
	int SoundSize ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	if( SSND_MASKHCHK_ASYNC( SoftSoundHandle, SSound ) )
		return -1 ;

	// ファイル名が NULL ではない場合はファイルから読み込む
	if( FileName != NULL )
	{
		// 打开文件
		Stream.DataPoint = (void *)FOPEN( FileName ) ;
		if( Stream.DataPoint == NULL ) return -1 ;
		Stream.ReadShred = *GetFileStreamDataShredStruct() ;
	}
	else
	// それ以外の場合はメモリから読み込み
	{
		Stream.DataPoint = MemStreamOpen( ( void *)FileImage, FileImageSize ) ;
		Stream.ReadShred = *GetMemStreamDataShredStruct() ;
	}

	// ＰＣＭ形式に変換
	{
		_MEMSET( &ConvData, 0, sizeof( ConvData ) ) ;
		if( SetupSoundConvert( &ConvData, &Stream, GParam->DisableReadSoundFunctionMask
#ifndef DX_NON_OGGVORBIS
								,GParam->OggVorbisBitDepth, GParam->OggVorbisFromTheoraFile
#endif
								) < 0 )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "ソフトサウンド用の音声ファイルのＰＣＭへの変換に失敗しました" ) )) ;
			goto ERR ;
		}
		if( SoundConvertFast( &ConvData, &Format, &SrcBuffer, &SoundSize ) < 0 )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "ソフトサウンド用の音声ファイルを格納するメモリ領域の確保に失敗しました" ) )) ;
			goto ERR ;
		}

		TerminateSoundConvert( &ConvData ) ;
	}

	// ファイルを閉じる
	if( FileName != NULL )
	{
		FCLOSE( ( DWORD_PTR )Stream.DataPoint ) ;
	}
	else
	{
		MemStreamClose( Stream.DataPoint ) ;
	}
	Stream.DataPoint = NULL ;

	// サンプルの数を算出する
	SampleNum = SoundSize / Format.nBlockAlign ;

	// ハンドルのセットアップ
	if( SetupSoftSoundHandle( SoftSoundHandle, FALSE, Format.nChannels, Format.wBitsPerSample, Format.nSamplesPerSec, SampleNum ) < 0 )
		goto ERR ;

	// サウンドデータのコピー
	_MEMCPY( SSound->Wave.Buffer, SrcBuffer, SoundSize ) ;

	// サウンドデータの解放
	if( SrcBuffer )
	{
		DXFREE( SrcBuffer ) ;
	}

	// 正常終了
	return 0 ;

ERR :
	if( Stream.DataPoint )
	{
		if( FileName != NULL )
		{
			FCLOSE( ( DWORD_PTR )Stream.DataPoint ) ;
		}
		else
		{
			MemStreamClose( Stream.DataPoint ) ;
		}
	}
	TerminateSoundConvert( &ConvData ) ;
	if( SrcBuffer )
	{
		DXFREE( SrcBuffer ) ;
	}

	// エラー終了
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadSoftSoundBase の非同期読み込みスレッドから呼ばれる関数
static void LoadSoftSoundBase_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	LOADSOUND_GPARAM *GParam ;
	int SoftSoundHandle ;
	const TCHAR *FileName ;
	const void *FileImage ;
	int FileImageSize ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( LOADSOUND_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	SoftSoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	FileImage = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	FileImageSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = LoadSoftSoundBase_Static( GParam, SoftSoundHandle, FileName, FileImage, FileImageSize, TRUE ) ;

	DecASyncLoadCount( SoftSoundHandle ) ;
	if( Result < 0 )
	{
		SubHandle( SoftSoundHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// ソフトウエアで扱う波形データをファイルまたはメモリ上に展開されたファイルイメージから作成する
extern int LoadSoftSoundBase_UseGParam(
	LOADSOUND_GPARAM *GParam,
	const TCHAR *FileName,
	const void *FileImage,
	int FileImageSize,
	int ASyncLoadFlag
)
{
	int SoftSoundHandle ;

	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	SoftSoundHandle = AddHandle( DX_HANDLETYPE_SOFTSOUND ) ;
	if( SoftSoundHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FileName, FullPath ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, SoftSoundHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, ( void * )FileImage ) ;
		AddASyncLoadParamInt( NULL, &Addr, FileImageSize ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadSoftSoundBase_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SoftSoundHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, ( void * )FileImage ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileImageSize ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( SoftSoundHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadSoftSoundBase_Static( GParam, SoftSoundHandle, FileName, FileImage, FileImageSize, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return SoftSoundHandle ;

ERR :
	SubHandle( SoftSoundHandle ) ;

	return -1 ;
}

// ソフトウエアで扱う波形データをファイルから作成する
extern	int NS_LoadSoftSound( const TCHAR *FileName )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;
	return LoadSoftSoundBase_UseGParam( &GParam, FileName, NULL, 0, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データをメモリ上に展開されたファイルイメージから作成する
extern	int	NS_LoadSoftSoundFromMemImage( const void *FileImageBuffer, int FileImageSize )
{
	LOADSOUND_GPARAM GParam ;

	InitLoadSoundGParam( &GParam ) ;
	return LoadSoftSoundBase_UseGParam( &GParam, NULL, FileImageBuffer, FileImageSize, GetASyncLoadFlag() ) ;
}

// MakeSoftSoundBase の実処理関数
static int MakeSoftSoundBase_Static(
	int SoftSoundHandle,
	int IsPlayer,
	int Channels,
	int BitsPerSample,
	int SamplesPerSec,
	int SampleNum,
	int UseFormat_SoftSoundHandle,
	int /*ASyncThread*/
)
{
	SOFTSOUND * SSound ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	if( SSND_MASKHCHK_ASYNC( SoftSoundHandle, SSound ) )
		return -1 ;

	if( UseFormat_SoftSoundHandle >= 0 )
	{
		if( NS_GetSoftSoundFormat( UseFormat_SoftSoundHandle, &Channels, &BitsPerSample, &SamplesPerSec ) < 0 )
			return -1 ;
	}

	if( SetupSoftSoundHandle( SoftSoundHandle, IsPlayer, Channels, BitsPerSample, SamplesPerSec, SampleNum ) < 0 )
		return -1 ;

	// 正常終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD
// MakeSoftSoundBase の非同期読み込みスレッドから呼ばれる関数
static void MakeSoftSoundBase_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int SoftSoundHandle ;
	int IsPlayer ;
	int Channels ;
	int BitsPerSample ;
	int SamplesPerSec ;
	int SampleNum ;
	int UseFormat_SoftSoundHandle ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	SoftSoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	IsPlayer = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	Channels = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	BitsPerSample = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SamplesPerSec = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	SampleNum = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	UseFormat_SoftSoundHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = MakeSoftSoundBase_Static( SoftSoundHandle, IsPlayer, Channels, BitsPerSample, SamplesPerSec, SampleNum, UseFormat_SoftSoundHandle, TRUE ) ;

	DecASyncLoadCount( SoftSoundHandle ) ;
	if( Result < 0 )
	{
		SubHandle( SoftSoundHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// ソフトウエアで制御するサウンドデータハンドルの作成
extern int MakeSoftSoundBase_UseGParam(
	int IsPlayer,
	int Channels,
	int BitsPerSample,
	int SamplesPerSec,
	int SampleNum,
	int UseFormat_SoftSoundHandle,
	int ASyncLoadFlag
)
{
	int SoftSoundHandle ;

	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	SoftSoundHandle = AddHandle( DX_HANDLETYPE_SOFTSOUND ) ;
	if( SoftSoundHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, SoftSoundHandle ) ;
		AddASyncLoadParamInt( NULL, &Addr, IsPlayer ) ;
		AddASyncLoadParamInt( NULL, &Addr, Channels ) ;
		AddASyncLoadParamInt( NULL, &Addr, BitsPerSample ) ;
		AddASyncLoadParamInt( NULL, &Addr, SamplesPerSec ) ;
		AddASyncLoadParamInt( NULL, &Addr, SampleNum ) ;
		AddASyncLoadParamInt( NULL, &Addr, UseFormat_SoftSoundHandle ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = MakeSoftSoundBase_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SoftSoundHandle ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, IsPlayer ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, Channels ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, BitsPerSample ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SamplesPerSec ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, SampleNum ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, UseFormat_SoftSoundHandle ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( SoftSoundHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MakeSoftSoundBase_Static( SoftSoundHandle, IsPlayer, Channels, BitsPerSample, SamplesPerSec, SampleNum, UseFormat_SoftSoundHandle, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return SoftSoundHandle ;

ERR :
	SubHandle( SoftSoundHandle ) ;

	return -1 ;
}

// ソフトウエアで扱う空の波形データを作成する( フォーマットは引数のソフトウエアサウンドハンドルと同じものにする )
extern	int NS_MakeSoftSound( int UseFormat_SoftSoundHandle, int SampleNum )
{
	if( UseFormat_SoftSoundHandle < 0 )
		return -1 ;

	return MakeSoftSoundBase_UseGParam( FALSE, 0, 0, 0, SampleNum, UseFormat_SoftSoundHandle, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:2 量子化ビット数:16bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSound2Ch16Bit44KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 2, 16, 44100, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:2 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSound2Ch16Bit22KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 2, 16, 22050, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:2 量子化ビット数:8bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSound2Ch8Bit44KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 2, 8, 44100, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:2 量子化ビット数:8bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSound2Ch8Bit22KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 2, 8, 22050, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:1 量子化ビット数:16bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSound1Ch16Bit44KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 1, 16, 44100, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:1 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSound1Ch16Bit22KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 1, 16, 22050, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:1 量子化ビット数:8bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSound1Ch8Bit44KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 1, 8, 44100, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する( チャンネル数:1 量子化ビット数:8bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSound1Ch8Bit22KHz( int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, 1, 8, 22050, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う空の波形データを作成する
extern	int NS_MakeSoftSoundCustom( int ChannelNum, int BitsPerSample, int SamplesPerSec, int SampleNum )
{
	return MakeSoftSoundBase_UseGParam( FALSE, ChannelNum, BitsPerSample, SamplesPerSec, SampleNum, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データを解放する
extern	int NS_DeleteSoftSound( int SoftSoundHandle )
{
	SOFTSOUND *SSound ;

	if( SSND_MASKHCHK_ASYNC( SoftSoundHandle, SSound ) ) return -1 ;

	// プレイヤーかどうかのチェック
	if( SSound->IsPlayer != FALSE )
	{
		// 違ったらエラー
		return -1 ;
	}

	return SubHandle( SoftSoundHandle ) ;
}

#ifndef DX_NON_SAVEFUNCTION

// ソフトウエアで扱う波形データを無圧縮Wav形式で保存する
extern int NS_SaveSoftSound( int SoftSoundHandle, const TCHAR *FileName )
{
	FILE *fp ;
	BYTE Header[NORMALWAVE_HEADERSIZE], *p ;
	WAVEFORMATEX *format;
	SOFTSOUND * SSound ;

	// エラー判定
	if( SSND_MASKHCHK( SoftSoundHandle, SSound ) ) return -1 ;
	if( SSound->IsPlayer == 1 ) return -1 ;

#ifdef UNICODE
	fp = _wfopen( FileName, L"wb" ) ;
#else
	fp = fopen( FileName, "wb" ) ;
#endif

	// フォーマットをセット
	format = (WAVEFORMATEX *)&Header[20]; 
	format->wFormatTag      = WAVE_FORMAT_PCM ;
	format->nChannels       = SSound->BufferFormat.nChannels ;
	format->nSamplesPerSec  = SSound->BufferFormat.nSamplesPerSec ;
	format->wBitsPerSample  = SSound->BufferFormat.wBitsPerSample ;
	format->nBlockAlign     = format->wBitsPerSample / 8 * format->nChannels ;
	format->nAvgBytesPerSec = format->nSamplesPerSec * format->nBlockAlign ;
	format->cbSize          = 0 ;
	
	// ヘッダを書き出してファイルを閉じる
	p = Header;
	_MEMCPY( (char *)p, "RIFF", 4 ) ;																	p += 4 ;
	*((DWORD *)p) = SSound->Wave.BufferSampleNum * format->nBlockAlign + NORMALWAVE_HEADERSIZE - 8 ;	p += 4 ;
	_MEMCPY( (char *)p, "WAVE", 4 ) ;																	p += 4 ;

	_MEMCPY( (char *)p, "fmt ", 4 ) ;																	p += 4 ;
	*((DWORD *)p) = NORMALWAVE_FORMATSIZE ;																p += 4 + NORMALWAVE_FORMATSIZE ;

	_MEMCPY( (char *)p, "data", 4 ) ;																	p += 4 ;
	*((DWORD *)p) = SSound->Wave.BufferSampleNum * format->nBlockAlign ;								p += 4 ;
	
	fwrite( Header, NORMALWAVE_HEADERSIZE, 1, fp ) ;
	fwrite( SSound->Wave.Buffer, SSound->Wave.BufferSampleNum * format->nBlockAlign, 1, fp ) ;
	fclose( fp ) ;

	// 終了
	return 0 ;
}

#endif // DX_NON_SAVEFUNCTION

// ソフトウエアで扱う波形データのサンプル数を取得する
extern	int NS_GetSoftSoundSampleNum( int SoftSoundHandle )
{
	SOFTSOUND * SSound ;

	// エラー判定
	if( SSND_MASKHCHK( SoftSoundHandle, SSound ) ) return -1 ;
	if( SSound->IsPlayer == 1 ) return -1 ;

	// サンプル数を返す
	return SSound->Wave.BufferSampleNum ;
}

// ソフトウエアで扱う波形データのフォーマットを取得する
extern	int NS_GetSoftSoundFormat( int SoftSoundHandle, int *Channels, int *BitsPerSample, int *SamplesPerSec )
{
	SOFTSOUND * SSound ;

	// エラー判定
	if( SSND_MASKHCHK( SoftSoundHandle, SSound ) ) return -1 ;
	if( SSound->IsPlayer == 1 ) return -1 ;

	// データをセット
	if( Channels      ) *Channels      = SSound->BufferFormat.nChannels ;
	if( BitsPerSample ) *BitsPerSample = SSound->BufferFormat.wBitsPerSample ;
	if( SamplesPerSec ) *SamplesPerSec = SSound->BufferFormat.nSamplesPerSec ;

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データのサンプルを読み取る
extern	int NS_ReadSoftSoundData( int SoftSoundHandle, int SamplePosition, int *Channel1, int *Channel2 )
{
	SOFTSOUND * SSound ;
	BYTE *Src ;

	// エラー判定
	if( SSND_MASKHCHK( SoftSoundHandle, SSound ) ) return -1 ;
	if( SSound->IsPlayer == 1 ) return -1 ;

	// サンプルの範囲外だった場合もエラー
	if( SSound->Wave.BufferSampleNum <= SamplePosition || SamplePosition < 0 )
		return -1 ;

	// サンプルの位置を算出
	Src = ( BYTE * )SSound->Wave.Buffer + SSound->BufferFormat.nBlockAlign * SamplePosition ;

	// サンプルを返す
	switch( SSound->BufferFormat.wBitsPerSample )
	{
	case 8 :
		if( Channel1 )
		{
			*Channel1 = ( int )Src[ 0 ] ;
		}

		if( Channel2 )
		{
			if( SSound->BufferFormat.nChannels == 1 )
			{
				*Channel2 = 127 ;
			}
			else
			{
				*Channel2 = ( int )Src[ 1 ] ;
			}
		}
		break ;

	case 16 :
		if( Channel1 )
		{
			*Channel1 = ( ( short * )Src )[ 0 ] ;
		}

		if( Channel2 )
		{
			if( SSound->BufferFormat.nChannels == 1 )
			{
				*Channel2 = 0 ;
			}
			else
			{
				*Channel2 = ( ( short * )Src )[ 1 ] ;
			}
		}
		break ;
	}

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データのサンプルを書き込む
extern	int NS_WriteSoftSoundData( int SoftSoundHandle, int SamplePosition, int Channel1, int Channel2 )
{
	SOFTSOUND * SSound ;
	BYTE *Dest ;

	// エラー判定
	if( SSND_MASKHCHK( SoftSoundHandle, SSound ) ) return -1 ;
	if( SSound->IsPlayer == 1 ) return -1 ;

	// サンプルの範囲外だった場合もエラー
	if( SSound->Wave.BufferSampleNum <= SamplePosition || SamplePosition < 0 )
		return -1 ;

	// サンプルの位置を算出
	Dest = ( BYTE * )SSound->Wave.Buffer + SSound->BufferFormat.nBlockAlign * SamplePosition ;

	// サンプルを書き込む
	switch( SSound->BufferFormat.wBitsPerSample )
	{
	case 8 :
		     if( Channel1 > 255 ) Channel1 = 255 ;
		else if( Channel1 <   0 ) Channel1 =   0 ;
		Dest[ 0 ] = ( BYTE )Channel1 ;

		if( SSound->BufferFormat.nChannels != 1 )
		{
				 if( Channel2 > 255 ) Channel2 = 255 ;
			else if( Channel2 <   0 ) Channel2 =   0 ;
			Dest[ 1 ] = ( BYTE )Channel2 ;
		}
		break ;

	case 16 :
			 if( Channel1 >  32767 ) Channel1 =  32767 ;
		else if( Channel1 < -32768 ) Channel1 = -32768 ;
		( ( short * )Dest )[ 0 ] = ( short )Channel1 ;

		if( SSound->BufferFormat.nChannels != 1 )
		{
				 if( Channel2 >  32767 ) Channel2 =  32767 ;
			else if( Channel2 < -32768 ) Channel2 = -32768 ;
			( ( short * )Dest )[ 1 ] = ( short )Channel2 ;
		}
		break ;
	}

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データの波形イメージが格納されているメモリアドレスを取得する
extern	void *NS_GetSoftSoundDataImage( int SoftSoundHandle )
{
	SOFTSOUND * SSound ;

	// エラー判定
	if( SSND_MASKHCHK_ASYNC( SoftSoundHandle, SSound ) ) return NULL ;
	if( SSound->IsPlayer == 1 ) return NULL ;

	// 終了
	return SSound->Wave.Buffer ;
}


// ソフトウエアで扱う波形データのプレイヤーをすべて解放する
extern	int NS_InitSoftSoundPlayer( void )
{
	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	return AllHandleSub( DX_HANDLETYPE_SOFTSOUND, DeleteCancelCheckSoftSoundPlayerFunction );
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( フォーマットは引数のソフトウエアサウンドハンドルと同じものにする )
extern int NS_MakeSoftSoundPlayer( int UseFormat_SoftSoundHandle )
{
	if( UseFormat_SoftSoundHandle < 0 )
		return -1 ;

	return MakeSoftSoundBase_UseGParam( TRUE, 0, 0, 0, 0, UseFormat_SoftSoundHandle, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:2 量子化ビット数:16bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSoundPlayer2Ch16Bit44KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 2, 16, 44100, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:2 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSoundPlayer2Ch16Bit22KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 2, 16, 22050, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:2 量子化ビット数:8bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSoundPlayer2Ch8Bit44KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 2, 8, 44100, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:2 量子化ビット数:8bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSoundPlayer2Ch8Bit22KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 2, 8, 22050, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:1 量子化ビット数:16bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSoundPlayer1Ch16Bit44KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 1, 16, 44100, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:1 量子化ビット数:16bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSoundPlayer1Ch16Bit22KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 1, 16, 22050, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:1 量子化ビット数:8bit サンプリング周波数:44KHz )
extern	int NS_MakeSoftSoundPlayer1Ch8Bit44KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 1, 8, 44100, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する( チャンネル数:1 量子化ビット数:8bit サンプリング周波数:22KHz )
extern	int NS_MakeSoftSoundPlayer1Ch8Bit22KHz( void )
{
	return MakeSoftSoundBase_UseGParam( TRUE, 1, 8, 22050, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを作成する
extern	int NS_MakeSoftSoundPlayerCustom( int ChannelNum, int BitsPerSample, int SamplesPerSec )
{
	return MakeSoftSoundBase_UseGParam( TRUE, ChannelNum, BitsPerSample, SamplesPerSec, 0, -1, GetASyncLoadFlag() ) ;
}

// ソフトウエアで扱う波形データのプレイヤーを削除する
extern	int NS_DeleteSoftSoundPlayer( int SSoundPlayerHandle )
{
	SOFTSOUND *SSound ;

	if( SSND_MASKHCHK_ASYNC( SSoundPlayerHandle, SSound ) ) return -1 ;

	// プレイヤーかどうかのチェック
	if( SSound->IsPlayer != TRUE )
	{
		// 違ったらエラー
		return -1 ;
	}

	return SubHandle( SSoundPlayerHandle ) ;
}

// ソフトウエアで扱う波形データのプレイヤーに波形データを追加する( フォーマットが同じではない場合はエラー )
extern	int NS_AddDataSoftSoundPlayer( int SSoundPlayerHandle, int SoftSoundHandle, int AddSamplePosition, int AddSampleNum )
{
	SOFTSOUND *SPlayer, *SSound ;
	void *Src ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// エラー判定
	if( SSND_MASKHCHK( SoftSoundHandle, SSound ) ) return -1 ;
	if( SSound->IsPlayer == 1 ) return -1 ;

	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) ) return -1 ;
	if( SPlayer->IsPlayer == 0 ) return -1 ;

	// フォーマットが違ったらエラー
	if( SSound->BufferFormat.nChannels != SPlayer->BufferFormat.nChannels ||
		SSound->BufferFormat.wBitsPerSample != SPlayer->BufferFormat.wBitsPerSample ||
		SSound->BufferFormat.nSamplesPerSec != SPlayer->BufferFormat.nSamplesPerSec )
		return -1 ;

	// サンプル位置の指定が間違っていたらエラー
	if( AddSampleNum == 0 || SSound->Wave.BufferSampleNum > AddSamplePosition + AddSampleNum )
		return -1 ; 

	// 転送位置のアドレスを算出
	Src = ( BYTE * )SSound->Wave.Buffer + SSound->BufferFormat.nBlockAlign * AddSamplePosition ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// リングバッファにデータを追加
	if( RingBufDataAdd( &SPlayer->Player.StockSample, Src, AddSampleNum * SSound->BufferFormat.nBlockAlign ) < 0 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;
		return -1 ;
	}

	// 追加した分サンプル数を増やす
	SPlayer->Player.StockSampleNum += AddSampleNum ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データのプレイヤーにプレイヤーが対応したフォーマットの生波形データを追加する
extern	int	NS_AddDirectDataSoftSoundPlayer( int SSoundPlayerHandle, const void *SoundData, int AddSampleNum )
{
	SOFTSOUND * SPlayer ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) ) return -1 ;
	if( SPlayer->IsPlayer == 0 ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// リングバッファにデータを追加
	if( RingBufDataAdd( &SPlayer->Player.StockSample, SoundData, AddSampleNum * SPlayer->BufferFormat.nBlockAlign ) < 0 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;
		return -1 ;
	}

	// 追加した分サンプル数を増やす
	SPlayer->Player.StockSampleNum += AddSampleNum ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データのプレイヤーに波形データを一つ追加する
extern int NS_AddOneDataSoftSoundPlayer( int SSoundPlayerHandle, int Channel1, int Channel2 )
{
	SOFTSOUND * SPlayer ;
	BYTE Dest[ 16 ] ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) ) return -1 ;
	if( SPlayer->IsPlayer == 0 ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// データの準備
	switch( SPlayer->BufferFormat.wBitsPerSample )
	{
	case 8 :
		     if( Channel1 > 255 ) Channel1 = 255 ;
		else if( Channel1 <   0 ) Channel1 =   0 ;
		Dest[ 0 ] = ( BYTE )Channel1 ;

		if( SPlayer->BufferFormat.nChannels != 1 )
		{
				 if( Channel2 > 255 ) Channel2 = 255 ;
			else if( Channel2 <   0 ) Channel2 =   0 ;
			Dest[ 1 ] = ( BYTE )Channel2 ;
		}
		break ;

	case 16 :
			 if( Channel1 >  32767 ) Channel1 =  32767 ;
		else if( Channel1 < -32768 ) Channel1 = -32768 ;
		( ( short * )Dest )[ 0 ] = ( short )Channel1 ;

		if( SPlayer->BufferFormat.nChannels != 1 )
		{
				 if( Channel2 >  32767 ) Channel2 =  32767 ;
			else if( Channel2 < -32768 ) Channel2 = -32768 ;
			( ( short * )Dest )[ 1 ] = ( short )Channel2 ;
		}
		break ;
	}

	// リングバッファにデータを追加
	if( RingBufDataAdd( &SPlayer->Player.StockSample, Dest, SPlayer->BufferFormat.nBlockAlign ) < 0 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;
		return -1 ;
	}

	// サンプル数を一つ増やす
	SPlayer->Player.StockSampleNum ++ ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}


// ソフトウエアで扱う波形データのプレイヤーに再生用サウンドバッファに転送していない波形データが無く、再生用サウンドバッファにも無音データ以外無いかどうかを取得する( TRUE:無音データ以外無い  FALSE:有効データがある )
extern	int	NS_CheckSoftSoundPlayerNoneData( int SSoundPlayerHandle )
{
	SOFTSOUND * SPlayer ;
	int Result = -1 ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) || SPlayer->IsPlayer == 0 )
		goto END ;

	// 更新処理を行う
	_SoftSoundPlayerProcess( SPlayer ) ;

	// 無音データ再生中かどうかのフラグをセット
	Result = SPlayer->Player.NoneDataPlayStartFlag ;

	// 音データのストックがある場合は無音データ再生中ではない
	if( SPlayer->Player.StockSampleNum != 0 )
		Result = FALSE ;

END :
	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 状態を返す
	return Result ;
}

// ソフトウエアで扱う波形データのプレイヤーに追加した波形データでまだ再生用サウンドバッファに転送されていない波形データのサンプル数を取得する
extern	int	NS_GetStockDataLengthSoftSoundPlayer( int SSoundPlayerHandle )
{
	SOFTSOUND * SPlayer ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) || SPlayer->IsPlayer == 0 )
		return -1 ;

	// リングバッファにある未転送分のサンプルを返す
	return SPlayer->Player.StockSampleNum ;
}

// ソフトウエアで扱う波形データのプレイヤーが扱うデータフォーマットを取得する
extern	int	NS_GetSoftSoundPlayerFormat( int SSoundPlayerHandle, int *Channels, int *BitsPerSample, int *SamplesPerSec )
{
	SOFTSOUND * SPlayer ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) ) return -1 ;
	if( SPlayer->IsPlayer == 0 ) return -1 ;

	// データをセット
	if( Channels      ) *Channels      = SPlayer->BufferFormat.nChannels ;
	if( BitsPerSample ) *BitsPerSample = SPlayer->BufferFormat.wBitsPerSample ;
	if( SamplesPerSec ) *SamplesPerSec = SPlayer->BufferFormat.nSamplesPerSec ;

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データのプレイヤーの再生処理を開始する
extern	int NS_StartSoftSoundPlayer( int SSoundPlayerHandle )
{
	SOFTSOUND * SPlayer ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) || SPlayer->IsPlayer == 0 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

		return -1 ;
	}

	// 既に再生されていたら何もしない
	if( SPlayer->Player.IsPlayFlag )
		goto END ;

	// 再生状態にする
	SPlayer->Player.IsPlayFlag = TRUE ;

	// 再生用データをサウンドバッファにセットする
	_SoftSoundPlayerProcess( SPlayer ) ;

	// サウンドバッファの再生開始
	SoundBuffer_Play( &SPlayer->Player.SoundBuffer, TRUE ) ;

END :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データのプレイヤーの再生処理が開始されているか取得する( TRUE:開始している  FALSE:停止している )
extern	int	NS_CheckStartSoftSoundPlayer( int SSoundPlayerHandle )
{
	SOFTSOUND * SPlayer ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) || SPlayer->IsPlayer == 0 )
		return -1 ;

	// 再生中フラグを返す
	return SPlayer->Player.IsPlayFlag ;
}


// ソフトウエアで扱う波形データのプレイヤーの再生処理を停止する
extern	int NS_StopSoftSoundPlayer( int SSoundPlayerHandle )
{
	SOFTSOUND * SPlayer ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) || SPlayer->IsPlayer == 0 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

		return -1 ;
	}

	// 再生されていなかったら何もしない
	if( SPlayer->Player.IsPlayFlag == FALSE )
		goto END ;

	// 再生を止める
	SoundBuffer_Stop( &SPlayer->Player.SoundBuffer, TRUE ) ;

	// 再生中フラグを倒す
	SPlayer->Player.IsPlayFlag = FALSE ;

END :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データのプレイヤーの状態を初期状態に戻す( 追加された波形データは削除され、再生状態だった場合は停止する )
extern	int NS_ResetSoftSoundPlayer( int SSoundPlayerHandle )
{
	SOFTSOUND * SPlayer ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// エラー判定
	if( SSND_MASKHCHK( SSoundPlayerHandle, SPlayer ) || SPlayer->IsPlayer == 0 )
	{
		// クリティカルセクションの解放
		CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

		return -1 ;
	}

	// 再生を止める
	SoundBuffer_Stop( &SPlayer->Player.SoundBuffer, TRUE ) ;

	// 再生位置を先頭に戻す
	SoundBuffer_SetCurrentPosition( &SPlayer->Player.SoundBuffer, 0 ) ;

	// リングバッファを再初期化する
	RingBufTerminate( &SPlayer->Player.StockSample ) ;
	RingBufInitialize( &SPlayer->Player.StockSample ) ;
	SPlayer->Player.StockSampleNum = 0 ;

	// その他情報を初期化する
	SPlayer->Player.DataSetCompOffset = 0 ;
	SPlayer->Player.NoneDataSetCompOffset = 0 ;
	SPlayer->Player.NoneDataPlayCheckBackPlayOffset = 0 ;
	SPlayer->Player.NoneDataPlayStartFlag = FALSE ;
	SPlayer->Player.IsPlayFlag = FALSE ;

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// 全てのソフトウエアサウンドの一時停止状態を変更する
extern int PauseSoftSoundAll( int PauseFlag )
{
	HANDLELIST *List ;
	SOFTSOUND * SPlayer ;
	DWORD State ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 停止するか開始するかで処理を分岐
	if( PauseFlag )
	{
		// 停止する場合

		for( List = DSOUND.SoftSoundPlayerListFirst.Next ; List->Next != NULL ; List = List->Next )
		{
			SPlayer = ( SOFTSOUND * )List->Data ;

			if( SPlayer->Player.SoundBuffer.Valid == FALSE || SPlayer->Player.SoundBuffer.DSBuffer == NULL )
				continue ;

			// 既に状態保存済みの場合は何もしない
			if( SPlayer->Player.SoundBufferPlayStateBackupFlagValid )
				continue ;

			// サウンドバッファの再生状態を保存
			if( SoundBuffer_GetStatus( &SPlayer->Player.SoundBuffer, &State ) == D_DS_OK )
			{
				SPlayer->Player.SoundBufferPlayStateBackupFlagValid = TRUE ;

				if( State & D_DSBSTATUS_PLAYING )
				{
					SPlayer->Player.SoundBufferPlayStateBackupFlag = TRUE ;

					// 再生されていたら再生を止める
					SoundBuffer_Stop( &SPlayer->Player.SoundBuffer, TRUE ) ;
				}
				else
				{
					SPlayer->Player.SoundBufferPlayStateBackupFlag = FALSE ;
				}
			}
			else
			{
				SPlayer->Player.SoundBufferPlayStateBackupFlagValid = FALSE ;
			}
		}
	}
	else
	{
		// 再開する場合

		for( List = DSOUND.SoftSoundPlayerListFirst.Next ; List->Next != NULL ; List = List->Next )
		{
			SPlayer = ( SOFTSOUND * )List->Data ;

			// 再生中ではない場合のみ処理
			if( SPlayer->Player.IsPlayFlag )
			{
				if( SPlayer->Player.SoundBuffer.Valid == FALSE || SPlayer->Player.SoundBuffer.DSBuffer == NULL ) continue ;

				// サウンドバッファの再生状態が有効で、且つ再生していた場合は再生を再開する
				if( SPlayer->Player.SoundBufferPlayStateBackupFlagValid &&
					SPlayer->Player.SoundBufferPlayStateBackupFlag )
				{
					SoundBuffer_Play( &SPlayer->Player.SoundBuffer, SPlayer->Player.SoundBuffer.Loop ) ;
				}
			}

			SPlayer->Player.SoundBufferPlayStateBackupFlagValid = FALSE ;
		}
	}

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return 0 ;
}

// ソフトウエアで制御する全てのサウンドデータプレイヤーの定期処理を行う
extern	int ST_SoftSoundPlayerProcessAll( void )
{
	HANDLELIST *List ;
	SOFTSOUND * SPlayer ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	for( List = DSOUND.SoftSoundPlayerListFirst.Next ; List->Next != NULL ; List = List->Next )
	{
		SPlayer = ( SOFTSOUND * )List->Data ;

		// 再生中ではない場合は何もせず次へ
		if( SPlayer->Player.IsPlayFlag == FALSE ) continue ;

		_SoftSoundPlayerProcess( SPlayer ) ;
	}

	// 終了
	return 0 ;
}

// ソフトウエアで制御するサウンドデータプレイヤーの定期処理
static int _SoftSoundPlayerProcess( SOFTSOUND * SPlayer )
{
	int WriteSize, NoneWriteSize, WriteStartPos ;
	DWORD PlayPos, WritePos ;
	HRESULT hr ;
	int Result = -1 ;
	DWORD MoveSize, MoveTempSize ;
	DWORD C, S, P, N ;
	SOUNDBUFFERLOCKDATA LockData ;

	if( DSOUND.InitializeFlag == FALSE ) return -1 ;

	// 再生状態ではない場合は何もしない
	if( SPlayer->Player.IsPlayFlag == FALSE ) return 0 ;

	// クリティカルセクションの取得
	CRITICALSECTION_LOCK( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 再生位置の取得
	SoundBuffer_GetCurrentPosition( &SPlayer->Player.SoundBuffer, &PlayPos, &WritePos ) ;

	// 無音データ再生チェック
	if( SPlayer->Player.NoneDataSetCompOffset != -1 && SPlayer->Player.NoneDataPlayStartFlag == FALSE )
	{
		P = SPlayer->Player.NoneDataPlayCheckBackPlayOffset ;
		N = PlayPos ;
		if( ( N > P && ( P <= ( DWORD )SPlayer->Player.NoneDataSetCompOffset && N >= ( DWORD )SPlayer->Player.NoneDataSetCompOffset ) ) ||
			( N < P && ( P <= ( DWORD )SPlayer->Player.NoneDataSetCompOffset || N >= ( DWORD )SPlayer->Player.NoneDataSetCompOffset ) ) )
		{
			SPlayer->Player.NoneDataPlayStartFlag = TRUE ;
		}
		else
		{
			SPlayer->Player.NoneDataPlayCheckBackPlayOffset = N ;
		}
	}

	// 転送する必要があるかどうかを調べる
	WriteStartPos = ( int )SPlayer->Player.DataSetCompOffset - ( int )SPlayer->Player.MinDataSetSize ;
	if( WriteStartPos < 0 )
	{
		WriteStartPos += SPlayer->Player.SoundBufferSize ;
	}
	C = SPlayer->Player.DataSetCompOffset ;
	S = WriteStartPos ;
	if( ( S > C && ( S > PlayPos && C < PlayPos ) ) ||
		( S < C && ( S > PlayPos || C < PlayPos ) ) )
	{
		Result = 0 ;
		goto END ;
	}

	// 転送するサイズを算出する
	WriteSize = 0 ;
	if( S > C )
	{
		if( S < PlayPos )
		{
			WriteSize = PlayPos - S ;
		}
		else
		{
			WriteSize = ( SPlayer->Player.SoundBufferSize - S ) + PlayPos ;
		}
	}
	else
	{
		WriteSize = PlayPos - S ;
	}
	WriteSize += SPlayer->Player.MinDataSetSize ;
	WriteSize /= SPlayer->BufferFormat.nBlockAlign ;
	if( WriteSize < 0 )
	{
		Result = 0 ;
		goto END ;
	}

	// サウンドバッファに転送する有効データのサイズと無効データのサイズを算出
	NoneWriteSize = 0 ;
	if( WriteSize > SPlayer->Player.StockSampleNum )
	{
		if( SPlayer->Player.StockSampleNum <= 0 )
		{
			NoneWriteSize = WriteSize ;
			WriteSize = 0 ;
		}
		else
		{
			WriteSize = SPlayer->Player.StockSampleNum ;
		}
	}

	// サウンドバッファをロックする
	MoveSize = ( NoneWriteSize + WriteSize ) * SPlayer->BufferFormat.nBlockAlign ;
	hr = SoundBuffer_Lock(
		&SPlayer->Player.SoundBuffer, SPlayer->Player.DataSetCompOffset,
		MoveSize,
		( void ** )&LockData.WriteP,  &LockData.Length,
		( void ** )&LockData.WriteP2, &LockData.Length2 ) ;
	if( hr != D_DS_OK )
		goto END ;

	// 無音データをセットする場合は無音データのセットを開始したオフセットを保存する
	if( NoneWriteSize != 0 && SPlayer->Player.NoneDataSetCompOffset == -1 )
	{
		SPlayer->Player.NoneDataSetCompOffset = SPlayer->Player.DataSetCompOffset + WriteSize * SPlayer->BufferFormat.nBlockAlign ;
		if( SPlayer->Player.NoneDataSetCompOffset > SPlayer->Player.SoundBufferSize )
		{
			SPlayer->Player.NoneDataSetCompOffset -= SPlayer->Player.SoundBufferSize ;
		}
		SPlayer->Player.NoneDataPlayCheckBackPlayOffset = PlayPos ;
		SPlayer->Player.NoneDataPlayStartFlag = FALSE ;
	}

	// 無音データ再生中に有効データを転送する場合は無音データ再生フラグを倒す
	if( SPlayer->Player.NoneDataPlayStartFlag && WriteSize != 0 )
	{
		SPlayer->Player.NoneDataPlayStartFlag = FALSE ;
		SPlayer->Player.NoneDataPlayCheckBackPlayOffset = 0 ;
		SPlayer->Player.NoneDataSetCompOffset = -1 ;
	}

	// 残サンプル数を減らす
	SPlayer->Player.StockSampleNum -= WriteSize ;

	// ロック情報をセット
	LockData.StartOffst = SPlayer->Player.DataSetCompOffset ;
	LockData.Offset = 0 ;
	LockData.Offset2 = 0 ;
	LockData.Valid = LockData.Length ;
	LockData.Valid2 = LockData.Length2 ;

	// 有効データを転送
	if( LockData.Valid != 0 && WriteSize != 0 )
	{
		MoveTempSize = WriteSize * SPlayer->BufferFormat.nBlockAlign ;
		if( MoveTempSize > LockData.Valid )
			MoveTempSize = LockData.Valid ;

		RingBufDataGet( &SPlayer->Player.StockSample, LockData.WriteP + LockData.Offset, MoveTempSize, FALSE ) ;
		LockData.Offset += MoveTempSize ;
		LockData.Valid -= MoveTempSize ;
		WriteSize -= MoveTempSize / SPlayer->BufferFormat.nBlockAlign ;
	}
	if( LockData.Valid2 != 0 && WriteSize != 0 )
	{
		MoveTempSize = WriteSize * SPlayer->BufferFormat.nBlockAlign ;
		if( MoveTempSize > LockData.Valid2 )
			MoveTempSize = LockData.Valid2 ;

		RingBufDataGet( &SPlayer->Player.StockSample, LockData.WriteP2 + LockData.Offset2, MoveTempSize, FALSE ) ;
		LockData.Offset2 += MoveTempSize ;
		LockData.Valid2 -= MoveTempSize ;
		WriteSize -= MoveTempSize / SPlayer->BufferFormat.nBlockAlign ;
	}

	// 無音データを転送
	if( LockData.Valid != 0 && NoneWriteSize != 0 )
	{
		MoveTempSize = NoneWriteSize * SPlayer->BufferFormat.nBlockAlign ;
		if( MoveTempSize > LockData.Valid )
			MoveTempSize = LockData.Valid ;

		switch( SPlayer->BufferFormat.wBitsPerSample )
		{
		case 8  : _MEMSET( LockData.WriteP + LockData.Offset, 127, MoveTempSize ) ; break ;
		case 16 : _MEMSET( LockData.WriteP + LockData.Offset,   0, MoveTempSize ) ; break ;
		}

		LockData.Offset += MoveTempSize ;
		LockData.Valid -= MoveTempSize ;
		NoneWriteSize -= MoveTempSize / SPlayer->BufferFormat.nBlockAlign ;
	}
	if( LockData.Valid2 != 0 && NoneWriteSize != 0 )
	{
		MoveTempSize = NoneWriteSize * SPlayer->BufferFormat.nBlockAlign ;
		if( MoveTempSize > LockData.Valid2 )
			MoveTempSize = LockData.Valid2 ;

		switch( SPlayer->BufferFormat.wBitsPerSample )
		{
		case 8  : _MEMSET( LockData.WriteP2 + LockData.Offset2, 127, MoveTempSize ) ; break ;
		case 16 : _MEMSET( LockData.WriteP2 + LockData.Offset2,   0, MoveTempSize ) ; break ;
		}

		LockData.Offset2 += MoveTempSize ;
		LockData.Valid2 -= MoveTempSize ;
		NoneWriteSize -= MoveTempSize / SPlayer->BufferFormat.nBlockAlign ;
	}

	// サウンドバッファのロックを解除する
	SoundBuffer_Unlock(
		&SPlayer->Player.SoundBuffer,
		LockData.WriteP,  LockData.Length,
		LockData.WriteP2, LockData.Length2 ) ;

	// データセットが完了した位置を変更する
	SPlayer->Player.DataSetCompOffset += MoveSize ;
	if( SPlayer->Player.DataSetCompOffset >= SPlayer->Player.SoundBufferSize )
		SPlayer->Player.DataSetCompOffset -= SPlayer->Player.SoundBufferSize ;

	Result = 0 ;
END :

	// クリティカルセクションの解放
	CriticalSection_Unlock( &HandleManageArray[ DX_HANDLETYPE_SOFTSOUND ].CriticalSection ) ;

	// 終了
	return Result ;
}





































// ＭＩＤＩ控制函数

// ＭＩＤＩハンドルを初期化をする関数
extern int InitializeMidiHandle( HANDLEINFO *HandleInfo )
{
	MIDIHANDLEDATA *MusicData = ( MIDIHANDLEDATA * )HandleInfo ;

	// 音量をセット
	MusicData->Volume = 255 ;

	// 終了
	return 0 ;
}

// ＭＩＤＩハンドルの後始末を行う関数
extern int TerminateMidiHandle( HANDLEINFO *HandleInfo )
{
	MIDIHANDLEDATA *MusicData = ( MIDIHANDLEDATA * )HandleInfo ;

	// 音の再生を止める
	if( NS_CheckMusicMem( HandleInfo->Handle ) == TRUE )
		NS_StopMusicMem( HandleInfo->Handle ) ;

	// DirectMusicSegment8 オブジェクトの解放
	if( MusicData->DirectMusicSegmentObject != NULL )
	{
		// 音色データの解放
		MusicData->DirectMusicSegmentObject->Unload( DSOUND.DirectMusicPerformanceObject ) ;

		// オブジェクトの解放
		DSOUND.DirectMusicLoaderObject->ReleaseObjectByUnknown( MusicData->DirectMusicSegmentObject ) ;
		MusicData->DirectMusicSegmentObject->Release() ;
		MusicData->DirectMusicSegmentObject = NULL ;
	}

	// ＭＩＤＩデータの解放
	if( MusicData->DataImage != NULL )
	{
		_MEMSET( MusicData->DataImage, 0, MusicData->DataSize );
		DXFREE( MusicData->DataImage ) ;
		MusicData->DataImage = NULL ;
	}

	// 終了
	return 0 ;
}

// 新しいＭＩＤＩハンドルを取得する
extern int NS_AddMusicData( void )
{
	return AddHandle( DX_HANDLETYPE_MUSIC ) ;
}

// ＭＩＤＩハンドルを削除する
extern int NS_DeleteMusicMem( int MusicHandle )
{
	return SubHandle( MusicHandle ) ;
}

// LoadMusicMemByMemImage の実処理関数
static int LoadMusicMemByMemImage_Static(
	int MusicHandle,
	const void *FileImage,
	int FileImageSize,
	int ASyncThread
)
{
	MIDIHANDLEDATA *MusicData ;

	if( ASyncThread )
	{
		if( MIDI_MASKHCHK_ASYNC( MusicHandle, MusicData ) )
			return -1 ;
	}
	else
	{
		if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
			return -1 ;
	}

	// イメージのコピーを作成
	MusicData->DataImage = DXALLOC( FileImageSize ) ;
	if( MusicData->DataImage == NULL )
	{
		DXST_ERRORLOG_ADD( _T( "ミュージックデータを一時的に保存しておくメモリ領域の確保に失敗しました\n" ) ) ;
		return -1 ;
	}
	_MEMCPY( MusicData->DataImage, FileImage, FileImageSize ) ;
	MusicData->DataSize = FileImageSize ;

	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		break ;

	case DX_MIDIMODE_DM :
		// DirectMusic を使用する場合はメモリから DirectMusicSegment8 を作成する
		{
			D_DMUS_OBJECTDESC ObjDesc ;

			_MEMSET( &ObjDesc, 0, sizeof( ObjDesc ) ) ;
			ObjDesc.dwSize = sizeof( D_DMUS_OBJECTDESC ) ;
			ObjDesc.dwValidData = D_DMUS_OBJ_MEMORY | D_DMUS_OBJ_CLASS ;
			ObjDesc.guidClass = CLSID_DIRECTMUSICSEGMENT ;
			ObjDesc.pbMemData = (BYTE *)MusicData->DataImage ;
			ObjDesc.llMemLength = MusicData->DataSize ;

			DSOUND.DirectMusicLoaderObject->ClearCache( IID_IDIRECTMUSICSEGMENT8 );
//			if( DSOUND.DirectMusicLoaderObject->GetObject( &ObjDesc, IID_IDirectMusicSegment8, ( void ** )&MusicData->DirectMusicSegmentObject ) != S_OK )
			if( DSOUND.DirectMusicLoaderObject->GetObject( &ObjDesc, IID_IDIRECTMUSICSEGMENT8, ( void ** )&MusicData->DirectMusicSegmentObject ) != S_OK )
			{
				DXST_ERRORLOG_ADD( _T( "ミュージックデータから DirectMusicSegment8 を取得する処理が失敗しました\n" ) ) ;
				return -1 ;
			}

			// データ形式を MIDI にセット
			MusicData->DirectMusicSegmentObject->SetParam( GUID_STANDARDMIDIFILE, 0xFFFFFFFF, 0, 0, NULL);

			// 音色データのダウンロード
			MusicData->DirectMusicSegmentObject->Download( DSOUND.DirectMusicPerformanceObject ) ;
		}
		break ;
	}

	// 再生中フラグを倒す
	MusicData->PlayFlag = FALSE ;

	// 終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadMusicMemByMemImage の非同期読み込みスレッドから呼ばれる関数
static void LoadMusicMemByMemImage_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MusicHandle ;
	const void *FileImage ;
	int FileImageSize ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MusicHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileImage = GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	FileImageSize = GetASyncLoadParamInt( AParam->Data, &Addr ) ;

	Result = LoadMusicMemByMemImage_Static( MusicHandle, FileImage, FileImageSize, TRUE ) ;

	DecASyncLoadCount( MusicHandle ) ;
	if( Result < 0 )
	{
		SubHandle( MusicHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// LoadMusicMemByMemImage のグローバル変数にアクセスしないバージョン
extern int LoadMusicMemByMemImage_UseGParam(
	const void *FileImage,
	int FileImageSize,
	int ASyncLoadFlag
)
{
	int MusicHandle ;

	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	MusicHandle = AddHandle( DX_HANDLETYPE_MUSIC ) ;
	if( MusicHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, MusicHandle ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, FileImage ) ;
		AddASyncLoadParamInt( NULL, &Addr, FileImageSize ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadMusicMemByMemImage_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MusicHandle ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, FileImage ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileImageSize ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MusicHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadMusicMemByMemImage_Static( MusicHandle, FileImage, FileImageSize, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return MusicHandle ;

ERR :
	SubHandle( MusicHandle ) ;

	return -1 ;
}

// メモリ上に展開されたＭＩＤＩファイルを読み込む
extern int NS_LoadMusicMemByMemImage( const void *FileImageBuffer, int FileImageSize )
{
	return LoadMusicMemByMemImage_UseGParam( FileImageBuffer, FileImageSize, GetASyncLoadFlag() ) ;
}

// LoadMusicMem の実処理関数
static int LoadMusicMem_Static(
	int MusicHandle,
	const TCHAR *FileName,
	int ASyncThread
)
{
	MIDIHANDLEDATA * MusicData ;
	DWORD_PTR fp = 0 ;
	void *Buffer = NULL ;
	size_t FileSize ;
	int Ret ;

	if( ASyncThread )
	{
		if( MIDI_MASKHCHK_ASYNC( MusicHandle, MusicData ) )
			return -1 ;
	}
	else
	{
		if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
			return -1 ;
	}

	// ファイルの読み込み
	{
		fp = FOPEN( FileName ) ;
		if( fp == 0 )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "ミュージックファイル %s の読み込みに失敗しました in LoadMusicMem" ), FileName )) ;
			goto ERR ;
		}
		FSEEK( fp, 0L, SEEK_END ) ;
		FileSize = ( size_t )FTELL( fp ) ;
		FSEEK( fp, 0L, SEEK_SET ) ;
		Buffer = DXALLOC( FileSize ) ;
		if( Buffer == NULL )
		{
			DXST_ERRORLOGFMT_ADD(( _T( "ミュージックファイル %s を読み込むメモリ領域の確保に失敗しました in LoadMusicMem" ), FileName )) ;
			goto ERR ;
		}
		FREAD( Buffer, FileSize, 1, fp ) ;
		FCLOSE( fp ) ;
		fp = 0 ;
	}

	// ハンドルのセットアップ
	Ret = LoadMusicMemByMemImage_Static( MusicHandle, Buffer, ( int )FileSize, ASyncThread ) ;

	// メモリの解放
	DXFREE( Buffer ) ;

	// 結果を返す
	return Ret ;

ERR :
	if( fp != 0 ) FCLOSE( fp ) ;
	if( Buffer != NULL ) DXFREE( Buffer ) ;

	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadMusicMem の非同期読み込みスレッドから呼ばれる関数
static void LoadMusicMem_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MusicHandle ;
	const TCHAR *FileName ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MusicHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = LoadMusicMem_Static( MusicHandle, FileName, TRUE ) ;

	DecASyncLoadCount( MusicHandle ) ;
	if( Result < 0 )
	{
		SubHandle( MusicHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// LoadMusicMem のグローバル変数にアクセスしないバージョン
extern int LoadMusicMem_UseGParam(
	const TCHAR *FileName,
	int ASyncLoadFlag
)
{
	int MusicHandle ;

	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	MusicHandle = AddHandle( DX_HANDLETYPE_MUSIC ) ;
	if( MusicHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;
		TCHAR FullPath[ 1024 ] ;

		ConvertFullPathT_( FileName, FullPath ) ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, MusicHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FullPath ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadMusicMem_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MusicHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FullPath ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MusicHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadMusicMem_Static( MusicHandle, FileName, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return MusicHandle ;

ERR :
	SubHandle( MusicHandle ) ;

	return -1 ;
}

// ＭＩＤＩファイルを読み込む
extern int NS_LoadMusicMem( const TCHAR *FileName )
{
	return LoadMusicMem_UseGParam( FileName, GetASyncLoadFlag() ) ;
}

// LoadMusicMemByResource の実処理関数
static int LoadMusicMemByResource_Static(
	int MusicHandle,
	const TCHAR *ResourceName,
	const TCHAR *ResourceType,
	int ASyncThread
)
{
	MIDIHANDLEDATA * MusicData ;
	void *Image ;
	int ImageSize ;

	if( ASyncThread )
	{
		if( MIDI_MASKHCHK_ASYNC( MusicHandle, MusicData ) )
			return -1 ;
	}
	else
	{
		if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
			return -1 ;
	}

	// リソースの情報を取得
	if( NS_GetResourceInfo( ResourceName, ResourceType, &Image, &ImageSize ) < 0 )
		return -1 ;

	// ハンドルのセットアップ
	return LoadMusicMemByMemImage_Static( MusicHandle, Image, ImageSize, ASyncThread ) ;
}

#ifndef DX_NON_ASYNCLOAD
// LoadMusicMemByResource の非同期読み込みスレッドから呼ばれる関数
static void LoadMusicMemByResource_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MusicHandle ;
	const TCHAR *ResourceName ;
	const TCHAR *ResourceType ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MusicHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	ResourceName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	ResourceType = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = LoadMusicMemByResource_Static( MusicHandle, ResourceName, ResourceType, TRUE ) ;

	DecASyncLoadCount( MusicHandle ) ;
	if( Result < 0 )
	{
		SubHandle( MusicHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// LoadMusicMemByResource のグローバル変数にアクセスしないバージョン
extern int LoadMusicMemByResource_UseGParam(
	const TCHAR *ResourceName,
	const TCHAR *ResourceType,
	int ASyncLoadFlag
)
{
	int MusicHandle ;

	if( DSOUND.InitializeFlag == FALSE )
		return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// ハンドルの作成
	MusicHandle = AddHandle( DX_HANDLETYPE_MUSIC ) ;
	if( MusicHandle == -1 ) return -1 ;

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt( NULL, &Addr, MusicHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, ResourceName ) ;
		AddASyncLoadParamString( NULL, &Addr, ResourceType ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = LoadMusicMemByResource_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MusicHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, ResourceName ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, ResourceType ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MusicHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( LoadMusicMemByResource_Static( MusicHandle, ResourceName, ResourceType, FALSE ) < 0 )
			goto ERR ;
	}

	// 終了
	return MusicHandle ;

ERR :
	SubHandle( MusicHandle ) ;

	return -1 ;
}

// リソース上のＭＩＤＩファイルを読み込む
extern int NS_LoadMusicMemByResource( const TCHAR *ResourceName, const TCHAR *ResourceType )
{
	return LoadMusicMemByResource_UseGParam( ResourceName, ResourceType, GetASyncLoadFlag() ) ;
}

// 読み込んだＭＩＤＩデータの演奏を開始する
extern int NS_PlayMusicMem( int MusicHandle, int PlayType )
{
	MIDIHANDLEDATA * MusicData ;
	int Result ;
	MCI_OPEN_PARMS		mciOpenParms;
	MCI_PLAY_PARMS		mciPlayParms;
	MCI_STATUS_PARMS	mciStatusParms;
	HANDLE FileHandle ;
	DWORD WriteSize ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	// エラー判定
	if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
		return -1 ;

	// 演奏を停止する
	NS_StopMusicMem( MIDI.PlayHandle ) ;

	Result = 0 ;
	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		// 以前のテンポラリファイルが残っているかもしれないので、一応削除
		DeleteFile( MIDI.FileName ) ;

		// 打开临时文件
		FileHandle = CreateTemporaryFile( MIDI.FileName ) ;
		if( FileHandle == NULL ) return -1 ;

		// テンポラリファイルにデータを書き込む
		WriteFile( FileHandle, MusicData->DataImage, MusicData->DataSize, &WriteSize, NULL ) ;
		CloseHandle( FileHandle ) ;

		// オープンステータスセット
		mciOpenParms.lpstrElementName = MIDI.FileName;
		mciOpenParms.lpstrDeviceType = _T( "sequencer" );

		// ＭＣＩのオープン
		Result = WinAPIData.Win32Func.mciSendCommandFunc( 0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, ( DWORD_PTR )( LPVOID )&mciOpenParms ) ;
		if( Result != 0 )
		{
			LOADSOUND_GPARAM GParam ;
			int OldHandle ;

//			DXST_ERRORLOG_ADD( _T( "ＭＣＩのオープンに失敗しました\n" ) ) ;

			// エラーが起きたら一時ファイルを削除
			DeleteFile( MIDI.FileName ) ;

			// 演奏に失敗したら普通のサウンドファイルの可能性がある
			OldHandle = MIDI.DefaultHandle ;
			InitLoadSoundGParam( &GParam ) ;
			GParam.CreateSoundDataType = DX_SOUNDDATATYPE_MEMPRESS ;
			MIDI.DefaultHandle = LoadSoundMemByMemImageBase_UseGParam( &GParam, TRUE, -1, MusicData->DataImage, MusicData->DataSize, 1, -1, 0 ) ;
			if( MIDI.DefaultHandle == -1 )
			{
				// それでも失敗したらファイルがないということ
				MIDI.DefaultHandle = 0;
				return -1 ;
			}

			// 音量設定
			NS_ChangeVolumeSoundMem( MusicData->Volume, MIDI.DefaultHandle ) ;

			// 今までのハンドルは削除
			if( OldHandle != 0 )
			{
				if( MIDI.DefaultHandleToSoundHandleFlag == TRUE )	NS_DeleteSoundMem( OldHandle ) ;
				else												NS_DeleteMusicMem( OldHandle ) ;
				OldHandle = 0 ;
			}
			MIDI.DefaultHandleToSoundHandleFlag = TRUE ;

			// 再生開始
			NS_PlaySoundMem( MIDI.DefaultHandle, PlayType ) ;
			return 0 ;
		}

		// ＭＩＤＩのＩＤを保存
		MIDI.MidiID = mciOpenParms.wDeviceID ;

		// ＭＩＤＩマッパーか判定
		mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT ;
		if( WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID , MCI_STATUS , MCI_STATUS_ITEM , ( DWORD_PTR )( LPVOID )&mciStatusParms ) )
		{
			WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID , MCI_CLOSE , 0 , 0 ) ;
			DXST_ERRORLOG_ADD( _T( "ＭＩＤＩ演奏開始処理でエラーが起きました１\n" ) ) ;
			goto MCI_ERROR ;
		}
		if( LOWORD( mciStatusParms.dwReturn ) != LOWORD( MIDI_MAPPER ) )
		{
			WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID , MCI_CLOSE , 0 , 0 ) ;
			DXST_ERRORLOG_ADD( _T( "ＭＩＤＩ演奏開始処理でエラーが起きました２\n" ) ) ;
			goto MCI_ERROR ;
		}

		// コールバック対象をメインウインドウに設定して演奏開始
		mciPlayParms.dwCallback = ( DWORD_PTR )NS_GetMainWindowHandle() ;
		if( WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID , MCI_PLAY , MCI_NOTIFY , ( DWORD_PTR )( LPVOID )&mciPlayParms ) )
		{
			WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID , MCI_CLOSE , 0 , 0 ) ;
			DXST_ERRORLOG_ADD( _T( "ＭＩＤＩ演奏に失敗しました\n" ) ) ;
			goto MCI_ERROR ;
		}

		// テンポラリファイルから再生していることを示すフラグを立てる
		MIDI.MemImagePlayFlag = TRUE ;
		break ;

	case DX_MIDIMODE_DM :
		{
			D_REFERENCE_TIME RTime ;
			HRESULT hr ;

			// ループ設定
			MusicData->DirectMusicSegmentObject->SetRepeats( PlayType == DX_PLAYTYPE_LOOP ? D_DMUS_SEG_REPEAT_INFINITE : 0 ) ;

			// 演奏開始
			hr = DSOUND.DirectMusicPerformanceObject->PlaySegmentEx( 
				MusicData->DirectMusicSegmentObject,	// 演奏するセグメント。
				NULL,									// ソングに使用するパラメータ。実装されていない。
				NULL,									// トランジションに関するパラメータ。
				D_DMUS_SEGF_REFTIME,					// フラグ。
				0,										// 開始タイム。0は直ちに開始。
				NULL,									// セグメント状態を受け取るポインタ。
				NULL,									// 停止するオブジェクト。
				NULL									// デフォルトでない場合はオーディオパス。
			) ;

			// 演奏開始時間の保存
			DSOUND.DirectMusicPerformanceObject->GetTime( &RTime, NULL ) ;
			MusicData->StartTime = _DTOL( (double)RTime / 10000.0 ) ;
		}
		// テンポラリファイルから再生していることを示すフラグを倒す
		MIDI.MemImagePlayFlag = FALSE ;
		break ;
	}

	MIDI.PlayFlag			 = TRUE ;								// 演奏フラグを立てる
	MIDI.PlayHandle			 = MusicHandle ;						// 演奏しているハンドルの更新
	MIDI.LoopFlag			 = PlayType == DX_PLAYTYPE_LOOP ;		// ループフラグをセットする

	MusicData->PlayFlag		 = TRUE ;								// 状態を再生中にする
	MusicData->PlayStartFlag = FALSE ;								// 演奏が開始されたかフラグを倒す

	// 演奏終了まで待つ指定の場合はここで待つ
	if( PlayType == DX_PLAYTYPE_NORMAL )
	{
		// ＭＣＩ再生の場合は演奏が開始されるまで待つ
//		if( DSOUND.SoundMode == DX_MIDIMODE_MCI )
//			while( NS_ProcessMessage() == 0 && CheckMusicMCI() == FALSE ){}

		// 再生終了まで待つ
		while( NS_ProcessMessage() == 0 && NS_CheckMusicMem( MusicHandle ) == TRUE ){}

		// テンポラリファイルから再生されていた場合は削除する
		if( MIDI.MemImagePlayFlag == TRUE )
			DeleteFile( MIDI.FileName ) ;
		MIDI.MemImagePlayFlag = FALSE ;
	}

	// 終了
	return 0 ;

MCI_ERROR:
	// エラーが起きたら一時ファイルを削除して終了
	DeleteFile( MIDI.FileName ) ;

	// エラー終了
	return -1 ;
}

// ＭＩＤＩデータの演奏を停止する
extern int NS_StopMusicMem( int MusicHandle )
{
	int i ;
	MIDIHANDLEDATA * MusicData ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
		return -1 ;

	// サウンドハンドルの再生の場合はサウンドを止める
	if( MIDI.DefaultHandle != 0 && MIDI.DefaultHandleToSoundHandleFlag == TRUE )
	{
		NS_StopSoundMem( MIDI.DefaultHandle ) ;
		NS_DeleteSoundMem( MIDI.DefaultHandle ) ;

		MIDI.DefaultHandle = 0 ;
		return 0 ;
	}

	// ループフラグを倒す
	MIDI.LoopFlag = FALSE ;

	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		// 演奏終了関数を呼ぶ
		MidiCallBackProcess() ;

		// ウエイト
		for( i = 0 ; i <= 4 ; i++ ) NS_ProcessMessage() ;
		break ;

	case DX_MIDIMODE_DM :
		// 演奏を停止する
		DSOUND.DirectMusicPerformanceObject->StopEx( MusicData->DirectMusicSegmentObject, 0, 0 ) ;
		break ;
	}

	// テンポラリファイルから再生されていた場合は削除する
	if( MIDI.MemImagePlayFlag == TRUE )
	{
		DeleteFile( MIDI.FileName ) ;
		MIDI.MemImagePlayFlag = FALSE ;
	}

	// 状態を停止中にする
	MusicData->PlayFlag = FALSE ;

	// 演奏が開始されたかフラグを倒す
	MusicData->PlayStartFlag = FALSE ;

	// 終了
	return 0 ;
}

// ＭＩＤＩデータが演奏中かどうかを取得する( TRUE:演奏中  FALSE:停止中 )
extern int NS_CheckMusicMem( int MusicHandle )
{
	MIDIHANDLEDATA * MusicData ;
	int Result = -1 ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
		return -1 ;

	// サウンドハンドルの再生の場合はサウンドの再生状態を返す
	if( MIDI.DefaultHandle != 0 && MIDI.DefaultHandleToSoundHandleFlag == TRUE )
	{
		return NS_CheckSoundMem( MIDI.DefaultHandle ) ;
	}

	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		Result = MIDI.PlayFlag ;
		break ;

	case DX_MIDIMODE_DM :
		Result = MusicData->PlayFlag ;
	}

	return Result ;
}

// ＭＩＤＩデータの再生音量をセットする
extern int NS_SetVolumeMusicMem( int Volume, int MusicHandle )
{
	MIDIHANDLEDATA * MusicData ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
		return -1 ;

	// 音量をセット
	MusicData->Volume = Volume ;

	// サウンドハンドルの再生の場合はサウンドの音量を変更する
	if( MIDI.DefaultHandle != 0 && MIDI.DefaultHandleToSoundHandleFlag == TRUE )
	{
		return NS_ChangeVolumeSoundMem( Volume, MIDI.DefaultHandle );
	}

	return NS_SetVolumeMusic( Volume )  ;
}

// ＭＩＤＩデータハンドルをすべて削除する
extern int NS_InitMusicMem( void )
{
	return AllHandleSub( DX_HANDLETYPE_MUSIC ) ;
}

// ＭＩＤＩデータの周期的処理
extern int NS_ProcessMusicMem( void )
{
	MIDIHANDLEDATA * MusicData ;
	int i, play ;

	if( DSOUND.DirectSoundObject == NULL )
		return -1 ;

	if( HandleManageArray[ DX_HANDLETYPE_MUSIC ].InitializeFlag == FALSE )
		return -1 ;

	for( i = HandleManageArray[ DX_HANDLETYPE_MUSIC ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MUSIC ].AreaMax ; i ++ )
	{
		MusicData = ( MIDIHANDLEDATA * )HandleManageArray[ DX_HANDLETYPE_MUSIC ].Handle[ i ] ;
		if( MusicData == NULL ) continue ;

		if( MusicData->PlayFlag == FALSE ) continue ;

		switch( DSOUND.SoundMode )
		{
		case DX_MIDIMODE_MCI :
			break ;

		case DX_MIDIMODE_DM :
			play = DSOUND.DirectMusicPerformanceObject->IsPlaying( MusicData->DirectMusicSegmentObject , NULL ) != S_FALSE ;
			if( MusicData->PlayStartFlag == FALSE )
			{
				// まだ演奏が始まっていなかった場合は状態が演奏中になったことで
				// 初めて演奏中ということになる
				if( play == TRUE ) MusicData->PlayStartFlag = TRUE ;
			}
			else
			{
				// 演奏が始まったあとは現在の状態がそのまま反映される
				MusicData->PlayFlag = play ;
			}
			break ;
		}
	}

	// 終了
	return 0;
}

// ＭＩＤＩデータの現在の再生位置を取得する
extern int NS_GetMusicMemPosition( int MusicHandle )
{
	MIDIHANDLEDATA * MusicData ;
	int Result = -1 ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( MIDI_MASKHCHK( MusicHandle, MusicData ) )
		return -1 ;

	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		Result = NS_GetMusicPosition() ;
		break ;

	case DX_MIDIMODE_DM :
		{
			D_REFERENCE_TIME RTime ;
			D_MUSIC_TIME Time ;
			D_IDirectMusicSegmentState *State ;

			DSOUND.DirectMusicPerformanceObject->GetTime( &RTime, &Time ) ;
			if( DSOUND.DirectMusicPerformanceObject->GetSegmentState( &State, Time ) != S_OK )
				return -1 ;

			State->GetSeek( &Time ) ;
			State->Release() ;

			// 時間を返す
			Result = _DTOL( (double)RTime / 10000.0 ) - MusicData->StartTime ;
		}
		break ;
	}

	return Result ;
}




// ＭＩＤＩファイルを再生する
extern int NS_PlayMusic( const TCHAR *FileName , int PlayType )
{
	// もし演奏中だったら止める
	if( MIDI.DefaultHandle != 0 )
	{
		if( MIDI.DefaultHandleToSoundHandleFlag == TRUE )	NS_DeleteSoundMem( MIDI.DefaultHandle ) ;
		else												NS_DeleteMusicMem( MIDI.DefaultHandle ) ;
		MIDI.DefaultHandle = 0 ;
	}

	// 读取
	MIDI.DefaultHandle = LoadMusicMem_UseGParam( FileName, FALSE ) ;
	if( MIDI.DefaultHandle == -1 )
	{
		MIDI.DefaultHandle = 0;
		return -1 ;
	}
	MIDI.DefaultHandleToSoundHandleFlag = FALSE ;

	// 演奏
	if( NS_PlayMusicMem( MIDI.DefaultHandle, PlayType ) == -1 )
	{
		LOADSOUND_GPARAM GParam ;

		// 演奏に失敗したら普通のサウンドファイルの可能性がある
		NS_DeleteMusicMem( MIDI.DefaultHandle ) ;

		InitLoadSoundGParam( &GParam ) ;
		GParam.CreateSoundDataType = DX_SOUNDDATATYPE_MEMPRESS ;
		MIDI.DefaultHandle = LoadSoundMemBase_UseGParam( &GParam, FileName, 1, -1, FALSE, FALSE ) ;
		if( MIDI.DefaultHandle == -1 )
		{
			// それでも失敗したらファイルがないということ
			MIDI.DefaultHandle = 0;
			return -1 ;
		}

		// 再生開始
		NS_PlaySoundMem( MIDI.DefaultHandle, PlayType ) ;
		MIDI.DefaultHandleToSoundHandleFlag = TRUE ;
	}

	return 0;
}





// メモリ上に展開されているＭＩＤＩファイルを演奏する
extern int NS_PlayMusicByMemImage( const void *FileImageBuffer, int FileImageSize, int PlayType )
{
	// もし演奏中だったら止める
	if( MIDI.DefaultHandle != 0 )
	{
		if( MIDI.DefaultHandleToSoundHandleFlag == TRUE )	NS_DeleteSoundMem( MIDI.DefaultHandle ) ;
		else												NS_DeleteMusicMem( MIDI.DefaultHandle ) ;
		MIDI.DefaultHandle = 0 ;
	}

	// 读取
	MIDI.DefaultHandle = LoadMusicMemByMemImage_UseGParam( FileImageBuffer, FileImageSize, FALSE ) ;
	if( MIDI.DefaultHandle == -1 )
	{
		LOADSOUND_GPARAM GParam ;

		// 読み込みに失敗したら音声として再生する
		InitLoadSoundGParam( &GParam ) ;
		MIDI.DefaultHandle = LoadSoundMemByMemImageBase_UseGParam( &GParam, TRUE, -1, FileImageBuffer, FileImageSize, 1, -1, FALSE, FALSE ) ;
		if( MIDI.DefaultHandle == -1 )
		{
			// それでも失敗したらデータが壊れているということ
			MIDI.DefaultHandle = 0;
			return -1 ;
		}
		MIDI.DefaultHandleToSoundHandleFlag = TRUE ;
	}
	else
	{
		MIDI.DefaultHandleToSoundHandleFlag = FALSE ;
	}

	// 演奏
	NS_PlayMusicMem( MIDI.DefaultHandle, PlayType ) ;

	return 0;
}

// リソースからＭＩＤＩファイルを読み込んで演奏する
extern int NS_PlayMusicByResource( const TCHAR *ResourceName, const TCHAR *ResourceType, int PlayType )
{
	void *Image ;
	int ImageSize ;

	// リソースの情報を取得
	if( NS_GetResourceInfo( ResourceName, ResourceType, &Image, &ImageSize ) < 0 )
		return -1 ;

	// 演奏開始
	return NS_PlayMusicByMemImage( Image, ImageSize, PlayType ) ;
}

// ＭＩＤＩファイルの演奏停止
extern int NS_StopMusic( void )
{
	if( MIDI.DefaultHandle == 0 ) return 0 ;

	if( MIDI.DefaultHandleToSoundHandleFlag == TRUE )
	{
		NS_StopSoundMem( MIDI.DefaultHandle ) ;
		NS_DeleteSoundMem( MIDI.DefaultHandle ) ;
	}
	else
	{
		NS_StopMusicMem( MIDI.DefaultHandle ) ;
		NS_DeleteMusicMem( MIDI.DefaultHandle ) ;
	}

	MIDI.DefaultHandle = 0 ;

	return 0 ;
}



// ＭＩＤＩファイルが演奏中か否か情報を取得する
extern int NS_CheckMusic( void )
{
	if( MIDI.DefaultHandle == 0 ) return 0 ;

	if( MIDI.DefaultHandleToSoundHandleFlag == TRUE )	return NS_CheckSoundMem( MIDI.DefaultHandle ) ;
	else												return NS_CheckMusicMem( MIDI.DefaultHandle ) ;
}

// ＭＩＤＩ演奏終了時呼ばれるコールバック関数
extern int MidiCallBackProcess( void )
{
	MCI_PLAY_PARMS		mciPlayParms;

	// ループ指定がある場合再び演奏を開始する
	if( MIDI.LoopFlag == TRUE )
	{
		mciPlayParms.dwCallback = (DWORD_PTR) NS_GetMainWindowHandle() ;
		mciPlayParms.dwFrom		= 0 ;
		WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID , MCI_PLAY, MCI_NOTIFY | MCI_FROM, (DWORD_PTR)(LPVOID) &mciPlayParms );

		MIDI.PlayFlag = TRUE ;
	}
	else 
	{
		// 演奏中だった場合は止める
		if( MIDI.PlayFlag == TRUE )
		{
			WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID , MCI_CLOSE, 0, 0 );
			MIDI.PlayFlag = FALSE;
		}
	}

	return 0 ;
}


// ＭＩＤＩの再生形式をセットする
extern int NS_SelectMidiMode( int Mode )
{
	// もしモードが今までと同じ場合はなにもせず終了
	if( Mode == DSOUND.SoundMode ) return 0 ;

	// 再生モードをセット
	DSOUND.SoundMode = Mode ;

	if( DSOUND.DirectSoundObject == NULL ) return 0 ;

	// オブジェクトを解放
	TerminateDirectSound() ;

	// 音再生系初期化
	return InitializeDirectSound() ;
}

// ＭＩＤＩの再生音量をセットする
extern int NS_SetVolumeMusic( int Volume )
{
	long V ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	if( MIDI.DefaultHandleToSoundHandleFlag == FALSE )
	{
		switch( DSOUND.SoundMode )
		{
		case DX_MIDIMODE_MCI :
			// ＭＣＩの場合は音量の変更は出来ない
			break ;

		case DX_MIDIMODE_DM :
			// 音量のセット
			V = _DTOL( ( double )( DSOUND.MaxVolume - DM_MIN_VOLUME ) / 256 * Volume ) + DM_MIN_VOLUME ;
			if( Volume == 255 ) V = DSOUND.MaxVolume ;
			DSOUND.DirectMusicPerformanceObject->SetGlobalParam( GUID_PERFMASTERVOLUME , &V , sizeof( long ) ) ;
			
			break ;
		}
	}
	else
	{
		NS_ChangeVolumeSoundMem( Volume, MIDI.DefaultHandle );
	}

	// 終了
	return 0 ;
}

// ＭＩＤＩの現在の再生位置を取得する
extern int NS_GetMusicPosition( void )
{
	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// 再生中ではなかったら何もしない
	if( NS_CheckMusic() == FALSE ) return -1 ;

	switch( DSOUND.SoundMode )
	{
	case DX_MIDIMODE_MCI :
		{
			MCI_SET_PARMS mciSetParms ;
			MCI_STATUS_PARMS mciStatusParms ;

			// 取得する時間の単位をミリ秒単位にする
			_MEMSET( &mciSetParms, 0, sizeof( mciSetParms ) ) ;
			mciSetParms.dwTimeFormat = MCI_FORMAT_MILLISECONDS ;
			if( WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD_PTR)&mciSetParms ) != 0 )
				return -1 ;

			// 時間を取得する
			_MEMSET( &mciStatusParms, 0, sizeof( mciStatusParms ) ) ;
			mciStatusParms.dwItem = MCI_STATUS_POSITION ;
			if( WinAPIData.Win32Func.mciSendCommandFunc( MIDI.MidiID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD_PTR)&mciStatusParms ) != 0 )
				return -1 ;

			// 時間を返す
			return ( int )mciStatusParms.dwReturn ;
		}
		break ;

	case DX_MIDIMODE_DM :
		{
			D_REFERENCE_TIME RTime ;
			D_MUSIC_TIME /*StartTime,*/ Time ;
			D_IDirectMusicSegmentState *State ;

			DSOUND.DirectMusicPerformanceObject->GetTime( &RTime, &Time ) ;
			if( DSOUND.DirectMusicPerformanceObject->GetSegmentState( &State, Time ) != S_OK )
				return -1 ;

			State->GetSeek( &Time ) ;
			State->Release() ;

//			DSOUND.DirectMusicPerformanceObject->MusicToReferenceTime( Time, &RTime ) ;

			// 時間を返す
//			return (int)Time ;
			return _DTOL( (double)RTime / 10000.0 ) - MIDI.StartTime ;
		}
		break ;
	}

	// 終了
	return 0 ;
}
	





// 補助系

// 音声フォーマットと波形イメージからＷＡＶＥファイルイメージを作成する
extern	int CreateWaveFileImage( 	void **DestBufferP, int *DestBufferSizeP,
									const WAVEFORMATEX *Format, int FormatSize,
									const void *WaveData, int WaveDataSize )
{
	int BufferSize ;
	void *Buffer ;
	BYTE *p ;
	
	// バッファを確保
	BufferSize = FormatSize + WaveDataSize
					+ 12/*"RIFF" + ファイルサイズ + "WAVE"*/
					+ 8 * 2/*"fmt "チャンク + "data"チャンク*/ ;
	Buffer = DXALLOC( BufferSize ) ;
	if( Buffer == NULL )
	{
		DXST_ERRORLOG_ADD( _T( "仮WAVEファイルイメージ用のメモリ確保に失敗しました\n" ) ) ;
		return -1 ;
	}
	
	// データのセット
	p = (BYTE *)Buffer ;
	_MEMCPY( (char *)p, "RIFF", 4 ) ;		p += 4 ;
	*((DWORD *)p) = BufferSize - 8 ;		p += 4 ;
	_MEMCPY( (char *)p, "WAVE", 4 ) ;		p += 4 ;

	_MEMCPY( (char *)p, "fmt ", 4 ) ;		p += 4 ;
	*((DWORD *)p) = FormatSize ;			p += 4 ;
	_MEMCPY( p, Format, FormatSize ) ;		p += FormatSize ;

	_MEMCPY( (char *)p, "data", 4 ) ;		p += 4 ;
	*((DWORD *)p) = WaveDataSize ;			p += 4 ;
	_MEMCPY( p, WaveData, WaveDataSize ) ;	p += WaveDataSize ;

	// 情報を保存
	*DestBufferP = Buffer ;
	*DestBufferSizeP = BufferSize ;

	// 終了
	return 0 ;
}

// ファイルを丸まるメモリに読み込む
static	int FileFullRead( const TCHAR *FileName, void **BufferP, int *SizeP )
{
	DWORD_PTR fp = 0 ;
	size_t Size ;
	void *Buffer ;
	
	fp = FOPEN( FileName ) ;
	if( fp == 0 )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "右記のファイルのオープンに失敗しました：%s" ), FileName )) ;
		goto ERR ;
	}

	FSEEK( fp, 0, SEEK_END ) ;
	Size = ( size_t )FTELL( fp ) ;
	FSEEK( fp, 0, SEEK_SET ) ;
	
	Buffer = DXALLOC( Size ) ;
	if( Buffer == NULL )
	{
		DXST_ERRORLOGFMT_ADD(( _T( "右記のファイルのロードに必要なメモリの確保に失敗しました：%s" ) , FileName )) ;
		goto ERR ;
	}
	
	FREAD( Buffer, Size, 1, fp ) ;
	FCLOSE( fp ) ;
	
	*BufferP = Buffer ;
	*SizeP = ( int )Size ;
	
	// 終了
	return 0 ;
	
ERR :
	if( fp != 0 ) FCLOSE( fp ) ;
	
	return -1 ;
}

// ストリームデータを丸まるメモリに読み込む
extern	int StreamFullRead( STREAMDATA *Stream, void **BufferP, int *SizeP )
{
	size_t Size ;
	void *Buffer ;
	
	Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_END ) ;
	Size = ( size_t )Stream->ReadShred.Tell( Stream->DataPoint ) ;
	Stream->ReadShred.Seek( Stream->DataPoint, 0, SEEK_SET ) ;
	
	Buffer = DXALLOC( Size ) ;
	if( Buffer == NULL )
		return DXST_ERRORLOGFMT_ADD(( _T( "ストリームの読み込みに必要なメモリの確保に失敗しました" ) )) ;

	Stream->ReadShred.Read( Buffer, Size, 1, Stream->DataPoint ) ;
	
	*BufferP = Buffer ;
	*SizeP = ( int )Size ;
	
	// 終了
	return 0 ;
}

// ソフトウエアで扱う波形データからサウンドハンドルを作成する
extern int NS_LoadSoundMemFromSoftSound( int SoftSoundHandle, int BufferNum )
{
	SOFTSOUND * SSound ;
	int NewHandle ;
	void *WaveImage ;
	int WaveSize ;
	LOADSOUND_GPARAM GParam ;

	// エラー判定
	if( SSND_MASKHCHK( SoftSoundHandle, SSound ) ) return -1 ;
	if( SSound->IsPlayer == TRUE ) return -1 ;

	// ＷＡＶＥファイルをでっち上げる
	if( CreateWaveFileImage( &WaveImage, &WaveSize,
						 &SSound->BufferFormat, sizeof( WAVEFORMATEX ),
						 SSound->Wave.Buffer, SSound->Wave.BufferSampleNum * SSound->BufferFormat.nBlockAlign ) < 0 )
		 return -1 ;

	// ハンドルの作成
	InitLoadSoundGParam( &GParam ) ;
	if( GParam.CreateSoundDataType == DX_SOUNDDATATYPE_FILE )
		GParam.CreateSoundDataType = DX_SOUNDDATATYPE_MEMNOPRESS ;
	NewHandle = LoadSoundMemByMemImageBase_UseGParam( &GParam, TRUE, -1, WaveImage, WaveSize, BufferNum, -1, FALSE, FALSE ) ;

	// メモリの解放
	DXFREE( WaveImage ) ;
	WaveImage = NULL ;

	// 終了
	return NewHandle ;
}

// サウンドハンドルのタイプがストリームハンドルではない場合にストリームハンドルに変更する
static int SoundTypeChangeToStream( int SoundHandle )
{
	SOUND * sd ;
	void *WaveImage ;
	int WaveSize ;
	int Time, Volume, Frequency, Pan ;

	if( DSOUND.DirectSoundObject == NULL ) return -1 ;

	// エラー判定
	if( SOUNDHCHK( SoundHandle, sd ) )
		return -1 ;

	// 既にストリームハンドルだったら何もせずに終了
	if( sd->Type == DX_SOUNDTYPE_STREAMSTYLE )
		return 0 ;

	// ＷＡＶＥファイルをでっち上げる
	if( CreateWaveFileImage( &WaveImage, &WaveSize,
						 &sd->BufferFormat, sizeof( WAVEFORMATEX ),
						 sd->Normal.WaveData, sd->Normal.WaveSize ) < 0 )
		return -1 ;

	// 既存ハンドルの設定を取得しておく
	Volume = NS_GetVolumeSoundMem( SoundHandle ) ;
	Time = NS_GetSoundCurrentPosition( SoundHandle ) ;
	Frequency = NS_GetFrequencySoundMem( SoundHandle ) ;
	Pan = NS_GetPanSoundMem( SoundHandle ) ;

	// 既存ハンドルの削除
	{
		NS_StopSoundMem( SoundHandle ) ;					// 再生中だったときのことを考えて止めておく
		NS_DeleteSoundMem( SoundHandle ) ;					// ハンドルを削除
	}

	// ストリーム再生形式のハンドルとして作り直す
	{
		int f, res ;
		STREAMDATA Stream ;
		
		Stream.DataPoint = MemStreamOpen( WaveImage, WaveSize ) ;
		Stream.ReadShred = *GetMemStreamDataShredStruct() ;

		SoundHandle = AddHandle( DX_HANDLETYPE_SOUND, SoundHandle ) ;
		res = NS_AddStreamSoundMem( &Stream, 0, SoundHandle, DX_SOUNDDATATYPE_MEMNOPRESS, &f ) ;
		if( res == -1 )
		{
			DXST_ERRORLOG_ADD( _T( "ストリームサウンドハンドルの作成に失敗しました\n" ) ) ;
			return -1 ;
		}

		Stream.ReadShred.Close( Stream.DataPoint ) ;
	}
	
	// メモリの解放
	DXFREE( WaveImage ) ;

	// 元のハンドルの設定をセットする
	NS_SetVolumeSoundMem( Volume, SoundHandle ) ;
	NS_SetSoundCurrentPosition( Time, SoundHandle ) ;
	NS_SetFrequencySoundMem( Frequency, SoundHandle ) ;
	NS_SetPanSoundMem( Pan, SoundHandle ) ;

	// 正常終了
	return 0 ;
}

// ミリ秒単位の数値をサンプル単位の数値に変換する
static	int MilliSecPositionToSamplePosition( int SamplesPerSec, int MilliSecTime )
{
	LONGLONG TempValue1, TempValue2 ;
	unsigned int Temp[ 4 ] ;

	TempValue1 = SamplesPerSec ;
	TempValue2 = MilliSecTime ;
	_MUL128_1( ( DWORD * )&TempValue1, ( DWORD * )&TempValue2, ( DWORD * )Temp ) ;
	TempValue2 = 1000 ;
	_DIV128_1( ( DWORD * )Temp, ( DWORD * )&TempValue2, ( DWORD * )&TempValue1 ) ;
	return ( int )TempValue1 ;
}

// サンプル単位の数値をミリ秒単位の数値に変換する
static	int SamplePositionToMilliSecPosition( int SamplesPerSec, int SampleTime )
{
	LONGLONG TempValue1, TempValue2 ;
	unsigned int Temp[ 4 ] ;

	TempValue1 = SampleTime ;
	TempValue2 = 1000 ;
	_MUL128_1( ( DWORD * )&TempValue1, ( DWORD * )&TempValue2, ( DWORD * )Temp ) ;
	TempValue2 = SamplesPerSec ;
	_DIV128_1( ( DWORD * )Temp, ( DWORD * )&TempValue2, ( DWORD * )&TempValue1 ) ;
	return ( int )TempValue1 ;
}

}

#endif // DX_NON_SOUND







