#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_ocs;
layout(location = 1) in vec2 vertexUV;

uniform sampler2D heightMap;
uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform float numPoints;

// Output data ; will be interpolated for each fragment.
out vec2 UVcoords;
out vec3 vertexNormal;
out vec3 lightDirection;
out vec3 cameraPosition;
out mat4 modelViewMatrix;
out vec3 fragPos;
out float pointHeight;
out mat3 TBN;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 modelView;
uniform float scaleValue;

void main(){
	//Get the colour values of the texture (value is between 0-1)
	vec3 colour = texture(heightMap, vertexUV).rgb;

	//Calculate height (r*2^16 + g*2^8 + b)
	//We also multiply by 255 to get the colour in the range (0-255)
	float height = ((colour.r*255) * pow(2, 16)) + ((colour.g*255) * pow(2, 8)) + (colour.b*255);

	//Set the height to be in front of the camera
	//The height depends on the scale value
	float reducedHeight = (height / 1000000) * scaleValue;
	pointHeight = reducedHeight;

	//Add the height to the existing vertexPosition vector
	vec3 heightVector = {0.0f, reducedHeight, 0.0f};
	vec3 updatedVector = vertexPosition_ocs + heightVector;
	
	//UV coordinates are in the range (0,1)
	float offset = 1.0 / numPoints;

	//Get the colour coordinates of the texel's neighbours
	vec3 topLeft = texture(heightMap, vertexUV + vec2(-offset,offset)).rgb;
	vec3 centerLeft = texture(heightMap, vertexUV + vec2(-offset,0)).rgb;
	vec3 bottomLeft = texture(heightMap, vertexUV + vec2(-offset, -offset)).rgb;

	vec3 topRight = texture(heightMap, vertexUV + vec2(offset,offset)).rgb;
	vec3 centerRight = texture(heightMap, vertexUV + vec2(offset,0)).rgb;
	vec3 bottomRight = texture(heightMap, vertexUV + vec2(offset, -offset)).rgb;

	vec3 down = texture(heightMap, vertexUV + vec2(0,-offset)).rgb * 2;
	vec3 up = texture(heightMap, vertexUV + vec2(0,offset)).rgb * 2;
	
	//Calculate height for each neighbour
	float bottomLeftHeight = ((bottomLeft.r*255) * pow(2, 16)) + ((bottomLeft.g*255) * pow(2, 8)) + (bottomLeft.b*255);
	float centerLeftHeight = ((centerLeft.r*255) * pow(2, 16)) + ((centerLeft.g*255) * pow(2, 8)) + (centerLeft.b*255);
	float topLeftHeight = ((topLeft.r*255) * pow(2, 16)) + ((topLeft.g*255) * pow(2, 8)) + (topLeft.b*255);

	float bottomRightHeight = ((bottomRight.r*255) * pow(2, 16)) + ((bottomRight.g*255) * pow(2, 8)) + (bottomRight.b*255);
	float centerRightHeight = ((centerRight.r*255) * pow(2, 16)) + ((centerRight.g*255) * pow(2, 8)) + (centerRight.b*255);
	float topRightHeight = ((topRight.r*255) * pow(2, 16)) + ((topRight.g*255) * pow(2, 8)) + (topRight.b*255);

	float downHeight = ((down.r*255) * pow(2, 16)) + ((down.g*255) * pow(2, 8)) + (down.b*255);
	float upHeight = ((up.r*255) * pow(2, 16)) + ((up.g*255) * pow(2, 8)) + (up.b*255);
	
	//Calculate difference
	float xNormal = (topLeftHeight - topRightHeight) + 2*(centerLeftHeight - centerRightHeight) + (bottomLeftHeight - bottomRightHeight); 
	float yNormal = 35000;
	float zNormal = (topLeftHeight - bottomLeftHeight) + 2*(upHeight - downHeight) + (topRightHeight - bottomRightHeight);

	vertexNormal = normalize(vec3(xNormal, yNormal, zNormal));

	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(updatedVector,1);
	
	//Send vertexPosition_ocs to the fragment shader
	fragPos = updatedVector;
	
	//Send vertex normal to fragment shader
	vertexNormal = vertexNormal;

	//Setup TBN matrix
	vec3 tangent = vec3(1,0,0);
	vec3 bitangent = vec3(0,0,1);

	//Perform Gram-Schmidt to ensure orthonormality
	tangent = normalize(tangent - dot(tangent, vertexNormal) * vertexNormal);
	bitangent = normalize((bitangent - dot(bitangent, vertexNormal) * vertexNormal) - (bitangent - dot(bitangent, tangent) * tangent));

	mat3 tbn = mat3(tangent, bitangent, vertexNormal);
	TBN = tbn;

	// UV of the vertex. No special space for this one.
	UVcoords = vertexUV;

	lightDirection = lightPos;
	cameraPosition = cameraPos;
	modelViewMatrix = modelView;
}

