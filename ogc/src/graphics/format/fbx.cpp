#include "graphics/format/fbx.h"

#include <utils/log.h>
#include <file/filemanager.h>

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

FbxProperty *FbxProperty::readProperty(Buffer &buffer, u32 &offset) {

	if (buffer.size() == 0)
		return (FbxProperty*) Log::error("Couldn't read FbxProperty; buffer was null");

	char type = buffer[0];

	FbxProperty *allocated = FbxTypeAlloc::allocate(type);

	if(allocated == nullptr)
		return (FbxProperty*) Log::error("Couldn't allocate FbxProperty; invalid type");

	if (!allocated->read(buffer, offset)) {
		delete allocated;
		return (FbxProperty*)Log::error("Couldn't read FbxProperty");
	}

	return allocated;
}

FbxNode *FbxNode::read(Buffer &buf, u32 &offset, bool is64bit) {

	u32 headerSize = is64bit ? FbxNodeHeader64::size() : FbxNodeHeader32::size();

	if (buf.size() < headerSize)
		return (FbxNode*) Log::error("Couldn't read FbxNode; invalid buffer size");

	FbxNode *node = new FbxNode();
	node->childs.reserve(16);

	if (is64bit) {

		memset(&node->header, 0, headerSize);
		memcpy(&node->header, buf.addr(), headerSize);

		buf = buf.offset(headerSize);
		offset += headerSize;

	} else {

		FbxNodeHeader32 header;

		memset(&header, 0, headerSize);
		memcpy(&header, buf.addr(), headerSize);

		buf = buf.offset(headerSize);
		offset += headerSize;

		node->header = header;

	}

	if (buf.size() < node->header.nameLen)
		return (FbxNode*) Log::error("Couldn't read FbxNode's name; invalid buffer size");

	node->name = String((char*) buf.addr(), node->header.nameLen);
	buf = buf.offset(node->header.nameLen);
	offset += node->header.nameLen;

	u32 i = 0;
	FbxProperty *prop;

	while (i < node->header.numProperties) {

		if((prop = FbxProperty::readProperty(buf, offset)) != nullptr)
			node->properties.push_back(prop);
		else 
			return (FbxNode*) Log::error(String("Couldn't read FbxNode's properties (") + node->name + ")");

		++i;
	}

	FbxNode *subnode;

	while (offset < node->header.endOffset) {

		if ((subnode = FbxNode::read(buf, offset, is64bit)) == nullptr)
			return (FbxNode*) Log::error(String("Couldn't read FbxNode's child nodes (") + node->name + ")");

		if (subnode->header.nameLen != 0)
			node->childs.push_back(subnode);

	}

	return node;

}

FbxNodes FbxNode::readAll(Buffer &buf, u32 &offset, bool is64bit) {

	FbxNode *node = nullptr;
	FbxNodes nodes;
	nodes.reserve(16);

	while (buf.size() > 0) {

		if ((node = read(buf, offset, is64bit)) == nullptr) {
			Log::error("Couldn't read an fbx node");
			return {};
		}

		if (node->header.nameLen != 0)
			nodes.push_back(node);
		else
			break;

	}

	return nodes;

}

void FbxFile::findNodes(String path, const FbxNodes &loc, FbxNodes &target) {

	String term = path.untilFirst("/");
	String child = path.fromFirst("/");

	for(FbxNode *node : loc)
		if (node->getName() == term) {
			if (child != "")
				findNodes(child, node->getChildArray(), target);
			else
				target.push_back(node);
		}

}

FbxNodes FbxFile::findNodes(String path) {
	FbxNodes result;
	findNodes(path, nodes, result);
	return result;

}

FbxFile::~FbxFile() {
	for (FbxNode *node : nodes)
		delete node;
}

void FbxFile::copy(const FbxFile &other) {

	header = other.header;
	nodes = other.nodes;

	for (FbxNode *&node : nodes)
		node = new FbxNode(*node);

}

FbxFile::FbxFile(const FbxFile &other) { copy(other); }
FbxFile &FbxFile::operator=(const FbxFile &other) { copy(other); return *this; }

FbxNodes FbxFile::findMeshes() { return findNodes("Objects/Model", { 2 }, String("Mesh")); }
FbxNodes FbxFile::findLights() { return findNodes("Objects/Model", { 2 }, String("Light")); }
FbxNodes FbxFile::findCameras() { return findNodes("Objects/Model", { 2 }, String("Camera")); }
FbxNodes FbxFile::findGeometry() { return findNodes("Objects/Geometry"); }
FbxFile::FbxFile(FbxHeader header, FbxNodes nodes) : header(header), nodes(nodes) {}

u32 FbxNode::getChildren() { return (u32) childs.size(); }
FbxNode &FbxNode::getChild(u32 i) { return *childs[i]; }
u32 FbxNode::getProperties(){ return (u32) properties.size(); }
FbxProperty *FbxNode::getProperty(u32 i) { return properties[i]; }
String FbxNode::getName() { return name; }

FbxProperties::iterator FbxNode::getPropertyBegin() { return properties.begin(); }
FbxProperties::iterator FbxNode::getPropertyEnd() { return properties.end(); }
const FbxProperties &FbxNode::getPropertyArray() { return properties; }
FbxNodes::iterator FbxNode::getChildBegin() { return childs.begin(); }
FbxNodes::iterator FbxNode::getChildEnd() { return childs.end(); }
const FbxNodes &FbxNode::getChildArray() { return childs; }

u32 FbxFile::getVersion() { return header.getVersion(); }
bool FbxFile::isValid() { return header.getHeader() == "Kaydara FBX Binary  " && header.getUnknown() == 0x1A; }

u32 FbxFile::getNodeCount() { return (u32) nodes.size(); }
FbxNode *FbxFile::getNode(u32 id) { return nodes[id]; }

FbxFile *FbxFile::read(Buffer buf) {

	goto binary;

binary:

	if (buf.size() < FbxHeader::size())
		return (FbxFile*) Log::error("Couldn't read Fbx; invalid buffer size");

	FbxHeader head;

	memset(&head, 0, sizeof(head));
	memcpy(&head, buf.addr(), FbxHeader::size());

	u32 offset = (u32)FbxHeader::size();
	buf = buf.offset(offset);

	if (head.getHeader() != "Kaydara FBX Binary  " || head.getUnknown() != 0x001A)
		return (FbxFile*) Log::error("Couldn't read Fbx; invalid header");

	FbxNodes nodes = FbxNode::readAll(buf, offset, head.getVersion() >= 7500 /* Starting from version 7.5, they use 64-bit format for nodes*/);

	if (nodes.size() != 0)
		Log::println(String("Successfully read fbx file with version ") + head.getVersion());

	return new FbxFile(head, nodes);
}

FbxFile *FbxFile::read(String fbxPath) {

	Buffer buf;
	FileManager::get()->read(fbxPath, buf);

	if (buf.size() == 0) {
		Log::error("Couldn't read from file");
		return {};
	}

	FbxFile *file = read(buf);
	buf.deconstruct();
	return file;

}

std::vector<Buffer> Fbx::convertMeshes(Buffer buf) {

	FbxFile *file = FbxFile::read(buf);

	std::vector<Buffer> meshes;

	FbxNodes geometry = file->findGeometry();

	for (FbxNode *node : geometry);

	return meshes;

}


bool Fbx::convertMeshes(Buffer fbxBuffer, String outPath) {

	std::vector<Buffer> buf = convertMeshes(fbxBuffer);

	String base = outPath.getFilePath();
	String extension = outPath.getExtension();

	if (buf.size() == 0)
		return Log::error("Fbx conversion failed (or it didn't contain any meshes)");

	for (u32 i = 0; i < (u32)buf.size(); ++i) {

		if (!FileManager::get()->write(i == 0 ? outPath : (base + "." + (i - 1) + "." + extension), buf[i])) {

			for (Buffer b : buf)
				b.deconstruct();

			return Log::error("Couldn't write oiRM file to disk");
		}

	}

	for (Buffer b : buf)
		b.deconstruct();

	return true;
}

std::vector<Buffer> Fbx::convertMeshes(String fbxPath) {

	Buffer buf;
	FileManager::get()->read(fbxPath, buf);

	if (buf.size() == 0) {
		Log::error("Couldn't read from file");
		return {};
	}

	return convertMeshes(buf);

}

bool Fbx::convertMeshes(String fbxPath, String outPath) {

	Buffer buf;
	FileManager::get()->read(fbxPath, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't read from file");

	bool converted = convertMeshes(buf, outPath);
	buf.deconstruct();
	return converted;
}