#pragma once

#include "texturelist.h"
#include <types/vector.h>

namespace oi {

	namespace gc {

		class GBuffer;

		typedef u32 MaterialHandle;

		//Material
		//112 bytes; ~9362 material (structs) per MiB
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
			TextureHandle t_opacitySpecular = 0;	//RG8 (2 Bpp)

			TextureHandle t_emissive = 0;			//RGB8 (3 Bpp)
			TextureHandle t_rahm = 0;				//RGBA8 (4 Bpp); Roughness, ao, height, metallic
			TextureHandle t_normal = 0;				//RGB8s (3 Bpp)
			u32 p0 = 0;

		};

		class MaterialList;

		//The data for a material buffer
		struct MaterialListInfo {

			typedef MaterialList ResourceType;

			TextureList *textures;
			std::vector<MaterialStruct> materials;
			GBuffer *buffer = nullptr;

			MaterialListInfo(TextureList *textures, u32 maxCount) : textures(textures), materials(maxCount) { }
			MaterialListInfo(): textures(nullptr) {}

		};

		//MaterialLists are managed by the user, the MaterialStructs should only contain valid texture handles, 
		//otherwise referencing them can cause major problems. These texture handles have to reference to a valid handle (non-nullptr) in the TextureList.
		class MaterialList : public GraphicsObject {

			friend class Graphics;

		public:

			MaterialHandle alloc(MaterialStruct info);
			bool dealloc(MaterialHandle handle);
			void update();

			MaterialStruct &operator[](MaterialHandle handle);
			const MaterialStruct operator[](MaterialHandle handle) const;

			u32 getSize() const;
			u32 getBufferSize() const;

			const MaterialListInfo getInfo() const;
			GBuffer *getBuffer() const;

		protected:

			~MaterialList();
			MaterialList(MaterialListInfo info);
			bool init();

		private:

			MaterialListInfo info;

		};

	}

}