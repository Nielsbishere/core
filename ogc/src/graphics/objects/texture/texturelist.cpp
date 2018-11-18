#include "graphics/objects/texture/texturelist.h"
using namespace oi::gc;

Texture *TextureList::get(TextureHandle i) { return info.textures[i]; }
u32 TextureList::size() { return (u32) info.textures.size(); }

TextureHandle TextureList::alloc(Texture *tex) {
	TextureHandle j = size();
	for (TextureHandle i = 0; i < j; ++i)
		if (get(i) == nullptr) {
			info.textures[i] = tex;
			return i;
		}
	return j;
}

void TextureList::dealloc(Texture *tex) {
	for (TextureHandle i = 0, j = size(); i < j; ++i)
		if (get(i) == tex)
			info.textures[i] = nullptr;
}