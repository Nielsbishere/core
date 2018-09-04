#pragma once

#include "api/rapidjson/document.h"
#include "types/string.h"
#include "types/matrix.h"
#include "utils/log.h"

namespace oi {

	//JSON (rapid json wrapper):
	//+=+=+=+=+=+
	//Example JSON:
	//The JSON below will be used as showcase for the funcs
	//{ "testJson": { "object": [ [ 3, 3, 3 ], [ 4, 4, 4 ] ] } }
	//+=+=+=+=+=+
	//Funcs:
	//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
	//Getting members
	//========================
	//T getT(String path, T default)
	//Get a value from the JSON; path works like a file path:
	//get("testJson/object/1/0") will return 4
	//T &value; where T is either String, u64/u32, i64/i32, f64/f32, bool or std::vector<T>
	//@return bool success
	//----------------------
	//Querying folder structure:
	//========================
	//getMemberIds(String path) or getMemberNames(String path)
	//Get the members at the path; also works with arrays but shows them as 0->len-1
	//getMemberIds("testJson/object/1") will return [ "0", "1", "2" ]
	//getMemberNames("testJson/object/1") will return the names after the path; so element 0 would be "testJson/object/1/0"
	//getAllMembers is recursive and lists the names, not ids.
	//getMembers returns the number of members
	//----------------------
	//'File' system:
	//========================
	//exists(String path)
	//Returns whether or not a 'directory' or 'file' has been made yet
	//----------------------
	//mkdir(String path, bool useLists = true)
	//Ensures that all objects are made so that the directory at 'path' exists
	//mkdir steps into directories until it can't find the directory, then it will start creating.
	//If it encounters an array, it will create enough empty space before the index to fill it up.
	//A new array is only made when 'useLists' is true, otherwise it will assume new directories to always be a name
	//"testJson/object/3/1" would reference to the 4th index in an array; which doesn't exist yet; it would make two empty objects
	//Then it would turn the 4th object into an array (because '1' is a number); but only if 'useLists' is true.
	//It returns false if it hit an object that doesn't have contents (so numbers & strings)
	//this means that mkdir("testJson/object/3/id") will still jump into the object array and add an element (even if you turned off 'assumeNumberIsList')
	//mkdir("testJson/object/3/0") will result in the following JSON:
	//{ "testJson": { "object": [ [ 3, 3, 3 ], [ 4, 4, 4 ], {}, [ {} ] ] } }
	//Empty objects can be replaced by any object.
	//----------------------
	//Setting members:
	//========================
	//bool setT(String path, T value, bool assumeNumberIsList = true)
	//Set the member at the path; uses mkdir if the directory is not created yet
	//'assumeNumberIsList' is used to check if uint's indicate an index in a list (see mkdir)
	//----------------------
	class JSON {

		template<typename T, bool b> friend struct JSONSerialize;

	public:

		template<typename T>
		void serialize(String path, T &val, bool save) {
			JSONSerialize<T>::serialize(*this, path, val, save);
		}

		template<typename T>
		void set(String path, const T value) {
			JSONSerialize<T>::serialize(*this, path, (T&)value, true);
		}

		template<typename T>
		T get(String path) {
			T val;
			JSONSerialize<T>::serialize(*this, path, val, false);
			return val;
		}

		u32 getMembers(String path = "") const;
		std::vector<String> getMemberNames(String path) const;
		std::vector<String> getMemberIds(String path) const;
		bool exists(String path) const;
		bool mkdir(String dir, bool useLists = true);

		void getAllMembers(String path, std::vector<String> &out) const;

		JSON(const String fromString);
		JSON();

		String toString(bool pretty = true) const;

	protected:

		template<typename T>
		bool get_inter(String path, T &value, T def) const {
			value = def;

			const rapidjson::Value *val;
			if (!getValue(path, val)) return false;

			return isValid(val, value);
		}

		template<typename T>
		bool set_inter(String path, T value, bool useLists) {

			if (!mkdir(path, useLists)) return false;

			rapidjson::Value *val;
			if (!getValue(path, val)) return false;

			if ((val->IsObject() && val->MemberCount() == 0) || (!val->IsObject() && !val->IsArray())) {
				rapidjson::Value &valr = *val;
				valr = rapidjson::Value(value);
			}

			return true;
		}

		bool set_inter(String path, String value, bool useLists);

		bool getValue(String path, rapidjson::Value *&val);
		bool getValue(String path, const rapidjson::Value *&val) const;

		template<typename T>
		bool isValid(const rapidjson::Value *val, std::vector<T> &res) const {
			
			if(!val->IsArray()) return false;

			auto arr = val->GetArray();

			std::vector<T> tres(arr.Size());

			for (u32 i = 0; i < arr.Size(); ++i)
				if (!isValid(&arr[i], tres[i])) return false;

			res = std::move(tres);
		
			return true;
		}

		bool isValid(const rapidjson::Value *val, String &res) const;
		bool isValid(const rapidjson::Value *val, bool &res) const;
		bool isValid(const rapidjson::Value *val, f64 &res) const;
		bool isValid(const rapidjson::Value *val, f32 &res) const;
		bool isValid(const rapidjson::Value *val, i64 &res) const;
		bool isValid(const rapidjson::Value *val, i32 &res) const;
		bool isValid(const rapidjson::Value *val, u64 &res) const;
		bool isValid(const rapidjson::Value *val, u32 &res) const;

	private:

		rapidjson::Document json;

	};

	//For any function
	template<typename T, bool isDataType = std::is_arithmetic<T>::value>
	struct JSONSerialize {

		static void serialize(JSON &json, String path, T &val, bool save) {
			val.serialize(json, path, save);
		}

	};

	//For vectors
	template<typename T>
	struct JSONSerialize<std::vector<T>, false> {

		static void serialize(JSON &json, String path, std::vector<T> &val, bool save) {

			if (!save)
				val.resize(json.getMembers(path));

			for (u32 i = 0; i < (u32)val.size(); ++i)
				JSONSerialize<T>::serialize(json, path + "/" + i, val[i], save);

		}

	};

	template<typename T, u32 n>
	struct JSONSerialize<oi::TVec<T, n>, false> {

		static void serialize(JSON &json, String path, oi::TVec<T, n> &val, bool save) {
			for (u32 i = 0; i < n; ++i)
				JSONSerialize<T>::serialize(json, path + "/" + i, val[i], save);
		}

	};

	template<typename T, u32 x, u32 y>
	struct JSONSerialize<oi::TMatrix<T, x, y>, false> {
		static void serialize(JSON &json, String path, oi::TMatrix<T, x, y> &val, bool save) {
			for (u32 i = 0; i < x * y; ++i)
				JSONSerialize<T>::serialize(json, path + "/" + (i % x) + "/" + (j / x), val[i], save);
		}
	};

	//For data types
	template<typename T>
	struct JSONSerialize<T, true> {

		static void serialize(JSON &json, String path, T &val, bool save) {
			if (save)
				json.set_inter(path, val, true);
			else
				json.get_inter(path, val, (T) 0);
		}

	};

	//For strings
	template<>
	struct JSONSerialize<String, false> {

		static void serialize(JSON &json, String path, String &val, bool save) {
			if (save)
				json.set_inter(path, val, true);
			else
				json.get_inter(path, val, {});
		}

	};

	template<>
	struct JSONSerialize<const char*, false> {

		static void serialize(JSON &json, String path, const char* val, bool save) {
			if (save)
				json.set_inter(path, String(val), true);
			else
				Log::throwError<JSONSerialize<const char*, false>, 0x0>("JSONSerialize read not possible with a char*");
		}

	};

}