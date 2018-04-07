R"zzz(
#version 330 core
layout(location = 0) in vec4 vertex_position;
layout(location = 1) in vec2 cube_offset;
uniform mat4 view;
uniform vec4 light_position;
out vec4 vs_light_direction;
out vec4 u_pos;
void main()
{
    u_pos = vertex_position;
    gl_Position = view * vertex_position;
    vs_light_direction = -gl_Position + view * light_position;
}
)zzz"