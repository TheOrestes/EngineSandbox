#version 450

//-- Input from Program
layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Col;

//-- Output to Fragment shader
layout(location = 0) out vec3 fragCol;

//-- Uniforms
layout(binding = 0) uniform mvpData
{
    mat4 World;
    mat4 View;
    mat4 Projection;
}MVP;

void main()
{
    gl_Position = MVP.Projection * MVP.View * MVP.World * vec4(Pos, 1.0f);
    fragCol = Col;
}