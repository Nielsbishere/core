#include "types/string.h"
#include "types/buffer.h"
#include "utils/json.h"
#include "types/vector.h"
#include <cmath>
#include <algorithm>
#include <inttypes.h>
using namespace oi;

String::String(const char *begin, const char *end) : String(end - begin, begin) {}
String::String(char val) : len(1) { stackData[0] = val; }

String::String(const char *source) {

	len = strlen(source);

	if (len + 1 >= stackSize)
		heapData = Array<char>(len + 1);

	memcpy(begin(), source, len);
}

String::String(size_t count, const char &def): len(count) {

	if (len + 1 >= stackSize)
		heapData = Array<char>(len + 1);

	memset(begin(), def, len);
}

String::String(size_t count, const char *dat) {

	len = count;

	if (len + 1 >= stackSize)
		heapData = Array<char>(len + 1);

	memcpy(begin(), dat, len);
}

String::String(f32 val) : String((f64)val) {}
String::String(u32 val) : String((u64)val) {}
String::String(i32 val) : String((i64)val) {}
String::String(u16 val) : String((u64)val) {}
String::String(i16 val) : String((i64)val) {}
String::String(u8 val) : String((u64)val) {}
String::String(i8 val) : String((i64)val) {}

String::String(f64 val) {
	int siz = snprintf(nullptr, 0, "%f", val);
	*this = String((size_t)siz, ' ');
	snprintf(begin(), dataSize(), "%f", val);
}

String::String(i64 val) {
	int siz = snprintf(nullptr, 0, "%" PRIi64, val);
	*this = String((size_t)siz, ' ');
	snprintf(begin(), dataSize(), "%" PRIi64, val);
}

String::String(u64 val) {
	int siz = snprintf(nullptr, 0, "%" PRIu64, val);
	*this = String((size_t)siz, ' ');
	snprintf(begin(), dataSize(), "%" PRIu64, val);
}

String::String(void *v) {
	int siz = snprintf(nullptr, 0, "%p", v);
	*this = String((size_t)siz, ' ');
	snprintf(begin(), dataSize(), "%p", v);
}

String::operator f64() {
	f64 result{};
	sscanf(begin(), "%lf", &result);
	return result;
}

String::operator i64() {
	i64 result{};
	sscanf(begin(), "%" PRIi64, &result);
	return result;
}

String::operator u64() {
	u64 result{};
	sscanf(begin(), "%" PRIu64, &result);
	return result;
}

String::operator f32() {
	return (f32) operator f64();
}

String::operator i32() {
	return (i32) operator i64();
}

String::operator u32() {
	return (u32) operator u64();
}

String::operator i16() {
	return (i16) operator i64();
}

String::operator u16() {
	return (u16) operator u64();
}

String::operator i8() {
	return (i8) operator i64();
}

String::operator u8() {
	return (u8) operator u64();
}

size_t String::size() const { 
	return len;
}

size_t String::dataSize() const {
	return len + 1;
}

size_t String::lastIndex() const {
	return (len - 1) * (len > 0);
}

char &String::operator[](size_t i) { return begin()[i]; }
const char &String::operator[](size_t i) const { return begin()[i]; }

char *String::begin() { 
	return (char*)((size_t)heapData.begin() * (heapData.size() != 0) + (size_t)stackData.begin() * (heapData.size() == 0));
}

char *String::last() { return begin() + lastIndex(); }
char *String::end() { return begin() + size(); }

const char *String::begin() const { 
	return (const char*)((size_t)heapData.begin() * (heapData.size() != 0) + (size_t)stackData.begin() * (heapData.size() == 0));
}

const char *String::last() const { return begin() + lastIndex(); }
const char *String::end() const { return begin() + size(); }

String String::cutBegin(size_t start) const {
	if (start >= size()) return String();
	return String(size() - start, begin() + start);
}

String String::cutEnd(size_t end) const {
	if (end == 0) return String();
	return String(end > size() ? size() : end, begin());
}

String String::substring(size_t start, size_t end) const {

	if (end <= start || start >= size())
		return String();

	return String(end > size() ? (size() - start) : (end - start), begin() + start);
}

size_t String::count(char c) const {

	size_t i = 0;

	for (const char &cc : *this)
		i += cc == c;

	return i;
}

bool String::contains(char c) const {

	for (const char &cc : *this)
		if (cc == c)
			return true;

	return false;
}

size_t String::count(const String &s) const {

	size_t j = 0, k = 0;

	for (size_t i = 0; i < size(); ++i)
		if (begin()[i] == s[j]){

			++j;

			if (j == s.size()) {
				j = 0;
				++k;
			}

		} else if(j != 0) {
			j = 0;
			--i;
		}

	if (j == s.size())
		++k;

	return k;
}

bool String::contains(const String &s) const {

	size_t j = 0;

	for (size_t i = 0; i < size(); ++i)
		if (begin()[i] == s[j]) {

			++j;

			if (j == s.size())
				return true;

		} else if (j != 0) {
			j = 0;
			--i;
		}
	
	return j == s.size();
}

Array<size_t> String::find(char c) const {

	Array<size_t> result(count(c));

	size_t i = 0, j = 0;

	for (const char &curr : *this)
		if (curr == c) {
			result[j] = i;
			++j;
		} else ++i;

	return result;

}

Array<size_t> String::find(const String &s) const {

	Array<size_t> found(count(s));

	size_t j = 0, k = 0;

	for (size_t i = 0; i < size(); ++i)
		if (begin()[i] == s[j]) {

			++j;

			if (j == s.size()) {
				j = 0;
				found[k] = i + 1 - s.size();
				++k;
			}
		} else if(j != 0) {
			j = 0;
			--i;
		}

	if (j == s.size())
		found[k] = size() + 1 - s.size();

	return found;

}

size_t String::findFirst(char c) const {

	size_t i = 0;

	for (const char &curr : *this)
		if (curr == c)
			return i;
		else ++i;

	return nowhere;

}

size_t String::findFirst(const String &s) const {

	size_t j = 0;

	for (size_t i = 0; i < size(); ++i)
		if (begin()[i] == s[j]) {
			if (++j == s.size())
				return i + 1 - s.size();
		} else if (j != 0) {
			j = 0;
			--i;
		}

	if (j == s.size())
		return size() + 1 - s.size();

	return nowhere;

}

size_t String::findLast(char c) const {

	size_t i = 0;

	for (const char *it = last(); it >= begin(); it--)
		if (*it == c)
			return i;
		else ++i;

	return nowhere;

}

size_t String::findLast(const String &s) const {

	size_t j = 0;

	for (size_t i = lastIndex(); i != nowhere; --i)
		if (begin()[i] == s[s.lastIndex() - j]) {
			if (++j == s.size())
				return i;
		} else if (j != 0) {
			j = 0;
			--i;
		}

	if (j == s.size())
		return 0;

	return nowhere;

}

Array<TVec<size_t, 2>> String::find(const String &s, const String &end) const {

	Array<size_t> sit = find(s);
	Array<size_t> endit = find(end);
	Array<TVec<size_t, 2>> strings(sit.size());

	size_t k = 0;

	for (size_t i : sit) {

		size_t next = size();

		for (size_t j : endit) {
			if (j > i) {
				next = j;
				break;
			}
		}

		strings[k] = TVec<size_t, 2>(i, next);
		++k;
	}

	return strings;

}

String String::operator+(const String &s) const {

	String res(size() + s.size(), ' ');

	memcpy(res.begin(), begin(), size());
	memcpy(res.begin() + size(), s.begin(), s.size());

	return res;
}

String &String::operator+=(const String &s) {
	return *this = operator+(s);
}

bool String::operator<(const String &s) const {
	size_t shortest = s.size() < size() ? s.size() : size();
	return memcmp(begin(), s.begin(), shortest) < 0;
}

String String::replace(const String &s0, const String &s1) const {

	Array<size_t> pos = find(s0);

	String result(size() - s0.size() * pos.size() + s1.size() * pos.size(), ' ');

	size_t prev = 0, off = 0;

	for (size_t i : pos) {

		memcpy(result.begin() + off, begin() + prev, i - prev);
		off += i - prev;

		memcpy(result.begin() + off, s1.begin(), s1.size());
		off += s1.size();

		prev = i + s0.size();

	}

	memcpy(result.begin() + off, begin() + prev, size() - prev);

	return result;

}

String String::replaceFirst(const String &s0, const String &s1) const {

	size_t first = findFirst(s0);

	if (first == nowhere)
		return *this;

	String replaced(size() - s0.size() + s1.size(), ' ');

	memcpy(replaced.begin(), begin(), first);
	memcpy(replaced.begin() + first, s1.begin(), s1.size());
	memcpy(replaced.begin() + first + s1.size(), begin() + first + s0.size(), size() - first - s0.size());

	return replaced;
}

String String::replaceLast(const String &s0, const String &s1) const {

	size_t last = findLast(s0);

	if (last == nowhere)
		return *this;

	String replaced(size() - s0.size() + s1.size(), ' ');

	memcpy(replaced.begin(), begin(), last);
	memcpy(replaced.begin() + last, s1.begin(), s1.size());
	memcpy(replaced.begin() + last + s1.size(), begin() + last + s0.size(), size() - last - s0.size());

	return replaced;
}

String String::fromLast(const String &split) const {
	size_t last = findLast(split);
	if (last == nowhere) return {};
	return cutBegin(last + 1);
}

String String::untilLast(const String &split) const {
	size_t last = findLast(split);
	return cutEnd(last);
}

String String::fromFirst(const String &split) const {
	size_t last = findFirst(split);
	if (last == nowhere) return {};
	return cutBegin(last + 1);
}

String String::untilFirst(const String &split) const {
	size_t last = findFirst(split);
	return cutEnd(last);
}

String String::combine(const Array<String> &strings, const String &seperator) {

	if (strings.size() == 0) return {};
	if (strings.size() == 1) return strings[0];

	size_t count = 0;
	for (u32 i = 0; i < strings.size(); ++i)
		if (i == 0) count += strings[i].size();
		else count += strings[i].size() + seperator.size();

	String res(count, ' ');
	memcpy(res.begin(), strings[0].begin(), strings[0].size());

	size_t begin = 0;

	for (size_t i = 1; i < strings.size(); ++i) {
		memcpy(res.begin() + begin, seperator.begin(), seperator.size());
		memcpy(res.begin() + begin + seperator.size(), strings[i].begin(), strings[i].size());
		begin += strings[i].size() + seperator.size();
	}

	return res;
}

Array<String> String::split(const String &s) const {

	Array<size_t> results(find(s));
	Array<String> res(results.size() + 1);

	for (size_t i = 0; i < res.size(); ++i)
		res[i] = substring(i == 0 ? 0 : results[i - 1] + 1, i == results.size() ? size() : results[i]);

	return res;
}

String String::trim() const {

	size_t start = 0;
	for (size_t i = 0; i < size(); ++i) {
		const char &c = begin()[i];
		if (c != ' ' && c != '\t' && c != '\n') {
			start = i;
			break;
		}
	}

	size_t end = 0;
	for (size_t i = size() - 1; i != u32_MAX; --i) {
		const char &c = begin()[i];
		if (c != ' ' && c != '\t' && c != '\n') {
			end = i;
			break;
		}
	}

	return substring(start, end + 1);
}


String String::toLowerCase() const {

	String other(*this);

	for (u32 i = 0; i < size(); ++i)
		other[i] = (char) tolower(other[i]);

	return other;
}

String String::toUpperCase() const {

	String other(*this);

	for (u32 i = 0; i < size(); ++i)
		other[i] = (char) toupper(other[i]);

	return other;
}

bool String::equalsIgnoreCase(const String &other) const {
	
	if (size() != other.size())
		return false;

	for (size_t i = 0; i < size(); ++i)
		if (tolower(begin()[i]) != tolower(other[i]))
			return false;

	return true;
}

bool String::endsWithIgnoreCase(const String &other) const {

	if (other.size() > size())
		return false;

	for (size_t beg = size() - other.size(), i = beg; i < size(); ++i)
		if (tolower(begin()[i]) != tolower(other[i - beg]))
			return false;

	return true;

}

bool String::startsWithIgnoreCase(const String &other) const {

	if (other.size() > size())
		return false;

	for (size_t i = 0; i < other.size(); ++i)
		if (tolower(begin()[i]) != tolower(other[i]))
			return false;

	return true;
}

bool String::endsWith(const String &other) const {

	if (other.size() > size())
		return false;

	for (size_t beg = size() - other.size(), i = beg; i < size(); ++i)
		if (begin()[i] != other[i - beg])
			return false;

	return true;
}

bool String::startsWith(const String &other) const {

	if (other.size() > size())
		return false;

	return memcmp(begin(), other.begin(), other.size()) == 0;
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

	for (u32 i = 0; i < size(); ++i) {
		const char &c = begin()[i];
		if (!(c >= '0' && c <= '9') && !(c == '-' && i == 0))
			return false;
	}

	return size() != 0;
}

bool String::isUint() const {

	for (const char &c : *this)
		if (c < '0' || c > '9')
			return false;

	return size() != 0;
}

bool String::isFloatNoExp() const {

	bool containsDot = false;

	for (u32 i = 0; i < size(); ++i) {
		const char &c = begin()[i];
		if (!(c >= '0' && c <= '9') && !(c == '-' && i == 0)) {
			if (c == '.' && !containsDot)
				containsDot = true;
			else
				return false;
		}
	}

	return size() != 0;
}

bool String::isFloat() const {

	bool containsDot = false;
	bool containsE = false;

	for (u32 i = 0; i < size(); ++i) {
		const char &c = begin()[i];
		if (!(c >= '0' && c <= '9') && !(c == '-' && i == 0)) {
			if (c == '.' && !containsDot)
				containsDot = true;
			else if (tolower(c) == 'e' && !containsE) {
				containsE = true;
				containsDot = false;
			}
			else
				return false;
		}
	}

	return size() != 0;
}

bool String::isVector() const {
	
	bool containsDot = false;
	bool containsE = false;

	for (u32 i = 0; i < size(); ++i) {
		const char &c = begin()[i];
		if (!(c >= '0' && c <= '9') && !(c == '-' && i == 0)) {
			if (c == '.' && !containsDot)
				containsDot = true;
			else if (tolower(c) == 'e' && !containsE) {
				containsE = true;
				containsDot = false;
			}
			else if (c != ' ' && c != '\t' && c != ',')
				return false;
		}
	}

	return size() != 0;

}

size_t String::getVectorLength() const {
	if (!isVector()) return 0;
	return count(',');
}

//CRLF instead of CR or LF (so both Unix, Mac and Windows understand line endings)
String String::lineEnd() {
	return "\r\n";
}

bool String::operator==(const String &other) const {
	if(size() != other.size()) return false;
	return memcmp(begin(), other.begin(), size()) == 0;
}

bool String::operator!=(const String &other) const {
	return !(*this == other);
}

String String::toHex(u64 u) {

	u64 cpy = u;

	u = 0;

	for (u32 i = 0; i < 8; ++i)
		u |= ((cpy & (0xFF << (i * 8))) >> (i * 8)) << ((7 - i) * 8);

	Buffer b = Buffer::construct((u8*)&u, 8);
	return b.toHex();
}

String String::getHex(u8 u) {
	return String(BinaryHelper::hexChar((u & 0xF0_u8) >> 4_u8), BinaryHelper::hexChar(u & 0xF_u8));
}

String String::padStart(char c, u32 maxCount) const {
	if (size() >= maxCount) return *this;
	return String(maxCount - size(), c) + *this;
}

String String::padEnd(char c, u32 maxCount) const {
	if (size() >= maxCount) return *this;
	return operator+(String(maxCount - size(), c));
}

String String::decode(const Buffer &buf, const String &charset, u8 perChar) {
	return decode(buf, charset, perChar, buf.size() * 8 / perChar);
}

String String::decode(const Buffer &buf, const String &charset, u8 perChar, u32 length) {

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

Buffer String::encode(const String &charset, u8 perChar) const {
	
	if (perChar > 32)
		Log::throwError<String, 0x2>("Couldn't encode the string; perChar can't be bigger than 32 bits");

	if (charset.size() == 0)
		Log::throwError<String, 0x3>("Couldn't encode the string; keyset requires at least 1 char");

	u32 siz = (u32) std::ceil(size() * perChar / 8.f);

	Buffer buf(siz);
	
	for (u32 i = 0; i < size(); ++i) {

		auto it = std::find(charset.begin(), charset.end(), begin()[i]);

		u32 value = it == charset.end() ? 0 : (u32)(it - charset.begin());

		buf.setBits(i * perChar, perChar, value);

	}

	return buf;
}

String String::getDefaultCharset() {
	return " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.";
}

size_t String::getHash() const {

	size_t hash = 5381;

	for (const char &c : *this)
		hash += (hash << 5) + c;

	return hash;

}