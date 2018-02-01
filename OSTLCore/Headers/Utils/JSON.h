#pragma once

#include "API/rapidjson/document.h"
#include "Types/OString.h"
#include "Utils/Log.h"

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
	//T getT(OString path, T default)
	//Get a value from the JSON; path works like a file path:
	//get("testJson/object/1/0") will return 4
	//T &value; where T is either OString, u64/u32, i64/i32, f64/f32, bool or std::vector<T>
	//@return bool success
	//----------------------
	//Querying folder structure:
	//========================
	//getMemberIds(OString path) or getMemberNames(OString path)
	//Get the members at the path; also works with arrays but shows them as 0->len-1
	//getMemberIds("testJson/object/1") will return [ "0", "1", "2" ]
	//getMemberNames("testJson/object/1") will return the names after the path; so element 0 would be "testJson/object/1/0"
	//getAllMembers is recursive and lists the names, not ids.
	//getMembers returns the number of members
	//----------------------
	//'File' system:
	//========================
	//exists(OString path)
	//Returns whether or not a 'directory' or 'file' has been made yet
	//----------------------
	//mkdir(OString path, bool useLists = true)
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
	//bool setT(OString path, T value, bool assumeNumberIsList = true)
	//Set the member at the path; uses mkdir if the directory is not created yet
	//'assumeNumberIsList' is used to check if uint's indicate an index in a list (see mkdir)
	//----------------------
	class JSON {

	public:

		OString getString(OString path, OString def = "") { get_inter(path, def, def); return def; }
		i32 getInt(OString path, i32 def = 0) { get_inter(path, def, def); return def; }
		i64 getLong(OString path, i64 def = 0) { get_inter(path, def, def); return def; }
		u32 getUInt(OString path, u32 def = 0U) { get_inter(path, def, def); return def; }
		u64 getULong(OString path, u64 def = 0U) { get_inter(path, def, def); return def; }
		f32 getFloat(OString path, f32 def = 0.f) { get_inter(path, def, def); return def; }
		f64 getDouble(OString path, f64 def = 0.0) { get_inter(path, def, def); return def; }
		bool getBool(OString path, bool def = false) { get_inter(path, def, def); return def; }

		bool setString(OString path, OString o, bool useLists = true) { return set_inter(path, o, useLists); }
		bool setInt(OString path, i32 o, bool useLists = true) { return set_inter(path, o, useLists); }
		bool setLong(OString path, i64 o, bool useLists = true) { return set_inter(path, o, useLists); }
		bool setUInt(OString path, u32 o, bool useLists = true) { return set_inter(path, o, useLists); }
		bool setULong(OString path, u64 o, bool useLists = true) { return set_inter(path, o, useLists); }
		bool setFloat(OString path, f32 o, bool useLists = true) { return set_inter(path, o, useLists); }
		bool setDouble(OString path, f64 o, bool useLists = true) { return set_inter(path, o, useLists); }
		bool setBool(OString path, bool o, bool useLists = true) { return set_inter(path, o, useLists); }

		template<typename T>
		bool getList(OString path, std::vector<T> &value, std::vector<T> def = std::vector<T>()) {
			return get_inter(path, value, def);
		}

		template<typename T>
		bool setList(OString path, std::vector<T> &o, bool useLists = true) {

			for (u32 i = 0; i < o.size(); ++i)
				if (!set_inter(path + "/" + i, o[i], useLists)) return false;

			return true;
		}

		u32 getMembers(OString path = "");
		std::vector<OString> getMemberNames(OString path);
		std::vector<OString> getMemberIds(OString path);
		bool exists(OString path);
		bool mkdir(OString dir, bool useLists = true);

		void getAllMembers(OString path, std::vector<OString> &out);

		JSON(OString fromString);
		JSON();
		operator OString();

	public:


		template<typename T>
		bool get_inter(OString path, T &value, T def) {
			value = def;

			rapidjson::Value *val;
			if (!getValue(path, val)) return false;

			bool b = isValid(val, value);
			return b;
		}

		template<typename T>
		bool set_inter(OString path, T value, bool useLists) {

			if (!mkdir(path, useLists)) return false;

			rapidjson::Value *val;
			if (!getValue(path, val)) return false;

			if ((val->IsObject() && val->MemberCount() == 0) || (!val->IsObject() && !val->IsArray())) {
				rapidjson::Value &valr = *val;
				valr = rapidjson::Value(value);
			}

			return true;
		}

		template<>
		bool set_inter(OString path, OString value, bool useLists) {

			if (!mkdir(path, useLists)) return false;

			rapidjson::Value *val;
			if (!getValue(path, val)) return false;

			if ((val->IsObject() && val->MemberCount() == 0) || (!val->IsObject() && !val->IsArray())) {
				rapidjson::Value &valr = *val;
				valr = rapidjson::Value(value.c_str(), json.GetAllocator());
			}

			return true;
		}

		bool getValue(OString path, rapidjson::Value *&val);

		template<typename T>
		bool isValid(rapidjson::Value *val, T &res) { return false; }

		template<typename T>
		bool isValid(rapidjson::Value *val, std::vector<T> &res) {
			
			if(!val->IsArray()) return false;

			auto arr = val->GetArray();

			std::vector<T> tres(arr.Size());

			for (u32 i = 0; i < arr.Size(); ++i)
				if (!isValid(&arr[i], tres[i])) return false;

			res = std::move(tres);
		
			return true;
		}

		template<>
		bool isValid(rapidjson::Value *val, OString &res) {

			if (val->IsString()) {
				res = val->GetString();
				return true;
			}

			return false;
		}

		template<>
		bool isValid(rapidjson::Value *val, bool &res) {

			if (val->IsBool()) {
				res = val->GetBool();
				return true;
			}

			return false;
		}

		template<>
		bool isValid(rapidjson::Value *val, f64 &res) {

			if (val->IsNumber()) {
				res = val->GetDouble();
				return true;
			}

			return false;
		}

		template<>
		bool isValid(rapidjson::Value *val, f32 &res) {

			if (val->IsNumber()) {
				res = val->GetFloat();
				return true;
			}

			return false;
		}

		template<>
		bool isValid(rapidjson::Value *val, i64 &res) {

			if (val->IsNumber()) {
				res = val->GetInt64();
				return true;
			}

			return false;
		}

		template<>
		bool isValid(rapidjson::Value *val, i32 &res) {

			if (val->IsNumber()) {
				res = val->GetInt();
				return true;
			}

			return false;
		}

		template<>
		bool isValid(rapidjson::Value *val, u64 &res) {

			if (val->IsNumber()) {
				res = val->GetInt64() < 0 ? 0 : (u64)val->GetInt64();
				return true;
			}

			return false;
		}

		template<>
		bool isValid(rapidjson::Value *val, u32 &res) {

			if (val->IsNumber()) {
				res = val->GetInt() < 0 ? 0 : (u32)val->GetInt();
				return true;
			}

			return false;
		}

	private:

		rapidjson::Document json;

	};

}