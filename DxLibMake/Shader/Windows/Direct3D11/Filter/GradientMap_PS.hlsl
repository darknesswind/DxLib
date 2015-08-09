#include "../PixelShader.h"

cbuffer cbNormal : register( b3 )
{
	float4 RGBToY ;
} ;

SamplerState g_SrcSampler           : register( s0 ) ;
Texture2D    g_SrcTexture           : register( t0 ) ;

SamplerState g_GradientMapSampler   : register( s1 ) ;
Texture2D    g_GradientMapTexture   : register( t1 ) ;

float4 GradientMap_PS(STRETCHRECT_VS_OUTPUT In) : SV_TARGET0
{
	float4 Color;

	Color = g_SrcTexture.Sample( g_SrcSampler, In.TexCoords0 ) ;
	return  g_GradientMapTexture.Sample( g_GradientMapSampler, float2( dot( Color.rgb, RGBToY.rgb ), 0 ) ) ;
}

float4 GradientMapRev_PS(STRETCHRECT_VS_OUTPUT In) : SV_TARGET0
{
	float4 Color;

	Color = g_SrcTexture.Sample( g_SrcSampler, In.TexCoords0 ) ;
	return g_GradientMapTexture.Sample( g_GradientMapSampler, float2( 1.0f - dot( Color.rgb, RGBToY.rgb ), 0 ) ) ;
}
