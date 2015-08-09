#include "../VertexShader.h"

// 頂点シェーダーの入力
struct VS_INPUT
{
	float4 Position        : POSITION0 ;		// 座標( トランスフォーム済み )

#ifdef USE_DIFFUSE
	float4 Diffuse         : COLOR0 ;			// ディフューズカラー
#endif // USE_DIFFUSE

#ifdef USE_SPECULAR
	float4 Specular        : COLOR1 ;			// スペキュラカラー
#endif // USE_SPECULAR

#ifdef USE_TEXCOORD0
	float2 TexCoords0      : TEXCOORD0 ;		// テクスチャ座標０
#endif // USE_TEXCOORD0

#ifdef USE_TEXCOORD1
	float2 TexCoords1      : TEXCOORD1 ;		// テクスチャ座標１
#endif // USE_TEXCOORD1
} ;

// 頂点シェーダーの出力
struct VS_OUTPUT
{
	float4 Position        : SV_POSITION ;
	float4 Diffuse         : COLOR0 ;
	float2 TexCoords0      : TEXCOORD0 ;
	float2 TexCoords1      : TEXCOORD1 ;
} ;

// 2D用
VS_OUTPUT VS2D_Normal( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 Position;
	

	Position = VSInput.Position ;
	Position.w = 1.0f / Position.w ;
	Position.x *= Position.w ;
	Position.y *= Position.w ;
	Position.z *= Position.w ;


	// 座標変換
	VSOutput.Position.x = dot( Position, g_Base.AntiViewportMatrix[ 0 ] ) ;
	VSOutput.Position.y = dot( Position, g_Base.AntiViewportMatrix[ 1 ] ) ;
	VSOutput.Position.z = dot( Position, g_Base.AntiViewportMatrix[ 2 ] ) ;
	VSOutput.Position.w = dot( Position, g_Base.AntiViewportMatrix[ 3 ] ) ;


	// パラメータセット
#ifdef USE_DIFFUSE
	VSOutput.Diffuse    = VSInput.Diffuse ;
#else  // USE_DIFFUSE
	VSOutput.Diffuse    = 1.0f ;
#endif // USE_DIFFUSE


#ifdef USE_TEXCOORD0
	VSOutput.TexCoords0 = VSInput.TexCoords0 ;
#else  // USE_TEXCOORD0
	VSOutput.TexCoords0 = 0.0f ;
#endif // USE_TEXCOORD0


#ifdef USE_TEXCOORD1
	VSOutput.TexCoords1 = VSInput.TexCoords1 ;
#else  // USE_TEXCOORD1
	VSOutput.TexCoords1 = 0.0f ;
#endif // USE_TEXCOORD1

	
	return VSOutput ;
}





// 単純転送用頂点シェーダーの入力
struct STRETCHRECT_VS_INPUT
{
	float2 Position        : POSITION0 ;		// 座標
	float2 TexCoords0      : TEXCOORD0 ;		// テクスチャ座標0
	float2 TexCoords1      : TEXCOORD1 ;		// テクスチャ座標1
} ;

// 単純転送用頂点シェーダーの出力
struct STRETCHRECT_VS_OUTPUT
{
	float4 Position        : SV_POSITION ;
	float2 TexCoords0      : TEXCOORD0 ;
	float2 TexCoords1      : TEXCOORD1 ;
} ;

// 単純転送用頂点シェーダー
STRETCHRECT_VS_OUTPUT StretchRect_VS( STRETCHRECT_VS_INPUT VSInput )
{
	STRETCHRECT_VS_OUTPUT VSOutput ;

	VSOutput.Position.x = VSInput.Position.x ;
	VSOutput.Position.y = VSInput.Position.y ;
	VSOutput.Position.z = 0.0f ;
	VSOutput.Position.w = 1.0f ;

	VSOutput.TexCoords0 = VSInput.TexCoords0 ;
	VSOutput.TexCoords1 = VSInput.TexCoords1 ;
	
	return VSOutput ;
}
