#pragma once

#include <Template/Enum.h>
#include "API/OpenGL.h"

namespace oi {

	namespace gc {

		DEnum(OpenGLSamplerWrapping, u32, Repeat = GL_REPEAT, MirrorRepeat = GL_MIRRORED_REPEAT, ClampEdge = GL_CLAMP_TO_EDGE, ClampBorder = GL_CLAMP_TO_BORDER, MirrorClampEdge = GL_MIRROR_CLAMP_TO_EDGE);
		DEnum(OpenGLSamplerMin, u32, LinearMip = GL_LINEAR_MIPMAP_LINEAR, NearestMip = GL_NEAREST_MIPMAP_NEAREST, Linear = GL_LINEAR, Nearest = GL_NEAREST, LinearMipNearest = GL_LINEAR_MIPMAP_NEAREST, NearestMipLinear = GL_NEAREST_MIPMAP_LINEAR);
		DEnum(OpenGLSamplerMag, u32, Linear = GL_LINEAR, Nearest = GL_NEAREST);

	}

}