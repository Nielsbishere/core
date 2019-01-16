#include <lighting.ogsl>

struct PerObject {

	Matrix m;
	Matrix mvp;

	Vec3u padding;
	MaterialHandle material;

};

ConstArray(0, objects, Objects, PerObject);

In(0, inPosition, Vec3);
In(1, inUv, Vec2);
In(2, inNormal, Vec3);

Out(0, uv, Vec2);
Out(1, normal, Vec3);
Out(2, material, MaterialHandle);

Vertex() {

	PerObject obj = objects[instanceId];

    vPosition = mul(obj.mvp, Vec4(inPosition, 1));

	uv = inUv;
	normal = normalize(mul(obj.m, Vec4(normalize(inNormal), 0))).xyz;
	material = obj.material;

}