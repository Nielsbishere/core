#include <file/filemanager.h>
#include "graphics/format/oirm.h"
#include "graphics/mesh.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

bool oiRM::read(String path, RMFile &file) {

	Buffer buf;
	FileManager::get()->read(path, buf);

	if (buf.size() == 0)
		return Log::error("Couldn't open file");

	if (!read(buf, file))
		return Log::error("Couldn't read file");

	buf.deconstruct();
	return true;
}

bool oiRM::read(Buffer data, RMFile &file) {

	const char magicNumber[] = { 'o', 'i', 'R', 'M' };

	if (data.size() < 4 || memcmp(magicNumber, &(file.header = data.operator[]<RMHeader>(0)), sizeof(magicNumber)) != 0)
		return Log::error("Couldn't read oiRM file; invalid header");

	Buffer read = data.offset((u32) sizeof(RMHeader));

	RMHeaderVersion v(file.header.version);

	switch (v.getValue()) {

	case RMHeaderVersion::V0_0_1.value:
		goto V0_0_1;

	default:
		return Log::error("Invalid oiRM (header) file");

	}

V0_0_1:
	{

		u32 perIndex = file.header.vertices <= 256 ? 1 : (file.header.vertices <= 65536 ? 2 : 4);

		u32 vertexBuffer = file.header.vertexBuffers * (u32) sizeof(RMVBO);
		u32 vertexAttribute = file.header.vertexAttributes * (u32) sizeof(RMAttribute);
		u32 misc = file.header.miscs * (u32) sizeof(RMMisc);
		u32 index = file.header.indices * perIndex;

		u32 destSize = vertexBuffer + vertexAttribute + misc + index;

		if (read.size() < destSize)
			return Log::error("Couldn't read oiRM file; invalid size");

		file.vbos.resize(file.header.vertexBuffers);
		memcpy(file.vbos.data(), read.addr(), vertexBuffer);
		read = read.offset(vertexBuffer);

		file.vbo.resize(file.header.vertexAttributes);
		memcpy(file.vbo.data(), read.addr(), vertexAttribute);
		read = read.offset(vertexAttribute);

		file.miscs.resize(file.header.miscs);
		memcpy(file.miscs.data(), read.addr(), misc);
		read = read.offset(misc);

		file.vertices.resize(file.header.vertexBuffers);
		for (u32 i = 0; i < file.header.vertexBuffers; ++i) {

			u32 length = file.vbos[i].stride * file.header.vertices;

			if (length + index > read.size())
				return Log::error("Couldn't read oiRM file; invalid vertex length");

			file.vertices[i].resize(length);
			memcpy(file.vertices[i].data(), read.addr(), length);
			read = read.offset(length);
		}

		if (file.header.indices != 0) {
			file.indices.resize(index);
			memcpy(file.indices.data(), read.addr(), index);
			read = read.offset(index);
		}

		file.miscBuffer.resize(file.header.miscs);
		for (u32 i = 0; i < file.header.miscs; ++i) {

			u32 length = file.miscs[i].size;

			if (length > read.size())
				return Log::error("Couldn't read oiRM file; invalid misc length");

			file.miscBuffer[i].resize(length);
			memcpy(file.miscBuffer[i].data(), read.addr(), length);
			read = read.offset(length);
		}

		if (!oiSL::read(read, file.names))
			return Log::error("Couldn't read oiRM file; invalid oiSL");

		read = read.offset(file.names.size);
		goto end;
	}

end:

	file.size = (u32)(read.addr() - data.addr());
	if (read.size() == 0) file.size = data.size();

	Log::println(String("Successfully loaded oiRM file with version ") + v.getName() + " (" + file.size + " bytes)");
	return true;
}

std::pair<MeshBufferInfo, MeshInfo> oiRM::convert(Graphics *g, RMFile file) {

	std::pair<MeshBufferInfo, MeshInfo> result;

	if (g == nullptr)
		return result;

	std::vector<std::vector<std::pair<String, TextureFormat>>> vbos(file.vbos.size());
	std::vector<Buffer> vb(vbos.size());
	Buffer ib;

	u32 i = 0, j = 0;

	for (RMVBO vbo : file.vbos) {

		vbos[i].resize(vbo.layouts);
		vb[i] = Buffer(file.vertices[i].data(), (u32)file.vertices[i].size());

		for (u32 k = 0; k < vbo.layouts; ++k)
			vbos[i][k] = { file.names.names[file.vbo[k].name], file.vbo[k].format };

		j += vbo.layouts;
		++i;
	}

	if (file.header.indices != 0) {

		ib = Buffer(4 * file.header.indices);
		u32 perIndex = file.header.vertices <= 256 ? 1 : (file.header.vertices <= 65536 ? 2 : 4);

		if (perIndex == 4) ib.copy(Buffer::construct(file.indices.data(), (u32) file.indices.size()));
		else if (perIndex == 2)
			for (u32 i = 0; i < (u32) file.indices.size() / 2; ++i)
				ib.operator[]<u32>(i * 4) = *(u16*)(file.indices.data() + i * 2);
		else if (perIndex == 1)
			for (u32 i = 0; i < (u32)file.indices.size(); ++i)
				ib.operator[]<u32>(i * 4) = (u32) file.indices[i];

	}

	result.first = MeshBufferInfo(file.header.vertices, file.header.indices, vbos, file.header.topologyMode, file.header.fillMode);
	result.second = MeshInfo(nullptr, file.header.vertices, file.header.indices, vb, ib);
	
	return result;
}

RMFile oiRM::convert(MeshInfo info) {

	MeshBufferInfo meshBuffer = info.buffer->getInfo();

	auto buffers = meshBuffer.buffers;
	u32 attributeCount = 0;

	std::vector<RMAttribute> attributes;
	std::vector<String> names;
	std::vector<RMVBO> vbos(buffers.size());
	std::vector<std::vector<u8>> vertices(buffers.size());

	u32 i = 0, j = 0;

	for (auto &elem : buffers) {

		attributeCount += (u32) elem.size();
		attributes.reserve(attributeCount);
		names.reserve(attributeCount);

		u32 size = 0;

		for (auto &pair : elem) {
			attributes.push_back({ 0, (u8)pair.second.getValue(), (u16)j });
			names.push_back(pair.first);
			size += Graphics::getFormatSize(pair.second);
			++j;
		}

		vbos[i] = { (u16) size, (u16) elem.size() };
		vertices[i] = std::move(info.vbo[i].toArray());

		++i;
	}

	std::vector<RMMisc> miscs;

	return {

		{
			{ 'o', 'i', 'R', 'M' },

			(u8)RMHeaderVersion::V0_0_1,
			(u8)RMHeaderFlag1::None,
			(u8)buffers.size(),
			(u8)attributeCount,

			(u8)meshBuffer.topologyMode.getValue(),
			(u8)meshBuffer.fillMode.getValue(),
			0,													//TODO: Saving miscs
			0,

			{ 0, 0, 0, 0 },

			info.vertices,

			info.indices
		},

		vbos,
		attributes,
		miscs,
		vertices,
		info.ibo.toArray(),
		{},
		SLFile(String::getDefaultCharset(), names)

	};
}

Buffer oiRM::write(RMFile &file) {

	RMHeader &header = file.header;

	u32 perIndex = header.vertices <= 256 ? 1 : (header.vertices <= 65536 ? 2 : 4);

	u32 vertexBuffer = (u32)(header.vertexBuffers * sizeof(RMVBO));
	u32 vertexAttribute = (u32)(header.vertexAttributes * sizeof(RMAttribute));
	u32 misc = (u32)(header.miscs * sizeof(RMMisc));
	u32 index = (u32)(header.indices * perIndex);

	std::vector<u8> &indices = file.indices;

	Buffer b = oiSL::write(file.names);
	Buffer vertices(file.vertices);
	Buffer miscBuf(file.miscBuffer);

	file.size = (u32) sizeof(header) + vertexBuffer + vertexAttribute + misc + vertices.size() + index + miscBuf.size() + file.names.size;

	Buffer output(file.size);
	Buffer write = output;

	memcpy(write.addr(), &header, sizeof(header));
	write = write.offset((u32) sizeof(header));

	memcpy(write.addr(), file.vbos.data(), vertexBuffer);
	write = write.offset(vertexBuffer);

	memcpy(write.addr(), file.vbo.data(), vertexAttribute);
	write = write.offset(vertexAttribute);

	memcpy(write.addr(), file.miscs.data(), misc);
	write = write.offset(misc);

	memcpy(write.addr(), vertices.addr(), vertices.size());
	write = write.offset(vertices.size());
	vertices.deconstruct();

	memcpy(write.addr(), indices.data(), index);
	write = write.offset(index);

	memcpy(write.addr(), miscBuf.addr(), miscBuf.size());
	write = write.offset(miscBuf.size());
	miscBuf.deconstruct();

	write.copy(b, b.size(), 0, 0);
	write = write.offset(b.size());
	b.deconstruct();

	return output;

}

bool oiRM::write(String path, RMFile &file) {

	Buffer buf = write(file);

	if (!FileManager::get()->write(path, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write to file");
	}

	buf.deconstruct();
	return true;

}