#version 420 core

// Input
in vec2 UVcoords;
in vec3 vertexNormal;
in vec3 lightDirection;
in vec3 cameraPosition;
in vec3 fragPos;
in mat3 TBN;
in float pointHeight;

in mat4 modelViewMatrix;
// Output
out vec3 color;
//Uniforms
layout (binding=0) uniform sampler2D DiffuseTextureSampler;
layout (binding=2) uniform sampler2D ShininessTextureSampler;
layout (binding=3) uniform sampler2D rockNormals;
layout (binding=4) uniform sampler2D snowDiffuseSampler;
layout (binding=5) uniform sampler2D snowShininessSampler;
layout (binding=6) uniform sampler2D snowNormals;
layout (binding=7) uniform sampler2D grassDiffuseSampler;
layout (binding=8) uniform sampler2D grassShininessSampler;
layout (binding=9) uniform sampler2D grassNormals;

void main(){
	//Tiling - multiply UV coords by a scale factor
	vec2 UV = vec2(UVcoords.x * 2, UVcoords.y * 2);

	//Get the diffuse textures of the surfaces
	vec3 grassTexture = texture(grassDiffuseSampler, vec2(UV.x, UV.y)).rgb;
	vec3 rockTexture = texture(DiffuseTextureSampler, vec2(UV.x, UV.y)).rgb;
	vec3 snowTexture = texture(snowDiffuseSampler, vec2(UV.x, UV.y)).rgb;

	//Get the roughness of each surface
	float grassRoughness = texture(grassShininessSampler, vec2(UV.x, UV.y)).r;
	float rockRoughness = texture(ShininessTextureSampler, vec2(UV.x, UV.y)).r;
	float snowRoughness = texture(snowShininessSampler, vec2(UV.x, UV.y)).r;

	//Calculate the shininess of each surface given the roughness	
	float grassShininess = clamp((2/(pow(grassRoughness,4)+1e-2))-2,0,500.0f);
	float rockShininess = clamp((2/(pow(rockRoughness,4)+1e-2))-2,0,500.0f);
	float snowShininess = clamp((2/(pow(snowRoughness,4)+1e-2))-2,0,500.0f);

	//Get the normals of each surface
	vec3 rockNormal = texture(rockNormals, vec2(UV.x, UV.y)).rgb;
	vec3 grassNormal = texture(grassNormals, vec2(UV.x, UV.y)).rgb;
	vec3 snowNormal = texture(snowNormals, vec2(UV.x, UV.y)).rgb;

	//Interpolate the textures based on the height
	float grassThreshold = 0.0;
	float rockThreshold = 1.0;
	float snowThreshold = 2.5;

	vec3 finalDiffuse;
	vec3 finalNormal;
	float finalShininess;

	//Interpolate to calculate how much of each texture we want at the point
	//Works similar to barycentric interpolation with distOtoQR / distPtoQR
	float RocktoGrassInterpolate = clamp( (pointHeight - grassThreshold) / (rockThreshold - grassThreshold), 0.0, 1.0);
	float SnowtoRockInterpolate = clamp( (pointHeight - rockThreshold) / (snowThreshold - rockThreshold), 0.0, 1.0);

	//Interpolate the shininess values
	finalShininess = mix(grassShininess, rockShininess, RocktoGrassInterpolate);
	finalShininess = mix(finalShininess, snowShininess, SnowtoRockInterpolate);

	//Interpolate normal mapping
	finalNormal = mix(grassNormal, rockNormal, RocktoGrassInterpolate);
	finalNormal = mix(finalNormal, snowNormal, SnowtoRockInterpolate);

	//Interpolate diffuse colours
	finalDiffuse = mix(grassTexture, rockTexture, RocktoGrassInterpolate);
	finalDiffuse = mix(finalDiffuse, snowTexture, SnowtoRockInterpolate);

	//Transform normals coordinate system from [0,1] to [-1,1]
	vec3 transformedNormals = (finalNormal * 2) - 1;

	//Apply TBN matrix
	transformedNormals = normalize(TBN * transformedNormals);

	//Calculate Light
	//Initialising light	
	vec3 lightColour = {1, 1, 1}; //Set light colour to white
	vec3 lightDir = normalize(lightDirection); //Ensure the light direction is normalised

	//The specular colour is constant
	vec3 specularColour = {0.1, 0.1, 0.1};
	
	//Ambient - weaker version of regular colour
	vec3 ambient = 0.2 * finalDiffuse * lightColour;
	
	//Diffuse
	float diffuseStrength = max(dot(transformedNormals, lightDirection), 0.0);
	vec3 diffuse = diffuseStrength * finalDiffuse * lightColour;

	//Before calculating specular, we must initialise some values
	vec3 fragPosVCS = vec3(modelViewMatrix * vec4(fragPos, 1)); //Convert fragPos from OCS to VCS
	vec3 cameraDirection = normalize(cameraPosition - fragPosVCS); //Get the direction of the eye (camera)
	vec3 bisector = normalize(lightDirection + cameraDirection);

	//Specular
	float specularStrength = pow(max(dot(transformedNormals, bisector), 0.0), finalShininess);
	vec3 specular = specularStrength * specularColour * lightColour;
	
	//color = vec3(abs(vertexNormal.x),abs(vertexNormal.y),abs(vertexNormal.z));
	color = ambient + diffuse + specular;

}
