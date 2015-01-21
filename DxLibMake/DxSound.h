// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		サウンドプログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXSOUND_H__
#define __DXSOUND_H__

#include "DxCompileConfig.h"

#ifndef DX_NON_SOUND

// Include ------------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxRingBuffer.h"
#include "DxSoundConvert.h"
#include "DxHandle.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#define MAX_SOUNDBUFFER_NUM				(10)			// DIRECTSOUNDBUFFER バッファの最大数
#define STS_SETSOUNDNUM					(4)				// ストリーム風サウンド再生に登録できるサウンドの数

#define D_X3DAUDIO_INPUTCHANNELS		(8)				// X3DAudio の計算で使用する最大入力チャンネル数
#define SOUNDBUFFER_MAX_CHANNEL_NUM		(8)				// 対応するサウンドチャンネルの最大数

// 结构体定义 --------------------------------------------------------------------

// ファイルからサウンドハンドルを作成する処理に必要なグローバルデータを纏めたもの
struct LOADSOUND_GPARAM
{
	int						Create3DSoundFlag ;						// 3Dサウンドを作成するかどうかのフラグ( TRUE:３Ｄサウンドを作成する  FALSE:３Ｄサウンドを作成しない )
	int						CreateSoundDataType ;					// 作成するサウンドデータのデータタイプ
	int						DisableReadSoundFunctionMask ;			// 使用しない読み込み処理のマスク
#ifndef DX_NON_OGGVORBIS
	int						OggVorbisBitDepth ;						// ＯｇｇＶｏｒｂｉｓ使用時のビット深度(1:8bit 2:16bit)
	int						OggVorbisFromTheoraFile ;				// Ogg Theora ファイル中の Vorbis データを参照するかどうかのフラグ( TRUE:Theora ファイル中の Vorbis データを参照する )
#endif
} ;

// サウンドバッファをロックした際に使用するデータ型
struct SOUNDBUFFERLOCKDATA
{
	DWORD						StartOffst ;
	BYTE						*WriteP, *WriteP2 ;
	DWORD						Length, Length2 ;
	DWORD						Offset, Offset2 ;
	DWORD						Valid, Valid2 ;
} ;

// 波形データ構造体
struct WAVEDATA
{
	void						*Buffer ;				// 波形データ
	void						*DoubleSizeBuffer ;		// ２倍容量の波形データ
	int							Bytes ;					// サイズ
	int							RefCount ;				// 参照数
} ;

// XAudio2用コールバック処理用構造体
struct SOUNDBUFFER_CALLBACK : public D_IXAudio2VoiceCallback
{
	virtual void    __stdcall OnVoiceProcessingPassStart( DWORD BytesRequired ) ;
	virtual void    __stdcall OnVoiceProcessingPassEnd	() ;
	virtual void    __stdcall OnStreamEnd				() ;
	virtual void    __stdcall OnBufferStart				( void* pBufferContext ) ;
	virtual void    __stdcall OnBufferEnd				( void* pBufferContext ) ;
	virtual void    __stdcall OnLoopEnd					( void* pBufferContext ) ;
	virtual void    __stdcall OnVoiceError				( void* pBufferContext, HRESULT Error ) ;

	struct SOUNDBUFFER			*Buffer ;
} ;

// ３Ｄサウンドデータの基本的な情報
struct _3DSOUNDINFO
{
	VECTOR						Position ;				// 位置
	VECTOR						FrontDirection ;		// 前方方向
	VECTOR						UpDirection ;			// 上方向
	VECTOR						Velocity ;				// 移動速度
	float						InnerAngle ;			// 可聴内角度
	float						OuterAngle ;			// 可聴外角度
	float						InnerVolume ;			// 可聴内角度での音量スケール
	float						OuterVolume ;			// 可聴外角度での音量スケール
} ;

// サウンドバッファ構造体
struct SOUNDBUFFER
{
	int							Valid ;					// データが有効か否か(TRUE:有効 FALSE:無効)

	union
	{
		D_IDirectSoundBuffer 		*DSBuffer ;				// DirectSoundBuffer
		D_IXAudio2SourceVoice		*XA2SourceVoice ;		// XAudio2SourceVoice
		D_IXAudio2_8SourceVoice		*XA2_8SourceVoice ;		// XAudio2_8SourceVoice
	} ;
	union
	{
		D_IXAudio2SubmixVoice		*XA2SubmixVoice ;		// XAudio2SubmixVoice
		D_IXAudio2_8SubmixVoice		*XA2_8SubmixVoice ;		// XAudio2_8SubmixVoice
	} ;
	IUnknown					*XA2ReverbEffect ;		// リバーブエフェクト
	SOUNDBUFFER_CALLBACK		*XA2Callback ;			// XAudio2用のコールバック

	union
	{
		D_XAUDIO2FX_REVERB_PARAMETERS    XAudio2ReverbParameter ;	// リバーブ計算用パラメータ
		D_XAUDIO2FX_REVERB_PARAMETERS2_8 XAudio2_8ReverbParameter ;	// リバーブ計算用パラメータXAudio2.8用
	} ;

	int							Pan ;					// パン( 10000:左100%右0%  0:左右100%  -100000:右100% )
	int							Volume[ SOUNDBUFFER_MAX_CHANNEL_NUM ] ;	// ボリューム( 10000:0%  0:100% )
	int							Frequency ;				// 再生周波数( -1:デフォルト )
	int							SampleNum ;				// サンプルの数
	WAVEFORMATEX				Format ;				// バッファのフォーマット

	// DirectSound 以外の場合に使用
	WAVEDATA					*Wave ;					// 波形データバッファ
	volatile int				State ;					// 再生状態( TRUE:再生中  FALSE:停止中 )
	volatile int				Pos ;					// 再生位置
	volatile int				CompPos ;				// 転送が完了した位置
	int							Loop ;					// ループするかどうか( TRUE:ループする  FALSE:ループしない )
	int							CalcPan ;				// キャプチャ用に計算した後のパン( 256:左100%右0%  0:左右100%  -256:右100% )
	int							CalcVolume ;			// キャプチャ用に計算した後のボリューム( 256:100%  0:0% )
	int							XAudioChannels ;		// XAudio 側でのチャンネル数
	volatile int				StopTimeState ;			// StopTime の状態( 0:動作無し 1:再生終了待ち 2:時間経過待ち )
	volatile int				StopTime ;				// 再生が停止した時間

	// ３Ｄサウンド再生用データ
	int							Is3DSound ;												// ３Ｄサウンドかどうか
	D_X3DAUDIO_EMITTER			X3DAudioEmitterData ;									// ３Ｄサウンド用音源情報
	D_X3DAUDIO_CONE				X3DAudioEmitterConeData ;								// ３Ｄサウンド用音源情報で使用するコーン情報
	float						X3DAudioEmitterChannelAzimuths[ D_X3DAUDIO_INPUTCHANNELS ] ;	// ３Ｄサウンド用音源情報で使用するチャンネル位置テーブル
	_3DSOUNDINFO				EmitterInfo ;											// ３Ｄサウンド用の情報
	float						EmitterRadius ;											// ３Ｄサウンド用音源の音が届く距離
	int							EmitterDataChangeFlag ;									// ３Ｄサウンド用音源情報が変更されたかどうか( TRUE:変更された  FALSE:変更されていない )
	float						DSound_Calc3DVolume ;									// DirectSound用の３Ｄ計算後のボリューム
	float						DSound_Calc3DPan ;										// DirectSound用の３Ｄ計算後のパン
} ;

// ストリーム風サウンドプレイ用ファイルデータ
struct STREAMFILEDATA
{
	char						LoopNum ;					// ループ回数( -1:無限ループ )
	int							DataType ;					// データのタイプ( DX_SOUNDDATATYPE_MEMNOPRESS 等 )
	void						*FileImage ;				// メモリ上に展開したファイルイメージ( DX_SOUNDDATATYPE_FILE 以外で使用 )
	int							FileImageSize ;				// メモリ上に展開したファイルイメージのサイズ( DX_SOUNDDATATYPE_FILE 以外で使用 )
	STREAMDATA					MemStream ;					// メモリストリーム処理用データ( DX_SOUNDDATATYPE_FILE 以外で使用 )
	int							LoopSamplePosition ;		// ループ後の再生位置(サンプル単位、OGG か 無圧縮WAV のみ有効 )
	int							LoopStartSamplePosition ;	// ループを開始する再生位置(サンプル単位、OGG か 無圧縮WAV のみ有効 )
	
	SOUNDCONV					ConvData ;					// フォーマット変換処理用データ
} ;

// ストリーム風サウンドプレイ用データ
struct STREAMPLAYDATA
{
	int							BufferBorrowSoundHandle ;								// DirectSoundBuffer を貸りているサウンドハンドル(-1:借りていない)
	int							BufferBorrowSoundHandleCount ;							// DirectSoundBuffer を貸している数(0:貸していない)
	int							BufferUseSoundHandle ;									// DirectSoundBuffer を現在使用しているサウンドハンドル(-1:誰も今は使っていない)
	int							DeleteWaitFlag ;										// DirectSoundBuffer を貸しているから削除できないで居るのか、フラグ
	
	BYTE						*SoundBuffer ;											// サウンドバッファ( XAudio2用 )
	DWORD						SoundBufferSize ;										// DirectSoundBuffer のサイズ( 単位はバイト )
	int							SoundBufferCompCopyOffset ;								// 前回のサウンドデータコピーで転送が完了したサウンドデータ領域の末端オフセット
	int							SoundBufferCopyStartOffset ;							// 『ここまできたら次のサウンドデータコピーを開始する』というデータオフセット

	STREAMFILEDATA				File[ STS_SETSOUNDNUM ] ;								// ストリームファイルデータ
	BYTE						FileLibraryLoadFlag[ STS_SETSOUNDNUM ] ;				// ライブラリがロードしたデータかどうか
	int							FileNum ;												// ストリームファイルデータの数
	int							FileActive ;											// アクティブなストリームファイルデータ番号
	int							FileLoopCount ;											// 現在アクティブなストリームデータがループした回数
	int							FileCompCopyLength ;									// 現在アクティブなストリームデータ中のサウンドバッファへの転送が完了したバイト数
	int							TotalSample ;											// 全ストリームのサンプル総数

	DWORD						CompPlayWaveLength ;									// 再生し終わった波形データの長さ
	DWORD						PrevCheckPlayPosition ;									// 前回チェックした再生位置

	BYTE						StartSetupCompFlag ;									// 再生準備完了フラグ
	BYTE						EndWaitFlag ;											// 終了待ち中フラグ
	BYTE						LoopPositionValidFlag ;									// ループ位置が有効フラグ
	BYTE						AllPlayFlag ;											// 全体を通して一度でも最後でも再生したかフラグ
	DWORD						LoopAfterCompPlayWaveLength ;							// ループ後の CompPlayWaveLength に加算する値
	
	DWORD						EndOffset ;												// 終了オフセット
	DWORD						EndStartOffset ;										// 終了処理開始オフセット
	DWORD						LoopPosition ;											// ループ位置(再生バッファ内のアドレス)

	HANDLELIST					StreamSoundList ;										// ストリームサウンドリスト処理用構造体
} ;

// ノーマルサウンドプレイ用データ
struct NORMALPLAYDATA
{
	int							WaveSize ;												// 波形データのサイズ
	void						*WaveData ;												// 波形データ
	int							*WaveDataUseCount ;										// 波形データ参照数
	int							BufferDuplicateFlag[ MAX_SOUNDBUFFER_NUM ] ;			// 複製されたサウンドバッファかどうかのフラグ
	int							BackPlayBufferNo ;										// 一つ前に再生したサウンドバッファの番号(再生用バッファが複数ある場合用)
} ;

// サウンドデータ
struct SOUND
{
	HANDLEINFO					HandleInfo ;											// ハンドル共通情報

	HANDLELIST					_3DSoundList ;											// ３Ｄサウンドリスト処理用構造体
	int							Is3DSound ;												// ３Ｄサウンドかどうか( TRUE:３Ｄサウンド  FALSE:非３Ｄサウンド )
	int							AddPlay3DSoundList ;									// Play3DSoundList がリストに追加されているかどうか( TRUE:追加されている  FALSE:追加されていない )
	HANDLELIST					Play3DSoundList ;										// 再生中の３Ｄサウンドのリスト処理用構造体

	int							PlayFinishDeleteFlag ;									// サウンドの再生が終了したら削除するかどうか( TRUE:削除する  FALSE:削除しない )
	HANDLELIST					PlayFinishDeleteSoundList ;								// サウンドの再生が終了したら削除するサウンドのリスト処理用構造体

//	char						FilePath[ 256 ] ;										// ディレクトリ
//	char						SoundName[ 256 ] ;										// サウンドファイルネーム

	int							ValidBufferNum ;										// 有効なバッファの数
	int							BufferPlayStateBackupFlagValid[ MAX_SOUNDBUFFER_NUM ] ;	// BufferPlayStateBackupFlag が有効かどうかのフラグ( TRUE:有効  FALSE:無効 )
	int							BufferPlayStateBackupFlag[ MAX_SOUNDBUFFER_NUM ] ;		// サウンドバッファを一時停止するときの再生状態フラグ
	SOUNDBUFFER					Buffer[ MAX_SOUNDBUFFER_NUM ] ;							// サウンドバッファ
	WAVEFORMATEX				BufferFormat ;											// サウンドバッファのフォーマット

	int							Type ;													// サウンドのタイプ
	int							PlayType ;												// 再生タイプ

	STREAMPLAYDATA				Stream ;												// ストリーム風サウンドプレイ用データ
	NORMALPLAYDATA				Normal ;												// ノーマルサウンドプレイ用データ

	int							PresetReverbParam ;										// ３Ｄサウンド時に設定するプリセットリバーブパラメータ番号( DX_REVERB_PRESET_DEFAULT 等 )
	SOUND3D_REVERB_PARAM		ReverbParam ;											// ３Ｄサウンド時に設定するリバーブパラメータ( PresetReverbParam が -1 の際に使用 )

	int							BaseVolume[ SOUNDBUFFER_MAX_CHANNEL_NUM ] ;				// 基本ボリューム( -1:デフォルト )
	int							BasePan ;												// 基本パン( -1:デフォルト )
	int							BaseFrequency ;											// 基本再生周波数( -1:デフォルト )
	VECTOR						Base3DPosition ;										// 基本再生位置
	float						Base3DRadius ;											// 基本聞こえる距離
	VECTOR						Base3DVelocity ;										// 基本音の速度

	BYTE						ValidNextPlayVolume[ SOUNDBUFFER_MAX_CHANNEL_NUM ] ;	// NextPlayVolume が有効かどうか( 1:有効 0:無効 )
	BYTE						ValidNextPlayPan ;										// NextPlayPan が有効かどうか( 1:有効 0:無効 )
	BYTE						ValidNextPlayFrequency ;								// NextPlayFrequency が有効かどうか( 1:有効 0:無効 )
	BYTE						ValidNextPlay3DPosition ;								// NextPlay3DPosition が有効かどうか( 1:有効 0:無効 )
	BYTE						ValidNextPlay3DRadius ;									// NextPlay3DRadius が有効かどうか( 1:有効 0:無効 )
	BYTE						ValidNextPlay3DVelocity ;								// NextPlay3DVelocity が有効かどうか( 1:有効 0:無効 )
	int							NextPlayVolume[ SOUNDBUFFER_MAX_CHANNEL_NUM ] ;			// 次に鳴らす音のボリューム
	int							NextPlayPan ;											// 次に鳴らす音のパン
	int							NextPlayFrequency ;										// 次に鳴らす音の周波数
	VECTOR						NextPlay3DPosition ;									// 次に鳴らす音の位置
	float						NextPlay3DRadius ;										// 次に鳴らす音の聞こえる距離
	VECTOR						NextPlay3DVelocity ;									// 次に鳴らす音の移動速度
} ;

// ソフトウエアで扱う波形データの波形データ構造体
struct SOFTSOUND_WAVE
{
	int							BufferSampleNum ;										// バッファのサイズ( サンプル数 )
	void						*Buffer ;												// データバッファ
} ;

// ソフトウエアで扱う波形データのプレイヤー構造体
struct SOFTSOUND_PLAYER
{
	HANDLELIST					SoftSoundPlayerList ;									// ソフトウエアサウンドプレイヤーリスト処理用構造体

	int							StockSampleNum ;										// ストックしているサンプルの数
	RINGBUF						StockSample ;											// サンプルストック用リングバッファ

	int							SoundBufferPlayStateBackupFlagValid ;					// SoundBufferPlayStateBackupFlag が有効かどうかのフラグ( TRUE:有効  FALSE:無効 )
	int							SoundBufferPlayStateBackupFlag ;						// サウンドバッファを一時停止するときの再生状態フラグ
	SOUNDBUFFER					SoundBuffer ;											// 再生用サウンドバッファ
	int							SoundBufferSize ;										// 再生用サウンドバッファのサイズ( バイト単位 )
//	int							BackCheckPlayOffset ;									// 一つ前にチェックした再生位置
	int							DataSetCompOffset ;										// データセットが済んでいるサウンドバッファ上の位置( -1:まだセットしていない )

	int							NoneDataSetCompOffset ;									// 無音データのセットを開始したサウンドバッファ上の位置( -1:まだセットしていない )
	int							NoneDataPlayCheckBackPlayOffset ;						// 無音データの再生を開始したかを検出するための前回チェック時の再生位置
	int							NoneDataPlayStartFlag ;									// 無音データの再生を開始しているかどうかフラグ( TRUE:している  FALSE:していない )

	int							MaxDataSetSize ;										// 再生位置に先行してセットする最大データサイズ、バイト単位( 先行してセットしてあるデータのサイズがこのサイズ以上にならないようにする )
	int							MinDataSetSize ;										// 再生位置に先行してセットする最小データサイズ、バイト単位( 先行してセットしてあるデータのサイズがこのサイズ以下にならないようにする )

	int							IsPlayFlag ;											// 再生中かどうかのフラグ( TRUE:再生中   FALSE:停止中 )
} ;

// ソフトウエアで扱う波形データ
struct SOFTSOUND
{
	HANDLEINFO					HandleInfo ;											// ハンドル共通情報

	int							IsPlayer ;												// プレイヤーかどうかのフラグ( TRUE:プレイヤー  FALSE:サウンドデータ )
	WAVEFORMATEX				BufferFormat ;											// サウンドのフォーマット

	union
	{
		SOFTSOUND_WAVE			Wave ;													// 波形データ
		SOFTSOUND_PLAYER		Player ;												// プレイヤー
	} ;
} ;


// ＤｉｒｅｃｔＳｏｕｎｄデータ構造体定義
struct DIRECTSOUNDDATA
{
	int							InitializeFlag ;				// 初期化フラグ

	D_IDirectMusicLoader8		*DirectMusicLoaderObject ;		// DirectMusicLoader8 オブジェクト
	D_IDirectMusicPerformance8	*DirectMusicPerformanceObject ;	// DirectMusicPerformance8 オブジェクト

	HMODULE						XAudio2_8DLL ;					// XAudio2_8.dll
	HRESULT						( WINAPI *XAudio2CreateFunc )( D_IXAudio2_8 ** ppXAudio2, DWORD Flags, D_XAUDIO2_PROCESSOR XAudio2Processor );
	HRESULT						( WINAPI *CreateAudioVolumeMeterFunc )( IUnknown** ppApo ) ;
	HRESULT						( WINAPI *CreateAudioReverbFunc )( IUnknown** ppApo ) ;

	HMODULE						X3DAudioDLL ;					// X3DAudio.dll
	void						( __cdecl *X3DAudioInitializeFunc )( DWORD SpeakerChannelMask, float SpeedOfSound, D_X3DAUDIO_HANDLE Instance ) ;
	void						( __cdecl *X3DAudioCalculateFunc )( const D_X3DAUDIO_HANDLE Instance, const D_X3DAUDIO_LISTENER* pListener, const D_X3DAUDIO_EMITTER* pEmitter, DWORD Flags, D_X3DAUDIO_DSP_SETTINGS* pDSPSettings ) ;

	union
	{
		D_IXAudio2				*XAudio2Object ;				// XAudio2オブジェクト
		D_IXAudio2_8			*XAudio2_8Object ;				// XAudio2_8オブジェクト
		D_IDirectSound			*DirectSoundObject ;			// ＤｉｒｅｃｔＳｏｕｎｄオブジェクト
	} ;

	D_XAUDIO2FX_REVERB_PARAMETERS    XAudio2ReverbParameters[ D_XAUDIO2FX_PRESET_NUM ] ;	// リバーブ計算用パラメータ
	D_XAUDIO2FX_REVERB_PARAMETERS2_8 XAudio2_8ReverbParameters[ D_XAUDIO2FX_PRESET_NUM ] ;	// リバーブ計算用パラメータXAudio2.8用
	D_IXAudio2MasteringVoice	*XAudio2MasteringVoiceObject ;	// XAudio2MasteringVoiceオブジェクト
	D_IXAudio2_8MasteringVoice	*XAudio2_8MasteringVoiceObject ;// XAudio2_8MasteringVoiceオブジェクト
	D_XAUDIO2_EFFECT_CHAIN		XAudio2MasteringEffectChain ;	// マスターボイス用のエフェクトチェイン

	DWORD						XAudio2OutputChannelMask ;		// 出力チャンネルマスク
	DWORD						OutputChannels ;				// 出力チャンネル数
	DWORD						OutputSmaplesPerSec ;			// 出力サンプリングレート

	int							_3DSoundOneMetreEnable ;		// _3DSoundOneMetre が有効かどうか( TRUE:有効  FALSE:無効 )
	float						_3DSoundOneMetre ;				// ３Ｄサウンド処理用の１メートル
	D_X3DAUDIO_HANDLE			X3DAudioInstance ;				// ３Ｄサウンド計算用インスタンスハンドル
	D_X3DAUDIO_CONE				X3DAudioListenerConeData ;		// ３Ｄサウンドのリスナー情報に使用するコーン情報
	D_X3DAUDIO_LISTENER			X3DAudioListenerData ;			// ３Ｄサウンドのリスナー情報
	_3DSOUNDINFO				ListenerInfo ;					// ３Ｄサウンドのリスナーの情報
	VECTOR						ListenerSideDirection ;			// ３Ｄサウンドのリスナーの横方向

	D_IDirectSoundBuffer		*PrimarySoundBuffer ;			// プライマリサウンドバッファ
	D_IDirectSoundBuffer		*BeepSoundBuffer ;				// ビープ音用バッファ

	HANDLELIST					_3DSoundListFirst ;				// ３Ｄサウンドハンドルリストの先頭
	HANDLELIST					_3DSoundListLast ;				// ３Ｄサウンドハンドルリストの終端

	HANDLELIST					StreamSoundListFirst ;			// ストリームサウンドハンドルリストの先頭
	HANDLELIST					StreamSoundListLast ;			// ストリームサウンドハンドルリストの終端

	HANDLELIST					SoftSoundPlayerListFirst ;		// ソフトウエアで扱う波形データのプレイヤーのハンドルリストの先頭
	HANDLELIST					SoftSoundPlayerListLast ;		// ソフトウエアで扱う波形データのプレイヤーのハンドルリストの終端

	HANDLELIST					PlayFinishDeleteSoundListFirst ; // 再生終了時に削除するサウンドハンドルリストの先頭
	HANDLELIST					PlayFinishDeleteSoundListLast ;	 // 再生終了時に削除するサウンドハンドルリストの終端

	HANDLELIST					Play3DSoundListFirst ;			// 再生している３Ｄサウンドリストの先頭
	HANDLELIST					Play3DSoundListLast ;			// 再生している３Ｄサウンドリストの終端

	int							PlayWavSoundHandle ;			// PlayWav関数で鳴らされているWAVEデータのハンドル
	D_IDirectSoundBuffer		*NoSoundBuffer ;				// 無音バッファ

	int							DisableXAudioFlag ;				// XAudioを使用しないかどうかのフラグ( TRUE:使用しない  FALSE:使用する ) 
	int							Create3DSoundFlag ;				// 3Dサウンドを作成するかどうかのフラグ( TRUE:３Ｄサウンドを作成する  FALSE:３Ｄサウンドを作成しない )
	int							OldVolumeTypeFlag ;				// Ver3.10c以前の音量計算式を使用するかどうかのフラグ( TRUE:古い計算式を使用する  FALSE:新しい計算式を使用する )
	int							SoundMode ;						// 再生形式

	int							MaxVolume ;						// 最大音量
	int							UseSoftwareMixing ;				// ソフトウエアミキシングを行うかどうかのフラグ( TRUE:ソフトウエア  FALSE:ハードウエア )

	int							EnableSoundCaptureFlag ;		// サウンドキャプチャを前提とした動作をする
	int							SoundCaptureFlag ;				// サウンドキャプチャを実行している最中かどうかのフラグ(TRUE:最中 FASLE:違う)
	int							SoundCaptureSample ;			// キャプチャしたサンプルの数(44.1KHz換算)
	HANDLE						SoundCaptureFileHandle ;		// キャプチャしたサウンドを保存しているファイル

	int							CreateSoundDataType ;			// 作成するサウンドデータのデータタイプ
	int							DisableReadSoundFunctionMask ;	// 使用しない読み込み処理のマスク
#ifndef DX_NON_OGGVORBIS
	int							OggVorbisBitDepth ;				// ＯｇｇＶｏｒｂｉｓ使用時のビット深度(1:8bit 2:16bit)
	int							OggVorbisFromTheoraFile ;		// Ogg Theora ファイル中の Vorbis データを参照するかどうかのフラグ( TRUE:Theora ファイル中の Vorbis データを参照する )
#endif

	HANDLE						StreamSoundThreadHandle ;		// ストリームサウンドを再生する処理を行うスレッドのハンドル
	DWORD						StreamSoundThreadID ;			// ストリームサウンドを再生する処理を行うスレッドのＩＤ
	DWORD						StreamSoundThreadEndFlag ;		// ストリームサウンドを再生する処理を終了するかどうかのフラグ
} ;

// ＭＩＤＩデータ
struct MIDIHANDLEDATA
{
	HANDLEINFO					HandleInfo ;					// ハンドル共通データ

	int							PlayFlag ;						// 再生中か、フラグ
	int							PlayStartFlag ;					// 再生が開始されたか、フラグ
	int							Volume ;						// 音量

	void						*DataImage ;					// ＭＩＤＩファイルのデータが格納されているアドレス( DSOUND.SoundMode が DX_MIDIMODE_MCI の時のみ有効 )
	int							DataSize ;						// ＭＩＤＩファイルのデータサイズ( DSOUND.SoundMode が DX_MIDIMODE_MCI の時のみ有効 )

	D_IDirectMusicSegment8		*DirectMusicSegmentObject ;		// DirectMusicSegment8 オブジェクト
	int							StartTime ;						// 再生開始時間
} ;

// ＭＩＤＩ管理データ型
struct MIDIDATA
{
	int							PlayFlag ;						// 演奏中フラグ
	int							PlayHandle ;					// 演奏中のハンドル

	TCHAR						FileName[ MAX_PATH ] ;			// ファイルネーム

	UINT						MidiID ;						// ＭＩＤＩ演奏時のＩＤ
	int							LoopFlag ;						// ループ演奏か、フラグ

	D_IDirectMusicSegment8		*DirectMusicSegmentObject ;		// DirectMusicSegment8 オブジェクト
	int							StartTime ;						// 再生開始時間

	int							MemImagePlayFlag ;				// メモリ上のイメージを使用して再生しているか、フラグ( TRUE:メモリイメージ  FALSE:ファイル )

	int							DefaultHandle ;					// 標準の再生関数で使用するMIDIハンドル
	int							DefaultHandleToSoundHandleFlag ;	// 標準の再生関数で再生しているハンドルは音声ハンドルか、フラグ
} ;

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// ＤｉｒｅｃｔＳｏｕｎｄデータ
extern DIRECTSOUNDDATA DX_DirectSoundData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	int		InitializeDirectSound( void ) ;																			// ＤｉｒｅｃｔＳｏｕｎｄを初期化する
extern	int		TerminateDirectSound( void ) ;																			// ＤｉｒｅｃｔＳｏｕｎｄの使用を終了する

extern	int		RefreshStreamSoundPlayCompLength( int SoundHandle, int CurrentPosition = -1, int ASyncThread = FALSE ) ;	// サウンドハンドルの再生完了時間を更新する
extern	int		ST_SoftSoundPlayerProcessAll( void ) ;																	// ソフトウエアで制御する全てのサウンドデータプレイヤーの定期処理を行う

extern	int		MidiCallBackProcess( void ) ;																			// ＭＩＤＩ演奏終了時呼ばれるコールバック関数

extern	int		StartSoundCapture( const char *SaveFilePath ) ;															// サウンドキャプチャの開始
extern	int		SoundCaptureProcess( int CaptureSample ) ;																// サウンドキャプチャの周期的処理
extern	int		EndSoundCapture( void ) ;																				// サウンドキャプチャの終了

extern	void	InitLoadSoundGParam( LOADSOUND_GPARAM *GParam ) ;														// LOADSOUND_GPARAM のデータをセットする

extern	int		InitializeSoundHandle( HANDLEINFO *HandleInfo ) ;														// サウンドハンドルの初期化
extern	int		TerminateSoundHandle( HANDLEINFO *HandleInfo ) ;														// サウンドハンドルの後始末

extern	int		LoadSoundMemBase_UseGParam( LOADSOUND_GPARAM *GParam, const TCHAR *WaveName, int BufferNum, int UnionHandle, int ASyncLoadFlag = FALSE, int ASyncThread = FALSE ) ;																					// LoadSoundMemBase のグローバル変数にアクセスしないバージョン
extern	int		LoadSoundMemByMemImageBase_UseGParam( LOADSOUND_GPARAM *GParam, int CreateSoundHandle, int SoundHandle, const void *FileImageBuffer, int ImageSize, int BufferNum, int UnionHandle = -1, int ASyncLoadFlag = FALSE, int ASyncThread = FALSE ) ;		// LoadSoundMemByMemImageBase のグローバル変数にアクセスしないバージョン
extern	int		LoadSoundMem2_UseGParam( LOADSOUND_GPARAM *GParam, const TCHAR *WaveName1 , const TCHAR *WaveName2, int ASyncLoadFlag = FALSE ) ;																													// LoadSoundMem2 のグローバル変数にアクセスしないバージョン
extern	int		LoadSoundMem2ByMemImage_UseGParam( LOADSOUND_GPARAM *GParam, const void *FileImageBuffer1, int ImageSize1, const void *FileImageBuffer2, int ImageSize2, int ASyncLoadFlag = FALSE ) ;																			// LoadSoundMem2ByMemImage のグローバル変数にアクセスしないバージョン

extern	int		AddStreamSoundMem_UseGParam( LOADSOUND_GPARAM *GParam, STREAMDATA *Stream, int LoopNum,  int SoundHandle, int StreamDataType, int *CanStreamCloseFlag, int UnionHandle = -1, int ASyncThread = FALSE ) ;											// AddStreamSoundMem のグローバル変数にアクセスしないバージョン
extern	int		AddStreamSoundMemToFile_UseGParam( LOADSOUND_GPARAM *GParam, const TCHAR *WaveFile, int LoopNum,  int SoundHandle, int StreamDataType, int UnionHandle = -1, int ASyncThread = FALSE ) ;															// AddStreamSoundMemToFile のグローバル変数にアクセスしないバージョン
extern	int		AddStreamSoundMemToMem_UseGParam( LOADSOUND_GPARAM *GParam, void *FileImageBuffer, int ImageSize, int LoopNum,  int SoundHandle, int StreamDataType, int UnionHandle = -1, int ASyncThread = FALSE ) ;												// AddStreamSoundMemToMem のグローバル変数にアクセスしないバージョン
extern	int		SetupStreamSoundMem_UseGParam( int SoundHandle, int ASyncThread = FALSE ) ;																																											// SetupStreamSoundMem のグローバル変数にアクセスしないバージョン
extern	int		ProcessStreamSoundMem_UseGParam( int SoundHandle, int ASyncThread = FALSE ) ;																																										// ProcessStreamSoundMem のグローバル変数にアクセスしないバージョン

extern	int		Refresh3DSoundParamAll() ;																				// 再生中のすべての３Ｄサウンドのパラメータを更新する

extern	int		ProcessPlayFinishDeleteSoundMemAll( void ) ;															// 再生が終了したらサウンドハンドルを削除するサウンドの処理を行う
extern	int		ProcessPlay3DSoundMemAll( void ) ;																		// ３Ｄサウンドを再生しているサウンドハンドルに対する処理を行う

extern	int		SetupSoftSoundHandle( int SoftSoundHandle, int IsPlayer, int Channels, int BitsPerSample, int SamplesPerSec, int SampleNum ) ;	// ソフトサウンドハンドルをセットアップ

extern	int		InitializeSoftSoundHandle( HANDLEINFO *HandleInfo ) ;													// ソフトサウンドハンドルの初期化
extern	int		TerminateSoftSoundHandle( HANDLEINFO *HandleInfo ) ;													// ソフトサウンドハンドルの後始末

extern	int		DeleteCancelCheckSoftSoundFunction( HANDLEINFO *HandleInfo ) ;											// ソフトサウンドハンドルを削除するかどうかをチェックする関数
extern	int		DeleteCancelCheckSoftSoundPlayerFunction( HANDLEINFO *HandleInfo ) ;									// ソフトサウンドプレーヤーハンドルを削除するかどうかをチェックする関数

extern	int		LoadSoftSoundBase_UseGParam( LOADSOUND_GPARAM *GParam, const TCHAR *FileName, const void *FileImage, int FileImageSize, int ASyncLoadFlag = FALSE ) ;		// ソフトウエアで扱う波形データをファイルまたはメモリ上に展開されたファイルイメージから作成する
extern	int		MakeSoftSoundBase_UseGParam( int IsPlayer, int Channels, int BitsPerSample, int SamplesPerSec, int SampleNum, int UseFormat_SoftSoundHandle = -1, int ASyncLoadFlag = FALSE ) ;	// ソフトウエアで制御するサウンドデータハンドルの作成

extern	int		AddMusicData( void ) ;																					// 新しいＭＩＤＩハンドルを取得する

extern	int		InitializeMidiHandle( HANDLEINFO *HandleInfo ) ;														// ＭＩＤＩハンドルを初期化をする関数
extern	int		TerminateMidiHandle( HANDLEINFO *HandleInfo ) ;															// ＭＩＤＩハンドルの後始末を行う関数

extern	int		LoadMusicMemByMemImage_UseGParam( void *FileImage, int FileImageSize, int ASyncLoadFlag = FALSE ) ;		// LoadMusicMemByMemImage のグローバル変数にアクセスしないバージョン
extern	int		LoadMusicMem_UseGParam( const TCHAR *FileName, int ASyncLoadFlag = FALSE ) ;							// LoadMusicMem のグローバル変数にアクセスしないバージョン
extern	int		LoadMusicMemByResource_UseGParam( const TCHAR *ResourceName, const TCHAR *ResourceType, int ASyncLoadFlag = FALSE ) ;	// LoadMusicMemByResource のグローバル変数にアクセスしないバージョン

extern	int		PauseSoundMemAll( int PauseFlag ) ;																		// 全ての音の一時停止状態を変更する
extern	int		PauseSoftSoundAll( int PauseFlag ) ;																	// 全てのソフトウエアサウンドの一時停止状態を変更する

}

#endif // DX_NON_SOUND

#endif // __DXSOUND_H__
