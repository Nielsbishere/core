#pragma once

#include "Generic.h"
#include "Buffer.h"
#include "Template/TemplateFuncs.h"

namespace oi {

	//All color types; EXTENDED is CPU only and therefore isn't typedeffed
	enum ColorType {
		COMPRESSED_R, NORMAL_R, EXTENDED_R,
		COMPRESSED_RG, NORMAL_RG, EXTENDED_RG,
		COMPRESSED_RGB, NORMAL_RGB, EXTENDED_RGB,
		COMPRESSED_RGBA, NORMAL_RGBA, EXTENDED_RGBA
	};

	template<typename T> struct TColorType {
		T c;
		TColorType() : c(0) {}
	};

	template<u32 i> struct TColorChannel { };

	template<> struct TColorChannel<0> : TColorType<u8> {
		OString toString() { return OString::getHex(c); }

		template<typename T2>
		TColorChannel &operator=(T2 other) { c = (u8)other; return *this; }

		operator u8() { return c; }

		void inverse() { c = (u8)255 - c;  }
	};

	template<> struct TColorChannel<1> : TColorType<f32> {
		OString toString() { return OString(c) + "f"; }

		template<typename T2>
		TColorChannel &operator=(T2 other) { c = (f32)other; return *this; }

		TColorChannel &operator=(u8 b) { c = b / 255.f; return *this; }

		operator f32() { return c; }
	};

	template<> struct TColorChannel<2> : TColorType<f64> {
		OString toString() { return OString((f32)c); }

		template<typename T2>
		TColorChannel &operator=(T2 other) { c = (f64)other; return *this; }

		TColorChannel &operator=(u8 b) { c = b / 255.0; return *this; }

		operator f64() { return c; }
	};

	template<ColorType T>
	using ColorChannel = TColorChannel<u32(T) % 3>;

	template<ColorType T>
	constexpr u32 ColorsPerChannel = u32(T) / 3 + 1;

	struct TColorBase {};

	template<ColorType CT>
	class TColor : TColorBase {

		friend struct ColorHelper;

	public:

		TColor() {}

		template<typename T, typename ...args>
		TColor(T t, args... arg) {
			FillArray::run(contents, 0, t, arg...);
		}


		ColorChannel<CT> &operator[](u32 where) {
			return contents[where];
		}

		operator OString() {

			OString result;

			for (u32 i = 0; i < ColorsPerChannel<CT>; ++i)
				result += contents[i].toString() + " ";

			return result;
		}

		TColor operator~() const {

			TColor other = *this;

			for (u32 i = 0; i < ColorsPerChannel<CT> && i < 3; ++i)
				other.contents[i].inverse();

			return other;
		}


	private:

		ColorChannel<CT> contents[ColorsPerChannel<CT>];
	};

	struct ColorHelper {

		template<ColorType CT>
		static ColorType getType(const TColor<CT> &color) {
			return CT;
		}

		template<ColorType CT>
		static u32 getChannels(const TColor<CT> &color) {
			return ColorsPerChannel<CT>;
		}

		static u32 sizeOf(ColorType CT) {
			return (u32(CT) / 3 + 1) * (u32(CT) % 3 == 0 ? 1 : (u32(CT) % 3 == 1 ? 4 : 8));
		}

		template<ColorType CT>
		static ColorChannel<CT> getChannel(const TColor<CT> &color, u32 i) {
			return color.contents[i];
		}

		template<typename T, ColorType CT>
		static bool hasType(const TColor<CT> &color) {
			return typeid(color.contents[0].c) == typeid(T);
		}
	};

	typedef TColor<COMPRESSED_RGBA> RGBA;
	typedef TColor<NORMAL_RGBA> RGBAf;

	typedef TColor<COMPRESSED_RGB> RGB;
	typedef TColor<NORMAL_RGB> RGBf;

	typedef TColor<COMPRESSED_RG> RG;
	typedef TColor<NORMAL_RG> RGf;

	typedef TColor<COMPRESSED_R> Red;
	typedef TColor<NORMAL_R> Redf;
}