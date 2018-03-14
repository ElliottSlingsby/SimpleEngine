#version 460 core

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;

out vec3 normal;
out vec2 texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
	mat4 matrix = projection * view * model;
	gl_Position = matrix * vec4(inVertex, 1);

	normal = inNormal;
	texcoord = inTexcoord;
};