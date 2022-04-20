#pragma once

#include <stdio.h>
#include <glm/glm/glm.hpp>

	enum class CameraMovement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UPWARD,
		DOWNWARD
	};

class GeneralCamera
{
public:

	GeneralCamera();

	// fly through camera
	void moveCamera(CameraMovement direction);
	void orientCamera(float xoffset, float yoffset);

	// model view camera
	void moveAndOrientCamera(glm::vec3 target, float xoffset, float yoffset);

	// getter setter
	glm::vec3 getPosition();
	glm::vec3 getFront();
	glm::vec3 getUp();
	float getFOV();
	void increaseFOV(float value);
	void decreaseFOV(float value);
	void increaseModelViewDistance(float value);
	void decreaseModelViewDistance(float value);
	void setModelViewDistance(float value);
	float getModelViewDistance();
private:

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	glm::vec3 worldUp;

	float yaw = -90.f;
	float pitch = 0.f;

	float movementSpeed = 8.f;
	float mouseSensitivity = .05f;
	float modelViewSpeed = 0.2f;
	float FOV = 70.f;
	float modelViewDistance = 2000.f;

	// fly through camera
	glm::vec3 calculateHorizontalFront();
};

