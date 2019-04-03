#pragma once
#include "array.h"

namespace oi {

	class Buffer;

	template<typename T, u32 n>
	class TVec;

	//TODO: char functions instead of String
	//TODO: union instead of storing 2 arrays
	class String {

	public:

		static constexpr size_t nowhere = size_t_MAX, stackSize = 32;

	private:

		size_t len;					//Length (excluding null character)
		Array<char> heapData;		//Heap data
		Array<char, stackSize> stackData;	//Stack data (if string is smaller than 32 chars)

	public:

		String(): len(0) {}
		String(const char *dat);
		String(size_t count, const char &def);
		String(size_t count, const char *dat);
		String(const char *begin, const char *end);

		explicit String(void *v);

		String operator+(const String &s) const;
		String &operator+=(const String &s);

		//Sort operator
		bool operator<(const String &s) const;

		String cutBegin(size_t start) const;
		String cutEnd(size_t end) const;
		String substring(size_t start, size_t end) const;

		bool contains(char c) const;
		bool contains(const String &s) const;
		size_t count(char c) const;
		size_t count(const String &s) const;

		String padStart(char c, u32 count) const;		//Add padding to the start of this string, until it this->size() reaches the specified count
		String padEnd(char c, u32 count) const;			//Add padding to the end of this string, until it this->size() reaches the specified count

		Array<size_t> find(char c) const;
		Array<size_t> find(const String &s) const;
		size_t findFirst(char c) const;
		size_t findFirst(const String &s) const;
		size_t findLast(char c) const;
		size_t findLast(const String &s) const;

		//Get all strings starting at find(s) with length of find(s) - find(end)
		//If it can't find anything it will just pick size()
		Array<TVec<size_t, 2>> find(const String &s, const String &end) const;

		String replace(const String &needle, const String &replacement) const;
		String replaceLast(const String &needle, const String &replacement) const;
		String replaceFirst(const String &needle, const String &replacement) const;

		String insert(const String &str, size_t loc) const;
		String replace(const String &str, size_t loc, size_t count) const;

		String fromLast(const String &split) const;
		String untilLast(const String &split) const;
		String fromFirst(const String &split) const;
		String untilFirst(const String &split) const;

		Array<String> split(const String &s) const;
		String trim() const;

		static String combine(const Array<String> &string, const String &insert);

		String toLowerCase() const;
		String toUpperCase() const;

		bool operator==(const String &other) const;
		bool operator!=(const String &other) const;
		bool equalsIgnoreCase(const String &other) const;
		bool endsWithIgnoreCase(const String &other) const;
		bool startsWithIgnoreCase(const String &other) const;
		bool endsWith(const String &other) const;
		bool startsWith(const String &other) const;

		String getPath() const;								//File path without file name
		String getExtension() const;						//Get extension
		String getFileName() const;							//Get file name (+ extension)
		String getFile() const;								//Get file name (- extension)
		String getFilePath() const;							//Get file path without extension

		bool isInt() const;
		bool isUint() const;
		bool isFloat() const;
		bool isFloatNoExp() const;							//Is float; without e notation (ex. 1e5)
		bool isVector() const;
		size_t getVectorLength() const;

		static String lineEnd();
		static String toHex(u64 u);
		static String getHex(u8 u);

		//Decode the buffer with the given charset
		static String decode(const Buffer &buf, const String &charset, u8 perChar);

		//Decode the buffer with the given charset
		static String decode(const Buffer &buf, const String &charset, u8 perChar, u32 length);

		//Encode the string given with the charset
		Buffer encode(const String &charset, u8 perChar) const;

		static String getDefaultCharset();

		size_t getHash() const;
		size_t size() const;
		size_t dataSize() const;

		char *begin();
		char *last();
		char *end();

		const char *begin() const;
		const char *last() const;
		const char *end() const;

		size_t lastIndex() const;

		char &operator[](size_t i);
		const char &operator[](size_t i) const;

		//Conversions to and from data types

		String(f64 val);
		String(u64 val);
		String(i64 val);
		String(f32 val);
		String(u32 val);
		String(i32 val);
		String(u16 val);
		String(i16 val);
		String(u8 val);
		String(i8 val);
		String(char val);

		explicit operator f64();
		explicit operator i64();
		explicit operator u64();
		explicit operator f32();
		explicit operator i32();
		explicit operator u32();
		explicit operator i16();
		explicit operator u16();
		explicit operator i8();
		explicit operator u8();

	};

}

//Hashing for String
namespace std {
	template<>
	struct hash<oi::String> {
		inline size_t operator()(const oi::String &str) const {
			return str.getHash();
		}
	};
}