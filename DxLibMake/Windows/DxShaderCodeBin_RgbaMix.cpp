﻿// -------------------------------------------------------------------------------
// 
// 		ＤＸライブラリ		RGBAブレンド用シェーダーバイナリ
// 
// 				Ver 3.11f
// 
// -------------------------------------------------------------------------------

// ＤＸLibrary 生成时使用的定义
#define __DX_MAKE

#include "../DxCompileConfig.h"

#ifndef DX_NON_FILTER

// インクルード----------------------------------------------------------------
#include "../DxLib.h"
#include "../DxStatic.h"

// データ定義------------------------------------------------------------------

namespace DxLib
{

// シェーダーファイルを圧縮したもの
BYTE DxShaderCodeBin_RgbaMix[] = 
{
	0x14,0x5a,0x00,0x00,0xef,0x09,0x00,0x00,0xfe,0x48,0x00,0x54,0x00,0x54,0x00,0xfe,
	0x10,0x05,0x60,0x00,0x48,0x00,0x48,0x00,0x60,0xfe,0x08,0x0b,0xfe,0x00,0x0d,0xfe,
	0x00,0x09,0xfe,0x00,0x19,0xfe,0x10,0x07,0xfe,0x30,0x19,0x54,0x00,0x3c,0xfe,0x08,
	0x1d,0xfe,0x10,0x0f,0xfe,0x30,0x1d,0xfe,0x20,0x3f,0xfe,0x10,0x33,0x6c,0xfe,0x18,
	0x21,0xfe,0x30,0x05,0xfe,0x10,0x11,0xfe,0x30,0x37,0xfe,0x10,0x29,0xfe,0x10,0x41,
	0xfe,0x20,0x07,0xfe,0x20,0x21,0xfe,0x20,0x2f,0xfe,0x20,0x97,0xfe,0x10,0x89,0xfe,
	0x20,0x5d,0xfe,0x20,0x2b,0xfe,0x20,0x8f,0xfe,0x10,0x13,0xfe,0x30,0x4b,0xfe,0x10,
	0x8d,0xfe,0x30,0x4f,0xfe,0x30,0x2f,0xfe,0x40,0x83,0xfe,0x30,0xbf,0xfe,0x40,0x47,
	0xfe,0x10,0xcf,0xfe,0x50,0x3f,0xfe,0x20,0xb3,0xfe,0x11,0x03,0x01,0xfe,0x00,0x25,
	0xfe,0x20,0x47,0xfe,0x50,0x3f,0xfe,0x20,0x45,0xfe,0x10,0x3f,0xfe,0x10,0x5d,0xfe,
	0x30,0x71,0xfe,0x31,0x0d,0x01,0xfe,0x30,0x07,0xfe,0x60,0x11,0xfe,0xa1,0x37,0x01,
	0xfe,0x50,0x99,0xfe,0x20,0xcf,0xfe,0x10,0x15,0xfe,0x20,0x93,0xfe,0x51,0xa5,0x01,
	0xfe,0x60,0x5f,0xfe,0x31,0x5f,0x01,0xfe,0x31,0x2f,0x01,0x48,0x00,0x00,0x02,0xff,
	0xff,0x1f,0x00,0x00,0x02,0x00,0x00,0x00,0x80,0x00,0x00,0x03,0xb0,0xfe,0x18,0x0b,
	0x90,0x00,0x08,0x0f,0xa0,0x42,0x00,0x00,0x03,0x00,0x00,0x0f,0x80,0x00,0x00,0xe4,
	0xb0,0x00,0x08,0xe4,0xa0,0x01,0xfe,0x08,0x1b,0xfe,0x00,0x0f,0x00,0x80,0xfe,0x08,
	0x0b,0x08,0xfe,0x08,0x1b,0x80,0xff,0xff,0x00,0xfe,0x6c,0x01,0x47,0x01,0x00,0x07,
	0xfe,0x28,0x47,0x01,0x00,0x08,0x80,0x00,0x00,0x55,0xfe,0x28,0x53,0x01,0xfe,0x2c,
	0x02,0x53,0xaa,0xfe,0xec,0x01,0x53,0x00,0xfe,0x38,0x53,0xfe,0xd4,0x01,0xef,0x0b,
	0xfe,0x38,0x9b,0x04,0xfe,0x1c,0x02,0xef,0x0c,0xfe,0x28,0x47,0x01,0x00,0x03,0xfe,
	0x48,0xa7,0xfe,0xb4,0x01,0x53,0xfe,0x30,0x47,0xfe,0x41,0x43,0x01,0xfe,0x24,0x02,
	0xb3,0x00,0x00,0x0e,0x80,0x00,0x00,0xd2,0xfe,0xed,0x01,0x4f,0x01,0x00,0xfe,0x39,
	0x4f,0x01,0xfe,0xd5,0x01,0x97,0x01,0xfe,0x60,0xef,0xfe,0x21,0x4f,0x01,0xfe,0x00,
	0xef,0xfe,0xf5,0x01,0x3f,0x02,0xfe,0x11,0xa3,0x01,0xfe,0x21,0x43,0x01,0xfe,0x25,
	0x02,0xa3,0x01,0x0d,0xfe,0x08,0xef,0xfe,0x10,0x0b,0x02,0xfe,0x1c,0x02,0x53,0xfe,
	0x61,0x9f,0x02,0xff,0xfe,0x7d,0x02,0x4b,0x02,0xfe,0x41,0x5b,0x01,0xfe,0x84,0x02,
	0x5f,0xfe,0x41,0xab,0x02,0xfe,0x04,0x02,0x5f,0xfe,0x01,0xbb,0x01,0xfe,0x10,0x47,
	0xfe,0x25,0x02,0xbb,0x01,0x00,0xfe,0x09,0xbb,0x01,0xfe,0x05,0x02,0xb7,0x02,0x0a,
	0xfe,0x39,0x4f,0x01,0x05,0xfe,0x3d,0x02,0xfb,0x03,0xfe,0x21,0x57,0x02,0xfe,0x61,
	0x4f,0x01,0xfe,0x25,0x02,0x63,0x02,0xfe,0x20,0x5f,0xfe,0x14,0x02,0x53,0x00,0xfe,
	0x09,0xa3,0x01,0x1b,0xfe,0xfd,0x01,0x4f,0x04,0xfe,0x01,0x97,0x04,0xfe,0x31,0xdf,
	0x04,0xfe,0xd5,0x01,0x4f,0x04,0xfe,0xa1,0x43,0x01,0xfe,0x05,0x02,0xf3,0x05,0xfe,
	0x61,0x43,0x01,0xfe,0x25,0x02,0x37,0x07,0xfe,0x40,0xb3,0xfe,0x05,0x02,0xeb,0x01,
	0xfe,0x60,0x53,0xfe,0x84,0x02,0xa7,0xfe,0xfd,0x01,0xdf,0x07,0xfe,0x4d,0x02,0xeb,
	0x01,0xfe,0x85,0x02,0x3f,0x05,0xfe,0x71,0x43,0x01,0xfe,0xb4,0x02,0x5f,0xfe,0x65,
	0x02,0x3f,0x05,0xfe,0x61,0xab,0x02,0xfe,0x04,0x02,0x53,0xfe,0x31,0x3f,0x05,0xfe,
	0x11,0xfb,0x06,0xfe,0x05,0x02,0xa3,0x07,0xfe,0x31,0x3f,0x05,0xfe,0x51,0x67,0x01,
	0xfe,0x24,0x02,0x5f,0xfe,0x01,0xab,0x05,0xfe,0x21,0x5b,0x01,0xfe,0x15,0x02,0xab,
	0x05,0xfe,0x11,0xaf,0x04,0xc9,0xfe,0x1d,0x02,0xf7,0x04,0xfe,0xf5,0x01,0x3f,0x05,
	0x01,0x00,0x06,0xfe,0x28,0x47,0x01,0x00,0x01,0xfe,0x7d,0x02,0xdb,0x0b,0xfe,0x40,
	0x5f,0xfe,0x41,0xab,0x02,0xfe,0x30,0x6b,0xfe,0xd5,0x01,0x4f,0x01,0xfe,0x30,0x5f,
	0xfe,0xd5,0x01,0x4f,0x01,0xfe,0x01,0xeb,0x01,0xfe,0x05,0x02,0x4f,0x01,0xfe,0x01,
	0x27,0x02,0xfe,0x81,0xfb,0x03,0xfe,0x15,0x02,0x93,0x02,0xfe,0x11,0xf7,0x01,0xfe,
	0x30,0x47,0xfe,0xb4,0x01,0xa7,0xfe,0x61,0x8b,0x0a,0xfe,0x05,0x02,0x2f,0x03,0x09,
	0xfe,0x89,0x4b,0x05,0xfe,0x34,0x02,0x5f,0xfe,0x41,0x9f,0x02,0xfe,0x30,0x53,0xfe,
	0x50,0x6b,0xfe,0x25,0x02,0xb7,0x02,0xfe,0x41,0xfb,0x03,0xfe,0x40,0x5f,0xfe,0x15,
	0x02,0xb7,0x02,0xfe,0x11,0x2b,0x01,0xfe,0x71,0x63,0x05,0xfe,0x35,0x02,0x2b,0x01,
	0xfe,0x61,0x6f,0x05,0xfe,0x05,0x02,0xc3,0x05,0xfe,0xc1,0x6f,0x05,0xfe,0x84,0x02,
	0x5f,0xfe,0x21,0x7f,0x01,0xfe,0x24,0x02,0x5f,0xfe,0x01,0xd3,0x01,0xfe,0x21,0x67,
	0x01,0xfe,0x25,0x02,0x7b,0x0b,0xfe,0x61,0xf3,0x02,0xfe,0x65,0x02,0x13,0x01,0xfe,
	0x41,0x2b,0x10,0xfe,0x30,0x53,0xfe,0x35,0x02,0x87,0x02,0xfe,0x61,0x53,0x03,0xfe,
	0x20,0x5f,0xfe,0x40,0xcb,0xfe,0x15,0x02,0xb3,0x09,0xfe,0x71,0xf3,0x02,0xfe,0x85,
	0x02,0x3f,0x02,0xfe,0x81,0xd7,0x09,0xfe,0xa4,0x02,0x5f,0xfe,0xe4,0x02,0xbf,0xfe,
	0x65,0x02,0xff,0x02,0xfe,0x61,0xf3,0x05,0xfe,0x04,0x02,0x53,0xfe,0x01,0x7f,0x07,
	0xfe,0x21,0x33,0x02,0xfe,0x31,0x0b,0x03,0xfe,0x34,0x02,0x53,0xfe,0xa5,0x02,0x93,
	0x08,0xfe,0x41,0x2f,0x06,0xfe,0x65,0x02,0x93,0x08,0xfe,0x01,0xa3,0x07,0xfe,0x31,
	0x5b,0x01,0xfe,0xb5,0x01,0xa3,0x07,0xfe,0x00,0x47,0xfe,0x20,0xfb,0xfe,0x00,0x9b,
	0xfe,0x24,0x02,0xfb,0xfe,0x20,0x53,0xfe,0x01,0xaf,0x01,0xfe,0x04,0x02,0x53,0xfe,
	0x01,0xb7,0x02,0xfe,0x11,0xd3,0x0a,0xfe,0x71,0xb7,0x02,0xfe,0x25,0x02,0x37,0x13,
	0xfe,0x20,0x5f,0xfe,0x24,0x02,0x53,0xfe,0x01,0x0b,0x06,0xfe,0x21,0x07,0x01,0xfe,
	0x00,0xbf,0xfe,0x05,0x02,0x07,0x01,0xfe,0x00,0xa7,0xfe,0x41,0x13,0x04,0xfe,0x04,
	0x02,0xa7,0xfe,0x30,0x9b,0xfe,0x95,0x02,0x5b,0x01,0xfe,0x31,0xd7,0x15,0xfe,0x35,
	0x02,0x63,0x02,0xfe,0x41,0x5b,0x01,0xfe,0x31,0x8f,0x15,0xfe,0x34,0x02,0x5f,0xfe,
	0x31,0xb7,0x02,0xfe,0x50,0x5f,0xfe,0x25,0x02,0x13,0x01,0xfe,0x41,0xfb,0x15,0xfe,
	0x30,0xb3,0xfe,0x34,0x02,0x5f,0xfe,0x00,0xbf,0xfe,0x21,0xcf,0x02,0xfe,0x84,0x02,
	0xbf,0xfe,0x61,0x7f,0x01,0xfe,0x41,0x4b,0x05,0xfe,0x85,0x02,0x7f,0x01,0xfe,0x41,
	0xbb,0x0a,0xfe,0x25,0x02,0x93,0x08,0xfe,0x71,0x73,0x01,0xfe,0x71,0xe7,0x0b,0xfe,
	0x25,0x02,0x93,0x08,0xfe,0x41,0x8b,0x01,0xfe,0x64,0x02,0xcb,0xfe,0x01,0x4b,0x02,
	0xfe,0x20,0xbf,0xfe,0x01,0xb7,0x11,0xfe,0x25,0x02,0x4b,0x14,0xfe,0x41,0x7f,0x01,
	0xfe,0x25,0x02,0x43,0x16,0xfe,0x81,0x67,0x04,0xfe,0x25,0x02,0xef,0x15,0xfe,0x31,
	0xc3,0x05,0xfe,0xf5,0x01,0x7f,0x1c,0xfe,0x40,0xa7,0xfe,0x04,0x02,0x9b,0xfe,0x00,
	0x0f,0xfe,0x05,0x02,0x43,0x16,0xfe,0x60,0x9b,0xfe,0x25,0x02,0xc7,0x1c,0xfe,0x25,
	0x02,0xdf,0x16,0xfe,0x71,0x37,0x01,0xfe,0x25,0x02,0x2f,0x1b,0xfe,0x11,0xd7,0x1b,
	0xfe,0x04,0x02,0xa7,0xfe,0x61,0x77,0x1b,0xfe,0x05,0x02,0x2f,0x03,0xfe,0x01,0xe3,
	0x06,0xfe,0x21,0xf7,0x0a,0xfe,0x25,0x02,0x03,0x05,0xfe,0xc1,0x4f,0x01,0xfe,0x25,
	0x02,0x83,0x15,0xfe,0x85,0x02,0x37,0x16,0xfe,0xa1,0xe3,0x03,0xfe,0x85,0x02,0x77,
	0x1b,0xfe,0x25,0x02,0x3b,0x18,0xfe,0x41,0x5b,0x0a,0xfe,0x25,0x02,0xe3,0x18,0xfe,
	0x40,0x53,0xfe,0x84,0x02,0xa7,0xfe,0x05,0x02,0x53,0x03,0xfe,0x00,0x0f,0xfe,0x05,
	0x02,0x33,0x05,0xfe,0x31,0x2b,0x16,0xfe,0x35,0x02,0xff,0x02,0xfe,0x61,0xeb,0x0a,
	0xfe,0x85,0x02,0x6f,0x17,0xfe,0x25,0x02,0x2b,0x07,0xfe,0x41,0xdf,0x07,0xfe,0x25,
	0x02,0xfb,0x03,0xfe,0x71,0x93,0x02,0xfe,0x35,0x02,0x7b,0x20,0xfe,0x01,0xa3,0x0d,
	0xfe,0x21,0x07,0x01,0xfe,0x25,0x02,0xa3,0x0d,0xfe,0x31,0x37,0x16,0xfe,0x25,0x02,
	0x03,0x02,0xfe,0x11,0xf3,0x05,0xfe,0x05,0x02,0x9f,0x02,0xfe,0xc1,0x4f,0x01,0xfe,
	0x25,0x02,0xf7,0x04,0xfe,0x71,0xef,0x03,0xfe,0xb4,0x02,0x5f,0xfe,0x85,0x02,0x8f,
	0x1b,0xfe,0x71,0xa3,0x0a,0xfe,0x55,0x02,0xb3,0x0f,0xfe,0x85,0x02,0x2b,0x13,0xfe,
	0x81,0x73,0x01,0xfe,0xa4,0x02,0x5f,0xfe,0x85,0x02,0x2b,0x13,0xfe,0x41,0x73,0x10,
	0xfe,0x24,0x02,0xb3,0xfe,0x81,0x33,0x08,0xfe,0x44,0x02,0xb3,0xfe,0x85,0x02,0x67,
	0x01,0xfe,0xa1,0x7f,0x10,0xfe,0x85,0x02,0x93,0x17,0xfe,0x05,0x02,0xeb,0x04,0xfe,
	0x61,0x3f,0x0e,0xfe,0x41,0xd3,0x16,0xfe,0x20,0xbf,0xfe,0x25,0x02,0x4b,0x08,0xfe,
	0xc1,0x8b,0x10,0xfe,0x25,0x02,0x53,0x09,0xfe,0x70,0x53,0xfe,0x55,0x02,0xcb,0x0f,
	0xfe,0xa1,0x8b,0x10,0xfe,0x65,0x02,0x1f,0x16,0xfe,0x41,0x97,0x01,0xfe,0x25,0x02,
	0x7f,0x01,0xfe,0x61,0x7f,0x16,0xfe,0xe5,0x02,0x1f,0x16,0xfe,0x25,0x02,0x8b,0x07,
	0xfe,0x65,0x02,0xc7,0x01,0xfe,0x61,0x2b,0x16,0xfe,0x05,0x02,0x87,0x08,0xfe,0x01,
	0xd7,0x06,0xfe,0x21,0x1b,0x14,0xfe,0x31,0x07,0x10,0xfe,0x55,0x02,0x87,0x17,0xfe,
	0x41,0x8b,0x16,0xfe,0x04,0x02,0xb3,0xfe,0x01,0x2b,0x07,0xfe,0x70,0xb3,0xfe,0xf5,
	0x01,0xdb,0x0b,0xfe,0x20,0x53,0xfe,0x45,0x02,0x83,0x0c,0xfe,0x40,0x53,0xfe,0x25,
	0x02,0x83,0x0c,0xfe,0x40,0x53,0xfe,0x25,0x02,0x83,0x0c,0xfe,0xf5,0x01,0xf7,0x2b,
	0xfe,0x71,0x6f,0x17,0xfe,0x65,0x02,0x9f,0x02,0xfe,0x85,0x02,0x03,0x2c,0xfe,0x01,
	0x4f,0x07,0xfe,0x55,0x02,0x9b,0x15,0xfe,0x11,0x33,0x0b,0xfe,0x05,0x02,0x4f,0x01,
	0xfe,0x60,0xfb,0xfe,0x85,0x02,0x33,0x0b,0xfe,0xa1,0x5b,0x01,0xfe,0x84,0x02,0x5f,
	0xfe,0x85,0x02,0x33,0x0b,0xfe,0x41,0x07,0x04,0xfe,0x25,0x02,0x8b,0x07,0xfe,0x41,
	0xb3,0x09,0xfe,0x21,0x13,0x01,0xfe,0x25,0x02,0x6f,0x05,0xfe,0x41,0x1b,0x2c,0xfe,
	0x41,0x6f,0x05,0xfe,0x24,0x02,0x5f,0xfe,0x31,0x63,0x05,0xfe,0x50,0xbf,0xfe,0x25,
	0x02,0x27,0x05,0xfe,0x00,0x5f,0xfe,0x81,0x2f,0x06,0xfe,0x30,0x6b,0xfe,0xf5,0x01,
	0x8b,0x0a,0xfe,0x20,0x5f,0xfe,0x40,0xbf,0xfe,0x25,0x02,0x93,0x05,0xfe,0x31,0x4b,
	0x2c,0xfe,0x35,0x02,0x73,0x01,0xfe,0x01,0xe3,0x06,0xfe,0x20,0x53,0xfe,0x15,0x02,
	0xe3,0x06,0xfe,0x11,0x4b,0x17,0xfe,0x05,0x02,0x43,0x04,0xfe,0x31,0x87,0x05,0xfe,
	0x95,0x02,0x4f,0x01,0xfe,0x61,0xeb,0x2e,0xfe,0x24,0x02,0xfb,0xfe,0x05,0x02,0xff,
	0x26,0xfe,0x61,0x13,0x16,0xfe,0x25,0x02,0x9f,0x26,0xfe,0xa1,0x5f,0x03,0xfe,0x41,
	0x4f,0x0a,0xfe,0x65,0x02,0x5b,0x01,0xfe,0x61,0xe3,0x1b,0xfe,0x65,0x02,0x2b,0x04,
	0xfe,0x61,0x17,0x1b,0xfe,0x25,0x02,0x77,0x0c,0xfe,0x71,0x9f,0x05,0xfe,0x34,0x02,
	0x5f,0xfe,0x61,0x77,0x1b,0xfe,0x25,0x02,0x6f,0x08,0xfe,0x71,0x17,0x1b,0xfe,0x35,
	0x02,0xbf,0x09,0xfe,0x01,0xb3,0x06,0xfe,0x65,0x02,0xbb,0x31,0xfe,0x31,0x6f,0x2c,
	0xfe,0x35,0x02,0x1b,0x02,0xfe,0x61,0x4f,0x16,0xfe,0x25,0x02,0x6f,0x35,0xfe,0x41,
	0x13,0x37,0xfe,0x25,0x02,0x0b,0x0c,0xfe,0x65,0x02,0xc7,0x31,0xfe,0x31,0x7b,0x05,
	0xfe,0x35,0x02,0xf7,0x16,0xfe,0x01,0xb7,0x0b,0xfe,0x65,0x02,0x97,0x10,0xfe,0x60,
	0x53,0xfe,0x05,0x02,0xef,0x15,0xfe,0x00,0x0f,0xfe,0xf5,0x01,0x7b,0x05,0xfe,0x11,
	0xd3,0x1c,0xfe,0x05,0x02,0xc3,0x05,0xfe,0x41,0x8f,0x03,0xfe,0xa5,0x02,0x03,0x0b,
	0xfe,0x71,0x43,0x01,0xfe,0x55,0x02,0xa3,0x0a,0xfe,0xe5,0x02,0x83,0x1b,0xfe,0x25,
	0x02,0x9b,0x03,0xfe,0x81,0x23,0x2a,0xfe,0xa5,0x02,0x37,0x07,0xfe,0x41,0xaf,0x13,
	0xfe,0x65,0x02,0x63,0x05,0xfe,0x61,0xd7,0x06,0xfe,0x65,0x02,0x83,0x15,0xfe,0xc1,
	0x03,0x0b,0xfe,0x25,0x02,0x8f,0x15,0xfe,0x81,0xf7,0x0d,0xfe,0xa4,0x02,0x5f,0xfe,
	0x85,0x02,0x4f,0x16,0xfe,0x81,0xf7,0x0d,0xfe,0x45,0x02,0x03,0x17,0xfe,0x41,0x9f,
	0x05,0xfe,0x24,0x02,0xb3,0xfe,0x81,0x67,0x04,0xfe,0xa4,0x02,0x5f,0xfe,0x65,0x02,
	0x73,0x01,0xfe,0x61,0xc7,0x10,0xfe,0x85,0x02,0xb7,0x17,0xfe,0x25,0x02,0x2b,0x0d,
	0xfe,0x71,0x27,0x0b,0xfe,0x55,0x02,0x8b,0x0d,0xfe,0x71,0x27,0x32,0xfe,0x94,0x02,
	0x5f,0xfe,0x25,0x02,0xcf,0x14,0xfe,0x61,0x2b,0x16,0xfe,0x24,0x02,0x53,0xfe,0x41,
	0x5b,0x01,0xfe,0x25,0x02,0xe7,0x17,0xfe,0x81,0x1b,0x32,0xfe,0xa4,0x02,0x5f,0xfe,
	0x41,0x93,0x17,0xfe,0x64,0x02,0x5f,0xfe,0x01,0xbb,0x01,0xfe,0x25,0x02,0x77,0x03,
	0xfe,0x21,0x27,0x0b,0xfe,0x45,0x02,0x0f,0x2c,0xfe,0x70,0x53,0xfe,0x35,0x02,0x1f,
	0x07,0xfe,0x40,0x5f,0xfe,0x05,0x03,0x6f,0x2c,0xfe,0x25,0x02,0x73,0x07,0xfe,0x20,
	0xb3,0xfe,0x85,0x02,0x4f,0x2b,0xfe,0x30,0xa7,0xfe,0x71,0x8f,0x2a,0xfe,0x55,0x02,
	0x63,0x0b,0xfe,0x71,0xef,0x1b,0xfe,0x65,0x02,0xa7,0x03,0xfe,0x31,0xdb,0x02,0xfe,
	0xf5,0x02,0x7b,0x2c,0xfe,0x01,0x4b,0x02,0xfe,0x25,0x02,0x1f,0x07,0xfe,0x81,0x7f,
	0x01,0xfe,0x45,0x02,0xf3,0x02,0xfe,0x81,0xef,0x1b,0xfe,0x45,0x02,0xbf,0x30,0xfe,
	0x65,0x02,0x57,0x2c,0xfe,0x41,0xf3,0x02,0xfe,0x85,0x02,0xcb,0x15,0xfe,0xc1,0x8b,
	0x16,0xfe,0x05,0x02,0xd3,0x01,0xfe,0x61,0x53,0x03,0xfe,0x80,0x6b,0xfe,0x45,0x02,
	0xe3,0x2d,0xfe,0xd0,0x6b,0xfe,0xf5,0x01,0x93,0x14,0xfe,0x05,0x02,0x3b,0x3c,0xfe,
	0x61,0x7f,0x16,0xfe,0x25,0x02,0x3b,0x03,0xfe,0xa1,0xcf,0x05,0xfe,0x25,0x02,0xeb,
	0x31,0xfe,0x21,0x47,0x15,0xfe,0x30,0x6b,0xfe,0xd5,0x01,0x4f,0x01,0xfe,0x31,0xdb,
	0x02,0xfe,0xf5,0x01,0xdb,0x3e,0xfe,0x81,0xaf,0x37,0xfe,0x45,0x02,0x33,0x08,0xfe,
	0x81,0xaf,0x37,0xfe,0x24,0x02,0x5f,0xfe,0x61,0xa3,0x16,0xfe,0x85,0x02,0x4b,0x05,
	0xfe,0xe5,0x02,0xf3,0x17,0xfe,0x64,0x02,0xb3,0xfe,0x75,0x02,0xc3,0x02,0xfe,0x11,
	0xbf,0x03,0xfe,0x05,0x02,0xaf,0x37,0xfe,0x01,0xcf,0x11,0xfe,0x41,0x87,0x08,0xfe,
	0x25,0x02,0x97,0x31,0xfe,0x70,0x53,0xfe,0x35,0x02,0xfb,0x06,0xfe,0x40,0x5f,0xfe,
	0x85,0x02,0x3b,0x48,0xfe,0x61,0x7f,0x16,0xfe,0xf5,0x01,0x0f,0x02,0xfe,0x10,0x0f,
	0xfe,0x05,0x02,0x1f,0x04,0xfe,0x01,0x87,0x14,0xfe,0x40,0xfb,0xfe,0x25,0x02,0x83,
	0x21,0xfe,0x71,0x4f,0x01,0xfe,0xf5,0x01,0x03,0x32,0xfe,0x05,0x02,0xe3,0x42,0xfe,
	0x31,0x9f,0x02,0xfe,0xf5,0x01,0x77,0x15,0xfe,0x71,0x37,0x01,0xfe,0x35,0x02,0x77,
	0x21,0xfe,0x61,0xab,0x2c,0xfe,0x25,0x02,0x17,0x15,0xfe,0x21,0x4b,0x05,0xfe,0x30,
	0xbf,0xfe,0xf5,0x01,0xbf,0x15,0xfe,0x41,0x9b,0x03,0xfe,0x25,0x02,0x73,0x16,0xfe,
	0xa1,0x5b,0x01,0xfe,0x25,0x02,0xb7,0x11,0xfe,0x41,0x63,0x2f,0xfe,0x25,0x02,0xe7,
	0x0e,0xfe,0x65,0x02,0xbf,0x42,0xfe,0x61,0xbf,0x1b,0xfe,0x25,0x02,0x5b,0x01,0xfe,
	0xc5,0x02,0x5b,0x16,0xfe,0x40,0x5f,0xfe,0xa5,0x02,0x87,0x4d,0xfe,0x70,0xbf,0xfe,
	0x55,0x02,0xa3,0x10,0xfe,0x65,0x02,0xb3,0x42,0xfe,0x85,0x02,0xbb,0x01,0xfe,0xa5,
	0x02,0x9f,0x4d,0xfe,0x71,0xfb,0x42,0xfe,0x54,0x02,0xb3,0xfe,0x65,0x02,0x27,0x32,
	0xfe,0x60,0x5f,0xfe,0xe5,0x02,0x03,0x53,0xfe,0x05,0x02,0xbb,0x07,0xfe,0x31,0x67,
	0x01,0xfe,0xf5,0x01,0xbb,0x07,0xfe,0x65,0x02,0x53,0x21,0xfe,0x60,0x53,0xfe,0x25,
	0x02,0x0f,0x08,0xfe,0x55,0x02,0xb3,0x3c,0xfe,0x10,0x0f,0xfe,0x71,0x43,0x01,
} ;

}

#endif // DX_NON_FILTER
