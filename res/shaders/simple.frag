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

void main()
{
    vec3 normal = normalize(normal_in);

    vec3 ambientColor = vec3(0.2, 0.2, 0.2);
    vec3 surfaceColor = vec3(0.3, 0.3, 0.3);

    vec3 resultColor = ambientColor;

    for (int i = 0; i < lightsCount; i++) {
        vec3 lightPos = lights[i];
        vec3 lightDir = normalize(lightPos - fragPos);

        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * surfaceColor;

        resultColor += diffuse;
    }

    color = vec4(resultColor, 1.0);
}
