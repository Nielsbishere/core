#pragma once
#include <Types/Generic.h>
#include <Template/PlatformDefines.h>

namespace oi {

	namespace gc {

		enum class GraphicLibrary {
			OpenGL, DirectX, Vulkan, MetalGL, Undefined
		};

		#ifdef __WINDOWS__
		//#define __DX__
		//const GraphicsType type = GraphicsType::DirectX;
		//#else
		#define __OGL__
		const GraphicLibrary graphicLibrary = GraphicLibrary::OpenGL;
		#endif

	}
}