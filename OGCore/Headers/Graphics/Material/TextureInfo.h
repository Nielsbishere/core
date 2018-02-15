#pragma once
#include <Template/Enum.h>
#include <Types/Buffer.h>
#undef RGB

namespace oi {

	namespace gc {
		
		///Format of the texture; just tells what data it contains
		DEnum(TextureFormat, u32, 
			Undefined = 0,
			R = 1, RG = 2, RGB = 3, RGBA = 4,
			Ri = 5, RGi = 6, RGBi = 7, RGBAi = 8,
			Depth = 9, DepthStencil = 10, Stencil = 11
		);

		///Layout of the texture; just tells how the data is structured
		///Suffixes:
		///-i is signed int, 
		///-f is float,
		///-s is signed 'percentage' (-1 -> 1), 
		///-u is unsigned int,
		///-c is compressed
		///-<none> is default; unsigned 'percentage' (0 -> 1)
		///Prefixes:
		///NS_- is not supported; it is just there because it might be supported in the future and padding
		DEnum(TextureLayout, u32, 
			Undefined = 0, 

			R = 1, Rs = 2, R16 = 3, R16s = 4, R16f = 5, R32f = 6, Rc = 7,
			RG = 8, RGs = 9, RG16 = 10, RG16s = 11, RG16f = 12, RG32f = 13, RGc = 14,
			RGB = 15, RGBs = 16, RGB16 = 17, RGB16s = 18, RGB16f = 19, RGB32f = 20, RGBc = 21,
			RGBA = 22, RGBAs = 23, RGBA16 = 24, RGBA16s = 25, RGBA16f = 26, RGBA32f = 27, RGBAc = 28,

			Ri = 29, Ru = 30, R16i = 31, R16u = 32, R32i = 33, R32u = 34,
			RGi = 35, RGu = 36, RG16i = 37, RG16u = 38, RG32i = 39, RG32u = 40,
			RGBi = 41, RGBu = 42, RGB16i = 43, RGB16u = 44, RGB32i = 45, RGB32u = 46,
			RGBAi = 47, RGBAu = 48, RGBA16i = 49, RGBA16u = 50, RGBA32i = 51, RGBA32u = 52,

			sRGB = 53, sRGBc = 54, sRGBA = 55, sRGBAc = 56,

			D16 = 57, D24 = 58, D32 = 59, D32f = 60, D24_S8 = 61, D32f_S8 = 62, S8 = 63
		);

		///Type of the texture; how the data is accessed through arrays
		DEnum(TextureType, u32, Texture1D = 0, Texture2D = 1, Texture3D = 2);

		struct TextureLayoutHelper {

			static TextureFormat getFormat(TextureLayout layout) {
				if(layout.getIndex() == 0) return TextureFormat::Undefined;
				if (layout.getIndex() < TextureLayout::Ri) return (layout.getIndex() - TextureLayout::R) / 7 + TextureFormat::R;
				if (layout.getIndex() < TextureLayout::sRGB) return (layout.getIndex() - TextureLayout::Ri) / 6 + TextureFormat::Ri;
				if (layout.getIndex() < TextureLayout::sRGBA) return TextureFormat::RGB;
				if (layout.getIndex() < TextureLayout::D16) return TextureFormat::RGBA;
				if (layout.getIndex() < TextureLayout::D24_S8) return TextureFormat::Depth;
				if (layout.getIndex() == TextureLayout::S8) return TextureFormat::Stencil;
				return TextureFormat::DepthStencil;
			}

			static bool isCompressed(TextureLayout layout) {
				return layout.getName().endsWith("c");
			}

			static bool isLinear(TextureLayout layout) {
				return layout.getName().startsWith("s");
			}

			static bool isValid(TextureLayout layout) {
				return layout.getIndex() != 0 && !layout.getName().startsWith("NS_");
			}

			static bool isInteger(TextureLayout layout) {
				return layout.getName().endsWith("i") || layout.getName().endsWith("u");
			}
		};

		class TextureInfo {

		public:

			//1D texture
			TextureInfo(u32 _width, TextureLayout _layout, Buffer _buf = Buffer::construct(nullptr, 0), u32 _binding = 0) : width(_width), height(0), length(0), type(TextureType::Texture1D), layout(_layout), format(TextureLayoutHelper::getFormat(_layout)), buf(_buf), binding(_binding) {}

			//2D texture
			TextureInfo(u32 _width, u32 _height, TextureLayout _layout, Buffer _buf = Buffer::construct(nullptr, 0), u32 _binding = 0) : width(_width), height(_height), length(0), type(TextureType::Texture2D), layout(_layout), format(TextureLayoutHelper::getFormat(_layout)), buf(_buf), binding(_binding) {}

			//3D texture
			TextureInfo(u32 _width, u32 _height, u32 _length, TextureLayout _layout, Buffer _buf = Buffer::construct(nullptr, 0), u32 _binding = 0) : width(_width), height(_height), length(_length), type(TextureType::Texture3D), layout(_layout), format(TextureLayoutHelper::getFormat(_layout)), buf(_buf), binding(_binding) {}

			TextureType getType() { return type; }
			TextureFormat getFormat() { return format; }
			TextureLayout getLayout() { return layout; }
			bool isValid() { return TextureLayoutHelper::isValid(layout); }

			u32 getWidth() { return width; }
			u32 getHeight() { return height; }
			u32 getLength() { return length; }

			void setBinding(u32 _binding) { binding = _binding; }
			u32 getBinding() { return binding; }

			Buffer getBuffer() { return buf; }
			u32 size() { return buf.size(); }

		private:

			u32 width, height, length;
			TextureType type;
			TextureFormat format;
			TextureLayout layout;
			u32 binding;
			Buffer buf;
		};

	}

}