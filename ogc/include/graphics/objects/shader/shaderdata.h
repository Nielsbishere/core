#pragma once

#include "types/bitset.h"
#include "graphics/objects/shader/shader.h"

namespace oi {

	namespace gc {

		class GraphicsResource;
		class ShaderData;

		struct ShaderDataInfo {

			typedef ShaderData ResourceType;

			std::vector<ShaderRegister> registers;

			std::unordered_map<String, ShaderBufferInfo> buffer;
			std::unordered_map<String, GraphicsResource*> shaderData;

			ShaderDataInfo(std::vector<ShaderRegister> registers, std::unordered_map<String, ShaderBufferInfo> buffer) : registers(registers), buffer(buffer) {}
			ShaderDataInfo() : ShaderDataInfo({}, {}) {}

		};

		class ShaderData : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			ShaderDataExt &getExtension();
			const ShaderDataInfo &getInfo() const { return info; }

			bool set(String path, GraphicsResource *res);

			template<typename T>
			void setValue(String path, const T &val);

			template<typename T>
			T *get(String path);

			template<typename T>
			void getValue(String path, T &val);

			void update();

			void requestUpdate();

		protected:

			~ShaderData();
			ShaderData(ShaderDataInfo info) : info(info) {}
			bool init();

			bool initData();
			void destroyData();

		private:

			ShaderDataInfo info;
			ShaderDataExt *ext;

			Bitset changed;

		};

		template<typename T>
		T *ShaderData::get(String path) {

			static_assert(std::is_base_of<GraphicsResource, T>::value, "ShaderData::get<T>(path) where T is base of ShaderResource");

			auto it = info.shaderData.find(path);

			if (it == info.shaderData.end() || it->second == nullptr)
				return nullptr;

			return it->second->cast<T>();

		}

		template<typename T>
		void ShaderData::setValue(String path, const T &val) {

			auto it = info.shaderData.find(path.untilFirst("/"));

			if (it == info.shaderData.end() || it->second == nullptr) {
				Log::warn(String("Shader::setValue(") + path.untilFirst("/") + ") failed; the path couldn't be found");
				return;
			}

			ShaderBuffer *shaderBuffer = it->second->cast<ShaderBuffer>();

			if (shaderBuffer == nullptr) {
				Log::warn(String("Shader::setValue(") + path.untilFirst("/") + ") failed; the path didn't evaluate to a buffer");
				return;
			}

			shaderBuffer->set(path.fromFirst("/"), val);
		}

		template<typename T>
		void ShaderData::getValue(String path, T &val) {

			auto it = info.shaderData.find(path.untilFirst("/"));

			if (it == info.shaderData.end() || it->second == nullptr) {
				Log::warn(String("Shader::getValue(") + path.untilFirst("/") + ") failed; the path couldn't be found");
				return;
			}

			ShaderBuffer *shaderBuffer = it->second->cast<ShaderBuffer>();

			if (shaderBuffer == nullptr) {
				Log::warn(String("Shader::getValue(") + path.untilFirst("/") + ") failed; the path didn't evaluate to a buffer");
				return;
			}

			val = shaderBuffer->get<T>(path.fromFirst("/"));
		}


	}

}