#pragma once
#include "viewport.hpp"
#include "window/windowinfo.h"

namespace oi {

	namespace wc {

		struct WindowViewportExt;

		class WindowViewport : public Viewport {

		public:

			WindowViewport(const WindowInfo &info);
			~WindowViewport();
			void init(Window*) final override;
			void begin() final override {}
			void end() final override {}
			f32 update() final override;

			void setInterface(WindowInterface*) final override;

			WindowInfo &getInfo() { return info; }
			WindowViewportExt *getExt() { return ext; }

			void setFullScreen(bool fullScreen);
			bool isFullScreen() { return fullScreen; }

		private:
			
			WindowInfo info;

			size_t finalizeCount = 0;
			Window *w;
			WindowInterface *wi{};
			WindowViewportExt *ext;

			f32 lastTick = 0;
			bool fullScreen = false;

		};

	}

}