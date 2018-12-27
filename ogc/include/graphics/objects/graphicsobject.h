#pragma once
#include <typeinfo>
#include "types/generic.h"
#include "types/string.h"

namespace oi {

	class BlockAllocator;

	namespace gc {

		class Graphics;
		class GraphicsResource;
		class TextureObject;

		class GraphicsObject { 
		
			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			virtual ~GraphicsObject();

			size_t getHash() const;

			String getName() const;
			String getTypeName() const;

			i32 getRefCount() const;

			template<typename T>
			bool isType() {

				static_assert(!std::is_same<T, GraphicsObject>::value && !std::is_same<T, GraphicsResource>::value && !std::is_same<T, TextureObject>::value,
					"GraphicsObject<T>::isType is only available for exact types, GraphicsObjects, GraphicsResources and TextureObjects cannot be used");

				return hash == typeid(T).hash_code();
			}

			template<typename T>
			T *cast() {

				if (isType<T>)
					return (T*)this;

				return nullptr;
			}

		protected:

			Graphics *g = nullptr;
			i32 refCount = 0;

			template<typename T>
			void setHash() {

				hash = typeid(T).hash_code();
				
				auto it = names.find(hash);

				if (it == names.end())
					names[hash] = typeid(T).name();

			}

		private:

			size_t hash = (size_t) -1;
			String name;

			static std::unordered_map<size_t, String> names;


		};

	}
}