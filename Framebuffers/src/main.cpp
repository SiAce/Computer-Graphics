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
    window = glfwCreateWindow(1000, 1000, "Texture Mapping", NULL, NULL);
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

    // Create Vertex Array Object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLfloat vertices[] = {
        1.000000f, 1.000000f, -1.000000f,
        1.000000f, -1.000000f, -1.000000f,
        -1.000000f, -1.000000f, -1.000000f,
        1.000000f, 1.000000f, -1.000000f,
        -1.000000f, -1.000000f, -1.000000f,
        -1.000000f, 1.000000f, -1.000000f,
        -1.000000f, -1.000000f, 1.000000f,
        -1.000000f, 1.000000f, 1.000000f,
        -1.000000f, 1.000000f, -1.000000f,
        -1.000000f, -1.000000f, 1.000000f,
        -1.000000f, 1.000000f, -1.000000f,
        -1.000000f, -1.000000f, -1.000000f,
        1.000000f, -1.000000f, 1.000000f,
        0.999999f, 1.000000f, 1.000001f,
        -1.000000f, -1.000000f, 1.000000f,
        0.999999f, 1.000000f, 1.000001f,
        -1.000000f, 1.000000f, 1.000000f,
        -1.000000f, -1.000000f, 1.000000f,
        1.000000f, -1.000000f, -1.000000f,
        1.000000f, 1.000000f, -1.000000f,
        1.000000f, -1.000000f, 1.000000f,
        1.000000f, 1.000000f, -1.000000f,
        0.999999f, 1.000000f, 1.000001f,
        1.000000f, -1.000000f, 1.000000f,
        1.000000f, 1.000000f, -1.000000f,
        -1.000000f, 1.000000f, -1.000000f,
        0.999999f, 1.000000f, 1.000001f,
        -1.000000f, 1.000000f, -1.000000f,
        -1.000000f, 1.000000f, 1.000000f,
        0.999999f, 1.000000f, 1.000001f,
        1.000000f, -1.000000f, -1.000000f,
        1.000000f, -1.000000f, 1.000000f,
        -1.000000f, -1.000000f, 1.000000f,
        1.000000f, -1.000000f, -1.000000f,
        -1.000000f, -1.000000f, 1.000000f,
        -1.000000f, -1.000000f, -1.000000f};

    GLfloat uvs[] = {
        0.748573f, 0.750412f,
        0.749279f, 0.501284f,
        0.999110f, 0.501077f,
        0.748573f, 0.750412f,
        0.999110f, 0.501077f,
        0.999455f, 0.750380f,
        0.250471f, 0.500702f,
        0.249682f, 0.749677f,
        0.001085f, 0.750380f,
        0.250471f, 0.500702f,
        0.001085f, 0.750380f,
        0.001517f, 0.499994f,
        0.499422f, 0.500239f,
        0.500149f, 0.750166f,
        0.250471f, 0.500702f,
        0.500149f, 0.750166f,
        0.249682f, 0.749677f,
        0.250471f, 0.500702f,
        0.749279f, 0.501284f,
        0.748573f, 0.750412f,
        0.499422f, 0.500239f,
        0.748573f, 0.750412f,
        0.500149f, 0.750166f,
        0.499422f, 0.500239f,
        0.748573f, 0.750412f,
        0.748355f, 0.998230f,
        0.500149f, 0.750166f,
        0.748355f, 0.998230f,
        0.500193f, 0.998728f,
        0.500149f, 0.750166f,
        0.749279f, 0.501284f,
        0.499422f, 0.500239f,
        0.498993f, 0.250415f,
        0.749279f, 0.501284f,
        0.498993f, 0.250415f,
        0.748953f, 0.250920f};

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

    const GLchar *vertexSource = R"glsl(
        #version 150 core

        in vec3 position;
        in vec3 color;
        in vec2 texcoord;

        out vec3 Color;
        out vec2 Texcoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 proj;

        void main()
        {
            Color = color;
            Texcoord = texcoord;
            gl_Position = proj * view * model * vec4(position, 1.0);
        }

    )glsl";

    const GLchar *fragmentSource = R"glsl(
        #version 150 core

        in vec3 Color;
        in vec2 Texcoord;

        out vec4 outColor;

        uniform sampler2D texKitten;
        uniform sampler2D texPuppy;

        void main()
        {
            outColor = texture(texKitten, Texcoord);
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
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Load textures
    GLuint textures[2];
    glGenTextures(2, textures);

    int width, height, channels;
    unsigned char *image;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    image = stbi_load("../img/sample.png", &width, &height, &channels, STBI_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    glUniform1i(glGetUniformLocation(shaderProgram, "texKitten"), 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    image = stbi_load("../img/sample2.png", &width, &height, &channels, STBI_rgb);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    stbi_image_free(image);
    glUniform1i(glGetUniformLocation(shaderProgram, "texPuppy"), 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Counter-clockwise rotation matrix
    GLint uniTrans = glGetUniformLocation(shaderProgram, "model");

    // View Matrix
    glm::mat4 view = glm::lookAt(
        glm::vec3(1.2f, 1.2f, 1.2f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f));
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    // Projection Matrix
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 10.0f);
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

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

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        model = glm::rotate(
            model,
            time * glm::radians(180.0f),
            glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

        // // Draw a rectangle from the 2 triangles using 6 indices
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // 1rst attribute buffer : vertices
        GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
        glEnableVertexAttribArray(posAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
            posAttrib, // attribute. No particular reason for 0, but must match the layout in the shader.
            3,         // size
            GL_FLOAT,  // type
            GL_FALSE,  // normalized?
            0,         // stride
            (void *)0  // array buffer offset
        );

        // 2nd attribute buffer : UVs
        GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
        glEnableVertexAttribArray(texAttrib);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            texAttrib, // attribute. No particular reason for 1, but must match the layout in the shader.
            2,         // size : U+V => 2
            GL_FLOAT,  // type
            GL_FALSE,  // normalized?
            0,         // stride
            (void *)0  // array buffer offset
        );

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDisableVertexAttribArray(posAttrib);
        glDisableVertexAttribArray(texAttrib);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    glDeleteTextures(2, textures);

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteVertexArrays(1, &vao);
    glDeleteTextures(2, textures);

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
