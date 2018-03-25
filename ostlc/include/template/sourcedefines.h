#pragma once

///Include this file in main.cpp and set defines before that

#ifdef HIDE_CONSOLE
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif