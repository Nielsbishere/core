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
void Material::setAmbient(Vec3 val){ info.ptr->ambient = val; notify(); }
void Material::setShininess(f32 val){ info.ptr->shininess = val; notify(); }
void Material::setEmissive(Vec3 val){ info.ptr->emissive = val; notify(); }
void Material::setShininessExponent(f32 val){ info.ptr->shininessExponent = val; notify(); }
void Material::setSpecular(Vec3 val){ info.ptr->specular = val; notify(); }
void Material::setRoughness(f32 val){ info.ptr->roughness = val; notify(); }
void Material::setMetallic(f32 val){ info.ptr->metallic = val; notify(); }
void Material::setTransparency(f32 val){ info.ptr->transparency = val; notify(); }
void Material::setClearcoat(f32 val){ info.ptr->clearcoat = val; notify(); }
void Material::setClearcoatGloss(f32 val){ info.ptr->clearcoatGloss = val; notify(); }
void Material::setReflectiveness(f32 val) { info.ptr->reflectiveness = val; notify(); }
void Material::setSheen(f32 val) { info.ptr->sheen = val; notify(); }

void Material::setDiffuse(Texture *tex) { setTex(tex, MaterialTextureType::DIFFUSE); }
void Material::setOpacity(Texture *tex) { setTex(tex, MaterialTextureType::OPACITY); }
void Material::setEmissive(Texture *tex) { setTex(tex, MaterialTextureType::EMISSIVE); }
void Material::setRoughness(Texture *tex) { setTex(tex, MaterialTextureType::ROUGHNESS); }
void Material::setAmbientOcclusion(Texture *tex) { setTex(tex, MaterialTextureType::AMBIENT_OCCLUSION); }
void Material::setHeight(Texture *tex) { setTex(tex, MaterialTextureType::HEIGHT); }
void Material::setMetallic(Texture *tex) { setTex(tex, MaterialTextureType::METALLIC); }
void Material::setNormal(Texture *tex) { setTex(tex, MaterialTextureType::NORMAL); }
void Material::setSpecular(Texture *tex) { setTex(tex, MaterialTextureType::SPECULAR); }

Material::~Material() {

	for (u32 i = 0, j = (u32)MaterialTextureType::LENGTH; i < j; ++i)
		if (info.usedTextures[i])
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

	TextureHandle &texHandle = *(&info.ptr->t_diffuse + (u32)type);

	if (tex == nullptr) {

		if (!info.usedTextures[(u32)type])
			return;

		g->destroyObject(info.parent->getInfo().textures->get(texHandle));
		info.usedTextures[(u32)type] = false;
		texHandle = 0;
		notify();
		return;
	}

	if (tex->getInfo().parent != info.parent->getInfo().textures)
		Log::throwError<Material, 0x0>("Texture should be in same TextureList used in MaterialList");

	if (texHandle == tex->getHandle())
		return;

	if (info.usedTextures[(u32)type])
		g->destroyObject(info.parent->getInfo().textures->get(texHandle));
	else
		info.usedTextures[(u32)type] = true;

	texHandle = tex->getHandle();
	g->use(tex);
	notify();

}

void Material::notify() {
	if(info.ptr != &info.temp)
		info.parent->notify(info.ptr);
}