#version 450
#extension GL_ARB_separate_shader_objects : enable

//Lighting

struct Light {

	vec3 pos;
	float intensity;

	vec3 col;
	float radius;

	vec3 dir;
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

LightResult calculateDirectional(Light dli, vec3 pos, vec3 normal, vec3 cdir, float power){

	LightResult res;
	res.diffuse = calculateDiffuse(dli, -dli.dir, normal);
	res.specular = calculateSpecular(dli, -dli.dir, normal, cdir, power);

	return res;

}

float attenuation(float r, float d, float smoothness) {
	return 1.0f - smoothstep(r * smoothness, r, d);
}

float attenuation(float r, float d) {
	return attenuation(r, d, 0); 
}

LightResult calculatePoint(Light pli, vec3 pos, vec3 normal, vec3 cdir, float power){

	vec3 dir = pli.pos - pos;
	float dist = length(dir);
	dir = dir / dist;

	float a = attenuation(pli.radius, dist);

	LightResult res;
	res.diffuse = calculateDiffuse(pli, dir, normal) * a;
	res.specular = calculateSpecular(pli, dir, normal, cdir, power) * a;

	return res;

}

float calculateSpotIntensity(Light sli, vec3 dir){

	float minCos = cos(radians(sli.angle));
	float maxCos = mix(minCos, 1, 0.5f);
	float cosAngle = dot(sli.dir, -dir);
	return smoothstep(minCos, maxCos, cosAngle);

}

LightResult calculateSpot(Light sli, vec3 pos, vec3 normal, vec3 cdir, float power){

	vec3 dir = sli.pos - pos;
	float dist = length(dir);
	dir = dir / dist;

	float a = attenuation(sli.radius, dist) * calculateSpotIntensity(sli, dir);

	LightResult res;
	res.diffuse = calculateDiffuse(sli, dir, normal) * a;
	res.specular = calculateSpecular(sli, dir, normal, cdir, power) * a;

	return res;

}

vec3 calculateLighting(LightResult res, vec3 col, vec3 ambient){
	return (res.diffuse + res.specular + ambient) * col;
}

//Shader

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;
layout(location = 3) in flat uint diffuse;

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
layout(binding = 4) uniform texture2D tex[2];

layout(std430, binding = 5) buffer Lights {

	Light directional, point, spot;

} lights;

vec4 sample2D(sampler s, uint index, vec2 uv){
	return texture(sampler2D(tex[index], s), uv);
}

void main() {

	vec3 cpos = normalize(cam.position - pos);

	LightResult lr = calculateDirectional(lights.directional, pos, normal, cpos, exc.power);

	LightResult plr = calculatePoint(lights.point, pos, normal, cpos, exc.power);
	lr.diffuse += plr.diffuse;
	lr.specular += plr.specular;

	LightResult slr = calculateSpot(lights.spot, pos, normal, cpos, exc.power);
	lr.diffuse += slr.diffuse;
	lr.specular += slr.specular;

    outColor = vec4(calculateLighting(lr, sample2D(samp, diffuse, uv).rgb, exc.ambient), 1);
}