#pragma once
#include <Template/Enum.h>

namespace oi {

	namespace gc {

		DEnum(SamplerWrapping, u32, Repeat = 0, MirrorRepeat = 1, ClampEdge = 2, ClampBorder = 3, MirrorClampEdge = 4);
		DEnum(SamplerMin, u32, LinearMip = 0, NearestMip = 1, Linear = 2, Nearest = 3, LinearMipNearest = 4, NearestMipLinear = 5);
		DEnum(SamplerMag, u32, Linear = 0, Nearest = 1);

		class SamplerInfo {

		public:

			SamplerInfo(SamplerWrapping _s, SamplerMin _min, SamplerMag _mag, u32 _binding = 0, bool anisotropy = true) : s(_s), r(_s), t(_s), minFilter(_min), magFilter(_mag), aniso(anisotropy), binding(_binding) {}
			SamplerInfo(SamplerWrapping _s, SamplerWrapping _r, SamplerMin _min, SamplerMag _mag, u32 _binding = 0, bool anisotropy = true) : s(_s), r(_r), minFilter(_min), magFilter(_mag), aniso(anisotropy), binding(_binding) {}
			SamplerInfo(SamplerWrapping _s, SamplerWrapping _r, SamplerWrapping _t, SamplerMin _min, SamplerMag _mag, u32 _binding = 0, bool anisotropy = true) : s(_s), r(_r), t(_t), minFilter(_min), magFilter(_mag), aniso(anisotropy), binding(_binding) {}

			SamplerWrapping getU() { return s; }
			SamplerWrapping getV() { return r; }
			SamplerWrapping getW() { return t; }

			SamplerMin getMinification() { return minFilter; }
			SamplerMag getMagnification() { return magFilter; }
			bool hasAnisotropy() { return aniso; }

			u32 getBinding() { return binding; }
			void setBinding(u32 _binding) { binding = _binding; }

		private:

			SamplerWrapping s, r, t;
			SamplerMin minFilter;
			SamplerMag magFilter;
			bool aniso;
			u32 binding;
		};
	}

}