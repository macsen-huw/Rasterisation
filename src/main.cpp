#include <iostream>
using namespace std;

#include <GL/glew.h>
#include <GLFW/glfw3.h>


//Including GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <vector>
#include <limits>

#include "common/utils.hpp"
#include "common/controls.hpp" //Calculates camera, inputs and matrices

//Include the stb_image library to read external textures (not bmp)
#define STB_IMAGE_IMPLEMENTATION
#include "external/stb_image.h"

//Including file reading
#include <fstream>
#include <sstream>

//Variables
GLFWwindow* window;
static const int window_width = 1920;
static const int window_height = 1080;

static const int n_points = 200; //There must be a minimum of 2 points
static const float m_scale = 5.0;
float scaleValue = 1.0;

//VAO and Buffers Needed
GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
GLuint elementbuffer;

//Additional VAO and Buffers needed (for the advanced tasks)
GLuint skyboxVertexArray;
GLuint skyboxBuffer;
GLuint sunflowerVertexArray;
GLuint sunflowerBuffer;

//Number of indices in the plane
unsigned int nIndices;

//Store the rock textures
GLuint rockDiffuseID;
GLuint rockShininessID;
GLuint rockNormalsID;

//Store the snow textures
GLuint snowDiffuseID;
GLuint snowShininessID;
GLuint snowNormalsID;

//Store the grass textures
GLuint grassDiffuseID;
GLuint grassShininessID;
GLuint grassNormalsID;

//Height map
GLuint heightMapID;

//Store the program
GLuint programID;

//Additional render passes
GLuint skyboxID;
GLuint sunflowerID;

//Store the skybox textures
GLuint skyboxTextureID;

//Store the sunflower textures
GLuint sunflowerTextureID;

//Store the vertices of the skybox cube
const vector<vec3> skyboxVerts = {
	//Right face (2 CCW triangles, 6 vertices)
	{-1.0f,  1.0f, -1.0f},
	{-1.0f, -1.0f, -1.0f},
	{1.0f, -1.0f, -1.0f},
	{1.0f, -1.0f, -1.0f},
	{1.0f,  1.0f, -1.0f},
	{-1.0f,  1.0f, -1.0f},

	//Left face
	{-1.0f, -1.0f,  1.0f },
	{-1.0f, -1.0f, -1.0f },
	{-1.0f,  1.0f, -1.0f},
	{-1.0f,  1.0f, -1.0f },
	{-1.0f,  1.0f,  1.0f},
	{-1.0f, -1.0f,  1.0f},

	//Top face
	{1.0f, -1.0f, -1.0f},
	{1.0f, -1.0f,  1.0f},
	{1.0f,  1.0f,  1.0f},
	{1.0f,  1.0f,  1.0f},
	{1.0f,  1.0f, -1.0f},
	{1.0f, -1.0f, -1.0f},

	//Bottom face
	{-1.0f, -1.0f,  1.0f},
	{-1.0f,  1.0f,  1.0f},
	{1.0f,  1.0f,  1.0f},
	{1.0f,  1.0f,  1.0f},
	{1.0f, -1.0f,  1.0f},
	{-1.0f, -1.0f,  1.0f},

	//Front face
	{-1.0f,  1.0f, -1.0f },
	{ 1.0f,  1.0f, -1.0f},
	{ 1.0f,  1.0f,  1.0f},
	{ 1.0f,  1.0f,  1.0f},
	{ -1.0f,  1.0f,  1.0f},
	{-1.0f,  1.0f, -1.0f},

	//Back face
	{- 1.0f, -1.0f, -1.0f},
	{-1.0f, -1.0f, 1.0f},
	{1.0f, -1.0f, -1.0f},
	{1.0f, -1.0f, -1.0f},
	{-1.0f, -1.0f, 1.0f},
	{1.0f, -1.0f, 1.0f}

};

//Store either polygon or wireframe
bool isWireframe = false;

//Initial position of the directional light
vec3 lightPos = vec3(0, -0.5, -0.5);

bool initializeGL()
{
	//Initialise GLFW
	if (!glfwInit())
	{
		cerr << "Failed to initialise GLFW" << endl;
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 1); //No anti-aliasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //Statement to please MacOS
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(window_width, window_height, "OpenGLRenderer", NULL, NULL);

	if (window == NULL)
	{
		cerr << "Failed to open GLFW window. If you have an IntelGPU, they may not be 4.5 compatible." << endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(window);

	//Initialise GLEW
	glewExperimental = true; //Needed for core profile
	if (glewInit() != GLEW_OK)
	{
		cerr << "Failed to initialise GLEW" << endl;
		glfwTerminate();
		return false;
	}

	if (!GLEW_ARB_debug_output)
		return -1;

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwPollEvents();
	glfwSetCursorPos(window, window_width / 2, window_height / 2);
}

//Create the mesh, and connect it to OpenGL
void LoadModel()
{
	std::vector<vec3> vertices;
	std::vector<vec2>uvs;
	std::vector<unsigned int> indices;

	//Create points by mapping them to [-1, 1] interval and then multiply by scale factor
	for (int i = 0; i < n_points; i++)
	{
		float x = (m_scale) * ((i / float(n_points - 1)) - 0.5f) * 2.0f;
		for (int j = 0; j < n_points; j++)
		{
			float z = (m_scale) * ((j / float(n_points - 1)) - 0.5f) * 2.0f;
			vertices.push_back(vec3(x, 0, z));
			uvs.push_back(vec2(float(i + 0.5f) / float(n_points - 1),
								float(j + 0.5f) / float(n_points - 1)));
		}
	}

	//Specify a triangle strip for each row of two vertices
	//We don't want them to be connected, so we restart the primitive when changing rows
	//This way, we specify less vertices and run the shader less times
	glEnable(GL_PRIMITIVE_RESTART);
	constexpr unsigned int restartIndex = numeric_limits<uint32_t>::max(); //Choose the largest index that can possibly be used
	glPrimitiveRestartIndex(restartIndex);
	int n = 0;
	for (int i = 0; i < n_points - 1; i++)
	{
		for (int j = 0; j < n_points; j++)
		{
			unsigned int topLeft = n;
			unsigned int bottomLeft = topLeft + n_points;
			indices.push_back(bottomLeft);
			indices.push_back(topLeft);
			n++;
		}

		indices.push_back(restartIndex);
	}

	glGenVertexArrays(1, &VertexArrayID); //Initialise VAO
	glBindVertexArray(VertexArrayID); // All of the following function calls affects the VAO with the given name

	//Describe vertex positions
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,	//attribute
		3,	//Size
		GL_FLOAT,	//Type of each individual element
		GL_FALSE,	//Normalised?
		0,	//Stride
		(void*)0	//Array buffer object
	);

	//Describe UVs
	glEnableVertexAttribArray(1);
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(vec2), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(
		1,	//attribute
		2,	//Size
		GL_FLOAT,	//Type of each individual element
		GL_FALSE,	//Normalised?
		0,	//Stride
		(void*)0	//Array buffer object
	);

	//Describe indices
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
	
	//Store how many indices in the plane (needed for rendering)
	nIndices = indices.size();

}

//Loading Textures
void LoadTextures()
{
	/*
	***************************************************
		Loading the rock textures
	***************************************************

	*/
	//Load BMP File
	int width, height;
	unsigned char* data = nullptr;
	loadBMP_custom("rocks.bmp", width, height, data);

	//Hand over the BMP data to OpenGL
	glGenTextures(1, &rockDiffuseID);
	glBindTexture(GL_TEXTURE_2D, rockDiffuseID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	delete[] data;

	//Finish texture setup
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbind current texture -> Good practice, only bind when you need to
	glBindTexture(GL_TEXTURE_2D, -1);

	//Load the shininess texture
	unsigned char* rockShininessData;
	loadBMP_custom("rocks-r.bmp", width, height, rockShininessData);

	//Hand over rock shininess data to OpenGL
	glGenTextures(1, &rockShininessID);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, rockShininessID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, rockShininessData);
	delete[] rockShininessData;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	//Load the rock normals texture
	unsigned char* rockNormalsData;
	loadBMP_custom("rocks-n.bmp", width, height, rockNormalsData);

	//Hand over rock shininess data to OpenGL
	glGenTextures(1, &rockNormalsID);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, rockNormalsID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, rockNormalsData);
	delete[] rockNormalsData;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);


	/*
	***************************************************
		Loading the heightmap
	***************************************************
	*/

	unsigned char* heightData = nullptr;
	loadBMP_custom("rugged.bmp", width, height, heightData);

	//Hand over heightmap data to OpenGl
	glGenTextures(1, &heightMapID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, heightMapID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, heightData);
	delete[] heightData;

	//Sampling method for the height map
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbind current texture after initialisation (good practice)
	glBindTexture(GL_TEXTURE_2D, -1);


	/*
	***************************************************
		Loading the snow textures
	***************************************************
	*/

	//Snow Diffuse
	unsigned char* snowDiffuseData = nullptr;
	loadBMP_custom("snow.bmp", width, height, snowDiffuseData);

	//Hand over snow diffuse data to OpenGl
	glGenTextures(1, &snowDiffuseID);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, snowDiffuseID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, snowDiffuseData);
	delete[] snowDiffuseData;

	//Sampling method for the snow diffuse
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbind current texture after initialisation (good practice)
	glBindTexture(GL_TEXTURE_2D, -1);

	//Snow Shininess
	unsigned char* snowShininessData;
	loadBMP_custom("snow-r.bmp", width, height, snowShininessData);

	//Hand over snow shininess data to OpenGL
	glGenTextures(1, &snowShininessID);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, snowShininessID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, snowShininessData);
	delete[] snowShininessData;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	//Snow Normals
	unsigned char* snowNormalData;
	loadBMP_custom("snow-n.bmp", width, height, snowNormalData);

	//Hand over snow shininess data to OpenGL
	glGenTextures(1, &snowNormalsID);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, snowNormalsID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, snowNormalData);
	delete[] snowNormalData;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	/*
	***************************************************
		Loading the grass textures
	***************************************************
	*/

	//Grass Diffuse
	unsigned char* grassDiffuseData = nullptr;
	loadBMP_custom("grass.bmp", width, height, grassDiffuseData);

	//Hand over snow diffuse data to OpenGl
	glGenTextures(1, &grassDiffuseID);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, grassDiffuseID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, grassDiffuseData);
	delete[] grassDiffuseData;

	//Sampling method for the snow diffuse
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	//Unbind current texture after initialisation (good practice)
	glBindTexture(GL_TEXTURE_2D, -1);

	//Grass Shininess
	unsigned char* grassShininessData;
	loadBMP_custom("grass-r.bmp", width, height, grassShininessData);

	//Hand over grass shininess data to OpenGL
	glGenTextures(1, &grassShininessID);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, grassShininessID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, grassShininessData);
	delete[] grassShininessData;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);

	//Grass Normals
	unsigned char* grassNormalData;
	loadBMP_custom("grass-n.bmp", width, height, grassNormalData);

	//Hand over grass shininess data to OpenGL
	glGenTextures(1, &grassNormalsID);
	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, grassNormalsID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, grassNormalData);
	delete[] grassNormalData;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, -1);
}

//Read shader file and compile it
bool readAndCompileShader(const char* shader_path, const GLuint& id)
{
	string shaderCode;
	ifstream shaderStream(shader_path, ios::in);
	if (shaderStream.is_open())
	{
		stringstream sstr;
		sstr << shaderStream.rdbuf();
		shaderCode = sstr.str();
		shaderStream.close();
	}

	else
	{
		cout << "Impossible to open " << shader_path << ". Are you in the right directory?" << endl;
		return false;
	}

	cout << "Compiling shader : " << shader_path << endl;
	char const* sourcePointer = shaderCode.c_str();
	glShaderSource(id, 1, &sourcePointer, NULL);
	glCompileShader(id);

	//Test to see if the compilation went okay
	GLint Result = GL_FALSE;

	int infoLogLength;
	glGetShaderiv(id, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0)
	{
		vector<char> shaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(id, infoLogLength, NULL, &shaderErrorMessage[0]);
		cout << &shaderErrorMessage[0] << endl;
	}

	cout << "Compilation of Shader: " << shader_path << " " << (Result == GL_TRUE ? "Success" : "Failed") << endl;
	return Result == 1;
}

void LoadShaders(GLuint& program, const char* vertex_file_path, const char* fragment_file_path, const char* geometry_file_path = "")
{
	//Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint GeometryShaderID;
	bool geometryExists = false;
	if (geometry_file_path != "")
	{
		geometryExists = true;
		GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
	}
	

	bool vok = readAndCompileShader(vertex_file_path, VertexShaderID);

	bool gok;
	if (geometryExists)
		gok = readAndCompileShader(geometry_file_path, GeometryShaderID);

	bool fok = readAndCompileShader(fragment_file_path, FragmentShaderID);

	//Check whether vertex and fragment are fine, or vertex, fragment and geometry, depending on the geometry shader's existence
	if ((vok && fok && !geometryExists) || (vok && fok && gok && geometryExists))
	{
		GLint Result = GL_FALSE;
		int InfoLogLength;

		cout << "Linking Program" << endl;
		program = glCreateProgram();
		glAttachShader(program, VertexShaderID);

		if (geometryExists)
			glAttachShader(program, GeometryShaderID);

		glAttachShader(program, FragmentShaderID);

		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &Result);
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);

		if (InfoLogLength > 0)
		{
			std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
			glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]);
			cout << &ProgramErrorMessage[0];
		}

		cout << "Linking program: " << (Result == GL_TRUE ? "Success" : "Failed") << endl;

	}
	
	else
	{
		cout << "Program will not be linked: one of the shaders has an error" << endl;
	}

	//The shader objects have been compiled into the program -> we don't need them anymore
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	if (geometryExists)
		glDeleteShader(GeometryShaderID);
}

//
//Clean Up Routines
//
void UnloadModel()
{
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);

	glDeleteVertexArrays(1, &skyboxVertexArray);
	glDeleteBuffers(1, &skyboxBuffer);

	glDeleteVertexArrays(1, &sunflowerVertexArray);
	glDeleteBuffers(1, &sunflowerBuffer);
}

void UnloadTextures()
{
	glDeleteTextures(1, &rockDiffuseID);
	glDeleteTextures(1, &rockShininessID);
	glDeleteTextures(1, &rockNormalsID);

	glDeleteTextures(1, &heightMapID);

	glDeleteTextures(1, &snowDiffuseID);
	glDeleteTextures(1, &snowShininessID);
	glDeleteTextures(1, &snowNormalsID);

	glDeleteTextures(1, &grassDiffuseID);
	glDeleteTextures(1, &grassShininessID);
	glDeleteTextures(1, &grassNormalsID);

	//Delete the skybox texture
	glDeleteTextures(1, &skyboxTextureID);

	//Delete sunflower texture
	glDeleteTextures(1, &sunflowerTextureID);
}

void UnloadShaders()
{
	glDeleteProgram(programID);
	glDeleteProgram(skyboxID);
	glDeleteProgram(sunflowerID);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	//Is we press ESC, tell the program the close the window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//Enable/Disable wireframe rendering with SPACE
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		isWireframe = !isWireframe;

		if (isWireframe == true)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	}

	//Reload shaders if r is pressed
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		UnloadShaders();
		LoadShaders(programID, "Basic.vert", "Texture.frag");
		LoadShaders(skyboxID, "src/skyboxVert.vert", "src/skyboxFrag.frag");
		LoadShaders(sunflowerID, "src/sunflower.vert", "src/sunflower.frag", "src/sunflower.geom");

	}

	//Rotate Directional Light around x axis
	if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		//Create rotation matrix
		float angle = radians(5.0);
		mat4 rotationMatrix = rotate(mat4(1.0), angle, vec3(1.0, 0.0, 0.0));

		//Apply rotation matrix to light Position
		lightPos = vec3(rotationMatrix * vec4(lightPos, 0.0));

	}

	if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		float angle = radians(355.0);
		mat4 rotationMatrix = rotate(mat4(1.0), angle, vec3(1.0, 0.0, 0.0));
		lightPos = vec3(rotationMatrix * vec4(lightPos, 0.0));

	}

	//Rotate Directional Light around z axis
	if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		float angle = radians(5.0);
		mat4 rotationMatrix = rotate(mat4(1.0), angle, vec3(0.0, 0.0, 1.0));
		lightPos = vec3(rotationMatrix * vec4(lightPos, 0.0));

	}

	if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		float angle = radians(355.0);
		mat4 rotationMatrix = rotate(mat4(1.0), angle, vec3(0.0, 0.0, 1.0));
		lightPos = vec3(rotationMatrix * vec4(lightPos, 0.0));
	}

	//Increment and decrement the scale value (affects the height of the mountains)
	if (key == GLFW_KEY_T && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		scaleValue = scaleValue + 0.1;
	}

	if (key == GLFW_KEY_G && (action == GLFW_PRESS || action == GLFW_REPEAT))
	{
		if(scaleValue > 0)
			scaleValue = scaleValue - 0.1;
	}

}

void LoadSkybox()
{
	//Initialise skybox VAO and VBO
	glGenVertexArrays(1, &skyboxVertexArray); //Initialise VAO
	glBindVertexArray(skyboxVertexArray); // All of the following function calls affects the VAO with the given name

	//Describe vertex positions
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &skyboxBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxBuffer);
	glBufferData(GL_ARRAY_BUFFER, skyboxVerts.size() * sizeof(vec3), &skyboxVerts[0], GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,	//attribute
		3,	//Size
		GL_FLOAT,	//Type of each individual element
		GL_FALSE,	//Normalised?
		0,	//Stride
		(void*)0	//Array buffer object
	);

	//Load the textures
	glGenTextures(1, &skyboxTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);

	vector <const char*> cubeTextures = {
		"external/skybox/right.jpg",
		"external/skybox/left.jpg",
		"external/skybox/top.jpg",
		"external/skybox/bottom.jpg",
		"external/skybox/front.jpg",
		"external/skybox/back.jpg"
	};

	//Load every face of the texture individually in a loop
	for (int i = 0; i < cubeTextures.size(); i++)
	{
		//Load the image using the external stbi_image library
		int width, height, numChannels;
		unsigned char* skyboxData = stbi_load(cubeTextures.at(i), &width, &height, &numChannels, 0);

		//Check that it went through alright
		if (skyboxData) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, skyboxData);
			cout << "Successfully loaded: " << cubeTextures.at(i) << endl;
		}
		else
			cout << "Failed to load at path: " << cubeTextures.at(i) << endl;


		//Free image data (we don't need it anymore)
		stbi_image_free(skyboxData);
	}
	
	//Set the texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//Load the sunflower billboards
void LoadSunflower()
{
	//Initialise sunflower vertex positions
	//Start by mapping the corners of [-1, 1]
	vector <vec3> sunflowerVerts = {
		{-4.09,1.0,-3.77},
		{3.63, 1.0, 1.14},
		{1.16, 0.86, 2.48}
	};

	//Initialise VAO and VBO
	glGenVertexArrays(1, &sunflowerVertexArray);
	glBindVertexArray(sunflowerVertexArray);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &sunflowerBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sunflowerBuffer);
	glBufferData(GL_ARRAY_BUFFER, sunflowerVerts.size() * sizeof(vec3), &sunflowerVerts[0], GL_STATIC_DRAW);
	
	glVertexAttribPointer(
		0,	//attribute
		3,	//Size
		GL_FLOAT,	//Type of each individual element
		GL_FALSE,	//Normalised?
		0,	//Stride
		(void*)0	//Array buffer object
	);

	//Load sunflower texture
	glGenTextures(1, &sunflowerTextureID);
	glBindTexture(GL_TEXTURE_2D, sunflowerTextureID);

	//Flip the texture whilst reading
	stbi_set_flip_vertically_on_load(true);

	int width, height, numChannels;
	unsigned char* sunflowerData = stbi_load("external/sunflower.png", &width, &height, &numChannels, 0);

	if (sunflowerData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sunflowerData);
		cout << "Successfully loaded: external/sunflower.png" << endl;
	}

	else
		cout << "Failed to load: external/sunflower.png" << endl;
	
	stbi_image_free(sunflowerData);

	//Set Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

}

int main(){
	//Initialise OpenGL and its extensions
	if (!initializeGL())
		return -1;

	glfwSetKeyCallback(window, key_callback);

	//Setup program for the model
	LoadModel();
	LoadTextures();
	programID = glCreateProgram();
	LoadShaders(programID, "Basic.vert", "Texture.frag");

	//Setup program for the skybox
	skyboxID = glCreateProgram();
	LoadSkybox();
	LoadShaders(skyboxID, "src/skyboxVert.vert", "src/skyboxFrag.frag");

	//Setup Program for the billboards
	sunflowerID = glCreateProgram();
	LoadSunflower();
	LoadShaders(sunflowerID, "src/sunflower.vert", "src/sunflower.frag", "src/sunflower.geom");

	//Set general OpenGL properties related to rendering
	glClearColor(0.7f, 0.8f, 1.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);

	do
	{
		//Clear the screen (prevents drawing on top of previous frame)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();

		mat4 ProjectionMatrix = getProjectionMatrix();
		mat4 ViewMatrix = getViewMatrix();
		mat4 ModelMatrix = mat4(1.0);
		mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		//Create only the model view matrix to get the view coordinate system
		mat4 modelViewMatrix = ViewMatrix * ModelMatrix;

		vec3 cameraPos = getCameraPosition();
		string title = "OpenGL Renderer - (" + to_string(cameraPos[0]) + "," + to_string(cameraPos[1]) + "," + to_string(cameraPos[2]) + ")";
		glfwSetWindowTitle(window, title.c_str());

		//First pass -> draw skybox
		glDepthMask(GL_FALSE);
		glUseProgram(skyboxID);

		//Get the view matrix without translation (required for skybox)
		mat4 modifiedView = mat4(mat3(ViewMatrix));

		glUniformMatrix4fv(glGetUniformLocation(skyboxID, "view"), 1, GL_FALSE, &modifiedView[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(skyboxID, "projection"), 1, GL_FALSE, &ProjectionMatrix[0][0]);

		glBindVertexArray(skyboxVertexArray);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
		glDrawArrays(GL_TRIANGLES, 0, skyboxVerts.size());

		glBindVertexArray(0);

		glUseProgram(programID);
		glDepthMask(GL_TRUE);


		//Bind the mesh VAO
		glBindVertexArray(VertexArrayID);

		glUniformMatrix4fv(glGetUniformLocation(programID, "modelView"), 1, GL_FALSE, &modelViewMatrix[0][0]);

		//Send the uniform values to the shaders
		glUniform3f(glGetUniformLocation(programID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(glGetUniformLocation(programID, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

		glUniform1f(glGetUniformLocation(programID, "numPoints"), n_points);
		glUniform1f(glGetUniformLocation(programID, "scaleValue"), scaleValue);
		
		
		//Second pass -> base mesh
		glUseProgram(programID);

		//Get a handle for our uniforms
		GLuint MatrixID = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		//Assign the height map to the correct uniform value in the vertex shader
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, heightMapID);
		glUniform1i(glGetUniformLocation(programID, "heightMap"), 1);

		//Assign the rock shininess to the fragment shader
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, rockShininessID);

		//Assign the texture colour to the fragment shader
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rockDiffuseID);

		//Draw
		glDrawElements(GL_TRIANGLE_STRIP, //Mode
					  (GLsizei)nIndices, //Count
					  GL_UNSIGNED_INT, //Type
					  (void*)0	//Element array buffer offset
		);


		//Third pass -> handle billboards
		glUseProgram(sunflowerID);

		glDisable(GL_CULL_FACE);
		mat4 projectionViewMatrix = ProjectionMatrix * ViewMatrix;

		glUniformMatrix4fv(glGetUniformLocation(sunflowerID, "viewProjection"), 1, GL_FALSE, &projectionViewMatrix[0][0]);

		glUniformMatrix4fv(glGetUniformLocation(sunflowerID, "cameraPosition"), 1, GL_FALSE, &cameraPos[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sunflowerTextureID);

		glBindVertexArray(sunflowerVertexArray);
		glDrawArrays(GL_POINTS, 0, 3);
		glBindVertexArray(0);

		glUseProgram(programID);
		glEnable(GL_CULL_FACE);

		//Swap Buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (glfwWindowShouldClose(window) == 0);

	//Before we return, we clean up glfw
	UnloadModel();
	UnloadShaders();
	UnloadTextures();
	glfwTerminate();
	return 0;
}