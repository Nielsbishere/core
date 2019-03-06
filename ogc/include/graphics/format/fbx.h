#pragma once
#include "types/buffer.h"
#include "types/vector.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		//Unfortunately, Fbx's data types are not aligned properly
		//Which requires us to manually align it (size(), u8[y] and getters)
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

		//A class to read a value from a buffer into a variable and to get the pointer of a T[]
		template<typename T>
		struct FbxPropertyHelper {

			static bool readOne(Buffer &buf, u32 &offset, T &dest) {

				if (buf.size() < sizeof(T))
					return Log::error("Couldn't read FbxProperty; Buffer didn't store the (full size) binary object");

				memcpy(&dest, buf.addr(), sizeof(T));
				offset += (u32) sizeof(T);
				buf = buf.offset((u32) sizeof(T));
				return true;

			}

			static T *getPtr(std::vector<T> &arr) {
				return arr.data();
			}

		};

		//Specialized for bool, because std::vector<bool>::reference is used instead of bool&
		template<>
		struct FbxPropertyHelper<bool> {

			//sizeof(bool) isn't defined; it can be something different than 1
			//We need to hardcode it as 1 byte
			static bool readOne(Buffer &buf, u32 &offset, std::vector<bool>::reference dest) {

				if (buf.size() == 0)
					return Log::error("Couldn't read FbxProperty; Buffer didn't store the (full size) binary object");

				dest = buf[0] != 0;
				++offset;
				buf = buf.offset(1);
				return true;

			}

			//There is no bool[] stored low-level, so nullptr
			static bool *getPtr(std::vector<bool> &) { return nullptr; }

		};

		//Any regular dataType (bool; C, i16; Y, i32; I, i64; L, f32; F, f64; D) uses this header
		struct FbxPropertyHeader {

			u8 data[1];

			static constexpr u32 size() { return (u32) sizeof(data); }

			char getType() { return (char) data[0]; }

			template<typename T>
			bool read(Buffer &buf, u32 &offset, std::vector<T> &vec) {
				vec.resize(1);
				return FbxPropertyHelper<T>::readOne(buf, offset, vec[0]);
			}

		};

		//Any array dataType (bool; b, i32; i, i64; l, f32; f, f64; d) uses this header
		struct FbxPropertyArrayHeader {

			u8 data[13];

			static constexpr u32 size() { return (u32) sizeof(data); }

			char getType() { return (char)data[0]; }
			u32 getArrayLength() { return *(u32*)(data + 1); }
			u32 getEncoding() { return *(u32*)(data + 5); }
			u32 getCompressedLength() { return *(u32*)(data + 9); }

			//This function loads data from a buffer like a T[] or a compressed T[], depending on this header
			template<typename T>
			bool read(Buffer &buf, u32 &offset, std::vector<T> &vec) {

				vec.resize(getArrayLength());

				if (getEncoding() != 0) {

					bool result = buf.subbuffer(0, getCompressedLength()).uncompress(Buffer::construct((u8*) vec.data(), getArrayLength() * (u32) sizeof(T) + 1));

					if (!result)
						return Log::error("Couldn't read FbxPropertyArray; couldn't uncompress array");

					buf = buf.offset(getCompressedLength());
					offset += getCompressedLength();
					return result;
				}

				for (u32 i = 0; i < getArrayLength(); ++i)
					if (!FbxPropertyHelper<T>::readOne(buf, offset, vec[i]))
						return Log::error("Couldn't read FbxPropertyArray; one of the elements was invalid");

				return true;
			}

			//This function loads data from a buffer like a char[] or a compressed char[], depending on this header
			//Since a std::vector<bool> is implemented as a bitset (low level), it requires a different function
			bool read(Buffer &buf, u32 &offset, std::vector<bool> &vec) {

				vec.resize(getArrayLength());

				if (getEncoding() != 0) {

					CopyBuffer tempVec((u32)vec.size());	//A vector that is dependable, and not a bitset

					bool result = buf.subbuffer(0, getCompressedLength()).uncompress(Buffer::construct((u8*) tempVec.addr(), getArrayLength() + 1));

					//Copy it into a bitset
					for (u32 i = 0; i < (u32)tempVec.size(); ++i)
						vec[i] = tempVec[i];

					buf = buf.offset(getCompressedLength());
					offset += getCompressedLength();
					return result;
				}

				for (u32 i = 0; i < getArrayLength(); ++i)
					if (!FbxPropertyHelper<bool>::readOne(buf, offset, vec[i]))
						return Log::error("Couldn't read FbxPropertyArray; one of the elements was invalid");

				return true;

			}

		};

		//Any data (String; S, Raw/Buffer; R) uses this header
		struct FbxPropertyDataHeader {

			u8 data[5];

			static constexpr u32 size() { return (u32) sizeof(data); }

			char getType() { return (char)data[0]; }
			u32 getLength() { return *(u32*)(data + 1); }

			//This function is used to parse the string(s) from a buffer
			bool read(Buffer &buf, u32 &offset, std::vector<String> &vec) {

				if (buf.size() < getLength())
					return Log::error("Couldn't read FbxProperty; Buffer didn't store the (full size) binary object");

				///TODO: [ 0x00, 0x01 ] is used to seperate strings

				vec.push_back(String(getLength(), (char*)buf.addr()));

				offset += getLength();
				buf = buf.offset(getLength());
				return true;
			}

			//This function is used to parse the subbuffer from a buffer
			bool read(Buffer &buf, u32 &offset, std::vector<Buffer> &vec) {

				if (buf.size() < getLength())
					return Log::error("Couldn't read FbxProperty; Buffer didn't store the (full size) binary object");

				vec.push_back(buf.subbuffer(0, getLength()));

				offset += getLength();
				buf = buf.offset(getLength());
				return true;
			}

		};

		//32-bit header of an FbxNode; only used before version 7.5
		struct FbxNodeHeader32 {

			u32 endOffset = 0xFFFFFFFF;
			u32 numProperties;
			u32 propertyListLen;

			u8 nameLen;

			static constexpr u32 size() { return 13; }		//To avoid padding

		};

		//64-bit header of an FbxNode; always used after version 7.5
		struct FbxNodeHeader64 {

			u64 endOffset = 0xFFFFFFFFFFFFFFFF;
			u64 numProperties;
			u64 propertyListLen;

			u8 nameLen;

			static constexpr u32 size() { return 25; }		//To avoid padding

			//We use 64-bit headers in FbxNode, but 32-bit should still be supported
			FbxNodeHeader64 &operator=(const FbxNodeHeader32 &other) {

				endOffset = other.endOffset;
				numProperties = other.numProperties;
				propertyListLen = other.propertyListLen;
				nameLen = other.nameLen;

				return *this;
			}

		};

		//The base class for all derived types
		class FbxProperty {

		public:

			FbxProperty() {}
			virtual ~FbxProperty() {}

			virtual char getCode() = 0;								//char identifier for the property
			virtual FbxProperty *clone() = 0;					//Cloning the contents of a property
			virtual bool read(Buffer &buf, u32 &offset) = 0;		//Reading the property from a buffer

			//Allocate and fill a property from buffer
			static FbxProperty *readProperty(Buffer &buffer, u32 &offset);

			template<typename T>
			T *cast();

		};

		typedef std::vector<FbxProperty*> FbxProperties;

		//The class that handles property info
		//char Id = identifier used in the binary file
		//Header = the header used for this type
		//T = the type that should be stored
		template<char Id, typename Header, typename T>
		class TFbxProperty : public FbxProperty {

		public:

			char getCode() override { return header.getType(); }

			static constexpr char getTypeCode() { return Id; }
			static constexpr u32 getHeaderSize() { return Header::size(); }

			//Get a value from this property
			//If this property has more values, you can use an index
			T &get(u32 i = 0) {
				return contents[i];
			}

			//Copy this property
			FbxProperty *clone() override {
				return (FbxProperty*) new TFbxProperty<Id, Header, T>(*this);
			}

			TFbxProperty() {
				memset(&header, 0, sizeof(header));
			}

			//Read from binary
			bool read(Buffer &buf, u32 &offset) override {

				if (buf.size() < Header::size())
					return Log::error(String("Couldn't read TFbxProperty (") + Id + "); buffer out of bounds");

				memcpy(header.data, buf.addr(), Header::size());
				buf = buf.offset(Header::size());
				offset += Header::size();

				return header.read(buf, offset, contents);
			}

			//Compare this property with a raw value (variable)
			bool operator==(const T &other) const {
				return contents.size() == 1 && contents[0] == other;
			}

			//Compare this property with raw values (array)
			bool operator==(const std::vector<T> &other) const {
				return contents == other;
			}

			//For any other compare, it returns false (but it is needed to compile)
			template<typename T2>
			bool operator==(const T2 &) const {
				return false;
			}

			//Returns nullptr for FbxBool or FbxBoolArray; because that's a bitset, not a C array
			T *getPtr() {
				return FbxPropertyHelper<T>::getPtr(contents);
			}

			u32 size() {
				return (u32) contents.size();
			}

		protected:

			Header header;
			std::vector<T> contents;

		};

		///These are the available types in the fbx file format

		typedef TFbxProperty<'Y', FbxPropertyHeader, i16> FbxShort;
		typedef TFbxProperty<'C', FbxPropertyHeader, bool> FbxBool;
		typedef TFbxProperty<'I', FbxPropertyHeader, i32> FbxInt;
		typedef TFbxProperty<'F', FbxPropertyHeader, f32> FbxFloat;
		typedef TFbxProperty<'D', FbxPropertyHeader, f64> FbxDouble;
		typedef TFbxProperty<'L', FbxPropertyHeader, i64> FbxLong;
		typedef TFbxProperty<'f', FbxPropertyArrayHeader, f32> FbxFloatArray;
		typedef TFbxProperty<'d', FbxPropertyArrayHeader, f64> FbxDoubleArray;
		typedef TFbxProperty<'l', FbxPropertyArrayHeader, i64> FbxLongArray;
		typedef TFbxProperty<'i', FbxPropertyArrayHeader, i32> FbxIntArray;
		typedef TFbxProperty<'b', FbxPropertyArrayHeader, bool> FbxBoolArray;
		typedef TFbxProperty<'R', FbxPropertyDataHeader, Buffer> FbxBuffer;
		typedef TFbxProperty<'S', FbxPropertyDataHeader, String> FbxString;

		//The type that will be used to hold our (compile-time) types
		template<typename ...args>
		struct TFbxTypes {};

		//A compile-time list of all of our types
		typedef TFbxTypes<
			FbxShort, FbxBool, FbxInt, FbxFloat, FbxDouble, FbxLong,
			FbxFloatArray, FbxDoubleArray, FbxLongArray, FbxIntArray, FbxBoolArray,
			FbxBuffer, FbxString
		> FbxTypes;

		struct FbxTypeCheck {

			//Check if an FbxProperty's type matches the type code
			template<char Id, typename Header, typename T>
			static bool check(TFbxProperty<Id, Header, T> prop, char c) {
				return c == Id;
			}

		};
		
		template<typename T>
		T *FbxProperty::cast() {

			if (FbxTypeCheck::template check(T{}, getCode()))
				return (T*) this;

			return (T*) Log::throwError<FbxProperty, 0x0>("Couldn't cast an FbxProperty");

		}

		//Allocate a type from a type code (non-last recursion)
		template<typename T, typename ...args>
		struct TFbxTypeAlloc {

			//Cascade down into the next if the type doesn't match
			static FbxProperty *get(char type) {

				if (FbxTypeCheck::template check(T{}, type))
					return new T();

				return TFbxTypeAlloc<args...>::get(type);
			}

		};

		//Allocate type from a type code (last recursion)
		template<typename T>
		struct TFbxTypeAlloc<T> {

			//Return nullptr if the type doesn't match
			static FbxProperty *get(char type) {

				if (FbxTypeCheck::template check(T{}, type))
					return new T();

				return nullptr;
			}

		};

		//A wrapper to allocate types more easily
		struct FbxTypeAlloc {

			//Instantiate the template loop from types
			template<typename ...args>
			static FbxProperty *allocate(TFbxTypes<args...>, char type) {
				return TFbxTypeAlloc<args...>::get(type);
			}

			//Allocate by type code
			static FbxProperty *allocate(char type) {
				return FbxTypeAlloc::template allocate(FbxTypes{}, type);
			}

		};

		class FbxNode;

		//Shortcut for multiple nodes
		typedef std::vector<FbxNode*> FbxNodes;

		//A class to handle an fbx node
		//A node contains subnodes and properties
		class FbxNode {

		public:

			//Clean up all children and properties
			~FbxNode() {

				for (FbxProperty *elem : properties)
					delete elem;

				for (FbxNode *elem : childs)
					delete elem;

			}

			FbxNode() {}
			FbxNode(FbxNodes nodes) : childs(nodes), name("Root") { }

			FbxNode(const FbxNode &other) {
				copy(other);
			}

			FbxNode &operator=(const FbxNode &other) {
				copy(other);
				return *this;
			}

			//Read a Buffer as an FbxNode and increment the Buffer by the required amount
			static FbxNode *read(Buffer &buf, u32 &offset, bool is64bit);

			//Read a Buffer's FbxNode(s)
			static FbxNodes readAll(Buffer &buf, u32 &offset, bool is64bit);

			u32 getChildren();
			FbxNode *getChild(u32 i);

			FbxNodes::iterator getChildBegin();
			FbxNodes::iterator getChildEnd();
			const FbxNodes &getChildArray();

			u32 getProperties();
			FbxProperty *getProperty(u32 i);

			FbxProperties::iterator getPropertyBegin();
			FbxProperties::iterator getPropertyEnd();
			const FbxProperties &getPropertyArray();

			String getName();

			//Path works just like file paths, but there can be nodes with the same path
			//Objects/Model is the path for all object data
			//Objects/Geometry is the path for all geometry data
			//For scanning on properties, use findNodes(path, { propertyId0, propertyId1, ... }, value0, value1, ...);
			FbxNodes findNodes(String path);

			//Example:
			//findNodes("Objects/Model", { 2 }, String("Mesh")) -> findMeshes()
			//findNodes("Objects/Model", { 2 }, String("Light")) -> findLights()
			//findNodes("Objects/Model", { 2 }, String("Camera")) -> findCameras()
			template<typename ...args>
			FbxNodes findNodes(String path, std::vector<u32> propertyIds, args... arg);

		private:

			//Find nodes with a path
			void findNodes(String path, const FbxNodes &loc, FbxNodes &target);

			//Copy all children and properties
			void copy(const FbxNode &other) {

				header = other.header;
				name = other.name;

				properties = other.properties;
				
				for (FbxProperty *&elem : properties)
					elem = elem->clone();

				childs = other.childs;

				for (FbxNode *&elem : childs)
					elem = new FbxNode(*elem);

			}

			FbxNodeHeader64 header;
			String name;

			FbxProperties properties;
			FbxNodes childs;

		};

		//Check if the property value matches (non-last recursion)
		template<typename T, typename ...args>
		struct FbxCheckPropertyValue_inner {

			//Loop over all types and find the type of FbxProperty*
			template<typename T2>
			static bool check(FbxProperty *prop, T2 t) {

				if (!FbxTypeCheck::template check(T{}, prop->getCode()))
					return FbxCheckPropertyValue_inner<args...>::template check(prop, t);

				T &castProperty = *(T*)prop;
				return castProperty == t;

			}

		};

		//Check if the property value matches (last recursion)
		template<typename T>
		struct FbxCheckPropertyValue_inner<T> {

			//Check if FbxProperty* matches this type then cast, else throw error
			template<typename T2>
			static bool check(FbxProperty *prop, T2 t) {

				if (!FbxTypeCheck::template check(T{}, prop->getCode()))
					return Log::error("FbxCheckPropertyValue couldn't find the requested type");

				T &castProperty = *(T*)prop;
				return castProperty == t;

			}

		};

		//Wrapper to check if property value matches
		struct FbxCheckPropertyValue {

			template<typename T, typename ...args>
			static bool check(TFbxTypes<args...>, FbxProperty *prop, T t) {
				return FbxCheckPropertyValue_inner<args...>::template check(prop, t);
			}

		};

		//Input the argument from the argument list and check if the value and type matches a property (non-last recursion)
		template<u32 i, typename T, typename ...args>
		struct FbxCheckProperty {

			static bool check(FbxProperty *prop, u32 index, T t, args... arg) {

				if (i != index)
					return FbxCheckProperty<i + 1, args...>::check(prop, index, arg...);

				return FbxCheckPropertyValue::template check(FbxTypes{}, prop, t);

			}

		};

		//Input the argument from the argument list and check if the value and type matches a property (last recursion)
		template<u32 i, typename T>
		struct FbxCheckProperty<i, T> {

			static bool check(FbxProperty *prop, u32 index, T t) {

				if (i != index)
					return Log::error("FbxCheckProperty couldn't find the right property; out of bounds");

				return FbxCheckPropertyValue::template check(FbxTypes{}, prop, t);

			}

		};
		template<typename ...args>
		FbxNodes FbxNode::findNodes(String path, std::vector<u32> propertyIds, args... arg) {

			if (propertyIds.size() != sizeof...(args)) {
				Log::error("Fbx::findNodes failed; propertyIds should have the same elements as type arguments");
				return {};
			}

			FbxNodes nodes = findNodes(path);
			FbxNodes result;

			for (FbxNode *node : nodes) {

				bool match = true;

				for (u32 i = 0; i < node->getProperties(); ++i) {

					FbxProperty *prop = node->getProperty(i);

					for (u32 j = 0; j < (u32)propertyIds.size(); ++j) {

						u32 k = propertyIds[j];

						if (i == k)
							match = match && FbxCheckProperty<0, args...>::check(prop, j, arg...);

					}

				}

				if (match)
					result.push_back(node);

			}

			return result;

		}


		//Class for storing and traversing fbx nodes
		class FbxFile {

		public:

			static FbxFile *read(Buffer fbxBuffer);		//Read from Buffer
			static FbxFile *read(String fbxPath);		//Read from String

			u32 getVersion();	//Get the version (7500 = 7.5, 7400 = 7.4)
			bool isValid();		//Checks if the header is valid

			FbxNode *get();

			FbxNodes findMeshes();
			FbxNodes findLights();
			FbxNodes findCameras();
			FbxNodes findGeometry();

			FbxFile(const FbxFile &other);
			FbxFile &operator=(const FbxFile &other);
			~FbxFile();

		protected:

			FbxFile(FbxHeader header, FbxNodes nodes);
			void copy(const FbxFile &other);

		private:

			FbxHeader header;
			FbxNode *root;

		};

		//We only accept converting fbx files
		//This is because an fbx can be either a scene or a mesh
		//An fbx is also too broad for its own good and nodes with properties make it
		//very slow to read and relatively complicated to parse
		//convertMesh picks all meshes (geometry) and converts them to oiRM format
		struct Fbx {

			static std::unordered_map<String, Buffer> convertMeshes(Buffer fbxBuffer, bool compression);
			static bool convertMeshes(Buffer fbxBuffer, String outPath, bool compression);

			static std::unordered_map<String, Buffer> convertMeshes(String fbxPath, bool compression);
			static bool convertMeshes(String fbxPath, String outPath, bool compression);

		private:

			static Vec3 getMaterialCol(FbxNode *nod, Vec3 def = {});
			static f32 getMaterialNum(FbxNode *nod, f32 def = 0.f);

		};

	}

}