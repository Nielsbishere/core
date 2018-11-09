#include "window/windowinterface.h"
#include "../graphics.h"

namespace oi {

	namespace gc {

		class GraphicsInterface : public oi::wc::WindowInterface {

		public:

			GraphicsInterface(u32 heapSize) : g(heapSize) {}
			virtual ~GraphicsInterface() { }

			void init() override final;
			virtual void initScene(){}
			virtual void initSceneSurface(Vec2u) {}
			
			void initSurface(Vec2u res) override final;
			void destroySurface() override final;
			void onResize(Vec2u res) override final;
			
			void render() override final;
			virtual void renderScene(){}

			Graphics &getGraphics() { return g; }
			
		protected:

			Graphics g;

		private:

			bool initialized = false;
			
		};

	}
}