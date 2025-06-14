#include "glfont.h"
#include <iostream>

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth)
{
    float characterWidth = totalTextWidth / float(text.length());
    float characterHeight = characterHeightOverWidth * characterWidth;

    unsigned int vertexCount = 4 * text.length();
    unsigned int indexCount = 6 * text.length();
    unsigned int textureCoordinateCount = 4 * text.length();

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);
    mesh.textureCoordinates.resize(textureCoordinateCount);

    for (unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};

        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};

        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;

        char character = text[i];
        float u = character / 128.0f;
        float uWidth = 1.0f / 128.0f;

        mesh.textureCoordinates.at(4 * i + 0) = {u, 0};
        mesh.textureCoordinates.at(4 * i + 1) = {u + uWidth, 0};
        mesh.textureCoordinates.at(4 * i + 2) = {u + uWidth, 1.0f};

        mesh.textureCoordinates.at(4 * i + 0) = {u, 0};
        mesh.textureCoordinates.at(4 * i + 2) = {u + uWidth, 1.0f};
        mesh.textureCoordinates.at(4 * i + 3) = {u, 1.0f};
    }

    return mesh;
}
