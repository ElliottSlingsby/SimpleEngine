#version 460 core

in vec3 normal;
in vec2 texcoord;
//out vec3 colour;
//out vec3 tangent;
//out vec3 bitangent;

layout (location = 0) out vec4 fragColour;

uniform sampler2D texture;

void main(){
	fragColour = texture2D(texture, texcoord).rgba;
};