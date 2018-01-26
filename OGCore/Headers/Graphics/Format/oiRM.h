//#pragma once
//
//#include <Types/Buffer.h>
//#include "Graphics/Format/VAO.h"
//
//namespace oi {
//	namespace gc {
//
//		enum class RMx1Flags {
//			perTriMaterials = 0x1,
//			containsMaterialList = 0x2,
//			containsIndices = 0x4,
//			containsMisc = 0x8
//		};
//
//		enum class RMPrimitive {
//			Triangle, TriangleFan, TriangleStrip, Points, Line, LineLoop, LineStrip,
//			TriangleAdjacency, TriangleStripAdjacency, LineAdjacency, LineStripAdjecency,
//
//			RESERVED0, RESERVED1, RESERVED2, RESERVED3, RESERVED4
//		};
//
//		struct RMHeader {
//
//			char head[4];
//			u16 version, flags;
//
//			u32 vertices, indices;
//
//			u16 layouts;
//			u8 materials, defaultMaterial;
//
//			u16 miscs;
//			u8 reserved[10];
//		};
//
//		class RMInfo {
//
//		public:
//
//			RMInfo(): path("") {}
//			RMInfo(OString p) : path(p) {}
//
//			OString getPath() { return path; }
//
//		private:
//
//			OString path;
//
//		};
//
//		class RM {
//
//		public:
//
//			RM() {}
//
//			bool init(RMInfo oiRMinf);
//
//		protected:
//
//			bool loadx1(Buffer &b, RMHeader &head);
//
//			static u32 compressNormal(Vec3 normal);
//			static u32 floatAsUint(f32 f, u32 mask);
//
//			static OString parseSlimString(Buffer b, u32 len, const OString &encoding);
//
//		private:
//
//			VAO vao;
//			/*VBO vbo;
//			IBO ibo;
//			MBO mbo;*/
//			std::vector<Layout> layouts;
//
//			static const OString varName, name;		//Encoding for a name
//		};
//
//	}
//}