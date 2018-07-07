#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler samp;
layout(binding = 1) uniform texture2D tex;

layout(binding = 2) uniform PostProcessingSettings {

	float exposure;
	float gamma;
	vec2 padding;

} settings;

vec4 sample2D(sampler s, texture2D t, vec2 uv){
	return texture(sampler2D(t, s), uv);
}

void main() {
	
	vec3 col = sample2D(samp, tex, uv).rgb;
	
    //Exposure with gamma correction 
	
    col = pow(vec3(1) - exp(-col * settings.exposure), vec3(1.0 / settings.gamma));
	
    outColor = vec4(col, 1);
}