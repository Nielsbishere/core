#version 450
#extension GL_ARB_separate_shader_objects : enable

//Lighting

struct Light {

	vec3 pos;
	float intensity;

	vec3 col;
	float radius;

	vec3 dir;
	float length;

	vec3 padding;
	float angle;

};

struct LightResult {

	vec3 diffuse;
	float p0;

	vec3 specular;
	float p1;

};

vec3 calculateDiffuse(Light li, vec3 dir, vec3 normal){
	return li.col * max(0, dot(normal, dir)) * li.intensity;
}

vec3 calculateSpecular(Light li, vec3 dir, vec3 normal, vec3 cdir, float power){
	return li.col * pow(max(0, dot(normalize(reflect(-dir, normal)), cdir)), power) * li.intensity;
}

LightResult calculateDirection(Light dli, vec3 pos, vec3 normal, vec3 cdir, float power){

	LightResult res;
	res.diffuse = calculateDiffuse(dli, -dli.dir, normal);
	res.specular = calculateSpecular(dli, -dli.dir, normal, cdir, power);

	return res;
}

vec3 calculateLighting(LightResult res, vec3 col, vec3 ambient){
	return (res.diffuse + res.specular + ambient) * col;
}

//Shader

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform PerExecution {

	vec3 ambient;
	float time;

	vec3 padding;
	float power;
	
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
	
	uvec2 resolution;
	float near;
	float far;

} cam;

layout(binding = 3) uniform sampler samp;
layout(binding = 4) uniform texture2D tex;

layout(std430, binding = 5) buffer DirectionalLights {

	Light light;

} directional;

vec4 sample2D(sampler s, texture2D t, vec2 uv){
	return texture(sampler2D(t, s), uv);
}

void main() {
	LightResult lr = calculateDirection(directional.light, pos, normal, normalize(cam.position - pos), exc.power);
    outColor = vec4(calculateLighting(lr, sample2D(samp, tex, uv).rgb, exc.ambient), 1);
}