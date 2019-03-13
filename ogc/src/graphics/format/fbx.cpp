#include "utils/log.h"
#include "utils/timer.h"
#include "types/vector.h"
#include "file/filemanager.h"
#include "graphics/format/fbx.h"
#include "graphics/format/oirm.h"

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

	node->name = String(node->header.nameLen, (char*)buf.addr());
	buf = buf.offset(node->header.nameLen);
	offset += node->header.nameLen;

	u32 i = 0;

	while (i < node->header.numProperties) {

		FbxProperty *prop;

		if((prop = FbxProperty::readProperty(buf, offset)) != nullptr)
			node->properties.push_back(prop);
		else 
			return (FbxNode*) Log::error(String("Couldn't read FbxNode's properties (") + node->name + ")");

		++i;
	}

	while (offset < node->header.endOffset) {

	FbxNode *subnode;

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

	Array<size_t> pathChilds = term.find(":");
	Array<String> pathChildren;

	size_t s = String::nowhere, i = 0;

	while (i < pathChilds.size()) {
		s = pathChilds[i];

		if (s == 0 || term[s - 1] != '\\') {

			if (i != pathChilds.size())
				pathChildren = term.cutBegin(s + 1).split(",");

			term = term.cutEnd(s);
			break;
		}

		++i;
	}

	for(FbxNode *node : loc)
		if (node->getName() == term || term == "*") {

			bool valid = true;

			for (String &str : pathChildren) {

				bool contains = false;

				for (FbxNode *c : node->childs)
					if (c->getName().contains(str)) {
						contains = true;
						break;
					}

				if (!contains) {
					valid = false;
					break;
				}
			}

			if (!valid)
				continue;

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
FbxNodes FbxFile::findGeometry() { return header.getVersion() <= 6200 ? get()->findNodes("Objects/Model:Vertices") : get()->findNodes("Objects/Geometry"); }
FbxNodes FbxFile::findMaterials() { return get()->findNodes("Objects/Material"); }
FbxFile::FbxFile(FbxHeader header, FbxNodes nodes) : header(header), root(new FbxNode(nodes)) {}

u32 FbxNode::getChildren() { return (u32) childs.size(); }
FbxNode *FbxNode::getChild(u32 i) { return i >= getChildren() ? nullptr : childs[i]; }
u32 FbxNode::getProperties(){ return (u32) properties.size(); }
FbxProperty *FbxNode::getProperty(u32 i) { return i >= getProperties() ? nullptr : properties[i]; }
const String &FbxNode::getName() const { return name; }

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

Vec3 Fbx::getMaterialCol(FbxNode *nod, Vec3 def) {

	if (nod == nullptr || nod->getProperties() < 7)
		return def;

	return Vec3((f32)nod->getProperty(4)->cast<FbxDouble>()->get(), (f32)nod->getProperty(5)->cast<FbxDouble>()->get(), (f32)nod->getProperty(6)->cast<FbxDouble>()->get());

}

f32 Fbx::getMaterialNum(FbxNode *nod, f32 def) {

	if (nod == nullptr || nod->getProperties() < 5)
		return def;

	return (f32)nod->getProperty(4)->cast<FbxDouble>()->get();

}

std::unordered_map<String, Buffer> Fbx::convertMeshes(Buffer buf, bool compression) {

	Timer t;

	FbxFile *file = FbxFile::read(buf);

	const char *zeroNe = "\0\x1";
	String zerone = String(2, (char*)zeroNe);

	FbxNodes materials = file->findMaterials();

	for (FbxNode *node : materials) {

		String name = node->getProperty(1)->cast<FbxString>()->get().untilFirst(zerone);

		std::unordered_map<String, FbxNode*> types;

		for (FbxNode *mat : node->findNodes("Properties70/P")) {
			String prop = mat->getProperty(0)->cast<FbxString>()->get().untilFirst(zerone);
			types[prop] = mat;
		}

		Vec3 emissive = getMaterialCol(types["EmissiveColor"]) * getMaterialNum(types["EmissiveFactor"], 1);
		Vec3 ambient = getMaterialCol(types["AmbientColor"]) *  getMaterialNum(types["AmbientFactor"], 1);
		Vec3 diffuse = getMaterialCol(types["DiffuseColor"], Vec3(1));
		Vec3 transparency = getMaterialCol(types["TransparentColor"], Vec3(1)) * getMaterialNum(types["TransparencyFactor"], 1) * getMaterialNum(types["Opacity"], 1);
		Vec3 specular = getMaterialCol(types["SpecularColor"]) * getMaterialNum(types["SpecularFactor"], 1);
		f32 shininess = getMaterialNum(types["Shininess"]);
		f32 shininessExponent = getMaterialNum(types["ShininessExponent"]);
		Vec3 reflection = getMaterialCol(types["ReflectionColor"]) * getMaterialNum(types["ReflectionFactor"], 1);

		Log::println(String("Material: ") + emissive + " " + diffuse + " " + transparency + " " + specular + " " + shininess + " " + shininessExponent + " " + reflection);

		//TODO: Save materials

	}

	std::unordered_map<String, Buffer> meshes;

	FbxNodes geometry = file->findGeometry();

	String lastError;

	u32 id = 0;

	for (FbxNode *node : geometry) {

		FbxProperty *namep = node->getProperty(1);

		if (namep == nullptr || namep->getCode() != 'S') {
			lastError = "Couldn't convert geometry object; there was no string object for name";
			goto failed;
		}
		
		String name = namep->cast<FbxString>()->get().untilFirst(zerone);

		if (name == "") name = id;
		if (geometry.size() == 1) name = "";

		FbxNodes vertices = node->findNodes("Vertices");						//Vec3d[]
		FbxNodes vertexOrder = node->findNodes("PolygonVertexIndex");			//i32[]
		FbxNodes normals = node->findNodes("LayerElementNormal");				//Vec3d[]
		FbxNodes uvs = node->findNodes("LayerElementUV");						//Vec2d[] and i32[]
		//FbxNodes materials = node->findNodes("LayerElementMaterial");

		if (vertices.size() == 0 || vertexOrder.size() == 0 || vertices[0]->getProperties() == 0 || vertexOrder[0]->getProperties() == 0) {
			lastError = String("Couldn't find the vertices of a geometry object (") + name + ")";
			goto failed;
		}

		if (meshes.find(name) != meshes.end()) {
			lastError = String("The geometry object \"") + name + "\" is duplicated. This is not supported";
			goto failed;
		}

		//TODO: Normals, uv, etc. can also be per-polygon instead of per vertex

		const bool useArray = file->getVersion() > 6200;
		u32 indCount, vertCount;

		//Vertex positions are only stored once, but referenced to by vertexOrder
		Vec3d *vpos = nullptr;
		i32 *ind = nullptr;

		if (useArray) {

			FbxDoubleArray *pos = vertices[0]->getProperty(0)->cast<FbxDoubleArray>();
			FbxIntArray *posOrder = vertexOrder[0]->getProperty(0)->cast<FbxIntArray>();

			if (pos == nullptr || posOrder == nullptr) {
				lastError = String("The geometry object \"") + name + "\" has invalid positional data";
				goto failed;
			}

			vpos = (Vec3d*)&pos->get();
			vertCount = pos->size();
			ind = (i32*)&posOrder->get();
			indCount = posOrder->size();
		}
		else {
			vertCount = vertices[0]->getProperties();
			indCount = vertexOrder[0]->getProperties();
		}

		//required Vec3 pos; optional Vec2 uv; optional Vec3 normal;
		u32 posuv = 3 + (uvs.size() != 0 ? 2 : 0);
		u32 stride = posuv + (normals.size() != 0 ? 3 : 0);
		Array<f32> buffer(indCount * stride);

		std::vector<Vec2u> faces;
		u32 prev = 0, indices = 0;

		for (u32 i = 0; i < indCount; ++i) {

			i32 j = useArray ? ind[i] : vertexOrder[0]->getProperty(i)->get<FbxInt>();

			i32 k = j < 0 ? (-j - 1) : j;

			if ((u32)k >= vertCount / 3) {
				lastError = String("The geometry object \"") + name + "\" has invalid positional data";
				goto failed;
			}

			Vec3 v = useArray ? Vec3(vpos[k]) : 
				Vec3(
					vertices[0]->get<FbxDouble>(k * 3),
					vertices[0]->get<FbxDouble>(k * 3 + 1),
					vertices[0]->get<FbxDouble>(k * 3 + 2)
				);

			*(Vec3*)(buffer.begin() + i * stride) = ((v * 1000).round() / 1000).fix();

			if (j < 0) {
				indices += (i - prev - 1) * 3;
				faces.push_back(Vec2u(prev, i));
				prev = i + 1;
			}

		}

		//Normals are stored in a Vec3d[]
		if (normals.size() > 0) {

			if (normals.size() != 1) {
				lastError = String("The geometry object \"") + name + "\" has more than 1 normal set. This is not supported";
				goto failed;
			}

			FbxNodes normalDat = normals[0]->findNodes("Normals");

			if (normalDat.size() != 1 || normalDat[0]->getProperties() == 0) {
				lastError = String("The geometry object \"") + name + "\" doesn't have a valid normal set";
				goto failed;
			}

			if (useArray) {

				FbxDoubleArray *normalp = normalDat[0]->getProperty(0)->cast<FbxDoubleArray>();

				if (normalp == nullptr) {
					lastError = String("The geometry object \"") + name + "\" doesn't have a valid normal set";
					goto failed;
				}

				Vec3d *uvDat = (Vec3d*)normalp->getPtr();

				for (u32 i = 0; i < normalp->size() / 3; ++i)
					*(Vec3*)(buffer.begin() + i * stride + posuv) = ((Vec3(uvDat[i]) * 1000).round() / 1000).fix();

			} else {

				FbxNode *dat = normalDat[0];

				for (u32 i = 0; i < dat->getProperties() / 3; ++i)
					*(Vec3*)(buffer.begin() + i * stride + posuv) = ((
						Vec3(
							dat->get<FbxDouble>(i * 3),
							dat->get<FbxDouble>(i * 3 + 1),
							dat->get<FbxDouble>(i * 3 + 2)
						) * 1000).round() / 1000).fix();
			}

		}

		//Uvs are stored in a Vec2d[] and duplicated uvs are not allowed
		//so an index to a UV is used to avoid duplicating uvs
		if (uvs.size() > 0) {

			if (uvs.size() != 1) {
				lastError = String("The geometry object \"") + name + "\" has more than 1 uv set. This is not supported";
				goto failed;
			}

			FbxNodes uvDatn = uvs[0]->findNodes("UV");
			FbxNodes uvIndn = uvs[0]->findNodes("UVIndex");

			if (uvDatn.size() != 1 || uvIndn.size() != 1 || uvDatn[0]->getProperties() == 0 || uvIndn[0]->getProperties() == 0) {
				lastError = String("The geometry object \"") + name + "\" had an invalid UV set";
				goto failed;
			}

			if (useArray) {

				FbxDoubleArray *uvData = uvDatn[0]->getProperty(0)->cast<FbxDoubleArray>();

				if (uvData == nullptr) {
					lastError = String("The geometry object \"") + name + "\" had an invalid UV set";
					goto failed;
				}

				Vec2d *uvDat = (Vec2d*)uvData->getPtr();

				FbxIntArray *uvInd = uvIndn[0]->getProperty(0)->cast<FbxIntArray>();

				if (uvInd == nullptr) {
					lastError = String("The geometry object \"") + name + "\" had an invalid UVIndex array.";
					goto failed;
				}

				for (u32 i = 0; i < uvInd->size(); ++i)
					*(Vec2*)(buffer.begin() + i * stride + 3) = ((Vec2(uvDat[uvInd->get(i)]) * 1000).round() / 1000).fix();

			}
			else {

				FbxNode *uvDat = uvDatn[0];
				FbxNode *uvInd = uvIndn[0];

				for (u32 i = 0; i < uvInd->getProperties(); ++i)
					*(Vec2*)(buffer.begin() + i * stride + 3) = ((
						Vec2(
							uvDat->get<FbxDouble>(uvInd->get<FbxInt>(i) * 2),
							uvDat->get<FbxDouble>(uvInd->get<FbxInt>(i) * 2 + 1)
						) * 1000).round() / 1000).fix();


			}

		}

		u32 indx = 0;
		Array<u32> index(indices);

		for (Vec2u face : faces) {
			for (u32 x = 1; x < face.y - face.x; ++x) {
				*(Vec3u*)(index.begin() + indx) = { face.x + x + 1, face.x + x, face.x };
				indx += 3;
			}
		}

		/*t.print();*/

		RMFile rfile = oiRM::generate(Buffer::construct((u8*) buffer.begin(), (u32) buffer.dataSize()), Buffer::construct((u8*)index.begin(), (u32) index.dataSize()), true, uvs.size() != 0, normals.size() != 0, (u32) buffer.size() / 8, indices);
		Buffer obuf = oiRM::write(rfile, compression);

		if (obuf.size() == 0) {
			lastError = String("The geometry object \"") + name + "\" couldn't be converted to oiRM.";
			goto failed;
		}

		meshes[name.untilFirst(zerone)] = obuf;

		++id;

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


bool Fbx::convertMeshes(Buffer fbxBuffer, String outPath, bool compression) {

	std::unordered_map<String, Buffer> buf = convertMeshes(fbxBuffer, compression);

	String base = outPath.getFilePath();
	String fileName = outPath.getFileName();

	if (buf.size() == 0)
		return Log::error("Fbx conversion failed (or it didn't contain any meshes)");

	for (auto &elem : buf)
		if (elem.first.equalsIgnoreCase(fileName) || elem.first == "") {
			if (!FileManager::get()->write(outPath, elem.second)) {

				for (auto &elem0 : buf)
					elem0.second.deconstruct();

				return Log::error("Couldn't write oiRM file to disk");

			}
		} else if(!FileManager::get()->write(base + "." + elem.first + ".oiRM", elem.second)) {

			for (auto &elem0 : buf)
				elem0.second.deconstruct();

			return Log::error("Couldn't write oiRM file to disk");

		}

	for (auto &elem : buf)
		elem.second.deconstruct();

	return true;
}

std::unordered_map<String, Buffer> Fbx::convertMeshes(String fbxPath, bool compression) {

	Buffer buf;
	FileManager::get()->read(fbxPath, buf);

	if (buf.size() == 0) {
		Log::error("Couldn't read from file");
		return {};
	}

	return convertMeshes(buf, compression);

}

bool Fbx::convertMeshes(String fbxPath, String outPath, bool compression) {

	Buffer buf;
	FileManager::get()->read(fbxPath, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't read from file");

	bool converted = convertMeshes(buf, outPath, compression);
	buf.deconstruct();
	return converted;
}