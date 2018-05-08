#version 330 core

in vec3 WorldPosition;

/*layout(location = 0)*/ out vec3 Color;

void main()
{
    Color = abs(WorldPosition);
}
