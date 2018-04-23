#pragma once
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
		class Shader;
		class ShaderStage;
		class Pipeline;
		class PipelineState;

		struct TextureInfo;
		struct RenderTargetInfo;
		struct CommandListInfo;
		struct ShaderInfo;
		struct ShaderStageInfo;
		struct PipelineInfo;
		struct PipelineStateInfo;

		enum class TextureFormatStorage;

		DEnum(TopologyMode, u32,

			Points = 0, Line = 1, Line_strip = 2, Triangle = 3, Triangle_strip = 4, Triangle_fan = 5,
			Line_adj = 6, Line_strip_adj = 7, Triangle_adj = 8, Triangle_strip_adj = 9

		);

		DEnum(FillMode, u32, Fill = 0, Line = 1, Point = 2);
		DEnum(CullMode, u32, None = 0, Back = 1, Front = 2);
		DEnum(WindMode, u32, CCW = 0, CW = 1);
		DEnum(DepthMode, u32, None = 0, DepthTestOnly = 1, DepthWriteOnly = 2, All = 3);
		DEnum(BlendMode, u32, Off = 0, Alpha = 1, Add = 2, Subtract = 3);

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
			Shader *create(ShaderInfo info);
			ShaderStage *create(ShaderStageInfo info);
			Pipeline *create(PipelineInfo info);
			PipelineState *create(PipelineStateInfo info);

			bool cleanCommandList(CommandList *cmd);

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