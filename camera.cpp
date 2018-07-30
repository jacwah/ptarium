#include "camera.h"
#include "maths.h"
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>

const glm::vec3 Up(0.0f, 0.0f, 1.0f);

camera
camera_params::MakeCamera()
{
    camera Camera;
    float TanHalfFov = tanf(FovY / 2.0f);
    Camera.HalfScreen = glm::vec2(TanHalfFov * AspectRatio, TanHalfFov);

    Camera.Position = Distance * SphericalToCartesian(Orientation) + Focus;
#if 0
    glm::mat4 Perspective = glm::perspective(
            FovY,
            AspectRatio,
            0.1f,
            100000.0f);
#else
    glm::mat4 Perspective = glm::infinitePerspective(
            FovY,
            AspectRatio,
            NearDistance);
#endif
    glm::mat4 CameraTransform = glm::lookAt(Camera.Position, Focus, Up);
    Camera.FullTransform = Perspective * CameraTransform;
    Camera.InvCameraTransform = glm::inverse(CameraTransform);
    Camera.LookVector = glm::normalize(Focus - Camera.Position);

    return Camera;
}

glm::vec3
camera::WorldDirectionFromScreen(glm::vec2 ScreenPoint)
{
    glm::vec4 CameraPoint(
            HalfScreen.x * (2.0f * ScreenPoint.x - 1.0f),
            HalfScreen.y * (2.0f * ScreenPoint.y - 1.0f),
            -1.0f, 0.0f);
    return glm::normalize(InvCameraTransform * CameraPoint);
}
