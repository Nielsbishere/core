#pragma once
#include "nodesystem.h"

namespace oi {

	class BlockAllocator;

	namespace gc {

		template<typename T>
		class TNode : GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			NodeSystem *getSystem() const { return info->system; }
			BaseNode *getParent() const { return info->parent; }
			String getName() const { return info->name; }
			u32 getLocalId() const { return info->localId; }
			u32 getGlobalId() const { return info->localId; }

			const T *getNode() const { return info; }

		protected:

			TNode(T initInfo) {

				static_assert(std::is_base_of<BaseNode, T>::value, "TNode<T> can only be used on classes that inherrit from BaseNode"); 

				info = g->alloc(info, initInfo);

			}

			~TNode() { 
				getSystem()->remove((BaseNode*)info); 
				g->dealloc(info); 
			}

			bool init() { 
				return getSystem()->init((BaseNode*)info); 
			}

		private:

			T *info;

		};

	}

}