#include <cassert>
#include <cstring>
#include <memory>
#include <random>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "Object.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

using glm::inverse;
using glm::transpose;
using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Program> prog_blinn;
shared_ptr<Program> prog_rev;
shared_ptr<Shape> shape;
shared_ptr<Shape> teapot;
vector<Object> objects;
shared_ptr<Shape> plane;
shared_ptr<Shape> sphere;
map<string,GLuint> sphereIDs;
map<string,GLuint> revIDS;
int indCount = 0;


double RANDOM_COLORS[7][3] = {
	{0.0000,    0.4470,    0.7410},
	{0.8500,    0.3250,    0.0980},
	{0.9290,    0.6940,    0.1250},
	{0.4940,    0.1840,    0.5560},
	{0.4660,    0.6740,    0.1880},
	{0.3010,    0.7450,    0.9330},
	{0.6350,    0.0780,    0.1840},
};

bool keyToggles[256] = {false}; // only for English keyboards!

struct Light {
	glm::vec3 color; 
	glm::vec3 pos;

	Light() {

	}
};


// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	camera->mouseMoved((float)xmouse, (float)ymouse);
}

float speed = 1.0f;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	keyToggles[key] = !keyToggles[key];
	glm::vec3 movement_vec(0.0f, 0.0f, 0.0f);
	int a_state = glfwGetKey(window, GLFW_KEY_A);
	int w_state = glfwGetKey(window, GLFW_KEY_W);
	int s_state = glfwGetKey(window, GLFW_KEY_S);
	int d_state = glfwGetKey(window, GLFW_KEY_D);

	if (key == GLFW_KEY_A && a_state == GLFW_PRESS) { // a
		movement_vec.x -= 1.0f;
	} 

	if (key == GLFW_KEY_D && d_state == GLFW_PRESS) { // d
		movement_vec.x += 1.0f;
	} 

	if (key == GLFW_KEY_S && s_state == GLFW_PRESS) { // s
		movement_vec.z -= 1.0f;
	} 

	if (key == GLFW_KEY_W && w_state == GLFW_PRESS) { // w
		movement_vec.z += 1.0f;
	}

	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	camera->move(movement_vec);
}

static void char_callback(GLFWwindow *window, unsigned int key) {
	if (key == 90) {
		camera->zoom(false);
	} else if (key == 122) {
		camera->zoom(true);
	}
}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char *filepath, GLFWwindow *w)
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if(rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

int light_count = 10;
glm::vec3 lights_pos[10];
glm::vec3 lights_col[10];
float light_rot[10];

// This function is called once to initialize the scene and OpenGL
static void init()
{

	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	prog_blinn = make_shared<Program>();
	prog_blinn->setShaderNames(RESOURCE_DIR + "blinn_vert.glsl", RESOURCE_DIR + "blinn_frag.glsl");
	prog_blinn->init();
	prog_blinn->addUniform("ke");
	prog_blinn->addUniform("MVit");
	prog_blinn->addUniform("kd");
	prog_blinn->addUniform("ks");
	prog_blinn->addUniform("s");
	prog_blinn->addUniform("MV");
	prog_blinn->addUniform("P");
	prog_blinn->addUniform("lights_pos");
	prog_blinn->addUniform("lights_col");
	prog_blinn->addAttribute("aPos");
	prog_blinn->addAttribute("aNor");
	prog_blinn->addAttribute("aTex");
	prog_blinn->setVerbose(true);

	prog_rev = make_shared<Program>();
	prog_rev->setShaderNames(RESOURCE_DIR + "blinn_vert_rev.glsl", RESOURCE_DIR + "blinn_frag_rev.glsl");
	prog_rev->init();
	prog_rev->addUniform("ke");
	prog_rev->addUniform("MVit");
	prog_rev->addUniform("kd");
	prog_rev->addUniform("ks");
	prog_rev->addUniform("s");
	prog_rev->addUniform("MV");
	prog_rev->addUniform("P");
	prog_rev->addUniform("lights_pos");
	prog_rev->addUniform("lights_col");
	prog_rev->addUniform("t");
	prog_rev->addAttribute("aPos");
	prog_rev->addAttribute("aTex");
	prog_rev->setVerbose(true);

	prog = prog_blinn;
	
	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f); // Camera's initial Z translation
	
	shape = make_shared<Shape>();
	shape->loadMesh(RESOURCE_DIR + "bunny.obj");
	shape->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();

	plane = make_shared<Shape>();
	plane->loadMesh(RESOURCE_DIR + "cube.obj");
	plane->init();

	sphere = make_shared<Shape>();
	sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
	sphere->init();

	objects = vector<Object>();

	//MAKE LIGHTS 
	for (int i = 0; i < light_count; i++) {
		float pos = (i-5) / 1.5f + .5f;
		light_rot[i] = i + 1;
		lights_pos[i] = glm::vec3(pos, .2f, pos);
		lights_col[i] = glm::vec3(RANDOM_COLORS[i%7][0], RANDOM_COLORS[i%7][1], RANDOM_COLORS[i%7][2]);
	}

	std::random_device rd;     // only used once to initialise (seed) engine
	std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
	std::uniform_int_distribution<int> uni(-1, 1); // guaranteed unbiased

	// init sphere 
	vector<float> posBuf;
	vector<float> norBuf;
	vector<float> texBuf;
	vector<unsigned int> indBuf;
	
	int rows = 50;
	int cols =  50;
	int triangleCount = rows * cols - 1;
	float theta_interval = M_PI / (rows - 1);
	float phi_int = (2*M_PI) / (cols-1);
	float interval = 1.0f / (rows - 1);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			float theta = theta_interval * i;
			float phi = phi_int * j;
			posBuf.push_back(sinf(theta) * sinf(phi));
			posBuf.push_back(cosf(theta));
			posBuf.push_back(sinf(theta) * cosf(phi));
			norBuf.push_back(sinf(theta) * sinf(phi));
			norBuf.push_back(cosf(theta));
			norBuf.push_back(sinf(theta) * cosf(phi));
		}
	}

	for (int i = 0; i < rows - 1; i++) {
		for (int j = 0; j < cols - 1; j++) {
			int index = rows * i + j;
			indBuf.push_back(index);
			indBuf.push_back(index+1);
			indBuf.push_back(index+cols+1);
			indBuf.push_back(index);
			indBuf.push_back(index+cols+1);
			indBuf.push_back(index+cols);
		}
	}

	indCount = (int)indBuf.size();
		
	// Generate buffer IDs and put them in the bufIDs map.
	GLuint tmp[4];
	glGenBuffers(4, tmp);
	sphereIDs["bPos"] = tmp[0];
	sphereIDs["bNor"] = tmp[1];
	sphereIDs["bTex"] = tmp[2];
	sphereIDs["bInd"] = tmp[3];
	glBindBuffer(GL_ARRAY_BUFFER, sphereIDs["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, sphereIDs["bNor"]);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, sphereIDs["bTex"]);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIDs["bInd"]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(norBuf.size() == posBuf.size());

	// init surface of revolution
	posBuf.clear();
	indBuf.clear();
	theta_interval = (2 * M_PI) / (rows-1);
	interval = 10.0f / (rows - 1);
	for (int i = 0; i < rows; i++) {
		float x = interval * i;
		for (int j = 0; j < cols; j++) {
			float theta = theta_interval * j;
			posBuf.push_back(x);
			posBuf.push_back(theta);
			posBuf.push_back(0.0f);
		}
	}

	for (int i = 0; i < rows - 1; i++) {
		for (int j = 0; j < cols - 1; j++) {
			int index = rows * i + j;
			indBuf.push_back(index);
			indBuf.push_back(index+1);
			indBuf.push_back(index+cols+1);
			indBuf.push_back(index);
			indBuf.push_back(index+cols+1);
			indBuf.push_back(index+cols);
		}
	}

	indCount = (int)indBuf.size();
		
	// Generate buffer IDs and put them in the bufIDs map.
	GLuint tmp2[3];
	glGenBuffers(3, tmp2);
	revIDS["bPos"] = tmp2[0];
	revIDS["bTex"] = tmp2[1];
	revIDS["bInd"] = tmp2[2];
	glBindBuffer(GL_ARRAY_BUFFER, revIDS["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, revIDS["bInd"]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// init objects 
	glm::vec3 transform(0,0,0);
	bool scale;
	bool rotate;
	glm::vec3 ke;
	glm::vec3 kd;
	glm::vec3 ks(1.0, 1.0, 1.0);
	float s = 10.0f;
	shared_ptr<Shape> spawn_shape = shape;
	int count = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			transform = glm::vec3((i - 5), 0, (j-5));
			scale = false;
			ke = glm::vec3(0,0,0); 
			kd = glm::vec3(RANDOM_COLORS[count % 6][0], RANDOM_COLORS[count % 6][1], RANDOM_COLORS[count % 6][2]);
			rotate = false;
			bool shear = false;
			if ((i * 10 + j) % 2 == 0) {
				spawn_shape = teapot;
				shear = true;
			} else {
				spawn_shape = shape;
				rotate = true;
			}
			srand(i);
			float initScaleDown = (rand() % 5 / 10.0f) + .1f;
			Object o = Object(transform, rotate, scale, shear, ke, kd, ks, s, spawn_shape, initScaleDown);
			objects.push_back(o);
			count += 1;
		}
	}
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	glViewport(0, 0, width, height); 
	
	double t = glfwGetTime();
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

	prog->bind();
	// Apply camera transforms
	P->pushMatrix();
	MV->pushMatrix();

	camera->applyProjectionMatrix(P, false);
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

	camera->applyViewMatrix(MV);

	glm::vec3 lights_world[light_count];
	for (int i = 0; i < light_count; i++) {
		MV->pushMatrix();
		MV->rotate(t/light_rot[i], glm::vec3(0,1,0));
		glm::vec4 pos = MV->topMatrix() * glm::vec4(lights_pos[i].x,lights_pos[i].y,lights_pos[i].z,1);
		MV->popMatrix();
		lights_world[i] = glm::vec3(pos.x, pos.y, pos.z);
	}


	// light rendering
	glm::vec3 pos(0,0,0);
	glUniform3fv(prog->getUniform("lights_pos"), light_count, glm::value_ptr(lights_world[0]));
	glUniform3fv(prog->getUniform("lights_col"), light_count, glm::value_ptr(lights_col[0]));

	for (int i = 0; i < light_count; i++) {
		pos = lights_pos[i];
		MV->pushMatrix();
		MV->rotate(t/light_rot[i], glm::vec3(0,1,0));
		MV->translate(pos);
		MV->scale(.1f);
		// glm::vec4 new_pos = MV->topMatrix() * glm::vec4(1.0f,1.0f,1.0f,1.0f);
		// lights_pos[i] = glm::vec3(new_pos.x, new_pos.y, new_pos.z);
		glUniform3f(prog->getUniform("ke"), lights_col[i].r, lights_col[i].g, lights_col[i].b);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
		sphere->draw(prog);
		MV->popMatrix();
	}

	MV->pushMatrix();
	MV->translate(glm::vec3(0, -.5f, 0));
	MV->scale(glm::vec3(100, 1, 100));
	glUniform1f(prog->getUniform("s"), 10.0f);
	glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, &MV->topMatrix()[0][0]);
	glUniform3f(prog->getUniform("ke"), 0,0,0);
	glUniformMatrix4fv(prog->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
    plane->draw(prog);
	MV->popMatrix();

	for (int i = 0; i < objects.size(); i++) {
		objects.at(i).render(MV, prog);
	}

	// bouncy boys 
	for (int i = 0; i < 10; i++) {
		MV->pushMatrix();
		float y = .7f * (0.5f * sinf(2*M_PI/3.0f*(t + 0.9f)) + .5f);
		float s = -.5f * (0.5f * cosf(4*M_PI/3.0f*(t + .9f)) + .5f) + 1.0f;
		srand(i);
		MV->translate(glm::vec3(rand() % 10 - 5 + .5f ,y+.05f,rand() % 10 - 5 + .5f));
		MV->scale(glm::vec3(s * .1f,.1f,s * .1f));
		glEnableVertexAttribArray(prog->getAttribute("aPos"));
		glEnableVertexAttribArray(prog->getAttribute("aNor"));
		glBindBuffer(GL_ARRAY_BUFFER, sphereIDs["bPos"]);
		glVertexAttribPointer(prog->getAttribute("aPos"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindBuffer(GL_ARRAY_BUFFER, sphereIDs["bNor"]);
		glVertexAttribPointer(prog->getAttribute("aNor"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIDs["bInd"]);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
		glUniform3f(prog->getUniform("ke"), 0, 0, 0);
		srand(i);
		float color = rand() % 10 / 10.0f + .4f;
		glUniform3f(prog->getUniform("kd"), color, color, color);    
		glUniform3f(prog->getUniform("ks"), 1.0f, 1.0f, 1.0f);
		glUniform1f(prog->getUniform("s"), 10.0f);
		glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_INT, (void *)0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(prog->getAttribute("aNor"));
		glDisableVertexAttribArray(prog->getAttribute("aPos"));
		MV->popMatrix();
	}
	prog->unbind();
	prog = prog_rev;
	prog->bind();
	glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
	glUniform3fv(prog->getUniform("lights_pos"), light_count, glm::value_ptr(lights_world[0]));
	glUniform3fv(prog->getUniform("lights_col"), light_count, glm::value_ptr(lights_col[0]));

	// rev 
	for (int i = 0; i < 10; i++) {
		MV->pushMatrix();
		srand(i+10);
		MV->translate(glm::vec3(rand() % 10 - 5 + .5f ,0,rand() % 10 - 5 + .5f));
		MV->scale(.05f);
		MV->rotate(M_PI/2, glm::vec3(0,0,1));
		glEnableVertexAttribArray(prog->getAttribute("aPos"));
		glBindBuffer(GL_ARRAY_BUFFER, revIDS["bPos"]);
		glVertexAttribPointer(prog->getAttribute("aPos"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, revIDS["bInd"]);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
		glUniformMatrix4fv(prog->getUniform("MVit"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
		glUniform3f(prog->getUniform("ke"), 0, 0, 0);
		float color = rand() % 10 / 10.0f + .4f;
		glUniform3f(prog->getUniform("kd"), RANDOM_COLORS[i%7][0], RANDOM_COLORS[i%7][1], RANDOM_COLORS[i%7][2]); 
		glUniform1f(prog->getUniform("t"), t);     
		glUniform3f(prog->getUniform("ks"), 1.0f, 1.0f, 1.0f);
		glUniform1f(prog->getUniform("s"), 10.0f);
		glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_INT, (void *)0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(prog->getAttribute("aPos"));
		MV->popMatrix();
	}
	prog->unbind();
	prog = prog_blinn;

	MV->popMatrix();
	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);
	
	if(OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Usage: A3 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Optional argument
	if(argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
