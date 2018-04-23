#pragma once

#include "generic.h"
#include <sstream>

namespace oi {

	template<typename T, u32 n> class TVec;

	class JSON;
	class Buffer;

	class String {

		friend struct std::hash<String>;

	public:

		String();
		String(std::string source);
		String(const char *source);
		String(char *source, u32 len);
		String(u32 len, char filler);
		String(JSON json);
		String(const String &str);

		template<typename T, u32 n>
		String(TVec<T, n> t);

		String(i32 i);
		String(u32 u);
		String(flp f);
		String(char c);
		explicit String(void *v);

		u32 size() const;
		char &operator[](u32 i);
		char at(u32 i) const;

		String operator+(const String &s) const;
		String &operator+=(const String &s);

		//Sort operator
		bool operator<(const String &s) const;

		String cutBegin(u32 start) const;
		String cutEnd(u32 end) const;
		String substring(u32 start, u32 end) const;

		u32 count(char c) const;
		u32 count(String s) const;

		String padStart(char c, u32 count) const;		//Add padding to the start of this string, until it this->size() eaches the specified count
		String padEnd(char c, u32 count) const;			//Add padding to the end of this string, until it this->size() eaches the specified count

		std::vector<u32> find(char c) const;
		std::vector<u32> find(const String s) const;

		String replace(String s0, String s1) const;
		String replaceLast(String s0, String s1) const;
		String replaceFirst(String s0, String s1) const;

		std::vector<String> split(String s) const;
		std::vector<String> splitIgnoreCase(String s) const;
		String trim() const;

		template<class T>
		static String fromNumber(T t) {

			static_assert(std::is_arithmetic<T>::value, "T is not a number");

			std::stringstream ss;
			ss << t;
			return ss.str();
		}

		i64 toLong() const;
		flp toFloat() const;
		std::string toStdString() const;
		const char *toCString() const;

		template<typename T, u32 n>
		TVec<T, n> toVector() const;

		static String combine(std::vector<String> string, String insert);

		String toLowerCase() const;
		String toUpperCase() const;

		String &operator=(const String &other);
		bool operator==(const String &other) const;
		bool operator!=(String other) const;
		bool equalsIgnoreCase(String other) const;
		bool endsWithIgnoreCase(String other) const;
		bool startsWithIgnoreCase(String other) const;
		bool endsWith(String other) const;
		bool startsWith(String other) const;

		static String readFromFile(String file);			//Read file as plain text
		bool writeToFile(String file) const;				//Write file to disk
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
		u32 getVectorLength() const;

		bool contains(String other) const;
		bool containsIgnoreCase(String other) const;

		static String lineEnd();
		static String toHex(u32 u);
		static String getHex(u8 u);

		auto begin() const { return source.begin(); }
		auto end() const { return source.end(); }

		static String decode(Buffer buf, String charset, u8 perChar);	//Decode the buffer given; perChar is in bits; so you could use decode(buf, { '0', '1' }, 1) to turn it into a binary string (Would not recommend; is slow)
		static String decode(Buffer buf, String charset, u8 perChar, u32 length);
		Buffer encode(String charset, u8 perChar) const;				//Encode the String given; allocates a new buffer, make sure to deconstruct it.

		static String getDefaultCharset();

	protected:

		std::string source;
	};

}

//Hashing for String
namespace std {
	template<>
	struct hash<oi::String> {
		inline size_t operator()(const oi::String& str) const {
			return hash<std::string>()(str.source);
		}
	};
}