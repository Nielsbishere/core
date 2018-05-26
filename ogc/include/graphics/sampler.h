#pragma once

#include <template/enum.h>
#include "gl/generic.h"
#include "graphicsresource.h"

namespace oi {

	namespace gc {

		UEnum(SamplerWrapping, Repeat = 0, MirrorRepeat = 1, ClampEdge = 2, ClampBorder = 3, MirrorClampEdge = 4);
		UEnum(SamplerMin, LinearMip = 0, NearestMip = 1, Linear = 2, Nearest = 3, LinearMipNearest = 4, NearestMipLinear = 5);
		UEnum(SamplerMag, Linear = 0, Nearest = 1);

		struct SamplerInfo {

			SamplerWrapping s, r, t;
			SamplerMin minFilter;
			SamplerMag magFilter;
			u32 aniso = 16U;

			SamplerInfo() {}
			SamplerInfo(SamplerMin minFilter, SamplerMag magFilter, SamplerWrapping srt, u32 aniso = 16U) : minFilter(minFilter), magFilter(magFilter), s(srt), r(srt), t(srt), aniso(aniso) {}
			SamplerInfo(SamplerMin minFilter, SamplerMag magFilter, SamplerWrapping s, SamplerWrapping r, SamplerWrapping t, u32 aniso = 16U) : minFilter(minFilter), magFilter(magFilter), s(s), r(r), t(t), aniso(aniso) {}

		};

		class Sampler : public GraphicsResource {

			friend class Graphics;

		public:

			const SamplerInfo getInfo();
			SamplerExt &getExtension();

		protected:

			~Sampler();
			Sampler(SamplerInfo info);
			bool init();

		private:

			SamplerInfo info;
			SamplerExt ext;

		};


	}

}