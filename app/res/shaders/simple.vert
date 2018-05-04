#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform PerObject {

	mat4 m;
	mat4 mvp;
	
} obj;

layout(binding = 1) uniform PerExecution {

	mat4 p;
	mat4 v;
	
	vec3 ambient;
	float time;
	
} exc;

layout(location = 0) out vec3 fragColor;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = obj.mvp * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor * exc.ambient;
}