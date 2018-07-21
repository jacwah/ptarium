#include "maths.h"
#include <math.h>

glm::vec3
SphericalToCartesian(glm::vec2 Spherical)
{
    float SinYaw = sinf(Spherical.x);
    float CosYaw = cosf(Spherical.x);
    float SinPitch = sinf(Spherical.y);
    float CosPitch = cosf(Spherical.y);

    return glm::vec3(
            SinPitch * CosYaw,
            CosPitch,
            SinPitch * SinYaw);
}

/* 1 in front, 0 none, -1 behind. */
int
LineSphereIntersect(
        glm::vec3 SphereCenter,
        float SphereRadius,
        glm::vec3 LineOrigin,
        glm::vec3 LineDirection) // Normalized
{
    glm::vec3 SphereToLine = LineOrigin - SphereCenter;
    float LineProject = glm::dot(LineDirection, SphereToLine);
    float PointDiffSquared = LineProject*LineProject - glm::dot(SphereToLine, SphereToLine) + SphereRadius*SphereRadius;

    if (PointDiffSquared < 0.0f) {
        return 0;
    } else {
        float GreatestDistance = -LineProject + sqrtf(PointDiffSquared);

        if (GreatestDistance < 0.0f) {
            return -1;
        } else {
            return 1;
        }
    }
}

