#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;
in layout(location = 3) vec3 tangent_in;
in layout(location = 4) vec3 bitangent_in;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 fragPos_out;
out layout(location = 3) mat3 TBN;

uniform mat4 M;
uniform mat4 VP;
uniform mat3 N;

void main()
{
    normal_out = normalize(N * normal_in);
    textureCoordinates_out = textureCoordinates_in;
    vec3 T = normalize(N * tangent_in);
    vec3 B = normalize(N * bitangent_in);
    TBN = mat3(T, B, normal_out);

    vec4 modelPos = M * vec4(position, 1.0f);
    fragPos_out = vec3(modelPos);

    gl_Position = VP * modelPos;
}
