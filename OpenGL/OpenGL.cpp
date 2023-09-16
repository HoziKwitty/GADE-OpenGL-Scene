#pragma region Camera

#include <string.h>
#include <stdio.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // pointer used to pass the value to the shader

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"

// formula to convert degrees to radians.
// multiply any degree value by this to get it 
// in radians
const float degreeToRadians = 3.14159265f / 180.0f;

//GLuint shader, uniformModel, uniformProjection;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

Camera camera;

GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// The model matrix will allow us to move from just model model coordinates where it's built around the
// origin to the actual world, coordinates where they are actually in the world.
// Each time we go through the loop, it will
// Increment by the offset by triIncrement
// When it reaches triMaxOffset, we will switch direction repeat
bool direction = true;

float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.0005f;

//for rotation
float curAngle = 0.0f; //current angle

//for scaling
bool sizeDirection = true;
float currSize = 0.4f;
float maxSize = 0.8f;
float minSize = 0.1f;

#pragma region Vertex&Fragment
// creating vertex shader 
// taking each point that you pass into the vertices
// allows you to do things with the values (such as display or move around)
// allows us to modify the vertices values and pass it to the fragment shader
// vec4(0.4 * pos.x, 0.4 * pos.y, pos.z, 1.0); final pos values of the vertices on screen

// creating a uniform 4x4 matrix called model
// model is the identity matrix,
static const char* vShader = "Shaders/shader.vert";
// fragment shader
// handling each pixel on the screen
// how each value works with the vertices
	// out vec4 colour (the out value for the colour values for each pixel)
	// colour = vec4(1.0, 0.0, 0.0, 1.0) -  setting the colour values (RGBa)
static const char* fShader = "Shaders/shader.frag";
#pragma endregion

void CreateObjects()
{
	// Placing the points in a specific order
	unsigned int indices[] =
	{
		0, 2, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	// Array with GLfloats
	// Points that make up the triangle
	GLfloat vertices[] =
	{
		-1.0f, -1.0f, 0.0f,
		0.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	Mesh* obj1 = new Mesh();
	obj1->createMesh(vertices, indices, 12, 12);
	meshList.push_back(obj1);
}

void CreateShaders()
{
	Shader* shader1 = new Shader();
	shader1->CreateFromFiles(vShader, fShader);
	shaderList.push_back(shader1);
}

int main()
{
	GLuint uniformProjection = 0, uniformModel = 0, uniformView = 0;

	mainWindow = Window(800, 600);
	mainWindow.initialise();

	CreateObjects();
	CreateShaders();

	camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.5f);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

	// Loop until window closed
	while (!mainWindow.getShouldClose())
	{
		GLfloat now = glfwGetTime();
		deltaTime = now - lastTime;
		lastTime = now;

		// Get and Handle user input events
		glfwPollEvents();

		camera.keyControl(mainWindow.getsKeys(), deltaTime);
		camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

		// Check if direction is going to the right
		if (direction)
		{
			triOffset += triIncrement; //right

		}
		else
		{
			triOffset -= triIncrement; //left
		}

		// When it reaches the maxOffset we want to switch directions
		if (abs(triOffset) >= triMaxOffset)
		{
			direction = !direction;
		}

		#pragma region matrix trans
		curAngle += 0.01f;

		if (curAngle >= 360)
		{
			curAngle -= 360;
		}

		// Adjusting the size of the triangle
		if (sizeDirection)
		{
			currSize += 0.0001f;
		}
		else
		{
			currSize -= 0.0001f;
		}

		if (currSize >= maxSize || currSize <= minSize)
		{
			sizeDirection = !sizeDirection;
		}
		#pragma endregion		

		//Setting the window colour
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		// Clear window
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderList[0]->useShader();

		uniformModel = shaderList[0]->getModelLocation();
		uniformProjection = shaderList[0]->getProjectionLocation();
		uniformView = shaderList[0]->getViewLocation();
		//glUseProgram(shader);

		// Creating the model matrix
		// 4x4 idenity matrix
		glm::mat4 model(1.0f);

		// Take the identiy matrix
		// Apply a translation to it by altering the x values
		// Top right value in the matrix will change to triOffSet (x values)
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		//model = glm::rotate(model, curAngle * degreeToRadians, glm::vec3(0.0f, 1.0f, 0.0f));

		// Going to hold a 4x4 matrix with 4 float values
		// Pass in the model matrix
		// 1 matrix and not transposing it (flipping)
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		// Projection
		glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

		// Camera
		glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

		meshList[0]->renderMesh();

		glUseProgram(0);

		mainWindow.swapBuffers();
	}

	return 0;
}
#pragma endregion