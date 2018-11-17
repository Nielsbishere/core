#pragma once
#include "template/enum.h"
#include "format/oisl.h"
#include "graphics/objects/texture/texture.h"

namespace oi {

	struct SLFile;

	namespace gc {

		struct MeshInfo;
		struct MeshBufferInfo;

		UEnum(RMHeaderVersion, Undefined = 0, V0_0_1 = 1);
		UEnum(RMHeaderFlag1, None = 0, Contains_materials = 1, Per_tri_materials = 2, Uses_compression = 4);

		enum class RMOperationFlag {
			NoOp = 0b00,		/* 3 indices = 1 triangle; [i, j, k] */
			Quad = 0b01,		/* 1 index = 2 triangles; [i + 2, i + 1, i, i + 3, i + 2, i ]*/
			RevIndInc = 0b10,	/* 1 index = 1 triangle; [i + 2, i + 1, i ] */
			RevIndInc2 = 0b11	/* 1 index = 1 triangle; [ i + 3, i + 2, i ] */
		};

		struct RMHeader {

			char header[4];			//oiRM

			u8 version;				//RMHeaderVersion
			u8 flags;				//RMHeaderFlag1
			u8 vertexBuffers;
			u8 vertexAttributes;

			u8 topologyMode;		//TopologyMode
			u8 fillMode;			//FillMode
			u8 miscs;
			u8 p0;

			u32 indexOperations;	//can only be non zero if indices != 0 and Uses_compression and topologyMode supports triangle

			u32 vertices;

			u32 indices;			//Optional (=0 if no indices)

		};

		struct RMVBO {

			u16 stride;
			u16 layouts;

			RMVBO(u16 stride, u16 layouts) : stride(stride), layouts(layouts) {}
			RMVBO() : RMVBO(0, 0) {}

		};

		struct RMAttribute {

			u8 padding = 0;
			u8 format;			//TextureFormat
			u16 name;

			RMAttribute(u8 format, u16 name) : format(format), name(name) {}
			RMAttribute() : RMAttribute(0, 0) {}

		};

		UEnum(RMMiscType, Vertex = 0, Primitive = 1, Center_primitive = 2, Point = 3);
		UEnum(RMMiscFlag, None = 0, Uses_offset = 1, Is_array = 2, Offset_in_relative_space = 4);

		struct RMMisc {

			u8 flags;			//RMMiscFlags
			u8 type;			//RMMiscType
			u16 size;			//Data size
			
		};

		struct RMMaterial {

			u16 name;
			u16 t_diffuse;

			u16 t_opacitySpecular;
			u16 t_emissive;

			u16 t_rahm;
			u16 t_normal;

			u16 roughness;			// / 65535.f
			u16 metallic;			//^

			//----

			u16 transparency;		//^
			u16 clearCoat;			//^

			u16 clearCoatGloss;		//^
			u16 reflectiveness;		//^

			u16 sheen;				//^
			u16 shininess;			//^

			f32 shininessExponent;

			//----

			Vec3 emissive;

			f32 diffuseScale;

			//----

			u16 specularScale;		//( - 32766.f) / 32765 (starting at 1)
			u16 ambientScale;		//^

			TVec3<u8> diffuse;
			TVec3<u8> specular;
			TVec3<u8> ambient;

			TVec3<u8> p0;

			//----

		};

		struct RMFile {

			RMHeader header;
			std::vector<RMVBO> vbos;
			std::vector<RMAttribute> vbo;
			std::vector<RMMisc> miscs;
			std::vector<CopyBuffer> vertices;
			CopyBuffer indices;
			std::vector<CopyBuffer> miscBuffer;			//Per misc data
			SLFile names;

			u32 size = 0;

			RMFile(RMHeader header, std::vector<RMVBO> vbos, std::vector<RMAttribute> vbo, std::vector<RMMisc> miscs, std::vector<CopyBuffer> vertices, CopyBuffer indices, std::vector<CopyBuffer> miscBuffer, SLFile names) : header(header), vbos(vbos), vbo(vbo), miscs(miscs), vertices(vertices), indices(indices), miscBuffer(miscBuffer), names(names) {}
			RMFile() { memset(&header, 0, sizeof(header)); }

		};

		struct oiRM {

			static bool read(String path, RMFile &file);
			static bool read(Buffer data, RMFile &file);

			static RMFile convert(MeshInfo info);

			//Returns a MeshInfo and a MeshBufferInfo for the format
			//Ideally you would remember MeshBuffer's with this exact info (except vertices and indices)
			//And create one if it doesn't exist yet (with a certain number of vertices/indices).
			//But you can just create a MeshBuffer and allocate it in there; but not recommended (because of batching).
			//It does allocate memory for vertices and indices, so clean that up.
			static std::pair<MeshBufferInfo, MeshInfo> convert(RMFile file);

			static Buffer write(RMFile &file, bool compression = true);					//Creates new buffer
			static bool write(RMFile &file, String path, bool compression = true);

			//Generate a default oiRM file
			//The layout is as follows:
			//Vec3 inPosition (if hasPos)
			//Vec2 inUv (if hasUv)
			//Vec3 inNormal (if hasNrm)
			//Meaning that vbo should be stride * vertices
			//Ibo should be u8[indices], u16[indices] or u32[indices], depending on how many vertices are present.
			static RMFile generate(Buffer vbo, Buffer ibo, bool hasPos, bool hasUv, bool hasNrm, u32 vertices, u32 indices);

			//static std::pair<MeshBufferInfo, MeshInfo> generate(Buffer vbo, Buffer ibo, bool hasPos, bool hasUv, bool hasNrm, u32 vertices, u32 indices, bool compression);
		};



	}

}