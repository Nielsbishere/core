#include "Graphics/Mesh/oiRM.h"
#include <Types/GDataType.h>
using namespace oi;

void RM::read(Buffer buf) {

	///Header

	RMHeader &head = buf.operator[]<RMHeader>(0);

	///Strings

	u32 off = sizeof(head);
	u8 *sizes = buf.addr() + off;

	std::vector<OString> names = parseSimpleStringBlock(buf.offset(off += head.strings), sizes, head.strings, off);

	///Textures

	u16 *textures = (u16*) ( buf.addr() + off);
	off += 2 * head.textures;

	///Materials

	RMMaterial *materials = (RMMaterial*) (buf.addr() + off);
	off += sizeof(RMMaterial) * head.materials;

	///Material lists

	u16 *materialLengths = (u16*)(buf.addr() + off);
	off += 2 * head.materialList;

	u32 materialListCount = 0;
	for (u32 i = 0; i < head.materialList; ++i)
		materialListCount += materialLengths[i];

	u16 *materialList = (u16*)(buf.addr() + off);
	off += 2 * materialListCount;

	///Material registers

	u16 *registerLengths = (u16*)(buf.addr() + off);
	off += 2 * head.registers;

	u32 registerCount = 0;
	for (u32 i = 0; i < head.registers; ++i)
		registerCount += registerLengths[i];

	u16 *registers = (u16*)(buf.addr() + off);
	off += 2 * registerCount;

	///Layouts

	RMLayout *layout = (RMLayout*)(buf.addr() + off);
	off += head.layouts * sizeof(RMLayout);

	u32 layoutStride = 0;
	for (u32 i = 0; i < head.layouts; ++i) {
		GDataType_s value = GDataType(layout[i].inputType).getValue();
		layoutStride += value.stride * value.length;
	}

	///Vertices

	Buffer vertex = Buffer(buf.addr() + off, layoutStride * head.vertices);
	off += layoutStride * head.vertices;

	///Indices

	Buffer index = head.indices == 0 ? Buffer() : Buffer(buf.addr() + off, 4 * head.indices);
	off += 4 * head.indices;

	///Miscs


	///Get rid of buffers for test

	vertex.deconstruct();
	index.deconstruct();
}

char RM::decode(u8 val) {
	return val < 10 ? '0' + val : (val < 36 ? 'A' - 10 + val : (val < 62 ? 'a' - 36 + val : (val == 62 ? '.' : '_')));
}

char RM::read(Buffer buf, u32 bitOffset) {

	const u32 bo = bitOffset % 8;
	const u32 byo = bitOffset / 8;
	const u8 &curr = buf[byo];
	const u32 mask0 = 0xFC >> bo;

	if (bo <= 2)
		return decode((curr & mask0) >> (2 - bo));

	const u8 &next = buf[byo + 1];
	const u32 mask1 = (0xFC00 >> bo) & 0xFF;

	return decode(((curr & mask0) << (bo - 2)) | ((next & mask1) >> (10 - bo)));
}

std::vector<OString> RM::parseSimpleStringBlock(Buffer buf, u8 *arr, u16 ilen, u32 &bytes) {

	u32 index = 0;
	std::vector<OString> strings(ilen);

	for (u32 i = 0; i < ilen; ++i) {
		u8 &len = arr[i];

		OString str((u32)len, ' ');

		for (u32 j = 0; j < len; ++j)
			str[j] = read(buf, (index + j) * 6);

		strings[i] = str;
		index += len;
	}

	bytes += (u32) ceil(index * 6 / 8.f);

	return strings;
}