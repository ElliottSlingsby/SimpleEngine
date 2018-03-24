#version 460 core

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;
//layout (location = 4) in vec3 inTangent;
//layout (location = 5) in vec3 inBitangent;

out vec3 normal;
out vec2 texcoord;

uniform mat4 model; // model to world
uniform mat4 view; // world to view
uniform mat4 projection; // view to projection

uniform mat4 modelView; // model to view

void main(){
	mat4 matrix = projection * modelView;
	gl_Position = matrix * vec4(inVertex, 1);

	normal = inNormal;
	texcoord = inTexcoord;
};