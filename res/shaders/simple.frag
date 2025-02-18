#version 430 core

#define MAX_LIGHTS 3

in layout(location = 0) vec3 normal_in;
in layout(location = 1) vec2 textureCoordinates;
in layout(location = 2) vec3 fragPos;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

uniform int lightsCount;
uniform vec3 lights[MAX_LIGHTS];
uniform vec3 cameraPos;

void main()
{
    vec3 normal = normalize(normal_in);

    vec3 ambientColor = vec3(0.3, 0.3, 0.3);
    vec3 lightColor = vec3(0.4, 0.4, 0.4);

    vec3 resultColor = vec3(0.0);

    float specularIntensity = 0.3;

    float l_a = 0.9;
    float l_b = 0.002;
    float l_c = 0.002;

    for (int i = 0; i < lightsCount; i++) {
        vec3 lightPos = lights[i];
        vec3 lightDir = normalize(lightPos - fragPos);
        float lightDist = length(lightPos - fragPos);

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;

        vec3 reflection = reflect(-lightDir, normal);
        vec3 viewDir = normalize(cameraPos - fragPos);

        float spec = pow(max(dot(viewDir, reflection), 0.0), 32);
        vec3 specular = specularIntensity * spec * lightColor;

        float attenuation = 1.0 / (l_a + l_b * lightDist + l_c * lightDist * lightDist);

        resultColor += (diffuse + specular) * attenuation;
    }

    resultColor += ambientColor;

    color = vec4(resultColor + dither(textureCoordinates), 1.0);
}
