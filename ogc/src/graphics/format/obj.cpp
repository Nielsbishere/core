#include "graphics/format/obj.h"
#include <file/filemanager.h>
#include "graphics/graphics.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

Buffer Obj::convert(Buffer objBuffer) {

	//Convert obj to oiRM
	String str((char*) objBuffer.addr(), objBuffer.size());

	//Find all locations of objects
	std::vector<Vec2u> objects = str.find("\no ", "\no ", 1);

	if(objects.size() == 0){
		Log::error("Couldn't convert Obj; missing an object");
		return {};
	}

	bool hasPos = false, hasUv = false, hasNrm = false;
	u32 vertexCount = 0;

	std::vector<f32> vertices;
	std::vector<u32> indices;
	std::vector<f32> perVert;

	u32 stride = 0;

	for (u32 i = 0; i < (u32)objects.size(); ++i) {

		Vec2u object = objects[i];
		String s = str.substring(object.x, object.y);
		std::vector<Vec2u> lines = s.find("\n", "\n", 1);

		std::vector<Vec3f> positions, normals;
		std::vector<Vec2f> uvs;

		//Approx 1/4 goes to every attribute, so reserve that to minimize resizing as much as possible
		positions.reserve(lines.size() / 4);
		normals.reserve(lines.size() / 4);
		uvs.reserve(lines.size() / 4);
		vertices.reserve(vertices.size() + lines.size() / 4 * 5);
		indices.reserve(indices.size() + lines.size() / 4 * 5);

		for (u32 j = 0; j < lines.size(); ++j) {

			String line = s.substring(lines[j].x, lines[j].y);

			if (!line.contains(' '))
				continue;

			u32 modifier = line.find(' ')[0];
			String identifier = line.cutEnd(modifier);

			if (identifier == "v") {
				positions.push_back(line.cutBegin(modifier + 1).toVector<f32, 3>());
				hasPos = true;
			} else if (identifier == "vt"){
				uvs.push_back(line.cutBegin(modifier + 1).toVector<f32, 2>()); 
				hasUv = true;
			} else if (identifier == "vn"){
				normals.push_back(line.cutBegin(modifier + 1).toVector<f32, 3>());
				hasNrm = true;
			} else if (identifier == "f") {

				if (line.count(' ') != 3) {												//TODO: Multi face
					Log::error("Couldn't convert Obj; please triangulate the mesh");
					return {};
				}

				std::vector<String> parts = line.split(' ');
				stride = (hasPos ? 3 : 0) + (hasUv ? 2 : 0) + (hasNrm ? 3 : 0);

				if ((u32)perVert.size() == 0)
					perVert.resize(stride);

				for (u32 k = 1; k < (u32)4; ++k) {

					std::vector<String> part = parts[k].split('/');

					Vec3u vert;
					u32 m = 0;

					for (u32 l = 0; l < 3 && l < (u32)part.size(); ++l) {

						String p = part[l];
						if (p != "")
							vert[l] = (u32) p.toLong() - 1;

					}

					if (hasPos) {
						perVert[m] = positions[vert.x].x;
						perVert[m + 1] = positions[vert.x].y;
						perVert[m + 2] = positions[vert.x].z;
						m += 3;
					}

					if (hasUv) {
						perVert[m] = uvs[vert.y].x;
						perVert[m + 1] = uvs[vert.y].y;
						m += 2;
					}

					if (hasNrm) {
						perVert[m] = normals[vert.z].x;
						perVert[m + 1] = normals[vert.z].y;
						perVert[m + 2] = normals[vert.z].z;
					}

					u32 next = (u32) vertices.size() / stride;
					u32 index = next;

					for (u32 n = 0; n < (u32)vertices.size() / stride; ++n)
						if (memcmp(vertices.data() + n * stride, perVert.data(), stride * 4) == 0) {
							index = n;
							break;
						}

					if (index == next)
						vertices.insert(vertices.end(), perVert.begin(), perVert.end());

					indices.push_back(index);

				}

			}
		}

	}

	u32 perIndex = vertices.size() <= 256 ? 1 : (vertices.size() <= 65536 ? 2 : 4);

	std::vector<u8> ibo(indices.size() * perIndex);

	if (perIndex == 4)
		memcpy(ibo.data(), indices.data(), ibo.size());
	else
		for (u32 i = 0; i < (u32)indices.size(); ++i) {

			if (perIndex == 1)
				ibo[i] = (u8) indices[i];
			else if(perIndex == 2)
				*(u16*) (ibo.data() + i * 2) = (u16) indices[i];
		}

	u32 attributeCount = (u32)hasPos + hasUv + hasNrm;
	std::vector<String> names;
	std::vector<RMAttribute> attributes;

	names.reserve(attributeCount);
	attributes.reserve(attributeCount);

	if (hasPos) {
		names.push_back("inPosition");
		attributes.push_back({ (u8) 0, (u8) TextureFormat::RGB32f, (u16) 0 });
	}

	if (hasUv) {
		names.push_back("inUv");
		attributes.push_back({ (u8) 0, (u8) TextureFormat::RG32f, (u16) attributes.size() });
	}

	if (hasNrm) {
		names.push_back("inNormal");
		attributes.push_back({ (u8) 0, (u8) TextureFormat::RGB32f, (u16) attributes.size() });
	}

	RMFile file = {

		//Header
		{
			{ 'o', 'i', 'R', 'M' },
			(u8) RMHeaderVersion::V0_0_1,
			(u8) RMHeaderFlag1::None,
			(u8) 1,
			(u8) attributeCount,

			(u8) TopologyMode::Triangle,
			(u8) FillMode::Fill,
			(u8) 0,
			(u8) 0,

			{ 0, 0, 0, 0 },

			(u32) (vertices.size() / stride),
			(u32) indices.size()

		},

		//VBO
		{{
			(u16) ((hasPos ? sizeof(Vec3f) : 0) + (hasUv ? sizeof(Vec2f) : 0) + (hasNrm ? sizeof(Vec3f) : 0)),
			(u16) attributeCount
		}},

		attributes,
		{},
		{ Buffer::construct((u8*)vertices.data(), (u32)(vertices.size() * sizeof(f32))).toArray() },
		ibo,
		{},
		SLFile(String::getDefaultCharset(), names),

	};

	return oiRM::write(file);

}

bool Obj::convert(Buffer objBuffer, String outPath) {

	Buffer buf = convert(objBuffer);

	if(buf.size() == 0)
		return Log::error("Obj conversion failed");

	if (!FileManager::get()->write(outPath, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write obj file to disk");
	}

	buf.deconstruct();
	return true;
}

Buffer Obj::convert(String objPath) {

	Buffer buf;
	FileManager::get()->read(objPath, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't read from file");

	return convert(buf);

}

bool Obj::convert(String objPath, String outPath) {
	
	Buffer buf;
	FileManager::get()->read(objPath, buf);

	if (buf.size() == 0) 
		return Log::error("Couldn't read from file");

	bool converted = convert(buf, outPath);
	buf.deconstruct();
	return converted;
}