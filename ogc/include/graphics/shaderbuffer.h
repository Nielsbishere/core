#pragma once

#include "gbuffer.h"
#include "shaderenums.h"
#include "graphicsresource.h"

namespace oi {

	namespace gc {

		struct ShaderBufferObject {

			ShaderBufferObject *parent;
			u32 offset, length, arraySize;
			String name;
			TextureFormat format;

			std::vector<ShaderBufferObject*> childs;

			ShaderBufferObject(ShaderBufferObject *parent, u32 offset, u32 length, u32 arraySize, String name, TextureFormat format);
			ShaderBufferObject();

			void addChild(ShaderBufferObject *obj);
			ShaderBufferObject *find(String name);

		};

		struct ShaderBufferInfo {

			ShaderRegisterType type;
			u32 size;
			bool allocate;

			ShaderBufferObject self;
			std::vector<ShaderBufferObject> elements;

			ShaderBufferInfo(ShaderRegisterType type, u32 size, u32 elements, bool allocate = true);
			ShaderBufferInfo();

			void addRoot(ShaderBufferObject *obj);
			u32 getRoots();

			ShaderBufferObject &operator[](u32 i);
			ShaderBufferObject &getRoot(u32 i);

			u32 lookup(ShaderBufferObject *elem);

			ShaderBufferInfo &operator=(const ShaderBufferInfo &info);
			ShaderBufferInfo(const ShaderBufferInfo &info);

		private:

			void copy(const ShaderBufferInfo &info);

		};

		class ShaderBufferVar {

		public:

			ShaderBufferVar(ShaderBufferObject &obj, Buffer buf, bool available);

			//Read and write?

		private:

			ShaderBufferObject &obj;
			Buffer buf;
			bool available;

		};

		class ShaderBuffer : public GraphicsResource {

			friend class Graphics;

		public:

			const ShaderBufferInfo getInfo();

			ShaderBufferVar get(String path);
			ShaderBufferVar get();

			u32 getElements();
			u32 getSize();

			GBuffer *getBuffer();
			void setBuffer(GBuffer *buf);	//If info.allocate is false, you have to set the buffer yourself

			void open();					//Call this if you start writing/reading
			void copy(Buffer buf);			//Copy a buffer
			void close();					//Call this if you end writing

			void set(Buffer buf);			//Same as GBuffer::set; open(), copy(), close()

		protected:

			ShaderBuffer(ShaderBufferInfo info);
			~ShaderBuffer();

			bool init();

		private:

			ShaderBufferInfo info;
			GBuffer *buffer = nullptr;
			Buffer current;

			bool isOpen = false;

		};

	}

}