#include "utils/json.h"
#include "api/rapidjson/stringbuffer.h"
#include "api/rapidjson/prettywriter.h"
using namespace oi;

JSON::JSON(const String fromString) {
	json.Parse(fromString.toCString());
}

JSON::JSON() : JSON("{}") {}

String JSON::toString(bool pretty) const {

	rapidjson::StringBuffer buffer;

	buffer.Clear();

	if (!pretty) {
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		json.Accept(writer);
	} else {
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		json.Accept(writer);
	}

	return buffer.GetString();
}

u32 JSON::getMembers(String path) const {

	if (path == "")
		return json.MemberCount();

	const rapidjson::Value *val;
	if (!getValue(path, val) || !(val->IsArray() || val->IsObject())) return 0;

	if (val->IsArray()) return (u32)val->GetArray().Size();

	return (u32)val->MemberCount();
}

bool JSON::getValue(String path, const rapidjson::Value *&oval) const {

	if (path == "" || !json.IsObject()) return false;

	u32 i = 0;
	const rapidjson::Value *val = nullptr;
	auto arr = path.split("/");

	for (String sub : arr) {

		if (i == 0)
			if (!json.HasMember(sub.toCString())) return false;
			else 
				val = &json.FindMember(sub.toCString())->value;
		else {

			if (val->IsArray()) {
				if (!sub.isUint() || sub.toLong() >= val->GetArray().Size()) return false;
				val = &val->GetArray()[(rapidjson::SizeType)sub.toLong()];
				continue;
			}

			if (!val->IsObject()) {

				if (i == arr.size() - 1) {
					oval = val;
					return true;
				}
				else
					return false;

			}

			if (!val->HasMember(sub.toCString())) return false;
			val = &(*val)[sub.toCString()];
		}

		++i;
	}

	oval = val;
	return true;
}

bool JSON::getValue(String path, rapidjson::Value *&val) {
	const rapidjson::Value *tval;
	bool b = getValue(path, tval);
	val = (rapidjson::Value*)tval;
	return b;
}

std::vector<String> JSON::getMemberNames(String path) const {

	std::vector<String> res;

	if (path == "")
		for (u32 i = 0; i < json.MemberCount(); ++i)
			res.push_back((json.MemberBegin() + i)->name.GetString());

	const rapidjson::Value *val;
	if (!getValue(path, val)) return res;

	if (val->IsArray()) {
		for (u32 i = 0; i < val->GetArray().Size(); ++i)
			res.push_back(path + "/" + i);
	}

	if (!val->IsObject()) return res;

	for (u32 i = 0; i < val->MemberCount(); ++i)
		res.push_back(path + "/" + (val->MemberBegin() + i)->name.GetString());

	return res;
}

std::vector<String> JSON::getMemberIds(String path) const {

	std::vector<String> res;

	if (path == "")
		for (u32 i = 0; i < json.MemberCount(); ++i)
			res.push_back((json.MemberBegin() + i)->name.GetString());

	const rapidjson::Value *val;
	if (!getValue(path, val)) return res;

	if (val->IsArray()) {
		for (u32 i = 0; i < val->GetArray().Size(); ++i)
			res.push_back(i);
	}

	if (!val->IsObject()) return res;

	for (u32 i = 0; i < val->MemberCount(); ++i)
		res.push_back((val->MemberBegin() + i)->name.GetString());

	return res;
}

void JSON::getAllMembers(String path, std::vector<String> &out) const {

	if (path == "")
		for (u32 i = 0; i < json.MemberCount(); ++i) {
			String tpath = (json.MemberBegin() + i)->name.GetString();
			out.push_back(tpath);
			getAllMembers(tpath, out);
		}

	const rapidjson::Value *val;
	if (!getValue(path, val)) return;

	if (val->IsArray()) {
		for (u32 i = 0; i < val->GetArray().Size(); ++i) {
			String tpath = path + "/" + i;
			out.push_back(tpath);
			getAllMembers(tpath, out);
		}
	}

	if (!val->IsObject())
		return;

	for (u32 i = 0; i < val->MemberCount(); ++i) {
		String tpath = path + "/" + (val->MemberBegin() + i)->name.GetString();
		out.push_back(tpath);
		getAllMembers(tpath, out);
	}
}

bool JSON::exists(String path) const {

	if (path == "") return true;

	const rapidjson::Value *oval;
	return getValue(path, oval);
}

bool JSON::mkdir(String path, bool useLists) {

	if (exists(path)) return true;

	u32 i = 0;
	String cpath;

	const rapidjson::Value *val;

	auto addElem = [&](rapidjson::Value *tval, String name, rapidjson::Type type = rapidjson::Type::kObjectType) { tval->AddMember(rapidjson::Value(name.toCString(), name.size(), json.GetAllocator()), rapidjson::Value(type), json.GetAllocator()); };

	auto addElemToArr = [&](rapidjson::GenericArray<false, rapidjson::Value> &arr, rapidjson::Type type = rapidjson::Type::kObjectType) {
		arr.PushBack(rapidjson::Value(type), json.GetAllocator());
	};

	for (String sub : path.split("/")) {

		String tpath = cpath;
		cpath = (i == 0 ? "" : cpath + "/") + sub;

		bool isListIndex = sub.isUint();

		if (!getValue(cpath, val)) {

			if (i > 0) {

				rapidjson::Value *tval;
				getValue(tpath, tval);

				if (useLists && tval->IsObject() && tval->MemberCount() == 0 && sub.isUint()) {
					rapidjson::Value &tvalr = *tval;
					tvalr = rapidjson::Value(rapidjson::Type::kArrayType);
				}

				if (tval->IsObject())
					addElem(tval, sub);
				 else if (tval->IsArray()) {

					auto arr = tval->GetArray();
					u32 arrSize = (u32) arr.Size();

					if (isListIndex) {

						i64 num = sub.toLong();

						if(num < 0)
							return Log::error(String("Couldn't mkdir \"") + cpath + "\" arrays range from [0, u32_MAX]");

						for (u32 i = arrSize; i <= num; ++i)
							addElemToArr(arr);
					}
					else
						return Log::error(String("Couldn't mkdir \"") + cpath + "\" it didn't use an index to reference to something in an array");

				} else 
					return Log::error(String("Couldn't mkdir \"") + cpath + "\" it was a data type, not an object or list");

			} else 
				json.AddMember(rapidjson::Value(sub.toCString(), sub.size(), json.GetAllocator()), rapidjson::Value(rapidjson::Type::kObjectType), json.GetAllocator());

		}

		++i;
	}

	return true;
}

bool JSON::isValid(const rapidjson::Value *val, String &res) const {

	if (val->IsString()) {
		res = val->GetString();
		return true;
	}

	return false;
}

bool JSON::isValid(const rapidjson::Value *val, bool &res) const {

	if (val->IsBool()) {
		res = val->GetBool();
		return true;
	}

	return false;
}

bool JSON::isValid(const rapidjson::Value *val, f64 &res) const {

	if (val->IsNumber()) {
		res = val->GetDouble();
		return true;
	}

	return false;
}

bool JSON::isValid(const rapidjson::Value *val, f32 &res) const {

	if (val->IsNumber()) {
		res = val->GetFloat();
		return true;
	}

	return false;
}

bool JSON::isValid(const rapidjson::Value *val, i64 &res) const {

	if (val->IsNumber()) {
		res = val->GetInt64();
		return true;
	}

	return false;
}

bool JSON::isValid(const rapidjson::Value *val, i32 &res) const {

	if (val->IsNumber()) {
		res = val->GetInt();
		return true;
	}

	return false;
}

bool JSON::isValid(const rapidjson::Value *val, u64 &res) const {

	if (val->IsNumber()) {
		res = val->GetInt64() < 0 ? 0 : (u64)val->GetInt64();
		return true;
	}

	return false;
}

bool JSON::isValid(const rapidjson::Value *val, u32 &res) const {

	if (val->IsNumber()) {
		res = val->GetInt() < 0 ? 0 : (u32)val->GetInt();
		return true;
	}

	return false;
}

bool JSON::set_inter(String path, String value, bool useLists) {

	if (!mkdir(path, useLists)) return false;

	const rapidjson::Value *val;
	if (!getValue(path, val)) return false;

	if ((val->IsObject() && val->MemberCount() == 0) || (!val->IsObject() && !val->IsArray())) {
		rapidjson::Value &valr = *(rapidjson::Value*)val;
		valr = rapidjson::Value(value.toCString(), json.GetAllocator());
	}

	return true;
}