#include "graphics/graphics.h"
#include "graphics/objects/model/material.h"
#include "graphics/objects/model/materiallist.h"
#include "graphics/objects/texture/texture.h"
using namespace oi::gc;
using namespace oi;

MaterialHandle Material::getHandle() const { return info.ptr->id; }
const MaterialInfo &Material::getInfo() const { return info; }
MaterialList *Material::getParent() const { return info.parent; }

void Material::setDiffuse(Vec3 val){ info.ptr->diffuse = val; notify(); }
void Material::setEmissive(Vec3 val){ info.ptr->emissive = val; notify(); }
void Material::setRoughness(f32 val){ info.ptr->roughness = val; notify(); }
void Material::setMetallic(f32 val){ info.ptr->metallic = val; notify(); }
void Material::setOpacity(f32 val){ info.ptr->opacity = val; notify(); }
void Material::setReflectiveColor(Vec3 val){ info.ptr->reflective = val; notify(); }

void Material::setDiffuse(Texture *tex) { setTex(tex, MaterialTextureType::DIFFUSE); }
void Material::setOpacity(Texture *tex) { setTex(tex, MaterialTextureType::OPACITY); }
void Material::setEmissive(Texture *tex) { setTex(tex, MaterialTextureType::EMISSIVE); }
void Material::setRoughness(Texture *tex) { setTex(tex, MaterialTextureType::ROUGHNESS); }
void Material::setAmbientOcclusion(Texture *tex) { setTex(tex, MaterialTextureType::AMBIENT_OCCLUSION); }
void Material::setHeight(Texture *tex) { setTex(tex, MaterialTextureType::HEIGHT); }
void Material::setMetallic(Texture *tex) { setTex(tex, MaterialTextureType::METALLIC); }
void Material::setNormal(Texture *tex) { setTex(tex, MaterialTextureType::NORMAL); }

Material::~Material() {

	for (u32 i = 0, j = (u32)MaterialTextureType::LENGTH; i < j; ++i)
		if (info.ptr->textureFlags & (1 << i))
			setTex(nullptr, (MaterialTextureType)i);

	info.parent->dealloc(info.ptr);
	g->destroy(info.parent);
}

Material::Material(MaterialInfo info) : info(info) {}
bool Material::init() {
	info.ptr = info.parent->alloc(info.temp);
	g->use(info.parent);
	return true;
}

void Material::setTex(Texture *tex, MaterialTextureType type) {

	const u32 flag = 1 << (u32)type;
	TextureHandle &texHandle = *(&info.ptr->t_diffuse + (u32)type);

	const bool occupied = (info.ptr->textureFlags & flag) != 0;

	if (tex == nullptr) {

		if (!occupied)
			return;

		g->destroyObject(info.parent->getInfo().textures->get(texHandle));
		info.ptr->textureFlags ^= flag;
		texHandle = 0;
		notify();
		return;
	}

	if (tex->getInfo().parent != info.parent->getInfo().textures)
		Log::throwError<Material, 0x0>("Texture should be in same TextureList used in MaterialList");

	if (texHandle == tex->getHandle())
		return;

	if (occupied)
		g->destroyObject(info.parent->getInfo().textures->get(texHandle));
	else
		info.ptr->textureFlags |= flag;

	texHandle = tex->getHandle();
	g->use(tex);
	notify();

}

void Material::notify() {
	if(info.ptr != &info.temp)
		info.parent->notify(info.ptr);
}