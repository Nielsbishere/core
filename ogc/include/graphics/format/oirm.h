#pragma once
#include <template/enum.h>
#include "../texture.h"

namespace oi {

	struct SLFile;

	namespace gc {

		struct MeshInfo;

		UEnum(RMHeaderVersion, Undefined = 0, V0_0_1 = 1);
		UEnum(RMHeaderFlag1, None = 0, Contains_materials = 1, Contains_misc = 2, Per_tri_materials = 4);

		struct RMHeader {

			char header[4];		//oiRM

			u8 version;			//RMHeaderVersion
			u8 flags;			//RMHeaderFlag1
			u8 vertexBuffers;
			u8 vertexAttributes;

			u32 vertices;

			u32 indices;		//Optional (=0 if no indices)

			u8 topologyMode;	//TopologyMode
			u8 fillMode;		//FillMode
			u8 p0[2];

			u32 p1;

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

		struct RMFile {

			RMHeader header;
			std::vector<RMVBO> vbos;
			std::vector<RMAttribute> vbo;
			std::vector<u8> buffers;			//Stores foreach x in vbos RMVBO[x].stride * vertices and then indices * bytesPerIndex(vertices)

			RMFile();

		};

		struct oiRM {

			static bool read(String path, RMFile &file);
			static bool read(Buffer data, RMFile &file);

			static RMFile convert(MeshInfo info, SLFile *names = nullptr);
			static MeshInfo convert(Graphics *g, RMFile file, SLFile *names = nullptr);

			static Buffer write(RMFile file);					//Creates new buffer
			static bool write(String path, RMFile file);

		};



	}

}