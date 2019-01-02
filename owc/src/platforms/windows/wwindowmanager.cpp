#ifdef __WINDOWS__

#include "window/windowmanager.h"
#include "platforms/windows.h"
using namespace oi::wc;

void WindowManager::waitAll() {

	MSG msg;
	memset(&msg, 0, sizeof(msg));

	initAll();

	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
			updateAll();
	}

}


#endif