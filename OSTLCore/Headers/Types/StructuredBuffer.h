#pragma once
#include "GDataType.h"
#include "Types/Buffer.h"
#include "Types/Vector.h"

namespace oi {

	class StructuredBuffer;

	class StructuredBufferVar {

		friend class StructuredBuffer;

	public:

		StructuredBufferVar();
		StructuredBufferVar(StructuredBuffer *buf, u32 id, OString name, GDataType type, u32 offset, u32 stride, u32 length, u32 parent);
		
		GDataType getType() const;
		u32 getOffset() const;
		u32 getLength() const;
		u32 getStride() const;

		bool operator==(const StructuredBufferVar &var) const;	//Compare

		StructuredBufferVar *getParent() const;					//nullptr when no parent; aka root variable in buffer
		StructuredBuffer *getBuffer() const;

		OString getName() const;
		u32 getId() const;

		u32 forOffset(u32 i = 0) const;

		void setDimensions(std::vector<u32> dims);				//set the multi-dimensional array size
		std::vector<u32> getDimensions();

		u32 getLocalOffset() const;

	private:

		GDataType type;
		u32 id, offset, length, stride, parent;
		OString name;
		StructuredBuffer *buf;
		std::vector<u32> lengths;
	};

	class BufferVar {

	public:

		BufferVar();
		BufferVar(StructuredBufferVar var, u32 offset);

		Buffer operator[](u32 i);	//For arrays
		Buffer operator*();			//For objects

		template<u32 n>
		Buffer operator[](TVec<u32, n> vec) {

			u32 index = 0;
			std::vector<u32> dims = var.getDimensions();

			if (n != dims.size())
				Log::throwError<BufferVar, 0x0>("Couldn't fetch Buffer at location; vector sizes must match");

			for (u32 i = 0; i < dims.size(); ++i) {

				u32 effect = 1;

				for (u32 j = i + 1; j < dims.size(); ++j)
					effect *= dims[j];

				index += vec[i] * effect;
			}

			return operator[](index);
		}

		bool operator==(const BufferVar &var) const;	//Compare

		Buffer toBuffer();
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

		static OString simplifyPath(OString str);		//Gets rid of brackets

		//Add SBV (by fully qualified name)
		//Stride = 0; default stride (length of data type)
		StructuredBufferVar &add(OString name, GDataType type, u32 offset, u32 stride = 0, u32 arrayLength = 1, StructuredBufferVar *parent = nullptr);

		//Add all SBV objects that occur in the path
		//If this function is called for every variable, it will form structs even if the elements aren't aligned
		//This can be useful for things like shader reflection (to turn plain types with implicit structs into explicit structs)
		bool addAll(OString path, GDataType type, u32 offset, u32 stride = 0, u32 arrayLength = 1);

		BufferVar operator[](OString path);	//By path

	private:

		Buffer buf;
		std::unordered_map<OString, StructuredBufferVar> vars;
	};

}