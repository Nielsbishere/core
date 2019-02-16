#pragma once
#include "utils/log.h"
#include "template/enum.h"

namespace oi {

	namespace gc {

		class NodeSystem;

		UEnum(NodeType,

			Empty = 0, Mesh = 1, Skeleton = 2, Bone = 3, Spline = 4, Billboard = 5, Emitter = 6, Skybox = 7,
			Line = 8, Camera = 9, Speaker = 10, Sprite = 11,

			Point_light = 12, Directional_light = 13, Spot_light = 14, Sun = 15, Area_light = 16, Cube_light = 17,

			Texture = 18, Model = 19, Sound = 20, Video = 21,

			Frustum = 22, Material = 23, View = 24, Viewport = 25, AABB = 26

		);

		struct BaseNode {

			u32 localId, globalId;
			NodeType type;
			NodeSystem *system;
			BaseNode *parent;
			size_t ctype;
			String name;

			template<typename T>
			BaseNode(NodeSystem *system, BaseNode *parent, String name, u32 localId, u32 globalId, NodeType type) :
				system(system), parent(parent), name(name), ctype(typeid(T).hash_code()),
				localId(u32_MAX), globalId(u32_MAX), type(type) {}

			template<typename T>
			bool isType() {
				return ctype == typeid(T).hash_code();
			}

			template<typename T>
			T &cast() {

				if (!isType<T>())
					Log::throwError<BaseNode, 0x0>("Couldn't cast type; it wasn't of the same node subtype");

				return *(T*)this;

			}

		};

	}

}