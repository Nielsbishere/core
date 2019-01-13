#pragma once
#include "graphics/objects/graphicsresource.h"

namespace oi {

	namespace gc {

		class TextureObject;
		class TextureList;

		struct TextureListInfo {

			typedef TextureList ResourceType;

			std::vector<TextureObject*> textures;

			TextureListInfo(u32 count) : textures(count) {
				memset(textures.data(), 0, sizeof(TextureObject*) * textures.size());
			}

			TextureListInfo() {}

		};

		typedef u32 TextureHandle;

		class TextureList : public GraphicsResource {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			TextureObject *get(TextureHandle i) const;
			TextureHandle alloc(TextureObject *tex);
			void dealloc(TextureObject *tex);

			template<typename T>
			T *get(TextureHandle i) const;

			u32 size() const;

			const TextureListInfo &getInfo() const { return info; }

		protected:

			~TextureList() {}
			TextureList(TextureListInfo info) : info(info) {}
			bool init() { return true; }

		private:

			TextureListInfo info;

		};

		template<typename T>
		T *TextureList::get(TextureHandle i) const {

			static_assert(std::is_base_of<TextureObject, T>::value, "Texture::get<T>: T has to be a TextureObject (VersionedTexture or Texture)");

			return dynamic_cast<T*>(get(i));
		}

	}

}