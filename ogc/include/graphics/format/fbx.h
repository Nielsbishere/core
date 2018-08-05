#pragma once
#include <types/buffer.h>
#include <template/enum.h>

namespace oi {

	namespace gc {

		///All headers

		//Unfortunately, Fbx's data types are not aligned properly
		//Which requires us to manually align it (size, u8[y] and getters)
		struct FbxHeader {

			u8 data[27];

			static constexpr u32 size() { return (u32) sizeof(data); }

			//Kaydara FBX Binary\x20\x20\x00
			String getHeader() {
				return (char*)data;
			}

			//0x001A
			u16 getUnknown() {
				return *(u16*)(data + 21);
			}

			//7400 (0x1CE8) = 7.4 (32 bit)
			//7500 = 7.5 (64 bit)
			u32 getVersion() {
				return *(u32*)(data + 23);
			}

		};

		struct FbxPropertyHelper {

			template<typename T>
			static bool readOne(Buffer &buf, u32 &offset, T &dest) {

				if (buf.size() < sizeof(T))
					return Log::error("Couldn't read FbxProperty; Buffer didn't store the (full size) binary object");

				memcpy(&dest, buf.addr(), sizeof(T));
				offset += (u32) sizeof(T);
				buf = buf.offset((u32) sizeof(T));
				return true;

			}

		};

		struct FbxPropertyHeader {

			u8 data[1];

			static constexpr u32 size() { return (u32) sizeof(data); }

			char getType() { return (char) data[0]; }

			template<typename T>
			bool read(Buffer &buf, u32 &offset, std::vector<T> &vec) {
				vec.resize(1);
				return FbxPropertyHelper::readOne(buf, offset, vec[0]);
			}

		};

		struct FbxPropertyArrayHeader {

			u8 data[13];

			static constexpr u32 size() { return (u32) sizeof(data); }

			char getType() { return (char)data[0]; }
			u32 getArrayLength() { return *(u32*)(data + 1); }
			u32 getEncoding() { return *(u32*)(data + 5); }
			u32 getCompressedLength() { return *(u32*)(data + 9); }

			template<typename T>
			bool read(Buffer &buf, u32 &offset, std::vector<T> &vec) {

				vec.resize(getArrayLength());

				if (getEncoding() != 0) {

					bool result = buf.subbuffer(0, getCompressedLength()).uncompress(Buffer::construct((u8*) &vec[0], getArrayLength() * (u32) sizeof(T) + 1));

					buf = buf.offset(getCompressedLength());
					offset += getCompressedLength();
					return result;
				}

				for (u32 i = 0; i < getArrayLength(); ++i)
					if (!FbxPropertyHelper::readOne(buf, offset, vec[i]))
						return Log::error("Couldn't read FbxPropertyArray; one of the elements was invalid");

				return true;
			}

		};

		struct FbxPropertyDataHeader {

			u8 data[5];

			static constexpr u32 size() { return (u32) sizeof(data); }

			char getType() { return (char)data[0]; }
			u32 getLength() { return *(u32*)(data + 1); }

			bool read(Buffer &buf, u32 &offset, std::vector<String> &vec) {

				if (buf.size() < getLength())
					return Log::error("Couldn't read FbxProperty; Buffer didn't store the (full size) binary object");

				vec.push_back(String((char*) buf.addr(), getLength()));

				offset += getLength();
				buf = buf.offset(getLength());
				return true;
			}

			bool read(Buffer &buf, u32 &offset, std::vector<Buffer> &vec) {

				if (buf.size() < getLength())
					return Log::error("Couldn't read FbxProperty; Buffer didn't store the (full size) binary object");

				vec.push_back(buf.subbuffer(0, getLength()));

				offset += getLength();
				buf = buf.offset(getLength());
				return true;
			}

		};

		struct FbxNodeHeader32 {

			u32 endOffset = 0xFFFFFFFF;
			u32 numProperties;
			u32 propertyListLen;

			u8 nameLen;

			static constexpr u32 size() { return 13; }		//To avoid padding

		};

		struct FbxNodeHeader64 {

			u64 endOffset = 0xFFFFFFFFFFFFFFFF;
			u64 numProperties;
			u64 propertyListLen;

			u8 nameLen;

			static constexpr u32 size() { return 25; }		//To avoid padding

			FbxNodeHeader64 &operator=(const FbxNodeHeader32 &other) {

				endOffset = other.endOffset;
				numProperties = other.numProperties;
				propertyListLen = other.propertyListLen;
				nameLen = other.nameLen;

				return *this;
			}

		};

		///FbxProperty: a class that handles multiple data types of an Fbx

		enum class FbxPropertyType {
			SHORT, BOOL, INT, FLOAT, DOUBLE, LONG,
			AFLOAT, ADOUBLE, ALONG, AINT, ABOOL,
			BUFFER, STRING
		};

		class TFbxPropertyBase {

		public:

			TFbxPropertyBase() {}
			virtual ~TFbxPropertyBase() {}

			virtual char getCode() = 0;
			virtual TFbxPropertyBase *clone() = 0;
			virtual bool read(Buffer &buf, u32 &offset) = 0;

		};

		template<char Id, FbxPropertyType Type, typename Header, typename T>
		class TFbxProperty : public TFbxPropertyBase {

		public:

			char getCode() override { return header.getType(); }

			static constexpr char getTypeCode() { return Id; }
			static constexpr FbxPropertyType getType() { return Type; }
			static constexpr u32 getHeaderSize() { return header.size(); }

			T &get(u32 i = 0) {
				return contents[i];
			}

			TFbxPropertyBase *clone() override {
				return new TFbxProperty<Id, Type, Header, T>(*this);
			}

			TFbxProperty() {
				memset(&header, 0, sizeof(header));
			}

			bool read(Buffer &buf, u32 &offset) override {

				if (buf.size() < Header::size())
					return Log::error(String("Couldn't read TFbxProperty (") + Id + "); buffer out of bounds");

				memcpy(header.data, buf.addr(), Header::size());
				buf = buf.offset(Header::size());
				offset += Header::size();

				return header.read(buf, offset, contents);
			}

		protected:

			Header header;
			std::vector<T> contents;

		};

		typedef TFbxProperty<'C', FbxPropertyType::SHORT, FbxPropertyHeader, i16> FbxShort;
		typedef TFbxProperty<'Y', FbxPropertyType::BOOL, FbxPropertyHeader, bool> FbxBool;
		typedef TFbxProperty<'I', FbxPropertyType::INT, FbxPropertyHeader, i32> FbxInt;
		typedef TFbxProperty<'F', FbxPropertyType::FLOAT, FbxPropertyHeader, f32> FbxFloat;
		typedef TFbxProperty<'D', FbxPropertyType::DOUBLE, FbxPropertyHeader, f64> FbxDouble;
		typedef TFbxProperty<'L', FbxPropertyType::LONG, FbxPropertyHeader, i64> FbxLong;
		typedef TFbxProperty<'f', FbxPropertyType::AFLOAT, FbxPropertyArrayHeader, f32> FbxFloatArray;
		typedef TFbxProperty<'d', FbxPropertyType::ADOUBLE, FbxPropertyArrayHeader, f64> FbxDoubleArray;
		typedef TFbxProperty<'l', FbxPropertyType::ALONG, FbxPropertyArrayHeader, i64> FbxLongArray;
		typedef TFbxProperty<'i', FbxPropertyType::AINT, FbxPropertyArrayHeader, i32> FbxIntArray;
		typedef TFbxProperty<'b', FbxPropertyType::ABOOL, FbxPropertyArrayHeader, bool> FbxBoolArray;
		typedef TFbxProperty<'R', FbxPropertyType::BUFFER, FbxPropertyDataHeader, Buffer> FbxBuffer;
		typedef TFbxProperty<'S', FbxPropertyType::STRING, FbxPropertyDataHeader, String> FbxString;

		template<typename ...args>
		struct TFbxTypes {};

		typedef TFbxTypes<
			FbxShort, FbxBool, FbxInt, FbxFloat, FbxDouble, FbxLong,
			FbxFloatArray, FbxDoubleArray, FbxLongArray, FbxIntArray, FbxBoolArray,
			FbxBuffer, FbxString
		> FbxTypes;

		struct FbxTypeCheck {

			template<char Id, FbxPropertyType Type, typename Header, typename T>
			static bool check(TFbxProperty<Id, Type, Header, T> prop, char c) {
				return c == Id;
			}

		};

		template<typename T, typename ...args>
		struct TFbxTypeAlloc {

			static TFbxPropertyBase *get(char type) {

				if (FbxTypeCheck::check(T{}, type))
					return new T();

				return TFbxTypeAlloc<args...>::get(type);
			}

		};

		template<typename T>
		struct TFbxTypeAlloc<T> {

			static TFbxPropertyBase *get(char type) {

				if (FbxTypeCheck::check(T{}, type))
					return new T();

				return nullptr;
			}

		};

		struct FbxTypeAlloc {

			template<typename ...args>
			static TFbxPropertyBase *allocate(TFbxTypes<args...> types, char type) {
				return TFbxTypeAlloc<args...>::get(type);
			}

			static TFbxPropertyBase *allocate(char type) {
				return allocate(FbxTypes{}, type);
			}

		};

		struct FbxProperty {

			static TFbxPropertyBase *read(Buffer &buffer, u32 &offset);

		};

		//For storing and reading FbxNode(s)

		struct FbxNode {

			FbxNodeHeader64 header;
			String name;

			std::vector<TFbxPropertyBase*> properties;

			FbxNode() {}

			~FbxNode() {
				for (auto *elem : properties)
					delete elem;
			}

			FbxNode(const FbxNode &other) {
				copy(other);
			}

			FbxNode &operator=(const FbxNode &other) {
				copy(other);
				return *this;
			}

			//Reads a Buffer as an FbxNode and increments the Buffer by the required amount
			static FbxNode read(Buffer &buf, u32 &offset, bool is64bit);

			//Reads a Buffer's FbxNode(s)
			static std::vector<FbxNode> readAll(Buffer &buf, u32 &offset, bool is64bit);

			u32 getChildren();
			FbxNode &getChild(u32 i);

		private:

			void copy(const FbxNode &other) {

				header = other.header;
				name = other.name;

				properties = other.properties;
				
				for (auto *&elem : properties)
					elem = elem->clone();

				childs = other.childs;

			}

			std::vector<FbxNode> childs;

		};

		//We only accept converting fbx files
		//This is because an fbx can be either a scene or a mesh
		//convertMesh picks all meshes and exports them (oiRM)
		struct Fbx {

			//Stores all meshes (in oiRM format)
			typedef std::vector<Buffer> FbxMeshes;

			static FbxMeshes convertMeshes(Buffer fbxBuffer);
			static bool convertMeshes(Buffer fbxBuffer, String outPath);

			static FbxMeshes convertMeshes(String fbxPath);
			static bool convertMeshes(String fbxPath, String outPath);

			static std::vector<FbxNode> read(Buffer fbxBuffer);
			static std::vector<FbxNode> read(String fbxPath);

		};

	}

}