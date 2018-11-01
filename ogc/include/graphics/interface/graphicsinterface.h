#include "window/windowinterface.h"
#include "../graphics.h"

namespace oi {

	namespace gc {

		class GraphicsInterface : public oi::wc::WindowInterface {

		public:

			void init() override;
			virtual void initScene(){}
			virtual void initSceneSurface(Vec2u) {}
			
			void initSurface(Vec2u res) override;
			void destroySurface() override;
			void onResize(Vec2u res) override;
			
			void render() override;
			virtual void renderScene(){}

			Graphics &getGraphics() { return g; }
			
		protected:

			Graphics g;

		private:

			bool initialized = false;
			
		};

	}
}