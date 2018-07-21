#pragma once

#include <glm/glm.hpp>

glm::vec3 SphericalToCartesian(glm::vec2 Spherical);
int LineSphereIntersect(
        glm::vec3 SphereCenter,
        float SphereRadius,
        glm::vec3 LineOrigin,
        glm::vec3 LineDirection);
