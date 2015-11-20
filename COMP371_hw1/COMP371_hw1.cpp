// COMP371_hw1.cpp : Defines the entry point for the console application.
//
#define _CRT_SECURE_NO_DEPRECATE

#include "stdafx.h"

#include "..\glew\glew.h"		// include GL Extension Wrangler

#include "..\glfw\glfw3.h"  // include GLFW helper library

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "gtc/constants.hpp"


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "objloader.hpp"
#include "texture.hpp"
using namespace std;

#define M_PI        3.14159265358979323846264338327950288   /* pi */
#define DEG_TO_RAD	M_PI/180.0f

GLFWwindow* window = 0x00;
void refresh();
//Create Vec3 for Vertices, UVs & Normals of Jet
std::vector<glm::vec3> jetVertices;
std::vector<glm::vec2> jetUvs;
std::vector<glm::vec3> jetNormals;

//Create Vec3 for Vertices, UVs & Normals of Building
std::vector<glm::vec3> buildVertices;
std::vector<glm::vec2> buildUvs;
std::vector<glm::vec3> buildNormals;

std::vector<glm::vec3> terrainVertices;
std::vector<glm::vec2> terrainUvs;
std::vector<glm::vec3> terrainNormals;

//Camera Vector Locations
glm::vec3 camera = glm::vec3(0, 35, -65); // Camera is at (0,3,-5), in World Space
glm::vec3 look = glm::vec3(0, 0,1.0f); // and looks at the origin
glm::vec3 up = glm::vec3(0, 1, 0);  // Head is up
float x = 0;
float y = 0;
float z = 0;
GLuint shader_program = 0;
GLuint vbo[6];
GLuint vao[3];
GLuint MVP_id = 0;
GLuint jetTexture;
GLuint buildTexture;
GLuint terrainTexture;
GLuint TextureID;
bool res;
///Transformations
glm::mat4 view; 
glm::mat4 projection;

glm::mat4 jetTranslation = glm::mat4(1.0f);
glm::mat4 jetRotation = glm::mat4(1.0f);

glm::mat4 terrainTranslation[4];
glm::mat4 terrainScale = glm::mat4(1.0f);

glm::mat4 buildTranslation = glm::mat4(1.0f);
glm::mat4 buildRotation = glm::mat4(1.0f);
glm::mat4 jetModel = glm::mat4(1.0f);

glm::mat4 buildModel = glm::mat4(1.0f);
vector <glm::mat4> terrainModels;
glm::mat4 terrainTemps[4];
glm::mat4 identityM = glm::mat4(1.0f);
glm::mat4 * trans;
glm::mat4 MVP;
int w = 1200;
int already = 1;
int h = 900;
float distance;
float xJet,yJet,zJet;
float lastTime = glfwGetTime();
float currentTime;
float deltaTime;
int planes = 0;
int place = 0;
unsigned char header[54];
unsigned int dataPos;
unsigned int width, height;
unsigned int imageSize;
unsigned char * data;
///Change the size of the rendered points
GLfloat point_size = 1.0f;

void keyPressed(GLFWwindow *_window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_KP_SUBTRACT) {
		point_size = std::max(point_size - 0.5f, 1.0f);
		std::cout << "Decreased point size to " << point_size << std::endl;
	}
	if (key == GLFW_KEY_KP_ADD) {
		point_size += 0.5f;
		std::cout << "Increased point size to " << point_size << std::endl;
	}

	//TODO: Handle other input data from the keyboard

	return;
}
void window_size_callback(GLFWwindow* window, int width, int height)
{
	cout << height;
	float aspect = (float)width / height;
	projection = glm::perspective(45.0f, aspect, 0.1f, 300.0f);
	glViewport(0, 0, width, height);
}

bool initialize() {
	/// Initialize GL context and O/S window using the GLFW helper library
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/// Create a window of size 640x480 and with title "Lecture 2: First Triangle"
	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	window = glfwCreateWindow(w, h, "Project", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}
	
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	///Register the keyboard callback function: keyPressed(...)
	glfwSetKeyCallback(window, keyPressed);
	//TODO: Add more callback functions for mouse, window resize
	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwMakeContextCurrent(window);

	/// Initialize GLEW extension handler
	glewExperimental = GL_TRUE;	///Needed to get the latest version of OpenGL
	glewInit();

	/// Get the current OpenGL version 
	const GLubyte* renderer = glGetString(GL_RENDERER); /// Get renderer string
	const GLubyte* version = glGetString(GL_VERSION); /// Version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	/// Enable the depth test i.e. draw a pixel if it's closer to the viewer
	glEnable(GL_DEPTH_TEST); /// Enable depth-testing
	glDepthFunc(GL_LESS);	/// The type of testing i.e. a smaller value as "closer"

	//TODO: Initialize other variables you are using

	return true;
}

bool cleanUp() {
	//Release memory e.g. vao, vbo, etc

	// Close GL context and any other GLFW resources
	glfwTerminate();

	return true;
}



GLuint loadShaders(std::string vertex_shader_path, std::string fragment_shader_path)	{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_shader_path.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_shader_path.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	
	glBindAttribLocation(ProgramID, 0, "in_Position");
	//Uncomment below for extra credit. You should also uncomment the variable of the same name 
	//appearing in the vertex shader.
	//glBindAttribLocation(ProgramID, 1, "in_Color");

	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	//The three variables below hold the id of each of the variables in the shader
	//If you read the vertex shader file you'll see that the same variable names are used.
	MVP_id = glGetUniformLocation(ProgramID, "MVP");
	TextureID = glGetUniformLocation(ProgramID, "myTextureSampler");
	return ProgramID;
}


GLuint raw_texture_load(const char *filename, int width, int height)
{
	GLuint texture;
	unsigned char *data;
	FILE *file;

	// open texture data
	file = fopen(filename, "rb");
	if (file == NULL) return 0;

	// allocate buffer
	data = (unsigned char*)malloc(width * height * 4);

	// read texture data
	fread(data, width * height * 4, 1, file);
	fclose(file);

	// allocate a texture name
	glGenTextures(1, &texture);

	// select our current texture
	glBindTexture(GL_TEXTURE_2D, texture);

	// select modulate to mix texture with color for shading
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);

	// when texture area is small, bilinear filter the closest mipmap
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	// when texture area is large, bilinear filter the first mipmap
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// texture should tile
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// build our texture mipmaps
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	// free buffer
	free(data);

	return texture;
}

void loadJet() 
{
	res = loadOBJ("FA_22_Raptor3.obj", jetVertices, jetUvs, jetNormals);
	jetTexture = loadBMP_custom("FA-22_Raptor_P01.bmp");
	if (res == false) {
		cout << "Jet could not be loaded" << endl;
		exit(1);
	}
}

void loadTerrain()
{
	res = loadOBJ("plane.obj", terrainVertices, terrainUvs, terrainNormals);
	terrainTexture= loadBMP_custom("grass.bmp");
	if (res == false) {
		cout << "Terrain could not be loaded" << endl;
		exit(1);
	}
}

void loadBuilding()
{
	res = loadOBJ("cube.obj", buildVertices, buildUvs, buildNormals);
	buildTexture = loadBMP_custom("house.bmp");
	if (res == false) {
		cout << "Building could not be loaded" << endl;
		exit(1);
	}
}



void setJet(float xi, float yi, float zi) {
	glm::mat4 rot;
	jetTranslation = glm::translate(identityM, glm::vec3(xi, yi, zi));
	jetModel = jetTranslation;
}

void setTerrain() {
	for (int i = 0; i < 4; i++) {
		terrainTranslation[i] = glm::translate(identityM, glm::vec3(0, 0, (0+(planes)*170)));
		terrainModels.push_back(terrainTranslation[i]);
		++planes;
	}
	terrainModels.resize(4);
	terrainModels.shrink_to_fit();
}

void resize(int h) {
	terrainModels.clear();
	terrainTranslation[h] = glm::translate(identityM, glm::vec3(0, 0, (0 + (planes * 170))));
	for (int i = 0; i < 4; i++) {
			terrainModels.push_back(terrainTranslation[i]);
		}
	terrainModels.shrink_to_fit();
	//terrainModels.pop_back();
++planes;
}

void setBuilding(float xBuild, float yBuild, float zBuild) {
	buildTranslation = glm::translate(terrainModels[0], glm::vec3(xBuild, yBuild, zBuild));
	buildModel = buildTranslation;
}

void moveJet(float speed) {
	currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	float d;
	d = deltaTime*speed;
	zJet += d;
	jetTranslation = glm::translate(identityM, glm::vec3(xJet, yJet,zJet));
	jetModel = jetTranslation;
	camera.z += d;
	look.z += d;
}

void loadObjects() {
	loadJet();
	loadTerrain();
	loadBuilding();
}

void refresh() {
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, NULL , NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(glm::vec3), &terrainVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
}
int main() {
	///Make sure the size of vec3 is the same as 3 floats
	assert(sizeof(glm::vec3) == sizeof(float) * 3);
	assert(sizeof(glm::uvec3) == sizeof(unsigned int) * 3);

	initialize();
	loadObjects();
	xJet = 0;
	yJet = 30.0f;
	zJet = 0.0f;
	setJet(xJet, yJet, zJet);
	setTerrain();
	setBuilding(59.0f, 1.0f, 79.0f);
	

	///Load the shaders
	shader_program = loadShaders("COMP371_hw1.vs", "COMP371_hw1.fs");

	view = glm::lookAt(
		camera,
		look,
		up
		);

	//create the projection matrix
	projection = glm::perspective(45.0f, (float)w / h, 0.1f, 500.0f);
	
	glGenVertexArrays(3, vao);
	glGenBuffers(6, vbo);
	
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, jetVertices.size() * sizeof(glm::vec3), &jetVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, jetUvs.size() * sizeof(glm::vec2), &jetUvs[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);


	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(glm::vec3), &terrainVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, terrainUvs.size() * sizeof(glm::vec2), &terrainUvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, buildVertices.size() * sizeof(glm::vec3), &buildVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, buildUvs.size() * sizeof(glm::vec2), &buildUvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	while (!glfwWindowShouldClose(window)) {
		lastTime = glfwGetTime();
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPointSize(point_size);
		
		glUseProgram(shader_program);

		lastTime = glfwGetTime();
		moveJet(30000.0f);
		view = glm::lookAt(
			camera,
			look,
			up
			);
		//cout << "Look Z: " << look.z << "Camera Z" << camera.z << endl;
		MVP = projection*view*jetModel;
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, jetTexture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		glBindVertexArray(vao[0]);
		glDrawArrays(GL_TRIANGLES, 0, jetVertices.size());

		for (int i = 0; i < terrainModels.size(); i++) {
			MVP = projection*view*terrainModels[i];
			glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, terrainTexture);
			glUniform1i(TextureID, 0);

			glBindVertexArray(vao[1]);
			glDrawArrays(GL_TRIANGLES, 0, terrainVertices.size());
		}
		MVP = projection*view*buildModel;
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, buildTexture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(TextureID, 0);

		glBindVertexArray(vao[2]);
		glDrawArrays(GL_TRIANGLES, 0, buildVertices.size());

		//TODO: Once you have passed all information you are ready to do the drawing
		//glDrawElements(..........)
		if ((int)look.z % 170 ==0 && already == 0) {
			if (place == 4) {
				place = 0;
			}
				resize(place);
			++place;
			already = 2;
		}

		if ((int)look.z % 52 == 0) {
			already = 0;
		}
		// update other events like input handling 
		glfwPollEvents();
		// put the stuff we've been drawing onto the display

		glfwSwapBuffers(window);
		
	}

	cleanUp();
	return 0;
}



