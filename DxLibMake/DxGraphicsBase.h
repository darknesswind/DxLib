// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		共通描画プログラムヘッダファイル
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

#ifndef __DXGRAPHICSBASE_H__
#define __DXGRAPHICSBASE_H__


// Include ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include "DxLib.h"
#include "DxStatic.h"
#include "DxBaseImage.h"

namespace DxLib
{

// 宏定义 --------------------------------------------------------------------

#define GBASE						GraphicsBaseData
#define MASKD						MaskManageData

// 記録しておく有効になっているライトのインデックスの数
#define MAX_D3DENABLELIGHTINDEX_NUM			(256)

// 同時に描画できるレンダリングターゲットの最大数
#define DX_RENDERTARGET_COUNT				(16)

// 使用するテクスチャステージの最大数
#define USE_TEXTURESTAGE_NUM				(12)

// 同時に適用できるシャドウマップの最大数
#define MAX_USE_SHADOWMAP_NUM				(2)

// 结构体定义 --------------------------------------------------------------------

// 浮動小数点型の RECT 構造体
struct RECTF
{
	float left, top ;
	float right, bottom ;
} ;

// グラフィックハンドルのセットアップに必要なグローバルデータを纏めた構造体
struct SETUP_GRAPHHANDLE_GPARAM
{
	DWORD					TransColor ;							// 透過色

	int						CreateImageColorBitDepth ;				// 作成する画像の色深度
	int						CreateImageChannelBitDepth ;			// 作成する画像の１チャンネル辺りのビット深度( こちらが設定されている場合は CreateImageColorBitDepth より優先される )
	int						AlphaTestImageCreateFlag ;				// αテスト付き画像作成指定フラグ( AlphaGraphCreateFlag の方が優先度が高い )( テクスチャサーフェスのみ )
	int						AlphaChannelImageCreateFlag ;			// αチャンネル付き画像作成指定フラグ( DrawValidGraphCreateFlag の方が優先度が高い )
	int						CubeMapTextureCreateFlag ;				// キューブマップテクスチャ作成指定フラグ( 1:キューブマップテクスチャを作成する  0:通常テクスチャを作成する )
	int						BlendImageCreateFlag ;					// ブレンド処理用画像作成指定フラグ
	int						UseManagedTextureFlag ;					// マネージドテクスチャを使用するか、フラグ( 1:使用する  0:使用しない )

	int						DrawValidImageCreateFlag ;				// 描画可能画像作成指定フラグ( テクスチャサーフェスのみ )
	int						DrawValidAlphaImageCreateFlag ;			// 描画可能なαチャンネル付き画像作成指定フラグ( テクスチャサーフェスのみ )
	int						DrawValidFloatTypeGraphCreateFlag ;		// 描画可能なFloat型画像作成指定フラグ( テクスチャサーフェスのみ )
	int						DrawValidGraphCreateZBufferFlag ;		// 描画可能画像を作成する際に専用のＺバッファも作成するかどうか
	int						CreateDrawValidGraphChannelNum ;		// 描画可能画像のチャンネル数( テクスチャサーフェスのみ )
	int						CreateDrawValidGraphZBufferBitDepth ;	// 描画可能画像のＺバッファのビット深度( テクスチャサーフェスのみ )
	int						DrawValidMSSamples ;					// 描画可能な画像のマルチサンプリング数
	int						DrawValidMSQuality ;					// 描画可能な画像のマルチサンプリングクオリティ

	int						MipMapCount ;							// 自動で作成するミップマップの数( -1:最大レベルまで作成する )
	int						UserMaxTextureSize ;					// ユーザー指定のテクスチャ最大サイズ
	int						NotUseDivFlag ;							// 画像分割を行わないかどうか( TRUE:行わない  FALSE:行う )
} ;

// シャドウマップハンドルのセットアップに必要なグローバルデータを纏めた構造体
struct SETUP_SHADOWMAPHANDLE_GPARAM
{
	int						Dummy ;
} ;

// 画像データからグラフィックハンドルの作成・画像データの転送に必要なグローバルデータを纏めたもの
struct CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM
{
	SETUP_GRAPHHANDLE_GPARAM InitHandleGParam ;						// グラフィックハンドルのセットアップに必要なグローバルデータ

	int						NotUseTransColor;						// 透過色を使用しないかどうかフラグ(TRUE:使用しない  FALSE:使用する)
	int						UseTransColorGraphCreateFlag ;			// 透過色とそうでない部分の境界部分を滑らかにするか、フラグ
	int						LeftUpColorIsTransColorFlag ;			// 画像左上のピクセル色を透過色にするかどうかのフラグ
} ;

#ifndef DX_NON_MOVIE

// ムービーファイルのオープンに必要なグローバルデータを纏めたもの
struct OPENMOVIE_GPARAM
{
	int						RightAlphaFlag;							// 動画像の右側をアルファとみなす動画ハンドルを作成するか(TRUE:作成する)
	int						A8R8G8B8Flag ;							// 32bitカラーフォーマットの動画を A8R8G8B8 形式として扱うかどうかのフラグ
} ;

#endif // DX_NON_MOVIE

// 画像の元データの情報に必要なグローバルデータを纏めたもの
struct SETGRAPHBASEINFO_GPARAM
{
	int						NotGraphBaseDataBackupFlag ;			// グラフィックハンドルを作成した際に使用した画像データをバックアップしないかどうかのフラグ( TRUE:バックアップしない  FALSE:バックアップする )
} ;

// ファイルからグラフィックハンドルを作成する処理に必要なグローバルデータを纏めたもの
struct LOADGRAPH_GPARAM
{
	LOADBASEIMAGE_GPARAM	LoadBaseImageGParam ;					// 画像データの読み込みに必要なグローバルデータ
#ifndef DX_NON_MOVIE
	OPENMOVIE_GPARAM		OpenMovieGParam ;						// ムービーファイルのオープンに必要なグローバルデータ
#endif
	CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM CreateGraphGParam ;	// グラフィックハンドルの作成・初期化に必要なグローバルデータ
	SETGRAPHBASEINFO_GPARAM	SetGraphBaseInfoGParam ;				// 画像の元データの情報の保存に必要なグローバルデータ
} ;

// グラフィックデータ読み込み引数を纏めたもの
struct LOADGRAPH_PARAM
{
	LOADGRAPH_GPARAM		GParam ;
	int						ReCreateFlag ;
	int						GrHandle ;
	int						BaseHandle ;

	const TCHAR *			FileName ;
	const void *			RgbMemImage ;
	int						RgbMemImageSize ;
	const void *			AlphaMemImage ;
	int						AlphaMemImageSize ;
	const BITMAPINFO *		RgbBmpInfo ;
	void *					RgbBmpImage ;
	const BITMAPINFO *		AlphaBmpInfo ;
	void *					AlphaBmpImage ;
	BASEIMAGE *				RgbBaseImage ;
	BASEIMAGE *				AlphaBaseImage ;

	int						AllNum ;
	int						XNum ;
	int						YNum ;
	int						SizeX ;
	int						SizeY ;
	int *					HandleBuf ;
	int						TextureFlag ;
	int						ReverseFlag ;
	int						SurfaceMode ;
} ;

// ライトハンドル構造体
struct LIGHT_HANDLE
{
	unsigned int			ID ;												// エラーチェック用のＩＤ
	int						Handle ;											// 自身のハンドル値

	LIGHTPARAM				Param ;												// ライトのパラメータ
	int						SetD3DIndex ;										// Direct3D 上のセットされているインデックス( -1:セットされていない )
	int						EnableFlag ;										// 有効フラグ
	int						ChangeFlag ;										// Direct3D に反映していない設定があるかどうかのフラグ( TRUE:ある  FALSE:ない )
	int						ShadowMapSlotDisableFlag[ MAX_USE_SHADOWMAP_NUM ] ;	// シャドウマップを使用しないかどうかのフラグ( TRUE:使用しない  FALSE:使用する )
} ;

// ライト関係情報の構造体
struct LIGHTBASEDATA
{
	int						ProcessDisable ;						// ライト処理を無効にするかどうか
	int						ChangeMaterial ;						// ライト計算用マテリアルが変更したかどうかのフラグ
	MATERIALPARAM			Material ;								// ライト計算用マテリアル
	int						MaterialNotUseVertexDiffuseColor ;		// ライト計算に頂点のディフューズカラーを使用しないかどうか
	int						MaterialNotUseVertexSpecularColor;		// ライト計算に頂点のスペキュラカラーを使用しないかどうか
	LIGHT_HANDLE			*Data[ MAX_LIGHT_NUM ] ;				// ライト情報へのポインタ
	int						Num ;									// ライトの数
	int						Area ;									// 有効なライトがある範囲
	int						HandleID ;								// ライトに割り当てるＩＤ
	int						EnableNum ;								// 有効になっているライトの数
	int						MaxD3DEnableIndex ;						// Direct3DDevice に対して有効になっているライトで一番大きな値のインデックス
	int						EnableD3DIndex[ MAX_D3DENABLELIGHTINDEX_NUM ] ;	// Direct3DDevice に対して有効にしているライトのリスト
	int						D3DChange ;								// Direct3DDevice 側への反映が必要な変更があったかどうかのフラグ
	int						D3DRefresh ;							// Direct3DDevice 側への全項目の反映が必要かどうかのフラグ
	int						DefaultHandle ;							// デフォルトライト用ハンドル
} ;

// グラフィック描画基本データ型
struct GRAPHICSBASEDATA
{
	void					( *GraphRestoreShred )( void ) ;		// グラフィック復元関数のポインタ 

	int						TargetScreen[ DX_RENDERTARGET_COUNT ] ;	// 描画先グラフィック識別値
	int						TargetScreenSurface[ DX_RENDERTARGET_COUNT ] ; // 描画先グラフィック内サーフェスインデックス
	int						TargetScreenVramFlag ;					// 描画先グラフィックがＶＲＡＭに存在するか否か
	int						TargetZBuffer ;							// 描画先Ｚバッファ識別値
	RECT					WindowDrawRect ;						// デスクトップのあるサーフェスに描画処理を行う
																	// 場合ウインドウのクライアント領域の矩形データが入っている
	RECT					DrawArea ;								// 描画可能矩形
	RECTF					DrawAreaF ;								// 描画可能矩形浮動小数点型
	int						DrawSizeX, DrawSizeY ;					// 描画対象のサイズ
	float					Draw3DScale ;							// ３Ｄ描画処理のスケール

	int						ProjectionMatrixMode ;					// 射影行列モード( 0:遠近法  1:正射影  2:行列指定 )
	float					ProjNear, ProjFar ;						// Ｚクリッピングの Near面と Far面
	float					ProjDotAspect ;							// ドットアスペクト比( 縦 / 横 )
	float					ProjFov ;								// 遠近法時の視野角
	float					ProjSize ;								// 正射影時のサイズ
	MATRIX					ProjMatrix ;							// 射影行列

	int						EnableZBufferFlag2D ;					// Ｚバッファの有効フラグ
	int						WriteZBufferFlag2D ;					// Ｚバッファの更新を行うか、フラグ
	int						ZBufferCmpType2D ;						// Ｚ値の比較モード
	int						ZBias2D ;								// Ｚバイアス
	int						EnableZBufferFlag3D ;					// Ｚバッファの有効フラグ
	int						WriteZBufferFlag3D ;					// Ｚバッファの更新を行うか、フラグ
	int						ZBufferCmpType3D ;						// Ｚ値の比較モード
	int						ZBias3D ;								// Ｚバイアス
	int						FillMode ;								// フィルモード
	int						CullMode ;								// カリングモード
	int						TexAddressModeU[ USE_TEXTURESTAGE_NUM ] ;	// テクスチャアドレスモードＵ
	int						TexAddressModeV[ USE_TEXTURESTAGE_NUM ] ;	// テクスチャアドレスモードＶ
	int						TexAddressModeW[ USE_TEXTURESTAGE_NUM ] ;	// テクスチャアドレスモードＷ
	int						FogEnable ;								// フォグが有効かどうか( TRUE:有効  FALSE:無効 )
	int						FogMode ;								// フォグモード
	DWORD					FogColor ;								// フォグカラー
	float					FogStart, FogEnd ;						// フォグ開始アドレスと終了アドレス
	float					FogDensity ;							// フォグ密度
	float					DrawZ;									// ２Ｄ描画時にＺバッファに書き込むＺ値
	int						DrawMode ;								// 描画モード
	int						MaxAnisotropy ;							// 最大異方性
	int						AlphaChDrawMode ;						// 描画先に正しいα値を書き込むかどうか( TRUE:正しい値を書き込む  FALSE:通常モード )
	int						BlendMode ;								// ブレンドモード
	int						BlendParam ;							// ブレンドパラメータ
	int						BlendGraph ;							// ブレンドグラフィックハンドル
	int						BlendGraphType ;						// ブレンドグラフィックタイプ
	int						BlendGraphFadeRatio ;					// ブレンドグラフィックのフェードパラメータ
	int						BlendGraphBorderParam ;					// ブレンドグラフィックハンドルのブレンド境界値(0～255)
	int						BlendGraphBorderRange ;					// ブレンドグラフィックハンドルの境界部分の幅(0～255)
	int						BlendGraphX, BlendGraphY ;				// ブレンドグラフィックの起点座標
	int						AlphaTestMode ;							// アルファテストモード
	int						AlphaTestParam ;						// アルファテストパラメータ
	int						NotUseSpecular ;						// スペキュラを使用しないかどうか
	int						ShadowMap[ MAX_USE_SHADOWMAP_NUM ] ;	// シャドウマップグラフィックハンドル

	union
	{
		RGBCOLOR			DrawBright ;							// 描画輝度
		DWORD				bDrawBright ;
	} ;
	int						IgnoreGraphColorFlag ;					// 描画する画像の色成分を無視するかどうかのフラグ

	int						NotGraphBaseDataBackupFlag ;			// グラフィックハンドルを作成した際に使用した画像データをバックアップしないかどうかのフラグ( TRUE:バックアップしない  FALSE:バックアップする )
	int						CreateImageColorBitDepth ;				// 作成する画像の色深度
	int						CreateImageChannelBitDepth ;			// 作成する画像の１チャンネル辺りのビット深度( こちらが設定されている場合は CreateImageColorBitDepth より優先される )
	int						TextureImageCreateFlag ;				// テクスチャ画像作成フラグ
	int						DrawValidImageCreateFlag ;				// 描画可能画像作成指定フラグ( テクスチャサーフェスのみ )
	int						DrawValidAlphaImageCreateFlag ;			// 描画可能なαチャンネル付き画像作成指定フラグ( テクスチャサーフェスのみ )
	int						DrawValidFloatTypeGraphCreateFlag ;		// 描画可能なFloat型画像作成指定フラグ( テクスチャサーフェスのみ )
	int						NotDrawValidGraphCreateZBufferFlag ;	// 描画可能画像を作成する際に専用のＺバッファは作成しないかどうか
	int						CreateDrawValidGraphChannelNum ;		// 描画可能画像のチャンネル数( テクスチャサーフェスのみ )
	int						CreateDrawValidGraphZBufferBitDepth ;	// 描画可能画像のＺバッファのビット深度( テクスチャサーフェスのみ )
	int						DrawValidMSSamples ;					// 描画可能な画像のマルチサンプリング数
	int						DrawValidMSQuality ;					// 描画可能な画像のマルチサンプリングクオリティ
	int						UseNoBlendModeParam ;					// DX_BLENDMODE_NOBLEND 時でも Param の値を使用するかどうかのフラグ( TRUE:使用する  FALSE:使用しない )
	int						AlphaChannelImageCreateFlag ;			// αチャンネル付き画像作成指定フラグ( DrawValidGraphCreateFlag の方が優先度が高い )
	int						AlphaTestImageCreateFlag ;				// αテスト付き画像作成指定フラグ( AlphaGraphCreateFlag の方が優先度が高い )( テクスチャサーフェスのみ )
	int						CubeMapTextureCreateFlag ;				// キューブマップテクスチャ作成指定フラグ( 1:キューブマップテクスチャを作成する  0:通常テクスチャを作成する )
	int						SystemMemImageCreateFlag ;				// システムメモリを使用する画像作成指定フラグ( 標準サーフェスのみ )
	int						BlendImageCreateFlag ;					// ブレンド処理用画像作成指定フラグ
	int						NotUseManagedTextureFlag ;				// マネージドテクスチャを使用しないか、フラグ( 1:使用しない  0:使用する )
	int						LeftUpColorIsTransColorFlag ;			// 画像左上のピクセル色を透過色にするかどうかのフラグ
	int						NotUseBasicGraphDraw3DDeviceMethodFlag ;// 単純図形の描画に３Ｄデバイスの機能を使用しないかどうかのフラグ
	int						NotWaitVSyncFlag ;						// ＶＳＹＮＣ待ちをしないかどうかのフラグ（TRUE：しない FALSE：する）
	int						PreSetWaitVSyncFlag ;					// DxLib_Init が呼ばれる前に SetWaitVSyncFlag( TRUE ) ; が実行されたかどうかのフラグ( TRUE:実行された  FALSE:実行されていない )
	int						Emulation320x240Flag ;					// 640x480 の画面に 320x240 の画面を出力するかどうかのフラグ

	DWORD					TransColor ;							// 透過色
	int						NotUseTransColor;						// 透過色を使用しないかどうかフラグ(TRUE:使用しない  FALSE:使用する)
	int						UseTransColorGraphCreateFlag ;			// 透過色とそうでない部分の境界部分を滑らかにするか、フラグ

	int						NotDrawFlag ;							// 描画不可能フラグ
	int						NotDrawFlagInSetDrawArea ;				// 描画不可能フラグ（SetDrawArea用）

	int						ScreenFlipFlag ;						// ScreenFlip関数を呼びだし中フラグ

	int						Screen3DWidth, Screen3DHeight ;			// ３Ｄ描画を行う際の想定するスクリーンのサイズ
	int						Screen3DCenterX, Screen3DCenterY ;		// ３Ｄ描画を行う際の想定するスクリーンの中心座標
	LONGLONG				FrameCounter ;							// フレームカウンター

	int						MatchHardwareMatrix ;					// ３Ｄデバイスに設定されている行列と本構造体中の行列が一致しているかどうか( TRUE:一致している  FALSE:一致していない )
	int						MatchHardwareWorldMatrix ;				// ３Ｄデバイスに設定されているワールド変換行列と本構造体中のワールド変換行列が一致しているかどうか( TRUE:一致している  FALSE:一致していない )
	MATRIX					WorldMatrix ;							// ワールド変換行列
	int						MatchHardwareViewMatrix ;				// ３Ｄデバイスに設定されているビュー変換行列と本構造体中のビュー変換行列が一致しているかどうか( TRUE:一致している  FALSE:一致していない )
	MATRIX					ViewMatrix ;							// ビュー変換行列
	int						MatchHardwareProjectionMatrix ;			// ３Ｄデバイスに設定されている射影変換行列と本構造体中の射影変換行列が一致しているかどうか( TRUE:一致している  FALSE:一致していない )
	MATRIX					ProjectionMatrix ;						// 射影変換行列
	MATRIX					ViewportMatrix ;						// ビューポート行列
	MATRIX					Direct3DViewportMatrix ;				// Ｄｉｒｅｃｔ３Ｄ 的なビューポート行列
	MATRIX					Direct3DViewportMatrixAnti ;			// Ｄｉｒｅｃｔ３Ｄ 的なビューポート行列を無効にする行列
	int						TextureTransformUse ;					// テクスチャ座標変換処理を行うかどうか( TRUE:行う  FALSE:行わない )
	float					TextureTransX, TextureTransY ;			// テクスチャ平行移動
	float					TextureScaleX, TextureScaleY ;			// テクスチャ拡大率
	float					TextureRotateCenterX, TextureRotateCenterY ;	// テクスチャ回転の中心座標
	float					TextureRotate ;							// テクスチャ回転値
	int						TextureMatrixValid ;					// TextureMatrix が有効かどうか( TRUE:有効  FALSE:無効 )
	MATRIX					TextureMatrix ;							// TextureTransX や TextureScaleY などを使用しないで設定する行列
	MATRIX					TextureTransformMatrix ;				// テクスチャ座標変換行列
	int						TempVertexBufferSize ;					// 一時的に頂点データを格納するためのバッファのサイズ
	void					*TempVertexBuffer ;						// 一時的に頂点データを格納するためのバッファ

	int						ValidBlendMatrix ;						// BlendMatrix, ViewProjectionMatrix, BillboardMatrix が有効かどうか( TRUE:有効  FALSE:無効 )
	MATRIX					BlendMatrix ;							// ワールド変換、ビュー変換、射影変換、ビューポート変換行列を掛け合わせたもの
	int						ValidInverseBlendMatrix ;				// BlendMatrix の逆行列が有効かどうか( TRUE:有効  FALSE:無効 )
	MATRIX					InverseBlendMatrix ;					// BlendMatrix の逆行列
	MATRIX					ViewProjectionViewportMatrix ;			// ビュー行列と射影行列とビューポート行列を掛け合わせたもの
	MATRIX					BillboardMatrix ;						// ビルボード用の行列
	VECTOR					ViewClipPos[ 2 ][ 2 ][ 2 ] ;			// 視錐台頂点( [ 0:z+ 1:z- ][ 0:top 1:bottom ][ 0:left 1:right ] )
	FLOAT4					ClippingPlane[ 6 ] ;					// クリッピング平面( 0:-x 1:+x 2:-y 3:+y 4:-z 5:+z )
	MATRIX					ConvScreenPosToWorldPosMatrix ;			// 画面座標からスクリーン座標に変換する際に使用する行列
	int						ValidConvScreenPosToWorldPosMatrix ;	// 画面座標からスクリーン座標に変換する際に使用する行列が有効かどうか( TRUE:有効  FALSE:無効 )

	LIGHTBASEDATA			Light ;									// ライト関係の情報

	VECTOR					CameraPosition ;						// カメラの位置
	VECTOR					CameraTarget ;							// カメラの注視点
	VECTOR					CameraUp ;								// カメラのアップベクトル
	float					CameraHRotate ;							// カメラの水平角度
	float					CameraVRotate ;							// カメラの垂直角度
	float					CameraTRotate ;							// カメラの捻り角度
	MATRIX					CameraMatrix ;							// ビュー行列
	float					CameraScreenCenterX, CameraScreenCenterY ;	// カメラの消失点
} ;

// 内部大域変数宣言 --------------------------------------------------------------

// 描画周りの基本的な情報
extern GRAPHICSBASEDATA GraphicsBaseData ;

// 関数プロトタイプ宣言-----------------------------------------------------------

extern	int		InitializeGraphicsBase( void ) ;											// グラフィック描画基本データの初期化
extern	int		CheckCameraViewClip_Box_PosDim( VECTOR *CheckBoxPos ) ;						// ８座標で形作るボックスがカメラの視界に入っているかどうかを判定する( 戻り値 TRUE:視界に入っていない  FALSE:視界に入っている )( CheckPosは VECTOR 8個分の配列の先頭アドレス、配列の各要素番号の内容 0:+x +y +z   1:-x +y +z   2:-x -y +z   3:+x -y +z   4:+x -y -z   5:+x +y -z   6:-x +y -z   7:-x -y -z )


extern	int		MakeGraph_UseGParam( SETUP_GRAPHHANDLE_GPARAM *GParam, int SizeX, int SizeY, int NotUse3DFlag, int ASyncLoadFlag = FALSE, int ASyncThread = FALSE ) ;																																											// 空のグラフィックハンドルを作成する関数
extern	int		MakeShadowMap_UseGParam( SETUP_SHADOWMAPHANDLE_GPARAM *GParam, int SizeX, int SizeY, int ASyncLoadFlag = FALSE, int ASyncThread = FALSE ) ;																																														// シャドウマップハンドルを作成する
extern	int		CreateGraph_UseGParam(                  LOADGRAPH_PARAM *Param, int ASyncLoadFlag = FALSE, int ASyncThread = FALSE ) ;																																																			// 画像データからグラフィックハンドルを作成する関数
extern	int		CreateDivGraph_UseGParam(               LOADGRAPH_PARAM *Param, int ASyncLoadFlag = FALSE ) ;																																																									// 画像データを分割してグラフィックハンドルを作成する関数
extern	int		LoadBmpToGraph_UseGParam(               LOADGRAPH_GPARAM *GParam, int ReCreateFlag, int GrHandle, const TCHAR *GraphName, int TextureFlag, int ReverseFlag, int SurfaceMode = DX_MOVIESURFACE_NORMAL, int ASyncLoadFlag = FALSE ) ;																								// LoadBmpToGraph のグローバル変数にアクセスしないバージョン
extern	int		LoadDivBmpToGraph_UseGParam(            LOADGRAPH_GPARAM *GParam, int ReCreateFlag, const TCHAR *FileName, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag, int ASyncLoadFlag = FALSE ) ;																				// LoadDivBmpToGraph のグローバル変数にアクセスしないバージョン
extern	int		CreateGraphFromMem_UseGParam(           LOADGRAPH_GPARAM *GParam, int ReCreateFlag, int GrHandle, const void *MemImage, int MemImageSize, const void *AlphaImage = NULL, int AlphaImageSize = 0, int TextureFlag = TRUE, int ReverseFlag = FALSE, int ASyncLoadFlag = FALSE ) ;																// CreateGraphFromMem のグローバル変数にアクセスしないバージョン
extern	int		CreateDivGraphFromMem_UseGParam(        LOADGRAPH_GPARAM *GParam, int ReCreateFlag, const void *MemImage, int MemImageSize, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag, const void *AlphaImage, int AlphaImageSize, int ASyncLoadFlag = FALSE ) ;					// CreateDivGraphFromMem のグローバル変数にアクセスしないバージョン
extern	int		CreateGraphFromBmp_UseGParam(           LOADGRAPH_GPARAM *GParam, int ReCreateFlag, int GrHandle, BITMAPINFO *BmpInfo, void *GraphData, BITMAPINFO *AlphaInfo = NULL, void *AlphaData = NULL, int TextureFlag = TRUE, int ReverseFlag = FALSE, int ASyncLoadFlag = FALSE ) ;													// CreateGraphFromBmp のグローバル変数にアクセスしないバージョン
extern	int		CreateDivGraphFromBmp_UseGParam(        LOADGRAPH_GPARAM *GParam, int ReCreateFlag, const BITMAPINFO *BmpInfo, const void *GraphData, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf,int TextureFlag, int ReverseFlag, const BITMAPINFO *AlphaInfo, const void *AlphaData, int ASyncLoadFlag = FALSE ) ;	// CreateDivGraphFromBmp のグローバル変数にアクセスしないバージョン
extern	int		CreateGraphFromGraphImage_UseGParam(    LOADGRAPH_GPARAM *GParam, int ReCreateFlag, int GrHandle, const BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int TextureFlag = TRUE , int ReverseFlag = FALSE, int ASyncLoadFlag = FALSE, int ASyncThread = FALSE ) ;																	// CreateGraphFromGraphImage のグローバル変数にアクセスしないバージョン
extern	int		CreateDivGraphFromGraphImage_UseGParam( LOADGRAPH_GPARAM *GParam, int ReCreateFlag, const BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag = TRUE , int ReverseFlag = FALSE, int ASyncLoadFlag = FALSE ) ;									// CreateDivGraphFromGraphImage のグローバル変数にアクセスしないバージョン

extern	int		CreateGraphFromGraphImageBase_UseGParam(    CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam, int ReCreateFlag, int GrHandle,   BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int TextureFlag, int ASyncThread = FALSE ) ;																							// CreateGraphFromGraphImageBase のグローバル変数にアクセスしないバージョン
extern	int		CreateDivGraphFromGraphImageBase_UseGParam( CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam, int ReCreateFlag, int BaseHandle, BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag, int ASyncThread = FALSE ) ;		// CreateDivGraphFromGraphImageBase のグローバル変数にアクセスしないバージョン


extern	int		ResetFrameCounter( void ) ;													// フレームカウンターを初期化する
extern	int		IncFrameCounter( void ) ;													// フレームカウンターをインクリメントする
extern	LONGLONG GetFrameCounter( void ) ;													// フレームカウンターの値を取得する
extern	int		RefreshLightState( void ) ;													// ライトの変更を反映する
extern	void	RefreshProjectionMatrix( void ) ;											// 射影行列パラメータに基づいて射影行列を構築する
extern	void	RefreshBlendTransformMatrix( void ) ;										// 頂点変換行列を掛け合わせた行列を更新する
extern	void	SetupDisplayModeData( int *Num, DISPLAYMODEDATA **Buffer ) ;				// ディスプレイモードデータの情報をセットアップする

extern	int		CreateGraphFromGraphImageBase(      BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int TextureFlag ) ;																														// CreateGraphFromGraphImage の内部関数
extern	int		CreateDivGraphFromGraphImageBase(   BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag, int ReverseFlag ) ;							// CreateDivGraphFromGraphImage の内部関数
extern	int		ReCreateGraphFromGraphImageBase(    BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int GrHandle, int TextureFlag ) ;																										// ReCreateGraphFromGraphImage の内部関数
extern	int		ReCreateDivGraphFromGraphImageBase( BASEIMAGE *Image, const BASEIMAGE *AlphaImage, int AllNum, int XNum, int YNum, int SizeX, int SizeY, int *HandleBuf, int TextureFlag , int ReverseFlag ) ;						// ReCreateDivGraphFromGraphImage の内部関数

extern	void	InitCreateGraphHandleAndBltGraphImageGParam( CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM *GParam ) ;			// CREATE_GRAPHHANDLE_AND_BLTGRAPHIMAGE_GPARAM のデータをセットする
extern	void	InitSetupGraphHandleGParam( SETUP_GRAPHHANDLE_GPARAM *GParam ) ;												// SETUP_GRAPHHANDLE_GPARAM のデータをセットする
extern	void	InitSetupGraphHandleGParam_Normal_NonDrawValid( SETUP_GRAPHHANDLE_GPARAM *GParam, int BitDepth = 32, int AlphaChannel = TRUE, int AlphaTest = FALSE ) ;
extern	void	InitSetupGraphHandleGParam_Normal_DrawValid_NoneZBuffer( SETUP_GRAPHHANDLE_GPARAM *GParam, int BitDepth = 32, int AlphaChannel = TRUE ) ;

#ifndef DX_NON_DSHOW_MOVIE
#ifndef DX_NON_MOVIE
extern	void	InitOpenMovieGParam( OPENMOVIE_GPARAM *GParam ) ;																// OPENMOVIE_GPARAM のデータをセットする
#endif
#endif
extern	void	InitSetGraphBaseInfoGParam( SETGRAPHBASEINFO_GPARAM *GParam ) ;													// SETGRAPHBASEINFO_GPARAM のデータをセットする
extern	void	InitLoadGraphGParam( LOADGRAPH_GPARAM *GParam ) ;																// LOADGRAPH_GPARAM のデータをセットする

}

#endif // __DXGRAPHICSBASE_H__
