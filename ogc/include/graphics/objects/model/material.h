#pragma once
#include "types/vector.h"
#include "graphics/objects/texture/texturelist.h"

namespace oi {

	namespace gc {

		typedef u32 MaterialHandle;
		class Material;
		class MaterialList;

		struct MaterialStruct {

			Vec3 diffuse = 1.f;
			MaterialHandle id = u32_MAX;

			Vec3 ambient = 0.1f;
			f32 shininess = 0.f;

			Vec3 emissive = 0.f;
			f32 shininessExponent = 1.f;

			Vec3 specular = 1.f;
			f32 roughness = 0.5f;

			f32 metallic = 0.5f;
			f32 transparency = 0.f;
			f32 clearcoat = 0.5f;
			f32 clearcoatGloss = 0.5f;

			f32 reflectiveness = 0.f;
			f32 sheen = 0.f;
			TextureHandle t_diffuse = 0;			//sRGB8 (3 Bpp)
			TextureHandle t_opacity = 0;			//R8 (1 Bpp)

			TextureHandle t_emissive = 0;			//RGB16 (6 Bpp)
			TextureHandle t_roughness = 0;			//R8 (1 Bpp)
			TextureHandle t_ao = 0;					//R8 (1 Bpp)
			TextureHandle t_height = 0;				//R8 (1 Bpp)

			TextureHandle t_metallic = 0;			//R8 (1 Bpp); Metallic
			TextureHandle t_normal = 0;				//RGB8s (3 Bpp)
			TextureHandle t_specular = 0;			//R8 (1 Bpp)
			u32 p0 = 0;

		};

		struct MaterialInfo {

			typedef Material ResourceType;

			MaterialStruct temp;

			MaterialList *parent;
			MaterialStruct *ptr;

			MaterialInfo(MaterialList *parent) : parent(parent), ptr(&temp) {}

			MaterialStruct *operator->() { return ptr; }
			const MaterialStruct *operator->() const { return ptr; }

		};

		class Material : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			const MaterialInfo &getInfo() const;

			MaterialHandle getHandle() const;
			MaterialList *getParent() const;

			void setDiffuse(Vec3 dif);
			void setAmbient(Vec3 amb);
			void setShininess(f32 shn);
			void setEmissive(Vec3 emi);
			void setShininessExponent(f32 sne);
			void setSpecular(Vec3 spc);
			void setRoughness(f32 rgh);
			void setMetallic(f32 met);
			void setTransparency(f32 trn);
			void setClearcoat(f32 clc);
			void setClearcoatGloss(f32 clg);
			void setReflectiveness(f32 rfn);
			void setSheen(f32 shn);

			void setDiffuse(Texture *difTex);
			void setOpacity(Texture *opcTex);
			void setEmissive(Texture *emiTex);
			void setRoughness(Texture *rghTex);
			void setAmbientOcclusion(Texture *aocTex);
			void setHeight(Texture *hghTex);
			void setMetallic(Texture *metTex);
			void setNormal(Texture *nrmTex);
			void setSpecular(Texture *spcTex);

		protected:

			~Material();
			Material(MaterialInfo info);
			bool init();

			void setTex(TextureHandle &texHandle, Texture *tex);
			void notify();

		private:

			MaterialInfo info;

		};

	}

}