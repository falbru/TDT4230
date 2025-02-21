#version 430 core

#define MAX_LIGHTS 3

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;
in layout(location = 3) mat3 TBN;

layout(binding = 0) uniform sampler2D sampler;
layout(binding = 1) uniform sampler2D normalMap;
layout(binding = 2) uniform sampler2D roughnessMap;

out vec4 color;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}
float dither(vec2 uv)
{
    return (rand(uv) * 2.0 - 1.0) / 256.0;
}
vec3 reject(vec3 from, vec3 onto)
{
    return from - onto * dot(from, onto) / dot(onto, onto);
}

struct Light
{
    vec3 position;
    vec3 color;
};

uniform int lightsCount;
uniform Light lights[MAX_LIGHTS];
uniform vec3 cameraPos;

uniform float ballRadius;
uniform vec3 ballPos;

uniform bool is2D;
uniform bool useNM;

void main()
{
    if (is2D)
    {
        color = texture(sampler, textureCoordinates);
        return;
    }

    vec3 normal = normalize(normal_in);
    if (useNM)
    {
        normal = TBN * (texture(normalMap, textureCoordinates).xyz * 2.0 - 1.0);
    }

    vec3 ambientColor = vec3(0.1, 0.1, 0.1);

    vec3 resultColor = vec3(0.0);

    float specularIntensity = 0.3;
    if (useNM)
    {
        float roughness = length(texture(roughnessMap, textureCoordinates));
        specularIntensity = 5.0 / (roughness * roughness);
    }

    float l_a = 1;
    float l_b = 0.01009;
    float l_c = 0.00107;

    float softShadowRadius = 1.0;

    for (int i = 0; i < lightsCount; i++)
    {
        vec3 lightPos = lights[i].position;
        vec3 lightColor = lights[i].color;

        vec3 toLight = lightPos - fragPos;
        float lightDist = length(toLight);
        vec3 lightDir = normalize(toLight);

        vec3 toBall = ballPos - fragPos;
        float ballDist = length(toBall);

        float shadowFactor = 1.0;

        float rejection = length(reject(toBall, toLight));
        if (lightDist > ballDist && dot(toLight, toBall) > 0.0 && rejection < ballRadius + softShadowRadius)
        {
            shadowFactor = (rejection < ballRadius) ? 0.0 : mix(0.0, 1.0, (rejection - ballRadius) / softShadowRadius);
        }

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 reflection = reflect(-lightDir, normal);
        vec3 viewDir = normalize(cameraPos - fragPos);

        float spec = pow(max(dot(viewDir, reflection), 0.0), 32);
        vec3 specular = specularIntensity * spec * lightColor;

        float attenuation = 1.0 / (l_a + l_b * lightDist + l_c * lightDist * lightDist);

        resultColor += (diffuse + specular) * attenuation * shadowFactor;
    }

    resultColor += ambientColor;

    if (useNM)
    {
        resultColor *= texture(sampler, textureCoordinates).xyz;
    }

    color = vec4(resultColor + dither(textureCoordinates), 1.0);
}
