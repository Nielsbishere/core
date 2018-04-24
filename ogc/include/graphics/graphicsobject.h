#pragma once
#include <types/generic.h>

namespace oi {

	namespace gc {

		class Graphics;

		class GraphicsObject { 
		
			friend class Graphics;

		public:

			virtual ~GraphicsObject();

			size_t getHash() const;

		protected:

			Graphics *g = nullptr;
			i32 refCount = 0;

		private:

			size_t hash = (size_t) -1;


		};

	}
}