#pragma once
#include "types/vector.h"
#include "graphics/objects/texture/texturelist.h"

namespace oi {

	namespace gc {

		typedef u32 MaterialHandle;
		class Material;
		class MaterialList;
		class Texture;

		enum class MaterialTextureType {
			DIFFUSE = 0,
			OPACITY = 1,
			EMISSIVE = 2,
			ROUGHNESS = 3,
			AMBIENT_OCCLUSION = 4,
			HEIGHT = 5,
			METALLIC = 6,
			NORMAL = 7,
			LENGTH
		};

		//Stride 6 * 16 = 96
		struct MaterialStruct {

			Vec3 diffuse = 1.f;
			f32 opacity = 1.f;

			Vec3 reflective = 1.f;
			f32 roughness = 1.f;

			Vec3 emissive = 0.f;
			f32 metallic = 0.f;

			MaterialHandle id = u32_MAX;
			u32 textureFlags = 0;					//1 << MaterialTextureType
			TextureHandle t_diffuse = 0;			//sRGB8 (3 Bpp) or HDR RGB16f (6 Bpp)
			TextureHandle t_opacity = 0;			//R8 (1 Bpp)

			TextureHandle t_emissive = 0;			//RGB16 (6 Bpp)
			TextureHandle t_roughness = 0;			//R8 (1 Bpp)
			TextureHandle t_ao = 0;					//R8 (1 Bpp)
			TextureHandle t_height = 0;				//R8 (1 Bpp)

			Vec2u padding = 0;
			TextureHandle t_metallic = 0;			//R8 (1 Bpp)
			TextureHandle t_normal = 0;				//RGB8s (3 Bpp)

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
			void setEmissive(Vec3 emi);
			void setRoughness(f32 rgh);
			void setMetallic(f32 met);
			void setOpacity(f32 trn);
			void setReflectiveColor(Vec3 rfn);

			void setDiffuse(Texture *difTex);
			void setOpacity(Texture *opcTex);
			void setEmissive(Texture *emiTex);
			void setRoughness(Texture *rghTex);
			void setAmbientOcclusion(Texture *aocTex);
			void setHeight(Texture *hghTex);
			void setMetallic(Texture *metTex);
			void setNormal(Texture *nrmTex);

		protected:

			~Material();
			Material(MaterialInfo info);
			bool init();

			void setTex(Texture *tex, MaterialTextureType type);
			void notify();

		private:

			MaterialInfo info;

		};

	}

}