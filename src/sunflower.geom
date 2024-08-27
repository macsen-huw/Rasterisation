#version 330

layout (points) in;
layout (triangle_strip) out;
layout(max_vertices = 4) out;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPosition;

out vec2 textureCoords;

void main()
{
	//Loop through every sunflower
	for(int i = 0; i < gl_in.length(); i++)
	{
		vec3 position = gl_in[i].gl_Position.xyz;

		float scale = 0.1;

		//Get right and up vectors from the view matrix
		vec3 camRight = vec3(view[0][0], view[1][0], view[2][0]) * 0.5 * scale;
		vec3 camUp = vec3(view[0][1], view[1][1], view[2][1]) * 0.5 * scale;

		vec3 bottomLeft = position - camRight - camUp;
		vec3 bottomRight = position + camRight - camUp;
		vec3 topRight = position + camRight + camUp;
		vec3 topLeft = position - camRight + camUp;

		//Output vertices with texture coordinates
		textureCoords = vec2(0.0, 0.0);
		gl_Position = projection * view * vec4(bottomLeft, 1.0);
		EmitVertex();

		textureCoords = vec2(1.0, 0.0);
		gl_Position = projection * view * vec4(bottomRight, 1.0);
		EmitVertex();

		textureCoords = vec2(1.0, 1.0);
		gl_Position = projection * view * vec4(topRight, 1.0);
		EmitVertex();

		textureCoords = vec2(0.0, 1.0);
		gl_Position = projection * view * vec4(topLeft, 1.0);
		EmitVertex();


		/*
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
		*/




		EndPrimitive();
	}
	

}