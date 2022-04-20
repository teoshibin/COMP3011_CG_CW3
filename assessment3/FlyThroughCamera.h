#pragma once

#include <glm/glm/glm.hpp>
#include "camera.h"

glm::vec3 CalHorizontalFront(SCamera& in)
{
	return glm::normalize(glm::cross(in.WorldUp, in.Right));
}

void MoveCamera(SCamera& in, SCamera::Camera_Movement direction)
{
	if (direction == in.FORWARD) in.Position += CalHorizontalFront(in) * in.MovementSpeed;
	if (direction == in.BACKWARD) in.Position -= CalHorizontalFront(in) * in.MovementSpeed;
	if (direction == in.LEFT) in.Position -= in.Right * in.MovementSpeed;
	if (direction == in.RIGHT) in.Position += in.Right * in.MovementSpeed;
	if (direction == in.UPWARD) in.Position += in.WorldUp * in.MovementSpeed;
	if (direction == in.DOWNWARD) in.Position -= in.WorldUp * in.MovementSpeed;

}

void OrientCamera(SCamera& in, float xoffset, float yoffset)
{

	in.Yaw += xoffset * in.MouseSensitivity;
	in.Pitch -= yoffset * in.MouseSensitivity;


	if (in.Pitch > 89.0f) in.Pitch = 89.0f;
	if (in.Pitch < -89.0f) in.Pitch = -89.0f;
	
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
	newFront.y = sin(glm::radians(in.Pitch));
	newFront.z = sin(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch));
	in.Front = glm::normalize(newFront);

	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));
	in.Up = glm::normalize(glm::cross(in.Right, in.Front));

}
