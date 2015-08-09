﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		モデルデータ制御プログラム
// 
// 				Ver 3.14d
// 
// -------------------------------------------------------------------------------

#define __DX_MAKE
//#define DXLIB_DEBUG
//#define NOT_RENDER

#include "DxModel.h"

#ifndef DX_NON_MODEL

// インクルード ---------------------------------
#include "DxModelFile.h"
#include "DxModelRead.h"

#include "DxModelLoader3.h"
#include "DxFile.h"
#include "DxFont.h"
#include "DxLog.h"

#include "DxBaseFunc.h"
#include "DxMemory.h"
#include "DxSystem.h"
#include "DxMovie.h"
#include "DxMask.h"
#include "DxBaseImage.h"
#include "DxMath.h"
#include "DxGraphics.h"
#include "DxASyncLoad.h"

#ifdef __WINDOWS__
#include "Windows/DxModelWin.h"
#endif // __WINDOWS__

#ifdef __PSVITA
#include "PSVita/DxModelPSVita.h"
#endif // __PSVITA

#ifdef __PS4
#include "PS4/DxModelPS4.h"
#endif // __PS4

#include <math.h>

#ifdef DXLIB_DEBUG
#include "DxShaderPath.h"
#endif

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 -----------------------------------


// モデルデータだけを扱う関数の冒頭共通文
#define MV1START( MODELHAND, MODEL, MODELBASE, ERR )\
	MV1_MODEL *MODEL ;\
	MV1_MODEL_BASE *MODELBASE ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1MDLCHK( MODELHAND, MODEL ) )\
		return ERR ;\
	MODELBASE = MODEL->BaseData ;



// フレームを扱う関数の冒頭共通文
#define MV1FRAMESTART( MODELHAND, MODEL, MODELBASE, FRAME, FRAMEIND, ERR )\
	MV1_MODEL *MODEL ;\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_FRAME *FRAME ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1MDLCHK( MODELHAND, MODEL ) )\
		return ERR ;\
	MODELBASE = MODEL->BaseData ;\
\
	/* インデックスが不正だったらエラー */\
	if( FRAMEIND < 0 || FRAMEIND >= MODELBASE->FrameNum )\
		return ERR ;\
	FRAME = &MODEL->Frame[ FRAMEIND ] ;


// メッシュを扱う関数の冒頭共通文
#define MV1MESHSTART( MODELHAND, MODEL, MODELBASE, MESH, MESHIND, ERR )\
	MV1_MODEL *MODEL ;\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_MESH *MESH ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1MDLCHK( MODELHAND, MODEL ) )\
		return ERR ;\
	MODELBASE = MODEL->BaseData ;\
\
	/* インデックスが不正だったらエラー */\
	if( MESHIND < 0 || MESHIND >= MODELBASE->MeshNum )\
		return ERR ;\
	MESH = &MODEL->Mesh[ MESHIND ] ;


// シェイプを扱う関数の冒頭共通文
#define MV1SHAPESTART( MODELHAND, MODEL, MODELBASE, SHAPE, SHAPEIND, ERR )\
	MV1_MODEL *MODEL ;\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_SHAPE *SHAPE ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1MDLCHK( MODELHAND, MODEL ) )\
		return ERR ;\
	MODELBASE = MODEL->BaseData ;\
\
	/* インデックスが不正だったらエラー */\
	if( SHAPEIND < 0 || SHAPEIND >= MODELBASE->ShapeNum )\
		return ERR ;\
	SHAPE = &MODEL->Shape[ SHAPEIND ] ;


// トライアングルリストを扱う関数の冒頭共通文
#define MV1TLISTSTART( MODELHAND, MODEL, MODELBASE, TLIST, TLISTIND, ERR )\
	MV1_MODEL *MODEL ;\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_TRIANGLE_LIST_BASE *TLIST ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1MDLCHK( MODELHAND, MODEL ) )\
		return ERR ;\
	MODELBASE = MODEL->BaseData ;\
\
	/* インデックスが不正だったらエラー */\
	if( TLISTIND < 0 || TLISTIND >= MODELBASE->TriangleNum )\
		return ERR ;\
	TLIST = &MODELBASE->TriangleList[ TLISTIND ] ;


// マテリアルを扱う関数の冒頭共通文
#define MV1MATERIALSTART( MODELHAND, MODEL, MODELBASE, MATERIAL, MATERIALIND, ERR )\
	MV1_MODEL *MODEL ;\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_MATERIAL *MATERIAL ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1MDLCHK( MODELHAND, MODEL ) )\
		return ERR ;\
	MODELBASE = MODEL->BaseData ;\
\
	/* インデックスが不正だったらエラー */\
	if( MATERIALIND < 0 || MATERIALIND >= MODELBASE->MaterialNum )\
		return ERR ;\
	MATERIAL = &MODEL->Material[ MATERIALIND ] ;


// テクスチャを扱う関数の冒頭共通文
#define MV1TEXTURESTART( MODELHAND, MODEL, MODELBASE, TEXTURE, TEXTUREIND, ERR )\
	MV1_MODEL *MODEL ;\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_TEXTURE *TEXTURE ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1MDLCHK( MODELHAND, MODEL ) )\
		return ERR ;\
	MODELBASE = MODEL->BaseData ;\
\
	/* インデックスが不正だったらエラー */\
	if( TEXTUREIND < 0 || TEXTUREIND >= MODELBASE->TextureNum )\
		return ERR ;\
	TEXTURE = &MODEL->Texture[ TEXTUREIND ] ;


// 基本データのマテリアルを扱う関数の冒頭共通文
#define MV1BASEMATERIALSTART( MODELBASEHAND, MODELBASE, MATERIAL, MATERIALIND, ERR )\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_MATERIAL_BASE *MATERIAL ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1BMDLCHK( MODELBASEHAND, MODELBASE ) )\
		return ERR ;\
\
	/* インデックスが不正だったらエラー */\
	if( MATERIALIND < 0 || MATERIALIND >= MODELBASE->MaterialNum )\
		return ERR ;\
	MATERIAL = &MODELBASE->Material[ MATERIALIND ] ;



// 基本データのテクスチャを扱う関数の冒頭共通文
#define MV1BASETEXTURELSTART( MODELBASEHAND, MODELBASE, TEXTURE, TEXTUREIND, ERR )\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_TEXTURE_BASE *TEXTURE ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1BMDLCHK( MODELBASEHAND, MODELBASE ) )\
		return ERR ;\
\
	/* インデックスが不正だったらエラー */\
	if( TEXTUREIND < 0 || TEXTUREIND >= MODELBASE->TextureNum )\
		return ERR ;\
	TEXTURE = &MODELBASE->Texture[ TEXTUREIND ] ;



// 基本データのメッシュを扱う関数の冒頭共通文
#define MV1BASEMESHSTART( MODELBASEHAND, MODELBASE, MESH, MESHIND, ERR )\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_MESH_BASE *MESH ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1BMDLCHK( MODELBASEHAND, MODELBASE ) )\
		return ERR ;\
\
	/* インデックスが不正だったらエラー */\
	if( MESHIND < 0 || MESHIND >= MODELBASE->MeshNum )\
		return ERR ;\
	MESH = &MODELBASE->Mesh[ MESHIND ] ;



// 基本データのシェイプを扱う関数の冒頭共通文
#define MV1BASESHAPESTART( MODELBASEHAND, MODELBASE, SHAPE, SHAPEIND, ERR )\
	MV1_MODEL_BASE *MODELBASE ;\
	MV1_SHAPE_BASE *SHAPE ;\
\
	/* 初期化されていなかったらエラー */\
	if( MV1Man.Initialize == false )\
		return ERR ;\
\
	/* アドレス取得 */\
	if( MV1BMDLCHK( MODELBASEHAND, MODELBASE ) )\
		return ERR ;\
\
	/* インデックスが不正だったらエラー */\
	if( SHAPEIND < 0 || SHAPEIND >= MODELBASE->ShapeNum )\
		return ERR ;\
	SHAPE = &MODELBASE->Shape[ SHAPEIND ] ;


// メッシュの半透明ステータスセットアップ完了フラグが立っていたら倒す
#define MV1MESH_RESET_SEMITRANSSETUP( MESH )\
	if( (MESH)->SemiTransStateSetupFlag )\
	{\
		MV1_FRAME *Frame ;\
		(MESH)->SemiTransStateSetupFlag = false ;\
		for( Frame = MESH->Container ; Frame ; Frame = Frame->Parent )\
			Frame->SemiTransStateSetupFlag = false ;\
		Mesh->Container->Container->SemiTransStateSetupFlag = false ;\
	}


// フレームの半透明ステータスセットアップ完了フラグが立っていたら倒す
#define MV1FRAME_RESET_SEMITRANSSETUP( FRAME )\
	if( (FRAME)->SemiTransStateSetupFlag )\
	{\
		MV1_FRAME *_Frame ;\
		(FRAME)->SemiTransStateSetupFlag = false ;\
		for( _Frame = (FRAME)->Parent ; _Frame ; _Frame = _Frame->Parent )\
			_Frame->SemiTransStateSetupFlag = false ;\
		(FRAME)->Container->SemiTransStateSetupFlag = false ;\
	}



// 更新が必要かチェック( 必要なら 0 以外 )
#define MV1CCHK( CHANGE )				( *( CHANGE ).Target & ( CHANGE ).BaseData->CheckBit )

// 自分のビットをリセットする
#define MV1CRST( CHANGE )				( *( CHANGE ).Target &= ~( CHANGE ).BaseData->CheckBit )

// モデルのルート行列を更新する必要がある場合に更新をする
#define MV1SETUPMATRIX( MODEL )	\
	if( ( MODEL )->LocalWorldMatrixSetupFlag == false )\
		MV1SetupMatrix( ( MODEL ) ) ;

// メッシュの描画マテリアル情報を更新する必要がある場合に更新をする
#define MV1SETUPDRAWMATERIALMESH( MESH )	\
	if( MV1CCHK( ( MESH )->DrawMaterialChange ) )	\
		MV1SetupDrawMaterial( NULL, ( MESH ) ) ;

// フレームの描画マテリアル情報を更新する必要がある場合に更新をする
#define MV1SETUPDRAWMATERIALFRAME( FRAME )	\
	if( MV1CCHK( ( FRAME )->DrawMaterialChange ) )	\
		MV1SetupDrawMaterial( ( FRAME ), NULL ) ;

// データ型宣言 ---------------------------------

// データ宣言 -----------------------------------

extern BYTE Tga256x8ToonTextureFileImage[ 172 ] ;
extern BYTE TgaSpecularDefaultGradFileImage[ 172 /*124*/ ] ;
extern BYTE TgaDiffuseDefaultGradFileImage[ 172 /*156*/ ] ;
extern BYTE Tga8x8TextureFileImage[ 84 ] ;
extern BYTE Tga8x8BlackTextureFileImage[ 76 ] ;




MV1_MODEL_MANAGE MV1Man;

// 関数宣言 -------------------------------------

static  int					_MV1GetAnimKeyDataIndexFromTime( MV1_ANIM_KEYSET_BASE *AnimKeySetBase, float Time, float &Rate ) ;		// 指定の時間を超える一番小さい番号のキーのインデックスを取得する
static	int					_MV1AnimSetSyncNowKey( MV1_ANIMSET *AnimSet, bool AboutSetting = false ) ;						// 現在のアニメーション再生時間に各キーの NowKey の値を合わせる
static	MV1_ANIMSET_BASE	*MV1GetAnimSetBase( int MV1ModelHandle, const wchar_t *Name = NULL, int Index = -1 ) ;				// 名前やインデックスからモデル基本データ内のアニメーションを取得する
static	MV1_ANIMSET			*MV1CreateAnimSet( MV1_ANIMSET_BASE *MV1AnimSetBase ) ;											// アニメーションセット基本データから実行用アニメーションセットを作成する
static	MV1_ANIM			*MV1GetAnimSetAnim( MV1_ANIMSET *AnimSet, const wchar_t *Name = NULL, int Index = -1 ) ;			// アニメーションセットから特定の名前、若しくはインデックスのアニメーションを得る
//static	int					MV1PlayAnimSet( MV1_ANIMSET *AnimSet, int Loop ) ;											// アニメーションを再生する
static	int					MV1SetAnimSetTime( MV1_ANIMSET *AnimSet, float Time ) ;											// アニメーションを指定の時間に設定する

//static	int					MV1AnimSetAddTime( MV1_ANIMSET *AnimSet, float AddTime ) ;									// アニメーションを進める
//static	int					MV1StopAnimSet( MV1_ANIMSET *AnimSet ) ;													// アニメーションを止める
//static	int					MV1GetAnimSetState( MV1_ANIMSET *AnimSet ) ;												// アニメーションが再生中かどうかを取得する( TRUE:再生中  FALSE:停止中 )
static	int					MV1AnimSetSetupParam( MV1_ANIMSET *AnimSet ) ;													// アニメーションの現在の再生経過時間に合わせたパラメータを計算する
static	void				MV1SetupTransformMatrix( MATRIX_4X4CT_F * RST BlendMatrix, int ValidFlag, VECTOR * RST Translate, VECTOR * RST Scale, int RotateOrder, VECTOR * RST PreRotate, VECTOR * RST Rotate, VECTOR * RST PostRotate, FLOAT4 * RST Quaternion ) ;	// 座標変換情報を使用して座標変換行列を作成する
static	int					MV1SetupReferenceMeshFrame( MV1_MODEL *Model, MV1_MODEL_BASE *ModelBase, MV1_FRAME *Frame, MV1_REF_POLYGONLIST *DestBuffer, int VIndexTarget, bool IsTransform, bool IsPositionOnly ) ;	// 参照用メッシュのセットアップを行う
static	int					MV1RefreshReferenceMeshFrame( MV1_FRAME *Frame, int IsPositionOnly, MV1_REF_POLYGONLIST *DestBuffer ) ;	// 参照用メッシュのリフレッシュを行う
static	void				MV1SetupAnimMatrix( MV1_MODEL *Model ) ;														// アニメーションの行列をセットアップする
static	void				MV1SetupMatrix( MV1_MODEL *Model ) ;															// 描画用の行列を構築する
static	void				MV1SetupDrawMaterial( MV1_FRAME *Frame = NULL, MV1_MESH *Mesh = NULL ) ;						// 描画用のマテリアル情報を構築する、Frame を指定した場合は Mesh は NULLで、Mesh を指定した場合は Frame は NULLである必要があります( 中で渡されたメッシュ自体の更新チェックはしない )
static	void				MV1BitSetChange( MV1_CHANGE *Change ) ;															// 状態変更管理データに設定されている対象ビットを立てる
static	void				MV1BitResetChange( MV1_CHANGE *Change ) ;														// 状態変更管理データに設定されている対象ビットを倒す
static	bool				MV1SetDrawMaterialDif( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale ) ;	// 描画用マテリアル情報のディフューズカラーを変更する
static	bool				MV1SetDrawMaterialAmb( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale ) ;	// 描画用マテリアル情報のアンビエントカラーを変更する
static	bool				MV1SetDrawMaterialSpc( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale ) ;	// 描画用マテリアル情報のスペキュラカラーを変更する
static	bool				MV1SetDrawMaterialEmi( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale ) ;	// 描画用マテリアル情報のエミッシブカラーを変更する
static	bool				MV1SetDrawMaterialOpacityRate( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, float Rate) ;	// 描画用マテリアル情報の不透明度を変更する
static	bool				MV1SetDrawMaterialVisible( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, BYTE Visible ) ;	// 描画用マテリアル情報の表示設定を変更する
static	int					MV1DrawModelBase( MV1_MODEL *Model ) ;															// モデルの描画処理を行う
static	int					_MV1ReCreateGradationGraph( int GrHandle ) ;													// グラデーション画像を再作成する
static	int					_MV1ReMakeNormalFrameBase( MV1_FRAME_BASE *Frame, float SmoothingAngle ) ;						// フレームの法線を再計算する
static	int					_MV1PositionOptimizeFrameBase( MV1_FRAME_BASE *Frame ) ;										// フレームの座標情報を最適化する
static	int					_MV1SetupCombineMesh( MV1_FRAME_BASE *Frame ) ;													// コンバインメッシュのセットアップを行う
static	void				_MV1SetupReferenceMeshMaxAndMinPosition( MV1_REF_POLYGONLIST *PolyList ) ;						// 参照用メッシュの最大値と最小値を更新する
static	bool				_MV1CreateWideCharNameBase( MV1_MODEL_BASE *MBase, const char *NameA, wchar_t **NameWP ) ;		// マルチバイト文字名からワイド文字名を作成する
static	bool				_MV1CreateMultiByteNameBase( MV1_MODEL_BASE *MBase, const wchar_t *NameW, char **NameAP ) ;		// ワイド文字名からマルチバイト文字名を作成する
static	bool				_MV1CreateWideCharName( const char *NameA, wchar_t **NameWP ) ;									// マルチバイト文字名からワイド文字名を作成する
static	bool				_MV1CreateMultiByteName( const wchar_t *NameW, char **NameAP ) ;								// ワイド文字名からマルチバイト文字名を作成する
static	bool				_MV1AllocAndMultiByteNameCopy( const char *NameA, char **NameAP ) ;								// マルチバイト文字名を新たにメモリを確保してコピーする
static	bool				_MV1AllocAndWideCharNameCopy( const wchar_t *NameW, wchar_t **NameWP ) ;						// ワイド文字名を新たにメモリを確保してコピーする
static	void				_MV1CreateFileNameAndCurrentDirectory( const wchar_t *FilePath, wchar_t *FileName, wchar_t *CurrentDirectory ) ;	// ファイルパスからファイル名とディレクトリパスを作成する

// テクスチャを読み込む

static	int					_MV1TextureLoadBase(
									MV1_MODEL_BASE *ModelBase, MV1_TEXTURE_BASE *Texture,
									const wchar_t *ColorFilePathW, const wchar_t *AlphaFilePathW,
									int BumpImageFlag, float BumpImageNextPixelLength,
									bool ReverseFlag,
									bool Bmp32AllZeroAlphaToXRGB8Flag,
									int ASyncThread ) ;
static	int					_MV1TextureLoad(
									MV1_MODEL_BASE *ModelBase, MV1_TEXTURE *Texture,
									const wchar_t *ColorFilePathW, const wchar_t *AlphaFilePathW,
									int BumpImageFlag, float BumpImageNextPixelLength,
									bool ReverseFlag,
									bool Bmp32AllZeroAlphaToXRGB8Flag,
									int ASyncThread ) ;



// プログラム -----------------------------------

static __inline int DrawMeshListResize( int RequestSize )
{
	if( RequestSize > MV1Man.DrawMeshListSize )
	{
		if( MV1Man.DrawMeshList )
		{
			DXFREE( MV1Man.DrawMeshList ) ;
		}
		MV1Man.DrawMeshList = ( MV1_MESH ** )DXALLOC( RequestSize * 3 * sizeof( MV1_MESH * ) ) ;
		if( MV1Man.DrawMeshList == NULL ) return -1 ;
		MV1Man.DrawMeshListSize = RequestSize ;
	}
	return 0 ;
}

static __inline void TableSinCos( float Angle, float * RST Sin, float * RST Cos )
{
	if( Angle > 10.0f || Angle < -10.0f )
	{
		_SINCOS( Angle, Sin, Cos ) ;
	}
	else
	{
		Angle = Angle * ( MV1_SINTABLE_DIV / ( ( float )DX_PI * 2 ) ) + 12582912.0f ;
		*Sin = MV1Man.SinTable[ *( ( DWORD * )&Angle ) & ( MV1_SINTABLE_DIV - 1 ) ] ;
		*Cos = MV1Man.SinTable[ ( *( ( DWORD * )&Angle ) + MV1_SINTABLE_DIV / 4 ) & ( MV1_SINTABLE_DIV - 1 ) ] ;
	}
}

// クォータニオンから回転行列を作成する
static __inline void CreateQuaternionRotateMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, FLOAT4 * RST In )
{
	MATRIX_D RotateQ1Mat, RotateQ2Mat, RotateMat ;

	RotateQ1Mat.m[0][0] =  In->w ; RotateQ1Mat.m[0][1] = -In->z ; RotateQ1Mat.m[0][2] =  In->y ; RotateQ1Mat.m[0][3] =  In->x ;
	RotateQ1Mat.m[1][0] =  In->z ; RotateQ1Mat.m[1][1] =  In->w ; RotateQ1Mat.m[1][2] = -In->x ; RotateQ1Mat.m[1][3] =  In->y ;
	RotateQ1Mat.m[2][0] = -In->y ; RotateQ1Mat.m[2][1] =  In->x ; RotateQ1Mat.m[2][2] =  In->w ; RotateQ1Mat.m[2][3] =  In->z ;
	RotateQ1Mat.m[3][0] = -In->x ; RotateQ1Mat.m[3][1] = -In->y ; RotateQ1Mat.m[3][2] = -In->z ; RotateQ1Mat.m[3][3] =  In->w ;

	RotateQ2Mat.m[0][0] =  In->w ; RotateQ2Mat.m[0][1] = -In->z ; RotateQ2Mat.m[0][2] =  In->y ; RotateQ2Mat.m[0][3] = -In->x ;
	RotateQ2Mat.m[1][0] =  In->z ; RotateQ2Mat.m[1][1] =  In->w ; RotateQ2Mat.m[1][2] = -In->x ; RotateQ2Mat.m[1][3] = -In->y ;
	RotateQ2Mat.m[2][0] = -In->y ; RotateQ2Mat.m[2][1] =  In->x ; RotateQ2Mat.m[2][2] =  In->w ; RotateQ2Mat.m[2][3] = -In->z ;
	RotateQ2Mat.m[3][0] =  In->x ; RotateQ2Mat.m[3][1] =  In->y ; RotateQ2Mat.m[3][2] =  In->z ; RotateQ2Mat.m[3][3] =  In->w ;

	CreateMultiplyMatrixD( &RotateMat, &RotateQ2Mat, &RotateQ1Mat ) ;
	ConvertMatrixDToMatrix4x4cD( Out, &RotateMat ) ;
}

static __inline void CreateQuaternionRotateMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, FLOAT4 * RST In )
{
	MATRIX RotateQ1Mat, RotateQ2Mat, RotateMat ;

	RotateQ1Mat.m[0][0] =  In->w ; RotateQ1Mat.m[0][1] = -In->z ; RotateQ1Mat.m[0][2] =  In->y ; RotateQ1Mat.m[0][3] =  In->x ;
	RotateQ1Mat.m[1][0] =  In->z ; RotateQ1Mat.m[1][1] =  In->w ; RotateQ1Mat.m[1][2] = -In->x ; RotateQ1Mat.m[1][3] =  In->y ;
	RotateQ1Mat.m[2][0] = -In->y ; RotateQ1Mat.m[2][1] =  In->x ; RotateQ1Mat.m[2][2] =  In->w ; RotateQ1Mat.m[2][3] =  In->z ;
	RotateQ1Mat.m[3][0] = -In->x ; RotateQ1Mat.m[3][1] = -In->y ; RotateQ1Mat.m[3][2] = -In->z ; RotateQ1Mat.m[3][3] =  In->w ;

	RotateQ2Mat.m[0][0] =  In->w ; RotateQ2Mat.m[0][1] = -In->z ; RotateQ2Mat.m[0][2] =  In->y ; RotateQ2Mat.m[0][3] = -In->x ;
	RotateQ2Mat.m[1][0] =  In->z ; RotateQ2Mat.m[1][1] =  In->w ; RotateQ2Mat.m[1][2] = -In->x ; RotateQ2Mat.m[1][3] = -In->y ;
	RotateQ2Mat.m[2][0] = -In->y ; RotateQ2Mat.m[2][1] =  In->x ; RotateQ2Mat.m[2][2] =  In->w ; RotateQ2Mat.m[2][3] = -In->z ;
	RotateQ2Mat.m[3][0] =  In->x ; RotateQ2Mat.m[3][1] =  In->y ; RotateQ2Mat.m[3][2] =  In->z ; RotateQ2Mat.m[3][3] =  In->w ;

	CreateMultiplyMatrix( &RotateMat, &RotateQ2Mat, &RotateQ1Mat ) ;
	ConvertMatrixFToMatrix4x4cF( Out, &RotateMat ) ;
}

static __inline void CreateQuaternionRotateMatrix4X4CT( MATRIX_4X4CT * RST Out, FLOAT4 * RST In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateQuaternionRotateMatrix4X4CTD( &Out->md, In ) ;
	}
	else
	{
		CreateQuaternionRotateMatrix4X4CTF( &Out->mf, In ) ;
	}
}

// 単位行列を作成する
static __inline void CreateIdentityMatrix4X4CTF( MATRIX_4X4CT_F * RST Out )
{
	Out->m[ 0 ][ 0 ] = 1.0f ;
	Out->m[ 0 ][ 1 ] = 0.0f ;
	Out->m[ 0 ][ 2 ] = 0.0f ;
	Out->m[ 0 ][ 3 ] = 0.0f ;

	Out->m[ 1 ][ 0 ] = 0.0f ;
	Out->m[ 1 ][ 1 ] = 1.0f ;
	Out->m[ 1 ][ 2 ] = 0.0f ;
	Out->m[ 1 ][ 3 ] = 0.0f ;

	Out->m[ 2 ][ 0 ] = 0.0f ;
	Out->m[ 2 ][ 1 ] = 0.0f ;
	Out->m[ 2 ][ 2 ] = 1.0f ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateIdentityMatrix4X4CTD( MATRIX_4X4CT_D * RST Out )
{
	Out->m[ 0 ][ 0 ] = 1.0 ;
	Out->m[ 0 ][ 1 ] = 0.0 ;
	Out->m[ 0 ][ 2 ] = 0.0 ;
	Out->m[ 0 ][ 3 ] = 0.0 ;

	Out->m[ 1 ][ 0 ] = 0.0 ;
	Out->m[ 1 ][ 1 ] = 1.0 ;
	Out->m[ 1 ][ 2 ] = 0.0 ;
	Out->m[ 1 ][ 3 ] = 0.0 ;

	Out->m[ 2 ][ 0 ] = 0.0 ;
	Out->m[ 2 ][ 1 ] = 0.0 ;
	Out->m[ 2 ][ 2 ] = 1.0 ;
	Out->m[ 2 ][ 3 ] = 0.0 ;
}

static __inline void CreateIdentityMatrix4X4CTD( MATRIX_4X4CT * RST Out )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateIdentityMatrix4X4CTD( &Out->md ) ;
	}
	else
	{
		CreateIdentityMatrix4X4CTF( &Out->mf ) ;
	}
}

// 平行移動行列を作成する
static __inline void CreateTranslationMatrix4X4CTF( MATRIX_4X4CT_F *Out, float x, float y, float z )
{
	_MEMSET( Out, 0, sizeof( *Out ) ) ;
	Out->m[ 0 ][ 3 ] = x ;
	Out->m[ 1 ][ 3 ] = y ;
	Out->m[ 2 ][ 3 ] = z ;
	Out->m[ 0 ][ 0 ] = 1.0f ;
	Out->m[ 1 ][ 1 ] = 1.0f ;
	Out->m[ 2 ][ 2 ] = 1.0f ;
}

static __inline void CreateTranslationMatrix4X4CTD( MATRIX_4X4CT_D *Out, double x, double y, double z )
{
	_MEMSET( Out, 0, sizeof( *Out ) ) ;
	Out->m[ 0 ][ 3 ] = x ;
	Out->m[ 1 ][ 3 ] = y ;
	Out->m[ 2 ][ 3 ] = z ;
	Out->m[ 0 ][ 0 ] = 1.0 ;
	Out->m[ 1 ][ 1 ] = 1.0 ;
	Out->m[ 2 ][ 2 ] = 1.0 ;
}

static __inline void CreateTranslationMatrix4X4CT( MATRIX_4X4CT *Out, double x, double y, double z )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateTranslationMatrix4X4CTD( &Out->md, x, y, z ) ;
	}
	else
	{
		CreateTranslationMatrix4X4CTF( &Out->mf, ( float )x, ( float )y, ( float )z ) ;
	}
}

// スケーリング行列を作成する
static __inline void CreateScalingMatrix4X4CTF( MATRIX_4X4CT_F *Out, float sx, float sy, float sz )
{
	_MEMSET( Out, 0, sizeof( *Out ) ) ;

	Out->m[ 0 ][ 0 ] = sx ;
	Out->m[ 1 ][ 1 ] = sy ;
	Out->m[ 2 ][ 2 ] = sz ;
	Out->m[ 3 ][ 3 ] = 1.0f ;
}

static __inline void CreateScalingMatrix4X4CTD( MATRIX_4X4CT_D *Out, double sx, double sy, double sz )
{
	_MEMSET( Out, 0, sizeof( *Out ) ) ;

	Out->m[ 0 ][ 0 ] = sx ;
	Out->m[ 1 ][ 1 ] = sy ;
	Out->m[ 2 ][ 2 ] = sz ;
	Out->m[ 3 ][ 3 ] = 1.0 ;
}

static __inline void CreateScalingMatrix4X4CT( MATRIX_4X4CT *Out, double sx, double sy, double sz )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateScalingMatrix4X4CTD( &Out->md, sx, sy, sz ) ;
	}
	else
	{
		CreateScalingMatrix4X4CTF( &Out->mf, ( float )sx, ( float )sy, ( float )sz ) ;
	}
}

// Ｘ軸回転→Ｙ軸回転→Ｚ軸回転を合成した行列を作成する
static __inline void CreateRotationXYZMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float SinXSinY, SinYCosX ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	SinXSinY = SinX * SinY ;
	SinYCosX = SinY * CosX ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = SinXSinY * CosZ - CosX * SinZ ;
	Out->m[ 0 ][ 2 ] = SinYCosX * CosZ + SinX * SinZ ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = CosY * SinZ ;
	Out->m[ 1 ][ 1 ] = SinXSinY * SinZ + CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = SinYCosX * SinZ - SinX * CosZ ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = -SinY ;
	Out->m[ 2 ][ 1 ] = SinX * CosY ;
	Out->m[ 2 ][ 2 ] = CosX * CosY ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationXYZMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float SinXSinY, SinYCosX ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	SinXSinY = SinX * SinY ;
	SinYCosX = SinY * CosX ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = SinXSinY * CosZ - CosX * SinZ ;
	Out->m[ 0 ][ 2 ] = SinYCosX * CosZ + SinX * SinZ ;
	Out->m[ 0 ][ 3 ] = 0.0 ;
	Out->m[ 1 ][ 0 ] = CosY * SinZ ;
	Out->m[ 1 ][ 1 ] = SinXSinY * SinZ + CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = SinYCosX * SinZ - SinX * CosZ ;
	Out->m[ 1 ][ 3 ] = 0.0 ;
	Out->m[ 2 ][ 0 ] = -SinY ;
	Out->m[ 2 ][ 1 ] = SinX * CosY ;
	Out->m[ 2 ][ 2 ] = CosX * CosY ;
	Out->m[ 2 ][ 3 ] = 0.0 ;
}

static __inline void CreateRotationXYZMatrix4X4CT( MATRIX_4X4CT * RST Out, float XRot, float YRot, float ZRot )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateRotationXYZMatrix4X4CTD( &Out->md, XRot, YRot, ZRot ) ;
	}
	else
	{
		CreateRotationXYZMatrix4X4CTF( &Out->mf, XRot, YRot, ZRot ) ;
	}
}

// Ｘ軸回転→Ｚ軸回転→Ｙ軸回転を合成した行列を作成する
static __inline void CreateRotationXZYMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float CosXSinZ, SinXSinZ ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	CosXSinZ = CosX * SinZ ;
	SinXSinZ = SinX * SinZ ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = CosXSinZ * -CosY + SinX * SinY ;
	Out->m[ 0 ][ 2 ] = SinXSinZ *  CosY + CosX * SinY ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = SinZ ;
	Out->m[ 1 ][ 1 ] = CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = -SinX * CosZ ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = CosZ * -SinY ;
	Out->m[ 2 ][ 1 ] = CosXSinZ *  SinY + SinX * CosY ;
	Out->m[ 2 ][ 2 ] = SinXSinZ * -SinY + CosX * CosY ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationXZYMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float CosXSinZ, SinXSinZ ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	CosXSinZ = CosX * SinZ ;
	SinXSinZ = SinX * SinZ ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = CosXSinZ * -CosY + SinX * SinY ;
	Out->m[ 0 ][ 2 ] = SinXSinZ *  CosY + CosX * SinY ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = SinZ ;
	Out->m[ 1 ][ 1 ] = CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = -SinX * CosZ ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = CosZ * -SinY ;
	Out->m[ 2 ][ 1 ] = CosXSinZ *  SinY + SinX * CosY ;
	Out->m[ 2 ][ 2 ] = SinXSinZ * -SinY + CosX * CosY ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationXZYMatrix4X4CT( MATRIX_4X4CT * RST Out, float XRot, float YRot, float ZRot )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateRotationXZYMatrix4X4CTD( &Out->md, XRot, YRot, ZRot ) ;
	}
	else
	{
		CreateRotationXZYMatrix4X4CTF( &Out->mf, XRot, YRot, ZRot ) ;
	}
}

// Ｙ軸回転→Ｘ軸回転→Ｚ軸回転を合成した行列を作成する
static __inline void CreateRotationYXZMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float SinXSinY, CosYSinX ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	SinXSinY = SinX * SinY ;
	CosYSinX = CosY * SinX ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ - SinXSinY * SinZ ;
	Out->m[ 0 ][ 1 ] = CosX * -SinZ ;
	Out->m[ 0 ][ 2 ] = SinY * CosZ + CosYSinX * SinZ ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = CosY * SinZ + SinXSinY * CosZ ;
	Out->m[ 1 ][ 1 ] = CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = SinY * SinZ - CosYSinX * CosZ ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = -SinY * CosX ;
	Out->m[ 2 ][ 1 ] = SinX ;
	Out->m[ 2 ][ 2 ] = CosY * CosX ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationYXZMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float SinXSinY, CosYSinX ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	SinXSinY = SinX * SinY ;
	CosYSinX = CosY * SinX ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ - SinXSinY * SinZ ;
	Out->m[ 0 ][ 1 ] = CosX * -SinZ ;
	Out->m[ 0 ][ 2 ] = SinY * CosZ + CosYSinX * SinZ ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = CosY * SinZ + SinXSinY * CosZ ;
	Out->m[ 1 ][ 1 ] = CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = SinY * SinZ - CosYSinX * CosZ ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = -SinY * CosX ;
	Out->m[ 2 ][ 1 ] = SinX ;
	Out->m[ 2 ][ 2 ] = CosY * CosX ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationYXZMatrix4X4CT( MATRIX_4X4CT * RST Out, float XRot, float YRot, float ZRot )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateRotationYXZMatrix4X4CTD( &Out->md, XRot, YRot, ZRot ) ;
	}
	else
	{
		CreateRotationYXZMatrix4X4CTF( &Out->mf, XRot, YRot, ZRot ) ;
	}
}

// Ｙ軸回転→Ｚ軸回転→Ｘ軸回転を合成した行列を作成する
static __inline void CreateRotationYZXMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float CosYSinZ, SinYSinZ ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	CosYSinZ = CosY * SinZ ;
	SinYSinZ = SinY * SinZ ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = -SinZ ;
	Out->m[ 0 ][ 2 ] = SinY * CosZ ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = CosYSinZ * CosX + SinY * SinX ;
	Out->m[ 1 ][ 1 ] = CosZ * CosX ;
	Out->m[ 1 ][ 2 ] = SinYSinZ * CosX - CosY * SinX ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = CosYSinZ * SinX - SinY * CosX ;
	Out->m[ 2 ][ 1 ] = CosZ * SinX ;
	Out->m[ 2 ][ 2 ] = SinYSinZ * SinX + CosY * CosX ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationYZXMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float CosYSinZ, SinYSinZ ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	CosYSinZ = CosY * SinZ ;
	SinYSinZ = SinY * SinZ ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = -SinZ ;
	Out->m[ 0 ][ 2 ] = SinY * CosZ ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = CosYSinZ * CosX + SinY * SinX ;
	Out->m[ 1 ][ 1 ] = CosZ * CosX ;
	Out->m[ 1 ][ 2 ] = SinYSinZ * CosX - CosY * SinX ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = CosYSinZ * SinX - SinY * CosX ;
	Out->m[ 2 ][ 1 ] = CosZ * SinX ;
	Out->m[ 2 ][ 2 ] = SinYSinZ * SinX + CosY * CosX ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationYZXMatrix4X4CT( MATRIX_4X4CT * RST Out, float XRot, float YRot, float ZRot )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateRotationYZXMatrix4X4CTD( &Out->md, XRot, YRot, ZRot ) ;
	}
	else
	{
		CreateRotationYZXMatrix4X4CTF( &Out->mf, XRot, YRot, ZRot ) ;
	}
}

// Ｚ軸回転→Ｘ軸回転→Ｙ軸回転を合成した行列を作成する
static __inline void CreateRotationZXYMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float SinXSinZ, SinXCosZ ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	SinXSinZ = SinX * SinZ ;
	SinXCosZ = SinX * CosZ ;

	Out->m[ 0 ][ 0 ] = CosY *  CosZ + SinXSinZ * SinY ;
	Out->m[ 0 ][ 1 ] = CosY * -SinZ + SinXCosZ * SinY ;
	Out->m[ 0 ][ 2 ] = CosX * SinY ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = CosX * SinZ ;
	Out->m[ 1 ][ 1 ] = CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = -SinX ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = CosZ * -SinY + SinXSinZ * CosY ;
	Out->m[ 2 ][ 1 ] = SinZ *  SinY + SinXCosZ * CosY ;
	Out->m[ 2 ][ 2 ] = CosX * CosY ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationZXYMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float SinXSinZ, SinXCosZ ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	SinXSinZ = SinX * SinZ ;
	SinXCosZ = SinX * CosZ ;

	Out->m[ 0 ][ 0 ] = CosY *  CosZ + SinXSinZ * SinY ;
	Out->m[ 0 ][ 1 ] = CosY * -SinZ + SinXCosZ * SinY ;
	Out->m[ 0 ][ 2 ] = CosX * SinY ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = CosX * SinZ ;
	Out->m[ 1 ][ 1 ] = CosX * CosZ ;
	Out->m[ 1 ][ 2 ] = -SinX ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = CosZ * -SinY + SinXSinZ * CosY ;
	Out->m[ 2 ][ 1 ] = SinZ *  SinY + SinXCosZ * CosY ;
	Out->m[ 2 ][ 2 ] = CosX * CosY ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationZXYMatrix4X4CT( MATRIX_4X4CT * RST Out, float XRot, float YRot, float ZRot )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateRotationZXYMatrix4X4CTD( &Out->md, XRot, YRot, ZRot ) ;
	}
	else
	{
		CreateRotationZXYMatrix4X4CTF( &Out->mf, XRot, YRot, ZRot ) ;
	}
}

// Ｚ軸回転→Ｙ軸回転→Ｘ軸回転を合成した行列を作成する
static __inline void CreateRotationZYXMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float CosZSinY, SinZSinY ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	CosZSinY = CosZ * SinY ;
	SinZSinY = SinZ * SinY ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = -SinZ * CosY ;
	Out->m[ 0 ][ 2 ] = SinY ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = SinZ * CosX + CosZSinY * SinX ;
	Out->m[ 1 ][ 1 ] = CosZ * CosX - SinZSinY * SinX ;
	Out->m[ 1 ][ 2 ] = CosY * -SinX ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = SinZ * SinX - CosZSinY * CosX ;
	Out->m[ 2 ][ 1 ] = CosZ * SinX + SinZSinY * CosX ;
	Out->m[ 2 ][ 2 ] = CosY *  CosX ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationZYXMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, float XRot, float YRot, float ZRot )
{
	float SinX, CosX, SinY, CosY, SinZ, CosZ ;
	float CosZSinY, SinZSinY ;

	TableSinCos( XRot, &SinX, &CosX ) ;
	TableSinCos( YRot, &SinY, &CosY ) ;
	TableSinCos( ZRot, &SinZ, &CosZ ) ;

	CosZSinY = CosZ * SinY ;
	SinZSinY = SinZ * SinY ;

	Out->m[ 0 ][ 0 ] = CosY * CosZ ;
	Out->m[ 0 ][ 1 ] = -SinZ * CosY ;
	Out->m[ 0 ][ 2 ] = SinY ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = SinZ * CosX + CosZSinY * SinX ;
	Out->m[ 1 ][ 1 ] = CosZ * CosX - SinZSinY * SinX ;
	Out->m[ 1 ][ 2 ] = CosY * -SinX ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = SinZ * SinX - CosZSinY * CosX ;
	Out->m[ 2 ][ 1 ] = CosZ * SinX + SinZSinY * CosX ;
	Out->m[ 2 ][ 2 ] = CosY *  CosX ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
}

static __inline void CreateRotationZYXMatrix4X4CT( MATRIX_4X4CT * RST Out, float XRot, float YRot, float ZRot )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		CreateRotationZYXMatrix4X4CTD( &Out->md, XRot, YRot, ZRot ) ;
	}
	else
	{
		CreateRotationZYXMatrix4X4CTF( &Out->mf, XRot, YRot, ZRot ) ;
	}
}

static __inline void UnSafeTranslateOnlyMultiplyMatrix4X4CTF( MATRIX_4X4CT_F * RST Out, MATRIX_4X4CT_F * RST In1, MATRIX_4X4CT_F * RST In2 )
{
	Out->m[ 0 ][ 0 ] = In2->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In2->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] = In2->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 0 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 0 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 0 ][ 2 ] + In2->m[ 0 ][ 3 ] ;
	Out->m[ 1 ][ 0 ] = In2->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In2->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In2->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 1 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 1 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 1 ][ 2 ] + In2->m[ 1 ][ 3 ] ;
	Out->m[ 2 ][ 0 ] = In2->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] = In2->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In2->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 2 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 2 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 2 ][ 2 ] + In2->m[ 2 ][ 3 ] ;
}

static __inline void UnSafeTranslateOnlyMultiplyMatrix4X4CTD( MATRIX_4X4CT_D * RST Out, MATRIX_4X4CT_D * RST In1, MATRIX_4X4CT_D * RST In2 )
{
	Out->m[ 0 ][ 0 ] = In2->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In2->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] = In2->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 0 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 0 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 0 ][ 2 ] + In2->m[ 0 ][ 3 ] ;
	Out->m[ 1 ][ 0 ] = In2->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In2->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In2->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 1 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 1 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 1 ][ 2 ] + In2->m[ 1 ][ 3 ] ;
	Out->m[ 2 ][ 0 ] = In2->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] = In2->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In2->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 2 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 2 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 2 ][ 2 ] + In2->m[ 2 ][ 3 ] ;
}

static __inline void UnSafeTranslateOnlyMultiplyMatrix4X4CT_DF_D( MATRIX_4X4CT_D * RST Out, MATRIX_4X4CT_D * RST In1, MATRIX_4X4CT_F * RST In2 )
{
	Out->m[ 0 ][ 0 ] = In2->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In2->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] = In2->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 0 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 0 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 0 ][ 2 ] + In2->m[ 0 ][ 3 ] ;
	Out->m[ 1 ][ 0 ] = In2->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In2->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In2->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 1 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 1 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 1 ][ 2 ] + In2->m[ 1 ][ 3 ] ;
	Out->m[ 2 ][ 0 ] = In2->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] = In2->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In2->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 2 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 2 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 2 ][ 2 ] + In2->m[ 2 ][ 3 ] ;
}

static __inline void UnSafeTranslateOnlyMultiplyMatrix4X4CT_FD_D( MATRIX_4X4CT_D * RST Out, MATRIX_4X4CT_F * RST In1, MATRIX_4X4CT_D * RST In2 )
{
	Out->m[ 0 ][ 0 ] = In2->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In2->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] = In2->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 0 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 0 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 0 ][ 2 ] + In2->m[ 0 ][ 3 ] ;
	Out->m[ 1 ][ 0 ] = In2->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In2->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In2->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 1 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 1 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 1 ][ 2 ] + In2->m[ 1 ][ 3 ] ;
	Out->m[ 2 ][ 0 ] = In2->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] = In2->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In2->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = In1->m[ 0 ][ 3 ] * In2->m[ 2 ][ 0 ] + In1->m[ 1 ][ 3 ] * In2->m[ 2 ][ 1 ] + In1->m[ 2 ][ 3 ] * In2->m[ 2 ][ 2 ] + In2->m[ 2 ][ 3 ] ;
}

static __inline void UnSafeTranslateOnlyMultiplyMatrix4X4CT( MATRIX_4X4CT * RST Out, MATRIX_4X4CT * RST In1, MATRIX_4X4CT * RST In2 )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		UnSafeTranslateOnlyMultiplyMatrix4X4CTD( &Out->md, &In1->md, &In2->md ) ;
	}
	else
	{
		UnSafeTranslateOnlyMultiplyMatrix4X4CTF( &Out->mf, &In1->mf, &In2->mf ) ;
	}
}

static __inline void UnSafeTranslateOnlyMultiplyMatrix4X4CT_FC( MATRIX_4X4CT * RST Out, MATRIX_4X4CT_F * RST In1, MATRIX_4X4CT * RST In2 )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		UnSafeTranslateOnlyMultiplyMatrix4X4CT_FD_D( &Out->md, In1, &In2->md ) ;
	}
	else
	{
		UnSafeTranslateOnlyMultiplyMatrix4X4CTF( &Out->mf, In1, &In2->mf ) ;
	}
}

static __inline void UnSafeTranslateOnlyMultiplyMatrix4X4CT_CF( MATRIX_4X4CT * RST Out, MATRIX_4X4CT * RST In1, MATRIX_4X4CT_F * RST In2 )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		UnSafeTranslateOnlyMultiplyMatrix4X4CT_DF_D( &Out->md, &In1->md, In2 ) ;
	}
	else
	{
		UnSafeTranslateOnlyMultiplyMatrix4X4CTF( &Out->mf, &In1->mf, In2 ) ;
	}
}

static __inline void UnSafeMatrix4X4CT_F_EqPlus_F_Sub_F( MATRIX_4X4CT_F *RST Out, MATRIX_4X4CT_F * RST In1, MATRIX_4X4CT_F * RST In2 )
{
	Out->m[ 0 ][ 0 ] += In1->m[ 0 ][ 0 ] - In2->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] += In1->m[ 0 ][ 1 ] - In2->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] += In1->m[ 0 ][ 2 ] - In2->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] += In1->m[ 0 ][ 3 ] - In2->m[ 0 ][ 3 ] ;

	Out->m[ 1 ][ 0 ] += In1->m[ 1 ][ 0 ] - In2->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] += In1->m[ 1 ][ 1 ] - In2->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] += In1->m[ 1 ][ 2 ] - In2->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] += In1->m[ 1 ][ 3 ] - In2->m[ 1 ][ 3 ] ;

	Out->m[ 2 ][ 0 ] += In1->m[ 2 ][ 0 ] - In2->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] += In1->m[ 2 ][ 1 ] - In2->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] += In1->m[ 2 ][ 2 ] - In2->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] += In1->m[ 2 ][ 3 ] - In2->m[ 2 ][ 3 ] ;
}

static __inline void UnSafeMatrix4X4CT_F_EqPlus_F_Sub_F_Mul_S( MATRIX_4X4CT_F *RST Out, MATRIX_4X4CT_F * RST In1, MATRIX_4X4CT_F * RST In2, float In3 )
{
	Out->m[ 0 ][ 0 ] += ( In1->m[ 0 ][ 0 ] - In2->m[ 0 ][ 0 ] ) * In3 ;
	Out->m[ 0 ][ 1 ] += ( In1->m[ 0 ][ 1 ] - In2->m[ 0 ][ 1 ] ) * In3 ;
	Out->m[ 0 ][ 2 ] += ( In1->m[ 0 ][ 2 ] - In2->m[ 0 ][ 2 ] ) * In3 ;
	Out->m[ 0 ][ 3 ] += ( In1->m[ 0 ][ 3 ] - In2->m[ 0 ][ 3 ] ) * In3 ;

	Out->m[ 1 ][ 0 ] += ( In1->m[ 1 ][ 0 ] - In2->m[ 1 ][ 0 ] ) * In3 ;
	Out->m[ 1 ][ 1 ] += ( In1->m[ 1 ][ 1 ] - In2->m[ 1 ][ 1 ] ) * In3 ;
	Out->m[ 1 ][ 2 ] += ( In1->m[ 1 ][ 2 ] - In2->m[ 1 ][ 2 ] ) * In3 ;
	Out->m[ 1 ][ 3 ] += ( In1->m[ 1 ][ 3 ] - In2->m[ 1 ][ 3 ] ) * In3 ;

	Out->m[ 2 ][ 0 ] += ( In1->m[ 2 ][ 0 ] - In2->m[ 2 ][ 0 ] ) * In3 ;
	Out->m[ 2 ][ 1 ] += ( In1->m[ 2 ][ 1 ] - In2->m[ 2 ][ 1 ] ) * In3 ;
	Out->m[ 2 ][ 2 ] += ( In1->m[ 2 ][ 2 ] - In2->m[ 2 ][ 2 ] ) * In3 ;
	Out->m[ 2 ][ 3 ] += ( In1->m[ 2 ][ 3 ] - In2->m[ 2 ][ 3 ] ) * In3 ;
}

static __inline void UnSafeMatrix4X4CT_F_EqPlus_F( MATRIX_4X4CT_F *RST Out, MATRIX_4X4CT_F * RST In1 )
{
	Out->m[ 0 ][ 0 ] += In1->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] += In1->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] += In1->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] += In1->m[ 0 ][ 3 ] ;

	Out->m[ 1 ][ 0 ] += In1->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] += In1->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] += In1->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] += In1->m[ 1 ][ 3 ] ;

	Out->m[ 2 ][ 0 ] += In1->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] += In1->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] += In1->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] += In1->m[ 2 ][ 3 ] ;
}

static __inline void SetupSkiningBoneMatrix( MV1_MODEL &Model, MV1_FRAME &Frame )
{
	MV1_SKIN_BONE *SkinW ;
	int index ;
	int i ;

	if( Frame.BaseData->SkinBoneNum != 0 )
	{
		SkinW = Frame.BaseData->SkinBone ;
		index = ( int )( SkinW - Model.BaseData->SkinBone ) ;
		for( i = 0 ; i < Frame.BaseData->SkinBoneNum ; i ++, SkinW ++, index ++ )
		{
			if( SkinW->ModelLocalMatrixIsTranslateOnly == 0 )
			{
				UnSafeMultiplyMatrix4X4CT_FC( &Model.SkinBoneMatrix[ index ], &SkinW->ModelLocalMatrix, &Frame.LocalWorldMatrix ) ;
			}
			else
			{
				UnSafeTranslateOnlyMultiplyMatrix4X4CT_FC( &Model.SkinBoneMatrix[ index ], &SkinW->ModelLocalMatrix, &Frame.LocalWorldMatrix ) ;
			}
		}
	}
}

static 
#ifndef __BCC
__inline
#endif
int __FTOL( float Real )
{
#ifdef DX_NON_INLINE_ASM
	return (int)Real ;
#else
	DWORD Result ;
	WORD STFlag, DSTFlag ;
	__asm
	{
		fnstcw	STFlag
		mov		ax, STFlag
		or		ax, 0xC00
		mov		DSTFlag, ax
		fld		Real
		fldcw	DSTFlag
		fistp	Result
		fldcw	STFlag
	}
	return (int)Result ;
#endif
}

// 座標変換情報を使用して座標変換行列を作成する
static void MV1SetupTransformMatrix( MATRIX_4X4CT_F * RST BlendMatrix, int ValidFlag, VECTOR * RST Translate, VECTOR * RST Scale, int RotateOrder, VECTOR * RST PreRotate, VECTOR * RST Rotate, VECTOR * RST PostRotate, FLOAT4 * RST Quaternion )
{
	// 回転
	if( ValidFlag & MV1_ANIMVALUE_ROTATE )
	{
		if( PreRotate == NULL && PostRotate == NULL )
		{
			switch( RotateOrder )
			{
			case MV1_ROTATE_ORDER_XYZ : CreateRotationXYZMatrix4X4CTF( BlendMatrix, Rotate->x, Rotate->y, Rotate->z ) ; break ;
			case MV1_ROTATE_ORDER_XZY : CreateRotationXZYMatrix4X4CTF( BlendMatrix, Rotate->x, Rotate->y, Rotate->z ) ; break ;
			case MV1_ROTATE_ORDER_YXZ : CreateRotationYXZMatrix4X4CTF( BlendMatrix, Rotate->x, Rotate->y, Rotate->z ) ; break ;
			case MV1_ROTATE_ORDER_YZX : CreateRotationYZXMatrix4X4CTF( BlendMatrix, Rotate->x, Rotate->y, Rotate->z ) ; break ;
			case MV1_ROTATE_ORDER_ZXY : CreateRotationZXYMatrix4X4CTF( BlendMatrix, Rotate->x, Rotate->y, Rotate->z ) ; break ;
			case MV1_ROTATE_ORDER_ZYX : CreateRotationZYXMatrix4X4CTF( BlendMatrix, Rotate->x, Rotate->y, Rotate->z ) ; break ;
			}
		}
		else
		{
			MATRIX_4X4CT_F PreRotMat, PostRotMat, RotMat ;

			switch( RotateOrder )
			{
			case MV1_ROTATE_ORDER_XYZ :
				if( PreRotate )  CreateRotationXYZMatrix4X4CTF( &PreRotMat,  PreRotate->x,  PreRotate->y,  PreRotate->z ) ;
				if( PostRotate ) CreateRotationXYZMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ;
				CreateRotationXYZMatrix4X4CTF( &RotMat, Rotate->x, Rotate->y, Rotate->z ) ;
				break ;

			case MV1_ROTATE_ORDER_XZY :
				if( PreRotate )  CreateRotationXZYMatrix4X4CTF( &PreRotMat,  PreRotate->x,  PreRotate->y,  PreRotate->z ) ;
				if( PostRotate ) CreateRotationXZYMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ;
				CreateRotationXZYMatrix4X4CTF( &RotMat, Rotate->x, Rotate->y, Rotate->z ) ;
				break ;

			case MV1_ROTATE_ORDER_YXZ :
				if( PreRotate )  CreateRotationYXZMatrix4X4CTF( &PreRotMat,  PreRotate->x,  PreRotate->y,  PreRotate->z ) ;
				if( PostRotate ) CreateRotationYXZMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ;
				CreateRotationYXZMatrix4X4CTF( &RotMat, Rotate->x, Rotate->y, Rotate->z ) ;
				break ;

			case MV1_ROTATE_ORDER_YZX :
				if( PreRotate )  CreateRotationYZXMatrix4X4CTF( &PreRotMat,  PreRotate->x,  PreRotate->y,  PreRotate->z ) ;
				if( PostRotate ) CreateRotationYZXMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ;
				CreateRotationYZXMatrix4X4CTF( &RotMat, Rotate->x, Rotate->y, Rotate->z ) ;
				break ;

			case MV1_ROTATE_ORDER_ZXY :
				if( PreRotate )  CreateRotationZXYMatrix4X4CTF( &PreRotMat,  PreRotate->x,  PreRotate->y,  PreRotate->z ) ;
				if( PostRotate ) CreateRotationZXYMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ;
				CreateRotationZXYMatrix4X4CTF( &RotMat, Rotate->x, Rotate->y, Rotate->z ) ;
				break ;

			case MV1_ROTATE_ORDER_ZYX :
				if( PreRotate )  CreateRotationZYXMatrix4X4CTF( &PreRotMat,  PreRotate->x,  PreRotate->y,  PreRotate->z ) ;
				if( PostRotate ) CreateRotationZYXMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ;
				CreateRotationZYXMatrix4X4CTF( &RotMat, Rotate->x, Rotate->y, Rotate->z ) ;
				break ;
			}

			if( PreRotate && PostRotate )
			{
				MATRIX_4X4CT_F TempMatrix ;

				UnSafeMultiplyMatrix4X4CTF( &TempMatrix, &RotMat, &PreRotMat ) ;
				UnSafeMultiplyMatrix4X4CTF( BlendMatrix, &PostRotMat, &TempMatrix ) ;
			}
			else
			if( PreRotate )
			{
				UnSafeMultiplyMatrix4X4CTF( BlendMatrix, &RotMat, &PreRotMat ) ;
			}
			else
			{
				UnSafeMultiplyMatrix4X4CTF( BlendMatrix, &PostRotMat, &RotMat ) ;
			}
		}
	}
	else
	// クォータニオン回転
	if( ValidFlag & MV1_ANIMVALUE_QUATERNION_X )
	{
		MATRIX TempMatrix1, TempMatrix2 ;

		TempMatrix1.m[3][0] = -Quaternion->x ; TempMatrix1.m[3][1] = -Quaternion->y ; TempMatrix1.m[3][2] = -Quaternion->z ; TempMatrix1.m[3][3] =  Quaternion->w ;
		TempMatrix1.m[2][0] = -Quaternion->y ; TempMatrix1.m[2][1] =  Quaternion->x ; TempMatrix1.m[2][2] =  Quaternion->w ; TempMatrix1.m[2][3] =  Quaternion->z ;
		TempMatrix1.m[1][0] =  Quaternion->z ; TempMatrix1.m[1][1] =  Quaternion->w ; TempMatrix1.m[1][2] = -Quaternion->x ; TempMatrix1.m[1][3] =  Quaternion->y ;
		TempMatrix1.m[0][0] =  Quaternion->w ; TempMatrix1.m[0][1] = -Quaternion->z ; TempMatrix1.m[0][2] =  Quaternion->y ; TempMatrix1.m[0][3] =  Quaternion->x ;

		TempMatrix2.m[3][0] =  Quaternion->x ; TempMatrix2.m[3][1] =  Quaternion->y ; TempMatrix2.m[3][2] =  Quaternion->z ; TempMatrix2.m[3][3] =  Quaternion->w ;
		TempMatrix2.m[2][0] = -Quaternion->y ; TempMatrix2.m[2][1] =  Quaternion->x ; TempMatrix2.m[2][2] =  Quaternion->w ; TempMatrix2.m[2][3] = -Quaternion->z ;
		TempMatrix2.m[1][0] =  Quaternion->z ; TempMatrix2.m[1][1] =  Quaternion->w ; TempMatrix2.m[1][2] = -Quaternion->x ; TempMatrix2.m[1][3] = -Quaternion->y ;
		TempMatrix2.m[0][0] =  Quaternion->w ; TempMatrix2.m[0][1] = -Quaternion->z ; TempMatrix2.m[0][2] =  Quaternion->y ; TempMatrix2.m[0][3] = -Quaternion->x ;

		CreateMultiplyMatrix( &TempMatrix1, &TempMatrix2, &TempMatrix1 ) ;
		ConvertMatrixFToMatrix4x4cF( BlendMatrix, &TempMatrix1 ) ;
	}
	else
	if( ValidFlag & MV1_ANIMVALUE_QUATERNION_VMD )
	{
		float	x2 = Quaternion->x * Quaternion->x * 2.0f ;
		float	y2 = Quaternion->y * Quaternion->y * 2.0f ;
		float	z2 = Quaternion->z * Quaternion->z * 2.0f ;
		float	xy = Quaternion->x * Quaternion->y * 2.0f ;
		float	yz = Quaternion->y * Quaternion->z * 2.0f ;
		float	zx = Quaternion->z * Quaternion->x * 2.0f ;
		float	xw = Quaternion->x * Quaternion->w * 2.0f ;
		float	yw = Quaternion->y * Quaternion->w * 2.0f ;
		float	zw = Quaternion->z * Quaternion->w * 2.0f ;

		BlendMatrix->m[ 0 ][ 0 ] = 1.0f - y2 - z2 ;
		BlendMatrix->m[ 1 ][ 0 ] = xy + zw ;
		BlendMatrix->m[ 2 ][ 0 ] = zx - yw ;
		BlendMatrix->m[ 0 ][ 1 ] = xy - zw ;
		BlendMatrix->m[ 1 ][ 1 ] = 1.0f - z2 - x2 ;
		BlendMatrix->m[ 2 ][ 1 ] = yz + xw ;
		BlendMatrix->m[ 0 ][ 2 ] = zx + yw ;
		BlendMatrix->m[ 1 ][ 2 ] = yz - xw ;
		BlendMatrix->m[ 2 ][ 2 ] = 1.0f - x2 - y2 ;

		BlendMatrix->m[ 0 ][ 3 ] =
		BlendMatrix->m[ 1 ][ 3 ] =
		BlendMatrix->m[ 2 ][ 3 ] = 0.0f ;
	}
	else
	{
		BlendMatrix->m[ 0 ][ 0 ] = 1.0f ;
		BlendMatrix->m[ 0 ][ 1 ] = 0.0f ;
		BlendMatrix->m[ 0 ][ 2 ] = 0.0f ;
		BlendMatrix->m[ 1 ][ 0 ] = 0.0f ;
		BlendMatrix->m[ 1 ][ 1 ] = 1.0f ;
		BlendMatrix->m[ 1 ][ 2 ] = 0.0f ;
		BlendMatrix->m[ 2 ][ 0 ] = 0.0f ;
		BlendMatrix->m[ 2 ][ 1 ] = 0.0f ;
		BlendMatrix->m[ 2 ][ 2 ] = 1.0f ;
	}

	// 平行移動
	if( ValidFlag & MV1_ANIMVALUE_TRANSLATE )
	{
		BlendMatrix->m[ 0 ][ 3 ] = Translate->x ;
		BlendMatrix->m[ 1 ][ 3 ] = Translate->y ;
		BlendMatrix->m[ 2 ][ 3 ] = Translate->z ;
	}
	else
	{
		BlendMatrix->m[ 0 ][ 3 ] = 0.0f ;
		BlendMatrix->m[ 1 ][ 3 ] = 0.0f ;
		BlendMatrix->m[ 2 ][ 3 ] = 0.0f ;
	}

	// スケーリング
	if( ValidFlag & MV1_ANIMVALUE_SCALE )
	{
		BlendMatrix->m[ 0 ][ 0 ] *= Scale->x ;
		BlendMatrix->m[ 0 ][ 1 ] *= Scale->y ;
		BlendMatrix->m[ 0 ][ 2 ] *= Scale->z ;
		BlendMatrix->m[ 1 ][ 0 ] *= Scale->x ;
		BlendMatrix->m[ 1 ][ 1 ] *= Scale->y ;
		BlendMatrix->m[ 1 ][ 2 ] *= Scale->z ;
		BlendMatrix->m[ 2 ][ 0 ] *= Scale->x ;
		BlendMatrix->m[ 2 ][ 1 ] *= Scale->y ;
		BlendMatrix->m[ 2 ][ 2 ] *= Scale->z ;
	}
}

// アニメーションの行列をセットアップする
static void MV1SetupAnimMatrix( MV1_MODEL *Model )
{
	int i ;

	for( i = 0 ; i < Model->AnimSetMaxNum ; i ++ )
	{
		if( Model->AnimSet[ i ].Use == false || Model->AnimSet[ i ].AnimSet->ParamSetup ) continue ;
		MV1AnimSetSetupParam( Model->AnimSet[ i ].AnimSet ) ;
	}

	Model->AnimSetupFlag = true ;
}

// 描画用の行列を構築する
static void MV1SetupMatrix( MV1_MODEL *Model )
{
	int i, j, mcon ;
	MV1_FRAME *Frame ;
	MV1_MODEL_BASE *MBase ;
	MV1_FRAME_BASE *FrameBase ;
	MV1_MODEL_ANIM *MAnim, *MAnim2, *MAnim3 = NULL;
	VECTOR DivSize ;

	MBase = Model->BaseData ;

	// 行列がセットアップ完了状態の場合は何もせずに終了
	if( Model->LocalWorldMatrixSetupFlag == true )
		return ;

	// ポリゴンリスト更新フラグを倒す
	Model->SetupRefPolygon[ 1 ][ 0 ] = false ;
	Model->SetupRefPolygon[ 1 ][ 1 ] = false ;

	// アタッチしているアニメーションのパラメータに変化がある場合は行列を計算
	if( Model->AnimSetupFlag == false )
		MV1SetupAnimMatrix( Model ) ;

	// トップの更新チェック
	if( Model->ChangeMatrixFlag[ 0 ] & 1 )
	{
		// スケーリングを使用しているかどうかのフラグを倒す
		Model->LocalWorldMatrixUseScaling = false ;

		// 行列が有効な場合と無効な場合で処理を分岐
		if( Model->ValidMatrix )
		{
			// 行列が有効な場合はそのまま使用する
			Model->LocalWorldMatrix = Model->Matrix ;

			// スケーリングを使用しているかどうかのフラグを立てる
			Model->LocalWorldMatrixUseScaling = true ;
		}
		else
		{
			MATRIX_4X4CT TranslateMat, ScaleMat, RotateMat, TransformMat ;

			// 無効な場合はトランスフォームパラメータから行列を作成する
			CreateTranslationMatrix4X4CT( &TranslateMat, Model->Translation.x, Model->Translation.y, Model->Translation.z ) ;
			if( Model->Scale.x != 1.0f || Model->Scale.y != 1.0f || Model->Scale.z != 1.0f )
			{
				CreateScalingMatrix4X4CT( &ScaleMat, Model->Scale.x, Model->Scale.y, Model->Scale.z ) ;

				// スケーリングを使用しているかどうかのフラグを立てる
				Model->LocalWorldMatrixUseScaling = true ;
			}
			else
			{
				CreateIdentityMatrix4X4CTD( &ScaleMat ) ;
			}
			switch( Model->Rotation.Type )
			{
			case MV1_ROTATE_TYPE_XYZROT :
				CreateRotationXYZMatrix4X4CT( &RotateMat, Model->Rotation.XYZRot.x, Model->Rotation.XYZRot.y, Model->Rotation.XYZRot.z ) ;
				break ;

			case MV1_ROTATE_TYPE_QUATERNION :
				CreateQuaternionRotateMatrix4X4CT( &RotateMat, &Model->Rotation.Qt ) ;
				break ;

			case MV1_ROTATE_TYPE_MATRIX :
				ConvertMatrix4x4cFToMatrix4x4c( &RotateMat, &Model->Rotation.Mat ) ;

				// スケーリングを使用しているかどうかのフラグを立てる
				Model->LocalWorldMatrixUseScaling = true ;
				break ;

			case MV1_ROTATE_TYPE_ZAXIS :
				{
					MATRIX TempMatrix ;

					TempMatrix = MMult( MGetRotZ( Model->Rotation.ZAxis.Twist ), MGetAxis1( VCross( Model->Rotation.ZAxis.Up, Model->Rotation.ZAxis.Z ), Model->Rotation.ZAxis.Up, Model->Rotation.ZAxis.Z, VGet( 0.0f, 0.0f, 0.0f ) ) ) ;
					ConvertMatrixFToMatrix4x4c( &RotateMat, &TempMatrix ) ;
				}
				break ;
			}

			UnSafeMultiplyMatrix4X4CT( &TransformMat,            &ScaleMat,     &RotateMat ) ;
			UnSafeMultiplyMatrix4X4CT( &Model->LocalWorldMatrix, &TransformMat, &TranslateMat ) ;
		}
	}

	// フレームの更新チェック
	Frame = Model->Frame ;
	MAnim = Model->Anim ;
	for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++, MAnim += Model->AnimSetMaxNum )
	{
		MATRIX_4X4CT *ParentMatrix ;
		bool ParentUseScaling ;

		// 物理演算で行列を導き出すようになっている場合は何もしない
		if( Frame->PhysicsRigidBody != NULL ) continue ;

		// 更新が必要ない場合は何もしない
		if( MV1CCHK( Frame->LocalWorldMatrixChange ) == 0 ) continue ;
		FrameBase = Frame->BaseData ;

		// ポリゴンリスト更新フラグを倒す
		Frame->SetupRefPolygon[ 1 ][ 0 ] = false ;
		Frame->SetupRefPolygon[ 1 ][ 1 ] = false ;

		// スケーリングを使用しているかどうかのフラグを倒す
		Frame->LocalWorldMatrixUseScaling = false ;

		// 親行列をセットする
		if( FrameBase->IgnoreParentTransform || Frame->Parent == NULL )
		{
			ParentMatrix = &Model->LocalWorldMatrix ;
			ParentUseScaling = Model->LocalWorldMatrixUseScaling ;
		}
		else
		{
			ParentMatrix = &Frame->Parent->LocalWorldMatrix ;
			ParentUseScaling = Frame->Parent->LocalWorldMatrixUseScaling ;
		}

		// ユーザー指定の行列がある場合はそれを使用する
		if( Frame->ValidUserLocalTransformMatrix )
		{
			UnSafeMultiplyMatrix4X4CT( &Frame->LocalWorldMatrix, &Frame->UserLocalTransformMatrix, ParentMatrix ) ;

			// スケーリングを使用しているかどうかのフラグを立てる
			Frame->LocalWorldMatrixUseScaling = true ;
		}
		else
		// アニメーションがある場合と無い場合で処理を分岐
		if( Model->AnimSetNum == 0 )
		{
			// アニメーションがない場合
			UnSafeMultiplyMatrix4X4CT_FC( &Frame->LocalWorldMatrix, &FrameBase->LocalTransformMatrix, ParentMatrix ) ;

			// 親のフレーム又はデフォルトの設定でスケーリングを使用していたらスケーリングを使用しているかどうかのフラグを立てる
			if( FrameBase->LocalTransformMatrixUseScaling || ParentUseScaling )
			{
				Frame->LocalWorldMatrixUseScaling = true ;
			}
		}
		else
		{
			// アニメーションがある場合
			MATRIX_4X4CT_F BlendMat ;
			float BlendRate ;
			VECTOR Translate, Scale, Rotate ;
			FLOAT4 Quaternion ;
			int BlendFlag ;

			// パラメータレベルのブレンドが行えるかを調べる
			MAnim2 = MAnim ;
			BlendFlag = 0 ;
			mcon = 0 ;
			for( j = 0 ; j < Model->AnimSetMaxNum ; j ++, MAnim2 ++ )
			{
				if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f )
					continue ;
				mcon ++ ;
				MAnim3 = MAnim2 ;

				BlendFlag |= MAnim2->Anim->ValidFlag ;
			}

			// アニメーションが再生されていない場合はデフォルトの行列を適応する
			if( mcon == 0 )
			{
				if( FrameBase->LocalTransformMatrixType == 0 )
				{
					if( Frame->Parent == NULL )
					{
						Frame->LocalWorldMatrix = Model->LocalWorldMatrix ;
						Frame->LocalWorldMatrixUseScaling = Model->LocalWorldMatrixUseScaling ;
					}
					else
					{
						Frame->LocalWorldMatrix = Frame->Parent->LocalWorldMatrix ;
						Frame->LocalWorldMatrixUseScaling = Frame->Parent->LocalWorldMatrixUseScaling ;
					}
				}
				else
				if( FrameBase->LocalTransformMatrixType == 1 )
				{
					UnSafeTranslateOnlyMultiplyMatrix4X4CT_FC( &Frame->LocalWorldMatrix, &FrameBase->LocalTransformMatrix, ParentMatrix ) ;
					if( FrameBase->LocalTransformMatrixUseScaling || ParentUseScaling )
					{
						Frame->LocalWorldMatrixUseScaling = true ;
					}
				}
				else
				{
					UnSafeMultiplyMatrix4X4CT_FC( &Frame->LocalWorldMatrix, &FrameBase->LocalTransformMatrix, ParentMatrix ) ;
					if( FrameBase->LocalTransformMatrixUseScaling || ParentUseScaling )
					{
						Frame->LocalWorldMatrixUseScaling = true ;
					}
				}
			}
			else
			// アニメーションが一つだけ再生されている場合は別処理
			if( mcon == 1 )
			{
				// 行列のセットアップ
				if( BlendFlag & MV1_ANIMVALUE_MATRIX )
				{
					// 親フレームの行列を掛ける
					UnSafeMultiplyMatrix4X4CT_FC( &Frame->LocalWorldMatrix, &MAnim3->Anim->Matrix, ParentMatrix ) ;

					Frame->LocalWorldMatrixUseScaling = true ;
				}
				else
				{
					// 親フレームの行列を掛ける
					if( MAnim3->Anim->ValidBlendMatrix == false )
					{
						MV1SetupTransformMatrix(
							&MAnim3->Anim->BlendMatrix,
							BlendFlag,
							&MAnim3->Anim->Translate,
							&MAnim3->Anim->Scale,
							MAnim3->Anim->RotateOrder,
							( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
							&MAnim3->Anim->Rotate,
							( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
							&MAnim3->Anim->Quaternion
						) ;

						MAnim3->Anim->ValidBlendMatrix = true ;

						if( BlendFlag & MV1_ANIMVALUE_SCALE )
						{
							MAnim3->Anim->BlendMatrixUseScaling = true ;
						}
					}
					BlendMat = MAnim3->Anim->BlendMatrix ;

					if( MAnim3->Anim->BlendMatrixUseScaling )
					{
						Frame->LocalWorldMatrixUseScaling = true ;
					}

					/*
					NS_ErrorLogFmtAdd( "No:%d Flag:%d Trans:%.2f,%.2f,%.2f Scale:%.2f,%.2f,%.2f RotO:%d Rotate:%.2f,%.2f,%.2f Qt:%.2f,%.2f,%.2f,%.2f        Name:%s",
						i,
						BlendFlag,
						MAnim3->Anim->Translate.x, MAnim3->Anim->Translate.y, MAnim3->Anim->Translate.z, 
						MAnim3->Anim->Scale.x, MAnim3->Anim->Scale.y, MAnim3->Anim->Scale.z,
						MAnim3->Anim->RotateOrder,
						MAnim3->Anim->Rotate.x, MAnim3->Anim->Rotate.y, MAnim3->Anim->Rotate.z,
						MAnim3->Anim->Quaternion.x, MAnim3->Anim->Quaternion.y, MAnim3->Anim->Quaternion.z, MAnim3->Anim->Quaternion.w, 
						Frame->BaseData->Name ); 
					*/

					/*
					NS_ErrorLogFmtAdd( "no:%d Mat:%.2f,%.2f,%.2f,%.2f, %.2f,%.2f,%.2f,%.2f, %.2f,%.2f,%.2f,%.2f",
						i,
						BlendMat.m[0][0], BlendMat.m[0][1], BlendMat.m[0][2], BlendMat.m[0][3], 
						BlendMat.m[1][0], BlendMat.m[1][1], BlendMat.m[1][2], BlendMat.m[1][3], 
						BlendMat.m[2][0], BlendMat.m[2][1], BlendMat.m[2][2], BlendMat.m[2][3] );
					*/

					UnSafeMultiplyMatrix4X4CT_FC( &Frame->LocalWorldMatrix, &BlendMat, ParentMatrix ) ;
					//NS_ErrorLogFmtAdd( "no:%d proc:%d", i, 6 );
				}
			}
			else
			{
				MV1_ANIM * RST Anim ;

				// 行列があるか、クォータニオンとＸＹＺ軸回転が混同しているか
				// デフォルトパラメータが無効な上に当ててあるアニメーションの種類が違う場合は行列ブレンド
//				if( ( BlendFlag & MV1_ANIMVALUE_MATRIX ) ||
//					( BlendFlag & ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_X ) ) == ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_X ) ||
//					( BlendFlag & ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_VMD ) ) == ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_VMD ) )
				if( 1 )
				{
					_MEMSET( &BlendMat, 0, sizeof( BlendMat ) ) ;
					MAnim2 = MAnim ;
					for( j = 0 ; j < Model->AnimSetMaxNum ; j ++, MAnim2 ++ )
					{
						if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f ) continue ;

//						BlendRate = Model->AnimSet[ j ].BlendRate ;
						BlendRate = MAnim2->BlendRate ;
						Anim = MAnim2->Anim ;

						if( Anim->ValidFlag & MV1_ANIMVALUE_MATRIX )
						{
							if( BlendRate == 1.0f )
							{
								UnSafeMatrix4X4CT_F_EqPlus_F_Sub_F( &BlendMat, &Anim->Matrix, &FrameBase->LocalTransformMatrix ) ;
							}
							else
							{
								UnSafeMatrix4X4CT_F_EqPlus_F_Sub_F_Mul_S( &BlendMat, &Anim->Matrix, &FrameBase->LocalTransformMatrix, BlendRate ) ;
							}

							Frame->LocalWorldMatrixUseScaling = true ;
						}
						else
						{
							// 行列のセットアップ
							if( Anim->ValidBlendMatrix == false )
							{
								MV1SetupTransformMatrix(
									&Anim->BlendMatrix,
									Anim->ValidFlag,
									&Anim->Translate,
									&Anim->Scale,
									Anim->RotateOrder,
									( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
									&Anim->Rotate,
									( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
									&Anim->Quaternion ) ;
								Anim->ValidBlendMatrix = true ;

								if( ( Anim->ValidFlag & MV1_ANIMVALUE_SCALE ) &&
									( Anim->Scale.x < 0.9999999f || Anim->Scale.x > 1.0000001f ||
									  Anim->Scale.y < 0.9999999f || Anim->Scale.y > 1.0000001f ||
									  Anim->Scale.z < 0.9999999f || Anim->Scale.z > 1.0000001f ) )
								{
									Anim->BlendMatrixUseScaling = true ;
								}
							}

							if( Anim->BlendMatrixUseScaling )
							{
								Frame->LocalWorldMatrixUseScaling = true ;
							}

							if( BlendRate == 1.0f )
							{
								UnSafeMatrix4X4CT_F_EqPlus_F_Sub_F( &BlendMat, &Anim->BlendMatrix, &FrameBase->LocalTransformMatrix ) ;
							}
							else
							{
								UnSafeMatrix4X4CT_F_EqPlus_F_Sub_F_Mul_S( &BlendMat, &Anim->BlendMatrix, &FrameBase->LocalTransformMatrix, BlendRate ) ;
							}
						}
					}

					UnSafeMatrix4X4CT_F_EqPlus_F( &BlendMat, &FrameBase->LocalTransformMatrix ) ;

					if( FrameBase->LocalTransformMatrixUseScaling )
					{
						Frame->LocalWorldMatrixUseScaling = true ;
					}

					// スケーリングが使用されていなかったら行列の回転部分の正規化
					if( Frame->LocalWorldMatrixUseScaling == false )
					{
						DivSize.x = 1.0f / _SQRT( BlendMat.m[ 0 ][ 0 ] * BlendMat.m[ 0 ][ 0 ] + BlendMat.m[ 0 ][ 1 ] * BlendMat.m[ 0 ][ 1 ] + BlendMat.m[ 0 ][ 2 ] * BlendMat.m[ 0 ][ 2 ] ) ;
						DivSize.y = 1.0f / _SQRT( BlendMat.m[ 1 ][ 0 ] * BlendMat.m[ 1 ][ 0 ] + BlendMat.m[ 1 ][ 1 ] * BlendMat.m[ 1 ][ 1 ] + BlendMat.m[ 1 ][ 2 ] * BlendMat.m[ 1 ][ 2 ] ) ;
						DivSize.z = 1.0f / _SQRT( BlendMat.m[ 2 ][ 0 ] * BlendMat.m[ 2 ][ 0 ] + BlendMat.m[ 2 ][ 1 ] * BlendMat.m[ 2 ][ 1 ] + BlendMat.m[ 2 ][ 2 ] * BlendMat.m[ 2 ][ 2 ] ) ;

						BlendMat.m[ 0 ][ 0 ] *= DivSize.x ;
						BlendMat.m[ 0 ][ 1 ] *= DivSize.x ;
						BlendMat.m[ 0 ][ 2 ] *= DivSize.x ;

						BlendMat.m[ 1 ][ 0 ] *= DivSize.y ;
						BlendMat.m[ 1 ][ 1 ] *= DivSize.y ;
						BlendMat.m[ 1 ][ 2 ] *= DivSize.y ;

						BlendMat.m[ 2 ][ 0 ] *= DivSize.z ;
						BlendMat.m[ 2 ][ 1 ] *= DivSize.z ;
						BlendMat.m[ 2 ][ 2 ] *= DivSize.z ;
					}
				}
				else
				{
					// それ以外の場合はパラメータレベルのブレンド処理
					Translate.x = 0.0f ;
					Translate.y = 0.0f ;
					Translate.z = 0.0f ;
					Scale.x = 0.0f ;
					Scale.y = 0.0f ;
					Scale.z = 0.0f ;
					Rotate.x = 0.0f ;
					Rotate.y = 0.0f ;
					Rotate.z = 0.0f ;
					Quaternion.x = 0.0f ;
					Quaternion.y = 0.0f ;
					Quaternion.z = 0.0f ;
					Quaternion.w = 1.0f ;

					MAnim2 = MAnim ;
					for( j = 0 ; j < Model->AnimSetMaxNum ; j ++, MAnim2 ++ )
					{
						if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f ) continue ;

						BlendRate = MAnim2->BlendRate ;
						Anim = MAnim2->Anim ;

						if( BlendRate == 1.0f )
						{
							if( Anim->ValidFlag & MV1_ANIMVALUE_TRANSLATE )
							{
								Translate.x += Anim->Translate.x - ( float )FrameBase->Translate.x ;
								Translate.y += Anim->Translate.y - ( float )FrameBase->Translate.y ;
								Translate.z += Anim->Translate.z - ( float )FrameBase->Translate.z ;
							}

							if( Anim->ValidFlag & MV1_ANIMVALUE_SCALE )
							{
								Scale.x += Anim->Scale.x - FrameBase->Scale.x ;
								Scale.y += Anim->Scale.y - FrameBase->Scale.y ;
								Scale.z += Anim->Scale.z - FrameBase->Scale.z ;
							}

							if( Anim->ValidFlag & MV1_ANIMVALUE_ROTATE )
							{
								Rotate.x += Anim->Rotate.x - FrameBase->Rotate.x ;
								Rotate.y += Anim->Rotate.y - FrameBase->Rotate.y ;
								Rotate.z += Anim->Rotate.z - FrameBase->Rotate.z ;
							}

							if( Anim->ValidFlag & ( MV1_ANIMVALUE_QUATERNION_X | MV1_ANIMVALUE_QUATERNION_VMD ) )
							{
								Quaternion.x = Anim->Quaternion.x - FrameBase->Quaternion.x ;
								Quaternion.y = Anim->Quaternion.y - FrameBase->Quaternion.y ;
								Quaternion.z = Anim->Quaternion.z - FrameBase->Quaternion.z ;
								Quaternion.w = Anim->Quaternion.w - FrameBase->Quaternion.w ;
							}
						}
						else
						{
							if( Anim->ValidFlag & MV1_ANIMVALUE_TRANSLATE )
							{
								Translate.x += ( Anim->Translate.x - ( float )FrameBase->Translate.x ) * BlendRate ;
								Translate.y += ( Anim->Translate.y - ( float )FrameBase->Translate.y ) * BlendRate ;
								Translate.z += ( Anim->Translate.z - ( float )FrameBase->Translate.z ) * BlendRate ;
							}

							if( Anim->ValidFlag & MV1_ANIMVALUE_SCALE )
							{
								Scale.x += ( Anim->Scale.x - FrameBase->Scale.x ) * BlendRate ;
								Scale.y += ( Anim->Scale.y - FrameBase->Scale.y ) * BlendRate ;
								Scale.z += ( Anim->Scale.z - FrameBase->Scale.z ) * BlendRate ;
							}

							if( Anim->ValidFlag & MV1_ANIMVALUE_ROTATE )
							{
								Rotate.x += ( Anim->Rotate.x - FrameBase->Rotate.x ) * BlendRate ;
								Rotate.y += ( Anim->Rotate.y - FrameBase->Rotate.y ) * BlendRate ;
								Rotate.z += ( Anim->Rotate.z - FrameBase->Rotate.z ) * BlendRate ;
							}

							if( Anim->ValidFlag & ( MV1_ANIMVALUE_QUATERNION_X | MV1_ANIMVALUE_QUATERNION_VMD ) )
							{
								Quaternion.x = ( Anim->Quaternion.x - FrameBase->Quaternion.x ) * BlendRate ;
								Quaternion.y = ( Anim->Quaternion.y - FrameBase->Quaternion.y ) * BlendRate ;
								Quaternion.z = ( Anim->Quaternion.z - FrameBase->Quaternion.z ) * BlendRate ;
								Quaternion.w = ( Anim->Quaternion.w - FrameBase->Quaternion.w ) * BlendRate ;
							}
						}
					}

					Translate.x += ( float )FrameBase->Translate.x ;
					Translate.y += ( float )FrameBase->Translate.y ;
					Translate.z += ( float )FrameBase->Translate.z ;

					Scale.x += FrameBase->Scale.x ;
					Scale.y += FrameBase->Scale.y ;
					Scale.z += FrameBase->Scale.z ;

					Rotate.x += FrameBase->Rotate.x ;
					Rotate.y += FrameBase->Rotate.y ;
					Rotate.z += FrameBase->Rotate.z ;

					Quaternion.x += FrameBase->Quaternion.x ;
					Quaternion.y += FrameBase->Quaternion.y ;
					Quaternion.z += FrameBase->Quaternion.z ;
					Quaternion.w += FrameBase->Quaternion.w ;

					// 行列のセットアップ
					MV1SetupTransformMatrix(
						&BlendMat,
						BlendFlag,
						&Translate,
						&Scale,
						FrameBase->RotateOrder,
						( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
						&Rotate,
						( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
						&Quaternion
					) ;

					if( BlendFlag & MV1_ANIMVALUE_SCALE )
					{
						Frame->LocalWorldMatrixUseScaling = true ;
					}
				}

				// 親フレームの行列を掛ける
				UnSafeMultiplyMatrix4X4CT_FC( &Frame->LocalWorldMatrix, &BlendMat, ParentMatrix ) ;
				if( ParentUseScaling )
				{
					Frame->LocalWorldMatrixUseScaling = true ;
				}
			}
		}

		// 固定機能パイプライン用行列を無効にする
		Frame->ValidLocalWorldMatrixNM = false ;

		// このフレームを対象としているボーンがある場合は、そのボーンの行列も更新する
		SetupSkiningBoneMatrix( *Model, *Frame ) ;

		//NS_ErrorLogFmtAdd( "no:%d parent:%d", i, Frame->Parent != NULL ? ( Frame - Model->Frame ) - ( Frame->Parent - Model->Frame ) : -1 );

		/*
		NS_ErrorLogFmtAdd( "no:%d Mat:%.2f,%.2f,%.2f,%.2f, %.2f,%.2f,%.2f,%.2f, %.2f,%.2f,%.2f,%.2f",
			i,
			Frame->LocalWorldMatrix.m[0][0], Frame->LocalWorldMatrix.m[0][1], Frame->LocalWorldMatrix.m[0][2], Frame->LocalWorldMatrix.m[0][3], 
			Frame->LocalWorldMatrix.m[1][0], Frame->LocalWorldMatrix.m[1][1], Frame->LocalWorldMatrix.m[1][2], Frame->LocalWorldMatrix.m[1][3], 
			Frame->LocalWorldMatrix.m[2][0], Frame->LocalWorldMatrix.m[2][1], Frame->LocalWorldMatrix.m[2][2], Frame->LocalWorldMatrix.m[2][3] );
		*/
	}
/*
	for( i = 0; i < Model->BaseData->SkinBoneNum ; i ++ )
	{
		NS_ErrorLogFmtAdd( "Mat%d:%.2f,%.2f,%.2f,%.2f, %.2f,%.2f,%.2f,%.2f, %.2f,%.2f,%.2f,%.2f",
			i,
			Model->SkinBoneMatrix[i].m[0][0], Model->SkinBoneMatrix[i].m[0][1], Model->SkinBoneMatrix[i].m[0][2], Model->SkinBoneMatrix[i].m[0][3], 
			Model->SkinBoneMatrix[i].m[1][0], Model->SkinBoneMatrix[i].m[1][1], Model->SkinBoneMatrix[i].m[1][2], Model->SkinBoneMatrix[i].m[1][3], 
			Model->SkinBoneMatrix[i].m[2][0], Model->SkinBoneMatrix[i].m[2][1], Model->SkinBoneMatrix[i].m[2][2], Model->SkinBoneMatrix[i].m[2][3] );
	}
*/
	// 更新が必要フラグをリセットする
	_MEMSET( Model->ChangeMatrixFlag, 0, Model->ChangeMatrixFlagSize ) ;

	// モデル中の行列セットアップ完了フラグを立てる
	Model->LocalWorldMatrixSetupFlag = true ;
}



#define MV1MATCOLOR_UPDATE( PARENT, CHILD )	\
	if( ( PARENT ).UseColorScale )\
	{\
		if( ( CHILD )->DrawMaterial.UseColorScale )\
		{\
			( CHILD )->SetupDrawMaterial.DiffuseScale.r = ( BYTE )( ( CHILD )->DrawMaterial.DiffuseScale.r * ( PARENT ).DiffuseScale.r ) ;\
			( CHILD )->SetupDrawMaterial.DiffuseScale.g = ( BYTE )( ( CHILD )->DrawMaterial.DiffuseScale.g * ( PARENT ).DiffuseScale.g ) ;\
			( CHILD )->SetupDrawMaterial.DiffuseScale.b = ( BYTE )( ( CHILD )->DrawMaterial.DiffuseScale.b * ( PARENT ).DiffuseScale.b ) ;\
			( CHILD )->SetupDrawMaterial.DiffuseScale.a = ( BYTE )( ( CHILD )->DrawMaterial.DiffuseScale.a * ( PARENT ).DiffuseScale.a ) ;\
			( CHILD )->SetupDrawMaterial.AmbientScale.r = ( BYTE )( ( CHILD )->DrawMaterial.AmbientScale.r * ( PARENT ).AmbientScale.r ) ;\
			( CHILD )->SetupDrawMaterial.AmbientScale.g = ( BYTE )( ( CHILD )->DrawMaterial.AmbientScale.g * ( PARENT ).AmbientScale.g ) ;\
			( CHILD )->SetupDrawMaterial.AmbientScale.b = ( BYTE )( ( CHILD )->DrawMaterial.AmbientScale.b * ( PARENT ).AmbientScale.b ) ;\
			( CHILD )->SetupDrawMaterial.AmbientScale.a = ( BYTE )( ( CHILD )->DrawMaterial.AmbientScale.a * ( PARENT ).AmbientScale.a ) ;\
			( CHILD )->SetupDrawMaterial.SpecularScale.r = ( BYTE )( ( CHILD )->DrawMaterial.SpecularScale.r * ( PARENT ).SpecularScale.r ) ;\
			( CHILD )->SetupDrawMaterial.SpecularScale.g = ( BYTE )( ( CHILD )->DrawMaterial.SpecularScale.g * ( PARENT ).SpecularScale.g ) ;\
			( CHILD )->SetupDrawMaterial.SpecularScale.b = ( BYTE )( ( CHILD )->DrawMaterial.SpecularScale.b * ( PARENT ).SpecularScale.b ) ;\
			( CHILD )->SetupDrawMaterial.SpecularScale.a = ( BYTE )( ( CHILD )->DrawMaterial.SpecularScale.a * ( PARENT ).SpecularScale.a ) ;\
			( CHILD )->SetupDrawMaterial.EmissiveScale.r = ( BYTE )( ( CHILD )->DrawMaterial.EmissiveScale.r * ( PARENT ).EmissiveScale.r ) ;\
			( CHILD )->SetupDrawMaterial.EmissiveScale.g = ( BYTE )( ( CHILD )->DrawMaterial.EmissiveScale.g * ( PARENT ).EmissiveScale.g ) ;\
			( CHILD )->SetupDrawMaterial.EmissiveScale.b = ( BYTE )( ( CHILD )->DrawMaterial.EmissiveScale.b * ( PARENT ).EmissiveScale.b ) ;\
			( CHILD )->SetupDrawMaterial.EmissiveScale.a = ( BYTE )( ( CHILD )->DrawMaterial.EmissiveScale.a * ( PARENT ).EmissiveScale.a ) ;\
		}\
		else\
		{\
			( CHILD )->SetupDrawMaterial.DiffuseScale = ( PARENT ).DiffuseScale ;\
			( CHILD )->SetupDrawMaterial.AmbientScale = ( PARENT ).AmbientScale ;\
			( CHILD )->SetupDrawMaterial.SpecularScale = ( PARENT ).SpecularScale ;\
			( CHILD )->SetupDrawMaterial.EmissiveScale = ( PARENT ).EmissiveScale ;\
		}\
		( CHILD )->SetupDrawMaterial.UseColorScale = true ;\
	}\
	else\
	{\
		if( ( CHILD )->DrawMaterial.UseColorScale )\
		{\
			( CHILD )->SetupDrawMaterial.DiffuseScale = ( CHILD )->DrawMaterial.DiffuseScale ;\
			( CHILD )->SetupDrawMaterial.AmbientScale = ( CHILD )->DrawMaterial.AmbientScale ;\
			( CHILD )->SetupDrawMaterial.SpecularScale = ( CHILD )->DrawMaterial.SpecularScale ;\
			( CHILD )->SetupDrawMaterial.EmissiveScale = ( CHILD )->DrawMaterial.EmissiveScale ;\
		}\
		( CHILD )->SetupDrawMaterial.UseColorScale = ( CHILD )->DrawMaterial.UseColorScale ;\
	}\
\
	( CHILD )->SetupDrawMaterial.OpacityRate = ( CHILD )->DrawMaterial.OpacityRate * ( PARENT ).OpacityRate ;\
	( CHILD )->SetupDrawMaterial.Visible     = ( BYTE )( ( CHILD )->DrawMaterial.Visible != 0 && ( PARENT ).Visible != 0 ? 1 : 0 ) ;

// 描画用のマテリアル情報を構築する( 中でメッシュ自体の更新チェックはしません )
static void MV1SetupDrawMaterial( MV1_FRAME *Frame, MV1_MESH *Mesh )
{
	int StackNum, i ;
	MV1_MODEL *Model ;
	MV1_FRAME *TopFrame, *TempFrame, *ParentFrame, *StackFrame[ 1024 ] ;
	MV1_MESH *TempMesh ;
	char NextFlag[ 1024 ] ;

	if( Frame == NULL ) Frame = Mesh->Container ;
	Model = Frame->Container ;

	// 自分に影響がある最上級フレームの更新
	if( MV1CCHK( Frame->DrawMaterialChange ) != 0 )
	{
		// トップが更新されていたらそこから
		if( Model->ChangeDrawMaterialFlag[ 0 ] & 1 )
		{
			_MEMSET( Model->ChangeDrawMaterialFlag, 0, Model->ChangeDrawMaterialFlagSize ) ;

			StackFrame[ 0 ] = Model->Frame ;
			NextFlag[ 0 ] = 1 ;
		}
		else
		{
			for( TopFrame = Frame ; TopFrame->Parent && MV1CCHK( TopFrame->Parent->DrawMaterialChange ) ; TopFrame = TopFrame->Parent ){}

			MV1BitResetChange( &TopFrame->DrawMaterialChange ) ;

			StackFrame[ 0 ] = TopFrame ;
			NextFlag[ 0 ] = 0 ;
		}
		StackNum = 1 ;

		while( StackNum )
		{
			StackNum -- ;
			TempFrame = StackFrame[ StackNum ] ;

			if( TempFrame->Parent == NULL )
			{
				MV1MATCOLOR_UPDATE( Model->DrawMaterial, TempFrame )
			}
			else
			{
				ParentFrame = TempFrame->Parent ;
				MV1MATCOLOR_UPDATE( ParentFrame->SetupDrawMaterial, TempFrame )
			}
			TempFrame->SemiTransStateSetupFlag = false ;

			if( TempFrame->BaseData->MeshNum )
			{
				TempMesh = TempFrame->Mesh ;
				for( i = 0 ; i < TempFrame->BaseData->MeshNum ; i ++, TempMesh ++ )
				{
					MV1MATCOLOR_UPDATE( TempFrame->SetupDrawMaterial, TempMesh )
					TempMesh->SemiTransStateSetupFlag = false ;
				}
			}

			if( NextFlag[ StackNum ] && TempFrame->Next )
			{
				StackFrame[ StackNum ] = TempFrame->Next ;
				NextFlag[ StackNum ] = 1 ;
				StackNum ++ ;
			}

			if( TempFrame->Child )
			{
				StackFrame[ StackNum ] = TempFrame->Child ;
				NextFlag[ StackNum ] = 1 ;
				StackNum ++ ;
			}
		}
	}
	else
	{
		if( Mesh )
		{
			MV1MATCOLOR_UPDATE( Frame->SetupDrawMaterial, Mesh )
			MV1CRST( Mesh->DrawMaterialChange ) ;
			Mesh->SemiTransStateSetupFlag = false ;
		}
	}
}

// 状態変更管理データに設定されている対象ビットを立てる
static void MV1BitSetChange( MV1_CHANGE *Change )
{
	// 既にビットが立っていたら何もしない
	if( MV1CCHK( *Change ) ) return ;

	if( Change->BaseData->Fill )
	{
		DWORD i, Size ;
		DWORD *Target, *Src ;

		Target = Change->Target ;
		Src    = Change->BaseData->Fill ;
		Size   = Change->BaseData->Size ;
		for( i = 0 ; i < Size ; i ++ )
			Target[ i ] |= Src[ i ] ;
	}
	else
	{
		*Change->Target |= Change->BaseData->CheckBit ;
	}
}

// 状態変更管理データに設定されている対象ビットを倒す
static void MV1BitResetChange( MV1_CHANGE *Change )
{
	if( Change->BaseData->Fill )
	{
		DWORD i, Size ;
		DWORD *Target, *Src ;

		Target = Change->Target ;
		Src    = Change->BaseData->Fill ;
		Size   = Change->BaseData->Size ;
		for( i = 0 ; i < Size ; i ++ )
			Target[ i ] &= ~Src[ i ] ;
	}
	else
	{
		*Change->Target &= ~Change->BaseData->CheckBit ;
	}
}

// 描画用マテリアル情報のディフューズカラーを変更する
static bool MV1SetDrawMaterialDif( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale )
{
	// 今までと値が同じ場合は何もせず終了
	if( *( ( DWORD * )&Scale.r ) == *( ( DWORD * )&DrawMaterial->DiffuseScale.r ) &&
		*( ( DWORD * )&Scale.g ) == *( ( DWORD * )&DrawMaterial->DiffuseScale.g ) &&
		*( ( DWORD * )&Scale.b ) == *( ( DWORD * )&DrawMaterial->DiffuseScale.b ) &&
		*( ( DWORD * )&Scale.a ) == *( ( DWORD * )&DrawMaterial->DiffuseScale.a ) )
		return false ;

	// 全ての要素が 1.0f かどうかで処理を分岐
	if( *( ( DWORD * )&Scale.r ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.g ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.b ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.a ) == 0x3f800000 )
	{
		// ディフューズカラーに 1.0f を代入
		*( ( DWORD * )&DrawMaterial->DiffuseScale.r ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->DiffuseScale.g ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->DiffuseScale.b ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->DiffuseScale.a ) = 0x3f800000 ;

		// 他の要素が 1.0f 以外か調べる
		if( *( ( DWORD * )&DrawMaterial->AmbientScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->SpecularScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->EmissiveScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.a ) == 0x3f800000 )
		{
			// カラースケールは使用していない、にする
			DrawMaterial->UseColorScale = false ;
		}
	}
	else
	{
		// ディフーズカラーに値を代入
		DrawMaterial->DiffuseScale = Scale ;

		// カラースケールを使用している、にする
		DrawMaterial->UseColorScale = true ;
	}

	// 変更の印を付ける
	if( Change ) MV1BitSetChange( Change ) ;

	return true ;
}

// 描画用マテリアル情報のアンビエントカラーを変更する
static bool MV1SetDrawMaterialAmb( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale )
{
	// 今までと値が同じ場合は何もせず終了
	if( *( ( DWORD * )&Scale.r ) == *( ( DWORD * )&DrawMaterial->AmbientScale.r ) &&
		*( ( DWORD * )&Scale.g ) == *( ( DWORD * )&DrawMaterial->AmbientScale.g ) &&
		*( ( DWORD * )&Scale.b ) == *( ( DWORD * )&DrawMaterial->AmbientScale.b ) &&
		*( ( DWORD * )&Scale.a ) == *( ( DWORD * )&DrawMaterial->AmbientScale.a ) )
		return false ;

	// 全ての要素が 1.0f かどうかで処理を分岐
	if( *( ( DWORD * )&Scale.r ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.g ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.b ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.a ) == 0x3f800000 )
	{
		// アンビエントカラーに 1.0f を代入
		*( ( DWORD * )&DrawMaterial->AmbientScale.r ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->AmbientScale.g ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->AmbientScale.b ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->AmbientScale.a ) = 0x3f800000 ;

		// 他の要素が 1.0f 以外か調べる
		if( *( ( DWORD * )&DrawMaterial->DiffuseScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->SpecularScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->EmissiveScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.a ) == 0x3f800000 )
		{
			// カラースケールは使用していない、にする
			DrawMaterial->UseColorScale = false ;
		}
	}
	else
	{
		// アンビエントカラーに値を代入
		DrawMaterial->AmbientScale = Scale ;

		// カラースケールを使用している、にする
		DrawMaterial->UseColorScale = true ;
	}

	// 変更の印を付ける
	if( Change ) MV1BitSetChange( Change ) ;

	return true ;
}

// 描画用マテリアル情報のスペキュラカラーを変更する
static bool MV1SetDrawMaterialSpc( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale )
{
	// 今までと値が同じ場合は何もせず終了
	if( *( ( DWORD * )&Scale.r ) == *( ( DWORD * )&DrawMaterial->SpecularScale.r ) &&
		*( ( DWORD * )&Scale.g ) == *( ( DWORD * )&DrawMaterial->SpecularScale.g ) &&
		*( ( DWORD * )&Scale.b ) == *( ( DWORD * )&DrawMaterial->SpecularScale.b ) &&
		*( ( DWORD * )&Scale.a ) == *( ( DWORD * )&DrawMaterial->SpecularScale.a ) )
		return false ;

	// 全ての要素が 1.0f かどうかで処理を分岐
	if( *( ( DWORD * )&Scale.r ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.g ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.b ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.a ) == 0x3f800000 )
	{
		// スペキュラカラーに 1.0f を代入
		*( ( DWORD * )&DrawMaterial->SpecularScale.r ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->SpecularScale.g ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->SpecularScale.b ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->SpecularScale.a ) = 0x3f800000 ;

		// 他の要素が 1.0f 以外か調べる
		if( *( ( DWORD * )&DrawMaterial->DiffuseScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->AmbientScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->EmissiveScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->EmissiveScale.a ) == 0x3f800000 )
		{
			// カラースケールは使用していない、にする
			DrawMaterial->UseColorScale = false ;
		}
	}
	else
	{
		// スペキュラカラーに値を代入
		DrawMaterial->SpecularScale = Scale ;

		// カラースケールを使用している、にする
		DrawMaterial->UseColorScale = true ;
	}

	// 変更の印を付ける
	if( Change ) MV1BitSetChange( Change ) ;

	return true ;
}

// 描画用マテリアル情報のエミッシブカラーを変更する
static bool MV1SetDrawMaterialEmi( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, COLOR_F Scale )
{
	// 今までと値が同じ場合は何もせず終了
	if( *( ( DWORD * )&Scale.r ) == *( ( DWORD * )&DrawMaterial->EmissiveScale.r ) &&
		*( ( DWORD * )&Scale.g ) == *( ( DWORD * )&DrawMaterial->EmissiveScale.g ) &&
		*( ( DWORD * )&Scale.b ) == *( ( DWORD * )&DrawMaterial->EmissiveScale.b ) &&
		*( ( DWORD * )&Scale.a ) == *( ( DWORD * )&DrawMaterial->EmissiveScale.a ) )
		return false ;

	// 全ての要素が 1.0f かどうかで処理を分岐
	if( *( ( DWORD * )&Scale.r ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.g ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.b ) == 0x3f800000 &&
		*( ( DWORD * )&Scale.a ) == 0x3f800000 )
	{
		// エミッシブカラーに 1.0f を代入
		*( ( DWORD * )&DrawMaterial->EmissiveScale.r ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->EmissiveScale.g ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->EmissiveScale.b ) = 0x3f800000 ;
		*( ( DWORD * )&DrawMaterial->EmissiveScale.a ) = 0x3f800000 ;

		// 他の要素が 1.0f 以外か調べる
		if( *( ( DWORD * )&DrawMaterial->DiffuseScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->DiffuseScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->AmbientScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->AmbientScale.a ) == 0x3f800000 &&
		    *( ( DWORD * )&DrawMaterial->SpecularScale.r ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.g ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.b ) == 0x3f800000 &&
			*( ( DWORD * )&DrawMaterial->SpecularScale.a ) == 0x3f800000 )
		{
			// カラースケールは使用していない、にする
			DrawMaterial->UseColorScale = false ;
		}
	}
	else
	{
		// エミッシブカラーに値を代入
		DrawMaterial->EmissiveScale = Scale ;

		// カラースケールを使用している、にする
		DrawMaterial->UseColorScale = true ;
	}

	// 変更の印を付ける
	if( Change ) MV1BitSetChange( Change ) ;

	return true ;
}

// 描画用マテリアル情報の不透明度を変更する
static bool MV1SetDrawMaterialOpacityRate( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, float Rate )
{
	// 今までと値が同じ場合は何もせず終了
	if( *( ( DWORD * )&Rate ) == *( ( DWORD * )&DrawMaterial->OpacityRate ) )
		return false ;

	// 値を代入
	DrawMaterial->OpacityRate = Rate ;

	// 変更の印を付ける
	if( Change ) MV1BitSetChange( Change ) ;

	return true ;
}

// 描画用マテリアル情報の表示設定を変更する
static bool MV1SetDrawMaterialVisible( MV1_DRAW_MATERIAL *DrawMaterial, MV1_CHANGE *Change, BYTE Visible )
{
	// 今までと値が同じ場合は何もせず終了
	if( Visible == DrawMaterial->Visible )
		return false ;

	// 値を代入
	DrawMaterial->Visible = Visible ;

	// 変更の印を付ける
	if( Change ) MV1BitSetChange( Change ) ;

	return true ;
}

// グラデーション画像を再作成する
static int _MV1ReCreateGradationGraph( int GrHandle )
{
	DWORD i ;
	BASEIMAGE BaseImage ;
	DWORD *Dest, DestData ;
	LOADGRAPH_GPARAM GParam ;

	if( NS_CreateXRGB8ColorBaseImage( 256, 8, &BaseImage ) < 0 )
		return -1 ;

	Dest = ( DWORD * )BaseImage.GraphData ;
	for( i = 0 ; i < 256 ; i ++, Dest ++ )
	{
		DestData = i | ( i << 8 ) | ( i << 16 ) ;
		Dest[ 256 * 0 ] = DestData ;
		Dest[ 256 * 1 ] = DestData ;
		Dest[ 256 * 2 ] = DestData ;
		Dest[ 256 * 3 ] = DestData ;
		Dest[ 256 * 4 ] = DestData ;
		Dest[ 256 * 5 ] = DestData ;
		Dest[ 256 * 6 ] = DestData ;
		Dest[ 256 * 7 ] = DestData ;
	}

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;
	GParam.CreateGraphGParam.NotUseTransColor = TRUE ;
	Graphics_Image_InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam.CreateGraphGParam.InitHandleGParam, 32, TRUE, FALSE ) ;
	Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, TRUE, GrHandle, &BaseImage, NULL, TRUE, FALSE, FALSE, FALSE ) ;

	NS_ReleaseBaseImage( &BaseImage ) ;

	return GrHandle ;
}




// ビットデータリストを初期化する( -1:失敗  0:成功 )
int InitBitList( BITLIST *BitList, int BitDepth, int DataNum, MEMINFO **FirstMem )
{
	BitList->BitDepth = BitDepth ;
	BitList->UnitSize = ( BitDepth + 7 ) / 8 + 1 ;
	BitList->DataNum = 0 ;
	BitList->MaxDataNum = DataNum ;

	BitList->Data = ADDMEMAREA( ( size_t )( ( BitList->UnitSize + 4 ) * DataNum ), FirstMem ) ;
	if( BitList->Data == NULL )
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xd3\x30\xc3\x30\xc8\x30\xc7\x30\xfc\x30\xbf\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"ビットデータを格納するメモリの確保に失敗しました\n" @*/ ) ;
		return -1 ;
	}
	BitList->PressData = ( int * )( ( BYTE * )BitList->Data + BitList->UnitSize * DataNum ) ;

	// 正常終了
	return 0 ;
}

// ビットデータリスト中の指定の要素のビットを立てる
int SetBitList( BITLIST *BitList, int Index, int SetBitIndex )
{
	BYTE *Addr, *SetAddr, *FlagAddr ;
	int SetBit ;

	// データアドレスの計算
	Addr = ( BYTE * )BitList->Data + BitList->UnitSize * Index ;

	// ビットをセットするアドレスの計算
	SetAddr = Addr + SetBitIndex / 8 ;

	// フラグがあるアドレスの計算
	FlagAddr = Addr + BitList->UnitSize - 1 ;

	// 立てるビットを計算
	SetBit = 1 << ( SetBitIndex % 8 ) ;

	// 今まで一度でもデータを代入されたことが無かったらビットを立てる
	if( *FlagAddr == 0 )
	{
		*FlagAddr = 1 ;

		// データの数をインクリメント
		BitList->DataNum ++ ;
	}

	// ビットを立てる
	*SetAddr |= SetBit ;

	// 正常終了
	return 0 ;
}

// ビットデータリスト中の値の種類を抽出する( 出力側は初期化されている必要は無い )( -1:失敗  0:成功 )
int PressBitList( BITLIST *BitListDest, BITLIST *BitListSrc, MEMINFO **FirstMem )
{
	int i, j, k, l, m, UnitSize, vnum ;
	BYTE *SrcData ;
	BYTE *DestData ;

	// 抽出したデータを格納するビットデータリストの初期化
	if( InitBitList( BitListDest, BitListSrc->BitDepth, BitListSrc->MaxDataNum, FirstMem ) == -1 )
		return -1 ;

	// 抽出元のデータの数だけ繰り返し
	UnitSize = BitListSrc->UnitSize ;
	SrcData = ( BYTE * )BitListSrc->Data ;
	vnum = 0 ;
	for( i = 0 ; vnum < BitListSrc->DataNum ; i ++, SrcData += UnitSize )
	{
		// 使用されていないデータの場合は何もしない
		if( SrcData[ UnitSize - 1 ] == 0 )
		{
			BitListSrc->PressData[ i ] = -1 ;
			continue ;
		}
		vnum ++ ;

		// 抽出先に既に同じデータがあるかどうかを調べる
		DestData = ( BYTE * )BitListDest->Data ;
		for( j = 0 ; j < BitListDest->DataNum ; j ++, DestData += UnitSize )
		{
			for( k = 0 ; k < UnitSize - 1 && SrcData[ k ] == DestData[ k ] ; k ++ ){}
			if( k == UnitSize - 1 ) break ;
		}

		// 無かったら追加
		if( j == BitListDest->DataNum )
		{
			DestData = ( BYTE * )BitListDest->Data + UnitSize * BitListDest->DataNum ;

			// データをコピーすると同時に立っているビットの数も数える
			m = 0 ;
			for( k = 0 ; k < UnitSize - 1 ; k ++ )
			{
				for( l = 0 ; l < 8 ; l ++ )
					if( SrcData[ k ] & ( 1 << l ) ) m ++ ;
				DestData[ k ] = SrcData[ k ] ;
			}

			// ビットの数を保存
			DestData[ k ] = ( BYTE )m ;

			// 出力先データの数を増やす
			BitListDest->DataNum ++ ;
			BitListDest->PressData[ j ] = 1 ;
		}
		else
		{
			// あったらデータの数をインクリメント
			BitListDest->PressData[ j ] ++ ;
		}

		// 抽出番号をセット
		BitListSrc->PressData[ i ] = j ;
	}

	// 終了
	return 0 ;
}

// 指定のビットデータに一致するターゲット側のデータインデックスを得る( -1:エラー、又は無かった  -1以外:データインデックス )
int SearchBitList( BITLIST *BitListTarget, void *Buffer )
{
	int i, j, vnum, UnitSize ;
	BYTE *TargetData ;

	// 指定のデータと同じデータが対象のビットデータリストにあるかどうか調べる
	UnitSize = BitListTarget->UnitSize ;
	TargetData = ( BYTE * )BitListTarget->Data ;
	vnum = BitListTarget->DataNum ;
	for( i = 0 ; vnum ; i ++, TargetData += UnitSize )
	{
		if( ( TargetData[ UnitSize - 1 ] & 1 ) == 0 ) continue ;
		vnum -- ;

		for( j = 0 ; j < UnitSize - 1 && TargetData[ j ] == *( ( BYTE * )Buffer + j ) ; j ++ ){}
		if( j == UnitSize - 1 )
		{
			vnum ++ ;
			break ;
		}
	}

	// 結果を返す
	return vnum ? i : -1 ;
}

// 二つの要素が一致しているかどうかを取得する( 0:一致している  0:一致していない )
int CmpBitList( BITLIST *BitList1, int Index1, BITLIST *BitList2, int Index2 )
{
	return _MEMCMP( ( BYTE * )BitList1->Data + BitList1->UnitSize * Index1,
					( BYTE * )BitList2->Data + BitList2->UnitSize * Index2,
					BitList1->UnitSize - 1 ) == 0 ? 0 : 1 ;
}

// ソース側の指定のデータに一致するターゲット側のデータインデックスを得る( -1:エラー、又は無かった  -1以外:データインデックス )
int SearchBitList( BITLIST *BitListTarget, BITLIST *BitListSrc, int SrcIndex )
{
	// １データのサイズが違ったらエラー
	if( BitListSrc->UnitSize != BitListTarget->UnitSize )
		return -1 ;

	// 調べる
	return SearchBitList( BitListTarget, ( BYTE * )BitListSrc->Data + BitListSrc->UnitSize * SrcIndex ) ;
}


// ビットデータリストから指定番号のデータを取得する
int GetBitList( BITLIST *BitList, int Index, void *Buffer )
{
	int i ;
	BYTE *Dest, *Src ;

	Src  = ( BYTE * )BitList->Data + Index * BitList->UnitSize ;
	Dest = ( BYTE * )Buffer ;
	for( i = BitList->UnitSize - 2 ; i >= 0 ; i -- )
		Dest[ i ] = Src[ i ] ;

	*( ( BYTE * )Buffer + BitList->UnitSize - 1 ) = 0 ;

	// 終了
	return 0 ;
}

// 指定のビットデータを指定のインデックスにコピーする
int CopyBitList( BITLIST *BitListDest, int Index, void *Buffer )
{
	BYTE *Dest ;
	int i ;

	// 今まで使用されていなかったデータの場合はデータ数を増やす
	Dest = ( BYTE * )BitListDest->Data + BitListDest->UnitSize * Index ;
	if( ( Dest[ BitListDest->UnitSize - 1 ] & 1 ) == 0 )
	{
		BitListDest->DataNum ++ ;
	}

	// データをコピー
	for( i = 0 ; i < BitListDest->UnitSize ; i ++ )
		Dest[ i ] = ( ( BYTE * )Buffer )[ i ] ;

	// 終了
	return 0 ;
}

// 任意のビットデータとビットデータリスト中の指定のデータを Or 演算する
int OrBitList( BITLIST *BitList, int Index, void *Buffer )
{
	int i ;
	BYTE *Dest, *Src ;

	Src  = ( BYTE * )BitList->Data + Index * BitList->UnitSize ;
	Dest = ( BYTE * )Buffer ;
	for( i = BitList->UnitSize - 2 ; i >= 0 ; i -- )
		Dest[ i ] |= Src[ i ] ;

	// 終了
	return 0 ;
}

// 指定のビットデータ中、何ビット立っているか調べる
int GetBitCount( void *Buffer, int UnitSize )
{
	int i, m ;

	m = 0 ;
	for( i = 0 ; i < UnitSize - 1 ; i ++, Buffer = ( BYTE * )Buffer + 1 )
	{
		if( *( ( BYTE * )Buffer ) & ( 1 << 0 ) ) m ++ ;
		if( *( ( BYTE * )Buffer ) & ( 1 << 1 ) ) m ++ ;
		if( *( ( BYTE * )Buffer ) & ( 1 << 2 ) ) m ++ ;
		if( *( ( BYTE * )Buffer ) & ( 1 << 3 ) ) m ++ ;
		if( *( ( BYTE * )Buffer ) & ( 1 << 4 ) ) m ++ ;
		if( *( ( BYTE * )Buffer ) & ( 1 << 5 ) ) m ++ ;
		if( *( ( BYTE * )Buffer ) & ( 1 << 6 ) ) m ++ ;
		if( *( ( BYTE * )Buffer ) & ( 1 << 7 ) ) m ++ ;
	}

	return m ;
}

// 指定のビットデータを、ターゲット側のビットデータに加える
int AddBitList( BITLIST *BitListDest, void *Buffer, int RefCount )
{
	int DestIndex, i, UnitSize ;
	BYTE *Dest ;

	UnitSize = BitListDest->UnitSize ;

	// 既にあるかどうか調べる
	DestIndex = SearchBitList( BitListDest, Buffer ) ;
	if( DestIndex == -1 )
	{
		// 無かったら新規追加
		Dest = ( BYTE * )BitListDest->Data + UnitSize * BitListDest->DataNum ;
		for( i = 0 ; i < UnitSize ; i ++ )
			*( Dest + i ) = *( ( BYTE * )Buffer + i ) ;
		*( Dest + UnitSize - 1 ) |= 1 ;

		// 参照数をセット
		BitListDest->PressData[ BitListDest->DataNum ] = RefCount ;

		// データの数をインクリメント
		BitListDest->DataNum ++ ;
	}
	else
	{
		// あったら参照数を追加
		BitListDest->PressData[ DestIndex ] += RefCount ;
	}

	// 終了
	return 0 ;
}

// ソース側の指定のビットデータを、ターゲット側のビットデータに加える
int AddBitList( BITLIST *BitListDest, BITLIST *BitListSrc, int SrcIndex, int RefCount )
{
	// データサイズが違ったらエラー
	if( BitListDest->UnitSize != BitListSrc->UnitSize )
		return -1 ;

	// 追加する
	return AddBitList( BitListDest, ( BYTE * )BitListSrc->Data + BitListSrc->UnitSize * SrcIndex, RefCount ) ;
}

// 指定のビットデータ中で立っているビットのリストを取得する( 戻り値  -1:エラー  0以上:立っているビットの数 )
int GetBitListNumber( BITLIST *BitList, int Index, WORD *Buffer )
{
	BYTE *Src ;
	int i, ByteNum, BitNum, Num ;
	WORD BitCount ;

	Src = ( BYTE * )BitList->Data + Index * BitList->UnitSize ;
	ByteNum = BitList->BitDepth / 8 ;
	Num = 0 ;
	BitCount = 0 ;
	for( i = 0 ; i < ByteNum ; i ++, BitCount += 8, Src ++ )
	{
		if( *Src & 0x01 ){ Buffer[ Num ] = ( WORD )( BitCount + 0 ) ; Num ++ ; }
		if( *Src & 0x02 ){ Buffer[ Num ] = ( WORD )( BitCount + 1 ) ; Num ++ ; }
		if( *Src & 0x04 ){ Buffer[ Num ] = ( WORD )( BitCount + 2 ) ; Num ++ ; }
		if( *Src & 0x08 ){ Buffer[ Num ] = ( WORD )( BitCount + 3 ) ; Num ++ ; }
		if( *Src & 0x10 ){ Buffer[ Num ] = ( WORD )( BitCount + 4 ) ; Num ++ ; }
		if( *Src & 0x20 ){ Buffer[ Num ] = ( WORD )( BitCount + 5 ) ; Num ++ ; }
		if( *Src & 0x40 ){ Buffer[ Num ] = ( WORD )( BitCount + 6 ) ; Num ++ ; }
		if( *Src & 0x80 ){ Buffer[ Num ] = ( WORD )( BitCount + 7 ) ; Num ++ ; }
	}

	BitNum = BitList->BitDepth - ByteNum * 8 ;
	for( i = 0 ; i < BitNum ; i ++ )
	{
		if( *Src & ( 1 << i ) )
		{
			Buffer[ Num ] = ( WORD )( BitCount + i ) ;
			Num ++ ;
		}
	}

	// 終了
	return Num ;
}


// 16bit補助情報関係

// float型の値から MV1_ANIM_KEY_16BIT構造体の Min 変数用の値を作成する
extern BYTE MV1AnimKey16BitMinFtoB( float Min )
{
	float f ;

	// 最初に零かどうかを調べる
	if( ( *( ( DWORD * )&Min ) & 0x7fffffff ) == 0 )
	{
		return 0x80;
	}
	else
	{
		BYTE ret ;

		// 戻り値を初期化
		ret = 0 ;

		// 符号をセット
		if( *( ( DWORD * )&Min ) & 0x80000000 )
		{
			ret |= 0x40 ;
			*( ( DWORD * )&Min ) &= 0x7fffffff ;
		}

		// 乗数方向によって処理を分岐
		if( Min < 1.0f )
		{
			// 1.0f 以下
			ret |= 0x20 ;

			// ピッタリな値があるか調べる
			f = 0.1f ;               if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 1 ) ;
			f = 0.01f ;              if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 2 ) ;
			f = 0.001f ;             if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 3 ) ;
			f = 0.0001f ;            if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 4 ) ;
			f = 0.00001f ;           if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 5 ) ;
			f = 0.000001f ;          if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 6 ) ;
			f = 0.0000001f ;         if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 7 ) ;
			f = 0.00000001f ;        if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 8 ) ;
			f = 0.000000001f ;       if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 9 ) ;
			f = 0.0000000001f ;      if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 10 ) ;
			f = 0.00000000001f ;     if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 11 ) ;
			f = 0.000000000001f ;    if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 12 ) ;
			f = 0.0000000000001f ;   if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 13 ) ;
			f = 0.00000000000001f ;  if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 14 ) ;
			f = 0.000000000000001f ; if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 15 ) ;

			// マイナスかどうかで処理を分岐
			// 元の値より小さい値を設定する
			if( ret & 0x40 )
			{
				// マイナスの場合
				if( Min < 0.00000000000001f && Min >= 0.000000000000001f ) return ( BYTE )( ret | 14 ) ;
				if( Min < 0.0000000000001f  && Min >= 0.00000000000001f  ) return ( BYTE )( ret | 13 ) ;
				if( Min < 0.000000000001f   && Min >= 0.0000000000001f   ) return ( BYTE )( ret | 12 ) ;
				if( Min < 0.00000000001f    && Min >= 0.000000000001f    ) return ( BYTE )( ret | 11 ) ;
				if( Min < 0.0000000001f     && Min >= 0.00000000001f     ) return ( BYTE )( ret | 10 ) ;
				if( Min < 0.000000001f      && Min >= 0.0000000001f      ) return ( BYTE )( ret | 9 ) ;
				if( Min < 0.00000001f       && Min >= 0.000000001f       ) return ( BYTE )( ret | 8 ) ;
				if( Min < 0.0000001f        && Min >= 0.00000001f        ) return ( BYTE )( ret | 7 ) ;
				if( Min < 0.000001f         && Min >= 0.0000001f         ) return ( BYTE )( ret | 6 ) ;
				if( Min < 0.00001f          && Min >= 0.000001f          ) return ( BYTE )( ret | 5 ) ;
				if( Min < 0.0001f           && Min >= 0.00001f           ) return ( BYTE )( ret | 4 ) ;
				if( Min < 0.001f            && Min >= 0.0001f            ) return ( BYTE )( ret | 3 ) ;
				if( Min < 0.01f             && Min >= 0.001f             ) return ( BYTE )( ret | 2 ) ;
				if( Min < 0.1f              && Min >= 0.01f              ) return ( BYTE )( ret | 1 ) ;
				if( Min < 1.0f              && Min >= 0.1f               ) return ( BYTE )( ret | 0 ) ;
				return ( BYTE )( ret | 0 ) ;
			}
			else
			{
				// プラスの場合
				if( Min < 0.00000000000001f && Min >= 0.000000000000001f ) return ( BYTE )( ret | 15 ) ;
				if( Min < 0.0000000000001f  && Min >= 0.00000000000001f  ) return ( BYTE )( ret | 14 ) ;
				if( Min < 0.000000000001f   && Min >= 0.0000000000001f   ) return ( BYTE )( ret | 13 ) ;
				if( Min < 0.00000000001f    && Min >= 0.000000000001f    ) return ( BYTE )( ret | 12 ) ;
				if( Min < 0.0000000001f     && Min >= 0.00000000001f     ) return ( BYTE )( ret | 11 ) ;
				if( Min < 0.000000001f      && Min >= 0.0000000001f      ) return ( BYTE )( ret | 10 ) ;
				if( Min < 0.00000001f       && Min >= 0.000000001f       ) return ( BYTE )( ret | 9 ) ;
				if( Min < 0.0000001f        && Min >= 0.00000001f        ) return ( BYTE )( ret | 8 ) ;
				if( Min < 0.000001f         && Min >= 0.0000001f         ) return ( BYTE )( ret | 7 ) ;
				if( Min < 0.00001f          && Min >= 0.000001f          ) return ( BYTE )( ret | 6 ) ;
				if( Min < 0.0001f           && Min >= 0.00001f           ) return ( BYTE )( ret | 5 ) ;
				if( Min < 0.001f            && Min >= 0.0001f            ) return ( BYTE )( ret | 4 ) ;
				if( Min < 0.01f             && Min >= 0.001f             ) return ( BYTE )( ret | 3 ) ;
				if( Min < 0.1f              && Min >= 0.01f              ) return ( BYTE )( ret | 2 ) ;
				if( Min < 1.0f              && Min >= 0.1f               ) return ( BYTE )( ret | 1 ) ;
				return 0x80 ;
			}
		}
		else
		{
			// ピッタリな値があるか調べる
			f = 1.0f ;                if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 0 ) ;
			f = 10.0f ;               if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 1 ) ;
			f = 100.0f ;              if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 2 ) ;
			f = 1000.0f ;             if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 3 ) ;
			f = 10000.0f ;            if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 4 ) ;
			f = 100000.0f ;           if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 5 ) ;
			f = 1000000.0f ;          if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 6 ) ;
			f = 10000000.0f ;         if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 7 ) ;
			f = 100000000.0f ;        if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 8 ) ;
			f = 1000000000.0f ;       if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 9 ) ;
			f = 10000000000.0f ;      if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 10 ) ;
			f = 100000000000.0f ;     if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 11 ) ;
			f = 1000000000000.0f ;    if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 12 ) ;
			f = 10000000000000.0f ;   if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 13 ) ;
			f = 100000000000000.0f ;  if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 14 ) ;
			f = 1000000000000000.0f ; if( *( ( DWORD * )&f ) == *( ( DWORD * )&Min ) ) return ( BYTE )( ret | 15 ) ;

			// マイナスかどうかで処理を分岐
			// 元の値より小さい値を設定する
			if( ret & 0x40 )
			{
				// マイナスの場合
				if( Min < 1.0f                && Min >= 0.0f               ) return ( BYTE )( ret | 0 ) ;
				if( Min < 10.0f               && Min >= 1.0f               ) return ( BYTE )( ret | 1 ) ;
				if( Min < 100.0f              && Min >= 10.0f              ) return ( BYTE )( ret | 2 ) ;
				if( Min < 1000.0f             && Min >= 100.0f             ) return ( BYTE )( ret | 3 ) ;
				if( Min < 10000.0f            && Min >= 1000.0f            ) return ( BYTE )( ret | 4 ) ;
				if( Min < 100000.0f           && Min >= 10000.0f           ) return ( BYTE )( ret | 5 ) ;
				if( Min < 1000000.0f          && Min >= 100000.0f          ) return ( BYTE )( ret | 6 ) ;
				if( Min < 10000000.0f         && Min >= 1000000.0f         ) return ( BYTE )( ret | 7 ) ;
				if( Min < 100000000.0f        && Min >= 10000000.0f        ) return ( BYTE )( ret | 8 ) ;
				if( Min < 1000000000.0f       && Min >= 100000000.0f       ) return ( BYTE )( ret | 9 ) ;
				if( Min < 10000000000.0f      && Min >= 1000000000.0f      ) return ( BYTE )( ret | 10 ) ;
				if( Min < 100000000000.0f     && Min >= 10000000000.0f     ) return ( BYTE )( ret | 11 ) ;
				if( Min < 1000000000000.0f    && Min >= 100000000000.0f    ) return ( BYTE )( ret | 12 ) ;
				if( Min < 10000000000000.0f   && Min >= 1000000000000.0f   ) return ( BYTE )( ret | 13 ) ;
				if( Min < 100000000000000.0f  && Min >= 10000000000000.0f  ) return ( BYTE )( ret | 14 ) ;
				if( Min < 1000000000000000.0f && Min >= 100000000000000.0f ) return ( BYTE )( ret | 15 ) ;
				return ( BYTE )( ret | 15 ) ;
			}
			else
			{
				// プラスの場合
				if( Min < 1.0f                 && Min >= 0.0f                ) return 0x80 ;
				if( Min < 10.0f                && Min >= 1.0f                ) return ( BYTE )( ret | 0 ) ;
				if( Min < 100.0f               && Min >= 10.0f               ) return ( BYTE )( ret | 1 ) ;
				if( Min < 1000.0f              && Min >= 100.0f              ) return ( BYTE )( ret | 2 ) ;
				if( Min < 10000.0f             && Min >= 1000.0f             ) return ( BYTE )( ret | 3 ) ;
				if( Min < 100000.0f            && Min >= 10000.0f            ) return ( BYTE )( ret | 4 ) ;
				if( Min < 1000000.0f           && Min >= 100000.0f           ) return ( BYTE )( ret | 5 ) ;
				if( Min < 10000000.0f          && Min >= 1000000.0f          ) return ( BYTE )( ret | 6 ) ;
				if( Min < 100000000.0f         && Min >= 10000000.0f         ) return ( BYTE )( ret | 7 ) ;
				if( Min < 1000000000.0f        && Min >= 100000000.0f        ) return ( BYTE )( ret | 8 ) ;
				if( Min < 10000000000.0f       && Min >= 1000000000.0f       ) return ( BYTE )( ret | 9 ) ;
				if( Min < 100000000000.0f      && Min >= 10000000000.0f      ) return ( BYTE )( ret | 10 ) ;
				if( Min < 1000000000000.0f     && Min >= 100000000000.0f     ) return ( BYTE )( ret | 11 ) ;
				if( Min < 10000000000000.0f    && Min >= 1000000000000.0f    ) return ( BYTE )( ret | 12 ) ;
				if( Min < 100000000000000.0f   && Min >= 10000000000000.0f   ) return ( BYTE )( ret | 13 ) ;
				if( Min < 1000000000000000.0f  && Min >= 100000000000000.0f  ) return ( BYTE )( ret | 14 ) ;
				if( Min < 10000000000000000.0f && Min >= 1000000000000000.0f ) return ( BYTE )( ret | 15 ) ;
				return ( BYTE )( ret | 15 ) ;
			}
		}
	}
}

// float型の値から MV1_ANIM_KEY_16BIT構造体の Unit 変数用の値を作成する
extern BYTE MV1AnimKey16BitUnitFtoB( float Unit )
{
	// マイナスだったらエラー
	if( Unit < 0.0f )
	{
		return 0 ;
	}

	// 最初にゼロかどうか調べる
	if( * ( ( DWORD * )&Unit ) == 0.0f )
	{
		return 0 ;
	}
	else
	{
		int i, ret, mb, mi ;
		float f, mf ;

		// ピッタリの値が無いか調べる
		mf = 1000000000000.0f ;
		mb = -1 ;
		mi = -1 ;
		if( Unit < 1.0f )
		{
			ret = 0x80 ;

			// 小数点以下
			for( i = 1 ; i < 15 ; i ++ ){ f = i / 10.0f       - Unit ; if( f > -0.0000000001f   && f < 0.0000000001f   ) return ( BYTE )( ret | ( 1 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 1 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i / 100.0f      - Unit ; if( f > -0.0000000001f   && f < 0.0000000001f   ) return ( BYTE )( ret | ( 2 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 2 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i / 1000.0f     - Unit ; if( f > -0.00000000001f  && f < 0.00000000001f  ) return ( BYTE )( ret | ( 3 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 3 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i / 10000.0f    - Unit ; if( f > -0.00000000001f  && f < 0.00000000001f  ) return ( BYTE )( ret | ( 4 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 4 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i / 100000.0f   - Unit ; if( f > -0.00000000001f  && f < 0.00000000001f  ) return ( BYTE )( ret | ( 5 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 5 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i / 1000000.0f  - Unit ; if( f > -0.000000000001f && f < 0.000000000001f ) return ( BYTE )( ret | ( 6 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 6 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i / 10000000.0f - Unit ; if( f > -0.000000000001f && f < 0.000000000001f ) return ( BYTE )( ret | ( 7 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 7 ; mf = f ; } }
			return ( BYTE )( ret | ( mb << 4 ) | mi ) ;
		}
		else
		{
			ret = 0 ;

			// １．０以上
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 1.0f        - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 0 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 0 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 10.0f       - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 1 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 1 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 100.0f      - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 2 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 2 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 1000.0f     - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 3 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 3 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 10000.0f    - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 4 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 4 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 100000.0f   - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 5 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 5 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 1000000.0f  - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 6 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 6 ; mf = f ; } }
			for( i = 1 ; i < 15 ; i ++ ){ f = i * 10000000.0f - Unit ; if( f > -0.000000001f && f < 0.000000001f ) return ( BYTE )( ret | ( 7 << 4 ) | i ) ; if( f > 0.0f && mf > f ){ mi = i ; mb = 7 ; mf = f ; } }
			return ( BYTE )( ret | ( mb << 4 ) | mi ) ;
		}
	}
}

// MV1_ANIM_KEY_16BIT構造体の Min 変数の値から float型の値を作成する
extern float MV1AnimKey16BitMinBtoF( BYTE Min )
{
	static float Table[ 2 ][ 16 ] =
	{
		1.0f,
		10.0f,
		100.0f,
		1000.0f,
		10000.0f,
		100000.0f,
		1000000.0f,
		10000000.0f,
		100000000.0f,
		1000000000.0f,
		10000000000.0f,
		100000000000.0f,
		1000000000000.0f,
		10000000000000.0f,
		100000000000000.0f,
		1000000000000000.0f,

		1.0f,
		0.1f,
		0.01f,
		0.001f,
		0.0001f,
		0.00001f,
		0.000001f,
		0.0000001f,
		0.00000001f,
		0.000000001f,
		0.0000000001f,
		0.00000000001f,
		0.000000000001f,
		0.0000000000001f,
		0.00000000000001f,
		0.000000000000001f,
	} ;

	if( Min & 0x80 )
	{
		return 0 ;
	}
	else
	{
		return Table[ ( Min >> 5 ) & 1 ][ Min & 0x1f ] * ( ( Min & 0x40 ) ? -1.0f : 1.0f ) ;
	}
}

// MV1_ANIM_KEY_16BIT構造体の Unit 変数の値から float型の値を作成する
extern float MV1AnimKey16BitUnitBtoF( BYTE Unit )
{
	static float Table[ 2 ][ 8 ] =
	{
		1.0f,
		10.0f,
		100.0f,
		1000.0f,
		10000.0f,
		100000.0f,
		1000000.0f,
		10000000.0f,

		1.0f,
		0.1f,
		0.01f,
		0.001f,
		0.0001f,
		0.00001f,
		0.000001f,
		0.0000001f,
	} ;
	return Table[ Unit >> 7 ][ ( Unit >> 4 ) & 0x7 ] * ( Unit & 0x0f ) ;
}













// MATRIX 構造体を MATRIX_4X4CT_F 構造体に変換する
extern void ConvertMatrixFToMatrix4x4cF( MATRIX_4X4CT_F *Out, const MATRIX *In )
{
	Out->m[ 0 ][ 0 ] = In->m[ 0 ][ 0 ] ;
	Out->m[ 1 ][ 0 ] = In->m[ 0 ][ 1 ] ;
	Out->m[ 2 ][ 0 ] = In->m[ 0 ][ 2 ] ;

	Out->m[ 0 ][ 1 ] = In->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In->m[ 1 ][ 1 ] ;
	Out->m[ 2 ][ 1 ] = In->m[ 1 ][ 2 ] ;

	Out->m[ 0 ][ 2 ] = In->m[ 2 ][ 0 ] ;
	Out->m[ 1 ][ 2 ] = In->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In->m[ 2 ][ 2 ] ;

	Out->m[ 0 ][ 3 ] = In->m[ 3 ][ 0 ] ;
	Out->m[ 1 ][ 3 ] = In->m[ 3 ][ 1 ] ;
	Out->m[ 2 ][ 3 ] = In->m[ 3 ][ 2 ] ;
}

// MATRIX_D 構造体を MATRIX_4X4CT_D 構造体に変換する
extern void ConvertMatrixDToMatrix4x4cD( MATRIX_4X4CT_D *Out, const MATRIX_D *In )
{
	Out->m[ 0 ][ 0 ] = In->m[ 0 ][ 0 ] ;
	Out->m[ 1 ][ 0 ] = In->m[ 0 ][ 1 ] ;
	Out->m[ 2 ][ 0 ] = In->m[ 0 ][ 2 ] ;

	Out->m[ 0 ][ 1 ] = In->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In->m[ 1 ][ 1 ] ;
	Out->m[ 2 ][ 1 ] = In->m[ 1 ][ 2 ] ;

	Out->m[ 0 ][ 2 ] = In->m[ 2 ][ 0 ] ;
	Out->m[ 1 ][ 2 ] = In->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In->m[ 2 ][ 2 ] ;

	Out->m[ 0 ][ 3 ] = In->m[ 3 ][ 0 ] ;
	Out->m[ 1 ][ 3 ] = In->m[ 3 ][ 1 ] ;
	Out->m[ 2 ][ 3 ] = In->m[ 3 ][ 2 ] ;
}

// MATRIX_D 構造体を MATRIX_4X4CT_F 構造体に変換する
extern void ConvertMatrixDToMatrix4x4cF( MATRIX_4X4CT_F *Out, const MATRIX_D *In )
{
	Out->m[ 0 ][ 0 ] = ( float )In->m[ 0 ][ 0 ] ;
	Out->m[ 1 ][ 0 ] = ( float )In->m[ 0 ][ 1 ] ;
	Out->m[ 2 ][ 0 ] = ( float )In->m[ 0 ][ 2 ] ;

	Out->m[ 0 ][ 1 ] = ( float )In->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = ( float )In->m[ 1 ][ 1 ] ;
	Out->m[ 2 ][ 1 ] = ( float )In->m[ 1 ][ 2 ] ;

	Out->m[ 0 ][ 2 ] = ( float )In->m[ 2 ][ 0 ] ;
	Out->m[ 1 ][ 2 ] = ( float )In->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = ( float )In->m[ 2 ][ 2 ] ;

	Out->m[ 0 ][ 3 ] = ( float )In->m[ 3 ][ 0 ] ;
	Out->m[ 1 ][ 3 ] = ( float )In->m[ 3 ][ 1 ] ;
	Out->m[ 2 ][ 3 ] = ( float )In->m[ 3 ][ 2 ] ;
}

// MATRIX 構造体を MATRIX_4X4CT_D 構造体に変換する
extern void ConvertMatrixFToMatrix4x4cD( MATRIX_4X4CT_D *Out, const MATRIX *In )
{
	Out->m[ 0 ][ 0 ] = In->m[ 0 ][ 0 ] ;
	Out->m[ 1 ][ 0 ] = In->m[ 0 ][ 1 ] ;
	Out->m[ 2 ][ 0 ] = In->m[ 0 ][ 2 ] ;

	Out->m[ 0 ][ 1 ] = In->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In->m[ 1 ][ 1 ] ;
	Out->m[ 2 ][ 1 ] = In->m[ 1 ][ 2 ] ;

	Out->m[ 0 ][ 2 ] = In->m[ 2 ][ 0 ] ;
	Out->m[ 1 ][ 2 ] = In->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In->m[ 2 ][ 2 ] ;

	Out->m[ 0 ][ 3 ] = In->m[ 3 ][ 0 ] ;
	Out->m[ 1 ][ 3 ] = In->m[ 3 ][ 1 ] ;
	Out->m[ 2 ][ 3 ] = In->m[ 3 ][ 2 ] ;
}

// MATRIX_4X4CT_F 構造体を MATRIX 構造体に変換する
extern void ConvertMatrix4x4cFToMatrixF( MATRIX *Out, const MATRIX_4X4CT_F *In )
{
	Out->m[ 0 ][ 0 ] = In->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In->m[ 1 ][ 0 ] ;
	Out->m[ 0 ][ 2 ] = In->m[ 2 ][ 0 ] ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = In->m[ 0 ][ 1 ] ;
	Out->m[ 1 ][ 1 ] = In->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In->m[ 2 ][ 1 ] ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = In->m[ 0 ][ 2 ] ;
	Out->m[ 2 ][ 1 ] = In->m[ 1 ][ 2 ] ;
	Out->m[ 2 ][ 2 ] = In->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
	Out->m[ 3 ][ 0 ] = In->m[ 0 ][ 3 ] ;
	Out->m[ 3 ][ 1 ] = In->m[ 1 ][ 3 ] ;
	Out->m[ 3 ][ 2 ] = In->m[ 2 ][ 3 ] ;
	Out->m[ 3 ][ 3 ] = 1.0f ;
}

// MATRIX_4X4CT_D 構造体を MATRIX_D 構造体に変換する
extern void ConvertMatrix4x4cDToMatrixD( MATRIX_D *Out, const MATRIX_4X4CT_D *In )
{
	Out->m[ 0 ][ 0 ] = In->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In->m[ 1 ][ 0 ] ;
	Out->m[ 0 ][ 2 ] = In->m[ 2 ][ 0 ] ;
	Out->m[ 0 ][ 3 ] = 0.0 ;
	Out->m[ 1 ][ 0 ] = In->m[ 0 ][ 1 ] ;
	Out->m[ 1 ][ 1 ] = In->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In->m[ 2 ][ 1 ] ;
	Out->m[ 1 ][ 3 ] = 0.0 ;
	Out->m[ 2 ][ 0 ] = In->m[ 0 ][ 2 ] ;
	Out->m[ 2 ][ 1 ] = In->m[ 1 ][ 2 ] ;
	Out->m[ 2 ][ 2 ] = In->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = 0.0 ;
	Out->m[ 3 ][ 0 ] = In->m[ 0 ][ 3 ] ;
	Out->m[ 3 ][ 1 ] = In->m[ 1 ][ 3 ] ;
	Out->m[ 3 ][ 2 ] = In->m[ 2 ][ 3 ] ;
	Out->m[ 3 ][ 3 ] = 1.0 ;
}

// MATRIX_4X4CT_D 構造体を MATRIX   構造体に変換する
extern void ConvertMatrix4x4cDToMatrixF( MATRIX   *Out, const MATRIX_4X4CT_D *In )
{
	Out->m[ 0 ][ 0 ] = ( float )In->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = ( float )In->m[ 1 ][ 0 ] ;
	Out->m[ 0 ][ 2 ] = ( float )In->m[ 2 ][ 0 ] ;
	Out->m[ 0 ][ 3 ] = 0.0f ;
	Out->m[ 1 ][ 0 ] = ( float )In->m[ 0 ][ 1 ] ;
	Out->m[ 1 ][ 1 ] = ( float )In->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = ( float )In->m[ 2 ][ 1 ] ;
	Out->m[ 1 ][ 3 ] = 0.0f ;
	Out->m[ 2 ][ 0 ] = ( float )In->m[ 0 ][ 2 ] ;
	Out->m[ 2 ][ 1 ] = ( float )In->m[ 1 ][ 2 ] ;
	Out->m[ 2 ][ 2 ] = ( float )In->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = 0.0f ;
	Out->m[ 3 ][ 0 ] = ( float )In->m[ 0 ][ 3 ] ;
	Out->m[ 3 ][ 1 ] = ( float )In->m[ 1 ][ 3 ] ;
	Out->m[ 3 ][ 2 ] = ( float )In->m[ 2 ][ 3 ] ;
	Out->m[ 3 ][ 3 ] = 1.0f ;
}

// MATRIX_4X4CT_F 構造体を MATRIX_D 構造体に変換する
extern void ConvertMatrix4x4cFToMatrixD( MATRIX_D *Out, const MATRIX_4X4CT_F *In )
{
	Out->m[ 0 ][ 0 ] = In->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In->m[ 1 ][ 0 ] ;
	Out->m[ 0 ][ 2 ] = In->m[ 2 ][ 0 ] ;
	Out->m[ 0 ][ 3 ] = 0.0 ;
	Out->m[ 1 ][ 0 ] = In->m[ 0 ][ 1 ] ;
	Out->m[ 1 ][ 1 ] = In->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In->m[ 2 ][ 1 ] ;
	Out->m[ 1 ][ 3 ] = 0.0 ;
	Out->m[ 2 ][ 0 ] = In->m[ 0 ][ 2 ] ;
	Out->m[ 2 ][ 1 ] = In->m[ 1 ][ 2 ] ;
	Out->m[ 2 ][ 2 ] = In->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = 0.0 ;
	Out->m[ 3 ][ 0 ] = In->m[ 0 ][ 3 ] ;
	Out->m[ 3 ][ 1 ] = In->m[ 1 ][ 3 ] ;
	Out->m[ 3 ][ 2 ] = In->m[ 2 ][ 3 ] ;
	Out->m[ 3 ][ 3 ] = 1.0 ;
}

// MATRIX_4X4CT_F   構造体を MATRIX_4X4CT_D 構造体に変換する
extern void ConvertMatrix4x4cFToMatrix4x4cD(  MATRIX_4X4CT_D *Out, const MATRIX_4X4CT_F *In )
{
	Out->m[ 0 ][ 0 ] = In->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = In->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] = In->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] = In->m[ 0 ][ 3 ] ;

	Out->m[ 1 ][ 0 ] = In->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = In->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = In->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] = In->m[ 1 ][ 3 ] ;

	Out->m[ 2 ][ 0 ] = In->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] = In->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = In->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = In->m[ 2 ][ 3 ] ;
}

// MATRIX_4X4CT_F   構造体を MATRIX_4X4CT_D 構造体に変換する
extern void ConvertMatrix4x4cDToMatrix4x4cF(  MATRIX_4X4CT_F   *Out, const MATRIX_4X4CT_D *In )
{
	Out->m[ 0 ][ 0 ] = ( float )In->m[ 0 ][ 0 ] ;
	Out->m[ 0 ][ 1 ] = ( float )In->m[ 0 ][ 1 ] ;
	Out->m[ 0 ][ 2 ] = ( float )In->m[ 0 ][ 2 ] ;
	Out->m[ 0 ][ 3 ] = ( float )In->m[ 0 ][ 3 ] ;

	Out->m[ 1 ][ 0 ] = ( float )In->m[ 1 ][ 0 ] ;
	Out->m[ 1 ][ 1 ] = ( float )In->m[ 1 ][ 1 ] ;
	Out->m[ 1 ][ 2 ] = ( float )In->m[ 1 ][ 2 ] ;
	Out->m[ 1 ][ 3 ] = ( float )In->m[ 1 ][ 3 ] ;

	Out->m[ 2 ][ 0 ] = ( float )In->m[ 2 ][ 0 ] ;
	Out->m[ 2 ][ 1 ] = ( float )In->m[ 2 ][ 1 ] ;
	Out->m[ 2 ][ 2 ] = ( float )In->m[ 2 ][ 2 ] ;
	Out->m[ 2 ][ 3 ] = ( float )In->m[ 2 ][ 3 ] ;
}

// MATRIX 構造体を MATRIX_4X4CT 構造体に変換する
extern void ConvertMatrixFToMatrix4x4c( MATRIX_4X4CT *Out, const MATRIX   *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		ConvertMatrixFToMatrix4x4cD( &Out->md, In ) ;
	}
	else
	{
		ConvertMatrixFToMatrix4x4cF( &Out->mf, In ) ;
	}
}

// MATRIX_D 構造体を MATRIX_4X4CT 構造体に変換する
extern void ConvertMatrixDToMatrix4x4c( MATRIX_4X4CT *Out, const MATRIX_D *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		ConvertMatrixDToMatrix4x4cD( &Out->md, In ) ;
	}
	else
	{
		ConvertMatrixDToMatrix4x4cF( &Out->mf, In ) ;
	}
}

// MATRIX_4X4CT 構造体を MATRIX 構造体に変換する
extern void ConvertMatrix4x4cToMatrixF( MATRIX   *Out, const MATRIX_4X4CT *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		ConvertMatrix4x4cDToMatrixF( Out, &In->md ) ;
	}
	else
	{
		ConvertMatrix4x4cFToMatrixF( Out, &In->mf ) ;
	}
}

// MATRIX_4X4CT   構造体を MATRIX_D 構造体に変換する
extern void ConvertMatrix4x4cToMatrixD( MATRIX_D *Out, const MATRIX_4X4CT *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		ConvertMatrix4x4cDToMatrixD( Out, &In->md ) ;
	}
	else
	{
		ConvertMatrix4x4cFToMatrixD( Out, &In->mf ) ;
	}
}

// MATRIX_4X4CT   構造体を MATRIX_4X4CT_F 構造体に変換する
extern void ConvertMatrix4x4cToMatrix4x4cF(  MATRIX_4X4CT_F *Out, const MATRIX_4X4CT *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		ConvertMatrix4x4cDToMatrix4x4cF( Out, &In->md ) ;
	}
	else
	{
		*Out = In->mf ;
	}
}

// MATRIX_4X4CT   構造体を MATRIX_4X4CT_D 構造体に変換する
extern void ConvertMatrix4x4cToMatrix4x4cD(  MATRIX_4X4CT_D *Out, const MATRIX_4X4CT *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		*Out = In->md ;
	}
	else
	{
		ConvertMatrix4x4cFToMatrix4x4cD( Out, &In->mf ) ;
	}
}

// MATRIX_4X4CT_F 構造体を MATRIX_4X4CT 構造体に変換する
extern void ConvertMatrix4x4cFToMatrix4x4c( MATRIX_4X4CT *Out, const MATRIX_4X4CT_F *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		ConvertMatrix4x4cFToMatrix4x4cD( &Out->md, In ) ;
	}
	else
	{
		Out->mf = *In ;
	}
}

// MATRIX_4X4CT_D 構造体を MATRIX_4X4CT 構造体に変換する
extern void ConvertMatrix4x4cDToMatrix4x4c( MATRIX_4X4CT *Out, const MATRIX_4X4CT_D *In )
{
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Out->md = *In ;
	}
	else
	{
		ConvertMatrix4x4cDToMatrix4x4cF( &Out->mf, In ) ;
	}
}






// 指定の時間を超える一番小さい番号のキーのインデックスを取得する
__inline static int _MV1GetAnimKeyDataIndexFromTime_inline( MV1_ANIM_KEYSET_BASE *AnimKeySetBase, float Time, float TotalRate, int NowKey, bool AboutSetting )
{
	int KeyNum ;
	float *TimeList ;

	KeyNum = AnimKeySetBase->Num ;

	// キーを一つ一つ持っているかどうかで処理を分岐
	if( AnimKeySetBase->TimeType == MV1_ANIMKEY_TIME_TYPE_KEY )
	{
		// アバウトセッティング指定の場合は最初に大体のアクティブキーをセットする
		if( AboutSetting )
		{
			// 大体のキー番号を調べる
			NowKey = _FTOL( AnimKeySetBase->Num * TotalRate ) ;

			// 飽和処理
				 if( NowKey <                    0 ) NowKey = 0 ;
			else if( NowKey >= AnimKeySetBase->Num ) NowKey = AnimKeySetBase->Num - 1 ;
		}

		// 適合したキーに移動する
		TimeList = AnimKeySetBase->KeyTime ;
		if( TimeList[ NowKey ] > Time )
		{
			while( NowKey !=          0 && TimeList[ NowKey     ] >  Time )
				NowKey -- ;
		}
		else
		{
			while( NowKey <  KeyNum - 1 && TimeList[ NowKey + 1 ] <= Time )
				NowKey ++ ;
		}
	}
	else
	{
		// 各キーの間隔が一定の場合は単純計算
		NowKey = _FTOL( ( Time - AnimKeySetBase->StartTime ) / AnimKeySetBase->UnitTime ) ;
		     if( NowKey <  0      ) NowKey = 0 ;
		else if( NowKey >= KeyNum ) NowKey = KeyNum - 1 ;
	}

	return NowKey ;
}

static int _MV1GetAnimKeyDataIndexFromTime( MV1_ANIM_KEYSET_BASE *AnimKeySetBase, float Time, float &Rate )
{
	int KeyIndex ;

	// キーを一つ一つ持っているかどうかで処理を分岐
	if( AnimKeySetBase->TimeType == MV1_ANIMKEY_TIME_TYPE_KEY )
	{
		KeyIndex = _MV1GetAnimKeyDataIndexFromTime_inline( AnimKeySetBase, Time, ( Time - AnimKeySetBase->StartTime ) / ( AnimKeySetBase->KeyTime[ AnimKeySetBase->Num - 1 ] - AnimKeySetBase->StartTime ), 0, true ) ;
	}
	else
	{
		KeyIndex = _MV1GetAnimKeyDataIndexFromTime_inline( AnimKeySetBase, Time, 0.0f, 0, true ) ;
	}

	if( KeyIndex == AnimKeySetBase->Num - 1 || Time < AnimKeySetBase->StartTime )
	{
		Rate = 0.0f ;
		return KeyIndex ;
	}
	else
	{
		// キーを一つ一つ持っているかどうかで処理を分岐
		if( AnimKeySetBase->TimeType == MV1_ANIMKEY_TIME_TYPE_KEY )
		{
			Rate = ( Time - AnimKeySetBase->KeyTime[ KeyIndex ] ) / (  AnimKeySetBase->KeyTime[ KeyIndex + 1 ] - AnimKeySetBase->KeyTime[ KeyIndex ] ) ;
		}
		else
		{
			Rate = ( Time - KeyIndex * AnimKeySetBase->UnitTime - AnimKeySetBase->StartTime ) / AnimKeySetBase->UnitTime ;
		}
	}

	return KeyIndex ;
}

// 現在のアニメーション再生時間に各キーの NowKey の値を合わせる
static int _MV1AnimSetSyncNowKey( MV1_ANIMSET *AnimSet, bool AboutSetting )
{
	MV1_ANIM				*Anim ;
	MV1_ANIM_KEYSET			*KeySet ;
	int i, j ;
	float NowTime, NowRate ;

	// 現在の再生時間をセット
	NowTime = AnimSet->NowTime ;

	// 全体の時間に対する指定時間の比率を求める
	NowRate = NowTime / AnimSet->BaseData->MaxTime ;

	// 設定されているアクティブキーから現在の適切なアクティブキーを検索する
	Anim = AnimSet->Anim ;
	for( i = 0 ; i < AnimSet->BaseData->AnimNum ; i ++, Anim ++ )
	{
		KeySet = Anim->KeySet ;
		for( j = 0 ; j < Anim->BaseData->KeySetNum ; j ++, KeySet ++ )
		{
			KeySet->NowKey = _MV1GetAnimKeyDataIndexFromTime_inline( KeySet->BaseData, NowTime, NowRate, KeySet->NowKey, AboutSetting ) ;
		}
	}

	// 終了
	return 0 ;
}

// 名前やインデックスからモデル基本データ内のアニメーションを取得する
static MV1_ANIMSET_BASE *MV1GetAnimSetBase( int MV1ModelHandle, const wchar_t *Name, int Index )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	int i ;

	// アドレス取得
	if( MV1BMDLCHK( MV1ModelHandle, ModelBase ) )
		return NULL ;

	// 指定のアニメーションを調べる
	if( Index >= 0 )
	{
		// 指定のインデックスがアニメーションセットの数から外れていたらエラー
		if( Index >= ModelBase->AnimSetNum ) return NULL ;
		return &ModelBase->AnimSet[ Index ] ;
	}
	else
	{
		// 名前が同じなアニメーションセットを探す
		AnimSetBase = ModelBase->AnimSet ;
		for( i = 0 ; i < ModelBase->AnimSetNum && _WCSCMP( AnimSetBase->NameW, Name ) != 0 ; i ++, AnimSetBase ++ ){}
		return i == ModelBase->AnimSetNum ? NULL : &ModelBase->AnimSet[ i ] ;
	}
}

// アニメーションセット基本データから実行用アニメーションセットを作成する
static MV1_ANIMSET *MV1CreateAnimSet( MV1_ANIMSET_BASE *MV1AnimSetBase )
{
	MV1_ANIM_BASE *AnimBase ;
	MV1_ANIMSET *AnimSet ;
	MV1_ANIM *Anim ;
	int AllocSize ;
	int i, j ;

	// 確保すべきメモリのサイズを算出する
	AllocSize = sizeof( MV1_ANIMSET ) ;
	AllocSize += MV1AnimSetBase->AnimNum * sizeof( MV1_ANIM ) ;
	AnimBase = MV1AnimSetBase->Anim ;
	for( i = 0 ; i < MV1AnimSetBase->AnimNum ; i ++, AnimBase ++ )
	{
		AllocSize += sizeof( MV1_ANIM_KEYSET ) * AnimBase->KeySetNum ;
		AllocSize += sizeof( float ) * AnimBase->TargetFrame->ShapeNum ;
	}

	// メモリの確保
	AnimSet = ( MV1_ANIMSET * )MDALLOCMEM( ( size_t )AllocSize ) ;
	if( AnimSet == NULL ) return NULL ;
	_MEMSET( AnimSet, 0, ( size_t )AllocSize ) ;

	// 情報のセット
	AllocSize = 0 ;
	AnimSet->BaseData = MV1AnimSetBase ;
	AllocSize += sizeof( MV1_ANIMSET ) ;

	AnimSet->Anim = ( MV1_ANIM * )( ( BYTE * )AnimSet + AllocSize ) ;
	AllocSize += sizeof( MV1_ANIM ) * MV1AnimSetBase->AnimNum ;

	AnimBase = MV1AnimSetBase->Anim ;
	Anim = AnimSet->Anim ;
	for( i = 0 ; i < MV1AnimSetBase->AnimNum ; i ++, AnimBase ++, Anim ++ )
	{
		Anim->BaseData = AnimBase ;
		Anim->KeySet = ( MV1_ANIM_KEYSET * )( ( BYTE * )AnimSet + AllocSize ) ;
		AllocSize += sizeof( MV1_ANIM_KEYSET ) * AnimBase->KeySetNum ;
		for( j = 0 ; j < AnimBase->KeySetNum ; j ++ )
		{
			Anim->KeySet[ j ].BaseData = &AnimBase->KeySet[ j ] ;
			Anim->KeySet[ j ].ShapeTargetIndex = -1 ;
		}
		Anim->ShapeRate = ( float * )( ( BYTE * )AnimSet + AllocSize ) ;
		AllocSize += sizeof( float ) * Anim->BaseData->TargetFrame->ShapeNum ;
		for( j = 0 ; j < Anim->BaseData->TargetFrame->ShapeNum ; j ++ )
		{
			Anim->ShapeRate[ j ] = 0.0f ;
		}
	}

	// 終了
	return AnimSet ;
}

// アニメーションセットから特定の名前、若しくはインデックスのアニメーションを得る
static MV1_ANIM *MV1GetAnimSetAnim( MV1_ANIMSET *AnimSet, const wchar_t *Name, int Index )
{
	MV1_ANIM_BASE *AnimBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	int i ;

	// 名前が無効な場合はインデックスからアニメーションを返す
	if( Name == NULL ) return AnimSet->Anim + Index ;

	// 名前が有効な場合は名前から検索
	AnimSetBase = AnimSet->BaseData ;
	AnimBase = AnimSetBase->Anim ;
	for( i = 0 ; i < AnimSetBase->AnimNum ; i ++, AnimBase ++ )
	{
		if( _WCSCMP( AnimBase->TargetFrame->NameW, Name ) == 0 ) break ;
	}
	return i == AnimSetBase->AnimNum ? NULL : AnimSet->Anim + i ;
}

// アニメーションを指定の時間まで進める
static int MV1SetAnimSetTime( MV1_ANIMSET *AnimSet, float Time )
{
	// 指定の時間が範囲を超えていたらクランプする
	     if( AnimSet->BaseData->MaxTime < Time ) Time = AnimSet->BaseData->MaxTime ;
	else if(                       0.0f > Time ) Time = 0.0f ;

	// タイムをセットする
	AnimSet->NowTime = Time ;

	// 大体の位置をセットする
	_MV1AnimSetSyncNowKey( AnimSet, true ) ;

	// ループフラグを倒す
//	AnimSet->LoopCompFlag = false ;

	// 終了
	return 0 ;
}

// アニメーションの現在の再生経過時間に合わせたパラメータを計算する
static int MV1AnimSetSetupParam( MV1_ANIMSET *AnimSet )
{
	MV1_ANIM                     *Anim ;
	MV1_ANIMSET_BASE             *AnimSetBase ;
	MV1_ANIM_KEYSET_BASE         *KeySetBase ;
	VECTOR                  * RST KeyVector1 ;
	VECTOR                  * RST KeyVector2 ;
	float                   * RST KeyLinear1 ;
	float                   * RST KeyLinear2 ;
	MV1_ANIM_KEY_MATRIX4X4C * RST KeyMatrix4x4C1 ;
	MV1_ANIM_KEY_MATRIX4X4C * RST KeyMatrix4x4C2 ;
	VECTOR                        Translate ;
	VECTOR                        Scale ;
	VECTOR                        Rotate ;
	VECTOR                        vdata ;
	int                           RotateOrder = 0 ;
	int                           i ;
	int                           j ;
	int                           NowKey ;
	int                           NextKey ;
	int                           QtType = 0 ;
	FLOAT4                        Quaternion ;
	FLOAT4                  * RST q1 ;
	FLOAT4                  * RST q2 ;
	MATRIX_4X4CT_F                Matrix ;
	int                           ValidMatrix ;
	float                         t ;
	float                         fdata ;
	bool                          AddParam ;
	float                         Sub ;
	VECTOR                        SubVec ;

	// パラメータのセットアップが済んでいる場合は何もせずに終了
	if( AnimSet->ParamSetup ) return 0 ;

	// アニメーションの数だけ繰り返し
	AnimSetBase = AnimSet->BaseData ;
	AddParam = AnimSetBase->IsAddAnim == 1 ;

	// 行列レベルのブレンド処理を行うかどうかで処理を分岐
	if( AnimSetBase->IsMatrixLinearBlend )
//	if( 1 )
	{
		VECTOR Pos, NowRot, NextRot, Scale ;
		FLOAT4 NowQt, NextQt ;
		float NowRotX, NowRotY, NowRotZ ;
		float NextRotX, NextRotY, NextRotZ ;
		float RotRateX, RotRateY, RotRateZ, RotRate ;
		float QtRate ;
		VECTOR *PreRotate, *PostRotate ;

MATRIXLINEARBLEND :

		Anim = AnimSet->Anim ;
		for( i = 0 ; i < AnimSetBase->AnimNum ; i ++, Anim ++ )
		{
			MATRIX_4X4CT_F RotMat1, RotMat2 ;
			MATRIX_4X4CT_F PreRotMat, PostRotMat ;

			// セットアップ情報をリセット
			Anim->ValidFlag = 0 ;

			// 初期状態では目標ノードのデフォルト値を入れておく
			RotRate = -1.0f ;
			RotRateX = -1.0f ;
			RotRateY = -1.0f ;
			RotRateZ = -1.0f ;
			QtRate = -1.0f ;
			if( Anim->Frame )
			{
				NextRot    = NowRot    = Anim->Frame->BaseData->Rotate ;
				NextRotX   = NowRotX   = NowRot.x ;
				NextRotY   = NowRotY   = NowRot.y ;
				NextRotZ   = NowRotZ   = NowRot.z ;
				NextQt     = NowQt     = Anim->Frame->BaseData->Quaternion ;
				Pos    = Anim->Frame->BaseData->Translate ;
				Scale  = Anim->Frame->BaseData->Scale ;

				if( ( Anim->Frame->BaseData->Flag & MV1_FRAMEFLAG_PREROTATE  ) != 0 )
				{
					PreRotate = &Anim->Frame->BaseData->PreRotate ;
					switch( RotateOrder )
					{
					case MV1_ROTATE_ORDER_XYZ : CreateRotationXYZMatrix4X4CTF( &PreRotMat, PreRotate->x, PreRotate->y, PreRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_XZY : CreateRotationXZYMatrix4X4CTF( &PreRotMat, PreRotate->x, PreRotate->y, PreRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_YXZ : CreateRotationYXZMatrix4X4CTF( &PreRotMat, PreRotate->x, PreRotate->y, PreRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_YZX : CreateRotationYZXMatrix4X4CTF( &PreRotMat, PreRotate->x, PreRotate->y, PreRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_ZXY : CreateRotationZXYMatrix4X4CTF( &PreRotMat, PreRotate->x, PreRotate->y, PreRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_ZYX : CreateRotationZYXMatrix4X4CTF( &PreRotMat, PreRotate->x, PreRotate->y, PreRotate->z ) ; break ;
					default: return -1;
					}
				}
				else
				{
					PreRotate = NULL ;
				}

				if( ( Anim->Frame->BaseData->Flag & MV1_FRAMEFLAG_POSTROTATE  ) != 0 )
				{
					PostRotate = &Anim->Frame->BaseData->PostRotate ;
					switch( RotateOrder )
					{
					case MV1_ROTATE_ORDER_XYZ : CreateRotationXYZMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_XZY : CreateRotationXZYMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_YXZ : CreateRotationYXZMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_YZX : CreateRotationYZXMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_ZXY : CreateRotationZXYMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ; break ;
					case MV1_ROTATE_ORDER_ZYX : CreateRotationZYXMatrix4X4CTF( &PostRotMat, PostRotate->x, PostRotate->y, PostRotate->z ) ; break ;
					default: return -1;
					}
				}
				else
				{
					PostRotate = NULL ;
				}
			}
			else
			{
				PreRotate = NULL ;
				PostRotate = NULL ;

				Pos.x = 0.0f ;
				Pos.y = 0.0f ;
				Pos.z = 0.0f ;

				NextRotX = NowRotX = NextRot.x = NowRot.x = 0.0f ;
				NextRotY = NowRotY = NextRot.y = NowRot.y = 0.0f ;
				NextRotZ = NowRotZ = NextRot.z = NowRot.z = 0.0f ;

				Scale.x = 1.0f ;
				Scale.y = 1.0f ;
				Scale.z = 1.0f ;

				NextQt.w = NowQt.w = 1.0f ;
				NextQt.x = NowQt.x = 0.0f ;
				NextQt.y = NowQt.y = 0.0f ;
				NextQt.z = NowQt.z = 0.0f ;
			}
			RotateOrder = Anim->RotateOrder ;

			// 各要素の現在値割り出す
			for( j = 0 ; j < Anim->BaseData->KeySetNum ; j ++ )
			{
				NowKey     = Anim->KeySet[ j ].NowKey ;
				KeySetBase = Anim->KeySet[ j ].BaseData ;

				if( KeySetBase->Num == 0 )
				{
					continue ;
				}

				NextKey    = NowKey == KeySetBase->Num - 1 ? 0 : NowKey + 1 ;

				if( NowKey == KeySetBase->Num - 1 )
				{
					t = 0.0f ;
				}
				else
				{
					if( KeySetBase->TimeType == MV1_ANIMKEY_TIME_TYPE_ONE )
					{
						t = ( AnimSet->NowTime - ( KeySetBase->UnitTime * NowKey + KeySetBase->StartTime ) ) / KeySetBase->UnitTime ;
					}
					else
					{
						t = ( AnimSet->NowTime - KeySetBase->KeyTime[ NowKey ] ) / ( KeySetBase->KeyTime[ NowKey + 1 ] - KeySetBase->KeyTime[ NowKey ] ) ;
					}
					if( t < 0.0f ) t = 0.0f ;
				}

				switch( KeySetBase->Type )
				{
				case MV1_ANIMKEY_TYPE_LINEAR :
					KeyLinear1 = &KeySetBase->KeyLinear[ NowKey  ] ;
					KeyLinear2 = &KeySetBase->KeyLinear[ NextKey ] ;

					switch( KeySetBase->DataType )
					{
					case MV1_ANIMKEY_DATATYPE_TRANSLATE_X : Pos.x   = *KeyLinear1 + ( *KeyLinear2 - *KeyLinear1 ) * t ; break ;
					case MV1_ANIMKEY_DATATYPE_TRANSLATE_Y : Pos.y   = *KeyLinear1 + ( *KeyLinear2 - *KeyLinear1 ) * t ; break ;
					case MV1_ANIMKEY_DATATYPE_TRANSLATE_Z : Pos.z   = *KeyLinear1 + ( *KeyLinear2 - *KeyLinear1 ) * t ; break ;
					case MV1_ANIMKEY_DATATYPE_SCALE_X :     Scale.x = *KeyLinear1 + ( *KeyLinear2 - *KeyLinear1 ) * t ; break ;
					case MV1_ANIMKEY_DATATYPE_SCALE_Y :     Scale.y = *KeyLinear1 + ( *KeyLinear2 - *KeyLinear1 ) * t ; break ;
					case MV1_ANIMKEY_DATATYPE_SCALE_Z :     Scale.z = *KeyLinear1 + ( *KeyLinear2 - *KeyLinear1 ) * t ; break ;
					case MV1_ANIMKEY_DATATYPE_ROTATE_X :    NowRotX   = *KeyLinear1 ; NextRotX   = *KeyLinear2 ; RotRateX = t ; break ;
					case MV1_ANIMKEY_DATATYPE_ROTATE_Y :    NowRotY   = *KeyLinear1 ; NextRotY   = *KeyLinear2 ; RotRateY = t ; break ;
					case MV1_ANIMKEY_DATATYPE_ROTATE_Z :    NowRotZ   = *KeyLinear1 ; NextRotZ   = *KeyLinear2 ; RotRateZ = t ; break ;
					case MV1_ANIMKEY_DATATYPE_SHAPE :
						fdata = *KeyLinear1 + ( *KeyLinear2 - *KeyLinear1 ) * t ;
						Anim->KeySet[ j ].ShapeKeyFactor = fdata ;

						if( Anim->KeySet[ j ].ShapeTargetIndex != -1 )
						{
							float delta ;

							if( Anim->Frame )
							{
								delta = Anim->ShapeRate[ Anim->KeySet[ j ].ShapeTargetIndex ] - Anim->KeySet[ j ].ShapeKeyFactor ;
								if( delta < 0.0f ) delta = -delta ;

								if( delta >= 0.0000001f )
								{
									Anim->Frame->ShapeChangeFlag = true ;
									Anim->Frame->Container->ShapeChangeFlag = true ;
								}
							}

							Anim->ShapeRate[ Anim->KeySet[ j ].ShapeTargetIndex ] = Anim->KeySet[ j ].ShapeKeyFactor ;
						}

//						if( Anim->Frame )
//						{
//							if( Anim->KeySet[ j ].ShapeTargetIndex != -1 )
//							{
//								MV1_SHAPE *Shape ;
//								float delta ;
//
//								Shape = &Anim->Frame->Shape[ Anim->KeySet[ j ].ShapeTargetIndex ] ;
//								if( Anim->Frame->ShapeChangeFlag == false )
//								{
//									delta = Shape->Rate - Anim->KeySet[ j ].ShapeKeyFactor ;
//									if( delta < 0 ) delta = -delta ;
//									if( delta >= 0.0000001f )
//									{
//										Anim->Frame->ShapeChangeFlag = true ;
//										Anim->Frame->Container->ShapeChangeFlag = true ;
//									}
//								}
//								Shape->Rate = Anim->KeySet[ j ].ShapeKeyFactor ;
//							}
//						}
						break ;
					}
					break ;

				case MV1_ANIMKEY_TYPE_VECTOR :
					KeyVector1 = &KeySetBase->KeyVector[ NowKey ] ;
					KeyVector2 = &KeySetBase->KeyVector[ NextKey ] ;

					switch( KeySetBase->DataType )
					{
					case MV1_ANIMKEY_DATATYPE_ROTATE :
						NowRot  = *KeyVector1 ;
						NextRot = *KeyVector2 ;
						RotRate = t ;
						break ;

					case MV1_ANIMKEY_DATATYPE_SCALE :
						Scale.x = KeyVector1->x + ( KeyVector2->x - KeyVector1->x ) * t ;
						Scale.y = KeyVector1->y + ( KeyVector2->y - KeyVector1->y ) * t ;
						Scale.z = KeyVector1->z + ( KeyVector2->z - KeyVector1->z ) * t ;
						break ;

					case MV1_ANIMKEY_DATATYPE_TRANSLATE :
						Pos.x = KeyVector1->x + ( KeyVector2->x - KeyVector1->x ) * t ;
						Pos.y = KeyVector1->y + ( KeyVector2->y - KeyVector1->y ) * t ;
						Pos.z = KeyVector1->z + ( KeyVector2->z - KeyVector1->z ) * t ;
						break ;
					}
					break ;

				case MV1_ANIMKEY_TYPE_QUATERNION_X :
				case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
					QtType = KeySetBase->Type ;
					q1 = &KeySetBase->KeyFloat4[ NowKey ] ;
					q2 = &KeySetBase->KeyFloat4[ NextKey ] ;

					NowQt = *q1 ;
					NextQt = *q2 ;
					QtRate = t ;
					break ;
				}
			}

			// 情報をセット
			Anim->ValidFlag = MV1_ANIMVALUE_MATRIX ;

			if( RotRate > -0.5f )
			{
				switch( RotateOrder )
				{
				case MV1_ROTATE_ORDER_XYZ : CreateRotationXYZMatrix4X4CTF( &RotMat1, NowRot.x, NowRot.y, NowRot.z ) ; CreateRotationXYZMatrix4X4CTF( &RotMat2, NextRot.x, NextRot.y, NextRot.z ) ; break ;
				case MV1_ROTATE_ORDER_XZY : CreateRotationXZYMatrix4X4CTF( &RotMat1, NowRot.x, NowRot.y, NowRot.z ) ; CreateRotationXZYMatrix4X4CTF( &RotMat2, NextRot.x, NextRot.y, NextRot.z ) ; break ;
				case MV1_ROTATE_ORDER_YXZ : CreateRotationYXZMatrix4X4CTF( &RotMat1, NowRot.x, NowRot.y, NowRot.z ) ; CreateRotationYXZMatrix4X4CTF( &RotMat2, NextRot.x, NextRot.y, NextRot.z ) ; break ;
				case MV1_ROTATE_ORDER_YZX : CreateRotationYZXMatrix4X4CTF( &RotMat1, NowRot.x, NowRot.y, NowRot.z ) ; CreateRotationYZXMatrix4X4CTF( &RotMat2, NextRot.x, NextRot.y, NextRot.z ) ; break ;
				case MV1_ROTATE_ORDER_ZXY : CreateRotationZXYMatrix4X4CTF( &RotMat1, NowRot.x, NowRot.y, NowRot.z ) ; CreateRotationZXYMatrix4X4CTF( &RotMat2, NextRot.x, NextRot.y, NextRot.z ) ; break ;
				case MV1_ROTATE_ORDER_ZYX : CreateRotationZYXMatrix4X4CTF( &RotMat1, NowRot.x, NowRot.y, NowRot.z ) ; CreateRotationZYXMatrix4X4CTF( &RotMat2, NextRot.x, NextRot.y, NextRot.z ) ; break ;
				default: return -1;
				}

				Anim->Matrix.m[ 0 ][ 0 ] = RotMat1.m[ 0 ][ 0 ] + ( RotMat2.m[ 0 ][ 0 ] - RotMat1.m[ 0 ][ 0 ] ) * RotRate ;
				Anim->Matrix.m[ 0 ][ 1 ] = RotMat1.m[ 0 ][ 1 ] + ( RotMat2.m[ 0 ][ 1 ] - RotMat1.m[ 0 ][ 1 ] ) * RotRate ;
				Anim->Matrix.m[ 0 ][ 2 ] = RotMat1.m[ 0 ][ 2 ] + ( RotMat2.m[ 0 ][ 2 ] - RotMat1.m[ 0 ][ 2 ] ) * RotRate ;

				Anim->Matrix.m[ 1 ][ 0 ] = RotMat1.m[ 1 ][ 0 ] + ( RotMat2.m[ 1 ][ 0 ] - RotMat1.m[ 1 ][ 0 ] ) * RotRate ;
				Anim->Matrix.m[ 1 ][ 1 ] = RotMat1.m[ 1 ][ 1 ] + ( RotMat2.m[ 1 ][ 1 ] - RotMat1.m[ 1 ][ 1 ] ) * RotRate ;
				Anim->Matrix.m[ 1 ][ 2 ] = RotMat1.m[ 1 ][ 2 ] + ( RotMat2.m[ 1 ][ 2 ] - RotMat1.m[ 1 ][ 2 ] ) * RotRate ;

				Anim->Matrix.m[ 2 ][ 0 ] = RotMat1.m[ 2 ][ 0 ] + ( RotMat2.m[ 2 ][ 0 ] - RotMat1.m[ 2 ][ 0 ] ) * RotRate ;
				Anim->Matrix.m[ 2 ][ 1 ] = RotMat1.m[ 2 ][ 1 ] + ( RotMat2.m[ 2 ][ 1 ] - RotMat1.m[ 2 ][ 1 ] ) * RotRate ;
				Anim->Matrix.m[ 2 ][ 2 ] = RotMat1.m[ 2 ][ 2 ] + ( RotMat2.m[ 2 ][ 2 ] - RotMat1.m[ 2 ][ 2 ] ) * RotRate ;
			}
			else
			if( QtRate > 0.0f )
			{
				switch( QtType )
				{
				case MV1_ANIMKEY_TYPE_QUATERNION_X :
					CreateQuaternionRotateMatrix4X4CTF( &RotMat1, &NowQt  ) ;
					CreateQuaternionRotateMatrix4X4CTF( &RotMat2, &NextQt ) ;
					break ;

				case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
					float x2, y2, z2, xy, yz, zx, xw, yw, zw ;

					x2 = NowQt.x * NowQt.x * 2.0f ;
					y2 = NowQt.y * NowQt.y * 2.0f ;
					z2 = NowQt.z * NowQt.z * 2.0f ;
					xy = NowQt.x * NowQt.y * 2.0f ;
					yz = NowQt.y * NowQt.z * 2.0f ;
					zx = NowQt.z * NowQt.x * 2.0f ;
					xw = NowQt.x * NowQt.w * 2.0f ;
					yw = NowQt.y * NowQt.w * 2.0f ;
					zw = NowQt.z * NowQt.w * 2.0f ;

					RotMat1.m[ 0 ][ 0 ] = 1.0f - y2 - z2 ;
					RotMat1.m[ 1 ][ 0 ] = xy + zw ;
					RotMat1.m[ 2 ][ 0 ] = zx - yw ;
					RotMat1.m[ 0 ][ 1 ] = xy - zw ;
					RotMat1.m[ 1 ][ 1 ] = 1.0f - z2 - x2 ;
					RotMat1.m[ 2 ][ 1 ] = yz + xw ;
					RotMat1.m[ 0 ][ 2 ] = zx + yw ;
					RotMat1.m[ 1 ][ 2 ] = yz - xw ;
					RotMat1.m[ 2 ][ 2 ] = 1.0f - x2 - y2 ;

					x2 = NextQt.x * NextQt.x * 2.0f ;
					y2 = NextQt.y * NextQt.y * 2.0f ;
					z2 = NextQt.z * NextQt.z * 2.0f ;
					xy = NextQt.x * NextQt.y * 2.0f ;
					yz = NextQt.y * NextQt.z * 2.0f ;
					zx = NextQt.z * NextQt.x * 2.0f ;
					xw = NextQt.x * NextQt.w * 2.0f ;
					yw = NextQt.y * NextQt.w * 2.0f ;
					zw = NextQt.z * NextQt.w * 2.0f ;

					RotMat2.m[ 0 ][ 0 ] = 1.0f - y2 - z2 ;
					RotMat2.m[ 1 ][ 0 ] = xy + zw ;
					RotMat2.m[ 2 ][ 0 ] = zx - yw ;
					RotMat2.m[ 0 ][ 1 ] = xy - zw ;
					RotMat2.m[ 1 ][ 1 ] = 1.0f - z2 - x2 ;
					RotMat2.m[ 2 ][ 1 ] = yz + xw ;
					RotMat2.m[ 0 ][ 2 ] = zx + yw ;
					RotMat2.m[ 1 ][ 2 ] = yz - xw ;
					RotMat2.m[ 2 ][ 2 ] = 1.0f - x2 - y2 ;
					break ;

				default :
					return -1 ;
				}

				Anim->Matrix.m[ 0 ][ 0 ] = RotMat1.m[ 0 ][ 0 ] + ( RotMat2.m[ 0 ][ 0 ] - RotMat1.m[ 0 ][ 0 ] ) * QtRate ;
				Anim->Matrix.m[ 0 ][ 1 ] = RotMat1.m[ 0 ][ 1 ] + ( RotMat2.m[ 0 ][ 1 ] - RotMat1.m[ 0 ][ 1 ] ) * QtRate ;
				Anim->Matrix.m[ 0 ][ 2 ] = RotMat1.m[ 0 ][ 2 ] + ( RotMat2.m[ 0 ][ 2 ] - RotMat1.m[ 0 ][ 2 ] ) * QtRate ;

				Anim->Matrix.m[ 1 ][ 0 ] = RotMat1.m[ 1 ][ 0 ] + ( RotMat2.m[ 1 ][ 0 ] - RotMat1.m[ 1 ][ 0 ] ) * QtRate ;
				Anim->Matrix.m[ 1 ][ 1 ] = RotMat1.m[ 1 ][ 1 ] + ( RotMat2.m[ 1 ][ 1 ] - RotMat1.m[ 1 ][ 1 ] ) * QtRate ;
				Anim->Matrix.m[ 1 ][ 2 ] = RotMat1.m[ 1 ][ 2 ] + ( RotMat2.m[ 1 ][ 2 ] - RotMat1.m[ 1 ][ 2 ] ) * QtRate ;

				Anim->Matrix.m[ 2 ][ 0 ] = RotMat1.m[ 2 ][ 0 ] + ( RotMat2.m[ 2 ][ 0 ] - RotMat1.m[ 2 ][ 0 ] ) * QtRate ;
				Anim->Matrix.m[ 2 ][ 1 ] = RotMat1.m[ 2 ][ 1 ] + ( RotMat2.m[ 2 ][ 1 ] - RotMat1.m[ 2 ][ 1 ] ) * QtRate ;
				Anim->Matrix.m[ 2 ][ 2 ] = RotMat1.m[ 2 ][ 2 ] + ( RotMat2.m[ 2 ][ 2 ] - RotMat1.m[ 2 ][ 2 ] ) * QtRate ;
			}
			else
			if( RotRateX < -0.5f && RotRateY < -0.5f && RotRateZ < -0.5f ) 
			{
				if( NowRot.x != 0.0f || NowRot.y != 0.0f || NowRot.z != 0.0f )
				{
					switch( RotateOrder )
					{
					case MV1_ROTATE_ORDER_XYZ : CreateRotationXYZMatrix4X4CTF( &Anim->Matrix, NowRot.x, NowRot.y, NowRot.z ) ; break ;
					case MV1_ROTATE_ORDER_XZY : CreateRotationXZYMatrix4X4CTF( &Anim->Matrix, NowRot.x, NowRot.y, NowRot.z ) ; break ;
					case MV1_ROTATE_ORDER_YXZ : CreateRotationYXZMatrix4X4CTF( &Anim->Matrix, NowRot.x, NowRot.y, NowRot.z ) ; break ;
					case MV1_ROTATE_ORDER_YZX : CreateRotationYZXMatrix4X4CTF( &Anim->Matrix, NowRot.x, NowRot.y, NowRot.z ) ; break ;
					case MV1_ROTATE_ORDER_ZXY : CreateRotationZXYMatrix4X4CTF( &Anim->Matrix, NowRot.x, NowRot.y, NowRot.z ) ; break ;
					case MV1_ROTATE_ORDER_ZYX : CreateRotationZYXMatrix4X4CTF( &Anim->Matrix, NowRot.x, NowRot.y, NowRot.z ) ; break ;
					default :
						return -1 ;
					}
				}
				else
				{
					Anim->Matrix.m[ 0 ][ 0 ] = 1.0f ;
					Anim->Matrix.m[ 0 ][ 1 ] = 0.0f ;
					Anim->Matrix.m[ 0 ][ 2 ] = 0.0f ;

					Anim->Matrix.m[ 1 ][ 0 ] = 0.0f ;
					Anim->Matrix.m[ 1 ][ 1 ] = 1.0f ;
					Anim->Matrix.m[ 1 ][ 2 ] = 0.0f ;

					Anim->Matrix.m[ 2 ][ 0 ] = 0.0f ;
					Anim->Matrix.m[ 2 ][ 1 ] = 0.0f ;
					Anim->Matrix.m[ 2 ][ 2 ] = 1.0f ;
				}
			}
			else
			{
				MATRIX_4X4CT_F RotXMatTemp, RotYMatTemp, RotZMatTemp ;
				MATRIX_4X4CT_F TempMat ;
				float Sin1, Cos1, Sin2, Cos2 ;


				RotXMatTemp.m[ 0 ][ 0 ] = 1.0f ;
				RotXMatTemp.m[ 0 ][ 1 ] = 0.0f ;
				RotXMatTemp.m[ 0 ][ 2 ] = 0.0f ;
				RotXMatTemp.m[ 0 ][ 3 ] = 0.0f ;

				RotXMatTemp.m[ 1 ][ 0 ] = 0.0f ;
				RotXMatTemp.m[ 1 ][ 3 ] = 0.0f ;

				RotXMatTemp.m[ 2 ][ 0 ] = 0.0f ;
				RotXMatTemp.m[ 2 ][ 3 ] = 0.0f ;
				if( RotRateX > -0.5f )
				{
					TableSinCos( NowRotX,  &Sin1, &Cos1 ) ;
					TableSinCos( NextRotX, &Sin2, &Cos2 ) ;

					RotXMatTemp.m[ 1 ][ 1 ] =  Cos1 + ( Cos2 - Cos1 ) * RotRateX ;
					RotXMatTemp.m[ 2 ][ 1 ] =  Sin1 + ( Sin2 - Sin1 ) * RotRateX ;
				}
				else
				{
					TableSinCos( NowRotX,  &Sin1, &Cos1 ) ;

					RotXMatTemp.m[ 1 ][ 1 ] =  Cos1 ;
					RotXMatTemp.m[ 2 ][ 1 ] =  Sin1 ;
				}
				RotXMatTemp.m[ 1 ][ 2 ] = -RotXMatTemp.m[ 2 ][ 1 ] ;
				RotXMatTemp.m[ 2 ][ 2 ] =  RotXMatTemp.m[ 1 ][ 1 ] ;


				RotYMatTemp.m[ 0 ][ 1 ] = 0.0f ;
				RotYMatTemp.m[ 0 ][ 3 ] = 0.0f ;

				RotYMatTemp.m[ 1 ][ 0 ] = 0.0f ;
				RotYMatTemp.m[ 1 ][ 1 ] = 1.0f ;
				RotYMatTemp.m[ 1 ][ 2 ] = 0.0f ;
				RotYMatTemp.m[ 1 ][ 3 ] = 0.0f ;

				RotYMatTemp.m[ 2 ][ 1 ] = 0.0f ;
				RotYMatTemp.m[ 2 ][ 3 ] = 0.0f ;
				if( RotRateY > -0.5f )
				{
					TableSinCos( NowRotY,  &Sin1, &Cos1 ) ;
					TableSinCos( NextRotY, &Sin2, &Cos2 ) ;

					RotYMatTemp.m[ 0 ][ 0 ] =  Cos1 + ( Cos2 - Cos1 ) * RotRateY ;
					RotYMatTemp.m[ 0 ][ 2 ] =  Sin1 + ( Sin2 - Sin1 ) * RotRateY ;
				}
				else
				{
					TableSinCos( NowRotY,  &Sin1, &Cos1 ) ;

					RotYMatTemp.m[ 0 ][ 0 ] =  Cos1 ;
					RotYMatTemp.m[ 0 ][ 2 ] =  Sin1 ;
				}
				RotYMatTemp.m[ 2 ][ 0 ] = -RotYMatTemp.m[ 0 ][ 2 ] ;
				RotYMatTemp.m[ 2 ][ 2 ] =  RotYMatTemp.m[ 0 ][ 0 ] ;


				RotZMatTemp.m[ 0 ][ 2 ] = 0.0f ;
				RotZMatTemp.m[ 0 ][ 3 ] = 0.0f ;

				RotZMatTemp.m[ 1 ][ 2 ] = 0.0f ;
				RotZMatTemp.m[ 1 ][ 3 ] = 0.0f ;

				RotZMatTemp.m[ 2 ][ 0 ] = 0.0f ;
				RotZMatTemp.m[ 2 ][ 1 ] = 0.0f ;
				RotZMatTemp.m[ 2 ][ 2 ] = 1.0f ;
				RotZMatTemp.m[ 2 ][ 3 ] = 0.0f ;
				if( RotRateZ > -0.5f )
				{
					TableSinCos( NowRotZ,  &Sin1, &Cos1 ) ;
					TableSinCos( NextRotZ, &Sin2, &Cos2 ) ;

					RotZMatTemp.m[ 0 ][ 0 ] = Cos1 + ( Cos2 - Cos1 ) * RotRateZ ;
					RotZMatTemp.m[ 1 ][ 0 ] = Sin1 + ( Sin2 - Sin1 ) * RotRateZ ;
				}
				else
				{
					TableSinCos( NowRotZ,  &Sin1, &Cos1 ) ;

					RotZMatTemp.m[ 0 ][ 0 ] = Cos1 ;
					RotZMatTemp.m[ 1 ][ 0 ] = Sin1 ;
				}
				RotZMatTemp.m[ 0 ][ 1 ] = -RotZMatTemp.m[ 1 ][ 0 ] ;
				RotZMatTemp.m[ 1 ][ 1 ] =  RotZMatTemp.m[ 0 ][ 0 ] ;


				switch( RotateOrder )
				{
				case MV1_ROTATE_ORDER_XYZ :
					UnSafeMultiplyMatrix4X4CTF( &TempMat, &RotXMatTemp, &RotYMatTemp ) ;
					UnSafeMultiplyMatrix4X4CTF( &Anim->Matrix, &TempMat, &RotZMatTemp ) ;
					break ;

				case MV1_ROTATE_ORDER_XZY :
					UnSafeMultiplyMatrix4X4CTF( &TempMat, &RotXMatTemp, &RotZMatTemp ) ;
					UnSafeMultiplyMatrix4X4CTF( &Anim->Matrix, &TempMat, &RotYMatTemp ) ;
					break ;

				case MV1_ROTATE_ORDER_YZX :
					UnSafeMultiplyMatrix4X4CTF( &TempMat, &RotYMatTemp, &RotZMatTemp ) ;
					UnSafeMultiplyMatrix4X4CTF( &Anim->Matrix, &TempMat, &RotXMatTemp ) ;
					break ;

				case MV1_ROTATE_ORDER_ZXY :
					UnSafeMultiplyMatrix4X4CTF( &TempMat, &RotZMatTemp, &RotXMatTemp ) ;
					UnSafeMultiplyMatrix4X4CTF( &Anim->Matrix, &TempMat, &RotYMatTemp ) ;
					break ;

				case MV1_ROTATE_ORDER_ZYX :
					UnSafeMultiplyMatrix4X4CTF( &TempMat, &RotZMatTemp, &RotYMatTemp ) ;
					UnSafeMultiplyMatrix4X4CTF( &Anim->Matrix, &TempMat, &RotXMatTemp ) ;
					break ;
				}
			}

			// 前後行列の掛け合わせ
			{
				MATRIX_4X4CT_F TempMatrix ;

				if( PreRotate != NULL && PostRotate != NULL )
				{
					UnSafeMultiplyMatrix4X4CTF( &TempMatrix, &Anim->Matrix, &PreRotMat ) ;
					UnSafeMultiplyMatrix4X4CTF( &Anim->Matrix, &PostRotMat, &TempMatrix ) ;
				}
				else
				if( PreRotate != NULL )
				{
					UnSafeMultiplyMatrix4X4CTF( &TempMatrix, &Anim->Matrix, &PreRotMat ) ;
					Anim->Matrix = TempMatrix ;
				}
				else
				if( PostRotate != NULL )
				{
					UnSafeMultiplyMatrix4X4CTF( &TempMatrix, &PostRotMat, &Anim->Matrix ) ;
					Anim->Matrix = TempMatrix ;
				}
			}

			Anim->Matrix.m[ 0 ][ 3 ] = Pos.x ;
			Anim->Matrix.m[ 1 ][ 3 ] = Pos.y ;
			Anim->Matrix.m[ 2 ][ 3 ] = Pos.z ;

			if( Scale.x != 1.0f || Scale.y != 1.0f || Scale.z != 1.0f )
			{
				Anim->Matrix.m[ 0 ][ 0 ] *= Scale.x ;
				Anim->Matrix.m[ 0 ][ 1 ] *= Scale.y ;
				Anim->Matrix.m[ 0 ][ 2 ] *= Scale.z ;
				Anim->Matrix.m[ 1 ][ 0 ] *= Scale.x ;
				Anim->Matrix.m[ 1 ][ 1 ] *= Scale.y ;
				Anim->Matrix.m[ 1 ][ 2 ] *= Scale.z ;
				Anim->Matrix.m[ 2 ][ 0 ] *= Scale.x ;
				Anim->Matrix.m[ 2 ][ 1 ] *= Scale.y ;
				Anim->Matrix.m[ 2 ][ 2 ] *= Scale.z ;
			}

			// 目標ノードの行列を更新するようにセット
			if( Anim->Frame )
			{
				Anim->Frame->Container->LocalWorldMatrixSetupFlag = false ;
				MV1BitSetChange( &Anim->Frame->LocalWorldMatrixChange ) ;
			}
		}
	}
	else
	{
		Anim = AnimSet->Anim ;
		for( i = 0 ; i < AnimSetBase->AnimNum ; i ++, Anim ++ )
		{
			// セットアップ情報をリセット
			Anim->ValidFlag = 0 ;

			// 初期状態では目標ノードのデフォルト値を入れておく
			ValidMatrix = FALSE ;
			if( Anim->Frame )
			{
				Translate   = Anim->Frame->BaseData->Translate ;
				Rotate      = Anim->Frame->BaseData->Rotate ;
				Scale       = Anim->Frame->BaseData->Scale ;
				Quaternion  = Anim->Frame->BaseData->Quaternion ;
			}
			else
			{
				Translate.x = 0.0f ;
				Translate.y = 0.0f ;
				Translate.z = 0.0f ;

				Rotate.x = 0.0f ;
				Rotate.y = 0.0f ;
				Rotate.z = 0.0f ;

				Scale.x = 1.0f ;
				Scale.y = 1.0f ;
				Scale.z = 1.0f ;

				Quaternion.w = 1.0f ;
				Quaternion.x = 0.0f ;
				Quaternion.y = 0.0f ;
				Quaternion.z = 0.0f ;
			}
			RotateOrder = Anim->RotateOrder ;

			// 各要素の現在値割り出す
			for( j = 0 ; j < Anim->BaseData->KeySetNum ; j ++ )
			{
				NowKey     = Anim->KeySet[ j ].NowKey ;
				KeySetBase = Anim->KeySet[ j ].BaseData ;

				if( KeySetBase->Num == 0 )
				{
					continue ;
				}

				NextKey    = NowKey == KeySetBase->Num - 1 ? 0 : NowKey + 1 ;

				if( NowKey == KeySetBase->Num - 1 )
				{
					t = 0.0f ;
				}
				else
				{
					if( KeySetBase->TimeType == MV1_ANIMKEY_TIME_TYPE_ONE )
					{
						t = ( AnimSet->NowTime - ( KeySetBase->UnitTime * NowKey + KeySetBase->StartTime ) ) / KeySetBase->UnitTime ;
					}
					else
					{
						t = ( AnimSet->NowTime -   KeySetBase->KeyTime[ NowKey ]                                                                ) /   ( KeySetBase->KeyTime[ NowKey + 1 ]                                                                -   KeySetBase->KeyTime[ NowKey ]                                                                ) ;
					}
					if( t < 0.0f ) t = 0.0f ;
				}

				switch( KeySetBase->Type )
				{
				case MV1_ANIMKEY_TYPE_LINEAR :
					KeyLinear1 = &KeySetBase->KeyLinear[ NowKey  ] ;
					KeyLinear2 = &KeySetBase->KeyLinear[ NextKey ] ;

					Sub = *KeyLinear2 - *KeyLinear1 ;
					// 90度以上の急激な変化があった場合は行列ブレンドを行う
					if( KeySetBase->DataType >= MV1_ANIMKEY_DATATYPE_ROTATE_X &&
						KeySetBase->DataType <= MV1_ANIMKEY_DATATYPE_ROTATE_Z &&
						( Sub < -DX_PI_F * 0.5f || Sub > DX_PI_F * 0.5f ) )
						goto MATRIXLINEARBLEND ;
					fdata = *KeyLinear1 + Sub * t ;

					if( AddParam )
					{
						switch( KeySetBase->DataType )
						{
						case MV1_ANIMKEY_DATATYPE_TRANSLATE_X	: Translate.x += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_TRANSLATE_Y	: Translate.y += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_TRANSLATE_Z	: Translate.z += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE_X :     Scale.x     += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE_Y :     Scale.y     += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE_Z :     Scale.z     += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_ROTATE_X :    Rotate.x    += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_ROTATE_Y :    Rotate.y    += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_ROTATE_Z :    Rotate.z    += fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SHAPE :
							Anim->KeySet[ j ].ShapeKeyFactor = fdata ;

							if( Anim->KeySet[ j ].ShapeTargetIndex != -1 )
							{
								float delta ;

								if( Anim->Frame )
								{
									delta = Anim->ShapeRate[ Anim->KeySet[ j ].ShapeTargetIndex ] - Anim->KeySet[ j ].ShapeKeyFactor ;
									if( delta < 0.0f ) delta = -delta ;

									if( delta >= 0.0000001f )
									{
										Anim->Frame->ShapeChangeFlag = true ;
										Anim->Frame->Container->ShapeChangeFlag = true ;
									}
								}

								Anim->ShapeRate[ Anim->KeySet[ j ].ShapeTargetIndex ] = Anim->KeySet[ j ].ShapeKeyFactor ;
							}

//							if( Anim->Frame )
//							{
//								if( Anim->KeySet[ j ].ShapeTargetIndex != -1 )
//								{
//									MV1_SHAPE *Shape ;
//									float delta ;
//
//									Shape = &Anim->Frame->Shape[ Anim->KeySet[ j ].ShapeTargetIndex ] ;
//									if( Anim->Frame->ShapeChangeFlag == false )
//									{
//										delta = Shape->Rate - Anim->KeySet[ j ].ShapeKeyFactor ;
//										if( delta < 0 ) delta = -delta ;
//										if( delta >= 0.0000001f )
//										{
//											Anim->Frame->ShapeChangeFlag = true ;
//											Anim->Frame->Container->ShapeChangeFlag = true ;
//										}
//									}
//									Shape->Rate = Anim->KeySet[ j ].ShapeKeyFactor ;
//								}
//							}
							break ;
						}
					}
					else
					{
						switch( KeySetBase->DataType )
						{
						case MV1_ANIMKEY_DATATYPE_TRANSLATE_X	: Translate.x = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_TRANSLATE_Y	: Translate.y = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_TRANSLATE_Z	: Translate.z = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE_X :     Scale.x     = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE_Y :     Scale.y     = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE_Z :     Scale.z     = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_ROTATE_X :    Rotate.x    = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_ROTATE_Y :    Rotate.y    = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_ROTATE_Z :    Rotate.z    = fdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SHAPE :
							Anim->KeySet[ j ].ShapeKeyFactor = fdata ;

							if( Anim->KeySet[ j ].ShapeTargetIndex != -1 )
							{
								float delta ;

								if( Anim->Frame )
								{
									delta = Anim->ShapeRate[ Anim->KeySet[ j ].ShapeTargetIndex ] - Anim->KeySet[ j ].ShapeKeyFactor ;
									if( delta < 0.0f ) delta = -delta ;

									if( delta >= 0.0000001f )
									{
										Anim->Frame->ShapeChangeFlag = true ;
										Anim->Frame->Container->ShapeChangeFlag = true ;
									}
								}

								Anim->ShapeRate[ Anim->KeySet[ j ].ShapeTargetIndex ] = Anim->KeySet[ j ].ShapeKeyFactor ;
							}

//							if( Anim->Frame )
//							{
//								if( Anim->KeySet[ j ].ShapeTargetIndex != -1 )
//								{
//									MV1_SHAPE *Shape ;
//									float delta ;
//
//									Shape = &Anim->Frame->Shape[ Anim->KeySet[ j ].ShapeTargetIndex ] ;
//									if( Anim->Frame->ShapeChangeFlag == false )
//									{
//										delta = Shape->Rate - Anim->KeySet[ j ].ShapeKeyFactor ;
//										if( delta < 0 ) delta = -delta ;
//										if( delta >= 0.0000001f )
//										{
//											Anim->Frame->ShapeChangeFlag = true ;
//											Anim->Frame->Container->ShapeChangeFlag = true ;
//										}
//									}
//									Shape->Rate = Anim->KeySet[ j ].ShapeKeyFactor ;
//								}
//							}
							break ;
						}
					}
					break ;

				case MV1_ANIMKEY_TYPE_VECTOR :
					KeyVector1 = &KeySetBase->KeyVector[ NowKey ] ;
					KeyVector2 = &KeySetBase->KeyVector[ NextKey ] ;

					SubVec.x = KeyVector2->x - KeyVector1->x ;
					SubVec.y = KeyVector2->y - KeyVector1->y ;
					SubVec.z = KeyVector2->z - KeyVector1->z ;
					// 90度以上の急激な変化があった場合は行列ブレンドを行う
					if( KeySetBase->DataType == MV1_ANIMKEY_DATATYPE_ROTATE &&
						( SubVec.x < -DX_PI_F * 0.5f || SubVec.x > DX_PI_F * 0.5f ||
						  SubVec.y < -DX_PI_F * 0.5f || SubVec.y > DX_PI_F * 0.5f ||
						  SubVec.z < -DX_PI_F * 0.5f || SubVec.z > DX_PI_F * 0.5f ) )
						goto MATRIXLINEARBLEND ;
					vdata.x = KeyVector1->x + SubVec.x * t ;
					vdata.y = KeyVector1->y + SubVec.y * t ;
					vdata.z = KeyVector1->z + SubVec.z * t ;

					if( AddParam )
					{
						switch( KeySetBase->DataType )
						{
						case MV1_ANIMKEY_DATATYPE_ROTATE :     Rotate     = VAdd( Rotate,    vdata ) ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE :      Scale      = VAdd( Scale,     vdata ) ; break ;
						case MV1_ANIMKEY_DATATYPE_TRANSLATE :  Translate  = VAdd( Translate, vdata ) ; break ;
						}
					}
					else
					{
						switch( KeySetBase->DataType )
						{
						case MV1_ANIMKEY_DATATYPE_ROTATE :     Rotate     = vdata ; break ;
						case MV1_ANIMKEY_DATATYPE_SCALE :      Scale      = vdata ; break ;
						case MV1_ANIMKEY_DATATYPE_TRANSLATE :  Translate  = vdata ; break ;
						}
					}
					break ;

				case MV1_ANIMKEY_TYPE_QUATERNION_X :
				case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
					QtType = KeySetBase->Type ;
					q1 = &KeySetBase->KeyFloat4[ NowKey ] ;
					q2 = &KeySetBase->KeyFloat4[ NextKey ] ;

#if 0
					// 球面線形補間
					float d, f, angle, sin1, sin2, sin3 ;
					f = q2->x * q1->x + q2->y * q1->y + q2->z * q1->z + q2->w * q1->w ;
					if( f >  0.999999f ) f =  0.999999f ;
					if( f < -0.999999f ) f = -0.999999f ;

					angle = _ACOS( f ) ;

					TableSinCos( angle                , &sin1, &d ) ;
					TableSinCos( angle * ( 1.0f - t ) , &sin2, &d ) ;
					TableSinCos( angle * t            , &sin3, &d ) ;
					sin1 = 1.0f / sin1 ;

					Quaternion.x = ( q1->x * sin2 + q2->x * sin3 ) * sin1 ;
					Quaternion.y = ( q1->y * sin2 + q2->y * sin3 ) * sin1 ;
					Quaternion.z = ( q1->z * sin2 + q2->z * sin3 ) * sin1 ;
					Quaternion.w = ( q1->w * sin2 + q2->w * sin3 ) * sin1 ;
#endif
#if 0
					float	qr = q1->x * q2->x + q1->y * q2->y + q1->z * q2->z + q1->w * q2->w ;
					float	t0 = 1.0f - t ;
					FLOAT4  qdata ;

					if( qr < 0 )
					{
						qdata.x = q1->x * t0 - q2->x * t ;
						qdata.y = q1->y * t0 - q2->y * t ;
						qdata.z = q1->z * t0 - q2->z * t ;
						qdata.w = q1->w * t0 - q2->w * t ;
					}
					else
					{
						qdata.x = q1->x * t0 + q2->x * t ;
						qdata.y = q1->y * t0 + q2->y * t ;
						qdata.z = q1->z * t0 + q2->z * t ;
						qdata.w = q1->w * t0 + q2->w * t ;
					}

					float Sqr = 1.0f / _SQRT( qdata.x * qdata.x + qdata.y * qdata.y + qdata.z * qdata.z + qdata.w * qdata.w ) ;

					Quaternion.x = qdata.x * Sqr ;
					Quaternion.y = qdata.y * Sqr ;
					Quaternion.z = qdata.z * Sqr ;
					Quaternion.w = qdata.w * Sqr ;
#endif
#if 1
					_MV1SphereLinear( q1, q2, t, &Quaternion ) ;
#endif
#if 0
					Quaternion = *q1 ;
#endif
					break ;

				case MV1_ANIMKEY_TYPE_MATRIX4X4C :
					KeyMatrix4x4C1 = &KeySetBase->KeyMatrix4x4C[ NowKey  ] ;
					KeyMatrix4x4C2 = &KeySetBase->KeyMatrix4x4C[ NextKey ] ;

					ValidMatrix = TRUE ;
					Matrix.m[ 0 ][ 0 ] = ( KeyMatrix4x4C2->Matrix[ 0 ][ 0 ] - KeyMatrix4x4C1->Matrix[ 0 ][ 0 ] ) * t + KeyMatrix4x4C1->Matrix[ 0 ][ 0 ] ;
					Matrix.m[ 0 ][ 1 ] = ( KeyMatrix4x4C2->Matrix[ 1 ][ 0 ] - KeyMatrix4x4C1->Matrix[ 1 ][ 0 ] ) * t + KeyMatrix4x4C1->Matrix[ 1 ][ 0 ] ;
					Matrix.m[ 0 ][ 2 ] = ( KeyMatrix4x4C2->Matrix[ 2 ][ 0 ] - KeyMatrix4x4C1->Matrix[ 2 ][ 0 ] ) * t + KeyMatrix4x4C1->Matrix[ 2 ][ 0 ] ;
					Matrix.m[ 0 ][ 3 ] = ( KeyMatrix4x4C2->Matrix[ 3 ][ 0 ] - KeyMatrix4x4C1->Matrix[ 3 ][ 0 ] ) * t + KeyMatrix4x4C1->Matrix[ 3 ][ 0 ] ;

					Matrix.m[ 1 ][ 0 ] = ( KeyMatrix4x4C2->Matrix[ 0 ][ 1 ] - KeyMatrix4x4C1->Matrix[ 0 ][ 1 ] ) * t + KeyMatrix4x4C1->Matrix[ 0 ][ 1 ] ;
					Matrix.m[ 1 ][ 1 ] = ( KeyMatrix4x4C2->Matrix[ 1 ][ 1 ] - KeyMatrix4x4C1->Matrix[ 1 ][ 1 ] ) * t + KeyMatrix4x4C1->Matrix[ 1 ][ 1 ] ;
					Matrix.m[ 1 ][ 2 ] = ( KeyMatrix4x4C2->Matrix[ 2 ][ 1 ] - KeyMatrix4x4C1->Matrix[ 2 ][ 1 ] ) * t + KeyMatrix4x4C1->Matrix[ 2 ][ 1 ] ;
					Matrix.m[ 1 ][ 3 ] = ( KeyMatrix4x4C2->Matrix[ 3 ][ 1 ] - KeyMatrix4x4C1->Matrix[ 3 ][ 1 ] ) * t + KeyMatrix4x4C1->Matrix[ 3 ][ 1 ] ;

					Matrix.m[ 2 ][ 0 ] = ( KeyMatrix4x4C2->Matrix[ 0 ][ 2 ] - KeyMatrix4x4C1->Matrix[ 0 ][ 2 ] ) * t + KeyMatrix4x4C1->Matrix[ 0 ][ 2 ] ;
					Matrix.m[ 2 ][ 1 ] = ( KeyMatrix4x4C2->Matrix[ 1 ][ 2 ] - KeyMatrix4x4C1->Matrix[ 1 ][ 2 ] ) * t + KeyMatrix4x4C1->Matrix[ 1 ][ 2 ] ;
					Matrix.m[ 2 ][ 2 ] = ( KeyMatrix4x4C2->Matrix[ 2 ][ 2 ] - KeyMatrix4x4C1->Matrix[ 2 ][ 2 ] ) * t + KeyMatrix4x4C1->Matrix[ 2 ][ 2 ] ;
					Matrix.m[ 2 ][ 3 ] = ( KeyMatrix4x4C2->Matrix[ 3 ][ 2 ] - KeyMatrix4x4C1->Matrix[ 3 ][ 2 ] ) * t + KeyMatrix4x4C1->Matrix[ 3 ][ 2 ] ;
					break ;
				}
			}

			// 情報をセット
			if( ValidMatrix )
			{
				Anim->Matrix = Matrix ;
				Anim->ValidFlag = MV1_ANIMVALUE_MATRIX ;
			}
			else
			{
				Anim->Translate = Translate ;
				if( Translate.x != 0.0f || Translate.y != 0.0f || Translate.z != 0.0f )
					Anim->ValidFlag |= MV1_ANIMVALUE_TRANSLATE ;

				Anim->Scale = Scale ;
				if( Scale.x != 1.0f || Scale.y != 1.0f || Scale.z != 1.0f )
					Anim->ValidFlag |= MV1_ANIMVALUE_SCALE ;

				Anim->Rotate = Rotate ;
				Anim->RotateOrder = RotateOrder ;
				if( Rotate.x != 0.0f || Rotate.y != 0.0f || Rotate.z != 0.0f )
					Anim->ValidFlag |= MV1_ANIMVALUE_ROTATE ;

				Anim->Quaternion = Quaternion ;
				if( Quaternion.w != 1.0f || Quaternion.x != 0.0f || Quaternion.y != 0.0f || Quaternion.z != 0.0f )
				{
					Anim->ValidFlag |= QtType == MV1_ANIMKEY_TYPE_QUATERNION_X ? MV1_ANIMVALUE_QUATERNION_X : MV1_ANIMVALUE_QUATERNION_VMD ;
					Anim->ValidFlag &= ~MV1_ANIMVALUE_ROTATE ;
				}
			}

			// 目標ノードの行列を更新するようにセット
			if( Anim->Frame )
			{
				Anim->Frame->Container->LocalWorldMatrixSetupFlag = false ;
				MV1BitSetChange( &Anim->Frame->LocalWorldMatrixChange ) ;
			}

			// ブレンド行列を作成したフラグを倒す
			Anim->ValidBlendMatrix = false ;
		}
	}

	// パラメータのセットアップが済んだフラグを立てる
	AnimSet->ParamSetup = true ;

	// 終了
	return 0 ;
}







// メモリ管理系

// メモリ領域の確保
#if !defined( __BCC ) || defined( _DEBUG )
extern void *AddMemArea( size_t Size, MEMINFO **FirstMem, const char *FileName, int Line )
#else
extern void *AddMemArea( size_t Size, MEMINFO **FirstMem )
#endif
{
	MEMINFO *MI ;

	// メモリアドレス保存データを保存するメモリ領域と実際のデータを保存するメモリ領域を確保する
#if !defined( __BCC ) || defined( _DEBUG )
	if( ( MI = ( MEMINFO * )NS_DxAlloc( sizeof( MEMINFO ) + Size , FileName, Line ) ) == NULL )
#else
	if( ( MI = ( MEMINFO * )NS_DxAlloc( sizeof( MEMINFO ) + Size ) ) == NULL )
#endif
	{
		DXST_ERRORLOG_ADDUTF16LE( "\xe1\x30\xe2\x30\xea\x30\xa2\x30\xc9\x30\xec\x30\xb9\x30\xdd\x4f\x58\x5b\xc7\x30\xfc\x30\xbf\x30\x0b\xff\x9f\x5b\x9b\x96\x6e\x30\xc7\x30\xfc\x30\xbf\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"メモリアドレス保存データ＋実際のデータを保存するメモリ領域の確保に失敗しました\n" @*/ ) ;
		return NULL ;
	}

	// データをクリア
	_MEMSET( MI, 0, sizeof( MEMINFO ) + Size ) ;

	// 実際のデータの領域をセットする
	MI->Data = ( BYTE * )MI + sizeof( MEMINFO ) ;

	// データを加える
	MI->PrevInfo = NULL ;
	MI->NextInfo = *FirstMem ;
	if( *FirstMem == NULL )
	{
		MI->TotalSize = ( int )Size ;
	}
	else
	{
		( *FirstMem )->PrevInfo = MI ;
		MI->TotalSize = ( int )( (*FirstMem)->TotalSize + Size ) ;
	}
	*FirstMem = MI;

	// 確保したメモリ領域のアドレスを返す
	return MI->Data ;
}

// メモリ領域の解放
extern int SubMemArea( MEMINFO **FirstMem, void *Buffer )
{
	MEMINFO *MI/*, *MIB*/ ;

	if( Buffer == NULL )
		return 0 ;

	MI = ( MEMINFO * )( ( BYTE * )Buffer - sizeof( MEMINFO ) ) ;

	if( MI->NextInfo )
	{
		MI->NextInfo->PrevInfo = MI->PrevInfo ;
	}

	if( MI->PrevInfo )
	{
		MI->PrevInfo->NextInfo = MI->NextInfo ;
	}

	if( MI == *FirstMem )
	{
		*FirstMem = MI->NextInfo ;
	}

	DXFREE( MI ) ;

/*
	// 指定のメモリ領域を検索する
	MI = *FirstMem ;
	MIB = NULL ;
	while( MI && MI->Data != Buffer )
	{
		MIB = MI ;
		MI = MI->NextInfo ;
	}

	// 見つけたら解放する
	if( MI != NULL )
	{
		if( MIB == NULL )
		{
			*FirstMem = MI->NextInfo ;
		}
		else
		{
			MIB->NextInfo = MI->NextInfo ;
		}
		DXFREE( MI ) ;
	}
*/

	// 終了
	return 0 ;
}

// 全てのメモリ領域の解放
extern int ClearMemArea( MEMINFO **FirstMem )
{
	MEMINFO *MI, *MIB ;

	// モデルが AddMemArea 関数で確保していたメモリ領域をすべて解放
	MI = *FirstMem ;
	while( MI != NULL )
	{
		MIB = MI->NextInfo ;
		DXFREE( MI ) ;
		MI = MIB ;
	}

	// ＮＵＬＬをセット
	*FirstMem = NULL ;

	// 終了
	return 0 ;
}
























// データ一致検査系

// バイナリイメージからデータ一致検査データを作成する
extern void MakeDataCmpInfo( DATACMPINFO *CmpInfo, void *Image, int Size )
{
	int PackNum, AmariNum, i ;
	DWORD *Src ;

	// ゼロ初期化
	_MEMSET( CmpInfo, 0, sizeof( DATACMPINFO ) ) ;

	// サイズを保存
	CmpInfo->Size = ( DWORD )Size ;

	// ３２バイト何個分か調べる
	PackNum = Size / 32 ;

	// パックの数だけチェック用データを作成
	Src = ( DWORD * )Image ;
	for( i = PackNum ; i ; i --, Src += 8 )
	{
		CmpInfo->Data[ 0 ] += Src[ 0 ] ;
		CmpInfo->Data[ 1 ] += Src[ 1 ] ;
		CmpInfo->Data[ 2 ] += Src[ 2 ] ;
		CmpInfo->Data[ 3 ] += Src[ 3 ] ;
		CmpInfo->Data[ 4 ] += Src[ 4 ] ;
		CmpInfo->Data[ 5 ] += Src[ 5 ] ;
		CmpInfo->Data[ 6 ] += Src[ 6 ] ;
		CmpInfo->Data[ 7 ] += Src[ 7 ] ;
	}

	// ３２バイトの余りの部分を追加
	AmariNum = Size - PackNum * 32 ;
	for( i = 0 ; i < AmariNum ; i ++ )
	{
		( ( BYTE * )CmpInfo->Data )[ i ] += ( ( BYTE * )Src )[ i ] ;
	}
}

// 二つのチェックデータが一致しているかどうかを取得する( 1:一致している  0:一致していない )
extern int CheckDataCmpInfo( DATACMPINFO *CmpInfo1, DATACMPINFO *CmpInfo2 )
{
	return CmpInfo1->Size != CmpInfo2->Size ||
			CmpInfo1->Data[ 0 ] != CmpInfo2->Data[ 0 ] ||
			CmpInfo1->Data[ 1 ] != CmpInfo2->Data[ 1 ] ||
			CmpInfo1->Data[ 2 ] != CmpInfo2->Data[ 2 ] ||
			CmpInfo1->Data[ 3 ] != CmpInfo2->Data[ 3 ] ||
			CmpInfo1->Data[ 4 ] != CmpInfo2->Data[ 4 ] ||
			CmpInfo1->Data[ 5 ] != CmpInfo2->Data[ 5 ] ||
			CmpInfo1->Data[ 6 ] != CmpInfo2->Data[ 6 ] ||
			CmpInfo1->Data[ 7 ] != CmpInfo2->Data[ 7 ] ? 0 : 1 ;
}










// 更新検出情報構築用関数

// 状態変化管理用構造体のセットアップを行う
extern void MV1ChangeInfoSetup( MV1_CHANGE_BASE *ChangeB, void *FillTable, int BitAddress, int FillBitNum )
{
	DWORD *Dest ;
	DWORD Bit, i ;

	ChangeB->Target   = ( DWORD )( BitAddress / 32 ) ;
	ChangeB->Fill     = ( DWORD * )FillTable ;
	ChangeB->CheckBit = ( DWORD )( 1 << ( BitAddress % 32 ) ) ;
	ChangeB->Size     = ( DWORD )( ( BitAddress % 32 + FillBitNum + 31 ) / 32 ) ;

	if( ChangeB->Fill )
	{
		Bit = ChangeB->CheckBit ;
		Dest = ChangeB->Fill ;
		*Dest = 0 ;
		for( i = 0 ; i < ( DWORD )FillBitNum ; i ++ )
		{
			*Dest |= Bit ;
			Bit <<= 1 ;
			if( Bit == 0 )
			{
				Dest ++ ;
				Bit = 1 ;
			}
		}
	}
}





// モデル読み込み補助系

// MV1_MAKEVERTINDEXINFO を利用した高速同一データ検索を補助する関数
extern int AddVertexInfo( MV1_MAKEVERTINDEXINFO **InfoTable, MV1_MAKEVERTINDEXINFO *InfoBuffer, int *InfoNum, void *DataBuffer, int DataUnitSize, int DataNum, int AddDataIndex, void *AddData )
{
	MV1_MAKEVERTINDEXINFO *IInfo ;

	for( IInfo = InfoTable[ AddDataIndex ] ; IInfo ; IInfo = IInfo->Next )
	{
		if( _MEMCMP( ( BYTE * )DataBuffer + IInfo->VertexIndex * DataUnitSize, AddData, DataUnitSize ) == 0 )
			return IInfo->VertexIndex ;
	}

	IInfo = &InfoBuffer[ *InfoNum ] ;
	( *InfoNum ) ++ ;

	IInfo->VertexIndex = DataNum ;
	IInfo->Next = InfoTable[ AddDataIndex ] ;
	InfoTable[ AddDataIndex ] = IInfo ;

	return -1 ;
}


// メッシュの従法線と接線を計算する
extern int MV1MakeMeshBinormalsAndTangents( MV1_MESH_BASE *Mesh )
{
	int i, VertUnitSize, PosUnitSize, NormalNum ;
	MV1_MESH_FACE *Face ;
	MV1_MESH_NORMAL *Normal, *Norm[ 3 ], *NM ;
	MV1_MESH_POSITION *Position, *Pos[ 3 ] ;
	MV1_MESH_VERTEX *Vertex, *Vert[ 3 ], *VT ;
	VECTOR v1, v2, vt, du, dv, vb, vn ;
	BYTE *UseFlag ;

	// 補助情報の準備
	Normal = Mesh->Container->Normal ;
	NormalNum = Mesh->Container->NormalNum ;
	Position = Mesh->Container->Position ;
	Vertex = Mesh->Vertex ;
	VertUnitSize = Mesh->VertUnitSize ;
	PosUnitSize = Mesh->Container->PosUnitSize ;

	// 使用している頂点のテーブルを作成する
	{
		UseFlag = ( BYTE * )DXALLOC( sizeof( BYTE ) * NormalNum ) ;
		if( UseFlag == NULL )
		{
			DXST_ERRORLOGFMT_ADDUTF16LE(( "\x02\x98\xb9\x70\x6e\x30\xa5\x63\xda\x7d\x68\x30\x93\x5f\xd5\x6c\xda\x7d\x6e\x30\x5c\x4f\x10\x62\x5c\x4f\x6d\x69\x6b\x30\xc5\x5f\x81\x89\x6a\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"頂点の接線と従法線の作成作業に必要なメモリ領域の確保に失敗しました\n" @*/ )) ;
			return -1 ;
		}
		_MEMSET( UseFlag, 0, ( size_t )Mesh->Container->NormalNum ) ;

		VT = Vertex ;
		for( i = 0 ; i < Mesh->VertexNum ; i ++, VT = ( MV1_MESH_VERTEX * )( ( BYTE * )VT + VertUnitSize ) )
		{
			UseFlag[ VT->NormalIndex ] = 1 ;
		}
	}

	// 接線と従法線の初期化
	NM = Normal ;
	for( i = 0 ; i < NormalNum ; i ++, NM ++ )
	{
		if( UseFlag[ i ] == 0 ) continue ;
		NM->Binormal.x = 0.0f ;
		NM->Binormal.y = 0.0f ;
		NM->Binormal.z = 0.0f ;
		NM->Tangent.x = 0.0f ;
		NM->Tangent.y = 0.0f ;
		NM->Tangent.z = 0.0f ;
	}

	// 全ての面の数だけ繰り返し
	Face = Mesh->Face ;
	for( i = 0 ; i < Mesh->FaceNum ; i ++, Face ++ )
	{
		Vert[ 0 ] = ( MV1_MESH_VERTEX   * )( ( BYTE * )Vertex   + VertUnitSize * Face->VertexIndex[ 0 ] ) ;
		Vert[ 1 ] = ( MV1_MESH_VERTEX   * )( ( BYTE * )Vertex   + VertUnitSize * Face->VertexIndex[ 1 ] ) ;
		Vert[ 2 ] = ( MV1_MESH_VERTEX   * )( ( BYTE * )Vertex   + VertUnitSize * Face->VertexIndex[ 2 ] ) ;
		Pos[ 0 ]  = ( MV1_MESH_POSITION * )( ( BYTE * )Position + PosUnitSize  * Vert[ 0 ]->PositionIndex ) ;
		Pos[ 1 ]  = ( MV1_MESH_POSITION * )( ( BYTE * )Position + PosUnitSize  * Vert[ 1 ]->PositionIndex ) ;
		Pos[ 2 ]  = ( MV1_MESH_POSITION * )( ( BYTE * )Position + PosUnitSize  * Vert[ 2 ]->PositionIndex ) ;
		Norm[ 0 ] = Normal + Vert[ 0 ]->NormalIndex ;
		Norm[ 1 ] = Normal + Vert[ 1 ]->NormalIndex ;
		Norm[ 2 ] = Normal + Vert[ 2 ]->NormalIndex ;

		v1.x = Pos[ 1 ]->Position.x - Pos[ 0 ]->Position.x ;
		v1.y = Vert[ 1 ]->UVs[ 0 ][ 0 ] - Vert[ 0 ]->UVs[ 0 ][ 0 ] ;
		v1.z = Vert[ 1 ]->UVs[ 0 ][ 1 ] - Vert[ 0 ]->UVs[ 0 ][ 1 ] ;

		v2.x = Pos[ 2 ]->Position.x - Pos[ 0 ]->Position.x ;
		v2.y = Vert[ 2 ]->UVs[ 0 ][ 0 ] - Vert[ 0 ]->UVs[ 0 ][ 0 ] ;
		v2.z = Vert[ 2 ]->UVs[ 0 ][ 1 ] - Vert[ 0 ]->UVs[ 0 ][ 1 ] ;

		vt = VCross( v1, v2 ) ;
		du.x = 1.0f ;
		if( VDot( vt, vt ) >= 0.0000001f )
		{
			du.x = -vt.y / vt.x ;
			dv.x = -vt.z / vt.x ;
		}

		v1.x = Pos[ 1 ]->Position.y - Pos[ 0 ]->Position.y ;
		v2.x = Pos[ 2 ]->Position.y - Pos[ 0 ]->Position.y ;

		vt = VCross( v1, v2 ) ;
		du.y = 1.0f ;
		if( VDot( vt, vt ) >= 0.0000001f )
		{
			du.y = -vt.y / vt.x ;
			dv.y = -vt.z / vt.x ;
		}

		v1.x = Pos[ 1 ]->Position.z - Pos[ 0 ]->Position.z ;
		v2.x = Pos[ 2 ]->Position.z - Pos[ 0 ]->Position.z ;

		vt = VCross( v1, v2 ) ;
		du.z = 1.0f ;
		if( VDot( vt, vt ) >= 0.0000001f )
		{
			du.z = -vt.y / vt.x ;
			dv.z = -vt.z / vt.x ;
		}

		VectorAdd( &Norm[ 0 ]->Tangent, &Norm[ 0 ]->Tangent, &du ) ;
		VectorAdd( &Norm[ 1 ]->Tangent, &Norm[ 1 ]->Tangent, &du ) ;
		VectorAdd( &Norm[ 2 ]->Tangent, &Norm[ 2 ]->Tangent, &du ) ;

		VectorAdd( &Norm[ 0 ]->Binormal, &Norm[ 0 ]->Binormal, &dv ) ;
		VectorAdd( &Norm[ 1 ]->Binormal, &Norm[ 1 ]->Binormal, &dv ) ;
		VectorAdd( &Norm[ 2 ]->Binormal, &Norm[ 2 ]->Binormal, &dv ) ;
	}

	// 法線の算出と正規化
	NM = Normal ;
	for( i = 0 ; i < NormalNum ; i ++, NM ++ )
	{
		if( UseFlag[ i ] == 0 ) continue ;

		vt = VNorm( NM->Tangent ) ;
		vn = VNorm( VCross( vt, NM->Binormal ) ) ;
		vb = VNorm( VCross( vn, vt ) ) ;

		// 法線の方向を確認
		if( VDot( vn, NM->Normal ) < 0.0f )
		{
			vn.x = -vn.x ;
			vn.y = -vn.y ;
			vn.z = -vn.z ;
		}

		// 正規化
		NM->Tangent  = vt ;
		NM->Binormal = vb ;
//		NM->Normal   = vn ;
	}

	// メモリの解放
	DXFREE( UseFlag ) ;

	// 終了
	return 0 ;
}

// トライアングルリストの座標と法線情報のセットアップを行う
extern int MV1SetupTriangleListPositionAndNormal( MV1_TRIANGLE_LIST_BASE *List )
{
	int i, j, k ;
	MV1_TLIST_NORMAL_POS *NrmPos ;
	MV1_TLIST_SKIN_POS_4B *SK4BPos ;
	MV1_TLIST_SKIN_POS_8B *SK8BPos ;
	MV1_TLIST_SKIN_POS_FREEB *SKFBPos ;
	DWORD *MInd ;
	MV1_MESH_VERTEX *Vert ;
	MV1_MESH_POSITION *Pos ;
	MV1_MESH_NORMAL *Nrm ;
	MV1_SKINBONE_BLEND *VBlend ;
	MV1_MESH_BASE *Mesh ;
	MV1_FRAME_BASE *Frame ;

	Mesh = List->Container ;
	Frame = Mesh->Container ;

	MInd = List->MeshVertexIndex ;
	switch( List->VertexType )
	{
	case MV1_VERTEX_TYPE_NORMAL :
		NrmPos = ( MV1_TLIST_NORMAL_POS * )ADDR16( List->NormalPosition ) ;
		for( i = 0 ; i < List->VertexNum ; i ++, NrmPos ++, MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + *MInd * Mesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Vert->PositionIndex * Frame->PosUnitSize ) ;
			Nrm = Frame->Normal + Vert->NormalIndex ;

			*( ( VECTOR * )&NrmPos->Position ) = Pos->Position ;
			NrmPos->Position.w = 1.0f ;
			*( ( VECTOR * )&NrmPos->Normal ) = Nrm->Normal ;
			NrmPos->Normal.w = 0.0f ;
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_4BONE :
		SK4BPos = ( MV1_TLIST_SKIN_POS_4B * )ADDR16( List->SkinPosition4B ) ;
		for( i = 0 ; i < List->VertexNum ; i ++, SK4BPos ++, MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + *MInd * Mesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Vert->PositionIndex * Frame->PosUnitSize ) ;
			Nrm = Frame->Normal + Vert->NormalIndex ;

			*( ( VECTOR * )&SK4BPos->Position ) = Pos->Position ;
			SK4BPos->Position.w = 1.0f ;
			SK4BPos->Normal = Nrm->Normal ;

			VBlend = Pos->BoneWeight ;
			for( j = 0 ; j < Frame->UseSkinBoneNum && j < 4 && VBlend->Index != -1 ; j ++, VBlend ++ )
			{
				SK4BPos->MatrixWeight[ j ] = VBlend->W ;
				for( k = 0 ; k < List->UseBoneNum && List->UseBone[ k ] != VBlend->Index ; k ++ ){}
				SK4BPos->MatrixIndex[ j ] = ( BYTE )k ;
			}
			for( ; j < 4 ; j ++ )
			{
				SK4BPos->MatrixWeight[ j ] = 0.0f ;
				SK4BPos->MatrixIndex[ j ] = 0 ;
			}
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_8BONE :
		SK8BPos = ( MV1_TLIST_SKIN_POS_8B * )ADDR16( List->SkinPosition8B ) ;
		for( i = 0 ; i < List->VertexNum ; i ++, SK8BPos ++, MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + *MInd * Mesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Vert->PositionIndex * Frame->PosUnitSize ) ;
			Nrm = Frame->Normal + Vert->NormalIndex ;

			SK8BPos->Position = Pos->Position ;
			SK8BPos->Normal = Nrm->Normal ;

			VBlend = Pos->BoneWeight ;
			for( j = 0 ; j < Frame->UseSkinBoneNum && j < 8 && VBlend->Index != -1 ; j ++, VBlend ++ )
			{
				SK8BPos->MatrixWeight[ j ] = VBlend->W ;

				for( k = 0 ; k < List->UseBoneNum && List->UseBone[ k ] != VBlend->Index ; k ++ ){}
				if( j < 4 )
				{
					SK8BPos->MatrixIndex1[ j ] = ( BYTE )k ;
				}
				else
				{
					SK8BPos->MatrixIndex2[ j - 4 ] = ( BYTE )k ;
				}
			}
			for( ; j < 8 ; j ++ )
			{
				SK8BPos->MatrixWeight[ j ] = 0.0f ;
				if( j < 4 )
				{
					SK8BPos->MatrixIndex1[ j ] = 0 ;
				}
				else
				{
					SK8BPos->MatrixIndex2[ j - 4 ] = 0 ;
				}
			}
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_FREEBONE :
		SKFBPos = ( MV1_TLIST_SKIN_POS_FREEB * )ADDR16( List->SkinPositionFREEB ) ;
		for( i = 0 ; i < List->VertexNum ; i ++, SKFBPos = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )SKFBPos + List->PosUnitSize ), MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + *MInd * Mesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Vert->PositionIndex * Frame->PosUnitSize ) ;
			Nrm = Frame->Normal + Vert->NormalIndex ;

			*( ( VECTOR * )&SKFBPos->Position ) = Pos->Position ;
			SKFBPos->Position.w = 1.0f ;
			*( ( VECTOR * )&SKFBPos->Normal ) = Nrm->Normal ;
			SKFBPos->Normal.w = 0.0f ;

			VBlend = Pos->BoneWeight ;
			for( j = 0 ; j < List->MaxBoneNum && VBlend->Index != -1 ; j ++, VBlend ++ )
			{
				SKFBPos->MatrixWeight[ j ].Index = VBlend->Index ;
				SKFBPos->MatrixWeight[ j ].W     = VBlend->W ;
			}
			for( ; j < List->MaxBoneNum ; j ++ )
			{
				SKFBPos->MatrixWeight[ j ].Index = -1 ;
				SKFBPos->MatrixWeight[ j ].W = 0.0f ;
			}
		}
		break ;
	}

	// 終了
	return 0 ;
}

// シェイプ用トライアングルリストの座標と法線情報のセットアップを行う
extern int MV1SetupShapeTriangleListPositionAndNormal( MV1_TRIANGLE_LIST *TList )
{
	int i, j, k ;
	MV1_TLIST_NORMAL_POS *NrmPos ;
	MV1_TLIST_SKIN_POS_4B *SK4BPos ;
	MV1_TLIST_SKIN_POS_8B *SK8BPos ;
	MV1_TLIST_SKIN_POS_FREEB *SKFBPos ;
	DWORD *MInd ;
	MV1_MESH_VERTEX *Vert ;
	MV1_MESH_POSITION *Pos ;
	MV1_MESH_NORMAL *Nrm ;
	MV1_SKINBONE_BLEND *VBlend ;
	MV1_MESH_BASE *MBMesh ;
	MV1_FRAME_BASE *MBFrame ;
	MV1_TRIANGLE_LIST_BASE *MBTList ;

	MBTList = TList->BaseData ;
	MBMesh = MBTList->Container ;
	MBFrame = MBMesh->Container ;

	MInd = MBTList->MeshVertexIndex ;
	switch( MBTList->VertexType )
	{
	case MV1_VERTEX_TYPE_NORMAL :
		NrmPos = ( MV1_TLIST_NORMAL_POS * )ADDR16( TList->NormalPosition ) ;
		for( i = 0 ; i < MBTList->VertexNum ; i ++, NrmPos ++, MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )MBMesh->Vertex + *MInd * MBMesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )MBFrame->Position + Vert->PositionIndex * MBFrame->PosUnitSize ) ;
			Nrm = MBFrame->Normal + Vert->NormalIndex ;

			*( ( VECTOR * )&NrmPos->Position ) = Pos->Position ;
			NrmPos->Position.w = 1.0f ;
			*( ( VECTOR * )&NrmPos->Normal ) = Nrm->Normal ;
			NrmPos->Normal.w = 0.0f ;
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_4BONE :
		SK4BPos = ( MV1_TLIST_SKIN_POS_4B * )ADDR16( TList->SkinPosition4B ) ;
		for( i = 0 ; i < MBTList->VertexNum ; i ++, SK4BPos ++, MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )MBMesh->Vertex + *MInd * MBMesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )MBFrame->Position + Vert->PositionIndex * MBFrame->PosUnitSize ) ;
			Nrm = MBFrame->Normal + Vert->NormalIndex ;

			*( ( VECTOR * )&SK4BPos->Position ) = Pos->Position ;
			SK4BPos->Position.w = 1.0f ;
			SK4BPos->Normal = Nrm->Normal ;

			VBlend = Pos->BoneWeight ;
			for( j = 0 ; j < MBFrame->UseSkinBoneNum && j < 4 && VBlend->Index != -1 ; j ++, VBlend ++ )
			{
				SK4BPos->MatrixWeight[ j ] = VBlend->W ;
				for( k = 0 ; k < MBTList->UseBoneNum && MBTList->UseBone[ k ] != VBlend->Index ; k ++ ){}
				SK4BPos->MatrixIndex[ j ] = ( BYTE )k ;
			}
			for( ; j < 4 ; j ++ )
			{
				SK4BPos->MatrixWeight[ j ] = 0.0f ;
				SK4BPos->MatrixIndex[ j ] = 0 ;
			}
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_8BONE :
		SK8BPos = ( MV1_TLIST_SKIN_POS_8B * )ADDR16( TList->SkinPosition8B ) ;
		for( i = 0 ; i < MBTList->VertexNum ; i ++, SK8BPos ++, MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )MBMesh->Vertex + *MInd * MBMesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )MBFrame->Position + Vert->PositionIndex * MBFrame->PosUnitSize ) ;
			Nrm = MBFrame->Normal + Vert->NormalIndex ;

			SK8BPos->Position = Pos->Position ;
			SK8BPos->Normal = Nrm->Normal ;

			VBlend = Pos->BoneWeight ;
			for( j = 0 ; j < MBFrame->UseSkinBoneNum && j < 8 && VBlend->Index != -1 ; j ++, VBlend ++ )
			{
				SK8BPos->MatrixWeight[ j ] = VBlend->W ;

				for( k = 0 ; k < MBTList->UseBoneNum && MBTList->UseBone[ k ] != VBlend->Index ; k ++ ){}
				if( j < 4 )
				{
					SK8BPos->MatrixIndex1[ j ] = ( BYTE )k ;
				}
				else
				{
					SK8BPos->MatrixIndex2[ j - 4 ] = ( BYTE )k ;
				}
			}
			for( ; j < 8 ; j ++ )
			{
				SK8BPos->MatrixWeight[ j ] = 0.0f ;
				if( j < 4 )
				{
					SK8BPos->MatrixIndex1[ j ] = 0 ;
				}
				else
				{
					SK8BPos->MatrixIndex2[ j - 4 ] = 0 ;
				}
			}
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_FREEBONE :
		SKFBPos =( MV1_TLIST_SKIN_POS_FREEB * )ADDR16( TList->SkinPositionFREEB ) ;
		for( i = 0 ; i < MBTList->VertexNum ; i ++, SKFBPos = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )SKFBPos + MBTList->PosUnitSize ), MInd ++ )
		{
			Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )MBMesh->Vertex + *MInd * MBMesh->VertUnitSize ) ;
			Pos = ( MV1_MESH_POSITION * )( ( BYTE * )MBFrame->Position + Vert->PositionIndex * MBFrame->PosUnitSize ) ;
			Nrm = MBFrame->Normal + Vert->NormalIndex ;

			*( ( VECTOR * )&SKFBPos->Position ) = Pos->Position ;
			SKFBPos->Position.w = 1.0f ;
			*( ( VECTOR * )&SKFBPos->Normal ) = Nrm->Normal ;
			SKFBPos->Normal.w = 0.0f ;

			VBlend = Pos->BoneWeight ;
			for( j = 0 ; j < MBTList->MaxBoneNum && VBlend->Index != -1 ; j ++, VBlend ++ )
			{
				SKFBPos->MatrixWeight[ j ].Index = VBlend->Index ;
				SKFBPos->MatrixWeight[ j ].W     = VBlend->W ;
			}
			for( ; j < MBTList->MaxBoneNum ; j ++ )
			{
				SKFBPos->MatrixWeight[ j ].Index = -1 ;
				SKFBPos->MatrixWeight[ j ].W = 0.0f ;
			}
		}
		break ;
	}

	// 終了
	return 0 ;
}


















// 共通データ系

// グラデーション画像を作成する
extern int MV1CreateGradationGraph( void )
{
	DWORD i ;
	BASEIMAGE BaseImage ;
	DWORD *Dest, DestData ;
	int Handle ;
	LOADGRAPH_GPARAM GParam ;

	if( NS_CreateXRGB8ColorBaseImage( 256, 8, &BaseImage ) < 0 )
		return -1 ;

	Dest = ( DWORD * )BaseImage.GraphData ;
	for( i = 0 ; i < 256 ; i ++, Dest ++ )
	{
		DestData = i | ( i << 8 ) | ( i << 16 ) ;
		Dest[ 256 * 0 ] = DestData ;
		Dest[ 256 * 1 ] = DestData ;
		Dest[ 256 * 2 ] = DestData ;
		Dest[ 256 * 3 ] = DestData ;
		Dest[ 256 * 4 ] = DestData ;
		Dest[ 256 * 5 ] = DestData ;
		Dest[ 256 * 6 ] = DestData ;
		Dest[ 256 * 7 ] = DestData ;
	}

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;
	GParam.CreateGraphGParam.NotUseTransColor = TRUE ;
	Graphics_Image_InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam.CreateGraphGParam.InitHandleGParam, 32, TRUE, FALSE ) ;
	Handle = Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, FALSE, -1, &BaseImage, NULL, TRUE, FALSE, FALSE, FALSE ) ;

	NS_ReleaseBaseImage( &BaseImage ) ;

	return Handle ;
}

// デフォルトトゥーンテクスチャを取得する
extern int MV1GetDefaultToonTexture( int Type )
{
	switch( Type )
	{
	case -1 :
		if( MV1Man.ToonDefaultGradTexHandle[ 0 ] < 0 )
		{
			LOADGRAPH_GPARAM GParam ;

			Graphics_Image_InitLoadGraphGParam( &GParam ) ;
			GParam.CreateGraphGParam.NotUseTransColor = TRUE ;
			Graphics_Image_InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam.CreateGraphGParam.InitHandleGParam, 32, TRUE, FALSE ) ;
			MV1Man.ToonDefaultGradTexHandle[ 0 ] = Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, FALSE, -1, TgaDiffuseDefaultGradFileImage, sizeof( TgaDiffuseDefaultGradFileImage ), NULL, 0, TRUE, FALSE, FALSE ) ;
			NS_SetDeleteHandleFlag( MV1Man.ToonDefaultGradTexHandle[ 0 ], &MV1Man.ToonDefaultGradTexHandle[ 0 ] ) ;
			NS_SetDeviceLostDeleteGraphFlag( MV1Man.ToonDefaultGradTexHandle[ 0 ], TRUE ) ;
		}
		return MV1Man.ToonDefaultGradTexHandle[ 0 ] ;

	case -2 :
		if( MV1Man.ToonDefaultGradTexHandle[ 1 ] < 0 )
		{
			MV1Man.ToonDefaultGradTexHandle[ 1 ] = MV1CreateGradationGraph() ;
			NS_SetDeleteHandleFlag( MV1Man.ToonDefaultGradTexHandle[ 1 ], &MV1Man.ToonDefaultGradTexHandle[ 1 ] ) ;
			NS_SetDeviceLostDeleteGraphFlag( MV1Man.ToonDefaultGradTexHandle[ 1 ], TRUE ) ;
		}
		return MV1Man.ToonDefaultGradTexHandle[ 1 ] ;
	}

	return -1 ;
}

// TexNoneHandle のセットアップを行う
extern void MV1SetupTexNoneHandle( void )
{
	LOADGRAPH_GPARAM GParam ;
	Graphics_Image_InitLoadGraphGParam( &GParam ) ;
	GParam.CreateGraphGParam.NotUseTransColor            = FALSE ;
	GParam.CreateGraphGParam.LeftUpColorIsTransColorFlag = FALSE ;
	Graphics_Image_InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam.CreateGraphGParam.InitHandleGParam, 32, TRUE, FALSE ) ;
	GParam.CreateGraphGParam.InitHandleGParam.TransColor = ( BYTE )255 << 16 | ( BYTE )0 << 8 | ( BYTE )255 ;
	MV1Man.TexNoneHandle      = Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, FALSE, -1, Tga8x8TextureFileImage,      sizeof( Tga8x8TextureFileImage      ), NULL, 0, TRUE, FALSE, FALSE ) ;
	MV1Man.TexNoneBlackHandle = Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, FALSE, -1, Tga8x8BlackTextureFileImage, sizeof( Tga8x8BlackTextureFileImage ), NULL, 0, TRUE, FALSE, FALSE ) ;
	NS_SetDeleteHandleFlag( MV1Man.TexNoneHandle,      &MV1Man.TexNoneHandle      ) ;
	NS_SetDeleteHandleFlag( MV1Man.TexNoneBlackHandle, &MV1Man.TexNoneBlackHandle ) ;
	NS_SetDeviceLostDeleteGraphFlag( MV1Man.TexNoneHandle,      TRUE ) ;
	NS_SetDeviceLostDeleteGraphFlag( MV1Man.TexNoneBlackHandle, TRUE ) ;
}



















// モデル機能の初期化
extern int MV1Initialize()
{
	int i ;
	float Sin, Cos ;

	// 既に初期化されていたら何もせずに終了
	if( MV1Man.Initialize ) return 0 ;

	// ハードウエアの機能を使用できない場合は何もせず終了
	if( GSYS.Setting.ValidHardware == FALSE ) return 0 ;

	// トライアングルリスト基データの環境依存用バッファのサイズチェック
	if( MV1_TRIANGLE_LIST_BASE_PF_BUFFER_SIZE < sizeof( MV1_TRIANGLE_LIST_BASE_PF ) )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xc8\x30\xe9\x30\xa4\x30\xa2\x30\xf3\x30\xb0\x30\xeb\x30\xea\x30\xb9\x30\xc8\x30\xfa\x57\xc7\x30\xfc\x30\xbf\x30\x6e\x30\xb0\x74\x83\x58\x9d\x4f\x58\x5b\x28\x75\xd0\x30\xc3\x30\xd5\x30\xa1\x30\x6e\x30\xb5\x30\xa4\x30\xba\x30\x4c\x30\xb3\x8d\x8a\x30\x7e\x30\x5b\x30\x93\x30\x20\x00\x4d\x00\x56\x00\x31\x00\x5f\x00\x54\x00\x52\x00\x49\x00\x41\x00\x4e\x00\x47\x00\x4c\x00\x45\x00\x5f\x00\x4c\x00\x49\x00\x53\x00\x54\x00\x5f\x00\x42\x00\x41\x00\x53\x00\x45\x00\x5f\x00\x50\x00\x46\x00\x5f\x00\x42\x00\x55\x00\x46\x00\x46\x00\x45\x00\x52\x00\x5f\x00\x53\x00\x49\x00\x5a\x00\x45\x00\x3a\x00\x25\x00\x64\x00\x62\x00\x79\x00\x74\x00\x65\x00\x20\x00\x20\x00\x73\x00\x69\x00\x7a\x00\x65\x00\x6f\x00\x66\x00\x28\x00\x20\x00\x4d\x00\x56\x00\x31\x00\x5f\x00\x54\x00\x52\x00\x49\x00\x41\x00\x4e\x00\x47\x00\x4c\x00\x45\x00\x5f\x00\x4c\x00\x49\x00\x53\x00\x54\x00\x5f\x00\x42\x00\x41\x00\x53\x00\x45\x00\x5f\x00\x50\x00\x46\x00\x20\x00\x29\x00\x3a\x00\x25\x00\x64\x00\x62\x00\x79\x00\x74\x00\x65\x00\x00"/*@ L"トライアングルリスト基データの環境依存用バッファのサイズが足りません MV1_TRIANGLE_LIST_BASE_PF_BUFFER_SIZE:%dbyte  sizeof( MV1_TRIANGLE_LIST_BASE_PF ):%dbyte" @*/,
			MV1_TRIANGLE_LIST_BASE_PF_BUFFER_SIZE, sizeof( MV1_TRIANGLE_LIST_BASE_PF ) ));
		return -1 ;
	}

	// モデル基本データハンドルの初期化
	InitializeHandleManage( DX_HANDLETYPE_MODEL_BASE, sizeof( MV1_MODEL_BASE ), MAX_MODEL_BASE_NUM, InitializeModelBaseHandle, TerminateModelBaseHandle, L"ModelBase" ) ;

	// モデルデータハンドルの初期化
	InitializeHandleManage( DX_HANDLETYPE_MODEL, sizeof( MV1_MODEL ), MAX_MODEL_NUM, InitializeModelHandle, TerminateModelHandle, L"Model" ) ;

	// サインテーブルの初期化
	for( i = 0 ; i < MV1_SINTABLE_DIV ; i ++ )
	{
		_SINCOS( i * ( float )DX_PI * 2 / MV1_SINTABLE_DIV, &Sin, &Cos ) ;
		MV1Man.SinTable[ i ] = Sin ;
	}

	// 各種変数を初期化
	MV1Man.ModelBaseNum = 0 ;
	MV1Man.ModelBaseMaxNum = 0 ;
	MV1Man.ModelBaseNextIndex = 0 ;
	MV1Man.ModelBase = 0 ;

	MV1Man.ModelNum = 0 ;
	MV1Man.ModelMaxNum = 0 ;
	MV1Man.ModelNextIndex = 0 ;
	MV1Man.ModelNextCheckNumber = 0 ;
	MV1Man.Model = 0 ;

//	MV1Man.TextureNum = 0 ;
//	MV1Man.TextureMaxNum = 0 ;
//	MV1Man.TextureNextIndex = 0 ;
//	MV1Man.Texture = 0 ;

	MV1Man.LoadModelToReMakeNormalSmoothingAngle = 89.5f * ( float )DX_PI / 180.0f ;

	// デフォルトテクスチャのハンドル値を無効化
	MV1Man.TexNoneHandle = -1 ;
	MV1Man.ToonDefaultGradTexHandle[ 0 ] = -1 ;
	MV1Man.ToonDefaultGradTexHandle[ 1 ] = -1 ;

	// 初期化フラグを立てる
	MV1Man.Initialize = true ;

	// 終了
	return 0 ;
}

// モデル機能の後始末
extern int MV1Terminate()
{
	// 初期化されていなかったら何もせずに終了
	if( MV1Man.Initialize == false ) return 0 ;

	// 有効なすべてのモデルを解放する
	NS_MV1InitModel() ;

	// 有効なすべてのモデル基データを解放する
	MV1InitModelBase() ;

	// 描画用に確保されたメモリ領域の確保
	if( MV1Man.DrawMeshList )
	{
		DXFREE( MV1Man.DrawMeshList ) ;
		MV1Man.DrawMeshList = NULL ;
		MV1Man.DrawMeshListSize = 0 ;
	}

	// 環境依存の後始末処理を実行
	MV1_Terminate_PF() ;

	// モデルデータハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_MODEL ) ;

	// モデル基本データハンドル管理情報の後始末
	TerminateHandleManage( DX_HANDLETYPE_MODEL_BASE ) ;

	// 初期化フラグを倒す
	MV1Man.Initialize = false ;

	// 終了
	return 0 ;
}


// モデル用テクスチャのカラーイメージを作成する
extern int MV1CreateTextureColorBaseImage(
				BASEIMAGE *DestColorBaseImage,
				BASEIMAGE *DestAlphaBaseImage,
				void *ColorFileImage, int ColorFileSize,
				void *AlphaFileImage, int AlphaFileSize,
				int BumpImageFlag, float BumpImageNextPixelLength,
				int ReverseFlag )
{
	BASEIMAGE ScaleBaseImage, ScaleAlphaBaseImage ;
	int SizeX, SizeY ;

	// ファイルが読み込めなかったらエラー
	if( ColorFileImage )
	{
		if( NS_CreateBaseImage( NULL, ColorFileImage, ColorFileSize, LOADIMAGE_TYPE_MEM, DestColorBaseImage, FALSE ) < 0 )
		{
			DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 1\n" )) ;
			return -1 ;
		}
	}

	if( AlphaFileImage )
	{
		if( NS_CreateBaseImage( NULL, AlphaFileImage, AlphaFileSize, LOADIMAGE_TYPE_MEM, DestAlphaBaseImage, FALSE ) < 0 )
		{
			if( ColorFileImage ) NS_ReleaseBaseImage( DestColorBaseImage ) ;
			DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 2\n" )) ;
			return -1 ;
		}
	}

	// カラーテクスチャが無い場合は代替画像を充てる
	if( ColorFileImage == NULL )
	{
		if( NS_CreateXRGB8ColorBaseImage( DestAlphaBaseImage->Width, DestAlphaBaseImage->Height, DestColorBaseImage ) < 0 )
		{
			if( AlphaFileImage ) NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
			DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 3\n" )) ;
			return -1 ;
		}
		_MEMSET( DestColorBaseImage->GraphData, 0xff, ( size_t )( DestColorBaseImage->Pitch * DestColorBaseImage->Height ) ) ;
	}

	// バンプマップ指定の場合は法線マップ化する
	if( BumpImageFlag )
	{
		int c00, c01, c10, i, j ;
		VECTOR v1, v2, v ;
		BASEIMAGE NormalBaseImage ;

		if( BumpImageNextPixelLength < 0.000001f )
			BumpImageNextPixelLength = 0.000001f ;

		if( NS_CreateXRGB8ColorBaseImage( DestColorBaseImage->Width, DestColorBaseImage->Height, &NormalBaseImage ) < 0 )
		{
			NS_ReleaseBaseImage( DestColorBaseImage ) ;
			if( AlphaFileImage ) NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
			DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 4\n" )) ;
			return -1 ;
		}

		for( i = 0 ; i < DestColorBaseImage->Height ; i ++ )
		{
			for( j = 0 ; j < DestColorBaseImage->Width ; j ++ )
			{
				NS_GetPixelBaseImage( DestColorBaseImage, j, i, &c00, NULL, NULL, NULL ) ;
				NS_GetPixelBaseImage( DestColorBaseImage, j + 1 == DestColorBaseImage->Width ? 0 : j + 1, i,  &c01, NULL, NULL, NULL ) ;
				NS_GetPixelBaseImage( DestColorBaseImage, j, i + 1 == DestColorBaseImage->Height ? 0 : i + 1, &c10, NULL, NULL, NULL ) ;

				v1 = VGet( BumpImageNextPixelLength, 0.0f, ( c01 - c00 ) / 255.0f ) ;
				v2 = VGet( 0.0f, BumpImageNextPixelLength, ( c10 - c00 ) / 255.0f ) ;
				v = VNorm( VCross( v1, v2 ) ) ;
				NS_SetPixelBaseImage( &NormalBaseImage, j, i, _FTOL( ( v.x + 1.0f ) * 127.5f ), _FTOL( ( v.y + 1.0f ) * 127.5f ), _FTOL( ( v.z + 1.0f ) * 127.5f ), 0 ) ;
			}
		}
//		NS_SaveBaseImageToBmp( _T( "TestNormalMap.bmp" ), &NormalBaseImage ) ; 

		// カラー画像を解放
		NS_ReleaseBaseImage( DestColorBaseImage ) ;

		// 法線マップ画像をカラー画像とする
		*DestColorBaseImage = NormalBaseImage ;
	}

	// カラー画像のサイズが２のｎ乗ではなかったらスケーリングする
	if( ColorFileImage != NULL )
	{
		for( SizeX = 1 ; SizeX < DestColorBaseImage->Width  ; SizeX <<= 1 ){}
		for( SizeY = 1 ; SizeY < DestColorBaseImage->Height ; SizeY <<= 1 ){}
		if( SizeX != DestColorBaseImage->Width || SizeY != DestColorBaseImage->Height )
		{
			// ハードウエアが対応している最大テクスチャサイズ / 2は超えないようにする
			if( GSYS.HardInfo.MaxTextureSize / 2 < SizeX ) SizeX = GSYS.HardInfo.MaxTextureSize / 2 ;
			if( GSYS.HardInfo.MaxTextureSize / 2 < SizeY ) SizeY = GSYS.HardInfo.MaxTextureSize / 2 ;

			if( NS_CreateColorDataBaseImage( SizeX, SizeY, &DestColorBaseImage->ColorData, &ScaleBaseImage ) < 0 )
			{
				NS_ReleaseBaseImage( DestColorBaseImage ) ;
				if( AlphaFileImage ) NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
				DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 5\n" )) ;
				return -1 ;
			}
			
			ScalingBltBaseImage(
				0, 0, DestColorBaseImage->Width, DestColorBaseImage->Height, DestColorBaseImage,
				0, 0, SizeX, SizeY, &ScaleBaseImage, TRUE ) ;

			NS_ReleaseBaseImage( DestColorBaseImage ) ;
			*DestColorBaseImage = ScaleBaseImage ;
		}

		// 反転フラグが立っていたら反転する
		if( ReverseFlag )
		{
			NS_ReverseBaseImage( DestColorBaseImage ) ;
		}
	}

	// α画像のサイズが２のｎ乗ではなかったらスケーリングする
	if( AlphaFileImage != NULL )
	{
		for( SizeX = 1 ; SizeX < DestAlphaBaseImage->Width  ; SizeX <<= 1 ){}
		for( SizeY = 1 ; SizeY < DestAlphaBaseImage->Height ; SizeY <<= 1 ){}
		if( SizeX != DestAlphaBaseImage->Width || SizeY != DestAlphaBaseImage->Height )
		{
			// ハードウエアが対応している最大テクスチャサイズは超えないようにする
			if( GSYS.HardInfo.MaxTextureSize / 2 < SizeX ) SizeX = GSYS.HardInfo.MaxTextureSize / 2 ;
			if( GSYS.HardInfo.MaxTextureSize / 2 < SizeY ) SizeY = GSYS.HardInfo.MaxTextureSize / 2 ;

			if( NS_CreateColorDataBaseImage( SizeX, SizeY, &DestAlphaBaseImage->ColorData, &ScaleAlphaBaseImage ) < 0 )
			{
				NS_ReleaseBaseImage( DestColorBaseImage ) ;
				NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
				DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 6\n" )) ;
				return -1 ;
			}
			
			ScalingBltBaseImage(
				0, 0, DestAlphaBaseImage->Width, DestAlphaBaseImage->Height, DestAlphaBaseImage,
				0, 0, SizeX, SizeY, &ScaleAlphaBaseImage, TRUE ) ;

			NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
			*DestAlphaBaseImage = ScaleAlphaBaseImage ;
		}

		// 反転フラグが立っていたら反転する
		if( ReverseFlag )
		{
			NS_ReverseBaseImage( DestAlphaBaseImage ) ;
		}
	}

	// 二つの画像のサイズが異なる場合は大きい方に合わせる
	if( AlphaFileImage != NULL && ColorFileImage != NULL )
	{
		if( DestColorBaseImage->Width > DestAlphaBaseImage->Width ||
			DestColorBaseImage->Height > DestAlphaBaseImage->Height )
		{
			if( NS_CreateColorDataBaseImage( DestColorBaseImage->Width, DestColorBaseImage->Height, &DestAlphaBaseImage->ColorData, &ScaleAlphaBaseImage ) < 0 )
			{
				NS_ReleaseBaseImage( DestColorBaseImage ) ;
				NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
				DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 7\n" )) ;
				return -1 ;
			}
			
			ScalingBltBaseImage(
				0, 0, DestAlphaBaseImage->Width, DestAlphaBaseImage->Height, DestAlphaBaseImage,
				0, 0, DestColorBaseImage->Width, DestColorBaseImage->Height, &ScaleAlphaBaseImage, TRUE ) ;

			NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
			*DestAlphaBaseImage = ScaleAlphaBaseImage ;
		}
		else
		{
			if( NS_CreateColorDataBaseImage( DestAlphaBaseImage->Width, DestAlphaBaseImage->Height, &DestColorBaseImage->ColorData, &ScaleBaseImage ) < 0 )
			{
				NS_ReleaseBaseImage( DestColorBaseImage ) ;
				NS_ReleaseBaseImage( DestAlphaBaseImage ) ;
				DXST_ERRORLOGFMT_ADDW(( L"MV1CreateTextureColorBaseImage : Error 8\n" )) ;
				return -1 ;
			}
			
			ScalingBltBaseImage(
				0, 0, DestColorBaseImage->Width, DestColorBaseImage->Height, DestColorBaseImage,
				0, 0, DestAlphaBaseImage->Width, DestAlphaBaseImage->Height, &ScaleBaseImage, TRUE ) ;

			NS_ReleaseBaseImage( DestColorBaseImage ) ;
			*DestColorBaseImage = ScaleBaseImage ;
		}
	}

	// 終了
	return 0 ;
}

// テクスチャの再読み込み
extern int MV1ReloadTexture( void )
{
	int i, j ;
	MV1_TEXTURE_BASE *TexBase ;
	MV1_TEXTURE *Tex ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_MODEL *Model ;
	BASEIMAGE ColorBaseImage, AlphaBaseImage ;
	LOADGRAPH_GPARAM GParam ;

	if( HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].InitializeFlag == FALSE )
		return -1 ;

	Graphics_Image_InitLoadGraphGParam( &GParam ) ;
	GParam.CreateGraphGParam.NotUseTransColor = TRUE ;
	Graphics_Image_InitSetupGraphHandleGParam_Normal_NonDrawValid( &GParam.CreateGraphGParam.InitHandleGParam, 32, TRUE, FALSE ) ;
	Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, MV1Man.TexNoneHandle,                 -1, Tga8x8TextureFileImage,         sizeof( Tga8x8TextureFileImage ),         NULL, 0, TRUE, FALSE, FALSE ) ;
	Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, MV1Man.TexNoneBlackHandle,            -1, Tga8x8BlackTextureFileImage,    sizeof( Tga8x8BlackTextureFileImage ),    NULL, 0, TRUE, FALSE, FALSE ) ;
	Graphics_Image_CreateGraphFromMem_UseGParam( &GParam, MV1Man.ToonDefaultGradTexHandle[ 0 ], -1, TgaDiffuseDefaultGradFileImage, sizeof( TgaDiffuseDefaultGradFileImage ), NULL, 0, TRUE, FALSE, FALSE ) ;
//	NS_ReCreateGraphFromMem( &TgaSpecularDefaultGradFileImage, sizeof( TgaSpecularDefaultGradFileImage ), MV1Man.ToonDefaultGradTexHandle[ 1 ] ) ;
	_MV1ReCreateGradationGraph( MV1Man.ToonDefaultGradTexHandle[ 1 ] ) ;

	for( i = HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].AreaMax ; i ++ )
	{
		ModelBase = ( MV1_MODEL_BASE * )HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].Handle[ i ] ;
		if( ModelBase == NULL ) continue ;

		TexBase = ModelBase->Texture ;
		for( j = 0 ; j < ModelBase->TextureNum ; j ++, TexBase ++ )
		{
			if( MV1CreateTextureColorBaseImage(
					&ColorBaseImage,
					&AlphaBaseImage,
					TexBase->ColorImage, TexBase->ColorImageSize,
					TexBase->AlphaImage, TexBase->AlphaImageSize,
					TexBase->BumpImageFlag, TexBase->BumpImageNextPixelLength ) == 0 )
			{
				Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, TRUE, TexBase->GraphHandle, &ColorBaseImage, TexBase->AlphaImage ? &AlphaBaseImage : NULL, TRUE, FALSE, FALSE, FALSE ) ;

				NS_ReleaseBaseImage( &ColorBaseImage ) ;
				if( TexBase->AlphaImage ) NS_ReleaseBaseImage( &AlphaBaseImage ) ;
			}
		}
	}

	for( i = HandleManageArray[ DX_HANDLETYPE_MODEL ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MODEL ].AreaMax ; i ++ )
	{
		Model = ( MV1_MODEL * )HandleManageArray[ DX_HANDLETYPE_MODEL ].Handle[ i ] ;
		if( Model == NULL ) continue ;
		ModelBase = Model->BaseData ;

		Tex = Model->Texture ;
		for( j = 0 ; j < ModelBase->TextureNum ; j ++, Tex ++ )
		{
			if( Tex->UseGraphHandle == FALSE ) continue ;

			if( MV1CreateTextureColorBaseImage(
					&ColorBaseImage,
					&AlphaBaseImage,
					Tex->ColorImage, Tex->ColorImageSize,
					Tex->AlphaImage, Tex->AlphaImageSize,
					Tex->BumpImageFlag, Tex->BumpImageNextPixelLength ) == 0 )
			{
				Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, TRUE, Tex->GraphHandle, &ColorBaseImage, Tex->AlphaImage ? &AlphaBaseImage : NULL, TRUE, FALSE, FALSE, FALSE ) ;

				NS_ReleaseBaseImage( &ColorBaseImage ) ;
				if( Tex->AlphaImage ) NS_ReleaseBaseImage( &AlphaBaseImage ) ;
			}
		}
	}

	// 終了
	return 0 ;
}


// 追加ロード関数関係

// モデル読み込み関数を追加する
extern int MV1AddLoadFunc( int ( *AddLoadFunc )( const MV1_MODEL_LOAD_PARAM *LoadParam ) )
{
	int i ;

	if( MV1Man.AddLoadFuncNum == MV1_ADD_LOAD_FUNC_MAXNUM ) return -1 ;

	for( i = 0 ; i < MV1Man.AddLoadFuncNum && MV1Man.AddLoadFunc[ i ] != AddLoadFunc ; i ++ ){}
	if( i != MV1Man.AddLoadFuncNum ) return 0 ;

	MV1Man.AddLoadFunc[ MV1Man.AddLoadFuncNum ] = AddLoadFunc ;
	MV1Man.AddLoadFuncNum ++ ;

	// 終了
	return 0 ;
}

// モデル読み込み関数を削除する
extern int MV1SubLoadFunc( int ( *AddLoadFunc )( const MV1_MODEL_LOAD_PARAM *LoadParam ) )
{
	int i ;

	for( i = 0 ; i < MV1Man.AddLoadFuncNum && MV1Man.AddLoadFunc[ i ] != AddLoadFunc ; i ++ ){}
	if( i == MV1Man.AddLoadFuncNum ) return 0 ;

	MV1Man.AddLoadFuncNum -- ;

	if( i != MV1Man.AddLoadFuncNum )
	{
		_MEMMOVE( &MV1Man.AddLoadFunc[ i ], &MV1Man.AddLoadFunc[ i + 1 ], ( MV1Man.AddLoadFuncNum - i ) * sizeof( int ( * )( const MV1_MODEL_LOAD_PARAM * ) ) ) ;
	}

	// 終了
	return 0 ;
}


// 有効なモデル基本データをすべて削除する
extern int MV1InitModelBase( void )
{
	return AllHandleSub( DX_HANDLETYPE_MODEL_BASE ) ;
}

// モデルデータハンドルの初期化
extern int InitializeModelBaseHandle( HANDLEINFO * )
{
	// 特に何もしない
	return 0 ;
}

// モデルデータハンドルの後始末
extern int TerminateModelBaseHandle( HANDLEINFO *HandleInfo )
{
	MV1_MODEL_BASE *MBase = ( MV1_MODEL_BASE * )HandleInfo ;
	MV1_TRIANGLE_LIST_BASE *TList ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_TEXTURE_BASE *Texture ;
	int i ;

	// 参照カウントをデクリメント
	MBase->RefCount -- ;

	// 参照カウントが０以上の場合はまだ解放しない
	if( MBase->RefCount > 0 )
		return 1 ;

	// 頂点バッファを開放
	MV1_TerminateVertexBufferBase_PF( HandleInfo->Handle ) ;

	// 環境依存のデータの後始末
	MV1_TerminateModelBaseHandle_PF( MBase ) ;

	// 頂点データの解放
	if( MBase->VertexData != NULL )
		DXFREE( MBase->VertexData ) ;
	MBase->TriangleListNormalPosition = NULL ;
	MBase->TriangleListSkinPosition4B = NULL ;
	MBase->TriangleListSkinPosition8B = NULL ;
	MBase->TriangleListSkinPositionFREEB = NULL ;
	MBase->MeshVertexIndex = NULL ;
	MBase->MeshPosition = NULL ;
	MBase->MeshNormal = NULL ;
	MBase->MeshFace = NULL ;
	MBase->MeshVertex = NULL ;

	// テクスチャを解放する
	Texture = MBase->Texture ;
	for( i = 0 ; i < MBase->TextureNum ; i ++, Texture ++ )
	{
		// 名前の解放
		if( Texture->NameAllocMem )
		{
#ifndef UNICODE
			if( Texture->NameA )
			{
				DXFREE( Texture->NameA ) ;
				Texture->NameA = NULL ;
			}
#endif

			if( Texture->NameW )
			{
				DXFREE( Texture->NameW ) ;
				Texture->NameW = NULL ;
			}
		}

		// 画像ハンドルを削除
		NS_DeleteGraph( Texture->GraphHandle ) ;
		Texture->GraphHandle = 0 ;

		// ファイルパスの解放
		if( Texture->ColorImageFilePathAllocMem )
		{
#ifndef UNICODE
			if( Texture->ColorFilePathA )
			{
				DXFREE( Texture->ColorFilePathA ) ;
			}
			Texture->ColorFilePathA = NULL ;
#endif
			if( Texture->ColorFilePathW )
			{
				DXFREE( Texture->ColorFilePathW ) ;
			}
			Texture->ColorFilePathW = NULL ;
		}

		if( Texture->AlphaImageFilePathAllocMem )
		{
#ifndef UNICODE
			if( Texture->AlphaFilePathA )
			{
				DXFREE( Texture->AlphaFilePathA ) ;
			}
			Texture->AlphaFilePathA = NULL ;
#endif
			if( Texture->AlphaFilePathW )
			{
				DXFREE( Texture->AlphaFilePathW ) ;
			}
			Texture->AlphaFilePathW = NULL ;
		}

		// ファイルイメージの解放
		if( Texture->ColorImage )
		{
			DXFREE( Texture->ColorImage ) ;
			Texture->ColorImage = NULL ;
		}

		if( Texture->AlphaImage )
		{
			DXFREE( Texture->AlphaImage ) ;
			Texture->AlphaImage = NULL ;
		}
	}
	if( MBase->TextureAllocMem )
	{
		DXFREE( MBase->Texture ) ;
		MBase->Texture = NULL ;
	}

	// 別個にシェイプ頂点データ用のメモリを確保していた場合は解放する
	for( i = 0 ; i < MBase->ShapeMeshNum ; i ++ )
	{
		if( MBase->ShapeMesh[ i ].VertexAllocMem )
		{
			DXFREE( MBase->ShapeMesh[ i ].Vertex ) ;
			MBase->ShapeMesh[ i ].Vertex = NULL ;
		}
	}

	// 別個に法線データ用のメモリを確保していた場合は解放する
	for( i = 0 ; i < MBase->FrameNum ; i ++ )
	{
		if( MBase->Frame[ i ].NormalAllocMem )
		{
			DXFREE( MBase->Frame[ i ].Normal ) ;
			MBase->Frame[ i ].Normal = NULL ;
		}
	}

	// 別個に頂点データ用のメモリを確保していた場合は解放する
	for( i = 0 ; i < MBase->MeshNum ; i ++ )
	{
		if( MBase->Mesh[ i ].VertexAllocMem )
		{
			DXFREE( MBase->Mesh[ i ].Vertex ) ;
			MBase->Mesh[ i ].Vertex = NULL ;
		}
	}

	// 別個に確保していたメモリがあった場合は解放する
	TList = MBase->TriangleList ;
	for( i = 0 ; i < MBase->TriangleListNum ; i ++, TList ++ )
	{
		MV1_TRIANGLE_LIST_BASE_PF *TListPF ;

		TListPF = ( MV1_TRIANGLE_LIST_BASE_PF * )TList->PFBuffer ;

		if( TList->MeshVertexIndexAllocMem )
		{
			DXFREE( TList->MeshVertexIndex ) ;
			TList->MeshVertexIndex = NULL ;
		}
		if( TList->PositionAllocMem )
		{
			DXFREE( TList->NormalPosition ) ;
			TList->NormalPosition = NULL ;
		}

		MV1_TerminateTriangleListBaseTempBuffer_PF( TList ) ;
	}

	// アニメーション名を後から変更した場合は開放する
	AnimSetBase = MBase->AnimSet ;
	for( i = 0 ; i < MBase->AnimSetNum ; i ++, AnimSetBase ++ )
	{
		if( AnimSetBase->NameAllocMem )
		{
#ifndef UNICODE
			if( AnimSetBase->NameA )
			{
				DXFREE( AnimSetBase->NameA ) ;
				AnimSetBase->NameA = NULL ;
			}
#endif
			if( AnimSetBase->NameW )
			{
				DXFREE( AnimSetBase->NameW ) ;
				AnimSetBase->NameW = NULL ;
			}
		}
	}


	// 追加確保メモリの解放
	ClearMemArea( &MBase->AddFirstMem ) ;

	// データを格納していたメモリの解放
	if( MBase->DataBuffer )
	{
		DXFREE( MBase->DataBuffer ) ;
		MBase->DataBuffer = NULL ;
	}

	// 終了
	return 0 ;
}

// モデル基本データを追加する( -1:エラー  0以上:モデル基本データハンドル )
int MV1AddModelBase( int ASyncThread )
{
	return AddHandle( DX_HANDLETYPE_MODEL_BASE, ASyncThread, -1 ) ;
}

// モデル基本データを削除する
extern int MV1SubModelBase( int MBHandle )
{
	return SubHandle( MBHandle ) ;
}

// モデル基本データを複製する
extern int MV1CreateCloneModelBase( int SrcMBHandle )
{
	MV1_MODEL_BASE MTBase, *MBase = NULL, *FHeader ;
	MV1_FRAME_BASE *Frame ;
	MV1_FRAME_BASE *F1Frame ;
	MV1_MATERIAL_BASE			*Material ;
	MV1_MATERIAL_BASE			*F1Material ;
	MV1_MATERIAL_LAYER			*MaterialLayer ;
	MV1_MATERIAL_LAYER			*F1MaterialLayer ;
	MV1_LIGHT					*Light ;
	MV1_LIGHT					*F1Light ;
	MV1_TEXTURE_BASE			*Texture ;
	MV1_TEXTURE_BASE			*F1Texture ;
	MV1_MESH_BASE				*Mesh ;
	MV1_MESH_BASE				*F1Mesh ;
	MV1_MESH_FACE				*Face ;
	MV1_SKIN_BONE				*SkinBone ;
	MV1_SKIN_BONE				*F1SkinBone ;
	MV1_TRIANGLE_LIST_BASE		*TriangleList ;
	MV1_TRIANGLE_LIST_BASE		*F1TriangleList ;
	MV1_ANIM_KEYSET_BASE		*AnimKeySet ;
	MV1_ANIM_KEYSET_BASE		*F1AnimKeySet ;
	MV1_ANIM_BASE				*Anim ;
	MV1_ANIM_BASE				*F1Anim ;
	MV1_ANIMSET_BASE			*AnimSet ;
	MV1_ANIMSET_BASE			*F1AnimSet ;
	MV1_SHAPE_BASE				*Shape ;
	MV1_SHAPE_BASE				*F1Shape ;
	MV1_SHAPE_MESH_BASE			*ShapeMesh ;
	MV1_SHAPE_MESH_BASE			*F1ShapeMesh ;
	MV1_PHYSICS_RIGIDBODY_BASE	*PhysicsRigidBody ;
	MV1_PHYSICS_RIGIDBODY_BASE	*F1PhysicsRigidBody ;
	MV1_PHYSICS_JOINT_BASE		*PhysicsJoint ;
	MV1_PHYSICS_JOINT_BASE		*F1PhysicsJoint ;
	int i, j, k, NewHandle = 0 ;
	DWORD_PTR AllocSize ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// ハンドルチェック
	if( MV1BMDLCHK( SrcMBHandle, FHeader ) )
		return -1 ;

	// 必要なメモリサイズの算出
	{
		_MEMSET( &MTBase, 0, sizeof( MTBase ) ) ;
		AllocSize = 0 ;

		MTBase.Name = ( wchar_t * )AllocSize ;
		AllocSize += ( ( _WCSLEN( FHeader->Name ) + 1 ) * sizeof( wchar_t ) + 3 ) / 4 * 4 ;

		MTBase.FilePath = ( wchar_t * )AllocSize ;
		AllocSize += ( ( _WCSLEN( FHeader->FilePath ) + 1 ) * sizeof( wchar_t ) + 3 ) / 4 * 4 ;

		MTBase.DirectoryPath = ( wchar_t * )AllocSize ;
		AllocSize += ( ( _WCSLEN( FHeader->DirectoryPath ) + 1 ) * sizeof( wchar_t ) + 3 ) / 4 * 4 ;

		MTBase.Frame = ( MV1_FRAME_BASE * )AllocSize ;
		AllocSize += FHeader->FrameNum * sizeof( MV1_FRAME_BASE ) ;

		MTBase.FrameUseSkinBone = ( MV1_SKIN_BONE ** )AllocSize ;
		AllocSize += FHeader->FrameUseSkinBoneNum * sizeof( MV1_SKIN_BONE * ) ;

		MTBase.Material = ( MV1_MATERIAL_BASE * )AllocSize ;
		AllocSize += FHeader->MaterialNum * sizeof( MV1_MATERIAL_BASE ) ;

		MTBase.Light = ( MV1_LIGHT * )AllocSize ;
		AllocSize += FHeader->LightNum * sizeof( MV1_LIGHT ) ;

		MTBase.Texture = ( MV1_TEXTURE_BASE * )AllocSize ;
		AllocSize += FHeader->TextureNum * sizeof( MV1_TEXTURE_BASE ) ;

		MTBase.Mesh = ( MV1_MESH_BASE * )AllocSize ;
		AllocSize += FHeader->MeshNum * sizeof( MV1_MESH_BASE ) ;

		MTBase.SkinBone = ( MV1_SKIN_BONE * )AllocSize ;
		AllocSize += FHeader->SkinBoneNum * sizeof( MV1_SKIN_BONE ) ;
		MTBase.SkinBoneUseFrame = ( MV1_SKIN_BONE_USE_FRAME * )AllocSize ;
		AllocSize += FHeader->SkinBoneUseFrameNum * sizeof( MV1_SKIN_BONE_USE_FRAME ) ;

		MTBase.TriangleList = ( MV1_TRIANGLE_LIST_BASE * )AllocSize ;
		AllocSize += FHeader->TriangleListNum * sizeof( MV1_TRIANGLE_LIST_BASE ) ;

		MTBase.TriangleListIndex = ( WORD * )AllocSize ;
		AllocSize += sizeof( WORD ) * FHeader->TriangleListIndexNum * 2 ;

#ifndef UNICODE
		MTBase.StringBufferA = ( char * )AllocSize ;
		AllocSize += FHeader->StringSizeA ;
#endif

		MTBase.StringBufferW = ( wchar_t * )AllocSize ;
		AllocSize += FHeader->StringSizeW ;

		MTBase.ChangeDrawMaterialTable = ( DWORD * )AllocSize ;
		AllocSize += FHeader->ChangeDrawMaterialTableSize ;

		MTBase.ChangeMatrixTable = ( DWORD * )AllocSize ;
		AllocSize += FHeader->ChangeMatrixTableSize ;

		MTBase.AnimKeyData = ( void * )AllocSize ;
		AllocSize += FHeader->AnimKeyDataSize ;

		MTBase.AnimKeySet = ( MV1_ANIM_KEYSET_BASE * )AllocSize ;
		AllocSize += FHeader->AnimKeySetNum * sizeof( MV1_ANIM_KEYSET_BASE ) ;

		MTBase.Anim = ( MV1_ANIM_BASE * )AllocSize ;
		AllocSize += FHeader->AnimNum * sizeof( MV1_ANIM_BASE ) ;

		MTBase.AnimSet = ( MV1_ANIMSET_BASE * )AllocSize ;
		AllocSize += FHeader->AnimSetNum * sizeof( MV1_ANIMSET_BASE ) ;

		MTBase.AnimTargetFrameTable = ( MV1_ANIM_BASE ** )AllocSize ;
		AllocSize += FHeader->FrameNum * FHeader->AnimSetNum * sizeof( MV1_ANIM_BASE * ) ;

		if( FHeader->Shape != NULL )
		{
			MTBase.Shape = ( MV1_SHAPE_BASE * )AllocSize ;
			AllocSize += FHeader->ShapeNum * sizeof( MV1_SHAPE_BASE ) ;

			MTBase.ShapeMesh = ( MV1_SHAPE_MESH_BASE * )AllocSize ;
			AllocSize += FHeader->ShapeMeshNum * sizeof( MV1_SHAPE_MESH_BASE ) ;
		}

		if( FHeader->PhysicsJoint != NULL )
		{
			MTBase.PhysicsRigidBody = ( MV1_PHYSICS_RIGIDBODY_BASE * )AllocSize ;
			AllocSize += FHeader->PhysicsRigidBodyNum * sizeof( MV1_PHYSICS_RIGIDBODY_BASE ) ;

			MTBase.PhysicsJoint = ( MV1_PHYSICS_JOINT_BASE * )AllocSize ;
			AllocSize += FHeader->PhysicsJointNum * sizeof( MV1_PHYSICS_JOINT_BASE ) ;
		}
	}

	// モデル基データハンドルの作成
	NewHandle = MV1AddModelBase( FALSE ) ;
	if( NewHandle < 0 )
	{
		goto ERRORLABEL ;
	}

	// メモリの確保
	if( MV1BMDLCHK_ASYNC( NewHandle, MBase ) )
	{
		return -1 ;
	}
	MBase->DataBuffer = MDALLOCMEM( AllocSize ) ;
	if( MBase->DataBuffer == NULL )
	{
		goto ERRORLABEL ;
	}
	_MEMSET( MBase->DataBuffer, 0, AllocSize ) ;
	MBase->AllocMemorySize = AllocSize ;

	// メモリアドレスのセット
	MBase->Name                   = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Name                 ) ;
	MBase->FilePath               = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.FilePath             ) ;
	MBase->DirectoryPath          = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.DirectoryPath        ) ;
	MBase->ChangeMatrixTable      = ( DWORD * )                     ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.ChangeMatrixTable    ) ;
	MBase->ChangeDrawMaterialTable = ( DWORD * )                    ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.ChangeDrawMaterialTable ) ;
	MBase->Frame                  = ( MV1_FRAME_BASE * )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Frame                ) ;
	MBase->FrameUseSkinBone       = ( MV1_SKIN_BONE ** )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.FrameUseSkinBone     ) ;
	MBase->Material               = ( MV1_MATERIAL_BASE * )         ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Material             ) ;
	MBase->Light                  = ( MV1_LIGHT * )                 ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Light                ) ;
	MBase->Texture                = ( MV1_TEXTURE_BASE * )          ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Texture              ) ;
	MBase->SkinBone               = ( MV1_SKIN_BONE * )             ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.SkinBone             ) ;
	MBase->SkinBoneUseFrame       = ( MV1_SKIN_BONE_USE_FRAME * )   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.SkinBoneUseFrame     ) ;
	MBase->Mesh                   = ( MV1_MESH_BASE * )             ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Mesh                 ) ;
	MBase->TriangleList           = ( MV1_TRIANGLE_LIST_BASE * )    ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.TriangleList         ) ;
	MBase->TriangleListIndex      = ( WORD * )                      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.TriangleListIndex    ) ;
#ifndef UNICODE
	MBase->StringBufferA          = ( char * )                      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.StringBufferA        ) ;
#endif
	MBase->StringBufferW          = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.StringBufferW        ) ;
	MBase->AnimSet                = ( MV1_ANIMSET_BASE * )          ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimSet              ) ;
	MBase->Anim                   = ( MV1_ANIM_BASE * )             ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Anim                 ) ;
	MBase->AnimKeySet             = ( MV1_ANIM_KEYSET_BASE * )      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimKeySet           ) ;
	MBase->AnimKeyData            = ( void * )                      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimKeyData          ) ;
	MBase->AnimTargetFrameTable   = ( MV1_ANIM_BASE ** )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimTargetFrameTable ) ;
	if( FHeader->Shape )
	{
		MBase->Shape              = ( MV1_SHAPE_BASE * )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Shape                ) ;
		MBase->ShapeMesh          = ( MV1_SHAPE_MESH_BASE * )       ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.ShapeMesh            ) ;
	}
	if( FHeader->PhysicsRigidBody )
	{
		MBase->PhysicsRigidBody   = ( MV1_PHYSICS_RIGIDBODY_BASE * )( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.PhysicsRigidBody     ) ;
		MBase->PhysicsJoint       = ( MV1_PHYSICS_JOINT_BASE *     )( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.PhysicsJoint         ) ;
	}

	// 頂点データインデックス値のコピー
	if( FHeader->TriangleListIndexNum != 0 )
	{
		_MEMCPY( MBase->TriangleListIndex, FHeader->TriangleListIndex, FHeader->TriangleListIndexNum * sizeof( WORD ) * 2 ) ;
	}

	// アニメーションキーデータのコピー
	if( FHeader->AnimKeyDataSize != 0 )
	{
		_MEMCPY( MBase->AnimKeyData, FHeader->AnimKeyData, ( size_t )FHeader->AnimKeyDataSize ) ;
	}

	// モデル名とファイルパスとフォルダパスを保存
	_WCSCPY( MBase->Name,          FHeader->Name ) ;
	_WCSCPY( MBase->FilePath,      FHeader->FilePath ) ;
	_WCSCPY( MBase->DirectoryPath, FHeader->DirectoryPath ) ;

	// 同時複数描画に対応するかどうかを保存
	MBase->UsePackDraw = FHeader->UsePackDraw ;

	// 座標系情報を保存
	MBase->RightHandType = FHeader->RightHandType ;

	// 割り当てられているマテリアルの番号が低いメッシュから描画するかどうかの情報を保存
	MBase->MaterialNumberOrderDraw = FHeader->MaterialNumberOrderDraw ;

	// 変更チェック用テーブルに必要なデータのサイズの保存とコピー
	MBase->ChangeDrawMaterialTableSize  = FHeader->ChangeDrawMaterialTableSize ;
	MBase->ChangeMatrixTableSize        = FHeader->ChangeMatrixTableSize ;
	_MEMCPY( MBase->ChangeDrawMaterialTable, FHeader->ChangeDrawMaterialTable, ( size_t )FHeader->ChangeDrawMaterialTableSize ) ;
	_MEMCPY( MBase->ChangeMatrixTable,       FHeader->ChangeMatrixTable,       ( size_t )FHeader->ChangeMatrixTableSize ) ;

	// ポリゴン数を保存
	MBase->TriangleNum = FHeader->TriangleNum ;

	// 頂点データの数を保存
	MBase->TriangleListVertexNum = FHeader->TriangleListVertexNum ;

	// フレームが使用しているボーンへのポインタの情報をコピー
	MBase->FrameUseSkinBoneNum = FHeader->FrameUseSkinBoneNum ;
	for( i = 0 ; i < MBase->FrameUseSkinBoneNum ; i ++ )
	{
		MBase->FrameUseSkinBone[ i ] = MBase->SkinBone + ( FHeader->FrameUseSkinBone[ i ] - FHeader->SkinBone ) ;
	}

	// スキニング用ボーンを使用するフレームの情報をコピー
	_MEMCPY( MBase->SkinBoneUseFrame, FHeader->SkinBoneUseFrame, FHeader->SkinBoneUseFrameNum * sizeof( MV1_SKIN_BONE_USE_FRAME_F1 ) ) ;

	// 文字列データをコピー
#ifndef UNICODE
	_MEMCPY( MBase->StringBufferA, FHeader->StringBufferA, ( size_t )FHeader->StringSizeA ) ;
	MBase->StringSizeA = FHeader->StringSizeA ;
#endif
	_MEMCPY( MBase->StringBufferW, FHeader->StringBufferW, ( size_t )FHeader->StringSizeW ) ;
	MBase->StringSizeW = FHeader->StringSizeW ;

	// 各オブジェクトの数の情報を保存
	MBase->FrameNum              = FHeader->FrameNum ;
	MBase->MaterialNum           = FHeader->MaterialNum ;
	MBase->LightNum              = FHeader->LightNum ;
	MBase->TextureNum            = FHeader->TextureNum ;
	MBase->MeshNum               = FHeader->MeshNum ;
	MBase->SkinBoneNum           = FHeader->SkinBoneNum ;
	MBase->SkinBoneUseFrameNum   = FHeader->SkinBoneUseFrameNum ;
	MBase->TriangleListNum       = FHeader->TriangleListNum ;
	MBase->AnimKeyDataSize       = FHeader->AnimKeyDataSize ;
	MBase->AnimKeySetNum         = FHeader->AnimKeySetNum ;
	MBase->AnimNum               = FHeader->AnimNum ;
	MBase->AnimSetNum            = FHeader->AnimSetNum ;
	if( FHeader->Shape )
	{
		MBase->ShapeNum          = FHeader->ShapeNum ;
		MBase->ShapeMeshNum      = FHeader->ShapeMeshNum ;

		MBase->ShapeNormalPositionNum = FHeader->ShapeNormalPositionNum ;
		MBase->ShapeSkinPosition4BNum = FHeader->ShapeSkinPosition4BNum ;
		MBase->ShapeSkinPosition8BNum = FHeader->ShapeSkinPosition8BNum ;
		MBase->ShapeSkinPositionFREEBSize = FHeader->ShapeSkinPositionFREEBSize ;

		MBase->ShapeTargetMeshVertexNum = FHeader->ShapeTargetMeshVertexNum ;
	}
	if( FHeader->PhysicsRigidBody )
	{
		MBase->PhysicsGravity    = FHeader->PhysicsGravity ;

		MBase->PhysicsRigidBodyNum = FHeader->PhysicsRigidBodyNum ;
		MBase->PhysicsJointNum   = FHeader->PhysicsJointNum ;
	}

	// 頂点データを格納するためのメモリ領域の確保
	{
		MBase->VertexDataSize = 
			FHeader->MeshVertexIndexNum                * sizeof( DWORD )                 +
			FHeader->MeshFaceNum                       * sizeof( MV1_MESH_FACE )         +
			FHeader->MeshNormalNum                     * sizeof( MV1_MESH_NORMAL )       +
			FHeader->TriangleListNormalPositionNum     * sizeof( MV1_TLIST_NORMAL_POS )  +
			FHeader->TriangleListSkinPosition4BNum     * sizeof( MV1_TLIST_SKIN_POS_4B ) +
			FHeader->TriangleListSkinPosition8BNum     * sizeof( MV1_TLIST_SKIN_POS_8B ) +
			FHeader->TriangleListSkinPositionFREEBSize                                   +
			FHeader->MeshPositionSize                                                    +
			FHeader->MeshVertexSize                                                      + 16 ;
		/*
		F1Mesh = FHeader->Mesh ;
		MeshVertexNum = 0 ;
		for( i = 0 ; i < FHeader->MeshNum ; i ++, F1Mesh ++ )
		{
			MeshVertexNum += F1Mesh->VertexNum ;
		}
		MBase->VertexDataSize += MeshVertexNum * sizeof( DWORD ) ;
		*/
		if( FHeader->Shape )
		{
			MBase->VertexDataSize += FHeader->ShapeVertexNum * sizeof( MV1_SHAPE_VERTEX_BASE ) ;
		}
		if( MBase->VertexDataSize != 0 )
		{
			MBase->VertexData = DXALLOC( MBase->VertexDataSize + 16 ) ;
			if( MBase->VertexData == NULL ) goto ERRORLABEL ;
			_MEMCPY( ( void * )( ( ( DWORD_PTR )MBase->VertexData + 15 ) / 16 * 16 ), ( void * )( ( ( DWORD_PTR )FHeader->VertexData + 15 ) / 16 * 16 ), MBase->VertexDataSize ) ;
			//_MEMSET( ( void * )( ( ( DWORD_PTR )MBase->VertexData + 15 ) / 16 * 16 ), 0, MBase->VertexDataSize ) ;
		}

		MBase->TriangleListNormalPosition    = ( MV1_TLIST_NORMAL_POS     * )( ( ( DWORD_PTR )MBase->VertexData + 15 ) / 16 * 16 ) ;
		MBase->TriangleListSkinPosition4B    = ( MV1_TLIST_SKIN_POS_4B    * )( MBase->TriangleListNormalPosition              + FHeader->TriangleListNormalPositionNum     ) ;
		MBase->TriangleListSkinPosition8B    = ( MV1_TLIST_SKIN_POS_8B    * )( MBase->TriangleListSkinPosition4B              + FHeader->TriangleListSkinPosition4BNum     ) ;
		MBase->TriangleListSkinPositionFREEB = ( MV1_TLIST_SKIN_POS_FREEB * )( MBase->TriangleListSkinPosition8B              + FHeader->TriangleListSkinPosition8BNum     ) ;
		MBase->MeshVertexIndex               = ( DWORD                    * )( ( BYTE * )MBase->TriangleListSkinPositionFREEB + FHeader->TriangleListSkinPositionFREEBSize ) ;
		MBase->MeshFace                      = ( MV1_MESH_FACE            * )( MBase->MeshVertexIndex                         + FHeader->MeshVertexIndexNum    ) ;
		MBase->MeshNormal                    = ( MV1_MESH_NORMAL          * )( MBase->MeshFace                                + FHeader->MeshFaceNum           ) ;
		MBase->MeshPosition                  = ( MV1_MESH_POSITION        * )( MBase->MeshNormal                              + FHeader->MeshNormalNum         ) ;
		MBase->MeshVertex                    = ( MV1_MESH_VERTEX          * )( ( BYTE * )MBase->MeshPosition                  + FHeader->MeshPositionSize      ) ;
		if( FHeader->Shape )
		{
			MBase->ShapeVertex               = ( MV1_SHAPE_VERTEX_BASE    * )( ( BYTE * )MBase->MeshVertex                    + FHeader->MeshVertexSize        ) ;
		}
		if( FHeader->TriangleListNormalPositionNum == 0     ) MBase->TriangleListNormalPosition    = NULL ;
		if( FHeader->TriangleListSkinPosition4BNum == 0     ) MBase->TriangleListSkinPosition4B    = NULL ;
		if( FHeader->TriangleListSkinPosition8BNum == 0     ) MBase->TriangleListSkinPosition8B    = NULL ;
		if( FHeader->TriangleListSkinPositionFREEBSize == 0 ) MBase->TriangleListSkinPositionFREEB = NULL ;
		if( FHeader->MeshVertexIndexNum == 0                ) MBase->MeshVertexIndex   = NULL ;
		if( FHeader->MeshFaceNum == 0                       ) MBase->MeshFace          = NULL ;
		if( FHeader->MeshNormalNum == 0                     ) MBase->MeshNormal        = NULL ;
		if( FHeader->MeshPositionSize == 0                  ) MBase->MeshPosition      = NULL ;
		if( FHeader->MeshVertexSize == 0                    ) MBase->MeshVertex        = NULL ;
		if( FHeader->Shape == NULL || FHeader->ShapeVertexNum == 0 ) MBase->ShapeVertex = NULL ;
	}

	// フレームの情報をセット
	MBase->TopFrameNum  = FHeader->TopFrameNum ;
	MBase->FirstTopFrame = FHeader->FirstTopFrame ? MBase->Frame + FHeader->FirstTopFrame->Index : NULL ;
	MBase->LastTopFrame  = FHeader->LastTopFrame  ? MBase->Frame + FHeader->LastTopFrame->Index  : NULL ;
	Frame = MBase->Frame ;
	F1Frame = FHeader->Frame ;
	for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++, F1Frame ++ )
	{
		Frame->Container = MBase ;

#ifndef UNICODE
		Frame->NameA = MBase->StringBufferA + ( F1Frame->NameA - FHeader->StringBufferA ) ;
#endif
		Frame->NameW = MBase->StringBufferW + ( F1Frame->NameW - FHeader->StringBufferW ) ;
		Frame->AutoCreateNormal = F1Frame->AutoCreateNormal ;

		Frame->Index = F1Frame->Index ;

		Frame->ChangeDrawMaterialInfo.Target   = F1Frame->ChangeDrawMaterialInfo.Target ;
		Frame->ChangeDrawMaterialInfo.Fill     = MBase->ChangeDrawMaterialTable + ( F1Frame->ChangeDrawMaterialInfo.Fill - FHeader->ChangeDrawMaterialTable ) ;
		Frame->ChangeDrawMaterialInfo.Size     = F1Frame->ChangeDrawMaterialInfo.Size ;
		Frame->ChangeDrawMaterialInfo.CheckBit = F1Frame->ChangeDrawMaterialInfo.CheckBit ;

		Frame->ChangeMatrixInfo.Target         = F1Frame->ChangeMatrixInfo.Target ;
		Frame->ChangeMatrixInfo.Fill           = MBase->ChangeMatrixTable       + ( F1Frame->ChangeMatrixInfo.Fill       - FHeader->ChangeMatrixTable ) ;
		Frame->ChangeMatrixInfo.Size           = F1Frame->ChangeMatrixInfo.Size ;
		Frame->ChangeMatrixInfo.CheckBit       = F1Frame->ChangeMatrixInfo.CheckBit ;

		Frame->TotalMeshNum  = F1Frame->TotalMeshNum ;
		Frame->TotalChildNum = F1Frame->TotalChildNum ;
		Frame->Parent     = F1Frame->Parent     ? MBase->Frame + F1Frame->Parent->Index : NULL ;
		Frame->FirstChild = F1Frame->FirstChild ? MBase->Frame + F1Frame->FirstChild->Index : NULL ;
		Frame->LastChild  = F1Frame->LastChild  ? MBase->Frame + F1Frame->LastChild->Index : NULL ;
		Frame->Prev       = F1Frame->Prev       ? MBase->Frame + F1Frame->Prev->Index : NULL ;
		Frame->Next       = F1Frame->Next       ? MBase->Frame + F1Frame->Next->Index : NULL ;

		Frame->Translate = F1Frame->Translate ;
		Frame->Scale = F1Frame->Scale ;
		Frame->Rotate = F1Frame->Rotate ;
		Frame->RotateOrder = F1Frame->RotateOrder ;
		Frame->Quaternion = F1Frame->Quaternion ;

		Frame->Flag = F1Frame->Flag ;
		if( F1Frame->Flag & MV1_FRAMEFLAG_IGNOREPARENTTRANS ) Frame->IgnoreParentTransform = 1 ;

		if( F1Frame->Flag & MV1_FRAMEFLAG_PREROTATE )
		{
			Frame->PreRotate = F1Frame->PreRotate ;
		}

		if( F1Frame->Flag & MV1_FRAMEFLAG_POSTROTATE )
		{
			Frame->PreRotate = F1Frame->PostRotate ;
		}

		Frame->IsSkinMesh = F1Frame->IsSkinMesh ;
		Frame->TriangleNum = F1Frame->TriangleNum ;
		Frame->VertexNum = F1Frame->VertexNum ;
		Frame->MeshNum = F1Frame->MeshNum ;
		if( F1Frame->Mesh )
		{
			Frame->Mesh = MBase->Mesh + ( F1Frame->Mesh - FHeader->Mesh ) ;
		}

		if( F1Frame->Shape )
		{
			Frame->ShapeNum = F1Frame->ShapeNum ;
			Frame->Shape = MBase->Shape + ( F1Frame->Shape - FHeader->Shape ) ;
		}

		Frame->SkinBoneNum = F1Frame->SkinBoneNum ;
		if( F1Frame->SkinBone )
		{
			Frame->SkinBone = MBase->SkinBone + ( F1Frame->SkinBone - FHeader->SkinBone ) ;
		}

		if( F1Frame->Light )
		{
			Frame->Light = MBase->Light + F1Frame->Light->Index ;
		}

		Frame->UseSkinBoneNum = F1Frame->UseSkinBoneNum ;
		if( F1Frame->UseSkinBoneNum )
		{
			Frame->UseSkinBone = MBase->FrameUseSkinBone + ( F1Frame->UseSkinBone - FHeader->FrameUseSkinBone ) ;
		}

		Frame->SmoothingAngle = F1Frame->SmoothingAngle ;

		Frame->MaxBoneBlendNum = F1Frame->MaxBoneBlendNum ;
		Frame->PositionNum     = F1Frame->PositionNum ;
		Frame->NormalNum       = F1Frame->NormalNum ;

		// 座標データと法線データのコピー
		if( F1Frame->PositionNum == 0 )
		{
			Frame->Position = NULL ;
			Frame->Normal = NULL ;
		}
		else
		{
			Frame->PosUnitSize = ( int )( sizeof( MV1_MESH_POSITION ) + ( Frame->MaxBoneBlendNum - 4 ) * sizeof( MV1_SKINBONE_BLEND ) ) ;

			Frame->Position = ( MV1_MESH_POSITION * )( ( BYTE * )MBase->MeshPosition + MBase->MeshPositionSize ) ;
			Frame->Normal   = MBase->MeshNormal + MBase->MeshNormalNum ;
			MBase->MeshPositionSize += Frame->PosUnitSize * Frame->PositionNum ;
			MBase->MeshNormalNum    += Frame->NormalNum ;

			// 頂点データのコピー
			_MEMCPY( Frame->Position, F1Frame->Position, ( size_t )( Frame->PosUnitSize * Frame->PositionNum ) ) ;

			// 法線データのコピー
			_MEMCPY( Frame->Normal, F1Frame->Normal, sizeof( MV1_MESH_NORMAL ) * Frame->NormalNum ) ;
		}
	}

	// テクスチャの情報をセット
	MBase->TextureAllocMem = FALSE ;
	Texture = MBase->Texture ;
	F1Texture = FHeader->Texture ;
	for( i = 0 ; i < MBase->TextureNum ; i ++, Texture ++, F1Texture ++ )
	{
		// ユーザーデータを初期化
		Texture->UserData[ 0 ] = 0 ;
		Texture->UserData[ 1 ] = 0 ;
		Texture->UseUserGraphHandle = 0 ;
		Texture->UserGraphHandle = 0 ;

		// 名前を保存
		Texture->NameAllocMem = FALSE ;
#ifndef UNICODE
		Texture->NameA = MBase->StringBufferA + ( F1Texture->NameA - FHeader->StringBufferA ) ;
#endif
		Texture->NameW = MBase->StringBufferW + ( F1Texture->NameW - FHeader->StringBufferW ) ;

		// 反転フラグをセットする
		Texture->ReverseFlag = F1Texture->ReverseFlag ;

		// ３２ビットＢＭＰのアルファ値が全部０だったら XRGB8 として扱うかどうかのフラグをセットする
		Texture->Bmp32AllZeroAlphaToXRGB8Flag = F1Texture->Bmp32AllZeroAlphaToXRGB8Flag ;

		// テクスチャの読み込み
		if( __MV1LoadTexture(
				&F1Texture->ColorImage, &F1Texture->ColorImageSize,
				&F1Texture->AlphaImage, &F1Texture->AlphaImageSize,
				&Texture->GraphHandle,
				&Texture->SemiTransFlag,
				&Texture->IsDefaultTexture,
#ifndef UNICODE
				NULL, NULL,
#endif
				NULL, NULL,
				NULL,
				NULL,
				NULL,
				F1Texture->BumpImageFlag, F1Texture->BumpImageNextPixelLength,
				Texture->ReverseFlag,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag,
				NULL,
				true,
				FALSE ) == -1 )
		{
			DXST_ERRORLOGFMT_ADDW(( L"MV1 CloneModel Error : Texture LoadError : %s\n", Texture->NameW ) ) ;
			goto ERRORLABEL ;
		}

		// ファイルパス用にメモリを確保したフラグを倒す
		Texture->ColorImageFilePathAllocMem = FALSE ;
		Texture->AlphaImageFilePathAllocMem = FALSE ;

		// ファイルパスを保存
		if( Texture->ColorImage )
		{
#ifndef UNICODE
			Texture->ColorFilePathA = MBase->StringBufferA + ( F1Texture->ColorFilePathA - FHeader->StringBufferA ) ;
#endif
			Texture->ColorFilePathW = MBase->StringBufferW + ( F1Texture->ColorFilePathW - FHeader->StringBufferW ) ;
		}
		if( Texture->AlphaImage )
		{
#ifndef UNICODE
			Texture->AlphaFilePathA = MBase->StringBufferA + ( F1Texture->AlphaFilePathA - FHeader->StringBufferA ) ;
#endif
			Texture->AlphaFilePathW = MBase->StringBufferW + ( F1Texture->AlphaFilePathW - FHeader->StringBufferW ) ;
		}

		// アドレッシングモードのセット
		Texture->AddressModeU = F1Texture->AddressModeU ;
		Texture->AddressModeV = F1Texture->AddressModeV ;

		// ＵＶのスケール値をセット
		Texture->ScaleU = F1Texture->ScaleU ;
		Texture->ScaleV = F1Texture->ScaleV ;

		// フィルタリングモードのセット
		Texture->FilterMode = F1Texture->FilterMode ;

		// バンプマップ情報を保存
		Texture->BumpImageFlag = F1Texture->BumpImageFlag ;
		Texture->BumpImageNextPixelLength = F1Texture->BumpImageNextPixelLength ;

		// 画像のサイズを取得
		NS_GetGraphSize( Texture->GraphHandle, &Texture->Width, &Texture->Height ) ;
	}

	// マテリアルの情報をセット
	Material = MBase->Material ;
	F1Material = FHeader->Material ;
	for( i = 0 ; i < MBase->MaterialNum ; i ++, Material ++, F1Material ++ )
	{
#ifndef UNICODE
		Material->NameA = MBase->StringBufferA + ( F1Material->NameA - FHeader->StringBufferA ) ;
#endif
		Material->NameW = MBase->StringBufferW + ( F1Material->NameW - FHeader->StringBufferW ) ;

		Material->Diffuse  = F1Material->Diffuse ;
		Material->Ambient  = F1Material->Ambient ;
		Material->Specular = F1Material->Specular ;
		Material->Emissive = F1Material->Emissive ;
		Material->Power    = F1Material->Power ;

		Material->UseAlphaTest = F1Material->UseAlphaTest ;
		Material->AlphaFunc    = F1Material->AlphaFunc ;
		Material->AlphaRef     = F1Material->AlphaRef ;

		Material->DiffuseLayerNum = F1Material->DiffuseLayerNum ;
		MaterialLayer      = Material->DiffuseLayer ;
		F1MaterialLayer    = F1Material->DiffuseLayer ;
		for( j = 0 ; j < 8 ; j ++, MaterialLayer ++, F1MaterialLayer ++ )
		{
			MaterialLayer->BlendType = F1MaterialLayer->BlendType ;
			MaterialLayer->Texture = F1MaterialLayer->Texture ;
		}

		Material->SpecularLayerNum = F1Material->SpecularLayerNum ;
		MaterialLayer      = Material->SpecularLayer ;
		F1MaterialLayer    = F1Material->SpecularLayer ;
		for( j = 0 ; j < 8 ; j ++, MaterialLayer ++, F1MaterialLayer ++ )
		{
			MaterialLayer->BlendType = F1MaterialLayer->BlendType ;
			MaterialLayer->Texture = F1MaterialLayer->Texture ;
		}

		Material->NormalLayerNum = F1Material->NormalLayerNum ;
		MaterialLayer      = Material->NormalLayer ;
		F1MaterialLayer    = F1Material->NormalLayer ;
		for( j = 0 ; j < 8 ; j ++, MaterialLayer ++, F1MaterialLayer ++ )
		{
			MaterialLayer->BlendType = F1MaterialLayer->BlendType ;
			MaterialLayer->Texture = F1MaterialLayer->Texture ;
		}

		Material->DrawBlendMode = F1Material->DrawBlendMode ;
		Material->DrawBlendParam = F1Material->DrawBlendParam ;

		Material->Type = F1Material->Type ;
		Material->DiffuseGradBlendType = F1Material->DiffuseGradBlendType ;
		Material->DiffuseGradTexture = F1Material->DiffuseGradTexture ;
		Material->SphereMapTexture = F1Material->SphereMapTexture ;
		Material->SpecularGradBlendType = F1Material->SpecularGradBlendType ;
		Material->SpecularGradTexture = F1Material->SpecularGradTexture ;
		Material->SphereMapBlendType = F1Material->SphereMapBlendType ;
		Material->OutLineColor = F1Material->OutLineColor ;
		Material->OutLineWidth = F1Material->OutLineWidth ;
		Material->OutLineDotWidth = F1Material->OutLineDotWidth ;
	}

	// ライトの情報をセット
	Light = MBase->Light ;
	F1Light = FHeader->Light ;
	for( i = 0 ; i < MBase->LightNum ; i ++, Light ++, F1Light ++ )
	{
#ifndef UNICODE
		Light->NameA = MBase->StringBufferA + ( F1Light->NameA - FHeader->StringBufferA ) ;
#endif
		Light->NameW = MBase->StringBufferW + ( F1Light->NameW - FHeader->StringBufferW ) ;

		Light->Index = F1Light->Index ;
		Light->FrameIndex = F1Light->FrameIndex ;
		Light->Type = F1Light->Type ;
		Light->Diffuse = F1Light->Diffuse ;
		Light->Specular = F1Light->Specular ;
		Light->Ambient = F1Light->Ambient ;
		Light->Range = F1Light->Range ;
		Light->Falloff = F1Light->Falloff ;
		Light->Attenuation0 = F1Light->Attenuation0 ;
		Light->Attenuation1 = F1Light->Attenuation1 ;
		Light->Attenuation2 = F1Light->Attenuation2 ;
		Light->Theta = F1Light->Theta ;
		Light->Phi = F1Light->Phi ;
	}

	// メッシュの情報をセット
	Mesh = MBase->Mesh ;
	F1Mesh = FHeader->Mesh ;
	for( i = 0 ; i < MBase->MeshNum ; i ++, Mesh ++, F1Mesh ++ )
	{
		if( F1Mesh->Container )
		{
			Mesh->Container = MBase->Frame + F1Mesh->Container->Index ;
		}
		if( F1Mesh->Material )
		{
			Mesh->Material = MBase->Material + ( F1Mesh->Material - FHeader->Material ) ;
		}

		Mesh->ChangeInfo.Target   = F1Mesh->ChangeInfo.Target ;
		Mesh->ChangeInfo.Fill     = NULL ;
		Mesh->ChangeInfo.Size     = F1Mesh->ChangeInfo.Size ;
		Mesh->ChangeInfo.CheckBit = F1Mesh->ChangeInfo.CheckBit ;

		Mesh->NotOneDiffuseAlpha = F1Mesh->NotOneDiffuseAlpha ;
		Mesh->Shape = F1Mesh->Shape ;
		Mesh->UseVertexDiffuseColor = F1Mesh->UseVertexDiffuseColor ;
		Mesh->UseVertexSpecularColor = F1Mesh->UseVertexSpecularColor ;

		Mesh->TriangleListNum = F1Mesh->TriangleListNum ;
		if( F1Mesh->TriangleList )
		{
			Mesh->TriangleList = MBase->TriangleList + ( F1Mesh->TriangleList - FHeader->TriangleList ) ;
		}

		Mesh->Visible = F1Mesh->Visible ;
		Mesh->BackCulling = F1Mesh->BackCulling ;

		Mesh->UVSetUnitNum = F1Mesh->UVSetUnitNum ;
		Mesh->UVUnitNum = F1Mesh->UVUnitNum ;

		Mesh->VertexNum = F1Mesh->VertexNum ;
		Mesh->FaceNum = 0 ;

		Mesh->VertUnitSize = ( int )( sizeof( MV1_MESH_VERTEX ) + Mesh->UVSetUnitNum * Mesh->UVUnitNum * sizeof( float ) - sizeof( float ) * 2 ) ;

		// 頂点データを取得する
		{
			Mesh->Vertex = ( MV1_MESH_VERTEX * )( ( BYTE * )MBase->MeshVertex + MBase->MeshVertexSize ) ;
			Mesh->Face   = MBase->MeshFace + MBase->MeshFaceNum ;
			MBase->MeshVertexSize += Mesh->VertexNum * Mesh->VertUnitSize ;
			MBase->MeshFaceNum    += F1Mesh->FaceNum ;
		}
	}

	if( FHeader->Shape )
	{
		// シェイプの情報をセット
		Shape = MBase->Shape ;
		F1Shape = FHeader->Shape ;
		for( i = 0 ; i < MBase->ShapeNum ; i ++, Shape ++, F1Shape ++ )
		{
			if( F1Shape->Container )
			{
				Shape->Container = MBase->Frame + F1Shape->Container->Index ;
			}

#ifndef UNICODE
			Shape->NameA = MBase->StringBufferA + ( F1Shape->NameA - FHeader->StringBufferA ) ;
#endif
			Shape->NameW = MBase->StringBufferW + ( F1Shape->NameW - FHeader->StringBufferW ) ;

			Shape->MeshNum = F1Shape->MeshNum ;

			if( F1Shape->Mesh )
			{
				Shape->Mesh = MBase->ShapeMesh + ( F1Shape->Mesh - FHeader->ShapeMesh ) ;
			}
		}

		// シェイプメッシュの情報をセット
		ShapeMesh = MBase->ShapeMesh ;
		F1ShapeMesh = FHeader->ShapeMesh ;
		for( i = 0 ; i < MBase->ShapeMeshNum ; i ++, ShapeMesh ++, F1ShapeMesh ++ )
		{
			if( F1ShapeMesh->TargetMesh )
			{
				ShapeMesh->TargetMesh = MBase->Mesh + ( F1ShapeMesh->TargetMesh - FHeader->Mesh ) ;
			}

			ShapeMesh->VertexNum = F1ShapeMesh->VertexNum ;

			if( F1ShapeMesh->Vertex )
			{
				ShapeMesh->Vertex = MBase->ShapeVertex + MBase->ShapeVertexNum ;
				MBase->ShapeVertexNum += ShapeMesh->VertexNum ;
			}
		}
	}

	if( FHeader->PhysicsRigidBody )
	{
		// 剛体の情報をセット
		PhysicsRigidBody = MBase->PhysicsRigidBody ;
		F1PhysicsRigidBody = FHeader->PhysicsRigidBody ;
		for( i = 0 ; i < MBase->PhysicsRigidBodyNum ; i ++, PhysicsRigidBody ++, F1PhysicsRigidBody ++ )
		{
			PhysicsRigidBody->Index = F1PhysicsRigidBody->Index ;

#ifndef UNICODE
			PhysicsRigidBody->NameA = MBase->StringBufferA + ( F1PhysicsRigidBody->NameA - FHeader->StringBufferA ) ;
#endif
			PhysicsRigidBody->NameW = MBase->StringBufferW + ( F1PhysicsRigidBody->NameW - FHeader->StringBufferW ) ;

			if( F1PhysicsRigidBody->TargetFrame )
			{
				PhysicsRigidBody->TargetFrame = MBase->Frame + F1PhysicsRigidBody->TargetFrame->Index ;
			}

			PhysicsRigidBody->RigidBodyGroupIndex = F1PhysicsRigidBody->RigidBodyGroupIndex ;
			PhysicsRigidBody->RigidBodyGroupTarget = F1PhysicsRigidBody->RigidBodyGroupTarget ;
			PhysicsRigidBody->ShapeType = F1PhysicsRigidBody->ShapeType ;
			PhysicsRigidBody->ShapeW = F1PhysicsRigidBody->ShapeW ;
			PhysicsRigidBody->ShapeH = F1PhysicsRigidBody->ShapeH ;
			PhysicsRigidBody->ShapeD = F1PhysicsRigidBody->ShapeD ;
			PhysicsRigidBody->Position = F1PhysicsRigidBody->Position ;
			PhysicsRigidBody->Rotation = F1PhysicsRigidBody->Rotation ;
			PhysicsRigidBody->RigidBodyWeight = F1PhysicsRigidBody->RigidBodyWeight ;
			PhysicsRigidBody->RigidBodyPosDim = F1PhysicsRigidBody->RigidBodyPosDim ;
			PhysicsRigidBody->RigidBodyRotDim = F1PhysicsRigidBody->RigidBodyRotDim ;
			PhysicsRigidBody->RigidBodyRecoil = F1PhysicsRigidBody->RigidBodyRecoil ;
			PhysicsRigidBody->RigidBodyFriction = F1PhysicsRigidBody->RigidBodyFriction ;
			PhysicsRigidBody->RigidBodyType = F1PhysicsRigidBody->RigidBodyType ;
			PhysicsRigidBody->NoCopyToBone = F1PhysicsRigidBody->NoCopyToBone ;
		}

		// 剛体のジョイント情報をセット
		PhysicsJoint = MBase->PhysicsJoint ;
		F1PhysicsJoint = FHeader->PhysicsJoint ;
		for( i = 0 ; i < MBase->PhysicsJointNum ; i ++, PhysicsJoint ++, F1PhysicsJoint ++ )
		{
			PhysicsJoint->Index = F1PhysicsJoint->Index ;

#ifndef UNICODE
			PhysicsJoint->NameA = MBase->StringBufferA + ( F1PhysicsJoint->NameA - FHeader->StringBufferA ) ;
#endif
			PhysicsJoint->NameW = MBase->StringBufferW + ( F1PhysicsJoint->NameW - FHeader->StringBufferW ) ;

			if( F1PhysicsJoint->RigidBodyA )
			{
				PhysicsJoint->RigidBodyA = MBase->PhysicsRigidBody + F1PhysicsJoint->RigidBodyA->Index ;
			}

			if( F1PhysicsJoint->RigidBodyB )
			{
				PhysicsJoint->RigidBodyB = MBase->PhysicsRigidBody + F1PhysicsJoint->RigidBodyB->Index ;
			}

			PhysicsJoint->Position = F1PhysicsJoint->Position ;
			PhysicsJoint->Rotation = F1PhysicsJoint->Rotation ;
			PhysicsJoint->ConstrainPosition1 = F1PhysicsJoint->ConstrainPosition1 ;
			PhysicsJoint->ConstrainPosition2 = F1PhysicsJoint->ConstrainPosition2 ;
			PhysicsJoint->ConstrainRotation1 = F1PhysicsJoint->ConstrainRotation1 ;
			PhysicsJoint->ConstrainRotation2 = F1PhysicsJoint->ConstrainRotation2 ;
			PhysicsJoint->SpringPosition = F1PhysicsJoint->SpringPosition ;
			PhysicsJoint->SpringRotation = F1PhysicsJoint->SpringRotation ;
		}
	}

	// スキニングメッシュ用のボーン情報をセット
	SkinBone = MBase->SkinBone ;
	F1SkinBone = FHeader->SkinBone ;
	for( i = 0 ; i < MBase->SkinBoneNum ; i ++, SkinBone ++, F1SkinBone ++ )
	{
		SkinBone->BoneFrame = F1SkinBone->BoneFrame ;
		SkinBone->ModelLocalMatrix = F1SkinBone->ModelLocalMatrix ;
		SkinBone->ModelLocalMatrixIsTranslateOnly = F1SkinBone->ModelLocalMatrixIsTranslateOnly ;
		SkinBone->UseFrameNum = F1SkinBone->UseFrameNum ;
		if( F1SkinBone->UseFrame )
		{
			SkinBone->UseFrame = MBase->SkinBoneUseFrame + ( F1SkinBone->UseFrame - FHeader->SkinBoneUseFrame ) ;
		}
	}

	// トライアングルリストの情報をセット
	TriangleList = MBase->TriangleList ;
	F1TriangleList = FHeader->TriangleList ;
	for( i = 0 ; i < MBase->TriangleListNum ; i ++, TriangleList ++, F1TriangleList ++ )
	{
		if( F1TriangleList->Container )
		{
			TriangleList->Container = MBase->Mesh + ( F1TriangleList->Container - FHeader->Mesh ) ;
		}
		TriangleList->VertexType = F1TriangleList->VertexType ;
		TriangleList->VertexNum = F1TriangleList->VertexNum ;
		TriangleList->IndexNum = F1TriangleList->IndexNum ;

		// 頂点データの読み込み
		{
			TriangleList->Index = MBase->TriangleListIndex + MBase->TriangleListIndexNum ;
			TriangleList->ToonOutLineIndex = TriangleList->Index + FHeader->TriangleListIndexNum ;
			TriangleList->ToonOutLineIndexNum = 0 ;
			TriangleList->MeshVertexIndex   = MBase->MeshVertexIndex   + MBase->MeshVertexIndexNum ;
			MBase->TriangleListIndexNum += TriangleList->IndexNum ;
			MBase->MeshVertexIndexNum   += TriangleList->VertexNum ;

			// ボーンの情報を取得
			switch( TriangleList->VertexType )
			{
			case MV1_VERTEX_TYPE_SKIN_4BONE :
			case MV1_VERTEX_TYPE_SKIN_8BONE :
				TriangleList->UseBoneNum = F1TriangleList->UseBoneNum ;
				for( j = 0 ; j < TriangleList->UseBoneNum ; j ++ )
				{
					TriangleList->UseBone[ j ] = F1TriangleList->UseBone[ j ] ;
				}
				break ;

			case MV1_VERTEX_TYPE_SKIN_FREEBONE :
				TriangleList->MaxBoneNum = F1TriangleList->MaxBoneNum ;
				break ;
			}

			// 高速アクセス用の頂点座標データをセット
			switch( TriangleList->VertexType )
			{
			case MV1_VERTEX_TYPE_NORMAL :
				TriangleList->NormalPosition = MBase->TriangleListNormalPosition + MBase->TriangleListNormalPositionNum ;
				TriangleList->NormalPosition = ( MV1_TLIST_NORMAL_POS * )( ( ( DWORD_PTR )TriangleList->NormalPosition + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = sizeof( MV1_TLIST_NORMAL_POS ) ;
				MBase->TriangleListNormalPositionNum += TriangleList->VertexNum ;
				break ;

			case MV1_VERTEX_TYPE_SKIN_4BONE :
				TriangleList->SkinPosition4B = MBase->TriangleListSkinPosition4B + MBase->TriangleListSkinPosition4BNum ;
				TriangleList->SkinPosition4B = ( MV1_TLIST_SKIN_POS_4B * )( ( ( DWORD_PTR )TriangleList->SkinPosition4B + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = sizeof( MV1_TLIST_SKIN_POS_4B ) ;
				MBase->TriangleListSkinPosition4BNum += TriangleList->VertexNum ;
				break ;

			case MV1_VERTEX_TYPE_SKIN_8BONE :
				TriangleList->SkinPosition8B = MBase->TriangleListSkinPosition8B + MBase->TriangleListSkinPosition8BNum ;
				TriangleList->SkinPosition8B = ( MV1_TLIST_SKIN_POS_8B * )( ( ( DWORD_PTR )TriangleList->SkinPosition8B + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = sizeof( MV1_TLIST_SKIN_POS_8B ) ;
				MBase->TriangleListSkinPosition8BNum += TriangleList->VertexNum ;
				break ;

			case MV1_VERTEX_TYPE_SKIN_FREEBONE :
				TriangleList->SkinPositionFREEB = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )MBase->TriangleListSkinPositionFREEB + MBase->TriangleListSkinPositionFREEBSize ) ;
				TriangleList->SkinPositionFREEB = ( MV1_TLIST_SKIN_POS_FREEB * )( ( ( DWORD_PTR )TriangleList->SkinPositionFREEB + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = ( unsigned short )( sizeof( MV1_TLIST_SKIN_POS_FREEB ) + sizeof( MV1_SKINBONE_BLEND ) * ( TriangleList->MaxBoneNum - 4 ) ) ;
				TriangleList->PosUnitSize = ( unsigned short )( ( TriangleList->PosUnitSize + 15 ) / 16 * 16 ) ;
				MBase->TriangleListSkinPositionFREEBSize += TriangleList->PosUnitSize * TriangleList->VertexNum ;
				break ;
			}
//			MV1SetupTriangleListPositionAndNormal( TriangleList ) ;
		}
	}

	// メッシュのフェイス情報を構築する
	{
		WORD *Ind ;
		DWORD *MInd, TListInd ;

		TriangleList = MBase->TriangleList ;
		for( i = 0 ; i < MBase->TriangleListNum ; i ++, TriangleList ++ )
		{
			Mesh = TriangleList->Container ;
			Ind = TriangleList->Index ;
			MInd = TriangleList->MeshVertexIndex ;
			TListInd = ( DWORD )( TriangleList - Mesh->TriangleList ) ;
			for( j = 0 ; j < TriangleList->IndexNum ; j += 3, Ind += 3 )
			{
				Face = &Mesh->Face[ Mesh->FaceNum ] ;
				Face->TriangleListIndex = ( WORD )TListInd ;
				Face->VertexType = TriangleList->VertexType ;
				Face->VertexIndex[ 0 ] = MInd[ Ind[ 0 ] ] ;
				Face->VertexIndex[ 1 ] = MInd[ Ind[ 1 ] ] ;
				Face->VertexIndex[ 2 ] = MInd[ Ind[ 2 ] ] ;
				Mesh->FaceNum ++ ;
			}
		}
	}

	// 法線の再生成の指定がある場合は生成を行う
	if( MV1Man.LoadModelToReMakeNormal )
	{
		// 法線の自動生成
		{
			VECTOR Nrm ;
			MV1_MESH_NORMAL *MNrm ;
			MV1_MESH_VERTEX *Vertex[ 3 ] ;
			DWORD VertUnitSize, PosUnitSize ;

			// 全てのフレームを処理
			Frame = MBase->Frame ;
			for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++ )
			{
				if( Frame->AutoCreateNormal == 0 ) continue ;

				PosUnitSize = ( DWORD )Frame->PosUnitSize ;

				// 面の法線を算出しながら足していく
				Mesh = Frame->Mesh ;
				for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
				{
					VertUnitSize = ( DWORD )Mesh->VertUnitSize ;

					Face = Mesh->Face ;
					for( k = 0 ; k < Mesh->FaceNum ; k ++, Face ++ )
					{
						Vertex[ 0 ] = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + VertUnitSize * Face->VertexIndex[ 0 ] ) ;
						Vertex[ 1 ] = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + VertUnitSize * Face->VertexIndex[ 1 ] ) ;
						Vertex[ 2 ] = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + VertUnitSize * Face->VertexIndex[ 2 ] ) ;

						Nrm = VNorm( VCross( 
							VSub( ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 1 ]->PositionIndex ) )->Position, 
								  ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 0 ]->PositionIndex ) )->Position ),
							VSub( ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 2 ]->PositionIndex ) )->Position, 
								  ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 0 ]->PositionIndex ) )->Position ) ) ) ;

						MNrm = &Frame->Normal[ Vertex[ 0 ]->NormalIndex ] ;
						MNrm->Normal = VAdd( MNrm->Normal, Nrm ) ;

						MNrm = &Frame->Normal[ Vertex[ 1 ]->NormalIndex ] ;
						MNrm->Normal = VAdd( MNrm->Normal, Nrm ) ;

						MNrm = &Frame->Normal[ Vertex[ 2 ]->NormalIndex ] ;
						MNrm->Normal = VAdd( MNrm->Normal, Nrm ) ;
					}
				}

				// 法線を正規化する
				MNrm = Frame->Normal ;
				for( j = 0 ; j < Frame->NormalNum ; j ++, MNrm ++ )
				{
					MNrm->Normal = VNorm( MNrm->Normal ) ;
				}
			}
		}

		// 接線と従法線の構築
		Mesh = MBase->Mesh ;
		for( i = 0 ; i < MBase->MeshNum ; i ++, Mesh ++ )
		{
			MV1MakeMeshBinormalsAndTangents( Mesh ) ;
		}
	}

	// アニメーションキーセットの情報をセット
	AnimKeySet = MBase->AnimKeySet ;
	F1AnimKeySet = FHeader->AnimKeySet ;
	for( i = 0 ; i < MBase->AnimKeySetNum ; i ++, AnimKeySet ++, F1AnimKeySet ++ )
	{
		AnimKeySet->Type = F1AnimKeySet->Type ;
		AnimKeySet->DataType = F1AnimKeySet->DataType ;

		AnimKeySet->TargetShapeIndex = F1AnimKeySet->TargetShapeIndex ;

		if( F1AnimKeySet->TimeType == MV1_ANIMKEY_TIME_TYPE_ONE )
		{
			AnimKeySet->TimeType = MV1_ANIMKEY_TIME_TYPE_ONE ;
			AnimKeySet->StartTime = F1AnimKeySet->StartTime ;
			AnimKeySet->UnitTime = F1AnimKeySet->UnitTime ;
		}
		else
		{
			AnimKeySet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
			AnimKeySet->KeyTime = ( float * )( ( DWORD_PTR )MBase->AnimKeyData + ( ( DWORD_PTR )F1AnimKeySet->KeyTime - ( DWORD_PTR )FHeader->AnimKeyData ) ) ;
		}
		AnimKeySet->Num = F1AnimKeySet->Num ;

		AnimKeySet->KeyLinear = ( float * )( ( DWORD_PTR )MBase->AnimKeyData  + ( ( DWORD_PTR )F1AnimKeySet->KeyLinear - ( DWORD_PTR )FHeader->AnimKeyData ) ) ;
	}

	// アニメーションセットの情報をセット
	AnimSet = MBase->AnimSet ;
	F1AnimSet = FHeader->AnimSet ;
	for( i = 0 ; i < MBase->AnimSetNum ; i ++, AnimSet ++, F1AnimSet ++ )
	{
#ifndef UNICODE
		AnimSet->NameA = MBase->StringBufferA + ( F1AnimSet->NameA - FHeader->StringBufferA ) ;
#endif
		AnimSet->NameW = MBase->StringBufferW + ( F1AnimSet->NameW - FHeader->StringBufferW ) ;

		AnimSet->Index = F1AnimSet->Index ;
		AnimSet->MaxTime = F1AnimSet->MaxTime ;
		AnimSet->AnimNum = F1AnimSet->AnimNum ;
		if( F1AnimSet->Anim )
		{
			AnimSet->Anim = MBase->Anim + ( F1AnimSet->Anim - FHeader->Anim ) ;
		}
		AnimSet->IsAddAnim = F1AnimSet->IsAddAnim ;
		AnimSet->IsMatrixLinearBlend = F1AnimSet->IsMatrixLinearBlend ;
	}

	// アニメーションの情報をセット
	Anim = MBase->Anim ;
	F1Anim = FHeader->Anim ;
	for( i = 0 ; i < MBase->AnimNum ; i ++, Anim ++, F1Anim ++ )
	{
		if( F1Anim->Container )
		{
			Anim->Container = MBase->AnimSet + F1Anim->Container->Index ;
		}

		Anim->TargetFrame = MBase->Frame + F1Anim->TargetFrameIndex ;
		Anim->TargetFrameIndex = F1Anim->TargetFrameIndex ;
		Anim->MaxTime = F1Anim->MaxTime ;
		Anim->RotateOrder = F1Anim->RotateOrder ;
		Anim->KeySetNum = F1Anim->KeySetNum ;
		Anim->KeySet = MBase->AnimKeySet + ( F1Anim->KeySet - FHeader->AnimKeySet ) ;

		MBase->AnimTargetFrameTable[ Anim->Container->Index * MBase->FrameNum + Anim->TargetFrameIndex ] = Anim ;
	}

	// 行列のセットアップ
	MV1SetupInitializeMatrixBase( MBase ) ;

	// メッシュの半透明かどうかの情報をセットアップする
	MV1SetupMeshSemiTransStateBase( MBase ) ;

	// 同時複数描画関係の情報をセットアップする
	if( MBase->UsePackDraw )
	{
		MV1SetupPackDrawInfo( MBase ) ;
	}

	// メモリの解放
	//DXFREE( FHeader ) ;

	// 指定がある場合は座標の最適化を行う
	if( MV1Man.LoadModelToPositionOptimize )
	{
		MV1PositionOptimizeBase( NewHandle ) ;
	}

	// 指定がある場合は法線の再計算を行う
	if( MV1Man.LoadModelToReMakeNormal )
	{
		MV1ReMakeNormalBase( NewHandle, MV1Man.LoadModelToReMakeNormalSmoothingAngle ) ;
	}

	// 高速処理用頂点データの構築
	TriangleList = MBase->TriangleList ;
	for( i = 0 ; i < MBase->TriangleListNum ; i ++, TriangleList ++ )
	{
		MV1SetupTriangleListPositionAndNormal( TriangleList ) ;
		MV1SetupToonOutLineTriangleList( TriangleList ) ;
	}

	// 正常終了
	return NewHandle ;

	// エラー処理
ERRORLABEL :
	if( NewHandle )
	{
		MV1SubModelBase( NewHandle ) ;
		NewHandle = 0 ;
	}

	// エラー終了
	return -1 ;
}



// 初期セットアップ系

// 初期状態の変換行列をセットアップする
extern void MV1SetupInitializeMatrixBase( MV1_MODEL_BASE *ModelBase )
{
	MV1_FRAME_BASE *MBFrame ;
	MATRIX TempMatrix ;
	int i ;

	MBFrame = ModelBase->Frame ;
	for( i = 0 ; i < ModelBase->FrameNum ; i ++, MBFrame ++ )
	{
		// スケーリングを使用しているかどうかのフラグをセットする
		MBFrame->LocalTransformMatrixUseScaling =
			MBFrame->Scale.x < 0.9999999f || MBFrame->Scale.x > 1.0000001f ||
			MBFrame->Scale.y < 0.9999999f || MBFrame->Scale.y > 1.0000001f ||
			MBFrame->Scale.z < 0.9999999f || MBFrame->Scale.z > 1.0000001f;

		// 単位行列かどうかを調べる
		if( ( *( ( DWORD * )&MBFrame->Rotate.x ) & 0x7fffffff ) == 0 &&
			( *( ( DWORD * )&MBFrame->Rotate.y ) & 0x7fffffff ) == 0 &&
			( *( ( DWORD * )&MBFrame->Rotate.z ) & 0x7fffffff ) == 0 &&
			MBFrame->LocalTransformMatrixUseScaling == false )
		{
			if( ( *( ( DWORD * )&MBFrame->Translate.x ) & 0x7fffffff ) == 0 &&
				( *( ( DWORD * )&MBFrame->Translate.y ) & 0x7fffffff ) == 0 &&
				( *( ( DWORD * )&MBFrame->Translate.z ) & 0x7fffffff ) == 0 )
			{
				MBFrame->LocalTransformMatrixType = 0 ;
				CreateIdentityMatrix( &TempMatrix ) ;
			}
			else
			{
				MBFrame->LocalTransformMatrixType = 1 ;
				CreateTranslationMatrix( &TempMatrix, ( float )MBFrame->Translate.x, ( float )MBFrame->Translate.y, ( float )MBFrame->Translate.z ) ;
			}
		}
		else
		{
			MBFrame->LocalTransformMatrixType = 2 ;

			// デフォルト座標値を元に変換行列を作成する
			MV1RMakeMatrix(
				( MBFrame->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &MBFrame->PreRotate : NULL,
				&MBFrame->Rotate,
				( MBFrame->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &MBFrame->PostRotate : NULL,
				&MBFrame->Scale,
				&MBFrame->Translate,
				NULL,
				&TempMatrix,
				MBFrame->RotateOrder
			) ;
		}
		ConvertMatrixFToMatrix4x4cF( &MBFrame->LocalTransformMatrix, &TempMatrix ) ;

		// 初期のローカル→ワールド行列を作成
		if( MBFrame->Parent == NULL )
		{
			ConvertMatrix4x4cFToMatrixF( &TempMatrix, &MBFrame->LocalTransformMatrix ) ;
			MBFrame->TransformMatrix = MBFrame->LocalTransformMatrix ;
		}
		else
		{
			MATRIX LocalMatrix, ParentMatrix ;

			ConvertMatrix4x4cFToMatrixF( &LocalMatrix,  &MBFrame->LocalTransformMatrix ) ;
			ConvertMatrix4x4cFToMatrixF( &ParentMatrix, &MBFrame->Parent->TransformMatrix ) ;
			CreateMultiplyMatrix( &TempMatrix, &LocalMatrix, &ParentMatrix ) ;
			ConvertMatrixFToMatrix4x4cF( &MBFrame->TransformMatrix, &TempMatrix ) ;
		}
		CreateInverseMatrix( &TempMatrix, &TempMatrix ) ;
		ConvertMatrixFToMatrix4x4cF( &MBFrame->InverseTransformMatrix, &TempMatrix ) ;
	}
}

// 同時複数描画関係の情報をセットアップする
extern void MV1SetupPackDrawInfo( MV1_MODEL_BASE *ModelBase )
{
	MV1_TRIANGLE_LIST_BASE *MBTList ;
	int i ;
	int UseBoneNum ;

	// トライアングルリストが無い場合は適用できない
	if( ModelBase->TriangleListNum == 0 )
	{
		ModelBase->UsePackDraw = FALSE ;
		return ;
	}

	// 頂点シェーダーが使用できない場合は同時複数描画は適用できない
	if( GSYS.HardInfo.UseShader == FALSE )
	{
		ModelBase->UsePackDraw = FALSE ;
		return ;
	}

	// １トライアングルリストで同時に使用するボーン数が８より多いものがある場合は同時複数描画は適用できない
	if( ModelBase->TriangleListSkinPositionFREEBSize > 0 )
	{
		ModelBase->UsePackDraw = FALSE ;
		return ;
	}

	// シェイプが使用されているモデルでは同時複数描画の適用はできない
	if( ModelBase->ShapeNum > 0 )
	{
		ModelBase->UsePackDraw = FALSE ;
		return ;
	}

	// 最小・最大頂点数と最小・最大使用行列数を取得する
	MBTList = ModelBase->TriangleList ;
	ModelBase->TriangleListMaxVertexNum = 0 ;
	ModelBase->TriangleListMinVertexNum = 0x7fffffff ;
	ModelBase->TriangleListMaxIndexNum = 0 ;
	ModelBase->TriangleListMinIndexNum = 0x7fffffff ;
	ModelBase->TriangleListMaxMatrixNum = 0 ;
	ModelBase->TriangleListMinMatrixNum = 0x7fffffff ;
	for( i = 0 ; i < ModelBase->TriangleListNum ; i ++, MBTList ++ )
	{
		switch( MBTList->VertexType )
		{
		case MV1_VERTEX_TYPE_NORMAL :
			UseBoneNum = 1 ;
			break ;

		case MV1_VERTEX_TYPE_SKIN_FREEBONE :
			continue ;

		default :
			UseBoneNum = MBTList->UseBoneNum ;
			break ;
		}

		if( UseBoneNum > ModelBase->TriangleListMaxMatrixNum )
		{
			ModelBase->TriangleListMaxMatrixNum = UseBoneNum ;
		}

		if( UseBoneNum < ModelBase->TriangleListMinMatrixNum )
		{
			ModelBase->TriangleListMinMatrixNum = UseBoneNum ;
		}

		if( MBTList->IndexNum + MBTList->ToonOutLineIndexNum > ModelBase->TriangleListMaxIndexNum )
		{
			ModelBase->TriangleListMaxIndexNum = MBTList->IndexNum + MBTList->ToonOutLineIndexNum ;
		}

		if( MBTList->IndexNum + MBTList->ToonOutLineIndexNum < ModelBase->TriangleListMinIndexNum )
		{
			ModelBase->TriangleListMinIndexNum = MBTList->IndexNum + MBTList->ToonOutLineIndexNum ;
		}

		if( MBTList->VertexNum > ModelBase->TriangleListMaxVertexNum )
		{
			ModelBase->TriangleListMaxVertexNum = MBTList->VertexNum ;
		}

		if( MBTList->VertexNum < ModelBase->TriangleListMinVertexNum )
		{
			ModelBase->TriangleListMinVertexNum = MBTList->VertexNum ;
		}
	}

	// 頂点やインデックスが一つも無い場合は適用できない
	if( ModelBase->TriangleListMaxVertexNum == 0 ||
		ModelBase->TriangleListMaxIndexNum  == 0 ||
		ModelBase->TriangleListMaxMatrixNum == 0 )
	{
		ModelBase->UsePackDraw = FALSE ;
		return ;
	}

	// 同時複数描画関係の情報のセットアップの環境依存処理
	MV1_SetupPackDrawInfo_PF( ModelBase ) ;

	// 同時複数描画の一描画分で使用する行列の数をセットする
	ModelBase->PackDrawMatrixUnitNum = ModelBase->SkinBoneNum + ModelBase->FrameNum ;
}

// トゥーン輪郭線用のメッシュを作成する
//#define CHECKPARAM	( 0.125f / 2.0f )
#define CHECKPARAM	( 0.0f )
extern void MV1SetupToonOutLineTriangleList( MV1_TRIANGLE_LIST_BASE *MBTList )
{
	int i, tindNum ;
	unsigned short *ind ;
	unsigned short *tind ;
//	VECTOR FaceDir ;
	MV1_MESH_BASE *MBMesh ;

	// 頂点の数だけ繰り返し
	ind = MBTList->Index ;
	tind = MBTList->ToonOutLineIndex ;
	tindNum = 0 ;
	MBMesh = MBTList->Container ;
	for( i = 0 ; i < MBTList->IndexNum ; i += 3, ind += 3 )
	{
		if( ( ( MV1_MESH_VERTEX * )( ( BYTE * )MBMesh->Vertex + MBMesh->VertUnitSize * MBTList->MeshVertexIndex[ ind[ 0 ] ] ) )->ToonOutLineScale < 0.0001f ) continue ;
		if( ( ( MV1_MESH_VERTEX * )( ( BYTE * )MBMesh->Vertex + MBMesh->VertUnitSize * MBTList->MeshVertexIndex[ ind[ 1 ] ] ) )->ToonOutLineScale < 0.0001f ) continue ;
		if( ( ( MV1_MESH_VERTEX * )( ( BYTE * )MBMesh->Vertex + MBMesh->VertUnitSize * MBTList->MeshVertexIndex[ ind[ 2 ] ] ) )->ToonOutLineScale < 0.0001f ) continue ;

		tind[ 0 ] = ind[ 0 ] ;
		tind[ 1 ] = ind[ 1 ] ;
		tind[ 2 ] = ind[ 2 ] ;
		tind += 3 ;
		tindNum += 3 ;
	}
/*
	switch( MBTList->VertexType )
	{
	case MV1_VERTEX_TYPE_NORMAL :
		{
			MV1_TLIST_NORMAL_POS *Norm, *Norm1, *Norm2, *Norm3 ;

			Norm = ( MV1_TLIST_NORMAL_POS * )ADDR16( MBTList->NormalPosition ) ;
			for( i = 0 ; i < MBTList->IndexNum ; i += 3, ind += 3 )
			{
				Norm1 = &Norm[ ind[ 0 ] ] ;
				Norm2 = &Norm[ ind[ 1 ] ] ;
				Norm3 = &Norm[ ind[ 2 ] ] ;

				FaceDir =	VNorm
							(
								VCross
								(
									VSub
									(
										*( ( VECTOR * )&Norm2->Position ),
										*( ( VECTOR * )&Norm1->Position )
									),
									VSub
									(
										*( ( VECTOR * )&Norm3->Position ),
										*( ( VECTOR * )&Norm1->Position )
									)
								)
							) ;
				if( VDot( FaceDir, *( ( VECTOR * )&Norm1->Normal ) ) < CHECKPARAM ||
					VDot( FaceDir, *( ( VECTOR * )&Norm2->Normal ) ) < CHECKPARAM ||
					VDot( FaceDir, *( ( VECTOR * )&Norm3->Normal ) ) < CHECKPARAM )
				{
//					continue ;
				}

				tind[ 0 ] = ind[ 0 ] ;
				tind[ 1 ] = ind[ 1 ] ;
				tind[ 2 ] = ind[ 2 ] ;
				tind += 3 ;
				tindNum += 3 ;
			}
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_4BONE :
		{
			MV1_TLIST_SKIN_POS_4B *SK4, *SK41, *SK42, *SK43 ;

			SK4 = ( MV1_TLIST_SKIN_POS_4B * )ADDR16( MBTList->SkinPosition4B ) ;
			for( i = 0 ; i < MBTList->IndexNum ; i += 3, ind += 3 )
			{
				SK41 = &SK4[ ind[ 0 ] ] ;
				SK42 = &SK4[ ind[ 1 ] ] ;
				SK43 = &SK4[ ind[ 2 ] ] ;

				FaceDir =	VNorm
							(
								VCross
								(
									VSub
									(
										*( ( VECTOR * )&SK42->Position ),
										*( ( VECTOR * )&SK41->Position )
									),
									VSub
									(
										*( ( VECTOR * )&SK43->Position ),
										*( ( VECTOR * )&SK41->Position )
									)
								)
							) ;
				if( VDot( FaceDir, SK41->Normal ) < CHECKPARAM ||
					VDot( FaceDir, SK42->Normal ) < CHECKPARAM ||
					VDot( FaceDir, SK43->Normal ) < CHECKPARAM )
				{
//					continue ;
				}

				tind[ 0 ] = ind[ 0 ] ;
				tind[ 1 ] = ind[ 1 ] ;
				tind[ 2 ] = ind[ 2 ] ;
				tind += 3 ;
				tindNum += 3 ;
			}
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_8BONE :
		{
			MV1_TLIST_SKIN_POS_8B *SK8, *SK81, *SK82, *SK83 ;

			SK8 = ( MV1_TLIST_SKIN_POS_8B * )ADDR16( MBTList->SkinPosition8B ) ;
			for( i = 0 ; i < MBTList->IndexNum ; i += 3, ind += 3 )
			{
				SK81 = &SK8[ ind[ 0 ] ] ;
				SK82 = &SK8[ ind[ 1 ] ] ;
				SK83 = &SK8[ ind[ 2 ] ] ;

				FaceDir =	VNorm
							(
								VCross
								(
									VSub
									(
										SK82->Position,
										SK81->Position
									),
									VSub
									(
										SK83->Position,
										SK81->Position
									)
								)
							) ;
				if( VDot( FaceDir, SK81->Normal ) < CHECKPARAM ||
					VDot( FaceDir, SK82->Normal ) < CHECKPARAM ||
					VDot( FaceDir, SK83->Normal ) < CHECKPARAM )
				{
//					continue ;
				}

				tind[ 0 ] = ind[ 0 ] ;
				tind[ 1 ] = ind[ 1 ] ;
				tind[ 2 ] = ind[ 2 ] ;
				tind += 3 ;
				tindNum += 3 ;
			}
		}
		break ;

	case MV1_VERTEX_TYPE_SKIN_FREEBONE :
		{
			MV1_TLIST_SKIN_POS_8B *SK8, *SK81, *SK82, *SK83 ;

			SK8 = ( MV1_TLIST_SKIN_POS_8B * )ADDR16( MBTList->SkinPosition8B ) ;
			for( i = 0 ; i < MBTList->IndexNum ; i += 3, ind += 3 )
			{
				SK81 = ( MV1_TLIST_SKIN_POS_8B * )( ( BYTE * )SK8 + MBTList->PosUnitSize * ind[ 0 ] ) ;
				SK82 = ( MV1_TLIST_SKIN_POS_8B * )( ( BYTE * )SK8 + MBTList->PosUnitSize * ind[ 1 ] ) ;
				SK83 = ( MV1_TLIST_SKIN_POS_8B * )( ( BYTE * )SK8 + MBTList->PosUnitSize * ind[ 2 ] ) ;

				FaceDir =	VNorm
							(
								VCross
								(
									VSub
									(
										*( ( VECTOR * )&SK82->Position ),
										*( ( VECTOR * )&SK81->Position )
									),
									VSub
									(
										*( ( VECTOR * )&SK83->Position ),
										*( ( VECTOR * )&SK81->Position )
									)
								)
							) ;
				if( VDot( FaceDir, *( ( VECTOR * )&SK81->Normal ) ) < CHECKPARAM ||
					VDot( FaceDir, *( ( VECTOR * )&SK82->Normal ) ) < CHECKPARAM ||
					VDot( FaceDir, *( ( VECTOR * )&SK83->Normal ) ) < CHECKPARAM )
				{
//					continue ;
				}

				tind[ 0 ] = ind[ 0 ] ;
				tind[ 1 ] = ind[ 1 ] ;
				tind[ 2 ] = ind[ 2 ] ;
				tind += 3 ;
				tindNum += 3 ;
			}
		}
		break ;
	}
*/
	MBTList->ToonOutLineIndexNum = ( unsigned short )tindNum ;

	return ;
}

// モデル中のメッシュの半透明要素があるかどうかを調べる
extern void MV1SetupMeshSemiTransStateBase( MV1_MODEL_BASE *ModelBase )
{
	MV1_MESH_BASE     *MBMesh ;
	MV1_MATERIAL_BASE *MBMaterial ;
	MV1_TEXTURE_BASE  *MBTexture ;
	int               i ;

	// メッシュの数だけ繰り返し
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		do
		{
			MBMesh->SemiTransState = 1 ;

			MBMaterial = MBMesh->Material ;

			// 描画モードがアルファブレンド以外なら半透明
			if( MBMaterial->DrawBlendMode != DX_BLENDMODE_ALPHA )
				break ;

			// ブレンドパラメータが 255 以外なら半透明
			if( MBMaterial->DrawBlendParam != 255 )
				break ;

			// 頂点カラーを使用するかどうかで分岐
			if( MBMesh->UseVertexDiffuseColor )
			{
				// 頂点ディフューズカラーに 1.0 以外のアルファ値があったら半透明
				if( MBMesh->NotOneDiffuseAlpha )
					break ;
			}
			else
			{
				// マテリアルのディフューズカラーのアルファ値が１．０以外なら半透明
				if( MBMaterial->Diffuse.a != 1.0f )
					break ;
			}

			// 半透明テクスチャを使用していたら半透明
			if( MBMaterial->DiffuseLayerNum )
			{
				MBTexture = &ModelBase->Texture[ MBMaterial->DiffuseLayer[ 0 ].Texture ] ;
				if( MBTexture->SemiTransFlag )
				{
					break ;
				}
			}

			// ここにきたら半透明要素はないということ
			MBMesh->SemiTransState = 0 ;
		}while( 0 ) ;
	}
}

// モデル全体の法線を再計算する
extern int MV1ReMakeNormalBase( int MBHandle, float SmoothingAngle, int ASyncThread )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME_BASE *Frame ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// 頂点バッファを解放する
	MV1_TerminateVertexBufferBase_PF( MBHandle ) ;

	// フレームの数だけ繰り返し
	Frame = ModelBase->Frame ;
	for( i = 0 ; i < ModelBase->FrameNum ; i ++, Frame ++ )
	{
		if( Frame->MeshNum == 0 ) continue ;

		if( _MV1ReMakeNormalFrameBase( Frame, SmoothingAngle ) == -1 )
			return -1 ;
	}

	// 頂点バッファを構築する
	MV1_SetupVertexBufferBase_PF( MBHandle, 1, ASyncThread ) ;

	// 終了
	return 0 ;
}

// 指定フレームが持つメッシュの法線を再計算する
extern int MV1ReMakeNormalFrameBase( int MBHandle, int FrameIndex, float SmoothingAngle )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME_BASE *Frame ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// フレームインデックスのチェック
	if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
		return -1 ;
	Frame = ModelBase->Frame + FrameIndex ;

	// メッシュが無ければ何もせず終了
	if( Frame->MeshNum == 0 )
		return -1 ;

	// 頂点バッファを解放する
	MV1_TerminateVertexBufferBase_PF( MBHandle ) ;

	// フレームの再計算
	if( _MV1ReMakeNormalFrameBase( Frame, SmoothingAngle ) == -1 )
		return -1 ;

	// 頂点バッファを構築する
	MV1_SetupVertexBufferBase_PF( MBHandle ) ;

	// 終了
	return 0 ;
}

// フレームの法線を再計算する
static int _MV1ReMakeNormalFrameBase( MV1_FRAME_BASE *Frame, float SmoothingAngle )
{
	MV1_MODEL_BASE *ModelBase ;
	int FaceNum, NewVertexNum, NewTVertexNum, i, j, k, m ;
	MV1_TRIANGLE_LIST_BASE *TList ;
	MV1_MESH_BASE *Mesh, *TMesh, *CurMesh, *MinMesh ;
	MV1_MESH_VERTEX *NewVertex, *Vert ;
	MV1_MESH_FACE *Face, *FaceT, *OldFace = NULL, *OFace ;
	VECTOR *FaceNormal, *FNorm, Pos[ 3 ], TPos[ 3 ] ;
	int *FaceNormalIndex, *FNInd, *FNInd2, *MFNInd = NULL, Index, *VertValidBuffer ;
	int NormalNum, IndexNum, PosInd[ 3 ], TPosInd[ 3 ], MPosInd[ 3 ] ;
	VECTOR *P0, *P1, *P2, V1, V2, V3, Norm, tv, xv, yv, zv, FaceNorm ;
	VECTOR *NormalBuf = NULL ;
	MV1_MESH_NORMAL *RNormal ;
	BYTE *NormalSetFlag, *NormalUseTable ;
	DWORD *UseNormalIndex, *NewNormalIndex, *NewMeshVertexIndex ;
	MV1_MAKEVERTINDEXINFO **VertexFaceList = NULL, *VertexFaceBuffer, *VFBuf ;
	MV1_MESH_FACE *FaceList[ 1024 ], *CurFace, *MinFace, *BackCurFace ;
	float SmoothCos, Sin ;
	float MinFaceRadi, MinFaceRadi2, FaceRadi, FaceRadi2 ;
	int FaceCount, MinFaceRadi2Use, FaceRadi2Use, CurFaceIndex, Issyuu ;
	BYTE FaceIndex[ 1024 ] ;
	int UseNormalNum, MinFaceUra, FaceUra, CmpFlag, MinFaceIndex, VertInfoNum, NewShapeVertexNum ;
	MV1_MAKEVERTINDEXINFO **VertInfoTable, *VInfo ;
	MV1_MAKEVERTINDEXINFO *VertInfoBuffer ;
	MV1_SHAPE_VERTEX_BASE *ShapeFrameVertBuffer, *ShapeFrameVert, *ShapeVert, *NewShapeVertBuffer ;
	MV1_SHAPE_MESH_BASE *ShapeMesh ;
	MV1_SHAPE_BASE *Shape ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// メッシュが無ければ何もせずに終了
	if( Frame->MeshNum == 0 ) return 0 ;

	// ポインタの初期化
	ModelBase = Frame->Container ;
	FaceNormal = NULL ;
	FaceNormalIndex = NULL ;
	NewVertex = NULL ;
	VertInfoTable = NULL ;
	NewMeshVertexIndex = NULL ;
	NewShapeVertBuffer = NULL ;

	// スムージングを行う角度のコサイン値を求めておく
	Frame->SmoothingAngle = SmoothingAngle ;
	_SINCOS( Frame->SmoothingAngle, &Sin, &SmoothCos ) ;
	Frame->AutoCreateNormal = TRUE ;

	// 面の数を数える
	Mesh = Frame->Mesh ;
	FaceNum = 0 ;
	for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
		FaceNum += Mesh->FaceNum ;

	// 面の法線と面が使用する法線のインデックスと、シェイプ頂点のフレーム頂点との
	// 関連データを保存するメモリ領域の確保する
	FaceNormal = ( VECTOR * )DXALLOC(
		sizeof( VECTOR ) * FaceNum +
		sizeof( int ) * 3 * FaceNum +
		sizeof( MV1_SHAPE_VERTEX_BASE ) * Frame->PositionNum * Frame->ShapeNum ) ;
	if( FaceNormal == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xd5\x6c\xda\x7d\x8d\x51\x08\x8a\x97\x7b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"法線再計算用のメモリの確保に失敗しました\n" @*/ )) ;
		goto ERR ;
	}
	FaceNormalIndex = ( int * )( FaceNormal + FaceNum ) ;
	ShapeFrameVertBuffer = ( MV1_SHAPE_VERTEX_BASE * )( FaceNormalIndex + 3 * FaceNum ) ;

	// シェイプ情報とフレーム頂点の関連データを構築する
	if( Frame->ShapeNum )
	{
		_MEMSET( ShapeFrameVertBuffer, 0xff, sizeof( MV1_SHAPE_VERTEX_BASE ) * Frame->PositionNum * Frame->ShapeNum ) ;
		Shape = Frame->Shape ;
		for( i = 0 ; i < Frame->ShapeNum ; i ++, Shape ++ )
		{
			ShapeFrameVert = ShapeFrameVertBuffer + i * Frame->PositionNum ;
			ShapeMesh = Shape->Mesh ;
			for( j = 0 ; j < Shape->MeshNum ; j ++, ShapeMesh ++ )
			{
				Mesh = ShapeMesh->TargetMesh ;
				ShapeVert = ShapeMesh->Vertex ;
				for( k = 0 ; ( DWORD )k < ShapeMesh->VertexNum ; k ++, ShapeVert ++ )
				{
					Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + ShapeVert->TargetMeshVertex * Mesh->VertUnitSize ) ;
					ShapeFrameVert[ Vert->PositionIndex ] = *ShapeVert ;
				}
			}
		}
	}

	// 面情報から幾つ法線が必要か調べる
	NormalNum = 0 ;
	FaceNum = 0 ;
	FNInd = FaceNormalIndex ;
	Mesh = Frame->Mesh ;
	for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
	{
		Face = Mesh->Face ;
		for( j = 0 ; j < Mesh->FaceNum ; j ++, Face ++, FaceNum ++ )
		{
			// ついでに面の法線を計算する
			P0 = &( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * ( ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * Face->VertexIndex[ 0 ] ) )->PositionIndex ) )->Position ;
			P1 = &( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * ( ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * Face->VertexIndex[ 1 ] ) )->PositionIndex ) )->Position ;
			P2 = &( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * ( ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * Face->VertexIndex[ 2 ] ) )->PositionIndex ) )->Position ;

			VectorSub( &V1, P1, P0 ) ;
			VectorSub( &V2, P2, P0 ) ;
			VectorOuterProduct( &Norm, &V1, &V2 ) ;
			VectorNormalize( &FaceNormal[ FaceNum ], &Norm ) ;

			// 法線インデックスをセット
			FNInd[ 0 ] = NormalNum ;
			FNInd[ 1 ] = NormalNum + 1 ;
			FNInd[ 2 ] = NormalNum + 2 ;

			// 法線の数をインデックスの数だけ増やす
			FNInd += 3 ;
			NormalNum += 3 ;
		}
	}
	IndexNum = NormalNum ;

	// 法線を格納するメモリを確保する
	NormalBuf = ( VECTOR * )DXALLOC( ( sizeof( VECTOR ) + sizeof( BYTE ) + sizeof( DWORD ) + sizeof( DWORD ) ) * NormalNum ) ;
	if( NormalBuf == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xd5\x6c\xda\x7d\x8d\x51\x08\x8a\x97\x7b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x5f\x00\x32\x00\x0a\x00\x00"/*@ L"法線再計算用のメモリの確保に失敗しました_2\n" @*/ )) ;
		goto ERR ;
	}
	_MEMSET( NormalBuf, 0, ( sizeof( VECTOR ) + sizeof( BYTE ) + sizeof( DWORD ) + sizeof( DWORD ) ) * NormalNum ) ;
	NormalSetFlag = ( BYTE * )( NormalBuf + NormalNum ) ;
	NormalUseTable = ( BYTE * )( NormalBuf + NormalNum ) ;
	UseNormalIndex = ( DWORD * )( NormalUseTable + NormalNum ) ;
	NewNormalIndex = UseNormalIndex + NormalNum ;

	// 各頂点に関係する面のリストを作成する
	{
		VertexFaceList = ( MV1_MAKEVERTINDEXINFO ** )DXALLOC( sizeof( MV1_MAKEVERTINDEXINFO * ) * Frame->PositionNum + sizeof( MV1_MAKEVERTINDEXINFO ) * IndexNum ) ;
		if( VertexFaceList == NULL )
		{
			DXST_ERRORLOGFMT_ADDUTF16LE(( "\xd5\x6c\xda\x7d\x8d\x51\x08\x8a\x97\x7b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x5f\x00\x33\x00\x0a\x00\x00"/*@ L"法線再計算用のメモリの確保に失敗しました_3\n" @*/ )) ;
			goto ERR ;
		}
		_MEMSET( VertexFaceList, 0, sizeof( MV1_MAKEVERTINDEXINFO * ) * Frame->PositionNum ) ;
		VertexFaceBuffer = ( MV1_MAKEVERTINDEXINFO * )( VertexFaceList + Frame->PositionNum ) ;

		// 全ての面の参照している頂点の情報をセットする
		Mesh = Frame->Mesh ;
		VFBuf = VertexFaceBuffer ;
		for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
		{
			Face = Mesh->Face ;
			for( j = 0 ; j < Mesh->FaceNum ; j ++, Face ++ )
			{
				PosInd[ 0 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * Face->VertexIndex[ 0 ] ) )->PositionIndex ;
				PosInd[ 1 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * Face->VertexIndex[ 1 ] ) )->PositionIndex ;
				PosInd[ 2 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * Face->VertexIndex[ 2 ] ) )->PositionIndex ;

				VFBuf->Face = Face ;
				VFBuf->Mesh = Mesh ;
				VFBuf->Next = VertexFaceList[ PosInd[ 0 ] ] ;
				VertexFaceList[ PosInd[ 0 ] ] = VFBuf ;
				VFBuf ++ ;

				VFBuf->Face = Face ;
				VFBuf->Mesh = Mesh ;
				VFBuf->Next = VertexFaceList[ PosInd[ 1 ] ] ;
				VertexFaceList[ PosInd[ 1 ] ] = VFBuf ;
				VFBuf ++ ;

				VFBuf->Face = Face ;
				VFBuf->Mesh = Mesh ;
				VFBuf->Next = VertexFaceList[ PosInd[ 2 ] ] ;
				VertexFaceList[ PosInd[ 2 ] ] = VFBuf ;
				VFBuf ++ ;
			}
		}
	}

	// 面の法線を構築する
	FNorm = FaceNormal ;
	FNInd = FaceNormalIndex ;
	Mesh = Frame->Mesh ;
	for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
	{
		Face = Mesh->Face ;
		for( j = 0 ; j < Mesh->FaceNum ; j ++, Face ++, FNInd += 3, FNorm ++ )
		{
			// 既に処理済みの場合は何もしない
			if( NormalSetFlag[ FNInd[ 0 ] ] &&
				NormalSetFlag[ FNInd[ 1 ] ] &&
				NormalSetFlag[ FNInd[ 2 ] ] )
				continue ;

			// 頂点の数だけ繰り返し
			for( k = 0 ; k < 3 ; k ++ )
			{
				// 既に計算済みの場合は何もしない
				if( NormalSetFlag[ FNInd[ k ] ] ) continue ;

				// 連結面をリストアップ
				{
					FaceList[ 0 ] = Face ;
					FaceNorm = *FNorm ;
					FaceIndex[ 0 ] = ( BYTE )k ;
					NormalSetFlag[ FNInd[ k ] ] |= 2 ;
					FaceCount = 1 ;

					// 片方の連結面をリストアップ
					CurFace = Face ;
					CurMesh = Mesh ;
					BackCurFace = NULL ;
					CurFaceIndex = k ;
					Issyuu = 0 ;
					for(;;)
					{
						MinFace = NULL ;
						MinMesh = NULL ;
						MPosInd[ 0 ] = -1 ;
						MPosInd[ 1 ] = -1 ;
						MPosInd[ 2 ] = -1 ;

						// 座標インデックスをセットしておく
						PosInd[ 0 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )CurMesh->Vertex + CurMesh->VertUnitSize * CurFace->VertexIndex[ 0 ] ) )->PositionIndex ;
						PosInd[ 1 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )CurMesh->Vertex + CurMesh->VertUnitSize * CurFace->VertexIndex[ 1 ] ) )->PositionIndex ;
						PosInd[ 2 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )CurMesh->Vertex + CurMesh->VertUnitSize * CurFace->VertexIndex[ 2 ] ) )->PositionIndex ;

						// 座標をセットしておく
						Pos[ 0 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * PosInd[ 0 ] ) )->Position ;
						Pos[ 1 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * PosInd[ 1 ] ) )->Position ;
						Pos[ 2 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * PosInd[ 2 ] ) )->Position ;

						// 法線を割り出したい頂点から隣接しているか調べたい辺の頂点へ向かうベクトルをＺ、もう片方の頂点へ向かうベクトルをＸとした三軸の作成
						VectorSub( &zv, &Pos[ ( CurFaceIndex + 1 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
						VectorSub( &xv, &Pos[ ( CurFaceIndex + 2 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
						VectorOuterProduct( &yv, &zv, &xv ) ;
						VectorOuterProduct( &xv, &yv, &zv ) ;
						VectorNormalize( &xv, &xv ) ;
						VectorNormalize( &yv, &yv ) ;
						VectorNormalize( &zv, &zv ) ;

						// 隣接しているか調べたい辺の頂点とは別のもう一つの頂点へ向かうベクトルを上記で算出した三軸上に投影したベクトルを算出
						VectorSub( &tv, &Pos[ ( CurFaceIndex + 2 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
						V1.x = xv.x * tv.x + xv.y * tv.y + xv.z * tv.z ;
						V1.y = yv.x * tv.x + yv.y * tv.y + yv.z * tv.z ;
						V1.z = zv.x * tv.x + zv.y * tv.y + zv.z * tv.z ;
						V1.z = 0.0f ;
						VectorNormalize( &V1, &V1 ) ;

						// 同一の辺を持つポリゴンの検索
						for( VFBuf = VertexFaceList[ PosInd[ CurFaceIndex % 3 ] ] ; VFBuf ; VFBuf = VFBuf->Next )
						{
							FaceT = ( MV1_MESH_FACE * )VFBuf->Face ;
							TMesh = ( MV1_MESH_BASE * )VFBuf->Mesh ;

							// 既に検出済みのポリゴンは無視
							FNInd2 = FaceNormalIndex + ( FaceT - Frame->Mesh->Face ) * 3 ;
							if( ( NormalSetFlag[ FNInd2[ 0 ] ] & 2 ) ||
								( NormalSetFlag[ FNInd2[ 1 ] ] & 2 ) ||
								( NormalSetFlag[ FNInd2[ 2 ] ] & 2 ) ) continue ;

							// 自分自身か、一つ前に自分だったポリゴンは無視
							if( FaceT == CurFace || FaceT == BackCurFace ) continue ;

							// 座標インデックスをセットしておく
							TPosInd[ 0 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )TMesh->Vertex + TMesh->VertUnitSize * FaceT->VertexIndex[ 0 ] ) )->PositionIndex ;
							TPosInd[ 1 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )TMesh->Vertex + TMesh->VertUnitSize * FaceT->VertexIndex[ 1 ] ) )->PositionIndex ;
							TPosInd[ 2 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )TMesh->Vertex + TMesh->VertUnitSize * FaceT->VertexIndex[ 2 ] ) )->PositionIndex ;

							// 座標をセットしておく
							TPos[ 0 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * TPosInd[ 0 ] ) )->Position ;
							TPos[ 1 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * TPosInd[ 1 ] ) )->Position ;
							TPos[ 2 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * TPosInd[ 2 ] ) )->Position ;

							// 頂点の数だけ繰り返し
							for( m = 0 ; m < 3 ; m ++ )
							{
								CmpFlag = 0 ;

								// 同じ辺を持つか調べる、頂点指定の順番から面の方向を調べる
								if( PosInd[ CurFaceIndex         % 3 ] == TPosInd[ m         % 3 ] &&
									PosInd[ ( CurFaceIndex + 1 ) % 3 ] == TPosInd[ ( m + 1 ) % 3 ] )
								{
									CmpFlag = 1 ;
									FaceUra = 1 ;
								}

								if( PosInd[ CurFaceIndex         % 3 ] == TPosInd[ ( m + 1 ) % 3 ] &&
									PosInd[ ( CurFaceIndex + 1 ) % 3 ] == TPosInd[ m         % 3 ] )
								{
									CmpFlag = 1 ;
									FaceUra = 0 ;
								}

								// カレント面と同一の頂点を使用する１８０度回転した面( カレント面の裏面 )だったら検出したことを無かったことにする
								if( CmpFlag == 1 && PosInd[ ( CurFaceIndex + 2 ) % 3 ] == TPosInd[ ( m + 2 ) % 3 ] )
								{
									CmpFlag = 0 ;
								}

								// 同一の辺が見つかったらループを抜ける
								if( CmpFlag != 0 ) break ;
							}

							// 同一の辺が見つかったら処理
							if( m != 3 )
							{
								// 見つけたポリゴンの、同一の辺を成す頂点とは別のもう一つの頂点へ向かうベクトルを先に算出した三軸へ投影したベクトルを算出する
								VectorSub( &tv, &TPos[ ( m + 2 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
								V2.x = xv.x * tv.x + xv.y * tv.y + xv.z * tv.z ;
								V2.y = yv.x * tv.x + yv.y * tv.y + yv.z * tv.z ;
								V2.z = zv.x * tv.x + zv.y * tv.y + zv.z * tv.z ;
								V2.z = 0.0f ;

								// 二つの面を真横から見たときにできる線が成す角のコサイン値を調べる
								// ( カレント面の表向き方向に回転したときに二辺が成す角の角度を調べる )
								{
									// 外積を使って１８０度以上かどうかを判定
									VectorOuterProduct( &tv, &V1, &V2 ) ;
									VectorNormalize( &V2, &V2 ) ;
									if( tv.z < 0.0f )
									{
										// １８０度以上だった場合は１８０度以上だったことを記録した上で
										// カレント面側のベクトルを逆転して内積を求める( コサイン値を求める )
										FaceRadi = -1.0f ;
										FaceRadi2Use = 1 ;
										V3.x = -V1.x ;
										V3.y = -V1.y ;
										V3.z = -V1.z ;
										FaceRadi2 = VectorInnerProduct( &V3, &V2 ) ;
									}
									else
									{
										// １８０度以下だった場合はそのまま二つの線の内積を求める( コサイン値を求める )
										FaceRadi = VectorInnerProduct( &V1, &V2 ) ;
										FaceRadi2Use = 0 ;
										FaceRadi2 = 1.0f ;
									}
								}

								// まだ隣接する面を見つけていないか
								// 既に見つけているが今回の面の裏面か
								// 既に見つけている隣接面よりも角度的に近い面だったら隣接面として記録する
								if( MinFace == NULL ||
									( ( ( TPosInd[ 0 ] == MPosInd[ 0 ] &&
										  TPosInd[ 1 ] == MPosInd[ 2 ] &&
										  TPosInd[ 2 ] == MPosInd[ 1 ] ) ||
										( TPosInd[ 0 ] == MPosInd[ 1 ] &&
										  TPosInd[ 1 ] == MPosInd[ 0 ] &&
										  TPosInd[ 2 ] == MPosInd[ 2 ] ) ||
										( TPosInd[ 0 ] == MPosInd[ 2 ] &&
										  TPosInd[ 1 ] == MPosInd[ 1 ] &&
										  TPosInd[ 2 ] == MPosInd[ 0 ] ) ) && FaceUra == 0 && MinFaceUra == 1 ) ||
									( FaceRadi2Use == 0 && MinFaceRadi2Use == 1 ) ||
									( FaceRadi2Use == 1 && MinFaceRadi2Use == 1 && FaceRadi2 > MinFaceRadi2 ) ||
									( FaceRadi2Use == 0 && MinFaceRadi2Use == 0 && FaceRadi  > MinFaceRadi ) )
								{
									MPosInd[ 0 ] = TPosInd[ 0 ] ;
									MPosInd[ 1 ] = TPosInd[ 1 ] ;
									MPosInd[ 2 ] = TPosInd[ 2 ] ;
									MFNInd = FNInd2 ;
									MinMesh = TMesh ;
									MinFace = FaceT ;
									MinFaceRadi = FaceRadi ;
									MinFaceRadi2Use = FaceRadi2Use ;
									MinFaceRadi2 = FaceRadi2 ;
									MinFaceUra = FaceUra ;
									MinFaceIndex = m + 1 ;
									if( MinFaceIndex == 3 ) MinFaceIndex = 0 ;
								}
							}
						}

						// 隣接面を発見できなかったか、隣接面が裏面だったか、隣接面との法線の角度差がスムージング対象の閾値を超えていたらエッジとみなす
						if( MinFace == NULL || MinFaceUra || VectorInnerProduct( FaceNormal + ( MinFace - Frame->Mesh->Face ), FaceNormal + ( CurFace - Frame->Mesh->Face ) ) < SmoothCos )
							break ;

						// 発見した隣接面が検索開始の面だった場合は一周したということ
						if( MinFace == FaceList[ 0 ] )
						{
							Issyuu = 1 ;
							break ;
						}

						// 確定した隣接面を記録
						FaceList[ FaceCount ] = MinFace ;
						FaceIndex[ FaceCount ] = ( BYTE )MinFaceIndex ;
						VectorAdd( &FaceNorm, &FaceNorm, FaceNormal + ( MinFace - Frame->Mesh->Face ) ) ;
						NormalSetFlag[ MFNInd[ MinFaceIndex ] ] |= 2 ;
						FaceCount ++ ;

						// カレント面の変更
						BackCurFace = CurFace ;
						CurMesh = MinMesh ;
						CurFace = MinFace ;
						CurFaceIndex = MinFaceIndex ;
					}

					// もう片方の連結面をリストアップ
					if( Issyuu == 0 )
					{
						BackCurFace = NULL ;
						CurMesh = Mesh ;
						CurFace = Face ;
						CurFaceIndex = k ;
						for(;;)
						{
							MinFace = NULL ;
							MinMesh = NULL ;
							MPosInd[ 0 ] = -1 ;
							MPosInd[ 1 ] = -1 ;
							MPosInd[ 2 ] = -1 ;

							// 座標インデックスをセットしておく
							PosInd[ 0 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )CurMesh->Vertex + CurMesh->VertUnitSize * CurFace->VertexIndex[ 0 ] ) )->PositionIndex ;
							PosInd[ 1 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )CurMesh->Vertex + CurMesh->VertUnitSize * CurFace->VertexIndex[ 1 ] ) )->PositionIndex ;
							PosInd[ 2 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )CurMesh->Vertex + CurMesh->VertUnitSize * CurFace->VertexIndex[ 2 ] ) )->PositionIndex ;

							// 座標をセットしておく
							Pos[ 0 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * PosInd[ 0 ] ) )->Position ;
							Pos[ 1 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * PosInd[ 1 ] ) )->Position ;
							Pos[ 2 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * PosInd[ 2 ] ) )->Position ;

							// 法線を割り出したい頂点から隣接しているか調べたい辺の頂点へ向かうベクトルをＺ、もう片方の頂点へ向かうベクトルをＸとした３軸ベクトルの作成
							VectorSub( &zv, &Pos[ ( CurFaceIndex + 3 - 1 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
							VectorSub( &xv, &Pos[ ( CurFaceIndex + 3 - 2 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
							VectorOuterProduct( &yv, &zv, &xv ) ;
							VectorOuterProduct( &xv, &yv, &zv ) ;
							VectorNormalize( &xv, &xv ) ;
							VectorNormalize( &yv, &yv ) ;
							VectorNormalize( &zv, &zv ) ;

							// 隣接しているか調べたい辺の頂点とは別のもう一つの頂点へ向かうベクトルを上記で算出した三軸上に投影したベクトルを算出
							VectorSub( &tv, &Pos[ ( CurFaceIndex + 3 - 2 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
							V1.x = xv.x * tv.x + xv.y * tv.y + xv.z * tv.z ;
							V1.y = yv.x * tv.x + yv.y * tv.y + yv.z * tv.z ;
							V1.z = zv.x * tv.x + zv.y * tv.y + zv.z * tv.z ;
							V1.z = 0.0f ;
							VectorNormalize( &V1, &V1 ) ;

							// 同一の辺を持つポリゴンの検索
							for( VFBuf = VertexFaceList[ PosInd[ CurFaceIndex % 3 ] ] ; VFBuf ; VFBuf = VFBuf->Next )
							{
								FaceT = ( MV1_MESH_FACE * )VFBuf->Face ;
								TMesh = ( MV1_MESH_BASE * )VFBuf->Mesh ;

								// 既に検出済みのポリゴンは無視
								FNInd2 = FaceNormalIndex + ( FaceT - Frame->Mesh->Face ) * 3 ;
								if( ( NormalSetFlag[ FNInd2[ 0 ] ] & 2 ) ||
									( NormalSetFlag[ FNInd2[ 1 ] ] & 2 ) ||
									( NormalSetFlag[ FNInd2[ 2 ] ] & 2 ) ) continue ;

								// 自分自身か、一つ前に自分だったポリゴンは無視
								if( FaceT == CurFace || FaceT == BackCurFace ) continue ;

								// 座標インデックスをセットしておく
								TPosInd[ 0 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )TMesh->Vertex + TMesh->VertUnitSize * FaceT->VertexIndex[ 0 ] ) )->PositionIndex ;
								TPosInd[ 1 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )TMesh->Vertex + TMesh->VertUnitSize * FaceT->VertexIndex[ 1 ] ) )->PositionIndex ;
								TPosInd[ 2 ] = ( int )( ( MV1_MESH_VERTEX * )( ( BYTE * )TMesh->Vertex + TMesh->VertUnitSize * FaceT->VertexIndex[ 2 ] ) )->PositionIndex ;

								// 座標をセットしておく
								TPos[ 0 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * TPosInd[ 0 ] ) )->Position ;
								TPos[ 1 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * TPosInd[ 1 ] ) )->Position ;
								TPos[ 2 ] = ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * TPosInd[ 2 ] ) )->Position ;

								// 頂点の数だけ繰り返し
								for( m = 0 ; m < 3 ; m ++ )
								{
									CmpFlag = 0 ;

									// 同じ辺を持つか調べる、頂点指定の順番から面の方向を調べる
									if( PosInd[ CurFaceIndex             % 3 ] == TPosInd[ m             % 3 ] &&
										PosInd[ ( CurFaceIndex + 3 - 1 ) % 3 ] == TPosInd[ ( m + 3 - 1 ) % 3 ] )
									{
										CmpFlag = 1 ;
										FaceUra = 1 ;
									}

									if( PosInd[ CurFaceIndex             % 3 ] == TPosInd[ ( m + 3 - 1 ) % 3 ] &&
										PosInd[ ( CurFaceIndex + 3 - 1 ) % 3 ] == TPosInd[ m             % 3 ] )
									{
										CmpFlag = 1 ;
										FaceUra = 0 ;
									}

									// カレント面と同一の頂点を使用する１８０度回転した面( カレント面の裏面 )だったら検出したことを無かったことにする
									if( CmpFlag == 1 && PosInd[ ( CurFaceIndex + 3 - 2 ) % 3 ] == TPosInd[ ( m + 3 - 2 ) % 3 ] )
									{
										CmpFlag = 0 ;
									}

									// 同一の辺が見つかったらループを抜ける
									if( CmpFlag != 0 ) break ;
								}

								// 同一の辺が見つかったら処理
								if( m != 3 )
								{
									// 見つけたポリゴンの、同一の辺を成す頂点とは別のもう一つの頂点へ向かうベクトルを先に算出した三軸へ投影したベクトルを算出する
									VectorSub( &tv, &TPos[ ( m + 3 - 2 ) % 3 ], &Pos[ CurFaceIndex % 3 ] ) ;
									V2.x = xv.x * tv.x + xv.y * tv.y + xv.z * tv.z ;
									V2.y = yv.x * tv.x + yv.y * tv.y + yv.z * tv.z ;
									V2.z = zv.x * tv.x + zv.y * tv.y + zv.z * tv.z ;
									V2.z = 0.0f ;

									// 二つの面を真横から見たときにできる線が成す角のコサイン値を調べる
									// ( カレント面の表向き方向に回転したときに二辺が成す角の角度を調べる )
									{
										// 外積を使って１８０度以上かどうかを判定
										VectorOuterProduct( &tv, &V1, &V2 ) ;
										VectorNormalize( &V2, &V2 ) ;
										if( tv.z > 0.0f )
										{
											// １８０度以上だった場合は１８０度以上だったことを記録した上で
											// カレント面側のベクトルを逆転して内積を求める( コサイン値を求める )
											FaceRadi = -1.0f ;
											FaceRadi2Use = 1 ;
											V3.x = -V1.x ;
											V3.y = -V1.y ;
											V3.z = -V1.z ;
											FaceRadi2 = VectorInnerProduct( &V3, &V2 ) ;
										}
										else
										{
											// １８０度以下だった場合はそのまま二つの線の内積を求める( コサイン値を求める )
											FaceRadi = VectorInnerProduct( &V1, &V2 ) ;
											FaceRadi2Use = 0 ;
											FaceRadi2 = 1.0f ;
										}
									}

									// まだ隣接する面を見つけていないか
									// 既に見つけているが今回の面の裏面か
									// 既に見つけている隣接面よりも角度的に近い面だったら隣接面として記録する
									if( MinFace == NULL ||
										( ( ( TPosInd[ 0 ] == MPosInd[ 0 ] &&
											  TPosInd[ 1 ] == MPosInd[ 2 ] &&
											  TPosInd[ 2 ] == MPosInd[ 1 ] ) ||
											( TPosInd[ 0 ] == MPosInd[ 1 ] &&
											  TPosInd[ 1 ] == MPosInd[ 0 ] &&
											  TPosInd[ 2 ] == MPosInd[ 2 ] ) ||
											( TPosInd[ 0 ] == MPosInd[ 2 ] &&
											  TPosInd[ 1 ] == MPosInd[ 1 ] &&
											  TPosInd[ 2 ] == MPosInd[ 0 ] ) ) && FaceUra == 0 && MinFaceUra == 1 ) ||
										( FaceRadi2Use == 0 && MinFaceRadi2Use == 1 ) ||
										( FaceRadi2Use == 1 && MinFaceRadi2Use == 1 && FaceRadi2 > MinFaceRadi2 ) ||
										( FaceRadi2Use == 0 && MinFaceRadi2Use == 0 && FaceRadi  > MinFaceRadi ) )
									{
										MPosInd[ 0 ] = TPosInd[ 0 ] ;
										MPosInd[ 1 ] = TPosInd[ 1 ] ;
										MPosInd[ 2 ] = TPosInd[ 2 ] ;
										MFNInd = FNInd2 ;
										MinMesh = TMesh ;
										MinFace = FaceT ;
										MinFaceRadi = FaceRadi ;
										MinFaceRadi2Use = FaceRadi2Use ;
										MinFaceRadi2 = FaceRadi2 ;
										MinFaceUra = FaceUra ;
										MinFaceIndex = m - 1 ;
										if( MinFaceIndex < 0 ) MinFaceIndex += 3 ;
									}
								}
							}

							// 隣接面を発見できなかったか、隣接面が裏面だったか、隣接面との法線の角度差がスムージング対象の閾値を超えていたらエッジとみなす
							if( MinFace == NULL || MinFaceUra || VectorInnerProduct( FaceNormal + ( MinFace - Frame->Mesh->Face ), FaceNormal + ( CurFace - Frame->Mesh->Face ) ) < SmoothCos )
								break ;

							// 発見した隣接面が検索開始の面だった場合は一周したということ
							if( MinFace == FaceList[ 0 ] )
							{
								Issyuu = 1 ;
								break ;
							}

							// 確定した隣接面を記録
							FaceList[ FaceCount ] = MinFace ;
							FaceIndex[ FaceCount ] = ( BYTE )MinFaceIndex ;
							VectorAdd( &FaceNorm, &FaceNorm, FaceNormal + ( MinFace - Frame->Mesh->Face ) ) ;
							NormalSetFlag[ MFNInd[ MinFaceIndex ] ] |= 2 ;
							FaceCount ++ ;

							// カレント面の変更
							BackCurFace = CurFace ;
							CurMesh = MinMesh ;
							CurFace = MinFace ;
							CurFaceIndex = MinFaceIndex ;
						}
					}

					// 検出した面の法線を足したものを正規化
					VectorNormalize( &FaceNorm, &FaceNorm ) ;
				}

				// リストアップされた面に正規化した法線をセットする
				if( FaceCount != 0 )
				{
					MFNInd = FaceNormalIndex + ( FaceList[ 0 ] - Frame->Mesh->Face ) * 3 ;
					for( m = 0 ; m < FaceCount ; m ++ )
					{
						FNInd2 = FaceNormalIndex + ( FaceList[ m ] - Frame->Mesh->Face ) * 3 ;
						Index = FNInd2[ FaceIndex[ m ] ] ;
						NormalBuf[ Index ] = FaceNorm ;
						NormalSetFlag[ Index ] = 1 ;
						FNInd2[ FaceIndex[ m ] ] = MFNInd[ FaceIndex[ 0 ] ] ;
					}
				}
			}
		}
	}

	// 使用している法線をインデックスの若い順に詰める
	{
		// 使用されている法線のマップを作成する
		_MEMSET( NormalUseTable, 0, ( size_t )NormalNum ) ;
		Mesh = Frame->Mesh ;
		UseNormalNum = 0 ;
		FNInd = FaceNormalIndex ;
		for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
		{
			Face = Mesh->Face ;
			for( j = 0 ; j < Mesh->FaceNum ; j ++, Face ++, FNInd += 3 )
			{
				if( NormalUseTable[ FNInd[ 0 ] ] == 0 )
				{
					NormalUseTable[ FNInd[ 0 ] ]   = 1 ;
					UseNormalIndex[ UseNormalNum ] = ( DWORD )FNInd[ 0 ] ;
					NewNormalIndex[ FNInd[ 0 ] ]   = ( DWORD )UseNormalNum ;
					UseNormalNum ++ ;
				}

				if( NormalUseTable[ FNInd[ 1 ] ] == 0 )
				{
					NormalUseTable[ FNInd[ 1 ] ]   = 1 ;
					UseNormalIndex[ UseNormalNum ] = ( DWORD )FNInd[ 1 ] ;
					NewNormalIndex[ FNInd[ 1 ] ]   = ( DWORD )UseNormalNum ;
					UseNormalNum ++ ;
				}

				if( NormalUseTable[ FNInd[ 2 ] ] == 0 )
				{
					NormalUseTable[ FNInd[ 2 ] ]   = 1 ;
					UseNormalIndex[ UseNormalNum ] = ( DWORD )FNInd[ 2 ] ;
					NewNormalIndex[ FNInd[ 2 ] ]   = ( DWORD )UseNormalNum ;
					UseNormalNum ++ ;
				}
			}
		}

		// 法線の数が増えたかどうかで格納先のメモリを変更
		if( UseNormalNum <= Frame->NormalNum )
		{
			// 減った場合

			// 新たなメモリは確保しない
			RNormal = Frame->Normal ;
		}
		else
		{
			// 増えた場合
			if( Frame->NormalAllocMem )
			{
				DXFREE( Frame->Normal ) ;
			}
			RNormal = ( MV1_MESH_NORMAL * )DXALLOC( sizeof( MV1_MESH_NORMAL ) * UseNormalNum ) ;
			if( RNormal == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\xd5\x6c\xda\x7d\x8d\x51\x08\x8a\x97\x7b\x67\x30\xd5\x6c\xda\x7d\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"法線再計算で法線を格納するメモリの確保に失敗しました\n" @*/ )) ;
				goto ERR ;
			}
			Frame->Normal = RNormal ;
			Frame->NormalAllocMem = TRUE ;
		}

		// 最終版の法線セットを作成する
		for( i = 0 ; i < UseNormalNum ; i ++ )
		{
			RNormal[ i ].Normal = NormalBuf[ UseNormalIndex[ i ] ] ;
		}

		// 面データの法線インデックスを更新する
		FNInd = FaceNormalIndex ;
		Mesh = Frame->Mesh ;
		for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
		{
			Face = Mesh->Face ;
			for( j = 0 ; j < Mesh->FaceNum ; j ++, Face ++, FNInd += 3 )
			{
				FNInd[ 0 ] = ( int )NewNormalIndex[ FNInd[ 0 ] ] ;
				FNInd[ 1 ] = ( int )NewNormalIndex[ FNInd[ 1 ] ] ;
				FNInd[ 2 ] = ( int )NewNormalIndex[ FNInd[ 2 ] ] ;
			}
		}

		// データのサイズを変更
		{
			int AddSize ;

			AddSize = UseNormalNum - Frame->NormalNum ;
			ModelBase->MeshNormalNum += AddSize ;
			ModelBase->VertexDataSize += AddSize * sizeof( MV1_MESH_NORMAL ) ;
		}

		// 最終的な法線の数をセット
		Frame->NormalNum = UseNormalNum ;
	}

	// 新しい法線情報を元に頂点データ配列を再構築する
	{
		BYTE TVertBuf[ sizeof( MV1_MESH_VERTEX ) + sizeof( float ) * 2 * 32 ] ;
		MV1_MESH_VERTEX *TVertex ;

		TVertex = ( MV1_MESH_VERTEX * )TVertBuf ;

		// トライアングルリスト構築用情報を格納するメモリの確保
		VertInfoTable = ( MV1_MAKEVERTINDEXINFO ** )DXALLOC( sizeof( MV1_MAKEVERTINDEXINFO * ) * Frame->PositionNum + sizeof( MV1_MAKEVERTINDEXINFO ) * Frame->TriangleNum * 3 ) ;
		if( VertInfoTable == NULL )
		{
			DXST_ERRORLOGFMT_ADDUTF16LE(( "\xd5\x6c\xda\x7d\x8d\x51\x08\x8a\x97\x7b\x6b\x30\x7f\x4f\x28\x75\x59\x30\x8b\x30\x5c\x4f\x6d\x69\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x5f\x00\x33\x00\x0a\x00\x00"/*@ L"法線再計算に使用する作業用メモリの確保に失敗しました_3\n" @*/ )) ;
			goto ERR ;
		}
		VertInfoBuffer = ( MV1_MAKEVERTINDEXINFO * )( VertInfoTable + Frame->PositionNum ) ;
		VertValidBuffer = ( int * )VertInfoBuffer ;

		// メッシュの数だけ繰り返し
		Mesh = Frame->Mesh ;
		FNInd = FaceNormalIndex ;
		for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
		{
			if( Mesh->FaceNum == 0 )
				continue ;

			// 新たな頂点データを格納するメモリ領域の確保
			NewVertex = ( MV1_MESH_VERTEX * )DXALLOC( ( size_t )( Mesh->VertUnitSize * Mesh->FaceNum * 3 ) ) ;
			if( NewVertex == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\xd5\x6c\xda\x7d\x8d\x51\x08\x8a\x97\x7b\x6b\x30\x7f\x4f\x28\x75\x59\x30\x8b\x30\x5c\x4f\x6d\x69\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"法線再計算に使用する作業用メモリの確保に失敗しました\n" @*/ )) ;
				goto ERR ;
			}

			// 今までの面情報を格納するメモリ領域の確保
			OldFace = ( MV1_MESH_FACE * )DXALLOC( sizeof( MV1_MESH_FACE ) * Mesh->FaceNum ) ;
			if( OldFace == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\xd5\x6c\xda\x7d\x8d\x51\x08\x8a\x97\x7b\x6b\x30\x7f\x4f\x28\x75\x59\x30\x8b\x30\x5c\x4f\x6d\x69\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x5f\x00\x32\x00\x0a\x00\x00"/*@ L"法線再計算に使用する作業用メモリの確保に失敗しました_2\n" @*/ )) ;
				goto ERR ;
			}
			_MEMCPY( OldFace, Mesh->Face, sizeof( MV1_MESH_FACE ) * Mesh->FaceNum ) ;

			// 頂点データと面データを構築
			NewVertexNum = 0 ;
			OFace = OldFace ;
			Face = Mesh->Face ;
			_MEMSET( VertInfoTable, 0, sizeof( MV1_MAKEVERTINDEXINFO * ) * Frame->PositionNum ) ;
			VertInfoNum = 0 ;
			for( j = 0 ; j < Mesh->FaceNum ; j ++, OFace ++, Face ++, FNInd += 3 )
			{
				// ポリゴンの頂点の数だけ繰り返し
				for( m = 0 ; m < 3 ; m ++ )
				{
					// 追加しようとしている頂点データの作成
					Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * OFace->VertexIndex[ m ] ) ;
					_MEMCPY( TVertex, Vert, ( size_t )Mesh->VertUnitSize ) ;
					TVertex->NormalIndex = ( DWORD )FNInd[ m ] ;

					// 今までに同じ頂点データが無かったかどうかを調べる
					for( VInfo = VertInfoTable[ Vert->PositionIndex ] ; VInfo ; VInfo = VInfo->Next )
					{
						if( _MEMCMP( ( BYTE * )NewVertex + VInfo->VertexIndex * Mesh->VertUnitSize, TVertex, Mesh->VertUnitSize ) == 0 )
							break ;
					}
					if( VInfo == NULL )
					{
						// 無かったらデータを追加
						VInfo = &VertInfoBuffer[ VertInfoNum ] ;
						VertInfoNum ++ ;

						VInfo->VertexIndex = NewVertexNum ;
						VInfo->Next = VertInfoTable[ Vert->PositionIndex ] ;
						VertInfoTable[ Vert->PositionIndex ] = VInfo ;

						_MEMCPY( ( BYTE * )NewVertex + NewVertexNum * Mesh->VertUnitSize, TVertex, ( size_t )Mesh->VertUnitSize ) ;
						Face->VertexIndex[ m ] = ( DWORD )NewVertexNum ;
						NewVertexNum ++ ;
					}
					else
					{
						// あったらインデックスをセット
						Face->VertexIndex[ m ] = ( DWORD )VInfo->VertexIndex ;
					}
				}
			}

			// 今までの面情報を格納していたメモリの解放
			DXFREE( OldFace ) ;
			OldFace = NULL ;

			// 頂点データの数が増えた場合と減った場合で処理を分岐
			if( NewVertexNum <= Mesh->VertexNum )
			{
				// 減った場合

				// 新たに確保した頂点用のメモリは解放する
				_MEMCPY( Mesh->Vertex, NewVertex, ( size_t )( NewVertexNum * Mesh->VertUnitSize ) ) ;
				DXFREE( NewVertex ) ;
			}
			else
			{
				// 増えた場合
				if( Mesh->VertexAllocMem )
				{
					DXFREE( Mesh->Vertex ) ;
				}
				Mesh->Vertex = NewVertex ;
				Mesh->VertexAllocMem = TRUE ;
			}
			NewVertex = NULL ;

			// データのサイズを変更
			{
				int AddSize ;

				AddSize = ( NewVertexNum - Mesh->VertexNum ) * Mesh->VertUnitSize ;
				ModelBase->MeshVertexSize += AddSize ;
				ModelBase->VertexDataSize += AddSize ;
				Frame->VertexNum += NewVertexNum - Mesh->VertexNum ;
			}

			// 頂点の数を保存
			Mesh->VertexNum = NewVertexNum ;

			// 従法線と接線を算出する
			MV1MakeMeshBinormalsAndTangents( Mesh ) ;

			// トライアングルリストの再構築
			TList = Mesh->TriangleList ;
			for( j = 0 ; j < Mesh->TriangleListNum ; j ++, TList ++ )
			{
				// 環境依存のテンポラリバッファを開放
				MV1_TerminateTriangleListBaseTempBuffer_PF( TList ) ;

				// 頂点が既に存在しているかどうかのフラグを初期化する
				_MEMSET( VertValidBuffer, 0xff, Mesh->VertexNum * sizeof( int ) ) ;

				// 新たな頂点インデックス配列を格納するためのメモリ領域の確保
				NewMeshVertexIndex = ( DWORD * )DXALLOC( sizeof( int ) * Mesh->VertexNum ) ;
				if( NewMeshVertexIndex == NULL )
				{
					DXST_ERRORLOGFMT_ADDUTF16LE(( "\xc8\x30\xe9\x30\xa4\x30\xa2\x30\xf3\x30\xb0\x30\xeb\x30\xea\x30\xb9\x30\xc8\x30\x6e\x30\xe1\x30\xc3\x30\xb7\x30\xe5\x30\x02\x98\xb9\x70\xa4\x30\xf3\x30\xc7\x30\xc3\x30\xaf\x30\xb9\x30\x3c\x68\x0d\x7d\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"トライアングルリストのメッシュ頂点インデックス格納用メモリの確保に失敗しました\n" @*/ )) ;
					goto ERR ;
				}

				// メッシュの面の数だけ繰り返し
				NewTVertexNum = 0 ;
				Face = Mesh->Face ;
				IndexNum = 0 ;
				for( k = 0 ; k < Mesh->FaceNum ; k ++, Face ++ )
				{
					// トライアングルリストナンバーが違ったら何もしない
					if( Face->TriangleListIndex != j ) continue ;

					// 頂点の数だけ繰り返し
					for( m = 0 ; m < 3 ; m ++, IndexNum ++ )
					{
						// トライアングルが使用している頂点データが既にあるかどうかを調べる
						if( VertValidBuffer[ Face->VertexIndex[ m ] ] == -1 )
						{
							// 無かったら追加する
							VertValidBuffer[ Face->VertexIndex[ m ] ] = NewTVertexNum ;
							TList->Index[ IndexNum ] = ( unsigned short )NewTVertexNum ;

							NewMeshVertexIndex[ NewTVertexNum ] = Face->VertexIndex[ m ] ;
							NewTVertexNum ++ ;
						}
						else
						{
							TList->Index[ IndexNum ] = ( unsigned short )VertValidBuffer[ Face->VertexIndex[ m ] ] ;
						}
					}
				}

				// 今までの頂点の数より多いかどうかで処理を分岐
				if( NewTVertexNum <= TList->VertexNum )
				{
					// 減った場合

					// 新たに確保した頂点インデックス配列用のメモリは解放する
					_MEMCPY( TList->MeshVertexIndex, NewMeshVertexIndex, sizeof( DWORD ) * NewTVertexNum ) ;
					DXFREE( NewMeshVertexIndex ) ;
				}
				else
				{
					// 増えた場合
					if( TList->MeshVertexIndexAllocMem )
					{
						DXFREE( TList->MeshVertexIndex ) ;
					}
					TList->MeshVertexIndex = NewMeshVertexIndex ;
					TList->MeshVertexIndexAllocMem = TRUE ;

					// 高速頂点データアクセス用メモリも新たな頂点データのサイズに応じて再確保する
					if( TList->PositionAllocMem )
					{
						DXFREE( TList->NormalPosition ) ;
					}
					TList->NormalPosition = ( MV1_TLIST_NORMAL_POS * )DXALLOC( ( size_t )( TList->PosUnitSize * NewTVertexNum + 16 ) ) ;
					if( TList->NormalPosition == NULL )
					{
						DXST_ERRORLOGFMT_ADDUTF16LE(( "\xc8\x30\xe9\x30\xa4\x30\xa2\x30\xf3\x30\xb0\x30\xeb\x30\xea\x30\xb9\x30\xc8\x30\x6e\x30\xa7\x5e\x19\x6a\xc7\x30\xfc\x30\xbf\x30\x3c\x68\x0d\x7d\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"トライアングルリストの座標データ格納用メモリの確保に失敗しました\n" @*/ )) ;
						goto ERR ;
					}
					TList->PositionAllocMem = TRUE ;
				}
				NewMeshVertexIndex = NULL ;

				// データのサイズを変更
				{
					int AddSize ;

					AddSize = NewTVertexNum - TList->VertexNum ;
					switch( TList->VertexType )
					{
					case MV1_VERTEX_TYPE_NORMAL :        ModelBase->TriangleListNormalPositionNum     += AddSize ;                      break ;
					case MV1_VERTEX_TYPE_SKIN_4BONE :    ModelBase->TriangleListSkinPosition4BNum     += AddSize ;                      break ;
					case MV1_VERTEX_TYPE_SKIN_8BONE :    ModelBase->TriangleListSkinPosition8BNum     += AddSize ;                      break ;
					case MV1_VERTEX_TYPE_SKIN_FREEBONE : ModelBase->TriangleListSkinPositionFREEBSize += AddSize * TList->PosUnitSize ; break ;
					}

					AddSize = ( NewTVertexNum - TList->VertexNum ) * TList->PosUnitSize ;
					ModelBase->VertexDataSize += AddSize ;
					ModelBase->MeshVertexIndexNum += NewTVertexNum - TList->VertexNum ;
					ModelBase->TriangleListVertexNum += NewTVertexNum - TList->VertexNum ;
				}

				// 頂点データの数をセット
				TList->VertexNum = NewTVertexNum ;

				// 高速処理用頂点データの再構築
				MV1SetupTriangleListPositionAndNormal( TList ) ;

				// トゥーン輪郭線用頂点インデックスの作成
				MV1SetupToonOutLineTriangleList( TList ) ;
			}
		}

		// シェイプ頂点データの更新
		Shape = Frame->Shape ;
		for( i = 0 ; i < Frame->ShapeNum ; i ++, Shape ++ )
		{
			ShapeFrameVert = ShapeFrameVertBuffer + i * Frame->PositionNum ;
			ShapeMesh = Shape->Mesh ;
			for( j = 0 ; j < Shape->MeshNum ; j ++, ShapeMesh ++ )
			{
				Mesh = ShapeMesh->TargetMesh ;
				NewShapeVertBuffer = ( MV1_SHAPE_VERTEX_BASE * )DXALLOC( sizeof( MV1_SHAPE_VERTEX_BASE ) * Mesh->VertexNum ) ;
				if( NewShapeVertBuffer == NULL )
				{
					DXST_ERRORLOGFMT_ADDUTF16LE(( "\xb7\x30\xa7\x30\xa4\x30\xd7\x30\x02\x98\xb9\x70\xc7\x30\xfc\x30\xbf\x30\x3c\x68\x0d\x7d\x28\x75\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"シェイプ頂点データ格納用メモリの確保に失敗しました\n" @*/ )) ;
					goto ERR ;
				}

				NewShapeVertexNum = 0 ;
				Vert = Mesh->Vertex ;
				for( k = 0 ; k < Mesh->VertexNum ; k ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					// 頂点が使用している座標値がシェイプ対象ではない場合は何もしない
					if( ShapeFrameVert[ Vert->PositionIndex ].TargetMeshVertex == -1 ) continue ;

					// 情報の追加
					NewShapeVertBuffer[ NewShapeVertexNum ].TargetMeshVertex = ( DWORD )k ;
					NewShapeVertBuffer[ NewShapeVertexNum ].Position         = ShapeFrameVert[ Vert->PositionIndex ].Position ;
					NewShapeVertBuffer[ NewShapeVertexNum ].Normal           = VGet( 0.0f, 0.0f, 0.0f ) ;
					NewShapeVertexNum ++ ;
				}

				// 情報が増えた場合は新たに確保したメモリを使用する
				if( ( DWORD )NewShapeVertexNum > ShapeMesh->VertexNum )
				{
					if( ShapeMesh->VertexAllocMem )
					{
						DXFREE( ShapeMesh->Vertex ) ;
					}
					ShapeMesh->Vertex = NewShapeVertBuffer ;
					ShapeMesh->VertexAllocMem = TRUE ;
				}
				else
				{
					// そうではない場合は今までのバッファに新しいデータをコピーする
					_MEMCPY( ShapeMesh->Vertex, NewShapeVertBuffer, sizeof( MV1_SHAPE_VERTEX_BASE ) * NewShapeVertexNum ) ;

					// 確保したメモリは解放する
					DXFREE( NewShapeVertBuffer ) ;
				}
				NewShapeVertBuffer = NULL ;

				// シェイプの頂点総数を変更する
				ModelBase->ShapeVertexNum += NewShapeVertexNum - ShapeMesh->VertexNum ;

				// シェイプの頂点数を更新する
				ShapeMesh->VertexNum = ( DWORD )NewShapeVertexNum ;
			}
		}
	}

	// メモリの解放
	if( VertexFaceList )
	{
		DXFREE( VertexFaceList ) ;
		VertexFaceList = NULL ;
	}
	if( FaceNormal )
	{
		DXFREE( FaceNormal ) ;
		FaceNormal = NULL ;
	}
	if( NormalBuf )
	{
		DXFREE( NormalBuf ) ;
		NormalBuf = NULL ;
	}
	if( OldFace )
	{
		DXFREE( OldFace ) ;
		OldFace = NULL ;
	}
	if( VertInfoTable )
	{
		DXFREE( VertInfoTable ) ;
		VertInfoTable = NULL ;
	}
	if( NewShapeVertBuffer )
	{
		DXFREE( NewShapeVertBuffer ) ;
		NewShapeVertBuffer = NULL ;
	}
	
	// 正常終了
	return true ;

ERR :
	// メモリの解放
	if( VertexFaceList )
	{
		DXFREE( VertexFaceList ) ;
		VertexFaceList = NULL ;
	}
	if( FaceNormal )
	{
		DXFREE( FaceNormal ) ;
		FaceNormal = NULL ;
	}
	if( NormalBuf )
	{
		DXFREE( NormalBuf ) ;
		NormalBuf = NULL ;
	}
	if( NewVertex )
	{
		DXFREE( NewVertex ) ;
		NewVertex = NULL ;
	}
	if( OldFace )
	{
		DXFREE( OldFace ) ;
		OldFace = NULL ;
	}
	if( VertInfoTable )
	{
		DXFREE( VertInfoTable ) ;
		VertInfoTable = NULL ;
	}
	if( NewMeshVertexIndex )
	{
		DXFREE( NewMeshVertexIndex ) ;
		NewMeshVertexIndex = NULL ;
	}
	if( NewShapeVertBuffer )
	{
		DXFREE( NewShapeVertBuffer ) ;
		NewShapeVertBuffer = NULL ;
	}

	// エラー
	return -1 ;
}

// モデル全体の座標情報を最適化する
extern int MV1PositionOptimizeBase( int MBHandle )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME_BASE *Frame ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// 頂点バッファを解放する
	MV1_TerminateVertexBufferBase_PF( MBHandle ) ;

	// フレームの数だけ繰り返し
	Frame = ModelBase->Frame ;
	for( i = 0 ; i < ModelBase->FrameNum ; i ++, Frame ++ )
	{
		if( Frame->MeshNum == 0 ) continue ;

		if( _MV1PositionOptimizeFrameBase( Frame ) == -1 )
			return -1 ;
	}

	// 頂点バッファを構築する
	MV1_SetupVertexBufferBase_PF( MBHandle ) ;

	// 終了
	return 0 ;
}

// 指定フレームが持つメッシュの座標情報を最適化する
extern int MV1PositionOptimizeFrameBase( int MBHandle, int FrameIndex )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME_BASE *Frame ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// フレームインデックスのチェック
	if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
		return -1 ;
	Frame = ModelBase->Frame + FrameIndex ;

	// メッシュが無ければ何もせず終了
	if( Frame->MeshNum == 0 )
		return -1 ;

	// 頂点バッファを解放する
	MV1_TerminateVertexBufferBase_PF( MBHandle ) ;

	// フレームの再計算
	if( _MV1PositionOptimizeFrameBase( Frame ) == -1 )
		return -1 ;

	// 頂点バッファを構築する
	MV1_SetupVertexBufferBase_PF( MBHandle ) ;

	// 終了
	return 0 ;
}

// フレームの座標情報を最適化する
static int _MV1PositionOptimizeFrameBase( MV1_FRAME_BASE *Frame )
{
	int i, j, k, l, m, SubNum, BoneNum1, BoneNum2, NewNum, *NewIndex, *NewMVertIndex, Size, MaxNum ;
	MV1_MESH_BASE *Mesh ;
	BYTE *DisableFlag ;
	MV1_MESH_POSITION *Pos1, *Pos2 ;
	MV1_MESH_VERTEX *Vert, *Vert1, *Vert2 ;
	MV1_MESH_FACE *Face ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_SHAPE_BASE *Shape ;
	MV1_SHAPE_MESH_BASE *ShapeMesh ;
	MV1_SHAPE_VERTEX_BASE *ShapeVertex ;
	MV1_TRIANGLE_LIST_BASE *TList ;
	DWORD *MInd1, *MInd2 ;
	WORD *Ind ;
	BYTE *TPos1, *TPos2 ;

	// メッシュが無かったら何もしない
	if( Frame->MeshNum == 0 )
		return 0 ;

	// ポインタの初期化
	ModelBase = Frame->Container ;
	DisableFlag = NULL ;

	// 最も頂点データの数が多いメッシュの頂点数を取得する
	Mesh = Frame->Mesh ;
	MaxNum = 0 ;
	for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->VertexNum > MaxNum )
			MaxNum = Mesh->VertexNum ;
	}
	if( MaxNum < Frame->PositionNum )
		MaxNum = Frame->PositionNum ;

	// 作業用メモリの確保
	DisableFlag = ( BYTE * )DXALLOC( ( sizeof( int ) * 2 + sizeof( BYTE ) ) * MaxNum ) ;
	if( DisableFlag == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x02\x98\xb9\x70\xc5\x60\x31\x58\x00\x67\x69\x90\x16\x53\xe6\x51\x06\x74\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"頂点情報最適化処理用のメモリの確保に失敗しました\n" @*/ )) ;
		goto ERR ;
	}
	NewIndex = ( int * )( DisableFlag + MaxNum ) ;
	NewMVertIndex = NewIndex + MaxNum ;
	_MEMSET( NewIndex, 0, sizeof( int ) * MaxNum ) ;
	_MEMSET( NewMVertIndex, 0, sizeof( int ) * MaxNum ) ;

	// 座標情報の一致する頂点を列挙する
	SubNum = 0 ;
	Pos1 = Frame->Position ;
	_MEMSET( DisableFlag, 0, sizeof( BYTE ) * MaxNum ) ;
	for( i = 0 ; i < Frame->PositionNum ; i ++, Pos1 = ( MV1_MESH_POSITION * )( ( BYTE * )Pos1 + Frame->PosUnitSize ) )
	{
		// 既に無効だったら何もしない
		if( DisableFlag[ i ] ) continue ;

		Pos2 = ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * ( i + 1 ) ) ;
		for( j = i + 1 ; j < Frame->PositionNum ; j ++, Pos2 = ( MV1_MESH_POSITION * )( ( BYTE * )Pos2 + Frame->PosUnitSize ) )
		{
			// 既に無効だったら何もしない
			if( DisableFlag[ j ] ) continue ;

			// 座標が同じか調べる
			if( *( ( DWORD * )&Pos1->Position.x ) != *( ( DWORD * )&Pos2->Position.x ) ||
				*( ( DWORD * )&Pos1->Position.y ) != *( ( DWORD * )&Pos2->Position.y ) ||
				*( ( DWORD * )&Pos1->Position.z ) != *( ( DWORD * )&Pos2->Position.z ) ) continue ;

			// スキニングウエイトが同じか調べる
			for( BoneNum1 = 0 ; BoneNum1 < Frame->UseSkinBoneNum && Pos1->BoneWeight[ BoneNum1 ].Index != -1 ; BoneNum1 ++ ){}
			for( BoneNum2 = 0 ; BoneNum2 < Frame->UseSkinBoneNum && Pos2->BoneWeight[ BoneNum2 ].Index != -1 ; BoneNum2 ++ ){}
			if( BoneNum1 != BoneNum2 ) continue ;
			for( k = 0 ; k < BoneNum1 ; k ++ )
			{
				if( Pos1->BoneWeight[ k ].Index              != Pos2->BoneWeight[ k ].Index ||
					*( ( DWORD * )&Pos1->BoneWeight[ k ].W ) != *( ( DWORD * )&Pos2->BoneWeight[ k ].W ) )
					break ;
			}
			if( k != BoneNum1 ) continue ;

			// 同じ頂点を発見したら番号の若い頂点に統合する
			Mesh = Frame->Mesh ;
			for( k = 0 ; k < Frame->MeshNum ; k ++, Mesh ++ )
			{
				Vert = Mesh->Vertex ;
				for( l = 0 ; l < Mesh->VertexNum ; l ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					if( Vert->PositionIndex != ( DWORD )j ) continue ;
					Vert->PositionIndex = ( DWORD )i ;
/*
					int n, o, p, q ;
					MV1_TRIANGLE_LIST_BASE *TTList ;
					MV1_FRAME_BASE *FFrame ;
					MV1_MESH_BASE *MMesh ;
					MV1_MESH_POSITION *PPos ;
					DWORD *MMInd ;
					MV1_MESH_VERTEX *VVert ;

					TTList = ModelBase->TriangleList ;
					for( o = 0 ; o < ModelBase->TriangleListNum ; o ++, TTList ++ )
					{
						MMesh = TTList->Container ;
						FFrame = MMesh->Container ;
						MMInd = TTList->MeshVertexIndex ;
						for( p = 0 ; p < TTList->VertexNum ; p ++, MMInd ++ )
						{
							VVert = ( MV1_MESH_VERTEX * )( ( BYTE * )MMesh->Vertex + *MMInd * MMesh->VertUnitSize ) ;
							PPos = ( MV1_MESH_POSITION * )( ( BYTE * )FFrame->Position + VVert->PositionIndex * FFrame->PosUnitSize ) ;
							for( n = 0 ; n < FFrame->UseSkinBoneNum && PPos->BoneWeight[ n ].Index != -1 ; n ++ )
							{
								for( m = 0 ; m < TTList->UseBoneNum && TTList->UseBone[ m ] != PPos->BoneWeight[ n ].Index ; m ++ ){}
								if( m == TTList->UseBoneNum )
								{
									m = 0 ;
								}
							}
						}
					}
*/
				}
			}

			// 無効フラグを立てる
			DisableFlag[ j ] = 1 ;

			// 減らした数を加算する
			SubNum ++ ;
		}
	}

	// 一つも減らない場合は何もしない
	if( SubNum != 0 )
	{
		// 減らした分だけ座標データサイズを減算する
		Size = Frame->PosUnitSize * SubNum ;
		ModelBase->MeshPositionSize -= Size ;
		ModelBase->VertexDataSize -= Size ;

		// 使用されている座標を列挙する
		NewNum = 0 ;
		Pos1 = Frame->Position ;
		Pos2 = Frame->Position ;
		for( i = 0 ; i < Frame->PositionNum ; i ++, Pos1 = ( MV1_MESH_POSITION * )( ( BYTE * )Pos1 + Frame->PosUnitSize ) )
		{
			if( DisableFlag[ i ] ) continue ;

			NewIndex[ i ] = NewNum ;
			_MEMCPY( Pos2, Pos1, ( size_t )Frame->PosUnitSize ) ;
			Pos2 = ( MV1_MESH_POSITION * )( ( BYTE * )Pos2 + Frame->PosUnitSize ) ;
			NewNum ++ ;
		}

		// 新しい座標数をセットする
		Frame->PositionNum = NewNum ;

		// 頂点データの座標番号を変更する
		Mesh = Frame->Mesh ;
		for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
		{
			Vert = Mesh->Vertex ;
			for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
			{
				Vert->PositionIndex = ( DWORD )NewIndex[ Vert->PositionIndex ] ;
			}
		}

		// 頂点情報の一致する頂点を列挙する
		Mesh = Frame->Mesh ;	
		_MEMSET( NewMVertIndex, 0xff, sizeof( int ) * MaxNum ) ;
		for( i = 0 ; i < Frame->MeshNum ; i ++, Mesh ++ )
		{
			SubNum = 0 ;
			Vert1 = Mesh->Vertex ;
			_MEMSET( DisableFlag, 0, sizeof( BYTE ) * MaxNum ) ;
			for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert1 = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert1 + Mesh->VertUnitSize ) )
			{
				// 既に無効だったら何もしない
				if( DisableFlag[ j ] ) continue ;

				Vert2 = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + Mesh->VertUnitSize * ( j + 1 ) ) ;
				for( k = j + 1 ; k < Mesh->VertexNum ; k ++, Vert2 = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert2 + Mesh->VertUnitSize ) )
				{
					// 既に無効だったら何もしない
					if( DisableFlag[ k ] ) continue ;

					// 情報が同じか調べる
					if( _MEMCMP( Vert1, Vert2, Mesh->VertUnitSize ) != 0 )
						continue ;

					// 同じ頂点を発見したら番号の若い頂点に統合する
					NewMVertIndex[ k ] = j ;
					Face = Mesh->Face ;
					for( l = 0 ; l < Mesh->FaceNum ; l ++, Face ++ )
					{
						if( Face->VertexIndex[ 0 ] == ( DWORD )k )
							Face->VertexIndex[ 0 ] = ( DWORD )j ;
						if( Face->VertexIndex[ 1 ] == ( DWORD )k )
							Face->VertexIndex[ 1 ] = ( DWORD )j ;
						if( Face->VertexIndex[ 2 ] == ( DWORD )k )
							Face->VertexIndex[ 2 ] = ( DWORD )j ;
					}
					TList = Mesh->TriangleList ;
					for( l = 0 ; l < Mesh->TriangleListNum ; l ++, TList ++ )
					{
						MInd1 = TList->MeshVertexIndex ;
						for( m = 0 ; m < TList->VertexNum ; m ++, MInd1 ++ )
						{
							if( *MInd1 == ( DWORD )k )
								*MInd1 = ( DWORD )j ;
						}
					}

					// 無効フラグを立てる
					DisableFlag[ k ] = 1 ;

					// 減らした数を加算する
					SubNum ++ ;
				}
			}

			// 一つも減らない場合は何もしない
			if( SubNum != 0 )
			{
				// シェイプメッシュがある場合はシェイプメッシュの対象頂点も変更する
				Shape = Frame->Shape ;
				for( j = 0 ; j < Frame->ShapeNum ; j ++, Shape ++ )
				{
					ShapeMesh = Shape->Mesh ;
					for( k = 0 ; k < Shape->MeshNum ; k ++, ShapeMesh ++ )
					{
						if( ShapeMesh->TargetMesh != Mesh ) continue ;

						ShapeVertex = ShapeMesh->Vertex ;
						for( l = 0 ; ( DWORD )l < ShapeMesh->VertexNum ; l ++, ShapeVertex ++ )
						{
							if( NewMVertIndex[ ShapeVertex->TargetMeshVertex ] == -1 ) continue ;
							ShapeVertex->TargetMeshVertex = ( DWORD )NewMVertIndex[ ShapeVertex->TargetMeshVertex ] ;
						}
					}
				}

				// 減らした分だけ頂点データサイズを減算する
				Size = Mesh->VertUnitSize * SubNum ;
				ModelBase->MeshVertexSize -= Size ;
				ModelBase->VertexDataSize -= Size ;
				Frame->VertexNum -= SubNum ;

				// 使用されている頂点を列挙する
				NewNum = 0 ;
				Vert1 = Mesh->Vertex ;
				Vert2 = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert1 = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert1 + Mesh->VertUnitSize ) )
				{
					if( DisableFlag[ j ] ) continue ;

					NewIndex[ j ] = NewNum ;
					_MEMCPY( Vert2, Vert1, ( size_t )Mesh->VertUnitSize ) ;
					Vert2 = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert2 + Mesh->VertUnitSize ) ;
					NewNum ++ ;
				}

				// 新しい座標数をセットする
				Mesh->VertexNum = NewNum ;

				// 頂点データの頂点番号を変更する
				Face = Mesh->Face ;
				for( j = 0 ; j < Mesh->FaceNum ; j ++, Face ++ )
				{
					Face->VertexIndex[ 0 ] = ( DWORD )NewIndex[ Face->VertexIndex[ 0 ] ] ;
					Face->VertexIndex[ 1 ] = ( DWORD )NewIndex[ Face->VertexIndex[ 1 ] ] ;
					Face->VertexIndex[ 2 ] = ( DWORD )NewIndex[ Face->VertexIndex[ 2 ] ] ;
				}
				TList = Mesh->TriangleList ;
				for( j = 0 ; j < Mesh->TriangleListNum ; j ++, TList ++ )
				{
					for( k = 0 ; k < TList->VertexNum ; k ++ )
						TList->MeshVertexIndex[ k ] = ( DWORD )NewIndex[ TList->MeshVertexIndex[ k ] ] ;
				}

				// トライアングルリストのシェーダー未使用時の頂点データを解放する
				TList = Mesh->TriangleList ;
				for( j = 0 ; j < Mesh->TriangleListNum ; j ++, TList ++ )
				{
					MV1_TerminateTriangleListBaseTempBuffer_PF( TList ) ;
				}

				// トライアングルリスト内のメッシュ頂点インデックスで重複している情報を列挙する
				TList = Mesh->TriangleList ;
				for( j = 0 ; j < Mesh->TriangleListNum ; j ++, TList ++ )
				{
					SubNum = 0 ;
					_MEMSET( DisableFlag, 0, sizeof( BYTE ) * TList->VertexNum ) ;
					MInd1 = TList->MeshVertexIndex ;
					for( k = 0 ; k < TList->VertexNum ; k ++, MInd1 ++ )
					{
						// 既に無効だったら何もしない
						if( DisableFlag[ k ] ) continue ;

						MInd2 = TList->MeshVertexIndex + k + 1 ;
						for( l = k + 1 ; l < TList->VertexNum ; l ++, MInd2 )
						{
							// 既に無効だったら何もしない
							if( DisableFlag[ l ] ) continue ;

							// 頂点情報が同じか調べる
							if( *MInd1 != *MInd2 ) continue ;

							// 同じ頂点を発見したら番号の若い頂点に統合する
							Ind = TList->Index ;
							for( m = 0 ; m < TList->IndexNum ; m ++, Ind ++ )
							{
								if( *Ind == l )
									*Ind = ( WORD )k ;
							}

							// 無効フラグを立てる
							DisableFlag[ l ] = 1 ;

							// 減らした数を加算する
							SubNum ++ ;
						}
					}

					// 一つも減らない場合は何もしない
					if( SubNum != 0 )
					{
						// 減らす分だけ頂点データサイズを減算する
						ModelBase->MeshVertexIndexNum -= SubNum ;
						ModelBase->TriangleListVertexNum -= SubNum ;
						ModelBase->VertexDataSize -= SubNum * sizeof( DWORD ) ;
						switch( TList->VertexType )
						{
						case MV1_VERTEX_TYPE_NORMAL :
							ModelBase->TriangleListNormalPositionNum -= SubNum ;
							ModelBase->VertexDataSize -= SubNum * sizeof( MV1_TLIST_NORMAL_POS ) ;
							break ;

						case MV1_VERTEX_TYPE_SKIN_4BONE :
							ModelBase->TriangleListSkinPosition4BNum -= SubNum ;
							ModelBase->VertexDataSize -= SubNum * sizeof( MV1_TLIST_SKIN_POS_4B ) ;
							break ;

						case MV1_VERTEX_TYPE_SKIN_8BONE :
							ModelBase->TriangleListSkinPosition8BNum -= SubNum ;
							ModelBase->VertexDataSize -= SubNum * sizeof( MV1_TLIST_SKIN_POS_8B ) ;
							break ;

						case MV1_VERTEX_TYPE_SKIN_FREEBONE :
							ModelBase->TriangleListSkinPositionFREEBSize -= TList->PosUnitSize * SubNum ;
							ModelBase->VertexDataSize -= TList->PosUnitSize * SubNum ;
							break ;
						}

						// 使用されているメッシュ頂点インデックスを列挙する
						NewNum = 0 ;
						MInd1 = TList->MeshVertexIndex ;
						MInd2 = TList->MeshVertexIndex ;
						TPos1 = ADDR16( TList->NormalPosition ) ;
						TPos2 = ADDR16( TList->NormalPosition ) ;
						for( k = 0 ; k < TList->VertexNum ; k ++, MInd1 ++, TPos1 += TList->PosUnitSize )
						{
							if( DisableFlag[ k ] ) continue ;

							NewIndex[ k ] = NewNum ;
							*MInd2 = *MInd1 ;
							_MEMCPY( TPos2, TPos1, TList->PosUnitSize ) ;
							MInd2 ++ ;
							TPos2 += TList->PosUnitSize ;
							NewNum ++ ;
						}

						// 新しいメッシュ頂点インデックス数をセットする
						TList->VertexNum = NewNum ;

						// 頂点データの頂点番号を変更する
						Ind = TList->Index ;
						for( k = 0 ; k < TList->IndexNum ; k ++, Ind ++ )
						{
							*Ind = ( WORD )NewIndex[ *Ind ] ;
						}
					}
				}
			}
		}
	}

	if( DisableFlag )
	{
		DXFREE( DisableFlag ) ;
		DisableFlag = NULL ;
	}

	// 正常終了
	return 0 ;

ERR:
	if( DisableFlag )
	{
		DXFREE( DisableFlag ) ;
		DisableFlag = NULL ;
	}

	// エラー
	return -1 ;
}

#if 0
// コンバインメッシュのセットアップを行う
static int _MV1SetupCombineMesh( MV1_FRAME_BASE *Frame )
{
	MV1_MESH_BASE *BaseMesh = NULL, *MBMesh ;
	MV1_FRAME_BASE *MBFrame ;
	int TotalPolygonNum = 0 ;
	int i, MeshFrameNum = 0 ;
	int TotalVertexNum = 0 ;
	int TotalIndexNum = 0 ;
	int TotalMeshVertexNum = 0 ;
	MV1_MESH_BASE *CMesh ;
	MV1_TRIANGLE_LIST_BASE *CTList, *MBTList ;
	int AllocMemSize ;

	// 既にセットアップ済みの場合は何もしない
	if( Frame->IsCombineMesh == true )
		return 0 ;

	// コンバインメッシュをセットアップする条件が合うか調べる

	// 子がいない場合はコンバインも何もなし
	if( Frame->TotalChildNum == 0 ) return 0 ;

	// メッシュがあり、非表示ではない場合のみ行うチェック
	if( Frame->MeshNum != 0 && Frame->Mesh->Visible == 1 )
	{
		// マテリアルを二つ以上使う場合は不適合
		if( Frame->MeshNum >= 2 ) return 0 ;

		// スキニングメッシュもシェイプメッシュも不適合
		if( Frame->IsSkinMesh || Frame->ShapeNum != 0 )
			return 0 ;

		// ポリゴン数や頂点やインデックスの数をセット
		TotalPolygonNum = Frame->TriangleNum ;
		TotalVertexNum = Frame->Mesh->TriangleList->VertexNum ;
		TotalIndexNum = Frame->Mesh->TriangleList->IndexNum ;
		TotalMeshVertexNum += Frame->Mesh->VertexNum ;

		// メッシュを持っているフレームの数を１にセット
		MeshFrameNum = 1 ;

		// 検査用メッシュのセット
		BaseMesh = Frame->Mesh ;
	}

	// 子にも同様のチェックをしながら総ポリゴン数を数える
	MBFrame = Frame->FirstChild ;
	for( i = 0 ; i < Frame->TotalChildNum ; i ++, MBFrame ++ )
	{
		// メッシュが無い場合は何もしない
		if( MBFrame->MeshNum == 0 ) continue ;
		MBMesh = MBFrame->Mesh ;

		// マテリアルを二つ以上使う場合は不適合
		if( MBFrame->MeshNum >= 2  )
			return 0 ;

		// スキニングメッシュやシェイプメッシュの場合も不適合
		if( MBFrame->IsSkinMesh || MBFrame->ShapeNum != 0 )
			return 0 ;

		// トライアングルリストの数が２個以上の場合も不適合
		if( MBMesh->TriangleListNum >= 2 )
			return 0 ;

		// 検査用メッシュが既に検出されているかどうかで処理を分岐
		if( BaseMesh == NULL )
		{
			BaseMesh = MBMesh ;
		}
		else
		{
			// 親と違うマテリアルの場合は不適合
			if( MBMesh->Material != BaseMesh->Material )
				return 0 ;

			// 親フレームのメッシュと頂点カラーの扱いが違ったら不適合
			if( MBMesh->UseVertexDiffuseColor  != BaseMesh->UseVertexDiffuseColor ||
				MBMesh->UseVertexSpecularColor != BaseMesh->UseVertexSpecularColor )
				return 0 ;

			// バックカリング設定が親フレームと違っていたら不適合
			if( MBMesh->BackCulling != BaseMesh->BackCulling )
				return 0 ;

			// 頂点一つ辺りのサイズが親フレームと違っていたら不適合
			if( MBMesh->VertUnitSize != BaseMesh->VertUnitSize )
				return 0 ;

			// 頂点データ一つに含まれるテクスチャ座標の数が違ったら不適合
			if( MBMesh->UVSetUnitNum != BaseMesh->UVSetUnitNum ||
				MBMesh->UVUnitNum    != BaseMesh->UVUnitNum )
				return 0 ;
		}

		// 非表示の場合は何もしない
		if( MBFrame->Mesh->Visible == 0 ) continue ;

		// ポリゴンと頂点とインデックスの総数を加算
		TotalPolygonNum += MBFrame->TriangleNum ;
		TotalVertexNum += MBMesh->TriangleList->VertexNum ;
		TotalIndexNum += MBMesh->TriangleList->IndexNum ;
		TotalMeshVertexNum += MBMesh->VertexNum ;

		// メッシュを持っているフレームの数をインクリメント
		MeshFrameNum ++ ;
	}

	// ポリゴンの総数が 5000 を超える場合は不適合
	if( TotalPolygonNum > 5000 ) return 0 ;

	// 頂点やインデックスの数が 32767 を超える場合は不適合
	if( TotalVertexNum > 32767 || TotalIndexNum > 32767 || TotalMeshVertexNum > 32767 ) return 0 ;

	// メッシュを持っているフレームがひとつ以下だった場合はコンバインメッシュを使う必要が無いので不適合
	if( MeshFrameNum <= 1 ) return 0 ;

	// 確保メモリのサイズを計算する
	AllocMemSize =
		TotalVertexNum     * ( BaseMesh->TriangleList->PosUnitSize + sizeof( DWORD ) ) +
		TotalIndexNum      * sizeof( unsigned short ) +
		TotalMeshVertexNum * BaseMesh->VertUnitSize +
		sizeof( MV1_TRIANGLE_LIST_BASE ) +
		sizeof( MV1_MESH_BASE ) +
		sizeof( MV1_VERTEXBUFFER ) ;

	// メモリの確保
	Frame->CombineMesh = ( MV1_MESH_BASE * )DXALLOC( AllocMemSize ) ;
	if( Frame->CombineMesh == NULL ) return 0 ;
	_MEMSET( Frame->CombineMesh, 0, AllocMemSize ) ;

	// メモリアドレスのセット
	CMesh = Frame->CombineMesh ;
	CTList = ( MV1_TRIANGLE_LIST_BASE * )( CMesh + 1 ) ;
	CMesh->VertexNum = TotalMeshVertexNum ;
	CMesh->Vertex = ( MV1_MESH_VERTEX * )( CTList + 1 ) ;
	CTList->MeshVertexIndex = ( DWORD * )( ( BYTE * )CMesh->Vertex + TotalMeshVertexNum * MBMesh->VertUnitSize ) ;
	CTList->NormalPosition = ( MV1_LIST_NORMAL_POS * )( ( BYTE * )

	// メッシュの情報をセット
	*CMesh = *BaseMesh ;
	CMesh->TriangleListNum = 1 ;
	CMesh->TriangleList = CTList ;
	CMesh->VertexAllocMem = FALSE ;
	CMesh->FaceNum = 0 ;
	CMesh->Face = NULL ;

	// トライアングルリストの情報をセット
	CTList->Container = CMesh ;
	CTList->VertexType = MBMesh->TriangleList->VertexType ;
	CTList->PosUnitSize = MBMesh->TriangleList->PosUnitSize ;
	CTList->TempUnitSize = MBMesh->TriangleList->TempUnitSize ;
	CTList->VertexNum = TotalVertexNum ;
	CTList->MaxBoneNum = 0 ;
	CTList->UseBoneNum = 0 ;

	// メッシュレンダリングに必要の無いデータはクリア
	TCMesh.VertexNum      = 0 ;
	TCMesh.VertUnitSize   = 0 ;
	TCMesh.Vertex         = NULL ;
	TCMesh.VertexAllocMem = FALSE ;
	TCMesh.FaceNum        = 0 ;
	TCMesh.Face           = NULL ;

	// トライアングルリストは一つだけ
	TCMesh.TriangleListNum = 1 ;

	// コンバイントライアングルリストの仮初期化
	_MEMSET( &TCTList, 0, sizeof( TCTList ) ) ;
	MBTList = BaseMesh->TriangleList ;
	TCTList.VertexType = MBTList->VertexType ;

	Frame->IsCombineMesh = TRUE ;

}

#endif

// 参照用メッシュの最大値と最小値を更新する
static void _MV1SetupReferenceMeshMaxAndMinPosition( MV1_REF_POLYGONLIST *PolyList )
{
	MV1_REF_POLYGON *Poly ;
	int i ;
	VECTOR *V0, *V1, *V2 ;

	Poly = PolyList->Polygons ;
	PolyList->MinPosition = PolyList->MaxPosition = PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position ;
	for( i = 0 ; i < PolyList->PolygonNum ; i ++, Poly ++ )
	{
		V0 = &PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position ;
		V1 = &PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position ;
		V2 = &PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ;
		if( V0->x > V1->x )
		{
			Poly->MaxPosition.x = V0->x < V2->x ? V2->x : V0->x ;
			Poly->MinPosition.x = V1->x > V2->x ? V2->x : V1->x ;
		}
		else
		{
			Poly->MaxPosition.x = V1->x < V2->x ? V2->x : V1->x ;
			Poly->MinPosition.x = V0->x > V2->x ? V2->x : V0->x ;
		}

		if( V0->y > V1->y )
		{
			Poly->MaxPosition.y = V0->y < V2->y ? V2->y : V0->y ;
			Poly->MinPosition.y = V1->y > V2->y ? V2->y : V1->y ;
		}
		else
		{
			Poly->MaxPosition.y = V1->y < V2->y ? V2->y : V1->y ;
			Poly->MinPosition.y = V0->y > V2->y ? V2->y : V0->y ;
		}

		if( V0->z > V1->z )
		{
			Poly->MaxPosition.z = V0->z < V2->z ? V2->z : V0->z ;
			Poly->MinPosition.z = V1->z > V2->z ? V2->z : V1->z ;
		}
		else
		{
			Poly->MaxPosition.z = V1->z < V2->z ? V2->z : V1->z ;
			Poly->MinPosition.z = V0->z > V2->z ? V2->z : V0->z ;
		}

		if( Poly->MaxPosition.x > PolyList->MaxPosition.x ) PolyList->MaxPosition.x = Poly->MaxPosition.x ;
		if( Poly->MinPosition.x < PolyList->MinPosition.x ) PolyList->MinPosition.x = Poly->MinPosition.x ;
		if( Poly->MaxPosition.y > PolyList->MaxPosition.y ) PolyList->MaxPosition.y = Poly->MaxPosition.y ;
		if( Poly->MinPosition.y < PolyList->MinPosition.y ) PolyList->MinPosition.y = Poly->MinPosition.y ;
		if( Poly->MaxPosition.z > PolyList->MaxPosition.z ) PolyList->MaxPosition.z = Poly->MaxPosition.z ;
		if( Poly->MinPosition.z < PolyList->MinPosition.z ) PolyList->MinPosition.z = Poly->MinPosition.z ;
	}
}

// マルチバイト文字名からワイド文字名を作成する
static bool _MV1CreateWideCharNameBase( MV1_MODEL_BASE *MBase, const char *NameA, wchar_t **NameWP )
{
	wchar_t TempName[ 512 ] ;

	ConvString( NameA, CHAR_CODEPAGE, ( char * )TempName, WCHAR_T_CODEPAGE ) ;
	*NameWP = ( wchar_t * )ADDMEMAREA( ( _WCSLEN( TempName ) + 1 ) * sizeof( wchar_t ), &MBase->AddFirstMem ) ;
	if( *NameWP == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xef\x30\xa4\x30\xc9\x30\x87\x65\x57\x5b\xdd\x4f\x58\x5b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x28\x00\x11\xff\x29\x00\x00"/*@ L"ワイド文字保存用のメモリ領域の確保に失敗しました(１)" @*/ )) ;
		return false ;
	}
	_WCSCPY( *NameWP, TempName ) ;

	return true ;
}

// ワイド文字名からマルチバイト文字名を作成する
static bool _MV1CreateMultiByteNameBase( MV1_MODEL_BASE *MBase, const wchar_t *NameW, char **NameAP )
{
	char TempName[ 512 ] ;

	ConvString( ( const char * )NameW, WCHAR_T_CODEPAGE, TempName, CHAR_CODEPAGE ) ;
	*NameAP = ( char * )ADDMEMAREA( ( size_t )( _STRLEN( TempName ) + 1 ), &MBase->AddFirstMem ) ;
	if( *NameAP == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xde\x30\xeb\x30\xc1\x30\xd0\x30\xa4\x30\xc8\x30\x87\x65\x57\x5b\xdd\x4f\x58\x5b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x28\x00\x11\xff\x29\x00\x00"/*@ L"マルチバイト文字保存用のメモリ領域の確保に失敗しました(１)" @*/ )) ;
		return false ;
	}
	_STRCPY( *NameAP, TempName ) ;

	return true ;
}

// ファイルパスからファイル名とディレクトリパスを作成する
static void _MV1CreateFileNameAndCurrentDirectory( const wchar_t *FilePath, wchar_t *Name, wchar_t *CurrentDirectory )
{
	wchar_t FileName2[ FILEPATH_MAX ], *cpt ;
	int i, Last = 0 ;

	// モデルファイルのあるディレクトリパスを作成する
	if( _WCSCHR( FilePath, L'\\' ) != NULL || _WCSCHR( FilePath, L'/' ) != NULL || _WCSCHR( FilePath, L':' ) != NULL )
	{
		wchar_t TempDirectory[ 1024 ] ;

		// 最後に『\』又は『/』が出てくるところを探す
		for( i = 0 ; FilePath[ i ] != L'\0' ; )
		{
			if( CHECK_WCHAR_T_DOUBLE( FilePath[ i ] ) )
			{
				i += 2 ;
			}
			else
			{
				if( FilePath[ i ] == L'\\' || 
					FilePath[ i ] == L'/' || 
					FilePath[ i ] == L':' ) Last = i ;
				i ++ ;
			}
		}

		// ディレクトリ指定部分のみをコピー
		_WCSNCPY( TempDirectory, FilePath, Last + 1 + 1 ) ;

		// 終端文字をセット
		if( ( TempDirectory[ Last ] == L'/' || TempDirectory[ Last ] == L'\\' ) && ( Last == 0 || TempDirectory[ Last - 1 ] != L':' ) )
		{
			TempDirectory[ Last ] = L'\0' ;
		}
		else
		{
			TempDirectory[ Last + 1 ] = L'\0' ;
		}

		// ディレクトリをフルパスにする
		ConvertFullPathW_( TempDirectory, CurrentDirectory ) ;

		// ファイル名部分のみをコピー
		_WCSCPY( FileName2, &FilePath[ Last + 1 ] ) ;
	}
	else
	{
		// カレントディレクトリを得る
		FGETDIR( CurrentDirectory ) ;

		// ファイル名をコピー
		_WCSCPY( FileName2, FilePath ) ;
	}

	// 拡張子を抜いたファイル名を得る
	_WCSCPY( Name, FileName2 ) ;
	cpt = ( wchar_t * )_WCSRCHR( Name, L'.' ) ;
	if( cpt ) *cpt = L'\0' ;
}

// マルチバイト文字名からワイド文字名を作成する
static bool _MV1CreateWideCharName( const char *NameA, wchar_t **NameWP )
{
	wchar_t TempName[ 512 ] ;

	ConvString( NameA, CHAR_CODEPAGE, ( char * )TempName, WCHAR_T_CODEPAGE ) ;

	*NameWP = ( wchar_t * )DXALLOC( ( _WCSLEN( TempName ) + 1 ) * sizeof( wchar_t ) ) ;
	if( *NameWP == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xef\x30\xa4\x30\xc9\x30\x87\x65\x57\x5b\xdd\x4f\x58\x5b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x28\x00\x12\xff\x29\x00\x00"/*@ L"ワイド文字保存用のメモリ領域の確保に失敗しました(２)" @*/ )) ;
		return false ;
	}
	_WCSCPY( *NameWP, TempName ) ;

	return true ;
}

// ワイド文字名からマルチバイト文字名を作成する
static bool _MV1CreateMultiByteName( const wchar_t *NameW, char **NameAP )
{
	char TempName[ 512 ] ;

	ConvString( ( const char * )NameW, WCHAR_T_CODEPAGE, TempName, CHAR_CODEPAGE ) ;

	*NameAP = ( char * )DXALLOC( ( size_t )( _STRLEN( TempName ) + 1 ) ) ;
	if( *NameAP == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xde\x30\xeb\x30\xc1\x30\xd0\x30\xa4\x30\xc8\x30\x87\x65\x57\x5b\xdd\x4f\x58\x5b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x28\x00\x12\xff\x29\x00\x00"/*@ L"マルチバイト文字保存用のメモリ領域の確保に失敗しました(２)" @*/ )) ;
		return false ;
	}
	_STRCPY( *NameAP, TempName ) ;

	return true ;
}

// マルチバイト文字名を新たにメモリを確保してコピーする
static bool _MV1AllocAndMultiByteNameCopy( const char *NameA, char **NameAP )
{
	*NameAP = ( char * )DXALLOC( ( _STRLEN( NameA ) + 1 ) * sizeof( char ) ) ;
	if( *NameAP == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xde\x30\xeb\x30\xc1\x30\xd0\x30\xa4\x30\xc8\x30\x87\x65\x57\x5b\xdd\x4f\x58\x5b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x28\x00\x13\xff\x29\x00\x00"/*@ L"マルチバイト文字保存用のメモリ領域の確保に失敗しました(３)" @*/ )) ;
		return false ;
	}
	_STRCPY( *NameAP, NameA ) ;

	return true ;
}

// ワイド文字名を新たにメモリを確保してコピーする
static bool _MV1AllocAndWideCharNameCopy( const wchar_t *NameW, wchar_t **NameWP )
{
	*NameWP = ( wchar_t * )DXALLOC( ( _WCSLEN( NameW ) + 1 ) * sizeof( wchar_t ) ) ;
	if( *NameWP == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xef\x30\xa4\x30\xc9\x30\x87\x65\x57\x5b\xdd\x4f\x58\x5b\x28\x75\x6e\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x28\x00\x13\xff\x29\x00\x00"/*@ L"ワイド文字保存用のメモリ領域の確保に失敗しました(３)" @*/ )) ;
		return false ;
	}
	_WCSCPY( *NameWP, NameW ) ;

	return true ;
}

extern int __MV1LoadTexture(
	void **ColorImage, int *ColorImageSize,
	void **AlphaImage, int *AlphaImageSize,
	int *GraphHandle,
	int *SemiTransFlag,
	int *DefaultTextureFlag,
#ifndef UNICODE
	char    **ColorFilePathAMem, char    **AlphaFilePathAMem,
#endif
	wchar_t **ColorFilePathWMem, wchar_t **AlphaFilePathWMem,
	const wchar_t *ColorFilePath, const wchar_t *AlphaFilePath, const wchar_t *StartFolderPath,
	int BumpImageFlag, float BumpImageNextPixelLength,
	int ReverseFlag,
	int Bmp32AllZeroAlphaToXRGB8Flag,
	const MV1_FILE_READ_FUNC *FileReadFunc,
	bool ValidImageAddr,
	int ASyncThread
	)
{
	BASEIMAGE ColorBaseImage, AlphaBaseImage ;
#ifndef UNICODE
	char TempPath[ 1024 ] ;
#endif
	int i/*, j*/, Result ;
	void *DataAddr ;

	// ポインタの初期化
	if( ValidImageAddr == false )
	{
#ifndef UNICODE
		if( ColorFilePathAMem ) *ColorFilePathAMem = NULL ;
		if( AlphaFilePathAMem ) *AlphaFilePathAMem = NULL ;
#endif
		if( ColorFilePathWMem ) *ColorFilePathWMem = NULL ;
		if( AlphaFilePathWMem ) *AlphaFilePathWMem = NULL ;
		*AlphaImage = NULL ;
		*ColorImage = NULL ;
	}
	*GraphHandle = -1 ;
	*DefaultTextureFlag = FALSE ;
	AlphaBaseImage.GraphData = NULL ;
	ColorBaseImage.GraphData = NULL ;

	if( ValidImageAddr == false )
	{
#ifndef UNICODE
		char    RelativePathA[ 1024 ] ;
		int StrLengthA ;
#endif
		wchar_t RelativePathW[ 1024 ] ;
		int StrLengthW ;

		// ファイルパスを保存するメモリ領域の確保
		if( ColorFilePath && ColorFilePathWMem && StartFolderPath )
		{
			CreateRelativePathW_( ColorFilePath, StartFolderPath, RelativePathW ) ;
			StrLengthW = _WCSLEN( RelativePathW ) ;

#ifndef UNICODE
			ConvString( ( const char * )RelativePathW, WCHAR_T_CODEPAGE, RelativePathA, CHAR_CODEPAGE ) ;
			StrLengthA = _STRLEN( RelativePathA ) ;

			*ColorFilePathAMem = ( char * )DXALLOC( ( size_t )( ( StrLengthA + 1 ) * sizeof( char ) ) ) ;
			if( *ColorFilePathAMem == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x28\x75\x6e\x30\xab\x30\xe9\x30\xfc\x30\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xd1\x30\xb9\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Load Texture Error : テクスチャ用のカラー画像ファイルパスを保存するメモリ領域の確保に失敗しました\n" @*/ )) ;
				goto ERRORLABEL ;
			}
			_STRCPY( *ColorFilePathAMem, RelativePathA ) ;
#endif
			*ColorFilePathWMem = ( wchar_t * )DXALLOC( ( size_t )( ( StrLengthW + 1 ) * sizeof( wchar_t ) ) ) ;
			if( *ColorFilePathWMem == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x28\x75\x6e\x30\xab\x30\xe9\x30\xfc\x30\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xd1\x30\xb9\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Load Texture Error : テクスチャ用のカラー画像ファイルパスを保存するメモリ領域の確保に失敗しました\n" @*/ )) ;
				goto ERRORLABEL ;
			}
			_WCSCPY( *ColorFilePathWMem, RelativePathW ) ;
		}
		if( AlphaFilePath && AlphaFilePathWMem && StartFolderPath )
		{
			CreateRelativePathW_( AlphaFilePath, StartFolderPath, RelativePathW ) ;
			StrLengthW = _WCSLEN( RelativePathW ) ;

#ifndef UNICODE
			ConvString( ( const char * )RelativePathW, WCHAR_T_CODEPAGE, RelativePathA, CHAR_CODEPAGE ) ;
			StrLengthA = _STRLEN( RelativePathA ) ;

			*AlphaFilePathAMem = ( char * )DXALLOC( ( size_t )( ( StrLengthA + 1 ) * sizeof( char ) ) ) ;
			if( AlphaFilePathAMem == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x28\x75\x6e\x30\xa2\x30\xeb\x30\xd5\x30\xa1\x30\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xd1\x30\xb9\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Load Texture Error : テクスチャ用のアルファ画像ファイルパスを保存するメモリ領域の確保に失敗しました\n" @*/ )) ;
				goto ERRORLABEL ;
			}
			_STRCPY( *AlphaFilePathAMem, RelativePathA ) ;
#endif

			*AlphaFilePathWMem = ( wchar_t * )DXALLOC( ( size_t )( ( StrLengthW + 1 ) * sizeof( wchar_t ) ) ) ;
			if( AlphaFilePathWMem == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x28\x75\x6e\x30\xa2\x30\xeb\x30\xd5\x30\xa1\x30\x3b\x75\xcf\x50\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xd1\x30\xb9\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Load Texture Error : テクスチャ用のアルファ画像ファイルパスを保存するメモリ領域の確保に失敗しました\n" @*/ )) ;
				goto ERRORLABEL ;
			}
			_WCSCPY( *AlphaFilePathWMem, RelativePathW ) ;
		}

		// カラーテクスチャファイルを読み込む
		*ColorImageSize = 0 ;
		if( ColorFilePath )
		{
			if( FileReadFunc )
			{
#ifdef UNICODE
				Result = FileReadFunc->Read( ColorFilePath, &DataAddr, ColorImageSize, FileReadFunc->Data ) ;
#else
				ConvString( ( const char * )ColorFilePath, WCHAR_T_CODEPAGE, TempPath, CHAR_CODEPAGE ) ;
				Result = FileReadFunc->Read( TempPath, &DataAddr, ColorImageSize, FileReadFunc->Data ) ;
#endif
				if( Result != -1 )
				{
					*ColorImage = DXALLOC( ( size_t )( *ColorImageSize ) ) ;
					if( *ColorImage == NULL )
					{
#ifndef DX_GCC_COMPILE
						DXST_ERRORLOGFMT_ADDW(( L"Load Texture Error : Color Channel Image File : Memory Alloc Error : %s\n", ColorFilePath ) ) ;
#endif
						Result = -1 ;
					}
					else
					{
						_MEMCPY( *ColorImage, DataAddr, ( size_t )( *ColorImageSize ) ) ;
					}
					if( FileReadFunc->Release )
					{
						FileReadFunc->Release( DataAddr, FileReadFunc->Data ) ;
					}
				}
			}
			else
			{
				Result = MV1RLoadFileW( ColorFilePath, ColorImage, ColorImageSize ) ;
			}

			if( Result == -1 )
			{
#ifndef DX_GCC_COMPILE
				DXST_ERRORLOGFMT_ADDW(( L"Load Texture Error : Color Channel Image File : Read Error : %s\n", ColorFilePath ) ) ;
#endif

				// 読み込みに失敗した場合はエラー時用テクスチャを充てる
				*ColorImage = DXALLOC( sizeof( Tga8x8TextureFileImage ) ) ;
				if( *ColorImage == NULL )
				{
					DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xa8\x30\xe9\x30\xfc\x30\xde\x56\x7f\x90\x28\x75\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"Load Texture Error : エラー回避用テクスチャを格納する領域の確保に失敗しました" @*/ )) ;
					goto ERRORLABEL ;
				}
				_MEMCPY( *ColorImage, Tga8x8TextureFileImage, sizeof( Tga8x8TextureFileImage ) ) ;
				*ColorImageSize = sizeof( Tga8x8TextureFileImage ) ;

				*DefaultTextureFlag = TRUE ;
			}
		}

		// アルファチャンネル用画像ファイルを読み込む
		*AlphaImageSize = 0 ;
		if( AlphaFilePath )
		{
			if( FileReadFunc )
			{
#ifdef UNICODE
				Result = FileReadFunc->Read( AlphaFilePath, &DataAddr, AlphaImageSize, FileReadFunc->Data ) ;
#else
				ConvString( ( const char * )AlphaFilePath, WCHAR_T_CODEPAGE, TempPath, CHAR_CODEPAGE ) ;
				Result = FileReadFunc->Read( TempPath, &DataAddr, AlphaImageSize, FileReadFunc->Data ) ;
#endif

				if( Result != -1 )
				{
					*AlphaImage = DXALLOC( ( size_t )( *AlphaImageSize ) ) ;
					if( *AlphaImage == NULL )
					{
#ifndef DX_GCC_COMPILE
						DXST_ERRORLOGFMT_ADDW(( L"Load Texture Error : Alpha Channel Image : Memory Alloc Error : %s\n", AlphaFilePath ) ) ;
#endif
						Result = -1 ;
					}
					else
					{
						_MEMCPY( *AlphaImage, DataAddr, ( size_t )( *AlphaImageSize ) ) ;
					}
					if( FileReadFunc->Release )
					{
						FileReadFunc->Release( DataAddr, FileReadFunc->Data ) ;
					}
				}
			}
			else
			{
				Result = MV1RLoadFileW( AlphaFilePath, AlphaImage, AlphaImageSize ) ;
			}

			if( Result == -1 )
			{
#ifndef DX_GCC_COMPILE
				DXST_ERRORLOGFMT_ADDW(( L"Load Texture Error : Alpha Channel Image : Read Error : %s\n", AlphaFilePath ) ) ;
#endif

				// 読み込みに失敗した場合はエラー時用テクスチャを充てる
				*AlphaImage = DXALLOC( sizeof( Tga8x8TextureFileImage ) ) ;
				if( *AlphaImage == NULL )
				{
					DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xa8\x30\xe9\x30\xfc\x30\xde\x56\x7f\x90\x28\x75\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"Load Texture Error : エラー回避用テクスチャを格納する領域の確保に失敗しました" @*/ )) ;
					goto ERRORLABEL ;
				}
				_MEMCPY( *AlphaImage, Tga8x8TextureFileImage, sizeof( Tga8x8TextureFileImage ) ) ;
				*AlphaImageSize = sizeof( Tga8x8TextureFileImage ) ;
			}
		}
		else
		{
			// αチャンネル用のファイルパスが無い場合はカラーテクスチャファイルの名前に _a を加えたファイルが無いか調べる
			wchar_t TempAlphaFilePath[ 1024 ] ;
			int len ;

			_WCSCPY( TempAlphaFilePath, ColorFilePath ) ;
			len = _WCSLEN( ColorFilePath ) ;
			for( i = len - 1 ; i >= 0 && ColorFilePath[ i ] != L'.' && ColorFilePath[ i ] != L'\0' && ColorFilePath[ i ] != L'\\' && ColorFilePath[ i ] != L':' ; i -- ){}
			if( ColorFilePath[ i ] == L'.' )
			{
				_WCSCPY( &TempAlphaFilePath[ i ], L"_a" ) ;
				_WCSCPY( &TempAlphaFilePath[ i + 2 ], &ColorFilePath[ i ] ) ;
			}
			else
			{
				_WCSCAT( TempAlphaFilePath, L"_a" ) ;
			}

			if( FileReadFunc )
			{
#ifdef UNICODE
				Result = FileReadFunc->Read( TempAlphaFilePath, &DataAddr, AlphaImageSize, FileReadFunc->Data ) ;
#else
				ConvString( ( const char * )TempAlphaFilePath, WCHAR_T_CODEPAGE, TempPath, CHAR_CODEPAGE ) ;
				Result = FileReadFunc->Read( TempPath, &DataAddr, AlphaImageSize, FileReadFunc->Data ) ;
#endif
				if( Result != -1 )
				{
					*AlphaImage = DXALLOC( ( size_t )( *AlphaImageSize ) ) ;
					if( *AlphaImage == NULL )
					{
#ifndef DX_GCC_COMPILE
						DXST_ERRORLOGFMT_ADDW(( L"Load Texture Error : Alpha Channel Image File : Memory Alloc Error : %s\n", AlphaFilePath ) ) ;
#endif
						Result = -1 ;
					}
					else
					{
						_MEMCPY( *AlphaImage, DataAddr, ( size_t )( *AlphaImageSize ) ) ;
					}
					if( FileReadFunc->Release )
					{
						FileReadFunc->Release( DataAddr, FileReadFunc->Data ) ;
					}
				}
			}
			else
			{
				Result = MV1RLoadFileW( TempAlphaFilePath, AlphaImage, AlphaImageSize ) ;
			}
		}
	}

	// BASEIMAGE を構築
	SetBmp32AllZeroAlphaToXRGB8( Bmp32AllZeroAlphaToXRGB8Flag ) ;
	if( MV1CreateTextureColorBaseImage(
				&ColorBaseImage,
				&AlphaBaseImage,
				*ColorImage, *ColorImageSize,
				*AlphaImage, *AlphaImageSize,
				BumpImageFlag, BumpImageNextPixelLength, ReverseFlag ) == -1 )
	{
		SetBmp32AllZeroAlphaToXRGB8( FALSE ) ;
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x28\x75\x6e\x30\x20\x00\x42\x00\x41\x00\x53\x00\x45\x00\x49\x00\x4d\x00\x41\x00\x47\x00\x45\x00\x20\x00\x6e\x30\x5c\x4f\x10\x62\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Load Texture Error : テクスチャ用の BASEIMAGE の作成に失敗しました\n" @*/ )) ;
		goto ERRORLABEL ;
	}
	SetBmp32AllZeroAlphaToXRGB8( FALSE ) ;

	// ハンドルを得る
	LOADGRAPH_GPARAM GParam ;
	Graphics_Image_InitLoadGraphGParam( &GParam ) ;
	GParam.LoadBaseImageGParam.ConvertPremultipliedAlpha = FALSE ;
	GParam.CreateGraphGParam.NotUseTransColor = TRUE ;
	GParam.CreateGraphGParam.InitHandleGParam.MipMapCount = -1 ;
	GParam.CreateGraphGParam.InitHandleGParam.NotInitGraphDelete = TRUE ;
	*GraphHandle = Graphics_Image_CreateGraphFromGraphImage_UseGParam( &GParam, FALSE, -1, &ColorBaseImage, *AlphaImage ? &AlphaBaseImage : NULL, TRUE, FALSE, FALSE, ASyncThread ) ;
	if( *GraphHandle < 0 )
	{
		NS_ReleaseBaseImage( &ColorBaseImage ) ;
		if( AlphaImage ) NS_ReleaseBaseImage( &AlphaBaseImage ) ;
		goto ERRORLABEL ;
	}

	// 半透明要素があるかどうかを調べる
	*SemiTransFlag = FALSE ;
	if( *AlphaImage ) *SemiTransFlag = TRUE ;
	if( *SemiTransFlag == FALSE && ColorBaseImage.ColorData.AlphaWidth != 0 )
	{
//		int r, g, b, a ;

		// フォーマットが標準フォーマットではなかったらDXTフォーマットで判断する
		if( ColorBaseImage.ColorData.Format != DX_BASEIMAGE_FORMAT_NORMAL )
		{
			//NS_ConvertNormalFormatBaseImage( &ColorBaseImage ) ;
			switch( ColorBaseImage.ColorData.Format )
			{
			case DX_BASEIMAGE_FORMAT_DXT1 :
				*SemiTransFlag = FALSE ;
				break ;

			case DX_BASEIMAGE_FORMAT_DXT2 :
				*SemiTransFlag = TRUE ;
				break ;

			case DX_BASEIMAGE_FORMAT_DXT3 :
				*SemiTransFlag = TRUE ;
				break ;

			case DX_BASEIMAGE_FORMAT_DXT4 :
				*SemiTransFlag = TRUE ;
				break ;

			case DX_BASEIMAGE_FORMAT_DXT5 :
				*SemiTransFlag = TRUE ;
				break ;
			}
		}
/*
		for( i = 0 ; i < ColorBaseImage.Height ; i ++ )
		{
			for( j = 0 ; j < ColorBaseImage.Width ; j ++ )
			{
				NS_GetPixelBaseImage( &ColorBaseImage, j, i, &r, &g, &b, &a ) ;
				if( a != 255 )
				{
					*SemiTransFlag = TRUE ;
					break ;
				}
			}
			if( *SemiTransFlag ) break ;
		}
*/
		else
		{
			// 全ピクセルを調べて半透明要素があるかどうかを調べる
			*SemiTransFlag = NS_CheckPixelAlphaBaseImage( &ColorBaseImage ) <= 1 ? 0 : 1 ; 
		}
	}

	// 基本イメージは解放
	NS_ReleaseBaseImage( &ColorBaseImage ) ;
	if( *AlphaImage ) NS_ReleaseBaseImage( &AlphaBaseImage ) ;

	// 終了
	return 0 ;

ERRORLABEL :
	if( ValidImageAddr == false )
	{
#ifndef UNICODE
		if( ColorFilePathAMem && *ColorFilePathAMem )
		{
			DXFREE( *ColorFilePathAMem ) ;
			*ColorFilePathAMem = NULL ;
		}
		if( AlphaFilePathAMem && *AlphaFilePathAMem )
		{
			DXFREE( *AlphaFilePathAMem ) ;
			*AlphaFilePathAMem = NULL ;
		}
#endif
		if( ColorFilePathWMem && *ColorFilePathWMem )
		{
			DXFREE( *ColorFilePathWMem ) ;
			*ColorFilePathWMem = NULL ;
		}
		if( AlphaFilePathWMem && *AlphaFilePathWMem )
		{
			DXFREE( *AlphaFilePathWMem ) ;
			*AlphaFilePathWMem = NULL ;
		}
		if( *ColorImage )
		{
			DXFREE( *ColorImage ) ;
			*ColorImage = NULL ;
		}
		if( *AlphaImage )
		{
			DXFREE( *AlphaImage ) ;
			*AlphaImage = NULL ;
		}
	}
	if( *GraphHandle != -1 )
	{
		NS_DeleteGraph( *GraphHandle ) ;
		*GraphHandle = -1 ;
	}

	// エラー終了
	return -1 ;
}


// 現在の設定でテクスチャを読み込む
static int _MV1TextureLoadBase(
	MV1_MODEL_BASE *ModelBase, MV1_TEXTURE_BASE *Texture,
	const wchar_t *ColorFilePathW, const wchar_t *AlphaFilePathW,
	int BumpImageFlag, float BumpImageNextPixelLength,
	bool ReverseFlag,
	bool Bmp32AllZeroAlphaToXRGB8Flag,
	int ASyncThread )
{
	void *ColorImage, *AlphaImage ;
	wchar_t *ColorFilePathWMem, *AlphaFilePathWMem ;
#ifndef UNICODE
	char    *ColorFilePathAMem, *AlphaFilePathAMem ;
#endif
	int ColorImageSize, AlphaImageSize ;
	int GraphHandle, SemiTransFlag, DefaultTextureFlag ;
	wchar_t *DirW ;
	int Result ;

	DirW = ModelBase->DirectoryPath ;

	// テクスチャの読み込み
	Result =  __MV1LoadTexture(
					&ColorImage, &ColorImageSize,
					&AlphaImage, &AlphaImageSize,
					&GraphHandle, &SemiTransFlag, &DefaultTextureFlag,
#ifndef UNICODE
					&ColorFilePathAMem, &AlphaFilePathAMem,
#endif
					&ColorFilePathWMem, &AlphaFilePathWMem,
					ColorFilePathW, AlphaFilePathW, DirW,
					BumpImageFlag, BumpImageNextPixelLength,
					ReverseFlag,
					Bmp32AllZeroAlphaToXRGB8Flag,
					NULL,
					false,
					ASyncThread
			) ;

	if( Result == -1 )
	{
		DXST_ERRORLOGFMT_ADDW(( L"Load Texture Error : Texture File : Load Error : %s\n", Texture->NameW ) ) ;
		return -1 ;
	}

	// 既にイメージを読み込んでいたらメモリを解放する
	if( Texture->ColorImage )
	{
		DXFREE( Texture->ColorImage ) ;
		Texture->ColorImage = NULL ;
	}
	if( Texture->AlphaImage )
	{
		DXFREE( Texture->AlphaImage ) ;
		Texture->AlphaImage = NULL ;
	}

	// 新しいメモリアドレスを保存
	Texture->ColorImage = ColorImage ;
	Texture->ColorImageSize = ColorImageSize ;

	Texture->AlphaImage = AlphaImage ;
	Texture->AlphaImageSize = AlphaImageSize ;

	// ファイルパスを保存
	if( Texture->ColorImage )
	{
		if( Texture->ColorImageFilePathAllocMem )
		{
#ifndef UNICODE
			if( Texture->ColorFilePathA )
			{
				DXFREE( Texture->ColorFilePathA ) ;
				Texture->ColorFilePathA = NULL ;
			}
#endif
			if( Texture->ColorFilePathW )
			{
				DXFREE( Texture->ColorFilePathW ) ;
				Texture->ColorFilePathW = NULL ;
			}
		}
#ifndef UNICODE
		Texture->ColorFilePathA = ColorFilePathAMem ;
#endif
		Texture->ColorFilePathW = ColorFilePathWMem ;
		Texture->ColorImageFilePathAllocMem = TRUE ;
	}
	if( Texture->AlphaImage )
	{
		if( Texture->AlphaImageFilePathAllocMem )
		{
#ifndef UNICODE
			if( Texture->AlphaFilePathA )
			{
				DXFREE( Texture->AlphaFilePathA ) ;
				Texture->AlphaFilePathA = NULL ;
			}
#endif
			if( Texture->AlphaFilePathW )
			{
				DXFREE( Texture->AlphaFilePathW ) ;
				Texture->AlphaFilePathW = NULL ;
			}
		}
#ifndef UNICODE
		Texture->AlphaFilePathA = AlphaFilePathAMem ;
#endif
		Texture->AlphaFilePathW = AlphaFilePathWMem ;
		Texture->AlphaImageFilePathAllocMem = TRUE ;
	}

	// 画像ハンドルの保存
	Texture->GraphHandle = GraphHandle ;

	// デフォルト画像が使用されたかどうかを保存
	Texture->IsDefaultTexture = DefaultTextureFlag ;

	// バンプマップ情報を保存
	Texture->BumpImageFlag = BumpImageFlag ;
	Texture->BumpImageNextPixelLength = BumpImageNextPixelLength ;

	// フラグを保存
	Texture->ReverseFlag = ReverseFlag ? 1 : 0 ;
	Texture->Bmp32AllZeroAlphaToXRGB8Flag = Bmp32AllZeroAlphaToXRGB8Flag ? 1 : 0 ;

	// 画像のサイズを取得
	NS_GetGraphSize( Texture->GraphHandle, &Texture->Width, &Texture->Height ) ;

	// 終了
	return 0 ;
}

static int _MV1TextureLoad(
	MV1_MODEL_BASE *ModelBase, MV1_TEXTURE *Texture,
	const wchar_t *ColorFilePathW, const wchar_t *AlphaFilePathW,
	int BumpImageFlag, float BumpImageNextPixelLength,
	bool ReverseFlag,
	bool Bmp32AllZeroAlphaToXRGB8Flag,
	int ASyncThread )
{
	void *ColorImage, *AlphaImage ;
	int ColorImageSize, AlphaImageSize ;
#ifndef UNICODE
	char    *ColorFilePathAMem, *AlphaFilePathAMem ;
#endif
	wchar_t *ColorFilePathWMem, *AlphaFilePathWMem ;
	int GraphHandle, SemiTransFlag, DefaultTextureFlag ;
	wchar_t *DirW ;
	int Result ;

	DirW = ModelBase->DirectoryPath ;

	// テクスチャの読み込み
	Result =  __MV1LoadTexture(
					&ColorImage, &ColorImageSize,
					&AlphaImage, &AlphaImageSize,
					&GraphHandle, &SemiTransFlag, &DefaultTextureFlag,
#ifndef UNICODE
					&ColorFilePathAMem, &AlphaFilePathAMem,
#endif
					&ColorFilePathWMem, &AlphaFilePathWMem,
					ColorFilePathW, AlphaFilePathW, DirW,
					BumpImageFlag, BumpImageNextPixelLength,
					ReverseFlag,
					Bmp32AllZeroAlphaToXRGB8Flag,
					NULL,
					false,
					ASyncThread ) ;

	if( Result == -1 )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4c\x00\x6f\x00\x61\x00\x64\x00\x20\x00\x54\x00\x65\x00\x78\x00\x74\x00\x75\x00\x72\x00\x65\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x6e\x30\xad\x8a\x7f\x30\xbc\x8f\x7f\x30\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"Load Texture Error : テクスチャの読み込みに失敗しました\n" @*/ )) ;
		return -1 ;
	}

	// 既にイメージを読み込んでいたらメモリを解放する
	if( Texture->UseGraphHandle )
	{
		if( Texture->ColorImage )
		{
			DXFREE( Texture->ColorImage ) ;
			Texture->ColorImage = NULL ;
		}
		if( Texture->AlphaImage )
		{
			DXFREE( Texture->AlphaImage ) ;
			Texture->AlphaImage = NULL ;
		}
#ifndef UNICODE
		if( Texture->ColorFilePathA_ )
		{
			DXFREE( Texture->ColorFilePathA_ ) ;
			Texture->ColorFilePathA_ = NULL ;
		}
		if( Texture->AlphaFilePathA_ )
		{
			DXFREE( Texture->AlphaFilePathA_ ) ;
			Texture->AlphaFilePathA_ = NULL ;
		}
#endif
		if( Texture->ColorFilePathW_ )
		{
			DXFREE( Texture->ColorFilePathW_ ) ;
			Texture->ColorFilePathW_ = NULL ;
		}
		if( Texture->AlphaFilePathW_ )
		{
			DXFREE( Texture->AlphaFilePathW_ ) ;
			Texture->AlphaFilePathW_ = NULL ;
		}
	}

	// 新しいメモリアドレスを保存
	Texture->ColorImage = ColorImage ;
	Texture->ColorImageSize = ColorImageSize ;

	Texture->AlphaImage = AlphaImage ;
	Texture->AlphaImageSize = AlphaImageSize ;

	// ファイルパスを保存
	if( Texture->ColorImage )
	{
#ifndef UNICODE
		Texture->ColorFilePathA_ = ColorFilePathAMem ;
#endif
		Texture->ColorFilePathW_ = ColorFilePathWMem ;
	}
	else
	{
#ifndef UNICODE
		Texture->ColorFilePathA_ = NULL ;
#endif
		Texture->ColorFilePathW_ = NULL ;
	}

	if( Texture->AlphaImage )
	{
#ifndef UNICODE
		Texture->AlphaFilePathA_ = AlphaFilePathAMem ;
#endif
		Texture->AlphaFilePathW_ = AlphaFilePathWMem ;
	}
	else
	{
#ifndef UNICODE
		Texture->AlphaFilePathA_ = NULL ;
#endif
		Texture->AlphaFilePathW_ = NULL ;
	}

	// 画像ハンドルの保存
	Texture->GraphHandle = GraphHandle ;
	Texture->UseGraphHandle = TRUE ;

	// デフォルト画像が使用されたかどうかを保存
	Texture->IsDefaultTexture = DefaultTextureFlag ;

	// バンプマップ情報を保存
	Texture->BumpImageFlag = BumpImageFlag ;
	Texture->BumpImageNextPixelLength = BumpImageNextPixelLength ;

	// フラグを保存
	Texture->ReverseFlag = ReverseFlag ? 1 : 0 ;
	Texture->Bmp32AllZeroAlphaToXRGB8Flag = Bmp32AllZeroAlphaToXRGB8Flag ? 1 : 0 ;

	// 画像のサイズを取得
	NS_GetGraphSize( Texture->GraphHandle, &Texture->Width, &Texture->Height ) ;

	// 終了
	return 0 ;
}

// モデルで使用しているマテリアルの数を取得する
extern int MV1GetMaterialNumBase( int MBHandle )
{
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// マテリアルの数を返す
	return ModelBase->MaterialNum ;
}

#ifndef UNICODE

// 指定のマテリアルの名前を取得する
extern	const char *MV1GetMaterialNameBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, NULL ) ;

	// 返す
	return Material->NameA ;
}

#endif

// 指定のマテリアルの名前を取得する
extern	const wchar_t *MV1GetMaterialNameBaseW( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, NULL ) ;

	// 返す
	return Material->NameW ;
}

// 指定のマテリアルのタイプを変更する( Type : DX_MATERIAL_TYPE_NORMAL など )
extern int MV1SetMaterialTypeBase( int MBHandle, int MaterialIndex, int Type )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までとタイプが同じだった場合は何もしない
	if( Material->Type == Type ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// タイプを保存
	Material->Type = Type ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのタイプを取得する( 戻り値 : DX_MATERIAL_TYPE_NORMAL など )
extern int MV1GetMaterialTypeBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// 返す
	return Material->Type ;
}

// 指定のマテリアルのディフューズカラーを取得する
extern	COLOR_F		MV1GetMaterialDifColorBase( int MBHandle, int MaterialIndex )
{
	COLOR_F RetColor = { 0.0f, 0.0f, 0.0f, 0.0f } ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, RetColor ) ;

	// 返す
	return Material->Diffuse ;
}

// 指定のマテリアルのスペキュラカラーを取得する
extern	COLOR_F		MV1GetMaterialSpcColorBase( int MBHandle, int MaterialIndex )
{
	COLOR_F RetColor = { 0.0f, 0.0f, 0.0f, 0.0f } ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, RetColor ) ;

	// 返す
	return Material->Specular ;
}

// 指定のマテリアルのエミッシブカラーを取得する
extern	COLOR_F		MV1GetMaterialEmiColorBase( int MBHandle, int MaterialIndex )
{
	COLOR_F RetColor = { 0.0f, 0.0f, 0.0f, 0.0f } ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, RetColor ) ;

	// 返す
	return Material->Emissive ;
}

// 指定のマテリアルのアンビエントカラーを取得する
extern	COLOR_F		MV1GetMaterialAmbColorBase( int MBHandle, int MaterialIndex )
{
	COLOR_F RetColor = { 0.0f, 0.0f, 0.0f, 0.0f } ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, RetColor ) ;

	// 返す
	return Material->Ambient ;
}

// 指定のマテリアルのスペキュラの強さを取得する
extern	float		MV1GetMaterialSpcPowerBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1.0f ) ;

	// 返す
	return Material->Power ;
}

// 指定のマテリアルでディフューズマップとして使用するテクスチャを指定する
extern int MV1SetMaterialDifMapTextureBase( int MBHandle, int MaterialIndex, int TexIndex )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < 0 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 今までディフューズマップが無かった場合は１にする
	if( Material->DiffuseLayerNum == 0 )
	{
		Material->DiffuseLayerNum = 1 ;
	}

	// 設定
	Material->DiffuseLayer[ 0 ].Texture = TexIndex ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでディフューズマップとして使用されているテクスチャのインデックスを取得する
extern int MV1GetMaterialDifMapTextureBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DiffuseLayerNum == 0 )
		return -1 ;

	return Material->DiffuseLayer[ 0 ].Texture ;
}

// 指定のマテリアルでスペキュラマップとして使用するテクスチャを指定する
extern int MV1SetMaterialSpcMapTextureBase( int MBHandle, int MaterialIndex, int TexIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex >= ModelBase->TextureNum )
		return -1 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// テクスチャインデックスが 0 以下だった場合はスペキュラマップを解除する
	if( TexIndex < 0 )
	{
		Material->SpecularLayerNum = 0 ;
	}
	else
	{
		// スペキュラマップの数が０だったら１にする
		if( Material->SpecularLayerNum == 0 )
		{
			Material->SpecularLayerNum = 1 ;
		}

		// 設定
		Material->SpecularLayer[ 0 ].Texture = TexIndex ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでスペキュラマップとして使用されているテクスチャのインデックスを取得する
extern int MV1GetMaterialSpcMapTextureBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->SpecularLayerNum == 0 )
		return -1 ;

	return Material->SpecularLayer[ 0 ].Texture ;
}

// 指定のマテリアルで法線マップとして使用するテクスチャを指定する
extern int MV1SetMaterialNormalMapTextureBase( int MBHandle, int MaterialIndex, int TexIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex >= ModelBase->TextureNum )
		return -1 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// テクスチャインデックスが 0 以下だった場合は法線マップを解除する
	if( TexIndex < 0 )
	{
		Material->NormalLayerNum = 0 ;
	}
	else
	{
		// 法線マップの数が０だったら１にする
		if( Material->NormalLayerNum == 0 )
		{
			Material->NormalLayerNum = 1 ;
		}

		// 設定
		Material->NormalLayer[ 0 ].Texture = TexIndex ;
	}

	// 頂点バッファの作り直し
	MV1_TerminateVertexBufferBase_PF( MBHandle ) ;
	MV1_SetupVertexBufferBase_PF( MBHandle ) ;

	// 終了
	return 0 ;
}

// 指定のマテリアルで法線マップとして使用されているテクスチャのインデックスを取得する
extern int NS_MV1GetMaterialNormalMapTextureBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->NormalLayerNum == 0 )
		return -1 ;

	return Material->NormalLayer[ 0 ].Texture ;
}


// 指定のマテリアルのディフューズカラーを設定する
extern int MV1SetMaterialDifColorBase( int MBHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Diffuse.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Diffuse.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Diffuse.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Diffuse.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Diffuse = Color ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのスペキュラカラーを設定する
extern int MV1SetMaterialSpcColorBase( int MBHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Specular.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Specular.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Specular.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Specular.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Specular = Color ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;

			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのエミッシブカラーを設定する
extern int MV1SetMaterialEmiColorBase( int MBHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Emissive.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Emissive.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Emissive.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Emissive.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Emissive = Color ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;

			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのアンビエントカラーを設定する
extern int MV1SetMaterialAmbColorBase( int MBHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Ambient.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Ambient.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Ambient.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Ambient.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Ambient = Color ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;

			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのスペキュラの強さを設定する
extern int MV1SetMaterialSpcPowerBase( int MBHandle, int MaterialIndex, float Power )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->Power == Power )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Power = Power ;

	// 終了
	return 0 ;
}


// 指定のマテリアルでトゥーンレンダリングのディフューズグラデーションマップとして使用するテクスチャを設定する
extern int MV1SetMaterialDifGradTextureBase( int MBHandle, int MaterialIndex, int TexIndex )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < -1 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	if( Material->DiffuseGradTexture == TexIndex )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->DiffuseGradTexture = TexIndex ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでトゥーンレンダリングのディフューズグラデーションマップとして使用するテクスチャを取得する
extern int MV1GetMaterialDifGradTextureBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->DiffuseGradTexture ;
}

// 指定のマテリアルでトゥーンレンダリングのスペキュラグラデーションマップとして使用するテクスチャを設定する
extern int MV1SetMaterialSpcGradTextureBase( int MBHandle, int MaterialIndex, int TexIndex )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < -1 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	if( Material->SpecularGradTexture == TexIndex )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SpecularGradTexture = TexIndex ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでトゥーンレンダリングのスペキュラグラデーションマップとして使用するテクスチャを取得する
extern int MV1GetMaterialSpcGradTextureBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->SpecularGradTexture ;
}

// 指定のマテリアルでトゥーンレンダリングのスフィアマップとして使用するテクスチャを設定する
extern int MV1SetMaterialSphereMapTextureBase( int MBHandle, int MaterialIndex, int TexIndex )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < -1 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	if( Material->SphereMapTexture == TexIndex )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SphereMapTexture = TexIndex ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでトゥーンレンダリングのスフィアマップとして使用するテクスチャを取得する
extern int MV1GetMaterialSphereMapTextureBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->SphereMapTexture ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するディフューズグラデーションマップとディフューズカラーの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int MV1SetMaterialDifGradBlendTypeBase( int MBHandle, int MaterialIndex, int BlendType )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DiffuseGradBlendType == BlendType )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->DiffuseGradBlendType = BlendType ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するディフューズグラデーションマップとディフューズカラーの合成方法を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int MV1GetMaterialDifGradBlendTypeBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->DiffuseGradBlendType ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスペキュラグラデーションマップとスペキュラカラーの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int MV1SetMaterialSpcGradBlendTypeBase( int MBHandle, int MaterialIndex, int BlendType )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->SpecularGradBlendType == BlendType )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SpecularGradBlendType = BlendType ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスペキュラグラデーションマップとスペキュラカラーの合成方法を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int MV1GetMaterialSpcGradBlendTypeBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->SpecularGradBlendType ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスフィアマップの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int MV1SetMaterialSphereMapBlendTypeBase( int MBHandle, int MaterialIndex, int BlendType )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->SphereMapBlendType == BlendType )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SphereMapBlendType = BlendType ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスフィアマップの合成方法を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int MV1GetMaterialSphereMapBlendTypeBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->SphereMapBlendType ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の太さを設定する( 0.0f ～ 1.0f )
extern int MV1SetMaterialOutLineWidthBase( int MBHandle, int MaterialIndex, float Width )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->OutLineWidth == Width )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->OutLineWidth = Width ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の太さを取得する( 0.0f ～ 1.0f )
extern float MV1GetMaterialOutLineWidthBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->OutLineWidth ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線のドット単位の太さを設定する
extern int MV1SetMaterialOutLineDotWidthBase( int MBHandle, int MaterialIndex, float Width )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->OutLineDotWidth == Width )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->OutLineDotWidth = Width ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線のドット単位の太さを取得する
extern float MV1GetMaterialOutLineDotWidthBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// パラメータを返す
	return Material->OutLineDotWidth ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の色を設定する
extern int MV1SetMaterialOutLineColorBase( int MBHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;

	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->OutLineColor.r == Color.r &&
		Material->OutLineColor.g == Color.g &&
		Material->OutLineColor.b == Color.b &&
		Material->OutLineColor.a == Color.a )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->OutLineColor = Color ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			// マテリアル情報も更新
			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の色を取得する
extern COLOR_F MV1GetMaterialOutLineColorBase( int MBHandle, int MaterialIndex )
{
	COLOR_F RetColor = { 0.0f, 0.0f, 0.0f, 0.0f } ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, RetColor ) ;

	// パラメータを返す
	return Material->OutLineColor ;
}

// 指定のマテリアルの描画ブレンドモードを設定する( DX_BLENDMODE_ALPHA 等 )
extern int MV1SetMaterialDrawBlendModeBase( int MBHandle, int MaterialIndex, int BlendMode )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DrawBlendMode == BlendMode )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// ブレンドモードのセット
	Material->DrawBlendMode = BlendMode ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルの描画ブレンドパラメータを設定する
extern int MV1SetMaterialDrawBlendParamBase( int MBHandle, int MaterialIndex, int BlendParam )
{
	MV1_MODEL *Model ;
	MV1_MESH_BASE * MBMesh ;
	MV1_MESH *Mesh ;
	int i ;
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DrawBlendParam == BlendParam )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// ブレンドパラメータのセット
	Material->DrawBlendParam = BlendParam ;

	// このマテリアルを使用しているメッシュのセットアップ完了フラグを倒す
	MBMesh = ModelBase->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++ )
	{
		if( MBMesh->Material != Material ) continue ;

		for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
		{
			Mesh = &Model->Mesh[ i ] ;
			MV1MESH_RESET_SEMITRANSSETUP( Mesh )

			if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
				MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
		}
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルの描画ブレンドモードを取得する( DX_BLENDMODE_ALPHA 等 )
extern int MV1GetMaterialDrawBlendModeBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// ブレンドモードを返す
	return Material->DrawBlendMode ;
}

// 指定のマテリアルの描画ブレンドパラメータを設定する
extern int MV1GetMaterialDrawBlendParamBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	// ブレンドパラメータを返す
	return Material->DrawBlendParam ;
}

// 指定のマテリアルの描画時のアルファテストの設定を行う( Enable:αテストを行うかどうか( TRUE:行う  FALSE:行わない( デフォルト )  Mode:テストモード( DX_CMP_GREATER等 )  Param:描画アルファ値との比較に使用する値( 0～255 ) )
extern int MV1SetMaterialDrawAlphaTestBase( int MBHandle, int MaterialIndex, int Enable, int Mode, int Param )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->UseAlphaTest == Enable &&
		Material->AlphaFunc == Mode &&
		Material->AlphaRef == Param )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// パラメータのセット
	Material->UseAlphaTest = Enable ;
	Material->AlphaFunc = Mode ;
	Material->AlphaRef = Param ;

	// 終了
	return 0 ;
}

// 指定のマテリアルの描画時のアルファテストを行うかどうかを取得する( 戻り値  TRUE:アルファテストを行う  FALSE:アルファテストを行わない )
extern int MV1GetMaterialDrawAlphaTestEnableBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->UseAlphaTest ;
}

// 指定のマテリアルの描画時のアルファテストのテストモードを取得する( 戻り値  テストモード( DX_CMP_GREATER等 ) )
extern int MV1GetMaterialDrawAlphaTestModeBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->AlphaFunc ;
}

// 指定のマテリアルの描画時のアルファテストの描画アルファ地との比較に使用する値( 0～255 )を取得する
extern int MV1GetMaterialDrawAlphaTestParamBase( int MBHandle, int MaterialIndex )
{
	MV1BASEMATERIALSTART( MBHandle, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->AlphaRef ;
}







// 基本データ内テクスチャ関係

// テクスチャの数を取得
extern int MV1GetTextureNumBase( int MBHandle )
{
	MV1_MODEL_BASE *ModelBase ;

	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// テクスチャの数を返す
	return ModelBase->TextureNum ;
}

// テクスチャの追加
extern int MV1AddTextureBase( 
	int MBHandle,
	const wchar_t *Name,
	const wchar_t *ColorFilePathW, const wchar_t *AlphaFilePathW,
	void * /*ColorFileImage*/, void * /*AlphaFileImage*/,
	int AddressModeU, int AddressModeV, int FilterMode,
	int BumpImageFlag, float BumpImageNextPixelLength,
	bool ReverseFlag,
	bool Bmp32AllZeroAlphaToXRGB8Flag,
	int ASyncThread )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_MODEL *Model ;
	MV1_TEXTURE_BASE *NewMBTexDim = NULL, *MBTexture = NULL ;
	MV1_TEXTURE *NewTexDim = NULL, *Texture ;
	int i, j ;
	wchar_t *ColorFilePathWMem = NULL, *AlphaFilePathWMem = NULL ;
#ifndef UNICODE
	char    *ColorFilePathAMem = NULL, *AlphaFilePathAMem = NULL ;
#endif
	wchar_t *DirW ;
	int Result ;

	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 新しいテクスチャの情報を格納するメモリ領域の確保
	NewMBTexDim = ( MV1_TEXTURE_BASE * )DXALLOC( sizeof( MV1_TEXTURE_BASE ) * ( ModelBase->TextureNum + 1 ) ) ;
	if( NewMBTexDim == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xb0\x65\x57\x30\x44\x30\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\xfa\x57\x2c\x67\xc5\x60\x31\x58\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"新しいテクスチャ基本情報を格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
		goto ERRORLABEL ;
	}

	DirW = ModelBase->DirectoryPath ;

	// 既存のデータを丸々コピー
	_MEMCPY( NewMBTexDim, ModelBase->Texture, sizeof( MV1_TEXTURE_BASE ) * ModelBase->TextureNum ) ;

	// 新しいテクスチャの情報を構築
	{
		MBTexture = NewMBTexDim + ModelBase->TextureNum ;
		_MEMSET( MBTexture, 0, sizeof( MV1_TEXTURE_BASE ) ) ;

		// ユーザーデータを初期化
		MBTexture->UserData[ 0 ] = 0 ;
		MBTexture->UserData[ 1 ] = 0 ;
		MBTexture->UseUserGraphHandle = 0 ;
		MBTexture->UserGraphHandle = 0 ;

		// 名前を保存
		if( Name == NULL )
		{
			Name = L"NoName" ;
		}
		MBTexture->NameAllocMem = TRUE ;
		MBTexture->NameW = ( wchar_t * )DXALLOC( ( size_t )( ( _WCSLEN( Name ) + 1 ) * sizeof( wchar_t ) ) ) ;
		_WCSCPY( MBTexture->NameW, Name ) ;

		// 反転フラグをセット
		MBTexture->ReverseFlag = ReverseFlag ? TRUE : FALSE ;

		// ３２ビットＢＭＰのアルファ値が全部０だったら XRGB8 として扱うかどうかのフラグをセットする
		MBTexture->Bmp32AllZeroAlphaToXRGB8Flag = Bmp32AllZeroAlphaToXRGB8Flag ? TRUE : FALSE ;

		// テクスチャの読み込み
		Result = __MV1LoadTexture(
				&MBTexture->ColorImage, &MBTexture->ColorImageSize,
				&MBTexture->AlphaImage, &MBTexture->AlphaImageSize,
				&MBTexture->GraphHandle,
				&MBTexture->SemiTransFlag,
				&MBTexture->IsDefaultTexture,
#ifndef UNICODE
				&ColorFilePathAMem, &AlphaFilePathAMem,
#endif
				&ColorFilePathWMem, &AlphaFilePathWMem,
				ColorFilePathW, AlphaFilePathW, DirW,
				BumpImageFlag, BumpImageNextPixelLength,
				MBTexture->ReverseFlag,
				MBTexture->Bmp32AllZeroAlphaToXRGB8Flag,
				NULL,
				false,
				ASyncThread ) ;

		if( Result == -1 )
		{
			DXST_ERRORLOGFMT_ADDW(( L"Read Model Convert Error : Texture File : Load Error : %s\n", Name ) ) ;
			goto ERRORLABEL ;
		}

		// ファイルパスを保存
		if( MBTexture->ColorImage )
		{
			MBTexture->ColorImageFilePathAllocMem = TRUE ;
			MBTexture->ColorFilePathW = ColorFilePathWMem ;
#ifndef UNICODE
			MBTexture->ColorFilePathA = ColorFilePathAMem ;
#endif
		}
		if( MBTexture->AlphaImage )
		{
			MBTexture->AlphaImageFilePathAllocMem = TRUE ;
			MBTexture->AlphaFilePathW = AlphaFilePathWMem ;
#ifndef UNICODE
			MBTexture->AlphaFilePathA = AlphaFilePathAMem ;
#endif
		}

		// アドレッシングモードのセット
		MBTexture->AddressModeU = AddressModeU ;
		MBTexture->AddressModeV = AddressModeV ;

		// ＵＶのスケール値をセット
		MBTexture->ScaleU = 1.0f ;
		MBTexture->ScaleV = 1.0f ;

		// フィルタリングモードのセット
		MBTexture->FilterMode = FilterMode ;

		// バンプマップ情報を保存
		MBTexture->BumpImageFlag = BumpImageFlag ;
		MBTexture->BumpImageNextPixelLength = BumpImageNextPixelLength ;

		// 画像のサイズを取得
		NS_GetGraphSize( MBTexture->GraphHandle, &MBTexture->Width, &MBTexture->Height ) ;
	}

	// このモデル基本データを使用しているモデル全てのテクスチャデータを増やす
	Model = ModelBase->UseFirst ;
	for( i = 0 ; i < ModelBase->UseNum ; i ++, Model = Model->UseBaseDataNext )
	{
		// 新しいテクスチャの情報を格納するメモリ領域の確保
		NewTexDim = ( MV1_TEXTURE * )DXALLOC( sizeof( MV1_TEXTURE ) * ( ModelBase->TextureNum + 1 ) ) ;
		if( NewTexDim == NULL )
		{
			DXST_ERRORLOGFMT_ADDUTF16LE(( "\xb0\x65\x57\x30\x44\x30\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\xc5\x60\x31\x58\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"新しいテクスチャ情報を格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
			goto ERRORLABEL ;
		}

		// 既存のデータを丸々コピー
		_MEMCPY( NewTexDim, Model->Texture, sizeof( MV1_TEXTURE ) * ModelBase->TextureNum ) ;

		// 新しいテクスチャの情報を構築
		{
			Texture = NewTexDim + ModelBase->TextureNum ;

			Texture->BaseData = MBTexture ;

#ifndef UNICODE
			Texture->AlphaFilePathA_ = MBTexture->AlphaFilePathA ;
#endif
			Texture->AlphaFilePathW_ = MBTexture->AlphaFilePathW ;
			Texture->AlphaImage = NULL ;
			Texture->AlphaImageSize = 0 ;

#ifndef UNICODE
			Texture->ColorFilePathA_ = MBTexture->ColorFilePathA ;
#endif
			Texture->ColorFilePathW_ = MBTexture->ColorFilePathW ;
			Texture->ColorImage = NULL ;
			Texture->ColorImageSize = 0 ;

			Texture->Width = 0 ;
			Texture->Height = 0 ;
			Texture->SemiTransFlag = 0 ;
			Texture->BumpImageFlag = MBTexture->BumpImageFlag ;
			Texture->BumpImageNextPixelLength = MBTexture->BumpImageNextPixelLength ;

			Texture->UseGraphHandle = 0 ;
			Texture->GraphHandle = -1 ;

			Texture->UseUserGraphHandle = MBTexture->UseUserGraphHandle ;
			Texture->UserGraphHandle = MBTexture->UserGraphHandle ;
			Texture->UserGraphWidth = MBTexture->UserGraphWidth ;
			Texture->UserGraphHeight = MBTexture->UserGraphHeight ;
			Texture->UserGraphHandleSemiTransFlag = MBTexture->UserGraphHandleSemiTransFlag ;

			Texture->AddressModeU = MBTexture->AddressModeU ;
			Texture->AddressModeV = MBTexture->AddressModeV ;
			Texture->ScaleU = MBTexture->ScaleU ;
			Texture->ScaleV = MBTexture->ScaleV ;
			Texture->FilterMode = MBTexture->FilterMode ;

			Texture->ReverseFlag = MBTexture->ReverseFlag ;
			Texture->Bmp32AllZeroAlphaToXRGB8Flag = MBTexture->Bmp32AllZeroAlphaToXRGB8Flag ;
		}

		// 既にテクスチャ用にメモリが確保されていたら解放
		if( Model->TextureAllocMem )
		{
			DXFREE( Model->Texture ) ;
			Model->Texture = NULL ;
		}

		// 新しい情報位置のセット
		Model->TextureAllocMem = TRUE ;
		Model->Texture = NewTexDim ;
	}

	// 既にテクスチャ用にメモリが確保されていたら解放
	if( ModelBase->TextureAllocMem )
	{
		DXFREE( ModelBase->Texture ) ;
		ModelBase->Texture = NULL ;
	}

	// 新しい情報位置のセット
	ModelBase->TextureAllocMem = TRUE ;
	ModelBase->Texture = NewMBTexDim ;

	// テクスチャの数を増やす
	ModelBase->TextureNum ++ ;

	// テクスチャ基本データへのアドレスを変更
	Model = ModelBase->UseFirst ;
	for( i = 0 ; i < ModelBase->UseNum ; i ++, Model = Model->UseBaseDataNext )
	{
		Texture = Model->Texture ;
		MBTexture = ModelBase->Texture ;
		for( j = 0 ; j < ModelBase->TextureNum ; j ++, Texture ++, MBTexture ++ ) 
		{
			Texture->BaseData = MBTexture ;
		}
	}

	// 正常終了
	return 0 ;

ERRORLABEL :
	if( NewMBTexDim )
	{
		DXFREE( NewMBTexDim ) ;
		NewMBTexDim = NULL ;
	}

	if( NewTexDim )
	{
		DXFREE( NewTexDim ) ;
		NewTexDim = NULL ;
	}

#ifndef UNICODE
	if( ColorFilePathAMem )
	{
		DXFREE( ColorFilePathAMem ) ;
		ColorFilePathAMem = NULL ;
	}

	if( AlphaFilePathAMem )
	{
		DXFREE( AlphaFilePathAMem ) ;
		AlphaFilePathAMem = NULL ;
	}
#endif

	if( ColorFilePathWMem )
	{
		DXFREE( ColorFilePathWMem ) ;
		ColorFilePathWMem = NULL ;
	}

	if( AlphaFilePathWMem )
	{
		DXFREE( AlphaFilePathWMem ) ;
		AlphaFilePathWMem = NULL ;
	}

	if( MBTexture )
	{
		if( MBTexture->AlphaFilePathW )
		{
#ifndef UNICODE
			DXFREE( MBTexture->AlphaFilePathA ) ;
			MBTexture->AlphaFilePathA = NULL ;
#endif
			DXFREE( MBTexture->AlphaFilePathW ) ;
			MBTexture->AlphaFilePathW = NULL ;

			DXFREE( MBTexture->AlphaImage ) ;
			MBTexture->AlphaImage = NULL ;
		}

		if( MBTexture->ColorFilePathW )
		{
#ifndef UNICODE
			DXFREE( MBTexture->ColorFilePathA ) ;
			MBTexture->ColorFilePathA = NULL ;
#endif
			DXFREE( MBTexture->ColorFilePathW ) ;
			MBTexture->ColorFilePathW = NULL ;

			DXFREE( MBTexture->ColorImage ) ;
			MBTexture->ColorImage = NULL ;
		}
	}

	return -1 ;
}

// テクスチャの削除
extern int MV1DeleteTextureBase( int MBHandle, int TexIndex )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_MODEL *Model ;
	MV1_MATERIAL_BASE *MBMaterial ;
	MV1_MATERIAL *Material ;
	int i, j, k ;

	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 指定のテクスチャインデックスが基本データと基本データを使用しているモデルデータで使用されていないか調べる
	MBMaterial = ModelBase->Material ;
	for( i = 0 ; i < ModelBase->MaterialNum ; i ++, MBMaterial ++ )
	{
		for( j = 0 ; j < MBMaterial->DiffuseLayerNum && TexIndex != MBMaterial->DiffuseLayer[ j ].Texture ; j ++ ){}
		if( j != MBMaterial->DiffuseLayerNum ) break ;

		for( j = 0 ; j < MBMaterial->SpecularLayerNum && TexIndex != MBMaterial->SpecularLayer[ j ].Texture ; j ++ ){}
		if( j != MBMaterial->SpecularLayerNum ) break ;

		for( j = 0 ; j < MBMaterial->NormalLayerNum && TexIndex != MBMaterial->NormalLayer[ j ].Texture ; j ++ ){}
		if( j != MBMaterial->NormalLayerNum ) break ;

		if( MBMaterial->DiffuseGradTexture == TexIndex ) break ;
		if( MBMaterial->SpecularGradTexture == TexIndex ) break ;
	}

	// 使用されていたらエラー
	if( i != ModelBase->MaterialNum )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4a\x52\x64\x96\x57\x30\x88\x30\x46\x30\x68\x30\x57\x30\x5f\x30\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x6f\x30\xfa\x57\x2c\x67\xc7\x30\xfc\x30\xbf\x30\x6e\x30\xde\x30\xc6\x30\xea\x30\xa2\x30\xeb\x30\x67\x30\x7f\x4f\x28\x75\x55\x30\x8c\x30\x66\x30\x44\x30\x7e\x30\x59\x30\x0a\x00\x00"/*@ L"削除しようとしたテクスチャは基本データのマテリアルで使用されています\n" @*/ )) ;
		return -1 ;
	}

	// 指定のテクスチャインデックスが基本データを使用しているモデルで使用されていないか調べる
	Model = ModelBase->UseFirst ;
	for( k = 0 ; k < ModelBase->UseNum ; k ++, Model = Model->UseBaseDataNext )
	{
		Material = Model->Material ;
		for( i = 0 ; i < ModelBase->MaterialNum ; i ++, Material ++ )
		{
			for( j = 0 ; j < Material->DiffuseLayerNum && TexIndex != Material->DiffuseLayer[ j ].Texture ; j ++ ){}
			if( j != Material->DiffuseLayerNum ) break ;

			for( j = 0 ; j < Material->SpecularLayerNum && TexIndex != Material->SpecularLayer[ j ].Texture ; j ++ ){}
			if( j != Material->SpecularLayerNum ) break ;
		}
		if( i != ModelBase->MaterialNum )
			break ;
	}

	// 使用されていたらエラー
	if( k != ModelBase->UseNum )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4a\x52\x64\x96\x57\x30\x88\x30\x46\x30\x68\x30\x57\x30\x5f\x30\xc6\x30\xaf\x30\xb9\x30\xc1\x30\xe3\x30\x6f\x30\xe2\x30\xc7\x30\xeb\x30\xc7\x30\xfc\x30\xbf\x30\x6e\x30\xde\x30\xc6\x30\xea\x30\xa2\x30\xeb\x30\x67\x30\x7f\x4f\x28\x75\x55\x30\x8c\x30\x66\x30\x44\x30\x7e\x30\x59\x30\x0a\x00\x00"/*@ L"削除しようとしたテクスチャはモデルデータのマテリアルで使用されています\n" @*/ )) ;
		return -1 ;
	}

	// テクスチャの数を減らす
	ModelBase->TextureNum -- ;

	// 使用されていないことが確認できたらデータ配列を縮める
	if( TexIndex != ModelBase->TextureNum )
	{
		_MEMMOVE( &ModelBase->Texture[ TexIndex ],
			      &ModelBase->Texture[ TexIndex + 1 ],
				  sizeof( MV1_TEXTURE_BASE ) * ( ModelBase->TextureNum - TexIndex ) ) ;

		Model = ModelBase->UseFirst ;
		for( i = 0 ; i < ModelBase->UseNum ; i ++, Model = Model->UseBaseDataNext )
		{
			_MEMMOVE( &Model->Texture[ TexIndex ],
					  &Model->Texture[ TexIndex + 1 ],
					  sizeof( MV1_TEXTURE ) * ( ModelBase->TextureNum - TexIndex ) ) ;
		}
	}

	// テクスチャインデックスを振りなおす
	MBMaterial = ModelBase->Material ;
	for( i = 0 ; i < ModelBase->MaterialNum ; i ++, MBMaterial ++ )
	{
		for( j = 0 ; j < MBMaterial->DiffuseLayerNum ; j ++ )
		{
			if( MBMaterial->DiffuseLayer[ j ].Texture > TexIndex )
				MBMaterial->DiffuseLayer[ j ].Texture -- ;
		}

		for( j = 0 ; j < MBMaterial->SpecularLayerNum ; j ++ )
		{
			if( MBMaterial->SpecularLayer[ j ].Texture > TexIndex )
				MBMaterial->SpecularLayer[ j ].Texture -- ;
		}

		for( j = 0 ; j < MBMaterial->NormalLayerNum ; j ++ )
		{
			if( MBMaterial->NormalLayer[ j ].Texture > TexIndex )
				MBMaterial->NormalLayer[ j ].Texture -- ;
		}
	}

	Model = ModelBase->UseFirst ;
	for( k = 0 ; k < ModelBase->UseNum ; k ++, Model = Model->UseBaseDataNext )
	{
		Material = Model->Material ;
		for( i = 0 ; i < ModelBase->MaterialNum ; i ++, Material ++ )
		{
			for( j = 0 ; j < Material->DiffuseLayerNum ; j ++ )
			{
				if( Material->DiffuseLayer[ j ].Texture > TexIndex )
					Material->DiffuseLayer[ j ].Texture -- ;
			}

			for( j = 0 ; j < Material->SpecularLayerNum ; j ++ )
			{
				if( Material->SpecularLayer[ j ].Texture > TexIndex )
					Material->SpecularLayer[ j ].Texture -- ;
			}
		}
	}

	// 終了
	return 0 ;
}

#ifndef UNICODE
// テクスチャの名前を取得
extern const char *MV1GetTextureNameBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, NULL ) ;

	// 値を返す
	return Texture->NameA ;
}
#endif

// テクスチャの名前を取得
extern const wchar_t *MV1GetTextureNameBaseW( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, NULL ) ;

	// 値を返す
	return Texture->NameW ;
}

// カラーテクスチャのファイルパスを変更する
extern int MV1SetTextureColorFilePathBaseW( int MBHandle, int TexIndex, const wchar_t *FilePathW )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return _MV1TextureLoadBase(
				ModelBase, Texture,
				FilePathW, Texture->AlphaFilePathW,
				Texture->BumpImageFlag, Texture->BumpImageNextPixelLength,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0,
				FALSE ) ;
}

// カラーテクスチャのファイルパスを取得
extern const wchar_t *MV1GetTextureColorFilePathBaseW( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, NULL ) ;

	// 値を返す
	return Texture->ColorFilePathW ;
}

// アルファテクスチャのファイルパスを変更する
extern int MV1SetTextureAlphaFilePathBaseW( int MBHandle, int TexIndex, const wchar_t *FilePathW )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return _MV1TextureLoadBase(
				ModelBase, Texture,
				Texture->ColorFilePathW, FilePathW,
				Texture->BumpImageFlag, Texture->BumpImageNextPixelLength,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0,
				FALSE ) ;
}

// アルファテクスチャのファイルパスを取得
extern const wchar_t *MV1GetTextureAlphaFilePathBaseW( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, NULL ) ;

	// 値を返す
	return Texture->AlphaFilePathW ;
}

// テクスチャで使用するグラフィックハンドルを変更する( GrHandle を -1 にすると解除 )
extern int MV1SetTextureGraphHandleBase( int MBHandle, int TexIndex, int GrHandle, int SemiTransFlag )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	// GrHandle が -1 かどうかで処理を分岐
	if( GrHandle == -1 )
	{
		Texture->UseUserGraphHandle = FALSE ;
		Texture->UserGraphHandle = 0 ;
	}
	else
	{
		Texture->UseUserGraphHandle = TRUE ;
		Texture->UserGraphHandle = GrHandle ;
		Texture->UserGraphHandleSemiTransFlag = SemiTransFlag ;
		NS_GetGraphSize( Texture->UserGraphHandle, &Texture->UserGraphWidth, &Texture->UserGraphHeight ) ;
	}

	// 終了
	return 0 ;
}

// テクスチャのグラフィックハンドルを取得する
extern int MV1GetTextureGraphHandleBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->UseUserGraphHandle == FALSE ? Texture->GraphHandle : Texture->UserGraphHandle ;
}

// テクスチャのアドレスモードを設定する
extern int MV1SetTextureAddressModeBase( int MBHandle, int TexIndex, int AddrUMode, int AddrVMode )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->AddressModeU == AddrUMode &&
		Texture->AddressModeV == AddrVMode )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Texture->AddressModeU = AddrUMode ;
	Texture->AddressModeV = AddrVMode ;

	// 終了
	return 0 ;
}

// テクスチャのＵ値のアドレスモードを取得する( 戻り値:DX_TEXADDRESS_WRAP 等 )
extern int MV1GetTextureAddressModeUBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->AddressModeU ;
}

// テクスチャのＶ値のアドレスモードを取得する( 戻り値:DX_TEXADDRESS_WRAP 等 )
extern int MV1GetTextureAddressModeVBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->AddressModeV ;
}

// テクスチャの幅を取得する
extern int MV1GetTextureWidthBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->UseUserGraphHandle ? Texture->UserGraphWidth : Texture->Width ;
}

// テクスチャの高さを取得する
extern int MV1GetTextureHeightBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->UseUserGraphHandle ? Texture->UserGraphHeight : Texture->Height ;
}

// テクスチャに半透明要素があるかどうかを取得する( 戻り値  TRUE:ある  FALSE:ない )
extern int MV1GetTextureSemiTransStateBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->UseUserGraphHandle ? Texture->UserGraphHandleSemiTransFlag : Texture->SemiTransFlag ;
}

// テクスチャで使用している画像がバンプマップかどうかを設定する
extern int MV1SetTextureBumpImageFlagBase( int MBHandle, int TexIndex, int Flag )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return _MV1TextureLoadBase(
				ModelBase, Texture,
				Texture->ColorFilePathW, Texture->AlphaFilePathW,
				Flag, Texture->BumpImageNextPixelLength,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0,
				FALSE ) ;
}

// テクスチャがバンプマップかどうかを取得する( 戻り値  TRUE:バンプマップ  FALSE:違う )
extern int MV1GetTextureBumpImageFlagBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->BumpImageFlag ;
}

// バンプマップ画像の場合の隣のピクセルとの距離を設定する
extern int MV1SetTextureBumpImageNextPixelLengthBase( int MBHandle, int TexIndex, float Length )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->BumpImageNextPixelLength == Length )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Texture->BumpImageNextPixelLength = Length ;

	return _MV1TextureLoadBase(
				ModelBase, Texture,
				Texture->ColorFilePathW, Texture->AlphaFilePathW,
				Texture->BumpImageFlag, Length,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0,
				FALSE ) ;
}

// バンプマップ画像の場合の隣のピクセルとの距離を取得する
extern float MV1GetTextureBumpImageNextPixelLengthBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1.0f ) ;

	return Texture->BumpImageNextPixelLength ;
}

// テクスチャのフィルタリングモードを設定する
extern int MV1SetTextureSampleFilterModeBase( int MBHandle, int TexIndex, int FilterMode )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->FilterMode == FilterMode )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Texture->FilterMode = FilterMode ;

	// 終了
	return 0 ;
}

// テクスチャのフィルタリングモードを取得する( 戻り値  MV1_TEXTURE_FILTER_MODE_POINT等 )
extern int MV1GetTextureSampleFilterModeBase( int MBHandle, int TexIndex )
{
	MV1BASETEXTURELSTART( MBHandle, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->FilterMode ;
}








// 指定のメッシュの頂点ディフューズカラーをマテリアルのディフューズカラーの代わりに使用するかどうかを設定する( TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int MV1SetMeshUseVertDifColorBase( int MBHandle, int MeshIndex, int UseFlag )
{
	MV1_MODEL *Model ;
	MV1_MESH *Mesh ;
	MV1BASEMESHSTART( MBHandle, ModelBase, MBMesh, MeshIndex, -1 ) ;

	// 今までとフラグが同じである場合は何もしない
	if( MBMesh->UseVertexDiffuseColor == UseFlag )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定を保存する
	MBMesh->UseVertexDiffuseColor = UseFlag ;

	// このメッシュを使用しているモデルの半透明情報有無情報のセットアップ完了フラグを倒す
	for( Model = ModelBase->UseFirst ; Model ; Model = Model->UseBaseDataNext )
	{
		Mesh = &Model->Mesh[ MeshIndex ] ;
		MV1MESH_RESET_SEMITRANSSETUP( Mesh )

		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のメッシュの頂点スペキュラカラーをマテリアルのスペキュラカラーの代わりに使用するかどうかを設定する( TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int MV1SetMeshUseVertSpcColorBase( int MBHandle, int MeshIndex, int UseFlag )
{
	MV1BASEMESHSTART( MBHandle, ModelBase, MBMesh, MeshIndex, -1 ) ;

	// 今までとフラグが同じである場合は何もしない
	if( MBMesh->UseVertexSpecularColor == UseFlag )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定を保存する
	MBMesh->UseVertexSpecularColor = UseFlag ;

	// 終了
	return 0 ;
}

// 指定のメッシュの頂点ディフューズカラーをマテリアルのディフューズカラーの代わりに使用するかどうかの設定を取得する( 戻り値  TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int MV1GetMeshUseVertDifColorBase( int MBHandle, int MeshIndex )
{
	MV1BASEMESHSTART( MBHandle, ModelBase, Mesh, MeshIndex, -1 ) ;

	return Mesh->UseVertexDiffuseColor ;
}

// 指定のメッシュの頂点スペキュラカラーをマテリアルのスペキュラカラーの代わりに使用するかどうかの設定を取得する( 戻り値  TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int MV1GetMeshUseVertSpcColorBase( int MBHandle, int MeshIndex )
{
	MV1BASEMESHSTART( MBHandle, ModelBase, Mesh, MeshIndex, -1 ) ;

	return Mesh->UseVertexSpecularColor ;
}

// 指定のメッシュがシェイプメッシュかどうかを取得する( 戻り値 TRUE:シェイプメッシュ  FALSE:通常メッシュ )
extern int MV1GetMeshShapeFlagBase( int MBHandle, int MeshIndex )
{
	MV1BASEMESHSTART( MBHandle, ModelBase, Mesh, MeshIndex, -1 ) ;

	return Mesh->Shape ;
}













// シェイプ関係

// モデルに含まれるシェイプの数を取得する
extern int MV1GetShapeNumBase( int MBHandle )
{
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// シェイプの数を返す
	return ModelBase->ShapeNum ;
}

// シェイプの名前からモデル中のシェイプのシェイプインデックスを取得する( 無かった場合は戻り値が-1 )
extern int MV1SearchShapeBase( int MBHandle, const wchar_t *ShapeName )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_SHAPE_BASE *MBShape ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1BMDLCHK( MBHandle, ModelBase ) )
		return -1 ;

	// 同名のシェイプを探す
	MBShape = ModelBase->Shape ;
	for( i = 0 ; i < ModelBase->ShapeNum && _WCSCMP( MBShape->NameW, ShapeName ) != 0 ; i ++, MBShape ++ ){}

	// シェイプのインデックスを返す
	return i == ModelBase->ShapeNum ? -2 : i ;
}


#ifndef UNICODE
// 指定シェイプの名前を取得する
extern const char *MV1GetShapeNameBaseA( int MBHandle, int ShapeIndex )
{
	MV1BASESHAPESTART( MBHandle, ModelBase, Shape, ShapeIndex, NULL ) ;

	// シェイプの名前を返す
	return Shape->NameA ;
}
#endif

// 指定シェイプの名前を取得する
extern const wchar_t *MV1GetShapeNameBaseW( int MBHandle, int ShapeIndex )
{
	MV1BASESHAPESTART( MBHandle, ModelBase, Shape, ShapeIndex, NULL ) ;

	// シェイプの名前を返す
	return Shape->NameW ;
}

// 指定シェイプが対象としているメッシュの数を取得する
extern int MV1GetShapeTargetMeshNumBase( int MBHandle, int ShapeIndex )
{
	MV1BASESHAPESTART( MBHandle, ModelBase, Shape, ShapeIndex, -1 ) ;

	return Shape->MeshNum ;
}

// 指定シェイプが対象としているメッシュのメッシュインデックスを取得する
extern int MV1GetShapeTargetMeshBase( int MBHandle, int ShapeIndex, int Index )
{
	MV1BASESHAPESTART( MBHandle, ModelBase, Shape, ShapeIndex, -1 ) ;

	if( Index < 0 || Shape->MeshNum <= Index )
	{
		return -1 ;
	}

	return ( int )( Shape->Mesh[ Index ].TargetMesh - ModelBase->Mesh ) ;
}
























// モデルデータハンドルの初期化
extern int InitializeModelHandle( HANDLEINFO * )
{
	// 特に何もしない
	return 0 ;
}

// モデルデータハンドルの後始末
extern int TerminateModelHandle( HANDLEINFO *HandleInfo )
{
	MV1_MODEL *Model = ( MV1_MODEL * )HandleInfo ;
	MV1_TEXTURE *Texture ;
	int i ;

	// 初期化直後でなければ解放処理を行う
	if( Model->BaseData != NULL )
	{
		// アニメーションのデタッチ
		for( i = 0 ; i < Model->AnimSetMaxNum ; i ++ )
			NS_MV1DetachAnim( Model->HandleInfo.Handle, i ) ;

		// 頂点データの解放
		MV1_TerminateVertexBuffer_PF( Model->HandleInfo.Handle ) ;

		// アニメーションポインタ格納用メモリの解放
		if( Model->AnimSet )
		{
			DXFREE( Model->AnimSet ) ;
			Model->AnimSet = NULL ;
		}

		// 参照用メッシュの解放
		NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, -1, FALSE, FALSE ) ;
		NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, -1, TRUE,  FALSE ) ;
		NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, -1, FALSE, TRUE ) ;
		NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, -1, TRUE,  TRUE ) ;
		for( i = 0 ; i < Model->BaseData->FrameNum ; i ++ )
		{
			NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, i, FALSE, FALSE ) ;
			NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, i, TRUE,  FALSE ) ;
			NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, i, FALSE, TRUE ) ;
			NS_MV1TerminateReferenceMesh( Model->HandleInfo.Handle, i, TRUE,  TRUE ) ;
		}

		// コリジョン情報の解放
		NS_MV1TerminateCollInfo( Model->HandleInfo.Handle, -1 ) ;
		for( i = 0 ; i < Model->BaseData->FrameNum ; i ++ )
		{
			NS_MV1TerminateCollInfo( Model->HandleInfo.Handle, i ) ;
		}

#ifndef DX_NON_BULLET_PHYSICS
		// 物理演算処理の後始末
		if( Model->BaseData->PhysicsRigidBodyNum != 0 )
			ReleasePhysicsObject_ModelPhysicsInfo( Model ) ;
#endif

		// テクスチャの削除処理
		Texture = Model->Texture ;
		for( i = 0 ; i < Model->BaseData->TextureNum ; i ++, Texture ++ )
		{
			if( Texture->UseGraphHandle )
			{
#ifndef UNICODE
				if( Texture->ColorFilePathA_ )
				{
					DXFREE( Texture->ColorFilePathA_ ) ;
					Texture->ColorFilePathA_ = NULL ;
				}

				if( Texture->AlphaFilePathA_ )
				{
					DXFREE( Texture->AlphaFilePathA_ ) ;
					Texture->AlphaFilePathA_ = NULL ;
				}
#endif
				if( Texture->ColorFilePathW_ )
				{
					DXFREE( Texture->ColorFilePathW_ ) ;
					Texture->ColorFilePathW_ = NULL ;
				}

				if( Texture->AlphaFilePathW_ )
				{
					DXFREE( Texture->AlphaFilePathW_ ) ;
					Texture->AlphaFilePathW_ = NULL ;
				}

				if( Texture->ColorImage )
				{
					DXFREE( Texture->ColorImage ) ;
					Texture->ColorImage = NULL ;
				}

				if( Texture->AlphaImage )
				{
					DXFREE( Texture->AlphaImage ) ;
					Texture->AlphaImage = NULL ;
				}

				NS_DeleteGraph( Texture->GraphHandle ) ;
				Texture->GraphHandle = -1 ;
				Texture->UseGraphHandle = 0 ;
			}
		}
		if( Model->TextureAllocMem )
		{
			DXFREE( Model->Texture ) ;
			Model->Texture = NULL ;
		}

		// リストからはずす
		if( Model->UseBaseDataNext )
		{
			Model->UseBaseDataNext->UseBaseDataPrev = Model->UseBaseDataPrev ;
		}
		if( Model->UseBaseDataPrev )
		{
			Model->UseBaseDataPrev->UseBaseDataNext = Model->UseBaseDataNext ;
		}
		if( Model->BaseData->UseFirst == Model )
		{
			Model->BaseData->UseFirst = Model->UseBaseDataNext ;
		}
		if( Model->BaseData->UseLast == Model )
		{
			Model->BaseData->UseLast = Model->UseBaseDataPrev ;
		}
		Model->BaseData->UseNum -- ;

		// モデル基本データの解放
		MV1SubModelBase( Model->BaseDataHandle ) ;
	}

	// データバッファの解放
	if( Model->DataBuffer )
	{
		DXFREE( Model->DataBuffer ) ;
		Model->DataBuffer = NULL ;
	}

	// 正常終了
	return 0 ;
}

// モデルデータを追加する( -1:エラー  0以上:モデルデータハンドル )
extern int MV1AddModel( int ASyncThread )
{
	return AddHandle( DX_HANDLETYPE_MODEL, ASyncThread, -1 ) ;
}

// モデル基データからモデルデータを構築する( -1:エラー 0:成功 )
extern int MV1MakeModel( int MV1ModelHandle, int MV1ModelBaseHandle, int ASyncThread )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *MBase ;
	MV1_FRAME *Frame ;
	MV1_FRAME_BASE *MBFrame ;
	MV1_MESH_BASE *MBMesh ;
	MV1_MESH *Mesh ;
	MV1_SHAPE_BASE *MBShape ;
	MV1_SHAPE *Shape ;
	MV1_TRIANGLE_LIST_BASE *MBTList ;
	MV1_TRIANGLE_LIST *TList ;
	MV1_MATERIAL *Material ;
	MV1_MATERIAL_BASE *MBMaterial ;
	MV1_MATERIAL_LAYER *Layer, *MBLayer ;
	MV1_TEXTURE_BASE *MBTexture ;
	MV1_TEXTURE *Texture ;
	MV1_PHYSICS_RIGIDBODY_BASE *MBPhysicsRigidBody ;
	MV1_PHYSICS_RIGIDBODY *PhysicsRigidBody ;
	MV1_PHYSICS_JOINT_BASE *MBPhysicsJoint ;
	MV1_PHYSICS_JOINT *PhysicsJoint ;
	int i, j, k ;
	DWORD Size, Num ;
	MV1_DRAW_MATERIAL InitDrawMat ;
	int ShapeNormalPositionNum ;
	int ShapeSkinPosition4BNum ;
	int ShapeSkinPosition8BNum ;
	int ShapeSkinPositionFREEBSize ;
	DWORD ChangeDrawMaterialFlagSize, ChangeMatrixFlagSize ;

	ShapeNormalPositionNum = 0 ;
	ShapeSkinPosition4BNum = 0 ;
	ShapeSkinPosition8BNum = 0 ;
	ShapeSkinPositionFREEBSize = 0 ;

	// 初期値のセット
	InitDrawMat.UseColorScale = false ;
	InitDrawMat.DiffuseScale.r = 1.0f ;
	InitDrawMat.DiffuseScale.g = 1.0f ;
	InitDrawMat.DiffuseScale.b = 1.0f ;
	InitDrawMat.DiffuseScale.a = 1.0f ;
	InitDrawMat.AmbientScale.r = 1.0f ;
	InitDrawMat.AmbientScale.g = 1.0f ;
	InitDrawMat.AmbientScale.b = 1.0f ;
	InitDrawMat.AmbientScale.a = 1.0f ;
	InitDrawMat.SpecularScale.r = 1.0f ;
	InitDrawMat.SpecularScale.g = 1.0f ;
	InitDrawMat.SpecularScale.b = 1.0f ;
	InitDrawMat.SpecularScale.a = 1.0f ;
	InitDrawMat.EmissiveScale.r = 1.0f ;
	InitDrawMat.EmissiveScale.g = 1.0f ;
	InitDrawMat.EmissiveScale.b = 1.0f ;
	InitDrawMat.EmissiveScale.a = 1.0f ;
	InitDrawMat.OpacityRate = 1.0f ;
	InitDrawMat.Visible = 1 ;

	// モデル基本データハンドルチェック
	if( MV1BMDLCHK( MV1ModelBaseHandle, MBase ) )
		return -1 ;

	// モデルデータハンドルチェック
	if( ASyncThread )
	{
		if( MV1MDLCHK_ASYNC( MV1ModelHandle, Model ) )
			return -1 ;
	}
	else
	{
		if( MV1MDLCHK( MV1ModelHandle, Model ) )
			return -1 ;
	}

	// モデルデータを格納するメモリ領域の確保
	ChangeDrawMaterialFlagSize = ( DWORD )( ( 1 + MBase->FrameNum + MBase->MeshNum + 31 ) / 32 ) ;
	ChangeMatrixFlagSize       = ( DWORD )( ( 1 + MBase->FrameNum                  + 31 ) / 32 ) ;
	Size = 
		sizeof( MV1_FRAME             ) * MBase->FrameNum * 2             +
		sizeof( MATRIX_4X4CT          ) * MBase->SkinBoneNum              + 16 +
		sizeof( MATRIX_4X4CT_F        ) * ( MBase->PackDrawMatrixUnitNum * MBase->PackDrawMaxNum ) +
		sizeof( MATRIX_4X4CT *        ) * MBase->FrameUseSkinBoneNum      +
		sizeof( MV1_MESH              ) * MBase->MeshNum                  + 
		sizeof( MV1_SHAPE             ) * MBase->ShapeNum                 + 
		( sizeof( MV1_TRIANGLE_LIST ) + sizeof( MV1_TRIANGLE_LIST_PF ) ) * MBase->TriangleListNum          +
		sizeof( MV1_MATERIAL          ) * MBase->MaterialNum              +
		sizeof( MV1_TEXTURE           ) * MBase->TextureNum               +
		sizeof( MV1_TLIST_NORMAL_POS  ) * MBase->ShapeNormalPositionNum   + 16 +
		sizeof( MV1_TLIST_SKIN_POS_4B ) * MBase->ShapeSkinPosition4BNum   +
		sizeof( MV1_TLIST_SKIN_POS_8B ) * MBase->ShapeSkinPosition8BNum   +
		MBase->ShapeSkinPositionFREEBSize + 
		ChangeDrawMaterialFlagSize * 4    +
		ChangeMatrixFlagSize       * 4    +
		sizeof( MV1_SHAPE_VERTEX      ) * MBase->ShapeTargetMeshVertexNum +
		sizeof( MV1_PHYSICS_RIGIDBODY ) * MBase->PhysicsRigidBodyNum      +
		sizeof( MV1_PHYSICS_JOINT     ) * MBase->PhysicsJointNum ;
	Model->DataBuffer = ( MV1_MODEL * )MDALLOCMEM( Size ) ;
	if( Model->DataBuffer == NULL ) return -1 ;
	_MEMSET( Model->DataBuffer, 0, Size ) ;
	Model->Frame                    = ( MV1_FRAME                * )( Model->DataBuffer                                            ) ;
	Model->Mesh                     = ( MV1_MESH                 * )( Model->Frame                    + MBase->FrameNum * 2        ) ;
	Model->Shape                    = ( MV1_SHAPE                * )( Model->Mesh                     + MBase->MeshNum             ) ;
	Model->TriangleList             = ( MV1_TRIANGLE_LIST        * )( Model->Shape                    + MBase->ShapeNum            ) ;
	Model->Material                 = ( MV1_MATERIAL             * )( ( BYTE * )Model->TriangleList + ( sizeof( MV1_TRIANGLE_LIST ) + sizeof( MV1_TRIANGLE_LIST_PF ) ) * MBase->TriangleListNum ) ;
	Model->Texture                  = ( MV1_TEXTURE              * )( Model->Material                 + MBase->MaterialNum         ) ;
	Model->PhysicsRigidBody         = ( MV1_PHYSICS_RIGIDBODY    * )( Model->Texture                  + MBase->TextureNum          ) ;
	Model->PhysicsJoint             = ( MV1_PHYSICS_JOINT        * )( Model->PhysicsRigidBody         + MBase->PhysicsRigidBodyNum ) ;
	Model->ChangeDrawMaterialFlag   = ( DWORD                    * )( Model->PhysicsJoint             + MBase->PhysicsJointNum     ) ;
	Model->ChangeMatrixFlag         = ( DWORD                    * )( Model->ChangeDrawMaterialFlag   + ChangeDrawMaterialFlagSize ) ;
	Model->SkinBoneMatrix           = ( MATRIX_4X4CT             * )( ( ( DWORD_PTR )( Model->ChangeMatrixFlag + ChangeMatrixFlagSize ) + 15 ) / 16 * 16 ) ;
	Model->PackDrawMatrix           = ( MATRIX_4X4CT_F           * )( Model->SkinBoneMatrix           + MBase->SkinBoneNum         ) ;
	Model->SkinBoneUseFrameMatrix   = ( MATRIX_4X4CT            ** )( Model->PackDrawMatrix           + MBase->PackDrawMatrixUnitNum * MBase->PackDrawMaxNum ) ;
	Model->ShapeVertex              = ( MV1_SHAPE_VERTEX         * )( Model->SkinBoneUseFrameMatrix   + MBase->FrameUseSkinBoneNum ) ;
	Model->ShapeNormalPosition      = ( MV1_TLIST_NORMAL_POS     * )( ( ( DWORD_PTR )( Model->ShapeVertex + MBase->ShapeTargetMeshVertexNum ) + 15 ) / 16 * 16 ) ;
	Model->ShapeSkinPosition4B      = ( MV1_TLIST_SKIN_POS_4B    * )( Model->ShapeNormalPosition      + MBase->ShapeNormalPositionNum ) ;
	Model->ShapeSkinPosition8B      = ( MV1_TLIST_SKIN_POS_8B    * )( Model->ShapeSkinPosition4B      + MBase->ShapeSkinPosition4BNum ) ;
	Model->ShapeSkinPositionFREEB   = ( MV1_TLIST_SKIN_POS_FREEB * )( Model->ShapeSkinPosition8B      + MBase->ShapeSkinPosition8BNum ) ;

	// リストにつなぐ
	if( MBase->UseFirst )
	{
		Model->UseBaseDataNext = MBase->UseFirst ;
		MBase->UseFirst->UseBaseDataPrev = Model ;
		Model->UseBaseDataPrev = NULL ;
		MBase->UseFirst = Model ;
	}
	else
	{
		MBase->UseFirst = Model ;
		MBase->UseLast = Model ;
		Model->UseBaseDataNext = NULL ;
		Model->UseBaseDataPrev = NULL ;
	}
	MBase->UseNum ++ ;

	// モデル情報をセット
	Model->BaseDataHandle = MV1ModelBaseHandle ;
	Model->BaseData = MBase ;

	// アニメーションは最初アタッチされていないのでセットアップは完了している
	Model->AnimSetupFlag = true ;

	// シェイプ情報は最初は必ず更新する
	Model->ShapeChangeFlag = true ;

	// 重力パラメータをセット
	Model->PhysicsGravity = MBase->PhysicsGravity ;

	// スケールは等倍
	Model->Scale.x = 1.0f ;
	Model->Scale.y = 1.0f ;
	Model->Scale.z = 1.0f ;

	// メッシュカテゴリ毎の非表示フラグをセット
	Model->MeshCategoryHide[ DX_MV1_MESHCATEGORY_NORMAL ] = FALSE ;
	Model->MeshCategoryHide[ DX_MV1_MESHCATEGORY_OUTLINE ] = FALSE ;
	Model->MeshCategoryHide[ DX_MV1_MESHCATEGORY_OUTLINE_ORIG_SHADER ] = TRUE ;

	// Ｚバッファ関係をセット
	Model->EnableZBufferFlag = TRUE ;
	Model->WriteZBufferFlag = TRUE ;
	Model->ZBufferCmpType = DX_CMP_LESSEQUAL ;
	Model->ZBias = 0 ;

	// 異方性フィルタリングの最大次数の初期値は２
	Model->MaxAnisotropy = 2 ;

	// デフォルトではワイヤーフレーム描画はしない
	Model->WireFrame = false ;

	// モデルの共通マテリアル関係を初期化
	Model->DrawMaterial = InitDrawMat ;

	// 状態変更された状態にする
	Model->ChangeDrawMaterialFlagSize = ChangeDrawMaterialFlagSize * 4 ;
	Model->ChangeMatrixFlagSize       = ChangeMatrixFlagSize       * 4 ;
	_MEMSET( Model->ChangeDrawMaterialFlag, 0xff, Model->ChangeDrawMaterialFlagSize ) ;
	_MEMSET( Model->ChangeMatrixFlag,       0xff, Model->ChangeMatrixFlagSize       ) ;

	// テクスチャ情報のコピー
	Model->TextureAllocMem = FALSE ;
	MBTexture = MBase->Texture ;
	Texture = Model->Texture ;
	for( i = 0 ; i < MBase->TextureNum ; i ++, MBTexture ++, Texture ++ )
	{
		Texture->BaseData = MBTexture ;

#ifndef UNICODE
		Texture->AlphaFilePathA_ = MBTexture->AlphaFilePathA ;
#endif
		Texture->AlphaFilePathW_ = MBTexture->AlphaFilePathW ;
		Texture->AlphaImage = NULL ;
		Texture->AlphaImageSize = 0 ;

#ifndef UNICODE
		Texture->ColorFilePathA_ = MBTexture->ColorFilePathA ;
#endif
		Texture->ColorFilePathW_ = MBTexture->ColorFilePathW ;
		Texture->ColorImage = NULL ;
		Texture->ColorImageSize = 0 ;

		Texture->Width = 0 ;
		Texture->Height = 0 ;
		Texture->SemiTransFlag = 0 ;
		Texture->BumpImageFlag = MBTexture->BumpImageFlag ;
		Texture->BumpImageNextPixelLength = MBTexture->BumpImageNextPixelLength ;

		Texture->UseGraphHandle = 0 ;
		Texture->GraphHandle = -1 ;

		Texture->UseUserGraphHandle = MBTexture->UseUserGraphHandle ;
		Texture->UserGraphHandle = MBTexture->UserGraphHandle ;
		Texture->UserGraphWidth = MBTexture->UserGraphWidth ;
		Texture->UserGraphHeight = MBTexture->UserGraphHeight ;
		Texture->UserGraphHandleSemiTransFlag = MBTexture->UserGraphHandleSemiTransFlag ;

		Texture->AddressModeU = MBTexture->AddressModeU ;
		Texture->AddressModeV = MBTexture->AddressModeV ;
		Texture->ScaleU = MBTexture->ScaleU ;
		Texture->ScaleV = MBTexture->ScaleV ;
		Texture->FilterMode = MBTexture->FilterMode ;

		Texture->ReverseFlag = MBTexture->ReverseFlag ;
		Texture->Bmp32AllZeroAlphaToXRGB8Flag = MBTexture->Bmp32AllZeroAlphaToXRGB8Flag ;
	}

	// マテリアル情報のコピー
	MBMaterial = MBase->Material ;
	Material = Model->Material ;
	for( i = 0 ; i < MBase->MaterialNum ; i ++, MBMaterial ++, Material ++ )
	{
		Material->BaseData = MBMaterial ;

//		Material->Type = MBMaterial->Type ;

		Material->Diffuse = MBMaterial->Diffuse ;
		Material->Ambient = MBMaterial->Ambient ;
		Material->Specular = MBMaterial->Specular ;
		Material->Emissive = MBMaterial->Emissive ;
		Material->Power = MBMaterial->Power ;

		Material->DiffuseLayerNum = MBMaterial->DiffuseLayerNum ;
		Layer = Material->DiffuseLayer ;
		MBLayer = MBMaterial->DiffuseLayer ;
		for( j = 0 ; j < MBMaterial->DiffuseLayerNum ; j ++, Layer ++, MBLayer ++ )
		{
			Layer->IsGraphHandleAlpha = MBLayer->IsGraphHandleAlpha ;
			Layer->GraphHandle        = MBLayer->GraphHandle ;
			Layer->BlendType          = MBLayer->BlendType ;
			Layer->Texture            = MBLayer->Texture ;
//			MV1AddTexture( MBase->Texture[ Layer->Texture ] ) ;
		}

		Material->SpecularLayerNum = MBMaterial->SpecularLayerNum ;
		Layer = Material->SpecularLayer ;
		MBLayer = MBMaterial->SpecularLayer ;
		for( j = 0 ; j < MBMaterial->SpecularLayerNum ; j ++, Layer ++, MBLayer ++ )
		{
			Layer->IsGraphHandleAlpha = MBLayer->IsGraphHandleAlpha ;
			Layer->GraphHandle        = MBLayer->GraphHandle ;
			Layer->BlendType          = MBLayer->BlendType ;
			Layer->Texture            = MBLayer->Texture ;
//			MV1AddTexture( MBase->Texture[ Layer->Texture ] ) ;
		}

		Material->DiffuseGradTexture = MBMaterial->DiffuseGradTexture ;
		Material->SpecularGradTexture = MBMaterial->SpecularGradTexture ;
		Material->SphereMapTexture = MBMaterial->SphereMapTexture ;
		Material->DiffuseGradBlendType = MBMaterial->DiffuseGradBlendType ;
		Material->SpecularGradBlendType = MBMaterial->SpecularGradBlendType ;
		Material->SphereMapBlendType = MBMaterial->SphereMapBlendType ;
		Material->OutLineWidth = MBMaterial->OutLineWidth ;
		Material->OutLineDotWidth = MBMaterial->OutLineDotWidth ;
		Material->OutLineColor = MBMaterial->OutLineColor ;

		Material->UseAlphaTest = MBMaterial->UseAlphaTest ;
		Material->AlphaFunc = MBMaterial->AlphaFunc ;
		Material->AlphaRef = MBMaterial->AlphaRef ;
		Material->DrawBlendMode = MBMaterial->DrawBlendMode ;
		Material->DrawBlendParam = MBMaterial->DrawBlendParam ;
	}

	// 物理演算の剛体情報をセットアップ
	MBPhysicsRigidBody = MBase->PhysicsRigidBody ;
	PhysicsRigidBody = Model->PhysicsRigidBody ;
	for( i = 0 ; i < MBase->PhysicsRigidBodyNum ; i ++, MBPhysicsRigidBody ++, PhysicsRigidBody ++ )
	{
		PhysicsRigidBody->BaseData = MBPhysicsRigidBody ;
		PhysicsRigidBody->TargetFrame = &Model->Frame[ MBPhysicsRigidBody->TargetFrame->Index ] ;
	}

	// 物理演算のジョイント情報をセットアップ
	MBPhysicsJoint = MBase->PhysicsJoint ;
	PhysicsJoint = Model->PhysicsJoint ;
	for( i = 0 ; i < MBase->PhysicsJointNum ; i ++, MBPhysicsJoint ++, PhysicsJoint ++ )
	{
		PhysicsJoint->BaseData = MBPhysicsJoint ;
	}

	// トライアングルリストの情報をセット
	MBTList = MBase->TriangleList ;
	TList = Model->TriangleList ;
	for( i = 0 ; i < MBase->TriangleListNum ; i ++, MBTList ++, TList = ( MV1_TRIANGLE_LIST * )( ( BYTE * )TList + sizeof( MV1_TRIANGLE_LIST ) + sizeof( MV1_TRIANGLE_LIST_PF ) ) )
	{
		TList->PF = ( MV1_TRIANGLE_LIST_PF * )( TList + 1 ) ;
		TList->BaseData = MBTList ;
		TList->Container = Model->Mesh + ( MBTList->Container - MBase->Mesh ) ;
		TList->NormalPosition = NULL ;
	}

	// シェイプの情報をセット
	MBShape = MBase->Shape ;
	Shape = Model->Shape ;
	for( i = 0 ; i < MBase->ShapeNum ; i ++, MBShape ++, Shape ++ )
	{
		Shape->Container = Model->Frame + ( MBShape->Container - MBase->Frame ) ;
		Shape->BaseData = MBShape ;

		Shape->Rate = 0.0f ;
	}

	// メッシュの情報をセット
	MBMesh = MBase->Mesh ;
	Mesh = Model->Mesh ;
	j = 0 ;
	for( i = 0 ; i < MBase->MeshNum ; i ++, MBMesh ++, Mesh ++ )
	{
		Mesh->Container = Model->Frame + ( MBMesh->Container - MBase->Frame ) ;
		Mesh->DrawMaterialChange.BaseData = &MBMesh->ChangeInfo ;
		Mesh->Material = Model->Material + ( MBMesh->Material - MBase->Material ) ;
		Mesh->DrawMaterialChange.Target   = Model->ChangeDrawMaterialFlag + MBMesh->ChangeInfo.Target ;
		Mesh->BaseData = MBMesh ;
		Mesh->DrawMaterial = InitDrawMat ;
		Mesh->DrawMaterial.Visible = MBMesh->Visible ;
		Mesh->DrawBlendMode = -1 ;
		Mesh->DrawBlendParam = -1 ;
		Mesh->SemiTransStateSetupFlag = false ;

		if( MBMesh->TriangleListNum )
		{
			Mesh->TriangleList = ( MV1_TRIANGLE_LIST * )( ( BYTE * )Model->TriangleList + ( MBMesh->TriangleList - MBase->TriangleList ) * ( sizeof( MV1_TRIANGLE_LIST ) + sizeof( MV1_TRIANGLE_LIST_PF ) ) );

			if( MBMesh->Shape )
			{
				Mesh->ShapeVertex = Model->ShapeVertex + j ;
				j += MBMesh->VertexNum ;

				TList = Mesh->TriangleList ;
				MBTList = MBMesh->TriangleList ;
				for( k = 0 ; k < MBMesh->TriangleListNum ; k ++, MBTList ++, TList = ( MV1_TRIANGLE_LIST * )( ( BYTE * )TList + sizeof( MV1_TRIANGLE_LIST ) + sizeof( MV1_TRIANGLE_LIST_PF ) ) )
				{
					switch( MBTList->VertexType )
					{
					case MV1_VERTEX_TYPE_NORMAL :
						TList->NormalPosition = Model->ShapeNormalPosition + ShapeNormalPositionNum ;
						ShapeNormalPositionNum += MBTList->VertexNum ;
						break ;

					case MV1_VERTEX_TYPE_SKIN_4BONE :
						TList->SkinPosition4B = Model->ShapeSkinPosition4B + ShapeSkinPosition4BNum ;
						ShapeSkinPosition4BNum += MBTList->VertexNum ;
						break ;

					case MV1_VERTEX_TYPE_SKIN_8BONE :
						TList->SkinPosition8B = Model->ShapeSkinPosition8B + ShapeSkinPosition8BNum ;
						ShapeSkinPosition8BNum += MBTList->VertexNum ;
						break ;

					case MV1_VERTEX_TYPE_SKIN_FREEBONE :
						TList->SkinPositionFREEB = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )Model->ShapeSkinPositionFREEB + ShapeSkinPositionFREEBSize ) ;
						ShapeSkinPositionFREEBSize += MBTList->VertexNum * MBTList->PosUnitSize ;
						break ;
					}

					MV1SetupShapeTriangleListPositionAndNormal( TList ) ;
				}
			}
		}
	}

	// フレームの情報をセット
	Model->TopFrameNum = 0 ;
	Frame = Model->Frame ;
	MBFrame = MBase->Frame ;
	for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++, MBFrame ++ )
	{
		Frame->Container = Model ;
		Frame->ModelHandle = MV1ModelHandle ;
		Frame->BaseData = MBFrame ;
		Frame->Parent = MBFrame->Parent     == NULL ? NULL : Model->Frame + MBFrame->Parent->Index ;
		Frame->Child  = MBFrame->FirstChild == NULL ? NULL : Model->Frame + MBFrame->FirstChild->Index ;
		Frame->Prev   = MBFrame->Prev       == NULL ? NULL : Model->Frame + MBFrame->Prev->Index ;
		Frame->Next   = MBFrame->Next       == NULL ? NULL : Model->Frame + MBFrame->Next->Index ;
		Frame->ChildNum = 0 ;

		CreateIdentityMatrix( &Frame->TextureAddressTransformMatrix ) ;
		Frame->LocalWorldMatrixChange.BaseData = &MBFrame->ChangeMatrixInfo ;
		Frame->LocalWorldMatrixChange.Target   = Model->ChangeMatrixFlag + MBFrame->ChangeMatrixInfo.Target ;

		Frame->DrawMaterialChange.BaseData = &MBFrame->ChangeDrawMaterialInfo ;
		Frame->DrawMaterialChange.Target   = Model->ChangeDrawMaterialFlag + MBFrame->ChangeDrawMaterialInfo.Target ;

		Frame->DrawMaterial = InitDrawMat ;
		Frame->DrawMaterial.Visible = ( BYTE )( ( MBFrame->Flag & MV1_FRAMEFLAG_VISIBLE ) != 0 ? 1 : 0 ) ;

		if( MBFrame->MeshNum )
		{
			Frame->Mesh = ( MV1_MESH * )( Model->Mesh + ( MBFrame->Mesh - MBase->Mesh ) ) ;
		}
		else
		{
			Frame->Mesh = NULL ;
		}

		if( MBFrame->ShapeNum )
		{
			Frame->Shape = ( MV1_SHAPE * )( Model->Shape + ( MBFrame->Shape - MBase->Shape ) ) ;
		}
		else
		{
			Frame->Shape = NULL ;
		}

		if( Frame->Parent )
		{
			Frame->ChildIndex = Frame->Parent->ChildNum ;
			Frame->Parent->ChildNum ++ ;
		}
		else
		{
			Frame->ChildIndex = Model->TopFrameNum ;
			Model->TopFrameNum ++ ;
		}

		if( MBFrame->UseSkinBoneNum )
		{
			Frame->UseSkinBoneMatrix = Model->SkinBoneUseFrameMatrix + ( MBFrame->UseSkinBone - MBase->FrameUseSkinBone ) ;

			for( j = 0 ; j < MBFrame->UseSkinBoneNum ; j ++ )
			{
				Frame->UseSkinBoneMatrix[ j ] = &Model->SkinBoneMatrix[ MBFrame->UseSkinBone[ j ] - MBase->SkinBone ] ;
			}
		}

		Frame->ShapeChangeFlag = true ;
	}

	// 各フレームの子リストを作成する
	Model->TopFrameList = ( MV1_FRAME ** )( ( MV1_FRAME * )Model->DataBuffer + MBase->FrameNum ) ;
	Num     = ( DWORD )Model->TopFrameNum ;
	Frame   = Model->Frame ;
	MBFrame = MBase->Frame ;
	for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++, MBFrame ++ )
	{
		if( Frame->Parent )
		{
			Frame->Parent->ChildList[ Frame->ChildIndex ] = Frame ;
		}
		else
		{
			Model->TopFrameList[ Frame->ChildIndex ] = Frame ;
		}
		if( Frame->ChildNum )
		{
			Frame->ChildList = ( MV1_FRAME ** )( ( MV1_FRAME * )Model->DataBuffer + MBase->FrameNum ) + Num ;
			Num += Frame->ChildNum ;
		}
	}

	// モデル基本データの参照カウントをインクリメントする
	MBase->RefCount ++ ;

	// 初期行列のセットアップ
	MV1SetupMatrix( Model ) ;

	// デフォルトポーズの最小座標値と最大座標値を算出する
	MV1GetMaxMinPosition( MV1ModelHandle, NULL, NULL, ASyncThread ) ;

	// 頂点バッファのセットアップを行う
	MV1_SetupVertexBuffer_PF( MV1ModelHandle, ASyncThread ) ;

#ifndef DX_NON_BULLET_PHYSICS
	// 物理演算のセットアップ
	if( MBase->PhysicsRigidBodyNum != 0 )
		SetupPhysicsObject_ModelPhysicsInfo( Model ) ;
#endif

	// 正常終了
	return 0 ;
}

// モデルの最大頂点座標と最小頂点座標を取得する
extern int MV1GetMaxMinPosition( int MHandle, VECTOR *MaxPosition, VECTOR *MinPosition, int ASyncThread )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *MBase ;
	MV1_FRAME *Frame ;
	MV1_FRAME_BASE *MBFrame ;
	MV1_MESH_BASE *Mesh ;
	MV1_TRIANGLE_LIST_BASE *TList ;
	MATRIX_4X4CT BlendMat, *pBlendMat[ MV1_TRIANGLE_LIST_USE_BONE_MAX_NUM ] ;
	int i, k, l, m, n ;
	float Weight ;
	VECTOR DestVector ;
	MV1_TLIST_NORMAL_POS *Norm ;
	MV1_TLIST_SKIN_POS_4B *Skin4B ;
	MV1_TLIST_SKIN_POS_8B *Skin8B ;
	MV1_TLIST_SKIN_POS_FREEB *SkinFB ;
	MV1_SKINBONE_BLEND *VBlend ;

	// アドレス取得
	if( ASyncThread )
	{
		if( MV1MDLCHK_ASYNC( MHandle, Model ) )
			return -1 ;
	}
	else
	{
		if( MV1MDLCHK( MHandle, Model ) )
			return -1 ;
	}
	MBase = Model->BaseData ;

	// 最大座標と最小座標が算出されていない場合はここで算出する
	if( MBase->ValidMaxMinPosition == 0 )
	{
		// 行列のセットアップ
		MV1SetupMatrix( Model ) ;

		// すべてのメッシュの頂点から最大座標と最小座標を算出する
		MBase->MaxPosition.x =
		MBase->MaxPosition.y =
		MBase->MaxPosition.z = -100000000.0f ;
		MBase->MinPosition.x =
		MBase->MinPosition.y =
		MBase->MinPosition.z =  100000000.0f ;

		// フレームの数だけ繰り返し
		Frame = Model->Frame ;
		for( i = 0 ; i < Model->BaseData->FrameNum ; i ++ , Frame ++ )
		{
			MBFrame = Frame->BaseData ;

			// メッシュが無かったら飛ばす
			if( MBFrame->MeshNum == 0 )
			{
				MBFrame->MaxPosition.x =
				MBFrame->MaxPosition.y =
				MBFrame->MaxPosition.z = 0.0f ;
				MBFrame->MinPosition.x =
				MBFrame->MinPosition.y =
				MBFrame->MinPosition.z = 0.0f ;
				continue ;
			}

			// 最大・最小値の初期化
			MBFrame->MaxPosition.x =
			MBFrame->MaxPosition.y =
			MBFrame->MaxPosition.z = -100000000.0f ;
			MBFrame->MinPosition.x =
			MBFrame->MinPosition.y =
			MBFrame->MinPosition.z =  100000000.0f ;

			// メッシュの数だけ繰り返し
			Mesh = MBFrame->Mesh ;
			for( k = 0 ; k < MBFrame->MeshNum ; k ++ , Mesh ++ )
			{
				// 最大・最小値の初期化
				Mesh->MaxPosition.x =
				Mesh->MaxPosition.y =
				Mesh->MaxPosition.z = -100000000.0f ;
				Mesh->MinPosition.x =
				Mesh->MinPosition.y =
				Mesh->MinPosition.z =  100000000.0f ;

				// トライアングルリストの数だけ繰り返し
				TList = Mesh->TriangleList ;
				for( l = 0 ; l < Mesh->TriangleListNum ; l ++ , TList ++ )
				{
					// 頂点のタイプによって処理を分岐
					switch( TList->VertexType )
					{
						// 剛体トライアングルリスト
					case MV1_VERTEX_TYPE_NORMAL :
						Norm = ( MV1_TLIST_NORMAL_POS * )ADDR16( TList->NormalPosition ) ;
						for( m = 0 ; m < TList->VertexNum ; m ++, Norm ++ )
						{
							VectorTransform4X4CT( &DestVector, ( VECTOR * )&Norm->Position, &Frame->LocalWorldMatrix ) ;
							if( MBase->MaxPosition.x < DestVector.x ) MBase->MaxPosition.x = DestVector.x ;
							if( MBase->MinPosition.x > DestVector.x ) MBase->MinPosition.x = DestVector.x ;
							if( MBase->MaxPosition.y < DestVector.y ) MBase->MaxPosition.y = DestVector.y ;
							if( MBase->MinPosition.y > DestVector.y ) MBase->MinPosition.y = DestVector.y ;
							if( MBase->MaxPosition.z < DestVector.z ) MBase->MaxPosition.z = DestVector.z ;
							if( MBase->MinPosition.z > DestVector.z ) MBase->MinPosition.z = DestVector.z ;

							if( Mesh->MaxPosition.x < Norm->Position.x ) Mesh->MaxPosition.x = Norm->Position.x ;
							if( Mesh->MinPosition.x > Norm->Position.x ) Mesh->MinPosition.x = Norm->Position.x ;
							if( Mesh->MaxPosition.y < Norm->Position.y ) Mesh->MaxPosition.y = Norm->Position.y ;
							if( Mesh->MinPosition.y > Norm->Position.y ) Mesh->MinPosition.y = Norm->Position.y ;
							if( Mesh->MaxPosition.z < Norm->Position.z ) Mesh->MaxPosition.z = Norm->Position.z ;
							if( Mesh->MinPosition.z > Norm->Position.z ) Mesh->MinPosition.z = Norm->Position.z ;
						}
						break ;

						// ４個以内ボーントライアングルリスト
					case MV1_VERTEX_TYPE_SKIN_4BONE :

						// 行列のポインタを準備
						for( m = 0 ; m < TList->UseBoneNum ; m ++ )
						{
							pBlendMat[ m ] = Frame->UseSkinBoneMatrix[ TList->UseBone[ m ] ] ;
						}

						// データのセット
						Skin4B = ( MV1_TLIST_SKIN_POS_4B * )ADDR16( TList->SkinPosition4B ) ;
						for( m = 0 ; m < TList->VertexNum ; m ++, Skin4B ++ )
						{
							// ブレンド行列の作成
							_MEMSET( &BlendMat, 0, sizeof( BlendMat ) ) ;
							for( n = 0 ; n < 4 ; n ++ )
							{
								Weight = Skin4B->MatrixWeight[ n ] ;

								if( Weight == 0.0f ) continue ;
								UnSafeMatrix4X4CT_C_EqPlus_C_Mul_S( &BlendMat, pBlendMat[ Skin4B->MatrixIndex[ n ] ], Weight ) ;
							}

							VectorTransform4X4CT( &DestVector, ( VECTOR * )&Skin4B->Position, &BlendMat ) ;
							if( MBase->MaxPosition.x < DestVector.x ) MBase->MaxPosition.x = DestVector.x ;
							if( MBase->MinPosition.x > DestVector.x ) MBase->MinPosition.x = DestVector.x ;
							if( MBase->MaxPosition.y < DestVector.y ) MBase->MaxPosition.y = DestVector.y ;
							if( MBase->MinPosition.y > DestVector.y ) MBase->MinPosition.y = DestVector.y ;
							if( MBase->MaxPosition.z < DestVector.z ) MBase->MaxPosition.z = DestVector.z ;
							if( MBase->MinPosition.z > DestVector.z ) MBase->MinPosition.z = DestVector.z ;

							if( Mesh->MaxPosition.x < Skin4B->Position.x ) Mesh->MaxPosition.x = Skin4B->Position.x ;
							if( Mesh->MinPosition.x > Skin4B->Position.x ) Mesh->MinPosition.x = Skin4B->Position.x ;
							if( Mesh->MaxPosition.y < Skin4B->Position.y ) Mesh->MaxPosition.y = Skin4B->Position.y ;
							if( Mesh->MinPosition.y > Skin4B->Position.y ) Mesh->MinPosition.y = Skin4B->Position.y ;
							if( Mesh->MaxPosition.z < Skin4B->Position.z ) Mesh->MaxPosition.z = Skin4B->Position.z ;
							if( Mesh->MinPosition.z > Skin4B->Position.z ) Mesh->MinPosition.z = Skin4B->Position.z ;
						}
						break ;

						// ８個以内ボーントライアングルリスト
					case MV1_VERTEX_TYPE_SKIN_8BONE :

						// 行列のポインタを準備
						for( m = 0 ; m < TList->UseBoneNum ; m ++ )
							pBlendMat[ m ] = Frame->UseSkinBoneMatrix[ TList->UseBone[ m ] ] ;

						// データのセット
						Skin8B = ( MV1_TLIST_SKIN_POS_8B * )ADDR16( TList->SkinPosition8B ) ;
						for( m = 0 ; m < TList->VertexNum ; m ++, Skin8B ++ )
						{
							// ブレンド行列の作成
							_MEMSET( &BlendMat, 0, sizeof( BlendMat ) ) ;
							for( n = 0 ; n < 8 ; n ++ )
							{
								Weight = Skin8B->MatrixWeight[ n ] ;

								if( Weight == 0.0f ) continue ;
								UnSafeMatrix4X4CT_C_EqPlus_C_Mul_S( &BlendMat, pBlendMat[ n < 4 ? Skin8B->MatrixIndex1[ n ] : Skin8B->MatrixIndex2[ n - 4 ] ], Weight ) ;
							}

							VectorTransform4X4CT( &DestVector, &Skin8B->Position, &BlendMat ) ;
							if( MBase->MaxPosition.x < DestVector.x ) MBase->MaxPosition.x = DestVector.x ;
							if( MBase->MinPosition.x > DestVector.x ) MBase->MinPosition.x = DestVector.x ;
							if( MBase->MaxPosition.y < DestVector.y ) MBase->MaxPosition.y = DestVector.y ;
							if( MBase->MinPosition.y > DestVector.y ) MBase->MinPosition.y = DestVector.y ;
							if( MBase->MaxPosition.z < DestVector.z ) MBase->MaxPosition.z = DestVector.z ;
							if( MBase->MinPosition.z > DestVector.z ) MBase->MinPosition.z = DestVector.z ;

							if( Mesh->MaxPosition.x < Skin8B->Position.x ) Mesh->MaxPosition.x = Skin8B->Position.x ;
							if( Mesh->MinPosition.x > Skin8B->Position.x ) Mesh->MinPosition.x = Skin8B->Position.x ;
							if( Mesh->MaxPosition.y < Skin8B->Position.y ) Mesh->MaxPosition.y = Skin8B->Position.y ;
							if( Mesh->MinPosition.y > Skin8B->Position.y ) Mesh->MinPosition.y = Skin8B->Position.y ;
							if( Mesh->MaxPosition.z < Skin8B->Position.z ) Mesh->MaxPosition.z = Skin8B->Position.z ;
							if( Mesh->MinPosition.z > Skin8B->Position.z ) Mesh->MinPosition.z = Skin8B->Position.z ;
						}
						break ;

						// ボーン数無制限トライアングルリスト
					case MV1_VERTEX_TYPE_SKIN_FREEBONE :
						// データのセット
						SkinFB = ( MV1_TLIST_SKIN_POS_FREEB * )ADDR16( TList->SkinPositionFREEB ) ;
						for( m = 0 ; m < TList->VertexNum ; m ++, SkinFB = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )SkinFB + TList->PosUnitSize ) )
						{
							// 使われているボーンがひとつの場合はブレンド行列の作成はスキップする
							if( SkinFB->MatrixWeight[ 0 ].W == 1.0f )
							{
								BlendMat = *Frame->UseSkinBoneMatrix[ SkinFB->MatrixWeight[ 0 ].Index ] ;
							}
							else
							{
								// ブレンド行列の作成
								VBlend = SkinFB->MatrixWeight ;
								Weight = VBlend->W ;
								_MEMSET( &BlendMat, 0, sizeof( BlendMat ) ) ;
								for( n = 0 ; n < TList->MaxBoneNum && VBlend->Index != -1 ; n ++, VBlend ++ )
								{
									Weight = VBlend->W ;

									if( Weight == 0.0f ) continue ;
									UnSafeMatrix4X4CT_C_EqPlus_C_Mul_S( &BlendMat, Frame->UseSkinBoneMatrix[ VBlend->Index ], Weight ) ;
								}
							}

							VectorTransform4X4CT( &DestVector, ( VECTOR * )&SkinFB->Position, &BlendMat ) ;
							if( MBase->MaxPosition.x < DestVector.x ) MBase->MaxPosition.x = DestVector.x ;
							if( MBase->MinPosition.x > DestVector.x ) MBase->MinPosition.x = DestVector.x ;
							if( MBase->MaxPosition.y < DestVector.y ) MBase->MaxPosition.y = DestVector.y ;
							if( MBase->MinPosition.y > DestVector.y ) MBase->MinPosition.y = DestVector.y ;
							if( MBase->MaxPosition.z < DestVector.z ) MBase->MaxPosition.z = DestVector.z ;
							if( MBase->MinPosition.z > DestVector.z ) MBase->MinPosition.z = DestVector.z ;

							if( Mesh->MaxPosition.x < SkinFB->Position.x ) Mesh->MaxPosition.x = SkinFB->Position.x ;
							if( Mesh->MinPosition.x > SkinFB->Position.x ) Mesh->MinPosition.x = SkinFB->Position.x ;
							if( Mesh->MaxPosition.y < SkinFB->Position.y ) Mesh->MaxPosition.y = SkinFB->Position.y ;
							if( Mesh->MinPosition.y > SkinFB->Position.y ) Mesh->MinPosition.y = SkinFB->Position.y ;
							if( Mesh->MaxPosition.z < SkinFB->Position.z ) Mesh->MaxPosition.z = SkinFB->Position.z ;
							if( Mesh->MinPosition.z > SkinFB->Position.z ) Mesh->MinPosition.z = SkinFB->Position.z ;
						}
						break ;
					}
				}

				if( MBFrame->MaxPosition.x < Mesh->MaxPosition.x ) MBFrame->MaxPosition.x = Mesh->MaxPosition.x ;
				if( MBFrame->MinPosition.x > Mesh->MinPosition.x ) MBFrame->MinPosition.x = Mesh->MinPosition.x ;
				if( MBFrame->MaxPosition.y < Mesh->MaxPosition.y ) MBFrame->MaxPosition.y = Mesh->MaxPosition.y ;
				if( MBFrame->MinPosition.y > Mesh->MinPosition.y ) MBFrame->MinPosition.y = Mesh->MinPosition.y ;
				if( MBFrame->MaxPosition.z < Mesh->MaxPosition.z ) MBFrame->MaxPosition.z = Mesh->MaxPosition.z ;
				if( MBFrame->MinPosition.z > Mesh->MinPosition.z ) MBFrame->MinPosition.z = Mesh->MinPosition.z ;
			}

			MBFrame->ValidMaxMinPosition = 1 ;
		}

		// 算出が終わったら有効フラグを立てる
		MBase->ValidMaxMinPosition = 1 ;
	}

	// 情報のセット
	if( MaxPosition ) *MaxPosition = MBase->MaxPosition ;
	if( MinPosition ) *MinPosition = MBase->MinPosition ;

	// 終了
	return 0 ;
}

// モデルデータを削除する
extern int MV1SubModel( int MV1ModelHandle )
{
	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	return SubHandle( MV1ModelHandle ) ;
}

// MV1LoadModelFromMem の実処理関数
static int MV1LoadModelFromMem_Static( 
	MV1LOADMODEL_GPARAM *GParam,
	int MHandle,
	const void *FileImage,
	int FileSize,
	int (* FileReadFunc )( const TCHAR *FilePath, void **FileImageAddr, int *FileSize, void *FileReadFuncData ),
	int (* FileReleaseFunc )( void *MemoryAddr, void *FileReadFuncData ),
	void *FileReadFuncData,
	int ASyncThread
)
{
	void *DataBuffer = NULL ;
	int NewBaseHandle = -1 ;
	MV1_MODEL_LOAD_PARAM LoadParam ;
	MV1_FILE_READ_FUNC FileReadFuncParam ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// ３Ｄモデル用シェーダーの初期化チェック
	if( Graphics_Hardware_Shader_ModelCode_Init_PF() < 0 )
	{
		return -1 ;
	}

	// ファイルを格納するメモリを確保
	DataBuffer = DXALLOC( ( size_t )( FileSize + 1 ) ) ;
	if( DataBuffer == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe2\x30\xc7\x30\xeb\x30\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xa4\x30\xe1\x30\xfc\x30\xb8\x30\x6e\x30\xc7\x30\xfc\x30\xbf\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"モデルファイルイメージのデータを格納するメモリ領域の確保に失敗しました" @*/ )) ;
		goto ERRORLABEL ;
	}

	// データをコピー
	_MEMCPY( DataBuffer, FileImage,  ( size_t )FileSize ) ;

	// テキスト形式の場合用に終端文字をセット
	( ( BYTE * )DataBuffer )[  FileSize ] = '\0' ;

	// 関数の引数を準備
	FileReadFuncParam.Read		= FileReadFunc ;
	FileReadFuncParam.Release	= FileReleaseFunc ;
	FileReadFuncParam.Data		= FileReadFuncData ;

	LoadParam.GParam			= *GParam ;
	LoadParam.DataBuffer		= DataBuffer ;
	LoadParam.DataSize			= FileSize ;
	LoadParam.FilePath			= L"" ;
	LoadParam.Name				= L"" ;
	LoadParam.CurrentDir		= NULL ;
	LoadParam.FileReadFunc		= &FileReadFuncParam ;

	// オリジナルモデルロード関数を試す
	for( i = 0 ; i < MV1Man.AddLoadFuncNum ; i ++ )
	{
		NewBaseHandle = MV1Man.AddLoadFunc[ i ]( &LoadParam ) ;
		if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;
	}

	// ＭＶ１ファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToMV1( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＭＱＯファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToMQO( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// Ｘファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToX( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＰＭＸファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToPMX( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＰＭＤファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToPMD( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＶＭＤファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToVMD( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

#ifdef DX_LOAD_FBX_MODEL

	// ＦＢＸファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToFBX( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

#endif

	// ここにきたら読み込めなかったということ
	goto ERRORLABEL ;

LOADCOMPLABEL :
	// 頂点バッファのセットアップを行う
	MV1_SetupVertexBufferBase_PF( NewBaseHandle, 1, ASyncThread ) ;

	// ファイルから読み込んだデータを解放する
	if( DataBuffer )
	{
		DXFREE( DataBuffer ) ;
		DataBuffer = NULL ;
	}

	// 構築
	if( MV1MakeModel( MHandle, NewBaseHandle, ASyncThread ) < 0 )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe2\x30\xc7\x30\xeb\x30\xd5\x30\xa1\x30\xa4\x30\xeb\x30\xa4\x30\xe1\x30\xfc\x30\xb8\x30\x4b\x30\x89\x30\x6e\x30\xe2\x30\xc7\x30\xeb\x30\xfa\x57\x2c\x67\xc7\x30\xfc\x30\xbf\x30\x4b\x30\x89\x30\xe2\x30\xc7\x30\xeb\x30\xc7\x30\xfc\x30\xbf\x30\x78\x30\x6e\x30\x09\x59\xdb\x63\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"モデルファイルイメージからのモデル基本データからモデルデータへの変換に失敗しました" @*/ )) ;
		goto ERRORLABEL ;
	}

	// 正常終了
	return 0 ;


ERRORLABEL :
	// モデル基本データの解放
	if( NewBaseHandle != -1 )
	{
		MV1SubModelBase( NewBaseHandle ) ;
	}
/*
	// モデルデータの解放
	if( NewHandle != -1 )
	{
		MV1SubModel( NewHandle ) ;
	}
*/
	// ファイルから読み込んだデータを解放する
	if( DataBuffer )
	{
		DXFREE( DataBuffer ) ;
		DataBuffer = NULL ;
	}

	// 値を返す
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// MV1LoadModelFromMem の非同期読み込みスレッドから呼ばれる関数
static void MV1LoadModelFromMem_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	MV1LOADMODEL_GPARAM *GParam ;
	int MHandle ;
	const void *FileImage ;
	int FileSize ;
	int (* FileReadFunc )( const TCHAR *FilePath, void **FileImageAddr, int *FileSize, void *FileReadFuncData ) ;
	int (* FileReleaseFunc )( void *MemoryAddr, void *FileReadFuncData ) ;
	void *FileReadFuncData ;
	int Addr ;
	int Result ;

	Addr				= 0 ;
	GParam				= ( MV1LOADMODEL_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	MHandle				= GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileImage			= GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	FileSize			= GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileReadFunc		= ( int (*)( const TCHAR *FilePath, void **FileImageAddr, int *FileSize, void *FileReadFuncData ) )GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	FileReleaseFunc		= ( int (*)( void *MemoryAddr, void *FileReadFuncData ) )GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;
	FileReadFuncData	= GetASyncLoadParamVoidP( AParam->Data, &Addr ) ;

	Result = MV1LoadModelFromMem_Static( GParam, MHandle, FileImage, FileSize, FileReadFunc, FileReleaseFunc, FileReadFuncData, TRUE ) ;
	DecASyncLoadCount( MHandle ) ;
	if( Result < 0 )
	{
		MV1SubModel( MHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// MV1LoadModelFromMem のグローバル変数にアクセスしないバージョン
extern int MV1LoadModelFromMem_UseGParam( 
	MV1LOADMODEL_GPARAM *GParam,
	const void *FileImage,
	int FileSize,
	int (* FileReadFunc )( const TCHAR *FilePath, void **FileImageAddr, int *FileSize, void *FileReadFuncData ),
	int (* FileReleaseFunc )( void *MemoryAddr, void *FileReadFuncData ),
	void *FileReadFuncData,
	int ASyncLoadFlag
)
{
	int MHandle ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	MHandle = MV1AddModel( FALSE ) ;
	if( MHandle < 0 )
	{
		return -1 ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, MHandle ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, FileImage ) ;
		AddASyncLoadParamInt( NULL, &Addr, FileSize ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, ( void * )FileReadFunc ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, ( void * )FileReleaseFunc ) ;
		AddASyncLoadParamConstVoidP( NULL, &Addr, ( void * )FileReadFuncData ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = MV1LoadModelFromMem_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MHandle ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, FileImage ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, FileSize ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, ( void * )FileReadFunc ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, ( void * )FileReleaseFunc ) ;
		AddASyncLoadParamConstVoidP( AParam->Data, &Addr, ( void * )FileReadFuncData ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MV1LoadModelFromMem_Static( GParam, MHandle, FileImage, FileSize, FileReadFunc, FileReleaseFunc, FileReadFuncData, FALSE ) < 0 )
			goto ERR ;
	}

	// ハンドルを返す
	return MHandle ;

ERR :
	MV1SubModel( MHandle ) ;
	MHandle = -1 ;

	return -1 ;
}

// メモリ上のモデルファイルイメージと独自の読み込みルーチンを使用してモデルを読み込む
extern int NS_MV1LoadModelFromMem(
	const void *FileImage,
	int FileSize,
	int (* FileReadFunc )( const TCHAR *FilePath, void **FileImageAddr, int *FileSize, void *FileReadFuncData ),
	int (* FileReleaseFunc )( void *MemoryAddr, void *FileReadFuncData ),
	void *FileReadFuncData
)
{
	MV1LOADMODEL_GPARAM GParam ;

	InitMV1LoadModelGParam( &GParam ) ;

	return MV1LoadModelFromMem_UseGParam( &GParam, FileImage, FileSize, FileReadFunc, FileReleaseFunc, FileReadFuncData, GetASyncLoadFlag() ) ;
}

// MV1LoadModel の実処理関数
static int MV1LoadModel_Static(
	MV1LOADMODEL_GPARAM *GParam,
	int MHandle,
	const wchar_t *FileName,
	const wchar_t *Directory,
	const wchar_t *Name,
	int ASyncThread
)
{
	void *DataBuffer = NULL ;
	size_t DataSize ;
	int NewBaseHandle = -1 ;
	DWORD_PTR FileHandle = 0 ;
	MV1_MODEL_LOAD_PARAM LoadParam ;
	wchar_t FullPath[ 1024 ] ;

	ConvertFullPathW_( FileName, FullPath, NULL ) ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// ３Ｄモデル用シェーダーの初期化チェック
	if( Graphics_Hardware_Shader_ModelCode_Init_PF() < 0 )
	{
		return -1 ;
	}

	// ファイルを丸ごと読み込む
	FileHandle = FOPEN( FileName ) ;
	if( FileHandle == 0 )
	{
		DXST_ERRORLOGFMT_ADDW(( L"Model File Open Error : %s", FullPath )) ;
		goto ERRORLABEL ;
	}

	// ファイルのサイズを取得する
	FSEEK( FileHandle, 0, SEEK_END ) ;
	DataSize = ( size_t )FTELL( FileHandle ) ;
	FSEEK( FileHandle, 0, SEEK_SET ) ;

	// ファイルを格納するメモリを確保
	DataBuffer = DXALLOC( DataSize + 1 ) ;
	if( DataBuffer == NULL )
	{
		DXST_ERRORLOGFMT_ADDW(( L"Model File Memory Alloc Error : %s", FullPath )) ;
		goto ERRORLABEL ;
	}

	// データを読み込む
	FREAD( DataBuffer, DataSize, 1, FileHandle ) ;

	// テキスト形式の場合用に終端文字をセット
	( ( BYTE * )DataBuffer )[ DataSize ] = '\0' ;

	// ファイルを閉じる
	FCLOSE( FileHandle ) ;
	FileHandle = 0 ;

	// 関数の引数を準備
	LoadParam.GParam			= *GParam ;
	LoadParam.DataBuffer		= DataBuffer ;
	LoadParam.DataSize			= ( int )DataSize ;
	LoadParam.FilePath			= FileName ;
	LoadParam.Name				= Name ;
	LoadParam.CurrentDir		= Directory ;
	LoadParam.FileReadFunc		= NULL ;

	// ＭＶ１ファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToMV1( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＭＱＯファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToMQO( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// Ｘファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToX( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＰＭＸファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToPMX( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＰＭＤファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToPMD( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

	// ＶＭＤファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToVMD( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

#ifdef DX_LOAD_FBX_MODEL

	// ＦＢＸファイルとして読み込んでみる
	NewBaseHandle = MV1LoadModelToFBX( &LoadParam, ASyncThread ) ;
	if( NewBaseHandle >= 0 ) goto LOADCOMPLABEL ;

#endif

	// ここにきたら読み込めなかったということ
	goto ERRORLABEL ;

LOADCOMPLABEL :
	// 頂点バッファのセットアップを行う
	MV1_SetupVertexBufferBase_PF( NewBaseHandle, 1, ASyncThread ) ;

	// ファイルから読み込んだデータを解放する
	if( DataBuffer )
	{
		DXFREE( DataBuffer ) ;
		DataBuffer = NULL ;
	}

	// ファイルを閉じる
	if( FileHandle )
	{
		FCLOSE( FileHandle ) ;
		FileHandle = 0 ;
	}

	// 構築
	if( MV1MakeModel( MHandle, NewBaseHandle, ASyncThread ) < 0 )
	{
		DXST_ERRORLOGFMT_ADDW(( L"MV1MakeModel Error : %s", FullPath )) ;
		goto ERRORLABEL ;
	}

	// 正常終了
	return 0 ;


ERRORLABEL :
	// モデル基本データの解放
	if( NewBaseHandle != -1 )
	{
		MV1SubModelBase( NewBaseHandle ) ;
	}
/*
	// モデルデータの解放
	if( NewHandle != -1 )
	{
		MV1SubModel( NewHandle ) ;
	}
*/
	// ファイルから読み込んだデータを解放する
	if( DataBuffer )
	{
		DXFREE( DataBuffer ) ;
		DataBuffer = NULL ;
	}

	// ファイルを閉じる
	if( FileHandle )
	{
		FCLOSE( FileHandle ) ;
		FileHandle = 0 ;
	}

	// 値を返す
	return -1 ;
}

#ifndef DX_NON_ASYNCLOAD
// MV1LoadModel の非同期読み込みスレッドから呼ばれる関数
static void MV1LoadModel_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	MV1LOADMODEL_GPARAM *GParam ;
	int MHandle ;
	const wchar_t *FileName ;
	const wchar_t *Directory ;
	const wchar_t *Name ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	GParam = ( MV1LOADMODEL_GPARAM * )GetASyncLoadParamStruct( AParam->Data, &Addr ) ;
	MHandle = GetASyncLoadParamInt( AParam->Data, &Addr ) ;
	FileName = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	Directory = GetASyncLoadParamString( AParam->Data, &Addr ) ;
	Name = GetASyncLoadParamString( AParam->Data, &Addr ) ;

	Result = MV1LoadModel_Static( GParam, MHandle, FileName, Directory, Name, TRUE ) ;
	DecASyncLoadCount( MHandle ) ;
	if( Result < 0 )
	{
		MV1SubModel( MHandle ) ;
	}
}
#endif // DX_NON_ASYNCLOAD

// MV1LoadModel のグローバル変数にアクセスしないバージョン
extern int MV1LoadModel_UseGParam( MV1LOADMODEL_GPARAM *GParam, const wchar_t *FileName, int ASyncLoadFlag )
{
	wchar_t Directory[ 1024 ], Name[ FILEPATH_MAX ] ;
	int MHandle ;

	// モデルファイルのあるディレクトリパスとファイル名を取得する
	_MV1CreateFileNameAndCurrentDirectory( FileName, Name, Directory ) ;

	if( WinData.ActiveFlag == FALSE )
		DxActiveWait() ;

	MHandle = MV1AddModel( FALSE ) ;
	if( MHandle < 0 )
	{
		return -1 ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamStruct( NULL, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( NULL, &Addr, MHandle ) ;
		AddASyncLoadParamString( NULL, &Addr, FileName ) ;
		AddASyncLoadParamString( NULL, &Addr, Directory ) ;
		AddASyncLoadParamString( NULL, &Addr, Name ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = MV1LoadModel_ASync ;
		Addr = 0 ;
		AddASyncLoadParamStruct( AParam->Data, &Addr, GParam, sizeof( *GParam ) ) ;
		AddASyncLoadParamInt( AParam->Data, &Addr, MHandle ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, FileName ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, Directory ) ;
		AddASyncLoadParamString( AParam->Data, &Addr, Name ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MV1LoadModel_Static( GParam, MHandle, FileName, Directory, Name, FALSE ) < 0 )
			goto ERR ;
	}

	// ハンドルを返す
	return MHandle ;

ERR :
	MV1SubModel( MHandle ) ;
	MHandle = -1 ;

	return -1 ;
}

// MV1LOADMODEL_GPARAM のデータをセットする
extern void InitMV1LoadModelGParam( MV1LOADMODEL_GPARAM *GParam )
{
	Graphics_Image_InitLoadGraphGParam( &GParam->LoadGraphGParam ) ;

	GParam->LoadModelToReMakeNormal               = MV1Man.LoadModelToReMakeNormal ;
	GParam->LoadModelToReMakeNormalSmoothingAngle = MV1Man.LoadModelToReMakeNormalSmoothingAngle ;
	GParam->LoadModelToIgnoreScaling              = MV1Man.LoadModelToIgnoreScaling ;
	GParam->LoadModelToPositionOptimize           = MV1Man.LoadModelToPositionOptimize ;
	GParam->LoadModelToUsePhysicsMode             = MV1Man.LoadModelToUsePhysicsMode ;
	GParam->LoadModelToWorldGravityInitialize     = MV1Man.LoadModelToWorldGravityInitialize ;
	GParam->LoadModelToWorldGravity               = MV1Man.LoadModelToWorldGravity ;
	GParam->LoadModelToPhysicsCalcPrecision       = MV1Man.LoadModelToPhysicsCalcPrecision ;
	GParam->LoadModelToUsePackDraw                = MV1Man.LoadModelToUsePackDraw ;
	_MEMCPY( GParam->LoadCalcPhysicsWorldGravity, MV1Man.LoadCalcPhysicsWorldGravity, sizeof( MV1Man.LoadCalcPhysicsWorldGravity ) ) ;

	GParam->AnimFilePathValid = MV1Man.AnimFilePathValid ;
	if( GParam->AnimFilePathValid )
	{
		_WCSCPY( GParam->AnimFileName,    MV1Man.AnimFileName ) ;
		_WCSCPY( GParam->AnimFileDirPath, MV1Man.AnimFileDirPath ) ;
	}
}

// モデルの読み込み( -1:エラー  0以上:モデルハンドル )
extern int NS_MV1LoadModel( const TCHAR *FileName )
{
#ifdef UNICODE
	return MV1LoadModel_WCHAR_T(
		FileName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = MV1LoadModel_WCHAR_T(
		UseFileNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// モデルの読み込み( -1:エラー  0以上:モデルハンドル )
extern int MV1LoadModel_WCHAR_T( const wchar_t *FileName )
{
	MV1LOADMODEL_GPARAM GParam ;

	InitMV1LoadModelGParam( &GParam ) ;

	return MV1LoadModel_UseGParam( &GParam, FileName, GetASyncLoadFlag() ) ;
}


// モデルを削除する
extern int NS_MV1DeleteModel( int MHandle )
{
	return MV1SubModel( MHandle ) ;
}

// すべてのモデルを削除する
extern int NS_MV1InitModel( void )
{
	return AllHandleSub( DX_HANDLETYPE_MODEL ) ;
}

// MV1LoadModelToMV1 での文字列取得処理
__inline void MV1LoadModelToMV1_GetString(
	MV1MODEL_FILEHEADER_F1 *	FHeader,
	MV1_MODEL_BASE *			MBase,
	BYTE *						FileStringBuffer,
	DWORD						FileStringAddr,
#ifndef UNICODE
	char **						StringA,
#endif
	wchar_t **					StringW
)
{
	if( FHeader->IsStringUTF8 )
	{
		*StringW = ( wchar_t * )( ( BYTE * )MBase->StringBufferW + MBase->StringSizeW ) ;
		MBase->StringSizeW += ConvString( ( const char * )( FileStringBuffer + FileStringAddr ), DX_CODEPAGE_UTF8, ( char * )*StringW, WCHAR_T_CODEPAGE ) ;

#ifndef UNICODE
		*StringA = ( char    * )( ( BYTE * )MBase->StringBufferA + MBase->StringSizeA ) ;
		MBase->StringSizeA += ConvString( ( const char * )*StringW, WCHAR_T_CODEPAGE, *StringA, CHAR_CODEPAGE ) ;
#endif
	}
	else
	{
		*StringW   = ( wchar_t * )( ( BYTE * )MBase->StringBufferW + MBase->StringSizeW ) ;
		MBase->StringSizeW += ConvString( ( const char * )( FileStringBuffer + FileStringAddr ), DX_CODEPAGE_SHIFTJIS, ( char * )*StringW, WCHAR_T_CODEPAGE ) ;

#ifndef UNICODE
		*StringA   = MBase->StringBufferA + FileStringAddr ;
#endif
	}
}

// ＭＶ１ファイルを読み込む( -1:エラー  0以上:モデルハンドル )
extern int MV1LoadModelToMV1( const MV1_MODEL_LOAD_PARAM *LoadParam, int ASyncThread )
{
	MV1MODEL_FILEHEADER_F1 *FHeader ;
	MV1_MODEL_BASE MTBase, *MBase = NULL ;
	MV1_FRAME_BASE			*Frame ;
	MV1_FRAME_F1			*F1Frame ;
	MV1_MATERIAL_BASE		*Material ;
	MV1_MATERIAL_F1			*F1Material ;
	MV1_MATERIAL_LAYER		*MaterialLayer ;
	MV1_MATERIAL_LAYER_F1	*F1MaterialLayer ;
	MV1_MATERIAL_TOON_F1	*F1MaterialToon ;
	MV1_LIGHT				*Light ;
	MV1_LIGHT_F1			*F1Light ;
	MV1_TEXTURE_BASE		*Texture ;
	MV1_TEXTURE_F1			*F1Texture ;
	MV1_MESH_BASE			*Mesh ;
	MV1_MESH_F1				*F1Mesh ;
	MV1_MESH_FACE			*Face ;
	MV1_SKIN_BONE			*SkinBone ;
	MV1_SKIN_BONE_F1		*F1SkinBone ;
	MV1_TRIANGLE_LIST_BASE	*TriangleList ;
	MV1_TRIANGLE_LIST_F1	*F1TriangleList ;
	MV1_ANIM_KEYSET_BASE	*AnimKeySet ;
	MV1_ANIM_KEYSET_F1		*F1AnimKeySet ;
	MV1_ANIM_BASE			*Anim ;
	MV1_ANIM_F1				*F1Anim ;
	MV1_ANIMSET_BASE		*AnimSet ;
	MV1_ANIMSET_F1			*F1AnimSet ;
	MV1_FILEHEAD_SHAPE_F1	*F1FileHeadShape ;
	MV1_FRAME_SHAPE_F1		*F1FrameShape ;
	MV1_SHAPE_BASE			*Shape ;
	MV1_SHAPE_F1			*F1Shape ;
	MV1_SHAPE_MESH_BASE		*ShapeMesh ;
	MV1_SHAPE_MESH_F1		*F1ShapeMesh ;
	MV1_SHAPE_VERTEX_BASE	*ShapeVertex ;
	MV1_SHAPE_VERTEX_F1		*F1ShapeVertex ;
	MV1_FILEHEAD_PHYSICS_F1		*F1FileHeadPhysics ;
	MV1_PHYSICS_RIGIDBODY_BASE	*PhysicsRigidBody ;
	MV1_PHYSICS_RIGIDBODY_F1	*F1PhysicsRigidBody ;
	MV1_PHYSICS_JOINT_BASE		*PhysicsJoint ;
	MV1_PHYSICS_JOINT_F1		*F1PhysicsJoint ;
	BYTE *FileStringBuffer ;
	int i, j, k, NewHandle = 0, dirlen ;
	DWORD_PTR AllocSize ;
	int MeshVertexNum ;
	int AutoNormalCreateFlag ;

	wchar_t DirectoryPath[ FILEPATH_MAX + 2 ] ;
	BYTE *Src ;

	AutoNormalCreateFlag = FALSE ;

	F1FileHeadShape = NULL ;
	F1FileHeadPhysics = NULL ;
	FHeader = ( MV1MODEL_FILEHEADER_F1 * )LoadParam->DataBuffer ;

	// ディレクトリパスを取得
	dirlen = 0 ;
	if( LoadParam->CurrentDir != NULL )
	{
		_WCSCPY( DirectoryPath, LoadParam->CurrentDir ) ;
		dirlen = _WCSLEN( DirectoryPath ) ;
		if( DirectoryPath[ dirlen - 1 ] != L'\\' && DirectoryPath[ dirlen - 1 ] != L'/' )
		{
			DirectoryPath[ dirlen     ] = L'/' ;
			DirectoryPath[ dirlen + 1 ] = L'\0' ;
			dirlen ++ ;
		}
	}

	// ＩＤチェック
	if( ( ( BYTE * )LoadParam->DataBuffer )[ 0 ] != 'M' || ( ( BYTE * )LoadParam->DataBuffer )[ 1 ] != 'V' ||
		( ( BYTE * )LoadParam->DataBuffer )[ 2 ] != '1' || ( ( BYTE * )LoadParam->DataBuffer )[ 3 ] != '1' )
		return -1 ;

	// 圧縮データを解凍するメモリ領域の確保
	FHeader = ( MV1MODEL_FILEHEADER_F1 * )DXALLOC( ( size_t )( DXA_Decode( ( BYTE * )LoadParam->DataBuffer + 4, NULL ) + 4 ) ) ;
	if( FHeader == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x4d\x00\x56\x00\x31\x00\x20\x00\x4c\x00\x6f\x00\x61\x00\x64\x00\x4d\x00\x6f\x00\x64\x00\x65\x00\x6c\x00\x20\x00\x45\x00\x72\x00\x72\x00\x6f\x00\x72\x00\x20\x00\x3a\x00\x20\x00\x27\x57\x2e\x7e\xc7\x30\xfc\x30\xbf\x30\x92\x30\xe3\x89\xcd\x51\x59\x30\x8b\x30\x5f\x30\x81\x30\x6e\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x00"/*@ L"MV1 LoadModel Error : 圧縮データを解凍するためのメモリ領域の確保に失敗しました" @*/ )) ;
		return -1 ;
	}
	DXA_Decode( ( BYTE * )LoadParam->DataBuffer + 4, ( BYTE * )FHeader + 4 ) ;

	// 必要なメモリサイズの算出
	{
		_MEMSET( &MTBase, 0, sizeof( MTBase ) ) ;
		AllocSize = 0 ;

		MTBase.Name = ( wchar_t * )AllocSize ;
		AllocSize += ( ( _WCSLEN( LoadParam->Name ) + 1 ) * sizeof( wchar_t ) + 3 ) / 4 * 4 ;

		MTBase.FilePath = ( wchar_t * )AllocSize ;
		AllocSize += ( ( _WCSLEN( LoadParam->FilePath ) + 1 ) * sizeof( wchar_t ) + 3 ) / 4 * 4 ;

		MTBase.DirectoryPath = ( wchar_t * )AllocSize ;
		AllocSize += ( ( dirlen + 1 ) * sizeof( wchar_t ) + 3 ) / 4 * 4 ;

		MTBase.Frame = ( MV1_FRAME_BASE * )AllocSize ;
		AllocSize += FHeader->FrameNum * sizeof( MV1_FRAME_BASE ) ;

		MTBase.FrameUseSkinBone = ( MV1_SKIN_BONE ** )AllocSize ;
		AllocSize += FHeader->FrameUseSkinBoneNum * sizeof( MV1_SKIN_BONE * ) ;

		MTBase.Material = ( MV1_MATERIAL_BASE * )AllocSize ;
		AllocSize += FHeader->MaterialNum * sizeof( MV1_MATERIAL_BASE ) ;

		MTBase.Light = ( MV1_LIGHT * )AllocSize ;
		AllocSize += FHeader->LightNum * sizeof( MV1_LIGHT ) ;

		MTBase.Texture = ( MV1_TEXTURE_BASE * )AllocSize ;
		AllocSize += FHeader->TextureNum * sizeof( MV1_TEXTURE_BASE ) ;

		MTBase.Mesh = ( MV1_MESH_BASE * )AllocSize ;
		AllocSize += FHeader->MeshNum * sizeof( MV1_MESH_BASE ) ;

		MTBase.SkinBone = ( MV1_SKIN_BONE * )AllocSize ;
		AllocSize += FHeader->SkinBoneNum * sizeof( MV1_SKIN_BONE ) ;
		MTBase.SkinBoneUseFrame = ( MV1_SKIN_BONE_USE_FRAME * )AllocSize ;
		AllocSize += FHeader->SkinBoneUseFrameNum * sizeof( MV1_SKIN_BONE_USE_FRAME ) ;

		MTBase.TriangleList = ( MV1_TRIANGLE_LIST_BASE * )AllocSize ;
		AllocSize += FHeader->TriangleListNum * sizeof( MV1_TRIANGLE_LIST_BASE ) ;

		MTBase.TriangleListIndex = ( WORD * )AllocSize ;
		AllocSize += sizeof( WORD ) * FHeader->TriangleListIndexNum * 2 ;

#ifndef UNICODE
		MTBase.StringBufferA = ( char * )AllocSize ;
		AllocSize += FHeader->StringSize * sizeof( char ) ;
#endif

		MTBase.StringBufferW = ( wchar_t * )AllocSize ;
		AllocSize += FHeader->StringSize * sizeof( wchar_t ) ;

		MTBase.ChangeDrawMaterialTable = ( DWORD * )AllocSize ;
		AllocSize += FHeader->ChangeDrawMaterialTableSize ;

		MTBase.ChangeMatrixTable = ( DWORD * )AllocSize ;
		AllocSize += FHeader->ChangeMatrixTableSize ;

		MTBase.AnimKeyData = ( void * )AllocSize ;
		AllocSize += FHeader->OriginalAnimKeyDataSize ;

		MTBase.AnimKeySet = ( MV1_ANIM_KEYSET_BASE * )AllocSize ;
		AllocSize += FHeader->AnimKeySetNum * sizeof( MV1_ANIM_KEYSET_BASE ) ;

		MTBase.Anim = ( MV1_ANIM_BASE * )AllocSize ;
		AllocSize += FHeader->AnimNum * sizeof( MV1_ANIM_BASE ) ;

		MTBase.AnimSet = ( MV1_ANIMSET_BASE * )AllocSize ;
		AllocSize += FHeader->AnimSetNum * sizeof( MV1_ANIMSET_BASE ) ;

		MTBase.AnimTargetFrameTable = ( MV1_ANIM_BASE ** )AllocSize ;
		AllocSize += FHeader->FrameNum * FHeader->AnimSetNum * sizeof( MV1_ANIM_BASE * ) ;

		if( FHeader->Shape != NULL )
		{
			F1FileHeadShape = ( MV1_FILEHEAD_SHAPE_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )FHeader->Shape ) ;

			MTBase.Shape = ( MV1_SHAPE_BASE * )AllocSize ;
			AllocSize += F1FileHeadShape->DataNum * sizeof( MV1_SHAPE_BASE ) ;

			MTBase.ShapeMesh = ( MV1_SHAPE_MESH_BASE * )AllocSize ;
			AllocSize += F1FileHeadShape->MeshNum * sizeof( MV1_SHAPE_MESH_BASE ) ;
		}

		if( FHeader->Physics != NULL && LoadParam->GParam.LoadModelToUsePhysicsMode != DX_LOADMODEL_PHYSICS_DISABLE )
		{
			F1FileHeadPhysics = ( MV1_FILEHEAD_PHYSICS_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )FHeader->Physics ) ;

			MTBase.PhysicsRigidBody = ( MV1_PHYSICS_RIGIDBODY_BASE * )AllocSize ;
			AllocSize += F1FileHeadPhysics->RigidBodyNum * sizeof( MV1_PHYSICS_RIGIDBODY_BASE ) ;

			MTBase.PhysicsJoint = ( MV1_PHYSICS_JOINT_BASE * )AllocSize ;
			AllocSize += F1FileHeadPhysics->JointNum * sizeof( MV1_PHYSICS_JOINT_BASE ) ;
		}
	}

	// モデル基データハンドルの作成
	NewHandle = MV1AddModelBase( ASyncThread ) ;
	if( NewHandle < 0 )
	{
		goto ERRORLABEL ;
	}

	// メモリの確保
	MV1BMDLCHK_ASYNC( NewHandle, MBase ) ;
	MBase->DataBuffer = MDALLOCMEM( AllocSize ) ;
	if( MBase->DataBuffer == NULL )
	{
		goto ERRORLABEL ;
	}
	_MEMSET( MBase->DataBuffer, 0, AllocSize ) ;
	MBase->AllocMemorySize = AllocSize ;

	// メモリアドレスのセット
	MBase->Name                   = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Name                 ) ;
	MBase->FilePath               = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.FilePath             ) ;
	MBase->DirectoryPath          = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.DirectoryPath        ) ;
	MBase->ChangeMatrixTable      = ( DWORD * )                     ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.ChangeMatrixTable    ) ;
	MBase->ChangeDrawMaterialTable = ( DWORD * )                    ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.ChangeDrawMaterialTable ) ;
	MBase->Frame                  = ( MV1_FRAME_BASE * )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Frame                ) ;
	MBase->FrameUseSkinBone       = ( MV1_SKIN_BONE ** )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.FrameUseSkinBone     ) ;
	MBase->Material               = ( MV1_MATERIAL_BASE * )         ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Material             ) ;
	MBase->Light                  = ( MV1_LIGHT * )                 ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Light                ) ;
	MBase->Texture                = ( MV1_TEXTURE_BASE * )          ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Texture              ) ;
	MBase->SkinBone               = ( MV1_SKIN_BONE * )             ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.SkinBone             ) ;
	MBase->SkinBoneUseFrame       = ( MV1_SKIN_BONE_USE_FRAME * )   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.SkinBoneUseFrame     ) ;
	MBase->Mesh                   = ( MV1_MESH_BASE * )             ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Mesh                 ) ;
	MBase->TriangleList           = ( MV1_TRIANGLE_LIST_BASE * )    ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.TriangleList         ) ;
	MBase->TriangleListIndex      = ( WORD * )                      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.TriangleListIndex    ) ;
#ifndef UNICODE
	MBase->StringBufferA          = ( char * )                      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.StringBufferA        ) ;
#endif
	MBase->StringBufferW          = ( wchar_t * )                   ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.StringBufferW        ) ;
	MBase->AnimSet                = ( MV1_ANIMSET_BASE * )          ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimSet              ) ;
	MBase->Anim                   = ( MV1_ANIM_BASE * )             ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Anim                 ) ;
	MBase->AnimKeySet             = ( MV1_ANIM_KEYSET_BASE * )      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimKeySet           ) ;
	MBase->AnimKeyData            = ( void * )                      ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimKeyData          ) ;
	MBase->AnimTargetFrameTable   = ( MV1_ANIM_BASE ** )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.AnimTargetFrameTable ) ;
	if( FHeader->Shape )
	{
		MBase->Shape              = ( MV1_SHAPE_BASE * )            ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.Shape                ) ;
		MBase->ShapeMesh          = ( MV1_SHAPE_MESH_BASE * )       ( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.ShapeMesh            ) ;
	}
	if( FHeader->Physics && LoadParam->GParam.LoadModelToUsePhysicsMode != DX_LOADMODEL_PHYSICS_DISABLE )
	{
		MBase->PhysicsRigidBody   = ( MV1_PHYSICS_RIGIDBODY_BASE * )( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.PhysicsRigidBody     ) ;
		MBase->PhysicsJoint       = ( MV1_PHYSICS_JOINT_BASE *     )( ( BYTE * )MBase->DataBuffer + ( DWORD_PTR )MTBase.PhysicsJoint         ) ;
	}

	// モデル名とファイルパスとフォルダパスを保存
	_WCSCPY( MBase->Name,          LoadParam->Name ) ;
	_WCSCPY( MBase->FilePath,      LoadParam->FilePath ) ;
	_WCSCPY( MBase->DirectoryPath, LoadParam->CurrentDir == NULL ? L"" : DirectoryPath ) ;

	// 同時複数描画に対応するかどうかを保存
	MBase->UsePackDraw = LoadParam->GParam.LoadModelToUsePackDraw ;

	// 座標系情報を保存
	MBase->RightHandType = FHeader->RightHandType ;

	// 割り当てられているマテリアルの番号が低いメッシュから描画するかどうかの情報を保存
	MBase->MaterialNumberOrderDraw = FHeader->MaterialNumberOrderDraw ;

	// 変更チェック用テーブルに必要なデータのサイズの保存とコピー
	MBase->ChangeDrawMaterialTableSize  = FHeader->ChangeDrawMaterialTableSize ;
	MBase->ChangeMatrixTableSize        = FHeader->ChangeMatrixTableSize ;
	_MEMCPY( MBase->ChangeDrawMaterialTable, ( BYTE * )FHeader + ( DWORD_PTR )FHeader->ChangeDrawMaterialTable, ( size_t )FHeader->ChangeDrawMaterialTableSize ) ;
	_MEMCPY( MBase->ChangeMatrixTable,       ( BYTE * )FHeader + ( DWORD_PTR )FHeader->ChangeMatrixTable,       ( size_t )FHeader->ChangeMatrixTableSize ) ;

	// ポリゴン数を保存
	MBase->TriangleNum = FHeader->TriangleNum ;

	// 頂点データの数を保存
	MBase->TriangleListVertexNum = FHeader->TriangleListVertexNum ;

	// フレームが使用しているボーンへのポインタの情報をコピー
	MBase->FrameUseSkinBoneNum = FHeader->FrameUseSkinBoneNum ;
	for( i = 0 ; i < MBase->FrameUseSkinBoneNum ; i ++ )
	{
		MBase->FrameUseSkinBone[ i ] = MBase->SkinBone + ( ( ( DWORD * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )FHeader->FrameUseSkinBone ) )[ i ] - FHeader->SkinBone ) / sizeof( MV1_SKIN_BONE_F1 ) ;
	}

	// スキニング用ボーンを使用するフレームの情報をコピー
	_MEMCPY( MBase->SkinBoneUseFrame, ( BYTE * )FHeader + ( DWORD_PTR )FHeader->SkinBoneUseFrame, FHeader->SkinBoneUseFrameNum * sizeof( MV1_SKIN_BONE_USE_FRAME_F1 ) ) ;

	// 文字列データをコピー
	FileStringBuffer = ( BYTE * )FHeader + FHeader->StringBuffer ;
	if( FHeader->IsStringUTF8 )
	{
#ifndef UNICODE
		MBase->StringSizeA = 0 ;
#endif
		MBase->StringSizeW = 0 ;
	}
	else
	{
#ifndef UNICODE
		_MEMCPY( MBase->StringBufferA, FileStringBuffer, ( size_t )FHeader->StringSize ) ;
		MBase->StringSizeA = FHeader->StringSize ;
#endif
		MBase->StringSizeW = 0 ;
	}

	// 各オブジェクトの数の情報を保存
	MBase->FrameNum              = FHeader->FrameNum ;
	MBase->MaterialNum           = FHeader->MaterialNum ;
	MBase->LightNum              = FHeader->LightNum ;
	MBase->TextureNum            = FHeader->TextureNum ;
	MBase->MeshNum               = FHeader->MeshNum ;
	MBase->SkinBoneNum           = FHeader->SkinBoneNum ;
	MBase->SkinBoneUseFrameNum   = FHeader->SkinBoneUseFrameNum ;
	MBase->TriangleListNum       = FHeader->TriangleListNum ;
	MBase->AnimKeyDataSize       = 0 ;
	MBase->AnimKeySetNum         = FHeader->AnimKeySetNum ;
	MBase->AnimNum               = FHeader->AnimNum ;
	MBase->AnimSetNum            = FHeader->AnimSetNum ;
	if( FHeader->Shape )
	{
		MBase->ShapeNum          = F1FileHeadShape->DataNum ;
		MBase->ShapeMeshNum      = F1FileHeadShape->MeshNum ;

		MBase->ShapeNormalPositionNum = F1FileHeadShape->NormalPositionNum ;
		MBase->ShapeSkinPosition4BNum = F1FileHeadShape->SkinPosition4BNum ;
		MBase->ShapeSkinPosition8BNum = F1FileHeadShape->SkinPosition8BNum ;
		MBase->ShapeSkinPositionFREEBSize = F1FileHeadShape->SkinPositionFREEBSize ;

		MBase->ShapeTargetMeshVertexNum = F1FileHeadShape->TargetMeshVertexNum ;
	}
	if( FHeader->Physics && LoadParam->GParam.LoadModelToUsePhysicsMode != DX_LOADMODEL_PHYSICS_DISABLE )
	{
		MBase->PhysicsGravity    = F1FileHeadPhysics->WorldGravity ;

		MBase->PhysicsRigidBodyNum = F1FileHeadPhysics->RigidBodyNum ;
		MBase->PhysicsJointNum   = F1FileHeadPhysics->JointNum ;
	}

	// メッシュ頂点データサイズの計算( FHeader->MeshVertexSize が古いバージョンと新しいバージョンで異なってしまったので )
	{
		DWORD MeshVertexSize ;

		MeshVertexSize = 0 ;

		Mesh = MBase->Mesh ;
		F1Mesh = ( MV1_MESH_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Mesh ) ;
		for( i = 0 ; i < MBase->MeshNum ; i ++, Mesh ++, F1Mesh = ( MV1_MESH_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Mesh->DimNext ) )
		{
			Mesh->UVSetUnitNum = F1Mesh->UVSetUnitNum ;
			Mesh->UVUnitNum = F1Mesh->UVUnitNum ;
			Mesh->VertexNum = F1Mesh->VertexNum ;
			Mesh->VertUnitSize = ( int )( sizeof( MV1_MESH_VERTEX ) + Mesh->UVSetUnitNum * Mesh->UVUnitNum * sizeof( float ) - sizeof( float ) * 2 ) ;

			MeshVertexSize += Mesh->VertexNum * Mesh->VertUnitSize ;
		}

		FHeader->MeshVertexSize = ( int )MeshVertexSize ;
	}


	// 頂点データを格納するためのメモリ領域の確保
	{
		MBase->VertexDataSize = 
			FHeader->MeshVertexIndexNum                * sizeof( DWORD )                 +
			FHeader->MeshFaceNum                       * sizeof( MV1_MESH_FACE )         +
			FHeader->MeshNormalNum                     * sizeof( MV1_MESH_NORMAL )       +
			FHeader->TriangleListNormalPositionNum     * sizeof( MV1_TLIST_NORMAL_POS )  +
			FHeader->TriangleListSkinPosition4BNum     * sizeof( MV1_TLIST_SKIN_POS_4B ) +
			FHeader->TriangleListSkinPosition8BNum     * sizeof( MV1_TLIST_SKIN_POS_8B ) +
			FHeader->TriangleListSkinPositionFREEBSize                                   +
			FHeader->MeshPositionSize                                                    +
			FHeader->MeshVertexSize                                                      + 16 ;
		F1Mesh = ( MV1_MESH_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Mesh ) ;
		if( ( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_NON_TOON_OUTLINE ) == 0 )
		{
			MeshVertexNum = 0 ;
			for( i = 0 ; i < FHeader->MeshNum ; i ++, F1Mesh = ( MV1_MESH_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Mesh->DimNext ) )
			{
				MeshVertexNum += F1Mesh->VertexNum ;
			}
			MBase->MeshVertexSize += MeshVertexNum * sizeof( DWORD ) ;
			MBase->VertexDataSize += MeshVertexNum * sizeof( DWORD ) ;
		}
		if( FHeader->Shape )
		{
			MBase->VertexDataSize += F1FileHeadShape->VertexNum * sizeof( MV1_SHAPE_VERTEX_BASE ) ;
		}
		MBase->VertexData = DXALLOC( MBase->VertexDataSize ) ;
		if( MBase->VertexData == NULL ) goto ERRORLABEL ;
		_MEMSET( MBase->VertexData, 0, MBase->VertexDataSize ) ;

		MBase->TriangleListNormalPosition    = ( MV1_TLIST_NORMAL_POS     * )( ( ( DWORD_PTR )MBase->VertexData + 15 ) / 16 * 16 ) ;
		MBase->TriangleListSkinPosition4B    = ( MV1_TLIST_SKIN_POS_4B    * )( MBase->TriangleListNormalPosition              + FHeader->TriangleListNormalPositionNum     ) ;
		MBase->TriangleListSkinPosition8B    = ( MV1_TLIST_SKIN_POS_8B    * )( MBase->TriangleListSkinPosition4B              + FHeader->TriangleListSkinPosition4BNum     ) ;
		MBase->TriangleListSkinPositionFREEB = ( MV1_TLIST_SKIN_POS_FREEB * )( MBase->TriangleListSkinPosition8B              + FHeader->TriangleListSkinPosition8BNum     ) ;
		MBase->MeshVertexIndex               = ( DWORD                    * )( ( BYTE * )MBase->TriangleListSkinPositionFREEB + FHeader->TriangleListSkinPositionFREEBSize ) ;
		MBase->MeshFace                      = ( MV1_MESH_FACE            * )( MBase->MeshVertexIndex                         + FHeader->MeshVertexIndexNum    ) ;
		MBase->MeshNormal                    = ( MV1_MESH_NORMAL          * )( MBase->MeshFace                                + FHeader->MeshFaceNum           ) ;
		MBase->MeshPosition                  = ( MV1_MESH_POSITION        * )( MBase->MeshNormal                              + FHeader->MeshNormalNum         ) ;
		MBase->MeshVertex                    = ( MV1_MESH_VERTEX          * )( ( BYTE * )MBase->MeshPosition                  + FHeader->MeshPositionSize      ) ;
		if( FHeader->Shape )
		{
			MBase->ShapeVertex               = ( MV1_SHAPE_VERTEX_BASE    * )( ( BYTE * )MBase->MeshVertex                    + FHeader->MeshVertexSize        ) ;
		}
		if( FHeader->TriangleListNormalPositionNum == 0     ) MBase->TriangleListNormalPosition    = NULL ;
		if( FHeader->TriangleListSkinPosition4BNum == 0     ) MBase->TriangleListSkinPosition4B    = NULL ;
		if( FHeader->TriangleListSkinPosition8BNum == 0     ) MBase->TriangleListSkinPosition8B    = NULL ;
		if( FHeader->TriangleListSkinPositionFREEBSize == 0 ) MBase->TriangleListSkinPositionFREEB = NULL ;
		if( FHeader->MeshVertexIndexNum == 0                ) MBase->MeshVertexIndex   = NULL ;
		if( FHeader->MeshFaceNum == 0                       ) MBase->MeshFace          = NULL ;
		if( FHeader->MeshNormalNum == 0                     ) MBase->MeshNormal        = NULL ;
		if( FHeader->MeshPositionSize == 0                  ) MBase->MeshPosition      = NULL ;
		if( FHeader->MeshVertexSize == 0                    ) MBase->MeshVertex        = NULL ;
		if( FHeader->Shape == NULL || F1FileHeadShape->VertexNum == 0 ) MBase->ShapeVertex = NULL ;
	}

	// フレームの情報をセット
	MBase->TopFrameNum  = FHeader->TopFrameNum ;
	MBase->FirstTopFrame = FHeader->FirstTopFrame ? MBase->Frame + ( ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->FirstTopFrame ) )->Index : NULL ;
	MBase->LastTopFrame  = FHeader->LastTopFrame  ? MBase->Frame + ( ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->LastTopFrame ) )->Index  : NULL ;
	Frame = MBase->Frame ;
	F1Frame = ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Frame ) ;
	for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++, F1Frame = ( MV1_FRAME_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Frame->DimNext ) )
	{
		Frame->Container = MBase ;

		MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1Frame->Name,
#ifndef UNICODE
			&Frame->NameA,
#endif
			&Frame->NameW
		) ;

		Frame->AutoCreateNormal = F1Frame->AutoCreateNormal ;

		Frame->Index = F1Frame->Index ;

		Frame->ChangeDrawMaterialInfo.Target   = F1Frame->ChangeDrawMaterialInfo.Target ;
		Frame->ChangeDrawMaterialInfo.Fill     = MBase->ChangeDrawMaterialTable + ( ( DWORD * )( DWORD_PTR )F1Frame->ChangeDrawMaterialInfo.Fill - ( DWORD * )( DWORD_PTR )FHeader->ChangeDrawMaterialTable ) ;
		Frame->ChangeDrawMaterialInfo.Size     = F1Frame->ChangeDrawMaterialInfo.Size ;
		Frame->ChangeDrawMaterialInfo.CheckBit = F1Frame->ChangeDrawMaterialInfo.CheckBit ;

		Frame->ChangeMatrixInfo.Target         = F1Frame->ChangeMatrixInfo.Target ;
		Frame->ChangeMatrixInfo.Fill           = MBase->ChangeMatrixTable       + ( ( DWORD * )( DWORD_PTR )F1Frame->ChangeMatrixInfo.Fill - ( DWORD * )( DWORD_PTR )FHeader->ChangeMatrixTable ) ;
		Frame->ChangeMatrixInfo.Size           = F1Frame->ChangeMatrixInfo.Size ;
		Frame->ChangeMatrixInfo.CheckBit       = F1Frame->ChangeMatrixInfo.CheckBit ;

		Frame->TotalMeshNum  = F1Frame->TotalMeshNum ;
		Frame->TotalChildNum = F1Frame->TotalChildNum ;
		Frame->Parent     = F1Frame->Parent     ? MBase->Frame + ( ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1Frame->Parent     ) )->Index : NULL ;
		Frame->FirstChild = F1Frame->FirstChild ? MBase->Frame + ( ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1Frame->FirstChild ) )->Index : NULL ;
		Frame->LastChild  = F1Frame->LastChild  ? MBase->Frame + ( ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1Frame->LastChild  ) )->Index : NULL ;
		Frame->Prev       = F1Frame->Prev       ? MBase->Frame + ( ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1Frame->Prev       ) )->Index : NULL ;
		Frame->Next       = F1Frame->Next       ? MBase->Frame + ( ( MV1_FRAME_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1Frame->Next       ) )->Index : NULL ;

		Frame->Translate = F1Frame->Translate ;
		if( LoadParam->GParam.LoadModelToIgnoreScaling )
		{
			Frame->Scale = VGet( 1.0f, 1.0f, 1.0f ); ;
		}
		else
		{
			Frame->Scale = F1Frame->Scale ;
		}
		Frame->Rotate = F1Frame->Rotate ;
		Frame->RotateOrder = F1Frame->RotateOrder ;
		Frame->Quaternion = F1Frame->Quaternion ;

		Frame->Flag = F1Frame->Flag ;
		if( F1Frame->Flag & MV1_FRAMEFLAG_IGNOREPARENTTRANS ) Frame->IgnoreParentTransform = 1 ;

		if( F1Frame->Flag & MV1_FRAMEFLAG_PREROTATE )
		{
			Frame->PreRotate = F1Frame->PreRotate ;
		}

		if( F1Frame->Flag & MV1_FRAMEFLAG_POSTROTATE )
		{
			Frame->PreRotate = F1Frame->PostRotate ;
		}

		Frame->IsSkinMesh = F1Frame->IsSkinMesh ;
		Frame->TriangleNum = F1Frame->TriangleNum ;
		Frame->VertexNum = F1Frame->VertexNum ;
		Frame->MeshNum = F1Frame->MeshNum ;
		if( F1Frame->Mesh )
		{
			Frame->Mesh = MBase->Mesh + ( ( MV1_MESH_F1 * )( ( DWORD_PTR )F1Frame->Mesh + ( DWORD_PTR )FHeader ) )->Index ;
		}

		if( F1Frame->FrameShape )
		{
			F1FrameShape = ( MV1_FRAME_SHAPE_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Frame->FrameShape ) ;
			Frame->ShapeNum = F1FrameShape->ShapeNum ;
			Frame->Shape = MBase->Shape + ( ( MV1_SHAPE_F1 * )( ( DWORD_PTR )F1FrameShape->Shape + ( DWORD_PTR )FHeader ) )->Index ;
		}

		Frame->SkinBoneNum = F1Frame->SkinBoneNum ;
		if( F1Frame->SkinBone )
		{
			Frame->SkinBone = MBase->SkinBone + ( ( MV1_SKIN_BONE_F1 * )( ( DWORD_PTR )F1Frame->SkinBone + ( DWORD_PTR )FHeader ) )->Index ;
		}

		if( F1Frame->Light )
		{
			Frame->Light = MBase->Light + ( ( MV1_LIGHT_F1 * )( ( DWORD_PTR )F1Frame->Light + ( DWORD_PTR )FHeader ) )->Index ;
		}

		Frame->UseSkinBoneNum = F1Frame->UseSkinBoneNum ;
		if( F1Frame->UseSkinBoneNum )
		{
			Frame->UseSkinBone = MBase->FrameUseSkinBone + ( F1Frame->UseSkinBone - FHeader->FrameUseSkinBone ) / sizeof( DWORD /* MV1_SKIN_BONE_F1** */ ) ;
		}

		Frame->SmoothingAngle = F1Frame->SmoothingAngle ;

		Frame->MaxBoneBlendNum = F1Frame->MaxBoneBlendNum ;
		Frame->PositionNum     = F1Frame->PositionNum ;
		Frame->NormalNum       = F1Frame->NormalNum ;

		// 座標データと法線データのコピー
		if( F1Frame->PositionNum == 0 )
		{
			Frame->Position = NULL ;
			Frame->Normal = NULL ;
		}
		else
		{
			MV1_MESH_POSITION *Pos ;
			MV1_MESH_NORMAL *Nrm ;
			MV1_POSITION_16BIT_SUBINFO_F1 xs, ys, zs ;

			Frame->PosUnitSize = ( int )( sizeof( MV1_MESH_POSITION ) + ( Frame->MaxBoneBlendNum - 4 ) * sizeof( MV1_SKINBONE_BLEND ) ) ;

			Frame->Position = ( MV1_MESH_POSITION * )( ( BYTE * )MBase->MeshPosition + MBase->MeshPositionSize ) ;
			Frame->Normal   = MBase->MeshNormal + MBase->MeshNormalNum ;
			MBase->MeshPositionSize += Frame->PosUnitSize * Frame->PositionNum ;
			MBase->MeshNormalNum    += Frame->NormalNum ;

			Src = ( BYTE * )( DWORD_PTR )F1Frame->PositionAndNormalData + ( DWORD_PTR )FHeader ;

			// 座標データの取得
			if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_POSITION_B16 )
			{
				xs = ( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Src )[ 0 ] ;
				ys = ( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Src )[ 1 ] ;
				zs = ( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Src )[ 2 ] ;
				Src += 24 ;

				Pos = Frame->Position ;
				for( j = 0 ; j < Frame->PositionNum ; j ++, Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Pos + Frame->PosUnitSize ) )
				{
					Pos->Position.x = ( ( WORD * )Src )[ 0 ] * xs.Width / 60000.0f + xs.Min ;
					Pos->Position.y = ( ( WORD * )Src )[ 1 ] * ys.Width / 60000.0f + ys.Min ;
					Pos->Position.z = ( ( WORD * )Src )[ 2 ] * zs.Width / 60000.0f + zs.Min ;
					Src += 6 ;
				}
			}
			else
			{
				Pos = Frame->Position ;
				for( j = 0 ; j < Frame->PositionNum ; j ++, Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Pos + Frame->PosUnitSize ) )
				{
					Pos->Position = *( ( VECTOR * )Src ) ;
					Src += 12 ;
				}
			}

			// 頂点ウエイトデータの取得
			if( ( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_MATRIX_WEIGHT_NONE ) == 0 )
			{
				DWORD IndexType, WeightType ;
				float TotalWeight ;
				int LastIndex ;

				IndexType  = ( DWORD )( ( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_MATRIX_INDEX_MASK  ) >> 4 ) ;
				WeightType = ( DWORD )( ( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_MATRIX_WEIGHT_MASK ) >> 5 ) ;

				Pos = Frame->Position ;
				for( j = 0 ; j < Frame->PositionNum ; j ++, Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Pos + Frame->PosUnitSize ) )
				{
					TotalWeight = 0.0f ;
					LastIndex = -1 ;
					for( k = 0 ; k < Frame->MaxBoneBlendNum ; k ++ )
					{
						if( IndexType == MV1_FRAME_MATRIX_INDEX_TYPE_U8 )
						{
							Pos->BoneWeight[ k ].Index = *Src == 255 ? -1 : ( int )*Src ;
							Src ++ ;
						}
						else
						{
							Pos->BoneWeight[ k ].Index = *( ( WORD * )Src ) == 65535 ? -1 : ( int )*( ( WORD * )Src ) ;
							Src += 2 ;
						}

						if( Pos->BoneWeight[ k ].Index == -1 )
							break ;

						LastIndex = k ;

						if( WeightType == MV1_FRAME_MATRIX_WEIGHT_TYPE_U8 )
						{
							Pos->BoneWeight[ k ].W = *Src == 255 ? 1.0f : *Src / 255.0f ;
							Src ++ ;
						}
						else
						{
							Pos->BoneWeight[ k ].W = *( ( WORD * )Src ) == 65535 ? 1.0f : *( ( WORD * )Src ) / 65535.0f ;
							Src += 2 ;
						}
						TotalWeight += Pos->BoneWeight[ k ].W ;
					}
					if( TotalWeight < 1.0f && LastIndex > 0 )
					{
						Pos->BoneWeight[ LastIndex ].W += 1.0f - TotalWeight ;
					}
				}
			}

			// 法線データの取得
			Nrm = Frame->Normal ;
			switch( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NORMAL_TYPE_MASK )
			{
			case MV1_FRAME_NORMAL_TYPE_NONE :
				AutoNormalCreateFlag = TRUE ;
				break ;

			case MV1_FRAME_NORMAL_TYPE_S8 :
				if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NOMRAL_TANGENT_BINORMAL )
				{
					for( j = 0 ; j < Frame->NormalNum ; j ++, Nrm ++ )
					{
						Nrm->Normal.x   = ( ( char * )Src )[ 0 ] / 127.0f ;
						Nrm->Normal.y   = ( ( char * )Src )[ 1 ] / 127.0f ;
						Nrm->Normal.z   = ( ( char * )Src )[ 2 ] / 127.0f ;

						Nrm->Tangent.x  = ( ( char * )Src )[ 3 ] / 127.0f ;
						Nrm->Tangent.y  = ( ( char * )Src )[ 4 ] / 127.0f ;
						Nrm->Tangent.z  = ( ( char * )Src )[ 5 ] / 127.0f ;

						Nrm->Binormal.x = ( ( char * )Src )[ 6 ] / 127.0f ;
						Nrm->Binormal.y = ( ( char * )Src )[ 7 ] / 127.0f ;
						Nrm->Binormal.z = ( ( char * )Src )[ 8 ] / 127.0f ;
						Src += 9 ;
					}
				}
				else
				{
					for( j = 0 ; j < Frame->NormalNum ; j ++, Nrm ++ )
					{
						Nrm->Normal.x = ( ( char * )Src )[ 0 ] / 127.0f ;
						Nrm->Normal.y = ( ( char * )Src )[ 1 ] / 127.0f ;
						Nrm->Normal.z = ( ( char * )Src )[ 2 ] / 127.0f ;
						Src += 3 ;
					}
				}
				break ;

			case MV1_FRAME_NORMAL_TYPE_S16 :
				if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NOMRAL_TANGENT_BINORMAL )
				{
					for( j = 0 ; j < Frame->NormalNum ; j ++, Nrm ++ )
					{
						Nrm->Normal.x   = ( ( short * )Src )[ 0 ] / 32767.0f ;
						Nrm->Normal.y   = ( ( short * )Src )[ 1 ] / 32767.0f ;
						Nrm->Normal.z   = ( ( short * )Src )[ 2 ] / 32767.0f ;

						Nrm->Tangent.x  = ( ( short * )Src )[ 3 ] / 32767.0f ;
						Nrm->Tangent.y  = ( ( short * )Src )[ 4 ] / 32767.0f ;
						Nrm->Tangent.z  = ( ( short * )Src )[ 5 ] / 32767.0f ;

						Nrm->Binormal.x = ( ( short * )Src )[ 6 ] / 32767.0f ;
						Nrm->Binormal.y = ( ( short * )Src )[ 7 ] / 32767.0f ;
						Nrm->Binormal.z = ( ( short * )Src )[ 8 ] / 32767.0f ;
						Src += 18 ;
					}
				}
				else
				{
					for( j = 0 ; j < Frame->NormalNum ; j ++, Nrm ++ )
					{
						Nrm->Normal.x = ( ( short * )Src )[ 0 ] / 32767.0f ;
						Nrm->Normal.y = ( ( short * )Src )[ 1 ] / 32767.0f ;
						Nrm->Normal.z = ( ( short * )Src )[ 2 ] / 32767.0f ;
						Src += 6 ;
					}
				}
				break ;

			case MV1_FRAME_NORMAL_TYPE_F32 :
				if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NOMRAL_TANGENT_BINORMAL )
				{
					for( j = 0 ; j < Frame->NormalNum ; j ++, Nrm ++ )
					{
						Nrm->Normal.x   = ( ( float * )Src )[ 0 ] ;
						Nrm->Normal.y   = ( ( float * )Src )[ 1 ] ;
						Nrm->Normal.z   = ( ( float * )Src )[ 2 ] ;

						Nrm->Tangent.x  = ( ( float * )Src )[ 3 ] ;
						Nrm->Tangent.y  = ( ( float * )Src )[ 4 ] ;
						Nrm->Tangent.z  = ( ( float * )Src )[ 5 ] ;

						Nrm->Binormal.x = ( ( float * )Src )[ 6 ] ;
						Nrm->Binormal.y = ( ( float * )Src )[ 7 ] ;
						Nrm->Binormal.z = ( ( float * )Src )[ 8 ] ;
						Src += 36 ;
					}
				}
				else
				{
					for( j = 0 ; j < Frame->NormalNum ; j ++, Nrm ++ )
					{
						Nrm->Normal.x = ( ( float * )Src )[ 0 ] ;
						Nrm->Normal.y = ( ( float * )Src )[ 1 ] ;
						Nrm->Normal.z = ( ( float * )Src )[ 2 ] ;
						Src += 12 ;
					}
				}
				break ;
			}
		}
	}

	// テクスチャの情報をセット
	MBase->TextureAllocMem = FALSE ;
	Texture = MBase->Texture ;
	F1Texture = ( MV1_TEXTURE_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Texture ) ;
	for( i = 0 ; i < MBase->TextureNum ; i ++, Texture ++, F1Texture = ( MV1_TEXTURE_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Texture->DimNext ) )
	{
		// ユーザーデータを初期化
		Texture->UserData[ 0 ] = 0 ;
		Texture->UserData[ 1 ] = 0 ;
		Texture->UseUserGraphHandle = 0 ;
		Texture->UserGraphHandle = 0 ;

		// 名前を保存
		Texture->NameAllocMem = FALSE ;
		MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1Texture->Name,
#ifndef UNICODE
			&Texture->NameA,
#endif
			&Texture->NameW
		) ;

		// 反転フラグをセットする
		Texture->ReverseFlag = ( F1Texture->Flag & MV1_TEXTURE_FLAG_REVERSE ) != 0 ? 1 : 0 ;

		// ３２ビットＢＭＰのアルファ値が全部０だったら XRGB8 として扱うかどうかのフラグをセットする
		Texture->Bmp32AllZeroAlphaToXRGB8Flag = ( F1Texture->Flag & MV1_TEXTURE_FLAG_BMP32_ALL_ZERO_ALPHA_TO_XRGB8 ) != 0 ? 1 : 0 ;

		// テクスチャの読み込み
		{
			wchar_t ColorPathW[ FILEPATH_MAX ] ;
			wchar_t AlphaPathW[ FILEPATH_MAX ] ;

			if( F1Texture->ColorFilePath )
			{
				MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1Texture->ColorFilePath,
#ifndef UNICODE
					&Texture->ColorFilePathA,
#endif
					&Texture->ColorFilePathW
				) ;

				if( LoadParam->CurrentDir == NULL )
				{
					_WCSCPY( ColorPathW, Texture->ColorFilePathW ) ;
				}
				else
				{
					ConvertFullPathW_( Texture->ColorFilePathW, ColorPathW, DirectoryPath ) ;
				}
			}

			if( F1Texture->AlphaFilePath )
			{
				MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1Texture->AlphaFilePath,
#ifndef UNICODE
					&Texture->AlphaFilePathA,
#endif
					&Texture->AlphaFilePathW
				) ;

				if( LoadParam->CurrentDir == NULL )
				{
					_WCSCPY( AlphaPathW, Texture->AlphaFilePathW ) ;
				}
				else
				{
					ConvertFullPathW_( Texture->AlphaFilePathW, AlphaPathW, DirectoryPath ) ;
				}
			}

			if( __MV1LoadTexture(
					&Texture->ColorImage, &Texture->ColorImageSize,
					&Texture->AlphaImage, &Texture->AlphaImageSize,
					&Texture->GraphHandle,
					&Texture->SemiTransFlag,
					&Texture->IsDefaultTexture,
#ifndef UNICODE
					NULL, NULL,
#endif
					NULL, NULL,
					F1Texture->ColorFilePath ? ColorPathW : NULL,
					F1Texture->AlphaFilePath ? AlphaPathW : NULL,
					LoadParam->CurrentDir == NULL ? NULL : DirectoryPath,
					F1Texture->BumpImageFlag, F1Texture->BumpImageNextPixelLength,
					Texture->ReverseFlag,
					Texture->Bmp32AllZeroAlphaToXRGB8Flag,
					LoadParam->FileReadFunc,
					false,
					ASyncThread ) == -1 )
			{
				DXST_ERRORLOGFMT_ADDW(( L"MV1 LoadModel Error : Txture Load Error : %s\n", Texture->NameW ) ) ;
				goto ERRORLABEL ;
			}

			// ファイルパス用にメモリを確保したフラグを倒す
			Texture->ColorImageFilePathAllocMem = FALSE ;
			Texture->AlphaImageFilePathAllocMem = FALSE ;

			// ファイルパスを保存
			if( Texture->ColorImage == NULL || F1Texture->ColorFilePath == 0 )
			{
#ifndef UNICODE
				Texture->ColorFilePathA = NULL ;
#endif
				Texture->ColorFilePathW = NULL ;
			}
			if( Texture->AlphaImage == NULL || F1Texture->AlphaFilePath == 0 )
			{
#ifndef UNICODE
				Texture->AlphaFilePathA = NULL ;
#endif
				Texture->AlphaFilePathW = NULL ;
			}
		}

		// アドレッシングモードのセット
		Texture->AddressModeU = F1Texture->AddressModeU ;
		Texture->AddressModeV = F1Texture->AddressModeV ;

		// スケーリングのセット
		if( ( F1Texture->Flag & MV1_TEXTURE_FLAG_VALID_SCALE_UV ) != 0 )
		{
			Texture->ScaleU = F1Texture->ScaleU ;
			Texture->ScaleV = F1Texture->ScaleV ;
		}
		else
		{
			Texture->ScaleU = 1.0f ;
			Texture->ScaleV = 1.0f ;
		}

		// フィルタリングモードのセット
		Texture->FilterMode = F1Texture->FilterMode ;

		// バンプマップ情報を保存
		Texture->BumpImageFlag = F1Texture->BumpImageFlag ;
		Texture->BumpImageNextPixelLength = F1Texture->BumpImageNextPixelLength ;

		// 画像のサイズを取得
		NS_GetGraphSize( Texture->GraphHandle, &Texture->Width, &Texture->Height ) ;
	}

	// マテリアルの情報をセット
	Material = MBase->Material ;
	F1Material = ( MV1_MATERIAL_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Material ) ;
	for( i = 0 ; i < MBase->MaterialNum ; i ++, Material ++, F1Material = ( MV1_MATERIAL_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Material->DimNext ) )
	{
		MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1Material->Name,
#ifndef UNICODE
			&Material->NameA,
#endif
			&Material->NameW
		) ;

		Material->Diffuse  = F1Material->Diffuse ;
		Material->Ambient  = F1Material->Ambient ;
		Material->Specular = F1Material->Specular ;
		Material->Emissive = F1Material->Emissive ;
		Material->Power    = F1Material->Power ;

		Material->UseAlphaTest = F1Material->UseAlphaTest ;
		Material->AlphaFunc    = F1Material->AlphaFunc ;
		Material->AlphaRef     = F1Material->AlphaRef ;

		Material->DiffuseLayerNum = F1Material->DiffuseLayerNum ;
		MaterialLayer      = Material->DiffuseLayer ;
		F1MaterialLayer    = F1Material->DiffuseLayer ;
		for( j = 0 ; j < 8 ; j ++, MaterialLayer ++, F1MaterialLayer ++ )
		{
			MaterialLayer->BlendType = F1MaterialLayer->BlendType ;
			MaterialLayer->Texture = F1MaterialLayer->Texture ;
		}

		Material->SpecularLayerNum = F1Material->SpecularLayerNum ;
		MaterialLayer      = Material->SpecularLayer ;
		F1MaterialLayer    = F1Material->SpecularLayer ;
		for( j = 0 ; j < 8 ; j ++, MaterialLayer ++, F1MaterialLayer ++ )
		{
			MaterialLayer->BlendType = F1MaterialLayer->BlendType ;
			MaterialLayer->Texture = F1MaterialLayer->Texture ;
		}

		Material->NormalLayerNum = F1Material->NormalLayerNum ;
		MaterialLayer      = Material->NormalLayer ;
		F1MaterialLayer    = F1Material->NormalLayer ;
		for( j = 0 ; j < 8 ; j ++, MaterialLayer ++, F1MaterialLayer ++ )
		{
			MaterialLayer->BlendType = F1MaterialLayer->BlendType ;
			MaterialLayer->Texture = F1MaterialLayer->Texture ;
		}

		Material->DrawBlendMode = F1Material->DrawBlendMode ;
		Material->DrawBlendParam = F1Material->DrawBlendParam ;

		if( F1Material->ToonInfo )
		{
			F1MaterialToon = ( MV1_MATERIAL_TOON_F1 * )( ( DWORD_PTR )F1Material->ToonInfo + ( DWORD_PTR )FHeader ) ;

			Material->Type = F1MaterialToon->Type ;
			Material->DiffuseGradBlendType = F1MaterialToon->DiffuseGradBlendType ;
			Material->DiffuseGradTexture = F1MaterialToon->DiffuseGradTexture ;
			Material->SpecularGradBlendType = F1MaterialToon->SpecularGradBlendType ;
			Material->SpecularGradTexture = F1MaterialToon->SpecularGradTexture ;
			Material->OutLineColor = F1MaterialToon->OutLineColor ;
			Material->OutLineWidth = F1MaterialToon->OutLineWidth ;
			Material->OutLineDotWidth = F1MaterialToon->OutLineDotWidth ;

			if( F1MaterialToon->EnableSphereMap != 0 )
			{
				Material->SphereMapTexture = ( int )F1MaterialToon->SphereMapTexture ;
				Material->SphereMapBlendType = ( int )F1MaterialToon->SphereMapBlendType ;
			}
			else
			{
				Material->SphereMapTexture = -1 ;
				Material->SphereMapBlendType = DX_MATERIAL_BLENDTYPE_MODULATE ;
			}
		}
		else
		{
			Material->Type = DX_MATERIAL_TYPE_NORMAL ;
			Material->DiffuseGradBlendType = DX_MATERIAL_BLENDTYPE_MODULATE ;
			Material->DiffuseGradTexture = -1 ;
			Material->SphereMapTexture = -1 ;
			Material->SpecularGradBlendType = DX_MATERIAL_BLENDTYPE_ADDITIVE ;
			Material->SpecularGradTexture = -1 ;
			Material->SphereMapBlendType = DX_MATERIAL_BLENDTYPE_MODULATE ;
			Material->OutLineColor = GetColorF( 0.0f, 0.0f, 0.0f, 1.0f ) ;
			Material->OutLineWidth = 0.0f ;
			Material->OutLineDotWidth = 0.0f ;
		}
	}

	// ライトの情報をセット
	Light = MBase->Light ;
	F1Light = ( MV1_LIGHT_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Light ) ;
	for( i = 0 ; i < MBase->LightNum ; i ++, Light ++, F1Light = ( MV1_LIGHT_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Light->DimNext ) )
	{
		MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1Light->Name,
#ifndef UNICODE
			&Light->NameA,
#endif
			&Light->NameW
		) ;

		Light->Index = F1Light->Index ;
		Light->FrameIndex = F1Light->FrameIndex ;
		Light->Type = F1Light->Type ;
		Light->Diffuse = F1Light->Diffuse ;
		Light->Specular = F1Light->Specular ;
		Light->Ambient = F1Light->Ambient ;
		Light->Range = F1Light->Range ;
		Light->Falloff = F1Light->Falloff ;
		Light->Attenuation0 = F1Light->Attenuation0 ;
		Light->Attenuation1 = F1Light->Attenuation1 ;
		Light->Attenuation2 = F1Light->Attenuation2 ;
		Light->Theta = F1Light->Theta ;
		Light->Phi = F1Light->Phi ;
	}

	// メッシュの情報をセット
	Mesh = MBase->Mesh ;
	F1Mesh = ( MV1_MESH_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Mesh ) ;
	for( i = 0 ; i < MBase->MeshNum ; i ++, Mesh ++, F1Mesh = ( MV1_MESH_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Mesh->DimNext ) )
	{
		if( F1Mesh->Container )
		{
			Mesh->Container = MBase->Frame + ( ( MV1_FRAME_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Mesh->Container ) )->Index ;
		}
		if( F1Mesh->Material )
		{
			Mesh->Material = MBase->Material + ( ( MV1_MATERIAL_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Mesh->Material ) )->Index ;
		}

		Mesh->ChangeInfo.Target   = F1Mesh->ChangeInfo.Target ;
		Mesh->ChangeInfo.Fill     = NULL ;
		Mesh->ChangeInfo.Size     = F1Mesh->ChangeInfo.Size ;
		Mesh->ChangeInfo.CheckBit = F1Mesh->ChangeInfo.CheckBit ;

		Mesh->NotOneDiffuseAlpha = F1Mesh->NotOneDiffuseAlpha ;
		Mesh->Shape = F1Mesh->Shape ;
		Mesh->UseVertexDiffuseColor = F1Mesh->UseVertexDiffuseColor ;
		Mesh->UseVertexSpecularColor = F1Mesh->UseVertexSpecularColor ;

		Mesh->TriangleListNum = F1Mesh->TriangleListNum ;
		if( F1Mesh->TriangleList )
		{
			Mesh->TriangleList = MBase->TriangleList + ( ( MV1_TRIANGLE_LIST_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Mesh->TriangleList ) )->Index ;
		}

		Mesh->Visible = F1Mesh->Visible ;
		Mesh->BackCulling = F1Mesh->BackCulling ;

		Mesh->FaceNum = 0 ;

		// 頂点データを取得する
		{
			MV1_MESH_VERTEX *Vert ;

			Mesh->Vertex = ( MV1_MESH_VERTEX * )( ( BYTE * )MBase->MeshVertex + MBase->MeshVertexSize ) ;
			Mesh->Face   = MBase->MeshFace + MBase->MeshFaceNum ;
			MBase->MeshVertexSize += Mesh->VertexNum * Mesh->VertUnitSize ;
			MBase->MeshFaceNum    += F1Mesh->FaceNum ;

			Src = ( BYTE * )( DWORD_PTR )F1Mesh->VertexData + ( DWORD_PTR )FHeader ;

			// 共有頂点カラーかどうかで処理を分岐
			if( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_COMMON_COLOR )
			{
				COLOR_U8 DiffuseColor, SpecularColor ;

				// 共有頂点カラーを取得する
				DiffuseColor  = ( ( COLOR_U8 * )Src )[ 0 ] ;
				SpecularColor = ( ( COLOR_U8 * )Src )[ 1 ] ;
				Src += 8 ;

				// 全ての頂点のカラーにセットする
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->DiffuseColor  = DiffuseColor ;
					Vert->SpecularColor = SpecularColor ;
				}
			}

			// 座標インデックスを取得する
			switch( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_POS_IND_TYPE_MASK )
			{
			case MV1_MESH_VERT_INDEX_TYPE_U8 :
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->PositionIndex = *Src ;
					Src ++ ;
				}
				break ;

			case MV1_MESH_VERT_INDEX_TYPE_U16 :
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->PositionIndex = *( ( WORD * )Src ) ;
					Src += 2 ;
				}
				break ;

			case MV1_MESH_VERT_INDEX_TYPE_U32 :
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->PositionIndex = *( ( DWORD * )Src ) ;
					Src += 4 ;
				}
				break ;
			}

			// 法線インデックスを取得する
			switch( ( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_NRM_IND_TYPE_MASK ) >> 2 )
			{
			case MV1_MESH_VERT_INDEX_TYPE_NONE :
				break ;

			case MV1_MESH_VERT_INDEX_TYPE_U8 :
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->NormalIndex = *Src ;
					Src ++ ;
				}
				break ;

			case MV1_MESH_VERT_INDEX_TYPE_U16 :
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->NormalIndex = *( ( WORD * )Src ) ;
					Src += 2 ;
				}
				break ;

			case MV1_MESH_VERT_INDEX_TYPE_U32 :
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->NormalIndex = *( ( DWORD * )Src ) ;
					Src += 4 ;
				}
				break ;
			}

			// 頂点カラーを取得する
			if( ( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_COMMON_COLOR ) == 0 )
			{
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					Vert->DiffuseColor  = ( ( COLOR_U8 * )Src )[ 0 ] ;
					Vert->SpecularColor = ( ( COLOR_U8 * )Src )[ 1 ] ;
					Src += 8 ;
				}
			}

			// ＵＶ値を取得する
			if( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_UV_U16 )
			{
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					for( k = 0 ; k < Mesh->UVSetUnitNum ; k ++ )
					{
						Vert->UVs[ k ][ 0 ] = ( ( WORD * )Src )[ 0 ] / 65535.0f ;
						Vert->UVs[ k ][ 1 ] = ( ( WORD * )Src )[ 1 ] / 65535.0f ;
						Src += 4 ;
					}
				}
			}
			else
			{
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					for( k = 0 ; k < Mesh->UVSetUnitNum ; k ++ )
					{
						Vert->UVs[ k ][ 0 ] = ( ( float * )Src )[ 0 ] ;
						Vert->UVs[ k ][ 1 ] = ( ( float * )Src )[ 1 ] ;
						Src += 8 ;
					}
				}
			}

			// トゥーン用の輪郭線を表示するかどうかの情報を取得する
			if( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_NON_TOON_OUTLINE )
			{
				DWORD BitNum ;
				BYTE In ;

				Vert = Mesh->Vertex ;
				BitNum = 0 ;
				In = 0 ;
				for( j = 0 ; j < Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					if( BitNum == 0 )
					{
						In = *Src ;
						Src ++ ;
						BitNum = 8 ;
					}
					if( In & ( 1 << ( 8 - BitNum ) ) )
					{
						Vert->ToonOutLineScale = 0.0f ;
					}
					else
					{
						Vert->ToonOutLineScale = 1.0f ;
					}
					BitNum -- ;
				}
			}
		}
	}

	if( F1FileHeadShape )
	{
		// シェイプの情報をセット
		Shape = MBase->Shape ;
		F1Shape = ( MV1_SHAPE_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1FileHeadShape->Data ) ;
		for( i = 0 ; i < MBase->ShapeNum ; i ++, Shape ++, F1Shape = ( MV1_SHAPE_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Shape->DimNext ) )
		{
			if( F1Shape->Container )
			{
				Shape->Container = MBase->Frame + ( ( MV1_FRAME_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Shape->Container ) )->Index ;
			}

			MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1Shape->Name,
#ifndef UNICODE
				&Shape->NameA,
#endif
				&Shape->NameW
			) ;

			Shape->MeshNum = F1Shape->MeshNum ;

			if( F1Shape->Mesh )
			{
				Shape->Mesh = MBase->ShapeMesh + ( ( MV1_SHAPE_MESH_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Shape->Mesh ) )->Index ;
			}
		}

		// シェイプメッシュの情報をセット
		ShapeMesh = MBase->ShapeMesh ;
		F1ShapeMesh = ( MV1_SHAPE_MESH_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1FileHeadShape->Mesh ) ;
		for( i = 0 ; i < MBase->ShapeMeshNum ; i ++, ShapeMesh ++, F1ShapeMesh = ( MV1_SHAPE_MESH_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1ShapeMesh->DimNext ) )
		{
			if( F1ShapeMesh->TargetMesh )
			{
				ShapeMesh->TargetMesh = MBase->Mesh + ( ( MV1_MESH_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1ShapeMesh->TargetMesh ) )->Index ;
			}

			ShapeMesh->VertexNum = F1ShapeMesh->VertexNum ;

			if( F1ShapeMesh->Vertex )
			{
				ShapeMesh->Vertex = MBase->ShapeVertex + MBase->ShapeVertexNum ;
				MBase->ShapeVertexNum += ShapeMesh->VertexNum ;
				ShapeVertex = ShapeMesh->Vertex ;
				F1ShapeVertex = ( MV1_SHAPE_VERTEX_F1 * )( ( DWORD_PTR )F1ShapeMesh->Vertex + ( DWORD_PTR )FHeader ) ;
				for( j = 0 ; ( DWORD )j < F1ShapeMesh->VertexNum ; j ++, ShapeVertex ++, F1ShapeVertex = ( MV1_SHAPE_VERTEX_F1 * )( ( BYTE * )F1ShapeVertex + F1FileHeadShape->ShapeVertexUnitSize ) )
				{
					ShapeVertex->TargetMeshVertex = F1ShapeVertex->TargetMeshVertex ;
					ShapeVertex->Position = F1ShapeVertex->Position ;
					ShapeVertex->Normal = F1ShapeVertex->Normal ;
				}
			}
		}
	}

	if( F1FileHeadPhysics )
	{
		// 剛体の情報をセット
		PhysicsRigidBody = MBase->PhysicsRigidBody ;
		F1PhysicsRigidBody = ( MV1_PHYSICS_RIGIDBODY_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1FileHeadPhysics->RigidBody ) ;
		for( i = 0 ; i < MBase->PhysicsRigidBodyNum ; i ++, PhysicsRigidBody ++, F1PhysicsRigidBody = ( MV1_PHYSICS_RIGIDBODY_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1PhysicsRigidBody->DimNext ) )
		{
			PhysicsRigidBody->Index = F1PhysicsRigidBody->Index ;
			
			MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1PhysicsRigidBody->Name,
#ifndef UNICODE
				&PhysicsRigidBody->NameA,
#endif
				&PhysicsRigidBody->NameW
			) ;

			if( F1PhysicsRigidBody->TargetFrame )
			{
				PhysicsRigidBody->TargetFrame = MBase->Frame + ( ( MV1_FRAME_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1PhysicsRigidBody->TargetFrame ) )->Index ;
			}

			PhysicsRigidBody->RigidBodyGroupIndex = F1PhysicsRigidBody->RigidBodyGroupIndex ;
			PhysicsRigidBody->RigidBodyGroupTarget = F1PhysicsRigidBody->RigidBodyGroupTarget ;
			PhysicsRigidBody->ShapeType = F1PhysicsRigidBody->ShapeType ;
			PhysicsRigidBody->ShapeW = F1PhysicsRigidBody->ShapeW ;
			PhysicsRigidBody->ShapeH = F1PhysicsRigidBody->ShapeH ;
			PhysicsRigidBody->ShapeD = F1PhysicsRigidBody->ShapeD ;
			PhysicsRigidBody->Position = F1PhysicsRigidBody->Position ;
			PhysicsRigidBody->Rotation = F1PhysicsRigidBody->Rotation ;
			PhysicsRigidBody->RigidBodyWeight = F1PhysicsRigidBody->RigidBodyWeight ;
			PhysicsRigidBody->RigidBodyPosDim = F1PhysicsRigidBody->RigidBodyPosDim ;
			PhysicsRigidBody->RigidBodyRotDim = F1PhysicsRigidBody->RigidBodyRotDim ;
			PhysicsRigidBody->RigidBodyRecoil = F1PhysicsRigidBody->RigidBodyRecoil ;
			PhysicsRigidBody->RigidBodyFriction = F1PhysicsRigidBody->RigidBodyFriction ;
			PhysicsRigidBody->RigidBodyType = F1PhysicsRigidBody->RigidBodyType ;
			PhysicsRigidBody->NoCopyToBone = F1PhysicsRigidBody->NoCopyToBone ;
		}

		// 剛体のジョイント情報をセット
		PhysicsJoint = MBase->PhysicsJoint ;
		F1PhysicsJoint = ( MV1_PHYSICS_JOINT_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )F1FileHeadPhysics->Joint ) ;
		for( i = 0 ; i < MBase->PhysicsJointNum ; i ++, PhysicsJoint ++, F1PhysicsJoint = ( MV1_PHYSICS_JOINT_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1PhysicsJoint->DimNext ) )
		{
			PhysicsJoint->Index = F1PhysicsJoint->Index ;
			
			MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1PhysicsJoint->Name,
#ifndef UNICODE
				&PhysicsJoint->NameA,
#endif
				&PhysicsJoint->NameW
			) ;

			if( F1PhysicsJoint->RigidBodyA )
			{
				PhysicsJoint->RigidBodyA = MBase->PhysicsRigidBody + ( ( MV1_PHYSICS_RIGIDBODY_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1PhysicsJoint->RigidBodyA ) )->Index ;
			}

			if( F1PhysicsJoint->RigidBodyB )
			{
				PhysicsJoint->RigidBodyB = MBase->PhysicsRigidBody + ( ( MV1_PHYSICS_RIGIDBODY_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1PhysicsJoint->RigidBodyB ) )->Index ;
			}

			PhysicsJoint->Position = F1PhysicsJoint->Position ;
			PhysicsJoint->Rotation = F1PhysicsJoint->Rotation ;
			PhysicsJoint->ConstrainPosition1 = F1PhysicsJoint->ConstrainPosition1 ;
			PhysicsJoint->ConstrainPosition2 = F1PhysicsJoint->ConstrainPosition2 ;
			PhysicsJoint->ConstrainRotation1 = F1PhysicsJoint->ConstrainRotation1 ;
			PhysicsJoint->ConstrainRotation2 = F1PhysicsJoint->ConstrainRotation2 ;
			PhysicsJoint->SpringPosition = F1PhysicsJoint->SpringPosition ;
			PhysicsJoint->SpringRotation = F1PhysicsJoint->SpringRotation ;
		}
	}

	// スキニングメッシュ用のボーン情報をセット
	SkinBone = MBase->SkinBone ;
	F1SkinBone = ( MV1_SKIN_BONE_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->SkinBone ) ;
	for( i = 0 ; i < MBase->SkinBoneNum ; i ++, SkinBone ++, F1SkinBone = ( MV1_SKIN_BONE_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1SkinBone->DimNext ) )
	{
		SkinBone->BoneFrame = F1SkinBone->BoneFrame ;
		SkinBone->ModelLocalMatrix = F1SkinBone->ModelLocalMatrix ;
		SkinBone->ModelLocalMatrixIsTranslateOnly = F1SkinBone->ModelLocalMatrixIsTranslateOnly ;
		SkinBone->UseFrameNum = F1SkinBone->UseFrameNum ;
		if( F1SkinBone->UseFrame )
		{
			SkinBone->UseFrame = MBase->SkinBoneUseFrame + ( ( MV1_SKIN_BONE_USE_FRAME_F1 * )( DWORD_PTR )F1SkinBone->UseFrame - ( MV1_SKIN_BONE_USE_FRAME_F1 * )( DWORD_PTR )FHeader->SkinBoneUseFrame ) ;
		}
	}

	// トライアングルリストの情報をセット
	TriangleList = MBase->TriangleList ;
	F1TriangleList = ( MV1_TRIANGLE_LIST_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->TriangleList ) ;
	for( i = 0 ; i < MBase->TriangleListNum ; i ++, TriangleList ++, F1TriangleList = ( MV1_TRIANGLE_LIST_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1TriangleList->DimNext ) )
	{
		if( F1TriangleList->Container )
		{
			TriangleList->Container = MBase->Mesh + ( ( MV1_MESH_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1TriangleList->Container ) )->Index ;
		}
		TriangleList->VertexType = F1TriangleList->VertexType ;
		TriangleList->VertexNum = F1TriangleList->VertexNum ;
		TriangleList->IndexNum = F1TriangleList->IndexNum ;

		// 頂点データの読み込み
		{
			DWORD *MInd ;
			WORD *Ind ;

			TriangleList->Index = MBase->TriangleListIndex + MBase->TriangleListIndexNum ;
			TriangleList->ToonOutLineIndex = TriangleList->Index + FHeader->TriangleListIndexNum ;
			TriangleList->ToonOutLineIndexNum = 0 ;
			TriangleList->MeshVertexIndex   = MBase->MeshVertexIndex   + MBase->MeshVertexIndexNum ;
			MBase->TriangleListIndexNum += TriangleList->IndexNum ;
			MBase->MeshVertexIndexNum   += TriangleList->VertexNum ;

			Src = ( BYTE * )( DWORD_PTR )F1TriangleList->MeshVertexIndexAndIndexData + ( DWORD_PTR )FHeader ;

			// ボーンの情報を取得
			switch( TriangleList->VertexType )
			{
			case MV1_VERTEX_TYPE_SKIN_4BONE :
			case MV1_VERTEX_TYPE_SKIN_8BONE :
				TriangleList->UseBoneNum = *( ( WORD * )Src ) ;
				Src += 2 ;
				for( j = 0 ; j < TriangleList->UseBoneNum ; j ++ )
				{
					TriangleList->UseBone[ j ] = *( ( WORD * )Src ) ;
					Src += 2 ;
				}
				break ;

			case MV1_VERTEX_TYPE_SKIN_FREEBONE :
				TriangleList->MaxBoneNum = *( ( WORD * )Src ) ;
				Src += 2 ;
				break ;
			}

			// メッシュ頂点インデックスを取得する
			MInd = TriangleList->MeshVertexIndex ;
			switch( F1TriangleList->Flag & MV1_TRIANGLE_LIST_FLAG_MVERT_INDEX_MASK )
			{
			case MV1_TRIANGLE_LIST_INDEX_TYPE_U8 :
				for( j = 0 ; j < TriangleList->VertexNum ; j ++, MInd ++ )
				{
					*MInd = *Src ;
					Src ++ ;
				}
				break ;

			case MV1_TRIANGLE_LIST_INDEX_TYPE_U16 :
				for( j = 0 ; j < TriangleList->VertexNum ; j ++, MInd ++ )
				{
					*MInd = *( ( WORD * )Src ) ;
					Src += 2 ;
				}
				break ;

			case MV1_TRIANGLE_LIST_INDEX_TYPE_U32 :
				for( j = 0 ; j < TriangleList->VertexNum ; j ++, MInd ++ )
				{
					*MInd = *( ( DWORD * )Src ) ;
					Src += 4 ;
				}
				break ;
			}

			// 高速アクセス用の頂点座標データをセット
			switch( TriangleList->VertexType )
			{
			case MV1_VERTEX_TYPE_NORMAL :
				TriangleList->NormalPosition = MBase->TriangleListNormalPosition + MBase->TriangleListNormalPositionNum ;
				TriangleList->NormalPosition = ( MV1_TLIST_NORMAL_POS * )( ( ( DWORD_PTR )TriangleList->NormalPosition + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = sizeof( MV1_TLIST_NORMAL_POS ) ;
				MBase->TriangleListNormalPositionNum += TriangleList->VertexNum ;
				break ;

			case MV1_VERTEX_TYPE_SKIN_4BONE :
				TriangleList->SkinPosition4B = MBase->TriangleListSkinPosition4B + MBase->TriangleListSkinPosition4BNum ;
				TriangleList->SkinPosition4B = ( MV1_TLIST_SKIN_POS_4B * )( ( ( DWORD_PTR )TriangleList->SkinPosition4B + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = sizeof( MV1_TLIST_SKIN_POS_4B ) ;
				MBase->TriangleListSkinPosition4BNum += TriangleList->VertexNum ;
				break ;

			case MV1_VERTEX_TYPE_SKIN_8BONE :
				TriangleList->SkinPosition8B = MBase->TriangleListSkinPosition8B + MBase->TriangleListSkinPosition8BNum ;
				TriangleList->SkinPosition8B = ( MV1_TLIST_SKIN_POS_8B * )( ( ( DWORD_PTR )TriangleList->SkinPosition8B + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = sizeof( MV1_TLIST_SKIN_POS_8B ) ;
				MBase->TriangleListSkinPosition8BNum += TriangleList->VertexNum ;
				break ;

			case MV1_VERTEX_TYPE_SKIN_FREEBONE :
				TriangleList->SkinPositionFREEB = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )MBase->TriangleListSkinPositionFREEB + MBase->TriangleListSkinPositionFREEBSize ) ;
				TriangleList->SkinPositionFREEB = ( MV1_TLIST_SKIN_POS_FREEB * )( ( ( DWORD_PTR )TriangleList->SkinPositionFREEB + 15 ) / 16 * 16 ) ;
				TriangleList->PosUnitSize = ( unsigned short )( sizeof( MV1_TLIST_SKIN_POS_FREEB ) + sizeof( MV1_SKINBONE_BLEND ) * ( TriangleList->MaxBoneNum - 4 ) ) ;
				TriangleList->PosUnitSize = ( unsigned short )( ( TriangleList->PosUnitSize + 15 ) / 16 * 16 ) ;
				MBase->TriangleListSkinPositionFREEBSize += TriangleList->PosUnitSize * TriangleList->VertexNum ;
				break ;
			}
//			MV1SetupTriangleListPositionAndNormal( TriangleList ) ;

			// 頂点インデックスを取得する
			Ind = TriangleList->Index ;
			switch( ( F1TriangleList->Flag & MV1_TRIANGLE_LIST_FLAG_INDEX_MASK ) >> 2 )
			{
			case MV1_TRIANGLE_LIST_INDEX_TYPE_U8 :
				for( j = 0 ; j < TriangleList->IndexNum ; j ++, Ind ++ )
				{
					*Ind = *Src ;
					Src ++ ;
				}
				break ;

			case MV1_TRIANGLE_LIST_INDEX_TYPE_U16 :
				for( j = 0 ; j < TriangleList->IndexNum ; j ++, Ind ++ )
				{
					*Ind = *( ( WORD * )Src ) ;
					Src += 2 ;
				}
				break ;
			}
		}
	}

	// メッシュのフェイス情報を構築する
	{
		WORD *Ind ;
		DWORD *MInd, TListInd ;

		TriangleList = MBase->TriangleList ;
		for( i = 0 ; i < MBase->TriangleListNum ; i ++, TriangleList ++ )
		{
			Mesh = TriangleList->Container ;
			Ind = TriangleList->Index ;
			MInd = TriangleList->MeshVertexIndex ;
			TListInd = ( DWORD )( TriangleList - Mesh->TriangleList ) ;
			for( j = 0 ; j < TriangleList->IndexNum ; j += 3, Ind += 3 )
			{
				Face = &Mesh->Face[ Mesh->FaceNum ] ;
				Face->TriangleListIndex = ( WORD )TListInd ;
				Face->VertexType = TriangleList->VertexType ;
				Face->VertexIndex[ 0 ] = MInd[ Ind[ 0 ] ] ;
				Face->VertexIndex[ 1 ] = MInd[ Ind[ 1 ] ] ;
				Face->VertexIndex[ 2 ] = MInd[ Ind[ 2 ] ] ;
				Mesh->FaceNum ++ ;
			}
		}
	}

	// 法線の再生成の指定がある場合は生成を行う
	if( MV1Man.LoadModelToReMakeNormal || AutoNormalCreateFlag )
	{
		// 法線の自動生成
		{
			VECTOR Nrm ;
			MV1_MESH_NORMAL *MNrm ;
			MV1_MESH_VERTEX *Vertex[ 3 ] ;
			DWORD VertUnitSize, PosUnitSize ;

			// 全てのフレームを処理
			Frame = MBase->Frame ;
			for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++ )
			{
				if( Frame->AutoCreateNormal == 0 ) continue ;

				PosUnitSize = ( DWORD )Frame->PosUnitSize ;

				// 面の法線を算出しながら足していく
				Mesh = Frame->Mesh ;
				for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
				{
					VertUnitSize = ( DWORD )Mesh->VertUnitSize ;

					Face = Mesh->Face ;
					for( k = 0 ; k < Mesh->FaceNum ; k ++, Face ++ )
					{
						Vertex[ 0 ] = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + VertUnitSize * Face->VertexIndex[ 0 ] ) ;
						Vertex[ 1 ] = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + VertUnitSize * Face->VertexIndex[ 1 ] ) ;
						Vertex[ 2 ] = ( MV1_MESH_VERTEX * )( ( BYTE * )Mesh->Vertex + VertUnitSize * Face->VertexIndex[ 2 ] ) ;

						Nrm = VNorm( VCross( 
							VSub( ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 1 ]->PositionIndex ) )->Position, 
								  ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 0 ]->PositionIndex ) )->Position ),
							VSub( ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 2 ]->PositionIndex ) )->Position, 
								  ( ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + PosUnitSize * Vertex[ 0 ]->PositionIndex ) )->Position ) ) ) ;

						MNrm = &Frame->Normal[ Vertex[ 0 ]->NormalIndex ] ;
						MNrm->Normal = VAdd( MNrm->Normal, Nrm ) ;

						MNrm = &Frame->Normal[ Vertex[ 1 ]->NormalIndex ] ;
						MNrm->Normal = VAdd( MNrm->Normal, Nrm ) ;

						MNrm = &Frame->Normal[ Vertex[ 2 ]->NormalIndex ] ;
						MNrm->Normal = VAdd( MNrm->Normal, Nrm ) ;
					}
				}

				// 法線を正規化する
				MNrm = Frame->Normal ;
				for( j = 0 ; j < Frame->NormalNum ; j ++, MNrm ++ )
				{
					MNrm->Normal = VNorm( MNrm->Normal ) ;
				}
			}
		}

		// 接線と従法線の構築
		Mesh = MBase->Mesh ;
		for( i = 0 ; i < MBase->MeshNum ; i ++, Mesh ++ )
		{
			MV1MakeMeshBinormalsAndTangents( Mesh ) ;
		}
	}

	// アニメーションキーセットの情報をセット
	AnimKeySet = MBase->AnimKeySet ;
	F1AnimKeySet = ( MV1_ANIM_KEYSET_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->AnimKeySet ) ;
	for( i = 0 ; i < MBase->AnimKeySetNum ; i ++, AnimKeySet ++, F1AnimKeySet = ( MV1_ANIM_KEYSET_F1 * )( ( DWORD_PTR )F1AnimKeySet + FHeader->AnimKeySetUnitSize ) )
	{
		MV1_ANIM_KEY_16BIT_F KeySubB16F ;
		float *Float ;
		VECTOR *Vector ;
		FLOAT4 *Quaternion ;
		BYTE *Src ;

		AnimKeySet->Type = F1AnimKeySet->Type ;
		AnimKeySet->DataType = F1AnimKeySet->DataType ;

		Src = ( BYTE * )FHeader + ( DWORD_PTR )F1AnimKeySet->KeyData ;

		if( AnimKeySet->DataType == MV1_ANIMKEY_DATATYPE_SHAPE )
		{
			AnimKeySet->TargetShapeIndex = *( ( WORD * )Src ) ;
			Src += 2 ;
		}

		if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_ONE )
		{
			AnimKeySet->TimeType = MV1_ANIMKEY_TIME_TYPE_ONE ;
			AnimKeySet->UnitTime = 1.0f ;
			AnimKeySet->StartTime = 0.0f ;
			AnimKeySet->Num = 1 ;
		}
		else
		{
			if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEYNUM_B )
			{
				AnimKeySet->Num = ( int )*Src ;
				Src ++ ;
			}
			else
			if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEYNUM_W )
			{
				AnimKeySet->Num = ( int )*( ( WORD * )Src ) ;
				Src += 2 ;
			}
			else
			{
				AnimKeySet->Num = ( int )*( ( DWORD * )Src ) ;
				Src += 4 ;
			}

			if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_TIME_UNIT )
			{
				AnimKeySet->TimeType = MV1_ANIMKEY_TIME_TYPE_ONE ;

				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_TIME_UNIT_ST_W )
				{
					AnimKeySet->StartTime = *( ( WORD * )Src ) ;
					Src += 2 ;
				}
				else
				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_TIME_UNIT_ST_Z )
				{
					AnimKeySet->StartTime = 0.0f ;
				}
				else
				{
					AnimKeySet->StartTime = *( ( float * )Src ) ;
					Src += 4 ;
				}

				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_TIME_UNIT_UN_W )
				{
					AnimKeySet->UnitTime = *( ( WORD * )Src ) ;
					Src += 2 ;
				}
				else
				{
					AnimKeySet->UnitTime = *( ( float * )Src ) ;
					Src += 4 ;
				}
			}
			else
			{
				AnimKeySet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;

				AnimKeySet->KeyTime = ( float * )( ( DWORD_PTR )MBase->AnimKeyData + MBase->AnimKeyDataSize ) ;
				MBase->AnimKeyDataSize += sizeof( float ) * AnimKeySet->Num ;
				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_TIME_BIT16 )
				{
					KeySubB16F.Min  = MV1AnimKey16BitMinBtoF(  ( ( MV1_ANIM_KEY_16BIT_F1 * )Src )->Min  ) ;
					KeySubB16F.Unit = MV1AnimKey16BitUnitBtoF( ( ( MV1_ANIM_KEY_16BIT_F1 * )Src )->Unit ) ;
					Src += 2 ;

					Float = AnimKeySet->KeyTime ;
					for( j = 0 ; j < AnimKeySet->Num ; j ++, Float ++ )
					{
						*Float = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
						Src += 2 ;
					}
				}
				else
				{
					_MEMCPY( AnimKeySet->KeyTime, Src, sizeof( float ) * AnimKeySet->Num ) ;
					Src += sizeof( float ) * AnimKeySet->Num ;
				}
			}
		}

		AnimKeySet->KeyLinear = ( float * )( ( DWORD_PTR )MBase->AnimKeyData + MBase->AnimKeyDataSize ) ;
		if( ( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_BIT16 ) == 0 )
		{
			switch( AnimKeySet->Type )
			{
			case MV1_ANIMKEY_TYPE_LINEAR :
				_MEMCPY( AnimKeySet->KeyLinear, Src, AnimKeySet->Num * sizeof( float ) ) ;
				Src += AnimKeySet->Num * sizeof( float ) ;
				MBase->AnimKeyDataSize += sizeof( float ) * AnimKeySet->Num ;
				break ;

			case MV1_ANIMKEY_TYPE_VECTOR :
				_MEMCPY( AnimKeySet->KeyVector, Src, AnimKeySet->Num * sizeof( VECTOR ) ) ;
				Src += AnimKeySet->Num * sizeof( VECTOR ) ;
				MBase->AnimKeyDataSize += sizeof( VECTOR ) * AnimKeySet->Num ;
				break ;

			case MV1_ANIMKEY_TYPE_QUATERNION_X :
			case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
				_MEMCPY( AnimKeySet->KeyFloat4, Src, AnimKeySet->Num * sizeof( FLOAT4 ) ) ;
				Src += AnimKeySet->Num * sizeof( FLOAT4 ) ;
				MBase->AnimKeyDataSize += sizeof( FLOAT4 ) * AnimKeySet->Num ;
				break ;

			case MV1_ANIMKEY_TYPE_MATRIX4X4C :
				_MEMCPY( AnimKeySet->KeyMatrix4x4C, Src, AnimKeySet->Num * sizeof( MATRIX_4X4CT_F ) ) ;
				Src += AnimKeySet->Num * sizeof( MATRIX_4X4CT_F ) ;
				MBase->AnimKeyDataSize += sizeof( MATRIX_4X4CT_F ) * AnimKeySet->Num ;
				break ;
			}
		}
		else
		{
			if( ( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_MP_PP ) == 0 &&
				( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_Z_TP ) == 0 )
			{
				KeySubB16F.Min  = MV1AnimKey16BitMinBtoF(  ( ( MV1_ANIM_KEY_16BIT_F1 * )Src )->Min  ) ;
				KeySubB16F.Unit = MV1AnimKey16BitUnitBtoF( ( ( MV1_ANIM_KEY_16BIT_F1 * )Src )->Unit ) ;
				Src += 2 ;
			}

			switch( AnimKeySet->Type )
			{
			case MV1_ANIMKEY_TYPE_LINEAR :
				Float = AnimKeySet->KeyLinear ;
				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_MP_PP )
				{
					for( k = 0 ; k < AnimKeySet->Num ; k ++, Float ++ )
					{
						*Float  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f - ( float )DX_PI ;
						Src += 2 ;
					}
				}
				else
				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_Z_TP )
				{
					for( k = 0 ; k < AnimKeySet->Num ; k ++, Float ++ )
					{
						*Float  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f ;
						Src += 2 ;
					}
				}
				else
				{
					for( k = 0 ; k < AnimKeySet->Num ; k ++, Float ++ )
					{
						*Float  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
						Src += 2 ;
					}
				}
				MBase->AnimKeyDataSize += sizeof( float ) * AnimKeySet->Num ;
				break ;

			case MV1_ANIMKEY_TYPE_VECTOR :
				Vector = AnimKeySet->KeyVector ;
				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_MP_PP )
				{
					for( k = 0 ; k < AnimKeySet->Num ; k ++, Vector ++ )
					{
						Vector->x  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f - ( float )DX_PI ;
						Src += 2 ;
						Vector->y  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f - ( float )DX_PI ;
						Src += 2 ;
						Vector->z  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f - ( float )DX_PI ;
						Src += 2 ;
					}
				}
				else
				if( F1AnimKeySet->Flag & MV1_ANIM_KEYSET_FLAG_KEY_Z_TP )
				{
					for( k = 0 ; k < AnimKeySet->Num ; k ++, Vector ++ )
					{
						Vector->x  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f ;
						Src += 2 ;
						Vector->y  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f ;
						Src += 2 ;
						Vector->z  = *( ( WORD * )Src ) * ( float )( DX_PI * 2.0f ) / 65535.0f ;
						Src += 2 ;
					}
				}
				else
				{
					for( k = 0 ; k < AnimKeySet->Num ; k ++, Vector ++ )
					{
						Vector->x  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
						Src += 2 ;
						Vector->y  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
						Src += 2 ;
						Vector->z  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
						Src += 2 ;
					}
				}
				MBase->AnimKeyDataSize += sizeof( VECTOR ) * AnimKeySet->Num ;
				break ;

			case MV1_ANIMKEY_TYPE_QUATERNION_X :
			case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
				Quaternion = AnimKeySet->KeyFloat4 ;
				for( k = 0 ; k < AnimKeySet->Num ; k ++, Quaternion ++ )
				{
					Quaternion->x  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
					Src += 2 ;
					Quaternion->y  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
					Src += 2 ;
					Quaternion->z  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
					Src += 2 ;
					Quaternion->w  = *( ( WORD * )Src ) * KeySubB16F.Unit + KeySubB16F.Min ;
					Src += 2 ;
				}
				MBase->AnimKeyDataSize += sizeof( FLOAT4 ) * AnimKeySet->Num ;
				break ;
			}
		}

		if( LoadParam->GParam.LoadModelToIgnoreScaling &&
			( AnimKeySet->DataType == MV1_ANIMKEY_DATATYPE_SCALE ||
			  AnimKeySet->DataType == MV1_ANIMKEY_DATATYPE_SCALE_X ||
			  AnimKeySet->DataType == MV1_ANIMKEY_DATATYPE_SCALE_Y ||
			  AnimKeySet->DataType == MV1_ANIMKEY_DATATYPE_SCALE_Z  ) )
		{
			AnimKeySet->Num = 0 ;
		}
	}

	// アニメーションセットの情報をセット
	AnimSet = MBase->AnimSet ;
	F1AnimSet = ( MV1_ANIMSET_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->AnimSet ) ;
	for( i = 0 ; i < MBase->AnimSetNum ; i ++, AnimSet ++, F1AnimSet = ( MV1_ANIMSET_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1AnimSet->DimNext ) )
	{
		MV1LoadModelToMV1_GetString( FHeader, MBase, FileStringBuffer, F1AnimSet->Name,
#ifndef UNICODE
			&AnimSet->NameA,
#endif
			&AnimSet->NameW
		) ;

		AnimSet->Index = F1AnimSet->Index ;
		AnimSet->MaxTime = F1AnimSet->MaxTime ;
		AnimSet->AnimNum = F1AnimSet->AnimNum ;
		if( F1AnimSet->Anim )
		{
			AnimSet->Anim = MBase->Anim + ( ( MV1_ANIM_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1AnimSet->Anim ) )->Index ;
		}
		if( F1AnimSet->Flag & 1 ) AnimSet->IsAddAnim = 1 ;
		if( F1AnimSet->Flag & 2 ) AnimSet->IsMatrixLinearBlend = 1 ;
	}

	// アニメーションの情報をセット
	Anim = MBase->Anim ;
	F1Anim = ( MV1_ANIM_F1 * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->Anim ) ;
	for( i = 0 ; i < MBase->AnimNum ; i ++, Anim ++, F1Anim = ( MV1_ANIM_F1 * )( ( BYTE * )F1Anim + FHeader->AnimUnitSize ) )
	{
		if( F1Anim->Container )
		{
			Anim->Container = MBase->AnimSet + ( ( MV1_ANIMSET_F1 * )( ( DWORD_PTR )FHeader + ( DWORD_PTR )F1Anim->Container ) )->Index ;
		}

		Anim->TargetFrame = MBase->Frame + F1Anim->TargetFrameIndex ;
		Anim->TargetFrameIndex = F1Anim->TargetFrameIndex ;
		Anim->MaxTime = F1Anim->MaxTime ;
		Anim->RotateOrder = F1Anim->RotateOrder ;
		Anim->KeySetNum = F1Anim->KeySetNum ;
		Anim->KeySet = MBase->AnimKeySet + ( ( MV1_ANIM_KEYSET_F1 * )( DWORD_PTR )F1Anim->KeySet - ( MV1_ANIM_KEYSET_F1 * )( DWORD_PTR )FHeader->AnimKeySet ) ;

		MBase->AnimTargetFrameTable[ Anim->Container->Index * MBase->FrameNum + Anim->TargetFrameIndex ] = Anim ;
	}

	// 行列のセットアップ
	MV1SetupInitializeMatrixBase( MBase ) ;

	// メッシュの半透明かどうかの情報をセットアップする
	MV1SetupMeshSemiTransStateBase( MBase ) ;

	// 同時複数描画関係の情報をセットアップする
	if( MBase->UsePackDraw )
	{
		MV1SetupPackDrawInfo( MBase ) ;
	}

	// メモリの解放
	DXFREE( FHeader ) ;

	// 指定がある場合は座標の最適化を行う
	if( LoadParam->GParam.LoadModelToPositionOptimize )
	{
		MV1PositionOptimizeBase( NewHandle ) ;
	}

	// 指定がある場合は法線の再計算を行う
	if( LoadParam->GParam.LoadModelToReMakeNormal )
	{
		MV1ReMakeNormalBase( NewHandle, LoadParam->GParam.LoadModelToReMakeNormalSmoothingAngle, ASyncThread ) ;
	}

	// 高速処理用頂点データの構築
	TriangleList = MBase->TriangleList ;
	for( i = 0 ; i < MBase->TriangleListNum ; i ++, TriangleList ++ )
	{
		MV1SetupTriangleListPositionAndNormal( TriangleList ) ;
		MV1SetupToonOutLineTriangleList( TriangleList ) ;
	}

#ifndef DX_NON_ASYNCLOAD
	if( ASyncThread )
	{
		DecASyncLoadCount( NewHandle ) ;
	}
#endif // DX_NON_ASYNCLOAD

	// 正常終了
	return NewHandle ;

	// エラー処理
ERRORLABEL :
	if( NewHandle )
	{
#ifndef DX_NON_ASYNCLOAD
		if( ASyncThread )
		{
			DecASyncLoadCount( NewHandle ) ;
		}
#endif // DX_NON_ASYNCLOAD

		MV1SubModelBase( NewHandle ) ;
		NewHandle = 0 ;
	}

	// メモリの解放
	DXFREE( FHeader ) ;

	// エラー終了
	return -1 ;
}

// 頂点バッファのセットアップをする( -1:エラー )
extern int MV1SetupVertexBufferAll( int ASyncThread )
{
	int i ;
	MV1_MODEL_BASE *MBase ;

	if( HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].InitializeFlag == FALSE )
		return -1 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 有効なすべてのモデル基データの頂点バッファをセットアップする
	for( i = HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].AreaMax ; i ++ )
	{
		MBase = ( MV1_MODEL_BASE * )HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].Handle[ i ] ;
		if( MBase == NULL ) continue ;

		MV1_SetupVertexBufferBase_PF( MBase->HandleInfo.Handle, 1, ASyncThread ) ;
	}

	// 終了
	return 0 ;
}

// 全ての頂点バッファの後始末をする( -1:エラー )
extern int MV1TerminateVertexBufferAll( void )
{
	int i ;
	MV1_MODEL_BASE *MBase ;
	MV1_MODEL *Model ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	if( HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].InitializeFlag )
	{
		// 有効なすべてのモデル基データの頂点バッファを解放する
		for( i = HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].AreaMax ; i ++ )
		{
			MBase = ( MV1_MODEL_BASE * )HandleManageArray[ DX_HANDLETYPE_MODEL_BASE ].Handle[ i ] ;
			if( MBase == NULL ) continue ;

#ifndef DX_NON_ASYNCLOAD
			WaitASyncLoad( MBase->HandleInfo.Handle ) ;
#endif

			MV1_TerminateVertexBufferBase_PF( MBase->HandleInfo.Handle ) ;
		}
	}

	if( HandleManageArray[ DX_HANDLETYPE_MODEL ].InitializeFlag )
	{
		// 有効な全てのモデルデータのシェイプ頂点バッファを開放する
		for( i = HandleManageArray[ DX_HANDLETYPE_MODEL ].AreaMin ; i <= HandleManageArray[ DX_HANDLETYPE_MODEL ].AreaMax ; i ++ )
		{
			Model = ( MV1_MODEL * )HandleManageArray[ DX_HANDLETYPE_MODEL ].Handle[ i ] ;
			if( Model == NULL ) continue ;

#ifndef DX_NON_ASYNCLOAD
			WaitASyncLoad( Model->HandleInfo.Handle ) ;
#endif

			MV1_TerminateVertexBuffer_PF( Model->HandleInfo.Handle ) ;
		}
	}

	// 終了
	return 0 ;
}

// ロードしたモデルのファイル名を取得する
extern	const wchar_t *MV1GetModelFileName( int MHandle )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NULL ;

	// アドレスを返す
	return Model->BaseData->FilePath ;
}

// ロードしたモデルが存在するディレクトリパスを取得する( 末端に / か \ が付いています )
extern	const wchar_t *MV1GetModelDirectoryPath( int MHandle )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NULL ;

	// アドレスを返す
	return Model->BaseData->DirectoryPath ;
}

// 指定のモデルと同じモデル基本データを使用してモデルを作成する( -1:エラー  0以上:モデルハンドル )
extern int NS_MV1DuplicateModel( int SrcMHandle )
{
	MV1_MODEL *Model ;
	int NewHandle ;
	int Result ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( SrcMHandle, Model ) )
		return -1 ;

	// モデルデータを作成する
	NewHandle = MV1AddModel( FALSE ) ;
	if( NewHandle == -1 )
	{
		return -1 ;
	}

	// 構築
	Result = MV1MakeModel( NewHandle, Model->BaseDataHandle ) ;
	if( Result < 0 )
	{
		MV1SubModel( NewHandle ) ;
		return -1 ;
	}

	// 正常終了
	return NewHandle ;
}

// 指定のモデルと全く同じ情報を持つ別のﾓﾃﾞﾙデータハンドルを作成する( -1:エラー  0以上:モデルハンドル )
extern int NS_MV1CreateCloneModel( int SrcMHandle )
{
	MV1_MODEL *Model ;
	int NewHandle = -1 ;
	int NewBaseHandle = -1 ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( SrcMHandle, Model ) )
		return -1 ;

	// 基本データを複製する
	NewBaseHandle = MV1CreateCloneModelBase( Model->BaseDataHandle ) ;
	if( NewBaseHandle == -1 )
	{
		goto ERR ;
	}

	// 頂点バッファのセットアップを行う
	MV1_SetupVertexBufferBase_PF( NewBaseHandle ) ;

	// モデルデータを作成する
	NewHandle = MV1AddModel( FALSE ) ;
	if( NewHandle == -1 )
	{
		goto ERR;
	}

	// 構築
	if( MV1MakeModel( NewHandle, NewBaseHandle ) < 0 )
	{
		goto ERR ;
	}

	// 正常終了
	return NewHandle ;

	// エラー
ERR:
	if( NewHandle != -1 )
	{
		MV1SubModel( NewHandle ) ;
		NewHandle = -1 ;
	}

	if( NewBaseHandle != -1 )
	{
		MV1SubModelBase( NewBaseHandle ) ;
		NewBaseHandle = -1 ;
	}

	// エラー終了
	return -1 ;
}


// モデルを読み込む際に法線の再計算を行うかどうかを設定する( TRUE:行う  FALSE:行わない )
extern int NS_MV1SetLoadModelReMakeNormal( int Flag )
{
	MV1Man.LoadModelToReMakeNormal = Flag ;

	// 正常終了
	return 0 ;
}

// モデルを読み込む際に行う法泉の再計算で使用するスムージング角度を設定する
extern int NS_MV1SetLoadModelReMakeNormalSmoothingAngle( float SmoothingAngle )
{
	MV1Man.LoadModelToReMakeNormalSmoothingAngle = SmoothingAngle ;

	// 正常終了
	return 0 ;
}

// モデルを読み込む際にスケーリングデータを無視するかどうかを設定する( TRUE:無視する  FALSE:無視しない( デフォルト ) )
extern int NS_MV1SetLoadModelIgnoreScaling( int Flag )
{
	MV1Man.LoadModelToIgnoreScaling = Flag ;

	// 正常終了
	return 0 ;
}

// モデルを読み込む際に座標データの最適化を行うかどうかを設定する( TRUE:行う  FALSE:行わない )
extern int NS_MV1SetLoadModelPositionOptimize( int Flag )
{
	MV1Man.LoadModelToPositionOptimize = Flag ;

	// 正常終了
	return 0 ;
}

// 読み込むモデルの物理演算モードを設定する
extern int NS_MV1SetLoadModelUsePhysicsMode( int PhysicsMode /* DX_LOADMODEL_PHYSICS_LOADCALC 等 */ )
{
	MV1Man.LoadModelToUsePhysicsMode = PhysicsMode ;

	// 正常終了
	return 0 ;
}

// 読み込むモデルの物理演算に適用する重力パラメータ
extern int NS_MV1SetLoadModelPhysicsWorldGravity( float Gravity )
{
	MV1Man.LoadModelToWorldGravity = Gravity ;
	MV1Man.LoadModelToWorldGravityInitialize = TRUE ;

	// 正常終了
	return 0 ;
}

// 読み込むモデルの物理演算モードが事前計算( DX_LOADMODEL_PHYSICS_LOADCALC )だった場合に適用される物理演算の時間進行の精度を設定する( 0:60FPS  1:120FPS  2:240FPS  3:480FPS  4:960FPS  5:1920FPS )
extern int NS_MV1SetLoadModelPhysicsCalcPrecision( int Precision )
{
	if( Precision < 0 )
	{
		return -1 ;
	}

	// 一応 1920FPS以上は指定できないようにする( 4:960FPS  5:1920FPS )
	if( Precision > 5 )
	{
		Precision = 5 ;
	}

	MV1Man.LoadModelToPhysicsCalcPrecision = Precision ;

	// 正常終了
	return 0 ;
}

// 読み込むモデルの物理演算モードが事前計算( DX_LOADMODEL_PHYSICS_LOADCALC )だった場合に適用される重力の設定をする
extern int NS_MV1SetLoadCalcPhysicsWorldGravity( int GravityNo, VECTOR Gravity )
{
	if( GravityNo < 0 || GravityNo > 255 )
		return -1 ;

	MV1Man.LoadCalcPhysicsWorldGravity[ GravityNo ] = Gravity ;

	// 正常終了
	return 0 ;
}

// 読み込むモデルに適用するアニメーションファイルのパスを設定する、NULLを渡すと設定リセット( 現在は PMD,PMX のみに効果あり )
extern int NS_MV1SetLoadModelAnimFilePath( const TCHAR *FileName )
{
#ifdef UNICODE
	return MV1SetLoadModelAnimFilePath_WCHAR_T(
		FileName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = MV1SetLoadModelAnimFilePath_WCHAR_T(
		UseFileNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// 読み込むモデルに適用するアニメーションファイルのパスを設定する、NULLを渡すと設定リセット( 現在は PMD,PMX のみに効果あり )
extern int MV1SetLoadModelAnimFilePath_WCHAR_T( const wchar_t *FileName )
{
	if( FileName == NULL )
	{
		MV1Man.AnimFilePathValid = FALSE ;
	}
	else
	{
		MV1Man.AnimFilePathValid = TRUE ;

		// ファイルのディレクトリパスとファイル名を取得する
		_MV1CreateFileNameAndCurrentDirectory( FileName, MV1Man.AnimFileName, MV1Man.AnimFileDirPath ) ;
	}

	// 終了
	return 0 ;
}

// 読み込むモデルを同時複数描画に対応させるかどうかを設定する( TRUE:対応させる  FALSE:対応させない( デフォルト ) )、( 「対応させる」にすると描画が高速になる可能性がある代わりに消費VRAMが増えます )
extern int NS_MV1SetLoadModelUsePackDraw( int Flag )
{
	MV1Man.LoadModelToUsePackDraw = Flag ;

	// 終了
	return 0 ;
}



// 指定のパスにモデルを保存する
extern int NS_MV1SaveModelToMV1File(
	int MHandle,
	const TCHAR *FileName,
	int SaveType,
	int AnimMHandle,
	int AnimNameCheck,
	int Normal8BitFlag,
	int Position16BitFlag,
	int Weight8BitFlag,
	int Anim16BitFlag
)
{
#ifdef UNICODE
	return MV1SaveModelToMV1File_WCHAR_T(
		MHandle,
		FileName,
		SaveType,
		AnimMHandle,
		AnimNameCheck,
		Normal8BitFlag,
		Position16BitFlag,
		Weight8BitFlag,
		Anim16BitFlag
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = MV1SaveModelToMV1File_WCHAR_T(
		MHandle,
		UseFileNameBuffer,
		SaveType,
		AnimMHandle,
		AnimNameCheck,
		Normal8BitFlag,
		Position16BitFlag,
		Weight8BitFlag,
		Anim16BitFlag
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// 指定のパスにモデルを保存する
extern int MV1SaveModelToMV1File_WCHAR_T(
	int MHandle,
	const wchar_t *FileName,
	int SaveType,
	int AnimMHandle,
	int AnimNameCheck,
	int Normal8BitFlag,
	int Position16BitFlag,
	int Weight8BitFlag,
	int Anim16BitFlag )
{
	MV1_MODEL					*Model, *AnimModel ;
	MV1_MODEL_BASE				*ModelBase, *AnimModelBase ;
	MV1_FRAME_BASE				*Frame ;
	MV1_FRAME_F1				*F1Frame ;
	MV1_MATERIAL_BASE			*MaterialBase ;
	MV1_MATERIAL				*Material ;
	MV1_MATERIAL_F1				*F1Material ;
	MV1_MATERIAL_LAYER			*MaterialLayer ;
	MV1_MATERIAL_LAYER_F1		*F1MaterialLayer ;
	MV1_MATERIAL_TOON_F1		*F1MaterialToonAddr = NULL, *F1MaterialToon ;
	MV1_LIGHT					*Light ;
	MV1_LIGHT_F1				*F1Light ;
	MV1_TEXTURE					*Texture ;
	MV1_TEXTURE_BASE			*TextureBase ;
	MV1_TEXTURE_F1				*F1Texture ;
	MV1_MESH_BASE				*Mesh ;
	MV1_MESH_F1					*F1Mesh ;
	MV1_SKIN_BONE				*SkinBone ;
	MV1_SKIN_BONE_F1			*F1SkinBone ;
	MV1_TRIANGLE_LIST_BASE		*TriangleList ;
	MV1_TRIANGLE_LIST_F1		*F1TriangleList ;
	MV1_ANIM_KEYSET_BASE		*AnimKeySet ;
	MV1_ANIM_KEYSET_F1			*F1AnimKeySet ;
	MV1_ANIM_BASE				*Anim ;
	MV1_ANIM_F1					*F1Anim ;
	MV1_ANIMSET_BASE			*AnimSet ;
	MV1_ANIMSET_F1				*F1AnimSet ;
	MV1_FILEHEAD_SHAPE_F1		*F1FileHeadShape ;
	MV1_FRAME_SHAPE_F1			*F1FrameShape ;
	MV1_SHAPE_BASE				*Shape ;
	MV1_SHAPE_F1				*F1Shape ;
	MV1_SHAPE_MESH_BASE			*ShapeMesh ;
	MV1_SHAPE_MESH_F1			*F1ShapeMesh ;
	MV1_SHAPE_VERTEX_BASE		*ShapeVertex ;
	MV1_SHAPE_VERTEX_F1			*F1ShapeVertex ;
	MV1_FILEHEAD_PHYSICS_F1		*F1FileHeadPhysics ;
	MV1_PHYSICS_RIGIDBODY_BASE	*PhysicsRigidBody ;
	MV1_PHYSICS_RIGIDBODY_F1	*F1PhysicsRigidBody ;
	MV1_PHYSICS_JOINT_BASE		*PhysicsJoint ;
	MV1_PHYSICS_JOINT_F1		*F1PhysicsJoint ;
	HANDLE FileHandle ;
	void *TempBuffer = NULL, *AnimBuffer = NULL, *VertexBuffer = NULL, *ChangeDrawMaterialTableBuffer = NULL ;
	DWORD HeaderSize, i, j, k ;
	int AttachIndex = 0 ;
	DWORD WriteSize ;
	MV1MODEL_FILEHEADER_F1 *FHeader ;
	VECTOR PosMin, PosMax, PosWidth ;
	BYTE *Dest, *Start ;
	DWORD Size ;
	void *PressData ;
	DWORD PressDataSize ;
	DWORD TempBufferUseSize ;
	DWORD TempBufferSize, HeaderBufferSize, VertexBufferSize, AnimBufferSize, ChangeDrawMaterialTableBufferSize ;
	bool MeshSave, AnimSave ;
	int Err = -1 ;

	MeshSave = ( SaveType & MV1_SAVETYPE_MESH ) ? true : false ;
	AnimSave = ( SaveType & MV1_SAVETYPE_ANIM ) ? true : false ;
	F1FileHeadShape = NULL ;
	F1FileHeadPhysics = NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// アニメーション用のモデルハンドルのチェック
	if( AnimMHandle < 0 || MV1MDLCHK( AnimMHandle, AnimModel ) )
	{
		AnimModelBase = ModelBase ;
		AnimModel = Model ;
		AnimMHandle = -1 ;
	}
	else
	{
		AnimModelBase = AnimModel->BaseData ;
	}

	// 最大頂点座標と最小頂点座標を算出する
	MV1GetMaxMinPosition( MHandle, NULL, NULL ) ;

	// ヘッダサイズの簡易計算
	{
		int FrameNum ;

		HeaderSize =
			sizeof( MV1MODEL_FILEHEADER_F1 ) + 128 +
			ModelBase->ChangeMatrixTableSize +
			sizeof( MV1_FRAME_F1 ) * ModelBase->FrameNum ;

		if( MeshSave )
		{
			HeaderSize +=
				sizeof( DWORD /*MV1_SKIN_BONE_F1 **/ )         * ModelBase->FrameUseSkinBoneNum +
				( sizeof( MV1_MATERIAL_F1 ) + sizeof( MV1_MATERIAL_TOON_F1 ) ) * ModelBase->MaterialNum +
				sizeof( MV1_LIGHT_F1 )               * ModelBase->LightNum +
				sizeof( MV1_TEXTURE_F1 )             * ModelBase->TextureNum +
				sizeof( MV1_MESH_F1 )                * ModelBase->MeshNum +
				sizeof( MV1_SKIN_BONE_F1 )           * ModelBase->SkinBoneNum +
				sizeof( MV1_SKIN_BONE_USE_FRAME_F1 ) * ModelBase->SkinBoneUseFrameNum +
				sizeof( MV1_TRIANGLE_LIST_F1 )       * ModelBase->TriangleListNum ;
			if( ModelBase->ShapeNum != 0 )
			{
				Frame = ModelBase->Frame ;
				FrameNum = 0 ;
				for( i = 0 ; i < ( DWORD )ModelBase->FrameNum ; i ++, Frame ++ )
				{
					if( Frame->ShapeNum != 0 )
						FrameNum ++ ;
				}

				HeaderSize +=
					sizeof( MV1_FILEHEAD_SHAPE_F1 ) +
					sizeof( MV1_FRAME_SHAPE_F1 )    * FrameNum +
					sizeof( MV1_SHAPE_F1 )          * ModelBase->ShapeNum +
					sizeof( MV1_SHAPE_MESH_F1 )     * ModelBase->ShapeMeshNum +
					sizeof( MV1_SHAPE_VERTEX_F1 )   * ModelBase->ShapeVertexNum ;
			}
		}

		if( AnimSave )
		{
			HeaderSize +=
				sizeof( MV1_ANIM_KEYSET_F1 ) * AnimModelBase->AnimKeySetNum +
				sizeof( MV1_ANIM_F1 )        * AnimModelBase->AnimNum +
				sizeof( MV1_ANIMSET_F1 )     * AnimModelBase->AnimSetNum ;
		}
	}

	// 各バッファの初期最大サイズをセット
	AnimBufferSize                    = AnimSave ? AnimModelBase->AnimKeyDataSize + sizeof( MV1_ANIM_KEYSET ) * AnimModelBase->AnimKeySetNum : 0 ;
	VertexBufferSize                  = MeshSave ? ModelBase->VertexDataSize * 2 : 0 ;
	ChangeDrawMaterialTableBufferSize = ( DWORD )( MeshSave ? 0 : 4 * 1024 * 1024 ) ;
	HeaderBufferSize                  = HeaderSize * 2 + 4 * 1024 * 1024 ;

SAVELOOP :
	TempBufferSize = ( HeaderBufferSize + VertexBufferSize + AnimBufferSize + ChangeDrawMaterialTableBufferSize ) * 2 ;

	if( TempBuffer )
	{
		DXFREE( TempBuffer ) ;
		TempBuffer = NULL ;
	}
	if( AnimBuffer )
	{
		DXFREE( AnimBuffer ) ;
		AnimBuffer = NULL ;
	}
	if( VertexBuffer )
	{
		DXFREE( VertexBuffer ) ;
		VertexBuffer = NULL ;
	}
	if( ChangeDrawMaterialTableBuffer )
	{
		DXFREE( ChangeDrawMaterialTableBuffer ) ;
		ChangeDrawMaterialTableBuffer = NULL ;
	}

	// バッファを確保する
	if( TempBufferSize )
	{
		TempBuffer = DXALLOC( TempBufferSize ) ;
		if( TempBuffer == NULL ) return -1 ;
		_MEMSET( TempBuffer, 0, TempBufferSize ) ;
	}

	if( VertexBufferSize )
	{
		VertexBuffer = DXALLOC( VertexBufferSize ) ;
		if( VertexBuffer == NULL ) return -1 ;
		_MEMSET( VertexBuffer, 0, VertexBufferSize ) ;
	}

	if( AnimBufferSize )
	{
		AnimBuffer = DXALLOC( AnimBufferSize ) ;
		if( AnimBuffer == NULL ) return -1 ;
		_MEMSET( AnimBuffer, 0, AnimBufferSize ) ;
	}

	if( ChangeDrawMaterialTableBufferSize )
	{
		ChangeDrawMaterialTableBuffer = DXALLOC( ChangeDrawMaterialTableBufferSize ) ;
		if( ChangeDrawMaterialTableBuffer == NULL ) return -1 ;
		_MEMSET( ChangeDrawMaterialTableBuffer, 0, ChangeDrawMaterialTableBufferSize ) ;
	}

	// ヘッダのセット
	FHeader = ( MV1MODEL_FILEHEADER_F1 * )TempBuffer ;
	TempBufferUseSize = sizeof( MV1MODEL_FILEHEADER_F1 ) + 128 ;
	FHeader->CheckID[ 0 ] = 'M' ;
	FHeader->CheckID[ 1 ] = 'V' ;
	FHeader->CheckID[ 2 ] = '1' ;
	FHeader->CheckID[ 3 ] = '1' ;
	FHeader->Version = 0 ;

	FHeader->RightHandType             = ModelBase->RightHandType ;
	FHeader->MaterialNumberOrderDraw   = ( BYTE )ModelBase->MaterialNumberOrderDraw ;
	FHeader->IsStringUTF8              = 1 ;

	if( MeshSave )
	{
		FHeader->ChangeDrawMaterialTableSize  = ModelBase->ChangeDrawMaterialTableSize ;
		FHeader->ChangeDrawMaterialTable      = TempBufferUseSize ;
		TempBufferUseSize                     += ModelBase->ChangeDrawMaterialTableSize ;
	}
	else
	{
		if( ChangeDrawMaterialTableBuffer == NULL ) goto ERRORLABEL ;
		FHeader->ChangeDrawMaterialTableSize  = 0 ;
	}

	FHeader->ChangeMatrixTableSize     = ModelBase->ChangeMatrixTableSize ;
	FHeader->ChangeMatrixTable         = TempBufferUseSize ;
	TempBufferUseSize                 += ModelBase->ChangeMatrixTableSize ;

	FHeader->FrameNum                  = ModelBase->FrameNum ;
	FHeader->Frame                     = TempBufferUseSize ;
	TempBufferUseSize                 += sizeof( MV1_FRAME_F1 ) * ModelBase->FrameNum ;

	if( MeshSave )
	{
		FHeader->FrameUseSkinBoneNum       = ModelBase->FrameUseSkinBoneNum ;
		FHeader->FrameUseSkinBone          = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( DWORD /*MV1_SKIN_BONE_F1 **/ ) * ModelBase->FrameUseSkinBoneNum ;

		FHeader->MaterialNum               = ModelBase->MaterialNum ;
		FHeader->Material                  = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_MATERIAL_F1 ) * ModelBase->MaterialNum ;
		F1MaterialToonAddr                 = ( MV1_MATERIAL_TOON_F1 * )( DWORD_PTR )TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_MATERIAL_TOON_F1 ) * ModelBase->MaterialNum ;

		FHeader->LightNum                  = ModelBase->LightNum ;
		FHeader->Light                     = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_LIGHT_F1 ) * ModelBase->LightNum ;

		FHeader->TextureNum                = ModelBase->TextureNum ;
		FHeader->Texture                   = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_TEXTURE_F1 ) * ModelBase->TextureNum ;

		FHeader->MeshNum                   = ModelBase->MeshNum ;
		FHeader->Mesh                      = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_MESH_F1 ) * ModelBase->MeshNum ;

		FHeader->SkinBoneNum               = ModelBase->SkinBoneNum ;
		FHeader->SkinBone                  = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_SKIN_BONE_F1 ) * ModelBase->SkinBoneNum ;
		FHeader->SkinBoneUseFrameNum       = ModelBase->SkinBoneUseFrameNum ;
		FHeader->SkinBoneUseFrame          = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_SKIN_BONE_USE_FRAME_F1 ) * ModelBase->SkinBoneUseFrameNum ;

		FHeader->TriangleListNum           = ModelBase->TriangleListNum ;
		FHeader->TriangleList              = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_TRIANGLE_LIST_F1 ) * ModelBase->TriangleListNum ;

		FHeader->TriangleNum               = ModelBase->TriangleNum ;
		FHeader->TriangleListVertexNum     = ModelBase->TriangleListVertexNum ;

		FHeader->TriangleListNormalPositionNum     = ModelBase->TriangleListNormalPositionNum ;
		FHeader->TriangleListSkinPosition4BNum     = ModelBase->TriangleListSkinPosition4BNum ;
		FHeader->TriangleListSkinPosition8BNum     = ModelBase->TriangleListSkinPosition8BNum ;
		FHeader->TriangleListSkinPositionFREEBSize = ModelBase->TriangleListSkinPositionFREEBSize ;
		FHeader->MeshPositionSize          = ModelBase->MeshPositionSize ;
		FHeader->MeshNormalNum             = ModelBase->MeshNormalNum ;
		FHeader->MeshVertexSize            = ModelBase->MeshVertexSize ;
		FHeader->MeshFaceNum               = ModelBase->MeshFaceNum ;
		FHeader->MeshVertexIndexNum        = ModelBase->MeshVertexIndexNum ;
		FHeader->TriangleListIndexNum      = ModelBase->TriangleListIndexNum ;

		FHeader->VertexDataSize            = 0 ;
		if( VertexBuffer == NULL )  goto ERRORLABEL ;

		if( ModelBase->ShapeNum != 0 )
		{
			FHeader->Shape                 = TempBufferUseSize ;
			TempBufferUseSize             += sizeof( MV1_FILEHEAD_SHAPE_F1 ) ;

			F1FileHeadShape                = ( MV1_FILEHEAD_SHAPE_F1 * )( ( DWORD_PTR )FHeader->Shape + ( DWORD_PTR )FHeader ) ;

			// シェイプデータの存在するフレームを数える
			Frame = ModelBase->Frame ;
			for( i = 0 ; i < ( DWORD )ModelBase->FrameNum ; i ++, Frame ++ )
			{
				if( Frame->ShapeNum != 0 )
					F1FileHeadShape->FrameNum ++ ;
			}
			F1FileHeadShape->Frame         = TempBufferUseSize ;
			TempBufferUseSize             += sizeof( MV1_FRAME_SHAPE_F1 ) * F1FileHeadShape->FrameNum ;

			F1FileHeadShape->DataNum       = ModelBase->ShapeNum ;
			F1FileHeadShape->Data          = TempBufferUseSize ;
			TempBufferUseSize             += sizeof( MV1_SHAPE_F1 ) * ModelBase->ShapeNum ;

			F1FileHeadShape->MeshNum       = ModelBase->ShapeMeshNum ;
			F1FileHeadShape->Mesh          = TempBufferUseSize ;
			TempBufferUseSize             += sizeof( MV1_SHAPE_MESH_F1 ) * ModelBase->ShapeMeshNum ;

			F1FileHeadShape->VertexNum       = ModelBase->ShapeVertexNum ;
			F1FileHeadShape->Vertex          = TempBufferUseSize ;
			TempBufferUseSize               += sizeof( MV1_SHAPE_VERTEX_F1 ) * ModelBase->ShapeVertexNum ;
			F1FileHeadShape->VertexNum       = 0 ;

			F1FileHeadShape->PressVertexDataSize = 0 ;
			F1FileHeadShape->PressVertexData = NULL ;

			F1FileHeadShape->TargetMeshVertexNum   = ModelBase->ShapeTargetMeshVertexNum ;
			F1FileHeadShape->ShapeVertexUnitSize   = sizeof( MV1_SHAPE_VERTEX_F1 ) ;
			F1FileHeadShape->NormalPositionNum     = ModelBase->ShapeNormalPositionNum ;
			F1FileHeadShape->SkinPosition4BNum     = ModelBase->ShapeSkinPosition4BNum ;
			F1FileHeadShape->SkinPosition8BNum     = ModelBase->ShapeSkinPosition8BNum ;
			F1FileHeadShape->SkinPositionFREEBSize = ModelBase->ShapeSkinPositionFREEBSize ;
		}

		if( ModelBase->PhysicsJointNum != 0 || ModelBase->PhysicsJointNum != 0 )
		{
			FHeader->Physics               = TempBufferUseSize ;
			TempBufferUseSize             += sizeof( MV1_FILEHEAD_PHYSICS_F1 ) ;

			F1FileHeadPhysics              = ( MV1_FILEHEAD_PHYSICS_F1 * )( ( DWORD_PTR )FHeader->Physics + ( DWORD_PTR )FHeader ) ;

			F1FileHeadPhysics->WorldGravity = ModelBase->PhysicsGravity ;

			F1FileHeadPhysics->RigidBodyNum = ModelBase->PhysicsRigidBodyNum ;
			F1FileHeadPhysics->RigidBody   = TempBufferUseSize ;
			TempBufferUseSize             += sizeof( MV1_PHYSICS_RIGIDBODY_F1 ) * F1FileHeadPhysics->RigidBodyNum ;

			F1FileHeadPhysics->JointNum    = ModelBase->PhysicsJointNum ;
			F1FileHeadPhysics->Joint       = TempBufferUseSize ;
			TempBufferUseSize             += sizeof( MV1_PHYSICS_JOINT_F1 ) * F1FileHeadPhysics->JointNum ;
		}
	}

	if( AnimSave )
	{
		FHeader->OriginalAnimKeyDataSize = AnimModelBase->AnimKeyDataSize ;
		FHeader->AnimKeyDataSize         = 0 ;
		if( AnimModelBase->AnimKeyDataSize != 0 || AnimModelBase->AnimKeySetNum != 0 )
		{
			if( AnimBuffer == NULL )  goto ERRORLABEL ;
		}
		else
		{
			FHeader->AnimKeyData           = NULL ;
		}

		FHeader->AnimKeySetNum             = AnimModelBase->AnimKeySetNum ;
		FHeader->AnimKeySetUnitSize        = sizeof( MV1_ANIM_KEYSET_F1 ) ;
		FHeader->AnimKeySet                = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_ANIM_KEYSET_F1 ) * AnimModelBase->AnimKeySetNum ;

		FHeader->AnimNum                   = AnimModelBase->AnimNum ;
		FHeader->AnimUnitSize              = sizeof( MV1_ANIM_F1 ) ;
		FHeader->Anim                      = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_ANIM_F1 ) * AnimModelBase->AnimNum ;

		FHeader->AnimSetNum                = AnimModelBase->AnimSetNum ;
		FHeader->AnimSet                   = TempBufferUseSize ;
		TempBufferUseSize                 += sizeof( MV1_ANIMSET_F1 ) * AnimModelBase->AnimSetNum ;
	}

	FHeader->StringSize                = 0 ;
	FHeader->StringBuffer              = TempBufferUseSize ;

	// 外部データの保存
	FHeader->UserData[ 0 ] = ModelBase->UserData[ 0 ] ;
	FHeader->UserData[ 1 ] = ModelBase->UserData[ 1 ] ;
	FHeader->UserData[ 2 ] = ModelBase->UserData[ 2 ] ;
	FHeader->UserData[ 3 ] = ModelBase->UserData[ 3 ] ;

	// 行列変更情報テーブルの保存
	_MEMCPY( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->ChangeMatrixTable,       ModelBase->ChangeMatrixTable,       ( size_t )ModelBase->ChangeMatrixTableSize ) ;

	if( MeshSave )
	{
		// マテリアル変更情報テーブルの保存
		_MEMCPY( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->ChangeDrawMaterialTable, ModelBase->ChangeDrawMaterialTable, ( size_t )ModelBase->ChangeDrawMaterialTableSize ) ;

		// スキニングメッシュのボーンを使用するフレームの情報を保存
		_MEMCPY( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->SkinBoneUseFrame, ModelBase->SkinBoneUseFrame, sizeof( MV1_SKIN_BONE_USE_FRAME_F1 ) * ModelBase->SkinBoneUseFrameNum ) ;

		// フレームが使用しているボーンへのポインタの情報を保存
		for( i = 0 ; i < ( DWORD )ModelBase->FrameUseSkinBoneNum ; i ++ )
		{
			*( ( DWORD * )( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->FrameUseSkinBone + sizeof( DWORD ) * i ) ) = ( DWORD )( DWORD_PTR )( ( MV1_SKIN_BONE_F1 * )( DWORD_PTR )FHeader->SkinBone + ( ModelBase->FrameUseSkinBone[ i ] - ModelBase->SkinBone ) ) ;
		}
	}

	// フレームの情報を埋める
	FHeader->TopFrameNum   = ModelBase->TopFrameNum ;
	FHeader->FirstTopFrame = ModelBase->FirstTopFrame ? FHeader->Frame + sizeof( MV1_FRAME_F1 ) * ModelBase->FirstTopFrame->Index : NULL ;
	FHeader->LastTopFrame  = ModelBase->LastTopFrame  ? FHeader->Frame + sizeof( MV1_FRAME_F1 ) * ModelBase->LastTopFrame->Index  : NULL ;
	F1Frame = ( MV1_FRAME_F1 * )( ( DWORD_PTR )FHeader->Frame + ( DWORD_PTR )FHeader ) ;
	Frame = ModelBase->Frame ;
	if( FHeader->Shape )
	{
		F1FileHeadShape->FrameNum = 0 ;
	}
	for( i = 0 ; i < ( DWORD )ModelBase->FrameNum ; i ++, F1Frame ++, Frame ++ )
	{
		F1Frame->Name = ( DWORD )FHeader->StringSize ;
		FHeader->StringSize += ConvString( ( const char * )Frame->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1Frame->Name ), DX_CODEPAGE_UTF8 ) ;
		FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

		F1Frame->Index = Frame->Index ;

		if( MeshSave == false )
		{
			MV1_CHANGE_BASE ChangeInfo ;
			MV1ChangeInfoSetup( &ChangeInfo, ( BYTE * )ChangeDrawMaterialTableBuffer + FHeader->ChangeDrawMaterialTableSize, ( int )( i + 1 ), Frame->TotalChildNum + 1 ) ;
			FHeader->ChangeDrawMaterialTableSize += F1Frame->ChangeDrawMaterialInfo.Size * 4 ;
			F1Frame->ChangeDrawMaterialInfo.Target   = ChangeInfo.Target ;
			F1Frame->ChangeDrawMaterialInfo.Fill	 = ( DWORD )FHeader->ChangeDrawMaterialTableSize ;
			F1Frame->ChangeDrawMaterialInfo.Size     = ChangeInfo.Size ;
			F1Frame->ChangeDrawMaterialInfo.CheckBit = ChangeInfo.CheckBit ;
		}
		else
		{
			F1Frame->ChangeDrawMaterialInfo.Target   = Frame->ChangeDrawMaterialInfo.Target ;
			F1Frame->ChangeDrawMaterialInfo.Fill     = ( DWORD )( DWORD_PTR )( ( DWORD * )( DWORD_PTR )FHeader->ChangeDrawMaterialTable + ( Frame->ChangeDrawMaterialInfo.Fill - ModelBase->ChangeDrawMaterialTable ) ) ;
			F1Frame->ChangeDrawMaterialInfo.Size     = Frame->ChangeDrawMaterialInfo.Size ;
			F1Frame->ChangeDrawMaterialInfo.CheckBit = Frame->ChangeDrawMaterialInfo.CheckBit ;
		}

		F1Frame->ChangeMatrixInfo.Target         = Frame->ChangeMatrixInfo.Target ;
		F1Frame->ChangeMatrixInfo.Fill           = ( DWORD )( DWORD_PTR )( ( DWORD * )( DWORD_PTR )FHeader->ChangeMatrixTable       + ( Frame->ChangeMatrixInfo.Fill       - ModelBase->ChangeMatrixTable ) ) ;
		F1Frame->ChangeMatrixInfo.Size           = Frame->ChangeMatrixInfo.Size ;
		F1Frame->ChangeMatrixInfo.CheckBit       = Frame->ChangeMatrixInfo.CheckBit ;

		F1Frame->TotalChildNum = Frame->TotalChildNum ;
		F1Frame->Parent     = Frame->Parent     ? FHeader->Frame + sizeof( MV1_FRAME_F1 ) * Frame->Parent->Index     : NULL ;
		F1Frame->FirstChild = Frame->FirstChild ? FHeader->Frame + sizeof( MV1_FRAME_F1 ) * Frame->FirstChild->Index : NULL ;
		F1Frame->LastChild  = Frame->LastChild  ? FHeader->Frame + sizeof( MV1_FRAME_F1 ) * Frame->LastChild->Index  : NULL ;
		F1Frame->Prev       = Frame->Prev       ? FHeader->Frame + sizeof( MV1_FRAME_F1 ) * Frame->Prev->Index       : NULL ;
		F1Frame->Next       = Frame->Next       ? FHeader->Frame + sizeof( MV1_FRAME_F1 ) * Frame->Next->Index       : NULL ;

		F1Frame->Translate = Frame->Translate ;
		F1Frame->Scale = Frame->Scale ;
		F1Frame->Rotate = Frame->Rotate ;
		F1Frame->RotateOrder = Frame->RotateOrder ;
		F1Frame->Quaternion = Frame->Quaternion ;

		F1Frame->Flag = Frame->Flag ;
		if( Frame->IgnoreParentTransform ) F1Frame->Flag |= MV1_FRAMEFLAG_IGNOREPARENTTRANS ;

		F1Frame->UserData[ 0 ] = Frame->UserData[ 0 ] ;
		F1Frame->UserData[ 1 ] = Frame->UserData[ 1 ] ;
		F1Frame->UserData[ 2 ] = Frame->UserData[ 2 ] ;
		F1Frame->UserData[ 3 ] = Frame->UserData[ 3 ] ;

		F1Frame->SmoothingAngle = Frame->SmoothingAngle ;
		F1Frame->AutoCreateNormal = Frame->AutoCreateNormal ;

		F1Frame->PreRotate = Frame->PreRotate ;
		F1Frame->PostRotate = Frame->PostRotate ;

		if( MeshSave )
		{
			F1Frame->IsSkinMesh = Frame->IsSkinMesh ;
			F1Frame->TriangleNum = Frame->TriangleNum ;
			F1Frame->VertexNum = Frame->VertexNum ;
			F1Frame->MeshNum = Frame->MeshNum ;
			if( Frame->Mesh )
			{
				F1Frame->Mesh = ( DWORD )( DWORD_PTR )( ( MV1_MESH_F1 * )( DWORD_PTR )FHeader->Mesh + ( Frame->Mesh - ModelBase->Mesh ) ) ;
			}

			F1Frame->SkinBoneNum = Frame->SkinBoneNum ;
			if( Frame->SkinBone )
			{
				F1Frame->SkinBone = ( DWORD )( DWORD_PTR )( ( MV1_SKIN_BONE_F1 * )( DWORD_PTR )FHeader->SkinBone + ( Frame->SkinBone - ModelBase->SkinBone ) ) ;
			}

			if( Frame->Light )
			{
				F1Frame->Light = ( DWORD )( DWORD_PTR )( ( MV1_LIGHT_F1 * )( DWORD_PTR )FHeader->Light + ( Frame->Light - ModelBase->Light ) ) ;
			}

			F1Frame->UseSkinBoneNum = Frame->UseSkinBoneNum ;
			if( Frame->UseSkinBoneNum )
			{
				F1Frame->UseSkinBone = ( DWORD )( FHeader->FrameUseSkinBone + ( Frame->UseSkinBone - ModelBase->FrameUseSkinBone ) * sizeof( DWORD /* MV1_SKIN_BONE_F1* */ ) ) ;
			}

			F1Frame->TotalMeshNum = Frame->TotalMeshNum ;
			F1Frame->MaxBoneBlendNum = ( unsigned short )Frame->MaxBoneBlendNum ;
			F1Frame->PositionNum = Frame->PositionNum ;
			F1Frame->NormalNum = Frame->NormalNum ;

			// 座標データと法線データの作成
			if( F1Frame->PositionNum != 0 )
			{
				MV1_MESH_POSITION *Pos ;
				MV1_MESH_NORMAL *Nrm ;

				// アドレスのセット
				F1Frame->PositionAndNormalData = FHeader->VertexDataSize ;
				Dest = ( BYTE * )VertexBuffer + FHeader->VertexDataSize ;
				Start = Dest ;

				// 頂点データタイプの決定
				F1Frame->VertFlag = 0 ;
//				if( F1Frame->AutoCreateNormal )
//				{
//					F1Frame->VertFlag |= MV1_FRAME_NORMAL_TYPE_NONE ;
//				}
//				else
				{
					if( Normal8BitFlag )
					{
						F1Frame->VertFlag |= MV1_FRAME_NORMAL_TYPE_S8 ;
					}
					else
					{
						F1Frame->VertFlag |= MV1_FRAME_NORMAL_TYPE_S16 ;
					}
				}
				if( Position16BitFlag )
				{
					F1Frame->VertFlag |= MV1_FRAME_VERT_FLAG_POSITION_B16 ;
				}

				// 法線の他に接線と従法線があるかをセットする
				if( Frame->Flag & MV1_FRAMEFLAG_TANGENT_BINORMAL )
				{
					F1Frame->VertFlag |= MV1_FRAME_VERT_FLAG_NOMRAL_TANGENT_BINORMAL ;
				}

				// 頂点のウエイト値があるかどうかをセットする
				if( F1Frame->UseSkinBoneNum == 0 )
				{
					F1Frame->VertFlag |= MV1_FRAME_VERT_FLAG_MATRIX_WEIGHT_NONE ;
				}
				else
				{
					// 頂点の行列インデックス値が０～２５４以内に収まっているかをセットする
					if( F1Frame->UseSkinBoneNum > 255 )
						F1Frame->VertFlag |= MV1_FRAME_MATRIX_INDEX_TYPE_U16 << 4 ;

					// 行列ウエイト値が８ビットかどうかの指定をセットする
					if( Weight8BitFlag == 0 )
						F1Frame->VertFlag |= MV1_FRAME_MATRIX_WEIGHT_TYPE_U16 << 5 ;
				}

				// 16bitかどうかで処理を分岐
				if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_POSITION_B16 )
				{
					// 座標値ＸＹＺの最大値と最小値を求める
					PosMax.x = -1000000000000.0f ;
					PosMax.y = -1000000000000.0f ;
					PosMax.z = -1000000000000.0f ;
					PosMin.x =  1000000000000.0f ;
					PosMin.y =  1000000000000.0f ;
					PosMin.z =  1000000000000.0f ;

					Pos = Frame->Position ;
					for( j = 0 ; j < ( DWORD )F1Frame->PositionNum ; j ++, Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Pos + Frame->PosUnitSize ) )
					{
						if( Pos->Position.x > PosMax.x ) PosMax.x = Pos->Position.x ;
						if( Pos->Position.x < PosMin.x ) PosMin.x = Pos->Position.x ;
						if( Pos->Position.y > PosMax.y ) PosMax.y = Pos->Position.y ;
						if( Pos->Position.y < PosMin.y ) PosMin.y = Pos->Position.y ;
						if( Pos->Position.z > PosMax.z ) PosMax.z = Pos->Position.z ;
						if( Pos->Position.z < PosMin.z ) PosMin.z = Pos->Position.z ;
					}
					PosWidth.x = PosMax.x - PosMin.x ;
					PosWidth.y = PosMax.y - PosMin.y ;
					PosWidth.z = PosMax.z - PosMin.z ;

					// 補助情報を格納する
					( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Dest )[ 0 ].Min   = PosMin.x ;
					( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Dest )[ 0 ].Width = PosWidth.x ;
					( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Dest )[ 1 ].Min   = PosMin.y ;
					( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Dest )[ 1 ].Width = PosWidth.y ;
					( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Dest )[ 2 ].Min   = PosMin.z ;
					( ( MV1_POSITION_16BIT_SUBINFO_F1 * )Dest )[ 2 ].Width = PosWidth.z ;
					Dest += 24 ;

					Pos = Frame->Position ;
					for( j = 0 ; j < ( DWORD )F1Frame->PositionNum ; j ++, Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Pos + Frame->PosUnitSize ) )
					{
						( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( ( Pos->Position.x - PosMin.x ) * 60000.0f / PosWidth.x ) ;
						( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( ( Pos->Position.y - PosMin.y ) * 60000.0f / PosWidth.y ) ;
						( ( WORD * )Dest )[ 2 ] = ( WORD )_FTOL( ( Pos->Position.z - PosMin.z ) * 60000.0f / PosWidth.z ) ;
						Dest += 6 ;
					}
				}
				else
				{
					Pos = Frame->Position ;
					for( j = 0 ; j < ( DWORD )F1Frame->PositionNum ; j ++, Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Pos + Frame->PosUnitSize ) )
					{
						*( ( VECTOR * )Dest ) = Pos->Position ;
						Dest += 12 ;
					}
				}

				// スキニング情報がある場合は保存
				if( ( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_MATRIX_WEIGHT_NONE ) == 0 )
				{
					Pos = Frame->Position ;
					for( j = 0 ; j < ( DWORD )F1Frame->PositionNum ; j ++, Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Pos + Frame->PosUnitSize ) )
					{
						for( k = 0 ; k < F1Frame->MaxBoneBlendNum ; k ++ )
						{
							int WeightParam ;

							if( ( ( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_MATRIX_INDEX_MASK ) >> 4 ) == MV1_FRAME_MATRIX_INDEX_TYPE_U8 )
							{
								*Dest = ( BYTE )( Pos->BoneWeight[ k ].Index == -1 ? 255 : Pos->BoneWeight[ k ].Index ) ;
								Dest ++ ;
							}
							else
							{
								*( ( WORD * )Dest ) = ( WORD )( Pos->BoneWeight[ k ].Index == -1 ? 65535 : Pos->BoneWeight[ k ].Index ) ;
								Dest += 2 ;
							}
							if( Pos->BoneWeight[ k ].Index == -1 )
								break ;

							if( ( ( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_MATRIX_WEIGHT_MASK ) >> 5 ) == MV1_FRAME_MATRIX_WEIGHT_TYPE_U8 )
							{
								WeightParam = _FTOL( Pos->BoneWeight[ k ].W * 255.0f ) ;
								if( WeightParam > 255 )
								{
									*Dest = 255 ;
								}
								else
								if( WeightParam < 0 )
								{
									*Dest = 0 ;
								}
								else
								{
									*Dest = ( BYTE )WeightParam ;
								}
								Dest ++ ;
							}
							else
							{
								WeightParam = _FTOL( Pos->BoneWeight[ k ].W * 65535.0f ) ;
								if( WeightParam > 65535 )
								{
									*( ( WORD * )Dest ) = 65535 ;
								}
								else
								if( WeightParam < 0 )
								{
									*( ( WORD * )Dest ) = 0 ;
								}
								else
								{
									*( ( WORD * )Dest ) = ( WORD )WeightParam ;
								}
								Dest += 2 ;
							}
						}
					}
				}

				// 法線情報の保存
				Nrm = Frame->Normal ;
				switch( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NORMAL_TYPE_MASK )
				{
				case MV1_FRAME_NORMAL_TYPE_NONE :
					break ;

				case MV1_FRAME_NORMAL_TYPE_S8 :
					if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NOMRAL_TANGENT_BINORMAL )
					{
						for( j = 0 ; j < ( DWORD )Frame->NormalNum ; j ++, Nrm ++ )
						{
							( ( char * )Dest )[ 0 ] = ( char )_FTOL( Nrm->Normal.x   * 127.0f ) ;
							( ( char * )Dest )[ 1 ] = ( char )_FTOL( Nrm->Normal.y   * 127.0f ) ;
							( ( char * )Dest )[ 2 ] = ( char )_FTOL( Nrm->Normal.z   * 127.0f ) ;

							( ( char * )Dest )[ 3 ] = ( char )_FTOL( Nrm->Tangent.x  * 127.0f ) ;
							( ( char * )Dest )[ 4 ] = ( char )_FTOL( Nrm->Tangent.y  * 127.0f ) ;
							( ( char * )Dest )[ 5 ] = ( char )_FTOL( Nrm->Tangent.z  * 127.0f ) ;

							( ( char * )Dest )[ 6 ] = ( char )_FTOL( Nrm->Binormal.x * 127.0f ) ;
							( ( char * )Dest )[ 7 ] = ( char )_FTOL( Nrm->Binormal.y * 127.0f ) ;
							( ( char * )Dest )[ 8 ] = ( char )_FTOL( Nrm->Binormal.z * 127.0f ) ;
							Dest += 9 ;
						}
					}
					else
					{
						for( j = 0 ; j < ( DWORD )Frame->NormalNum ; j ++, Nrm ++ )
						{
							( ( char * )Dest )[ 0 ] = ( char )_FTOL( Nrm->Normal.x * 127.0f ) ;
							( ( char * )Dest )[ 1 ] = ( char )_FTOL( Nrm->Normal.y * 127.0f ) ;
							( ( char * )Dest )[ 2 ] = ( char )_FTOL( Nrm->Normal.z * 127.0f ) ;
							Dest += 3 ;
						}
					}
					break ;

				case MV1_FRAME_NORMAL_TYPE_S16 :
					if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NOMRAL_TANGENT_BINORMAL )
					{
						for( j = 0 ; j < ( DWORD )Frame->NormalNum ; j ++, Nrm ++ )
						{
							( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( Nrm->Normal.x   * 32767.0f ) ;
							( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( Nrm->Normal.y   * 32767.0f ) ;
							( ( WORD * )Dest )[ 2 ] = ( WORD )_FTOL( Nrm->Normal.z   * 32767.0f ) ;

							( ( WORD * )Dest )[ 3 ] = ( WORD )_FTOL( Nrm->Tangent.x  * 32767.0f ) ;
							( ( WORD * )Dest )[ 4 ] = ( WORD )_FTOL( Nrm->Tangent.y  * 32767.0f ) ;
							( ( WORD * )Dest )[ 5 ] = ( WORD )_FTOL( Nrm->Tangent.z  * 32767.0f ) ;

							( ( WORD * )Dest )[ 6 ] = ( WORD )_FTOL( Nrm->Binormal.x * 32767.0f ) ;
							( ( WORD * )Dest )[ 7 ] = ( WORD )_FTOL( Nrm->Binormal.y * 32767.0f ) ;
							( ( WORD * )Dest )[ 8 ] = ( WORD )_FTOL( Nrm->Binormal.z * 32767.0f ) ;
							Dest += 18 ;
						}
					}
					else
					{
						for( j = 0 ; j < ( DWORD )Frame->NormalNum ; j ++, Nrm ++ )
						{
							( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( Nrm->Normal.x * 32767.0f ) ;
							( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( Nrm->Normal.y * 32767.0f ) ;
							( ( WORD * )Dest )[ 2 ] = ( WORD )_FTOL( Nrm->Normal.z * 32767.0f ) ;
							Dest += 6 ;
						}
					}
					break ;

				case MV1_FRAME_NORMAL_TYPE_F32 :
					if( F1Frame->VertFlag & MV1_FRAME_VERT_FLAG_NOMRAL_TANGENT_BINORMAL )
					{
							( ( VECTOR * )Dest )[ 0 ] = Nrm->Normal ;
							( ( VECTOR * )Dest )[ 1 ] = Nrm->Tangent ;
							( ( VECTOR * )Dest )[ 2 ] = Nrm->Binormal ;
							Dest += 36 ;
					}
					else
					{
						for( j = 0 ; j < ( DWORD )Frame->NormalNum ; j ++, Nrm ++ )
						{
							*( ( VECTOR * )Dest ) = Nrm->Normal ;
							Dest += 12 ;
						}
					}
					break ;
				}

				Size = ( DWORD )( Dest - Start ) ;
				Size = ( Size + 3 ) / 4 * 4 ;
				FHeader->VertexDataSize += Size ;
			}

			// シェイプデータがある場合はそれの保存
			if( Frame->ShapeNum )
			{
				F1Frame->FrameShape = ( DWORD )( ( DWORD_PTR )( F1FileHeadShape->Frame + sizeof( MV1_FRAME_SHAPE_F1 ) * F1FileHeadShape->FrameNum ) ) ;
				F1FileHeadShape->FrameNum ++ ;

				F1FrameShape = ( MV1_FRAME_SHAPE_F1 * )( ( DWORD_PTR )F1Frame->FrameShape + ( DWORD_PTR )FHeader ) ;
				F1FrameShape->ShapeNum = Frame->ShapeNum ;
				F1FrameShape->Shape = ( DWORD )( DWORD_PTR )( ( MV1_SHAPE_F1 * )( DWORD_PTR )F1FileHeadShape->Data + ( Frame->Shape - ModelBase->Shape ) ) ;
			}
		}

		F1Frame->DimPrev = i == 0                                ? 0 : FHeader->Frame + sizeof( MV1_FRAME_F1 ) * ( i - 1 ) ;
		F1Frame->DimNext = i == ( DWORD )ModelBase->FrameNum - 1 ? 0 : FHeader->Frame + sizeof( MV1_FRAME_F1 ) * ( i + 1 ) ;
	}

	if( MeshSave )
	{
		// シェイプの情報を埋める
		if( F1FileHeadShape )
		{
			F1Shape = ( MV1_SHAPE_F1 * )( ( DWORD_PTR )F1FileHeadShape->Data + ( DWORD_PTR )FHeader ) ;
			Shape = ModelBase->Shape ;
			for( i = 0 ; i < ( DWORD )ModelBase->ShapeNum ; i ++, Shape ++, F1Shape ++ )
			{
				F1Shape->Index = ( int )i ;

				F1Shape->Name = ( DWORD )FHeader->StringSize ;
				FHeader->StringSize += ConvString( ( const char * )Shape->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1Shape->Name ), DX_CODEPAGE_UTF8 ) ;
				FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

				if( Shape->Container )
				{
					F1Shape->Container = ( DWORD )( DWORD_PTR )( ( MV1_FRAME_F1 * )( DWORD_PTR )FHeader->Frame + ( Shape->Container - ModelBase->Frame ) ) ;
				}

				F1Shape->MeshNum = Shape->MeshNum ;
				if( Shape->MeshNum )
				{
					F1Shape->Mesh = ( DWORD )( DWORD_PTR )( ( MV1_SHAPE_MESH_F1 * )( DWORD_PTR )F1FileHeadShape->Mesh + ( Shape->Mesh - ModelBase->ShapeMesh ) ) ;
				}

				F1Shape->DimPrev = i == 0                                ? 0 : F1FileHeadShape->Data + sizeof( MV1_SHAPE_F1 ) * ( i - 1 ) ;
				F1Shape->DimNext = i == ( DWORD )ModelBase->ShapeNum - 1 ? 0 : F1FileHeadShape->Data + sizeof( MV1_SHAPE_F1 ) * ( i + 1 ) ;
			}

			// シェイプメッシュの情報を埋める
			F1ShapeMesh = ( MV1_SHAPE_MESH_F1 * )( ( DWORD_PTR )F1FileHeadShape->Mesh + ( DWORD_PTR )FHeader ) ;
			ShapeMesh = ModelBase->ShapeMesh ;
			for( i = 0 ; i < ( DWORD )ModelBase->ShapeMeshNum ; i ++, ShapeMesh ++, F1ShapeMesh ++ )
			{
				F1ShapeMesh->Index = ( int )i ;

				F1ShapeMesh->TargetMesh = ( DWORD )( DWORD_PTR )( ( MV1_MESH_F1 * )( DWORD_PTR )FHeader->Mesh + ( ShapeMesh->TargetMesh - ModelBase->Mesh ) ) ;
				F1ShapeMesh->IsVertexPress = 0 ;
				F1ShapeMesh->VertexPressParam = 0 ;
				F1ShapeMesh->VertexNum = ShapeMesh->VertexNum ;
				if( F1ShapeMesh->VertexNum )
				{
					F1ShapeMesh->Vertex = F1FileHeadShape->Vertex + sizeof( MV1_SHAPE_VERTEX_F1 ) * F1FileHeadShape->VertexNum ;
					F1ShapeVertex = ( MV1_SHAPE_VERTEX_F1 * )( ( BYTE * )( DWORD_PTR )F1ShapeMesh->Vertex + ( DWORD_PTR )FHeader ) ;
					ShapeVertex = ShapeMesh->Vertex ;
					for( j = 0 ; ( DWORD )j < ShapeMesh->VertexNum ; j ++, F1ShapeVertex ++, ShapeVertex ++ )
					{
						F1ShapeVertex->TargetMeshVertex = ShapeVertex->TargetMeshVertex ;
						F1ShapeVertex->Position = ShapeVertex->Position ;
						F1ShapeVertex->Normal = ShapeVertex->Normal ;
					}
					F1FileHeadShape->VertexNum += ShapeMesh->VertexNum ;
				}

				F1ShapeMesh->DimPrev = i == 0                                    ? 0 : F1FileHeadShape->Mesh + sizeof( MV1_SHAPE_MESH_F1 ) * ( i - 1 ) ;
				F1ShapeMesh->DimNext = i == ( DWORD )ModelBase->ShapeMeshNum - 1 ? 0 : F1FileHeadShape->Mesh + sizeof( MV1_SHAPE_MESH_F1 ) * ( i + 1 ) ;
			}

			// シェイプ頂点の情報を埋める
/*
			F1ShapeVertex = ( MV1_SHAPE_VERTEX_F1 * )( ( DWORD_PTR )F1FileHeadShape->Vertex + ( DWORD_PTR )FHeader ) ;
			ShapeVertex = ModelBase->ShapeVertex ;
			for( i = 0 ; i < ModelBase->ShapeVertexNum ; i ++, ShapeVertex ++, F1ShapeVertex ++ )
			{
				F1ShapeVertex->TargetMeshVertex = ShapeVertex->TargetMeshVertex ;
				F1ShapeVertex->Position = ShapeVertex->Position ;
				F1ShapeVertex->Normal = ShapeVertex->Normal ;
			}
*/
		}

		// 物理演算用の情報を埋める
		if( F1FileHeadPhysics )
		{
			F1PhysicsRigidBody = ( MV1_PHYSICS_RIGIDBODY_F1 * )( ( DWORD_PTR )F1FileHeadPhysics->RigidBody + ( DWORD_PTR )FHeader ) ;
			PhysicsRigidBody = ModelBase->PhysicsRigidBody ;
			for( i = 0 ; i < ( DWORD )ModelBase->PhysicsRigidBodyNum ; i ++, PhysicsRigidBody ++, F1PhysicsRigidBody ++ )
			{
				F1PhysicsRigidBody->Index = ( int )i ;

				F1PhysicsRigidBody->Name = ( DWORD )FHeader->StringSize ;
				FHeader->StringSize += ConvString( ( const char * )PhysicsRigidBody->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1PhysicsRigidBody->Name ), DX_CODEPAGE_UTF8 ) ;
				FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

				if( PhysicsRigidBody->TargetFrame )
				{
					F1PhysicsRigidBody->TargetFrame = ( DWORD )( DWORD_PTR )( ( MV1_FRAME_F1 * )( DWORD_PTR )FHeader->Frame + ( PhysicsRigidBody->TargetFrame - ModelBase->Frame ) ) ;
				}

				F1PhysicsRigidBody->RigidBodyGroupIndex = PhysicsRigidBody->RigidBodyGroupIndex ;
				F1PhysicsRigidBody->RigidBodyGroupTarget = PhysicsRigidBody->RigidBodyGroupTarget ;
				F1PhysicsRigidBody->ShapeType = PhysicsRigidBody->ShapeType ;
				F1PhysicsRigidBody->ShapeW = PhysicsRigidBody->ShapeW ;
				F1PhysicsRigidBody->ShapeH = PhysicsRigidBody->ShapeH ;
				F1PhysicsRigidBody->ShapeD = PhysicsRigidBody->ShapeD ;
				F1PhysicsRigidBody->Position = PhysicsRigidBody->Position ;
				F1PhysicsRigidBody->Rotation = PhysicsRigidBody->Rotation ;
				F1PhysicsRigidBody->RigidBodyWeight = PhysicsRigidBody->RigidBodyWeight ;
				F1PhysicsRigidBody->RigidBodyPosDim = PhysicsRigidBody->RigidBodyPosDim ;
				F1PhysicsRigidBody->RigidBodyRotDim = PhysicsRigidBody->RigidBodyRotDim ;
				F1PhysicsRigidBody->RigidBodyRecoil = PhysicsRigidBody->RigidBodyRecoil ;
				F1PhysicsRigidBody->RigidBodyFriction = PhysicsRigidBody->RigidBodyFriction ;
				F1PhysicsRigidBody->RigidBodyType = PhysicsRigidBody->RigidBodyType ;
				F1PhysicsRigidBody->NoCopyToBone = PhysicsRigidBody->NoCopyToBone ;

				F1PhysicsRigidBody->DimPrev = i == 0                                           ? 0 : F1FileHeadPhysics->RigidBody + sizeof( MV1_PHYSICS_RIGIDBODY_F1 ) * ( i - 1 ) ;
				F1PhysicsRigidBody->DimNext = i == ( DWORD )ModelBase->PhysicsRigidBodyNum - 1 ? 0 : F1FileHeadPhysics->RigidBody + sizeof( MV1_PHYSICS_RIGIDBODY_F1 ) * ( i + 1 ) ;
			}

			F1PhysicsJoint = ( MV1_PHYSICS_JOINT_F1 * )( ( DWORD_PTR )F1FileHeadPhysics->Joint + ( DWORD_PTR )FHeader ) ;
			PhysicsJoint = ModelBase->PhysicsJoint ;
			for( i = 0 ; i < ( DWORD )ModelBase->PhysicsJointNum ; i ++, PhysicsJoint ++, F1PhysicsJoint ++ )
			{
				F1PhysicsJoint->Index = ( int )i ;

				F1PhysicsJoint->Name = ( DWORD )FHeader->StringSize ;
				FHeader->StringSize += ConvString( ( const char * )PhysicsJoint->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1PhysicsJoint->Name ), DX_CODEPAGE_UTF8 ) ;
				FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

				if( PhysicsJoint->RigidBodyA )
				{
					F1PhysicsJoint->RigidBodyA = ( DWORD )( DWORD_PTR )( ( MV1_PHYSICS_RIGIDBODY_F1 * )( DWORD_PTR )F1FileHeadPhysics->RigidBody + ( PhysicsJoint->RigidBodyA - ModelBase->PhysicsRigidBody ) ) ;
				}

				if( PhysicsJoint->RigidBodyB )
				{
					F1PhysicsJoint->RigidBodyB = ( DWORD )( DWORD_PTR )( ( MV1_PHYSICS_RIGIDBODY_F1 * )( DWORD_PTR )F1FileHeadPhysics->RigidBody + ( PhysicsJoint->RigidBodyB - ModelBase->PhysicsRigidBody ) ) ;
				}

				F1PhysicsJoint->Position = PhysicsJoint->Position ;
				F1PhysicsJoint->Rotation = PhysicsJoint->Rotation ;
				F1PhysicsJoint->ConstrainPosition1 = PhysicsJoint->ConstrainPosition1 ;
				F1PhysicsJoint->ConstrainPosition2 = PhysicsJoint->ConstrainPosition2 ;
				F1PhysicsJoint->ConstrainRotation1 = PhysicsJoint->ConstrainRotation1 ;
				F1PhysicsJoint->ConstrainRotation2 = PhysicsJoint->ConstrainRotation2 ;
				F1PhysicsJoint->SpringPosition = PhysicsJoint->SpringPosition ;
				F1PhysicsJoint->SpringRotation = PhysicsJoint->SpringRotation ;

				F1PhysicsJoint->DimPrev = i == 0                                       ? 0 : F1FileHeadPhysics->Joint + sizeof( MV1_PHYSICS_JOINT_F1 ) * ( i - 1 ) ;
				F1PhysicsJoint->DimNext = i == ( DWORD )ModelBase->PhysicsJointNum - 1 ? 0 : F1FileHeadPhysics->Joint + sizeof( MV1_PHYSICS_JOINT_F1 ) * ( i + 1 ) ;
			}
		}

		// マテリアルの情報を埋める
		F1Material = ( MV1_MATERIAL_F1 * )( ( DWORD_PTR )FHeader->Material + ( DWORD_PTR )FHeader ) ;
		MaterialBase = ModelBase->Material ;
		Material = Model->Material ;
		F1MaterialToon = ( MV1_MATERIAL_TOON_F1 * )( ( DWORD_PTR )F1MaterialToonAddr + ( DWORD_PTR )FHeader ) ;
		for( i = 0 ; i < ( DWORD )ModelBase->MaterialNum ; i ++, F1Material ++, F1MaterialToon ++, MaterialBase ++, Material ++ )
		{
			F1Material->Index = ( int )i ;

			F1Material->Name = ( DWORD )FHeader->StringSize ;
			FHeader->StringSize += ConvString( ( const char * )MaterialBase->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1Material->Name ), DX_CODEPAGE_UTF8 ) ;
			FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

			F1Material->Diffuse = Material->Diffuse ;
			F1Material->Ambient = Material->Ambient ;
			F1Material->Specular = Material->Specular ;
			F1Material->Emissive = Material->Emissive ;
			F1Material->Power = Material->Power ;

			F1Material->UseAlphaTest = Material->UseAlphaTest ;
			F1Material->AlphaFunc = Material->AlphaFunc ;
			F1Material->AlphaRef = Material->AlphaRef ;

			F1Material->DiffuseLayerNum = Material->DiffuseLayerNum ;
			F1MaterialLayer = F1Material->DiffuseLayer ;
			MaterialLayer = Material->DiffuseLayer ;
			for( j = 0 ; j < ( DWORD )Material->DiffuseLayerNum ; j ++, F1MaterialLayer ++, MaterialLayer ++ )
			{
				F1MaterialLayer->BlendType = MaterialLayer->BlendType ;
				F1MaterialLayer->Texture = MaterialLayer->Texture ;
			}

			F1Material->SpecularLayerNum = Material->SpecularLayerNum ;
			F1MaterialLayer = F1Material->SpecularLayer ;
			MaterialLayer = Material->SpecularLayer ;
			for( j = 0 ; j < ( DWORD )Material->SpecularLayerNum ; j ++, F1MaterialLayer ++, MaterialLayer ++ )
			{
				F1MaterialLayer->BlendType = MaterialLayer->BlendType ;
				F1MaterialLayer->Texture = MaterialLayer->Texture ;
			}

			F1Material->NormalLayerNum = MaterialBase->NormalLayerNum ;
			F1MaterialLayer = F1Material->NormalLayer ;
			MaterialLayer = MaterialBase->NormalLayer ;
			for( j = 0 ; j < ( DWORD )MaterialBase->NormalLayerNum ; j ++, F1MaterialLayer ++, MaterialLayer ++ )
			{
				F1MaterialLayer->BlendType = MaterialLayer->BlendType ;
				F1MaterialLayer->Texture = MaterialLayer->Texture ;
			}

			F1Material->DrawBlendMode = Material->DrawBlendMode ;
			F1Material->DrawBlendParam = Material->DrawBlendParam ;

			F1Material->UserData[ 0 ] = MaterialBase->UserData[ 0 ] ;
			F1Material->UserData[ 1 ] = MaterialBase->UserData[ 1 ] ;
			F1Material->UserData[ 2 ] = MaterialBase->UserData[ 2 ] ;
			F1Material->UserData[ 3 ] = MaterialBase->UserData[ 3 ] ;

			F1Material->DimPrev = i == 0                                   ? 0 : FHeader->Material + sizeof( MV1_MATERIAL_F1 ) * ( i - 1 ) ;
			F1Material->DimNext = i == ( DWORD )ModelBase->MaterialNum - 1 ? 0 : FHeader->Material + sizeof( MV1_MATERIAL_F1 ) * ( i + 1 ) ;

			F1Material->ToonInfo = ( DWORD )( ( DWORD_PTR )F1MaterialToon - ( DWORD_PTR )FHeader ) ;

			F1MaterialToon->Type = ( WORD )MaterialBase->Type ;
			F1MaterialToon->DiffuseGradTexture = Material->DiffuseGradTexture ;
			F1MaterialToon->DiffuseGradBlendType = Material->DiffuseGradBlendType ;
			F1MaterialToon->SpecularGradTexture = Material->SpecularGradTexture ;
			F1MaterialToon->SpecularGradBlendType = Material->SpecularGradBlendType ;
			F1MaterialToon->OutLineWidth = Material->OutLineWidth ;
			F1MaterialToon->OutLineDotWidth = Material->OutLineDotWidth ;
			F1MaterialToon->OutLineColor = Material->OutLineColor ;

			if( MaterialBase->SphereMapTexture < 0 )
			{
				F1MaterialToon->EnableSphereMap = 0 ;
				F1MaterialToon->SphereMapTexture = 0 ;
				F1MaterialToon->SphereMapBlendType = 0 ;
			}
			else
			{
				F1MaterialToon->EnableSphereMap = 1 ;
				F1MaterialToon->SphereMapTexture = ( short )Material->SphereMapTexture ;
				F1MaterialToon->SphereMapBlendType = ( BYTE )Material->SphereMapBlendType ;
			}
		}

		// ライトの情報を埋める
		F1Light = ( MV1_LIGHT_F1 * )( ( DWORD_PTR )FHeader->Light + ( DWORD_PTR )FHeader ) ;
		Light = ModelBase->Light ;
		for( i = 0 ; i < ( DWORD )ModelBase->LightNum ; i ++, F1Light ++, Light ++ )
		{
			F1Light->Index = ( int )i ;

			F1Light->Name = ( DWORD )FHeader->StringSize ;
			FHeader->StringSize += ConvString( ( const char * )Light->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1Light->Name ), DX_CODEPAGE_UTF8 ) ;
			FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

			F1Light->FrameIndex = Light->FrameIndex ;
			F1Light->Type = Light->Type ;
			F1Light->Diffuse = Light->Diffuse ;
			F1Light->Specular = Light->Specular ;
			F1Light->Ambient = Light->Ambient ;
			F1Light->Range = Light->Range ;
			F1Light->Falloff = Light->Falloff ;
			F1Light->Attenuation0 = Light->Attenuation0 ;
			F1Light->Attenuation1 = Light->Attenuation1 ;
			F1Light->Attenuation2 = Light->Attenuation2 ;
			F1Light->Theta = Light->Theta ;
			F1Light->Phi = Light->Phi ;

			F1Light->UserData[ 0 ] = Light->UserData[ 0 ] ;
			F1Light->UserData[ 1 ] = Light->UserData[ 1 ] ;

			F1Light->DimPrev = i == 0                                ? 0 : FHeader->Light + sizeof( MV1_LIGHT_F1 ) * ( i - 1 ) ;
			F1Light->DimNext = i == ( DWORD )ModelBase->LightNum - 1 ? 0 : FHeader->Light + sizeof( MV1_LIGHT_F1 ) * ( i + 1 ) ;
		}

		// テクスチャの情報を埋める
		F1Texture = ( MV1_TEXTURE_F1 * )( ( DWORD_PTR )FHeader->Texture + ( DWORD_PTR )FHeader ) ;
		TextureBase = ModelBase->Texture ;
		Texture = Model->Texture ;
		for( i = 0 ; i < ( DWORD )ModelBase->TextureNum ; i ++, F1Texture ++, TextureBase ++, Texture ++ )
		{
			F1Texture->Index = ( int )i ;
		
			F1Texture->BumpImageFlag = Texture->BumpImageFlag ;
			F1Texture->BumpImageNextPixelLength = Texture->BumpImageNextPixelLength ;

			F1Texture->Name = ( DWORD )FHeader->StringSize ;
			FHeader->StringSize += ConvString( ( const char * )TextureBase->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1Texture->Name ), DX_CODEPAGE_UTF8 ) ;
			FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

			F1Texture->AddressModeU = Texture->AddressModeU ;
			F1Texture->AddressModeV = Texture->AddressModeV ;
			F1Texture->ScaleU = Texture->ScaleU ;
			F1Texture->ScaleV = Texture->ScaleV ;
			F1Texture->FilterMode = Texture->FilterMode ;
			F1Texture->Flag = 0 ;
			if( Texture->ReverseFlag )
			{
				F1Texture->Flag |= MV1_TEXTURE_FLAG_REVERSE ;
			}
			if( Texture->Bmp32AllZeroAlphaToXRGB8Flag )
			{
				F1Texture->Flag |= MV1_TEXTURE_FLAG_BMP32_ALL_ZERO_ALPHA_TO_XRGB8 ;
			}
			F1Texture->Flag |= MV1_TEXTURE_FLAG_VALID_SCALE_UV ;

			if( Texture->ColorFilePathW_ == NULL )
			{
				F1Texture->ColorFilePath = NULL ;
			}
			else
			{
				F1Texture->ColorFilePath = ( DWORD )FHeader->StringSize ;
				FHeader->StringSize += ConvString( ( const char * )Texture->ColorFilePathW_, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1Texture->ColorFilePath ), DX_CODEPAGE_UTF8 ) ;
				FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;
			}

			if( Texture->AlphaFilePathW_ == NULL )
			{
				F1Texture->AlphaFilePath = NULL ;
			}
			else
			{
				F1Texture->AlphaFilePath = ( DWORD )FHeader->StringSize ;
				FHeader->StringSize += ConvString( ( const char * )Texture->AlphaFilePathW_, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1Texture->AlphaFilePath ), DX_CODEPAGE_UTF8 ) ;
				FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;
			}

			F1Texture->UserData[ 0 ] = TextureBase->UserData[ 0 ] ;
			F1Texture->UserData[ 1 ] = TextureBase->UserData[ 1 ] ;

			F1Texture->DimPrev = i == 0                                  ? 0 : FHeader->Texture + sizeof( MV1_TEXTURE_F1 ) * ( i - 1 ) ;
			F1Texture->DimNext = i == ( DWORD )ModelBase->TextureNum - 1 ? 0 : FHeader->Texture + sizeof( MV1_TEXTURE_F1 ) * ( i + 1 ) ;
		}

		// メッシュの情報を埋める
		F1Mesh = ( MV1_MESH_F1 * )( ( DWORD_PTR )FHeader->Mesh + ( DWORD_PTR )FHeader ) ;
		Mesh = ModelBase->Mesh ;
		for( i = 0 ; i < ( DWORD )ModelBase->MeshNum ; i ++, F1Mesh ++, Mesh ++ )
		{
			F1Mesh->Index = ( int )i ;
			if( Mesh->Container )
			{
				F1Mesh->Container = ( DWORD )( DWORD_PTR )( ( MV1_FRAME_F1 * )( DWORD_PTR )FHeader->Frame + ( Mesh->Container - ModelBase->Frame ) ) ;
			}
			if( Mesh->Material )
			{
				F1Mesh->Material = ( DWORD )( DWORD_PTR )( ( MV1_MATERIAL_F1 * )( DWORD_PTR )FHeader->Material + ( Mesh->Material - ModelBase->Material ) ) ;
			}

			F1Mesh->ChangeInfo.Target   = Mesh->ChangeInfo.Target ;
			F1Mesh->ChangeInfo.Fill     = NULL ;
			F1Mesh->ChangeInfo.Size     = Mesh->ChangeInfo.Size ;
			F1Mesh->ChangeInfo.CheckBit = Mesh->ChangeInfo.CheckBit ;

			F1Mesh->NotOneDiffuseAlpha = ( BYTE )Mesh->NotOneDiffuseAlpha ;
			F1Mesh->Shape = ( BYTE )Mesh->Shape ;
			F1Mesh->UseVertexDiffuseColor = Mesh->UseVertexDiffuseColor ;
			F1Mesh->UseVertexSpecularColor = Mesh->UseVertexSpecularColor ;
			F1Mesh->TriangleListNum = Mesh->TriangleListNum ;
			if( Mesh->TriangleList )
			{
				F1Mesh->TriangleList = ( DWORD )( DWORD_PTR )( ( MV1_TRIANGLE_LIST_F1 * )( DWORD_PTR )FHeader->TriangleList + ( Mesh->TriangleList - ModelBase->TriangleList ) ) ;
			}
			F1Mesh->Visible = Mesh->Visible ;
			F1Mesh->BackCulling = Mesh->BackCulling ;

			F1Mesh->UserData[ 0 ] = Mesh->UserData[ 0 ] ;
			F1Mesh->UserData[ 1 ] = Mesh->UserData[ 1 ] ;
			F1Mesh->UserData[ 2 ] = Mesh->UserData[ 2 ] ;
			F1Mesh->UserData[ 3 ] = Mesh->UserData[ 3 ] ;

			F1Mesh->UVSetUnitNum = ( BYTE )Mesh->UVSetUnitNum ;
			F1Mesh->UVUnitNum = ( BYTE )Mesh->UVUnitNum ;

			F1Mesh->VertexNum = Mesh->VertexNum ;
			F1Mesh->FaceNum = Mesh->FaceNum ;

			// 頂点データの作成
			{
				MV1_MESH_VERTEX *Vert ;

				// アドレスのセット
				F1Mesh->VertexData = FHeader->VertexDataSize ;
				Dest = ( BYTE * )VertexBuffer + FHeader->VertexDataSize ;
				Start = Dest ;

				// フラグを初期化
				F1Mesh->VertFlag = 0 ;

				// トゥーン用のエッジがあるかないか情報を出力するフラグをセット
				F1Mesh->VertFlag |= MV1_MESH_VERT_FLAG_NON_TOON_OUTLINE ;

				// 座標インデックスに何ビット必要か調べる
				if( Mesh->Container->PositionNum < 256 )
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_INDEX_TYPE_U8 ;
				}
				else
				if( Mesh->Container->PositionNum < 65536 )
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_INDEX_TYPE_U16 ;
				}
				else
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_INDEX_TYPE_U32 ;
				}

				// 法線インデックスに何ビット必要か調べる
				if( Mesh->Container->NormalNum < 256 )
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_INDEX_TYPE_U8 << 2 ;
				}
				else
				if( Mesh->Container->NormalNum < 65536 )
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_INDEX_TYPE_U16 << 2 ;
				}
				else
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_INDEX_TYPE_U32 << 2 ;
				}

				// UV値が 0.0～1.0 の範囲内か調べる
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					for( k = 0 ; k < ( DWORD )Mesh->UVSetUnitNum ; k ++ )
					{
						if( Vert->UVs[ k ][ 0 ] < 0.0f || Vert->UVs[ k ][ 0 ] > 1.0f ||
							Vert->UVs[ k ][ 1 ] < 0.0f || Vert->UVs[ k ][ 1 ] > 1.0f )
							break ;
					}
					if( k != ( DWORD )Mesh->UVSetUnitNum )
						break ;
				}
				if( j == ( DWORD )F1Mesh->VertexNum )
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_FLAG_UV_U16 ;
				}

				// 頂点色が一色かどうかを調べる
				Vert = Mesh->Vertex ;
				for( j = 0 ; j < ( DWORD )Mesh->VertexNum - 1 ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
				{
					if( *( ( DWORD * )&Vert->DiffuseColor  ) != *( ( DWORD * )&( ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )->DiffuseColor  ) ||
						*( ( DWORD * )&Vert->SpecularColor ) != *( ( DWORD * )&( ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )->SpecularColor ) )
						break ;
				}
				if( Mesh->VertexNum == 1 || j == ( DWORD )Mesh->VertexNum - 1 )
				{
					F1Mesh->VertFlag |= MV1_MESH_VERT_FLAG_COMMON_COLOR ;

					( ( COLOR_U8 * )Dest )[ 0 ] = Mesh->Vertex->DiffuseColor ;
					( ( COLOR_U8 * )Dest )[ 1 ] = Mesh->Vertex->SpecularColor ;
					Dest += 8 ;
				}

				// 頂点インデックスの格納
				Vert = Mesh->Vertex ;
				switch( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_POS_IND_TYPE_MASK )
				{
				case MV1_MESH_VERT_INDEX_TYPE_U8 :
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						*Dest = ( BYTE )Vert->PositionIndex ;
						Dest ++ ;
					}
					break ;

				case MV1_MESH_VERT_INDEX_TYPE_U16 :
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						*( ( WORD * )Dest ) = ( WORD )Vert->PositionIndex ;
						Dest += 2 ;
					}
					break ;

				case MV1_MESH_VERT_INDEX_TYPE_U32 :
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						*( ( DWORD * )Dest ) = ( DWORD )Vert->PositionIndex ;
						Dest += 4 ;
					}
					break ;
				}

				// 法線インデックスの格納
				Vert = Mesh->Vertex ;
				switch( ( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_NRM_IND_TYPE_MASK ) >> 2 )
				{
				case MV1_MESH_VERT_INDEX_TYPE_NONE :
					break ;

				case MV1_MESH_VERT_INDEX_TYPE_U8 :
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						*Dest = ( BYTE )Vert->NormalIndex ;
						Dest ++ ;
					}
					break ;

				case MV1_MESH_VERT_INDEX_TYPE_U16 :
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						*( ( WORD * )Dest ) = ( WORD )Vert->NormalIndex ;
						Dest += 2 ;
					}
					break ;

				case MV1_MESH_VERT_INDEX_TYPE_U32 :
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						*( ( DWORD * )Dest ) = ( DWORD )Vert->NormalIndex ;
						Dest += 4 ;
					}
					break ;
				}

				// 頂点カラーの格納
				if( ( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_COMMON_COLOR ) == 0 )
				{
					Vert = Mesh->Vertex ;
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						( ( COLOR_U8 * )Dest )[ 0 ] = Vert->DiffuseColor ;
						( ( COLOR_U8 * )Dest )[ 1 ] = Vert->SpecularColor ;
						Dest += 8 ;
					}
				}

				// ＵＶ値の格納
				Vert = Mesh->Vertex ;
				if( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_UV_U16 )
				{
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						for( k = 0 ; k < ( DWORD )Mesh->UVSetUnitNum ; k ++ )
						{
							( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( Vert->UVs[ k ][ 0 ] * 65535 ) ;
							( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( Vert->UVs[ k ][ 1 ] * 65535 ) ;
							Dest += 4 ;
						}
					}
				}
				else
				{
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						for( k = 0 ; k < ( DWORD )Mesh->UVSetUnitNum ; k ++ )
						{
							( ( float * )Dest )[ 0 ] = Vert->UVs[ k ][ 0 ] ;
							( ( float * )Dest )[ 1 ] = Vert->UVs[ k ][ 1 ] ;
							Dest += 8 ;
						}
					}
				}

				// トゥーン用の輪郭線を表示するかどうかのフラグを出力する
				if( F1Mesh->VertFlag & MV1_MESH_VERT_FLAG_NON_TOON_OUTLINE )
				{
					DWORD BitCount ;
					BYTE Out ;

					Vert = Mesh->Vertex ;
					Out = 0 ;
					BitCount = 0 ;
					for( j = 0 ; j < ( DWORD )Mesh->VertexNum ; j ++, Vert = ( MV1_MESH_VERTEX * )( ( BYTE * )Vert + Mesh->VertUnitSize ) )
					{
						Out |= ( Vert->ToonOutLineScale > 0.0f ? 0 : 1 ) << BitCount ;
						BitCount ++ ;
						if( BitCount == 8 )
						{
							*Dest = Out ;
							Dest ++ ;
							BitCount = 0 ;
							Out = 0 ;
						}
					}
					if( BitCount != 0 )
					{
						*Dest = Out ;
						Dest ++ ;
						BitCount = 0 ;
						Out = 0 ;
					}
				}

				Size = ( DWORD )( Dest - Start ) ;
				Size = ( Size + 3 ) / 4 * 4 ;
				FHeader->VertexDataSize += Size ;
			}

			F1Mesh->DimPrev = i == 0                               ? 0 : FHeader->Mesh + sizeof( MV1_MESH_F1 ) * ( i - 1 ) ;
			F1Mesh->DimNext = i == ( DWORD )ModelBase->MeshNum - 1 ? 0 : FHeader->Mesh + sizeof( MV1_MESH_F1 ) * ( i + 1 ) ;
		}

		// スキニングメッシュ用のボーン情報を埋める
		F1SkinBone = ( MV1_SKIN_BONE_F1 * )( ( DWORD_PTR )FHeader->SkinBone + ( DWORD_PTR )FHeader ) ;
		SkinBone = ModelBase->SkinBone ;
		for( i = 0 ; i < ( DWORD )ModelBase->SkinBoneNum ; i ++, F1SkinBone ++, SkinBone ++ )
		{
			F1SkinBone->Index = ( int )i ;
			F1SkinBone->BoneFrame = SkinBone->BoneFrame ;
			F1SkinBone->ModelLocalMatrix = SkinBone->ModelLocalMatrix ;
			F1SkinBone->ModelLocalMatrixIsTranslateOnly = SkinBone->ModelLocalMatrixIsTranslateOnly ;
			F1SkinBone->UseFrameNum = SkinBone->UseFrameNum ;
			if( F1SkinBone->UseFrame )
			{
				F1SkinBone->UseFrame = ( DWORD )( DWORD_PTR )( ( MV1_SKIN_BONE_USE_FRAME_F1 ** )( DWORD_PTR )FHeader->SkinBoneUseFrame + ( SkinBone->UseFrame - ModelBase->SkinBoneUseFrame ) ) ;
			}

			F1SkinBone->DimPrev = i == 0                                   ? 0 : FHeader->SkinBone + sizeof( MV1_SKIN_BONE_F1 ) * ( i - 1 ) ;
			F1SkinBone->DimNext = i == ( DWORD )ModelBase->SkinBoneNum - 1 ? 0 : FHeader->SkinBone + sizeof( MV1_SKIN_BONE_F1 ) * ( i + 1 ) ;
		}

		// トライアングルリストの情報を埋める
		F1TriangleList = ( MV1_TRIANGLE_LIST_F1 * )( ( DWORD_PTR )FHeader->TriangleList + ( DWORD_PTR )FHeader ) ;
		TriangleList = ModelBase->TriangleList ;
		for( i = 0 ; i < ( DWORD )ModelBase->TriangleListNum ; i ++, F1TriangleList ++, TriangleList ++ )
		{
			F1TriangleList->Index = ( int )i ;

			if( TriangleList->Container )
			{
				F1TriangleList->Container = ( DWORD )( DWORD_PTR )( ( MV1_MESH_F1 * )( DWORD_PTR )FHeader->Mesh + ( TriangleList->Container - ModelBase->Mesh ) ) ;
			}
			F1TriangleList->VertexType = TriangleList->VertexType ;
			F1TriangleList->VertexNum = ( unsigned short )TriangleList->VertexNum ;
			F1TriangleList->IndexNum = TriangleList->IndexNum ;

			// 頂点データの作成
			{
				DWORD *MVInd ;
				WORD *Ind ;

				// アドレスのセット
				F1TriangleList->MeshVertexIndexAndIndexData = FHeader->VertexDataSize ;
				Dest = ( BYTE * )VertexBuffer + FHeader->VertexDataSize ;
				Start = Dest ;

				// フラグの初期化
				F1TriangleList->Flag = 0 ;

				// メッシュ頂点インデックス一つ辺りのビット数を調べる
				MVInd = TriangleList->MeshVertexIndex ;
				for( j = 0 ; j < ( DWORD )TriangleList->VertexNum ; j ++, MVInd ++ )
				{
					if( *MVInd > 65535 )
					{
						F1TriangleList->Flag = MV1_TRIANGLE_LIST_INDEX_TYPE_U32 ;
						break ;
					}
					else
					if( *MVInd > 255 )
					{
						if( F1TriangleList->Flag != MV1_TRIANGLE_LIST_INDEX_TYPE_U32 )
							F1TriangleList->Flag = MV1_TRIANGLE_LIST_INDEX_TYPE_U16 ;
					}
					else
					{
						if( F1TriangleList->Flag == 0 )
							F1TriangleList->Flag = MV1_TRIANGLE_LIST_INDEX_TYPE_U8 ;
					}
				}

				// 頂点インデックス一つ辺りのビット数を調べる
				Ind = TriangleList->Index ;
				for( j = 0 ; j < TriangleList->IndexNum && *Ind < 256 ; j ++, Ind ++ ){}
				F1TriangleList->Flag |= j == TriangleList->IndexNum ? ( MV1_TRIANGLE_LIST_INDEX_TYPE_U8 << 2 ) : ( MV1_TRIANGLE_LIST_INDEX_TYPE_U16 << 2 ) ;

				// ボーン情報の保存
				switch( TriangleList->VertexType )
				{
				case MV1_VERTEX_TYPE_SKIN_4BONE :
				case MV1_VERTEX_TYPE_SKIN_8BONE :
					*( ( WORD * )Dest ) = ( WORD )TriangleList->UseBoneNum ;
					Dest += 2 ;
					for( j = 0 ; j < ( DWORD )TriangleList->UseBoneNum ; j ++, Dest += 2 )
						*( ( WORD * )Dest ) = ( WORD )TriangleList->UseBone[ j ] ;
					break ;

				case MV1_VERTEX_TYPE_SKIN_FREEBONE :
					*( ( WORD * )Dest ) = ( WORD )TriangleList->MaxBoneNum ;
					Dest += 2 ;
					break ;
				}

				// メッシュ頂点インデックスの格納
				MVInd = TriangleList->MeshVertexIndex ;
				switch( F1TriangleList->Flag & MV1_TRIANGLE_LIST_FLAG_MVERT_INDEX_MASK )
				{
				case MV1_TRIANGLE_LIST_INDEX_TYPE_U8 :
					for( j = 0 ; j < ( DWORD )TriangleList->VertexNum ; j ++, MVInd ++ )
					{
						*Dest = ( BYTE )*MVInd ;
						Dest ++ ;
					}
					break ;

				case MV1_TRIANGLE_LIST_INDEX_TYPE_U16 :
					for( j = 0 ; j < ( DWORD )TriangleList->VertexNum ; j ++, MVInd ++ )
					{
						*( ( WORD * )Dest ) = ( WORD )*MVInd ;
						Dest += 2 ;
					}
					break ;

				case MV1_TRIANGLE_LIST_INDEX_TYPE_U32 :
					for( j = 0 ; j < ( DWORD )TriangleList->VertexNum ; j ++, MVInd ++ )
					{
						*( ( DWORD * )Dest ) = ( DWORD )*MVInd ;
						Dest += 4 ;
					}
					break ;
				}

				// 頂点インデックスの格納
				Ind = TriangleList->Index ;
				switch( ( F1TriangleList->Flag & MV1_TRIANGLE_LIST_FLAG_INDEX_MASK ) >> 2 )
				{
				case MV1_TRIANGLE_LIST_INDEX_TYPE_U8 :
					for( j = 0 ; j < TriangleList->IndexNum ; j ++, Ind ++ )
					{
						*Dest = ( BYTE )*Ind ;
						Dest ++ ;
					}
					break ;

				case MV1_TRIANGLE_LIST_INDEX_TYPE_U16 :
					for( j = 0 ; j < TriangleList->IndexNum ; j ++, Ind ++ )
					{
						*( ( WORD * )Dest ) = ( WORD )*Ind ;
						Dest += 2 ;
					}
					break ;
				}

				Size = ( DWORD )( Dest - Start ) ;
				Size = ( Size + 3 ) / 4 * 4 ;
				FHeader->VertexDataSize += Size ;
			}

			F1TriangleList->DimPrev = i == 0                                       ? 0 : FHeader->TriangleList + sizeof( MV1_TRIANGLE_LIST_F1 ) * ( i - 1 ) ;
			F1TriangleList->DimNext = i == ( DWORD )ModelBase->TriangleListNum - 1 ? 0 : FHeader->TriangleList + sizeof( MV1_TRIANGLE_LIST_F1 ) * ( i + 1 ) ;
		}
	}

	if( AnimSave )
	{
		// アニメーションキーセットの情報を埋める
		F1AnimKeySet = ( MV1_ANIM_KEYSET_F1 * )( ( DWORD_PTR )FHeader->AnimKeySet + ( DWORD_PTR )FHeader ) ;
		AnimKeySet = AnimModelBase->AnimKeySet ;
		for( i = 0 ; i < ( DWORD )AnimModelBase->AnimKeySetNum ; i ++, F1AnimKeySet ++, AnimKeySet ++ )
		{
			MV1_ANIM_KEY_16BIT_F  Time16BSubF = { 0.0f } ;
			MV1_ANIM_KEY_16BIT_F  Key16BSubF = { 0.0f } ;
			MV1_ANIM_KEY_16BIT_F1 Time16BSub = { 0 } ;
			MV1_ANIM_KEY_16BIT_F1 Key16BSub = { 0 } ;
			int KeyBit16, TimeBit16, MPPP, ZTP ;
			VECTOR *Vector ;
			FLOAT4 *Quaternion ;
			float *Float ;
			float f ;

			F1AnimKeySet->UserData[ 0 ] = AnimKeySet->UserData[ 0 ] ;

			F1AnimKeySet->Type = AnimKeySet->Type ;
			F1AnimKeySet->DataType = AnimKeySet->DataType ;

			F1AnimKeySet->KeyData = ( DWORD )FHeader->AnimKeyDataSize ;
			Dest = ( BYTE * )AnimBuffer + ( DWORD_PTR )F1AnimKeySet->KeyData ;
			Start = Dest ;
			
			F1AnimKeySet->Flag = 0 ;
			KeyBit16 = 0 ;
			TimeBit16 = 0 ;
			MPPP = 0 ;
			ZTP = 0 ;

			// 回転キーの場合は処理を分岐
			if( AnimKeySet->DataType >= MV1_ANIMKEY_DATATYPE_ROTATE && AnimKeySet->DataType <= MV1_ANIMKEY_DATATYPE_ROTATE_Z )
			{
				// キーデータが回転で -DX_PI～DX_PI の間に値が収まっているかと、0～2*DX_PI の間に収まっているかどうかを調べる
				switch( AnimKeySet->Type )
				{
				case MV1_ANIMKEY_TYPE_VECTOR :
					MPPP = 1 ;
					ZTP = 1 ;
					Vector = AnimKeySet->KeyVector ;
					for( j = 0 ; j < ( DWORD )AnimKeySet->Num ; j ++, Vector ++ )
					{
						if( Vector->x < 0.0f         || Vector->x > 2 * DX_PI      ) ZTP  = 0 ;
						if( Vector->y < 0.0f         || Vector->y > 2 * DX_PI      ) ZTP  = 0 ;
						if( Vector->z < 0.0f         || Vector->z > 2 * DX_PI      ) ZTP  = 0 ;
						if( Vector->x < -( float )DX_PI || Vector->x > ( float )DX_PI ) MPPP = 0 ;
						if( Vector->y < -( float )DX_PI || Vector->y > ( float )DX_PI ) MPPP = 0 ;
						if( Vector->z < -( float )DX_PI || Vector->z > ( float )DX_PI ) MPPP = 0 ;
						if( MPPP == 0 && ZTP == 0 ) break ;
					}
					break ;

				case MV1_ANIMKEY_TYPE_LINEAR :
					MPPP = 1 ;
					ZTP = 1 ;
					Float = AnimKeySet->KeyLinear ;
					for( j = 0 ; j < ( DWORD )AnimKeySet->Num ; j ++, Float ++ )
					{
						if( *Float < 0.0f         || *Float > 2 * DX_PI      ) ZTP  = 0 ;
						if( *Float < -( float )DX_PI || *Float > ( float )DX_PI ) MPPP = 0 ;
						if( MPPP == 0 && ZTP == 0 ) break ;
					}
					break ;
				}
			}
			if( MPPP && ZTP )
			{
				MPPP = 0 ;
			}

			// 16ビット化キーが立っている場合は１６ビット化できるか調べる
			if( Anim16BitFlag )
			{
				// データが一つだけだった場合は処理を分岐
				if( AnimKeySet->Num == 1 )
				{
					// 補助キーが要らないデータ形式が可能な場合のみ１６ビット化
					if( MPPP || ZTP )
						KeyBit16 = 1 ;
				}
				else
				{
					// タイム値を１６ビット化できるか調べる
					if( AnimKeySet->TimeType == MV1_ANIMKEY_TIME_TYPE_KEY )
					{
						// 時間データ部分を固定小数点化
						Time16BSubF.Unit = 1.0f ;
						Time16BSubF.Min = 0.0f ;

						Float = AnimKeySet->KeyTime ;
						Time16BSubF.Min  = Float[ 0 ] ;
						Time16BSubF.Unit = Float[ 0 ] ;
						Float ++ ;
						for( k = 1 ; k < ( DWORD )AnimKeySet->Num ; k ++, Float ++ )
						{
							if( Time16BSubF.Min  > *Float ) Time16BSubF.Min  = *Float ;
							if( Time16BSubF.Unit < *Float ) Time16BSubF.Unit = *Float ;
						}

						// 16bit補助情報の作成
						Time16BSub.Min    = MV1AnimKey16BitMinFtoB( Time16BSubF.Min  ) ;
						Time16BSubF.Min   = MV1AnimKey16BitMinBtoF( Time16BSub.Min ) ;
						Time16BSubF.Unit -= Time16BSubF.Min ;
						if( Time16BSubF.Unit < 0.00000001f )
							Time16BSubF.Unit = 1.0f ;
						Time16BSubF.Unit /= 60000.0f ;
						Time16BSub.Unit   = MV1AnimKey16BitUnitFtoB( Time16BSubF.Unit ) ;
						Time16BSubF.Unit  = MV1AnimKey16BitUnitBtoF( Time16BSub.Unit ) ;

						TimeBit16 = 1 ;
					}

					// キーを１６ビット化できるか調べる
					if( MPPP || ZTP )
					{
						KeyBit16 = 1 ;
					}
					else
					{
						switch( AnimKeySet->Type )
						{
						case MV1_ANIMKEY_TYPE_LINEAR :
							{
								float *Key ;

								// 最小値と最大値を取得
								Key16BSubF.Min  =  1000000000000.0f ;
								Key16BSubF.Unit = -1000000000000.0f ;
								Key = AnimKeySet->KeyLinear ;
								for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Key ++ )
								{
									 if( Key16BSubF.Min  > *Key ) Key16BSubF.Min  = *Key ;
									 if( Key16BSubF.Unit < *Key ) Key16BSubF.Unit = *Key ;
								}
								KeyBit16 = 1 ;
							}
							break ;

						case MV1_ANIMKEY_TYPE_VECTOR :
							{
								VECTOR *Key ;

								// 最小値と最大値を取得
								Key16BSubF.Min  =  1000000000000.0f ;
								Key16BSubF.Unit = -1000000000000.0f ;
								Key = AnimKeySet->KeyVector ;
								for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Key ++ )
								{
									 if( Key16BSubF.Min  > Key->x ) Key16BSubF.Min  = Key->x ;
									 if( Key16BSubF.Unit < Key->x ) Key16BSubF.Unit = Key->x ;
									 if( Key16BSubF.Min  > Key->y ) Key16BSubF.Min  = Key->y ;
									 if( Key16BSubF.Unit < Key->y ) Key16BSubF.Unit = Key->y ;
									 if( Key16BSubF.Min  > Key->z ) Key16BSubF.Min  = Key->z ;
									 if( Key16BSubF.Unit < Key->z ) Key16BSubF.Unit = Key->z ;
								}
								KeyBit16 = 1 ;
							}
							break ;

						case MV1_ANIMKEY_TYPE_QUATERNION_X :
						case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
							{
								FLOAT4 *Key ;

								// 最小値と最大値を取得
								Key16BSubF.Min  =  1000000000000.0f ;
								Key16BSubF.Unit = -1000000000000.0f ;
								Key = AnimKeySet->KeyFloat4 ;
								for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Key ++ )
								{
									if( Key16BSubF.Min  > Key->x ) Key16BSubF.Min  = Key->x ;
									if( Key16BSubF.Unit < Key->x ) Key16BSubF.Unit = Key->x ;
									if( Key16BSubF.Min  > Key->y ) Key16BSubF.Min  = Key->y ;
									if( Key16BSubF.Unit < Key->y ) Key16BSubF.Unit = Key->y ;
									if( Key16BSubF.Min  > Key->z ) Key16BSubF.Min  = Key->z ;
									if( Key16BSubF.Unit < Key->z ) Key16BSubF.Unit = Key->z ;
									if( Key16BSubF.Min  > Key->w ) Key16BSubF.Min  = Key->w ;
									if( Key16BSubF.Unit < Key->w ) Key16BSubF.Unit = Key->w ;
								}
								KeyBit16 = 1 ;
							}
							break ;
						}
						if( KeyBit16 )
						{
							// 16bit補助情報の作成
							Key16BSub.Min    = MV1AnimKey16BitMinFtoB( Key16BSubF.Min  ) ;
							Key16BSubF.Min   = MV1AnimKey16BitMinBtoF( Key16BSub.Min ) ;
							Key16BSubF.Unit -= Key16BSubF.Min ;
							if( Key16BSubF.Unit < 0.00000001f )
								Key16BSubF.Unit = 1.0f ;
							Key16BSubF.Unit /= 60000.0f ;
							Key16BSub.Unit   = MV1AnimKey16BitUnitFtoB( Key16BSubF.Unit ) ;
							Key16BSubF.Unit  = MV1AnimKey16BitUnitBtoF( Key16BSub.Unit ) ;
						}
					}
				}
			}

			// キータイプがシェイプの場合は対象のシェイプインデックスを保存する
			if( AnimKeySet->DataType == MV1_ANIMKEY_DATATYPE_SHAPE )
			{
				*( ( WORD * )Dest ) = ( WORD )AnimKeySet->TargetShapeIndex ;
				Dest += 2 ;
			}

			// キーが一つかどうかで処理を分岐
			if( AnimKeySet->Num == 1 )
			{
				F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_KEY_ONE ;
			}
			else
			{
				// キーの数を保存
				if( AnimKeySet->Num < 256 )
				{
					F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_KEYNUM_B ;
					*Dest = ( BYTE )AnimKeySet->Num ;
					Dest ++ ;
				}
				else
				if( AnimKeySet->Num < 65536 )
				{
					F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_KEYNUM_W ;
					*( ( WORD * )Dest ) = ( WORD )AnimKeySet->Num ;
					Dest += 2 ;
				}
				else
				{
					*( ( DWORD * )Dest ) = ( DWORD )AnimKeySet->Num ;
					Dest += 4 ;
				}

				// タイム値が一定間隔かどうかで処理を分岐
				if( AnimKeySet->TimeType == MV1_ANIMKEY_TIME_TYPE_ONE )
				{
					F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_TIME_UNIT ;

					// 開始値が０かどうか調べる
					if( AnimKeySet->StartTime > -0.000001f && AnimKeySet->StartTime < 0.000001f )
					{
						// ゼロの場合は値を出力しない
						F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_TIME_UNIT_ST_Z ;
					}
					else
					{
						// 値が整数値で 65535 以下かどうかを調べる
						f = AnimKeySet->StartTime - _FTOL( AnimKeySet->StartTime ) ;
						if( f > -0.000001f && f < 0.000001f && AnimKeySet->StartTime >= 0.0f && AnimKeySet->StartTime <= 65535.0f )
						{
							// 開始値をWORD値で保存
							F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_TIME_UNIT_ST_W ;
							*( ( WORD * )Dest ) = ( WORD )_FTOL( AnimKeySet->StartTime ) ;
							Dest += 2 ;
						}
						else
						{
							// 開始値をfloat値で保存
							*( ( float * )Dest ) = AnimKeySet->StartTime ;
							Dest += 4 ;
						}
					}

					// 間隔値が整数値で 65535 以下かどうかを調べる
					f = AnimKeySet->UnitTime - _FTOL( AnimKeySet->UnitTime ) ;
					if( f > -0.000001f && f < 0.000001f && AnimKeySet->UnitTime >= 0.0f && AnimKeySet->UnitTime <= 65535.0f )
					{
						// 間隔値をWORD値で保存
						F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_TIME_UNIT_UN_W ;
						*( ( WORD * )Dest ) = ( WORD )_FTOL( AnimKeySet->UnitTime ) ;
						Dest += 2 ;
					}
					else
					{
						// 間隔値をfloat値で保存
						*( ( float * )Dest ) = AnimKeySet->UnitTime ;
						Dest += 4 ;
					}
				}
				else
				// 16bit化する場合は補助情報を保存
				if( TimeBit16 )
				{
					F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_TIME_BIT16 ;

					*( ( MV1_ANIM_KEY_16BIT_F1 * )Dest ) = Time16BSub ;
					Dest += 2 ;

					// 値を変換
					Float = AnimKeySet->KeyTime ;
					for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Float ++ )
					{
						*( ( WORD * )Dest ) = ( WORD )_FTOL( ( *Float - Time16BSubF.Min ) / Time16BSubF.Unit ) ;
						Dest += 2 ;
					}
				}
				else
				{
					_MEMCPY( Dest, AnimKeySet->KeyTime, AnimKeySet->Num * sizeof( float ) ) ;
					Dest += AnimKeySet->Num * sizeof( float ) ;
				}

				// 16bit化補助情報が必要な場合は保存
				if( MPPP == 0 && ZTP == 0 && KeyBit16 )
				{
					*( ( MV1_ANIM_KEY_16BIT_F1 * )Dest ) = Key16BSub ;
					Dest += 2 ;
				}
			}

			// キーの値を保存
			if( KeyBit16 == 0 )
			{
				switch( AnimKeySet->Type )
				{
				case MV1_ANIMKEY_TYPE_LINEAR :
					_MEMCPY( Dest, AnimKeySet->KeyLinear, AnimKeySet->Num * sizeof( float ) ) ;
					Dest += AnimKeySet->Num * sizeof( float ) ;
					break ;

				case MV1_ANIMKEY_TYPE_VECTOR :
					_MEMCPY( Dest, AnimKeySet->KeyVector, AnimKeySet->Num * sizeof( VECTOR ) ) ;
					Dest += AnimKeySet->Num * sizeof( VECTOR ) ;
					break ;

				case MV1_ANIMKEY_TYPE_QUATERNION_X :
				case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
					_MEMCPY( Dest, AnimKeySet->KeyFloat4, AnimKeySet->Num * sizeof( FLOAT4 ) ) ;
					Dest += AnimKeySet->Num * sizeof( FLOAT4 ) ;
					break ;

				case MV1_ANIMKEY_TYPE_MATRIX4X4C :
					_MEMCPY( Dest, AnimKeySet->KeyMatrix4x4C, AnimKeySet->Num * sizeof( MATRIX_4X4CT_F ) ) ;
					Dest += AnimKeySet->Num * sizeof( MATRIX_4X4CT_F ) ;
					break ;
				}
			}
			else
			{
				float f ;

				F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_KEY_BIT16 ;
				if( MPPP ) 
				{
					F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_KEY_MP_PP ;
				}
				if( ZTP )
				{
					F1AnimKeySet->Flag |= MV1_ANIM_KEYSET_FLAG_KEY_Z_TP ;
				}

				switch( AnimKeySet->Type )
				{
				case MV1_ANIMKEY_TYPE_LINEAR :
					if( MPPP )
					{
						Float = AnimKeySet->KeyLinear ;
						for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Float ++ )
						{
							*( ( WORD * )Dest ) = ( WORD )_FTOL( ( *Float + ( float )DX_PI ) * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = *Float - ( *( ( WORD * )Dest ) / 65535.0f * ( float )( DX_PI * 2.0f ) - ( float )DX_PI ) ;
							if( f < -0.5f )
							{
								*( ( WORD * )Dest ) = 0 ;
							}
							else
							if( f > 0.5f )
							{
								*( ( WORD * )Dest ) = 65535 ;
							}
							Dest += 2 ;
						}
					}
					else
					if( ZTP )
					{
						Float = AnimKeySet->KeyLinear ;
						for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Float ++ )
						{
							*( ( WORD * )Dest ) = ( WORD )_FTOL( *Float * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = *Float - ( *( ( WORD * )Dest ) / 65535.0f * ( float )( DX_PI * 2.0f ) ) ;
							if( f < -0.5f )
							{
								*( ( WORD * )Dest ) = 0 ;
							}
							else
							if( f > 0.5f )
							{
								*( ( WORD * )Dest ) = 65535 ;
							}
							Dest += 2 ;
						}
					}
					else
					{
						Float = AnimKeySet->KeyLinear ;
						for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Float ++ )
						{
							*( ( WORD * )Dest ) = ( WORD )_FTOL( ( *Float - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
							Dest += 2 ;
						}
					}
					break ;

				case MV1_ANIMKEY_TYPE_VECTOR :
					if( MPPP )
					{
						Vector = AnimKeySet->KeyVector ;
						for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Vector ++ )
						{
							( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( ( Vector->x + ( float )DX_PI ) * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = Vector->x - ( ( ( WORD * )Dest )[ 0 ] / 65535.0f * ( float )( DX_PI * 2.0f ) - ( float )DX_PI ) ;
							if( f < -0.5f )
							{
								( ( WORD * )Dest )[ 0 ] = 0 ;
							}
							else
							if( f > 0.5f )
							{
								( ( WORD * )Dest )[ 0 ] = 65535 ;
							}

							( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( ( Vector->y + ( float )DX_PI ) * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = Vector->y - ( ( ( WORD * )Dest )[ 1 ] / 65535.0f * ( float )( DX_PI * 2.0f ) - ( float )DX_PI ) ;
							if( f < -0.5f )
							{
								( ( WORD * )Dest )[ 1 ] = 0 ;
							}
							else
							if( f > 0.5f )
							{
								( ( WORD * )Dest )[ 1 ] = 65535 ;
							}

							( ( WORD * )Dest )[ 2 ] = ( WORD )_FTOL( ( Vector->z + ( float )DX_PI ) * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = Vector->z - ( ( ( WORD * )Dest )[ 2 ] / 65535.0f * ( float )( DX_PI * 2.0f ) - ( float )DX_PI ) ;
							if( f < -0.5f )
							{
								( ( WORD * )Dest )[ 2 ] = 0 ;
							}
							else
							if( f > 0.5f )
							{
								( ( WORD * )Dest )[ 2 ] = 65535 ;
							}

							Dest += 6 ;
						}
					}
					else
					if( ZTP )
					{
						Vector = AnimKeySet->KeyVector ;
						for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Vector ++ )
						{
							( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( Vector->x * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = Vector->x - ( ( ( WORD * )Dest )[ 0 ] / 65535.0f * ( float )( DX_PI * 2.0f ) ) ;
							if( f < -0.5f )
							{
								( ( WORD * )Dest )[ 0 ] = 0 ;
							}
							else
							if( f > 0.5f )
							{
								( ( WORD * )Dest )[ 0 ] = 65535 ;
							}

							( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( Vector->y * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = Vector->y - ( ( ( WORD * )Dest )[ 1 ] / 65535.0f * ( float )( DX_PI * 2.0f ) ) ;
							if( f < -0.5f )
							{
								( ( WORD * )Dest )[ 1 ] = 0 ;
							}
							else
							if( f > 0.5f )
							{
								( ( WORD * )Dest )[ 1 ] = 65535 ;
							}

							( ( WORD * )Dest )[ 2 ] = ( WORD )_FTOL( Vector->z * 65535.0f / ( float )( DX_PI * 2.0f ) ) ;
							f = Vector->z - ( ( ( WORD * )Dest )[ 2 ] / 65535.0f * ( float )( DX_PI * 2.0f ) ) ;
							if( f < -0.5f )
							{
								( ( WORD * )Dest )[ 2 ] = 0 ;
							}
							else
							if( f > 0.5f )
							{
								( ( WORD * )Dest )[ 2 ] = 65535 ;
							}

							Dest += 6 ;
						}
					}
					else
					{
						Vector = AnimKeySet->KeyVector ;
						for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Vector ++ )
						{
							( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( ( Vector->x - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
							( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( ( Vector->y - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
							( ( WORD * )Dest )[ 2 ] = ( WORD )_FTOL( ( Vector->z - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
							Dest += 6 ;
						}
					}
					break ;

				case MV1_ANIMKEY_TYPE_QUATERNION_X :
				case MV1_ANIMKEY_TYPE_QUATERNION_VMD :
					Quaternion        = AnimKeySet->KeyFloat4 ;
					for( k = 0 ; k < ( DWORD )AnimKeySet->Num ; k ++, Quaternion ++ )
					{
						( ( WORD * )Dest )[ 0 ] = ( WORD )_FTOL( ( Quaternion->x - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
						( ( WORD * )Dest )[ 1 ] = ( WORD )_FTOL( ( Quaternion->y - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
						( ( WORD * )Dest )[ 2 ] = ( WORD )_FTOL( ( Quaternion->z - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
						( ( WORD * )Dest )[ 3 ] = ( WORD )_FTOL( ( Quaternion->w - Key16BSubF.Min ) / Key16BSubF.Unit ) ;
						Dest += 8 ;
					}
					break ;
				}
			}
			FHeader->AnimKeyDataSize += ( int )( Dest - Start ) ;
			FHeader->AnimKeyDataSize = ( FHeader->AnimKeyDataSize + 3 ) / 4 * 4 ;
		}

		// アニメーションセットとアニメーションの情報を埋める
		F1AnimSet = ( MV1_ANIMSET_F1 * )( ( DWORD_PTR )FHeader->AnimSet + ( DWORD_PTR )FHeader ) ;
		AnimSet = AnimModelBase->AnimSet ;
		for( i = 0 ; i < ( DWORD )AnimModelBase->AnimSetNum ; i ++, F1AnimSet ++, AnimSet ++ )
		{
			F1AnimSet->Name = ( DWORD )FHeader->StringSize ;
			FHeader->StringSize += ConvString( ( const char * )AnimSet->NameW, WCHAR_T_CODEPAGE, ( char * )( ( BYTE * )FHeader + FHeader->StringBuffer + F1AnimSet->Name ), DX_CODEPAGE_UTF8 ) ;
			FHeader->StringSize = ( FHeader->StringSize + 3 ) / 4 * 4 ;

			F1AnimSet->Index = AnimSet->Index ;
			F1AnimSet->MaxTime = AnimSet->MaxTime ;
			F1AnimSet->AnimNum = AnimSet->AnimNum ;
			if( AnimSet->Anim )
			{
				F1AnimSet->Anim = ( DWORD )( DWORD_PTR )( ( MV1_ANIM_F1 * )( DWORD_PTR )FHeader->Anim + ( AnimSet->Anim - AnimModelBase->Anim ) ) ;
			}

			if( AnimSet->IsAddAnim )           F1AnimSet->Flag |= 1 ;
			if( AnimSet->IsMatrixLinearBlend ) F1AnimSet->Flag |= 2 ;

			F1AnimSet->UserData[ 0 ] = AnimSet->UserData[ 0 ] ;
			F1AnimSet->UserData[ 1 ] = AnimSet->UserData[ 1 ] ;
			F1AnimSet->UserData[ 2 ] = AnimSet->UserData[ 2 ] ;
			F1AnimSet->UserData[ 3 ] = AnimSet->UserData[ 3 ] ;

			F1AnimSet->DimPrev = i == 0                                      ? 0 : FHeader->AnimSet + sizeof( MV1_ANIMSET_F1 ) * ( i - 1 ) ;
			F1AnimSet->DimNext = i == ( DWORD )AnimModelBase->AnimSetNum - 1 ? 0 : FHeader->AnimSet + sizeof( MV1_ANIMSET_F1 ) * ( i + 1 ) ;

			if( AnimMHandle >= 0 )
			{
				AttachIndex = NS_MV1AttachAnim( MHandle, ( int )i, AnimMHandle, AnimNameCheck ) ;
				if( AttachIndex == -1 )  goto ERRORLABEL ;
			}

			F1Anim = ( MV1_ANIM_F1 * )( ( DWORD_PTR )FHeader->Anim + ( DWORD_PTR )FHeader ) + ( AnimSet->Anim - AnimModelBase->Anim ) ;
			Anim = AnimSet->Anim ;
			for( j = 0 ; j < ( DWORD )AnimSet->AnimNum ; j ++, F1Anim ++, Anim ++ )
			{
				F1Anim->Index = ( int )( Anim - AnimModelBase->Anim ) ;

				if( Anim->Container )
				{
					F1Anim->Container = ( DWORD )( DWORD_PTR )( ( MV1_ANIMSET_F1 * )( DWORD_PTR )FHeader->AnimSet + ( Anim->Container - AnimModelBase->AnimSet ) ) ;
				}
				if( AnimMHandle < 0 )
				{
					F1Anim->TargetFrameIndex = Anim->TargetFrameIndex ;
				}
				else
				{
					MV1_MODEL_ANIM *MAnim ;

					for( k = 0 ; k < ( DWORD )ModelBase->FrameNum ; k ++ )
					{
						MAnim = &Model->Anim[ AttachIndex + Model->AnimSetMaxNum * k ] ;
						if( MAnim->Use == false ) continue ;
						if( MAnim->Anim->BaseData == Anim ) break ;
					}
					if( k == ( DWORD )ModelBase->FrameNum )
					{
						NS_MV1DetachAnim( MHandle, AttachIndex ) ;
						Err = -2 ;
						goto ERRORLABEL ;
					}
					else
					{
						F1Anim->TargetFrameIndex = ( int )k ;
					}
				}
				F1Anim->MaxTime = Anim->MaxTime ;
				F1Anim->RotateOrder = Anim->RotateOrder ;
				F1Anim->KeySetNum = Anim->KeySetNum ;
				F1Anim->KeySet = ( DWORD )( ( DWORD_PTR )( ( MV1_ANIM_KEYSET_F1 * )( DWORD_PTR )FHeader->AnimKeySet + ( Anim->KeySet - AnimModelBase->AnimKeySet ) ) ) ;

				F1Anim->UserData[ 0 ] = Anim->UserData[ 0 ] ;
				F1Anim->UserData[ 1 ] = Anim->UserData[ 1 ] ;
			}

			if( AnimMHandle >= 0 )
			{
				NS_MV1DetachAnim( MHandle, AttachIndex ) ;
				AttachIndex = 0 ;
			}
		}
	}

	// 文字列データのサイズを１６の倍数に合わせる
	FHeader->StringSize = ( FHeader->StringSize + 15 ) / 16 * 16 ;
	TempBufferUseSize += FHeader->StringSize ;

	// 頂点データをファイルの末端に配置する
	if( MeshSave )
	{
		if( TempBufferUseSize + FHeader->VertexDataSize > TempBufferSize )
		{
			VertexBufferSize += TempBufferUseSize + FHeader->VertexDataSize - TempBufferSize + 1024 * 1024 ;
			goto SAVELOOP ;
		}

		FHeader->VertexData = TempBufferUseSize ;
		_MEMCPY( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->VertexData, VertexBuffer, FHeader->VertexDataSize ) ;
		TempBufferUseSize += FHeader->VertexDataSize ;

		// 頂点データアドレスを補正
		F1Frame = ( MV1_FRAME_F1 * )( ( DWORD_PTR )FHeader->Frame + ( DWORD_PTR )FHeader ) ;
		for( i = 0 ; i < ( DWORD )ModelBase->FrameNum ; i ++, F1Frame ++ )
		{
			F1Frame->PositionAndNormalData = ( DWORD )( ( DWORD_PTR )F1Frame->PositionAndNormalData + ( DWORD_PTR )FHeader->VertexData ) ;
		}
		F1Mesh = ( MV1_MESH_F1 * )( ( DWORD_PTR )FHeader->Mesh + ( DWORD_PTR )FHeader ) ;
		for( i = 0 ; i < ( DWORD )ModelBase->MeshNum ; i ++, F1Mesh ++ )
		{
			F1Mesh->VertexData = ( DWORD )( ( DWORD_PTR )F1Mesh->VertexData + ( DWORD_PTR )FHeader->VertexData ) ;
		}
		F1TriangleList = ( MV1_TRIANGLE_LIST_F1 * )( ( DWORD_PTR )FHeader->TriangleList + ( DWORD_PTR )FHeader ) ;
		for( i = 0 ; i < ( DWORD )ModelBase->TriangleListNum ; i ++, F1TriangleList ++ )
		{
			F1TriangleList->MeshVertexIndexAndIndexData = ( DWORD )( ( DWORD_PTR )F1TriangleList->MeshVertexIndexAndIndexData + ( DWORD_PTR )FHeader->VertexData ) ;
		}
	}

	// マテリアル情報更新検出用テーブルをファイルの末端に配置する
	if( MeshSave == false )
	{
		DWORD *Table ;

		if( TempBufferUseSize + FHeader->ChangeDrawMaterialTableSize > TempBufferSize )
		{
			HeaderSize += TempBufferUseSize + FHeader->ChangeDrawMaterialTableSize - TempBufferSize + 1024 * 1024 ;
			goto SAVELOOP ;
		}

		Table = ( DWORD * )ChangeDrawMaterialTableBuffer ;
		FHeader->ChangeDrawMaterialTable = TempBufferUseSize ;
		_MEMCPY( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->ChangeDrawMaterialTable, Table, ( size_t )FHeader->ChangeDrawMaterialTableSize ) ;
		TempBufferUseSize += FHeader->ChangeDrawMaterialTableSize ;

		// 検出用テーブルのアドレスを補正
		F1Frame = ( MV1_FRAME_F1 * )( ( DWORD_PTR )FHeader->Frame + ( DWORD_PTR )FHeader ) ;
		for( i = 0 ; i < ( DWORD )ModelBase->FrameNum ; i ++, F1Frame ++ )
		{
			F1Frame->ChangeDrawMaterialInfo.Fill = ( DWORD )( ( DWORD_PTR )( ( DWORD * )( ( BYTE * )ChangeDrawMaterialTableBuffer + F1Frame->ChangeDrawMaterialInfo.Fill ) - Table ) + ( DWORD_PTR )FHeader->ChangeDrawMaterialTable ) ;
		}
	}

	// アニメーションキーデータをファイルの末端に配置する
	if( AnimSave )
	{
		if( TempBufferUseSize + FHeader->AnimKeyDataSize > TempBufferSize )
		{
			AnimBufferSize += TempBufferUseSize + FHeader->AnimKeyDataSize - TempBufferSize + 1024 * 1024 ;
			goto SAVELOOP ;
		}

		FHeader->AnimKeyData = TempBufferUseSize ;
		_MEMCPY( ( BYTE * )FHeader + ( DWORD_PTR )FHeader->AnimKeyData, AnimBuffer, ( size_t )FHeader->AnimKeyDataSize ) ;
		TempBufferUseSize += FHeader->AnimKeyDataSize ;

		// アニメーションキーセットのキーデータアドレスを補正
		F1AnimKeySet = ( MV1_ANIM_KEYSET_F1 * )( ( DWORD_PTR )FHeader->AnimKeySet + ( DWORD_PTR )FHeader ) ;
		for( i = 0 ; i < ( DWORD )AnimModelBase->AnimKeySetNum ; i ++, F1AnimKeySet ++ )
		{
			F1AnimKeySet->KeyData = ( DWORD )( ( DWORD_PTR )F1AnimKeySet->KeyData + ( DWORD_PTR )FHeader->AnimKeyData ) ;
		}
	}

	// データを圧縮
	{
		if( TempBufferUseSize * 2 + TempBufferUseSize / 2 > TempBufferSize )
		{
			HeaderBufferSize += TempBufferUseSize * 2 + TempBufferUseSize / 2 - TempBufferSize + 1024 * 1024 ;
			goto SAVELOOP ;
		}

		PressData = ( BYTE * )TempBuffer + TempBufferUseSize ;
		( ( MV1MODEL_FILEHEADER_F1 * )PressData )->CheckID[ 0 ] = FHeader->CheckID[ 0 ] ;
		( ( MV1MODEL_FILEHEADER_F1 * )PressData )->CheckID[ 1 ] = FHeader->CheckID[ 1 ] ;
		( ( MV1MODEL_FILEHEADER_F1 * )PressData )->CheckID[ 2 ] = FHeader->CheckID[ 2 ] ;
		( ( MV1MODEL_FILEHEADER_F1 * )PressData )->CheckID[ 3 ] = FHeader->CheckID[ 3 ] ;
		PressDataSize = ( DWORD )DXA_Encode( ( BYTE * )FHeader + 4, ( DWORD )( TempBufferUseSize - 4 ), ( BYTE * )PressData + 4 ) ;
	}

	// ファイルに書き出す
	FileHandle = CreateFileW( FileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( FileHandle )
	{
		WriteFile( FileHandle, PressData, PressDataSize + 4, &WriteSize, NULL ) ;
		CloseHandle( FileHandle ) ;
	}

	// メモリの解放
	if( TempBuffer )
	{
		DXFREE( TempBuffer ) ;
		TempBuffer = NULL ;
	}

	if( AnimBuffer )
	{
		DXFREE( AnimBuffer ) ;
		AnimBuffer = NULL ;
	}

	if( VertexBuffer )
	{
		DXFREE( VertexBuffer ) ;
		VertexBuffer = NULL ;
	}

	if( ChangeDrawMaterialTableBuffer )
	{
		DXFREE( ChangeDrawMaterialTableBuffer ) ;
		ChangeDrawMaterialTableBuffer = NULL ;
	}

	// 終了
	return 0 ;

ERRORLABEL :
	if( TempBuffer )
	{
		DXFREE( TempBuffer ) ;
		TempBuffer = NULL ;
	}

	if( AnimBuffer )
	{
		DXFREE( AnimBuffer ) ;
		AnimBuffer = NULL ;
	}

	if( VertexBuffer )
	{
		DXFREE( VertexBuffer ) ;
		VertexBuffer = NULL ;
	}

	if( ChangeDrawMaterialTableBuffer )
	{
		DXFREE( ChangeDrawMaterialTableBuffer ) ;
		ChangeDrawMaterialTableBuffer = NULL ;
	}

	return Err ;
}

// スペースを _ に変更して返す
static const char *MV1SaveModelToXFileConvSpace( const char *String )
{
	static char TempChar[ 512 ] ;
	int i ;

	for( i = 0 ; String[ i ] ; i ++ )
	{
		if( String[ i ] == ' ' || String[ i ] == ':' )
		{
			TempChar[ i ] = '_' ;
		}
		else
		{
			TempChar[ i ] = String[ i ] ;
		}
	}
	TempChar[ i ] = '\0' ;

	return TempChar ;
}

// 指定数のタブを出力する
static	void MV1SaveModelToXFileOutputTab( FILE *fp, int TabNum )
{
	int i ;
	char Temp[ 512 ] ;

	for( i = 0 ; i < TabNum ; i ++ )
	{
		Temp[ i ] = '\t' ;
	}
	Temp[ i ] = '\0' ;
	fprintf( fp, Temp ) ;
}

#ifndef DX_NON_SAVEFUNCTION

// 指定のパスにモデルをＸファイル形式で保存する
extern int NS_MV1SaveModelToXFile( int MHandle, const TCHAR *FileName, int SaveType, int AnimMHandle, int AnimNameCheck )
{
#ifdef UNICODE
	return MV1SaveModelToXFile_WCHAR_T(
		MHandle, FileName, SaveType, AnimMHandle, AnimNameCheck
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FileName, return -1 )

	Result = MV1SaveModelToXFile_WCHAR_T(
		MHandle, UseFileNameBuffer, SaveType, AnimMHandle, AnimNameCheck
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FileName )

	return Result ;
#endif
}

// 指定のパスにモデルをＸファイル形式で保存する
extern int MV1SaveModelToXFile_WCHAR_T( int MHandle, const wchar_t *FileName, int SaveType, int AnimMHandle, int AnimNameCheck )
{
	MV1_MODEL *Model, *AnimModel ;
	MV1_MODEL_BASE *ModelBase, *AnimModelBase ;
	MV1_MATERIAL *Material ;
	MV1_MATERIAL_BASE *MaterialBase ;
	MV1_MESH_BASE *Mesh ;
	MV1_MESH_FACE *Face ;
	MV1_MESH_POSITION *Pos ;
	MV1_MESH_NORMAL *Nrm ;
	MV1_MESH_VERTEX *MVert ;
	MV1_SKIN_BONE *SkinB ;
	MV1_FRAME_BASE *Frame, *FrameStack[ 1024 ] ;
	MV1_ANIMSET_BASE *AnimSet ;
	MV1_ANIMSET *MotSet ;
	MV1_ANIM_BASE *Anim ;
	MV1_ANIM *Mot ;
	MV1_ANIM_KEYSET_BASE *KeySet ;
	int FrameStackStep[ 1024 ] ;
	int i, j, k, l, m, FrameStackNum, UseNum, FrameNum, VertexNum, VertexStartIndex ;
	int AnimIndex, AnimNum, Err = -1 ;
	int *KeyTiming = NULL, time, KeyTimingNum ;
	MATRIX_4X4CT_F *KeyMatrix, *Matrix ;
	FILE *fp = NULL ;
	MV1_SKINBONE_BLEND *UseBoneMap = NULL ;
	bool MeshSave, AnimSave ;
	char String[ 1024 ] ;

	MeshSave = ( SaveType & MV1_SAVETYPE_MESH ) ? true : false ;
	AnimSave = ( SaveType & MV1_SAVETYPE_ANIM ) ? true : false ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// アニメーション用のモデルハンドルのチェック
	if( MV1MDLCHK( AnimMHandle, AnimModel ) )
	{
		AnimModelBase = ModelBase ;
		AnimModel = Model ;
		AnimMHandle = -1 ;
	}
	else
	{
		AnimModelBase = AnimModel->BaseData ;
	}

	// ファイルを開く
	fp = _wfopen( FileName, L"wt" ) ;
	if( fp == NULL )
		return -1 ;

	// ヘッダの出力
	fprintf( fp, "xof 0303txt 0032\n" ) ;

	if( MeshSave )
	{
		// マテリアルの出力
		MaterialBase = ModelBase->Material ;
		Material = Model->Material ;
		for( i = 0 ; i < ModelBase->MaterialNum ; i ++, MaterialBase ++, Material ++ )
		{
			if( MaterialBase->NameW[ 0 ] == L'\0' )
			{
				fprintf( fp, "Material Material_%03d {\n", i ) ;
			}
			else
			{
				ConvString( ( const char * )MaterialBase->NameW, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
				fprintf( fp, "Material %s {\n", MV1SaveModelToXFileConvSpace( String ) ) ;
			}
			fprintf( fp, "\t%f;%f;%f;%f;;\n", Material->Diffuse.r, Material->Diffuse.g, Material->Diffuse.b, Material->DrawBlendParam / 255.0f ) ;
			fprintf( fp, "\t%f;\n", Material->Power ) ;
			fprintf( fp, "\t%f;%f;%f;;\n", Material->Specular.r, Material->Specular.g, Material->Specular.b ) ;
			fprintf( fp, "\t%f;%f;%f;;\n", Material->Emissive.r, Material->Emissive.g, Material->Emissive.b ) ;
			if( Material->DiffuseLayerNum )
			{
				MV1_TEXTURE *Texture ;

				Texture = &Model->Texture[ Material->DiffuseLayer[ 0 ].Texture ] ;

				ConvString( ( const char * )Texture->ColorFilePathW_, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
				fprintf( fp, "\tTextureFilename {\n\t\t\"%s\";\n\t}\n", String ) ;
			}
			fprintf( fp, "}\n" ) ;
		}

		// スキニングメッシュ用のメモリ領域と座標インデックス作成用のメモリ領域を確保
		UseBoneMap = ( MV1_SKINBONE_BLEND * )DXALLOC( ( ModelBase->TriangleNum * 3 ) * ( sizeof( VECTOR ) + sizeof( int ) ) ) ;
		if( UseBoneMap == NULL ) goto ERRORLABEL ;
	}

	// トップフレームから開始
	{
		Frame = Model->TopFrameList[ 0 ]->BaseData ;
		FrameStack[ 0 ] = Frame ;
		FrameStackStep[ 0 ] = 0 ;
		FrameStackNum = 1 ;

		while( FrameStackNum != 0 )
		{
			Frame = FrameStack[ FrameStackNum - 1 ] ;

			switch( FrameStackStep[ FrameStackNum - 1 ] )
			{
			case 0 :
				MV1SaveModelToXFileOutputTab( fp, FrameStackNum - 1 ) ;

				ConvString( ( const char * )Frame->NameW, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
				fprintf( fp, "Frame %s {\n", MV1SaveModelToXFileConvSpace( String ) ) ;

				// 行列の出力
				{
					MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
					fprintf( fp, "FrameTransformMatrix {\n" ) ;
					MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
					fprintf( fp, "\t%f,%f,%f,%f,\n",
						Frame->LocalTransformMatrix.m[0][0],
						Frame->LocalTransformMatrix.m[1][0],
						Frame->LocalTransformMatrix.m[2][0],
						0.0f ) ;
					MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
					fprintf( fp, "\t%f,%f,%f,%f,\n",
						Frame->LocalTransformMatrix.m[0][1],
						Frame->LocalTransformMatrix.m[1][1],
						Frame->LocalTransformMatrix.m[2][1],
						0.0f ) ;
					MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
					fprintf( fp, "\t%f,%f,%f,%f,\n",
						Frame->LocalTransformMatrix.m[0][2],
						Frame->LocalTransformMatrix.m[1][2],
						Frame->LocalTransformMatrix.m[2][2],
						0.0f ) ;
					MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
					fprintf( fp, "\t%f,%f,%f,%f;;\n",
						Frame->LocalTransformMatrix.m[0][3],
						Frame->LocalTransformMatrix.m[1][3],
						Frame->LocalTransformMatrix.m[2][3],
						1.0f ) ;
					MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
					fprintf( fp, "}\n" ) ;
				}

				if( MeshSave )
				{
					// Meshの出力
					if( Frame->MeshNum != 0 )
					{
						MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
						fprintf( fp, "Mesh {\n" ) ;

						VertexNum = 0 ;
						Mesh = Frame->Mesh ;
						for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
						{
							VertexNum += Mesh->VertexNum ;
						}

						MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
						fprintf( fp, "%d;\n", VertexNum ) ;

						Mesh = Frame->Mesh ;
						for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
						{
							MVert = Mesh->Vertex ;
							for( k = 0 ; k < Mesh->VertexNum ; k ++, MVert = ( MV1_MESH_VERTEX * )( ( BYTE * )MVert + Mesh->VertUnitSize ) )
							{
								Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * MVert->PositionIndex ) ;

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
								fprintf( fp, "%f;%f;%f;", Pos->Position.x, Pos->Position.y, Pos->Position.z ) ;
								fprintf( fp, k == Mesh->VertexNum - 1 && j == Frame->MeshNum - 1 ? ";\n" : ",\n" ) ;
							}
						}

						MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
						fprintf( fp, "%d;\n", Frame->TriangleNum ) ;

						VertexStartIndex = 0 ;
						Mesh = Frame->Mesh ;
						for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
						{
							Face = Mesh->Face ;
							for( k = 0 ; k < Mesh->FaceNum ; k ++, Face ++ )
							{
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
								fprintf( fp, "3;%d,%d,%d;", Face->VertexIndex[ 0 ] + VertexStartIndex, Face->VertexIndex[ 1 ] + VertexStartIndex, Face->VertexIndex[ 2 ] + VertexStartIndex ) ;
								fprintf( fp, j == Frame->MeshNum - 1 && k == Mesh->FaceNum - 1 ? ";\n" : ",\n" ) ;
							}
							VertexStartIndex += Mesh->VertexNum ;
						}

						// 法線の出力
						{
							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
							fprintf( fp, "MeshNormals {\n" ) ;

							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
							fprintf( fp, "%d;\n", VertexNum ) ;

							Mesh = Frame->Mesh ;
							for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
							{
								MVert = Mesh->Vertex ;
								for( k = 0 ; k < Mesh->VertexNum ; k ++, MVert = ( MV1_MESH_VERTEX * )( ( BYTE * )MVert + Mesh->VertUnitSize ) )
								{
									Nrm = Frame->Normal + MVert->NormalIndex ;

									MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
									fprintf( fp, "%f;%f;%f;", Nrm->Normal.x, Nrm->Normal.y, Nrm->Normal.z ) ;
									fprintf( fp, k == Mesh->VertexNum - 1 && j == Frame->MeshNum - 1 ? ";\n" : ",\n" ) ;
								}
							}

							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
							fprintf( fp, "%d;\n", Frame->TriangleNum ) ;

							VertexStartIndex = 0 ;
							Mesh = Frame->Mesh ;
							for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
							{
								Face = Mesh->Face ;
								for( k = 0 ; k < Mesh->FaceNum ; k ++, Face ++ )
								{
									MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
									fprintf( fp, "3;%d,%d,%d;", Face->VertexIndex[ 0 ] + VertexStartIndex, Face->VertexIndex[ 1 ] + VertexStartIndex, Face->VertexIndex[ 2 ] + VertexStartIndex ) ;
									fprintf( fp, j == Frame->MeshNum - 1 && k == Mesh->FaceNum - 1 ? ";\n" : ",\n" ) ;
								}
								VertexStartIndex += Mesh->VertexNum ;
							}

							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
							fprintf( fp, "}\n" ) ;
						}

						// テクスチャアドレスの出力
						{
							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
							fprintf( fp, "MeshTextureCoords {\n" ) ;

							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
							fprintf( fp, "%d;\n", VertexNum ) ;

							Mesh = Frame->Mesh ;
							for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
							{
								MVert = Mesh->Vertex ;
								for( k = 0 ; k < Mesh->VertexNum ; k ++, MVert = ( MV1_MESH_VERTEX * )( ( BYTE * )MVert + Mesh->VertUnitSize ) )
								{
									MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
									fprintf( fp, "%f;%f;", MVert->UVs[ 0 ][ 0 ], MVert->UVs[ 0 ][ 1 ] ) ;
									fprintf( fp, k == Mesh->VertexNum - 1 && j == Frame->MeshNum - 1 ? ";\n" : ",\n" ) ;
								}
							}

							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
							fprintf( fp, "}\n" ) ;
						}

						// MeshMaterialList の出力
						{
							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
							fprintf( fp, "MeshMaterialList {\n" ) ;

							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
							fprintf( fp, "%d;\n", Frame->MeshNum ) ;
							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
							fprintf( fp, "%d;\n", Frame->TriangleNum ) ;
							Mesh = Frame->Mesh ;
							for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
							{
								Face = Mesh->Face ;
								for( k = 0 ; k < Mesh->FaceNum ; k ++, Face ++ )
								{
									MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
									fprintf( fp, "%d", j ) ;
									fprintf( fp, j == Frame->MeshNum - 1 && k == Mesh->FaceNum - 1 ? ";\n" : ",\n" ) ;
								}
							}

							Mesh = Frame->Mesh ;
							for( j = 0 ; j < Frame->MeshNum ; j ++, Mesh ++ )
							{
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								if( Mesh->Material->NameW[ 0 ] == L'\0' )
								{
									fprintf( fp, "{Material_%03d}\n", Mesh->Material - ModelBase->Material ) ;
								}
								else
								{
									ConvString( ( const char * )Mesh->Material->NameW, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
									fprintf( fp, "{%s}\n", MV1SaveModelToXFileConvSpace( String ) ) ;
								}
							}

							MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
							fprintf( fp, "}\n" ) ;
						}

						// スキニングメッシュ情報の出力
						if( Frame->UseSkinBoneNum != 0 )
						{
							// XSkinMeshHeader の出力
							{
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
								fprintf( fp, "XSkinMeshHeader {\n" ) ;

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%d;\n", Frame->MaxBoneBlendNum ) ;
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%d;\n", Frame->MaxBoneBlendNum * 3 ) ;
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%d;\n", Frame->UseSkinBoneNum ) ;

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
								fprintf( fp, "}\n" ) ;
							}

							// スキニングウエイト情報の出力
							for( j = 0 ; j < Frame->UseSkinBoneNum ; j ++ )
							{
								SkinB = Frame->UseSkinBone[ j ] ;

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
								fprintf( fp, "SkinWeights {\n" ) ;

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;

								ConvString( ( const char * )ModelBase->Frame[ SkinB->BoneFrame ].NameW, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
								fprintf( fp, "\"%s\";\n", MV1SaveModelToXFileConvSpace( String ) ) ;

								UseNum = 0 ;
								VertexStartIndex = 0 ;
								Mesh = Frame->Mesh ;
								for( k = 0 ; k < Frame->MeshNum ; k ++, Mesh ++ )
								{
									MVert = Mesh->Vertex ;
									for( l = 0 ; l < Mesh->VertexNum ; l ++, MVert = ( MV1_MESH_VERTEX * )( ( BYTE * )MVert + Mesh->VertUnitSize ) )
									{
										Pos = ( MV1_MESH_POSITION * )( ( BYTE * )Frame->Position + Frame->PosUnitSize * MVert->PositionIndex ) ;
										for( m = 0 ; m < Frame->MaxBoneBlendNum ; m ++ )
										{
											if( Pos->BoneWeight[ m ].Index == j )
												break ;
										}
										if( m != Frame->MaxBoneBlendNum && ( *( ( DWORD * )&Pos->BoneWeight[ m ].W ) & 0x7fffffff ) != 0 )
										{
											UseBoneMap[ UseNum ].Index = l + VertexStartIndex ;
											UseBoneMap[ UseNum ].W = Pos->BoneWeight[ m ].W ;
											UseNum ++ ;
										}
									}
									VertexStartIndex += Mesh->VertexNum ;
								}

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%d;\n", UseNum ) ;

								for( k = 0 ; k < UseNum ; k ++ )
								{
									MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
									fprintf( fp, "%d", UseBoneMap[ k ].Index ) ;
									if( k != UseNum - 1 )
									{
										fprintf( fp, ",\n" ) ;
									}
								}
								fprintf( fp, ";\n" ) ;

								for( k = 0 ; k < UseNum ; k ++ )
								{
									MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
									fprintf( fp, "%f", UseBoneMap[ k ].W ) ;
									if( k != UseNum - 1 )
									{
										fprintf( fp, ",\n" ) ;
									}
								}
								fprintf( fp, ";\n" ) ;

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%f,%f,%f,%f,\n",
									SkinB->ModelLocalMatrix.m[ 0 ][ 0 ],
									SkinB->ModelLocalMatrix.m[ 1 ][ 0 ],
									SkinB->ModelLocalMatrix.m[ 2 ][ 0 ],
									0.0f ) ;
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%f,%f,%f,%f,\n",
									SkinB->ModelLocalMatrix.m[ 0 ][ 1 ],
									SkinB->ModelLocalMatrix.m[ 1 ][ 1 ],
									SkinB->ModelLocalMatrix.m[ 2 ][ 1 ],
									0.0f ) ;
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%f,%f,%f,%f,\n",
									SkinB->ModelLocalMatrix.m[ 0 ][ 2 ],
									SkinB->ModelLocalMatrix.m[ 1 ][ 2 ],
									SkinB->ModelLocalMatrix.m[ 2 ][ 2 ],
									0.0f ) ;
								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 2 ) ;
								fprintf( fp, "%f,%f,%f,%f;;\n",
									SkinB->ModelLocalMatrix.m[ 0 ][ 3 ],
									SkinB->ModelLocalMatrix.m[ 1 ][ 3 ],
									SkinB->ModelLocalMatrix.m[ 2 ][ 3 ],
									1.0f ) ;

								MV1SaveModelToXFileOutputTab( fp, FrameStackNum + 1 ) ;
								fprintf( fp, "}\n" ) ;
							}
						}

						MV1SaveModelToXFileOutputTab( fp, FrameStackNum ) ;
						fprintf( fp, "}\n" ) ;
					}
				}

				FrameStackStep[ FrameStackNum - 1 ] = 1 ;
				if( Frame->FirstChild )
				{
					FrameStack[ FrameStackNum ] = Frame->FirstChild ;
					FrameStackStep[ FrameStackNum ] = 0 ;
					FrameStackNum ++ ;
				}
				break ;

			case 1 :
				MV1SaveModelToXFileOutputTab( fp, FrameStackNum - 1 ) ;
				fprintf( fp, "}\n" ) ;

				FrameStackNum -- ;
				if( Frame->Next )
				{
					FrameStack[ FrameStackNum ] = Frame->Next ;
					FrameStackStep[ FrameStackNum ] = 0 ;
					FrameStackNum ++ ;
				}
				break ;
			}
		}
	}

	// アニメーションの出力
	if( AnimSave )
	{
		KeyTiming = NULL ;
		if( AnimModelBase->AnimSetNum != 0 )
		{
			j = 0 ;
			KeySet = AnimModelBase->AnimKeySet ;
			for( i = 0 ; i < AnimModelBase->AnimKeySetNum ; i ++, KeySet ++ )
				j += KeySet->Num ;

			FrameNum = AnimModelBase->FrameNum ;
			KeyTiming = ( int * )DXALLOC( ( sizeof( MATRIX_4X4CT_F ) * AnimModelBase->FrameNum + sizeof( int ) ) * j ) ;
			if( KeyTiming == NULL ) goto ERRORLABEL ;
			KeyMatrix = ( MATRIX_4X4CT_F * )( KeyTiming + j ) ;

			AnimSet = AnimModelBase->AnimSet ;
			for( i = 0 ; i < AnimModelBase->AnimSetNum ; i ++, AnimSet ++ )
			{
				AnimNum = AnimSet->AnimNum ;

				ConvString( ( const char * )AnimSet->NameW, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
				fprintf( fp, "AnimationSet %s {\n", MV1SaveModelToXFileConvSpace( String ) ) ;

				// 全てのキータイミングを列挙
				KeyTimingNum = 0 ;
				Anim = AnimSet->Anim ;
				for( j = 0 ; j < AnimSet->AnimNum ; j ++, Anim ++ )
				{
					KeySet = Anim->KeySet ;
					for( k = 0 ; k < Anim->KeySetNum ; k ++, KeySet ++ )
					{
						for( l = 0 ; l < KeySet->Num ; l ++ )
						{
							switch( KeySet->TimeType )
							{
							case MV1_ANIMKEY_TIME_TYPE_ONE :
								time = _FTOL( l * KeySet->UnitTime + KeySet->StartTime ) ;
								break ;

							case MV1_ANIMKEY_TIME_TYPE_KEY :
								time = _FTOL( KeySet->KeyTime[ l ] ) ;
								break ;
							}
							for( m = 0 ; m < KeyTimingNum && KeyTiming[ m ] < time ; m ++ ){}
							if( m == KeyTimingNum || KeyTiming[ m ] != time )
							{
								if( m != KeyTimingNum )
								{
									_MEMMOVE( &KeyTiming[ m + 1 ], &KeyTiming[ m ], sizeof( int ) * ( KeyTimingNum - m ) ) ;
								}
								KeyTiming[ m ] = time ;
								KeyTimingNum ++ ;
							}
						}
					}
				}

				// 全てのキータイミングでの行列の割り出し
				AnimIndex = NS_MV1AttachAnim( MHandle, i, AnimMHandle >= 0 ? AnimMHandle : MHandle, AnimNameCheck ) ;
				if( AnimIndex == -1 ) goto ERRORLABEL ;

				MotSet = Model->AnimSet[ AnimIndex ].AnimSet ;
				Matrix = KeyMatrix ;
				for( j = 0 ; j < KeyTimingNum ; j ++ )
				{
					NS_MV1SetAttachAnimTime( MHandle, AnimIndex, ( float )KeyTiming[ j ] ) ;
					MV1SetupAnimMatrix( Model ) ;

					Mot = MotSet->Anim ;
					for( k = 0 ; k < AnimSet->AnimNum ; k ++, Mot ++, Matrix ++ )
					{
						MV1SetupTransformMatrix(
							Matrix,
							Mot->ValidFlag,
							&Mot->Translate,
							&Mot->Scale,
							Mot->RotateOrder,
							( Mot->Frame->BaseData->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &Mot->Frame->BaseData->PreRotate : NULL,
							&Mot->Rotate,
							( Mot->Frame->BaseData->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &Mot->Frame->BaseData->PostRotate : NULL,
							&Mot->Quaternion
						) ;
					}
				}

				// キー情報の出力
				Anim = AnimSet->Anim ;
				for( j = 0 ; j < AnimSet->AnimNum ; j ++, Anim ++ )
				{
					if( AnimMHandle < 0 )
					{
						ConvString( ( const char * )Anim->TargetFrame->NameW, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
						fprintf( fp, "\tAnimation {\n\t\t{%s}\n", MV1SaveModelToXFileConvSpace( String ) ) ;
					}
					else
					{
						MV1_MODEL_ANIM *MAnim ;

						for( k = 0 ; k < ModelBase->FrameNum ; k ++ )
						{
							MAnim = &Model->Anim[ AnimIndex + Model->AnimSetMaxNum * k ] ;
							if( MAnim->Use == false ) continue ;
							if( MAnim->Anim->BaseData == Anim ) break ;
						}
						if( k == ModelBase->FrameNum )
						{
							NS_MV1DetachAnim( AnimMHandle, AnimIndex ) ;
							Err = -2 ;
							goto ERRORLABEL ;
						}

						ConvString( ( const char * )ModelBase->Frame[ k ].NameW, WCHAR_T_CODEPAGE, String, DX_CODEPAGE_SHIFTJIS ) ;
						fprintf( fp, "\tAnimation {\n\t\t{%s}\n", MV1SaveModelToXFileConvSpace( String ) ) ;
					}

					fprintf( fp, "\t\tAnimationKey {\n\t\t\t4;\n\t\t\t%d;\n", KeyTimingNum ) ;
					Matrix = KeyMatrix + j ;
					for( k = 0 ; k < KeyTimingNum ; k ++, Matrix += AnimNum )
					{
						fprintf( fp, "\t\t\t%d;16;\n\t\t\t%f,%f,%f,%f,\n\t\t\t%f,%f,%f,%f,\n\t\t\t%f,%f,%f,%f,\n\t\t\t%f,%f,%f,%f;;",
							KeyTiming[ k ],
							Matrix->m[ 0 ][ 0 ], Matrix->m[ 1 ][ 0 ], Matrix->m[ 2 ][ 0 ], 0.0f,
							Matrix->m[ 0 ][ 1 ], Matrix->m[ 1 ][ 1 ], Matrix->m[ 2 ][ 1 ], 0.0f,
							Matrix->m[ 0 ][ 2 ], Matrix->m[ 1 ][ 2 ], Matrix->m[ 2 ][ 2 ], 0.0f,
							Matrix->m[ 0 ][ 3 ], Matrix->m[ 1 ][ 3 ], Matrix->m[ 2 ][ 3 ], 1.0f ) ;
						fprintf( fp, k == KeyTimingNum - 1 ? ";\n" : ",\n" ) ;
					}

					fprintf( fp, "\t\t}\n" ) ;
					fprintf( fp, "\t}\n" ) ;
				}
				fprintf( fp, "}\n" ) ;

				NS_MV1DetachAnim( MHandle, AnimIndex ) ;
			}
		}
	}

	// メモリの解放
	if( UseBoneMap != 0 )
	{
		DXFREE( UseBoneMap ) ;
		UseBoneMap = NULL ;
	}

	if( KeyTiming != 0 )
	{
		DXFREE( KeyTiming ) ;
		KeyTiming = NULL ;
	}

	// ファイルを閉じる
	fclose( fp ) ;

	// 終了
	return 0 ;

ERRORLABEL :
	if( UseBoneMap != 0 )
	{
		DXFREE( UseBoneMap ) ;
		UseBoneMap = NULL ;
	}

	if( KeyTiming != 0 )
	{
		DXFREE( KeyTiming ) ;
		KeyTiming = NULL ;
	}

	if( fp )
	{
		fclose( fp ) ;
		fp = NULL ;
	}

	return Err ;
}

#endif // DX_NON_SAVEFUNCTION
































// モデルハンドルで使用されているモデル基本データハンドルを取得する
extern int MV1GetModelBaseHandle( int MHandle )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// モデル基本データハンドルを返す
	return Model->BaseDataHandle ;
}

// モデルのデータサイズを取得する
extern int MV1GetModelDataSize( int MHandle, int DataType )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// データタイプによって処理を分岐
	switch( DataType )
	{
//	case MV1_DATASIZE_TOTAL :
//		return ModelBase->TotalDataSize ;

	case MV1_DATASIZE_VERTEX :
		return ( int )ModelBase->VertexDataSize ;

	case MV1_DATASIZE_STRING :
		return
#ifndef UNICODE
			ModelBase->StringSizeA +
#endif
			ModelBase->StringSizeW ;

	case MV1_DATASIZE_ANIM :
		return ( int )(
				ModelBase->AnimKeyDataSize +
				ModelBase->AnimSetNum      * sizeof( MV1_ANIMSET_BASE     ) +
				ModelBase->AnimNum         * sizeof( MV1_ANIM_BASE        ) +
				ModelBase->AnimKeySetNum   * sizeof( MV1_ANIM_KEYSET_BASE ) ) ;

//	case MV1_DATASIZE_OTHER :
//		return ModelBase->TotalDataSize -
//			( MV1GetModelDataSize( MHandle, MV1_DATASIZE_VERTEX ) +
//			  MV1GetModelDataSize( MHandle, MV1_DATASIZE_STRING ) +
//			  MV1GetModelDataSize( MHandle, MV1_DATASIZE_ANIM ) ) ;
	}

	// エラー
	return -1 ;
}

// アニメーションのデータサイズを取得する
extern int MV1GetAnimDataSize( int MHandle, const wchar_t *AnimName, int AnimIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_ANIM_BASE *AnimBase ;
	DWORD DataSize, i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったらエラー
	if( AnimName == NULL && ( AnimIndex < 0 || AnimIndex >= Model->BaseData->AnimSetNum ) )
		return -1 ;

	// アニメーションを取得
	AnimSetBase = MV1GetAnimSetBase( Model->BaseDataHandle, AnimName, AnimIndex ) ;
	if( AnimSetBase == NULL ) return -1 ;

	// データサイズの割り出し開始
	DataSize = AnimSetBase->KeyDataSize + sizeof( MV1_ANIMSET_BASE ) + sizeof( MV1_ANIM_BASE ) * AnimSetBase->AnimNum ;
	
	// アニメーションの数だけ繰り返し
	AnimBase = AnimSetBase->Anim ;
	for( i = 0 ; i < ( DWORD )AnimSetBase->AnimNum ; i ++, AnimBase ++ )
	{
		DataSize += AnimBase->KeySetNum * sizeof( MV1_ANIM_KEYSET_BASE  ) ;
	}

	// データサイズを返す
	return ( int )DataSize ;
}


















// モデルのローカル座標からワールド座標に変換する行列を得る
extern MATRIX NS_MV1GetLocalWorldMatrix( int MHandle )
{
	MV1_MODEL *Model ;
	MATRIX ResultMatrix ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return MGetIdent() ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return MGetIdent() ;

	// 行列を返す
	MV1SETUPMATRIX( Model ) ;
	ConvertMatrix4x4cToMatrixF( &ResultMatrix, &Model->LocalWorldMatrix ) ;
	return ResultMatrix ;
}

// モデルのローカル座標からワールド座標に変換する行列を得る
extern MATRIX_D NS_MV1GetLocalWorldMatrixD( int MHandle )
{
	MV1_MODEL *Model ;
	MATRIX_D ResultMatrix ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return MGetIdentD() ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return MGetIdentD() ;

	// 行列を返す
	MV1SETUPMATRIX( Model ) ;
	ConvertMatrix4x4cToMatrixD( &ResultMatrix, &Model->LocalWorldMatrix ) ;
	return ResultMatrix ;
}

// モデルの座標をセット
extern int NS_MV1SetPosition( int MHandle, VECTOR Position )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 今までと同じ座標だったら何もせずに終了
	if( Model->Translation.x == ( double )Position.x &&
		Model->Translation.y == ( double )Position.y &&
		Model->Translation.z == ( double )Position.z )
	{
		return 0 ;
	}

	// 平行移動値のセット
	Model->Translation = VConvFtoD( Position ) ;

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;

	// 終了
	return 0 ;
}

// モデルの座標をセット
extern int NS_MV1SetPositionD( int MHandle, VECTOR_D Position )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 今までと同じ座標だったら何もせずに終了
	if( Model->Translation.x == Position.x &&
		Model->Translation.y == Position.y &&
		Model->Translation.z == Position.z )
	{
		return 0 ;
	}

	// 平行移動値のセット
	Model->Translation = Position ;

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;

	// 終了
	return 0 ;
}

// モデルの座標を取得
extern VECTOR NS_MV1GetPosition( int MHandle )
{
	MV1_MODEL *Model ;
	VECTOR NullPos = { 0.0f, 0.0f, 0.0f } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NullPos ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NullPos ;

	// 平行移動値を返す
	return VConvDtoF( Model->Translation ) ;
}

// モデルの座標を取得
extern VECTOR_D NS_MV1GetPositionD( int MHandle )
{
	MV1_MODEL *Model ;
	VECTOR_D NullPos = { 0.0, 0.0, 0.0 } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NullPos ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NullPos ;

	// 平行移動値を返す
	return Model->Translation ;
}

// モデルの拡大値をセット
extern int NS_MV1SetScale( int MHandle, VECTOR Scale )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 今までと同じ座標だったら何もせずに終了
	if( *( ( DWORD * )&Model->Scale.x ) == *( ( DWORD * )&Scale.x ) &&
		*( ( DWORD * )&Model->Scale.y ) == *( ( DWORD * )&Scale.y ) &&
		*( ( DWORD * )&Model->Scale.z ) == *( ( DWORD * )&Scale.z ) )
		return 0 ;

	// スケール値のセット
	Model->Scale = Scale ;

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;

	// 終了
	return 0 ;
}

// モデルの拡大値を取得
extern VECTOR NS_MV1GetScale( int MHandle )
{
	MV1_MODEL *Model ;
	VECTOR NullPos = { 0.0f, 0.0f, 0.0f } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NullPos ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NullPos ;

	// スケール値を返す
	return Model->Scale ;
}

// モデルの回転値をセット( X軸回転→Y軸回転→Z軸回転方式 )
extern int NS_MV1SetRotationXYZ( int MHandle, VECTOR Rotate )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 今までと同じ座標だったら何もせずに終了
	if( Model->Rotation.Type == MV1_ROTATE_TYPE_XYZROT &&
		*( ( DWORD * )&Model->Rotation.XYZRot.x ) == *( ( DWORD * )&Rotate.x ) &&
		*( ( DWORD * )&Model->Rotation.XYZRot.y ) == *( ( DWORD * )&Rotate.y ) &&
		*( ( DWORD * )&Model->Rotation.XYZRot.z ) == *( ( DWORD * )&Rotate.z ) )
		return 0 ;

	// 回転値のセット
	Model->Rotation.Type = MV1_ROTATE_TYPE_XYZROT ;
	Model->Rotation.XYZRot = Rotate ;

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;

	// 終了
	return 0 ;
}

// モデルの回転値を取得( X軸回転→Y軸回転→Z軸回転方式 )
extern VECTOR NS_MV1GetRotationXYZ( int MHandle )
{
	MV1_MODEL *Model ;
	VECTOR NullPos = { 0.0f, 0.0f, 0.0f } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NullPos ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NullPos ;

	// 回転値を返す
	return Model->Rotation.XYZRot ;
}

// モデルの回転値をセット( Ｚ軸の向ける方向を指定する方式 )
extern	int	NS_MV1SetRotationZYAxis( int MHandle, VECTOR ZAxisDirection, VECTOR YAxisDirection, float ZAxisTwistRotate )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 回転値のセット
	Model->Rotation.Type = MV1_ROTATE_TYPE_ZAXIS ;
	Model->Rotation.ZAxis.Z = VNorm( ZAxisDirection ) ;
	Model->Rotation.ZAxis.Up = VNorm( VCross( VCross( Model->Rotation.ZAxis.Z, YAxisDirection ), Model->Rotation.ZAxis.Z ) ) ; 
	Model->Rotation.ZAxis.Twist = ZAxisTwistRotate ;

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;

	// 終了
	return 0 ;
}

// モデルのＺ軸の方向を得る
extern	VECTOR MV1GetRotationZAxisZ( int MHandle )
{
	MV1_MODEL *Model ;
	VECTOR NullPos = { 0.0f, 0.0f, 0.0f } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NullPos ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NullPos ;

	// 軸方向を返す
	return Model->Rotation.ZAxis.Z ;
}

// モデルのＺ軸の方向を指定した際の上方向ベクトルを得る
extern VECTOR MV1GetRotationZAxisUp( int MHandle )
{
	MV1_MODEL *Model ;
	VECTOR NullPos = { 0.0f, 0.0f, 0.0f } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NullPos ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NullPos ;

	// 軸方向を返す
	return Model->Rotation.ZAxis.Up ;
}

// モデルのＺ軸の方向を指定した際の捻り角度を得る
extern	float MV1GetRotationZAxisTwist( int MHandle )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 軸方向を返す
	return Model->Rotation.ZAxis.Twist ;
}

// モデルの回転用行列をセットする
extern	int NS_MV1SetRotationMatrix( int MHandle, MATRIX Matrix )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 今までと同じだった場合は何もせずに終了
	if( Model->Rotation.Type == MV1_ROTATE_TYPE_MATRIX &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 0 ][ 0 ] ) == *( ( DWORD * )&Matrix.m[ 0 ][ 0 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 0 ][ 1 ] ) == *( ( DWORD * )&Matrix.m[ 1 ][ 0 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 0 ][ 2 ] ) == *( ( DWORD * )&Matrix.m[ 2 ][ 0 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 1 ][ 0 ] ) == *( ( DWORD * )&Matrix.m[ 0 ][ 1 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 1 ][ 1 ] ) == *( ( DWORD * )&Matrix.m[ 1 ][ 1 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 1 ][ 2 ] ) == *( ( DWORD * )&Matrix.m[ 2 ][ 1 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 2 ][ 0 ] ) == *( ( DWORD * )&Matrix.m[ 0 ][ 2 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 2 ][ 1 ] ) == *( ( DWORD * )&Matrix.m[ 1 ][ 2 ] ) &&
		*( ( DWORD * )&Model->Rotation.Mat.m[ 2 ][ 2 ] ) == *( ( DWORD * )&Matrix.m[ 2 ][ 2 ] ) )
		return 0 ;

	// 回転値のセット
	Model->Rotation.Type = MV1_ROTATE_TYPE_MATRIX ;
	ConvertMatrixFToMatrix4x4cF( &Model->Rotation.Mat, &Matrix ) ;
	Model->Rotation.Mat.m[ 0 ][ 3 ] = 0.0f ;
	Model->Rotation.Mat.m[ 1 ][ 3 ] = 0.0f ;
	Model->Rotation.Mat.m[ 2 ][ 3 ] = 0.0f ;

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;

	// 終了
	return 0 ;
}

// モデルの回転用行列を取得する
extern	MATRIX NS_MV1GetRotationMatrix( int MHandle )
{
	MV1_MODEL *Model ;
	static MATRIX NullMat = { 0.0f, 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f, 0.0f } ;
	MATRIX ResultMatrix ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NullMat ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NullMat ;

	// 回転行列を返す
	ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &Model->Rotation.Mat ) ;
	return ResultMatrix ;
}

// モデルの変形用行列をセットする
extern int NS_MV1SetMatrix( int MHandle, MATRIX Matrix )
{
	static const MATRIX IdentityMatrix =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	} ;
	MATRIX_4X4CT Matrix4X4CT ;
	bool ValidMatrix ;
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 今までと同じ場合は何もせずに終了
	ValidMatrix = _MEMCMP( &IdentityMatrix, &Matrix, sizeof( MATRIX ) ) != 0 ;
	ConvertMatrixFToMatrix4x4c( &Matrix4X4CT, &Matrix ) ;
	if( ( Model->ValidMatrix == false && ValidMatrix == false ) ||
		( Model->ValidMatrix == true  && ValidMatrix == true  &&
		  _MEMCMP( &Matrix4X4CT, &Model->Matrix, sizeof( MATRIX_4X4CT ) ) == 0 ) )
	{
		return 0 ;
	}

	// 行列のセット
	Model->ValidMatrix = ValidMatrix ;
	if( Model->ValidMatrix )
	{
		Model->Matrix = Matrix4X4CT ;
	}

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
	{
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルの変形用行列をセットする
extern int NS_MV1SetMatrixD( int MHandle, MATRIX_D Matrix )
{
	static const MATRIX_D IdentityMatrix =
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0,
	} ;
	MATRIX_4X4CT Matrix4X4CT ;
	bool ValidMatrix ;
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 今までと同じ場合は何もせずに終了
	ValidMatrix = _MEMCMP( &IdentityMatrix, &Matrix, sizeof( MATRIX_D ) ) != 0 ;
	ConvertMatrixDToMatrix4x4c( &Matrix4X4CT, &Matrix ) ;
	if( ( Model->ValidMatrix == false && ValidMatrix == false ) ||
		( Model->ValidMatrix == true  && ValidMatrix == true  &&
		  _MEMCMP( &Matrix4X4CT, &Model->Matrix, sizeof( MATRIX_4X4CT ) ) == 0 ) )
	{
		return 0 ;
	}

	// 行列のセット
	Model->ValidMatrix = ValidMatrix ;
	if( Model->ValidMatrix )
	{
		Model->Matrix = Matrix4X4CT ;
	}

	// 更新フラグを立てる
	Model->LocalWorldMatrixSetupFlag = false ;
	if( ( Model->ChangeMatrixFlag[ 0 ] & 1 ) == 0 )
	{
		_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルの変形用行列を取得する
extern MATRIX NS_MV1GetMatrix( int MHandle )
{
	MV1_MODEL *Model ;
	MATRIX ResultMatrix ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return MGetIdent() ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return MGetIdent() ;

	// 行列を返す
	if( Model->ValidMatrix )
	{
		ConvertMatrix4x4cToMatrixF( &ResultMatrix, &Model->Matrix ) ;
	}
	else
	{
		MV1SETUPMATRIX( Model ) ;
		ConvertMatrix4x4cToMatrixF( &ResultMatrix, &Model->LocalWorldMatrix ) ;
	}

	return ResultMatrix ;
}

// モデルの変形用行列を取得する
extern MATRIX_D NS_MV1GetMatrixD( int MHandle )
{
	MV1_MODEL *Model ;
	MATRIX_D ResultMatrix ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return MGetIdentD() ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return MGetIdentD() ;

	// 行列を返す
	if( Model->ValidMatrix )
	{
		ConvertMatrix4x4cToMatrixD( &ResultMatrix, &Model->Matrix ) ;
	}
	else
	{
		MV1SETUPMATRIX( Model ) ;
		ConvertMatrix4x4cToMatrixD( &ResultMatrix, &Model->LocalWorldMatrix ) ;
	}

	return ResultMatrix ;
}

// モデルの表示、非表示状態を変更する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1SetVisible( int MHandle, int VisibleFlag )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialVisible( &Model->DrawMaterial, NULL, ( BYTE )VisibleFlag ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 更新ビットが立っていなかったら立てる
		if( ( Model->ChangeDrawMaterialFlag[ 0 ] & 1 ) == 0 )
			_MEMSET( Model->ChangeDrawMaterialFlag, 0xff, Model->ChangeDrawMaterialFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルの表示、非表示状態を取得する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1GetVisible( int MHandle )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// フラグを返す
	return Model->DrawMaterial.Visible ;
}

// モデルのメッシュの種類( DX_MV1_MESHCATEGORY_NORMAL など )毎の表示、非表示を設定する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1SetMeshCategoryVisible( int MHandle, int MeshCategory, int VisibleFlag )
{
	int HideFlag ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 値の有効チェック
	if( MeshCategory < 0 || MeshCategory >= DX_MV1_MESHCATEGORY_NUM )
		return -1 ;

	HideFlag = VisibleFlag == FALSE ? TRUE : FALSE ;

	if( HideFlag == Model->MeshCategoryHide[ MeshCategory ] )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// フラグを保存
	Model->MeshCategoryHide[ MeshCategory ] = HideFlag ;

	// 終了
	return 0 ;
}

// モデルのメッシュの種類( DX_MV1_MESHCATEGORY_NORMAL など )毎の表示、非表示を取得する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1GetMeshCategoryVisible( int MHandle, int MeshCategory )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 値の有効チェック
	if( MeshCategory < 0 || MeshCategory >= DX_MV1_MESHCATEGORY_NUM )
		return -1 ;

	// フラグを返す
	return Model->MeshCategoryHide[ MeshCategory ] == FALSE ? TRUE : FALSE ;
}

// モデルのディフューズカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetDifColorScale( int MHandle, COLOR_F Scale )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialDif( &Model->DrawMaterial, NULL, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 半透明要素有無情報のセットアップ完了フラグを倒す
		Model->SemiTransStateSetupFlag = false ;

		// 更新ビットが立っていなかったら立てる
		if( ( Model->ChangeDrawMaterialFlag[ 0 ] & 1 ) == 0 )
			_MEMSET( Model->ChangeDrawMaterialFlag, 0xff, Model->ChangeDrawMaterialFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルのスペキュラカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetSpcColorScale( int MHandle, COLOR_F Scale )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialSpc( &Model->DrawMaterial, NULL, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 更新ビットが立っていなかったら立てる
		if( ( Model->ChangeDrawMaterialFlag[ 0 ] & 1 ) == 0 )
			_MEMSET( Model->ChangeDrawMaterialFlag, 0xff, Model->ChangeDrawMaterialFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルのエミッシブカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetEmiColorScale( int MHandle, COLOR_F Scale )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialEmi( &Model->DrawMaterial, NULL, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 更新ビットが立っていなかったら立てる
		if( ( Model->ChangeDrawMaterialFlag[ 0 ] & 1 ) == 0 )
			_MEMSET( Model->ChangeDrawMaterialFlag, 0xff, Model->ChangeDrawMaterialFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルのアンビエントカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetAmbColorScale( int MHandle, COLOR_F Scale )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialAmb( &Model->DrawMaterial, NULL, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 更新ビットが立っていなかったら立てる
		if( ( Model->ChangeDrawMaterialFlag[ 0 ] & 1 ) == 0 )
			_MEMSET( Model->ChangeDrawMaterialFlag, 0xff, Model->ChangeDrawMaterialFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルのディフューズカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetDifColorScale( int MHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1START( MHandle, Model, ModelBase, ErrorResult ) ;

	// 値を返す
	return Model->DrawMaterial.DiffuseScale ;
}

// モデルのスペキュラカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetSpcColorScale( int MHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1START( MHandle, Model, ModelBase, ErrorResult ) ;

	// 値を返す
	return Model->DrawMaterial.SpecularScale ;
}

// モデルのエミッシブカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetEmiColorScale( int MHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1START( MHandle, Model, ModelBase, ErrorResult ) ;

	// 値を返す
	return Model->DrawMaterial.EmissiveScale ;
}

// モデルのアンビエントカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetAmbColorScale( int MHandle )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1START( MHandle, Model, ModelBase, ErrorResult ) ;

	// 値を返す
	return Model->DrawMaterial.AmbientScale ;
}

// モデルに半透明要素があるかどうかを取得する( 戻り値 TRUE:ある  FALSE:ない )
extern int NS_MV1GetSemiTransState( int MHandle )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 半透明要素有無情報のセットアップが完了していなければセットアップをする
	if( Model->SemiTransStateSetupFlag == false )
	{
		// トップフレームの中に半透明要素を持ったフレームが一つでもあれば半透明要素を持ったモデルということ
		Model->SemiTransState = false ;

		for( i = 0 ; i < Model->TopFrameNum ; i ++ )
		{
			if( NS_MV1GetFrameSemiTransState( MHandle, ( int )( Model->TopFrameList[ i ] - Model->Frame ) ) )
				Model->SemiTransState = true ;
		}

		// セットアップ完了
		Model->SemiTransStateSetupFlag = true ;
	}

	// 返す
	return Model->SemiTransState ;
}

// モデルの不透明度を設定する( 不透明 1.0f ～ 透明 0.0f )
extern int NS_MV1SetOpacityRate( int MHandle, float Rate )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 範囲を超えていたら補正する
	     if( Rate < 0.0f ) Rate = 0.0f ;
	else if( Rate > 1.0f ) Rate = 1.0f ;

	// 値をセット
	if( MV1SetDrawMaterialOpacityRate( &Model->DrawMaterial, NULL, Rate ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 半透明要素有無情報のセットアップ完了フラグを倒す
		Model->SemiTransStateSetupFlag = false ;

		// 更新ビットが立っていなかったら立てる
		if( ( Model->ChangeDrawMaterialFlag[ 0 ] & 1 ) == 0 )
			_MEMSET( Model->ChangeDrawMaterialFlag, 0xff, Model->ChangeDrawMaterialFlagSize ) ;
	}

	// 終了
	return 0 ;
}

// モデルの不透明度を取得する( 不透明 1.0f ～ 透明 0.0f )
extern	float		NS_MV1GetOpacityRate( int MHandle )
{
	MV1START( MHandle, Model, ModelBase, -1.0f ) ;

	// 不透明度を返す
	return Model->DrawMaterial.OpacityRate ;
}

// モデルを描画する際にRGB値に対してA値を乗算するかどうかを設定する
// ( 描画結果が乗算済みアルファ画像になります )( Flag   TRUE:RGB値に対してA値を乗算する  FALSE:乗算しない(デフォルト) )
extern int NS_MV1SetUseDrawMulAlphaColor( int MHandle, int Flag )
{
	bool BoolFlag ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	BoolFlag = Flag != FALSE ;

	if( Model->UseDrawMulAlphaColor == BoolFlag )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 値を保存
	Model->UseDrawMulAlphaColor = BoolFlag ;

	// 終了
	return 0 ;
}

// モデルを描画する際にRGB値に対してA値を乗算するかどうかを取得する
// ( 描画結果が乗算済みアルファ画像になります )( 戻り値 TRUE:RGB値に対してA値を乗算する  FALSE:乗算しない(デフォルト) )
extern int NS_MV1GetUseDrawMulAlphaColor( int MHandle )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	return Model->UseDrawMulAlphaColor ? TRUE : FALSE ;
}

// モデルを描画する際にＺバッファを使用するかどうかを設定する
extern int NS_MV1SetUseZBuffer( int MHandle, int Flag )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	if( Model->EnableZBufferFlag == Flag )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Model->EnableZBufferFlag = Flag ;

	// 終了
	return 0 ;
}

// モデルを描画する際にＺバッファに書き込みを行うかどうかを設定する
extern int NS_MV1SetWriteZBuffer( int MHandle, int Flag )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	if( Model->WriteZBufferFlag == Flag )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Model->WriteZBufferFlag = Flag ;

	// 終了
	return 0 ;
}

// モデルの描画時のＺ値の比較モードを設定する
extern int NS_MV1SetZBufferCmpType( int MHandle, int CmpType /* DX_CMP_NEVER 等 */ )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	if( Model->ZBufferCmpType == CmpType )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Model->ZBufferCmpType = CmpType ;

	// 終了
	return 0 ;
}

// モデルの描画時の書き込むＺ値のバイアスを設定する
extern int NS_MV1SetZBias( int MHandle, int Bias )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	if( Model->ZBias == Bias )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Model->ZBias = Bias ;

	// 終了
	return 0 ;
}

// モデルの含まれるメッシュの頂点ディフューズカラーをマテリアルのディフューズカラーの代わりに使用するかどうかを設定する( TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int NS_MV1SetUseVertDifColor( int MHandle, int UseFlag )
{
	int i ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// すべてのメッシュに設定する
	for( i = 0 ; i < ModelBase->MeshNum ; i ++ )
		NS_MV1SetMeshUseVertDifColor( MHandle, i, UseFlag ) ;
	
	// 終了
	return 0 ;
}

// モデルに含まれるメッシュの頂点スペキュラカラーをマテリアルのスペキュラカラーの代わりに使用するかどうかを設定する( TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int NS_MV1SetUseVertSpcColor( int MHandle, int UseFlag )
{
	int i ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// すべてのメッシュに設定する
	for( i = 0 ; i < ModelBase->MeshNum ; i ++ )
		NS_MV1SetMeshUseVertSpcColor( MHandle, i, UseFlag ) ;
	
	// 終了
	return 0 ;
}

// モデルのサンプルフィルターモードを変更する
extern int NS_MV1SetSampleFilterMode( int MHandle, int FilterMode )
{
	int i ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 全てのテクスチャのフィルターモードを変更する
	for( i = 0 ; i < ModelBase->TextureNum ; i ++ )
	{
		NS_MV1SetTextureSampleFilterMode( MHandle, i, FilterMode ) ;
	}

	// 終了
	return 0 ;
}

// モデルの異方性フィルタリングの最大次数を設定する
extern int NS_MV1SetMaxAnisotropy( int MHandle, int MaxAnisotropy )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	if( Model->MaxAnisotropy == MaxAnisotropy )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Model->MaxAnisotropy = MaxAnisotropy ;

	// 終了
	return 0 ;
}

// モデルをワイヤーフレームで描画するかどうかを設定する
extern int NS_MV1SetWireFrameDrawFlag( int MHandle, int Flag )
{
	bool WireFrameFlag ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	WireFrameFlag = Flag != 0 ;

	if( WireFrameFlag == Model->WireFrame )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Model->WireFrame = WireFrameFlag ;

	// 終了
	return 0 ;
}

// モデルの頂点カラーを現在設定されているマテリアルのカラーにする
extern int NS_MV1RefreshVertColorFromMaterial( int MHandle )
{
	MV1_MESH_BASE *MBMesh ;
	MV1_MESH *Mesh ;
	MV1_MESH_VERTEX *MBVert ;
	MV1_TRIANGLE_LIST_BASE *MBTList ;
	int i, j ;
	int VertexNum ;
	COLOR_U8 DiffuseColor ;
	COLOR_U8 SpecularColor ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	MBMesh = ModelBase->Mesh ;
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, MBMesh ++, Mesh ++ )
	{
		if( MBMesh->Vertex == NULL ) continue ;

		VertexNum = MBMesh->VertexNum ;
		MBVert = MBMesh->Vertex ;
		DiffuseColor.r = ( BYTE )( Mesh->Material->Diffuse.r >= 1.0f ? 255 : ( Mesh->Material->Diffuse.r < 0.0f ? 0 : _FTOL( Mesh->Material->Diffuse.r * 255.0f ) ) ) ;
		DiffuseColor.g = ( BYTE )( Mesh->Material->Diffuse.g >= 1.0f ? 255 : ( Mesh->Material->Diffuse.g < 0.0f ? 0 : _FTOL( Mesh->Material->Diffuse.g * 255.0f ) ) ) ;
		DiffuseColor.b = ( BYTE )( Mesh->Material->Diffuse.b >= 1.0f ? 255 : ( Mesh->Material->Diffuse.b < 0.0f ? 0 : _FTOL( Mesh->Material->Diffuse.b * 255.0f ) ) ) ;
		DiffuseColor.a = ( BYTE )( Mesh->Material->Diffuse.a >= 1.0f ? 255 : ( Mesh->Material->Diffuse.a < 0.0f ? 0 : _FTOL( Mesh->Material->Diffuse.a * 255.0f ) ) ) ;
		SpecularColor.r = ( BYTE )( Mesh->Material->Specular.r >= 1.0f ? 255 : ( Mesh->Material->Specular.r < 0.0f ? 0 : _FTOL( Mesh->Material->Specular.r * 255.0f ) ) ) ;
		SpecularColor.g = ( BYTE )( Mesh->Material->Specular.g >= 1.0f ? 255 : ( Mesh->Material->Specular.g < 0.0f ? 0 : _FTOL( Mesh->Material->Specular.g * 255.0f ) ) ) ;
		SpecularColor.b = ( BYTE )( Mesh->Material->Specular.b >= 1.0f ? 255 : ( Mesh->Material->Specular.b < 0.0f ? 0 : _FTOL( Mesh->Material->Specular.b * 255.0f ) ) ) ;
		SpecularColor.a = ( BYTE )( Mesh->Material->Specular.a >= 1.0f ? 255 : ( Mesh->Material->Specular.a < 0.0f ? 0 : _FTOL( Mesh->Material->Specular.a * 255.0f ) ) ) ;
		for( j = 0 ; j < VertexNum ; j ++, MBVert ++ )
		{
			MBVert->DiffuseColor = DiffuseColor ;
			MBVert->SpecularColor = SpecularColor ;
		}
	}

	// テンポラリバッファがある場合は一度全て解放する
	MBTList = ModelBase->TriangleList ;
	for( i = 0 ; i < ModelBase->TriangleListNum ; i ++, MBTList ++ )
	{
		// 環境依存のテンポラリバッファを開放
		MV1_TerminateTriangleListBaseTempBuffer_PF( MBTList ) ;
	}

	// 頂点バッファの作り直し
	MV1_TerminateVertexBufferBase_PF( ModelBase->HandleInfo.Handle ) ;
	MV1_SetupVertexBufferBase_PF( ModelBase->HandleInfo.Handle ) ;

	// 終了
	return 0 ;
}

// モデルの物理演算の重力を設定する
extern int NS_MV1SetPhysicsWorldGravity( int MHandle, VECTOR Gravity )
{
#ifndef DX_NON_BULLET_PHYSICS
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 物理データが無い場合は何もしない
	if( Model->BaseData->PhysicsRigidBodyNum == 0 )
		return 0 ;

	SetWorldGravity_ModelPhysiceInfo( Model, Gravity ) ;
#endif

	// 終了
	return 0 ;
}

// モデルの物理演算を指定時間分経過したと仮定して計算する( MillisecondTime で指定する時間の単位はミリ秒 )
int NS_MV1PhysicsCalculation( int MHandle, float MillisecondTime )
{
	return MV1PhysicsCalculationBase( MHandle, MillisecondTime, GetASyncLoadFlag() ) ;
}

// モデルの物理演算を指定時間分経過したと仮定して計算する( MillisecondTime で指定する時間の単位はミリ秒 )
static int MV1PhysicsCalculationBase_Static( int MHandle, float MillisecondTime, int ASyncThread )
{
#ifndef DX_NON_BULLET_PHYSICS
	MV1_PHYSICS_RIGIDBODY *PhysicsRigidBody ;
	int i ;
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false )
	{
		return -1 ;
	}

	// アドレス取得
	if( ASyncThread )
	{
		if( MV1MDLCHK_ASYNC( MHandle, Model ) )
		{
			return -1 ;
		}
	}
	else
	{
		if( MV1MDLCHK( MHandle, Model ) )
		{
			return -1 ;
		}
	}

	// 物理データが無い場合は何もしない
	if( Model->BaseData->PhysicsRigidBodyNum == 0 )
	{
		return 0 ;
	}

	MV1SetupMatrix( Model ) ;

	StepSimulation_ModelPhysicsInfo( Model, MillisecondTime / 1000.0f ) ;

	// 更新した行列に関わるスキニングウェイトボーンの行列も更新
	PhysicsRigidBody = Model->PhysicsRigidBody ;
	for( i = 0 ; i < Model->BaseData->PhysicsRigidBodyNum ; i ++, PhysicsRigidBody ++ )
	{
		if( PhysicsRigidBody->BaseData->RigidBodyType == 0 || PhysicsRigidBody->BaseData->NoCopyToBone ) continue ;

		SetupSkiningBoneMatrix( *Model, *PhysicsRigidBody->TargetFrame ) ;
	}
#endif

	// 終了
	return 0 ;
}

#ifndef DX_NON_ASYNCLOAD

// CreateGraph の非同期読み込みスレッドから呼ばれる関数
static void MV1PhysicsCalculationBase_ASync( ASYNCLOADDATA_COMMON *AParam )
{
	int MHandle ;
	float MillisecondTime ;
	int Addr ;
	int Result ;

	Addr = 0 ;
	MHandle         = GetASyncLoadParamInt(   AParam->Data, &Addr ) ;
	MillisecondTime = GetASyncLoadParamFloat( AParam->Data, &Addr ) ;

	Result = MV1PhysicsCalculationBase_Static( MHandle, MillisecondTime, TRUE ) ;
	DecASyncLoadCount( MHandle ) ;
}

#endif // DX_NON_ASYNCLOAD

// モデルの物理演算を指定時間分経過したと仮定して計算する( MillisecondTime で指定する時間の単位はミリ秒 )
extern int MV1PhysicsCalculationBase( int MHandle, float MillisecondTime, int ASyncLoadFlag )
{
#ifndef DX_NON_ASYNCLOAD
	if( ASyncLoadFlag )
	{
		ASYNCLOADDATA_COMMON *AParam = NULL ;
		int Addr ;

		// パラメータに必要なメモリのサイズを算出
		Addr = 0 ;
		AddASyncLoadParamInt(   NULL, &Addr, MHandle ) ;
		AddASyncLoadParamFloat( NULL, &Addr, MillisecondTime ) ;

		// メモリの確保
		AParam = AllocASyncLoadDataMemory( Addr ) ;
		if( AParam == NULL )
			goto ERR ;

		// 処理に必要な情報をセット
		AParam->ProcessFunction = MV1PhysicsCalculationBase_ASync ;
		Addr = 0 ;
		AddASyncLoadParamInt(   AParam->Data, &Addr, MHandle ) ;
		AddASyncLoadParamFloat( AParam->Data, &Addr, MillisecondTime ) ;

		// データを追加
		if( AddASyncLoadData( AParam ) < 0 )
		{
			DXFREE( AParam ) ;
			AParam = NULL ;
			goto ERR ;
		}

		// 非同期読み込みカウントをインクリメント
		IncASyncLoadCount( MHandle, AParam->Index ) ;
	}
	else
#endif // DX_NON_ASYNCLOAD
	{
		if( MV1PhysicsCalculationBase_Static( MHandle, MillisecondTime, FALSE ) < 0 )
		{
			goto ERR ;
		}
	}

	// 正常終了
	return 0 ;

ERR :
	return -1 ;
}

// モデルの物理演算の状態をリセットする( 位置がワープしたとき用 )
extern int NS_MV1PhysicsResetState( int MHandle )
{
#ifndef DX_NON_BULLET_PHYSICS
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// 物理データが無い場合は何もしない
	if( Model->BaseData->PhysicsRigidBodyNum == 0 )
		return 0 ;

	MV1SetupMatrix( Model ) ;
	ResetState_ModelPhysicsInfo( Model ) ;
#endif

	// 終了
	return 0 ;
}

// モデルのシェイプ機能を使用するかどうかを設定する
extern int NS_MV1SetUseShapeFlag( int MHandle, int Flag )
{
	int i ;
	MV1_FRAME *Frame ;

	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// フラグが同じ場合は何もしない
	if( Model->ShapeDisableFlag == ( Flag == 0 ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// フラグを保存する
	Model->ShapeDisableFlag = ( Flag == 0 ) ;

	// シェイプの状態に変化があったフラグを立てる
	Model->ShapeChangeFlag = true ;
	Frame = Model->Frame ;
	for( i = 0 ; i < ModelBase->FrameNum ; i ++, Frame ++ )
	{
		if( Frame->BaseData->MeshNum > 0 && Frame->Mesh->BaseData->Shape == 0 )
			continue ;

		Frame->ShapeChangeFlag = true ;
	}

	// 終了
	return 0 ;
}

// モデルのマテリアル番号順にメッシュを描画するかどうかのフラグを取得する( TRUE:マテリアル番号順に描画  FALSE:不透明メッシュの後半透明メッシュ )
extern int NS_MV1GetMaterialNumberOrderFlag( int MHandle )
{
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	// フラグを返す
	return ModelBase->MaterialNumberOrderDraw ;
}






















// ライトの数を取得する
extern int MV1GetLightNum( int MHandle )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// ライトの数を返す
	return ModelBase->LightNum ;
}

// アニメーションをアタッチする
extern int NS_MV1AttachAnim( int MHandle, int AnimIndex, int AnimSrcMHandle, int NameCheck )
{
	MV1_MODEL *Model, *AModel ;
	MV1_MODEL_BASE *MBase, *AMBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_ANIM_BASE **ABaseTable ;
	MV1_MODEL_ANIM *MAnim, *Dest, *Src ;
	MV1_MODEL_ANIMSET *MAnimSet ;
	MV1_FRAME *Frame, *AFrame = NULL, *CFrame, *CAFrame = NULL ;
	int i, j, k, l, UnitSize, MaxNum, NewMaxNum, AttachIndex, Count ;
	MV1_FRAME *FrameStack[ 1024 ], *AFrameStack[ 1024 ] ;
	int StackCount[ 1024 ] ;
	int StackNum ;
	MV1_ANIM_BASE *AAnimBase ;
	MV1_ANIM_KEYSET_BASE *AKeySetBase ;
	MV1_FRAME_BASE *AFrameBase, *FrameBase ;
	const wchar_t *AShapeName ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	MBase = Model->BaseData ;
	if( AnimSrcMHandle < 0 )
	{
		AModel = Model ;
		AMBase = AModel->BaseData ;
		NameCheck = FALSE ;
	}
	else
	{
		if( MV1MDLCHK( AnimSrcMHandle, AModel ) )
			return -1 ;
		AMBase = AModel->BaseData ;
	}

	// インデックスが不正だったらエラー
	if( AnimIndex < 0 || AnimIndex >= AMBase->AnimSetNum )
		return -1 ;

	// 指定のアニメーションを取得する
	AnimSetBase = MV1GetAnimSetBase( AModel->BaseDataHandle, NULL, AnimIndex ) ;
	if( AnimSetBase == NULL ) return -1 ;
	ABaseTable = AMBase->AnimTargetFrameTable + AMBase->FrameNum * AnimIndex ;

	// データポインタ配列拡張
	if( Model->AnimSetNum >= Model->AnimSetMaxNum )
	{
		UnitSize       = ( int )( sizeof( MV1_MODEL_ANIMSET ) + sizeof( MV1_MODEL_ANIM ) * MBase->FrameNum ) ;
		Model->AnimSet = ( MV1_MODEL_ANIMSET * )DXREALLOC( Model->AnimSet, ( size_t )( UnitSize * ( Model->AnimSetMaxNum + MV1_ANIMSET_NUM_UNIT ) ) ) ;
		if( Model->AnimSet == NULL ) return -1 ;
		
		// 新たに確保された領域に今までの情報をコピー
		MaxNum = Model->AnimSetMaxNum ;
		NewMaxNum = MaxNum + MV1_ANIMSET_NUM_UNIT ;

		// 奥から移動
		Src  = ( MV1_MODEL_ANIM * )( Model->AnimSet + MaxNum    ) + MaxNum    * ( MBase->FrameNum - 1 ) ;
		Dest = ( MV1_MODEL_ANIM * )( Model->AnimSet + NewMaxNum ) + NewMaxNum * ( MBase->FrameNum - 1 ) ;
		for( i = 0 ; i < MBase->FrameNum ; i ++ )
		{
			for( j = NewMaxNum - 1 ; j >= MaxNum ; j -- )
				_MEMSET( &Dest[ j ], 0, sizeof( MV1_MODEL_ANIM ) ) ;

			for( j = MaxNum    - 1 ; j >=      0 ; j -- )
				Dest[ j ] = Src[ j ] ;

			Dest -= NewMaxNum ;
			Src  -= MaxNum    ;
		}
		for( i = NewMaxNum - 1 ; i >= MaxNum ; i -- )
			_MEMSET( &Model->AnimSet[ i ], 0, sizeof( MV1_MODEL_ANIMSET ) ) ;

		// 新たに確保したアドレスをセット
		Model->Anim = ( MV1_MODEL_ANIM * )( Model->AnimSet + NewMaxNum ) ;

		// 最大数を加算
		Model->AnimSetMaxNum += MV1_ANIMSET_NUM_UNIT ;
	}

	// 使用されていないアタッチホルダを探す
	for( AttachIndex = 0 ; Model->AnimSet[ AttachIndex ].Use ; AttachIndex ++ ){}

	// アドレスをセット
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// アニメーションセットの作成
	MAnimSet->AnimSet = MV1CreateAnimSet( AnimSetBase ) ; 
	if( MAnimSet->AnimSet == NULL ) return -1 ;

	// 残りの情報を初期化
	MAnimSet->Use = true ;
	MAnimSet->DisableShapeFlag = false ;
	MAnimSet->BaseDataHandle = AModel->BaseDataHandle ;
	MAnimSet->BaseDataAnimIndex = AnimSetBase->Index ;
	MAnimSet->UseAnimNum = 0 ;

	// 各フレームのアニメーション情報の初期化
	MAnim = &Model->Anim[ AttachIndex ] ;
	for( i = 0 ; i < MBase->FrameNum ; i ++, MAnim += Model->AnimSetMaxNum )
	{
		MAnim->Anim = NULL ;
		MAnim->BlendRate = 1.0f ;
		MAnim->Use = false ;
	}

	// 名前のみで検索するかどうかで処理を分岐
/*	if( MBase->AnimAttachNameSearch )
	{
		// 各フレームのアニメーションのアドレスをセット
		Frame = Model->Frame ;
		MAnim = &Model->Anim[ AttachIndex ] ;
		for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++, MAnim += Model->AnimSetMaxNum )
		{
			MAnim->Anim = MV1GetAnimSetAnim( MAnimSet->AnimSet, Frame->BaseData->Name ) ;
			MAnim->Use = MAnim->Anim != NULL ;
			if( MAnim->Use )
			{
				MAnim->Anim->Frame = Frame ;
				MAnimSet->UseAnimNum ++ ;
			}
		}
	}
	else*/
	// 名前をチェックするかどうかで処理を分岐
	if( NameCheck == FALSE )
	{
		// チェックしない

		for( i = 0 ; i < Model->TopFrameNum && i < AModel->TopFrameNum ; i ++ )
		{
			Frame = Model->TopFrameList[ i ] ;
			AFrame = AModel->TopFrameList[ i ] ;
			if( ABaseTable[ AFrame->BaseData->Index ] != NULL )
			{
				MAnim = &Model->Anim[ AttachIndex ] + Model->AnimSetMaxNum * Frame->BaseData->Index ;
				MAnim->Use = true ;
				MAnim->Anim = MAnimSet->AnimSet->Anim + ( ABaseTable[ AFrame->BaseData->Index ] - AnimSetBase->Anim ) ;
				MAnim->Anim->Frame = Frame ;

				AAnimBase = MAnim->Anim->BaseData ;
				AKeySetBase = AAnimBase->KeySet ;
				for( k = 0 ; k < AAnimBase->KeySetNum ; k ++, AKeySetBase ++ )
				{
					if( AKeySetBase->DataType != MV1_ANIMKEY_DATATYPE_SHAPE ) continue ;
					MAnim->Anim->KeySet[ k ].ShapeTargetIndex = AKeySetBase->TargetShapeIndex ;
				}

				MAnimSet->UseAnimNum ++ ;
			}

			FrameStack[ 0 ] = Frame ;
			AFrameStack[ 0 ] = AFrame ;
			StackCount[ 0 ] = 0 ;
			StackNum = 1 ;
			while( StackNum != 0 )
			{
				Frame = FrameStack[ StackNum - 1 ] ;
				AFrame = AFrameStack[ StackNum - 1 ] ;

				Count = StackCount[ StackNum - 1 ] ;
				if( Count < Frame->ChildNum && Count < AFrame->ChildNum )
				{
					StackCount[ StackNum - 1 ] ++ ;

					CFrame = Frame->ChildList[ Count ] ;
					CAFrame = AFrame->ChildList[ Count ] ;
					if( ABaseTable[ CAFrame->BaseData->Index ] )
					{
						MAnim = &Model->Anim[ AttachIndex ] + Model->AnimSetMaxNum * CFrame->BaseData->Index ;
						MAnim->Use = true ;
						MAnim->Anim = MAnimSet->AnimSet->Anim + ( ABaseTable[ CAFrame->BaseData->Index ] - AnimSetBase->Anim ) ;
						MAnim->Anim->Frame = CFrame ;

						AAnimBase = MAnim->Anim->BaseData ;
						AKeySetBase = AAnimBase->KeySet ;
						for( k = 0 ; k < AAnimBase->KeySetNum ; k ++, AKeySetBase ++ )
						{
							if( AKeySetBase->DataType != MV1_ANIMKEY_DATATYPE_SHAPE ) continue ;
							MAnim->Anim->KeySet[ k ].ShapeTargetIndex = AKeySetBase->TargetShapeIndex ;
						}

						MAnimSet->UseAnimNum ++ ;
					}

					FrameStack[ StackNum ] = CFrame ;
					AFrameStack[ StackNum ] = CAFrame ;
					StackCount[ StackNum ] = 0 ;
					StackNum ++ ;
				}
				else
				{
					StackNum -- ;
				}
			}
		}
	}
	else
	{
		// 名前をチェックする

		for( i = 0 ; i < Model->TopFrameNum ; i ++ )
		{
			Frame = Model->TopFrameList[ i ] ;
			FrameBase = Frame->BaseData ;

			for( j = 0 ; j < AModel->TopFrameNum ; j ++ )
			{
				AFrame = AModel->TopFrameList[ j ] ;
				if( _WCSCMP( Frame->BaseData->NameW, AFrame->BaseData->NameW ) == 0 )
					break ;
			}
			if( j == AModel->TopFrameNum ) continue ;

			if( ABaseTable[ AFrame->BaseData->Index ] )
			{
				MAnim = &Model->Anim[ AttachIndex ] + Model->AnimSetMaxNum * Frame->BaseData->Index ;
				MAnim->Use = true ;
				MAnim->Anim = MAnimSet->AnimSet->Anim + ( ABaseTable[ AFrame->BaseData->Index ] - AnimSetBase->Anim ) ;
				MAnim->Anim->Frame = Frame ;

				AAnimBase = MAnim->Anim->BaseData ;
				AKeySetBase = AAnimBase->KeySet ;
				for( k = 0 ; k < AAnimBase->KeySetNum ; k ++, AKeySetBase ++ )
				{
					if( AKeySetBase->DataType != MV1_ANIMKEY_DATATYPE_SHAPE ) continue ;

					AFrameBase = &AMBase->Frame[ AAnimBase->TargetFrameIndex ] ;
					AShapeName = AFrameBase->Shape[ AKeySetBase->TargetShapeIndex ].NameW ;
					for( l = 0 ; l < FrameBase->ShapeNum && _WCSCMP( FrameBase->Shape[ l ].NameW, AShapeName ) != 0 ; l ++ ){}
					if( l != FrameBase->ShapeNum )
					{
						MAnim->Anim->KeySet[ k ].ShapeTargetIndex = l ;
					}
				}

				MAnimSet->UseAnimNum ++ ;
			}

			FrameStack[ 0 ] = Frame ;
			AFrameStack[ 0 ] = AFrame ;
			StackCount[ 0 ] = 0 ;
			StackNum = 1 ;
			while( StackNum != 0 )
			{
				Frame = FrameStack[ StackNum - 1 ] ;
				AFrame = AFrameStack[ StackNum - 1 ] ;

				Count = StackCount[ StackNum - 1 ] ;
				if( Count < Frame->ChildNum )
				{
					StackCount[ StackNum - 1 ] ++ ;

					CFrame = Frame->ChildList[ Count ] ;
					for( j = 0 ; j < AFrame->ChildNum ; j ++ )
					{
						CAFrame = AFrame->ChildList[ j ] ;
						if( _WCSCMP( CFrame->BaseData->NameW, CAFrame->BaseData->NameW ) == 0 )
							break ;
					}
					if( j != AFrame->ChildNum )
					{
						if( ABaseTable[ CAFrame->BaseData->Index ] )
						{
							MAnim = &Model->Anim[ AttachIndex ] + Model->AnimSetMaxNum * CFrame->BaseData->Index ;
							MAnim->Use = true ;
							MAnim->Anim = MAnimSet->AnimSet->Anim + ( ABaseTable[ CAFrame->BaseData->Index ] - AnimSetBase->Anim ) ;
							MAnim->Anim->Frame = CFrame ;

							AAnimBase = MAnim->Anim->BaseData ;
							AKeySetBase = AAnimBase->KeySet ;
							for( k = 0 ; k < AAnimBase->KeySetNum ; k ++, AKeySetBase ++ )
							{
								if( AKeySetBase->DataType != MV1_ANIMKEY_DATATYPE_SHAPE ) continue ;

								AFrameBase = &AMBase->Frame[ AAnimBase->TargetFrameIndex ] ;
								AShapeName = AFrameBase->Shape[ AKeySetBase->TargetShapeIndex ].NameW ;
								for( l = 0 ; l < FrameBase->ShapeNum && _WCSCMP( FrameBase->Shape[ l ].NameW, AShapeName ) != 0 ; l ++ ){}
								if( l != FrameBase->ShapeNum )
								{
									MAnim->Anim->KeySet[ k ].ShapeTargetIndex = l ;
								}
							}

							MAnimSet->UseAnimNum ++ ;
						}

						FrameStack[ StackNum ] = CFrame ;
						AFrameStack[ StackNum ] = CAFrame ;
						StackCount[ StackNum ] = 0 ;
						StackNum ++ ;
					}
				}
				else
				{
					StackNum -- ;
				}
			}
		}
	}
/*
	// 各フレームのアニメーションのアドレスをセット
	Frame = Model->Frame ;
	MAnim = &Model->Anim[ AttachIndex ] ;
	for( i = 0 ; i < MBase->FrameNum ; i ++, Frame ++, MAnim += Model->AnimSetMaxNum )
	{
		MAnim->Anim = MV1GetAnimSetAnim( MAnimSet->AnimSet, Frame->BaseData->Name ) ;
		MAnim->Use = MAnim->Anim != NULL ;
		if( MAnim->Use )
		{
			MAnim->Anim->Frame = Frame ;
			MAnimSet->UseAnimNum ++ ;
		}
	}
*/

	// アタッチされているアニメーションの数をインクリメント
	Model->AnimSetNum ++ ;

	// アニメーションパラメータを元にした行列がセットアップされていない状態にする
	Model->AnimSetupFlag = false ;
	Model->LocalWorldMatrixSetupFlag = false ;

	// アニメーションの時間を初期化する
	MV1SetAnimSetTime( MAnimSet->AnimSet, 0.0f ) ;

	// アタッチしたインデックスを返す
	return AttachIndex ;
}

// アニメーションをデタッチする
extern int NS_MV1DetachAnim( int MHandle, int AttachIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIM *MAnim ;
	int i, FrameNum ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;

	// アタッチされていたらメモリを解放
	if( Model->AnimSet[ AttachIndex ].Use )
	{
		MDFREEMEM( Model->AnimSet[ AttachIndex ].AnimSet ) ;
		Model->AnimSet[ AttachIndex ].AnimSet = NULL ;
		Model->AnimSet[ AttachIndex ].Use = false ;

		// 各フレームのアニメーション情報の初期化
		MAnim = &Model->Anim[ AttachIndex ] ;
		FrameNum = Model->BaseData->FrameNum ;
		for( i = 0 ; i < FrameNum ; i ++, MAnim += Model->AnimSetMaxNum )
		{
			MAnim->Anim = NULL ;
			MAnim->Use = false ;
		}

		// アタッチされているアニメーションの数をデクリメント
		Model->AnimSetNum -- ;
	}

	// アニメーションパラメータを元にした行列がセットアップされていない状態にする
	Model->AnimSetupFlag = false ;
	Model->LocalWorldMatrixSetupFlag = false ;

	// 正常終了
	return 0 ;
}





















// 同時複数描画の為に描画待機しているモデルを描画する
extern int MV1DrawPackDrawModel( void )
{
	MV1_MODEL *Model ;

	if( MV1Man.PackDrawModel == NULL )
	{
		return 0 ;
	}

	Model = MV1Man.PackDrawModel ;
	MV1Man.PackDrawModel = NULL ;

	return MV1DrawModelBase( Model ) ;
}

// モデルの描画処理を行う
static int MV1DrawModelBase( MV1_MODEL *Model )
{
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_MESH *Mesh ;
	int i, j, k, l ;
	VECTOR CheckPos[ 8 ], Tmp ;
	MV1_MESH **OpacityMeshList ;
	MV1_MESH **SemiTransBaseOpacityMeshList ;
	MV1_MESH **SemiTransMeshList ;
	int OpacityMeshNum ;
	int SemiTransBaseOpacityMeshNum ;
	int SemiTransMeshNum ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	ModelBase = Model->BaseData ;

	// 使用しているテクスチャの中にムービーが含まれていたらその更新チェックをする
#ifndef DX_NON_MOVIE
	{
		int ind ;
		IMAGEDATA *Image ;

		for( ind = 0 ; ind < ModelBase->TextureNum ; ind ++ )
		{
			if( Model->Texture[ ind ].UseUserGraphHandle )
			{
				Image = Graphics_Image_GetData( Model->Texture[ ind ].UserGraphHandle ) ;
				if( Image != NULL && Image->MovieHandle != -1 )
				{
					UpdateMovie( Image->MovieHandle, FALSE ) ;
				}
			}
		}
	}
#endif

	// 描画するメッシュのアドレス配列を格納するメモリ領域の確保
	if( DrawMeshListResize( Model->BaseData->MeshNum ) < 0 )
	{
		return -1 ;
	}
	OpacityMeshList              = MV1Man.DrawMeshList ;
	SemiTransBaseOpacityMeshList = OpacityMeshList              + Model->BaseData->MeshNum ;
	SemiTransMeshList            = SemiTransBaseOpacityMeshList + Model->BaseData->MeshNum ;
	OpacityMeshNum               = 0 ;
	SemiTransBaseOpacityMeshNum  = 0 ;
	SemiTransMeshNum             = 0 ;

	// シェイプデータのセットアップ
	if( Model->BaseData->ShapeMeshNum != 0 )
		MV1_SetupShapeVertex_PF( Model->HandleInfo.Handle ) ;

	// レンダリングの準備
	MV1_BeginRender_PF( Model ) ;

	// 描画するメッシュのリストを作成する
	Frame = Model->Frame ;
	for( i = 0 ; i < Model->BaseData->FrameNum ; i ++ , Frame ++ )
	{
		// メッシュが無かったら飛ばす
		if( Frame->BaseData->MeshNum == 0 ) continue ;

		// マテリアル更新チェック
		MV1SETUPDRAWMATERIALFRAME( Frame ) ;

		// 非表示指定だったら何もしない
		if( Frame->SetupDrawMaterial.Visible == FALSE ) continue ;

		// 同時複数描画対応ハンドルではなく、シャドウマップへの描画中でもなくスキンメッシュでもない場合は可視判定
		if( ModelBase->UsePackDraw == FALSE && GSYS.DrawSetting.ShadowMapDraw == FALSE && Frame->BaseData->IsSkinMesh == FALSE )
		{
			// 可視チェック用頂点座標を算出する
			Tmp   = Frame->BaseData->MaxPosition   ; VectorTransform4X4CT( &CheckPos[ 0 ], &Tmp, &Frame->LocalWorldMatrix ) ;
			Tmp.x = Frame->BaseData->MinPosition.x ; VectorTransform4X4CT( &CheckPos[ 1 ], &Tmp, &Frame->LocalWorldMatrix ) ;
			Tmp.y = Frame->BaseData->MinPosition.y ; VectorTransform4X4CT( &CheckPos[ 2 ], &Tmp, &Frame->LocalWorldMatrix ) ;
			Tmp.x = Frame->BaseData->MaxPosition.x ; VectorTransform4X4CT( &CheckPos[ 3 ], &Tmp, &Frame->LocalWorldMatrix ) ;
			Tmp.z = Frame->BaseData->MinPosition.z ; VectorTransform4X4CT( &CheckPos[ 4 ], &Tmp, &Frame->LocalWorldMatrix ) ;
			Tmp.y = Frame->BaseData->MaxPosition.y ; VectorTransform4X4CT( &CheckPos[ 5 ], &Tmp, &Frame->LocalWorldMatrix ) ;
			Tmp.x = Frame->BaseData->MinPosition.x ; VectorTransform4X4CT( &CheckPos[ 6 ], &Tmp, &Frame->LocalWorldMatrix ) ;
			Tmp.y = Frame->BaseData->MinPosition.y ; VectorTransform4X4CT( &CheckPos[ 7 ], &Tmp, &Frame->LocalWorldMatrix ) ;

			// 可視チェック
			if( Graphics_Camera_CheckCameraViewClip_Box_PosDim( CheckPos ) == TRUE )
				continue ;
		}

		// MaterialNumberOrderDraw が立っているかどうかで処理を分岐
		if( Model->BaseData->MaterialNumberOrderDraw != 0 )
		{
			int MaterialIndex ;

			// MaterialNumberOrderDraw が立っている場合はメッシュに割り当てられているマテリアルの番号が低い順位描画する

			// メッシュの数だけ繰り返し
			Mesh = Frame->Mesh ;
			for( k = 0 ; k < Frame->BaseData->MeshNum ; k ++ , Mesh ++ )
			{
				// マテリアル更新チェック
				MV1SETUPDRAWMATERIALMESH( Mesh ) ;

				// 非表示の場合は何もしない
				if( Mesh->SetupDrawMaterial.Visible == 0 ) continue ;

				// マテリアル番号でソート挿入
				MaterialIndex = ( int )( Mesh->Material->BaseData - Model->BaseData->Material ) ;
				for( j = 0 ; j < OpacityMeshNum ; j ++ )
				{
					if( MaterialIndex < OpacityMeshList[ j ]->Material->BaseData - Model->BaseData->Material )
						break ;
				}

				for( l = OpacityMeshNum - 1 ; l >= j ; l -- )
				{
					OpacityMeshList[ l + 1 ] = OpacityMeshList[ l ] ;
				}

				OpacityMeshList[ j ] = Mesh ;
				OpacityMeshNum ++ ;
			}
		}
		else
		{
			// MaterialNumberOrderDraw が立っていない場合は不透明のメッシュを描画した後に半透明要素のあるメッシュを描画する

			// メッシュの数だけ繰り返し
			Mesh = Frame->Mesh ;
			for( k = 0 ; k < Frame->BaseData->MeshNum ; k ++ , Mesh ++ )
			{
				// マテリアル更新チェック
				MV1SETUPDRAWMATERIALMESH( Mesh ) ;

				// 非表示の場合は何もしない
				if( Mesh->SetupDrawMaterial.Visible == 0 ) continue ;

				// 半透明要素があるかどうかを調べる
				if( Mesh->SemiTransStateSetupFlag == false )
				{
					NS_MV1GetMeshSemiTransState( Model->HandleInfo.Handle, ( int )( Mesh - Model->Mesh ) ) ;
				}

				if( Mesh->SemiTransState == false )
				{
					OpacityMeshList[ OpacityMeshNum ] = Mesh ;
					OpacityMeshNum ++ ;
				}
				else
				if( Mesh->BaseData->SemiTransState == 0 )
				{
					SemiTransBaseOpacityMeshList[ SemiTransBaseOpacityMeshNum ] = Mesh ;
					SemiTransBaseOpacityMeshNum ++ ;
				}
				else
				{
					SemiTransMeshList[ SemiTransMeshNum ] = Mesh ;
					SemiTransMeshNum ++ ;
				}
			}
		}
	}

	// 不透明オブジェクトの描画
	if( MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_ALWAYS ||
		MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_NOT_SEMITRANS_ONLY )
	{
		for( i = 0 ; i < OpacityMeshNum ; i ++ )
		{
			MV1_DrawMesh_PF( OpacityMeshList[ i ] ) ;
		}
	}

	// 半透明オブジェクトの描画
	if( MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_ALWAYS ||
		MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_SEMITRANS_ONLY )
	{
		// 半透明オブジェクトながら初期状態では不透明のオブジェクトを先に描画
		for( i = 0 ; i < SemiTransBaseOpacityMeshNum ; i ++ )
		{
			MV1_DrawMesh_PF( SemiTransBaseOpacityMeshList[ i ] ) ;
		}

		for( i = 0 ; i < SemiTransMeshNum ; i ++ )
		{
			MV1_DrawMesh_PF( SemiTransMeshList[ i ] ) ;
		}
	}

	// レンダリングの後始末
	MV1_EndRender_PF() ;

	// 描画ストックの初期化
	Model->PackDrawStockNum = 0 ;

	// 終了
	return 0 ;
}

// モデルを描画する
extern int NS_MV1DrawModel( int MHandle )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 非表示設定だったら描画しない
	if( Model->DrawMaterial.Visible == 0 )
		return 0 ;

	// 頂点データの描画を終わらせておく
	Graphics_Hardware_RenderVertex() ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// 同時複数描画に対応している場合はストックに情報を追加する
	if( ModelBase->UsePackDraw )
	{
		MATRIX_4X4CT_F *DestMatrix ;

		// 既に描画待ちされているモデルがあったら描画する
		if( MV1Man.PackDrawModel != NULL && MV1Man.PackDrawModel != Model )
		{
			MV1DrawPackDrawModel() ;
		}

		// ストックの数が限界に達していたら描画処理を行う
		if( Model->PackDrawStockNum >= ModelBase->PackDrawMaxNum )
		{
			MV1DrawPackDrawModel() ;
		}

		// ストック格納先のアドレスを算出
		DestMatrix = Model->PackDrawMatrix + Model->PackDrawStockNum * ModelBase->PackDrawMatrixUnitNum ;

		// スキニングメッシュの行列をコピーする
		if( ModelBase->SkinBoneNum > 0 )
		{
			for( i = 0 ; i < ModelBase->SkinBoneNum ; i ++ )
			{
				ConvertMatrix4x4cToMatrix4x4cF( &DestMatrix[ i ], &Model->SkinBoneMatrix[ i ] ) ;
			}
		}
		DestMatrix += ModelBase->SkinBoneNum ;

		// フレームの行列をコピーする
		Frame = Model->Frame ;
		for( i = 0 ; i < ModelBase->FrameNum ; i ++ )
		{
			ConvertMatrix4x4cToMatrix4x4cF( &DestMatrix[ i ], &Frame[ i ].LocalWorldMatrix ) ;
		}

		// ストックの数を増やす
		Model->PackDrawStockNum ++ ;

		// 同時複数描画の為に描画待機をしているモデルのアドレスとして保存
		MV1Man.PackDrawModel = Model ;
	}
	else
	{
		// 同時複数描画に対応していない場合は普通に描画する
		MV1DrawModelBase( Model ) ;
	}

	// 終了
	return 0 ;
}

// モデルの指定のフレームを描画する
extern int NS_MV1DrawFrame( int MHandle, int FrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_MESH *Mesh ;
	int k ;
	VECTOR CheckPos[ 8 ], Tmp ;
	MV1_MESH **OpacityMeshList ;
	MV1_MESH **SemiTransBaseOpacityMeshList ;
	MV1_MESH **SemiTransMeshList ;
	int OpacityMeshNum ;
	int SemiTransBaseOpacityMeshNum ;
	int SemiTransMeshNum ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 非表示設定だったら描画しない
	if( Model->DrawMaterial.Visible == 0 )
		return 0 ;

	// 使用しているテクスチャの中にムービーが含まれていたらその更新チェックをする
#ifndef DX_NON_MOVIE
	{
		int ind ;
		IMAGEDATA *Image ;

		for( ind = 0 ; ind < ModelBase->TextureNum ; ind ++ )
		{
			if( Model->Texture[ ind ].UseUserGraphHandle )
			{
				Image = Graphics_Image_GetData( Model->Texture[ ind ].UserGraphHandle ) ;
				if( Image != NULL && Image->MovieHandle != -1 )
				{
					UpdateMovie( Image->MovieHandle, FALSE ) ;
				}
			}
		}
	}
#endif

	// フレームインデックスのチェック
	if( FrameIndex < 0 || FrameIndex >= Model->BaseData->FrameNum )
		return -1 ;
	Frame = Model->Frame + FrameIndex ;

	// メッシュが無かったら何もしない
	if( Frame->BaseData->MeshNum == 0 )
		return -1 ;

	// 描画するメッシュのアドレス配列を格納するメモリ領域の確保
	if( DrawMeshListResize( Model->BaseData->MeshNum ) < 0 )
	{
		return -1 ;
	}
	OpacityMeshList              = MV1Man.DrawMeshList ;
	SemiTransBaseOpacityMeshList = OpacityMeshList              + Model->BaseData->MeshNum ;
	SemiTransMeshList            = SemiTransBaseOpacityMeshList + Model->BaseData->MeshNum ;
	OpacityMeshNum               = 0 ;
	SemiTransBaseOpacityMeshNum  = 0 ;
	SemiTransMeshNum             = 0 ;

	// マテリアル更新チェック
	MV1SETUPDRAWMATERIALFRAME( Frame ) ;

	// 非表示設定だったら描画しない
	if( Frame->SetupDrawMaterial.Visible == 0 )
		return 0 ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// シャドウマップへの描画中ではなくスキンメッシュでもない場合は可視判定
	if( GSYS.DrawSetting.ShadowMapDraw == FALSE && Frame->BaseData->IsSkinMesh == FALSE )
	{
		// 可視チェック用頂点座標を算出する
		Tmp   = Frame->BaseData->MaxPosition   ; VectorTransform4X4CT( &CheckPos[ 0 ], &Tmp, &Frame->LocalWorldMatrix ) ;
		Tmp.x = Frame->BaseData->MinPosition.x ; VectorTransform4X4CT( &CheckPos[ 1 ], &Tmp, &Frame->LocalWorldMatrix ) ;
		Tmp.y = Frame->BaseData->MinPosition.y ; VectorTransform4X4CT( &CheckPos[ 2 ], &Tmp, &Frame->LocalWorldMatrix ) ;
		Tmp.x = Frame->BaseData->MaxPosition.x ; VectorTransform4X4CT( &CheckPos[ 3 ], &Tmp, &Frame->LocalWorldMatrix ) ;
		Tmp.z = Frame->BaseData->MinPosition.z ; VectorTransform4X4CT( &CheckPos[ 4 ], &Tmp, &Frame->LocalWorldMatrix ) ;
		Tmp.y = Frame->BaseData->MaxPosition.y ; VectorTransform4X4CT( &CheckPos[ 5 ], &Tmp, &Frame->LocalWorldMatrix ) ;
		Tmp.x = Frame->BaseData->MinPosition.x ; VectorTransform4X4CT( &CheckPos[ 6 ], &Tmp, &Frame->LocalWorldMatrix ) ;
		Tmp.y = Frame->BaseData->MinPosition.y ; VectorTransform4X4CT( &CheckPos[ 7 ], &Tmp, &Frame->LocalWorldMatrix ) ;

		// 可視チェック
		if( Graphics_Camera_CheckCameraViewClip_Box_PosDim( CheckPos ) == TRUE )
			return -1 ;
	}

	// シェイプデータのセットアップ
	if( Model->BaseData->ShapeMeshNum != 0 )
		MV1_SetupShapeVertex_PF( MHandle ) ;

	// レンダリングの準備
	MV1_BeginRender_PF( Model ) ;

	// 描画するメッシュの振り分け
	Mesh = Frame->Mesh ;
	for( k = 0 ; k < Frame->BaseData->MeshNum ; k ++ , Mesh ++ )
	{
		// マテリアル更新チェック
		MV1SETUPDRAWMATERIALMESH( Mesh ) ;

		// 非表示の場合は何もしない
		if( Mesh->SetupDrawMaterial.Visible == 0 ) continue ;

		// 半透明要素があるかどうかを調べる
		if( Mesh->SemiTransStateSetupFlag == false )
		{
			NS_MV1GetMeshSemiTransState( MHandle, ( int )( Mesh - Model->Mesh ) ) ;
		}

		if( Mesh->SemiTransState == false )
		{
			OpacityMeshList[ OpacityMeshNum ] = Mesh ;
			OpacityMeshNum ++ ;
		}
		else
		if( Mesh->BaseData->SemiTransState == 0 )
		{
			SemiTransBaseOpacityMeshList[ SemiTransBaseOpacityMeshNum ] = Mesh ;
			SemiTransBaseOpacityMeshNum ++ ;
		}
		else
		{
			SemiTransMeshList[ SemiTransMeshNum ] = Mesh ;
			SemiTransMeshNum ++ ;
		}
	}

	// 不透明メッシュの描画
	if( MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_ALWAYS ||
		MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_NOT_SEMITRANS_ONLY )
	{
		for( k = 0 ; k < OpacityMeshNum ; k ++ )
		{
			MV1_DrawMesh_PF( OpacityMeshList[ k ] ) ;
		}
	}

	// 半透明メッシュの描画
	if( MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_ALWAYS ||
		MV1Man.SemiTransDrawMode == DX_SEMITRANSDRAWMODE_SEMITRANS_ONLY )
	{
		// 半透明オブジェクトながら初期状態では不透明のオブジェクトを先に描画
		for( k = 0 ; k < SemiTransBaseOpacityMeshNum ; k ++ )
		{
			MV1_DrawMesh_PF( SemiTransBaseOpacityMeshList[ k ] ) ;
		}

		for( k = 0 ; k < SemiTransMeshNum ; k ++ )
		{
			MV1_DrawMesh_PF( SemiTransMeshList[ k ] ) ;
		}
	}

	// レンダリングの後始末
	MV1_EndRender_PF() ;

	// 終了
	return 0 ;
}

// モデルの指定のメッシュを描画する
extern int NS_MV1DrawMesh( int MHandle, int MeshIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_MESH *Mesh ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 非表示設定だったら描画しない
	if( Model->DrawMaterial.Visible == 0 )
		return 0 ;

	// 使用しているテクスチャの中にムービーが含まれていたらその更新チェックをする
#ifndef DX_NON_MOVIE
	{
		int ind ;
		IMAGEDATA *Image ;

		for( ind = 0 ; ind < ModelBase->TextureNum ; ind ++ )
		{
			if( Model->Texture[ ind ].UseUserGraphHandle )
			{
				Image = Graphics_Image_GetData( Model->Texture[ ind ].UserGraphHandle ) ;
				if( Image != NULL && Image->MovieHandle != -1 )
				{
					UpdateMovie( Image->MovieHandle, FALSE ) ;
				}
			}
		}
	}
#endif

	// メッシュインデックスのチェック
	if( MeshIndex < 0 || MeshIndex >= ModelBase->MeshNum )
		return -1 ;
	Mesh = Model->Mesh + MeshIndex ;
	Frame = Mesh->Container ;

	// マテリアル更新チェック
	MV1SETUPDRAWMATERIALMESH( Mesh ) ;

	// 非表示の場合は何もしない
	if( Mesh->SetupDrawMaterial.Visible == 0 )
		return 0 ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// シェイプデータのセットアップ
	if( Model->BaseData->ShapeMeshNum != 0 )
		MV1_SetupShapeVertex_PF( MHandle ) ;

	// レンダリングの準備
	MV1_BeginRender_PF( Model ) ;

	// メッシュの描画
	MV1_DrawMesh_PF( Mesh ) ;

	// レンダリングの後始末
	MV1_EndRender_PF() ;

	// 終了
	return 0 ;
}

// モデルの指定のトライアングルリストを描画する
extern int NS_MV1DrawTriangleList( int MHandle, int TriangleListIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_MESH *Mesh ;
	MV1_TRIANGLE_LIST *TList ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 非表示設定だったら描画しない
	if( Model->DrawMaterial.Visible == 0 )
		return 0 ;

	// 使用しているテクスチャの中にムービーが含まれていたらその更新チェックをする
#ifndef DX_NON_MOVIE
	{
		int ind ;
		IMAGEDATA *Image ;

		for( ind = 0 ; ind < ModelBase->TextureNum ; ind ++ )
		{
			if( Model->Texture[ ind ].UseUserGraphHandle )
			{
				Image = Graphics_Image_GetData( Model->Texture[ ind ].UserGraphHandle ) ;
				if( Image != NULL && Image->MovieHandle != -1 )
				{
					UpdateMovie( Image->MovieHandle, FALSE ) ;
				}
			}
		}
	}
#endif

	// トライアングルリストインデックスのチェック
	if( TriangleListIndex < 0 || TriangleListIndex >= ModelBase->TriangleListNum )
		return -1 ;
	TList = ( MV1_TRIANGLE_LIST * )( ( BYTE * )Model->TriangleList + TriangleListIndex * ( sizeof( MV1_TRIANGLE_LIST ) + sizeof( MV1_TRIANGLE_LIST_PF ) ) );
	Mesh = TList->Container ;
	Frame = Mesh->Container ;

	// マテリアル更新チェック
	MV1SETUPDRAWMATERIALMESH( Mesh ) ;

	// 非表示の場合は何もしない
	if( Mesh->SetupDrawMaterial.Visible == 0 )
		return 0 ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// シェイプデータのセットアップ
	if( Model->BaseData->ShapeMeshNum != 0 )
		MV1_SetupShapeVertex_PF( MHandle ) ;

	// レンダリングの準備
	MV1_BeginRender_PF( Model ) ;

	// トライアングルリストの描画
	MV1_DrawMesh_PF( Mesh, ( int )( ( int )( ( ( BYTE * )TList - ( BYTE * )Mesh->TriangleList ) ) / ( sizeof( MV1_TRIANGLE_LIST ) + sizeof( MV1_TRIANGLE_LIST_PF ) ) ) ) ;

	// レンダリングの後始末
	MV1_EndRender_PF() ;

	// 終了
	return 0 ;
}

// モデルのデバッグ描画
extern int NS_MV1DrawModelDebug(
	  int MHandle, unsigned int Color,
	  int IsNormalLine, float NormalLineLength,
	  int IsPolyLine,
	  int IsCollisionBox )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_REF_POLYGONLIST PolyList ;
	MV1_REF_POLYGON *Poly ;
	MV1_REF_VERTEX *Vert ;
	int i, j, VertCount ;
	VERTEX_3D Vertex[ 300 ], *vert ;
	int r, g, b ;
	MATRIX Mat, OrigTransMat ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 元々設定されていたローカル→ワールド行列を保存して単位行列をセットする
	NS_GetTransformToWorldMatrix( &OrigTransMat ) ;
	CreateIdentityMatrix( &Mat ) ;
	NS_SetTransformToWorld( &Mat ) ;

	// 色の分解
	NS_GetColor2( Color, &r, &g, &b ) ;

	// 参照用ポリゴンの取得
	if( IsPolyLine && IsNormalLine == FALSE )
	{
		NS_MV1RefreshReferenceMesh( MHandle, -1, TRUE, TRUE ) ;
		PolyList = NS_MV1GetReferenceMesh( MHandle, -1, TRUE, TRUE ) ;
	}
	else
	{
		NS_MV1RefreshReferenceMesh( MHandle, -1, TRUE, FALSE ) ;
		PolyList = NS_MV1GetReferenceMesh( MHandle, -1, TRUE, FALSE ) ;
	}

	// 法線ラインの描画
	if( IsNormalLine )
	{
		Poly = PolyList.Polygons ;
		Vert = PolyList.Vertexs ;
		VertCount = 0 ;
		_MEMSET( Vertex, 0, sizeof( Vertex ) ) ;
		vert = Vertex ;
		for( i = 0 ; i < PolyList.PolygonNum ; i ++, Poly ++ )
		{
			for( j = 0 ; j < 3 ; j ++ )
			{
				vert[ 0 ].pos = Vert[ Poly->VIndex[ j ] ].Position ;
				vert[ 0 ].r = ( unsigned char )r ;
				vert[ 0 ].g = ( unsigned char )g ;
				vert[ 0 ].b = ( unsigned char )b ;
				vert[ 0 ].a = 255 ;

				vert[ 1 ].pos.x = Vert[ Poly->VIndex[ j ] ].Position.x + Vert[ Poly->VIndex[ j ] ].Normal.x * NormalLineLength ;
				vert[ 1 ].pos.y = Vert[ Poly->VIndex[ j ] ].Position.y + Vert[ Poly->VIndex[ j ] ].Normal.y * NormalLineLength ;
				vert[ 1 ].pos.z = Vert[ Poly->VIndex[ j ] ].Position.z + Vert[ Poly->VIndex[ j ] ].Normal.z * NormalLineLength ;
				vert[ 1 ].r = ( unsigned char )r ;
				vert[ 1 ].g = ( unsigned char )g ;
				vert[ 1 ].b = ( unsigned char )b ;
				vert[ 1 ].a = 255 ;

				vert += 2 ;
			}
			VertCount += 3 * 2 ;
			if( VertCount == 300 )
			{
				NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
				VertCount = 0 ;
				vert = Vertex ;
			}
		}
		if( VertCount )
		{
			NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
		}
	}

	// ポリゴンラインの描画
	if( IsPolyLine )
	{
		Poly = PolyList.Polygons ;
		Vert = PolyList.Vertexs ;
		VertCount = 0 ;
		_MEMSET( Vertex, 0, sizeof( Vertex ) ) ;
		vert = Vertex ;
		for( i = 0 ; i < PolyList.PolygonNum ; i ++, Poly ++ )
		{
			for( j = 0 ; j < 2 ; j ++ )
			{
				vert[ 0 ].pos = Vert[ Poly->VIndex[ j ] ].Position ;
				vert[ 0 ].r = ( unsigned char )r ;
				vert[ 0 ].g = ( unsigned char )g ;
				vert[ 0 ].b = ( unsigned char )b ;
				vert[ 0 ].a = 255 ;

				vert[ 1 ].pos = Vert[ Poly->VIndex[ j + 1 ] ].Position ;
				vert[ 1 ].r = ( unsigned char )r ;
				vert[ 1 ].g = ( unsigned char )g ;
				vert[ 1 ].b = ( unsigned char )b ;
				vert[ 1 ].a = 255 ;

				vert += 2 ;
			}
			vert[ 0 ].pos = Vert[ Poly->VIndex[ 2 ] ].Position ;
			vert[ 0 ].r = ( unsigned char )r ;
			vert[ 0 ].g = ( unsigned char )g ;
			vert[ 0 ].b = ( unsigned char )b ;
			vert[ 0 ].a = 255 ;

			vert[ 1 ].pos = Vert[ Poly->VIndex[ 0 ] ].Position ;
			vert[ 1 ].r = ( unsigned char )r ;
			vert[ 1 ].g = ( unsigned char )g ;
			vert[ 1 ].b = ( unsigned char )b ;
			vert[ 1 ].a = 255 ;

			vert += 2 ;

			VertCount += 3 * 2 ;
			if( VertCount == 300 )
			{
				NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
				VertCount = 0 ;
				vert = Vertex ;
			}
		}
		if( VertCount )
		{
			NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
		}
	}

	// コリジョンボックスの描画
	if( IsCollisionBox )
	{
		if( Model->Collision )
		{
			VECTOR Pos ;
			MV1_COLLISION *Collision ;

			Collision = Model->Collision ;
			_MEMSET( Vertex, 0, sizeof( Vertex ) ) ;
			vert = Vertex ;
			VertCount = 0 ;
			Pos.z = Collision->MinPosition.z ;
			for( i = 0 ; i <= Collision->ZDivNum ; i ++, Pos.z += Collision->UnitSize.z )
			{
				Pos.y = Collision->MinPosition.y ;
				Pos.x = Collision->MinPosition.x ;
				for( j = 0 ; j <= Collision->XDivNum ; j ++, Pos.x += Collision->UnitSize.x )
				{
					vert[ 0 ].pos = Pos ;
					vert[ 0 ].r = ( unsigned char )r ;
					vert[ 0 ].g = ( unsigned char )g ;
					vert[ 0 ].b = ( unsigned char )b ;
					vert[ 0 ].a = 255 ;

					vert[ 1 ].pos = Pos ;
					vert[ 1 ].pos.y = Collision->MaxPosition.y ;
					vert[ 1 ].r = ( unsigned char )r ;
					vert[ 1 ].g = ( unsigned char )g ;
					vert[ 1 ].b = ( unsigned char )b ;
					vert[ 1 ].a = 255 ;

					vert += 2 ;

					VertCount += 2 ;
					if( VertCount == 300 )
					{
						NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
						VertCount = 0 ;
						vert = Vertex ;
					}
				}

				Pos.y = Collision->MinPosition.y ;
				Pos.x = Collision->MinPosition.x ;
				for( j = 0 ; j <= Collision->YDivNum ; j ++, Pos.y += Collision->UnitSize.y )
				{
					vert[ 0 ].pos = Pos ;
					vert[ 0 ].r = ( unsigned char )r ;
					vert[ 0 ].g = ( unsigned char )g ;
					vert[ 0 ].b = ( unsigned char )b ;
					vert[ 0 ].a = 255 ;

					vert[ 1 ].pos = Pos ;
					vert[ 1 ].pos.x = Collision->MaxPosition.x ;
					vert[ 1 ].r = ( unsigned char )r ;
					vert[ 1 ].g = ( unsigned char )g ;
					vert[ 1 ].b = ( unsigned char )b ;
					vert[ 1 ].a = 255 ;

					vert += 2 ;

					VertCount += 2 ;
					if( VertCount == 300 )
					{
						NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
						VertCount = 0 ;
						vert = Vertex ;
					}
				}
			}

			Pos.z = Collision->MinPosition.z ;
			Pos.y = Collision->MinPosition.y ;
			for( i = 0 ; i <= Collision->YDivNum ; i ++, Pos.y += Collision->UnitSize.y )
			{
				Pos.x = Collision->MinPosition.x ;
				for( j = 0 ; j <= Collision->XDivNum ; j ++, Pos.x += Collision->UnitSize.x )
				{
					vert[ 0 ].pos = Pos ;
					vert[ 0 ].r = ( unsigned char )r ;
					vert[ 0 ].g = ( unsigned char )g ;
					vert[ 0 ].b = ( unsigned char )b ;
					vert[ 0 ].a = 255 ;

					vert[ 1 ].pos = Pos ;
					vert[ 1 ].pos.z = Collision->MaxPosition.z ;
					vert[ 1 ].r = ( unsigned char )r ;
					vert[ 1 ].g = ( unsigned char )g ;
					vert[ 1 ].b = ( unsigned char )b ;
					vert[ 1 ].a = 255 ;

					vert += 2 ;

					VertCount += 2 ;
					if( VertCount == 300 )
					{
						NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
						VertCount = 0 ;
						vert = Vertex ;
					}
				}
			}

			if( VertCount )
			{
				NS_DrawPolygon3DBase( Vertex, VertCount, DX_PRIMTYPE_LINELIST, DX_NONE_GRAPH, FALSE ) ;
				VertCount = 0 ;
				vert = Vertex ;
			}
		}
	}

	// 元々設定されていたワールド→ビュートランスフォーム行列を元に戻す
	NS_SetTransformToWorld( &OrigTransMat ) ;

	// 終了
	return 0 ;
}




















// 描画設定関係

// モデルの描画に SetUseVertexShader, SetUsePixelShader で指定したシェーダーを使用するかどうかを設定する( TRUE:使用する  FALSE:使用しない( デフォルト ) )
extern int NS_MV1SetUseOrigShader( int UseFlag )
{
	if( MV1Man.UseOrigShaderFlag == UseFlag )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// フラグを保存する
	MV1Man.UseOrigShaderFlag = UseFlag ;

	// 終了
	return 0 ;
}

// モデルの半透明要素がある部分についての描画モードを設定する
extern int NS_MV1SetSemiTransDrawMode( int DrawMode /* DX_SEMITRANSDRAWMODE_ALWAYS 等 */ )
{
	if( MV1Man.SemiTransDrawMode == DrawMode )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 描画モードを保存する
	MV1Man.SemiTransDrawMode = DrawMode ;

	// 終了
	return 0 ;
}






















// アタッチしているアニメーションの再生時間を設定する
extern int NS_MV1SetAttachAnimTime( int MHandle, int AttachIndex, float Time )
{
	MV1_MODEL *Model ;
	MV1_ANIMSET *AnimSet ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1 ;
	AnimSet = Model->AnimSet[ AttachIndex ].AnimSet ;

	// パラメータのセットアップフラグを倒す
	AnimSet->ParamSetup = false ;

	// アニメーションパラメータを元にした行列がセットアップされていない状態にする
	Model->AnimSetupFlag = false ;
	Model->LocalWorldMatrixSetupFlag = false ;

	// アニメーションの時間をセットする
	MV1SetAnimSetTime( AnimSet, Time ) ;

	// 終了
	return 0 ;
}

// アタッチしているアニメーションの再生時間を取得する
extern float NS_MV1GetAttachAnimTime( int MHandle, int AttachIndex )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1.0f ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1.0f ;

	// アニメーションの時間を返す
	return Model->AnimSet[ AttachIndex ].AnimSet->NowTime ;
}

// アタッチしているアニメーションの総時間を得る
extern float NS_MV1GetAttachAnimTotalTime( int MHandle, int AttachIndex )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1.0f ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1.0f ;

	// アニメーションの総時間を返す
	return Model->AnimSet[ AttachIndex ].AnimSet->BaseData->MaxTime ;
}


// アタッチしているアニメーションのブレンド率を設定する
extern int NS_MV1SetAttachAnimBlendRate( int MHandle, int AttachIndex, float Rate )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIMSET *MAnimSet ;
	MV1_MODEL_ANIM *MAnim ;
	int i, FrameNum ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1 ;
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// ブレンドレートが今までと同じ場合は何もせずに終了
//	if( *( ( DWORD * )&Model->AnimSet[ AttachIndex ].BlendRate ) == *( ( DWORD * )&Rate ) ) return 0 ;

	// ブレンドレートを変更する
	FrameNum = Model->BaseData->FrameNum ;
	MAnim = &Model->Anim[ AttachIndex ] ;
	for( i = 0 ; i < FrameNum ; i ++, MAnim += Model->AnimSetMaxNum )
	{
		MAnim->BlendRate = Rate ;
	}

	// 行列の全更新
	_MEMSET( Model->ChangeMatrixFlag, 0xff, Model->ChangeMatrixFlagSize ) ;

	// 行列の更新が済んでいるフラグを倒す
	Model->LocalWorldMatrixSetupFlag = false ;

	// 正常終了
	return 0 ;
}

// アタッチしているアニメーションのブレンド率を取得する
extern float NS_MV1GetAttachAnimBlendRate( int MHandle, int AttachIndex )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1.0f ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1.0f ;

	// ブレンドレートを返す
	return Model->Anim[ AttachIndex ].BlendRate ;
}

// アタッチしているアニメーションのブレンド率を設定する( フレーム単位 )
extern int NS_MV1SetAttachAnimBlendRateToFrame( int MHandle, int AttachIndex, int FrameIndex, float Rate, int SetChild )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIM *MAnim ;
	MV1_FRAME *Frame ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;

	// フレームインデックスが不正だったら何もせずに終了
	if( FrameIndex < 0 || FrameIndex >= Model->BaseData->FrameNum ) return -1 ;
	Frame = &Model->Frame[ FrameIndex ] ;
	MAnim = &Model->Anim[ AttachIndex ] + Model->AnimSetMaxNum * FrameIndex ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1 ;

	// ブレンドレートが今までと同じ場合は何もせずに終了
	if( *( ( DWORD * )&MAnim->BlendRate ) == *( ( DWORD * )&Rate ) ) return 0 ;

	// ブレンドレートを変更する
	MAnim->BlendRate = Rate ;

	// 子フレームもセットする指定があった場合は子フレームもセットする
	if( SetChild )
	{
		MV1_MODEL_ANIM *MAnim2 ;

		MAnim2 = MAnim + Model->AnimSetMaxNum ;
		for( i = 0 ; i < Frame->BaseData->TotalChildNum ; i ++, MAnim2 += Model->AnimSetMaxNum )
			MAnim2->BlendRate = Rate ;
	}

	// 行列がセットアップされていない状態にする
	Model->LocalWorldMatrixSetupFlag = false ;
	MV1BitSetChange( &Model->Frame[ FrameIndex ].LocalWorldMatrixChange ) ;

	// 正常終了
	return 0 ;
}

// アタッチしているアニメーションのブレンド率を設定する( フレーム単位 )
extern float NS_MV1GetAttachAnimBlendRateToFrame( int MHandle, int AttachIndex, int FrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIM *MAnim ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1.0f ;

	// フレームインデックスが不正だったら何もせずに終了
	if( FrameIndex < 0 || FrameIndex >= Model->BaseData->FrameNum ) return -1.0f ;
	MAnim = &Model->Anim[ AttachIndex ] + Model->AnimSetMaxNum * FrameIndex ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1.0f ;

	// ブレンドレートを返す
	return MAnim->BlendRate ;
}

// アタッチしているアニメーションがターゲットとするフレームの数を取得する
extern int MV1GetAttachAnimTargetFrameNum( int MHandle, int AttachIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIMSET *MAnimSet ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( MAnimSet->Use == false ) return -1 ;

	// 適合しているアニメーションの数を返す
	return MAnimSet->UseAnimNum ;
}

// アタッチしているアニメーションがターゲットとするフレームのインデックスを所得する
extern int MV1GetAttachAnimTargetFrame( int MHandle, int AttachIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIM *MAnim ;
	MV1_MODEL_ANIMSET *MAnimSet ;
	int con ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( MAnimSet->Use == false ) return -1 ;

	// 不正なインデックスの場合は何もせずに終了
	if( Index < 0 || Index >= MAnimSet->UseAnimNum )
		return -1 ;

	// 指定の有効インデックスのフレームを探す
	con = 0 ;
	for( MAnim = &Model->Anim[ AttachIndex ] ; ; MAnim += Model->AnimSetMaxNum )
	{
		if( MAnim->Use == false ) continue ;
		if( con == Index ) break ;
		con ++ ;
	}

	// フレームのインデックスを返す
	return ( int )( MAnim->Anim->Frame - Model->Frame ) ;
}

// アタッチしているアニメーションのアニメーションインデックスを取得する
extern int NS_MV1GetAttachAnim( int MHandle, int AttachIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIMSET *MAnimSet ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( MAnimSet->Use == false ) return -1 ;

	// アニメーションインデックスを返す
	return MAnimSet->BaseDataAnimIndex ;
}

// アタッチしているアニメーションのシェイプを使用するかどうかを設定する( UseFlag  TRUE:使用する( デフォルト )  FALSE:使用しない )
extern int NS_MV1SetAttachAnimUseShapeFlag( int MHandle, int AttachIndex, int UseFlag )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIMSET *MAnimSet ;
	MV1_FRAME *Frame ;
	int i ;
	int FrameNum ;
	bool bDisableFlag ;

	bDisableFlag = UseFlag == FALSE;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1 ;
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// フラグが今までと同じ場合は何もせずに終了
	if( MAnimSet->DisableShapeFlag == bDisableFlag ) return 0 ;

	// フラグを保存
	MAnimSet->DisableShapeFlag = bDisableFlag ;

	// シェイプの状態に変化があったフラグを立てる
	Model->ShapeChangeFlag = true ;
	Frame = Model->Frame ;
	FrameNum = Model->BaseData->FrameNum ;
	for( i = 0 ; i < FrameNum ; i ++, Frame ++ )
	{
		if( Frame->BaseData->MeshNum > 0 && Frame->Mesh->BaseData->Shape == 0 )
			continue ;

		Frame->ShapeChangeFlag = true ;
	}

	// 正常終了
	return 0 ;
}

// アタッチしているアニメーションのシェイプを使用するかどうかを取得する
extern int NS_MV1GetAttachAnimUseShapeFlag( int MHandle, int AttachIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_ANIMSET *MAnimSet ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false ) return -1 ;
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// フラグを返す
	return MAnimSet->DisableShapeFlag ? FALSE : TRUE ;
}

// アタッチしているアニメーションの指定のフレームの現在のローカル変換行列を取得する
extern MATRIX NS_MV1GetAttachAnimFrameLocalMatrix(	int MHandle, int AttachIndex, int FrameIndex )
{
	static MATRIX ErrorValue = { -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f  } ;
	MV1_MODEL *Model ;
	MATRIX ResultMatrix ;
	MV1_MODEL_ANIM *MAnim ;
	MV1_FRAME *Frame ;
	MV1_FRAME_BASE *FrameBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false )
		return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;

	// 存在しないフレーム番号だった場合は何もせずに終了
	if( FrameIndex < 0 || FrameIndex >= Model->BaseData->FrameNum )
		return ErrorValue ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum )
		return ErrorValue ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false )
		return ErrorValue ;

	// 行列のセットアップを行う
	MV1SETUPMATRIX( Model ) ;

	Frame = &Model->Frame[ FrameIndex ] ;
	FrameBase = Frame->BaseData ;
	MAnim = Model->Anim + Model->AnimSetMaxNum * FrameBase->Index + AttachIndex ;

	// アニメーションが存在しなかったら単位行列を返す
	if( MAnim->Use == false )
	{
		return MGetIdent() ;
	}

	if( MAnim->Anim->ValidFlag & MV1_ANIMVALUE_MATRIX )
	{
		ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &MAnim->Anim->Matrix ) ;
	}
	else
	{
		// 行列のセットアップ
		if( MAnim->Anim->ValidBlendMatrix == false )
		{
			MV1SetupTransformMatrix(
				&MAnim->Anim->BlendMatrix,
				MAnim->Anim->ValidFlag,
				&MAnim->Anim->Translate,
				&MAnim->Anim->Scale,
				MAnim->Anim->RotateOrder,
				( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
				&MAnim->Anim->Rotate,
				( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
				&MAnim->Anim->Quaternion
			) ;
			MAnim->Anim->ValidBlendMatrix = true ;
		}
		ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &MAnim->Anim->BlendMatrix ) ;
	}

	// ローカル行列を返す
	return ResultMatrix ;
}

// アタッチしているアニメーションの指定のフレームの現在のローカル座標を取得する
extern VECTOR NS_MV1GetAttachAnimFrameLocalPosition( int MHandle, int AttachIndex, int FrameIndex )
{
	static VECTOR ErrorValue = { -1.0f, -1.0f, -1.0f } ;
	MV1_MODEL *Model ;
	VECTOR ResultPosition ;
	MV1_MODEL_ANIM *MAnim ;
	MV1_FRAME *Frame ;
	MV1_FRAME_BASE *FrameBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false )
		return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;

	// 存在しないフレーム番号だった場合は何もせずに終了
	if( FrameIndex < 0 || FrameIndex >= Model->BaseData->FrameNum )
		return ErrorValue ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum )
		return ErrorValue ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( Model->AnimSet[ AttachIndex ].Use == false )
		return ErrorValue ;

	// 行列のセットアップを行う
	MV1SETUPMATRIX( Model ) ;

	Frame = &Model->Frame[ FrameIndex ] ;
	FrameBase = Frame->BaseData ;
	MAnim = Model->Anim + Model->AnimSetMaxNum * FrameBase->Index + AttachIndex ;

	// アニメーションが存在しなかったら０ベクトルを返す
	if( MAnim->Use == false )
	{
		return VGet( 0.0f, 0.0f, 0.0f ) ;
	}

	if( MAnim->Anim->ValidFlag & MV1_ANIMVALUE_MATRIX )
	{
		ResultPosition.x = MAnim->Anim->Matrix.m[ 0 ][ 3 ] ;
		ResultPosition.y = MAnim->Anim->Matrix.m[ 1 ][ 3 ] ;
		ResultPosition.z = MAnim->Anim->Matrix.m[ 2 ][ 3 ] ;
	}
	else
	{
		// 行列のセットアップ
		if( MAnim->Anim->ValidBlendMatrix == false )
		{
			MV1SetupTransformMatrix(
				&MAnim->Anim->BlendMatrix,
				MAnim->Anim->ValidFlag,
				&MAnim->Anim->Translate,
				&MAnim->Anim->Scale,
				MAnim->Anim->RotateOrder,
				( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
				&MAnim->Anim->Rotate,
				( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
				&MAnim->Anim->Quaternion
			) ;
			MAnim->Anim->ValidBlendMatrix = true ;
		}
		ResultPosition.x = MAnim->Anim->BlendMatrix.m[ 0 ][ 3 ] ;
		ResultPosition.y = MAnim->Anim->BlendMatrix.m[ 1 ][ 3 ] ;
		ResultPosition.z = MAnim->Anim->BlendMatrix.m[ 2 ][ 3 ] ;
	}

	// ローカル座標を返す
	return ResultPosition ;
}


// 指定のフレームがアタッチしているアニメーションの何番目のターゲットフレームかを取得する( AnimFrameIndex として使用する )
extern int MV1GetAttachAnimTargetFrameToAnimFrameIndex( int MHandle, int AttachIndex, int FrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_MODEL_ANIMSET *MAnimSet ;
	MV1_MODEL_ANIM *MAnim ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 確保しているアタッチインデックス外だった場合は何もせずに終了
	if( AttachIndex < 0 || AttachIndex >= Model->AnimSetMaxNum ) return -1 ;
	MAnimSet = &Model->AnimSet[ AttachIndex ] ;

	// フレームインデックスが不正だったら何もせず終了
	if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
		return -1 ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	if( MAnimSet->Use == false ) return -1 ;

	// 指定のフレームにアニメーションがアタッチされていなかったら何もせずに終了
	MAnim = &Model->Anim[ AttachIndex + Model->AnimSetMaxNum * FrameIndex ] ;
	if( MAnim->Use == false ) return -1 ;

	// 指定のフレームで使用されているアニメーション内フレームインデックスを返す
	return ( int )( MAnim->Anim->BaseData - MAnimSet->AnimSet->BaseData->Anim ) ;
}

// アニメーションの数を取得する
extern int NS_MV1GetAnimNum( int MHandle )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;

	// アニメーションの数を返す
	return Model->BaseData->AnimSetNum ;
}

// 指定番号のアニメーション名を取得する
extern const TCHAR *NS_MV1GetAnimName( int MHandle, int AnimIndex )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NULL ;

	// インデックスがアニメーションの数を超えていたらエラー
	if( Model->BaseData->AnimSetNum <= AnimIndex ) return NULL ;

	// アニメーション名を返す
#ifdef UNICODE
	return Model->BaseData->AnimSet[ AnimIndex ].NameW ;
#else
	return Model->BaseData->AnimSet[ AnimIndex ].NameA ;
#endif
}

// 指定番号のアニメーション名を取得する
extern const wchar_t *MV1GetAnimName_WCHAR_T( int MHandle, int AnimIndex )
{
	MV1_MODEL *Model ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NULL ;

	// インデックスがアニメーションの数を超えていたらエラー
	if( Model->BaseData->AnimSetNum <= AnimIndex ) return NULL ;

	// アニメーション名を返す
	return Model->BaseData->AnimSet[ AnimIndex ].NameW ;
}

// 指定番号のアニメーション名を変更する
extern int NS_MV1SetAnimName( int MHandle, int AnimIndex, const TCHAR *AnimName )
{
#ifdef UNICODE
	return MV1SetAnimName_WCHAR_T(
		MHandle, AnimIndex, AnimName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( AnimName, return -1 )

	Result = MV1SetAnimName_WCHAR_T(
		MHandle, AnimIndex, UseAnimNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( AnimName )

	return Result ;
#endif
}

// 指定番号のアニメーション名を変更する
extern int MV1SetAnimName_WCHAR_T( int MHandle, int AnimIndex, const wchar_t *AnimName )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *MBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	MBase = Model->BaseData ;

	// インデックスがアニメーションの数を超えていたらエラー
	if( Model->BaseData->AnimSetNum <= AnimIndex ) return -1 ;
	AnimSetBase = &MBase->AnimSet[ AnimIndex ] ;

	if( AnimSetBase->NameAllocMem )
	{
#ifndef UNICODE
		if( AnimSetBase->NameA != NULL )
		{
			DXFREE( AnimSetBase->NameA ) ;
			AnimSetBase->NameA = NULL ;
		}
#endif
		if( AnimSetBase->NameW != NULL )
		{
			DXFREE( AnimSetBase->NameW ) ;
			AnimSetBase->NameW = NULL ;
		}
	}

	AnimSetBase->NameAllocMem = TRUE ;

	if( _MV1AllocAndWideCharNameCopy( AnimName, &AnimSetBase->NameW ) == false )
	{
		return -1 ;
	}

#ifndef UNICODE
	if( _MV1CreateMultiByteName( AnimName, &AnimSetBase->NameA ) == false )
	{
		return -1 ;
	}
#endif

	// 正常終了
	return 0 ;
}

// 指定名のアニメーション番号を取得する( -1:エラー )
extern int NS_MV1GetAnimIndex( int MHandle, const TCHAR *AnimName )
{
#ifdef UNICODE
	return MV1GetAnimIndex_WCHAR_T(
		MHandle, AnimName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( AnimName, return -1 )

	Result = MV1GetAnimIndex_WCHAR_T(
		MHandle, UseAnimNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( AnimName )

	return Result ;
#endif
}

// 指定名のアニメーション番号を取得する( -1:エラー )
extern int MV1GetAnimIndex_WCHAR_T( int MHandle, const wchar_t *AnimName )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *MBase ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	MBase = Model->BaseData ;

	// 同名のアニメーションを探す
	for( i = 0 ; i < MBase->AnimSetNum && _WCSCMP( MBase->AnimSet[ i ].NameW, AnimName ) != 0 ; i ++ ){}

	return i == MBase->AnimSetNum ? -1 : i ;
}

// 指定番号のアニメーションの総時間を得る
extern float NS_MV1GetAnimTotalTime( int MHandle, int AnimIndex )
{
	MV1_MODEL *Model ;
	MV1_ANIMSET_BASE *AnimSetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;

	// インデックスが不正だったらエラー
	if( AnimIndex < 0 || AnimIndex >= Model->BaseData->AnimSetNum )
		return -1 ;

	// アニメーションがアタッチされていなかったら何もせずに終了
	AnimSetBase = MV1GetAnimSetBase( Model->BaseDataHandle, NULL, AnimIndex ) ;
	if( AnimSetBase == NULL ) return -1.0f ;

	// アニメーションの総時間を返す
	return AnimSetBase->MaxTime ;
}

// 指定のアニメーションがターゲットとするフレームの数を取得する
extern int NS_MV1GetAnimTargetFrameNum( int MHandle, int AnimIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimIndex < 0 || AnimIndex >= ModelBase->AnimSetNum )
		return -1 ;
	AnimSetBase = &ModelBase->AnimSet[ AnimIndex ] ;

	// ターゲットとするフレームの数を返す
	return AnimSetBase->AnimNum ;
}

// 指定のアニメーションがターゲットとするフレームの名前を取得する
extern const TCHAR *NS_MV1GetAnimTargetFrameName( int MHandle, int AnimIndex, int AnimFrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_ANIM_BASE *AnimBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NULL ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimIndex < 0 || AnimIndex >= ModelBase->AnimSetNum )
		return NULL ;
	AnimSetBase = &ModelBase->AnimSet[ AnimIndex ] ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimFrameIndex < 0 || AnimFrameIndex >= AnimSetBase->AnimNum )
		return NULL ;
	AnimBase = &AnimSetBase->Anim[ AnimFrameIndex ] ;

	// ターゲットとするフレームの名前を返す
#ifdef UNICODE
	return AnimBase->TargetFrame->NameW ;
#else
	return AnimBase->TargetFrame->NameA ;
#endif
}

// 指定のアニメーションがターゲットとするフレームの名前を取得する
extern const wchar_t *MV1GetAnimTargetFrameName_WCHAR_T( int MHandle, int AnimIndex, int AnimFrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_ANIM_BASE *AnimBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return NULL ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimIndex < 0 || AnimIndex >= ModelBase->AnimSetNum )
		return NULL ;
	AnimSetBase = &ModelBase->AnimSet[ AnimIndex ] ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimFrameIndex < 0 || AnimFrameIndex >= AnimSetBase->AnimNum )
		return NULL ;
	AnimBase = &AnimSetBase->Anim[ AnimFrameIndex ] ;

	// ターゲットとするフレームの名前を返す
	return AnimBase->TargetFrame->NameW ;
}

// 指定のアニメーションがターゲットとするフレームの番号を取得する
extern int NS_MV1GetAnimTargetFrame( int MHandle, int AnimIndex, int AnimFrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_ANIM_BASE *AnimBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimIndex < 0 || AnimIndex >= ModelBase->AnimSetNum )
		return -1 ;
	AnimSetBase = &ModelBase->AnimSet[ AnimIndex ] ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimFrameIndex < 0 || AnimFrameIndex >= AnimSetBase->AnimNum )
		return -1 ;
	AnimBase = &AnimSetBase->Anim[ AnimFrameIndex ] ;

	// ターゲットとするフレームの名前を返す
	return AnimBase->TargetFrameIndex ;
}

// 指定のアニメーションがターゲットとするフレーム用のアニメーションキーセットの数を取得する
extern int NS_MV1GetAnimTargetFrameKeySetNum( int MHandle, int AnimIndex, int AnimFrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_ANIM_BASE *AnimBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimIndex < 0 || AnimIndex >= ModelBase->AnimSetNum )
		return -1 ;
	AnimSetBase = &ModelBase->AnimSet[ AnimIndex ] ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimFrameIndex < 0 || AnimFrameIndex >= AnimSetBase->AnimNum )
		return -1 ;
	AnimBase = &AnimSetBase->Anim[ AnimFrameIndex ] ;

	// キーセットの数を返す
	return AnimBase->KeySetNum ;
}

// 指定のアニメーションがターゲットとするフレーム用のアニメーションキーセットキーセットインデックスを取得する
extern int NS_MV1GetAnimTargetFrameKeySet( int MHandle, int AnimIndex, int AnimFrameIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIMSET_BASE *AnimSetBase ;
	MV1_ANIM_BASE *AnimBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimIndex < 0 || AnimIndex >= ModelBase->AnimSetNum )
		return -1 ;
	AnimSetBase = &ModelBase->AnimSet[ AnimIndex ] ;

	// インデックスが不正だった場合は何もせずに終了
	if( AnimFrameIndex < 0 || AnimFrameIndex >= AnimSetBase->AnimNum )
		return -1 ;
	AnimBase = &AnimSetBase->Anim[ AnimFrameIndex ] ;

	// インデックスが不正だった場合は何もせずに終了
	if( Index < 0 || Index >= AnimBase->KeySetNum )
		return -1 ;

	// キーセットインデックスを返す
	return ( int )( &AnimBase->KeySet[ Index ] - ModelBase->AnimKeySet ) ;
}

// モデルに含まれるアニメーションキーセットの総数を得る
extern int NS_MV1GetAnimKeySetNum( int MHandle )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// アニメーションキーセットの数を返す
	return ModelBase->AnimKeySetNum ;
}

// 指定のアニメーションキーセットのタイプを取得する
extern int NS_MV1GetAnimKeySetType( int MHandle, int AnimKeySetIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1 ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーセットのタイプを返す
	return AnimKeySetBase->Type ;
}

// 指定のアニメーションキーセットのデータタイプを取得する
extern int NS_MV1GetAnimKeySetDataType( int MHandle, int AnimKeySetIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1 ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーセットのデータタイプを返す
	return AnimKeySetBase->DataType ;
}

// 指定のアニメーションキーセットのキーの時間データタイプを取得する( MV1_ANIMKEY_TIME_TYPE_ONE 等 )
extern int NS_MV1GetAnimKeySetTimeType( int MHandle, int AnimKeySetIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1 ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーセットの時間データタイプを返す
	return AnimKeySetBase->TimeType ;
}

// 指定のアニメーションキーセットのキーの数を取得する
extern int NS_MV1GetAnimKeySetDataNum( int MHandle, int AnimKeySetIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1 ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーの数を返す
	return AnimKeySetBase->Num ;
}

// 指定のアニメーションキーセットのキーの時間を取得する
extern	float NS_MV1GetAnimKeyDataTime( int MHandle, int AnimKeySetIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1 ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスが不正だったら何もせずに終了
	if( Index < 0 || Index >= AnimKeySetBase->Num )
		return -1 ;

	// キーの時間を返す
	if( AnimKeySetBase->TimeType == MV1_ANIMKEY_TIME_TYPE_KEY )
	{
		return AnimKeySetBase->KeyTime[ Index ] ;
	}
	else
	{
		return AnimKeySetBase->StartTime + AnimKeySetBase->UnitTime * Index ;
	}
}

// 指定のアニメーションキーセットの指定の時間でのキーの番号を取得する
extern int NS_MV1GetAnimKeyDataIndexFromTime( int MHandle, int AnimKeySetIndex, float Time )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	int KeyIndex ;
	float Rate ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1 ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスを取得
	KeyIndex = _MV1GetAnimKeyDataIndexFromTime( AnimKeySetBase, Time, Rate ) ;

	// 補間したキーの番号を返す
	return KeyIndex ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_QUATERNION_X か MV1_ANIMKEY_TYPE_QUATERNION_VMD では無かった場合は失敗する
extern	FLOAT4			NS_MV1GetAnimKeyDataToQuaternion( int MHandle, int AnimKeySetIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	FLOAT4 ErrorValue = { -1.0f, -1.0f, -1.0f, -1.0f } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return ErrorValue ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスが不正だったら何もせずに終了
	if( Index < 0 || Index >= AnimKeySetBase->Num )
		return ErrorValue ;

	// キーのタイプが MV1_ANIMKEY_TYPE_QUATERNION_X か MV1_ANIMKEY_TYPE_QUATERNION_VMD ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_QUATERNION_X &&
		AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_QUATERNION_VMD )
		return ErrorValue ;

	// キーの値を返す
	return AnimKeySetBase->KeyFloat4[ Index ] ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_QUATERNION では無かった場合は失敗する( 時間指定版 )
extern	FLOAT4		NS_MV1GetAnimKeyDataToQuaternionFromTime( int MHandle, int AnimKeySetIndex, float Time )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	FLOAT4 ErrorValue = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	FLOAT4 Ret ;
	int KeyIndex ;
	float Rate ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return ErrorValue ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスを取得
	KeyIndex = _MV1GetAnimKeyDataIndexFromTime( AnimKeySetBase, Time, Rate ) ;

	// キーのタイプが MV1_ANIMKEY_TYPE_QUATERNION_X か MV1_ANIMKEY_TYPE_QUATERNION_VMD ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_QUATERNION_X &&
		AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_QUATERNION_VMD )
		return ErrorValue ;

	// 終端だったら補間しない
	if( KeyIndex == AnimKeySetBase->Num - 1 )
	{
		return AnimKeySetBase->KeyFloat4[ KeyIndex ] ;
	}

	// 補間処理
	_MV1SphereLinear( &AnimKeySetBase->KeyFloat4[ KeyIndex ], &AnimKeySetBase->KeyFloat4[ KeyIndex + 1 ], Rate, &Ret ) ; 

	// 補間したキーの値を返す
	return Ret ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_VECTOR では無かった場合は失敗する
extern	VECTOR		NS_MV1GetAnimKeyDataToVector( int MHandle, int AnimKeySetIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	VECTOR ErrorValue = { -1.0f, -1.0f, -1.0f } ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return ErrorValue ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスが不正だったら何もせずに終了
	if( Index < 0 || Index >= AnimKeySetBase->Num )
		return ErrorValue ;

	// キーのタイプが MV1_ANIMKEY_TYPE_VECTOR ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_VECTOR )
		return ErrorValue ;

	// キーを返す
	return AnimKeySetBase->KeyVector[ Index ] ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_VECTOR では無かった場合は失敗する( 時間指定版 )
extern	VECTOR		NS_MV1GetAnimKeyDataToVectorFromTime( int MHandle, int AnimKeySetIndex, float Time )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	VECTOR ErrorValue = { -1.0f, -1.0f, -1.0f } ;
	VECTOR Ret ;
	int KeyIndex ;
	float Rate ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return ErrorValue ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスを取得
	KeyIndex = _MV1GetAnimKeyDataIndexFromTime( AnimKeySetBase, Time, Rate ) ;

	// キーのタイプが MV1_ANIMKEY_TYPE_VECTOR ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_VECTOR )
		return ErrorValue ;

	// 終端だったら補間しない
	if( KeyIndex == AnimKeySetBase->Num - 1 )
	{
		return AnimKeySetBase->KeyVector[ KeyIndex ] ;
	}

	// 補間処理
	Ret.x = ( 1.0f - Rate ) * AnimKeySetBase->KeyVector[ KeyIndex ].x + Rate * AnimKeySetBase->KeyVector[ KeyIndex + 1 ].x ;
	Ret.y = ( 1.0f - Rate ) * AnimKeySetBase->KeyVector[ KeyIndex ].y + Rate * AnimKeySetBase->KeyVector[ KeyIndex + 1 ].y ;
	Ret.z = ( 1.0f - Rate ) * AnimKeySetBase->KeyVector[ KeyIndex ].z + Rate * AnimKeySetBase->KeyVector[ KeyIndex + 1 ].z ;

	// 補間したキーの値を返す
	return Ret ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_MATRIX3X3 か MV1_ANIMKEY_TYPE_MATRIX4X4C では無かった場合は失敗する
extern	MATRIX		NS_MV1GetAnimKeyDataToMatrix( int MHandle, int AnimKeySetIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	static MATRIX ErrorValue = { -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f  } ;
	MATRIX Matrix ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return ErrorValue ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスが不正だったら何もせずに終了
	if( Index < 0 || Index >= AnimKeySetBase->Num )
		return ErrorValue ;

	// キーのデータタイプが MV1_ANIMKEY_DATATYPE_MATRIX4X4C か MV1_ANIMKEY_DATATYPE_MATRIX3X3 ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_MATRIX4X4C &&
		AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_MATRIX3X3 )
		return ErrorValue ;

	// キーの値を返す
	switch( AnimKeySetBase->Type )
	{
	case MV1_ANIMKEY_TYPE_MATRIX4X4C :
		Matrix.m[ 0 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 0 ][ 0 ] ;
		Matrix.m[ 0 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 0 ][ 1 ] ;
		Matrix.m[ 0 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 0 ][ 2 ] ;
		Matrix.m[ 0 ][ 3 ] = 0.0f ;

		Matrix.m[ 1 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 1 ][ 0 ] ;
		Matrix.m[ 1 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 1 ][ 1 ] ;
		Matrix.m[ 1 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 1 ][ 2 ] ;
		Matrix.m[ 1 ][ 3 ] = 0.0f ;

		Matrix.m[ 2 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 2 ][ 0 ] ;
		Matrix.m[ 2 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 2 ][ 1 ] ;
		Matrix.m[ 2 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 2 ][ 2 ] ;
		Matrix.m[ 2 ][ 3 ] = 0.0f ;

		Matrix.m[ 3 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 3 ][ 0 ] ;
		Matrix.m[ 3 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 3 ][ 1 ] ;
		Matrix.m[ 3 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ Index ].Matrix[ 3 ][ 2 ] ;
		Matrix.m[ 3 ][ 3 ] = 1.0f ;
		break ;

	case MV1_ANIMKEY_TYPE_MATRIX3X3 :
		Matrix.m[ 0 ][ 0 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 0 ][ 0 ] ;
		Matrix.m[ 0 ][ 1 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 0 ][ 1 ] ;
		Matrix.m[ 0 ][ 2 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 0 ][ 2 ] ;
		Matrix.m[ 0 ][ 3 ] = 0.0f ;

		Matrix.m[ 1 ][ 0 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 1 ][ 0 ] ;
		Matrix.m[ 1 ][ 1 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 1 ][ 1 ] ;
		Matrix.m[ 1 ][ 2 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 1 ][ 2 ] ;
		Matrix.m[ 1 ][ 3 ] = 0.0f ;

		Matrix.m[ 2 ][ 0 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 2 ][ 0 ] ;
		Matrix.m[ 2 ][ 1 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 2 ][ 1 ] ;
		Matrix.m[ 2 ][ 2 ] = AnimKeySetBase->KeyMatrix3x3[ Index ].Matrix[ 2 ][ 2 ] ;
		Matrix.m[ 2 ][ 3 ] = 0.0f ;

		Matrix.m[ 3 ][ 0 ] = 0.0f ;
		Matrix.m[ 3 ][ 1 ] = 0.0f ;
		Matrix.m[ 3 ][ 2 ] = 0.0f ;
		Matrix.m[ 3 ][ 3 ] = 1.0f ;
		break ;

	default:
		return ErrorValue ;
	}

	return Matrix ;
}


// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_MATRIX4X4C か MV1_ANIMKEY_TYPE_MATRIX3X3 では無かった場合は失敗する( 時間指定版 )
extern	MATRIX		NS_MV1GetAnimKeyDataToMatrixFromTime( int MHandle, int AnimKeySetIndex, float Time )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	static MATRIX ErrorValue = { -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f,   -1.0f, -1.0f, -1.0f, -1.0f  } ;
	MATRIX Ret ;
	int KeyIndex ;
	float Rate ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return ErrorValue ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return ErrorValue ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return ErrorValue ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスを取得
	KeyIndex = _MV1GetAnimKeyDataIndexFromTime( AnimKeySetBase, Time, Rate ) ;

	// キーのデータタイプが MV1_ANIMKEY_DATATYPE_MATRIX4X4C か MV1_ANIMKEY_DATATYPE_MATRIX3X3 ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_MATRIX4X4C &&
		AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_MATRIX3X3 )
		return ErrorValue ;

	// 終端だったら補間しない
	if( KeyIndex == AnimKeySetBase->Num - 1 )
	{
		// キーの値を返す
		switch( AnimKeySetBase->Type )
		{
		case MV1_ANIMKEY_TYPE_MATRIX4X4C :
			Ret.m[ 0 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 0 ][ 0 ] ;
			Ret.m[ 0 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 0 ][ 1 ] ;
			Ret.m[ 0 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 0 ][ 2 ] ;
			Ret.m[ 0 ][ 3 ] = 0.0f ;

			Ret.m[ 1 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 1 ][ 0 ] ;
			Ret.m[ 1 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 1 ][ 1 ] ;
			Ret.m[ 1 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 1 ][ 2 ] ;
			Ret.m[ 1 ][ 3 ] = 0.0f ;

			Ret.m[ 2 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 2 ][ 0 ] ;
			Ret.m[ 2 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 2 ][ 1 ] ;
			Ret.m[ 2 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 2 ][ 2 ] ;
			Ret.m[ 2 ][ 3 ] = 0.0f ;

			Ret.m[ 3 ][ 0 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 3 ][ 0 ] ;
			Ret.m[ 3 ][ 1 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 3 ][ 1 ] ;
			Ret.m[ 3 ][ 2 ] = AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 3 ][ 2 ] ;
			Ret.m[ 3 ][ 3 ] = 1.0f ;
			break ;

		case MV1_ANIMKEY_TYPE_MATRIX3X3 :
			Ret.m[ 0 ][ 0 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 0 ][ 0 ] ;
			Ret.m[ 0 ][ 1 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 0 ][ 1 ] ;
			Ret.m[ 0 ][ 2 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 0 ][ 2 ] ;
			Ret.m[ 0 ][ 3 ] = 0.0f ;

			Ret.m[ 1 ][ 0 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 1 ][ 0 ] ;
			Ret.m[ 1 ][ 1 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 1 ][ 1 ] ;
			Ret.m[ 1 ][ 2 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 1 ][ 2 ] ;
			Ret.m[ 1 ][ 3 ] = 0.0f ;

			Ret.m[ 2 ][ 0 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 2 ][ 0 ] ;
			Ret.m[ 2 ][ 1 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 2 ][ 1 ] ;
			Ret.m[ 2 ][ 2 ] = AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 2 ][ 2 ] ;
			Ret.m[ 2 ][ 3 ] = 0.0f ;

			Ret.m[ 3 ][ 0 ] = 0.0f ;
			Ret.m[ 3 ][ 1 ] = 0.0f ;
			Ret.m[ 3 ][ 2 ] = 0.0f ;
			Ret.m[ 3 ][ 3 ] = 1.0f ;
			break ;

		default :
			return ErrorValue ;
		}
		return Ret ;
	}

	// 補間処理
	switch( AnimKeySetBase->Type )
	{
	case MV1_ANIMKEY_TYPE_MATRIX4X4C :
		Ret.m[ 0 ][ 0 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 0 ][ 0 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 0 ][ 0 ] ;
		Ret.m[ 0 ][ 1 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 0 ][ 1 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 0 ][ 1 ] ;
		Ret.m[ 0 ][ 2 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 0 ][ 2 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 0 ][ 2 ] ;
		Ret.m[ 0 ][ 3 ] = 0.0f ;

		Ret.m[ 1 ][ 0 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 1 ][ 0 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 1 ][ 0 ] ;
		Ret.m[ 1 ][ 1 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 1 ][ 1 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 1 ][ 1 ] ;
		Ret.m[ 1 ][ 2 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 1 ][ 2 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 1 ][ 2 ] ;
		Ret.m[ 1 ][ 3 ] = 0.0f ;

		Ret.m[ 2 ][ 0 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 2 ][ 0 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 2 ][ 0 ] ;
		Ret.m[ 2 ][ 1 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 2 ][ 1 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 2 ][ 1 ] ;
		Ret.m[ 2 ][ 2 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 2 ][ 2 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 2 ][ 2 ] ;
		Ret.m[ 2 ][ 3 ] = 0.0f ;

		Ret.m[ 3 ][ 0 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 3 ][ 0 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 3 ][ 0 ] ;
		Ret.m[ 3 ][ 1 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 3 ][ 1 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 3 ][ 1 ] ;
		Ret.m[ 3 ][ 2 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex ].Matrix[ 3 ][ 2 ] + Rate * AnimKeySetBase->KeyMatrix4x4C[ KeyIndex + 1 ].Matrix[ 3 ][ 2 ] ;
		Ret.m[ 3 ][ 3 ] = 1.0f ;
		break ;

	case MV1_ANIMKEY_TYPE_MATRIX3X3 :
		Ret.m[ 0 ][ 0 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 0 ][ 0 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 0 ][ 0 ] ;
		Ret.m[ 0 ][ 1 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 0 ][ 1 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 0 ][ 1 ] ;
		Ret.m[ 0 ][ 2 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 0 ][ 2 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 0 ][ 2 ] ;
		Ret.m[ 0 ][ 3 ] = 0.0f ;

		Ret.m[ 1 ][ 0 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 1 ][ 0 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 1 ][ 0 ] ;
		Ret.m[ 1 ][ 1 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 1 ][ 1 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 1 ][ 1 ] ;
		Ret.m[ 1 ][ 2 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 1 ][ 2 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 1 ][ 2 ] ;
		Ret.m[ 1 ][ 3 ] = 0.0f ;

		Ret.m[ 2 ][ 0 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 2 ][ 0 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 2 ][ 0 ] ;
		Ret.m[ 2 ][ 1 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 2 ][ 1 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 2 ][ 1 ] ;
		Ret.m[ 2 ][ 2 ] = ( 1.0f - Rate ) * AnimKeySetBase->KeyMatrix3x3[ KeyIndex ].Matrix[ 2 ][ 2 ] + Rate * AnimKeySetBase->KeyMatrix3x3[ KeyIndex + 1 ].Matrix[ 2 ][ 2 ] ;
		Ret.m[ 2 ][ 3 ] = 0.0f ;

		Ret.m[ 3 ][ 0 ] = 0.0f ;
		Ret.m[ 3 ][ 1 ] = 0.0f ;
		Ret.m[ 3 ][ 2 ] = 0.0f ;
		Ret.m[ 3 ][ 3 ] = 1.0f ;
		break ;

	default :
		return ErrorValue ;
	}

	// 補間処理をした行列を返す
	return Ret ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_FLAT では無かった場合は失敗する
extern	float		NS_MV1GetAnimKeyDataToFlat( int MHandle, int AnimKeySetIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1.0f ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスが不正だったら何もせずに終了
	if( Index < 0 || Index >= AnimKeySetBase->Num )
		return -1.0f ;

	// キーのタイプが MV1_ANIMKEY_TYPE_FLAT ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_FLAT )
		return -1.0f ;

	// キーの値を返す
	return AnimKeySetBase->KeyFlat[ Index ] ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_FLAT では無かった場合は失敗する( 時間指定版 )
extern	float		NS_MV1GetAnimKeyDataToFlatFromTime( int MHandle, int AnimKeySetIndex, float Time )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	int KeyIndex ;
	float Rate ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1.0f ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスを取得
	KeyIndex = _MV1GetAnimKeyDataIndexFromTime( AnimKeySetBase, Time, Rate ) ;

	// キーのタイプが MV1_ANIMKEY_TYPE_FLAT ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_FLAT )
		return -1.0f ;

	// キーの値を返す
	return AnimKeySetBase->KeyFlat[ KeyIndex ] ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_LINEAR では無かった場合は失敗する
extern	float		NS_MV1GetAnimKeyDataToLinear( int MHandle, int AnimKeySetIndex, int Index )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1.0f ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスが不正だったら何もせずに終了
	if( Index < 0 || Index >= AnimKeySetBase->Num )
		return -1.0f ;

	// キーのタイプが MV1_ANIMKEY_TYPE_LINEAR ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_LINEAR )
		return -1.0f ;

	// キーの値を返す
	return AnimKeySetBase->KeyLinear[ Index ] ;
}

// 指定のアニメーションキーセットのキーを取得する、キータイプが MV1_ANIMKEY_TYPE_LINEAR では無かった場合は失敗する( 時間指定版 )
extern	float		NS_MV1GetAnimKeyDataToLinearFromTime( int MHandle, int AnimKeySetIndex, float Time )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_ANIM_KEYSET_BASE *AnimKeySetBase ;
	int KeyIndex ;
	float Rate ;
	float Ret ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1.0f ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1.0f ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら何もせずに終了
	if( AnimKeySetIndex < 0 || AnimKeySetIndex >= ModelBase->AnimKeySetNum )
		return -1.0f ;
	AnimKeySetBase = &ModelBase->AnimKeySet[ AnimKeySetIndex ] ;

	// キーのインデックスを取得
	KeyIndex = _MV1GetAnimKeyDataIndexFromTime( AnimKeySetBase, Time, Rate ) ;

	// キーのタイプが MV1_ANIMKEY_TYPE_LINEAR ではなかったらエラー
	if( AnimKeySetBase->Type != MV1_ANIMKEY_TYPE_LINEAR )
		return -1.0f ;

	// 終端だったら補間しない
	if( KeyIndex == AnimKeySetBase->Num - 1 )
	{
		return AnimKeySetBase->KeyLinear[ KeyIndex ] ;
	}

	// 補間処理
	Ret = ( 1.0f - Rate ) * AnimKeySetBase->KeyLinear[ KeyIndex ] + Rate * AnimKeySetBase->KeyLinear[ KeyIndex + 1 ] ;

	// 補間したキーの値を返す
	return Ret ;
}


// モデルで使用しているマテリアルの数を取得する
extern int NS_MV1GetMaterialNum( int MHandle )
{
	return MV1GetMaterialNumBase( MV1GetModelBaseHandle( MHandle ) ) ;
}

// 指定のマテリアルの名前を取得する
extern	const TCHAR *NS_MV1GetMaterialName( int MHandle, int MaterialIndex )
{
#ifdef UNICODE
	return MV1GetMaterialNameBaseW( MV1GetModelBaseHandle( MHandle ), MaterialIndex ) ;
#else
	return MV1GetMaterialNameBase( MV1GetModelBaseHandle( MHandle ), MaterialIndex ) ;
#endif
}

// 指定のマテリアルの名前を取得する
extern	const wchar_t *MV1GetMaterialName_WCHAR_T( int MHandle, int MaterialIndex )
{
	return MV1GetMaterialNameBaseW( MV1GetModelBaseHandle( MHandle ), MaterialIndex ) ;
}

// 指定のマテリアルのタイプを変更する( Type : DX_MATERIAL_TYPE_NORMAL など )
extern int NS_MV1SetMaterialType( int MHandle, int MaterialIndex, int Type )
{
	return MV1SetMaterialTypeBase( MV1GetModelBaseHandle( MHandle ), MaterialIndex, Type ) ;
}

// 指定のマテリアルのタイプを取得する( 戻り値 : DX_MATERIAL_TYPE_NORMAL など )
extern int NS_MV1GetMaterialType( int MHandle, int MaterialIndex )
{
	return MV1GetMaterialTypeBase( MV1GetModelBaseHandle( MHandle ), MaterialIndex ) ;
}

// 指定のマテリアルのディフューズカラーを取得する
extern	COLOR_F		NS_MV1GetMaterialDifColor( int MHandle, int MaterialIndex )
{
	COLOR_F ErrorRet = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, ErrorRet ) ;

	return Material->Diffuse ;
}

// 指定のマテリアルのスペキュラカラーを取得する
extern	COLOR_F		NS_MV1GetMaterialSpcColor( int MHandle, int MaterialIndex )
{
	COLOR_F ErrorRet = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, ErrorRet ) ;

	return Material->Specular ;
}

// 指定のマテリアルのエミッシブカラーを取得する
extern	COLOR_F		NS_MV1GetMaterialEmiColor( int MHandle, int MaterialIndex )
{
	COLOR_F ErrorRet = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, ErrorRet ) ;

	return Material->Emissive ;
}

// 指定のマテリアルのアンビエントカラーを取得する
extern	COLOR_F		NS_MV1GetMaterialAmbColor( int MHandle, int MaterialIndex )
{
	COLOR_F ErrorRet = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, ErrorRet ) ;

	return Material->Ambient ;
}

// 指定のマテリアルのスペキュラの強さを取得する
extern	float		NS_MV1GetMaterialSpcPower( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1.0f ) ;

	return Material->Power ;
}




// 指定のマテリアルでディフューズマップとして使用するテクスチャを指定する
extern int NS_MV1SetMaterialDifMapTexture( int MHandle, int MaterialIndex, int TexIndex )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < 0 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	if( Material->DiffuseLayerNum != 0 && Material->DiffuseLayer[ 0 ].Texture == TexIndex )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// ディフューズレイヤーが無かった場合は１にする
	if( Material->DiffuseLayerNum == 0 )
	{
		Material->DiffuseLayerNum = 1 ;
	}

	// 設定
	Material->DiffuseLayer[ 0 ].Texture = TexIndex ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでディフューズマップとして使用されているテクスチャのインデックスを取得する
extern int NS_MV1GetMaterialDifMapTexture( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DiffuseLayerNum == 0 )
		return -1 ;

	return Material->DiffuseLayer[ 0 ].Texture ;
}

// 指定のマテリアルでスペキュラマップとして使用するテクスチャを指定する
extern int NS_MV1SetMaterialSpcMapTexture( int MHandle, int MaterialIndex, int TexIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex >= ModelBase->TextureNum )
		return -1 ;

	// テクスチャインデックスが 0 以下だった場合はスペキュラマップを解除する
	if( TexIndex < 0 )
	{
		if( Material->SpecularLayerNum == 0 )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		Material->SpecularLayerNum = 0 ;
	}
	else
	{
		if( Material->SpecularLayerNum != 0 && Material->SpecularLayer[ 0 ].Texture == TexIndex )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// スペキュラマップの数が０だったら１にする
		if( Material->SpecularLayerNum == 0 )
		{
			Material->SpecularLayerNum = 1 ;
		}

		// 設定
		Material->SpecularLayer[ 0 ].Texture = TexIndex ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでスペキュラマップとして使用されているテクスチャのインデックスを取得する
extern int NS_MV1GetMaterialSpcMapTexture( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->SpecularLayerNum == 0 )
		return -1 ;

	return Material->SpecularLayer[ 0 ].Texture ;
}

// 指定のマテリアルで法線マップとして使用されているテクスチャのインデックスを取得する
extern int NS_MV1GetMaterialNormalMapTexture( int MHandle, int MaterialIndex )
{
	return NS_MV1GetMaterialNormalMapTextureBase( MV1GetModelBaseHandle( MHandle ), MaterialIndex ) ;
}

// 指定のマテリアルのディフューズカラーを設定する
extern int NS_MV1SetMaterialDifColor( int MHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MESH *Mesh ;
	int i ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Diffuse.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Diffuse.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Diffuse.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Diffuse.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Diffuse = Color ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのスペキュラカラーを設定する
extern int NS_MV1SetMaterialSpcColor( int MHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MESH *Mesh ;
	int i ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Specular.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Specular.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Specular.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Specular.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Specular = Color ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのエミッシブカラーを設定する
extern int NS_MV1SetMaterialEmiColor( int MHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MESH *Mesh ;
	int i ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Emissive.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Emissive.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Emissive.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Emissive.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Emissive = Color ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのアンビエントカラーを設定する
extern int NS_MV1SetMaterialAmbColor( int MHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MESH *Mesh ;
	int i ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// 今までと同じだったら何もしない
	if( *( ( DWORD * )&Material->Ambient.r ) == *( ( DWORD * )&Color.r ) &&
		*( ( DWORD * )&Material->Ambient.g ) == *( ( DWORD * )&Color.g ) &&
		*( ( DWORD * )&Material->Ambient.b ) == *( ( DWORD * )&Color.b ) &&
		*( ( DWORD * )&Material->Ambient.a ) == *( ( DWORD * )&Color.a ) )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Ambient = Color ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのスペキュラの強さを設定する
extern int NS_MV1SetMaterialSpcPower( int MHandle, int MaterialIndex, float Power )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->Power == Power )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// セット
	Material->Power = Power ;

	// 終了
	return 0 ;
}

// 指定のマテリアルでトゥーンレンダリングのディフューズグラデーションマップとして使用するテクスチャを設定する
extern int NS_MV1SetMaterialDifGradTexture( int MHandle, int MaterialIndex, int TexIndex )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < -1 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	if( Material->DiffuseGradTexture == TexIndex )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->DiffuseGradTexture = TexIndex ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでトゥーンレンダリングのディフューズグラデーションマップとして使用するテクスチャを取得する
extern int NS_MV1GetMaterialDifGradTexture( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->DiffuseGradTexture ;
}

// 指定のマテリアルでトゥーンレンダリングのスペキュラグラデーションマップとして使用するテクスチャを設定する
extern int NS_MV1SetMaterialSpcGradTexture( int MHandle, int MaterialIndex, int TexIndex )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < -1 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	if( Material->SpecularGradTexture == TexIndex )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SpecularGradTexture = TexIndex ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでトゥーンレンダリングのスペキュラグラデーションマップとして使用するテクスチャを取得する
extern int NS_MV1GetMaterialSpcGradTexture( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->SpecularGradTexture ;
}

// 指定のマテリアルでトゥーンレンダリングのスフィアマップとして使用するテクスチャを設定する
extern int NS_MV1SetMaterialSphereMapTexture( int MHandle, int MaterialIndex, int TexIndex )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( TexIndex < -1 || TexIndex >= ModelBase->TextureNum )
		return -1 ;

	if( Material->SphereMapTexture == TexIndex )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SphereMapTexture = TexIndex ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルでトゥーンレンダリングのスフィアマップとして使用するテクスチャを取得する
extern int NS_MV1GetMaterialSphereMapTexture( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->SphereMapTexture ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するディフューズグラデーションマップとディフューズカラーの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1SetMaterialDifGradBlendType( int MHandle, int MaterialIndex, int BlendType )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DiffuseGradBlendType == BlendType )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->DiffuseGradBlendType = BlendType ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するディフューズグラデーションマップとディフューズカラーの合成方法を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1GetMaterialDifGradBlendType( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->DiffuseGradBlendType ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスペキュラグラデーションマップとスペキュラカラーの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1SetMaterialSpcGradBlendType( int MHandle, int MaterialIndex, int BlendType )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->SpecularGradBlendType == BlendType )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SpecularGradBlendType = BlendType ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスペキュラグラデーションマップとスペキュラカラーの合成方法を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1GetMaterialSpcGradBlendType( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->SpecularGradBlendType ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスフィアマップの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1SetMaterialSphereMapBlendType(	int MHandle, int MaterialIndex, int BlendType )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->SphereMapBlendType == BlendType )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->SphereMapBlendType = BlendType ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用するスフィアマップの合成方法を取得する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1GetMaterialSphereMapBlendType(	int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->SphereMapBlendType ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の太さを設定する( 0.0f ～ 1.0f )
extern int NS_MV1SetMaterialOutLineWidth( int MHandle, int MaterialIndex, float Width )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// 同じだったら何もしない
	if( Material->OutLineWidth == Width )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->OutLineWidth = Width ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の太さを取得する( 0.0f ～ 1.0f )
extern float NS_MV1GetMaterialOutLineWidth( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->OutLineWidth ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線のドット単位の太さを設定する
extern int NS_MV1SetMaterialOutLineDotWidth( int MHandle, int MaterialIndex, float Width )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// 同じだったら何もしない
	if( Material->OutLineDotWidth == Width )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->OutLineDotWidth = Width ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線のドット単位の太さを取得する
extern float NS_MV1GetMaterialOutLineDotWidth( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->OutLineDotWidth ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の色を設定する
extern int NS_MV1SetMaterialOutLineColor( int MHandle, int MaterialIndex, COLOR_F Color )
{
	MV1_MESH *Mesh ;
	int i ;

	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// 同じだったら何もしない
	if( ( ( DWORD * )&Material->OutLineColor )[ 0 ] == ( ( DWORD * )&Color )[ 0 ] &&
		( ( DWORD * )&Material->OutLineColor )[ 1 ] == ( ( DWORD * )&Color )[ 1 ] &&
		( ( DWORD * )&Material->OutLineColor )[ 2 ] == ( ( DWORD * )&Color )[ 2 ] &&
		( ( DWORD * )&Material->OutLineColor )[ 3 ] == ( ( DWORD * )&Color )[ 3 ] ) return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// 設定
	Material->OutLineColor = Color ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルのトゥーンレンダリングで使用する輪郭線の色を取得する
extern COLOR_F NS_MV1GetMaterialOutLineColor( int MHandle, int MaterialIndex )
{
	COLOR_F RetColor = { 0.0f, 0.0f, 0.0f, 0.0f } ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, RetColor ) ;

	return Material->OutLineColor ;
}

// 指定のマテリアルの描画ブレンドモードを設定する( DX_BLENDMODE_ALPHA 等 )
extern int NS_MV1SetMaterialDrawBlendMode( int MHandle, int MaterialIndex, int BlendMode )
{
	MV1_MESH *Mesh ;
	int i ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DrawBlendMode == BlendMode )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// ブレンドモードのセット
	Material->DrawBlendMode = BlendMode ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルの描画ブレンドパラメータを設定する
extern int NS_MV1SetMaterialDrawBlendParam( int MHandle, int MaterialIndex, int BlendParam )
{
	MV1_MESH *Mesh ;
	int i ;
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->DrawBlendParam == BlendParam )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// ブレンドモードのセット
	Material->DrawBlendParam = BlendParam ;

	// このマテリアルを使用しているメッシュの半透明要素有無情報のセットアップ完了フラグを倒す
	Mesh = Model->Mesh ;
	for( i = 0 ; i < ModelBase->MeshNum ; i ++, Mesh ++ )
	{
		if( Mesh->Material != Material ) continue ;

		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
		if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
			MV1BitSetChange( &Mesh->DrawMaterialChange ) ;
	}

	// 終了
	return 0 ;
}

// 指定のマテリアルの描画ブレンドモードを取得する( DX_BLENDMODE_ALPHA 等 )
extern int NS_MV1GetMaterialDrawBlendMode( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// ブレンドモードを返す
	return Material->DrawBlendMode ;
}

// 指定のマテリアルの描画ブレンドパラメータを設定する
extern int NS_MV1GetMaterialDrawBlendParam( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	// ブレンドモードを返す
	return Material->DrawBlendParam ;
}

// 指定のマテリアルの描画時のアルファテストの設定を行う( Enable:αテストを行うかどうか( TRUE:行う  FALSE:行わない( デフォルト )  Mode:テストモード( DX_CMP_GREATER等 )  Param:描画アルファ値との比較に使用する値( 0～255 ) )
extern int NS_MV1SetMaterialDrawAlphaTest( int MHandle, int MaterialIndex,	int Enable, int Mode, int Param )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	if( Material->UseAlphaTest == Enable &&
		Material->AlphaFunc == Mode &&
		Material->AlphaRef == Param )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// パラメータをセット
	Material->UseAlphaTest = Enable ;
	Material->AlphaFunc = Mode ;
	Material->AlphaRef = Param ;

	// 終了
	return 0 ;
}

// 指定のマテリアルの描画時のアルファテストを行うかどうかを取得する( 戻り値  TRUE:アルファテストを行う  FALSE:アルファテストを行わない )
extern int NS_MV1GetMaterialDrawAlphaTestEnable( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->UseAlphaTest ;
}

// 指定のマテリアルの描画時のアルファテストのテストモードを取得する( 戻り値  テストモード( DX_CMP_GREATER等 ) )
extern int NS_MV1GetMaterialDrawAlphaTestMode( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->AlphaFunc ;
}

// 指定のマテリアルの描画時のアルファテストの描画アルファ地との比較に使用する値( 0～255 )を取得する
extern int NS_MV1GetMaterialDrawAlphaTestParam( int MHandle, int MaterialIndex )
{
	MV1MATERIALSTART( MHandle, Model, ModelBase, Material, MaterialIndex, -1 ) ;

	return Material->AlphaRef ;
}

// 全てのマテリアルのタイプを変更する( Type : DX_MATERIAL_TYPE_NORMAL など )
extern int NS_MV1SetMaterialTypeAll(				int MHandle, int Type )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialType( MHandle, i, Type ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルのトゥーンレンダリングで使用するディフューズグラデーションマップとディフューズカラーの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1SetMaterialDifGradBlendTypeAll(	int MHandle, int BlendType )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialDifGradBlendType( MHandle, i, BlendType ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルのトゥーンレンダリングで使用するスペキュラグラデーションマップとスペキュラカラーの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1SetMaterialSpcGradBlendTypeAll(	int MHandle, int BlendType )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialSpcGradBlendType( MHandle, i, BlendType ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルのトゥーンレンダリングで使用するスフィアマップの合成方法を設定する( DX_MATERIAL_BLENDTYPE_ADDITIVE など )
extern int NS_MV1SetMaterialSphereMapBlendTypeAll( int MHandle,  int BlendType )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialSphereMapBlendType( MHandle, i, BlendType ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルのトゥーンレンダリングで使用する輪郭線の太さを設定する
extern int NS_MV1SetMaterialOutLineWidthAll(		int MHandle, float Width )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialOutLineWidth( MHandle, i, Width ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルのトゥーンレンダリングで使用する輪郭線のドット単位の太さを設定する
extern int NS_MV1SetMaterialOutLineDotWidthAll(	int MHandle,     float Width )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialOutLineDotWidth( MHandle, i, Width ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルのトゥーンレンダリングで使用する輪郭線の色を設定する
extern int NS_MV1SetMaterialOutLineColorAll(		int MHandle, COLOR_F Color )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialOutLineColor( MHandle, i, Color ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルの描画ブレンドモードを設定する( DX_BLENDMODE_ALPHA 等 )
extern int NS_MV1SetMaterialDrawBlendModeAll(		int MHandle, int BlendMode )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialDrawBlendMode( MHandle, i, BlendMode ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルの描画ブレンドパラメータを設定する
extern int NS_MV1SetMaterialDrawBlendParamAll(	int MHandle,     int BlendParam )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialDrawBlendParam( MHandle, i, BlendParam ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}

// 全てのマテリアルの描画時のアルファテストの設定を行う( Enable:αテストを行うかどうか( TRUE:行う  FALSE:行わない( デフォルト ) ) Mode:テストモード( DX_CMP_GREATER等 )  Param:描画アルファ値との比較に使用する値( 0～255 ) )
extern int NS_MV1SetMaterialDrawAlphaTestAll(		int MHandle, int Enable, int Mode, int Param )
{
	int i ;
	MV1START( MHandle, Model, ModelBase, -1 ) ;

	for( i = 0 ; i < ModelBase->MaterialNum ; i ++ )
	{
		if( NS_MV1SetMaterialDrawAlphaTest( MHandle, i, Enable, Mode, Param ) != 0 )
		{
			return -1 ;
		}
	}

	return 0 ;
}











// テクスチャ関係

// テクスチャ関係

// テクスチャの数を取得
extern int NS_MV1GetTextureNum( int MHandle )
{
	return MV1GetTextureNumBase( MV1GetModelBaseHandle( MHandle ) ) ;
}

// テクスチャの名前を取得
extern	const TCHAR *NS_MV1GetTextureName( int MHandle, int TexIndex )
{
#ifdef UNICODE
	return MV1GetTextureNameBaseW( MV1GetModelBaseHandle( MHandle ), TexIndex ) ;
#else
	return MV1GetTextureNameBase( MV1GetModelBaseHandle( MHandle ), TexIndex ) ;
#endif
}

// テクスチャの名前を取得
extern	const wchar_t *MV1GetTextureName_WCHAR_T( int MHandle, int TexIndex )
{
	return MV1GetTextureNameBaseW( MV1GetModelBaseHandle( MHandle ), TexIndex ) ;
}

// カラーテクスチャのファイルパスを変更する
extern int NS_MV1SetTextureColorFilePath( int MHandle, int TexIndex, const TCHAR *FilePath )
{
#ifdef UNICODE
	return MV1SetTextureColorFilePath_WCHAR_T(
		MHandle, TexIndex, FilePath
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FilePath, return -1 )

	Result = MV1SetTextureColorFilePath_WCHAR_T(
		MHandle, TexIndex, UseFilePathBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FilePath )

	return Result ;
#endif
}

// カラーテクスチャのファイルパスを変更する
extern int MV1SetTextureColorFilePath_WCHAR_T( int MHandle, int TexIndex, const wchar_t *FilePath )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	return _MV1TextureLoad(
				ModelBase, Texture,
				FilePath, Texture->AlphaFilePathW_,
				Texture->BumpImageFlag, Texture->BumpImageNextPixelLength,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0,
				FALSE ) ;
}

// カラーテクスチャのファイルパスを取得
extern	const TCHAR *NS_MV1GetTextureColorFilePath( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, NULL ) ;

#ifdef UNICODE
	return Texture->ColorFilePathW_ ;
#else
	return Texture->ColorFilePathA_ ;
#endif
}

// カラーテクスチャのファイルパスを取得
extern	const wchar_t *MV1GetTextureColorFilePath_WCHAR_T( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, NULL ) ;

	return Texture->ColorFilePathW_ ;
}

// アルファテクスチャのファイルパスを変更する
extern int NS_MV1SetTextureAlphaFilePath( int MHandle, int TexIndex, const TCHAR *FilePath )
{
#ifdef UNICODE
	return MV1SetTextureAlphaFilePath_WCHAR_T(
		MHandle, TexIndex, FilePath
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FilePath, return -1 )

	Result = MV1SetTextureAlphaFilePath_WCHAR_T(
		MHandle, TexIndex, UseFilePathBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FilePath )

	return Result ;
#endif
}

// アルファテクスチャのファイルパスを変更する
extern int MV1SetTextureAlphaFilePath_WCHAR_T( int MHandle, int TexIndex, const wchar_t *FilePath )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	return _MV1TextureLoad(
				ModelBase, Texture,
				Texture->ColorFilePathW_, FilePath,
				Texture->BumpImageFlag, Texture->BumpImageNextPixelLength,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0,
				FALSE ) ;
}

// アルファテクスチャのファイルパスを取得
extern	const TCHAR *NS_MV1GetTextureAlphaFilePath( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, NULL ) ;

#ifdef UNICODE
	return Texture->AlphaFilePathW_ ;
#else
	return Texture->AlphaFilePathA_ ;
#endif
}

// アルファテクスチャのファイルパスを取得
extern	const wchar_t *MV1GetTextureAlphaFilePath_WCHAR_T( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, NULL ) ;

	return Texture->AlphaFilePathW_ ;
}

// テクスチャで使用するグラフィックハンドルを変更する( GrHandle を -1 にすると解除 )
extern int NS_MV1SetTextureGraphHandle( int MHandle, int TexIndex, int GrHandle, int SemiTransFlag )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	// GrHandle が -1 かどうかで処理を分岐
	if( GrHandle == -1 )
	{
		if( Texture->UseUserGraphHandle == FALSE && Texture->UseGraphHandle == 0 )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		Texture->UseUserGraphHandle = FALSE ;
		Texture->UserGraphHandle = 0 ;
	}
	else
	{
		int TexWidth, TexHeight ;

		NS_GetGraphSize( GrHandle, &TexWidth, &TexHeight ) ;
		if( Texture->UseGraphHandle == TRUE &&
			Texture->UserGraphHandle == GrHandle &&
			Texture->UserGraphHandleSemiTransFlag == SemiTransFlag &&
			TexWidth == Texture->UserGraphWidth &&
			TexHeight == Texture->UserGraphHeight )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		Texture->UseUserGraphHandle = TRUE ;
		Texture->UserGraphHandle = GrHandle ;
		Texture->UserGraphHandleSemiTransFlag = SemiTransFlag ;
		Texture->UserGraphWidth = TexWidth ;
		Texture->UserGraphHeight = TexHeight ;
	}

	// 終了
	return 0 ;
}

// テクスチャのグラフィックハンドルを取得する
extern int NS_MV1GetTextureGraphHandle( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->UseUserGraphHandle )
	{
		return Texture->UserGraphHandle ;
	}
	else
	if( Texture->UseGraphHandle )
	{
		return Texture->GraphHandle ;
	}
	else
	{
		return MV1GetTextureGraphHandleBase( MV1GetModelBaseHandle( MHandle ), TexIndex ) ;
	}
}

// テクスチャのアドレスモードを設定する
extern int NS_MV1SetTextureAddressMode( int MHandle, int TexIndex, int AddrUMode, int AddrVMode )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->AddressModeU == AddrUMode &&
		Texture->AddressModeV == AddrVMode )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Texture->AddressModeU = AddrUMode ;
	Texture->AddressModeV = AddrVMode ;

	// 終了
	return 0 ;
}

// テクスチャのＵ値のアドレスモードを取得する( 戻り値:DX_TEXADDRESS_WRAP 等 )
extern int NS_MV1GetTextureAddressModeU( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->AddressModeU ;
}

// テクスチャのＶ値のアドレスモードを取得する( 戻り値:DX_TEXADDRESS_WRAP 等 )
extern int NS_MV1GetTextureAddressModeV( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->AddressModeV ;
}

// テクスチャの幅を取得する
extern int NS_MV1GetTextureWidth( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->UseUserGraphHandle )
	{
		return Texture->UserGraphWidth ;
	}
	else
	if( Texture->UseGraphHandle )
	{
		return Texture->Width ;
	}
	else
	{
		return MV1GetTextureWidthBase( MV1GetModelBaseHandle( MHandle ), TexIndex ) ;
	}
}

// テクスチャの高さを取得する
extern int NS_MV1GetTextureHeight( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->UseUserGraphHandle )
	{
		return Texture->UserGraphHeight ;
	}
	else
	if( Texture->UseGraphHandle )
	{
		return Texture->Height ;
	}
	else
	{
		return MV1GetTextureHeightBase( MV1GetModelBaseHandle( MHandle ), TexIndex ) ;
	}
}

// テクスチャに半透明要素があるかどうかを取得する( 戻り値  TRUE:ある  FALSE:ない )
extern int NS_MV1GetTextureSemiTransState( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->UseUserGraphHandle )
	{
		return Texture->UserGraphHandleSemiTransFlag ;
	}
	else
	if( Texture->UseGraphHandle )
	{
		return Texture->SemiTransFlag ;
	}
	else
	{
		return MV1GetTextureSemiTransStateBase( MV1GetModelBaseHandle( MHandle ), TexIndex ) ;
	}
}

// テクスチャで使用している画像がバンプマップかどうかを設定する
extern int NS_MV1SetTextureBumpImageFlag( int MHandle, int TexIndex, int Flag )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	return _MV1TextureLoad(
				ModelBase, Texture,
				Texture->ColorFilePathW_, Texture->AlphaFilePathW_,
				Flag, Texture->BumpImageNextPixelLength,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0, FALSE ) ;
}

// テクスチャがバンプマップかどうかを取得する( 戻り値  TRUE:バンプマップ  FALSE:違う )
extern int NS_MV1GetTextureBumpImageFlag( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->BumpImageFlag ;
}

// バンプマップ画像の場合の隣のピクセルとの距離を設定する
extern int NS_MV1SetTextureBumpImageNextPixelLength( int MHandle, int TexIndex, float Length )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->BumpImageNextPixelLength == Length )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Texture->BumpImageNextPixelLength = Length ;

	return _MV1TextureLoad(
				ModelBase, Texture,
				Texture->ColorFilePathW_, Texture->AlphaFilePathW_,
				Texture->BumpImageFlag, Length,
				Texture->ReverseFlag != 0,
				Texture->Bmp32AllZeroAlphaToXRGB8Flag != 0, FALSE ) ;
}

// バンプマップ画像の場合の隣のピクセルとの距離を取得する
extern float NS_MV1GetTextureBumpImageNextPixelLength( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1.0f ) ;

	return Texture->BumpImageNextPixelLength ;
}

// テクスチャのフィルタリングモードを設定する
extern int NS_MV1SetTextureSampleFilterMode( int MHandle, int TexIndex, int FilterMode )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	if( Texture->FilterMode == FilterMode )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Texture->FilterMode = FilterMode ;

	// 終了
	return 0 ;
}

// テクスチャのフィルタリングモードを取得する( 戻り値  MV1_TEXTURE_FILTER_MODE_POINT等 )
extern int NS_MV1GetTextureSampleFilterMode( int MHandle, int TexIndex )
{
	MV1TEXTURESTART( MHandle, Model, ModelBase, Texture, TexIndex, -1 ) ;

	return Texture->FilterMode ;
}

// ３Ｄモデルに貼り付けるのに向いた画像の読み込み方式で画像を読み込む( 戻り値  -1:エラー  0以上:グラフィックハンドル )
extern int NS_MV1LoadTexture( const TCHAR *FilePath )
{
#ifdef UNICODE
	return MV1LoadTexture_WCHAR_T(
		FilePath
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FilePath, return -1 )

	Result = MV1LoadTexture_WCHAR_T(
		UseFilePathBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FilePath )

	return Result ;
#endif
}

// ３Ｄモデルに貼り付けるのに向いた画像の読み込み方式で画像を読み込む( 戻り値  -1:エラー  0以上:グラフィックハンドル )
extern int MV1LoadTexture_WCHAR_T( const wchar_t *FilePath )
{
	int NewGraphHandle ;
	void *ColorImage, *AlphaImage ;
	int ColorImageSize, AlphaImageSize ;
	int SemiTransFlag, DefaultTextureFlag ;
	const wchar_t *ColorFilePath ;

	ColorFilePath = FilePath ;
	if( __MV1LoadTexture(
			&ColorImage, &ColorImageSize,
			&AlphaImage, &AlphaImageSize,
			&NewGraphHandle,
			&SemiTransFlag,
			&DefaultTextureFlag,
#ifndef UNICODE
			NULL, NULL,
#endif
			NULL, NULL,
			ColorFilePath, NULL, NULL,
			FALSE, 0.1f,
			FALSE,
			FALSE,
			NULL,
			false,
			FALSE ) == -1 )
		return -1 ;

	if( ColorImage )
	{
		DXFREE( ColorImage ) ;
		ColorImage = NULL ;
	}

	if( AlphaImage )
	{
		DXFREE( AlphaImage ) ;
		AlphaImage = NULL ;
	}

	// ハンドルを返す
	return NewGraphHandle ;
}















// フレームの数を取得する
extern int NS_MV1GetFrameNum( int MHandle )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// フレームの数を返す
	return ModelBase->FrameNum ;
}

// フレームの名前からモデル中のフレームのインデックスを取得する( 無かった場合は戻り値が-1 )
extern int NS_MV1SearchFrame( int MHandle, const TCHAR *FrameName )
{
#ifdef UNICODE
	return MV1SearchFrame_WCHAR_T(
		MHandle, FrameName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( FrameName, return -1 )

	Result = MV1SearchFrame_WCHAR_T(
		MHandle, UseFrameNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( FrameName )

	return Result ;
#endif
}

// フレームの名前からモデル中のフレームのインデックスを取得する( 無かった場合は戻り値が-1 )
extern int MV1SearchFrame_WCHAR_T( int MHandle, const wchar_t *FrameName )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME_BASE *MBFrame ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 同名のフレームを探す
	MBFrame = ModelBase->Frame ;
	for( i = 0 ; i < ModelBase->FrameNum && _WCSCMP( MBFrame->NameW, FrameName ) != 0 ; i ++, MBFrame ++ ){}

	// フレームのインデックスを返す
	return i == ModelBase->FrameNum ? -2 : i ;
}

// フレームの名前から指定のフレームの子フレームのフレームインデックスを取得する( 名前指定版 )( FrameIndex を -1 にすると親を持たないフレームを ChildIndex で指定する )( 無かった場合は戻り値が-1 )
extern int NS_MV1SearchFrameChild( int MHandle, int FrameIndex, const TCHAR *ChildName )
{
#ifdef UNICODE
	return MV1SearchFrameChild_WCHAR_T(
		MHandle, FrameIndex, ChildName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( ChildName, return -1 )

	Result = MV1SearchFrameChild_WCHAR_T(
		MHandle, FrameIndex, UseChildNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( ChildName )

	return Result ;
#endif
}

// フレームの名前から指定のフレームの子フレームのフレームインデックスを取得する( 名前指定版 )( FrameIndex を -1 にすると親を持たないフレームを ChildIndex で指定する )( 無かった場合は戻り値が-1 )
extern int MV1SearchFrameChild_WCHAR_T( int MHandle, int FrameIndex, const wchar_t *ChildName )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	int i ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// 名前のアドレスが不正だったらエラー
	if( ChildName == NULL ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら -1 を返す
	if( FrameIndex < -1 || FrameIndex >= ModelBase->FrameNum )
		return -1 ;

	// フレームインデックスが -1 かどうかで処理を分岐
	if( FrameIndex == -1 )
	{
		// トップフレームの中から指定の名前のフレームを探す
		for( i = 0 ; i < Model->TopFrameNum && _WCSCMP( Model->TopFrameList[ i ]->BaseData->NameW, ChildName ) != 0 ; i ++ ){}
		return i == Model->TopFrameNum ? -2 : ( int )( Model->TopFrameList[ i ] - Model->Frame ) ;
	}
	else
	{
		// 子インデックスが不正だったら -1 を返す
		Frame = &Model->Frame[ FrameIndex ] ;

		// 子フレームの中から指定の名前のフレームを探す
		for( i = 0 ; i < Frame->BaseData->TotalChildNum && _WCSCMP( Frame->Child[ i ].BaseData->NameW, ChildName ) != 0 ; i ++ ){}
		return i == Frame->BaseData->TotalChildNum ? -2 : ( int )( &Frame->Child[ i ] - Model->Frame ) ;
	}
}

// 指定のフレームの名前を取得する( エラーの場合は戻り値が NULL )
extern const TCHAR *NS_MV1GetFrameName( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, NULL ) ;

	// フレームの名前を返す
#ifdef UNICODE
	return Frame->BaseData->NameW ;
#else
	return Frame->BaseData->NameA ;
#endif
}

// 指定のフレームの名前を取得する( エラーの場合は戻り値が NULL )
extern const wchar_t *MV1GetFrameName_WCHAR_T( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, NULL ) ;

	// フレームの名前を返す
	return Frame->BaseData->NameW ;
}

// 指定のフレームの名前を取得する( 戻り値   -1:エラー  -1以外:文字列のサイズ )
extern int NS_MV1GetFrameName2( int MHandle, int FrameIndex, TCHAR *StrBuffer )
{
#ifdef UNICODE
	return MV1GetFrameName2_WCHAR_T( MHandle, FrameIndex, StrBuffer ) ;
#else
	int Length = 0 ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, NULL ) ;

	if( StrBuffer != NULL )
	{
		_STRCPY( StrBuffer, Frame->BaseData->NameA ) ;
	}

	Length = _STRLEN( Frame->BaseData->NameA ) ;

	return Length ;
#endif
}

// 指定のフレームの名前を取得する( 戻り値   -1:エラー  -1以外:文字列のサイズ )
extern int MV1GetFrameName2_WCHAR_T( int MHandle, int FrameIndex, wchar_t *StrBuffer )
{
	int Length = 0 ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, NULL ) ;

	if( StrBuffer != NULL )
	{
		_WCSCPY( StrBuffer, Frame->BaseData->NameW ) ;
	}

	Length = _WCSLEN( Frame->BaseData->NameW ) ;

	return Length ;
}

// 指定のフレームの子フレームの数を取得する
extern int NS_MV1GetFrameChildNum( int MHandle, int FrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら -1 を返す
	if( FrameIndex < -1 || FrameIndex >= ModelBase->FrameNum )
		return -1 ;

	// 子フレームの数を返す
	return FrameIndex == -1 ? Model->TopFrameNum : Model->Frame[ FrameIndex ].ChildNum ;
}

// 指定のフレームの親フレームのインデックスを得る( 親がいない場合は -2 が返る )
extern int NS_MV1GetFrameParent( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 親フレームのフレームインデックスを返す
	return Frame->Parent == NULL ? -2 : ( int )( Frame->Parent - Model->Frame ) ;
}

// 指定のフレームの子フレームのフレームインデックスを取得する( 番号指定版 )( FrameIndex を -1 にすると親を持たないフレームを ChildIndex で指定する )( エラーの場合は戻り値が-1 )
extern int NS_MV1GetFrameChild( int MHandle, int FrameIndex, int ChildIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// インデックスが不正だったら -1 を返す
	if( FrameIndex < -1 || FrameIndex >= ModelBase->FrameNum )
		return -1 ;

	// フレームインデックスが -1 かどうかで処理を分岐
	if( FrameIndex == -1 )
	{
		// 子インデックスが不正だったら -1 を返す
		if( ChildIndex < 0 || ChildIndex >= Model->TopFrameNum )
			return -1 ;

		// トップフレームのフレームインデックスを返す
		return ( int )( Model->TopFrameList[ ChildIndex ] - Model->Frame ) ;
	}
	else
	{
		// 子インデックスが不正だったら -1 を返す
		Frame = &Model->Frame[ FrameIndex ] ;
		if( ChildIndex < 0 || ChildIndex >= Frame->ChildNum )
			return -1 ;

		// 子フレームのフレームインデックスを返す
		return ( int )( Frame->ChildList[ ChildIndex ] - Model->Frame ) ;
	}
}

// 指定のフレームの座標を取得する
extern VECTOR NS_MV1GetFramePosition( int MHandle, int FrameIndex )
{
	VECTOR Pos ;
	VECTOR ErrorRet = { 0.0f, 0.0f, 0.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// フレームの座標を返す
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Pos.x = ( float )Frame->LocalWorldMatrix.md.m[ 0 ][ 3 ] ;
		Pos.y = ( float )Frame->LocalWorldMatrix.md.m[ 1 ][ 3 ] ;
		Pos.z = ( float )Frame->LocalWorldMatrix.md.m[ 2 ][ 3 ] ;
	}
	else
	{
		Pos.x = Frame->LocalWorldMatrix.mf.m[ 0 ][ 3 ] ;
		Pos.y = Frame->LocalWorldMatrix.mf.m[ 1 ][ 3 ] ;
		Pos.z = Frame->LocalWorldMatrix.mf.m[ 2 ][ 3 ] ;
	}

	return Pos ;
}

// 指定のフレームの座標を取得する
extern VECTOR_D NS_MV1GetFramePositionD( int MHandle, int FrameIndex )
{
	VECTOR_D Pos ;
	VECTOR_D ErrorRet = { 0.0f, 0.0f, 0.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// フレームの座標を返す
	if( GSYS.DrawSetting.Large3DPositionSupport )
	{
		Pos.x = Frame->LocalWorldMatrix.md.m[ 0 ][ 3 ] ;
		Pos.y = Frame->LocalWorldMatrix.md.m[ 1 ][ 3 ] ;
		Pos.z = Frame->LocalWorldMatrix.md.m[ 2 ][ 3 ] ;
	}
	else
	{
		Pos.x = Frame->LocalWorldMatrix.mf.m[ 0 ][ 3 ] ;
		Pos.y = Frame->LocalWorldMatrix.mf.m[ 1 ][ 3 ] ;
		Pos.z = Frame->LocalWorldMatrix.mf.m[ 2 ][ 3 ] ;
	}

	return Pos ;
}

// 指定のフレームの初期状態での座標変換行列を取得する
extern MATRIX NS_MV1GetFrameBaseLocalMatrix( int MHandle, int FrameIndex )
{
	MATRIX ResultMatrix ;

	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, MGetIdent() ) ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// 行列を返す
	ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &Frame->BaseData->LocalTransformMatrix ) ;
	return ResultMatrix ;
}

// 指定のフレームの初期状態での座標変換行列を取得する
extern MATRIX_D NS_MV1GetFrameBaseLocalMatrixD( int MHandle, int FrameIndex )
{
	MATRIX_D ResultMatrix ;

	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, MGetIdentD() ) ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// 行列を返す
	ConvertMatrix4x4cFToMatrixD( &ResultMatrix, &Frame->BaseData->LocalTransformMatrix ) ;
	return ResultMatrix ;
}

// 指定のフレームのローカル座標変換行列を取得する
#ifdef __BCC
static DummyFunc_MV1GetFrameLocalMatrixF(){}
#endif
extern MATRIX NS_MV1GetFrameLocalMatrix( int MHandle, int FrameIndex )
{
	MATRIX ResultMatrix ;
	MV1_FRAME_BASE *FrameBase ;

// BorlandC++ の最適化バグ抑制用
#ifdef __BCC
	DummyFunc_MV1GetFrameLocalMatrixF();
#endif

	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, MGetIdent() ) ;
	FrameBase = Frame->BaseData ;

	// 行列のセットアップを行う
	MV1SETUPMATRIX( Model ) ;

	// ユーザー指定の行列がある場合はそれを返す
	if( Frame->ValidUserLocalTransformMatrix )
	{
		ConvertMatrix4x4cToMatrixF( &ResultMatrix, &Frame->UserLocalTransformMatrix ) ;
	}
	else
	// アニメーションがある場合と無い場合で処理を分岐
	if( Model->AnimSetNum == 0 )
	{
		// アニメーションがない場合はデフォルト行列を返す
		ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &FrameBase->LocalTransformMatrix ) ;
	}
	else
	{
		// アニメーションがある場合
		MATRIX_4X4CT_F BlendMat ;
		float BlendRate ;
		VECTOR Translate, Scale, Rotate ;
		FLOAT4 Quaternion ;
		int BlendFlag, mcon, i ;
		MV1_MODEL_ANIM *MAnim, *MAnim2, *MAnim3 = NULL ;

		// パラメータレベルのブレンドが行えるかを調べる
		MAnim = Model->Anim + Model->AnimSetMaxNum * Frame->BaseData->Index ;
		MAnim2 = MAnim ;
		BlendFlag = 0 ;
		mcon = 0 ;
		for( i = 0 ; i < Model->AnimSetMaxNum ; i ++, MAnim2 ++ )
		{
			if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f )
				continue ;
			mcon ++ ;
			MAnim3 = MAnim2 ;

			BlendFlag |= MAnim2->Anim->ValidFlag ;
		}

		// アニメーションが再生されていない場合はデフォルトの行列を適応する
		if( mcon == 0 )
		{
			if( FrameBase->LocalTransformMatrixType == 0 )
			{
				CreateIdentityMatrix( &ResultMatrix ) ;
			}
			else
			if( FrameBase->LocalTransformMatrixType == 1 )
			{
				CreateTranslationMatrix( &ResultMatrix, FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ], FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ], FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ) ;
			}
			else
			{
				ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &FrameBase->LocalTransformMatrix ) ;
			}
		}
		else
		// アニメーションが一つだけ再生されている場合は別処理
		if( mcon == 1 )
		{
			if( MAnim3->Anim->ValidFlag & MV1_ANIMVALUE_MATRIX )
			{
				ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &MAnim3->Anim->Matrix ) ;
			}
			else
			{
				// 行列のセットアップ
				if( MAnim3->Anim->ValidBlendMatrix == false )
				{
					MV1SetupTransformMatrix(
						&MAnim3->Anim->BlendMatrix,
						MAnim3->Anim->ValidFlag,
						&MAnim3->Anim->Translate,
						&MAnim3->Anim->Scale,
						MAnim3->Anim->RotateOrder,
						( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
						&MAnim3->Anim->Rotate,
						( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
						&MAnim3->Anim->Quaternion
					) ;
					MAnim3->Anim->ValidBlendMatrix = true ;
				}
				ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &MAnim3->Anim->BlendMatrix ) ;
			}
		}
		else
		{
			MV1_ANIM * RST Anim ;
			VECTOR DivSize ;

			// 行列があるか、クォータニオンとＸＹＺ軸回転が混同しているか
			// デフォルトパラメータが無効な上に当ててあるアニメーションの種類が違う場合は行列ブレンド
//			if( ( BlendFlag & MV1_ANIMVALUE_MATRIX ) ||
//				( BlendFlag & ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_X   ) ) == ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_X ) ||
//				( BlendFlag & ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_VMD ) ) == ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_VMD ) )
			if( 1 )
			{
				int Normalize ;

				Normalize = TRUE ;

				_MEMSET( &BlendMat, 0, sizeof( BlendMat ) ) ;
				MAnim2 = MAnim ;
				for( i = 0 ; i < Model->AnimSetMaxNum ; i ++, MAnim2 ++ )
				{
					if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f ) continue ;

					BlendRate = MAnim2->BlendRate ;
					Anim = MAnim2->Anim ;

					if( Anim->ValidFlag & MV1_ANIMVALUE_MATRIX )
					{
						if( BlendRate == 1.0f )
						{
							BlendMat.m[ 0 ][ 0 ] += Anim->Matrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ;
							BlendMat.m[ 0 ][ 1 ] += Anim->Matrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ;
							BlendMat.m[ 0 ][ 2 ] += Anim->Matrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ;
							BlendMat.m[ 0 ][ 3 ] += Anim->Matrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ;

							BlendMat.m[ 1 ][ 0 ] += Anim->Matrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ;
							BlendMat.m[ 1 ][ 1 ] += Anim->Matrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ;
							BlendMat.m[ 1 ][ 2 ] += Anim->Matrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ;
							BlendMat.m[ 1 ][ 3 ] += Anim->Matrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ;

							BlendMat.m[ 2 ][ 0 ] += Anim->Matrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ;
							BlendMat.m[ 2 ][ 1 ] += Anim->Matrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ;
							BlendMat.m[ 2 ][ 2 ] += Anim->Matrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ;
							BlendMat.m[ 2 ][ 3 ] += Anim->Matrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ;
						}
						else
						{
							BlendMat.m[ 0 ][ 0 ] += ( Anim->Matrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 1 ] += ( Anim->Matrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 2 ] += ( Anim->Matrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 3 ] += ( Anim->Matrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 1 ][ 0 ] += ( Anim->Matrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 1 ] += ( Anim->Matrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 2 ] += ( Anim->Matrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 3 ] += ( Anim->Matrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 2 ][ 0 ] += ( Anim->Matrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 1 ] += ( Anim->Matrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 2 ] += ( Anim->Matrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 3 ] += ( Anim->Matrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ) * BlendRate ;
						}

						Normalize = FALSE ;
					}
					else
					{
						// 行列のセットアップ
						if( Anim->ValidBlendMatrix == false )
						{
							MV1SetupTransformMatrix(
								&Anim->BlendMatrix,
								Anim->ValidFlag,
								&Anim->Translate,
								&Anim->Scale,
								Anim->RotateOrder,
								( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
								&Anim->Rotate,
								( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
								&Anim->Quaternion ) ;
							Anim->ValidBlendMatrix = true ;

							if( ( Anim->ValidFlag & MV1_ANIMVALUE_SCALE ) &&
								( Anim->Scale.x < 0.9999999f || Anim->Scale.x > 1.0000001f ||
								  Anim->Scale.y < 0.9999999f || Anim->Scale.y > 1.0000001f ||
								  Anim->Scale.z < 0.9999999f || Anim->Scale.z > 1.0000001f ) )
							{
								Anim->BlendMatrixUseScaling = true ;
							}
						}

						if( Anim->BlendMatrixUseScaling )
						{
							Normalize = FALSE ;
						}

						if( BlendRate == 1.0f )
						{
							BlendMat.m[ 0 ][ 0 ] += Anim->BlendMatrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ;
							BlendMat.m[ 0 ][ 1 ] += Anim->BlendMatrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ;
							BlendMat.m[ 0 ][ 2 ] += Anim->BlendMatrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ;
							BlendMat.m[ 0 ][ 3 ] += Anim->BlendMatrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ;

							BlendMat.m[ 1 ][ 0 ] += Anim->BlendMatrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ;
							BlendMat.m[ 1 ][ 1 ] += Anim->BlendMatrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ;
							BlendMat.m[ 1 ][ 2 ] += Anim->BlendMatrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ;
							BlendMat.m[ 1 ][ 3 ] += Anim->BlendMatrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ;

							BlendMat.m[ 2 ][ 0 ] += Anim->BlendMatrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ;
							BlendMat.m[ 2 ][ 1 ] += Anim->BlendMatrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ;
							BlendMat.m[ 2 ][ 2 ] += Anim->BlendMatrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ;
							BlendMat.m[ 2 ][ 3 ] += Anim->BlendMatrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ;
						}
						else
						{
							BlendMat.m[ 0 ][ 0 ] += ( Anim->BlendMatrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 1 ] += ( Anim->BlendMatrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 2 ] += ( Anim->BlendMatrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 3 ] += ( Anim->BlendMatrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 1 ][ 0 ] += ( Anim->BlendMatrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 1 ] += ( Anim->BlendMatrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 2 ] += ( Anim->BlendMatrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 3 ] += ( Anim->BlendMatrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 2 ][ 0 ] += ( Anim->BlendMatrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 1 ] += ( Anim->BlendMatrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 2 ] += ( Anim->BlendMatrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 3 ] += ( Anim->BlendMatrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ) * BlendRate ;
						}
					}
				}

				BlendMat.m[ 0 ][ 0 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ;
				BlendMat.m[ 0 ][ 1 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ;
				BlendMat.m[ 0 ][ 2 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ;
				BlendMat.m[ 0 ][ 3 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ;

				BlendMat.m[ 1 ][ 0 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ;
				BlendMat.m[ 1 ][ 1 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ;
				BlendMat.m[ 1 ][ 2 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ;
				BlendMat.m[ 1 ][ 3 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ;

				BlendMat.m[ 2 ][ 0 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ;
				BlendMat.m[ 2 ][ 1 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ;
				BlendMat.m[ 2 ][ 2 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ;
				BlendMat.m[ 2 ][ 3 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ;

				if( FrameBase->LocalTransformMatrixUseScaling )
				{
					Normalize = FALSE ;
				}

				if( Normalize )
				{
					DivSize.x = 1.0f / _SQRT( BlendMat.m[ 0 ][ 0 ] * BlendMat.m[ 0 ][ 0 ] + BlendMat.m[ 0 ][ 1 ] * BlendMat.m[ 0 ][ 1 ] + BlendMat.m[ 0 ][ 2 ] * BlendMat.m[ 0 ][ 2 ] ) ;
					DivSize.y = 1.0f / _SQRT( BlendMat.m[ 1 ][ 0 ] * BlendMat.m[ 1 ][ 0 ] + BlendMat.m[ 1 ][ 1 ] * BlendMat.m[ 1 ][ 1 ] + BlendMat.m[ 1 ][ 2 ] * BlendMat.m[ 1 ][ 2 ] ) ;
					DivSize.z = 1.0f / _SQRT( BlendMat.m[ 2 ][ 0 ] * BlendMat.m[ 2 ][ 0 ] + BlendMat.m[ 2 ][ 1 ] * BlendMat.m[ 2 ][ 1 ] + BlendMat.m[ 2 ][ 2 ] * BlendMat.m[ 2 ][ 2 ] ) ;

					BlendMat.m[ 0 ][ 0 ] *= DivSize.x ;
					BlendMat.m[ 0 ][ 1 ] *= DivSize.x ;
					BlendMat.m[ 0 ][ 2 ] *= DivSize.x ;

					BlendMat.m[ 1 ][ 0 ] *= DivSize.y ;
					BlendMat.m[ 1 ][ 1 ] *= DivSize.y ;
					BlendMat.m[ 1 ][ 2 ] *= DivSize.y ;

					BlendMat.m[ 2 ][ 0 ] *= DivSize.z ;
					BlendMat.m[ 2 ][ 1 ] *= DivSize.z ;
					BlendMat.m[ 2 ][ 2 ] *= DivSize.z ;
				}
			}
			else
			{
				// それ以外の場合はパラメータレベルのブレンド処理
				Translate.x = 0.0f ;
				Translate.y = 0.0f ;
				Translate.z = 0.0f ;
				Scale.x = 0.0f ;
				Scale.y = 0.0f ;
				Scale.z = 0.0f ;
				Rotate.x = 0.0f ;
				Rotate.y = 0.0f ;
				Rotate.z = 0.0f ;
				Quaternion.x = 0.0f ;
				Quaternion.y = 0.0f ;
				Quaternion.z = 0.0f ;
				Quaternion.w = 1.0f ;

				MAnim2 = MAnim ;
				for( i = 0 ; i < Model->AnimSetMaxNum ; i ++, MAnim2 ++ )
				{
					if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f ) continue ;

					BlendRate = MAnim2->BlendRate ;
					Anim = MAnim2->Anim ;

					if( BlendRate == 1.0f )
					{
						if( Anim->ValidFlag & MV1_ANIMVALUE_TRANSLATE )
						{
							Translate.x += Anim->Translate.x - FrameBase->Translate.x ;
							Translate.y += Anim->Translate.y - FrameBase->Translate.y ;
							Translate.z += Anim->Translate.z - FrameBase->Translate.z ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_SCALE )
						{
							Scale.x += Anim->Scale.x - FrameBase->Scale.x ;
							Scale.y += Anim->Scale.y - FrameBase->Scale.y ;
							Scale.z += Anim->Scale.z - FrameBase->Scale.z ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_ROTATE )
						{
							Rotate.x += Anim->Rotate.x - FrameBase->Rotate.x ;
							Rotate.y += Anim->Rotate.y - FrameBase->Rotate.y ;
							Rotate.z += Anim->Rotate.z - FrameBase->Rotate.z ;
						}

						if( Anim->ValidFlag & ( MV1_ANIMVALUE_QUATERNION_X | MV1_ANIMVALUE_QUATERNION_VMD ) )
						{
							Quaternion.x = Anim->Quaternion.x - FrameBase->Quaternion.x ;
							Quaternion.y = Anim->Quaternion.y - FrameBase->Quaternion.y ;
							Quaternion.z = Anim->Quaternion.z - FrameBase->Quaternion.z ;
							Quaternion.w = Anim->Quaternion.w - FrameBase->Quaternion.w ;
						}
					}
					else
					{
						if( Anim->ValidFlag & MV1_ANIMVALUE_TRANSLATE )
						{
							Translate.x += ( Anim->Translate.x - FrameBase->Translate.x ) * BlendRate ;
							Translate.y += ( Anim->Translate.y - FrameBase->Translate.y ) * BlendRate ;
							Translate.z += ( Anim->Translate.z - FrameBase->Translate.z ) * BlendRate ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_SCALE )
						{
							Scale.x += ( Anim->Scale.x - FrameBase->Scale.x ) * BlendRate ;
							Scale.y += ( Anim->Scale.y - FrameBase->Scale.y ) * BlendRate ;
							Scale.z += ( Anim->Scale.z - FrameBase->Scale.z ) * BlendRate ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_ROTATE )
						{
							Rotate.x += ( Anim->Rotate.x - FrameBase->Rotate.x ) * BlendRate ;
							Rotate.y += ( Anim->Rotate.y - FrameBase->Rotate.y ) * BlendRate ;
							Rotate.z += ( Anim->Rotate.z - FrameBase->Rotate.z ) * BlendRate ;
						}

						if( Anim->ValidFlag & ( MV1_ANIMVALUE_QUATERNION_X | MV1_ANIMVALUE_QUATERNION_VMD ) )
						{
							Quaternion.x = ( Anim->Quaternion.x - FrameBase->Quaternion.x ) * BlendRate ;
							Quaternion.y = ( Anim->Quaternion.y - FrameBase->Quaternion.y ) * BlendRate ;
							Quaternion.z = ( Anim->Quaternion.z - FrameBase->Quaternion.z ) * BlendRate ;
							Quaternion.w = ( Anim->Quaternion.w - FrameBase->Quaternion.w ) * BlendRate ;
						}
					}
				}

				Translate.x += FrameBase->Translate.x ;
				Translate.y += FrameBase->Translate.y ;
				Translate.z += FrameBase->Translate.z ;

				Scale.x += FrameBase->Scale.x ;
				Scale.y += FrameBase->Scale.y ;
				Scale.z += FrameBase->Scale.z ;

				Rotate.x += FrameBase->Rotate.x ;
				Rotate.y += FrameBase->Rotate.y ;
				Rotate.z += FrameBase->Rotate.z ;

				Quaternion.x += FrameBase->Quaternion.x ;
				Quaternion.y += FrameBase->Quaternion.y ;
				Quaternion.z += FrameBase->Quaternion.z ;
				Quaternion.w += FrameBase->Quaternion.w ;

				// 行列のセットアップ
				MV1SetupTransformMatrix(
					&BlendMat,
					BlendFlag,
					&Translate,
					&Scale,
					FrameBase->RotateOrder,
					( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
					&Rotate,
					( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
					&Quaternion
				) ;
			}

			// 戻り値用の行列に置き換える
			ConvertMatrix4x4cFToMatrixF( &ResultMatrix, &BlendMat ) ;
		}
	}

	// 戻り値として返す
	return ResultMatrix ;
}

// 指定のフレームのローカル座標変換行列を取得する
#ifdef __BCC
static DummyFunc_MV1GetFrameLocalMatrixD(){}
#endif
extern MATRIX_D NS_MV1GetFrameLocalMatrixD( int MHandle, int FrameIndex )
{
	MATRIX_D ResultMatrix ;
	MV1_FRAME_BASE *FrameBase ;

// BorlandC++ の最適化バグ抑制用
#ifdef __BCC
	DummyFunc_MV1GetFrameLocalMatrixD();
#endif

	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, MGetIdentD() ) ;
	FrameBase = Frame->BaseData ;

	// 行列のセットアップを行う
	MV1SETUPMATRIX( Model ) ;

	// ユーザー指定の行列がある場合はそれを返す
	if( Frame->ValidUserLocalTransformMatrix )
	{
		ConvertMatrix4x4cToMatrixD( &ResultMatrix, &Frame->UserLocalTransformMatrix ) ;
	}
	else
	// アニメーションがある場合と無い場合で処理を分岐
	if( Model->AnimSetNum == 0 )
	{
		// アニメーションがない場合はデフォルト行列を返す
		ConvertMatrix4x4cFToMatrixD( &ResultMatrix, &FrameBase->LocalTransformMatrix ) ;
	}
	else
	{
		// アニメーションがある場合
		MATRIX_4X4CT_F BlendMat ;
		float BlendRate ;
		VECTOR Translate, Scale, Rotate ;
		FLOAT4 Quaternion ;
		int BlendFlag, mcon, i ;
		MV1_MODEL_ANIM *MAnim, *MAnim2, *MAnim3 = NULL ;

		// パラメータレベルのブレンドが行えるかを調べる
		MAnim = Model->Anim + Model->AnimSetMaxNum * Frame->BaseData->Index ;
		MAnim2 = MAnim ;
		BlendFlag = 0 ;
		mcon = 0 ;
		for( i = 0 ; i < Model->AnimSetMaxNum ; i ++, MAnim2 ++ )
		{
			if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f )
				continue ;
			mcon ++ ;
			MAnim3 = MAnim2 ;

			BlendFlag |= MAnim2->Anim->ValidFlag ;
		}

		// アニメーションが再生されていない場合はデフォルトの行列を適応する
		if( mcon == 0 )
		{
			if( FrameBase->LocalTransformMatrixType == 0 )
			{
				CreateIdentityMatrixD( &ResultMatrix ) ;
			}
			else
			if( FrameBase->LocalTransformMatrixType == 1 )
			{
				CreateTranslationMatrixD( &ResultMatrix, FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ], FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ], FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ) ;
			}
			else
			{
				ConvertMatrix4x4cFToMatrixD( &ResultMatrix, &FrameBase->LocalTransformMatrix ) ;
			}
		}
		else
		// アニメーションが一つだけ再生されている場合は別処理
		if( mcon == 1 )
		{
			if( MAnim3->Anim->ValidFlag & MV1_ANIMVALUE_MATRIX )
			{
				ConvertMatrix4x4cFToMatrixD( &ResultMatrix, &MAnim3->Anim->Matrix ) ;
			}
			else
			{
				// 行列のセットアップ
				if( MAnim3->Anim->ValidBlendMatrix == false )
				{
					MV1SetupTransformMatrix(
						&MAnim3->Anim->BlendMatrix,
						MAnim3->Anim->ValidFlag,
						&MAnim3->Anim->Translate,
						&MAnim3->Anim->Scale,
						MAnim3->Anim->RotateOrder,
						( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
						&MAnim3->Anim->Rotate,
						( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
						&MAnim3->Anim->Quaternion
					) ;
					MAnim3->Anim->ValidBlendMatrix = true ;
				}
				ConvertMatrix4x4cFToMatrixD( &ResultMatrix, &MAnim3->Anim->BlendMatrix ) ;
			}
		}
		else
		{
			MV1_ANIM * RST Anim ;
			VECTOR DivSize ;

			// 行列があるか、クォータニオンとＸＹＺ軸回転が混同しているか
			// デフォルトパラメータが無効な上に当ててあるアニメーションの種類が違う場合は行列ブレンド
//			if( ( BlendFlag & MV1_ANIMVALUE_MATRIX ) ||
//				( BlendFlag & ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_X   ) ) == ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_X ) ||
//				( BlendFlag & ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_VMD ) ) == ( MV1_ANIMVALUE_ROTATE | MV1_ANIMVALUE_QUATERNION_VMD ) )
			if( 1 )
			{
				int Normalize ;

				Normalize = TRUE ;

				_MEMSET( &BlendMat, 0, sizeof( BlendMat ) ) ;
				MAnim2 = MAnim ;
				for( i = 0 ; i < Model->AnimSetMaxNum ; i ++, MAnim2 ++ )
				{
					if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f ) continue ;

					BlendRate = MAnim2->BlendRate ;
					Anim = MAnim2->Anim ;

					if( Anim->ValidFlag & MV1_ANIMVALUE_MATRIX )
					{
						if( BlendRate == 1.0f )
						{
							BlendMat.m[ 0 ][ 0 ] += Anim->Matrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ;
							BlendMat.m[ 0 ][ 1 ] += Anim->Matrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ;
							BlendMat.m[ 0 ][ 2 ] += Anim->Matrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ;
							BlendMat.m[ 0 ][ 3 ] += Anim->Matrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ;

							BlendMat.m[ 1 ][ 0 ] += Anim->Matrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ;
							BlendMat.m[ 1 ][ 1 ] += Anim->Matrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ;
							BlendMat.m[ 1 ][ 2 ] += Anim->Matrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ;
							BlendMat.m[ 1 ][ 3 ] += Anim->Matrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ;

							BlendMat.m[ 2 ][ 0 ] += Anim->Matrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ;
							BlendMat.m[ 2 ][ 1 ] += Anim->Matrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ;
							BlendMat.m[ 2 ][ 2 ] += Anim->Matrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ;
							BlendMat.m[ 2 ][ 3 ] += Anim->Matrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ;
						}
						else
						{
							BlendMat.m[ 0 ][ 0 ] += ( Anim->Matrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 1 ] += ( Anim->Matrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 2 ] += ( Anim->Matrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 3 ] += ( Anim->Matrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 1 ][ 0 ] += ( Anim->Matrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 1 ] += ( Anim->Matrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 2 ] += ( Anim->Matrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 3 ] += ( Anim->Matrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 2 ][ 0 ] += ( Anim->Matrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 1 ] += ( Anim->Matrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 2 ] += ( Anim->Matrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 3 ] += ( Anim->Matrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ) * BlendRate ;
						}

						Normalize = FALSE ;
					}
					else
					{
						// 行列のセットアップ
						if( Anim->ValidBlendMatrix == false )
						{
							MV1SetupTransformMatrix(
								&Anim->BlendMatrix,
								Anim->ValidFlag,
								&Anim->Translate,
								&Anim->Scale,
								Anim->RotateOrder,
								( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
								&Anim->Rotate,
								( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
								&Anim->Quaternion ) ;
							Anim->ValidBlendMatrix = true ;

							if( ( Anim->ValidFlag & MV1_ANIMVALUE_SCALE ) &&
								( Anim->Scale.x < 0.9999999f || Anim->Scale.x > 1.0000001f ||
								  Anim->Scale.y < 0.9999999f || Anim->Scale.y > 1.0000001f ||
								  Anim->Scale.z < 0.9999999f || Anim->Scale.z > 1.0000001f ) )
							{
								Anim->BlendMatrixUseScaling = true ;
							}
						}

						if( Anim->BlendMatrixUseScaling )
						{
							Normalize = FALSE ;
						}

						if( BlendRate == 1.0f )
						{
							BlendMat.m[ 0 ][ 0 ] += Anim->BlendMatrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ;
							BlendMat.m[ 0 ][ 1 ] += Anim->BlendMatrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ;
							BlendMat.m[ 0 ][ 2 ] += Anim->BlendMatrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ;
							BlendMat.m[ 0 ][ 3 ] += Anim->BlendMatrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ;

							BlendMat.m[ 1 ][ 0 ] += Anim->BlendMatrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ;
							BlendMat.m[ 1 ][ 1 ] += Anim->BlendMatrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ;
							BlendMat.m[ 1 ][ 2 ] += Anim->BlendMatrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ;
							BlendMat.m[ 1 ][ 3 ] += Anim->BlendMatrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ;

							BlendMat.m[ 2 ][ 0 ] += Anim->BlendMatrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ;
							BlendMat.m[ 2 ][ 1 ] += Anim->BlendMatrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ;
							BlendMat.m[ 2 ][ 2 ] += Anim->BlendMatrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ;
							BlendMat.m[ 2 ][ 3 ] += Anim->BlendMatrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ;
						}
						else
						{
							BlendMat.m[ 0 ][ 0 ] += ( Anim->BlendMatrix.m[ 0 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 1 ] += ( Anim->BlendMatrix.m[ 0 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 2 ] += ( Anim->BlendMatrix.m[ 0 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 0 ][ 3 ] += ( Anim->BlendMatrix.m[ 0 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 1 ][ 0 ] += ( Anim->BlendMatrix.m[ 1 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 1 ] += ( Anim->BlendMatrix.m[ 1 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 2 ] += ( Anim->BlendMatrix.m[ 1 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 1 ][ 3 ] += ( Anim->BlendMatrix.m[ 1 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ) * BlendRate ;

							BlendMat.m[ 2 ][ 0 ] += ( Anim->BlendMatrix.m[ 2 ][ 0 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 1 ] += ( Anim->BlendMatrix.m[ 2 ][ 1 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 2 ] += ( Anim->BlendMatrix.m[ 2 ][ 2 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ) * BlendRate ;
							BlendMat.m[ 2 ][ 3 ] += ( Anim->BlendMatrix.m[ 2 ][ 3 ] - FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ) * BlendRate ;
						}
					}
				}

				BlendMat.m[ 0 ][ 0 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 0 ] ;
				BlendMat.m[ 0 ][ 1 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 1 ] ;
				BlendMat.m[ 0 ][ 2 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 2 ] ;
				BlendMat.m[ 0 ][ 3 ] += FrameBase->LocalTransformMatrix.m[ 0 ][ 3 ] ;

				BlendMat.m[ 1 ][ 0 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 0 ] ;
				BlendMat.m[ 1 ][ 1 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 1 ] ;
				BlendMat.m[ 1 ][ 2 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 2 ] ;
				BlendMat.m[ 1 ][ 3 ] += FrameBase->LocalTransformMatrix.m[ 1 ][ 3 ] ;

				BlendMat.m[ 2 ][ 0 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 0 ] ;
				BlendMat.m[ 2 ][ 1 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 1 ] ;
				BlendMat.m[ 2 ][ 2 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 2 ] ;
				BlendMat.m[ 2 ][ 3 ] += FrameBase->LocalTransformMatrix.m[ 2 ][ 3 ] ;

				if( FrameBase->LocalTransformMatrixUseScaling )
				{
					Normalize = FALSE ;
				}

				if( Normalize )
				{
					DivSize.x = 1.0f / _SQRT( BlendMat.m[ 0 ][ 0 ] * BlendMat.m[ 0 ][ 0 ] + BlendMat.m[ 0 ][ 1 ] * BlendMat.m[ 0 ][ 1 ] + BlendMat.m[ 0 ][ 2 ] * BlendMat.m[ 0 ][ 2 ] ) ;
					DivSize.y = 1.0f / _SQRT( BlendMat.m[ 1 ][ 0 ] * BlendMat.m[ 1 ][ 0 ] + BlendMat.m[ 1 ][ 1 ] * BlendMat.m[ 1 ][ 1 ] + BlendMat.m[ 1 ][ 2 ] * BlendMat.m[ 1 ][ 2 ] ) ;
					DivSize.z = 1.0f / _SQRT( BlendMat.m[ 2 ][ 0 ] * BlendMat.m[ 2 ][ 0 ] + BlendMat.m[ 2 ][ 1 ] * BlendMat.m[ 2 ][ 1 ] + BlendMat.m[ 2 ][ 2 ] * BlendMat.m[ 2 ][ 2 ] ) ;

					BlendMat.m[ 0 ][ 0 ] *= DivSize.x ;
					BlendMat.m[ 0 ][ 1 ] *= DivSize.x ;
					BlendMat.m[ 0 ][ 2 ] *= DivSize.x ;

					BlendMat.m[ 1 ][ 0 ] *= DivSize.y ;
					BlendMat.m[ 1 ][ 1 ] *= DivSize.y ;
					BlendMat.m[ 1 ][ 2 ] *= DivSize.y ;

					BlendMat.m[ 2 ][ 0 ] *= DivSize.z ;
					BlendMat.m[ 2 ][ 1 ] *= DivSize.z ;
					BlendMat.m[ 2 ][ 2 ] *= DivSize.z ;
				}
			}
			else
			{
				// それ以外の場合はパラメータレベルのブレンド処理
				Translate.x = 0.0f ;
				Translate.y = 0.0f ;
				Translate.z = 0.0f ;
				Scale.x = 0.0f ;
				Scale.y = 0.0f ;
				Scale.z = 0.0f ;
				Rotate.x = 0.0f ;
				Rotate.y = 0.0f ;
				Rotate.z = 0.0f ;
				Quaternion.x = 0.0f ;
				Quaternion.y = 0.0f ;
				Quaternion.z = 0.0f ;
				Quaternion.w = 1.0f ;

				MAnim2 = MAnim ;
				for( i = 0 ; i < Model->AnimSetMaxNum ; i ++, MAnim2 ++ )
				{
					if( MAnim2->Use == false || MAnim2->BlendRate == 0.0f ) continue ;

					BlendRate = MAnim2->BlendRate ;
					Anim = MAnim2->Anim ;

					if( BlendRate == 1.0f )
					{
						if( Anim->ValidFlag & MV1_ANIMVALUE_TRANSLATE )
						{
							Translate.x += Anim->Translate.x - FrameBase->Translate.x ;
							Translate.y += Anim->Translate.y - FrameBase->Translate.y ;
							Translate.z += Anim->Translate.z - FrameBase->Translate.z ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_SCALE )
						{
							Scale.x += Anim->Scale.x - FrameBase->Scale.x ;
							Scale.y += Anim->Scale.y - FrameBase->Scale.y ;
							Scale.z += Anim->Scale.z - FrameBase->Scale.z ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_ROTATE )
						{
							Rotate.x += Anim->Rotate.x - FrameBase->Rotate.x ;
							Rotate.y += Anim->Rotate.y - FrameBase->Rotate.y ;
							Rotate.z += Anim->Rotate.z - FrameBase->Rotate.z ;
						}

						if( Anim->ValidFlag & ( MV1_ANIMVALUE_QUATERNION_X | MV1_ANIMVALUE_QUATERNION_VMD ) )
						{
							Quaternion.x = Anim->Quaternion.x - FrameBase->Quaternion.x ;
							Quaternion.y = Anim->Quaternion.y - FrameBase->Quaternion.y ;
							Quaternion.z = Anim->Quaternion.z - FrameBase->Quaternion.z ;
							Quaternion.w = Anim->Quaternion.w - FrameBase->Quaternion.w ;
						}
					}
					else
					{
						if( Anim->ValidFlag & MV1_ANIMVALUE_TRANSLATE )
						{
							Translate.x += ( Anim->Translate.x - FrameBase->Translate.x ) * BlendRate ;
							Translate.y += ( Anim->Translate.y - FrameBase->Translate.y ) * BlendRate ;
							Translate.z += ( Anim->Translate.z - FrameBase->Translate.z ) * BlendRate ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_SCALE )
						{
							Scale.x += ( Anim->Scale.x - FrameBase->Scale.x ) * BlendRate ;
							Scale.y += ( Anim->Scale.y - FrameBase->Scale.y ) * BlendRate ;
							Scale.z += ( Anim->Scale.z - FrameBase->Scale.z ) * BlendRate ;
						}

						if( Anim->ValidFlag & MV1_ANIMVALUE_ROTATE )
						{
							Rotate.x += ( Anim->Rotate.x - FrameBase->Rotate.x ) * BlendRate ;
							Rotate.y += ( Anim->Rotate.y - FrameBase->Rotate.y ) * BlendRate ;
							Rotate.z += ( Anim->Rotate.z - FrameBase->Rotate.z ) * BlendRate ;
						}

						if( Anim->ValidFlag & ( MV1_ANIMVALUE_QUATERNION_X | MV1_ANIMVALUE_QUATERNION_VMD ) )
						{
							Quaternion.x = ( Anim->Quaternion.x - FrameBase->Quaternion.x ) * BlendRate ;
							Quaternion.y = ( Anim->Quaternion.y - FrameBase->Quaternion.y ) * BlendRate ;
							Quaternion.z = ( Anim->Quaternion.z - FrameBase->Quaternion.z ) * BlendRate ;
							Quaternion.w = ( Anim->Quaternion.w - FrameBase->Quaternion.w ) * BlendRate ;
						}
					}
				}

				Translate.x += FrameBase->Translate.x ;
				Translate.y += FrameBase->Translate.y ;
				Translate.z += FrameBase->Translate.z ;

				Scale.x += FrameBase->Scale.x ;
				Scale.y += FrameBase->Scale.y ;
				Scale.z += FrameBase->Scale.z ;

				Rotate.x += FrameBase->Rotate.x ;
				Rotate.y += FrameBase->Rotate.y ;
				Rotate.z += FrameBase->Rotate.z ;

				Quaternion.x += FrameBase->Quaternion.x ;
				Quaternion.y += FrameBase->Quaternion.y ;
				Quaternion.z += FrameBase->Quaternion.z ;
				Quaternion.w += FrameBase->Quaternion.w ;

				// 行列のセットアップ
				MV1SetupTransformMatrix(
					&BlendMat,
					BlendFlag,
					&Translate,
					&Scale,
					FrameBase->RotateOrder,
					( FrameBase->Flag & MV1_FRAMEFLAG_PREROTATE ) != 0 ? &FrameBase->PreRotate : NULL,
					&Rotate,
					( FrameBase->Flag & MV1_FRAMEFLAG_POSTROTATE ) != 0 ? &FrameBase->PostRotate : NULL,
					&Quaternion
				) ;
			}

			// 戻り値用の行列に置き換える
			ConvertMatrix4x4cFToMatrixD( &ResultMatrix, &BlendMat ) ;
		}
	}

	// 戻り値として返す
	return ResultMatrix ;
}

// 指定のフレームのローカル座標からワールド座標に変換する行列を得る
extern MATRIX NS_MV1GetFrameLocalWorldMatrix( int MHandle, int FrameIndex )
{
	MATRIX ResultMatrix ;

	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, MGetIdent() ) ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// 行列を返す
	ConvertMatrix4x4cToMatrixF( &ResultMatrix, &Frame->LocalWorldMatrix ) ;
	return ResultMatrix ;
}

// 指定のフレームのローカル座標からワールド座標に変換する行列を得る
extern MATRIX_D NS_MV1GetFrameLocalWorldMatrixD( int MHandle, int FrameIndex )
{
	MATRIX_D ResultMatrix ;

	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, MGetIdentD() ) ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// 行列を返す
	ConvertMatrix4x4cToMatrixD( &ResultMatrix, &Frame->LocalWorldMatrix ) ;
	return ResultMatrix ;
}

// 指定のフレームのローカル座標変換行列を設定する
extern int NS_MV1SetFrameUserLocalMatrix( int MHandle, int FrameIndex, MATRIX Matrix )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 行列をセット
	ConvertMatrixFToMatrix4x4c( &Frame->UserLocalTransformMatrix, &Matrix ) ;

	// 独自行列を使用するフラグを立てる
	Frame->ValidUserLocalTransformMatrix = true ;

	// 行列がセットアップされていない状態にする
	Model->LocalWorldMatrixSetupFlag = false ;
	MV1BitSetChange( &Frame->LocalWorldMatrixChange ) ;

	// 終了
	return 0 ;
}

// 指定のフレームのローカル座標変換行列を設定する
extern int NS_MV1SetFrameUserLocalMatrixD( int MHandle, int FrameIndex, MATRIX_D Matrix )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 行列をセット
	ConvertMatrixDToMatrix4x4c( &Frame->UserLocalTransformMatrix, &Matrix ) ;

	// 独自行列を使用するフラグを立てる
	Frame->ValidUserLocalTransformMatrix = true ;

	// 行列がセットアップされていない状態にする
	Model->LocalWorldMatrixSetupFlag = false ;
	MV1BitSetChange( &Frame->LocalWorldMatrixChange ) ;

	// 終了
	return 0 ;
}

// 指定のフレームのローカル座標変換行列をデフォルトに戻す
extern int NS_MV1ResetFrameUserLocalMatrix( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 既にフラグが倒れていたら何もしない
	if( Frame->ValidUserLocalTransformMatrix == false ) return 0 ;

	// 独自行列を使用するフラグを倒す
	Frame->ValidUserLocalTransformMatrix = false ;

	// 行列がセットアップされていない状態にする
	Model->LocalWorldMatrixSetupFlag = false ;
	MV1BitSetChange( &Frame->LocalWorldMatrixChange ) ;

	// 終了
	return 0 ;
}

// 指定のフレームが持つメッシュ頂点のローカル座標での最大値を得る
extern VECTOR NS_MV1GetFrameMaxVertexLocalPosition( int MHandle, int FrameIndex )
{
	VECTOR ErrorRet = { 0.0f, 0.0f, 0.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 最大座標値を返す
	return Frame->BaseData->MaxPosition ;
}

// 指定のフレームが持つメッシュ頂点のローカル座標での最大値を得る
extern VECTOR_D NS_MV1GetFrameMaxVertexLocalPositionD( int MHandle, int FrameIndex )
{
	VECTOR_D ErrorRet = { 0.0, 0.0, 0.0 } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 最大座標値を返す
	return VConvFtoD( Frame->BaseData->MaxPosition ) ;
}

// 指定のフレームが持つメッシュ頂点のローカル座標での最小値を得る
extern VECTOR NS_MV1GetFrameMinVertexLocalPosition( int MHandle, int FrameIndex )
{
	VECTOR ErrorRet = { 0.0f, 0.0f, 0.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 最小座標値を返す
	return Frame->BaseData->MinPosition ;
}

// 指定のフレームが持つメッシュ頂点のローカル座標での最小値を得る
extern VECTOR_D NS_MV1GetFrameMinVertexLocalPositionD( int MHandle, int FrameIndex )
{
	VECTOR_D ErrorRet = { 0.0, 0.0, 0.0 } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 最小座標値を返す
	return VConvFtoD( Frame->BaseData->MinPosition ) ;
}

// 指定のフレームが持つメッシュ頂点のローカル座標での平均値を得る
extern VECTOR NS_MV1GetFrameAvgVertexLocalPosition( int MHandle, int FrameIndex )
{
	VECTOR ErrorRet = { 0.0f, 0.0f, 0.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 平均座標値を返す
	return VScale( VAdd( Frame->BaseData->MaxPosition, Frame->BaseData->MinPosition ), 0.5f ) ;
}

// 指定のフレームが持つメッシュ頂点のローカル座標での平均値を得る
extern VECTOR_D NS_MV1GetFrameAvgVertexLocalPositionD( int MHandle, int FrameIndex )
{
	VECTOR_D ErrorRet = { 0.0, 0.0, 0.0 } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorRet ) ;

	// 平均座標値を返す
	return VConvFtoD( VScale( VAdd( Frame->BaseData->MaxPosition, Frame->BaseData->MinPosition ), 0.5f ) ) ;
}

// 指定のフレームに含まれるポリゴンの数を取得する
extern int NS_MV1GetFrameTriangleNum( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// ポリゴンの数を返す
	return Frame->BaseData->TriangleNum ;
}

// 指定のフレームが持つメッシュの数を取得する
extern int NS_MV1GetFrameMeshNum( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// メッシュの数を返す
	return Frame->BaseData->MeshNum ;
}

// 指定のフレームが持つメッシュのメッシュインデックスを取得する
extern int NS_MV1GetFrameMesh( int MHandle, int FrameIndex, int Index )
{
	MV1_MESH_BASE *Mesh ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// メッシュインデックスが不正だったらエラー
	if( Index < 0 || Index >= Frame->BaseData->MeshNum )
		return -1 ;
	Mesh = &Frame->BaseData->Mesh[ Index ] ;

	// メッシュのインデックスを返す
	return ( int )( Mesh - ModelBase->Mesh ) ;
}

// 指定のフレームの表示、非表示を変更する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1SetFrameVisible( int MHandle, int FrameIndex, int VisibleFlag )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialVisible( &Frame->DrawMaterial, &Frame->DrawMaterialChange, ( BYTE )VisibleFlag ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定のフレームの表示、非表示状態を取得する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1GetFrameVisible( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// フラグを返す
	return Frame->DrawMaterial.Visible ;
}


// 指定のフレームのディフューズカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetFrameDifColorScale( int MHandle, int FrameIndex, COLOR_F Scale )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialDif( &Frame->DrawMaterial, &Frame->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 半透明要素有無情報のセットアップ完了フラグを倒す
		MV1FRAME_RESET_SEMITRANSSETUP( Frame ) ;
	}

	// 終了
	return 0 ;
}

// 指定のフレームのスペキュラカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetFrameSpcColorScale( int MHandle, int FrameIndex, COLOR_F Scale )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialSpc( &Frame->DrawMaterial, &Frame->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定のフレームのエミッシブカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetFrameEmiColorScale( int MHandle, int FrameIndex, COLOR_F Scale )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialEmi( &Frame->DrawMaterial, &Frame->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定のフレームのアンビエントカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetFrameAmbColorScale( int MHandle, int FrameIndex, COLOR_F Scale )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialAmb( &Frame->DrawMaterial, &Frame->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定のフレームのディフューズカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetFrameDifColorScale( int MHandle, int FrameIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorResult ) ;

	// 値を返す
	return Frame->DrawMaterial.DiffuseScale ;
}

// 指定のフレームのスペキュラカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetFrameSpcColorScale( int MHandle, int FrameIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorResult ) ;

	// 値を返す
	return Frame->DrawMaterial.SpecularScale ;
}

// 指定のフレームのエミッシブカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetFrameEmiColorScale( int MHandle, int FrameIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorResult ) ;

	// 値を返す
	return Frame->DrawMaterial.EmissiveScale ;
}

// 指定のフレームのアンビエントカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetFrameAmbColorScale( int MHandle, int FrameIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, ErrorResult ) ;

	// 値を返す
	return Frame->DrawMaterial.AmbientScale ;
}

// 指定のフレームに半透明要素があるかどうかを取得する( 戻り値 TRUE:ある  FALSE:ない )
extern int NS_MV1GetFrameSemiTransState( int MHandle, int FrameIndex )
{
	int i, MeshIndex ;
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 描画マテリアルの更新
	MV1SETUPDRAWMATERIALFRAME( Frame ) ;

	// セットアップが完了していない場合はセットアップを行う
	if( Frame->SemiTransStateSetupFlag == false )
	{
		// 下位フレーム、メッシュの半透明要素があるかどうかを調べ、
		// どれか一つでも半透明要素があればこのフレームも半透明要素があるということになる
		Frame->SemiTransState = false ;

		// 最初にメッシュから
		MeshIndex = ( int )( Frame->Mesh - Model->Mesh ) ;
		for( i = 0 ; i < Frame->BaseData->MeshNum ; i ++, MeshIndex ++ )
		{
			if( NS_MV1GetMeshSemiTransState( MHandle, MeshIndex ) )
			{
				Frame->SemiTransState = true ;
			}
		}

		// 次に子フレーム
		for( i = 0 ; i < Frame->ChildNum ; i ++ )
		{
			if( NS_MV1GetFrameSemiTransState( MHandle, ( int )( Frame->ChildList[ i ] - Model->Frame ) ) )
				Frame->SemiTransState = true ;
		}

		// セットアップ完了フラグを立てる
		Frame->SemiTransStateSetupFlag = true ;
	}

	// 半透明要素チェックフラグを返す
	return Frame->SemiTransState ;
}


// 指定のフレームの不透明度を設定する( 不透明 1.0f ～ 透明 0.0f )
extern	int			NS_MV1SetFrameOpacityRate( int MHandle, int FrameIndex, float Rate )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialOpacityRate( &Frame->DrawMaterial, &Frame->DrawMaterialChange, Rate ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 半透明要素有無情報のセットアップ完了フラグを倒す
		MV1FRAME_RESET_SEMITRANSSETUP( Frame ) ;
	}

	// 終了
	return 0 ;
}

// 指定のフレームの不透明度を取得する( 不透明 1.0f ～ 透明 0.0f )
extern	float		NS_MV1GetFrameOpacityRate( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1.0f ) ;

	// 不透明度を返す
	return Frame->DrawMaterial.OpacityRate ;
}

// 指定のフレームの初期表示状態を設定する
extern int NS_MV1SetFrameBaseVisible( int MHandle, int FrameIndex, int VisibleFlag )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 初期表示状態を設定する
	if( VisibleFlag )
	{
		if( Frame->BaseData->Flag & MV1_FRAMEFLAG_VISIBLE )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		Frame->BaseData->Flag |= MV1_FRAMEFLAG_VISIBLE ;
	}
	else
	{
		if( ( Frame->BaseData->Flag & MV1_FRAMEFLAG_VISIBLE ) == 0 )
		{
			return 0 ;
		}

		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		Frame->BaseData->Flag &= ~MV1_FRAMEFLAG_VISIBLE ;
	}

	// 終了
	return 0 ;
}

// 指定のフレームの初期表示状態を取得する( 戻り値  TRUE:表示   FALSE:非表示 )
extern int NS_MV1GetFrameBaseVisible( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 初期表示状態を返す
	return ( Frame->BaseData->Flag & MV1_FRAMEFLAG_VISIBLE ) != 0 ? TRUE : FALSE ;
}

// 指定のフレームのテクスチャ座標変換パラメータを設定する
extern int NS_MV1SetFrameTextureAddressTransform( int MHandle, int FrameIndex, float TransU, float TransV, float ScaleU, float ScaleV, float RotCenterU, float RotCenterV, float Rotate )
{
	MATRIX Transform, Temp1, Temp2, Temp3 ;
	int UseFlag ;

	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	UseFlag = FALSE ;

	if( Rotate != 0.0f )
	{
		CreateIdentityMatrix( &Temp1 ) ;
		Temp1.m[ 2 ][ 0 ] = -RotCenterU ;
		Temp1.m[ 2 ][ 1 ] = -RotCenterV ;
//		CreateTranslationMatrix( &Temp1, -RotCenterU, -RotCenterV, 0.0f ) ; 
		CreateRotationZMatrix( &Temp2, Rotate ) ;
//		CreateTranslationMatrix( &Temp3,  RotCenterU,  RotCenterV, 0.0f ) ; 
		CreateIdentityMatrix( &Temp3 ) ;
		Temp3.m[ 2 ][ 0 ] = RotCenterU ;
		Temp3.m[ 2 ][ 1 ] = RotCenterV ;
		CreateMultiplyMatrix( &Transform, &Temp1, &Temp2 ) ;
		CreateMultiplyMatrix( &Transform, &Transform, &Temp3 ) ;
		UseFlag = TRUE ;
	}
	else
	{
		CreateIdentityMatrix( &Transform ) ;
	}

	if( TransU != 0.0f || TransV != 0.0f )
	{
//		CreateTranslationMatrix( &Temp1, TransU, TransV, 0.0f ) ;
		CreateIdentityMatrix( &Temp1 ) ;
		Temp1.m[ 2 ][ 0 ] = TransU ;
		Temp1.m[ 2 ][ 1 ] = TransV ;
		CreateMultiplyMatrix( &Transform, &Transform, &Temp1 ) ;
		UseFlag = TRUE ;
	}

	if( ScaleU != 1.0f || ScaleV != 1.0f )
	{
		CreateScalingMatrix( &Temp1, ScaleU, ScaleV, 1.0f ) ;
		CreateMultiplyMatrix( &Transform, &Transform, &Temp1 ) ;
		UseFlag = TRUE ;
	}

	if( UseFlag || Frame->TextureAddressTransformUse )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	Frame->TextureAddressTransformUse = UseFlag ;
	Frame->TextureAddressTransformMatrix = Transform ;

	// 終了
	return 0 ;
}

// 指定のフレームのテクスチャ座標変換行列をセットする
extern int NS_MV1SetFrameTextureAddressTransformMatrix( int MHandle, int FrameIndex, MATRIX Matrix )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	Frame->TextureAddressTransformUse = TRUE ;
	Frame->TextureAddressTransformMatrix = Matrix ;

	// 終了
	return 0 ;
}

// 指定のフレームのテクスチャ座標変換パラメータをリセットする
extern int NS_MV1ResetFrameTextureAddressTransform( int MHandle, int FrameIndex )
{
	MV1FRAMESTART( MHandle, Model, ModelBase, Frame, FrameIndex, -1 ) ;

	if( Frame->TextureAddressTransformUse )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	Frame->TextureAddressTransformUse = FALSE ;
	CreateIdentityMatrix( &Frame->TextureAddressTransformMatrix ) ;

	// 終了
	return 0 ;
}

// モデルに含まれるメッシュの数を取得する
extern int NS_MV1GetMeshNum( int MHandle )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false )
		return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// メッシュの数を返す
	return ModelBase->MeshNum ;
}




// 指定メッシュが使用しているマテリアルのインデックスを取得する
extern int NS_MV1GetMeshMaterial( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 使用しているマテリアルのインデックスを返す
	return ( int )( Mesh->BaseData->Material - ModelBase->Material ) ;
}

// 指定メッシュに含まれる三角形ポリゴンの数を取得する
extern int NS_MV1GetMeshTriangleNum( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;
	MV1_MESH_BASE *MeshBase ;
	int i, Num ;

	// 含まれる三角形ポリゴンの数を返す
	MeshBase = Mesh->BaseData ;
	Num = 0 ;
	for( i = 0 ; i < MeshBase->TriangleListNum ; i ++ )
		Num += MeshBase->TriangleList[ i ].IndexNum / 3 ;
	return Num ;
}

// 指定メッシュの表示、非表示状態を変更する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1SetMeshVisible( int MHandle, int MeshIndex, int VisibleFlag )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialVisible( &Mesh->DrawMaterial, &Mesh->DrawMaterialChange, ( BYTE )VisibleFlag ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定メッシュの表示、非表示状態を取得する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1GetMeshVisible( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// フラグを返す
	return Mesh->DrawMaterial.Visible ;
}

// 指定のメッシュのディフューズカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetMeshDifColorScale( int MHandle, int MeshIndex, COLOR_F Scale )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialDif( &Mesh->DrawMaterial, &Mesh->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 半透明ステータスセットアップ完了フラグが足っていたら倒す
		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
	}

	// 終了
	return 0 ;
}

// 指定のメッシュのスペキュラカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetMeshSpcColorScale( int MHandle, int MeshIndex, COLOR_F Scale )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialSpc( &Mesh->DrawMaterial, &Mesh->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定のメッシュのエミッシブカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetMeshEmiColorScale( int MHandle, int MeshIndex, COLOR_F Scale )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialEmi( &Mesh->DrawMaterial, &Mesh->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定のメッシュのアンビエントカラーのスケール値を設定する( デフォルト値は 1.0f )
extern	int			NS_MV1SetMeshAmbColorScale( int MHandle, int MeshIndex, COLOR_F Scale )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialAmb( &Mesh->DrawMaterial, &Mesh->DrawMaterialChange, Scale ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO
	}

	// 終了
	return 0 ;
}

// 指定のメッシュのディフューズカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetMeshDifColorScale( int MHandle, int MeshIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, ErrorResult ) ;

	// 値を返す
	return Mesh->DrawMaterial.DiffuseScale ;
}

// 指定のメッシュのスペキュラカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetMeshSpcColorScale( int MHandle, int MeshIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, ErrorResult ) ;

	// 値を返す
	return Mesh->DrawMaterial.SpecularScale ;
}

// 指定のメッシュのエミッシブカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetMeshEmiColorScale( int MHandle, int MeshIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, ErrorResult ) ;

	// 値を返す
	return Mesh->DrawMaterial.EmissiveScale ;
}

// 指定のメッシュのアンビエントカラーのスケール値を取得する( デフォルト値は 1.0f )
extern	COLOR_F		NS_MV1GetMeshAmbColorScale( int MHandle, int MeshIndex )
{
	COLOR_F ErrorResult = { -1.0f, -1.0f, -1.0f, -1.0f } ;
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, ErrorResult ) ;

	// 値を返す
	return Mesh->DrawMaterial.AmbientScale ;
}

// 指定のメッシュの不透明度を設定する( 不透明 1.0f ～ 透明 0.0f )
extern	int			NS_MV1SetMeshOpacityRate( int MHandle, int MeshIndex, float Rate )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 値をセット
	if( MV1SetDrawMaterialOpacityRate( &Mesh->DrawMaterial, &Mesh->DrawMaterialChange, Rate ) )
	{
		// 描画待機している描画物を描画
		DRAWSTOCKINFO

		// 半透明ステータスセットアップ完了フラグが足っていたら倒す
		MV1MESH_RESET_SEMITRANSSETUP( Mesh )
	}

	// 終了
	return 0 ;
}


// 指定のメッシュの不透明度を取得する( 不透明 1.0f ～ 透明 0.0f )
extern	float		NS_MV1GetMeshOpacityRate( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1.0f ) ;

	// 不透明度を返す
	return Mesh->DrawMaterial.OpacityRate ;
}

// 指定のメッシュの描画ブレンドモードを設定する( DX_BLENDMODE_ALPHA 等 )
extern	int			NS_MV1SetMeshDrawBlendMode( int MHandle, int MeshIndex, int BlendMode )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 今までとフラグが同じだったら何もせずに終了
	if( Mesh->DrawBlendMode == BlendMode )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// ブレンドモードを設定する
	Mesh->DrawBlendMode = BlendMode ;

	// 半透明ステータスセットアップ完了フラグが足っていたら倒す
	MV1MESH_RESET_SEMITRANSSETUP( Mesh )

	// マテリアル情報も更新
	if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
		MV1BitSetChange( &Mesh->DrawMaterialChange ) ;

	// 終了
	return 0 ;
}

// 指定のメッシュの描画ブレンドパラメータを設定する
extern int NS_MV1SetMeshDrawBlendParam( int MHandle, int MeshIndex, int BlendParam )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// 今までとパラメータが同じ場合は何もせずに終了
	if( Mesh->DrawBlendParam == BlendParam )
		return 0 ;

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// ブレンドパラメータを設定する
	Mesh->DrawBlendParam = BlendParam ;

	// 半透明ステータスセットアップ完了フラグが足っていたら倒す
	MV1MESH_RESET_SEMITRANSSETUP( Mesh )

	// マテリアル情報も更新
	if( MV1CCHK( Mesh->DrawMaterialChange ) == 0 )
		MV1BitSetChange( &Mesh->DrawMaterialChange ) ;

	// 終了
	return 0 ;
}

// 指定のメッシュの描画ブレンドモードを取得する( DX_BLENDMODE_ALPHA 等 )
extern int NS_MV1GetMeshDrawBlendMode( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// ブレンドモードを返す
	return Mesh->DrawBlendMode ;
}

// 指定のメッシュの描画ブレンドパラメータを設定する
extern int NS_MV1GetMeshDrawBlendParam( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// ブレンドパラメータを返す
	return Mesh->DrawBlendParam ;
}

// 指定のメッシュの初期表示状態を設定する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1SetMeshBaseVisible( int MHandle, int MeshIndex, int VisibleFlag )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	VisibleFlag = VisibleFlag != 0 ? 1 : 0 ;

	if( VisibleFlag == Mesh->BaseData->Visible )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// パラメータを設定する
	Mesh->BaseData->Visible = ( BYTE )VisibleFlag ;

	// 終了
	return 0 ;
}

// 指定のメッシュの初期表示状態を取得する( TRUE:表示  FALSE:非表示 )
extern int NS_MV1GetMeshBaseVisible( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// パラメータを返す
	return Mesh->BaseData->Visible ;
}

// 指定のメッシュのバックカリングを行うかどうかを設定する( DX_CULLING_LEFT 等 )
extern int NS_MV1SetMeshBackCulling( int MHandle, int MeshIndex, int CullingFlag )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	if( Mesh->BaseData->BackCulling == CullingFlag )
	{
		return 0 ;
	}

	// 描画待機している描画物を描画
	DRAWSTOCKINFO

	// パラメータを設定する
	Mesh->BaseData->BackCulling = ( BYTE )CullingFlag ;

	// 終了
	return 0 ;
}

// 指定のメッシュのバックカリングを行うかどうかを取得する( DX_CULLING_LEFT 等 )
extern int NS_MV1GetMeshBackCulling( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// パラメータを返す
	return Mesh->BaseData->BackCulling ;
}

// 指定のメッシュに含まれるポリゴンの最大ローカル座標を取得する
extern VECTOR NS_MV1GetMeshMaxPosition( int MHandle, int MeshIndex )
{
	VECTOR ErrorValue = { -1.0f, -1.0f, -1.0f } ;
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, ErrorValue ) ;

	// パラメータを返す
	return Mesh->BaseData->MaxPosition ;
}

// 指定のメッシュに含まれるポリゴンの最小ローカル座標を取得する
extern VECTOR NS_MV1GetMeshMinPosition( int MHandle, int MeshIndex )
{
	VECTOR ErrorValue = { -1.0f, -1.0f, -1.0f } ;
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, ErrorValue ) ;

	// パラメータを返す
	return Mesh->BaseData->MinPosition ;
}

// 指定のメッシュに含まれるトライアングルリストの数を取得する
extern int NS_MV1GetMeshTListNum( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// トライアングルリストの数を返す
	return Mesh->BaseData->TriangleListNum ;
}

// 指定のメッシュが半透明要素があるかどうかを取得する( 戻り値 TRUE:ある  FALSE:ない )
extern int NS_MV1GetMeshSemiTransState( int MHandle, int MeshIndex )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;
	MV1_MATERIAL *Material ;
	MV1_TEXTURE *Texture ;
	MV1_TEXTURE_BASE *TextureBase ;

	Material = Mesh->Material ;

	// 描画マテリアルの更新チェック
	MV1SETUPDRAWMATERIALMESH( Mesh ) ;

	// SemiTransStateSetupFlag が立ってい無い場合は半透明要素があるかどうかを調べる
	if( Mesh->SemiTransStateSetupFlag == false )
	{
		do
		{
			Mesh->SemiTransState = true ;

			// 描画モードがアルファブレンド以外なら半透明
			if( Mesh->DrawBlendMode != -1 )
			{
				if( Mesh->DrawBlendMode != DX_BLENDMODE_ALPHA )
					break ;
			}
			else
			{
				if( Material->DrawBlendMode != DX_BLENDMODE_ALPHA )
					break ;
			}

			// ブレンドパラメータが 255 以外なら半透明
			if( Mesh->DrawBlendParam != -1 )
			{
				if( Mesh->DrawBlendParam != 255 )
					break ;
			}
			else
			{
				if( Material->DrawBlendParam != 255 )
					break ;
			}

			// 頂点カラーを使用するかどうかで分岐
			if( Mesh->BaseData->UseVertexDiffuseColor )
			{
				// 頂点ディフューズカラーに 1.0 以外のアルファ値があったら半透明
				if( Mesh->BaseData->NotOneDiffuseAlpha )
					break ;
			}
			else
			{
				// マテリアルのディフューズカラーのアルファ値が１．０以外なら半透明
				if( Material->Diffuse.a != 1.0f || ( Mesh->SetupDrawMaterial.UseColorScale && Mesh->SetupDrawMaterial.DiffuseScale.a != 1.0f ) )
					break ;
			}

			// メッシュの不透明度設定が 1.0f 以外なら半透明
			if( Mesh->SetupDrawMaterial.OpacityRate != 1.0f )
				break ;

			// 半透明テクスチャを使用していたら半透明
			if( Material->DiffuseLayerNum )
			{
				Texture = &Model->Texture[ Material->DiffuseLayer[ 0 ].Texture ] ;
				TextureBase = Texture->BaseData ;

				if( Texture->UseUserGraphHandle )
				{
					if( Texture->UserGraphHandleSemiTransFlag )
						break ;
				}
				else
				if( Texture->UseGraphHandle )
				{
					if( Texture->SemiTransFlag )
						break ;
				}
				else
				if( TextureBase->UseUserGraphHandle )
				{
					if( TextureBase->UserGraphHandleSemiTransFlag )
						break ;
				}
				else
				if( TextureBase->SemiTransFlag )
				{
					break ;
				}
			}

			// ここにきたら半透明要素はないということ
			Mesh->SemiTransState = false ;
		}while( 0 ) ;

		// セットアップ完了フラグを立てる
		Mesh->SemiTransStateSetupFlag = true ;
	}

	// ステータスを返す
	return Mesh->SemiTransState ;
}

// 指定のメッシュに含まれるトライアングルリストのインデックスを取得する
extern int NS_MV1GetMeshTList( int MHandle, int MeshIndex, int Index )
{
	MV1MESHSTART( MHandle, Model, ModelBase, Mesh, MeshIndex, -1 ) ;

	// インデックスが不正だったらエラー
	if( Index < 0 || Index >= Mesh->BaseData->TriangleListNum )
		return -1 ;

	// トライアングルリストのインデックスを返す
	return ( int )( &Mesh->BaseData->TriangleList[ Index ] - ModelBase->TriangleList ) ;
}

// 指定のメッシュの頂点ディフューズカラーをマテリアルのディフューズカラーの代わりに使用するかどうかを設定する( TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int NS_MV1SetMeshUseVertDifColor( int MHandle, int MeshIndex, int UseFlag )
{
	return MV1SetMeshUseVertDifColorBase( MV1GetModelBaseHandle( MHandle ), MeshIndex, UseFlag ) ;
}

// 指定のメッシュの頂点スペキュラカラーをマテリアルのスペキュラカラーの代わりに使用するかどうかを設定する( TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int NS_MV1SetMeshUseVertSpcColor( int MHandle, int MeshIndex, int UseFlag )
{
	return MV1SetMeshUseVertSpcColorBase( MV1GetModelBaseHandle( MHandle ), MeshIndex, UseFlag ) ;
}

// 指定のメッシュの頂点ディフューズカラーをマテリアルのディフューズカラーの代わりに使用するかどうかの設定を取得する( 戻り値  TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int NS_MV1GetMeshUseVertDifColor( int MHandle, int MeshIndex )
{
	return MV1GetMeshUseVertDifColorBase( MV1GetModelBaseHandle( MHandle ), MeshIndex ) ;
}

// 指定のメッシュの頂点スペキュラカラーをマテリアルのスペキュラカラーの代わりに使用するかどうかの設定を取得する( 戻り値  TRUE:マテリアルカラーの代わりに使用する  FALSE:マテリアルカラーを使用する )
extern int NS_MV1GetMeshUseVertSpcColor( int MHandle, int MeshIndex )
{
	return MV1GetMeshUseVertSpcColorBase( MV1GetModelBaseHandle( MHandle ), MeshIndex ) ;
}

// 指定のメッシュがシェイプメッシュかどうかを取得する( 戻り値 TRUE:シェイプメッシュ  FALSE:通常メッシュ )
extern int NS_MV1GetMeshShapeFlag( int MHandle, int MeshIndex )
{
	return MV1GetMeshShapeFlagBase( MV1GetModelBaseHandle( MHandle ), MeshIndex ) ;
}























// シェイプ関係

// モデルに含まれるシェイプの数を取得する
extern int NS_MV1GetShapeNum( int MHandle )
{
	return MV1GetShapeNumBase( MV1GetModelBaseHandle( MHandle ) ) ;
}

// シェイプの名前からモデル中のシェイプのシェイプインデックスを取得する( 無かった場合は戻り値が-1 )
extern int NS_MV1SearchShape( int MHandle, const TCHAR *ShapeName )
{
#ifdef UNICODE
	return MV1SearchShape_WCHAR_T(
		MHandle, ShapeName
	) ;
#else
	int Result ;

	TCHAR_TO_WCHAR_T_STRING_ONE_BEGIN( ShapeName, return -1 )

	Result = MV1SearchShape_WCHAR_T(
		MHandle, UseShapeNameBuffer
	) ;

	TCHAR_TO_WCHAR_T_STRING_END( ShapeName )

	return Result ;
#endif
}

// シェイプの名前からモデル中のシェイプのシェイプインデックスを取得する( 無かった場合は戻り値が-1 )
extern int MV1SearchShape_WCHAR_T( int MHandle, const wchar_t *ShapeName )
{
	return MV1SearchShapeBase( MV1GetModelBaseHandle( MHandle ), ShapeName ) ;
}

// 指定シェイプの名前を取得する
extern const TCHAR *NS_MV1GetShapeName( int MHandle, int ShapeIndex )
{
#ifdef UNICODE
	return MV1GetShapeNameBaseW( MV1GetModelBaseHandle( MHandle ), ShapeIndex ) ;
#else
	return MV1GetShapeNameBaseA( MV1GetModelBaseHandle( MHandle ), ShapeIndex ) ;
#endif
}

// 指定シェイプの名前を取得する
extern const wchar_t *MV1GetShapeName_WCHAR_T( int MHandle, int ShapeIndex )
{
	return MV1GetShapeNameBaseW( MV1GetModelBaseHandle( MHandle ), ShapeIndex ) ;
}

// 指定シェイプが対象としているメッシュの数を取得する
extern int NS_MV1GetShapeTargetMeshNum( int MHandle, int ShapeIndex )
{
	return MV1GetShapeTargetMeshNumBase( MV1GetModelBaseHandle( MHandle ), ShapeIndex ) ;
}

// 指定シェイプが対象としているメッシュのメッシュインデックスを取得する
extern int NS_MV1GetShapeTargetMesh( int MHandle, int ShapeIndex, int Index )
{
	return MV1GetShapeTargetMeshBase( MV1GetModelBaseHandle( MHandle ), ShapeIndex, Index ) ;
}

// 指定シェイプの有効率を設定する( Rate  0.0f:0% ～ 1.0f:100% )
extern int NS_MV1SetShapeRate( int MHandle, int ShapeIndex, float Rate )
{
	MV1SHAPESTART( MHandle, Model, ModelBase, Shape, ShapeIndex, -1 ) ;

	// ほぼ値が変化しない場合は何もしない
	if( _FABS( Shape->Rate - Rate ) < 0.0000001f )
	{
		return 0 ;
	}

	// 有効率を保存
	Shape->Rate = Rate ;

	// シェイプの状態が変化したフラグを立てる
	Shape->Container->ShapeChangeFlag = true ;
	Shape->Container->Container->ShapeChangeFlag = true ;

	// 正常終了
	return 0 ;
}

// 指定シェイプの有効率を取得する( 戻り値  0.0f:0% ～ 1.0f:100% )
extern float NS_MV1GetShapeRate( int MHandle, int ShapeIndex )
{
	MV1SHAPESTART( MHandle, Model, ModelBase, Shape, ShapeIndex, -1.0f ) ;

	// 有効率を返す
	return Shape->Rate ;
}
























// トライアングルリストの数を取得する
extern int NS_MV1GetTriangleListNum( int MHandle )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;

	// 初期化されていなかったらエラー
	if( MV1Man.Initialize == false ) return -1 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// マテリアルの数を返す
	return ModelBase->TriangleListNum ;
}

// 指定のトライアングルリストの頂点データタイプを取得する( MV1_VERTEX_TYPE_NORMAL 等 )
extern int NS_MV1GetTriangleListVertexType( int MHandle, int TListIndex )
{
	int VertexType ;

	MV1TLISTSTART( MHandle, Model, ModelBase, TList, TListIndex, -1 ) ;

	// 頂点データタイプを返す
	VertexType = TList->VertexType ;
	if( TList->Container->Material->NormalLayerNum != 0 )
	{
		VertexType += DX_MV1_VERTEX_TYPE_NMAP_1FRAME ;
	}

	return VertexType ;
}

// 指定のトライアングルリストに含まれるポリゴンの数を取得する
extern int NS_MV1GetTriangleListPolygonNum( int MHandle, int TListIndex )
{
	MV1TLISTSTART( MHandle, Model, ModelBase, TList, TListIndex, -1 ) ;

	// ポリゴンの数を返す
	return TList->IndexNum / 3 ;
}

// 指定のトライアングルリストに含まれる頂点データの数を取得する
extern int NS_MV1GetTriangleListVertexNum( int MHandle, int TListIndex )
{
	MV1TLISTSTART( MHandle, Model, ModelBase, TList, TListIndex, -1 ) ;

	// 頂点データの数を返す
	return TList->VertexNum ;
}

// 指定のトライアングルリストが使用しているボーンフレームの数を取得する
extern int MV1GetTriangleListUseBoneFrameNum( int MHandle, int TListIndex )
{
	MV1TLISTSTART( MHandle, Model, ModelBase, TList, TListIndex, -1 ) ;

	// 使用しているボーンの数を返す
	return TList->UseBoneNum ;
}

// 指定のトライアングルリストが使用しているボーンフレームのフレームインデックスを取得する
extern int MV1GetTriangleListUseBoneFrame( int MHandle, int TListIndex, int Index )
{
	int i, con ;
	MV1_FRAME_BASE *FrameBase ;

	MV1TLISTSTART( MHandle, Model, ModelBase, TList, TListIndex, -1 ) ;

	FrameBase = TList->Container->Container ;

	// 指定のボーンとして使用しているフレームを返す
	con = 0 ;
	for( i = 0 ; i < MV1_TRIANGLE_LIST_USE_BONE_MAX_NUM ; i ++ )
	{
		if( TList->UseBone[ i ] == -1 ) continue ;

		if( con == Index )
			return FrameBase->UseSkinBone[ TList->UseBone[ i ] ]->BoneFrame ;
		con ++ ;
	}

	// ここにきたらエラー
	return -1 ;
}






// コリジョン情報を構築する
extern int NS_MV1SetupCollInfo( int MHandle, int FrameIndex, int XDivNum, int YDivNum, int ZDivNum )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// 参照用ポリゴン情報がない場合は構築する
		if( Model->RefPolygon[ 1 ][ 1 ] == NULL )
			if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, TRUE, TRUE ) < 0 )
				return -1 ;

		// 確保されていない場合のみ確保
		if( Model->Collision == NULL )
		{
			Model->Collision = ( MV1_COLLISION * )DXALLOC( sizeof( MV1_COLLISION ) + sizeof( MV1_COLL_POLYGON * ) * XDivNum * YDivNum * ZDivNum + sizeof( MV1_COLL_POLY_BUFFER ) + sizeof( MV1_COLL_POLYGON ) * ( Model->RefPolygon[ 1 ][ 1 ]->PolygonNum * 2 ) ) ;
			if( Model->Collision == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\xb3\x30\xea\x30\xb8\x30\xe7\x30\xf3\x30\xc5\x60\x31\x58\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"コリジョン情報を格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
				return -1 ;
			}
			Model->Collision->Polygon = ( MV1_COLL_POLYGON ** )( Model->Collision + 1 ) ;
			Model->Collision->LastBuffer = Model->Collision->FirstBuffer = ( MV1_COLL_POLY_BUFFER * )( Model->Collision->Polygon + XDivNum * YDivNum * ZDivNum ) ;
			Model->Collision->FirstBuffer->BufferSize = Model->RefPolygon[ 1 ][ 1 ]->PolygonNum * 2 ;
			Model->Collision->FirstBuffer->UseSize = 0 ;
			Model->Collision->FirstBuffer->Next = NULL ;
			Model->Collision->FirstBuffer->Buffer = ( MV1_COLL_POLYGON * )( Model->Collision->FirstBuffer + 1 ) ;
		}

		Model->Collision->XDivNum = XDivNum ;
		Model->Collision->YDivNum = YDivNum ;
		Model->Collision->ZDivNum = ZDivNum ;
	}
	else
	{
		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return -1 ;
		Frame = Model->Frame + FrameIndex ;

		// 参照用ポリゴン情報がない場合は構築する
		if( Frame->RefPolygon[ 1 ][ 1 ] == NULL )
			if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, TRUE, TRUE ) < 0 )
				return -1 ;

		// 確保されていない場合のみ確保
		if( Frame->Collision == NULL )
		{
			Frame->Collision = ( MV1_COLLISION * )DXALLOC( sizeof( MV1_COLLISION ) + sizeof( MV1_COLL_POLYGON * ) * XDivNum * YDivNum * ZDivNum + sizeof( MV1_COLL_POLY_BUFFER ) + sizeof( MV1_COLL_POLYGON ) * ( Frame->RefPolygon[ 1 ][ 1 ]->PolygonNum * 2 ) ) ;
			if( Frame->Collision == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\xb3\x30\xea\x30\xb8\x30\xe7\x30\xf3\x30\xc5\x60\x31\x58\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"コリジョン情報を格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
				return -1 ;
			}
			Frame->Collision->Polygon = ( MV1_COLL_POLYGON ** )( Frame->Collision + 1 ) ;
			Frame->Collision->LastBuffer = Frame->Collision->FirstBuffer = ( MV1_COLL_POLY_BUFFER * )( Frame->Collision->Polygon + XDivNum * YDivNum * ZDivNum ) ;
			Frame->Collision->FirstBuffer->BufferSize = Frame->RefPolygon[ 1 ][ 1 ]->PolygonNum * 2 ;
			Frame->Collision->FirstBuffer->UseSize = 0 ;
			Frame->Collision->FirstBuffer->Next = NULL ;
			Frame->Collision->FirstBuffer->Buffer = ( MV1_COLL_POLYGON * )( Frame->Collision->FirstBuffer + 1 ) ;
		}

		Frame->Collision->XDivNum = XDivNum ;
		Frame->Collision->YDivNum = YDivNum ;
		Frame->Collision->ZDivNum = ZDivNum ;
	}

	// コリジョン情報を構築しておく
	NS_MV1RefreshCollInfo( MHandle, FrameIndex ) ;

	// 終了
	return 0 ;
}

// コリジョン情報の後始末
extern int NS_MV1TerminateCollInfo( int MHandle, int FrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_COLL_POLY_BUFFER *Buffer, *NextBuffer ;
	MV1_COLLISION *Collision ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;
	Collision = NULL ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合
		Collision = Model->Collision ;
		Model->Collision = NULL ;
	}
	else
	{
		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return -1 ;
		Frame = Model->Frame + FrameIndex ;

		// フレーム単体の場合
		Collision = Frame->Collision ;
		Frame->Collision = NULL ;
	}

	// コリジョン情報があったら解放
	if( Collision )
	{
		// バッファを開放
		for( Buffer = Collision->FirstBuffer->Next ; Buffer ; Buffer = NextBuffer )
		{
			NextBuffer = Buffer->Next ;
			DXFREE( Buffer ) ;
		}

		// 本体を解放
		DXFREE( Collision ) ;
	}

	// 終了
	return 0 ;
}

// コリジョン情報を更新する
extern int NS_MV1RefreshCollInfo( int MHandle, int FrameIndex )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_REF_POLYGONLIST *PolyList ;
	MV1_REF_POLYGON *Poly ;
	MV1_COLLISION *Collision ;
	MV1_COLL_POLYGON *ColPoly ;
	MV1_COLL_POLY_BUFFER *ColBuffer ;
	int i, xc, yc, zc, ZStep, YStep ;
	int MinX, MinY, MinZ, MaxX, MaxY, MaxZ, PostIndexZ, PostIndexY, PostIndexX ;
	VECTOR UnitDiv, MinAreaPos ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// コリジョン情報が無かったらセットアップする
		if( Model->Collision == NULL )
			if( NS_MV1SetupCollInfo( MHandle, FrameIndex ) < 0 )
				return -1 ;

		// 参照用メッシュの更新
		NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, TRUE, TRUE ) ;

		// コリジョン情報の更新が完了していたら何もしない
		if( Model->SetupCollision == true )
			return 0 ;

		// ポリゴン情報のセット
		PolyList = Model->RefPolygon[ 1 ][ 1 ] ;
		Collision = Model->Collision ;

		// コリジョン情報の更新完了フラグを立てる
		Model->SetupCollision = true ;
	}
	else
	{
		// フレーム単体の場合

		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return -1 ;
		Frame = Model->Frame + FrameIndex ;

		// コリジョン情報が無かったらセットアップする
		if( Frame->Collision == NULL )
			if( NS_MV1SetupCollInfo( MHandle, FrameIndex ) < 0 )
				return -1 ;

		// 参照用メッシュの更新
		NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, TRUE, TRUE ) ;

		// コリジョン情報の更新が完了していたら何もしない
		if( Frame->SetupCollision == true )
			return 0 ;

		// ポリゴン情報のセット
		PolyList = Frame->RefPolygon[ 1 ][ 1 ] ;
		Collision = Frame->Collision ;

		// コリジョン情報の更新完了フラグを立てる
		Frame->SetupCollision = true ;
	}

	// 最大値と最小値周りをセット
	Collision->MaxPosition = PolyList->MaxPosition ;
	Collision->MinPosition = PolyList->MinPosition ;
	VectorSub( &Collision->Size, &Collision->MaxPosition, &Collision->MinPosition ) ;
	Collision->UnitSize.x = Collision->Size.x / ( float )Collision->XDivNum ;
	Collision->UnitSize.y = Collision->Size.y / ( float )Collision->YDivNum ;
	Collision->UnitSize.z = Collision->Size.z / ( float )Collision->ZDivNum ;
	if( Collision->UnitSize.x < 0.00001f ) Collision->UnitSize.x = 1.0f ;
	if( Collision->UnitSize.y < 0.00001f ) Collision->UnitSize.y = 1.0f ;
	if( Collision->UnitSize.z < 0.00001f ) Collision->UnitSize.z = 1.0f ;
	Collision->UnitSizeRev.x = 1.0f / Collision->UnitSize.x ;
	Collision->UnitSizeRev.y = 1.0f / Collision->UnitSize.y ;
	Collision->UnitSizeRev.z = 1.0f / Collision->UnitSize.z ;

	// テーブルを初期化
	_MEMSET( Collision->Polygon, 0, sizeof( MV1_COLL_POLYGON * ) * Collision->XDivNum * Collision->YDivNum * Collision->ZDivNum ) ;

	// バッファを初期化
	Collision->FirstBuffer->UseSize = 0 ;

	// コリジョン情報構築
	Poly = PolyList->Polygons ;
	ColBuffer = Collision->FirstBuffer ;
	UnitDiv = Collision->UnitSizeRev ;
	MinAreaPos = PolyList->MinPosition ;
	ZStep = Collision->YDivNum * Collision->XDivNum ;
	YStep = Collision->XDivNum ;
	for( i = 0 ; i < PolyList->PolygonNum ; i ++, Poly ++ )
	{
		// 範囲を整数値に変換
		MinX = __FTOL( ( Poly->MinPosition.x - MinAreaPos.x ) * UnitDiv.x ) ;
		MinY = __FTOL( ( Poly->MinPosition.y - MinAreaPos.y ) * UnitDiv.y ) ;
		MinZ = __FTOL( ( Poly->MinPosition.z - MinAreaPos.z ) * UnitDiv.z ) ;
		MaxX = __FTOL( ( Poly->MaxPosition.x - MinAreaPos.x ) * UnitDiv.x ) ;
		MaxY = __FTOL( ( Poly->MaxPosition.y - MinAreaPos.y ) * UnitDiv.y ) ;
		MaxZ = __FTOL( ( Poly->MaxPosition.z - MinAreaPos.z ) * UnitDiv.z ) ;
		if( MaxX == Collision->XDivNum ) MaxX = Collision->XDivNum - 1 ;
		if( MaxY == Collision->YDivNum ) MaxY = Collision->YDivNum - 1 ;
		if( MaxZ == Collision->ZDivNum ) MaxZ = Collision->ZDivNum - 1 ;
		if( MinX == Collision->XDivNum ) MinX = Collision->XDivNum - 1 ;
		if( MinY == Collision->YDivNum ) MinY = Collision->YDivNum - 1 ;
		if( MinZ == Collision->ZDivNum ) MinZ = Collision->ZDivNum - 1 ;

		// ポリゴンをリストに追加
		PostIndexZ = MinX + MinY * YStep + MinZ * ZStep ;
		for( zc = MinZ ; zc <= MaxZ ; zc ++, PostIndexZ += ZStep )
		{
			PostIndexY = PostIndexZ ;
			for( yc = MinY ; yc <= MaxY ; yc ++, PostIndexY += YStep )
			{
				PostIndexX = PostIndexY ;
				for( xc = MinX ; xc <= MaxX ; xc ++, PostIndexX ++ )
				{
					// 追加処理に使うポリゴン情報の確保
					if( ColBuffer->UseSize == ColBuffer->BufferSize )
					{
						// 次のバッファが無かったら新しいメモリ領域を確保
						if( ColBuffer->Next == NULL )
						{
							ColBuffer->Next = ( MV1_COLL_POLY_BUFFER * )DXALLOC( sizeof( MV1_COLL_POLY_BUFFER ) + sizeof( MV1_COLL_POLYGON ) * PolyList->PolygonNum ) ;
							if( ColBuffer->Next == NULL )
							{
								DXST_ERRORLOGFMT_ADDUTF16LE(( "\xb3\x30\xea\x30\xb8\x30\xe7\x30\xf3\x30\xfd\x8f\xa0\x52\xc5\x60\x31\x58\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"コリジョン追加情報を格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
								return -1 ;
							}
							ColBuffer->Next->BufferSize = PolyList->PolygonNum ;
							ColBuffer->Next->UseSize = 0 ;
							ColBuffer->Next->Buffer = ( MV1_COLL_POLYGON * )( ColBuffer->Next + 1 ) ;
							ColBuffer->Next->Next = NULL ;
							Collision->LastBuffer = ColBuffer->Next ;
						}
						ColBuffer = ColBuffer->Next ;
						ColBuffer->UseSize = 0 ;
					}
					ColPoly = &ColBuffer->Buffer[ ColBuffer->UseSize ] ;
					ColBuffer->UseSize ++ ;

					// コリジョンポリゴン情報をセット
					ColPoly->Polygon = Poly ;
					ColPoly->Next = Collision->Polygon[ PostIndexX ] ;
					Collision->Polygon[ PostIndexX ] = ColPoly ;
				}
			}
		}
	}

	// 終了
	return 0 ;
}

// 線とモデルの当たり判定
extern MV1_COLL_RESULT_POLY NS_MV1CollCheck_Line( int MHandle, int FrameIndex, VECTOR PosStart, VECTOR PosEnd )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_REF_POLYGONLIST *PolyList ;
	MV1_REF_POLYGON *Poly, *MinPoly ;
	MV1_COLLISION *Collision ;
	MV1_COLL_POLYGON *ColPoly ;
	int xc, yc, zc, ZStep, YStep, PolyIndex ;
	int MinX, MinY, MinZ, MaxX, MaxY, MaxZ, MinPolyIndex = 0, PostIndexZ, PostIndexY, PostIndexX ;
	VECTOR UnitDiv, MinPos, MaxPos, Sa, MinHitPos = { 0.0f } ;
	MV1_COLL_RESULT_POLY Result ;
	float MinLength = 0.0f, Length ;
	HITRESULT_LINE LineRes ;
	BYTE *BitBuffer = NULL ;

	// 戻り値を初期化
	_MEMSET( &Result, 0, sizeof( Result ) ) ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return Result ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// コリジョン情報が無かったらセットアップする
		if( Model->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Model->RefPolygon[ 1 ][ 1 ] ;
		Collision = Model->Collision ;
	}
	else
	{
		// フレーム単体の場合

		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return Result ;
		Frame = Model->Frame + FrameIndex ;

		// コリジョン情報が無かったらセットアップする
		if( Frame->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Frame->RefPolygon[ 1 ][ 1 ] ;
		Collision = Frame->Collision ;
	}

	// 指定座標の最小値と最大値をセット
	if( PosStart.x > PosEnd.x )
	{
		MaxPos.x = PosStart.x ;
		MinPos.x = PosEnd.x ;
	}
	else
	{
		MaxPos.x = PosEnd.x ;
		MinPos.x = PosStart.x ;
	}

	if( PosStart.y > PosEnd.y )
	{
		MaxPos.y = PosStart.y ;
		MinPos.y = PosEnd.y ;
	}
	else
	{
		MaxPos.y = PosEnd.y ;
		MinPos.y = PosStart.y ;
	}

	if( PosStart.z > PosEnd.z )
	{
		MaxPos.z = PosStart.z ;
		MinPos.z = PosEnd.z ;
	}
	else
	{
		MaxPos.z = PosEnd.z ;
		MinPos.z = PosStart.z ;
	}

	// 範囲がコリジョン空間から外れていたらどのポリゴンにも当たらないということ
	if( Collision->MaxPosition.x < MinPos.x ||
		Collision->MaxPosition.y < MinPos.y ||
		Collision->MaxPosition.z < MinPos.z ||
		Collision->MinPosition.x > MaxPos.x ||
		Collision->MinPosition.y > MaxPos.y ||
		Collision->MinPosition.z > MaxPos.z )
		goto END ;

	// 当たり判定を行ったかどうかを保存するメモリ領域の確保
	BitBuffer = ( BYTE * )DXALLOC( sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;
	if( BitBuffer == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x53\x5f\x5f\x30\x8a\x30\x24\x52\x9a\x5b\xe6\x51\x06\x74\x28\x75\xc6\x30\xf3\x30\xdd\x30\xe9\x30\xea\x30\xd0\x30\xc3\x30\xd5\x30\xa1\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"当たり判定処理用テンポラリバッファの確保に失敗しました\n" @*/ )) ;
		return Result ;
	}
	_MEMSET( BitBuffer, 0, sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;

	// 検索範囲を決定
	UnitDiv = Collision->UnitSizeRev ;
	MinX = __FTOL( ( MinPos.x - Collision->MinPosition.x ) * UnitDiv.x ) ;
	MinY = __FTOL( ( MinPos.y - Collision->MinPosition.y ) * UnitDiv.y ) ;
	MinZ = __FTOL( ( MinPos.z - Collision->MinPosition.z ) * UnitDiv.z ) ;
	MaxX = __FTOL( ( MaxPos.x - Collision->MinPosition.x ) * UnitDiv.x ) ;
	MaxY = __FTOL( ( MaxPos.y - Collision->MinPosition.y ) * UnitDiv.y ) ;
	MaxZ = __FTOL( ( MaxPos.z - Collision->MinPosition.z ) * UnitDiv.z ) ;
	if( MinX < 0 ) MinX = 0 ;
	if( MinY < 0 ) MinY = 0 ;
	if( MinZ < 0 ) MinZ = 0 ;
	if( MaxX >= Collision->XDivNum ) MaxX = Collision->XDivNum - 1 ;
	if( MaxY >= Collision->YDivNum ) MaxY = Collision->YDivNum - 1 ;
	if( MaxZ >= Collision->ZDivNum ) MaxZ = Collision->ZDivNum - 1 ;

	// 範囲内の全てのポリゴンと当たり判定
	ZStep = Collision->YDivNum * Collision->XDivNum ;
	YStep = Collision->XDivNum ;
	PostIndexZ = MinX + MinY * YStep + MinZ * ZStep ;
	MinPoly = NULL ;
	for( zc = MinZ ; zc <= MaxZ ; zc ++, PostIndexZ += ZStep )
	{
		PostIndexY = PostIndexZ ;
		for( yc = MinY ; yc <= MaxY ; yc ++, PostIndexY += YStep )
		{
			PostIndexX = PostIndexY ;
			for( xc = MinX ; xc <= MaxX ; xc ++, PostIndexX ++ )
			{
				for( ColPoly = Collision->Polygon[ PostIndexX ] ; ColPoly ; ColPoly = ColPoly->Next )
				{
					Poly = ColPoly->Polygon ;
					PolyIndex = ( int )( Poly - PolyList->Polygons ) ;

					// 既にチェック済みのポリゴンだった場合は何もしない
					if( BitBuffer[ PolyIndex >> 3 ] & ( 1 << ( PolyIndex & 7 ) ) ) continue ;

					// 最初に立方体単位の当たり判定を行う
					if( ( MinPos.x > Poly->MaxPosition.x ||
						  MinPos.y > Poly->MaxPosition.y ||
						  MinPos.z > Poly->MaxPosition.z ||
						  MaxPos.x < Poly->MinPosition.x ||
						  MaxPos.y < Poly->MinPosition.y ||
						  MaxPos.z < Poly->MinPosition.z ) == false )
					{
						// 三角形と線の当たり判定
						LineRes = HitCheck_Line_Triangle(
										*( ( VECTOR * )&PosStart ),
										*( ( VECTOR * )&PosEnd ),
										PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) ;
						if( LineRes.HitFlag )
						{
							VectorSub( &Sa, &LineRes.Position, ( VECTOR * )&PosStart ) ;
							Length = Sa.x * Sa.x + Sa.y * Sa.y + Sa.z * Sa.z ;
							if( MinPoly == NULL || Length < MinLength )
							{
								if( VSquareSize( VSub( PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position, PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position ) ) > 0.0000001f &&
									VSquareSize( VSub( PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position, PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) ) > 0.0000001f &&
									VSquareSize( VSub( PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position, PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) ) > 0.0000001f )
								{
									MinPoly = Poly ;
									MinLength = Length ;
									MinHitPos = LineRes.Position ;
									MinPolyIndex = PolyIndex ;
								}
							}
						}
					}

					// チェック済みフラグを立てる
					BitBuffer[ PolyIndex >> 3 ] |= 1 << ( PolyIndex & 7 ) ;
				}
			}
		}
	}

	// 結果を代入
	if( MinPoly )
	{
		Result.HitFlag = 1 ;
		Result.HitPosition = MinHitPos ;

		Result.FrameIndex = MinPoly->FrameIndex ;
		Result.MaterialIndex = MinPoly->MaterialIndex ;
		Result.PolygonIndex = MinPolyIndex ;
		Result.Position[ 0 ] = PolyList->Vertexs[ MinPoly->VIndex[ 0 ] ].Position ;
		Result.Position[ 1 ] = PolyList->Vertexs[ MinPoly->VIndex[ 1 ] ].Position ;
		Result.Position[ 2 ] = PolyList->Vertexs[ MinPoly->VIndex[ 2 ] ].Position ;
		Result.Normal = VNorm( VCross( VSub( Result.Position[ 1 ], Result.Position[ 0 ] ), VSub( Result.Position[ 2 ], Result.Position[ 0 ] ) ) ) ; 
	}

END :

	// メモリの解放
	if( BitBuffer )
	{
		DXFREE( BitBuffer ) ;
		BitBuffer = NULL ;
	}

	return Result ;
}

// 線とモデルの当たり判定( 戻り値が MV1_COLL_RESULT_POLY_DIM )
extern	MV1_COLL_RESULT_POLY_DIM NS_MV1CollCheck_LineDim( int MHandle, int FrameIndex, VECTOR PosStart, VECTOR PosEnd )
{
	int MaxNum ;
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_REF_POLYGONLIST *PolyList ;
	MV1_REF_POLYGON *Poly, *MinPoly ;
	MV1_COLLISION *Collision ;
	MV1_COLL_POLYGON *ColPoly ;
	int xc, yc, zc, ZStep, YStep, PolyIndex ;
	int MinX, MinY, MinZ, MaxX, MaxY, MaxZ, PostIndexZ, PostIndexY, PostIndexX ;
	VECTOR UnitDiv, MinPos, MaxPos ;
	MV1_COLL_RESULT_POLY_DIM Result ;
	MV1_COLL_RESULT_POLY *RPoly ;
	HITRESULT_LINE LineRes ;
	BYTE *BitBuffer = NULL ;

	// 結果初期化
	Result.HitNum = 0 ;
	Result.Dim = NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return Result ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// コリジョン情報が無かったらセットアップする
		if( Model->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Model->RefPolygon[ 1 ][ 1 ] ;
		Collision = Model->Collision ;
	}
	else
	{
		// フレーム単体の場合

		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return Result ;
		Frame = Model->Frame + FrameIndex ;

		// コリジョン情報が無かったらセットアップする
		if( Frame->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Frame->RefPolygon[ 1 ][ 1 ] ;
		Collision = Frame->Collision ;
	}

	// 指定座標の最小値と最大値をセット
	if( PosStart.x > PosEnd.x )
	{
		MaxPos.x = PosStart.x ;
		MinPos.x = PosEnd.x ;
	}
	else
	{
		MaxPos.x = PosEnd.x ;
		MinPos.x = PosStart.x ;
	}

	if( PosStart.y > PosEnd.y )
	{
		MaxPos.y = PosStart.y ;
		MinPos.y = PosEnd.y ;
	}
	else
	{
		MaxPos.y = PosEnd.y ;
		MinPos.y = PosStart.y ;
	}

	if( PosStart.z > PosEnd.z )
	{
		MaxPos.z = PosStart.z ;
		MinPos.z = PosEnd.z ;
	}
	else
	{
		MaxPos.z = PosEnd.z ;
		MinPos.z = PosStart.z ;
	}

	// 範囲がコリジョン空間から外れていたらどのポリゴンにも当たらないということ
	if( Collision->MaxPosition.x < MinPos.x ||
		Collision->MaxPosition.y < MinPos.y ||
		Collision->MaxPosition.z < MinPos.z ||
		Collision->MinPosition.x > MaxPos.x ||
		Collision->MinPosition.y > MaxPos.y ||
		Collision->MinPosition.z > MaxPos.z )
		goto END ;

	// 初期バッファを確保
	MaxNum = 1000 ;
	Result.Dim = ( MV1_COLL_RESULT_POLY * )DXALLOC( sizeof( MV1_COLL_RESULT_POLY ) * MaxNum ) ;
	if( Result.Dim == NULL ) return Result ;

	// 当たり判定を行ったかどうかを保存するメモリ領域の確保
	BitBuffer = ( BYTE * )DXALLOC( sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;
	if( BitBuffer == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x53\x5f\x5f\x30\x8a\x30\x24\x52\x9a\x5b\xe6\x51\x06\x74\x28\x75\xc6\x30\xf3\x30\xdd\x30\xe9\x30\xea\x30\xd0\x30\xc3\x30\xd5\x30\xa1\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"当たり判定処理用テンポラリバッファの確保に失敗しました\n" @*/ )) ;
		return Result ;
	}
	_MEMSET( BitBuffer, 0, sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;

	// 検索範囲を決定
	UnitDiv = Collision->UnitSizeRev ;
	MinX = __FTOL( ( MinPos.x - Collision->MinPosition.x ) * UnitDiv.x ) ;
	MinY = __FTOL( ( MinPos.y - Collision->MinPosition.y ) * UnitDiv.y ) ;
	MinZ = __FTOL( ( MinPos.z - Collision->MinPosition.z ) * UnitDiv.z ) ;
	MaxX = __FTOL( ( MaxPos.x - Collision->MinPosition.x ) * UnitDiv.x ) ;
	MaxY = __FTOL( ( MaxPos.y - Collision->MinPosition.y ) * UnitDiv.y ) ;
	MaxZ = __FTOL( ( MaxPos.z - Collision->MinPosition.z ) * UnitDiv.z ) ;
	if( MinX < 0 ) MinX = 0 ;
	if( MinY < 0 ) MinY = 0 ;
	if( MinZ < 0 ) MinZ = 0 ;
	if( MaxX >= Collision->XDivNum ) MaxX = Collision->XDivNum - 1 ;
	if( MaxY >= Collision->YDivNum ) MaxY = Collision->YDivNum - 1 ;
	if( MaxZ >= Collision->ZDivNum ) MaxZ = Collision->ZDivNum - 1 ;

	// 範囲内の全てのポリゴンと当たり判定
	ZStep = Collision->YDivNum * Collision->XDivNum ;
	YStep = Collision->XDivNum ;
	PostIndexZ = MinX + MinY * YStep + MinZ * ZStep ;
	MinPoly = NULL ;
	for( zc = MinZ ; zc <= MaxZ ; zc ++, PostIndexZ += ZStep )
	{
		PostIndexY = PostIndexZ ;
		for( yc = MinY ; yc <= MaxY ; yc ++, PostIndexY += YStep )
		{
			PostIndexX = PostIndexY ;
			for( xc = MinX ; xc <= MaxX ; xc ++, PostIndexX ++ )
			{
				for( ColPoly = Collision->Polygon[ PostIndexX ] ; ColPoly ; ColPoly = ColPoly->Next )
				{
					Poly = ColPoly->Polygon ;
					PolyIndex = ( int )( Poly - PolyList->Polygons ) ;

					// 既にチェック済みのポリゴンだった場合は何もしない
					if( BitBuffer[ PolyIndex >> 3 ] & ( 1 << ( PolyIndex & 7 ) ) ) continue ;

					// 最初に立方体単位の当たり判定を行う
					if( ( MinPos.x > Poly->MaxPosition.x ||
						  MinPos.y > Poly->MaxPosition.y ||
						  MinPos.z > Poly->MaxPosition.z ||
						  MaxPos.x < Poly->MinPosition.x ||
						  MaxPos.y < Poly->MinPosition.y ||
						  MaxPos.z < Poly->MinPosition.z ) == false )
					{
						// 三角形と線の当たり判定
						LineRes = HitCheck_Line_Triangle(
										*( ( VECTOR * )&PosStart ),
										*( ( VECTOR * )&PosEnd ),
										PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) ;
						if( LineRes.HitFlag )
						{
							// 数が最大数に達していたら再確保
							if( Result.HitNum == MaxNum )
							{
								MV1_COLL_RESULT_POLY *NewBuffer ;
								int NewMaxNum ;

								NewMaxNum = MaxNum * 2 ;
								NewBuffer = ( MV1_COLL_RESULT_POLY * )DXREALLOC( Result.Dim, sizeof( MV1_COLL_RESULT_POLY ) * NewMaxNum ) ;
								if( NewBuffer == NULL )
								{
									DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe1\x30\xe2\x30\xea\x30\x0d\x4e\xb3\x8d\x6e\x30\xba\x70\x53\x5f\x5f\x30\x63\x30\x5f\x30\xdd\x30\xea\x30\xb4\x30\xf3\x30\x59\x30\x79\x30\x66\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\x53\x30\x68\x30\x4c\x30\x67\x30\x4d\x30\x7e\x30\x5b\x30\x93\x30\x67\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"メモリ不足の為当たったポリゴンすべてを保存することができませんでした\n" @*/ )) ;
									goto END ;
								}

								// 新しいバッファの保存
								Result.Dim = NewBuffer ;
								MaxNum = NewMaxNum ;
							}

							// 当たっているポリゴンリストに追加する
							RPoly = &Result.Dim[ Result.HitNum ] ;
							Result.HitNum ++ ;

							RPoly->HitFlag = 1 ;
							RPoly->HitPosition = LineRes.Position ;

							RPoly->FrameIndex = Poly->FrameIndex ;
							RPoly->MaterialIndex = Poly->MaterialIndex ;
							RPoly->PolygonIndex = PolyIndex ;
							RPoly->Position[ 0 ] = PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position ;
							RPoly->Position[ 1 ] = PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position ;
							RPoly->Position[ 2 ] = PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ;
							RPoly->Normal = VNorm( VCross( VSub( RPoly->Position[ 1 ], RPoly->Position[ 0 ] ), VSub( RPoly->Position[ 2 ], RPoly->Position[ 0 ] ) ) ) ; 
						}
					}

					// チェック済みフラグを立てる
					BitBuffer[ PolyIndex >> 3 ] |= 1 << ( PolyIndex & 7 ) ;
				}
			}
		}
	}

	// 一つも当たっていなかったら当たっているポリゴンリスト用に確保したメモリを解放
	if( Result.HitNum == 0 )
	{
		if( Result.Dim != NULL )
		{
			DXFREE( Result.Dim ) ;
			Result.Dim = NULL ;
		}
	}

END :

	// メモリの解放
	if( BitBuffer )
	{
		DXFREE( BitBuffer ) ;
		BitBuffer = NULL ;
	}

	return Result ;
}

// 球とモデルの当たり判定
extern	MV1_COLL_RESULT_POLY_DIM NS_MV1CollCheck_Sphere( int MHandle, int FrameIndex, VECTOR CenterPos, float r )
{
	int MaxNum ;
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_COLL_RESULT_POLY_DIM Result ;
	MV1_COLL_RESULT_POLY *RPoly ;
	MV1_REF_POLYGONLIST *PolyList ;
	MV1_REF_POLYGON *Poly ;
	MV1_COLLISION *Collision ;
	MV1_COLL_POLYGON *ColPoly ;
	BYTE *BitBuffer = NULL ;
	VECTOR UnitDiv, MinPos, MaxPos, Sa, ResultPos ;
	int xc, yc, zc, ZStep, YStep, PolyIndex ;
	int MinX, MinY, MinZ, MaxX, MaxY, MaxZ, PostIndexZ, PostIndexY, PostIndexX ;

	// 結果初期化
	Result.HitNum = 0 ;
	Result.Dim = NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return Result ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// コリジョン情報が無かったらセットアップする
		if( Model->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Model->RefPolygon[ 1 ][ 1 ] ;
		Collision = Model->Collision ;
	}
	else
	{
		// フレーム単体の場合

		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return Result ;
		Frame = Model->Frame + FrameIndex ;

		// コリジョン情報が無かったらセットアップする
		if( Frame->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Frame->RefPolygon[ 1 ][ 1 ] ;
		Collision = Frame->Collision ;
	}

	// 座標値の最小値と最大値をセット
	MaxPos.x = CenterPos.x + r ;
	MaxPos.y = CenterPos.y + r ;
	MaxPos.z = CenterPos.z + r ;
	MinPos.x = CenterPos.x - r ;
	MinPos.y = CenterPos.y - r ;
	MinPos.z = CenterPos.z - r ;

	// 範囲がコリジョン空間から外れていたらどのポリゴンにも当たらないということ
	if( Collision->MaxPosition.x < MinPos.x ||
		Collision->MaxPosition.y < MinPos.y ||
		Collision->MaxPosition.z < MinPos.z ||
		Collision->MinPosition.x > MaxPos.x ||
		Collision->MinPosition.y > MaxPos.y ||
		Collision->MinPosition.z > MaxPos.z )
		goto END ;

	// 初期バッファを確保
	MaxNum = 1000 ;
	Result.Dim = ( MV1_COLL_RESULT_POLY * )DXALLOC( sizeof( MV1_COLL_RESULT_POLY ) * MaxNum ) ;
	if( Result.Dim == NULL ) return Result ;

	// 当たり判定を行ったかどうかを保存するメモリ領域の確保
	BitBuffer = ( BYTE * )DXALLOC( sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;
	if( BitBuffer == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x53\x5f\x5f\x30\x8a\x30\x24\x52\x9a\x5b\xe6\x51\x06\x74\x28\x75\xc6\x30\xf3\x30\xdd\x30\xe9\x30\xea\x30\xd0\x30\xc3\x30\xd5\x30\xa1\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x32\x00\x0a\x00\x00"/*@ L"当たり判定処理用テンポラリバッファの確保に失敗しました2\n" @*/ )) ;
		DXFREE( Result.Dim ) ;
		return Result ;
	}
	_MEMSET( BitBuffer, 0, sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;

	// 検索範囲を決定
	UnitDiv = Collision->UnitSizeRev ;
	MinX = __FTOL( ( MinPos.x - Collision->MinPosition.x ) * UnitDiv.x ) ;
	MinY = __FTOL( ( MinPos.y - Collision->MinPosition.y ) * UnitDiv.y ) ;
	MinZ = __FTOL( ( MinPos.z - Collision->MinPosition.z ) * UnitDiv.z ) ;
	MaxX = __FTOL( ( MaxPos.x - Collision->MinPosition.x ) * UnitDiv.x ) + 1 ;
	MaxY = __FTOL( ( MaxPos.y - Collision->MinPosition.y ) * UnitDiv.y ) + 1 ;
	MaxZ = __FTOL( ( MaxPos.z - Collision->MinPosition.z ) * UnitDiv.z ) + 1 ;
	if( MinX < 0 ) MinX = 0 ;
	if( MinY < 0 ) MinY = 0 ;
	if( MinZ < 0 ) MinZ = 0 ;
	if( MaxX >= Collision->XDivNum ) MaxX = Collision->XDivNum - 1 ;
	if( MaxY >= Collision->YDivNum ) MaxY = Collision->YDivNum - 1 ;
	if( MaxZ >= Collision->ZDivNum ) MaxZ = Collision->ZDivNum - 1 ;

	// 範囲内の全てのポリゴンと当たり判定
	ZStep = Collision->YDivNum * Collision->XDivNum ;
	YStep = Collision->XDivNum ;
	PostIndexZ = MinX + MinY * YStep + MinZ * ZStep ;
	for( zc = MinZ ; zc <= MaxZ ; zc ++, PostIndexZ += ZStep )
	{
		PostIndexY = PostIndexZ ;
		for( yc = MinY ; yc <= MaxY ; yc ++, PostIndexY += YStep )
		{
			PostIndexX = PostIndexY ;
			for( xc = MinX ; xc <= MaxX ; xc ++, PostIndexX ++ )
			{
				for( ColPoly = Collision->Polygon[ PostIndexX ] ; ColPoly ; ColPoly = ColPoly->Next )
				{
					Poly = ColPoly->Polygon ;
					PolyIndex = ( int )( Poly - PolyList->Polygons ) ;

					// 既にチェック済みのポリゴンだった場合は何もしない
					if( BitBuffer[ PolyIndex >> 3 ] & ( 1 << ( PolyIndex & 7 ) ) ) continue ;

					// 最初に立方体単位の当たり判定を行う
					if( ( MinPos.x > Poly->MaxPosition.x ||
						  MinPos.y > Poly->MaxPosition.y ||
						  MinPos.z > Poly->MaxPosition.z ||
						  MaxPos.x < Poly->MinPosition.x ||
						  MaxPos.y < Poly->MinPosition.y ||
						  MaxPos.z < Poly->MinPosition.z ) == false )
					{
						// 三角形と球の当たり判定

						// 球に一番近い三角形上の座標との距離が、球の半径以下だったら当たっているということ
						ResultPos = Get_Triangle_Point_MinPosition( CenterPos, 
										PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) ;
						Sa.x = CenterPos.x - ResultPos.x ;
						Sa.y = CenterPos.y - ResultPos.y ;
						Sa.z = CenterPos.z - ResultPos.z ;
						if( Sa.x * Sa.x + Sa.y * Sa.y + Sa.z * Sa.z <= r * r )
						{
							// 数が最大数に達していたら再確保
							if( Result.HitNum == MaxNum )
							{
								MV1_COLL_RESULT_POLY *NewBuffer ;
								int NewMaxNum ;

								NewMaxNum = MaxNum * 2 ;
								NewBuffer = ( MV1_COLL_RESULT_POLY * )DXREALLOC( Result.Dim, sizeof( MV1_COLL_RESULT_POLY ) * NewMaxNum ) ;
								if( NewBuffer == NULL )
								{
									DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe1\x30\xe2\x30\xea\x30\x0d\x4e\xb3\x8d\x6e\x30\xba\x70\x53\x5f\x5f\x30\x63\x30\x5f\x30\xdd\x30\xea\x30\xb4\x30\xf3\x30\x59\x30\x79\x30\x66\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\x53\x30\x68\x30\x4c\x30\x67\x30\x4d\x30\x7e\x30\x5b\x30\x93\x30\x67\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"メモリ不足の為当たったポリゴンすべてを保存することができませんでした\n" @*/ )) ;
									goto END ;
								}

								// 新しいバッファの保存
								Result.Dim = NewBuffer ;
								MaxNum = NewMaxNum ;
							}

							// 当たっているポリゴンリストに追加する
							RPoly = &Result.Dim[ Result.HitNum ] ;
							Result.HitNum ++ ;

							RPoly->HitFlag = 1 ;
							RPoly->HitPosition = ResultPos ;

							RPoly->FrameIndex = Poly->FrameIndex ;
							RPoly->MaterialIndex = Poly->MaterialIndex ;
							RPoly->PolygonIndex = PolyIndex ;
							RPoly->Position[ 0 ] = PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position ;
							RPoly->Position[ 1 ] = PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position ;
							RPoly->Position[ 2 ] = PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ;
							RPoly->Normal = VNorm( VCross( VSub( RPoly->Position[ 1 ], RPoly->Position[ 0 ] ), VSub( RPoly->Position[ 2 ], RPoly->Position[ 0 ] ) ) ) ; 
						}
					}

					// チェック済みフラグを立てる
					BitBuffer[ PolyIndex >> 3 ] |= 1 << ( PolyIndex & 7 ) ;
				}
			}
		}
	}

	// 一つも当たっていなかったら当たっているポリゴンリスト用に確保したメモリを解放
	if( Result.HitNum == 0 )
	{
		if( Result.Dim != NULL )
		{
			DXFREE( Result.Dim ) ;
			Result.Dim = NULL ;
		}
	}

END :

	// メモリの解放
	if( BitBuffer )
	{
		DXFREE( BitBuffer ) ;
		BitBuffer = NULL ;
	}

	return Result ;
}

// カプセルとモデルの当たり判定
extern MV1_COLL_RESULT_POLY_DIM NS_MV1CollCheck_Capsule( int MHandle, int FrameIndex, VECTOR Pos1, VECTOR Pos2, float r )
{
	int MaxNum ;
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_COLL_RESULT_POLY_DIM Result ;
	MV1_COLL_RESULT_POLY *RPoly ;
	MV1_REF_POLYGONLIST *PolyList ;
	MV1_REF_POLYGON *Poly ;
	MV1_COLLISION *Collision ;
	MV1_COLL_POLYGON *ColPoly ;
	BYTE *BitBuffer = NULL ;
	VECTOR UnitDiv, MinPos, MaxPos/*, Sa, ResultPos */ ;
	int xc, yc, zc, ZStep, YStep, PolyIndex ;
	int MinX, MinY, MinZ, MaxX, MaxY, MaxZ, PostIndexZ, PostIndexY, PostIndexX ;

	// 結果初期化
	Result.HitNum = 0 ;
	Result.Dim = NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return Result ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// コリジョン情報が無かったらセットアップする
		if( Model->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Model->RefPolygon[ 1 ][ 1 ] ;
		Collision = Model->Collision ;
	}
	else
	{
		// フレーム単体の場合

		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return Result ;
		Frame = Model->Frame + FrameIndex ;

		// コリジョン情報が無かったらセットアップする
		if( Frame->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Frame->RefPolygon[ 1 ][ 1 ] ;
		Collision = Frame->Collision ;
	}

	// 座標値の最小値と最大値をセット
	if( Pos1.x < Pos2.x )
	{
		MaxPos.x = Pos2.x + r ;
		MinPos.x = Pos1.x - r ;
	}
	else
	{
		MaxPos.x = Pos1.x + r ;
		MinPos.x = Pos2.x - r ;
	}
	if( Pos1.y < Pos2.y )
	{
		MaxPos.y = Pos2.y + r ;
		MinPos.y = Pos1.y - r ;
	}
	else
	{
		MaxPos.y = Pos1.y + r ;
		MinPos.y = Pos2.y - r ;
	}
	if( Pos1.z < Pos2.z )
	{
		MaxPos.z = Pos2.z + r ;
		MinPos.z = Pos1.z - r ;
	}
	else
	{
		MaxPos.z = Pos1.z + r ;
		MinPos.z = Pos2.z - r ;
	}

	// 範囲がコリジョン空間から外れていたらどのポリゴンにも当たらないということ
	if( Collision->MaxPosition.x < MinPos.x ||
		Collision->MaxPosition.y < MinPos.y ||
		Collision->MaxPosition.z < MinPos.z ||
		Collision->MinPosition.x > MaxPos.x ||
		Collision->MinPosition.y > MaxPos.y ||
		Collision->MinPosition.z > MaxPos.z )
		goto END ;

	// 初期バッファを確保
	MaxNum = 1000 ;
	Result.Dim = ( MV1_COLL_RESULT_POLY * )DXALLOC( sizeof( MV1_COLL_RESULT_POLY ) * MaxNum ) ;
	if( Result.Dim == NULL ) return Result ;

	// 当たり判定を行ったかどうかを保存するメモリ領域の確保
	BitBuffer = ( BYTE * )DXALLOC( sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;
	if( BitBuffer == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x53\x5f\x5f\x30\x8a\x30\x24\x52\x9a\x5b\xe6\x51\x06\x74\x28\x75\xc6\x30\xf3\x30\xdd\x30\xe9\x30\xea\x30\xd0\x30\xc3\x30\xd5\x30\xa1\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x32\x00\x0a\x00\x00"/*@ L"当たり判定処理用テンポラリバッファの確保に失敗しました2\n" @*/ )) ;
		DXFREE( Result.Dim ) ;
		return Result ;
	}
	_MEMSET( BitBuffer, 0, sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;

	// 検索範囲を決定
	UnitDiv = Collision->UnitSizeRev ;
	MinX = __FTOL( ( MinPos.x - Collision->MinPosition.x ) * UnitDiv.x ) ;
	MinY = __FTOL( ( MinPos.y - Collision->MinPosition.y ) * UnitDiv.y ) ;
	MinZ = __FTOL( ( MinPos.z - Collision->MinPosition.z ) * UnitDiv.z ) ;
	MaxX = __FTOL( ( MaxPos.x - Collision->MinPosition.x ) * UnitDiv.x ) + 1 ;
	MaxY = __FTOL( ( MaxPos.y - Collision->MinPosition.y ) * UnitDiv.y ) + 1 ;
	MaxZ = __FTOL( ( MaxPos.z - Collision->MinPosition.z ) * UnitDiv.z ) + 1 ;
	if( MinX < 0 ) MinX = 0 ;
	if( MinY < 0 ) MinY = 0 ;
	if( MinZ < 0 ) MinZ = 0 ;
	if( MaxX >= Collision->XDivNum ) MaxX = Collision->XDivNum - 1 ;
	if( MaxY >= Collision->YDivNum ) MaxY = Collision->YDivNum - 1 ;
	if( MaxZ >= Collision->ZDivNum ) MaxZ = Collision->ZDivNum - 1 ;

	// 範囲内の全てのポリゴンと当たり判定
	ZStep = Collision->YDivNum * Collision->XDivNum ;
	YStep = Collision->XDivNum ;
	PostIndexZ = MinX + MinY * YStep + MinZ * ZStep ;
	for( zc = MinZ ; zc <= MaxZ ; zc ++, PostIndexZ += ZStep )
	{
		PostIndexY = PostIndexZ ;
		for( yc = MinY ; yc <= MaxY ; yc ++, PostIndexY += YStep )
		{
			PostIndexX = PostIndexY ;
			for( xc = MinX ; xc <= MaxX ; xc ++, PostIndexX ++ )
			{
				for( ColPoly = Collision->Polygon[ PostIndexX ] ; ColPoly ; ColPoly = ColPoly->Next )
				{
					Poly = ColPoly->Polygon ;
					PolyIndex = ( int )( Poly - PolyList->Polygons ) ;

					// 既にチェック済みのポリゴンだった場合は何もしない
					if( BitBuffer[ PolyIndex >> 3 ] & ( 1 << ( PolyIndex & 7 ) ) ) continue ;

					// 最初に立方体単位の当たり判定を行う
					if( ( MinPos.x > Poly->MaxPosition.x ||
						  MinPos.y > Poly->MaxPosition.y ||
						  MinPos.z > Poly->MaxPosition.z ||
						  MaxPos.x < Poly->MinPosition.x ||
						  MaxPos.y < Poly->MinPosition.y ||
						  MaxPos.z < Poly->MinPosition.z ) == false )
					{
						// 三角形と球の当たり判定

						// 球に一番近い三角形上の座標との距離が、球の半径以下だったら当たっているということ
/*						ResultPos = Get_Triangle_Point_MinPosition( CenterPos, 
										PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position,
										PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) ;
						Sa.x = CenterPos.x - ResultPos.x ;
						Sa.y = CenterPos.y - ResultPos.y ;
						Sa.z = CenterPos.z - ResultPos.z ;
						if( Sa.x * Sa.x + Sa.y * Sa.y + Sa.z * Sa.z <= r * r )
*/						if( Segment_Triangle_MinLength_Square(
							Pos1, Pos2, 
							PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position,
							PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position,
							PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) <= r * r )
						{
							// 数が最大数に達していたら再確保
							if( Result.HitNum == MaxNum )
							{
								MV1_COLL_RESULT_POLY *NewBuffer ;
								int NewMaxNum ;

								NewMaxNum = MaxNum * 2 ;
								NewBuffer = ( MV1_COLL_RESULT_POLY * )DXREALLOC( Result.Dim, sizeof( MV1_COLL_RESULT_POLY ) * NewMaxNum ) ;
								if( NewBuffer == NULL )
								{
									DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe1\x30\xe2\x30\xea\x30\x0d\x4e\xb3\x8d\x6e\x30\xba\x70\x53\x5f\x5f\x30\x63\x30\x5f\x30\xdd\x30\xea\x30\xb4\x30\xf3\x30\x59\x30\x79\x30\x66\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\x53\x30\x68\x30\x4c\x30\x67\x30\x4d\x30\x7e\x30\x5b\x30\x93\x30\x67\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"メモリ不足の為当たったポリゴンすべてを保存することができませんでした\n" @*/ )) ;
									goto END ;
								}

								// 新しいバッファの保存
								Result.Dim = NewBuffer ;
								MaxNum = NewMaxNum ;
							}

							// 当たっているポリゴンリストに追加する
							RPoly = &Result.Dim[ Result.HitNum ] ;
							Result.HitNum ++ ;

							RPoly->HitFlag = 1 ;
//							RPoly->HitPosition = ResultPos ;
							RPoly->HitPosition.x = 0.0f ;
							RPoly->HitPosition.y = 0.0f ;
							RPoly->HitPosition.z = 0.0f ;

							RPoly->FrameIndex = Poly->FrameIndex ;
							RPoly->MaterialIndex = Poly->MaterialIndex ;
							RPoly->PolygonIndex = PolyIndex ;
							RPoly->Position[ 0 ] = PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position ;
							RPoly->Position[ 1 ] = PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position ;
							RPoly->Position[ 2 ] = PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ;
							RPoly->Normal = VNorm( VCross( VSub( RPoly->Position[ 1 ], RPoly->Position[ 0 ] ), VSub( RPoly->Position[ 2 ], RPoly->Position[ 0 ] ) ) ) ; 
						}
					}

					// チェック済みフラグを立てる
					BitBuffer[ PolyIndex >> 3 ] |= 1 << ( PolyIndex & 7 ) ;
				}
			}
		}
	}

	// 一つも当たっていなかったら当たっているポリゴンリスト用に確保したメモリを解放
	if( Result.HitNum == 0 )
	{
		if( Result.Dim != NULL )
		{
			DXFREE( Result.Dim ) ;
			Result.Dim = NULL ;
		}
	}

END :

	// メモリの解放
	if( BitBuffer )
	{
		DXFREE( BitBuffer ) ;
		BitBuffer = NULL ;
	}

	return Result ;
}

// 三角形とモデルの当たり判定
extern MV1_COLL_RESULT_POLY_DIM NS_MV1CollCheck_Triangle( int MHandle, int FrameIndex, VECTOR Pos1, VECTOR Pos2, VECTOR Pos3 )
{
	int MaxNum ;
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_COLL_RESULT_POLY_DIM Result ;
	MV1_COLL_RESULT_POLY *RPoly ;
	MV1_REF_POLYGONLIST *PolyList ;
	MV1_REF_POLYGON *Poly ;
	MV1_COLLISION *Collision ;
	MV1_COLL_POLYGON *ColPoly ;
	BYTE *BitBuffer = NULL ;
	VECTOR UnitDiv, MinPos, MaxPos/*, Sa, ResultPos */ ;
	int xc, yc, zc, ZStep, YStep, PolyIndex ;
	int MinX, MinY, MinZ, MaxX, MaxY, MaxZ, PostIndexZ, PostIndexY, PostIndexX ;

	// 結果初期化
	Result.HitNum = 0 ;
	Result.Dim = NULL ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return Result ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// コリジョン情報が無かったらセットアップする
		if( Model->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Model->RefPolygon[ 1 ][ 1 ] ;
		Collision = Model->Collision ;
	}
	else
	{
		// フレーム単体の場合

		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return Result ;
		Frame = Model->Frame + FrameIndex ;

		// コリジョン情報が無かったらセットアップする
		if( Frame->Collision == NULL )
			if( NS_MV1RefreshCollInfo( MHandle, FrameIndex ) < 0 )
				return Result ;

		// ポリゴン情報のセット
		PolyList = Frame->RefPolygon[ 1 ][ 1 ] ;
		Collision = Frame->Collision ;
	}

	// 座標値の最小値と最大値をセット
	if( Pos1.x < Pos2.x )
	{
		if( Pos1.x < Pos3.x )
		{
			MinPos.x = Pos1.x ;
		}
		else
		{
			MinPos.x = Pos3.x ;
		}

		if( Pos2.x > Pos3.x )
		{
			MaxPos.x = Pos2.x ;
		}
		else
		{
			MaxPos.x = Pos3.x ;
		}
	}
	else
	{
		if( Pos2.x < Pos3.x )
		{
			MinPos.x = Pos2.x ;
		}
		else
		{
			MinPos.x = Pos3.x ;
		}

		if( Pos1.x > Pos3.x )
		{
			MaxPos.x = Pos1.x ;
		}
		else
		{
			MaxPos.x = Pos3.x ;
		}
	}

	if( Pos1.y < Pos2.y )
	{
		if( Pos1.y < Pos3.y )
		{
			MinPos.y = Pos1.y ;
		}
		else
		{
			MinPos.y = Pos3.y ;
		}

		if( Pos2.y > Pos3.y )
		{
			MaxPos.y = Pos2.y ;
		}
		else
		{
			MaxPos.y = Pos3.y ;
		}
	}
	else
	{
		if( Pos2.y < Pos3.y )
		{
			MinPos.y = Pos2.y ;
		}
		else
		{
			MinPos.y = Pos3.y ;
		}

		if( Pos1.y > Pos3.y )
		{
			MaxPos.y = Pos1.y ;
		}
		else
		{
			MaxPos.y = Pos3.y ;
		}
	}

	if( Pos1.z < Pos2.z )
	{
		if( Pos1.z < Pos3.z )
		{
			MinPos.z = Pos1.z ;
		}
		else
		{
			MinPos.z = Pos3.z ;
		}

		if( Pos2.z > Pos3.z )
		{
			MaxPos.z = Pos2.z ;
		}
		else
		{
			MaxPos.z = Pos3.z ;
		}
	}
	else
	{
		if( Pos2.z < Pos3.z )
		{
			MinPos.z = Pos2.z ;
		}
		else
		{
			MinPos.z = Pos3.z ;
		}

		if( Pos1.z > Pos3.z )
		{
			MaxPos.z = Pos1.z ;
		}
		else
		{
			MaxPos.z = Pos3.z ;
		}
	}

	// 範囲がコリジョン空間から外れていたらどのポリゴンにも当たらないということ
	if( Collision->MaxPosition.x < MinPos.x ||
		Collision->MaxPosition.y < MinPos.y ||
		Collision->MaxPosition.z < MinPos.z ||
		Collision->MinPosition.x > MaxPos.x ||
		Collision->MinPosition.y > MaxPos.y ||
		Collision->MinPosition.z > MaxPos.z )
		goto END ;

	// 初期バッファを確保
	MaxNum = 1000 ;
	Result.Dim = ( MV1_COLL_RESULT_POLY * )DXALLOC( sizeof( MV1_COLL_RESULT_POLY ) * MaxNum ) ;
	if( Result.Dim == NULL ) return Result ;

	// 当たり判定を行ったかどうかを保存するメモリ領域の確保
	BitBuffer = ( BYTE * )DXALLOC( sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;
	if( BitBuffer == NULL )
	{
		DXST_ERRORLOGFMT_ADDUTF16LE(( "\x53\x5f\x5f\x30\x8a\x30\x24\x52\x9a\x5b\xe6\x51\x06\x74\x28\x75\xc6\x30\xf3\x30\xdd\x30\xe9\x30\xea\x30\xd0\x30\xc3\x30\xd5\x30\xa1\x30\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x32\x00\x0a\x00\x00"/*@ L"当たり判定処理用テンポラリバッファの確保に失敗しました2\n" @*/ )) ;
		DXFREE( Result.Dim ) ;
		return Result ;
	}
	_MEMSET( BitBuffer, 0, sizeof( BYTE ) * ( ( PolyList->PolygonNum + 7 ) / 8 ) ) ;

	// 検索範囲を決定
	UnitDiv = Collision->UnitSizeRev ;
	MinX = __FTOL( ( MinPos.x - Collision->MinPosition.x ) * UnitDiv.x ) ;
	MinY = __FTOL( ( MinPos.y - Collision->MinPosition.y ) * UnitDiv.y ) ;
	MinZ = __FTOL( ( MinPos.z - Collision->MinPosition.z ) * UnitDiv.z ) ;
	MaxX = __FTOL( ( MaxPos.x - Collision->MinPosition.x ) * UnitDiv.x ) + 1 ;
	MaxY = __FTOL( ( MaxPos.y - Collision->MinPosition.y ) * UnitDiv.y ) + 1 ;
	MaxZ = __FTOL( ( MaxPos.z - Collision->MinPosition.z ) * UnitDiv.z ) + 1 ;
	if( MinX < 0 ) MinX = 0 ;
	if( MinY < 0 ) MinY = 0 ;
	if( MinZ < 0 ) MinZ = 0 ;
	if( MaxX >= Collision->XDivNum ) MaxX = Collision->XDivNum - 1 ;
	if( MaxY >= Collision->YDivNum ) MaxY = Collision->YDivNum - 1 ;
	if( MaxZ >= Collision->ZDivNum ) MaxZ = Collision->ZDivNum - 1 ;

	// 範囲内の全てのポリゴンと当たり判定
	ZStep = Collision->YDivNum * Collision->XDivNum ;
	YStep = Collision->XDivNum ;
	PostIndexZ = MinX + MinY * YStep + MinZ * ZStep ;
	for( zc = MinZ ; zc <= MaxZ ; zc ++, PostIndexZ += ZStep )
	{
		PostIndexY = PostIndexZ ;
		for( yc = MinY ; yc <= MaxY ; yc ++, PostIndexY += YStep )
		{
			PostIndexX = PostIndexY ;
			for( xc = MinX ; xc <= MaxX ; xc ++, PostIndexX ++ )
			{
				for( ColPoly = Collision->Polygon[ PostIndexX ] ; ColPoly ; ColPoly = ColPoly->Next )
				{
					Poly = ColPoly->Polygon ;
					PolyIndex = ( int )( Poly - PolyList->Polygons ) ;

					// 既にチェック済みのポリゴンだった場合は何もしない
					if( BitBuffer[ PolyIndex >> 3 ] & ( 1 << ( PolyIndex & 7 ) ) ) continue ;

					// 最初に立方体単位の当たり判定を行う
					if( ( MinPos.x > Poly->MaxPosition.x ||
						  MinPos.y > Poly->MaxPosition.y ||
						  MinPos.z > Poly->MaxPosition.z ||
						  MaxPos.x < Poly->MinPosition.x ||
						  MaxPos.y < Poly->MinPosition.y ||
						  MaxPos.z < Poly->MinPosition.z ) == false )
					{
						// 三角形と三角形の当たり判定
						if( HitCheck_Triangle_Triangle(
							Pos1, Pos2, Pos3, 
							PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position,
							PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position,
							PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ) )
						{
							// 数が最大数に達していたら再確保
							if( Result.HitNum == MaxNum )
							{
								MV1_COLL_RESULT_POLY *NewBuffer ;
								int NewMaxNum ;

								NewMaxNum = MaxNum * 2 ;
								NewBuffer = ( MV1_COLL_RESULT_POLY * )DXREALLOC( Result.Dim, sizeof( MV1_COLL_RESULT_POLY ) * NewMaxNum ) ;
								if( NewBuffer == NULL )
								{
									DXST_ERRORLOGFMT_ADDUTF16LE(( "\xe1\x30\xe2\x30\xea\x30\x0d\x4e\xb3\x8d\x6e\x30\xba\x70\x53\x5f\x5f\x30\x63\x30\x5f\x30\xdd\x30\xea\x30\xb4\x30\xf3\x30\x59\x30\x79\x30\x66\x30\x92\x30\xdd\x4f\x58\x5b\x59\x30\x8b\x30\x53\x30\x68\x30\x4c\x30\x67\x30\x4d\x30\x7e\x30\x5b\x30\x93\x30\x67\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"メモリ不足の為当たったポリゴンすべてを保存することができませんでした\n" @*/ )) ;
									goto END ;
								}

								// 新しいバッファの保存
								Result.Dim = NewBuffer ;
								MaxNum = NewMaxNum ;
							}

							// 当たっているポリゴンリストに追加する
							RPoly = &Result.Dim[ Result.HitNum ] ;
							Result.HitNum ++ ;

							RPoly->HitFlag = 1 ;
							RPoly->HitPosition.x = 0.0f ;
							RPoly->HitPosition.y = 0.0f ;
							RPoly->HitPosition.z = 0.0f ;

							RPoly->FrameIndex = Poly->FrameIndex ;
							RPoly->MaterialIndex = Poly->MaterialIndex ;
							RPoly->PolygonIndex = PolyIndex ;
							RPoly->Position[ 0 ] = PolyList->Vertexs[ Poly->VIndex[ 0 ] ].Position ;
							RPoly->Position[ 1 ] = PolyList->Vertexs[ Poly->VIndex[ 1 ] ].Position ;
							RPoly->Position[ 2 ] = PolyList->Vertexs[ Poly->VIndex[ 2 ] ].Position ;
							RPoly->Normal = VNorm( VCross( VSub( RPoly->Position[ 1 ], RPoly->Position[ 0 ] ), VSub( RPoly->Position[ 2 ], RPoly->Position[ 0 ] ) ) ) ; 
						}
					}

					// チェック済みフラグを立てる
					BitBuffer[ PolyIndex >> 3 ] |= 1 << ( PolyIndex & 7 ) ;
				}
			}
		}
	}

END :

	// 一つも当たっていなかったら当たっているポリゴンリスト用に確保したメモリを解放
	if( Result.HitNum == 0 )
	{
		if( Result.Dim != NULL )
		{
			DXFREE( Result.Dim ) ;
			Result.Dim = NULL ;
		}
	}

	// メモリの解放
	if( BitBuffer )
	{
		DXFREE( BitBuffer ) ;
		BitBuffer = NULL ;
	}

	return Result ;
}

// コリジョン結果ポリゴン配列から指定番号のポリゴン情報を取得する
extern MV1_COLL_RESULT_POLY NS_MV1CollCheck_GetResultPoly( MV1_COLL_RESULT_POLY_DIM ResultPolyDim, int PolyNo )
{
	MV1_COLL_RESULT_POLY ErrorResult ;

	if( PolyNo < 0 || PolyNo >= ResultPolyDim.HitNum || ResultPolyDim.Dim == NULL )
	{
		_MEMSET( &ErrorResult, 0, sizeof( ErrorResult ) ) ;
		return ErrorResult ;
	}

	return ResultPolyDim.Dim[ PolyNo ] ;
}

// コリジョン結果ポリゴン配列の後始末をする
extern	int NS_MV1CollResultPolyDimTerminate( MV1_COLL_RESULT_POLY_DIM ResultPolyDim )
{
	if( ResultPolyDim.Dim != NULL )
	{
		DXFREE( ResultPolyDim.Dim ) ;
		ResultPolyDim.Dim = NULL ;
	}
	ResultPolyDim.HitNum = 0 ;

	// 終了
	return 0 ;
}

// 参照用メッシュのセットアップ
extern int NS_MV1SetupReferenceMesh( int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly )
{
	int i ;
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	int VertexNum ;

	IsTransform = IsTransform != 0 ? 1 : 0 ;
	IsPositionOnly = IsPositionOnly != 0 ? 1 : 0 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// 参照ポリゴンを格納するメモリ領域の確保
		// 確保されていない場合のみ確保
		if( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
		{
			if( IsPositionOnly )
			{
				MV1_FRAME_BASE *FrameBase ;

				FrameBase = ModelBase->Frame ;
				VertexNum = 0 ;
				for( i = 0 ; i < ModelBase->FrameNum ; i ++, FrameBase ++ )
				{
					VertexNum += FrameBase->PositionNum ;
				}
			}
			else
			{
				VertexNum = ModelBase->TriangleListVertexNum ;
			}
			Model->RefPolygon[ IsTransform ][ IsPositionOnly ] = ( MV1_REF_POLYGONLIST * )DXALLOC( sizeof( MV1_REF_POLYGONLIST ) + sizeof( MV1_REF_POLYGON ) * ModelBase->TriangleNum + sizeof( MV1_REF_VERTEX ) * VertexNum ) ;
			if( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\xc2\x53\x67\x71\x28\x75\xdd\x30\xea\x30\xb4\x30\xf3\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"参照用ポリゴンを格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
				return -1 ;
			}
			Model->RefPolygon[ IsTransform ][ IsPositionOnly ]->Polygons = ( MV1_REF_POLYGON * )( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] + 1 ) ;
			Model->RefPolygon[ IsTransform ][ IsPositionOnly ]->Vertexs  = ( MV1_REF_VERTEX  * )( Model->RefPolygon[ IsTransform ][ IsPositionOnly ]->Polygons + ModelBase->TriangleNum ) ;

			// ポリゴン情報は最初にセットアップしておく
			Model->RefPolygon[ IsTransform ][ IsPositionOnly ]->PolygonNum = 0 ;
			Model->RefPolygon[ IsTransform ][ IsPositionOnly ]->VertexNum = 0 ;
			for( i = 0 ; i < ModelBase->FrameNum ; i ++ )
			{
				MV1SetupReferenceMeshFrame( Model, ModelBase, Model->Frame + i, Model->RefPolygon[ IsTransform ][ IsPositionOnly ], 0, IsTransform ? true : false, IsPositionOnly ? true : false ) ;
			}

			// 最小値と最大値を計算しておく
			if( IsTransform == false )
			{
				_MV1SetupReferenceMeshMaxAndMinPosition( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] ) ;
			}
		}
/*
		// 参照ポリゴンを格納するメモリ領域の確保
		if( IsTransform )
		{
			// 確保されていない場合のみ確保
			if( Model->TransformPolygon == NULL )
			{
				Model->TransformPolygon = ( MV1_REF_POLYGONLIST * )DXALLOC( sizeof( MV1_REF_POLYGONLIST ) + sizeof( MV1_REF_POLYGON ) * ModelBase->TriangleNum + sizeof( MV1_REF_VERTEX ) * ModelBase->TriangleListVertexNum ) ;
				if( Model->TransformPolygon == NULL )
				{
					DXST_ERRORLOGFMT_ADDW(( L"参照用ポリゴンを格納するメモリ領域の確保に失敗しました\n" )) ;
					return -1 ;
				}
				Model->TransformPolygon->Polygons = ( MV1_REF_POLYGON * )( Model->TransformPolygon + 1 ) ;
				Model->TransformPolygon->Vertexs = ( MV1_REF_VERTEX * )( Model->TransformPolygon->Polygons + ModelBase->TriangleNum ) ;

				// ポリゴン情報は最初にセットアップしておく
				Model->TransformPolygon->PolygonNum = 0 ;
				Model->TransformPolygon->VertexNum = 0 ;
				for( i = 0 ; i < ModelBase->FrameNum ; i ++ )
				{
					MV1SetupReferenceMeshFrame( Model, ModelBase, Model->Frame + i, Model->TransformPolygon, 0, IsTransform ? true : false ) ;
				}
			}
		}
		else
		{
			// 確保されていない場合のみ確保
			if( Model->NonTransformPolygon == NULL )
			{
				Model->NonTransformPolygon = ( MV1_REF_POLYGONLIST * )DXALLOC( sizeof( MV1_REF_POLYGONLIST ) + sizeof( MV1_REF_POLYGON ) * ModelBase->TriangleNum + sizeof( MV1_REF_VERTEX ) * ModelBase->TriangleListVertexNum ) ;
				if( Model->NonTransformPolygon == NULL )
				{
					DXST_ERRORLOGFMT_ADDW(( L"参照用ポリゴンを格納するメモリ領域の確保に失敗しました\n" )) ;
					return -1 ;
				}
				Model->NonTransformPolygon->Polygons = ( MV1_REF_POLYGON * )( Model->NonTransformPolygon + 1 ) ;
				Model->NonTransformPolygon->Vertexs = ( MV1_REF_VERTEX * )( Model->NonTransformPolygon->Polygons + ModelBase->TriangleNum ) ;

				// ポリゴン情報は最初にセットアップしておく
				Model->NonTransformPolygon->PolygonNum = 0 ;
				Model->NonTransformPolygon->VertexNum = 0 ;
				for( i = 0 ; i < ModelBase->FrameNum ; i ++ )
				{
					MV1SetupReferenceMeshFrame( Model, ModelBase, Model->Frame + i, Model->NonTransformPolygon, 0, IsTransform ? true : false ) ;
				}

				// 最小値と最大値を計算しておく
				_MV1SetupReferenceMeshMaxAndMinPosition( Model->NonTransformPolygon ) ;
			}
		}
*/
	}
	else
	{
		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return -1 ;
		Frame = Model->Frame + FrameIndex ;

		// 参照ポリゴンを格納するメモリ領域の確保
		// 確保されていない場合のみ確保
		if( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
		{
			if( IsPositionOnly )
			{
				VertexNum = Frame->BaseData->PositionNum ;
			}
			else
			{
				VertexNum = Frame->BaseData->VertexNum ;
			}
			Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] = ( MV1_REF_POLYGONLIST * )DXALLOC( sizeof( MV1_REF_POLYGONLIST ) + sizeof( MV1_REF_POLYGON ) * Frame->BaseData->TriangleNum + sizeof( MV1_REF_VERTEX ) * VertexNum ) ;
			if( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
			{
				DXST_ERRORLOGFMT_ADDUTF16LE(( "\xc2\x53\x67\x71\x28\x75\xdd\x30\xea\x30\xb4\x30\xf3\x30\x92\x30\x3c\x68\x0d\x7d\x59\x30\x8b\x30\xe1\x30\xe2\x30\xea\x30\x18\x98\xdf\x57\x6e\x30\xba\x78\xdd\x4f\x6b\x30\x31\x59\x57\x65\x57\x30\x7e\x30\x57\x30\x5f\x30\x0a\x00\x00"/*@ L"参照用ポリゴンを格納するメモリ領域の確保に失敗しました\n" @*/ )) ;
				return -1 ;
			}
			Frame->RefPolygon[ IsTransform ][ IsPositionOnly ]->Polygons = ( MV1_REF_POLYGON * )( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] + 1 ) ;
			Frame->RefPolygon[ IsTransform ][ IsPositionOnly ]->Vertexs = ( MV1_REF_VERTEX * )( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ]->Polygons + Frame->BaseData->TriangleNum ) ;

			// ポリゴン情報は最初にセットアップしておく
			Frame->RefPolygon[ IsTransform ][ IsPositionOnly ]->PolygonNum = 0 ;
			Frame->RefPolygon[ IsTransform ][ IsPositionOnly ]->VertexNum = 0 ;
			MV1SetupReferenceMeshFrame( Model, ModelBase, Frame, Frame->RefPolygon[ IsTransform ][ IsPositionOnly ], 1, IsTransform ? true : false, IsPositionOnly ? true : false ) ;

			// 最小値と最大値を計算しておく
			if( IsTransform == false )
			{
				_MV1SetupReferenceMeshMaxAndMinPosition( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] ) ;
			}
		}
/*
		// 参照ポリゴンを格納するメモリ領域の確保
		if( IsTransform )
		{
			// 確保されていない場合のみ確保
			if( Frame->TransformPolygon == NULL )
			{
				Frame->TransformPolygon = ( MV1_REF_POLYGONLIST * )DXALLOC( sizeof( MV1_REF_POLYGONLIST ) + sizeof( MV1_REF_POLYGON ) * Frame->BaseData->TriangleNum + sizeof( MV1_REF_VERTEX ) * Frame->BaseData->VertexNum ) ;
				if( Frame->TransformPolygon == NULL )
				{
					DXST_ERRORLOGFMT_ADDW(( L"参照用ポリゴンを格納するメモリ領域の確保に失敗しました\n" )) ;
					return -1 ;
				}
				Frame->TransformPolygon->Polygons = ( MV1_REF_POLYGON * )( Frame->TransformPolygon + 1 ) ;
				Frame->TransformPolygon->Vertexs = ( MV1_REF_VERTEX * )( Frame->TransformPolygon->Polygons + Frame->BaseData->TriangleNum ) ;

				// ポリゴン情報は最初にセットアップしておく
				Frame->TransformPolygon->PolygonNum = 0 ;
				Frame->TransformPolygon->VertexNum = 0 ;
				MV1SetupReferenceMeshFrame( Model, ModelBase, Frame, Frame->TransformPolygon, 1, IsTransform ? true : false  ) ;
			}
		}
		else
		{
			// 確保されていない場合のみ確保
			if( Frame->NonTransformPolygon == NULL )
			{
				Frame->NonTransformPolygon = ( MV1_REF_POLYGONLIST * )DXALLOC( sizeof( MV1_REF_POLYGONLIST ) + sizeof( MV1_REF_POLYGON ) * Frame->BaseData->TriangleNum + sizeof( MV1_REF_VERTEX ) * Frame->BaseData->VertexNum ) ;
				if( Frame->NonTransformPolygon == NULL )
				{
					DXST_ERRORLOGFMT_ADDW(( L"参照用ポリゴンを格納するメモリ領域の確保に失敗しました\n" )) ;
					return -1 ;
				}
				Frame->NonTransformPolygon->Polygons = ( MV1_REF_POLYGON * )( Frame->NonTransformPolygon + 1 ) ;
				Frame->NonTransformPolygon->Vertexs = ( MV1_REF_VERTEX * )( Frame->NonTransformPolygon->Polygons + Frame->BaseData->TriangleNum ) ;

				// ポリゴン情報は最初にセットアップしておく
				Frame->NonTransformPolygon->PolygonNum = 0 ;
				Frame->NonTransformPolygon->VertexNum = 0 ;
				MV1SetupReferenceMeshFrame( Model, ModelBase, Frame, Frame->NonTransformPolygon, 1, IsTransform ? true : false  ) ;

				// 最小値と最大値を計算しておく
				_MV1SetupReferenceMeshMaxAndMinPosition( Frame->NonTransformPolygon ) ;
			}
		}
*/
	}

	// 参照用メッシュを構築しておく
	NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, IsTransform, IsPositionOnly ) ;

	// 終了
	return 0 ;
}

// 参照用メッシュの後始末
extern int NS_MV1TerminateReferenceMesh( int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;

	IsTransform = IsTransform != 0 ? 1 : 0 ;
	IsPositionOnly = IsPositionOnly != 0 ? 1 : 0 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// フレーム単体かモデル全体かで処理を分岐
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// メモリが確保されていたら解放
		if( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] )
		{
			DXFREE( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] ) ;
			Model->RefPolygon[ IsTransform ][ IsPositionOnly ] = NULL ;
		}
/*
		// 変換頂点を取得するかどうかで処理を分岐
		if( IsTransform )
		{
			// メモリが確保されていたら解放
			if( Model->TransformPolygon )
			{
				DXFREE( Model->TransformPolygon ) ;
				Model->TransformPolygon = NULL ;
			}
		}
		else
		{
			// 確保されていない場合のみ確保
			if( Model->NonTransformPolygon )
			{
				DXFREE( Model->NonTransformPolygon ) ;
				Model->NonTransformPolygon = NULL ;
			}
		}
*/
	}
	else
	{
		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return -1 ;
		Frame = Model->Frame + FrameIndex ;

		// メモリが確保されていたら解放
		if( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] )
		{
			DXFREE( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] ) ;
			Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] = NULL ;
		}
/*
		// 参照ポリゴンを格納するメモリ領域の確保
		if( IsTransform )
		{
			// 確保されていない場合のみ確保
			if( Frame->TransformPolygon )
			{
				DXFREE( Frame->TransformPolygon ) ;
				Frame->TransformPolygon = NULL ;
			}
		}
		else
		{
			// 確保されていない場合のみ確保
			if( Frame->NonTransformPolygon )
			{
				DXFREE( Frame->NonTransformPolygon ) ;
				Frame->NonTransformPolygon = NULL ;
			}
		}
*/
	}

	// 終了
	return 0 ;
}

// 参照用メッシュのセットアップを行う
static int MV1SetupReferenceMeshFrame(
	  MV1_MODEL           * /*Model*/,
	  MV1_MODEL_BASE      *ModelBase,
	  MV1_FRAME           *Frame,
	  MV1_REF_POLYGONLIST *DestBuffer,
	  int                  VIndexTarget,
	  bool                 IsTransform,
	  bool                 IsPositionOnly )
{
	MV1_MESH_BASE *Mesh ;
	MV1_TRIANGLE_LIST_BASE *TList ;
	MV1_REF_POLYGON *Poly ;
	MV1_REF_VERTEX *Vert, *TVert ;
	MV1_TLIST_NORMAL_POS *Norm ;
	MV1_TLIST_SKIN_POS_4B *SkinB4 ;
	MV1_TLIST_SKIN_POS_8B *SkinB8 ;
	MV1_TLIST_SKIN_POS_FREEB *SkinBF ;
	MV1_MESH_POSITION *Position ;
	MV1_MESH_FACE *Face ;
	MV1_MESH_VERTEX *MeshVert, *MVert ;
	DWORD *MVertIndex ;
	int i, j, k, l, StartIndex, MVertUnitSize, PosUnitSize, UVNum ;

	// 出力アドレスをセット
	Poly = DestBuffer->Polygons + DestBuffer->PolygonNum ;
	Vert = DestBuffer->Vertexs  + DestBuffer->VertexNum ;

	// このフレームが所有しているポリゴンの情報と変化しない頂点の情報をセットする
	Mesh = Frame->BaseData->Mesh ;
	for( i = 0 ; i < Frame->BaseData->MeshNum ; i ++, Mesh ++ )
	{
		MVertUnitSize = Mesh->VertUnitSize ;
		MeshVert = Mesh->Vertex ;

		// 座標のみかどうかで処理を分岐
		if( IsPositionOnly )
		{
			// ポリゴンのデータをセット
			Face = Mesh->Face ;
			StartIndex = DestBuffer->VertexNum ;
			for( k = 0 ; k < Mesh->FaceNum ; k ++, Poly ++, Face ++ )
			{
				Poly->FrameIndex = ( unsigned short )Frame->BaseData->Index ;
				Poly->MaterialIndex = ( unsigned short )( Mesh->Material - ModelBase->Material ) ;
				Poly->VIndexTarget = VIndexTarget ;
				Poly->VIndex[ 0 ] = ( int )( ( ( MV1_MESH_VERTEX * )( ( BYTE * )MeshVert + MVertUnitSize * Face->VertexIndex[ 0 ] ) )->PositionIndex + StartIndex ) ;
				Poly->VIndex[ 1 ] = ( int )( ( ( MV1_MESH_VERTEX * )( ( BYTE * )MeshVert + MVertUnitSize * Face->VertexIndex[ 1 ] ) )->PositionIndex + StartIndex ) ;
				Poly->VIndex[ 2 ] = ( int )( ( ( MV1_MESH_VERTEX * )( ( BYTE * )MeshVert + MVertUnitSize * Face->VertexIndex[ 2 ] ) )->PositionIndex + StartIndex ) ;
			}

			// ポリゴンの数を増やす
			DestBuffer->PolygonNum += Mesh->FaceNum ;
		}
		else
		{
			UVNum = Mesh->UVSetUnitNum > 2 ? 2 : Mesh->UVSetUnitNum ;

			TList = Mesh->TriangleList ;
			for( j = 0 ; j < Mesh->TriangleListNum ; j ++, TList ++ )
			{
				// ポリゴンデータのセット
				StartIndex = DestBuffer->VertexNum ;
				for( k = 0 ; k < TList->IndexNum ; k += 3, Poly ++ )
				{
					Poly->FrameIndex = ( unsigned short )Frame->BaseData->Index ;
					Poly->MaterialIndex = ( unsigned short )( Mesh->Material - ModelBase->Material ) ;
					Poly->VIndexTarget = VIndexTarget ;
					Poly->VIndex[ 0 ] = TList->Index[ k     ] + StartIndex ;
					Poly->VIndex[ 1 ] = TList->Index[ k + 1 ] + StartIndex ;
					Poly->VIndex[ 2 ] = TList->Index[ k + 2 ] + StartIndex ;
				}
				DestBuffer->PolygonNum += TList->IndexNum / 3 ;

				// 頂点データのセット
				if( IsTransform == false )
				{
					TVert = Vert ;
					switch( TList->VertexType )
					{
					case MV1_VERTEX_TYPE_NORMAL :
						Norm = ( MV1_TLIST_NORMAL_POS * )ADDR16( TList->NormalPosition ) ;
						for( k = 0 ; k < TList->VertexNum ; k ++, Vert ++, Norm ++ )
						{
							Vert->Position = *( ( VECTOR * )&Norm->Position ) ;
							Vert->Normal   = *( ( VECTOR * )&Norm->Normal ) ;
						}
						break ;

					case MV1_VERTEX_TYPE_SKIN_4BONE :
						SkinB4 = ( MV1_TLIST_SKIN_POS_4B * )ADDR16( TList->SkinPosition4B ) ;
						for( k = 0 ; k < TList->VertexNum ; k ++, Vert ++, SkinB4 ++ )
						{
							Vert->Position = *( ( VECTOR * )&SkinB4->Position ) ;
							Vert->Normal   = SkinB4->Normal ;
						}
						break ;

					case MV1_VERTEX_TYPE_SKIN_8BONE :
						SkinB8 = ( MV1_TLIST_SKIN_POS_8B * )ADDR16( TList->SkinPosition8B ) ;
						for( k = 0 ; k < TList->VertexNum ; k ++, Vert ++, SkinB8 ++ )
						{
							Vert->Position = SkinB8->Position ;
							Vert->Normal   = SkinB8->Normal ;
						}
						break ;

					case MV1_VERTEX_TYPE_SKIN_FREEBONE :
						SkinBF = ( MV1_TLIST_SKIN_POS_FREEB * )ADDR16( TList->SkinPositionFREEB ) ;
						for( k = 0 ; k < TList->VertexNum ; k ++, Vert ++, SkinBF = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )SkinBF + TList->PosUnitSize ) )
						{
							Vert->Position = *( ( VECTOR * )&SkinBF->Position ) ;
							Vert->Normal   = *( ( VECTOR * )&SkinBF->Normal ) ;
						}
						break ;
					}
					Vert = TVert ;
				}

				MVertIndex = TList->MeshVertexIndex ;
				for( k = 0 ; k < TList->VertexNum ; Vert ++, k ++, MVertIndex ++ )
				{
					MVert = ( MV1_MESH_VERTEX * )( ( BYTE * )MeshVert + MVertUnitSize * *MVertIndex ) ;
					Vert->DiffuseColor = MVert->DiffuseColor ;
					Vert->SpecularColor = MVert->SpecularColor ;
					for( l = 0 ; l < UVNum ; l ++ )
					{
						Vert->TexCoord[ l ].u = MVert->UVs[ l ][ 0 ] ;
						Vert->TexCoord[ l ].v = MVert->UVs[ l ][ 1 ] ;
					}
				}

				// 頂点データの数を増やす
				DestBuffer->VertexNum += TList->VertexNum ;
			}
		}
	}

	if( IsPositionOnly )
	{
		// 頂点データのセット
		if( IsTransform == false )
		{
			PosUnitSize = Frame->BaseData->PosUnitSize ;
			Position = Frame->BaseData->Position ;
			for( k = 0 ; k < Frame->BaseData->PositionNum ; k ++, Vert ++, Position = ( MV1_MESH_POSITION * )( ( BYTE * )Position + PosUnitSize ) )
			{
				Vert->Position = Position->Position ;
			}
		}

		// 頂点データの数を増やす
		DestBuffer->VertexNum += Frame->BaseData->PositionNum ;
	}

	// 終了
	return 0 ;
}

// 参照用メッシュのリフレッシュを行う
static int MV1RefreshReferenceMeshFrame(
	MV1_FRAME			*Frame,
	int					IsPositionOnly,
	MV1_REF_POLYGONLIST *DestBuffer )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_MESH_BASE *Mesh ;
	MV1_FRAME_BASE *MBFrame ;
	MV1_TRIANGLE_LIST_BASE *TList ;
	MV1_REF_VERTEX *Vert ;
	int i, k, l, m, VertNum, PosUnitSize ;
	MV1_TLIST_NORMAL_POS *Norm ;
	MV1_TLIST_SKIN_POS_4B *SkinB4 ;
	MV1_TLIST_SKIN_POS_8B *SkinB8 ;
	MV1_TLIST_SKIN_POS_FREEB *SkinBF ;
	MV1_SKINBONE_BLEND *Blend ;
	MATRIX_4X4CT BlendMat, *BMat ;
	float Weight ;

	// アドレスをセット
	Model = Frame->Container ;
	MBFrame = Frame->BaseData ;
	ModelBase = Model->BaseData ;

	// 出力アドレスをセット
	Vert = DestBuffer->Vertexs + DestBuffer->VertexNum ;

	// このフレームが所有しているポリゴンの情報をセットする
	if( IsPositionOnly )
	{
		MV1_MESH_POSITION *Position ;
		int PosUnitSize ;
		int MaxWeightNum ;

		Position     = Frame->BaseData->Position ;
		PosUnitSize  = Frame->BaseData->PosUnitSize ;
		MaxWeightNum = ( int )( ( PosUnitSize - sizeof( VECTOR ) ) / sizeof( MV1_SKINBONE_BLEND ) ) ;
		for( i = 0 ; i < Frame->BaseData->PositionNum ; i ++, Vert ++, Position = ( MV1_MESH_POSITION * )( ( BYTE * )Position + PosUnitSize ) )
		{
			if( MaxWeightNum == 0 || Position->BoneWeight[ 0 ].Index == -1 )
			{
				VectorTransform4X4CT( &Vert->Position, &Position->Position, &Frame->LocalWorldMatrix ) ;
			}
			else
			{
				// ブレンド行列の作成
				if( Position->BoneWeight[ 0 ].W == 1.0f )
				{
					BMat = Frame->UseSkinBoneMatrix[ Position->BoneWeight[ 0 ].Index ] ;
					VectorTransform4X4CT( &Vert->Position, &Position->Position, BMat ) ;
				}
				else
				{
					Blend = Position->BoneWeight ;
					Weight = Blend->W ;
					BMat = Frame->UseSkinBoneMatrix[ Blend->Index ] ;
					UnSafeMatrix4X4CT_C_Eq_C_Mul_S( &BlendMat, BMat, Weight ) ;

					Blend ++ ;
					for( m = 1 ; m < MaxWeightNum && Blend->Index != -1 ; m ++, Blend ++ )
					{
						Weight = Blend->W ;
						if( Weight == 0.0f ) continue ;

						BMat = Frame->UseSkinBoneMatrix[ Blend->Index ] ;
						UnSafeMatrix4X4CT_C_EqPlus_C_Mul_S( &BlendMat, BMat, Weight ) ;
					}

					// 変換座標の作成
					VectorTransform4X4CT( &Vert->Position, &Position->Position, &BlendMat ) ;
				}
			}
		}

		// 頂点データの数を増やす
		DestBuffer->VertexNum += Frame->BaseData->PositionNum ;
	}
	else
	{
		Mesh = Frame->BaseData->Mesh ;
		for( i = 0 ; i < Frame->BaseData->MeshNum ; i ++, Mesh ++ )
		{
			TList = Mesh->TriangleList ;
			for( k = 0 ; k < Mesh->TriangleListNum ; k ++, TList ++ )
			{
				// 頂点情報の変換処理
				VertNum = TList->VertexNum ;
				switch( TList->VertexType )
				{
				case MV1_VERTEX_TYPE_NORMAL :
					Norm = ( MV1_TLIST_NORMAL_POS * )ADDR16( TList->NormalPosition ) ;
					for( l = 0 ; l < VertNum ; l ++, Vert ++, Norm ++ )
					{
						VectorTransform4X4CT(   &Vert->Position, ( VECTOR * )&Norm->Position, &Frame->LocalWorldMatrix ) ;
						VectorTransformSR4X4CT( &Vert->Normal,   ( VECTOR * )&Norm->Normal,   &Frame->LocalWorldMatrix ) ;
					}
					break ;

				case MV1_VERTEX_TYPE_SKIN_4BONE :
					SkinB4 = ( MV1_TLIST_SKIN_POS_4B * )ADDR16( TList->SkinPosition4B ) ;
					for( l = 0 ; l < VertNum ; l ++, Vert ++, SkinB4 ++ )
					{
						// ブレンド行列の作成
						BMat = Frame->UseSkinBoneMatrix[ TList->UseBone[ SkinB4->MatrixIndex[ 0 ] ] ] ;
						Weight = SkinB4->MatrixWeight[ 0 ] ;
						if( Weight == 1.0f )
						{
							// 変換座標の作成
							VectorTransform4X4CT(   &Vert->Position, ( VECTOR * )&SkinB4->Position, BMat ) ;
							VectorTransformSR4X4CT( &Vert->Normal,   ( VECTOR * )&SkinB4->Normal,   BMat ) ;
						}
						else
						{
							UnSafeMatrix4X4CT_C_Eq_C_Mul_S( &BlendMat, BMat, Weight ) ;
							for( m = 1 ; m < 4 ; m ++ )
							{
								Weight = SkinB4->MatrixWeight[ m ] ;
								if( Weight == 0.0f ) continue ;

								BMat = Frame->UseSkinBoneMatrix[ TList->UseBone[ SkinB4->MatrixIndex[ m ] ] ] ;
								UnSafeMatrix4X4CT_C_EqPlus_C_Mul_S( &BlendMat, BMat, Weight ) ;
							}

							// 変換座標の作成
							VectorTransform4X4CT(   &Vert->Position, ( VECTOR * )&SkinB4->Position, &BlendMat ) ;
							VectorTransformSR4X4CT( &Vert->Normal,   ( VECTOR * )&SkinB4->Normal,   &BlendMat ) ;
						}
					}
					break ;

				case MV1_VERTEX_TYPE_SKIN_8BONE :
					SkinB8 = ( MV1_TLIST_SKIN_POS_8B * )ADDR16( TList->SkinPosition8B ) ;
					for( l = 0 ; l < VertNum ; l ++, Vert ++, SkinB8 ++ )
					{
						// ブレンド行列の作成
						BMat = Frame->UseSkinBoneMatrix[ TList->UseBone[ SkinB8->MatrixIndex1[ 0 ] ] ] ;
						Weight = SkinB8->MatrixWeight[ 0 ] ;
						if( Weight == 1.0f )
						{
							// 変換座標の作成
							VectorTransform4X4CT(   &Vert->Position, &SkinB8->Position, BMat ) ;
							VectorTransformSR4X4CT( &Vert->Normal,   &SkinB8->Normal,   BMat ) ;
						}
						else
						{
							UnSafeMatrix4X4CT_C_Eq_C_Mul_S( &BlendMat, BMat, Weight ) ;
							for( m = 1 ; m < 8 ; m ++ )
							{
								Weight = SkinB8->MatrixWeight[ m ] ;
								if( Weight == 0.0f ) continue ;

								BMat = Frame->UseSkinBoneMatrix[ TList->UseBone[ m < 4 ? SkinB8->MatrixIndex1[ m ] : SkinB8->MatrixIndex2[ m - 4 ] ] ] ;
								UnSafeMatrix4X4CT_C_EqPlus_C_Mul_S( &BlendMat, BMat, Weight ) ;
							}

							// 変換座標の作成
							VectorTransform4X4CT(   &Vert->Position, &SkinB8->Position, &BlendMat ) ;
							VectorTransformSR4X4CT( &Vert->Normal,   &SkinB8->Normal,   &BlendMat ) ;
						}
					}
					break ;

				case MV1_VERTEX_TYPE_SKIN_FREEBONE :
					SkinBF = ( MV1_TLIST_SKIN_POS_FREEB * )ADDR16( TList->SkinPositionFREEB ) ;
					PosUnitSize = TList->PosUnitSize ;
					for( l = 0 ; l < VertNum ; l ++, Vert ++, SkinBF = ( MV1_TLIST_SKIN_POS_FREEB * )( ( BYTE * )SkinBF + PosUnitSize ) )
					{
						// ブレンド行列の作成
						if( SkinBF->MatrixWeight[ 0 ].W == 1.0f )
						{
							BMat = Frame->UseSkinBoneMatrix[ SkinBF->MatrixWeight[ 0 ].Index ] ;
							VectorTransform4X4CT(   &Vert->Position, ( VECTOR * )&SkinBF->Position, BMat ) ;
							VectorTransformSR4X4CT( &Vert->Normal,   ( VECTOR * )&SkinBF->Normal,   BMat ) ;
						}
						else
						{
							Blend = SkinBF->MatrixWeight ;
							Weight = Blend->W ;
							BMat = Frame->UseSkinBoneMatrix[ Blend->Index ] ;
							UnSafeMatrix4X4CT_C_Eq_C_Mul_S( &BlendMat, BMat, Weight ) ;

							Blend ++ ;
							for( m = 1 ; m < TList->MaxBoneNum && Blend->Index != -1 ; m ++, Blend ++ )
							{
								Weight = Blend->W ;
								if( Weight == 0.0f ) continue ;

								BMat = Frame->UseSkinBoneMatrix[ Blend->Index ] ;
								UnSafeMatrix4X4CT_C_EqPlus_C_Mul_S( &BlendMat, BMat, Weight ) ;
							}

							// 変換座標の作成
							VectorTransform4X4CT(   &Vert->Position, ( VECTOR * )&SkinBF->Position, &BlendMat ) ;
							VectorTransformSR4X4CT( &Vert->Normal,   ( VECTOR * )&SkinBF->Normal,   &BlendMat ) ;
						}
					}
					break ;
				}

				// 頂点データの数を増やす
				DestBuffer->VertexNum += TList->VertexNum ;
			}
		}
	}

	// 終了
	return 0 ;
}

// 参照用メッシュの更新
extern int NS_MV1RefreshReferenceMesh( int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_REF_POLYGONLIST *PolyList ;
	int i ;
	bool Change ;

	IsTransform = IsTransform != 0 ? 1 : 0 ;
	IsPositionOnly = IsPositionOnly != 0 ? 1 : 0 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		return -1 ;
	ModelBase = Model->BaseData ;

	// 行列のセットアップ
	MV1SETUPMATRIX( Model ) ;

	// フレーム単体かモデル全体かで処理を分岐
	Change = false ;
	if( FrameIndex == -1 )
	{
		// モデル全体の場合

		// セットアップがされていなかったらセットアップ
		if( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
		{
			Change = true ;
			if( NS_MV1SetupReferenceMesh( MHandle, FrameIndex, IsTransform, IsPositionOnly ) < 0 )
				return -1 ;
		}

		// データを更新する必要が無い場合は更新しない
		if( Model->SetupRefPolygon[ IsTransform ][ IsPositionOnly ] )
			return 0 ;

		// 更新するポリゴン配列の先頭アドレスをセット
		PolyList = Model->RefPolygon[ IsTransform ][ IsPositionOnly ] ;

		// 更新完了フラグを立てる
		Model->SetupRefPolygon[ IsTransform ][ IsPositionOnly ] = true ;

		// 頂点変換が必要かどうかで処理を分岐
		if( IsTransform )
		{
			// コリジョンの更新完了フラグを倒す
			Model->SetupCollision = false ;

			// 全てのフレームのポリゴンを構築
			PolyList->VertexNum = 0 ;
			for( i = 0 ; i < ModelBase->FrameNum ; i ++ )
				MV1RefreshReferenceMeshFrame( Model->Frame + i, IsPositionOnly, PolyList ) ;

			Change = true ;
		}
	}
	else
	{
		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			return -1 ;
		Frame = Model->Frame + FrameIndex ;

		// セットアップがされていなかったらセットアップ
		if( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
		{
			Change = true ;
			if( NS_MV1SetupReferenceMesh( MHandle, FrameIndex, IsTransform, IsPositionOnly ) < 0 )
				return -1 ;
		}

		// データを更新する必要が無い場合は更新しない
		if( Frame->SetupRefPolygon[ IsTransform ][ IsPositionOnly ] )
			return 0 ;

		// 更新するポリゴン配列の先頭アドレスをセット
		PolyList = Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] ;

		// 更新完了フラグを立てる
		Frame->SetupRefPolygon[ IsTransform ][ IsPositionOnly ] = true ;

		// 頂点変換が必要かどうかで処理を分岐
		if( IsTransform )
		{
			// コリジョンの更新完了フラグを倒す
			Frame->SetupCollision = false ;

			// フレームのポリゴンを構築
			PolyList->VertexNum = 0 ;
			MV1RefreshReferenceMeshFrame( Frame, IsPositionOnly, PolyList ) ;

			Change = true ;
		}
	}

	// ポリゴンの最大座標値、最小座標値の更新
	if( PolyList->PolygonNum && Change )
		_MV1SetupReferenceMeshMaxAndMinPosition( PolyList ) ;

	// 終了
	return 0 ;
}

// 参照用メッシュを取得する
extern MV1_REF_POLYGONLIST NS_MV1GetReferenceMesh( int MHandle, int FrameIndex, int IsTransform, int IsPositionOnly )
{
	MV1_MODEL *Model ;
	MV1_MODEL_BASE *ModelBase ;
	MV1_FRAME *Frame ;
	MV1_REF_POLYGONLIST PolyList ;

	IsTransform = IsTransform != 0 ? 1 : 0 ;
	IsPositionOnly = IsPositionOnly != 0 ? 1 : 0 ;

	// アドレス取得
	if( MV1MDLCHK( MHandle, Model ) )
		goto ERR ;
	ModelBase = Model->BaseData ;

	// フレームインデックスが -1 だったらモデル全体
	if( FrameIndex == -1 )
	{
		// セットアップがされていなかった場合はセットアップを行う
		if( Model->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
			if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, IsTransform, IsPositionOnly ) < 0 )
				goto ERR ;

		return *Model->RefPolygon[ IsTransform ][ IsPositionOnly ] ;

/*
		// 頂点変換が必要かどうかで処理を分岐
		if( IsTransform )
		{
			// セットアップがされていなかった場合はセットアップを行う
			if( Model->TransformPolygon == NULL )
				if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, IsTransform ) < 0 )
					return PolyList ;

			return *Model->TransformPolygon ;
		}
		else
		{
			// セットアップがされていなかった場合はセットアップを行う
			if( Model->NonTransformPolygon == NULL )
				if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, IsTransform ) < 0 )
					return PolyList ;

			return *Model->NonTransformPolygon ;
		}
*/
	}
	else
	{
		// フレームインデックスのチェック
		if( FrameIndex < 0 || FrameIndex >= ModelBase->FrameNum )
			goto ERR ;
		Frame = Model->Frame + FrameIndex ;

		// セットアップがされていなかった場合はセットアップを行う
		if( Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] == NULL )
			if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, IsTransform, IsPositionOnly ) < 0 )
				goto ERR ;

		return *Frame->RefPolygon[ IsTransform ][ IsPositionOnly ] ;
/*
		// 頂点変換が必要かどうかで処理を分岐
		if( IsTransform )
		{
			// セットアップがされていなかった場合はセットアップを行う
			if( Frame->TransformPolygon == NULL )
				if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, IsTransform ) < 0 )
					return PolyList ;

			return *Frame->TransformPolygon ;
		}
		else
		{
			// セットアップがされていなかった場合はセットアップを行う
			if( Frame->NonTransformPolygon == NULL )
				if( NS_MV1RefreshReferenceMesh( MHandle, FrameIndex, IsTransform ) < 0 )
					return PolyList ;

			return *Frame->NonTransformPolygon ;
		}
*/
	}

ERR :
	_MEMSET( &PolyList, 0, sizeof( PolyList ) ) ;
	return PolyList ;
}


#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif  // DX_NON_MODEL


