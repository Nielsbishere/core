#pragma once

#include "Generic.h"

namespace oi {

	template<typename T, u32 n> class TVec;

	class OString {

	public:

		OString();
		OString(std::string source);
		OString(const char *source);
		OString(char *source, u32 len);
		OString(u32 len, char filler);

		OString(i32 i);
		OString(u32 u);
		OString(f32 f);
		OString(char c);

		u32 size() const;
		char &operator[](u32 i);
		char at(u32 i) const;

		OString operator+(const OString &s) const;
		OString &operator+=(const OString &s);

		OString cutBegin(u32 start) const;
		OString cutEnd(u32 end) const;
		OString substring(u32 start, u32 end) const;

		u32 count(char c) const;
		u32 count(OString s) const;

		std::vector<u32> find(char c) const;
		std::vector<u32> find(const OString s) const;

		OString replace(OString s0, OString s1) const;

		std::vector<OString> split(OString s) const;
		std::vector<OString> splitIgnoreCase(OString s) const;
		OString trim();

		template<class T>
		static OString fromNumber(T t) {

			static_assert(std::is_arithmetic<T>::value, "T is not a number");

			std::stringstream ss;
			ss << t;
			return ss.str();
		}

		template<typename T, u32 n>
		static OString fromVector(TVec<T, n> t);

		i64 toLong();
		f64 toDouble();

		operator std::string() const { return source; }

		const char* c_str() const { return source.c_str(); }

		static OString combine(std::vector<OString> string, OString insert);

		OString toLowerCase() const;
		OString toUpperCase() const;

		bool operator==(OString other) const;
		bool operator!=(OString other) const;
		bool equalsIgnoreCase(OString other) const;
		bool endsWithIgnoreCase(OString other) const;
		bool startsWithIgnoreCase(OString other) const;
		bool endsWith(OString other) const;
		bool startsWith(OString other) const;

		static OString readFromFile(OString file);				//Read file as plain text
		OString getPath() const;								//File path without file name
		OString getExtension() const;							//Get extension
		OString getFileName() const;							//Get file name (+ extension)
		OString getFile() const;								//Get file name (- extension)
		OString getFilePath() const;							//Get file path without extension

		bool isInt();
		bool isUint();
		bool isFloat();
		bool isFloatNoExp();									//Is float; without e notation (1e5)

		bool contains(OString other) const;
		bool containsIgnoreCase(OString other) const;

		static OString lineEnd();
		static OString toHex(u32 u);/*
		static OString toOctal(u32 u);
		static OString toBinary(u32 u);*/
		static OString getHex(u8 u);

		auto begin() const { return source.begin(); }
		auto end() const { return source.end(); }

	private:

		std::string source;
	};

}