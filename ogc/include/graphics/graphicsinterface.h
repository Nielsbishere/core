#include <window/windowinterface.h>
#include "graphics.h"

namespace oi {

	namespace gc {

		class GraphicsInterface : public oi::wc::WindowInterface {

		public:

			void init() override;
			virtual void initScene(){}
			
			virtual void initSurface() override;
			void destroySurface() override;
			virtual void onResize(Vec2u res) override;
			
			void render() override;
			virtual void renderScene(){}

			
		protected:
		
			Graphics g;
			CommandList *cmdList;
			
		};

	}
}