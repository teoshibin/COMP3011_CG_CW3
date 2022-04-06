#pragma once

#include <stdio.h>
#include <glm/glm/glm.hpp>
#include "camera.h"

float cam_dist = 2.f;

void MoveAndOrientCamera(SCamera& in, glm::vec3 target, float distance, float xoffset, float yoffset)
{

	if (cam_dist < 2.f)
	{
		cam_dist = 2.f;
	}
	else if (cam_dist > 5.f)
	{
		cam_dist = 5.f;
	}

	in.Yaw -= xoffset * in.MovementSpeed;
	in.Pitch -= yoffset * in.MovementSpeed;

	if (in.Pitch > 89.0f) in.Pitch = 89.0f;
	if (in.Pitch < -89.0f) in.Pitch = -89.0f;

	in.Position.x = cos(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch)) * cam_dist;
	in.Position.y = sin(glm::radians(in.Pitch)) * cam_dist;
	in.Position.z = sin(glm::radians(in.Yaw)) * cos(glm::radians(in.Pitch)) * cam_dist;
	in.Front = glm::normalize(target - in.Position);
	in.Right = glm::normalize(glm::cross(in.Front, in.WorldUp));
	in.Up = glm::normalize(glm::cross(in.Right, in.Front));
}
