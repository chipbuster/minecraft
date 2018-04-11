R"zzz(#version 330 core
flat in vec4 normal;
in vec4 light_direction;
in vec4 world_pos;
in float seed;
in vec4 cube_pos;
uniform mat4 view;
out vec4 fragment_color;

#define PI 3.1415926535897932384626433832795

float rand(vec2 co){
    return PI * fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 circle(float inp){
    return vec2(cos(inp), sin(inp));
}

float perlinFade(float t){
    return 6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t;
}

float perlin(vec2 coord, vec2 grad[4])
{
    vec2 from0 = coord - vec2(0, 0);
    vec2 from1 = coord - vec2(1, 0);
    vec2 from2 = coord - vec2(0, 1);
    vec2 from3 = coord - vec2(1, 1);

    float inf0 = dot(grad[0], from0);
    float inf1 = dot(grad[1], from1);
    float inf2 = dot(grad[2], from2);
    float inf3 = dot(grad[3], from3);

    float top = mix(inf2, inf3, perlinFade(coord[0]));
    float bot = mix(inf0, inf1, perlinFade(coord[0]));
    float mid = mix(bot, top, perlinFade(coord[1]));

    return (mid / (sqrt(2) / 2) + 1) / 2;
}

void main()
{
    // Get plane positions
    vec2 plane_pos;
    if(abs(normal[0]) > 0.9){
        plane_pos = world_pos.yz;
    }else if(abs(normal[1]) > 0.9){
        plane_pos = world_pos.xz;
    }
    else{
        plane_pos = world_pos.xy;
    }

    // Coordinates for octaves
    vec2 plane_pos_base = floor(plane_pos);
    vec2 plane_pos_mod = fract(plane_pos);


    // O1 noise
    vec2 o1[4];
    o1[0] = circle(seed * rand(plane_pos_base));
    o1[1] = circle(seed * rand(plane_pos_base + vec2(1,0)));
    o1[2] = circle(seed * rand(plane_pos_base + vec2(0,1)));
    o1[3] = circle(seed * rand(plane_pos_base + vec2(1,1)));

    float col = perlin(plane_pos_mod, o1);

    // O2 noise
    vec2 o2[9];
    o2[0] = circle(seed * rand(plane_pos_base));
    o2[1] = circle(seed * rand(plane_pos_base + vec2(0.5,0)));
    o2[2] = circle(seed * rand(plane_pos_base + vec2(1,0)));
    o2[3] = circle(seed * rand(plane_pos_base + vec2(0,0.5)));
    o2[4] = circle(seed * rand(plane_pos_base + vec2(0.5,0.5)));
    o2[5] = circle(seed * rand(plane_pos_base + vec2(1,0.5)));
    o2[6] = circle(seed * rand(plane_pos_base + vec2(0,1)));
    o2[7] = circle(seed * rand(plane_pos_base + vec2(0.5,1)));
    o2[8] = circle(seed * rand(plane_pos_base + vec2(1,1)));

    // Which vectors to use?
    int start;
    if(plane_pos_mod.x < 0.5 && plane_pos_mod.y < 0.5){
        start = 0;
    }
    if(plane_pos_mod.x > 0.5 && plane_pos_mod.y < 0.5){
        start = 1;
    }
    if(plane_pos_mod.x < 0.5 && plane_pos_mod.y > 0.5){
        start = 3;
    }
    if(plane_pos_mod.x > 0.5 && plane_pos_mod.y > 0.5){
        start = 4;
    }
    vec2 o2grad[4];
    o2grad[0] = o2[start];
    o2grad[1] = o2[start + 1];
    o2grad[2] = o2[start + 3];
    o2grad[3] = o2[start + 4];

    //col += perlin(mod(plane_pos_mod * 2.0,2.0), o2grad);

    vec4 baseCol;
    if(world_pos.y < -8.999){
        baseCol = vec4(0.1,0.4,0.8,1.0);
    }
    else if(world_pos.y < -5.999){
        baseCol = vec4(0.3,0.6,0.1,1.0);
    }else{
        baseCol = vec4(1.0,1.0,1.0,1.0);
    }

    fragment_color = col * baseCol; 
    fragment_color += vec4(0.10,0.10,0.10, 1.0);

    float dot_nl = dot(normalize(light_direction), view * normalize(normal));
    dot_nl = clamp(dot_nl, 0.2, 1.0);
    fragment_color = clamp( fragment_color * dot_nl, 0.0, 1.0);
    fragment_color[3] = 1.0;
}
)zzz"
