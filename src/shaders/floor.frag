R"zzz(#version 330 core
flat in vec4 normal;
in vec4 light_direction;
in vec4 world_pos;
in float seed;
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

    if(newX > 1 && newZ > 1 || newX < 1 && newZ < 1){
        fragment_color = vec4(1.0,1.0,1.0,1.0);
    }else{
        fragment_color = vec4(0.0,0.0,0.0,0.0);
    }

    float dot_nl = dot(normalize(light_direction), view * normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);
    fragment_color = clamp( fragment_color * dot_nl, 0.0, 1.0);
    fragment_color[3] = 1.0;
}
)zzz"