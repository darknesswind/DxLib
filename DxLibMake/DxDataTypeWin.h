// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		Windows用データタイプ定義ヘッダファイル
// 
// 				Ver 3.14d
// 
// -------------------------------------------------------------------------------

#ifndef __DXDATATYPEWIN_H__
#define __DXDATATYPEWIN_H__

// インクルード ------------------------------------------------------------------
#include "DxCompileConfig.h"
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>

// ライブラリリンク定義--------------------------------------------------------

#ifndef __DX_MAKE
	#ifndef DX_LIB_NOT_DEFAULTPATH
		#ifndef DX_GCC_COMPILE
			#ifndef DX_SRC_COMPILE
				#ifdef _MSC_VER
					#if _MSC_VER >= 1900
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2015_x64_d.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2015_x64_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2015_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2015_x64_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2015_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2015_x64.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2015_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2015_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2015_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2015_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#else // _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2015_x86_d.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2015_x86_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2015_x86_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2015_x86_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2015_x86_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2015_x86.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2015_x86.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2015_x86.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2015_x86.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2015_x86.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#endif // _WIN64
					#elif _MSC_VER >= 1800
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2012_x64_d.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2012_x64_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2013_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2012_x64_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2013_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2012_x64.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2012_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2013_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2012_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2013_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#else // _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_d.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#endif // _WIN64
					#elif _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2012_x64_d.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2012_x64_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2012_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2012_x64_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2012_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2012_x64.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2012_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2012_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2012_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2012_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#else // _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2012_x86_d.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2012_x86_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2012_x86_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2012_x86_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2012_x86_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc_vs2012_x86.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_vs2012_x86.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_vs2012_x86.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_vs2012_x86.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_vs2012_x86.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#endif // _WIN64
					#else // _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_x64_d.lib"		)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_x64_d.lib"		)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_x64_d.lib"	)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_x64_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_x64_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc_x64.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_x64.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_x64.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#else // _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "DxDrawFunc_d.lib"		)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW_d.lib"		)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW_d.lib"	)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib_d.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib_d.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#else // _DEBUG
								#pragma comment( lib, "DxDrawFunc.lib"			)		//  描画部分の抜き出し
								#ifdef UNICODE
									#pragma comment( lib, "DxLibW.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLibW.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#else
									#pragma comment( lib, "DxLib.lib"			)		//  ＤＸライブラリ使用指定
									#pragma comment( lib, "DxUseCLib.lib"		)		//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
								#endif
							#endif // _DEBUG
						#endif // _WIN64
					#endif // // _MSC_VER >= 1700
				#else // _MSC_VER
					#pragma comment( lib, "DxDrawFunc.lib"		)			//  描画部分の抜き出し
					#ifdef UNICODE
						#pragma comment( lib, "DxLibW.lib"		)			//  ＤＸライブラリ使用指定
						#pragma comment( lib, "DxUseCLibW.lib"	)			//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
					#else
						#pragma comment( lib, "DxLib.lib"		)			//  ＤＸライブラリ使用指定
						#pragma comment( lib, "DxUseCLib.lib"	)			//  標準Ｃライブラリを使用する部分の lib ファイルの使用指定
					#endif
				#endif // _MSC_VER

				#ifdef _DEBUG
					#pragma comment( linker, "/NODEFAULTLIB:libcmt.lib" )
					#pragma comment( linker, "/NODEFAULTLIB:libc.lib" )
					#pragma comment( linker, "/NODEFAULTLIB:libcd.lib" )
		//			#pragma comment( linker, "/NODEFAULTLIB:msvcrt.lib" )
		//			#pragma comment( linker, "/NODEFAULTLIB:msvcrtd.lib" )
				#else
					#pragma comment( linker, "/NODEFAULTLIB:libcmtd.lib" )
					#pragma comment( linker, "/NODEFAULTLIB:libc.lib" )
					#pragma comment( linker, "/NODEFAULTLIB:libcd.lib" )
		//			#pragma comment( linker, "/NODEFAULTLIB:msvcrt.lib" )
		//			#pragma comment( linker, "/NODEFAULTLIB:msvcrtd.lib" )
				#endif
			#endif
			//#pragma comment( lib, "libcmt.lib"		)				//  C標準マルチスレッド対応ライブラリ
			#pragma comment( lib, "kernel32.lib"		)			//  Win32カーネルライブラリ
			//#pragma comment( lib, "comctl32.lib"		)			//　Win32APIライブラリ
			#pragma comment( lib, "user32.lib"		)				//  Win32APIライブラリ
			#pragma comment( lib, "gdi32.lib"		)				//  Win32APIライブラリ
			#pragma comment( lib, "advapi32.lib"		)			//  Win32APIライブラリ
			//#pragma comment( lib, "ole32.lib"		)				//  Win32APIライブラリ
			#pragma comment( lib, "shell32.lib"		)				//  マルチメディアライブラリ
			//#pragma comment( lib, "winmm.lib"		)				//  マルチメディアライブラリ
			#ifndef DX_NON_MOVIE
				//#pragma comment( lib, "Strmiids.lib" )			//　DirectShowライブラリ
			#endif
			#ifndef DX_NON_NETWORK
				//#pragma comment( lib, "wsock32.lib" )				//  WinSocketsライブラリ
			#endif
			#ifndef DX_NON_KEYEX
				//#pragma comment( lib, "imm32.lib" )					// ＩＭＥ操作用ライブラリ
			#endif
			#ifndef DX_NON_ACM
				//#pragma comment( lib, "msacm32.lib" )				// ＡＣＭ操作用ライブラリ 
			#endif
			#ifndef DX_NON_BULLET_PHYSICS
				#ifdef _MSC_VER
					#if _MSC_VER >= 1900
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libbulletcollision_vs2015_x64_d.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2015_x64_d.lib" )
								#pragma comment( lib, "libbulletmath_vs2015_x64_d.lib" )
							#else
								#pragma comment( lib, "libbulletcollision_vs2015_x64.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2015_x64.lib" )
								#pragma comment( lib, "libbulletmath_vs2015_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libbulletcollision_vs2015_x86_d.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2015_x86_d.lib" )
								#pragma comment( lib, "libbulletmath_vs2015_x86_d.lib" )
							#else
								#pragma comment( lib, "libbulletcollision_vs2015_x86.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2015_x86.lib" )
								#pragma comment( lib, "libbulletmath_vs2015_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1800
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libbulletcollision_vs2013_x64_d.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2013_x64_d.lib" )
								#pragma comment( lib, "libbulletmath_vs2013_x64_d.lib" )
							#else
								#pragma comment( lib, "libbulletcollision_vs2013_x64.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2013_x64.lib" )
								#pragma comment( lib, "libbulletmath_vs2013_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libbulletcollision_vs2013_x86_d.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2013_x86_d.lib" )
								#pragma comment( lib, "libbulletmath_vs2013_x86_d.lib" )
							#else
								#pragma comment( lib, "libbulletcollision_vs2013_x86.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2013_x86.lib" )
								#pragma comment( lib, "libbulletmath_vs2013_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libbulletcollision_vs2012_x64_d.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2012_x64_d.lib" )
								#pragma comment( lib, "libbulletmath_vs2012_x64_d.lib" )
							#else
								#pragma comment( lib, "libbulletcollision_vs2012_x64.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2012_x64.lib" )
								#pragma comment( lib, "libbulletmath_vs2012_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libbulletcollision_vs2012_x86_d.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2012_x86_d.lib" )
								#pragma comment( lib, "libbulletmath_vs2012_x86_d.lib" )
							#else
								#pragma comment( lib, "libbulletcollision_vs2012_x86.lib" )
								#pragma comment( lib, "libbulletdynamics_vs2012_x86.lib" )
								#pragma comment( lib, "libbulletmath_vs2012_x86.lib" )
							#endif
						#endif
					#else // _MSC_VER >= 1700
						#ifdef DX_USE_VC8_BULLET_PHYSICS_LIB
							#ifdef _WIN64
								#ifdef _DEBUG
									#pragma comment( lib, "libbulletcollision_vc8_x64_d.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc8_x64_d.lib" )
									#pragma comment( lib, "libbulletmath_vc8_x64_d.lib" )
								#else
									#pragma comment( lib, "libbulletcollision_vc8_x64.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc8_x64.lib" )
									#pragma comment( lib, "libbulletmath_vc8_x64.lib" )
								#endif
							#else
								#ifdef _DEBUG
									#pragma comment( lib, "libbulletcollision_vc8_d.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc8_d.lib" )
									#pragma comment( lib, "libbulletmath_vc8_d.lib" )
								#else
									#pragma comment( lib, "libbulletcollision_vc8.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc8.lib" )
									#pragma comment( lib, "libbulletmath_vc8.lib" )
								#endif
							#endif
						#else // DX_USE_VC8_BULLET_PHYSICS_LIB
							#ifdef _WIN64
								#ifdef _DEBUG
									#pragma comment( lib, "libbulletcollision_vc8_x64_d.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc8_x64_d.lib" )
									#pragma comment( lib, "libbulletmath_vc8_x64_d.lib" )
								#else
									#pragma comment( lib, "libbulletcollision_vc8_x64.lib" )	// Visual C++ 8.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc8_x64.lib" )
									#pragma comment( lib, "libbulletmath_vc8_x64.lib" )
								#endif
							#else
								#ifdef _DEBUG
									#pragma comment( lib, "libbulletcollision_vc6_d.lib" )	// Visual C++ 6.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc6_d.lib" )
									#pragma comment( lib, "libbulletmath_vc6_d.lib" )
								#else
									#pragma comment( lib, "libbulletcollision_vc6.lib" )	// Visual C++ 6.0 でコンパイルした Bullet Physics ライブラリ 
									#pragma comment( lib, "libbulletdynamics_vc6.lib" )
									#pragma comment( lib, "libbulletmath_vc6.lib" )
								#endif
							#endif
						#endif // DX_USE_VC8_BULLET_PHYSICS_LIB
					#endif // _MSC_VER >= 1700
				#else // _MSC_VER
					#pragma comment( lib, "libbulletcollision.lib" )	// Bullet Physics ライブラリ 
					#pragma comment( lib, "libbulletdynamics.lib" )
					#pragma comment( lib, "libbulletmath.lib" )
				#endif // _MSC_VER
			#endif
			#ifndef DX_NON_TIFFREAD
				#ifdef _MSC_VER
					#if _MSC_VER >= 1900
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_vs2015_x64_d.lib" )	// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff_vs2015_x64.lib" )	// ＴＩＦＦライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_vs2015_x86_d.lib" )		// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff_vs2015_x86.lib" )		// ＴＩＦＦライブラリ
							#endif
						#endif
					#elif _MSC_VER >= 1800
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_vs2013_x64_d.lib" )		// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff_vs2013_x64.lib" )		// ＴＩＦＦライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_vs2013_x86_d.lib" )		// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff_vs2013_x86.lib" )		// ＴＩＦＦライブラリ
							#endif
						#endif
					#elif _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_vs2012_x64_d.lib" )	// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff_vs2012_x64.lib" )	// ＴＩＦＦライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_vs2012_x86_d.lib" )	// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff_vs2012_x86.lib" )	// ＴＩＦＦライブラリ
							#endif
						#endif
					#else // _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_x64_d.lib" )		// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff_x64.lib" )		// ＴＩＦＦライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libtiff_d.lib" )			// ＴＩＦＦライブラリ
							#else
								#pragma comment( lib, "libtiff.lib" )			// ＴＩＦＦライブラリ
							#endif
						#endif
					#endif // _MSC_VER >= 1700
				#else // _MSC_VER
					#pragma comment( lib, "libtiff.lib" )			// ＴＩＦＦライブラリ
				#endif // _MSC_VER
			#endif
			#ifndef DX_NON_PNGREAD
				#ifdef _MSC_VER
					#if _MSC_VER >= 1900
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_vs2015_x64_d.lib" )	// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2015_x64_d.lib" )
							#else
								#pragma comment( lib, "libpng_vs2015_x64.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2015_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_vs2015_x86_d.lib" )	// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2015_x86_d.lib" )
							#else
								#pragma comment( lib, "libpng_vs2015_x86.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2015_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1800
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_vs2013_x64_d.lib" )	// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2013_x64_d.lib" )
							#else
								#pragma comment( lib, "libpng_vs2013_x64.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2013_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_vs2013_x86_d.lib" )	// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2013_x86_d.lib" )
							#else
								#pragma comment( lib, "libpng_vs2013_x86.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2013_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_vs2012_x64_d.lib" )	// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2012_x64_d.lib" )
							#else
								#pragma comment( lib, "libpng_vs2012_x64.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2012_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_vs2012_x86_d.lib" )	// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2012_x86_d.lib" )
							#else
								#pragma comment( lib, "libpng_vs2012_x86.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_vs2012_x86.lib" )
							#endif
						#endif
					#else // _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_x64_d.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_x64_d.lib" )
							#else
								#pragma comment( lib, "libpng_x64.lib" )		// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libpng_d.lib" )			// ＰＮＧライブラリ
								#pragma comment( lib, "zlib_d.lib" )
							#else
								#pragma comment( lib, "libpng.lib" )			// ＰＮＧライブラリ
								#pragma comment( lib, "zlib.lib" )
							#endif
						#endif
					#endif // _MSC_VER >= 1700
				#else // _MSC_VER
					#pragma comment( lib, "libpng.lib" )			// ＰＮＧライブラリ
					#pragma comment( lib, "zlib.lib" )
				#endif // _MSC_VER
			#endif
			#ifndef DX_NON_JPEGREAD
				#ifdef _MSC_VER
					#if _MSC_VER >= 1900
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_vs2015_x64_d.lib" )		// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg_vs2015_x64.lib" )		// ＪＰＥＧライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_vs2015_x86_d.lib" )		// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg_vs2015_x86.lib" )		// ＪＰＥＧライブラリ
							#endif
						#endif
					#elif _MSC_VER >= 1800
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_vs2013_x64_d.lib" )		// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg_vs2013_x64.lib" )		// ＪＰＥＧライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_vs2013_x86_d.lib" )		// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg_vs2013_x86.lib" )		// ＪＰＥＧライブラリ
							#endif
						#endif
					#elif _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_vs2012_x64_d.lib" )		// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg_vs2012_x64.lib" )		// ＪＰＥＧライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_vs2012_x86_d.lib" )		// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg_vs2012_x86.lib" )		// ＪＰＥＧライブラリ
							#endif
						#endif
					#else // _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_x64_d.lib" )		// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg_x64.lib" )		// ＪＰＥＧライブラリ
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "libjpeg_d.lib" )			// ＪＰＥＧライブラリ
							#else
								#pragma comment( lib, "libjpeg.lib" )			// ＪＰＥＧライブラリ
							#endif
						#endif
					#endif // _MSC_VER >= 1700
				#else // _MSC_VER
					#pragma comment( lib, "libjpeg.lib" )			// ＪＰＥＧライブラリ
				#endif // _MSC_VER
			#endif
			#ifndef DX_NON_OGGVORBIS								// ＯｇｇＶｏｒｂｉｓライブラリ
				#ifdef _MSC_VER
					#if _MSC_VER >= 1900
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2015_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2015_x64.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2015_x86_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x86_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x86_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2015_x86.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x86.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1800
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2013_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2013_x64.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2013_x86_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x86_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x86_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2013_x86.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x86.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2012_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2012_x64.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2012_x86_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x86_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x86_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2012_x86.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x86.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x86.lib" )
							#endif
						#endif
					#else // _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_x64.lib" )
								#pragma comment( lib, "vorbis_static_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_d.lib" )
								#pragma comment( lib, "vorbis_static_d.lib" )
								#pragma comment( lib, "vorbisfile_static_d.lib" )
							#else
								#pragma comment( lib, "ogg_static.lib" )
								#pragma comment( lib, "vorbis_static.lib" )
								#pragma comment( lib, "vorbisfile_static.lib" )
							#endif
						#endif
					#endif // _MSC_VER >= 1700
				#else // _MSC_VER
					#pragma comment( lib, "ogg_static.lib" )
					#pragma comment( lib, "vorbis_static.lib" )
					#pragma comment( lib, "vorbisfile_static.lib" )
				#endif // _MSC_VER
			#endif
			#ifndef DX_NON_OGGTHEORA								// ＯｇｇＴｈｅｏｒａライブラリ
				#ifdef _MSC_VER
					#if _MSC_VER >= 1900
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2015_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x64_d.lib" )

								#pragma comment( lib, "libtheora_static_vs2015_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2015_x64.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x64.lib" )

								#pragma comment( lib, "libtheora_static_vs2015_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2015_x86_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x86_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x86_d.lib" )

								#pragma comment( lib, "libtheora_static_vs2015_x86_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2015_x86.lib" )
								#pragma comment( lib, "vorbis_static_vs2015_x86.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2015_x86.lib" )

								#pragma comment( lib, "libtheora_static_vs2015_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1800
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2013_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x64_d.lib" )

								#pragma comment( lib, "libtheora_static_vs2013_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2013_x64.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x64.lib" )

								#pragma comment( lib, "libtheora_static_vs2013_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2013_x86_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x86_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x86_d.lib" )

								#pragma comment( lib, "libtheora_static_vs2013_x86_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2013_x86.lib" )
								#pragma comment( lib, "vorbis_static_vs2013_x86.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2013_x86.lib" )

								#pragma comment( lib, "libtheora_static_vs2013_x86.lib" )
							#endif
						#endif
					#elif _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2012_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x64_d.lib" )

								#pragma comment( lib, "libtheora_static_vs2012_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2012_x64.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x64.lib" )

								#pragma comment( lib, "libtheora_static_vs2012_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_vs2012_x86_d.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x86_d.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x86_d.lib" )

								#pragma comment( lib, "libtheora_static_vs2012_x86_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_vs2012_x86.lib" )
								#pragma comment( lib, "vorbis_static_vs2012_x86.lib" )
								#pragma comment( lib, "vorbisfile_static_vs2012_x86.lib" )

								#pragma comment( lib, "libtheora_static_vs2012_x86.lib" )
							#endif
						#endif
					#else // _MSC_VER >= 1700
						#ifdef _WIN64
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_x64_d.lib" )
								#pragma comment( lib, "vorbis_static_x64_d.lib" )
								#pragma comment( lib, "vorbisfile_static_x64_d.lib" )

								#pragma comment( lib, "libtheora_static_x64_d.lib" )
							#else
								#pragma comment( lib, "ogg_static_x64.lib" )
								#pragma comment( lib, "vorbis_static_x64.lib" )
								#pragma comment( lib, "vorbisfile_static_x64.lib" )

								#pragma comment( lib, "libtheora_static_x64.lib" )
							#endif
						#else
							#ifdef _DEBUG
								#pragma comment( lib, "ogg_static_d.lib" )
								#pragma comment( lib, "vorbis_static_d.lib" )
								#pragma comment( lib, "vorbisfile_static_d.lib" )

								#pragma comment( lib, "libtheora_static_d.lib" )
							#else
								#pragma comment( lib, "ogg_static.lib" )
								#pragma comment( lib, "vorbis_static.lib" )
								#pragma comment( lib, "vorbisfile_static.lib" )

								#pragma comment( lib, "libtheora_static.lib" )
							#endif
						#endif
					#endif // _MSC_VER >= 1700
				#else // _MSC_VER
					#pragma comment( lib, "ogg_static.lib" )
					#pragma comment( lib, "vorbis_static.lib" )
					#pragma comment( lib, "vorbisfile_static.lib" )

					#pragma comment( lib, "libtheora_static.lib" )
				#endif // _MSC_VER
			#endif
		#endif  // DX_GCC_COMPILE
	#endif	// DX_LIB_NOT_DEFAULTPATH
#endif // __DX_MAKE

#ifdef DX_USE_NAMESPACE

namespace DxLib
{

#endif // DX_USE_NAMESPACE

// マクロ定義 --------------------------------------------------------------------

// 環境依存キーワードなど
#ifndef FASTCALL
#define FASTCALL	__fastcall
#endif

// ＤｉｒｅｃｔＩｎｐｕｔのバージョン設定
#define DIRECTINPUT_VERSION 0x700

#ifndef DWORD_PTR
#ifdef _WIN64
#define DWORD_PTR	ULONGLONG
#else
#define DWORD_PTR	DWORD
#endif
#endif

#ifndef LONG_PTR
#ifdef _WIN64
#define LONG_PTR	__int64
#else
#define LONG_PTR	int
#endif
#endif

// 構造体定義 --------------------------------------------------------------------

// テーブル-----------------------------------------------------------------------

// 内部大域変数宣言 --------------------------------------------------------------

// 関数プロトタイプ宣言-----------------------------------------------------------

#ifdef DX_USE_NAMESPACE

}

#endif // DX_USE_NAMESPACE

#endif // __DXDATATYPEWIN_H__
