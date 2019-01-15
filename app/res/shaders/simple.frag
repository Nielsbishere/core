In(0, uv, Vec2);
In(1, normal, Vec3);
ConstIn(2, material, u32);

Out(0, guv, Vec2);
Out(1, gnormal, Vec2);
Out(2, gmaterial, u32);

//Spheremap transform
Vec2 encodeNormal(Vec3 n) {
    f32 p = sqrt(n.z * 8 + 8);
    return n.xy / p + 0.5;
}

//Output to G-Buffer
Fragment() {
	guv = uv;
	gnormal = encodeNormal(normal);
	gmaterial = material + 1;
}