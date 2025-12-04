#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D texture1;
// 默认索引0激活

void main()
{    
    FragColor = texture(texture1, TexCoords);
}