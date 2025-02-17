#version 430 core

#define MAX_LIGHTS 3

in layout(location = 0) vec3 normal;
in layout(location = 1) vec2 textureCoordinates;

out vec4 color;

float rand(vec2 co) { return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453); }
float dither(vec2 uv) { return (rand(uv)*2.0-1.0) / 256.0; }

uniform int lightsCount;
uniform vec3 lights[MAX_LIGHTS];

void main()
{
    vec3 ambientColor = vec3(0.2, 0.2, 0.2);
    vec3 normalizedNormal = normalize(normal);
    color = vec4(0.5 * normalizedNormal + 0.5, 1.0);
}
