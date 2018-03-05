#pragma once

#include <Types/Generic.h>
#include <Types/GDataType.h>
#include <Types/Buffer.h>
#include <Types/Vector.h>
#include <Graphics/GPU/GraphicsResource.h>

namespace oi {

	namespace gc {
		class Graphics;
	}

	///All file types

	//RMHeader
	//@param u8[4] head; oiRM
	//@param u16 flags; determine the primitive, material list, materials and miscs
	//@param u32 vertices/indices; count for buffers
	//@param u16 layouts; how many layouts/shader inputs are stored in the VAO
	//@param u16 registers; how many registers there are (each register is like a 'radio button' where you define a material list)
	//						example: Sword with a gem and blade material; gem and blade are different register, because they can be set to different material lists (red gem & iron blade, green gem & gold blade, etc.).
	//@param u16 materialList; how many ids are stored
	//@param u16 materials; how many materials are stored
	//@param u16 defaultMaterial; if there is no per triangle material data (or points to invalid register or material list), this will be used
	//@param u16 miscs; the number of misc data for this model
	//@param u16 textures; How many textures are used
	//@param u16 strings; how many simple strings are stored (max 256 chars; 0-9A-Za-z & _ & SPACE), these are used for texture names and misc names
	//@optional u8[32] padding
	struct RMHeader {

		char head[4];
		u16 version, flags;

		u32 vertices, indices;

		u16 layouts, registers, materialList, materials;

		u16 defaultMaterial, miscs, textures, strings;

		u8 padding[32];

	};

	//RMMaterial
	//t_ references a texture handle
	struct RMMaterial {

		Vec3 ambient;
		f32 opacity;

		Vec3 emissive;
		f32 roughness;

		Vec3 diffuse;
		f32 specularScale;

		Vec3 specular;
		f32 specularPower;

		u16 t_ambient, t_emissive, t_diffuse, t_specular;

		u16 t_specularPower, t_normal, t_bump, t_opacity;

		f32 metallic;
		u16 t_roughness, t_metallic;

		u8 padding[40];
	};

	//Named layout with index
	struct RMLayout {
		u8 inputType;		//GDataType
		u8 index;
		u16 name;
	};

	//Stores misc data in a model
	//This could be a vertex where something spawns
	//or a primitive list that spawns something
	//@param u64 flags; first nibble (LE left) stores RMMiscType, second nibble stores 'usesMiscSpace' for now
	//					Misc space is space relative to the RMMiscType; so vertex, index or primitive
	//					Misc space is only available when tangent and/or normal have been set. If you displace along the X axis,
	//						you need the tangent, if you displace along Y axis, you need normal and any other axes need both (a.k.a. bitangent)
	//@param u16 count; number of RMMiscTypes in the array
	//@param u16 id; global id of the misc
	//@param u16 tangent; references the Layout that represents a tangent (index + 1), 0 references no tangent present or required
	//@param u16 normal; references the Layout that represents a normal (index + 1), 0 references no normal present or required
	//@param Vec3 displacement; the displacement in misc or model space
	//@param u16 name; a reference to the name of the object
	//@param u16 p0; padding
	struct RMMisc {

		u64 flags;

		u16 count, id;
		u16 tangent, normal;

		Vec3 displacement;
		u16 name, p0;

	};

	///CPU versions of the types

	using RMString = OString*;

	struct RMTexture {
		RMString name;
	};

	using RMTexturePtr = RMTexture*;

	struct RMMaterial_CPU {

		Vec3 ambient, emissive, diffuse, specular;

		f32 opacity, roughness, specularScale;
		f32 specularPower, metallic;

		RMTexturePtr t_ambient, t_emissive, t_diffuse, t_specular;
		RMTexturePtr t_specularPower, t_normal, t_bump, t_opacity;
		RMTexturePtr t_roughness, t_metallic;
	};

	using RMMaterialPtr = RMMaterial_CPU*;

	struct RMMaterialList {
		std::vector<RMMaterialPtr> materials;
	};

	using RMMaterialListPtr = RMMaterialList*;

	struct RMRegister {
		std::vector<RMMaterialListPtr> materialLists;
	};

	struct RMLayout_CPU {
		GDataType type;
		u32 index;
		RMString name;
	};

	using RMLayoutPtr = RMLayout_CPU*;

	DEnum(RMLayoutType, u32, Vertex = 0, Index = 1, Primitive = 2);

	struct RMMisc_CPU {

		RMLayoutType type;
		bool hasDisplacement;

		u32 count, id;
		RMLayoutPtr tangent, normal;

		Vec3 displacement;
		RMString name;
	};

	///Wrapper to convert the raw data into CPU data and upload to GPU

	//Osomi Raw Model format
	//Contains materials, vertices, indices, misc data if needed
	//layout data, material lists, material registers and textures
	//Written by Niels Brunekreef (n@osomi.net) for Osomi Graphics Core
	class RM : public gc::GraphicsResource {

	public:

		RM(gc::Graphics *&_gl);
		~RM();

		bool initData(OString path) override;
		bool init() override;

		void destroyData() override;
		void destroy() override;

		void bind() override;
		void unbind() override;

	protected:

		static std::vector<OString> parseSimpleStringBlock(Buffer buf, u8 *i, u16 ilen, u32 &off);				//Parse a simple string (6-bit string)
		static inline char read(Buffer buf, u32 bitOffset);														//Read a u6 value in a buffer
		static inline char decode(u8 val);																		//Decode a number into a u6

	private:

		std::vector<OString> names;
		std::vector<RMTexture> textures;
		std::vector<RMMaterial> material;
		std::vector<RMMaterialList> materialList;
		std::vector<RMRegister> registers;
		std::vector<RMLayout_CPU> layouts;
		std::vector<RMMisc> miscs;

		Buffer vertex, index;
	};

}