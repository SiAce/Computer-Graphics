// This example is heavily based on the tutorial at https://open.gl

// Sine and Cosine
#include "math.h"

#define PI 3.14159265

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

// VertexBufferObject wrapper
VertexBufferObject VBO;
VertexBufferObject VBO_C;

// Contains the vertex positions
Eigen::MatrixXf V(2, 300);

// Contains the per-vertex color
Eigen::MatrixXf C(3, 300);

// Contains the vertex starting points
Eigen::MatrixXf VSP(2, 3);

// Contains the keyframe starting points
Eigen::MatrixXf KSP(2, 3);

// Contains the vertex ending points
Eigen::MatrixXf KEP(2, 3);

// Contains the view transformation
Eigen::Matrix4f view(4, 4);

// Mode control
int mode = 0;

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

    if (mode == 1)
    {
        if (click_time == 2)
        {
            V.col(3 * triangle_number + 1) << xworld, yworld;
        }
        else if (click_time == 3)
        {
            V.col(3 * triangle_number + 2) << xworld, yworld;
        }
    }
    else if (mode == 2)
    {
        if (selected_triangle != -1)
        {
            if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            {
                double delta_x = xworld - xworld_start;
                double delta_y = yworld - yworld_start;

                // Vertex Delta Matrix
                Eigen::MatrixXf VDP(2, 3);

                VDP << delta_x, delta_x, delta_x,
                    delta_y, delta_y, delta_y;

                V.block<2, 3>(0, 3 * selected_triangle) = VSP + VDP;
            }
        }
    }

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

    // Update the position of the first vertex if the left button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        selected_triangle = -1;

        if (mode == 1)
        {
            if (click_time == 1)
            {
                V.col(3 * triangle_number + 0) << xworld, yworld;
                click_time++;
            }
            else if (click_time == 2)
            {
                V.col(3 * triangle_number + 1) << xworld, yworld;
                click_time++;
            }
            else if (click_time == 3)
            {
                V.col(3 * triangle_number + 2) << xworld, yworld;
                triangle_number++;
                click_time = 1;
            }
        }
        else if (mode == 2)
        {
            for (int i = triangle_number - 1; i >= 0; i--)
            {
                // test if i-th triangle is selected, if so break loop, record the selected triangle
                if (point_in_triangle(V(0, 3 * i), V(1, 3 * i), V(0, 3 * i + 1), V(1, 3 * i + 1), V(0, 3 * i + 2), V(1, 3 * i + 2), xworld, yworld))
                {
                    selected_triangle = i;
                    break;
                }
            }

            if (selected_triangle != -1)
            {
                xworld_start = xworld;
                yworld_start = yworld;

                VSP = V.block<2, 3>(0, 3 * selected_triangle);
            }
        }
        else if (mode == 3)
        {
            for (int i = triangle_number - 1; i >= 0; i--)
            {
                // test if i-th triangle is selected, if so break loop, record the selected triangle
                if (point_in_triangle(V(0, 3 * i), V(1, 3 * i), V(0, 3 * i + 1), V(1, 3 * i + 1), V(0, 3 * i + 2), V(1, 3 * i + 2), xworld, yworld))
                {
                    selected_triangle = i;
                    break;
                }
            }

            if (selected_triangle != -1)
            {
                // Copy the triangle information
                V.block(0, 3 * selected_triangle, 2, 3 * (triangle_number - selected_triangle - 1)) =
                    V.block(0, 3 * (selected_triangle + 1), 2, 3 * (triangle_number - selected_triangle - 1));

                triangle_number--;
                selected_triangle = -1;
            }
        }
        else if (mode == 4)
        {
            for (int i = triangle_number - 1; i >= 0; i--)
            {
                // test if i-th triangle is selected, if so break loop, record the selected triangle
                if (point_in_triangle(V(0, 3 * i), V(1, 3 * i), V(0, 3 * i + 1), V(1, 3 * i + 1), V(0, 3 * i + 2), V(1, 3 * i + 2), xworld, yworld))
                {
                    selected_triangle = i;
                    break;
                }
            }
        }
        else if (mode == 5)
        {
            selected_vertex = -1;

            double nearest_distance_square = 10.;
            // Select the nearest vertex
            for (int i = 0; i < 3 * triangle_number; i++)
            {
                double distance_square = (xworld - V(0, i)) * (xworld - V(0, i)) + (yworld - V(1, i)) * (yworld - V(1, i));

                if (distance_square < nearest_distance_square)
                {
                    selected_vertex = i;
                    nearest_distance_square = distance_square;
                }
            }
        }

        if (is_keyframe)
        {
            keyframe_triangle = -1;
            for (int i = triangle_number - 1; i >= 0; i--)
            {
                // test if i-th triangle is selected, if so break loop, record the selected triangle
                if (point_in_triangle(V(0, 3 * i), V(1, 3 * i), V(0, 3 * i + 1), V(1, 3 * i + 1), V(0, 3 * i + 2), V(1, 3 * i + 2), xworld, yworld))
                {
                    keyframe_triangle = i;
                    break;
                }
            }
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        if (mode == 2)
        {
            selected_triangle = -1;
            xworld_start = 0;
            yworld_start = 0;
        }
    }

    // Upload the change to the GPU
    VBO.update(V);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS)
    {
        Eigen::Matrix4f view_transform(4, 4);

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
        // Keyframe Mode
        case GLFW_KEY_F:
            is_keyframe = true;
            start_animation = false;
            break;
        case GLFW_KEY_EQUAL:
            view_transform << 1.2, 0, 0, 0,
                0, 1.2, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
            view = view_transform * view;
            break;
        case GLFW_KEY_MINUS:
            view_transform << 0.8, 0, 0, 0,
                0, 0.8, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
            view = view_transform * view;
            break;
        case GLFW_KEY_W:
            view_transform << 1, 0, 0, 0,
                0, 1, 0, -0.4,
                0, 0, 1, 0,
                0, 0, 0, 1;
            view = view_transform * view;
            break;
        case GLFW_KEY_S:
            view_transform << 1, 0, 0, 0,
                0, 1, 0, 0.4,
                0, 0, 1, 0,
                0, 0, 0, 1;
            view = view_transform * view;
            break;
        case GLFW_KEY_A:
            view_transform << 1, 0, 0, 0.4,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
            view = view_transform * view;
            break;
        case GLFW_KEY_D:
            view_transform << 1, 0, 0, -0.4,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
            view = view_transform * view;
            break;
        default:
            break;
        }

        // Rotation/Scale
        if (mode == 4 && selected_triangle != -1)
        {
            VSP = V.block<2, 3>(0, 3 * selected_triangle);

            float xc = (VSP(0, 0) + VSP(0, 1) + VSP(0, 2)) / 3;
            float yc = (VSP(1, 0) + VSP(1, 1) + VSP(1, 2)) / 3;

            // Vertex Center Points
            Eigen::MatrixXf VCP(2, 3);

            // Transform Matrix
            Eigen::Matrix2f transform(2, 2);

            transform << 1.0, 0.0,
                0.0, 1.0;

            double alpha, scale;

            VCP << xc, xc, xc,
                yc, yc, yc;

            VSP = VSP - VCP;

            switch (key)
            {
            // Rotate 10 degree clockwise
            case GLFW_KEY_H:
                alpha = -10.0 * PI / 180;
                transform << cos(alpha), -sin(alpha),
                    sin(alpha), cos(alpha);
                break;
            // Rotate 10 degree counter-clockwise
            case GLFW_KEY_J:
                alpha = 10.0 * PI / 180;
                transform << cos(alpha), -sin(alpha),
                    sin(alpha), cos(alpha);
                break;
            // Scaled up 25%
            case GLFW_KEY_K:
                scale = 1.25;
                transform << scale, 0,
                    0, scale;
                break;
            // Scaled down 25%
            case GLFW_KEY_L:
                scale = 0.75;
                transform << scale, 0,
                    0, scale;
                break;
            default:
                break;
            }

            VSP = transform * VSP;
            VSP = VSP + VCP;
            V.block<2, 3>(0, 3 * selected_triangle) = VSP;
        }

        if (mode == 5 && selected_vertex != -1)
        {
            switch (key)
            {
            case GLFW_KEY_1:
                C.col(selected_vertex) << 0.1, 0.5, 0.9;
                break;
            case GLFW_KEY_2:
                C.col(selected_vertex) << 0.2, 0.6, 0.0;
                break;
            case GLFW_KEY_3:
                C.col(selected_vertex) << 0.3, 0.7, 0.1;
                break;
            case GLFW_KEY_4:
                C.col(selected_vertex) << 0.4, 0.8, 0.2;
                break;
            case GLFW_KEY_5:
                C.col(selected_vertex) << 0.5, 0.9, 0.3;
                break;
            case GLFW_KEY_6:
                C.col(selected_vertex) << 0.6, 0.0, 0.4;
                break;
            case GLFW_KEY_7:
                C.col(selected_vertex) << 0.7, 0.1, 0.5;
                break;
            case GLFW_KEY_8:
                C.col(selected_vertex) << 0.8, 0.2, 0.6;
                break;
            case GLFW_KEY_9:
                C.col(selected_vertex) << 0.9, 0.3, 0.7;
                break;
            default:
                break;
            }
        }

        if (is_keyframe && keyframe_triangle != -1)
        {
            switch (key)
            {
            // Start Keyframing
            case GLFW_KEY_Z:
                KSP = V.block<2, 3>(0, 3 * keyframe_triangle);
                break;
            // End Keyframing
            case GLFW_KEY_X:
                KEP = V.block<2, 3>(0, 3 * keyframe_triangle);
                /* code */
                start_animation = true;
                t_start = std::chrono::high_resolution_clock::now();
                break;
            default:
                break;
            }
        }

        VBO.update(V);
        VBO_C.update(C);
    }
}

int main(void)
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
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
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
    VBO.update(V);

    // Second VBO for colors
    VBO_C.init();

    C = Eigen::MatrixXf::Zero(3, 300);

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

    // Initialize view matrix
    view << 1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    // Initialize the OpenGL Program
    // A program controls the OpenGL pipeline and it must contains
    // at least a vertex shader and a fragment shader to be valid
    Program program;
    const GLchar *vertex_shader =
        "#version 150 core\n"
        "in vec2 position;"
        "uniform mat4 view;"
        "in vec3 color;"
        "out vec3 f_color;"
        "void main()"
        "{"
        "    gl_Position = view * vec4(position, 0.0, 1.0);"
        "    f_color = color;"
        "}";
    const GLchar *fragment_shader =
        "#version 150 core\n"
        "in vec3 f_color;"
        "out vec4 outColor;"
        "uniform vec4 triangleColor;"
        "void main()"
        "{"
        "if (f_color == vec3(0.,0.,0.))"
        "{"
        "    outColor = vec4(triangleColor);"
        "} else"
        "{"
        "    outColor = vec4(f_color, 0.5);"
        "}"
        "}";

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

        // Enable blending test
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Set the uniform view value
        glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, view.data());

        // Set the uniform triangleColor value
        for (int i = 0; i < triangle_number; i++)
        {
            if (i == keyframe_triangle)
            {
                if (is_keyframe && start_animation)
                {
                    /* code */
                    auto t_now = std::chrono::high_resolution_clock::now();
                    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

                    if (time < 1.)
                    {
                        V(0, keyframe_triangle * 3) = (1 - time) * KSP(0, 0) + time * KEP(0, 0);
                        V(1, keyframe_triangle * 3) = (1 - time) * KSP(1, 0) + time * KEP(1, 0);

                        V(0, keyframe_triangle * 3 + 1) = (1 - time) * KSP(0, 1) + time * KEP(0, 1);
                        V(1, keyframe_triangle * 3 + 1) = (1 - time) * KSP(1, 1) + time * KEP(1, 1);

                        V(0, keyframe_triangle * 3 + 2) = (1 - time) * KSP(0, 2) + time * KEP(0, 2);
                        V(1, keyframe_triangle * 3 + 2) = (1 - time) * KSP(1, 2) + time * KEP(1, 2);

                        VBO.update(V);

                    }
                    
                }

                glUniform4f(program.uniform("triangleColor"), 0.0f, 0.0f, 1.0f, 0.5f);
                glDrawArrays(GL_TRIANGLES, 3 * i, 3);
            }
            else if (i == selected_triangle)
            {
                glUniform4f(program.uniform("triangleColor"), 0.0f, 0.0f, 1.0f, 0.5f);
                glDrawArrays(GL_TRIANGLES, 3 * i, 3);
            }
            else
            {
                glUniform4f(program.uniform("triangleColor"), 1.0f, 0.0f, 0.0f, 0.5f);
                glDrawArrays(GL_TRIANGLES, 3 * i, 3);
            }
        }

        if (mode == 1)
        {
            if (click_time == 2)
            {
                // Set the uniform value
                glUniform4f(program.uniform("triangleColor"), 0.0f, 0.0f, 0.0f, 0.5f);

                // Draw a line
                glDrawArrays(GL_LINES, triangle_number * 3, 2);
            }
            else if (click_time == 3)
            {
                // Set the uniform value
                glUniform4f(program.uniform("triangleColor"), 0.0f, 0.0f, 0.0f, 0.5f);

                // Draw a line loop
                glDrawArrays(GL_LINE_LOOP, triangle_number * 3, 3);
            }
        }

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
