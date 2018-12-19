#extension GL_ARB_shader_draw_parameters : require
#include <lighting.glsl>

struct PerObject {

	Matrix m;
	Matrix mvp;

	Vec3u padding;
	MaterialHandle material;

};

layout(std430, binding = 0) buffer Objects {

	PerObject arr[];
	
} obj;

layout(location = 0) in Vec3 inPosition;
layout(location = 1) in Vec2 inUv;
layout(location = 2) in Vec3 inNormal;

layout(location = 0) out Vec2 uv;
layout(location = 1) out Vec3 normal;
layout(location = 2) flat out MaterialHandle material;

out gl_PerVertex {
    Vec4 gl_Position;
};

void main() {

	PerObject obj = obj.arr[gl_InstanceIndex];

    gl_Position = obj.mvp * Vec4(inPosition, 1);

	uv = inUv;
	normal = normalize(obj.m * Vec4(normalize(inNormal), 0)).xyz;
	material = obj.material;

}