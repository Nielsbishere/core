#version 450 core
in layout(location = 0) vec3 pos;
in layout(location = 1) vec2 uv;
in layout(location = 2) vec3 norm;

out vec2 luv;
out vec3 wpos;
out vec3 cpos;

void main() {
	gl_Position = vec4(pos.xy, 0, 1);
	luv = vec2(uv.x, uv.y);
}