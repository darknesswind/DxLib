// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		頂点シェーダー定義コード
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// 共通部分

#include "../Main/DxVertexShader.h"

// 頂点シェーダーの入力
struct VS_INPUT
{
	float4 Position        : POSITION ;			// 座標( ローカル空間 )
	float3 Normal          : NORMAL0 ;			// 法線( ローカル空間 )
	float4 Diffuse         : COLOR0 ;			// ディフューズカラー
	float4 Specular        : COLOR1 ;			// スペキュラカラー
	float4 TexCoords0      : TEXCOORD0 ;		// テクスチャ座標
	float4 TexCoords1      : TEXCOORD1 ;		// サブテクスチャ座標

#ifdef BUMPMAP
	// バンプマップ
	float3 Tan             : TANGENT0 ;			// 接線( ローカル空間 )
	float3 Bin             : BINORMAL0 ;		// 従法線( ローカル空間 )
#endif // BUMPMAP

#ifdef SKINMESH
	// スキニングメッシュ
	int4   BlendIndices0   : BLENDINDICES0 ;	// ボーン処理用 Float型定数配列インデックス０
	float4 BlendWeight0    : BLENDWEIGHT0 ;		// ボーン処理用ウエイト値０

	#ifdef BONE8

		int4   BlendIndices1   : BLENDINDICES1 ;	// ボーン処理用 Float型定数配列インデックス１
		float4 BlendWeight1    : BLENDWEIGHT1 ;		// ボーン処理用ウエイト値１

	#endif // BONE8

#endif // SKINMESH
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 Position        : POSITION ;		// 座標( プロジェクション空間 )
	float4 Diffuse         : COLOR0 ;		// ディフューズカラー
	float4 Specular        : COLOR1 ;		// スペキュラカラー
	float4 TexCoords0_1    : TEXCOORD0 ;	// xy:テクスチャ座標 zw:サブテクスチャ座標
	float3 VPosition       : TEXCOORD1 ;	// 座標( ビュー空間 )
	float3 VNormal         : TEXCOORD2 ;	// 法線( ビュー空間 )
#ifdef BUMPMAP
	float3 VTan            : TEXCOORD3 ;    // 接線( ビュー空間 )
	float3 VBin            : TEXCOORD4 ;    // 従法線( ビュー空間 )
#endif // BUMPMAP
	float  Fog             : TEXCOORD5 ;	// フォグパラメータ

#if SHADOWMAP
	float3 ShadowMap1Pos   : TEXCOORD6 ;		// シャドウマップ１のライト座標( x, y, z )
	float3 ShadowMap2Pos   : TEXCOORD7 ;		// シャドウマップ２のライト座標( x, y, z )
#endif // SHADOWMAP
} ;

// 頂点シェーダーのマテリアルパラメータ
struct VS_CONST_MATERIAL
{
	float4 Diffuse ;				// ディフューズカラー
	float4 Specular ;				// スペキュラカラー
	float4 Power ;					// スペキュラの強さ
} ;




// C++ 側で設定する定数の定義
float4              cfZeroOne                                                  : register( c0  /* DX_VS_CONSTF_ZERO_ONE              */ ) ;	// ０と１の定数( x:0.0f  y:1.0f )
float4              cfProjectionMatrix[ 4 ]                                    : register( c2  /* DX_VS_CONSTF_PROJECTION_MAT        */ ) ;	// ビュー　→　プロジェクション行列
float4              cfViewMatrix[ 3 ]                                          : register( c6  /* DX_VS_CONSTF_VIEW_MAT              */ ) ;	// ワールド　→　ビュー行列
float4              cfFog                                                      : register( c10 /* DX_VS_CONSTF_FOG                   */ ) ;	// フォグ用パラメータ( x:end/(end - start)  y:-1/(end - start)  z:density  w:自然対数の低 )

VS_CONST_MATERIAL   cfMaterial                                                 : register( c11 /* DX_VS_CONSTF_MATERIAL_START        */ ) ;	// マテリアルパラメータ
float4              cfToonOutLineSize                                          : register( c42 /* DX_VS_CONSTF_TOON_OUTLINE_SIZE     */ ) ;	// トゥーンの輪郭線の大きさ
float4              cfDifSpeSource                                             : register( c43 /* DX_VS_CONSTF_DIF_SPE_SOURCE        */ ) ;	// x:ディフューズカラー( 0.0f:マテリアル  1.0f:頂点 )  y:スペキュラカラー( 0.0f:マテリアル  1.0f:頂点 )

float4              cfShadowMap1LightViewMatrix[ 3 ]                           : register( c44 /* DX_VS_CONSTF_SHADOWMAP1_LIGHT_VIEW_MAT       */ ) ;	// シャドウマップ用のライトビュー行列
float4              cfShadowMap1LightProjectionMatrix[ 4 ]                     : register( c47 /* DX_VS_CONSTF_SHADOWMAP1_LIGHT_PROJECTION_MAT */ ) ;	// シャドウマップ用のライト射影行列
float4              cfShadowMap2LightViewMatrix[ 3 ]                           : register( c51 /* DX_VS_CONSTF_SHADOWMAP2_LIGHT_VIEW_MAT       */ ) ;	// シャドウマップ用のライトビュー行列
float4              cfShadowMap2LightProjectionMatrix[ 4 ]                     : register( c54 /* DX_VS_CONSTF_SHADOWMAP2_LIGHT_PROJECTION_MAT */ ) ;	// シャドウマップ用のライト射影行列

float4              cfTextureMatrix[ DX_VS_CONSTF_TEXTURE_MATRIX_NUM ][ 2 ]    : register( c88 /* DX_VS_CONSTF_TEXTURE_MATRIX_START  */ ) ;	// テクスチャ座標操作用行列
float4              cfLocalWorldMatrix[ DX_VS_CONSTF_WORLD_MAT_NUM * 3 ]       : register( c94 /* DX_VS_CONSTF_WORLD_MAT_START       */ ) ;	// ローカル　→　ワールド行列




#define FOG_LINEAR_ADD          cfFog.x
#define FOG_LINEAR_DIV          cfFog.y
#define FOG_EXP_DENSITY         cfFog.z
#define FOG_EXP_E               cfFog.w

#ifdef SKINMESH
#define LOCAL_WORLD_MAT         lLocalWorldMatrix
#else
#define LOCAL_WORLD_MAT         cfLocalWorldMatrix
#endif
/*
#ifdef DIF_MAT
#define DIF                     cfMaterial.Diffuse
#endif

#ifdef DIF_VTX
#define DIF                     VSInput.Diffuse
#endif

#ifdef SPE_MAT
#define SPE                     cfMaterial.Specular
#endif

#ifdef SPE_VTX
#define SPE                     VSInput.Specular
#endif
*/
#define TEXMTX0_U               cfTextureMatrix[ 0 ][ 0 ]
#define TEXMTX0_V               cfTextureMatrix[ 0 ][ 1 ]


