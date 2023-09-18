#include <string>
#include <vector>

#include "Camera.h"

Camera::Camera()
{
}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
{
	currentTime = 0.0f;
	deltaTime = 0.0f;
	lastTime = 0.0f;
	totalTime = 0.0f;

	pos1 = glm::vec3(0.0f, 5.0f, 4.0f);
	pos2 = glm::vec3(-4.0f, 5.0f, -4.0f);
	pos3 = glm::vec3(4.0f, 5.0f, 4.0f);
	isStatic = false;
	
	//position = startPosition;
	position = pos1;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	worldOrigin = glm::vec3(0.0f, 0.0f, 0.0f);

	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;

	update();
}

glm::vec3 Camera::getCameraPosition()
{
	return position;
}

void Camera::keyControl(bool* keys, GLfloat inDeltaTime)
{
	#pragma region CODE FOR PART 2 -- PLEASE IGNORE
	//// Toggle between static and free-look
	//currentTime = glfwGetTime();
	//deltaTime = currentTime - lastTime;
	//lastTime = currentTime;
	//totalTime += deltaTime;

	//if (keys[GLFW_KEY_TAB] && totalTime >= 0.25f)
	//{
	//	totalTime = 0.0f;

	//	isStatic = !isStatic;
	//	position = pos1;
	//}
	#pragma endregion

	isStatic = true;

	// Camera is in static mode
	if (isStatic)
	{
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
		totalTime += deltaTime;

		if (keys[GLFW_KEY_LEFT] && totalTime >= 0.25f)
		{
			totalTime = 0.0f;

			changePosition(true);
		}
		else if (keys[GLFW_KEY_RIGHT] && totalTime >= 0.25f)
		{
			totalTime = 0.0f;

			changePosition(false);
		}
	}
	// Camera is in free-look mode
	else
	{
		GLfloat velocity = moveSpeed * inDeltaTime;

		if (keys[GLFW_KEY_W])
		{
			position += front * velocity;
		}

		if (keys[GLFW_KEY_S])
		{
			position -= front * velocity;
		}

		if (keys[GLFW_KEY_A])
		{
			position -= right * velocity;
		}

		if (keys[GLFW_KEY_D])
		{
			position += right * velocity;
		}
	}
}

void Camera::mouseControl(GLfloat xChange, GLfloat yChange)
{
	// Camera is in free-look mode
	if (!isStatic)
	{
		xChange *= turnSpeed;
		yChange *= turnSpeed;

		yaw += xChange;
		pitch += yChange;

		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}

		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}
	}

	update();
}

void Camera::changePosition(bool isLeft)
{
	// Cycle to the left
	if (isLeft)
	{
		if (equals(position, pos1))
		{
			position = pos3;
		}
		else if (equals(position, pos2))
		{
			position = pos1;
		}
		else if (equals(position, pos3))
		{
			position = pos2;
		}
	}
	// Cycle to the right
	else
	{
		if (equals(position, pos1))
		{
			position = pos2;
		}
		else if (equals(position, pos2))
		{
			position = pos3;
		}
		else if (equals(position, pos3))
		{
			position = pos1;
		}
	}
}

bool Camera::equals(glm::vec3 v1, glm::vec3 v2)
{
	if (v1.x == v2.x &&
		v1.y == v2.y &&
		v1.z == v2.z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

glm::mat4 Camera::calculateViewMatrix()
{
	if (isStatic)
	{
		return glm::lookAt(position, worldOrigin, worldUp);
	}
	else
	{
		return glm::lookAt(position, position + front, worldUp);
	}
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

Camera::~Camera()
{
}