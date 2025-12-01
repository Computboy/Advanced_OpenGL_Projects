#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 model;
uniform mat3 NormalMatrix;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos   = worldPos.xyz;
    vs_out.Normal    = NormalMatrix * aNormal;
    vs_out.TexCoords = aTexCoords;

    gl_Position = projection * view * worldPos;
}

// model.h中已经设置好了Uniform变量读入，包括漫反射材质、镜面光贴图、法线等
// 我们只需要在片段着色器中去声明这些采样器，并采样使用它们，就能直接用上这些贴图。