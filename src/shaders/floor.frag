R"zzz(#version 330 core
flat in vec4 normal;
in vec4 light_direction;
in vec4 world_pos;
in float seed;
in vec4 cube_pos;
uniform mat4 view;
out vec4 fragment_color;


// These functions nicked from https://stackoverflow.com/a/17479300
// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u ); x ^= ( x >>  6u ); x += ( x <<  3u );
    x ^= ( x >> 11u ); x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
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

    vec2 plane_pos_mod = fract(plane_pos);

    float dot_nl = dot(normalize(light_direction), view * normalize(normal));
    dot_nl = clamp(dot_nl, 0.0, 1.0);
    fragment_color = clamp( fragment_color * dot_nl, 0.0, 1.0);
    fragment_color[3] = 1.0;
}
)zzz"