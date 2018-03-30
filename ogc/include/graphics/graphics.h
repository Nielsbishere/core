#include <types/vector.h>

namespace oi {
	
	namespace wc {
		class Window;
	}
	
	namespace gc {
		
		class Graphics {
			
		public:
			
			~Graphics();
			void init(oi::wc::Window *w);
			
			void clear(Vec4f color);
			
		protected:
		
			void initPlatform(oi::wc::Window *w);
			
		private:
			
			bool initialized = false;
			
			static constexpr u32 platformSize = 4096U;
			u8 platformData[platformSize];
			
		};
		
	}
	
}