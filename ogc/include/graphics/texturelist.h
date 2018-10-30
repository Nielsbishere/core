#pragma once
#include "texture.h"

namespace oi {

	namespace gc {

		class TextureList;

		struct TextureListInfo {

			typedef TextureList ResourceType;

			std::vector<Texture*> textures;

			TextureListInfo(u32 count) : textures(count) {
				memset(textures.data(), 0, sizeof(Texture*) * textures.size());
			}

			TextureListInfo() {}

		};

		typedef u32 TextureHandle;

		class TextureList : public GraphicsResource {

			friend class Graphics;

		public:

			Texture *get(TextureHandle i);
			void set(TextureHandle i, Texture *tex);

			TextureHandle alloc(Texture *tex);
			void dealloc(Texture *tex);

			u32 size();

			const TextureListInfo getInfo() { return info; }

		protected:

			~TextureList() {}
			TextureList(TextureListInfo info) : info(info) {}
			bool init() { return true; }

		private:

			TextureListInfo info;

		};

	}

}