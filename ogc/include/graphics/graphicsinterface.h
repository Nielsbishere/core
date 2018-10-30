#include <window/windowinterface.h>
#include "graphics.h"

namespace oi {

	namespace gc {

		class GraphicsInterface : public oi::wc::WindowInterface {

		public:

			void init() override;
			virtual void initScene(){}
			virtual void initSceneSurface() {}
			
			void initSurface() override;
			void destroySurface() override;
			void onResize(Vec2u res) override;
			
			void render() override;
			virtual void renderScene(){}

			Graphics &getGraphics() { return g; }
			
		protected:

			Graphics g;
			CommandList *cmdList;

		private:

			bool initialized = false;
			
		};

	}
}