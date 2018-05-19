#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

vec3 sinf(vec3 f){
	return sin(f * 3.1415926535) * 0.5 + 0.5;
}

void main() {
    outColor = vec4(sinf(fragColor) * vec3(uv, 1), 1);
}