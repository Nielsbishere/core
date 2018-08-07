#include "graphics/format/fbx.h"

#include <utils/log.h>
#include <file/filemanager.h>
#include <types/vector.h>

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

void FbxNode::findNodes(String path, const FbxNodes &loc, FbxNodes &target) {

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

FbxNodes FbxNode::findNodes(String path) {
	FbxNodes result;
	findNodes(path, childs, result);
	return result;

}

FbxFile::~FbxFile() {
	delete root;
}

void FbxFile::copy(const FbxFile &other) {

	header = other.header;
	root = new FbxNode(*other.root);

}

FbxFile::FbxFile(const FbxFile &other) { copy(other); }
FbxFile &FbxFile::operator=(const FbxFile &other) { copy(other); return *this; }

FbxNodes FbxFile::findMeshes() { return get()->findNodes("Objects/Model", { 2 }, String("Mesh")); }
FbxNodes FbxFile::findLights() { return get()->findNodes("Objects/Model", { 2 }, String("Light")); }
FbxNodes FbxFile::findCameras() { return get()->findNodes("Objects/Model", { 2 }, String("Camera")); }
FbxNodes FbxFile::findGeometry() { return get()->findNodes("Objects/Geometry"); }
FbxFile::FbxFile(FbxHeader header, FbxNodes nodes) : header(header), root(new FbxNode(nodes)) {}

u32 FbxNode::getChildren() { return (u32) childs.size(); }
FbxNode *FbxNode::getChild(u32 i) { return i >= getChildren() ? nullptr : childs[i]; }
u32 FbxNode::getProperties(){ return (u32) properties.size(); }
FbxProperty *FbxNode::getProperty(u32 i) { return i >= getProperties() ? nullptr : properties[i]; }
String FbxNode::getName() { return name; }

FbxProperties::iterator FbxNode::getPropertyBegin() { return properties.begin(); }
FbxProperties::iterator FbxNode::getPropertyEnd() { return properties.end(); }
const FbxProperties &FbxNode::getPropertyArray() { return properties; }
FbxNodes::iterator FbxNode::getChildBegin() { return childs.begin(); }
FbxNodes::iterator FbxNode::getChildEnd() { return childs.end(); }
const FbxNodes &FbxNode::getChildArray() { return childs; }

u32 FbxFile::getVersion() { return header.getVersion(); }
bool FbxFile::isValid() { return header.getHeader() == "Kaydara FBX Binary  " && header.getUnknown() == 0x1A; }

FbxNode *FbxFile::get() { return root; }

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

std::unordered_map<String, Buffer> Fbx::convertMeshes(Buffer buf) {

	FbxFile *file = FbxFile::read(buf);

	std::unordered_map<String, Buffer> meshes;

	FbxNodes geometry = file->findGeometry();

	String lastError;

	for (FbxNode *node : geometry) {

		FbxProperty *namep = node->getProperty(1);

		if (namep == nullptr || namep->getCode() != 'S') {
			lastError = "Couldn't convert geometry object; there was no string object for name";
			goto failed;
		}

		String name = namep->cast<FbxString>()->get();

		FbxNodes vertices = node->findNodes("Vertices");
		FbxNodes indices = node->findNodes("PolygonVertexIndex");
		FbxNodes normals = node->findNodes("LayerElementNormal");
		FbxNodes uvs = node->findNodes("LayerElementUV");
		//FbxNodes materials = node->findNodes("LayerElementMaterial");

		if (vertices.size() == 0) {
			lastError = String("Couldn't find the vertices of a geometry object (") + name + ")";
			goto failed;
		}

		if (meshes.find(name) != meshes.end()) {
			lastError = String("The geometry object \"") + name + "\" is duplicated. This is not supported.";
			goto failed;
		}

		std::vector<Vec3> position;

		//TODO: Construct position

		std::vector<Vec3> normal;

		if (normals.size() > 0) {

			//TODO: Construct normals

		}

		std::vector<Vec2> uv;

		if (uvs.size() > 0) {

			if (uvs.size() != 1) {
				lastError = String("The geometry object \"") + name + "\" has more than 1 uv set. This is not supported.";
				goto failed;
			}

			FbxNodes uvDatn = uvs[0]->findNodes("UV");
			FbxNodes uvIndn = uvs[0]->findNodes("UVIndex");

			if (uvDatn.size() != 1 || uvIndn.size() != 1 || uvDatn[0]->getProperties() == 0 || uvIndn[0]->getProperties() == 0) {
				lastError = String("The geometry object \"") + name + "\" had an invalid UV set.";
				goto failed;
			}

			Vec2d *uvDat = (Vec2d*) uvDatn[0]->getProperty(0)->cast<FbxDoubleArray>()->getPtr();

			FbxIntArray *uvInd = uvIndn[0]->getProperty(0)->cast<FbxIntArray>();

			uv.resize(uvInd->size());

			for (u32 i = 0; i < uvInd->size(); ++i)
				uv[i] = Vec2(uvDat[uvInd->get(i)]);

		}

		meshes[name] = /* TODO: This is the fun part!*/{};

	}

	goto success;

	failed:

	for (auto &elem : meshes)
		elem.second.deconstruct();

	Log::error(lastError);
	meshes.clear();

	success:

	delete file;

	return meshes;

}


bool Fbx::convertMeshes(Buffer fbxBuffer, String outPath) {

	std::unordered_map<String, Buffer> buf = convertMeshes(fbxBuffer);

	String base = outPath.getFilePath();
	String fileName = outPath.getFileName();
	String extension = outPath.getExtension();

	if (buf.size() == 0)
		return Log::error("Fbx conversion failed (or it didn't contain any meshes)");

	String match;

	for (auto &elem : buf)
		if (elem.first.untilFirst(String("\0\x1", 2)).equalsIgnoreCase(fileName)) {

			if (!FileManager::get()->write(outPath, elem.second)) {

				for (auto &elem : buf)
					elem.second.deconstruct();

				return Log::error("Couldn't write oiRM file to disk");

			}

			match = elem.first;
			break;
		}

	if (match == "")
		Log::warn("Converting an fbx with multiple meshes to oiRM is not recommended, unless a mesh matches the fbx's name. Otherwise, scene conversion is recommended.");

	for (auto &elem : buf) {

		String dest = base + "." + elem.first.untilFirst(String("\0\x1", 2)) + "." + extension;

		if (elem.first != match && !FileManager::get()->write(dest, elem.second)) {

			for (auto &elem : buf)
				elem.second.deconstruct();

			return Log::error("Couldn't write oiRM file to disk");

		}

	}

	for (auto &elem : buf)
		elem.second.deconstruct();

	return true;
}

std::unordered_map<String, Buffer> Fbx::convertMeshes(String fbxPath) {

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