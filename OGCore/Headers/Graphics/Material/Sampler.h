#pragma once

#include "SamplerInfo.h"
#include "Graphics/GPU/GraphicsResource.h"
#include <Types/Buffer.h>

namespace oi {

	namespace gc {

		class Sampler : public GraphicsResource {


		public:

			Sampler(SamplerInfo ti) : info(ti) {}
			virtual ~Sampler() {}

			SamplerInfo getInfo() { return info; }

		protected:

			SamplerInfo info;

		};

	}

}