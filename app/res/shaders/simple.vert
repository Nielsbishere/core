#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_draw_parameters : require

struct PerObject {

	mat4 m;
	mat4 mvp;

};

layout(std430, binding = 0) buffer Objects {

	PerObject arr[64];
	
} obj;

layout(binding = 2) uniform Camera {

	mat4 p;

	mat4 v;

	vec3 position;
	float fov;

	vec3 up;
	float aspect;

	vec3 forward;
	float padding;
	
	float near;
	float far;
	uvec2 resolution;

} cam;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = obj.arr[gl_InstanceIndex + gl_BaseVertexARB].mvp * vec4(inPosition, 1.0);
	uv = inUv;
	normal = inNormal;
}