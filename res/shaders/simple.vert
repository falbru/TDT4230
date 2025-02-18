#version 430 core

in layout(location = 0) vec3 position;
in layout(location = 1) vec3 normal_in;
in layout(location = 2) vec2 textureCoordinates_in;

out layout(location = 0) vec3 normal_out;
out layout(location = 1) vec2 textureCoordinates_out;
out layout(location = 2) vec3 fragPos_out;

uniform mat4 M;
uniform mat4 VP;
uniform mat3 N;

uniform bool is2D;

void main()
{
    normal_out = normalize(N * normal_in);
    textureCoordinates_out = textureCoordinates_in;

    vec4 modelPos = M * vec4(position, 1.0f);
    fragPos_out = vec3(modelPos);

    vec4 vertPos = modelPos;
    if (is2D)
    {
        vertPos = vec4(modelPos.x, modelPos.y, 0.0, 1.0);
    }
    else
    {
        vertPos = VP * vertPos;
    }

    gl_Position = vertPos;
}
