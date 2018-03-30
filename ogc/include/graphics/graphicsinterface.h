#include <window/windowinterface.h>
#include "graphics.h"

namespace oi {

	namespace gc {

		class GraphicsInterface : public oi::wc::WindowInterface {

		public:

			void init() override;
			virtual void initScene(){}
			
			void render() override;
			virtual void renderScene(){}

			
		private:
		
			Graphics g;
			
		};

	}
}