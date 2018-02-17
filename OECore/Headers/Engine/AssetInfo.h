#pragma once

#include <Template/Enum.h>

namespace oi {

	namespace gc {
		class GraphicsResource;
	}

	namespace ec {

		DEnum(AssetState, u32, NotLoaded = 0, CpuLoaded = 1, GpuLoaded = 2);

		struct AssetInfo {

			friend class AssetManager;

		public:

			AssetInfo() : AssetInfo("", "") {}
			AssetInfo(OString _name, OString _path) : name(_name), path(_path), state(AssetState::NotLoaded), ptr(nullptr), assetHandle(0) {}

			OString getName() const { return name; }
			OString getPath() const { return path; }
			AssetState getState() const { return state; }

			gc::GraphicsResource *getResource() { return ptr; }
			u32 getAssetHandle() const { return assetHandle; }

			bool isNull() const { return name == "" && path == ""; }

		private:

			OString name, path;
			AssetState state;

		protected:

			gc::GraphicsResource *ptr;
			u32 assetHandle;
		};

	}

}