#include <types/vector.h>

namespace oi {
	
	namespace wc {
		class Window;
	}
	
	namespace gc {

		class Texture;
		class TextureFormat;
		class TextureUsage;
		
		class Graphics {
			
		public:
			
			~Graphics();
			
			void init(oi::wc::Window *w, u32 buffering = 2);
			
			void initSurface(oi::wc::Window *w);
			void destroySurface();
			
			void clear(Vec4f color);
			void swapBuffers();

			Texture *create(Vec2u res, TextureFormat format, TextureUsage usage);

			const char *getShaderExtension();
			
			u8 *getPlatformData();

		private:
			
			bool initialized = false;
			u32 buffering;
			
			static constexpr u32 platformSize = 4096U;
			u8 platformData[platformSize];
			
		};
		
	}
	
}