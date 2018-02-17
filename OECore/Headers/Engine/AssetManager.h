#pragma once

#include <Graphics/Graphics.h>
#include <Utils/JSON.h>
#include "AssetInfo.h"

namespace oi {

	namespace ec {

		class AssetManager {

		public:

			AssetManager();
			~AssetManager();

			void unloadGPU();
			void unloadCPU();
			void unload();

			u32 size();
			AssetInfo find(u32 i);						//Get asset info at asset handle
			AssetInfo find(OString name);				//Get asset info at name
			AssetInfo findByPath(OString path);			//Get asset info at path

			template<class T>
			T *get(u32 i) {
				static_assert(std::is_base_of<gc::GraphicsResource, T>::value, "AssetManager::get<T>(i) requires T to be a subclass of GraphicsResource");
				return dynamic_cast<T*>((*this)[i]);
			}

			template<class T>
			T *get(OString name) {
				static_assert(std::is_base_of<gc::GraphicsResource, T>::value, "AssetManager::get<T>(i) requires T to be a subclass of GraphicsResource");
				return dynamic_cast<T*>(find(name).getResource());
			}

			template<class T>
			T *getByPath(OString path) {
				static_assert(std::is_base_of<gc::GraphicsResource, T>::value, "AssetManager::get<T>(i) requires T to be a subclass of GraphicsResource");
				return dynamic_cast<T*>(findByPath(path).getResource());
			}

			u32 create(AssetInfo info);
			
			template<class T>
			bool initCPU(u32 index, T tinfo) {

				if (index >= assetIndex || assets.find(index) == assets.end()) {
					Log::error("Asset not found exception");
					return false;
				}

				AssetInfo &info = assets[index];

				if (info.state.getIndex() >= AssetState::CpuLoaded) {
					Log::error("Asset already loaded exception");
					return false;
				}

				info.ptr = (GraphicsResource*) gc::Graphics::get()->create(tinfo);
				info.state = AssetState::CpuLoaded;

				info.ptr->initData(info.path);

				return true;
			}

			bool initData(u32 index);

			bool initResource(JSON &json, OString path, u32 i, OString type);
			u32 addResources(JSON &json, OString path);			//Automatically adds & loads all resources (returns u32_MAX if invalid); doesn't load on GPU (call 'loadGPU' after done adding resources)
			u32 addResources(OString path);						//Automatically adds resources located in path "Resources"

			void initGPU();
			void init(OString path);							//Init from path

			gc::GraphicsResource *operator[](u32 i);			//Returns GraphicsResource from asset index

		private:

			std::unordered_map<u32, AssetInfo> assets;
			u32 assetIndex;

		};

	}

}