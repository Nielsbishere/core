#include "types/buffer.h"
#include <fstream>
#include <math.h>
#include <string.h>
using namespace oi;


Buffer::Buffer() : data(nullptr), length(0) {}
Buffer::Buffer(u32 length) : data(new u8[length]), length(length) { }
Buffer::Buffer(String ostr) : Buffer((u8*)ostr.toCString(), ostr.size()) {}
u32 Buffer::size() const { return length; }

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

String Buffer::getString(u32 where, u32 length) const {
	if (where >= this->length) return String();
	return String((char*)(data + where), this->length - where < length ? this->length - where : length);
}

bool Buffer::setString(u32 where, String str) {
	if (where >= this->length) return false;
	memcpy(data + where, str.toCString(), this->length - where < str.size() ? this->length - where : str.size());
	return true;
}


CopyBuffer::CopyBuffer(u32 length): Buffer(length) { }
CopyBuffer::CopyBuffer(u8 *initData, u32 length): Buffer(initData, length) { }
CopyBuffer::CopyBuffer(Buffer buf): Buffer(buf) {}
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

Buffer Buffer::readFile(String where) {
	std::ifstream file(where.toCString(), std::ios::binary);

	if (!file.good())
		return { nullptr, 0 };

	u32 length = (u32) file.rdbuf()->pubseekoff(0, std::ios_base::end);

	file.seekg(0, std::ios::beg);
	Buffer b(length);
	memset(b.addr(), 0, b.size());
	file.read((char*)b.addr(), b.size());

	return b;
}

Buffer Buffer::operator+(u32 off) const {
	return offset(off);
}

Buffer Buffer::subbuffer(u32 off, u32 length) const {
	Buffer b = offset(off);
	if (b.length == 0) return b;

	b.length = length;
	return b;
}