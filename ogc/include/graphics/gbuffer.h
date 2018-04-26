#pragma once

#include "graphicsobject.h"
#include "gl/generic.h"

namespace oi {

	namespace gc {

		DEnum(GBufferType, u32, UBO = 0, SSBO = 1, IBO = 2, VBO = 3, CBO = 4);

		struct GBufferInfo {

			GBufferType type;
			u32 size;
			u8 *ptr = nullptr;
			bool persistent;

			//Empty gpu buffer
			GBufferInfo(GBufferType type, u32 size, bool persistent = false): type(type), size(size), persistent(persistent) {}

			//Filled gpu buffer
			GBufferInfo(GBufferType type, u32 size, u8 *ptr, bool persistent = false) : type(type), size(size), ptr(ptr), persistent(persistent) {}

		};

		class GBuffer : public GraphicsObject {

			friend class Graphics;

		public:

			GBufferType getType();
			u32 getSize();

			GBufferExt &getExtension();
			bool set(Buffer buf);

		protected:

			~GBuffer();
			GBuffer(GBufferInfo info);
			bool init();

		private:

			GBufferInfo info;
			GBufferExt ext;

		};

	}

}