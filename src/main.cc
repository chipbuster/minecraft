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
#include "camera.h"
#include "menger.h"

#include <ctime>

int window_width = 800, window_height = 600;

std::ostream& operator<<(std::ostream& os, const glm::vec4 x);
std::ostream& operator<<(std::ostream& os, const glm::uvec3 x);

// VBO and VAO descriptors.
enum { kVertexBuffer, kIndexBuffer, kNumVbos };

// These are our VAOs.
enum { kGeometryVao, kFloorVao, kOceanVao, kLightVao, kNumVaos };

GLuint g_array_objects[kNumVaos]; // This will store the VAO descriptors.
GLuint g_buffer_objects[kNumVaos]
                       [kNumVbos]; // These will store VBO descriptors.

// Use lexical include to add shader programs
#include "shaders.cc"

void ErrorCallback(int error, const char* description)
{
    std::cerr << "GLFW Error: " << description << "\n";
}

std::shared_ptr<Menger> g_menger;
Camera g_camera;
std::vector<glm::vec4>* verts_ptr = nullptr;
std::vector<glm::uvec3>* faces_ptr = nullptr;
struct timespec start, end, tw_start, tw_end;
bool draw_frames = false;
bool unif_frames = false;
bool draw_faces = true;
int tess_inner = 4;
int tess_outer = 4;
bool draw_ocean = false;
bool tidal_wave_active = false;

void KeyCallback(GLFWwindow* window, int key, int scancode, int action,
                 int mods)
{
    // Note:
    // This is only a list of functions to implement.
    // you may want to re-organize this piece of code.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_S && mods == GLFW_MOD_CONTROL &&
             action == GLFW_RELEASE) {
        std::ofstream objfile("geometry.obj");
        if (verts_ptr != nullptr && faces_ptr != nullptr) {
            std::cout << "Dumping geometry into geometry.obj..." << std::flush;
            objfile << Menger::objdump(*verts_ptr, *faces_ptr);
            objfile.close();
            std::cout << "Finished!" << std::endl;
        } else {
            std::cout << "Sorry, the geometry isn't ready yet." << std::endl;
        }
    } else if (key == GLFW_KEY_O && mods == GLFW_MOD_CONTROL &&
               action == GLFW_RELEASE) {
        draw_ocean = !draw_ocean;
    } else if (key == GLFW_KEY_F && mods == GLFW_MOD_CONTROL &&
               action == GLFW_RELEASE) {
        draw_faces = !draw_faces;
    } else if (key == GLFW_KEY_T && mods == GLFW_MOD_CONTROL &&
               action == GLFW_RELEASE) {
        tidal_wave_active = true;
        clock_gettime(CLOCK_MONOTONIC, &tw_start);
    } else if (key == GLFW_KEY_W && action != GLFW_RELEASE) {
        g_camera.ws_walk_cam(1);
    } else if (key == GLFW_KEY_U && action != GLFW_RELEASE) {
        unif_frames = !unif_frames;
    } else if (key == GLFW_KEY_S && action != GLFW_RELEASE) {
        g_camera.ws_walk_cam(-1);
    } else if (key == GLFW_KEY_A && action != GLFW_RELEASE) {
        g_camera.ad_strafe_cam(-1);
    } else if (key == GLFW_KEY_D && action != GLFW_RELEASE) {
        g_camera.ad_strafe_cam(1);
    } else if (key == GLFW_KEY_LEFT && action != GLFW_RELEASE) {
        g_camera.lr_roll_cam(-1);
    } else if (key == GLFW_KEY_RIGHT && action != GLFW_RELEASE) {
        g_camera.lr_roll_cam(1);
    } else if (key == GLFW_KEY_DOWN && action != GLFW_RELEASE) {
        g_camera.ud_move_cam(-1);
    } else if (key == GLFW_KEY_UP && action != GLFW_RELEASE) {
        g_camera.ud_move_cam(1);
    } else if (key == GLFW_KEY_C && action != GLFW_RELEASE) {
        // No non-FPS mode here
        ((void)0);
    } else if (key == GLFW_KEY_F && action != GLFW_RELEASE) {
        draw_frames = !draw_frames;
    } else if (key == GLFW_KEY_EQUAL && action != GLFW_RELEASE) {
        tess_outer++;
    } else if (key == GLFW_KEY_MINUS && action != GLFW_RELEASE) {
        if (tess_outer > 1)
            tess_outer--;
    } else if (key == GLFW_KEY_PERIOD && action != GLFW_RELEASE) {
        tess_inner++;
    } else if (key == GLFW_KEY_COMMA && action != GLFW_RELEASE) {
        if (tess_inner > 1)
            tess_inner--;
    }
    if (!g_menger)
        return; // 0-4 only available in Menger mode.
    if (key == GLFW_KEY_0 && action != GLFW_RELEASE) {
        g_menger->set_nesting_level(0);
    } else if (key == GLFW_KEY_1 && action != GLFW_RELEASE) {
        g_menger->set_nesting_level(1);
    } else if (key == GLFW_KEY_2 && action != GLFW_RELEASE) {
        g_menger->set_nesting_level(2);
    } else if (key == GLFW_KEY_3 && action != GLFW_RELEASE) {
        g_menger->set_nesting_level(3);
    } else if (key == GLFW_KEY_4 && action != GLFW_RELEASE) {
        g_menger->set_nesting_level(4);
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
    std::string window_title = "Menger";
    if (!glfwInit())
        exit(EXIT_FAILURE);
    g_menger = std::make_shared<Menger>();
    glfwSetErrorCallback(ErrorCallback);

    // Set initial clock time for simulation
    uint64_t time_diff;
    clock_gettime(CLOCK_MONOTONIC, &start);

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

    std::vector<glm::vec4> obj_vertices;
    std::vector<glm::uvec3> obj_faces;
    verts_ptr = &obj_vertices; // Initialize pointers for objdump calls
    faces_ptr = &obj_faces;

    g_menger->set_nesting_level(0);
    g_menger->generate_geometry(obj_vertices, obj_faces);
    g_menger->set_clean();

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
    CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kGeometryVao]));

    // Generate buffer objects
    CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kGeometryVao][0]));

    // Setup vertex data in a VBO.
    CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
                                g_buffer_objects[kGeometryVao][kVertexBuffer]));
    // NOTE: We do not send anything right now, we just describe it to OpenGL.
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
                                sizeof(float) * obj_vertices.size() * 4,
                                obj_vertices.data(), GL_STATIC_DRAW));
    CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
    CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    // Setup element array buffer.
    CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                g_buffer_objects[kGeometryVao][kIndexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                sizeof(uint32_t) * obj_faces.size() * 3,
                                obj_faces.data(), GL_STATIC_DRAW));

#include "floorGeom.cc"

    // Switch to VAO for floor and generate VBOs
    CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kFloorVao]));
    CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kFloorVao][0]));

    // Set up vertex data in the VBO
    CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
                                g_buffer_objects[kFloorVao][kVertexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
                                sizeof(float) * floor_vertices.size() * 4,
                                floor_vertices.data(), GL_STATIC_DRAW));
    CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
    CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                g_buffer_objects[kFloorVao][kIndexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                sizeof(uint32_t) * floor_faces.size() * 3,
                                floor_faces.data(), GL_STATIC_DRAW));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Switch to VAO for ocean and generate VBOs
    CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kOceanVao]));
    CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kOceanVao][0]));

    // Set up vertex data in the VBO
    CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
                                g_buffer_objects[kOceanVao][kVertexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
                                sizeof(float) * ocean_vertices.size() * 4,
                                ocean_vertices.data(), GL_STATIC_DRAW));
    CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
    CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                g_buffer_objects[kOceanVao][kIndexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                sizeof(uint32_t) * ocean_faces.size() * 4,
                                ocean_faces.data(), GL_STATIC_DRAW));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Switch to VAO for light and generate VBOs
    CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kLightVao]));
    CHECK_GL_ERROR(glGenBuffers(kNumVbos, &g_buffer_objects[kLightVao][0]));

    // Set up vertex data in the VBO
    const std::vector<glm::vec4>* lightVerts = g_menger->getBaseVerts();
    const std::vector<glm::uvec3>* lightFaces = g_menger->getBaseFaces();
    CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
                                g_buffer_objects[kLightVao][kVertexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
                                sizeof(float) * lightVerts->size() * 4,
                                lightVerts->data(), GL_STATIC_DRAW));
    CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
    CHECK_GL_ERROR(glEnableVertexAttribArray(0));

    CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                g_buffer_objects[kLightVao][kIndexBuffer]));
    CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                sizeof(uint32_t) * lightFaces->size() * 3,
                                lightFaces->data(), GL_STATIC_DRAW));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Setup vertex shader.
    GLuint vertex_shader_id = 0;
    const char* vertex_source_pointer = vertex_shader;
    CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
    CHECK_GL_ERROR(glShaderSource(vertex_shader_id, 1, &vertex_source_pointer,
                                  nullptr));
    glCompileShader(vertex_shader_id);
    CHECK_GL_SHADER_ERROR(vertex_shader_id);

    // Setup cube geometry shader.
    GLuint cube_geometry_shader_id = 0;
    const char* cube_geometry_source_pointer = cube_geometry_shader;
    CHECK_GL_ERROR(cube_geometry_shader_id =
                           glCreateShader(GL_GEOMETRY_SHADER));
    CHECK_GL_ERROR(glShaderSource(cube_geometry_shader_id, 1,
                                  &cube_geometry_source_pointer, nullptr));
    glCompileShader(cube_geometry_shader_id);
    CHECK_GL_SHADER_ERROR(cube_geometry_shader_id);

    // Setup floor geometry shader.
    GLuint floor_geometry_shader_id = 0;
    const char* floor_geometry_source_pointer = floor_geometry_shader;
    CHECK_GL_ERROR(floor_geometry_shader_id =
                           glCreateShader(GL_GEOMETRY_SHADER));
    CHECK_GL_ERROR(glShaderSource(floor_geometry_shader_id, 1,
                                  &floor_geometry_source_pointer, nullptr));
    glCompileShader(floor_geometry_shader_id);
    CHECK_GL_SHADER_ERROR(floor_geometry_shader_id);

    // Setup fragment shader.
    GLuint fragment_shader_id = 0;
    const char* fragment_source_pointer = fragment_shader;
    CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(fragment_shader_id, 1,
                                  &fragment_source_pointer, nullptr));
    glCompileShader(fragment_shader_id);
    CHECK_GL_SHADER_ERROR(fragment_shader_id);

    // Setup floor TCS
    GLuint tcs_floor_shader_id = 0;
    const char* tcs_floor_source_pointer = tess_ctrl_floor_shader;
    CHECK_GL_ERROR(tcs_floor_shader_id =
                           glCreateShader(GL_TESS_CONTROL_SHADER));
    CHECK_GL_ERROR(glShaderSource(tcs_floor_shader_id, 1,
                                  &tcs_floor_source_pointer, nullptr));
    glCompileShader(tcs_floor_shader_id);
    CHECK_GL_SHADER_ERROR(tcs_floor_shader_id);

    // Setup floor TES
    GLuint tes_floor_shader_id = 0;
    const char* tes_floor_source_pointer = tess_eval_floor_shader;
    CHECK_GL_ERROR(tes_floor_shader_id =
                           glCreateShader(GL_TESS_EVALUATION_SHADER));
    CHECK_GL_ERROR(glShaderSource(tes_floor_shader_id, 1,
                                  &tes_floor_source_pointer, nullptr));
    glCompileShader(tes_floor_shader_id);
    CHECK_GL_SHADER_ERROR(tes_floor_shader_id);

    // Setup floor fragment shader
    GLuint floor_fragment_shader_id = 0;
    const char* floor_fragment_source_pointer = floor_fragment_shader;
    CHECK_GL_ERROR(floor_fragment_shader_id =
                           glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(floor_fragment_shader_id, 1,
                                  &floor_fragment_source_pointer, nullptr));
    glCompileShader(floor_fragment_shader_id);
    CHECK_GL_SHADER_ERROR(floor_fragment_shader_id);

    // Setup ocean TCS
    GLuint tcs_ocean_shader_id = 0;
    const char* tcs_ocean_source_pointer = tess_ctrl_ocean_shader;
    CHECK_GL_ERROR(tcs_ocean_shader_id =
                           glCreateShader(GL_TESS_CONTROL_SHADER));
    CHECK_GL_ERROR(glShaderSource(tcs_ocean_shader_id, 1,
                                  &tcs_ocean_source_pointer, nullptr));
    glCompileShader(tcs_ocean_shader_id);
    CHECK_GL_SHADER_ERROR(tcs_ocean_shader_id);

    // Setup ocean TES
    GLuint tes_ocean_shader_id = 0;
    const char* tes_ocean_source_pointer = tess_eval_ocean_shader;
    CHECK_GL_ERROR(tes_ocean_shader_id =
                           glCreateShader(GL_TESS_EVALUATION_SHADER));
    CHECK_GL_ERROR(glShaderSource(tes_ocean_shader_id, 1,
                                  &tes_ocean_source_pointer, nullptr));
    glCompileShader(tes_ocean_shader_id);
    CHECK_GL_SHADER_ERROR(tes_ocean_shader_id);

    // Setup ocean fragment shader
    GLuint ocean_fragment_shader_id = 0;
    const char* ocean_fragment_source_pointer = ocean_fragment_shader;
    CHECK_GL_ERROR(ocean_fragment_shader_id =
                           glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(ocean_fragment_shader_id, 1,
                                  &ocean_fragment_source_pointer, nullptr));
    glCompileShader(ocean_fragment_shader_id);
    CHECK_GL_SHADER_ERROR(ocean_fragment_shader_id);

    // Setup ocean geometry shader
    GLuint ocean_geometry_shader_id = 0;
    const char* ocean_geometry_source_pointer = ocean_geometry_shader;
    CHECK_GL_ERROR(ocean_geometry_shader_id =
                           glCreateShader(GL_GEOMETRY_SHADER));
    CHECK_GL_ERROR(glShaderSource(ocean_geometry_shader_id, 1,
                                  &ocean_geometry_source_pointer, nullptr));
    glCompileShader(ocean_geometry_shader_id);
    CHECK_GL_SHADER_ERROR(ocean_geometry_shader_id);

    // Setup shaders for light source
    // Setup light TES
    GLuint tess_eval_light_shader_id = 0;
    const char* tess_eval_light_source_pointer = tess_eval_light_shader;
    CHECK_GL_ERROR(tess_eval_light_shader_id =
                           glCreateShader(GL_TESS_EVALUATION_SHADER));
    CHECK_GL_ERROR(glShaderSource(tess_eval_light_shader_id, 1,
                                  &tess_eval_light_source_pointer, nullptr));
    glCompileShader(tess_eval_light_shader_id);
    CHECK_GL_SHADER_ERROR(tess_eval_light_shader_id);

    // Set up light vertex shader
    GLuint light_vertex_shader_id = 0;
    const char* light_vertex_source_pointer = light_vertex_shader;
    CHECK_GL_ERROR(light_vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
    CHECK_GL_ERROR(glShaderSource(light_vertex_shader_id, 1,
                                  &light_vertex_source_pointer, nullptr));
    glCompileShader(light_vertex_shader_id);
    CHECK_GL_SHADER_ERROR(light_vertex_shader_id);

    // Set up light TCS
    GLuint tess_ctrl_light_shader_id = 0;
    const char* tess_ctrl_light_source_pointer = tess_ctrl_light_shader;
    CHECK_GL_ERROR(tess_ctrl_light_shader_id =
                           glCreateShader(GL_TESS_CONTROL_SHADER));
    CHECK_GL_ERROR(glShaderSource(tess_ctrl_light_shader_id, 1,
                                  &tess_ctrl_light_source_pointer, nullptr));
    glCompileShader(tess_ctrl_light_shader_id);
    CHECK_GL_SHADER_ERROR(tess_ctrl_light_shader_id);

    // Set up light fragment shader
    GLuint light_fragment_shader_id = 0;
    const char* light_fragment_source_pointer = light_fragment_shader;
    CHECK_GL_ERROR(light_fragment_shader_id =
                           glCreateShader(GL_FRAGMENT_SHADER));
    CHECK_GL_ERROR(glShaderSource(light_fragment_shader_id, 1,
                                  &light_fragment_source_pointer, nullptr));
    glCompileShader(light_fragment_shader_id);
    CHECK_GL_SHADER_ERROR(light_fragment_shader_id);

    // Let's create our program.
    GLuint program_id = 0;
    CHECK_GL_ERROR(program_id = glCreateProgram());
    CHECK_GL_ERROR(glAttachShader(program_id, vertex_shader_id));
    CHECK_GL_ERROR(glAttachShader(program_id, fragment_shader_id));
    CHECK_GL_ERROR(glAttachShader(program_id, cube_geometry_shader_id));

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
    GLint draw_frames_flag_location = 0;
    CHECK_GL_ERROR(draw_frames_flag_location =
                           glGetUniformLocation(program_id, "frames"));
    GLint unif_frames_flag_location = 0;
    CHECK_GL_ERROR(unif_frames_flag_location =
                           glGetUniformLocation(program_id, "unif_frames"));

    // Note: you can reuse the vertex and geometry shader objects
    GLuint floor_program_id = 0;
    GLint floor_projection_matrix_location = 0;
    GLint floor_view_matrix_location = 0;
    GLint floor_light_position_location = 0;

    // Smash shaders together
    CHECK_GL_ERROR(floor_program_id = glCreateProgram());
    CHECK_GL_ERROR(glAttachShader(floor_program_id, vertex_shader_id));
    CHECK_GL_ERROR(glAttachShader(floor_program_id, tcs_floor_shader_id));
    CHECK_GL_ERROR(glAttachShader(floor_program_id, tes_floor_shader_id));
    CHECK_GL_ERROR(glAttachShader(floor_program_id, floor_fragment_shader_id));
    CHECK_GL_ERROR(glAttachShader(floor_program_id, floor_geometry_shader_id));

    CHECK_GL_ERROR(
            glBindAttribLocation(floor_program_id, 0, "vertex_position"));
    CHECK_GL_ERROR(
            glBindFragDataLocation(floor_program_id, 0, "fragment_color"));
    glLinkProgram(floor_program_id);
    CHECK_GL_PROGRAM_ERROR(floor_program_id);

    // Get uniforms
    CHECK_GL_ERROR(floor_projection_matrix_location = glGetUniformLocation(
                           floor_program_id, "projection"));
    CHECK_GL_ERROR(floor_view_matrix_location =
                           glGetUniformLocation(floor_program_id, "view"));
    CHECK_GL_ERROR(floor_light_position_location = glGetUniformLocation(
                           floor_program_id, "light_position"));
    GLint tess_level_inner_location = 0;
    CHECK_GL_ERROR(tess_level_inner_location = glGetUniformLocation(
                           floor_program_id, "TessLevelInner"));
    GLint tess_level_outer_location = 0;
    CHECK_GL_ERROR(tess_level_outer_location = glGetUniformLocation(
                           floor_program_id, "TessLevelOuter"));
    GLint draw_frames_floor_flag_location = 0;
    CHECK_GL_ERROR(draw_frames_floor_flag_location =
                           glGetUniformLocation(floor_program_id, "frames"));
    GLint unif_frames_floor_flag_location = 0;
    CHECK_GL_ERROR(unif_frames_floor_flag_location = glGetUniformLocation(
                           floor_program_id, "unif_frames"));

    // Note: you can reuse the vertex and geometry shader objects
    GLuint ocean_program_id = 0;
    GLint ocean_projection_matrix_location = 0;
    GLint ocean_view_matrix_location = 0;
    GLint ocean_light_position_location = 0;
    GLint ocean_timer_location = 0;
    GLint draw_frames_ocean_flag_location = 0;
    GLint tidal_wave_timer_location = 0;
    GLint unif_frames_ocean_flag_location = 0;

    // Smash shaders together
    CHECK_GL_ERROR(ocean_program_id = glCreateProgram());
    CHECK_GL_ERROR(glAttachShader(ocean_program_id, vertex_shader_id));
    CHECK_GL_ERROR(glAttachShader(ocean_program_id, tcs_ocean_shader_id));
    CHECK_GL_ERROR(glAttachShader(ocean_program_id, tes_ocean_shader_id));
    CHECK_GL_ERROR(glAttachShader(ocean_program_id, ocean_fragment_shader_id));
    CHECK_GL_ERROR(glAttachShader(ocean_program_id, ocean_geometry_shader_id));

    CHECK_GL_ERROR(
            glBindAttribLocation(ocean_program_id, 0, "vertex_position"));
    CHECK_GL_ERROR(
            glBindFragDataLocation(ocean_program_id, 0, "fragment_color"));
    glLinkProgram(ocean_program_id);
    CHECK_GL_PROGRAM_ERROR(ocean_program_id);

    // Get uniforms
    CHECK_GL_ERROR(ocean_projection_matrix_location = glGetUniformLocation(
                           ocean_program_id, "projection"));
    CHECK_GL_ERROR(ocean_view_matrix_location =
                           glGetUniformLocation(ocean_program_id, "view"));
    CHECK_GL_ERROR(ocean_light_position_location = glGetUniformLocation(
                           ocean_program_id, "light_position"));
    CHECK_GL_ERROR(tess_level_inner_location = glGetUniformLocation(
                           ocean_program_id, "TessLevelInner"));
    CHECK_GL_ERROR(tess_level_outer_location = glGetUniformLocation(
                           ocean_program_id, "TessLevelOuter"));
    CHECK_GL_ERROR(draw_frames_ocean_flag_location =
                           glGetUniformLocation(ocean_program_id, "frames"));
    CHECK_GL_ERROR(ocean_timer_location =
                           glGetUniformLocation(ocean_program_id, "timer"));
    CHECK_GL_ERROR(tidal_wave_timer_location =
                           glGetUniformLocation(ocean_program_id, "tw_timer"));
    CHECK_GL_ERROR(unif_frames_ocean_flag_location = glGetUniformLocation(
                           ocean_program_id, "unif_frames"));

    // Create program for light
    GLuint light_program_id = 0;
    CHECK_GL_ERROR(light_program_id = glCreateProgram());
    CHECK_GL_ERROR(glAttachShader(light_program_id, light_vertex_shader_id));
    CHECK_GL_ERROR(glAttachShader(light_program_id, tess_ctrl_light_shader_id));
    CHECK_GL_ERROR(glAttachShader(light_program_id, tess_eval_light_shader_id));
    CHECK_GL_ERROR(glAttachShader(light_program_id, light_fragment_shader_id));

    // Bind attributes.
    CHECK_GL_ERROR(glBindAttribLocation(program_id, 0, "vertex_position"));
    CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
    glLinkProgram(program_id);
    CHECK_GL_PROGRAM_ERROR(program_id);

    // Link program
    CHECK_GL_ERROR(
            glBindAttribLocation(light_program_id, 0, "vertex_position"));
    CHECK_GL_ERROR(
            glBindFragDataLocation(light_program_id, 0, "fragment_color"));
    glLinkProgram(light_program_id);
    CHECK_GL_PROGRAM_ERROR(light_program_id);

    // Get uniforms
    GLuint light_projection_matrix_location = 0;
    CHECK_GL_ERROR(light_projection_matrix_location = glGetUniformLocation(
                           light_program_id, "projection"));
    GLuint light_view_matrix_location = 0;
    CHECK_GL_ERROR(light_view_matrix_location =
                           glGetUniformLocation(light_program_id, "view"));
    GLuint light_center_location = 0;
    CHECK_GL_ERROR(light_center_location =
                           glGetUniformLocation(light_program_id, "center"));
    GLuint draw_frames_light_location = 0;
    CHECK_GL_ERROR(draw_frames_light_location =
                           glGetUniformLocation(light_program_id, "frames"));



    glm::vec4 light_position = glm::vec4(-10.0f, 10.0f, 0.0f, 1.0f);
    float aspect = 0.0f;
    float theta = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        // Setup some basic window stuff.
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_LESS);

        // Switch to the Geometry VAO.
        CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kGeometryVao]));

        if (g_menger && g_menger->is_dirty()) {
            g_menger->generate_geometry(obj_vertices, obj_faces);
            g_menger->set_clean();

            // Redefine the
            CHECK_GL_ERROR(glBindBuffer(
                    GL_ARRAY_BUFFER,
                    g_buffer_objects[kGeometryVao][kVertexBuffer]));
            CHECK_GL_ERROR(
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                 g_buffer_objects[kGeometryVao][kIndexBuffer]));
            CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
                                        sizeof(float) * obj_vertices.size() * 4,
                                        obj_vertices.data(), GL_STATIC_DRAW));
            CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                                        sizeof(uint32_t) * obj_faces.size() * 3,
                                        obj_faces.data(), GL_STATIC_DRAW));
        }

        // Compute the projection matrix.
        aspect = static_cast<float>(window_width) / window_height;
        glm::mat4 projection_matrix =
                glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.0f);

        // Compute the view matrix
        glm::mat4 view_matrix = g_camera.get_view_matrix();

        // Do we draw wireframes?
        float frames_flag = draw_frames ? 1.0f : -1.0f;
        float u_frames_flag = unif_frames ? 1.0f : -1.0f;

        // Use our program.
        CHECK_GL_ERROR(glUseProgram(program_id));

        // Pass uniforms in.
        CHECK_GL_ERROR(glUniformMatrix4fv(projection_matrix_location, 1,
                                          GL_FALSE, &projection_matrix[0][0]));
        CHECK_GL_ERROR(glUniform1f(draw_frames_flag_location, frames_flag));
        CHECK_GL_ERROR(glUniform1f(unif_frames_flag_location, u_frames_flag));
        CHECK_GL_ERROR(glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE,
                                          &view_matrix[0][0]));
        CHECK_GL_ERROR(
                glUniform4fv(light_position_location, 1, &light_position[0]));

        // Draw our triangles.
        CHECK_GL_ERROR(glPolygonMode(GL_FRONT_AND_BACK,
                                     draw_faces ? GL_FILL : GL_LINE));
        CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, obj_faces.size() * 3,
                                      GL_UNSIGNED_INT, 0));

        if (draw_ocean) {
            // Note: What you need to do is
            // 0. Set the time for the ocean
            float tw_elapsed_sec;
            clock_gettime(CLOCK_MONOTONIC, &end);
            time_diff = 1000000000L * (end.tv_sec - start.tv_sec) +
                        end.tv_nsec - start.tv_nsec;
            float elapsed_sec = static_cast<float>(time_diff) / 1000000000.0f;

            if (tidal_wave_active) {
                clock_gettime(CLOCK_MONOTONIC, &tw_end);
                time_diff = 1000000000L * (tw_end.tv_sec - tw_start.tv_sec) +
                            tw_end.tv_nsec - tw_start.tv_nsec;
                tw_elapsed_sec = static_cast<float>(time_diff) / 1000000000.0f;
            } else {
                tw_elapsed_sec = -1.0f;
            }
            if (tw_elapsed_sec > 25) {
                tidal_wave_active = false;
            }
            // 	1. Switch VAO
            CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kOceanVao]));
            CHECK_GL_ERROR(
                    glBindBuffer(GL_ARRAY_BUFFER,
                                 g_buffer_objects[kOceanVao][kVertexBuffer]));
            CHECK_GL_ERROR(
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                 g_buffer_objects[kOceanVao][kIndexBuffer]));
            // 	2. Switch Program
            CHECK_GL_ERROR(glUseProgram(ocean_program_id));
            // 	3. Pass Uniforms
            CHECK_GL_ERROR(
                    glUniform1f(draw_frames_ocean_flag_location, frames_flag));
            CHECK_GL_ERROR(
                    glUniform1f(tess_level_inner_location, (float)tess_inner));
            CHECK_GL_ERROR(
                    glUniform1f(tess_level_outer_location, (float)tess_outer));
            CHECK_GL_ERROR(
                    glUniform1f(tidal_wave_timer_location, tw_elapsed_sec));
            CHECK_GL_ERROR(glUniform1f(unif_frames_ocean_flag_location,
                                       u_frames_flag));
            CHECK_GL_ERROR(
                    glUniform1f(ocean_timer_location, (float)elapsed_sec));
            CHECK_GL_ERROR(glUniformMatrix4fv(ocean_projection_matrix_location,
                                              1, GL_FALSE,
                                              &projection_matrix[0][0]));
            CHECK_GL_ERROR(glUniformMatrix4fv(ocean_view_matrix_location, 1,
                                              GL_FALSE, &view_matrix[0][0]));
            CHECK_GL_ERROR(glUniform4fv(ocean_light_position_location, 1,
                                        &light_position[0]));
            // 	4. Call glDrawElements, since input geometry is
            // 	indicated by VAO.
            CHECK_GL_ERROR(glPolygonMode(GL_FRONT_AND_BACK,
                                         draw_faces ? GL_FILL : GL_LINE));
            CHECK_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 4));
            CHECK_GL_ERROR(glDrawElements(GL_PATCHES, ocean_faces.size() * 4,
                                          GL_UNSIGNED_INT, 0));

        } else {
            // Note: What you need to do is
            // 	1. Switch VAO
            CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kFloorVao]));
            CHECK_GL_ERROR(
                    glBindBuffer(GL_ARRAY_BUFFER,
                                 g_buffer_objects[kFloorVao][kVertexBuffer]));
            CHECK_GL_ERROR(
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                 g_buffer_objects[kFloorVao][kIndexBuffer]));
            // 	2. Switch Program
            CHECK_GL_ERROR(glUseProgram(floor_program_id));
            // 	3. Pass Uniforms
            CHECK_GL_ERROR(
                    glUniform1f(draw_frames_floor_flag_location, frames_flag));
            CHECK_GL_ERROR(glUniform1f(unif_frames_floor_flag_location,
                                       u_frames_flag));
            CHECK_GL_ERROR(
                    glUniform1f(tess_level_inner_location, (float)tess_inner));
            CHECK_GL_ERROR(
                    glUniform1f(tess_level_outer_location, (float)tess_outer));
            CHECK_GL_ERROR(glUniformMatrix4fv(floor_projection_matrix_location,
                                              1, GL_FALSE,
                                              &projection_matrix[0][0]));
            CHECK_GL_ERROR(glUniformMatrix4fv(floor_view_matrix_location, 1,
                                              GL_FALSE, &view_matrix[0][0]));
            CHECK_GL_ERROR(glUniform4fv(floor_light_position_location, 1,
                                        &light_position[0]));
            // 	4. Call glDrawElements, since input geometry is
            // 	indicated by VAO.
            CHECK_GL_ERROR(glPolygonMode(GL_FRONT_AND_BACK,
                                         draw_faces ? GL_FILL : GL_LINE));
            CHECK_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 3));
            CHECK_GL_ERROR(glDrawElements(GL_PATCHES, floor_faces.size() * 3,
                                          GL_UNSIGNED_INT, 0));
        }

        // Switch to the light program
        CHECK_GL_ERROR(glBindVertexArray(g_array_objects[kLightVao]));
        CHECK_GL_ERROR(glBindBuffer(
                GL_ARRAY_BUFFER, g_buffer_objects[kLightVao][kVertexBuffer]));
        CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,
                                    g_buffer_objects[kLightVao][kIndexBuffer]));

        CHECK_GL_ERROR(glUseProgram(light_program_id));

        // Pass uniforms
        CHECK_GL_ERROR(glUniformMatrix4fv(light_projection_matrix_location, 1,
                                          GL_FALSE, &projection_matrix[0][0]));
        CHECK_GL_ERROR(glUniformMatrix4fv(light_view_matrix_location, 1,
                                          GL_FALSE, &view_matrix[0][0]));
        CHECK_GL_ERROR(
                glUniform4fv(light_center_location, 1, &light_position[0]));

        CHECK_GL_ERROR(glPatchParameteri(GL_PATCH_VERTICES, 3));
        CHECK_GL_ERROR(glDrawElements(GL_PATCHES, lightFaces->size() * 3,
                                      GL_UNSIGNED_INT, 0));

        // Poll and swap.
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
