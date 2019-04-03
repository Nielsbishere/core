#pragma once
#include "reflection.hpp"
#include "types/array.h"

namespace oi {

	namespace ser {

		//Function checking

		HAS_FUNC_NAMED(IsSerializable, serialize, void, u8*);
		HAS_FUNC_NAMED(IsSized, serializeSize, size_t);

		template<typename T, bool b = std::is_arithmetic<T>::value || std::is_pod<T>::value, bool b2 = IsSerializable<T>::value && IsSized<T>::value>
		struct TBinary { };

		//POD structs and data
		template<typename T>
		struct TBinary<T, true, false> {

			static constexpr size_t dataSize(const T&) {
				return sizeof(T);
			}

			static inline void write(u8 *dst, T &src) {
				memcpy(dst, &src, sizeof(src));
			}

		};

		//Serializable structs
		template<typename T, bool b>
		struct TBinary<T, b, true> {

			static inline size_t dataSize(const T &src) {
				return src.serializeSize();
			}

			static inline void write(u8 *dst, T &src) {
				src.serialize(dst);
			}

		};

		//Binary serializer
		//NOTE: Never use size_t or raw pointers in a struct, as it varies between 64 and 32 bit programs
		struct Binary {

			template<typename T>
			static inline size_t dataSize(const T &src) {
				return TBinary<T>::dataSize(src);
			}

			template<typename T, typename ...args>
			static inline size_t dataSize(const T &t, const args& ...arg) {
				return Binary::dataSize(t) + Binary::dataSize(arg...);
			}

			template<typename T>
			static inline void write(CopyBuffer &result, T &src) {
				size_t size = Binary::dataSize(src);
				result = CopyBuffer(size);
				TBinary<T>::write(result.begin(), src);
			}

			template<typename T, typename ...args>
			static inline void write(CopyBuffer &result, T &src, args&... arg) {
				size_t size = Binary::dataSize(src, arg...);
				result = CopyBuffer(size);
				Binary::writeData(result.begin(), src, arg...);
			}

			template<typename T>
			static inline void writeData(u8 *&res, T &src) {
				TBinary<T>::write(res, src);
				res += Binary::dataSize(src);
			}

			template<typename T, typename ...args>
			static inline void writeData(u8 *&res, T &src, args&... arg) {
				TBinary<T>::write(res, src);
				res += Binary::dataSize(src);
				Binary::writeData(res, arg...);
			}

		};

	}

	//There's currently 3 serializers, however, you could create your own serializer as well (it could also just setup a UI for example)
	//Serializer<ser::Binary>; Binary serialization (put it into a format that's readable from disk)
	//Serializer<ser::Text>; JSON serialization
	template<typename T>
	struct Serializer {

		template<typename ...args>
		static inline size_t dataSize(const args&... arg) {
			return T::dataSize(arg...);
		}

		template<typename ...args>
		static inline void write(CopyBuffer &result, args&... arg) {
			T::write(result, arg...);
		}

		template<typename ...args>
		static inline void writeData(u8 *&res, args&... arg) {
			T::writeData(res, arg...);
		}

	};


}