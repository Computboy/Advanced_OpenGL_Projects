#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;
// 以顶点着色器的输出作为的片段着色器输入

// 来自 model.h 绑定的纹理名
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

// 相机位置
uniform vec3 viewPos;

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;      // 内圆锥 cos 值
    float outerCutOff; // 外圆锥 cos 值

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform SpotLight spot;

uniform DirectionLight dirLight;

vec3 CalcDirLight(DirectionLight light, vec3 normal, vec3 viewDir);
void main()
{
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    // 采样漫反射 / 镜面贴图
    vec3 albedo    = texture(texture_diffuse1,  fs_in.TexCoords).rgb;
    vec3 specColor = texture(texture_specular1, fs_in.TexCoords).rgb;

    // 光照方向（spot.position → frag）
    vec3 lightDir = normalize(spot.position - fs_in.FragPos);

    // 计算 spotlight 强度（柔性边缘）
    float theta   = dot(lightDir, normalize(-spot.direction)); // 光方向与-spotDir
    float epsilon = spot.cutOff - spot.outerCutOff;
    float intensity = clamp((theta - spot.outerCutOff) / epsilon, 0.0, 1.0);

    // 基本漫反射
    float diff = max(dot(norm, lightDir), 0.0);

    // 镜面 (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);

    // 距离衰减
    float distance    = length(spot.position - fs_in.FragPos);
    float attenuation = 1.0 /
        (spot.constant + spot.linear * distance + spot.quadratic * distance * distance);

    // 环境光 / 漫反射 / 镜面
    vec3 ambient  = spot.ambient  * albedo;
    vec3 diffuse  = spot.diffuse  * diff * albedo;
    vec3 specular = spot.specular * spec * specColor;

    // 应用聚光强度 & 衰减
    diffuse  *= intensity;
    specular *= intensity;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    result += CalcDirLight(dirLight, norm, viewDir);
    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirectionLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    // 合并结果
    vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, fs_in.TexCoords));
    return (ambient + diffuse + specular);
}