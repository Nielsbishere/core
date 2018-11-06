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

			~BasicGraphicsInterface();

			void initScene() override;
			void onAspectChange(float asp) override;

		protected:

			PipelineState *pipelineState;
			Sampler *sampler;
			ViewBuffer *views;
			Camera *camera;
			CameraFrustum *cameraFrustum;
			View *view;
			CommandList *cmdList;
			MeshManager *meshManager;


		};

	}

}