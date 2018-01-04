#pragma once

#include "Grid.h"
#include "Color.h"
#include "API/stbi/stbi_load.h"
#include "API/stbi/stbi_write.h"
#include "Resource.h"
#include "Vector.h"
#include "Thread.h"

namespace oi {

	///Texture performance is pretty bad!
	///Fix that

	template<typename T, u32 d>
	class TTextureBase;

	template<typename T>
	struct TextureHelper {

		template<u32 d>
		static bool load(TTextureBase<T, d> &t, const OString &path) {
			Log::throwError<TTextureBase<T, d>, 0x0>("TTextureBase<T,d>::load is a function that is only implemented for TTextureBase<R(G)(B)(A)(f/d),2>");
			return false;
		}

		template<u32 d>
		static bool write(TTextureBase<T, d> &t, const OString &path) {
			Log::throwError<TTextureBase<T, d>, 0x0>("TTextureBase<T,d>::write is a function that is only implemented for TTextureBase<R(G)(B)(A)(f/d),2>");
			return false;
		}

		static bool load(TTextureBase<T, 2> &t, const OString &path) {
			
			if (!std::is_base_of<TColorBase, T>::value) {
				Log::throwError<TTextureBase<T, 2>, 0x1>("TTextureBase<T,2>::load is a function that is only implemented for TTextureBase<R(G)(B)(A)(f/d),2>");
				return false;
			}

			u32 rcomp = ColorHelper::getChannels(t.values[0]);

			int x, y, comp;
			u8 *buf = (u8*)stbi_load(path.c_str(), &x, &y, &comp, (int)rcomp);

			if(buf == nullptr) {
				Log::throwError<TTextureBase<T, 2>, 0x2>(OString("TTextureBase<T,2>::load couldn't load the following file: ") + path);
				return false;
			}

			u32 w = (u32)x, h = (u32)y;

			bool isCompressed = ColorHelper::hasType<u8>(t[0]);

			t.values = std::move(TGrid<T, 2>(T(), { w, h }, false));

			if (!isCompressed) {
				for (u32 j = 0; j < h; ++j)
					for (u32 i = 0; i < w; ++i)
						for (u32 k = 0; k < rcomp; ++k)
							t[j * w + i][k] = buf[(j * w + i) * rcomp + k];
			} else 
				memcpy(t.values.arr, buf, (size_t)(w * h * rcomp));

			stbi_image_free(buf);
			return true;
		}

		static bool write(TTextureBase<T, 2> &t, const OString &path) {

			if (!std::is_base_of<TColorBase, T>::value) {
				Log::throwError<TTextureBase<T, 2>, 0x3>("TTextureBase<T,2>::write is a function that is only implemented for TTextureBase<R(G)(B)(A),2>");
				return false;
			}

			u32 rcomp = ColorHelper::getChannels(t.values[0]);
			bool isCompressed = ColorHelper::hasType<u8>(t[0]);

			int w = (int)t.getDimension(0), h = (int)t.getDimension(1), comp = (int)rcomp;

			if (!isCompressed) {

				if(path.endsWithIgnoreCase(".hdr") && ColorHelper::hasType<f32>(t[0]))
					return stbi_write_hdr(path.c_str(), w, h, comp, (const float*)t.values.arr);

				Log::throwError<TTextureBase<T, 2>, 0x4>("TTextureBase<T,2>::write is a function that is only implemented for TTextureBase<R(G)(B)(A),2>. Writing float/double to generic image types is not recommended, as it will destroy the image data. The only exception is when you write a .hdr file, which uses floats.");
				return false;
			}

			if (path.endsWithIgnoreCase(".png"))
				return stbi_write_png(path.c_str(), w, h, comp, t.values.arr, comp * w);
			else if (path.endsWithIgnoreCase(".bmp"))
				return stbi_write_bmp(path.c_str(), w, h, comp, t.values.arr);
			else if (path.endsWithIgnoreCase(".tga"))
				return stbi_write_tga(path.c_str(), w, h, comp, t.values.arr);

			return false;
		}
	};

	template<typename T, u32 d>
	class TTexture;

	template<typename T, u32 d = 2>
	class TTextureBase : public Resource {

		friend struct TextureHelper<T>;

	public:

		TTextureBase() {}
		TTextureBase(TGrid<T, d> &_values) : values(_values) { }

		T &operator[](u32 wh) { return values[wh]; }
		T &operator[](TVec<u32, d> wh) { return values[wh.asIndex(values.getDimensions())]; }

		const T &get(u32 wh) const { return values.get(wh); }
		const T &get(TVec<u32, d> wh) const { return values.get(wh.asIndex(values.getDimensions())); }

		u32 getSize() { return values.getSize(); }
		u32 getBufferSize() { return getSize() * sizeof(T); }

		u32 getDimension(u32 wh) const { return values.getDimension(wh); }
		TVec<u32, d> getDimensions() const { return values.getDimensions(); }

		bool load(const OString &path) override {
			return TextureHelper<T>::load(*this, path);
		}

		bool write(const OString &path) override {
			return TextureHelper<T>::write(*this, path);
		}

		void runCPUShader(std::function< T (TTexture<T, d> *, TVec<u32, d> ) > f) {

			u32 thrs = Thread::cores();

			u32 size = values.getSize();
			u32 perCore = size / thrs;
			u32 remainder = size % thrs;

			Thread::foreachCore([thrs, perCore, remainder, this, f](u32 x) -> void {
				u32 execute = perCore;
				if (x == thrs - 1) execute += remainder;

				u32 start = x * perCore;
				u32 end = start + execute;

				for (u32 i = start; i < end; ++i)
					(*this)[i] = f((TTexture<T, d>*)this, this->getDimensions().fromIndex(i));
			});
		}

	protected:

		TGrid<T, d> values;
	};

	template<typename T, u32 d = 2>
	class TTexture : public TTextureBase<T, d> {};

	template<typename T>
	class TTexture<T, 1> : public TTextureBase<T, 1> {

	public:

		u32 getWidth() { return getDimension(0); }

		TTexture() : TTexture(0) {}
		TTexture(u32 width) : TTextureBase<T, 1>(TGrid<T, 1>(T(), TVec<u32, 1>(width))) {}
		TTexture(T def, u32 width) : TTextureBase<T, 1>(TGrid<T, 1>(def, TVec<u32, 1>(width))) {}
	};

	template<typename T>
	class TTexture<T, 2> : public TTextureBase<T, 2> {

	public:

		u32 getWidth() { return getDimension(0); }
		u32 getHeight() { return getDimension(1); }

		TTexture() : TTexture(0, 0) {}
		TTexture(Vec2u v) : TTextureBase<T, 2>(TGrid<T, 2>(T(), v)) {}
		TTexture(T def, Vec2u v) : TTextureBase<T, 2>(TGrid<T, 2>(def, v)) { }
	};

	template<typename T>
	class TTexture<T, 3> : public TTextureBase<T, 3> {

	public:

		u32 getWidth() { return getDimension(0); }
		u32 getHeight() { return getDimension(1); }
		u32 getLength() { return getDimension(2); }

		TTexture() : TTexture(0, 0, 0) {}
		TTexture(Vec3u v) : TTextureBase<T, 3>(TGrid<T, 3>(T(), v)) {}
		TTexture(T def, Vec3u v) : TTextureBase<T, 3>(TGrid<T, 3>(def, v)) {}
	};

	template<typename T>
	using Texture1D = TTexture<T, 1>;

	template<typename T>
	using Texture2D = TTexture<T, 2>;

	template<typename T>
	using Texture3D = TTexture<T, 3>;
}