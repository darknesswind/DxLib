// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		ピクセルシェーダー定義コード
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// 共通部分

// ピクセルシェーダーの入力
struct PS_INPUT
{
	float4 Diffuse         : COLOR0 ;		// ディフューズカラー
#if USE_SPE
	float4 Specular        : COLOR1 ;		// スペキュラカラー
#endif
	float4 TexCoords0_1    : TEXCOORD0 ;	// テクスチャ座標とサブテクスチャ座標( x:テクスチャ座標x  y:テクスチャ座標y  z:サブテクスチャ座標x  w:サブテクスチャ座標y )
#if ( BUMPMAP || PHONG ) && USE_SPE
	float3 V_to_Eye        : TEXCOORD1 ;	// 頂点座標から視線へのベクトル( ビュー空間 )
#endif
#if ( PHONG == 1 && BUMPMAP == 0 ) || SM_3
	float4 Normal_Fog      : TEXCOORD2 ;	// x.y.z:法線( ビュー空間 )  w:SM3.0用のフォグパラメータ
#endif
#if LG0_USE
	float4 Light0_Dir_Gen  : TEXCOORD3 ;	// xyz:ライト０の逆ベクトル( ビュー空間 ) w:ライト０減衰情報
#endif
#if LG1_USE
	float4 Light1_Dir_Gen  : TEXCOORD4 ;	// xyz:ライト１の逆ベクトル( ビュー空間 ) w:ライト１減衰情報
#endif
#if LG2_USE
	float4 Light2_Dir_Gen  : TEXCOORD5 ;	// xyz:ライト２の逆ベクトル( ビュー空間 ) w:ライト２減衰情報
#endif

#if SHADOWMAP || SHADOWMAP_DRAW
	float3 ShadowMap1Pos   : TEXCOORD6 ;	// シャドウマップ１のライト座標( x, y, z )
#endif
#if SHADOWMAP
	float3 ShadowMap2Pos   : TEXCOORD7 ;	// シャドウマップ２のライト座標( x, y, z )
#endif
} ;

// ピクセルシェーダーの出力
struct PS_OUTPUT
{
	float4 Color0          : COLOR0 ;	// 色
} ;

// マテリアルパラメータ
struct MATERIAL
{
	float4 Diffuse ;      // ディフューズカラー
	float4 Specular ;     // スペキュラカラー
	float4 Power ;        // スペキュラの強さ
} ;

// ライトパラメータ
struct LIGHT
{
	float4 Diffuse ;      // ディフューズカラー
	float4 Specular ;     // スペキュラカラー
	float4 Ambient ;      // アンビエントカラー
} ;



// C++ 側で設定する定数の定義
sampler  DiffuseMapTexture              : register( s0 ) ;		// ディフューズマップテクスチャ

#if BUMPMAP
sampler  NormalMapTexture               : register( s1 ) ;		// 法線マップテクスチャ
#endif // BUMPMAP

#if USE_SPETEX
sampler  SpecularMapTexture             : register( s2 ) ;		// スペキュラマップテクスチャ
#endif // USE_SPETEX

#if TOON
sampler  ToonDiffuseGradTexture         : register( s3 ) ;		// トゥーンレンダリング用ディフューズカラーグラデーションテクスチャ
sampler  ToonSpecularGradTexture        : register( s4 ) ;		// トゥーンレンダリング用スペキュラカラーグラデーションテクスチャ
#if TOON_SPHEREOP_MUL || TOON_SPHEREOP_ADD
sampler  ToonSphereMapTexture           : register( s5 ) ;		// トゥーンレンダリング用スフィアマップテクスチャ
#endif // TOON_SPHEREOP_MUL || TOON_SPHEREOP_ADD
sampler  ToonRGBtoVMaxRGBVolumeTexture  : register( s6 ) ;		// トゥーンレンダリング用RGB輝度飽和処理用ボリュームテクスチャ
#endif // TOON

#if SUBTEXTUREMODE != 0
sampler  SubTexture                     : register( s7 ) ;		// サブテクスチャ
#endif // SUBTEXTUREMODE != 0

#if SHADOWMAP
sampler  ShadowMap1Texture              : register( s8 ) ;		// シャドウマップ１テクスチャ
sampler  ShadowMap2Texture              : register( s9 ) ;		// シャドウマップ２テクスチャ
#endif


float4   cfZeroHalfOneTwo               : register( c0 /* DX_PS_CONSTF_ZERO_HALF_ONE_TWO          */ ) ;		// x=0.0f, y=0.5f, z=1.0f, w=2.0f
float4   cfAmbient_Emissive             : register( c1 /* DX_PS_CONSTF_AMBIENT_EMISSIVE           */ ) ;		// エミッシブカラー + マテリアルアンビエントカラー * グローバルアンビエントカラー

MATERIAL cfMaterial                     : register( c2 /* DX_PS_CONSTF_MATERIAL_START             */ ) ;		// マテリアルパラメータ

float4   cfFactorColor                  : register( c5 /* DX_PS_CONSTF_FACTORCOLOR                */ ) ;		// アルファ値等
float4   cfToonOutLineColor             : register( c6 /* DX_PS_CONSTF_TOON_OUTLINE_COLOR         */ ) ;		// トゥーンレンダリングの輪郭線の色
float4   cfToonOutLineWidth             : register( c7 /* DX_PS_CONSTF_TOON_OUTLINE_WIDTH         */ ) ;		// トゥーンレンダリングの輪郭線の太さ
float4   cfFogColor                     : register( c8 /* DX_PS_CONSTF_FOG_COLOR                  */ ) ;		// フォグカラー

LIGHT    cfLight[ 4 ]                   : register( c9 /* DX_PS_CONSTF_LIGHT_START                */ ) ;		// ライトパラメータ

float4   cfShadowMap1_DAdj_Grad_Enbl0_1 : register( c18 /* DX_PS_CONSTF_SHADOW1_DADJ_GRAD_ENBL0_1 */ ) ;		// シャドウマップ１用の情報( x:閾値深度補正値  y:グラデーション範囲  zw:ライト０・１への適用情報)
float4   cfShadowMap1_Enable2_3_4_5     : register( c19 /* DX_PS_CONSTF_SHADOW1_ENABLE_2_3_4_5    */ ) ;		// シャドウマップ１用の情報( xyzw:ライト２～５への適用情報 )
float4   cfShadowMap2_DAdj_Grad_Enbl0_1 : register( c20 /* DX_PS_CONSTF_SHADOW2_DADJ_GRAD_ENBL0_1 */ ) ;		// シャドウマップ１用の情報( x:閾値深度補正値  y:グラデーション範囲  zw:ライト０・１への適用情報)
float4   cfShadowMap2_Enable2_3_4_5     : register( c21 /* DX_PS_CONSTF_SHADOW2_ENABLE_2_3_4_5    */ ) ;		// シャドウマップ１用の情報( xyzw:ライト２～５への適用情報 )
float4   cfMulAlphaColor                : register( c22 /* DX_PS_CONSTF_MUL_ALPHA_COLOR           */ ) ;		// カラーにアルファ値を乗算するかどうか( x( 0.0f:乗算しない  1.0f:乗算する ) yzw( 未使用 ) )

#define SHADOWMAP1_ENABLE_LGT0 cfShadowMap1_DAdj_Grad_Enbl0_1.z
#define SHADOWMAP1_ENABLE_LGT1 cfShadowMap1_DAdj_Grad_Enbl0_1.w
#define SHADOWMAP1_ENABLE_LGT2 cfShadowMap1_Enable2_3_4_5.x
#define SHADOWMAP1_ENABLE_LGT3 cfShadowMap1_Enable2_3_4_5.y
#define SHADOWMAP1_ENABLE_LGT4 cfShadowMap1_Enable2_3_4_5.z
#define SHADOWMAP1_ENABLE_LGT5 cfShadowMap1_Enable2_3_4_5.w

#define SHADOWMAP2_ENABLE_LGT0 cfShadowMap2_DAdj_Grad_Enbl0_1.z
#define SHADOWMAP2_ENABLE_LGT1 cfShadowMap2_DAdj_Grad_Enbl0_1.w
#define SHADOWMAP2_ENABLE_LGT2 cfShadowMap2_Enable2_3_4_5.x
#define SHADOWMAP2_ENABLE_LGT3 cfShadowMap2_Enable2_3_4_5.y
#define SHADOWMAP2_ENABLE_LGT4 cfShadowMap2_Enable2_3_4_5.z
#define SHADOWMAP2_ENABLE_LGT5 cfShadowMap2_Enable2_3_4_5.w



