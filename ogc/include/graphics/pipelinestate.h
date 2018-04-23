#pragma once

#include "graphics.h"

namespace oi {

	namespace gc {

		struct PipelineStateInfo {

			TopologyMode topologyMode;
			FillMode fillMode;
			f32 lineWidth = 1.f;
			CullMode cullMode = CullMode::Back;
			WindMode windMode = WindMode::CW;
			u32 samples = 1U;
			BlendMode blendMode = BlendMode::Alpha;
			DepthMode depthMode;

			PipelineStateInfo(TopologyMode topologyMode = TopologyMode::Triangle, FillMode fillMode = FillMode::Fill, DepthMode depthMode = DepthMode::All) : topologyMode(topologyMode), fillMode(fillMode), depthMode(depthMode) {}

		};

		class PipelineState {

			friend class Graphics;

		public:

			~PipelineState();

			PipelineStateExt &getExtension();
			
		protected:

			PipelineState(PipelineStateInfo info);
			bool init(Graphics *g);

		private:

			PipelineStateInfo info;
			Graphics *g = nullptr;

			PipelineStateExt ext;

		};

	}

}