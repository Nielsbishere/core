#pragma once
#include "graphicsinterface.h"

namespace oi {

	namespace gc {

		class PipelineState;
		class Sampler;
		class Camera;
		class CameraFrustum;
		class View;
		class ViewBuffer;
		class CommandList;
		class MeshManager;

		class BasicGraphicsInterface : public GraphicsInterface {

		public:

			static constexpr u32 defaultGraphicsHeapSize = 64 * 1024 * 1024;	//Allocate 64 MiB for our heap

			BasicGraphicsInterface(u32 heapSize = defaultGraphicsHeapSize) : GraphicsInterface(heapSize) {}
			virtual ~BasicGraphicsInterface();

			void initScene() override;
			void onAspectChange(float asp) override;

		protected:

			Sampler *linearSampler, *nearestSampler;
			ViewBuffer *views;
			Camera *camera;
			CameraFrustum *cameraFrustum;
			View *view;
			CommandList *cmdList;
			MeshManager *meshManager;


		};

	}

}