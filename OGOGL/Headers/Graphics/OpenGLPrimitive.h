#pragma once

#include <Template/Enum.h>
#include "API/OpenGL.h"

namespace oi {

	namespace gc {

		DEnum(OpenGLPrimitive, u32,
			Triangle = GL_TRIANGLES, TriangleFan = GL_TRIANGLE_FAN, TriangleStrip = GL_TRIANGLE_STRIP,
			Points = GL_POINTS,
			Line = GL_LINES, LineLoop = GL_LINE_LOOP, LineStrip = GL_LINE_STRIP,
			TriangleAdj = GL_TRIANGLES_ADJACENCY, TriangleStripAdj = GL_TRIANGLE_STRIP_ADJACENCY,
			LineAdj = GL_LINES_ADJACENCY, LineStripAdj = GL_LINE_STRIP_ADJACENCY);

	}

}