#include <types.glsl>

layout(location = 0) in Vec2 uv;

layout(location = 0) out Vec4 outColor;

layout(binding = 0) uniform sampler samp;
layout(binding = 1) uniform texture2D tex;

layout(binding = 2) uniform PostProcessingSettings {

	Vec2 padding;
	f32 exposure;
	f32 gamma;

} settings;

Vec4 sample2D(sampler s, texture2D t, Vec2 uv){
	return texture(sampler2D(t, s), uv);
}

void main() {
	
	Vec3 col = sample2D(samp, tex, uv).rgb;
	
    //Exposure with gamma correction 
	
    col = pow(Vec3(1) - exp(-col * settings.exposure), Vec3(1.0f / settings.gamma));
	
    outColor = Vec4(col, 1);
}