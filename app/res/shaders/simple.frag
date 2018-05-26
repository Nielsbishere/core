#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler samp;
layout(binding = 3) uniform texture2D tex;

vec4 sample2D(sampler s, texture2D t, vec2 uv){
	return texture(sampler2D(t, s), uv);
}

void main() {
    outColor = vec4(fragColor * sample2D(samp, tex, uv).rgb, 1);
}