#include "utils/timer.h"
#include "file/filemanager.h"
#include "graphics/graphics.h"
#include "graphics/format/obj.h"

using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

Buffer Obj::convert(Buffer objBuffer, bool compression) {

	Timer t;

	//Convert obj to oiRM
	String str(objBuffer.size(), (char*)objBuffer.addr());

	//Find all locations of objects
	Array<TVec<size_t, 2>> objects = str.find("\no ", "\no ");

	if(objects.size() == 0){
		Log::error("Couldn't convert Obj; missing an object");
		return {};
	}

	bool hasPos = false, hasUv = false, hasNrm = false;
	u32 vertexCount = 0;

	std::vector<f32> vertices;
	std::vector<u32> indices;
	std::vector<f32> perVert;

	f32 *aperVert = nullptr, *avertices = nullptr;

	u32 avertexSiz = 0;

	std::vector<u32> polind;
	polind.reserve(32);

	u32 stride = 0;

	for (u32 i = 0; i < (u32)objects.size(); ++i) {

		TVec<size_t, 2> object = objects[i];
		String s = str.substring(object.x, object.y);

		Array<TVec<size_t, 2>> lines = s.find("\n", "\n");

		std::vector<Vec3> positions, normals;
		std::vector<Vec2> uvs;

		u32 lineCount = (u32) lines.size();

		//Approx 1/4 goes to every attribute, so reserve that to minimize resizing as much as possible
		positions.reserve(lineCount / 4);
		normals.reserve(lineCount / 4);
		uvs.reserve(lineCount / 4);
		vertices.reserve(vertices.size() + lineCount / 4 * 5);
		indices.reserve(indices.size() + lineCount / 4 * 5);

		for (u32 j = 0; j < lineCount; ++j) {

			String line = s.substring(lines[j].x, lines[j].y);

			if (!line.contains(' '))
				continue;

			size_t modifier = line.find(' ')[0];
			String identifier = line.cutEnd(modifier);

			if (identifier == "v") {
				positions.push_back(line.cutBegin(modifier + 1));
				hasPos = true;
			} else if (identifier == "vt"){
				uvs.push_back(line.cutBegin(modifier + 1)); 
				hasUv = true;
			} else if (identifier == "vn"){
				normals.push_back(line.cutBegin(modifier + 1));
				hasNrm = true;
			} else if (identifier == "f") {

				Array<String> parts = line.split(' ');
				stride = (hasPos ? 3 : 0) + (hasUv ? 2 : 0) + (hasNrm ? 3 : 0);

				if ((u32)perVert.size() == 0) {
					perVert.resize(stride);
					aperVert = perVert.data();
				}

				polind.resize(parts.size() - 1);
				u32 poli = 0;

				for (u32 k = 1; k < (u32) parts.size(); ++k) {

					Array<String> part = parts[k].split('/');

					Vec3u vert;
					u32 m = 0;

					for (u32 l = 0; l < 3 && l < (u32)part.size(); ++l) {

						String p = part[l];
						if (p != "")
							vert[l] = (u32) p - 1;

					}

					if (hasPos) {
						*(Vec3*)(aperVert + m) = positions[vert.x];
						m += 3;
					}

					if (hasUv) {
						*(Vec2*)(aperVert + m) = uvs[vert.y];
						m += 2;
					}

					if (hasNrm)
						*(Vec3*)(aperVert + m) = normals[vert.z];

					u32 next = (u32) avertexSiz / stride;
					u32 index = next;

					//TODO: memcmp

					for (u32 n = 0; n < (u32) avertexSiz / stride; ++n)
						if (memcmp(avertices + n * stride, aperVert, stride * 4) == 0) {
							index = n;
							break;
						}

					if (index == next) {
						vertices.insert(vertices.end(), perVert.begin(), perVert.end());
						avertices = vertices.data();
						avertexSiz = (u32) vertices.size();
						++vertexCount;
					}

					polind[poli] = index;
					++poli;
				}

				u32 indlen = (poli - 2) * 3;
				u32 *ind = new u32[indlen];

				for (u32 x = 1; x < poli - 1; ++x)
					*(Vec3u*)(ind + (x - 1) * 3) = Vec3u(polind[0], polind[x], polind[x + 1]);

				indices.insert(indices.end(), ind, ind + indlen);
				delete[] ind;

			}
		}

	}

	t.stop();
	t.print();

	RMFile file = oiRM::generate(Buffer::construct((u8*) avertices, vertexCount * stride * 4), Buffer::construct((u8*) indices.data(), (u32) indices.size() * 4), hasPos, hasUv, hasNrm, vertexCount, (u32) indices.size());
	return oiRM::write(file, compression);
}

bool Obj::convert(Buffer objBuffer, String outPath, bool compression) {

	Buffer buf = convert(objBuffer, compression);

	if(buf.size() == 0)
		return Log::error("Obj conversion failed");

	if (!FileManager::get()->write(outPath, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write oiRM file to disk");
	}

	buf.deconstruct();
	return true;
}

Buffer Obj::convert(String objPath, bool compression) {

	Buffer buf;
	FileManager::get()->read(objPath, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't read from file");

	return convert(buf, compression);

}

bool Obj::convert(String objPath, String outPath, bool compression) {
	
	Buffer buf;
	FileManager::get()->read(objPath, buf);

	if (buf.size() == 0) 
		return Log::error("Couldn't read from file");

	bool converted = convert(buf, outPath, compression);
	buf.deconstruct();
	return converted;
}