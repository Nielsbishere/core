#version 450 core
in layout(location = 0) vec2 position;
in layout(location = 1) vec2 texCoord;

out vec2 uv;
out vec3 wpos;
out vec3 cpos;

void main() {
	gl_Position = vec4(position, 0, 1);
	uv = vec2(texCoord.x, texCoord.y);
}