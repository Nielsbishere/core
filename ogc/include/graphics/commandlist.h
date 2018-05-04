#pragma once

#include <types/vector.h>
#include "graphics/gl/generic.h"
#include "graphics/graphicsobject.h"

namespace oi {

	namespace gc {

		class Graphics;
		class RenderTarget;
		class Pipeline;
		class GBuffer;

		struct CommandListInfo { };

		struct RenderTargetClear {

			Vec4d colorClear;
			f32 depthClear;
			u32 stencilClear;

			RenderTargetClear(Vec4d color = {}, f32 depth = 1.f, u32 stencil = 0U) : colorClear(color), depthClear(depth), stencilClear(stencil) {}

		};

		class CommandList : public GraphicsObject {

			friend class Graphics;

		public:

			void begin();
			void begin(RenderTarget *target, RenderTargetClear clear = {});

			void end();
			void end(RenderTarget *target);

			void bind(Pipeline *pipeline);
			bool bind(std::vector<GBuffer*> buffers);
			void draw(u32 vertices, u32 instances = 1U, u32 startVertex = 0U, u32 startInstance = 0U);
			void drawIndexed(u32 indices, u32 instances = 1U, u32 startIndex = 0U, u32 startVertex = 0U, u32 startInstance = 0U);

			CommandListExt &getExtension();

			void setViewport(Vec2 size, Vec2 offset = {}, f32 startDepth = 0.f, f32 endDepth = 1.f);

		protected:

			~CommandList();
			CommandList(CommandListInfo info);
			bool init();

		private:

			CommandListInfo info;
			CommandListExt ext;


		};

	}

}