#pragma once

#include "graphics/graphics.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class PipelineState;

		struct PipelineStateInfo {

			typedef PipelineState ResourceType;

			f32 lineWidth;
			CullMode cullMode;
			WindMode windMode;
			u32 samples;
			BlendMode blendMode;
			DepthMode depthMode;

			PipelineStateInfo(DepthMode depthMode = DepthMode::All, BlendMode blendMode = BlendMode::Alpha, CullMode cullMode = CullMode::Back, WindMode windMode = WindMode::CCW, f32 lineWidth = 1.f, u32 samples = 1) : depthMode(depthMode), blendMode(blendMode), samples(samples), cullMode(cullMode), windMode(windMode), lineWidth(lineWidth) {}

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