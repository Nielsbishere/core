#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform PerExecution {

	vec3 ambient;
	float time;
	
} exc;

layout(binding = 3) uniform sampler samp;
layout(binding = 4) uniform texture2D tex;

vec4 sample2D(sampler s, texture2D t, vec2 uv){
	return texture(sampler2D(t, s), uv);
}

void main() {
	vec3 col = sample2D(samp, tex, uv).rgb;
	col.r = pow(col.r, 2);
    outColor = vec4(col, 1);
}