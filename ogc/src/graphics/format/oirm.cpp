#include <file/filemanager.h>
#include <types/bitset.h>
#include <utils/timer.h>
#include "graphics/format/oirm.h"
#include "graphics/mesh.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

Buffer oiRM::generate(Buffer vbo, Buffer bibo, bool hasPos, bool hasUv, bool hasNrm, u32 vertices, u32 indices, bool compression) {

	u32 stride = (hasPos ? 12 : 0) + (hasUv ? 8 : 0) + (hasNrm ? 12 : 0);

	if (vbo.size() != stride * vertices) {
		Log::error("Couldn't generate oiRM file; the vbo was of invalid size");
		return {};
	}

	if (bibo.size() != 4 * indices) {
		Log::error("Couldn't generate oiRM file; the vbo was of invalid size");
		return {};
	}

	u32 *ibo = (u32*) bibo.addr();

	u32 perIndex = vertices <= 256 ? 1 : (vertices <= 65536 ? 2 : 4);
	std::vector<u8> fibo(indices * perIndex);

	if (perIndex == 4)
		memcpy(fibo.data(), ibo, bibo.size());
	else
		for (u32 i = 0; i < (u32) indices; ++i) {

			if (perIndex == 1)
				fibo[i] = (u8) ibo[i];
			else if (perIndex == 2)
				*(u16*)(fibo.data() + i * 2) = (u16) ibo[i];
		}

	u32 attributeCount = (u32) hasPos + hasUv + hasNrm;
	std::vector<String> names;
	std::vector<RMAttribute> attributes;

	names.reserve(attributeCount);
	attributes.reserve(attributeCount);

	if (hasPos) {
		names.push_back("inPosition");
		attributes.push_back({ (u8)0, (u8)TextureFormat::RGB32f, (u16)0 });
	}

	if (hasUv) {
		names.push_back("inUv");
		attributes.push_back({ (u8)0, (u8)TextureFormat::RG32f, (u16)attributes.size() });
	}

	if (hasNrm) {
		names.push_back("inNormal");
		attributes.push_back({ (u8)0, (u8)TextureFormat::RGB32f, (u16)attributes.size() });
	}

	RMFile file = {

		//Header
		{
			{ 'o', 'i', 'R', 'M' },
			(u8)RMHeaderVersion::V0_0_1,
			(u8)RMHeaderFlag1::None,
			(u8)1,
			(u8)attributeCount,

			(u8)TopologyMode::Triangle,
			(u8)FillMode::Fill,
			(u8)0,
			(u8)0,

			{ 0, 0, 0, 0 },

			(u32) vertices,
			(u32) indices

		},

		//VBO
		{ {
			(u16)((hasPos ? sizeof(Vec3) : 0) + (hasUv ? sizeof(Vec2) : 0) + (hasNrm ? sizeof(Vec3) : 0)),
			(u16)attributeCount
		} },

		attributes,
		{},
		{ vbo.toArray() },
		fibo,
		{},
		SLFile(String::getDefaultCharset(), names),

	};

	return oiRM::write(file, compression);
}

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

	Timer t;

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

		u32 attr = 0;
		bool compression = isSet((RMHeaderFlag1_s) file.header.flags, RMHeaderFlag1::Uses_compression);

		file.vertices.resize(file.header.vertexBuffers);

		RMVBO *vbo = file.vbos.data();
		std::vector<u8> *vbdat = file.vertices.data();
		std::vector<u32> indices;

		for (u32 i = 0; i < file.header.vertexBuffers; ++i) {

			vbdat->resize(vbo->stride * file.header.vertices);

			u8 *vbdata = vbdat->data();

			memset(vbdata, 0, vbo->stride * file.header.vertices);

			if (!compression) {

				u32 length = vbo->stride * file.header.vertices;

				if (length + index > read.size())
					return Log::error("Couldn't read oiRM file; invalid vertex length");

				memcpy(vbdata, read.addr(), length);
				read = read.offset(length);

			} else {

				RMAttribute *attrib = file.vbo.data();

				for (u32 j = 0, offset = 0; j < vbo->layouts; ++j) {

					TextureFormat format = attrib->format;

					u32 channels = Graphics::getChannels(format);
					u32 bpc = Graphics::getChannelSize(format);
					
					u32 keyset;

					if (!read.read(keyset) || read.size() < keyset * bpc)
						return Log::error("Couldn't read oiRM file; invalid keyset");

					Buffer values = read.subbuffer(0, keyset * bpc);
					read += keyset * bpc;

					u32 perKey = (u32)std::ceil(std::log2((f64)keyset));

					Bitset bitset;

					if (!read.read(bitset, file.header.vertices * channels * perKey))
						return Log::error("Couldn't read oiRM file; invalid bitset");

					indices.resize(file.header.vertices * channels);
					bitset.read(indices, perKey);

					u32 uncompSiz = channels * bpc * file.header.vertices;

					u8 *dest = values.addr();
					u32 *aindices = indices.data();

					for (u32 k = 0; k < channels * file.header.vertices; ++k)
						memcpy(vbdata + k / channels * vbo->stride + offset + k % channels * bpc, dest + aindices[k] * bpc, bpc);

					offset += channels * bpc;
					++attrib;

				}

				attr += vbo->layouts;

			}

			++vbo;
			++vbdat;

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

	t.stop();
	t.print();

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

template<typename T>
void fillKeyset(CopyBuffer &buf, std::vector<u8> &vbo, T *loc, u32 m, std::vector<u32> &indices) {

	T *beg = (T*) buf.addr();
	T *end = (T*)(buf.addr() + buf.size());
	T &val = *loc;

	auto it = std::find(beg, end, val);

	if (it == end) {
		indices[m] = buf.size() / sizeof(T);
		buf += CopyBuffer((u8*)loc, sizeof(T));
	} else
		indices[m] = (u32)(it - beg);

}

Buffer oiRM::write(RMFile &file, bool compression) {

	RMHeader &header = file.header;

	u32 perIndex = header.vertices <= 256 ? 1 : (header.vertices <= 65536 ? 2 : 4);

	u32 vertexBuffer = (u32)(header.vertexBuffers * sizeof(RMVBO));
	u32 vertexAttribute = (u32)(header.vertexAttributes * sizeof(RMAttribute));
	u32 misc = (u32)(header.miscs * sizeof(RMMisc));
	u32 index = (u32)(header.indices * perIndex);

	std::vector<u8> &indices = file.indices;

	Buffer b = oiSL::write(file.names);
	CopyBuffer vertices;
	Buffer miscBuf(file.miscBuffer);

	if (compression) {

		u32 layoutOff = 0;

		for (u32 j = 0; j < (u32) file.vbos.size(); ++j) {

			RMVBO &vb = file.vbos[j];
			std::vector<u8> &vbo = file.vertices[j];

			u32 offset = 0;

			for (u32 i = layoutOff; i < layoutOff + vb.layouts; ++i) {

				RMAttribute &attrib = file.vbo[i];
				TextureFormat format = attrib.format;

				u32 channels = Graphics::getChannels(format);
				u32 bpc = Graphics::getChannelSize(format);

				std::vector<u32> indices(channels * file.header.vertices);

				CopyBuffer buf;

				for (u32 k = 0; k < file.header.vertices; ++k) {

					for (u32 l = 0; l < channels; ++l)
						if (bpc == 4)
							fillKeyset(buf, vbo, (u32*)(vbo.data() + offset + k * vb.stride + l * bpc), l + k * channels, indices);
						else if (bpc == 2)
							fillKeyset(buf, vbo, (u16*)(vbo.data() + offset + k * vb.stride + l * bpc), l + k * channels, indices);
						else if (bpc == 1)
							fillKeyset(buf, vbo, vbo.data() + offset + k * vb.stride + l * bpc, l + k * channels, indices);
						else if (bpc == 8)
							fillKeyset(buf, vbo, (u64*)(vbo.data() + offset + k * vb.stride + l * bpc), l + k * channels, indices);

				}

				u32 keyset = buf.size() / bpc;
				u32 perKey = (u32) std::ceil(std::log2((f64)keyset));

				Bitset bitset(perKey * (u32) indices.size());
				bitset.write(indices, perKey);

				vertices += CopyBuffer((u8*)&keyset, 4) + buf + bitset.toBuffer();

				offset += channels * bpc;

			}

			layoutOff += vb.layouts;

		}

		header.flags |= RMHeaderFlag1::Uses_compression;

	} else {
		Buffer vert = file.vertices;
		vertices = CopyBuffer(vert);
		vert.deconstruct();
	}

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