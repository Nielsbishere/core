In(0, inPos, Vec2);
Out(0, uv, Vec2);

Vertex() {
    vPosition = Vec4(inPos, 0, 1);
	uv = inPos * 0.5f + 0.5f;
}