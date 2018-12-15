#pragma once
#include "types/vector.h"
#include "template/enum.h"
#include "graphics/gl/generic.h"
#include "graphics/objects/graphicsresource.h"

#undef RGB

namespace oi {

	namespace gc {
		
		class Graphics;
		class CommandList;
		class Texture;
		class TextureList;

		typedef u32 TextureHandle;

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

			RGBA64f = 49, RGB64f = 50, RG64f = 51, R64f = 52,
			RGBA64u = 53, RGB64u = 54, RG64u = 55, R64u = 56,
			RGBA64i = 57, RGB64i = 58, RG64i = 59, R64i = 60,

			D16 = 61, D32 = 62, D16S8 = 63, D24S8 = 64, D32S8 = 65, Depth = 66,

			sRGBA8 = 67, sRGB8 = 68, sRG8 = 69, sR8 = 70,

			BGRA8 = 71, BGR8 = 72,
			BGRA8s = 73, BGR8s = 74,
			BGRA8u = 75, BGR8u = 76,
			BGRA8i = 77, BGR8i = 78,
			sBGRA8 = 79, sBGR8 = 80

		);

		DEnum(TextureUsage, u32, Undefined = 0,

			Render_target = 1, Render_depth = 2,
			Compute_target = 3,
			Image = 4

		);

		UEnum(TextureMipFilter, None = 0, Nearest = 1, Linear = 2);

		enum class TextureFormatStorage {
			INT,
			UINT,
			FLOAT,
			DOUBLE
		};

		DEnum(TextureLoadFormat, u32, Undefined = 0,
			R8 = 1,
			RG8 = 2,
			RGB8 = 3,
			RGBA8 = 4,
			sR8 = 5,
			sRG8 = 6,
			sRGB8 = 7,
			sRGBA8 = 8
		);

		struct TextureInfo {

			typedef Texture ResourceType;

			Vec2u res;
			TextureFormat format;
			TextureUsage usage;

			String path;
			Buffer dat;
			TextureLoadFormat loadFormat = TextureLoadFormat::Undefined;
			TextureMipFilter mipFilter = TextureMipFilter::Linear;

			u32 mipLevels = 1U;													//Automatic detection. No need to set it

			TextureList *parent;
			TextureHandle handle = u32_MAX;

			Vec2u changedStart = Vec2u(u32_MAX, u32_MAX), changedEnd = Vec2u();

			TextureInfo(Vec2u res, TextureFormat format, TextureUsage usage);
			explicit TextureInfo(TextureList *parent, String path, TextureLoadFormat loadFormat = TextureLoadFormat::sRGBA8, TextureMipFilter mipFilter = TextureMipFilter::Linear);
			explicit TextureInfo(TextureList *parent, Vec2u res, TextureLoadFormat format, TextureMipFilter mipFilter);

		};

		class Texture : public GraphicsResource {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			TextureFormat getFormat();
			TextureUsage getUsage();
			Vec2u getSize();
			u32 getStride();
			u32 getCpuSize();
			bool isOwned();

			TextureHandle getHandle();

			TextureExt &getExtension();
			const TextureInfo getInfo();

			void initParent(TextureList *parent);

			bool setPixels(Vec2u start, Vec2u length, Buffer values);
			bool getPixels(Vec2u start, Vec2u length, CopyBuffer &output);

			bool write(String path, Vec2u start = Vec2u(), Vec2u length = Vec2u());
			bool read(String path, Vec2u start = Vec2u(), Vec2u length = Vec2u());

		protected:

			~Texture();
			Texture(TextureInfo info);
			bool init(bool isOwned = true);

			bool getPixelsGpu(Vec2u start, Vec2u length, CopyBuffer &output);

			bool initData(bool isOwned);

			bool shouldStage();

			//Push changes to GPU
			void push();

			void flush(Vec2u start, Vec2u length);

		private:

			bool owned = false;

			TextureInfo info;
			TextureExt ext;

		};

	}

}