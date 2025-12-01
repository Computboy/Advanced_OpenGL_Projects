// 把 G-Buffer 换成 MSAA FBO 只是在固定管线层面把附件改为多重采样纹理；
// Geometry Pass 的顶点和片元着色器不需要修改，仍然按单样本方式输出 gPosition / gNormal / gAlbedoSpec，由硬件负责将每个 fragment 的输出写入多个 sample

#version 330 core
layout (location = 0) out vec3 gPositionMS;
layout (location = 1) out vec3 gNormalMS;
layout (location = 2) out vec4 gAlbedoSpecMS;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPositionMS = FragPos;
    // also store the per-fragment normals into the gbuffer
    gNormalMS = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedoSpecMS.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpecMS.a = texture(texture_specular1, TexCoords).r;
}