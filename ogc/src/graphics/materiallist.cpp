#include "graphics/materiallist.h"
#include "graphics/graphics.h"
#include "graphics/gbuffer.h"
using namespace oi;
using namespace gc;

MaterialHandle MaterialList::alloc(MaterialStruct info) {

	for (MaterialHandle i = 0; i < getSize(); ++i)
		if (operator[](i).id == u32_MAX) {
			info.id = i;
			operator[](i) = info;
			return i;
		}

	Log::error("Couldn't allocate material");
	return getSize();
}

bool MaterialList::dealloc(MaterialHandle handle) {
	
	if (handle >= getSize())
		return Log::error("Couldn't deallocate material");

	operator[](handle).id = u32_MAX;
	return true;

}

void MaterialList::update() {
	getBuffer()->set(Buffer::construct((u8*) info.materials.data(), getBufferSize()));
}

MaterialStruct &MaterialList::operator[](MaterialHandle handle) { return info.materials[handle]; }
const MaterialStruct MaterialList::operator[](MaterialHandle handle) const { return info.materials[handle]; }

u32 MaterialList::getSize() const { return (u32) info.materials.size(); }
u32 MaterialList::getBufferSize() const { return getSize() * (u32) sizeof(MaterialStruct); }

const MaterialListInfo MaterialList::getInfo() const { return info; }
GBuffer *MaterialList::getBuffer() const { return info.buffer; }

MaterialList::~MaterialList() { g->destroy(info.buffer); }
MaterialList::MaterialList(MaterialListInfo info) : info(info) {}

bool MaterialList::init() {
	info.buffer = g->create(getName() + " GBuffer", GBufferInfo(GBufferType::SSBO, getBufferSize()));
	return true;
}