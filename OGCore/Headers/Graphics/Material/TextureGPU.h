#pragma once

#include <Types/Color.h>
#include <Types/Vector.h>
#include "../GPU/BufferGPU.h"

namespace oi {

	namespace gc {

		class TextureGPU {

		public:

			TextureGPU(Vec2u dim, ColorType format, BufferGPU *resource): res(dim), gpuResource(resource) {}
			virtual ~TextureGPU() { delete gpuResource; }

			virtual bool init() = 0;

			//Returns the address on the GPU
			virtual u64 getHandle() = 0;

			virtual void bind() = 0;
			virtual void unbind() = 0;

			//Get part of the buffer (to read/write)
			//Length = 0; full buffer
			//subbuffer() returns full buffer
			Buffer subbuffer(u32 offset = 0, u32 length = 0) {
				return gpuResource->subbuffer(offset, length);
			}

			template<ColorType CT>
			TColor<CT> &operator[](u32 i) {
				return subbuffer(i * getBPP(), getBPP()).operator[]<TColor<CT>>(0);
			}

			template<ColorType CT>
			TColor<CT> &operator[](Vec2u loc) {
				return (*this)[loc.asIndex(dims)];
			}

			u32 getBPP() { return sizeof(ColorHelper::sizeOf(CT)); }
			Vec2u getResolution() { return res; }
			ColorType getFormat() { return CT; }

		protected:

			Vec2u res;
			ColorType CT;
			BufferGPU *gpuResource;
		};
	}

}