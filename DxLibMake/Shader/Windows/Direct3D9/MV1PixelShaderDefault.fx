﻿#include "DxPixelShaderFxCode.h"

// main関数
PS_OUTPUT main( PS_INPUT PSInput )
{
	PS_OUTPUT PSOutput ;
	float4 TextureDiffuseColor ;
	float4 TextureSpecularColor ;
	float4 ToonSpecularGradColor ;
	float4 ToonDiffuseGradColor ;
	float4 ToonSphereMapColor ;
	float4 ToonRGBtoVMaxRGBVolumeColor ;
	float3 Normal ;
	float4 TotalDiffuse ;
	float4 TotalSpecular ;
	float4 TotalLightDiffuse ;
	float3 TotalAmbient ;
	float TotalAngleGen ;
	float DiffuseAngleGen ;
	float4 TempF4 ;
	float3 TempF3 ;
	float Temp ;
	float3 ShadowRate ;
	float ShadowGen ;
	float ParamAlpha;

#if SHADOWMAP
	float2 DepthTexCoord ;
	float4 TextureDepth ;
//	float4 variance_md_p_pmax ;
#endif

	ParamAlpha = PSInput.Diffuse.a * cfFactorColor.a;

#if SUBTEXTUREMODE != 0
	float4 SubTextureColor;
#endif  // SUBTEXTUREMODE != 0

/*
#if SHADOWMAP_DRAW
	// シャドウマップへの描画 ************************************************************( 開始 )

		// Ｚ値を色として出力
		PSOutput.Color0.r = PSInput.ShadowMap1Pos_ShadowMap3PosX.z;
//		PSOutput.Color0.g = PSInput.ShadowMap1Pos_ShadowMap3PosX.z * PSInput.ShadowMap1Pos_ShadowMap3PosX.z + 0.25f * ( ddx( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) * ddx( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) + ddy( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) * ddy( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) ) ;
		PSOutput.Color0.g = 0.0f ;
		PSOutput.Color0.b = 0.0f ;

		// 透明にならないようにアルファは必ず１
		PSOutput.Color0.a = 1.0f ;

	// シャドウマップへの描画 ************************************************************( 終了 )
#else // SHADOWMAP_DRAW
*/
		TextureDiffuseColor = tex2D( DiffuseMapTexture, PSInput.TexCoords0_1.xy ) ;
	
	#ifdef USE_SPE
		#ifdef USE_SPETEX
		TextureSpecularColor = tex2D( SpecularMapTexture, PSInput.TexCoords0_1.xy ) ;
		#endif // USE_SPETEX
	#endif // USE_SPE

	#if SUBTEXTUREMODE != 0
		SubTextureColor = tex2D( SubTexture, PSInput.TexCoords0_1.zw ) ;
	#endif  // SUBTEXTUREMODE != 0

	#if SUBTEXTUREMODE == 1  // αブレンド( MV1_LAYERBLEND_TYPE_TRANSLUCENT + 1 )
		TextureDiffuseColor.rgb = lerp( TextureDiffuseColor.rgb, SubTextureColor.rgb, SubTextureColor.a ) ;
	#endif // SUBTEXTUREMODE == 1

	#if SUBTEXTUREMODE == 2  // 加算( MV1_LAYERBLEND_TYPE_ADDITIVE + 1 )
		TextureDiffuseColor.rgb += SubTextureColor.rgb;
	#endif // SUBTEXTUREMODE == 2

	#if SUBTEXTUREMODE == 3  // 乗算( MV1_LAYERBLEND_TYPE_MODULATE + 1 )
		TextureDiffuseColor.rgb *= SubTextureColor.rgb;
	#endif // SUBTEXTUREMODE == 3

	#if SUBTEXTUREMODE == 4  // 乗算×２( MV1_LAYERBLEND_TYPE_MODULATE2 + 1 )
		TextureDiffuseColor.rgb *= SubTextureColor.rgb * 2.0f;
	#endif // SUBTEXTUREMODE == 4

		// ライトを使わない場合はカラーをそのまま出力 *****************************************( 開始 )
		#ifndef LG_USE

			// シャドウマップへの描画 *********************************************************( 開始 )
			#if SHADOWMAP_DRAW
				#if TOON
					#if TOON == 1
						// 出力α = テクスチャα * ディフューズα * 大域α
						PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;

					#else	// TOON == 1
						#if TOON_SPHEREOP_MUL
							Normal = normalize( PSInput.Normal_Fog.xyz ) ;
						
							// スフィアマップの色を取得
							TempF3 = Normal * float3( 0.5f, -0.5f, 0.0f ) + cfZeroHalfOneTwo.yyy ;
							ToonSphereMapColor = tex2D( ToonSphereMapTexture, TempF3.xy ) ;

							PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha * ToonSphereMapColor.a ;

						#else // TOON_SPHEREOP_MUL

							PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;

						#endif // TOON_SPHEREOP_MUL

					#endif		// TOON == 1
				#else  // TOON
				
					// 出力α = テクスチャα * ディフューズα * 大域α
					PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;
					
				#endif // TOON
				
				// Ｚ値を色として出力
				PSOutput.Color0.r = PSInput.ShadowMap1Pos_ShadowMap3PosX.z;
//				PSOutput.Color0.g = PSInput.ShadowMap1Pos_ShadowMap3PosX.z * PSInput.ShadowMap1Pos_ShadowMap3PosX.z + 0.25f * ( ddx( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) * ddx( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) + ddy( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) * ddy( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) ) ;
				PSOutput.Color0.g = 0.0f ;
				PSOutput.Color0.b = 0.0f ;
			
			// シャドウマップへの描画 *********************************************************( 終了 )
			#else // SHADOWMAP_DRAW
			
				#ifdef USE_SPE

					#ifdef USE_SPETEX

						PSOutput.Color0 = PSInput.Specular * TextureSpecularColor + PSInput.Diffuse * TextureDiffuseColor ;

					#else  // USE_SPETEX

						PSOutput.Color0 = PSInput.Specular + PSInput.Diffuse * TextureDiffuseColor ;

					#endif // USE_SPETEX

				#else  // USE_SPE

					PSOutput.Color0 = PSInput.Diffuse * TextureDiffuseColor ;

				#endif // USE_SPE

				PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;
				
			#endif // SHADOWMAP_DRAW

		// ライトを使わない場合はカラーをそのまま出力 *****************************************( 終了 )
		#else // LG_USE
		// ライトを使う場合 *******************************************************************( 開始 )

			// バンプマップ or フォンシェーディングの場合 =========================================( 開始 )
			#if BUMPMAP || PHONG

				#if BUMPMAP

					// 法線の 0～1 の値を -1.0～1.0 に変換する
					Normal = ( tex2D( NormalMapTexture, PSInput.TexCoords0_1.xy ).rgb - cfZeroHalfOneTwo.y ) * cfZeroHalfOneTwo.w ;

				#else // BUMPMAP

					Normal = PSInput.Normal_Fog.xyz ;

				#endif // BUMPMAP


				#ifdef    PHONG
					Normal = normalize( Normal ) ;
				#endif

				#if	SHADOWMAP
					// 頂点のテクスチャ座標値が範囲内の場合のみ処理する
					if( PSInput.ShadowMap1Pos_ShadowMap3PosX.x < -1.0f || PSInput.ShadowMap1Pos_ShadowMap3PosX.x > 1.0f ||
						PSInput.ShadowMap1Pos_ShadowMap3PosX.y < -1.0f || PSInput.ShadowMap1Pos_ShadowMap3PosX.y > 1.0f ||
						PSInput.ShadowMap1Pos_ShadowMap3PosX.z <  0.0f || PSInput.ShadowMap1Pos_ShadowMap3PosX.z > 1.0f )
					{
						ShadowRate.x = 1.0f;
					}
					else
					{
						// 深度テクスチャの座標を算出
						// PSInput.ShadowMap1Pos_ShadowMap3PosX.xy は -1.0f ～ 1.0f の値なので、これを 0.0f ～ 1.0f の値にする
						DepthTexCoord.x = ( PSInput.ShadowMap1Pos_ShadowMap3PosX.x + 1.0f ) / 2.0f;

						// yは更に上下反転
						DepthTexCoord.y = 1.0f - ( PSInput.ShadowMap1Pos_ShadowMap3PosX.y + 1.0f ) / 2.0f;

						// 深度バッファテクスチャから深度を取得
						TextureDepth = tex2D( ShadowMap1Texture, DepthTexCoord );

						// テクスチャに記録されている深度( +補正値 )よりＺ値が大きかったら奥にあるということで減衰率を最大にする
						ShadowRate.x = smoothstep( PSInput.ShadowMap1Pos_ShadowMap3PosX.z - cfShadowMap1_DAdj_Grad_Enbl0_1.y, PSInput.ShadowMap1Pos_ShadowMap3PosX.z, TextureDepth.r + cfShadowMap1_DAdj_Grad_Enbl0_1.x ) ;
					}

					// 頂点のテクスチャ座標値が範囲内の場合のみ処理する
					if( PSInput.ShadowMap2Pos_ShadowMap3PosY.x < -1.0f || PSInput.ShadowMap2Pos_ShadowMap3PosY.x > 1.0f ||
						PSInput.ShadowMap2Pos_ShadowMap3PosY.y < -1.0f || PSInput.ShadowMap2Pos_ShadowMap3PosY.y > 1.0f ||
						PSInput.ShadowMap2Pos_ShadowMap3PosY.z <  0.0f || PSInput.ShadowMap2Pos_ShadowMap3PosY.z > 1.0f )
					{
						ShadowRate.y = 1.0f;
					}
					else
					{
						// 深度テクスチャの座標を算出
						// PSInput.ShadowMap2Pos_ShadowMap3PosY.xy は -1.0f ～ 1.0f の値なので、これを 0.0f ～ 1.0f の値にする
						DepthTexCoord.x = ( PSInput.ShadowMap2Pos_ShadowMap3PosY.x + 1.0f ) / 2.0f;

						// yは更に上下反転
						DepthTexCoord.y = 1.0f - ( PSInput.ShadowMap2Pos_ShadowMap3PosY.y + 1.0f ) / 2.0f;

						// 深度バッファテクスチャから深度を取得
						TextureDepth = tex2D( ShadowMap2Texture, DepthTexCoord );

						// テクスチャに記録されている深度( +補正値 )よりＺ値が大きかったら奥にあるということで減衰率を最大にする
						ShadowRate.y = smoothstep( PSInput.ShadowMap2Pos_ShadowMap3PosY.z - cfShadowMap1_Enb2_ShadowMap2_DAdj_Grad_Enbl0.z, PSInput.ShadowMap2Pos_ShadowMap3PosY.z, TextureDepth.r + cfShadowMap1_Enb2_ShadowMap2_DAdj_Grad_Enbl0.y ) ;
					}

					// 頂点のテクスチャ座標値が範囲内の場合のみ処理する
					if( PSInput.ShadowMap1Pos_ShadowMap3PosX.w < -1.0f || PSInput.ShadowMap1Pos_ShadowMap3PosX.w > 1.0f ||
						PSInput.ShadowMap2Pos_ShadowMap3PosY.w < -1.0f || PSInput.ShadowMap2Pos_ShadowMap3PosY.w > 1.0f ||
						PSInput.V_to_Eye_ShadowMap3PosZ.w      <  0.0f || PSInput.V_to_Eye_ShadowMap3PosZ.w      > 1.0f )
					{
						ShadowRate.z = 1.0f;
					}
					else
					{
						// 深度テクスチャの座標を算出
						// PSInput.ShadowMap1Pos_ShadowMap3PosX.w と PSInput.ShadowMap2Pos_ShadowMap3PosY.w は -1.0f ～ 1.0f の値なので、これを 0.0f ～ 1.0f の値にする
						DepthTexCoord.x = ( PSInput.ShadowMap1Pos_ShadowMap3PosX.w + 1.0f ) / 2.0f;

						// yは更に上下反転
						DepthTexCoord.y = 1.0f - ( PSInput.ShadowMap2Pos_ShadowMap3PosY.w + 1.0f ) / 2.0f;

						// 深度バッファテクスチャから深度を取得
						TextureDepth = tex2D( ShadowMap3Texture, DepthTexCoord );

						// テクスチャに記録されている深度( +補正値 )よりＺ値が大きかったら奥にあるということで減衰率を最大にする
						ShadowRate.z = smoothstep( PSInput.V_to_Eye_ShadowMap3PosZ.w - cfShadowMap2_Enbl1_2_ShadowMap3_DAdj_Grad.w, PSInput.V_to_Eye_ShadowMap3PosZ.w, TextureDepth.r + cfShadowMap2_Enbl1_2_ShadowMap3_DAdj_Grad.z ) ;
					}

/*
					variance_md_p_pmax.x = TextureDepth.g - ( TextureDepth.r * TextureDepth.r ) ;
					variance_md_p_pmax.y = TextureDepth.r - ( PSInput.ShadowMap1Pos_ShadowMap3PosX.z + 1.0f ) ;
					variance_md_p_pmax.z = variance_md_p_pmax.x / ( variance_md_p_pmax.x + variance_md_p_pmax.y * variance_md_p_pmax.y ) ;
					ShadowRate = max( saturate( variance_md_p_pmax.z ), ( PSInput.ShadowMap1Pos_ShadowMap3PosX.z + 1.0f ) <= TextureDepth.r + cfShadowMap_DAdj_Grad.x ) ;
*/
/*
					variance_md_p_pmax.x = TextureDepth.g - ( TextureDepth.r * TextureDepth.r ) ;
					variance_md_p_pmax.y = TextureDepth.r - PSInput.ShadowMap1Pos_ShadowMap3PosX.z ;
					variance_md_p_pmax.z = variance_md_p_pmax.x / ( variance_md_p_pmax.x + variance_md_p_pmax.y * variance_md_p_pmax.y ) ;
//					ShadowRate = max( saturate( variance_md_p_pmax.z ), PSInput.ShadowMap1Pos_ShadowMap3PosX.z <= TextureDepth.r + cfShadowMap_DAdj_Grad.x ) ;
					ShadowRate = saturate( variance_md_p_pmax.z ) ;
*/
/*
					variance_md_p_pmax.z = smoothstep( PSInput.ShadowMap1Pos_ShadowMap3PosX.z - 0.02f, PSInput.ShadowMap1Pos_ShadowMap3PosX.z, TextureDepth.r ) ;
					variance_md_p_pmax.x = max( TextureDepth.g - ( TextureDepth.r * TextureDepth.r ), 0.0f ) ;
					variance_md_p_pmax.y = PSInput.ShadowMap1Pos_ShadowMap3PosX.z - TextureDepth.r ;
					variance_md_p_pmax.y = variance_md_p_pmax.y * variance_md_p_pmax.y ;
					variance_md_p_pmax.y = max( variance_md_p_pmax.y, 0.001f ) ;
					variance_md_p_pmax.w = variance_md_p_pmax.x / ( variance_md_p_pmax.x + variance_md_p_pmax.y ) ;
					variance_md_p_pmax.w = clamp( variance_md_p_pmax.w, 0.0f, 1.0f ) ;
//					variance_md_p_pmax.w = clamp( ( variance_md_p_pmax.w - 0.2f ) / ( 1.0f - 0.2f ), 0.0f, 1.0f ) ;
//					ShadowRate = clamp( max( variance_md_p_pmax.z, variance_md_p_pmax.w ), 0.0f, 1.0f ) ;
					ShadowRate = clamp( variance_md_p_pmax.z, 0.0f, 1.0f ) ;
*/
/*
					if( saturate( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) < TextureDepth.r + cfShadowMap_DAdj_Grad.x )
			 		{
						ShadowRate += 1.0f / 5.0f ;
					}

					// 深度バッファテクスチャから深度を取得
					DepthTexCoord.x -= 1.0f / 4096.0f ;
					TextureDepth = tex2D( ShadowMapTexture, DepthTexCoord );
					if( saturate( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) < TextureDepth.r + cfShadowMap_DAdj_Grad.x )
			 		{
						ShadowRate += 1.0f / 5.0f ;
					}
					DepthTexCoord.x += 2.0f / 4096.0f ;
					TextureDepth = tex2D( ShadowMapTexture, DepthTexCoord );
					if( saturate( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) < TextureDepth.r + cfShadowMap_DAdj_Grad.x )
			 		{
						ShadowRate += 1.0f / 5.0f ;
					}
					DepthTexCoord.x -= 1.0f / 4096.0f ;
					DepthTexCoord.y -= 1.0f / 4096.0f ;
					TextureDepth = tex2D( ShadowMapTexture, DepthTexCoord );
					if( saturate( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) < TextureDepth.r + cfShadowMap_DAdj_Grad.x )
			 		{
						ShadowRate += 1.0f / 5.0f ;
					}
					DepthTexCoord.y += 2.0f / 4096.0f ;
					TextureDepth = tex2D( ShadowMapTexture, DepthTexCoord );
					if( saturate( PSInput.ShadowMap1Pos_ShadowMap3PosX.z ) < TextureDepth.r + cfShadowMap_DAdj_Grad.x )
			 		{
						ShadowRate += 1.0f / 5.0f ;
					}
*/
				#else // SHADOWMAP
					ShadowRate.x = 1.0f ;
					ShadowRate.y = 1.0f ;
					ShadowRate.z = 1.0f ;
				#endif // SHADOWMAP

				// ディフューズカラーとスペキュラカラーの蓄積値を初期化
				TotalDiffuse = cfZeroHalfOneTwo.x ;
				TotalSpecular = cfZeroHalfOneTwo.x ;
				#if TOON
					#if TOON == 1
						TotalLightDiffuse = cfZeroHalfOneTwo.x ;
					#else // TOON == 1
						TotalAmbient = cfZeroHalfOneTwo.x ;
						TotalAngleGen = cfZeroHalfOneTwo.x ;
					#endif
				#endif // TOON




				
// ライト0の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )
#if LG0_USE

	// ディフューズ色計算

	// DiffuseAngleGen = ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, PSInput.Light0_Dir_Gen.xyz ) ) ;
	
	// 影による減衰率算出
	ShadowGen = max( ShadowRate.x, SHADOWMAP1_ENABLE_LGT0 ) *
	            max( ShadowRate.y, SHADOWMAP2_ENABLE_LGT0 ) *
	            max( ShadowRate.z, SHADOWMAP3_ENABLE_LGT0 ) ;

	#if LG0_DIR

		#if TOON
			#if TOON == 1
				// トゥーンの場合はこの時点では頂点orマテリアルのディフューズカラーは使用しない
				// ディフューズカラー蓄積値 += ライトのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
				TempF4 = cfLight[ 0 ].Diffuse * DiffuseAngleGen * ShadowGen ;
				TotalLightDiffuse += TempF4 ;
				TotalDiffuse += TempF4 + cfLight[ 0 ].Ambient ;

			#else // TOON == 1
				// トゥーン２の場合はディフューズカラーとライトの角度減衰は別々に保存しておく
				// ディフューズカラー蓄積値 += ライトのディフューズカラー
				// ライトの角度減衰率の蓄積値 += ライトの角度減衰率
				// ライトのアンビエントカラーの蓄積値 += ライトのアンビエントカラー
				TotalDiffuse += cfLight[ 0 ].Diffuse ;

				TotalAmbient += cfLight[ 0 ].Ambient.rgb ;
				TotalAngleGen += DiffuseAngleGen * ShadowGen ;

			#endif // TOON == 1
		#else // TOON
			// ディフューズカラー蓄積値 += ライトのディフューズカラー * ディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
			TotalDiffuse += cfLight[ 0 ].Diffuse * PSInput.Diffuse * DiffuseAngleGen * ShadowGen + cfLight[ 0 ].Ambient ;

		#endif //TOON

	#else // LG0_DIR

		#if TOON
			#if TOON == 1
				// トゥーンの場合はこの時点では頂点orマテリアルのディフューズカラーは使用しない
				// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
				TempF4 = cfLight[ 0 ].Diffuse * DiffuseAngleGen * ShadowGen ;
				TotalLightDiffuse += TempF4 * PSInput.Light0_Dir_Gen.w ;
				TotalDiffuse += ( TempF4 + cfLight[ 0 ].Ambient ) * PSInput.Light0_Dir_Gen.w ;

			#else // TOON == 1
				// トゥーン２の場合はディフューズカラーとライトの角度減衰は別々に保存しておく
				// ディフューズカラー蓄積値 += ライトのディフューズカラー
				// ライトのアンビエントカラーの蓄積値 += ライトのアンビエントカラー
				// ライトの角度減衰率の蓄積値 += ライトの角度減衰率
				TotalDiffuse += cfLight[ 0 ].Diffuse ;

				TotalAmbient += cfLight[ 0 ].Ambient.rgb ;
				TotalAngleGen += DiffuseAngleGen * ShadowGen * PSInput.Light0_Dir_Gen.w ;

			#endif // TOON == 1
		#else // TOON
			// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * ディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
			TotalDiffuse += ( cfLight[ 0 ].Diffuse * PSInput.Diffuse * DiffuseAngleGen * ShadowGen + cfLight[ 0 ].Ambient ) * PSInput.Light0_Dir_Gen.w ;
			

		#endif // TOON

	#endif // LG0_DIR


	// スペキュラカラー計算
	#ifdef USE_SPE

		// ハーフベクトルの計算
		TempF3 = PSInput.V_to_Eye_ShadowMap3PosZ.xyz + PSInput.Light0_Dir_Gen.xyz ;
		#ifdef    PHONG
			TempF3 = normalize( TempF3 ) ;
		#else  // PHONG
			TempF3 *= 0.5f ;
		#endif // PHONG

		// Temp = pow( max( 0.0f, N * H ), cfMaterial.Power.x )
		Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), cfMaterial.Power.x ) ;

		#if LG0_DIR

			// スペキュラカラー蓄積値 += Temp * ライトのスペキュラカラー
			TotalSpecular += Temp * cfLight[ 0 ].Specular * ShadowGen ;

		#else // LG0_DIR

			// スペキュラカラー蓄積値 += Temp * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
			TotalSpecular += Temp * PSInput.Light0_Dir_Gen.w * cfLight[ 0 ].Specular * ShadowGen ;

		#endif // LG0_DIR

	#endif // USE_SPE

#endif // LG0_USE
// ライト0の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )









				
// ライト1の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )
#if LG1_USE

	// ディフューズ色計算

	// DiffuseAngleGen = ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, PSInput.Light1_Dir_Gen.xyz ) ) ;
	
	// 影による減衰率算出
	ShadowGen = max( ShadowRate.x, SHADOWMAP1_ENABLE_LGT1 ) *
	            max( ShadowRate.y, SHADOWMAP2_ENABLE_LGT1 ) *
	            max( ShadowRate.z, SHADOWMAP3_ENABLE_LGT1 ) ;

	#if LG1_DIR

		#if TOON
			#if TOON == 1
				// トゥーンの場合はこの時点では頂点orマテリアルのディフューズカラーは使用しない
				// ディフューズカラー蓄積値 += ライトのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
				TempF4 = cfLight[ 1 ].Diffuse * DiffuseAngleGen * ShadowGen ;
				TotalLightDiffuse += TempF4 ;
				TotalDiffuse += TempF4 + cfLight[ 1 ].Ambient ;

			#else // TOON == 1
				// トゥーン２の場合はディフューズカラーとライトの角度減衰は別々に保存しておく
				// ディフューズカラー蓄積値 += ライトのディフューズカラー
				// ライトの角度減衰率の蓄積値 += ライトの角度減衰率
				// ライトのアンビエントカラーの蓄積値 += ライトのアンビエントカラー
				TotalDiffuse += cfLight[ 1 ].Diffuse ;

				TotalAmbient += cfLight[ 1 ].Ambient.rgb ;
				TotalAngleGen += DiffuseAngleGen * ShadowGen ;

			#endif // TOON == 1
		#else // TOON
			// ディフューズカラー蓄積値 += ライトのディフューズカラー * ディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
			TotalDiffuse += cfLight[ 1 ].Diffuse * PSInput.Diffuse * DiffuseAngleGen * ShadowGen + cfLight[ 1 ].Ambient ;

		#endif //TOON

	#else // LG1_DIR

		#if TOON
			#if TOON == 1
				// トゥーンの場合はこの時点では頂点orマテリアルのディフューズカラーは使用しない
				// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
				TempF4 = cfLight[ 1 ].Diffuse * DiffuseAngleGen * ShadowGen ;
				TotalLightDiffuse += TempF4 * PSInput.Light1_Dir_Gen.w ;
				TotalDiffuse += ( TempF4 + cfLight[ 1 ].Ambient ) * PSInput.Light1_Dir_Gen.w ;

			#else // TOON == 1
				// トゥーン２の場合はディフューズカラーとライトの角度減衰は別々に保存しておく
				// ディフューズカラー蓄積値 += ライトのディフューズカラー
				// ライトのアンビエントカラーの蓄積値 += ライトのアンビエントカラー
				// ライトの角度減衰率の蓄積値 += ライトの角度減衰率
				TotalDiffuse += cfLight[ 1 ].Diffuse ;

				TotalAmbient += cfLight[ 1 ].Ambient.rgb ;
				TotalAngleGen += DiffuseAngleGen * ShadowGen * PSInput.Light1_Dir_Gen.w ;

			#endif // TOON == 1
		#else // TOON
			// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * ディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
			TotalDiffuse += ( cfLight[ 1 ].Diffuse * PSInput.Diffuse * DiffuseAngleGen * ShadowGen + cfLight[ 1 ].Ambient ) * PSInput.Light1_Dir_Gen.w ;
			

		#endif // TOON

	#endif // LG1_DIR


	// スペキュラカラー計算
	#ifdef USE_SPE

		// ハーフベクトルの計算
		TempF3 = PSInput.V_to_Eye_ShadowMap3PosZ.xyz + PSInput.Light1_Dir_Gen.xyz ;
		#ifdef    PHONG
			TempF3 = normalize( TempF3 ) ;
		#else  // PHONG
			TempF3 *= 0.5f ;
		#endif // PHONG

		// Temp = pow( max( 0.0f, N * H ), cfMaterial.Power.x )
		Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), cfMaterial.Power.x ) ;

		#if LG1_DIR

			// スペキュラカラー蓄積値 += Temp * ライトのスペキュラカラー
			TotalSpecular += Temp * cfLight[ 1 ].Specular * ShadowGen ;

		#else // LG1_DIR

			// スペキュラカラー蓄積値 += Temp * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
			TotalSpecular += Temp * PSInput.Light1_Dir_Gen.w * cfLight[ 1 ].Specular * ShadowGen ;

		#endif // LG1_DIR

	#endif // USE_SPE

#endif // LG1_USE
// ライト1の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )









				
// ライト2の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )
#if LG2_USE

	// ディフューズ色計算

	// DiffuseAngleGen = ディフューズ角度減衰率計算
	DiffuseAngleGen = saturate( dot( Normal, PSInput.Light2_Dir_Gen.xyz ) ) ;
	
	// 影による減衰率算出
	ShadowGen = max( ShadowRate.x, SHADOWMAP1_ENABLE_LGT2 ) *
	            max( ShadowRate.y, SHADOWMAP2_ENABLE_LGT2 ) *
	            max( ShadowRate.z, SHADOWMAP3_ENABLE_LGT2 ) ;

	#if LG2_DIR

		#if TOON
			#if TOON == 1
				// トゥーンの場合はこの時点では頂点orマテリアルのディフューズカラーは使用しない
				// ディフューズカラー蓄積値 += ライトのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
				TempF4 = cfLight[ 2 ].Diffuse * DiffuseAngleGen * ShadowGen ;
				TotalLightDiffuse += TempF4 ;
				TotalDiffuse += TempF4 + cfLight[ 2 ].Ambient ;

			#else // TOON == 1
				// トゥーン２の場合はディフューズカラーとライトの角度減衰は別々に保存しておく
				// ディフューズカラー蓄積値 += ライトのディフューズカラー
				// ライトの角度減衰率の蓄積値 += ライトの角度減衰率
				// ライトのアンビエントカラーの蓄積値 += ライトのアンビエントカラー
				TotalDiffuse += cfLight[ 2 ].Diffuse ;

				TotalAmbient += cfLight[ 2 ].Ambient.rgb ;
				TotalAngleGen += DiffuseAngleGen * ShadowGen ;

			#endif // TOON == 1
		#else // TOON
			// ディフューズカラー蓄積値 += ライトのディフューズカラー * ディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの 
			TotalDiffuse += cfLight[ 2 ].Diffuse * PSInput.Diffuse * DiffuseAngleGen * ShadowGen + cfLight[ 2 ].Ambient ;

		#endif //TOON

	#else // LG2_DIR

		#if TOON
			#if TOON == 1
				// トゥーンの場合はこの時点では頂点orマテリアルのディフューズカラーは使用しない
				// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
				TempF4 = cfLight[ 2 ].Diffuse * DiffuseAngleGen * ShadowGen ;
				TotalLightDiffuse += TempF4 * PSInput.Light2_Dir_Gen.w ;
				TotalDiffuse += ( TempF4 + cfLight[ 2 ].Ambient ) * PSInput.Light2_Dir_Gen.w ;

			#else // TOON == 1
				// トゥーン２の場合はディフューズカラーとライトの角度減衰は別々に保存しておく
				// ディフューズカラー蓄積値 += ライトのディフューズカラー
				// ライトのアンビエントカラーの蓄積値 += ライトのアンビエントカラー
				// ライトの角度減衰率の蓄積値 += ライトの角度減衰率
				TotalDiffuse += cfLight[ 2 ].Diffuse ;

				TotalAmbient += cfLight[ 2 ].Ambient.rgb ;
				TotalAngleGen += DiffuseAngleGen * ShadowGen * PSInput.Light2_Dir_Gen.w ;

			#endif // TOON == 1
		#else // TOON
			// ディフューズカラー蓄積値 += ( ライトのディフューズカラー * ディフューズカラー * ディフューズカラー角度減衰率 + ライトのアンビエントカラーとマテリアルのアンビエントカラーを乗算したもの  ) * 距離・スポットライトの角度減衰率
			TotalDiffuse += ( cfLight[ 2 ].Diffuse * PSInput.Diffuse * DiffuseAngleGen * ShadowGen + cfLight[ 2 ].Ambient ) * PSInput.Light2_Dir_Gen.w ;
			

		#endif // TOON

	#endif // LG2_DIR


	// スペキュラカラー計算
	#ifdef USE_SPE

		// ハーフベクトルの計算
		TempF3 = PSInput.V_to_Eye_ShadowMap3PosZ.xyz + PSInput.Light2_Dir_Gen.xyz ;
		#ifdef    PHONG
			TempF3 = normalize( TempF3 ) ;
		#else  // PHONG
			TempF3 *= 0.5f ;
		#endif // PHONG

		// Temp = pow( max( 0.0f, N * H ), cfMaterial.Power.x )
		Temp = pow( max( 0.0f, dot( Normal, TempF3 ) ), cfMaterial.Power.x ) ;

		#if LG2_DIR

			// スペキュラカラー蓄積値 += Temp * ライトのスペキュラカラー
			TotalSpecular += Temp * cfLight[ 2 ].Specular * ShadowGen ;

		#else // LG2_DIR

			// スペキュラカラー蓄積値 += Temp * 距離・スポットライトの角度減衰率 * ライトのスペキュラカラー
			TotalSpecular += Temp * PSInput.Light2_Dir_Gen.w * cfLight[ 2 ].Specular * ShadowGen ;

		#endif // LG2_DIR

	#endif // USE_SPE

#endif // LG2_USE
// ライト2の処理 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


















				// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 開始 )

				#if TOON != 2
					// TotalDiffuse = ライトディフューズカラー蓄積値 + ( マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの )
					TotalDiffuse += cfAmbient_Emissive ;
				#else // TOON != 2
					// アンビエントカラーの蓄積値 += マテリアルのアンビエントカラーとグローバルアンビエントカラーを乗算したものとマテリアルエミッシブカラーを加算したもの
					TotalAmbient += cfAmbient_Emissive.rgb ;
				#endif  // TOON != 2

				#ifdef USE_SPE
					#if TOON
						#if TOON == 1
							// トゥーン１の時の出力計算
							ToonSpecularGradColor = tex2D( ToonSpecularGradTexture, max( TotalSpecular.b, max( TotalSpecular.r, TotalSpecular.g ) ) ) ;
						
							#ifdef USE_SPETEX
								TextureSpecularColor.rgb *= ToonSpecularGradColor.rgb ;
							#else  // USE_SPETEX
								TextureSpecularColor = ToonSpecularGradColor ;
							#endif // USE_SPETEX
						
							TextureSpecularColor.rgb *= tex3D( ToonRGBtoVMaxRGBVolumeTexture, TotalSpecular.rgb ).rgb * PSInput.Specular.rgb ;

							TextureDiffuseColor.rgb *= tex3D( ToonRGBtoVMaxRGBVolumeTexture, TotalLightDiffuse.rgb ).rgb * PSInput.Diffuse.rgb ;

							ToonDiffuseGradColor = tex2D( ToonDiffuseGradTexture, max( TotalDiffuse.b, max( TotalDiffuse.r, TotalDiffuse.g ) ) ) ;

							#if TOON_DIFOP_MUL
								PSOutput.Color0 = TextureDiffuseColor * ToonDiffuseGradColor ;
							#endif // TOON_DIFOP_MUL
							#if TOON_DIFOP_TRANS
								PSOutput.Color0 = lerp( TextureDiffuseColor, ToonDiffuseGradColor, ToonDiffuseGradColor.a ) ;
							#endif // TOON_DIFOP_TRANS

							#if TOON_SPEOP_ADD
								PSOutput.Color0 = PSOutput.Color0 + TextureSpecularColor ;
							#endif // TOON_SPEOP_ADD
							#if TOON_SPEOP_TRANS
								PSOutput.Color0 = lerp( PSOutput.Color0, TextureSpecularColor, TextureSpecularColor.a ) ;
							#endif // TOON_SPEOP_TRANS

							#if TOON_DIFOP_MUL
								ParamAlpha *= ToonDiffuseGradColor.a ;
							#endif // TOON_DIFOP_MUL

							PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;
						
						#else  // TOON == 1
							// トゥーン２の時の出力計算
							ToonDiffuseGradColor = tex2D( ToonDiffuseGradTexture, TotalAngleGen ) ;

							// 出力 = saturate( saturate( アンビエントカラーの蓄積値 + マテリアルのディフューズカラー * ライトのディフューズカラーの蓄積値 ) * テクスチャグラデーションカラー + スペキュラカラー蓄積値 * マテリアルのスペキュラカラー ) * テクスチャカラー
							PSOutput.Color0.rgb = saturate( saturate( TotalDiffuse.rgb * PSInput.Diffuse.rgb + TotalAmbient ) * ToonDiffuseGradColor.rgb + PSInput.Specular.rgb * TotalSpecular.rgb ) * TextureDiffuseColor.rgb ;
						
							#if TOON_SPHEREOP_MUL || TOON_SPHEREOP_ADD
								// スフィアマップの色を取得
								TempF3 = Normal * float3( 0.5f, -0.5f, 0.0f ) + cfZeroHalfOneTwo.yyy ;
								ToonSphereMapColor = tex2D( ToonSphereMapTexture, TempF3.xy ) ;
							
								#if TOON_SPHEREOP_MUL
									// スフィアマップの色を出力に乗算
									PSOutput.Color0.rgb *= ToonSphereMapColor.rgb ;
								#endif // TOON_SPHEREOP_MUL
							
								#if TOON_SPHEREOP_ADD
									// スフィアマップの色を出力に加算
									PSOutput.Color0.rgb += ToonSphereMapColor.rgb ;
								#endif // TOON_SPHEREOP_ADD
							
							#endif // TOON_SPHEREOP_MUL || TOON_SPHEREOP_ADD

							#if TOON_SPHEREOP_MUL 

								// 出力アルファ = テクスチャアルファ * Factorアルファ * 頂点アルファ * スフィアマップアルファ
								PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha * ToonSphereMapColor.a ;

							#else // TOON_SPHEREOP_MUL

								// 出力アルファ = テクスチャアルファ * Factorアルファ * 頂点アルファ
								PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;

							#endif // TOON_SPHEREOP_MUL

						#endif   // TOON == 1
					#else  // TOON

						#ifdef USE_SPETEX
							// TextureSpecularColor = ライトのスペキュラカラー蓄積値 * スペキュラカラー * スペキュラテクスチャカラー
							TextureSpecularColor = tex2D( SpecularMapTexture, PSInput.TexCoords0_1.xy ) * TotalSpecular * PSInput.Specular ;
						#else  // USE_SPETEX
							// TextureSpecularColor = ライトのスペキュラカラー蓄積値 * スペキュラカラー
							TextureSpecularColor = TotalSpecular * PSInput.Specular ;
						#endif // USE_SPETEX

						// 出力カラー = TotalDiffuse * テクスチャカラー + TextureSpecularColor

						PSOutput.Color0.rgb = TextureDiffuseColor.rgb * TotalDiffuse.rgb + TextureSpecularColor.rgb ;
						PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;

					#endif // TOON

				#else  // USE_SPE

					#if TOON
						#if TOON == 1
							ToonDiffuseGradColor = tex2D( ToonDiffuseGradTexture, max( TotalDiffuse.b, max( TotalDiffuse.r, TotalDiffuse.g ) ) ) ;
							TextureDiffuseColor.rgb *= tex3D( ToonRGBtoVMaxRGBVolumeTexture, TotalLightDiffuse.rgb ).rgb * PSInput.Diffuse.rgb ;

							#if TOON_DIFOP_MUL
								// 出力カラー = テクスチャカラー * ディフューズカラー蓄積値 * グラデーションカラー
								PSOutput.Color0 = TextureDiffuseColor * ToonDiffuseGradColor ;
							#endif // TOON_DIFOP_MUL
							#if TOON_DIFOP_TRANS
								// 出力カラー = グラデーションカラー * グラデーションα + ( 1 - グラデーションα ) * ( テクスチャカラー * ディフューズカラー蓄積値 )
								PSOutput.Color0 = lerp( TextureDiffuseColor, ToonDiffuseGradColor, ToonDiffuseGradColor.a ) ;
							#endif // TOON_DIFOP_TRANS

							// 乗算の場合はα値もグラデーションαと乗算する
							#if TOON_DIFOP_MUL
								ParamAlpha *= ToonDiffuseGradColor.a ;
							#endif // TOON_DIFOP_MUL

							// 出力α = テクスチャα * ディフューズα * 大域α
							PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;

						#else	// TOON == 1
							// トゥーン２の時の出力計算
							ToonDiffuseGradColor = tex2D( ToonDiffuseGradTexture, TotalAngleGen ) ;

							// 出力 = ( アンビエントカラーの蓄積値 + マテリアルのディフューズカラー * ライトのディフューズカラーの蓄積値 ) * テクスチャグラデーションカラー * テクスチャカラー
							PSOutput.Color0.rgb = saturate( TotalDiffuse.rgb * PSInput.Diffuse.rgb + TotalAmbient.rgb ) * ToonDiffuseGradColor.rgb * TextureDiffuseColor.rgb ;

							#if TOON_SPHEREOP_MUL || TOON_SPHEREOP_ADD
								// スフィアマップの色を取得
								TempF3 = Normal * float3( 0.5f, -0.5f, 0.0f ) + cfZeroHalfOneTwo.yyy ;
								ToonSphereMapColor = tex2D( ToonSphereMapTexture, TempF3.xy ) ;
							
								#if TOON_SPHEREOP_MUL
									// スフィアマップの色を出力に乗算
									PSOutput.Color0.rgb *= ToonSphereMapColor.rgb ;
								#endif // TOON_SPHEREOP_MUL
							
								#if TOON_SPHEREOP_ADD
									// スフィアマップの色を出力に加算
									PSOutput.Color0.rgb += ToonSphereMapColor.rgb ;
								#endif // TOON_SPHEREOP_ADD
							
							#endif // TOON_SPHEREOP_MUL || TOON_SPHEREOP_ADD

							#if TOON_SPHEREOP_MUL

								PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha * ToonSphereMapColor.a ;

							#else // TOON_SPHEREOP_MUL

								PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;

							#endif // TOON_SPHEREOP_MUL

						#endif		// TOON == 1
					#else  // TOON
						// 出力カラー = TotalDiffuse * テクスチャカラー
						PSOutput.Color0.rgb = TotalDiffuse.rgb * TextureDiffuseColor.rgb ;

						// 出力α = テクスチャα * ディフューズα * 大域α
						PSOutput.Color0.a = TextureDiffuseColor.a * ParamAlpha ;
					#endif // TOON

				#endif // USE_SPE

				// 出力カラー計算 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++( 終了 )


			// バンプマップ or フォンシェーディングの場合 =========================================( 終了 )
			#else  // BUMPMAP || PHONG
				// バンプマップ or フォンシェーディングではない場合 ===================================( 開始 )

				#ifdef USE_SPE

					#ifdef USE_SPETEX

						PSOutput.Color0.rgb = TextureDiffuseColor.rgb * PSInput.Diffuse.rgb + TextureSpecularColor.rgb * PSInput.Specular.rgb ;

					#else  // USE_SPETEX

						PSOutput.Color0.rgb = TextureDiffuseColor.rgb * PSInput.Diffuse.rgb + PSInput.Specular.rgb ;

					#endif // USE_SPETEX

				#else  // USE_SPE

					PSOutput.Color0.rgb = TextureDiffuseColor.rgb * PSInput.Diffuse.rgb ;

				#endif // USE_SPE

				PSOutput.Color0.a   = TextureDiffuseColor.a   * ParamAlpha ;

			#endif // BUMPMAP || PHONG
			// バンプマップ or フォンシェーディングではない場合 ===================================( 終了 )

		#endif // LG_USE
		// ライトを使う場合 *******************************************************************( 終了 )

	#if SM_3
		// フォグ効果
		PSOutput.Color0.rgb = lerp( cfFogColor.rgb, PSOutput.Color0.rgb, saturate( PSInput.Normal_Fog.w ) );
	#endif // SM_3

	// アルファ乗算カラー
	if( cfMulAlphaColor.x > 0.5f )
	{
		PSOutput.Color0.rgb *= PSOutput.Color0.a;
	}

// #endif // SHADOWMAP_DRAW

	return PSOutput ;
}

