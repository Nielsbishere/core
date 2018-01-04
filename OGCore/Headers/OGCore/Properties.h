#pragma once
#include <Template/PlatformDefines.h>

#define OI_GRAPHICS_OPENGL 0
#define OI_GRAPHICS_DX11 1

#ifdef __WINDOWS__
//#define __DX__
//#define OI_GRAPHICS_TYPE 1
//#define __DX11__
//#define CAN_HANDLE_DX11
//#else
#define __OGL__
#define OI_GRAPHICS_TYPE OI_GRAPHICS_OPENGL
#include "API/OpenGL.h"
#define CAN_HANDLE_OPENGL
#endif