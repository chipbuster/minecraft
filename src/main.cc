#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

// OpenGL library includes
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <debuggl.h>
#include "Terrain.h"
#include "camera.h"
#include "tictoc.h"

int window_width = 800, window_height = 600;

std::ostream& operator<<(std::ostream& os, const glm::vec4 x);

// VBO and VAO descriptors.
enum { kVertexBuffer, kIndexBuffer, kNumVbos };

// These are our VAOs.
enum { kCubeVao, kFloorVao, kNumVaos };

GLuint g_array_objects[kNumVaos]; // This will store the VAO descriptors.
GLuint g_buffer_objects[kNumVaos]
                       [kNumVbos]; // These will store VBO descriptors.

// Include shader program strings
#include "cubedata.cc"
#include "shaders_include.cc"
namespace {
constexpr float m = 1024.0f;
constexpr float t = -10.0f;
}
std::vector<glm::vec4> floor_vertices = {
        {m, t, m, 1.0}, {-m, t, m, 1.0}, {-m, t, -m, 1.0}, {m, t, -m, 1.0}};
std::vector<glm::uvec3> floor_faces = {{0, 2, 1}, {3, 2, 0}};

constexpr unsigned int nCubeInstance =
        32000; // 5x5 chunks, 16x16 each, with spares

void ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error: " << description << "\n";
}

Camera g_camera;
int walk_cam = 0;
int strafe_cam = 0;
int roll_cam = 0;
int lev_cam = 0;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods)
{
    // Note:
    // This is only a list of functions to implement.
    // you may want to re-organize this piece of code.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_F && mods == GLFW_MOD_CONTROL &&
             action == GLFW_RELEASE) {
        g_camera.physics_mode = !g_camera.physics_mode;
    } else if (key == GLFW_KEY_W) {
        if(action == GLFW_PRESS){ walk_cam = 1;}
        if(action == GLFW_RELEASE){ walk_cam = 0;}
    } else if (key == GLFW_KEY_S) {
        if(action == GLFW_PRESS){ walk_cam = -1;}
        if(action == GLFW_RELEASE){ walk_cam = 0;}
    } else if (key == GLFW_KEY_A ) {
        if(action == GLFW_PRESS){ strafe_cam = -1;}
        if(action == GLFW_RELEASE){ strafe_cam = 0;}
    } else if (key == GLFW_KEY_D ) {
        if(action == GLFW_PRESS){ strafe_cam = 1;}
        if(action == GLFW_RELEASE){ strafe_cam = 0;}
    } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        g_camera.jump();
    } else if (key == GLFW_KEY_LEFT) {
        if(action == GLFW_PRESS){ roll_cam = 1;}
        if(action == GLFW_RELEASE){ roll_cam = 0;}
    } else if (key == GLFW_KEY_RIGHT ) {
        if(action == GLFW_PRESS){ roll_cam = -1;}
        if(action == GLFW_RELEASE){ roll_cam = 0;}
    } else if (key == GLFW_KEY_DOWN ) {
        if(action == GLFW_PRESS){ lev_cam = 1;}
        if(action == GLFW_RELEASE){ lev_cam = 0;}
    } else if (key == GLFW_KEY_UP) {
        if(action == GLFW_PRESS){ lev_cam = -1;}
        if(action == GLFW_RELEASE){ lev_cam = 0;}
    } else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
        // No non-FPS mode here
        ((void)0);
    }
    if (key == GLFW_KEY_0 && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_1 && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_2 && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_3 && action != GLFW_RELEASE) {
    } else if (key == GLFW_KEY_4 && action != GLFW_RELEASE) {
    }
}

int g_current_button;
bool g_mouse_pressed;

void MousePosCallback(GLFWwindow* window, double mouse_x, double mouse_y)
{
    static double prev_x, prev_y;
    if (!g_mouse_pressed)
        return;
    if (g_current_button == GLFW_MOUSE_BUTTON_LEFT) {
        g_camera.lm_rotate_cam(mouse_x - prev_x, mouse_y - prev_y);
    } else if (g_current_button == GLFW_MOUSE_BUTTON_RIGHT) {
        g_camera.rm_zoom_cam(mouse_y - prev_y);
    } else if (g_current_button == GLFW_MOUSE_BUTTON_MIDDLE) {
        g_camera.mm_trans_cam(mouse_x - prev_x, mouse_y - prev_y);
    }
    prev_x = mouse_x;
    prev_y = mouse_y;
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    g_mouse_pressed = (action == GLFW_PRESS);
    g_current_button = button;
}

int main(int argc, char* argv[])
{
    std::string window_title = "Minecraft, made by Microsoft";
    if (!glfwInit())
        exit(EXIT_FAILURE);
    glfwSetErrorCallback(ErrorCallback);

    // Set up Terrain
    srand((unsigned)time(0));
    Terrain T(rand());

    // Ask an OpenGL 4.1 core profile context
    // It is required on OSX and non-NVIDIA Linux
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          &window_title[0], nullptr, nullptr);
    CHECK_SUCCESS(window != nullptr);
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    CHECK_SUCCESS(glewInit() == GLEW_OK);
    glGetError(); // clear GLEW's error for it
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MousePosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSwapInterval(1);
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION);   // version as a string
    std::cout << "Renderer: " << renderer << "\n";
    std::cout << "OpenGL version supported:" << version << "\n";

    std::vector<glm::vec4> obj_vertices = CubeData::baseVerts;
    std::vector<glm::uvec3> obj_faces = CubeData::baseFaces;
    std::vector<glm::vec3> offsets;
    std::vector<float> seeds;
    seeds.resize(nCubeInstance);
    offsets.resize(nCubeInstance);

    // Testing
    for (size_t i = 0; i < offsets.size(); i++) {
        offsets[i] = glm::vec3(i, i, i);
        seeds[i] = 1.0;
    }

    glm::vec4 min_bounds = glm::vec4(std::numeric_limits<float>::max());
    glm::vec4 max_bounds = glm::vec4(-std::numeric_limits<float>::max());
    for (const auto& vert : obj_vertices) {
        min_bounds = glm::min(vert, min_bounds);
        max_bounds = glm::max(vert, max_bounds);
    }
    std::cout << "min_bounds = " << glm::to_string(min_bounds) << "\n";
    std::cout << "max_bounds = " << glm::to_string(max_bounds) << "\n";

    // Setup our VAO array.
    CHECK_GL_ERROR(glGenVertexArrays(kNumVaos, &g_array_objects[0]));

    // Switch to the VAO for Geometry.
    CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kCubeVao]));

    // Generate buffer objects
    CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kCubeVao][0]));

    // Setup vertex data in a VBO.
    CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
                                g_buffer_objects[kCubeVao][kVertexBuffer]));
    size_t vertSz = sizeof(float) * obj_vertices.size() * 4;
    size_t offsetSz = sizeof(float) * offsets.size() * 3;
    size_t seedSz = sizeof(float) * seeds.size();
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER, vertSz + offsetSz + seedSz, 0,
                                GL_STATIC_DRAW));
    CHECK_GL_ERROR(
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertSz, obj_vertices.data()));
    CHECK_GL_ERROR(
            glBufferSubData(GL_ARRAY_BUFFER, vertSz, offsetSz, offsets.data()));
    CHECK_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, vertSz + offsetSz, seedSz,
                                   seeds.data()));

    // Enable vertex positions to be passed in under location 0
    CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
    CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    // Enable vertex offsets to be passed in under location 1, instanced
    CHECK_GL_ERROR(
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)vertSz));
    CHECK_GL_ERROR(glEnableVertexAttribArray(1));
    CHECK_GL_ERROR(glVertexAttribDivisor(1, 1)); // Per-instance locations

    // Enable random seeds to pass in location 2, instanced
    CHECK_GL_ERROR(glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0,
                                         (void*)(vertSz + offsetSz)));
    CHECK_GL_ERROR(glEnableVertexAttribArray(2));
    CHECK_GL_ERROR(glVertexAttribDivisor(2, 1)); // Per-instance locations

    // Setup element array buffer.
    CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                g_buffer_objects[kCubeVao][kIndexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                sizeof(uint32_t) * obj_faces.size() * 3,
                                obj_faces.data(), GL_STATIC_DRAW));

    // Setup vertex shader.
    GLuint vertex_shader_id = 0;
    const char* vertex_source_pointer = vertex_shader;
    CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
    CHECK_GL_ERROR(glShaderSource(vertex_shader_id, 1, &vertex_source_pointer,
                                  nullptr));
    glCompileShader(vertex_shader_id);
    CHECK_GL_SHADER_ERROR(vertex_shader_id);

    // Setup geometry shader.
    GLuint geometry_shader_id = 0;
    const char* geometry_source_pointer = geometry_shader;
    CHECK_GL_ERROR(geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER));
    CHECK_GL_ERROR(glShaderSource(geometry_shader_id, 1,
                                  &geometry_source_pointer, nullptr));
    glCompileShader(geometry_shader_id);
    CHECK_GL_SHADER_ERROR(geometry_shader_id);

    // Setup fragment shader.
    GLuint fragment_shader_id = 0;
    const char* fragment_source_pointer = fragment_shader;
    CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(fragment_shader_id, 1,
                                  &fragment_source_pointer, nullptr));
    glCompileShader(fragment_shader_id);
    CHECK_GL_SHADER_ERROR(fragment_shader_id);

    // Let's create our program.
    GLuint program_id = 0;
    CHECK_GL_ERROR(program_id = glCreateProgram());
    CHECK_GL_ERROR(glAttachShader(program_id, vertex_shader_id));
    CHECK_GL_ERROR(glAttachShader(program_id, fragment_shader_id));
    CHECK_GL_ERROR(glAttachShader(program_id, geometry_shader_id));

    // Bind attributes.
    CHECK_GL_ERROR(glBindAttribLocation(program_id, 0, "vertex_position"));
    CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
    glLinkProgram(program_id);
    CHECK_GL_PROGRAM_ERROR(program_id);

    // Get the uniform locations.
    GLint projection_matrix_location = 0;
    CHECK_GL_ERROR(projection_matrix_location =
                           glGetUniformLocation(program_id, "projection"));
    GLint view_matrix_location = 0;
    CHECK_GL_ERROR(view_matrix_location =
                           glGetUniformLocation(program_id, "view"));
    GLint light_position_location = 0;
    CHECK_GL_ERROR(light_position_location =
                           glGetUniformLocation(program_id, "light_position"));

    glm::vec4 light_position = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
    float aspect = 0.0f;
    float theta = 0.0f;
    TicTocTimer timer = tic();

    glm::ivec2 chunkOver(-10000, 100000);
    while (!glfwWindowShouldClose(window)) {
        glm::ivec2 currChunkOver = T.getChunkCoords(g_camera.getEye());
        // Copy in new offset data
        if (currChunkOver != chunkOver) {
            chunkOver = currChunkOver;
            offsets = T.getOffsetsForRender(g_camera.getEye(),
                                            glm::vec2(-15.0, 0.0));
            seeds = T.getSeedsForRender(g_camera.getEye());

            CHECK_GL_ERROR(
                    glBindBuffer(GL_ARRAY_BUFFER,
                                 g_buffer_objects[kCubeVao][kVertexBuffer]));
            size_t vertSz = sizeof(float) * obj_vertices.size() * 4;
            size_t offsetSz = sizeof(float) * nCubeInstance * 3;
            size_t seedSz = sizeof(float) * seeds.size();
            CHECK_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, vertSz, offsetSz,
                                           offsets.data()));
            CHECK_GL_ERROR(glBufferSubData(GL_ARRAY_BUFFER, vertSz + offsetSz,
                                           seedSz, seeds.data()));
        }

        // Setup some basic window stuff.
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);

        // Switch to the Geometry VAO.
        CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kCubeVao]));

        // Compute the projection matrix.
        aspect = static_cast<float>(window_width) / window_height;
        glm::mat4 projection_matrix =
                glm::perspective(glm::radians(90.0f), aspect, 0.0001f, 512.0f);

        // Compute the view matrix
        glm::mat4 view_matrix = g_camera.get_view_matrix();

        // Use our program.
        CHECK_GL_ERROR(glUseProgram(program_id));

        // Pass uniforms in.
        CHECK_GL_ERROR(glUniformMatrix4fv(projection_matrix_location, 1,
                                          GL_FALSE, &projection_matrix[0][0]));
        CHECK_GL_ERROR(glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE,
                                          &view_matrix[0][0]));
        CHECK_GL_ERROR(
                glUniform4fv(light_position_location, 1, &light_position[0]));

        // Draw our triangles.
        CHECK_GL_ERROR(
                glDrawElementsInstanced(GL_TRIANGLES, obj_faces.size() * 3,
                                        GL_UNSIGNED_INT, 0, nCubeInstance));

        // Let camera velocities decay
        double timeDiff = toc(&timer);
        g_camera.update_physics(
                timeDiff, T.getChunk(T.getChunkCoords(g_camera.getEye())), offsets);
        std::cout << '\r';
        std::cout << "FPS = " << 1.0 / timeDiff;

        // Apply camera transforms
        if(walk_cam){g_camera.ws_walk_cam(walk_cam);}
        if(strafe_cam){g_camera.ad_strafe_cam(strafe_cam);}
        if(walk_cam){g_camera.lr_roll_cam(roll_cam);}
        if(walk_cam){g_camera.ud_move_cam(lev_cam);}

        // Poll and swap.
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    std::cout << std::endl;
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
