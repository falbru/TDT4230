#include "gamelogic.h"
#include "sceneGraph.hpp"
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <chrono>
#include <fmt/format.h>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/glutils.h>
#include <utilities/mesh.h>
#include <utilities/shader.hpp>
#include <utilities/shapes.h>
#include <utilities/timeutils.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/glfont.h"
#include "utilities/imageLoader.hpp"

enum KeyFrameAction
{
    BOTTOM,
    TOP
};

#include <timestamps.h>

double padPositionX = 0;
double padPositionZ = 0;

unsigned int currentKeyFrame = 0;
unsigned int previousKeyFrame = 0;

SceneNode *rootNode;
SceneNode *boxNode;
SceneNode *ballNode;
SceneNode *padNode;
SceneNode *textNode;

SceneNode *ballLightNode;

double ballRadius = 3.0f;

glm::mat4 VP;
glm::mat4 VP_2D;

glm::vec3 cameraPosition(0, 2, -20);

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer *buffer;
Gloom::Shader *shader;
sf::Sound *sound;

const glm::vec3 boxDimensions(180, 90, 90);
const glm::vec3 padDimensions(30, 3, 40);

glm::vec3 ballPosition(0, ballRadius + padDimensions.y, boxDimensions.z / 2);
glm::vec3 ballDirection(1, 1, 0.2f);

CommandLineOptions options;

bool hasStarted = false;
bool hasLost = false;
bool jumpedToNextFrame = false;
bool isPaused = false;

bool mouseLeftPressed = false;
bool mouseLeftReleased = false;
bool mouseRightPressed = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow *window, double x, double y)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    double deltaX = x - lastMouseX;
    double deltaY = y - lastMouseY;

    padPositionX -= mouseSensitivity * deltaX / windowWidth;
    padPositionZ -= mouseSensitivity * deltaY / windowHeight;

    if (padPositionX > 1)
        padPositionX = 1;
    if (padPositionX < 0)
        padPositionX = 0;
    if (padPositionZ > 1)
        padPositionZ = 1;
    if (padPositionZ < 0)
        padPositionZ = 0;

    glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
}

unsigned int genTexture(const PNGImage &img)
{
    unsigned int textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return textureId;
}

void initGame(GLFWwindow *window, CommandLineOptions gameOptions)
{
    buffer = new sf::SoundBuffer();
    if (!buffer->loadFromFile("../res/Hall of the Mountain King.ogg"))
    {
        return;
    }

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");
    shader->activate();

    // Create meshes
    Mesh pad = cube(padDimensions, glm::vec2(30, 40), true);
    Mesh box = cube(boxDimensions, glm::vec2(90), true, true);
    Mesh sphere = generateSphere(1.0, 40, 40);
    Mesh text = generateTextGeometryBuffer("Click to start the game", 39.0f / 29.0f, 500);

    // Fill buffers
    unsigned int ballVAO = generateBuffer(sphere);
    unsigned int boxVAO = generateBuffer(box);
    unsigned int padVAO = generateBuffer(pad);
    unsigned int textVAO = generateBuffer(text);

    // Load textures
    PNGImage charmap = loadPNGFile("../res/textures/charmap.png");
    int charmapTextureID = genTexture(charmap);

    int boxDiffuseTextureID = genTexture(loadPNGFile("../res/textures/Brick03_col.png"));
    int boxNormalMapTextureID = genTexture(loadPNGFile("../res/textures/Brick03_nrm.png"));
    int boxRoughnessMapTextureID = genTexture(loadPNGFile("../res/textures/Brick03_rgh.png"));

    // Construct scene
    rootNode = createSceneNode();
    boxNode = createSceneNode();
    padNode = createSceneNode();
    ballNode = createSceneNode();
    textNode = createSceneNode();
    ballLightNode = createLightSceneNode();

    rootNode->children.push_back(boxNode);
    rootNode->children.push_back(padNode);
    rootNode->children.push_back(ballNode);
    rootNode->children.push_back(textNode);

    boxNode->nodeType = SceneNodeType::NORMAL_MAPPED_GEOMETRY;
    boxNode->vertexArrayObjectID = boxVAO;
    boxNode->VAOIndexCount = box.indices.size();
    boxNode->textureID = boxDiffuseTextureID;
    boxNode->normalMapTextureID = boxNormalMapTextureID;
    boxNode->roughnessMapTextureID = boxRoughnessMapTextureID;

    padNode->vertexArrayObjectID = padVAO;
    padNode->VAOIndexCount = pad.indices.size();

    ballNode->vertexArrayObjectID = ballVAO;
    ballNode->VAOIndexCount = sphere.indices.size();
    ballNode->children.push_back(ballLightNode);

    textNode->vertexArrayObjectID = textVAO;
    textNode->nodeType = SceneNodeType::GEOMETRY_2D;
    textNode->VAOIndexCount = text.indices.size();
    textNode->textureID = charmapTextureID;

    ballNode->position = glm::vec3(0, 0, 0);
    padNode->position = glm::vec3(0, 0, 0);
    textNode->position = glm::vec3(50, 50, 0);

    ballLightNode->lightColor = glm::vec3(1, 1, 1);

    getTimeDeltaSeconds();

    std::cout << fmt::format("Initialized scene with {} SceneNodes.", totalChildren(rootNode)) << std::endl;

    std::cout << "Ready. Click to start!" << std::endl;
}

void updateFrame(GLFWwindow *window)
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    const float ballBottomY = boxNode->position.y - (boxDimensions.y / 2) + ballRadius + padDimensions.y;
    const float ballTopY = boxNode->position.y + (boxDimensions.y / 2) - ballRadius;
    const float BallVerticalTravelDistance = ballTopY - ballBottomY;

    const float cameraWallOffset = 30; // Arbitrary addition to prevent ball from going too much into camera

    const float ballMinX = boxNode->position.x - (boxDimensions.x / 2) + ballRadius;
    const float ballMaxX = boxNode->position.x + (boxDimensions.x / 2) - ballRadius;
    const float ballMinZ = boxNode->position.z - (boxDimensions.z / 2) + ballRadius;
    const float ballMaxZ = boxNode->position.z + (boxDimensions.z / 2) - ballRadius - cameraWallOffset;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1))
    {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    }
    else
    {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2))
    {
        mouseRightPressed = true;
        mouseRightReleased = false;
    }
    else
    {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    if (!hasStarted)
    {
        if (mouseLeftPressed)
        {
            if (options.enableMusic)
            {
                sound = new sf::Sound();
                sound->setBuffer(*buffer);
                sf::Time startTime = sf::seconds(debug_startTime);
                sound->setPlayingOffset(startTime);
                sound->play();
            }
            totalElapsedTime = debug_startTime;
            gameElapsedTime = debug_startTime;
            hasStarted = true;
        }

        ballPosition.x = ballMinX + (1 - padPositionX) * (ballMaxX - ballMinX);
        ballPosition.y = ballBottomY;
        ballPosition.z = ballMinZ + (1 - padPositionZ) * ((ballMaxZ + cameraWallOffset) - ballMinZ);
    }
    else
    {
        totalElapsedTime += timeDelta;
        if (hasLost)
        {
            if (mouseLeftReleased)
            {
                hasLost = false;
                hasStarted = false;
                currentKeyFrame = 0;
                previousKeyFrame = 0;
            }
        }
        else if (isPaused)
        {
            if (mouseRightReleased)
            {
                isPaused = false;
                if (options.enableMusic)
                {
                    sound->play();
                }
            }
        }
        else
        {
            gameElapsedTime += timeDelta;
            if (mouseRightReleased)
            {
                isPaused = true;
                if (options.enableMusic)
                {
                    sound->pause();
                }
            }
            // Get the timing for the beat of the song
            for (unsigned int i = currentKeyFrame; i < keyFrameTimeStamps.size(); i++)
            {
                if (gameElapsedTime < keyFrameTimeStamps.at(i))
                {
                    continue;
                }
                currentKeyFrame = i;
            }

            jumpedToNextFrame = currentKeyFrame != previousKeyFrame;
            previousKeyFrame = currentKeyFrame;

            double frameStart = keyFrameTimeStamps.at(currentKeyFrame);
            double frameEnd = keyFrameTimeStamps.at(currentKeyFrame + 1); // Assumes last keyframe at infinity

            double elapsedTimeInFrame = gameElapsedTime - frameStart;
            double frameDuration = frameEnd - frameStart;
            double fractionFrameComplete = elapsedTimeInFrame / frameDuration;

            double ballYCoord;

            KeyFrameAction currentOrigin = keyFrameDirections.at(currentKeyFrame);
            KeyFrameAction currentDestination = keyFrameDirections.at(currentKeyFrame + 1);

            // Synchronize ball with music
            if (currentOrigin == BOTTOM && currentDestination == BOTTOM)
            {
                ballYCoord = ballBottomY;
            }
            else if (currentOrigin == TOP && currentDestination == TOP)
            {
                ballYCoord = ballBottomY + BallVerticalTravelDistance;
            }
            else if (currentDestination == BOTTOM)
            {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * (1 - fractionFrameComplete);
            }
            else if (currentDestination == TOP)
            {
                ballYCoord = ballBottomY + BallVerticalTravelDistance * fractionFrameComplete;
            }

            // Make ball move
            const float ballSpeed = 60.0f;
            ballPosition.x += timeDelta * ballSpeed * ballDirection.x;
            ballPosition.y = ballYCoord;
            ballPosition.z += timeDelta * ballSpeed * ballDirection.z;

            // Make ball bounce
            if (ballPosition.x < ballMinX)
            {
                ballPosition.x = ballMinX;
                ballDirection.x *= -1;
            }
            else if (ballPosition.x > ballMaxX)
            {
                ballPosition.x = ballMaxX;
                ballDirection.x *= -1;
            }
            if (ballPosition.z < ballMinZ)
            {
                ballPosition.z = ballMinZ;
                ballDirection.z *= -1;
            }
            else if (ballPosition.z > ballMaxZ)
            {
                ballPosition.z = ballMaxZ;
                ballDirection.z *= -1;
            }

            if (options.enableAutoplay)
            {
                padPositionX = 1 - (ballPosition.x - ballMinX) / (ballMaxX - ballMinX);
                padPositionZ = 1 - (ballPosition.z - ballMinZ) / ((ballMaxZ + cameraWallOffset) - ballMinZ);
            }

            // Check if the ball is hitting the pad when the ball is at the bottom.
            // If not, you just lost the game! (hehe)
            if (jumpedToNextFrame && currentOrigin == BOTTOM && currentDestination == TOP)
            {
                double padLeftX = boxNode->position.x - (boxDimensions.x / 2) +
                                  (1 - padPositionX) * (boxDimensions.x - padDimensions.x);
                double padRightX = padLeftX + padDimensions.x;
                double padFrontZ = boxNode->position.z - (boxDimensions.z / 2) +
                                   (1 - padPositionZ) * (boxDimensions.z - padDimensions.z);
                double padBackZ = padFrontZ + padDimensions.z;

                if (ballPosition.x < padLeftX || ballPosition.x > padRightX || ballPosition.z < padFrontZ ||
                    ballPosition.z > padBackZ)
                {
                    hasLost = true;
                    if (options.enableMusic)
                    {
                        sound->stop();
                        delete sound;
                    }
                }
            }
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);

    // Some math to make the camera move in a nice way
    float lookRotation = -0.6 / (1 + exp(-5 * (padPositionX - 0.5))) + 0.3;
    glm::mat4 cameraTransform = glm::rotate(0.3f + 0.2f * float(-padPositionZ * padPositionZ), glm::vec3(1, 0, 0)) *
                                glm::rotate(lookRotation, glm::vec3(0, 1, 0)) * glm::translate(-cameraPosition);

    VP = projection * cameraTransform;

    // Move and rotate various SceneNodes
    boxNode->position = {0, -10, -80};

    ballNode->position = ballPosition;
    ballNode->scale = glm::vec3(ballRadius);
    ballNode->rotation = {0, totalElapsedTime * 2, 0};

    padNode->position = {boxNode->position.x - (boxDimensions.x / 2) + (padDimensions.x / 2) +
                             (1 - padPositionX) * (boxDimensions.x - padDimensions.x),
                         boxNode->position.y - (boxDimensions.y / 2) + (padDimensions.y / 2),
                         boxNode->position.z - (boxDimensions.z / 2) + (padDimensions.z / 2) +
                             (1 - padPositionZ) * (boxDimensions.z - padDimensions.z)};

    updateNodeTransformations(rootNode, glm::mat4(1.0f));
}

void updateNodeTransformations(SceneNode *node, glm::mat4 transformationThusFar)
{
    glm::mat4 transformationMatrix = glm::translate(node->position) * glm::translate(node->referencePoint) *
                                     glm::rotate(node->rotation.y, glm::vec3(0, 1, 0)) *
                                     glm::rotate(node->rotation.x, glm::vec3(1, 0, 0)) *
                                     glm::rotate(node->rotation.z, glm::vec3(0, 0, 1)) * glm::scale(node->scale) *
                                     glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;

    switch (node->nodeType)
    {
    case GEOMETRY:
    case NORMAL_MAPPED_GEOMETRY:
        node->currentNormalMatrix = glm::transpose(glm::inverse(glm::mat3(node->currentTransformationMatrix)));
        break;
    case POINT_LIGHT:
        break;
    case SPOT_LIGHT:
        break;
    }

    for (SceneNode *child : node->children)
    {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderNode(SceneNode *node)
{
    glUniformMatrix4fv(shader->getUniformFromName("M"), 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
    glUniformMatrix3fv(shader->getUniformFromName("N"), 1, GL_FALSE, glm::value_ptr(node->currentNormalMatrix));

    switch (node->nodeType)
    {
    case GEOMETRY:
        glUniform1i(shader->getUniformFromName("is2D"), false);
        glUniform1i(shader->getUniformFromName("useNM"), false);
        glUniformMatrix4fv(shader->getUniformFromName("VP"), 1, GL_FALSE, glm::value_ptr(VP));
        if (node->vertexArrayObjectID != -1)
        {
            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
        }
        break;
    case NORMAL_MAPPED_GEOMETRY:
        glUniform1i(shader->getUniformFromName("is2D"), false);
        glUniform1i(shader->getUniformFromName("useNM"), true);
        glUniformMatrix4fv(shader->getUniformFromName("VP"), 1, GL_FALSE, glm::value_ptr(VP));
        glBindTextureUnit(0, node->textureID);
        glBindTextureUnit(1, node->normalMapTextureID);
        glBindTextureUnit(2, node->roughnessMapTextureID);
        if (node->vertexArrayObjectID != -1)
        {
            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
        }
        break;
    case GEOMETRY_2D:
        glUniform1i(shader->getUniformFromName("is2D"), true);
        glUniform1i(shader->getUniformFromName("useNM"), false);
        glUniformMatrix4fv(shader->getUniformFromName("VP"), 1, GL_FALSE, glm::value_ptr(VP_2D));
        glBindTextureUnit(0, node->textureID);
        if (node->vertexArrayObjectID != -1)
        {
            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
        }
        break;
    case POINT_LIGHT:
        break;
    case SPOT_LIGHT:
        break;
    }

    for (SceneNode *child : node->children)
    {
        renderNode(child);
    }
}

void updateLightsInShader(SceneNode *node)
{
    switch (node->nodeType)
    {
    case POINT_LIGHT:
        glUniform3fv(shader->getUniformFromName(fmt::format("lights[{}].position", std::to_string(node->lightIndex))),
                     1, glm::value_ptr(glm::vec3(node->currentTransformationMatrix[3])));
        glUniform3fv(shader->getUniformFromName(fmt::format("lights[{}].color", std::to_string(node->lightIndex))), 1,
                     glm::value_ptr(glm::vec3(node->lightColor)));
        break;
    default:
        break;
    }

    for (SceneNode *child : node->children)
    {
        updateLightsInShader(child);
    }
}

void renderFrame(GLFWwindow *window)
{
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    VP_2D = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);

    glUniform1i(shader->getUniformFromName("lightsCount"), SceneNode::lightsCount);
    glUniform3fv(shader->getUniformFromName("cameraPos"), 1, glm::value_ptr(cameraPosition));
    glUniform3fv(shader->getUniformFromName("ballPos"), 1, glm::value_ptr(ballPosition));
    glUniform1f(shader->getUniformFromName("ballRadius"), ballRadius);

    updateLightsInShader(rootNode);
    renderNode(rootNode);
}
