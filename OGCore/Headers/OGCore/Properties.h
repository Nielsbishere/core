#pragma once
#include <Types/Generic.h>
#include <Template/Enum.h>
#include <Template/PlatformDefines.h>

namespace oi {

	namespace gc {

		enum class GraphicLibrary {
			OpenGL = 0, DirectX = 1, Vulkan = 2, MetalGL = 3
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