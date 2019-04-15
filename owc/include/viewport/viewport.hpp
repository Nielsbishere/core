#pragma once
#include "types/array.h"
#include "types/vector.h"

namespace oi {

	namespace wc {

		class Window;
		class WindowInterface;

		struct ViewportLayer {

			Vec2u start, size;
			u32 layer;

			ViewportLayer() : start(0), size(0), layer(0) {}
			ViewportLayer(Vec2u start, Vec2u size, u32 layer) : start(start), size(size), layer(layer) {}

		};

		class Viewport {

		public:

			Viewport(const Array<ViewportLayer> &layers) : layers(layers) {}
			virtual ~Viewport() = default;
			Viewport(const Viewport&) = delete;
			Viewport(Viewport&&) = delete;
			Viewport &operator=(const Viewport&) = delete;
			Viewport &operator=(Viewport&&) = delete;

			virtual void init(Window*) = 0;
			virtual void setInterface(WindowInterface *wif) { wi = wif; }
			virtual void begin() = 0;
			virtual void end() = 0;
			virtual f32 update() = 0;

			Array<ViewportLayer> &getLayers() { return layers; }
			ViewportLayer &getLayer(size_t i) { return layers[i]; }

		protected:

			Array<ViewportLayer> layers;
			WindowInterface *wi = nullptr;

		};

	}

}