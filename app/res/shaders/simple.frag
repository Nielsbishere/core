#include <types.glsl>

layout(location = 0) in Vec2 uv;
layout(location = 1) in Vec3 normal;
layout(location = 2) in flat u32 material;

layout(location = 0) out Vec2 guv;
layout(location = 1) out Vec2 gnormal;
layout(location = 2) out u32 gmaterial;

layout(early_fragment_tests) in;

//Spheremap transform
Vec2 encodeNormal(Vec3 n) {
    float p = sqrt(n.z * 8 + 8);
    return n.xy / p + 0.5;
}

//Output to G-Buffer
void main() {
	guv = uv;
	gnormal = encodeNormal(normal);
	gmaterial = material + 1;
}