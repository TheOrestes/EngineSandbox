#version 450

//---------------------------------------------------------------------------------------------------------------------
//-- Input from Program
layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec2 in_UV;

//---------------------------------------------------------------------------------------------------------------------
//-- Output to Fragment shader
layout(location = 0) out vec2 vs_outUV;

//---------------------------------------------------------------------------------------------------------------------
//-- Uniforms
layout(set = 0, binding = 0) uniform mvpData
{
    mat4 World;
    mat4 View;
    mat4 Projection;

    vec4 albedoColor;
    vec4 emissionColor;
    vec4 hasTextureAEN;
    vec4 hasTextureRMO;
    float occlusion;
    float roughness;
    float metalness;

}shaderData;

//---------------------------------------------------------------------------------------------------------------------
void main()
{
    gl_Position = shaderData.Projection * shaderData.View * shaderData.World * vec4(in_Pos, 1.0f);
    vs_outUV = in_UV;
}