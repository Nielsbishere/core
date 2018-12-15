#pragma once

#include "types/vector.h"
#include "graphics/gl/generic.h"
#include "graphics/objects/graphicsobject.h"

namespace oi {

	namespace gc {

		class Graphics;
		class RenderTarget;
		class Pipeline;
		class GBuffer;
		class MeshBuffer;
		class Mesh;
		class DrawList;
		class ComputeList;

		struct CommandListInfo { 

			typedef CommandList ResourceType; 

			CommandListInfo() {}
		
		};

		struct RenderTargetClear {

			Vec4d colorClear;
			f32 depthClear;
			u32 stencilClear;

			RenderTargetClear(Vec4d color = {}, f32 depth = 1.f, u32 stencil = 0U) : colorClear(color), depthClear(depth), stencilClear(stencil) {}

		};

		class CommandList : public GraphicsObject {

			friend class Graphics;
			friend class Texture;
			friend class oi::BlockAllocator;

		public:

			void begin();
			void begin(RenderTarget *target, RenderTargetClear clear = {});

			void end();
			void end(RenderTarget *target);

			void bind(Pipeline *pipeline);
			void draw(DrawList *drawList);

			void dispatch(ComputeList *computeList);

			CommandListExt &getExtension();

		protected:

			bool bind(std::vector<GBuffer*> vertices, GBuffer *indices = nullptr);
			bool bind(MeshBuffer *meshBuffer);

			~CommandList();
			CommandList(CommandListInfo info);
			bool init();

		private:

			CommandListInfo info;
			CommandListExt ext;
			MeshBuffer *boundMB = nullptr;

		};

	}

}