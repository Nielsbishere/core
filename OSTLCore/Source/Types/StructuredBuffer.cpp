#include "Types/StructuredBuffer.h"
using namespace oi;

#define CAST_VAR(dtype, dgdt, did) if (var.getType() == GDataType(dgdt)) return *(dtype*)&(**this)[0]; Log::throwError<BufferVar, did>("BufferVar couldn't be cast"); return *(dtype*)nullptr;

StructuredBufferVar::StructuredBufferVar() : StructuredBufferVar(nullptr, 0, "", GDataType::oi_undefined, 0, 0, 0, u32_MAX) {}
StructuredBufferVar::StructuredBufferVar(StructuredBuffer *_buf, u32 _id, OString _name, GDataType _type, u32 _offset, u32 _stride, u32 _length, u32 _parent): buf(_buf), id(_id), name(_name), type(_type), offset(_offset), stride(_stride), length(_length), parent(_parent) { }

GDataType StructuredBufferVar::getType() const { return type; }
u32 StructuredBufferVar::getOffset() const { return offset; }
u32 StructuredBufferVar::getLength() const { return length; }
u32 StructuredBufferVar::getStride() const { return stride; }

StructuredBufferVar *StructuredBufferVar::getParent() const { if (buf != nullptr) return &buf->operator[](parent - 1); return nullptr; }
StructuredBuffer *StructuredBufferVar::getBuffer() const { return buf; }

OString StructuredBufferVar::getName() const { return name; }
u32 StructuredBufferVar::getId() const { return id; }

u32 StructuredBufferVar::forOffset(u32 i) const {
	return getLocalOffset() + stride * i;
}

u32 StructuredBufferVar::getLocalOffset() const {

	if (parent == u32_MAX)
		return offset;

	return offset - buf->operator[](parent).offset;
}

bool StructuredBufferVar::operator==(const StructuredBufferVar &var) const {
	return id == var.id;
}

Buffer BufferVar::operator[](u32 i) {

	if (i >= var.getLength())
		return Buffer::construct(nullptr, 0);

	return var.getBuffer()->getBuffer().subbuffer(offset + var.getStride() * i, var.getStride());
}

Buffer BufferVar::operator*() {
	return this->operator[](0);
}

Buffer BufferVar::toBuffer() {
	return var.getBuffer()->getBuffer().subbuffer(offset, var.getStride() * var.getLength());
}

bool BufferVar::operator==(const BufferVar &ovar) const {
	return var.getId() == ovar.var.getId();
}

i32 &BufferVar::toInt() {
	CAST_VAR(i32, GDataType::oi_int, 0)
}

u32 &BufferVar::toUInt() {
	CAST_VAR(u32, GDataType::oi_uint, 1)
}

u64 &BufferVar::toULong() {
	CAST_VAR(u64, GDataType::oi_sampler, 2)
}

f32 &BufferVar::toFloat() {
	CAST_VAR(f32, GDataType::oi_float, 3)
}

f64 &BufferVar::toDouble() {
	CAST_VAR(f64, GDataType::oi_double, 4)
}

gbool &BufferVar::toBool() {
	CAST_VAR(gbool, GDataType::oi_bool, 5)
}

Vec2i &BufferVar::toInt2() {
	CAST_VAR(Vec2i, GDataType::oi_int2, 6)
}

Vec2u &BufferVar::toUInt2() {
	CAST_VAR(Vec2u, GDataType::oi_uint2, 7)
}

Vec2d &BufferVar::toDouble2() {
	CAST_VAR(Vec2d, GDataType::oi_double2, 8)
}

Vec2b &BufferVar::toBool2() {
	CAST_VAR(Vec2b, GDataType::oi_bool2, 9)
}

Vec2 &BufferVar::toFloat2() {
	CAST_VAR(Vec2, GDataType::oi_float2, 10)
}


Vec3i &BufferVar::toInt3() {
	CAST_VAR(Vec3i, GDataType::oi_int3, 11)
}

Vec3u &BufferVar::toUInt3() {
	CAST_VAR(Vec3u, GDataType::oi_uint3, 12)
}

Vec3d &BufferVar::toDouble3() {
	CAST_VAR(Vec3d, GDataType::oi_double3, 13)
}

Vec3b &BufferVar::toBool3() {
	CAST_VAR(Vec3b, GDataType::oi_bool3, 14)
}

Vec3 &BufferVar::toFloat3() {
	CAST_VAR(Vec3, GDataType::oi_float3, 15)
}


Vec4i &BufferVar::toInt4() {
	CAST_VAR(Vec4i, GDataType::oi_int4, 16)
}

Vec4u &BufferVar::toUInt4() {
	CAST_VAR(Vec4u, GDataType::oi_uint4, 17)
}

Vec4d &BufferVar::toDouble4() {
	CAST_VAR(Vec4d, GDataType::oi_double4, 18)
}

Vec4b &BufferVar::toBool4() {
	CAST_VAR(Vec4b, GDataType::oi_bool4, 19)
}

Vec4 &BufferVar::toFloat4() {
	CAST_VAR(Vec4, GDataType::oi_float4, 20)
}

StructuredBuffer::StructuredBuffer(Buffer _buf) : buf(_buf) {}

StructuredBufferVar &StructuredBuffer::operator[](u32 i) {

	if (i >= size()) return *(StructuredBufferVar*)nullptr;
	for (auto &val : vars)
		if (val.second.getId() == i)
			return val.second;

	return *(StructuredBufferVar*)nullptr;
}

StructuredBufferVar &StructuredBuffer::find(OString s) {
	auto it = vars.find(s);
	if (it == vars.end()) return *(StructuredBufferVar*)nullptr;
	return it->second;
}

u32 StructuredBuffer::size() { return (u32)vars.size(); }
u32 StructuredBuffer::dataSize() { return buf.size(); }

bool StructuredBuffer::contains(u32 i) { return i < size(); }
bool StructuredBuffer::contains(OString s) { return vars.find(s) != vars.end(); }

StructuredBuffer::operator Buffer() { return buf; }
Buffer StructuredBuffer::getBuffer() { return buf; }

void StructuredBuffer::setBuffer(Buffer _buf) { buf = _buf; }

u32 StructuredBuffer::find(StructuredBufferVar *var) {
	if (var == nullptr) return u32_MAX;

	for (auto &val : vars)
		if (val.second == *var)
			return val.second.getId();

	return u32_MAX;
}

StructuredBufferVar &StructuredBuffer::add(OString name, GDataType type, u32 offset, u32 stride, u32 arrayLength, StructuredBufferVar *parent) {
	return vars[name] = StructuredBufferVar(this, size(), name, type, offset, stride, arrayLength, find(parent));
}

OString StructuredBuffer::simplifyPath(OString str) {

	auto arr = str.split(".");

	for (u32 i = (u32) arr.size(); i-- > 0;)
		arr[i] = arr[i].split("[")[0];

	return OString::combine(arr, ".");
}

BufferVar StructuredBuffer::operator[](OString s) {

	auto arr = s.split(".");
	u32 offset = 0;

	StructuredBufferVar svar;
	bool isFirst = true;

	for (u32 i = arr.size(); i-- > 0;) {

		OString fullName = OString::combine(arr, "."), name = simplifyPath(fullName), varName = arr[i];

		if (!contains(name)) {
			Log::throwError<StructuredBufferVar, 21>("StructuredBufferVar couldn't be found");
			return BufferVar();
		}

		u32 off = !varName.contains("[") ? 0U : (u32)varName.split("[")[1].split("]")[0].toLong();

		auto &var = find(name);
		offset += var.forOffset(off);

		if (isFirst) {
			svar = var;
			isFirst = false;
		}

		arr.erase(arr.begin() + i);
	}

	return BufferVar(svar, offset);
}

BufferVar::BufferVar() : BufferVar({}, 0) {}
BufferVar::BufferVar(StructuredBufferVar _var, u32 _offset): var(_var), offset(_offset) {}

bool StructuredBuffer::addAll(OString path, GDataType type, u32 offset, u32 stride, u32 arrayLength) {

	if (contains(simplifyPath(path))) return true;
	if (type == GDataType::oi_struct || type == GDataType::oi_undefined) return false;

	u32 vec = type.getValue().length;
	u32 vecStride = type.getValue().stride;
	if (stride == 0)
		stride = type.getValue().stride * vec;

	auto arr = path.split(".");
	OString cpath;
	u32 i = 0, arrlen = arr.size();

	while(arr.size() != 0) {

		OString fullName = *arr.begin();

		arr.erase(arr.begin());
		cpath += OString(i == 0 ? "" : ".") + fullName;

		OString simplified = simplifyPath(cpath);
		auto dots = simplified.find('.');
		OString prevPath = dots.size() == 0 ? "" : simplified.cutEnd(dots[dots.size() - 1]);

		u32 len = fullName.contains("[") ? (u32)fullName.split("[")[1].split("]")[0].toLong() : 1;

		StructuredBufferVar *parent = i == 0 ? nullptr : &find(prevPath);
		
		if (!contains(simplified)) {

			if (i == arrlen - 1) {

				add(simplified, type, offset, stride, arrayLength, parent);

				if (vec >= 2) {
					add(simplified + ".x", type.getValue().derivedId, offset, vecStride, 1, &find(simplified));
					add(simplified + ".y", type.getValue().derivedId, offset + vecStride, vecStride, 1, &find(simplified));
				}

				if (vec >= 3)
					add(simplified + ".z", type.getValue().derivedId, offset + vecStride * 2, vecStride, 1, &find(simplified));

				if (vec >= 4)
					add(simplified + ".w", type.getValue().derivedId, offset + vecStride * 3, vecStride, 1, &find(simplified));

				return true;
			}

			add(simplified, GDataType::oi_struct, offset, stride, len, parent);
			++i;
			continue;
		}

		StructuredBufferVar &var = find(simplified);

		u32 eoff = var.offset + var.stride * var.length;

		if (var.length < len)
			var.length = len;

		if (var.offset > offset)
			var.offset = offset;

		if (offset - var.offset + stride > var.stride)
			var.stride = offset - var.offset + stride;

		++i;
	}

	return false;
}