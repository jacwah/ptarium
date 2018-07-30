#pragma once

#include <glm/mat4x4.hpp>

struct camera {
    glm::vec3 Position;
    glm::vec3 LookVector;
    glm::vec2 HalfScreen;
    glm::mat4 FullTransform;
    glm::mat4 InvCameraTransform;

    glm::vec3 WorldDirectionFromScreen(glm::vec2 ScreenPoint);
};

struct camera_params {
    float FovY;
    float AspectRatio;
    float Distance;
    float NearDistance;
    glm::vec2 Orientation;
    glm::vec3 Focus;

    camera MakeCamera();
};

