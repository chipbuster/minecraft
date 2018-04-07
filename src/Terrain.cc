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
float perlinNoiseSquare(const glm::vec2& coords, glm::vec2* grad ){
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
    std::vector<float> outputs;

    // Generate gradients at the corners, edge midpoints, and center
    // Follow row-major bottom-left to top-right convention (see Terrain.h)
    glm::vec2 chunkGrads3[9];
    for(int i = 0; i < 9; i++){
        chunkGrads3[i] = circleSample(ran());
    }

    // Generate a higher octave at the corners only
    glm::vec2 chunkGrads2[4];
    for(int i = 0; i < 9; i++){
        chunkGrads2[i] = circleSample(ran());
    }

    // Generate 16 equispaced points on a (0,2) square.
    glm::vec2 cellGrads1[4]; // Octave 1 noise
    float delta = 2 / float(extent - 1);
    for(int i = 0; i < extent; i++){
        for(int j = 0; j < extent; j++){
            // Set grads
            if(i < extent/2){
                if(j < extent/2){
                    // lower-left quadrant
                    cellGrads1[0] = chunkGrads3[0];
                    cellGrads1[1] = chunkGrads3[1];
                    cellGrads1[2] = chunkGrads3[3];
                    cellGrads1[3] = chunkGrads3[4];
                }else{
                    // upper-left quadrant
                    cellGrads1[0] = chunkGrads3[3];
                    cellGrads1[1] = chunkGrads3[4];
                    cellGrads1[2] = chunkGrads3[6];
                    cellGrads1[3] = chunkGrads3[7];
                }
            }else{
                if(j < extent/2){
                    // lower-right quadrant
                    cellGrads1[0] = chunkGrads3[1];
                    cellGrads1[1] = chunkGrads3[2];
                    cellGrads1[2] = chunkGrads3[4];
                    cellGrads1[3] = chunkGrads3[5];
 
                }else{
                    // upper-right quadrant
                    cellGrads1[0] = chunkGrads3[4];
                    cellGrads1[1] = chunkGrads3[5];
                    cellGrads1[2] = chunkGrads3[7];
                    cellGrads1[3] = chunkGrads3[8];
                }
            }

            // Set coordinates
            float cellX = i * delta - (int)(i * delta);
            float cellY = j * delta - (int)(j * delta);
            glm::vec2 coordsLoOctave(cellX, cellY);
            glm::vec2 coordsHiOctave(i * delta / 2.0, j * delta / 2.0);

            int index = i * (extent) + j;

            outputs[index] = 0.5 * perlinNoiseSquare(coordsHiOctave, cellGrads1);
            outputs[index] += perlinNoiseSquare(coordsLoOctave, chunkGrads2);
            outputs[index] /= 1.5;
        }
    }

    return outputs;
}