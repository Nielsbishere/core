#pragma once

#include <Template/Enum.h>

namespace oi {

	namespace gc {

		DEnum(Primitive, u32, 
			Triangle = 0, TriangleFan = 1, TriangleStrip = 2,
			Points = 3,
			Line = 4, LineLoop = 5, LineStrip = 6,
			TriangleAdj = 7, TriangleStripAdj = 8,
			LineAdj = 9, LineStripAdj = 10);

	}

}