#pragma once

#include <types/vector.h>
#include "graphics/gl/generic.h"

namespace oi {

	namespace gc {

		class Graphics;
		class RenderTarget;
		class Pipeline;

		struct CommandListInfo { };

		struct RenderTargetClear {

			Vec4d colorClear;
			f32 depthClear;
			u32 stencilClear;

			RenderTargetClear(Vec4d color = {}, f32 depth = 0.f, u32 stencil = 0U) : colorClear(color), depthClear(depth), stencilClear(stencil) {}

		};

		class CommandList {

			friend class Graphics;

		public:

			~CommandList();

			void begin();
			void begin(RenderTarget *target, RenderTargetClear clear = {});

			void end();
			void end(RenderTarget *target);

			void bind(Pipeline *pipeline);

			CommandListExt &getExtension();

			void setViewport(Vec2 size, Vec2 offset = {}, f32 startDepth = 0.f, f32 endDepth = 1.f);

		protected:

			CommandList(CommandListInfo info);
			bool init(Graphics *gl);

		private:

			Graphics *gl = nullptr;

			CommandListInfo info;
			CommandListExt ext;


		};

	}

}