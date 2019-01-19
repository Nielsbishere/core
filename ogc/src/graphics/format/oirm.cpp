#include "file/filemanager.h"
#include "types/bitset.h"
#include "utils/timer.h"
#include "graphics/format/oirm.h"
#include "graphics/objects/model/mesh.h"
using namespace oi::gc;
using namespace oi::wc;
using namespace oi;

RMFile oiRM::generate(Buffer vbo, Buffer bibo, bool hasPos, bool hasUv, bool hasNrm, u32 vertices, u32 indices) {

	u32 stride = (hasPos ? 12 : 0) + (hasUv ? 8 : 0) + (hasNrm ? 12 : 0);

	if (vbo.size() != stride * vertices) {
		Log::error("Couldn't generate oiRM file; the vbo was of invalid size");
		return RMFile();
	}

	if (bibo.size() != 4 * indices) {
		Log::error("Couldn't generate oiRM file; the ibo was of invalid size");
		return RMFile();
	}

	CopyBuffer fibo(bibo.addr(), indices * 4);

	u32 attributeCount = (u32) hasPos + hasUv + hasNrm;
	std::vector<String> names;
	std::vector<RMAttribute> attributes;

	names.reserve(attributeCount);
	attributes.reserve(attributeCount);

	if (hasPos) {
		names.push_back("inPosition");
		attributes.push_back({ (u8)TextureFormat::RGB32f, (u16)0 });
	}

	if (hasUv) {
		names.push_back("inUv");
		attributes.push_back({ (u8)TextureFormat::RG32f, (u16)attributes.size() });
	}

	if (hasNrm) {
		names.push_back("inNormal");
		attributes.push_back({ (u8)TextureFormat::RGB32f, (u16)attributes.size() });
	}

	std::vector<RMVBO> vboInfo(1); 
	vboInfo[0] = RMVBO(
		(u16)((hasPos ? sizeof(Vec3) : 0) + (hasUv ? sizeof(Vec2) : 0) + (hasNrm ? sizeof(Vec3) : 0)),
		(u16)attributeCount
	);

	return RMFile(

		//Header
		{
			{ 'o', 'i', 'R', 'M' },
			(u8)RMHeaderVersion::V0_0_1,
			(u8)RMHeaderFlag1::None,
			(u8)1,
			(u8)attributeCount,

			(u8)TopologyMode::Undefined,
			(u8)FillMode::Undefined,
			(u8)0,
			(u8)0,

			0,

			(u32)vertices,
			(u32)indices

		},

		vboInfo,
		attributes,
		{},
		{ vbo },
		fibo,
		{},
		SLFile(String::getDefaultCharset(), names)
	);
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

		u32 perIndexb = (u32) std::ceil(std::log2(file.header.vertices));

		u32 vertexBuffer = file.header.vertexBuffers * (u32) sizeof(RMVBO);
		u32 vertexAttribute = file.header.vertexAttributes * (u32) sizeof(RMAttribute);
		u32 misc = file.header.miscs * (u32) sizeof(RMMisc);

		u32 destSize = vertexBuffer + vertexAttribute + misc;

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
		CopyBuffer *vbdat = file.vertices.data();
		std::vector<u32> indices, indices0;

		for (u32 i = 0; i < file.header.vertexBuffers; ++i) {

			*vbdat = CopyBuffer(vbo->stride * file.header.vertices);

			u8 *vbdata = vbdat->addr();

			memset(vbdata, 0, vbo->stride * file.header.vertices);

			if (!compression) {

				u32 length = vbo->stride * file.header.vertices;

				if (length > read.size())
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

					if (!read.read(keyset))
						return Log::error("Couldn't read oiRM file; invalid keyCount");

					Bitset bitset;

					if (!read.read(bitset, keyset * channels * perKey))
						return Log::error("Couldn't read oiRM file; invalid bitset");

					indices.resize(keyset * channels);
					bitset.read(indices, perKey);

					perKey = (u32)std::ceil(std::log2((f64)keyset));
					keyset = file.header.vertices;

					if (!read.read(bitset, keyset * perKey))
						return Log::error("Couldn't read oiRM file; invalid bitset");

					indices0.resize(keyset);
					bitset.read(indices0, perKey);

					u8 *dest = values.addr();
					u32 *aindices = indices.data();
					u32 *aindices0 = indices0.data();

					for (u32 k = 0; k < channels * file.header.vertices; ++k)
						memcpy(vbdata + offset + k / channels * vbo->stride + k % channels * bpc, dest + aindices[aindices0[k / channels] * channels + k % channels] * bpc, bpc);

					offset += channels * bpc;
					++attrib;

				}

				attr += vbo->layouts;

			}

			++vbo;
			++vbdat;

		}

		if (file.header.indices != 0) {

			if (!compression) {

				u32 index = file.header.indices * 4;

				if (read.size() < index)
					return Log::error("Couldn't read oiRM file; invalid index buffer length");

				file.indices = CopyBuffer(read.addr(), index);
				read = read.offset(index);

			} else {

				if (file.header.indexOperations == 0) {

					u32 indexb = file.header.indices * perIndexb;

					Bitset bitset;
					if (!read.read(bitset, indexb))
						return Log::error("Couldn't read oiRM file; invalid index buffer length");

					std::vector<u32> ind(file.header.indices);
					bitset.read(ind, perIndexb);

					u32 indexRes = file.header.indices * 4;
					file.indices = CopyBuffer((u8*)ind.data(), indexRes);

				} else {
				
					Bitset ops;
					if(!read.read(ops, 2 * file.header.indexOperations))
						return Log::error("Couldn't read oiRM file; invalid operation bitset length");

					u32 opLen = 0;
					for (u32 i = 0; i < file.header.indexOperations; ++i)
						if (ops[i * 2] || ops[i * 2 + 1])
							++opLen;
						else
							opLen += 3;

					Bitset contents;
					if(!read.read(contents, opLen * perIndexb))
						return Log::error("Couldn't read oiRM file; invalid operationData bitset length");

					std::vector<u32> indOps(opLen);
					contents.read(indOps, perIndexb);

					file.indices = CopyBuffer(file.header.indices * 4);
					u32 *aindices = file.indices.addr<u32>();
					u32 *aindOps = indOps.data();

					u32 i = 0, j = 0;
					while (i < file.header.indices) {

						bool b0 = ops[j * 2];
						bool b1 = ops[j * 2 + 1];
						u32 n = aindOps[j];

						if (!b0 && b1) {				//Quad
							aindices[i] = n + 2;
							aindices[i + 1] = n + 1;
							aindices[i + 2] = n;
							aindices[i + 3] = n + 3;
							aindices[i + 4] = n + 2;
							aindices[i + 5] = n;
							i += 3;
						} else if (b0 && !b1) {			//RevIndInc
							aindices[i] = n + 2;
							aindices[i + 1] = n + 1;
							aindices[i + 2] = n;
						} else if (b0 && b1) {			//RevIndInc2
							aindices[i] = n + 3;
							aindices[i + 1] = n + 2;
							aindices[i + 2] = n;
						} else {
							memcpy(aindices + i, aindOps + j, 12);
							j += 2;
						}

						i += 3;
						++j;
					}
				
				}

			}
		}

		file.miscBuffer.resize(file.header.miscs);
		for (u32 i = 0; i < file.header.miscs; ++i) {

			u32 length = file.miscs[i].size;

			if (length > read.size())
				return Log::error("Couldn't read oiRM file; invalid misc length");

			file.miscBuffer[i] = CopyBuffer(length);
			memcpy(file.miscBuffer[i].addr(), read.addr(), length);
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

std::pair<MeshBufferInfo, MeshInfo> oiRM::convert(RMFile file) {

	std::pair<MeshBufferInfo, MeshInfo> result;

	std::vector<std::vector<std::pair<String, TextureFormat>>> vbos(file.vbos.size());
	std::vector<Buffer> vb(vbos.size());
	Buffer ib;

	u32 i = 0, j = 0;

	for (RMVBO vbo : file.vbos) {

		vbos[i].resize(vbo.layouts);
		vb[i] = Buffer(file.vertices[i].addr(), (u32)file.vertices[i].size());

		for (u32 k = 0; k < vbo.layouts; ++k)
			vbos[i][k] = { file.names.names[file.vbo[k].name], file.vbo[k].format };

		j += vbo.layouts;
		++i;
	}

	if (file.header.indices != 0)
		ib = Buffer(file.indices.addr(), file.header.indices * 4);

	result.first = MeshBufferInfo(file.header.vertices, file.header.indices, 0, vbos, file.header.topologyMode, file.header.fillMode);
	result.second = MeshInfo(nullptr, file.header.vertices, file.header.indices, vb, ib);
	
	return result;
}

RMFile oiRM::convert(const MeshInfo &info) {

	const MeshBufferInfo &meshBuffer = info.buffer->getInfo();

	auto buffers = meshBuffer.buffers;
	u32 attributeCount = 0;

	std::vector<RMAttribute> attributes;
	std::vector<String> names;
	std::vector<RMVBO> vbos(buffers.size());
	std::vector<CopyBuffer> vertices(buffers.size());

	u32 i = 0, j = 0;

	for (auto &elem : buffers) {

		attributeCount += (u32) elem.size();
		attributes.reserve(attributeCount);
		names.reserve(attributeCount);

		u32 size = 0;

		for (auto &pair : elem) {
			attributes.push_back({ (u8)pair.second.getValue(), (u16)j });
			names.push_back(pair.first);
			size += Graphics::getFormatSize(pair.second);
			++j;
		}

		vbos[i] = { (u16) size, (u16) elem.size() };
		vertices[i] = info.vbo[i];

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

			0,

			info.vertices,

			info.indices
		},

		vbos,
		attributes,
		miscs,
		vertices,
		info.ibo,
		{},
		SLFile(String::getDefaultCharset(), names)

	};
}

u32 findChannel(u8 *data, u32 totalSize, u8 *c, u32 bpc) {

	for (u32 i = 0; i < totalSize; i += bpc)
		if (memcmp(data + i, c, bpc) == 0)
			return i / bpc;

	return totalSize / bpc;

}

u32 findAttribute(u32 *attributes, u32 totalSize, u32 *channels, u32 channelCount) {

	for (u32 i = 0; i < totalSize / channelCount; ++i)
		if (memcmp(attributes + i * channelCount, channels, 4 * channelCount) == 0)
			return i;

	return totalSize / channelCount;
}

bool addChannel(CopyBuffer &buf, u32 bpc, u8 *addr, u32 *channel) {

	*channel = findChannel(buf.addr(), buf.size(), addr, bpc);

	if (*channel == buf.size() / bpc) {
		buf += CopyBuffer(addr, bpc);
		return true;
	}

	return false;
}

Buffer oiRM::write(RMFile &file, bool compression) {

	Timer t;

	RMHeader &header = file.header;

	u32 perIndexb = (u32) std::ceil(std::log2(file.header.vertices));

	u32 vertexBuffer = (u32)(header.vertexBuffers * sizeof(RMVBO));
	u32 vertexAttribute = (u32)(header.vertexAttributes * sizeof(RMAttribute));
	u32 misc = (u32)(header.miscs * sizeof(RMMisc));

	CopyBuffer ind;

	Buffer b = oiSL::write(file.names);
	CopyBuffer vertices;

	u32 miscBufLen = 0;
	for (CopyBuffer &cb : file.miscBuffer)
		miscBufLen += cb.size();

	Buffer miscBuf(miscBufLen);

	miscBufLen = 0;
	for (CopyBuffer &cb : file.miscBuffer) {
		memcpy(miscBuf.addr() + miscBufLen, cb.addr(), cb.size());
		miscBufLen += cb.size();
	}

	if (compression) {

		u32 layoutOff = 0;

		//Vertices

		for (u32 j = 0; j < (u32) file.vbos.size(); ++j) {

			RMVBO &vb = file.vbos[j];
			CopyBuffer &vbo = file.vertices[j];
			u8 *avbo = vbo.addr();

			u32 offset = 0;

			for (u32 i = layoutOff; i < layoutOff + vb.layouts; ++i) {

				RMAttribute &attr = file.vbo[i];
				TextureFormat format = attr.format;

				u32 channels = Graphics::getChannels(format);
				u32 bpc = Graphics::getChannelSize(format);

				std::vector<u32> uniqueChannels;
				uniqueChannels.reserve(channels * file.header.vertices);
				u32 *auniqueChannels = uniqueChannels.data(), auniqueChannelC = 0;

				std::vector<u32> currentChannel(channels);
				u32 *acurrentChannel = currentChannel.data();

				std::vector<u32> attributes(file.header.vertices);
				u32 *aattributes = attributes.data();

				CopyBuffer chan;

				for (u32 k = 0; k < file.header.vertices; ++k) {

					u8 *ptr = avbo + offset + k * vb.stride;

					bool modified = false;

					//Add channels (if the channel is new, the attribute is)
					for (u32 l = 0; l < channels; ++l)
						modified = addChannel(chan, bpc, ptr + l * bpc, acurrentChannel + l) || modified;

					u32 attrib = auniqueChannelC / channels;

					//Check if the combinations of channels is new
					if (!modified) {
						attrib = findAttribute(auniqueChannels, auniqueChannelC, acurrentChannel, channels);
						modified = attrib == auniqueChannelC / channels;
					}

					//Push attribute
					if (modified) {
						uniqueChannels.insert(uniqueChannels.end(), currentChannel.begin(), currentChannel.end());
						auniqueChannelC = (u32)uniqueChannels.size();
					}

					aattributes[k] = attrib;

				}

				u32 channelKey = chan.size() / bpc;
				u32 perChannel = (u32)std::ceil(std::log2(channelKey));
				u32 totalAttributes = (u32)uniqueChannels.size() / channels;
				u32 perAttribute = (u32)std::ceil(std::log2(totalAttributes));

				Bitset bitset(perChannel * (u32)uniqueChannels.size());
				bitset.write(uniqueChannels, perChannel);

				vertices += CopyBuffer((u8*)&channelKey, 4) + chan + CopyBuffer((u8*)&totalAttributes, 4) + bitset.toBuffer();

				//TODO: Don't export per attribute compression for 1 channel
				//if (channels != 1) {

					Bitset bitset0(perAttribute * (u32) attributes.size());
					bitset0.write(attributes, perAttribute);

					vertices += bitset0.toBuffer();

				//}

				offset += channels * bpc;

			}

			layoutOff += vb.layouts;

		}

		//Indices

		if (file.header.indices != 0) {

			u32 *aindices = file.indices.addr<u32>();
			std::vector<u32> indices(aindices, file.indices.addrEnd<u32>());

			//Undefined mode is basically triangle; but allows you to turn it into wireframe
			//This is operation compression
			if (file.header.topologyMode == TopologyMode::Triangle || file.header.topologyMode == TopologyMode::Undefined) {

				bool prev = false;
				u32 i00 = 0,	//no op
					i01 = 0,	//quad op
					i10 = 0,	//rev inc op
					i11 = 0;	//rev inc op; first step 2

				Bitset ops(2 * file.header.indices / 3);			//Could be smaller than per tri; operations bitset
				std::vector<u32> start(file.header.indices / 3);	//Info where the operation's begin is
				u32 opOff = 0, totalInd = 0, test = 0;

				for (u32 i = 0, j = file.header.indices / 3; i < j; ++i) {

					Vec3u curr = *(Vec3u*) (aindices + i * 3);

					if (curr.x == curr.y + 1 && curr.y == curr.z + 1) {
						if(!prev)
							prev = true;
						else {				//RevIndInc
							ops[2 * opOff] = true;
							ops[2 * opOff + 1] = false;
							start[opOff] = (i - 1) * 3 + 2;		//Save the base index
							++opOff;
							++i10;
							++totalInd;
							test += 3;
						}
					} else if (curr.x == curr.y + 1 && curr.y == curr.z + 2) {

						//RevIndInc2
						if (!prev) {
							ops[2 * opOff] = true;
							ops[2 * opOff + 1] = true;
							start[opOff] = i * 3 + 2;			//Save the base index
							++opOff;
							++i11;
							++totalInd;
							test += 3;
							continue;
						}

						//Quad
						ops[2 * opOff] = false;
						ops[2 * opOff + 1] = true;
						start[opOff] = i * 3 + 2;				//Save the base index
						++opOff;
						++i01;
						++totalInd;
						test += 6;
						prev = false;

					} else {

						//RevIndInc
						if (prev) {
							ops[2 * opOff] = true;
							ops[2 * opOff + 1] = false;
							start[opOff] = (i - 1) * 3 + 2;		//Save the base index
							++opOff;
							++i10;
							++totalInd;
							test += 3;
							prev = false;
						}

						//NoOp
						ops[2 * opOff] = false;
						ops[2 * opOff + 1] = false;
						start[opOff] = i * 3;					//Save the base index
						++opOff;
						++i00;
						test += 3;
						totalInd += 3;

					}


				}

				//If last op was RevIndInc
				if (prev) {
					ops[2 * opOff] = true;
					ops[2 * opOff + 1] = false;
					start[opOff] = (file.header.indices / 3 - 1) * 3 + 2;	//Save the base index
					++opOff;
					++i10;
					test += 3;
				}

				if (i01 == 0 && i10 == 0 && i11 == 0) {		//Operations don't have any effect; so don't use them

					Bitset bitset(perIndexb * file.header.indices);
					bitset.write(indices, perIndexb);

					ind = bitset.toBuffer();

				} else {

					file.header.indexOperations = opOff;

					Bitset operationData(totalInd * perIndexb);
					std::vector<u32> indOpBuf(totalInd);
					u32 *aindOpBuf = indOpBuf.data();

					u32 j = 0;
					for (u32 i = 0; i < opOff; ++i) {

						if (ops[2 * i] || ops[2 * i + 1])
							indOpBuf[j] = aindices[start[i]];
						else {
							memcpy(aindOpBuf + j, aindices + start[i], 12);
							j += 2;
						}

						++j;
					}

					operationData.write(indOpBuf, perIndexb);

					Bitset aops(opOff * 2);
					aops |= ops;

					ind = aops.toBuffer() + operationData.toBuffer();

				}

			} else {

				Bitset bitset(perIndexb * file.header.indices);
				bitset.write(indices, perIndexb);

				ind = bitset.toBuffer();

			}

		}

		//Turn on the flag

		header.flags |= RMHeaderFlag1::Uses_compression;

	} else {

		
		u32 vertexLength = 0;
		for (CopyBuffer &cb : file.vertices)
			vertexLength += cb.size();

		vertices = CopyBuffer(vertexLength);
		vertexLength = 0;
		for (CopyBuffer &cb : file.vertices) {
			memcpy(vertices.addr() + vertexLength, cb.addr(), cb.size());
			vertexLength += cb.size();
		}

		ind = file.indices;

	}

	file.size = (u32) sizeof(header) + vertexBuffer + vertexAttribute + misc + vertices.size() + ind.size() + miscBuf.size() + file.names.size;

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

	memcpy(write.addr(), ind.addr(), ind.size());
	write = write.offset(ind.size());

	memcpy(write.addr(), miscBuf.addr(), miscBuf.size());
	write = write.offset(miscBuf.size());
	miscBuf.deconstruct();

	write.copy(b, b.size(), 0, 0);
	write = write.offset(b.size());
	b.deconstruct();

	/*t.print();*/

	return output;

}

bool oiRM::write(RMFile &file, String path, bool compression) {

	Buffer buf = write(file, compression);

	if (!FileManager::get()->write(path, buf)) {
		buf.deconstruct();
		return Log::error("Couldn't write to file");
	}

	buf.deconstruct();
	return true;

}