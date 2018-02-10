#pragma once
#include "Template/Enum.h"
#include "Types/Buffer.h"
#include "Types/Vector.h"

namespace oi {

	///Define shader data types
	///TODO: Matrices
	SEnum(GDataType, _(u32 id, stride, length;),
		oi_undefined = _({ 0, 0, 0 }),
		oi_float = _({ 1, 4, 1 }), oi_float2 = _({ 2, 4, 2 }), oi_float3 = _({ 3, 4, 3 }), oi_float4 = _({ 4, 4, 4 }),
		oi_double = _({ 5, 8, 1 }), oi_double2 = _({ 6, 8, 2 }), oi_double3 = _({ 7, 8, 3 }), oi_double4 = _({ 8, 8, 4 }),
		oi_int = _({ 9, 4, 1 }), oi_int2 = _({ 10, 4, 2 }), oi_int3 = _({ 11, 4, 3 }), oi_int4 = _({ 11, 4, 4 }),
		oi_uint = _({ 12, 4, 1 }), oi_uint2 = _({ 13, 4, 2 }), oi_uint3 = _({ 14, 4, 3 }), oi_uint4 = _({ 15, 4, 4 }),
		oi_bool = _({ 16, 4, 1 }), oi_bool2 = _({ 17, 4, 2 }), oi_bool3 = _({ 18, 4, 3 }), oi_bool4 = _({ 19, 4, 4 }),
		oi_sampler = _({ 20, 8, 1 }),
		oi_struct = _({ 255, 0, 0 })
	);

	class StructuredBuffer;

	class StructuredBufferVar {

	public:

		StructuredBufferVar();
		StructuredBufferVar(StructuredBuffer *buf, u32 id, OString name, GDataType type, u32 offset, u32 stride, u32 length, u32 parent);
		
		GDataType getType() const;
		u32 getOffset() const;
		u32 getLength() const;
		u32 getStride() const;

		bool operator==(const StructuredBufferVar &var) const;	//Compare

		StructuredBufferVar *getParent() const;		//nullptr when no parent; aka root variable in buffer
		StructuredBuffer *getBuffer() const;

		OString getName() const;
		u32 getId() const;

		u32 forOffset(u32 i = 0) const;

	private:

		GDataType type;
		u32 id, offset, length, stride, parent;
		OString name;
		StructuredBuffer *buf;
	};

	class BufferVar {

	public:

		BufferVar();
		BufferVar(StructuredBufferVar var, u32 offset);

		Buffer operator[](u32 i);	//For arrays
		Buffer operator*();			//For objects

		bool operator==(const BufferVar &var) const;	//Compare

		i32 &toInt();
		u32 &toUInt();
		u64 &toULong();				//Get value as u64 (for samplers aka 'bindless textures')
		f32 &toFloat();
		f64 &toDouble();
		gbool &toBool();			//Get value as 'bool'; since bool doesn't really exist on GPU; it simply wraps a uint

		Vec2i &toInt2();
		Vec2u &toUInt2();
		Vec2 &toFloat2();
		Vec2d &toDouble2();
		Vec2b &toBool2();

		Vec3i &toInt3();
		Vec3u &toUInt3();
		Vec3 &toFloat3();
		Vec3d &toDouble3();
		Vec3b &toBool3();

		Vec4i &toInt4();
		Vec4u &toUInt4();
		Vec4 &toFloat4();
		Vec4d &toDouble4();
		Vec4b &toBool4();

	private:

		u32 offset;
		StructuredBufferVar var;
	};

	class StructuredBuffer {

	public:

		StructuredBuffer(Buffer buf);

		StructuredBufferVar &operator[](u32 i);			//By id
		StructuredBufferVar &find(OString s);			//By name

		u32 size();
		u32 dataSize();

		bool contains(u32 i);							//Contains id
		bool contains(OString s);						//Contains name
		u32 find(StructuredBufferVar *var);				//Get index from value

		operator Buffer();
		Buffer getBuffer();

		void setBuffer(Buffer buf);

		OString simplifyPath(OString str);				//Gets rid of brackets

		//Add SBV (by fully qualified name)
		//Stride = 0; default stride (length of data type)
		StructuredBufferVar &add(OString name, GDataType type, u32 offset, u32 stride = 0, u32 arrayLength = 1, StructuredBufferVar *parent = nullptr);

		//Add all SBV objects that occur in the path
		//If this function is called for every variable, it will form structs even if the elements aren't aligned
		//This can be useful for things like shader reflection (to turn plain types with implicit structs into explicit structs)
		StructuredBufferVar &addAll(OString path, GDataType type, u32 offset, u32 stride = 0, u32 arrayLength = 1);

		BufferVar operator[](OString path);	//By path

	private:

		Buffer buf;
		std::unordered_map<OString, StructuredBufferVar> vars;
	};

}