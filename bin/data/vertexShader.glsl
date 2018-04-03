#version 460 core

layout (location = 0) in vec3 inVertex;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;
//layout (location = 3) in vec3 inColour;
//layout (location = 4) in vec3 inTangent;
//layout (location = 5) in vec3 inBitangent;

out vec3 normal;
out vec2 texcoord;
//out vec3 colour;
//out vec3 tangent;
//out vec3 bitangent;

uniform mat4 model; // model to world
uniform mat4 view; // world to view
uniform mat4 projection; // view to projection

uniform mat4 modelView; // model to view (more precise)

void main(){
	gl_Position = projection * modelView * vec4(inVertex, 1);

	normal = inNormal;
	texcoord = inTexcoord;
	//colour = inColour;
	//tangent = inTangent;
	//bitangent = inBitangent;
};