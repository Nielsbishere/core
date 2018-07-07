#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 uv;

layout(location = 0) in vec2 inPos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = vec4(inPos, 0, 1);
	uv = inPos * 0.5 + 0.5;
}