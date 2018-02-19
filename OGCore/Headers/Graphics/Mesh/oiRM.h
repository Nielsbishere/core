#pragma once

#include <Types/Generic.h>
#include <Types/Buffer.h>

namespace oi {

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
	//@optional u16 undefined
	//@optional u8[32] padding
	struct RMHeader {

		char head[4];
		u16 version, flags;

		u32 vertices, indices;

		u16 layouts, registers, materialList, materials;

		u16 defaultMaterial, miscs, textures, undefined;

		u8 padding[32];

	};

	class RM {

	public:

		static void read(Buffer buf);

	private:

		static std::vector<OString> parseSimpleStringBlock(Buffer buf, u8 *i, u16 ilen);
	};

}