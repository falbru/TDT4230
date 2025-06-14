#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <stack>
#include <stdbool.h>
#include <vector>

enum SceneNodeType
{
    GEOMETRY,
    GEOMETRY_2D,
    NORMAL_MAPPED_GEOMETRY,
    POINT_LIGHT,
    SPOT_LIGHT
};

struct SceneNode
{
    SceneNode()
    {
        position = glm::vec3(0, 0, 0);
        rotation = glm::vec3(0, 0, 0);
        scale = glm::vec3(1, 1, 1);

        referencePoint = glm::vec3(0, 0, 0);
        vertexArrayObjectID = -1;
        VAOIndexCount = 0;
        lightIndex = -1;

        nodeType = GEOMETRY;
    }

    // A list of all children that belong to this node.
    // For instance, in case of the scene graph of a human body shown in the assignment text, the "Upper Torso" node
    // would contain the "Left Arm", "Right Arm", "Head" and "Lower Torso" nodes in its list of children.
    std::vector<SceneNode *> children;

    // The node's position and rotation relative to its parent
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // A transformation matrix representing the transformation of the node's location relative to its parent. This
    // matrix is updated every frame.
    glm::mat4 currentTransformationMatrix;

    // A matrix for transorming the normal in the shader. This matrix is updated every frame.
    glm::mat3 currentNormalMatrix;

    // The location of the node's reference point
    glm::vec3 referencePoint;

    // The ID of the VAO containing the "appearance" of this SceneNode.
    int vertexArrayObjectID;
    unsigned int VAOIndexCount;

    // Node type is used to determine how to handle the contents of a node
    SceneNodeType nodeType;

    // Textures
    unsigned int textureID;
    unsigned int normalMapTextureID;
    unsigned int roughnessMapTextureID;

    // Light logic
    static int lightsCount;
    int lightIndex;
    glm::vec3 lightColor;
};

SceneNode *createSceneNode();
SceneNode *createLightSceneNode();
void addChild(SceneNode *parent, SceneNode *child);
void printNode(SceneNode *node);
int totalChildren(SceneNode *parent);

// For more details, see SceneGraph.cpp.
