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
		class TextureList;
		class MaterialList;

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
		struct TextureListInfo;
		struct MaterialListInfo;

		enum class TextureFormatStorage;

		DEnum(TopologyMode, u32,
			Points = 0, Line = 1, Line_strip = 2, Triangle = 3, Triangle_strip = 4, Triangle_fan = 5,
			Line_adj = 6, Line_strip_adj = 7, Triangle_adj = 8, Triangle_strip_adj = 9, Undefined = 255
		);

		DEnum(FillMode, u32, Fill = 0, Line = 1, Point = 2, Undefined = 255);
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

			Texture *create(String name, TextureInfo info);
			RenderTarget *create(String name, RenderTargetInfo info);
			CommandList *create(String name, CommandListInfo info);
			Shader *create(String name, ShaderInfo info);
			ShaderStage *create(String name, ShaderStageInfo info);
			Pipeline *create(String name, PipelineInfo info);
			PipelineState *create(String name, PipelineStateInfo info);
			GBuffer *create(String name, GBufferInfo info);
			ShaderBuffer *create(String name, ShaderBufferInfo info);
			Sampler *create(String name, SamplerInfo info);
			Camera *create(String name, CameraInfo info);
			MeshBuffer *create(String name, MeshBufferInfo info);
			Mesh *create(String name, MeshInfo info);
			DrawList *create(String name, DrawListInfo info);
			VersionedTexture *create(String name, VersionedTextureInfo info);
			TextureList *create(String name, TextureListInfo info);
			MaterialList *create(String name, MaterialListInfo info);

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
			void printObjects();

			bool contains(GraphicsObject *go) const;
			bool destroy(GraphicsObject *go);
			void use(GraphicsObject *go);

			template<typename T>
			std::vector<GraphicsObject*> get();

		protected:

			template<typename T>
			void add(T *t);

			template<typename T, typename TInfo>
			T *init(String name, TInfo info);

			bool remove(GraphicsObject *go);

		private:
			
			bool initialized = false;
			u32 buffering;

			RenderTarget *backBuffer = nullptr;
			GraphicsExt ext;

			std::unordered_map<size_t, std::vector<GraphicsObject*>> objects;
			
		};
		

		template<typename T>
		void Graphics::add(T *t) {

			static_assert(std::is_base_of<GraphicsObject, T>::value, "Graphics::add is only available to GraphicsObjects");

			size_t id = typeid(T).hash_code();

			std::vector<GraphicsObject*> &o = objects[id];
			auto it = std::find(o.begin(), o.end(), (GraphicsObject*) t);

			if (it != o.end()) Log::warn("Graphics::add called on an already existing object");
			else o.push_back(t);

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
		T *Graphics::init(String name, TInfo info) {

			T *t = new T(info);
			t->g = this;

			t->name = name;
			t->template setHash<T>();

			if (!t->init())
				return (T*) Log::throwError<Graphics, 0xB>("Couldn't init GraphicsObject");

			add(t);
			return t;
		}

	}
	
}