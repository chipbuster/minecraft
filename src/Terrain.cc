#include "Terrain.h"
#include <cmath>

constexpr double pi = 3.14159265358979323846264338;

Chunk::Chunk(const glm::vec2& location, int extent,
             std::mt19937& gen)
{
    this->tex_seed = gen();
    this->per_seed = gen();
    this->loc = location;
    this->extent = extent;
}

constexpr float perlinFade(float t){
    return 6 * t * t * t * t * t 
        - 15 * t * t * t * t
        + 10 * t * t * t;
}

/* coords: coordinates to take noise at. Scaled to be on a 0-1 square
   grads: gradients at corners. grad[0] is at (0,0), grad[1] is at (1,0),
                                grad[2] is at (0,1), grad[3] is at (1,1) */
float perlinNoiseSquare(const glm::vec2& coords, const std::vector<glm::vec2>& grad ){
    glm::vec2 from0 = coords - glm::vec2(0,0);
    glm::vec2 from1 = coords - glm::vec2(1,0);
    glm::vec2 from2 = coords - glm::vec2(0,1);
    glm::vec2 from3 = coords - glm::vec2(1,1);

    float inf0 = glm::dot(grad[0], from0);
    float inf1 = glm::dot(grad[1], from1);
    float inf2 = glm::dot(grad[2], from2);
    float inf3 = glm::dot(grad[3], from3);

    float top = glm::mix(inf2, inf3, coords[0]);
    float bot = glm::mix(inf0, inf1, coords[0]);
    float mid = glm::mix(bot,top,coords[1]);

    return mid;
}

// Generate a point on a circle
glm::vec2 circleSample(float theta){
    return glm::vec2(cos(theta), sin(theta));
}

std::vector<float> Chunk::genPerlinNoise(uint64_t seed){
    std::mt19937 ran(seed);

    // Generate gradients at the corners, edge midpoints, and center
    glm::vec2 grads[9];
    for(int i = 0; i < 9; i++){
        grads[i] = circleSample(ran());
    }
}