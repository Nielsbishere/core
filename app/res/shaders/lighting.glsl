#include <types.glsl>

//Handle to a texture
#define TextureHandle uint

//Handle to a material
#define MaterialHandle uint

//Material
//128 bytes; ~8192 material (structs) per MiB
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
	TextureHandle t_opacity;			//R8 (1 Bpp)
	
	TextureHandle t_emissive;			//RGB16 (6 Bpp)
	TextureHandle t_roughness;			//R8 (1 Bpp)
	TextureHandle t_ao;					//R8 (1 Bpp)
	TextureHandle t_height;				//R8 (1 Bpp)

	TextureHandle t_metallic;			//R8 (1 Bpp)
	TextureHandle t_normal;				//RGB8s (3 Bpp)
	TextureHandle t_specular;			//R8 (1 Bpp)
	u32 p1;
	
};

//Lighting

//A handle to a light
#define LightHandle uint

//Point, spot or directional light; 48 bytes
struct Light {

	Vec3 pos;
	f32 intensity;

	Vec3 col;
	f32 radius;

	Vec3 dir;
	f32 angle;

};

//Result of lighting
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

Vec3 reconstructPosition(Vec2 uv, f32 depth, Matrix vpInv) {
	Vec4 clipPos = Vec4(uv.x * 2 - 1, (1 - uv.y) * 2 - 1, depth, 1);
	Vec4 worldPos = vpInv * clipPos;
	worldPos /= worldPos.w;
	return worldPos.xyz;
}

//Handle to a camera (0 - 127) (MAX_CAMERAS)
#define CameraHandle uint

//Camera (positional / orientation data)
//length: 7x16 = 112 bytes
struct Camera {

	Matrix v;			//View matrix

	Vec3 position;		//Camera 'eye'
	f32 p0;

	Vec3 up;			//Normal / camera up direction
	f32 p1;

	Vec4 forward;		//Forward / camera direction or center (if .w == 1)

};

//Handle to a camera frustum (0 - 127) (MAX_CAMERA_FRUSTA)
#define CameraFrustumHandle uint

//CameraFrustum (projection data)
//length: 6x16 = 96
struct CameraFrustum {

	Matrix p;			//Projection matrix

	f32 near;			//Near clipping plane (depth = 0)
	f32 far;			//Far clipping plane (depth = 1)
	f32 aspect;			//Aspect ratio (resolution.x / resolution.y)
	f32 fov;			//FOV in degrees

	Vec2u resolution;	//Resolution (w, h) in pixels
	u32 p0;
	u32 p1;

};

//Handle to a view (0 - 255) (MAX_VIEWS)
#define ViewHandle uint

//View (projection and orientation data)
//It only includes handles to cameras and viewports
//cameras[camera] is the view's camera
//frusta[cameraFrustum] is the view's frustum
//views[view] is the view
//144 bytes per view (camera and camera frustum data is re-used)
struct View {

	CameraHandle camera;
	CameraFrustumHandle cameraFrustum;
	uint p0, p1;

	Matrix vp;

	Matrix vpInv;

};

#define MAX_CAMERAS 128
#define MAX_CAMERA_FRUSTA 128
#define MAX_VIEWS 256

//ViewData (contains all cameras, frusta and viewports)
//128 * (112 + 96) + 256 * 80 = 128 * 208 + 256 * 144 = 62 KiB = 64 KiB (min UBO max length) - 2 KiB
struct ViewBuffer {

	Camera cameras[MAX_CAMERAS];
	CameraFrustum frusta[MAX_CAMERA_FRUSTA];
	View views[MAX_VIEWS];

};