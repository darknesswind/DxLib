#include "../VertexShader.h"

// ���_�V�F�[�_�[�̓���
struct VS_INPUT
{
	float4 Position        : POSITION0 ;		// ���W( �g�����X�t�H�[���ς� )

#ifdef USE_DIFFUSE
	float4 Diffuse         : COLOR0 ;			// �f�B�t���[�Y�J���[
#endif // USE_DIFFUSE

#ifdef USE_SPECULAR
	float4 Specular        : COLOR1 ;			// �X�y�L�����J���[
#endif // USE_SPECULAR

#ifdef USE_TEXCOORD0
	float2 TexCoords0      : TEXCOORD0 ;		// �e�N�X�`�����W�O
#endif // USE_TEXCOORD0

#ifdef USE_TEXCOORD1
	float2 TexCoords1      : TEXCOORD1 ;		// �e�N�X�`�����W�P
#endif // USE_TEXCOORD1
} ;

// ���_�V�F�[�_�[�̏o��
struct VS_OUTPUT
{
	float4 Position        : SV_POSITION ;
	float4 Diffuse         : COLOR0 ;
	float2 TexCoords0      : TEXCOORD0 ;
	float2 TexCoords1      : TEXCOORD1 ;
} ;

// 2D�p
VS_OUTPUT VS2D_Normal( VS_INPUT VSInput )
{
	VS_OUTPUT VSOutput ;
	float4 Position;
	

	Position = VSInput.Position ;
	Position.w = 1.0f / Position.w ;
	Position.x *= Position.w ;
	Position.y *= Position.w ;
	Position.z *= Position.w ;


	// ���W�ϊ�
	VSOutput.Position.x = dot( Position, g_Base.AntiViewportMatrix[ 0 ] ) ;
	VSOutput.Position.y = dot( Position, g_Base.AntiViewportMatrix[ 1 ] ) ;
	VSOutput.Position.z = dot( Position, g_Base.AntiViewportMatrix[ 2 ] ) ;
	VSOutput.Position.w = dot( Position, g_Base.AntiViewportMatrix[ 3 ] ) ;


	// �p�����[�^�Z�b�g
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





// �P���]���p���_�V�F�[�_�[�̓���
struct STRETCHRECT_VS_INPUT
{
	float2 Position        : POSITION0 ;		// ���W
	float2 TexCoords0      : TEXCOORD0 ;		// �e�N�X�`�����W0
	float2 TexCoords1      : TEXCOORD1 ;		// �e�N�X�`�����W1
} ;

// �P���]���p���_�V�F�[�_�[�̏o��
struct STRETCHRECT_VS_OUTPUT
{
	float4 Position        : SV_POSITION ;
	float2 TexCoords0      : TEXCOORD0 ;
	float2 TexCoords1      : TEXCOORD1 ;
} ;

// �P���]���p���_�V�F�[�_�[
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
