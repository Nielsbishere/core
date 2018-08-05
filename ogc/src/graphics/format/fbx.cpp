#include "graphics/format/fbx.h"

#include <utils/log.h>
#include <file/filemanager.h>

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

TFbxPropertyBase *FbxProperty::read(Buffer &buffer, u32 &offset) {

	if (buffer.size() == 0)
		return (TFbxPropertyBase*) Log::error("Couldn't read FbxProperty; buffer was null");

	char type = buffer[0];

	TFbxPropertyBase *allocated = FbxTypeAlloc::allocate(type);

	if(allocated == nullptr)
		return (TFbxPropertyBase*) Log::error("Couldn't allocate FbxProperty; invalid type");

	if (!allocated->read(buffer, offset)) {
		delete allocated;
		return (TFbxPropertyBase*)Log::error("Couldn't read FbxProperty");
	}

	return allocated;
}

FbxNode FbxNode::read(Buffer &buf, u32 &offset, bool is64bit) {

	u32 headerSize = is64bit ? FbxNodeHeader64::size() : FbxNodeHeader32::size();

	if (buf.size() < headerSize) {
		Log::error("Couldn't read FbxNode; invalid buffer size");
		return {};
	}

	FbxNode node;

	if (is64bit) {

		memset(&node, 0, headerSize);
		memcpy(&node, buf.addr(), headerSize);

		buf = buf.offset(headerSize);
		offset += headerSize;

	} else {

		FbxNodeHeader32 header;

		memset(&header, 0, headerSize);
		memcpy(&header, buf.addr(), headerSize);

		buf = buf.offset(headerSize);
		offset += headerSize;

		node.header = header;

	}

	if (buf.size() < node.header.nameLen) {
		Log::error("Couldn't read FbxNode's name; invalid buffer size");
		return {};
	}

	node.name = String((char*) buf.addr(), node.header.nameLen);
	buf = buf.offset(node.header.nameLen);
	offset += node.header.nameLen;

	u32 i = 0;
	TFbxPropertyBase *prop;

	while (i < node.header.numProperties) {

		if((prop = FbxProperty::read(buf, offset)) != nullptr)
			node.properties.push_back(prop);
		else {
			Log::error(String("Couldn't read FbxNode's properties (") + node.name + ")");
			return {};
		}

		++i;
	}

	FbxNode subnode;

	while (offset < node.header.endOffset) {

		if ((subnode = FbxNode::read(buf, offset, is64bit)).header.endOffset == (u64) -1) {
			Log::error(String("Couldn't read FbxNode's child nodes (") + node.name + ")");
			return {};
		}

		if (subnode.header.nameLen != 0)
			node.childs.push_back(subnode);

	}

	return node;

}

std::vector<FbxNode> FbxNode::readAll(Buffer &buf, u32 &offset, bool is64bit) {

	FbxNode node;
	std::vector<FbxNode> nodes;

	while (buf.size() > 0) {

		if ((node = read(buf, offset, is64bit)).header.endOffset == -1 && node.header.nameLen != 0) {
			Log::error("Couldn't read an fbx node");
			return {};
		}

		if(node.header.nameLen != 0)
			nodes.push_back(node);

	}

	return nodes;

}


u32 FbxNode::getChildren() { return (u32) childs.size(); }
FbxNode &FbxNode::getChild(u32 i) { return childs[i]; }

std::vector<FbxNode> Fbx::read(Buffer buf) {

	goto binary;

binary:

	if (buf.size() < FbxHeader::size()) {
		Log::error("Couldn't read Fbx; invalid buffer size");
		return {};
	}

	FbxHeader head;

	memset(&head, 0, sizeof(head));
	memcpy(&head, buf.addr(), FbxHeader::size());

	u32 offset = (u32)FbxHeader::size();
	buf = buf.offset(offset);

	if (head.getHeader() != "Kaydara FBX Binary  " || head.getUnknown() != 0x001A) {
		Log::error("Couldn't read Fbx; invalid header");
		return {};
	}

	std::vector<FbxNode> nodes = FbxNode::readAll(buf, offset, head.getVersion() >= 7500 /* Starting from version 7.5, they use 64-bit format for nodes*/);

	if (nodes.size() != 0)
		Log::println(String("Successfully read fbx file with version ") + head.getVersion());

	return nodes;
}

std::vector<FbxNode> Fbx::read(String fbxPath) {

	Buffer buf;
	FileManager::get()->read(fbxPath, buf);

	if (buf.size() == 0) {
		Log::error("Couldn't read from file");
		return {};
	}

	std::vector<FbxNode> data = read(buf);
	buf.deconstruct();
	return data;

}

Fbx::FbxMeshes Fbx::convertMeshes(Buffer buf) {

	//TODO:

	return {};

}


bool Fbx::convertMeshes(Buffer fbxBuffer, String outPath) {

	Fbx::FbxMeshes buf = convertMeshes(fbxBuffer);

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

Fbx::FbxMeshes Fbx::convertMeshes(String fbxPath) {

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