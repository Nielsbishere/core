#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#define RAPIDJSON
#include "utils/json.h"
using namespace oi;

JSON::JSON(const String fromString): JSON() {
	json->Parse(fromString.begin());
	reconstruct();
}

JSON::~JSON() { delete json; }
JSON::JSON() : JSONNode(new rapidjson::Document(), nullptr) { json->Parse("{}"); }

void JSON::copy(const JSON &other) {
	json = new rapidjson::Document();
	json->CopyFrom(*other.json, json->GetAllocator(), true);
	value = nullptr;
	reconstruct();
}

u64 JSONNodeUtils::getUInt(rapidjson::Value *node) { return node->GetUint64(); }
i64 JSONNodeUtils::getInt(rapidjson::Value *node) { return node->GetInt64(); }
f64 JSONNodeUtils::getFloat(rapidjson::Value *node) { return node->GetDouble(); }
bool JSONNodeUtils::getBool(rapidjson::Value *node) { return node->GetBool(); }
void JSONNodeUtils::setUInt(rapidjson::Value *node, u64 val) { node->SetUint64(val); }
void JSONNodeUtils::setInt(rapidjson::Value *node, i64 val) { node->SetInt64(val); }
void JSONNodeUtils::setFloat(rapidjson::Value *node, f64 val) { node->SetDouble(val); }
void JSONNodeUtils::setBool(rapidjson::Value *node, bool val) { node->SetBool(val); }
String JSONNodeUtils::getString(rapidjson::Value *node) { return node->GetString(); }
void JSONNodeUtils::setString(rapidjson::Value *node, String str) { node->SetString(str.begin(), (rapidjson::SizeType)str.size()); }

void JSONNode::reconstruct() {

	children.clear();

	if (value == nullptr) {
		for (auto it = json->MemberBegin(); it != json->MemberEnd(); ++it) {
			JSONNode &node = children[it->name.GetString()];
			node.json = json;
			node.value = &it->value;
			node.reconstruct();
		}
	} else {
		if (value->IsObject()) {
			for (auto it = value->MemberBegin(); it != value->MemberEnd(); ++it) {
				JSONNode &node = children[it->name.GetString()];
				node.json = json;
				node.value = &it->value;
				node.reconstruct();
			}
		} else if(value->IsArray()){
			auto arr = value->GetArray();
			u32 i = 0;
			for (auto it = arr.Begin(); it != arr.End(); ++it) {
				JSONNode &node = children[i];
				node.json = json;
				node.value = it;
				node.reconstruct();
				++i;
			}
		}
	}

}

JSONNode &JSONNode::operator[](String str) {

	if (str.size() == 0)
		return *this;

	if (str.isUint() && canChangeType())
		value->SetArray();

	auto it = children.find(str);

	if (it == children.end()) {
		mkdir(str);
		return children[str];
	}

	return it->second;

}

const JSONNode &JSONNode::operator[](String str) const {

	if (str.size() == 0)
		return *this;

	auto it = children.find(str);

	if (it == children.end())
		Log::throwError<JSONNode, 0x0>("Couldn't find JSONNode");

	return it->second;

}

u32 JSONNode::getMembers() const { return (u32) children.size(); }

bool JSONNode::exists(String var) const {

	auto it = children.find(var);

	if (it == children.end())
		return false;

	return true;

}

bool JSONNode::mkdir(String var) {

	if (!exists(var)) {

		JSONNode &node = children[var];
		node.json = json;

		if (value == nullptr) {
			json->AddMember(rapidjson::Value(var.begin(), (rapidjson::SizeType) var.size(), json->GetAllocator()), rapidjson::Value(rapidjson::Type::kObjectType), json->GetAllocator());
			node.value = &json->FindMember(var.begin())->value;
		} else if (value->IsArray()) {

			u32 j = (u32) var;

			auto arr = value->GetArray();

			for (u32 i = arr.Size(); i < j + 1; ++i)
				arr.PushBack(rapidjson::Value(rapidjson::Type::kObjectType), json->GetAllocator());

			node.value = &arr[j];

		} else {
			value->AddMember(rapidjson::Value(var.begin(), (rapidjson::SizeType) var.size(), json->GetAllocator()), rapidjson::Value(rapidjson::Type::kObjectType), json->GetAllocator());
			node.value = &value->FindMember(var.begin())->value;
		}
	}

	return true;
}

String JSONNode::toString(bool pretty) const {

	rapidjson::StringBuffer buffer;

	buffer.Clear();

	if (!pretty) {

		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		if (value != nullptr)
			value->Accept(writer);
		else
			json->Accept(writer);

	} else {

		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

		if (value != nullptr)
			value->Accept(writer);
		else
			json->Accept(writer);

	}

	return buffer.GetString();
}

bool JSONNode::isEmpty() {
	return value->IsObject() && value->MemberCount() == 0;
}

rapidjson::Type JSONNode::getType() {
	return value == nullptr ? json->GetType() : value->GetType();
}

bool JSONNode::canChangeType() {
	return value != nullptr && isEmpty();
}

bool JSONNode::isObject() {
	return value == nullptr || value->IsObject();
}

bool JSONNode::isList() {
	return value == nullptr || value->IsArray();
}