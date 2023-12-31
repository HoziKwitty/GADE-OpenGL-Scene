#pragma once

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <GLFW\glfw3.h>

class Camera
{
public:
	Camera();

	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	void keyControl(bool* keys, GLfloat inDeltaTime);
	void mouseControl(GLfloat xChange, GLfloat yChange);
	void changePosition(bool isLeft);

	bool equals(glm::vec3 v1, glm::vec3 v2);

	glm::vec3 getCameraPosition();
	glm::mat4 calculateViewMatrix();

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	glm::vec3 worldOrigin;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat moveSpeed;
	GLfloat turnSpeed;

	glm::vec3 pos1;
	glm::vec3 pos2;
	glm::vec3 pos3;
	bool isStatic;

	GLfloat currentTime;
	GLfloat deltaTime;
	GLfloat lastTime;
	GLfloat totalTime;

	void update();
};