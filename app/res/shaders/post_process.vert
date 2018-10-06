#include <types.glsl>

layout(location = 0) out Vec2 uv;

layout(location = 0) in Vec2 inPos;

out gl_PerVertex {
    Vec4 gl_Position;
};

void main() {
    gl_Position = Vec4(inPos, 0, 1);
	uv = inPos * 0.5f + 0.5f;
}