#define STB_IMAGE_IMPLEMENTATION

#include <string.h>
#include <stdio.h>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Light.h"
#include "Material.h"
#include "Main.h"

const float degreeToRadians = 3.14159265f / 180.0f;

// Camera
Camera camera;
double xPos, yPos, prevXPos, prevYPos;
GLfloat moveSpeed = 5.0f;
GLfloat turnSpeed = 0.1f;
glm::mat4 projection;

// Textures
Texture brickTexture;
Texture dirtTexture;

// Heightmaps
std::vector<float> heightmapVertices;
std::vector<unsigned int> heightmapIndices;
unsigned char* heightmapData;

int width, height, nChannels;
unsigned int NUM_STRIPS;
unsigned int NUM_VERTS_PER_STRIP;

// Materials
Material shinyMaterial;
Material dullMaterial;

// Lighting
Light mainLight;

// Time
GLfloat deltaTime = 0.0f;
GLfloat lastTime = 0.0f;

// Transforming
bool direction = true;
float triOffset = 0.0f;
float triMaxOffset = 0.7f;
float triIncrement = 0.0005f;

// Rotating
float curAngle = 0.0f;

// Scaling
bool sizeDirection = true;
float currSize = 0.4f;
float maxSize = 0.8f;
float minSize = 0.1f;

// Models
std::vector<Mesh*> meshList;
std::vector<glm::mat4> modelList;
std::vector<GLuint> uniformModelList;
GLuint uniformProjection;
GLuint uniformView;
GLuint uniformEyePosition;
std::vector<GLuint> uniformAmbientIntensityList;
std::vector<GLuint> uniformAmbientColourList;
std::vector<GLuint> uniformDirectionList;
std::vector<GLuint> uniformDiffuseIntensityList;
std::vector<GLuint> uniformSpecularIntensityList;
std::vector<GLuint> uniformShininessList;

glm::vec3 position = glm::vec3(0.0f);
glm::vec3 scale = glm::vec3(1.0f);
glm::vec3 rotation = glm::vec3(0.0f);

// Shaders
static const char* vShader = "Shaders/shader.vert";
static const char* fShader = "Shaders/shader.frag";
std::vector<Shader*> shaderList;

// Window properties
Window mainWindow;
int screenWidth = 800;
int screenHeight = 600;

void loadTextures()
{
    brickTexture = Texture((char*)"Textures/brick.png");
    brickTexture.LoadTexture();

    dirtTexture = Texture((char*)"Textures/dirt.png");
    dirtTexture.LoadTexture();
}

void generateHeightmapVertices()
{
    float yScale = 0.25f;
    float yShift = 16.0f;

    // Loop through the heightmap's texels
    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            // Access each texel individually
            unsigned char* texel = heightmapData + (j + width * i) * nChannels;

            // Height value for texel
            unsigned char y = texel[0];

            // Store transformed values for x, y, and z
            heightmapVertices.push_back(-height / 2.0f + height * i / (float)height);
            heightmapVertices.push_back(y * yScale - yShift);
            heightmapVertices.push_back(-width / 2.0f + width * j / (float)width);
        }
    }

    NUM_STRIPS = height - 1;
    NUM_VERTS_PER_STRIP = width * 2;

    // Release heightmap data from memory
    stbi_image_free(heightmapData);
}

void generateHeightmapIndices()
{
    for (unsigned int i = 0; i < height; i++)
    {
        for (unsigned int j = 0; j < width; j++)
        {
            for (unsigned int k = 0; k < 2; k++)
            {
                heightmapIndices.push_back(j + width * (i + k));
            }
        }
    }
}

void createHeightMap()
{
    // Load heightmap from memory
    heightmapData = stbi_load("Heightmaps/custom_heightmap_2.png", &width, &height, &nChannels, 0);

    // Check if the heightmap has loaded correctly
    if (heightmapData)
    {
        std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    // Initialise all necessary heightmap details
    generateHeightmapVertices();
    generateHeightmapIndices();

    // Create heightmap mesh
    Mesh* heightmapMesh = new Mesh();
    heightmapMesh->createMeshFromHeightmap(heightmapVertices, heightmapIndices);
    meshList.push_back(heightmapMesh);

    // Create heightmap model
    glm::mat4 heightmapModel = glm::mat4(1.0f);
    modelList.push_back(heightmapModel);
}

void calcAverageNormals(unsigned int* indices, unsigned int indexCount, GLfloat* vertices,
    unsigned int vertexCount, unsigned int vLength, unsigned int normalOffset)
{
    for (size_t i = 0; i < indexCount; i += 3)
    {
        unsigned int in0 = indices[i] * vLength;
        unsigned int in1 = indices[i + 1] * vLength;
        unsigned int in2 = indices[i + 2] * vLength;

        glm::vec3 v1(vertices[in1] - vertices[in0], vertices[in1 + 1] - vertices[in0 + 1], vertices[in1 + 2] - vertices[in0 + 2]);
        glm::vec3 v2(vertices[in2] - vertices[in0], vertices[in2 + 1] - vertices[in0 + 1], vertices[in2 + 2] - vertices[in0 + 2]);
        glm::vec3 normal = glm::cross(v1, v2);
        normal = glm::normalize(normal);

        in0 += normalOffset;
        in1 += normalOffset;
        in2 += normalOffset;

        vertices[in0] += normal.x;
        vertices[in0 + 1] += normal.y;
        vertices[in0 + 2] += normal.z;

        vertices[in1] += normal.x;
        vertices[in1 + 1] += normal.y;
        vertices[in1 + 2] += normal.z;

        vertices[in2] += normal.x;
        vertices[in2 + 1] += normal.y;
        vertices[in2 + 2] += normal.z;
    }

    for (size_t i = 0; i < vertexCount / vLength; i++)
    {
        unsigned int nOffset = i * vLength + normalOffset;
        glm::vec3 vec(vertices[nOffset], vertices[nOffset + 1], vertices[nOffset + 2]);
        vec = glm::normalize(vec);
        vertices[nOffset] = vec.x; vertices[nOffset + 1] = vec.y; vertices[nOffset + 2] = vec.z;
    }
}

void createSpecifiedObject(int type, GLfloat vertices[], unsigned int indices[], unsigned int numVertices, unsigned int numIndices)
{
    if (type == 0) {
        calcAverageNormals(indices, 12, vertices, 36, 8, 5);
    }
    else if (type == 1) {
        calcAverageNormals(indices, 36, vertices, 64, 8, 5);
    }

    Mesh* mesh = new Mesh();
    mesh->createMesh(vertices, indices, numVertices, numIndices);
    meshList.push_back(mesh);

    glm::mat4 model = glm::mat4(1.0f);
    modelList.push_back(model);
}

void createObjects()
{
    #pragma region Model Indices
    unsigned int indices[] =
    {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    unsigned int indices2[] =
    {
        // 2 triangles per face
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Back face
        4, 5, 6,
        6, 7, 4,

        // Top face
        1, 6, 5,
        5, 2, 1,

        // Bottom face
        7, 0, 3,
        3, 4, 7,

        // Right face
        3, 2, 5,
        5, 4, 3,

        // Left face
        7, 6, 1,
        1, 0, 7
    };
    #pragma endregion

    #pragma region Model Vertices
    // Points that make up the pyramid
    GLfloat vertices[] =
    {
        // x     y     z         u     v         Nx    Ny    Nz
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,     0.0f, 0.0f, 0.0f,
        0.0f, -1.0f, 1.0f,      0.5f, 0.0f,     0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f,      1.0f, 0.0f,     0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,       0.5f, 1.0f,     0.0f, 0.0f, 0.0f
    };

    // Points that make up the first cube
    GLfloat vertices2[] =
    {
        // x     y     z         u     v         Nx    Ny    Nz
        -3.0f, -1.0f, -1.0f,    0.0f, 0.0f,     0.0f, 0.0f, 0.0f,
        -3.0f, 1.0f, -1.0f,     0.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, -1.0f,     1.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,    1.0f, 0.0f,     0.0f, 0.0f, 0.0f,

        -1.0f, -1.0f, 1.0f,     0.0f, 0.0f,     0.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 1.0f,      0.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        -3.0f, 1.0f, 1.0f,      1.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        -3.0f, -1.0f, 1.0f,     1.0f, 0.0f,     0.0f, 0.0f, 0.0f
    };

    // Points that make up the second cube
    GLfloat vertices3[] =
    {
        // x     y     z         u     v         Nx    Ny    Nz
        1.0f, -1.0f, -1.0f,     0.0f, 0.0f,     0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, -1.0f,      0.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        3.0f, 1.0f, -1.0f,      1.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        3.0f, -1.0f, -1.0f,     1.0f, 0.0f,     0.0f, 0.0f, 0.0f,

        3.0f, -1.0f, 1.0f,      0.0f, 0.0f,     0.0f, 0.0f, 0.0f,
        3.0f, 1.0f, 1.0f,       0.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f,       1.0f, 1.0f,     0.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 1.0f,      1.0f, 0.0f,     0.0f, 0.0f, 0.0f
    };
    #pragma endregion

    // Pyramids
    createSpecifiedObject(0, vertices, indices, 32, 12);

    // Cubes
    createSpecifiedObject(1, vertices2, indices2, 64, 36);
    createSpecifiedObject(1, vertices3, indices2, 64, 36);
}

void createShaders()
{
    Shader* shader = new Shader();
    shader->CreateFromFiles(vShader, fShader);
    shaderList.push_back(shader);
}

void initialiseUniforms()
{
    for (size_t i = 0; i < modelList.size(); i++)
    {
        uniformModelList.push_back(0);
        uniformProjection = 0;
        uniformView = 0;
        uniformEyePosition = 0;

        uniformAmbientIntensityList.push_back(0);
        uniformAmbientColourList.push_back(0);
        uniformDirectionList.push_back(0);
        uniformDiffuseIntensityList.push_back(0);
        uniformSpecularIntensityList.push_back(0);
        uniformShininessList.push_back(0);
    }
}

void setUniforms()
{
    for (size_t i = 0; i < uniformModelList.size(); i++)
    {
        uniformModelList.at(i) = shaderList[0]->getModelLocation();
        uniformProjection = shaderList[0]->getProjectionLocation();
        uniformView = shaderList[0]->getViewLocation();

        uniformAmbientColourList.at(i) = shaderList[0]->getAmbientColourLocation();
        uniformAmbientIntensityList.at(i) = shaderList[0]->getAmbientIntensityLocation();
        uniformDirectionList.at(i) = shaderList[0]->getDirectionLocation();
        uniformDiffuseIntensityList.at(i) = shaderList[0]->getDiffuseIntensityLocation();

        uniformEyePosition = shaderList[0]->getEyePositionLocation();

        uniformSpecularIntensityList.at(i) = shaderList[0]->getSpecularIntensityLocation();
        uniformShininessList.at(i) = shaderList[0]->getShininessLocation();

        glUniformMatrix4fv(uniformModelList.at(i), 1, GL_FALSE, glm::value_ptr(modelList.at(i)));
    }
}

void initialiseModelPositions() 
{
    glm::mat4 tempModel;

    for (size_t i = 0; i < meshList.size(); i++)
    {
        tempModel = glm::mat4(1.0f);

        if (i != 0)
        {
            tempModel = glm::translate(tempModel, glm::vec3(0.0f, 0.0f, -3.0f));
            tempModel = glm::scale(tempModel, glm::vec3(0.3f, 0.3f, 0.3f));
            tempModel = glm::rotate(tempModel, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            tempModel = glm::rotate(tempModel, glm::radians(2.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            tempModel = glm::rotate(tempModel, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        }

        // Add each model to the respective list
        modelList.at(i) = tempModel;
    }
}

void updateTransformations()
{
    // Loop through all existing models
    for (size_t i = 0; i < modelList.size(); i++)
    {
        // Heightmap will not be transformed, therefore skip
        if (i != 0)
        {
            // Model Logic
            modelList.at(i) = glm::translate(modelList.at(i), position);
            modelList.at(i) = glm::scale(modelList.at(i), scale);
            modelList.at(i) = glm::rotate(modelList.at(i), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            modelList.at(i) = glm::rotate(modelList.at(i), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            modelList.at(i) = glm::rotate(modelList.at(i), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        }

        // Lighting and Texturing Logic
        // Access first pyramid object
        if (i == 1)
        {
            shinyMaterial.UseMaterial(uniformSpecularIntensityList.at(i), uniformShininessList.at(i));
            brickTexture.UseTexture();
        }
        // Access next 2 cube objects
        else if (i == 2)
        {
            shinyMaterial.UseMaterial(uniformSpecularIntensityList.at(i), uniformShininessList.at(i));
            dirtTexture.UseTexture();
        }
        else if (i == 3)
        {
            dullMaterial.UseMaterial(uniformSpecularIntensityList.at(i), uniformShininessList.at(i));
            dirtTexture.UseTexture();
        }

        // Rendering Logic
        // Render heightmaps
        if (i == 0)
        {
            meshList[i]->renderMeshFromHeightmap(NUM_STRIPS, NUM_VERTS_PER_STRIP);
        }
        // Render normal meshes
        else
        {
            //meshList[i]->renderMesh();
        }
    }
}

int main(void)
{
    mainWindow = Window(screenWidth, screenHeight);
    mainWindow.initialise();

    createHeightMap();
    createObjects();
    createShaders();

    camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 15.0f, 0.25f);

    loadTextures();

    shinyMaterial = Material(1.0f, 32);
    dullMaterial = Material(0.3f, 4);

    // Ambient lighting so the models can be seen
                     /* r     g      b    aI    x     y     z     dI */
    mainLight = Light(0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    projection = glm::perspective(glm::radians(45.0f), mainWindow.getBufferWidth() / mainWindow.getBufferHeight(), 0.1f, 100.0f);

    initialiseUniforms();

    while (!mainWindow.getShouldClose())
    {
        GLfloat now = glfwGetTime();
        deltaTime = now - lastTime;
        lastTime = now;

        glfwPollEvents();

        // Get input for camera
        camera.keyControl(mainWindow.getKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange());

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0]->useShader();

        #pragma region Update Model Transformations
        setUniforms();

        // Projection
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

        // View
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(camera.calculateViewMatrix()));

        // Eye position
        glUniform3f(uniformEyePosition, camera.getCameraPosition().x, camera.getCameraPosition().y, camera.getCameraPosition().z);

        mainLight.UseLight(uniformAmbientIntensityList.at(0), uniformAmbientColourList.at(0),
            uniformDiffuseIntensityList.at(0), uniformDirectionList.at(0));

        initialiseModelPositions();

        updateTransformations();
        #pragma endregion

        glUseProgram(0);

        mainWindow.swapBuffers();
    }

    return 0;
}