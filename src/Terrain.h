#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

// *** INDEXING CONVENTION *** //

/* All chunk functions return a single vector of values from low
   indices to high indices, varying quickly in x and slowly in z. Example:

   loc + (0, extent)                      loc + (extent, extent)
   +z    8        9        10       11
   |     4        5        6        7
   |     0        1        2        3
   loc    --->  +x                        loc + (extent, 0)

   This is the single-index indexing convention.

   For two indices, the first index varies with x and the second index varies
   with j.

   loc + (0, extent)                      loc + (extent, extent)
   +z    (0,2)   (1,2)    (2,2)    (3,2)
   |     (0,1)   (1,1)    (2,1)    (3,1)
   |     (0,0)   (1,0)    (2,0)    (3,0)
   loc    --->  +x                        loc + (extent, 0)

   This is the two-index indexing convention

*/

// A chunk is a finite-sized (16x16?) size of cubes
class Chunk {
    uint32_t tex_seed; // Seed used to determine textures
    uint32_t per_seed; // Seed used to determine heights (via perlin noise)
    glm::vec2 loc;     // Coordinates of the bottom-left (x,z) corner
    int extent;        // Number of blocks in the x and z edges.
                       // both values should be powers of two.

    std::vector<float> genPerlinNoise(uint64_t seed);

    public:
    Chunk(const glm::vec2& location, int extent, std::mt19937& gen);

    std::vector<float> heightMap() const;
    std::vector<uint64_t> texSeedMap() const;
};

//
class Terrain {
    std::unordered_map<glm::vec2, Chunk, std::hash<glm::vec2>,
                       std::equal_to<glm::vec2>>
            chunkMap;
    uint64_t seed;

    public:

};