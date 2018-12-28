#pragma once
#include "graphicsinterface.h"
#include "graphics/objects/objects.h"

namespace oi {

	namespace gc {

		class BasicGraphicsInterface : public GraphicsInterface {

		public:

			static constexpr u32 defaultGraphicsHeapSize = 64 * 1024 * 1024;	//Allocate 64 MiB for our heap

			BasicGraphicsInterface(u32 heapSize = defaultGraphicsHeapSize) : GraphicsInterface(heapSize) {}
			virtual ~BasicGraphicsInterface() {}

			void initScene() override;
			void onAspectChange(float asp) override;

		protected:

			SamplerRef linearSampler, nearestSampler;
			ViewBufferRef views;
			CameraRef camera;
			CameraFrustumRef cameraFrustum;
			ViewRef view;
			CommandListRef cmdList;
			MeshManagerRef meshManager;


		};

	}

}