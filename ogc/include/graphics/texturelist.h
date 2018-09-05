#pragma once
#include "texture.h"

namespace oi {

	namespace gc {

		struct TextureListInfo {

			std::vector<Texture*> textures;

			TextureListInfo(u32 count) : textures(count) {
				memset(textures.data(), 0, sizeof(Texture*) * textures.size());
			}

			TextureListInfo() {}

		};

		class TextureList : public GraphicsResource {

			friend class Graphics;

		public:

			Texture *get(u32 i);
			void set(u32 i, Texture *tex);

			u32 alloc(Texture *tex);
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