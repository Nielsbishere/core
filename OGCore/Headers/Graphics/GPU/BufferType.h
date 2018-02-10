#pragma once

namespace oi {

	namespace gc {

		enum class BufferType {
			VBO,					//Vertex Buffer (Object)
			IBO,					//Index
			SSBO,					//Shader Storage
			CBO						//Command (Mainly for Multi Draw Instanced Indirect (MDII))
		};

	}

}