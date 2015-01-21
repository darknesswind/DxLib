// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ＰＭＤモデルデータ読み込みプログラム
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

/*

本ソースを作成する際に参考にさせていただいたウェブサイトとプログラムソース


PMD形式について参考にさせていただいたウェブサイト

<< 通りすがりの記憶 >> 通りすがり様
http://blog.goo.ne.jp/torisu_tetosuki


VMD形式とBulletによる物理演算について参考にさせていただいたプログラムソース

<< PY >> PY様
http://ppyy.hp.infoseek.co.jp/
<< artk_mmd_src.zip >> 


IK処理のプログラムソース
樋口M様
<< ik.zip >>

*/

#define __DX_MAKE

#include "DxModelLoader3.h"

#ifndef DX_NON_MODEL

// インクルード ---------------------------------
#include "DxFile.h"
#include "DxLog.h"
#include "DxModelLoaderVMD.h"
#include "DxMemory.h"
#include "Windows/DxGuid.h"

namespace DxLib
{

// マクロ定義 -----------------------------------

// データ宣言 -----------------------------------


// 関数宣言 -------------------------------------

// ＶＭＤファイルを読み込む( -1:エラー )
static int _MV1LoadModelToVMD_PMD(
	int								DataIndex,
	MV1_MODEL_R *					RModel,
	void *							DataBuffer,
	int								DataSize,
	const char *					Name,
	PMD_READ_BONE_INFO *			PmdBoneInfo,
	int								PmdBoneNum,
	PMD_READ_IK_INFO *				PmdIKInfoFirst,
#ifndef DX_NON_BULLET_PHYSICS
	DX_MODELLOADER3_PMD_PHYSICS_INFO *	MLPhysicsInfo,
#endif
	bool							FPS60
) ;

// 行列を計算する( 骨一つだけ )
static void MV1LoadModelToPMD_SetupMatrix_One( PMD_READ_BONE_INFO *BoneInfo ) ;

// 行列を計算する
static void MV1LoadModelToPMD_SetupMatrix( PMD_READ_BONE_INFO *BoneInfo, int BoneNum, int UseInitParam, int IKSkip ) ;

// ＩＫボーンのトランスレーションを回転値に変換する
//static void MV1LoadModelToPMD_ConvertTransToRotate( PMD_READ_BONE_INFO *BoneInfo, PMD_READ_IK_INFO *IKInfoFirst ) ;

// ＩＫを計算する
static void MV1LoadModelToPMD_SetupIK( PMD_READ_BONE_INFO *BoneInfo, PMD_READ_IK_INFO *IKInfoFirst ) ;

// 指定のボーンにアニメーションの指定キーのパラメータを反映させる
static void MV1LoadModelToPMD_SetupOneBoneMatrixFormAnimKey( PMD_READ_BONE_INFO *BoneInfo, int Time, int LoopNo ) ;

// プログラム -----------------------------------

/*
// VMDのカメラパラメータから座標と回転値を算出する
void CalculateCameraParam( VECTOR *Position, VECTOR *Rotation, float Length, VECTOR *OutPosition, VECTOR *OutRotation )
{
	VECTOR OutPosition ;
	VECTOR DirVec ;
	MATRIX VRotate, HRotate, TwistRotate ;
	MATRIX PosTransMix ;

	OutPosition = VGet( 0.0f, 0.0f, 1.0f ) ;
	DirVec = VGet( 0.0f, 0.0f, -1.0f ) ;
	VRotate = MGetRotX( Rotation->x * DX_PI_F / 180.0f ) ;
	HRotate = MGetRotY( Rotation->y * DX_PI_F / 180.0f ) ;
	PosTransMix = MMult( VRotate, HRotate ) ;

	DirVec = VTransform( DirVec, PosTransMix ) ;
	TwistRotate = MGetRotAxis( DirVec, Rotation->z * DX_PI_F / 180.0f ) ;

	*OutPosition = VTransform( *Position, PosTransMix ) ;
	OutRotation->x = Rotation->x ;
	OutRotation->y = Rotation->y ;
}
*/
// ＰＭＤファイルを読み込む( -1:エラー  0以上:モデルハンドル )
extern int MV1LoadModelToPMD( const MV1_MODEL_LOAD_PARAM *LoadParam, int ASyncThread )
{
	int NewHandle = -1 ;
	int ErrorFlag = 1 ;
	int i, j, weightcount, facecount ;
	PMD_HEADER *PmdHeader ;
	PMD_VERTEX *PmdVertex ;
	DWORD PmdVertexNum ;
	WORD *PmdFaceVertex ;
	DWORD PmdFaceVertexNum ;
	DWORD PmdFaceNum ;
	BYTE *PmdSkin ;
	PMD_MATERIAL *PmdMaterial ;
	PMD_READ_IK_INFO *IKInfoDim = NULL, *IKInfoFirst = NULL, *IKInfo ;
	PMD_READ_BONE_INFO *BoneInfoDim = NULL, *BoneInfo ;
	DWORD PmdMaterialFaceVertexNum ;
	DWORD PmdMaterialNum ;
	PMD_BONE *PmdBone ;
	WORD PmdBoneNum ;
	PMD_IK *PmdIK ;
	WORD PmdIKNum ;
	PMD_SKIN_VERT *PmdBaseSkinVert, *PmdSkinVert ;
	WORD PmdSkinNum ;
	char *PmdToonFileName = NULL ;
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
	DWORD SkinVertNum, SkinBaseVertNum, SkinTargetVert, SkinFaceNum, NextFaceNum, SkinMaterialNum ;
	char String[ 1024 ] ;
	BYTE *Src ;
	int ValidPhysics = FALSE ;
	int BoneDispNum ;
	BYTE ToonTexIndex ;
#ifndef DX_NON_BULLET_PHYSICS
	DX_MODELLOADER3_PMD_PHYSICS_INFO MLPhysicsInfo ;

	_MEMSET( &MLPhysicsInfo, 0, sizeof( MLPhysicsInfo ) ) ;
#endif

	// 読み込みようデータの初期化
	MV1InitReadModel( &RModel ) ;
	RModel.MaterialNumberOrderDraw = TRUE ;
	RModel.MeshFaceRightHand = FALSE ;
	RModel.TranslateIsBackCulling = TRUE ;

	// Pmdモデルデータの情報をセット
	Src = ( BYTE * )LoadParam->DataBuffer ;
	PmdHeader = ( PMD_HEADER * )LoadParam->DataBuffer ;

	// PMDファイルかどうかを確認
	if( Src[ 0 ] != 'P' || Src[ 1 ] != 'm' || Src[ 2 ] != 'd' )
		return -1 ;

	// バージョン１以外は読み込めない
	if( *( ( DWORD * )&Src[ 3 ] ) != 0x3f800000 )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : バージョン１．０以外は読み込めません\n" ) ) ) ;
		return -1 ;
	}

	// モデル名とファイル名をセット
	RModel.FilePath = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->FilePath ) + 1 ) * sizeof( TCHAR ) ) ;
	RModel.Name     = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->Name     ) + 1 ) * sizeof( TCHAR ) ) ;
	lstrcpy( RModel.FilePath, LoadParam->FilePath ) ;
	lstrcpy( RModel.Name,     LoadParam->Name ) ;

	// 法泉の自動生成は使用しない
	RModel.AutoCreateNormal = FALSE ;

	// 各データの先頭アドレスをセット
	{
		Src += 283 ;

		PmdVertexNum = *( ( DWORD * )Src ) ;
		Src += 4 ;

		PmdVertex = ( PMD_VERTEX * )Src ;
		Src += 38 * PmdVertexNum ;

		PmdFaceVertexNum = *( ( DWORD * )Src ) ;
		PmdFaceNum = PmdFaceVertexNum / 3 ;
		Src += 4 ;

		PmdFaceVertex = ( WORD * )Src ;
		Src += 2 * PmdFaceVertexNum ;

		PmdMaterialNum = *( ( DWORD * )Src ) ;
		Src += 4 ;

		PmdMaterial = ( PMD_MATERIAL * )Src ;
		Src += 70 * PmdMaterialNum ;

		PmdBoneNum = *( ( WORD * )Src ) ;
		Src += 2 ;

		PmdBone = ( PMD_BONE * )Src ;
		Src += 39 * PmdBoneNum ;

		PmdIKNum = *( ( WORD * )Src ) ;
		Src += 2 ;

		PmdIK = ( PMD_IK * )Src ;
	}

	// メッシュを収めるフレームの追加
	Frame = MV1RAddFrame( &RModel, "Mesh", NULL ) ;
	if( Frame == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : フレームオブジェクトの追加に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// 読み込み処理用のボーンデータを格納するメモリ領域の確保
	BoneInfoDim = ( PMD_READ_BONE_INFO * )DXALLOC( sizeof( PMD_READ_BONE_INFO ) * PmdBoneNum ) ;
	if( BoneInfoDim == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 読み込み処理用ボーン情報を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// ボーンデータの追加
	_MEMSET( FrameDimEnable, 0, sizeof( FrameDimEnable ) ) ;
	_MEMSET( FrameDim, 0, sizeof( FrameDim ) ) ;
	do
	{
		_MEMSET( FrameSkipDim, 0, sizeof( FrameSkipDim ) ) ;
		Src = ( BYTE * )PmdBone ;
		BoneInfo = BoneInfoDim ;
		FrameSkipNum = 0 ;
		for( i = 0 ; i < PmdBoneNum ; i ++, Src += 39, BoneInfo ++ )
		{
			WORD ParentBoneIndex = *( ( WORD * )&Src[ 20 ] ) ;

			if( ParentBoneIndex != 0xffff && FrameDimEnable[ ParentBoneIndex ] == 0 )
			{
				FrameSkipDim[ i ] = true ;
				FrameSkipNum ++ ;
				continue ;
			}
			if( FrameDimEnable[ i ] != 0 )
				continue ;
			FrameDimEnable[ i ] = 1 ;

			_MEMSET( String, 0, sizeof( String ) ) ;
			_MEMCPY( String, Src, 20 ) ;
			FrameDim[ i ] = MV1RAddFrame( &RModel, String, ParentBoneIndex == 0xffff ? NULL : FrameDim[ ParentBoneIndex ] ) ;
			if( FrameDim[ i ] == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : フレームオブジェクトの追加に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}
//			BoneInfo->IKLimitAngle = _STRCMP( "左ひざ", FrameDim[ i ]->Name ) == 0 || _STRCMP( "右ひざ", FrameDim[ i ]->Name ) == 0 ? 1 : 0 ;
			BoneInfo->IKLimitAngle = _STRSTR( FrameDim[ i ]->Name, "ひざ" ) != NULL ;
			*( ( DWORD * )&BoneInfo->OrgTranslate.x ) = *( ( DWORD * )&Src[ 27 ] ) ;
			*( ( DWORD * )&BoneInfo->OrgTranslate.y ) = *( ( DWORD * )&Src[ 31 ] ) ;
			*( ( DWORD * )&BoneInfo->OrgTranslate.z ) = *( ( DWORD * )&Src[ 35 ] ) ;
			*( ( DWORD * )&FrameDim[ i ]->TempVector.x ) = *( ( DWORD * )&Src[ 27 ] ) ;
			*( ( DWORD * )&FrameDim[ i ]->TempVector.y ) = *( ( DWORD * )&Src[ 31 ] ) ;
			*( ( DWORD * )&FrameDim[ i ]->TempVector.z ) = *( ( DWORD * )&Src[ 35 ] ) ;
	//		FrameDim[ i ]->TempVector.z = -FrameDim[ i ]->TempVector.z ;
			if( ParentBoneIndex != 0xffff )
			{
				FrameDim[ i ]->Translate = VSub( FrameDim[ i ]->TempVector, FrameDim[ ParentBoneIndex ]->TempVector ) ;
			}
			else
			{
				FrameDim[ i ]->Translate = FrameDim[ i ]->TempVector ;
			}
			FrameDim[ i ]->UserData = BoneInfo ;

			BoneInfo->Base = ( PMD_BONE * )Src ;
			BoneInfo->Frame = FrameDim[ i ] ;
			BoneInfo->Type = ( int )Src[ 24 ] ;
			BoneInfo->IsPhysics = FALSE ;
			BoneInfo->IsIK = FALSE ;
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
	MV1LoadModelToPMD_SetupMatrix( BoneInfoDim, PmdBoneNum, TRUE, FALSE ) ;

	// メッシュを追加
	{
		Mesh = MV1RAddMesh( &RModel, Frame ) ;
		if( Mesh == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : メッシュオブジェクトの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// 面情報を格納するメモリ領域の確保
		if( MV1RSetupMeshFaceBuffer( &RModel, Mesh, PmdFaceNum, 3 ) < 0 )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 面情報を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		Mesh->PositionNum = PmdVertexNum ;
		Mesh->Positions = ( VECTOR * )ADDMEMAREA( ( sizeof( VECTOR ) + sizeof( float ) ) * Mesh->PositionNum, &RModel.Mem ) ;
		if( Mesh->Positions == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 頂点座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		Mesh->PositionToonOutLineScale = ( float * )( Mesh->Positions + Mesh->PositionNum ) ;

		Mesh->NormalNum = PmdVertexNum ;
		Mesh->Normals = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * Mesh->NormalNum, &RModel.Mem ) ;
		if( Mesh->Normals == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 頂点法線を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		Mesh->UVNum[ 0 ] = PmdVertexNum ;
		Mesh->UVs[ 0 ] = ( FLOAT4 * )ADDMEMAREA( sizeof( FLOAT4 ) * Mesh->UVNum[ 0 ], &RModel.Mem ) ;
		if( Mesh->UVs[ 0 ] == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 頂点テクスチャ座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// ボーンの情報を追加
		Mesh->SkinWeightsNum = 0 ;
		for( i = 0 ; i < PmdBoneNum ; i ++ )
		{
			// このボーンを使用している頂点の数を数える
			Src = ( BYTE * )PmdVertex ;
			weightcount = 0 ;
			for( j = 0 ; ( DWORD )j < PmdVertexNum ; j ++, Src += 38 )
			{
				if( *( ( WORD * )&Src[ 32 ] ) == i || *( ( WORD * )&Src[ 34 ] ) == i )
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
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : スキニングメッシュウエイト値を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				return -1 ;
			}

			// ウエイトの情報を格納する
			SkinW = SkinWeight->Data ;
			Src = ( BYTE * )PmdVertex ;
			for( j = 0 ; ( DWORD )j < PmdVertexNum ; j ++, Src += 38 )
			{
				if( *( ( WORD * )&Src[ 32 ] ) == i && *( ( WORD * )&Src[ 34 ] ) == i )
				{
					SkinW->TargetVertex = j ;
					SkinW->Weight = 1.0f ;
					SkinW ++ ;
				}
				else
				if( *( ( WORD * )&Src[ 32 ] ) == i )
				{
					SkinW->TargetVertex = j ;
					SkinW->Weight = Src[ 36 ] / 100.0f ;
					SkinW ++ ;
				}
				else
				if( *( ( WORD * )&Src[ 34 ] ) == i )
				{
					SkinW->TargetVertex = j ;
					SkinW->Weight = ( 100 - Src[ 36 ] ) / 100.0f ;
					SkinW ++ ;
				}
			}
		}

		// 頂点データをセット
		Src = ( BYTE * )PmdVertex ;
		for( i = 0 ; ( DWORD )i < PmdVertexNum ; i ++, Src += 38 )
		{
			*( ( DWORD * )&Mesh->Positions[ i ].x ) = *( ( DWORD * )&Src[ 0 ] ) ;
			*( ( DWORD * )&Mesh->Positions[ i ].y ) = *( ( DWORD * )&Src[ 4 ] ) ;
			*( ( DWORD * )&Mesh->Positions[ i ].z ) = *( ( DWORD * )&Src[ 8 ] ) ;
			*( ( DWORD * )&Mesh->Normals[ i ].x ) = *( ( DWORD * )&Src[ 12 ] ) ;
			*( ( DWORD * )&Mesh->Normals[ i ].y ) = *( ( DWORD * )&Src[ 16 ] ) ;
			*( ( DWORD * )&Mesh->Normals[ i ].z ) = *( ( DWORD * )&Src[ 20 ] ) ;
			*( ( DWORD * )&Mesh->UVs[ 0 ][ i ].x ) = *( ( DWORD * )&Src[ 24 ] ) ;
			*( ( DWORD * )&Mesh->UVs[ 0 ][ i ].y ) = *( ( DWORD * )&Src[ 28 ] ) ;
			Mesh->PositionToonOutLineScale[ i ] = Src[ 37 ] != 0 ? 0.0f : 1.0f ;
		}

		// 面データをセット
		Src = ( BYTE * )PmdFaceVertex ;
		MeshFace = Mesh->Faces ;
		j = 0 ;
		for( i = 0 ; ( DWORD )i < PmdFaceNum ; i ++, Src += 2 * 3 )
		{
			MeshFace->IndexNum = 3 ;
			MeshFace->VertexIndex[ 0 ] = *( ( WORD * )&Src[ 0 ] ) ;
			MeshFace->VertexIndex[ 1 ] = *( ( WORD * )&Src[ 2 ] ) ;
			MeshFace->VertexIndex[ 2 ] = *( ( WORD * )&Src[ 4 ] ) ;
			MeshFace->NormalIndex[ 0 ] = *( ( WORD * )&Src[ 0 ] ) ;
			MeshFace->NormalIndex[ 1 ] = *( ( WORD * )&Src[ 2 ] ) ;
			MeshFace->NormalIndex[ 2 ] = *( ( WORD * )&Src[ 4 ] ) ;
			MeshFace->UVIndex[ 0 ][ 0 ] = *( ( WORD * )&Src[ 0 ] ) ;
			MeshFace->UVIndex[ 0 ][ 1 ] = *( ( WORD * )&Src[ 2 ] ) ;
			MeshFace->UVIndex[ 0 ][ 2 ] = *( ( WORD * )&Src[ 4 ] ) ;

			MeshFace ++ ;
			j ++ ;
		}
		PmdFaceNum = j ;
	}

	// ＩＫの情報を格納するメモリ領域の確保
	if( PmdIKNum )
	{
		IKInfoDim = ( PMD_READ_IK_INFO * )DXALLOC( sizeof( PMD_READ_IK_INFO ) * PmdIKNum ) ;
		if( IKInfoDim == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 読み込み処理用ＩＫ情報を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
	}

	// ＩＫデータの追加
	Src = ( BYTE * )PmdIK ;
	IKInfo = IKInfoDim ;
	IKInfoFirst = NULL ;
	for( i = 0 ; i < PmdIKNum ; i ++, IKInfo ++ )
	{
		PMD_READ_BONE_INFO *BoneBone ;

		// データをセット
		IKInfo->Base = ( PMD_IK * )Src ;
		IKInfo->Bone = &BoneInfoDim[ *( ( WORD * )&Src[ 0 ] ) ] ;
		IKInfo->TargetBone = &BoneInfoDim[ *( ( WORD * )&Src[ 2 ] ) ] ;
		IKInfo->TargetBone->IsIK = TRUE ;
		IKInfo->ChainBoneNum = Src[ 4 ] ;
		IKInfo->Iterations = *( ( WORD * )&Src[ 5 ] ) ;
		IKInfo->ControlWeight = *( ( float * )&Src[ 7 ] ) ;
		IKInfo->LimitAngleIK = IKInfo->Bone->IKLimitAngle || IKInfo->TargetBone->IKLimitAngle ? 1 : 0 ;
		IKInfo->IKTotalLength = 0.0f ;
		Src += 11 ;
		IKInfo->ChainBone = ( WORD * )Src ;
		Src += sizeof( WORD ) * IKInfo->ChainBoneNum ;

		for( j = 0 ; j < IKInfo->ChainBoneNum ; j ++ )
		{
			BoneBone = &BoneInfoDim[ IKInfo->ChainBone[ j ] ] ;
			BoneInfoDim[ IKInfo->ChainBone[ j ] ].IsIK = TRUE ;
			if( BoneBone->IKLimitAngle ) IKInfo->LimitAngleIK = 1 ;
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
			PMD_READ_IK_INFO *IKInfoTemp ;

			for( IKInfoTemp = IKInfoFirst ; IKInfoTemp->Next != NULL && *IKInfoTemp->ChainBone < *IKInfo->ChainBone ; IKInfoTemp = IKInfoTemp->Next ){}
			if( IKInfoTemp->Next == NULL && *IKInfoTemp->ChainBone < *IKInfo->ChainBone )
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
	}

	// ＩＫの影響を受けるボーンの子でＩＫの影響を受けないボーンに印をつける
	BoneInfo = BoneInfoDim ;
	for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
	{
		PMD_READ_BONE_INFO *ParentBone ;

		if( BoneInfo->IsIK )
			continue ;

		if( BoneInfo->Frame->Parent == NULL )
			continue ;

		ParentBone = ( PMD_READ_BONE_INFO * )BoneInfo->Frame->Parent->UserData ;
		if( ParentBone->IsIK )
		{
			BoneInfo->IsIKChild = TRUE ;
		}
	}

	// 表情データのアドレスを保存
	PmdSkinNum = *( ( WORD * )Src ) ;
	Src += 2 ;
	PmdSkin = Src ;
	if( PmdSkinNum > 1 )
	{
		for( i = 0 ; i < PmdSkinNum ; i ++ )
		{
			Src += 20 ;

			j = *( ( DWORD * )Src ) ;
			Src += 5 + j * 16 ;
		}
	}

	// 表情枠用表示リストの読み飛ばし
	i = ( int )*Src ;
	Src += sizeof( BYTE ) + sizeof( WORD ) * i ;

	// ボーン枠用枠名リストの読み飛ばし
	BoneDispNum = ( int )*Src ;
	Src += sizeof( BYTE ) + sizeof( char ) * 50 * BoneDispNum ;

	// これ以上データが存在しなかったら物理演算用のデータはないということ
	if( Src - ( BYTE * )LoadParam->DataBuffer >= LoadParam->DataSize ) goto PHYSICSDATAREADEND ;

	// ボーン枠用表示リストの読み飛ばし
	i = ( int )*( ( DWORD * )Src ) ;
	Src += sizeof( DWORD ) + 3 * i ;

	// これ以上データが存在しなかったら物理演算用のデータはないということ
	if( Src - ( BYTE * )LoadParam->DataBuffer >= LoadParam->DataSize ) goto PHYSICSDATAREADEND ;

	// 英語名情報の読み飛ばし
	i = ( int )*Src ;
	Src ++ ;
	if( i )
	{
		// モデル名とコメントの読み飛ばし
		Src += 276 ;

		// ボーン名の読み飛ばし
		Src += PmdBoneNum * 20 ;

		// 表情名の読み飛ばし
		if( PmdSkinNum > 1 )
		{
			Src += ( PmdSkinNum - 1 ) * 20 ;
		}

		// ボーン枠用枠名の読み飛ばし
		Src += BoneDispNum * 50 ;
	}

	// これ以上データが存在しなかったら物理演算用のデータはないということ
	if( Src - ( BYTE * )LoadParam->DataBuffer >= LoadParam->DataSize ) goto PHYSICSDATAREADEND ;

	// トゥーンシェーディング用テクスチャファイル名リストのアドレスをセット
	PmdToonFileName = ( char * )Src ;
	Src += 100 * 10 ;

	// これ以上データが存在しなかったら物理演算用のデータはないということ
	if( Src - ( BYTE * )LoadParam->DataBuffer >= LoadParam->DataSize ) goto PHYSICSDATAREADEND ;

#ifndef DX_NON_BULLET_PHYSICS
	// 物理演算データの読み込み
	if( LoadParam->GParam.LoadModelToUsePhysicsMode != DX_LOADMODEL_PHYSICS_DISABLE )
	{
		// 物理演算データがある場合のみ処理をする
		if( *( ( DWORD * )Src ) > 0 && *( ( DWORD * )Src ) != 0xfdfdfdfd )
		{
			PMD_READ_PHYSICS_INFO *PhysicsInfo ;
			PMD_READ_PHYSICS_JOINT_INFO *JointInfo ;

			// ここにきたら物理演算用データがあるということ
			ValidPhysics = TRUE ;

			// 物理演算情報の読み込み

			// ゼロ初期化
			_MEMSET( &MLPhysicsInfo, 0, sizeof( DX_MODELLOADER3_PMD_PHYSICS_INFO ) ) ;

			// 物理演算情報の数を取得
			MLPhysicsInfo.PmdPhysicsNum = *( ( DWORD * )Src ) ;
			Src += 4 ;

			// データを格納するメモリ領域の確保
			MLPhysicsInfo.PmdPhysicsInfoDim = ( PMD_READ_PHYSICS_INFO * )DXALLOC( sizeof( PMD_READ_PHYSICS_INFO ) * MLPhysicsInfo.PmdPhysicsNum ) ;
			if( MLPhysicsInfo.PmdPhysicsInfoDim == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 物理演算情報配列を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// データを読み出す
		//	PmdPhysics = ( PMD_PHYSICS * )Src ;
			PhysicsInfo = MLPhysicsInfo.PmdPhysicsInfoDim ;
			for( i = 0 ; i < MLPhysicsInfo.PmdPhysicsNum ; i ++, PhysicsInfo ++, Src += 83 )
			{
				PhysicsInfo->Base = ( PMD_PHYSICS * )Src ;

				_MEMCPY( PhysicsInfo->Name, ( char * )Src, 20 ) ;
				PhysicsInfo->Name[ 20 ] = '\0' ;
				PhysicsInfo->ShapeType = ( int )Src[ 25 ] ;
				PhysicsInfo->BoneIndex = *( ( WORD * )&Src[ 20 ] ) ;
				if( PhysicsInfo->BoneIndex == 0xffff )
				{
					for( j = 0 ; _STRCMP( BoneInfoDim[ j ].Frame->Name, "センター" ) != 0 ; j ++ ){}
					PhysicsInfo->Bone = &BoneInfoDim[ j ] ;
				}
				else
				{
					PhysicsInfo->Bone = &BoneInfoDim[ PhysicsInfo->BoneIndex ] ;
				}
				PhysicsInfo->Bone->IsPhysics = 1 ;
				PhysicsInfo->Bone->PhysicsIndex = i ;
				PhysicsInfo->Bone->SetupPhysicsAnim = 0 ;

				// 剛体タイプを保存
				PhysicsInfo->RigidBodyType = Src[ 82 ] ;
				PhysicsInfo->NoCopyToBone = _STRCMP( PhysicsInfo->Bone->Frame->Name, "センター" ) == 0 ;
			}

			// ジョイント情報の数を取得
			MLPhysicsInfo.PmdPhysicsJointNum = *( ( DWORD * )Src ) ;
			Src += 4 ;

			// ジョイント情報がある場合のみ処理
			if( MLPhysicsInfo.PmdPhysicsJointNum != 0 )
			{
				// データを格納するメモリ領域の確保
				MLPhysicsInfo.PmdPhysicsJointInfoDim = ( PMD_READ_PHYSICS_JOINT_INFO * )DXALLOC( sizeof( PMD_READ_PHYSICS_JOINT_INFO ) * MLPhysicsInfo.PmdPhysicsJointNum ) ;
				if( MLPhysicsInfo.PmdPhysicsJointInfoDim == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 物理演算ジョイント情報を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}

				// データを読み出す
			//	PmdPhysicsJoint = ( PMD_PHYSICS_JOINT * )Src ;
				JointInfo = MLPhysicsInfo.PmdPhysicsJointInfoDim ;
				for( i = 0 ; i < MLPhysicsInfo.PmdPhysicsJointNum ; i ++, JointInfo ++, Src += 124 )
				{
					JointInfo->Base = ( PMD_PHYSICS_JOINT * )Src ;
				}
			}

			// もしリアルタイム物理処理を行う場合は読み込み情報に物理ボーンと物理ジョイントの情報を加える
			if( LoadParam->GParam.LoadModelToUsePhysicsMode == DX_LOADMODEL_PHYSICS_REALTIME )
			{
				MV1_PHYSICS_RIGIDBODY_R *RigidBody ;
				MV1_PHYSICS_JOINT_R *Joint ;

				PhysicsInfo = MLPhysicsInfo.PmdPhysicsInfoDim ;
				for( i = 0 ; i < MLPhysicsInfo.PmdPhysicsNum ; i ++, PhysicsInfo ++ )
				{
					RigidBody = MV1RAddPhysicsRididBody( &RModel, PhysicsInfo->Name, PhysicsInfo->Bone->Frame ) ;
					if( RigidBody == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 物理計算用剛体情報の追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					RigidBody->RigidBodyGroupIndex = ( int )*( ( BYTE * )&PhysicsInfo->Base->Data[ 22 ] ) ;
					RigidBody->RigidBodyGroupTarget = *( ( WORD * )&PhysicsInfo->Base->Data[ 23 ] ) ;
					RigidBody->ShapeType = ( int )*( ( BYTE * )&PhysicsInfo->Base->Data[ 25 ] ) ;
					*( ( DWORD * )&RigidBody->ShapeW ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 26 ] ) ;
					*( ( DWORD * )&RigidBody->ShapeH ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 30 ] ) ;
					*( ( DWORD * )&RigidBody->ShapeD ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 34 ] ) ;
					*( ( DWORD * )&RigidBody->Position.x ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 38 ] ) ;
					*( ( DWORD * )&RigidBody->Position.y ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 42 ] ) ;
					*( ( DWORD * )&RigidBody->Position.z ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 46 ] ) ;
					*( ( DWORD * )&RigidBody->Rotation.x ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 50 ] ) ;
					*( ( DWORD * )&RigidBody->Rotation.y ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 54 ] ) ;
					*( ( DWORD * )&RigidBody->Rotation.z ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 58 ] ) ;
					*( ( DWORD * )&RigidBody->RigidBodyWeight ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 62 ] ) ;
					*( ( DWORD * )&RigidBody->RigidBodyPosDim ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 66 ] ) ;
					*( ( DWORD * )&RigidBody->RigidBodyRotDim ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 70 ] ) ;
					*( ( DWORD * )&RigidBody->RigidBodyRecoil ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 74 ] ) ;
					*( ( DWORD * )&RigidBody->RigidBodyFriction ) = *( ( DWORD * )&PhysicsInfo->Base->Data[ 78 ] ) ;
					RigidBody->RigidBodyType = ( int )*( ( BYTE * )&PhysicsInfo->Base->Data[ 82 ] ) ;
					RigidBody->NoCopyToBone = _STRCMP( PhysicsInfo->Bone->Frame->Name, "センター" ) == 0 ? TRUE : FALSE ;
				}

				JointInfo = MLPhysicsInfo.PmdPhysicsJointInfoDim ;
				for( i = 0 ; i < MLPhysicsInfo.PmdPhysicsJointNum ; i ++, JointInfo ++ )
				{
					Joint = MV1RAddPhysicsJoint( &RModel, JointInfo->Base->Name ) ;
					if( Joint == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 物理計算用剛体接合情報の追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					Joint->RigidBodyA = JointInfo->Base->RigidBodyA ;
					Joint->RigidBodyB = JointInfo->Base->RigidBodyB ;
					Joint->Position.x = JointInfo->Base->Position[ 0 ] ;
					Joint->Position.y = JointInfo->Base->Position[ 1 ] ;
					Joint->Position.z = JointInfo->Base->Position[ 2 ] ;
					Joint->Rotation.x = JointInfo->Base->Rotation[ 0 ] ;
					Joint->Rotation.y = JointInfo->Base->Rotation[ 1 ] ;
					Joint->Rotation.z = JointInfo->Base->Rotation[ 2 ] ;
					Joint->ConstrainPosition1.x = JointInfo->Base->ConstrainPosition1[ 0 ] ;
					Joint->ConstrainPosition1.y = JointInfo->Base->ConstrainPosition1[ 1 ] ;
					Joint->ConstrainPosition1.z = JointInfo->Base->ConstrainPosition1[ 2 ] ;
					Joint->ConstrainPosition2.x = JointInfo->Base->ConstrainPosition2[ 0 ] ;
					Joint->ConstrainPosition2.y = JointInfo->Base->ConstrainPosition2[ 1 ] ;
					Joint->ConstrainPosition2.z = JointInfo->Base->ConstrainPosition2[ 2 ] ;
					Joint->ConstrainRotation1.x = JointInfo->Base->ConstrainRotation1[ 0 ] ;
					Joint->ConstrainRotation1.y = JointInfo->Base->ConstrainRotation1[ 1 ] ;
					Joint->ConstrainRotation1.z = JointInfo->Base->ConstrainRotation1[ 2 ] ;
					Joint->ConstrainRotation2.x = JointInfo->Base->ConstrainRotation2[ 0 ] ;
					Joint->ConstrainRotation2.y = JointInfo->Base->ConstrainRotation2[ 1 ] ;
					Joint->ConstrainRotation2.z = JointInfo->Base->ConstrainRotation2[ 2 ] ;
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
	else
#endif
	// 物理演算データの分だけアドレスを進める
	{
		// 物理演算データがある場合のみ進める
		if( *( ( DWORD * )Src ) > 0 && *( ( DWORD * )Src ) != 0xfdfdfdfd )
		{
			// 物理演算情報の分だけ進める
			i = *( ( DWORD * )Src ) ;
			Src += 83 * i + 4 ;

			// ジョイント情報の分だけ進める
			i = *( ( DWORD * )Src ) ;
			Src += 124 * i + 4 ;
		}
	}

PHYSICSDATAREADEND :

	// マテリアルの読みこみ
	Src = ( BYTE * )PmdMaterial ;
	facecount = 0 ;
	for( i = 0 ; ( DWORD )i < PmdMaterialNum ; i ++ )
	{
		char *ap ;

		_SPRINTF( String, "Mat_%d", i ) ;
		Material = MV1RAddMaterial( &RModel, String ) ;
		if( Material == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : No.%d の Material オブジェクトの追加に失敗しました\n" ), i ) ) ;
			goto ENDLABEL ;
		}

		// マテリアルタイプはトゥーン
		Material->Type = DX_MATERIAL_TYPE_TOON_2 ;

		// 情報をセット
		*( ( DWORD * )&Material->Diffuse.r ) = *( ( DWORD * )&Src[  0 ] ) ;
		*( ( DWORD * )&Material->Diffuse.g ) = *( ( DWORD * )&Src[  4 ] ) ;
		*( ( DWORD * )&Material->Diffuse.b ) = *( ( DWORD * )&Src[  8 ] ) ;
		Src += 12 ;

		*( ( DWORD * )&Material->Diffuse.a ) = *( ( DWORD * )&Src[ 0 ] ) ;
		Src += 4 ;

		Material->Power = *( ( float * )Src ) ;
		Src += 4 ;

		*( ( DWORD * )&Material->Specular.r ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&Material->Specular.g ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&Material->Specular.b ) = *( ( DWORD * )&Src[ 8 ] ) ;
		Material->Specular.a = 0.0f ;
		Src += 12 ;

		*( ( DWORD * )&Material->Ambient.r ) = *( ( DWORD * )&Src[ 0 ] ) ;
		*( ( DWORD * )&Material->Ambient.g ) = *( ( DWORD * )&Src[ 4 ] ) ;
		*( ( DWORD * )&Material->Ambient.b ) = *( ( DWORD * )&Src[ 8 ] ) ;
		Material->Ambient.a = 0.0f ;
		Src += 12 ;

		// トゥーンテクスチャを追加
		ToonTexIndex = *Src ;
		{
			// スフィアマップのファイル名が含まれる場合はそれを除く
			if( *Src != 0xff && PmdToonFileName != NULL )
			{
				_STRCPY( String, &PmdToonFileName[ *Src * 100 ] ) ;
				ap = _STRCHR( String, '*' ) ;
				if( ap != NULL ) *ap = '\0' ;
			}
			else
			{
				_STRCPY( String, "toon0.bmp" ) ;
			}
			Texture = MV1RAddTexture( &RModel, String, String, NULL, FALSE, 0.1f, true, true, true ) ;
			if( Texture == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : No.%d の トゥーン用テクスチャオブジェクトの作成に失敗しました\n" ), i ) ) ;
				goto ENDLABEL ;
			}
			Texture->AddressModeU = DX_TEXADDRESS_CLAMP ;
			Texture->AddressModeV = DX_TEXADDRESS_CLAMP ;
			Material->DiffuseGradTexture = Texture ;
			Material->DiffuseGradBlendType = MV1_LAYERBLEND_TYPE_MODULATE ;
		}
		Material->SpecularGradTextureDefault = -1 ;
		Material->SpecularGradBlendType = MV1_LAYERBLEND_TYPE_ADDITIVE ;
		Src ++ ;

		// 線の太さをセット
		if( *Src )
		{
			Material->OutLineWidth = 0.01f ;
			Material->OutLineDotWidth = 1.1f ;
		}
		Src ++ ;

		// 面データにマテリアル番号をセットする
		PmdMaterialFaceVertexNum = *( ( DWORD * )Src ) ;
		Src += 4 ;
		for( j = 0 ; ( DWORD )j < PmdMaterialFaceVertexNum ; j += 3, facecount ++ )
		{
			Mesh->Faces[ facecount ].MaterialIndex = Mesh->MaterialNum ;
		}
		
		// テクスチャを追加
		_MEMSET( String, 0, sizeof( String ) ) ;
		_MEMCPY( String, Src, 20 ) ;
		Src += 20 ;
		if( String[ 0 ] != '\0' )
		{
			char SphFileName[ 32 ], TexFileName[ 32 ] ;
			int SphBlendType ;
			int len ;

			SphFileName[ 0 ] = '\0' ;
			TexFileName[ 0 ] = '\0' ;
			SphBlendType = DX_MATERIAL_BLENDTYPE_MODULATE ;

			// ファイル名解析
			for(;;)
			{
				for( j = 0 ; String[ j ] != '.' && String[ j ] != '\0' && String[ j ] != '*' ; j ++ ){}

				// ピリオドに行きついたらスフィアマップかどうかを判定するために拡張子検査
				if( String[ j ] == '.' )
				{
					// 最初のファイル名がスフィアマップかどうかで処理を分岐
					if( String[ j + 1 ] == 's' && String[ j + 2 ] == 'p' && ( String[ j + 3 ] == 'h' || String[ j + 3 ] == 'a' ) )
					{
						// スフィアマップのファイル名を保存する
						_MEMCPY( SphFileName, String, j + 4 ) ;
						SphFileName[ j + 4 ] = '\0' ;

						// スフィアマップのブレンド方式を調べる
						SphBlendType = String[ j + 3 ] == 'a' ? DX_MATERIAL_BLENDTYPE_ADDITIVE : DX_MATERIAL_BLENDTYPE_MODULATE ;
						break ;
					}

					// スフィアマップではなかった場合は普通のテクスチャなので、継続して文字列の終端まで j を進める
					for( j = 0 ; String[ j ] != '\0' && String[ j ] != '*' ; j ++ ){}
				}

				// テクスチャファイル名を保存
				_MEMCPY( TexFileName, String, j ) ;
				TexFileName[ j ] = '\0' ;

				// 文字列の終端だったらここで処理は終了
				if( String[ j ] == '\0' )
					break ;

				// ここにきたら String[ j ] == '*' ということ
				j ++ ;

				// スフィアマップのファイル名を保存する
				len = _STRLEN( &String[ j ] ) ;
				_MEMCPY( SphFileName, &String[ j ], len ) ;
				SphFileName[ len ] = '\0' ;

				// スフィアマップのブレンド方式を調べる
				SphBlendType = String[ j + len - 1 ] == 'a' ? DX_MATERIAL_BLENDTYPE_ADDITIVE : DX_MATERIAL_BLENDTYPE_MODULATE ;
				break ;
			}

//			ap = _STRCHR( String, '*' ) ;
//			if( ap != NULL ) *ap = '\0' ;

			// 普通のテクスチャがある場合の処理
			if( TexFileName[ 0 ] != '\0' )
			{
				Texture = MV1RAddTexture( &RModel, TexFileName, TexFileName, NULL, false, 0.1f, true, false, true ) ;
				if( Texture == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : No.%d の テクスチャオブジェクトの作成に失敗しました\n" ), i ) ) ;
					goto ENDLABEL ;
				}
				Material->DiffuseTexNum = 1 ;
				Material->DiffuseTexs[ 0 ] = Texture ;
			}

			// スフィアマップテクスチャがある場合の処理
			if( SphFileName[ 0 ] != '\0' )
			{
				Texture = MV1RAddTexture( &RModel, SphFileName, SphFileName, NULL, false, 0.1f, true, false, true ) ;
				if( Texture == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : No.%d の スフィアマップテクスチャオブジェクトの作成に失敗しました\n" ), i ) ) ;
					goto ENDLABEL ;
				}
				Material->SphereMapTexture = Texture ;
				Material->SphereMapBlendType = SphBlendType ;
			}
		}


		// マテリアルが１ポリゴンにでも使用されている場合はマテリアルのアドレスをセット
		if( PmdMaterialFaceVertexNum )
		{
			Mesh->Materials[ Mesh->MaterialNum ] = Material ;
			Mesh->MaterialNum ++ ;
		}
	}

	// 表情データの追加
	if( PmdSkinNum > 1 )
	{
		Src = PmdSkin ;

		// 表情データ本体の頂点数を取得
		Src += 20 ;
		SkinBaseVertNum = *( ( DWORD * )Src ) ;

		// 表情データの頂点数が０の場合は表情データの読み込みをしない
		if( SkinBaseVertNum != 0 )
		{
			SkinMesh = MV1RAddMesh( &RModel, Frame ) ;
			if( SkinMesh == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : スキンメッシュオブジェクトの追加に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// base メッシュから元モデルから使用する頂点と面の情報を割り出す
			SkinNextVertIndex = ( DWORD * )DXALLOC( sizeof( DWORD ) * ( 2 * PmdVertexNum + 2 * PmdFaceNum + 2 * PmdMaterialNum ) ) ;
			if( SkinNextVertIndex == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 一時保存用の頂点データを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}
			SkinPrevVertIndex     = SkinNextVertIndex     + PmdVertexNum ;
			SkinNextFaceIndex     = SkinPrevVertIndex     + PmdVertexNum ;
			SkinPrevFaceIndex     = SkinNextFaceIndex     + PmdFaceNum ;
			SkinNextMaterialIndex = SkinPrevFaceIndex     + PmdFaceNum ;
			SkinPrevMaterialIndex = SkinNextMaterialIndex + PmdMaterialNum ;
			_MEMSET( SkinNextVertIndex,     0xff, sizeof( DWORD ) * PmdVertexNum   ) ;
			_MEMSET( SkinPrevVertIndex,     0xff, sizeof( DWORD ) * PmdVertexNum   ) ;
			_MEMSET( SkinNextFaceIndex,     0xff, sizeof( DWORD ) * PmdFaceNum     ) ;
			_MEMSET( SkinPrevFaceIndex,     0xff, sizeof( DWORD ) * PmdFaceNum     ) ;
			_MEMSET( SkinNextMaterialIndex, 0xff, sizeof( DWORD ) * PmdMaterialNum ) ;
			_MEMSET( SkinPrevMaterialIndex, 0xff, sizeof( DWORD ) * PmdMaterialNum ) ;

			// 表情データ本体で使用する頂点の元のメッシュでの頂点番号を取得
			Src += 5 ;
			PmdBaseSkinVert = ( PMD_SKIN_VERT * )Src ;
			SkinVertNum = 0 ;
			for( i = 0 ; ( DWORD )i < SkinBaseVertNum ; i ++, Src += 16 )
			{
				SkinTargetVert = *( ( DWORD * )Src ) ;
				SkinPrevVertIndex[ SkinVertNum ] = SkinTargetVert ;
				SkinNextVertIndex[ SkinTargetVert ] = SkinVertNum ;
				SkinVertNum ++ ;
			}

			// 表情データで使用されている頂点を使っている面の数を数える
			MeshFace = Mesh->Faces ;
			SkinFaceNum = 0 ;
			SkinMaterialNum = 0 ;
			for( i = 0 ; ( DWORD )i < PmdFaceNum ; i ++, MeshFace ++ )
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

			// 表情データで私用する面を保存するメモリ領域を確保する
			if( MV1RSetupMeshFaceBuffer( &RModel, SkinMesh, SkinFaceNum, 3 ) < 0 )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 表情データの面情報を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			// 表情データで使用する頂点を保存するメモリ領域を確保する
			SkinMesh->PositionNum = SkinVertNum ;
			SkinMesh->Positions = ( VECTOR * )ADDMEMAREA( ( sizeof( VECTOR ) + sizeof( float ) ) * SkinMesh->PositionNum, &RModel.Mem ) ;
			if( SkinMesh->Positions == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 表情データの頂点座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}
			SkinMesh->PositionToonOutLineScale = ( float * )( SkinMesh->Positions + SkinMesh->PositionNum ) ;

			SkinMesh->NormalNum = SkinVertNum ;
			SkinMesh->Normals = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * SkinMesh->NormalNum, &RModel.Mem ) ;
			if( SkinMesh->Normals == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 表情データの頂点法線を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
				goto ENDLABEL ;
			}

			SkinMesh->UVNum[ 0 ] = SkinVertNum ;
			SkinMesh->UVs[ 0 ] = ( FLOAT4 * )ADDMEMAREA( sizeof( FLOAT4 ) * SkinMesh->UVNum[ 0 ], &RModel.Mem ) ;
			if( SkinMesh->UVs[ 0 ] == NULL )
			{
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 表情データの頂点テクスチャ座標を保存するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 表情データ用スキニングメッシュウエイト値を格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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

			// 残りの表情データを追加する
			for( i = 1 ; i < PmdSkinNum ; i ++ )
			{
				// 表情データの追加
				_MEMSET( String, 0, sizeof( String ) ) ;
				_MEMCPY( String, Src, 20 ) ;
				Shape = MV1RAddShape( &RModel, String, Frame ) ; 
				if( Shape == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : シェイプオブジェクトの追加に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}
				Src += 20 ;

				// 対象メッシュのセット
				Shape->TargetMesh = SkinMesh ;

				// 法線は無し
				Shape->ValidVertexNormal = FALSE ;

				// 頂点の数を保存
				Shape->VertexNum = *( ( int * )Src ) ;
				Src += 5 ;

				// 頂点データを格納するメモリ領域の確保
				Shape->Vertex = ( MV1_SHAPE_VERTEX_R * )ADDMEMAREA( sizeof( MV1_SHAPE_VERTEX_R ) * Shape->VertexNum, &RModel.Mem ) ;
				if( Shape->Vertex == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : シェイプ頂点データを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}

				// 頂点データを埋める
				ShapeVert = Shape->Vertex ;
				for( j = 0 ; j < Shape->VertexNum ; j ++, ShapeVert ++, Src += 16 )
				{
					SkinTargetVert = *( ( DWORD * )&Src[ 0 ] ) ;
					PmdSkinVert = &PmdBaseSkinVert[ SkinTargetVert ] ;
					ShapeVert->TargetPositionIndex = SkinTargetVert ;
					ShapeVert->Position.x = ( PmdSkinVert->Position[ 0 ] - SkinMesh->Positions[ SkinTargetVert ].x ) + *( ( float * )&Src[  4 ] ) ;
					ShapeVert->Position.y = ( PmdSkinVert->Position[ 1 ] - SkinMesh->Positions[ SkinTargetVert ].y ) + *( ( float * )&Src[  8 ] ) ;
					ShapeVert->Position.z = ( PmdSkinVert->Position[ 2 ] - SkinMesh->Positions[ SkinTargetVert ].z ) + *( ( float * )&Src[ 12 ] ) ;
				}
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
				MV1LoadModelToPMD_SetupMatrix( BoneInfoDim, PmdBoneNum, TRUE, FALSE ) ;

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
				Result = _MV1LoadModelToVMD_PMD(
					k,
					&RModel,
					VmdData,
					FileSize,
					String,
					BoneInfoDim,
					PmdBoneNum,
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
		for( i = 0 ; i < PmdBoneNum ; i ++ )
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
		if( MLPhysicsInfo.PmdPhysicsInfoDim != NULL )
		{
			DXFREE( MLPhysicsInfo.PmdPhysicsInfoDim ) ;
			MLPhysicsInfo.PmdPhysicsInfoDim = NULL ;
		}

		if( MLPhysicsInfo.PmdPhysicsJointInfoDim != NULL )
		{
			DXFREE( MLPhysicsInfo.PmdPhysicsJointInfoDim ) ;
			MLPhysicsInfo.PmdPhysicsJointInfoDim = NULL ;
		}
	}
#endif

	// 読み込みようモデルを解放
	MV1TermReadModel( &RModel ) ; 

	// 返回句柄
	return NewHandle ;
}

// 行列を計算する( 骨一つだけ )
static void MV1LoadModelToPMD_SetupMatrix_One( PMD_READ_BONE_INFO *BoneInfo )
{
	MATRIX Matrix ;

	float	x2 = BoneInfo->Rotate.x * BoneInfo->Rotate.x * 2.0f ;
	float	y2 = BoneInfo->Rotate.y * BoneInfo->Rotate.y * 2.0f ;
	float	z2 = BoneInfo->Rotate.z * BoneInfo->Rotate.z * 2.0f ;
	float	xy = BoneInfo->Rotate.x * BoneInfo->Rotate.y * 2.0f ;
	float	yz = BoneInfo->Rotate.y * BoneInfo->Rotate.z * 2.0f ;
	float	zx = BoneInfo->Rotate.z * BoneInfo->Rotate.x * 2.0f ;
	float	xw = BoneInfo->Rotate.x * BoneInfo->Rotate.w * 2.0f ;
	float	yw = BoneInfo->Rotate.y * BoneInfo->Rotate.w * 2.0f ;
	float	zw = BoneInfo->Rotate.z * BoneInfo->Rotate.w * 2.0f ;

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
	Matrix.m[ 3 ][ 0 ] = BoneInfo->Translate.x ;
	Matrix.m[ 3 ][ 1 ] = BoneInfo->Translate.y ;
	Matrix.m[ 3 ][ 2 ] = BoneInfo->Translate.z ;
	Matrix.m[ 3 ][ 3 ] = 1.0f ;

	if( BoneInfo->Frame->Parent == NULL )
	{
		BoneInfo->LocalWorldMatrix = Matrix ;
	}
	else
	{
		MV1LoadModelToVMD_CreateMultiplyMatrix( &BoneInfo->LocalWorldMatrix, &Matrix, &( ( PMD_READ_BONE_INFO * )BoneInfo->Frame->Parent->UserData )->LocalWorldMatrix ) ;
	}
}

// 行列を計算する
static void MV1LoadModelToPMD_SetupMatrix( PMD_READ_BONE_INFO *BoneInfo, int BoneNum, int UseInitParam, int IKSkip )
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
			MV1LoadModelToVMD_CreateMultiplyMatrix( &BoneInfo->LocalWorldMatrix, &Matrix, &( ( PMD_READ_BONE_INFO * )BoneInfo->Frame->Parent->UserData )->LocalWorldMatrix ) ;
		}
	}
}

// ＩＫを計算する
static void MV1LoadModelToPMD_SetupIK( PMD_READ_BONE_INFO *BoneInfo, PMD_READ_IK_INFO *IKInfoFirst )
{
	int i, j, k ;
	PMD_READ_BONE_INFO *ChainBone ;
	PMD_READ_BONE_INFO *ChainParentBone ;
	VECTOR IKBonePos ;
	VECTOR BonePos ;
	VECTOR TargPos ;
	float Rot ;
	float Cos ;
	float IKsin, IKcos ;
	FLOAT4 qIK ;
	PMD_READ_IK_INFO *IKInfo ;

	for( IKInfo = IKInfoFirst ; IKInfo ; IKInfo = IKInfo->Next )
	{
		// IKのターゲットのワールド座標を取得しておく
		IKBonePos.x = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 0 ];
		IKBonePos.y = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 1 ];
		IKBonePos.z = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 2 ];

		QuatConvertToMatrix( IKInfo->TargetBone->IKmat, IKInfo->TargetBone->IKQuat, IKInfo->TargetBone->Translate );

		for( i = 0 ; i < IKInfo->Iterations ; i ++ )
		{
			for( j = 0 ; j < IKInfo->ChainBoneNum ; j ++ )
			{
				ChainBone = &BoneInfo[ IKInfo->ChainBone[ j ] ] ;
				ChainParentBone = ( PMD_READ_BONE_INFO * )ChainBone->Frame->Parent->UserData ;

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
				if( ChainBone->IKLimitAngle != 0 )
				{
					// if bone=knee roll only x-axis +
					v.x =  v.x * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 0 ] + v.y * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 1 ] + v.z * ChainParentBone->LocalWorldMatrix.m[ 0 ][ 2 ] >= 0.0f ? 1.0f : -1.0f ;
					v.y = 0.0f ;
					v.z = 0.0f ;
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
				if( Rot > IKInfo->ControlWeight * ( j + 1 ) * 2 )
					Rot = IKInfo->ControlWeight * ( j + 1 ) * 2 ;

				_SINCOS( Rot, &IKsin, &IKcos );
				qIK.x = v.x * IKsin;
				qIK.y = v.y * IKsin;
				qIK.z = v.z * IKsin;
				qIK.w = IKcos;

				ChainBone->IKQuat = QuatMul( qIK, ChainBone->IKQuat );

				QuatConvertToMatrix( ChainBone->IKmat, ChainBone->IKQuat, ChainBone->Translate );
				if( ChainBone->IKLimitAngle != 0 )
				{
					if( _ATAN2( ChainBone->IKmat.m[2][1], ChainBone->IKmat.m[2][2] ) < 0.0f )
					{
						ChainBone->IKmat.m[1][2] = -ChainBone->IKmat.m[1][2];
						ChainBone->IKmat.m[2][1] = -ChainBone->IKmat.m[2][1];
						ChainBone->IKQuat.x = -ChainBone->IKQuat.x;
					}
				}

				for( k = j ; k >= 0 ; k-- )
				{
					ChainBone= &BoneInfo[ IKInfo->ChainBone[ k ] ];
					MV1LoadModelToVMD_CreateMultiplyMatrix( &ChainBone->LocalWorldMatrix, &ChainBone->IKmat, &( ( PMD_READ_BONE_INFO * )ChainBone->Frame->Parent->UserData )->LocalWorldMatrix ) ;
				}

				MV1LoadModelToVMD_CreateMultiplyMatrix( &IKInfo->TargetBone->LocalWorldMatrix, &IKInfo->TargetBone->IKmat, &( ( PMD_READ_BONE_INFO * )IKInfo->TargetBone->Frame->Parent->UserData )->LocalWorldMatrix );
			}

			for( j = 0 ; j < IKInfo->ChainBoneNum ; j ++ )
			{
				ChainBone = &BoneInfo[ IKInfo->ChainBone[ j ] ] ;

				ChainBone->Rotate = ChainBone->IKQuat ;
			}
			IKInfo->TargetBone->Rotate = IKInfo->TargetBone->IKQuat ;
		}
	}
}

#if 0
// ＩＫを計算する
static void MV1LoadModelToPMD_SetupIK( PMD_READ_BONE_INFO *BoneInfo, PMD_READ_IK_INFO *IKInfoFirst, int Time, int LoopNo )
{
	PMD_READ_IK_INFO *IKInfo ;
	PMD_READ_BONE_INFO *IKLockChainBone ;
	int i ;

	for( IKInfo = IKInfoFirst ; IKInfo ; IKInfo = IKInfo->Next )
	{
		/*
		if( _MEMCMP( IKInfo->Bone->Base->Data, "左足ＩＫ", 4 * 2 ) == 0 )
		{
			IKInfo = IKInfo ;
		}
		*/

		// 最初は角度リミット無しでＩＫ
		if( MV1LoadModelToPMD_SetupOneIK( BoneInfo, IKInfo, FALSE, &IKLockChainBone ) == 1 )
		{
			// リミット越えしてしまったらリミットありでＩＫ

			// ボーンの行列の状態を初期に戻す
			for( i = 0 ; i < IKInfo->ChainBoneNum ; i ++ )
			{
				MV1LoadModelToPMD_SetupOneBoneMatrixFormAnimKey( &BoneInfo[ IKInfo->ChainBone[ i ] ], Time, LoopNo ) ;
			}

			// リミットありでＩＫ
			MV1LoadModelToPMD_SetupOneIK( BoneInfo, IKInfo, TRUE, NULL ) ;
		}
	}
}

// 指定のＩＫを計算する( 戻り値 0:リミット越え無し  1:リミット越えあり )
static int MV1LoadModelToPMD_SetupOneIK( PMD_READ_BONE_INFO *BoneInfo, PMD_READ_IK_INFO *IKInfo, int IKLimitEnable, PMD_READ_BONE_INFO **IKLimitBone )
{
	int i, j, k, AddIteration ;
	PMD_READ_BONE_INFO *ChainBone, *IKLockChainBone = NULL, *FirstChainBone ;
	VECTOR IKBonePos, WorldLastBonePos ;
	VECTOR IKBonePosLcl, LastBonePosLcl ;
	VECTOR IKBoneDirLcl, LastBoneDirLcl ;
	VECTOR RotAxis, VAngle, SubV ;
	MATRIX InvMatrix ;
	FLOAT4 RotQt, RotBlend ;
	float Angle ;
	bool LimitFlag, LimitFuncCompFlag ;
	bool FreezeUnlockFlag, AngleCheck, LimitAnglePrev ;
	bool LoopNormalEnd ;
	float Cos ;

//	int zz0 = 0, zz1 = 0, zz2 = 0, zz3 = 0, zz4 = 0, zz5 = 0, zz6 = 0, zz7 = 0, zz8 = 0, zz9 = 0, zz10 = 0, zz11 = 0, zz12 = 0, zz13 = 0, zz14 = 0, zz15 = 0, zz16 = 0 ;

	LimitFlag = false ;
	LimitFuncCompFlag = false ;
	FreezeUnlockFlag = false ;
	AddIteration = IKInfo->LimitAngleIK ? 50 : 0 ;

	// IKのターゲットのワールド座標を取得しておく
	IKBonePos.x = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 0 ] ;
	IKBonePos.y = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 1 ] ;
	IKBonePos.z = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 2 ] ;

	// IK処理を行うボーンの行列を計算しておく
	for( k = IKInfo->ChainBoneNum - 1 ; k >= 0 ; k -- )
	{
		MV1LoadModelToPMD_SetupMatrix_One( &BoneInfo[ IKInfo->ChainBone[ k ] ] ) ;
	}
	MV1LoadModelToPMD_SetupMatrix_One( IKInfo->TargetBone ) ;

	FirstChainBone = &BoneInfo[ IKInfo->ChainBone[ IKInfo->ChainBoneNum - 1 ] ] ;

	for( i = 0 ; i < IKInfo->Iterations + AddIteration && i < 200 ; i ++ )
	{
		for( j = 0 ; j < IKInfo->ChainBoneNum ; j ++ )
		{
			LoopNormalEnd = false ;

//			zz13 ++ ;

			// 角度制限のあるボーンの一つ上の親かどうかを保存
			LimitAnglePrev = j > 0 && BoneInfo[ IKInfo->ChainBone[ j - 1 ] ].IKLimitAngle == 1 ;

			// IKのターゲットと座標を一致させる予定のボーンの現在のワールド座標を取得しておく
			WorldLastBonePos.x = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 0 ] ;
			WorldLastBonePos.y = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 1 ] ;
			WorldLastBonePos.z = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 2 ] ;

			// 今回角度を補正するボーンのアドレスを取得
			ChainBone = &BoneInfo[ IKInfo->ChainBone[ j ] ] ;

			// 今回角度を補正するボーンの逆行列を計算
			MV1LoadModelToPMD_InverseMatrix( ChainBone->LocalWorldMatrix, InvMatrix ) ;

			// IKのターゲットの座標とIKのターゲットと座標を一致させたいボーンの座標間の距離が非常に短くなっていたら回転処理はキャンセル
			if( VSquareSize( VSub( IKBonePos, WorldLastBonePos ) ) < 0.000000001f )
			{
//				zz0 ++ ;

				// 望まない収束が起こっていないかだけチェック
				goto FREEZECHECK ;
			}

			// IKのターゲットのワールド座標とIKのターゲットと座標を一致させたいボーンのワールド座標を
			// 今回角度補正を行うボーンのローカル座標に落とし込む
			MV1LoadModelToPMD_VectorTransform( IKBonePosLcl,   IKBonePos,        InvMatrix ) ;
			MV1LoadModelToPMD_VectorTransform( LastBonePosLcl, WorldLastBonePos, InvMatrix ) ;

			// ローカル座標を正規化して原点からの方向にする
			VectorNormalize( &IKBoneDirLcl, &IKBonePosLcl ) ;
			VectorNormalize( &LastBoneDirLcl, &LastBonePosLcl ) ;

			// 二つのベクトルの成す角のコサイン値を取得
			Cos = VDot( IKBoneDirLcl, LastBoneDirLcl ) ;
			if( Cos >= 1.0f )
			{
//				zz3 ++ ;
				Cos = 1.0f ;
				Angle = 0.0f ;
			}
			else
			if( Cos <= -1.0f )
			{
				Cos = -1.0f ;
				Angle = DX_PI_F ;
			}
			else
			{
				Angle = _ACOS( Cos ) ;
			}

			// 角度制限
			if( Angle * 0.5f > IKInfo->ControlWeight * ( j + 1 ) * 2.0f )
			{
				Angle = IKInfo->ControlWeight * ( j + 1 ) * 2.0f * 2.0f ;
			}
/*
		    if( Angle < -IKInfo->ControlWeight )
			{
//				zz1 ++ ;
				Angle = -IKInfo->ControlWeight ;
			}
			else
			if( Angle >  IKInfo->ControlWeight )
			{
//				zz2 ++ ;
				Angle =  IKInfo->ControlWeight ;
			}
*/

			// ほぼボーンの角度がIKのターゲットに向いているか成す角の角度で判断
			AngleCheck = false ;
			if( LimitAnglePrev )
			{
				if( FreezeUnlockFlag == false )
				{
					// 主に太もも( 角度制限ボーンの一つ前のボーン )の関節は望まない収束が発生しやすいので判定をゆるくする
					AngleCheck = Angle < 0.001f && Angle > -0.001f ;
/*
					if( AngleCheck )
					{
						zz4 ++ ;
					}
*/
				}
				else
				{
					AngleCheck = Angle < 0.000001f && Angle > -0.000001f ;
/*
					if( AngleCheck )
					{
						zz5 ++ ;
					}
*/
				}
			}
			else
			{
				AngleCheck = Angle < 0.000001f && Angle > -0.000001f ;
/*
				if( AngleCheck )
				{
					zz6 ++ ;
				}
*/
			}

			// 成す角がほぼ０度の場合は処理を分岐
			if( AngleCheck )
			{
				// 角度制限ボーンの一つ上のボーンだった場合は望まない収束が発生していないかをチェックする
				if( LimitAnglePrev )
				{
					// IKのターゲットの座標とIKのターゲットと座標を一致させたいボーンとの距離が開いているか判定
					// ( ボーンがほぼIKのターゲットの方向を向いているのに距離が開いているのはおかしい )
					SubV = VSub( IKBonePos, WorldLastBonePos ) ;
					if( VDot( SubV, SubV ) > 0.00001f )
					{
						VECTOR FirstPos ;
						float Length ;

//						zz7 ++ ;

FREEZECHECK :
						// 望まない収束が発生した場合はIKのターゲットとIKのターゲットと座標を一致させたいボーンとの座標間の距離が
						// IKを行うボーンの全長より短いので、その条件を利用して望まない収束が発生しているかを判定
						FirstPos = VGet( FirstChainBone->LocalWorldMatrix.m[ 3 ][ 0 ], FirstChainBone->LocalWorldMatrix.m[ 3 ][ 1 ], FirstChainBone->LocalWorldMatrix.m[ 3 ][ 2 ] ) ;
						Length = VSquareSize( VSub( FirstPos, IKBonePos ) ) ;
						if( FreezeUnlockFlag == false && Length < IKInfo->IKTotalLength * IKInfo->IKTotalLength )
						{
//							zz8 ++ ;

							// 望まない収束が発生していたらわざとボーンの角度をずらす
							QuaternionToEuler( &VAngle, &ChainBone->Rotate ) ;
							VAngle.x += 0.2f;
							QuaternionCreateEuler( &ChainBone->Rotate, &VAngle ) ;

							FreezeUnlockFlag = true ;
							AddIteration += 10 ;
							goto LOOPEND ;
						}
					}
				}
//				zz16 ++ ;
//				goto LOOPEND ;
			}

			// IKのターゲットの方向にボーンの角度を向けるための回転軸を計算
			VectorOuterProduct( &RotAxis, &LastBonePosLcl, &IKBonePosLcl ) ;

			// 回転軸が限りなく０に近かったら回転はとりやめ
			if( VDot( RotAxis, RotAxis ) < 0.000001f )
			{
//				zz9 ++ ;
				goto LOOPEND ;
			}

			// 回転軸のベクトルを正規化
			VectorNormalize( &RotAxis, &RotAxis ) ;

			// 指定軸で回転するクォータニオンを作成
			{
				float Sin, Cos ;

				_SINCOS( Angle * 0.5f , &Sin, &Cos ) ;
				RotQt.x = RotAxis.x * Sin ;
				RotQt.y = RotAxis.y * Sin ;
				RotQt.z = RotAxis.z * Sin ;
				RotQt.w = Cos ;
			}

			// ボーンの角度をIKのターゲットの方向に向ける計算
			RotBlend.x = ChainBone->Rotate.w * RotQt.x + ChainBone->Rotate.x * RotQt.w + ChainBone->Rotate.y * RotQt.z - ChainBone->Rotate.z * RotQt.y ;
			RotBlend.y = ChainBone->Rotate.w * RotQt.y - ChainBone->Rotate.x * RotQt.z + ChainBone->Rotate.y * RotQt.w + ChainBone->Rotate.z * RotQt.x ;
			RotBlend.z = ChainBone->Rotate.w * RotQt.z + ChainBone->Rotate.x * RotQt.y - ChainBone->Rotate.y * RotQt.x + ChainBone->Rotate.z * RotQt.w ;
			RotBlend.w = ChainBone->Rotate.w * RotQt.w - ChainBone->Rotate.x * RotQt.x - ChainBone->Rotate.y * RotQt.y - ChainBone->Rotate.z * RotQt.z ;

			// 角度制限を加えるボーンの一つ上のボーンかどうかで処理を分岐
			if( LimitAnglePrev )
			{
				// 角度制限を加えるボーンが角度制限に引っかかった場合は
				// 望まない収束が発生している可能性があるので、その場合は
				// ボーンの角度をわざとIKのターゲットから一度逸らす
				if( LimitFlag && LimitFuncCompFlag == false )
				{
					QuaternionNormalize( &RotBlend, &RotBlend );
					QuaternionToEuler( &VAngle, &RotBlend );

					LimitFlag = false ;
					LimitFuncCompFlag = true ;
					VAngle.x += 0.2f;

//					zz14 ++ ;

					QuaternionCreateEuler( &RotBlend, &VAngle ) ;
				}
			}
			else
			// 角度制限を加えるボーンかどうかで処理を分岐
			if( ChainBone->IKLimitAngle )
			{
				QuaternionNormalize( &RotBlend, &RotBlend );
				QuaternionToEuler( &VAngle, &RotBlend );

				// Ｘ軸角度制限チェック
				if( VAngle.x > 0.0001f )
				{
//					zz10 ++ ;

					// 角度制限に引っかかったら角度を補正
					VAngle.x = 0.0001f ;
					LimitFlag = true;

					if( LimitFuncCompFlag == false )
					{
						AddIteration += 10 ;
					}
				}

				// Ｙ軸角度制限処理( 3度以上は曲がらないようにする )
			    if( VAngle.y >  3.0f / 180.0f * DX_PI_F )
				{
//					zz11 ++ ;
					VAngle.y =  3.0f / 180.0f * DX_PI_F ;
				}
				else 
				if( VAngle.y < -3.0f / 180.0f * DX_PI_F )
				{
//					zz12 ++ ;
					VAngle.y = -3.0f / 180.0f * DX_PI_F ;
				}

				// Ｚ軸は回転を完全に制限
				VAngle.z = 0.0f;

				QuaternionCreateEuler(&RotBlend, &VAngle ) ;
			}

			QuaternionNormalize( &ChainBone->Rotate, &RotBlend );

			LoopNormalEnd = true ;

LOOPEND :
			// 通常のループ終了ではない場合はここで分岐処理
			if( LoopNormalEnd == false )
			{
				// 角度制限を加えるボーンの一つ前のボーンかどうかで処理を分岐
				if( LimitAnglePrev )
				{
					// 角度制限を加えるボーンが角度制限に引っかかった場合は
					// 望まない収束が発生している可能性があるので、その場合は
					// ボーンの角度をわざとIKのターゲットから一度逸らす
					if( LimitFlag && LimitFuncCompFlag == false )
					{
//						zz15 ++ ;

						QuaternionToEuler( &VAngle, &ChainBone->Rotate ) ;
						VAngle.x += 0.2f;
						QuaternionCreateEuler( &ChainBone->Rotate, &VAngle ) ;
						QuaternionNormalize( &ChainBone->Rotate, &RotBlend );

						LimitFlag = false ;
						LimitFuncCompFlag = true ;
					}
				}
			}
/*
			char String[ 1024 ] ;
			static int jjjcount = 0 ;
			static int jjjcount2 = 0 ;
			jjjcount2 ++ ;
			if( jjjcount2 == 10 )
			{
				jjjcount2 = 0 ;

				sprintf( String, "%f %f %f %f  ", ChainBone->Rotate.x, ChainBone->Rotate.y, ChainBone->Rotate.z, ChainBone->Rotate.w ) ;
				jjjcount ++ ;
				if( jjjcount == 2 )
				{
					jjjcount = 0 ;
					strcat( String, "\n" ) ;
				}
				NS_ErrorLogAdd( String ) ;
			}
*/
			// IKの処理をしたボーンから上のボーンの行列を再計算
			for( k = j ; k >= 0 ; k -- )
			{
				MV1LoadModelToPMD_SetupMatrix_One( &BoneInfo[ IKInfo->ChainBone[ k ] ] ) ;
			}
			MV1LoadModelToPMD_SetupMatrix_One( IKInfo->TargetBone ) ;
		}
	}
/*
	char String[ 1024 ] ;
	static int jjjcount = 0 ;
	sprintf( String, "i:%d ", i ) ;
	jjjcount ++ ;
	if( jjjcount == 16 )
	{
		jjjcount = 0 ;
		strcat( String, "\n" ) ;
	}
	NS_ErrorLogAdd( String ) ;
*/
/*
	char String[ 1024 ] ;
	static int jjjcount = 0 ;
	sprintf( String, "i:%3d 0:%2d 1:%2d 2:%2d 3:%3d 4:%2d 5:%2d 6:%2d 7:%2d 8:%2d 9:%2d 10:%2d 11:%2d 12:%2d 13:%3d 14:%d 15:%d 16:%d\n", i, zz0, zz1, zz2, zz3, zz4, zz5, zz6, zz7, zz8, zz9, zz10, zz11, zz12, zz13, zz14, zz15, zz16 ) ;
	jjjcount ++ ;
	if( jjjcount == 16 )
	{
		jjjcount = 0 ;
		NS_ErrorLogAdd( String ) ;
	}
*/
	return 0 ;
}


// 指定のＩＫを計算する( 戻り値 0:リミット越え無し  1:リミット越えあり )
static int MV1LoadModelToPMD_SetupOneIK( PMD_READ_BONE_INFO *BoneInfo, PMD_READ_IK_INFO *IKInfo, int IKLimitEnable, PMD_READ_BONE_INFO **IKLimitBone )
{
	int i, j, k, retflag ;
	PMD_READ_BONE_INFO *ChainBone, *IKLockChainBone = NULL ;
	VECTOR BonePos, TargPos, BonePosLcl, TargPosLcl ;
	VECTOR RotAxis ;
	MATRIX InvMatrix ;
	FLOAT4 RotQt ;
	float Angle ;
	float AngleX ;
	BYTE flagbuf[ 256 ], *flagbuf2, *flag ;
	bool LimitFlag ;
	float TotalLength ;

	LimitFlag = false ;
	TotalLength = 0.0f ;

	BonePos.x = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 0 ] ;
	BonePos.y = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 1 ] ;
	BonePos.z = IKInfo->Bone->LocalWorldMatrix.m[ 3 ][ 2 ] ;

	for( k = IKInfo->ChainBoneNum - 1 ; k >= 0 ; k -- )
	{
		MV1LoadModelToPMD_SetupMatrix_One( &BoneInfo[ IKInfo->ChainBone[ k ] ] ) ;
	}
	MV1LoadModelToPMD_SetupMatrix_One( IKInfo->TargetBone ) ;

	flagbuf2 = NULL ;
	if( IKInfo->ChainBoneNum >= 256 )
	{
		flagbuf2 = ( BYTE * )DXALLOC( IKInfo->ChainBoneNum ) ;
		flag = flagbuf2 ;
	}
	else
	{
		flag = flagbuf ;
	}

	for( i = 0 ; i < IKInfo->ChainBoneNum ; i ++ )
	{
		flag[ i ] = 0 ;
	}

	for( i = 0 ; i < IKInfo->Iterations ; i ++ )
	{
		for( j = 0 ; j < IKInfo->ChainBoneNum ; j ++ )
		{
			TargPos.x = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 0 ] ;
			TargPos.y = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 1 ] ;
			TargPos.z = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 2 ] ;

			ChainBone = &BoneInfo[ IKInfo->ChainBone[ j ] ] ;
			CreateInverseMatrix( &InvMatrix, &ChainBone->LocalWorldMatrix ) ;

			VectorTransform( &BonePosLcl, &BonePos, &InvMatrix ) ;
			VectorTransform( &TargPosLcl, &TargPos, &InvMatrix ) ;

			if( VSquareSize( VSub( BonePosLcl, TargPosLcl ) ) < 0.0000001f ) break ; 

			VectorNormalize( &BonePosLcl, &BonePosLcl ) ;
			VectorNormalize( &TargPosLcl, &TargPosLcl ) ;

			float Cos ;
			Cos = VDot( BonePosLcl, TargPosLcl ) ;
			if( Cos > 1.0f ) Cos = 1.0f ;
			Angle = _ACOS( Cos ) ;
			if( Angle > 0.00000001f )
			{
				     if( Angle < -IKInfo->ControlWeight ) Angle = -IKInfo->ControlWeight ;
				else if( Angle >  IKInfo->ControlWeight ) Angle =  IKInfo->ControlWeight ;

				VectorOuterProduct( &RotAxis, &TargPosLcl, &BonePosLcl ) ;
				if( VDot( RotAxis, RotAxis ) < 0.0000001f )	continue ;
				VectorNormalize( &RotAxis, &RotAxis ) ;

				// 指定軸で回転するクォータニオン
				{
					float Sin, Cos ;

					if( _FABS( Angle ) < 0.0001f )
					{
						RotQt.x = 0.0f ;
						RotQt.y = 0.0f ;
						RotQt.z = 0.0f ;
						RotQt.w = 1.0f ;
					}
					else
					{
						Angle *= 0.5f ;
						
						_SINCOS( Angle, &Sin, &Cos ) ;
						RotQt.x = RotAxis.x * Sin ;
						RotQt.y = RotAxis.y * Sin ;
						RotQt.z = RotAxis.z * Sin ;
						RotQt.w = Cos ;
					}
				}

				// ひざだけは角度制限をつける
				if( ChainBone->IKLimitAngle )
				{
					IKLockChainBone = ChainBone ;

					// クォータニオンをオイラー回転値にしたときのＸ回転だけ算出する
					{
						float x2, y2, z2 ;
						float xz2, wy2, xx2 ;
						float xy2, zz2, wz2 ;
						float yz2, wx2, yy2 ;
						float temp, yRadian ;

						x2 = RotQt.x + RotQt.x ;
						y2 = RotQt.y + RotQt.y ;
						z2 = RotQt.z + RotQt.z ;
						xz2 = RotQt.x * z2 ;
						wy2 = RotQt.w * y2 ;
						temp = -( xz2 - wy2 ) ;

							 if( temp >=  1.0f ) temp =  1.0f ;
						else if( temp <= -1.0f ) temp = -1.0f ;

						yRadian = _ASIN( temp ) ;

						xx2 = RotQt.x * x2 ;
						xy2 = RotQt.x * y2 ;
						zz2 = RotQt.z * z2 ;
						wz2 = RotQt.w * z2 ;

						if( yRadian < DX_PI_F * 0.5f )
						{
							if( yRadian > -DX_PI_F * 0.5f )
							{
								yz2 = RotQt.y * z2 ;
								wx2 = RotQt.w * x2 ;
								yy2 = RotQt.y * y2 ;
								AngleX =  _ATAN2( yz2 + wx2, 1.0f - ( xx2 + yy2 ) ) ;
							}
							else
							{
								AngleX = -_ATAN2( xy2 - wz2, 1.0f - ( xx2 + zz2 ) ) ;
							}
						}
						else
						{
							AngleX = _ATAN2( xy2 - wz2, 1.0f - ( xx2 + zz2 ) ) ;
						}
					}

					if( AngleX < -DX_PI_F  )
					{
						flag[ j ] = 1 ;
						if( IKLimitEnable )
						{
							AngleX = -DX_PI_F ;
						}
					}
					else
					if( AngleX > -0.002f )
					{
						flag[ j ] = 1 ;
						if( IKLimitEnable )
						{
							AngleX = -0.002f ;
						}
					}
					else
					{
//						flag[ j ] = 0 ;
					}

					_SINCOS( AngleX * 0.5f, &RotQt.x, &RotQt.w ) ;
					RotQt.y = 0.0f ;
					RotQt.z = 0.0f ;
				}

				QuaternionNormalize( &RotQt, &RotQt );

				// 乗算
				ChainBone->Rotate.x = ChainBone->Rotate.w * RotQt.x + ChainBone->Rotate.x * RotQt.w + ChainBone->Rotate.y * RotQt.z - ChainBone->Rotate.z * RotQt.y ;
				ChainBone->Rotate.y = ChainBone->Rotate.w * RotQt.y - ChainBone->Rotate.x * RotQt.z + ChainBone->Rotate.y * RotQt.w + ChainBone->Rotate.z * RotQt.x ;
				ChainBone->Rotate.z = ChainBone->Rotate.w * RotQt.z + ChainBone->Rotate.x * RotQt.y - ChainBone->Rotate.y * RotQt.x + ChainBone->Rotate.z * RotQt.w ;
				ChainBone->Rotate.w = ChainBone->Rotate.w * RotQt.w - ChainBone->Rotate.x * RotQt.x - ChainBone->Rotate.y * RotQt.y - ChainBone->Rotate.z * RotQt.z ;

				QuaternionNormalize( &ChainBone->Rotate, &ChainBone->Rotate );

				for( k = j ; k >= 0 ; k -- )
				{
					MV1LoadModelToPMD_SetupMatrix_One( &BoneInfo[ IKInfo->ChainBone[ k ] ] ) ;
				}
				MV1LoadModelToPMD_SetupMatrix_One( IKInfo->TargetBone ) ;
			}
		}
	}

	// リミット越えチェック
	if( IKLimitEnable == FALSE )
	{
		for( j = 0 ; j < IKInfo->ChainBoneNum ; j ++ )
		{
			TargPos.x = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 0 ] ;
			TargPos.y = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 1 ] ;
			TargPos.z = IKInfo->TargetBone->LocalWorldMatrix.m[ 3 ][ 2 ] ;

			ChainBone = &BoneInfo[ IKInfo->ChainBone[ j ] ] ;
			CreateInverseMatrix( &InvMatrix, &ChainBone->LocalWorldMatrix ) ;

			VectorTransform( &BonePosLcl, &BonePos, &InvMatrix ) ;
			VectorTransform( &TargPosLcl, &TargPos, &InvMatrix ) ;

			if( VSquareSize( VSub( BonePosLcl, TargPosLcl ) ) < 0.0000001f ) continue ; 

			VectorNormalize( &BonePosLcl, &BonePosLcl ) ;
			VectorNormalize( &TargPosLcl, &TargPosLcl ) ;

			float Cos ;
			Cos = VDot( BonePosLcl, TargPosLcl ) ;
			if( Cos > 1.0f ) Cos = 1.0f ;
			Angle = _ACOS( Cos ) ;
			if( Angle > 0.00000001f )
			{
					 if( Angle < -IKInfo->ControlWeight ) Angle = -IKInfo->ControlWeight ;
				else if( Angle >  IKInfo->ControlWeight ) Angle =  IKInfo->ControlWeight ;

				VectorOuterProduct( &RotAxis, &TargPosLcl, &BonePosLcl ) ;
				if( VDot( RotAxis, RotAxis ) < 0.0000001f )	continue ;
				VectorNormalize( &RotAxis, &RotAxis ) ;

				// 指定軸で回転するクォータニオン
				{
					float Sin, Cos ;

					if( _FABS( Angle ) < 0.0001f )
					{
						RotQt.x = 0.0f ;
						RotQt.y = 0.0f ;
						RotQt.z = 0.0f ;
						RotQt.w = 1.0f ;
					}
					else
					{
						Angle *= 0.5f ;
						
						_SINCOS( Angle, &Sin, &Cos ) ;
						RotQt.x = RotAxis.x * Sin ;
						RotQt.y = RotAxis.y * Sin ;
						RotQt.z = RotAxis.z * Sin ;
						RotQt.w = Cos ;
					}
				}

				// ひざだけは角度制限をつける
				if( ChainBone->IKLimitAngle )
				{
					IKLockChainBone = ChainBone ;

					// クォータニオンをオイラー回転値にしたときのＸ回転だけ算出する
					{
						float x2, y2, z2 ;
						float xz2, wy2, xx2 ;
						float xy2, zz2, wz2 ;
						float yz2, wx2, yy2 ;
						float temp, yRadian ;

						x2 = RotQt.x + RotQt.x ;
						y2 = RotQt.y + RotQt.y ;
						z2 = RotQt.z + RotQt.z ;
						xz2 = RotQt.x * z2 ;
						wy2 = RotQt.w * y2 ;
						temp = -( xz2 - wy2 ) ;

							 if( temp >=  1.0f ) temp =  1.0f ;
						else if( temp <= -1.0f ) temp = -1.0f ;

						yRadian = _ASIN( temp ) ;

						xx2 = RotQt.x * x2 ;
						xy2 = RotQt.x * y2 ;
						zz2 = RotQt.z * z2 ;
						wz2 = RotQt.w * z2 ;

						if( yRadian < DX_PI_F * 0.5f )
						{
							if( yRadian > -DX_PI_F * 0.5f )
							{
								yz2 = RotQt.y * z2 ;
								wx2 = RotQt.w * x2 ;
								yy2 = RotQt.y * y2 ;
								AngleX =  _ATAN2( yz2 + wx2, 1.0f - ( xx2 + yy2 ) ) ;
							}
							else
							{
								AngleX = -_ATAN2( xy2 - wz2, 1.0f - ( xx2 + zz2 ) ) ;
							}
						}
						else
						{
							AngleX = _ATAN2( xy2 - wz2, 1.0f - ( xx2 + zz2 ) ) ;
						}
					}

					if( AngleX < -DX_PI_F  )
					{
						flag[ j ] = 1 ;
					}
					else
					if( AngleX > -0.002f )
					{
						flag[ j ] = 1 ;
					}
					else
					{
	//					flag[ j ] = 0 ;
					}
				}
			}
		}
	}

	for( i = 0 ; i < IKInfo->ChainBoneNum && flag[ i ] == 0 ; i ++ ){}
//	i = IKInfo->ChainBoneNum ;

	retflag = 0 ;
	if( i != IKInfo->ChainBoneNum )
	{
		VECTOR rot ;
		GetMatrixXYZRotation( &IKLockChainBone->LocalWorldMatrix, &rot.x, &rot.y, &rot.z ) ;

		retflag = 1 ;

		if( rot.x < -0.1f && rot.x > -DX_PI_F && rot.y < DX_PI_F / 4.0f && rot.y > -DX_PI_F / 4.0f )
		{
			retflag = 0 ;
		}
	}

	if( IKLimitBone != NULL )
	{
		*IKLimitBone = IKLockChainBone ;
	}

	if( flagbuf2 )
	{
		DXFREE( flagbuf2 ) ;
		flagbuf2 = NULL ;
	}

	return retflag ;
}

// ＩＫボーンのトランスレーションを回転値に変換する
static void MV1LoadModelToPMD_ConvertTransToRotate( PMD_READ_BONE_INFO *BoneInfo, PMD_READ_IK_INFO *IKInfoFirst )
{
	PMD_READ_IK_INFO *IKInfo ;
	PMD_READ_BONE_INFO *IKLockChainBone, *BackIKLockChainBone ;
	VECTOR hb1, hb2, RotAxis ;
	FLOAT4 RotQt, RotBlend ;
	int i ;
	float Dot, Angle, Sin, Cos, BoneLength ;

	// 実際はＩＫボーン中の膝が含まれるボーンだけトランスレーションをローテンションにする
	for( IKInfo = IKInfoFirst ; IKInfo ; IKInfo = IKInfo->Next )
	{
		if( IKInfo->LimitAngleIK == 0 ) continue ;

		// トータルの長さを初期化
		IKInfo->IKTotalLength = 0.0f ;

		// 根元から順に変換する
		for( i = IKInfo->ChainBoneNum - 1 ; i >= 0 ; i -- )
		{
			if( i == 0 )
			{
				IKLockChainBone     = IKInfo->TargetBone ;
				BackIKLockChainBone = &BoneInfo[ IKInfo->ChainBone[ 0 ] ] ;
			}
			else
			{
				IKLockChainBone     = &BoneInfo[ IKInfo->ChainBone[ i - 1 ] ] ;
				BackIKLockChainBone = &BoneInfo[ IKInfo->ChainBone[ i ] ] ;
			}

			BoneLength = VSize( IKLockChainBone->Translate ) ;
			IKInfo->IKTotalLength += BoneLength ;

			// 長さを測るだけで回転しないことにしました！
			continue ;

			hb1 = VGet( 0.0f, 0.0f, 1.0f );
			hb2 = VNorm( IKLockChainBone->Translate );
			Dot = VDot( hb1, hb2 );
			if( Dot > 1.0f ) Dot = 1.0f ;
			Angle = _ACOS( Dot ) ;
			if( Angle < 0.00001f && Angle > -0.00001f ) continue;

			RotAxis = VCross( hb1, hb2 ) ;
			if( VDot( RotAxis, RotAxis ) < 0.000001f ) continue;
			RotAxis = VNorm( RotAxis ) ;

			_SINCOS( Angle / 2.0f, &Sin, &Cos ) ;
			RotQt.x = RotAxis.x * Sin ;
			RotQt.y = RotAxis.y * Sin ;
			RotQt.z = RotAxis.z * Sin ;
			RotQt.w = Cos ;

			RotBlend.x = BackIKLockChainBone->Rotate.w * RotQt.x + BackIKLockChainBone->Rotate.x * RotQt.w + BackIKLockChainBone->Rotate.y * RotQt.z - BackIKLockChainBone->Rotate.z * RotQt.y ;
			RotBlend.y = BackIKLockChainBone->Rotate.w * RotQt.y - BackIKLockChainBone->Rotate.x * RotQt.z + BackIKLockChainBone->Rotate.y * RotQt.w + BackIKLockChainBone->Rotate.z * RotQt.x ;
			RotBlend.z = BackIKLockChainBone->Rotate.w * RotQt.z + BackIKLockChainBone->Rotate.x * RotQt.y - BackIKLockChainBone->Rotate.y * RotQt.x + BackIKLockChainBone->Rotate.z * RotQt.w ;
			RotBlend.w = BackIKLockChainBone->Rotate.w * RotQt.w - BackIKLockChainBone->Rotate.x * RotQt.x - BackIKLockChainBone->Rotate.y * RotQt.y - BackIKLockChainBone->Rotate.z * RotQt.z ;

			QuaternionNormalize( &RotBlend, &RotBlend ) ;

			BackIKLockChainBone->Rotate = RotBlend ;
			IKLockChainBone->Translate.z = BoneLength ;
			IKLockChainBone->Translate.x = 0.0f ;
			IKLockChainBone->Translate.y = 0.0f ;

			_SINCOS( -Angle / 2.0f, &Sin, &Cos ) ;
			RotQt.x = RotAxis.x * Sin ;
			RotQt.y = RotAxis.y * Sin ;
			RotQt.z = RotAxis.z * Sin ;
			RotQt.w = Cos ;

			RotBlend.x = IKLockChainBone->Rotate.w * RotQt.x + IKLockChainBone->Rotate.x * RotQt.w + IKLockChainBone->Rotate.y * RotQt.z - IKLockChainBone->Rotate.z * RotQt.y ;
			RotBlend.y = IKLockChainBone->Rotate.w * RotQt.y - IKLockChainBone->Rotate.x * RotQt.z + IKLockChainBone->Rotate.y * RotQt.w + IKLockChainBone->Rotate.z * RotQt.x ;
			RotBlend.z = IKLockChainBone->Rotate.w * RotQt.z + IKLockChainBone->Rotate.x * RotQt.y - IKLockChainBone->Rotate.y * RotQt.x + IKLockChainBone->Rotate.z * RotQt.w ;
			RotBlend.w = IKLockChainBone->Rotate.w * RotQt.w - IKLockChainBone->Rotate.x * RotQt.x - IKLockChainBone->Rotate.y * RotQt.y - IKLockChainBone->Rotate.z * RotQt.z ;

			QuaternionNormalize( &RotBlend, &RotBlend ) ;

			IKLockChainBone->Rotate = RotBlend ;
		}
	}
}
#endif

// 指定のボーンにアニメーションの指定キーのパラメータを反映させる
static void MV1LoadModelToPMD_SetupOneBoneMatrixFormAnimKey( PMD_READ_BONE_INFO *BoneInfo, int Time, int LoopNo )
{
	VMD_READ_KEY_INFO *NowKey, *NextKey ;

	if( BoneInfo->IsIK )
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
static int _MV1LoadModelToVMD_PMD(
	int								DataIndex,
	MV1_MODEL_R *					RModel,
	void *							DataBuffer,
	int								DataSize,
	const char *					Name,
	PMD_READ_BONE_INFO *			PmdBoneInfo,
	int								PmdBoneNum,
	PMD_READ_IK_INFO *				PmdIKInfoFirst,
#ifndef DX_NON_BULLET_PHYSICS
	DX_MODELLOADER3_PMD_PHYSICS_INFO *	MLPhysicsInfo,
#endif
	bool							FPS60
)
{
	int i, j, k ;
	VMD_READ_NODE_INFO *VmdNode ;
	VMD_READ_KEY_INFO *VmdKey, *VmdKeyTemp1, *VmdKeyTemp2 ;
	VMD_READ_FACE_KEY_SET_INFO *VmdFaceKeySet ;
	VMD_READ_FACE_KEY_INFO *VmdFaceKey ;
	PMD_READ_BONE_INFO *BoneInfo ;
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
		SetupPhysicsObject_PMDPhysicsInfo( MLPhysicsInfo ) ;
	}
#endif

	// アニメーションセットを追加
	AnimSet = MV1RAddAnimSet( RModel, Name ) ;
	if( AnimSet == NULL )
	{
		DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションセットの追加に失敗しました\n" ) ) ) ;
		goto ENDLABEL ;
	}

	// ボーンのアニメーションポインタの初期化
	BoneInfo = PmdBoneInfo ;
	for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
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

		BoneInfo = ( PMD_READ_BONE_INFO * )Frame->UserData ;

		// アニメーションの追加
		Anim = MV1RAddAnim( RModel, AnimSet ) ;
		if( Anim == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		// ボーンに情報セット
		BoneInfo->Anim = Anim ;
		BoneInfo->Node = VmdNode ;

		// 対象ノードのセット
		Anim->TargetFrameName = MV1RAddString( RModel, Frame->Name ) ;
		Anim->TargetFrameIndex = Frame->Index ;

		// 最大時間をセット
		if( BoneInfo->IsIK )
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
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーション座標キーセットの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		KeyRotSet = MV1RAddAnimKeySet( RModel, Anim ) ;
		if( KeyRotSet == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーション回転キーセットの追加に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		KeyPosSet->Type = MV1_ANIMKEY_TYPE_VECTOR ;
		KeyPosSet->DataType = MV1_ANIMKEY_DATATYPE_TRANSLATE ;
		KeyPosSet->TimeType = MV1_ANIMKEY_TIME_TYPE_KEY ;
		KeyPosSet->TotalTime = ( float )VmdNode->MaxFrame ;
		KeyPosSet->Num = BoneInfo->IsIK ? VmdData.MaxTime : VmdNode->MaxFrame - VmdNode->MinFrame ;
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
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		KeyPosSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * KeyPosSet->Num, &RModel->Mem ) ;
		if( KeyPosSet->KeyVector == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}

		KeyRotSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyRotSet->Num, &RModel->Mem ) ;
		if( KeyRotSet->KeyTime == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
			goto ENDLABEL ;
		}
		KeyRotSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( FLOAT4 ) * KeyRotSet->Num, &RModel->Mem ) ;
		if( KeyRotSet->KeyVector == NULL )
		{
			DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
			if( BoneInfo->IsIK )
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
			if( BoneInfo->IsIK && VmdKey->Frame > 0 )
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

			if( BoneInfo->IsIK && KeyRot - BoneInfo->KeyRot < KeyRotSet->Num )
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
	if( PmdBoneInfo != NULL )
	{
		// アニメーションの再生準備
		BoneInfo = PmdBoneInfo ;
		for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
		{
			BoneInfo->IsIKAnim = FALSE ;
			if( BoneInfo->Anim != NULL )
			{
				BoneInfo->NowKey = BoneInfo->Node->FirstKey ;
			}
			else
			if( BoneInfo->IsIK )
			{
				// IKボーンでアニメーションがない場合はキーを打つ準備をする
				if( BoneInfo->Anim == NULL )
				{
					BoneInfo->IsIKAnim = TRUE ;

					// アニメーションの追加
					BoneInfo->Anim = MV1RAddAnim( RModel, AnimSet ) ;
					if( BoneInfo->Anim == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションの追加に失敗しました\n" ) ) ) ;
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyRotSet = MV1RAddAnimKeySet( RModel, BoneInfo->Anim ) ;
					if( KeyRotSet == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyPosSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( VECTOR ) * KeyPosSet->Num, &RModel->Mem ) ;
					if( KeyPosSet->KeyVector == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}

					KeyRotSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyRotSet->Num, &RModel->Mem ) ;
					if( KeyRotSet->KeyTime == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyRotSet->KeyVector = ( VECTOR * )ADDMEMAREA( sizeof( FLOAT4 ) * KeyRotSet->Num, &RModel->Mem ) ;
					if( KeyRotSet->KeyVector == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーション行列キーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}
			}
		}

#ifndef DX_NON_BULLET_PHYSICS
		if( MLPhysicsInfo )
		{
			BoneInfo = PmdBoneInfo ;
			for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
			{
				if( BoneInfo->IsPhysics == 0 ) continue ;

				BoneInfo->KeyMatrix = ( MATRIX * )DXALLOC( sizeof( MATRIX ) * ( ( FPS60 ? VmdData.MaxTime * 2 : VmdData.MaxTime ) + 1 ) ) ;
				if( BoneInfo->KeyMatrix == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーション行列キーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
			BoneInfo = PmdBoneInfo ;
			for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
			{
				if( BoneInfo->Anim != NULL && BoneInfo->IsIK == FALSE )
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
					for( j = 0 ; j < PmdBoneNum ; j ++ )
					{
						if( PmdBoneInfo[ j ].IsIK == FALSE )
							MV1LoadModelToPMD_SetupOneBoneMatrixFormAnimKey( &PmdBoneInfo[ j ], TimeNo, LoopNo ) ;
					}

					// 行列の計算
					MV1LoadModelToPMD_SetupMatrix( PmdBoneInfo, PmdBoneNum, FALSE, TRUE ) ;
				}
				else
				{
					// すべてのフレームの現在のフレームでのパラメータを算出する
					for( j = 0 ; j < PmdBoneNum ; j ++ )
						MV1LoadModelToPMD_SetupOneBoneMatrixFormAnimKey( &PmdBoneInfo[ j ], TimeNo, LoopNo ) ;

					// IKの計算を行うのはモーションループの最初だけ
//					if( LoopNo == 0 )
//					{
//						// ひざが含まれるＩＫボーンのトランスレーションを回転値に変換する
//						MV1LoadModelToPMD_ConvertTransToRotate( PmdBoneInfo, PmdIKInfoFirst ) ;
//					}

					// 行列の計算
					MV1LoadModelToPMD_SetupMatrix( PmdBoneInfo, PmdBoneNum, FALSE, FALSE ) ;

					// IKの計算を行うのはモーションループの最初だけ
					if( LoopNo == 0 )
					{
						// ＩＫの計算
						if( PmdIKInfoFirst )
						{
							MV1LoadModelToPMD_SetupIK( PmdBoneInfo, PmdIKInfoFirst ) ;
						}
					}
				}

#ifndef DX_NON_BULLET_PHYSICS
				// 物理演算を行う
				if( MLPhysicsInfo )
				{
					OneFrameProcess_PMDPhysicsInfo( MLPhysicsInfo, TimeNo, LoopNo, FPS60 ) ;
				}
#endif

				if( LoopNo == 0 && ( FPS60 || ( FPS60 == false && TimeNo % 2 == 0 ) ) )
				{
					// ＩＫに関わっているボーン又はＩＫの影響しないＩＫボーンの子ボーンのキーを保存
					BoneInfo = PmdBoneInfo ;
					for( j = 0 ; j < PmdBoneNum ; j ++, BoneInfo ++ )
					{
						if( BoneInfo->IsIK )
						{
							BoneInfo->KeyPosTime[ TimeNo ] = ( float )TimeNo / 2.0f ;
							BoneInfo->KeyRotTime[ TimeNo ] = ( float )TimeNo / 2.0f ;
							BoneInfo->KeyPos[ TimeNo ] = BoneInfo->Translate ;
							BoneInfo->KeyRot[ TimeNo ] = BoneInfo->Rotate ;
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
								MV1LoadModelToVMD_InverseMatrix( ( ( PMD_READ_BONE_INFO * )BoneInfo->Frame->Parent->UserData )->LocalWorldMatrix, InvParentBoneLWM ) ;
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
			int PmdPhysicsNum ;

			// 物理適応のアニメーションデータをセットする
			PmdPhysicsNum = MLPhysicsInfo->PmdPhysicsNum ;
			for( i = 0 ; i < PmdPhysicsNum ; i ++ )
			{
				if( CheckDisablePhysicsAnim_PMDPhysicsInfo( MLPhysicsInfo, i ) ) continue ;

				BoneInfo = MLPhysicsInfo->PmdPhysicsInfoDim[ i ].Bone ;

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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					if( BoneInfo->Anim->AnimKeySetFirst->Num != ( int )( VmdData.MaxTime * 2 + 1 ) )
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
							DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 座標アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyPosSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyPosSet->Num, &RModel->Mem ) ;
						if( KeyPosSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyMatrixSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyMatrixSet->Num, &RModel->Mem ) ;
						if( KeyMatrixSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
					// ないか物理のアニメーションが付いている場合の処理

					// 新たにアニメーションを追加
					BoneInfo->Anim = MV1RAddAnim( RModel, AnimSet ) ;
					if( BoneInfo->Anim == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションの追加に失敗しました\n" ) ) ) ;
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyMatrixSet->KeyMatrix4x4C = ( MV1_ANIM_KEY_MATRIX4X4C * )ADDMEMAREA( sizeof( MV1_ANIM_KEY_MATRIX4X4C ) * KeyMatrixSet->Num, &RModel->Mem ) ;
					if( KeyMatrixSet->KeyMatrix4x4C == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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

			BoneInfo = PmdBoneInfo ;
			for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					if( BoneInfo->Anim->AnimKeySetFirst->Num != ( int )( VmdData.MaxTime * 2 + 1 ) )
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
							DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 座標アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyPosSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyPosSet->Num, &RModel->Mem ) ;
						if( KeyPosSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
							goto ENDLABEL ;
						}
						KeyMatrixSet->KeyTime = ( float * )ADDMEMAREA( sizeof( float ) * KeyMatrixSet->Num, &RModel->Mem ) ;
						if( KeyMatrixSet->KeyTime == NULL )
						{
							DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションの追加に失敗しました\n" ) ) ) ;
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキーセットの追加に失敗しました\n" ) ) ) ;
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
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
						goto ENDLABEL ;
					}
					KeyMatrixSet->KeyMatrix4x4C = ( MV1_ANIM_KEY_MATRIX4X4C * )ADDMEMAREA( sizeof( MV1_ANIM_KEY_MATRIX4X4C ) * KeyMatrixSet->Num, &RModel->Mem ) ;
					if( KeyMatrixSet->KeyMatrix4x4C == NULL )
					{
						DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : 行列アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
			BoneInfo = PmdBoneInfo ;
			for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
			{
#ifndef DX_NON_BULLET_PHYSICS
				if( MLPhysicsInfo && BoneInfo->IsPhysics == TRUE && CheckDisablePhysicsAnim_PMDPhysicsInfo( MLPhysicsInfo, BoneInfo->PhysicsIndex ) == false ) continue ;
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
				DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションの追加に失敗しました\n" ) ) ) ;
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
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーション表情キーセットの追加に失敗しました\n" ) ) ) ;
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
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキータイムを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
					goto ENDLABEL ;
				}
				KeyFactorSet->KeyLinear = ( float * )ADDMEMAREA( sizeof( float ) * KeyFactorSet->Num, &RModel->Mem ) ;
				if( KeyFactorSet->KeyLinear == NULL )
				{
					DXST_ERRORLOGFMT_ADD( ( _T( "PMD Load Error : アニメーションキーを格納するメモリ領域の確保に失敗しました\n" ) ) ) ;
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
		ReleasePhysicsObject_PMDPhysicsInfo( MLPhysicsInfo ) ;

		BoneInfo = PmdBoneInfo ;
		for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
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

	BoneInfo = PmdBoneInfo ;
	for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
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
		ReleasePhysicsObject_PMDPhysicsInfo( MLPhysicsInfo ) ;

		BoneInfo = PmdBoneInfo ;
		for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
		{
			if( BoneInfo->KeyMatrix )
			{
				DXFREE( BoneInfo->KeyMatrix ) ;
				BoneInfo->KeyMatrix = NULL ;
			}
		}
	}
#endif

	BoneInfo = PmdBoneInfo ;
	for( i = 0 ; i < PmdBoneNum ; i ++, BoneInfo ++ )
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

// ＶＭＤファイルを読み込む( -1:エラー  0以上:モデルハンドル )、現時点ではカメラ情報を読み込むだけ
extern int MV1LoadModelToVMD( const MV1_MODEL_LOAD_PARAM *LoadParam, int ASyncThread )
{
	MV1_MODEL_R RModel ;
	int NewHandle = -1 ;
	int ErrorFlag = 1 ;
	int Result ;

	// 読み込み用データの初期化
	MV1InitReadModel( &RModel ) ;
	RModel.MeshFaceRightHand = FALSE ;
	RModel.TranslateIsBackCulling = TRUE ;
	
	// モデル名とファイル名をセット
	RModel.FilePath = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->FilePath ) + 1 ) * sizeof( TCHAR ) ) ;
	RModel.Name     = ( TCHAR * )DXALLOC( ( lstrlen( LoadParam->Name     ) + 1 ) * sizeof( TCHAR ) ) ;
	lstrcpy( RModel.FilePath, LoadParam->FilePath ) ;
	lstrcpy( RModel.Name,     LoadParam->Name ) ;

	// 法泉の自動生成は使用しない
	RModel.AutoCreateNormal = FALSE ;

	// アニメデータの分解をしない
	RModel.AnimDataNotDecomposition = TRUE ;

	// ＶＭＤファイルの読み込み
	Result = _MV1LoadModelToVMD_PMD(
		0,
		&RModel,
		LoadParam->DataBuffer,
		LoadParam->DataSize,
		"Anim000",
		NULL,
		0,
		NULL,
#ifndef DX_NON_BULLET_PHYSICS
		NULL,
#endif
		false
	) ;
	if( Result != 0 )
		goto ENDLABEL ;

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

	// 読み込み用モデルを解放
	MV1TermReadModel( &RModel ) ; 

	// 返回句柄
	return NewHandle ;
}

}

#endif


