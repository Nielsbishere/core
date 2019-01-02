#pragma once
#include "graphicsobject.h"

namespace oi {

	namespace gc {

		template<typename T>
		class TGraphicsObjectRef {

		private:

			T *ptr;

		public:

			T *operator->() { return ptr; }
			operator T*() { return ptr; }
			T *get() { return ptr; }

			TGraphicsObjectRef(T *ptr): ptr(ptr) {

				static_assert(std::is_base_of<GraphicsObject, T>::value, "TGraphicsObjectRef<T> only works on GraphicsObjects");

				if(ptr != nullptr)
					ptr->g->use(ptr);

			}

			TGraphicsObjectRef(const TGraphicsObjectRef &other) : TGraphicsObjectRef(other.ptr) { }

			template<typename T2>
			TGraphicsObjectRef(Graphics &g, String name, T2 tinfo) {

				static_assert(std::is_same<typename T2::ResourceType, T>::value, "TGraphicsObject<T>(T2&) only works on TInfo structs");

				ptr = g.create(name, tinfo);

				if(ptr != nullptr)
					g.use(ptr);

			}

			TGraphicsObjectRef() : ptr(nullptr) { }

			~TGraphicsObjectRef() {
				if (ptr != nullptr)
					ptr->g->destroy(ptr);
			}

			TGraphicsObjectRef &operator=(const TGraphicsObjectRef &other) {
				ptr = other.ptr;

				if (ptr != nullptr)
					ptr->g->use(ptr);

				return *this;
			}

		};

	}

}