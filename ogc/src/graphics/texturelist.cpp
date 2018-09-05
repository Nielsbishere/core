#include "graphics/texturelist.h"
using namespace oi::gc;

Texture *TextureList::get(u32 i) { return info.textures[i]; }
u32 TextureList::size() { return (u32) info.textures.size(); }
void TextureList::set(u32 i, Texture *tex) { info.textures[i] = tex; }

u32 TextureList::alloc(Texture *tex) {
	u32 j = size();
	for (u32 i = 0; i < j; ++i)
		if (get(i) == nullptr) {
			set(i, tex);
			return i;
		}
	return j;
}

void TextureList::dealloc(Texture *tex) {
	for (u32 i = 0, j = size(); i < j; ++i)
		if (get(i) == tex)
			set(i, nullptr);
}