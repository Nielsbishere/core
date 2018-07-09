#pragma once

#include "graphics.h"
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		struct PipelineStateInfo {

			f32 lineWidth = 1.f;
			CullMode cullMode = CullMode::Back;
			WindMode windMode = WindMode::CCW;
			u32 samples = 1U;
			BlendMode blendMode = BlendMode::Alpha;
			DepthMode depthMode;

			PipelineStateInfo(DepthMode depthMode = DepthMode::All) : depthMode(depthMode) {}

		};

		class PipelineState : public GraphicsObject {

			friend class Graphics;

		public:

			PipelineStateExt &getExtension();
			const PipelineStateInfo getInfo();
			
		protected:

			~PipelineState();
			PipelineState(PipelineStateInfo info);
			bool init();

		private:

			PipelineStateInfo info;
			PipelineStateExt ext;

		};

	}

}