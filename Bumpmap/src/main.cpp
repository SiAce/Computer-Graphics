// C++ include
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <sstream>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Include Object Loader
#include <objloader.hpp>

// Include tangent space calculator
#include <tangentspace.hpp>

// Include vbo indexer
#include <vboindexer.hpp>

// This example is heavily based on the tutorial at https://open.gl

// Sine and Cosine
#include "math.h"

#define PI 3.1415926535897

// OpenGL Helpers to reduce the clutter
#include "Helpers.h"

// stb library to load image
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#else
// GLFW is necessary to handle the OpenGL context
#include <GLFW/glfw3.h>
#endif

// Linear Algebra Library
#include <Eigen/Core>

// Timer
#include <chrono>

using namespace std;
using namespace Eigen;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    GLFWwindow *window;

    // Initialize the library
    if (!glfwInit())
        return -1;

    // Activate supersampling
    glfwWindowHint(GLFW_SAMPLES, 8);

    // Ensure that we get at least a 3.2 context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    // On apple we have to load a core profile with forward compatibility
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(1000, 1000, "Bump Mapping", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

#ifndef __APPLE__
    glewExperimental = true;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    int major, minor, rev;
    major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
    printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
    printf("Supported OpenGL is %s\n", (const char *)glGetString(GL_VERSION));
    printf("Supported GLSL is %s\n", (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));

    const GLchar *vertexSource = R"glsl(
        #version 330 core

        // Input vertex data, different for all executions of this shader.
        layout(location = 0) in vec3 vertexPosition_modelspace;
        layout(location = 1) in vec2 vertexUV;
        layout(location = 2) in vec3 vertexNormal_modelspace;
        layout(location = 3) in vec3 vertexTangent_modelspace;
        layout(location = 4) in vec3 vertexBitangent_modelspace;

        // Output data ; will be interpolated for each fragment.
        out vec2 UV;
        out vec3 Position_worldspace;
        out vec3 EyeDirection_cameraspace;
        out vec3 LightDirection_cameraspace;

        out vec3 LightDirection_tangentspace;
        out vec3 EyeDirection_tangentspace;

        // Values that stay constant for the whole mesh.
        uniform mat4 MVP;
        uniform mat4 V;
        uniform mat4 M;
        uniform mat3 MV3x3;
        uniform vec3 LightPosition_worldspace;

        void main(){

            // Output position of the vertex, in clip space : MVP * position
            gl_Position =  MVP * vec4(vertexPosition_modelspace,1);
            
            // Position of the vertex, in worldspace : M * position
            Position_worldspace = (M * vec4(vertexPosition_modelspace,1)).xyz;
            
            // Vector that goes from the vertex to the camera, in camera space.
            // In camera space, the camera is at the origin (0,0,0).
            vec3 vertexPosition_cameraspace = ( V * M * vec4(vertexPosition_modelspace,1)).xyz;
            EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

            // Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
            vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
            LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
            
            // UV of the vertex. No special space for this one.
            UV = vertexUV;
            
            // model to camera = ModelView
            vec3 vertexTangent_cameraspace = MV3x3 * vertexTangent_modelspace;
            vec3 vertexBitangent_cameraspace = MV3x3 * vertexBitangent_modelspace;
            vec3 vertexNormal_cameraspace = MV3x3 * vertexNormal_modelspace;
            
            mat3 TBN = transpose(mat3(
                vertexTangent_cameraspace,
                vertexBitangent_cameraspace,
                vertexNormal_cameraspace	
            )); // You can use dot products instead of building this matrix and transposing it. See References for details.

            LightDirection_tangentspace = TBN * LightDirection_cameraspace;
            EyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;
            
            
        }

    )glsl";

    const GLchar *fragmentSource = R"glsl(
        #version 330 core

        // Interpolated values from the vertex shaders
        in vec2 UV;
        in vec3 Position_worldspace;
        in vec3 EyeDirection_cameraspace;
        in vec3 LightDirection_cameraspace;

        in vec3 LightDirection_tangentspace;
        in vec3 EyeDirection_tangentspace;

        // Ouput data
        out vec3 color;

        // Values that stay constant for the whole mesh.
        uniform sampler2D DiffuseTextureSampler;
        uniform sampler2D NormalTextureSampler;
        uniform sampler2D DisplacementTextureSampler;
        uniform mat4 V;
        uniform mat4 M;
        uniform mat3 MV3x3;
        uniform vec3 LightPosition_worldspace;

        void main(){

            // Light emission properties
            // You probably want to put them as uniforms
            vec3 LightColor = vec3(1,1,1);
            float LightPower = 40.0f;
            
            // Material properties
            vec3 MaterialDiffuseColor = texture( DiffuseTextureSampler, UV ).rgb;
            vec3 MaterialAmbientColor = vec3(0.3,0.3,0.3) * MaterialDiffuseColor;
            vec3 MaterialSpecularColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;

            // Output position of the vertex, in clip space : MVP * position
            float Bu =
                length(
                    texture( DisplacementTextureSampler, vec2(UV.x + 0.01, UV.y) ).rgb -
                    texture( DisplacementTextureSampler, vec2(UV.x - 0.01, UV.y) ).rgb
                ) * 50.0
            ;

            float Bv =
                length(
                    texture( DisplacementTextureSampler, vec2(UV.x, UV.y + 0.01) ).rgb -
                    texture( DisplacementTextureSampler, vec2(UV.x, UV.y - 0.01) ).rgb
                ) * 50.0
            ;

            vec3 n_origin = vec3(0, 0, 1.0);
            vec3 tangent = vec3(1.0, 0, 0);
            vec3 bitangent = vec3(0, 1.0, 0);
            
            vec3 TextureNormal_tangentspace = normalize(n_origin + Bu * cross(n_origin, tangent) + Bv * cross(n_origin, bitangent));
            
            // Distance to the light
            float distance = length( LightPosition_worldspace - Position_worldspace );

            // Normal of the computed fragment, in camera space
            vec3 n = TextureNormal_tangentspace;
            // Direction of the light (from the fragment to the light)
            vec3 l = normalize(LightDirection_tangentspace);
            // Cosine of the angle between the normal and the light direction, 
            // clamped above 0
            //  - light is at the vertical of the triangle -> 1
            //  - light is perpendicular to the triangle -> 0
            //  - light is behind the triangle -> 0
            float cosTheta = clamp( dot( n,l ), 0,1 );

            // Eye vector (towards the camera)
            vec3 E = normalize(EyeDirection_tangentspace);
            // Direction in which the triangle reflects the light
            vec3 R = reflect(-l,n);
            // Cosine of the angle between the Eye vector and the Reflect vector,
            // clamped to 0
            //  - Looking into the reflection -> 1
            //  - Looking elsewhere -> < 1
            float cosAlpha = clamp( dot( E,R ), 0,1 );
            
            color = 
                // Ambient : simulates indirect lighting
                MaterialAmbientColor +
                // Diffuse : "color" of the object
                MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
                // Specular : reflective highlight, like a mirror
                MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);

        }

    )glsl";

    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glBindFragDataLocation(programID, 0, "outColor");
    glLinkProgram(programID);
    glUseProgram(programID);

    // Create Vertex Array Object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = loadOBJ("../model/sphere.obj", vertices, uvs, normals);

    for (size_t i = 0; i < vertices.size(); i++)
    {
        vertices[i] = vertices[i] * 0.025f;
    }

    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;

    computeTangentBasis(
        vertices, uvs, normals, // input
        tangents, bitangents    // output
    );

    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    std::vector<glm::vec3> indexed_tangents;
    std::vector<glm::vec3> indexed_bitangents;

    indexVBO_TBN(
        vertices, uvs, normals, tangents, bitangents,
        indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents);

    // Load it into VBOs
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    GLuint tangentbuffer;
    glGenBuffers(1, &tangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_tangents.size() * sizeof(glm::vec3), &indexed_tangents[0], GL_STATIC_DRAW);

    GLuint bitangentbuffer;
    glGenBuffers(1, &bitangentbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_bitangents.size() * sizeof(glm::vec3), &indexed_bitangents[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        1,        // attribute
        2,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    // 3rd attribute buffer : normals
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(
        2,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    // 4th attribute buffer : tangents
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, tangentbuffer);
    glVertexAttribPointer(
        3,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    // 5th attribute buffer : bitangents
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, bitangentbuffer);
    glVertexAttribPointer(
        4,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void *)0 // array buffer offset
    );

    // Load textures
    GLuint textures[3];
    glGenTextures(3, textures);

    int width, height, channels;
    unsigned char *image;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    image = stbi_load("../img/texture.jpg", &width, &height, &channels, STBI_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    glUniform1i(glGetUniformLocation(programID, "DiffuseTextureSampler"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    image = stbi_load("../img/texture_NRM.png", &width, &height, &channels, STBI_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    glUniform1i(glGetUniformLocation(programID, "NormalTextureSampler"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    image = stbi_load("../img/texture_DISP.png", &width, &height, &channels, STBI_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    glUniform1i(glGetUniformLocation(programID, "DisplacementTextureSampler"), 2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
    GLuint ModelView3x3MatrixID = glGetUniformLocation(programID, "MV3x3");

    // Matrices calculation
    glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    glm::mat4 ViewMatrix = glm::lookAt(
        glm::vec3(1.2f, 0.6f, 1.2f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // Set light source
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    glm::vec3 lightPos = glm::vec3(4, 4, -2);
    glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Get the time when program starts
    auto t_start = std::chrono::high_resolution_clock::now();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {

        // Clear the framebuffer
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        auto t_now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> diff = t_now - t_start;
        float time = diff.count();

        glm::mat4 ModelMatrix = glm::mat4(1.0);
        ModelMatrix = glm::rotate(
            ModelMatrix,
            time * glm::radians(10.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 ModelViewMatrix = ViewMatrix * ModelMatrix;
        glm::mat3 ModelView3x3Matrix = glm::mat3(ModelViewMatrix);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
        glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &ModelView3x3Matrix[0][0]);

        // Draw the triangles !
        glDrawElements(
            GL_TRIANGLES,      // mode
            indices.size(),    // count
            GL_UNSIGNED_SHORT, // type
            (void *)0          // element array buffer offset
        );

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glDeleteProgram(programID);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);

    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(1, textures);

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
