#include "types/string.h"
#include "types/buffer.h"
#include "utils/json.h"
#include "types/vector.h"
#include <cmath>
#include <algorithm>
using namespace oi;

String::String() {}
String::String(std::string source) : source(source) {}
String::String(const char *source): source(source) { }
String::String(char *source, u32 len) : source(source, len) {}
String::String(u32 len, char filler) : source(len, filler) {}
String::String(i32 i) { *this = fromNumber<i32>(i); }
String::String(i64 i) { *this = fromNumber<i64>(i); }
String::String(u64 i) { *this = fromNumber<u64>(i); }
String::String(u32 i) { *this = fromNumber<u32>(i); }
String::String(f32 f) { *this = fromNumber<f32>(f); }
String::String(char c): source(1, c) {}
String::String(JSON json) {  *this = json.toString(); }
String::String(const String &str): source(str.source) { }

String::String(void *v){
	std::stringstream ss;
	ss << v;
	source = ss.str();
}

u32 String::size() const { return (u32)source.size(); }
char &String::operator[](u32 i) { return source[i]; }
char String::at(u32 i) const { return source[i]; }

String String::cutBegin(u32 start) const {

	if (start >= size()) return String();

	return String(const_cast<char*>(source.c_str() + start), (u32)(source.size() - start));
}

String String::cutEnd(u32 end) const {
	if (end == 0) return String();

	return String(const_cast<char*>(source.c_str()), end);
}

String String::substring(u32 start, u32 end) const {
	if (end <= start || start >= size())
		return String();

	return String(const_cast<char*>(source.c_str() + start), end > size() ? size() : (end - start));
}

u32 String::count(char c) const {
	size_t offset = 0;
	u32 count = 0;
	while ((offset = source.find(c, offset)) != std::string::npos) {
		++count;
		++offset;
	}
	return count;
}

u32 String::count(String s) const {
	size_t offset = 0;
	u32 count = 0;
	while ((offset = source.find(s.source, offset)) != std::string::npos) {
		++count;
		offset += s.source.size();
	}
	return count;
}

std::vector<u32> String::find(char c) const {
	std::vector<u32> result;
	size_t offset = 0;
	while ((offset = source.find(c, offset)) != std::string::npos) {
		result.push_back((u32)offset);
		++offset;
	}

	return result;
}

std::vector<u32> String::find(const String s) const {
	std::vector<u32> result;
	size_t offset = 0;
	while ((offset = source.find(s.source, offset)) != std::string::npos) {
		result.push_back((u32)offset);
		++offset;
	}

	return result;
}

std::vector<Vec2u> String::find(const String s, const String end, u32 offset) const {

	std::vector<u32> sit = find(s);
	std::vector<Vec2u> strings(sit.size());

	u32 j = 0;
	for (u32 i : sit) {

		size_t endit = source.find(end.source, i + s.size());
		u32 k = 0;

		if (endit == std::string::npos)
			k = size();
		else
			k = (u32) endit;

		strings[j] = i + offset >= k ? Vec2u() : Vec2u(i + offset, k);

		++j;
	}

	return strings;

}

String String::operator+(const String &s) const {
	return source + s.source;
}

String &String::operator+=(const String &s) {
	source += s.source;
	return *this;
}

bool String::operator<(const String &s) const {

	u32 shortest = s.size() < size() ? s.size() : size();

	return memcmp(toCString(), s.toCString(), shortest) < 0;
}

String String::replace(String s0, String s1) const {
	return combine(split(s0), s1);
}

String String::replaceFirst(String s0, String s1) const {
	auto arr = split(s0);
	String first = arr[0];
	arr.erase(arr.begin());
	String last = combine(arr, s0);
	return first + s1 + last;
}

String String::replaceLast(String s0, String s1) const {
	auto arr = split(s0);
	String last = arr[arr.size() - 1];
	arr.erase(arr.end() - 1);
	String first = combine(arr, s0);
	return first + s1 + last;
}

String String::fromLast(String split) const {

	std::vector<u32> parts = find(split);

	if (parts.size() == 0) return "";
	return cutBegin(parts[(u32) parts.size() - 1] + 1);

}

String String::untilLast(String split) const {

	std::vector<u32> parts = find(split);

	if (parts.size() == 0) return *this;
	return cutEnd(parts[(u32)parts.size() - 1]);

}

String String::fromFirst(String split) const {

	std::vector<u32> parts = find(split);

	if (parts.size() == 0) return "";
	return cutBegin(parts[0] + 1);

}

String String::untilFirst(String split) const {

	std::vector<u32> parts = find(split);

	if (parts.size() == 0) return *this;
	return cutEnd(parts[0]);

}

i64 String::toLong() const {

	i64 object;

	std::stringstream ss;
	ss << source;
	ss >> object;

	return object;
}

f32 String::toFloat() const {

	f32 object;

	std::stringstream ss;
	ss << source;
	ss >> object;

	return object;
}

std::string String::toStdString() const { return source; }
const char *String::toCString() const { return source.c_str(); }

String String::combine(std::vector<String> strings, String seperator) {

	if (strings.size() == 0) return "";
	if (strings.size() == 1) return strings[0];

	u32 count = 0;
	for (u32 i = 0; i < strings.size(); ++i)
		if (i == 0) count += strings[i].size();
		else count += strings[i].size() + seperator.size();

	String res(count, '\0');

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

std::vector<String> String::split(String s) const {

	std::vector<String> res(count(s) + 1);

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

std::vector<String> String::splitIgnoreCase(String s) const {
	return toLowerCase().split(s.toLowerCase());
}

String String::trim() const {

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


String String::toLowerCase() const {

	String other(*this);

	for (u32 i = 0; i < size(); ++i)
		if (other[i] >= 'A' && other[i] <= 'Z')
			other[i] = other[i] - 'A' + 'a';

	return other;
}

String String::toUpperCase() const {

	String other(*this);

	for (u32 i = 0; i < size(); ++i)
		if (other[i] >= 'a' && other[i] <= 'z')
			other[i] = other[i] - 'a' + 'A';

	return other;
}

bool String::equalsIgnoreCase(String other) const {
	return toLowerCase() == other.toLowerCase();
}

bool String::endsWithIgnoreCase(String other) const {
	return toLowerCase().endsWith(other.toLowerCase());
}

bool String::startsWithIgnoreCase(String other) const {
	return toLowerCase().startsWith(other.toLowerCase());
}

bool String::endsWith(String other) const {
	if (other.size() > size()) return false;
	return cutBegin(size() - other.size()) == other;
}

bool String::startsWith(String other) const {
	if (other.size() > size()) return false;
	return cutEnd(other.size()) == other;
}

String String::getPath() const {
	return untilLast("/");
}

String String::getExtension() const {
	return fromLast(".");
}

String String::getFileName() const {
	return getFile().untilLast(".");

}

String String::getFile() const {
	return fromLast("/");
}

String String::getFilePath() const {
	return untilLast(".");
}

bool String::isInt() const {
	for (u32 i = 0; i < size(); ++i)
		if (!(source[i] >= '0' && source[i] <= '9') && !(source[i] == '-' && i == 0))
			return false;
	return true;
}

bool String::isUint() const {
	for (u32 i = 0; i < size(); ++i)
		if (!(source[i] >= '0' && source[i] <= '9'))
			return false;
	return true;
}

bool String::isFloatNoExp() const {
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

bool String::isFloat() const {
	if (!containsIgnoreCase("e"))
		return isFloatNoExp();
	
	std::vector<String> splits = splitIgnoreCase("e");

	if (splits.size() != 2)
		return false;

	return splits[0].isFloatNoExp() && splits[1].isFloatNoExp();
}

bool String::isVector() const {

	std::vector<String> splits = contains(',') ? splitIgnoreCase(',') : splitIgnoreCase(' ');

	for (u32 i = 0; i < splits.size(); ++i)
		if (!splits[i].trim().isFloat())
			return false;

	return splits.size() >= 2;
}

u32 String::getVectorLength() const {

	if (!isVector()) return 0;

	std::vector<String> splits = splitIgnoreCase(",");
	return (u32) splits.size();
}

bool String::contains(String other) const {
	return source.find(other.source) != std::string::npos;
}

bool String::containsIgnoreCase(String other) const {
	return toLowerCase().contains(other.toLowerCase());
}

String String::lineEnd() {
	std::stringstream ss;
	ss << std::endl;
	return ss.str();
}

String &String::operator=(const String &other) {
	source = other.source;
	return *this;
}

bool String::operator==(const String &other) const {
	if(size() != other.size()) return false;
	return memcmp(other.source.c_str(), source.c_str(), size()) == 0;
}

bool String::operator!=(String other) const {
	return !((*this) == other);
}

String String::toHex(u32 u) {
	u = ((u & 0xFF000000U) >> 24U) | ((u & 0xFF0000U) >> 8U) | ((u & 0xFF00U) << 8U) | ((u & 0xFFU) << 24U);
	CopyBuffer cb = CopyBuffer((u8*)&u, 4);
	return cb.toHex();
}

String String::getHex(u8 u) {
	return String(BinaryHelper::hexChar((u & 0xF0) >> 4)) + String(BinaryHelper::hexChar(u & 0xF));
}

String String::padStart(char c, u32 maxCount) const {
	if (size() >= maxCount) return *this;
	return String(maxCount - size(), c) + *this;
}

String String::padEnd(char c, u32 maxCount) const {
	if (size() >= maxCount) return *this;
	return operator+(String(maxCount - size(), c));
}

String String::decode(Buffer buf, String charset, u8 perChar) {
	return decode(buf, charset, perChar, buf.size() * 8 / perChar);
}

String String::decode(Buffer buf, String charset, u8 perChar, u32 length) {

	if (perChar > 32)
		Log::throwError<String, 0x0>("Couldn't decode the string; perChar can't be bigger than 32 bits");

	if (charset.size() == 0)
		Log::throwError<String, 0x1>("Couldn't decode the string; keyset requires at least 1 char");

	u32 i = 0;

	String decoded(length, ' ');

	while ((perChar * (i + 1)) / 8 <= buf.size()) {

		u32 value = buf.getBits(perChar * i, perChar);

		decoded[i] = charset[value >= charset.size() ? 0 : value];

		++i;

		if (i == length) break;
	}

	return decoded;

}

Buffer String::encode(String charset, u8 perChar) const {
	
	if (perChar > 32)
		Log::throwError<String, 0x2>("Couldn't encode the string; perChar can't be bigger than 32 bits");

	if (charset.size() == 0)
		Log::throwError<String, 0x3>("Couldn't encode the string; keyset requires at least 1 char");

	u32 size = (u32) std::ceil(source.size() * perChar / 8.f);

	Buffer buf(size);
	
	for (u32 i = 0; i < source.size(); ++i) {

		auto it = std::find(charset.begin(), charset.end(), source[i]);

		u32 value = it == charset.end() ? 0 : (u32)(it - charset.begin());

		buf.setBits(i * perChar, perChar, value);

	}

	return buf;
}

String String::getDefaultCharset() { return " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz."; }