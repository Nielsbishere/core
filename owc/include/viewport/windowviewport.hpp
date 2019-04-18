#pragma once
#include "viewport.hpp"
#include "window/windowinfo.h"

namespace oi {

	namespace wc {

		struct WindowViewportExt;

		class WindowViewport : public Viewport {

			friend struct WindowViewportExt;

		public:

			WindowViewport(const WindowInfo &info);
			~WindowViewport();
			void init(Window*) final override;
			bool begin() final override { return visible; }
			void end() final override {}
			f32 update() final override;

			void setInterface(WindowInterface*) final override;

			WindowInfo &getInfo() { return info; }
			WindowViewportExt *getExt() { return ext; }

		protected:

			f32 updateInternal();

		private:
			
			WindowInfo info;

			Window *w;
			WindowInterface *wi{};
			WindowViewportExt *ext;

			f32 lastTick = 0;
			bool visible = true, hasInitialized = false;

		};

	}

}