#pragma once
#include <types/vector.h>
#include "graphics/gl/generic.h"
#include <algorithm>

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
		class GBuffer;
		class ShaderBuffer;
		class Sampler;
		class Camera;
		class MeshBuffer;
		class Mesh;
		class DrawList;
		class VersionedTexture;

		class GraphicsObject;

		struct TextureInfo;
		struct RenderTargetInfo;
		struct CommandListInfo;
		struct ShaderInfo;
		struct ShaderStageInfo;
		struct PipelineInfo;
		struct PipelineStateInfo;
		struct GBufferInfo;
		struct ShaderBufferInfo;
		struct SamplerInfo;
		struct CameraInfo;
		struct MeshBufferInfo;
		struct MeshInfo;
		struct DrawListInfo;
		struct VersionedTextureInfo;

		enum class TextureFormatStorage;

		DEnum(TopologyMode, u32,
			Points = 0, Line = 1, Line_strip = 2, Triangle = 3, Triangle_strip = 4, Triangle_fan = 5,
			Line_adj = 6, Line_strip_adj = 7, Triangle_adj = 8, Triangle_strip_adj = 9
		);

		DEnum(FillMode, u32, Fill = 0, Line = 1, Point = 2);
		DEnum(CullMode, u32, None = 0, Back = 1, Front = 2);
		DEnum(WindMode, u32, CCW = 0, CW = 1);
		DEnum(DepthMode, u32, None = 0, Depth_test = 1, Depth_write = 2, All = 3);
		DEnum(BlendMode, u32, Off = 0, Alpha = 1, Add = 2, Subtract = 3);

		class Graphics {

			friend class GraphicsObject;
			
		public:
			
			~Graphics();
			
			void init(oi::wc::Window *w);
			
			void initSurface(oi::wc::Window *w);							//Inits surface & backbuffer
			void destroySurface();											//Destroys surface & backBuffer
			
			void begin();
			void end();
			void finish();

			Texture *create(TextureInfo info);
			RenderTarget *create(RenderTargetInfo info);
			CommandList *create(CommandListInfo info);
			Shader *create(ShaderInfo info);
			ShaderStage *create(ShaderStageInfo info);
			Pipeline *create(PipelineInfo info);
			PipelineState *create(PipelineStateInfo info);
			GBuffer *create(GBufferInfo info);
			ShaderBuffer *create(ShaderBufferInfo info);
			Sampler *create(SamplerInfo info);
			Camera *create(CameraInfo info);
			MeshBuffer *create(MeshBufferInfo info);
			Mesh *create(MeshInfo info);
			DrawList *create(DrawListInfo info);
			VersionedTexture *create(VersionedTextureInfo info);

			GraphicsExt &getExtension();

			static bool isDepthFormat(TextureFormat format);
			static u32 getChannelSize(TextureFormat format);						//Returns size of one channel in bytes
			static u32 getChannels(TextureFormat format);							//Returns number of channels
			static u32 getFormatSize(TextureFormat format);							//Returns size of pixel
			static TextureFormatStorage getFormatStorage(TextureFormat format);		//The type of a texture (float, uint, int)
			static bool isCompatible(TextureFormat a, TextureFormat b);				//Textures are compatible if they match channels and format storage

			static Vec4d convertColor(Vec4d color, TextureFormat format);			//Convert color to the correct params

			RenderTarget *getBackBuffer();
			u32 getBuffering();

			bool contains(GraphicsObject *go) const;
			bool destroy(GraphicsObject *go);
			void use(GraphicsObject *go);

			template<typename T>
			std::vector<GraphicsObject*> get();

		protected:

			template<typename T>
			size_t add(T *t);

			template<typename T, typename TInfo>
			T *init(TInfo info);

			bool remove(GraphicsObject *go);

		private:
			
			bool initialized = false;
			u32 buffering;

			RenderTarget *backBuffer = nullptr;
			GraphicsExt ext;

			std::unordered_map<size_t, std::vector<GraphicsObject*>> objects;
			
		};
		

		template<typename T>
		size_t Graphics::add(T *t) {

			static_assert(std::is_base_of<GraphicsObject, T>::value, "Graphics::add is only available to GraphicsObjects");

			size_t id = typeid(T).hash_code();

			std::vector<GraphicsObject*> &o = objects[id];
			auto it = std::find(o.begin(), o.end(), (GraphicsObject*) t);

			if (it != o.end()) Log::warn("Graphics::add called on an already existing object");
			else o.push_back(t);

			return id;
		}

		template<typename T>
		std::vector<GraphicsObject*> Graphics::get() {

			static_assert(std::is_base_of<GraphicsObject, T>::value, "Graphics::get is only available to GraphicsObjects");

			size_t id = typeid(T).hash_code();

			auto it = objects.find(id);
			if (it == objects.end()) return {};

			return it->second;
		}

		template<typename T, typename TInfo>
		T *Graphics::init(TInfo info) {
			T *t = new T(info);
			t->g = this;

			if (!t->init())
				return (T*) Log::throwError<Graphics, 0xB>("Couldn't init GraphicsObject");

			t->hash = add(t);
			return t;
		}

	}
	
}