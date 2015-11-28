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
#include "gtx/norm.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <random>
#include <algorithm>
#include "objloader.hpp"
#include "texture.hpp"
#include "shader.hpp"
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

std::vector<glm::vec3> skyboxVertices;
std::vector<glm::vec2> skyboxUvs;
std::vector<glm::vec3> skyboxNormals;

std::vector<glm::vec3> shrubVertices;
std::vector<glm::vec2> shrubUvs;
std::vector<glm::vec3> shrubNormals;
void checkShrub(int i, int j);
void checkBuilding(int i, int j);
//Camera Vector Locations
glm::vec3 camera = glm::vec3(0, 30, -65); // Camera is at (0,3,-5), in World Space
glm::vec3 look = glm::vec3(0, 0,1.0f); // and looks at the origin
glm::vec3 up = glm::vec3(0, 1, 0);  // Head is up
float x = 0;
float y = 0;
float z = 0;
GLuint shader= 0;
glm::vec3 lightPosition;
GLuint vbo[15];
GLuint vao[5];
GLuint MVP_id = 0;
GLuint jetTexture;
GLuint skyboxTexture;
GLuint sunTexture;
GLuint shrubTexture;
GLuint buildTexture;
GLuint buildTexture1;
GLuint buildTexture2;
GLuint buildTexture3;
GLuint buildTextures[4][7];
GLuint terrainTexture;
GLuint TextureID;
GLuint ViewMatrixID = 0;
GLuint ModelMatrixID = 0;
GLuint LightID;

//particle initilization
GLuint programID;
GLuint CameraRight_worldspace_ID;
GLuint CameraUp_worldspace_ID;
GLuint ViewProjMatrixID;
GLuint pTextureID;
GLuint VertexID;
GLuint VertexArrayID;
GLuint sTexture;
GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;
glm::vec3 CameraPosition;
glm::mat4 ViewProjectionMatrix;

bool res;
///Transformations
glm::mat4 view; 
glm::mat4 projection;

glm::mat4 jetTranslation = glm::mat4(1.0f);
glm::mat4 jetRotation = glm::mat4(1.0f);

glm::mat4 terrainTranslation[4];
glm::mat4 terrainScale = glm::mat4(1.0f);

glm::mat4 buildModels[4][8];
glm::mat4 buildTranslations[4][8];
glm::mat4 buildRotations[4][8];

glm::mat4 shrubModels[4][10];
glm::mat4 shrubTranslations[4][10];
glm::mat4 shrubScale;
glm::mat4 skyboxModel;
glm::mat4 skyboxScale;
glm::mat4 skyboxTranslation;

glm::mat4 buildScale = glm::mat4(1.0f);
glm::mat4 jetModel = glm::mat4(1.0f);
vector <glm::mat4> terrainModels;
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
float zSkybox;
float yShrub;
float snowMov=15;

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_real_distribution<double> xdistribution(-120.0, 120.0);
std::uniform_real_distribution<double> zdistribution(-150.0, 150.0);
std::uniform_real_distribution<double> adistribution(0.0, 360.0);
std::uniform_real_distribution<double> hdistribution(0, 3);
///Change the size of the rendered points
GLfloat point_size = 1.0f;

static const GLfloat g_vertex_buffer_data[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f, 0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
};



struct Particle {
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};
const int MaxParticles = 4000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

int FindUnusedParticle() {

	for (int i = LastUsedParticle; i<MaxParticles; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i<LastUsedParticle; i++) {
		if (ParticlesContainer[i].life < 0) {
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}


void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}


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
	float aspect = (float)width / height;
	projection = glm::perspective(45.0f, aspect, 0.1f, 500.0f);
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
	glDeleteBuffers(15, vbo);
	glDeleteBuffers(5, vao);
	//delete the program
	//glDeleteProgram(shader);
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
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
	res = loadOBJ("plane3.obj", terrainVertices, terrainUvs, terrainNormals);
	terrainTexture= loadBMP_custom("snow.bmp");
	if (res == false) {
		cout << "Terrain could not be loaded" << endl;
		exit(1);
	}
}

void loadBuilding()
{
	res = loadOBJ("cottage2.obj", buildVertices, buildUvs, buildNormals);
	buildTexture = loadBMP_custom("cottage.bmp");
	buildTexture1 = loadBMP_custom("cottage2.bmp");
	buildTexture2= loadBMP_custom("cottage3.bmp");
	buildTexture3 = loadBMP_custom("cottage4.bmp");
	if (res == false) {
		cout << "Building could not be loaded" << endl;
		exit(1);
	}
}

void loadSkyBox() {
	res = loadOBJ("cube.obj", skyboxVertices, skyboxUvs, skyboxNormals);
	skyboxTexture = loadBMP_custom("uvtemplate4.bmp");
	if (res == false) {
		cout << "Skybox could not be loaded" << endl;
		exit(1);
	}
}

void loadShrub() {
	res = loadOBJ("Sphere.obj", shrubVertices, shrubUvs, shrubNormals);
	shrubTexture = loadBMP_custom("shrub.bmp");
	if (res == false) {
		cout << "Shrub could not be loaded" << endl;
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
		terrainTranslation[i] = glm::translate(identityM, glm::vec3(0, 0, (planes*425)));
		terrainModels.push_back(terrainTranslation[i]);
		++planes;
	}
	//terrainModels.resize(4);
	terrainModels.shrink_to_fit();
}

void setSkybox() {
	skyboxScale = glm::scale(identityM, glm::vec3(140.0f, 100.0f, 300.0f));
	skyboxTranslation = glm::translate(identityM, glm::vec3(0, 99, 0));
	skyboxModel = skyboxTranslation * skyboxScale;
}

void redoBuidlings(int i) {
	float c, k, r;
	for (int j = 0; j < 8; j++) {
		c = xdistribution(mt);
		k = zdistribution(mt);
		buildTranslations[i][j] = glm::translate(terrainModels[i], glm::vec3(c, 0.0f, k));
		r = adistribution(mt);
		buildRotations[i][j] = glm::rotate(identityM, r, glm::vec3(0, 1.0f, 0));
		buildModels[i][j] = buildTranslations[i][j] * buildRotations[i][j] * buildScale;
		if (j>0) {
			checkBuilding(i, j);
		}
		h = hdistribution(mt);
		if (h == 0) {
			buildTextures[i][j] = buildTexture;
		}
		if (h == 1) {
			buildTextures[i][j] = buildTexture1;
		}
		if (h == 2) {
			buildTextures[i][j] = buildTexture2;
		}
		if (h == 3) {
			buildTextures[i][j] = buildTexture3;
		}
	}

}


void resize(int h) {
	terrainModels.clear();
	terrainTranslation[h] = glm::translate(identityM, glm::vec3(0, 0, (0 + (planes * 425))));
	for (int i = 0; i < 4; i++) {
			terrainModels.push_back(terrainTranslation[i]);
		}
	terrainModels.shrink_to_fit();
	redoBuidlings(h);
++planes;
}


void checkBuilding(int i, int j) {
	float x= buildModels[i][j][3][0];
	float z = buildModels[i][j][3][2];
	for (int t = 0; t< j; t++) {
		float xCheck = buildModels[i][t][3][0];
		float zCheck = buildModels[i][t][3][2];
		if (abs(x - xCheck) > 10 && abs(z - zCheck) > 10) {
			
		}
		else {
			cout << i << j << endl;
			float d, r;
			d = xdistribution(mt);
			r = zdistribution(mt);
			buildTranslations[i][j] = glm::translate(terrainModels[i], glm::vec3(d, 0.0f, r));
			buildModels[i][j] = buildTranslations[i][j] * buildRotations[i][j] * buildScale;
			checkBuilding(i, j);
		}
	}
}

void setBuilding() {
	buildScale = glm::scale(identityM, glm::vec3(0.4f, 0.4f, 0.4f));
	float r, c, k;
	int h;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 8; j++) {
			c = xdistribution(mt);
			k = zdistribution(mt);
			buildTranslations[i][j] = glm::translate(terrainModels[i], glm::vec3(c, 0.0f, k));
			r = adistribution(mt);
			buildRotations[i][j] = glm::rotate(identityM, r, glm::vec3(0, 1.0f, 0));
			buildModels[i][j] = buildTranslations[i][j] * buildRotations[i][j] * buildScale;
			if(j>0){
			checkBuilding(i, j);
			}
			h = hdistribution(mt);
			if (h == 0) {
				buildTextures[i][j] = buildTexture;
			}
			if (h == 1) {
				buildTextures[i][j] = buildTexture1;
			}
			if (h == 2) {
				buildTextures[i][j] = buildTexture2;
			}
			if (h == 3) {
				buildTextures[i][j] = buildTexture3;
			}
		}
	}
}

void moveJet(float speed) {
	currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	float d;
	d = deltaTime*speed;
	zJet += d;
	jetTranslation = glm::translate(identityM, glm::vec3(xJet, yJet,zJet));
	jetModel = jetTranslation;
	zSkybox += d;
	skyboxTranslation= glm::translate(identityM, glm::vec3(0, 99, zSkybox));
	skyboxModel = skyboxTranslation * skyboxScale;
	camera.z += d;
	look.z += d;
	lightPosition.z += d;

	snowMov += d;
}

void loadObjects() {
	loadJet();
	loadTerrain();
	loadBuilding();
	loadSkyBox();
	sTexture = loadDDS("particle.DDS");
	xJet = 0;
	yJet = 32.0f;
	zJet = 0.0f;
	setJet(xJet, yJet, zJet);
	setTerrain();
	setBuilding();
	setSkybox();
	lightPosition = glm::vec3(xJet, yJet+14, zJet+6);
}


int main() {
	///Make sure the size of vec3 is the same as 3 floats
	assert(sizeof(glm::vec3) == sizeof(float) * 3);
	assert(sizeof(glm::uvec3) == sizeof(unsigned int) * 3);

	initialize();
	loadObjects();
	///Load the shaders for Jet/Skybox/Sun/houses
	shader= LoadShaders("COMP371_hw1.vs", "COMP371_hw1.fs");
	MVP_id = glGetUniformLocation(shader, "MVP");
	TextureID = glGetUniformLocation(shader, "myTextureSampler");
	ViewMatrixID = glGetUniformLocation(shader, "V");
	ModelMatrixID = glGetUniformLocation(shader, "M");
	LightID = glGetUniformLocation(shader, "LightPosition_worldspace");

	// Load shaders for snow

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("Particle.vertexshader", "Particle.fragmentshader");

	// Vertex shader
	CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
	CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
	ViewProjMatrixID = glGetUniformLocation(programID, "VP");

	// fragment shader
	pTextureID = glGetUniformLocation(programID, "myTextureSampler");

	static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];


	for (int i = 0; i<MaxParticles; i++) {
		ParticlesContainer[i].life = -1.0f;
		ParticlesContainer[i].cameradistance = -1.0f;
	}

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

	view = glm::lookAt(
		camera,
		look,
		up
		);

	//create the projection matrix
	projection = glm::perspective(45.0f, (float)w / h, 0.1f, 500.0f);
	
	glGenVertexArrays(4, vao);
	glGenBuffers(12, vbo);
	
	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, jetVertices.size() * sizeof(glm::vec3), &jetVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, jetUvs.size() * sizeof(glm::vec2), &jetUvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, jetNormals.size() * sizeof(glm::vec3), &jetNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(vao[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(glm::vec3), &terrainVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glBufferData(GL_ARRAY_BUFFER, terrainUvs.size() * sizeof(glm::vec2), &terrainUvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glBufferData(GL_ARRAY_BUFFER, terrainNormals.size() * sizeof(glm::vec3), &terrainNormals[0], GL_STATIC_DRAW);
	

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[5]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	
	glBindVertexArray(vao[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glBufferData(GL_ARRAY_BUFFER, buildVertices.size() * sizeof(glm::vec3), &buildVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glBufferData(GL_ARRAY_BUFFER, buildUvs.size() * sizeof(glm::vec2), &buildUvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glBufferData(GL_ARRAY_BUFFER, jetNormals.size() * sizeof(glm::vec3), &buildNormals[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[6]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[7]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[8]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindVertexArray(vao[3]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glBufferData(GL_ARRAY_BUFFER, skyboxVertices.size() * sizeof(glm::vec3), &skyboxVertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
	glBufferData(GL_ARRAY_BUFFER, skyboxUvs.size() * sizeof(glm::vec2), &skyboxUvs[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
	glBufferData(GL_ARRAY_BUFFER, skyboxNormals.size() * sizeof(glm::vec3), &skyboxNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[9]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[10]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[11]);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	double slastTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		lastTime = glfwGetTime();
		// wipe the drawing surface clear
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPointSize(point_size);
		
		

		moveJet(8000.0f);
		view = glm::lookAt(
			camera,
			look,
			up
			);
		CameraPosition = camera;
		ViewProjectionMatrix = projection*view;
		glUseProgram(shader);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, glm::value_ptr(view));
		glUniform3f(LightID, lightPosition.x, lightPosition.y, lightPosition.z);


		MVP = projection*view*jetModel;
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, jetTexture);
		glUniform1i(TextureID, 0);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(jetModel));
		glBindVertexArray(vao[0]);
		glDrawArrays(GL_TRIANGLES, 0, jetVertices.size());

		MVP = projection*view*skyboxModel;
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, skyboxTexture);
		glUniform1i(TextureID, 0);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(skyboxModel));
		glBindVertexArray(vao[3]);
		glDrawArrays(GL_TRIANGLES, 0, skyboxVertices.size());


		for (int i = 0; i < terrainModels.size(); i++) {
			MVP = projection*view*terrainModels[i];
			glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(terrainModels[i]));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, terrainTexture);
			glUniform1i(TextureID, 0);

			glBindVertexArray(vao[1]);
			glDrawArrays(GL_TRIANGLES, 0, terrainVertices.size());
		}

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 8; j++) {
				MVP = projection*view*buildModels[i][j];
				glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
				glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(buildModels[i][j]));
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, buildTextures[i][j]);
				glUniform1i(TextureID, 0);
				glBindVertexArray(vao[2]);
				glDrawArrays(GL_TRIANGLES, 0, buildVertices.size());
			}
		}

		glm::mat4 model1 = glm::translate(identityM, glm::vec3(-2, -2, -2));
		MVP = projection*view*model1;
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(model1));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shrubTexture);
		glUniform1i(TextureID, 0);
		glBindVertexArray(vao[4]);
		glDrawArrays(GL_TRIANGLES, 0, shrubVertices.size());

		glm::mat4 model2 = glm::translate(identityM, glm::vec3(-4, -2, -2));
		MVP = projection*view*model1;
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, glm::value_ptr(MVP));
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, glm::value_ptr(model2));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shrubTexture);
		glUniform1i(TextureID, 0);
		glBindVertexArray(vao[4]);
		glDrawArrays(GL_TRIANGLES, 0, shrubVertices.size());

		//TODO: Once you have passed all information you are ready to do the drawing
		//glDrawElements(..........)
		if ((int)look.z % 425 ==0 && already == 0) {
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

		double scurrentTime = glfwGetTime();
		double delta = scurrentTime - slastTime;
		slastTime = scurrentTime;

		int newparticles = (int)(delta*10000.0);
		if (newparticles > (int)(0.006f*10000.0))
			newparticles = (int)(0.006f*10000.0);
		for (int i = 0; i<newparticles; i++) {
			int particleIndex = FindUnusedParticle();
			ParticlesContainer[particleIndex].life = 10.0f; // This particle will live 5 seconds.
			
			ParticlesContainer[particleIndex].pos = glm::vec3(0, 50, snowMov);
			float spread = 20.0f;
			glm::vec3 maindir = glm::vec3(0.0f, -10.0f, 0.0f);
			// Very bad way to generate a random direction; 
			// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// combined with some user-controlled parameters (main direction, spread, etc)
			glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f
				);

			ParticlesContainer[particleIndex].speed = maindir + randomdir*spread;



			ParticlesContainer[particleIndex].size = (rand() % 1000) / 2000.0f + 0.1f;

		}



		// Simulate all particles
		int ParticlesCount = 0;
		for (int i = 0; i<MaxParticles; i++) {

			Particle& p = ParticlesContainer[i]; // shortcut

			if (p.life > 0.0f) {

				// Decrease life
				p.life -= delta;
				if (p.life > 0.0f) {

					// Simulate simple physics : gravity only, no collisions
					p.speed += glm::vec3(0.0f, -4.81f, 0.5f) * (float)delta * 0.5f;
					p.pos += p.speed * (float)delta;
					p.cameradistance = glm::length2(p.pos - CameraPosition);
					//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

					// Fill the GPU buffer
					g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
					g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
					g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

					g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

				}
				else {
					// Particles that just died will be put at the end of the buffer in SortParticles();
					p.cameradistance = -1.0f;
				}

				ParticlesCount++;

			}
		}

		SortParticles();
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glUseProgram(programID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sTexture);
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(pTextureID, 0);

		// Same as the billboards tutorial
		glUniform3f(CameraRight_worldspace_ID, view[0][0], view[1][0], view[2][0]);
		glUniform3f(CameraUp_worldspace_ID, view[0][1], view[1][1], view[2][1]);

		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
			);


		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

									 // Draw the particules !
									 // This draws many times a small triangle_strip (which looks like a quad).
									 // This is equivalent to :
									 // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
									 // but faster.
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// update other events like input handling 
		glfwPollEvents();
		// put the stuff we've been drawing onto the display

		glfwSwapBuffers(window);
		
	}

	cleanUp();
	return 0;
}