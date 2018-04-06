// C++ 11 String Literal
// See http://en.cppreference.com/w/cpp/language/string_literal
const char* vertex_shader =
        R"zzz(#version 410 core
in vec4 vertex_position;
uniform mat4 view;
uniform vec4 light_position;
out vec4 vs_light_direction;
out vec4 vs_u_pos;
void main()
{
    vs_u_pos = vertex_position;
    gl_Position = view * vertex_position;
    vs_light_direction = light_position - vertex_position;
}
)zzz";

const char* light_vertex_shader =
        R"zzz(#version 410 core
in vec4 vertex_position;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 center;
void main()
{
    vec4 shifted = (vertex_position - vec4(vec3(0.5),0.0));
    vec4 enlarged = 2 * shifted;
    gl_Position = projection * view * enlarged;
}
)zzz";

const char* cube_geometry_shader =
        R"zzz(#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection;
uniform mat4 view;
in vec4 vs_light_direction[];
in vec4 vs_u_pos[];
flat out vec4 normal;
out vec4 light_direction;
out vec4 world_pos;
out vec3 bary_pos;
void main()
{
    int n = 0;
    vec3 side2 = (vs_u_pos[2] - vs_u_pos[0]).xyz;
    vec3 side1 = (vs_u_pos[1] - vs_u_pos[0]).xyz;
    vec3 normal3 = normalize(cross(side1,side2));
    vec4 faceNormal = vec4(normal3[0], normal3[1], normal3[2], 0.0);
    for (n = 0; n < gl_in.length(); n++) {
        light_direction = vs_light_direction[n];
        gl_Position = projection * gl_in[n].gl_Position;
        normal = faceNormal;
        world_pos = vs_u_pos[n];

        bary_pos = vec3(0.0);
        bary_pos[n] = 1.0;

        EmitVertex();
    }
    EndPrimitive();
}
)zzz";

const char* floor_geometry_shader =
        R"zzz(#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection;
uniform mat4 view;
in vec4 te_light_direction[];
in vec4 te_u_pos[];
flat out vec4 normal;
out vec4 light_direction;
out vec4 world_pos;
out vec3 bary_pos;
void main()
{
    int n = 0;
    vec3 side2 = (te_u_pos[2] - te_u_pos[0]).xyz;
    vec3 side1 = (te_u_pos[1] - te_u_pos[0]).xyz;
    vec3 normal3 = normalize(cross(side2,side1));
    vec4 faceNormal = vec4(normal3[0], normal3[1], normal3[2], 0.0);
    for (n = 0; n < gl_in.length(); n++) {
        light_direction = te_light_direction[n];
        gl_Position = projection * gl_in[n].gl_Position;
        normal = faceNormal;
        world_pos = te_u_pos[n];

        bary_pos = vec3(0.0);
        bary_pos[n] = 1.0;

        EmitVertex();
    }
    EndPrimitive();
}
)zzz";

const char* fragment_shader =
        R"zzz(#version 410 core
flat in vec4 normal;
in vec4 light_direction;
in vec3 bary_pos;
uniform mat4 view;
uniform float frames;
uniform float unif_frames;
out vec4 fragment_color;
void main()
{
    vec4 color = vec4(1.0,1.0,1.0,1.0); 
    float thresh = 0.98;
    float pos_x = dot(normal, vec4(1.0,0.0,0.0,0.0));
    float pos_y = dot(normal, vec4(0.0,1.0,0.0,0.0));
    float pos_z = dot(normal, vec4(0.0,0.0,1.0,0.0));
    float neg_x = dot(normal, vec4(-1.0,0.0,0.0,0.0));
    float neg_y = dot(normal, vec4(0.0,-1.0,0.0,0.0));
    float neg_z = dot(normal, vec4(0.0,0.0,-1.0,0.0));

    if(pos_x > thresh){
        color = vec4(1.0,0.0,0.0,1.0);
    }
    if(pos_y > thresh){
        color = vec4(0.0,1.0,0.0,1.0);
    }
    if(pos_z > thresh){
        color = vec4(0.0,0.0,1.0,1.0);
    }
    if(neg_x > thresh){
        color = vec4(1.0,0.0,0.0,1.0);
    }
    if(neg_y > thresh){
        color = vec4(0.0,1.0,0.0,1.0);
    }
    if(neg_z > thresh){
        color = vec4(0.0,0.0,1.0,1.0);
    }

    float dot_nl = dot(normalize(light_direction), view * normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);
    fragment_color = clamp(dot_nl * color, 0.0, 1.0);

    // Draw wireframes
    vec3 d = fwidth(bary_pos);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.5, bary_pos);
    float eFact = min(min(a3.x,a3.y),a3.z);

    float min_coord = min(min(bary_pos.x, bary_pos.y),bary_pos.z);
    if(eFact < 0.3 && frames > 0 && unif_frames > 0){
        fragment_color = vec4(0.0,1.0,0.0,1.0);
    }else if(min_coord < 0.01 && frames > 0 && unif_frames < 0){
        fragment_color = vec4(0.0,1.0,0.0,1.0);
    }
}
)zzz";

const char* floor_fragment_shader =
        R"zzz(#version 410 core
        #extension GL_OES_standard_derivatives : enable
flat in vec4 normal;
in vec4 light_direction;
in vec4 world_pos;
in vec3 bary_pos;
uniform mat4 view;
uniform float frames;
uniform float unif_frames;
out vec4 fragment_color;
void main()
{
    float newX, newZ;
    if(world_pos[0] < 0){
        newX = abs(world_pos[0]) + 1;
    }else{
        newX = abs(world_pos[0]);
    }
    if(world_pos[2] < 0){
        newZ = abs(world_pos[2]) + 1;
    }else{
        newZ = abs(world_pos[2]);
    }

    newX = mod(newX, 2);
    newZ = mod(newZ, 2);

    if(newX < 1 && newZ < 1 || newX > 1 && newZ > 1){
        fragment_color = vec4(0.0,0.0,0.0,1.0);
    }else{
        fragment_color = vec4(1.0,1.0,1.0,1.0);
    }

    float dot_nl = dot(normalize(light_direction), normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);

    fragment_color = clamp( fragment_color * dot_nl, 0.0, 1.0);
    fragment_color[3] = 1.0;

    // Draw wireframes
    vec3 d = fwidth(bary_pos);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.5, bary_pos);
    float eFact = min(min(a3.x,a3.y),a3.z);

    float min_coord = min(min(bary_pos.x, bary_pos.y),bary_pos.z);
    if(eFact < 0.3 && frames > 0 && unif_frames > 0){
        fragment_color = vec4(0.0,1.0,0.0,1.0);
    }else if(min_coord < 0.01 && frames > 0 && unif_frames < 0){
        fragment_color = vec4(0.0,1.0,0.0,1.0);
    }
}
)zzz";

/*
 */

const char* tess_ctrl_floor_shader =
        R"zzz(#version 410 core

layout(vertices = 3) out;
in vec4 vs_u_pos[];
in vec4 vs_light_direction[];
uniform float TessLevelInner;
uniform float TessLevelOuter;
out vec4 tc_u_pos[];
out vec4 tc_light_direction[];

#define ID gl_InvocationID

void main()
{
    if(ID == 0){
      gl_TessLevelInner[0] = TessLevelInner;
      gl_TessLevelOuter[0] = TessLevelOuter;
      gl_TessLevelOuter[1] = TessLevelOuter;
      gl_TessLevelOuter[2] = TessLevelOuter;
    }
    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
    tc_u_pos[ID] = vs_u_pos[ID];
    tc_light_direction[ID] = vs_light_direction[ID];
}

)zzz";

const char* tess_eval_floor_shader =
        R"zzz(#version 410 core
layout(triangles, equal_spacing, cw) in;

in vec4 tc_u_pos[];
in vec4 tc_light_direction[];
out vec4 te_u_pos;
out vec4 te_light_direction;
uniform mat4 view;

void main()
{
    vec4 p0 = gl_TessCoord.x * gl_in[0].gl_Position;
    vec4 p1 = gl_TessCoord.y * gl_in[1].gl_Position;
    vec4 p2 = gl_TessCoord.z * gl_in[2].gl_Position;

    gl_Position = p0 + p1 + p2;

    vec4 u0 = gl_TessCoord.x * tc_u_pos[0];
    vec4 u1 = gl_TessCoord.y * tc_u_pos[1];
    vec4 u2 = gl_TessCoord.z * tc_u_pos[2];

    te_u_pos = u0 + u1 + u2;

    vec4 t0 = gl_TessCoord.x * tc_light_direction[0];
    vec4 t1 = gl_TessCoord.y * tc_light_direction[1];
    vec4 t2 = gl_TessCoord.z * tc_light_direction[2];

   te_light_direction = t0 + t1 + t2;
}
)zzz";

const char* ocean_geometry_shader =
        R"zzz(#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection;
uniform mat4 view;
in vec4 te_light_direction[];
in vec4 te_u_pos[];
in vec4 te_vert_normal[];
out vec4 normal;
out vec4 light_direction;
out vec4 world_pos;
out vec4 cam_dir;
out vec3 bary_pos;
void main()
{
    int n;
    for (n = 0; n < gl_in.length(); n++) {
        light_direction = te_light_direction[n];
        gl_Position = projection * gl_in[n].gl_Position;
        normal = te_vert_normal[n];
        world_pos = te_u_pos[n];
        cam_dir = -gl_Position;

        bary_pos = vec3(0.0);
        bary_pos[n] = 1.0;

        EmitVertex();
    }
    EndPrimitive();
}
)zzz";

const char* ocean_fragment_shader =
        R"zzz(#version 410 core
in vec4 normal;
in vec4 light_direction;
in vec4 world_pos;
in vec4 cam_dir;
in vec3 bary_pos;
uniform mat4 view;
uniform mat4 projection;
uniform float frames;
uniform float unif_frames;
out vec4 fragment_color;
void main()
{
    mat4 inv = inverse(view);

    vec4 amb_color = vec4(0.0, 0.476, 0.745, 1.0);
    vec4 lmb_color = vec4(0.1687, 0.5120, 0.3193, 1.0);
    vec4 spc_color = vec4(1.0,1.0,1.0,1.0);
    fragment_color = vec4(0.0,0.0,0.0,0.0);

    vec4 amb_coeff = vec4(0.3,0.3,0.3,1.0);
    vec4 lmb_coeff = vec4(0.5,0.6,0.6,1.0);
    vec4 spc_coeff = vec4(0.7,0.7,0.7,1.0);
    float spc_pow = 4.0;

    // Ambient shading
    fragment_color += amb_color * amb_coeff;

    // Lambertian shading
    vec4 lmb_contrib = lmb_color * lmb_coeff * 
          max(dot(normalize(light_direction), normalize(normal)), 0);
    fragment_color += lmb_contrib;

    // Specular shading
    vec3 max_reflect_dir = reflect(-normalize(light_direction.xyz), normalize(normal.xyz));
    vec3 mrd = normalize((projection * view * vec4(max_reflect_dir,0.0)).xyz);
    float spec_amt = max(dot(mrd, normalize(cam_dir.xyz)),0);
    vec4 spc_contrib = spc_color * spc_coeff * pow(spec_amt, spc_pow);
    fragment_color += spc_contrib;

    fragment_color = clamp( fragment_color, 0.0, 1.0);
    fragment_color[3] = 1.0;

    // Draw wireframes
    vec3 d = fwidth(bary_pos);
    vec3 a3 = smoothstep(vec3(0.0), d * 1.5, bary_pos);
    float eFact = min(min(a3.x,a3.y),a3.z);

    float min_coord = min(min(bary_pos.x, bary_pos.y),bary_pos.z);
    if(eFact < 0.3 && frames > 0 && unif_frames > 0){
        fragment_color = vec4(0.0,1.0,0.0,1.0);
    }else if(min_coord < 0.01 && frames > 0 && unif_frames < 0){
        fragment_color = vec4(0.0,1.0,0.0,1.0);
    }
}
)zzz";

/*
 */

const char* tess_ctrl_ocean_shader =
        R"zzz(#version 410 core

layout(vertices = 4) out;
in vec4 vs_u_pos[];
in vec4 vs_light_direction[];
uniform float TessLevelInner;
uniform float TessLevelOuter;
uniform float tw_timer;
out vec4 tc_u_pos[];
out vec4 tc_light_direction[];

#define ID gl_InvocationID

void main()
{
    if(ID == 0){
       float TLO = TessLevelOuter;
       float TLI = TessLevelInner;
       if(tw_timer > 0){
            vec2 tw_center = vec2(tw_timer,0.0);
            float dist0 = length(vs_u_pos[0].xz - tw_center );
            float dist1 = length(vs_u_pos[1].xz - tw_center );
            float dist2 = length(vs_u_pos[2].xz - tw_center );
            float dist3 = length(vs_u_pos[3].xz - tw_center );

            float mdist = max(max(max(dist0,dist1),dist2),dist3);
            float tessFact;
            if(mdist < 3){ tessFact = 8.0; }
            else if(mdist < 4){tessFact = 6.0;}
            else if(mdist < 5){tessFact = 4.0;}
            else if(mdist < 6){tessFact = 3.0;}
            else if(mdist < 7){tessFact = 2.0;}
            else{tessFact = 1.0; }
            
            TLI *= tessFact;
            TLO *= tessFact;
        }


        gl_TessLevelInner[0] = TLI;
        gl_TessLevelInner[1] = TLI;
        gl_TessLevelOuter[0] = TLO;
        gl_TessLevelOuter[1] = TLO;
        gl_TessLevelOuter[2] = TLO;
        gl_TessLevelOuter[3] = TLO;
    }

    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
    tc_u_pos[ID] = vs_u_pos[ID];
    tc_light_direction[ID] = vs_light_direction[ID];
}

)zzz";

/*

*/

/* Use constant parameters for waves */
const char* tess_eval_ocean_shader =
        R"zzz(#version 410 core
layout(quads, equal_spacing, cw) in;

in vec4 tc_u_pos[];
in vec4 tc_light_direction[];
out vec4 te_u_pos;
out vec4 te_light_direction;
out vec4 te_vert_normal;
uniform mat4 view;
uniform float timer;
uniform float tw_timer;

void tidal(in float x, in float z, in float t, out float h, out float dx, out float dz){
    if(t < 0){
        h = 0; dx = 0; dz = 0; return;
    }
    float sig = 2.0;
    float A = 15.0;
    vec2 center = vec2(t, 0.0);
    vec2 delta = vec2(x,z) - center;
    h = A * exp(-dot(delta,delta) / sig);
    dx = h * (t - x);
    dz = h * -z;
}

void wave1(in float x, in float z, in float t, out float h, out float dx, out float dz){
    vec2 D = normalize(vec2(0.7,-0.2));
    float A = 0.3;
    float L = 2.0;
    float w = 2/L;
    float S = 0.7;
    float p = S * 2 / L;

    h = A * sin(w * dot(D,vec2(x,z)) + t * p);
    dx = A * cos(w * dot(D,vec2(x,z)) + t * p) * w * D.x;
    dz = A * cos(w * dot(D,vec2(x,z)) + t * p) * w * D.y;
}
void wave2(in float x, in float z, in float t, out float h, out float dx, out float dz){
    vec2 D = normalize(vec2(0.3,0.5));
    float A = 0.5;
    float L = 4.0;
    float w = 2/L;
    float S = 1.5;
    float p = S * 2 / L;

    h = A * sin(w * dot(D,vec2(x,z)) + t * p);
    dx = A * cos(w * dot(D,vec2(x,z)) + t * p) * w * D.x;
    dz = A * cos(w * dot(D,vec2(x,z)) + t * p) * w * D.y;
}
void wave3(in float x, in float z, in float t, out float h, out float dx, out float dz){
    vec2 D = normalize(vec2(1.0,0.4)); // 1.0, 0.4
    float A = 1.2;
    float L = 8.0;
    float w = 2/L;
    float S = 2.0; 
    float p = S * 2 / L;

    h = A * sin(w * dot(D,vec2(x,z)) + t * p);
    dx = A * cos(w * dot(D,vec2(x,z)) + t * p) * w * D.x;
    dz = A * cos(w * dot(D,vec2(x,z)) + t * p) * w * D.y;
}

void main()
{
    vec4 p0 = mix(gl_in[1].gl_Position, gl_in[0].gl_Position, gl_TessCoord.x);
    vec4 p1 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);

    vec4 u0 = mix(tc_u_pos[1], tc_u_pos[0], gl_TessCoord.x);
    vec4 u1 = mix(tc_u_pos[2], tc_u_pos[3], gl_TessCoord.x);
    te_u_pos = mix(u0,u1,gl_TessCoord.y);

    float h1 = 0, h2 = 0, h3 = 0 , h4 = 0;
    float dx1=0, dx2=0, dx3=0, dx4=0;
    float dz1=0, dz2=0, dz3=0, dz4=0;
    wave1(te_u_pos.x, te_u_pos.z, timer, h1,dx1,dz1);
    wave2(te_u_pos.x, te_u_pos.z, timer, h2,dx2,dz2);
    wave3(te_u_pos.x, te_u_pos.z, timer, h3,dx3,dz3);
    tidal(te_u_pos.x, te_u_pos.z, tw_timer, h4,dx4,dz4);

    te_u_pos.y = te_u_pos.y + h1 + h2 + h3 + h4;

    vec3 norml =  normalize(vec3(-dx1 - dx2 - dx3 - dx4, 1.0, -dz1 - dz2 - dz3 - dz4));
    te_vert_normal = vec4(norml, 0.0);

    gl_Position = view * te_u_pos;

    vec4 t0 = mix(tc_light_direction[1], tc_light_direction[0], gl_TessCoord.x);
    vec4 t1 = mix(tc_light_direction[2], tc_light_direction[3], gl_TessCoord.x);
    te_light_direction = mix(t0,t1,gl_TessCoord.y);
}
)zzz";


const char* tess_ctrl_light_shader =
        R"zzz(#version 410 core

layout(vertices = 3) out;

#define ID gl_InvocationID

void main()
{
    if(ID == 0){
        float TLI = 4;
        float TLO = 4;
        gl_TessLevelInner[0] = TLI;
        gl_TessLevelOuter[0] = TLO;
        gl_TessLevelOuter[1] = TLO;
        gl_TessLevelOuter[2] = TLO;
    }

    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
}
)zzz";

/* Use constant parameters for waves */
const char* tess_eval_light_shader =
        R"zzz(#version 410 core
layout(triangles, equal_spacing, ccw) in;
uniform vec4 center;
uniform mat4 view;
uniform mat4 projection;
out vec4 normal;

void main()
{
    vec4 viewC = projection * view * center;

    vec4 p0 = gl_TessCoord.x * gl_in[0].gl_Position;
    vec4 p1 = gl_TessCoord.y * gl_in[1].gl_Position;
    vec4 p2 = gl_TessCoord.z * gl_in[2].gl_Position;
    vec4 rawPos = 2 * (p0 + p1 + p2);

    normal = projection * view * vec4(normalize(rawPos.xyz),0.0);

    gl_Position = rawPos + viewC + 2 * normal;
}
)zzz";

const char* light_fragment_shader =
        R"zzz(#version 410 core
in vec4 normal;
uniform mat4 view;
out vec4 fragment_color;
void main()
{
    fragment_color = vec4(0.988,0.831,0.251,1.0);
}
)zzz";

