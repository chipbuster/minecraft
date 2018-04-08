R"zzz(
#version 330 core
layout(location = 0) in vec4 vertex_position;
layout(location = 1) in vec3 cube_offset;
layout(location = 2) in float in_seed;
uniform mat4 view;
uniform vec4 light_position;
out vec4 vs_light_direction;
out vec4 u_pos;
out float vs_seed;

void main()
{
    u_pos = vertex_position + vec4(cube_offset, 0.0);
    gl_Position = view * u_pos;
    vs_light_direction = -gl_Position + view * light_position;
    vs_seed = in_seed;
}
)zzz"