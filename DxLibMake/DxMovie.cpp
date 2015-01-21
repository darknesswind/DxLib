// ----------------------------------------------------------------------------
//
//		ＤＸライブラリ　ムービー再生処理用プログラム
//
//				Ver 3.11f
//
// ----------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

#include "DxMovie.h"

#ifndef DX_NON_MOVIE

// インクルード----------------------------------------------------------------
#include "DxLib.h"
#include "DxStatic.h"
#include "DxFile.h"
#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxSound.h"
#include "DxUseCLib.h"
#include "DxLog.h"
#include "DxSystem.h"
#include "Windows/DxWindow.h"
#include "Windows/DxWinAPI.h"
#include "Windows/DxGuid.h"
#include "Windows/DxFileWin.h"

namespace DxLib
{

// マクロ定義------------------------------------------------------------------

// ムービーを更新する間隔
#define MOVIEREFRESHWAIT		(17)

// ムービーハンドルの有効性チェック
#define MOVIEHCHK( HAND, MPOINT )		HANDLECHK(       DX_HANDLETYPE_MOVIE, HAND, *( ( HANDLEINFO ** )&MPOINT ) )
#define MOVIEHCHK_ASYNC( HAND, MPOINT )	HANDLECHK_ASYNC( DX_HANDLETYPE_MOVIE, HAND, *( ( HANDLEINFO ** )&MPOINT ) )

#define VFW_E_NOT_FOUND                  ((HRESULT)0x80040216L)

#define MOVIE MovieData

// 構造体型宣言----------------------------------------------------------------

// データ宣言------------------------------------------------------------------

MOVIEGRAPHMANAGE MovieData ;								// 動画関連データ

// クラス宣言 -----------------------------------------------------------------

// 関数プロトタイプ宣言--------------------------------------------------------

#ifndef DX_NON_DSHOW_MOVIE
// ムービーファイルをオープンする  
extern int OpenMovieFunction(
	MOVIEGRAPH * Movie,
	const TCHAR *FileName,
	int *Width,
	int *Height,
	int SurfaceMode,
	int ASyncThread
) ;
#endif

// プログラムコード------------------------------------------------------------

// ムービー関連の管理処理の初期化
extern int InitializeMovieManage( void )
{
	if( MOVIE.InitializeFlag == TRUE )
		return -1 ;

	// ハンドル管理情報の初期化
	InitializeHandleManage( DX_HANDLETYPE_MOVIE, sizeof( MOVIEGRAPH ), MAX_MOVIE_NUM, InitializeMovieHandle, TerminateMovieHandle, DXSTRING( _T( "ムービー" ) ) ) ;

	// 初期化フラグ立てる
	MOVIE.InitializeFlag = TRUE ;

	// 終了
	return 0 ;
}

// ムービー関連の管理処理の後始末
extern int TerminateMovieManage( void )
{
	if( MOVIE.InitializeFlag == FALSE )
		return -1 ;

	// ハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_MOVIE ) ;

	// 初期化フラグ倒す
	MOVIE.InitializeFlag = FALSE ;

	// 終了
	return 0 ;
}

// OPENMOVIE_GPARAM のデータをセットする
extern void InitOpenMovieGParam( OPENMOVIE_GPARAM *GParam )
{
	GParam->RightAlphaFlag = MOVIE.RightAlphaFlag ;
	GParam->A8R8G8B8Flag = MOVIE.A8R8G8B8Flag ;
}

#ifndef DX_NON_DSHOW_MOVIE

// ムービーファイルをオープンする  
extern int OpenMovieFunction(
	MOVIEGRAPH * Movie,
	const TCHAR *FileName,
	int *Width,
	int *Height,
	int SurfaceMode,
	int ASyncThread
)
{
	TCHAR ErStr[256] ;
	D_IAMMultiMediaStream *pAMStream = NULL;
	HRESULT hr ;

	Movie->NowImage.GraphData = NULL;
	Movie->YUVFlag = FALSE ;
	Movie->OverlayDestX = 0 ;
	Movie->OverlayDestY = 0 ;
	Movie->OverlayDestExRate = 0 ;
	Movie->OverlayDispFlag = 0 ;
	Movie->FirstUpdateFlag = FALSE ;
	_MEMSET( &Movie->OverlaySrcRect, 0, sizeof( RECT ) ) ;
	_MEMSET( &Movie->OverlayDestRect, 0, sizeof( RECT ) ) ;

	// もしオーバーレイが使えない場合はフルカラーにする
	if( SurfaceMode == DX_MOVIESURFACE_OVERLAY  )
	{
		SurfaceMode = DX_MOVIESURFACE_FULLCOLOR ;
	}

	// もし画面が３２ビットカラーモードでフルカラーモードを指定してきた場合はノーマルにする
	if( SurfaceMode == DX_MOVIESURFACE_FULLCOLOR && NS_GetColorBitDepth() == 32 ) SurfaceMode = DX_MOVIESURFACE_NORMAL ;

	SurfaceMode = DX_MOVIESURFACE_FULLCOLOR ;

	// グラフィックビルダーオブジェクトの作成
	if( ( FAILED( WinAPI_CoCreateInstance_ASync(CLSID_FILTERGRAPH, NULL, CLSCTX_INPROC, IID_IGRAPHBUILDER, (void **)&Movie->pGraph, ASyncThread ) ) ) )
	{
		lstrcpy( ErStr, DXSTRING( _T( "CLSID_FilterGraph の作成に失敗しました\n" ) ) ) ;
		goto ERROR_R ;
	}

	// Create the Texture Renderer object
	Movie->pMovieImage = New_D_CMovieRender( NULL, &hr ) ;
    
    // Get a pointer to the IBaseFilter on the TextureRenderer, add it to graph
    if (FAILED(hr = Movie->pGraph->AddFilter( Movie->pMovieImage, L"MovieRenderer")))
    {
        lstrcpy( ErStr, DXSTRING( _T( "Could not add renderer filter to graph!\n" )) );
        return hr;
    }

	// BasicAudio インターフェイスを得る
	if( FAILED( Movie->pGraph->QueryInterface( IID_IBASICAUDIO, ( void ** )&Movie->pBasicAudio ) ) )
	{
		lstrcpy( ErStr, DXSTRING( _T( "BasicAudio インターフェースの取得に失敗しました\n" ) ) ) ;
		goto ERROR_R ;
	}

	// メディアコントローラオブジェクトを取得する
	if( FAILED( Movie->pGraph->QueryInterface( IID_IMEDIACONTROL, ( void ** )&Movie->pMediaControl ) ) )
	{
		lstrcpy( ErStr, DXSTRING( _T( "IID_IMediaControlインターフェイスの取得に失敗しました\n" ) ) ) ;
		goto ERROR_R ;
	}

	// メディアシーキングオブジェクトを取得する
	if( FAILED( Movie->pGraph->QueryInterface( IID_IMEDIASEEKING, ( void ** )&Movie->pMediaSeeking ) ) )
	{
		lstrcpy( ErStr, DXSTRING( _T( "IID_IMediaSeekingインターフェイスの取得に失敗しました\n" ) ) ) ;
		goto ERROR_R ;
	}

   	// ファイル名保存
#ifdef UNICODE
	lstrcpy( Movie->FileName, FileName ) ;
#else
	// ファイル名をUNICODEに変換
	MBCharToWChar( _GET_CODEPAGE(), FileName, ( DXWCHAR * )Movie->FileName, sizeof( Movie->FileName ) / sizeof( wchar_t ) ) ;
#endif

	hr = Movie->pGraph->RenderFile( Movie->FileName, NULL ) ;
	if( FAILED( hr ) )
	{
		lstrcpy( ErStr, DXSTRING( _T( "RenderFile faired!\n" )) ) ;

		DWORD_PTR fp ;
		HANDLE FileHandle ;
		void *TempBuffer ;
		size_t CompSize, MoveSize ;
		DWORD WriteSize ;
		size_t FileSize ;
		const DWORD BufferSize = 0x100000 ;

		lstrcpy( ErStr, DXSTRING( _T( "ファイル:" ) ) ) ;
		lstrcat( ErStr, FileName ) ;
		lstrcat( ErStr, DXSTRING( _T( " が開けませんでした\n" ) ) );

		// 既にテンポラリファイルを作成している場合は、
		// 更にテンポラリファイルを作成することはしない
		if( Movie->UseTemporaryFile == TRUE )
			goto ERROR_R ;

		// ファイルが開けなかったらアーカイブされている可能性がある
#ifdef UNICODE
		fp = FOPEN( Movie->FileName ) ;
#else
		fp = FOPEN( FileName ) ;
#endif
		if( fp == 0 ) goto ERROR_R ;
		
		// 開けた場合はテンポラリファイルに書き出す
		{
			// ファイルサイズの取得
			FSEEK( fp, 0L, SEEK_END ) ;
			FileSize = ( size_t )FTELL( fp ) ;
			FSEEK( fp, 0L, SEEK_SET ) ;

			// 一時的にデータを格納するバッファを確保
			TempBuffer = DXALLOC( BufferSize ) ;
			if( TempBuffer == NULL )
			{
				FCLOSE( fp ) ;
				goto ERROR_R ;
			}

			// テンポラリファイルの作成
#ifdef UNICODE
			FileHandle = CreateTemporaryFile( Movie->FileName ) ;
#else
			char TempFileNameA[ 512 ] ;
			FileHandle = CreateTemporaryFile( TempFileNameA ) ;
			MBCharToWChar( 932, TempFileNameA, ( DXWCHAR * )Movie->FileName, MAX_PATH ) ;
#endif
			if( FileHandle == NULL )
			{
				FCLOSE( fp ) ;
				DXFREE( TempBuffer ) ;
				goto ERROR_R ;
			}
			Movie->UseTemporaryFile = TRUE ;

			// テンポラリファイルにデータを書き出す
			CompSize = 0 ;
			while( CompSize < FileSize )
			{
				MoveSize = CompSize - FileSize ;
				if( MoveSize > BufferSize ) MoveSize = BufferSize ;

				FREAD( TempBuffer, MoveSize, 1, fp ) ;
				WriteFile( FileHandle, TempBuffer, ( DWORD )MoveSize, &WriteSize, NULL ) ;

				if( MoveSize != WriteSize ) break ;
				CompSize += MoveSize ;
			}

			// ファイルを閉じ、メモリを解放する
			FCLOSE( fp ) ;
			CloseHandle( FileHandle ) ;
			DXFREE( TempBuffer ) ;
		}

		// 改めてファイルを開く
		hr = Movie->pGraph->RenderFile( Movie->FileName, NULL ) ;
		if( FAILED( hr ) )
		{
			// テンポラリファイルを削除
			DeleteFileW( Movie->FileName ) ;
			goto ERROR_R ;
		}
	}

	// １フレームあたりの時間を得る
	Movie->pMediaSeeking->GetDuration( &Movie->FrameTime ) ;
	if( Movie->FrameTime == 0 )
	{
		Movie->FrameTime = 10000000 / 60 ;
	}

	// 終了時間を取得する
	Movie->pMediaSeeking->GetStopPosition( &Movie->StopTime ) ;

	// 画像イメージの情報をセットする
	Movie->NowImage.Width = Movie->pMovieImage->Width ;
	Movie->NowImage.Height = Movie->pMovieImage->Height ;
	Movie->NowImage.Pitch = Movie->pMovieImage->Pitch ;
	Movie->NowImage.GraphData = Movie->pMovieImage->ImageBuffer ;
	Movie->NowImageGraphOutAlloc = TRUE ;
	if( Movie->pMovieImage->ImageType == 0 )
	{
		NS_CreateFullColorData( &Movie->NowImage.ColorData ) ;
	}
	else
	if( Movie->pMovieImage->ImageType == 1 && Movie->A8R8G8B8Flag )
	{
		NS_CreateARGB8ColorData( &Movie->NowImage.ColorData ) ;
	}
	else
	{
		NS_CreateXRGB8ColorData( &Movie->NowImage.ColorData ) ;
	}
	Movie->UseNowImage = &Movie->NowImage ;

	if( Width ) *Width = Movie->pMovieImage->Width ;
	if( Height ) *Height = Movie->pMovieImage->Height ;

	// 終了
	return 0 ;


ERROR_R:

	// 各種ＣＯＭオブジェクトを終了する
	if( pAMStream					){ pAMStream->Release()					; pAMStream = NULL ; }

	if( Movie->pFilter				){ Movie->pFilter->Release()			; Movie->pFilter = NULL ; }
	if( Movie->pAllocator			){ Movie->pAllocator->Release()			; Movie->pAllocator = NULL ; }

	if( Movie->pGraph				){ Movie->pGraph->Release()				; Movie->pGraph = NULL ; }
	if( Movie->pMediaControl		){ Movie->pMediaControl->Release()		; Movie->pMediaControl = NULL ; }
	if( Movie->pMediaSeeking		){ Movie->pMediaSeeking->Release()		; Movie->pMediaSeeking = NULL ; }
	if( Movie->pBasicAudio			){ Movie->pBasicAudio->Release()		; Movie->pBasicAudio = NULL ; }

	return DXST_ERRORLOG_ADD( ErStr ) ;
}

#endif // DX_NON_DSHOW_MOVIE

// ムービーファイルをオープンする
extern int OpenMovie( const TCHAR *FileName, int *Width, int *Height, int SurfaceMode )
{
	OPENMOVIE_GPARAM GParam ;

	InitOpenMovieGParam( &GParam ) ;

	return OpenMovie_UseGParam( &GParam, FileName, Width, Height, SurfaceMode ) ;
}

// ムービーハンドルを初期化をする関数
extern int InitializeMovieHandle( HANDLEINFO * )
{
	// 不需要特别处理
	return 0 ;
}

// ムービーハンドルの後始末を行う関数
extern int TerminateMovieHandle( HANDLEINFO *HandleInfo )
{
	MOVIEGRAPH *Movie = ( MOVIEGRAPH * )HandleInfo ;

#ifndef DX_NON_OGGTHEORA
	// もし Theora を使用していた場合はその開放処理を行う
	if( Movie->TheoraFlag )
	{
		if( Movie->TheoraStreamData )
		{
			FCLOSE( Movie->TheoraStreamData ) ;
			Movie->TheoraStreamData = 0 ;
		}

		TheoraDecode_Terminate( Movie->TheoraHandle ) ;
#ifndef DX_NON_SOUND
		NS_DeleteSoundMem( Movie->TheoraVorbisHandle ) ;
		Movie->TheoraVorbisHandle = 0 ;
#endif // DX_NON_SOUND
		Movie->TheoraHandle = 0 ;
		Movie->TheoraFlag = 0 ;
	}
#endif

#ifndef DX_NON_DSHOW_MOVIE
	// もしテンポラリファイルを使用していた場合はテンポラリファイルを削除する
	if( Movie->UseTemporaryFile == TRUE )
	{
		DeleteFileW( Movie->FileName ) ;
		Movie->UseTemporaryFile = FALSE ;
	}
#endif // DX_NON_DSHOW_MOVIE

	if( Movie->NowImage.GraphData != NULL )
	{
		if( Movie->NowImageGraphOutAlloc == FALSE )
		{
			DXFREE( Movie->NowImage.GraphData ) ;
		}
		Movie->NowImage.GraphData = NULL ;
	}

	// 各種ＣＯＭオブジェクトを終了する
//	if( Movie->pMovieImage )		{ Movie->pMovieImage->Release() ;			Movie->pMovieImage = NULL ; }
//	if( Movie->pFilter )			{ Movie->pFilter->Release() ;				Movie->pFilter = NULL ; }
	if( Movie->RefreshEvent )		{ CloseHandle( Movie->RefreshEvent ) ; 		Movie->RefreshEvent = NULL ; }
#ifndef DX_NON_DSHOW_MOVIE
	if( Movie->pBasicAudio )		{ Movie->pBasicAudio->Release() ; 			Movie->pBasicAudio = NULL ; }
	if( Movie->pMediaSeeking )		{ Movie->pMediaSeeking->Release() ; 		Movie->pMediaSeeking = NULL ; }
	if( Movie->pMediaControl )		{ Movie->pMediaControl->Release(); 			Movie->pMediaControl = NULL ; }
	if( Movie->pGraph )				{ Movie->pGraph->Release(); 				Movie->pGraph = NULL ; }
#endif

	// 正常終了
	return 0 ;
}

// OpenMovie のグローバル変数にアクセスしないバージョン
extern int OpenMovie_UseGParam( OPENMOVIE_GPARAM *GParam, const TCHAR *FileName, int *Width, int *Height, int SurfaceMode, int ASyncThread )
{
	int NewHandle ;
	MOVIEGRAPH * Movie ;

	// ハンドルの作成
	NewHandle = AddHandle( DX_HANDLETYPE_MOVIE ) ;
	if( NewHandle < 0 )
		return -1 ;

	if( MOVIEHCHK( NewHandle, Movie ) )
		return -1 ;

	// 右側をアルファとして扱うかフラグをセットする
	Movie->RightAlpha = GParam->RightAlphaFlag ;

	// 32bitカラーフォーマットの動画を A8R8G8B8 形式として扱うかどうかのフラグをセットする
	Movie->A8R8G8B8Flag = GParam->A8R8G8B8Flag ;

#ifndef DX_NON_OGGTHEORA
	// Ogg Theora としてオープンしようとしてみる
	Movie->TheoraStreamData = FOPEN( FileName ) ;
	Movie->TheoraHandle = TheoraDecode_InitializeStream( &StreamFunction, Movie->TheoraStreamData, 10, ASyncThread ) ;
	if( Movie->TheoraHandle == 0 )
	{
		FCLOSE( Movie->TheoraStreamData ) ;
		Movie->TheoraStreamData = 0 ;
	}
	if( Movie->TheoraHandle != 0 )
	{
		THEORADECODE_INFO Info ;

		// 情報を埋める
		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;
		Movie->Width  = Info.Width ;
		Movie->Height = Info.Height ;
		if( Width  ) *Width  = Info.Width ;
		if( Height ) *Height = Info.Height ;
		Movie->TheoraFrameRate = Info.FrameRate ;
		Movie->TheoraTotalPlayTime = _DTOL( 1000.0 / Info.FrameRate * Info.TotalFrame ) ;

#ifndef DX_NON_SOUND
		LOADSOUND_GPARAM GParam ;

		InitLoadSoundGParam( &GParam ) ;

		// 開いたら Thera 用のセットアップを行う

		// サウンド再生用にサウンドデータとしても読み込み
		GParam.CreateSoundDataType = DX_SOUNDDATATYPE_FILE ;
		GParam.OggVorbisFromTheoraFile = TRUE ;
		GParam.DisableReadSoundFunctionMask = ~DX_READSOUNDFUNCTION_OGG ;
		Movie->TheoraVorbisHandle = LoadSoundMemBase_UseGParam( &GParam, FileName, 1, -1, FALSE, ASyncThread ) ;
		Movie->TheoraVorbisTotalTime = NS_GetSoundTotalTime( Movie->TheoraVorbisHandle ) ;

		// ループタイプの決定( 長いほうを基準にする )
		Movie->TheoraLoopType = Movie->TheoraVorbisTotalTime > Movie->TheoraTotalPlayTime ? 1 : 0 ;

#else // DX_NON_SOUND
		// ループタイプは動画データ合わせ
		Movie->TheoraLoopType = 0 ;
#endif // DX_NON_SOUND

		// Theora を使用しているフラグを立てる
		Movie->TheoraFlag = 1 ;

		// 使用すべき BASEIMAGE をセット
		Movie->UseNowImage = ( BASEIMAGE * )TheoraDecode_GetBaseImage( Movie->TheoraHandle ) ;

		// サーフェスモードはノーマルということで
		Movie->SurfaceMode = DX_MOVIESURFACE_NORMAL ;
	}
	else
#endif
	{
		// オープンできなかったら普通のムービーとして開いてみる

#ifndef DX_NON_DSHOW_MOVIE
		// ファイルのオープン
		Movie->UseTemporaryFile = FALSE ;
		if( OpenMovieFunction( Movie, FileName, Width, Height,SurfaceMode, ASyncThread ) == -1 )
#endif
		{
			DXST_ERRORLOG_ADD( _T( "ムービーファイルオープン処理時にエラーが発生しました" ) ) ;
			goto ERR ;
		}

		// 使用すべき BASEIMAGE をセット
		Movie->UseNowImage = &Movie->NowImage ;
	}

	// 画像が更新されたフラグを倒す
	Movie->NowImageUpdateFlag = FALSE ;

	// 内部一時停止フラグを立てる
	Movie->SysPauseFlag = 1 ;

	// 再生中フラグを倒す
	Movie->PlayFlag = FALSE ;

	// 再生タイプはバックグラウンドにしておく
	Movie->PlayType = DX_PLAYTYPE_BACK ;

	// 返回句柄
	return NewHandle ;

ERR :
	CloseMovie( NewHandle ) ;

	// エラー終了
	return -1 ;
}

// ムービーグラフィックを終了する
extern int CloseMovie( int MovieHandle )
{
	return SubHandle( MovieHandle ) ;
}

// ムービーの再生を開始する
extern int PlayMovie_( int MovieHandle, int PlayType, int SysPlay )
{
	MOVIEGRAPH * Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

	// PlayType が -1 の場合は変更しない
	if( PlayType == -1 ) PlayType = Movie->PlayType ;

	// PlayType に DX_PLAYTYPE_NORMAL を指定した場合は DX_PLAYTYPE_BACK になる
	if( PlayType == DX_PLAYTYPE_NORMAL ) PlayType = DX_PLAYTYPE_BACK ;

	// 内部一時停止フラグが倒れていたら何もしない
	if( Movie->SysPauseFlag == 0 ) return 0 ;

	if( SysPlay == 1 && Movie->PlayFlag == FALSE ) return 0 ;

#ifndef DX_NON_OGGTHEORA
	// Theora を使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		int CurFrame ;
		THEORADECODE_INFO Info ;

		// 再生開始時の時間を取得
		Movie->TheoraPlayTime = NS_GetNowHiPerformanceCount() ;

		// 既に再生済みのフレーム数分だけ前に再生したことにする
		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;
		CurFrame = TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) ;
		Movie->TheoraPlayTime -= _DTOL( 1000000.0 / Movie->TheoraFrameRate * CurFrame ) ;

		// Vorbis の再生も開始する
#ifndef DX_NON_SOUND
//		NS_SetSoundCurrentTime( _DTOL( 1000.0 / Movie->TheoraFrameRate * CurFrame ), Movie->TheoraVorbisHandle ) ; 
		NS_PlaySoundMem( Movie->TheoraVorbisHandle, Movie->TheoraLoopType == 1 ? PlayType : DX_PLAYTYPE_BACK, FALSE ) ;
#endif // DX_NON_SOUND
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		// 再生
		Movie->pMediaControl->Run() ;
	}
#else // DX_NON_DSHOW_MOVIE
	{
	}
#endif

	// 画像が更新されたフラグを倒す
	Movie->NowImageUpdateFlag = FALSE ;

	// 内部一時停止フラグを倒す
	Movie->SysPauseFlag = 0 ;

	// 再生タイプを保存する
	Movie->PlayType = PlayType ;

	// 再生中フラグを立てる
	Movie->PlayFlag = TRUE ;

	// 終了
	return 0 ;
}

// ムービーの再生をストップする
extern int PauseMovie( int MovieHandle, int SysPause )
{
	MOVIEGRAPH * Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

	// 内部一時停止フラグが立っていたらなにもしない
	if( Movie->SysPauseFlag == 1 ) return 0 ;

#ifndef DX_NON_OGGTHEORA
	// Theora を使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		// Theora を使用している場合

		// Vorbis の再生を停止する
#ifndef DX_NON_SOUND
		NS_StopSoundMem( Movie->TheoraVorbisHandle ) ;
#endif // DX_NON_SOUND

		// 現在の再生時間分までフレームを進めておく
		UpdateMovie( MovieHandle ) ;
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		if( Movie->pMediaControl == NULL ) return 0 ; 

		// 停止
		Movie->pMediaControl->Pause() ;
	}
#else // DX_NON_DSHOW_MOVIE
	{
	}
#endif

	// 内部一時停止フラグを立てる
	Movie->SysPauseFlag = 1 ;

	// 再生中フラグを倒す
	if( SysPause == 0 )
	{
		Movie->PlayFlag = FALSE ;
	}

	// 終了
	return 0 ;
}

// ムービーのフレームを進める、戻すことは出来ない( ムービーが停止状態で、且つ Ogg Theora のみ有効 )
extern int AddMovieFrame( int MovieHandle, unsigned int FrameNum )
{
	MOVIEGRAPH * Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

	// 画像が更新されたフラグを倒す
	Movie->NowImageUpdateFlag = FALSE ;

#ifndef DX_NON_OGGTHEORA
	// Ogg Theora ではなかったら変更できない
	if( Movie->TheoraFlag == FALSE )
		return -1 ;

	// 再生中だったら変更できない
	if( GetMovieState( MovieHandle ) == TRUE )
		return -1 ;

	// 再生位置を変更
	TheoraDecode_IncToFrame( Movie->TheoraHandle, FrameNum ) ;

	// 終了
	return 0 ;
#else
	return -1 ;
#endif
}

// ムービーの再生位置を設定する(ミリ秒単位)
extern int SeekMovie( int MovieHandle, int Time )
{
	MOVIEGRAPH * Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

	// 再生を止める
	PauseMovie( MovieHandle ) ;

	// 画像が更新されたフラグを倒す
	Movie->NowImageUpdateFlag = FALSE ;

#ifndef DX_NON_OGGTHEORA
	// Theora を使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		int CurFrame ;
		THEORADECODE_INFO Info ;

		// コンバート位置を変更する
		TheoraDecode_SeekToTime( Movie->TheoraHandle, Time * 1000 ) ;

		// 再生開始タイムを変更する
		Movie->TheoraPlayTime = NS_GetNowHiPerformanceCount() ;
		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;
		CurFrame = TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) ;
		Movie->TheoraPlayTime -= _DTOL( 1000000.0 / Movie->TheoraFrameRate * CurFrame ) ;
	
		// 再生位置を変更する
#ifndef DX_NON_SOUND
		NS_SetSoundCurrentTime( Time, Movie->TheoraVorbisHandle ) ;
#endif // DX_NON_SOUND
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		LONGLONG Now, Stop ;

		if( Movie->pMediaSeeking == NULL ) return 0 ;

		Now = ( LONGLONG )Time * 10000 ;
		Stop = 0 ;
		Movie->pMediaSeeking->SetPositions( &Now, D_AM_SEEKING_AbsolutePositioning, &Stop, D_AM_SEEKING_NoPositioning ) ;
	}
#else // DX_NON_DSHOW_MOVIE
	{
	}
#endif

	// 終了
	return 0 ;
}

// ムービーの再生状態を得る
extern int GetMovieState( int MovieHandle )
{
	MOVIEGRAPH * Movie ;
//	OAFilterState PlayFlag ;
#ifndef DX_NON_DSHOW_MOVIE
	LONGLONG Current ;
#endif

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	// Theoraを使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
/*
#ifndef DX_NON_SOUND
		if( Movie->TheoraVorbisHandle != -1 )
		{
			// サウンドが停止していたら再生停止
			if( NS_CheckSoundMem( Movie->TheoraVorbisHandle ) == 0 )
			{
				// 現在の再生時間分までフレームを進めておく
				UpdateMovie( MovieHandle ) ;

				// 内部一時停止フラグを立てる
				Movie->SysPauseFlag = 1 ;

				// 再生中フラグを倒す
				Movie->PlayFlag = FALSE ;

				// 内部一時停止フラグを立てる
				Movie->SysPauseFlag = 1 ;
			}
		}
		else
#endif // DX_NON_SOUND
*/
		{
			return Movie->PlayFlag ;
		}
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		D_OAFilterState state ;

		if( Movie->pMediaSeeking == NULL )
			return Movie->PlayFlag ; 

		if( Movie->pMediaControl->GetState( 1000, &state ) != S_OK )
		{
			return Movie->PlayFlag ;
		}

		Movie->pMediaSeeking->GetCurrentPosition( &Current ) ;
		if( ( Movie->PlayType & DX_PLAYTYPE_LOOPBIT ) == 0 && Current >= Movie->StopTime && state == D_State_Stopped )
		{
			Movie->PlayFlag = FALSE ;

			// 内部一時停止フラグを立てる
			Movie->SysPauseFlag = 1 ;
		}
	}
#else // DX_NON_DSHOW_MOVIE
	{
	}
#endif

//	if( Movie->pMediaControl->GetState( INFINITE, &PlayFlag ) != S_OK ) return -1 ;
//	return PlayFlag == State_Running ? 1 : 0 ;
	return Movie->PlayFlag ;
}

// ムービーのボリュームをセットする(0～10000)
extern int SetMovieVolume( int Volume, int MovieHandle )
{
	MOVIEGRAPH * Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	// Theora を使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		// Vorbis サウンドの音量をセット
#ifndef DX_NON_SOUND
		NS_SetVolumeSoundMem( Volume, Movie->TheoraVorbisHandle ) ;
#endif // DX_NON_SOUND
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		if( Movie->pBasicAudio == NULL ) return 0 ; 

		// 音量セット
		if( Volume > 10000 ) Volume = 10000 ;
		if( Volume < 0 ) Volume = 0 ;
		Movie->pBasicAudio->put_Volume( -10000 + Volume ) ;
	}
#else // DX_NON_DSHOW_MOVIE
	{
	}
#endif

	// 終了
	return 0 ;
}

// ムービーの基本イメージデータを取得する
extern BASEIMAGE *GetMovieBaseImage( int MovieHandle, int *ImageUpdateFlag )
{
	MOVIEGRAPH * Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return NULL ;

#ifndef DX_NON_OGGTHEORA
	// Theora を使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		// ムービーのフレームを更新
		UpdateMovie( MovieHandle ) ;

		// NowImage の内容が更新されたかどうかのフラグを代入する
		if( ImageUpdateFlag != NULL )
		{
			*ImageUpdateFlag = Movie->NowImageUpdateFlag ;
		}
		Movie->NowImageUpdateFlag = FALSE ;

		// Theora のイメージを返す
		return ( BASEIMAGE * )TheoraDecode_GetBaseImage( Movie->TheoraHandle ) ;
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		// 使用していない場合

		// ムービーのフレームを更新
		if( GetMovieState( MovieHandle ) == FALSE )
		{
			int Time ;

			Time = TellMovie( MovieHandle ) ;
			PlayMovie_( MovieHandle ) ;
			UpdateMovie( MovieHandle, TRUE ) ;
			PauseMovie( MovieHandle ) ;
			SeekMovie( MovieHandle, Time ) ;
		}
		else
		{
			UpdateMovie( MovieHandle ) ;
		}

		// NowImage の内容が更新されたかどうかのフラグを代入する
		if( ImageUpdateFlag != NULL )
		{
			*ImageUpdateFlag = Movie->NowImageUpdateFlag ;
		}
		Movie->NowImageUpdateFlag = FALSE ;

		return &Movie->NowImage ;
	}
#else // DX_NON_DSHOW_MOVIE
	return NULL ;
#endif
}

// ムービーの総フレーム数を得る( Ogg Theora でのみ有効 )
extern int GetMovieTotalFrame( int MovieHandle )
{
	MOVIEGRAPH * Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	THEORADECODE_INFO info ;

	// Ogg Theora ではなかったら変更できない
	if( Movie->TheoraFlag == FALSE )
		return -1 ;

	// 情報の取得
	TheoraDecode_GetInfo( Movie->TheoraHandle, &info ) ;

	// 総フレーム数を返す
	return info.TotalFrame ;
#else
	return -1 ;
#endif
}

// ムービーの再生位置を取得する(ミリ秒単位)
extern int TellMovie( int MovieHandle )
{
	MOVIEGRAPH * Movie ;
#ifndef DX_NON_DSHOW_MOVIE
	D_STREAM_TIME NowTime ;
#endif
	
	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	// Theora を使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		THEORADECODE_INFO Info ;

		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;

		// フレームを更新
		UpdateMovie( MovieHandle ) ;

		// フレームから再生時間を割り出す
		return _DTOL( TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) * 1000 / Movie->TheoraFrameRate ) ;
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		if( Movie->pMediaSeeking == NULL ) return 0 ;

		// 時間取得
		if( Movie->pMediaSeeking->GetCurrentPosition( &NowTime ) != S_OK ) return -1 ;

		// 時間を返す
		return _DTOL( (double)NowTime / 10000 ) ;
	}
#else // DX_NON_DSHOW_MOVIE
	return -1 ;
#endif
}
 
// ムービーの再生位置を取得する(フレーム単位)
extern int TellMovieToFrame( int MovieHandle )
{
	MOVIEGRAPH * Movie ;
#ifndef DX_NON_DSHOW_MOVIE
	D_STREAM_TIME NowTime ;
#endif
	
	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	// Theora かどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		THEORADECODE_INFO Info ;

		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;

		// フレームを更新
		UpdateMovie( MovieHandle ) ;

		// 現在のフレームを返す
		return TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) ;
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		if( Movie->pMediaSeeking == NULL ) return 0 ;

		// 時間取得
		if( Movie->pMediaSeeking->GetCurrentPosition( &NowTime ) != S_OK ) return -1 ;

		// 時間を返す
		return _DTOL( (double)NowTime / Movie->FrameTime ) ;
	}
#else // DX_NON_DSHOW_MOVIE
	return -1 ;
#endif
}

// ムービーの再生位置を設定する(フレーム単位)
extern int SeekMovieToFrame( int MovieHandle, int Frame )
{
	MOVIEGRAPH * Movie ;
//	D_OAFilterState State ;
	
	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

	// 画像が更新されたフラグを倒す
	Movie->NowImageUpdateFlag = FALSE ;

#ifndef DX_NON_OGGTHEORA
	// Theora かどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		int CurFrame ;
		THEORADECODE_INFO Info ;

		// コンバート位置を変更する
		TheoraDecode_SeekToFrame( Movie->TheoraHandle, Frame ) ;

		// 再生開始タイムを変更する
		Movie->TheoraPlayTime = NS_GetNowHiPerformanceCount() ;
		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;
		CurFrame = TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) ;
		Movie->TheoraPlayTime -= _DTOL( 1000000.0 / Movie->TheoraFrameRate * CurFrame ) ;
	
		// 再生位置を変更する
#ifndef DX_NON_SOUND
		NS_SetSoundCurrentTime( _DTOL( Frame * 1000 / Movie->TheoraFrameRate ), Movie->TheoraVorbisHandle ) ;
#endif // DX_NON_SOUND
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		LONGLONG Now, Stop ;

		if( Movie->pMediaSeeking == NULL ) return 0 ;

		Now = ( D_STREAM_TIME )_DTOL64( (double)Frame * Movie->FrameTime ) ;
		Stop = 0 ;
		Movie->pMediaSeeking->SetPositions( &Now, D_AM_SEEKING_AbsolutePositioning, &Stop, D_AM_SEEKING_NoPositioning ) ;
	}
#else  // DX_NON_DSHOW_MOVIE
	{
	}
#endif

	// 終了
	return 0 ;
}

// ムービーの１フレームあたりの時間を得る
extern LONGLONG GetOneFrameTimeMovie( int MovieHandle )
{
	MOVIEGRAPH * Movie ;
	
	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	// Theora かどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		THEORADECODE_INFO Info ;

		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;
		return _DTOL( 1000000 / Movie->TheoraFrameRate ) ;
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		return Movie->FrameTime ;
	}
#else // DX_NON_DSHOW_MOVIE
	return -1 ;
#endif
}

// ムービーグラフィックのデータを取得する
extern MOVIEGRAPH * GetMovieData( int MovieHandle )
{
	MOVIEGRAPH * Movie ;
	
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return NULL ;

	return Movie ;
}


// ムービーで使用しているサーフェスの解放を行う
extern int ReleaseMovieSurface( int MovieHandle )
{
	MOVIEGRAPH * Movie ;
	
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	if( Movie->TheoraFlag )
	{
		TheoraDecode_SurfaceTerminate( Movie->TheoraHandle ) ;
	}
	else
#endif
	{
	}

	// 終了
	return 0 ;
}

// ムービーの更新を行う
extern int UpdateMovie( int MovieHandle, int /*AlwaysFlag*/ )
{
	MOVIEGRAPH * Movie ;
//	LONGLONG NowFrame, OldFrame ;
	
	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

#ifndef DX_NON_OGGTHEORA
	// Theora を使用しているかどうかで処理を分岐
	if( Movie->TheoraFlag )
	{
		// Theora を使用している場合
		int NowFrame, CurFrame, AddFrame ;
		LONGLONG NowTime ;
		THEORADECODE_INFO Info ;

		// Theora の情報を取得
		TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;

		// 今の再生時間分フレームを進める
		if( Movie->SysPauseFlag == 0 )
		{
			// 再生時間は音声が有効な場合は音声から取得する
#ifndef DX_NON_SOUND
			if( Movie->TheoraVorbisHandle != -1 && NS_CheckSoundMem( Movie->TheoraVorbisHandle ) == 1 )
			{
				NowTime = NS_GetSoundCurrentTime( Movie->TheoraVorbisHandle ) * 1000 ;
				NowFrame = _DTOL( ( double )( NowTime * Movie->TheoraFrameRate ) / 1000000 ) ;

				// 音声から音を取得した場合は、映像の最大フレームを超えてしまった場合は現在フレームを映像の最大フレームにする
				if( NowFrame >= Info.TotalFrame )
				{
					NowFrame = Info.TotalFrame - 1 ;
				}
			}
			else
#endif // DX_NON_SOUND
			{
				NowTime = NS_GetNowHiPerformanceCount() ;
				NowFrame = _DTOL( ( double )( ( NowTime - Movie->TheoraPlayTime ) * Movie->TheoraFrameRate ) / 1000000 ) ;

				// ループ指定があるかどうかで総フレーム数を超えている場合の処理を分岐する
				if( Info.TotalFrame <= NowFrame )
				{
					if( Movie->PlayType & DX_PLAYTYPE_LOOPBIT )
					{
						// ループする場合は総フレーム数で割った余りを出す
						NowFrame %= Info.TotalFrame ;
					}
					else
					{
						// ループしない場合は最終フレームで止まる
						NowFrame = Info.TotalFrame - 1 ;
					}
				}
			}

			// ムービーの再生フレームを取得する
			CurFrame = TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) ;

			// 希望のフレームが現在バッファに格納されているフレームと違う場合はバッファを更新する
			if( CurFrame != NowFrame )
			{
				Movie->NowImageUpdateFlag = TRUE ;

				// 加算するフレーム数を算出
				if( NowFrame < CurFrame )
				{
					// ループする場合
					AddFrame = Info.TotalFrame - CurFrame + NowFrame ;

#ifndef DX_NON_SOUND
					// ループする場合で、再生タイプが動画基準の場合はサウンドも再度再生を開始する
					if( Movie->TheoraLoopType == 0 )
					{
						NS_PlaySoundMem( Movie->TheoraVorbisHandle, DX_PLAYTYPE_BACK ) ;
					}
#endif // DX_NON_SOUND
				}
				else
				{
					AddFrame = NowFrame - CurFrame ;
				}

				// フレームを進める
				TheoraDecode_IncToFrame( Movie->TheoraHandle, AddFrame ) ;

				// 最終更新時間を保存
				Movie->RefreshTime = ( int )( NowTime / 1000 ) ;
			}
			else
			{
				// 希望のフレームも現フレームも終端に達していて且つループ指定でも無くサウンドも無い場合はここで再生終了
				if( NowFrame == Info.TotalFrame - 1 &&
					( Movie->PlayType & DX_PLAYTYPE_LOOPBIT ) == 0
#ifndef DX_NON_SOUND
					&& ( Movie->TheoraVorbisHandle == -1 || NS_CheckSoundMem( Movie->TheoraVorbisHandle ) != 1 )
#endif // DX_NON_SOUND
					)
				{
					Movie->PlayFlag = FALSE ;

					// 内部一時停止フラグを立てる
					Movie->SysPauseFlag = 1 ;
				}
			}
		}

		// イメージの構築
		if( Movie->UpdateFunction )
		{
			Movie->UpdateFunction( Movie, Movie->UpdateFunctionData ) ;

			// 最初の更新が行われたフラグをセットする
			Movie->FirstUpdateFlag = TRUE ;
		}
	}
	else
#endif
#ifndef DX_NON_DSHOW_MOVIE
	{
		LONGLONG Now, Stop ;

		if( Movie->pMovieImage->NewImageSet )
		{
			Movie->NowImageUpdateFlag = TRUE ;

			Movie->pMovieImage->NewImageSet = 0 ;
			if( Movie->UpdateFunction )
			{
				Movie->NowImage.Width = Movie->pMovieImage->Width ;
				Movie->NowImage.Height = Movie->pMovieImage->Height ;
				Movie->NowImage.Pitch = Movie->pMovieImage->Pitch ;
				Movie->NowImage.GraphData = Movie->pMovieImage->ImageBuffer ;
				Movie->NowImageGraphOutAlloc = TRUE ;
				if( Movie->pMovieImage->ImageType == 0 )
				{
					NS_CreateFullColorData( &Movie->NowImage.ColorData ) ;
				}
				else
				if( Movie->pMovieImage->ImageType == 1 && Movie->A8R8G8B8Flag )
				{
					NS_CreateARGB8ColorData( &Movie->NowImage.ColorData ) ;
				}
				else
				{
					NS_CreateXRGB8ColorData( &Movie->NowImage.ColorData ) ;
				}
				Movie->UseNowImage = &Movie->NowImage ;
				Movie->UpdateFunction( Movie, Movie->UpdateFunctionData ) ;
			}
		}

		if( Movie->pMediaSeeking && Movie->pMediaControl )
		{
			Movie->pMediaSeeking->GetCurrentPosition( &Now ) ;
			if( Now >= Movie->StopTime )
			{
				if( Movie->PlayType & DX_PLAYTYPE_LOOPBIT )
				{
					Now = 0 ;
					Stop = 0 ;
					Movie->pMediaSeeking->SetPositions( &Now, D_AM_SEEKING_AbsolutePositioning, &Stop, D_AM_SEEKING_NoPositioning ) ;
					Movie->pMediaControl->Run() ;
				}
				else
				{
					// 停止
					Movie->pMediaControl->Pause() ;

					Movie->PlayFlag = FALSE ;

					// 内部一時停止フラグを立てる
					Movie->SysPauseFlag = 1 ;
				}
			}
		}
	}
#else // DX_NON_DSHOW_MOVIE
	{
	}
#endif

	// 終了
	return 0 ;
}

// ムービーの更新時に呼び出すコールバック関数を登録する
extern int SetCallbackMovie( int MovieHandle, void ( *Callback )( MOVIEGRAPH *Movie, void *Data ), void *Data )
{
	MOVIEGRAPH *Movie ;

	// ムービーデータハンドルを取得
	if( MOVIEHCHK( MovieHandle, Movie ) )
		return -1 ;

	// アドレスをセット
	Movie->UpdateFunction = Callback ;
	Movie->UpdateFunctionData = Data ;

	// 終了
	return 0 ;
}

// ムービーの再生状態を停止する
extern int DisableMovieAll( void )
{
	MOVIEGRAPH *Movie ;
	int i ;

	if( MOVIE.InitializeFlag == FALSE )
		return -1 ;

	// すべてのムービーについて処理を行う
	for( i = HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMax ; i ++ )
	{
		Movie = ( MOVIEGRAPH * )HandleManageArray[ DX_HANDLETYPE_MOVIE ].Handle[ i ] ;
		if( Movie == NULL ) continue ;

#ifndef DX_NON_OGGTHEORA
		// Theora を使用しているかどうかで処理を分岐
		if( Movie->TheoraFlag )
		{
			// 何もしない
/*
			// 内部一時停止フラグが倒れていたら処理
			if( Movie->SysPauseFlag == 0 )
			{
				// Vorbis の再生を停止する
				NS_StopSoundMem( Movie->TheoraVorbisHandle ) ;

				// 現在の再生時間分までフレームを進めておく
				{
					int NowFrame, CurFrame ;
					LONGLONG NowTime ;
					THEORADECODE_INFO Info ;

					// Theora の情報を取得
					TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;

					// 今の再生時間分フレームを進める
					NowTime = NS_GetNowHiPerformanceCount() ;
					NowFrame = _DTOL( ( double )( ( NowTime - Movie->TheoraPlayTime ) * Movie->TheoraFrameRate ) / 1000000 ) ;
					CurFrame = TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) ;
					if( CurFrame != NowFrame )
					{
						if( NowFrame < CurFrame )
						{
							TheoraDecode_SeekToFrame( Movie->TheoraHandle, NowFrame ) ;
						}
						else
						{
							TheoraDecode_IncToFrame( Movie->TheoraHandle, NowFrame - CurFrame ) ;
						}
					}

					// イメージの構築
					TheoraDecode_SetupImage( Movie->TheoraHandle ) ;
				}

				// 内部一時停止フラグを立てる
				Movie->SysPauseFlag = 1 ;
			}
*/		}
		else
#endif
#ifndef DX_NON_DSHOW_MOVIE
		{
/*			if( Movie )
			{
				// 現在の再生時間を保存
				Movie->pMediaSeeking->GetCurrentPosition( &Movie->BackUpTime ) ;

				// 各種ＣＯＭオブジェクトを終了する

				if( Movie->pMediaControl		){ Movie->pMediaControl->Release() ;		Movie->pMediaControl = NULL ;		}
				if( Movie->pMediaSeeking		){ Movie->pMediaSeeking->Release() ;		Movie->pMediaSeeking = NULL ;		}
				if( Movie->pBasicAudio			){ Movie->pBasicAudio->Release() ;			Movie->pBasicAudio = NULL ;			}
				if( Movie->pMovieImage			){ Movie->pMovieImage->Release() ;			Movie->pMovieImage = NULL ;			}
				if( Movie->pFilter				){ Movie->pFilter->Release() ;				Movie->pFilter = NULL ;				}
				if( Movie->pGraph				){ Movie->pGraph->Release() ;				Movie->pGraph = NULL ;				}
			}
*/		}
#else  // DX_NON_DSHOW_MOVIE
		{
		}
#endif
	}

	// 終了
	return 0 ;
}


// ムービーの再生状態を復元する
extern int RestoreMovieAll( void )
{
	MOVIEGRAPH *Movie ;
	int i ;

	if( MOVIE.InitializeFlag == FALSE )
		return -1 ;

	// すべてのムービーについて処理を行う
	for( i = HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMax ; i ++ )
	{
		Movie = ( MOVIEGRAPH * )HandleManageArray[ DX_HANDLETYPE_MOVIE ].Handle[ i ] ;
		if( Movie == NULL ) continue ;

#ifndef DX_NON_OGGTHEORA
		// Theora かどうかで処理を分岐
		if( Movie->TheoraFlag )
		{
			// 何もしない
/*
			// 再生中フラグが立っている場合のみ処理
			if( Movie->PlayFlag )
			{
				// 内部一時停止フラグが立っていたら処理
				if( Movie->SysPauseFlag == 1 )
				{
					int CurFrame ;
					THEORADECODE_INFO Info ;

					// 再生開始時の時間を取得
					Movie->TheoraPlayTime = NS_GetNowHiPerformanceCount() ;

					// 既に再生済みのフレーム数分だけ前に再生したことにする
					TheoraDecode_GetInfo( Movie->TheoraHandle, &Info ) ;
					CurFrame = TheoraDecode_GetCurrentFrame( Movie->TheoraHandle ) ;
					Movie->TheoraPlayTime -= _DTOL( 1000000.0 / Movie->TheoraFrameRate * CurFrame ) ;

					// Vorbis の再生も開始する
					NS_PlaySoundMem( Movie->TheoraVorbisHandle, DX_PLAYTYPE_BACK, FALSE ) ;

					// 内部一時停止フラグを倒す
					Movie->SysPauseFlag = 0 ;
				}
			}
*/
		}
		else
#endif
#ifndef DX_NON_DSHOW_MOVIE
		{
/*			if( Movie )
			{
				LONGLONG Now, Stop ;

				// 再オープン処理を行う
				if( OpenMovieFunction( Movie, Movie->FileName, NULL, NULL, Movie->SurfaceMode ) == -1 )
					return -1 ;

				// 削除前の再生ポイントに設定
				Now = Movie->BackUpTime ;
				Stop = 0 ;
				Movie->pMediaSeeking->SetPositions( &Now, D_AM_SEEKING_AbsolutePositioning, &Stop, D_AM_SEEKING_NoPositioning ) ;

				// 再生中フラグが立っている場合は再生
				if( Movie->SysPauseFlag == 0 )
				{
					Movie->pMediaControl->Run() ;
				}
			}
*/		}
#else // DX_NON_DSHOW_MOVIE
		{
		}
#endif
	}

	// 終了
	return 0 ;

}

// すべてのムービーグラフィックをスタート
extern int PlayMovieAll( void )
{
	MOVIEGRAPH *Movie ;
	int i ;

	if( MOVIE.InitializeFlag == FALSE )
		return -1 ;

	for( i = HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMax ; i ++ )
	{
		Movie = ( MOVIEGRAPH * )HandleManageArray[ DX_HANDLETYPE_MOVIE ].Handle[ i ] ;
		if( Movie == NULL ) continue ; 

		PlayMovie_( Movie->HandleInfo.Handle, -1, 1 ) ;
	}

	// 終了
	return 0 ;
}

// すべてのムービーグラフィックをストップ
extern int PauseMovieAll( void )
{
	MOVIEGRAPH *Movie ;
	int i ;

	if( MOVIE.InitializeFlag == FALSE )
		return -1 ;

	for( i = HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MOVIE ].AreaMax ; i ++ )
	{
		Movie = ( MOVIEGRAPH * )HandleManageArray[ DX_HANDLETYPE_MOVIE ].Handle[ i ] ;
		if( Movie == NULL ) continue ; 

		PauseMovie( Movie->HandleInfo.Handle, 1 ) ;
	}

	// 終了
	return 0 ;
}

// 動画ファイルの右半分をアルファ情報として扱うかどうかをセットする( TRUE:アルファ情報として扱う )
extern int NS_SetMovieRightImageAlphaFlag( int Flag )
{
	MOVIE.RightAlphaFlag = Flag;
	return TRUE;
}

// 読み込む動画ファイルが32bitカラーだった場合、A8R8G8B8 形式として扱うかどうかをセットする、
// 32bitカラーではない動画ファイルに対しては無効( Flag  TRUE:A8R8G8B8として扱う  FALSE:X8R8G8B8として扱う( デフォルト ) )
extern int NS_SetMovieColorA8R8G8B8Flag( int Flag )
{
	MOVIE.A8R8G8B8Flag = Flag;
	return TRUE;
}


}

#endif
