R"zzz(#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection;
uniform mat4 view;
in vec4 vs_light_direction[];
in vec4 u_pos[];
in float vs_seed[];
flat out vec4 normal;
out vec4 light_direction;
out vec4 world_pos;
out float seed;

void main()
{
    int n = 0;
    mat4 inv = inverse(view);
    vec3 side2 = (u_pos[2] - u_pos[0]).xyz;
    vec3 side1 = (u_pos[1] - u_pos[0]).xyz;
    vec3 normal3 = normalize(cross(side1,side2));
    vec4 faceNormal = vec4(normal3[0], normal3[1], normal3[2], 0.0);
    for (n = 0; n < gl_in.length(); n++) {
        light_direction = vs_light_direction[n];
        gl_Position = projection * gl_in[n].gl_Position;
        normal = faceNormal;
        seed = vs_seed[0];
        world_pos = u_pos[n];
        EmitVertex();
    }
    EndPrimitive();
}
)zzz"