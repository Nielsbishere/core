#include "graphics/graphics.h"
#include "graphics/objects/gbuffer.h"
#include "graphics/objects/model/material.h"
#include "graphics/objects/model/materiallist.h"
using namespace oi;
using namespace gc;

MaterialStruct *MaterialList::alloc(MaterialStruct minfo) {

	for (MaterialHandle i = 0; i < getSize(); ++i)
		if (operator[](i)->id == u32_MAX) {
			minfo.id = i;
			*operator[](i) = minfo;
			notify(operator[](i));
			return info.buffer->getBuffer().addr<MaterialStruct>() + i;
		}

	return (MaterialStruct*) Log::error("Couldn't allocate material");
}

bool MaterialList::dealloc(MaterialStruct *str) {
	
	size_t val = str - info.buffer->getBuffer().addr<MaterialStruct>();

	if (val >= getSize())
		return Log::error("Couldn't deallocate struct; out of bounds");

	operator[](MaterialHandle(val))->id = u32_MAX;
	return true;

}

void MaterialList::notify(MaterialStruct *material) {

	size_t dif = size_t((u8*)material - info.buffer->getAddress());

	if (dif >= info.buffer->getSize())
		return;

	u32 udif = (u32)dif;

	info.buffer->flush(Vec2u(udif, udif + (u32)sizeof(MaterialStruct)));

}

MaterialStruct *MaterialList::operator[](MaterialHandle handle) { return info.buffer->getBuffer().addr<MaterialStruct>() + (handle >= getSize() ? getSize() - 1 : handle); }
const MaterialStruct *MaterialList::operator[](MaterialHandle handle) const { return info.buffer->getBuffer().addr<MaterialStruct>() + (handle >= getSize() ? getSize() - 1 : handle); }

u32 MaterialList::getSize() const { return (u32) info.size; }
u32 MaterialList::getBufferSize() const { return getSize() * (u32) sizeof(MaterialStruct); }

const MaterialListInfo MaterialList::getInfo() const { return info; }
GBuffer *MaterialList::getBuffer() const { return info.buffer; }

MaterialList::~MaterialList() { g->destroy(info.buffer); }
MaterialList::MaterialList(MaterialListInfo info) : info(info) {}

bool MaterialList::init() {

	if (getSize() == 0)
		return Log::error("Material list max size can't be zero");

	info.buffer = g->create(getName() + " GBuffer", GBufferInfo(GBufferType::SSBO, getBufferSize()));

	for (u32 i = 0; i < getSize(); ++i)
		::new(info.buffer->getBuffer().addr<MaterialStruct>() + i) MaterialStruct();

	return true;
}