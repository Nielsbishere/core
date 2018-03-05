#pragma once

#include <Types/OString.h>
#include <Graphics/GPU/GraphicsResource.h>
#include <map>

namespace oi {

	namespace gc {

		enum class ResourceState {
			RNL, CPU, GPU
		};

		struct ResourceCounter {
			GraphicsResource *gr;
			u32 count, id;
			ResourceState state;
		};

		//Class for keeping track of all objects created through GL
		//so that they can be reloaded when the GLAPI changes and/or destroyed when the application ends
		//Uses refcount and names per resource; wrapping around GraphicsResource
		class ResourceManager {

			friend class Graphics;

		public:

			~ResourceManager() {
				
				for (auto resource : resources)
					release(resource.first, true);

				resources.clear();
			}

			void initCPU();
			void initGPU();

			void initCPU(OString name);
			void initGPU(OString name);

			u32 ResourceManager::track(OString name) {
				bool b = false;
				return track(name, b);
			}

			u32 ResourceManager::track(OString name, bool &isAdded) {

				name = name.toLowerCase();

				auto it = resources.find(name);

				if (it == resources.end()) {

					ResourceCounter rc;
					rc.id = (u32) counter;
					rc.count = 0;
					rc.gr = nullptr;
					rc.state = ResourceState::RNL;

					resources[name] = rc;

					it = resources.find(name);
					isAdded = true;
					++counter;
				}

				auto &elem = it->second;
				++elem.count;
				return elem.id;
			}

			void ResourceManager::set(OString s, GraphicsResource *gr) {

				auto it = resources.find(s);

				if (it == resources.end())
					return;

				auto *&sgr = it->second.gr;

				if (sgr == nullptr)
					sgr = gr;
			}

			bool ResourceManager::release(OString name, bool avoidMapRemove = false) {

				auto it = resources.find(name);

				if (it == resources.end())
					return false;

				auto &elem = it->second;
				--elem.count;

				if (elem.count > 0) return true;

				delete elem.gr;

				if(!avoidMapRemove)
					resources.erase(name);

				return false;
			}

			template<class T = GraphicsResource>
			T *get(OString name){ 

				auto it = resources.find(name);

				if (it == resources.end())
					return nullptr;

				return dynamic_cast<T*>(it->second.gr);
			}

		protected:

			OString getPath(OString handle);

		private:

			std::unordered_map<OString, OString> resourceTypes = {
				{ "tex", "textures" },
				{ "sam", "samplers" },
				{ "mod", "models" },
				{ "mat", "materials" },
				{ "buf", "buffers" },
				{ "sha", "shaders" }
			};

			std::map<OString, ResourceCounter> resources;
			u32 counter = 0;
		};

	}

}