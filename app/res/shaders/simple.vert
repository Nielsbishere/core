#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shader_draw_parameters : require

struct PerObject {

	mat4 m;
	mat4 mvp;

};

layout(std430, binding = 0) buffer Objects {

	PerObject arr[1];
	
} obj;

layout(location = 0) out vec3 pos;
layout(location = 1) out vec2 uv;
layout(location = 2) out vec3 normal;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUv;
layout(location = 2) in vec3 inNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {

	PerObject obj = obj.arr[gl_InstanceIndex + gl_BaseVertexARB];

    gl_Position = obj.mvp * vec4(inPosition, 1.0);

	pos = (obj.m * vec4(inPosition, 1.0)).xyz;
	uv = inUv;
	normal = normalize(obj.m * vec4(normalize(inNormal), 0.0)).xyz;

}