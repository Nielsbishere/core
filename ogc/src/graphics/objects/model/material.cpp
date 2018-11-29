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

void Material::setDiffuse(Texture *tex) { setTex(info.ptr->t_diffuse, tex); }
void Material::setOpacity(Texture *tex) { setTex(info.ptr->t_opacity, tex); }
void Material::setEmissive(Texture *tex) { setTex(info.ptr->t_emissive, tex); }
void Material::setRoughness(Texture *tex) { setTex(info.ptr->t_roughness, tex); }
void Material::setAmbientOcclusion(Texture *tex) { setTex(info.ptr->t_ao, tex); }
void Material::setHeight(Texture *tex) { setTex(info.ptr->t_height, tex); }
void Material::setMetallic(Texture *tex) { setTex(info.ptr->t_metallic, tex); }
void Material::setNormal(Texture *tex) { setTex(info.ptr->t_normal, tex); }
void Material::setSpecular(Texture *tex) { setTex(info.ptr->t_specular, tex); }

Material::~Material() { info.parent->dealloc(info.ptr); }
Material::Material(MaterialInfo info) : info(info) {}
bool Material::init() {
	info.ptr = info.parent->alloc(info.temp);
	return true;
}

void Material::setTex(TextureHandle &texHandle, Texture *tex) {

	if (tex->getInfo().parent != info.parent->getInfo().textures)
		Log::throwError<Material, 0x0>("Texture should be in same TextureList used in MaterialList");

	texHandle = tex->getHandle();
	notify();

}

void Material::notify() {
	if(info.ptr != &info.temp)
		info.parent->notify(info.ptr);
}