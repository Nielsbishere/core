#include "Types/Buffer.h"
#include <fstream>
using namespace oi;


Buffer::Buffer() : data(nullptr), length(0) {}

Buffer Buffer::offset(u32 i) const {
	if (i >= length) return { nullptr, 0 };
	return { data + i, length - i };
}

Buffer::Buffer(u32 _length): data(new u8[_length]), length(_length) { }
Buffer Buffer::construct(u8 *initData, u32 length) { 
	Buffer buf;
	buf.data = initData;
	buf.length = length;
	return buf;
}
u32 Buffer::size() const { return length; }
Buffer::Buffer(u8 *_initData, u32 _length): Buffer(_length) {
	copy(construct(_initData, _length));
}

Buffer::Buffer(OString ostr) : Buffer((u8*)ostr.c_str(), ostr.size()) {}

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

Buffer::operator std::string() const {

	std::string result(length * 3 - 1, '\0');

	for (u32 i = 0; i < length * 3 - 1; ++i)
		if (i % 3 < 2)
			result[i] = BinaryHelper::hexChar(BinaryHelper::getNibble(data[i / 3], i % 3));
		else
			result[i] = ' ';

	return result;
}

std::string Buffer::toOctal() const {

	u32 len = (u32)ceil(length * 8 / 3.0);
	std::string result(len, '\0');

	for (u32 i = 0; i < len; ++i) 
		result[i] = BinaryHelper::hexChar(nibble(BinaryHelper::getOctal(*this, i * 3)));

	return result;
}

std::string Buffer::toBinary() const {

	std::string result(length * 9 - 1, '\0');

	for (u32 i = 0; i < length * 9 - 1; ++i)
		if (i % 9 < 8)
			result[i] = BinaryHelper::getBit(*this, i / 9 * 8 + i % 9) ? '1' : '0';
		else
			result[i] = ' ';

	return result;
}

std::string Buffer::getString(u32 where, u32 length) const {
	if (where >= this->length) return std::string();
	return std::string((char*)(data + where), this->length - where < length ? this->length - where : length);
}

bool Buffer::setString(u32 where, std::string str) {
	if (where >= this->length) return false;
	memcpy(data + where, str.c_str(), this->length - where < str.size() ? this->length - where : str.size());
	return true;
}


CopyBuffer::CopyBuffer(u32 _length): Buffer(_length) { }
CopyBuffer::CopyBuffer(u8 *initData, u32 length): Buffer(initData, length) { }
CopyBuffer::CopyBuffer(Buffer buf): Buffer(buf) {}
CopyBuffer::~CopyBuffer() {
	deconstruct();
}

CopyBuffer::CopyBuffer(const CopyBuffer &cb) { copy(cb); }
CopyBuffer &CopyBuffer::operator=(const CopyBuffer &cb) { return copy(cb); }

Buffer &CopyBuffer::toBuffer() const { return (Buffer&)*this; }

CopyBuffer &CopyBuffer::copy(const CopyBuffer &cb) {
	Buffer &buf = cb.toBuffer();
	buf.length = cb.length;
	buf.data = new u8[cb.length];
	Buffer::copy(buf);
	return *this;
}

Buffer Buffer::readFile(OString where) {
	std::ifstream file(where.c_str(), std::ios::binary);

	if (!file.good())
		return { nullptr, 0 };

	u32 length = 0;
	file.seekg(file.end);
	length = (u32)file.tellg();
	file.seekg(file.beg);

	Buffer b(length);
	file.read((char*)&b[0], b.length);

	return b;
}