#pragma once

#include "SamplerInfo.h"
#include "Graphics/GPU/GraphicsResource.h"
#include <Types/Buffer.h>

namespace oi {

	namespace gc {

		class Sampler : public GraphicsResource {


		public:

			Sampler(SamplerInfo ti, u32 _binding) : info(ti), binding(_binding) {}
			virtual ~Sampler() {}

			SamplerInfo getInfo() { return info; }

			void setBinding(u32 _binding) { binding = _binding; }
			u32 getBinding() { return binding; }

		protected:

			SamplerInfo info;
			u32 binding;

		};

	}

}