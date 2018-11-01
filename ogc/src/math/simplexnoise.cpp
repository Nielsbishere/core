#include <math.h>
#include "template/common.h"
#include "math/simplexnoise.h"
using namespace oi;

Grad::Grad(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) { }

Grad::Grad(f32 x, f32 y, f32 z) : Grad(x, y, z, (f32) 0) {}

Grad::Grad() : Grad(0, 0, 0) { }

Grad SimplexNoise::grad3[12] = {
	Grad(1, 1, 0), Grad(-1, 1, 0), Grad(1, -1, 0), Grad(-1, -1, 0),
	Grad(1, 0, 1), Grad(-1, 0, 1), Grad(1, 0, -1), Grad(-1, 0, -1),
	Grad(0, 1, 1), Grad(0, -1, 1), Grad(0, 1, -1), Grad(0, -1, -1) };

Grad SimplexNoise::grad4[32] = {
	Grad(0, 1, 1, 1), Grad(0, 1, 1, -1),
	Grad(0, 1, -1, 1), Grad(0, 1, -1, -1), Grad(0, -1, 1, 1),
	Grad(0, -1, 1, -1), Grad(0, -1, -1, 1), Grad(0, -1, -1, -1),
	Grad(1, 0, 1, 1), Grad(1, 0, 1, -1), Grad(1, 0, -1, 1),
	Grad(1, 0, -1, -1), Grad(-1, 0, 1, 1), Grad(-1, 0, 1, -1),
	Grad(-1, 0, -1, 1), Grad(-1, 0, -1, -1), Grad(1, 1, 0, 1),
	Grad(1, 1, 0, -1), Grad(1, -1, 0, 1), Grad(1, -1, 0, -1),
	Grad(-1, 1, 0, 1), Grad(-1, 1, 0, -1), Grad(-1, -1, 0, 1),
	Grad(-1, -1, 0, -1), Grad(1, 1, 1, 0), Grad(1, 1, -1, 0),
	Grad(1, -1, 1, 0), Grad(1, -1, -1, 0), Grad(-1, 1, 1, 0),
	Grad(-1, 1, -1, 0), Grad(-1, -1, 1, 0), Grad(-1, -1, -1, 0) };

u8 SimplexNoise::p[512] = {
	151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194,
	233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21,
	10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94,
	252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149,
	56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
	134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
	60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132,
	187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159,
	86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250,
	124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207,
	206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183,
	170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101,
	155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110,
	79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
	81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181,
	199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4,
	150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243,
	141, 128, 195, 78, 66, 215, 61, 156, 180
};

u8 SimplexNoise::perm[512] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,

	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF
};

u8 SimplexNoise::permMod12[512] = {
	0x00 % 12,  0x01 % 12,  0x02 % 12,  0x03 % 12,  0x04 % 12,  0x05 % 12,  0x06 % 12,  0x07 % 12,  0x08 % 12,  0x09 % 12,  0x0A % 12,  0x0B % 12,  0x0C % 12,  0x0D % 12,  0x0E % 12,  0x0F % 12,
	0x10 % 12,  0x11 % 12,  0x12 % 12,  0x13 % 12,  0x14 % 12,  0x15 % 12,  0x16 % 12,  0x17 % 12,  0x18 % 12,  0x19 % 12,  0x1A % 12,  0x1B % 12,  0x1C % 12,  0x1D % 12,  0x1E % 12,  0x1F % 12,
	0x20 % 12,  0x21 % 12,  0x22 % 12,  0x23 % 12,  0x24 % 12,  0x25 % 12,  0x26 % 12,  0x27 % 12,  0x28 % 12,  0x29 % 12,  0x2A % 12,  0x2B % 12,  0x2C % 12,  0x2D % 12,  0x2E % 12,  0x2F % 12,
	0x30 % 12,  0x31 % 12,  0x32 % 12,  0x33 % 12,  0x34 % 12,  0x35 % 12,  0x36 % 12,  0x37 % 12,  0x38 % 12,  0x39 % 12,  0x3A % 12,  0x3B % 12,  0x3C % 12,  0x3D % 12,  0x3E % 12,  0x3F % 12,
	0x40 % 12,  0x41 % 12,  0x42 % 12,  0x43 % 12,  0x44 % 12,  0x45 % 12,  0x46 % 12,  0x47 % 12,  0x48 % 12,  0x49 % 12,  0x4A % 12,  0x4B % 12,  0x4C % 12,  0x4D % 12,  0x4E % 12,  0x4F % 12,
	0x50 % 12,  0x51 % 12,  0x52 % 12,  0x53 % 12,  0x54 % 12,  0x55 % 12,  0x56 % 12,  0x57 % 12,  0x58 % 12,  0x59 % 12,  0x5A % 12,  0x5B % 12,  0x5C % 12,  0x5D % 12,  0x5E % 12,  0x5F % 12,
	0x60 % 12,  0x61 % 12,  0x62 % 12,  0x63 % 12,  0x64 % 12,  0x65 % 12,  0x66 % 12,  0x67 % 12,  0x68 % 12,  0x69 % 12,  0x6A % 12,  0x6B % 12,  0x6C % 12,  0x6D % 12,  0x6E % 12,  0x6F % 12,
	0x70 % 12,  0x71 % 12,  0x72 % 12,  0x73 % 12,  0x74 % 12,  0x75 % 12,  0x76 % 12,  0x77 % 12,  0x78 % 12,  0x79 % 12,  0x7A % 12,  0x7B % 12,  0x7C % 12,  0x7D % 12,  0x7E % 12,  0x7F % 12,
	0x80 % 12,  0x81 % 12,  0x82 % 12,  0x83 % 12,  0x84 % 12,  0x85 % 12,  0x86 % 12,  0x87 % 12,  0x88 % 12,  0x89 % 12,  0x8A % 12,  0x8B % 12,  0x8C % 12,  0x8D % 12,  0x8E % 12,  0x8F % 12,
	0x90 % 12,  0x91 % 12,  0x92 % 12,  0x93 % 12,  0x94 % 12,  0x95 % 12,  0x96 % 12,  0x97 % 12,  0x98 % 12,  0x99 % 12,  0x9A % 12,  0x9B % 12,  0x9C % 12,  0x9D % 12,  0x9E % 12,  0x9F % 12,
	0xA0 % 12,  0xA1 % 12,  0xA2 % 12,  0xA3 % 12,  0xA4 % 12,  0xA5 % 12,  0xA6 % 12,  0xA7 % 12,  0xA8 % 12,  0xA9 % 12,  0xAA % 12,  0xAB % 12,  0xAC % 12,  0xAD % 12,  0xAE % 12,  0xAF % 12,
	0xB0 % 12,  0xB1 % 12,  0xB2 % 12,  0xB3 % 12,  0xB4 % 12,  0xB5 % 12,  0xB6 % 12,  0xB7 % 12,  0xB8 % 12,  0xB9 % 12,  0xBA % 12,  0xBB % 12,  0xBC % 12,  0xBD % 12,  0xBE % 12,  0xBF % 12,
	0xC0 % 12,  0xC1 % 12,  0xC2 % 12,  0xC3 % 12,  0xC4 % 12,  0xC5 % 12,  0xC6 % 12,  0xC7 % 12,  0xC8 % 12,  0xC9 % 12,  0xCA % 12,  0xCB % 12,  0xCC % 12,  0xCD % 12,  0xCE % 12,  0xCF % 12,
	0xD0 % 12,  0xD1 % 12,  0xD2 % 12,  0xD3 % 12,  0xD4 % 12,  0xD5 % 12,  0xD6 % 12,  0xD7 % 12,  0xD8 % 12,  0xD9 % 12,  0xDA % 12,  0xDB % 12,  0xDC % 12,  0xDD % 12,  0xDE % 12,  0xDF % 12,
	0xE0 % 12,  0xE1 % 12,  0xE2 % 12,  0xE3 % 12,  0xE4 % 12,  0xE5 % 12,  0xE6 % 12,  0xE7 % 12,  0xE8 % 12,  0xE9 % 12,  0xEA % 12,  0xEB % 12,  0xEC % 12,  0xED % 12,  0xEE % 12,  0xEF % 12,
	0xF0 % 12,  0xF1 % 12,  0xF2 % 12,  0xF3 % 12,  0xF4 % 12,  0xF5 % 12,  0xF6 % 12,  0xF7 % 12,  0xF8 % 12,  0xF9 % 12,  0xFA % 12,  0xFB % 12,  0xFC % 12,  0xFD % 12,  0xFE % 12,  0xFF % 12,

	0x00 % 12,  0x01 % 12,  0x02 % 12,  0x03 % 12,  0x04 % 12,  0x05 % 12,  0x06 % 12,  0x07 % 12,  0x08 % 12,  0x09 % 12,  0x0A % 12,  0x0B % 12,  0x0C % 12,  0x0D % 12,  0x0E % 12,  0x0F % 12,
	0x10 % 12,  0x11 % 12,  0x12 % 12,  0x13 % 12,  0x14 % 12,  0x15 % 12,  0x16 % 12,  0x17 % 12,  0x18 % 12,  0x19 % 12,  0x1A % 12,  0x1B % 12,  0x1C % 12,  0x1D % 12,  0x1E % 12,  0x1F % 12,
	0x20 % 12,  0x21 % 12,  0x22 % 12,  0x23 % 12,  0x24 % 12,  0x25 % 12,  0x26 % 12,  0x27 % 12,  0x28 % 12,  0x29 % 12,  0x2A % 12,  0x2B % 12,  0x2C % 12,  0x2D % 12,  0x2E % 12,  0x2F % 12,
	0x30 % 12,  0x31 % 12,  0x32 % 12,  0x33 % 12,  0x34 % 12,  0x35 % 12,  0x36 % 12,  0x37 % 12,  0x38 % 12,  0x39 % 12,  0x3A % 12,  0x3B % 12,  0x3C % 12,  0x3D % 12,  0x3E % 12,  0x3F % 12,
	0x40 % 12,  0x41 % 12,  0x42 % 12,  0x43 % 12,  0x44 % 12,  0x45 % 12,  0x46 % 12,  0x47 % 12,  0x48 % 12,  0x49 % 12,  0x4A % 12,  0x4B % 12,  0x4C % 12,  0x4D % 12,  0x4E % 12,  0x4F % 12,
	0x50 % 12,  0x51 % 12,  0x52 % 12,  0x53 % 12,  0x54 % 12,  0x55 % 12,  0x56 % 12,  0x57 % 12,  0x58 % 12,  0x59 % 12,  0x5A % 12,  0x5B % 12,  0x5C % 12,  0x5D % 12,  0x5E % 12,  0x5F % 12,
	0x60 % 12,  0x61 % 12,  0x62 % 12,  0x63 % 12,  0x64 % 12,  0x65 % 12,  0x66 % 12,  0x67 % 12,  0x68 % 12,  0x69 % 12,  0x6A % 12,  0x6B % 12,  0x6C % 12,  0x6D % 12,  0x6E % 12,  0x6F % 12,
	0x70 % 12,  0x71 % 12,  0x72 % 12,  0x73 % 12,  0x74 % 12,  0x75 % 12,  0x76 % 12,  0x77 % 12,  0x78 % 12,  0x79 % 12,  0x7A % 12,  0x7B % 12,  0x7C % 12,  0x7D % 12,  0x7E % 12,  0x7F % 12,
	0x80 % 12,  0x81 % 12,  0x82 % 12,  0x83 % 12,  0x84 % 12,  0x85 % 12,  0x86 % 12,  0x87 % 12,  0x88 % 12,  0x89 % 12,  0x8A % 12,  0x8B % 12,  0x8C % 12,  0x8D % 12,  0x8E % 12,  0x8F % 12,
	0x90 % 12,  0x91 % 12,  0x92 % 12,  0x93 % 12,  0x94 % 12,  0x95 % 12,  0x96 % 12,  0x97 % 12,  0x98 % 12,  0x99 % 12,  0x9A % 12,  0x9B % 12,  0x9C % 12,  0x9D % 12,  0x9E % 12,  0x9F % 12,
	0xA0 % 12,  0xA1 % 12,  0xA2 % 12,  0xA3 % 12,  0xA4 % 12,  0xA5 % 12,  0xA6 % 12,  0xA7 % 12,  0xA8 % 12,  0xA9 % 12,  0xAA % 12,  0xAB % 12,  0xAC % 12,  0xAD % 12,  0xAE % 12,  0xAF % 12,
	0xB0 % 12,  0xB1 % 12,  0xB2 % 12,  0xB3 % 12,  0xB4 % 12,  0xB5 % 12,  0xB6 % 12,  0xB7 % 12,  0xB8 % 12,  0xB9 % 12,  0xBA % 12,  0xBB % 12,  0xBC % 12,  0xBD % 12,  0xBE % 12,  0xBF % 12,
	0xC0 % 12,  0xC1 % 12,  0xC2 % 12,  0xC3 % 12,  0xC4 % 12,  0xC5 % 12,  0xC6 % 12,  0xC7 % 12,  0xC8 % 12,  0xC9 % 12,  0xCA % 12,  0xCB % 12,  0xCC % 12,  0xCD % 12,  0xCE % 12,  0xCF % 12,
	0xD0 % 12,  0xD1 % 12,  0xD2 % 12,  0xD3 % 12,  0xD4 % 12,  0xD5 % 12,  0xD6 % 12,  0xD7 % 12,  0xD8 % 12,  0xD9 % 12,  0xDA % 12,  0xDB % 12,  0xDC % 12,  0xDD % 12,  0xDE % 12,  0xDF % 12,
	0xE0 % 12,  0xE1 % 12,  0xE2 % 12,  0xE3 % 12,  0xE4 % 12,  0xE5 % 12,  0xE6 % 12,  0xE7 % 12,  0xE8 % 12,  0xE9 % 12,  0xEA % 12,  0xEB % 12,  0xEC % 12,  0xED % 12,  0xEE % 12,  0xEF % 12,
	0xF0 % 12,  0xF1 % 12,  0xF2 % 12,  0xF3 % 12,  0xF4 % 12,  0xF5 % 12,  0xF6 % 12,  0xF7 % 12,  0xF8 % 12,  0xF9 % 12,  0xFA % 12,  0xFB % 12,  0xFC % 12,  0xFD % 12,  0xFE % 12,  0xFF % 12
};

f32 SimplexNoise::F2 = (f32) 0.5 * (sqrt((f32) 3.0) - (f32) 1.0);
f32 SimplexNoise::G2 = ((f32) 3.0 - sqrt((f32) 3.0)) / (f32) 6.0;
f32 SimplexNoise::F4 = (sqrt((f32) 5.0) - (f32) 1.0) / (f32) 4.0;
f32 SimplexNoise::G4 = ((f32) 5.0 - sqrt((f32) 5.0)) / (f32) 20.0;
f32 SimplexNoise::F3 = (f32) 1.0 / (f32) 3.0;
f32 SimplexNoise::G3 = (f32) 1.0 / (f32) 6.0;

// 2D simplex noise
f32 SimplexNoise::noise(Vec2 pos) {
	f32 xin = pos.x;
	f32 yin = pos.y;

	f32 n0, n1, n2; // Noise contributions from the three corners
					// Skew the input space to determine which simplex cell we're in
	f32 s = (xin + yin) * F2; // Hairy factor for 2D
	i32 i = fastfloor(xin + s);
	i32 j = fastfloor(yin + s);
	f32 t = (i + j) * G2;
	f32 X0 = i - t; // Unskew the cell origin back to (x,y) space
	f32 Y0 = j - t;
	f32 x0 = xin - X0; // The x,y distances from the cell origin
	f32 y0 = yin - Y0;
	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	i32 i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
	if (x0 > y0) {
		i1 = 1;
		j1 = 0;
	} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else {
		i1 = 0;
		j1 = 1;
	}      // upper triangle, YX order: (0,0)->(0,1)->(1,1)
		   // A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
		   // a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
		   // c = (3-sqrt(3))/6
	f32 x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	f32 y1 = y0 - j1 + G2;
	f32 x2 = x0 - (f32) 1.0 + (f32) 2.0 * G2; // Offsets for last corner in (x,y) unskewed coords
	f32 y2 = y0 - (f32) 1.0 + (f32) 2.0 * G2;
	// Work out the hashed gradient indices of the three simplex corners
	i32 ii = i & 255;
	i32 jj = j & 255;
	i32 gi0 = permMod12[ii + perm[jj]];
	i32 gi1 = permMod12[ii + i1 + perm[jj + j1]];
	i32 gi2 = permMod12[ii + 1 + perm[jj + 1]];
	// Calculate the contribution from the three corners
	f32 t0 = (f32) 0.5 - x0 * x0 - y0 * y0;
	if (t0 < 0)
		n0 = (f32) 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0); // (x,y) of grad3 used for 2D gradient
	}
	f32 t1 = (f32) 0.5 - x1 * x1 - y1 * y1;
	if (t1 < 0)
		n1 = (f32) 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}
	f32 t2 = (f32) 0.5 - x2 * x2 - y2 * y2;
	if (t2 < 0)
		n2 = (f32) 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the i32erval [-1,1].
	return ((f32) 70.0 * (n0 + n1 + n2) + 1) / 2;
}

// 3D simplex noise
f32 SimplexNoise::noise(Vec3 pos) {
	f32 xin = pos.x;
	f32 yin = pos.y;
	f32 zin = pos.z;

	f32 n0, n1, n2, n3; // Noise contributions from the four corners
						// Skew the input space to determine which simplex cell we're in
	f32 s = (xin + yin + zin) * F3; // Very nice and simple skew factor for 3D
	i32 i = fastfloor(xin + s);
	i32 j = fastfloor(yin + s);
	i32 k = fastfloor(zin + s);
	f32 t = (i + j + k) * G3;
	f32 X0 = i - t; // Unskew the cell origin back to (x,y,z) space
	f32 Y0 = j - t;
	f32 Z0 = k - t;
	f32 x0 = xin - X0; // The x,y,z distances from the cell origin
	f32 y0 = yin - Y0;
	f32 z0 = zin - Z0;
	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	i32 i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
	i32 i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords
	if (x0 >= y0) {
		if (y0 >= z0) {
			i1 = 1;
			j1 = 0;
			k1 = 0;
			i2 = 1;
			j2 = 1;
			k2 = 0;
		} // X Y Z order
		else if (x0 >= z0) {
			i1 = 1;
			j1 = 0;
			k1 = 0;
			i2 = 1;
			j2 = 0;
			k2 = 1;
		} // X Z Y order
		else {
			i1 = 0;
			j1 = 0;
			k1 = 1;
			i2 = 1;
			j2 = 0;
			k2 = 1;
		} // Z X Y order
	}
	else { // x0<y0
		if (y0 < z0) {
			i1 = 0;
			j1 = 0;
			k1 = 1;
			i2 = 0;
			j2 = 1;
			k2 = 1;
		} // Z Y X order
		else if (x0 < z0) {
			i1 = 0;
			j1 = 1;
			k1 = 0;
			i2 = 0;
			j2 = 1;
			k2 = 1;
		} // Y Z X order
		else {
			i1 = 0;
			j1 = 1;
			k1 = 0;
			i2 = 1;
			j2 = 1;
			k2 = 0;
		} // Y X Z order
	}
	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.
	f32 x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
	f32 y1 = y0 - j1 + G3;
	f32 z1 = z0 - k1 + G3;
	f32 x2 = x0 - i2 + (f32) 2.0 * G3; // Offsets for third corner in (x,y,z) coords
	f32 y2 = y0 - j2 + (f32) 2.0 * G3;
	f32 z2 = z0 - k2 + (f32) 2.0 * G3;
	f32 x3 = x0 - (f32) 1.0 + (f32) 3.0 * G3; // Offsets for last corner in (x,y,z) coords
	f32 y3 = y0 - (f32) 1.0 + (f32) 3.0 * G3;
	f32 z3 = z0 - (f32) 1.0 + (f32) 3.0 * G3;
	// Work out the hashed gradient indices of the four simplex corners
	i32 ii = i & 255;
	i32 jj = j & 255;
	i32 kk = k & 255;
	i32 gi0 = permMod12[ii + perm[jj + perm[kk]]];
	i32 gi1 = permMod12[ii + i1 + perm[jj + j1 + perm[kk + k1]]];
	i32 gi2 = permMod12[ii + i2 + perm[jj + j2 + perm[kk + k2]]];
	i32 gi3 = permMod12[ii + 1 + perm[jj + 1 + perm[kk + 1]]];
	// Calculate the contribution from the four corners
	f32 t0 = (f32) 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
	if (t0 < 0)
		n0 = (f32) 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
	}
	f32 t1 = (f32) 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
	if (t1 < 0)
		n1 = (f32) 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
	}
	f32 t2 = (f32) 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
	if (t2 < 0)
		n2 = (f32) 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
	}
	f32 t3 = (f32) 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
	if (t3 < 0)
		n3 = (f32) 0.0;
	else {
		t3 *= t3;
		n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
	}
	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1]
	return (((f32) 32.0 * (n0 + n1 + n2 + n3)) + 1) / 2;
}

// 4D simplex noise, better simplex rank ordering method 2012-03-09
f32 SimplexNoise::noise(Vec4 pos) {
	f32 x = pos.x;
	f32 y = pos.y;
	f32 z = pos.z;
	f32 w = pos.w;

	f32 n0, n1, n2, n3, n4; // Noise contributions from the five corners
							// Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
	f32 s = (x + y + z + w) * F4; // Factor for 4D skewing
	i32 i = fastfloor(x + s);
	i32 j = fastfloor(y + s);
	i32 k = fastfloor(z + s);
	i32 l = fastfloor(w + s);
	f32 t = (i + j + k + l) * G4; // Factor for 4D unskewing
	f32 X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
	f32 Y0 = j - t;
	f32 Z0 = k - t;
	f32 W0 = l - t;
	f32 x0 = x - X0;  // The x,y,z,w distances from the cell origin
	f32 y0 = y - Y0;
	f32 z0 = z - Z0;
	f32 w0 = w - W0;
	// For the 4D case, the simplex is a 4D shape I won't even try to describe.
	// To find out which of the 24 possible simplices we're in, we need to
	// determine the magnitude ordering of x0, y0, z0 and w0.
	// Six pair-wise comparisons are performed between each possible pair
	// of the four coordinates, and the results are used to rank the numbers.
	i32 rankx = 0;
	i32 ranky = 0;
	i32 rankz = 0;
	i32 rankw = 0;
	if (x0 > y0)
		rankx++;
	else
		ranky++;
	if (x0 > z0)
		rankx++;
	else
		rankz++;
	if (x0 > w0)
		rankx++;
	else
		rankw++;
	if (y0 > z0)
		ranky++;
	else
		rankz++;
	if (y0 > w0)
		ranky++;
	else
		rankw++;
	if (z0 > w0)
		rankz++;
	else
		rankw++;
	i32 i1, j1, k1, l1; // The i32eger offsets for the second simplex corner
	i32 i2, j2, k2, l2; // The i32eger offsets for the third simplex corner
	i32 i3, j3, k3, l3; // The i32eger offsets for the fourth simplex corner
						// simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
						// Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
						// impossible. Only the 24 indices which have non-zero entries make any sense.
						// We use a thresholding to set the coordinates in turn from the largest magnitude.
						// Rank 3 denotes the largest coordinate.
	i1 = rankx >= 3 ? 1 : 0;
	j1 = ranky >= 3 ? 1 : 0;
	k1 = rankz >= 3 ? 1 : 0;
	l1 = rankw >= 3 ? 1 : 0;
	// Rank 2 denotes the second largest coordinate.
	i2 = rankx >= 2 ? 1 : 0;
	j2 = ranky >= 2 ? 1 : 0;
	k2 = rankz >= 2 ? 1 : 0;
	l2 = rankw >= 2 ? 1 : 0;
	// Rank 1 denotes the second smallest coordinate.
	i3 = rankx >= 1 ? 1 : 0;
	j3 = ranky >= 1 ? 1 : 0;
	k3 = rankz >= 1 ? 1 : 0;
	l3 = rankw >= 1 ? 1 : 0;
	// The fifth corner has all coordinate offsets = 1, so no need to compute that.
	f32 x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
	f32 y1 = y0 - j1 + G4;
	f32 z1 = z0 - k1 + G4;
	f32 w1 = w0 - l1 + G4;
	f32 x2 = x0 - i2 + (f32) 2.0 * G4; // Offsets for third corner in (x,y,z,w) coords
	f32 y2 = y0 - j2 + (f32) 2.0 * G4;
	f32 z2 = z0 - k2 + (f32) 2.0 * G4;
	f32 w2 = w0 - l2 + (f32) 2.0 * G4;
	f32 x3 = x0 - i3 + (f32) 3.0 * G4; // Offsets for fourth corner in (x,y,z,w) coords
	f32 y3 = y0 - j3 + (f32) 3.0 * G4;
	f32 z3 = z0 - k3 + (f32) 3.0 * G4;
	f32 w3 = w0 - l3 + (f32) 3.0 * G4;
	f32 x4 = x0 - (f32) 1.0 + (f32) 4.0 * G4; // Offsets for last corner in (x,y,z,w) coords
	f32 y4 = y0 - (f32) 1.0 + (f32) 4.0 * G4;
	f32 z4 = z0 - (f32) 1.0 + (f32) 4.0 * G4;
	f32 w4 = w0 - (f32) 1.0 + (f32) 4.0 * G4;
	// Work out the hashed gradient indices of the five simplex corners
	i32 ii = i & 255;
	i32 jj = j & 255;
	i32 kk = k & 255;
	i32 ll = l & 255;
	i32 gi0 = perm[ii + perm[jj + perm[kk + perm[ll]]]] % 32;
	i32 gi1 = perm[ii + i1 + perm[jj + j1 + perm[kk + k1 + perm[ll + l1]]]]
		% 32;
	i32 gi2 = perm[ii + i2 + perm[jj + j2 + perm[kk + k2 + perm[ll + l2]]]]
		% 32;
	i32 gi3 = perm[ii + i3 + perm[jj + j3 + perm[kk + k3 + perm[ll + l3]]]]
		% 32;
	i32 gi4 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1 + perm[ll + 1]]]]
		% 32;
	// Calculate the contribution from the five corners
	f32 t0 = (f32) 0.6 - x0 * x0 - y0 * y0 - z0 * z0 - w0 * w0;
	if (t0 < 0)
		n0 = (f32) 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad4[gi0], x0, y0, z0, w0);
	}
	f32 t1 = (f32) 0.6 - x1 * x1 - y1 * y1 - z1 * z1 - w1 * w1;
	if (t1 < 0)
		n1 = (f32) 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad4[gi1], x1, y1, z1, w1);
	}
	f32 t2 = (f32) 0.6 - x2 * x2 - y2 * y2 - z2 * z2 - w2 * w2;
	if (t2 < 0)
		n2 = (f32) 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad4[gi2], x2, y2, z2, w2);
	}
	f32 t3 = (f32) 0.6 - x3 * x3 - y3 * y3 - z3 * z3 - w3 * w3;
	if (t3 < 0)
		n3 = (f32) 0.0;
	else {
		t3 *= t3;
		n3 = t3 * t3 * dot(grad4[gi3], x3, y3, z3, w3);
	}
	f32 t4 = (f32) 0.6 - x4 * x4 - y4 * y4 - z4 * z4 - w4 * w4;
	if (t4 < 0)
		n4 = (f32) 0.0;
	else {
		t4 *= t4;
		n4 = t4 * t4 * dot(grad4[gi4], x4, y4, z4, w4);
	}
	// Sum up and scale the result to cover the range [-1,1]
	return ((f32) 27.0 * (n0 + n1 + n2 + n3 + n4) + 1) / 2;
}

f32 SimplexNoise::noise(f32 pos) {
	return noise(Vec2((f32) pos, (f32) 0.0));
}

f32 SimplexNoise::noise(f32 pos, f32 offset, u32 octaves, f32 persistence, f32 roughness, f32 freq) {
	f32 amp = 1, y = 0, max = 0;
	for (u32 i = 0; i < octaves; i++) {
		y += (noise(pos * freq + offset) + 1) * 0.5f * amp;
		max += amp;
		amp *= persistence;
		freq *= roughness;
	}
	return y / max;
}

f32 SimplexNoise::noise(Vec2 pos, Vec2 offset, u32 octaves, f32 persistence, f32 roughness, f32 freq) {
	f32 amp = 1, z = 0, max = 0;
	for (u32 i = 0; i < octaves; i++) {
		z += (noise(pos * freq + offset) + 1) * 0.5f * amp;
		max += amp;
		amp *= persistence;
		freq *= roughness;
	}
	return z / max;
}

f32 SimplexNoise::noise(Vec3 pos, Vec3 offset, u32 octaves, f32 persistence, f32 roughness, f32 freq) {
	f32 amp = 1, w = 0, max = 0;
	for (u32 i = 0; i < octaves; i++) {
		w += (noise(pos * freq + offset) + 1) * 0.5f * amp;
		max += amp;
		amp *= persistence;
		freq *= roughness;
	}
	return w / max;
}

f32 SimplexNoise::noise(Vec4 pos, Vec4 offset, u32 octaves, f32 persistence, f32 roughness, f32 freq) {
	f32 amp = 1, u = 0, max = 0;
	for (u32 i = 0; i < octaves; i++) {
		u += (noise(pos * freq + offset) + 1) * 0.5f * amp;
		max += amp;
		amp *= persistence;
		freq *= roughness;
	}
	return u / max;
}

i32 SimplexNoise::fastfloor(f32 x) {
	i32 xi = (i32)x;
	return x < xi ? xi - 1 : xi;
}

f32 SimplexNoise::dot(Grad g, f32 x, f32 y) {
	return g.x * x + g.y * y;
}

f32 SimplexNoise::dot(Grad g, f32 x, f32 y, f32 z) {
	return g.x * x + g.y * y + g.z * z;
}

f32 SimplexNoise::dot(Grad g, f32 x, f32 y, f32 z, f32 w) {
	return g.x * x + g.y * y + g.z * z + g.w * w;
}