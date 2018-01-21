#pragma once

#include "Graphics/Graphics.h"
#include "ModelInfo.h"
#include <Types/BufferAllocator.h>

namespace oi {

	namespace gc {

		class ModelInstance {

		};

		//Class for storing all model data
		//This is so it works with TMIR (Tiled Material Indirect Rendering)
		//It allocates two buffers on the GPU that will be used to store all vertices and indices
		//This data is not used by the default pipeline but instead used with a compute shader
		//Default is 384 MiB vertex size, 128 MiB index size
		//This is enough for 33 million indices and 12 million vertices
		//So be careful when allocating large models (either increase vertex/index data size or deallocate)
		class ModelData {

		public:

			ModelData(Graphics &graphics, u32 vboSize = 384 * 1024 * 1024, u32 iboSize = 128 * 1024 * 1024);

			//Allocate a ModelInstance
			//Stores the model in the massive arrays
			//ModelInstance stores all information about the location and index
			ModelInstance alloc(ModelInfo mi);

			//Free the ModelInstance so that it can be used later
			bool dealloc(ModelInstance mi);

			//Called before render; sets up all qued models
			void preRender();

		private:

			Graphics &graphics;
			bool init;									//Not set at start; so start will initialize buffers

			std::vector<ModelInstance> instances;		//All instances that are allocated
			std::vector<ModelInstance> changed;			//All allocated/changed model instances in this frame

			BufferAllocator vertices, indices;
			GBuffer vertex, index;
		};

	 }

}