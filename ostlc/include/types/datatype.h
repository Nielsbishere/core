#pragma once

#include "template/enum.h"

namespace oi {
	SEnum(DataType, _(u32 id, stride, length, derivedId;),
		oi_undefined = _({ 0, 0, 0, 0 }),
		oi_float = _({ 1, 4, 1, 1 }), oi_float2 = _({ 2, 4, 2, 1 }), oi_float3 = _({ 3, 4, 3, 1 }), oi_float4 = _({ 4, 4, 4, 1 }),
		oi_double = _({ 5, 8, 1, 5 }), oi_double2 = _({ 6, 8, 2, 5 }), oi_double3 = _({ 7, 8, 3, 5 }), oi_double4 = _({ 8, 8, 4, 5 }),
		oi_int = _({ 9, 4, 1, 9 }), oi_int2 = _({ 10, 4, 2, 9 }), oi_int3 = _({ 11, 4, 3, 9 }), oi_int4 = _({ 11, 4, 4, 9 }),
		oi_uint = _({ 12, 4, 1, 12 }), oi_uint2 = _({ 13, 4, 2, 12 }), oi_uint3 = _({ 14, 4, 3, 12 }), oi_uint4 = _({ 15, 4, 4, 12 }),
		oi_bool = _({ 16, 4, 1, 16 }), oi_bool2 = _({ 17, 4, 2, 16 }), oi_bool3 = _({ 18, 4, 3, 16 }), oi_bool4 = _({ 19, 4, 4, 16 }),
		oi_struct = _({ 255, 0, 0, 255 })
	);
}