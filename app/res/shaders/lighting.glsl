#include <types.glsl>

//Material
//112 bytes; ~9362 material (structs) per MiB
struct MaterialStruct {

	Vec3 diffuse;
	MaterialHandle id;

	Vec3 ambient;
	f32 shininess;

	Vec3 emissive;
	f32 shininessExponent;

	Vec3 specular;
	f32 roughness;

	f32 metallic;
	f32 transparency;
	f32 clearcoat;
	f32 clearcoatGloss;

	f32 reflectiveness;
	f32 sheen;
	TextureHandle t_diffuse;			//sRGB8 (3 Bpp)
	TextureHandle t_opacitySpecular;	//RG8 (2 Bpp)

	TextureHandle t_emissive;			//RGB8 (3 Bpp)
	TextureHandle t_rahm;				//RGBA8 (4 Bpp); Roughness, ao, height, metallic
	TextureHandle t_normal;				//RGB8s (3 Bpp)
	u32 p0;
	
};

//Lighting

struct Light {

	Vec3 pos;
	f32 intensity;

	Vec3 col;
	f32 radius;

	Vec3 dir;
	f32 angle;

};

struct LightResult {

	Vec3 diffuse;
	f32 p0;

	Vec3 specular;
	f32 p1;

};

Vec3 calculateDiffuse(Light li, Vec3 dir, Vec3 normal){
	return li.col * max(0, dot(normal, dir)) * li.intensity;
}

Vec3 calculateSpecular(Light li, Vec3 dir, Vec3 normal, Vec3 cdir, f32 power){
	return li.col * pow(max(0, dot(normalize(reflect(-dir, normal)), cdir)), power) * li.intensity;
}

LightResult calculateDirectional(Light dli, Vec3 pos, Vec3 normal, Vec3 cdir, f32 power){

	LightResult res;
	res.diffuse = calculateDiffuse(dli, -dli.dir, normal);
	res.specular = calculateSpecular(dli, -dli.dir, normal, cdir, power);

	return res;

}

f32 attenuation(f32 r, f32 d, f32 smoothness) {
	return 1.0f - smoothstep(r * smoothness, r, d);
}

f32 attenuation(f32 r, f32 d) {
	return attenuation(r, d, 0); 
}

LightResult calculatePoint(Light pli, Vec3 pos, Vec3 normal, Vec3 cdir, f32 power){

	Vec3 dir = pli.pos - pos;
	f32 dist = length(dir);
	dir = dir / dist;

	f32 a = attenuation(pli.radius, dist);

	LightResult res;
	res.diffuse = calculateDiffuse(pli, dir, normal) * a;
	res.specular = calculateSpecular(pli, dir, normal, cdir, power) * a;

	return res;

}

f32 calculateSpotIntensity(Light sli, Vec3 dir){

	f32 minCos = cos(radians(sli.angle));
	f32 maxCos = mix(minCos, 1, 0.5f);
	f32 cosAngle = dot(sli.dir, -dir);
	return smoothstep(minCos, maxCos, cosAngle);

}

LightResult calculateSpot(Light sli, Vec3 pos, Vec3 normal, Vec3 cdir, f32 power){

	Vec3 dir = sli.pos - pos;
	f32 dist = length(dir);
	dir = dir / dist;

	f32 a = attenuation(sli.radius, dist) * calculateSpotIntensity(sli, dir);

	LightResult res;
	res.diffuse = calculateDiffuse(sli, dir, normal) * a;
	res.specular = calculateSpecular(sli, dir, normal, cdir, power) * a;

	return res;

}

Vec3 calculateLighting(LightResult res, Vec3 col, Vec3 ambient, MaterialStruct mat){
	return (res.diffuse * mat.diffuse + res.specular * mat.specular * mat.shininess + ambient * mat.ambient) * col;
}

//Camera
struct CameraStruct {

	Matrix p;

	Matrix v;

	Vec3 position;
	f32 fov;

	Vec3 up;
	f32 aspect;

	Vec3 forward;
	f32 padding;
	
	Vec2u resolution;
	f32 near;
	f32 far;

};