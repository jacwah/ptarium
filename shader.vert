#version 330 core

layout(location = 0) in vec3 Point;

out vec3 WorldPosition;

uniform mat4 Transform;

void main()
{
    gl_Position = Transform * vec4(Point, 1.0);
    WorldPosition = Point;
}
