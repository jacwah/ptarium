#pragma once

#include <glm/vec3.hpp>

#define MAX_BODY 100
#define MAX_NAME 20

struct world {
	int Count;
	char Name[MAX_BODY][MAX_NAME];
	float Radius[MAX_BODY];
	float Mass[MAX_BODY];
	glm::vec3 Position[MAX_BODY];
	glm::vec3 Velocity[MAX_BODY];
};
