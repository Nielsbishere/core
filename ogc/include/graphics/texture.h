#pragma once
#include <types/vector.h>
#include <template/enum.h>

namespace oi {

	namespace gc {
		
		class Graphics;

		DEnum(TextureFormat, u32, Undefined = 0, 

			RGBA8 = 1, RGB8 = 2, RG8 = 3, R8 = 4,
			RGBA8s = 5, RGB8s = 6, RG8s = 7, R8s = 8,
			RGBA8u = 9, RGB8u = 10, RG8u = 11, R8u = 12,
			RGBA8i = 13, RGB8i = 14, RG8i = 15, R8i = 16,

			RGBA16 = 17, RGB16 = 18, RG16 = 19, R16 = 20,
			RGBA16s = 21, RGB16s = 22, RG16s = 23, R16s = 24,
			RGBA16u = 25, RGB16u = 26, RG16u = 27, R16u = 28,
			RGBA16i = 29, RGB16i = 30, RG16i = 31, R16i = 32,
			RGBA16f = 33, RGB16f = 34, RG16f = 35, R16f = 36,

			RGBA32f = 37, RGB32f = 38, RG32f = 39, R32f = 40,
			RGBA32u = 41, RGB32u = 42, RG32u = 43, R32u = 44,
			RGBA32i = 45, RGB32i = 46, RG32i = 47, R32i = 48,

			D16 = 49, D32 = 50, D16S8 = 51, D24S8 = 52, D32S8 = 53, Depth = 54,

			sRGBA8 = 55, sRGB8 = 56, sRG8 = 57, sR8 = 58

		);

		DEnum(TextureUsage, u32, Undefined = 0,

			Render_target = 1, Render_depth = 2,
			Shader_input_only = 3, General = 4

		);

		class Texture {

			friend class Graphics;

		public:

			~Texture();

			TextureFormat getFormat();
			TextureUsage getUsage();
			Vec2u getSize();

		protected:

			Texture(Vec2u size, TextureFormat format, TextureUsage usage);
			bool init(Graphics *g, bool isOwned = true);

		private:

			Vec2u size;
			TextureFormat format;
			TextureUsage usage;
			Graphics *g = nullptr;
			
			bool isOwned = false;

			u8 platformData[128];

		};

	}

}