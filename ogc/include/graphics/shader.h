#pragma once

#include "gl/generic.h"
#include "graphics/graphicsobject.h"
#include "graphics/shaderbuffer.h"
#include "graphics/shaderstage.h"

namespace oi {

	namespace gc {

		class GraphicsResource;

		struct ShaderInfo {

			String path;

			std::vector<ShaderStage*> stage;
			std::vector<ShaderStageInfo> stages;

			std::vector<ShaderVBVar> var;
			std::vector<ShaderOutput> output;
			std::vector<ShaderRegister> registers;
			std::unordered_map<String, ShaderBufferInfo> buffer;

			std::unordered_map<String, GraphicsResource*> shaderRegister;

			ShaderInfo(String path) : path(path) {}
			ShaderInfo() : ShaderInfo("") {}

		};

		class Shader : public GraphicsObject {

			friend class Graphics;

		public:

			ShaderExt &getExtension();
			const ShaderInfo &getInfo();

			bool isCompute();

			bool set(String path, GraphicsResource *res);

			template<typename T>
			T *get(String path);

			void update();

		protected:

			~Shader();
			Shader(ShaderInfo info);
			bool init();

		private:

			ShaderInfo info;
			ShaderExt ext;

			bool changed = false;

		};

		template<typename T>
		T *Shader::get(String path) {

			static_assert(std::is_base_of<GraphicsResource, T>::value, "Shader::get<T>(path) where T is base of ShaderResource");

			auto it = info.shaderRegister.find(path);

			if (it == info.shaderRegister.end())
				return nullptr;
			
			return dynamic_cast<T*>(it->second);

		}


	}

}