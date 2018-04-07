#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <random>

#include <glm/glm.hpp>

// A chunk is a finite-sized (16x16?) size of cubes
class Chunk {
    uint32_t tex_seed; // Seed used to determine textures
    uint32_t per_seed; // Seed used to determine heights (via perlin noise)
    glm::vec2 loc;    // Coordinates of the bottom-left (x,z) corner
    int extent;       // Number of blocks in the x and z edges.
                      // both values should be powers of two.

    std::vector<float> genPerlinNoise(uint64_t seed);

  public:
   Chunk(const glm::vec2& location, int extent, std::mt19937& gen);
    
    /* All chunk functions return a single vector of values from low
       indices to high indices, varying quickly in x and slowly in z. Example:

                                              loc + (extent, extent)
       +z    8        9        10       11
       |     4        5        6        7
       |     0        1        2        3
       loc    --->  +x                                               */

    std::vector<float> heightMap() const;
    std::vector<uint64_t> texSeedMap() const;
};

//
class Terrain{

};