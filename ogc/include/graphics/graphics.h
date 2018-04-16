#include <types/vector.h>
#include "graphics/gl/generic.h"

namespace oi {
	
	namespace wc {
		class Window;
	}
	
	namespace gc {

		class Texture;
		class TextureFormat;
		class TextureUsage;
		class RenderTarget;
		class CommandList;

		struct TextureInfo;
		struct RenderTargetInfo;
		struct CommandListInfo;

		enum class TextureFormatStorage;

		class Graphics {
			
		public:
			
			~Graphics();
			
			void init(oi::wc::Window *w, u32 buffering = 2);
			
			void initSurface(oi::wc::Window *w);							//Inits surface & backbuffer
			void destroySurface();											//Destroys surface & backBuffer
			
			void begin();
			void end();

			Texture *create(TextureInfo info);
			RenderTarget *create(RenderTargetInfo info);
			CommandList *create(CommandListInfo info);

			bool cleanCommandList(CommandList *cmd);

			const char *getShaderExtension();
			GraphicsExt &getExtension();

			bool isDepthFormat(TextureFormat format);
			u32 getChannelSize(TextureFormat format);						//Returns size of one channel in bytes
			u32 getChannels(TextureFormat format);							//Returns number of channels
			u32 getFormatSize(TextureFormat format);						//Returns size of pixel
			TextureFormatStorage getFormatStorage(TextureFormat format);

			Vec4d convertColor(Vec4d color, TextureFormat format);			//Convert color to the correct params

			RenderTarget *getBackBuffer();

		private:
			
			bool initialized = false;
			u32 buffering;

			RenderTarget *backBuffer = nullptr;
			GraphicsExt ext;

			std::vector<CommandList*> commandList;
			
		};
		
	}
	
}