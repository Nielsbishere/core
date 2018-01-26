//#include "Graphics/Format/oiRM.h"
//#include <Utils/Log.h>
//using namespace oi::gc;
//using namespace oi;
//
//bool RM::init(RMInfo oiRMinf) {
//
//	Buffer b = Buffer::readFile(oiRMinf.getPath());
//
//	RMHeader &head = b.operator[]<RMHeader>(0);
//	Buffer off = b.offset(sizeof(RMHeader));
//
//	OString error = "";
//
//	if (OString(head.head, 4) != "oiRM") {
//		error = "Couldn't read oiRM; invalid header";
//		goto exit;
//	}
//
//	bool result = true;
//
//	if (head.version == 1)
//		result = loadx1(off, head);
//	else {
//		error = "Couldn't read oiRM; version is not supported. Support for x0 is dropped and x2+ is not available.";
//		goto exit;
//	}
//
//	exit:
//
//	b.deconstruct();
//
//	if (error != "")
//		Log::error(error);
//
//	return error == "" && result;
//}
//
//
//const OString RM::varName = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz__";
//const OString RM::name = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz _";
//
//OString RM::parseSlimString(Buffer b, u32 len, const OString &encoding) {
//
//	OString result(len, ' ');
//
//	for (u32 i = 0; i < len; ++i) {
//		switch (i % 4U) {
//		case 0:
//			result[i] = encoding.at((b.get<u8>(i / 4) & 0xFCU) >> 2U);
//			break;
//		case 1:
//			result[i] = encoding.at(((b.get<u8>(i / 4) & 0x03U) << 4U) | ((b.get<u8>(i / 4 + 1) & 0xF0U) >> 4));
//			break;
//		case 2:
//			result[i] = encoding.at(((b.get<u8>(i / 4 + 1) & 0x0FU) << 2U) | ((b.get<u8>(i / 4 + 2) & 0xC0U) >> 6));
//			break;
//		default:
//			result[i] = encoding.at(b.get<u8>(i / 4 + 2) & 0x3FU);
//			break;
//		}
//	}
//
//	return result;
//}
//
//bool RM::loadx1(Buffer &b, RMHeader &head) {
//
//	layouts.resize(head.layouts);
//
//	for (u32 i = 0; i < head.layouts; ++i) {
//		auto &layout = layouts[i];
//		layout.type = b.get<u16>(0);
//		layout.count = b.get<u16>(2);
//		layout.name = parseSlimString(b.offset(5), b.get(4), varName);
//		b = b.offset(5 + ceil(layout.name.size() * 6 / 8.f));
//	}
//
//	vao.init(VAOInfo(layouts));
//
//	return true;
//}
//
//u32 RM::floatAsUint(f32 f, u32 i) {
//	u32 u = (u32)abs(f);
//	if (f >= 0) return u;
//	--u;
//	u = i - u;
//	return u | (i + 1U);
//}
//
//u32 RM::compressNormal(Vec3 normal) {
//	Vec3i v = normal.normalize() * Vec3(511, 1023, 1023);
//	return (floatAsUint(v.x(), 511) << 22) | (floatAsUint(v.y(), 1023) << 11) | floatAsUint(v.z(), 1023);
//}