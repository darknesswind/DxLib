﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ＰＭＸモデルデータ読み込みプログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

/*

本ソースを作成する際に参考にさせていただいたプログラムソース

PMX仕様
PMXエディタ付属 PMX仕様.txt

IK処理のプログラムソース
樋口M様
<< ikx.zip >>

*/

#define __DX_MAKE

#include "DxModelLoader4.h"

#ifndef DX_NON_MODEL

// インクルード ---------------------------------
#include "DxModelLoaderVMD.h"
#include "DxFile.h"
#include "DxLog.h"
#include "DxMemory.h"
#include "DxSystem.h"
#include "Windows/DxGuid.h"

namespace DxLib
{

// マクロ定義 -----------------------------------

// データ宣言 -----------------------------------


// 関数宣言 -------------------------------------

// ＶＭＤファイルを読み込む( -1:エラー )
static int _MV1LoadModelToVMD_PMX(
	int								DataIndex,
	MV1_MODEL_R *					RModel,
	void *							DataBuffer,
	int								DataSize,
	const char *					Name,
	PMX_READ_BONE_INFO *			PmxBoneInfo,
	int								PmxBoneNum,
	PMX_READ_IK_INFO *				PmxIKInfoFirst,
#ifndef DX_NON_BULLET_PHYSICS
	DX_MODELLOADER3_PMX_PHYSICS_INFO *	MLPhysicsInfo,
#endif
	bool							FPS60
) ;

// テキストデータをシフトJISに変換して取得
__inline void MV1LoadModelToPMX_GetString( BYTE **Src, char *DestBuffer, BYTE EncodeType )
{
	char Buffer[ 1024 ] ;
	wchar_t WBuffer[ 1024 ] ;
	int TextSize ;

	TextSize = *( ( DWORD * )*Src ) ;
	*Src = *Src + 4 ;

	if( EncodeType == 0 )
	{
		// UTF16

		_MEMCPY( WBuffer, *Src, TextSize ) ;
		( ( BYTE * )WBuffer )[ TextSize     ] = 0 ;
		( ( BYTE * )WBuffer )[ TextSize + 1 ] = 0 ;
		*Src = *Src + TextSize ;

		// ワイド文字をシフトJISに変換
		WCharToMBChar( 932, ( DXWCHAR * )WBuffer, DestBuffer,  512 ) ;
	}
	else
	if( EncodeType == 1 )
	{
		// UTF8

		_MEMCPY( Buffer, *Src, TextSize ) ;
		Buffer[ TextSize ] = '\0' ;
		*Src = *Src + TextSize ;

		// 最初に UTF-8 をワイド文字に変換
		MBCharToWChar( CP_UTF8,  Buffer, ( DXWCHAR * )WBuffer,    1024 ) ;

		// ワイド文字をシフトJISに変換
		WCharToMBChar( 932, ( DXWCHAR * )WBuffer, DestBuffer,  512 ) ;
	}
}

// 整数値サイズに合わせた整数値を取得する(符号あり)
__inline int MV1LoadModelToPMX_GetInt( BYTE **Src, BYTE Size )
{
	int res = 0 ;

	switch( Size )
	{
	case 1 :
		res = ( char )( ( *Src )[ 0 ] ) ;
		*Src = *Src + 1 ;
		break ;

	case 2 :
		res = ( short )( ( ( WORD * )*Src )[ 0 ] ) ;
		*Src = *Src + 2 ;
		break ;

	case 4 :
		res = ( ( int * )*Src )[ 0 ] ;
		*Src = *Src + 4 ;
		break ;
	}

	return res ;
}

// 整数値サイズに合わせた整数値を取得する(32bit以外符号なし)
__inline int MV1LoadModelToPMX_GetUInt( BYTE **Src, BYTE Size )
{
	int res = 0 ;

	switch( Size )
	{
	case 1 :
		res = ( int )( ( *Src )[ 0 ] ) ;
		*Src = *Src + 1 ;
		break ;

	case 2 :
		res = ( int )( ( ( WORD * )*Src )[ 0 ] ) ;
		*Src = *Src + 2 ;
		break ;

	case 4 :
		res = ( ( int * )*Src )[ 0 ] ;
		*Src = *Src + 4 ;
		break ;
	}

	return res ;
}

// 行列を計算する
static void MV1LoadModelToPMX_SetupMatrix( PMX_READ_BONE_INFO *BoneInfo, int BoneNum, int UseInitParam, int IKSkip ) ;

// ＩＫを計算する
static void MV1LoadModelToPMX_SetupIK( PMX_READ_BONE_INFO *BoneInfo, PMX_READ_IK_INFO *IKInfoFirst ) ;

// 指定のボーンにアニメーションの指定キーのパラメータを反映させる
static void MV1LoadModelToPMX_SetupOneBoneMatrixFormAnimKey( PMX_READ_BONE_INFO *BoneInfo, int Time, int LoopNo ) ;

// プログラム -----------------------------------

// ＰＭＸファイルを読み込む( -1:エラー  0以上:モデルハンドル )
extern int MV1LoadModelToPMX( const MV1_MODEL_LOAD_PARAM *LoadParam, int ASyncThread )
{
	int NewHandle = -1 ;
	int ErrorFlag = 1 ;
	int i, j, k, weightcount, facecount ;
	PMX_VERTEX *PmxVertex ;
	DWORD PmxVertexNum ;
	PMX_FACE *PmxFace ;
	DWORD PmxTextureNum ;
	PMX_TEXTURE *PmxTexture ;
	DWORD PmxFaceNum ;
	PMX_MATERIAL *PmxMaterial ;
	PMX_READ_IK_INFO *IKInfoDim = NULL, *IKInfoFirst = NULL, *IKInfo ;
	PMX_READ_BONE_INFO *BoneInfoDim = NULL, *BoneInfo ;
	DWORD PmxMaterialNum ;
	DWORD PmxBoneNum = 0 ;
	PMX_BONE *PmxBone ;
	DWORD PmxMorphNum ;
	PMX_MORPH *PmxMorph ;
	DWORD PmxRigidbodyNum ;
	PMX_RIGIDBODY *PmxRigidbody ;
	DWORD PmxJointNum ;
	PMX_JOINT *PmxJoint ;
	DWORD PmxIKNum ;
	DWORD PmxSkinNum ;
	MV1_MODEL_R RModel ;
	MV1_TEXTURE_R *Texture ;
	MV1_MATERIAL_R *Material ;
	MV1_SKIN_WEIGHT_R *SkinWeight, *SkinWeightTemp ;
	MV1_SKIN_WEIGHT_ONE_R *SkinW, *SkinWTemp ;
	MV1_FRAME_R *Frame ;
	char FrameDimEnable[ 1024 ] ;
	MV1_FRAME_R *FrameDim[ 1024 ] ;
	char FrameSkipDim[ 1024 ] ;
	int FrameSkipNum ;
	MV1_MESH_R *Mesh, *SkinMesh ;
	MV1_MESHFACE_R *MeshFace, *MeshFaceTemp ;
	MV1_SHAPE_R *Shape ;
	MV1_SHAPE_VERTEX_R *ShapeVert ;
	DWORD *SkinNextVertIndex = NULL, *SkinPrevVertIndex ;
	DWORD *SkinNextFaceIndex, *SkinPrevFaceIndex ;
	DWORD *SkinNextMaterialIndex, *SkinPrevMaterialIndex ;
	DWORD SkinVertNum, SkinBaseVertNum, SkinFaceNum, NextFaceNum, SkinMaterialNum ;
	char String[ 1024 ] ;
	BYTE *Src ;
	int ValidPhysics = FALSE ;
#ifndef DX_NON_BULLET_PHYSICS
	DX_MODELLOADER3_PMX_PHYSICS_INFO MLPhysicsInfo ;

	_MEMSET( &MLPhysicsInfo, 0, sizeof( MLPhysicsInfo ) ) ;
#endif
	PMX_BASEINFO PmxInfo ;
	BYTE AddHeadDataSize ;

	// 読み込みようデータの初期化
	MV1InitReadModel( &RModel ) ;
	RModel.MaterialNumberOrderDraw = TRUE ;
	RModel.MeshFaceRightHand = FALSE ;

	// Pmxモデルデータの情報をセット
	Src = ( BYTE * )LoadParam->DataBuffer ;

	// PMXファイルかどうかを確認
	if( Src[ 0 ] != 'P' || Src[ 1 ] != 'M' || Src[ 2 ] != 'X' || Src[ 3 ] != ' ' )
		return -1 ;
	Src += 4 ;

	// Ver2.0 かを確認
	if( Src[ 0 ] != 0x00 || Src[ 1 ] != 0x00 || Src[ 2 ] != 0x00 || Src[ 3 ] != 0x40 )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : バージョン２．０ではありません\n" ) ) ) ;
		return -1 ;
	}
	Src += 4 ;

	// 追加情報のサイズを取得
	AddHeadDataSize = Src[ 0 ] ;
	Src ++ ;

	// 追加情報を取得
	PmxInfo.EncodeType        = Src[ 0 ] ;
	PmxInfo.UVNum             = Src[ 1 ] ;
	PmxInfo.VertexIndexSize   = Src[ 2 ] ;
	PmxInfo.TextureIndexSize  = Src[ 3 ] ;
	PmxInfo.MaterialIndexSize = Src[ 4 ] ;
	PmxInfo.BoneIndexSize     = Src[ 5 ] ;
	PmxInfo.MorphIndexSize    = Src[ 6 ] ;
	PmxInfo.RigidIndexSize    = Src[ 7 ] ;
	Src += AddHeadDataSize ;

	// モデル情報スキップ
	Src += *( ( DWORD * )Src ) + 4 ;
	Src += *( ( DWORD * )Src ) + 4 ;
	Src += *( ( DWORD * )Src ) + 4 ;
	Src += *( ( DWORD * )Src ) + 4 ;

	// モデル名とファイル名をセット
	RModel.FilePath = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->FilePath ) + 1 ) * sizeof( TCHAR ) ) ;
	RModel.Name     = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->Name     ) + 1 ) * sizeof( TCHAR ) ) ;
	lstrcpy( RModel.FilePath, LoadParam->FilePath ) ;
	lstrcpy( RModel.Name,     LoadParam->Name ) ;

	// 法泉の自動生成は使用しない
	RModel.AutoCreateNormal = FALSE ;

	// 頂点数を取得
	PmxVertexNum = *( ( DWORD * )Src ) ;
	Src += 4 ;

	// 頂点データを格納するメモリ領域の確保
	PmxVertex = ( PMX_VERTEX * )ADDMEMAREA( sizeof( PMX_VERTEX ) * PmxVertexNum, &RModel.Mem ) ;
	if( PmxVertex == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 頂点データの一時記憶領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// 頂点データを取得
	for( i = 0 ; ( DWORD )i < PmxVertexNum ; i ++ )
	{
		*( ( DWORD * )&PmxVertex[ i ].Position[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&PmxVertex[ i ].Position[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&PmxVertex[ i ].Position[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
		*( ( DWORD * )&PmxVertex[ i ].Normal[ 0 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
		*( ( DWORD * )&PmxVertex[ i ].Normal[ 1 ] ) = *( ( DWORD * )&Src[ 16 ] ) ;
		*( ( DWORD * )&PmxVertex[ i ].Normal[ 2 ] ) = *( ( DWORD * )&Src[ 20 ] ) ;
		*( ( DWORD * )&PmxVertex[ i ].UV[ 0 ] ) = *( ( DWORD * )&Src[ 24 ] ) ;
		*( ( DWORD * )&PmxVertex[ i ].UV[ 1 ] ) = *( ( DWORD * )&Src[ 28 ] ) ;

		Src += 32 ;
		for( j = 0 ; j < PmxInfo.UVNum ; j ++ )
		{
			*( ( DWORD * )&PmxVertex[ i ].AddUV[ j ][ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].AddUV[ j ][ 0 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].AddUV[ j ][ 0 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].AddUV[ j ][ 0 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
			Src += 16 ;
		}

		PmxVertex[ i ].WeightType = Src[ 0 ] ;
		Src ++ ;

		PmxVertex[ i ].BoneIndex[ 0 ] = -1 ;
		PmxVertex[ i ].BoneIndex[ 1 ] = -1 ;
		PmxVertex[ i ].BoneIndex[ 2 ] = -1 ;
		PmxVertex[ i ].BoneIndex[ 3 ] = -1 ;
		switch( PmxVertex[ i ].WeightType )
		{
		case 0 :	// BDEF1
			PmxVertex[ i ].BoneIndex[ 0 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			PmxVertex[ i ].BoneWeight[ 0 ] = 1.0f ;
			break ;

		case 1 :	// BDEF2
			PmxVertex[ i ].BoneIndex[ 0 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			PmxVertex[ i ].BoneIndex[ 1 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			*( ( DWORD * )&PmxVertex[ i ].BoneWeight[ 0 ] ) = *( ( DWORD * )Src ) ;
			Src += 4 ;
			PmxVertex[ i ].BoneWeight[ 1 ] = 1.0f - PmxVertex[ i ].BoneWeight[ 0 ] ;
			break ;

		case 2 :	// BDEF4
			PmxVertex[ i ].BoneIndex[ 0 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			PmxVertex[ i ].BoneIndex[ 1 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			PmxVertex[ i ].BoneIndex[ 2 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			PmxVertex[ i ].BoneIndex[ 3 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			*( ( DWORD * )&PmxVertex[ i ].BoneWeight[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].BoneWeight[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].BoneWeight[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].BoneWeight[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
			Src += 16 ;
			break ;

		case 3 :	// SDEF
			PmxVertex[ i ].BoneIndex[ 0 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			PmxVertex[ i ].BoneIndex[ 1 ] = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			*( ( DWORD * )&PmxVertex[ i ].BoneWeight[ 0 ] ) = *( ( DWORD * )Src ) ;
			Src += 4 ;
			PmxVertex[ i ].BoneWeight[ 1 ] = 1.0f - PmxVertex[ i ].BoneWeight[ 0 ] ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_C[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_C[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_C[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_R0[ 0 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_R0[ 1 ] ) = *( ( DWORD * )&Src[ 16 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_R0[ 2 ] ) = *( ( DWORD * )&Src[ 20 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_R1[ 0 ] ) = *( ( DWORD * )&Src[ 24 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_R1[ 1 ] ) = *( ( DWORD * )&Src[ 28 ] ) ;
			*( ( DWORD * )&PmxVertex[ i ].SDEF_R1[ 2 ] ) = *( ( DWORD * )&Src[ 32 ] ) ;
			Src += 36 ;
			break ;
		}

		*( ( DWORD * )&PmxVertex[ i ].ToonEdgeScale ) = *( ( DWORD * )&Src[ 0 ] ) ;
		Src += 4 ;
	}

	// 面の数を取得
	PmxFaceNum = *( ( DWORD * )Src ) / 3 ;
	Src += 4 ;

	// 面データを格納するメモリ領域の確保
	PmxFace = ( PMX_FACE * )ADDMEMAREA( sizeof( PMX_FACE ) * PmxFaceNum, &RModel.Mem ) ;
	if( PmxFace == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 面データの一時記憶領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// 面を構成する頂点インデックスの取得
	switch( PmxInfo.VertexIndexSize )
	{
	case 1 :
		for( i = 0 ; ( DWORD )i < PmxFaceNum ; i ++ )
		{
			PmxFace[ i ].VertexIndex[ 0 ] = ( int )( Src[ 0 ] ) ;
			PmxFace[ i ].VertexIndex[ 1 ] = ( int )( Src[ 1 ] ) ;
			PmxFace[ i ].VertexIndex[ 2 ] = ( int )( Src[ 2 ] ) ;
			Src += 3 ;
		}
		break ;

	case 2 :
		for( i = 0 ; ( DWORD )i < PmxFaceNum ; i ++ )
		{
			PmxFace[ i ].VertexIndex[ 0 ] = ( int )( *( ( WORD * )&Src[ 0 ] ) ) ;
			PmxFace[ i ].VertexIndex[ 1 ] = ( int )( *( ( WORD * )&Src[ 2 ] ) ) ;
			PmxFace[ i ].VertexIndex[ 2 ] = ( int )( *( ( WORD * )&Src[ 4 ] ) ) ;
			Src += 6 ;
		}
		break ;

	case 4 :
		for( i = 0 ; ( DWORD )i < PmxFaceNum ; i ++ )
		{
			PmxFace[ i ].VertexIndex[ 0 ] = *( ( int * )&Src[ 0 ] ) ;
			PmxFace[ i ].VertexIndex[ 1 ] = *( ( int * )&Src[ 4 ] ) ;
			PmxFace[ i ].VertexIndex[ 2 ] = *( ( int * )&Src[ 8 ] ) ;
			Src += 12 ;
		}
		break ;
	}


	// テクスチャの数を取得
	PmxTextureNum = *( ( DWORD * )Src ) ;
	Src += 4 ;

	// テクスチャデータを格納するメモリ領域の確保
	PmxTexture = ( PMX_TEXTURE * )ADDMEMAREA( sizeof( PMX_TEXTURE ) * PmxTextureNum, &RModel.Mem ) ;
	if( PmxTexture == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : テクスチャデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// テクスチャの情報を取得
	for( i = 0 ; ( DWORD )i < PmxTextureNum ; i ++ )
	{
		MV1LoadModelToPMX_GetString( &Src, PmxTexture[ i ].TexturePath, PmxInfo.EncodeType ) ;
	}


	// マテリアルの数を取得
	PmxMaterialNum = *( ( DWORD * )Src ) ;
	Src += 4 ;

	// マテリアルデータを格納するメモリ領域の確保
	PmxMaterial = ( PMX_MATERIAL * )ADDMEMAREA( sizeof( PMX_MATERIAL ) * PmxMaterialNum, &RModel.Mem ) ;
	if( PmxMaterial == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : マテリアルデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// マテリアルの読みこみ
	for( i = 0 ; ( DWORD )i < PmxMaterialNum ; i ++ )
	{
		// 材質名の取得
		MV1LoadModelToPMX_GetString( &Src, PmxMaterial[ i ].Name, PmxInfo.EncodeType ) ;
		if( PmxMaterial[ i ].Name[ 0 ] == '\0' )
		{
			MV1LoadModelToPMX_GetString( &Src, PmxMaterial[ i ].Name, PmxInfo.EncodeType ) ;
			if( PmxMaterial[ i ].Name[ 0 ] == '\0' )
			{
				_SPRINTF( PmxMaterial[ i ].Name, "Mat_%d", i ) ;
			}
		}
		else
		{
			Src += *( ( DWORD * )Src ) + 4 ;
		}

		*( ( DWORD * )&PmxMaterial[ i ].Diffuse[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].Diffuse[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].Diffuse[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].Diffuse[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
		Src += 16 ;

		*( ( DWORD * )&PmxMaterial[ i ].Specular[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].Specular[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].Specular[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
		Src += 12 ;

		*( ( DWORD * )&PmxMaterial[ i ].SpecularPower ) = *( ( DWORD * )&Src[ 0 ] ) ;
		Src += 4 ;

		*( ( DWORD * )&PmxMaterial[ i ].Ambient[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].Ambient[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].Ambient[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
		Src += 12 ;

		PmxMaterial[ i ].CullingOff     = ( *Src & 0x01 ) ? 1 : 0 ;
		PmxMaterial[ i ].GroundShadow   = ( *Src & 0x02 ) ? 1 : 0 ;
		PmxMaterial[ i ].SelfShadowMap  = ( *Src & 0x04 ) ? 1 : 0 ;
		PmxMaterial[ i ].SelfShadowDraw = ( *Src & 0x08 ) ? 1 : 0 ;
		PmxMaterial[ i ].EdgeDraw       = ( *Src & 0x10 ) ? 1 : 0 ;
		Src ++ ;

		*( ( DWORD * )&PmxMaterial[ i ].EdgeColor[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].EdgeColor[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].EdgeColor[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
		*( ( DWORD * )&PmxMaterial[ i ].EdgeColor[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
		Src += 16 ;

		*( ( DWORD * )&PmxMaterial[ i ].EdgeSize ) = *( ( DWORD * )Src ) ;
		Src += 4 ;

		PmxMaterial[ i ].TextureIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.TextureIndexSize ) ;
		PmxMaterial[ i ].SphereTextureIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.TextureIndexSize ) ;

		PmxMaterial[ i ].SphereMode = *Src ;
		Src ++ ;

		PmxMaterial[ i ].ToonFlag = *Src ;
		Src ++ ;
		if( PmxMaterial[ i ].ToonFlag == 0 )
		{
			PmxMaterial[ i ].ToonTextureIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.TextureIndexSize ) ;
		}
		else
		{
			PmxMaterial[ i ].ToonTextureIndex = *Src ;
			Src ++ ;
		}

		// メモはスキップ
		Src += *( ( DWORD * )Src ) + 4 ;

		PmxMaterial[ i ].MaterialFaceNum = *( ( int * )Src ) ;
		Src += 4 ;
	}


	// ボーンの数を取得
	PmxBoneNum = *( ( DWORD * )Src ) ;
	Src += 4 ;

	// ボーンデータを格納するメモリ領域の確保
	PmxBone = ( PMX_BONE * )ADDMEMAREA( sizeof( PMX_BONE ) * PmxBoneNum, &RModel.Mem ) ;
	if( PmxBone == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : ボーンデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// ボーン情報の取得
	PmxIKNum = 0 ;
	for( i = 0 ; ( DWORD )i < PmxBoneNum ; i ++ )
	{
		MV1LoadModelToPMX_GetString( &Src, PmxBone[ i ].Name, PmxInfo.EncodeType ) ;
		if( PmxBone[ i ].Name[ 0 ] == '\0' ) 
		{
			MV1LoadModelToPMX_GetString( &Src, PmxBone[ i ].Name, PmxInfo.EncodeType ) ;
		}
		else
		{
			Src += *( ( DWORD * )Src ) + 4 ;
		}

		*( ( DWORD * )&PmxBone[ i ].Position[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&PmxBone[ i ].Position[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&PmxBone[ i ].Position[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
		Src += 12 ;

		PmxBone[ i ].ParentBoneIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
		PmxBone[ i ].TransformLayer = *( ( int * )Src ) ;
		Src += 4 ;

		WORD Flag = *( ( WORD * )Src ) ;
		Src += 2 ;
		PmxBone[ i ].Flag_LinkDest              = ( Flag & 0x0001 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_EnableRot             = ( Flag & 0x0002 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_EnableMov             = ( Flag & 0x0004 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_Disp                  = ( Flag & 0x0008 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_EnableControl         = ( Flag & 0x0010 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_IK                    = ( Flag & 0x0020 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_AddRot                = ( Flag & 0x0100 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_AddMov                = ( Flag & 0x0200 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_LockAxis              = ( Flag & 0x0400 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_LocalAxis             = ( Flag & 0x0800 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_AfterPhysicsTransform = ( Flag & 0x1000 ) != 0 ? 1 : 0 ;
		PmxBone[ i ].Flag_OutParentTransform    = ( Flag & 0x2000 ) != 0 ? 1 : 0 ;

		if( PmxBone[ i ].Flag_LinkDest == 0 )
		{
			*( ( DWORD * )&PmxBone[ i ].OffsetPosition[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].OffsetPosition[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].OffsetPosition[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
			Src += 12 ;
		}
		else
		{
			PmxBone[ i ].LinkBoneIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
		}

		if( PmxBone[ i ].Flag_AddRot == 1 || PmxBone[ i ].Flag_AddMov == 1 )
		{
			PmxBone[ i ].AddParentBoneIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			*( ( DWORD * )&PmxBone[ i ].AddRatio ) = *( ( DWORD * )Src ) ;
			Src += 4 ;
		}

		if( PmxBone[ i ].Flag_LockAxis == 1 )
		{
			*( ( DWORD * )&PmxBone[ i ].LockAxisVector[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].LockAxisVector[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].LockAxisVector[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
			Src += 12 ;
		}

		if( PmxBone[ i ].Flag_LocalAxis == 1 )
		{
			*( ( DWORD * )&PmxBone[ i ].LocalAxisXVector[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].LocalAxisXVector[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].LocalAxisXVector[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxBone[ i ].LocalAxisZVector[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].LocalAxisZVector[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
			*( ( DWORD * )&PmxBone[ i ].LocalAxisZVector[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
			Src += 12 ;
		}

		if( PmxBone[ i ].Flag_OutParentTransform == 1 )
		{
			PmxBone[ i ].OutParentTransformKey = *( ( int * )Src ) ;
			Src += 4 ;
		}

		if( PmxBone[ i ].Flag_IK == 1 )
		{
			PmxIKNum ++ ;

			PmxBone[ i ].IKInfo.TargetBoneIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
			PmxBone[ i ].IKInfo.LoopNum = *( ( int * )Src ) ;
			Src += 4 ;

			*( ( DWORD * )&PmxBone[ i ].IKInfo.RotLimit ) = *( ( DWORD * )Src ) ;
			Src += 4 ;

			PmxBone[ i ].IKInfo.LinkNum = *( ( int * )Src ) ;
			Src += 4 ;
			if( PmxBone[ i ].IKInfo.LinkNum >= PMX_MAX_IKLINKNUM )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : ＩＫリンクの数が対応数を超えています\n" ) ) ) ;
				goto ENDLABEL ;
			}

			for( j = 0 ; j < PmxBone[ i ].IKInfo.LinkNum ; j ++ )
			{
				PmxBone[ i ].IKInfo.Link[ j ].BoneIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
				PmxBone[ i ].IKInfo.Link[ j ].RotLockFlag = Src[ 0 ] ;
				Src ++ ;

				if( PmxBone[ i ].IKInfo.Link[ j ].RotLockFlag == 1 )
				{
					*( ( DWORD * )&PmxBone[ i ].IKInfo.Link[ j ].RotLockMin[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
					*( ( DWORD * )&PmxBone[ i ].IKInfo.Link[ j ].RotLockMin[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
					*( ( DWORD * )&PmxBone[ i ].IKInfo.Link[ j ].RotLockMin[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
					Src += 12 ;

					*( ( DWORD * )&PmxBone[ i ].IKInfo.Link[ j ].RotLockMax[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
					*( ( DWORD * )&PmxBone[ i ].IKInfo.Link[ j ].RotLockMax[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
					*( ( DWORD * )&PmxBone[ i ].IKInfo.Link[ j ].RotLockMax[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
					Src += 12 ;
				}
			}
		}
	}


	// モーフ情報の数を取得
	PmxMorphNum = *( ( DWORD * )Src ) ;
	Src += 4 ;

	// モーフデータを格納するメモリ領域の確保
	PmxMorph = ( PMX_MORPH * )ADDMEMAREA( sizeof( PMX_MORPH ) * PmxMorphNum, &RModel.Mem ) ;
	if( PmxMorph == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : モーフデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// モーフ情報の読み込み
	PmxSkinNum = 0 ;
	for( i = 0 ; ( DWORD )i < PmxMorphNum ; i ++ )
	{
		MV1LoadModelToPMX_GetString( &Src, PmxMorph[ i ].Name, PmxInfo.EncodeType ) ;
		if( PmxMorph[ i ].Name[ 0 ] == '\0' ) 
		{
			MV1LoadModelToPMX_GetString( &Src, PmxMorph[ i ].Name, PmxInfo.EncodeType ) ;
		}
		else
		{
			Src += *( ( DWORD * )Src ) + 4 ;
		}

		PmxMorph[ i ].ControlPanel = Src[ 0 ] ;
		PmxMorph[ i ].Type = Src[ 1 ] ;
		PmxMorph[ i ].DataNum = *( ( int * )&Src[ 2 ] ) ;
		Src += 6 ;

		switch( PmxMorph[ i ].Type )
		{
		case 0 :	// グループモーフ
			PmxMorph[ i ].Group = ( PMX_MORPH_GROUP * )ADDMEMAREA( sizeof( PMX_MORPH_GROUP ) * PmxMorph[ i ].DataNum, &RModel.Mem ) ;
			if( PmxMorph[ i ].Group == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : グループモーフデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			for( j = 0 ; j < PmxMorph[ i ].DataNum ; j ++ )
			{
				PmxMorph[ i ].Group[ j ].Index = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.MorphIndexSize ) ;
				*( ( DWORD * )&PmxMorph[ i ].Group[ j ].Ratio ) = *( ( DWORD * )Src ) ;
				Src += 4 ;
			}
			break ;

		case 1 :	// 頂点
			PmxSkinNum ++ ;
			PmxMorph[ i ].Vertex = ( PMX_MORPH_VERTEX * )ADDMEMAREA( sizeof( PMX_MORPH_VERTEX ) * PmxMorph[ i ].DataNum, &RModel.Mem ) ;
			if( PmxMorph[ i ].Vertex == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 頂点モーフデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			for( j = 0 ; j < PmxMorph[ i ].DataNum ; j ++ )
			{
				PmxMorph[ i ].Vertex[ j ].Index = MV1LoadModelToPMX_GetUInt( &Src, PmxInfo.VertexIndexSize ) ;
				*( ( DWORD * )&PmxMorph[ i ].Vertex[ j ].Offset[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Vertex[ j ].Offset[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Vertex[ j ].Offset[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
				Src += 12 ;
			}
			break ;

		case 2 :	// ボーンモーフ
			PmxMorph[ i ].Bone = ( PMX_MORPH_BONE * )ADDMEMAREA( sizeof( PMX_MORPH_BONE ) * PmxMorph[ i ].DataNum, &RModel.Mem ) ;
			if( PmxMorph[ i ].Bone == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : ボーンモーフデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			for( j = 0 ; j < PmxMorph[ i ].DataNum ; j ++ )
			{
				PmxMorph[ i ].Bone[ j ].Index = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
				*( ( DWORD * )&PmxMorph[ i ].Bone[ j ].Offset[ 0 ] ) = *( ( DWORD * )&Src[ 0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Bone[ j ].Offset[ 1 ] ) = *( ( DWORD * )&Src[ 4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Bone[ j ].Offset[ 2 ] ) = *( ( DWORD * )&Src[ 8 ] ) ;
				Src += 12 ;

				*( ( DWORD * )&PmxMorph[ i ].Bone[ j ].Quat[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Bone[ j ].Quat[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Bone[ j ].Quat[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Bone[ j ].Quat[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
				Src += 16 ;
			}
			break ;

		case 3 :	// UVモーフ
		case 4 :	// 追加UV1モーフ
		case 5 :	// 追加UV2モーフ
		case 6 :	// 追加UV3モーフ
		case 7 :	// 追加UV4モーフ
			PmxMorph[ i ].UV = ( PMX_MORPH_UV * )ADDMEMAREA( sizeof( PMX_MORPH_UV ) * PmxMorph[ i ].DataNum, &RModel.Mem ) ;
			if( PmxMorph[ i ].UV == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : UVモーフデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			for( j = 0 ; j < PmxMorph[ i ].DataNum ; j ++ )
			{
				PmxMorph[ i ].UV[ j ].Index = MV1LoadModelToPMX_GetUInt( &Src, PmxInfo.VertexIndexSize ) ;
				*( ( DWORD * )&PmxMorph[ i ].UV[ j ].Offset.x ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].UV[ j ].Offset.y ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].UV[ j ].Offset.z ) = *( ( DWORD * )&Src[  8 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].UV[ j ].Offset.w ) = *( ( DWORD * )&Src[ 12 ] ) ;
				Src += 16 ;
			}
			break ;

		case 8 :	// 材質モーフ
			PmxMorph[ i ].Material = ( PMX_MORPH_MATERIAL * )ADDMEMAREA( sizeof( PMX_MORPH_MATERIAL ) * PmxMorph[ i ].DataNum, &RModel.Mem ) ;
			if( PmxMorph[ i ].Material == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : マテリアルモーフデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			for( j = 0 ; j < PmxMorph[ i ].DataNum ; j ++ )
			{
				PmxMorph[ i ].Material[ j ].Index = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.MaterialIndexSize ) ;
				PmxMorph[ i ].Material[ j ].CalcType = Src[ 0 ] ;
				Src ++ ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Diffuse[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Diffuse[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Diffuse[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Diffuse[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
				Src += 16 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Specular[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Specular[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Specular[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				Src += 12 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].SpecularPower ) = *( ( DWORD * )&Src[  0 ] ) ;
				Src += 4 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Ambient[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Ambient[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].Ambient[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				Src += 12 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].EdgeColor[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].EdgeColor[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].EdgeColor[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].EdgeColor[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
				Src += 16 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].EdgeSize ) = *( ( DWORD * )&Src[  0 ] ) ;
				Src += 4 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].TextureScale[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].TextureScale[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].TextureScale[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].TextureScale[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
				Src += 16 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].SphereTextureScale[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].SphereTextureScale[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].SphereTextureScale[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].SphereTextureScale[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
				Src += 16 ;

				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].ToonTextureScale[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].ToonTextureScale[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].ToonTextureScale[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
				*( ( DWORD * )&PmxMorph[ i ].Material[ j ].ToonTextureScale[ 3 ] ) = *( ( DWORD * )&Src[ 12 ] ) ;
				Src += 16 ;
			}
			break ;
		}
	}


	// 表示枠データの読み込みはスキップ
	{
		DWORD PmxDispFrameNum ;
		DWORD Num ;
		BYTE Target ;

		// 表示枠データの数を取得
		PmxDispFrameNum = *( ( DWORD * )Src ) ;
		Src += 4 ;

		// 枠の数だけ繰り返し
		for( i = 0 ; ( DWORD )i < PmxDispFrameNum ; i ++ )
		{
			// 名前読み飛ばし
			Src += *( ( DWORD * )Src ) + 4 ;
			Src += *( ( DWORD * )Src ) + 4 ;

			// 特殊枠フラグ読み飛ばし
			Src ++ ;

			// 枠内要素の数を取得
			Num = *( ( DWORD * )Src ) ;
			Src += 4 ;
			
			// 枠内要素の読み飛ばし
			for( j = 0 ; ( DWORD )j < Num ; j ++ )
			{
				// 要素対象を取得
				Target = Src[ 0 ] ;
				Src ++ ;

				// 要素対象によって処理を分岐
				switch( Target )
				{
				case 0 :	// ボーンがターゲットの場合
					MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;
					break ;

				case 1 :	// モーフがターゲットの場合
					MV1LoadModelToPMX_GetInt( &Src, PmxInfo.MorphIndexSize ) ;
					break ;
				}
			}
		}
	}


	// 剛体情報の数を取得
	PmxRigidbodyNum = *( ( DWORD * )Src ) ;
	Src += 4 ;

	// 剛体データを格納するメモリ領域の確保
	PmxRigidbody = ( PMX_RIGIDBODY * )ADDMEMAREA( sizeof( PMX_RIGIDBODY ) * PmxRigidbodyNum, &RModel.Mem ) ;
	if( PmxRigidbody == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 剛体データの一時記憶領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// 剛体情報の読み込み
	for( i = 0 ; ( DWORD )i < PmxRigidbodyNum ; i++ )
	{
		MV1LoadModelToPMX_GetString( &Src, PmxRigidbody[ i ].Name, PmxInfo.EncodeType ) ;
		if( PmxRigidbody[ i ].Name[ 0 ] == '\0' ) 
		{
			MV1LoadModelToPMX_GetString( &Src, PmxRigidbody[ i ].Name, PmxInfo.EncodeType ) ;
		}
		else
		{
			Src += *( ( DWORD * )Src ) + 4 ;
		}

		PmxRigidbody[ i ].BoneIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.BoneIndexSize ) ;

		PmxRigidbody[ i ].RigidBodyGroupIndex = Src[ 0 ] ;
		Src ++ ;

		*( ( WORD * )&PmxRigidbody[ i ].RigidBodyGroupTarget ) = *( ( WORD * )&Src[ 0 ] ) ;
		Src += 2 ;

		PmxRigidbody[ i ].ShapeType = Src[ 0 ] ;
		Src ++ ;

		*( ( DWORD * )&PmxRigidbody[ i ].ShapeW ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].ShapeH ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].ShapeD ) = *( ( DWORD * )&Src[ 8 ] ) ;
		Src += 12 ;

		*( ( DWORD * )&PmxRigidbody[ i ].Position[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].Position[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].Position[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
		Src += 12 ;

		*( ( DWORD * )&PmxRigidbody[ i ].Rotation[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].Rotation[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].Rotation[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
		Src += 12 ;

		*( ( DWORD * )&PmxRigidbody[ i ].RigidBodyWeight ) = *( ( DWORD * )&Src[  0 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].RigidBodyPosDim ) = *( ( DWORD * )&Src[  4 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].RigidBodyRotDim ) = *( ( DWORD * )&Src[  8 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].RigidBodyRecoil ) = *( ( DWORD * )&Src[ 12 ] ) ;
		*( ( DWORD * )&PmxRigidbody[ i ].RigidBodyFriction ) = *( ( DWORD * )&Src[ 16 ] ) ;
		Src += 20 ;

		PmxRigidbody[ i ].RigidBodyType = Src[ 0 ] ;
		Src ++ ;
	}


	// ジョイント情報の数を取得
	PmxJointNum = *( ( DWORD * )Src ) ;
	Src += 4 ;

	// ジョイントデータを格納するメモリ領域の確保
	if( PmxJointNum != 0 )
	{
		PmxJoint = ( PMX_JOINT * )ADDMEMAREA( sizeof( PMX_JOINT ) * PmxJointNum, &RModel.Mem ) ;
		if( PmxJoint == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : ジョイントデータの一時記憶領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// ジョイント情報の読み込み
		for( i = 0 ; ( DWORD )i < PmxJointNum ; i ++ )
		{
			MV1LoadModelToPMX_GetString( &Src, PmxJoint[ i ].Name, PmxInfo.EncodeType ) ;
			if( PmxJoint[ i ].Name[ 0 ] == '\0' ) 
			{
				MV1LoadModelToPMX_GetString( &Src, PmxJoint[ i ].Name, PmxInfo.EncodeType ) ;
			}
			else
			{
				Src += *( ( DWORD * )Src ) + 4 ;
			}

			PmxJoint[ i ].Type = Src[ 0 ] ;
			Src ++ ;

			PmxJoint[ i ].RigidBodyAIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.RigidIndexSize ) ;
			PmxJoint[ i ].RigidBodyBIndex = MV1LoadModelToPMX_GetInt( &Src, PmxInfo.RigidIndexSize ) ;

			*( ( DWORD * )&PmxJoint[ i ].Position[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].Position[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].Position[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxJoint[ i ].Rotation[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].Rotation[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].Rotation[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxJoint[ i ].ConstrainPositionMin[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainPositionMin[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainPositionMin[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxJoint[ i ].ConstrainPositionMax[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainPositionMax[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainPositionMax[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxJoint[ i ].ConstrainRotationMin[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainRotationMin[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainRotationMin[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxJoint[ i ].ConstrainRotationMax[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainRotationMax[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].ConstrainRotationMax[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxJoint[ i ].SpringPosition[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].SpringPosition[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].SpringPosition[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;

			*( ( DWORD * )&PmxJoint[ i ].SpringRotation[ 0 ] ) = *( ( DWORD * )&Src[  0 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].SpringRotation[ 1 ] ) = *( ( DWORD * )&Src[  4 ] ) ;
			*( ( DWORD * )&PmxJoint[ i ].SpringRotation[ 2 ] ) = *( ( DWORD * )&Src[  8 ] ) ;
			Src += 12 ;
		}
	}





	// メッシュを収めるフレームの追加
	Frame = MV1RAddFrame( &RModel, "Mesh", NULL ) ;
	if( Frame == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : フレームオブジェクトの追加に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// 読み込み処理用のボーンデータを格納するメモリ領域の確保
	BoneInfoDim = ( PMX_READ_BONE_INFO * )DXALLOC( sizeof( PMX_READ_BONE_INFO ) * PmxBoneNum ) ;
	if( BoneInfoDim == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 読み込み処理用ボーン情報を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// ボーンデータの追加
	_MEMSET( FrameDimEnable, 0, sizeof( FrameDimEnable ) ) ;
	_MEMSET( FrameDim, 0, sizeof( FrameDim ) ) ;
	do
	{
		_MEMSET( FrameSkipDim, 0, sizeof( FrameSkipDim ) ) ;
		BoneInfo = BoneInfoDim ;
		FrameSkipNum = 0 ;
		for( i = 0 ; ( DWORD )i < PmxBoneNum ; i ++, BoneInfo ++ )
		{
			if( PmxBone[ i ].ParentBoneIndex != -1 && FrameDimEnable[ PmxBone[ i ].ParentBoneIndex ] == 0 )
			{
				FrameSkipDim[ i ] = true ;
				FrameSkipNum ++ ;
				continue ;
			}
			if( FrameDimEnable[ i ] != 0 )
				continue ;
			FrameDimEnable[ i ] = 1 ;

			FrameDim[ i ] = MV1RAddFrame( &RModel, PmxBone[ i ].Name, PmxBone[ i ].ParentBoneIndex == -1 ? NULL : FrameDim[ PmxBone[ i ].ParentBoneIndex ] ) ;
			if( FrameDim[ i ] == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : フレームオブジェクトの追加に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			BoneInfo->OrgTranslate.x = PmxBone[ i ].Position[ 0 ] ;
			BoneInfo->OrgTranslate.y = PmxBone[ i ].Position[ 1 ] ;
			BoneInfo->OrgTranslate.z = PmxBone[ i ].Position[ 2 ] ;
			FrameDim[ i ]->TempVector.x = PmxBone[ i ].Position[ 0 ] ;
			FrameDim[ i ]->TempVector.y = PmxBone[ i ].Position[ 1 ] ;
			FrameDim[ i ]->TempVector.z = PmxBone[ i ].Position[ 2 ] ;
			if( PmxBone[ i ].ParentBoneIndex != -1 )
			{
				FrameDim[ i ]->Translate = VSub( FrameDim[ i ]->TempVector, FrameDim[ PmxBone[ i ].ParentBoneIndex ]->TempVector ) ;
			}
			else
			{
				FrameDim[ i ]->Translate = FrameDim[ i ]->TempVector ;
			}
			FrameDim[ i ]->UserData = BoneInfo ;

			BoneInfo->Base = &PmxBone[ i ] ;
			BoneInfo->Frame = FrameDim[ i ] ;
			BoneInfo->IsPhysics = FALSE ;
			BoneInfo->IsIK = FALSE ;
			BoneInfo->IsAddParent = PmxBone[ i ].Flag_AddMov == 1 || PmxBone[ i ].Flag_AddRot == 1 ? 1 : 0 ;
			BoneInfo->IsIKAnim = FALSE ;
			BoneInfo->IsIKChild = FALSE ;
			BoneInfo->Translate = FrameDim[ i ]->Translate ;
			BoneInfo->Rotate.x = 0.0f ;
			BoneInfo->Rotate.y = 0.0f ;
			BoneInfo->Rotate.z = 0.0f ;
			BoneInfo->Rotate.w = 1.0f ;
			BoneInfo->KeyMatrix = NULL ;
			BoneInfo->KeyMatrix2 = NULL ;

			BoneInfo->InitTranslate = BoneInfo->Translate ;
			BoneInfo->InitRotate    = BoneInfo->Rotate ;
		}
	}while( FrameSkipNum != 0 ) ;
	MV1LoadModelToPMX_SetupMatrix( BoneInfoDim, PmxBoneNum, TRUE, FALSE ) ;


	// メッシュを追加
	{
		// メッシュを追加
		Mesh = MV1RAddMesh( &RModel, Frame ) ;
		if( Mesh == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : メッシュオブジェクトの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// 面情報を格納するメモリ領域の確保
		if( MV1RSetupMeshFaceBuffer( &RModel, Mesh, PmxFaceNum, 3 ) < 0 )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 面情報を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		Mesh->PositionNum = PmxVertexNum ;
		Mesh->Positions = ( VECTOR * )ADDMEMAREA( ( sizeof( VECTOR ) + sizeof( float ) ) * Mesh->PositionNum, &RModel.Mem ) ;
		if( Mesh->Positions == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 頂点座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		Mesh->PositionToonOutLineScale = ( float * )( Mesh->Positions + Mesh->PositionNum ) ;

		Mesh->NormalNum = PmxVertexNum ;
		Mesh->Normals = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * Mesh->NormalNum, &RModel.Mem ) ;
		if( Mesh->Normals == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 頂点法線を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		Mesh->UVNum[ 0 ] = PmxVertexNum ;
		Mesh->UVs[ 0 ] = ( FLOAT4 * )ADDMEMAREA( sizeof( FLOAT4 ) * Mesh->UVNum[ 0 ], &RModel.Mem ) ;
		if( Mesh->UVs[ 0 ] == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 頂点テクスチャ座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		for( i = 0 ; i < PmxInfo.UVNum; i ++ )
		{
			Mesh->UVNum[ i + 1 ] = PmxVertexNum ;
			Mesh->UVs[ i + 1 ] = ( FLOAT4 * )ADDMEMAREA( sizeof( FLOAT4 ) * Mesh->UVNum[ i + 1 ], &RModel.Mem ) ;
			if( Mesh->UVs[ i + 1 ] == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 頂点テクスチャ座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}
		}

		// ボーンの情報を追加
		Mesh->SkinWeightsNum = 0 ;
		for( i = 0 ; ( DWORD )i < PmxBoneNum ; i ++ )
		{
			// このボーンを使用している頂点の数を数える
			weightcount = 0 ;
			for( j = 0 ; ( DWORD )j < PmxVertexNum ; j ++ )
			{
				if( PmxVertex[ j ].BoneIndex[ 0 ] == i ||
					PmxVertex[ j ].BoneIndex[ 1 ] == i ||
					PmxVertex[ j ].BoneIndex[ 2 ] == i ||
					PmxVertex[ j ].BoneIndex[ 3 ] == i )
				{
					weightcount ++ ;
				}
			}

			// 使用されていなかったらウエイト情報は付けない
			if( weightcount == 0 ) continue ;

			Mesh->SkinWeights[ Mesh->SkinWeightsNum ] = MV1RAddSkinWeight( &RModel ) ;
			SkinWeight = Mesh->SkinWeights[ Mesh->SkinWeightsNum ] ;
			Mesh->SkinWeightsNum ++ ;
			SkinWeight->TargetFrame = FrameDim[ i ]->Index ;
			CreateTranslationMatrix( &SkinWeight->ModelLocalMatrix, -FrameDim[ i ]->TempVector.x, -FrameDim[ i ]->TempVector.y, -FrameDim[ i ]->TempVector.z ) ;

			// データを格納するメモリ領域の確保
			SkinWeight->DataNum = weightcount ;
			SkinWeight->Data = ( MV1_SKIN_WEIGHT_ONE_R * )ADDMEMAREA( sizeof( MV1_SKIN_WEIGHT_ONE_R ) * SkinWeight->DataNum, &RModel.Mem ) ;
			if( SkinWeight->Data == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : スキニングメッシュウエイト値を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				return -1 ;
			}

			// ウエイトの情報を格納する
			SkinW = SkinWeight->Data ;
			for( j = 0 ; ( DWORD )j < PmxVertexNum ; j ++ )
			{
				bool Use = false ;
				float Ratio = 0.0f ;

				if( PmxVertex[ j ].BoneIndex[ 0 ] == i )
				{
					Use = true ;
					Ratio += PmxVertex[ j ].BoneWeight[ 0 ] ;
				}

				if( PmxVertex[ j ].BoneIndex[ 1 ] == i )
				{
					Use = true ;
					Ratio += PmxVertex[ j ].BoneWeight[ 1 ] ;
				}

				if( PmxVertex[ j ].BoneIndex[ 2 ] == i )
				{
					Use = true ;
					Ratio += PmxVertex[ j ].BoneWeight[ 2 ] ;
				}

				if( PmxVertex[ j ].BoneIndex[ 3 ] == i )
				{
					Use = true ;
					Ratio += PmxVertex[ j ].BoneWeight[ 3 ] ;
				}

				if( Use )
				{
					SkinW->TargetVertex = j ;
					SkinW->Weight = Ratio ;
					SkinW ++ ;
				}
			}
		}

		// 頂点データをセット
		for( i = 0 ; ( DWORD )i < PmxVertexNum ; i ++ )
		{
			Mesh->Positions[ i ].x = PmxVertex[ i ].Position[ 0 ] ;
			Mesh->Positions[ i ].y = PmxVertex[ i ].Position[ 1 ] ;
			Mesh->Positions[ i ].z = PmxVertex[ i ].Position[ 2 ] ;
			Mesh->Normals[ i ].x = PmxVertex[ i ].Normal[ 0 ] ;
			Mesh->Normals[ i ].y = PmxVertex[ i ].Normal[ 1 ] ;
			Mesh->Normals[ i ].z = PmxVertex[ i ].Normal[ 2 ] ;
			Mesh->UVs[ 0 ][ i ].x = PmxVertex[ i ].UV[ 0 ] ;
			Mesh->UVs[ 0 ][ i ].y = PmxVertex[ i ].UV[ 1 ] ;

			for( j = 0 ; j < PmxInfo.UVNum ; j ++ )
			{
				Mesh->UVs[ j + 1 ][ i ].x = PmxVertex[ i ].AddUV[ j ][ 0 ] ;
				Mesh->UVs[ j + 1 ][ i ].y = PmxVertex[ i ].AddUV[ j ][ 1 ] ;
				Mesh->UVs[ j + 1 ][ i ].z = PmxVertex[ i ].AddUV[ j ][ 2 ] ;
				Mesh->UVs[ j + 1 ][ i ].w = PmxVertex[ i ].AddUV[ j ][ 3 ] ;
			}

			Mesh->PositionToonOutLineScale[ i ] = PmxVertex[ i ].ToonEdgeScale ;
		}

		// 面データをセット
		MeshFace = Mesh->Faces ;
		for( i = 0 ; ( DWORD )i < PmxFaceNum ; i ++ )
		{
			MeshFace->IndexNum = 3 ;
			MeshFace->VertexIndex[ 0 ] = PmxFace[ i ].VertexIndex[ 0 ] ;
			MeshFace->VertexIndex[ 1 ] = PmxFace[ i ].VertexIndex[ 1 ] ;
			MeshFace->VertexIndex[ 2 ] = PmxFace[ i ].VertexIndex[ 2 ] ;
			MeshFace->NormalIndex[ 0 ] = PmxFace[ i ].VertexIndex[ 0 ] ;
			MeshFace->NormalIndex[ 1 ] = PmxFace[ i ].VertexIndex[ 1 ] ;
			MeshFace->NormalIndex[ 2 ] = PmxFace[ i ].VertexIndex[ 2 ] ;
			MeshFace->UVIndex[ 0 ][ 0 ] = PmxFace[ i ].VertexIndex[ 0 ] ;
			MeshFace->UVIndex[ 0 ][ 1 ] = PmxFace[ i ].VertexIndex[ 1 ] ;
			MeshFace->UVIndex[ 0 ][ 2 ] = PmxFace[ i ].VertexIndex[ 2 ] ;

			MeshFace ++ ;
		}
	}

	// ＩＫの情報を格納するメモリ領域の確保
	if( PmxIKNum )
	{
		IKInfoDim = ( PMX_READ_IK_INFO * )DXALLOC( sizeof( PMX_READ_IK_INFO ) * PmxIKNum ) ;
		if( IKInfoDim == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 読み込み処理用ＩＫ情報を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
	}

	// ＩＫデータの追加
	IKInfo = IKInfoDim ;
	IKInfoFirst = NULL ;
	i = 0 ;
	for( k = 0 ; ( DWORD )k < PmxBoneNum ; k ++ )
	{
		if( PmxBone[ k ].Flag_IK == 0 ) continue ;

		PMX_READ_BONE_INFO *BoneBone ;

		// データをセット
		IKInfo->Base = &PmxBone[ k ].IKInfo ;
		IKInfo->Bone = &BoneInfoDim[ k ] ;
		IKInfo->TargetBone = &BoneInfoDim[ PmxBone[ k ].IKInfo.TargetBoneIndex ] ;
		IKInfo->TargetBone->IsIK = TRUE ;

		for( j = 0 ; j < IKInfo->Base->LinkNum ; j ++ )
		{
			BoneBone = &BoneInfoDim[ IKInfo->Base->Link[ j ].BoneIndex ] ;
			BoneInfoDim[ IKInfo->Base->Link[ j ].BoneIndex ].IsIK = TRUE ;
		}

		// リストに追加
		if( IKInfoFirst == NULL )
		{
			IKInfoFirst = IKInfo ;
			IKInfo->Prev = NULL ;
			IKInfo->Next = NULL ;
		}
		else
		{
			PMX_READ_IK_INFO *IKInfoTemp ;

			for( IKInfoTemp = IKInfoFirst ; IKInfoTemp->Next != NULL && IKInfoTemp->Base->Link[ 0 ].BoneIndex < IKInfo->Base->Link[ 0 ].BoneIndex ; IKInfoTemp = IKInfoTemp->Next ){}
			if( IKInfoTemp->Next == NULL && IKInfoTemp->Base->Link[ 0 ].BoneIndex < IKInfo->Base->Link[ 0 ].BoneIndex )
			{
				IKInfoTemp->Next = IKInfo ;
				IKInfo->Next = NULL ;
				IKInfo->Prev = IKInfoTemp ;
			}
			else
			{
				if( IKInfoTemp->Prev == NULL )
				{
					IKInfoTemp->Prev = IKInfo ;
					IKInfo->Next = IKInfoTemp ;
					IKInfo->Prev = NULL ;
					IKInfoFirst = IKInfo ;
				}
				else
				{
					IKInfo->Prev = IKInfoTemp->Prev ;
					IKInfo->Next = IKInfoTemp ;
					IKInfoTemp->Prev->Next = IKInfo ;
					IKInfoTemp->Prev = IKInfo ;
				}
			}
		}

		IKInfo ++ ;
	}

	// ＩＫの影響を受けるボーンの子でＩＫの影響を受けないボーンに印をつける
	BoneInfo = BoneInfoDim ;
	for( i = 0 ; ( DWORD )i < PmxBoneNum ; i ++, BoneInfo ++ )
	{
		PMX_READ_BONE_INFO *ParentBone ;

		if( BoneInfo->IsIK || BoneInfo->IsAddParent )
			continue ;

		if( BoneInfo->Frame->Parent == NULL )
			continue ;

		ParentBone = ( PMX_READ_BONE_INFO * )BoneInfo->Frame->Parent->UserData ;
		if( ParentBone->IsIK || ParentBone->IsAddParent )
		{
			BoneInfo->IsIKChild = TRUE ;
		}
	}

#ifndef DX_NON_BULLET_PHYSICS
	// 物理演算データの読み込み
	if( LoadParam->GParam.LoadModelToUsePhysicsMode != DX_LOADMODEL_PHYSICS_DISABLE )
	{
		// 物理演算データがある場合のみ処理をする
		if( PmxRigidbodyNum > 0 && PmxJointNum > 0 )
		{
			PMX_READ_PHYSICS_INFO *PhysicsInfo ;
			PMX_READ_PHYSICS_JOINT_INFO *JointInfo ;

			// ここにきたら物理演算用データがあるということ
			ValidPhysics = TRUE ;

			// 物理演算情報の読み込み

			// ゼロ初期化
			_MEMSET( &MLPhysicsInfo, 0, sizeof( DX_MODELLOADER3_PMX_PHYSICS_INFO ) ) ;

			// 物理演算情報の数を取得
			MLPhysicsInfo.PmxPhysicsNum = PmxRigidbodyNum ;

			// データを格納するメモリ領域の確保
			MLPhysicsInfo.PmxPhysicsInfoDim = ( PMX_READ_PHYSICS_INFO * )DXALLOC( sizeof( PMX_READ_PHYSICS_INFO ) * MLPhysicsInfo.PmxPhysicsNum ) ;
			if( MLPhysicsInfo.PmxPhysicsInfoDim == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 物理演算情報配列を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}
			_MEMSET( MLPhysicsInfo.PmxPhysicsInfoDim, 0, sizeof( PMX_READ_PHYSICS_INFO ) * MLPhysicsInfo.PmxPhysicsNum ) ;

			// データを読み出す
			PhysicsInfo = MLPhysicsInfo.PmxPhysicsInfoDim ;
			for( i = 0 ; i < MLPhysicsInfo.PmxPhysicsNum ; i ++, PhysicsInfo ++ )
			{
				PhysicsInfo->Base = &PmxRigidbody[ i ] ;

				if( PhysicsInfo->Base->BoneIndex == -1 )
				{
					for( j = 0 ; _STRCMP( BoneInfoDim[ j ].Frame->Name, "センター" ) != 0 ; j ++ ){}
					PhysicsInfo->Bone = &BoneInfoDim[ j ] ;
				}
				else
				{
					PhysicsInfo->Bone = &BoneInfoDim[ PhysicsInfo->Base->BoneIndex ] ;
				}
				PhysicsInfo->Bone->IsPhysics = 1 ;
				PhysicsInfo->Bone->PhysicsIndex = i ;
				PhysicsInfo->Bone->SetupPhysicsAnim = 0 ;

				// 剛体タイプを保存
				PhysicsInfo->NoCopyToBone = _STRCMP( PhysicsInfo->Bone->Frame->Name, "センター" ) == 0 ;
			}

			// ジョイント情報の数を取得
			MLPhysicsInfo.PmxPhysicsJointNum = PmxJointNum ;

			// データを格納するメモリ領域の確保
			MLPhysicsInfo.PmxPhysicsJointInfoDim = ( PMX_READ_PHYSICS_JOINT_INFO * )DXALLOC( sizeof( PMX_READ_PHYSICS_JOINT_INFO ) * MLPhysicsInfo.PmxPhysicsJointNum ) ;
			if( MLPhysicsInfo.PmxPhysicsJointInfoDim == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 物理演算ジョイント情報を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}
			_MEMSET( MLPhysicsInfo.PmxPhysicsJointInfoDim, 0, sizeof( PMX_READ_PHYSICS_JOINT_INFO ) * MLPhysicsInfo.PmxPhysicsJointNum ) ;

			// データを読み出す
			JointInfo = MLPhysicsInfo.PmxPhysicsJointInfoDim ;
			for( i = 0 ; i < MLPhysicsInfo.PmxPhysicsJointNum ; i ++, JointInfo ++ )
			{
				JointInfo->Base = &PmxJoint[ i ] ;
			}


			// もしリアルタイム物理処理を行う場合は読み込み情報に物理ボーンと物理ジョイントの情報を加える
			if( LoadParam->GParam.LoadModelToUsePhysicsMode == DX_LOADMODEL_PHYSICS_REALTIME )
			{
				MV1_PHYSICS_RIGIDBODY_R *RigidBody ;
				MV1_PHYSICS_JOINT_R *Joint ;

				PhysicsInfo = MLPhysicsInfo.PmxPhysicsInfoDim ;
				for( i = 0 ; i < MLPhysicsInfo.PmxPhysicsNum ; i ++, PhysicsInfo ++ )
				{
					RigidBody = MV1RAddPhysicsRididBody( &RModel, PhysicsInfo->Base->Name, PhysicsInfo->Bone->Frame ) ;
					if( RigidBody == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 物理計算用剛体情報の追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					RigidBody->RigidBodyGroupIndex = PhysicsInfo->Base->RigidBodyGroupIndex ;
					RigidBody->RigidBodyGroupTarget = PhysicsInfo->Base->RigidBodyGroupTarget ;
					RigidBody->ShapeType = PhysicsInfo->Base->ShapeType ;
					RigidBody->ShapeW = PhysicsInfo->Base->ShapeW ;
					RigidBody->ShapeH = PhysicsInfo->Base->ShapeH ;
					RigidBody->ShapeD = PhysicsInfo->Base->ShapeD ;
					RigidBody->Position.x = PhysicsInfo->Base->Position[ 0 ] - PhysicsInfo->Bone->LocalWorldMatrix.m[ 3 ][ 0 ] ;
					RigidBody->Position.y = PhysicsInfo->Base->Position[ 1 ] - PhysicsInfo->Bone->LocalWorldMatrix.m[ 3 ][ 1 ] ;
					RigidBody->Position.z = PhysicsInfo->Base->Position[ 2 ] - PhysicsInfo->Bone->LocalWorldMatrix.m[ 3 ][ 2 ] ;
					RigidBody->Rotation.x = PhysicsInfo->Base->Rotation[ 0 ] ;
					RigidBody->Rotation.y = PhysicsInfo->Base->Rotation[ 1 ] ;
					RigidBody->Rotation.z = PhysicsInfo->Base->Rotation[ 2 ] ;
					RigidBody->RigidBodyWeight = PhysicsInfo->Base->RigidBodyWeight ;
					RigidBody->RigidBodyPosDim = PhysicsInfo->Base->RigidBodyPosDim ;
					RigidBody->RigidBodyRotDim = PhysicsInfo->Base->RigidBodyRotDim ;
					RigidBody->RigidBodyRecoil = PhysicsInfo->Base->RigidBodyRecoil ;
					RigidBody->RigidBodyFriction = PhysicsInfo->Base->RigidBodyFriction ;
					RigidBody->RigidBodyType = PhysicsInfo->Base->RigidBodyType ;
					RigidBody->NoCopyToBone = PhysicsInfo->NoCopyToBone ? TRUE : FALSE ;
				}

				JointInfo = MLPhysicsInfo.PmxPhysicsJointInfoDim ;
				for( i = 0 ; i < MLPhysicsInfo.PmxPhysicsJointNum ; i ++, JointInfo ++ )
				{
					Joint = MV1RAddPhysicsJoint( &RModel, JointInfo->Base->Name ) ;
					if( Joint == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 物理計算用剛体接合情報の追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					Joint->RigidBodyA = JointInfo->Base->RigidBodyAIndex ;
					Joint->RigidBodyB = JointInfo->Base->RigidBodyBIndex ;
					Joint->Position.x = JointInfo->Base->Position[ 0 ] ;
					Joint->Position.y = JointInfo->Base->Position[ 1 ] ;
					Joint->Position.z = JointInfo->Base->Position[ 2 ] ;
					Joint->Rotation.x = JointInfo->Base->Rotation[ 0 ] ;
					Joint->Rotation.y = JointInfo->Base->Rotation[ 1 ] ;
					Joint->Rotation.z = JointInfo->Base->Rotation[ 2 ] ;
					Joint->ConstrainPosition1.x = JointInfo->Base->ConstrainPositionMin[ 0 ] ;
					Joint->ConstrainPosition1.y = JointInfo->Base->ConstrainPositionMin[ 1 ] ;
					Joint->ConstrainPosition1.z = JointInfo->Base->ConstrainPositionMin[ 2 ] ;
					Joint->ConstrainPosition2.x = JointInfo->Base->ConstrainPositionMax[ 0 ] ;
					Joint->ConstrainPosition2.y = JointInfo->Base->ConstrainPositionMax[ 1 ] ;
					Joint->ConstrainPosition2.z = JointInfo->Base->ConstrainPositionMax[ 2 ] ;
					Joint->ConstrainRotation1.x = JointInfo->Base->ConstrainRotationMin[ 0 ] ;
					Joint->ConstrainRotation1.y = JointInfo->Base->ConstrainRotationMin[ 1 ] ;
					Joint->ConstrainRotation1.z = JointInfo->Base->ConstrainRotationMin[ 2 ] ;
					Joint->ConstrainRotation2.x = JointInfo->Base->ConstrainRotationMax[ 0 ] ;
					Joint->ConstrainRotation2.y = JointInfo->Base->ConstrainRotationMax[ 1 ] ;
					Joint->ConstrainRotation2.z = JointInfo->Base->ConstrainRotationMax[ 2 ] ;
					Joint->SpringPosition.x = JointInfo->Base->SpringPosition[ 0 ] ;
					Joint->SpringPosition.y = JointInfo->Base->SpringPosition[ 1 ] ;
					Joint->SpringPosition.z = JointInfo->Base->SpringPosition[ 2 ] ;
					Joint->SpringRotation.x = JointInfo->Base->SpringRotation[ 0 ] * DX_PI_F / 180.0f ;
					Joint->SpringRotation.y = JointInfo->Base->SpringRotation[ 1 ] * DX_PI_F / 180.0f ;
					Joint->SpringRotation.z = JointInfo->Base->SpringRotation[ 2 ] * DX_PI_F / 180.0f ;
				}
			}
		}
	}
#endif

	// マテリアルの読みこみ
	facecount = 0 ;
	for( i = 0 ; ( DWORD )i < PmxMaterialNum ; i ++ )
	{
		Material = MV1RAddMaterial( &RModel, PmxMaterial[ i ].Name ) ;
		if( Material == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : No.%d の Material オブジェクトの追加に失敗しました\n" ), i ) ) ;
			goto ENDLABEL ;
		}

		// マテリアルタイプはトゥーン
		Material->Type = DX_MATERIAL_TYPE_TOON_2 ;

		// 情報をセット
		Material->Diffuse.r = PmxMaterial[ i ].Diffuse[ 0 ] ;
		Material->Diffuse.g = PmxMaterial[ i ].Diffuse[ 1 ] ;
		Material->Diffuse.b = PmxMaterial[ i ].Diffuse[ 2 ] ;
		Material->Diffuse.a = PmxMaterial[ i ].Diffuse[ 3 ] ;

		Material->Specular.r = PmxMaterial[ i ].Specular[ 0 ] ;
		Material->Specular.g = PmxMaterial[ i ].Specular[ 1 ] ;
		Material->Specular.b = PmxMaterial[ i ].Specular[ 2 ] ;
		Material->Specular.a = 0.0f ;

		Material->Power = PmxMaterial[ i ].SpecularPower ;

		Material->Ambient.r = PmxMaterial[ i ].Ambient[ 0 ] ;
		Material->Ambient.g = PmxMaterial[ i ].Ambient[ 1 ] ;
		Material->Ambient.b = PmxMaterial[ i ].Ambient[ 2 ] ;
		Material->Ambient.a = 0.0f ;

		Material->OutLineColor.r = PmxMaterial[ i ].EdgeColor[ 0 ] ;
		Material->OutLineColor.g = PmxMaterial[ i ].EdgeColor[ 1 ] ;
		Material->OutLineColor.b = PmxMaterial[ i ].EdgeColor[ 2 ] ;
		Material->OutLineColor.a = PmxMaterial[ i ].EdgeColor[ 3 ] ;

		Material->DisableBackCulling = PmxMaterial[ i ].CullingOff ;

		// 輪郭線の太さをセット
		if( PmxMaterial[ i ].EdgeDraw == 1 )
		{
			Material->OutLineDotWidth = PmxMaterial[ i ].EdgeSize ;
			Material->OutLineWidth = PmxMaterial[ i ].EdgeSize / 100.0f ;
		}

		// トゥーンテクスチャを追加
		if( PmxMaterial[ i ].ToonTextureIndex >= 0 )
		{
			// スフィアマップのファイル名が含まれる場合はそれを除く
			if( PmxMaterial[ i ].ToonFlag == 0 )
			{
				_STRCPY( String, PmxTexture[ PmxMaterial[ i ].ToonTextureIndex ].TexturePath ) ;
			}
			else
			{
				_SPRINTF( String, "toon%02d.bmp", PmxMaterial[ i ].ToonTextureIndex + 1 ) ;
			}
			Texture = MV1RAddTexture( &RModel, String, String, NULL, FALSE, 0.1f, true, true, true ) ;
			if( Texture == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : No.%d の トゥーン用テクスチャオブジェクトの作成に失敗しました\n" ), i ) ) ;
				goto ENDLABEL ;
			}
			Texture->AddressModeU = DX_TEXADDRESS_CLAMP ;
			Texture->AddressModeV = DX_TEXADDRESS_CLAMP ;
			Material->DiffuseGradTexture = Texture ;
			Material->DiffuseGradBlendType = MV1_LAYERBLEND_TYPE_MODULATE ;
		}
		Material->SpecularGradTextureDefault = -1 ;
		Material->SpecularGradBlendType = MV1_LAYERBLEND_TYPE_ADDITIVE ;

		// 面データにマテリアル番号をセットする
		for( j = 0 ; j < PmxMaterial[ i ].MaterialFaceNum ; j += 3, facecount ++ )
		{
			Mesh->Faces[ facecount ].MaterialIndex = Mesh->MaterialNum ;
		}

		// テクスチャを追加
		if( PmxMaterial[ i ].TextureIndex >= 0 &&
			PmxTexture[ PmxMaterial[ i ].TextureIndex ].TexturePath[ 0 ] != '\0' )
		{
			Texture = MV1RAddTexture( &RModel, PmxTexture[ PmxMaterial[ i ].TextureIndex ].TexturePath, PmxTexture[ PmxMaterial[ i ].TextureIndex ].TexturePath, NULL, false, 0.1f, true, false, true ) ;
			if( Texture == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : No.%d の テクスチャオブジェクトの作成に失敗しました\n" ), i ) ) ;
				goto ENDLABEL ;
			}
			Material->DiffuseTexNum = 1 ;
			Material->DiffuseTexs[ 0 ] = Texture ;
		}

		// スフィアマップを追加
		if( PmxMaterial[ i ].SphereTextureIndex >= 0 &&
			PmxTexture[ PmxMaterial[ i ].SphereTextureIndex ].TexturePath[ 0 ] != '\0' )
		{
			Texture = MV1RAddTexture( &RModel, PmxTexture[ PmxMaterial[ i ].SphereTextureIndex ].TexturePath, PmxTexture[ PmxMaterial[ i ].SphereTextureIndex ].TexturePath, NULL, false, 0.1f, true, false, true ) ;
			if( Texture == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : No.%d の テクスチャオブジェクトの作成に失敗しました\n" ), i ) ) ;
				goto ENDLABEL ;
			}

			Material->SphereMapTexture = Texture ;
			switch( PmxMaterial[ i ].SphereMode )
			{
			default :
				Material->SphereMapBlendType = DX_MATERIAL_BLENDTYPE_NONE ;
				break ;

			case 1 :
				Material->SphereMapBlendType = DX_MATERIAL_BLENDTYPE_MODULATE ;
				break ;

			case 2 :
				Material->SphereMapBlendType = DX_MATERIAL_BLENDTYPE_ADDITIVE ;
				break ;
			}
		}

		// マテリアルが１ポリゴンにでも使用されている場合はマテリアルのアドレスをセット
		if( PmxMaterial[ i ].MaterialFaceNum != 0 )
		{
			Mesh->Materials[ Mesh->MaterialNum ] = Material ;
			Mesh->MaterialNum ++ ;
		}
	}

	// 表情データの追加
	if( PmxSkinNum > 0 )
	{
		SkinMesh = MV1RAddMesh( &RModel, Frame ) ;
		if( SkinMesh == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : スキンメッシュオブジェクトの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// base メッシュから元モデルから使用する頂点と面の情報を割り出す
		SkinNextVertIndex = ( DWORD * )DXALLOC( sizeof( DWORD ) * ( 2 * PmxVertexNum + 2 * PmxFaceNum + 2 * PmxMaterialNum ) ) ;
		if( SkinNextVertIndex == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 一時保存用の頂点データを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		SkinPrevVertIndex     = SkinNextVertIndex     + PmxVertexNum ;
		SkinNextFaceIndex     = SkinPrevVertIndex     + PmxVertexNum ;
		SkinPrevFaceIndex     = SkinNextFaceIndex     + PmxFaceNum ;
		SkinNextMaterialIndex = SkinPrevFaceIndex     + PmxFaceNum ;
		SkinPrevMaterialIndex = SkinNextMaterialIndex + PmxMaterialNum ;
		_MEMSET( SkinNextVertIndex,     0xff, sizeof( DWORD ) * PmxVertexNum   ) ;
		_MEMSET( SkinPrevVertIndex,     0xff, sizeof( DWORD ) * PmxVertexNum   ) ;
		_MEMSET( SkinNextFaceIndex,     0xff, sizeof( DWORD ) * PmxFaceNum     ) ;
		_MEMSET( SkinPrevFaceIndex,     0xff, sizeof( DWORD ) * PmxFaceNum     ) ;
		_MEMSET( SkinNextMaterialIndex, 0xff, sizeof( DWORD ) * PmxMaterialNum ) ;
		_MEMSET( SkinPrevMaterialIndex, 0xff, sizeof( DWORD ) * PmxMaterialNum ) ;

		// 表情データ本体で使用する頂点の元のメッシュでの頂点番号を取得
		SkinVertNum = 0 ;
		for( i = 0 ; ( DWORD )i < PmxMorphNum ; i ++ )
		{
			if( PmxMorph[ i ].Type != 1 ) continue ;

			for( j = 0 ; j < PmxMorph[ i ].DataNum ; j ++ )
			{
				for( k = 0 ; ( DWORD )k < SkinVertNum && SkinPrevVertIndex[ k ] != ( DWORD )PmxMorph[ i ].Vertex[ j ].Index ; k ++ ){}
				if( ( DWORD )k == SkinVertNum )
				{
					SkinPrevVertIndex[ SkinVertNum ] = PmxMorph[ i ].Vertex[ j ].Index ;
					SkinNextVertIndex[ PmxMorph[ i ].Vertex[ j ].Index ] = SkinVertNum ;
					SkinVertNum ++ ;
				}
			}
		}

		// 表情データで使用されている頂点を使っている面の数を数える
		MeshFace = Mesh->Faces ;
		SkinFaceNum = 0 ;
		SkinMaterialNum = 0 ;
		SkinBaseVertNum = SkinVertNum ;
		for( i = 0 ; ( DWORD )i < PmxFaceNum ; i ++, MeshFace ++ )
		{
			if( SkinNextVertIndex[ MeshFace->VertexIndex[ 0 ] ] >= SkinBaseVertNum &&
				SkinNextVertIndex[ MeshFace->VertexIndex[ 1 ] ] >= SkinBaseVertNum &&
				SkinNextVertIndex[ MeshFace->VertexIndex[ 2 ] ] >= SkinBaseVertNum ) continue ;

			if( SkinNextMaterialIndex[ MeshFace->MaterialIndex ] == 0xffffffff )
			{
				SkinPrevMaterialIndex[ SkinMaterialNum ] = MeshFace->MaterialIndex ;
				SkinNextMaterialIndex[ MeshFace->MaterialIndex ] = SkinMaterialNum ;
				SkinMaterialNum ++ ;
			}

			SkinPrevFaceIndex[ SkinFaceNum ] = i ;
			SkinNextFaceIndex[ i ] = SkinFaceNum ;
			SkinFaceNum ++ ;

			if( SkinNextVertIndex[ MeshFace->VertexIndex[ 0 ] ] == 0xffffffff )
			{
				SkinPrevVertIndex[ SkinVertNum ] = MeshFace->VertexIndex[ 0 ] ;
				SkinNextVertIndex[ MeshFace->VertexIndex[ 0 ] ] = SkinVertNum ;
				SkinVertNum ++ ;
			}

			if( SkinNextVertIndex[ MeshFace->VertexIndex[ 1 ] ] == 0xffffffff )
			{
				SkinPrevVertIndex[ SkinVertNum ] = MeshFace->VertexIndex[ 1 ] ;
				SkinNextVertIndex[ MeshFace->VertexIndex[ 1 ] ] = SkinVertNum ;
				SkinVertNum ++ ;
			}

			if( SkinNextVertIndex[ MeshFace->VertexIndex[ 2 ] ] == 0xffffffff )
			{
				SkinPrevVertIndex[ SkinVertNum ] = MeshFace->VertexIndex[ 2 ] ;
				SkinNextVertIndex[ MeshFace->VertexIndex[ 2 ] ] = SkinVertNum ;
				SkinVertNum ++ ;
			}
		}

		// 使用されているマテリアルのリストを作成する
		SkinMesh->MaterialNum = SkinMaterialNum ;
		for( i = 0 ; ( DWORD )i < SkinMaterialNum ; i ++ )
		{
			SkinMesh->Materials[ i ] = Mesh->Materials[ SkinPrevMaterialIndex[ i ] ] ;
		}

		// 表情データで使用する面を保存するメモリ領域を確保する
		if( MV1RSetupMeshFaceBuffer( &RModel, SkinMesh, SkinFaceNum, 3 ) < 0 )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 表情データの面情報を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// 表情データで使用する頂点を保存するメモリ領域を確保する
		SkinMesh->PositionNum = SkinVertNum ;
		SkinMesh->Positions = ( VECTOR * )ADDMEMAREA( ( sizeof( VECTOR ) + sizeof( float ) ) * SkinMesh->PositionNum, &RModel.Mem ) ;
		if( SkinMesh->Positions == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 表情データの頂点座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		SkinMesh->PositionToonOutLineScale = ( float * )( SkinMesh->Positions + SkinMesh->PositionNum ) ;

		SkinMesh->NormalNum = SkinVertNum ;
		SkinMesh->Normals = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * SkinMesh->NormalNum, &RModel.Mem ) ;
		if( SkinMesh->Normals == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 表情データの頂点法線を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		SkinMesh->UVNum[ 0 ] = SkinVertNum ;
		SkinMesh->UVs[ 0 ] = ( FLOAT4 * )ADDMEMAREA( sizeof( FLOAT4 ) * SkinMesh->UVNum[ 0 ], &RModel.Mem ) ;
		if( SkinMesh->UVs[ 0 ] == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 表情データの頂点テクスチャ座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// 表情データで使用する頂点のデータをセットする
		for( i = 0 ; ( DWORD )i < SkinVertNum ; i ++ )
		{
			SkinMesh->Positions[ i ] = Mesh->Positions[ SkinPrevVertIndex[ i ] ] ;
			SkinMesh->Normals[ i ] = Mesh->Normals[ SkinPrevVertIndex[ i ] ] ;
			SkinMesh->PositionToonOutLineScale[ i ] = Mesh->PositionToonOutLineScale[ SkinPrevVertIndex[ i ] ] ;
			SkinMesh->UVs[ 0 ][ i ] = Mesh->UVs[ 0 ][ SkinPrevVertIndex[ i ] ] ;
		}

		// 表情データで使用する面のデータをセットする
		MeshFace = SkinMesh->Faces ;
		for( i = 0 ; ( DWORD )i < SkinFaceNum ; i ++, MeshFace ++ )
		{
			MeshFaceTemp = &Mesh->Faces[ SkinPrevFaceIndex[ i ] ] ;
			MeshFace->IndexNum = 3 ;
			MeshFace->VertexIndex[ 0 ] = SkinNextVertIndex[ MeshFaceTemp->VertexIndex[ 0 ] ] ;
			MeshFace->VertexIndex[ 1 ] = SkinNextVertIndex[ MeshFaceTemp->VertexIndex[ 1 ] ] ;
			MeshFace->VertexIndex[ 2 ] = SkinNextVertIndex[ MeshFaceTemp->VertexIndex[ 2 ] ] ;
			MeshFace->NormalIndex[ 0 ] = MeshFace->VertexIndex[ 0 ] ;
			MeshFace->NormalIndex[ 1 ] = MeshFace->VertexIndex[ 1 ] ;
			MeshFace->NormalIndex[ 2 ] = MeshFace->VertexIndex[ 2 ] ;
			MeshFace->UVIndex[ 0 ][ 0 ] = MeshFace->VertexIndex[ 0 ] ;
			MeshFace->UVIndex[ 0 ][ 1 ] = MeshFace->VertexIndex[ 1 ] ;
			MeshFace->UVIndex[ 0 ][ 2 ] = MeshFace->VertexIndex[ 2 ] ;
			MeshFace->MaterialIndex = SkinNextMaterialIndex[ MeshFaceTemp->MaterialIndex ] ;
		}

		// 元のメッシュで表情データに使用されている面を無効化する
		NextFaceNum = 0 ;
		MeshFaceTemp = Mesh->Faces ;
		MeshFace = Mesh->Faces ;
		for( i = 0 ; ( DWORD )i < Mesh->FaceNum ; i ++, MeshFaceTemp ++ )
		{
			if( SkinNextFaceIndex[ i ] != 0xffffffff ) continue ;
			*MeshFace = *MeshFaceTemp ;
			MeshFace ++ ;
			NextFaceNum ++ ;
		}
		Mesh->FaceNum = NextFaceNum ;

		// スキニングメッシュ情報を構築する
		for( i = 0 ; ( DWORD )i < Mesh->SkinWeightsNum ; i ++ )
		{
			// 表情データで使用するかどうかを調べる
			SkinWeightTemp = Mesh->SkinWeights[ i ] ;
			SkinWTemp = SkinWeightTemp->Data ;
			weightcount = 0 ;
			for( j = 0 ; ( DWORD )j < SkinWeightTemp->DataNum ; j ++, SkinWTemp ++ )
			{
				if( SkinNextVertIndex[ SkinWTemp->TargetVertex ] == 0xffffffff ) continue ;
				weightcount ++ ;
			}
			if( weightcount == 0 ) continue ;

			// 使う場合は追加
			SkinMesh->SkinWeights[ SkinMesh->SkinWeightsNum ] = MV1RAddSkinWeight( &RModel ) ;
			SkinWeight = SkinMesh->SkinWeights[ SkinMesh->SkinWeightsNum ] ;
			SkinMesh->SkinWeightsNum ++ ;

			SkinWeight->ModelLocalMatrix = SkinWeightTemp->ModelLocalMatrix ;
			SkinWeight->TargetFrame = SkinWeightTemp->TargetFrame ;
			SkinWeight->Data = ( MV1_SKIN_WEIGHT_ONE_R * )ADDMEMAREA( sizeof( MV1_SKIN_WEIGHT_ONE_R ) * weightcount, &RModel.Mem ) ;
			if( SkinWeight->Data == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 表情データ用スキニングメッシュウエイト値を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				return -1 ;
			}

			// データをセット
			SkinW = SkinWeight->Data ;
			SkinWTemp = SkinWeightTemp->Data ;
			for( j = 0 ; ( DWORD )j < SkinWeightTemp->DataNum ; j ++, SkinWTemp ++ )
			{
				if( SkinNextVertIndex[ SkinWTemp->TargetVertex ] == 0xffffffff ) continue ;
				SkinW->TargetVertex = SkinNextVertIndex[ SkinWTemp->TargetVertex ] ;
				SkinW->Weight = SkinWTemp->Weight ;
				SkinW ++ ;
				SkinWeight->DataNum ++ ;
			}
		}

		// 元のメッシュで表情データに使用されているスキニングメッシュ情報を無効化する
		for( i = 0 ; ( DWORD )i < Mesh->SkinWeightsNum ; i ++ )
		{
			// 表情データで使用するかどうかを調べる
			SkinWeightTemp = Mesh->SkinWeights[ i ] ;
			SkinWTemp = SkinWeightTemp->Data ;
			SkinW = SkinWeightTemp->Data ;
			weightcount = 0 ;
			for( j = 0 ; ( DWORD )j < SkinWeightTemp->DataNum - 1 ; j ++, SkinWTemp ++ )
			{
				if( SkinNextVertIndex[ SkinWTemp->TargetVertex ] < SkinBaseVertNum ) continue ;
				*SkinW = *SkinWTemp ;
				SkinW ++ ;
				weightcount ++ ;
			}
			// 最後の一つは別処理
			if( weightcount == 0 || SkinNextVertIndex[ SkinWTemp->TargetVertex ] >= SkinBaseVertNum )
			{
				*SkinW = *SkinWTemp ;
				SkinW ++ ;
				weightcount ++ ;
			}
			SkinWeightTemp->DataNum = weightcount ;
		}

		// 表情データを追加する
		for( i = 0 ; ( DWORD )i < PmxMorphNum ; i ++ )
		{
			if( PmxMorph[ i ].Type != 1 ) continue ;

			// 表情データの追加
			Shape = MV1RAddShape( &RModel, PmxMorph[ i ].Name, Frame ) ; 
			if( Shape == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : モーフオブジェクトの追加に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// 対象メッシュのセット
			Shape->TargetMesh = SkinMesh ;

			// 法線は無し
			Shape->ValidVertexNormal = FALSE ;

			// 頂点の数を保存
			Shape->VertexNum = PmxMorph[ i ].DataNum ;

			// 頂点データを格納するメモリ領域の確保
			Shape->Vertex = ( MV1_SHAPE_VERTEX_R * )ADDMEMAREA( sizeof( MV1_SHAPE_VERTEX_R ) * Shape->VertexNum, &RModel.Mem ) ;
			if( Shape->Vertex == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : モーフ頂点データを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// 頂点データを埋める
			ShapeVert = Shape->Vertex ;
			for( j = 0 ; j < Shape->VertexNum ; j ++, ShapeVert ++ )
			{
				ShapeVert->TargetPositionIndex = SkinNextVertIndex[ PmxMorph[ i ].Vertex[ j ].Index ] ;
				ShapeVert->Position.x = PmxMorph[ i ].Vertex[ j ].Offset[ 0 ] ;
				ShapeVert->Position.y = PmxMorph[ i ].Vertex[ j ].Offset[ 1 ] ;
				ShapeVert->Position.z = PmxMorph[ i ].Vertex[ j ].Offset[ 2 ] ;
			}
		}
	}

	// VMDファイルがあったら読み込む
	{
		int FileSize ;
		void *VmdData ;
		int Result ;
		int LoopMotionFlag ;
		int GravityNo ;
		VECTOR Gravity ;
		int GravityEnable ;
		float BaseGravity ;
		int NameLen ;
		const TCHAR *Name ;
		const TCHAR *CurrentDir ;
		int j, k ;

		k = 0 ;
		for( j = 0 ; j < 2 ; j ++ )
		{
			switch( j )
			{
			case 0 :
				if( LoadParam->GParam.AnimFilePathValid == FALSE )
					continue ;

				Name = LoadParam->GParam.AnimFileName ;
				CurrentDir = LoadParam->GParam.AnimFileDirPath ;
				break ;

			case 1 :
				Name = LoadParam->Name ;
				CurrentDir = LoadParam->CurrentDir ;
				break ;
			}

			NameLen = lstrlen( Name ) ;
			if( LoadParam->GParam.LoadModelToWorldGravityInitialize == FALSE )
			{
				BaseGravity = -9.8f * 12.5f ;
			}
			else
			{
				BaseGravity = LoadParam->GParam.LoadModelToWorldGravity ;
			}

			for( i = 0 ; ; i ++ )
			{
				// VMDファイルの読み込み
				if( LoadFile_VMD(
						&VmdData,
						&FileSize,
						i,
						LoadParam->FileReadFunc,
						Name,
						NameLen,
						CurrentDir,
						&LoopMotionFlag,
						&BaseGravity,
						&GravityNo,
						&GravityEnable,
						&Gravity
					) == FALSE )
					break ;

				// 行列の再セットアップ
				MV1LoadModelToPMX_SetupMatrix( BoneInfoDim, PmxBoneNum, TRUE, FALSE ) ;

#ifndef DX_NON_BULLET_PHYSICS
				// ループモーションかどうかの情報をセットする
				if( LoopMotionFlag )
				{
					MLPhysicsInfo.LoopMotionFlag = TRUE ;
					MLPhysicsInfo.LoopMotionNum = 3 ;	// ←現在この値を有効にする場合は３固定 
				}
				else
				{
					MLPhysicsInfo.LoopMotionFlag = FALSE ;
					MLPhysicsInfo.LoopMotionNum = 1 ;
				}

				// 重力パラメータをセット
				if( GravityEnable )
				{
					MLPhysicsInfo.Gravity = Gravity ;
				}
				else
				if( GravityNo != -1 )
				{
					MLPhysicsInfo.Gravity = MV1Man.LoadCalcPhysicsWorldGravity[ GravityNo ] ;
				}
				else
				{
					MLPhysicsInfo.Gravity.x = 0.0f ;
					MLPhysicsInfo.Gravity.y = BaseGravity ;
					MLPhysicsInfo.Gravity.z = 0.0f ;
				}
#endif
				_SPRINTF( String, "Anim%03d", k ) ;
				Result = _MV1LoadModelToVMD_PMX(
					k,
					&RModel,
					VmdData,
					FileSize,
					String,
					BoneInfoDim,
					PmxBoneNum,
					IKInfoFirst,
#ifndef DX_NON_BULLET_PHYSICS
					ValidPhysics && LoadParam->GParam.LoadModelToUsePhysicsMode == DX_LOADMODEL_PHYSICS_LOADCALC ? &MLPhysicsInfo : NULL,
#endif
					false
				) ;
				if( LoadParam->CurrentDir == NULL )
				{
					LoadParam->FileReadFunc->Release( VmdData, LoadParam->FileReadFunc->Data ) ;
				}
				else
				{
					DXFREE( VmdData ) ;
				}

				if( Result != 0 )
					break ;

				k ++ ;
			}
		}
	}

	// モデル基データハンドルの作成
	NewHandle = MV1LoadModelToReadModel( &LoadParam->GParam, &RModel, LoadParam->CurrentDir, LoadParam->FileReadFunc, ASyncThread ) ;
	if( NewHandle < 0 ) goto ENDLABEL ;

	// エラーフラグを倒す
	ErrorFlag = 0 ;

ENDLABEL :

	// エラーフラグが立っていたらモデルハンドルを解放
	if( ErrorFlag == 1 && NewHandle != -1 )
	{
		MV1SubModelBase( NewHandle ) ;
		NewHandle = -1 ;
	}

	// メモリの解放
	if( SkinNextVertIndex != NULL )
	{
		DXFREE( SkinNextVertIndex ) ;
		SkinNextVertIndex = NULL ;
	}
	if( BoneInfoDim != NULL )
	{
		for( i = 0 ; ( DWORD )i < PmxBoneNum ; i ++ )
		{
			if( BoneInfoDim[ i ].KeyMatrix != NULL )
			{
				DXFREE( BoneInfoDim[ i ].KeyMatrix ) ;
				BoneInfoDim[ i ].KeyMatrix = NULL ;
			}

			if( BoneInfoDim[ i ].KeyMatrix2 != NULL )
			{
				DXFREE( BoneInfoDim[ i ].KeyMatrix2 ) ;
				BoneInfoDim[ i ].KeyMatrix2 = NULL ;
			}
		}

		DXFREE( BoneInfoDim ) ;
		BoneInfoDim = NULL ;
	}
	if( IKInfoDim != NULL )
	{
		DXFREE( IKInfoDim ) ;
		IKInfoDim = NULL ;
	}

#ifndef DX_NON_BULLET_PHYSICS
	if( LoadParam->GParam.LoadModelToUsePhysicsMode != DX_LOADMODEL_PHYSICS_DISABLE )
	{
		if( MLPhysicsInfo.PmxPhysicsInfoDim != NULL )
		{
			DXFREE( MLPhysicsInfo.PmxPhysicsInfoDim ) ;
			MLPhysicsInfo.PmxPhysicsInfoDim = NULL ;
		}

		if( MLPhysicsInfo.PmxPhysicsJointInfoDim != NULL )
		{
			DXFREE( MLPhysicsInfo.PmxPhysicsJointInfoDim ) ;
			MLPhysicsInfo.PmxPhysicsJointInfoDim = NULL ;
		}
	}
#endif

	// 読み込みようモデルを解放
	MV1TermReadModel( &RModel ) ; 

	// 返回句柄
	return NewHandle ;
}

// 行列を計算する
static void MV1LoadModelToPMX_SetupMatrix( PMX_READ_BONE_INFO *BoneInfo, int BoneNum, int UseInitParam, int IKSkip )
{
	int i ;
	MATRIX Matrix ;

	for( i = 0 ; i < BoneNum ; i ++, BoneInfo ++ )
	{
		float x2 ;
		float y2 ;
		float z2 ;
		float xy ;
		float yz ;
		float zx ;
		float xw ;
		float yw ;
		float zw ;

		if( IKSkip && BoneInfo->IsIK ) continue ;

		if( UseInitParam )
		{
			x2 = BoneInfo->InitRotate.x * BoneInfo->InitRotate.x * 2.0f ;
			y2 = BoneInfo->InitRotate.y * BoneInfo->InitRotate.y * 2.0f ;
			z2 = BoneInfo->InitRotate.z * BoneInfo->InitRotate.z * 2.0f ;
			xy = BoneInfo->InitRotate.x * BoneInfo->InitRotate.y * 2.0f ;
			yz = BoneInfo->InitRotate.y * BoneInfo->InitRotate.z * 2.0f ;
			zx = BoneInfo->InitRotate.z * BoneInfo->InitRotate.x * 2.0f ;
			xw = BoneInfo->InitRotate.x * BoneInfo->InitRotate.w * 2.0f ;
			yw = BoneInfo->InitRotate.y * BoneInfo->InitRotate.w * 2.0f ;
			zw = BoneInfo->InitRotate.z * BoneInfo->InitRotate.w * 2.0f ;

			BoneInfo->IKQuat = BoneInfo->InitRotate ;

			Matrix.m[ 3 ][ 0 ] = BoneInfo->InitTranslate.x ;
			Matrix.m[ 3 ][ 1 ] = BoneInfo->InitTranslate.y ;
			Matrix.m[ 3 ][ 2 ] = BoneInfo->InitTranslate.z ;
			Matrix.m[ 3 ][ 3 ] = 1.0f ;
		}
		else
		{
			x2 = BoneInfo->Rotate.x * BoneInfo->Rotate.x * 2.0f ;
			y2 = BoneInfo->Rotate.y * BoneInfo->Rotate.y * 2.0f ;
			z2 = BoneInfo->Rotate.z * BoneInfo->Rotate.z * 2.0f ;
			xy = BoneInfo->Rotate.x * BoneInfo->Rotate.y * 2.0f ;
			yz = BoneInfo->Rotate.y * BoneInfo->Rotate.z * 2.0f ;
			zx = BoneInfo->Rotate.z * BoneInfo->Rotate.x * 2.0f ;
			xw = BoneInfo->Rotate.x * BoneInfo->Rotate.w * 2.0f ;
			yw = BoneInfo->Rotate.y * BoneInfo->Rotate.w * 2.0f ;
			zw = BoneInfo->Rotate.z * BoneInfo->Rotate.w * 2.0f ;

			BoneInfo->IKQuat = BoneInfo->Rotate ;

			Matrix.m[ 3 ][ 0 ] = BoneInfo->Translate.x ;
			Matrix.m[ 3 ][ 1 ] = BoneInfo->Translate.y ;
			Matrix.m[ 3 ][ 2 ] = BoneInfo->Translate.z ;
			Matrix.m[ 3 ][ 3 ] = 1.0f ;
		}

		Matrix.m[ 0 ][ 0 ] = 1.0f - y2 - z2 ;
		Matrix.m[ 0 ][ 1 ] = xy + zw ;
		Matrix.m[ 0 ][ 2 ] = zx - yw ;
		Matrix.m[ 0 ][ 3 ] = 0.0f ;
		Matrix.m[ 1 ][ 0 ] = xy - zw ;
		Matrix.m[ 1 ][ 1 ] = 1.0f - z2 - x2 ;
		Matrix.m[ 1 ][ 2 ] = yz + xw ;
		Matrix.m[ 1 ][ 3 ] = 0.0f ;
		Matrix.m[ 2 ][ 0 ] = zx + yw ;
		Matrix.m[ 2 ][ 1 ] = yz - xw ;
		Matrix.m[ 2 ][ 2 ] = 1.0f - x2 - y2 ;
		Matrix.m[ 2 ][ 3 ] = 0.0f ;

		if( BoneInfo->Frame->Parent == NULL )
		{
			BoneInfo->LocalWorldMatrix = Matrix ;
		}
		else
		{
			MV1LoadModelToVMD_CreateMultiplyMatrix( &BoneInfo->LocalWorldMatrix, &Matrix, &( ( PMX_READ_BONE_INFO * )BoneInfo->Frame->Parent->UserData )->LocalWorldMatrix ) ;
		}
	}
}

// ＩＫを計算する
static void MV1LoadModelToPMX_SetupIK( PMX_READ_BONE_INFO *BoneInfo, PMX_READ_IK_INFO *IKInfoFirst )
{
	int i, j, k ;
	PMX_READ_BONE_INFO *ChainBone ;
	PMX_READ_BONE_INFO *ChainParentBone ;
	VECTOR IKBonePos ;
	VECTOR BonePos ;
	VECTOR TargPos ;
	float Rot ;
	float Cos ;
	float IKsin, IKcos ;
	float Dummy ;
	FLOAT4 qIK ;
	PMX_READ_IK_INFO *IKInfo ;

	for( IKInfo = IKInfoFirst ; IKInfo ; IKInfo = IKInfo->Next )
	{
		// IKのターゲットのワールド座標を取得しておく
		IKBonePos.x = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 0 ];
		IKBonePos.y = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 1 ];
		IKBonePos.z = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 2 ];

		QuatConvertToMatrix( IKInfo->TargetBone->IKmat, IKInfo->TargetBone->IKQuat, IKInfo->TargetBone->Translate );

		int ikt = IKInfo->Base->LoopNum / 2 ;
		for( i = 0 ; i < IKInfo->Base->LoopNum ; i ++ )
		{
			for( j = 0 ; j < IKInfo->Base->LinkNum ; j ++ )
			{
				ChainBone = &BoneInfo[ IKInfo->Base->Link[ j ].BoneIndex ] ;
				ChainParentBone = ( PMX_READ_BONE_INFO * )ChainBone->Frame->Parent->UserData ;

				// ターゲットボーンと同じボーンだったらデータのエラーなので無視
				if( ChainBone == IKInfo->TargetBone )
					continue ;

				// calculate IK bone position
				TargPos.x = IKInfo->TargetBone->LocalWorldMatrix.m[3][0];
				TargPos.y = IKInfo->TargetBone->LocalWorldMatrix.m[3][1];
				TargPos.z = IKInfo->TargetBone->LocalWorldMatrix.m[3][2];

				// calculate [k]th bone position
				BonePos.x = ChainBone->LocalWorldMatrix.m[3][0];
				BonePos.y = ChainBone->LocalWorldMatrix.m[3][1];
				BonePos.z = ChainBone->LocalWorldMatrix.m[3][2];

				// calculate [k]th bone quaternion
				VECTOR v1;
				v1.x = BonePos.x - TargPos.x;
				v1.y = BonePos.y - TargPos.y;
				v1.z = BonePos.z - TargPos.z;

				VECTOR v2;
				v2.x = BonePos.x - IKBonePos.x;
				v2.y = BonePos.y - IKBonePos.y;
				v2.z = BonePos.z - IKBonePos.z;

				v1 = VNorm( v1 );
				v2 = VNorm( v2 );
				if( ( v1.x - v2.x ) * ( v1.x - v2.x ) + ( v1.y - v2.y ) * ( v1.y - v2.y ) + ( v1.z - v2.z ) * ( v1.z - v2.z ) < 0.0000001f ) break;

				VECTOR v;
				v = VCross( v1, v2 );

				// calculate roll axis
				if( IKInfo->Base->Link[ j ].RotLockFlag == 1 && i < ikt )
				{
					if( IKInfo->Base->Link[ j ].RotLockMin[ 1 ] == 0 && IKInfo->Base->Link[ j ].RotLockMax[ 1 ] == 0 && IKInfo->Base->Link[ j ].RotLockMin[ 2 ] == 0 && IKInfo->Base->Link[ j ].RotLockMax[ 2 ] == 0 )
					{
						float vvx = v.x * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 2 ] ;
						v.x = vvx >= 0.0f ? 1.0f : -1.0f ;
						v.y = 0.0f ;
						v.z = 0.0f ;
					}
					else
					if( IKInfo->Base->Link[ j ].RotLockMin[ 0 ] == 0 && IKInfo->Base->Link[ j ].RotLockMax[ 0 ] == 0 && IKInfo->Base->Link[ j ].RotLockMin[ 2 ] == 0 && IKInfo->Base->Link[ j ].RotLockMax[ 2 ] == 0 )
					{
						float vvy = v.x * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 2 ] ;
						v.y = vvy >= 0.0f ? 1.0f : -1.0f ;
						v.x = 0.0f ;
						v.z = 0.0f ;
					}
					else
					if( IKInfo->Base->Link[ j ].RotLockMin[ 0 ] == 0 && IKInfo->Base->Link[ j ].RotLockMax[ 0 ] == 0 && IKInfo->Base->Link[ j ].RotLockMin[ 1 ] == 0 && IKInfo->Base->Link[ j ].RotLockMax[ 1 ] == 0 )
					{
						float vvz = v.x * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 2 ] ;
						v.z = vvz >= 0.0f ? 1.0f : -1.0f ;
						v.x = 0.0f ;
						v.y = 0.0f ;
					}
					else
					{
						// calculate roll axis
						VECTOR vv;

						vv.x = v.x * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 2 ] ;
						vv.y = v.x * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 2 ] ;
						vv.z = v.x * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 2 ] ;

						v = VNorm( vv ) ;
					}
				}
				else
				{
					// calculate roll axis
					VECTOR vv;
					vv.x = v.x * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 2 ] ;
					vv.y = v.x * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 1 ][ 2 ] ;
					vv.z = v.x * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 2 ][ 2 ] ;

					v = VNorm( vv );
				}

				// calculate roll angle of [k]th bone(limited by p_IK[i].dlimit*(k+1)*2)
				Cos = v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; 
				if( Cos >  1.0f ) Cos =  1.0f;
				if( Cos < -1.0f ) Cos = -1.0f;

				Rot = 0.5f * _ACOS( Cos );
				if( Rot > IKInfo->Base->RotLimit * ( j + 1 ) * 2 )
					Rot = IKInfo->Base->RotLimit * ( j + 1 ) * 2 ;

				_SINCOS( Rot, &IKsin, &IKcos );
				qIK.x = v.x * IKsin;
				qIK.y = v.y * IKsin;
				qIK.z = v.z * IKsin;
				qIK.w = IKcos;

				ChainBone->IKQuat = QuatMul( qIK, ChainBone->IKQuat );

				QuatConvertToMatrix( ChainBone->IKmat, ChainBone->IKQuat, ChainBone->Translate );

				if( IKInfo->Base->Link[ j ].RotLockFlag == 1 )
				{
					// 軸回転角度を算出
					if( ( IKInfo->Base->Link[ j ].RotLockMin[ 0 ] > -1.570796f ) & ( IKInfo->Base->Link[ j ].RotLockMax[ 0 ] < 1.570796f ) )
					{
						// Z*X*Y順
						// X軸回り
						float fLimit = 1.535889f ;			// 88.0f/180.0f*3.14159265f;
						float fSX = -ChainBone->IKmat.m[ 2 ][ 1 ] ;				// sin(θx)
						float fX  = _ASIN( fSX ) ;			// X軸回り決定
						float fCX ;
						_SINCOS( fX, &Dummy, &fCX ) ;

						// ジンバルロック回避
						if( _FABS( fX ) > fLimit )
						{
							fX  = ( fX < 0 ) ? - fLimit : fLimit ;
							_SINCOS( fX, &Dummy, &fCX ) ;
						}

						// Y軸回り
						float fSY = ChainBone->IKmat.m[ 2 ][ 0 ] / fCX ;
						float fCY = ChainBone->IKmat.m[ 2 ][ 2 ] / fCX ;
						float fY = _ATAN2( fSY, fCY ) ;	// Y軸回り決定

						// Z軸回り
						float fSZ = ChainBone->IKmat.m[ 0 ][ 1 ] / fCX ;
						float fCZ = ChainBone->IKmat.m[ 1 ][ 1 ] / fCX ;
						float fZ = _ATAN2( fSZ, fCZ ) ;

						// 角度の制限
						if( fX < IKInfo->Base->Link[ j ].RotLockMin[ 0 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMin[ 0 ] - fX ;
							fX = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 0 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 0 ] ;
						}
						if( fX > IKInfo->Base->Link[ j ].RotLockMax[ 0 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMax[ 0 ] - fX ;
							fX = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 0 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 0 ] ;
						}
						if( fY < IKInfo->Base->Link[ j ].RotLockMin[ 1 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMin[ 1 ] - fY ;
							fY = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 1 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 1 ] ;
						}
						if( fY > IKInfo->Base->Link[ j ].RotLockMax[ 1 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMax[ 1 ] - fY ;
							fY = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 1 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 1 ] ;
						}
						if( fZ < IKInfo->Base->Link[ j ].RotLockMin[ 2 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMin[ 2 ] - fZ ;
							fZ = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 2 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 2 ] ;
						}
						if( fZ > IKInfo->Base->Link[ j ].RotLockMax[ 2 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMax[ 2 ] - fZ ;
							fZ = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 2 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 2 ] ;
						}

						// 決定した角度でベクトルを回転
						MATRIX mX, mY, mZ, mT ;

						CreateRotationXMatrix( &mX, fX ) ;
						CreateRotationXMatrix( &mY, fY ) ;
						CreateRotationXMatrix( &mZ, fZ ) ;

						MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly(               &mT, &mZ, &mX ) ;
						MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly( &ChainBone->IKmat, &mT, &mY ) ;
					}
					else
					if((IKInfo->Base->Link[ j ].RotLockMin[ 1 ]>-1.570796f)&(IKInfo->Base->Link[ j ].RotLockMax[ 1 ]<1.570796f))
					{
						// X*Y*Z順
						// Y軸回り
						float fLimit = 1.535889f ;		// 88.0f/180.0f*3.14159265f;
						float fSY = -ChainBone->IKmat.m[ 0 ][ 2 ] ;			// sin(θy)
						float fY  = _ASIN( fSY ) ;	// Y軸回り決定
						float fCY ;
						_SINCOS( fY, &Dummy, &fCY ) ;

						// ジンバルロック回避
						if( _FABS( fY ) > fLimit )
						{
							fY = ( fY < 0 ) ? - fLimit : fLimit ;
							_SINCOS( fY, &Dummy, &fCY ) ;
						}

						// X軸回り
						float fSX = ChainBone->IKmat.m[ 1 ][ 2 ] / fCY ;
						float fCX = ChainBone->IKmat.m[ 2 ][ 2 ] / fCY ;
						float fX  = _ATAN2( fSX, fCX ) ;	// X軸回り決定

						// Z軸回り
						float fSZ = ChainBone->IKmat.m[ 0 ][ 1 ] / fCY ;
						float fCZ = ChainBone->IKmat.m[ 0 ][ 0 ] / fCY ;
						float fZ  = _ATAN2( fSZ, fCZ ) ;	// Z軸回り決定

						// 角度の制限
						if( fX < IKInfo->Base->Link[ j ].RotLockMin[ 0 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMin[ 0 ] - fX ;
							fX = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 0 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 0 ] ;
						}
						if( fX > IKInfo->Base->Link[ j ].RotLockMax[ 0 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMax[ 0 ] - fX ;
							fX = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 0 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 0 ] ;
						}
						if( fY < IKInfo->Base->Link[ j ].RotLockMin[ 1 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMin[ 1 ] - fY ;
							fY = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 1 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 1 ] ;
						}
						if( fY > IKInfo->Base->Link[ j ].RotLockMax[ 1 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMax[ 1 ] - fY ;
							fY = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 1 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 1 ] ;
						}
						if( fZ < IKInfo->Base->Link[ j ].RotLockMin[ 2 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMin[ 2 ] - fZ ;
							fZ = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 2 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 2 ] ;
						}
						if( fZ > IKInfo->Base->Link[ j ].RotLockMax[ 2 ] )
						{
							float tf = 2 * IKInfo->Base->Link[ j ].RotLockMax[ 2 ] - fZ ;
							fZ = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 2 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 2 ] ;
						}

						// 決定した角度でベクトルを回転
						MATRIX mX, mY, mZ, mT ;

						CreateRotationXMatrix( &mX, fX ) ;
						CreateRotationXMatrix( &mY, fY ) ;
						CreateRotationXMatrix( &mZ, fZ ) ;

						MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly(               &mT, &mX, &mY ) ;
						MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly( &ChainBone->IKmat, &mT, &mZ ) ;
					}
					else
					{
						// Y*Z*X順
						// Z軸回り
						float fLimit = 1.535889f ;		// 88.0f/180.0f*3.14159265f;
						float fSZ = -ChainBone->IKmat.m[ 1 ][ 0 ] ;			// sin(θy)
						float fZ  = _ASIN( fSZ ) ;	// Y軸回り決定
						float fCZ ;
						_SINCOS( fZ, &Dummy, &fCZ ) ;

						// ジンバルロック回避
						if( _FABS( fZ ) > fLimit )
						{
							fZ = ( fZ < 0 ) ? - fLimit : fLimit ;
							_SINCOS( fZ, &Dummy, &fCZ ) ;
						}

						// X軸回り
						float fSX = ChainBone->IKmat.m[ 1 ][ 2 ] / fCZ ;
						float fCX = ChainBone->IKmat.m[ 1 ][ 1 ] / fCZ ;
						float fX  = _ATAN2( fSX, fCX ) ;	// X軸回り決定

						// Y軸回り
						float fSY = ChainBone->IKmat.m[ 2 ][ 0 ] / fCZ ;
						float fCY = ChainBone->IKmat.m[ 0 ][ 0 ] / fCZ ;
						float fY  = _ATAN2( fSY, fCY ) ;	// Z軸回り決定

						// 角度の制限
						if( fX < IKInfo->Base->Link[ j ].RotLockMin[ 0 ] )
						{
							float tf= 2 * IKInfo->Base->Link[ j ].RotLockMin[ 0 ] - fX;
							fX = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 0 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 0 ] ;
						}
						if( fX > IKInfo->Base->Link[ j ].RotLockMax[ 0 ] )
						{
							float tf= 2 * IKInfo->Base->Link[ j ].RotLockMax[ 0 ] - fX;
							fX = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 0 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 0 ] ;
						}
						if( fY < IKInfo->Base->Link[ j ].RotLockMin[ 1 ] )
						{
							float tf= 2 * IKInfo->Base->Link[ j ].RotLockMin[ 1 ] - fY;
							fY = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 1 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 1 ] ;
						}
						if( fY > IKInfo->Base->Link[ j ].RotLockMax[ 1 ] )
						{
							float tf= 2 * IKInfo->Base->Link[ j ].RotLockMax[ 1 ] - fY;
							fY = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 1 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 1 ] ;
						}
						if( fZ < IKInfo->Base->Link[ j ].RotLockMin[ 2 ] )
						{
							float tf= 2 * IKInfo->Base->Link[ j ].RotLockMin[ 2 ] - fZ;
							fZ = tf <= IKInfo->Base->Link[ j ].RotLockMax[ 2 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMin[ 2 ] ;
						}
						if( fZ > IKInfo->Base->Link[ j ].RotLockMax[ 2 ] )
						{
							float tf= 2 * IKInfo->Base->Link[ j ].RotLockMax[ 2 ] - fZ;
							fZ = tf >= IKInfo->Base->Link[ j ].RotLockMin[ 2 ] && j < ikt ? tf : IKInfo->Base->Link[ j ].RotLockMax[ 2 ] ;
						}

						// 決定した角度でベクトルを回転
						MATRIX mX, mY, mZ, mT ;

						CreateRotationXMatrix( &mX, fX ) ;
						CreateRotationXMatrix( &mY, fY ) ;
						CreateRotationXMatrix( &mZ, fZ ) ;

						MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly(               &mT, &mY, &mZ ) ;
						MV1LoadModelToVMD_CreateMultiplyMatrixRotOnly( &ChainBone->IKmat, &mT, &mX ) ;
					}
					QuatConvertFromMatrix( ChainBone->IKQuat, ChainBone->IKmat ) ;
				}

				for( k = j ; k >= 0 ; k-- )
				{
					ChainBone= &BoneInfo[ IKInfo->Base->Link[ k ].BoneIndex ];
					MV1LoadModelToVMD_CreateMultiplyMatrix( &ChainBone->LocalWorldMatrix, &ChainBone->IKmat, &( ( PMX_READ_BONE_INFO * )ChainBone->Frame->Parent->UserData )->LocalWorldMatrix ) ;
				}

				MV1LoadModelToVMD_CreateMultiplyMatrix( &IKInfo->TargetBone->LocalWorldMatrix, &IKInfo->TargetBone->IKmat, &( ( PMX_READ_BONE_INFO * )IKInfo->TargetBone->Frame->Parent->UserData )->LocalWorldMatrix );
			}

			for( j = 0 ; j < IKInfo->Base->LinkNum ; j ++ )
			{
				ChainBone = &BoneInfo[ IKInfo->Base->Link[ j ].BoneIndex ] ;

				ChainBone->Rotate = ChainBone->IKQuat ;
			}
			IKInfo->TargetBone->Rotate = IKInfo->TargetBone->IKQuat ;
		}
	}
}


// 指定のボーンにアニメーションの指定キーのパラメータを反映させる
static void MV1LoadModelToPMX_SetupOneBoneMatrixFormAnimKey( PMX_READ_BONE_INFO *BoneInfo, int Time, int LoopNo )
{
	VMD_READ_KEY_INFO *NowKey, *NextKey ;

	if( BoneInfo->IsIK || BoneInfo->IsAddParent )
	{
		if( BoneInfo->IsIKAnim == FALSE || LoopNo != 0 )
		{
			BoneInfo->Translate = BoneInfo->KeyPos[ Time ] ;
			BoneInfo->Rotate    = BoneInfo->KeyRot[ Time ] ;
		}
		else
		{
			BoneInfo->Translate = BoneInfo->Frame->Translate ;
			BoneInfo->Rotate.x  = 0.0f ;
			BoneInfo->Rotate.y  = 0.0f ;
			BoneInfo->Rotate.z  = 0.0f ;
			BoneInfo->Rotate.w  = 1.0f ;
		}
	}
	else
	if( BoneInfo->IsIK == 0 && BoneInfo->Anim != NULL )
	{
		// キーの準備
		if( BoneInfo->NowKey->Next == NULL )
		{
			NowKey = BoneInfo->NowKey ;
			NextKey = NULL ;
		}
		else
		if( BoneInfo->NowKey->Next->Frame * 2 <= ( DWORD )Time )
		{
			BoneInfo->NowKey = BoneInfo->NowKey->Next ;
			NowKey = BoneInfo->NowKey ;
			NextKey = NowKey->Next ;
		}
		else
		{
			NowKey = BoneInfo->NowKey ;
			NextKey = NowKey->Next ;
		}

		// 次のキーがないか現在のキーのフレーム以下の場合は現在のフレームの値そのまま
		if( NextKey == NULL || NowKey->Frame * 2 >= ( DWORD )Time )
		{
			BoneInfo->Translate = NowKey->MVRPosKey[ 0 ] ;
			BoneInfo->Rotate = NowKey->MVRRotKey[ 0 ] ;
		}
		else
		{
			int KeyNo ;

			// あらかじめ算出した補間値をセット
			KeyNo = Time - NowKey->Frame * 2 ;
			BoneInfo->Translate = NowKey->MVRPosKey[ KeyNo ] ;
			BoneInfo->Rotate    = NowKey->MVRRotKey[ KeyNo ] ;
		}
	}
}

// ＶＭＤファイルを読み込む( -1:エラー )
static int _MV1LoadModelToVMD_PMX(
	int								DataIndex,
	MV1_MODEL_R *					RModel,
	void *							DataBuffer,
	int								DataSize,
	const char *					Name,
	PMX_READ_BONE_INFO *			PmxBoneInfo,
	int								PmxBoneNum,
	PMX_READ_IK_INFO *				PmxIKInfoFirst,
#ifndef DX_NON_BULLET_PHYSICS
	DX_MODELLOADER3_PMX_PHYSICS_INFO *	MLPhysicsInfo,
#endif
	bool							FPS60
)
{
	int i, j, k ;
	VMD_READ_NODE_INFO *VmdNode ;
	VMD_READ_KEY_INFO *VmdKey, *VmdKeyTemp1, *VmdKeyTemp2 ;
	VMD_READ_FACE_KEY_SET_INFO *VmdFaceKeySet ;
	VMD_READ_FACE_KEY_INFO *VmdFaceKey ;
	PMX_READ_BONE_INFO *BoneInfo ;
	MV1_SHAPE_R *Shape ;
	MV1_FRAME_R *Frame ;
	MV1_ANIM_R *Anim ;
	MV1_ANIMSET_R *AnimSet ;
	MV1_ANIMKEYSET_R *KeyPosSet, *KeyRotSet, *KeyFactorSet ;
	float *KeyPosTime, *KeyRotTime, *KeyFactorTime ;
	float *KeyFactor ;
	VECTOR *KeyPos ;
	FLOAT4 *KeyRot ;
	VMD_READ_INFO VmdData ;
	char String[ 256 ] ;

	// 基本情報の読み込み
	if( LoadVMDBaseData( &VmdData, DataBuffer, DataSize ) < 0 )
		return -1 ;

#ifndef DX_NON_BULLET_PHYSICS
	if( MLPhysicsInfo )
	{
		// 物理オブジェクトの準備をする
		SetupPhysicsObject_PMXPhysicsInfo( MLPhysicsInfo ) ;
	}
#endif

	// アニメーションセットを追加
	AnimSet = MV1RAddAnimSet( RModel, Name ) ;
	if( AnimSet == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションセットの追加に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// ボーンのアニメーションポインタの初期化
	BoneInfo = PmxBoneInfo ;
	for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
	{
		BoneInfo->Anim = NULL ;
		BoneInfo->Node = NULL ;
	}

	AnimSet->StartTime = 0.0f ;
	AnimSet->EndTime = ( float )VmdData.MaxTime ;

	// ノードの数だけ繰り返し
	VmdNode = VmdData.Node ;
	for( i = 0 ; ( DWORD )i < VmdData.NodeNum ; i ++, VmdNode ++ )
	{
		// フレームの検索
		for( Frame = RModel->FrameFirst ; Frame && _STRCMP( Frame->Name, VmdNode->Name ) != 0 ; Frame = Frame->DataNext ){}
		if( Frame == NULL ) continue ;

		BoneInfo = ( PMX_READ_BONE_INFO * )Frame->UserData ;

		// アニメーションの追加
		Anim = MV1RAddAnim( RModel, AnimSet ) ;
		if( Anim == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// ボーンに情報セット
		BoneInfo->Anim = Anim ;
		BoneInfo->Node = VmdNode ;

		// 対象ノードのセット
		Anim->TargetFrameName = MV1RAddString( RModel, Frame->Name ) ;
		Anim->TargetFrameIndex = Frame->Index ;

		// 最大時間をセット
		if( BoneInfo->IsIK || BoneInfo->IsAddParent )
		{
			Anim->MaxTime = ( float )VmdData.MaxTime ;
		}
		else
		{
			Anim->MaxTime = ( float )VmdNode->MaxFrame ;
		}

		// 最大時間と最小時間を更新
//		if( AnimSet->StartTime > ( float )VmdNode->MinFrame ) AnimSet->StartTime = ( float )VmdNode->MinFrame ;
//		if( AnimSet->EndTime   < ( float )VmdNode->MaxFrame ) AnimSet->EndTime   = ( float )VmdNode->MaxFrame ;

		// アニメーションキーの情報をセット
		KeyPosSet = MV1RAddAnimKeySet( RModel, Anim ) ;
		if( KeyPosSet == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーション座標キーセットの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		KeyRotSet = MV1RAddAnimKeySet( RModel, Anim ) ;
		if( KeyRotSet == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーション回転キーセットの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		KeyPosSet->Type = MV1_ANIMKEY_TYPE_VECTOR ;
		KeyPosSet->DataType = MV1_ANIMKEY_DATATYPE_TRANSLATE ;
		KeyPosSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
		KeyPosSet->TotalTime = ( float )VmdNode->MaxFrame ;
		KeyPosSet->Num = BoneInfo->IsIK || BoneInfo->IsAddParent ? VmdData.MaxTime : VmdNode->MaxFrame - VmdNode->MinFrame ;
		KeyPosSet->Num *= 2 ;
		KeyPosSet->Num += 1 ;

		KeyRotSet->Type = MV1_ANIMKEY_TYPE_QUATERNION_VMD ;
		KeyRotSet->DataType = MV1_ANIMKEY_DATATYPE_ROTATE ;
		KeyRotSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
		KeyRotSet->TotalTime = ( float )VmdNode->MaxFrame ;
		KeyRotSet->Num = KeyPosSet->Num ;

		KeyPosSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyPosSet->Num, &RModel->Mem ) ;
		if( KeyPosSet->KeyTime == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		KeyPosSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * KeyPosSet->Num, &RModel->Mem ) ;
		if( KeyPosSet->KeyVector == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		KeyRotSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyRotSet->Num, &RModel->Mem ) ;
		if( KeyRotSet->KeyTime == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		KeyRotSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( FLOAT4 ) * KeyRotSet->Num, &RModel->Mem ) ;
		if( KeyRotSet->KeyVector == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		RModel->AnimKeyDataSize += KeyRotSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( FLOAT4 ) ) ;

		VmdKey = VmdNode->FirstKey ;
		BoneInfo->KeyPos     = KeyPos     = KeyPosSet->KeyVector ;
		BoneInfo->KeyRot     = KeyRot     = KeyRotSet->KeyFloat4 ;
		BoneInfo->KeyPosTime = KeyPosTime = KeyPosSet->KeyTime ;
		BoneInfo->KeyRotTime = KeyRotTime = KeyRotSet->KeyTime ;
		if( VmdNode->KeyNum == 1 )
		{
			if( BoneInfo->IsIK || BoneInfo->IsAddParent )
			{
				for( j = 0 ; j < KeyRotSet->Num ; j ++ )
				{
					*KeyPosTime = ( float )j / 2.0f ;
					*KeyRotTime = ( float )j / 2.0f ;
					KeyPos->x = VmdKey->Position[ 0 ] + Frame->Translate.x ;
					KeyPos->y = VmdKey->Position[ 1 ] + Frame->Translate.y ;
					KeyPos->z = VmdKey->Position[ 2 ] + Frame->Translate.z ;
					KeyRot->x = VmdKey->Quaternion[ 0 ] ;
					KeyRot->y = VmdKey->Quaternion[ 1 ] ;
					KeyRot->z = VmdKey->Quaternion[ 2 ] ;
					KeyRot->w = VmdKey->Quaternion[ 3 ] ;
					QuaternionNormalize( KeyRot, KeyRot ) ;
					KeyPos ++ ;
					KeyRot ++ ;
					KeyPosTime ++ ;
					KeyRotTime ++ ;
				}
			}
			else
			{
				VmdKey->MVRPosKey = KeyPos ;
				VmdKey->MVRRotKey = KeyRot ;
				*KeyPosTime = ( float )VmdKey->Frame ;
				*KeyRotTime = ( float )VmdKey->Frame ;
				KeyPos->x = VmdKey->Position[ 0 ] + Frame->Translate.x ;
				KeyPos->y = VmdKey->Position[ 1 ] + Frame->Translate.y ;
				KeyPos->z = VmdKey->Position[ 2 ] + Frame->Translate.z ;
				KeyRot->x = VmdKey->Quaternion[ 0 ] ;
				KeyRot->y = VmdKey->Quaternion[ 1 ] ;
				KeyRot->z = VmdKey->Quaternion[ 2 ] ;
				KeyRot->w = VmdKey->Quaternion[ 3 ] ;
				QuaternionNormalize( KeyRot, KeyRot ) ;
			}
		}
		else
		{
			if( ( BoneInfo->IsIK || BoneInfo->IsAddParent ) && VmdKey->Frame > 0 )
			{
				for( j = 0 ; ( DWORD )j < VmdKey->Frame * 2 ; j ++ )
				{
					*KeyPosTime = ( float )VmdKey->Frame ;
					*KeyRotTime = ( float )VmdKey->Frame ;
					KeyPos->x = VmdKey->Position[ 0 ] + Frame->Translate.x ;
					KeyPos->y = VmdKey->Position[ 1 ] + Frame->Translate.y ;
					KeyPos->z = VmdKey->Position[ 2 ] + Frame->Translate.z ;
					KeyRot->x = VmdKey->Quaternion[ 0 ] ;
					KeyRot->y = VmdKey->Quaternion[ 1 ] ;
					KeyRot->z = VmdKey->Quaternion[ 2 ] ;
					KeyRot->w = VmdKey->Quaternion[ 3 ] ;
					QuaternionNormalize( KeyRot, KeyRot ) ;
					KeyPos ++ ;
					KeyRot ++ ;
					KeyPosTime ++ ;
					KeyRotTime ++ ;
				}
			}

			for( j = 0 ; ( DWORD )j < VmdNode->KeyNum ; j ++, VmdKey = VmdKey->Next )
			{
				int XLinear, YLinear, ZLinear, RLinear ;
				float XX1, XY1, XX2, XY2 ;
				float YX1, YY1, YX2, YY2 ;
				float ZX1, ZY1, ZX2, ZY2 ;
				float RX1, RY1, RX2, RY2 ;

				if( j == 0 ) continue ;

				VmdKey->Prev->MVRPosKey = KeyPos ;
				VmdKey->Prev->MVRRotKey = KeyRot ;

				VmdKeyTemp1 = VmdKey->Prev ;
				VmdKeyTemp2 = VmdKey ;

				XLinear = VmdKeyTemp2->Linear[ 0 ] ;
				YLinear = VmdKeyTemp2->Linear[ 1 ] ;
				ZLinear = VmdKeyTemp2->Linear[ 2 ] ;
				RLinear = VmdKeyTemp2->Linear[ 3 ] ;

				XX1 = VmdKeyTemp2->PosXBezier[ 0 ] ;
				XY1 = VmdKeyTemp2->PosXBezier[ 1 ] ;
				XX2 = VmdKeyTemp2->PosXBezier[ 2 ] ;
				XY2 = VmdKeyTemp2->PosXBezier[ 3 ] ;
				YX1 = VmdKeyTemp2->PosYBezier[ 0 ] ;
				YY1 = VmdKeyTemp2->PosYBezier[ 1 ] ;
				YX2 = VmdKeyTemp2->PosYBezier[ 2 ] ;
				YY2 = VmdKeyTemp2->PosYBezier[ 3 ] ;
				ZX1 = VmdKeyTemp2->PosZBezier[ 0 ] ;
				ZY1 = VmdKeyTemp2->PosZBezier[ 1 ] ;
				ZX2 = VmdKeyTemp2->PosZBezier[ 2 ] ;
				ZY2 = VmdKeyTemp2->PosZBezier[ 3 ] ;
				RX1 = VmdKeyTemp2->RotBezier[ 0 ] ;
				RY1 = VmdKeyTemp2->RotBezier[ 1 ] ;
				RX2 = VmdKeyTemp2->RotBezier[ 2 ] ;
				RY2 = VmdKeyTemp2->RotBezier[ 3 ] ;

				for( k = VmdKeyTemp1->Frame * 2 ; ( DWORD )k < VmdKeyTemp2->Frame * 2 ; k ++ )
				{
					float Rate, RateH ;
					float fFrame ;

					fFrame = k / 2.0f ;
					Rate = ( fFrame - VmdKeyTemp1->Frame ) / ( float )( VmdKeyTemp2->Frame - VmdKeyTemp1->Frame ) ;
					*KeyPosTime = fFrame ;
					*KeyRotTime = fFrame ;
/*
					KeyPos->x = VmdKeyTemp1->Position[ 0 ] + ( VmdKeyTemp2->Position[ 0 ] - VmdKeyTemp1->Position[ 0 ] ) * Rate + Frame->Translate.x ;
					KeyPos->y = VmdKeyTemp1->Position[ 1 ] + ( VmdKeyTemp2->Position[ 1 ] - VmdKeyTemp1->Position[ 1 ] ) * Rate + Frame->Translate.y ;
					KeyPos->z = VmdKeyTemp1->Position[ 2 ] + ( VmdKeyTemp2->Position[ 2 ] - VmdKeyTemp1->Position[ 2 ] ) * Rate + Frame->Translate.z ;
					KeyRot->x = VmdKeyTemp1->Quaternion[ 0 ] + ( VmdKeyTemp2->Quaternion[ 0 ] - VmdKeyTemp1->Quaternion[ 0 ] ) * Rate ;
					KeyRot->y = VmdKeyTemp1->Quaternion[ 1 ] + ( VmdKeyTemp2->Quaternion[ 1 ] - VmdKeyTemp1->Quaternion[ 1 ] ) * Rate ;
					KeyRot->z = VmdKeyTemp1->Quaternion[ 2 ] + ( VmdKeyTemp2->Quaternion[ 2 ] - VmdKeyTemp1->Quaternion[ 2 ] ) * Rate ;
					KeyRot->w = VmdKeyTemp1->Quaternion[ 3 ] + ( VmdKeyTemp2->Quaternion[ 3 ] - VmdKeyTemp1->Quaternion[ 3 ] ) * Rate ;
*/
					VmdCalcLine( XLinear, Rate, RateH, XX1, XX2, XY1, XY2 ) ;
					KeyPos->x = VmdKeyTemp1->Position[ 0 ] + ( VmdKeyTemp2->Position[ 0 ] - VmdKeyTemp1->Position[ 0 ] ) * RateH + Frame->Translate.x ;

					VmdCalcLine( YLinear, Rate, RateH, YX1, YX2, YY1, YY2 ) ;
					KeyPos->y = VmdKeyTemp1->Position[ 1 ] + ( VmdKeyTemp2->Position[ 1 ] - VmdKeyTemp1->Position[ 1 ] ) * RateH + Frame->Translate.y ;

					VmdCalcLine( ZLinear, Rate, RateH, ZX1, ZX2, ZY1, ZY2 ) ;
					KeyPos->z = VmdKeyTemp1->Position[ 2 ] + ( VmdKeyTemp2->Position[ 2 ] - VmdKeyTemp1->Position[ 2 ] ) * RateH + Frame->Translate.z ;

					// 球面線形補間
					VmdCalcLine( RLinear, Rate, RateH, RX1, RX2, RY1, RY2 ) ;
					{
						float qr ;
						float t0 ;

						qr = VmdKeyTemp1->Quaternion[ 0 ] * VmdKeyTemp2->Quaternion[ 0 ] +
							 VmdKeyTemp1->Quaternion[ 1 ] * VmdKeyTemp2->Quaternion[ 1 ] +
							 VmdKeyTemp1->Quaternion[ 2 ] * VmdKeyTemp2->Quaternion[ 2 ] +
							 VmdKeyTemp1->Quaternion[ 3 ] * VmdKeyTemp2->Quaternion[ 3 ] ;
						t0 = 1.0f - RateH ;

						if( qr < 0 )
						{
							KeyRot->x = VmdKeyTemp1->Quaternion[ 0 ] * t0 - VmdKeyTemp2->Quaternion[ 0 ] * RateH ;
							KeyRot->y = VmdKeyTemp1->Quaternion[ 1 ] * t0 - VmdKeyTemp2->Quaternion[ 1 ] * RateH ;
							KeyRot->z = VmdKeyTemp1->Quaternion[ 2 ] * t0 - VmdKeyTemp2->Quaternion[ 2 ] * RateH ;
							KeyRot->w = VmdKeyTemp1->Quaternion[ 3 ] * t0 - VmdKeyTemp2->Quaternion[ 3 ] * RateH ;
						}
						else
						{
							KeyRot->x = VmdKeyTemp1->Quaternion[ 0 ] * t0 + VmdKeyTemp2->Quaternion[ 0 ] * RateH ;
							KeyRot->y = VmdKeyTemp1->Quaternion[ 1 ] * t0 + VmdKeyTemp2->Quaternion[ 1 ] * RateH ;
							KeyRot->z = VmdKeyTemp1->Quaternion[ 2 ] * t0 + VmdKeyTemp2->Quaternion[ 2 ] * RateH ;
							KeyRot->w = VmdKeyTemp1->Quaternion[ 3 ] * t0 + VmdKeyTemp2->Quaternion[ 3 ] * RateH ;
						}
						QuaternionNormalize( KeyRot, KeyRot ) ;
					}

					KeyPos ++ ;
					KeyRot ++ ;
					KeyPosTime ++ ;
					KeyRotTime ++ ;
				}
				if( ( DWORD )j == VmdNode->KeyNum - 1 )
				{
					VmdKey->MVRPosKey = KeyPos ;
					VmdKey->MVRRotKey = KeyRot ;
					*KeyPosTime = ( float )VmdKey->Frame ;
					*KeyRotTime = ( float )VmdKey->Frame ;
					KeyPos->x = VmdKey->Position[ 0 ] + Frame->Translate.x ;
					KeyPos->y = VmdKey->Position[ 1 ] + Frame->Translate.y ;
					KeyPos->z = VmdKey->Position[ 2 ] + Frame->Translate.z ;
					KeyRot->x = VmdKey->Quaternion[ 0 ] ;
					KeyRot->y = VmdKey->Quaternion[ 1 ] ;
					KeyRot->z = VmdKey->Quaternion[ 2 ] ;
					KeyRot->w = VmdKey->Quaternion[ 3 ] ;
					QuaternionNormalize( KeyRot, KeyRot ) ;
					KeyPos ++ ;
					KeyRot ++ ;
					KeyPosTime ++ ;
					KeyRotTime ++ ;
				}
			}

			if( ( BoneInfo->IsIK || BoneInfo->IsAddParent ) && KeyRot - BoneInfo->KeyRot < KeyRotSet->Num )
			{
				for( j = ( int )( KeyRot - BoneInfo->KeyRot ) ; j < KeyRotSet->Num ; j ++ )
				{
					KeyPosTime[ 0 ] = ( float )j / 2.0f ;
					KeyRotTime[ 0 ] = ( float )j / 2.0f ;
					KeyPos[ 0 ] = KeyPos[ -1 ] ;
					KeyRot[ 0 ] = KeyRot[ -1 ] ;
					KeyPos ++ ;
					KeyRot ++ ;
					KeyPosTime ++ ;
					KeyRotTime ++ ;
				}
			}
		}
	}

	// カメラのモーション情報がある場合はカメラ情報を追加する
	if( VmdData.Camera != NULL )
	{
		_SPRINTF( String, "Camera%03d", DataIndex ) ;
		if( SetupVMDCameraAnim( &VmdData, RModel, String, AnimSet ) < 0 )
			goto ENDLABEL ;
	}

	// ボーン情報がある場合のみこの先の処理を行う
	if( PmxBoneInfo != NULL )
	{
		// アニメーションの再生準備
		BoneInfo = PmxBoneInfo ;
		for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
		{
			BoneInfo->IsIKAnim = FALSE ;
			if( BoneInfo->Anim != NULL )
			{
				BoneInfo->NowKey = BoneInfo->Node->FirstKey ;
			}
			else
			if( BoneInfo->IsIK || BoneInfo->IsAddParent )
			{
				// IKボーンでアニメーションがない場合はキーを打つ準備をする
				if( BoneInfo->Anim == NULL )
				{
					BoneInfo->IsIKAnim = TRUE ;

					// アニメーションの追加
					BoneInfo->Anim = MV1RAddAnim( RModel, AnimSet ) ;
					if( BoneInfo->Anim == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					// 対象ノードのセット
					BoneInfo->Anim->TargetFrameName = MV1RAddString( RModel, BoneInfo->Frame->Name ) ;
					BoneInfo->Anim->TargetFrameIndex = BoneInfo->Frame->Index ;
					BoneInfo->Anim->MaxTime = ( float )VmdData.MaxTime ;

					// アニメーションキーの情報をセット
					KeyPosSet = MV1RAddAnimKeySet( RModel, BoneInfo->Anim ) ;
					if( KeyPosSet == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyRotSet = MV1RAddAnimKeySet( RModel, BoneInfo->Anim ) ;
					if( KeyRotSet == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					KeyPosSet->Type = MV1_ANIMKEY_TYPE_VECTOR ;
					KeyPosSet->DataType = MV1_ANIMKEY_DATATYPE_TRANSLATE ;
					KeyPosSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
					KeyPosSet->TotalTime = ( float )VmdData.MaxTime ;
					KeyPosSet->Num = VmdData.MaxTime * 2 + 1 ;

					KeyRotSet->Type = MV1_ANIMKEY_TYPE_QUATERNION_VMD ;
					KeyRotSet->DataType = MV1_ANIMKEY_DATATYPE_ROTATE ;
					KeyRotSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
					KeyRotSet->TotalTime = ( float )VmdData.MaxTime ;
					KeyRotSet->Num = VmdData.MaxTime * 2 + 1 ;

					KeyPosSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyPosSet->Num, &RModel->Mem ) ;
					if( KeyPosSet->KeyTime == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyPosSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * KeyPosSet->Num, &RModel->Mem ) ;
					if( KeyPosSet->KeyVector == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					KeyRotSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyRotSet->Num, &RModel->Mem ) ;
					if( KeyRotSet->KeyTime == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyRotSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( FLOAT4 ) * KeyRotSet->Num, &RModel->Mem ) ;
					if( KeyRotSet->KeyVector == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					RModel->AnimKeyDataSize += KeyRotSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( FLOAT4 ) ) ;

					BoneInfo->KeyPos = KeyPosSet->KeyVector ;
					BoneInfo->KeyRot = KeyRotSet->KeyFloat4 ;
					BoneInfo->KeyPosTime = KeyPosSet->KeyTime ;
					BoneInfo->KeyRotTime = KeyRotSet->KeyTime ;
				}
			}
			else
			{
				BoneInfo->Translate = BoneInfo->Frame->Translate ;
				BoneInfo->Rotate.x = 0.0f ;
				BoneInfo->Rotate.y = 0.0f ;
				BoneInfo->Rotate.z = 0.0f ;
				BoneInfo->Rotate.w = 1.0f ;
			}

			if( BoneInfo->IsIKChild )
			{
				BoneInfo->KeyMatrix2 = ( MATRIX * )DXALLOC( sizeof( MATRIX ) * ( ( FPS60 ? VmdData.MaxTime * 2 : VmdData.MaxTime ) + 1 ) ) ;
				if( BoneInfo->KeyMatrix2 == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーション行列キーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}
			}
		}

#ifndef DX_NON_BULLET_PHYSICS
		if( MLPhysicsInfo )
		{
			BoneInfo = PmxBoneInfo ;
			for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
			{
				if( BoneInfo->IsPhysics == 0 ) continue ;

				BoneInfo->KeyMatrix = ( MATRIX * )DXALLOC( sizeof( MATRIX ) * ( ( FPS60 ? VmdData.MaxTime * 2 : VmdData.MaxTime ) + 1 ) ) ;
				if( BoneInfo->KeyMatrix == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーション行列キーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}
				if( MLPhysicsInfo->LoopMotionFlag )
				{
					_MEMSET( BoneInfo->KeyMatrix, 0, sizeof( MATRIX ) * ( ( FPS60 ? VmdData.MaxTime * 2 : VmdData.MaxTime ) + 1 ) ) ;
				}
			}
		}
#endif

		// アニメーションを再生する
		bool IKSkip ;
		int PlayLoopNum ;
		int LoopNo ;
		int TimeNo ;

#ifndef DX_NON_BULLET_PHYSICS
		if( MLPhysicsInfo )
		{
			PlayLoopNum = MLPhysicsInfo->LoopMotionFlag ? MLPhysicsInfo->LoopMotionNum : 1 ;
			MLPhysicsInfo->MotionTotalFrameNum = VmdData.MaxTime * 2 + 1 ;
		}
		else
		{
			PlayLoopNum = 1 ;
		}
#else
		PlayLoopNum = 1 ;
#endif
		for( LoopNo = 0 ; LoopNo < PlayLoopNum ; LoopNo ++ )
		{
			// すべてのフレームの参照アニメーションキーをリセットする
			BoneInfo = PmxBoneInfo ;
			for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
			{
				if( BoneInfo->Anim != NULL && BoneInfo->IsIK == FALSE && BoneInfo->IsAddParent == FALSE )
				{
					BoneInfo->NowKey = BoneInfo->Node->FirstKey ;
				}
			}

			for( TimeNo = 0 ; ( DWORD )TimeNo <= VmdData.MaxTime * 2 ; TimeNo ++ )
			{
				IKSkip = TimeNo % 2 != 0 ;

				if( IKSkip && FPS60 == false )
				{
					// すべてのフレームの現在のフレームでのパラメータを算出する
					for( j = 0 ; j < PmxBoneNum ; j ++ )
					{
						if( PmxBoneInfo[ j ].IsIK == FALSE && BoneInfo->IsAddParent == FALSE )
							MV1LoadModelToPMX_SetupOneBoneMatrixFormAnimKey( &PmxBoneInfo[ j ], TimeNo, LoopNo ) ;
					}

					// 行列の計算
					MV1LoadModelToPMX_SetupMatrix( PmxBoneInfo, PmxBoneNum, FALSE, TRUE ) ;
				}
				else
				{
					// すべてのフレームの現在のフレームでのパラメータを算出する
					for( j = 0 ; j < PmxBoneNum ; j ++ )
						MV1LoadModelToPMX_SetupOneBoneMatrixFormAnimKey( &PmxBoneInfo[ j ], TimeNo, LoopNo ) ;

					// 行列の計算
					MV1LoadModelToPMX_SetupMatrix( PmxBoneInfo, PmxBoneNum, FALSE, FALSE ) ;

					// IKの計算を行うのはモーションループの最初だけ
					if( LoopNo == 0 )
					{
						// ＩＫの計算
						if( PmxIKInfoFirst )
						{
							MV1LoadModelToPMX_SetupIK( PmxBoneInfo, PmxIKInfoFirst ) ;
						}
					}
				}

#ifndef DX_NON_BULLET_PHYSICS
				// 物理演算を行う
				if( MLPhysicsInfo )
				{
					OneFrameProcess_PMXPhysicsInfo( MLPhysicsInfo, TimeNo, LoopNo, FPS60 ) ;
				}
#endif

				if( LoopNo == 0 && ( FPS60 || ( FPS60 == false && TimeNo % 2 == 0 ) ) )
				{
					// ＩＫに関わっているボーン又はＩＫの影響しないＩＫボーンの子ボーンのキーを保存
					BoneInfo = PmxBoneInfo ;
					for( j = 0 ; j < PmxBoneNum ; j ++, BoneInfo ++ )
					{
						if( BoneInfo->IsIK || BoneInfo->IsAddParent )
						{
							BoneInfo->KeyPosTime[ TimeNo ] = ( float )TimeNo / 2.0f ;
							BoneInfo->KeyRotTime[ TimeNo ] = ( float )TimeNo / 2.0f ;

							// 付与処理
							if( BoneInfo->IsAddParent )
							{
								PMX_READ_BONE_INFO *AddBoneInfo ;

								if( BoneInfo->Base->Flag_AddMov && BoneInfo->Base->AddParentBoneIndex >= 0 )
								{
									AddBoneInfo = &PmxBoneInfo[ BoneInfo->Base->AddParentBoneIndex ] ;
									while( AddBoneInfo->Base->Flag_AddMov )
									{
										AddBoneInfo = &PmxBoneInfo[ AddBoneInfo->Base->AddParentBoneIndex ] ;
									}

									if( BoneInfo->Base->AddRatio != 1.0f )
									{
										BoneInfo->KeyPos[ TimeNo ].x = BoneInfo->Translate.x + ( AddBoneInfo->Translate.x - BoneInfo->Translate.x ) * BoneInfo->Base->AddRatio ;
										BoneInfo->KeyPos[ TimeNo ].y = BoneInfo->Translate.y + ( AddBoneInfo->Translate.y - BoneInfo->Translate.y ) * BoneInfo->Base->AddRatio ;
										BoneInfo->KeyPos[ TimeNo ].z = BoneInfo->Translate.z + ( AddBoneInfo->Translate.z - BoneInfo->Translate.z ) * BoneInfo->Base->AddRatio ;
									}
									else
									{
										BoneInfo->KeyPos[ TimeNo ] = AddBoneInfo->Translate ;
									}
								}
								else
								{
									BoneInfo->KeyPos[ TimeNo ] = BoneInfo->Translate ;
								}

								if( BoneInfo->Base->Flag_AddRot && BoneInfo->Base->AddParentBoneIndex >= 0 )
								{
									AddBoneInfo = &PmxBoneInfo[ BoneInfo->Base->AddParentBoneIndex ] ;
									while( AddBoneInfo->Base->Flag_AddRot )
									{
										AddBoneInfo = &PmxBoneInfo[ AddBoneInfo->Base->AddParentBoneIndex ] ;
									}

									if( BoneInfo->Base->AddRatio != 1.0f )
									{
										_MV1SphereLinear( &BoneInfo->Rotate, &AddBoneInfo->Rotate, BoneInfo->Base->AddRatio, &BoneInfo->KeyRot[ TimeNo ] ) ;
									}
									else
									{
										BoneInfo->KeyRot[ TimeNo ] = AddBoneInfo->Rotate ;
									}
								}
								else
								{
									BoneInfo->KeyRot[ TimeNo ] = BoneInfo->Rotate ;
								}
							}
							else
							{
								BoneInfo->KeyPos[ TimeNo ] = BoneInfo->Translate ;
								BoneInfo->KeyRot[ TimeNo ] = BoneInfo->Rotate ;
							}
						}
						else
						if( BoneInfo->IsIKChild )
						{
							int DestIndex ;

							DestIndex = FPS60 ? TimeNo : TimeNo / 2 ;

							if( BoneInfo->Frame->Parent )
							{
								MATRIX InvParentBoneLWM ;

								// 親ボーンからの逆行列を自分の行列に掛けて、ボーンのローカル行列を取得する
								MV1LoadModelToVMD_InverseMatrix( ( ( PMX_READ_BONE_INFO * )BoneInfo->Frame->Parent->UserData )->LocalWorldMatrix, InvParentBoneLWM ) ;
								MV1LoadModelToVMD_CreateMultiplyMatrix( &BoneInfo->KeyMatrix2[ DestIndex ], &BoneInfo->LocalWorldMatrix, &InvParentBoneLWM ) ;
							}
							else
							{
								BoneInfo->KeyMatrix2[ DestIndex ] = BoneInfo->LocalWorldMatrix ;
							}
						}
					}
				}
			}
		}

#ifndef DX_NON_BULLET_PHYSICS
		if( MLPhysicsInfo )
		{
			MV1_ANIMKEYSET_R *KeyMatrixSet ;
			int PmxPhysicsNum ;

			// 物理適応のアニメーションデータをセットする
			PmxPhysicsNum = MLPhysicsInfo->PmxPhysicsNum ;
			for( i = 0 ; i < PmxPhysicsNum ; i ++ )
			{
				if( CheckDisablePhysicsAnim_PMXPhysicsInfo( MLPhysicsInfo, i ) ) continue ;

				BoneInfo = MLPhysicsInfo->PmxPhysicsInfoDim[ i ].Bone ;

				// すでにアニメのデータがボーンについていて且つ物理のアニメーションがついていないかどうかで処理を分岐
				if( BoneInfo->Anim != NULL && BoneInfo->SetupPhysicsAnim == 0 )
				{
					// 既にある場合は片方を MATRIX3X3型、もう片方を VECTOR型 の TRANSLATE にする
					if( BoneInfo->Anim->AnimKeySetFirst->Type == MV1_ANIMKEY_TYPE_VECTOR )
					{
						KeyPosSet = BoneInfo->Anim->AnimKeySetFirst ;
						KeyRotSet = BoneInfo->Anim->AnimKeySetLast ;
					}
					else
					{
						KeyRotSet = BoneInfo->Anim->AnimKeySetFirst ;
						KeyPosSet = BoneInfo->Anim->AnimKeySetLast ;
					}
					KeyMatrixSet = KeyRotSet ;

					// キー用メモリの再確保
					KeyMatrixSet->KeyMatrix3x3 = ( MV1_ANIM_KEY_MATRIX3X3 * )ADDMEMAREA( sizeof( MV1_ANIM_KEY_MATRIX3X3 ) * ( ( FPS60 ? VmdData.MaxTime * 2 : VmdData.MaxTime ) + 1 ), &RModel->Mem ) ;
					if( KeyMatrixSet->KeyMatrix3x3 == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					if( ( DWORD )BoneInfo->Anim->AnimKeySetFirst->Num != VmdData.MaxTime * 2 + 1 )
					{
						RModel->AnimKeyDataSize -= KeyMatrixSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( FLOAT4 ) ) ;

						if( FPS60 )
						{
							KeyPosSet->Num    = VmdData.MaxTime * 2 + 1 ;
							KeyMatrixSet->Num = VmdData.MaxTime * 2 + 1 ;
						}
						else
						{
							KeyPosSet->Num    = VmdData.MaxTime + 1 ;
							KeyMatrixSet->Num = VmdData.MaxTime + 1 ;
						}

						RModel->AnimKeyDataSize += KeyMatrixSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( MV1_ANIM_KEY_MATRIX3X3 ) ) ;

						KeyPosSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * KeyPosSet->Num, &RModel->Mem ) ;
						if( KeyPosSet->KeyVector == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 座標アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyPosSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyPosSet->Num, &RModel->Mem ) ;
						if( KeyPosSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyMatrixSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyMatrixSet->Num, &RModel->Mem ) ;
						if( KeyMatrixSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
					}
					else
					{
						RModel->AnimKeyDataSize -= KeyMatrixSet->Num * sizeof( FLOAT4 ) ;

						if( FPS60 == false )
						{
							KeyPosSet->Num    = VmdData.MaxTime + 1 ;
							KeyMatrixSet->Num = VmdData.MaxTime + 1 ;
						}

						RModel->AnimKeyDataSize += KeyMatrixSet->Num * sizeof( MV1_ANIM_KEY_MATRIX3X3 ) ;
					}

					KeyMatrixSet->Type = MV1_ANIMKEY_TYPE_MATRIX3X3 ;
					KeyMatrixSet->DataType = MV1_ANIMKEY_DATATYPE_MATRIX3X3 ;

					for( j = 0 ; j < KeyMatrixSet->Num ; j ++ )
					{
						if( FPS60 )
						{
							KeyMatrixSet->KeyTime[ j ] = ( float )j / 2.0f ;
							KeyPosSet->KeyTime[ j ]    = ( float )j / 2.0f ;
						}
						else
						{
							KeyMatrixSet->KeyTime[ j ] = ( float )j ;
							KeyPosSet->KeyTime[ j ]    = ( float )j ;
						}

						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 0 ][ 0 ] = BoneInfo->KeyMatrix[ j ].m[ 0 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 0 ][ 1 ] = BoneInfo->KeyMatrix[ j ].m[ 0 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 0 ][ 2 ] = BoneInfo->KeyMatrix[ j ].m[ 0 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 1 ][ 0 ] = BoneInfo->KeyMatrix[ j ].m[ 1 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 1 ][ 1 ] = BoneInfo->KeyMatrix[ j ].m[ 1 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 1 ][ 2 ] = BoneInfo->KeyMatrix[ j ].m[ 1 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 2 ][ 0 ] = BoneInfo->KeyMatrix[ j ].m[ 2 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 2 ][ 1 ] = BoneInfo->KeyMatrix[ j ].m[ 2 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 2 ][ 2 ] = BoneInfo->KeyMatrix[ j ].m[ 2 ][ 2 ] ;

						KeyPosSet->KeyVector[ j ].x = BoneInfo->KeyMatrix[ j ].m[ 3 ][ 0 ] ;
						KeyPosSet->KeyVector[ j ].y = BoneInfo->KeyMatrix[ j ].m[ 3 ][ 1 ] ;
						KeyPosSet->KeyVector[ j ].z = BoneInfo->KeyMatrix[ j ].m[ 3 ][ 2 ] ;
					}
				}
				else
				{
					// ない場合はここで追加

					BoneInfo->Anim = MV1RAddAnim( RModel, AnimSet ) ;
					if( BoneInfo->Anim == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					// 対象ノードのセット
					BoneInfo->Anim->TargetFrameName = MV1RAddString( RModel, BoneInfo->Frame->Name ) ;
					BoneInfo->Anim->TargetFrameIndex = BoneInfo->Frame->Index ;
					BoneInfo->Anim->MaxTime = ( float )VmdData.MaxTime ;

					// アニメーションキーの情報をセット
					KeyMatrixSet = MV1RAddAnimKeySet( RModel, BoneInfo->Anim ) ;
					if( KeyMatrixSet == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					KeyMatrixSet->Type = MV1_ANIMKEY_TYPE_MATRIX4X4C ;
					KeyMatrixSet->DataType = MV1_ANIMKEY_DATATYPE_MATRIX4X4C ;
					KeyMatrixSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
					KeyMatrixSet->TotalTime = ( float )VmdData.MaxTime ;
					KeyMatrixSet->Num = VmdData.MaxTime ;
					if( FPS60 ) KeyMatrixSet->Num *= 2  ;
					KeyMatrixSet->Num += 1 ;

					KeyMatrixSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyMatrixSet->Num, &RModel->Mem ) ;
					if( KeyMatrixSet->KeyTime == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyMatrixSet->KeyMatrix4x4C = ( MV1_ANIM_KEY_MATRIX4X4C * )ADDMEMAREA( sizeof( MV1_ANIM_KEY_MATRIX4X4C ) * KeyMatrixSet->Num, &RModel->Mem ) ;
					if( KeyMatrixSet->KeyMatrix4x4C == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					RModel->AnimKeyDataSize += KeyMatrixSet->Num * ( sizeof( float ) + sizeof( MV1_ANIM_KEY_MATRIX4X4C ) ) ;

					for( j = 0 ; j < KeyMatrixSet->Num ; j ++ )
					{
						KeyMatrixSet->KeyTime[ j ] = FPS60 ? ( float )j / 2.0f : ( float )j ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 0 ][ 0 ] = BoneInfo->KeyMatrix[ j ].m[ 0 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 0 ][ 1 ] = BoneInfo->KeyMatrix[ j ].m[ 0 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 0 ][ 2 ] = BoneInfo->KeyMatrix[ j ].m[ 0 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 1 ][ 0 ] = BoneInfo->KeyMatrix[ j ].m[ 1 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 1 ][ 1 ] = BoneInfo->KeyMatrix[ j ].m[ 1 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 1 ][ 2 ] = BoneInfo->KeyMatrix[ j ].m[ 1 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 2 ][ 0 ] = BoneInfo->KeyMatrix[ j ].m[ 2 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 2 ][ 1 ] = BoneInfo->KeyMatrix[ j ].m[ 2 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 2 ][ 2 ] = BoneInfo->KeyMatrix[ j ].m[ 2 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 3 ][ 0 ] = BoneInfo->KeyMatrix[ j ].m[ 3 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 3 ][ 1 ] = BoneInfo->KeyMatrix[ j ].m[ 3 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 3 ][ 2 ] = BoneInfo->KeyMatrix[ j ].m[ 3 ][ 2 ] ;
					}
				}

				// 物理のアニメを付けた証拠を残す
				BoneInfo->SetupPhysicsAnim = 1 ;
			}
		}
#endif

		// ＩＫボーンの影響を受けないＩＫボーンの影響を受けるボーンの直下にあるボーンの行列キーの準備をする
		{
			MV1_ANIMKEYSET_R *KeyMatrixSet ;

			BoneInfo = PmxBoneInfo ;
			for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
			{
				if( BoneInfo->IsIKChild == FALSE )
					continue ;

				if( BoneInfo->IsPhysics )
					continue ;

				// すでにアニメのデータがボーンについているかどうかで処理を分岐
				if( BoneInfo->Anim != NULL )
				{
					// 既にある場合は片方を MATRIX3X3型、もう片方を VECTOR型 の TRANSLATE にする
					if( BoneInfo->Anim->AnimKeySetFirst->Type == MV1_ANIMKEY_TYPE_VECTOR )
					{
						KeyPosSet = BoneInfo->Anim->AnimKeySetFirst ;
						KeyRotSet = BoneInfo->Anim->AnimKeySetLast ;
					}
					else
					{
						KeyRotSet = BoneInfo->Anim->AnimKeySetFirst ;
						KeyPosSet = BoneInfo->Anim->AnimKeySetLast ;
					}
					KeyMatrixSet = KeyRotSet ;

					// キー用メモリの再確保
					KeyMatrixSet->KeyMatrix3x3 = ( MV1_ANIM_KEY_MATRIX3X3 * )ADDMEMAREA( sizeof( MV1_ANIM_KEY_MATRIX3X3 ) * ( ( FPS60 ? VmdData.MaxTime * 2 : VmdData.MaxTime ) + 1 ), &RModel->Mem ) ;
					if( KeyMatrixSet->KeyMatrix3x3 == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					if( ( DWORD )BoneInfo->Anim->AnimKeySetFirst->Num != VmdData.MaxTime * 2 + 1 )
					{
						RModel->AnimKeyDataSize -= KeyMatrixSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( FLOAT4 ) ) ;

						if( FPS60 )
						{
							KeyPosSet->Num    = VmdData.MaxTime * 2 + 1 ;
							KeyMatrixSet->Num = VmdData.MaxTime * 2 + 1 ;
						}
						else
						{
							KeyPosSet->Num    = VmdData.MaxTime + 1 ;
							KeyMatrixSet->Num = VmdData.MaxTime + 1 ;
						}

						RModel->AnimKeyDataSize += KeyMatrixSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( MV1_ANIM_KEY_MATRIX3X3 ) ) ;

						KeyPosSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * KeyPosSet->Num, &RModel->Mem ) ;
						if( KeyPosSet->KeyVector == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 座標アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyPosSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyPosSet->Num, &RModel->Mem ) ;
						if( KeyPosSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyMatrixSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyMatrixSet->Num, &RModel->Mem ) ;
						if( KeyMatrixSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
					}
					else
					{
						RModel->AnimKeyDataSize -= KeyMatrixSet->Num * sizeof( FLOAT4 ) ;

						if( FPS60 == false )
						{
							KeyPosSet->Num    = VmdData.MaxTime + 1 ;
							KeyMatrixSet->Num = VmdData.MaxTime + 1 ;
						}

						RModel->AnimKeyDataSize += KeyMatrixSet->Num * sizeof( MV1_ANIM_KEY_MATRIX3X3 ) ;
					}

					KeyMatrixSet->Type = MV1_ANIMKEY_TYPE_MATRIX3X3 ;
					KeyMatrixSet->DataType = MV1_ANIMKEY_DATATYPE_MATRIX3X3 ;

					for( j = 0 ; j < KeyMatrixSet->Num ; j ++ )
					{
						if( FPS60 )
						{
							KeyMatrixSet->KeyTime[ j ] = ( float )j / 2.0f ;
							KeyPosSet->KeyTime[ j ]    = ( float )j / 2.0f ;
						}
						else
						{
							KeyMatrixSet->KeyTime[ j ] = ( float )j ;
							KeyPosSet->KeyTime[ j ]    = ( float )j ;
						}

						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 0 ][ 0 ] = BoneInfo->KeyMatrix2[ j ].m[ 0 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 0 ][ 1 ] = BoneInfo->KeyMatrix2[ j ].m[ 0 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 0 ][ 2 ] = BoneInfo->KeyMatrix2[ j ].m[ 0 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 1 ][ 0 ] = BoneInfo->KeyMatrix2[ j ].m[ 1 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 1 ][ 1 ] = BoneInfo->KeyMatrix2[ j ].m[ 1 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 1 ][ 2 ] = BoneInfo->KeyMatrix2[ j ].m[ 1 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 2 ][ 0 ] = BoneInfo->KeyMatrix2[ j ].m[ 2 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 2 ][ 1 ] = BoneInfo->KeyMatrix2[ j ].m[ 2 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix3x3[ j ].Matrix[ 2 ][ 2 ] = BoneInfo->KeyMatrix2[ j ].m[ 2 ][ 2 ] ;

						KeyPosSet->KeyVector[ j ].x = BoneInfo->KeyMatrix2[ j ].m[ 3 ][ 0 ] ;
						KeyPosSet->KeyVector[ j ].y = BoneInfo->KeyMatrix2[ j ].m[ 3 ][ 1 ] ;
						KeyPosSet->KeyVector[ j ].z = BoneInfo->KeyMatrix2[ j ].m[ 3 ][ 2 ] ;
					}
				}
				else
				{
					// ない場合はここで追加

					BoneInfo->Anim = MV1RAddAnim( RModel, AnimSet ) ;
					if( BoneInfo->Anim == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					// 対象ノードのセット
					BoneInfo->Anim->TargetFrameName = MV1RAddString( RModel, BoneInfo->Frame->Name ) ;
					BoneInfo->Anim->TargetFrameIndex = BoneInfo->Frame->Index ;
					BoneInfo->Anim->MaxTime = ( float )VmdData.MaxTime ;

					// アニメーションキーの情報をセット
					KeyMatrixSet = MV1RAddAnimKeySet( RModel, BoneInfo->Anim ) ;
					if( KeyMatrixSet == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					KeyMatrixSet->Type = MV1_ANIMKEY_TYPE_MATRIX4X4C ;
					KeyMatrixSet->DataType = MV1_ANIMKEY_DATATYPE_MATRIX4X4C ;
					KeyMatrixSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
					KeyMatrixSet->TotalTime = ( float )VmdData.MaxTime ;
					KeyMatrixSet->Num = VmdData.MaxTime ;
					if( FPS60 ) KeyMatrixSet->Num *= 2  ;
					KeyMatrixSet->Num += 1 ;

					KeyMatrixSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyMatrixSet->Num, &RModel->Mem ) ;
					if( KeyMatrixSet->KeyTime == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyMatrixSet->KeyMatrix4x4C = ( MV1_ANIM_KEY_MATRIX4X4C * )ADDMEMAREA( sizeof( MV1_ANIM_KEY_MATRIX4X4C ) * KeyMatrixSet->Num, &RModel->Mem ) ;
					if( KeyMatrixSet->KeyMatrix4x4C == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					RModel->AnimKeyDataSize += KeyMatrixSet->Num * ( sizeof( float ) + sizeof( MV1_ANIM_KEY_MATRIX4X4C ) ) ;

					for( j = 0 ; j < KeyMatrixSet->Num ; j ++ )
					{
						KeyMatrixSet->KeyTime[ j ] = FPS60 ? ( float )j / 2.0f : ( float )j ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 0 ][ 0 ] = BoneInfo->KeyMatrix2[ j ].m[ 0 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 0 ][ 1 ] = BoneInfo->KeyMatrix2[ j ].m[ 0 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 0 ][ 2 ] = BoneInfo->KeyMatrix2[ j ].m[ 0 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 1 ][ 0 ] = BoneInfo->KeyMatrix2[ j ].m[ 1 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 1 ][ 1 ] = BoneInfo->KeyMatrix2[ j ].m[ 1 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 1 ][ 2 ] = BoneInfo->KeyMatrix2[ j ].m[ 1 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 2 ][ 0 ] = BoneInfo->KeyMatrix2[ j ].m[ 2 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 2 ][ 1 ] = BoneInfo->KeyMatrix2[ j ].m[ 2 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 2 ][ 2 ] = BoneInfo->KeyMatrix2[ j ].m[ 2 ][ 2 ] ;

						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 3 ][ 0 ] = BoneInfo->KeyMatrix2[ j ].m[ 3 ][ 0 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 3 ][ 1 ] = BoneInfo->KeyMatrix2[ j ].m[ 3 ][ 1 ] ;
						KeyMatrixSet->KeyMatrix4x4C[ j ].Matrix[ 3 ][ 2 ] = BoneInfo->KeyMatrix2[ j ].m[ 3 ][ 2 ] ;
					}
				}
			}
		}

		// ３０ＦＰＳ保存指定の場合はここでアニメーションキーを削る
		if( FPS60 == false )
		{
			BoneInfo = PmxBoneInfo ;
			for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
			{
#ifndef DX_NON_BULLET_PHYSICS
				if( MLPhysicsInfo && BoneInfo->IsPhysics == TRUE && CheckDisablePhysicsAnim_PMXPhysicsInfo( MLPhysicsInfo, BoneInfo->PhysicsIndex ) == false ) continue ;
#endif
				if( BoneInfo->IsIKChild ) continue ;

				if( BoneInfo->Anim == NULL ) continue ;

				if( BoneInfo->Anim->AnimKeySetFirst->Type == MV1_ANIMKEY_TYPE_VECTOR )
				{
					KeyPosSet = BoneInfo->Anim->AnimKeySetFirst ;
					KeyRotSet = BoneInfo->Anim->AnimKeySetLast ;
				}
				else
				{
					KeyRotSet = BoneInfo->Anim->AnimKeySetFirst ;
					KeyPosSet = BoneInfo->Anim->AnimKeySetLast ;
				}

				if( KeyPosSet->Num != KeyRotSet->Num || KeyPosSet->Num == 1 || KeyPosSet->Num % 2 == 0 ) continue ;

				RModel->AnimKeyDataSize -= KeyPosSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( FLOAT4 ) ) ;

				KeyPosSet->Num = KeyPosSet->Num / 2 + 1 ;
				KeyRotSet->Num = KeyRotSet->Num / 2 + 1 ;

				RModel->AnimKeyDataSize += KeyPosSet->Num * ( sizeof( float ) * 2 + sizeof( VECTOR ) + sizeof( FLOAT4 ) ) ;

				for( j = 0 ; j < KeyPosSet->Num ; j ++ )
				{
					KeyRotSet->KeyTime[ j ] = ( float )j ;
					KeyPosSet->KeyTime[ j ] = ( float )j ;

					KeyPosSet->KeyVector[ j ] = KeyPosSet->KeyVector[ j * 2 ] ;
					KeyRotSet->KeyFloat4[ j ] = KeyRotSet->KeyFloat4[ j * 2 ] ;
				}
			}
		}

		// 表情アニメーションを追加する
		if( VmdData.FaceKeySetNum != 0 )
		{
			// アニメーションの追加
			Anim = MV1RAddAnim( RModel, AnimSet ) ;
			if( Anim == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションの追加に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// 対象ノードのセット
			Anim->TargetFrameName = MV1RAddString( RModel, "Mesh" ) ;
			Anim->TargetFrameIndex = 0 ;

			// 最大時間をセット
			Anim->MaxTime = 0.0f ;

			// 表情キーセットの数だけ繰り返す
			VmdFaceKeySet = VmdData.FaceKeySet ;
			Frame = RModel->FrameFirst ; 
			for( i = 0 ; ( DWORD )i < VmdData.FaceKeySetNum ; i ++, VmdFaceKeySet ++ )
			{
				// 対象となるシェイプデータの検索
				Shape = Frame->ShapeFirst ;
				for( j = 0 ; j < Frame->ShapeNum ; j ++, Shape = Shape->Next )
				{
					if( _STRCMP( Shape->Name, VmdFaceKeySet->Name ) == 0 ) break ;
				}
				if( j == Frame->ShapeNum ) continue ;

				// 再生時間の更新
				if( Anim->MaxTime < ( float )VmdFaceKeySet->MaxFrame ) Anim->MaxTime = ( float )VmdFaceKeySet->MaxFrame ;

				// アニメーションキーの情報をセット
				KeyFactorSet = MV1RAddAnimKeySet( RModel, Anim ) ;
				if( KeyFactorSet == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーション表情キーセットの追加に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}

				KeyFactorSet->Type = MV1_ANIMKEY_TYPE_LINEAR ;
				KeyFactorSet->DataType = MV1_ANIMKEY_DATATYPE_SHAPE ;
				KeyFactorSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
				KeyFactorSet->TotalTime = ( float )VmdFaceKeySet->MaxFrame ;
				KeyFactorSet->TargetShapeIndex = j ;
				KeyFactorSet->Num = VmdFaceKeySet->KeyNum ;

				KeyFactorSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyFactorSet->Num, &RModel->Mem ) ;
				if( KeyFactorSet->KeyTime == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}
				KeyFactorSet->KeyLinear = ( float * )ADDMEMAREA( sizeof( float ) * KeyFactorSet->Num, &RModel->Mem ) ;
				if( KeyFactorSet->KeyLinear == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMX Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}

				RModel->AnimKeyDataSize += KeyFactorSet->Num * sizeof( float ) * 2 ;

				// キー情報をセット
				KeyFactor = KeyFactorSet->KeyLinear ;
				KeyFactorTime = KeyFactorSet->KeyTime ;
				VmdFaceKey = VmdFaceKeySet->FirstKey ;
				for( j = 0 ; ( DWORD )j < VmdFaceKeySet->KeyNum ; j ++, VmdFaceKey = VmdFaceKey->Next, KeyFactor ++, KeyFactorTime ++ )
				{
					*KeyFactorTime = ( float )VmdFaceKey->Frame ;
					*KeyFactor = VmdFaceKey->Factor ;
				}
			}
		}
	}

	// 動的に確保したメモリの解放
	TerminateVMDBaseData( &VmdData ) ;

#ifndef DX_NON_BULLET_PHYSICS
	if( MLPhysicsInfo )
	{
		ReleasePhysicsObject_PMXPhysicsInfo( MLPhysicsInfo ) ;

		BoneInfo = PmxBoneInfo ;
		for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
		{
			if( BoneInfo->KeyMatrix )
			{
				DXFREE( BoneInfo->KeyMatrix ) ;
				BoneInfo->KeyMatrix = NULL ;
			}

			if( BoneInfo->KeyMatrix2 )
			{
				DXFREE( BoneInfo->KeyMatrix2 ) ;
				BoneInfo->KeyMatrix2 = NULL ;
			}
		}
	}
#endif

	BoneInfo = PmxBoneInfo ;
	for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
	{
		if( BoneInfo->KeyMatrix2 )
		{
			DXFREE( BoneInfo->KeyMatrix2 ) ;
			BoneInfo->KeyMatrix2 = NULL ;
		}
	}

	// 正常終了
	return 0 ;

ENDLABEL :

	// 動的に確保したメモリの解放
	TerminateVMDBaseData( &VmdData ) ;

#ifndef DX_NON_BULLET_PHYSICS
	if( MLPhysicsInfo )
	{
		ReleasePhysicsObject_PMXPhysicsInfo( MLPhysicsInfo ) ;

		BoneInfo = PmxBoneInfo ;
		for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
		{
			if( BoneInfo->KeyMatrix )
			{
				DXFREE( BoneInfo->KeyMatrix ) ;
				BoneInfo->KeyMatrix = NULL ;
			}
		}
	}
#endif

	BoneInfo = PmxBoneInfo ;
	for( i = 0 ; i < PmxBoneNum ; i ++, BoneInfo ++ )
	{
		if( BoneInfo->KeyMatrix2 )
		{
			DXFREE( BoneInfo->KeyMatrix2 ) ;
			BoneInfo->KeyMatrix2 = NULL ;
		}
	}

	// エラー
	return -1 ;
}


}

#endif


