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

		std::vector<Vec3> positions, normals;
		std::vector<Vec2> uvs;

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

				std::vector<String> parts = line.split(' ');
				stride = (hasPos ? 3 : 0) + (hasUv ? 2 : 0) + (hasNrm ? 3 : 0);

				if ((u32)perVert.size() == 0)
					perVert.resize(stride);

				std::vector<u32> polind(parts.size() - 1);
				u32 poli = 0;

				for (u32 k = 1; k < (u32) parts.size(); ++k) {

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

					if (index == next) {
						vertices.insert(vertices.end(), perVert.begin(), perVert.end());
						++vertexCount;
					}

					polind[poli] = index;
					++poli;
				}

				for (u32 x = 1; x < poli - 1; ++x) {
					u32 ind[] = { polind[0], polind[x], polind[x + 1] };
					indices.insert(indices.end(), ind, ind + 3);
				}

			}
		}

	}

	return oiRM::generate(Buffer::construct((u8*) vertices.data(), vertexCount * stride * 4), Buffer::construct((u8*) indices.data(), (u32) indices.size() * 4), hasPos, hasUv, hasNrm, vertexCount, (u32) indices.size());
}

bool Obj::convert(Buffer objBuffer, String outPath) {

	Buffer buf = convert(objBuffer);

	if(buf.size() == 0)
		return Log::error("Obj conversion failed");

	if (!FileManager::get()->write(outPath, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write oiRM file to disk");
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