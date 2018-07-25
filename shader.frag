#version 330 core

in vec3 WorldPosition;

/*layout(location = 0)*/ out vec3 Color;

void main()
{
    //Color = vec3(0.25) + 0.75 * WorldPosition * WorldPosition;
    Color = normalize(abs(WorldPosition));
    //Color = vec3(1.0, 1.0, 1.0);
}
