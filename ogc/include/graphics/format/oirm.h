#pragma once
#include <template/enum.h>
#include <format/oisl.h>
#include "../texture.h"

namespace oi {

	struct SLFile;

	namespace gc {

		struct MeshInfo;
		struct MeshBufferInfo;

		UEnum(RMHeaderVersion, Undefined = 0, V0_0_1 = 1);
		UEnum(RMHeaderFlag1, None = 0, Contains_materials = 1, Per_tri_materials = 2, Uses_compression = 4);

		struct RMHeader {

			char header[4];		//oiRM

			u8 version;			//RMHeaderVersion
			u8 flags;			//RMHeaderFlag1
			u8 vertexBuffers;
			u8 vertexAttributes;

			u8 topologyMode;	//TopologyMode
			u8 fillMode;		//FillMode
			u8 miscs;
			u8 p0;

			u8 p1[4];

			u32 vertices;

			u32 indices;		//Optional (=0 if no indices)

		};

		struct RMVBO {
			u16 stride;
			u16 layouts;
		};

		struct RMAttribute {
			u8 padding;
			u8 format;			//TextureFormat
			u16 name;
		};

		UEnum(RMMiscType, Vertex = 0, Primitive = 1, Center_primitive = 2, Point = 3);
		UEnum(RMMiscFlag, None = 0, Uses_offset = 1, Is_array = 2, Offset_in_relative_space = 4);

		struct RMMisc {

			u8 flags;			//RMMiscFlags
			u8 type;			//RMMiscType
			u16 size;			//Data size
			
		};

		struct RMFile {

			RMHeader header;
			std::vector<RMVBO> vbos;
			std::vector<RMAttribute> vbo;
			std::vector<RMMisc> miscs;
			std::vector<std::vector<u8>> vertices;
			std::vector<u8> indices;
			std::vector<std::vector<u8>> miscBuffer;			//Per misc data
			SLFile names;

			u32 size = 0;

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
			static Buffer generate(Buffer vbo, Buffer ibo, bool hasPos, bool hasUv, bool hasNrm, u32 vertices, u32 indices, bool compression);

		};



	}

}