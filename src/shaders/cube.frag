R"zzz(#version 330 core
flat in vec4 normal;
in vec4 light_direction;
in float seed;
in vec4 world_pos;
uniform mat4 view;
out vec4 fragment_color;

highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

vec2 circleSample(float theta) {
    return vec2(cos(theta), sin(theta));
}

float perlinFade(float t)
{
    return 6 * t * t * t * t * t - 15 * t * t * t * t + 10 * t * t * t;
}

float perlinNoiseSquare(vec2 coords, vec2 grad0, vec2 grad1, vec2 grad2, vec2 grad3)
{
    vec2 from0 = coords - vec2(0,0);
    vec2 from1 = coords - vec2(1,0);
    vec2 from2 = coords - vec2(0,1);
    vec2 from3 = coords - vec2(1,1);
    float inf0 = dot(grad0, from0);
    float inf1 = dot(grad1, from1);
    float inf2 = dot(grad2, from2);
    float inf3 = dot(grad3, from3);
    float top = mix(inf2, inf3, perlinFade(coords[0]));
    float bot = mix(inf0, inf1, perlinFade(coords[0]));
    float mid = mix(bot, top, perlinFade(coords[1]));
    return mid;
}

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

    float x = mod(world_pos[0], 1.0);
    float z = mod(world_pos[2], 1.0);
    vec2 grad0 = seed * vec2(5,2.1); // Need to actually compute these 
    vec2 grad1 = seed * vec2(3,4.5); // Need to actually compute these
    vec2 grad2 = seed * vec2(6.7,2); // Need to actually compute these
    vec2 grad3 = seed * vec2(9.3,1); // Need to actually compute these
    vec2 coord = vec2(x,z);
    float noise = perlinNoiseSquare(coord, grad0, grad1, grad2, grad3);


    // comment out to see RGB cubes
    color = vec4(0, noise, 0, 1.0);

    float dot_nl = dot(normalize(light_direction), view * normalize(normal));
    dot_nl = clamp(dot_nl, 0.15, 1.0);
    fragment_color = clamp(dot_nl * color, 0.05, 1.0);
}
)zzz"
