#pragma once

#include "graphics/graphics.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class PipelineState;

		DEnum(TopologyMode, u32,
			Points = 0, Line = 1, Line_strip = 2, Triangle = 3, Triangle_strip = 4, Triangle_fan = 5,
			Line_adj = 6, Line_strip_adj = 7, Triangle_adj = 8, Triangle_strip_adj = 9, Undefined = 255
		);

		DEnum(FillMode, u32, Fill = 0, Line = 1, Point = 2, Undefined = 255);
		DEnum(CullMode, u32, None = 0, Back = 1, Front = 2);
		DEnum(WindMode, u32, CCW = 0, CW = 1);
		DEnum(DepthMode, u32, None = 0, Depth_test = 1, Depth_write = 2, All = 3);
		DEnum(BlendMode, u32, Off = 0, Alpha = 1, Add = 2, Subtract = 3);

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
			friend class oi::BlockAllocator;

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