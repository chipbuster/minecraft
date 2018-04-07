R"zzz(#version 330 core
flat in vec4 normal;
in vec4 light_direction;
uniform mat4 view;
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
    dot_nl = clamp(dot_nl, 0.15, 1.0);
    fragment_color = clamp(dot_nl * color, 0.05, 1.0);
}
)zzz"