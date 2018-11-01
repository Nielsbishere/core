#include <cstring>
#include "types/buffer.h"
#include "types/bitset.h"
#include "math.h"
#include "string.h"
using namespace oi;


Buffer::Buffer() : data(nullptr), length(0) {}
Buffer::Buffer(u32 length) : data(new u8[length]), length(length) { }
Buffer::Buffer(std::vector<u8> arr) : Buffer((u32)arr.size()) { memcpy(addr(), arr.data(), arr.size()); }
Buffer::Buffer(String ostr) : Buffer((u8*)ostr.toCString(), ostr.size()) {}
u32 Buffer::size() const { return length; }

Buffer::Buffer(std::vector<std::vector<u8>> dat): Buffer() {
	
	for (auto &arr : dat)
		length += (u32) arr.size();

	data = new u8[length];

	u32 offset = 0;
	for (auto &arr : dat) {
		memcpy(addr() + offset, arr.data(), arr.size());
		offset += (u32) arr.size();
	}

}

Buffer Buffer::offset(u32 i) const {
	if (i >= length) return { nullptr, 0 };
	return Buffer::construct(data + i, length - i);
}

Buffer Buffer::construct(u8 *initData, u32 length) { 
	Buffer buf;
	buf.data = initData;
	buf.length = length;
	return buf;
}

Buffer::Buffer(u8 *_initData, u32 _length): Buffer(_length) {
	copy(construct(_initData, _length));
}

void Buffer::copy(Buffer buf) {
	memcpy(data, buf.data, buf.length > length ? length : buf.length);
}

bool Buffer::copy(Buffer buf, u32 bytesToCopy, u32 bufferOffset, u32 where) {

	if (bufferOffset >= buf.length || where >= length) return false;

	memcpy(data + where, buf.data + bufferOffset, bytesToCopy < length - where && bytesToCopy < buf.length - bufferOffset ? bytesToCopy : (length - where < buf.length - bufferOffset ? length - where : buf.length - bufferOffset));
	return true;
}

bool Buffer::deconstruct() {
	if (data == nullptr || length == 0) return false;
	delete[] data;
	length = 0;
	data = nullptr;
	return true;
}

String Buffer::toHex() const {

	String result(length * 3 - 1, '\0');

	for (u32 i = 0; i < length * 3 - 1; ++i)
		if (i % 3 < 2)
			result[i] = BinaryHelper::hexChar(BinaryHelper::getNibble(data[i / 3], i % 3));
		else
			result[i] = ' ';

	return result;
}

String Buffer::toOctal() const {

	u32 len = (u32)ceil(length * 8 / 3.0);
	String result(len, '\0');

	for (u32 i = 0; i < len; ++i) 
		result[i] = BinaryHelper::hexChar(nibble(BinaryHelper::getOctal(*this, i * 3)));

	return result;
}

String Buffer::toBinary() const {

	String result(length * 9 - 1, '\0');

	for (u32 i = 0; i < length * 9 - 1; ++i)
		if (i % 9 < 8)
			result[i] = BinaryHelper::getBit(*this, i / 9 * 8 + i % 9) ? '1' : '0';
		else
			result[i] = ' ';

	return result;
}

String Buffer::getString(u32 where, u32 len) const {
	if (where >= length) return String();
	return String((char*)(data + where), length - where < len ? length - where : len);
}

bool Buffer::setString(u32 where, String str) {
	if (where >= this->length) return false;
	std::memcpy(data + where, str.toCString(), this->length - where < str.size() ? this->length - where : str.size());
	return true;
}

void Buffer::clear() {
	std::memset((char*)data, 0, length);
}

bool Buffer::read(Bitset &bitset, u32 bits) {

	u32 bytes = (u32) std::ceil(bits / 8.f);

	if (bytes > length)
		return Log::error("Couldn't read bitset from buffer; not enough bytes");

	bitset = Bitset(bits);
	std::memcpy(bitset.addr(), data, bytes);
	*this = offset(bytes);
	return true;
}


CopyBuffer::CopyBuffer(u32 length): Buffer(length) { }
CopyBuffer::CopyBuffer(u8 *initData, u32 length): Buffer(initData, length) { }
CopyBuffer::CopyBuffer(Buffer buf): Buffer(buf.addr(), buf.size()) {}
CopyBuffer::CopyBuffer() : CopyBuffer(0) {}
CopyBuffer::~CopyBuffer() {
	deconstruct();
}

CopyBuffer::CopyBuffer(const CopyBuffer &cb) { copy(cb); }
CopyBuffer &CopyBuffer::operator=(const CopyBuffer &cb) { return copy(cb); }

Buffer &CopyBuffer::toBuffer() const { return (Buffer&)*this; }

CopyBuffer &CopyBuffer::copy(const CopyBuffer &cb) {
	length = cb.length;
	data = new u8[cb.length];
	Buffer::copy((Buffer&)cb);
	return *this;
}

CopyBuffer::CopyBuffer(CopyBuffer &&cb) {
	length = cb.length;
	data = cb.data;
	cb.data = nullptr;
}

CopyBuffer &CopyBuffer::operator+=(const CopyBuffer &cb) {

	u8 *dat = new u8[length + cb.length];
	memcpy(dat, data, length);
	memcpy(dat + length, cb.data, cb.length);

	delete[] data;
	data = dat;
	length += cb.length;

	return *this;
}

CopyBuffer CopyBuffer::operator+(const CopyBuffer &cb) const {

	u8 *dat = new u8[length + cb.length];
	memcpy(dat, data, length);
	memcpy(dat + length, cb.data, cb.length);

	CopyBuffer buf;
	buf.data = dat;
	buf.length = length + cb.length;

	return buf;
}

Buffer Buffer::operator+(u32 off) const {
	return offset(off);
}

Buffer &Buffer::operator+=(Buffer other) {

	Buffer cpy(length + other.length);
	cpy.copy(*this, length, 0, 0);
	cpy.copy(other, other.length, 0, length);

	deconstruct();
	return *this = cpy;
}

Buffer &Buffer::operator+=(u32 delta) {
	if (delta >= length)
		return *this = {};
	return *this = offset(delta);
}

Buffer Buffer::subbuffer(u32 off, u32 len) const {
	Buffer b = offset(off);
	if (b.length == 0 || b.length < len) return b;

	b.length = len;
	return b;
}

void Buffer::setBits(u32 bitoff, u32 bits, u32 value) {
	for (u32 i = 0; i < bits; ++i)
		setBit(bitoff + i, value & (1U << i));
}

u32 Buffer::getBits(u32 bitoff, u32 bits) {

	u32 value = 0U;

	for (u32 i = 0; i < bits; ++i)
		value |= (u32) getBit(bitoff + i) << i;

	return value;
}

void Buffer::setBit(u32 bitoff, bool value) {

	u8 &val = data[bitoff / 8];

	u8 mask = 1U << (bitoff % 8U);

	if (value)
		val |= mask;
	else
		val &= ~mask;
}

bool Buffer::getBit(u32 bitoff) {
	u8 val = data[bitoff / 8];
	return val & (1U << (bitoff % 8U));
}

u8 *Buffer::addr() const { return data; }
std::vector<u8> Buffer::toArray() {

	if (size() == 0)
		return {};

	return { addr(), addr() + size() };
}

//Compressing and uncompressing (end of file to avoid poluting our Buffer.cpp's namespace)

#include "zlib/zlib.h"

bool Buffer::uncompress(Buffer output) const {

	uLong outLen = (uLong) output.size();

	if (::uncompress((Bytef*) output.data, &outLen, (Bytef*) data, (uLong) length) != Z_OK)
		return Log::error("Couldn't uncompress buffer");

	if ((u32)outLen != output.length - 1)
		return Log::error("Couldn't uncompress buffer; requested size wasn't equal to the actual size");

	return true;
}

Buffer Buffer::compress() const {

	Buffer buf(length);

	uLong outLen = buf.length;

	if (::compress((Bytef*) buf.data, &outLen, (Bytef*) data, length) != Z_OK)
		return Log::error("Couldn't uncompress buffer");

	Buffer output((u32)outLen);
	memcpy(output.data, buf.data, outLen);

	buf.deconstruct();

	return output;

}

bool Buffer::compress(Buffer output) const {

	uLong outLen = output.length;

	if (::compress((Bytef*)output.data, &outLen, (Bytef*)data, length) != Z_OK)
		return Log::error("Couldn't uncompress buffer");

	return true;

}