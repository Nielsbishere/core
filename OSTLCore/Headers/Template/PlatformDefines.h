#pragma once

#include "BuildProperties.h"

#ifdef _WIN32				//Windows
#define __WINDOWS__
#define __DESKTOP__
#elif __APPLE__				//Apple
#include "TargetConditionals.h"
#ifdef TARGET_OS_IPHONE				//iOS
#define __IOS__
#define __PHONE__
#else								//OS X
#define __OSX__
#define __DESKTOP__
#endif
#elif __ANDROID				//Android
#define __ANDROID__
#define __PHONE__
#elif XBOXONE
#define __XBONE__
#define __CONSOLE__
#elif _PS4
#define __PS4__
#define __CONSOLE__
#else						//Linux
#define __DESKTOP__
#endif

#if defined(__linux__) || defined(__APPLE__)
#define __UNIX__
#endif

#ifdef __WINDOWS__

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h> 
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#undef min
#undef max
#undef ERROR

#ifdef HIDE_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

#endif