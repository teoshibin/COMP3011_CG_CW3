
#include "GeneralCamera.h"

GeneralCamera::GeneralCamera()
{
	front = glm::vec3(0.0f, 0.0f, -1.0f);
	position = glm::vec3(0.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
	worldUp = up;
	right = glm::normalize(glm::cross(front, worldUp));
}

void GeneralCamera::moveCamera(CameraMovement direction)
{
	if (direction == CameraMovement::FORWARD) position += calculateHorizontalFront() * movementSpeed;
	if (direction == CameraMovement::BACKWARD) position -= calculateHorizontalFront() * movementSpeed;
	if (direction == CameraMovement::LEFT) position -= right * movementSpeed;
	if (direction == CameraMovement::RIGHT) position += right * movementSpeed;
	if (direction == CameraMovement::UPWARD) position += worldUp * movementSpeed;
	if (direction == CameraMovement::DOWNWARD) position -= worldUp * movementSpeed;
}

void GeneralCamera::orientCamera(float xoffset, float yoffset)
{
	yaw += xoffset * mouseSensitivity;
	pitch -= yoffset * mouseSensitivity;

	if (pitch > 89.f) pitch = 89.f;
	if (pitch < -89.f) pitch = -89.f;

	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	
	front = glm::normalize(newFront);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void GeneralCamera::moveAndOrientCamera(glm::vec3 target, float xoffset, float yoffset)
{

	if (modelViewDistance < 1.f) modelViewDistance = 1.f;
	else if (modelViewDistance > 99999.f) modelViewDistance = 99999.f;

	yaw -= xoffset * modelViewSpeed;
	pitch -= yoffset * modelViewSpeed;

	if (pitch > 89.f) pitch = 89.f;
	if (pitch < -89.f) pitch = -89.f;

	position.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)) * modelViewDistance + target.x;
	position.y = sin(glm::radians(pitch)) * modelViewDistance + target.y;
	position.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch)) * modelViewDistance + target.z;
	
	front = glm::normalize(target - position);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

glm::vec3 GeneralCamera::calculateHorizontalFront()
{
	return glm::normalize(glm::cross(worldUp, right));
}

glm::vec3 GeneralCamera::getPosition()
{
	return position;
}

float GeneralCamera::getFOV()
{
	return FOV;
}

void GeneralCamera::increaseFOV(float value)
{
	float newValue = FOV + value;
	if (newValue < 130)	FOV = newValue;
}

void GeneralCamera::decreaseFOV(float value)
{
	float newValue = FOV - value;
	if (newValue > 15) FOV = newValue;
}

void GeneralCamera::increaseModelViewDistance(float value)
{
	modelViewDistance += value;
}

void GeneralCamera::decreaseModelViewDistance(float value)
{
	modelViewDistance -= value;
}

void GeneralCamera::setModelViewDistance(float value)
{
	modelViewDistance = value;
}

float GeneralCamera::getModelViewDistance()
{
	return modelViewDistance;
}

float GeneralCamera::getYaw()
{
	return yaw;
}

float GeneralCamera::getPitch()
{
	return pitch;
}

void GeneralCamera::setYaw(float value)
{
	yaw = value;
}

glm::vec3 GeneralCamera::getFront()
{
	return front;
}

glm::vec3 GeneralCamera::getUp()
{
	return up;
}
