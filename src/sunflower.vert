#version 330

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexUVs;

uniform sampler2D heightMap;
uniform float scaleValue;

void main()
{

	//Calculate sunflower height based on the heightmap
	vec3 colour = texture(heightMap, vertexUVs).rgb;

	//Calculate and appropriately adjust height
	float height = ((colour.r*255) * pow(2, 16)) + ((colour.g*255) * pow(2, 8)) + (colour.b*255);
	float reducedHeight = (height / 1000000) * scaleValue;

	//Adjust slightly so bottom of sunflower touches ground
	reducedHeight = reducedHeight + 0.04;

	gl_Position = vec4(vertexPosition.x, reducedHeight, vertexPosition.z, 1.0);
}