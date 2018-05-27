#version 450 core
#extension GL_ARB_separate_shader_objects : enable

struct PerObject {

	mat4 m;
	mat4 mvp;

};

layout(std430, binding = 0) buffer Objects {

	PerObject arr[65536];
	
} obj;

layout(binding = 1) uniform PerExecution {

	vec3 ambient;
	float time;
	
} exc;

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

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUv;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = obj.arr[gl_InstanceIndex].mvp * vec4(inPosition, 1.0);
    fragColor = inColor * exc.ambient;
	uv = inUv;
}