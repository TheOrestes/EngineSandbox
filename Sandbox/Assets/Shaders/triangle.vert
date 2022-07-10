#version 450

//-- Input from Program
layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Col;

//-- Output to Fragment shader
layout(location = 0) out vec3 fragCol;

void main()
{
    gl_Position = vec4(Pos, 1.0f);
    fragCol = Col;
}