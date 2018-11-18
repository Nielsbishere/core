#pragma once

#include "types/vector.h"
#include "types/bitset.h"
#include "graphics/objects/texture/texturelist.h"

namespace oi {

	namespace gc {

		class GBuffer;

		class Material;
		struct MaterialStruct;
		class MaterialList;

		//The data for a material buffer
		struct MaterialListInfo {

			typedef MaterialList ResourceType;

			TextureList *textures;

			bool notified = false;
			u32 size;

			GBuffer *buffer = nullptr;

			MaterialListInfo(TextureList *textures, u32 maxCount) : textures(textures), size(maxCount) { }
			MaterialListInfo(): textures(nullptr) {}

		};

		class MaterialList : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;
			friend class Material;

		public:

			MaterialStruct *alloc(MaterialStruct info);
			bool dealloc(MaterialStruct *ptr);
			void flush();

			MaterialStruct *operator[](MaterialHandle handle);
			const MaterialStruct *operator[](MaterialHandle handle) const;

			u32 getSize() const;
			u32 getBufferSize() const;
			GBuffer *getBuffer() const;

			const MaterialListInfo getInfo() const;

		protected:

			~MaterialList();
			MaterialList(MaterialListInfo info);
			bool init();

			void notify();

		private:

			MaterialListInfo info;

		};

	}

}