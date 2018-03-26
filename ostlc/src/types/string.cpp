#include "types/string.h"
#include "types/buffer.h"
#include "utils/json.h"
#include <fstream>
using namespace oi;

String::String() {}
String::String(std::string source) : source(source) {}
String::String(const char *source): source(source) { }
String::String(char *source, u32 len) : source(source, len) {}
String::String(u32 len, char filler) : source(len, filler) {}
String::String(i32 i) { *this = fromNumber<i32>(i); }
String::String(u32 i) { *this = fromNumber<u32>(i); }
String::String(flp f) { *this = fromNumber<flp>(f); }
String::String(char c): source(1, c) {}
String::String(JSON json) {  *this = json.toString(); }
String::String(const String &str): source(str.source) { }

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

i64 String::toLong() const {

	i64 object;

	std::stringstream ss;
	ss << source;
	ss >> object;

	return object;
}

flp String::toFloat() const {

	flp object;

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

String String::readFromFile(String file) {
	std::ifstream str(file.source);
	if (!str.good()) 
		return "";

	return std::string(std::istreambuf_iterator<char>(str), std::istreambuf_iterator<char>());
}

bool String::writeToFile(String file) const {

	std::ofstream str(file.source);
	if (!str.good())
		return false;

	str.write(source.c_str(), source.size());
	return true;
}

String String::getPath() const {
	std::vector<String> subs = split("/");
	if (subs.size() > 1) {
		subs.erase(subs.end() - 1);
		return combine(subs, "/");
	}
	return "/";
}

String String::getExtension() const {
	std::vector<String> subs = getFile().split(".");
	if (subs.size() > 1)
		return subs[subs.size() - 1];
	return "";
}

String String::getFileName() const {
	std::vector<String> subs = getFile().split(".");
	if (subs.size() > 1) {
		subs.erase(subs.end() - 1);
		return combine(subs, ".");
	}
	return "";

}

String String::getFile() const {
	std::vector<String> subs = split("/");

	if (subs.size() > 1)
		return subs[subs.size() - 1];

	return "";
}

String String::getFilePath() const {
	std::vector<String> subs = split(".");
	if (subs.size() > 1) {
		subs.erase(subs.end() - 1);
		return combine(subs, ".");
	}
	return "";
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

	std::vector<String> splits = splitIgnoreCase(",");

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