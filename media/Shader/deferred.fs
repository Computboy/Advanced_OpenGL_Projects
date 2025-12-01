#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DMS gPosition;
uniform sampler2DMS gNormal;
uniform sampler2DMS gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
};
const int NR_LIGHTS = 64;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

const int MSAA_SAMPLES = 4;

void main()
{             
    // // retrieve data from gbuffer
    // vec3 FragPos = texture(gPosition, TexCoords).rgb;
    // vec3 Normal = texture(gNormal, TexCoords).rgb;
    // vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    // float Specular = texture(gAlbedoSpec, TexCoords).a;

    // 多重采样的方式获取数据
    ivec2 coord = ivec2(gl_FragCoord.xy);

    vec3 FragPos   = vec3(0.0);
    vec3 Normal    = vec3(0.0);
    vec3 Diffuse   = vec3(0.0);
    float Specular =       0.0;

    for (int i = 0; i < MSAA_SAMPLES; ++i) {
        FragPos  += texelFetch(gPosition, coord, i).xyz;
        Normal   += texelFetch(gNormal, coord, i).xyz;
        vec4 albSpec = texelFetch(gAlbedoSpec, coord, i);
        Diffuse  += albSpec.rgb;
        Specular += albSpec.a;
    }

    FragPos /= float(MSAA_SAMPLES);
    Normal  = normalize(Normal / float(MSAA_SAMPLES));
    Diffuse /= float(MSAA_SAMPLES);
    Specular   /= float(MSAA_SAMPLES);
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lights[i].Color * spec * Specular;
        // attenuation
        float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;        
    }
    FragColor = vec4(lighting, 1.0);
}