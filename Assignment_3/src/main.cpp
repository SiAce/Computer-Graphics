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

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_C;

// Contains the vertex positions
Eigen::MatrixXf V(3, 36);

// Contains the per-vertex color
Eigen::MatrixXf C(3, 36);

// Contains the vertex starting points
Eigen::MatrixXf VSP(2, 3);

// Contains the keyframe starting points
Eigen::MatrixXf KSP(2, 3);

// Contains the vertex ending points
Eigen::MatrixXf KEP(2, 3);

float camera_position[3] = {4.0f, 3.0f, 3.0f};

// Mode control
int mode = 0;

// Camera Mode
int camera_mode = 0;

// Click times
int click_time = 1;

// Number of triangles
int triangle_number = 0;

// The index of the triangle selected
int selected_triangle = -1;

// The index of the keyframe triangle
int keyframe_triangle = -1;

// The index of the vertex selected
int selected_vertex = -1;

// The coordinates of the drag start point
double xworld_start = 0.;
double yworld_start = 0.;

// Used to check if we are in keyframe mode
bool is_keyframe = false;

// Used to check if we should start animation
bool start_animation = false;

// Record the time
auto t_start = std::chrono::high_resolution_clock::now();

bool point_in_triangle(double x1, double y1, double x2, double y2, double x3, double y3, double xp, double yp)
{
    double alpha = ((y2 - y3) * (xp - x3) + (x3 - x2) * (yp - y3)) /
                   ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
    double beta = ((y3 - y1) * (xp - x3) + (x1 - x3) * (yp - y3)) /
                  ((y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3));
    double gamma = 1.0 - alpha - beta;

    if (alpha >= 0 && alpha <= 1 && beta >= 0 && beta <= 1 && gamma >= 0 && gamma <= 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos / double(width)) * 2) - 1;
    double yworld = (((height - 1 - ypos) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw

    // Upload the change to the GPU
    VBO.update(V);
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    // Get the position of the mouse in the window
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Get the size of the window
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    // Convert screen position to world coordinates
    double xworld = ((xpos / double(width)) * 2) - 1;
    double yworld = (((height - 1 - ypos) / double(height)) * 2) - 1; // NOTE: y axis is flipped in glfw

    // Upload the change to the GPU
    VBO.update(V);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {

        switch (key)
        {
        // Insertion Mode
        case GLFW_KEY_I:
            mode = 1;
            break;
        // Transformation Mode
        case GLFW_KEY_O:
            mode = 2;
            break;
        // Deletion Mode
        case GLFW_KEY_P:
            mode = 3;
            break;
        // Rotation/Scale Mode
        case GLFW_KEY_R:
            mode = 4;
            break;
        // Color Mode
        case GLFW_KEY_C:
            mode = 5;
            break;
        // Trackball
        case GLFW_KEY_T:
            mode = 6;
            break;
        // Keyframe Mode
        case GLFW_KEY_F:
            is_keyframe = true;
            start_animation = false;
            break;
        case GLFW_KEY_EQUAL:
            camera_position[0] = 0.8f * camera_position[0];
            camera_position[1] = 0.8f * camera_position[1];
            camera_position[2] = 0.8f * camera_position[2];
            break;
        case GLFW_KEY_MINUS:
            camera_position[0] = 1.25f * camera_position[0];
            camera_position[1] = 1.25f * camera_position[1];
            camera_position[2] = 1.25f * camera_position[2];
            break;
        case GLFW_KEY_LEFT_BRACKET:
            camera_mode = 0;
            break;
        case GLFW_KEY_RIGHT_BRACKET:
            camera_mode = 1;
            break;
        case GLFW_KEY_1:
            V << -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f,
                -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,
                -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f;
        default:
            break;
        }

        if (mode == 6)
        {
            float x = camera_position[0];
            float y = camera_position[1];
            float z = camera_position[2];
            float x2z2 = sqrtf(x * x + z * z);
            float ratio;
            float theta = 15. * PI / 180;

            switch (key)
            {
            case GLFW_KEY_W:
                theta = -theta;
                ratio = (x2z2 * cosf(theta) + y * sinf(theta)) / x2z2;
                camera_position[0] = ratio * x;
                camera_position[1] = -x2z2 * sinf(theta) + y * cosf(theta);
                camera_position[2] = ratio * z;
                break;
            case GLFW_KEY_S:
                ratio = (x2z2 * cosf(theta) + y * sinf(theta)) / x2z2;
                camera_position[0] = ratio * x;
                camera_position[1] = -x2z2 * sinf(theta) + y * cosf(theta);
                camera_position[2] = ratio * z;
                break;
            case GLFW_KEY_A:
                theta = -theta;
                camera_position[0] = x * cosf(theta) + z * sinf(theta);
                camera_position[2] = -x * sinf(theta) + z * cosf(theta);
                break;
            case GLFW_KEY_D:
                camera_position[0] = x * cosf(theta) + z * sinf(theta);
                camera_position[2] = -x * sinf(theta) + z * cosf(theta);
                break;
            default:
                break;
            }
        }
        else
        {
            switch (key)
            {
            case GLFW_KEY_W:
                camera_position[1] = camera_position[1] + 1;
                break;
            case GLFW_KEY_S:
                camera_position[1] = camera_position[1] - 1;
                break;
            case GLFW_KEY_A:
                camera_position[0] = camera_position[0] - 0.6;
                camera_position[2] = camera_position[2] + 0.8;
                break;
            case GLFW_KEY_D:
                camera_position[0] = camera_position[0] + 0.6;
                camera_position[2] = camera_position[2] - 0.8;
                break;
            default:
                break;
            }
        }
        

        VBO.update(V);
    }
}

int main()
{

    vector<string> off_files_string{"../data/cube.off", "../data/bunny.off", "../data/bumpy_cube.off"};
    vector<MatrixXf> vertices;
    vector<MatrixXi> faces;

    // Read file to Matrix vector
    for (int file_i = 0; file_i < off_files_string.size(); file_i++)
    {
        ifstream off_file(off_files_string[file_i]);
        string line;
        int number_of_vertices = 0;
        int number_of_faces = 0;

        // Ignore the first line
        getline(off_file, line);

        // Read the numbers from the second line
        if (getline(off_file, line))
        {
            istringstream line_stream(line);
            string element;
            int element_index = 0;

            // Handle each element
            while (getline(line_stream, element, ' '))
            {
                if (element_index == 0)
                {
                    number_of_vertices = stoi(element);
                }
                else if (element_index == 1)
                {
                    number_of_faces = stoi(element);
                }
                element_index++;
            }
        }

        // Declare the vertices and faces Matrices
        MatrixXf vertex = MatrixXf::Zero(number_of_vertices, 3);
        vertices.push_back(vertex);
        MatrixXi face = MatrixXi::Zero(number_of_faces, 3);
        faces.push_back(face);

        // Read the vertices data
        for (int row = 0; row < number_of_vertices; row++)
        {
            if (getline(off_file, line))
            {
                istringstream line_stream(line);
                string element;
                int element_index = 0;

                // Handle each element
                while (getline(line_stream, element, ' '))
                {
                    vertices[file_i](row, element_index) = stof(element);
                    element_index++;
                }
            }
        }

        // Read the faces data
        for (int row = 0; row < number_of_faces; row++)
        {
            if (getline(off_file, line))
            {
                istringstream line_stream(line);
                string element;
                int element_index = 0;

                while (getline(line_stream, element, ' '))
                {
                    if (element_index == 0)
                    {
                        element_index++;
                        continue;
                    }
                    faces[file_i](row, element_index - 1) = stoi(element);
                    element_index++;
                }
            }
        }
    }

    // Rescale and Reposition Picture
    vertices[1].col(0) = vertices[1].col(0) + 0.02 * VectorXf::Ones(vertices[1].rows());
    vertices[1].col(1) = vertices[1].col(1) - 0.1 * VectorXf::Ones(vertices[1].rows());
    vertices[1] = vertices[1] * 13;
    vertices[2] = vertices[2] / 4.37847;

    auto t_start = std::chrono::high_resolution_clock::now();

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
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
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

    // Initialize the VAO
    // A Vertex Array Object (or VAO) is an object that describes how the vertex
    // attributes are stored in a Vertex Buffer Object (or VBO). This means that
    // the VAO is not the actual object storing the vertex data,
    // but the descriptor of the vertex data.
    VertexArrayObject VAO;
    VAO.init();
    VAO.bind();

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();

    V = Eigen::MatrixXf::Zero(3, 36);

    VBO.update(V);

    // Second VBO for colors
    VBO_C.init();

    C = Eigen::MatrixXf::Zero(3, 36);
    C <<
        0.583f,  0.771f,  0.014f,
        0.609f,  0.115f,  0.436f,
        0.327f,  0.483f,  0.844f,
        0.822f,  0.569f,  0.201f,
        0.435f,  0.602f,  0.223f,
        0.310f,  0.747f,  0.185f,
        0.597f,  0.770f,  0.761f,
        0.559f,  0.436f,  0.730f,
        0.359f,  0.583f,  0.152f,
        0.483f,  0.596f,  0.789f,
        0.559f,  0.861f,  0.639f,
        0.195f,  0.548f,  0.859f,
        0.014f,  0.184f,  0.576f,
        0.771f,  0.328f,  0.970f,
        0.406f,  0.615f,  0.116f,
        0.676f,  0.977f,  0.133f,
        0.971f,  0.572f,  0.833f,
        0.140f,  0.616f,  0.489f,
        0.997f,  0.513f,  0.064f,
        0.945f,  0.719f,  0.592f,
        0.543f,  0.021f,  0.978f,
        0.279f,  0.317f,  0.505f,
        0.167f,  0.620f,  0.077f,
        0.347f,  0.857f,  0.137f,
        0.055f,  0.953f,  0.042f,
        0.714f,  0.505f,  0.345f,
        0.783f,  0.290f,  0.734f,
        0.722f,  0.645f,  0.174f,
        0.302f,  0.455f,  0.848f,
        0.225f,  0.587f,  0.040f,
        0.517f,  0.713f,  0.338f,
        0.053f,  0.959f,  0.120f,
        0.393f,  0.621f,  0.362f,
        0.673f,  0.211f,  0.457f,
        0.820f,  0.883f,  0.371f,
        0.982f,  0.099f,  0.879f;

    VBO_C.update(C);

    // Initialize Vertex Starting Points
    VSP << 0., 0., 0.,
        0., 0., 0.;

    // Initialize Keyframe Starting Points
    KSP << 0., 0., 0.,
        0., 0., 0.;

    // Initialize Keyframe Ending Points
    KEP << 0., 0., 0.,
        0., 0., 0.;

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar *vertex_shader = R"glsl(
        #version 150 core
        in vec3 position;
        in vec3 color;
        out vec3 f_color;
        uniform mat4 MVP;
        void main()
        {
            gl_Position = MVP * vec4(position, 1.0);
            f_color = color;
        }
    )glsl";

    const GLchar *fragment_shader = R"glsl(
        #version 150 core
        in vec3 f_color;
        out vec4 outColor;
        uniform vec4 triangleColor;
        void main()
        {
            // outColor = vec4(0.6, 0.6, 0.8, 1);
            outColor = vec4(f_color, 0.5);
        }
    )glsl";

    // Compile the two shaders and upload the binary to the GPU
    // Note that we have to explicitly specify that the output "slot" called outColor
    // is the one that we want in the fragment buffer (and thus on screen)
    program.init(vertex_shader, fragment_shader, "outColor");
    program.bind();

    // The vertex shader wants the position of the vertices as an input.
    // The following line connects the VBO we defined above with the position "slot"
    // in the vertex shader
    program.bindVertexAttribArray("position", VBO);
    program.bindVertexAttribArray("color", VBO_C);

    // Register the keyboard callback
    glfwSetKeyCallback(window, key_callback);

    // Register the cursor callback
    glfwSetCursorPosCallback(window, cursor_position_callback);

    // Register the mouse callback
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Update viewport
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
        // Bind your VAO (not necessary if you have only one)
        VAO.bind();

        // Bind your program
        program.bind();

        // Clear the framebuffer
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);

        // // Enable blending test
        // glEnable(GL_BLEND);
        // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::mat4 Projection;

        if (camera_mode == 0)
        {
            // Projection matrix : 45 degrees Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
            Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        }
        else if (camera_mode == 1)
        {
            // Or, for an ortho camera :
            Projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.0f, 100.0f); // In world coordinates
        }

        glm::mat4 View = glm::lookAt(
            glm::vec3(camera_position[0], camera_position[1], camera_position[2]),
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
        );

        // Calculate transformation
        auto t_now = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 Model = glm::mat4(1.0f);

        // Get size of the window
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float aspect_ratio = float(height) / float(width);

        float aspect_adjust[16] = {
            aspect_ratio, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };

        glm::mat4 Aspect = glm::make_mat4(aspect_adjust);

        // Our ModelViewProjection : multiplication of our 3 matrices
        glm::mat4 MVP = Projection * Aspect * View * Model; // Remember, matrix multiplication is the other way around

        // Set the uniform view value
        glUniformMatrix4fv(program.uniform("MVP"), 1, GL_FALSE, glm::value_ptr(MVP));

        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Deallocate opengl memory
    program.free();
    VAO.free();
    VBO.free();

    // Deallocate glfw internals
    glfwTerminate();
    return 0;
}
