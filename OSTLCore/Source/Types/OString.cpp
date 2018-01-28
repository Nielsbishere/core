#include "Types/OString.h"
#include "Types/Buffer.h"
#include <fstream>
using namespace oi;

OString::OString() {}
OString::OString(std::string _source) : source(_source) {}
OString::OString(const char *_source): source(_source) { }
OString::OString(char *source, u32 len) : source(source, len) {}
OString::OString(u32 len, char filler) : source(len, filler) {}
OString::OString(i32 i) { *this = fromNumber<i32>(i); }
OString::OString(u32 i) { *this = fromNumber<u32>(i); }
OString::OString(f32 f) { *this = fromNumber<f32>(f); }
OString::OString(char c): source(1, c) {}

u32 OString::size() const { return (u32)source.size(); }
char &OString::operator[](u32 i) { return source[i]; }
char OString::at(u32 i) const { return source[i]; }

OString OString::cutBegin(u32 start) const {

	if (start >= size()) return OString();

	return OString(const_cast<char*>(source.c_str() + start), (u32)(source.size() - start));
}

OString OString::cutEnd(u32 end) const {
	if (end == 0) return OString();

	return OString(const_cast<char*>(source.c_str()), end);
}

OString OString::substring(u32 start, u32 end) const {
	if (end <= start || start >= size())
		return OString();

	return OString(const_cast<char*>(source.c_str() + start), end > size() ? size() : (end - start));
}

u32 OString::count(char c) const {
	size_t offset = 0;
	u32 count = 0;
	while ((offset = source.find(c, offset)) != std::string::npos) {
		++count;
		++offset;
	}
	return count;
}

u32 OString::count(OString s) const {
	size_t offset = 0;
	u32 count = 0;
	while ((offset = source.find(s.source, offset)) != std::string::npos) {
		++count;
		offset += s.source.size();
	}
	return count;
}

std::vector<u32> OString::find(char c) const {
	std::vector<u32> result;
	size_t offset = 0;
	while ((offset = source.find(c, offset)) != std::string::npos) {
		result.push_back((u32)offset);
		++offset;
	}

	return result;
}

std::vector<u32> OString::find(const OString s) const {
	std::vector<u32> result;
	size_t offset = 0;
	while ((offset = source.find(s.source, offset)) != std::string::npos) {
		result.push_back((u32)offset);
		++offset;
	}

	return result;
}

OString OString::operator+(const OString &s) const {
	return source + s.source;
}

OString &OString::operator+=(const OString &s) {
	source += s.source;
	return *this;
}

bool OString::operator<(const OString &s) const {

	u32 shortest = s.size() < size() ? s.size() : size();

	return memcmp(c_str(), s.c_str(), shortest) < 0;
}

OString OString::replace(OString s0, OString s1) const {
	return combine(split(s0), s1);
}

i64 OString::toLong() {

	i64 object;

	std::stringstream ss;
	ss << source;
	ss >> object;

	return object;
}

f64 OString::toDouble() {

	f64 object;

	std::stringstream ss;
	ss << source;
	ss >> object;

	return object;
}

OString OString::combine(std::vector<OString> strings, OString seperator) {

	if (strings.size() == 0) return "";
	if (strings.size() == 1) return strings[0];

	u32 count = 0;
	for (u32 i = 0; i < strings.size(); ++i)
		if (i == 0) count += strings[i].size();
		else count += strings[i].size() + seperator.size();

	OString res(count + 1, '\0');

	u32 begin = 0;

	for (u32 i = 0; i < strings.size(); ++i) {
		if (i != 0) {
			memcpy(&res[begin], seperator.source.c_str(), seperator.size());
			begin += seperator.size();
		}
		memcpy(&res[begin], strings[i].source.c_str(), strings[i].size());
		begin += strings[i].size();
	}

	return res;
}

std::vector<OString> OString::split(OString s) const {

	std::vector<OString> res(count(s) + 1);

	u32 prev = 0;
	size_t offset = 0;
	u32 count = 0;

	while ((offset = source.find(s.source, offset)) != std::string::npos) {

		res[count] = substring(prev, (u32)offset);

		offset += s.source.size();
		prev = (u32)offset;
		++count;
	}

	res[res.size() - 1] = substring(prev, size());

	return res;
}

std::vector<OString> OString::splitIgnoreCase(OString s) const {
	return toLowerCase().split(s.toLowerCase());
}

OString OString::trim() {

	u32 start = 0;
	for (u32 i = 0; i < size(); ++i)
		if (source[i] != ' ' && source[i] != '\t' && source[i] != '\n' && source[i] != '\0') {
			start = i;
			break;
		}

	u32 end = 0;
	for (u32 i = size() - 1; i != u32_MAX; --i)
		if (source[i] != ' ' && source[i] != '\t' && source[i] != '\n' && source[i] != '\0') {
			end = i;
			break;
		}

	return substring(start, end + 1);
}


OString OString::toLowerCase() const {

	OString other(*this);

	for (u32 i = 0; i < size(); ++i)
		if (other[i] >= 'A' && other[i] <= 'Z')
			other[i] = other[i] - 'A' + 'a';

	return other;
}

OString OString::toUpperCase() const {

	OString other(*this);

	for (u32 i = 0; i < size(); ++i)
		if (other[i] >= 'a' && other[i] <= 'z')
			other[i] = other[i] - 'a' + 'A';

	return other;
}

bool OString::equalsIgnoreCase(OString other) const {
	return toLowerCase() == other.toLowerCase();
}

bool OString::endsWithIgnoreCase(OString other) const {
	return toLowerCase().endsWith(other.toLowerCase());
}

bool OString::startsWithIgnoreCase(OString other) const {
	return toLowerCase().startsWith(other.toLowerCase());
}

bool OString::endsWith(OString other) const {
	if (other.size() > size()) return false;
	return cutBegin(size() - other.size()) == other;
}

bool OString::startsWith(OString other) const {
	if (other.size() > size()) return false;
	return cutEnd(other.size()) == other;
}

OString OString::readFromFile(OString file) {
	std::ifstream str(file.source);
	if (!str.good()) 
		return "";

	return std::string(std::istreambuf_iterator<char>(str), std::istreambuf_iterator<char>());
}

OString OString::getPath() const {
	std::vector<OString> subs = split("/");
	if (subs.size() > 1) {
		subs.erase(subs.end() - 1);
		return combine(subs, "/");
	}
	return "/";
}

OString OString::getExtension() const {
	std::vector<OString> subs = getFile().split(".");
	if (subs.size() > 1)
		return subs[subs.size() - 1];
	return "";
}

OString OString::getFileName() const {
	std::vector<OString> subs = getFile().split(".");
	if (subs.size() > 1) {
		subs.erase(subs.end() - 1);
		return combine(subs, ".");
	}
	return "";

}

OString OString::getFile() const {
	std::vector<OString> subs = split("/");

	if (subs.size() > 1)
		return subs[subs.size() - 1];

	return "";
}

OString OString::getFilePath() const {
	std::vector<OString> subs = split(".");
	if (subs.size() > 1) {
		subs.erase(subs.end() - 1);
		return combine(subs, ".");
	}
	return "";
}

bool OString::isInt() {
	for (u32 i = 0; i < size(); ++i)
		if (!(source[i] >= '0' && source[i] <= '9') && !(source[i] == '-' && i == 0))
			return false;
	return true;
}

bool OString::isUint() {
	for (u32 i = 0; i < size(); ++i)
		if (!(source[i] >= '0' && source[i] <= '9'))
			return false;
	return true;
}

bool OString::isFloatNoExp() {
	bool containsDot = false;

	for (u32 i = 0; i < size(); ++i)
		if (!(source[i] >= '0' && source[i] <= '9') && !(source[i] == '-' && i == 0)) {
			if (source[i] == '.' && !containsDot)
				containsDot = true;
			else
				return false;
		}

	return true;
}

bool OString::isFloat() {
	if (!containsIgnoreCase("e"))
		return isFloatNoExp();
	
	std::vector<OString> splits = splitIgnoreCase("e");

	if (splits.size() != 2)
		return false;

	return splits[0].isFloatNoExp() && splits[1].isFloatNoExp();
}

bool OString::contains(OString other) const {
	return source.find(other.source) != std::string::npos;
}

bool OString::containsIgnoreCase(OString other) const {
	return toLowerCase().contains(other.toLowerCase());
}

OString OString::lineEnd() {
	std::stringstream ss;
	ss << std::endl;
	return ss.str();
}

bool OString::operator==(OString other) const {
	if(size() != other.size()) return false;
	return memcmp(other.source.c_str(), source.c_str(), size()) == 0;
}

bool OString::operator!=(OString other) const {
	return !((*this) == other);
}

OString OString::toHex(u32 u) {
	CopyBuffer cb = CopyBuffer((u8*)&u, 4);
	return (std::string)cb;
}

//OString OString::toOctal(u32 u) {
//	CopyBuffer cb = CopyBuffer((u8*)&u, 4);
//	if (BinaryHelper::isLittleEndian)
//		cb.reverse();
//	return cb.toOctal();
//}
//
//OString OString::toBinary(u32 u) {
//	CopyBuffer cb = CopyBuffer((u8*)&u, 4);
//	if (BinaryHelper::isLittleEndian)
//		cb.reverse();
//	return cb.toBinary();
//}

OString OString::getHex(u8 u) {
	return OString(BinaryHelper::hexChar((u & 0xF0) >> 4)) + OString(BinaryHelper::hexChar(u & 0xF));
}