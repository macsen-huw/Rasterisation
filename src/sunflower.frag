#version 330

uniform sampler2D sampler;

in vec2 textureCoords;
out vec4 color;

void main()
{
	
	vec4 colour = texture(sampler, textureCoords);

	//Discard the transparent background
	if(colour.a < 1.0)
		discard;
	
	color = colour;
}