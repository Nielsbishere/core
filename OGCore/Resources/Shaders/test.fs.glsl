#version 450 core
uniform sampler2D t;

in vec2 uv;

out layout(location=0) vec4 color;

void main(){
	vec3 col = texture(t, uv).rgb;
	color = vec4(col, 1);
}