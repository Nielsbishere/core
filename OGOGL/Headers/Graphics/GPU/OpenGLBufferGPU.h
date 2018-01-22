#pragma once

#include "API/OpenGL.h"
#include <Graphics/GPU/BufferGPU.h>

namespace oi {

	namespace gc {

		class OpenGLBufferGPU : public BufferGPU { 

		public:

			OpenGLBufferGPU(BufferType type, Buffer buf) : BufferGPU(type, buf), driverHandle(Buffer()), gpuHandle(0), fence(0) {}

			~OpenGLBufferGPU() { 
				if (initialized) {
					driverHandle.deconstruct();
					OpenGL::glBindBuffer(GL_ARRAY_BUFFER, gpuHandle);
					OpenGL::glUnmapBuffer(GL_ARRAY_BUFFER);
					OpenGL::glDeleteBuffers(1, &gpuHandle);
					OpenGL::glBindBuffer(GL_ARRAY_BUFFER, 0);
				}
			}

			bool init() override {

				GLenum flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

				OpenGL::glGenBuffers(1, &gpuHandle);
				OpenGL::glBindBuffer(GL_ARRAY_BUFFER, gpuHandle);

				if (gpuHandle == 0)
					return Log::throwError<OpenGLBufferGPU, 0x0>("Couldn't generate gpu handle!");

				OpenGL::glBufferStorage(GL_ARRAY_BUFFER, size(), &buf[0], flags);

				u8 *dat = (u8*) OpenGL::glMapBufferRange(GL_ARRAY_BUFFER, 0, size(), flags);
				
				if (dat == nullptr) {
					OpenGL::glUnmapBuffer(GL_ARRAY_BUFFER);
					OpenGL::glBindBuffer(GL_ARRAY_BUFFER, 0);
					return Log::throwError<OpenGLBufferGPU, 0x1>("Couldn't generate driver handle!");
				}

				OpenGL::glBindBuffer(GL_ARRAY_BUFFER, 0);

				driverHandle = { dat, size() };

				return initialized = true;
			}

			void lockBuffer() {
				if (fence)
					OpenGL::glDeleteSync(fence);
				fence = OpenGL::glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			}
			
			void unlockBuffer() {
				if (fence)
					while(true) {
						GLenum waitReturn = OpenGL::glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
						if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
							return;
					}
			}

			bool update() override {

				unlockBuffer();

				for (Buffer elem : updates) {
					u32 offset = (u32)(&elem[0] - &buf[0]);
					memcpy(&driverHandle[offset], &buf[offset], elem.size());
				}

				lockBuffer();
				return true;
			}

		private:

			Buffer driverHandle;
			GLuint gpuHandle;
			GLsync fence;

		};

	}

}