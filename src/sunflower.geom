#version 330

layout (points) in;
layout (triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 viewProjection;
uniform vec3 cameraPosition;

out vec2 textureCoords;

void main()
{
	//Loop through every sunflower
	for(int i = 0; i < gl_in.length(); i++)
	{
		vec3 position = gl_in[i].gl_Position.xyz;

		//Get the cross product between the upwards vector and the camera vector
		vec3 vecToCamera = normalize(cameraPosition - position) * 0.2;
		vec3 up = vec3(0.0, 1.0, 0.0) * 0.2;
		vec3 right = cross(vecToCamera, up);

		//We are looking directly towards the right vector
		//Deal with each corner individually, starting with bottom left
		gl_Position = viewProjection * vec4(position - (right * 0.5), 1.0);
		textureCoords = vec2(0.0,0.0);
		EmitVertex();

		//Top Left
		gl_Position = viewProjection * vec4((position + up) - (right * 0.5), 1.0);
		textureCoords = vec2(0.0, 1.0);
		EmitVertex();

		//Bottom Right
		gl_Position = viewProjection * vec4(position + (right * 0.5), 1.0);
		textureCoords = vec2(1.0, 0.0);
		EmitVertex();

		//Top Right
		gl_Position = viewProjection * vec4((position + up) + (right * 0.5), 1.0);
		textureCoords = vec2(1.0, 1.0);

		EmitVertex();

		EndPrimitive();
	}
	

}