﻿#include "../PixelShader.h"

SamplerState g_SrcSampler       : register( s0 ) ;
Texture2D    g_SrcTexture       : register( t0 ) ;

SamplerState g_BlendSampler     : register( s1 ) ;
Texture2D    g_BlendTexture	    : register( t1 ) ;

struct VS_OUTPUT
{
	float4 Position    : SV_POSITION ;
	float4 Diffuse     : COLOR0 ;
	float2 TexCoords0  : TEXCOORD0 ;
	float2 TexCoords1  : TEXCOORD1 ;
};

// テクスチャなし
float4 ps_notex_normal( VS_OUTPUT In ) : SV_TARGET0
{
	float4 DestColor ;

#ifdef INVERSE_COLOR
	DestColor.rgb = 1.0f - In.Diffuse.rgb ;
	DestColor.a = In.Diffuse.a ;
#else
	DestColor = In.Diffuse ;
#endif

#ifdef PRE_MUL_ALPHA
	DestColor.rgb *= In.Diffuse.a ;
#endif

#ifdef MUL_X_4
	DestColor.rgb *= 4.0f ;
#endif
#ifdef MUL_BLEND
	DestColor = lerp( 1.0f, DestColor, DestColor.a ) ;
#endif

	AlphaTest( DestColor.a ) ;

	return DestColor ;
}


// ブレンドテクスチャ無し
float4 ps_normal( VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;

	TexColor = g_SrcTexture.Sample( g_SrcSampler, In.TexCoords0 ) ;

#ifdef IGNORE_COLOR
	TexColor.rgb = 1.0f;
#endif
#ifdef INVERSE_COLOR
	TexColor.rgb = 1.0f - TexColor.rgb;
#endif

	DestColor = TexColor * In.Diffuse ;

#ifdef IGNORE_TEXALPHA
	DestColor.a = In.Diffuse.a ;
#endif

#ifdef PRE_MUL_ALPHA
	DestColor.rgb *= In.Diffuse.a ;
#endif

#ifdef MUL_X_4
	DestColor.rgb *= 4.0f ;
#endif
#ifdef MUL_BLEND
	DestColor = lerp( 1.0f, DestColor, DestColor.a ) ;
#endif

	AlphaTest( DestColor.a ) ;

	return DestColor ;
}

//ブレンドテクスチャあり DX_BLENDGRAPHTYPE_NORMAL
float4 ps_blend_normal( VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;
	float4 BlendTexColor ;
#ifdef PRE_MUL_ALPHA
	float4 PremulTexColor;
	float4 PremulBlendTexColor ;
#endif

#ifdef PRE_MUL_ALPHA
	PremulBlendTexColor = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	PremulTexColor      = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;

	BlendTexColor       = PremulBlendTexColor / PremulBlendTexColor.a;
	BlendTexColor.a     = PremulBlendTexColor.a;
	TexColor            = PremulTexColor      / PremulTexColor.a;
	TexColor.a          = PremulTexColor.a;
#else
	BlendTexColor       = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	TexColor            = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;
#endif

#ifdef IGNORE_COLOR
	TexColor.rgb = 1.0f;
#endif
#ifdef INVERSE_COLOR
	TexColor.rgb = 1.0f - TexColor.rgb;
#endif

	DestColor = lerp( TexColor, BlendTexColor, g_Base.FactorColor.a ) * In.Diffuse ;

#ifdef IGNORE_TEXALPHA
	DestColor.a = In.Diffuse.a ;
#endif

#ifdef MUL_X_4
	DestColor.rgb *= 4.0f ;
#endif

#ifdef MUL_BLEND
	DestColor = lerp( 1.0f, DestColor, DestColor.a ) ;
#endif

#ifdef PRE_MUL_ALPHA
	DestColor.rgb *= DestColor.a ;
#endif

	AlphaTest( DestColor.a ) ;

	return DestColor ;
}

//ブレンドテクスチャあり DX_BLENDGRAPHTYPE_WIPE
float4 ps_blend_wipe( VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;
	float4 BlendTexColor ;
#ifdef PRE_MUL_ALPHA
	float4 PremulTexColor;
	float4 PremulBlendTexColor ;
#endif

#ifdef PRE_MUL_ALPHA
	PremulBlendTexColor = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	PremulTexColor      = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;

	BlendTexColor       = PremulBlendTexColor / PremulBlendTexColor.a ;
	BlendTexColor.a     = PremulBlendTexColor.a ;
	TexColor            = PremulTexColor      / PremulTexColor.a ;
	TexColor.a          = PremulTexColor.a ;
#else
	BlendTexColor       = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	TexColor            = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;
#endif

#ifdef IGNORE_COLOR
	TexColor.rgb = 1.0f;
#endif
#ifdef INVERSE_COLOR
	TexColor.rgb = 1.0f - TexColor.rgb;
#endif

	DestColor = TexColor * In.Diffuse ;

#ifdef IGNORE_TEXALPHA
	DestColor.a = In.Diffuse.a ;
#endif

	DestColor.a *= saturate( ( BlendTexColor.a + g_Base.FactorColor.a ) * g_Base.FactorColor.b ) ;

#ifdef MUL_X_4
	DestColor.rgb *= 4.0f ;
#endif

#ifdef MUL_BLEND
	DestColor = lerp( 1.0f, DestColor, DestColor.a ) ;
#endif

#ifdef PRE_MUL_ALPHA
	DestColor.rgb *= DestColor.a ;
#endif

	AlphaTest( DestColor.a ) ;

	return DestColor ;
}

//ブレンドテクスチャあり DX_BLENDGRAPHTYPE_ALPHA
float4 ps_blend_alpha( VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;
	float4 BlendTexColor ;
#ifdef PRE_MUL_ALPHA
	float4 PremulTexColor;
	float4 PremulBlendTexColor ;
#endif

#ifdef PRE_MUL_ALPHA
	PremulBlendTexColor = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	PremulTexColor      = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;

	BlendTexColor       = PremulBlendTexColor / PremulBlendTexColor.a;
	BlendTexColor.a     = PremulBlendTexColor.a;
	TexColor            = PremulTexColor      / PremulTexColor.a;
	TexColor.a          = PremulTexColor.a;
#else
	BlendTexColor       = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	TexColor            = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;
#endif
	
#ifdef IGNORE_COLOR
	TexColor.rgb = 1.0f;
#endif
#ifdef INVERSE_COLOR
	TexColor.rgb = 1.0f - TexColor.rgb;
#endif

	DestColor = TexColor * In.Diffuse ;

#ifdef IGNORE_TEXALPHA
	DestColor.a = In.Diffuse.a ;
#endif

	DestColor.a *= BlendTexColor.a ;

#ifdef MUL_X_4
	DestColor.rgb *= 4.0f ;
#endif

#ifdef MUL_BLEND
	DestColor = lerp( 1.0f, DestColor, DestColor.a ) ;
#endif

#ifdef PRE_MUL_ALPHA
	DestColor.rgb *= DestColor.a ;
#endif

	AlphaTest( DestColor.a ) ;

	return DestColor ;
}








//マスク処理用シェーダー
float4 MaskBlend_PS( STRETCHRECT_VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;
	float4 BlendTexColor ;

	BlendTexColor = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	TexColor      = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;
	
	if( BlendTexColor.r > 0.001f )
	{
		discard ;
	}
	
	DestColor = TexColor ;
//	DestColor.rgb = TexColor.rgb ;
//	DestColor.a   = 1.0f - BlendTexColor.r ;

	return DestColor ;
}

float4 MaskBlend_ReverseEffect_PS( STRETCHRECT_VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;
	float4 BlendTexColor ;

	BlendTexColor = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	TexColor      = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;
		
	if( BlendTexColor.r < 0.999f )
	{
		discard ;
	}
	
	DestColor = TexColor ;
//	DestColor.rgb = TexColor.rgb ;
//	DestColor.a   = BlendTexColor.r ;

	return DestColor ;
}

float4 MaskBlend_UseGraphHandle_PS( STRETCHRECT_VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;
	float4 BlendTexColor ;

	BlendTexColor = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	TexColor      = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;

//	if( BlendTexColor.a > 0.001f )
//	{
//		discard ;
//	}

//	DestColor = TexColor ;
	DestColor.rgb = TexColor.rgb ;
	DestColor.a   = 1.0f - BlendTexColor.a ;

	return DestColor ;
}

float4 MaskBlend_UseGraphHandle_ReverseEffect_PS( STRETCHRECT_VS_OUTPUT In ) : SV_TARGET0
{
	float4 TexColor ;
	float4 DestColor ;
	float4 BlendTexColor ;

	BlendTexColor = g_BlendTexture.Sample( g_BlendSampler, In.TexCoords1 ) ;
	TexColor      = g_SrcTexture.Sample(   g_SrcSampler,   In.TexCoords0 ) ;

//	if( BlendTexColor.a < 0.999f )
//	{
//		discard ;
//	}
		
//	DestColor = TexColor ;
	DestColor.rgb = TexColor.rgb ;
	DestColor.a   = BlendTexColor.a ;

	return DestColor ;
}

// 単純転送用ピクセルシェーダー
float4 StretchRect_PS( STRETCHRECT_VS_OUTPUT In ) : SV_TARGET0
{
	return g_SrcTexture.Sample( g_SrcSampler,  In.TexCoords0 ) ;
}








