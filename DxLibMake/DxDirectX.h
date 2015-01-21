﻿// ----------------------------------------------------------------------------
//
//		ＤＸライブラリ		DirectX 関連定義用ヘッダファイル
//
//				Ver 3.11f
//
// ----------------------------------------------------------------------------

// 多重インクルード防止用マクロ
#ifndef __DXDIRECTX_H__
#define __DXDIRECTX_H__

#include "DxCompileConfig.h"
#ifdef __WINDOWS__
	#ifndef DX_GCC_COMPILE
	#include <mmreg.h>
	#endif
	#include <unknwn.h>
#endif

namespace DxLib
{

typedef void *D_HMONITOR ;

// ＤｉｒｅｃｔＳｏｕｎｄ -----------------------------------------------------

#define D_DS_OK									(S_OK)

#define D_DSBVOLUME_MIN							(-10000)
#define D_DSBVOLUME_MAX							(0)

#define D_DSSCL_NORMAL							(0x00000001)
#define D_DSSCL_PRIORITY						(0x00000002)
#define D_DSSCL_EXCLUSIVE						(0x00000003)
#define D_DSSCL_WRITEPRIMARY					(0x00000004)

#define D_DSBPLAY_LOOPING						(0x00000001)
#define D_DSBSTATUS_PLAYING						(0x00000001)
#define D_DSBSTATUS_BUFFERLOST					(0x00000002)
#define D_DSBSTATUS_LOOPING						(0x00000004)
#define D_DSBSTATUS_LOCHARDWARE					(0x00000008)
#define D_DSBSTATUS_LOCSOFTWARE					(0x00000010)
#define D_DSBSTATUS_TERMINATED					(0x00000020)
#define D_DSBFREQUENCY_ORIGINAL					(0)

#define D_DSBCAPS_PRIMARYBUFFER					(0x00000001)
#define D_DSBCAPS_STATIC						(0x00000002)
#define D_DSBCAPS_LOCHARDWARE					(0x00000004)
#define D_DSBCAPS_LOCSOFTWARE					(0x00000008)
#define D_DSBCAPS_CTRLFREQUENCY					(0x00000020)
#define D_DSBCAPS_CTRLPAN						(0x00000040)
#define D_DSBCAPS_CTRLVOLUME					(0x00000080)
#define D_DSBCAPS_GLOBALFOCUS					(0x00008000)
#define D_DSBCAPS_GETCURRENTPOSITION2			(0x00010000)

#define D_DSCAPS_PRIMARYMONO					(0x00000001)
#define D_DSCAPS_PRIMARYSTEREO					(0x00000002)
#define D_DSCAPS_PRIMARY8BIT					(0x00000004)
#define D_DSCAPS_PRIMARY16BIT					(0x00000008)
#define D_DSCAPS_SECONDARYMONO					(0x00000100)
#define D_DSCAPS_SECONDARYSTEREO				(0x00000200)
#define D_DSCAPS_SECONDARY8BIT					(0x00000400)
#define D_DSCAPS_SECONDARY16BIT					(0x00000800)

typedef struct tagD_DSCAPS
{
	DWORD										dwSize;
	DWORD										dwFlags;
	DWORD										dwMinSecondarySampleRate;
	DWORD										dwMaxSecondarySampleRate;
	DWORD										dwPrimaryBuffers;
	DWORD										dwMaxHwMixingAllBuffers;
	DWORD										dwMaxHwMixingStaticBuffers;
	DWORD										dwMaxHwMixingStreamingBuffers;
	DWORD										dwFreeHwMixingAllBuffers;
	DWORD										dwFreeHwMixingStaticBuffers;
	DWORD										dwFreeHwMixingStreamingBuffers;
	DWORD										dwMaxHw3DAllBuffers;
	DWORD										dwMaxHw3DStaticBuffers;
	DWORD										dwMaxHw3DStreamingBuffers;
	DWORD										dwFreeHw3DAllBuffers;
	DWORD										dwFreeHw3DStaticBuffers;
	DWORD										dwFreeHw3DStreamingBuffers;
	DWORD										dwTotalHwMemBytes;
	DWORD										dwFreeHwMemBytes;
	DWORD										dwMaxContigFreeHwMemBytes;
	DWORD										dwUnlockTransferRateHwBuffers;
	DWORD										dwPlayCpuOverheadSwBuffers;
	DWORD										dwReserved1;
	DWORD										dwReserved2;
} D_DSCAPS ;

typedef struct tagD_DSBUFFERDESC
{
	DWORD										dwSize;
	DWORD										dwFlags;
	DWORD										dwBufferBytes;
	DWORD										dwReserved;
	WAVEFORMATEX *								lpwfxFormat;

	GUID										guid3DAlgorithm;
} D_DSBUFFERDESC ;

typedef struct tagD_DSBPOSITIONNOTIFY
{
	DWORD										dwOffset;
	HANDLE										hEventNotify;
} D_DSBPOSITIONNOTIFY ;

#ifdef __WINDOWS__

typedef BOOL ( CALLBACK *LPD_DSENUMCALLBACKA )	( LPGUID, LPCSTR, LPCSTR, LPVOID ) ;
typedef BOOL ( CALLBACK *LPD_DSENUMCALLBACKW )	( LPGUID, LPCWSTR, LPCWSTR, LPVOID ) ;

class D_IDirectSound : public IUnknown
{
public :
	virtual HRESULT __stdcall CreateSoundBuffer			( const D_DSBUFFERDESC *pcDSBufferDesc, class D_IDirectSoundBuffer **ppDSBuffer, IUnknown *pUnkOuter ) = 0 ;
	virtual HRESULT __stdcall GetCaps					( D_DSCAPS *pDSCaps ) = 0 ;
//		virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// DuplicateSoundBuffer( LPDIRECTSOUNDBUFFER pDSBufferOriginal, LPDIRECTSOUNDBUFFER *ppDSBufferDuplicate ) = 0 ;
	virtual HRESULT __stdcall DuplicateSoundBuffer		( class D_IDirectSoundBuffer *pDSBufferOriginal, class D_IDirectSoundBuffer **ppDSBufferDuplicate ) = 0 ;
	virtual HRESULT __stdcall SetCooperativeLevel		( HWND hwnd, DWORD dwLevel ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// Compact( void ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetSpeakerConfig( LPDWORD pdwSpeakerConfig ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// SetSpeakerConfig( DWORD dwSpeakerConfig ) = 0 ;
	virtual HRESULT __stdcall Initialize				( const GUID *pcGuidDevice ) = 0 ;
} ;

class D_IDirectSound8 : public D_IDirectSound
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// VerifyCertification( LPDWORD pdwCertified ) = 0 ;
} ;

class D_IDirectSoundBuffer : public IUnknown
{
public :
	virtual HRESULT __stdcall GetCaps					( void ) = 0 ;				// GetCaps( LPDSBCAPS pDSBufferCaps ) = 0 ;
	virtual HRESULT __stdcall GetCurrentPosition		( DWORD *pdwCurrentPlayCursor, DWORD *pdwCurrentWriteCursor ) = 0 ;
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetFormat( LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten ) = 0 ;
	virtual HRESULT __stdcall GetVolume					( LPLONG plVolume ) = 0 ;
	virtual HRESULT __stdcall GetPan					( LPLONG plPan ) = 0 ;
	virtual HRESULT __stdcall GetFrequency				( LPDWORD pdwFrequency ) = 0 ;
	virtual HRESULT __stdcall GetStatus					( DWORD *pdwStatus ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// Initialize( LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pcDSBufferDesc ) = 0 ;
	virtual HRESULT __stdcall Lock						( DWORD dwOffset, DWORD dwBytes, void **ppvAudioPtr1, DWORD *pdwAudioBytes1, void **ppvAudioPtr2, DWORD *pdwAudioBytes2, DWORD dwFlags ) = 0 ;
	virtual HRESULT __stdcall Play						( DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags ) = 0 ;
	virtual HRESULT __stdcall SetCurrentPosition		( DWORD dwNewPosition ) = 0 ;
	virtual HRESULT __stdcall SetFormat					( const WAVEFORMATEX *pcfxFormat ) = 0 ;
	virtual HRESULT __stdcall SetVolume					( LONG lVolume ) = 0 ;
	virtual HRESULT __stdcall SetPan					( LONG lPan ) = 0 ;
	virtual HRESULT __stdcall SetFrequency				( DWORD dwFrequency ) = 0 ;
	virtual HRESULT __stdcall Stop						( void ) = 0 ;
	virtual HRESULT __stdcall Unlock					( void *pvAudioPtr1, DWORD dwAudioBytes1, void *pvAudioPtr2, DWORD dwAudioBytes2 ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// Restore( void ) = 0 ;
} ;

class D_IDirectSoundBuffer8 : public D_IDirectSoundBuffer
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// SetFX( DWORD dwEffectsCount, LPDSEFFECTDESC pDSFXDesc, LPDWORD pdwResultCodes ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// AcquireResources( DWORD dwFlags, DWORD dwEffectsCount, LPDWORD pdwResultCodes ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetObjectInPath( REFGUID rguidObject, DWORD dwIndex, REFGUID rguidInterface, LPVOID *ppObject ) = 0 ;
} ;

class D_IDirectSoundNotify : public IUnknown
{
public :
	virtual HRESULT __stdcall SetNotificationPositions	( DWORD dwPositionNotifies, const D_DSBPOSITIONNOTIFY *pcPositionNotifies ) = 0 ;
} ;

// Ｘ３ＤＡｕｄｉｏ -----------------------------------------------------------

#if !defined(D__SPEAKER_POSITIONS_)
	#define D__SPEAKER_POSITIONS_
	#define D_SPEAKER_FRONT_LEFT				(0x00000001)
	#define D_SPEAKER_FRONT_RIGHT				(0x00000002)
	#define D_SPEAKER_FRONT_CENTER				(0x00000004)
	#define D_SPEAKER_LOW_FREQUENCY				(0x00000008)
	#define D_SPEAKER_BACK_LEFT					(0x00000010)
	#define D_SPEAKER_BACK_RIGHT				(0x00000020)
	#define D_SPEAKER_FRONT_LEFT_OF_CENTER		(0x00000040)
	#define D_SPEAKER_FRONT_RIGHT_OF_CENTER		(0x00000080)
	#define D_SPEAKER_BACK_CENTER				(0x00000100)
	#define D_SPEAKER_SIDE_LEFT					(0x00000200)
	#define D_SPEAKER_SIDE_RIGHT				(0x00000400)
	#define D_SPEAKER_TOP_CENTER				(0x00000800)
	#define D_SPEAKER_TOP_FRONT_LEFT			(0x00001000)
	#define D_SPEAKER_TOP_FRONT_CENTER			(0x00002000)
	#define D_SPEAKER_TOP_FRONT_RIGHT			(0x00004000)
	#define D_SPEAKER_TOP_BACK_LEFT				(0x00008000)
	#define D_SPEAKER_TOP_BACK_CENTER			(0x00010000)
	#define D_SPEAKER_TOP_BACK_RIGHT			(0x00020000)
	#define D_SPEAKER_RESERVED					(0x7FFC0000)
	#define D_SPEAKER_ALL						(0x80000000)
#endif

#if !defined(D_SPEAKER_MONO)
	#define D_SPEAKER_MONO						(D_SPEAKER_FRONT_CENTER)
	#define D_SPEAKER_STEREO					(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT)
	#define D_SPEAKER_2POINT1					(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_LOW_FREQUENCY)
	#define D_SPEAKER_SURROUND					(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_FRONT_CENTER | D_SPEAKER_BACK_CENTER)
	#define D_SPEAKER_QUAD						(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_BACK_LEFT | D_SPEAKER_BACK_RIGHT)
	#define D_SPEAKER_4POINT1					(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_LOW_FREQUENCY | D_SPEAKER_BACK_LEFT | D_SPEAKER_BACK_RIGHT)
	#define D_SPEAKER_5POINT1					(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_FRONT_CENTER | D_SPEAKER_LOW_FREQUENCY | D_SPEAKER_BACK_LEFT | D_SPEAKER_BACK_RIGHT)
	#define D_SPEAKER_7POINT1					(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_FRONT_CENTER | D_SPEAKER_LOW_FREQUENCY | D_SPEAKER_BACK_LEFT | D_SPEAKER_BACK_RIGHT | D_SPEAKER_FRONT_LEFT_OF_CENTER | D_SPEAKER_FRONT_RIGHT_OF_CENTER)
	#define D_SPEAKER_5POINT1_SURROUND			(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_FRONT_CENTER | D_SPEAKER_LOW_FREQUENCY | D_SPEAKER_SIDE_LEFT | D_SPEAKER_SIDE_RIGHT)
	#define D_SPEAKER_7POINT1_SURROUND			(D_SPEAKER_FRONT_LEFT | D_SPEAKER_FRONT_RIGHT | D_SPEAKER_FRONT_CENTER | D_SPEAKER_LOW_FREQUENCY | D_SPEAKER_BACK_LEFT | D_SPEAKER_BACK_RIGHT | D_SPEAKER_SIDE_LEFT | D_SPEAKER_SIDE_RIGHT)
#endif

#define D_X3DAUDIO_CALCULATE_MATRIX				(0x00000001)
#define D_X3DAUDIO_CALCULATE_DELAY				(0x00000002)
#define D_X3DAUDIO_CALCULATE_LPF_DIRECT			(0x00000004)
#define D_X3DAUDIO_CALCULATE_LPF_REVERB			(0x00000008)
#define D_X3DAUDIO_CALCULATE_REVERB				(0x00000010)
#define D_X3DAUDIO_CALCULATE_DOPPLER			(0x00000020)
#define D_X3DAUDIO_CALCULATE_EMITTER_ANGLE		(0x00000040)
#define D_X3DAUDIO_CALCULATE_ZEROCENTER			(0x00010000)
#define D_X3DAUDIO_CALCULATE_REDIRECT_TO_LFE	(0x00020000)

#define D_X3DAUDIO_SPEED_OF_SOUND				(343.5f)

typedef struct tagD_X3DAUDIO_VECTOR
{
	float										x ;
	float			  							y ;
	float										z ;
} D_X3DAUDIO_VECTOR ;

#ifndef DX_GCC_COMPILE
#pragma pack(push)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

#define D_X3DAUDIO_HANDLE_BYTESIZE				(20)

typedef BYTE D_X3DAUDIO_HANDLE[ D_X3DAUDIO_HANDLE_BYTESIZE ] ;

typedef struct tagD_X3DAUDIO_CONE
{
	float										InnerAngle ;
	float										OuterAngle ;
	float										InnerVolume ;
	float										OuterVolume ;
	float										InnerLPF ;
	float										OuterLPF ;
	float										InnerReverb ;
	float										OuterReverb ;
} D_X3DAUDIO_CONE ;

typedef struct tagD_X3DAUDIO_DISTANCE_CURVE_POINT
{
	float										Distance ;
	float										DSPSetting ;
} D_X3DAUDIO_DISTANCE_CURVE_POINT ;

typedef struct tagD_X3DAUDIO_DISTANCE_CURVE
{
	D_X3DAUDIO_DISTANCE_CURVE_POINT*			pPoints ;
	DWORD										PointCount ;
} D_X3DAUDIO_DISTANCE_CURVE ;

typedef struct tagD_X3DAUDIO_EMITTER
{
	D_X3DAUDIO_CONE*							pCone ;
	D_X3DAUDIO_VECTOR							OrientFront ;
	D_X3DAUDIO_VECTOR							OrientTop ;

	D_X3DAUDIO_VECTOR							Position ;
	D_X3DAUDIO_VECTOR							Velocity ;

	float										InnerRadius ;
	float										InnerRadiusAngle ;

	DWORD										ChannelCount ;
	float										ChannelRadius ;
	float*										pChannelAzimuths ;

	D_X3DAUDIO_DISTANCE_CURVE*					pVolumeCurve ;
	D_X3DAUDIO_DISTANCE_CURVE*					pLFECurve ;
	D_X3DAUDIO_DISTANCE_CURVE*					pLPFDirectCurve ;
	D_X3DAUDIO_DISTANCE_CURVE*					pLPFReverbCurve ;
	D_X3DAUDIO_DISTANCE_CURVE*					pReverbCurve ;

	float										CurveDistanceScaler ;
	float										DopplerScaler ;
} D_X3DAUDIO_EMITTER ;

typedef struct tagD_X3DAUDIO_LISTENER
{
	D_X3DAUDIO_VECTOR							OrientFront ;
	D_X3DAUDIO_VECTOR							OrientTop ;

	D_X3DAUDIO_VECTOR							Position ;
	D_X3DAUDIO_VECTOR							Velocity ;

	D_X3DAUDIO_CONE*							pCone ;
} D_X3DAUDIO_LISTENER ;

typedef struct tagD_X3DAUDIO_DSP_SETTINGS
{
	float*										pMatrixCoefficients ;
	float*										pDelayTimes ;
	DWORD										SrcChannelCount ;
	DWORD										DstChannelCount ;

	float										LPFDirectCoefficient ;
	float										LPFReverbCoefficient ;
	float										ReverbLevel ;
	float										DopplerFactor ;
	float										EmitterToListenerAngle ;

	float										EmitterToListenerDistance ;
	float										EmitterVelocityComponent ;
	float										ListenerVelocityComponent ;
} D_X3DAUDIO_DSP_SETTINGS ;

enum D_XAUDIO2FX_PRESET
{
	D_XAUDIO2FX_PRESET_DEFAULT,
	D_XAUDIO2FX_PRESET_GENERIC,
	D_XAUDIO2FX_PRESET_PADDEDCELL,
	D_XAUDIO2FX_PRESET_ROOM,
	D_XAUDIO2FX_PRESET_BATHROOM,
	D_XAUDIO2FX_PRESET_LIVINGROOM,
	D_XAUDIO2FX_PRESET_STONEROOM,
	D_XAUDIO2FX_PRESET_AUDITORIUM,
	D_XAUDIO2FX_PRESET_CONCERTHALL,
	D_XAUDIO2FX_PRESET_CAVE,
	D_XAUDIO2FX_PRESET_ARENA,
	D_XAUDIO2FX_PRESET_HANGAR,
	D_XAUDIO2FX_PRESET_CARPETEDHALLWAY,
	D_XAUDIO2FX_PRESET_HALLWAY,
	D_XAUDIO2FX_PRESET_STONECORRIDOR,
	D_XAUDIO2FX_PRESET_ALLEY,
	D_XAUDIO2FX_PRESET_FOREST,
	D_XAUDIO2FX_PRESET_CITY,
	D_XAUDIO2FX_PRESET_MOUNTAINS,
	D_XAUDIO2FX_PRESET_QUARRY,
	D_XAUDIO2FX_PRESET_PLAIN,
	D_XAUDIO2FX_PRESET_PARKINGLOT,
	D_XAUDIO2FX_PRESET_SEWERPIPE,
	D_XAUDIO2FX_PRESET_UNDERWATER,
	D_XAUDIO2FX_PRESET_SMALLROOM,
	D_XAUDIO2FX_PRESET_MEDIUMROOM,
	D_XAUDIO2FX_PRESET_LARGEROOM,
	D_XAUDIO2FX_PRESET_MEDIUMHALL,
	D_XAUDIO2FX_PRESET_LARGEHALL,
	D_XAUDIO2FX_PRESET_PLATE,

	D_XAUDIO2FX_PRESET_NUM,
} ;

#define D_XAUDIO2FX_DEBUG								(1)

#define D_XAUDIO2FX_REVERB_MIN_FRAMERATE				(20000)
#define D_XAUDIO2FX_REVERB_MAX_FRAMERATE				(48000)

#define D_XAUDIO2FX_REVERB_MIN_WET_DRY_MIX				(0.0f)
#define D_XAUDIO2FX_REVERB_MIN_REFLECTIONS_DELAY		(0)
#define D_XAUDIO2FX_REVERB_MIN_REVERB_DELAY				(0)
#define D_XAUDIO2FX_REVERB_MIN_REAR_DELAY				(0)
#define D_XAUDIO2FX_REVERB_MIN_POSITION					(0)
#define D_XAUDIO2FX_REVERB_MIN_DIFFUSION				(0)
#define D_XAUDIO2FX_REVERB_MIN_LOW_EQ_GAIN				(0)
#define D_XAUDIO2FX_REVERB_MIN_LOW_EQ_CUTOFF			(0)
#define D_XAUDIO2FX_REVERB_MIN_HIGH_EQ_GAIN				(0)
#define D_XAUDIO2FX_REVERB_MIN_HIGH_EQ_CUTOFF			(0)
#define D_XAUDIO2FX_REVERB_MIN_ROOM_FILTER_FREQ			(20.0f)
#define D_XAUDIO2FX_REVERB_MIN_ROOM_FILTER_MAIN			(-100.0f)
#define D_XAUDIO2FX_REVERB_MIN_ROOM_FILTER_HF			(-100.0f)
#define D_XAUDIO2FX_REVERB_MIN_REFLECTIONS_GAIN			(-100.0f)
#define D_XAUDIO2FX_REVERB_MIN_REVERB_GAIN				(-100.0f)
#define D_XAUDIO2FX_REVERB_MIN_DECAY_TIME				(0.1f)
#define D_XAUDIO2FX_REVERB_MIN_DENSITY					(0.0f)
#define D_XAUDIO2FX_REVERB_MIN_ROOM_SIZE				(0.0f)

#define D_XAUDIO2FX_REVERB_MAX_WET_DRY_MIX				(100.0f)
#define D_XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY		(300)
#define D_XAUDIO2FX_REVERB_MAX_REVERB_DELAY				(85)
#define D_XAUDIO2FX_REVERB_MAX_REAR_DELAY				(5)
#define D_XAUDIO2FX_REVERB_MAX_POSITION					(30)
#define D_XAUDIO2FX_REVERB_MAX_DIFFUSION				(15)
#define D_XAUDIO2FX_REVERB_MAX_LOW_EQ_GAIN				(12)
#define D_XAUDIO2FX_REVERB_MAX_LOW_EQ_CUTOFF			(9)
#define D_XAUDIO2FX_REVERB_MAX_HIGH_EQ_GAIN				(8)
#define D_XAUDIO2FX_REVERB_MAX_HIGH_EQ_CUTOFF			(14)
#define D_XAUDIO2FX_REVERB_MAX_ROOM_FILTER_FREQ			(20000.0f)
#define D_XAUDIO2FX_REVERB_MAX_ROOM_FILTER_MAIN			(0.0f)
#define D_XAUDIO2FX_REVERB_MAX_ROOM_FILTER_HF			(0.0f)
#define D_XAUDIO2FX_REVERB_MAX_REFLECTIONS_GAIN			(20.0f)
#define D_XAUDIO2FX_REVERB_MAX_REVERB_GAIN				(20.0f)
#define D_XAUDIO2FX_REVERB_MAX_DENSITY					(100.0f)
#define D_XAUDIO2FX_REVERB_MAX_ROOM_SIZE				(100.0f)

#define D_XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX			(100.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY	(5)
#define D_XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY			(5)
#define D_XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY			(5)
#define D_XAUDIO2FX_REVERB_DEFAULT_POSITION				(6)
#define D_XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX		(27)
#define D_XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION		(8)
#define D_XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION		(8)
#define D_XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN			(8)
#define D_XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF		(4)
#define D_XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN			(8)
#define D_XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF		(4)
#define D_XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ		(5000.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN		(0.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF		(0.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN		(0.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN			(0.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME			(1.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_DENSITY				(100.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE			(100.0f)
#define D_XAUDIO2FX_REVERB_DEFAULT_DISABLE_LATE_FIELD	(FALSE)

typedef struct tagD_XAUDIO2FX_REVERB_PARAMETERS
{
	float										WetDryMix ;

	UINT32										ReflectionsDelay ;
	BYTE										ReverbDelay ;
	BYTE										RearDelay ;

	BYTE										PositionLeft ;
	BYTE										PositionRight ;
	BYTE										PositionMatrixLeft ;
	BYTE										PositionMatrixRight ;
	BYTE										EarlyDiffusion ;
	BYTE										LateDiffusion ;
	BYTE										LowEQGain ;
	BYTE										LowEQCutoff ;
	BYTE										HighEQGain ;
	BYTE										HighEQCutoff ;

	float										RoomFilterFreq ;
	float										RoomFilterMain ;
	float										RoomFilterHF ;
	float										ReflectionsGain ;
	float										ReverbGain ;
	float										DecayTime ;
	float										Density ;
	float										RoomSize ;
} D_XAUDIO2FX_REVERB_PARAMETERS;

typedef struct tagD_XAUDIO2FX_REVERB_PARAMETERS2_8
{
	float										WetDryMix ;

	UINT32										ReflectionsDelay ;
	BYTE										ReverbDelay ;
	BYTE										RearDelay ;

	BYTE										PositionLeft ;
	BYTE										PositionRight ;
	BYTE										PositionMatrixLeft ;
	BYTE										PositionMatrixRight ;
	BYTE										EarlyDiffusion ;
	BYTE										LateDiffusion ;
	BYTE										LowEQGain ;
	BYTE										LowEQCutoff ;
	BYTE										HighEQGain ;
	BYTE										HighEQCutoff ;

	float										RoomFilterFreq ;
	float										RoomFilterMain ;
	float										RoomFilterHF ;
	float										ReflectionsGain ;
	float										ReverbGain ;
	float										DecayTime ;
	float										Density ;
	float										RoomSize ;

	BOOL										DisableLateField ;
} D_XAUDIO2FX_REVERB_PARAMETERS2_8 ;

typedef struct tagD_XAUDIO2FX_REVERB_I3DL2_PARAMETERS
{
	float										WetDryMix ;

	int											Room ;
	int											RoomHF ;
	float										RoomRolloffFactor ;
	float										DecayTime ;
	float										DecayHFRatio ;
	int											Reflections ;
	float										ReflectionsDelay ;
	int											Reverb ;
	float										ReverbDelay ;
	float										Diffusion ;
	float										Density ;
	float										HFReference ;
} D_XAUDIO2FX_REVERB_I3DL2_PARAMETERS ;

#define D_XAUDIO2FX_I3DL2_PRESET_DEFAULT         {100,-10000,    0,0.0f, 1.00f,0.50f,-10000,0.020f,-10000,0.040f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_GENERIC         {100, -1000, -100,0.0f, 1.49f,0.83f, -2602,0.007f,   200,0.011f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_PADDEDCELL      {100, -1000,-6000,0.0f, 0.17f,0.10f, -1204,0.001f,   207,0.002f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_ROOM            {100, -1000, -454,0.0f, 0.40f,0.83f, -1646,0.002f,    53,0.003f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_BATHROOM        {100, -1000,-1200,0.0f, 1.49f,0.54f,  -370,0.007f,  1030,0.011f,100.0f, 60.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_LIVINGROOM      {100, -1000,-6000,0.0f, 0.50f,0.10f, -1376,0.003f, -1104,0.004f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_STONEROOM       {100, -1000, -300,0.0f, 2.31f,0.64f,  -711,0.012f,    83,0.017f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_AUDITORIUM      {100, -1000, -476,0.0f, 4.32f,0.59f,  -789,0.020f,  -289,0.030f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_CONCERTHALL     {100, -1000, -500,0.0f, 3.92f,0.70f, -1230,0.020f,    -2,0.029f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_CAVE            {100, -1000,    0,0.0f, 2.91f,1.30f,  -602,0.015f,  -302,0.022f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_ARENA           {100, -1000, -698,0.0f, 7.24f,0.33f, -1166,0.020f,    16,0.030f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_HANGAR          {100, -1000,-1000,0.0f,10.05f,0.23f,  -602,0.020f,   198,0.030f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY {100, -1000,-4000,0.0f, 0.30f,0.10f, -1831,0.002f, -1630,0.030f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_HALLWAY         {100, -1000, -300,0.0f, 1.49f,0.59f, -1219,0.007f,   441,0.011f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR   {100, -1000, -237,0.0f, 2.70f,0.79f, -1214,0.013f,   395,0.020f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_ALLEY           {100, -1000, -270,0.0f, 1.49f,0.86f, -1204,0.007f,    -4,0.011f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_FOREST          {100, -1000,-3300,0.0f, 1.49f,0.54f, -2560,0.162f,  -613,0.088f, 79.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_CITY            {100, -1000, -800,0.0f, 1.49f,0.67f, -2273,0.007f, -2217,0.011f, 50.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_MOUNTAINS       {100, -1000,-2500,0.0f, 1.49f,0.21f, -2780,0.300f, -2014,0.100f, 27.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_QUARRY          {100, -1000,-1000,0.0f, 1.49f,0.83f,-10000,0.061f,   500,0.025f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_PLAIN           {100, -1000,-2000,0.0f, 1.49f,0.50f, -2466,0.179f, -2514,0.100f, 21.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_PARKINGLOT      {100, -1000,    0,0.0f, 1.65f,1.50f, -1363,0.008f, -1153,0.012f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_SEWERPIPE       {100, -1000,-1000,0.0f, 2.81f,0.14f,   429,0.014f,   648,0.021f, 80.0f, 60.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_UNDERWATER      {100, -1000,-4000,0.0f, 1.49f,0.10f,  -449,0.007f,  1700,0.011f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_SMALLROOM       {100, -1000, -600,0.0f, 1.10f,0.83f,  -400,0.005f,   500,0.010f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM      {100, -1000, -600,0.0f, 1.30f,0.83f, -1000,0.010f,  -200,0.020f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_LARGEROOM       {100, -1000, -600,0.0f, 1.50f,0.83f, -1600,0.020f, -1000,0.040f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL      {100, -1000, -600,0.0f, 1.80f,0.70f, -1300,0.015f,  -800,0.030f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_LARGEHALL       {100, -1000, -600,0.0f, 1.80f,0.70f, -2000,0.030f, -1400,0.060f,100.0f,100.0f,5000.0f}
#define D_XAUDIO2FX_I3DL2_PRESET_PLATE           {100, -1000, -200,0.0f, 1.30f,0.90f,     0,0.002f,     0,0.010f,100.0f, 75.0f,5000.0f}

#ifndef DX_GCC_COMPILE
#pragma pack(pop)
#pragma pack()
#else
#pragma pack(pop)
#endif


// ＸＡｕｄｉｏ ---------------------------------------------------------------

#ifndef DX_GCC_COMPILE
#pragma pack(push)
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

#define D_XAUDIO2_MAX_BUFFER_BYTES				(0x80000000)
#define D_XAUDIO2_MAX_QUEUED_BUFFERS			(64)
#define D_XAUDIO2_MAX_BUFFERS_SYSTEM			(2)
#define D_XAUDIO2_MAX_AUDIO_CHANNELS			(64)
#define D_XAUDIO2_MIN_SAMPLE_RATE				(1000)
#define D_XAUDIO2_MAX_SAMPLE_RATE				(200000)
#define D_XAUDIO2_MAX_VOLUME_LEVEL				(16777216.0f)
#define D_XAUDIO2_MIN_FREQ_RATIO				(1/1024.0f)
#define D_XAUDIO2_MAX_FREQ_RATIO				(1024.0f)
#define D_XAUDIO2_DEFAULT_FREQ_RATIO			(2.0f)
#define D_XAUDIO2_MAX_FILTER_ONEOVERQ			(1.5f)
#define D_XAUDIO2_MAX_FILTER_FREQUENCY			(1.0f)
#define D_XAUDIO2_MAX_LOOP_COUNT				(254)
#define D_XAUDIO2_MAX_INSTANCES					(8)

#define D_XAUDIO2_COMMIT_NOW					(0)
#define D_XAUDIO2_COMMIT_ALL					(0)
#define D_XAUDIO2_INVALID_OPSET					((DWORD)(-1))
#define D_XAUDIO2_NO_LOOP_REGION				(0)
#define D_XAUDIO2_LOOP_INFINITE					(255)
#define D_XAUDIO2_DEFAULT_CHANNELS				(0)
#define D_XAUDIO2_DEFAULT_SAMPLERATE			(0)

#define D_XAUDIO2_DEBUG_ENGINE					(0x0001)
#define D_XAUDIO2_VOICE_NOPITCH					(0x0002)
#define D_XAUDIO2_VOICE_NOSRC					(0x0004)
#define D_XAUDIO2_VOICE_USEFILTER				(0x0008)
#define D_XAUDIO2_VOICE_MUSIC					(0x0010)
#define D_XAUDIO2_PLAY_TAILS					(0x0020)
#define D_XAUDIO2_END_OF_STREAM					(0x0040)
#define D_XAUDIO2_SEND_USEFILTER				(0x0080)

typedef enum tagD_XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER
{
	D_Processor1 								= 0x00000001,
	D_Processor2 								= 0x00000002,
	D_Processor3 								= 0x00000004,
	D_Processor4 								= 0x00000008,
	D_Processor5 								= 0x00000010,
	D_Processor6 								= 0x00000020,
	D_Processor7 								= 0x00000040,
	D_Processor8 								= 0x00000080,
	D_Processor9 								= 0x00000100,
	D_Processor10								= 0x00000200,
	D_Processor11								= 0x00000400,
	D_Processor12								= 0x00000800,
	D_Processor13								= 0x00001000,
	D_Processor14								= 0x00002000,
	D_Processor15								= 0x00004000,
	D_Processor16								= 0x00008000,
	D_Processor17								= 0x00010000,
	D_Processor18								= 0x00020000,
	D_Processor19								= 0x00040000,
	D_Processor20								= 0x00080000,
	D_Processor21								= 0x00100000,
	D_Processor22								= 0x00200000,
	D_Processor23								= 0x00400000,
	D_Processor24								= 0x00800000,
	D_Processor25								= 0x01000000,
	D_Processor26								= 0x02000000,
	D_Processor27								= 0x04000000,
	D_Processor28								= 0x08000000,
	D_Processor29								= 0x10000000,
	D_Processor30								= 0x20000000,
	D_Processor31								= 0x40000000,
	D_Processor32								= 0x80000000,
	D_XAUDIO2_ANY_PROCESSOR						= 0xffffffff,
	D_XAUDIO2_DEFAULT_PROCESSOR					= D_XAUDIO2_ANY_PROCESSOR
} D_XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER, D_XAUDIO2_PROCESSOR ;

enum D_XAUDIO2_DEVICE_ROLE
{
	D_NotDefaultDevice							= 0x0,
	D_DefaultConsoleDevice						= 0x1,
	D_DefaultMultimediaDevice					= 0x2,
	D_DefaultCommunicationsDevice				= 0x4,
	D_DefaultGameDevice							= 0x8,
	D_GlobalDefaultDevice						= 0xf,
	D_InvalidDeviceRole							= ~D_GlobalDefaultDevice
} ;

enum D_XAUDIO2_FILTER_TYPE
{
	D_LowPassFilter,
	D_BandPassFilter,
	D_HighPassFilter,
	D_NotchFilter
} ;

enum D_AUDIO_STREAM_CATEGORY
{
	D_AudioCategory_Other = 0,
	D_AudioCategory_ForegroundOnlyMedia,
	D_AudioCategory_BackgroundCapableMedia,
	D_AudioCategory_Communications,
	D_AudioCategory_Alerts,
	D_AudioCategory_SoundEffects,
	D_AudioCategory_GameEffects,
	D_AudioCategory_GameMedia,
} ;

typedef struct tagD_WAVEFORMATEXTENSIBLE
{
	WAVEFORMATEX								Format ;
	union
	{
		WORD									wValidBitsPerSample ;
		WORD									wSamplesPerBlock ;
		WORD									wReserved ;
	} Samples ;
	DWORD										dwChannelMask ;
	GUID										SubFormat ;
} D_WAVEFORMATEXTENSIBLE ;

typedef struct tagD_XAUDIO2_VOICE_STATE
{
	void *										pCurrentBufferContext ;
	DWORD										BuffersQueued ;
	ULONGLONG									SamplesPlayed ;
} D_XAUDIO2_VOICE_STATE;

typedef struct tagD_XAUDIO2_EFFECT_DESCRIPTOR
{
	IUnknown*									pEffect ;
	BOOL										InitialState ;
	DWORD										OutputChannels ;
} D_XAUDIO2_EFFECT_DESCRIPTOR ;

typedef struct tagD_XAUDIO2_DEVICE_DETAILS
{
	WCHAR										DeviceID[256] ;
	WCHAR										DisplayName[256] ;
	D_XAUDIO2_DEVICE_ROLE						Role ;
	D_WAVEFORMATEXTENSIBLE						OutputFormat ;
} D_XAUDIO2_DEVICE_DETAILS ;

typedef struct tagD_XAUDIO2_VOICE_DETAILS
{
	DWORD										CreationFlags ;
	DWORD										InputChannels ;
	DWORD										InputSampleRate ;
} D_XAUDIO2_VOICE_DETAILS ;

typedef struct tagD_XAUDIO2_VOICE_DETAILS2_8
{
	DWORD										CreationFlags ;
	DWORD										ActiveFlags ;
	DWORD										InputChannels ;
	DWORD										InputSampleRate ;
} D_XAUDIO2_VOICE_DETAILS2_8 ;

typedef struct tagD_XAUDIO2_SEND_DESCRIPTOR
{
	DWORD										Flags ;
	class D_IXAudio2Voice*						pOutputVoice ;
} D_XAUDIO2_SEND_DESCRIPTOR ;

typedef struct tagD_XAUDIO2_SEND_DESCRIPTOR2_8
{
	DWORD										Flags ;
	class D_IXAudio2_8Voice*					pOutputVoice ;
} D_XAUDIO2_SEND_DESCRIPTOR2_8 ;

typedef struct tagD_XAUDIO2_VOICE_SENDS
{
	DWORD										SendCount ;
	D_XAUDIO2_SEND_DESCRIPTOR*					pSends ;
} D_XAUDIO2_VOICE_SENDS ;

typedef struct tagD_XAUDIO2_VOICE_SENDS2_8
{
	DWORD										SendCount ;
	D_XAUDIO2_SEND_DESCRIPTOR2_8*				pSends ;
} D_XAUDIO2_VOICE_SENDS2_8 ;

typedef struct tagD_XAUDIO2_PERFORMANCE_DATA
{
	ULONGLONG									AudioCyclesSinceLastQuery ;
	ULONGLONG									TotalCyclesSinceLastQuery ;
	DWORD										MinimumCyclesPerQuantum ;
	DWORD										MaximumCyclesPerQuantum ;
	DWORD										MemoryUsageInBytes ;
	DWORD										CurrentLatencyInSamples ;
	DWORD										GlitchesSinceEngineStarted ;
	DWORD										ActiveSourceVoiceCount ;
	DWORD										TotalSourceVoiceCount ;
	DWORD										ActiveSubmixVoiceCount ;
	DWORD										ActiveResamplerCount ;
	DWORD										ActiveMatrixMixCount ;
	DWORD										ActiveXmaSourceVoices ;
	DWORD										ActiveXmaStreams ;
} D_XAUDIO2_PERFORMANCE_DATA ;

typedef struct tagD_XAUDIO2_DEBUG_CONFIGURATION
{
	DWORD										TraceMask ;
	DWORD										BreakMask ;
	BOOL										LogThreadID ;
	BOOL										LogFileline ;
	BOOL										LogFunctionName ;
	BOOL										LogTiming ;
} D_XAUDIO2_DEBUG_CONFIGURATION ;

typedef struct tagD_XAUDIO2_EFFECT_CHAIN
{
	DWORD										EffectCount ;
	D_XAUDIO2_EFFECT_DESCRIPTOR*				pEffectDescriptors ;
} D_XAUDIO2_EFFECT_CHAIN ;

typedef struct tagD_XAUDIO2_BUFFER
{
	DWORD										Flags ;
	DWORD										AudioBytes ;
	const BYTE*									pAudioData ;
	DWORD										PlayBegin ;
	DWORD										PlayLength ;
	DWORD										LoopBegin ;
	DWORD										LoopLength ;
	DWORD										LoopCount ;
	void*										pContext ;
} D_XAUDIO2_BUFFER;

typedef struct tagD_XAUDIO2_FILTER_PARAMETERS
{
	D_XAUDIO2_FILTER_TYPE						Type;
	float										Frequency;
	float										OneOverQ;
} D_XAUDIO2_FILTER_PARAMETERS ;

typedef struct tagD_XAUDIO2_BUFFER_WMA
{
	const DWORD*								pDecodedPacketCumulativeBytes ;
	DWORD										PacketCount ;
} D_XAUDIO2_BUFFER_WMA ;

class D_IXAudio2VoiceCallback
{
public:
	virtual void    __stdcall OnVoiceProcessingPassStart( DWORD BytesRequired ) = 0 ;
	virtual void    __stdcall OnVoiceProcessingPassEnd	() = 0 ;
	virtual void    __stdcall OnStreamEnd				() = 0 ;
	virtual void    __stdcall OnBufferStart				( void* pBufferContext ) = 0 ;
	virtual void    __stdcall OnBufferEnd				( void* pBufferContext ) = 0 ;
	virtual void    __stdcall OnLoopEnd					( void* pBufferContext ) = 0 ;
	virtual void    __stdcall OnVoiceError				( void* pBufferContext, HRESULT Error ) = 0 ;
};

class D_IXAudio2EngineCallback
{
public:
	virtual void    __stdcall OnProcessingPassStart		() = 0 ;
	virtual void    __stdcall OnProcessingPassEnd		() = 0 ;
	virtual void    __stdcall OnCriticalError			( HRESULT Error ) = 0 ;
};

class D_IXAudio2 : public IUnknown
{
public:
	virtual HRESULT __stdcall QueryInterface			( REFIID riid,  void** ppvInterface ) = 0 ;
	virtual ULONG   __stdcall AddRef					( void ) = 0 ;
	virtual ULONG   __stdcall Release					( void ) = 0 ;
	virtual HRESULT __stdcall GetDeviceCount			( DWORD* pCount ) = 0 ;
	virtual HRESULT __stdcall GetDeviceDetails			( DWORD Index,  D_XAUDIO2_DEVICE_DETAILS* pDeviceDetails ) = 0 ;
	virtual HRESULT __stdcall Initialize				( DWORD Flags = 0, D_XAUDIO2_PROCESSOR XAudio2Processor = D_XAUDIO2_DEFAULT_PROCESSOR ) = 0 ;
	virtual HRESULT __stdcall RegisterForCallbacks		( D_IXAudio2EngineCallback* pCallback ) = 0 ;
	virtual void    __stdcall UnregisterForCallbacks	( D_IXAudio2EngineCallback* pCallback ) = 0 ;
	virtual HRESULT __stdcall CreateSourceVoice			( class D_IXAudio2SourceVoice** ppSourceVoice, const WAVEFORMATEX* pSourceFormat, DWORD Flags = 0, float MaxFrequencyRatio = D_XAUDIO2_DEFAULT_FREQ_RATIO, class D_IXAudio2VoiceCallback* pCallback = NULL, const D_XAUDIO2_VOICE_SENDS* pSendList = NULL, const D_XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL ) = 0 ;
	virtual HRESULT __stdcall CreateSubmixVoice			( class D_IXAudio2SubmixVoice** ppSubmixVoice, DWORD InputChannels, DWORD InputSampleRate, DWORD Flags = 0, DWORD ProcessingStage = 0, const D_XAUDIO2_VOICE_SENDS* pSendList = NULL, const D_XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL ) = 0 ;
	virtual HRESULT __stdcall CreateMasteringVoice		( class D_IXAudio2MasteringVoice** ppMasteringVoice, DWORD InputChannels = D_XAUDIO2_DEFAULT_CHANNELS, DWORD InputSampleRate = D_XAUDIO2_DEFAULT_SAMPLERATE, DWORD Flags = 0, DWORD DeviceIndex = 0, const D_XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL ) = 0 ;
	virtual HRESULT __stdcall StartEngine				( void ) = 0 ;
	virtual void    __stdcall StopEngine				( void ) = 0 ;
	virtual HRESULT __stdcall CommitChanges				( DWORD OperationSet ) = 0 ;
	virtual void    __stdcall GetPerformanceData		( D_XAUDIO2_PERFORMANCE_DATA* pPerfData ) = 0 ;
	virtual void    __stdcall SetDebugConfigurationv	( const D_XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration, void* pReserved = NULL ) = 0 ;
} ;

class D_IXAudio2_8 : public IUnknown
{
public :
    virtual HRESULT __stdcall QueryInterface			( REFIID riid, void** ppvInterface ) = 0 ;
    virtual ULONG   __stdcall AddRef					( void ) = 0 ;
    virtual ULONG   __stdcall Release					( void ) = 0 ;
    virtual HRESULT __stdcall RegisterForCallbacks		( D_IXAudio2EngineCallback* pCallback ) = 0 ;
    virtual void    __stdcall UnregisterForCallbacks	( D_IXAudio2EngineCallback* pCallback ) = 0 ;
    virtual HRESULT __stdcall CreateSourceVoice			( class D_IXAudio2_8SourceVoice** ppSourceVoice, const WAVEFORMATEX* pSourceFormat, DWORD Flags = 0, float MaxFrequencyRatio = D_XAUDIO2_DEFAULT_FREQ_RATIO, D_IXAudio2VoiceCallback* pCallback = NULL, const D_XAUDIO2_VOICE_SENDS2_8* pSendList = NULL, const D_XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL ) = 0 ;
    virtual HRESULT __stdcall CreateSubmixVoice			( class D_IXAudio2_8SubmixVoice** ppSubmixVoice, DWORD InputChannels, DWORD InputSampleRate, DWORD Flags = 0, DWORD ProcessingStage = 0, const D_XAUDIO2_VOICE_SENDS* pSendList = NULL, const D_XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL ) = 0 ;
    virtual HRESULT __stdcall CreateMasteringVoice		( class D_IXAudio2_8MasteringVoice** ppMasteringVoice, DWORD InputChannels = D_XAUDIO2_DEFAULT_CHANNELS, DWORD InputSampleRate = D_XAUDIO2_DEFAULT_SAMPLERATE, DWORD Flags = 0, LPCWSTR szDeviceId = NULL, const D_XAUDIO2_EFFECT_CHAIN* pEffectChain = NULL, D_AUDIO_STREAM_CATEGORY StreamCategory = D_AudioCategory_GameEffects ) = 0 ;
    virtual HRESULT __stdcall StartEngine				( void ) = 0 ;
    virtual void    __stdcall StopEngine				( void ) = 0 ;
    virtual HRESULT __stdcall CommitChanges				( DWORD OperationSet ) = 0 ;
    virtual void    __stdcall GetPerformanceData		( D_XAUDIO2_PERFORMANCE_DATA* pPerfData ) = 0 ;
    virtual void    __stdcall SetDebugConfiguration		( const D_XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration, void* pReserved = NULL ) = 0 ;
} ;

class D_IXAudio2Voice
{
public:
	virtual void    __stdcall GetVoiceDetailsv			( D_XAUDIO2_VOICE_DETAILS* pVoiceDetails ) = 0 ;
	virtual HRESULT __stdcall SetOutputVoices			( const D_XAUDIO2_VOICE_SENDS* pSendList ) = 0 ;
	virtual HRESULT __stdcall SetEffectChain			( const D_XAUDIO2_EFFECT_CHAIN* pEffectChain ) = 0 ;
	virtual HRESULT __stdcall EnableEffect				( DWORD EffectIndex, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall DisableEffect				( DWORD EffectIndex, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetEffectState			( DWORD EffectIndex,  BOOL* pEnabled ) = 0 ;
	virtual HRESULT __stdcall SetEffectParameters		( DWORD EffectIndex, const void* pParameters, DWORD ParametersByteSize, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall GetEffectParameters		( DWORD EffectIndex, void* pParameters, DWORD ParametersByteSize ) = 0 ;
	virtual HRESULT __stdcall SetFilterParameters		( const D_XAUDIO2_FILTER_PARAMETERS* pParameters, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetFilterParameters		( D_XAUDIO2_FILTER_PARAMETERS* pParameters ) = 0 ;
	virtual HRESULT __stdcall SetOutputFilterParameters	( D_IXAudio2Voice* pDestinationVoice, const D_XAUDIO2_FILTER_PARAMETERS* pParameters, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetOutputFilterParameters	( D_IXAudio2Voice* pDestinationVoice, D_XAUDIO2_FILTER_PARAMETERS* pParameters ) = 0 ;
	virtual HRESULT __stdcall SetVolume					( float Volume, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetVolume					( float* pVolume ) = 0 ;
	virtual HRESULT __stdcall SetChannelVolumes			( DWORD Channels, const float* pVolumes, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetChannelVolumes			( DWORD Channels, float* pVolumes ) = 0 ; 
	virtual HRESULT __stdcall SetOutputMatrix			( D_IXAudio2Voice* pDestinationVoice, DWORD SourceChannels, DWORD DestinationChannels, const float* pLevelMatrix, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetOutputMatrix			( D_IXAudio2Voice* pDestinationVoice, DWORD SourceChannels, DWORD DestinationChannels, float* pLevelMatrix ) = 0 ;
	virtual void    __stdcall DestroyVoice				( void ) = 0 ;
} ;

class D_IXAudio2_8Voice
{
public:
	virtual void    __stdcall GetVoiceDetails			( D_XAUDIO2_VOICE_DETAILS2_8* pVoiceDetails ) = 0 ;
	virtual HRESULT __stdcall SetOutputVoices			( const D_XAUDIO2_VOICE_SENDS* pSendList ) = 0 ;
	virtual HRESULT __stdcall SetEffectChain			( const D_XAUDIO2_EFFECT_CHAIN* pEffectChain ) = 0 ;
	virtual HRESULT __stdcall EnableEffect				( DWORD EffectIndex, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall DisableEffect				( DWORD EffectIndex, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetEffectState			( DWORD EffectIndex, BOOL* pEnabled ) = 0 ;
	virtual HRESULT __stdcall SetEffectParameters		( DWORD EffectIndex, const void* pParameters, DWORD ParametersByteSize, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall GetEffectParameters		( DWORD EffectIndex, void* pParameters, DWORD ParametersByteSize ) = 0 ;
	virtual HRESULT __stdcall SetFilterParameters		( const D_XAUDIO2_FILTER_PARAMETERS* pParameters, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetFilterParameters		( D_XAUDIO2_FILTER_PARAMETERS* pParameters ) = 0 ;
	virtual HRESULT __stdcall SetOutputFilterParameters	( D_IXAudio2_8Voice* pDestinationVoice, const D_XAUDIO2_FILTER_PARAMETERS* pParameters, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetOutputFilterParameters	( D_IXAudio2_8Voice* pDestinationVoice, D_XAUDIO2_FILTER_PARAMETERS* pParameters ) = 0 ;
	virtual HRESULT __stdcall SetVolume					( float Volume, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetVolume					( float* pVolume ) = 0 ;
	virtual HRESULT __stdcall SetChannelVolumes			( DWORD Channels, const float* pVolumes, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetChannelVolumes			( DWORD Channels, float* pVolumes ) = 0 ;
	virtual HRESULT __stdcall SetOutputMatrix			( D_IXAudio2_8Voice* pDestinationVoice, DWORD SourceChannels, DWORD DestinationChannels, const float* pLevelMatrix, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetOutputMatrix			( D_IXAudio2_8Voice* pDestinationVoice, DWORD SourceChannels, DWORD DestinationChannels, float* pLevelMatrix ) = 0 ;
	virtual void    __stdcall DestroyVoice				( void ) = 0 ;
} ;

class D_IXAudio2SubmixVoice : public D_IXAudio2Voice
{
};

class D_IXAudio2_8SubmixVoice : public D_IXAudio2_8Voice
{
};

class D_IXAudio2MasteringVoice : public D_IXAudio2Voice
{
};

class D_IXAudio2_8MasteringVoice : public D_IXAudio2_8Voice
{
public :
	virtual HRESULT __stdcall GetChannelMask			( DWORD * pChannelmask ) = 0 ;
};

class D_IXAudio2SourceVoice : public D_IXAudio2Voice
{
public:
	virtual HRESULT __stdcall Start						( DWORD Flags = 0, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall Stop						( DWORD Flags = 0, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall SubmitSourceBuffer		( const D_XAUDIO2_BUFFER* pBuffer,  const D_XAUDIO2_BUFFER_WMA* pBufferWMA = NULL ) = 0 ;
	virtual HRESULT __stdcall FlushSourceBuffers		( void ) = 0 ;
	virtual HRESULT __stdcall Discontinuity				( void ) = 0 ;
	virtual HRESULT __stdcall ExitLoop					( DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetState					( D_XAUDIO2_VOICE_STATE* pVoiceState ) = 0 ;
	virtual HRESULT __stdcall SetFrequencyRatio			( float Ratio, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetFrequencyRatio			( float* pRatio ) = 0 ;
	virtual HRESULT __stdcall SetSourceSampleRate		( DWORD NewSourceSampleRate ) = 0 ;
};

class D_IXAudio2_8SourceVoice : public D_IXAudio2_8Voice
{
public:
	virtual HRESULT __stdcall Start						( DWORD Flags = 0, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall Stop						( DWORD Flags = 0, DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual HRESULT __stdcall SubmitSourceBuffer		( const D_XAUDIO2_BUFFER* pBuffer, const D_XAUDIO2_BUFFER_WMA* pBufferWMA = NULL ) = 0 ;
	virtual HRESULT __stdcall FlushSourceBuffers		( void ) = 0 ;
	virtual HRESULT __stdcall Discontinuity				( void ) = 0 ;
	virtual HRESULT __stdcall ExitLoop					( DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetState					( D_XAUDIO2_VOICE_STATE* pVoiceState, DWORD Flags = 0 ) = 0 ;
	virtual HRESULT __stdcall SetFrequencyRatio			( float Ratio,  DWORD OperationSet = D_XAUDIO2_COMMIT_NOW ) = 0 ;
	virtual void    __stdcall GetFrequencyRatio			( float* pRatio ) = 0 ;
	virtual HRESULT __stdcall SetSourceSampleRate		( DWORD NewSourceSampleRate ) = 0 ;
} ;

#ifndef DX_GCC_COMPILE
#pragma pack(pop)
#pragma pack()
#else
#pragma pack(pop)
#endif

// ＷＡＳＡＰＩ ---------------------------------------------------------------

typedef long									D_MUSIC_TIME ;
typedef LONGLONG								D_REFERENCE_TIME ;

#if 0

enum D_EDataFlow
{
	D_eRender,
	D_eCapture,
	D_eAll,
	D_EDataFlow_enum_count 
} ;

enum D_ERole
{
	D_eConsole,
	D_eMultimedia,
	D_eCommunications,
	D_ERole_enum_count 
} ;

enum D_AUDCLNT_SHAREMODE
{ 
	D_AUDCLNT_SHAREMODE_SHARED, 
	D_AUDCLNT_SHAREMODE_EXCLUSIVE 
} ;

#define D_DEVICE_STATE_ACTIVE							(0x00000001)
#define D_DEVICE_STATE_DISABLED							(0x00000002)
#define D_DEVICE_STATE_NOTPRESENT						(0x00000004)
#define D_DEVICE_STATE_UNPLUGGED						(0x00000008)
#define D_DEVICE_STATEMASK_ALL							(0x0000000f)

#define D_AUDCLNT_STREAMFLAGS_CROSSPROCESS				(0x00010000)
#define D_AUDCLNT_STREAMFLAGS_LOOPBACK					(0x00020000)
#define D_AUDCLNT_STREAMFLAGS_EVENTCALLBACK				(0x00040000)
#define D_AUDCLNT_STREAMFLAGS_NOPERSIST					(0x00080000)
#define D_AUDCLNT_STREAMFLAGS_RATEADJUST				(0x00100000)
#define D_AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED		(0x10000000)
#define D_AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE				(0x20000000)
#define D_AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED	(0x40000000)

class D_IPropertyStore : public IUnknown
{
public:
	virtual HRESULT __stdcall GetCount					( DWORD *cProps ) = 0 ;
	virtual HRESULT __stdcall GetAt						( DWORD iProp, PROPERTYKEY *pkey ) = 0 ;
	virtual HRESULT __stdcall GetValue					( PROPERTYKEY &key, PROPVARIANT *pv ) = 0 ;
	virtual HRESULT __stdcall SetValue					( PROPERTYKEY &key, PROPVARIANT &propvar ) = 0 ;
	virtual HRESULT __stdcall Commit					( void ) = 0 ;
};

class D_IMMDevice : public IUnknown
{
public:
	virtual  HRESULT __stdcall Activate					( REFIID iid, DWORD dwClsCtx, PROPVARIANT *pActivationParams, void **ppInterface ) = 0 ;
	virtual  HRESULT __stdcall OpenPropertyStore		( DWORD stgmAccess, D_IPropertyStore **ppProperties ) = 0 ;
	virtual  HRESULT __stdcall GetId					( LPWSTR *ppstrId ) = 0 ;
	virtual  HRESULT __stdcall GetState					( DWORD *pdwState ) = 0 ;
} ;

class D_IMMNotificationClient : public IUnknown
{
public:
	virtual HRESULT __stdcall OnDeviceStateChanged		( LPCWSTR pwstrDeviceId, DWORD dwNewState ) = 0 ;
	virtual HRESULT __stdcall OnDeviceAdded				( LPCWSTR pwstrDeviceId ) = 0 ;
	virtual HRESULT __stdcall OnDeviceRemoved			( LPCWSTR pwstrDeviceId ) = 0 ;
	virtual HRESULT __stdcall OnDefaultDeviceChanged	( D_EDataFlow flow, D_ERole role, LPCWSTR pwstrDefaultDeviceId ) = 0 ;
	virtual HRESULT __stdcall OnPropertyValueChanged	( LPCWSTR pwstrDeviceId, const PROPERTYKEY key ) = 0 ;
} ;

class D_IMMDeviceCollection : public IUnknown
{
public:
	virtual HRESULT __stdcall GetCount					( UINT *pcDevices ) = 0 ;
	virtual HRESULT __stdcall Item						( UINT nDevice, D_IMMDevice **ppDevice ) = 0 ;
} ;

class D_IMMDeviceEnumerator : public IUnknown
{
public:
	virtual HRESULT __stdcall EnumAudioEndpoints		( D_EDataFlow dataFlow, DWORD dwStateMask, D_IMMDeviceCollection **ppDevices ) = 0 ;
	virtual HRESULT __stdcall GetDefaultAudioEndpoint	( D_EDataFlow dataFlow, D_ERole role, D_IMMDevice **ppEndpoint ) = 0 ;
	virtual HRESULT __stdcall GetDevice					( LPCWSTR pwstrId, D_IMMDevice **ppDevice ) = 0 ;
	virtual HRESULT __stdcall RegisterEndpointNotificationCallback( D_IMMNotificationClient *pClient ) = 0 ;
	virtual HRESULT __stdcall UnregisterEndpointNotificationCallback( D_IMMNotificationClient *pClient ) = 0 ;
} ;

class D_IAudioRenderClient : public IUnknown
{
public:
	virtual HRESULT __stdcall GetBuffer					( DWORD NumFramesRequested, BYTE **ppData ) = 0 ;
	virtual HRESULT __stdcall ReleaseBuffer				( DWORD NumFramesWritten, DWORD dwFlags ) = 0 ;
} ;

class D_IAudioClient : public IUnknown
{
public:
	virtual HRESULT __stdcall Initialize				( D_AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags, D_REFERENCE_TIME hnsBufferDuration, D_REFERENCE_TIME hnsPeriodicity, const WAVEFORMATEX *pFormat, LPCGUID AudioSessionGuid ) = 0 ;
	virtual HRESULT __stdcall GetBufferSize				( DWORD *pNumBufferFrames ) = 0 ;
	virtual HRESULT __stdcall GetStreamLatency			( D_REFERENCE_TIME *phnsLatency ) = 0 ;
	virtual HRESULT __stdcall GetCurrentPadding			( DWORD *pNumPaddingFrames ) = 0 ;
	virtual HRESULT __stdcall IsFormatSupported			( D_AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX *pFormat, WAVEFORMATEX **ppClosestMatch ) = 0 ;
	virtual HRESULT __stdcall GetMixFormat				( WAVEFORMATEX **ppDeviceFormat ) = 0 ;
	virtual HRESULT __stdcall GetDevicePeriod			( D_REFERENCE_TIME *phnsDefaultDevicePeriod, D_REFERENCE_TIME *phnsMinimumDevicePeriod ) = 0 ;
	virtual HRESULT __stdcall Start						( void ) = 0 ;
	virtual HRESULT __stdcall Stop						( void ) = 0 ;
	virtual HRESULT __stdcall Reset						( void ) = 0 ;
	virtual HRESULT __stdcall SetEventHandle			( HANDLE eventHandle ) = 0 ;
	virtual HRESULT __stdcall GetService				( REFIID riid, void **ppv ) = 0 ;
} ;

#endif

#endif // __WINDOWS__

// ＤｉｒｅｃｔＭｕｓｉｃ -----------------------------------------------------

#ifdef __WINDOWS__

#define D_DMUS_APATH_SHARED_STEREOPLUSREVERB	(1)
#define D_DMUS_AUDIOF_ALL						(0x3F)
#define D_DMUS_PC_OUTPUTCLASS					(1)
#define D_DMUS_SEG_REPEAT_INFINITE				(0xFFFFFFFF)

#define D_DMUS_MAX_DESCRIPTION					(128)
#define D_DMUS_MAX_CATEGORY						(64)
#define D_DMUS_MAX_NAME							(64)
#define D_DMUS_MAX_FILENAME						MAX_PATH

#define D_DMUS_AUDIOPARAMS_FEATURES				(0x00000001)
#define D_DMUS_AUDIOPARAMS_SAMPLERATE			(0x00000004)
#define D_DMUS_AUDIOPARAMS_DEFAULTSYNTH			(0x00000008)

#define D_DMUS_OBJ_CLASS						(1 << 1)
#define D_DMUS_OBJ_MEMORY						(1 << 10)

typedef enum tagD_DMUS_SEGF_FLAGS
{
	D_DMUS_SEGF_REFTIME							= 1 << 6,
} D_DMUS_SEGF_FLAGS ;

typedef struct tagD_DMUS_PORTCAPS
{
	DWORD										dwSize;
	DWORD										dwFlags;
	GUID										guidPort;
	DWORD										dwClass;
	DWORD										dwType;
	DWORD										dwMemorySize;
	DWORD										dwMaxChannelGroups;
	DWORD										dwMaxVoices;	
	DWORD										dwMaxAudioChannels;
	DWORD										dwEffectFlags;
	WCHAR										wszDescription[D_DMUS_MAX_DESCRIPTION];
} D_DMUS_PORTCAPS ;

typedef struct tagD_DMUS_VERSION
{
	DWORD										dwVersionMS;
	DWORD										dwVersionLS;
} D_DMUS_VERSION ;

typedef struct tagD_DMUS_OBJECTDESC
{
	DWORD										dwSize;
	DWORD										dwValidData;
	GUID										guidObject;
	GUID										guidClass;
	FILETIME									ftDate;
	D_DMUS_VERSION								vVersion;
	WCHAR										wszName[D_DMUS_MAX_NAME];
	WCHAR										wszCategory[D_DMUS_MAX_CATEGORY];
	WCHAR										wszFileName[D_DMUS_MAX_FILENAME];
	LONGLONG									llMemLength;
	BYTE										*pbMemData;
	IStream 									*pStream;
} D_DMUS_OBJECTDESC ;

typedef struct tagD_DMUS_AUDIOPARAMS
{
	DWORD										dwSize;
	BOOL										fInitNow;
	DWORD 										dwValidData;
	DWORD										dwFeatures;
	DWORD										dwVoices;
	DWORD										dwSampleRate;
	CLSID										clsidDefaultSynth;
} D_DMUS_AUDIOPARAMS ;

class D_IDirectMusic : public IUnknown
{
public :
	virtual HRESULT __stdcall EnumPort					( DWORD dwIndex, D_DMUS_PORTCAPS *pPortCaps ) = 0 ;
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// CreateMusicBuffer( LPDMUS_BUFFERDESC pBufferDesc, LPDIRECTMUSICBUFFER *ppBuffer, IUnknown * pUnkOuter ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// CreatePort( REFCLSID rclsidPort, LPDMUS_PORTPARAMS pPortParams, LPDIRECTMUSICPORT *ppPort, IUnknown * pUnkOuter ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// EnumMasterClock( DWORD dwIndex, LPDMUS_CLOCKINFO lpClockInfo ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// GetMasterClock( LPGUID pguidClock, IReferenceClock **ppReferenceClock ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// SetMasterClock( REFGUID rguidClock ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// Activate( BOOL fEnable ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// GetDefaultPort( LPGUID pguidPort ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// SetDirectSound( LPDIRECTSOUND pDirectSound, HWND hWnd ) = 0 ;
} ;

class D_IDirectMusic8 : public D_IDirectMusic
{
public :
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// SetExternalMasterClock( IReferenceClock *pClock ) = 0 ;
} ;

class D_IDirectMusicSegmentState : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetRepeats( DWORD* pdwRepeats ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// GetSegment( class IDirectMusicSegment** ppSegment ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetStartTime( MUSIC_TIME* pmtStart ) = 0 ;
	virtual HRESULT __stdcall GetSeek					( D_MUSIC_TIME *pmtSeek ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// GetStartPoint( MUSIC_TIME* pmtStart ) = 0 ;
} ;

class D_IDirectMusicSegment : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetLength( MUSIC_TIME* pmtLength ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// SetLength( MUSIC_TIME mtLength ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetRepeats( DWORD* pdwRepeats ) = 0 ;
	virtual HRESULT __stdcall SetRepeats				( DWORD	dwRepeats ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// GetDefaultResolution( DWORD* pdwResolution ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// SetDefaultResolution( DWORD	dwResolution ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// GetTrack( REFGUID rguidType, DWORD dwGroupBits, DWORD dwIndex, IDirectMusicTrack** ppTrack ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// GetTrackGroup( IDirectMusicTrack* pTrack, DWORD* pdwGroupBits ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// InsertTrack( IDirectMusicTrack* pTrack, DWORD dwGroupBits ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// RemoveTrack( IDirectMusicTrack* pTrack ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// InitPlay( IDirectMusicSegmentState** ppSegState, class IDirectMusicPerformance* pPerformance, DWORD dwFlags ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// GetGraph( IDirectMusicGraph** ppGraph ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// SetGraph( IDirectMusicGraph* pGraph ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// AddNotificationType( REFGUID rguidNotificationType ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// RemoveNotificationType( REFGUID rguidNotificationType ) = 0 ;
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// GetParam( REFGUID rguidType, DWORD dwGroupBits, DWORD dwIndex, MUSIC_TIME mtTime, MUSIC_TIME* pmtNext, void* pParam ) = 0 ; 
	virtual HRESULT __stdcall SetParam					( REFGUID rguidType, DWORD dwGroupBits, DWORD dwIndex, D_MUSIC_TIME mtTime, void *pParam ) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// Clone( MUSIC_TIME mtStart, MUSIC_TIME mtEnd, IDirectMusicSegment** ppSegment ) = 0 ;
	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// SetStartPoint( MUSIC_TIME mtStart ) = 0 ;
	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// GetStartPoint( MUSIC_TIME* pmtStart ) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// SetLoopPoints( MUSIC_TIME mtStart, MUSIC_TIME mtEnd ) = 0 ;
	virtual HRESULT __stdcall NonUse19					( void ) = 0 ;				// GetLoopPoints( MUSIC_TIME* pmtStart, MUSIC_TIME* pmtEnd ) = 0 ;
	virtual HRESULT __stdcall NonUse20					( void ) = 0 ;				// SetPChannelsUsed( DWORD dwNumPChannels, DWORD* paPChannels ) = 0 ;
} ;

class D_IDirectMusicSegment8 : public D_IDirectMusicSegment
{
public :
	virtual HRESULT __stdcall NonUse21					( void ) = 0 ;				// SetTrackConfig( REFGUID rguidTrackClassID, DWORD dwGroupBits, DWORD dwIndex, DWORD dwFlagsOn, DWORD dwFlagsOff ) = 0 ;
	virtual HRESULT __stdcall NonUse22					( void ) = 0 ;				// GetAudioPathConfig( IUnknown ** ppAudioPathConfig ) = 0 ;
	virtual HRESULT __stdcall NonUse23					( void ) = 0 ;				// Compose( MUSIC_TIME mtTime, IDirectMusicSegment* pFromSegment, IDirectMusicSegment* pToSegment, IDirectMusicSegment** ppComposedSegment ) = 0 ;
	virtual HRESULT __stdcall Download					( IUnknown *pAudioPath ) = 0 ;
	virtual HRESULT __stdcall Unload					( IUnknown *pAudioPath ) = 0 ;
} ;

class D_IDirectMusicLoader : public IUnknown
{
public :
	virtual HRESULT __stdcall GetObject					( D_DMUS_OBJECTDESC *pDesc, REFIID riid, void **ppv ) = 0 ;
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// SetObject( LPDMUS_OBJECTDESC pDesc ) = 0 ;
	virtual HRESULT __stdcall SetSearchDirectory		( REFGUID rguidClass, WCHAR *pwzPath, BOOL fClear ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// ScanDirectory( REFGUID rguidClass, WCHAR *pwzFileExtension, WCHAR *pwzScanFileName ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// CacheObject( IDirectMusicObject * pObject ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// ReleaseObject( IDirectMusicObject * pObject ) = 0 ;
	virtual HRESULT __stdcall ClearCache				( REFGUID rguidClass ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// EnableCache( REFGUID rguidClass, BOOL fEnable ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// EnumObject( REFGUID rguidClass, DWORD dwIndex, LPDMUS_OBJECTDESC pDesc ) = 0 ;
} ;

class D_IDirectMusicLoader8 : public D_IDirectMusicLoader
{
public :
	virtual void	__stdcall NonUse07					( void ) = 0 ;				// CollectGarbage( void ) = 0 ;
	virtual HRESULT __stdcall ReleaseObjectByUnknown( IUnknown *pObject ) = 0 ;
	virtual HRESULT __stdcall LoadObjectFromFile		( REFGUID rguidClassID, REFIID iidInterfaceID, WCHAR *pwzFilePath, void **ppObject ) = 0 ;
} ;

class D_IDirectMusicPerformance : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// Init( IDirectMusic** ppDirectMusic, LPDIRECTSOUND pDirectSound, HWND hWnd ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// PlaySegment( IDirectMusicSegment* pSegment, DWORD dwFlags, __int64 i64StartTime, IDirectMusicSegmentState** ppSegmentState ) = 0 ;
	virtual HRESULT __stdcall Stop						( D_IDirectMusicSegment *pSegment, D_IDirectMusicSegmentState *pSegmentState, D_MUSIC_TIME mtTime, DWORD dwFlags ) = 0 ;
	virtual HRESULT __stdcall GetSegmentState			( D_IDirectMusicSegmentState **ppSegmentState, D_MUSIC_TIME mtTime ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// SetPrepareTime( DWORD dwMilliSeconds ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// GetPrepareTime( DWORD* pdwMilliSeconds ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// SetBumperLength( DWORD dwMilliSeconds ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// GetBumperLength( DWORD* pdwMilliSeconds ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// SendPMsg( DMUS_PMSG* pPMSG ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// MusicToReferenceTime( MUSIC_TIME mtTime, D_REFERENCE_TIME* prtTime ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// ReferenceToMusicTime( D_REFERENCE_TIME rtTime, MUSIC_TIME* pmtTime ) = 0 ;
	virtual HRESULT __stdcall IsPlaying					( D_IDirectMusicSegment *pSegment, D_IDirectMusicSegmentState *pSegState ) = 0 ;
	virtual HRESULT __stdcall GetTime					( D_REFERENCE_TIME* prtNow, D_MUSIC_TIME *pmtNow ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// AllocPMsg( ULONG cb, DMUS_PMSG** ppPMSG ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// FreePMsg( DMUS_PMSG* pPMSG ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// GetGraph( IDirectMusicGraph** ppGraph ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// SetGraph( IDirectMusicGraph* pGraph ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// SetNotificationHandle( HANDLE hNotification, D_REFERENCE_TIME rtMinimum ) = 0 ;
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// GetNotificationPMsg( DMUS_NOTIFICATION_PMSG** ppNotificationPMsg ) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// AddNotificationType( REFGUID rguidNotificationType ) = 0 ;
	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// RemoveNotificationType( REFGUID rguidNotificationType ) = 0 ;
	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// AddPort( IDirectMusicPort* pPort ) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// RemovePort( IDirectMusicPort* pPort	) = 0 ;
	virtual HRESULT __stdcall NonUse19					( void ) = 0 ;				// AssignPChannelBlock( DWORD dwBlockNum, IDirectMusicPort* pPort, DWORD dwGroup	) = 0 ;
	virtual HRESULT __stdcall NonUse20					( void ) = 0 ;				// AssignPChannel( DWORD dwPChannel, IDirectMusicPort* pPort, DWORD dwGroup, DWORD dwMChannel ) = 0 ;
	virtual HRESULT __stdcall NonUse21					( void ) = 0 ;				// PChannelInfo( DWORD dwPChannel, IDirectMusicPort** ppPort, DWORD* pdwGroup, DWORD* pdwMChannel ) = 0 ;
	virtual HRESULT __stdcall NonUse22					( void ) = 0 ;				// DownloadInstrument( IDirectMusicInstrument* pInst, DWORD dwPChannel, IDirectMusicDownloadedInstrument** ppDownInst, DMUS_NOTERANGE* pNoteRanges, DWORD dwNumNoteRanges, IDirectMusicPort** ppPort, DWORD* pdwGroup, DWORD* pdwMChannel ) = 0 ;
	virtual HRESULT __stdcall NonUse23					( void ) = 0 ;				// Invalidate( MUSIC_TIME mtTime, DWORD dwFlags ) = 0 ;
	virtual HRESULT __stdcall NonUse24					( void ) = 0 ;				// GetParam( REFGUID rguidType, DWORD dwGroupBits, DWORD dwIndex, MUSIC_TIME mtTime, MUSIC_TIME* pmtNext, void* pParam ) = 0 ; 
	virtual HRESULT __stdcall NonUse25					( void ) = 0 ;				// SetParam( REFGUID rguidType, DWORD dwGroupBits, DWORD dwIndex, MUSIC_TIME mtTime, void* pParam ) = 0 ;
	virtual HRESULT __stdcall GetGlobalParam			( REFGUID rguidType, void *pParam, DWORD dwSize ) = 0 ;
	virtual HRESULT __stdcall SetGlobalParam			( REFGUID rguidType, void *pParam, DWORD dwSize ) = 0 ;
	virtual HRESULT __stdcall NonUse26					( void ) = 0 ;				// GetLatencyTime( D_REFERENCE_TIME* prtTime ) = 0 ;
	virtual HRESULT __stdcall NonUse27					( void ) = 0 ;				// GetQueueTime( D_REFERENCE_TIME* prtTime ) = 0 ;
	virtual HRESULT __stdcall NonUse28					( void ) = 0 ;				// AdjustTime( D_REFERENCE_TIME rtAmount ) = 0 ;
	virtual HRESULT __stdcall CloseDown					( void ) = 0 ;
	virtual HRESULT __stdcall NonUse29					( void ) = 0 ;				// GetResolvedTime( D_REFERENCE_TIME rtTime, D_REFERENCE_TIME* prtResolved, DWORD dwTimeResolveFlags ) = 0 ;
	virtual HRESULT __stdcall NonUse30					( void ) = 0 ;				// MIDIToMusic( BYTE bMIDIValue, DMUS_CHORD_KEY* pChord, BYTE bPlayMode, BYTE bChordLevel, WORD *pwMusicValue ) = 0 ;
	virtual HRESULT __stdcall NonUse31					( void ) = 0 ;				// MusicToMIDI( WORD wMusicValue, DMUS_CHORD_KEY* pChord, BYTE bPlayMode, BYTE bChordLevel, BYTE *pbMIDIValue ) = 0 ;
	virtual HRESULT __stdcall NonUse32					( void ) = 0 ;				// TimeToRhythm( MUSIC_TIME mtTime, DMUS_TIMESIGNATURE *pTimeSig, WORD *pwMeasure, BYTE *pbBeat, BYTE *pbGrid, short *pnOffset ) = 0 ;
	virtual HRESULT __stdcall NonUse33					( void ) = 0 ;				// RhythmToTime( WORD wMeasure, BYTE bBeat, BYTE bGrid, short nOffset, DMUS_TIMESIGNATURE *pTimeSig, MUSIC_TIME *pmtTime ) = 0 ;	
} ;

class D_IDirectMusicPerformance8 : public D_IDirectMusicPerformance
{
public :
	virtual HRESULT __stdcall InitAudio					( D_IDirectMusic **ppDirectMusic, D_IDirectSound **ppDirectSound, HWND hWnd, DWORD dwDefaultPathType, DWORD dwPChannelCount, DWORD dwFlags, D_DMUS_AUDIOPARAMS *pParams ) = 0 ;
	virtual HRESULT __stdcall PlaySegmentEx				( IUnknown *pSource, WCHAR *pwzSegmentName, IUnknown *pTransition, DWORD dwFlags, __int64 i64StartTime, D_IDirectMusicSegmentState **ppSegmentState, IUnknown *pFrom, IUnknown *pAudioPath ) = 0 ;
	virtual HRESULT __stdcall StopEx					( IUnknown *pObjectToStop, __int64 i64StopTime, DWORD dwFlags ) = 0 ;
	virtual HRESULT __stdcall NonUse34					( void ) = 0 ;				// ClonePMsg( DMUS_PMSG* pSourcePMSG, DMUS_PMSG** ppCopyPMSG ) = 0 ;
	virtual HRESULT __stdcall NonUse35					( void ) = 0 ;				// CreateAudioPath( IUnknown *pSourceConfig, BOOL fActivate, IDirectMusicAudioPath **ppNewPath ) = 0 ;
	virtual HRESULT __stdcall NonUse36					( void ) = 0 ;				// CreateStandardAudioPath( DWORD dwType, DWORD dwPChannelCount, BOOL fActivate, IDirectMusicAudioPath **ppNewPath ) = 0 ;
	virtual HRESULT __stdcall NonUse37					( void ) = 0 ;				// SetDefaultAudioPath( IDirectMusicAudioPath *pAudioPath ) = 0 ;
	virtual HRESULT __stdcall NonUse38					( void ) = 0 ;				// GetDefaultAudioPath( IDirectMusicAudioPath **ppAudioPath ) = 0 ;
	virtual HRESULT __stdcall NonUse39					( void ) = 0 ;				// GetParamEx( REFGUID rguidType, DWORD dwTrackID, DWORD dwGroupBits, DWORD dwIndex, MUSIC_TIME mtTime, MUSIC_TIME* pmtNext, void* pParam ) = 0 ;
} ;

#endif // __WINDOWS__

// ＤｉｒｅｃｔＤｒａｗ -------------------------------------------------------

#define D_DD_OK									S_OK

#define D_DD_ROP_SPACE							(256/32)
#define D_MAX_DDDEVICEID_STRING					(512)

#define D_DDPF_ALPHAPIXELS						(0x00000001l)
#define D_DDPF_ALPHA							(0x00000002l)
#define D_DDPF_FOURCC							(0x00000004l)
#define D_DDPF_RGB								(0x00000040l)
#define D_DDPF_ZBUFFER							(0x00000400l)
#define D_DDPF_LUMINANCE						(0x00020000l)
#define D_DDPF_BUMPLUMINANCE					(0x00040000l)
#define D_DDPF_BUMPDUDV							(0x00080000l)
#define D_DDPF_RGBTOYUV							(0x000001001)
#define D_DDPF_YUV								(0x000002001)

#define D_DDWAITVB_BLOCKBEGIN					(0x00000001l)

#define D_DDCAPS_ALIGNBOUNDARYDEST				(0x00000002l)
#define D_DDCAPS_ALIGNSIZEDEST					(0x00000004l)
#define D_DDCAPS_ALIGNBOUNDARYSRC				(0x00000008l)
#define D_DDCAPS_ALIGNSIZESRC					(0x00000010l)

#define D_DDENUMSURFACES_ALL					(0x00000001l)
#define D_DDENUMSURFACES_MATCH					(0x00000002l)
#define D_DDENUMSURFACES_NOMATCH				(0x00000004l)
#define D_DDENUMSURFACES_CANBECREATED			(0x00000008l)
#define D_DDENUMSURFACES_DOESEXIST				(0x00000010l)

#define D_DDOVER_SHOW							(0x00004000l)
#define D_DDOVER_HIDE							(0x00000200l)

#define D_DDSCAPS_BACKBUFFER					(0x00000004l)
#define D_DDSCAPS_COMPLEX						(0x00000008l)
#define D_DDSCAPS_FLIP							(0x00000010l)
#define D_DDSCAPS_OFFSCREENPLAIN				(0x00000040l)
#define D_DDSCAPS_OVERLAY						(0x00000080l)
#define D_DDSCAPS_PRIMARYSURFACE				(0x00000200l)
#define D_DDSCAPS_SYSTEMMEMORY					(0x00000800l)
#define D_DDSCAPS_TEXTURE						(0x00001000l)
#define D_DDSCAPS_3DDEVICE						(0x00002000l)
#define D_DDSCAPS_VIDEOMEMORY					(0x00004000l)
#define D_DDSCAPS_ZBUFFER						(0x00020000l)
#define D_DDSCAPS_MIPMAP						(0x00400000l)
#define D_DDSCAPS_LOCALVIDMEM					(0x10000000l)

#define D_DDCKEY_SRCBLT							(0x00000008l)

#define D_DDFLIP_NOVSYNC						(0x00000008L)
#define D_DDFLIP_WAIT							(0x00000001L)

#define D_DDPCAPS_8BIT							(0x00000004l)

#define D_DDSCAPS2_RESERVED4					(0x00000002L)
#define D_DDSCAPS2_HARDWAREDEINTERLACE			(0x00000000L)
#define D_DDSCAPS2_HINTDYNAMIC					(0x00000004L)
#define D_DDSCAPS2_HINTSTATIC					(0x00000008L)
#define D_DDSCAPS2_TEXTUREMANAGE				(0x00000010L)
#define D_DDSCAPS2_RESERVED1					(0x00000020L)
#define D_DDSCAPS2_RESERVED2					(0x00000040L)
#define D_DDSCAPS2_OPAQUE						(0x00000080L)
#define D_DDSCAPS2_HINTANTIALIASING				(0x00000100L)
#define D_DDSCAPS2_CUBEMAP						(0x00000200L)
#define D_DDSCAPS2_CUBEMAP_POSITIVEX			(0x00000400L)
#define D_DDSCAPS2_CUBEMAP_NEGATIVEX			(0x00000800L)
#define D_DDSCAPS2_CUBEMAP_POSITIVEY			(0x00001000L)
#define D_DDSCAPS2_CUBEMAP_NEGATIVEY			(0x00002000L)
#define D_DDSCAPS2_CUBEMAP_POSITIVEZ			(0x00004000L)
#define D_DDSCAPS2_CUBEMAP_NEGATIVEZ			(0x00008000L)
#define D_DDSCAPS2_CUBEMAP_ALLFACES				(	D_DDSCAPS2_CUBEMAP_POSITIVEX |\
													D_DDSCAPS2_CUBEMAP_NEGATIVEX |\
													D_DDSCAPS2_CUBEMAP_POSITIVEY |\
													D_DDSCAPS2_CUBEMAP_NEGATIVEY |\
													D_DDSCAPS2_CUBEMAP_POSITIVEZ |\
													D_DDSCAPS2_CUBEMAP_NEGATIVEZ )
#define D_DDSCAPS2_MIPMAPSUBLEVEL				(0x00010000L)
#define D_DDSCAPS2_D3DTEXTUREMANAGE				(0x00020000L)
#define D_DDSCAPS2_DONOTPERSIST					(0x00040000L)
#define D_DDSCAPS2_STEREOSURFACELEFT			(0x00080000L)
#define D_DDSCAPS2_VOLUME						(0x00200000L)
#define D_DDSCAPS2_NOTUSERLOCKABLE				(0x00400000L)
#define D_DDSCAPS2_POINTS						(0x00800000L)
#define D_DDSCAPS2_RTPATCHES					(0x01000000L)
#define D_DDSCAPS2_NPATCHES						(0x02000000L)
#define D_DDSCAPS2_RESERVED3					(0x04000000L)
#define D_DDSCAPS2_DISCARDBACKBUFFER			(0x10000000L)
#define D_DDSCAPS2_ENABLEALPHACHANNEL			(0x20000000L)
#define D_DDSCAPS2_EXTENDEDFORMATPRIMARY		(0x40000000L)
#define D_DDSCAPS2_ADDITIONALPRIMARY			(0x80000000L)
#define D_DDSCAPS3_MULTISAMPLE_MASK				(0x0000001FL)
#define D_DDSCAPS3_MULTISAMPLE_QUALITY_MASK		(0x000000E0L)
#define D_DDSCAPS3_MULTISAMPLE_QUALITY_SHIFT	(5)
#define D_DDSCAPS3_RESERVED1					(0x00000100L)
#define D_DDSCAPS3_RESERVED2					(0x00000200L)
#define D_DDSCAPS3_LIGHTWEIGHTMIPMAP			(0x00000400L)
#define D_DDSCAPS3_AUTOGENMIPMAP				(0x00000800L)
#define D_DDSCAPS3_DMAP							(0x00001000L)

#define D_DDBLT_COLORFILL						(0x00000400l)
#define D_DDBLT_WAIT							(0x01000000l)
#define D_DDBLT_DEPTHFILL						(0x02000000l)

#define D_DDENUM_ATTACHEDSECONDARYDEVICES		(0x00000001L)
#define D_DDENUMRET_CANCEL						(0)
#define D_DDENUMRET_OK							(1)

#define D_DDLOCK_WAIT							(0x00000001L)
#define D_DDLOCK_READONLY						(0x00000010L)
#define D_DDLOCK_WRITEONLY						(0x00000020L)
#define D_DDLOCK_NOSYSLOCK						(0x00000800L)

#define D_DDBLTFAST_NOCOLORKEY					(0x00000000)
#define D_DDBLTFAST_SRCCOLORKEY					(0x00000001)
#define D_DDBLTFAST_DESTCOLORKEY				(0x00000002)
#define D_DDBLTFAST_WAIT						(0x00000010)
#define D_DDBLTFAST_DONOTWAIT					(0x00000020)

#define D_DDSD_CAPS								(0x00000001l)
#define D_DDSD_HEIGHT							(0x00000002l)
#define D_DDSD_WIDTH							(0x00000004l)
#define D_DDSD_PITCH							(0x00000008l)
#define D_DDSD_BACKBUFFERCOUNT					(0x00000020l)
#define D_DDSD_ZBUFFERBITDEPTH					(0x00000040l)
#define D_DDSD_ALPHABITDEPTH					(0x00000080l)
#define D_DDSD_LPSURFACE						(0x00000800l)
#define D_DDSD_PIXELFORMAT						(0x00001000l)
#define D_DDSD_CKDESTOVERLAY					(0x00002000l)
#define D_DDSD_CKDESTBLT						(0x00004000l)
#define D_DDSD_CKSRCOVERLAY						(0x00008000l)
#define D_DDSD_CKSRCBLT							(0x00010000l)
#define D_DDSD_MIPMAPCOUNT						(0x00020000l)
#define D_DDSD_REFRESHRATE						(0x00040000l)
#define D_DDSD_LINEARSIZE						(0x00080000l)
#define D_DDSD_TEXTURESTAGE						(0x00100000l)
#define D_DDSD_FVF								(0x00200000l)
#define D_DDSD_SRCVBHANDLE						(0x00400000l)
#define D_DDSD_DEPTH							(0x00800000l)

#define D_DDBD_16								(0x00000400l)
#define D_DDBD_32								(0x00000100l)

#define D_DDSCL_FULLSCREEN						(0x00000001l)
#define D_DDSCL_NORMAL							(0x00000008l)
#define D_DDSCL_EXCLUSIVE						(0x00000010l)
#define D_DDSCL_MULTITHREADED					(0x00000400l)
#define D_DDSCL_FPUPRESERVE						(0x00001000l)

#define D_MAKE_DDHRESULT( code )				MAKE_HRESULT( 1, 0x876, code )
#define D_DDERR_SURFACEBUSY						D_MAKE_DDHRESULT( 430 )
#define D_DDERR_SURFACELOST						D_MAKE_DDHRESULT( 450 )
#define D_DDERR_HWNDSUBCLASSED					D_MAKE_DDHRESULT( 570 )
#define D_DDERR_HWNDALREADYSET					D_MAKE_DDHRESULT( 571 )
#define D_DDERR_EXCLUSIVEMODEALREADYSET			D_MAKE_DDHRESULT( 581 )

#define D_DDEDM_REFRESHRATES					(0x00000001l)
#define D_DDEDM_STANDARDVGAMODES				(0x00000002l)

typedef struct tagD_DDSCAPS2
{
	DWORD										dwCaps;
	DWORD										dwCaps2;
	DWORD										dwCaps3;
	union
	{
		DWORD									dwCaps4;
		DWORD									dwVolumeDepth;
	} ;
} D_DDSCAPS2 ;

typedef struct tagD_DDSCAPS
{
	DWORD										dwCaps;
} D_DDSCAPS ;

typedef struct tagD_DDCAPS
{
	DWORD										dwSize;
	DWORD										dwCaps;
	DWORD										dwCaps2;
	DWORD										dwCKeyCaps;
	DWORD										dwFXCaps;
	DWORD										dwFXAlphaCaps;
	DWORD										dwPalCaps;
	DWORD										dwSVCaps;
	DWORD										dwAlphaBltConstBitDepths;
	DWORD										dwAlphaBltPixelBitDepths;
	DWORD										dwAlphaBltSurfaceBitDepths;
	DWORD										dwAlphaOverlayConstBitDepths;
	DWORD										dwAlphaOverlayPixelBitDepths;
	DWORD										dwAlphaOverlaySurfaceBitDepths;
	DWORD										dwZBufferBitDepths;
	DWORD										dwVidMemTotal;
	DWORD										dwVidMemFree;
	DWORD										dwMaxVisibleOverlays;
	DWORD										dwCurrVisibleOverlays;
	DWORD										dwNumFourCCCodes;
	DWORD										dwAlignBoundarySrc;
	DWORD										dwAlignSizeSrc;
	DWORD										dwAlignBoundaryDest;
	DWORD										dwAlignSizeDest;
	DWORD										dwAlignStrideAlign;
	DWORD										dwRops[D_DD_ROP_SPACE];
	D_DDSCAPS									ddsOldCaps;
	DWORD										dwMinOverlayStretch;
	DWORD										dwMaxOverlayStretch;
	DWORD										dwMinLiveVideoStretch;
	DWORD										dwMaxLiveVideoStretch;
	DWORD										dwMinHwCodecStretch;
	DWORD										dwMaxHwCodecStretch;
	DWORD										dwReserved1;
	DWORD										dwReserved2;
	DWORD										dwReserved3;
	DWORD										dwSVBCaps;
	DWORD										dwSVBCKeyCaps;
	DWORD										dwSVBFXCaps;
	DWORD										dwSVBRops[D_DD_ROP_SPACE];
	DWORD										dwVSBCaps;
	DWORD										dwVSBCKeyCaps;
	DWORD										dwVSBFXCaps;
	DWORD										dwVSBRops[D_DD_ROP_SPACE];
	DWORD										dwSSBCaps;
	DWORD										dwSSBCKeyCaps;
	DWORD										dwSSBFXCaps;
	DWORD										dwSSBRops[D_DD_ROP_SPACE];
	DWORD										dwMaxVideoPorts;
	DWORD										dwCurrVideoPorts;
	DWORD										dwSVBCaps2;
	DWORD										dwNLVBCaps;
	DWORD										dwNLVBCaps2;
	DWORD										dwNLVBCKeyCaps;
	DWORD										dwNLVBFXCaps;
	DWORD										dwNLVBRops[D_DD_ROP_SPACE];
	// DirectX6
	D_DDSCAPS2									ddsCaps;
} D_DDCAPS ;

typedef struct tagD_DDPIXELFORMAT
{
	DWORD										dwSize;
	DWORD										dwFlags;
	DWORD										dwFourCC;
	union
	{
		DWORD									dwRGBBitCount;
		DWORD									dwYUVBitCount;
		DWORD									dwZBufferBitDepth;
		DWORD									dwAlphaBitDepth;
		DWORD									dwLuminanceBitCount;
		DWORD									dwBumpBitCount;
		DWORD									dwPrivateFormatBitCount;
	} ;
	union
	{
		DWORD									dwRBitMask;
		DWORD									dwYBitMask;
		DWORD									dwStencilBitDepth;
		DWORD									dwLuminanceBitMask;
		DWORD									dwBumpDuBitMask;
		DWORD									dwOperations;
	} ;
	union
	{
		DWORD									dwGBitMask;
		DWORD									dwUBitMask;
		DWORD									dwZBitMask;
		DWORD									dwBumpDvBitMask;
		struct
		{
			WORD								wFlipMSTypes;
			WORD								wBltMSTypes;
		} MultiSampleCaps;

	} ;
	union
	{
		DWORD									dwBBitMask;
		DWORD									dwVBitMask;
		DWORD									dwStencilBitMask;
		DWORD									dwBumpLuminanceBitMask;
	} ;
	union
	{
		DWORD									dwRGBAlphaBitMask;
		DWORD									dwYUVAlphaBitMask;
		DWORD									dwLuminanceAlphaBitMask;
		DWORD									dwRGBZBitMask;
		DWORD									dwYUVZBitMask;
	} ;
} D_DDPIXELFORMAT ;

typedef struct tagD_DDCOLORKEY
{
	DWORD										dwColorSpaceLowValue;
	DWORD										dwColorSpaceHighValue;
} D_DDCOLORKEY ;

typedef struct tagD_DDSURFACEDESC
{
	DWORD										dwSize;
	DWORD										dwFlags;
	DWORD										dwHeight;
	DWORD										dwWidth;
	union
	{
		LONG									lPitch;
		DWORD									dwLinearSize;
	} ;
	DWORD										dwBackBufferCount;
	union
	{
		DWORD									dwMipMapCount;
		DWORD									dwZBufferBitDepth;
		DWORD									dwRefreshRate;
	} ;
	DWORD										dwAlphaBitDepth;
	DWORD										dwReserved;
	LPVOID										lpSurface;
	D_DDCOLORKEY								ddckCKDestOverlay;
	D_DDCOLORKEY								ddckCKDestBlt;
	D_DDCOLORKEY								ddckCKSrcOverlay;
	D_DDCOLORKEY								ddckCKSrcBlt;
	D_DDPIXELFORMAT								ddpfPixelFormat;
	D_DDSCAPS									ddsCaps;
} D_DDSURFACEDESC ;

typedef struct tagD_DDSURFACEDESC2
{
	DWORD										dwSize;
	DWORD										dwFlags;
	DWORD										dwHeight;
	DWORD										dwWidth;
	union
	{
		LONG									lPitch;
		DWORD									dwLinearSize;
	} ;
	union
	{
		DWORD									dwBackBufferCount;
		DWORD									dwDepth;
	} ;
	union
	{
		DWORD									dwMipMapCount;
		DWORD									dwRefreshRate;
		DWORD									dwSrcVBHandle;
	} ;
	DWORD										dwAlphaBitDepth;
	DWORD										dwReserved;
	LPVOID										lpSurface;
	union
	{
		D_DDCOLORKEY							ddckCKDestOverlay;
		DWORD									dwEmptyFaceColor;
	} ;
	D_DDCOLORKEY								ddckCKDestBlt;
	D_DDCOLORKEY								ddckCKSrcOverlay;
	D_DDCOLORKEY								ddckCKSrcBlt;
	union
	{
		D_DDPIXELFORMAT							ddpfPixelFormat;
		DWORD									dwFVF;
	} ;
	D_DDSCAPS2									ddsCaps;
	DWORD										dwTextureStage;
} D_DDSURFACEDESC2 ;

typedef struct tagD_DDDEVICEIDENTIFIER2
{
	char										szDriver[D_MAX_DDDEVICEID_STRING];
	char										szDescription[D_MAX_DDDEVICEID_STRING];

	LARGE_INTEGER								liDriverVersion;

	DWORD										dwVendorId;
	DWORD										dwDeviceId;
	DWORD										dwSubSysId;
	DWORD										dwRevision;
	GUID										guidDeviceIdentifier;
	DWORD										dwWHQLLevel;

} D_DDDEVICEIDENTIFIER2 ;

typedef struct tagD_DDBLTFX
{
	DWORD										dwSize;
	DWORD										dwDDFX;
	DWORD										dwROP;
	DWORD										dwDDROP;
	DWORD										dwRotationAngle;
	DWORD										dwZBufferOpCode;
	DWORD										dwZBufferLow;
	DWORD										dwZBufferHigh;
	DWORD										dwZBufferBaseDest;
	DWORD										dwZDestConstBitDepth;
	union
	{
		DWORD									dwZDestConst;
		class D_IDirectDrawSurface 				*lpDDSZBufferDest;
	} ;
	DWORD										dwZSrcConstBitDepth;
	union
	{
		DWORD									dwZSrcConst;
		class D_IDirectDrawSurface 				*lpDDSZBufferSrc;
	} ;
	DWORD										dwAlphaEdgeBlendBitDepth;
	DWORD										dwAlphaEdgeBlend;
	DWORD										dwReserved;
	DWORD										dwAlphaDestConstBitDepth;
	union
	{
		DWORD									dwAlphaDestConst;
		class D_IDirectDrawSurface 				*lpDDSAlphaDest;
	} ;
	DWORD										dwAlphaSrcConstBitDepth;
	union
	{
		DWORD									dwAlphaSrcConst;
		class D_IDirectDrawSurface 				*lpDDSAlphaSrc;
	} ;
	union
	{
		DWORD									dwFillColor;
		DWORD									dwFillDepth;
		DWORD									dwFillPixel;
		class D_IDirectDrawSurface 				*lpDDSPattern;
	} ;
	D_DDCOLORKEY								ddckDestColorkey;
	D_DDCOLORKEY								ddckSrcColorkey;
} D_DDBLTFX ;

typedef struct tagD_DDOVERLAYFX
{
	DWORD										dwSize;
	DWORD										dwAlphaEdgeBlendBitDepth;
	DWORD										dwAlphaEdgeBlend;
	DWORD										dwReserved;
	DWORD										dwAlphaDestConstBitDepth;
	union
	{
		DWORD									dwAlphaDestConst;
		class D_IDirectDrawSurface 				*lpDDSAlphaDest;
	} ;
	DWORD										dwAlphaSrcConstBitDepth;
	union
	{
		DWORD									dwAlphaSrcConst;
		class D_IDirectDrawSurface 				*lpDDSAlphaSrc;
	} ;
	D_DDCOLORKEY								dckDestColorkey;
	D_DDCOLORKEY								dckSrcColorkey;
	DWORD										dwDDFX;
	DWORD										dwFlags;
} D_DDOVERLAYFX ;

#ifdef __WINDOWS__

typedef HRESULT ( FAR PASCAL * LPD_DDENUMSURFACESCALLBACK7 )	( class D_IDirectDrawSurface7 *, D_DDSURFACEDESC2 *, void * ) ;
typedef HRESULT ( FAR PASCAL * LPD_DDENUMSURFACESCALLBACK2 )	( class D_IDirectDrawSurface4 *, D_DDSURFACEDESC2 *, void * ) ;
typedef HRESULT ( FAR PASCAL * LPD_DDENUMSURFACESCALLBACK )		( class D_IDirectDrawSurface *,  D_DDSURFACEDESC *,  void * ) ;
typedef HRESULT ( FAR PASCAL * LPD_DDENUMMODESCALLBACK2)		( D_DDSURFACEDESC2 *, void * ) ;
typedef BOOL 	( FAR PASCAL * LPD_DDENUMCALLBACKEXA )			( GUID *, LPSTR,  LPSTR,  void *, void * ) ;
typedef BOOL 	( FAR PASCAL * LPD_DDENUMCALLBACKEXW )			( GUID *, LPWSTR, LPWSTR, void *, void * ) ;

class D_IDirectDraw : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// Compact( void ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// CreateClipper( DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// CreatePalette( DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) = 0 ;
	virtual HRESULT __stdcall CreateSurface				( D_DDSURFACEDESC *, D_IDirectDrawSurface **, IUnknown * ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// DuplicateSurface( LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// EnumDisplayModes( DWORD, LPDDSURFACEDESC, LPVOID, LPDDENUMMODESCALLBACK ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// EnumSurfaces( DWORD, LPDDSURFACEDESC, LPVOID,LPDDENUMSURFACESCALLBACK ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// FlipToGDISurface( void ) = 0 ;
	virtual HRESULT __stdcall GetCaps					( D_DDCAPS *, D_DDCAPS * ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// GetDisplayMode( LPDDSURFACEDESC ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// GetFourCCCodes( LPDWORD, LPDWORD	) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// GetGDISurface( LPDIRECTDRAWSURFACE FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// GetMonitorFrequency( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// GetScanLine( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// GetVerticalBlankStatus( LPBOOL ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// Initialize( GUID FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// RestoreDisplayMode( void ) = 0 ;
	virtual HRESULT __stdcall SetCooperativeLevel		( HWND, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// SetDisplayMode( DWORD, DWORD,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// WaitForVerticalBlank( DWORD, HANDLE ) = 0 ;
} ;

class D_IDirectDraw2 : public D_IDirectDraw
{
public :
	virtual HRESULT __stdcall GetAvailableVidMem		( D_DDSCAPS2 *, DWORD *, DWORD * ) = 0 ;
} ;

class D_IDirectDraw4 : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// Compact( void ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// CreateClipper( DWORD, LPDIRECTDRAWCLIPPER FAR*, IUnknown FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// CreatePalette( DWORD, LPPALETTEENTRY, LPDIRECTDRAWPALETTE FAR*, IUnknown FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// CreateSurface(	LPDDSURFACEDESC2, LPDIRECTDRAWSURFACE4 FAR *, IUnknown FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// DuplicateSurface( LPDIRECTDRAWSURFACE4, LPDIRECTDRAWSURFACE4 FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// EnumDisplayModes( DWORD, LPDDSURFACEDESC2, LPVOID, LPDDENUMMODESCALLBACK2 ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// EnumSurfaces( DWORD, LPDDSURFACEDESC2, LPVOID,LPDDENUMSURFACESCALLBACK2 ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// FlipToGDISurface( void ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// GetCaps( LPDDCAPS, LPDDCAPS ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// GetDisplayMode( LPDDSURFACEDESC2 ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// GetFourCCCodes( LPDWORD, LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// GetGDISurface( LPDIRECTDRAWSURFACE4 FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// GetMonitorFrequency( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// GetScanLine( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// GetVerticalBlankStatus( LPBOOL ) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// Initialize( GUID FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// RestoreDisplayMode( void ) = 0 ;
	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// SetCooperativeLevel( HWND, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// SetDisplayMode( DWORD, DWORD,DWORD, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse19					( void ) = 0 ;				// WaitForVerticalBlank( DWORD, HANDLE ) = 0 ;

	virtual HRESULT __stdcall NonUse20					( void ) = 0 ;				// GetAvailableVidMem( LPDDSCAPS2, LPDWORD, LPDWORD ) = 0 ;

	virtual HRESULT __stdcall NonUse21					( void ) = 0 ;				// GetSurfaceFromDC( HDC, LPDIRECTDRAWSURFACE4 * ) = 0 ;
	virtual HRESULT __stdcall NonUse22					( void ) = 0 ;				// RestoreAllSurfaces( void ) = 0 ;
	virtual HRESULT __stdcall NonUse23					( void ) = 0 ;				// TestCooperativeLevel( void ) = 0 ;
	virtual HRESULT __stdcall NonUse24					( void ) = 0 ;				// GetDeviceIdentifier( LPDDDEVICEIDENTIFIER, DWORD ) = 0 ;
} ;

class D_IDirectDraw7 : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// Compact( void ) = 0 ;
	virtual HRESULT __stdcall CreateClipper				( DWORD, class D_IDirectDrawClipper **, IUnknown * ) = 0 ;
	virtual HRESULT __stdcall CreatePalette				( DWORD, LPPALETTEENTRY, class D_IDirectDrawPalette **, IUnknown * ) = 0 ;
	virtual HRESULT __stdcall CreateSurface				( D_DDSURFACEDESC2 *, D_IDirectDrawSurface7 **, IUnknown * ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// DuplicateSurface( D_IDirectDrawSurface7 *, D_IDirectDrawSurface7 * FAR * ) = 0 ;
	virtual HRESULT __stdcall EnumDisplayModes			( DWORD, D_DDSURFACEDESC2 *, LPVOID, LPD_DDENUMMODESCALLBACK2 ) = 0 ;
	virtual HRESULT __stdcall EnumSurfaces				( DWORD, D_DDSURFACEDESC2 *, LPVOID, LPD_DDENUMSURFACESCALLBACK7 ) = 0 ;
	virtual HRESULT __stdcall FlipToGDISurface			( void ) = 0 ;
	virtual HRESULT __stdcall GetCaps					( D_DDCAPS *, D_DDCAPS * ) = 0 ;
	virtual HRESULT __stdcall GetDisplayMode			( D_DDSURFACEDESC2 * ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// GetFourCCCodes( LPDWORD, LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// GetGDISurface( D_IDirectDrawSurface7 * FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// GetMonitorFrequency( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall GetScanLine				( DWORD * ) = 0 ;
	virtual HRESULT __stdcall GetVerticalBlankStatus	( BOOL * ) = 0 ;
	virtual HRESULT __stdcall Initialize				( GUID * ) = 0 ;
	virtual HRESULT __stdcall RestoreDisplayMode		( void ) = 0 ;
	virtual HRESULT __stdcall SetCooperativeLevel		( HWND, DWORD ) = 0 ;
	virtual HRESULT __stdcall SetDisplayMode			( DWORD, DWORD,DWORD, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall WaitForVerticalBlank		( DWORD, HANDLE ) = 0 ;

	virtual HRESULT __stdcall GetAvailableVidMem		( D_DDSCAPS2 *, DWORD *, DWORD * ) = 0 ;

	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// GetSurfaceFromDC( HDC, D_IDirectDrawSurface7 * * ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// RestoreAllSurfaces( void ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// TestCooperativeLevel( void ) = 0 ;
	virtual HRESULT __stdcall GetDeviceIdentifier		( D_DDDEVICEIDENTIFIER2 *, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// StartModeTest( LPSIZE, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// EvaluateMode( DWORD, DWORD * ) = 0 ;
} ;

class D_IDirectDrawSurface : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// AddAttachedSurface( LPDIRECTDRAWSURFACE ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// AddOverlayDirtyRect( LPRECT ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// Blt( LPRECT,LPDIRECTDRAWSURFACE, LPRECT,DWORD, LPDDBLTFX ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// BltBatch( LPDDBLTBATCH, DWORD, DWORD	) = 0 ;
	virtual HRESULT __stdcall BltFast					( DWORD, DWORD, D_IDirectDrawSurface *, LPRECT, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// DeleteAttachedSurface( DWORD,LPDIRECTDRAWSURFACE ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// EnumAttachedSurfaces( LPVOID,LPDDENUMSURFACESCALLBACK ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// EnumOverlayZOrders( DWORD,LPVOID,LPDDENUMSURFACESCALLBACK7 ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// Flip( LPDIRECTDRAWSURFACE, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// GetAttachedSurface( LPDDSCAPS, LPDIRECTDRAWSURFACE FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// GetBltStatus( DWORD ) = 0 ;
	virtual HRESULT __stdcall GetCaps					( D_DDSCAPS * ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// GetClipper( LPDIRECTDRAWCLIPPER FAR* ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// GetColorKey( DWORD, LPDDCOLORKEY ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// GetDC( HDC FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// GetFlipStatus( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// GetOverlayPosition( LPLONG, LPLONG	) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// GetPalette( LPDIRECTDRAWPALETTE FAR* ) = 0 ;
	virtual HRESULT __stdcall GetPixelFormat			( D_DDPIXELFORMAT * ) = 0 ;
	virtual HRESULT __stdcall GetSurfaceDesc			( D_DDSURFACEDESC * ) = 0 ;
	virtual HRESULT __stdcall Initialize				( D_IDirectDraw *, D_DDSURFACEDESC * ) = 0 ;
	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// IsLost( void	) = 0 ;
	virtual HRESULT __stdcall Lock						( LPRECT, D_DDSURFACEDESC *, DWORD, HANDLE ) = 0 ;
	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// ReleaseDC( HDC ) = 0 ;
	virtual HRESULT __stdcall Restore					( void ) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// SetClipper( LPDIRECTDRAWCLIPPER ) = 0 ;
	virtual HRESULT __stdcall NonUse19					( void ) = 0 ;				// SetColorKey( DWORD, LPDDCOLORKEY ) = 0 ;
	virtual HRESULT __stdcall NonUse20					( void ) = 0 ;				// SetOverlayPosition( LONG, LONG	) = 0 ;
	virtual HRESULT __stdcall NonUse21					( void ) = 0 ;				// SetPalette( LPDIRECTDRAWPALETTE ) = 0 ;
	virtual HRESULT __stdcall Unlock					( LPRECT ) = 0 ;
	virtual HRESULT __stdcall NonUse22					( void ) = 0 ;				// UpdateOverlay( LPRECT, LPDIRECTDRAWSURFACE,LPRECT,DWORD, LPDDOVERLAYFX ) = 0 ;
	virtual HRESULT __stdcall NonUse23					( void ) = 0 ;				// UpdateOverlayDisplay( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse24					( void ) = 0 ;				// UpdateOverlayZOrder( DWORD, LPDIRECTDRAWSURFACE ) = 0 ;
} ;

class D_IDirectDrawSurface2 : public D_IDirectDrawSurface
{
public :
	virtual HRESULT __stdcall NonUse25					( void ) = 0 ;				// GetDDInterface( LPVOID FAR * ) = 0 ;
	virtual HRESULT __stdcall PageLock					( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse27					( void ) = 0 ;				// PageUnlock( DWORD ) = 0 ;
} ;

class D_IDirectDrawSurface3 : public D_IDirectDrawSurface2
{
public :
	virtual HRESULT __stdcall NonUse28					( void ) = 0 ;				// SetSurfaceDesc( LPDDSURFACEDESC2, DWORD ) = 0 ;
} ;

class D_IDirectDrawSurface4 : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// AddAttachedSurface( LPDIRECTDRAWSURFACE4 ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// AddOverlayDirtyRect( LPRECT ) = 0 ;
	virtual HRESULT __stdcall Blt						( LPRECT, D_IDirectDrawSurface4 *, LPRECT, DWORD, D_DDBLTFX * ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// BltBatch( LPDDBLTBATCH, DWORD, DWORD	) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// BltFast( DWORD,DWORD,LPDIRECTDRAWSURFACE4, LPRECT,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// DeleteAttachedSurface( DWORD,LPDIRECTDRAWSURFACE4 ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// EnumAttachedSurfaces( LPVOID,LPDDENUMSURFACESCALLBACK2 ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// EnumOverlayZOrders( DWORD,LPVOID,LPDDENUMSURFACESCALLBACK2 ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// Flip( LPDIRECTDRAWSURFACE4, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// GetAttachedSurface( LPDDSCAPS2, LPDIRECTDRAWSURFACE4 FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// GetBltStatus( DWORD ) = 0 ;
	virtual HRESULT __stdcall GetCaps					( D_DDSCAPS2 * ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// GetClipper( LPDIRECTDRAWCLIPPER FAR* ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// GetColorKey( DWORD, LPDDCOLORKEY ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// GetDC( HDC FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// GetFlipStatus( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// GetOverlayPosition( LPLONG, LPLONG	) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// GetPalette( LPDIRECTDRAWPALETTE FAR* ) = 0 ;
	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// GetPixelFormat( LPDDPIXELFORMAT ) = 0 ;
	virtual HRESULT __stdcall GetSurfaceDesc			( D_DDSURFACEDESC2 * ) = 0 ;
	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// Initialize( LPDIRECTDRAW, LPDDSURFACEDESC2 ) = 0 ;
	virtual HRESULT __stdcall IsLost					( void ) = 0 ;
	virtual HRESULT __stdcall Lock						( LPRECT, D_DDSURFACEDESC2 *, DWORD, HANDLE ) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// ReleaseDC( HDC ) = 0 ;
	virtual HRESULT __stdcall Restore					( void ) = 0 ;
	virtual HRESULT __stdcall NonUse19					( void ) = 0 ;				// SetClipper( LPDIRECTDRAWCLIPPER ) = 0 ;
	virtual HRESULT __stdcall NonUse20					( void ) = 0 ;				// SetColorKey( DWORD, LPDDCOLORKEY ) = 0 ;
	virtual HRESULT __stdcall NonUse21					( void ) = 0 ;				// SetOverlayPosition( LONG, LONG	) = 0 ;
	virtual HRESULT __stdcall NonUse22					( void ) = 0 ;				// SetPalette( LPDIRECTDRAWPALETTE ) = 0 ;
	virtual HRESULT __stdcall Unlock					( LPRECT ) = 0 ;
	virtual HRESULT __stdcall NonUse23					( void ) = 0 ;				// UpdateOverlay( LPRECT, LPDIRECTDRAWSURFACE4,LPRECT,DWORD, LPDDOVERLAYFX ) = 0 ;
	virtual HRESULT __stdcall NonUse24					( void ) = 0 ;				// UpdateOverlayDisplay( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse25					( void ) = 0 ;				// UpdateOverlayZOrder( DWORD, LPDIRECTDRAWSURFACE4 ) = 0 ;

	virtual HRESULT __stdcall NonUse26					( void ) = 0 ;				// GetDDInterface( LPVOID FAR * ) = 0 ;
	virtual HRESULT __stdcall PageLock					( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse28					( void ) = 0 ;				// PageUnlock( DWORD ) = 0 ;
	
	virtual HRESULT __stdcall NonUse29					( void ) = 0 ;				// SetSurfaceDesc( LPDDSURFACEDESC2, DWORD ) = 0 ;

	virtual HRESULT __stdcall NonUse30					( void ) = 0 ;				// SetPrivateData( REFGUID, LPVOID, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse31					( void ) = 0 ;				// GetPrivateData( REFGUID, LPVOID, LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse32					( void ) = 0 ;				// FreePrivateData( REFGUID ) = 0 ;
	virtual HRESULT __stdcall NonUse33					( void ) = 0 ;				// GetUniquenessValue( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse34					( void ) = 0 ;				// ChangeUniquenessValue( void ) = 0 ;
} ;

class D_IDirectDrawSurface7 : public IUnknown
{
public :
	virtual HRESULT __stdcall AddAttachedSurface		( D_IDirectDrawSurface7 * ) = 0 ;
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// AddOverlayDirtyRect( LPRECT ) = 0 ;
	virtual HRESULT __stdcall Blt						( LPRECT, D_IDirectDrawSurface7 *, LPRECT, DWORD, D_DDBLTFX * ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// BltBatch( LPDDBLTBATCH, DWORD, DWORD	) = 0 ;
	virtual HRESULT __stdcall BltFast					( DWORD, DWORD, D_IDirectDrawSurface7 *, LPRECT, DWORD ) = 0 ;
	virtual HRESULT __stdcall DeleteAttachedSurface		( DWORD, D_IDirectDrawSurface7 * ) = 0 ;
	virtual HRESULT __stdcall EnumAttachedSurfaces		( LPVOID, LPD_DDENUMSURFACESCALLBACK7 ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// EnumOverlayZOrders( void ) = 0 ;		// EnumOverlayZOrders( DWORD,LPVOID,LPDDENUMSURFACESCALLBACK7 ) = 0 ;
	virtual HRESULT __stdcall Flip						( D_IDirectDrawSurface7 *, DWORD ) = 0 ;
	virtual HRESULT __stdcall GetAttachedSurface		( D_DDSCAPS2 *, D_IDirectDrawSurface7 ** ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// GetBltStatus( DWORD ) = 0 ;
	virtual HRESULT __stdcall GetCaps					( D_DDSCAPS2 * ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// GetClipper( LPDIRECTDRAWCLIPPER FAR* ) = 0 ;
	virtual HRESULT __stdcall GetColorKey				( DWORD, D_DDCOLORKEY * ) = 0 ;
	virtual HRESULT __stdcall GetDC						( HDC * ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// GetFlipStatus( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// GetOverlayPosition( LPLONG, LPLONG	) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// GetPalette( LPDIRECTDRAWPALETTE FAR* ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// GetPixelFormat( LPDDPIXELFORMAT ) = 0 ;
	virtual HRESULT __stdcall GetSurfaceDesc			( D_DDSURFACEDESC2 * ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// Initialize( LPDIRECTDRAW, LPDDSURFACEDESC2 ) = 0 ;
	virtual HRESULT __stdcall IsLost					( void ) = 0 ;
	virtual HRESULT __stdcall Lock						( LPRECT, D_DDSURFACEDESC2 *, DWORD, HANDLE ) = 0 ;
	virtual HRESULT __stdcall ReleaseDC					( HDC ) = 0 ;
	virtual HRESULT __stdcall Restore					( void	) = 0 ;
	virtual HRESULT __stdcall SetClipper				( class D_IDirectDrawClipper * ) = 0 ;
	virtual HRESULT __stdcall SetColorKey				( DWORD, D_DDCOLORKEY * ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// SetOverlayPosition( LONG, LONG	) = 0 ;
	virtual HRESULT __stdcall SetPalette				( class D_IDirectDrawPalette * ) = 0 ;
	virtual HRESULT __stdcall Unlock					( LPRECT ) = 0 ;
	virtual HRESULT __stdcall UpdateOverlay				( LPRECT, D_IDirectDrawSurface7 *, LPRECT, DWORD, D_DDOVERLAYFX * ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// UpdateOverlayDisplay( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// UpdateOverlayZOrder( DWORD, D_IDirectDrawSurface7 * ) = 0 ;

	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// GetDDInterface( LPVOID FAR * ) = 0 ;
	virtual HRESULT __stdcall PageLock					( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// PageUnlock( DWORD ) = 0 ;

	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// SetSurfaceDesc( LPDDSURFACEDESC2, DWORD ) = 0 ;

	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// SetPrivateData( REFGUID, LPVOID, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// GetPrivateData( REFGUID, LPVOID, LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse19					( void ) = 0 ;				// FreePrivateData( REFGUID ) = 0 ;
	virtual HRESULT __stdcall NonUse20					( void ) = 0 ;				// GetUniquenessValue( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse21					( void ) = 0 ;				// ChangeUniquenessValue( void ) = 0 ;

	virtual HRESULT __stdcall NonUse22					( void ) = 0 ;				// SetPriority( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse23					( void ) = 0 ;				// GetPriority( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse24					( void ) = 0 ;				// SetLOD( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse25					( void ) = 0 ;				// GetLOD( LPDWORD ) = 0 ;
} ;

class D_IDirectDrawClipper : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetClipList( LPRECT, LPRGNDATA, LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// GetHWnd( HWND FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// Initialize( LPDIRECTDRAW, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// IsClipListChanged( BOOL FAR * ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// SetClipList( LPRGNDATA,DWORD ) = 0 ;
	virtual HRESULT __stdcall SetHWnd					( DWORD, HWND ) = 0 ;
} ;

class D_IDirectDrawPalette : public IUnknown 
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetCaps( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall GetEntries				( DWORD, DWORD, DWORD, LPPALETTEENTRY ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// Initialize( LPDIRECTDRAW, DWORD, LPPALETTEENTRY ) = 0 ;
	virtual HRESULT __stdcall SetEntries				( DWORD, DWORD, DWORD, LPPALETTEENTRY ) = 0 ;
} ;

#endif // __WINDOWS__

// Ｄｉｒｅｃｔ３Ｄ -----------------------------------------------------------

#define D_D3D_OK								(D_DD_OK)

#define D_D3DENUMRET_CANCEL						D_DDENUMRET_CANCEL
#define D_D3DENUMRET_OK							D_DDENUMRET_OK

#define D_D3DPTEXTURECAPS_PERSPECTIVE			(0x00000001L)
#define D_D3DPTEXTURECAPS_POW2					(0x00000002L)
#define D_D3DPTEXTURECAPS_ALPHA					(0x00000004L)
#define D_D3DPTEXTURECAPS_SQUAREONLY			(0x00000020L)
#define D_D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE (0x00000040L)
#define D_D3DPTEXTURECAPS_ALPHAPALETTE			(0x00000080L)
#define D_D3DPTEXTURECAPS_NONPOW2CONDITIONAL	(0x00000100L)
#define D_D3DPTEXTURECAPS_PROJECTED				(0x00000400L)
#define D_D3DPTEXTURECAPS_CUBEMAP				(0x00000800L)
#define D_D3DPTEXTURECAPS_VOLUMEMAP				(0x00002000L)
#define D_D3DPTEXTURECAPS_MIPMAP				(0x00004000L)
#define D_D3DPTEXTURECAPS_MIPVOLUMEMAP			(0x00008000L)
#define D_D3DPTEXTURECAPS_MIPCUBEMAP			(0x00010000L)
#define D_D3DPTEXTURECAPS_CUBEMAP_POW2			(0x00020000L)
#define D_D3DPTEXTURECAPS_VOLUMEMAP_POW2		(0x00040000L)
#define D_D3DPTEXTURECAPS_NOPROJECTEDBUMPENV	(0x00200000L)

#define D_D3DTA_DIFFUSE							(0x00000000)
#define D_D3DTA_CURRENT							(0x00000001)
#define D_D3DTA_TEXTURE							(0x00000002)
#define D_D3DTA_TFACTOR							(0x00000003)
#define D_D3DTA_SPECULAR						(0x00000004)
#define D_D3DTA_TEMP							(0x00000005)
#define D_D3DTA_COMPLEMENT						(0x00000010)
#define D_D3DTA_ALPHAREPLICATE					(0x00000020)

#define D_D3DFVF_XYZ							(0x002)
#define D_D3DFVF_XYZRHW							(0x004)
#define D_D3DFVF_NORMAL							(0x010)
#define D_D3DFVF_DIFFUSE						(0x040)
#define D_D3DFVF_SPECULAR						(0x080)
#define D_D3DFVF_TEX0							(0x000)
#define D_D3DFVF_TEX1							(0x100)
#define D_D3DFVF_TEX2							(0x200)
#define D_D3DFVF_TEX3							(0x300)
#define D_D3DFVF_TEX4							(0x400)
#define D_D3DFVF_TEX5							(0x500)
#define D_D3DFVF_TEX6							(0x600)
#define D_D3DFVF_TEX7							(0x700)
#define D_D3DFVF_TEX8							(0x800)
#define D_D3DFVF_XYZB1							(0x006)
#define D_D3DFVF_XYZB2							(0x008)
#define D_D3DFVF_XYZB3							(0x00a)
#define D_D3DFVF_XYZB4							(0x00c)
#define D_D3DFVF_XYZB5							(0x00e)

#define D_D3DVBCAPS_SYSTEMMEMORY				(0x00000800l)
#define D_D3DVBCAPS_WRITEONLY					(0x00010000l)
#define D_D3DVBCAPS_OPTIMIZED					(0x80000000l)
#define D_D3DVBCAPS_DONOTCLIP					(0x00000001l)

#define D_RGBA_MAKE(r, g, b, a)					((D_D3DCOLOR) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

#define D_D3DPRESENTFLAG_LOCKABLE_BACKBUFFER	(0x00000001)
#define D_D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL	(0x00000002)
#define D_D3DPRESENTFLAG_DEVICECLIP				(0x00000004)
#define D_D3DPRESENTFLAG_VIDEO					(0x00000010)

#define D_D3DPMISCCAPS_MASKZ					(0x00000002L)
#define D_D3DPMISCCAPS_CULLNONE					(0x00000010L)
#define D_D3DPMISCCAPS_CULLCW					(0x00000020L)
#define D_D3DPMISCCAPS_CULLCCW					(0x00000040L)
#define D_D3DPMISCCAPS_COLORWRITEENABLE			(0x00000080L)
#define D_D3DPMISCCAPS_CLIPPLANESCALEDPOINTS	(0x00000100L)
#define D_D3DPMISCCAPS_CLIPTLVERTS				(0x00000200L)
#define D_D3DPMISCCAPS_TSSARGTEMP				(0x00000400L)
#define D_D3DPMISCCAPS_BLENDOP					(0x00000800L)
#define D_D3DPMISCCAPS_NULLREFERENCE			(0x00001000L)
#define D_D3DPMISCCAPS_INDEPENDENTWRITEMASKS	(0x00004000L)
#define D_D3DPMISCCAPS_PERSTAGECONSTANT			(0x00008000L)
#define D_D3DPMISCCAPS_FOGANDSPECULARALPHA		(0x00010000L)
#define D_D3DPMISCCAPS_SEPARATEALPHABLEND		(0x00020000L)
#define D_D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS	(0x00040000L)
#define D_D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING	(0x00080000L)
#define D_D3DPMISCCAPS_FOGVERTEXCLAMPED			(0x00100000L)

#define D_D3DFVF_TEXTUREFORMAT2					(( DWORD )0)
#define D_D3DFVF_TEXTUREFORMAT1					(( DWORD )3)
#define D_D3DFVF_TEXTUREFORMAT3					(( DWORD )1)
#define D_D3DFVF_TEXTUREFORMAT4					(( DWORD )2)

#define D_D3DFVF_TEXCOORDSIZE2( CoordIndex )	( D_D3DFVF_TEXTUREFORMAT2 << ( DWORD )( CoordIndex * 2 + 16 ) )
#define D_D3DFVF_TEXCOORDSIZE1( CoordIndex )	( D_D3DFVF_TEXTUREFORMAT1 << ( DWORD )( CoordIndex * 2 + 16 ) )
#define D_D3DFVF_TEXCOORDSIZE3( CoordIndex )	( D_D3DFVF_TEXTUREFORMAT3 << ( DWORD )( CoordIndex * 2 + 16 ) )
#define D_D3DFVF_TEXCOORDSIZE4( CoordIndex )	( D_D3DFVF_TEXTUREFORMAT4 << ( DWORD )( CoordIndex * 2 + 16 ) )

typedef enum tagD_D3DVERTEXBLENDFLAGS
{
	D3DVBLEND_DISABLE							= 0,
	D3DVBLEND_1WEIGHT							= 1,
	D3DVBLEND_2WEIGHTS							= 2,
	D3DVBLEND_3WEIGHTS							= 3,
} D_D3DVERTEXBLENDFLAGS;

typedef enum tagD_D3DCULL
{
	D_D3DCULL_NONE								= 1,
	D_D3DCULL_CW								= 2,
	D_D3DCULL_CCW								= 3,
	D_D3DCULL_FORCE_DWORD						= 0x7fffffff,
} D_D3DCULL;

typedef enum tagD_D3DZBUFFERTYPE
{
	D_D3DZB_FALSE								= 0,
	D_D3DZB_TRUE								= 1,
	D_D3DZB_USEW								= 2,
	D_D3DZB_FORCE_DWORD							= 0x7fffffff,
} D_D3DZBUFFERTYPE;

typedef enum tagD_D3DBLEND
{
	D_D3DBLEND_ZERO								= 1,
	D_D3DBLEND_ONE								= 2,
	D_D3DBLEND_SRCCOLOR							= 3,
	D_D3DBLEND_INVSRCCOLOR						= 4,
	D_D3DBLEND_SRCALPHA							= 5,
	D_D3DBLEND_INVSRCALPHA						= 6,
	D_D3DBLEND_DESTALPHA						= 7,
	D_D3DBLEND_INVDESTALPHA						= 8,
	D_D3DBLEND_DESTCOLOR						= 9,
	D_D3DBLEND_INVDESTCOLOR						= 10,
	D_D3DBLEND_SRCALPHASAT						= 11,
	D_D3DBLEND_BOTHSRCALPHA						= 12,
	D_D3DBLEND_BOTHINVSRCALPHA					= 13,
	D_D3DBLEND_FORCE_DWORD						= 0x7fffffff,
} D_D3DBLEND;

typedef enum tagD_D3DMATERIALCOLORSOURCE
{
	D_D3DMCS_MATERIAL							= 0,
	D_D3DMCS_COLOR1								= 1,
	D_D3DMCS_COLOR2								= 2,
	D_D3DMCS_FORCE_DWORD						= 0x7fffffff,
} D_D3DMATERIALCOLORSOURCE;

typedef enum tagD_D3DTEXTUREOP
{
	D_D3DTOP_DISABLE							= 1,
	D_D3DTOP_SELECTARG1							= 2,
	D_D3DTOP_SELECTARG2							= 3,

	D_D3DTOP_MODULATE							= 4,
	D_D3DTOP_MODULATE2X							= 5,
	D_D3DTOP_MODULATE4X							= 6,

	D_D3DTOP_ADD								= 7,
	D_D3DTOP_ADDSIGNED							= 8,
	D_D3DTOP_ADDSIGNED2X						= 9,
	D_D3DTOP_SUBTRACT							= 10,
	D_D3DTOP_ADDSMOOTH							= 11,

	D_D3DTOP_BLENDDIFFUSEALPHA					= 12,
	D_D3DTOP_BLENDTEXTUREALPHA					= 13,
	D_D3DTOP_BLENDFACTORALPHA					= 14,

	D_D3DTOP_BLENDTEXTUREALPHAPM				= 15,
	D_D3DTOP_BLENDCURRENTALPHA					= 16,

	D_D3DTOP_PREMODULATE						= 17,
	D_D3DTOP_MODULATEALPHA_ADDCOLOR				= 18,
	D_D3DTOP_MODULATECOLOR_ADDALPHA				= 19,
	D_D3DTOP_MODULATEINVALPHA_ADDCOLOR			= 20,
	D_D3DTOP_MODULATEINVCOLOR_ADDALPHA			= 21,

	D_D3DTOP_BUMPENVMAP							= 22,
	D_D3DTOP_BUMPENVMAPLUMINANCE				= 23,
	D_D3DTOP_DOTPRODUCT3						= 24,

	D_D3DTOP_FORCE_DWORD						= 0x7fffffff,
} D_D3DTEXTUREOP;

typedef enum tagD_D3DSHADEMODE
{
	D_D3DSHADE_FLAT								= 1,
	D_D3DSHADE_GOURAUD							= 2,
	D_D3DSHADE_PHONG							= 3,
	D_D3DSHADE_FORCE_DWORD						= 0x7fffffff,
} D_D3DSHADEMODE;

typedef enum tagD_D3DFOGMODE {
	D_D3DFOG_NONE								= 0,
	D_D3DFOG_EXP								= 1,
	D_D3DFOG_EXP2								= 2,
	D_D3DFOG_LINEAR								= 3,
	D_D3DFOG_FORCE_DWORD						= 0x7fffffff,
} D_D3DFOGMODE;

typedef enum tagD_D3DTEXTUREMAGFILTER
{
	D_D3DTFG_POINT								= 1,
	D_D3DTFG_LINEAR								= 2,
	D_D3DTFG_FLATCUBIC							= 3,
	D_D3DTFG_GAUSSIANCUBIC						= 4,
	D_D3DTFG_ANISOTROPIC						= 5,
	D_D3DTFG_FORCE_DWORD						= 0x7fffffff,
} D_D3DTEXTUREMAGFILTER;

typedef enum tagD_D3DTEXTUREMINFILTER
{
	D_D3DTFN_POINT								= 1,
	D_D3DTFN_LINEAR								= 2,
	D_D3DTFN_ANISOTROPIC						= 3,
	D_D3DTFN_FORCE_DWORD						= 0x7fffffff,
} D_D3DTEXTUREMINFILTER;

typedef enum tagD_D3DTEXTUREMIPFILTER
{
	D_D3DTFP_NONE								= 1,
	D_D3DTFP_POINT								= 2,
	D_D3DTFP_LINEAR								= 3,
	D_D3DTFP_FORCE_DWORD						= 0x7fffffff,
} D_D3DTEXTUREMIPFILTER;

typedef enum tagD_D3DTEXTUREADDRESS
{
	D_D3DTADDRESS_WRAP							= 1,
	D_D3DTADDRESS_MIRROR						= 2,
	D_D3DTADDRESS_CLAMP							= 3,
	D_D3DTADDRESS_BORDER						= 4,
	D_D3DTADDRESS_MIRRORONCE					= 5,
	D_D3DTADDRESS_FORCE_DWORD					= 0x7fffffff,
} D_D3DTEXTUREADDRESS;

typedef enum tagD_D3DPRIMITIVETYPE
{
	D_D3DPT_POINTLIST							= 1,
	D_D3DPT_LINELIST							= 2,
	D_D3DPT_LINESTRIP							= 3,
	D_D3DPT_TRIANGLELIST						= 4,
	D_D3DPT_TRIANGLESTRIP						= 5,
	D_D3DPT_TRIANGLEFAN							= 6,
	D_D3DPT_FORCE_DWORD							= 0x7fffffff,
} D_D3DPRIMITIVETYPE;

typedef enum tagD_D3DCMPFUNC
{
	D_D3DCMP_NEVER								= 1, 
	D_D3DCMP_LESS								= 2, 
	D_D3DCMP_EQUAL								= 3, 
	D_D3DCMP_LESSEQUAL							= 4, 
	D_D3DCMP_GREATER							= 5, 
	D_D3DCMP_NOTEQUAL							= 6, 
	D_D3DCMP_GREATEREQUAL						= 7, 
	D_D3DCMP_ALWAYS								= 8, 
	D_D3DCMP_FORCE_DWORD						= 0x7fffffff, 
} D_D3DCMPFUNC ;

typedef enum tagD_D3DANTIALIASMODE
{
	D_D3DANTIALIAS_NONE							= 0,
	D_D3DANTIALIAS_SORTDEPENDENT				= 1,
	D_D3DANTIALIAS_SORTINDEPENDENT				= 2,
	D_D3DANTIALIAS_FORCE_DWORD					= 0x7fffffff,
} D_D3DANTIALIASMODE;

typedef enum tagD_D3DRENDERSTATETYPE
{
	D_D3DRENDERSTATE_ANTIALIAS					= 2,
	D_D3DRENDERSTATE_TEXTUREPERSPECTIVE			= 4,
	D_D3DRENDERSTATE_ZENABLE					= 7,
	D_D3DRENDERSTATE_FILLMODE					= 8,
	D_D3DRENDERSTATE_SHADEMODE					= 9,
	D_D3DRENDERSTATE_LINEPATTERN				= 10,
	D_D3DRENDERSTATE_ZWRITEENABLE				= 14,
	D_D3DRENDERSTATE_ALPHATESTENABLE			= 15,
	D_D3DRENDERSTATE_LASTPIXEL					= 16,
	D_D3DRENDERSTATE_SRCBLEND					= 19,
	D_D3DRENDERSTATE_DESTBLEND					= 20,
	D_D3DRENDERSTATE_CULLMODE					= 22,
	D_D3DRENDERSTATE_ZFUNC						= 23,
	D_D3DRENDERSTATE_ALPHAREF					= 24,
	D_D3DRENDERSTATE_ALPHAFUNC					= 25,
	D_D3DRENDERSTATE_DITHERENABLE				= 26,
	D_D3DRENDERSTATE_ALPHABLENDENABLE			= 27,
	D_D3DRENDERSTATE_FOGENABLE					= 28,
	D_D3DRENDERSTATE_SPECULARENABLE				= 29,
	D_D3DRENDERSTATE_ZVISIBLE					= 30,
	D_D3DRENDERSTATE_STIPPLEDALPHA				= 33,
	D_D3DRENDERSTATE_FOGCOLOR					= 34,
	D_D3DRENDERSTATE_FOGTABLEMODE				= 35,
	D_D3DRENDERSTATE_FOGSTART					= 36,
	D_D3DRENDERSTATE_FOGEND						= 37,
	D_D3DRENDERSTATE_FOGDENSITY					= 38,
	D_D3DRENDERSTATE_EDGEANTIALIAS				= 40,
	D_D3DRENDERSTATE_COLORKEYENABLE				= 41,
	D_D3DRENDERSTATE_ZBIAS						= 47,
	D_D3DRENDERSTATE_RANGEFOGENABLE				= 48,
	D_D3DRENDERSTATE_STENCILENABLE				= 52,
	D_D3DRENDERSTATE_STENCILFAIL				= 53,
	D_D3DRENDERSTATE_STENCILZFAIL				= 54,
	D_D3DRENDERSTATE_STENCILPASS				= 55,
	D_D3DRENDERSTATE_STENCILFUNC				= 56,
	D_D3DRENDERSTATE_STENCILREF					= 57,
	D_D3DRENDERSTATE_STENCILMASK				= 58,
	D_D3DRENDERSTATE_STENCILWRITEMASK			= 59,
	D_D3DRENDERSTATE_TEXTUREFACTOR				= 60,
	D_D3DRENDERSTATE_WRAP0						= 128,
	D_D3DRENDERSTATE_WRAP1						= 129,
	D_D3DRENDERSTATE_WRAP2						= 130,
	D_D3DRENDERSTATE_WRAP3						= 131,
	D_D3DRENDERSTATE_WRAP4						= 132,
	D_D3DRENDERSTATE_WRAP5						= 133,
	D_D3DRENDERSTATE_WRAP6						= 134,
	D_D3DRENDERSTATE_WRAP7						= 135,
	D_D3DRENDERSTATE_CLIPPING					= 136,
	D_D3DRENDERSTATE_LIGHTING					= 137,
	D_D3DRENDERSTATE_EXTENTS					= 138,
	D_D3DRENDERSTATE_AMBIENT					= 139,
	D_D3DRENDERSTATE_FOGVERTEXMODE				= 140,
	D_D3DRENDERSTATE_COLORVERTEX				= 141,
	D_D3DRENDERSTATE_LOCALVIEWER				= 142,
	D_D3DRENDERSTATE_NORMALIZENORMALS			= 143,
	D_D3DRENDERSTATE_COLORKEYBLENDENABLE		= 144,
	D_D3DRENDERSTATE_DIFFUSEMATERIALSOURCE		= 145,
	D_D3DRENDERSTATE_SPECULARMATERIALSOURCE		= 146,
	D_D3DRENDERSTATE_AMBIENTMATERIALSOURCE		= 147,
	D_D3DRENDERSTATE_EMISSIVEMATERIALSOURCE		= 148,
	D_D3DRENDERSTATE_VERTEXBLEND				= 151,
	D_D3DRENDERSTATE_CLIPPLANEENABLE			= 152,

	D_D3DRENDERSTATE_TEXTUREHANDLE				= 1,
	D_D3DRENDERSTATE_TEXTUREADDRESS				= 3,
	D_D3DRENDERSTATE_WRAPU						= 5,
	D_D3DRENDERSTATE_WRAPV						= 6,
	D_D3DRENDERSTATE_MONOENABLE					= 11,
	D_D3DRENDERSTATE_ROP2						= 12,
	D_D3DRENDERSTATE_PLANEMASK					= 13,
	D_D3DRENDERSTATE_TEXTUREMAG					= 17,
	D_D3DRENDERSTATE_TEXTUREMIN					= 18,
	D_D3DRENDERSTATE_TEXTUREMAPBLEND			= 21,
	D_D3DRENDERSTATE_SUBPIXEL					= 31,
	D_D3DRENDERSTATE_SUBPIXELX					= 32,
	D_D3DRENDERSTATE_STIPPLEENABLE				= 39,
	D_D3DRENDERSTATE_BORDERCOLOR				= 43,
	D_D3DRENDERSTATE_TEXTUREADDRESSU			= 44,
	D_D3DRENDERSTATE_TEXTUREADDRESSV			= 45,
	D_D3DRENDERSTATE_MIPMAPLODBIAS				= 46,
	D_D3DRENDERSTATE_ANISOTROPY					= 49,
	D_D3DRENDERSTATE_FLUSHBATCH					= 50,
	D_D3DRENDERSTATE_TRANSLUCENTSORTINDEPENDENT	= 51,
	D_D3DRENDERSTATE_STIPPLEPATTERN00			= 64,
	D_D3DRENDERSTATE_STIPPLEPATTERN01			= 65,
	D_D3DRENDERSTATE_STIPPLEPATTERN02			= 66,
	D_D3DRENDERSTATE_STIPPLEPATTERN03			= 67,
	D_D3DRENDERSTATE_STIPPLEPATTERN04			= 68,
	D_D3DRENDERSTATE_STIPPLEPATTERN05			= 69,
	D_D3DRENDERSTATE_STIPPLEPATTERN06			= 70,
	D_D3DRENDERSTATE_STIPPLEPATTERN07			= 71,
	D_D3DRENDERSTATE_STIPPLEPATTERN08			= 72,
	D_D3DRENDERSTATE_STIPPLEPATTERN09			= 73,
	D_D3DRENDERSTATE_STIPPLEPATTERN10			= 74,
	D_D3DRENDERSTATE_STIPPLEPATTERN11			= 75,
	D_D3DRENDERSTATE_STIPPLEPATTERN12			= 76,
	D_D3DRENDERSTATE_STIPPLEPATTERN13			= 77,
	D_D3DRENDERSTATE_STIPPLEPATTERN14			= 78,
	D_D3DRENDERSTATE_STIPPLEPATTERN15			= 79,
	D_D3DRENDERSTATE_STIPPLEPATTERN16			= 80,
	D_D3DRENDERSTATE_STIPPLEPATTERN17			= 81,
	D_D3DRENDERSTATE_STIPPLEPATTERN18			= 82,
	D_D3DRENDERSTATE_STIPPLEPATTERN19			= 83,
	D_D3DRENDERSTATE_STIPPLEPATTERN20			= 84,
	D_D3DRENDERSTATE_STIPPLEPATTERN21			= 85,
	D_D3DRENDERSTATE_STIPPLEPATTERN22			= 86,
	D_D3DRENDERSTATE_STIPPLEPATTERN23			= 87,
	D_D3DRENDERSTATE_STIPPLEPATTERN24			= 88,
	D_D3DRENDERSTATE_STIPPLEPATTERN25			= 89,
	D_D3DRENDERSTATE_STIPPLEPATTERN26			= 90,
	D_D3DRENDERSTATE_STIPPLEPATTERN27			= 91,
	D_D3DRENDERSTATE_STIPPLEPATTERN28			= 92,
	D_D3DRENDERSTATE_STIPPLEPATTERN29			= 93,
	D_D3DRENDERSTATE_STIPPLEPATTERN30			= 94,
	D_D3DRENDERSTATE_STIPPLEPATTERN31			= 95,

	D_D3DRENDERSTATE_FOGTABLESTART				= 36,
	D_D3DRENDERSTATE_FOGTABLEEND				= 37,
	D_D3DRENDERSTATE_FOGTABLEDENSITY			= 38,

	D_D3DRENDERSTATE_FORCE_DWORD				= 0x7fffffff,

	D_D3DRS_ZENABLE								= 7,
	D_D3DRS_FILLMODE							= 8,
	D_D3DRS_SHADEMODE							= 9,
	D_D3DRS_ZWRITEENABLE						= 14,
	D_D3DRS_ALPHATESTENABLE						= 15,
	D_D3DRS_LASTPIXEL							= 16,
	D_D3DRS_SRCBLEND							= 19,
	D_D3DRS_DESTBLEND							= 20,
	D_D3DRS_CULLMODE							= 22,
	D_D3DRS_ZFUNC								= 23,
	D_D3DRS_ALPHAREF							= 24,
	D_D3DRS_ALPHAFUNC							= 25,
	D_D3DRS_DITHERENABLE						= 26,
	D_D3DRS_ALPHABLENDENABLE					= 27,
	D_D3DRS_FOGENABLE							= 28,
	D_D3DRS_SPECULARENABLE						= 29,
	D_D3DRS_FOGCOLOR							= 34,
	D_D3DRS_FOGTABLEMODE						= 35,
	D_D3DRS_FOGSTART							= 36,
	D_D3DRS_FOGEND								= 37,
	D_D3DRS_FOGDENSITY							= 38,
	D_D3DRS_RANGEFOGENABLE						= 48,
	D_D3DRS_STENCILENABLE						= 52,
	D_D3DRS_STENCILFAIL							= 53,
	D_D3DRS_STENCILZFAIL						= 54,
	D_D3DRS_STENCILPASS							= 55,
	D_D3DRS_STENCILFUNC							= 56,
	D_D3DRS_STENCILREF							= 57,
	D_D3DRS_STENCILMASK							= 58,
	D_D3DRS_STENCILWRITEMASK					= 59,
	D_D3DRS_TEXTUREFACTOR						= 60,
	D_D3DRS_WRAP0								= 128,
	D_D3DRS_WRAP1								= 129,
	D_D3DRS_WRAP2								= 130,
	D_D3DRS_WRAP3								= 131,
	D_D3DRS_WRAP4								= 132,
	D_D3DRS_WRAP5								= 133,
	D_D3DRS_WRAP6								= 134,
	D_D3DRS_WRAP7								= 135,
	D_D3DRS_CLIPPING							= 136,
	D_D3DRS_LIGHTING							= 137,
	D_D3DRS_AMBIENT								= 139,
	D_D3DRS_FOGVERTEXMODE						= 140,
	D_D3DRS_COLORVERTEX							= 141,
	D_D3DRS_LOCALVIEWER							= 142,
	D_D3DRS_NORMALIZENORMALS					= 143,
	D_D3DRS_DIFFUSEMATERIALSOURCE				= 145,
	D_D3DRS_SPECULARMATERIALSOURCE				= 146,
	D_D3DRS_AMBIENTMATERIALSOURCE				= 147,
	D_D3DRS_EMISSIVEMATERIALSOURCE				= 148,
	D_D3DRS_VERTEXBLEND							= 151,
	D_D3DRS_CLIPPLANEENABLE						= 152,
	D_D3DRS_POINTSIZE							= 154,
	D_D3DRS_POINTSIZE_MIN						= 155,
	D_D3DRS_POINTSPRITEENABLE					= 156,
	D_D3DRS_POINTSCALEENABLE					= 157,
	D_D3DRS_POINTSCALE_A						= 158,
	D_D3DRS_POINTSCALE_B						= 159,
	D_D3DRS_POINTSCALE_C						= 160,
	D_D3DRS_MULTISAMPLEANTIALIAS				= 161,
	D_D3DRS_MULTISAMPLEMASK						= 162,
	D_D3DRS_PATCHEDGESTYLE						= 163,
	D_D3DRS_DEBUGMONITORTOKEN					= 165,
	D_D3DRS_POINTSIZE_MAX						= 166,
	D_D3DRS_INDEXEDVERTEXBLENDENABLE			= 167,
	D_D3DRS_COLORWRITEENABLE					= 168,
	D_D3DRS_TWEENFACTOR							= 170,
	D_D3DRS_BLENDOP								= 171,
	D_D3DRS_POSITIONDEGREE						= 172,
	D_D3DRS_NORMALDEGREE						= 173,
	D_D3DRS_SCISSORTESTENABLE					= 174,
	D_D3DRS_SLOPESCALEDEPTHBIAS					= 175,
	D_D3DRS_ANTIALIASEDLINEENABLE				= 176,
	D_D3DRS_MINTESSELLATIONLEVEL				= 178,
	D_D3DRS_MAXTESSELLATIONLEVEL				= 179,
	D_D3DRS_ADAPTIVETESS_X						= 180,
	D_D3DRS_ADAPTIVETESS_Y						= 181,
	D_D3DRS_ADAPTIVETESS_Z						= 182,
	D_D3DRS_ADAPTIVETESS_W						= 183,
	D_D3DRS_ENABLEADAPTIVETESSELLATION			= 184,
	D_D3DRS_TWOSIDEDSTENCILMODE					= 185,
	D_D3DRS_CCW_STENCILFAIL						= 186,
	D_D3DRS_CCW_STENCILZFAIL					= 187,
	D_D3DRS_CCW_STENCILPASS						= 188,
	D_D3DRS_CCW_STENCILFUNC						= 189,
	D_D3DRS_COLORWRITEENABLE1					= 190,
	D_D3DRS_COLORWRITEENABLE2					= 191,
	D_D3DRS_COLORWRITEENABLE3					= 192,
	D_D3DRS_BLENDFACTOR							= 193,
	D_D3DRS_SRGBWRITEENABLE						= 194,
	D_D3DRS_DEPTHBIAS							= 195,
	D_D3DRS_WRAP8								= 198,
	D_D3DRS_WRAP9								= 199,
	D_D3DRS_WRAP10								= 200,
	D_D3DRS_WRAP11								= 201,
	D_D3DRS_WRAP12								= 202,
	D_D3DRS_WRAP13								= 203,
	D_D3DRS_WRAP14								= 204,
	D_D3DRS_WRAP15								= 205,
	D_D3DRS_SEPARATEALPHABLENDENABLE			= 206,
	D_D3DRS_SRCBLENDALPHA						= 207,
	D_D3DRS_DESTBLENDALPHA						= 208,
	D_D3DRS_BLENDOPALPHA						= 209,

	D_D3DRS_FORCE_DWORD							= 0x7fffffff,
} D_D3DRENDERSTATETYPE;

typedef enum tagD_D3DSAMPLERSTATETYPE
{
	D_D3DSAMP_ADDRESSU							= 1,
	D_D3DSAMP_ADDRESSV							= 2,
	D_D3DSAMP_ADDRESSW							= 3,
	D_D3DSAMP_BORDERCOLOR						= 4,
	D_D3DSAMP_MAGFILTER							= 5,
	D_D3DSAMP_MINFILTER							= 6,
	D_D3DSAMP_MIPFILTER							= 7,
	D_D3DSAMP_MIPMAPLODBIAS						= 8,
	D_D3DSAMP_MAXMIPLEVEL						= 9,
	D_D3DSAMP_MAXANISOTROPY						= 10,
	D_D3DSAMP_SRGBTEXTURE						= 11,
	D_D3DSAMP_ELEMENTINDEX						= 12,
	D_D3DSAMP_DMAPOFFSET						= 13,
	D_D3DSAMP_FORCE_DWORD						= 0x7fffffff,
} D_D3DSAMPLERSTATETYPE;

typedef enum tagD_D3DSTATEBLOCKTYPE
{
	D_D3DSBT_ALL								= 1,
	D_D3DSBT_PIXELSTATE							= 2,
	D_D3DSBT_VERTEXSTATE						= 3,
	D_D3DSBT_FORCE_DWORD						= 0x7fffffff,
} D_D3DSTATEBLOCKTYPE;

typedef enum tagD_D3DTEXTURESTAGESTATETYPE
{
	D_D3DTSS_COLOROP							= 1,
	D_D3DTSS_COLORARG1							= 2,
	D_D3DTSS_COLORARG2							= 3,
	D_D3DTSS_ALPHAOP							= 4,
	D_D3DTSS_ALPHAARG1							= 5,
	D_D3DTSS_ALPHAARG2							= 6,
	D_D3DTSS_BUMPENVMAT00						= 7,
	D_D3DTSS_BUMPENVMAT01						= 8,
	D_D3DTSS_BUMPENVMAT10						= 9,
	D_D3DTSS_BUMPENVMAT11						= 10,
	D_D3DTSS_TEXCOORDINDEX						= 11,
	D_D3DTSS_ADDRESS							= 12,
	D_D3DTSS_ADDRESSU							= 13,
	D_D3DTSS_ADDRESSV							= 14,
	D_D3DTSS_BORDERCOLOR						= 15,
	D_D3DTSS_MAGFILTER							= 16,
	D_D3DTSS_MINFILTER							= 17,
	D_D3DTSS_MIPFILTER							= 18,
	D_D3DTSS_MIPMAPLODBIAS						= 19,
	D_D3DTSS_MAXMIPLEVEL						= 20,
	D_D3DTSS_MAXANISOTROPY						= 21,
	D_D3DTSS_BUMPENVLSCALE						= 22,
	D_D3DTSS_BUMPENVLOFFSET						= 23,
	D_D3DTSS_TEXTURETRANSFORMFLAGS				= 24,
	D_D3DTSS_COLORARG0							= 26,
	D_D3DTSS_ALPHAARG0							= 27,
	D_D3DTSS_RESULTARG							= 28,
	D_D3DTSS_CONSTANT							= 32,
	D_D3DTSS_FORCE_DWORD						= 0x7fffffff,
} D_D3DTEXTURESTAGESTATETYPE ;

typedef enum tagD_D3DTEXTURETRANSFORMFLAGS
{
	D_D3DTTFF_DISABLE							= 0,
	D_D3DTTFF_COUNT1							= 1,
	D_D3DTTFF_COUNT2							= 2,
	D_D3DTTFF_COUNT3							= 3,
	D_D3DTTFF_COUNT4							= 4,
	D_D3DTTFF_PROJECTED							= 256,
	D_D3DTTFF_FORCE_DWORD						= 0x7fffffff,
} D_D3DTEXTURETRANSFORMFLAGS;

typedef enum tagD_D3DTRANSFORMSTATETYPE
{
	D_D3DTRANSFORMSTATE_WORLD					= 1,
	D_D3DTRANSFORMSTATE_VIEW					= 2,
	D_D3DTRANSFORMSTATE_PROJECTION				= 3,
	D_D3DTRANSFORMSTATE_WORLD1					= 4,
	D_D3DTRANSFORMSTATE_WORLD2					= 5,
	D_D3DTRANSFORMSTATE_WORLD3					= 6,
	D_D3DTRANSFORMSTATE_TEXTURE0				= 16,
	D_D3DTRANSFORMSTATE_TEXTURE1				= 17,
	D_D3DTRANSFORMSTATE_TEXTURE2				= 18,
	D_D3DTRANSFORMSTATE_TEXTURE3				= 19,
	D_D3DTRANSFORMSTATE_TEXTURE4				= 20,
	D_D3DTRANSFORMSTATE_TEXTURE5				= 21,
	D_D3DTRANSFORMSTATE_TEXTURE6				= 22,
	D_D3DTRANSFORMSTATE_TEXTURE7				= 23,
	D_D3DTRANSFORMSTATE_FORCE_DWORD				= 0x7fffffff,

	D_D3DTS_VIEW								= 2,
	D_D3DTS_PROJECTION							= 3,
	D_D3DTS_TEXTURE0							= 16,
	D_D3DTS_TEXTURE1							= 17,
	D_D3DTS_TEXTURE2							= 18,
	D_D3DTS_TEXTURE3							= 19,
	D_D3DTS_TEXTURE4							= 20,
	D_D3DTS_TEXTURE5							= 21,
	D_D3DTS_TEXTURE6							= 22,
	D_D3DTS_TEXTURE7							= 23,
	D_D3DTS_FORCE_DWORD							= 0x7fffffff,
} D_D3DTRANSFORMSTATETYPE ;

typedef enum tagD_D3DLIGHTTYPE
{
	D_D3DLIGHT_POINT							= 1,
	D_D3DLIGHT_SPOT								= 2,
	D_D3DLIGHT_DIRECTIONAL						= 3,
	D_D3DLIGHT_PARALLELPOINT					= 4,
	D_D3DLIGHT_GLSPOT							= 5,
	D_D3DLIGHT_FORCE_DWORD						= 0x7fffffff,
} D_D3DLIGHTTYPE ;

typedef enum D_D3DFORMAT
{
	D_D3DFMT_UNKNOWN							= 0,

	D_D3DFMT_R8G8B8								= 20,
	D_D3DFMT_A8R8G8B8							= 21,
	D_D3DFMT_X8R8G8B8							= 22,
	D_D3DFMT_R5G6B5								= 23,
	D_D3DFMT_X1R5G5B5							= 24,
	D_D3DFMT_A1R5G5B5							= 25,
	D_D3DFMT_A4R4G4B4							= 26,
	D_D3DFMT_R3G3B2								= 27,
	D_D3DFMT_A8									= 28,
	D_D3DFMT_A8R3G3B2							= 29,
	D_D3DFMT_X4R4G4B4							= 30,
	D_D3DFMT_A2B10G10R10						= 31,
	D_D3DFMT_A8B8G8R8							= 32,
	D_D3DFMT_X8B8G8R8							= 33,
	D_D3DFMT_G16R16								= 34,
	D_D3DFMT_A2R10G10B10						= 35,
	D_D3DFMT_A16B16G16R16						= 36,

	D_D3DFMT_A8P8								= 40,
	D_D3DFMT_P8									= 41,

	D_D3DFMT_L8									= 50,
	D_D3DFMT_A8L8								= 51,
	D_D3DFMT_A4L4								= 52,

	D_D3DFMT_V8U8								= 60,
	D_D3DFMT_L6V5U5								= 61,
	D_D3DFMT_X8L8V8U8							= 62,
	D_D3DFMT_Q8W8V8U8							= 63,
	D_D3DFMT_V16U16								= 64,
	D_D3DFMT_A2W10V10U10						= 67,

	D_D3DFMT_UYVY								= ((DWORD)(BYTE)('U') | ((DWORD)(BYTE)('Y') << 8) | ((DWORD)(BYTE)('V') << 16) | ((DWORD)(BYTE)('Y') << 24 )),
	D_D3DFMT_R8G8_B8G8							= ((DWORD)(BYTE)('R') | ((DWORD)(BYTE)('G') << 8) | ((DWORD)(BYTE)('B') << 16) | ((DWORD)(BYTE)('G') << 24 )),
	D_D3DFMT_YUY2								= ((DWORD)(BYTE)('Y') | ((DWORD)(BYTE)('U') << 8) | ((DWORD)(BYTE)('Y') << 16) | ((DWORD)(BYTE)('2') << 24 )),
	D_D3DFMT_G8R8_G8B8							= ((DWORD)(BYTE)('G') | ((DWORD)(BYTE)('R') << 8) | ((DWORD)(BYTE)('G') << 16) | ((DWORD)(BYTE)('B') << 24 )),
	D_D3DFMT_DXT1								= ((DWORD)(BYTE)('D') | ((DWORD)(BYTE)('X') << 8) | ((DWORD)(BYTE)('T') << 16) | ((DWORD)(BYTE)('1') << 24 )),
	D_D3DFMT_DXT2								= ((DWORD)(BYTE)('D') | ((DWORD)(BYTE)('X') << 8) | ((DWORD)(BYTE)('T') << 16) | ((DWORD)(BYTE)('2') << 24 )),
	D_D3DFMT_DXT3								= ((DWORD)(BYTE)('D') | ((DWORD)(BYTE)('X') << 8) | ((DWORD)(BYTE)('T') << 16) | ((DWORD)(BYTE)('3') << 24 )),
	D_D3DFMT_DXT4								= ((DWORD)(BYTE)('D') | ((DWORD)(BYTE)('X') << 8) | ((DWORD)(BYTE)('T') << 16) | ((DWORD)(BYTE)('4') << 24 )),
	D_D3DFMT_DXT5								= ((DWORD)(BYTE)('D') | ((DWORD)(BYTE)('X') << 8) | ((DWORD)(BYTE)('T') << 16) | ((DWORD)(BYTE)('5') << 24 )),

	D_D3DFMT_D16_LOCKABLE						= 70,
	D_D3DFMT_D32								= 71,
	D_D3DFMT_D15S1								= 73,
	D_D3DFMT_D24S8								= 75,
	D_D3DFMT_D24X8								= 77,
	D_D3DFMT_D24X4S4							= 79,
	D_D3DFMT_D16								= 80,

	D_D3DFMT_D32F_LOCKABLE						= 82,
	D_D3DFMT_D24FS8								= 83,


	D_D3DFMT_L16								= 81,

	D_D3DFMT_VERTEXDATA							= 100,
	D_D3DFMT_INDEX16							= 101,
	D_D3DFMT_INDEX32							= 102,

	D_D3DFMT_Q16W16V16U16						= 110,

	D_D3DFMT_MULTI2_ARGB8						= ((DWORD)(BYTE)('M') | ((DWORD)(BYTE)('E') << 8) | ((DWORD)(BYTE)('T') << 16) | ((DWORD)(BYTE)('1') << 24 )),

	D_D3DFMT_R16F								= 111,
	D_D3DFMT_G16R16F							= 112,
	D_D3DFMT_A16B16G16R16F						= 113,

	D_D3DFMT_R32F								= 114,
	D_D3DFMT_G32R32F							= 115,
	D_D3DFMT_A32B32G32R32F						= 116,

	D_D3DFMT_CxV8U8								= 117,

	D_D3DFMT_FORCE_DWORD						= 0x7fffffff
} D_D3DFORMAT ;

typedef enum tagD_D3DQUERYTYPE
{
	D_D3DQUERYTYPE_VCACHE						= 4,
	D_D3DQUERYTYPE_RESOURCEMANAGER				= 5,
	D_D3DQUERYTYPE_VERTEXSTATS					= 6,
	D_D3DQUERYTYPE_EVENT						= 8,
	D_D3DQUERYTYPE_OCCLUSION					= 9,
} D_D3DQUERYTYPE;

typedef enum tagD_D3DTEXTUREFILTERTYPE
{
	D_D3DTEXF_NONE								= 0,
	D_D3DTEXF_POINT								= 1,
	D_D3DTEXF_LINEAR							= 2,
	D_D3DTEXF_ANISOTROPIC						= 3,
	D_D3DTEXF_PYRAMIDALQUAD						= 6,
	D_D3DTEXF_GAUSSIANQUAD						= 7,
	D_D3DTEXF_FORCE_DWORD						= 0x7fffffff,
} D_D3DTEXTUREFILTERTYPE;

typedef enum tagD_D3DBASISTYPE
{
	D_D3DBASIS_BEZIER							= 0,
	D_D3DBASIS_BSPLINE							= 1,
	D_D3DBASIS_CATMULL_ROM						= 2,
	D_D3DBASIS_FORCE_DWORD						= 0x7fffffff,
} D_D3DBASISTYPE;

typedef enum tagD_D3DDEGREETYPE
{
	D_D3DDEGREE_LINEAR							= 1,
	D_D3DDEGREE_QUADRATIC						= 2,
	D_D3DDEGREE_CUBIC							= 3,
	D_D3DDEGREE_QUINTIC							= 5,
	D_D3DDEGREE_FORCE_DWORD						= 0x7fffffff,
} D_D3DDEGREETYPE;

typedef enum tagD_D3DBLENDOP
{
	D_D3DBLENDOP_ADD							= 1,
	D_D3DBLENDOP_SUBTRACT						= 2,
	D_D3DBLENDOP_REVSUBTRACT					= 3,
	D_D3DBLENDOP_MIN							= 4,
	D_D3DBLENDOP_MAX							= 5,
	D_D3DBLENDOP_FORCE_DWORD					= 0x7fffffff,
} D_D3DBLENDOP;

typedef enum tagD_D3DCUBEMAP_FACES
{
	D_D3DCUBEMAP_FACE_POSITIVE_X				= 0,
	D_D3DCUBEMAP_FACE_NEGATIVE_X				= 1,
	D_D3DCUBEMAP_FACE_POSITIVE_Y				= 2,
	D_D3DCUBEMAP_FACE_NEGATIVE_Y				= 3,
	D_D3DCUBEMAP_FACE_POSITIVE_Z				= 4,
	D_D3DCUBEMAP_FACE_NEGATIVE_Z				= 5,

	D_D3DCUBEMAP_FACE_FORCE_DWORD				= 0x7fffffff
} D_D3DCUBEMAP_FACES;

typedef enum tagD_D3DDEVTYPE
{
	D_D3DDEVTYPE_HAL							= 1,
	D_D3DDEVTYPE_REF							= 2,
	D_D3DDEVTYPE_SW								= 3,

	D_D3DDEVTYPE_FORCE_DWORD					= 0x7fffffff
} D_D3DDEVTYPE;

typedef enum tagD_D3DPOOL
{
	D_D3DPOOL_DEFAULT							= 0,
	D_D3DPOOL_MANAGED							= 1,
	D_D3DPOOL_SYSTEMMEM							= 2,
	D_D3DPOOL_SCRATCH							= 3,

	D_D3DPOOL_FORCE_DWORD						= 0x7fffffff
} D_D3DPOOL;

typedef enum tagD_D39DBACKBUFFER_TYPE
{
	D_D3DBACKBUFFER_TYPE_MONO					= 0,
	D_D3DBACKBUFFER_TYPE_LEFT					= 1,
	D_D3DBACKBUFFER_TYPE_RIGHT					= 2,

	D_D3DBACKBUFFER_TYPE_FORCE_DWORD			= 0x7fffffff
} D_D3DBACKBUFFER_TYPE;

typedef enum tagD_D3DSWAPEFFECT
{
	D_D3DSWAPEFFECT_DISCARD						= 1,
	D_D3DSWAPEFFECT_FLIP						= 2,
	D_D3DSWAPEFFECT_COPY						= 3,

	D_D3DSWAPEFFECT_FORCE_DWORD					= 0x7fffffff
} D_D3DSWAPEFFECT;

typedef enum tagD_D3DMULTISAMPLE_TYPE
{
	D_D3DMULTISAMPLE_NONE						=  0,
	D_D3DMULTISAMPLE_NONMASKABLE				=  1,
	D_D3DMULTISAMPLE_2_SAMPLES					=  2,
	D_D3DMULTISAMPLE_3_SAMPLES					=  3,
	D_D3DMULTISAMPLE_4_SAMPLES					=  4,
	D_D3DMULTISAMPLE_5_SAMPLES					=  5,
	D_D3DMULTISAMPLE_6_SAMPLES					=  6,
	D_D3DMULTISAMPLE_7_SAMPLES					=  7,
	D_D3DMULTISAMPLE_8_SAMPLES					=  8,
	D_D3DMULTISAMPLE_9_SAMPLES					=  9,
	D_D3DMULTISAMPLE_10_SAMPLES					= 10,
	D_D3DMULTISAMPLE_11_SAMPLES					= 11,
	D_D3DMULTISAMPLE_12_SAMPLES					= 12,
	D_D3DMULTISAMPLE_13_SAMPLES					= 13,
	D_D3DMULTISAMPLE_14_SAMPLES					= 14,
	D_D3DMULTISAMPLE_15_SAMPLES					= 15,
	D_D3DMULTISAMPLE_16_SAMPLES					= 16,

	D_D3DMULTISAMPLE_FORCE_DWORD				= 0x7fffffff
} D_D3DMULTISAMPLE_TYPE;

typedef enum D_D3DRESOURCETYPE
{
	D_D3DRTYPE_SURFACE							= 1,
	D_D3DRTYPE_VOLUME							= 2,
	D_D3DRTYPE_TEXTURE							= 3,
	D_D3DRTYPE_VOLUMETEXTURE					= 4,
	D_D3DRTYPE_CUBETEXTURE						= 5,
	D_D3DRTYPE_VERTEXBUFFER						= 6,
	D_D3DRTYPE_INDEXBUFFER						= 7,

	D_D3DRTYPE_FORCE_DWORD						= 0x7fffffff
} D_D3DRESOURCETYPE;

typedef enum D_D3DDECLUSAGE
{
	D_D3DDECLUSAGE_POSITION						= 0,
	D_D3DDECLUSAGE_BLENDWEIGHT					= 1,
	D_D3DDECLUSAGE_BLENDINDICES					= 2,
	D_D3DDECLUSAGE_NORMAL						= 3,
	D_D3DDECLUSAGE_PSIZE						= 4,
	D_D3DDECLUSAGE_TEXCOORD						= 5,
	D_D3DDECLUSAGE_TANGENT						= 6,
	D_D3DDECLUSAGE_BINORMAL						= 7,
	D_D3DDECLUSAGE_TESSFACTOR					= 8,
	D_D3DDECLUSAGE_POSITIONT					= 9,
	D_D3DDECLUSAGE_COLOR						= 10,
	D_D3DDECLUSAGE_FOG							= 11,
	D_D3DDECLUSAGE_DEPTH						= 12,
	D_D3DDECLUSAGE_SAMPLE						= 13,
} D_D3DDECLUSAGE;

#define D_MAXD3DDECLUSAGE						D_D3DDECLUSAGE_SAMPLE
#define D_MAXD3DDECLUSAGEINDEX					15
#define D_MAXD3DDECLLENGTH						64

typedef enum D_D3DDECLMETHOD
{
	D_D3DDECLMETHOD_DEFAULT						= 0,
	D_D3DDECLMETHOD_PARTIALU					= 1,
	D_D3DDECLMETHOD_PARTIALV					= 2,
	D_D3DDECLMETHOD_CROSSUV						= 3,
	D_D3DDECLMETHOD_UV							= 4,
	D_D3DDECLMETHOD_LOOKUP						= 5,
	D_D3DDECLMETHOD_LOOKUPPRESAMPLED			= 6,
} D_D3DDECLMETHOD;

#define D_MAXD3DDECLMETHOD						D_D3DDECLMETHOD_LOOKUPPRESAMPLED

typedef enum tagD_D3DDECLTYPE
{
	D_D3DDECLTYPE_FLOAT1						=  0,
	D_D3DDECLTYPE_FLOAT2						=  1,
	D_D3DDECLTYPE_FLOAT3						=  2,
	D_D3DDECLTYPE_FLOAT4						=  3,
	D_D3DDECLTYPE_D3DCOLOR						=  4,
	D_D3DDECLTYPE_UBYTE4						=  5,
	D_D3DDECLTYPE_SHORT2						=  6,
	D_D3DDECLTYPE_SHORT4						=  7,
	D_D3DDECLTYPE_UBYTE4N						=  8,
	D_D3DDECLTYPE_SHORT2N						=  9,
	D_D3DDECLTYPE_SHORT4N						= 10,
	D_D3DDECLTYPE_USHORT2N						= 11,
	D_D3DDECLTYPE_USHORT4N						= 12,
	D_D3DDECLTYPE_UDEC3							= 13,
	D_D3DDECLTYPE_DEC3N							= 14,
	D_D3DDECLTYPE_FLOAT16_2						= 15,
	D_D3DDECLTYPE_FLOAT16_4						= 16,
	D_D3DDECLTYPE_UNUSED						= 17,
} D_D3DDECLTYPE;

typedef enum tagD_D3DXREGISTER_SET
{
	D_D3DXRS_BOOL								= 0,
	D_D3DXRS_INT4								= 1,
	D_D3DXRS_FLOAT4								= 2,
	D_D3DXRS_SAMPLER							= 3,

	D_D3DXRS_FORCE_DWORD						= 0x7fffffff
} D_D3DXREGISTER_SET ;

typedef enum tagD_D3DXPARAMETER_CLASS
{
	D_D3DXPC_SCALAR								= 0,
	D_D3DXPC_VECTOR								= 1,
	D_D3DXPC_MATRIX_ROWS						= 2,
	D_D3DXPC_MATRIX_COLUMNS						= 3,
	D_D3DXPC_OBJECT								= 4,
	D_D3DXPC_STRUCT								= 5,

	D_D3DXPC_FORCE_DWORD						= 0x7fffffff
} D_D3DXPARAMETER_CLASS ;

typedef enum tagD_D3DXPARAMETER_TYPE
{
	D_D3DXPT_VOID								= 0,
	D_D3DXPT_BOOL								= 1,
	D_D3DXPT_INT								= 2,
	D_D3DXPT_FLOAT								= 3,
	D_D3DXPT_STRING								= 4,
	D_D3DXPT_TEXTURE							= 5,
	D_D3DXPT_TEXTURE1D							= 6,
	D_D3DXPT_TEXTURE2D							= 7,
	D_D3DXPT_TEXTURE3D							= 8,
	D_D3DXPT_TEXTURECUBE						= 9,
	D_D3DXPT_SAMPLER							= 10,
	D_D3DXPT_SAMPLER1D							= 11,
	D_D3DXPT_SAMPLER2D							= 12,
	D_D3DXPT_SAMPLER3D							= 13,
	D_D3DXPT_SAMPLERCUBE						= 14,
	D_D3DXPT_PIXELSHADER						= 15,
	D_D3DXPT_VERTEXSHADER						= 16,
	D_D3DXPT_PIXELFRAGMENT						= 17,
	D_D3DXPT_VERTEXFRAGMENT						= 18,
	D_D3DXPT_UNSUPPORTED						= 19,

	D_D3DXPT_FORCE_DWORD						= 0x7fffffff
} D_D3DXPARAMETER_TYPE ;

typedef enum D_D3DSCANLINEORDERING
{
	D_D3DSCANLINEORDERING_UNKNOWN				= 0, 
	D_D3DSCANLINEORDERING_PROGRESSIVE			= 1,
	D_D3DSCANLINEORDERING_INTERLACED			= 2,
} D_D3DSCANLINEORDERING ;

typedef enum D_D3DCOMPOSERECTSOP
{
	D_D3DCOMPOSERECTS_COPY						= 1,
	D_D3DCOMPOSERECTS_OR						= 2,
	D_D3DCOMPOSERECTS_AND						= 3,
	D_D3DCOMPOSERECTS_NEG						= 4,

	D_D3DCOMPOSERECTS_FORCE_DWORD				= 0x7fffffff,
} D_D3DCOMPOSERECTSOP ;

typedef enum D_D3DDISPLAYROTATION
{
	D_D3DDISPLAYROTATION_IDENTITY				= 1,
	D_D3DDISPLAYROTATION_90						= 2,
	D_D3DDISPLAYROTATION_180					= 3,
	D_D3DDISPLAYROTATION_270					= 4 
} D_D3DDISPLAYROTATION ;

typedef enum D_D3DFILLMODE
{
	D_D3DFILL_POINT								= 1,
	D_D3DFILL_WIREFRAME							= 2,
	D_D3DFILL_SOLID								= 3,

	D_D3DFILL_FORCE_DWORD						= 0x7fffffff,
} D_D3DFILLMODE ;

#define D_D3DCREATE_FPU_PRESERVE				(0x00000002L)
#define D_D3DCREATE_MULTITHREADED				(0x00000004L)

#define D_D3DCREATE_PUREDEVICE					(0x00000010L)
#define D_D3DCREATE_SOFTWARE_VERTEXPROCESSING	(0x00000020L)
#define D_D3DCREATE_HARDWARE_VERTEXPROCESSING	(0x00000040L)
#define D_D3DCREATE_MIXED_VERTEXPROCESSING		(0x00000080L)

#define D_D3DCREATE_DISABLE_DRIVER_MANAGEMENT	(0x00000100L)

#define D_D3DADAPTER_DEFAULT					(0)

#define D_D3DPRESENT_INTERVAL_DEFAULT			(0x00000000L)
#define D_D3DPRESENT_INTERVAL_ONE				(0x00000001L)
#define D_D3DPRESENT_INTERVAL_TWO				(0x00000002L)
#define D_D3DPRESENT_INTERVAL_THREE				(0x00000004L)
#define D_D3DPRESENT_INTERVAL_FOUR				(0x00000008L)
#define D_D3DPRESENT_INTERVAL_IMMEDIATE			(0x80000000L)

#define D_D3DPRESENT_DONOTWAIT					(0x00000001L)
#define D_D3DPRESENT_LINEAR_CONTENT				(0x00000002L)

#define D_D3DUSAGE_RENDERTARGET					(0x00000001L)
#define D_D3DUSAGE_DEPTHSTENCIL					(0x00000002L)
#define D_D3DUSAGE_WRITEONLY					(0x00000008L)
#define D_D3DUSAGE_SOFTWAREPROCESSING			(0x00000010L)
#define D_D3DUSAGE_DONOTCLIP					(0x00000020L)
#define D_D3DUSAGE_POINTS						(0x00000040L)
#define D_D3DUSAGE_RTPATCHES					(0x00000080L)
#define D_D3DUSAGE_NPATCHES						(0x00000100L)
#define D_D3DUSAGE_DYNAMIC						(0x00000200L)

#define D_D3DLOCK_READONLY						(0x00000010L)
#define D_D3DLOCK_DISCARD						(0x00002000L)
#define D_D3DLOCK_NOOVERWRITE					(0x00001000L)
#define D_D3DLOCK_NOSYSLOCK						(0x00000800L)
#define D_D3DLOCK_DONOTWAIT						(0x00004000L)

#define D_D3DCLEAR_TARGET						(0x00000001l)
#define D_D3DCLEAR_ZBUFFER						(0x00000002l)
#define D_D3DCLEAR_STENCIL						(0x00000004l)


#define D_D3DDECL_END()							{ 0xFF, 0, D_D3DDECLTYPE_UNUSED, 0, 0, 0 }
#define D_D3DTS_WORLDMATRIX( index )			( D_D3DTRANSFORMSTATETYPE )( index + 256 )

typedef float									D_D3DVALUE ;
typedef DWORD									D_D3DCOLOR ;

typedef struct tagD_D3DVIEWPORT7
{
	DWORD										dwX;
	DWORD										dwY;
	DWORD										dwWidth;
	DWORD										dwHeight;
	D_D3DVALUE									dvMinZ;
	D_D3DVALUE									dvMaxZ;
} D_D3DVIEWPORT7 ;

typedef struct tagD_D3DPRIMCAPS
{
	DWORD										dwSize;
	DWORD										dwMiscCaps;
	DWORD										dwRasterCaps;
	DWORD										dwZCmpCaps;
	DWORD										dwSrcBlendCaps;
	DWORD										dwDestBlendCaps;
	DWORD										dwAlphaCmpCaps;
	DWORD										dwShadeCaps;
	DWORD										dwTextureCaps;
	DWORD										dwTextureFilterCaps;
	DWORD										dwTextureBlendCaps;
	DWORD										dwTextureAddressCaps;
	DWORD										dwStippleWidth;
	DWORD										dwStippleHeight;
} D_D3DPRIMCAPS ;

typedef struct tagD_D3DDEVICEDESC7
{
	DWORD										dwDevCaps;
	D_D3DPRIMCAPS								dpcLineCaps;
	D_D3DPRIMCAPS								dpcTriCaps;
	DWORD										dwDeviceRenderBitDepth;
	DWORD										dwDeviceZBufferBitDepth;

	DWORD										dwMinTextureWidth, dwMinTextureHeight;
	DWORD										dwMaxTextureWidth, dwMaxTextureHeight;

	DWORD										dwMaxTextureRepeat;
	DWORD										dwMaxTextureAspectRatio;
	DWORD										dwMaxAnisotropy;

	D_D3DVALUE									dvGuardBandLeft;
	D_D3DVALUE									dvGuardBandTop;
	D_D3DVALUE									dvGuardBandRight;
	D_D3DVALUE									dvGuardBandBottom;

	D_D3DVALUE									dvExtentsAdjust;
	DWORD										dwStencilCaps;

	DWORD										dwFVFCaps;
	DWORD										dwTextureOpCaps;
	WORD										wMaxTextureBlendStages;
	WORD										wMaxSimultaneousTextures;

	DWORD										dwMaxActiveLights;
	D_D3DVALUE									dvMaxVertexW;
	GUID										deviceGUID;

	WORD										wMaxUserClipPlanes;
	WORD										wMaxVertexBlendMatrices;

	DWORD										dwVertexProcessingCaps;

	DWORD										dwReserved1;
	DWORD										dwReserved2;
	DWORD										dwReserved3;
	DWORD										dwReserved4;
} D_D3DDEVICEDESC7 ;

typedef struct tagD_D3DRECT
{
	union
	{
		LONG 									x1;
		LONG 									lX1;
	} ;
	union
	{
		LONG 									y1;
		LONG 									lY1;
	} ;
	union
	{
		LONG 									x2;
		LONG 									lX2;
	} ;
	union
	{
		LONG 									y2;
		LONG 									lY2;
	} ;
} D_D3DRECT ;

typedef struct tagD_D3DMATRIX
{
	D_D3DVALUE  								m[4][4];
} D_D3DMATRIX ;

typedef struct tagD_D3DCOLORVALUE
{
	union
	{
		D_D3DVALUE  							r;
		D_D3DVALUE  							dvR;
	} ;
	union
	{
		D_D3DVALUE  							g;
		D_D3DVALUE  							dvG;
	} ;
	union
	{
		D_D3DVALUE  							b;
		D_D3DVALUE  							dvB;
	} ;
	union
	{
		D_D3DVALUE  							a;
		D_D3DVALUE  							dvA;
	} ;
} D_D3DCOLORVALUE;

typedef struct tagD_D3DVECTOR
{
	union
	{
		D_D3DVALUE  							x;
		D_D3DVALUE  							dvX;
	} ;
	union
	{
		D_D3DVALUE  							y;
		D_D3DVALUE  							dvY;
	} ;
	union
	{
		D_D3DVALUE  							z;
		D_D3DVALUE  							dvZ;
	} ;
} D_D3DVECTOR ;

typedef struct tagD_D3DMATERIAL7
{
	union
	{
		D_D3DCOLORVALUE							diffuse;
		D_D3DCOLORVALUE							dcvDiffuse;
	} ;
	union
	{
		D_D3DCOLORVALUE							ambient;
		D_D3DCOLORVALUE							dcvAmbient;
	} ;
	union
	{
		D_D3DCOLORVALUE							specular;
		D_D3DCOLORVALUE							dcvSpecular;
	} ;
	union
	{
		D_D3DCOLORVALUE							emissive;
		D_D3DCOLORVALUE							dcvEmissive;
	} ;
	union
	{
		D_D3DVALUE								power;
		D_D3DVALUE								dvPower;
	} ;
} D_D3DMATERIAL7 ;

typedef struct tagD_D3DLIGHT7
{
	D_D3DLIGHTTYPE								dltType;
	D_D3DCOLORVALUE								dcvDiffuse;
	D_D3DCOLORVALUE								dcvSpecular;
	D_D3DCOLORVALUE								dcvAmbient;
	D_D3DVECTOR									dvPosition;
	D_D3DVECTOR									dvDirection;
	D_D3DVALUE									dvRange;
	D_D3DVALUE									dvFalloff;
	D_D3DVALUE									dvAttenuation0;
	D_D3DVALUE									dvAttenuation1;
	D_D3DVALUE									dvAttenuation2;
	D_D3DVALUE									dvTheta;
	D_D3DVALUE									dvPhi;
} D_D3DLIGHT7 ;

typedef struct tagD_D3DVERTEXBUFFERDESC
{
	DWORD										dwSize;
	DWORD										dwCaps;
	DWORD										dwFVF;
	DWORD										dwNumVertices;
} D_D3DVERTEXBUFFERDESC ;

typedef struct D_D3DVSHADERCAPS2_0
{
	DWORD										Caps;
	int											DynamicFlowControlDepth;
	int											NumTemps;
	int											StaticFlowControlDepth;
} D_D3DVSHADERCAPS2_0;

typedef struct tagD_D3DPSHADERCAPS2_0
{
	DWORD										Caps;
	int											DynamicFlowControlDepth;
	int											NumTemps;
	int											StaticFlowControlDepth;
	int											NumInstructionSlots;
} D_D3DPSHADERCAPS2_0;

typedef struct tagD_D3DDISPLAYMODE
{
	UINT										Width;
	UINT										Height;
	UINT										RefreshRate;
	D_D3DFORMAT									Format;
} D_D3DDISPLAYMODE;

typedef struct tagD_D3DPRESENT_PARAMETERS
{
	UINT										BackBufferWidth;
	UINT										BackBufferHeight;
	D_D3DFORMAT									BackBufferFormat;
	UINT										BackBufferCount;

	D_D3DMULTISAMPLE_TYPE						MultiSampleType;
	DWORD										MultiSampleQuality;

	D_D3DSWAPEFFECT								SwapEffect;
	HWND										hDeviceWindow;
	BOOL										Windowed;
	BOOL										EnableAutoDepthStencil;
	D_D3DFORMAT									AutoDepthStencilFormat;
	DWORD										Flags;

	UINT										FullScreen_RefreshRateInHz;
	UINT										PresentationInterval;
} D_D3DPRESENT_PARAMETERS;

typedef struct tagD_D3DDEVICE_CREATION_PARAMETERS
{
	UINT										AdapterOrdinal;
	D_D3DDEVTYPE								DeviceType;
	HWND										hFocusWindow;
	DWORD										BehaviorFlags;
} D_D3DDEVICE_CREATION_PARAMETERS;

typedef struct tagD_D3DRASTER_STATUS
{
	BOOL										InVBlank;
	UINT										ScanLine;
} D_D3DRASTER_STATUS;

typedef struct tagD_D3DGAMMARAMP
{
	WORD										red  [256];
	WORD										green[256];
	WORD										blue [256];
} D_D3DGAMMARAMP;

typedef struct tagD_D3DVIEWPORT9
{
	DWORD										X;
	DWORD										Y;
	DWORD										Width;
	DWORD										Height;
	float										MinZ;
	float										MaxZ;
} D_D3DVIEWPORT9;

typedef struct tagD_D3DCLIPSTATUS9
{
	DWORD										ClipUnion;
	DWORD										ClipIntersection;
} D_D3DCLIPSTATUS9;

typedef struct tagD_D3DMATERIAL9
{
	D_D3DCOLORVALUE								Diffuse;
	D_D3DCOLORVALUE								Ambient;
	D_D3DCOLORVALUE								Specular;
	D_D3DCOLORVALUE								Emissive;
	float										Power;
} D_D3DMATERIAL9;

typedef struct tagD_D3DLIGHT9
{
	D_D3DLIGHTTYPE								Type;
	D_D3DCOLORVALUE								Diffuse;
	D_D3DCOLORVALUE								Specular;
	D_D3DCOLORVALUE								Ambient;
	D_D3DVECTOR									Position;
	D_D3DVECTOR									Direction;
	float										Range;
	float										Falloff;
	float										Attenuation0;
	float										Attenuation1;
	float										Attenuation2;
	float										Theta;
	float										Phi;
} D_D3DLIGHT9;

typedef struct tagD_D3DVERTEXELEMENT9
{
	WORD										Stream;
	WORD										Offset;
	BYTE										Type;
	BYTE										Method;
	BYTE										Usage;
	BYTE										UsageIndex;
} D_D3DVERTEXELEMENT9 ;

typedef struct tagD_D3DRECTPATCH_INFO
{
	UINT										StartVertexOffsetWidth;
	UINT										StartVertexOffsetHeight;
	UINT										Width;
	UINT										Height;
	UINT										Stride;
	D_D3DBASISTYPE								Basis;
	D_D3DDEGREETYPE								Degree;
} D_D3DRECTPATCH_INFO;

typedef struct tagD_D3DTRIPATCH_INFO
{
	UINT										StartVertexOffset;
	UINT										NumVertices;
	D_D3DBASISTYPE								Basis;
	D_D3DDEGREETYPE								Degree;
} D_D3DTRIPATCH_INFO;

typedef struct tagD_D3DSURFACE_DESC
{
	D_D3DFORMAT									Format;
	D_D3DRESOURCETYPE							Type;
	DWORD										Usage;
	D_D3DPOOL									Pool;
	UINT										Size;

	D_D3DMULTISAMPLE_TYPE						MultiSampleType;
	UINT										Width;
	UINT										Height;
} D_D3DSURFACE_DESC;

typedef struct tagD_D3DLOCKED_BOX
{
	INT											RowPitch;
	INT											SlicePitch;
	void*										pBits;
} D_D3DLOCKED_BOX;

typedef struct tagD_D3DVOLUME_DESC
{
	D_D3DFORMAT									Format;
	D_D3DRESOURCETYPE							Type;
	DWORD										Usage;
	D_D3DPOOL									Pool;
	UINT										Size;

	UINT										Width;
	UINT										Height;
	UINT										Depth;
} D_D3DVOLUME_DESC;

typedef struct tagD_D3DBOX
{
	UINT										Left;
	UINT										Top;
	UINT										Right;
	UINT										Bottom;
	UINT										Front;
	UINT										Back;
} D_D3DBOX;

typedef struct tagD_D3DLOCKED_RECT
{
	INT											Pitch;
	void*										pBits;
} D_D3DLOCKED_RECT;

typedef struct tagD_D3DVERTEXBUFFER_DESC
{
	D_D3DFORMAT									Format;
	D_D3DRESOURCETYPE							Type;
	DWORD										Usage;
	D_D3DPOOL									Pool;
	UINT										Size;

	DWORD										FVF;
} D_D3DVERTEXBUFFER_DESC;

typedef struct tagD_D3DINDEXBUFFER_DESC
{
	D_D3DFORMAT									Format;
	D_D3DRESOURCETYPE							Type;
	DWORD										Usage;
	D_D3DPOOL									Pool;
	UINT										Size;
} D_D3DINDEXBUFFER_DESC;

typedef struct tagD_D3DCAPS9
{
	D_D3DDEVTYPE								DeviceType;
	UINT										AdapterOrdinal;

	DWORD										Caps;
	DWORD										Caps2;
	DWORD										Caps3;
	DWORD										PresentationIntervals;

	DWORD										CursorCaps;

	DWORD										DevCaps;

	DWORD										PrimitiveMiscCaps;
	DWORD										RasterCaps;
	DWORD										ZCmpCaps;
	DWORD										SrcBlendCaps;
	DWORD										DestBlendCaps;
	DWORD										AlphaCmpCaps;
	DWORD										ShadeCaps;
	DWORD										TextureCaps;
	DWORD										TextureFilterCaps;
	DWORD										CubeTextureFilterCaps;
	DWORD										VolumeTextureFilterCaps;
	DWORD										TextureAddressCaps;
	DWORD										VolumeTextureAddressCaps;

	DWORD										LineCaps;

	DWORD										MaxTextureWidth, MaxTextureHeight;
	DWORD										MaxVolumeExtent;

	DWORD										MaxTextureRepeat;
	DWORD										MaxTextureAspectRatio;
	DWORD										MaxAnisotropy;
	float										MaxVertexW;

	float										GuardBandLeft;
	float										GuardBandTop;
	float										GuardBandRight;
	float										GuardBandBottom;

	float										ExtentsAdjust;
	DWORD										StencilCaps;

	DWORD										FVFCaps;
	DWORD										TextureOpCaps;
	DWORD										MaxTextureBlendStages;
	DWORD										MaxSimultaneousTextures;

	DWORD										VertexProcessingCaps;
	DWORD										MaxActiveLights;
	DWORD										MaxUserClipPlanes;
	DWORD										MaxVertexBlendMatrices;
	DWORD										MaxVertexBlendMatrixIndex;

	float										MaxPointSize;

	DWORD										MaxPrimitiveCount;
	DWORD										MaxVertexIndex;
	DWORD										MaxStreams;
	DWORD										MaxStreamStride;

	DWORD										VertexShaderVersion;
	DWORD										MaxVertexShaderConst;

	DWORD										PixelShaderVersion;
	float										PixelShader1xMaxValue;

	DWORD										DevCaps2;

	float										MaxNpatchTessellationLevel;
	DWORD										Reserved5;

	UINT										MasterAdapterOrdinal;
	UINT										AdapterOrdinalInGroup;
	UINT										NumberOfAdaptersInGroup;
	DWORD										DeclTypes;
	DWORD										NumSimultaneousRTs;
	DWORD										StretchRectFilterCaps;
	D_D3DVSHADERCAPS2_0							VS20Caps;
	D_D3DPSHADERCAPS2_0							PS20Caps;
	DWORD										VertexTextureFilterCaps;
	DWORD										MaxVShaderInstructionsExecuted;
	DWORD										MaxPShaderInstructionsExecuted;
	DWORD										MaxVertexShader30InstructionSlots; 
	DWORD										MaxPixelShader30InstructionSlots;
} D_D3DCAPS9;

#define MAX_DEVICE_IDENTIFIER_STRING        512
typedef struct tagD_D3DADAPTER_IDENTIFIER9
{
	char										Driver[MAX_DEVICE_IDENTIFIER_STRING];
	char										Description[MAX_DEVICE_IDENTIFIER_STRING];
	char										DeviceName[32];

#ifdef _WIN32
	LARGE_INTEGER								DriverVersion;
#else
	DWORD										DriverVersionLowPart;
	DWORD										DriverVersionHighPart;
#endif

	DWORD										VendorId;
	DWORD										DeviceId;
	DWORD										SubSysId;
	DWORD										Revision;

	GUID										DeviceIdentifier;

	DWORD										WHQLLevel;

} D_D3DADAPTER_IDENTIFIER9;

typedef struct tagD_D3DXMACRO
{
	const CHAR									*Name ;
	const CHAR									*Definition ;
} D_D3DXMACRO ;

typedef struct tagD_D3DXCONSTANTTABLE_DESC
{
	const CHAR									*Creator ;
	DWORD										Version ;
	UINT										Constants ;
} D_D3DXCONSTANTTABLE_DESC ;

typedef struct tagD_D3DXCONSTANT_DESC
{
	const CHAR									*Name ;

	D_D3DXREGISTER_SET							RegisterSet ;
	UINT										RegisterIndex ;
	UINT										RegisterCount ;

	D_D3DXPARAMETER_CLASS						Class ;
	D_D3DXPARAMETER_TYPE						Type ;

	UINT										Rows ;
	UINT										Columns ;
	UINT										Elements ;
	UINT										StructMembers ;

	UINT										Bytes ;
	const void									*DefaultValue ;
} D_D3DXCONSTANT_DESC ;

typedef struct tagD_D3DXSHADER_CONSTANTINFO
{
	DWORD										Name ;
	WORD										RegisterSet ;
	WORD										RegisterIndex ;
	WORD										RegisterCount ;
	WORD										Reserved ;
	DWORD										TypeInfo ;
	DWORD										DefaultValue ;
} D_D3DXSHADER_CONSTANTINFO ;

typedef struct tagD_D3DDISPLAYMODEEX
{
	UINT										Size ;
	UINT										Width ;
	UINT										Height ;
	UINT										RefreshRate ;
	D_D3DFORMAT									Format ;
	D_D3DSCANLINEORDERING						ScanLineOrdering ;
} D_D3DDISPLAYMODEEX ;

typedef struct tagD_D3DDISPLAYMODEFILTER
{
	UINT										Size ;
	D_D3DFORMAT									Format ;
	D_D3DSCANLINEORDERING						ScanLineOrdering ;
} D_D3DDISPLAYMODEFILTER ;

#ifdef __WINDOWS__

typedef HRESULT ( CALLBACK * LPD_D3DENUMDEVICESCALLBACK7 )		( LPSTR lpDeviceDescription, LPSTR lpDeviceName, class D_IDirect3DDevice7 *, LPVOID);
typedef HRESULT ( CALLBACK * LPD_D3DENUMPIXELFORMATSCALLBACK )	( D_DDPIXELFORMAT *lpDDPixFmt, LPVOID lpContext);
typedef HRESULT ( CALLBACK * LPD_D3DENUMPIXELFORMATSCALLBACK )	( D_DDPIXELFORMAT *lpDDPixFmt, LPVOID lpContext);

class D_IDirect3DResource9 : public IUnknown
{
public :
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice) = 0 ;
	virtual HRESULT __stdcall SetPrivateData			( REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags) = 0 ;
	virtual HRESULT __stdcall GetPrivateData			( REFGUID refguid, void* pData, DWORD* pSizeOfData) = 0 ;
	virtual HRESULT __stdcall FreePrivateData			( REFGUID refguid) = 0 ;
	virtual DWORD   __stdcall SetPriority				( DWORD PriorityNew) = 0 ;
	virtual DWORD   __stdcall GetPriority				( void ) = 0 ;
	virtual void    __stdcall PreLoad					( void ) = 0 ;
	virtual D_D3DRESOURCETYPE __stdcall GetType			( void ) = 0 ;
};

class D_IDirect3DSurface9 : public D_IDirect3DResource9
{
public :
	virtual HRESULT __stdcall GetContainer				( REFIID riid, void** ppContainer) = 0 ;
	virtual HRESULT __stdcall GetDesc					( D_D3DSURFACE_DESC *pDesc) = 0 ;
	virtual HRESULT __stdcall LockRect					( D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) = 0 ;
	virtual HRESULT __stdcall UnlockRect				( void ) = 0 ;
	virtual HRESULT __stdcall GetDC						( HDC *phdc) = 0 ;
	virtual HRESULT __stdcall ReleaseDC					( HDC hdc) = 0 ;
};

class D_IDirect3DBaseTexture9 : public D_IDirect3DResource9
{
public :
	virtual DWORD   __stdcall SetLOD					( DWORD LODNew) = 0 ;
	virtual DWORD   __stdcall GetLOD					( void ) = 0 ;
	virtual DWORD   __stdcall GetLevelCount				( void ) = 0 ;
	virtual HRESULT __stdcall SetAutoGenFilterType		( D_D3DTEXTUREFILTERTYPE FilterType) = 0 ;
	virtual D_D3DTEXTUREFILTERTYPE __stdcall GetAutoGenFilterType( void ) = 0 ;
	virtual void    __stdcall GenerateMipSubLevels		( void ) = 0 ;
};

class D_IDirect3DTexture9 : public D_IDirect3DBaseTexture9
{
public :
	virtual HRESULT __stdcall GetLevelDesc				( UINT Level, D_D3DSURFACE_DESC *pDesc) = 0 ;
	virtual HRESULT __stdcall GetSurfaceLevel			( UINT Level,  class D_IDirect3DSurface9** ppSurfaceLevel) = 0 ;
	virtual HRESULT __stdcall LockRect					( UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags) = 0 ;
	virtual HRESULT __stdcall UnlockRect				( UINT Level) = 0 ;
	virtual HRESULT __stdcall AddDirtyRect				( CONST RECT* pDirtyRect) = 0 ;
};

class D_IDirect3DVolume9 : public IUnknown
{
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice)  = 0 ;
	virtual HRESULT __stdcall SetPrivateData			( REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags)  = 0 ;
	virtual HRESULT __stdcall GetPrivateData			( REFGUID refguid, void* pData, DWORD* pSizeOfData)  = 0 ;
	virtual HRESULT __stdcall FreePrivateData			( REFGUID refguid)  = 0 ;
	virtual HRESULT __stdcall GetContainer				( REFIID riid, void** ppContainer)  = 0 ;
	virtual HRESULT __stdcall GetDesc					( D_D3DVOLUME_DESC *pDesc)  = 0 ;
	virtual HRESULT __stdcall LockBox					( D_D3DLOCKED_BOX * pLockedVolume, CONST D_D3DBOX* pBox, DWORD Flags)  = 0 ;
	virtual HRESULT __stdcall UnlockBox					( void )  = 0 ;
};

class D_IDirect3DVolumeTexture9 : public D_IDirect3DBaseTexture9
{
public :
	virtual HRESULT __stdcall GetLevelDesc				( UINT Level, D_D3DVOLUME_DESC *pDesc) = 0 ;
	virtual HRESULT __stdcall GetVolumeLevel			( UINT Level, class D_IDirect3DVolume9** ppVolumeLevel) = 0 ;
	virtual HRESULT __stdcall LockBox					( UINT Level, D_D3DLOCKED_BOX* pLockedVolume, CONST D_D3DBOX* pBox, DWORD Flags) = 0 ;
	virtual HRESULT __stdcall UnlockBox					( UINT Level) = 0 ;
	virtual HRESULT __stdcall AddDirtyBox				( CONST D_D3DBOX* pDirtyBox) = 0 ;
};

class D_IDirect3DCubeTexture9 : public D_IDirect3DBaseTexture9
{
public :
	virtual HRESULT __stdcall GetLevelDesc				( UINT Level, D_D3DSURFACE_DESC *pDesc)  = 0 ;
	virtual HRESULT __stdcall GetCubeMapSurface			( D_D3DCUBEMAP_FACES FaceType, UINT Level, class D_IDirect3DSurface9** ppCubeMapSurface)  = 0 ;
	virtual HRESULT __stdcall LockRect					( D_D3DCUBEMAP_FACES FaceType, UINT Level, D_D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)  = 0 ;
	virtual HRESULT __stdcall UnlockRect				( D_D3DCUBEMAP_FACES FaceType, UINT Level)  = 0 ;
	virtual HRESULT __stdcall AddDirtyRect				( D_D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect)  = 0 ;
};

class D_IDirect3DVertexBuffer9 : public D_IDirect3DResource9
{
public :
	virtual HRESULT __stdcall Lock						( UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags)  = 0 ;
	virtual HRESULT __stdcall Unlock					( void )  = 0 ;
	virtual HRESULT __stdcall GetDesc					( D_D3DVERTEXBUFFER_DESC *pDesc)  = 0 ;
};

class D_IDirect3DIndexBuffer9 : public D_IDirect3DResource9
{
public :
	virtual HRESULT __stdcall Lock						( UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags ) = 0 ;
	virtual HRESULT __stdcall Unlock					( void ) = 0 ;
	virtual HRESULT __stdcall GetDesc					( D_D3DINDEXBUFFER_DESC *pDesc) = 0 ;
};

class D_IDirect3DSwapChain9 : public IUnknown
{
public :
	virtual HRESULT __stdcall Present					( CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags) = 0 ;
	virtual HRESULT __stdcall GetFrontBufferData		( class D_IDirect3DSurface9* pDestSurface) = 0 ;
	virtual HRESULT __stdcall GetBackBuffer				( UINT iBackBuffer, D_D3DBACKBUFFER_TYPE Type, class D_IDirect3DSurface9** ppBackBuffer) = 0 ;
	virtual HRESULT __stdcall GetRasterStatus			( D_D3DRASTER_STATUS* pRasterStatus) = 0 ;
	virtual HRESULT __stdcall GetDisplayMode			( D_D3DDISPLAYMODE* pMode) = 0 ;
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice) = 0 ;
	virtual HRESULT __stdcall GetPresentParameters		( D_D3DPRESENT_PARAMETERS* pPresentationParameters) = 0 ;
};

class D_IDirect3DQuery9 : public IUnknown
{
public :
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice) = 0 ;
	virtual D_D3DQUERYTYPE __stdcall GetType			( void ) = 0 ;
	virtual DWORD   __stdcall GetDataSize				( void ) = 0 ;
	virtual HRESULT __stdcall Issue						( DWORD dwIssueFlags) = 0 ;
	virtual HRESULT __stdcall GetData					( void* pData, DWORD dwSize, DWORD dwGetDataFlags) = 0 ;
};

class D_IDirect3DStateBlock9 : public IUnknown
{
public :
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice) = 0 ;
	virtual HRESULT __stdcall Capture					( void ) = 0 ;
	virtual HRESULT __stdcall Apply						( void ) = 0 ;
};

class D_IDirect3DVertexDeclaration9 : public IUnknown
{
public :
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice) = 0 ;
	virtual HRESULT __stdcall GetDeclaration			( D_D3DVERTEXELEMENT9*, UINT* pNumElements) = 0 ;
};

class D_IDirect3DVertexShader9 : public IUnknown
{
public :
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice ) = 0 ;
	virtual HRESULT __stdcall GetFunction				( void*, UINT* pSizeOfData ) = 0 ;
};

class D_IDirect3DPixelShader9 : public IUnknown
{
public :
	virtual HRESULT __stdcall GetDevice					( class D_IDirect3DDevice9** ppDevice ) = 0 ;
	virtual HRESULT __stdcall GetFunction				( void*, UINT* pSizeOfData ) = 0 ;
};

class D_IDirect3DDevice9 : public IUnknown
{
public :
	virtual HRESULT __stdcall TestCooperativeLevel		( void ) = 0 ;
	virtual UINT    __stdcall GetAvailableTextureMem	( void ) = 0 ;
	virtual HRESULT __stdcall EvictManagedResources		( void ) = 0 ;
	virtual HRESULT __stdcall GetDirect3D				( class D_IDirect3D9** ppD3D9) = 0 ;
	virtual HRESULT __stdcall GetDeviceCaps				( D_D3DCAPS9* pCaps) = 0 ;
	virtual HRESULT __stdcall GetDisplayMode			( UINT iSwapChain, D_D3DDISPLAYMODE* pMode) = 0 ;
	virtual HRESULT __stdcall GetCreationParameters		( D_D3DDEVICE_CREATION_PARAMETERS *pParameters) = 0 ;
	virtual HRESULT __stdcall SetCursorProperties		( UINT XHotSpot, UINT YHotSpot, class D_IDirect3DSurface9* pCursorBitmap) = 0 ;
	virtual void    __stdcall SetCursorPosition			( int X, int Y, DWORD Flags) = 0 ;
	virtual BOOL    __stdcall ShowCursor				( BOOL bShow) = 0 ;
	virtual HRESULT __stdcall CreateAdditionalSwapChain	( D_D3DPRESENT_PARAMETERS* pPresentationParameters, class D_IDirect3DSwapChain9** pSwapChain) = 0 ;
	virtual HRESULT __stdcall GetSwapChain				( UINT iSwapChain, class D_IDirect3DSwapChain9** pSwapChain) = 0 ;
	virtual UINT    __stdcall GetNumberOfSwapChains		( void ) = 0 ;
	virtual HRESULT __stdcall Reset						( D_D3DPRESENT_PARAMETERS* pPresentationParameters) = 0 ;
	virtual HRESULT __stdcall Present					( CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion) = 0 ;
	virtual HRESULT __stdcall GetBackBuffer				( UINT iSwapChain, UINT iBackBuffer, D_D3DBACKBUFFER_TYPE Type, class D_IDirect3DSurface9** ppBackBuffer) = 0 ;
	virtual HRESULT __stdcall GetRasterStatus			( UINT iSwapChain, D_D3DRASTER_STATUS* pRasterStatus) = 0 ;
	virtual HRESULT __stdcall SetDialogBoxMode			( BOOL bEnableDialogs) = 0 ;
	virtual void    __stdcall SetGammaRamp				( UINT iSwapChain, DWORD Flags, CONST D_D3DGAMMARAMP* pRamp) = 0 ;
	virtual void    __stdcall GetGammaRamp				( UINT iSwapChain, D_D3DGAMMARAMP* pRamp) = 0 ;
	virtual HRESULT __stdcall CreateTexture				( UINT Width, UINT Height, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, class D_IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall CreateVolumeTexture		( UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, class D_IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall CreateCubeTexture			( UINT EdgeLength, UINT Levels, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, class D_IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall CreateVertexBuffer		( UINT Length, DWORD Usage, DWORD FVF, D_D3DPOOL Pool, class D_IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall CreateIndexBuffer			( UINT Length, DWORD Usage, D_D3DFORMAT Format, D_D3DPOOL Pool, class D_IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall CreateRenderTarget		( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, class D_IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall CreateDepthStencilSurface	( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, class D_IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall UpdateSurface				( class D_IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, class D_IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint) = 0 ;
	virtual HRESULT __stdcall UpdateTexture				( class D_IDirect3DBaseTexture9* pSourceTexture, class D_IDirect3DBaseTexture9* pDestinationTexture) = 0 ;
	virtual HRESULT __stdcall GetRenderTargetData		( class D_IDirect3DSurface9* pRenderTarget, class D_IDirect3DSurface9* pDestSurface) = 0 ;
	virtual HRESULT __stdcall GetFrontBufferData		( UINT iSwapChain, class D_IDirect3DSurface9* pDestSurface) = 0 ;
	virtual HRESULT __stdcall StretchRect				( class D_IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, class D_IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D_D3DTEXTUREFILTERTYPE Filter) = 0 ;
	virtual HRESULT __stdcall ColorFill					( class D_IDirect3DSurface9* pSurface, CONST RECT* pRect, D_D3DCOLOR color) = 0 ;
	virtual HRESULT __stdcall CreateOffscreenPlainSurface( UINT Width, UINT Height, D_D3DFORMAT Format, D_D3DPOOL Pool, class D_IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) = 0 ;
	virtual HRESULT __stdcall SetRenderTarget			( DWORD RenderTargetIndex, class D_IDirect3DSurface9* pRenderTarget) = 0 ;
	virtual HRESULT __stdcall GetRenderTarget			( DWORD RenderTargetIndex, class D_IDirect3DSurface9** ppRenderTarget) = 0 ;
	virtual HRESULT __stdcall SetDepthStencilSurface	( class D_IDirect3DSurface9* pNewZStencil) = 0 ;
	virtual HRESULT __stdcall GetDepthStencilSurface	( class D_IDirect3DSurface9** ppZStencilSurface) = 0 ;
	virtual HRESULT __stdcall BeginScene				( void ) = 0 ;
	virtual HRESULT __stdcall EndScene					( void ) = 0 ;
	virtual HRESULT __stdcall Clear						( DWORD Count, CONST D_D3DRECT* pRects, DWORD Flags, D_D3DCOLOR Color, float Z, DWORD Stencil) = 0 ;
	virtual HRESULT __stdcall SetTransform				( D_D3DTRANSFORMSTATETYPE State, CONST D_D3DMATRIX* pMatrix) = 0 ;
	virtual HRESULT __stdcall GetTransform				( D_D3DTRANSFORMSTATETYPE State, D_D3DMATRIX* pMatrix) = 0 ;
	virtual HRESULT __stdcall MultiplyTransform			( D_D3DTRANSFORMSTATETYPE, CONST D_D3DMATRIX*) = 0 ;
	virtual HRESULT __stdcall SetViewport				( CONST D_D3DVIEWPORT9* pViewport) = 0 ;
	virtual HRESULT __stdcall GetViewport				( D_D3DVIEWPORT9* pViewport) = 0 ;
	virtual HRESULT __stdcall SetMaterial				( CONST D_D3DMATERIAL9* pMaterial) = 0 ;
	virtual HRESULT __stdcall GetMaterial				( D_D3DMATERIAL9* pMaterial) = 0 ;
	virtual HRESULT __stdcall SetLight					( DWORD Index, CONST D_D3DLIGHT9*) = 0 ;
	virtual HRESULT __stdcall GetLight					( DWORD Index, D_D3DLIGHT9*) = 0 ;
	virtual HRESULT __stdcall LightEnable				( DWORD Index, BOOL Enable) = 0 ;
	virtual HRESULT __stdcall GetLightEnable			( DWORD Index, BOOL* pEnable) = 0 ;
	virtual HRESULT __stdcall SetClipPlane				( DWORD Index, CONST float* pPlane) = 0 ;
	virtual HRESULT __stdcall GetClipPlane				( DWORD Index, float* pPlane) = 0 ;
	virtual HRESULT __stdcall SetRenderState			( D_D3DRENDERSTATETYPE State, DWORD Value) = 0 ;
	virtual HRESULT __stdcall GetRenderState			( D_D3DRENDERSTATETYPE State, DWORD* pValue) = 0 ;
	virtual HRESULT __stdcall CreateStateBlock			( D_D3DSTATEBLOCKTYPE Type, class D_IDirect3DStateBlock9** ppSB) = 0 ;
	virtual HRESULT __stdcall BeginStateBlock			( void ) = 0 ;
	virtual HRESULT __stdcall EndStateBlock				( class D_IDirect3DStateBlock9** ppSB) = 0 ;
	virtual HRESULT __stdcall SetClipStatus				( CONST D_D3DCLIPSTATUS9* pClipStatus) = 0 ;
	virtual HRESULT __stdcall GetClipStatus				( D_D3DCLIPSTATUS9* pClipStatus) = 0 ;
	virtual HRESULT __stdcall GetTexture				( DWORD Stage, class D_IDirect3DBaseTexture9** ppTexture) = 0 ;
	virtual HRESULT __stdcall SetTexture				( DWORD Stage, class D_IDirect3DBaseTexture9* pTexture) = 0 ;
	virtual HRESULT __stdcall GetTextureStageState		( DWORD Stage, D_D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) = 0 ;
	virtual HRESULT __stdcall SetTextureStageState		( DWORD Stage, D_D3DTEXTURESTAGESTATETYPE Type, DWORD Value) = 0 ;
	virtual HRESULT __stdcall GetSamplerState			( DWORD Sampler, D_D3DSAMPLERSTATETYPE Type, DWORD* pValue) = 0 ;
	virtual HRESULT __stdcall SetSamplerState			( DWORD Sampler, D_D3DSAMPLERSTATETYPE Type, DWORD Value) = 0 ;
	virtual HRESULT __stdcall ValidateDevice			( DWORD* pNumPasses) = 0 ;
	virtual HRESULT __stdcall SetPaletteEntries			( UINT PaletteNumber, CONST PALETTEENTRY* pEntries) = 0 ;
	virtual HRESULT __stdcall GetPaletteEntries			( UINT PaletteNumber, PALETTEENTRY* pEntries) = 0 ;
	virtual HRESULT __stdcall SetCurrentTexturePalette	( UINT PaletteNumber) = 0 ;
	virtual HRESULT __stdcall GetCurrentTexturePalette	( UINT *PaletteNumber) = 0 ;
	virtual HRESULT __stdcall SetScissorRect			( CONST RECT* pRect) = 0 ;
	virtual HRESULT __stdcall GetScissorRect			( RECT* pRect) = 0 ;
	virtual HRESULT __stdcall SetSoftwareVertexProcessing( BOOL bSoftware) = 0 ;
	virtual BOOL    __stdcall GetSoftwareVertexProcessing( void ) = 0 ;
	virtual HRESULT __stdcall SetNPatchMode				( float nSegments) = 0 ;
	virtual float   __stdcall GetNPatchMode				( void ) = 0 ;
	virtual HRESULT __stdcall DrawPrimitive				( D_D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) = 0 ;
	virtual HRESULT __stdcall DrawIndexedPrimitive		( D_D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) = 0 ;
	virtual HRESULT __stdcall DrawPrimitiveUP			( D_D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) = 0 ;
	virtual HRESULT __stdcall DrawIndexedPrimitiveUP	( D_D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D_D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride) = 0 ;
	virtual HRESULT __stdcall ProcessVertices			( UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, class D_IDirect3DVertexBuffer9* pDestBuffer, class D_IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) = 0 ;
	virtual HRESULT __stdcall CreateVertexDeclaration	( CONST D_D3DVERTEXELEMENT9* pVertexElements, class D_IDirect3DVertexDeclaration9** ppDecl) = 0 ;
	virtual HRESULT __stdcall SetVertexDeclaration		( class D_IDirect3DVertexDeclaration9* pDecl) = 0 ;
	virtual HRESULT __stdcall GetVertexDeclaration		( class D_IDirect3DVertexDeclaration9** ppDecl) = 0 ;
	virtual HRESULT __stdcall SetFVF					( DWORD FVF) = 0 ;
	virtual HRESULT __stdcall GetFVF					( DWORD* pFVF) = 0 ;
	virtual HRESULT __stdcall CreateVertexShader		( CONST DWORD* pFunction, class D_IDirect3DVertexShader9** ppShader) = 0 ;
	virtual HRESULT __stdcall SetVertexShader			( class D_IDirect3DVertexShader9* pShader) = 0 ;
	virtual HRESULT __stdcall GetVertexShader			( class D_IDirect3DVertexShader9** ppShader) = 0 ;
	virtual HRESULT __stdcall SetVertexShaderConstantF	( UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) = 0 ;
	virtual HRESULT __stdcall GetVertexShaderConstantF	( UINT StartRegister, float* pConstantData, UINT Vector4fCount) = 0 ;
	virtual HRESULT __stdcall SetVertexShaderConstantI	( UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) = 0 ;
	virtual HRESULT __stdcall GetVertexShaderConstantI	( UINT StartRegister, int* pConstantData, UINT Vector4iCount) = 0 ;
	virtual HRESULT __stdcall SetVertexShaderConstantB	( UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) = 0 ;
	virtual HRESULT __stdcall GetVertexShaderConstantB	( UINT StartRegister, BOOL* pConstantData, UINT BoolCount) = 0 ;
	virtual HRESULT __stdcall SetStreamSource			( UINT StreamNumber, class D_IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) = 0 ;
	virtual HRESULT __stdcall GetStreamSource			( UINT StreamNumber, class D_IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride) = 0 ;
	virtual HRESULT __stdcall SetStreamSourceFreq		( UINT StreamNumber, UINT Divider) = 0 ;
	virtual HRESULT __stdcall GetStreamSourceFreq		( UINT StreamNumber, UINT* Divider) = 0 ;
	virtual HRESULT __stdcall SetIndices				( class D_IDirect3DIndexBuffer9* pIndexData) = 0 ;
	virtual HRESULT __stdcall GetIndices				( class D_IDirect3DIndexBuffer9** ppIndexData) = 0 ;
	virtual HRESULT __stdcall CreatePixelShader			( CONST DWORD* pFunction, class D_IDirect3DPixelShader9** ppShader) = 0 ;
	virtual HRESULT __stdcall SetPixelShader			( class D_IDirect3DPixelShader9* pShader) = 0 ;
	virtual HRESULT __stdcall GetPixelShader			( class D_IDirect3DPixelShader9** ppShader) = 0 ;
	virtual HRESULT __stdcall SetPixelShaderConstantF	( UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount) = 0 ;
	virtual HRESULT __stdcall GetPixelShaderConstantF	( UINT StartRegister, float* pConstantData, UINT Vector4fCount) = 0 ;
	virtual HRESULT __stdcall SetPixelShaderConstantI	( UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount) = 0 ;
	virtual HRESULT __stdcall GetPixelShaderConstantI	( UINT StartRegister, int* pConstantData, UINT Vector4iCount) = 0 ;
	virtual HRESULT __stdcall SetPixelShaderConstantB	( UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount) = 0 ;
	virtual HRESULT __stdcall GetPixelShaderConstantB	( UINT StartRegister, BOOL* pConstantData, UINT BoolCount) = 0 ;
	virtual HRESULT __stdcall DrawRectPatch				( UINT Handle, CONST float* pNumSegs, CONST D_D3DRECTPATCH_INFO* pRectPatchInfo) = 0 ;
	virtual HRESULT __stdcall DrawTriPatch				( UINT Handle, CONST float* pNumSegs, CONST D_D3DTRIPATCH_INFO* pTriPatchInfo) = 0 ;
	virtual HRESULT __stdcall DeletePatch				( UINT Handle) = 0 ;
	virtual HRESULT __stdcall CreateQuery				( D_D3DQUERYTYPE Type, class D_IDirect3DQuery9** ppQuery) = 0 ;
};

class D_IDirect3DDevice9Ex : public D_IDirect3DDevice9
{
public :
	virtual HRESULT __stdcall SetConvolutionMonoKernel	( UINT width,UINT height,float* rows,float* columns) = 0 ;
	virtual HRESULT __stdcall ComposeRects				( D_IDirect3DSurface9* pSrc, D_IDirect3DSurface9* pDst, D_IDirect3DVertexBuffer9* pSrcRectDescs,UINT NumRects,D_IDirect3DVertexBuffer9* pDstRectDescs,D_D3DCOMPOSERECTSOP Operation,int Xoffset,int Yoffset) = 0 ;
	virtual HRESULT __stdcall PresentEx					( CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags) = 0 ;
	virtual HRESULT __stdcall GetGPUThreadPriority		( INT* pPriority) = 0 ;
	virtual HRESULT __stdcall SetGPUThreadPriority		( INT Priority) = 0 ;
	virtual HRESULT __stdcall WaitForVBlank				( UINT iSwapChain) = 0 ;
	virtual HRESULT __stdcall CheckResourceResidency	( D_IDirect3DResource9** pResourceArray,DWORD NumResources) = 0 ;
	virtual HRESULT __stdcall SetMaximumFrameLatency	( UINT MaxLatency) = 0 ;
	virtual HRESULT __stdcall GetMaximumFrameLatency	( UINT* pMaxLatency) = 0 ;
	virtual HRESULT __stdcall CheckDeviceState			( HWND hDestinationWindow) = 0 ;
	virtual HRESULT __stdcall CreateRenderTargetEx		( UINT Width,UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, D_IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) = 0 ;
	virtual HRESULT __stdcall CreateOffscreenPlainSurfaceEx( UINT Width,UINT Height, D_D3DFORMAT Format, D_D3DPOOL Pool, D_IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle,DWORD Usage) = 0 ;
	virtual HRESULT __stdcall CreateDepthStencilSurfaceEx( UINT Width,UINT Height, D_D3DFORMAT Format, D_D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, D_IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DWORD Usage) = 0 ;
	virtual HRESULT __stdcall ResetEx					( D_D3DPRESENT_PARAMETERS* pPresentationParameters, D_D3DDISPLAYMODEEX *pFullscreenDisplayMode ) = 0 ;
	virtual HRESULT __stdcall GetDisplayModeEx			( UINT iSwapChain, D_D3DDISPLAYMODEEX* pMode, D_D3DDISPLAYROTATION* pRotation) = 0 ;
};

class D_IDirect3D9 : public IUnknown
{
public :
	virtual HRESULT __stdcall RegisterSoftwareDevice	( void* pInitializeFunction) = 0 ;
	virtual UINT    __stdcall GetAdapterCount			( void ) = 0 ;
	virtual HRESULT __stdcall GetAdapterIdentifier		( DWORD Adapter, DWORD Flags, D_D3DADAPTER_IDENTIFIER9* pIdentifier) = 0 ;
	virtual UINT    __stdcall GetAdapterModeCount		( DWORD Adapter, D_D3DFORMAT Format) = 0 ;
	virtual HRESULT __stdcall EnumAdapterModes			( DWORD Adapter, D_D3DFORMAT Format, DWORD Mode,D_D3DDISPLAYMODE* pMode) = 0 ;
	virtual HRESULT __stdcall GetAdapterDisplayMode		( DWORD Adapter, D_D3DDISPLAYMODE* pMode) = 0 ;
	virtual HRESULT __stdcall CheckDeviceType			( DWORD Adapter, D_D3DDEVTYPE DevType, D_D3DFORMAT DisplayFormat, D_D3DFORMAT BackBufferFormat, BOOL bWindowed) = 0 ;
	virtual HRESULT __stdcall CheckDeviceFormat			( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT AdapterFormat, DWORD Usage, D_D3DRESOURCETYPE RType, D_D3DFORMAT CheckFormat) = 0 ;
	virtual HRESULT __stdcall CheckDeviceMultiSampleType( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT SurfaceFormat, BOOL Windowed, D_D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) = 0 ;
	virtual HRESULT __stdcall CheckDepthStencilMatch	( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT AdapterFormat, D_D3DFORMAT RenderTargetFormat, D_D3DFORMAT DepthStencilFormat) = 0 ;
	virtual HRESULT __stdcall CheckDeviceFormatConversion( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DFORMAT SourceFormat, D_D3DFORMAT TargetFormat) = 0 ;
	virtual HRESULT __stdcall GetDeviceCaps				( DWORD Adapter, D_D3DDEVTYPE DeviceType, D_D3DCAPS9* pCaps) = 0 ;
	virtual HANDLE  __stdcall GetAdapterMonitor			( DWORD Adapter ) = 0 ;
	virtual HRESULT __stdcall CreateDevice				( DWORD Adapter, D_D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D_D3DPRESENT_PARAMETERS* pPresentationParameters, D_IDirect3DDevice9** ppReturnedDeviceInterface ) = 0 ;
} ;

class D_IDirect3D9Ex : public D_IDirect3D9
{
public :
	virtual UINT    __stdcall GetAdapterModeCountEx		( UINT Adapter,CONST D_D3DDISPLAYMODEFILTER* pFilter ) = 0 ;
	virtual HRESULT __stdcall EnumAdapterModesEx		( UINT Adapter,CONST D_D3DDISPLAYMODEFILTER* pFilter,UINT Mode,D_D3DDISPLAYMODEEX* pMode) = 0 ;
	virtual HRESULT __stdcall GetAdapterDisplayModeEx	( UINT Adapter,D_D3DDISPLAYMODEEX* pMode,D_D3DDISPLAYROTATION* pRotation) = 0 ;
	virtual HRESULT __stdcall CreateDeviceEx			( UINT Adapter, D_D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D_D3DPRESENT_PARAMETERS* pPresentationParameters, D_D3DDISPLAYMODEEX* pFullscreenDisplayMode, D_IDirect3DDevice9Ex** ppReturnedDeviceInterface ) = 0 ;
	virtual HRESULT __stdcall GetAdapterLUID			( UINT Adapter,LUID * pLUID) = 0 ;
};

class D_IDirect3D7 : public IUnknown
{
public :
	virtual HRESULT __stdcall EnumDevices				( LPD_D3DENUMDEVICESCALLBACK7, LPVOID ) = 0 ;
	virtual HRESULT __stdcall CreateDevice				( REFCLSID, D_IDirectDrawSurface7 *, class D_IDirect3DDevice7 ** ) = 0 ;
	virtual HRESULT __stdcall CreateVertexBuffer		( D_D3DVERTEXBUFFERDESC *, class D_IDirect3DVertexBuffer7 **, DWORD ) = 0 ;
	virtual HRESULT __stdcall EnumZBufferFormats		( REFCLSID, LPD_D3DENUMPIXELFORMATSCALLBACK, LPVOID ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// EvictManagedTextures( void ) = 0 ;
} ;

class D_IDirect3DDevice7 : public IUnknown
{
public :
	virtual HRESULT __stdcall GetCaps					( D_D3DDEVICEDESC7 * ) = 0 ;
	virtual HRESULT __stdcall EnumTextureFormats		( LPD_D3DENUMPIXELFORMATSCALLBACK, LPVOID ) = 0 ;
	virtual HRESULT __stdcall BeginScene				( void ) = 0 ;
	virtual HRESULT __stdcall EndScene					( void ) = 0 ;
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetDirect3D( LPDIRECT3D7* ) = 0 ;
	virtual HRESULT __stdcall SetRenderTarget			( D_IDirectDrawSurface7 *,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// GetRenderTarget( D_IDirectDrawSurface7 * * ) = 0 ;
	virtual HRESULT __stdcall Clear						( DWORD, D_D3DRECT *, DWORD, D_D3DCOLOR, D_D3DVALUE, DWORD ) = 0 ;
	virtual HRESULT __stdcall SetTransform				( D_D3DTRANSFORMSTATETYPE, D_D3DMATRIX * ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetTransform( D3DTRANSFORMSTATETYPE,LPD3DMATRIX ) = 0 ;
	virtual HRESULT __stdcall SetViewport				( D_D3DVIEWPORT7 * ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// MultiplyTransform( D3DTRANSFORMSTATETYPE,LPD3DMATRIX ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// GetViewport( LPD3DVIEWPORT7 ) = 0 ;
	virtual HRESULT __stdcall SetMaterial				( D_D3DMATERIAL7 * ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// GetMaterial( LPD3DMATERIAL7 ) = 0 ;
	virtual HRESULT __stdcall SetLight					( DWORD, D_D3DLIGHT7 * ) = 0 ;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// GetLight( DWORD,LPD3DLIGHT7 ) = 0 ;
	virtual HRESULT __stdcall SetRenderState			( D_D3DRENDERSTATETYPE, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// GetRenderState( D3DRENDERSTATETYPE,LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// BeginStateBlock( void	) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// EndStateBlock( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// PreLoad( D_IDirectDrawSurface7 * ) = 0 ;
	virtual HRESULT __stdcall DrawPrimitive				( D_D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall DrawIndexedPrimitive		( D_D3DPRIMITIVETYPE, DWORD, LPVOID, DWORD, LPWORD, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// SetClipStatus( void ) = 0 ;					// SetClipStatus( LPD3DCLIPSTATUS ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// GetClipStatus( LPD3DCLIPSTATUS ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// DrawPrimitiveStrided( D_D3DPRIMITIVETYPE,DWORD,LPD3DDRAWPRIMITIVESTRIDEDDATA,DWORD,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// DrawIndexedPrimitiveStrided( D_D3DPRIMITIVETYPE,DWORD,LPD3DDRAWPRIMITIVESTRIDEDDATA,DWORD,LPWORD,DWORD,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// DrawPrimitiveVB( D_D3DPRIMITIVETYPE,LPDIRECT3DVERTEXBUFFER7,DWORD,DWORD,DWORD ) = 0 ;
	virtual HRESULT __stdcall DrawIndexedPrimitiveVB	( D_D3DPRIMITIVETYPE, D_IDirect3DVertexBuffer7 *, DWORD, DWORD, LPWORD, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// ComputeSphereVisibility( LPD3DVECTOR,LPD3DVALUE,DWORD,DWORD,LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// GetTexture( DWORD,D_IDirectDrawSurface7 * * ) = 0 ;
	virtual HRESULT __stdcall SetTexture				( DWORD, D_IDirectDrawSurface7 * ) = 0 ;
	virtual HRESULT __stdcall NonUse19					( void ) = 0 ;				// GetTextureStageState( DWORD,D_D3DTEXTURESTAGESTATETYPE,LPDWORD ) = 0 ;
	virtual HRESULT __stdcall SetTextureStageState		( DWORD, D_D3DTEXTURESTAGESTATETYPE, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse20					( void ) = 0 ;				// ValidateDevice( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse21					( void ) = 0 ;				// ApplyStateBlock( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse22					( void ) = 0 ;				// CaptureStateBlock( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse23					( void ) = 0 ;				// DeleteStateBlock( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse24					( void ) = 0 ;				// CreateStateBlock( D_D3DSTATEBLOCKTYPE,LPDWORD ) = 0 ;
	virtual HRESULT __stdcall Load						( D_IDirectDrawSurface7 *, LPPOINT, D_IDirectDrawSurface7 *, LPRECT, DWORD ) = 0 ;
	virtual HRESULT __stdcall LightEnable				( DWORD, BOOL ) = 0 ;
	virtual HRESULT __stdcall NonUse25					( void ) = 0 ;				// GetLightEnable( DWORD,BOOL* ) = 0 ;
	virtual HRESULT __stdcall NonUse26					( void ) = 0 ;				// SetClipPlane( DWORD,D3DVALUE* ) = 0 ;
	virtual HRESULT __stdcall NonUse27					( void ) = 0 ;				// GetClipPlane( DWORD,D3DVALUE* ) = 0 ;
	virtual HRESULT __stdcall NonUse28					( void ) = 0 ;				// GetInfo( DWORD,LPVOID,DWORD ) = 0 ;
} ;

class D_IDirect3DVertexBuffer7 : public IUnknown
{
public :
	virtual HRESULT __stdcall Lock						( DWORD, LPVOID * , LPDWORD ) = 0 ;
	virtual HRESULT __stdcall Unlock					( void ) = 0 ;
	virtual HRESULT __stdcall ProcessVertices			( DWORD, DWORD, DWORD, D_IDirect3DVertexBuffer7 *, DWORD, D_IDirect3DDevice7 *, DWORD ) = 0 ;
	virtual HRESULT __stdcall GetVertexBufferDesc		( D_D3DVERTEXBUFFERDESC * ) = 0 ;
	virtual HRESULT __stdcall Optimize					( D_IDirect3DDevice7 *, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse1					( void ) = 0 ;				// ProcessVerticesStrided	( DWORD, DWORD, DWORD, LPD3DDRAWPRIMITIVESTRIDEDDATA, DWORD, LPDIRECT3DDEVICE7, DWORD ) = 0 ;
};

class D_ID3DXBuffer : public IUnknown
{
public :
	virtual LPVOID	__stdcall GetBufferPointer			( void ) = 0 ;
	virtual DWORD	__stdcall GetBufferSize				( void ) = 0 ;
} ;

class D_ID3DXConstantTable : public D_ID3DXBuffer
{
public :
	virtual HRESULT __stdcall GetDesc					( D_D3DXCONSTANTTABLE_DESC *pDesc ) = 0 ;
	virtual HRESULT __stdcall GetConstantDesc			( UINT_PTR hConstant, D_D3DXCONSTANT_DESC *pConstantDesc, UINT *pCount ) = 0 ;
	virtual UINT	__stdcall GetSamplerIndex			( UINT_PTR hConstant ) = 0 ;

	virtual UINT_PTR __stdcall GetConstant				( UINT_PTR hConstant, UINT Index ) = 0 ;
	virtual UINT_PTR __stdcall GetConstantByName		( UINT_PTR hConstant, LPCSTR pName ) = 0 ;
	virtual UINT_PTR __stdcall GetConstantElement		( UINT_PTR hConstant, UINT Index ) = 0 ;

	virtual HRESULT __stdcall SetDefaults				( D_IDirect3DDevice9 *pDevice ) = 0 ;
	virtual HRESULT __stdcall SetValue					( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, LPCVOID pData, UINT Bytes ) = 0 ;
	virtual HRESULT __stdcall SetBool					( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, BOOL b ) = 0 ;
	virtual HRESULT __stdcall SetBoolArray				( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST BOOL* pb, UINT Count ) = 0 ;
	virtual HRESULT __stdcall SetInt					( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, INT n ) = 0 ;
	virtual HRESULT __stdcall SetIntArray				( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST INT* pn, UINT Count ) = 0 ;
	virtual HRESULT __stdcall SetFloat					( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, FLOAT f ) = 0 ;
	virtual HRESULT __stdcall SetFloatArray				( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT* pf, UINT Count ) = 0 ;
	virtual HRESULT __stdcall SetVector					( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXVECTOR4 */* pVector ) = 0 ;
	virtual HRESULT __stdcall SetVectorArray			( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXVECTOR4 */* pVector, UINT Count ) = 0 ;
	virtual HRESULT __stdcall SetMatrix					( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXMATRIX */* pMatrix ) = 0 ;
	virtual HRESULT __stdcall SetMatrixArray			( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXMATRIX */* pMatrix, UINT Count ) = 0 ;
	virtual HRESULT __stdcall SetMatrixPointerArray		( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXMATRIX */** ppMatrix, UINT Count ) = 0 ;
	virtual HRESULT __stdcall SetMatrixTranspose		( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXMATRIX */* pMatrix ) = 0 ;
	virtual HRESULT __stdcall SetMatrixTransposeArray	( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXMATRIX */* pMatrix, UINT Count ) = 0 ;
	virtual HRESULT __stdcall SetMatrixTransposePointerArray( D_IDirect3DDevice9 *pDevice, UINT_PTR hConstant, CONST FLOAT /* D3DXMATRIX */** ppMatrix, UINT Count ) = 0 ;
} ;

#endif // __WINDOWS__

// ＸＩｎｐｕｔ ---------------------------------------------------------------

#define D_XINPUT_GAMEPAD_DPAD_UP				(0x00000001)
#define D_XINPUT_GAMEPAD_DPAD_DOWN				(0x00000002)
#define D_XINPUT_GAMEPAD_DPAD_LEFT				(0x00000004)
#define D_XINPUT_GAMEPAD_DPAD_RIGHT				(0x00000008)
#define D_XINPUT_GAMEPAD_START					(0x00000010)
#define D_XINPUT_GAMEPAD_BACK					(0x00000020)
#define D_XINPUT_GAMEPAD_LEFT_THUMB				(0x00000040)
#define D_XINPUT_GAMEPAD_RIGHT_THUMB			(0x00000080)
#define D_XINPUT_GAMEPAD_LEFT_SHOULDER			(0x00000100)
#define D_XINPUT_GAMEPAD_RIGHT_SHOULDER			(0x00000200)
#define D_XINPUT_GAMEPAD_A						(0x00001000)
#define D_XINPUT_GAMEPAD_B						(0x00002000)
#define D_XINPUT_GAMEPAD_X						(0x00004000)
#define D_XINPUT_GAMEPAD_Y						(0x00008000)

typedef struct tagD_XINPUT_GAMEPAD
{
    WORD										wButtons;
    BYTE										bLeftTrigger;
    BYTE										bRightTrigger;
    SHORT										sThumbLX;
    SHORT										sThumbLY;
    SHORT										sThumbRX;
    SHORT										sThumbRY;
} D_XINPUT_GAMEPAD ;

typedef struct tagD_XINPUT_STATE
{
    DWORD										dwPacketNumber;
    D_XINPUT_GAMEPAD							Gamepad;
} D_XINPUT_STATE ;

typedef struct tagD_XINPUT_VIBRATION
{
    WORD										wLeftMotorSpeed;
    WORD										wRightMotorSpeed;
} D_XINPUT_VIBRATION ;

// ＤｉｒｅｃｔＩｎｐｕｔ -----------------------------------------------------

#define D_DI_OK									(S_OK)
#define D_DIDEVTYPE_KEYBOARD					(3)
#define D_DIDEVTYPE_JOYSTICK					(4)
#define D_DIEDFL_ATTACHEDONLY					(0x00000001)

#define D_DIENUM_STOP							(0)
#define D_DIENUM_CONTINUE						(1)

#define D_DIPROP_BUFFERSIZE						(*(const GUID *)(1))
#define D_DIPROP_RANGE							(*(const GUID *)(4))
#define D_DIPROP_DEADZONE						(*(const GUID *)(5))

#define D_DI_POLLEDDEVICE						((HRESULT)0x00000002L)

#define D_DISCL_EXCLUSIVE						(0x00000001)
#define D_DISCL_NONEXCLUSIVE					(0x00000002)
#define D_DISCL_FOREGROUND						(0x00000004)
#define D_DISCL_BACKGROUND						(0x00000008)

#define D_DIJOFS_X								FIELD_OFFSET(D_DIJOYSTATE, lX)
#define D_DIJOFS_Y								FIELD_OFFSET(D_DIJOYSTATE, lY)
#define D_DIJOFS_Z								FIELD_OFFSET(D_DIJOYSTATE, lZ)
#define D_DIJOFS_RX								FIELD_OFFSET(D_DIJOYSTATE, lRx)
#define D_DIJOFS_RY								FIELD_OFFSET(D_DIJOYSTATE, lRy)
#define D_DIJOFS_RZ								FIELD_OFFSET(D_DIJOYSTATE, lRz)

#define D_DIPH_DEVICE							(0)
#define D_DIPH_BYOFFSET							(1)
#define D_DIDFT_ENUMCOLLECTION(n)				((WORD)(n) << 8)

#define D_DIK_ESCAPE							(0x01)
#define D_DIK_1									(0x02)
#define D_DIK_2									(0x03)
#define D_DIK_3									(0x04)
#define D_DIK_4									(0x05)
#define D_DIK_5									(0x06)
#define D_DIK_6									(0x07)
#define D_DIK_7									(0x08)
#define D_DIK_8									(0x09)
#define D_DIK_9									(0x0A)
#define D_DIK_0									(0x0B)
#define D_DIK_MINUS								(0x0C)
#define D_DIK_EQUALS							(0x0D)
#define D_DIK_BACK								(0x0E)
#define D_DIK_TAB								(0x0F)
#define D_DIK_Q									(0x10)
#define D_DIK_W									(0x11)
#define D_DIK_E									(0x12)
#define D_DIK_R									(0x13)
#define D_DIK_T									(0x14)
#define D_DIK_Y									(0x15)
#define D_DIK_U									(0x16)
#define D_DIK_I									(0x17)
#define D_DIK_O									(0x18)
#define D_DIK_P									(0x19)
#define D_DIK_LBRACKET							(0x1A)
#define D_DIK_RBRACKET							(0x1B)
#define D_DIK_RETURN							(0x1C)
#define D_DIK_LCONTROL							(0x1D)
#define D_DIK_A									(0x1E)
#define D_DIK_S									(0x1F)
#define D_DIK_D									(0x20)
#define D_DIK_F									(0x21)
#define D_DIK_G									(0x22)
#define D_DIK_H									(0x23)
#define D_DIK_J									(0x24)
#define D_DIK_K									(0x25)
#define D_DIK_L									(0x26)
#define D_DIK_SEMICOLON							(0x27)
#define D_DIK_APOSTROPHE						(0x28)
#define D_DIK_GRAVE								(0x29)
#define D_DIK_LSHIFT							(0x2A)
#define D_DIK_BACKSLASH							(0x2B)
#define D_DIK_Z									(0x2C)
#define D_DIK_X									(0x2D)
#define D_DIK_C									(0x2E)
#define D_DIK_V									(0x2F)
#define D_DIK_B									(0x30)
#define D_DIK_N									(0x31)
#define D_DIK_M									(0x32)
#define D_DIK_COMMA								(0x33)
#define D_DIK_PERIOD							(0x34)
#define D_DIK_SLASH								(0x35)
#define D_DIK_RSHIFT							(0x36)
#define D_DIK_MULTIPLY							(0x37)
#define D_DIK_LMENU								(0x38)
#define D_DIK_SPACE								(0x39)
#define D_DIK_CAPITAL							(0x3A)
#define D_DIK_F1								(0x3B)
#define D_DIK_F2								(0x3C)
#define D_DIK_F3								(0x3D)
#define D_DIK_F4								(0x3E)
#define D_DIK_F5								(0x3F)
#define D_DIK_F6								(0x40)
#define D_DIK_F7								(0x41)
#define D_DIK_F8								(0x42)
#define D_DIK_F9								(0x43)
#define D_DIK_F10								(0x44)
#define D_DIK_NUMLOCK							(0x45)
#define D_DIK_SCROLL							(0x46)
#define D_DIK_NUMPAD7							(0x47)
#define D_DIK_NUMPAD8							(0x48)
#define D_DIK_NUMPAD9							(0x49)
#define D_DIK_SUBTRACT							(0x4A)
#define D_DIK_NUMPAD4							(0x4B)
#define D_DIK_NUMPAD5							(0x4C)
#define D_DIK_NUMPAD6							(0x4D)
#define D_DIK_ADD								(0x4E)
#define D_DIK_NUMPAD1							(0x4F)
#define D_DIK_NUMPAD2							(0x50)
#define D_DIK_NUMPAD3							(0x51)
#define D_DIK_NUMPAD0							(0x52)
#define D_DIK_DECIMAL							(0x53)
#define D_DIK_OEM_102							(0x56)
#define D_DIK_F11								(0x57)
#define D_DIK_F12								(0x58)
#define D_DIK_F13								(0x64)
#define D_DIK_F14								(0x65)
#define D_DIK_F15								(0x66)
#define D_DIK_KANA								(0x70)
#define D_DIK_ABNT_C1							(0x73)
#define D_DIK_CONVERT							(0x79)
#define D_DIK_NOCONVERT							(0x7B)
#define D_DIK_YEN								(0x7D)
#define D_DIK_ABNT_C2							(0x7E)
#define D_DIK_NUMPADEQUALS						(0x8D)
#define D_DIK_PREVTRACK							(0x90)
#define D_DIK_AT								(0x91)
#define D_DIK_COLON								(0x92)
#define D_DIK_UNDERLINE							(0x93)
#define D_DIK_KANJI								(0x94)
#define D_DIK_STOP								(0x95)
#define D_DIK_AX								(0x96)
#define D_DIK_UNLABELED							(0x97)
#define D_DIK_NEXTTRACK							(0x99)
#define D_DIK_NUMPADENTER						(0x9C)
#define D_DIK_RCONTROL							(0x9D)
#define D_DIK_MUTE								(0xA0)
#define D_DIK_CALCULATOR						(0xA1)
#define D_DIK_PLAYPAUSE							(0xA2)
#define D_DIK_MEDIASTOP							(0xA4)
#define D_DIK_VOLUMEDOWN						(0xAE)
#define D_DIK_VOLUMEUP							(0xB0)
#define D_DIK_WEBHOME							(0xB2)
#define D_DIK_NUMPADCOMMA						(0xB3)
#define D_DIK_DIVIDE							(0xB5)
#define D_DIK_SYSRQ								(0xB7)
#define D_DIK_RMENU								(0xB8)
#define D_DIK_PAUSE								(0xC5)
#define D_DIK_HOME								(0xC7)
#define D_DIK_UP								(0xC8)
#define D_DIK_PRIOR								(0xC9)
#define D_DIK_LEFT								(0xCB)
#define D_DIK_RIGHT								(0xCD)
#define D_DIK_END								(0xCF)
#define D_DIK_DOWN								(0xD0)
#define D_DIK_NEXT								(0xD1)
#define D_DIK_INSERT							(0xD2)
#define D_DIK_DELETE							(0xD3)
#define D_DIK_LWIN								(0xDB)
#define D_DIK_RWIN								(0xDC)
#define D_DIK_APPS								(0xDD)
#define D_DIK_POWER								(0xDE)
#define D_DIK_SLEEP								(0xDF)
#define D_DIK_WAKE								(0xE3)
#define D_DIK_WEBSEARCH							(0xE5)
#define D_DIK_WEBFAVORITES						(0xE6)
#define D_DIK_WEBREFRESH						(0xE7)
#define D_DIK_WEBSTOP							(0xE8)
#define D_DIK_WEBFORWARD						(0xE9)
#define D_DIK_WEBBACK							(0xEA)
#define D_DIK_MYCOMPUTER						(0xEB)
#define D_DIK_MAIL								(0xEC)
#define D_DIK_MEDIASELECT						(0xED)

#define D_DIK_BACKSPACE							D_DIK_BACK
#define D_DIK_NUMPADSTAR						D_DIK_MULTIPLY
#define D_DIK_LALT								D_DIK_LMENU
#define D_DIK_CAPSLOCK							D_DIK_CAPITAL
#define D_DIK_NUMPADMINUS						D_DIK_SUBTRACT
#define D_DIK_NUMPADPLUS						D_DIK_ADD
#define D_DIK_NUMPADPERIOD						D_DIK_DECIMAL
#define D_DIK_NUMPADSLASH						D_DIK_DIVIDE
#define D_DIK_RALT								D_DIK_RMENU
#define D_DIK_UPARROW							D_DIK_UP
#define D_DIK_PGUP								D_DIK_PRIOR
#define D_DIK_LEFTARROW							D_DIK_LEFT
#define D_DIK_RIGHTARROW						D_DIK_RIGHT
#define D_DIK_DOWNARROW							D_DIK_DOWN
#define D_DIK_PGDN								D_DIK_NEXT

#define D_DI_DEGREES							(100)
#define D_DI_FFNOMINALMAX						(10000)
#define D_DI_SECONDS							(1000000)

#define D_DIEFF_OBJECTIDS						(0x00000001)
#define D_DIEFF_OBJECTOFFSETS					(0x00000002)
#define D_DIEFF_CARTESIAN						(0x00000010)
#define D_DIEFF_POLAR							(0x00000020)
#define D_DIEFF_SPHERICAL						(0x00000040)

#define D_DIJOFS_BUTTON(n)						(FIELD_OFFSET(D_DIJOYSTATE, rgbButtons) + (n))

#define D_DIEP_DURATION							(0x00000001)
#define D_DIEP_SAMPLEPERIOD						(0x00000002)
#define D_DIEP_GAIN								(0x00000004)
#define D_DIEP_TRIGGERBUTTON					(0x00000008)
#define D_DIEP_TRIGGERREPEATINTERVAL			(0x00000010)
#define D_DIEP_AXES								(0x00000020)
#define D_DIEP_DIRECTION						(0x00000040)
#define D_DIEP_ENVELOPE							(0x00000080)
#define D_DIEP_TYPESPECIFICPARAMS				(0x00000100)
#define D_DIEP_START							(0x20000000)
#define D_DIEP_NORESTART						(0x40000000)
#define D_DIEP_NODOWNLOAD						(0x80000000)
#define D_DIEB_NOTRIGGER						(0xFFFFFFFF)

#define D_DIEFT_ALL								(0x00000000)
#define D_DIEFT_CONSTANTFORCE					(0x00000001)
#define D_DIEFT_RAMPFORCE						(0x00000002)
#define D_DIEFT_PERIODIC						(0x00000003)
#define D_DIEFT_CONDITION						(0x00000004)
#define D_DIEFT_CUSTOMFORCE						(0x00000005)
#define D_DIEFT_HARDWARE						(0x000000FF)
#define D_DIEFT_FFATTACK						(0x00000200)
#define D_DIEFT_FFFADE							(0x00000400)
#define D_DIEFT_SATURATION						(0x00000800)
#define D_DIEFT_POSNEGCOEFFICIENTS				(0x00001000)
#define D_DIEFT_POSNEGSATURATION				(0x00002000)
#define D_DIEFT_DEADBAND						(0x00004000)
#define D_DIEFT_STARTDELAY						(0x00008000)

typedef struct tagD_DIDEVICEOBJECTINSTANCEA
{
	DWORD										dwSize;
	GUID										guidType;
	DWORD										dwOfs;
	DWORD										dwType;
	DWORD										dwFlags;
	CHAR										tszName[MAX_PATH];
//		#if(DIRECTINPUT_VERSION >= 0x0500)
	DWORD										dwFFMaxForce;
	DWORD										dwFFForceResolution;
	WORD										wCollectionNumber;
	WORD										wDesignatorIndex;
	WORD										wUsagePage;
	WORD										wUsage;
	DWORD										dwDimension;
	WORD										wExponent;
	WORD										wReportId;
//		#endif
} D_DIDEVICEOBJECTINSTANCEA;

typedef struct tagD_DIDEVICEOBJECTINSTANCEW
{
	DWORD										dwSize;
	GUID										guidType;
	DWORD										dwOfs;
	DWORD										dwType;
	DWORD										dwFlags;
	WCHAR										tszName[MAX_PATH];
//		#if(DIRECTINPUT_VERSION >= 0x0500)
	DWORD										dwFFMaxForce;
	DWORD										dwFFForceResolution;
	WORD										wCollectionNumber;
	WORD										wDesignatorIndex;
	WORD										wUsagePage;
	WORD										wUsage;
	DWORD										dwDimension;
	WORD										wExponent;
	WORD										wReportId;
//		#endif
} D_DIDEVICEOBJECTINSTANCEW;

typedef struct tagD_DIDEVICEINSTANCEA
{
	DWORD										dwSize;
	GUID										guidInstance;
	GUID										guidProduct;
	DWORD										dwDevType;
#ifdef UNICODE
	WCHAR										tszInstanceName[MAX_PATH];
	WCHAR										tszProductName[MAX_PATH];
#else // UNICODE
	CHAR										tszInstanceName[MAX_PATH];
	CHAR										tszProductName[MAX_PATH];
#endif // UNICODE
//		#if (DIRECTINPUT_VERSION >= 0x0500)
	GUID										guidFFDriver;
	WORD										wUsagePage;
	WORD										wUsage;
//		#endif
} D_DIDEVICEINSTANCEA ;

typedef struct tagD_DIDEVICEINSTANCEW
{
	DWORD										dwSize;
	GUID										guidInstance;
	GUID										guidProduct;
	DWORD										dwDevType;
	WCHAR										tszInstanceName[MAX_PATH];
	WCHAR										tszProductName[MAX_PATH];
//		#if (DIRECTINPUT_VERSION >= 0x0500)
	GUID										guidFFDriver;
	WORD										wUsagePage;
	WORD										wUsage;
//		#endif
} D_DIDEVICEINSTANCEW ;

typedef struct tagD_DIPROPHEADER
{
	DWORD										dwSize;
	DWORD										dwHeaderSize;
	DWORD										dwObj;
	DWORD										dwHow;
} D_DIPROPHEADER ;

typedef struct tagD_DIPROPRANGE
{
	D_DIPROPHEADER 								diph;
	LONG										lMin;
	LONG										lMax;
} D_DIPROPRANGE ;

typedef struct tagD_DIPROPDWORD
{
	D_DIPROPHEADER 								diph;
	DWORD										dwData;
} D_DIPROPDWORD ;

typedef struct tagD_DIDEVICEOBJECTDATA_DX8
{
	DWORD										dwOfs;
	DWORD										dwData;
	DWORD										dwTimeStamp;
	DWORD										dwSequence;
	UINT_PTR									uAppData;
} D_DIDEVICEOBJECTDATA_DX8 ;

typedef struct tagD_DIDEVICEOBJECTDATA_DX7
{
	DWORD										dwOfs;
	DWORD										dwData;
	DWORD										dwTimeStamp;
	DWORD										dwSequence;
} D_DIDEVICEOBJECTDATA_DX7 ;

typedef struct tagD_DIJOYSTATE
{
	LONG										lX;
	LONG										lY;
	LONG										lZ;
	LONG										lRx;
	LONG										lRy;
	LONG										lRz;
	LONG										rglSlider[2];
	DWORD										rgdwPOV[4];
	BYTE										rgbButtons[32];
} D_DIJOYSTATE ;

typedef struct tagD_DIJOYSTATE2
{
	LONG										lX;
	LONG										lY;
	LONG										lZ;
	LONG										lRx;
	LONG										lRy;
	LONG										lRz;
	LONG										rglSlider[2];
	DWORD										rgdwPOV[4];
	BYTE										rgbButtons[128];
	LONG										lVX;
	LONG										lVY;
	LONG										lVZ;
	LONG										lVRx;
	LONG										lVRy;
	LONG										lVRz;
	LONG										rglVSlider[2];
	LONG										lAX;
	LONG										lAY;
	LONG										lAZ;
	LONG										lARx;
	LONG										lARy;
	LONG										lARz;
	LONG										rglASlider[2];
	LONG										lFX;
	LONG										lFY;
	LONG										lFZ;
	LONG										lFRx;
	LONG										lFRy;
	LONG										lFRz;
	LONG										rglFSlider[2];
} D_DIJOYSTATE2 ;

typedef struct tagD_DIMOUSESTATE
{
	LONG										lX;
	LONG										lY;
	LONG										lZ;
	BYTE										rgbButtons[4];
} D_DIMOUSESTATE ;

typedef struct tagD_DIMOUSESTATE2
{
	LONG										lX;
	LONG										lY;
	LONG										lZ;
	BYTE										rgbButtons[8];
} D_DIMOUSESTATE2 ;

typedef struct tagD_DIOBJECTDATAFORMAT
{
	const GUID *pguid;
	DWORD										dwOfs;
	DWORD										dwType;
	DWORD										dwFlags;
} D_DIOBJECTDATAFORMAT ;

typedef struct tagD_DIDATAFORMAT
{
	DWORD										dwSize;
	DWORD										dwObjSize;
	DWORD										dwFlags;
	DWORD										dwDataSize;
	DWORD										dwNumObjs;
	D_DIOBJECTDATAFORMAT						*rgodf;
} D_DIDATAFORMAT ;

typedef struct tagD_DIDEVCAPS
{
	DWORD										dwSize;
	DWORD										dwFlags;
	DWORD										dwDevType;
	DWORD										dwAxes;
	DWORD										dwButtons;
	DWORD										dwPOVs;
//	#if(DIRECTINPUT_VERSION >= 0x0500)
	DWORD										dwFFSamplePeriod;
	DWORD										dwFFMinTimeResolution;
	DWORD										dwFirmwareRevision;
	DWORD										dwHardwareRevision;
	DWORD										dwFFDriverVersion;
//	#endif
} D_DIDEVCAPS ;

typedef struct tagD_DIPERIODIC
{
	DWORD										dwMagnitude;
	LONG										lOffset;
	DWORD										dwPhase;
	DWORD										dwPeriod;
} D_DIPERIODIC ;

typedef struct tagD_DIENVELOPE
{
	DWORD										dwSize;
	DWORD										dwAttackLevel;
	DWORD										dwAttackTime;
	DWORD										dwFadeLevel;
	DWORD										dwFadeTime;
} D_DIENVELOPE;

typedef struct tagD_DIEFFECT
{
	DWORD										dwSize;
	DWORD										dwFlags;
	DWORD										dwDuration;
	DWORD										dwSamplePeriod;
	DWORD										dwGain;
	DWORD										dwTriggerButton;
	DWORD										dwTriggerRepeatInterval;
	DWORD										cAxes;
	DWORD										*rgdwAxes;
	LONG										*rglDirection;
	D_DIENVELOPE								*lpEnvelope;
	DWORD										cbTypeSpecificParams;
	LPVOID										lpvTypeSpecificParams;
//	#if(DIRECTINPUT_VERSION >= 0x0600)
	DWORD										dwStartDelay;
//	#endif
} D_DIEFFECT ;

typedef struct tagD_DIEFFECTINFOA
{
	DWORD										dwSize;
	GUID										guid;
	DWORD										dwEffType;
	DWORD										dwStaticParams;
	DWORD										dwDynamicParams;
	CHAR										tszName[MAX_PATH];
} D_DIEFFECTINFOA ;

typedef struct tagD_DIEFFECTINFOW
{
	DWORD										dwSize;
	GUID										guid;
	DWORD										dwEffType;
	DWORD										dwStaticParams;
	DWORD										dwDynamicParams;
	WCHAR										tszName[MAX_PATH];
} D_DIEFFECTINFOW ;

typedef struct tagD_DICONSTANTFORCE
{
    LONG										lMagnitude;
} D_DICONSTANTFORCE ;

#ifdef __WINDOWS__

typedef BOOL ( FAR PASCAL * LPD_DIENUMDEVICESCALLBACKA )( const D_DIDEVICEINSTANCEA *, LPVOID ) ;
typedef BOOL ( FAR PASCAL * LPD_DIENUMDEVICESCALLBACKW )( const D_DIDEVICEINSTANCEW *, LPVOID ) ;
typedef BOOL ( FAR PASCAL * LPD_DIENUMEFFECTSCALLBACKA)( const D_DIEFFECTINFOA *, LPVOID);
typedef BOOL ( FAR PASCAL * LPD_DIENUMEFFECTSCALLBACKW)( const D_DIEFFECTINFOW *, LPVOID);

class D_IDirectInputDevice : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetCapabilities( LPDIDEVCAPS ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// EnumObjects( LPDIENUMDEVICEOBJECTSCALLBACK, LPVOID,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetProperty( REFGUID, LPDIPROPHEADER ) = 0 ;
	virtual HRESULT __stdcall SetProperty				( REFGUID, const D_DIPROPHEADER * ) = 0 ;
	virtual HRESULT __stdcall Acquire					( void ) = 0 ;
	virtual HRESULT __stdcall Unacquire					( void ) = 0 ;
	virtual HRESULT __stdcall GetDeviceState			( DWORD, LPVOID ) = 0 ;
	virtual HRESULT __stdcall GetDeviceData				( DWORD, void * /*D_DIDEVICEOBJECTDATA * */, LPDWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall SetDataFormat				( const D_DIDATAFORMAT * ) = 0 ;
	virtual HRESULT __stdcall SetEventNotification		( HANDLE ) = 0 ;
	virtual HRESULT __stdcall SetCooperativeLevel		( HWND, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// GetObjectInfo( LPDIDEVICEOBJECTINSTANCE, DWORD, DWORD ) = 0 ;
	virtual HRESULT __stdcall GetDeviceInfo				( D_DIDEVICEINSTANCEA * ) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// RunControlPanel( HWND, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// Initialize( HINSTANCE, DWORD, REFGUID ) = 0 ;
};

class D_IDirectInputDevice2 : public D_IDirectInputDevice
{
public :
	virtual HRESULT __stdcall CreateEffect				( REFGUID, const D_DIEFFECT *, class D_IDirectInputEffect **, IUnknown * ) = 0 ;
	virtual HRESULT __stdcall EnumEffects				( LPD_DIENUMEFFECTSCALLBACKA, LPVOID,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// GetEffectInfo( LPDIEFFECTINFOA,REFGUID ) = 0 ;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// GetForceFeedbackState( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// SendForceFeedbackCommand( DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse11					( void ) = 0 ;				// EnumCreatedEffectObjects( LPDIENUMCREATEDEFFECTOBJECTSCALLBACK,LPVOID,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse12					( void ) = 0 ;				// Escape( LPDIEFFESCAPE ) = 0 ;
	virtual HRESULT __stdcall Poll						( void ) = 0 ;
	virtual HRESULT __stdcall NonUse13					( void ) = 0 ;				// SendDeviceData( DWORD,LPCDIDEVICEOBJECTDATA,LPDWORD,DWORD ) = 0 ;
} ;

class D_IDirectInputDevice7 : public D_IDirectInputDevice2
{
public :
	virtual HRESULT __stdcall NonUse14					( void ) = 0 ;				// EnumEffectsInFile( LPCSTR,LPDIENUMEFFECTSINFILECALLBACK,LPVOID,DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse15					( void ) = 0 ;				// WriteEffectToFile( LPCSTR,DWORD,LPDIFILEEFFECT,DWORD ) = 0 ;
} ;

class D_IDirectInputDevice8 : public D_IDirectInputDevice7
{
public :
	virtual HRESULT __stdcall NonUse16					( void ) = 0 ;				// BuildActionMap( LPDIACTIONFORMATW,LPCWSTR,DWORD) = 0 ;
	virtual HRESULT __stdcall NonUse17					( void ) = 0 ;				// SetActionMap( LPDIACTIONFORMATW,LPCWSTR,DWORD) = 0 ;
	virtual HRESULT __stdcall NonUse18					( void ) = 0 ;				// GetImageInfo( LPDIDEVICEIMAGEINFOHEADERW) = 0 ;
};

class D_IDirectInput8 : public IUnknown
{
public :
	virtual HRESULT __stdcall CreateDevice				( REFGUID, D_IDirectInputDevice8 **, IUnknown * ) = 0 ;
	virtual HRESULT __stdcall EnumDevices				( DWORD, LPD_DIENUMDEVICESCALLBACKA, LPVOID, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// GetDeviceStatus( REFGUID) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// RunControlPanel( HWND,DWORD) = 0 ;
	virtual HRESULT __stdcall Initialize				( HINSTANCE, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// FindDevice( REFGUID,LPCWSTR,LPGUID) = 0 ;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// EnumDevicesBySemantics( LPCWSTR,LPDIACTIONFORMATW,LPDIENUMDEVICESBYSEMANTICSCBW,LPVOID,DWORD) = 0 ;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// ConfigureDevices( LPDICONFIGUREDEVICESCALLBACK,LPDICONFIGUREDEVICESPARAMSW,DWORD,LPVOID) = 0 ;
};

class D_IDirectInput : public IUnknown
{
public :
	virtual HRESULT __stdcall CreateDevice				( REFGUID, D_IDirectInputDevice **, IUnknown * ) = 0 ;
	virtual HRESULT __stdcall EnumDevices				( DWORD, LPD_DIENUMDEVICESCALLBACKA, LPVOID, DWORD ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// GetDeviceStatus( REFGUID ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// RunControlPanel( HWND, DWORD ) = 0 ;
	virtual HRESULT __stdcall Initialize				( HINSTANCE, DWORD ) = 0 ;
};

class D_IDirectInput2 : public D_IDirectInput
{
public :
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// FindDevice( REFGUID, LPCSTR, LPGUID ) = 0 ;
} ;

class D_IDirectInput7 : public D_IDirectInput2
{
public :
	virtual HRESULT __stdcall CreateDeviceEx			( REFGUID, REFIID, LPVOID *, IUnknown * ) = 0 ;
} ;

class D_IDirectInputEffect : public IUnknown
{
public :
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// Initialize( HINSTANCE,DWORD,REFGUID ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// GetEffectGuid( LPGUID ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetParameters( LPDIEFFECT,DWORD ) = 0 ;
	virtual HRESULT __stdcall SetParameters				( const D_DIEFFECT *, DWORD ) = 0 ;
	virtual HRESULT __stdcall Start						( DWORD,DWORD ) = 0 ;
	virtual HRESULT __stdcall Stop						( void ) = 0 ;
	virtual HRESULT __stdcall GetEffectStatus			( LPDWORD ) = 0 ;
	virtual HRESULT __stdcall Download					( void ) = 0 ;
	virtual HRESULT __stdcall Unload					( void ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// Escape( LPDIEFFESCAPE ) = 0 ;
} ;

#endif // __WINDOWS__

}

#ifdef __WINDOWS__

#ifndef DX_NON_MOVIE
#ifndef DX_NON_DSHOW_MOVIE

// ＤｉｒｅｃｔＳｈｏｗ -------------------------------------------------------

namespace DxLib
{

#define D_AMMSF_NOGRAPHTHREAD					(0x1)
#define D_AMMSF_ADDDEFAULTRENDERER				(0x1)
#define D_DDSFF_PROGRESSIVERENDER				(0x1)

#define D_MS_SUCCESS_CODE(x)					MAKE_HRESULT(0, FACILITY_ITF, x)
#define D_MS_S_PENDING							D_MS_SUCCESS_CODE(1)

#define D_EC_COMPLETE							(0x01)
#define D_EC_USERABORT							(0x02)
#define D_EC_ERRORABORT							(0x03)
#define D_EC_STREAM_ERROR_STOPPED				(0x06)
#define D_EC_STREAM_ERROR_STILLPLAYING			(0x07)
#define D_EC_ERROR_STILLPLAYING					(0x08)
#define D_EC_END_OF_SEGMENT						(0x1C)

typedef enum tagD_AM_SEEKING_SEEKINGFLAGS
{
	D_AM_SEEKING_NoPositioning					= 0,
	D_AM_SEEKING_AbsolutePositioning			= 0x1,
	D_AM_SEEKING_RelativePositioning			= 0x2,
	D_AM_SEEKING_IncrementalPositioning			= 0x3,
	D_AM_SEEKING_PositioningBitsMask			= 0x3,
	D_AM_SEEKING_SeekToKeyFrame					= 0x4,
	D_AM_SEEKING_ReturnTime						= 0x8,
	D_AM_SEEKING_Segment						= 0x10,
	D_AM_SEEKING_NoFlush						= 0x20
} D_AM_SEEKING_SEEKING_FLAGS ;

typedef enum tagD_STREAM_STATE
{
	D_STREAMSTATE_STOP 							= 0,
	D_STREAMSTATE_RUN							= 1
} D_STREAM_STATE ;

typedef enum tagD_STREAM_TYPE
{
	D_STREAMTYPE_READ							= 0,
	D_STREAMTYPE_WRITE 							= 1,
	D_STREAMTYPE_TRANSFORM 						= 2
} D_STREAM_TYPE ;

typedef enum tagD_FILTERSTATE
{
	D_State_Stopped 							= 0,
	D_State_Paused 								= D_State_Stopped + 1,
	D_State_Running 							= D_State_Paused + 1
} D_FILTER_STATE ;

typedef enum tagD_VMR9Mode
{
	D_VMR9Mode_Windowed							= 0x1,
	D_VMR9Mode_Windowless						= 0x2,
	D_VMR9Mode_Renderless						= 0x4,
	D_VMR9Mode_Mask								= 0x7
} D_VMR9Mode ;

typedef enum tagD_VMR9SurfaceAllocationFlags
{
	D_VMR9AllocFlag_3DRenderTarget				= 0x1,
	D_VMR9AllocFlag_DXVATarget					= 0x2,
	D_VMR9AllocFlag_TextureSurface				= 0x4,
	D_VMR9AllocFlag_OffscreenSurface			= 0x8,
	D_VMR9AllocFlag_UsageReserved				= 0xf0,
	D_VMR9AllocFlag_UsageMask					= 0xff
} D_VMR9SurfaceAllocationFlags ;

typedef enum tagD_PIN_DIRECTION
{
	D_PINDIR_INPUT								= 0,
	D_PINDIR_OUTPUT								= D_PINDIR_INPUT + 1
} D_PIN_DIRECTION ;

typedef DWORD 									D_OAEVENT ;
typedef DWORD 									D_OAHWND ;
typedef UINT 									D_MMRESULT ;
typedef REFGUID 								D_REFMSPID ;
typedef double									D_REFTIME ;
typedef GUID 									D_MSPID ;
typedef LONGLONG 								D_STREAM_TIME ;
typedef	long									D_OAFilterState ;
typedef DWORD_PTR								D_HEVENT;
typedef DWORD_PTR								D_HSEMAPHORE;

typedef void ( __stdcall *D_PAPCFUNC )( DWORD_PTR dwParam );

typedef struct tagD_VMR9ALLOCATIONINFO
{
	DWORD										dwFlags;
	DWORD										dwWidth;
	DWORD										dwHeight;
	D_D3DFORMAT									Format;
	D_D3DPOOL									Pool;
	DWORD										MinBuffers;
	SIZE										szAspectRatio;
	SIZE										szNativeSize;
} D_VMR9ALLOCATIONINFO ;

typedef struct tagD_VMR9PRESENTATIONINFO
{
	DWORD										dwFlags;
	D_IDirect3DSurface9							*lpSurf;
	D_REFERENCE_TIME							rtStart;
	D_REFERENCE_TIME							rtEnd;
	SIZE										szAspectRatio;
	RECT										rcSrc;
	RECT										rcDst;
	DWORD										dwReserved1;
	DWORD										dwReserved2;
} D_VMR9PRESENTATIONINFO ;

class D_AM_MEDIA_TYPE
{
public:
	GUID										majortype;
	GUID										subtype;
	BOOL										bFixedSizeSamples;
	BOOL										bTemporalCompression;
	ULONG										lSampleSize;
	GUID										formattype;
	IUnknown									*pUnk;
	ULONG										cbFormat;
	BYTE										*pbFormat;
};

typedef struct tagD_PIN_INFO
{
	class D_IBaseFilter							*pFilter ;
	D_PIN_DIRECTION								dir;
	WCHAR										achName[ 128 ];
} D_PIN_INFO;

typedef struct tagD_FILTER_INFO
{
	WCHAR										achName[ 128 ];
	class D_IFilterGraph						*pGraph;
} D_FILTER_INFO;

typedef struct tagD_ALLOCATOR_PROPERTIES
{
	long										cBuffers;
	long										cbBuffer;
	long										cbAlign;
	long										cbPrefix;
} D_ALLOCATOR_PROPERTIES;

typedef enum tagD_QualityMessageType
{
	D_Famine									= 0,
	D_Flood										= D_Famine + 1
} D_QualityMessageType ;

typedef struct tagD_Quality
{
	D_QualityMessageType						Type;
	long										Proportion;
	D_REFERENCE_TIME							Late;
	D_REFERENCE_TIME							TimeStamp;
} D_Quality;

typedef enum tagD_AM_SAMPLE_PROPERTY_FLAGS
{
	AM_SAMPLE_SPLICEPOINT						= 0x1,
	AM_SAMPLE_PREROLL							= 0x2,
	AM_SAMPLE_DATADISCONTINUITY					= 0x4,
	AM_SAMPLE_TYPECHANGED						= 0x8,
	AM_SAMPLE_TIMEVALID							= 0x10,
	AM_SAMPLE_TIMEDISCONTINUITY					= 0x40,
	AM_SAMPLE_FLUSH_ON_PAUSE					= 0x80,
	AM_SAMPLE_STOPVALID							= 0x100,
	AM_SAMPLE_ENDOFSTREAM						= 0x200,
	AM_STREAM_MEDIA								= 0,
	AM_STREAM_CONTROL							= 1
} D_AM_SAMPLE_PROPERTY_FLAGS ;

typedef struct tagD_AM_SAMPLE2_PROPERTIES
{
	DWORD										cbData;
	DWORD										dwTypeSpecificFlags;
	DWORD										dwSampleFlags;
	LONG										lActual;
	D_REFERENCE_TIME							tStart;
	D_REFERENCE_TIME							tStop;
	DWORD										dwStreamId;
	D_AM_MEDIA_TYPE								*pMediaType;
	BYTE										*pbBuffer;
	LONG										cbBuffer;
} D_AM_SAMPLE2_PROPERTIES ;

typedef struct tagD_REGFILTER
{
	CLSID										Clsid;
	LPWSTR										Name;
} D_REGFILTER;

typedef struct tagD_REGPINTYPES
{
	const CLSID									*clsMajorType;
	const CLSID									*clsMinorType;
} D_REGPINTYPES;

typedef struct tagD_REGFILTERPINS
{
	LPWSTR										strName;
	BOOL										bRendered;
	BOOL										bOutput;
	BOOL										bZero;
	BOOL										bMany;
	const CLSID									*clsConnectsToFilter;
	const WCHAR									*strConnectsToPin;
	UINT										nMediaTypes;
	const D_REGPINTYPES							*lpMediaType;
} D_REGFILTERPINS;

typedef D_REGFILTERPINS D_AMOVIESETUP_PIN, * D_PAMOVIESETUP_PIN, * FAR D_LPAMOVIESETUP_PIN ;

typedef struct tagD_AMOVIESETUP_FILTER
{
	const CLSID									*clsID;
	const WCHAR									*strName;
	DWORD										dwMerit;
	UINT										nPins;
	const D_AMOVIESETUP_PIN						*lpPin;
} D_AMOVIESETUP_FILTER, *D_PAMOVIESETUP_FILTER, * FAR D_LPAMOVIESETUP_FILTER;

STDAPI D_AMGetWideString( LPCWSTR pszString, LPWSTR *ppszReturn ) ;

class D_IEnumMediaTypes : public IUnknown
{
public:
	virtual HRESULT __stdcall Next						( ULONG cMediaTypes, D_AM_MEDIA_TYPE **ppMediaTypes, ULONG *pcFetched ) = 0;
	virtual HRESULT __stdcall Skip						( ULONG cMediaTypes ) = 0;
	virtual HRESULT __stdcall Reset						( void ) = 0;
	virtual HRESULT __stdcall Clone						( D_IEnumMediaTypes **ppEnum ) = 0;
};

class D_IPin : public IUnknown
{
public:
	virtual HRESULT __stdcall Connect					( D_IPin *pReceivePin, const D_AM_MEDIA_TYPE *pmt ) = 0 ;
	virtual HRESULT __stdcall ReceiveConnection			( D_IPin *pConnector, const D_AM_MEDIA_TYPE *pmt ) = 0 ;
	virtual HRESULT __stdcall Disconnect				( void ) = 0 ;
	virtual HRESULT __stdcall ConnectedTo				( D_IPin **pPin ) = 0 ;
	virtual HRESULT __stdcall ConnectionMediaType		( D_AM_MEDIA_TYPE *pmt ) = 0 ;
	virtual HRESULT __stdcall QueryPinInfo				( D_PIN_INFO *pInfo ) = 0 ;
	virtual HRESULT __stdcall QueryDirection			( D_PIN_DIRECTION *pPinDir ) = 0 ;
	virtual HRESULT __stdcall QueryId					( LPWSTR *Id ) = 0 ;
	virtual HRESULT __stdcall QueryAccept				( const D_AM_MEDIA_TYPE *pmt ) = 0 ;
	virtual HRESULT __stdcall EnumMediaTypes			( D_IEnumMediaTypes **ppEnum ) = 0 ;
	virtual HRESULT __stdcall QueryInternalConnections	( D_IPin **apPin, ULONG *nPin ) = 0 ;
	virtual HRESULT __stdcall EndOfStream				( void ) = 0 ;
	virtual HRESULT __stdcall BeginFlush				( void ) = 0 ;
	virtual HRESULT __stdcall EndFlush					( void ) = 0 ;
	virtual HRESULT __stdcall NewSegment				( D_REFERENCE_TIME tStart, D_REFERENCE_TIME tStop, double dRate) = 0 ;
};

class D_IEnumPins : public IUnknown
{
public:
	virtual HRESULT __stdcall Next						( ULONG cPins, D_IPin **ppPins, ULONG *pcFetched ) = 0 ;
	virtual HRESULT __stdcall Skip						( ULONG cPins ) = 0 ;
	virtual HRESULT __stdcall Reset						( void ) = 0 ;
	virtual HRESULT __stdcall Clone						( D_IEnumPins **ppEnum ) = 0 ;
};

class D_IReferenceClock : public IUnknown
{
public:
	virtual HRESULT __stdcall GetTime					( D_REFERENCE_TIME *pTime ) = 0 ;
	virtual HRESULT __stdcall AdviseTime				( D_REFERENCE_TIME baseTime, D_REFERENCE_TIME streamTime, D_HEVENT hEvent, DWORD_PTR *pdwAdviseCookie ) = 0;
	virtual HRESULT __stdcall AdvisePeriodic			( D_REFERENCE_TIME startTime, D_REFERENCE_TIME periodTime, D_HSEMAPHORE hSemaphore, DWORD_PTR *pdwAdviseCookie) = 0;
	virtual HRESULT __stdcall Unadvise					( DWORD_PTR dwAdviseCookie) = 0;
} ;

class D_IMediaSample : public IUnknown
{
public:
	virtual HRESULT __stdcall GetPointer				( BYTE **ppBuffer) = 0;
	virtual long    __stdcall GetSize					( void) = 0;
	virtual HRESULT __stdcall GetTime					( D_REFERENCE_TIME *pTimeStart, D_REFERENCE_TIME *pTimeEnd) = 0;
	virtual HRESULT __stdcall SetTime					( D_REFERENCE_TIME *pTimeStart, D_REFERENCE_TIME *pTimeEnd) = 0;
	virtual HRESULT __stdcall IsSyncPoint				( void) = 0;
	virtual HRESULT __stdcall SetSyncPoint				( BOOL bIsSyncPoint) = 0;
	virtual HRESULT __stdcall IsPreroll					( void) = 0;
	virtual HRESULT __stdcall SetPreroll				( BOOL bIsPreroll) = 0;
	virtual long    __stdcall GetActualDataLength		( void) = 0;
	virtual HRESULT __stdcall SetActualDataLength		( long __MIDL_0010) = 0;
	virtual HRESULT __stdcall GetMediaType				( D_AM_MEDIA_TYPE **ppMediaType) = 0;
	virtual HRESULT __stdcall SetMediaType				( D_AM_MEDIA_TYPE *pMediaType) = 0;
	virtual HRESULT __stdcall IsDiscontinuity			( void) = 0;
	virtual HRESULT __stdcall SetDiscontinuity			( BOOL bDiscontinuity) = 0;
	virtual HRESULT __stdcall GetMediaTime				( LONGLONG *pTimeStart, LONGLONG *pTimeEnd) = 0;
	virtual HRESULT __stdcall SetMediaTime				( LONGLONG *pTimeStart, LONGLONG *pTimeEnd) = 0;
};

class D_IMediaSample2 : public D_IMediaSample
{
public:
    virtual HRESULT __stdcall GetProperties				( DWORD cbProperties, BYTE *pbProperties) = 0;
    virtual HRESULT __stdcall SetProperties				( DWORD cbProperties, const BYTE *pbProperties) = 0;
};

class D_IMemAllocator : public IUnknown
{
public:
	virtual HRESULT __stdcall SetProperties				( D_ALLOCATOR_PROPERTIES *pRequest, D_ALLOCATOR_PROPERTIES *pActual) = 0;
	virtual HRESULT __stdcall GetProperties				( D_ALLOCATOR_PROPERTIES *pProps) = 0;
	virtual HRESULT __stdcall Commit					( void) = 0;
	virtual HRESULT __stdcall Decommit					( void) = 0;
	virtual HRESULT __stdcall GetBuffer					( D_IMediaSample **ppBuffer, D_REFERENCE_TIME *pStartTime, D_REFERENCE_TIME *pEndTime, DWORD dwFlags) = 0;
	virtual HRESULT __stdcall ReleaseBuffer				( D_IMediaSample *pBuffer) = 0;
};

class D_IMemAllocatorNotifyCallbackTemp : public IUnknown
{
public:
    virtual HRESULT __stdcall NotifyRelease				() = 0;
};

class D_IMemAllocatorCallbackTemp : public D_IMemAllocator
{
public:
	virtual HRESULT __stdcall SetNotify					( D_IMemAllocatorNotifyCallbackTemp *pNotify) = 0;		
	virtual HRESULT __stdcall GetFreeCount				( LONG *plBuffersFree) = 0;
};

class D_IMemInputPin : public IUnknown
{
public:
	virtual HRESULT __stdcall GetAllocator				( D_IMemAllocator **ppAllocator) = 0;
	virtual HRESULT __stdcall NotifyAllocator			( D_IMemAllocator *pAllocator, BOOL bReadOnly) = 0;
	virtual HRESULT __stdcall GetAllocatorRequirements	( D_ALLOCATOR_PROPERTIES *pProps) = 0;
	virtual HRESULT __stdcall Receive					( D_IMediaSample *pSample) = 0;
	virtual HRESULT __stdcall ReceiveMultiple			( D_IMediaSample **pSamples, long nSamples, long *nSamplesProcessed) = 0;
	virtual HRESULT __stdcall ReceiveCanBlock			( void) = 0;
};

class D_IMediaFilter : public IPersist
{
public:
	virtual HRESULT __stdcall Stop						( void ) = 0 ;
	virtual HRESULT __stdcall Pause						( void ) = 0 ;
	virtual HRESULT __stdcall Run						( D_REFERENCE_TIME tStart ) = 0 ;
	virtual HRESULT __stdcall GetState					( DWORD dwMilliSecsTimeout, D_FILTER_STATE *State ) = 0 ;
	virtual HRESULT __stdcall SetSyncSource				( D_IReferenceClock *pClock ) = 0 ;
	virtual HRESULT __stdcall GetSyncSource				( D_IReferenceClock **pClock ) = 0 ;

};

class D_IBaseFilter : public D_IMediaFilter
{
public:
	virtual HRESULT __stdcall EnumPins					( D_IEnumPins **ppEnum ) = 0 ;
	virtual HRESULT __stdcall FindPin					( LPCWSTR Id, D_IPin **ppPin) = 0;
	virtual HRESULT __stdcall QueryFilterInfo			( D_FILTER_INFO *pInfo) = 0;
	virtual HRESULT __stdcall JoinFilterGraph			( class D_IFilterGraph *pGraph, LPCWSTR pName) = 0;
	virtual HRESULT __stdcall QueryVendorInfo			( LPWSTR *pVendorInfo) = 0;

};

class D_IQualityControl : public IUnknown
{
public:
	virtual HRESULT __stdcall Notify					( D_IBaseFilter *pSelf, D_Quality q ) = 0 ;
	virtual HRESULT __stdcall SetSink					( D_IQualityControl *piqc ) = 0 ;
};

class D_IQualProp : public IUnknown
{
public :
	virtual HRESULT __stdcall get_FramesDroppedInRenderer( int *pcFrames) = 0 ;  // Out
	virtual HRESULT __stdcall get_FramesDrawn			( int *pcFramesDrawn) = 0 ;         // Out
	virtual HRESULT __stdcall get_AvgFrameRate			( int *piAvgFrameRate) = 0 ;       // Out
	virtual HRESULT __stdcall get_Jitter				( int *iJitter) = 0 ;                    // Out
	virtual HRESULT __stdcall get_AvgSyncOffset			( int *piAvg) = 0 ;               // Out
	virtual HRESULT __stdcall get_DevSyncOffset			( int *piDev) = 0 ;               // Out
} ;

class D_IEnumFilters : public IUnknown
{
public:
	virtual HRESULT __stdcall Next						( ULONG cFilters, D_IBaseFilter **ppFilter, ULONG *pcFetched) = 0;
	virtual HRESULT __stdcall Skip						( ULONG cFilters) = 0;
	virtual HRESULT __stdcall Reset						( void) = 0;
	virtual HRESULT __stdcall Clone						( D_IEnumFilters **ppEnum) = 0;
};

class D_IFilterGraph : public IUnknown
{
public:
	virtual HRESULT __stdcall AddFilter					( D_IBaseFilter *pFilter, LPCWSTR pName ) = 0 ;
	virtual HRESULT __stdcall RemoveFilter				( D_IBaseFilter *pFilter) = 0;
	virtual HRESULT __stdcall EnumFilters				( D_IEnumFilters **ppEnum) = 0;	
	virtual HRESULT __stdcall FindFilterByName			( LPCWSTR pName, D_IBaseFilter **ppFilter) = 0;
	virtual HRESULT __stdcall ConnectDirect				( D_IPin *ppinOut, D_IPin *ppinIn, const D_AM_MEDIA_TYPE *pmt) = 0;
	virtual HRESULT __stdcall Reconnect					( D_IPin *ppin) = 0;
	virtual HRESULT __stdcall Disconnect				( D_IPin *ppin) = 0;
	virtual HRESULT __stdcall SetDefaultSyncSource		( void) = 0;
	
};

class D_IGraphBuilder : public D_IFilterGraph
{
public:
	virtual HRESULT __stdcall Connect					( D_IPin *ppinOut, D_IPin *ppinIn) = 0;
	virtual HRESULT __stdcall Render					( D_IPin *ppinOut) = 0;
	virtual HRESULT __stdcall RenderFile				( LPCWSTR lpcwstrFile, LPCWSTR lpcwstrPlayList ) = 0;
	virtual HRESULT __stdcall AddSourceFilter			( LPCWSTR lpcwstrFileName, LPCWSTR lpcwstrFilterName, D_IBaseFilter **ppFilter) = 0;
	virtual HRESULT __stdcall SetLogFile				( DWORD_PTR hFile) = 0;
	virtual HRESULT __stdcall Abort						( void ) = 0;
	virtual HRESULT __stdcall ShouldOperationContinue	( void ) = 0;
};

class D_IFilterGraph2 : public D_IGraphBuilder
{
public:
	virtual HRESULT __stdcall AddSourceFilterForMoniker	( IMoniker *pMoniker, IBindCtx *pCtx, LPCWSTR lpcwstrFilterName, D_IBaseFilter **ppFilter) = 0;
	virtual HRESULT __stdcall ReconnectEx				( D_IPin *ppin, const D_AM_MEDIA_TYPE *pmt) = 0;
	virtual HRESULT __stdcall RenderEx					( D_IPin *pPinOut, DWORD dwFlags, DWORD *pvContext) = 0;
};

class D_IEnumRegFilters : public IUnknown
{
public:
	virtual HRESULT __stdcall Next						( ULONG cFilters, D_REGFILTER **apRegFilter, ULONG *pcFetched) = 0;
	virtual HRESULT __stdcall Skip						( ULONG cFilters) = 0;
	virtual HRESULT __stdcall Reset						( void) = 0;
	virtual HRESULT __stdcall Clone						( D_IEnumRegFilters **ppEnum) = 0;
};

class D_IFilterMapper : public IUnknown
{
public:
	virtual HRESULT __stdcall RegisterFilter			( CLSID clsid, LPCWSTR Name, DWORD dwMerit) = 0;
	virtual HRESULT __stdcall RegisterFilterInstance	( CLSID clsid, LPCWSTR Name, CLSID *MRId) = 0;
	virtual HRESULT __stdcall RegisterPin				( CLSID Filter, LPCWSTR Name, BOOL bRendered, BOOL bOutput, BOOL bZero, BOOL bMany, CLSID ConnectsToFilter, LPCWSTR ConnectsToPin) = 0;
	virtual HRESULT __stdcall RegisterPinType			( CLSID clsFilter, LPCWSTR strName, CLSID clsMajorType, CLSID clsSubType) = 0;
	virtual HRESULT __stdcall UnregisterFilter			( CLSID Filter) = 0;
	virtual HRESULT __stdcall UnregisterFilterInstance	( CLSID MRId) = 0;
	virtual HRESULT __stdcall UnregisterPin				( CLSID Filter, LPCWSTR Name) = 0;
	virtual HRESULT __stdcall EnumMatchingFilters		( D_IEnumRegFilters **ppEnum, DWORD dwMerit, BOOL bInputNeeded, CLSID clsInMaj, CLSID clsInSub, BOOL bRender, BOOL bOututNeeded, CLSID clsOutMaj, CLSID clsOutSub) = 0;
};

class D_IStreamSample : public IUnknown
{
public:
	virtual HRESULT __stdcall GetMediaStream			( class D_IMediaStream **ppMediaStream ) = 0 ;
	virtual HRESULT __stdcall GetSampleTimes			( D_STREAM_TIME *pStartTime, D_STREAM_TIME *pEndTime, D_STREAM_TIME *pCurrentTime) = 0;
	virtual HRESULT __stdcall SetSampleTimes			( const D_STREAM_TIME *pStartTime, const D_STREAM_TIME *pEndTime ) = 0;
	virtual HRESULT __stdcall Update					( DWORD dwFlags, HANDLE hEvent, D_PAPCFUNC pfnAPC, DWORD_PTR dwAPCData) = 0;
	virtual HRESULT __stdcall CompletionStatus			( DWORD dwFlags, DWORD dwMilliseconds) = 0;
};

class D_IDirectDrawStreamSample : public D_IStreamSample
{
public:
	virtual HRESULT __stdcall GetSurface				( D_IDirectDrawSurface **ppDirectDrawSurface, RECT *pRect) = 0;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// SetRect( const RECT *pRect) = 0;
};

class D_IMultiMediaStream : public IUnknown
{
public:
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetInformation( DWORD *pdwFlags, STREAM_TYPE *pStreamType ) = 0 ;
	virtual HRESULT __stdcall GetMediaStream			( D_REFMSPID idPurpose, class D_IMediaStream **ppMediaStream ) = 0 ;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// EnumMediaStreams( long Index, class IMediaStream **ppMediaStream ) = 0 ;
	virtual HRESULT __stdcall GetState					( D_STREAM_STATE *pCurrentState ) = 0 ;
	virtual HRESULT __stdcall SetState					( D_STREAM_STATE NewState ) = 0 ;
	virtual HRESULT __stdcall GetTime					( D_STREAM_TIME *pCurrentTime ) = 0 ;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// GetDuration( D_STREAM_TIME *pDuration ) = 0 ;
	virtual HRESULT __stdcall Seek						( D_STREAM_TIME SeekTime ) = 0 ;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// GetEndOfStreamEventHandle( HANDLE *phEOS ) = 0 ;
} ;

class D_IAMMultiMediaStream : public D_IMultiMediaStream
{
public:
	virtual HRESULT __stdcall Initialize				( D_STREAM_TYPE StreamType, DWORD dwFlags, D_IGraphBuilder *pFilterGraph) = 0;	
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// GetFilterGraph( IGraphBuilder **ppGraphBuilder) = 0;	
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// GetFilter( IMediaStreamFilter **ppFilter) = 0;	
	virtual HRESULT __stdcall AddMediaStream			( IUnknown *pStreamObject, const D_MSPID *PurposeId, DWORD dwFlags, D_IMediaStream **ppNewStream) = 0;
	virtual HRESULT __stdcall OpenFile					( LPCWSTR pszFileName, DWORD dwFlags) = 0;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// OpenMoniker( IBindCtx *pCtx, IMoniker *pMoniker, DWORD dwFlags) = 0;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// Render( DWORD dwFlags) = 0;
} ;

class D_IMediaStream : public IUnknown
{
public:
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetMultiMediaStream( IMultiMediaStream **ppMultiMediaStream) = 0;
	virtual HRESULT __stdcall NonUse01					( void ) = 0 ;				// GetInformation( MSPID *pPurposeId, STREAM_TYPE *pType) = 0;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// SetSameFormat( IMediaStream *pStreamThatHasDesiredFormat, DWORD dwFlags) = 0;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// AllocateSample( DWORD dwFlags, IStreamSample **ppSample) = 0;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// CreateSharedSample( IStreamSample *pExistingSample, DWORD dwFlags, IStreamSample **ppNewSample) = 0;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// SendEndOfStream( DWORD dwFlags) = 0;
};

class D_IAMMediaStream : public D_IMediaStream
{
public:
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// Initialize( IUnknown *pSourceObject, DWORD dwFlags, REFMSPID PurposeId, const STREAM_TYPE StreamType) = 0;
	virtual HRESULT __stdcall SetState( D_FILTER_STATE State) = 0;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// JoinAMMultiMediaStream( IAMMultiMediaStream *pAMMultiMediaStream) = 0;
	virtual HRESULT __stdcall NonUse09					( void ) = 0 ;				// JoinFilter( IMediaStreamFilter *pMediaStreamFilter) = 0;
	virtual HRESULT __stdcall NonUse10					( void ) = 0 ;				// JoinFilterGraph( IFilterGraph *pFilterGraph) = 0;
};

class D_IDirectDrawMediaStream : public D_IMediaStream
{
public:
	virtual HRESULT __stdcall GetFormat					( D_DDSURFACEDESC *pDDSDCurrent, D_IDirectDrawPalette **ppDirectDrawPalette, D_DDSURFACEDESC *pDDSDDesired, DWORD *pdwFlags) = 0;
	virtual HRESULT __stdcall NonUse06					( void ) = 0 ;				// SetFormat( const DDSURFACEDESC *pDDSurfaceDesc, IDirectDrawPalette *pDirectDrawPalette) = 0;
	virtual HRESULT __stdcall NonUse07					( void ) = 0 ;				// GetDirectDraw( IDirectDraw **ppDirectDraw) = 0;
	virtual HRESULT __stdcall NonUse08					( void ) = 0 ;				// SetDirectDraw( IDirectDraw *pDirectDraw) = 0;
	virtual HRESULT __stdcall CreateSample				( D_IDirectDrawSurface *pSurface, const RECT *pRect, DWORD dwFlags, class D_IDirectDrawStreamSample **ppSample) = 0;
	virtual HRESULT __stdcall GetTimePerFrame			( D_STREAM_TIME *pFrameTime) = 0;
};

class D_IMediaControl : public IDispatch
{
public:
	virtual HRESULT __stdcall Run						( void ) = 0 ;
	virtual HRESULT __stdcall Pause						( void ) = 0 ;
	virtual HRESULT __stdcall Stop						( void ) = 0 ;
	virtual HRESULT __stdcall GetState					( LONG msTimeout, D_OAFilterState *pfs ) = 0;
	virtual HRESULT __stdcall RenderFile				( BSTR strFilename) = 0;
	virtual HRESULT __stdcall AddSourceFilter			( BSTR strFilename, IDispatch **ppUnk) = 0;
	virtual HRESULT __stdcall get_FilterCollection		( IDispatch **ppUnk) = 0;
	virtual HRESULT __stdcall get_RegFilterCollection	( IDispatch **ppUnk) = 0;
	virtual HRESULT __stdcall StopWhenReady				( void ) = 0 ;
};

class D_IMediaSeeking : public IUnknown
{
public:
	virtual HRESULT __stdcall GetCapabilities			( DWORD *pCapabilities) = 0;
	virtual HRESULT __stdcall CheckCapabilities			( DWORD *pCapabilities) = 0;
	virtual HRESULT __stdcall IsFormatSupported			( const GUID *pFormat) = 0;
	virtual HRESULT __stdcall QueryPreferredFormat		( GUID *pFormat) = 0;
	virtual HRESULT __stdcall GetTimeFormat				( GUID *pFormat ) = 0 ;
	virtual HRESULT __stdcall IsUsingTimeFormat			( const GUID *pFormat) = 0;
	virtual HRESULT __stdcall SetTimeFormat				( const GUID *pFormat ) = 0 ;
	virtual HRESULT __stdcall GetDuration				( LONGLONG *pDuration) = 0;
	virtual HRESULT __stdcall GetStopPosition			( LONGLONG *pStop) = 0 ;
	virtual HRESULT __stdcall GetCurrentPosition		( LONGLONG *pCurrent) = 0 ;
	virtual HRESULT __stdcall ConvertTimeFormat			( LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat) = 0;
	virtual HRESULT __stdcall SetPositions				( LONGLONG *pCurrent, DWORD dwCurrentFlags, LONGLONG *pStop, DWORD dwStopFlags) = 0;
	virtual HRESULT __stdcall GetPositions				( LONGLONG *pCurrent, LONGLONG *pStop) = 0;
	virtual HRESULT __stdcall GetAvailable				( LONGLONG *pEarliest, LONGLONG *pLatest) = 0;
	virtual HRESULT __stdcall SetRate					( double dRate) = 0;
	virtual HRESULT __stdcall GetRate					( double *pdRate) = 0;
	virtual HRESULT __stdcall GetPreroll				( LONGLONG *pllPreroll) = 0;
};

class D_IMediaPosition : public IDispatch
{
public:
	virtual HRESULT __stdcall get_Duration				( D_REFTIME *plength) = 0;
	virtual HRESULT __stdcall put_CurrentPosition		( D_REFTIME llTime) = 0;
	virtual HRESULT __stdcall get_CurrentPosition		( D_REFTIME *pllTime) = 0;
	virtual HRESULT __stdcall get_StopTime				( D_REFTIME *pllTime) = 0;
	virtual HRESULT __stdcall put_StopTime				( D_REFTIME llTime) = 0;
	virtual HRESULT __stdcall get_PrerollTime			( D_REFTIME *pllTime) = 0;
	virtual HRESULT __stdcall put_PrerollTime			( D_REFTIME llTime) = 0;
	virtual HRESULT __stdcall put_Rate					( double dRate) = 0;
	virtual HRESULT __stdcall get_Rate					( double *pdRate) = 0;
	virtual HRESULT __stdcall CanSeekForward			( LONG *pCanSeekForward) = 0;
	virtual HRESULT __stdcall CanSeekBackward			( LONG *pCanSeekBackward) = 0;
};

class D_ISampleGrabberCB : public IUnknown
{
public:
	virtual HRESULT  __stdcall SampleCB					( double SampleTime, D_IMediaSample *pSample ) = 0;
	virtual HRESULT  __stdcall BufferCB					( double SampleTime, BYTE *pBuffer, long BufferLen ) = 0;
};

class D_ISampleGrabber : public IUnknown
{
public:
	virtual HRESULT __stdcall SetOneShot				( BOOL OneShot) = 0;
	virtual HRESULT __stdcall SetMediaType				( const D_AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT __stdcall GetConnectedMediaType		( D_AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT __stdcall SetBufferSamples			( BOOL BufferThem) = 0;
	virtual HRESULT __stdcall GetCurrentBuffer			( long *pBufferSize, long *pBuffer) = 0;
	virtual HRESULT __stdcall GetCurrentSample			( D_IMediaSample **ppSample) = 0;
	virtual HRESULT __stdcall SetCallback				( D_ISampleGrabberCB *pCallback, long WhichMethodToCallback) = 0;
};

class D_IFileSinkFilter : public IUnknown
{
public:
	virtual HRESULT __stdcall SetFileName				( LPCOLESTR pszFileName, const D_AM_MEDIA_TYPE *pmt) = 0;
	virtual HRESULT __stdcall GetCurFile				( LPOLESTR *ppszFileName, D_AM_MEDIA_TYPE *pmt) = 0;
	
};

class D_IFileSinkFilter2 : public D_IFileSinkFilter
{
public:
	virtual HRESULT __stdcall SetMode					( DWORD dwFlags) = 0;
	virtual HRESULT __stdcall GetMode					( DWORD *pdwFlags) = 0;
	
};

class D_IBasicAudio : public IDispatch
{
public:
	virtual	HRESULT __stdcall put_Volume				( long lVolume) = 0;
	virtual	HRESULT __stdcall get_Volume				( long *plVolume) = 0;
	virtual	HRESULT __stdcall put_Balance				( long lBalance) = 0;
	virtual	HRESULT __stdcall get_Balance				( long *plBalance) = 0;
};

class D_IMediaEvent : public IDispatch
{
public:
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// GetEventHandle( OAEVENT *hEvent ) = 0 ;
	virtual HRESULT __stdcall GetEvent					( long *lEventCode, LONG_PTR *lParam1, LONG_PTR *lParam2, long msTimeout) = 0;
	virtual HRESULT __stdcall WaitForCompletion			( long msTimeout, long *pEvCode) = 0;
	virtual HRESULT __stdcall NonUse02					( void ) = 0 ;				// CancelDefaultHandling( long lEvCode) = 0;
	virtual HRESULT __stdcall NonUse03					( void ) = 0 ;				// RestoreDefaultHandling( long lEvCode) = 0;
	virtual HRESULT __stdcall FreeEventParams			( long lEvCode, LONG_PTR lParam1, LONG_PTR lParam2) = 0;
};

class D_IMediaEventEx : public D_IMediaEvent
{
public:
	virtual HRESULT __stdcall SetNotifyWindow			( D_OAHWND hwnd, long lMsg, LONG_PTR lInstanceData) = 0;
	virtual HRESULT __stdcall NonUse04					( void ) = 0 ;				// SetNotifyFlags( long lNoNotifyFlags) = 0;
	virtual HRESULT __stdcall NonUse05					( void ) = 0 ;				// GetNotifyFlags( long *lplNoNotifyFlags) = 0;
};

class D_IMediaEventSink : public IUnknown
{
public:
	virtual HRESULT __stdcall Notify					( long EventCode, LONG_PTR EventParam1, LONG_PTR EventParam2 ) = 0 ;
};

class D_IVMRSurfaceAllocatorNotify9 : public IUnknown
{
public:
	virtual HRESULT __stdcall AdviseSurfaceAllocator	( DWORD_PTR dwUserID, class D_IVMRSurfaceAllocator9 *lpIVRMSurfaceAllocator) = 0;
	virtual HRESULT __stdcall SetD3DDevice				( D_IDirect3DDevice9 *lpD3DDevice, D_HMONITOR hMonitor) = 0;
	virtual HRESULT __stdcall ChangeD3DDevice			( D_IDirect3DDevice9 *lpD3DDevice, D_HMONITOR hMonitor) = 0;
	virtual HRESULT __stdcall AllocateSurfaceHelper		( D_VMR9ALLOCATIONINFO *lpAllocInfo, DWORD *lpNumBuffers, D_IDirect3DSurface9 **lplpSurface) = 0;
	virtual HRESULT __stdcall NotifyEvent				( LONG EventCode, LONG_PTR Param1, LONG_PTR Param2) = 0;
};

class D_IVMRSurfaceAllocator9 : public IUnknown
{
public:
	virtual HRESULT __stdcall InitializeDevice			( DWORD_PTR dwUserID, D_VMR9ALLOCATIONINFO *lpAllocInfo, DWORD *lpNumBuffers) = 0;
	virtual HRESULT __stdcall TerminateDevice			( DWORD_PTR dwID) = 0;
	virtual HRESULT __stdcall GetSurface				( DWORD_PTR dwUserID, DWORD SurfaceIndex, DWORD SurfaceFlags, D_IDirect3DSurface9 **lplpSurface) = 0;
	virtual HRESULT __stdcall AdviseNotify				( D_IVMRSurfaceAllocatorNotify9 *lpIVMRSurfAllocNotify) = 0;
} ;

class D_IVMRImagePresenter9 : public IUnknown
{
public:
	virtual HRESULT __stdcall StartPresenting			( DWORD_PTR dwUserID) = 0;
	virtual HRESULT __stdcall StopPresenting			( DWORD_PTR dwUserID) = 0;
	virtual HRESULT __stdcall PresentImage				( DWORD_PTR dwUserID, D_VMR9PRESENTATIONINFO *lpPresInfo) = 0;
} ;

class D_IVMRFilterConfig9 : public IUnknown
{
public:
	virtual HRESULT __stdcall NonUse00					( void ) = 0 ;				// SetImageCompositor( D_IVMRImageCompositor9 *lpVMRImgCompositor) = 0;
	virtual HRESULT __stdcall SetNumberOfStreams		( DWORD dwMaxStreams) = 0;
	virtual HRESULT __stdcall GetNumberOfStreams		( DWORD *pdwMaxStreams) = 0;
	virtual HRESULT __stdcall SetRenderingPrefs			( DWORD dwRenderFlags) = 0;
	virtual HRESULT __stdcall GetRenderingPrefs			( DWORD *pdwRenderFlags) = 0;
	virtual HRESULT __stdcall SetRenderingMode			( DWORD Mode) = 0;
	virtual HRESULT __stdcall GetRenderingMode			( DWORD *pMode) = 0;
} ;

class D_IAMovieSetup : public IUnknown
{
public:
	virtual HRESULT __stdcall Register					( void ) = 0 ;
	virtual HRESULT __stdcall Unregister				( void ) = 0 ;
};


struct __D_POSITION { int unused; };
typedef __D_POSITION* D_POSITION;

interface D_INonDelegatingUnknown
{
	virtual HRESULT	__stdcall NonDelegatingQueryInterface( REFIID, LPVOID * ) = 0 ;
	virtual ULONG	__stdcall NonDelegatingAddRef		( void ) = 0 ;
	virtual ULONG	__stdcall NonDelegatingRelease		( void ) = 0 ;
};

class D_CBaseList 
{
public:
	class CNode
	{
		CNode									*m_pPrev;
		CNode									*m_pNext;
		void									*m_pObject;
	public:
		CNode();
			CNode			  *Prev						() const;
			CNode			  *Next						() const;
			void			  SetPrev					(CNode *p);
			void			  SetNext					(CNode *p);
			void			  *GetData					() const;
			void			  SetData					(void *p);
	};

	class CNodeCache
	{
	public:
		CNodeCache(INT iCacheSize);
		~CNodeCache();

			void			  AddToCache				(CNode *pNode);
			CNode			  *RemoveFromCache			();
	private:
		INT										m_iCacheSize;
		INT										m_iUsed;
		CNode									*m_pHead;
	};

protected:
	CNode										*m_pFirst;
	CNode										*m_pLast;
	LONG										m_Count;

private:
	CNodeCache									m_Cache;

	D_CBaseList	(const D_CBaseList &refList);
	D_CBaseList &operator=(const D_CBaseList &refList);

public:
	D_CBaseList(TCHAR *pName, INT iItems);
	D_CBaseList(TCHAR *pName);
#ifdef UNICODE
	D_CBaseList(CHAR *pName, INT iItems);
	D_CBaseList(CHAR *pName);
#endif
	~D_CBaseList();

			void			  RemoveAll					();
			D_POSITION		  GetHeadPositionI			() const;
			D_POSITION		  GetTailPositionI			() const;
			int				  GetCountI					() const;

protected:
			void			  *GetNextI					(D_POSITION& rp) const;
			void			  *GetI						(D_POSITION p) const;
			D_POSITION		  AddBeforeI				(D_POSITION p, void * pObj);
			D_POSITION		  AddHeadI					(void * pObj);
			D_POSITION		  AddAfterI					(D_POSITION p, void * pObj);
			D_POSITION		  FindI						( void * pObj) const;
			void			  *RemoveHeadI				();
			void			  *RemoveTailI				();
			void			  *RemoveI					(D_POSITION p);
			D_POSITION		  AddTailI					(void * pObj);

public:
			D_POSITION		  Next						(D_POSITION pos) const ;
			D_POSITION		  Prev						(D_POSITION pos) const ;
			BOOL			  AddTail					(D_CBaseList *pList);
			BOOL			  AddHead					(D_CBaseList *pList);
			BOOL			  AddAfter					(D_POSITION p, D_CBaseList *pList);
			BOOL			  AddBefore					(D_POSITION p, D_CBaseList *pList);
			BOOL			  MoveToTail				(D_POSITION pos, D_CBaseList *pList);
			BOOL			  MoveToHead				(D_POSITION pos, D_CBaseList *pList);
			void			  Reverse					();
};


template < class OBJECT > class D_CGenericList : public D_CBaseList
{
public:
	D_CGenericList(TCHAR *pName, INT iItems, BOOL bLock = TRUE, BOOL bAlert = FALSE) : D_CBaseList(pName, iItems) {
		UNREFERENCED_PARAMETER(bAlert);
		UNREFERENCED_PARAMETER(bLock);
	};
	D_CGenericList(TCHAR *pName) : D_CBaseList(pName) {};

			D_POSITION		  GetHeadPosition			() const { return (D_POSITION)m_pFirst; }
			D_POSITION		  GetTailPosition			() const { return (D_POSITION)m_pLast; }
			int				  GetCount					() const { return m_Count; }
			OBJECT			  *GetNext					( D_POSITION& rp) const { return (OBJECT *) GetNextI(rp); }
			OBJECT			  *Get						( D_POSITION p) const { return (OBJECT *) GetI(p); }
			OBJECT			  *GetHead					() const  { return Get(GetHeadPosition()); }
			OBJECT			  *RemoveHead				() { return (OBJECT *) RemoveHeadI(); }
			OBJECT			  *RemoveTail				() { return (OBJECT *) RemoveTailI(); }
			OBJECT			  *Remove					( D_POSITION p) { return (OBJECT *) RemoveI(p); }
			D_POSITION		  AddBefore					( D_POSITION p, OBJECT * pObj) { return AddBeforeI(p, pObj); }
			D_POSITION		  AddAfter					( D_POSITION p, OBJECT * pObj)  { return AddAfterI(p, pObj); }
			D_POSITION		  AddHead					( OBJECT * pObj) { return AddHeadI(pObj); }
			D_POSITION		  AddTail					( OBJECT * pObj)  { return AddTailI(pObj); }
			BOOL			  AddTail					( D_CGenericList<OBJECT> *pList) { return D_CBaseList::AddTail((D_CBaseList *) pList); }
			BOOL			  AddHead					( D_CGenericList<OBJECT> *pList) { return D_CBaseList::AddHead((D_CBaseList *) pList); }
			BOOL			  AddAfter					( D_POSITION p, D_CGenericList<OBJECT> *pList) { return D_CBaseList::AddAfter(p, (D_CBaseList *) pList); };
			BOOL			  AddBefore					( D_POSITION p, D_CGenericList<OBJECT> *pList) { return D_CBaseList::AddBefore(p, (D_CBaseList *) pList); };
			D_POSITION		  Find						( OBJECT * pObj) const { return FindI(pObj); }
} ;


class D_CBaseObject
{
private:
	D_CBaseObject( const D_CBaseObject& objectSrc ) ;
	void operator = ( const D_CBaseObject& objectSrc ) ;

private:
	static LONG m_cObjects;

protected:
#ifdef DEBUG
	DWORD m_dwCookie;
#endif

public:
	D_CBaseObject(const TCHAR *pName);
#ifdef UNICODE
	D_CBaseObject(const char *pName);
#endif
	~D_CBaseObject();

	static LONG ObjectsActive(){ return m_cObjects; };
};



class 
#ifndef DX_GCC_COMPILE
__declspec( novtable )
#endif
 D_CUnknown : public D_INonDelegatingUnknown, public D_CBaseObject
{
private:
	const IUnknown * m_pUnknown ;

protected:
	volatile LONG m_cRef;

public:

	D_CUnknown(const TCHAR *pName, IUnknown * pUnk);
	virtual ~D_CUnknown() {};

	D_CUnknown(TCHAR *pName, IUnknown * pUnk,HRESULT *phr);
#ifdef UNICODE
	D_CUnknown(const char *pName, IUnknown * pUnk);
	D_CUnknown(char *pName, IUnknown * pUnk,HRESULT *phr);
#endif

	IUnknown * GetOwner() const { return ( IUnknown * )m_pUnknown; };

	HRESULT	__stdcall NonDelegatingQueryInterface( REFIID, void ** ) ;
	ULONG	__stdcall NonDelegatingAddRef() ;
	ULONG	__stdcall NonDelegatingRelease() ;
} ;




class D_CCritSec
{
	D_CCritSec(const D_CCritSec &refCritSec);
	D_CCritSec &operator=(const D_CCritSec &refCritSec);

	CRITICAL_SECTION m_CritSec;

#ifdef DEBUG
public:
	DWORD   m_currentOwner;
	DWORD   m_lockCount;
	BOOL    m_fTrace;        // Trace this one
public:
	D_CCritSec();
	~D_CCritSec();
	void Lock();
	void Unlock();
#else
public:
	D_CCritSec() { InitializeCriticalSection( &m_CritSec ) ; } ;
	~D_CCritSec(){ DeleteCriticalSection( &m_CritSec ) ; } ;
	void Lock()  { EnterCriticalSection( &m_CritSec ) ; } ;
	void Unlock(){ LeaveCriticalSection( &m_CritSec ) ; } ;
#endif
};

class D_CAutoLock
{
	D_CAutoLock(const D_CAutoLock &refAutoLock);
	D_CAutoLock &operator=(const D_CAutoLock &refAutoLock);

protected:
	D_CCritSec * m_pLock;

public:
	D_CAutoLock(D_CCritSec * plock)
	{
		m_pLock = plock;
		m_pLock->Lock();
	};

	~D_CAutoLock() {
		m_pLock->Unlock();
	};
};



const LONGLONG MILLISECONDS = (1000);            // 10 ^ 3
const LONGLONG NANOSECONDS = (1000000000);       // 10 ^ 9
const LONGLONG UNITS = (NANOSECONDS / 100);      // 10 ^ 7

#define MILLISECONDS_TO_100NS_UNITS(lMs) Int32x32To64((lMs), (UNITS / MILLISECONDS))

class D_CRefTime
{
public:
	D_REFERENCE_TIME							m_time;

	inline D_CRefTime(){ m_time = 0; };
	inline D_CRefTime(LONG msecs){ m_time = MILLISECONDS_TO_100NS_UNITS(msecs); };
	inline D_CRefTime(D_REFERENCE_TIME rt){ m_time = rt; };
	inline operator D_REFERENCE_TIME() const { return m_time; };
	inline D_CRefTime& operator=(const D_CRefTime& rt){ m_time = rt.m_time; return *this; };
	inline D_CRefTime& operator=(const LONGLONG ll){ m_time = ll; return *this; };
	inline D_CRefTime& operator+=(const D_CRefTime& rt){ return (*this = *this + rt); };
	inline D_CRefTime& operator-=(const D_CRefTime& rt){ return (*this = *this - rt); };
	inline LONG Millisecs(void){ return (LONG)(m_time / (UNITS / MILLISECONDS)); };
	inline LONGLONG GetUnits(void){ return m_time; };
};

class D_CMediaType : public D_AM_MEDIA_TYPE
{
public:
	~D_CMediaType();
	D_CMediaType();
	D_CMediaType(const GUID * majortype);
	D_CMediaType(const D_AM_MEDIA_TYPE&, HRESULT* phr = NULL);
	D_CMediaType(const D_CMediaType&, HRESULT* phr = NULL);

	D_CMediaType& operator=(const D_CMediaType&);
	D_CMediaType& operator=(const D_AM_MEDIA_TYPE&);
	BOOL operator == (const D_CMediaType&) const;
	BOOL operator != (const D_CMediaType&) const;
			HRESULT			  Set						(const D_CMediaType& rt);
			HRESULT			  Set						(const D_AM_MEDIA_TYPE& rt);
			BOOL			  IsValid					() const;
			const GUID		  *Type						() const { return &majortype;} ;
			void			  SetType					(const GUID *);
			const GUID		  *Subtype					() const { return &subtype;} ;
			void			  SetSubtype				(const GUID *);
			BOOL			  IsFixedSize				() const {return bFixedSizeSamples; };
			BOOL			  IsTemporalCompressed		() const {return bTemporalCompression; };
			ULONG			  GetSampleSize				() const;
			void			  SetSampleSize				(ULONG sz);
			void			  SetVariableSize			();
			void			  SetTemporalCompression	(BOOL bCompressed);
			BYTE			  *Format					() const {return pbFormat; };
			ULONG			  FormatLength				() const { return cbFormat; };
			void			  SetFormatType				(const GUID *);
			const GUID		  *FormatType				() const {return &formattype; };
			BOOL			  SetFormat					(BYTE *pFormat, ULONG length);
			void			  ResetFormatBuffer			();
			BYTE			  *AllocFormatBuffer		(ULONG length);
			BYTE			  *ReallocFormatBuffer		(ULONG length);
			void			  InitMediaType				();
			BOOL			  MatchesPartial			(const D_CMediaType* ppartial) const;
			BOOL			  IsPartiallySpecified		(void) const;
};

class 
#ifndef DX_GCC_COMPILE
__declspec( novtable )
#endif
 D_CBaseFilter : public D_CUnknown, public D_IBaseFilter, public D_IAMovieSetup
{
friend class D_CBasePin;

protected:
	D_FILTER_STATE								m_State;
	D_IReferenceClock							*m_pClock;
	D_CRefTime									m_tStart;
	CLSID										m_clsid;
	D_CCritSec									*m_pLock;

	WCHAR										*m_pName;
	D_IFilterGraph								*m_pGraph;
	D_IMediaEventSink							*m_pSink;
	LONG										m_PinVersion;

public:

	D_CBaseFilter( const TCHAR *pName, IUnknown * pUnk, D_CCritSec  *pLock, REFCLSID   clsid);
	D_CBaseFilter( const TCHAR *pName, IUnknown * pUnk, D_CCritSec  *pLock, REFCLSID   clsid, HRESULT   *phr);
#ifdef UNICODE
	D_CBaseFilter( const CHAR *pName, IUnknown * pUnk, D_CCritSec  *pLock, REFCLSID   clsid);
	D_CBaseFilter( CHAR       *pName, IUnknown * pUnk, D_CCritSec  *pLock, REFCLSID   clsid, HRESULT   *phr);
#endif
	virtual ~D_CBaseFilter();
			HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv ){ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
			ULONG	__stdcall AddRef					() { return GetOwner()->AddRef();  };
			ULONG	__stdcall Release					(){ return GetOwner()->Release(); };

			HRESULT	__stdcall NonDelegatingQueryInterface( REFIID riid, void ** ppv ) ;
#ifdef DEBUG
			ULONG	__stdcall NonDelegatingRelease		();
#endif

	// --- IPersist method ---
			HRESULT	__stdcall GetClassID				(CLSID *pClsID);

	// --- IMediaFilter methods ---
			HRESULT	__stdcall GetState					( DWORD dwMSecs, D_FILTER_STATE *State ) ;
			HRESULT	__stdcall SetSyncSource				( D_IReferenceClock *pClock ) ;
			HRESULT	__stdcall GetSyncSource				( D_IReferenceClock **pClock ) ;
			HRESULT	__stdcall Stop						();
			HRESULT	__stdcall Pause						();
			HRESULT	__stdcall Run						( D_REFERENCE_TIME tStart ) ;

	// --- helper methods ---
	virtual HRESULT			  StreamTime				(D_CRefTime& rtStream);
			BOOL			  IsActive					(){ D_CAutoLock cObjectLock(m_pLock); return ((m_State == D_State_Paused) || (m_State == D_State_Running)); };
			BOOL			  IsStopped					(){ return (m_State == D_State_Stopped); };

	// --- IBaseFilter methods ---
			HRESULT	__stdcall EnumPins					( D_IEnumPins ** ppEnum);
			HRESULT	__stdcall FindPin					( LPCWSTR Id, D_IPin ** ppPin );
			HRESULT	__stdcall QueryFilterInfo			( D_FILTER_INFO * pInfo);
			HRESULT	__stdcall JoinFilterGraph			( D_IFilterGraph * pGraph, LPCWSTR pName);
			HRESULT	__stdcall QueryVendorInfo			( LPWSTR* pVendorInfo );

	// --- helper methods ---
			HRESULT			  NotifyEvent				( long EventCode, LONG_PTR EventParam1, LONG_PTR EventParam2);
			D_IFilterGraph	  *GetFilterGraph			() { return m_pGraph; }
			HRESULT			  ReconnectPin				( D_IPin *pPin, D_AM_MEDIA_TYPE const *pmt ) ;
	virtual LONG			  GetPinVersion				();
			void			  IncrementPinVersion		();
	virtual int				  GetPinCount				() = 0 ;
	virtual class D_CBasePin  *GetPin					(int n) = 0 ;

	// --- IAMovieSetup methods ---
			HRESULT	__stdcall Register					();
			HRESULT	__stdcall Unregister				();

	// --- setup helper methods ---
	virtual D_LPAMOVIESETUP_FILTER GetSetupData			(){ return NULL; }
};

class
#ifndef DX_GCC_COMPILE
 __declspec( novtable )
#endif
  D_CBasePin : public D_CUnknown, public D_IPin, public D_IQualityControl
{
protected:
	WCHAR										*m_pName;
	D_IPin										*m_Connected;
	D_PIN_DIRECTION								m_dir;
	D_CCritSec									*m_pLock;
	bool										m_bRunTimeError;
	bool										m_bCanReconnectWhenActive;
	bool										m_bTryMyTypesFirst;
	class D_CBaseFilter							*m_pFilter;
	D_IQualityControl							*m_pQSink;
	LONG										m_TypeVersion;
	D_CMediaType								m_mt;
	D_CRefTime									m_tStart;
	D_CRefTime									m_tStop;
	double										m_dRate;
#ifdef DEBUG
	LONG										m_cRef;
#endif

	// displays pin connection information
#ifdef DEBUG
			void DisplayPinInfo							( D_IPin *pReceivePin);
			void DisplayTypeInfo						( D_IPin *pPin, const D_CMediaType *pmt);
#else
			void DisplayPinInfo							( D_IPin * /*pReceivePin*/) {};
			void DisplayTypeInfo						( D_IPin * /*pPin*/, const D_CMediaType * /*pmt*/) {};
#endif

			HRESULT AttemptConnection					( D_IPin* pReceivePin, const D_CMediaType* pmt );
			HRESULT TryMediaTypes						( D_IPin *pReceivePin, const D_CMediaType *pmt, D_IEnumMediaTypes *pEnum) ;
			HRESULT AgreeMediaType						( D_IPin *pReceivePin, const D_CMediaType *pmt);

public:
//		D_CBasePin( TCHAR *pObjectName, D_CBaseFilter *pFilter, D_CCritSec *pLock, HRESULT *phr, LPCWSTR pName, D_PIN_DIRECTION dir ) ;
//#ifdef UNICODE
	D_CBasePin( CHAR *pObjectName, D_CBaseFilter *pFilter, D_CCritSec *pLock, HRESULT *phr, LPCWSTR pName, D_PIN_DIRECTION dir ) ;
//#endif
	virtual ~D_CBasePin();
			HRESULT __stdcall QueryInterface			( REFIID riid, void **ppv ){ return GetOwner()->QueryInterface(riid,ppv); };
			ULONG   __stdcall AddRef					(){ return GetOwner()->AddRef(); }; 
			ULONG   __stdcall Release					(){ return GetOwner()->Release(); };

			HRESULT __stdcall NonDelegatingQueryInterface( REFIID riid, void ** ppv);
			ULONG   __stdcall NonDelegatingRelease		();
			ULONG   __stdcall NonDelegatingAddRef		();

	// --- IPin methods ---
			HRESULT __stdcall Connect					( D_IPin * pReceivePin, const D_AM_MEDIA_TYPE *pmt );
			HRESULT __stdcall ReceiveConnection			( D_IPin * pConnector, const D_AM_MEDIA_TYPE *pmt );
			HRESULT __stdcall Disconnect				();
			HRESULT __stdcall ConnectedTo				( D_IPin **pPin);
			HRESULT __stdcall ConnectionMediaType		( D_AM_MEDIA_TYPE *pmt);
			HRESULT __stdcall QueryPinInfo				( D_PIN_INFO * pInfo );
			HRESULT __stdcall QueryDirection			( D_PIN_DIRECTION * pPinDir );
			HRESULT __stdcall QueryId					( LPWSTR * Id );
			HRESULT __stdcall QueryAccept				( const D_AM_MEDIA_TYPE *pmt );
			HRESULT __stdcall EnumMediaTypes			( D_IEnumMediaTypes **ppEnum );
			HRESULT __stdcall QueryInternalConnections	( D_IPin* * /*apPin*/, ULONG * /*nPin*/ ){ return E_NOTIMPL; }
			HRESULT __stdcall EndOfStream				( void);
			HRESULT __stdcall NewSegment				( D_REFERENCE_TIME tStart, D_REFERENCE_TIME tStop, double dRate ) ;

	// IQualityControl methods
			HRESULT __stdcall Notify					( D_IBaseFilter * pSender, D_Quality q  ) ;
			HRESULT __stdcall SetSink					( D_IQualityControl * piqc ) ;

	// --- helper methods ---
			BOOL			  IsConnected				( void) {return (m_Connected != NULL); };
			D_IPin			  *GetConnected				() { return m_Connected; };
			BOOL			  IsStopped					(){ return (m_pFilter->m_State == D_State_Stopped ) ; };
	virtual LONG			  GetMediaTypeVersion		();
			void			  IncrementTypeVersion		();
	virtual HRESULT			  Active					( void);
	virtual HRESULT			  Inactive					( void);
	virtual HRESULT			  Run						( D_REFERENCE_TIME tStart);
	virtual HRESULT			  CheckMediaType			( const D_CMediaType *) = 0 ;
	virtual HRESULT			  SetMediaType				( const D_CMediaType *);
	virtual HRESULT			  CheckConnect				( D_IPin *);
	virtual HRESULT			  BreakConnect				();
	virtual HRESULT			  CompleteConnect			( D_IPin *pReceivePin);
	virtual HRESULT			  GetMediaType				( int iPosition,D_CMediaType *pMediaType);
			D_REFERENCE_TIME  CurrentStopTime			() { return m_tStop; }
			D_REFERENCE_TIME  CurrentStartTime			() { return m_tStart; }
			double			  CurrentRate				() { return m_dRate; }
			LPWSTR			  Name						() { return m_pName; };
			void			  SetReconnectWhenActive	( bool bCanReconnect){ m_bCanReconnectWhenActive = bCanReconnect; }
			bool			  CanReconnectWhenActive	(){ return m_bCanReconnectWhenActive; }

protected:
	virtual HRESULT __stdcall DisconnectInternal		();
};

class D_CEnumPins : public D_IEnumPins
{
	typedef D_CGenericList<D_CBasePin> CPinList;
	int											m_Position;
	int											m_PinCount;
	class D_CBaseFilter							*m_pFilter;
	LONG										m_Version;
	LONG										m_cRef;
	CPinList									m_PinCache;

#ifdef DEBUG
	DWORD m_dwCookie;
#endif
			BOOL			  AreWeOutOfSync			() { return (m_pFilter->GetPinVersion() == m_Version ? FALSE : TRUE); };
			HRESULT __stdcall Refresh					();

public:
	D_CEnumPins( class D_CBaseFilter *pFilter, D_CEnumPins *pEnumPins);
	virtual ~D_CEnumPins();

	// IUnknown
			HRESULT __stdcall QueryInterface			(REFIID riid, void **ppv);
			ULONG   __stdcall AddRef					();
			ULONG   __stdcall Release					();

	// D_IEnumPins
			HRESULT __stdcall Next						( ULONG cPins, D_IPin ** ppPins, ULONG * pcFetched ) ;
			HRESULT __stdcall Skip						( ULONG cPins ) ;
			HRESULT __stdcall Reset						() ;
			HRESULT __stdcall Clone						( D_IEnumPins **ppEnum ) ;
};

class
#ifndef DX_GCC_COMPILE
__declspec( novtable )
#endif
D_CBaseInputPin : public D_CBasePin,  public D_IMemInputPin
{
protected:
	D_IMemAllocator								*m_pAllocator;
	BYTE										m_bReadOnly;
	BYTE										m_bFlushing;
	D_AM_SAMPLE2_PROPERTIES						m_SampleProps;

public:
//		D_CBaseInputPin( TCHAR *pObjectName, D_CBaseFilter *pFilter, D_CCritSec *pLock,	HRESULT *phr, LPCWSTR pName);
//#ifdef UNICODE
	D_CBaseInputPin( CHAR *pObjectName, D_CBaseFilter *pFilter, D_CCritSec *pLock, HRESULT *phr, LPCWSTR pName);
//#endif
	virtual ~D_CBaseInputPin();
			HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv ){ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
			ULONG	__stdcall AddRef					() { return GetOwner()->AddRef();  };
			ULONG	__stdcall Release					(){ return GetOwner()->Release(); };

			HRESULT	__stdcall NonDelegatingQueryInterface(REFIID riid, void **ppv);
			HRESULT	__stdcall GetAllocator				( D_IMemAllocator ** ppAllocator);
			HRESULT	__stdcall NotifyAllocator			( D_IMemAllocator * pAllocator, BOOL bReadOnly);
			HRESULT	__stdcall Receive					( D_IMediaSample *pSample);
			HRESULT	__stdcall ReceiveMultiple			( D_IMediaSample **pSamples, long nSamples, long *nSamplesProcessed);
			HRESULT	__stdcall ReceiveCanBlock			();
			HRESULT	__stdcall BeginFlush				( void);
			HRESULT	__stdcall EndFlush					( void);
			HRESULT	__stdcall GetAllocatorRequirements	( D_ALLOCATOR_PROPERTIES *pProps);
			HRESULT			  BreakConnect				();
			BOOL			  IsReadOnly				(){ return m_bReadOnly; };
			BOOL			  IsFlushing				(){ return m_bFlushing; };
	virtual HRESULT			  CheckStreaming			();
			HRESULT			  PassNotify				( D_Quality& q);
			HRESULT	__stdcall Notify					( D_IBaseFilter * pSender, D_Quality q);
	virtual HRESULT			  Inactive					( void);
	D_AM_SAMPLE2_PROPERTIES * SampleProps				() { /*ASSERT(m_SampleProps.cbData != 0);*/ return &m_SampleProps; }
};

class D_CRendererInputPin : public D_CBaseInputPin
{
protected:
	class D_CBaseRenderer						*m_pRenderer;

public:
	D_CRendererInputPin( D_CBaseRenderer *pRenderer, HRESULT *phr, LPCWSTR Name );

	// Overriden from the base pin classes
			HRESULT			  BreakConnect				() ;
			HRESULT			  CompleteConnect			( D_IPin *pReceivePin ) ;
			HRESULT			  SetMediaType				( const D_CMediaType *pmt ) ;
			HRESULT			  CheckMediaType			( const D_CMediaType *pmt ) ;
			HRESULT			  Active					() ;
			HRESULT			  Inactive					() ;

	// Add rendering behaviour to interface functions
			HRESULT	__stdcall QueryId(LPWSTR *Id);
			HRESULT	__stdcall EndOfStream();
			HRESULT	__stdcall BeginFlush();
			HRESULT	__stdcall EndFlush();
			HRESULT	__stdcall Receive(D_IMediaSample *pMediaSample);

	// Helper
	D_IMemAllocator inline		*Allocator				() const { return m_pAllocator; }
};

class D_CBaseDispatch
{
	ITypeInfo									*m_pti;

public:

	D_CBaseDispatch() : m_pti(NULL) {}
	~D_CBaseDispatch();

	/* IDispatch methods */
			HRESULT	__stdcall GetTypeInfoCount			( UINT * pctinfo ) ;
			HRESULT	__stdcall GetTypeInfo				( REFIID riid, UINT itinfo, LCID lcid, ITypeInfo ** pptinfo ) ;
			HRESULT	__stdcall GetIDsOfNames				( REFIID riid, OLECHAR  ** rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid ) ;
};

class
#ifndef DX_GCC_COMPILE
__declspec( novtable )
#endif
D_CMediaPosition : public D_IMediaPosition, public D_CUnknown
{
	D_CBaseDispatch								m_basedisp;

public:
	D_CMediaPosition( const TCHAR *, IUnknown *);
	D_CMediaPosition( const TCHAR *, IUnknown *, HRESULT *phr);

			HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv ){ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
			ULONG	__stdcall AddRef					() { return GetOwner()->AddRef();  };
			ULONG	__stdcall Release					(){ return GetOwner()->Release(); };

	// override this to publicise our interfaces
			HRESULT	__stdcall NonDelegatingQueryInterface(REFIID riid, void **ppv);

	/* IDispatch methods */
			HRESULT	__stdcall GetTypeInfoCount			( UINT * pctinfo);
			HRESULT	__stdcall GetTypeInfo				( UINT itinfo, LCID lcid, ITypeInfo ** pptinfo);
			HRESULT	__stdcall GetIDsOfNames				( REFIID riid, OLECHAR  ** rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid);
			HRESULT	__stdcall Invoke					( DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr );
};

class D_CPosPassThru : public D_IMediaSeeking, public D_CMediaPosition
{
	D_IPin										*m_pPin;

			HRESULT			  GetPeer					( D_IMediaPosition **ppMP ) ;
			HRESULT			  GetPeerSeeking			( D_IMediaSeeking **ppMS ) ;
public:
	D_CPosPassThru(const TCHAR *, IUnknown *, HRESULT*, D_IPin *);

			HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv ){ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
			ULONG	__stdcall AddRef					() { return GetOwner()->AddRef();  };
			ULONG	__stdcall Release					(){ return GetOwner()->Release(); };
			HRESULT			  ForceRefresh				(){ return S_OK; };

	// override to return an accurate current position
	virtual HRESULT			  GetMediaTime				( LONGLONG * /*pStartTime*/,LONGLONG * /*pEndTime*/) { return E_FAIL; }
			HRESULT	__stdcall NonDelegatingQueryInterface( REFIID riid,void **ppv);

	// IMediaSeeking methods
			HRESULT	__stdcall GetCapabilities			( DWORD * pCapabilities );
			HRESULT	__stdcall CheckCapabilities			( DWORD * pCapabilities );
			HRESULT	__stdcall SetTimeFormat				( const GUID * pFormat);
			HRESULT	__stdcall GetTimeFormat				( GUID *pFormat);
			HRESULT	__stdcall IsUsingTimeFormat			( const GUID * pFormat);
			HRESULT	__stdcall IsFormatSupported			( const GUID * pFormat);
			HRESULT	__stdcall QueryPreferredFormat		( GUID *pFormat);
			HRESULT	__stdcall ConvertTimeFormat			( LONGLONG * pTarget, const GUID * pTargetFormat, LONGLONG    Source, const GUID * pSourceFormat ); 
			HRESULT	__stdcall SetPositions				( LONGLONG * pCurrent, DWORD CurrentFlags, LONGLONG * pStop, DWORD StopFlags );
			HRESULT	__stdcall GetPositions				( LONGLONG * pCurrent, LONGLONG * pStop );
			HRESULT	__stdcall GetCurrentPosition		( LONGLONG * pCurrent );
			HRESULT	__stdcall GetStopPosition			( LONGLONG * pStop );
			HRESULT	__stdcall SetRate					( double dRate);
			HRESULT	__stdcall GetRate					( double * pdRate);
			HRESULT	__stdcall GetDuration				( LONGLONG *pDuration);
			HRESULT	__stdcall GetAvailable				( LONGLONG *pEarliest, LONGLONG *pLatest );
			HRESULT	__stdcall GetPreroll				( LONGLONG *pllPreroll );

	// D_IMediaPosition properties
			HRESULT	__stdcall get_Duration				( D_REFTIME * plength);
			HRESULT	__stdcall put_CurrentPosition		( D_REFTIME llTime);
			HRESULT	__stdcall get_StopTime				( D_REFTIME * pllTime);
			HRESULT	__stdcall put_StopTime				( D_REFTIME llTime);
			HRESULT	__stdcall get_PrerollTime			( D_REFTIME * pllTime);
			HRESULT	__stdcall put_PrerollTime			( D_REFTIME llTime);
			HRESULT	__stdcall get_Rate					( double * pdRate);
			HRESULT	__stdcall put_Rate					( double dRate);
			HRESULT	__stdcall get_CurrentPosition		( D_REFTIME * pllTime);
			HRESULT	__stdcall CanSeekForward			( LONG *pCanSeekForward);
			HRESULT	__stdcall CanSeekBackward			( LONG *pCanSeekBackward ) ;

private:
			HRESULT			  GetSeekingLongLong		( HRESULT (__stdcall D_IMediaSeeking::*pMethod)( LONGLONG * ), LONGLONG * pll );
};

class D_CRendererPosPassThru : public D_CPosPassThru
{
	D_CCritSec									m_PositionLock;
	LONGLONG									m_StartMedia;
	LONGLONG									m_EndMedia;
	BOOL										m_bReset;

public:
	D_CRendererPosPassThru( const TCHAR *, IUnknown *, HRESULT*, D_IPin * ) ;
			HRESULT			  RegisterMediaTime			( D_IMediaSample *pMediaSample ) ;
			HRESULT			  RegisterMediaTime			( LONGLONG StartTime,LONGLONG EndTime ) ;
			HRESULT			  GetMediaTime				( LONGLONG *pStartTime,LONGLONG *pEndTime ) ;
			HRESULT			  ResetMediaTime			() ;
			HRESULT			  EOS						() ;
};

class D_CAMEvent
{
	D_CAMEvent(const D_CAMEvent &refEvent);
	D_CAMEvent &operator=(const D_CAMEvent &refEvent);
protected:
	HANDLE										m_hEvent;
public:
	D_CAMEvent(BOOL fManualReset = FALSE);
	~D_CAMEvent();

	// Cast to HANDLE - we don't support this as an lvalue
	operator HANDLE										() const { return m_hEvent; };
			void			  Set						(){ /*EXECUTE_ASSERT*/(SetEvent(m_hEvent));};
			BOOL			  Wait						( DWORD dwTimeout = INFINITE) { return (WaitForSingleObject(m_hEvent, dwTimeout) == WAIT_OBJECT_0); };
			void			  Reset						(){ ResetEvent(m_hEvent); };
			BOOL			  Check						(){ return Wait(0); };
};

class D_CBaseRenderer : public D_CBaseFilter
{
protected:
	friend class D_CRendererInputPin;

	friend void CALLBACK EndOfStreamTimer(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

	D_CRendererPosPassThru						*m_pPosition;
	D_CAMEvent									m_RenderEvent;
	D_CAMEvent									m_ThreadSignal;
	D_CAMEvent									m_evComplete;
	BOOL										m_bAbort;
	BOOL										m_bStreaming;
	DWORD_PTR									m_dwAdvise;
	D_IMediaSample								*m_pMediaSample;
	BOOL										m_bEOS;
	BOOL										m_bEOSDelivered;
	D_CRendererInputPin							*m_pInputPin;
	D_CCritSec									m_InterfaceLock;
	D_CCritSec									m_RendererLock;
	D_IQualityControl							*m_pQSink;
	BOOL										m_bRepaintStatus;
	volatile BOOL								m_bInReceive;
	D_REFERENCE_TIME							m_SignalTime;
	UINT										m_EndOfStreamTimer;
	D_CCritSec									m_ObjectCreationLock;

public:
	D_CBaseRenderer( REFCLSID RenderClass, TCHAR *pName, IUnknown * pUnk, HRESULT *phr ) ;
	virtual ~D_CBaseRenderer();

	virtual HRESULT			  GetMediaPositionInterface	( REFIID riid, void **ppv ) ;
			HRESULT	__stdcall NonDelegatingQueryInterface( REFIID, void ** ) ;
	virtual HRESULT			  SourceThreadCanWait		( BOOL bCanWait ) ;

#ifdef DEBUG
	// Debug only dump of the renderer state
			void			  DisplayRendererState		() ;
#endif
	virtual HRESULT			  WaitForRenderTime			() ;
	virtual HRESULT			  CompleteStateChange		( D_FILTER_STATE OldState ) ;

	// Return internal information about this filter
			BOOL			  IsEndOfStream				(){ return m_bEOS; };
			BOOL			  IsEndOfStreamDelivered	(){ return m_bEOSDelivered; };
			BOOL			  IsStreaming				(){ return m_bStreaming; };
			void			  SetAbortSignal			( BOOL bAbort) { m_bAbort = bAbort; };
	virtual void			  OnReceiveFirstSample		( D_IMediaSample * /*pMediaSample*/){ };
			D_CAMEvent		  *GetRenderEvent			(){ return &m_RenderEvent ; } ;

	// Permit access to the transition state
			void			  Ready						(){ m_evComplete.Set() ; } ;
			void			  NotReady					(){ m_evComplete.Reset() ; } ;
			BOOL			  CheckReady				(){ return m_evComplete.Check() ; } ;
	virtual int				  GetPinCount				() ;
	virtual D_CBasePin		  *GetPin					( int n ) ;
			D_FILTER_STATE	  GetRealState				() ;
			void			  SendRepaint				() ;
			void			  SendNotifyWindow			( D_IPin *pPin, HWND hwnd ) ;
			BOOL			  OnDisplayChange			() ;
			void			  SetRepaintStatus			( BOOL bRepaint ) ;

	// Override the filter and pin interface functions
			HRESULT	__stdcall Stop						() ;
			HRESULT	__stdcall Pause						() ;
			HRESULT	__stdcall Run						( D_REFERENCE_TIME StartTime ) ;
			HRESULT	__stdcall GetState					( DWORD dwMSecs, D_FILTER_STATE *State ) ;
			HRESULT	__stdcall FindPin					( LPCWSTR Id, D_IPin **ppPin ) ;

	// These are available for a quality management implementation
	virtual void			  OnRenderStart				( D_IMediaSample *pMediaSample ) ;
	virtual void			  OnRenderEnd				( D_IMediaSample *pMediaSample ) ;
	virtual HRESULT			  OnStartStreaming			(){ return NOERROR; } ;
	virtual HRESULT			  OnStopStreaming			(){ return NOERROR; } ;
	virtual void			  OnWaitStart				(){} ;
	virtual void			  OnWaitEnd					(){} ;
	virtual void			  PrepareRender				(){} ;

#ifdef PERF
	D_REFERENCE_TIME							m_trRenderStart ;
	int											m_idBaseStamp;
	int											m_idBaseRenderTime;
	int											m_idBaseAccuracy;
#endif

	// Quality management implementation for scheduling rendering
	virtual BOOL			  ScheduleSample			( D_IMediaSample *pMediaSample ) ;
	virtual HRESULT			  GetSampleTimes			( D_IMediaSample *pMediaSample, D_REFERENCE_TIME *pStartTime, D_REFERENCE_TIME *pEndTime ) ;
	virtual HRESULT			  ShouldDrawSampleNow		( D_IMediaSample *pMediaSample, D_REFERENCE_TIME *ptrStart, D_REFERENCE_TIME *ptrEnd ) ;

	// Lots of end of stream complexities
			void			  TimerCallback				() ;
			void			  ResetEndOfStreamTimer		() ;
			HRESULT			  NotifyEndOfStream			() ;
	virtual HRESULT			  SendEndOfStream			() ;
	virtual HRESULT			  ResetEndOfStream			() ;
	virtual HRESULT			  EndOfStream				() ;

	// Rendering is based around the clock
			void			  SignalTimerFired			() ;
	virtual HRESULT			  CancelNotification		() ;
	virtual HRESULT			  ClearPendingSample		() ;

	// Called when the filter changes state
	virtual HRESULT			  Active					() ;
	virtual HRESULT			  Inactive					() ;
	virtual HRESULT			  StartStreaming			() ;
	virtual HRESULT			  StopStreaming				() ;
	virtual HRESULT			  BeginFlush				() ;
	virtual HRESULT			  EndFlush					() ;

	// Deal with connections and type changes
	virtual HRESULT			  BreakConnect				() ;
	virtual HRESULT			  SetMediaType				( const D_CMediaType *pmt ) ;
	virtual HRESULT			  CompleteConnect			( D_IPin *pReceivePin ) ;

	// These look after the handling of data samples
	virtual HRESULT			  PrepareReceive			( D_IMediaSample *pMediaSample ) ;
	virtual HRESULT			  Receive					( D_IMediaSample *pMediaSample ) ;
	virtual BOOL			  HaveCurrentSample			() ;
	virtual D_IMediaSample	  *GetCurrentSample			() ;
	virtual HRESULT			  Render					( D_IMediaSample *pMediaSample ) ;

	// Derived classes MUST override these
	virtual HRESULT			  DoRenderSample			( D_IMediaSample *pMediaSample ) = 0 ;
	virtual HRESULT			  CheckMediaType			( const D_CMediaType * ) = 0 ;

	// Helper
			void			  WaitForReceiveToComplete();
};

class D_CBaseVideoRenderer : public D_CBaseRenderer, public D_IQualProp, public D_IQualityControl
{
protected:
	int											m_nNormal;
#ifdef PERF
	BOOL										m_bDrawLateFrames;
#endif
	BOOL										m_bSupplierHandlingQuality;
	int											m_trThrottle;
	int											m_trRenderAvg;
	int											m_trRenderLast;
	int											m_tRenderStart;
	int											m_trEarliness;
	int											m_trTarget;
	int											m_trWaitAvg;
	int											m_trFrameAvg;
	int											m_trDuration;

#ifdef PERF
	int											m_idTimeStamp;
	int											m_idEarliness;
	int											m_idTarget;
	int											m_idWaitReal;
	int											m_idWait;
	int											m_idFrameAccuracy;
	int											m_idRenderAvg;
	int											m_idSchLateTime;
	int											m_idQualityRate;
	int											m_idQualityTime;
	int											m_idDecision;
	int											m_idDuration;
	int											m_idThrottle;
#endif // PERF
	D_REFERENCE_TIME							m_trRememberStampForPerf;
#ifdef PERF
	D_REFERENCE_TIME							m_trRememberFrameForPerf;

	// debug...
	int											m_idFrameAvg;
	int											m_idWaitAvg;
#endif
	int											m_cFramesDropped;
	int											m_cFramesDrawn;
	LONGLONG									m_iTotAcc;
	LONGLONG									m_iSumSqAcc;
	D_REFERENCE_TIME							m_trLastDraw;
	LONGLONG									m_iSumSqFrameTime;
	LONGLONG									m_iSumFrameTime;
	int											m_trLate;
	int											m_trFrame;
	int											m_tStreamingStart;
#ifdef PERF
	LONGLONG									m_llTimeOffset;
#endif

public:
	D_CBaseVideoRenderer( REFCLSID RenderClass, TCHAR *pName, IUnknown * pUnk, HRESULT *phr ) ;
	virtual ~D_CBaseVideoRenderer();

	// IQualityControl methods - Notify allows audio-video throttling
	virtual HRESULT __stdcall SetSink					( D_IQualityControl * piqc ) ;
	virtual HRESULT __stdcall Notify					( D_IBaseFilter * pSelf, D_Quality q ) ;

	// These provide a full video quality management implementation
			void			  OnRenderStart				( D_IMediaSample *pMediaSample ) ;
			void			  OnRenderEnd				( D_IMediaSample *pMediaSample ) ;
			void			  OnWaitStart				();
			void			  OnWaitEnd					();
			HRESULT			  OnStartStreaming			();
			HRESULT			  OnStopStreaming			();
			void			  ThrottleWait				();

	// Handle the statistics gathering for our quality management
			void			  PreparePerformanceData	( int trLate, int trFrame ) ;
	virtual void			  RecordFrameLateness		( int trLate, int trFrame ) ;
	virtual void			  OnDirectRender			( D_IMediaSample *pMediaSample ) ;
	virtual HRESULT			  ResetStreamingTimes		();
	virtual BOOL			  ScheduleSample			( D_IMediaSample *pMediaSample ) ;
	virtual HRESULT			  ShouldDrawSampleNow		( D_IMediaSample *pMediaSample, D_REFERENCE_TIME *ptrStart, D_REFERENCE_TIME *ptrEnd ) ;
	virtual HRESULT			  SendQuality				( D_REFERENCE_TIME trLate, D_REFERENCE_TIME trRealStream ) ;
	virtual HRESULT __stdcall JoinFilterGraph			( D_IFilterGraph * pGraph, LPCWSTR pName ) ;
			HRESULT			  GetStdDev					( int nSamples, int *piResult, LONGLONG llSumSq, LONGLONG iTot ) ;

public:
	// IQualProp property page support
	virtual HRESULT __stdcall get_FramesDroppedInRenderer( int *cFramesDropped ) ;
	virtual HRESULT __stdcall get_FramesDrawn			( int *pcFramesDrawn ) ;
	virtual HRESULT __stdcall get_AvgFrameRate			( int *piAvgFrameRate ) ;
	virtual HRESULT __stdcall get_Jitter				( int *piJitter ) ;
	virtual HRESULT __stdcall get_AvgSyncOffset			( int *piAvg ) ;
	virtual HRESULT __stdcall get_DevSyncOffset			( int *piDev ) ;

	// Implement an IUnknown interface and expose IQualProp
	virtual HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv )	{ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
	virtual ULONG	__stdcall AddRef					()							{ return GetOwner()->AddRef();  } ;
	virtual ULONG	__stdcall Release					()							{ return GetOwner()->Release(); } ;
	virtual HRESULT __stdcall NonDelegatingQueryInterface( REFIID riid,VOID **ppv ) ;
};

class D_CEnumMediaTypes : public D_IEnumMediaTypes
{
	int											m_Position;
	D_CBasePin									*m_pPin;
	LONG										m_Version;
	LONG										m_cRef;
#ifdef DEBUG
	DWORD										m_dwCookie;
#endif

	BOOL AreWeOutOfSync(){ return (m_pPin->GetMediaTypeVersion() == m_Version ? FALSE : TRUE); };

public:
	D_CEnumMediaTypes( D_CBasePin *pPin, D_CEnumMediaTypes *pEnumMediaTypes ) ;
	virtual ~D_CEnumMediaTypes() ;

	// IUnknown
	virtual HRESULT __stdcall QueryInterface			( REFIID riid, void **ppv ) ;
	virtual ULONG   __stdcall AddRef					() ;
	virtual ULONG   __stdcall Release					() ;

	// IEnumMediaTypes
	virtual HRESULT __stdcall Next						( ULONG cMediaTypes, D_AM_MEDIA_TYPE ** ppMediaTypes, ULONG * pcFetched ) ;
	virtual HRESULT __stdcall Skip						( ULONG cMediaTypes ) ;
	virtual HRESULT __stdcall Reset						() ;
	virtual HRESULT __stdcall Clone						( D_IEnumMediaTypes **ppEnum) ;
};

class D_CMovieRender : public D_CBaseVideoRenderer
{
public:
	D_CMovieRender( IUnknown * pUnk, HRESULT *phr ) ;
	~D_CMovieRender();

	virtual ULONG  __stdcall Release					();

	virtual HRESULT CheckMediaType						( const D_CMediaType *pMediaType ) ;
	virtual HRESULT SetMediaType						( const D_CMediaType *pMediaType ) ;
	virtual HRESULT DoRenderSample						( D_IMediaSample *pMediaSample ) ;
	void AllocImageBuffer() ;

	int											SetMediaTypeFlag ;
	int											UseTempBaseImage ;
	struct tagBASEIMAGE							*TempBaseImage ;
	int											ImageType ;			// 0:RGB24   1:RGB32   2:YV12   3:NV12   4:YUY2   5:UYVY   6:YVYU   7:NV11
	void										*ImageBuffer ;
	BOOL										NewImageSet ;
	BOOL										ImageReverse ;
	DWORD										Width ;
	DWORD										Height ;
	DWORD										Pitch ;
} ;

#ifndef DX_NON_DSHOW_MP3

class D_ISeekingPassThru : public IUnknown
{
public:
	virtual HRESULT __stdcall Init						( BOOL bSupportRendering, D_IPin *pPin) = 0 ;
};

class D_CBaseOutputPin : public D_CBasePin
{

protected:

	D_IMemAllocator								*m_pAllocator ;
	D_IMemInputPin								*m_pInputPin ;

public:

	D_CBaseOutputPin( TCHAR *pObjectName, D_CBaseFilter *pFilter, D_CCritSec *pLock, HRESULT *phr, LPCWSTR pName);
#ifdef UNICODE
	D_CBaseOutputPin( CHAR  *pObjectName, D_CBaseFilter *pFilter, D_CCritSec *pLock, HRESULT *phr, LPCWSTR pName);
#endif
	virtual HRESULT			  CompleteConnect			( D_IPin *pReceivePin );
	virtual HRESULT			  DecideAllocator			( D_IMemInputPin * pPin, D_IMemAllocator ** pAlloc );
	virtual HRESULT			  DecideBufferSize			( D_IMemAllocator * pAlloc, D_ALLOCATOR_PROPERTIES * ppropInputRequest ) = 0 ;
	virtual HRESULT			  GetDeliveryBuffer			( D_IMediaSample ** ppSample, D_REFERENCE_TIME * pStartTime, D_REFERENCE_TIME * pEndTime, DWORD dwFlags);
	virtual HRESULT			  Deliver					( D_IMediaSample *);

	// override this to control the connection
	virtual HRESULT			  InitAllocator				( D_IMemAllocator **ppAlloc ) ;
			HRESULT			  CheckConnect				( D_IPin *pPin ) ;
			HRESULT			  BreakConnect				() ;

	// override to call Commit and Decommit
			HRESULT			  Active					() ;
			HRESULT			  Inactive					() ;

	STDMETHODIMP			  EndOfStream				() ;
	virtual HRESULT			  DeliverEndOfStream		() ;

	STDMETHODIMP			  BeginFlush				() ;
	STDMETHODIMP			  EndFlush					() ;
	virtual HRESULT			  DeliverBeginFlush			() ;
	virtual HRESULT			  DeliverEndFlush			() ;
	virtual HRESULT			  DeliverNewSegment			( D_REFERENCE_TIME tStart, D_REFERENCE_TIME tStop, double dRate ) ;
};

class D_CTransformInputPin : public D_CBaseInputPin
{
	friend class D_CTransformFilter;

protected:
	class D_CTransformFilter					*m_pTransformFilter ;

public:
//		D_CTransformInputPin( TCHAR *pObjectName, class D_CTransformFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName);
//	#ifdef UNICODE
	D_CTransformInputPin( char  *pObjectName, class D_CTransformFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName);
//	#endif

	STDMETHODIMP			  QueryId					( LPWSTR * Id ){ return D_AMGetWideString(L"In", Id); }

			HRESULT			  CheckConnect				( D_IPin *pPin );
			HRESULT			  BreakConnect				();
			HRESULT			  CompleteConnect			( D_IPin *pReceivePin );

			HRESULT			  CheckMediaType			( const D_CMediaType* mtIn ) ;
			HRESULT			  SetMediaType				( const D_CMediaType* mt ) ;

	// --- IMemInputPin -----
	STDMETHODIMP			  Receive					( D_IMediaSample * pSample ) ;
	STDMETHODIMP			  EndOfStream				() ;
	STDMETHODIMP			  BeginFlush				() ;
	STDMETHODIMP			  EndFlush					() ;
	STDMETHODIMP			  NewSegment				( D_REFERENCE_TIME tStart, D_REFERENCE_TIME tStop, double dRate ) ;

	virtual HRESULT			  CheckStreaming			() ;

	// Media type
public:
			D_CMediaType	 &CurrentMediaType			(){ return m_mt; };
};

class D_CTransformOutputPin : public D_CBaseOutputPin
{
	friend class D_CTransformFilter;

protected:
	class D_CTransformFilter					*m_pTransformFilter ;

public:
	IUnknown									*m_pPosition ;

//		D_CTransformOutputPin( TCHAR *pObjectName, class D_CTransformFilter *pTransformFilter, HRESULT *phr, LPCWSTR pName ) ;
//	#ifdef UNICODE
	D_CTransformOutputPin( CHAR  *pObjectName, class D_CTransformFilter *pTransformFilter, HRESULT * phr, LPCWSTR pName ) ;
//	#endif
	~D_CTransformOutputPin();

	STDMETHODIMP			  NonDelegatingQueryInterface( REFIID riid, void **ppv ) ;
	STDMETHODIMP			  QueryId					( LPWSTR * Id ){ return D_AMGetWideString( L"Out", Id ) ; }

			HRESULT			  CheckConnect				( D_IPin *pPin ) ;
			HRESULT			  BreakConnect				();
			HRESULT			  CompleteConnect			( D_IPin *pReceivePin ) ;

			HRESULT			  CheckMediaType			( const D_CMediaType* mtOut ) ;
			HRESULT			  SetMediaType				( const D_CMediaType *pmt ) ;
			HRESULT			  DecideBufferSize			( D_IMemAllocator * pAlloc, D_ALLOCATOR_PROPERTIES *pProp ) ;

			HRESULT			  GetMediaType				( int iPosition, D_CMediaType *pMediaType ) ;

	STDMETHODIMP			  Notify					( D_IBaseFilter * pSender, D_Quality q ) ;

public:
			D_CMediaType	 &CurrentMediaType			(){ return m_mt; } ;
};

class
#ifndef DX_GCC_COMPILE
__declspec(novtable)
#endif
D_CTransformFilter : public D_CBaseFilter
{
public:
	virtual int				  GetPinCount				() ;
	virtual D_CBasePin		 *GetPin					( int n ) ;
	STDMETHODIMP			  FindPin					( LPCWSTR Id, D_IPin **ppPin ) ;

	STDMETHODIMP			  Stop						() ;
	STDMETHODIMP			  Pause						() ;

//		D_CTransformFilter( TCHAR *, IUnknown *, REFCLSID clsid);
//	#ifdef UNICODE
	D_CTransformFilter( CHAR  *, IUnknown *, REFCLSID clsid);
//	#endif
	~D_CTransformFilter();

	virtual HRESULT			  Transform					( D_IMediaSample * pIn, D_IMediaSample *pOut ) ;

	virtual HRESULT			  CheckInputType			( const D_CMediaType* mtIn ) = 0 ;
	virtual HRESULT			  CheckTransform			( const D_CMediaType* mtIn, const D_CMediaType* mtOut ) = 0 ;
	virtual HRESULT			  DecideBufferSize			( D_IMemAllocator * pAllocator, D_ALLOCATOR_PROPERTIES *pprop ) = 0 ;

	virtual HRESULT			  GetMediaType				( int iPosition, D_CMediaType *pMediaType ) = 0 ;

	virtual HRESULT			  StartStreaming			() ;
	virtual HRESULT			  StopStreaming				() ;
	virtual HRESULT			  AlterQuality				( D_Quality q);
	virtual HRESULT			  SetMediaType				( D_PIN_DIRECTION direction, const D_CMediaType *pmt ) ;
	virtual HRESULT			  CheckConnect				( D_PIN_DIRECTION dir, D_IPin *pPin);
	virtual HRESULT			  BreakConnect				( D_PIN_DIRECTION dir ) ;
	virtual HRESULT			  CompleteConnect			( D_PIN_DIRECTION direction, D_IPin *pReceivePin ) ;
	virtual HRESULT			  Receive					( D_IMediaSample *pSample ) ;
			HRESULT			  InitializeOutputSample	( D_IMediaSample *pSample, D_IMediaSample **ppOutSample ) ;

	virtual HRESULT			  EndOfStream				() ;
	virtual HRESULT			  BeginFlush				() ;
	virtual HRESULT			  EndFlush					() ;
	virtual HRESULT			  NewSegment				( D_REFERENCE_TIME tStart, D_REFERENCE_TIME tStop, double dRate ) ;

#ifdef PERF
	virtual void			  RegisterPerfId			(){ m_idTransform = MSR_REGISTER( TEXT( "Transform" ) ) ; }
#endif // PERF

protected:

#ifdef PERF
	int											m_idTransform ;
#endif
	BOOL										m_bEOSDelivered ;
	BOOL										m_bSampleSkipped ;
	BOOL										m_bQualityChanged ;
	D_CCritSec									m_csFilter ;
	D_CCritSec									m_csReceive ;

	friend class D_CTransformInputPin ;
	friend class D_CTransformOutputPin ;
	D_CTransformInputPin						*m_pInput ;
	D_CTransformOutputPin						*m_pOutput ;
};

class D_CWavDestOutputPin : public D_CTransformOutputPin
{
public:
	D_CWavDestOutputPin( D_CTransformFilter *pFilter, HRESULT * phr ) ;

	STDMETHODIMP			  EnumMediaTypes			( D_IEnumMediaTypes **ppEnum ) ;
			HRESULT			  CheckMediaType			( const D_CMediaType* pmt ) ;
} ;

class D_CWavDestFilter : public D_CTransformFilter
{
public:
	virtual HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv )	{ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
	virtual ULONG	__stdcall AddRef					()							{ return GetOwner()->AddRef();  } ;
	virtual ULONG	__stdcall Release					()							{ return GetOwner()->Release(); } ;

	D_CWavDestFilter( IUnknown * pUnk, HRESULT *pHr) ;
	~D_CWavDestFilter();

	static	D_CUnknown * WINAPI CreateInstance			( IUnknown * punk, HRESULT *pHr ) ;
			HRESULT			  Transform					( D_IMediaSample *pIn, D_IMediaSample *pOut ) ;
			HRESULT			  Receive					( D_IMediaSample *pSample ) ;

			HRESULT			  CheckInputType			( const D_CMediaType* mtIn ) ;
			HRESULT			  CheckTransform			( const D_CMediaType *mtIn, const D_CMediaType *mtOut ) ;
			HRESULT			  GetMediaType				( int iPosition, D_CMediaType *pMediaType ) ;

			HRESULT			  DecideBufferSize			( D_IMemAllocator *pAlloc, D_ALLOCATOR_PROPERTIES *pProperties ) ;

			HRESULT			  StartStreaming			() ;
			HRESULT			  StopStreaming				() ;

			HRESULT			  CompleteConnect			( D_PIN_DIRECTION /*direction*/, D_IPin * /*pReceivePin*/ ){ return S_OK; }

private:
			HRESULT			  Copy						( D_IMediaSample *pSource, D_IMediaSample *pDest ) const ;
			HRESULT			  Transform					( D_IMediaSample *pMediaSample ) ;
			HRESULT			  Transform					( D_AM_MEDIA_TYPE *pType, const signed char ContrastLevel ) const ;

	ULONG										m_cbWavData;
	ULONG										m_cbHeader;
};

class D_CMediaSample : public D_IMediaSample2    // The interface we support
{
protected:
	friend class D_CBaseAllocator;

	enum
	{
		Sample_SyncPoint       = 0x01,
		Sample_Preroll         = 0x02,
		Sample_Discontinuity   = 0x04,
		Sample_TypeChanged     = 0x08,
		Sample_TimeValid       = 0x10,
		Sample_MediaTimeValid  = 0x20,
		Sample_TimeDiscontinuity = 0x40,
		Sample_StopValid       = 0x100,
		Sample_ValidFlags      = 0x1FF
	};

	DWORD										m_dwFlags;
	DWORD										m_dwTypeSpecificFlags;
	LPBYTE										m_pBuffer;
	LONG										m_lActual;
	LONG										m_cbBuffer;
	class D_CBaseAllocator						*m_pAllocator;
	D_CMediaSample								*m_pNext;
	D_REFERENCE_TIME							m_Start;
	D_REFERENCE_TIME							m_End;
	LONGLONG									m_MediaStart;
	LONG										m_MediaEnd;
	D_AM_MEDIA_TYPE								*m_pMediaType;
	DWORD										m_dwStreamId;
public:
	LONG										m_cRef;

public:
//		D_CMediaSample( const TCHAR *pName, class D_CBaseAllocator *pAllocator, HRESULT *phr, LPBYTE pBuffer = NULL, LONG length = 0);
//	#ifdef UNICODE
	D_CMediaSample( const CHAR *pName, class D_CBaseAllocator *pAllocator, HRESULT *phr, LPBYTE pBuffer = NULL, LONG length = 0);
//	#endif

	virtual ~D_CMediaSample();

	STDMETHODIMP			  QueryInterface			( REFIID riid, void **ppv );
	STDMETHODIMP_(ULONG)	  AddRef					();
	STDMETHODIMP_(ULONG)	  Release					();

	HRESULT					  SetPointer				( BYTE * ptr, LONG cBytes );
	STDMETHODIMP			  GetPointer				( BYTE ** ppBuffer ) ;
	STDMETHODIMP_(LONG)		  GetSize					();
	STDMETHODIMP			  GetTime					( D_REFERENCE_TIME * pTimeStart, D_REFERENCE_TIME * pTimeEnd );
	STDMETHODIMP			  SetTime					( D_REFERENCE_TIME * pTimeStart, D_REFERENCE_TIME * pTimeEnd );
	STDMETHODIMP			  IsSyncPoint				();
	STDMETHODIMP			  SetSyncPoint				( BOOL bIsSyncPoint );
	STDMETHODIMP			  IsPreroll					();
	STDMETHODIMP			  SetPreroll				( BOOL bIsPreroll );

	STDMETHODIMP_(LONG)		  GetActualDataLength		();
	STDMETHODIMP			  SetActualDataLength		( LONG lActual );
	STDMETHODIMP			  GetMediaType				( D_AM_MEDIA_TYPE **ppMediaType);
	STDMETHODIMP			  SetMediaType				( D_AM_MEDIA_TYPE *pMediaType);
	STDMETHODIMP			  IsDiscontinuity			();
	STDMETHODIMP			  SetDiscontinuity			( BOOL bDiscontinuity);
	STDMETHODIMP			  GetMediaTime				( LONGLONG * pTimeStart, LONGLONG * pTimeEnd );
	STDMETHODIMP			  SetMediaTime				( LONGLONG * pTimeStart, LONGLONG * pTimeEnd );
	STDMETHODIMP			  GetProperties				( DWORD cbProperties, BYTE * pbProperties );
	STDMETHODIMP			  SetProperties				( DWORD cbProperties, const BYTE * pbProperties );
};

class
#ifndef DX_GCC_COMPILE
__declspec(novtable)
#endif
D_CBaseAllocator : public D_CUnknown, public D_IMemAllocatorCallbackTemp, public D_CCritSec
{
	class D_CSampleList;
	friend class D_CSampleList;

	static	D_CMediaSample*	  &NextSample				( D_CMediaSample *pSample ){ return pSample->m_pNext; };

	class D_CSampleList
	{
	public:
		D_CSampleList() : m_List(NULL), m_nOnList(0) {};
			D_CMediaSample	  *Head						() const { return m_List; };
			D_CMediaSample	  *Next						( D_CMediaSample *pSample) const { return D_CBaseAllocator::NextSample(pSample); };
			int				  GetCount					() const { return m_nOnList; };
			void			  Add						( D_CMediaSample *pSample) ;
			D_CMediaSample	  *RemoveHead				() ;
			void			  Remove					( D_CMediaSample *pSample);

	public:
		D_CMediaSample							*m_List;
		int										m_nOnList;
	};
protected:

	D_CSampleList								m_lFree;
	HANDLE										m_hSem;
	long										m_lWaiting;
	long										m_lCount;
	long										m_lAllocated;
	long										m_lSize;
	long										m_lAlignment;
	long										m_lPrefix;
	BOOL										m_bChanged;
	BOOL										m_bCommitted;
	BOOL										m_bDecommitInProgress;
	D_IMemAllocatorNotifyCallbackTemp			*m_pNotify;
	BOOL										m_fEnableReleaseCallback;

	virtual void			  Free						() = 0 ;
	virtual HRESULT			  Alloc						() ;

public:
	D_CBaseAllocator( const TCHAR *, IUnknown *, HRESULT *, BOOL bEvent = TRUE, BOOL fEnableReleaseCallback = FALSE);
#ifdef UNICODE
	D_CBaseAllocator( const CHAR *, IUnknown *, HRESULT *, BOOL bEvent = TRUE, BOOL fEnableReleaseCallback = FALSE);
#endif
	virtual ~D_CBaseAllocator();

	virtual HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv )	{ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
	virtual ULONG	__stdcall AddRef					()							{ return GetOwner()->AddRef();  } ;
	virtual ULONG	__stdcall Release					()							{ return GetOwner()->Release(); } ;

	STDMETHODIMP			  NonDelegatingQueryInterface( REFIID riid, void **ppv );
	STDMETHODIMP			  SetProperties				( D_ALLOCATOR_PROPERTIES* pRequest, D_ALLOCATOR_PROPERTIES* pActual);
	STDMETHODIMP			  GetProperties				( D_ALLOCATOR_PROPERTIES* pProps);
	STDMETHODIMP			  Commit					();
	STDMETHODIMP			  Decommit					();
	STDMETHODIMP			  GetBuffer					( D_IMediaSample **ppBuffer, D_REFERENCE_TIME * pStartTime, D_REFERENCE_TIME * pEndTime, DWORD dwFlags);
	STDMETHODIMP			  ReleaseBuffer				( D_IMediaSample *pBuffer);
	STDMETHODIMP			  SetNotify					( D_IMemAllocatorNotifyCallbackTemp *pNotify);
	STDMETHODIMP			  GetFreeCount				( LONG *plBuffersFree);

	void					  NotifySample				();
	void					  SetWaiting				() { m_lWaiting++; };
};

class D_CMemAllocator : public D_CBaseAllocator
{
protected:
	LPBYTE										m_pBuffer;

			void			  Free						();
			void			  ReallyFree				();
			HRESULT			  Alloc						();

public:
	static D_CUnknown		 *CreateInstance			( IUnknown *, HRESULT * );
	STDMETHODIMP			  SetProperties				( D_ALLOCATOR_PROPERTIES* pRequest, D_ALLOCATOR_PROPERTIES* pActual);
//		D_CMemAllocator( const TCHAR *, IUnknown *, HRESULT *);
//#ifdef UNICODE
	D_CMemAllocator( const CHAR *, IUnknown *, HRESULT *);
//#endif
	~D_CMemAllocator();
};

class D_CAsyncRequest
{
	class D_CAsyncIo							*m_pIo;
	class D_CAsyncStream						*m_pStream;
	LONGLONG									m_llPos;
	BOOL										m_bAligned;
	LONG										m_lLength;
	BYTE										*m_pBuffer;
	LPVOID										m_pContext;
	DWORD										m_dwUser;
	HRESULT										m_hr;

public:
			HRESULT			  Request					( class D_CAsyncIo *pIo, class D_CAsyncStream *pStream, LONGLONG llPos, LONG lLength, BOOL bAligned, BYTE* pBuffer, LPVOID pContext, DWORD dwUser);
			HRESULT			  Complete					();
			HRESULT			  Cancel					(){ return S_OK; };
			LPVOID			  GetContext				(){ return m_pContext; };
			DWORD			  GetUser					(){ return m_dwUser; };
			HRESULT			  GetHResult				() { return m_hr;	};
			LONG			  GetActualLength			() { return m_lLength; };
			LONGLONG		  GetStart					() { return m_llPos; };
};

class D_CAsyncStream
{
public:
	virtual ~D_CAsyncStream() {};
	virtual HRESULT			  SetPointer( LONGLONG llPos ) = 0;
	virtual HRESULT			  Read( PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead ) = 0;

	virtual LONGLONG		  Size( LONGLONG *pSizeAvailable = NULL ) = 0;
	virtual DWORD			  Alignment() = 0 ;
	virtual void			  Lock() = 0;
	virtual void			  Unlock() = 0;
};

typedef D_CGenericList< D_CAsyncRequest > D_CRequestList;

class D_CAsyncIo
{
	D_CCritSec									m_csReader;
	D_CAsyncStream								*m_pStream;

	D_CCritSec									m_csLists;
	BOOL										m_bFlushing;

	D_CRequestList								m_listWork;
	D_CRequestList								m_listDone;

	D_CAMEvent									m_evWork;
	D_CAMEvent									m_evDone;

	LONG										m_cItemsOut;
	BOOL										m_bWaiting;
	D_CAMEvent									m_evAllDone;

	D_CAMEvent									m_evStop;
	HANDLE										m_hThread;

			LONGLONG		  Size						(){ /*ASSERT(m_pStream != NULL);*/ return m_pStream->Size(); };
			HRESULT			  StartThread				() ;
			HRESULT			  CloseThread				();
			D_CAsyncRequest	 *GetWorkItem				();
			D_CAsyncRequest	 *GetDoneItem				();
			HRESULT			  PutWorkItem				( D_CAsyncRequest* pRequest ) ;
			HRESULT			  PutDoneItem				( D_CAsyncRequest* pRequest ) ;
			void			  ProcessRequests			() ;

	static	DWORD	WINAPI	  InitialThreadProc			(LPVOID pv){ D_CAsyncIo * pThis = (D_CAsyncIo*) pv; return pThis->ThreadProc(); };

			DWORD			  ThreadProc				() ;

public:

	D_CAsyncIo( D_CAsyncStream *pStream );
	~D_CAsyncIo();

			HRESULT			  Open						( LPCTSTR pName);
			HRESULT			  AsyncActive				();
			HRESULT			  AsyncInactive				();
			HRESULT			  Request					( LONGLONG llPos, LONG lLength, BOOL bAligned, BYTE* pBuffer, LPVOID pContext, DWORD dwUser);
			HRESULT			  WaitForNext				( DWORD dwTimeout, LPVOID *ppContext, DWORD * pdwUser, LONG * pcbActual);
			HRESULT			  SyncReadAligned			( LONGLONG llPos, LONG lLength, BYTE* pBuffer, LONG* pcbActual, PVOID pvContext);
			HRESULT			  SyncRead					( LONGLONG llPos, LONG lLength, BYTE* pBuffer);
			HRESULT			  Length					( LONGLONG *pllTotal, LONGLONG* pllAvailable);
			HRESULT			  Alignment					( LONG* pl);
			HRESULT			  BeginFlush				();
			HRESULT			  EndFlush					();

			LONG			  Alignment					(){	return m_pStream->Alignment();};
			BOOL			  IsAligned					( LONG l)      {if ((l & (Alignment() -1)) == 0){return TRUE;} else {return FALSE;}};
			BOOL			  IsAligned					( LONGLONG ll) {return IsAligned( (LONG) (ll & 0xffffffff));};
			HANDLE			  StopEvent					() const { return m_evDone; }
};

class D_CMemStream : public D_CAsyncStream
{
public:
	static	D_CUnknown *WINAPI CreateInstance			( LPBYTE pbData, LONGLONG llLength, DWORD dwKBPerSec = INFINITE );
			void			   DeleteInstance			() ;

	D_CMemStream( LPBYTE pbData, LONGLONG llLength, DWORD dwKBPerSec = INFINITE ) ;

			HRESULT			  SetPointer				(LONGLONG llPos) ;
			HRESULT			  Read						(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead ) ;
			LONGLONG		  Size						(LONGLONG *pSizeAvailable) ;
			DWORD			  Alignment					() ;
			void			  Lock						() ;
			void			  Unlock					() ;
			LONGLONG		  GetPosition				() ;
			LONGLONG		  GetLength					() ;

private:
	D_CCritSec									m_csLock;
	const PBYTE									m_pbData;
	const LONGLONG								m_llLength;
	LONGLONG									m_llPosition;
	DWORD										m_dwKBPerSec;
	DWORD										m_dwTimeStart;
};

class D_IAsyncReader : public IUnknown
{
public:
    virtual HRESULT __stdcall RequestAllocator			( D_IMemAllocator *pPreferred, D_ALLOCATOR_PROPERTIES *pProps, D_IMemAllocator **ppActual) = 0;
    virtual HRESULT __stdcall Request					( D_IMediaSample *pSample, DWORD dwUser) = 0;
    virtual HRESULT __stdcall WaitForNext				( DWORD dwTimeout, D_IMediaSample **ppSample, DWORD *pdwUser) = 0;
    virtual HRESULT __stdcall SyncReadAligned			( D_IMediaSample *pSample) = 0;
    virtual HRESULT __stdcall SyncRead					( LONGLONG llPosition, LONG lLength, BYTE *pBuffer) = 0;
    virtual HRESULT __stdcall Length					( LONGLONG *pTotal, LONGLONG *pAvailable) = 0;
    virtual HRESULT __stdcall BeginFlush				() = 0;
    virtual HRESULT __stdcall EndFlush					() = 0;
};

class D_CAsyncOutputPin : public D_IAsyncReader, public D_CBasePin
{
protected:
	class D_CAsyncReader						*m_pReader;
	D_CAsyncIo									*m_pIo;
	BOOL										m_bQueriedForAsyncReader;

			HRESULT				  InitAllocator			( D_IMemAllocator **ppAlloc ) ;

public:
	D_CAsyncOutputPin( HRESULT * phr, class D_CAsyncReader *pReader, D_CAsyncIo *pIo, D_CCritSec * pLock);
	~D_CAsyncOutputPin();

	// --- CUnknown ---

	virtual HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv )	{ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
	virtual ULONG	__stdcall AddRef					()							{ return GetOwner()->AddRef();  } ;
	virtual ULONG	__stdcall Release					()							{ return GetOwner()->Release(); } ;
			STDMETHODIMP	  NonDelegatingQueryInterface( REFIID, void** );

	// --- IPin methods ---
			STDMETHODIMP	  Connect					( D_IPin * pReceivePin, const D_AM_MEDIA_TYPE *pmt );

	// --- CBasePin methods ---
			HRESULT			  GetMediaType				( int iPosition, D_CMediaType *pMediaType ) ;
			HRESULT			  CheckMediaType			( const D_CMediaType* pType ) ;
			HRESULT			  CheckConnect				( D_IPin *pPin ){ m_bQueriedForAsyncReader = FALSE; return D_CBasePin::CheckConnect(pPin); }
			HRESULT			  CompleteConnect			( D_IPin *pReceivePin ) ;
			HRESULT			  BreakConnect				() ;
			STDMETHODIMP	  RequestAllocator			( D_IMemAllocator* pPreferred, D_ALLOCATOR_PROPERTIES* pProps, D_IMemAllocator ** ppActual);
			STDMETHODIMP	  Request					( D_IMediaSample* pSample, DWORD dwUser);
			STDMETHODIMP	  WaitForNext				( DWORD dwTimeout, D_IMediaSample** ppSample, DWORD * pdwUser);
			STDMETHODIMP	  SyncReadAligned			( D_IMediaSample* pSample);
			STDMETHODIMP	  SyncRead					( LONGLONG llPosition, LONG lLength, BYTE* pBuffer);
			STDMETHODIMP	  Length					( LONGLONG* pTotal, LONGLONG* pAvailable);
			STDMETHODIMP	  BeginFlush				();
			STDMETHODIMP	  EndFlush					();
};

class D_CAsyncReader : public D_CBaseFilter
{

protected:
	D_CCritSec									m_csFilter;
	D_CAsyncIo									m_Io;
	D_CAsyncOutputPin							m_OutputPin;
	D_CMediaType								m_mt;

public:
    
	D_CAsyncReader( const TCHAR *pName, IUnknown * pUnk, D_CAsyncStream *pStream, HRESULT *phr);
	~D_CAsyncReader();

	// --- CBaseFilter methods ---
			int				  GetPinCount				();
			D_CBasePin		 *GetPin					(int n);

	// --- Access our media type
	const	D_CMediaType	 *LoadType					() const { return &m_mt;}
	virtual HRESULT			  Connect					( D_IPin * pReceivePin, const D_AM_MEDIA_TYPE *pmt ){ return m_OutputPin.D_CBasePin::Connect( pReceivePin, pmt ) ; }
} ;

class D_CMemReader : public D_CAsyncReader
{
public:
	D_CMemReader( D_CMemStream *pStream, D_CMediaType *pmt, HRESULT *phr) : D_CAsyncReader(TEXT("Mem Reader\0"), NULL, pStream, phr){ m_mt = *pmt; }

			STDMETHODIMP	  Register					(){ return S_OK; }
			STDMETHODIMP	  Unregister				(){ return S_OK; }
	
	static	D_CUnknown *WINAPI CreateInstance			(D_CMemStream *pStream, D_CMediaType *pmt, HRESULT *phr);
			void			  DeleteInstance			() ;
};






typedef HRESULT ( *D_SAMPLECALLBACK )( D_IMediaSample * pSample, D_REFERENCE_TIME * StartTime, D_REFERENCE_TIME * StopTime, BOOL TypeChanged, void *CallbackData );

class D_CTransInPlaceInputPin : public D_CTransformInputPin
{

protected:
	class D_CTransInPlaceFilter * const			m_pTIPFilter;
	BOOL										m_bReadOnly;

public:

//		D_CTransInPlaceInputPin( TCHAR               *pObjectName, D_CTransInPlaceFilter *pFilter, HRESULT             *phr, LPCWSTR              pName);
	D_CTransInPlaceInputPin( char               *pObjectName, D_CTransInPlaceFilter *pFilter, HRESULT             *phr, LPCWSTR              pName);

			STDMETHODIMP	  EnumMediaTypes			( D_IEnumMediaTypes **ppEnum );
			HRESULT			  CheckMediaType			(const D_CMediaType* pmt);
			STDMETHODIMP	  GetAllocator				(D_IMemAllocator ** ppAllocator);
			STDMETHODIMP	  NotifyAllocator			(D_IMemAllocator * pAllocator, BOOL bReadOnly);
			D_IMemAllocator	  *PeekAllocator			() const {  return m_pAllocator; }
			STDMETHODIMP	  GetAllocatorRequirements	(D_ALLOCATOR_PROPERTIES *pProps);
			HRESULT			  CompleteConnect			(D_IPin *pReceivePin);
			BOOL			  ReadOnly					() { return m_bReadOnly ; }
};

class D_CTransInPlaceOutputPin : public D_CTransformOutputPin
{

protected:
	D_CTransInPlaceFilter * const				m_pTIPFilter;

public:

//		D_CTransInPlaceOutputPin( TCHAR *pObjectName, D_CTransInPlaceFilter *pFilter, HRESULT *phr, LPCWSTR pName);
	D_CTransInPlaceOutputPin( char *pObjectName, D_CTransInPlaceFilter *pFilter, HRESULT *phr, LPCWSTR pName);

			STDMETHODIMP	  EnumMediaTypes			( D_IEnumMediaTypes **ppEnum );
			HRESULT			  CheckMediaType			(const D_CMediaType* pmt);
			void			  SetAllocator				(D_IMemAllocator * pAllocator);
			D_IMemInputPin	  *ConnectedIMemInputPin	(){ return m_pInputPin; }
			D_IMemAllocator	  *PeekAllocator			() const {  return m_pAllocator; }
			HRESULT			  CompleteConnect			(D_IPin *pReceivePin);
};

class
#ifndef DX_GCC_COMPILE
__declspec(novtable)
#endif
D_CTransInPlaceFilter : public D_CTransformFilter
{
public:
	virtual D_CBasePin		  *GetPin					(int n);

public:
	D_CTransInPlaceFilter(TCHAR *, IUnknown *, REFCLSID clsid, HRESULT *, bool bModifiesData = true);
#ifdef UNICODE
	D_CTransInPlaceFilter(CHAR *, IUnknown *, REFCLSID clsid, HRESULT *, bool bModifiesData = true);
#endif
			HRESULT			  GetMediaType				(int /*iPosition*/, D_CMediaType * /*pMediaType*/){ return E_UNEXPECTED;	}
			HRESULT			  DecideBufferSize			(D_IMemAllocator*, D_ALLOCATOR_PROPERTIES *);
			HRESULT			  CheckTransform			(const D_CMediaType * /*mtIn*/, const D_CMediaType * /*mtOut*/){ return S_OK; };
			HRESULT			  CompleteConnect			(D_PIN_DIRECTION dir,D_IPin *pReceivePin);
	virtual HRESULT			  Receive					(D_IMediaSample *pSample);
	virtual HRESULT			  Transform					(D_IMediaSample *pSample ) = 0 ;

#ifdef PERF
	virtual void			  RegisterPerfId			(){ m_idTransInPlace = MSR_REGISTER(TEXT("TransInPlace"));}
#endif

protected:

	D_IMediaSample								*Copy(D_IMediaSample *pSource);

#ifdef PERF
	int											m_idTransInPlace;
#endif
	bool										m_bModifiesData;

	friend class D_CTransInPlaceInputPin;
	friend class D_CTransInPlaceOutputPin;

			D_CTransInPlaceInputPin  *InputPin					() const { return (D_CTransInPlaceInputPin *)m_pInput; };
			D_CTransInPlaceOutputPin *OutputPin					() const { return (D_CTransInPlaceOutputPin *)m_pOutput; };
			BOOL					  TypesMatch				() { return InputPin()->CurrentMediaType() == OutputPin()->CurrentMediaType(); }
			BOOL					  UsingDifferentAllocators	() const { return InputPin()->PeekAllocator() != OutputPin()->PeekAllocator(); }
};

class D_IGrabberSample : public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE SetAcceptedMediaType		( const D_CMediaType *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType		( D_CMediaType *pType) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCallback				( D_SAMPLECALLBACK Callback, void *CallbackData) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDeliveryBuffer			( D_ALLOCATOR_PROPERTIES props, BYTE *pBuffer) = 0;
};

class D_CSampleGrabberInPin;
class D_CSampleGrabber;

class D_CSampleGrabberAllocator : public D_CMemAllocator
{
	friend class D_CSampleGrabberInPin;
	friend class D_CSampleGrabber;

protected:
	D_CSampleGrabberInPin						*m_pPin;

public:
//		D_CSampleGrabberAllocator( D_CSampleGrabberInPin * pParent, HRESULT *phr ) : D_CMemAllocator( TEXT("SampleGrabberAllocator\0"), NULL, phr ), m_pPin( pParent ){};
	D_CSampleGrabberAllocator( D_CSampleGrabberInPin * pParent, HRESULT *phr ) : D_CMemAllocator( "SampleGrabberAllocator\0", NULL, phr ), m_pPin( pParent ){};
	~D_CSampleGrabberAllocator( ){ m_pBuffer = NULL; }

			HRESULT			  Alloc						();
			void			  ReallyFree				();
			STDMETHODIMP	  SetProperties				(D_ALLOCATOR_PROPERTIES *pRequest, D_ALLOCATOR_PROPERTIES *pActual);
};

class D_CSampleGrabberInPin : public D_CTransInPlaceInputPin
{
	friend class D_CSampleGrabberAllocator;
	friend class D_CSampleGrabber;

	D_CSampleGrabberAllocator					*m_pPrivateAllocator;
	D_ALLOCATOR_PROPERTIES						m_allocprops;
	BYTE										*m_pBuffer;
	BOOL										m_bMediaTypeChanged;

protected:
			D_CSampleGrabber  *SampleGrabber			() { return (D_CSampleGrabber*) m_pFilter; }
			HRESULT			  SetDeliveryBuffer			( D_ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer );

public:
	D_CSampleGrabberInPin( D_CTransInPlaceFilter * pFilter, HRESULT * pHr ) ;
	~D_CSampleGrabberInPin() ;

			HRESULT			  GetMediaType				( int iPosition, D_CMediaType *pMediaType );
			STDMETHODIMP	  EnumMediaTypes			( D_IEnumMediaTypes **ppEnum );
			STDMETHODIMP	  NotifyAllocator			( D_IMemAllocator *pAllocator, BOOL bReadOnly );
			STDMETHODIMP	  GetAllocator				( D_IMemAllocator **ppAllocator );
			HRESULT			  SetMediaType				( const D_CMediaType *pmt );
			STDMETHODIMP	  GetAllocatorRequirements	( D_ALLOCATOR_PROPERTIES *pProps );
};

class D_CSampleGrabber : public D_CTransInPlaceFilter, public D_IGrabberSample
{
	friend class D_CSampleGrabberInPin;
	friend class D_CSampleGrabberAllocator;

protected:

	D_CMediaType								m_mtAccept;
	D_SAMPLECALLBACK							m_callback;
	D_CCritSec									m_Lock;
	void										*m_CallbackData ;

			BOOL			  IsReadOnly				( ) { return !m_bModifiesData; }
			HRESULT			  CheckInputType			( const D_CMediaType * pmt );
			HRESULT			  Transform					( D_IMediaSample * pms );
			HRESULT			  Receive					( D_IMediaSample * pms );

public:
	D_CSampleGrabber( IUnknown * pOuter, HRESULT * pHr, BOOL ModifiesData );

			STDMETHODIMP	  NonDelegatingQueryInterface(REFIID riid, void ** ppv);
	virtual HRESULT	__stdcall QueryInterface			( REFIID riid, void **ppv )	{ return GetOwner()->QueryInterface( riid,ppv ) ; } ;
	virtual ULONG	__stdcall AddRef					()							{ return GetOwner()->AddRef();  } ;
	virtual ULONG	__stdcall Release					()							{ return GetOwner()->Release(); } ;

			STDMETHODIMP	  SetAcceptedMediaType		( const D_CMediaType * pmt );
			STDMETHODIMP	  GetConnectedMediaType		( D_CMediaType * pmt );
			STDMETHODIMP	  SetCallback				( D_SAMPLECALLBACK Callback, void *CallbackData );
			STDMETHODIMP	  SetDeliveryBuffer			( D_ALLOCATOR_PROPERTIES props, BYTE * m_pBuffer );

	static	D_CUnknown *WINAPI CreateInstance			(IUnknown * punk, HRESULT *phr);
			void			   DeleteInstance			() ;
};

#endif // DX_NON_DSHOW_MP3


}
#endif	/* DX_NON_MOVIE */
#endif  /* DX_NON_DSHOW_MOVIE */

#endif // __WINDOWS__


#endif	/* __DXDIRECTX_H__ */


